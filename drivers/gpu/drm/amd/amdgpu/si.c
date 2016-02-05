#include <linux/firmware.h>
#include <linux/slab.h>
#include <linux/module.h>
#include "drmP.h"
#include "amdgpu.h"
#include "amdgpu_atombios.h"
#include "amdgpu_ih.h"
#include "amdgpu_uvd.h"
#include "amdgpu_vce.h"
#include "sid.h"
#include "si_reg.h"
#include "atom.h"
#include "amd_pcie.h"

#include "si.h"

#include "bif/bif_4_0_d.h"
#include "bif/bif_4_0_enum.h"
#include "bif/bif_4_0_sh_mask.h"

#include "smu/smu_6_0_d.h"
#include "smu/smu_6_0_enum.h"
#include "smu/smu_6_0_sh_mask.h"

#include "dce/dce_7_0_d.h"
#include "dce/dce_7_0_enum.h"
#include "dce/dce_7_0_sh_mask.h"

#include "gmc/gmc_6_0_d.h"
#include "gmc/gmc_6_0_enum.h"
#include "gmc/gmc_6_0_sh_mask.h"

#include "gca/gfx_6_0_d.h"
#include "gca/gfx_6_0_enum.h"
#include "gca/gfx_6_0_sh_mask.h"

#include "oss/oss_1_0_d.h"
#include "oss/oss_1_0_enum.h"
#include "oss/oss_1_0_sh_mask.h"

#include "gmc_v6_0.h"
#include "gfx_v6_0.h"

#include "amdgpu_amdkfd.h"
#include "amdgpu_powerplay.h"

/*
 * Indirect registers accessor
 */
u32 si_cg_rreg(struct amdgpu_device *adev, u32 reg)
{
	unsigned long flags;
	u32 r;

	spin_lock_irqsave(&adev->cg_idx_lock, flags);
	WREG32(mmSI_CG_IND_ADDR, ((reg) & 0xffff));
	r = RREG32(mmSI_CG_IND_DATA);
	spin_unlock_irqrestore(&adev->cg_idx_lock, flags);
	return r;
}

void si_cg_wreg(struct amdgpu_device *adev, u32 reg, u32 v)
{
	unsigned long flags;

	spin_lock_irqsave(&adev->cg_idx_lock, flags);
	WREG32(mmSI_CG_IND_ADDR, ((reg) & 0xffff));
	WREG32(mmSI_CG_IND_DATA, (v));
	spin_unlock_irqrestore(&adev->cg_idx_lock, flags);
}

u32 si_pif_phy0_rreg(struct amdgpu_device *adev, u32 reg)
{
	unsigned long flags;
	u32 r;

	spin_lock_irqsave(&adev->pif_idx_lock, flags);
	WREG32(mmSI_PIF_PHY0_INDEX, ((reg) & 0xffff));
	r = RREG32(mmSI_PIF_PHY0_DATA);
	spin_unlock_irqrestore(&adev->pif_idx_lock, flags);
	return r;
}

void si_pif_phy0_wreg(struct amdgpu_device *adev, u32 reg, u32 v)
{
	unsigned long flags;

	spin_lock_irqsave(&adev->pif_idx_lock, flags);
	WREG32(mmSI_PIF_PHY0_INDEX, ((reg) & 0xffff));
	WREG32(mmSI_PIF_PHY0_DATA, (v));
	spin_unlock_irqrestore(&adev->pif_idx_lock, flags);
}

u32 si_pif_phy1_rreg(struct amdgpu_device *adev, u32 reg)
{
	unsigned long flags;
	u32 r;

	spin_lock_irqsave(&adev->pif_idx_lock, flags);
	WREG32(mmSI_PIF_PHY1_INDEX, ((reg) & 0xffff));
	r = RREG32(mmSI_PIF_PHY1_DATA);
	spin_unlock_irqrestore(&adev->pif_idx_lock, flags);
	return r;
}

void si_pif_phy1_wreg(struct amdgpu_device *adev, u32 reg, u32 v)
{
	unsigned long flags;

	spin_lock_irqsave(&adev->pif_idx_lock, flags);
	WREG32(mmSI_PIF_PHY1_INDEX, ((reg) & 0xffff));
	WREG32(mmSI_PIF_PHY1_DATA, (v));
	spin_unlock_irqrestore(&adev->pif_idx_lock, flags);
}

u32 si_pcie_rreg(struct amdgpu_device *adev, u32 reg)
{
	unsigned long flags;
	u32 r;

	spin_lock_irqsave(&adev->pcie_idx_lock, flags);
	WREG32(mmSI_PORT_INDEX, ((reg) & 0xff));
	(void)RREG32(mmSI_PORT_INDEX);
	r = RREG32(mmSI_PORT_DATA);
	spin_unlock_irqrestore(&adev->pcie_idx_lock, flags);
	return r;
}

void si_pcie_wreg(struct amdgpu_device *adev, u32 reg, u32 v)
{
	unsigned long flags;

	spin_lock_irqsave(&adev->pcie_idx_lock, flags);
	WREG32(mmSI_PORT_INDEX, ((reg) & 0xff));
	(void)RREG32(mmSI_PORT_INDEX);
	WREG32(mmSI_PORT_DATA, (v));
	(void)RREG32(mmSI_PORT_DATA);
	spin_unlock_irqrestore(&adev->pcie_idx_lock, flags);
}

static u32 si_smc_rreg(struct amdgpu_device *adev, u32 reg)
{
	unsigned long flags;
	u32 r;

	spin_lock_irqsave(&adev->smc_idx_lock, flags);
	WREG32(mmSMC_IND_INDEX_0, reg);
	WREG32_P(mmSMC_IND_ACCESS_CNTL, 0, ~SMC_IND_ACCESS_CNTL__AUTO_INCREMENT_IND_0_MASK);
	r = RREG32(mmSMC_IND_DATA_0);
	spin_unlock_irqrestore(&adev->smc_idx_lock, flags);
	return r;
}

static void si_smc_wreg(struct amdgpu_device *adev, u32 reg, u32 v)
{
	unsigned long flags;

	spin_lock_irqsave(&adev->smc_idx_lock, flags);
	WREG32(mmSMC_IND_INDEX_0, reg);
	WREG32_P(mmSMC_IND_ACCESS_CNTL, 0, ~SMC_IND_ACCESS_CNTL__AUTO_INCREMENT_IND_0_MASK);
	WREG32(mmSMC_IND_DATA_0, v);
	spin_unlock_irqrestore(&adev->smc_idx_lock, flags);
}

u32 si_uvd_ctx_rreg(struct amdgpu_device *adev, u32 reg)
{
	unsigned long flags;
	u32 r;

	spin_lock_irqsave(&adev->uvd_ctx_idx_lock, flags);
	WREG32(mmSI_UVD_CTX_INDEX, ((reg) & 0x1ff));
	r = RREG32(mmSI_UVD_CTX_DATA);
	spin_unlock_irqrestore(&adev->uvd_ctx_idx_lock, flags);
	return r;
}

void si_uvd_ctx_wreg(struct amdgpu_device *adev, u32 reg, u32 v)
{
	unsigned long flags;

	spin_lock_irqsave(&adev->uvd_ctx_idx_lock, flags);
	WREG32(mmSI_UVD_CTX_INDEX, ((reg) & 0x1ff));
	WREG32(mmSI_UVD_CTX_DATA, (v));
	spin_unlock_irqrestore(&adev->uvd_ctx_idx_lock, flags);
}

static const u32 verde_rlc_save_restore_register_list[] =
{
	(0x8000 << 16) | (0x98f4 >> 2),
	0x00000000,
	(0x8040 << 16) | (0x98f4 >> 2),
	0x00000000,
	(0x8000 << 16) | (0xe80 >> 2),
	0x00000000,
	(0x8040 << 16) | (0xe80 >> 2),
	0x00000000,
	(0x8000 << 16) | (0x89bc >> 2),
	0x00000000,
	(0x8040 << 16) | (0x89bc >> 2),
	0x00000000,
	(0x8000 << 16) | (0x8c1c >> 2),
	0x00000000,
	(0x8040 << 16) | (0x8c1c >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x98f0 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0xe7c >> 2),
	0x00000000,
	(0x8000 << 16) | (0x9148 >> 2),
	0x00000000,
	(0x8040 << 16) | (0x9148 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x9150 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x897c >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x8d8c >> 2),
	0x00000000,
	(0x9c00 << 16) | (0xac54 >> 2),
	0X00000000,
	0x3,
	(0x9c00 << 16) | (0x98f8 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x9910 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x9914 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x9918 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x991c >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x9920 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x9924 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x9928 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x992c >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x9930 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x9934 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x9938 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x993c >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x9940 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x9944 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x9948 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x994c >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x9950 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x9954 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x9958 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x995c >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x9960 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x9964 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x9968 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x996c >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x9970 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x9974 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x9978 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x997c >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x9980 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x9984 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x9988 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x998c >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x8c00 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x8c14 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x8c04 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x8c08 >> 2),
	0x00000000,
	(0x8000 << 16) | (0x9b7c >> 2),
	0x00000000,
	(0x8040 << 16) | (0x9b7c >> 2),
	0x00000000,
	(0x8000 << 16) | (0xe84 >> 2),
	0x00000000,
	(0x8040 << 16) | (0xe84 >> 2),
	0x00000000,
	(0x8000 << 16) | (0x89c0 >> 2),
	0x00000000,
	(0x8040 << 16) | (0x89c0 >> 2),
	0x00000000,
	(0x8000 << 16) | (0x914c >> 2),
	0x00000000,
	(0x8040 << 16) | (0x914c >> 2),
	0x00000000,
	(0x8000 << 16) | (0x8c20 >> 2),
	0x00000000,
	(0x8040 << 16) | (0x8c20 >> 2),
	0x00000000,
	(0x8000 << 16) | (0x9354 >> 2),
	0x00000000,
	(0x8040 << 16) | (0x9354 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x9060 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x9364 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x9100 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x913c >> 2),
	0x00000000,
	(0x8000 << 16) | (0x90e0 >> 2),
	0x00000000,
	(0x8000 << 16) | (0x90e4 >> 2),
	0x00000000,
	(0x8000 << 16) | (0x90e8 >> 2),
	0x00000000,
	(0x8040 << 16) | (0x90e0 >> 2),
	0x00000000,
	(0x8040 << 16) | (0x90e4 >> 2),
	0x00000000,
	(0x8040 << 16) | (0x90e8 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x8bcc >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x8b24 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x88c4 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x8e50 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x8c0c >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x8e58 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x8e5c >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x9508 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x950c >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x9494 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0xac0c >> 2),
	0x00000000,
	(0x9c00 << 16) | (0xac10 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0xac14 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0xae00 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0xac08 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x88d4 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x88c8 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x88cc >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x89b0 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x8b10 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x8a14 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x9830 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x9834 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x9838 >> 2),
	0x00000000,
	(0x9c00 << 16) | (0x9a10 >> 2),
	0x00000000,
	(0x8000 << 16) | (0x9870 >> 2),
	0x00000000,
	(0x8000 << 16) | (0x9874 >> 2),
	0x00000000,
	(0x8001 << 16) | (0x9870 >> 2),
	0x00000000,
	(0x8001 << 16) | (0x9874 >> 2),
	0x00000000,
	(0x8040 << 16) | (0x9870 >> 2),
	0x00000000,
	(0x8040 << 16) | (0x9874 >> 2),
	0x00000000,
	(0x8041 << 16) | (0x9870 >> 2),
	0x00000000,
	(0x8041 << 16) | (0x9874 >> 2),
	0x00000000,
	0x00000000
};

static const u32 tahiti_golden_rlc_registers[] =
{
	0xc424, 0xffffffff, 0x00601005,
	0xc47c, 0xffffffff, 0x10104040,
	0xc488, 0xffffffff, 0x0100000a,
	0xc314, 0xffffffff, 0x00000800,
	0xc30c, 0xffffffff, 0x800000f4,
	0xf4a8, 0xffffffff, 0x00000000
};

static const u32 tahiti_golden_registers[] =
{
	0x9a10, 0x00010000, 0x00018208,
	0x9830, 0xffffffff, 0x00000000,
	0x9834, 0xf00fffff, 0x00000400,
	0x9838, 0x0002021c, 0x00020200,
	0xc78, 0x00000080, 0x00000000,
	0xd030, 0x000300c0, 0x00800040,
	0xd830, 0x000300c0, 0x00800040,
	0x5bb0, 0x000000f0, 0x00000070,
	0x5bc0, 0x00200000, 0x50100000,
	0x7030, 0x31000311, 0x00000011,
	0x277c, 0x00000003, 0x000007ff,
	0x240c, 0x000007ff, 0x00000000,
	0x8a14, 0xf000001f, 0x00000007,
	0x8b24, 0xffffffff, 0x00ffffff,
	0x8b10, 0x0000ff0f, 0x00000000,
	0x28a4c, 0x07ffffff, 0x4e000000,
	0x28350, 0x3f3f3fff, 0x2a00126a,
	0x30, 0x000000ff, 0x0040,
	0x34, 0x00000040, 0x00004040,
	0x9100, 0x07ffffff, 0x03000000,
	0x8e88, 0x01ff1f3f, 0x00000000,
	0x8e84, 0x01ff1f3f, 0x00000000,
	0x9060, 0x0000007f, 0x00000020,
	0x9508, 0x00010000, 0x00010000,
	0xac14, 0x00000200, 0x000002fb,
	0xac10, 0xffffffff, 0x0000543b,
	0xac0c, 0xffffffff, 0xa9210876,
	0x88d0, 0xffffffff, 0x000fff40,
	0x88d4, 0x0000001f, 0x00000010,
	0x1410, 0x20000000, 0x20fffed8,
	0x15c0, 0x000c0fc0, 0x000c0400
};

static const u32 tahiti_golden_registers2[] =
{
	0xc64, 0x00000001, 0x00000001
};

static const u32 pitcairn_golden_rlc_registers[] =
{
	0xc424, 0xffffffff, 0x00601004,
	0xc47c, 0xffffffff, 0x10102020,
	0xc488, 0xffffffff, 0x01000020,
	0xc314, 0xffffffff, 0x00000800,
	0xc30c, 0xffffffff, 0x800000a4
};

static const u32 pitcairn_golden_registers[] =
{
	0x9a10, 0x00010000, 0x00018208,
	0x9830, 0xffffffff, 0x00000000,
	0x9834, 0xf00fffff, 0x00000400,
	0x9838, 0x0002021c, 0x00020200,
	0xc78, 0x00000080, 0x00000000,
	0xd030, 0x000300c0, 0x00800040,
	0xd830, 0x000300c0, 0x00800040,
	0x5bb0, 0x000000f0, 0x00000070,
	0x5bc0, 0x00200000, 0x50100000,
	0x7030, 0x31000311, 0x00000011,
	0x2ae4, 0x00073ffe, 0x000022a2,
	0x240c, 0x000007ff, 0x00000000,
	0x8a14, 0xf000001f, 0x00000007,
	0x8b24, 0xffffffff, 0x00ffffff,
	0x8b10, 0x0000ff0f, 0x00000000,
	0x28a4c, 0x07ffffff, 0x4e000000,
	0x28350, 0x3f3f3fff, 0x2a00126a,
	0x30, 0x000000ff, 0x0040,
	0x34, 0x00000040, 0x00004040,
	0x9100, 0x07ffffff, 0x03000000,
	0x9060, 0x0000007f, 0x00000020,
	0x9508, 0x00010000, 0x00010000,
	0xac14, 0x000003ff, 0x000000f7,
	0xac10, 0xffffffff, 0x00000000,
	0xac0c, 0xffffffff, 0x32761054,
	0x88d4, 0x0000001f, 0x00000010,
	0x15c0, 0x000c0fc0, 0x000c0400
};

static const u32 verde_golden_rlc_registers[] =
{
	0xc424, 0xffffffff, 0x033f1005,
	0xc47c, 0xffffffff, 0x10808020,
	0xc488, 0xffffffff, 0x00800008,
	0xc314, 0xffffffff, 0x00001000,
	0xc30c, 0xffffffff, 0x80010014
};

static const u32 verde_golden_registers[] =
{
	0x9a10, 0x00010000, 0x00018208,
	0x9830, 0xffffffff, 0x00000000,
	0x9834, 0xf00fffff, 0x00000400,
	0x9838, 0x0002021c, 0x00020200,
	0xc78, 0x00000080, 0x00000000,
	0xd030, 0x000300c0, 0x00800040,
	0xd030, 0x000300c0, 0x00800040,
	0xd830, 0x000300c0, 0x00800040,
	0xd830, 0x000300c0, 0x00800040,
	0x5bb0, 0x000000f0, 0x00000070,
	0x5bc0, 0x00200000, 0x50100000,
	0x7030, 0x31000311, 0x00000011,
	0x2ae4, 0x00073ffe, 0x000022a2,
	0x2ae4, 0x00073ffe, 0x000022a2,
	0x2ae4, 0x00073ffe, 0x000022a2,
	0x240c, 0x000007ff, 0x00000000,
	0x240c, 0x000007ff, 0x00000000,
	0x240c, 0x000007ff, 0x00000000,
	0x8a14, 0xf000001f, 0x00000007,
	0x8a14, 0xf000001f, 0x00000007,
	0x8a14, 0xf000001f, 0x00000007,
	0x8b24, 0xffffffff, 0x00ffffff,
	0x8b10, 0x0000ff0f, 0x00000000,
	0x28a4c, 0x07ffffff, 0x4e000000,
	0x28350, 0x3f3f3fff, 0x0000124a,
	0x28350, 0x3f3f3fff, 0x0000124a,
	0x28350, 0x3f3f3fff, 0x0000124a,
	0x30, 0x000000ff, 0x0040,
	0x34, 0x00000040, 0x00004040,
	0x9100, 0x07ffffff, 0x03000000,
	0x9100, 0x07ffffff, 0x03000000,
	0x8e88, 0x01ff1f3f, 0x00000000,
	0x8e88, 0x01ff1f3f, 0x00000000,
	0x8e88, 0x01ff1f3f, 0x00000000,
	0x8e84, 0x01ff1f3f, 0x00000000,
	0x8e84, 0x01ff1f3f, 0x00000000,
	0x8e84, 0x01ff1f3f, 0x00000000,
	0x9060, 0x0000007f, 0x00000020,
	0x9508, 0x00010000, 0x00010000,
	0xac14, 0x000003ff, 0x00000003,
	0xac14, 0x000003ff, 0x00000003,
	0xac14, 0x000003ff, 0x00000003,
	0xac10, 0xffffffff, 0x00000000,
	0xac10, 0xffffffff, 0x00000000,
	0xac10, 0xffffffff, 0x00000000,
	0xac0c, 0xffffffff, 0x00001032,
	0xac0c, 0xffffffff, 0x00001032,
	0xac0c, 0xffffffff, 0x00001032,
	0x88d4, 0x0000001f, 0x00000010,
	0x88d4, 0x0000001f, 0x00000010,
	0x88d4, 0x0000001f, 0x00000010,
	0x15c0, 0x000c0fc0, 0x000c0400
};

static const u32 oland_golden_rlc_registers[] =
{
	0xc424, 0xffffffff, 0x00601005,
	0xc47c, 0xffffffff, 0x10104040,
	0xc488, 0xffffffff, 0x0100000a,
	0xc314, 0xffffffff, 0x00000800,
	0xc30c, 0xffffffff, 0x800000f4
};

static const u32 oland_golden_registers[] =
{
	0x9a10, 0x00010000, 0x00018208,
	0x9830, 0xffffffff, 0x00000000,
	0x9834, 0xf00fffff, 0x00000400,
	0x9838, 0x0002021c, 0x00020200,
	0xc78, 0x00000080, 0x00000000,
	0xd030, 0x000300c0, 0x00800040,
	0xd830, 0x000300c0, 0x00800040,
	0x5bb0, 0x000000f0, 0x00000070,
	0x5bc0, 0x00200000, 0x50100000,
	0x7030, 0x31000311, 0x00000011,
	0x2ae4, 0x00073ffe, 0x000022a2,
	0x240c, 0x000007ff, 0x00000000,
	0x8a14, 0xf000001f, 0x00000007,
	0x8b24, 0xffffffff, 0x00ffffff,
	0x8b10, 0x0000ff0f, 0x00000000,
	0x28a4c, 0x07ffffff, 0x4e000000,
	0x28350, 0x3f3f3fff, 0x00000082,
	0x30, 0x000000ff, 0x0040,
	0x34, 0x00000040, 0x00004040,
	0x9100, 0x07ffffff, 0x03000000,
	0x9060, 0x0000007f, 0x00000020,
	0x9508, 0x00010000, 0x00010000,
	0xac14, 0x000003ff, 0x000000f3,
	0xac10, 0xffffffff, 0x00000000,
	0xac0c, 0xffffffff, 0x00003210,
	0x88d4, 0x0000001f, 0x00000010,
	0x15c0, 0x000c0fc0, 0x000c0400
};

static const u32 hainan_golden_registers[] =
{
	0x9a10, 0x00010000, 0x00018208,
	0x9830, 0xffffffff, 0x00000000,
	0x9834, 0xf00fffff, 0x00000400,
	0x9838, 0x0002021c, 0x00020200,
	0xd0c0, 0xff000fff, 0x00000100,
	0xd030, 0x000300c0, 0x00800040,
	0xd8c0, 0xff000fff, 0x00000100,
	0xd830, 0x000300c0, 0x00800040,
	0x2ae4, 0x00073ffe, 0x000022a2,
	0x240c, 0x000007ff, 0x00000000,
	0x8a14, 0xf000001f, 0x00000007,
	0x8b24, 0xffffffff, 0x00ffffff,
	0x8b10, 0x0000ff0f, 0x00000000,
	0x28a4c, 0x07ffffff, 0x4e000000,
	0x28350, 0x3f3f3fff, 0x00000000,
	0x30, 0x000000ff, 0x0040,
	0x34, 0x00000040, 0x00004040,
	0x9100, 0x03e00000, 0x03600000,
	0x9060, 0x0000007f, 0x00000020,
	0x9508, 0x00010000, 0x00010000,
	0xac14, 0x000003ff, 0x000000f1,
	0xac10, 0xffffffff, 0x00000000,
	0xac0c, 0xffffffff, 0x00003210,
	0x88d4, 0x0000001f, 0x00000010,
	0x15c0, 0x000c0fc0, 0x000c0400
};

static const u32 hainan_golden_registers2[] =
{
	0x98f8, 0xffffffff, 0x02010001
};

static const u32 tahiti_mgcg_cgcg_init[] =
{
	0xc400, 0xffffffff, 0xfffffffc,
	0x802c, 0xffffffff, 0xe0000000,
	0x9a60, 0xffffffff, 0x00000100,
	0x92a4, 0xffffffff, 0x00000100,
	0xc164, 0xffffffff, 0x00000100,
	0x9774, 0xffffffff, 0x00000100,
	0x8984, 0xffffffff, 0x06000100,
	0x8a18, 0xffffffff, 0x00000100,
	0x92a0, 0xffffffff, 0x00000100,
	0xc380, 0xffffffff, 0x00000100,
	0x8b28, 0xffffffff, 0x00000100,
	0x9144, 0xffffffff, 0x00000100,
	0x8d88, 0xffffffff, 0x00000100,
	0x8d8c, 0xffffffff, 0x00000100,
	0x9030, 0xffffffff, 0x00000100,
	0x9034, 0xffffffff, 0x00000100,
	0x9038, 0xffffffff, 0x00000100,
	0x903c, 0xffffffff, 0x00000100,
	0xad80, 0xffffffff, 0x00000100,
	0xac54, 0xffffffff, 0x00000100,
	0x897c, 0xffffffff, 0x06000100,
	0x9868, 0xffffffff, 0x00000100,
	0x9510, 0xffffffff, 0x00000100,
	0xaf04, 0xffffffff, 0x00000100,
	0xae04, 0xffffffff, 0x00000100,
	0x949c, 0xffffffff, 0x00000100,
	0x802c, 0xffffffff, 0xe0000000,
	0x9160, 0xffffffff, 0x00010000,
	0x9164, 0xffffffff, 0x00030002,
	0x9168, 0xffffffff, 0x00040007,
	0x916c, 0xffffffff, 0x00060005,
	0x9170, 0xffffffff, 0x00090008,
	0x9174, 0xffffffff, 0x00020001,
	0x9178, 0xffffffff, 0x00040003,
	0x917c, 0xffffffff, 0x00000007,
	0x9180, 0xffffffff, 0x00060005,
	0x9184, 0xffffffff, 0x00090008,
	0x9188, 0xffffffff, 0x00030002,
	0x918c, 0xffffffff, 0x00050004,
	0x9190, 0xffffffff, 0x00000008,
	0x9194, 0xffffffff, 0x00070006,
	0x9198, 0xffffffff, 0x000a0009,
	0x919c, 0xffffffff, 0x00040003,
	0x91a0, 0xffffffff, 0x00060005,
	0x91a4, 0xffffffff, 0x00000009,
	0x91a8, 0xffffffff, 0x00080007,
	0x91ac, 0xffffffff, 0x000b000a,
	0x91b0, 0xffffffff, 0x00050004,
	0x91b4, 0xffffffff, 0x00070006,
	0x91b8, 0xffffffff, 0x0008000b,
	0x91bc, 0xffffffff, 0x000a0009,
	0x91c0, 0xffffffff, 0x000d000c,
	0x91c4, 0xffffffff, 0x00060005,
	0x91c8, 0xffffffff, 0x00080007,
	0x91cc, 0xffffffff, 0x0000000b,
	0x91d0, 0xffffffff, 0x000a0009,
	0x91d4, 0xffffffff, 0x000d000c,
	0x91d8, 0xffffffff, 0x00070006,
	0x91dc, 0xffffffff, 0x00090008,
	0x91e0, 0xffffffff, 0x0000000c,
	0x91e4, 0xffffffff, 0x000b000a,
	0x91e8, 0xffffffff, 0x000e000d,
	0x91ec, 0xffffffff, 0x00080007,
	0x91f0, 0xffffffff, 0x000a0009,
	0x91f4, 0xffffffff, 0x0000000d,
	0x91f8, 0xffffffff, 0x000c000b,
	0x91fc, 0xffffffff, 0x000f000e,
	0x9200, 0xffffffff, 0x00090008,
	0x9204, 0xffffffff, 0x000b000a,
	0x9208, 0xffffffff, 0x000c000f,
	0x920c, 0xffffffff, 0x000e000d,
	0x9210, 0xffffffff, 0x00110010,
	0x9214, 0xffffffff, 0x000a0009,
	0x9218, 0xffffffff, 0x000c000b,
	0x921c, 0xffffffff, 0x0000000f,
	0x9220, 0xffffffff, 0x000e000d,
	0x9224, 0xffffffff, 0x00110010,
	0x9228, 0xffffffff, 0x000b000a,
	0x922c, 0xffffffff, 0x000d000c,
	0x9230, 0xffffffff, 0x00000010,
	0x9234, 0xffffffff, 0x000f000e,
	0x9238, 0xffffffff, 0x00120011,
	0x923c, 0xffffffff, 0x000c000b,
	0x9240, 0xffffffff, 0x000e000d,
	0x9244, 0xffffffff, 0x00000011,
	0x9248, 0xffffffff, 0x0010000f,
	0x924c, 0xffffffff, 0x00130012,
	0x9250, 0xffffffff, 0x000d000c,
	0x9254, 0xffffffff, 0x000f000e,
	0x9258, 0xffffffff, 0x00100013,
	0x925c, 0xffffffff, 0x00120011,
	0x9260, 0xffffffff, 0x00150014,
	0x9264, 0xffffffff, 0x000e000d,
	0x9268, 0xffffffff, 0x0010000f,
	0x926c, 0xffffffff, 0x00000013,
	0x9270, 0xffffffff, 0x00120011,
	0x9274, 0xffffffff, 0x00150014,
	0x9278, 0xffffffff, 0x000f000e,
	0x927c, 0xffffffff, 0x00110010,
	0x9280, 0xffffffff, 0x00000014,
	0x9284, 0xffffffff, 0x00130012,
	0x9288, 0xffffffff, 0x00160015,
	0x928c, 0xffffffff, 0x0010000f,
	0x9290, 0xffffffff, 0x00120011,
	0x9294, 0xffffffff, 0x00000015,
	0x9298, 0xffffffff, 0x00140013,
	0x929c, 0xffffffff, 0x00170016,
	0x9150, 0xffffffff, 0x96940200,
	0x8708, 0xffffffff, 0x00900100,
	0xc478, 0xffffffff, 0x00000080,
	0xc404, 0xffffffff, 0x0020003f,
	0x30, 0xffffffff, 0x0000001c,
	0x34, 0x000f0000, 0x000f0000,
	0x160c, 0xffffffff, 0x00000100,
	0x1024, 0xffffffff, 0x00000100,
	0x102c, 0x00000101, 0x00000000,
	0x20a8, 0xffffffff, 0x00000104,
	0x264c, 0x000c0000, 0x000c0000,
	0x2648, 0x000c0000, 0x000c0000,
	0x55e4, 0xff000fff, 0x00000100,
	0x55e8, 0x00000001, 0x00000001,
	0x2f50, 0x00000001, 0x00000001,
	0x30cc, 0xc0000fff, 0x00000104,
	0xc1e4, 0x00000001, 0x00000001,
	0xd0c0, 0xfffffff0, 0x00000100,
	0xd8c0, 0xfffffff0, 0x00000100
};

static const u32 pitcairn_mgcg_cgcg_init[] =
{
	0xc400, 0xffffffff, 0xfffffffc,
	0x802c, 0xffffffff, 0xe0000000,
	0x9a60, 0xffffffff, 0x00000100,
	0x92a4, 0xffffffff, 0x00000100,
	0xc164, 0xffffffff, 0x00000100,
	0x9774, 0xffffffff, 0x00000100,
	0x8984, 0xffffffff, 0x06000100,
	0x8a18, 0xffffffff, 0x00000100,
	0x92a0, 0xffffffff, 0x00000100,
	0xc380, 0xffffffff, 0x00000100,
	0x8b28, 0xffffffff, 0x00000100,
	0x9144, 0xffffffff, 0x00000100,
	0x8d88, 0xffffffff, 0x00000100,
	0x8d8c, 0xffffffff, 0x00000100,
	0x9030, 0xffffffff, 0x00000100,
	0x9034, 0xffffffff, 0x00000100,
	0x9038, 0xffffffff, 0x00000100,
	0x903c, 0xffffffff, 0x00000100,
	0xad80, 0xffffffff, 0x00000100,
	0xac54, 0xffffffff, 0x00000100,
	0x897c, 0xffffffff, 0x06000100,
	0x9868, 0xffffffff, 0x00000100,
	0x9510, 0xffffffff, 0x00000100,
	0xaf04, 0xffffffff, 0x00000100,
	0xae04, 0xffffffff, 0x00000100,
	0x949c, 0xffffffff, 0x00000100,
	0x802c, 0xffffffff, 0xe0000000,
	0x9160, 0xffffffff, 0x00010000,
	0x9164, 0xffffffff, 0x00030002,
	0x9168, 0xffffffff, 0x00040007,
	0x916c, 0xffffffff, 0x00060005,
	0x9170, 0xffffffff, 0x00090008,
	0x9174, 0xffffffff, 0x00020001,
	0x9178, 0xffffffff, 0x00040003,
	0x917c, 0xffffffff, 0x00000007,
	0x9180, 0xffffffff, 0x00060005,
	0x9184, 0xffffffff, 0x00090008,
	0x9188, 0xffffffff, 0x00030002,
	0x918c, 0xffffffff, 0x00050004,
	0x9190, 0xffffffff, 0x00000008,
	0x9194, 0xffffffff, 0x00070006,
	0x9198, 0xffffffff, 0x000a0009,
	0x919c, 0xffffffff, 0x00040003,
	0x91a0, 0xffffffff, 0x00060005,
	0x91a4, 0xffffffff, 0x00000009,
	0x91a8, 0xffffffff, 0x00080007,
	0x91ac, 0xffffffff, 0x000b000a,
	0x91b0, 0xffffffff, 0x00050004,
	0x91b4, 0xffffffff, 0x00070006,
	0x91b8, 0xffffffff, 0x0008000b,
	0x91bc, 0xffffffff, 0x000a0009,
	0x91c0, 0xffffffff, 0x000d000c,
	0x9200, 0xffffffff, 0x00090008,
	0x9204, 0xffffffff, 0x000b000a,
	0x9208, 0xffffffff, 0x000c000f,
	0x920c, 0xffffffff, 0x000e000d,
	0x9210, 0xffffffff, 0x00110010,
	0x9214, 0xffffffff, 0x000a0009,
	0x9218, 0xffffffff, 0x000c000b,
	0x921c, 0xffffffff, 0x0000000f,
	0x9220, 0xffffffff, 0x000e000d,
	0x9224, 0xffffffff, 0x00110010,
	0x9228, 0xffffffff, 0x000b000a,
	0x922c, 0xffffffff, 0x000d000c,
	0x9230, 0xffffffff, 0x00000010,
	0x9234, 0xffffffff, 0x000f000e,
	0x9238, 0xffffffff, 0x00120011,
	0x923c, 0xffffffff, 0x000c000b,
	0x9240, 0xffffffff, 0x000e000d,
	0x9244, 0xffffffff, 0x00000011,
	0x9248, 0xffffffff, 0x0010000f,
	0x924c, 0xffffffff, 0x00130012,
	0x9250, 0xffffffff, 0x000d000c,
	0x9254, 0xffffffff, 0x000f000e,
	0x9258, 0xffffffff, 0x00100013,
	0x925c, 0xffffffff, 0x00120011,
	0x9260, 0xffffffff, 0x00150014,
	0x9150, 0xffffffff, 0x96940200,
	0x8708, 0xffffffff, 0x00900100,
	0xc478, 0xffffffff, 0x00000080,
	0xc404, 0xffffffff, 0x0020003f,
	0x30, 0xffffffff, 0x0000001c,
	0x34, 0x000f0000, 0x000f0000,
	0x160c, 0xffffffff, 0x00000100,
	0x1024, 0xffffffff, 0x00000100,
	0x102c, 0x00000101, 0x00000000,
	0x20a8, 0xffffffff, 0x00000104,
	0x55e4, 0xff000fff, 0x00000100,
	0x55e8, 0x00000001, 0x00000001,
	0x2f50, 0x00000001, 0x00000001,
	0x30cc, 0xc0000fff, 0x00000104,
	0xc1e4, 0x00000001, 0x00000001,
	0xd0c0, 0xfffffff0, 0x00000100,
	0xd8c0, 0xfffffff0, 0x00000100
};

static const u32 verde_mgcg_cgcg_init[] =
{
	0xc400, 0xffffffff, 0xfffffffc,
	0x802c, 0xffffffff, 0xe0000000,
	0x9a60, 0xffffffff, 0x00000100,
	0x92a4, 0xffffffff, 0x00000100,
	0xc164, 0xffffffff, 0x00000100,
	0x9774, 0xffffffff, 0x00000100,
	0x8984, 0xffffffff, 0x06000100,
	0x8a18, 0xffffffff, 0x00000100,
	0x92a0, 0xffffffff, 0x00000100,
	0xc380, 0xffffffff, 0x00000100,
	0x8b28, 0xffffffff, 0x00000100,
	0x9144, 0xffffffff, 0x00000100,
	0x8d88, 0xffffffff, 0x00000100,
	0x8d8c, 0xffffffff, 0x00000100,
	0x9030, 0xffffffff, 0x00000100,
	0x9034, 0xffffffff, 0x00000100,
	0x9038, 0xffffffff, 0x00000100,
	0x903c, 0xffffffff, 0x00000100,
	0xad80, 0xffffffff, 0x00000100,
	0xac54, 0xffffffff, 0x00000100,
	0x897c, 0xffffffff, 0x06000100,
	0x9868, 0xffffffff, 0x00000100,
	0x9510, 0xffffffff, 0x00000100,
	0xaf04, 0xffffffff, 0x00000100,
	0xae04, 0xffffffff, 0x00000100,
	0x949c, 0xffffffff, 0x00000100,
	0x802c, 0xffffffff, 0xe0000000,
	0x9160, 0xffffffff, 0x00010000,
	0x9164, 0xffffffff, 0x00030002,
	0x9168, 0xffffffff, 0x00040007,
	0x916c, 0xffffffff, 0x00060005,
	0x9170, 0xffffffff, 0x00090008,
	0x9174, 0xffffffff, 0x00020001,
	0x9178, 0xffffffff, 0x00040003,
	0x917c, 0xffffffff, 0x00000007,
	0x9180, 0xffffffff, 0x00060005,
	0x9184, 0xffffffff, 0x00090008,
	0x9188, 0xffffffff, 0x00030002,
	0x918c, 0xffffffff, 0x00050004,
	0x9190, 0xffffffff, 0x00000008,
	0x9194, 0xffffffff, 0x00070006,
	0x9198, 0xffffffff, 0x000a0009,
	0x919c, 0xffffffff, 0x00040003,
	0x91a0, 0xffffffff, 0x00060005,
	0x91a4, 0xffffffff, 0x00000009,
	0x91a8, 0xffffffff, 0x00080007,
	0x91ac, 0xffffffff, 0x000b000a,
	0x91b0, 0xffffffff, 0x00050004,
	0x91b4, 0xffffffff, 0x00070006,
	0x91b8, 0xffffffff, 0x0008000b,
	0x91bc, 0xffffffff, 0x000a0009,
	0x91c0, 0xffffffff, 0x000d000c,
	0x9200, 0xffffffff, 0x00090008,
	0x9204, 0xffffffff, 0x000b000a,
	0x9208, 0xffffffff, 0x000c000f,
	0x920c, 0xffffffff, 0x000e000d,
	0x9210, 0xffffffff, 0x00110010,
	0x9214, 0xffffffff, 0x000a0009,
	0x9218, 0xffffffff, 0x000c000b,
	0x921c, 0xffffffff, 0x0000000f,
	0x9220, 0xffffffff, 0x000e000d,
	0x9224, 0xffffffff, 0x00110010,
	0x9228, 0xffffffff, 0x000b000a,
	0x922c, 0xffffffff, 0x000d000c,
	0x9230, 0xffffffff, 0x00000010,
	0x9234, 0xffffffff, 0x000f000e,
	0x9238, 0xffffffff, 0x00120011,
	0x923c, 0xffffffff, 0x000c000b,
	0x9240, 0xffffffff, 0x000e000d,
	0x9244, 0xffffffff, 0x00000011,
	0x9248, 0xffffffff, 0x0010000f,
	0x924c, 0xffffffff, 0x00130012,
	0x9250, 0xffffffff, 0x000d000c,
	0x9254, 0xffffffff, 0x000f000e,
	0x9258, 0xffffffff, 0x00100013,
	0x925c, 0xffffffff, 0x00120011,
	0x9260, 0xffffffff, 0x00150014,
	0x9150, 0xffffffff, 0x96940200,
	0x8708, 0xffffffff, 0x00900100,
	0xc478, 0xffffffff, 0x00000080,
	0xc404, 0xffffffff, 0x0020003f,
	0x30, 0xffffffff, 0x0000001c,
	0x34, 0x000f0000, 0x000f0000,
	0x160c, 0xffffffff, 0x00000100,
	0x1024, 0xffffffff, 0x00000100,
	0x102c, 0x00000101, 0x00000000,
	0x20a8, 0xffffffff, 0x00000104,
	0x264c, 0x000c0000, 0x000c0000,
	0x2648, 0x000c0000, 0x000c0000,
	0x55e4, 0xff000fff, 0x00000100,
	0x55e8, 0x00000001, 0x00000001,
	0x2f50, 0x00000001, 0x00000001,
	0x30cc, 0xc0000fff, 0x00000104,
	0xc1e4, 0x00000001, 0x00000001,
	0xd0c0, 0xfffffff0, 0x00000100,
	0xd8c0, 0xfffffff0, 0x00000100
};

static const u32 oland_mgcg_cgcg_init[] =
{
	0xc400, 0xffffffff, 0xfffffffc,
	0x802c, 0xffffffff, 0xe0000000,
	0x9a60, 0xffffffff, 0x00000100,
	0x92a4, 0xffffffff, 0x00000100,
	0xc164, 0xffffffff, 0x00000100,
	0x9774, 0xffffffff, 0x00000100,
	0x8984, 0xffffffff, 0x06000100,
	0x8a18, 0xffffffff, 0x00000100,
	0x92a0, 0xffffffff, 0x00000100,
	0xc380, 0xffffffff, 0x00000100,
	0x8b28, 0xffffffff, 0x00000100,
	0x9144, 0xffffffff, 0x00000100,
	0x8d88, 0xffffffff, 0x00000100,
	0x8d8c, 0xffffffff, 0x00000100,
	0x9030, 0xffffffff, 0x00000100,
	0x9034, 0xffffffff, 0x00000100,
	0x9038, 0xffffffff, 0x00000100,
	0x903c, 0xffffffff, 0x00000100,
	0xad80, 0xffffffff, 0x00000100,
	0xac54, 0xffffffff, 0x00000100,
	0x897c, 0xffffffff, 0x06000100,
	0x9868, 0xffffffff, 0x00000100,
	0x9510, 0xffffffff, 0x00000100,
	0xaf04, 0xffffffff, 0x00000100,
	0xae04, 0xffffffff, 0x00000100,
	0x949c, 0xffffffff, 0x00000100,
	0x802c, 0xffffffff, 0xe0000000,
	0x9160, 0xffffffff, 0x00010000,
	0x9164, 0xffffffff, 0x00030002,
	0x9168, 0xffffffff, 0x00040007,
	0x916c, 0xffffffff, 0x00060005,
	0x9170, 0xffffffff, 0x00090008,
	0x9174, 0xffffffff, 0x00020001,
	0x9178, 0xffffffff, 0x00040003,
	0x917c, 0xffffffff, 0x00000007,
	0x9180, 0xffffffff, 0x00060005,
	0x9184, 0xffffffff, 0x00090008,
	0x9188, 0xffffffff, 0x00030002,
	0x918c, 0xffffffff, 0x00050004,
	0x9190, 0xffffffff, 0x00000008,
	0x9194, 0xffffffff, 0x00070006,
	0x9198, 0xffffffff, 0x000a0009,
	0x919c, 0xffffffff, 0x00040003,
	0x91a0, 0xffffffff, 0x00060005,
	0x91a4, 0xffffffff, 0x00000009,
	0x91a8, 0xffffffff, 0x00080007,
	0x91ac, 0xffffffff, 0x000b000a,
	0x91b0, 0xffffffff, 0x00050004,
	0x91b4, 0xffffffff, 0x00070006,
	0x91b8, 0xffffffff, 0x0008000b,
	0x91bc, 0xffffffff, 0x000a0009,
	0x91c0, 0xffffffff, 0x000d000c,
	0x91c4, 0xffffffff, 0x00060005,
	0x91c8, 0xffffffff, 0x00080007,
	0x91cc, 0xffffffff, 0x0000000b,
	0x91d0, 0xffffffff, 0x000a0009,
	0x91d4, 0xffffffff, 0x000d000c,
	0x9150, 0xffffffff, 0x96940200,
	0x8708, 0xffffffff, 0x00900100,
	0xc478, 0xffffffff, 0x00000080,
	0xc404, 0xffffffff, 0x0020003f,
	0x30, 0xffffffff, 0x0000001c,
	0x34, 0x000f0000, 0x000f0000,
	0x160c, 0xffffffff, 0x00000100,
	0x1024, 0xffffffff, 0x00000100,
	0x102c, 0x00000101, 0x00000000,
	0x20a8, 0xffffffff, 0x00000104,
	0x264c, 0x000c0000, 0x000c0000,
	0x2648, 0x000c0000, 0x000c0000,
	0x55e4, 0xff000fff, 0x00000100,
	0x55e8, 0x00000001, 0x00000001,
	0x2f50, 0x00000001, 0x00000001,
	0x30cc, 0xc0000fff, 0x00000104,
	0xc1e4, 0x00000001, 0x00000001,
	0xd0c0, 0xfffffff0, 0x00000100,
	0xd8c0, 0xfffffff0, 0x00000100
};

static const u32 hainan_mgcg_cgcg_init[] =
{
	0xc400, 0xffffffff, 0xfffffffc,
	0x802c, 0xffffffff, 0xe0000000,
	0x9a60, 0xffffffff, 0x00000100,
	0x92a4, 0xffffffff, 0x00000100,
	0xc164, 0xffffffff, 0x00000100,
	0x9774, 0xffffffff, 0x00000100,
	0x8984, 0xffffffff, 0x06000100,
	0x8a18, 0xffffffff, 0x00000100,
	0x92a0, 0xffffffff, 0x00000100,
	0xc380, 0xffffffff, 0x00000100,
	0x8b28, 0xffffffff, 0x00000100,
	0x9144, 0xffffffff, 0x00000100,
	0x8d88, 0xffffffff, 0x00000100,
	0x8d8c, 0xffffffff, 0x00000100,
	0x9030, 0xffffffff, 0x00000100,
	0x9034, 0xffffffff, 0x00000100,
	0x9038, 0xffffffff, 0x00000100,
	0x903c, 0xffffffff, 0x00000100,
	0xad80, 0xffffffff, 0x00000100,
	0xac54, 0xffffffff, 0x00000100,
	0x897c, 0xffffffff, 0x06000100,
	0x9868, 0xffffffff, 0x00000100,
	0x9510, 0xffffffff, 0x00000100,
	0xaf04, 0xffffffff, 0x00000100,
	0xae04, 0xffffffff, 0x00000100,
	0x949c, 0xffffffff, 0x00000100,
	0x802c, 0xffffffff, 0xe0000000,
	0x9160, 0xffffffff, 0x00010000,
	0x9164, 0xffffffff, 0x00030002,
	0x9168, 0xffffffff, 0x00040007,
	0x916c, 0xffffffff, 0x00060005,
	0x9170, 0xffffffff, 0x00090008,
	0x9174, 0xffffffff, 0x00020001,
	0x9178, 0xffffffff, 0x00040003,
	0x917c, 0xffffffff, 0x00000007,
	0x9180, 0xffffffff, 0x00060005,
	0x9184, 0xffffffff, 0x00090008,
	0x9188, 0xffffffff, 0x00030002,
	0x918c, 0xffffffff, 0x00050004,
	0x9190, 0xffffffff, 0x00000008,
	0x9194, 0xffffffff, 0x00070006,
	0x9198, 0xffffffff, 0x000a0009,
	0x919c, 0xffffffff, 0x00040003,
	0x91a0, 0xffffffff, 0x00060005,
	0x91a4, 0xffffffff, 0x00000009,
	0x91a8, 0xffffffff, 0x00080007,
	0x91ac, 0xffffffff, 0x000b000a,
	0x91b0, 0xffffffff, 0x00050004,
	0x91b4, 0xffffffff, 0x00070006,
	0x91b8, 0xffffffff, 0x0008000b,
	0x91bc, 0xffffffff, 0x000a0009,
	0x91c0, 0xffffffff, 0x000d000c,
	0x91c4, 0xffffffff, 0x00060005,
	0x91c8, 0xffffffff, 0x00080007,
	0x91cc, 0xffffffff, 0x0000000b,
	0x91d0, 0xffffffff, 0x000a0009,
	0x91d4, 0xffffffff, 0x000d000c,
	0x9150, 0xffffffff, 0x96940200,
	0x8708, 0xffffffff, 0x00900100,
	0xc478, 0xffffffff, 0x00000080,
	0xc404, 0xffffffff, 0x0020003f,
	0x30, 0xffffffff, 0x0000001c,
	0x34, 0x000f0000, 0x000f0000,
	0x160c, 0xffffffff, 0x00000100,
	0x1024, 0xffffffff, 0x00000100,
	0x20a8, 0xffffffff, 0x00000104,
	0x264c, 0x000c0000, 0x000c0000,
	0x2648, 0x000c0000, 0x000c0000,
	0x2f50, 0x00000001, 0x00000001,
	0x30cc, 0xc0000fff, 0x00000104,
	0xc1e4, 0x00000001, 0x00000001,
	0xd0c0, 0xfffffff0, 0x00000100,
	0xd8c0, 0xfffffff0, 0x00000100
};

static u32 verde_pg_init[] =
{
	0x353c, 0xffffffff, 0x40000,
	0x3538, 0xffffffff, 0x200010ff,
	0x353c, 0xffffffff, 0x0,
	0x353c, 0xffffffff, 0x0,
	0x353c, 0xffffffff, 0x0,
	0x353c, 0xffffffff, 0x0,
	0x353c, 0xffffffff, 0x0,
	0x353c, 0xffffffff, 0x7007,
	0x3538, 0xffffffff, 0x300010ff,
	0x353c, 0xffffffff, 0x0,
	0x353c, 0xffffffff, 0x0,
	0x353c, 0xffffffff, 0x0,
	0x353c, 0xffffffff, 0x0,
	0x353c, 0xffffffff, 0x0,
	0x353c, 0xffffffff, 0x400000,
	0x3538, 0xffffffff, 0x100010ff,
	0x353c, 0xffffffff, 0x0,
	0x353c, 0xffffffff, 0x0,
	0x353c, 0xffffffff, 0x0,
	0x353c, 0xffffffff, 0x0,
	0x353c, 0xffffffff, 0x0,
	0x353c, 0xffffffff, 0x120200,
	0x3538, 0xffffffff, 0x500010ff,
	0x353c, 0xffffffff, 0x0,
	0x353c, 0xffffffff, 0x0,
	0x353c, 0xffffffff, 0x0,
	0x353c, 0xffffffff, 0x0,
	0x353c, 0xffffffff, 0x0,
	0x353c, 0xffffffff, 0x1e1e16,
	0x3538, 0xffffffff, 0x600010ff,
	0x353c, 0xffffffff, 0x0,
	0x353c, 0xffffffff, 0x0,
	0x353c, 0xffffffff, 0x0,
	0x353c, 0xffffffff, 0x0,
	0x353c, 0xffffffff, 0x0,
	0x353c, 0xffffffff, 0x171f1e,
	0x3538, 0xffffffff, 0x700010ff,
	0x353c, 0xffffffff, 0x0,
	0x353c, 0xffffffff, 0x0,
	0x353c, 0xffffffff, 0x0,
	0x353c, 0xffffffff, 0x0,
	0x353c, 0xffffffff, 0x0,
	0x353c, 0xffffffff, 0x0,
	0x3538, 0xffffffff, 0x9ff,
	0x3500, 0xffffffff, 0x0,
	0x3504, 0xffffffff, 0x10000800,
	0x3504, 0xffffffff, 0xf,
	0x3504, 0xffffffff, 0xf,
	0x3500, 0xffffffff, 0x4,
	0x3504, 0xffffffff, 0x1000051e,
	0x3504, 0xffffffff, 0xffff,
	0x3504, 0xffffffff, 0xffff,
	0x3500, 0xffffffff, 0x8,
	0x3504, 0xffffffff, 0x80500,
	0x3500, 0xffffffff, 0x12,
	0x3504, 0xffffffff, 0x9050c,
	0x3500, 0xffffffff, 0x1d,
	0x3504, 0xffffffff, 0xb052c,
	0x3500, 0xffffffff, 0x2a,
	0x3504, 0xffffffff, 0x1053e,
	0x3500, 0xffffffff, 0x2d,
	0x3504, 0xffffffff, 0x10546,
	0x3500, 0xffffffff, 0x30,
	0x3504, 0xffffffff, 0xa054e,
	0x3500, 0xffffffff, 0x3c,
	0x3504, 0xffffffff, 0x1055f,
	0x3500, 0xffffffff, 0x3f,
	0x3504, 0xffffffff, 0x10567,
	0x3500, 0xffffffff, 0x42,
	0x3504, 0xffffffff, 0x1056f,
	0x3500, 0xffffffff, 0x45,
	0x3504, 0xffffffff, 0x10572,
	0x3500, 0xffffffff, 0x48,
	0x3504, 0xffffffff, 0x20575,
	0x3500, 0xffffffff, 0x4c,
	0x3504, 0xffffffff, 0x190801,
	0x3500, 0xffffffff, 0x67,
	0x3504, 0xffffffff, 0x1082a,
	0x3500, 0xffffffff, 0x6a,
	0x3504, 0xffffffff, 0x1b082d,
	0x3500, 0xffffffff, 0x87,
	0x3504, 0xffffffff, 0x310851,
	0x3500, 0xffffffff, 0xba,
	0x3504, 0xffffffff, 0x891,
	0x3500, 0xffffffff, 0xbc,
	0x3504, 0xffffffff, 0x893,
	0x3500, 0xffffffff, 0xbe,
	0x3504, 0xffffffff, 0x20895,
	0x3500, 0xffffffff, 0xc2,
	0x3504, 0xffffffff, 0x20899,
	0x3500, 0xffffffff, 0xc6,
	0x3504, 0xffffffff, 0x2089d,
	0x3500, 0xffffffff, 0xca,
	0x3504, 0xffffffff, 0x8a1,
	0x3500, 0xffffffff, 0xcc,
	0x3504, 0xffffffff, 0x8a3,
	0x3500, 0xffffffff, 0xce,
	0x3504, 0xffffffff, 0x308a5,
	0x3500, 0xffffffff, 0xd3,
	0x3504, 0xffffffff, 0x6d08cd,
	0x3500, 0xffffffff, 0x142,
	0x3504, 0xffffffff, 0x2000095a,
	0x3504, 0xffffffff, 0x1,
	0x3500, 0xffffffff, 0x144,
	0x3504, 0xffffffff, 0x301f095b,
	0x3500, 0xffffffff, 0x165,
	0x3504, 0xffffffff, 0xc094d,
	0x3500, 0xffffffff, 0x173,
	0x3504, 0xffffffff, 0xf096d,
	0x3500, 0xffffffff, 0x184,
	0x3504, 0xffffffff, 0x15097f,
	0x3500, 0xffffffff, 0x19b,
	0x3504, 0xffffffff, 0xc0998,
	0x3500, 0xffffffff, 0x1a9,
	0x3504, 0xffffffff, 0x409a7,
	0x3500, 0xffffffff, 0x1af,
	0x3504, 0xffffffff, 0xcdc,
	0x3500, 0xffffffff, 0x1b1,
	0x3504, 0xffffffff, 0x800,
	0x3508, 0xffffffff, 0x6c9b2000,
	0x3510, 0xfc00, 0x2000,
	0x3544, 0xffffffff, 0xfc0,
	0x28d4, 0x00000100, 0x100
};

static void si_init_golden_registers(struct amdgpu_device *adev)
{
	switch (adev->asic_type) {
	case CHIP_TAHITI:
		amdgpu_program_register_sequence(adev,
						 tahiti_golden_registers,
						 (const u32)ARRAY_SIZE(tahiti_golden_registers));
		amdgpu_program_register_sequence(adev,
						 tahiti_golden_rlc_registers,
						 (const u32)ARRAY_SIZE(tahiti_golden_rlc_registers));
		amdgpu_program_register_sequence(adev,
						 tahiti_mgcg_cgcg_init,
						 (const u32)ARRAY_SIZE(tahiti_mgcg_cgcg_init));
		amdgpu_program_register_sequence(adev,
						 tahiti_golden_registers2,
						 (const u32)ARRAY_SIZE(tahiti_golden_registers2));
		break;
	case CHIP_PITCAIRN:
		amdgpu_program_register_sequence(adev,
						 pitcairn_golden_registers,
						 (const u32)ARRAY_SIZE(pitcairn_golden_registers));
		amdgpu_program_register_sequence(adev,
						 pitcairn_golden_rlc_registers,
						 (const u32)ARRAY_SIZE(pitcairn_golden_rlc_registers));
		amdgpu_program_register_sequence(adev,
						 pitcairn_mgcg_cgcg_init,
						 (const u32)ARRAY_SIZE(pitcairn_mgcg_cgcg_init));
		break;
	case CHIP_VERDE:
		amdgpu_program_register_sequence(adev,
						 verde_golden_registers,
						 (const u32)ARRAY_SIZE(verde_golden_registers));
		amdgpu_program_register_sequence(adev,
						 verde_golden_rlc_registers,
						 (const u32)ARRAY_SIZE(verde_golden_rlc_registers));
		amdgpu_program_register_sequence(adev,
						 verde_mgcg_cgcg_init,
						 (const u32)ARRAY_SIZE(verde_mgcg_cgcg_init));
		amdgpu_program_register_sequence(adev,
						 verde_pg_init,
						 (const u32)ARRAY_SIZE(verde_pg_init));
		break;
	case CHIP_OLAND:
		amdgpu_program_register_sequence(adev,
						 oland_golden_registers,
						 (const u32)ARRAY_SIZE(oland_golden_registers));
		amdgpu_program_register_sequence(adev,
						 oland_golden_rlc_registers,
						 (const u32)ARRAY_SIZE(oland_golden_rlc_registers));
		amdgpu_program_register_sequence(adev,
						 oland_mgcg_cgcg_init,
						 (const u32)ARRAY_SIZE(oland_mgcg_cgcg_init));
		break;
	case CHIP_HAINAN:
		amdgpu_program_register_sequence(adev,
						 hainan_golden_registers,
						 (const u32)ARRAY_SIZE(hainan_golden_registers));
		amdgpu_program_register_sequence(adev,
						 hainan_golden_registers2,
						 (const u32)ARRAY_SIZE(hainan_golden_registers2));
		amdgpu_program_register_sequence(adev,
						 hainan_mgcg_cgcg_init,
						 (const u32)ARRAY_SIZE(hainan_mgcg_cgcg_init));
		break;
	default:
		break;
	}
}

#define PCIE_BUS_CLK                10000
#define TCLK                        (PCIE_BUS_CLK / 10)

/**
 * si_get_xclk - get the xclk
 *
 * @rdev: radeon_device pointer
 *
 * Returns the reference clock used by the gfx engine
 * (SI).
 */
u32 si_get_xclk(struct amdgpu_device *adev)
{
    u32 reference_clock = adev->clock.spll.reference_freq;
	u32 tmp;

	tmp = RREG32(mmCG_CLKPIN_CNTL_2);
	if (REG_GET_FIELD(tmp, CG_CLKPIN_CNTL_2, MUX_TCLK_TO_XCLK))
		return TCLK;

	tmp = RREG32(mmCG_CLKPIN_CNTL);
	if (REG_GET_FIELD(tmp, CG_CLKPIN_CNTL, XTALIN_DIVIDE))
		return reference_clock / 4;

	return reference_clock;
}

/* get temperature in millidegrees */
int si_get_temp(struct amdgpu_device *adev)
{
	u32 temp;
	int actual_temp = 0;

	temp = REG_GET_FIELD(RREG32(mmCG_MULT_THERMAL_STATUS), CG_MULT_THERMAL_STATUS, CTF_TEMP);

	if (temp & 0x200)
		actual_temp = 255;
	else
		actual_temp = temp & 0x1ff;

	actual_temp = (actual_temp * 1000);

	return actual_temp;
}

static void si_vga_set_state(struct amdgpu_device *adev, bool state)
{
	uint32_t tmp;

	tmp = RREG32(mmCONFIG_CNTL);
	if (state == false) {
		tmp &= ~(1<<0);
		tmp |= (1<<1);
	} else {
		tmp &= ~(1<<1);
	}
	WREG32(mmCONFIG_CNTL, tmp);
}

static bool si_read_disabled_bios(struct amdgpu_device *adev)
{
	u32 bus_cntl;
	u32 d1vga_control = 0;
	u32 d2vga_control = 0;
	u32 vga_render_control = 0;
	u32 rom_cntl;
	bool r;

	bus_cntl = RREG32(mmBUS_CNTL);
	if (adev->mode_info.num_crtc) {
		d1vga_control = RREG32(mmD1VGA_CONTROL);
		d2vga_control = RREG32(mmD2VGA_CONTROL);
		vga_render_control = RREG32(mmVGA_RENDER_CONTROL);
	}
	rom_cntl = RREG32(mmROM_CNTL);

	/* enable the rom */
	WREG32(mmBUS_CNTL, (bus_cntl & ~BUS_CNTL__BIOS_ROM_DIS_MASK));
	if (adev->mode_info.num_crtc) {
		/* Disable VGA mode */
		WREG32(mmD1VGA_CONTROL,
		       (d1vga_control & ~(D1VGA_CONTROL__DVGA_CONTROL_MODE_ENABLE_MASK |
					  D1VGA_CONTROL__DVGA_CONTROL_TIMING_SELECT_MASK)));
		WREG32(mmD2VGA_CONTROL,
		       (d2vga_control & ~(D1VGA_CONTROL__DVGA_CONTROL_MODE_ENABLE_MASK |
					  D1VGA_CONTROL__DVGA_CONTROL_TIMING_SELECT_MASK)));
		WREG32(mmVGA_RENDER_CONTROL,
		       (vga_render_control & ~VGA_RENDER_CONTROL__VGA_VSTATUS_CNTL_MASK));
	}
	WREG32(mmROM_CNTL, rom_cntl | ROM_CNTL__SCK_OVERWRITE_MASK);

	r = amdgpu_read_bios(adev);

	/* restore regs */
	WREG32(mmBUS_CNTL, bus_cntl);
	if (adev->mode_info.num_crtc) {
		WREG32(mmD1VGA_CONTROL, d1vga_control);
		WREG32(mmD2VGA_CONTROL, d2vga_control);
		WREG32(mmVGA_RENDER_CONTROL, vga_render_control);
	}
	WREG32(mmROM_CNTL, rom_cntl);
	return r;
}

static bool si_read_bios_from_rom(struct amdgpu_device *adev,
				   u8 *bios, u32 length_bytes)
{
	return false;
}

static struct amdgpu_allowed_register_entry si_allowed_read_registers[] = {
	{mmGRBM_STATUS, false},
	{mmGB_ADDR_CONFIG, false},
	{mmMC_ARB_RAMCFG, false},
	{mmGB_TILE_MODE0, false},
	{mmGB_TILE_MODE1, false},
	{mmGB_TILE_MODE2, false},
	{mmGB_TILE_MODE3, false},
	{mmGB_TILE_MODE4, false},
	{mmGB_TILE_MODE5, false},
	{mmGB_TILE_MODE6, false},
	{mmGB_TILE_MODE7, false},
	{mmGB_TILE_MODE8, false},
	{mmGB_TILE_MODE9, false},
	{mmGB_TILE_MODE10, false},
	{mmGB_TILE_MODE11, false},
	{mmGB_TILE_MODE12, false},
	{mmGB_TILE_MODE13, false},
	{mmGB_TILE_MODE14, false},
	{mmGB_TILE_MODE15, false},
	{mmGB_TILE_MODE16, false},
	{mmGB_TILE_MODE17, false},
	{mmGB_TILE_MODE18, false},
	{mmGB_TILE_MODE19, false},
	{mmGB_TILE_MODE20, false},
	{mmGB_TILE_MODE21, false},
	{mmGB_TILE_MODE22, false},
	{mmGB_TILE_MODE23, false},
	{mmGB_TILE_MODE24, false},
	{mmGB_TILE_MODE25, false},
	{mmGB_TILE_MODE26, false},
	{mmGB_TILE_MODE27, false},
	{mmGB_TILE_MODE28, false},
	{mmGB_TILE_MODE29, false},
	{mmGB_TILE_MODE30, false},
	{mmGB_TILE_MODE31, false},
	{mmCC_RB_BACKEND_DISABLE, false, true},
	{mmGC_USER_RB_BACKEND_DISABLE, false, true},
	{mmGB_BACKEND_MAP, false, false},
	{mmPA_SC_RASTER_CONFIG, false, true},
};

static uint32_t si_read_indexed_register(struct amdgpu_device *adev,
					  u32 se_num, u32 sh_num,
					  u32 reg_offset)
{
	uint32_t val;

	mutex_lock(&adev->grbm_idx_mutex);
	if (se_num != 0xffffffff || sh_num != 0xffffffff)
		gfx_v6_0_select_se_sh(adev, se_num, sh_num);

	val = RREG32(reg_offset);

	if (se_num != 0xffffffff || sh_num != 0xffffffff)
		gfx_v6_0_select_se_sh(adev, 0xffffffff, 0xffffffff);
	mutex_unlock(&adev->grbm_idx_mutex);
	return val;
}

static int si_read_register(struct amdgpu_device *adev, u32 se_num,
			     u32 sh_num, u32 reg_offset, u32 *value)
{
	uint32_t i;

	*value = 0;
	for (i = 0; i < ARRAY_SIZE(si_allowed_read_registers); i++) {
		if (reg_offset != si_allowed_read_registers[i].reg_offset)
			continue;

		if (!si_allowed_read_registers[i].untouched)
			*value = si_allowed_read_registers[i].grbm_indexed ?
				 si_read_indexed_register(adev, se_num,
							   sh_num, reg_offset) :
				 RREG32(reg_offset);
		return 0;
	}
	return -EINVAL;
}

u32 amdgpu_si_gpu_check_soft_reset(struct amdgpu_device *adev)
{
	u32 reset_mask = 0;
	u32 tmp;

	/* GRBM_STATUS */
	tmp = RREG32(mmGRBM_STATUS);
	if (tmp & (GRBM_STATUS__PA_BUSY_MASK | GRBM_STATUS__SC_BUSY_MASK |
		   GRBM_STATUS__BCI_BUSY_MASK | GRBM_STATUS__SX_BUSY_MASK |
		   GRBM_STATUS__TA_BUSY_MASK | GRBM_STATUS__VGT_BUSY_MASK |
		   GRBM_STATUS__DB_BUSY_MASK | GRBM_STATUS__CB_BUSY_MASK |
		   GRBM_STATUS__GDS_BUSY_MASK | GRBM_STATUS__SPI_BUSY_MASK |
		   GRBM_STATUS__IA_BUSY_MASK | GRBM_STATUS__IA_BUSY_NO_DMA_MASK))
		reset_mask |= AMDGPU_RESET_GFX;

	if (tmp & (GRBM_STATUS__CF_RQ_PENDING_MASK | GRBM_STATUS__PF_RQ_PENDING_MASK |
		   GRBM_STATUS__CP_BUSY_MASK | GRBM_STATUS__CP_COHERENCY_BUSY_MASK))
		reset_mask |= AMDGPU_RESET_CP;

	if (tmp & GRBM_STATUS__GRBM_EE_BUSY_MASK)
		reset_mask |= AMDGPU_RESET_GRBM | AMDGPU_RESET_GFX | AMDGPU_RESET_CP;

	/* GRBM_STATUS2 */
	tmp = RREG32(mmGRBM_STATUS2);
	if (tmp & (GRBM_STATUS2__RLC_RQ_PENDING_MASK | GRBM_STATUS2__RLC_BUSY_MASK))
		reset_mask |= AMDGPU_RESET_RLC;

	/* DMA_STATUS_REG 0 */
	tmp = RREG32(mmDMA_STATUS_REG + DMA0_REGISTER_OFFSET);
	if (!(tmp & DMA_STATUS_REG__DMA_IDLE_MASK))
		reset_mask |= AMDGPU_RESET_DMA;

	/* DMA_STATUS_REG 1 */
	tmp = RREG32(mmDMA_STATUS_REG + DMA1_REGISTER_OFFSET);
	if (!(tmp & DMA_STATUS_REG__DMA_IDLE_MASK))
		reset_mask |= AMDGPU_RESET_DMA1;

	/* SRBM_STATUS2 */
	tmp = RREG32(mmSRBM_STATUS2);
	if (tmp & SRBM_STATUS2__DMA_BUSY_MASK)
		reset_mask |= AMDGPU_RESET_DMA;

	if (tmp & SRBM_STATUS2__DMA1_BUSY_MASK)
		reset_mask |= AMDGPU_RESET_DMA1;

	/* SRBM_STATUS */
	tmp = RREG32(mmSRBM_STATUS);

	if (tmp & SRBM_STATUS__IH_BUSY_MASK)
		reset_mask |= AMDGPU_RESET_IH;

	if (tmp & SRBM_STATUS__SEM_BUSY_MASK)
		reset_mask |= AMDGPU_RESET_SEM;

	if (tmp & SRBM_STATUS__GRBM_RQ_PENDING_MASK)
		reset_mask |= AMDGPU_RESET_GRBM;

	if (tmp & SRBM_STATUS__VMC_BUSY_MASK)
		reset_mask |= AMDGPU_RESET_VMC;

	if (tmp & (SRBM_STATUS__MCB_BUSY_MASK | SRBM_STATUS__MCB_NON_DISPLAY_BUSY_MASK |
		   SRBM_STATUS__MCC_BUSY_MASK | SRBM_STATUS__MCD_BUSY_MASK))
		reset_mask |= AMDGPU_RESET_MC;

	if (amdgpu_display_is_display_hung(adev))
		reset_mask |= AMDGPU_RESET_DISPLAY;

	/* VM_L2_STATUS */
	tmp = RREG32(mmVM_L2_STATUS);
	if (tmp & VM_L2_STATUS__L2_BUSY_MASK)
		reset_mask |= AMDGPU_RESET_VMC;

	/* Skip MC reset as it's mostly likely not hung, just busy */
	if (reset_mask & AMDGPU_RESET_MC) {
		DRM_DEBUG("MC busy: 0x%08X, clearing.\n", reset_mask);
		reset_mask &= ~AMDGPU_RESET_MC;
	}

	return reset_mask;
}

static void si_set_bios_scratch_engine_hung(struct amdgpu_device *adev, bool hung)
{
	u32 tmp = RREG32(mmBIOS_3_SCRATCH);

	if (hung)
		tmp |= ATOM_S3_ASIC_GUI_ENGINE_HUNG;
	else
		tmp &= ~ATOM_S3_ASIC_GUI_ENGINE_HUNG;

	WREG32(mmBIOS_3_SCRATCH, tmp);
}

static void si_print_gpu_status_regs(struct amdgpu_device *adev)
{
	dev_info(adev->dev, "  GRBM_STATUS               = 0x%08X\n",
		RREG32(mmGRBM_STATUS));
	dev_info(adev->dev, "  GRBM_STATUS_SE0           = 0x%08X\n",
		RREG32(mmGRBM_STATUS_SE0));
	dev_info(adev->dev, "  GRBM_STATUS_SE1           = 0x%08X\n",
		RREG32(mmGRBM_STATUS_SE1));
	dev_info(adev->dev, "  SRBM_STATUS               = 0x%08X\n",
		RREG32(mmSRBM_STATUS));
	dev_info(adev->dev, "  SRBM_STATUS2              = 0x%08X\n",
		RREG32(mmSRBM_STATUS2));
	dev_info(adev->dev, "  R_008674_CP_STALLED_STAT1 = 0x%08X\n",
		RREG32(mmCP_STALLED_STAT1));
	dev_info(adev->dev, "  R_008678_CP_STALLED_STAT2 = 0x%08X\n",
		RREG32(mmCP_STALLED_STAT2));
	dev_info(adev->dev, "  R_00867C_CP_BUSY_STAT     = 0x%08X\n",
		RREG32(mmCP_BUSY_STAT));
	dev_info(adev->dev, "  R_008680_CP_STAT          = 0x%08X\n",
		RREG32(mmCP_STAT));
	dev_info(adev->dev, "  R_00D034_DMA_STATUS_REG   = 0x%08X\n",
		RREG32(mmDMA_STATUS_REG));
	dev_info(adev->dev, "  R_00D834_DMA_STATUS_REG   = 0x%08X\n",
		RREG32(mmDMA_STATUS_REG + 0x200));
}

static void si_gpu_soft_reset(struct amdgpu_device *adev, u32 reset_mask)
{
	struct amdgpu_mode_mc_save save;
	u32 grbm_soft_reset = 0, srbm_soft_reset = 0;
	u32 tmp;

	if (reset_mask == 0)
		return;

	dev_info(adev->dev, "GPU softreset: 0x%08X\n", reset_mask);

	si_print_gpu_status_regs(adev);
	dev_info(adev->dev, "  VM_CONTEXT1_PROTECTION_FAULT_ADDR   0x%08X\n",
		 RREG32(mmVM_CONTEXT1_PROTECTION_FAULT_ADDR));
	dev_info(adev->dev, "  VM_CONTEXT1_PROTECTION_FAULT_STATUS 0x%08X\n",
		 RREG32(mmVM_CONTEXT1_PROTECTION_FAULT_STATUS));

	/* disable PG/CG */
	/*
	si_fini_pg(rdev);
	si_fini_cg(rdev);
	*/

	/* stop the rlc */
	gfx_v6_0_rlc_stop(adev);

	/* Disable CP parsing/prefetching */
	WREG32(mmCP_ME_CNTL, CP_ME_CNTL__CP_ME_HALT_MASK | CP_ME_CNTL__CP_PFP_HALT_MASK | CP_ME_CNTL__CP_CE_HALT_MASK);

	if (reset_mask & AMDGPU_RESET_DMA) {
		/* dma0 */
		tmp = RREG32(mmDMA_RB_CNTL + DMA0_REGISTER_OFFSET);
		tmp &= ~DMA_RB_CNTL__DMA_RB_ENABLE_MASK;
		WREG32(mmDMA_RB_CNTL + DMA0_REGISTER_OFFSET, tmp);
	}
	if (reset_mask & AMDGPU_RESET_DMA1) {
		/* dma1 */
		tmp = RREG32(mmDMA_RB_CNTL + DMA1_REGISTER_OFFSET);
		tmp &= ~DMA_RB_CNTL__DMA_RB_ENABLE_MASK;
		WREG32(mmDMA_RB_CNTL + DMA1_REGISTER_OFFSET, tmp);
	}

	udelay(50);

	gmc_v6_0_mc_stop(adev, &save);
	if (amdgpu_asic_wait_for_mc_idle(adev)) {
		dev_warn(adev->dev, "Wait for MC idle timedout !\n");
	}

	if (reset_mask & (AMDGPU_RESET_GFX | AMDGPU_RESET_COMPUTE | AMDGPU_RESET_CP)) {
		grbm_soft_reset = GRBM_SOFT_RESET__SOFT_RESET_CB_MASK |
			GRBM_SOFT_RESET__SOFT_RESET_DB_MASK |
			GRBM_SOFT_RESET__SOFT_RESET_GDS_MASK |
			GRBM_SOFT_RESET__SOFT_RESET_PA_MASK |
			GRBM_SOFT_RESET__SOFT_RESET_SC_MASK |
			GRBM_SOFT_RESET__SOFT_RESET_BCI_MASK |
			GRBM_SOFT_RESET__SOFT_RESET_SPI_MASK |
			GRBM_SOFT_RESET__SOFT_RESET_SX_MASK |
			GRBM_SOFT_RESET__SOFT_RESET_TC_MASK |
			GRBM_SOFT_RESET__SOFT_RESET_TA_MASK |
			GRBM_SOFT_RESET__SOFT_RESET_VGT_MASK |
			GRBM_SOFT_RESET__SOFT_RESET_IA_MASK;
	}

	if (reset_mask & AMDGPU_RESET_CP) {
		grbm_soft_reset |= GRBM_SOFT_RESET__SOFT_RESET_CP_MASK | GRBM_SOFT_RESET__SOFT_RESET_VGT_MASK;

		srbm_soft_reset |= SRBM_SOFT_RESET__SOFT_RESET_GRBM_MASK;
	}

	if (reset_mask & AMDGPU_RESET_DMA)
		srbm_soft_reset |= SRBM_SOFT_RESET__SOFT_RESET_DMA_MASK;

	if (reset_mask & AMDGPU_RESET_DMA1)
		srbm_soft_reset |= SRBM_SOFT_RESET__SOFT_RESET_DMA1_MASK;

	if (reset_mask & AMDGPU_RESET_DISPLAY)
		srbm_soft_reset |= SRBM_SOFT_RESET__SOFT_RESET_DC_MASK;

	if (reset_mask & AMDGPU_RESET_RLC)
		grbm_soft_reset |= GRBM_SOFT_RESET__SOFT_RESET_RLC_MASK;

	if (reset_mask & AMDGPU_RESET_SEM)
		srbm_soft_reset |= SRBM_SOFT_RESET__SOFT_RESET_SEM_MASK;

	if (reset_mask & AMDGPU_RESET_IH)
		srbm_soft_reset |= SRBM_SOFT_RESET__SOFT_RESET_IH_MASK;

	if (reset_mask & AMDGPU_RESET_GRBM)
		srbm_soft_reset |= SRBM_SOFT_RESET__SOFT_RESET_GRBM_MASK;

	if (reset_mask & AMDGPU_RESET_VMC)
		srbm_soft_reset |= SRBM_SOFT_RESET__SOFT_RESET_VMC_MASK;

	if (reset_mask & AMDGPU_RESET_MC)
		srbm_soft_reset |= SRBM_SOFT_RESET__SOFT_RESET_MC_MASK;

	if (grbm_soft_reset) {
		tmp = RREG32(mmGRBM_SOFT_RESET);
		tmp |= grbm_soft_reset;
		dev_info(adev->dev, "GRBM_SOFT_RESET=0x%08X\n", tmp);
		WREG32(mmGRBM_SOFT_RESET, tmp);
		tmp = RREG32(mmGRBM_SOFT_RESET);

		udelay(50);

		tmp &= ~grbm_soft_reset;
		WREG32(mmGRBM_SOFT_RESET, tmp);
		tmp = RREG32(mmGRBM_SOFT_RESET);
	}

	if (srbm_soft_reset) {
		tmp = RREG32(mmSRBM_SOFT_RESET);
		tmp |= srbm_soft_reset;
		dev_info(adev->dev, "SRBM_SOFT_RESET=0x%08X\n", tmp);
		WREG32(mmSRBM_SOFT_RESET, tmp);
		tmp = RREG32(mmSRBM_SOFT_RESET);

		udelay(50);

		tmp &= ~srbm_soft_reset;
		WREG32(mmSRBM_SOFT_RESET, tmp);
		tmp = RREG32(mmSRBM_SOFT_RESET);
	}

	/* Wait a little for things to settle down */
	udelay(50);

	gmc_v6_0_mc_resume(adev, &save);
	udelay(50);

	si_print_gpu_status_regs(adev);
}

static void si_gpu_pci_config_reset(struct amdgpu_device *adev)
{
	struct amdgpu_mode_mc_save save;
	u32 tmp, i;

	dev_info(adev->dev, "GPU pci config reset\n");

	/* disable dpm? */

	/* disable cg/pg */

	/* Disable CP parsing/prefetching */
	WREG32(mmCP_ME_CNTL, CP_ME_CNTL__CP_ME_HALT_MASK | CP_ME_CNTL__CP_PFP_HALT_MASK | CP_ME_CNTL__CP_CE_HALT_MASK);
	/* dma0 */
	tmp = RREG32(mmDMA_RB_CNTL + DMA0_REGISTER_OFFSET);
	tmp = REG_SET_FIELD(tmp, DMA_RB_CNTL, DMA_RB_ENABLE, 0);
	WREG32(mmDMA_RB_CNTL + DMA0_REGISTER_OFFSET, tmp);
	/* dma1 */
	tmp = RREG32(mmDMA_RB_CNTL + DMA1_REGISTER_OFFSET);
	tmp = REG_SET_FIELD(tmp, DMA_RB_CNTL, DMA_RB_ENABLE, 0);
	WREG32(mmDMA_RB_CNTL + DMA1_REGISTER_OFFSET, tmp);
	/* XXX other engines? */

	/* halt the rlc, disable cp internal ints */
	gfx_v6_0_rlc_stop(adev);

	udelay(50);

	/* disable mem access */
	gmc_v6_0_mc_stop(adev, &save);
	if (amdgpu_asic_wait_for_mc_idle(adev)) {
		dev_warn(adev->dev, "Wait for MC idle timed out !\n");
	}

	/* set mclk/sclk to bypass */
	/*si_set_clk_bypass_mode(adev);*/
	/* powerdown spll */
	/*si_spll_powerdown(adev);*/
	/* disable BM */
	pci_clear_master(adev->pdev);
	/* reset */
	amdgpu_pci_config_reset(adev);
	/* wait for asic to come out of reset */
	for (i = 0; i < adev->usec_timeout; i++) {
		if (RREG32(mmCONFIG_MEMSIZE) != 0xffffffff)
			break;
		udelay(1);
	}
}

static int si_asic_reset(struct amdgpu_device *adev)
{
	u32 reset_mask;

	reset_mask = amdgpu_si_gpu_check_soft_reset(adev);

	if (reset_mask)
		si_set_bios_scratch_engine_hung(adev, true);

	/* try soft reset */
	si_gpu_soft_reset(adev, reset_mask);

	reset_mask = amdgpu_si_gpu_check_soft_reset(adev);

	/* try pci config reset */
	if (reset_mask && amdgpu_hard_reset)
		si_gpu_pci_config_reset(adev);

	reset_mask = amdgpu_si_gpu_check_soft_reset(adev);

	if (!reset_mask)
		si_set_bios_scratch_engine_hung(adev, false);

	return 0;
}

static unsigned si_uvd_calc_upll_post_div(unsigned vco_freq,
					      unsigned target_freq,
					      unsigned pd_min,
					      unsigned pd_even)
{
	unsigned post_div = vco_freq / target_freq;

	/* adjust to post divider minimum value */
	if (post_div < pd_min)
		post_div = pd_min;

	/* we alway need a frequency less than or equal the target */
	if ((vco_freq / post_div) > target_freq)
		post_div += 1;

	/* post dividers above a certain value must be even */
	if (post_div > pd_even && post_div % 2)
		post_div += 1;

	return post_div;
}

/**
 * si_uvd_calc_upll_dividers - calc UPLL clock dividers
 *
 * @rdev: radeon_device pointer
 * @vclk: wanted VCLK
 * @dclk: wanted DCLK
 * @vco_min: minimum VCO frequency
 * @vco_max: maximum VCO frequency
 * @fb_factor: factor to multiply vco freq with
 * @fb_mask: limit and bitmask for feedback divider
 * @pd_min: post divider minimum
 * @pd_max: post divider maximum
 * @pd_even: post divider must be even above this value
 * @optimal_fb_div: resulting feedback divider
 * @optimal_vclk_div: resulting vclk post divider
 * @optimal_dclk_div: resulting dclk post divider
 *
 * Calculate dividers for UVDs UPLL (R6xx-SI, except APUs).
 * Returns zero on success -EINVAL on error.
 */
static int si_uvd_calc_upll_dividers(struct amdgpu_device *adev,
				  unsigned vclk, unsigned dclk,
				  unsigned vco_min, unsigned vco_max,
				  unsigned fb_factor, unsigned fb_mask,
				  unsigned pd_min, unsigned pd_max,
				  unsigned pd_even,
				  unsigned *optimal_fb_div,
				  unsigned *optimal_vclk_div,
				  unsigned *optimal_dclk_div)
{
	unsigned vco_freq, ref_freq = adev->clock.spll.reference_freq;

	/* start off with something large */
	unsigned optimal_score = ~0;

	/* loop through vco from low to high */
	vco_min = max(max(vco_min, vclk), dclk);
	for (vco_freq = vco_min; vco_freq <= vco_max; vco_freq += 100) {

		uint64_t fb_div = (uint64_t)vco_freq * fb_factor;
		unsigned vclk_div, dclk_div, score;

		do_div(fb_div, ref_freq);

		/* fb div out of range ? */
		if (fb_div > fb_mask)
			break; /* it can oly get worse */

		fb_div &= fb_mask;

		/* calc vclk divider with current vco freq */
		vclk_div = si_uvd_calc_upll_post_div(vco_freq, vclk,
							 pd_min, pd_even);
		if (vclk_div > pd_max)
			break; /* vco is too big, it has to stop */

		/* calc dclk divider with current vco freq */
		dclk_div = si_uvd_calc_upll_post_div(vco_freq, dclk,
							 pd_min, pd_even);
		if (vclk_div > pd_max)
			break; /* vco is too big, it has to stop */

		/* calc score with current vco freq */
		score = vclk - (vco_freq / vclk_div) + dclk - (vco_freq / dclk_div);

		/* determine if this vco setting is better than current optimal settings */
		if (score < optimal_score) {
			*optimal_fb_div = fb_div;
			*optimal_vclk_div = vclk_div;
			*optimal_dclk_div = dclk_div;
			optimal_score = score;
			if (optimal_score == 0)
				break; /* it can't get better than this */
		}
	}

	/* did we found a valid setup ? */
	if (optimal_score == ~0)
		return -EINVAL;

	return 0;
}

static int si_send_uvd_upll_ctlreq(struct amdgpu_device *adev,
				unsigned cg_upll_func_cntl)
{
	unsigned i;

	/* make sure UPLL_CTLREQ is deasserted */
	WREG32_P(cg_upll_func_cntl, 0, ~CG_UPLL_FUNC_CNTL__UPLL_CTLREQ_MASK);

	mdelay(10);

	/* assert UPLL_CTLREQ */
	WREG32_P(cg_upll_func_cntl, CG_UPLL_FUNC_CNTL__UPLL_CTLREQ_MASK, ~CG_UPLL_FUNC_CNTL__UPLL_CTLREQ_MASK);

	/* wait for CTLACK and CTLACK2 to get asserted */
	for (i = 0; i < 100; ++i) {
		uint32_t mask = CG_UPLL_FUNC_CNTL__UPLL_CTLACK_MASK | CG_UPLL_FUNC_CNTL__UPLL_CTLACK2_MASK;
		if ((RREG32(cg_upll_func_cntl) & mask) == mask)
			break;
		mdelay(10);
	}

	/* deassert UPLL_CTLREQ */
	WREG32_P(cg_upll_func_cntl, 0, ~CG_UPLL_FUNC_CNTL__UPLL_CTLREQ_MASK);

	if (i == 100) {
		DRM_ERROR("Timeout setting UVD clocks!\n");
		return -ETIMEDOUT;
	}

	return 0;
}

static int si_set_uvd_clocks(struct amdgpu_device *adev, u32 vclk, u32 dclk)
{
	unsigned fb_div = 0, vclk_div = 0, dclk_div = 0;
	int r;
	u32 tmp;

	printk(KERN_ALERT "Set UVD clocks called. Prepare liquid N2\n");

	/* bypass vclk and dclk with bclk */
	tmp = RREG32(mmCG_UPLL_FUNC_CNTL_2);
	tmp = REG_SET_FIELD(tmp, CG_UPLL_FUNC_CNTL_2, VCLK_SRC_SEL, 1);
	tmp = REG_SET_FIELD(tmp, CG_UPLL_FUNC_CNTL_2, DCLK_SRC_SEL, 1);
	WREG32(mmCG_UPLL_FUNC_CNTL_2, tmp);

	/* put PLL in bypass mode */
	WREG32_P(mmCG_UPLL_FUNC_CNTL, CG_UPLL_FUNC_CNTL__UPLL_BYPASS_EN_MASK, ~CG_UPLL_FUNC_CNTL__UPLL_BYPASS_EN_MASK);

	if (!vclk || !dclk) {
		/* keep the Bypass mode */
		return 0;
	}

	r = si_uvd_calc_upll_dividers(adev, vclk, dclk, 125000, 250000,
					  16384, 0x03FFFFFF, 0, 128, 5,
					  &fb_div, &vclk_div, &dclk_div);
	if (r)
		return r;

	/* set RESET_ANTI_MUX to 0 */
	WREG32_P(mmCG_UPLL_FUNC_CNTL_5, 0, ~CG_UPLL_FUNC_CNTL_5__RESET_ANTI_MUX_MASK);

	/* set VCO_MODE to 1 */
	WREG32_P(mmCG_UPLL_FUNC_CNTL, CG_UPLL_FUNC_CNTL__UPLL_VCO_MODE_MASK, ~CG_UPLL_FUNC_CNTL__UPLL_VCO_MODE_MASK);

	/* disable sleep mode */
	WREG32_P(mmCG_UPLL_FUNC_CNTL, 0, ~CG_UPLL_FUNC_CNTL__UPLL_SLEEP_MASK);

	/* deassert UPLL_RESET */
	WREG32_P(mmCG_UPLL_FUNC_CNTL, 0, ~CG_UPLL_FUNC_CNTL__UPLL_RESET_MASK);

	mdelay(1);

	r = si_send_uvd_upll_ctlreq(adev, mmCG_UPLL_FUNC_CNTL);
	if (r)
		return r;

	/* assert UPLL_RESET again */
	WREG32_P(mmCG_UPLL_FUNC_CNTL, CG_UPLL_FUNC_CNTL__UPLL_RESET_MASK, ~CG_UPLL_FUNC_CNTL__UPLL_RESET_MASK);

	/* disable spread spectrum. */
	WREG32_P(mmCG_UPLL_SPREAD_SPECTRUM, 0, ~CG_UPLL_SPREAD_SPECTRUM__SSEN_MASK);

	/* set feedback divider */
	tmp = RREG32(mmCG_UPLL_FUNC_CNTL_3);
	tmp = REG_SET_FIELD(tmp, CG_UPLL_FUNC_CNTL_3, UPLL_FB_DIV, fb_div);
	WREG32(mmCG_UPLL_FUNC_CNTL_3, tmp);

	/* set ref divider to 0 */
	WREG32_P(mmCG_UPLL_FUNC_CNTL, 0, ~CG_UPLL_FUNC_CNTL__UPLL_REF_DIV_MASK);

	if (fb_div < 307200)
		WREG32_P(mmCG_UPLL_FUNC_CNTL_4, 0, ~CG_UPLL_FUNC_CNTL_4__UPLL_SPARE_ISPARE9_MASK);
	else
		WREG32_P(mmCG_UPLL_FUNC_CNTL_4, CG_UPLL_FUNC_CNTL_4__UPLL_SPARE_ISPARE9_MASK, ~CG_UPLL_FUNC_CNTL_4__UPLL_SPARE_ISPARE9_MASK);

	/* set PDIV_A and PDIV_B */
	tmp = RREG32(mmCG_UPLL_FUNC_CNTL_2);
	tmp = REG_SET_FIELD(tmp, CG_UPLL_FUNC_CNTL_2, UPLL_PDIV_A, vclk_div);
	tmp = REG_SET_FIELD(tmp, CG_UPLL_FUNC_CNTL_2, UPLL_PDIV_B, dclk_div);
	WREG32(mmCG_UPLL_FUNC_CNTL_2, tmp);

	/* give the PLL some time to settle */
	mdelay(15);

	/* deassert PLL_RESET */
	WREG32_P(mmCG_UPLL_FUNC_CNTL, 0, ~CG_UPLL_FUNC_CNTL__UPLL_RESET_MASK);

	mdelay(15);

	/* switch from bypass mode to normal mode */
	WREG32_P(mmCG_UPLL_FUNC_CNTL, 0, ~CG_UPLL_FUNC_CNTL__UPLL_BYPASS_EN_MASK);

	r = si_send_uvd_upll_ctlreq(adev, mmCG_UPLL_FUNC_CNTL);
	if (r)
		return r;

	/* switch VCLK and DCLK selection */
	tmp = RREG32(mmCG_UPLL_FUNC_CNTL_2);
	tmp = REG_SET_FIELD(tmp, CG_UPLL_FUNC_CNTL_2, VCLK_SRC_SEL, 2);
	tmp = REG_SET_FIELD(tmp, CG_UPLL_FUNC_CNTL_2, DCLK_SRC_SEL, 2);
	WREG32(mmCG_UPLL_FUNC_CNTL_2, tmp);

	mdelay(100);

	return 0;
}

static int si_vce_send_vcepll_ctlreq(struct amdgpu_device *adev)
{
    unsigned i;

    /* make sure VCEPLL_CTLREQ is deasserted */
    WREG32_SMC_P(ixCG_VCEPLL_FUNC_CNTL, 0, ~CG_UPLL_FUNC_CNTL__UPLL_CTLREQ_MASK);

    mdelay(10);

    /* assert UPLL_CTLREQ */
    WREG32_SMC_P(ixCG_VCEPLL_FUNC_CNTL, CG_UPLL_FUNC_CNTL__UPLL_CTLREQ_MASK, ~CG_UPLL_FUNC_CNTL__UPLL_CTLREQ_MASK);

    /* wait for CTLACK and CTLACK2 to get asserted */
    for (i = 0; i < 100; ++i) {
            uint32_t mask = CG_UPLL_FUNC_CNTL__UPLL_CTLACK_MASK | CG_UPLL_FUNC_CNTL__UPLL_CTLACK2_MASK;
            if ((RREG32_SMC(ixCG_VCEPLL_FUNC_CNTL) & mask) == mask)
                    break;
            mdelay(10);
    }

    /* deassert UPLL_CTLREQ */
    WREG32_SMC_P(ixCG_VCEPLL_FUNC_CNTL, 0, ~CG_UPLL_FUNC_CNTL__UPLL_CTLREQ_MASK);

    if (i == 100) {
            DRM_ERROR("Timeout setting UVD clocks!\n");
            return -ETIMEDOUT;
    }

    return 0;
}

static int si_set_vce_clocks(struct amdgpu_device *adev, u32 evclk, u32 ecclk)
{
	unsigned fb_div = 0, evclk_div = 0, ecclk_div = 0;
	int r;
	u32 tmp;

	printk(KERN_ALERT "Set VCE clocks called. Prepare liquid N2\n");

	/* bypass evclk and ecclk with bclk */
	tmp = RREG32_SMC(ixCG_VCEPLL_FUNC_CNTL_2);
	tmp = REG_SET_FIELD(tmp, CG_VCEPLL_FUNC_CNTL_2, EVCLK_SRC_SEL, 1);
	tmp = REG_SET_FIELD(tmp, CG_VCEPLL_FUNC_CNTL_2, ECCLK_SRC_SEL, 1);
	WREG32_SMC(ixCG_VCEPLL_FUNC_CNTL_2, tmp);

	/* put PLL in bypass mode */
	tmp = RREG32_SMC(ixCG_VCEPLL_FUNC_CNTL);
	tmp = REG_SET_FIELD(tmp, CG_VCEPLL_FUNC_CNTL, VCEPLL_BYPASS_EN, 1);
	WREG32_SMC(ixCG_VCEPLL_FUNC_CNTL, tmp);

	if (!evclk || !ecclk) {
		/* keep the Bypass mode, put PLL to sleep */
		tmp = RREG32_SMC(ixCG_VCEPLL_FUNC_CNTL);
		tmp = REG_SET_FIELD(tmp, CG_VCEPLL_FUNC_CNTL, VCEPLL_SLEEP, 1);
		WREG32_SMC(ixCG_VCEPLL_FUNC_CNTL, tmp);
		return 0;
	}

	r = si_uvd_calc_upll_dividers(adev, evclk, ecclk, 125000, 250000,
					  16384, 0x03FFFFFF, 0, 128, 5,
					  &fb_div, &evclk_div, &ecclk_div);
	if (r)
		return r;

	/* set RESET_ANTI_MUX to 0 */
	tmp = RREG32_SMC(ixCG_VCEPLL_FUNC_CNTL_5);
	tmp = REG_SET_FIELD(tmp, CG_VCEPLL_FUNC_CNTL_5, RESET_ANTI_MUX, 0);
	WREG32_SMC(ixCG_VCEPLL_FUNC_CNTL_5, tmp);

	/* set VCO_MODE to 1 */
	tmp = RREG32_SMC(ixCG_VCEPLL_FUNC_CNTL);
	tmp = REG_SET_FIELD(tmp, CG_VCEPLL_FUNC_CNTL, VCEPLL_VCO_MODE, 1);
	WREG32_SMC(ixCG_VCEPLL_FUNC_CNTL, tmp);

	/* toggle VCEPLL_SLEEP to 1 then back to 0 */
	tmp = RREG32_SMC(ixCG_VCEPLL_FUNC_CNTL);
	tmp = REG_SET_FIELD(tmp, CG_VCEPLL_FUNC_CNTL, VCEPLL_SLEEP, 1);
	WREG32_SMC(ixCG_VCEPLL_FUNC_CNTL, tmp);

	tmp = RREG32_SMC(ixCG_VCEPLL_FUNC_CNTL);
	tmp = REG_SET_FIELD(tmp, CG_VCEPLL_FUNC_CNTL, VCEPLL_SLEEP, 0);
	WREG32_SMC(ixCG_VCEPLL_FUNC_CNTL, tmp);

	/* deassert VCEPLL_RESET */
	tmp = RREG32_SMC(ixCG_VCEPLL_FUNC_CNTL);
	tmp = REG_SET_FIELD(tmp, CG_VCEPLL_FUNC_CNTL, VCEPLL_RESET, 0);
	WREG32_SMC(ixCG_VCEPLL_FUNC_CNTL, tmp);

	mdelay(1);

	r = si_vce_send_vcepll_ctlreq(adev);
	if (r)
		return r;

	/* assert VCEPLL_RESET again */
	WREG32_SMC_P(ixCG_VCEPLL_FUNC_CNTL, CG_VCEPLL_FUNC_CNTL__VCEPLL_RESET_MASK, ~CG_VCEPLL_FUNC_CNTL__VCEPLL_RESET_MASK);

	/* disable spread spectrum. */
	WREG32_SMC_P(ixCG_VCEPLL_SPREAD_SPECTRUM, 0, ~CG_VCEPLL_SPREAD_SPECTRUM__VCEPLL_SSEN_MASK);

	/* set feedback divider */
	tmp = RREG32_SMC(ixCG_VCEPLL_FUNC_CNTL_3);
	tmp = REG_SET_FIELD(tmp, CG_VCEPLL_FUNC_CNTL_3, VCEPLL_FB_DIV, fb_div);
	WREG32_SMC(ixCG_VCEPLL_FUNC_CNTL_3, tmp);

	/* set ref divider to 0 */
	WREG32_SMC_P(ixCG_VCEPLL_FUNC_CNTL, 0, ~CG_VCEPLL_FUNC_CNTL__VCEPLL_REF_DIV_MASK);

	/* set PDIV_A and PDIV_B */
	tmp = RREG32_SMC(ixCG_VCEPLL_FUNC_CNTL_2);
	tmp = REG_SET_FIELD(tmp, CG_VCEPLL_FUNC_CNTL_2, VCEPLL_PDIV_A, evclk_div);
	tmp = REG_SET_FIELD(tmp, CG_VCEPLL_FUNC_CNTL_2, VCEPLL_PDIV_B, ecclk_div);
	WREG32_SMC(ixCG_VCEPLL_FUNC_CNTL_2, tmp);

	/* give the PLL some time to settle */
	mdelay(15);

	/* deassert PLL_RESET */
	WREG32_SMC_P(ixCG_VCEPLL_FUNC_CNTL, 0, ~CG_VCEPLL_FUNC_CNTL__VCEPLL_RESET_MASK);

	mdelay(15);

	/* switch from bypass mode to normal mode */
	WREG32_SMC_P(ixCG_VCEPLL_FUNC_CNTL, 0, ~CG_VCEPLL_FUNC_CNTL__VCEPLL_BYPASS_EN_MASK);

	r = si_vce_send_vcepll_ctlreq(adev);
	if (r)
		return r;

	/* switch VCLK and DCLK selection */
	tmp = RREG32_SMC(ixCG_VCEPLL_FUNC_CNTL_2);
	tmp = REG_SET_FIELD(tmp, CG_VCEPLL_FUNC_CNTL_2, EVCLK_SRC_SEL, 16);
	tmp = REG_SET_FIELD(tmp, CG_VCEPLL_FUNC_CNTL_2, ECCLK_SRC_SEL, 16);
	WREG32_SMC(ixCG_VCEPLL_FUNC_CNTL_2, tmp);

	mdelay(100);

	return 0;
}

static void si_pcie_gen3_enable(struct amdgpu_device *adev)
{
	struct pci_dev *root = adev->pdev->bus->self;
	int bridge_pos, gpu_pos;
	u32 speed_cntl, mask, current_data_rate;
	int ret, i;
	u16 tmp16;

	if (pci_is_root_bus(adev->pdev->bus))
		return;

	if (amdgpu_pcie_gen2 == 0)
		return;

	if (adev->flags & AMD_IS_APU)
		return;

	if (!(adev->pm.pcie_gen_mask & (CAIL_PCIE_LINK_SPEED_SUPPORT_GEN2 |
					CAIL_PCIE_LINK_SPEED_SUPPORT_GEN3)))
		return;

	ret = drm_pcie_get_speed_cap_mask(adev->ddev, &mask);
	if (ret != 0)
		return;

	if (!(mask & (DRM_PCIE_SPEED_50 | DRM_PCIE_SPEED_80)))
		return;

	speed_cntl = RREG32_PCIE(ixPCIE_LC_SPEED_CNTL);
	current_data_rate = REG_GET_FIELD(speed_cntl, PCIE_LC_SPEED_CNTL, LC_CURRENT_DATA_RATE);
	if (mask & DRM_PCIE_SPEED_80) {
		if (current_data_rate == 2) {
			DRM_INFO("PCIE gen 3 link speeds already enabled\n");
			return;
		}
		DRM_INFO("enabling PCIE gen 3 link speeds, disable with amdgpu.pcie_gen2=0\n");
	} else if (mask & DRM_PCIE_SPEED_50) {
		if (current_data_rate == 1) {
			DRM_INFO("PCIE gen 2 link speeds already enabled\n");
			return;
		}
		DRM_INFO("enabling PCIE gen 2 link speeds, disable with amdgpu.pcie_gen2=0\n");
	}

	bridge_pos = pci_pcie_cap(root);
	if (!bridge_pos)
		return;

	gpu_pos = pci_pcie_cap(adev->pdev);
	if (!gpu_pos)
		return;

	if (mask & DRM_PCIE_SPEED_80) {
		/* re-try equalization if gen3 is not already enabled */
		if (current_data_rate != 2) {
			u16 bridge_cfg, gpu_cfg;
			u16 bridge_cfg2, gpu_cfg2;
			u32 max_lw, current_lw, tmp;

			pci_read_config_word(root, bridge_pos + PCI_EXP_LNKCTL, &bridge_cfg);
			pci_read_config_word(adev->pdev, gpu_pos + PCI_EXP_LNKCTL, &gpu_cfg);

			tmp16 = bridge_cfg | PCI_EXP_LNKCTL_HAWD;
			pci_write_config_word(root, bridge_pos + PCI_EXP_LNKCTL, tmp16);

			tmp16 = gpu_cfg | PCI_EXP_LNKCTL_HAWD;
			pci_write_config_word(adev->pdev, gpu_pos + PCI_EXP_LNKCTL, tmp16);

			tmp = RREG32_PCIE(ixPCIE_LC_STATUS1);
			max_lw = REG_GET_FIELD(tmp, PCIE_LC_STATUS1, LC_DETECTED_LINK_WIDTH);
			current_lw = REG_GET_FIELD(tmp, PCIE_LC_STATUS1, LC_OPERATING_LINK_WIDTH);

			if (current_lw < max_lw) {
				tmp = RREG32_PCIE(ixPCIE_LC_LINK_WIDTH_CNTL);
				if (REG_GET_FIELD(tmp, PCIE_LC_LINK_WIDTH_CNTL, LC_RENEGOTIATION_SUPPORT)) {
					tmp = REG_SET_FIELD(tmp, PCIE_LC_LINK_WIDTH_CNTL, LC_LINK_WIDTH, max_lw);
					tmp = REG_SET_FIELD(tmp, PCIE_LC_LINK_WIDTH_CNTL, LC_UPCONFIGURE_DIS, 0);
					tmp = REG_SET_FIELD(tmp, PCIE_LC_LINK_WIDTH_CNTL, LC_UPCONFIGURE_SUPPORT, 1);
					tmp = REG_SET_FIELD(tmp, PCIE_LC_LINK_WIDTH_CNTL, LC_RENEGOTIATE_EN, 1);
					tmp = REG_SET_FIELD(tmp, PCIE_LC_LINK_WIDTH_CNTL, LC_RECONFIG_NOW, 1);
					WREG32_PCIE(ixPCIE_LC_LINK_WIDTH_CNTL, tmp);
				}
			}

			for (i = 0; i < 10; i++) {
				/* check status */
				pci_read_config_word(adev->pdev, gpu_pos + PCI_EXP_DEVSTA, &tmp16);
				if (tmp16 & PCI_EXP_DEVSTA_TRPND)
					break;

				pci_read_config_word(root, bridge_pos + PCI_EXP_LNKCTL, &bridge_cfg);
				pci_read_config_word(adev->pdev, gpu_pos + PCI_EXP_LNKCTL, &gpu_cfg);

				pci_read_config_word(root, bridge_pos + PCI_EXP_LNKCTL2, &bridge_cfg2);
				pci_read_config_word(adev->pdev, gpu_pos + PCI_EXP_LNKCTL2, &gpu_cfg2);

				tmp = RREG32_PCIE(ixPCIE_LC_CNTL4);
				tmp = REG_SET_FIELD(tmp, PCIE_LC_CNTL4, LC_SET_QUIESCE, 1);
				WREG32_PCIE(ixPCIE_LC_CNTL4, tmp);

				tmp = RREG32_PCIE(ixPCIE_LC_CNTL4);
				tmp = REG_SET_FIELD(tmp, PCIE_LC_CNTL4, LC_REDO_EQ, 1);
				WREG32_PCIE(ixPCIE_LC_CNTL4, tmp);

				mdelay(100);

				/* linkctl */
				pci_read_config_word(root, bridge_pos + PCI_EXP_LNKCTL, &tmp16);
				tmp16 &= ~PCI_EXP_LNKCTL_HAWD;
				tmp16 |= (bridge_cfg & PCI_EXP_LNKCTL_HAWD);
				pci_write_config_word(root, bridge_pos + PCI_EXP_LNKCTL, tmp16);

				pci_read_config_word(adev->pdev, gpu_pos + PCI_EXP_LNKCTL, &tmp16);
				tmp16 &= ~PCI_EXP_LNKCTL_HAWD;
				tmp16 |= (gpu_cfg & PCI_EXP_LNKCTL_HAWD);
				pci_write_config_word(adev->pdev, gpu_pos + PCI_EXP_LNKCTL, tmp16);

				/* linkctl2 */
				pci_read_config_word(root, bridge_pos + PCI_EXP_LNKCTL2, &tmp16);
				tmp16 &= ~((1 << 4) | (7 << 9));
				tmp16 |= (bridge_cfg2 & ((1 << 4) | (7 << 9)));
				pci_write_config_word(root, bridge_pos + PCI_EXP_LNKCTL2, tmp16);

				pci_read_config_word(adev->pdev, gpu_pos + PCI_EXP_LNKCTL2, &tmp16);
				tmp16 &= ~((1 << 4) | (7 << 9));
				tmp16 |= (gpu_cfg2 & ((1 << 4) | (7 << 9)));
				pci_write_config_word(adev->pdev, gpu_pos + PCI_EXP_LNKCTL2, tmp16);

				tmp = RREG32_PCIE(ixPCIE_LC_CNTL4);
				tmp = REG_SET_FIELD(tmp, PCIE_LC_CNTL4, LC_SET_QUIESCE, 0);
				WREG32_PCIE(ixPCIE_LC_CNTL4, tmp);
			}
		}
	}

	/* set the link speed */
	speed_cntl = REG_SET_FIELD(speed_cntl, PCIE_LC_SPEED_CNTL, LC_FORCE_EN_SW_SPEED_CHANGE, 1);
	speed_cntl = REG_SET_FIELD(speed_cntl, PCIE_LC_SPEED_CNTL, LC_FORCE_DIS_HW_SPEED_CHANGE, 1);
	speed_cntl = REG_SET_FIELD(speed_cntl, PCIE_LC_SPEED_CNTL, LC_FORCE_DIS_SW_SPEED_CHANGE, 0);
	WREG32_PCIE(ixPCIE_LC_SPEED_CNTL, speed_cntl);

	pci_read_config_word(adev->pdev, gpu_pos + PCI_EXP_LNKCTL2, &tmp16);
	tmp16 &= ~0xf;
	if (mask & DRM_PCIE_SPEED_80)
		tmp16 |= 3; /* gen3 */
	else if (mask & DRM_PCIE_SPEED_50)
		tmp16 |= 2; /* gen2 */
	else
		tmp16 |= 1; /* gen1 */
	pci_write_config_word(adev->pdev, gpu_pos + PCI_EXP_LNKCTL2, tmp16);

	speed_cntl = RREG32_PCIE(ixPCIE_LC_SPEED_CNTL);
	speed_cntl = REG_SET_FIELD(speed_cntl, PCIE_LC_SPEED_CNTL, LC_INITIATE_LINK_SPEED_CHANGE, 1);
	WREG32_PCIE(ixPCIE_LC_SPEED_CNTL, speed_cntl);

	for (i = 0; i < adev->usec_timeout; i++) {
		speed_cntl = RREG32_PCIE(ixPCIE_LC_SPEED_CNTL);
		if (REG_GET_FIELD(speed_cntl, PCIE_LC_SPEED_CNTL, LC_INITIATE_LINK_SPEED_CHANGE) == 0)
			break;
		udelay(1);
	}
}

static void si_program_aspm(struct amdgpu_device *adev)
{
	u32 data, orig;
	bool disable_l0s = false, disable_l1 = false, disable_plloff_in_l1 = false;
	bool disable_clkreq = false;

	if (amdgpu_aspm == 0)
		return;

	/* XXX double check APUs */
	if (adev->flags & AMD_IS_APU)
		return;

	orig = data = RREG32_PCIE(ixPCIE_LC_N_FTS_CNTL);
	data = REG_SET_FIELD(data, PCIE_LC_N_FTS_CNTL, LC_XMIT_N_FTS, 0x24);
	data = REG_SET_FIELD(data, PCIE_LC_N_FTS_CNTL, LC_XMIT_N_FTS_OVERRIDE_EN, 1);
	if (orig != data)
		WREG32_PCIE(ixPCIE_LC_N_FTS_CNTL, data);

	orig = data = RREG32_PCIE(ixPCIE_LC_CNTL3);
	data = REG_SET_FIELD(data, PCIE_LC_CNTL3, LC_GO_TO_RECOVERY, 1);
	if (orig != data)
		WREG32_PCIE(ixPCIE_LC_CNTL3, data);

	orig = data = RREG32_PCIE(ixPCIE_P_CNTL);
	data = REG_SET_FIELD(data, PCIE_P_CNTL, P_IGNORE_EDB_ERR, 1);
	if (orig != data)
		WREG32_PCIE(ixPCIE_P_CNTL, data);

	orig = data = RREG32_PCIE(ixPCIE_LC_CNTL);
	data = REG_SET_FIELD(data, PCIE_LC_CNTL, LC_L0S_INACTIVITY, 0);
	data = REG_SET_FIELD(data, PCIE_LC_CNTL, LC_L1_INACTIVITY, 0);
	data = REG_SET_FIELD(data, PCIE_LC_CNTL, LC_PMI_TO_L1_DIS, 1);
	if (!disable_l0s)
		data = REG_SET_FIELD(data, PCIE_LC_CNTL, LC_L0S_INACTIVITY, 7);

	if (!disable_l1) {
		data = REG_SET_FIELD(data, PCIE_LC_CNTL, LC_L1_INACTIVITY, 7);
		data = REG_SET_FIELD(data, PCIE_LC_CNTL, LC_PMI_TO_L1_DIS, 0);
		if (orig != data)
			WREG32_PCIE(ixPCIE_LC_CNTL, data);

		if (!disable_plloff_in_l1) {
			bool clk_req_support;

			orig = data = RREG32_PIF_PHY0(ixPB0_PIF_PWRDOWN_0);
			data = REG_SET_FIELD(data, PB0_PIF_PWRDOWN_0, PLL_POWER_STATE_IN_OFF_0, 7);
			data = REG_SET_FIELD(data, PB0_PIF_PWRDOWN_0, PLL_POWER_STATE_IN_TXS2_0, 7);
			if (orig != data)
				WREG32_PIF_PHY0(ixPB0_PIF_PWRDOWN_0, data);

			orig = data = RREG32_PIF_PHY0(ixPB0_PIF_PWRDOWN_1);
			data = REG_SET_FIELD(data, PB0_PIF_PWRDOWN_1, PLL_POWER_STATE_IN_OFF_1, 7);
			data = REG_SET_FIELD(data, PB0_PIF_PWRDOWN_1, PLL_POWER_STATE_IN_TXS2_1, 7);
			if (orig != data)
				WREG32_PIF_PHY0(ixPB0_PIF_PWRDOWN_1, data);

			orig = data = RREG32_PIF_PHY1(ixPB1_PIF_PWRDOWN_0);
			data = REG_SET_FIELD(data, PB0_PIF_PWRDOWN_0, PLL_POWER_STATE_IN_OFF_0, 7);
			data = REG_SET_FIELD(data, PB0_PIF_PWRDOWN_0, PLL_POWER_STATE_IN_TXS2_0, 7);
			if (orig != data)
				WREG32_PIF_PHY1(ixPB1_PIF_PWRDOWN_0, data);

			orig = data = RREG32_PIF_PHY1(ixPB1_PIF_PWRDOWN_1);
			data = REG_SET_FIELD(data, PB0_PIF_PWRDOWN_1, PLL_POWER_STATE_IN_OFF_1, 7);
			data = REG_SET_FIELD(data, PB0_PIF_PWRDOWN_1, PLL_POWER_STATE_IN_TXS2_1, 7);
			if (orig != data)
				WREG32_PIF_PHY1(ixPB1_PIF_PWRDOWN_1, data);

			if ((adev->family != CHIP_OLAND) && (adev->family != CHIP_HAINAN)) {
				orig = data = RREG32_PIF_PHY0(ixPB0_PIF_PWRDOWN_0);
				data &= ~PB0_PIF_PWRDOWN_0__PLL_RAMP_UP_TIME_0_MASK;
				if (orig != data)
					WREG32_PIF_PHY0(ixPB0_PIF_PWRDOWN_0, data);

				orig = data = RREG32_PIF_PHY0(ixPB0_PIF_PWRDOWN_1);
				data &= ~PB0_PIF_PWRDOWN_1__PLL_RAMP_UP_TIME_1_MASK;
				if (orig != data)
					WREG32_PIF_PHY0(ixPB0_PIF_PWRDOWN_1, data);

				orig = data = RREG32_PIF_PHY0(ixPB0_PIF_PWRDOWN_2);
				data &= ~PB0_PIF_PWRDOWN_2__PLL_RAMP_UP_TIME_2_MASK;
				if (orig != data)
					WREG32_PIF_PHY0(ixPB0_PIF_PWRDOWN_2, data);

				orig = data = RREG32_PIF_PHY0(ixPB0_PIF_PWRDOWN_3);
				data &= ~PB0_PIF_PWRDOWN_3__PLL_RAMP_UP_TIME_3_MASK;
				if (orig != data)
					WREG32_PIF_PHY0(ixPB0_PIF_PWRDOWN_3, data);

				orig = data = RREG32_PIF_PHY1(ixPB1_PIF_PWRDOWN_0);
				data &= ~PB0_PIF_PWRDOWN_0__PLL_RAMP_UP_TIME_0_MASK;
				if (orig != data)
					WREG32_PIF_PHY1(ixPB1_PIF_PWRDOWN_0, data);

				orig = data = RREG32_PIF_PHY1(ixPB1_PIF_PWRDOWN_1);
				data &= ~PB0_PIF_PWRDOWN_1__PLL_RAMP_UP_TIME_1_MASK;
				if (orig != data)
					WREG32_PIF_PHY1(ixPB1_PIF_PWRDOWN_1, data);

				orig = data = RREG32_PIF_PHY1(ixPB1_PIF_PWRDOWN_2);
				data &= ~PB0_PIF_PWRDOWN_2__PLL_RAMP_UP_TIME_2_MASK;
				if (orig != data)
					WREG32_PIF_PHY1(ixPB1_PIF_PWRDOWN_2, data);

				orig = data = RREG32_PIF_PHY1(ixPB1_PIF_PWRDOWN_3);
				data &= ~PB0_PIF_PWRDOWN_3__PLL_RAMP_UP_TIME_3_MASK;
				if (orig != data)
					WREG32_PIF_PHY1(ixPB1_PIF_PWRDOWN_3, data);
			}
			orig = data = RREG32_PCIE(ixPCIE_LC_LINK_WIDTH_CNTL);
			data = REG_SET_FIELD(data, PCIE_LC_LINK_WIDTH_CNTL, LC_DYN_LANES_PWR_STATE, 3);
			if (orig != data)
				WREG32_PCIE(ixPCIE_LC_LINK_WIDTH_CNTL, data);

			orig = data = RREG32_PIF_PHY0(ixPB0_PIF_CNTL);
			data &= ~PB0_PIF_CNTL__LS2_EXIT_TIME_MASK;
			if ((adev->family == CHIP_OLAND) || (adev->family == CHIP_HAINAN))
				data = REG_SET_FIELD(data, PB0_PIF_CNTL, LS2_EXIT_TIME, 5);
			if (orig != data)
				WREG32_PIF_PHY0(ixPB0_PIF_CNTL, data);

			orig = data = RREG32_PIF_PHY1(ixPB1_PIF_CNTL);
			data &= ~PB0_PIF_CNTL__LS2_EXIT_TIME_MASK;
			if ((adev->family == CHIP_OLAND) || (adev->family == CHIP_HAINAN))
				data = REG_SET_FIELD(data, PB0_PIF_CNTL, LS2_EXIT_TIME, 5);
			if (orig != data)
				WREG32_PIF_PHY1(ixPB1_PIF_CNTL, data);

			if (!disable_clkreq &&
			    !pci_is_root_bus(adev->pdev->bus)) {
				struct pci_dev *root = adev->pdev->bus->self;
				u32 lnkcap;

				clk_req_support = false;
				pcie_capability_read_dword(root, PCI_EXP_LNKCAP, &lnkcap);
				if (lnkcap & PCI_EXP_LNKCAP_CLKPM)
					clk_req_support = true;
			} else {
				clk_req_support = false;
			}

			if (clk_req_support) {
				orig = data = RREG32_PCIE(ixPCIE_LC_CNTL2);
				data = REG_SET_FIELD(data, PCIE_LC_CNTL2, LC_ALLOW_PDWN_IN_L1, 1);
				data = REG_SET_FIELD(data, PCIE_LC_CNTL2, LC_ALLOW_PDWN_IN_L23, 1);
				if (orig != data)
					WREG32_PCIE(ixPCIE_LC_CNTL2, data);

				orig = data = RREG32(mmTHM_CLK_CNTL);
				data = REG_SET_FIELD(data, THM_CLK_CNTL, CMON_CLK_SEL, 1);
				data = REG_SET_FIELD(data, THM_CLK_CNTL, TMON_CLK_SEL, 1);
				if (orig != data)
					WREG32(mmTHM_CLK_CNTL, data);

				orig = data = RREG32(mmMISC_CLK_CNTL);
				data = REG_SET_FIELD(data, MISC_CLK_CNTL, DEEP_SLEEP_CLK_SEL, 1);
				data = REG_SET_FIELD(data, MISC_CLK_CNTL, ZCLK_SEL, 1);
				if (orig != data)
					WREG32(mmMISC_CLK_CNTL, data);

				orig = data = RREG32(mmCG_CLKPIN_CNTL);
				data = REG_SET_FIELD(data, CG_CLKPIN_CNTL, BCLK_AS_XCLK, 0);
				if (orig != data)
					WREG32(mmCG_CLKPIN_CNTL, data);

				orig = data = RREG32(mmCG_CLKPIN_CNTL_2);
				data = REG_SET_FIELD(data, CG_CLKPIN_CNTL_2, FORCE_BIF_REFCLK_EN, 0);
				if (orig != data)
					WREG32(mmCG_CLKPIN_CNTL_2, data);

				orig = data = RREG32(mmMPLL_BYPASSCLK_SEL);
				data = REG_SET_FIELD(data, MPLL_BYPASSCLK_SEL, MPLL_CLKOUT_SEL, 4);
				if (orig != data)
					WREG32(mmMPLL_BYPASSCLK_SEL, data);

				orig = data = RREG32(mmSPLL_CNTL_MODE);
				data = REG_SET_FIELD(data, SPLL_CNTL_MODE, SPLL_REFCLK_SEL, 0);
				if (orig != data)
					WREG32(mmSPLL_CNTL_MODE, data);
			}
		}
	} else {
		if (orig != data)
			WREG32_PCIE(ixPCIE_LC_CNTL, data);
	}

	orig = data = RREG32_PCIE(ixPCIE_CNTL2);
	data = REG_SET_FIELD(data, PCIE_CNTL2, SLV_MEM_LS_EN, 1);
	data = REG_SET_FIELD(data, PCIE_CNTL2, MST_MEM_LS_EN, 1);
	data = REG_SET_FIELD(data, PCIE_CNTL2, REPLAY_MEM_LS_EN, 1);
	if (orig != data)
		WREG32_PCIE(ixPCIE_CNTL2, data);

	if (!disable_l0s) {
		data = RREG32_PCIE(ixPCIE_LC_N_FTS_CNTL);
		if((data & REG_FIELD_MASK(PCIE_LC_N_FTS_CNTL, LC_N_FTS)) == REG_FIELD_MASK(PCIE_LC_N_FTS_CNTL, LC_N_FTS)) {
			data = RREG32_PCIE(ixPCIE_LC_STATUS1);
			if (REG_GET_FIELD(data, PCIE_LC_STATUS1, LC_REVERSE_XMIT) &&
				REG_GET_FIELD(data, PCIE_LC_STATUS1, LC_REVERSE_RCVR)) {
				orig = data = RREG32_PCIE(ixPCIE_LC_CNTL);
				data = REG_SET_FIELD(data, PCIE_LC_CNTL, LC_L0S_INACTIVITY, 0);
				if (orig != data)
					WREG32_PCIE(ixPCIE_LC_CNTL, data);
			}
		}
	}
}

static const struct amdgpu_ip_block_version kaveri_ip_blocks[] =
{
	/* ORDER MATTERS! */
	{
		.type = AMD_IP_BLOCK_TYPE_COMMON,
		.major = 1,
		.minor = 0,
		.rev = 0,
		.funcs = &si_common_ip_funcs,
	},
	{
		.type = AMD_IP_BLOCK_TYPE_GMC,
		.major = 7,
		.minor = 0,
		.rev = 0,
		.funcs = &gmc_v6_0_ip_funcs,
	},
};

static const struct amdgpu_ip_block_version verde_ip_blocks[] =
{
	/* ORDER MATTERS! */
	{
		.type = AMD_IP_BLOCK_TYPE_COMMON,
		.major = 1,
		.minor = 0,
		.rev = 0,
		.funcs = &si_common_ip_funcs,
	},
	{
		.type = AMD_IP_BLOCK_TYPE_GMC,
		.major = 7,
		.minor = 0,
		.rev = 0,
		.funcs = &gmc_v6_0_ip_funcs,
	},
};

static const struct amdgpu_ip_block_version pitcairn_ip_blocks[] =
{
	/* ORDER MATTERS! */
	{
		.type = AMD_IP_BLOCK_TYPE_COMMON,
		.major = 1,
		.minor = 0,
		.rev = 0,
		.funcs = &si_common_ip_funcs,
	},
	{
		.type = AMD_IP_BLOCK_TYPE_GMC,
		.major = 7,
		.minor = 0,
		.rev = 0,
		.funcs = &gmc_v6_0_ip_funcs,
	},
};

static const struct amdgpu_ip_block_version tahiti_ip_blocks[] =
{
	/* ORDER MATTERS! */
	{
		.type = AMD_IP_BLOCK_TYPE_COMMON,
		.major = 1,
		.minor = 0,
		.rev = 0,
		.funcs = &si_common_ip_funcs,
	},
	{
		.type = AMD_IP_BLOCK_TYPE_GMC,
		.major = 7,
		.minor = 0,
		.rev = 0,
		.funcs = &gmc_v6_0_ip_funcs,
	},
};

static const struct amdgpu_ip_block_version oland_ip_blocks[] =
{
	/* ORDER MATTERS! */
	{
		.type = AMD_IP_BLOCK_TYPE_COMMON,
		.major = 1,
		.minor = 0,
		.rev = 0,
		.funcs = &si_common_ip_funcs,
	},
	{
		.type = AMD_IP_BLOCK_TYPE_GMC,
		.major = 7,
		.minor = 0,
		.rev = 0,
		.funcs = &gmc_v6_0_ip_funcs,
	},
};

static const struct amdgpu_ip_block_version hainan_ip_blocks[] =
{
	/* ORDER MATTERS! */
	{
		.type = AMD_IP_BLOCK_TYPE_COMMON,
		.major = 1,
		.minor = 0,
		.rev = 0,
		.funcs = &si_common_ip_funcs,
	},
	{
		.type = AMD_IP_BLOCK_TYPE_GMC,
		.major = 7,
		.minor = 0,
		.rev = 0,
		.funcs = &gmc_v6_0_ip_funcs,
	},
};

int si_set_ip_blocks(struct amdgpu_device *adev)
{
	switch (adev->asic_type) {
	case CHIP_OLAND:
		adev->ip_blocks = oland_ip_blocks;
		adev->num_ip_blocks = ARRAY_SIZE(oland_ip_blocks);
		break;
	case CHIP_VERDE:
		adev->ip_blocks = verde_ip_blocks;
		adev->num_ip_blocks = ARRAY_SIZE(verde_ip_blocks);
		break;
	case CHIP_PITCAIRN:
		adev->ip_blocks = pitcairn_ip_blocks;
		adev->num_ip_blocks = ARRAY_SIZE(pitcairn_ip_blocks);
		break;
	case CHIP_TAHITI:
		adev->ip_blocks = tahiti_ip_blocks;
		adev->num_ip_blocks = ARRAY_SIZE(tahiti_ip_blocks);
		break;
	case CHIP_HAINAN:
		adev->ip_blocks = hainan_ip_blocks;
		adev->num_ip_blocks = ARRAY_SIZE(hainan_ip_blocks);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static const struct amdgpu_asic_funcs si_asic_funcs =
{
	.read_disabled_bios = &si_read_disabled_bios,
	.read_bios_from_rom = &si_read_bios_from_rom,
	.read_register = &si_read_register,
	.reset = &si_asic_reset,
	.set_vga_state = &si_vga_set_state,
	.get_xclk = &si_get_xclk,
	.set_uvd_clocks = &si_set_uvd_clocks,
	.set_vce_clocks = &si_set_vce_clocks,
	/*.get_cu_info = &gfx_v7_0_get_cu_info,*/

	/* these should be moved to their own ip modules */
	/*.get_gpu_clock_counter = &gfx_v7_0_get_gpu_clock_counter,*/
	.wait_for_mc_idle = &gmc_v6_0_mc_wait_for_idle,
};

static uint32_t si_get_rev_id(struct amdgpu_device *adev)
{
	return REG_GET_FIELD(RREG32(mmMC_SEQ_MISC0), MC_SEQ_MISC0, MC_SEQ_MISC0_REV_ID);
}

static int si_common_early_init(void *handle)
{
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;

	adev->smc_rreg = &si_smc_rreg;
	adev->smc_wreg = &si_smc_wreg;
	adev->pcie_rreg = &si_pcie_rreg;
	adev->pcie_wreg = &si_pcie_wreg;
	adev->uvd_ctx_rreg = &si_uvd_ctx_rreg;
	adev->uvd_ctx_wreg = &si_uvd_ctx_wreg;

	adev->asic_funcs = &si_asic_funcs;

	if (adev->family == CHIP_HAINAN)
		adev->has_uvd = false;
	else
		adev->has_uvd = true;

	adev->rev_id = si_get_rev_id(adev);
	/*Added for debug */ printk(KERN_ALERT "Device revision: %02x\n", adev->rev_id);
	adev->external_rev_id = 0xFF;
	switch (adev->asic_type) {
	case CHIP_TAHITI:
		adev->cg_flags =
			AMDGPU_CG_SUPPORT_GFX_MGCG |
			AMDGPU_CG_SUPPORT_GFX_MGLS |
			/*AMDGPU_CG_SUPPORT_GFX_CGCG |*/
			AMDGPU_CG_SUPPORT_GFX_CGLS |
			AMDGPU_CG_SUPPORT_GFX_CGTS |
			AMDGPU_CG_SUPPORT_GFX_CP_LS |
			AMDGPU_CG_SUPPORT_MC_MGCG |
			AMDGPU_CG_SUPPORT_SDMA_MGCG |
			AMDGPU_CG_SUPPORT_BIF_LS |
			AMDGPU_CG_SUPPORT_VCE_MGCG |
			AMDGPU_CG_SUPPORT_UVD_MGCG |
			AMDGPU_CG_SUPPORT_HDP_LS |
			AMDGPU_CG_SUPPORT_HDP_MGCG;
		adev->pg_flags = 0;
		break;
	case CHIP_PITCAIRN:
		adev->cg_flags =
			AMDGPU_CG_SUPPORT_GFX_MGCG |
			AMDGPU_CG_SUPPORT_GFX_MGLS |
			/*AMDGPU_CG_SUPPORT_GFX_CGCG |*/
			AMDGPU_CG_SUPPORT_GFX_CGLS |
			AMDGPU_CG_SUPPORT_GFX_CGTS |
			AMDGPU_CG_SUPPORT_GFX_CP_LS |
			AMDGPU_CG_SUPPORT_GFX_RLC_LS |
			AMDGPU_CG_SUPPORT_MC_LS |
			AMDGPU_CG_SUPPORT_MC_MGCG |
			AMDGPU_CG_SUPPORT_SDMA_MGCG |
			AMDGPU_CG_SUPPORT_BIF_LS |
			AMDGPU_CG_SUPPORT_VCE_MGCG |
			AMDGPU_CG_SUPPORT_UVD_MGCG |
			AMDGPU_CG_SUPPORT_HDP_LS |
			AMDGPU_CG_SUPPORT_HDP_MGCG;
		adev->pg_flags = 0;
		break;
	case CHIP_VERDE:
		/*Added for debug */ printk(KERN_ALERT "Verde initing.\n");
		adev->cg_flags =
			AMDGPU_CG_SUPPORT_GFX_MGCG |
			AMDGPU_CG_SUPPORT_GFX_MGLS |
			/*AMDGPU_CG_SUPPORT_GFX_CGCG |*/
			AMDGPU_CG_SUPPORT_GFX_CGLS |
			AMDGPU_CG_SUPPORT_GFX_CGTS |
			AMDGPU_CG_SUPPORT_GFX_CP_LS |
			AMDGPU_CG_SUPPORT_GFX_RLC_LS |
			AMDGPU_CG_SUPPORT_MC_LS |
			AMDGPU_CG_SUPPORT_MC_MGCG |
			AMDGPU_CG_SUPPORT_SDMA_MGCG |
			AMDGPU_CG_SUPPORT_BIF_LS |
			AMDGPU_CG_SUPPORT_VCE_MGCG |
			AMDGPU_CG_SUPPORT_UVD_MGCG |
			AMDGPU_CG_SUPPORT_HDP_LS |
			AMDGPU_CG_SUPPORT_HDP_MGCG;
		adev->pg_flags = 0;
		break;
	case CHIP_OLAND:
		/*Added for debug */ printk(KERN_ALERT "OLand initing.\n");
		adev->cg_flags =
			AMDGPU_CG_SUPPORT_GFX_MGCG |
			AMDGPU_CG_SUPPORT_GFX_MGLS |
			/*AMDGPU_CG_SUPPORT_GFX_CGCG |*/
			AMDGPU_CG_SUPPORT_GFX_CGLS |
			AMDGPU_CG_SUPPORT_GFX_CGTS |
			AMDGPU_CG_SUPPORT_GFX_CP_LS |
			AMDGPU_CG_SUPPORT_GFX_RLC_LS |
			AMDGPU_CG_SUPPORT_MC_LS |
			AMDGPU_CG_SUPPORT_MC_MGCG |
			AMDGPU_CG_SUPPORT_SDMA_MGCG |
			AMDGPU_CG_SUPPORT_BIF_LS |
			AMDGPU_CG_SUPPORT_UVD_MGCG |
			AMDGPU_CG_SUPPORT_HDP_LS |
			AMDGPU_CG_SUPPORT_HDP_MGCG;
		adev->pg_flags = 0;
		break;
	case CHIP_HAINAN:
		adev->cg_flags =
			AMDGPU_CG_SUPPORT_GFX_MGCG |
			AMDGPU_CG_SUPPORT_GFX_MGLS |
			/*AMDGPU_CG_SUPPORT_GFX_CGCG |*/
			AMDGPU_CG_SUPPORT_GFX_CGLS |
			AMDGPU_CG_SUPPORT_GFX_CGTS |
			AMDGPU_CG_SUPPORT_GFX_CP_LS |
			AMDGPU_CG_SUPPORT_GFX_RLC_LS |
			AMDGPU_CG_SUPPORT_MC_LS |
			AMDGPU_CG_SUPPORT_MC_MGCG |
			AMDGPU_CG_SUPPORT_SDMA_MGCG |
			AMDGPU_CG_SUPPORT_BIF_LS |
			AMDGPU_CG_SUPPORT_HDP_LS |
			AMDGPU_CG_SUPPORT_HDP_MGCG;
		adev->pg_flags = 0;
		break;
	default:
		/* FIXME: not supported yet */
		return -EINVAL;
	}

	amdgpu_get_pcie_info(adev);

	return 0;
}

static int si_common_sw_init(void *handle)
{
	return 0;
}

static int si_common_sw_fini(void *handle)
{
	return 0;
}

static int si_common_hw_init(void *handle)
{
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;

	/* move the golden regs per IP block */
	si_init_golden_registers(adev);
	/* enable pcie gen2/3 link */
	si_pcie_gen3_enable(adev);
	/* enable aspm */
	si_program_aspm(adev);

	return 0;
}

static int si_common_hw_fini(void *handle)
{
	return 0;
}

static int si_common_suspend(void *handle)
{
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;

	amdgpu_amdkfd_suspend(adev);

	return si_common_hw_fini(adev);
}

static int si_common_resume(void *handle)
{
	int r;
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;

	r = si_common_hw_init(adev);
	if (r)
		return r;

	return amdgpu_amdkfd_resume(adev);
}

static bool si_common_is_idle(void *handle)
{
	return true;
}

static int si_common_wait_for_idle(void *handle)
{
	return 0;
}

static void si_common_print_status(void *handle)
{

}

static int si_common_soft_reset(void *handle)
{
	/* XXX hard reset?? */
	return 0;
}

static int si_common_set_clockgating_state(void *handle,
					    enum amd_clockgating_state state)
{
	return 0;
}

static int si_common_set_powergating_state(void *handle,
					    enum amd_powergating_state state)
{
	return 0;
}

const struct amd_ip_funcs si_common_ip_funcs = {
	.early_init = si_common_early_init,
	.late_init = NULL,
	.sw_init = si_common_sw_init,
	.sw_fini = si_common_sw_fini,
	.hw_init = si_common_hw_init,
	.hw_fini = si_common_hw_fini,
	.suspend = si_common_suspend,
	.resume = si_common_resume,
	.is_idle = si_common_is_idle,
	.wait_for_idle = si_common_wait_for_idle,
	.soft_reset = si_common_soft_reset,
	.print_status = si_common_print_status,
	.set_clockgating_state = si_common_set_clockgating_state,
	.set_powergating_state = si_common_set_powergating_state,
};
