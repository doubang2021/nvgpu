/*
 * Copyright (c) 2019, NVIDIA CORPORATION.  All rights reserved.
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

#ifndef NVGPU_GOPS_FUSE_H
#define NVGPU_GOPS_FUSE_H

#include <nvgpu/types.h>

/**
 * @file
 *
 * Fuse HAL interface.
 */
struct gk20a;

/**
 * Fuse HAL operations.
 *
 * @see gpu_ops.
 */
struct gops_fuse {

	/**
	 * @brief Check and set PRIV security status.
	 *
	 * @param g [in]	The GPU driver struct.
	 *
	 * The HAL reads opt_priv_sec_en and gcplex_config fuses and:
	 * - If PRIV security feature is enabled, WPR is enabled and
	 * AUTO_FETCH is disabled in gcplex_config, then set
	 * NVGPU_SEC_PRIVSECURITY and NVGPU_SEC_SECUREGPCCS flags to
	 * true. Otherwise return error.
	 * - If PRIV security feature is not enabled, then set
	 * NVGPU_SEC_PRIVSECURITY and NVGPU_SEC_SECUREGPCCS flags to false.
	 *
	 * @return 0 in case of success, < 0 in case of failure.
	 */
	int (*check_priv_security)(struct gk20a *g);

	/**
	 * @brief Check ECC fuse.
	 *
	 * @param g [in]	The GPU driver struct.
	 *
	 * The HAL reads NV_FUSE_OPT_ECC_EN and checks if ECC is enabled or
	 * disabled for SM LRF/L1-DATA/L1-TAG/ICACHE,CBU and LTC.
	 *
	 * @return true if ECC is enabled, false otherwise.
	 */
	bool (*is_opt_ecc_enable)(struct gk20a *g);

	/**
	 * @brief Check feature override fuse.
	 *
	 * @param g [in]	The GPU driver struct.
	 *
	 * The HAL reads NV_FUSE_OPT_FEATURE_FUSES_OVERRIDE_DISABLE and checks
	 * if feature overriding is disabled or not.
	 *
	 * @return true if FEATURE_OVERRIDE is disabled, false otherwise.
	 */
	bool (*is_opt_feature_override_disable)(struct gk20a *g);

	/**
	 * @brief Read NV_FUSE_STATUS_OPT_FBIO fuse.
	 *
	 * @param g [in]	The GPU driver struct.
	 *
	 * The HAL reads NV_FUSE_STATUS_OPT_FBIO fuse value which provides FBIO
	 * floorsweeping status.
	 *
	 * @return fuse value read from NV_FUSE_STATUS_OPT_FBIO.
	 */
	u32 (*fuse_status_opt_fbio)(struct gk20a *g);

	/**
	 * @brief Read NV_FUSE_STATUS_OPT_FBP fuse.
	 *
	 * @param g [in]	The GPU driver struct.
	 *
	 * The HAL reads NV_FUSE_STATUS_OPT_FBP fuse value which provides Frame
	 * buffer partition floorsweeping status.
	 *
	 * @return fuse value read from NV_FUSE_STATUS_OPT_FBP.
	 */
	u32 (*fuse_status_opt_fbp)(struct gk20a *g);

	/**
	 * @brief Read NV_FUSE_STATUS_OPT_ROP_L2_FBP fuse.
	 *
	 * @param g [in]	The GPU driver struct.
	 * @param fbp [in]	Frame Buffer Partition index.
	 *
	 * The HAL reads NV_FUSE_STATUS_OPT_ROP_L2_FBP fuse value which provides
	 * ROP and L2 floorsweeping status in an FBP.
	 *
	 * @return fuse value read from NV_FUSE_STATUS_OPT_ROP_L2_FBP.
	 */
	u32 (*fuse_status_opt_rop_l2_fbp)(struct gk20a *g, u32 fbp);

	/**
	 * @brief Read NV_FUSE_STATUS_OPT_GPC fuse.
	 *
	 * @param g [in]	The GPU driver struct.
	 *
	 * The HAL reads NV_FUSE_STATUS_OPT_GPC fuse value which gives GPC
	 * floorsweeping status.
	 *
	 * @return fuse value read from NV_FUSE_STATUS_OPT_GPC.
	 */
	u32 (*fuse_status_opt_gpc)(struct gk20a *g);

	/**
	 * @brief Read NV_FUSE_STATUS_OPT_TPC_GPC fuse.
	 *
	 * @param g [in]	The GPU driver struct.
	 * @param gpc [in]	GPC index.
	 *
	 * The HAL reads NV_FUSE_STATUS_OPT_TPC_GPC fuse value which provides
	 * TPC floorsweeping status.
	 *
	 * @return fuse value read from NV_FUSE_STATUS_OPT_TPC_GPC.
	 */
	u32 (*fuse_status_opt_tpc_gpc)(struct gk20a *g, u32 gpc);

	/**
	 * @brief Write NV_FUSE_CTRL_OPT_TPC_GPC fuse.
	 *
	 * @param g [in]	The GPU driver struct.
	 * @param gpc [in]	GPC index.
	 * @param val [in]	Fuse value.
	 *
	 * The HAL programs NV_FUSE_CTRL_OPT_TPC_GPC fuse to floorsweep TPCs.
	 */
	void (*fuse_ctrl_opt_tpc_gpc)(struct gk20a *g, u32 gpc, u32 val);

	/**
	 * @brief Read NV_FUSE_OPT_PRIV_SEC_EN fuse.
	 *
	 * @param g [in]	The GPU driver struct.
	 *
	 * The HAL reads NV_FUSE_OPT_PRIV_SEC_EN fuse value which provides
	 *  Priv Security Feature enable status.
	 *
	 * @return fuse value read from NV_FUSE_OPT_PRIV_SEC_EN.
	 */
	u32 (*fuse_opt_priv_sec_en)(struct gk20a *g);

	/**
	 * @brief Read FUSE_GCPLEX_CONFIG_FUSE_0 fuse.
	 *
	 * @param g [in]	The GPU driver struct.
	 * @param val [out]	Fuse value read.
	 *
	 * The HAL reads FUSE_GCPLEX_CONFIG_FUSE_0 fuse value which provides
	 *  Priv Security Feature enable status.
	 *
	 * @return 0.
	 */
	int (*read_gcplex_config_fuse)(struct gk20a *g, u32 *val);

	/** @cond DOXYGEN_SHOULD_SKIP_THIS */

	u32 (*fuse_opt_sec_debug_en)(struct gk20a *g);

	u32 (*read_vin_cal_fuse_rev)(struct gk20a *g);
	int (*read_vin_cal_slope_intercept_fuse)(struct gk20a *g,
						u32 vin_id, u32 *slope,
						u32 *intercept);
	int (*read_vin_cal_gain_offset_fuse)(struct gk20a *g,
					u32 vin_id, s8 *gain,
					s8 *offset);
	/** @endcond DOXYGEN_SHOULD_SKIP_THIS */
};

#endif /* NVGPU_GOPS_FUSE_H */