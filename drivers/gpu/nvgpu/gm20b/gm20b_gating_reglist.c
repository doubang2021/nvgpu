/*
 * Copyright (c) 2014-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * This file is autogenerated.  Do not edit.
 */

#ifndef __gm20b_gating_reglist_h__
#define __gm20b_gating_reglist_h__

#include <linux/types.h>
#include "gm20b_gating_reglist.h"

struct gating_desc {
	u32 addr;
	u32 prod;
	u32 disable;
};
/* slcg bus */
static const struct gating_desc gm20b_slcg_bus[] = {
	{.addr = 0x00001c04, .prod = 0x00000000, .disable = 0x000003fe},
};

/* slcg ce2 */
static const struct gating_desc gm20b_slcg_ce2[] = {
	{.addr = 0x00106f28, .prod = 0x00000000, .disable = 0x000007fe},
};

/* slcg chiplet */
static const struct gating_desc gm20b_slcg_chiplet[] = {
	{.addr = 0x0010c07c, .prod = 0x00000000, .disable = 0x00000007},
	{.addr = 0x0010e07c, .prod = 0x00000000, .disable = 0x00000007},
	{.addr = 0x0010d07c, .prod = 0x00000000, .disable = 0x00000007},
	{.addr = 0x0010e17c, .prod = 0x00000000, .disable = 0x00000007},
};

/* slcg fb */
static const struct gating_desc gm20b_slcg_fb[] = {
	{.addr = 0x00100d14, .prod = 0x00000000, .disable = 0xfffffffe},
	{.addr = 0x00100c9c, .prod = 0x00000000, .disable = 0x000001fe},
};

/* slcg fifo */
static const struct gating_desc gm20b_slcg_fifo[] = {
	{.addr = 0x000026ac, .prod = 0x00000100, .disable = 0x0001fffe},
};

/* slcg gr */
static const struct gating_desc gm20b_slcg_gr[] = {
	{.addr = 0x004041f4, .prod = 0x00000002, .disable = 0x03fffffe},
	{.addr = 0x0040917c, .prod = 0x00020008, .disable = 0x0003fffe},
	{.addr = 0x00409894, .prod = 0x00000040, .disable = 0x0003fffe},
	{.addr = 0x004078c4, .prod = 0x00000000, .disable = 0x000001fe},
	{.addr = 0x00406004, .prod = 0x00000000, .disable = 0x0001fffe},
	{.addr = 0x00405864, .prod = 0x00000000, .disable = 0x000001fe},
	{.addr = 0x00405910, .prod = 0xfffffff0, .disable = 0xfffffffe},
	{.addr = 0x00408044, .prod = 0x00000000, .disable = 0x000007fe},
	{.addr = 0x00407004, .prod = 0x00000000, .disable = 0x0000007e},
	{.addr = 0x0041a17c, .prod = 0x00020008, .disable = 0x0003fffe},
	{.addr = 0x0041a894, .prod = 0x00000040, .disable = 0x0003fffe},
	{.addr = 0x00418504, .prod = 0x00000000, .disable = 0x0007fffe},
	{.addr = 0x0041860c, .prod = 0x00000000, .disable = 0x000001fe},
	{.addr = 0x0041868c, .prod = 0x00000000, .disable = 0x0000001e},
	{.addr = 0x0041871c, .prod = 0x00000000, .disable = 0x0000003e},
	{.addr = 0x00418388, .prod = 0x00000000, .disable = 0x00000001},
	{.addr = 0x0041882c, .prod = 0x00000000, .disable = 0x0001fffe},
	{.addr = 0x00418bc0, .prod = 0x00000000, .disable = 0x000001fe},
	{.addr = 0x00418974, .prod = 0x00000000, .disable = 0x0001fffe},
	{.addr = 0x00418c74, .prod = 0xffffffc0, .disable = 0xfffffffe},
	{.addr = 0x00418cf4, .prod = 0xfffffffc, .disable = 0xfffffffe},
	{.addr = 0x00418d74, .prod = 0xffffffe0, .disable = 0xfffffffe},
	{.addr = 0x00418f10, .prod = 0xffffffe0, .disable = 0xfffffffe},
	{.addr = 0x00418e10, .prod = 0xfffffffe, .disable = 0xfffffffe},
	{.addr = 0x00419024, .prod = 0x000001fe, .disable = 0x000001fe},
	{.addr = 0x0041889c, .prod = 0x00000000, .disable = 0x000001fe},
	{.addr = 0x00419d64, .prod = 0x00000000, .disable = 0x000001ff},
	{.addr = 0x00419a44, .prod = 0x00000000, .disable = 0x0000000e},
	{.addr = 0x00419a4c, .prod = 0x00000000, .disable = 0x000001fe},
	{.addr = 0x00419a54, .prod = 0x00000000, .disable = 0x0000003e},
	{.addr = 0x00419a5c, .prod = 0x00000000, .disable = 0x0000000e},
	{.addr = 0x00419a64, .prod = 0x00000000, .disable = 0x000001fe},
	{.addr = 0x00419a6c, .prod = 0x00000000, .disable = 0x0000000e},
	{.addr = 0x00419a74, .prod = 0x00000000, .disable = 0x0000000e},
	{.addr = 0x00419a7c, .prod = 0x00000000, .disable = 0x0000003e},
	{.addr = 0x00419a84, .prod = 0x00000000, .disable = 0x0000000e},
	{.addr = 0x0041986c, .prod = 0x00000104, .disable = 0x00fffffe},
	{.addr = 0x00419cd8, .prod = 0x00000000, .disable = 0x001ffffe},
	{.addr = 0x00419ce0, .prod = 0x00000000, .disable = 0x001ffffe},
	{.addr = 0x00419c74, .prod = 0x0000001e, .disable = 0x0000001e},
	{.addr = 0x00419fd4, .prod = 0x00000000, .disable = 0x0003fffe},
	{.addr = 0x00419fdc, .prod = 0xffedff00, .disable = 0xfffffffe},
	{.addr = 0x00419fe4, .prod = 0x00001b00, .disable = 0x00001ffe},
	{.addr = 0x00419ff4, .prod = 0x00000000, .disable = 0x00003ffe},
	{.addr = 0x00419ffc, .prod = 0x00000000, .disable = 0x0001fffe},
	{.addr = 0x0041be2c, .prod = 0x04115fc0, .disable = 0xfffffffe},
	{.addr = 0x0041bfec, .prod = 0xfffffff0, .disable = 0xfffffffe},
	{.addr = 0x0041bed4, .prod = 0xfffffff6, .disable = 0xfffffffe},
	{.addr = 0x00408814, .prod = 0x00000000, .disable = 0x0001fffe},
	{.addr = 0x0040881c, .prod = 0x00000000, .disable = 0x0001fffe},
	{.addr = 0x00408a84, .prod = 0x00000000, .disable = 0x0001fffe},
	{.addr = 0x00408a8c, .prod = 0x00000000, .disable = 0x0001fffe},
	{.addr = 0x00408a94, .prod = 0x00000000, .disable = 0x0001fffe},
	{.addr = 0x00408a9c, .prod = 0x00000000, .disable = 0x0001fffe},
	{.addr = 0x00408aa4, .prod = 0x00000000, .disable = 0x0001fffe},
	{.addr = 0x00408aac, .prod = 0x00000000, .disable = 0x0001fffe},
	{.addr = 0x004089ac, .prod = 0x00000000, .disable = 0x0001fffe},
	{.addr = 0x00408a24, .prod = 0x00000000, .disable = 0x000001ff},
};

/* slcg ltc */
static const struct gating_desc gm20b_slcg_ltc[] = {
	{.addr = 0x0017e050, .prod = 0x00000000, .disable = 0xfffffffe},
	{.addr = 0x0017e35c, .prod = 0x00000000, .disable = 0xfffffffe},
};

/* slcg perf */
static const struct gating_desc gm20b_slcg_perf[] = {
	{.addr = 0x001be018, .prod = 0x000001ff, .disable = 0x00000000},
	{.addr = 0x001bc018, .prod = 0x000001ff, .disable = 0x00000000},
	{.addr = 0x001b8018, .prod = 0x000001ff, .disable = 0x00000000},
	{.addr = 0x001b4124, .prod = 0x00000001, .disable = 0x00000000},
};

/* slcg PriRing */
static const struct gating_desc gm20b_slcg_priring[] = {
	{.addr = 0x001200a8, .prod = 0x00000000, .disable = 0x00000001},
};

/* slcg pwr_csb */
static const struct gating_desc gm20b_slcg_pwr_csb[] = {
	{.addr = 0x0000017c, .prod = 0x00020008, .disable = 0x0003fffe},
	{.addr = 0x00000e74, .prod = 0x00000000, .disable = 0x0000000f},
	{.addr = 0x00000a74, .prod = 0x00000000, .disable = 0x00007ffe},
	{.addr = 0x000016b8, .prod = 0x00000000, .disable = 0x0000000f},
};

/* slcg pmu */
static const struct gating_desc gm20b_slcg_pmu[] = {
	{.addr = 0x0010a17c, .prod = 0x00020008, .disable = 0x0003fffe},
	{.addr = 0x0010aa74, .prod = 0x00000000, .disable = 0x00007ffe},
	{.addr = 0x0010ae74, .prod = 0x00000000, .disable = 0x0000000f},
};

/* therm gr */
static const struct gating_desc gm20b_slcg_therm[] = {
	{.addr = 0x000206b8, .prod = 0x00000000, .disable = 0x0000000f},
};

/* slcg Xbar */
static const struct gating_desc gm20b_slcg_xbar[] = {
	{.addr = 0x0013cbe4, .prod = 0x00000000, .disable = 0x1ffffffe},
	{.addr = 0x0013cc04, .prod = 0x00000000, .disable = 0x1ffffffe},
};

/* blcg bus */
static const struct gating_desc gm20b_blcg_bus[] = {
	{.addr = 0x00001c00, .prod = 0x00000042, .disable = 0x00000000},
};

/* blcg ctxsw prog */
static const struct gating_desc gm20b_blcg_ctxsw_prog[] = {
};

/* blcg fb */
static const struct gating_desc gm20b_blcg_fb[] = {
	{.addr = 0x00100d10, .prod = 0x0000c242, .disable = 0x00000000},
	{.addr = 0x00100d30, .prod = 0x0000c242, .disable = 0x00000000},
	{.addr = 0x00100d3c, .prod = 0x00000242, .disable = 0x00000000},
	{.addr = 0x00100d48, .prod = 0x0000c242, .disable = 0x00000000},
	{.addr = 0x00100d1c, .prod = 0x00000042, .disable = 0x00000000},
	{.addr = 0x00100c98, .prod = 0x00000242, .disable = 0x00000000},
};

/* blcg fifo */
static const struct gating_desc gm20b_blcg_fifo[] = {
	{.addr = 0x000026a4, .prod = 0x0000c242, .disable = 0x00000000},
};

/* blcg gr */
static const struct gating_desc gm20b_blcg_gr[] = {
	{.addr = 0x004041f0, .prod = 0x00004046, .disable = 0x00000000},
	{.addr = 0x00409890, .prod = 0x0000007f, .disable = 0x00000000},
	{.addr = 0x004098b0, .prod = 0x0000007f, .disable = 0x00000000},
	{.addr = 0x004078c0, .prod = 0x00000042, .disable = 0x00000000},
	{.addr = 0x00406000, .prod = 0x00004044, .disable = 0x00000000},
	{.addr = 0x00405860, .prod = 0x00004042, .disable = 0x00000000},
	{.addr = 0x0040590c, .prod = 0x00004044, .disable = 0x00000000},
	{.addr = 0x00408040, .prod = 0x00004044, .disable = 0x00000000},
	{.addr = 0x00407000, .prod = 0x00004041, .disable = 0x00000000},
	{.addr = 0x00405bf0, .prod = 0x00004044, .disable = 0x00000000},
	{.addr = 0x0041a890, .prod = 0x0000007f, .disable = 0x00000000},
	{.addr = 0x0041a8b0, .prod = 0x0000007f, .disable = 0x00000000},
	{.addr = 0x00418500, .prod = 0x00004044, .disable = 0x00000000},
	{.addr = 0x00418608, .prod = 0x00004042, .disable = 0x00000000},
	{.addr = 0x00418688, .prod = 0x00004042, .disable = 0x00000000},
	{.addr = 0x00418718, .prod = 0x00000042, .disable = 0x00000000},
	{.addr = 0x00418828, .prod = 0x00000044, .disable = 0x00000000},
	{.addr = 0x00418bbc, .prod = 0x00004042, .disable = 0x00000000},
	{.addr = 0x00418970, .prod = 0x00004042, .disable = 0x00000000},
	{.addr = 0x00418c70, .prod = 0x00004044, .disable = 0x00000000},
	{.addr = 0x00418cf0, .prod = 0x00004044, .disable = 0x00000000},
	{.addr = 0x00418d70, .prod = 0x00004044, .disable = 0x00000000},
	{.addr = 0x00418f0c, .prod = 0x00004044, .disable = 0x00000000},
	{.addr = 0x00418e0c, .prod = 0x00004044, .disable = 0x00000000},
	{.addr = 0x00419020, .prod = 0x00004042, .disable = 0x00000000},
	{.addr = 0x00419038, .prod = 0x00000042, .disable = 0x00000000},
	{.addr = 0x00418898, .prod = 0x00000042, .disable = 0x00000000},
	{.addr = 0x00419a40, .prod = 0x00000042, .disable = 0x00000000},
	{.addr = 0x00419a48, .prod = 0x00004042, .disable = 0x00000000},
	{.addr = 0x00419a50, .prod = 0x00004042, .disable = 0x00000000},
	{.addr = 0x00419a58, .prod = 0x00004042, .disable = 0x00000000},
	{.addr = 0x00419a60, .prod = 0x00004042, .disable = 0x00000000},
	{.addr = 0x00419a68, .prod = 0x00004042, .disable = 0x00000000},
	{.addr = 0x00419a70, .prod = 0x00004042, .disable = 0x00000000},
	{.addr = 0x00419a78, .prod = 0x00004042, .disable = 0x00000000},
	{.addr = 0x00419a80, .prod = 0x00004042, .disable = 0x00000000},
	{.addr = 0x00419868, .prod = 0x00000042, .disable = 0x00000000},
	{.addr = 0x00419cd4, .prod = 0x00000002, .disable = 0x00000000},
	{.addr = 0x00419cdc, .prod = 0x00000002, .disable = 0x00000000},
	{.addr = 0x00419c70, .prod = 0x00004044, .disable = 0x00000000},
	{.addr = 0x00419fd0, .prod = 0x00004044, .disable = 0x00000000},
	{.addr = 0x00419fd8, .prod = 0x00004046, .disable = 0x00000000},
	{.addr = 0x00419fe0, .prod = 0x00004044, .disable = 0x00000000},
	{.addr = 0x00419fe8, .prod = 0x00000042, .disable = 0x00000000},
	{.addr = 0x00419ff0, .prod = 0x00004045, .disable = 0x00000000},
	{.addr = 0x00419ff8, .prod = 0x00000002, .disable = 0x00000000},
	{.addr = 0x00419f90, .prod = 0x00000002, .disable = 0x00000000},
	{.addr = 0x0041be28, .prod = 0x00000042, .disable = 0x00000000},
	{.addr = 0x0041bfe8, .prod = 0x00004044, .disable = 0x00000000},
	{.addr = 0x0041bed0, .prod = 0x00004044, .disable = 0x00000000},
	{.addr = 0x00408810, .prod = 0x00004042, .disable = 0x00000000},
	{.addr = 0x00408818, .prod = 0x00004042, .disable = 0x00000000},
	{.addr = 0x00408a80, .prod = 0x00004042, .disable = 0x00000000},
	{.addr = 0x00408a88, .prod = 0x00004042, .disable = 0x00000000},
	{.addr = 0x00408a90, .prod = 0x00004042, .disable = 0x00000000},
	{.addr = 0x00408a98, .prod = 0x00004042, .disable = 0x00000000},
	{.addr = 0x00408aa0, .prod = 0x00004042, .disable = 0x00000000},
	{.addr = 0x00408aa8, .prod = 0x00004042, .disable = 0x00000000},
	{.addr = 0x004089a8, .prod = 0x00004042, .disable = 0x00000000},
	{.addr = 0x004089b0, .prod = 0x00000042, .disable = 0x00000000},
	{.addr = 0x004089b8, .prod = 0x00004042, .disable = 0x00000000},
};

/* blcg ltc */
static const struct gating_desc gm20b_blcg_ltc[] = {
	{.addr = 0x0017e030, .prod = 0x00000044, .disable = 0x00000000},
	{.addr = 0x0017e040, .prod = 0x00000044, .disable = 0x00000000},
	{.addr = 0x0017e3e0, .prod = 0x00000044, .disable = 0x00000000},
	{.addr = 0x0017e3c8, .prod = 0x00000044, .disable = 0x00000000},
};

/* blcg pwr_csb  */
static const struct gating_desc gm20b_blcg_pwr_csb[] = {
	{.addr = 0x00000a70, .prod = 0x00000045, .disable = 0x00000000},
};

/* blcg pmu */
static const struct gating_desc gm20b_blcg_pmu[] = {
	{.addr = 0x0010aa70, .prod = 0x00000045, .disable = 0x00000000},
};

/* blcg Xbar */
static const struct gating_desc gm20b_blcg_xbar[] = {
	{.addr = 0x0013cbe0, .prod = 0x00000042, .disable = 0x00000000},
	{.addr = 0x0013cc00, .prod = 0x00000042, .disable = 0x00000000},
};

/* pg gr */
static const struct gating_desc gm20b_pg_gr[] = {
};

/* inline functions */
void gm20b_slcg_bus_load_gating_prod(struct gk20a *g,
	bool prod)
{
	u32 i;
	u32 size = sizeof(gm20b_slcg_bus) / sizeof(struct gating_desc);
	for (i = 0; i < size; i++) {
		if (prod)
			gk20a_writel(g, gm20b_slcg_bus[i].addr,
				gm20b_slcg_bus[i].prod);
		else
			gk20a_writel(g, gm20b_slcg_bus[i].addr,
				 gm20b_slcg_bus[i].disable);
	}
}

void gm20b_slcg_ce2_load_gating_prod(struct gk20a *g,
	bool prod)
{
	u32 i;
	u32 size = sizeof(gm20b_slcg_ce2) / sizeof(struct gating_desc);
	for (i = 0; i < size; i++) {
		if (prod)
			gk20a_writel(g, gm20b_slcg_ce2[i].addr,
				gm20b_slcg_ce2[i].prod);
		else
			gk20a_writel(g, gm20b_slcg_ce2[i].addr,
				 gm20b_slcg_ce2[i].disable);
	}
}

void gm20b_slcg_chiplet_load_gating_prod(struct gk20a *g,
	bool prod)
{
	u32 i;
	u32 size = sizeof(gm20b_slcg_chiplet) / sizeof(struct gating_desc);
	for (i = 0; i < size; i++) {
		if (prod)
			gk20a_writel(g, gm20b_slcg_chiplet[i].addr,
				gm20b_slcg_chiplet[i].prod);
		else
			gk20a_writel(g, gm20b_slcg_chiplet[i].addr,
				 gm20b_slcg_chiplet[i].disable);
	}
}

void gm20b_slcg_ctxsw_firmware_load_gating_prod(struct gk20a *g,
	bool prod)
{
}

void gm20b_slcg_fb_load_gating_prod(struct gk20a *g,
	bool prod)
{
	u32 i;
	u32 size = sizeof(gm20b_slcg_fb) / sizeof(struct gating_desc);
	for (i = 0; i < size; i++) {
		if (prod)
			gk20a_writel(g, gm20b_slcg_fb[i].addr,
				gm20b_slcg_fb[i].prod);
		else
			gk20a_writel(g, gm20b_slcg_fb[i].addr,
				 gm20b_slcg_fb[i].disable);
	}
}

void gm20b_slcg_fifo_load_gating_prod(struct gk20a *g,
	bool prod)
{
	u32 i;
	u32 size = sizeof(gm20b_slcg_fifo) / sizeof(struct gating_desc);
	for (i = 0; i < size; i++) {
		if (prod)
			gk20a_writel(g, gm20b_slcg_fifo[i].addr,
				gm20b_slcg_fifo[i].prod);
		else
			gk20a_writel(g, gm20b_slcg_fifo[i].addr,
				 gm20b_slcg_fifo[i].disable);
	}
}

void gr_gm20b_slcg_gr_load_gating_prod(struct gk20a *g,
	bool prod)
{
	u32 i;
	u32 size = sizeof(gm20b_slcg_gr) / sizeof(struct gating_desc);
	for (i = 0; i < size; i++) {
		if (prod)
			gk20a_writel(g, gm20b_slcg_gr[i].addr,
				gm20b_slcg_gr[i].prod);
		else
			gk20a_writel(g, gm20b_slcg_gr[i].addr,
				 gm20b_slcg_gr[i].disable);
	}
}

void ltc_gm20b_slcg_ltc_load_gating_prod(struct gk20a *g,
	bool prod)
{
	u32 i;
	u32 size = sizeof(gm20b_slcg_ltc) / sizeof(struct gating_desc);
	for (i = 0; i < size; i++) {
		if (prod)
			gk20a_writel(g, gm20b_slcg_ltc[i].addr,
				gm20b_slcg_ltc[i].prod);
		else
			gk20a_writel(g, gm20b_slcg_ltc[i].addr,
				gm20b_slcg_ltc[i].disable);
	}
}

void gm20b_slcg_perf_load_gating_prod(struct gk20a *g,
	bool prod)
{
	u32 i;
	u32 size = sizeof(gm20b_slcg_perf) / sizeof(struct gating_desc);
	for (i = 0; i < size; i++) {
		if (prod)
			gk20a_writel(g, gm20b_slcg_perf[i].addr,
				gm20b_slcg_perf[i].prod);
		else
			gk20a_writel(g, gm20b_slcg_perf[i].addr,
				gm20b_slcg_perf[i].disable);
	}
}

void gm20b_slcg_priring_load_gating_prod(struct gk20a *g,
	bool prod)
{
	u32 i;
	u32 size = sizeof(gm20b_slcg_priring) / sizeof(struct gating_desc);
	for (i = 0; i < size; i++) {
		if (prod)
			gk20a_writel(g, gm20b_slcg_priring[i].addr,
				gm20b_slcg_priring[i].prod);
		else
			gk20a_writel(g, gm20b_slcg_priring[i].addr,
				gm20b_slcg_priring[i].disable);
	}
}

void gm20b_slcg_pwr_csb_load_gating_prod(struct gk20a *g,
	bool prod)
{
	u32 i;
	u32 size = sizeof(gm20b_slcg_pwr_csb) / sizeof(struct gating_desc);
	for (i = 0; i < size; i++) {
		if (prod)
			gk20a_writel(g, gm20b_slcg_pwr_csb[i].addr,
				gm20b_slcg_pwr_csb[i].prod);
		else
			gk20a_writel(g, gm20b_slcg_pwr_csb[i].addr,
				gm20b_slcg_pwr_csb[i].disable);
	}
}

void gm20b_slcg_pmu_load_gating_prod(struct gk20a *g,
	bool prod)
{
	u32 i;
	u32 size = sizeof(gm20b_slcg_pmu) / sizeof(struct gating_desc);
	for (i = 0; i < size; i++) {
		if (prod)
			gk20a_writel(g, gm20b_slcg_pmu[i].addr,
				gm20b_slcg_pmu[i].prod);
		else
			gk20a_writel(g, gm20b_slcg_pmu[i].addr,
				gm20b_slcg_pmu[i].disable);
	}
}

void gm20b_slcg_therm_load_gating_prod(struct gk20a *g,
	bool prod)
{
	u32 i;
	u32 size = sizeof(gm20b_slcg_therm) / sizeof(struct gating_desc);
	for (i = 0; i < size; i++) {
		if (prod)
			gk20a_writel(g, gm20b_slcg_therm[i].addr,
				gm20b_slcg_therm[i].prod);
		else
			gk20a_writel(g, gm20b_slcg_therm[i].addr,
				gm20b_slcg_therm[i].disable);
	}
}

void gm20b_slcg_xbar_load_gating_prod(struct gk20a *g,
	bool prod)
{
	u32 i;
	u32 size = sizeof(gm20b_slcg_xbar) / sizeof(struct gating_desc);
	for (i = 0; i < size; i++) {
		if (prod)
			gk20a_writel(g, gm20b_slcg_xbar[i].addr,
				gm20b_slcg_xbar[i].prod);
		else
			gk20a_writel(g, gm20b_slcg_xbar[i].addr,
				gm20b_slcg_xbar[i].disable);
	}
}

void gm20b_blcg_bus_load_gating_prod(struct gk20a *g,
	bool prod)
{
	u32 i;
	u32 size = sizeof(gm20b_blcg_bus) / sizeof(struct gating_desc);
	for (i = 0; i < size; i++) {
		if (prod)
			gk20a_writel(g, gm20b_blcg_bus[i].addr,
				gm20b_blcg_bus[i].prod);
		else
			gk20a_writel(g, gm20b_blcg_bus[i].addr,
				gm20b_blcg_bus[i].disable);
	}
}

void gm20b_blcg_ctxsw_firmware_load_gating_prod(struct gk20a *g,
	bool prod)
{
	u32 i;
	u32 size = sizeof(gm20b_blcg_ctxsw_prog) / sizeof(struct gating_desc);
	for (i = 0; i < size; i++) {
		if (prod)
			gk20a_writel(g, gm20b_blcg_ctxsw_prog[i].addr,
				gm20b_blcg_ctxsw_prog[i].prod);
		else
			gk20a_writel(g, gm20b_blcg_ctxsw_prog[i].addr,
				gm20b_blcg_ctxsw_prog[i].disable);
	}
}

void gm20b_blcg_fb_load_gating_prod(struct gk20a *g,
	bool prod)
{
	u32 i;
	u32 size = sizeof(gm20b_blcg_fb) / sizeof(struct gating_desc);
	for (i = 0; i < size; i++) {
		if (prod)
			gk20a_writel(g, gm20b_blcg_fb[i].addr,
				gm20b_blcg_fb[i].prod);
		else
			gk20a_writel(g, gm20b_blcg_fb[i].addr,
				gm20b_blcg_fb[i].disable);
	}
}

void gm20b_blcg_fifo_load_gating_prod(struct gk20a *g,
	bool prod)
{
	u32 i;
	u32 size = sizeof(gm20b_blcg_fifo) / sizeof(struct gating_desc);
	for (i = 0; i < size; i++) {
		if (prod)
			gk20a_writel(g, gm20b_blcg_fifo[i].addr,
				gm20b_blcg_fifo[i].prod);
		else
			gk20a_writel(g, gm20b_blcg_fifo[i].addr,
				gm20b_blcg_fifo[i].disable);
	}
}

void gm20b_blcg_gr_load_gating_prod(struct gk20a *g,
	bool prod)
{
	u32 i;
	u32 size = sizeof(gm20b_blcg_gr) / sizeof(struct gating_desc);
	for (i = 0; i < size; i++) {
		if (prod)
			gk20a_writel(g, gm20b_blcg_gr[i].addr,
				gm20b_blcg_gr[i].prod);
		else
			gk20a_writel(g, gm20b_blcg_gr[i].addr,
				gm20b_blcg_gr[i].disable);
	}
}

void gm20b_blcg_ltc_load_gating_prod(struct gk20a *g,
	bool prod)
{
	u32 i;
	u32 size = sizeof(gm20b_blcg_ltc) / sizeof(struct gating_desc);
	for (i = 0; i < size; i++) {
		if (prod)
			gk20a_writel(g, gm20b_blcg_ltc[i].addr,
				gm20b_blcg_ltc[i].prod);
		else
			gk20a_writel(g, gm20b_blcg_ltc[i].addr,
				gm20b_blcg_ltc[i].disable);
	}
}

void gm20b_blcg_pwr_csb_load_gating_prod(struct gk20a *g,
	bool prod)
{
	u32 i;
	u32 size = sizeof(gm20b_blcg_pwr_csb) / sizeof(struct gating_desc);
	for (i = 0; i < size; i++) {
		if (prod)
			gk20a_writel(g, gm20b_blcg_pwr_csb[i].addr,
				gm20b_blcg_pwr_csb[i].prod);
		else
			gk20a_writel(g, gm20b_blcg_pwr_csb[i].addr,
				gm20b_blcg_pwr_csb[i].disable);
	}
}

void gm20b_blcg_pmu_load_gating_prod(struct gk20a *g,
	bool prod)
{
	u32 i;
	u32 size = sizeof(gm20b_blcg_pmu) / sizeof(struct gating_desc);
	for (i = 0; i < size; i++) {
		if (prod)
			gk20a_writel(g, gm20b_blcg_pmu[i].addr,
				gm20b_blcg_pmu[i].prod);
		else
			gk20a_writel(g, gm20b_blcg_pmu[i].addr,
				gm20b_blcg_pmu[i].disable);
	}
}

void gm20b_blcg_xbar_load_gating_prod(struct gk20a *g,
	bool prod)
{
	u32 i;
	u32 size = sizeof(gm20b_blcg_xbar) / sizeof(struct gating_desc);
	for (i = 0; i < size; i++) {
		if (prod)
			gk20a_writel(g, gm20b_blcg_xbar[i].addr,
				gm20b_blcg_xbar[i].prod);
		else
			gk20a_writel(g, gm20b_blcg_xbar[i].addr,
				gm20b_blcg_xbar[i].disable);
	}
}

void gr_gm20b_pg_gr_load_gating_prod(struct gk20a *g,
	bool prod)
{
	u32 i;
	u32 size = sizeof(gm20b_pg_gr) / sizeof(struct gating_desc);
	for (i = 0; i < size; i++) {
		if (prod)
			gk20a_writel(g, gm20b_pg_gr[i].addr,
				gm20b_pg_gr[i].prod);
		else
			gk20a_writel(g, gm20b_pg_gr[i].addr,
				gm20b_pg_gr[i].disable);
	}
}

#endif /* __gm20b_gating_reglist_h__ */
