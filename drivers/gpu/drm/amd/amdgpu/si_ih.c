/*
 * Copyright 2012 Advanced Micro Devices, Inc.
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
 /* Driver code adapted from radeon kernel module by Ronie Salgado */
 
#include "drmP.h"
#include "amdgpu.h"
#include "amdgpu_ih.h"
#include "sid.h"
#include "si_reg.h"

#include "dce/dce_6_0_d.h"
#include "dce/dce_6_0_sh_mask.h"
#include "dce/dce_6_0_enum.h"

#include "bif/bif_4_0_d.h"
#include "bif/bif_4_0_enum.h"
#include "bif/bif_4_0_sh_mask.h"

#include "oss/oss_1_0_d.h"
#include "oss/oss_1_0_enum.h"
#include "oss/oss_1_0_sh_mask.h"

#include "gca/gfx_6_0_d.h"
#include "gca/gfx_6_0_enum.h"
#include "gca/gfx_6_0_sh_mask.h"

/*
 * Interrupts
 * Starting with r6xx, interrupts are handled via a ring buffer.
 * Ring buffers are areas of GPU accessible memory that the GPU
 * writes interrupt vectors into and the host reads vectors out of.
 * There is a rptr (read pointer) that determines where the
 * host is currently reading, and a wptr (write pointer)
 * which determines where the GPU has written.  When the
 * pointers are equal, the ring is idle.  When the GPU
 * writes vectors to the ring buffer, it increments the
 * wptr.  When there is an interrupt, the host then starts
 * fetching commands and processing them until the pointers are
 * equal again at which point it updates the rptr.
 */

static void si_ih_set_interrupt_funcs(struct amdgpu_device *adev);

/**
 * si_ih_enable_interrupts - Enable the interrupt ring buffer
 *
 * @adev: amdgpu_device pointer
 *
 * Enable the interrupt ring buffer (SI).
 */
static void si_ih_enable_interrupts(struct amdgpu_device *adev)
{
	u32 ih_cntl = RREG32(mmIH_CNTL);
	u32 ih_rb_cntl = RREG32(mmIH_RB_CNTL);

	ih_cntl |= IH_CNTL__ENABLE_INTR_MASK;
	ih_rb_cntl |= IH_RB_CNTL__IH_RB_ENABLE_MASK;
	WREG32(mmIH_CNTL, ih_cntl);
	WREG32(mmIH_RB_CNTL, ih_rb_cntl);
	adev->irq.ih.enabled = true;
}

/**
 * si_ih_disable_interrupts - Disable the interrupt ring buffer
 *
 * @adev: amdgpu_device pointer
 *
 * Disable the interrupt ring buffer (SI).
 */
static void si_ih_disable_interrupts(struct amdgpu_device *adev)
{
	u32 ih_rb_cntl = RREG32(mmIH_RB_CNTL);
	u32 ih_cntl = RREG32(mmIH_CNTL);

	ih_rb_cntl &= ~IH_RB_CNTL__IH_RB_ENABLE_MASK;
	ih_cntl &= ~IH_CNTL__ENABLE_INTR_MASK;
	WREG32(mmIH_RB_CNTL, ih_rb_cntl);
	WREG32(mmIH_CNTL, ih_cntl);
	/* set rptr, wptr to 0 */
	WREG32(mmIH_RB_RPTR, 0);
	WREG32(mmIH_RB_WPTR, 0);
	adev->irq.ih.enabled = false;
	adev->irq.ih.rptr = 0;
}

static void si_disable_interrupt_state(struct amdgpu_device *adev)
{
	u32 tmp;

	tmp = RREG32(mmCP_INT_CNTL_RING0) &
		(CP_INT_CNTL_RING2__CNTX_BUSY_INT_ENABLE_MASK | CP_INT_CNTL_RING2__CNTX_EMPTY_INT_ENABLE_MASK);
	WREG32(mmCP_INT_CNTL_RING0, tmp);
	WREG32(mmCP_INT_CNTL_RING1, 0);
	WREG32(mmCP_INT_CNTL_RING2, 0);
	tmp = RREG32(mmDMA_CNTL + DMA0_REGISTER_OFFSET) & ~DMA_CNTL__TRAP_ENABLE_MASK;
	WREG32(mmDMA_CNTL + DMA0_REGISTER_OFFSET, tmp);
	tmp = RREG32(mmDMA_CNTL + DMA1_REGISTER_OFFSET) & ~DMA_CNTL__TRAP_ENABLE_MASK;
	WREG32(mmDMA_CNTL + DMA1_REGISTER_OFFSET, tmp);
	WREG32(mmGRBM_INT_CNTL, 0);
	WREG32(mmSRBM_INT_CNTL, 0);
	if (adev->mode_info.num_crtc >= 2) {
		WREG32(mmLB_INT_MASK + CRTC0_REGISTER_OFFSET, 0);
		WREG32(mmLB_INT_MASK + CRTC1_REGISTER_OFFSET, 0);
	}
	if (adev->mode_info.num_crtc >= 4) {
		WREG32(mmLB_INT_MASK + CRTC2_REGISTER_OFFSET, 0);
		WREG32(mmLB_INT_MASK + CRTC3_REGISTER_OFFSET, 0);
	}
	if (adev->mode_info.num_crtc >= 6) {
		WREG32(mmLB_INT_MASK + CRTC4_REGISTER_OFFSET, 0);
		WREG32(mmLB_INT_MASK + CRTC5_REGISTER_OFFSET, 0);
	}

	if (adev->mode_info.num_crtc >= 2) {
		WREG32(mmGRPH_INT_CONTROL + CRTC0_REGISTER_OFFSET, 0);
		WREG32(mmGRPH_INT_CONTROL + CRTC1_REGISTER_OFFSET, 0);
	}
	if (adev->mode_info.num_crtc >= 4) {
		WREG32(mmGRPH_INT_CONTROL + CRTC2_REGISTER_OFFSET, 0);
		WREG32(mmGRPH_INT_CONTROL + CRTC3_REGISTER_OFFSET, 0);
	}
	if (adev->mode_info.num_crtc >= 6) {
		WREG32(mmGRPH_INT_CONTROL + CRTC4_REGISTER_OFFSET, 0);
		WREG32(mmGRPH_INT_CONTROL + CRTC5_REGISTER_OFFSET, 0);
	}

	if (adev->mode_info.num_crtc > 0) {
		WREG32(mmDAC_AUTODETECT_INT_CONTROL, 0);

		tmp = RREG32(mmDC_HPD1_INT_CONTROL) & DC_HPD1_INT_CONTROL__DC_HPD1_INT_POLARITY_MASK;
		WREG32(mmDC_HPD1_INT_CONTROL, tmp);
		tmp = RREG32(mmDC_HPD2_INT_CONTROL) & DC_HPD2_INT_CONTROL__DC_HPD2_INT_POLARITY_MASK;
		WREG32(mmDC_HPD2_INT_CONTROL, tmp);
		tmp = RREG32(mmDC_HPD3_INT_CONTROL) & DC_HPD3_INT_CONTROL__DC_HPD3_INT_POLARITY_MASK;
		WREG32(mmDC_HPD3_INT_CONTROL, tmp);
		tmp = RREG32(mmDC_HPD4_INT_CONTROL) & DC_HPD4_INT_CONTROL__DC_HPD4_INT_POLARITY_MASK;
		WREG32(mmDC_HPD4_INT_CONTROL, tmp);
		tmp = RREG32(mmDC_HPD5_INT_CONTROL) & DC_HPD5_INT_CONTROL__DC_HPD5_INT_POLARITY_MASK;
		WREG32(mmDC_HPD5_INT_CONTROL, tmp);
		tmp = RREG32(mmDC_HPD6_INT_CONTROL) & DC_HPD6_INT_CONTROL__DC_HPD6_INT_POLARITY_MASK;
		WREG32(mmDC_HPD6_INT_CONTROL, tmp);
	}
}

/**
 * si_ih_irq_init - init and enable the interrupt ring
 *
 * @adev: amdgpu_device pointer
 *
 * Allocate a ring buffer for the interrupt controller,
 * enable the RLC, disable interrupts, enable the IH
 * ring buffer and enable it (SI).
 * Called at device load and reume.
 * Returns 0 for success, errors for failure.
 */
static int si_ih_irq_init(struct amdgpu_device *adev)
{
	int ret = 0;
	int rb_bufsz;
	u32 interrupt_cntl, ih_cntl, ih_rb_cntl;
	u64 wptr_off;

	/* disable irqs */
	si_ih_disable_interrupts(adev);

	/* setup interrupt control */
	WREG32(mmINTERRUPT_CNTL2, adev->dummy_page.addr >> 8);
	interrupt_cntl = RREG32(mmINTERRUPT_CNTL);
	/* INTERRUPT_CNTL__IH_DUMMY_RD_OVERRIDE_MASK=0 - dummy read disabled with msi, enabled without msi
	 * INTERRUPT_CNTL__IH_DUMMY_RD_OVERRIDE_MASK=1 - dummy read controlled by IH_DUMMY_RD_EN
	 */
	interrupt_cntl &= ~INTERRUPT_CNTL__IH_DUMMY_RD_OVERRIDE_MASK;
	/* INTERRUPT_CNTL__IH_REQ_NONSNOOP_EN_MASK=1 if ring is in non-cacheable memory, e.g., vram */
	interrupt_cntl &= ~INTERRUPT_CNTL__IH_REQ_NONSNOOP_EN_MASK;
	WREG32(mmINTERRUPT_CNTL, interrupt_cntl);

	WREG32(mmIH_RB_BASE, adev->irq.ih.gpu_addr >> 8);
	rb_bufsz = order_base_2(adev->irq.ih.ring_size / 4);

	ih_rb_cntl = (IH_RB_CNTL__IH_WPTR_OVERFLOW_ENABLE_MASK |
		      IH_RB_CNTL__IH_WPTR_OVERFLOW_CLEAR_MASK |
		      (rb_bufsz << 1));

	ih_rb_cntl |= IH_RB_CNTL__IH_WPTR_WRITEBACK_ENABLE_MASK;

	/* set the writeback address whether it's enabled or not */
	wptr_off = adev->wb.gpu_addr + (adev->irq.ih.wptr_offs * 4);
	WREG32(mmIH_RB_WPTR_ADDR_LO, lower_32_bits(wptr_off) & 0xFFFFFFFC);
	WREG32(mmIH_RB_WPTR_ADDR_HI, upper_32_bits(wptr_off) & 0xFF);

	WREG32(mmIH_RB_CNTL, ih_rb_cntl);

	/* set rptr, wptr to 0 */
	WREG32(mmIH_RB_RPTR, 0);
	WREG32(mmIH_RB_WPTR, 0);

	/* Default settings for IH_CNTL (disabled at first) */
    ih_cntl = MC_WRREQ_CREDIT(0x10) | MC_WR_CLEAN_CNT(0x10) | MC_VMID(0);
	/* IH_CNTL__RPTR_REARM_MASK only works if msi's are enabled */
	if (adev->irq.msi_enabled)
		ih_cntl |= IH_CNTL__RPTR_REARM_MASK;
	WREG32(mmIH_CNTL, ih_cntl);

    /* force the active interrupt state to all disabled */
	/* si_disable_interrupt_state(adev); */

	pci_set_master(adev->pdev);

	/* enable irqs */
	si_ih_enable_interrupts(adev);

	return ret;
}

/**
 * si_ih_irq_disable - disable interrupts
 *
 * @adev: amdgpu_device pointer
 *
 * Disable interrupts on the hw (SI).
 */
static void si_ih_irq_disable(struct amdgpu_device *adev)
{
	si_ih_disable_interrupts(adev);
	/* Wait and acknowledge irq */
	mdelay(1);
}

static inline u32 si_ih_get_wptr(struct amdgpu_device *adev)
{
	u32 wptr, tmp;

	wptr = le32_to_cpu(adev->wb.wb[adev->irq.ih.wptr_offs]);

	if (wptr & IH_RB_WPTR__RB_OVERFLOW_MASK) {
		wptr &= ~IH_RB_WPTR__RB_OVERFLOW_MASK;
		/* When a ring buffer overflow happen start parsing interrupt
		 * from the last not overwritten vector (wptr + 16). Hopefully
		 * this should allow us to catchup.
		 */
		dev_warn(adev->dev, "IH ring buffer overflow (0x%08X, 0x%08X, 0x%08X)\n",
			 wptr, adev->irq.ih.rptr, (wptr + 16) & adev->irq.ih.ptr_mask);
		adev->irq.ih.rptr = (wptr + 16) & adev->irq.ih.ptr_mask;
		tmp = RREG32(mmIH_RB_CNTL);
		tmp |= IH_RB_CNTL__IH_WPTR_OVERFLOW_CLEAR_MASK;
		WREG32(mmIH_RB_CNTL, tmp);
	}
	return (wptr & adev->irq.ih.ptr_mask);
}

/*        SI IV Ring
 * Each IV ring entry is 128 bits:
 * [7:0]    - interrupt source id
 * [31:8]   - reserved
 * [59:32]  - interrupt source data
 * [63:60]  - reserved
 * [71:64]  - RINGID
 * [79:72]  - VMID
 * [127:80] - reserved
 */

 /**
 * si_ih_decode_iv - decode an interrupt vector
 *
 * @adev: amdgpu_device pointer
 *
 * Decodes the interrupt vector at the current rptr
 * position and also advance the position.
 */
static void si_ih_decode_iv(struct amdgpu_device *adev,
                struct amdgpu_iv_entry *entry)
{
   /* wptr/rptr are in bytes! */
   u32 ring_index = adev->irq.ih.rptr >> 2;
   uint32_t dw[4];

   dw[0] = le32_to_cpu(adev->irq.ih.ring[ring_index + 0]);
   dw[1] = le32_to_cpu(adev->irq.ih.ring[ring_index + 1]);
   dw[2] = le32_to_cpu(adev->irq.ih.ring[ring_index + 2]);
   dw[3] = le32_to_cpu(adev->irq.ih.ring[ring_index + 3]);

   entry->src_id = dw[0] & 0xff;
   entry->src_data = dw[1] & 0xfffffff;
   entry->ring_id = dw[2] & 0xff;
   entry->vm_id = (dw[2] >> 8) & 0xff;
   entry->pas_id = (dw[2] >> 16) & 0xffff;

   /* wptr/rptr are in bytes! */
   adev->irq.ih.rptr += 16;

}

/**
 * si_ih_set_rptr - set the IH ring buffer rptr
 *
 * @adev: amdgpu_device pointer
 *
 * Set the IH ring buffer rptr.
 */
static void si_ih_set_rptr(struct amdgpu_device *adev)
{
	WREG32(mmIH_RB_RPTR, adev->irq.ih.rptr);
}

static int si_ih_early_init(void *handle)
{
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;
	int ret;

	ret = amdgpu_irq_add_domain(adev);
	if (ret)
		return ret;

	si_ih_set_interrupt_funcs(adev);

	return 0;
}

static int si_ih_sw_init(void *handle)
{
	int r;
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;

	r = amdgpu_ih_ring_init(adev, 64 * 1024, false);
	if (r)
		return r;

	r = amdgpu_irq_init(adev);

	return r;
}

static int si_ih_sw_fini(void *handle)
{
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;

	amdgpu_irq_fini(adev);
	amdgpu_ih_ring_fini(adev);
	amdgpu_irq_remove_domain(adev);

	return 0;
}

static int si_ih_hw_init(void *handle)
{
	int r;
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;

	r = si_ih_irq_init(adev);
	if (r)
		return r;

	return 0;
}

static int si_ih_hw_fini(void *handle)
{
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;

	si_ih_irq_disable(adev);

	return 0;
}

static int si_ih_suspend(void *handle)
{
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;

	return si_ih_hw_fini(adev);
}

static int si_ih_resume(void *handle)
{
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;

	return si_ih_hw_init(adev);
}

static bool si_ih_is_idle(void *handle)
{
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;
	u32 tmp = RREG32(mmSRBM_STATUS);

	if (tmp & SRBM_STATUS__IH_BUSY_MASK)
		return false;

	return true;
}

static int si_ih_wait_for_idle(void *handle)
{
	unsigned i;
	u32 tmp;
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;

	for (i = 0; i < adev->usec_timeout; i++) {
		/* read MC_STATUS */
		tmp = RREG32(mmSRBM_STATUS) & SRBM_STATUS__IH_BUSY_MASK;
		if (!tmp)
			return 0;
		udelay(1);
	}
	return -ETIMEDOUT;
}

static void si_ih_print_status(void *handle)
{
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;

	dev_info(adev->dev, "CIK IH registers\n");
	dev_info(adev->dev, "  SRBM_STATUS=0x%08X\n",
		RREG32(mmSRBM_STATUS));
	dev_info(adev->dev, "  SRBM_STATUS2=0x%08X\n",
		RREG32(mmSRBM_STATUS2));
	dev_info(adev->dev, "  INTERRUPT_CNTL=0x%08X\n",
		 RREG32(mmINTERRUPT_CNTL));
	dev_info(adev->dev, "  INTERRUPT_CNTL2=0x%08X\n",
		 RREG32(mmINTERRUPT_CNTL2));
	dev_info(adev->dev, "  IH_CNTL=0x%08X\n",
		 RREG32(mmIH_CNTL));
	dev_info(adev->dev, "  IH_RB_CNTL=0x%08X\n",
		 RREG32(mmIH_RB_CNTL));
	dev_info(adev->dev, "  IH_RB_BASE=0x%08X\n",
		 RREG32(mmIH_RB_BASE));
	dev_info(adev->dev, "  IH_RB_WPTR_ADDR_LO=0x%08X\n",
		 RREG32(mmIH_RB_WPTR_ADDR_LO));
	dev_info(adev->dev, "  IH_RB_WPTR_ADDR_HI=0x%08X\n",
		 RREG32(mmIH_RB_WPTR_ADDR_HI));
	dev_info(adev->dev, "  IH_RB_RPTR=0x%08X\n",
		 RREG32(mmIH_RB_RPTR));
	dev_info(adev->dev, "  IH_RB_WPTR=0x%08X\n",
		 RREG32(mmIH_RB_WPTR));
}

static int si_ih_soft_reset(void *handle)
{
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;

	u32 srbm_soft_reset = 0;
	u32 tmp = RREG32(mmSRBM_STATUS);

	if (tmp & SRBM_STATUS__IH_BUSY_MASK)
		srbm_soft_reset |= SRBM_SOFT_RESET__SOFT_RESET_IH_MASK;

	if (srbm_soft_reset) {
		si_ih_print_status((void *)adev);

		tmp = RREG32(mmSRBM_SOFT_RESET);
		tmp |= srbm_soft_reset;
		dev_info(adev->dev, "SRBM_SOFT_RESET=0x%08X\n", tmp);
		WREG32(mmSRBM_SOFT_RESET, tmp);
		tmp = RREG32(mmSRBM_SOFT_RESET);

		udelay(50);

		tmp &= ~srbm_soft_reset;
		WREG32(mmSRBM_SOFT_RESET, tmp);
		tmp = RREG32(mmSRBM_SOFT_RESET);

		/* Wait a little for things to settle down */
		udelay(50);

		si_ih_print_status((void *)adev);
	}

	return 0;
}

static int si_ih_set_clockgating_state(void *handle,
					  enum amd_clockgating_state state)
{
	return 0;
}

static int si_ih_set_powergating_state(void *handle,
					  enum amd_powergating_state state)
{
	return 0;
}

const struct amd_ip_funcs si_ih_ip_funcs = {
	.early_init = si_ih_early_init,
	.late_init = NULL,
	.sw_init = si_ih_sw_init,
	.sw_fini = si_ih_sw_fini,
	.hw_init = si_ih_hw_init,
	.hw_fini = si_ih_hw_fini,
	.suspend = si_ih_suspend,
	.resume = si_ih_resume,
	.is_idle = si_ih_is_idle,
	.wait_for_idle = si_ih_wait_for_idle,
	.soft_reset = si_ih_soft_reset,
	.print_status = si_ih_print_status,
	.set_clockgating_state = si_ih_set_clockgating_state,
	.set_powergating_state = si_ih_set_powergating_state,
};

static const struct amdgpu_ih_funcs si_ih_funcs = {
	.get_wptr = si_ih_get_wptr,
	.decode_iv = si_ih_decode_iv,
	.set_rptr = si_ih_set_rptr
};

static void si_ih_set_interrupt_funcs(struct amdgpu_device *adev)
{
	if (adev->irq.ih_funcs == NULL)
		adev->irq.ih_funcs = &si_ih_funcs;
}
