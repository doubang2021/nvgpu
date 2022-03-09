/*
 * Ampere GPU series Copy Engine.
 *
 * Copyright (c) 2020-2022, NVIDIA CORPORATION.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <nvgpu/io.h>
#include <nvgpu/log.h>
#include <nvgpu/device.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/fifo.h>
#include <nvgpu/cic_mon.h>
#include <nvgpu/mc.h>

#include "hal/ce/ce_gv11b.h"
#include "hal/ce/ce_ga10b.h"

#include <nvgpu/hw/ga10b/hw_ce_ga10b.h>

static u32 ce_lce_intr_mask(void)
{
	/* Note: Poison error(fault containment) is not supported on GA10b. */
	u32 mask =
#ifdef CONFIG_NVGPU_NONSTALL_INTR
		ce_lce_intr_en_nonblockpipe_m() |
#endif
#ifdef CONFIG_NVGPU_HAL_NON_FUSA
		ce_lce_intr_en_stalling_debug_m() |
		ce_lce_intr_en_blockpipe_m() |
		ce_lce_intr_en_invalid_config_m() |
		ce_lce_intr_en_mthd_buffer_fault_m() |
		ce_lce_intr_en_fbuf_crc_fail_m() |
		ce_lce_intr_en_fbuf_magic_chk_fail_m() |
#endif
		ce_lce_intr_en_launcherr_m();
	return mask;
}

static void ga10b_ce_intr_stall_nonstall_enable(struct gk20a *g,
		const struct nvgpu_device *dev, bool enable)
{
	u32 intr_en_mask = 0U;
	u32 intr_ctrl = 0U;
	u32 intr_notify_ctrl = 0U;
	u32 intr_ctrl_msk = 0U;
	u32 intr_notify_ctrl_msk = 0U;
	u32 inst_id = dev->inst_id;

	intr_ctrl = nvgpu_readl(g, ce_lce_intr_ctrl_r(inst_id));
	intr_notify_ctrl =
		nvgpu_readl(g, ce_lce_intr_notify_ctrl_r(inst_id));
	/*
	 * The copy engine interrupts are enabled using a single enable
	 * register: ce_lce_intr_en_r. The interrupts generated by the
	 * CE engine are grouped into two:
	 * Stall:
	 * 	ce_lce_intr_en_launcherr_m()
	 * 	ce_lce_intr_en_stalling_debug_m()
	 *	ce_lce_intr_en_blockpipe_m()
	 *	ce_lce_intr_en_invalid_config_m()
	 *	ce_lce_intr_en_mthd_buffer_fault_m()
	 *	ce_lce_intr_en_fbuf_crc_fail_m()
	 *	ce_lce_intr_en_fbuf_magic_chk_fail_m()
	 * Non-Stall:
	 *	ce_lce_intr_en_nonblockpipe_m().
	 * Stalling interrupts are routed either to the cpu/gsp using
	 * the POR value of vector_id in register: ce_lce_intr_ctrl_r.
	 * This vector aligns with the intr_id field in device info.
	 * Similarly non-stalling interrupts are routed to cpu/gsp using
	 * the POR value of vector_id in register:
	 * ce_lce_intr_notify_ctrl_r. However unlike the former, the
	 * non-stalling interrupt vectors for GRCE0,1 share the vector
	 * id of GR engine. Hence there is a mis-alignment between the
	 * POR value of vector_id in ce_lce_intr_notify_ctrl_r register
	 * of GRCE0,1 with the intr_id field in the device info.
	 */
	if (enable) {
		/*
		 * Enable all stall, non-stall interrupts. Configure
		 * intr_(notify_,)_ctrl_r, so that all engine interrupts
		 * are reported to CPU on the POR values of vector_ids.
		 * In addition, disable reporting to GSP.
		 */
		intr_en_mask = ce_lce_intr_mask();
		intr_ctrl_msk = ce_lce_intr_ctrl_cpu_enable_f() |
			ce_lce_intr_ctrl_gsp_disable_f();
		intr_notify_ctrl_msk =
			ce_lce_intr_notify_ctrl_cpu_enable_f() |
			ce_lce_intr_notify_ctrl_gsp_disable_f();

	} else {
		/*
		 * Mask all interrupts from the engine and disable
		 * reporting to both CPU, GSP.
		 */
		intr_en_mask = 0U;
		intr_ctrl_msk = ce_lce_intr_ctrl_cpu_disable_f() |
			ce_lce_intr_ctrl_gsp_disable_f();
		intr_notify_ctrl_msk =
			ce_lce_intr_notify_ctrl_cpu_disable_f() |
			ce_lce_intr_notify_ctrl_gsp_disable_f();
	}

#ifdef CONFIG_NVGPU_NONSTALL_INTR
	/* Disable nonstalling CE interrupts on safety build */
	intr_notify_ctrl_msk =
		ce_lce_intr_notify_ctrl_cpu_disable_f() |
		ce_lce_intr_notify_ctrl_gsp_disable_f();
#endif

	intr_ctrl = set_field(intr_ctrl, ce_lce_intr_ctrl_cpu_m() |
			ce_lce_intr_ctrl_gsp_m(),
			intr_ctrl_msk);
	intr_notify_ctrl = set_field(intr_notify_ctrl,
			ce_lce_intr_notify_ctrl_cpu_m() |
			ce_lce_intr_notify_ctrl_gsp_m(),
			intr_notify_ctrl_msk);

	nvgpu_log(g, gpu_dbg_intr, "ce(%d) intr_ctrl(0x%x) "\
			"intr_notify_ctrl(0x%x) intr_en_mask(0x%x)",
			inst_id, intr_ctrl, intr_notify_ctrl,
			intr_en_mask);

	nvgpu_writel(g, ce_lce_intr_ctrl_r(inst_id), intr_ctrl);
	nvgpu_writel(g, ce_lce_intr_notify_ctrl_r(inst_id),
			intr_notify_ctrl);
	nvgpu_writel(g, ce_lce_intr_en_r(inst_id), intr_en_mask);
}

#ifdef CONFIG_NVGPU_NONSTALL_INTR
void ga10b_ce_init_hw(struct gk20a *g)
{
	u32 nonstall_vectorid_tree[NVGPU_CIC_INTR_VECTORID_SIZE_MAX];
	u32 num_nonstall_vectors = 0;
	const struct nvgpu_device *dev;

	nvgpu_device_for_each(g, dev, NVGPU_DEVTYPE_LCE) {
		/*
		 * The intr_id in dev info is broken for non-stall interrupts
		 * from grce0,1. Therefore, instead read the vectors from the
		 * POR values of intr_notify_ctrl_r.
		 */
		nonstall_vectorid_tree[num_nonstall_vectors] =
			ce_lce_intr_notify_ctrl_vector_v(
			nvgpu_readl(g, ce_lce_intr_notify_ctrl_r(dev->inst_id)));
		nvgpu_log(g, gpu_dbg_intr, "ce(%d) non-stall vector(%d)",
			  dev->inst_id,
			  nonstall_vectorid_tree[num_nonstall_vectors]);
		num_nonstall_vectors++;
	}

	/*
	 * Initalize struct nvgpu_mc with POR values of non-stall vectors ids.
	 */
	nvgpu_cic_mon_intr_unit_vectorid_init(g, NVGPU_CIC_INTR_UNIT_CE,
			nonstall_vectorid_tree, num_nonstall_vectors);
}
#endif

void ga10b_ce_intr_enable(struct gk20a *g, bool enable)
{
	const struct nvgpu_device *dev;

	nvgpu_device_for_each(g, dev, NVGPU_DEVTYPE_LCE) {
		ga10b_ce_intr_stall_nonstall_enable(g, dev, enable);
	}
}

void ga10b_ce_stall_isr(struct gk20a *g, u32 inst_id, u32 pri_base,
				bool *needs_rc, bool *needs_quiesce)
{
	u32 ce_intr = nvgpu_readl(g, ce_intr_status_r(inst_id));
	u32 clear_intr = 0U;

	nvgpu_log(g, gpu_dbg_intr, "ce(%u) isr 0x%08x 0x%08x", inst_id,
			ce_intr, inst_id);

#ifdef CONFIG_NVGPU_HAL_NON_FUSA
	/*
	 * Mismatch between the CRC entry in fault buffer and the
	 * CRC computed from the methods in the buffer.
	 */
	if ((ce_intr & ce_intr_status_fbuf_crc_fail_pending_f()) != 0U) {
		nvgpu_err(g, "ce: inst %d, fault buffer crc mismatch", inst_id);
		*needs_quiesce |= true;
		clear_intr |= ce_intr_status_fbuf_crc_fail_reset_f();
	}

	/*
	 * The MAGIC_NUM entry in fault buffer does not match with the expected
	 * value: NV_CE_MTHD_BUFFER_GLOBAL_HDR_MAGIC_NUM_VAL. This error
	 * indicates a memory corruption.
	 */
	if ((ce_intr & ce_intr_status_fbuf_magic_chk_fail_pending_f()) != 0U) {
		nvgpu_err(g, "ce: inst %d, fault buffer magic check fail",
				inst_id);
		*needs_quiesce |= true;
		clear_intr |= ce_intr_status_fbuf_magic_chk_fail_reset_f();
	}

	/*
	 * The stalling_debug error interrupt is triggered when SW writes TRUE
	 * to NV_CE_LCE_OPT_EXT_DEBUG_TRIGGER_STALLING.
	 */
	if ((ce_intr & ce_intr_status_stalling_debug_pending_f()) != 0U) {
		nvgpu_err(g, "ce: inst %d: stalling debug interrupt", inst_id);
		clear_intr |= ce_intr_status_stalling_debug_pending_f();
	}
#endif

	nvgpu_writel(g, ce_intr_status_r(inst_id), clear_intr);

	/*
	 * The remaining legacy interrupts are handled by legacy interrupt
	 * handler.
	 */
	gv11b_ce_stall_isr(g, inst_id, pri_base, needs_rc, needs_quiesce);
}

void ga10b_ce_intr_retrigger(struct gk20a *g, u32 inst_id)
{
	nvgpu_writel(g, ce_intr_retrigger_r(inst_id),
			ce_intr_retrigger_trigger_true_f());
}

void ga10b_ce_request_idle(struct gk20a *g)
{
	u32 num_pce;

	/*
	 * After CE engine reset, LCE0/LCE1 are not done with
	 * the reset sequence.The state of these LCE is RESET0.
	 * Extra ce pri read is needed to bring LCE0/1 out of reset.
	 * Without extra pri read after ce engine reset, ELPG does
	 * not engage after recovery due to IDLE_SNAP causing ELPG
	 * to not engage.
	 */

	num_pce = g->ops.ce.get_num_pce(g);
	nvgpu_log_info(g, "num_pce=%u", num_pce);
}
