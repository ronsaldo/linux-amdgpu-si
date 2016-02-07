/*
 * Copyright 2014 Advanced Micro Devices, Inc.
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
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */
#include <linux/firmware.h>
#include "drmP.h"
#include "amdgpu.h"
#include "amdgpu_ih.h"
#include "amdgpu_gfx.h"
#include "sid.h"
#include "si_reg.h"
#include "atom.h"
#include "amdgpu_ucode.h"
#include "clearstate_ci.h"

#include "bif/bif_4_0_d.h"
#include "bif/bif_4_0_enum.h"
#include "bif/bif_4_0_sh_mask.h"

#include "smu/smu_6_0_d.h"
#include "smu/smu_6_0_enum.h"
#include "smu/smu_6_0_sh_mask.h"

#include "dce/dce_6_0_d.h"
#include "dce/dce_6_0_enum.h"
#include "dce/dce_6_0_sh_mask.h"

#include "gmc/gmc_6_0_d.h"
#include "gmc/gmc_6_0_enum.h"
#include "gmc/gmc_6_0_sh_mask.h"

#include "gca/gfx_6_0_d.h"
#include "gca/gfx_6_0_enum.h"
#include "gca/gfx_6_0_sh_mask.h"

#include "oss/oss_1_0_d.h"
#include "oss/oss_1_0_enum.h"
#include "oss/oss_1_0_sh_mask.h"

void gfx_v6_0_rlc_stop(struct amdgpu_device *adev)
{
	/* TODO: Implement me */
}

/**
 * gfx_v7_0_select_se_sh - select which SE, SH to address
 *
 * @adev: amdgpu_device pointer
 * @se_num: shader engine to address
 * @sh_num: sh block to address
 *
 * Select which SE, SH combinations to address. Certain
 * registers are instanced per SE or SH.  0xffffffff means
 * broadcast to all SEs or SHs (SI).
 */
void gfx_v6_0_select_se_sh(struct amdgpu_device *adev, u32 se_num, u32 sh_num)
{
	u32 data = GRBM_GFX_INDEX__INSTANCE_BROADCAST_WRITES_MASK;

	if ((se_num == 0xffffffff) && (sh_num == 0xffffffff))
		data |= GRBM_GFX_INDEX__SH_BROADCAST_WRITES_MASK |
			GRBM_GFX_INDEX__SE_BROADCAST_WRITES_MASK;
	else if (se_num == 0xffffffff)
		data |= GRBM_GFX_INDEX__SE_BROADCAST_WRITES_MASK |
			(sh_num << GRBM_GFX_INDEX__SH_INDEX__SHIFT);
	else if (sh_num == 0xffffffff)
		data |= GRBM_GFX_INDEX__SH_BROADCAST_WRITES_MASK |
			(se_num << GRBM_GFX_INDEX__SE_INDEX__SHIFT);
	else
		data |= (sh_num << GRBM_GFX_INDEX__SH_INDEX__SHIFT) |
			(se_num << GRBM_GFX_INDEX__SE_INDEX__SHIFT);
	WREG32(mmGRBM_GFX_INDEX, data);
}

uint64_t gfx_v6_0_get_gpu_clock_counter(struct amdgpu_device *adev)
{
	/* TODO: Implement me*/
	return 0;
}

int gfx_v6_0_get_cu_info(struct amdgpu_device *adev, struct amdgpu_cu_info *cu_info)
{
	/* TODO: Implement me*/
	return 0;
}
