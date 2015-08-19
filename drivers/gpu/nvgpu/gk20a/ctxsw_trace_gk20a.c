/*
 * Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 */

#include <asm/barrier.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/circ_buf.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/wait.h>
#include <linux/ktime.h>
#include <linux/nvgpu.h>
#include <linux/hashtable.h>
#include <linux/debugfs.h>
#include <linux/log2.h>
#include <uapi/linux/nvgpu.h>
#include "ctxsw_trace_gk20a.h"
#include "gk20a.h"
#include "gr_gk20a.h"
#include "hw_ctxsw_prog_gk20a.h"
#include "hw_gr_gk20a.h"

#define GK20A_CTXSW_TRACE_MAX_VM_RING_SIZE	(128*PAGE_SIZE)

/* Userland-facing FIFO (one global + eventually one per VM) */
struct gk20a_ctxsw_dev {
	struct gk20a *g;

	struct nvgpu_ctxsw_ring_header *hdr;
	struct nvgpu_ctxsw_trace_entry *ents;
	struct nvgpu_ctxsw_trace_filter filter;
	bool write_enabled;
	wait_queue_head_t readout_wq;
	size_t size;

	atomic_t vma_ref;

	struct mutex lock;
};


struct gk20a_ctxsw_trace {
	struct gk20a_ctxsw_dev devs[GK20A_CTXSW_TRACE_NUM_DEVS];
};

static inline int ring_is_empty(struct nvgpu_ctxsw_ring_header *hdr)
{
	return (hdr->write_idx == hdr->read_idx);
}

static inline int ring_is_full(struct nvgpu_ctxsw_ring_header *hdr)
{
	return ((hdr->write_idx + 1) % hdr->num_ents) == hdr->read_idx;
}

static inline int ring_len(struct nvgpu_ctxsw_ring_header *hdr)
{
	return (hdr->write_idx - hdr->read_idx) % hdr->num_ents;
}

static inline int ring_space(struct nvgpu_ctxsw_ring_header *hdr)
{
	return (hdr->read_idx - hdr->write_idx - 1) % hdr->num_ents;
}

ssize_t gk20a_ctxsw_dev_read(struct file *filp, char __user *buf, size_t size,
	loff_t *off)
{
	struct gk20a_ctxsw_dev *dev = filp->private_data;
	struct nvgpu_ctxsw_ring_header *hdr = dev->hdr;
	struct nvgpu_ctxsw_trace_entry __user *entry =
		(struct nvgpu_ctxsw_trace_entry *) buf;
	size_t copied = 0;
	int err;

	gk20a_dbg(gpu_dbg_fn|gpu_dbg_ctxsw,
		"filp=%p buf=%p size=%zu", filp, buf, size);

	mutex_lock(&dev->lock);
	while (ring_is_empty(hdr)) {
		mutex_unlock(&dev->lock);
		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
		err = wait_event_interruptible(dev->readout_wq,
			!ring_is_empty(hdr));
		if (err)
			return err;
		mutex_lock(&dev->lock);
	}

	while (size >= sizeof(struct nvgpu_ctxsw_trace_entry)) {
		if (ring_is_empty(hdr))
			break;

		if (copy_to_user(entry, &dev->ents[hdr->read_idx],
			sizeof(*entry))) {
			mutex_unlock(&dev->lock);
			return -EFAULT;
		}

		hdr->read_idx++;
		if (hdr->read_idx >= hdr->num_ents)
			hdr->read_idx = 0;

		entry++;
		copied += sizeof(*entry);
		size -= sizeof(*entry);
	}

	gk20a_dbg(gpu_dbg_ctxsw, "copied=%zu read_idx=%d", copied,
		hdr->read_idx);

	*off = hdr->read_idx;
	mutex_unlock(&dev->lock);

	return copied;
}

static int gk20a_ctxsw_dev_ioctl_trace_enable(struct gk20a_ctxsw_dev *dev)
{
	gk20a_dbg(gpu_dbg_fn|gpu_dbg_ctxsw, "trace enabled");
	dev->write_enabled = true;
	return 0;
}

static int gk20a_ctxsw_dev_ioctl_trace_disable(struct gk20a_ctxsw_dev *dev)
{
	gk20a_dbg(gpu_dbg_fn|gpu_dbg_ctxsw, "trace disabled");
	dev->write_enabled = false;
	return 0;
}

static int gk20a_ctxsw_dev_ring_alloc(struct gk20a_ctxsw_dev *dev,
		size_t size)
{
	struct nvgpu_ctxsw_ring_header *hdr;

	if (atomic_read(&dev->vma_ref))
		return -EBUSY;

	if ((dev->write_enabled) || (atomic_read(&dev->vma_ref)))
		return -EBUSY;

	size = roundup(size, PAGE_SIZE);
	hdr = vmalloc_user(size);
	if (!hdr)
		return -ENOMEM;

	if (dev->hdr)
		vfree(dev->hdr);

	dev->hdr = hdr;
	dev->ents = (struct nvgpu_ctxsw_trace_entry *) (dev->hdr + 1);
	dev->size = size;

	hdr->magic = NVGPU_CTXSW_RING_HEADER_MAGIC;
	hdr->version = NVGPU_CTXSW_RING_HEADER_VERSION;
	hdr->num_ents = (size - sizeof(struct nvgpu_ctxsw_ring_header))
		/ sizeof(struct nvgpu_ctxsw_trace_entry);
	hdr->ent_size = sizeof(struct nvgpu_ctxsw_trace_entry);
	hdr->drop_count = 0;
	hdr->read_idx = 0;
	hdr->write_idx = 0;
	hdr->write_seqno = 0;

	gk20a_dbg(gpu_dbg_ctxsw, "size=%zu hdr=%p ents=%p num_ents=%d",
		dev->size, dev->hdr, dev->ents, hdr->num_ents);
	return 0;
}

static int gk20a_ctxsw_dev_ioctl_ring_setup(struct gk20a_ctxsw_dev *dev,
	struct nvgpu_ctxsw_ring_setup_args *args)
{
	size_t size = args->size;

	gk20a_dbg(gpu_dbg_fn|gpu_dbg_ctxsw, "size=%zu", size);

	if (size > GK20A_CTXSW_TRACE_MAX_VM_RING_SIZE)
		return -EINVAL;

	return gk20a_ctxsw_dev_ring_alloc(dev, size);
}

static int gk20a_ctxsw_dev_ioctl_set_filter(struct gk20a_ctxsw_dev *dev,
	struct nvgpu_ctxsw_trace_filter_args *args)
{
	dev->filter = args->filter;
	return 0;
}

static int gk20a_ctxsw_dev_ioctl_get_filter(struct gk20a_ctxsw_dev *dev,
	struct nvgpu_ctxsw_trace_filter_args *args)
{
	args->filter = dev->filter;
	return 0;
}

static int gk20a_ctxsw_dev_ioctl_poll(struct gk20a_ctxsw_dev *dev)
{
	struct gk20a *g = dev->g;
	int err;

	gk20a_dbg(gpu_dbg_fn|gpu_dbg_ctxsw, "");

	err = gk20a_busy(g->dev);
	if (err)
		return err;

	if (g->ops.fecs_trace.flush(g))
		err = g->ops.fecs_trace.flush(g);

	if (likely(!err))
		err = g->ops.fecs_trace.poll(g);

	gk20a_idle(g->dev);
	return err;
}

int gk20a_ctxsw_dev_open(struct inode *inode, struct file *filp)
{
	struct gk20a *g;
	struct gk20a_ctxsw_trace *trace;
	struct gk20a_ctxsw_dev *dev;
	int err;
	size_t size;
	u32 n;

	/* only one VM for now */
	const int vmid = 0;

	g = container_of(inode->i_cdev, struct gk20a, ctxsw.cdev);
	gk20a_dbg(gpu_dbg_fn|gpu_dbg_ctxsw, "g=%p", g);

	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;

	err = gk20a_busy(g->dev);
	if (err)
		return err;

	trace = g->ctxsw_trace;
	if (!trace) {
		err = -ENODEV;
		goto idle;
	}

	/* Allow only one user for this device */
	dev = &trace->devs[vmid];
	mutex_lock(&dev->lock);
	if (dev->hdr) {
		err = -EBUSY;
		goto done;
	}

	/* By default, allocate ring buffer big enough to accommodate
	 * FECS records with default event filter */

	/* enable all traces by default */
	NVGPU_CTXSW_FILTER_SET_ALL(&dev->filter);

	/* compute max number of entries generated with this filter */
	n = g->ops.fecs_trace.max_entries(g, &dev->filter);

	size = sizeof(struct nvgpu_ctxsw_ring_header) +
			n * sizeof(struct nvgpu_ctxsw_trace_entry);
	gk20a_dbg(gpu_dbg_ctxsw, "size=%zu entries=%d ent_size=%zu",
		size, n, sizeof(struct nvgpu_ctxsw_trace_entry));

	err = gk20a_ctxsw_dev_ring_alloc(dev, size);
	if (!err) {
		filp->private_data = dev;
		gk20a_dbg(gpu_dbg_ctxsw, "filp=%p dev=%p size=%zu",
			filp, dev, size);
	}

	err = g->ops.fecs_trace.enable(g);

done:
	mutex_unlock(&dev->lock);

idle:
	gk20a_idle(g->dev);

	return err;
}

int gk20a_ctxsw_dev_release(struct inode *inode, struct file *filp)
{
	struct gk20a_ctxsw_dev *dev = filp->private_data;
	struct gk20a *g = container_of(inode->i_cdev, struct gk20a, ctxsw.cdev);

	gk20a_dbg(gpu_dbg_fn|gpu_dbg_ctxsw, "dev: %p", dev);

	mutex_lock(&dev->lock);
	dev->write_enabled = false;
	if (dev->hdr) {
		vfree(dev->hdr);
		dev->hdr = NULL;
	}

	g->ops.fecs_trace.disable(g);

	mutex_unlock(&dev->lock);

	return 0;
}

long gk20a_ctxsw_dev_ioctl(struct file *filp, unsigned int cmd,
	unsigned long arg)
{
	struct gk20a_ctxsw_dev *dev = filp->private_data;
	struct gk20a *g = dev->g;
	u8 buf[NVGPU_CTXSW_IOCTL_MAX_ARG_SIZE];
	int err = 0;

	gk20a_dbg(gpu_dbg_fn|gpu_dbg_ctxsw, "nr=%d", _IOC_NR(cmd));

	if ((_IOC_TYPE(cmd) != NVGPU_CTXSW_IOCTL_MAGIC) || (_IOC_NR(cmd) == 0)
		|| (_IOC_NR(cmd) > NVGPU_CTXSW_IOCTL_LAST))
		return -EINVAL;

	BUG_ON(_IOC_SIZE(cmd) > NVGPU_CTXSW_IOCTL_MAX_ARG_SIZE);

	memset(buf, 0, sizeof(buf));
	if (_IOC_DIR(cmd) & _IOC_WRITE) {
		if (copy_from_user(buf, (void __user *) arg, _IOC_SIZE(cmd)))
			return -EFAULT;
	}

	mutex_lock(&dev->lock);

	switch (cmd) {
	case NVGPU_CTXSW_IOCTL_TRACE_ENABLE:
		err = gk20a_ctxsw_dev_ioctl_trace_enable(dev);
		break;
	case NVGPU_CTXSW_IOCTL_TRACE_DISABLE:
		err = gk20a_ctxsw_dev_ioctl_trace_disable(dev);
		break;
	case NVGPU_CTXSW_IOCTL_RING_SETUP:
		err = gk20a_ctxsw_dev_ioctl_ring_setup(dev,
			(struct nvgpu_ctxsw_ring_setup_args *) buf);
		break;
	case NVGPU_CTXSW_IOCTL_SET_FILTER:
		err = gk20a_ctxsw_dev_ioctl_set_filter(dev,
			(struct nvgpu_ctxsw_trace_filter_args *) buf);
		break;
	case NVGPU_CTXSW_IOCTL_GET_FILTER:
		err = gk20a_ctxsw_dev_ioctl_get_filter(dev,
			(struct nvgpu_ctxsw_trace_filter_args *) buf);
		break;
	case NVGPU_CTXSW_IOCTL_POLL:
		mutex_unlock(&dev->lock);
		err = gk20a_ctxsw_dev_ioctl_poll(dev);
		mutex_lock(&dev->lock);
		break;
	default:
		dev_dbg(dev_from_gk20a(g), "unrecognized gpu ioctl cmd: 0x%x",
			cmd);
		err = -ENOTTY;
	}

	mutex_unlock(&dev->lock);

	if ((err == 0) && (_IOC_DIR(cmd) & _IOC_READ))
		err = copy_to_user((void __user *) arg, buf, _IOC_SIZE(cmd));

	return err;
}

unsigned int gk20a_ctxsw_dev_poll(struct file *filp, poll_table *wait)
{
	struct gk20a_ctxsw_dev *dev = filp->private_data;
	struct nvgpu_ctxsw_ring_header *hdr = dev->hdr;
	unsigned int mask = 0;

	gk20a_dbg(gpu_dbg_fn|gpu_dbg_ctxsw, "");

	mutex_lock(&dev->lock);
	poll_wait(filp, &dev->readout_wq, wait);
	if (!ring_is_empty(hdr))
		mask |= POLLIN | POLLRDNORM;
	mutex_unlock(&dev->lock);

	return mask;
}

static void gk20a_ctxsw_dev_vma_open(struct vm_area_struct *vma)
{
	struct gk20a_ctxsw_dev *dev = vma->vm_private_data;

	atomic_inc(&dev->vma_ref);
	gk20a_dbg(gpu_dbg_fn|gpu_dbg_ctxsw, "vma_ref=%d",
		atomic_read(&dev->vma_ref));
}

static void gk20a_ctxsw_dev_vma_close(struct vm_area_struct *vma)
{
	struct gk20a_ctxsw_dev *dev = vma->vm_private_data;

	atomic_dec(&dev->vma_ref);
	gk20a_dbg(gpu_dbg_fn|gpu_dbg_ctxsw, "vma_ref=%d",
		atomic_read(&dev->vma_ref));
}

static struct vm_operations_struct gk20a_ctxsw_dev_vma_ops = {
	.open = gk20a_ctxsw_dev_vma_open,
	.close = gk20a_ctxsw_dev_vma_close,
};

int gk20a_ctxsw_dev_mmap(struct file *filp, struct vm_area_struct *vma)
{
	struct gk20a_ctxsw_dev *dev = filp->private_data;
	int ret;

	gk20a_dbg(gpu_dbg_fn|gpu_dbg_ctxsw, "vm_start=%lx vm_end=%lx",
		vma->vm_start, vma->vm_end);

	ret = remap_vmalloc_range(vma, dev->hdr, 0);
	if (likely(!ret)) {
		vma->vm_private_data = dev;
		vma->vm_ops = &gk20a_ctxsw_dev_vma_ops;
		vma->vm_ops->open(vma);
	}

	return ret;
}

#ifdef CONFIG_GK20A_CTXSW_TRACE
static int gk20a_ctxsw_init_devs(struct gk20a *g)
{
	struct gk20a_ctxsw_trace *trace = g->ctxsw_trace;
	struct gk20a_ctxsw_dev *dev = trace->devs;
	int i;

	for (i = 0; i < GK20A_CTXSW_TRACE_NUM_DEVS; i++) {
		dev->g = g;
		dev->hdr = NULL;
		dev->write_enabled = false;
		init_waitqueue_head(&dev->readout_wq);
		mutex_init(&dev->lock);
		atomic_set(&dev->vma_ref, 0);
		dev++;
	}
	return 0;
}
#endif

int gk20a_ctxsw_trace_init(struct gk20a *g)
{
#ifdef CONFIG_GK20A_CTXSW_TRACE
	struct gk20a_ctxsw_trace *trace = g->ctxsw_trace;
	int err;

	gk20a_dbg(gpu_dbg_fn|gpu_dbg_ctxsw, "g=%p trace=%p", g, trace);

	if (likely(trace))
		return 0;

	trace = kzalloc(sizeof(*trace), GFP_KERNEL);
	if (unlikely(!trace))
		return -ENOMEM;
	g->ctxsw_trace = trace;

	err = gk20a_ctxsw_init_devs(g);
	if (err)
		goto fail;

	err = g->ops.fecs_trace.init(g);
	if (unlikely(err))
		goto fail;

	return 0;

fail:
	kfree(trace);
	g->ctxsw_trace = NULL;
	return err;
#else
	return 0;
#endif
}

void gk20a_ctxsw_trace_cleanup(struct gk20a *g)
{
#ifdef CONFIG_GK20A_CTXSW_TRACE
	kfree(g->ctxsw_trace);
	g->ctxsw_trace = NULL;

	g->ops.fecs_trace.deinit(g);
#endif
}

int gk20a_ctxsw_trace_write(struct gk20a *g,
		struct nvgpu_ctxsw_trace_entry *entry)
{
	struct nvgpu_ctxsw_ring_header *hdr;
	struct gk20a_ctxsw_dev *dev;
	int ret = 0;
	const char *reason;

	if (unlikely(entry->vmid >= GK20A_CTXSW_TRACE_NUM_DEVS))
		return -ENODEV;

	dev = &g->ctxsw_trace->devs[entry->vmid];
	hdr = dev->hdr;

	gk20a_dbg(gpu_dbg_fn | gpu_dbg_ctxsw,
		"dev=%p hdr=%p", dev, hdr);

	mutex_lock(&dev->lock);

	if (unlikely(!hdr)) {
		/* device has been released */
		ret = -ENODEV;
		goto done;
	}

	entry->seqno = hdr->write_seqno++;

	if (!dev->write_enabled) {
		ret = -EBUSY;
		reason = "write disabled";
		goto drop;
	}

	if (unlikely(ring_is_full(hdr))) {
		ret = -ENOSPC;
		reason = "user fifo full";
		goto drop;
	}

	if (!NVGPU_CTXSW_FILTER_ISSET(entry->tag, &dev->filter)) {
		reason = "filtered out";
		goto filter;
	}

	gk20a_dbg(gpu_dbg_ctxsw,
		"seqno=%d context_id=%08x pid=%lld tag=%x timestamp=%llx",
		entry->seqno, entry->context_id, entry->pid,
		entry->tag, entry->timestamp);

	dev->ents[hdr->write_idx] = *entry;

	/* ensure record is written before updating write index */
	smp_wmb();

	hdr->write_idx++;
	if (unlikely(hdr->write_idx >= hdr->num_ents))
		hdr->write_idx = 0;
	gk20a_dbg(gpu_dbg_ctxsw, "added: read=%d write=%d len=%d",
		hdr->read_idx, hdr->write_idx, ring_len(hdr));

	mutex_unlock(&dev->lock);
	return ret;

drop:
	hdr->drop_count++;

filter:
	gk20a_dbg(gpu_dbg_ctxsw,
			"dropping seqno=%d context_id=%08x pid=%lld "
			"tag=%x time=%llx (%s)",
			entry->seqno, entry->context_id, entry->pid,
			entry->tag, entry->timestamp, reason);

done:
	mutex_unlock(&dev->lock);
	return ret;
}

void gk20a_ctxsw_trace_wake_up(struct gk20a *g, int vmid)
{
	struct gk20a_ctxsw_dev *dev = &g->ctxsw_trace->devs[vmid];

	wake_up_interruptible(&dev->readout_wq);
}
