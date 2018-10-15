/*
 * Copyright (c) 2016-2018, NVIDIA CORPORATION.  All rights reserved.
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

#include <nvgpu/bios.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/boardobjgrp.h>
#include <nvgpu/boardobjgrp_e32.h>
#include <nvgpu/string.h>
#include <nvgpu/pmuif/ctrlclk.h>
#include <nvgpu/pmuif/ctrlvolt.h>

#include "clk.h"
#include "clk_fll.h"
#include "clk_domain.h"

static struct clk_domain *construct_clk_domain(struct gk20a *g, void *pargs);

static int devinit_get_clocks_table(struct gk20a *g,
	struct clk_domains *pclkdomainobjs);

static int clk_domain_pmudatainit_super(struct gk20a *g, struct boardobj
	*board_obj_ptr,	struct nv_pmu_boardobj *ppmudata);

static struct vbios_clocks_table_1x_hal_clock_entry
		vbiosclktbl1xhalentry_gp[] = {
	{ clkwhich_gpc2clk,    true,    1, },
	{ clkwhich_xbar2clk,   true,    1, },
	{ clkwhich_mclk,       false,   1, },
	{ clkwhich_sys2clk,    true,    1, },
	{ clkwhich_hub2clk,    false,   1, },
	{ clkwhich_nvdclk,     false,   1, },
	{ clkwhich_pwrclk,     false,   1, },
	{ clkwhich_dispclk,    false,   1, },
	{ clkwhich_pciegenclk, false,   1, }
};
/*
 * Updated from RM devinit_clock.c
 * GV100 is 0x03 and
 * GP10x is 0x02 in clocks_hal.
 */
static struct vbios_clocks_table_1x_hal_clock_entry
		vbiosclktbl1xhalentry_gv[] = {
	{ clkwhich_gpcclk,     true,    2, },
	{ clkwhich_xbarclk,    true,    1, },
	{ clkwhich_mclk,       false,   1, },
	{ clkwhich_sysclk,     true,    1, },
	{ clkwhich_hubclk,     false,   1, },
	{ clkwhich_nvdclk,     true,    1, },
	{ clkwhich_pwrclk,     false,   1, },
	{ clkwhich_dispclk,    false,   1, },
	{ clkwhich_pciegenclk, false,   1, },
	{ clkwhich_hostclk,    true,    1, }
};

static u32 clktranslatehalmumsettoapinumset(u32 clkhaldomains)
{
	u32   clkapidomains = 0;

	if (clkhaldomains & BIT(clkwhich_gpcclk)) {
		clkapidomains |= CTRL_CLK_DOMAIN_GPCCLK;
	}
	if (clkhaldomains & BIT(clkwhich_xbarclk)) {
		clkapidomains |= CTRL_CLK_DOMAIN_XBARCLK;
	}
	if (clkhaldomains & BIT(clkwhich_sysclk)) {
		clkapidomains |= CTRL_CLK_DOMAIN_SYSCLK;
	}
	if (clkhaldomains & BIT(clkwhich_hubclk)) {
		clkapidomains |= CTRL_CLK_DOMAIN_HUBCLK;
	}
	if (clkhaldomains & BIT(clkwhich_hostclk)) {
		clkapidomains |= CTRL_CLK_DOMAIN_HOSTCLK;
	}
	if (clkhaldomains & BIT(clkwhich_gpc2clk)) {
		clkapidomains |= CTRL_CLK_DOMAIN_GPC2CLK;
	}
	if (clkhaldomains & BIT(clkwhich_xbar2clk)) {
		clkapidomains |= CTRL_CLK_DOMAIN_XBAR2CLK;
	}
	if (clkhaldomains & BIT(clkwhich_sys2clk)) {
		clkapidomains |= CTRL_CLK_DOMAIN_SYS2CLK;
	}
	if (clkhaldomains & BIT(clkwhich_hub2clk)) {
		clkapidomains |= CTRL_CLK_DOMAIN_HUB2CLK;
	}
	if (clkhaldomains & BIT(clkwhich_pwrclk)) {
		clkapidomains |= CTRL_CLK_DOMAIN_PWRCLK;
	}
	if (clkhaldomains & BIT(clkwhich_pciegenclk)) {
		clkapidomains |= CTRL_CLK_DOMAIN_PCIEGENCLK;
	}
	if (clkhaldomains & BIT(clkwhich_mclk)) {
		clkapidomains |= CTRL_CLK_DOMAIN_MCLK;
	}
	if (clkhaldomains & BIT(clkwhich_nvdclk)) {
		clkapidomains |= CTRL_CLK_DOMAIN_NVDCLK;
	}
	if (clkhaldomains & BIT(clkwhich_dispclk)) {
		clkapidomains |= CTRL_CLK_DOMAIN_DISPCLK;
	}

	return clkapidomains;
}

static int _clk_domains_pmudatainit_3x(struct gk20a *g,
				       struct boardobjgrp *pboardobjgrp,
				       struct nv_pmu_boardobjgrp_super *pboardobjgrppmu)
{
	struct nv_pmu_clk_clk_domain_boardobjgrp_set_header *pset =
		(struct nv_pmu_clk_clk_domain_boardobjgrp_set_header *)
		pboardobjgrppmu;
	struct clk_domains *pdomains = (struct clk_domains *)pboardobjgrp;
	int status = 0;

	status = boardobjgrp_pmudatainit_e32(g, pboardobjgrp, pboardobjgrppmu);
	if (status != 0) {
		nvgpu_err(g,
			  "error updating pmu boardobjgrp for clk domain 0x%x",
			  status);
		goto done;
	}

	pset->vbios_domains = pdomains->vbios_domains;
	pset->cntr_sampling_periodms = pdomains->cntr_sampling_periodms;
	pset->version = CLK_DOMAIN_BOARDOBJGRP_VERSION;
	pset->b_override_o_v_o_c = false;
	pset->b_debug_mode = false;
	pset->b_enforce_vf_monotonicity = pdomains->b_enforce_vf_monotonicity;
	pset->b_enforce_vf_smoothening = pdomains->b_enforce_vf_smoothening;
	if (g->ops.clk.split_rail_support) {
		pset->volt_rails_max = 2;
	} else {
		pset->volt_rails_max = 1;
	}
	status = boardobjgrpmask_export(
				&pdomains->master_domains_mask.super,
				pdomains->master_domains_mask.super.bitcount,
				&pset->master_domains_mask.super);

	nvgpu_memcpy((u8 *)&pset->deltas, (u8 *)&pdomains->deltas,
		(sizeof(struct ctrl_clk_clk_delta)));

done:
	return status;
}

static int _clk_domains_pmudata_instget(struct gk20a *g,
					struct nv_pmu_boardobjgrp *pmuboardobjgrp,
					struct nv_pmu_boardobj **ppboardobjpmudata,
					u8 idx)
{
	struct nv_pmu_clk_clk_domain_boardobj_grp_set  *pgrp_set =
		(struct nv_pmu_clk_clk_domain_boardobj_grp_set *)
		pmuboardobjgrp;

	nvgpu_log_info(g, " ");

	/*check whether pmuboardobjgrp has a valid boardobj in index*/
	if (((u32)BIT(idx) &
		pgrp_set->hdr.data.super.obj_mask.super.data[0]) == 0U) {
		return -EINVAL;
	}

	*ppboardobjpmudata = (struct nv_pmu_boardobj *)
		&pgrp_set->objects[idx].data.board_obj;
	nvgpu_log_info(g, " Done");
	return 0;
}

int clk_domain_sw_setup(struct gk20a *g)
{
	int status;
	struct boardobjgrp *pboardobjgrp = NULL;
	struct clk_domains *pclkdomainobjs;
	struct clk_domain *pdomain;
	struct clk_domain_3x_master *pdomain_master;
	struct clk_domain_3x_slave *pdomain_slave;
	u8 i;

	nvgpu_log_info(g, " ");

	status = boardobjgrpconstruct_e32(g, &g->clk_pmu->clk_domainobjs.super);
	if (status != 0) {
		nvgpu_err(g,
			  "error creating boardobjgrp for clk domain, status - 0x%x",
			  status);
		goto done;
	}

	pboardobjgrp = &g->clk_pmu->clk_domainobjs.super.super;
	pclkdomainobjs = &(g->clk_pmu->clk_domainobjs);

	BOARDOBJGRP_PMU_CONSTRUCT(pboardobjgrp, CLK, CLK_DOMAIN);

	status = BOARDOBJGRP_PMU_CMD_GRP_SET_CONSTRUCT(g, pboardobjgrp,
			clk, CLK, clk_domain, CLK_DOMAIN);
	if (status != 0) {
		nvgpu_err(g,
			  "error constructing PMU_BOARDOBJ_CMD_GRP_SET interface - 0x%x",
			  status);
		goto done;
	}

	pboardobjgrp->pmudatainit  = _clk_domains_pmudatainit_3x;
	pboardobjgrp->pmudatainstget  = _clk_domains_pmudata_instget;

	/* Initialize mask to zero.*/
	boardobjgrpmask_e32_init(&pclkdomainobjs->prog_domains_mask, NULL);
	boardobjgrpmask_e32_init(&pclkdomainobjs->master_domains_mask, NULL);
	pclkdomainobjs->b_enforce_vf_monotonicity = true;
	pclkdomainobjs->b_enforce_vf_smoothening = true;

	(void) memset(&pclkdomainobjs->ordered_noise_aware_list, 0,
		sizeof(pclkdomainobjs->ordered_noise_aware_list));

	(void) memset(&pclkdomainobjs->ordered_noise_unaware_list, 0,
		sizeof(pclkdomainobjs->ordered_noise_unaware_list));

	(void) memset(&pclkdomainobjs->deltas, 0,
		sizeof(struct ctrl_clk_clk_delta));

	status = devinit_get_clocks_table(g, pclkdomainobjs);
	if (status != 0) {
		goto done;
	}

	BOARDOBJGRP_FOR_EACH(&(pclkdomainobjs->super.super),
			     struct clk_domain *, pdomain, i) {
		pdomain_master = NULL;
		if (pdomain->super.implements(g, &pdomain->super,
				CTRL_CLK_CLK_DOMAIN_TYPE_3X_PROG)) {
			status = boardobjgrpmask_bitset(
				&pclkdomainobjs->prog_domains_mask.super, i);
			if (status != 0) {
				goto done;
			}
		}

		if (pdomain->super.implements(g, &pdomain->super,
				CTRL_CLK_CLK_DOMAIN_TYPE_3X_MASTER)) {
			status = boardobjgrpmask_bitset(
				&pclkdomainobjs->master_domains_mask.super, i);
			if (status != 0) {
				goto done;
			}
		}

		if (pdomain->super.implements(g, &pdomain->super,
				CTRL_CLK_CLK_DOMAIN_TYPE_3X_SLAVE)) {
				pdomain_slave =
					(struct clk_domain_3x_slave *)pdomain;
				pdomain_master =
					(struct clk_domain_3x_master *)
					(CLK_CLK_DOMAIN_GET((g->clk_pmu),
					pdomain_slave->master_idx));
			pdomain_master->slave_idxs_mask |= BIT(i);
		}

	}

done:
	nvgpu_log_info(g, " done status %x", status);
	return status;
}

int clk_domain_pmu_setup(struct gk20a *g)
{
	int status;
	struct boardobjgrp *pboardobjgrp = NULL;

	nvgpu_log_info(g, " ");

	pboardobjgrp = &g->clk_pmu->clk_domainobjs.super.super;

	if (!pboardobjgrp->bconstructed) {
		return -EINVAL;
	}

	status = pboardobjgrp->pmuinithandle(g, pboardobjgrp);

	nvgpu_log_info(g, "Done");
	return status;
}

static int devinit_get_clocks_table_35(struct gk20a *g,
				    struct clk_domains *pclkdomainobjs, u8 *clocks_table_ptr)
{
	int status = 0;
	struct vbios_clocks_table_35_header clocks_table_header = { 0 };
	struct vbios_clocks_table_35_entry clocks_table_entry = { 0 };
	struct vbios_clocks_table_1x_hal_clock_entry *vbiosclktbl1xhalentry;
	u8 *clocks_tbl_entry_ptr = NULL;
	u32 index = 0;
	struct clk_domain *pclkdomain_dev;
	union {
		struct boardobj boardobj;
		struct clk_domain clk_domain;
		struct clk_domain_3x v3x;
		struct clk_domain_3x_fixed v3x_fixed;
		struct clk_domain_35_prog v35_prog;
		struct clk_domain_35_master v35_master;
		struct clk_domain_35_slave v35_slave;
	} clk_domain_data;

	nvgpu_log_info(g, " ");

	nvgpu_memcpy((u8 *)&clocks_table_header, clocks_table_ptr,
			VBIOS_CLOCKS_TABLE_35_HEADER_SIZE_09);
	if (clocks_table_header.header_size <
			(u8) VBIOS_CLOCKS_TABLE_35_HEADER_SIZE_09) {
		status = -EINVAL;
		goto done;
	}

	if (clocks_table_header.entry_size <
			(u8) VBIOS_CLOCKS_TABLE_35_ENTRY_SIZE_11) {
		status = -EINVAL;
		goto done;
	}

	switch (clocks_table_header.clocks_hal) {
	case CLK_TABLE_HAL_ENTRY_GP:
	{
		vbiosclktbl1xhalentry = vbiosclktbl1xhalentry_gp;
		break;
	}
	case CLK_TABLE_HAL_ENTRY_GV:
	{
		vbiosclktbl1xhalentry = vbiosclktbl1xhalentry_gv;
		break;
	}
	default:
	{
		status = -EINVAL;
		goto done;
	}
	}

	pclkdomainobjs->cntr_sampling_periodms =
		(u16)clocks_table_header.cntr_sampling_periodms;

	/* Read table entries*/
	clocks_tbl_entry_ptr = clocks_table_ptr +
			clocks_table_header.header_size;
	for (index = 0; index < clocks_table_header.entry_count; index++) {
		nvgpu_memcpy((u8 *)&clocks_table_entry,
			clocks_tbl_entry_ptr, clocks_table_header.entry_size);
		clk_domain_data.clk_domain.domain =
				(u8) vbiosclktbl1xhalentry[index].domain;
		clk_domain_data.clk_domain.api_domain =
				clktranslatehalmumsettoapinumset(
					(u32) BIT(clk_domain_data.clk_domain.domain));
		clk_domain_data.v3x.b_noise_aware_capable =
			vbiosclktbl1xhalentry[index].b_noise_aware_capable;

		switch (BIOS_GET_FIELD(clocks_table_entry.flags0,
				NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_FLAGS0_USAGE)) {
		case  NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_FLAGS0_USAGE_FIXED:
		{
			clk_domain_data.boardobj.type =
				CTRL_CLK_CLK_DOMAIN_TYPE_3X_FIXED;
			clk_domain_data.v3x_fixed.freq_mhz = (u16)BIOS_GET_FIELD(
				clocks_table_entry.param1,
				NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_PARAM1_FIXED_FREQUENCY_MHZ);
			break;
		}

		case  NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_FLAGS0_USAGE_MASTER:
		{
			clk_domain_data.boardobj.type =
				CTRL_CLK_CLK_DOMAIN_TYPE_35_MASTER;
			clk_domain_data.v35_prog.super.clk_prog_idx_first =
				(u8)(BIOS_GET_FIELD(clocks_table_entry.param0,
				     NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_PARAM0_PROG_CLK_PROG_IDX_FIRST));
			clk_domain_data.v35_prog.super.clk_prog_idx_last =
				(u8)(BIOS_GET_FIELD(clocks_table_entry.param0,
				     NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_PARAM0_PROG_CLK_PROG_IDX_LAST));
			clk_domain_data.v35_prog.super.noise_unaware_ordering_index =
				(u8)(BIOS_GET_FIELD(clocks_table_entry.param2,
				     NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_PARAM2_PROG_NOISE_UNAWARE_ORDERING_IDX));
			if (clk_domain_data.v3x.b_noise_aware_capable) {
				clk_domain_data.v35_prog.super.b_force_noise_unaware_ordering =
					(bool)(BIOS_GET_FIELD(clocks_table_entry.param2,
					NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_PARAM2_PROG_FORCE_NOISE_UNAWARE_ORDERING));

			} else {
				clk_domain_data.v35_prog.super.noise_aware_ordering_index =
					CTRL_CLK_CLK_DOMAIN_3X_PROG_ORDERING_INDEX_INVALID;
				clk_domain_data.v35_prog.super.b_force_noise_unaware_ordering = false;
			}
			clk_domain_data.v35_prog.pre_volt_ordering_index =
				(u8)(BIOS_GET_FIELD(clocks_table_entry.param2,
					NV_VBIOS_CLOCKS_TABLE_35_ENTRY_PARAM2_PROG_PRE_VOLT_ORDERING_IDX));

			clk_domain_data.v35_prog.post_volt_ordering_index =
				(u8)(BIOS_GET_FIELD(clocks_table_entry.param2,
					NV_VBIOS_CLOCKS_TABLE_35_ENTRY_PARAM2_PROG_POST_VOLT_ORDERING_IDX));

			clk_domain_data.v35_prog.super.factory_delta.data.delta_khz = 0;
			clk_domain_data.v35_prog.super.factory_delta.type = 0;

			clk_domain_data.v35_prog.super.freq_delta_min_mhz =
				(u16)(BIOS_GET_FIELD(clocks_table_entry.param1,
				      NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_PARAM1_MASTER_FREQ_OC_DELTA_MIN_MHZ));

			clk_domain_data.v35_prog.super.freq_delta_max_mhz =
				(u16)(BIOS_GET_FIELD(clocks_table_entry.param1,
				      NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_PARAM1_MASTER_FREQ_OC_DELTA_MAX_MHZ));
			clk_domain_data.v35_prog.clk_vf_curve_count =
				vbiosclktbl1xhalentry[index].clk_vf_curve_count;
			break;
		}

		case  NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_FLAGS0_USAGE_SLAVE:
		{
			clk_domain_data.boardobj.type =
				CTRL_CLK_CLK_DOMAIN_TYPE_35_SLAVE;
			clk_domain_data.v35_prog.super.clk_prog_idx_first =
				(u8)(BIOS_GET_FIELD(clocks_table_entry.param0,
				     NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_PARAM0_PROG_CLK_PROG_IDX_FIRST));
			clk_domain_data.v35_prog.super.clk_prog_idx_last =
				(u8)(BIOS_GET_FIELD(clocks_table_entry.param0,
				     NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_PARAM0_PROG_CLK_PROG_IDX_LAST));
			clk_domain_data.v35_prog.super.noise_unaware_ordering_index =
				(u8)(BIOS_GET_FIELD(clocks_table_entry.param2,
				     NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_PARAM2_PROG_NOISE_UNAWARE_ORDERING_IDX));

			if (clk_domain_data.v3x.b_noise_aware_capable) {
				clk_domain_data.v35_prog.super.b_force_noise_unaware_ordering =
					(bool)(BIOS_GET_FIELD(clocks_table_entry.param2,
					NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_PARAM2_PROG_FORCE_NOISE_UNAWARE_ORDERING));

			} else {
				clk_domain_data.v35_prog.super.noise_aware_ordering_index =
					CTRL_CLK_CLK_DOMAIN_3X_PROG_ORDERING_INDEX_INVALID;
				clk_domain_data.v35_prog.super.b_force_noise_unaware_ordering = false;
			}
			clk_domain_data.v35_prog.pre_volt_ordering_index =
				(u8)(BIOS_GET_FIELD(clocks_table_entry.param2,
					NV_VBIOS_CLOCKS_TABLE_35_ENTRY_PARAM2_PROG_PRE_VOLT_ORDERING_IDX));

			clk_domain_data.v35_prog.post_volt_ordering_index =
				(u8)(BIOS_GET_FIELD(clocks_table_entry.param2,
					NV_VBIOS_CLOCKS_TABLE_35_ENTRY_PARAM2_PROG_POST_VOLT_ORDERING_IDX));

			clk_domain_data.v35_prog.super.factory_delta.data.delta_khz = 0;
			clk_domain_data.v35_prog.super.factory_delta.type = 0;
			clk_domain_data.v35_prog.super.freq_delta_min_mhz = 0;
			clk_domain_data.v35_prog.super.freq_delta_max_mhz = 0;
			clk_domain_data.v35_slave.slave.master_idx =
				(u8)(BIOS_GET_FIELD(clocks_table_entry.param1,
				     NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_PARAM1_SLAVE_MASTER_DOMAIN));
			break;
		}

		default:
		{
			nvgpu_err(g,
				  "error reading clock domain entry %d", index);
			status = -EINVAL;
			goto done;
		}

		}
		pclkdomain_dev = construct_clk_domain(g,
				(void *)&clk_domain_data);
		if (pclkdomain_dev == NULL) {
			nvgpu_err(g,
				  "unable to construct clock domain boardobj for %d",
				  index);
			status = -EINVAL;
			goto done;
		}
		status = boardobjgrp_objinsert(
				&pclkdomainobjs->super.super,
				(struct boardobj *)(void*) pclkdomain_dev, index);
		if (status != 0) {
			nvgpu_err(g,
			"unable to insert clock domain boardobj for %d", index);
			status = (u32) -EINVAL;
			goto done;
		}
		clocks_tbl_entry_ptr += clocks_table_header.entry_size;
	}

done:
	nvgpu_log_info(g, " done status %x", status);
	return status;
}

static int devinit_get_clocks_table_1x(struct gk20a *g,
				    struct clk_domains *pclkdomainobjs, u8 *clocks_table_ptr)
{
	int status = 0;
	struct vbios_clocks_table_1x_header clocks_table_header = { 0 };
	struct vbios_clocks_table_1x_entry clocks_table_entry = { 0 };
	struct vbios_clocks_table_1x_hal_clock_entry *vbiosclktbl1xhalentry;
	u8 *clocks_tbl_entry_ptr = NULL;
	u32 index = 0;
	struct clk_domain *pclkdomain_dev;
	union {
		struct boardobj boardobj;
		struct clk_domain clk_domain;
		struct clk_domain_3x v3x;
		struct clk_domain_3x_fixed v3x_fixed;
		struct clk_domain_3x_prog v3x_prog;
		struct clk_domain_3x_master v3x_master;
		struct clk_domain_3x_slave v3x_slave;
	} clk_domain_data;

	nvgpu_log_info(g, " ");

	nvgpu_memcpy((u8 *)&clocks_table_header, clocks_table_ptr,
			VBIOS_CLOCKS_TABLE_1X_HEADER_SIZE_07);
	if (clocks_table_header.header_size <
			(u8) VBIOS_CLOCKS_TABLE_1X_HEADER_SIZE_07) {
		status = -EINVAL;
		goto done;
	}

	if (clocks_table_header.entry_size <
	    (u8) VBIOS_CLOCKS_TABLE_1X_ENTRY_SIZE_09) {
		status = -EINVAL;
		goto done;
	}

	switch (clocks_table_header.clocks_hal) {
	case CLK_TABLE_HAL_ENTRY_GP:
	{
		vbiosclktbl1xhalentry = vbiosclktbl1xhalentry_gp;
		break;
	}
	case CLK_TABLE_HAL_ENTRY_GV:
	{
		vbiosclktbl1xhalentry = vbiosclktbl1xhalentry_gv;
		break;
	}
	default:
	{
		status = -EINVAL;
		goto done;
	}
	}

	pclkdomainobjs->cntr_sampling_periodms =
		(u16)clocks_table_header.cntr_sampling_periodms;

	/* Read table entries*/
	clocks_tbl_entry_ptr = clocks_table_ptr +
		VBIOS_CLOCKS_TABLE_1X_HEADER_SIZE_07;
	for (index = 0; index < clocks_table_header.entry_count; index++) {
		nvgpu_memcpy((u8 *)&clocks_table_entry,
			clocks_tbl_entry_ptr, clocks_table_header.entry_size);
		clk_domain_data.clk_domain.domain =
				(u8) vbiosclktbl1xhalentry[index].domain;
		clk_domain_data.clk_domain.api_domain =
				clktranslatehalmumsettoapinumset(
					BIT(clk_domain_data.clk_domain.domain));
		clk_domain_data.v3x.b_noise_aware_capable =
			vbiosclktbl1xhalentry[index].b_noise_aware_capable;

		switch (BIOS_GET_FIELD(clocks_table_entry.flags0,
				NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_FLAGS0_USAGE)) {
		case  NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_FLAGS0_USAGE_FIXED:
		{
			clk_domain_data.boardobj.type =
				CTRL_CLK_CLK_DOMAIN_TYPE_3X_FIXED;
			clk_domain_data.v3x_fixed.freq_mhz = (u16)BIOS_GET_FIELD(
				clocks_table_entry.param1,
				NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_PARAM1_FIXED_FREQUENCY_MHZ);
			break;
		}

		case  NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_FLAGS0_USAGE_MASTER:
		{
			clk_domain_data.boardobj.type =
				CTRL_CLK_CLK_DOMAIN_TYPE_3X_MASTER;
			clk_domain_data.v3x_prog.clk_prog_idx_first =
				(u8)(BIOS_GET_FIELD(clocks_table_entry.param0,
				     NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_PARAM0_PROG_CLK_PROG_IDX_FIRST));
			clk_domain_data.v3x_prog.clk_prog_idx_last =
				(u8)(BIOS_GET_FIELD(clocks_table_entry.param0,
				     NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_PARAM0_PROG_CLK_PROG_IDX_LAST));
			clk_domain_data.v3x_prog.noise_unaware_ordering_index =
				(u8)(BIOS_GET_FIELD(clocks_table_entry.param2,
				     NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_PARAM2_PROG_NOISE_UNAWARE_ORDERING_IDX));
			if (clk_domain_data.v3x.b_noise_aware_capable) {
				clk_domain_data.v3x_prog.noise_aware_ordering_index =
					(u8)(BIOS_GET_FIELD(clocks_table_entry.param2,
					     NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_PARAM2_PROG_NOISE_AWARE_ORDERING_IDX));
				clk_domain_data.v3x_prog.b_force_noise_unaware_ordering =
					(u8)(BIOS_GET_FIELD(clocks_table_entry.param2,
					     NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_PARAM2_PROG_FORCE_NOISE_UNAWARE_ORDERING));
			} else {
				clk_domain_data.v3x_prog.noise_aware_ordering_index =
					CTRL_CLK_CLK_DOMAIN_3X_PROG_ORDERING_INDEX_INVALID;
				clk_domain_data.v3x_prog.b_force_noise_unaware_ordering = false;
			}

			clk_domain_data.v3x_prog.factory_delta.data.delta_khz = 0;
			clk_domain_data.v3x_prog.factory_delta.type = 0;

			clk_domain_data.v3x_prog.freq_delta_min_mhz =
				(u16)(BIOS_GET_FIELD(clocks_table_entry.param1,
				      NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_PARAM1_MASTER_FREQ_OC_DELTA_MIN_MHZ));

			clk_domain_data.v3x_prog.freq_delta_max_mhz =
				(u16)(BIOS_GET_FIELD(clocks_table_entry.param1,
				      NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_PARAM1_MASTER_FREQ_OC_DELTA_MAX_MHZ));
			break;
		}

		case  NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_FLAGS0_USAGE_SLAVE:
		{
			clk_domain_data.boardobj.type =
				CTRL_CLK_CLK_DOMAIN_TYPE_3X_SLAVE;
			clk_domain_data.v3x_prog.clk_prog_idx_first =
				(u8)(BIOS_GET_FIELD(clocks_table_entry.param0,
				     NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_PARAM0_PROG_CLK_PROG_IDX_FIRST));
			clk_domain_data.v3x_prog.clk_prog_idx_last =
				(u8)(BIOS_GET_FIELD(clocks_table_entry.param0,
				     NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_PARAM0_PROG_CLK_PROG_IDX_LAST));
			clk_domain_data.v3x_prog.noise_unaware_ordering_index =
				(u8)(BIOS_GET_FIELD(clocks_table_entry.param2,
				     NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_PARAM2_PROG_NOISE_UNAWARE_ORDERING_IDX));

			if (clk_domain_data.v3x.b_noise_aware_capable) {
				clk_domain_data.v3x_prog.noise_aware_ordering_index =
					(u8)(BIOS_GET_FIELD(clocks_table_entry.param2,
					     NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_PARAM2_PROG_NOISE_AWARE_ORDERING_IDX));
				clk_domain_data.v3x_prog.b_force_noise_unaware_ordering =
					(u8)(BIOS_GET_FIELD(clocks_table_entry.param2,
					     NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_PARAM2_PROG_FORCE_NOISE_UNAWARE_ORDERING));
			} else {
				clk_domain_data.v3x_prog.noise_aware_ordering_index =
					CTRL_CLK_CLK_DOMAIN_3X_PROG_ORDERING_INDEX_INVALID;
				clk_domain_data.v3x_prog.b_force_noise_unaware_ordering = false;
			}
			clk_domain_data.v3x_prog.factory_delta.data.delta_khz = 0;
			clk_domain_data.v3x_prog.factory_delta.type = 0;
			clk_domain_data.v3x_prog.freq_delta_min_mhz = 0;
			clk_domain_data.v3x_prog.freq_delta_max_mhz = 0;
			clk_domain_data.v3x_slave.master_idx =
				(u8)(BIOS_GET_FIELD(clocks_table_entry.param1,
				     NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_PARAM1_SLAVE_MASTER_DOMAIN));
			break;
		}

		default:
		{
			nvgpu_err(g,
				  "error reading clock domain entry %d", index);
			status = (u32) -EINVAL;
			goto done;
		}

		}
		pclkdomain_dev = construct_clk_domain(g,
				(void *)&clk_domain_data);
		if (pclkdomain_dev == NULL) {
			nvgpu_err(g,
				  "unable to construct clock domain boardobj for %d",
				  index);
			status = (u32) -EINVAL;
			goto done;
		}
		status = boardobjgrp_objinsert(&pclkdomainobjs->super.super,
				(struct boardobj *)(void *)pclkdomain_dev, index);
		if (status != 0) {
			nvgpu_err(g,
			"unable to insert clock domain boardobj for %d", index);
			status = (u32) -EINVAL;
			goto done;
		}
		clocks_tbl_entry_ptr += clocks_table_header.entry_size;
	}

done:
	nvgpu_log_info(g, " done status %x", status);
	return status;
}

static int devinit_get_clocks_table(struct gk20a *g,
	    struct clk_domains *pclkdomainobjs)
{
	int status = 0;
	u8 *clocks_table_ptr = NULL;
	struct vbios_clocks_table_1x_header clocks_table_header = { 0 };
	nvgpu_log_info(g, " ");

	clocks_table_ptr = (u8 *)nvgpu_bios_get_perf_table_ptrs(g,
			g->bios.clock_token, CLOCKS_TABLE);
	if (clocks_table_ptr == NULL) {
		status = -EINVAL;
		goto done;
	}
	nvgpu_memcpy((u8 *)&clocks_table_header, clocks_table_ptr,
			VBIOS_CLOCKS_TABLE_1X_HEADER_SIZE_07);
	if (clocks_table_header.version == 0x35U) {
		devinit_get_clocks_table_35(g, pclkdomainobjs, clocks_table_ptr);
	}
	else {
		devinit_get_clocks_table_1x(g, pclkdomainobjs, clocks_table_ptr);
	}
	done:
	return status;

}

static int clkdomainclkproglink_not_supported(struct gk20a *g,
					      struct clk_pmupstate *pclk,
					      struct clk_domain *pdomain)
{
	nvgpu_log_info(g, " ");
	return -EINVAL;
}

static int clkdomainvfsearch_stub(
	struct gk20a *g,
	struct clk_pmupstate *pclk,
	struct clk_domain *pdomain,
	u16 *clkmhz,
	u32 *voltuv,
	u8 rail)

{
	nvgpu_log_info(g, " ");
	return -EINVAL;
}

static int clkdomaingetfpoints_stub(
	struct gk20a *g,
	struct clk_pmupstate *pclk,
	struct clk_domain *pdomain,
	u32 *pfpointscount,
	u16 *pfreqpointsinmhz,
	u8 rail)
{
	nvgpu_log_info(g, " ");
	return -EINVAL;
}


static int clk_domain_construct_super(struct gk20a *g,
				      struct boardobj **ppboardobj,
				      u16 size, void *pargs)
{
	struct clk_domain *pdomain;
	struct clk_domain *ptmpdomain = (struct clk_domain *)pargs;
	int status = 0;

	status = boardobj_construct_super(g, ppboardobj,
		size, pargs);

	if (status != 0) {
		return -EINVAL;
	}

	pdomain = (struct clk_domain *)*ppboardobj;

	pdomain->super.pmudatainit =
			clk_domain_pmudatainit_super;

	pdomain->clkdomainclkproglink =
			clkdomainclkproglink_not_supported;

	pdomain->clkdomainclkvfsearch =
			clkdomainvfsearch_stub;

	pdomain->clkdomainclkgetfpoints =
			clkdomaingetfpoints_stub;

	pdomain->api_domain = ptmpdomain->api_domain;
	pdomain->domain = ptmpdomain->domain;
	pdomain->perf_domain_grp_idx =
		ptmpdomain->perf_domain_grp_idx;

	return status;
}

static int _clk_domain_pmudatainit_3x(struct gk20a *g,
				      struct boardobj *board_obj_ptr,
				      struct nv_pmu_boardobj *ppmudata)
{
	int status = 0;
	struct clk_domain_3x *pclk_domain_3x;
	struct nv_pmu_clk_clk_domain_3x_boardobj_set *pset;

	nvgpu_log_info(g, " ");

	status = clk_domain_pmudatainit_super(g, board_obj_ptr, ppmudata);
	if (status != 0) {
		return status;
	}

	pclk_domain_3x = (struct clk_domain_3x *)board_obj_ptr;

	pset = (struct nv_pmu_clk_clk_domain_3x_boardobj_set *)ppmudata;

	pset->b_noise_aware_capable = pclk_domain_3x->b_noise_aware_capable;

	return status;
}

static int clk_domain_construct_3x(struct gk20a *g,
				   struct boardobj **ppboardobj,
				   u16 size, void *pargs)
{
	struct boardobj *ptmpobj = (struct boardobj *)pargs;
	struct clk_domain_3x *pdomain;
	struct clk_domain_3x *ptmpdomain =
			(struct clk_domain_3x *)pargs;
	int status = 0;

	ptmpobj->type_mask = BIT(CTRL_CLK_CLK_DOMAIN_TYPE_3X);
	status = clk_domain_construct_super(g, ppboardobj,
					size, pargs);
	if (status != 0) {
		return -EINVAL;
	}

	pdomain = (struct clk_domain_3x *)*ppboardobj;

	pdomain->super.super.pmudatainit =
			_clk_domain_pmudatainit_3x;

	pdomain->b_noise_aware_capable = ptmpdomain->b_noise_aware_capable;

	return status;
}

static int clkdomainclkproglink_3x_prog(struct gk20a *g,
					struct clk_pmupstate *pclk,
					struct clk_domain *pdomain)
{
	int status = 0;
	struct clk_domain_3x_prog *p3xprog =
		(struct clk_domain_3x_prog *)pdomain;
	struct clk_prog *pprog = NULL;
	u8 i;

	nvgpu_log_info(g, " ");

	for (i = p3xprog->clk_prog_idx_first;
	     i <= p3xprog->clk_prog_idx_last;
	     i++) {
		pprog = CLK_CLK_PROG_GET(pclk, i);
		if (pprog == NULL) {
			status = -EINVAL;
		}
	}
	return status;
}

static int clkdomaingetslaveclk(struct gk20a *g,
				struct clk_pmupstate *pclk,
				struct clk_domain *pdomain,
				u16 *pclkmhz,
				u16 masterclkmhz)
{
	int status = 0;
	struct clk_prog *pprog = NULL;
	struct clk_prog_1x_master *pprog1xmaster = NULL;
	u8 slaveidx;
	struct clk_domain_3x_master *p3xmaster;

	nvgpu_log_info(g, " ");

	if (pclkmhz == NULL) {
		return -EINVAL;
	}

	if (masterclkmhz == 0U) {
		return -EINVAL;
	}

	slaveidx = BOARDOBJ_GET_IDX(pdomain);
	p3xmaster = (struct clk_domain_3x_master *)
			CLK_CLK_DOMAIN_GET(pclk,
			((struct clk_domain_3x_slave *)
				pdomain)->master_idx);
	pprog = CLK_CLK_PROG_GET(pclk, p3xmaster->super.clk_prog_idx_first);
	pprog1xmaster = (struct clk_prog_1x_master *)pprog;

	status = pprog1xmaster->getslaveclk(g, pclk, pprog1xmaster,
			slaveidx, pclkmhz, masterclkmhz);
	return status;
}

static int clkdomainvfsearch(struct gk20a *g,
				struct clk_pmupstate *pclk,
				struct clk_domain *pdomain,
				u16 *pclkmhz,
				u32 *pvoltuv,
				u8 rail)
{
	int status = 0;
	struct clk_domain_3x_master *p3xmaster  =
		(struct clk_domain_3x_master *)pdomain;
	struct clk_prog *pprog = NULL;
	struct clk_prog_1x_master *pprog1xmaster = NULL;
	u8 i;
	u8 *pslaveidx = NULL;
	u8 slaveidx;
	u16 clkmhz;
	u32 voltuv;
	u16 bestclkmhz;
	u32 bestvoltuv;

	nvgpu_log_info(g, " ");

	if ((pclkmhz == NULL) || (pvoltuv == NULL)) {
		return -EINVAL;
	}

	if ((*pclkmhz != 0U) && (*pvoltuv != 0U)) {
		return -EINVAL;
	}

	bestclkmhz = *pclkmhz;
	bestvoltuv = *pvoltuv;

	if (pdomain->super.implements(g, &pdomain->super,
			CTRL_CLK_CLK_DOMAIN_TYPE_3X_SLAVE)) {
		slaveidx = BOARDOBJ_GET_IDX(pdomain);
		pslaveidx = &slaveidx;
		p3xmaster = (struct clk_domain_3x_master *)
				CLK_CLK_DOMAIN_GET(pclk,
				((struct clk_domain_3x_slave *)
					pdomain)->master_idx);
	}
	/* Iterate over the set of CLK_PROGs pointed at by this domain.*/
	for (i = p3xmaster->super.clk_prog_idx_first;
	     i <= p3xmaster->super.clk_prog_idx_last;
	     i++) {
		clkmhz = *pclkmhz;
		voltuv = *pvoltuv;
		pprog = CLK_CLK_PROG_GET(pclk, i);

		/* MASTER CLK_DOMAINs must point to MASTER CLK_PROGs.*/
		if (!pprog->super.implements(g, &pprog->super,
				CTRL_CLK_CLK_PROG_TYPE_1X_MASTER)) {
			status = -EINVAL;
			goto done;
		}

		pprog1xmaster = (struct clk_prog_1x_master *)pprog;
		status = pprog1xmaster->vflookup(g, pclk, pprog1xmaster,
				pslaveidx, &clkmhz, &voltuv, rail);
		/* if look up has found the V or F value matching to other
		 exit */
		if (status == 0) {
			if (*pclkmhz == 0U) {
				bestclkmhz = clkmhz;
			} else {
				bestvoltuv = voltuv;
				break;
			}
		}
	}
	/* clk and volt sent as zero to print vf table */
	if ((*pclkmhz == 0U) && (*pvoltuv == 0U)) {
		status = 0;
		goto done;
	}
	/* atleast one search found a matching value? */
	if ((bestvoltuv != 0U) && (bestclkmhz != 0U)) {
		*pclkmhz = bestclkmhz;
		*pvoltuv = bestvoltuv;
		status = 0;
		goto done;
	}
done:
	nvgpu_log_info(g, "done status %x", status);
	return status;
}

static int clkdomaingetfpoints
(
	struct gk20a *g,
	struct clk_pmupstate *pclk,
	struct clk_domain *pdomain,
	u32 *pfpointscount,
	u16 *pfreqpointsinmhz,
	u8 rail
)
{
	int status = 0;
	struct clk_domain_3x_master *p3xmaster  =
		(struct clk_domain_3x_master *)pdomain;
	struct clk_prog *pprog = NULL;
	struct clk_prog_1x_master *pprog1xmaster = NULL;
	u32 fpointscount = 0;
	u32 remainingcount;
	u32 totalcount;
	u16 *freqpointsdata;
	u8 i;

	nvgpu_log_info(g, " ");

	if (pfpointscount == NULL) {
		return -EINVAL;
	}

	if ((pfreqpointsinmhz == NULL) && (*pfpointscount != 0U)) {
		return -EINVAL;
	}

	if (pdomain->super.implements(g, &pdomain->super,
			CTRL_CLK_CLK_DOMAIN_TYPE_3X_SLAVE)) {
		return -EINVAL;
	}

	freqpointsdata = pfreqpointsinmhz;
	totalcount = 0;
	fpointscount = *pfpointscount;
	remainingcount = fpointscount;
	/* Iterate over the set of CLK_PROGs pointed at by this domain.*/
	for (i = p3xmaster->super.clk_prog_idx_first;
	     i <= p3xmaster->super.clk_prog_idx_last;
	     i++) {
		pprog = CLK_CLK_PROG_GET(pclk, i);
		pprog1xmaster = (struct clk_prog_1x_master *)pprog;
		status = pprog1xmaster->getfpoints(g, pclk, pprog1xmaster,
				&fpointscount, &freqpointsdata, rail);
		if (status != 0) {
			*pfpointscount = 0;
			goto done;
		}
		totalcount += fpointscount;
		if (*pfpointscount != 0U) {
			remainingcount -= fpointscount;
			fpointscount = remainingcount;
		} else {
			fpointscount = 0;
		}

	}

	*pfpointscount = totalcount;
done:
	nvgpu_log_info(g, "done status %x", status);
	return status;
}

static int clk_domain_pmudatainit_35_prog(struct gk20a *g,
					   struct boardobj *board_obj_ptr,
					   struct nv_pmu_boardobj *ppmudata)
{
	int status = 0;
	struct clk_domain_35_prog *pclk_domain_35_prog;
	struct clk_domain_3x_prog *pclk_domain_3x_prog;
	struct nv_pmu_clk_clk_domain_35_prog_boardobj_set *pset;
	struct clk_domains *pdomains = &(g->clk_pmu->clk_domainobjs);

	nvgpu_log_info(g, " ");

	status = _clk_domain_pmudatainit_3x(g, board_obj_ptr, ppmudata);
	if (status != 0) {
		return status;
	}

	pclk_domain_35_prog = (struct clk_domain_35_prog *)(void*)board_obj_ptr;
	pclk_domain_3x_prog = &pclk_domain_35_prog->super;

	pset = (struct nv_pmu_clk_clk_domain_35_prog_boardobj_set *)
		(void*) ppmudata;

	pset->super.clk_prog_idx_first = pclk_domain_3x_prog->clk_prog_idx_first;
	pset->super.clk_prog_idx_last = pclk_domain_3x_prog->clk_prog_idx_last;
	pset->super.b_force_noise_unaware_ordering =
		pclk_domain_3x_prog->b_force_noise_unaware_ordering;
	pset->super.factory_delta = pclk_domain_3x_prog->factory_delta;
	pset->super.freq_delta_min_mhz = pclk_domain_3x_prog->freq_delta_min_mhz;
	pset->super.freq_delta_max_mhz = pclk_domain_3x_prog->freq_delta_max_mhz;
	nvgpu_memcpy((u8 *)&pset->super.deltas, (u8 *)&pdomains->deltas,
		(sizeof(struct ctrl_clk_clk_delta)));
	pset->pre_volt_ordering_index = pclk_domain_35_prog->pre_volt_ordering_index;
	pset->post_volt_ordering_index = pclk_domain_35_prog->post_volt_ordering_index;
	pset->clk_pos = pclk_domain_35_prog->clk_pos;
	pset->clk_vf_curve_count = pclk_domain_35_prog->clk_vf_curve_count;

	return status;
}

static int _clk_domain_pmudatainit_3x_prog(struct gk20a *g,
					   struct boardobj *board_obj_ptr,
					   struct nv_pmu_boardobj *ppmudata)
{
	int status = 0;
	struct clk_domain_3x_prog *pclk_domain_3x_prog;
	struct nv_pmu_clk_clk_domain_30_prog_boardobj_set *pset;
	struct clk_domains *pdomains = &(g->clk_pmu->clk_domainobjs);

	nvgpu_log_info(g, " ");

	status = _clk_domain_pmudatainit_3x(g, board_obj_ptr, ppmudata);
	if (status != 0) {
		return status;
	}

	pclk_domain_3x_prog = (struct clk_domain_3x_prog *)board_obj_ptr;

	pset = (struct nv_pmu_clk_clk_domain_30_prog_boardobj_set *)
		ppmudata;

	pset->super.clk_prog_idx_first = pclk_domain_3x_prog->clk_prog_idx_first;
	pset->super.clk_prog_idx_last = pclk_domain_3x_prog->clk_prog_idx_last;
	pset->noise_unaware_ordering_index =
		pclk_domain_3x_prog->noise_unaware_ordering_index;
	pset->noise_aware_ordering_index =
		pclk_domain_3x_prog->noise_aware_ordering_index;
	pset->super.b_force_noise_unaware_ordering =
		pclk_domain_3x_prog->b_force_noise_unaware_ordering;
	pset->super.factory_delta = pclk_domain_3x_prog->factory_delta;
	pset->super.freq_delta_min_mhz = pclk_domain_3x_prog->freq_delta_min_mhz;
	pset->super.freq_delta_max_mhz = pclk_domain_3x_prog->freq_delta_max_mhz;
	nvgpu_memcpy((u8 *)&pset->super.deltas, (u8 *)&pdomains->deltas,
		(sizeof(struct ctrl_clk_clk_delta)));

	return status;
}

static int clk_domain_construct_35_prog(struct gk20a *g,
					struct boardobj **ppboardobj,
					u16 size, void *pargs)
{
	struct boardobj *ptmpobj = (struct boardobj *)pargs;
	struct clk_domain_35_prog *pdomain;
	struct clk_domain_35_prog *ptmpdomain =
			(struct clk_domain_35_prog *)pargs;
	int status = 0;

	ptmpobj->type_mask |= BIT(CTRL_CLK_CLK_DOMAIN_TYPE_3X_PROG);
	status = clk_domain_construct_3x(g, ppboardobj, size, pargs);
	if (status != 0)
	{
		return (u32) -EINVAL;
	}

	pdomain = (struct clk_domain_35_prog *)(void*) *ppboardobj;

	pdomain->super.super.super.super.pmudatainit =
				clk_domain_pmudatainit_35_prog;

	pdomain->super.super.super.clkdomainclkproglink =
				clkdomainclkproglink_3x_prog;

	pdomain->super.super.super.clkdomainclkvfsearch =
				clkdomainvfsearch;

	pdomain->super.super.super.clkdomainclkgetfpoints =
				clkdomaingetfpoints;

	pdomain->super.clk_prog_idx_first = ptmpdomain->super.clk_prog_idx_first;
	pdomain->super.clk_prog_idx_last = ptmpdomain->super.clk_prog_idx_last;
	pdomain->super.noise_unaware_ordering_index =
		ptmpdomain->super.noise_unaware_ordering_index;
	pdomain->super.noise_aware_ordering_index =
		ptmpdomain->super.noise_aware_ordering_index;
	pdomain->super.b_force_noise_unaware_ordering =
		ptmpdomain->super.b_force_noise_unaware_ordering;
	pdomain->super.factory_delta = ptmpdomain->super.factory_delta;
	pdomain->super.freq_delta_min_mhz = ptmpdomain->super.freq_delta_min_mhz;
	pdomain->super.freq_delta_max_mhz = ptmpdomain->super.freq_delta_max_mhz;
	pdomain->pre_volt_ordering_index = ptmpdomain->pre_volt_ordering_index;
	pdomain->post_volt_ordering_index = ptmpdomain->post_volt_ordering_index;
	pdomain->clk_pos = ptmpdomain->clk_pos;
	pdomain->clk_vf_curve_count = ptmpdomain->clk_vf_curve_count;

	return status;
}

static int clk_domain_construct_3x_prog(struct gk20a *g,
					struct boardobj **ppboardobj,
					u16 size, void *pargs)
{
	struct boardobj *ptmpobj = (struct boardobj *)pargs;
	struct clk_domain_3x_prog *pdomain;
	struct clk_domain_3x_prog *ptmpdomain =
			(struct clk_domain_3x_prog *)pargs;
	int status = 0;

	ptmpobj->type_mask |= BIT(CTRL_CLK_CLK_DOMAIN_TYPE_3X_PROG);
	status = clk_domain_construct_3x(g, ppboardobj, size, pargs);
	if (status != 0) {
		return -EINVAL;
	}

	pdomain = (struct clk_domain_3x_prog *)*ppboardobj;

	pdomain->super.super.super.pmudatainit =
			_clk_domain_pmudatainit_3x_prog;

	pdomain->super.super.clkdomainclkproglink =
				clkdomainclkproglink_3x_prog;

	pdomain->super.super.clkdomainclkvfsearch =
				clkdomainvfsearch;

	pdomain->super.super.clkdomainclkgetfpoints =
				clkdomaingetfpoints;

	pdomain->clk_prog_idx_first = ptmpdomain->clk_prog_idx_first;
	pdomain->clk_prog_idx_last = ptmpdomain->clk_prog_idx_last;
	pdomain->noise_unaware_ordering_index =
		ptmpdomain->noise_unaware_ordering_index;
	pdomain->noise_aware_ordering_index =
		ptmpdomain->noise_aware_ordering_index;
	pdomain->b_force_noise_unaware_ordering =
		ptmpdomain->b_force_noise_unaware_ordering;
	pdomain->factory_delta = ptmpdomain->factory_delta;
	pdomain->freq_delta_min_mhz = ptmpdomain->freq_delta_min_mhz;
	pdomain->freq_delta_max_mhz = ptmpdomain->freq_delta_max_mhz;

	return status;
}

static int _clk_domain_pmudatainit_35_slave(struct gk20a *g,
					    struct boardobj *board_obj_ptr,
					    struct nv_pmu_boardobj *ppmudata)
{
	int status = 0;
	struct clk_domain_35_slave *pclk_domain_35_slave;
	struct nv_pmu_clk_clk_domain_35_slave_boardobj_set *pset;

	nvgpu_log_info(g, " ");

	status = clk_domain_pmudatainit_35_prog(g, board_obj_ptr, ppmudata);
	if (status != 0) {
		return status;
	}

	pclk_domain_35_slave = (struct clk_domain_35_slave *)(void*)board_obj_ptr;

	pset = (struct nv_pmu_clk_clk_domain_35_slave_boardobj_set *)
		(void*) ppmudata;

	pset->slave.master_idx = pclk_domain_35_slave->slave.master_idx;

	return status;
}

static int clk_domain_pmudatainit_3x_slave(struct gk20a *g,
					    struct boardobj *board_obj_ptr,
					    struct nv_pmu_boardobj *ppmudata)
{
	int status = 0;
	struct clk_domain_3x_slave *pclk_domain_3x_slave;
	struct nv_pmu_clk_clk_domain_3x_slave_boardobj_set *pset;

	nvgpu_log_info(g, " ");

	status = _clk_domain_pmudatainit_3x_prog(g, board_obj_ptr, ppmudata);
	if (status != 0) {
		return status;
	}

	pclk_domain_3x_slave = (struct clk_domain_3x_slave *)board_obj_ptr;

	pset = (struct nv_pmu_clk_clk_domain_3x_slave_boardobj_set *)
		ppmudata;

	pset->master_idx = pclk_domain_3x_slave->master_idx;

	return status;
}

static int clk_domain_construct_35_slave(struct gk20a *g,
					 struct boardobj **ppboardobj,
					 u16 size, void *pargs)
{
	struct boardobj *ptmpobj = (struct boardobj *)pargs;
	struct clk_domain_35_slave *pdomain;
	struct clk_domain_35_slave *ptmpdomain =
			(struct clk_domain_35_slave *)pargs;
	int status = 0;

	if (BOARDOBJ_GET_TYPE(pargs) != (u8) CTRL_CLK_CLK_DOMAIN_TYPE_35_SLAVE) {
		return (u32) -EINVAL;
	}

	ptmpobj->type_mask |= BIT(CTRL_CLK_CLK_DOMAIN_TYPE_35_SLAVE);
	status = clk_domain_construct_35_prog(g, ppboardobj, size, pargs);
	if (status != 0) {
		return (u32) -EINVAL;
	}

	pdomain = (struct clk_domain_35_slave *)(void*)*ppboardobj;

	pdomain->super.super.super.super.super.pmudatainit =
			_clk_domain_pmudatainit_35_slave;

	pdomain->slave.master_idx = ptmpdomain->slave.master_idx;

	pdomain->slave.clkdomainclkgetslaveclk =
				clkdomaingetslaveclk;

	return status;
}

static int clk_domain_construct_3x_slave(struct gk20a *g,
					 struct boardobj **ppboardobj,
					 u16 size, void *pargs)
{
	struct boardobj *ptmpobj = (struct boardobj *)pargs;
	struct clk_domain_3x_slave *pdomain;
	struct clk_domain_3x_slave *ptmpdomain =
			(struct clk_domain_3x_slave *)pargs;
	int status = 0;

	if (BOARDOBJ_GET_TYPE(pargs) != (u8) CTRL_CLK_CLK_DOMAIN_TYPE_3X_SLAVE) {
		return -EINVAL;
	}

	ptmpobj->type_mask |= BIT(CTRL_CLK_CLK_DOMAIN_TYPE_3X_SLAVE);
	status = clk_domain_construct_3x_prog(g, ppboardobj, size, pargs);
	if (status != 0) {
		return -EINVAL;
	}

	pdomain = (struct clk_domain_3x_slave *)*ppboardobj;

	pdomain->super.super.super.super.pmudatainit =
			clk_domain_pmudatainit_3x_slave;

	pdomain->master_idx = ptmpdomain->master_idx;

	pdomain->clkdomainclkgetslaveclk =
				clkdomaingetslaveclk;

	return status;
}

static int clkdomainclkproglink_3x_master(struct gk20a *g,
					  struct clk_pmupstate *pclk,
					  struct clk_domain *pdomain)
{
	int status = 0;
	struct clk_domain_3x_master *p3xmaster  =
		(struct clk_domain_3x_master *)pdomain;
	struct clk_prog *pprog = NULL;
	struct clk_prog_1x_master *pprog1xmaster = NULL;
	u16 freq_max_last_mhz = 0;
	u8 i;

	nvgpu_log_info(g, " ");

	status = clkdomainclkproglink_3x_prog(g, pclk, pdomain);
	if (status != 0) {
		goto done;
	}

	/* Iterate over the set of CLK_PROGs pointed at by this domain.*/
	for (i = p3xmaster->super.clk_prog_idx_first;
	     i <= p3xmaster->super.clk_prog_idx_last;
	     i++) {
		pprog = CLK_CLK_PROG_GET(pclk, i);

		/* MASTER CLK_DOMAINs must point to MASTER CLK_PROGs.*/
		if (!pprog->super.implements(g, &pprog->super,
				CTRL_CLK_CLK_PROG_TYPE_1X_MASTER)) {
			status = -EINVAL;
			goto done;
		}

		pprog1xmaster = (struct clk_prog_1x_master *)pprog;
		status = pprog1xmaster->vfflatten(g, pclk, pprog1xmaster,
			BOARDOBJ_GET_IDX(p3xmaster), &freq_max_last_mhz);
		if (status != 0) {
			goto done;
		}
	}
done:
	nvgpu_log_info(g, "done status %x", status);
	return status;
}

static int clk_domain_pmudatainit_35_master(struct gk20a *g,
					     struct boardobj *board_obj_ptr,
					     struct nv_pmu_boardobj *ppmudata)
{
	int status = 0;
	struct clk_domain_35_master *pclk_domain_35_master;
	struct nv_pmu_clk_clk_domain_35_master_boardobj_set *pset;

	nvgpu_log_info(g, " ");

	status = clk_domain_pmudatainit_35_prog(g, board_obj_ptr, ppmudata);
	if (status != 0) {
		return status;
	}

	pclk_domain_35_master = (struct clk_domain_35_master *)
		(void*) board_obj_ptr;

	pset = (struct nv_pmu_clk_clk_domain_35_master_boardobj_set *)
		(void*) ppmudata;

	pset->master.slave_idxs_mask = pclk_domain_35_master->master.slave_idxs_mask;

	return status;
}

static int _clk_domain_pmudatainit_3x_master(struct gk20a *g,
					     struct boardobj *board_obj_ptr,
					     struct nv_pmu_boardobj *ppmudata)
{
	int status = 0;
	struct clk_domain_3x_master *pclk_domain_3x_master;
	struct nv_pmu_clk_clk_domain_3x_master_boardobj_set *pset;

	nvgpu_log_info(g, " ");

	status = _clk_domain_pmudatainit_3x_prog(g, board_obj_ptr, ppmudata);
	if (status != 0) {
		return status;
	}

	pclk_domain_3x_master = (struct clk_domain_3x_master *)board_obj_ptr;

	pset = (struct nv_pmu_clk_clk_domain_3x_master_boardobj_set *)
		ppmudata;

	pset->slave_idxs_mask = pclk_domain_3x_master->slave_idxs_mask;

	return status;
}

static int clk_domain_construct_35_master(struct gk20a *g,
					  struct boardobj **ppboardobj,
					  u16 size, void *pargs)
{
	struct boardobj *ptmpobj = (struct boardobj *)pargs;
	struct clk_domain_35_master *pdomain;
	int status = 0;

	if (BOARDOBJ_GET_TYPE(pargs) != (u8) CTRL_CLK_CLK_DOMAIN_TYPE_35_MASTER) {
		return -EINVAL;
	}

	ptmpobj->type_mask |= BIT(CTRL_CLK_CLK_DOMAIN_TYPE_35_MASTER);
	status = clk_domain_construct_35_prog(g, ppboardobj, size, pargs);
	if (status != 0) {
		return (u32) -EINVAL;
	}

	pdomain = (struct clk_domain_35_master *)(void*) *ppboardobj;

	pdomain->super.super.super.super.super.pmudatainit =
			clk_domain_pmudatainit_35_master;
	pdomain->super.super.super.super.clkdomainclkproglink =
				clkdomainclkproglink_3x_master;

	pdomain->master.slave_idxs_mask = 0;

	return status;
}

static int clk_domain_construct_3x_master(struct gk20a *g,
					  struct boardobj **ppboardobj,
					  u16 size, void *pargs)
{
	struct boardobj *ptmpobj = (struct boardobj *)pargs;
	struct clk_domain_3x_master *pdomain;
	int status = 0;

	if (BOARDOBJ_GET_TYPE(pargs) != CTRL_CLK_CLK_DOMAIN_TYPE_3X_MASTER) {
		return -EINVAL;
	}

	ptmpobj->type_mask |= BIT(CTRL_CLK_CLK_DOMAIN_TYPE_3X_MASTER);
	status = clk_domain_construct_3x_prog(g, ppboardobj, size, pargs);
	if (status != 0) {
		return -EINVAL;
	}

	pdomain = (struct clk_domain_3x_master *)*ppboardobj;

	pdomain->super.super.super.super.pmudatainit =
			_clk_domain_pmudatainit_3x_master;
	pdomain->super.super.super.clkdomainclkproglink =
				clkdomainclkproglink_3x_master;

	pdomain->slave_idxs_mask = 0;

	return status;
}

static int clkdomainclkproglink_fixed(struct gk20a *g,
				      struct clk_pmupstate *pclk,
				      struct clk_domain *pdomain)
{
	nvgpu_log_info(g, " ");
	return 0;
}

static int _clk_domain_pmudatainit_3x_fixed(struct gk20a *g,
					    struct boardobj *board_obj_ptr,
					    struct nv_pmu_boardobj *ppmudata)
{
	int status = 0;
	struct clk_domain_3x_fixed *pclk_domain_3x_fixed;
	struct nv_pmu_clk_clk_domain_3x_fixed_boardobj_set *pset;

	nvgpu_log_info(g, " ");

	status = _clk_domain_pmudatainit_3x(g, board_obj_ptr, ppmudata);
	if (status != 0) {
		return status;
	}

	pclk_domain_3x_fixed = (struct clk_domain_3x_fixed *)board_obj_ptr;

	pset = (struct nv_pmu_clk_clk_domain_3x_fixed_boardobj_set *)
		ppmudata;

	pset->freq_mhz = pclk_domain_3x_fixed->freq_mhz;

	return status;
}

static int clk_domain_construct_3x_fixed(struct gk20a *g,
					 struct boardobj **ppboardobj,
					 u16 size, void *pargs)
{
	struct boardobj *ptmpobj = (struct boardobj *)pargs;
	struct clk_domain_3x_fixed *pdomain;
	struct clk_domain_3x_fixed *ptmpdomain =
			(struct clk_domain_3x_fixed *)pargs;
	int status = 0;

	if (BOARDOBJ_GET_TYPE(pargs) != CTRL_CLK_CLK_DOMAIN_TYPE_3X_FIXED) {
		return -EINVAL;
	}

	ptmpobj->type_mask |= BIT(CTRL_CLK_CLK_DOMAIN_TYPE_3X_FIXED);
	status = clk_domain_construct_3x(g, ppboardobj, size, pargs);
	if (status != 0) {
		return -EINVAL;
	}

	pdomain = (struct clk_domain_3x_fixed *)*ppboardobj;

	pdomain->super.super.super.pmudatainit =
			_clk_domain_pmudatainit_3x_fixed;

	pdomain->super.super.clkdomainclkproglink =
			clkdomainclkproglink_fixed;

	pdomain->freq_mhz = ptmpdomain->freq_mhz;

	return status;
}

static struct clk_domain *construct_clk_domain(struct gk20a *g, void *pargs)
{
	struct boardobj *board_obj_ptr = NULL;
	int status;

	nvgpu_log_info(g, " %d", BOARDOBJ_GET_TYPE(pargs));
	switch (BOARDOBJ_GET_TYPE(pargs)) {
	case CTRL_CLK_CLK_DOMAIN_TYPE_3X_FIXED:
		status = clk_domain_construct_3x_fixed(g, &board_obj_ptr,
			sizeof(struct clk_domain_3x_fixed), pargs);
		break;

	case CTRL_CLK_CLK_DOMAIN_TYPE_35_MASTER:
		status = clk_domain_construct_35_master(g, &board_obj_ptr,
			sizeof(struct clk_domain_35_master), pargs);
		break;


	case CTRL_CLK_CLK_DOMAIN_TYPE_3X_MASTER:
		status = clk_domain_construct_3x_master(g, &board_obj_ptr,
			sizeof(struct clk_domain_3x_master), pargs);
		break;

	case CTRL_CLK_CLK_DOMAIN_TYPE_35_SLAVE:
		status = clk_domain_construct_35_slave(g, &board_obj_ptr,
			sizeof(struct clk_domain_35_slave), pargs);
		break;

	case CTRL_CLK_CLK_DOMAIN_TYPE_3X_SLAVE:
		status = clk_domain_construct_3x_slave(g, &board_obj_ptr,
			sizeof(struct clk_domain_3x_slave), pargs);
		break;

	default:
		return NULL;
	}

	if (status != 0) {
		return NULL;
	}

	nvgpu_log_info(g, " Done");

	return (struct clk_domain *)board_obj_ptr;
}

static int clk_domain_pmudatainit_super(struct gk20a *g,
					struct boardobj *board_obj_ptr,
					struct nv_pmu_boardobj *ppmudata)
{
	int status = 0;
	struct clk_domain *pclk_domain;
	struct nv_pmu_clk_clk_domain_boardobj_set *pset;

	nvgpu_log_info(g, " ");

	status = boardobj_pmudatainit_super(g, board_obj_ptr, ppmudata);
	if (status != 0) {
		return status;
	}

	pclk_domain = (struct clk_domain *)board_obj_ptr;

	pset = (struct nv_pmu_clk_clk_domain_boardobj_set *)ppmudata;

	pset->domain = pclk_domain->domain;
	pset->api_domain = pclk_domain->api_domain;
	pset->perf_domain_grp_idx = pclk_domain->perf_domain_grp_idx;

	return status;
}

int clk_domain_clk_prog_link(struct gk20a *g, struct clk_pmupstate *pclk)
{
	int status = 0;
	struct clk_domain *pdomain;
	u8 i;

	/* Iterate over all CLK_DOMAINs and flatten their VF curves.*/
	BOARDOBJGRP_FOR_EACH(&(pclk->clk_domainobjs.super.super),
			struct clk_domain *, pdomain, i) {
		status = pdomain->clkdomainclkproglink(g, pclk, pdomain);
		if (status != 0) {
			nvgpu_err(g,
				  "error flattening VF for CLK DOMAIN - 0x%x",
				  pdomain->domain);
			goto done;
		}
	}

done:
	return status;
}
