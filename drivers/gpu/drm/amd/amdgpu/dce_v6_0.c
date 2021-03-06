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
 /* Driver code adapted from radeon kernel module by Ronie Salgado */
#include "drmP.h"
#include "amdgpu.h"
#include "amdgpu_pm.h"
#include "amdgpu_i2c.h"
#include "sid.h"
#include "si_reg.h"
#include "atom.h"
#include "amdgpu_atombios.h"
#include "atombios_crtc.h"
#include "atombios_encoders.h"
#include "amdgpu_pll.h"
#include "amdgpu_connectors.h"

#include "dce/dce_6_0_d.h"
#include "dce/dce_6_0_sh_mask.h"
#include "dce/dce_6_0_enum.h"

#include "gca/gfx_6_0_enum.h"

#include "gmc/gmc_6_0_d.h"
#include "gmc/gmc_6_0_enum.h"
#include "gmc/gmc_6_0_sh_mask.h"

#include "oss/oss_1_0_d.h"
#include "oss/oss_1_0_enum.h"
#include "oss/oss_1_0_sh_mask.h"

static void dce_v6_0_set_display_funcs(struct amdgpu_device *adev);
static void dce_v6_0_set_irq_funcs(struct amdgpu_device *adev);

static const u32 crtc_offsets[6] =
{
	CRTC0_REGISTER_OFFSET,
	CRTC1_REGISTER_OFFSET,
	CRTC2_REGISTER_OFFSET,
	CRTC3_REGISTER_OFFSET,
	CRTC4_REGISTER_OFFSET,
	CRTC5_REGISTER_OFFSET
};

static const uint32_t dig_offsets[] = {
	CRTC0_REGISTER_OFFSET,
	CRTC1_REGISTER_OFFSET,
	CRTC2_REGISTER_OFFSET,
	CRTC3_REGISTER_OFFSET,
	CRTC4_REGISTER_OFFSET,
	CRTC5_REGISTER_OFFSET,
};

static const struct {
	uint32_t	reg;
	uint32_t	vblank;
	uint32_t	vline;
	uint32_t	hpd;

} interrupt_status_offsets[6] = { {
	.reg = mmDISP_INTERRUPT_STATUS,
	.vblank = DISP_INTERRUPT_STATUS__LB_D1_VBLANK_INTERRUPT_MASK,
	.vline = DISP_INTERRUPT_STATUS__LB_D1_VLINE_INTERRUPT_MASK,
	.hpd = DISP_INTERRUPT_STATUS__DC_HPD1_INTERRUPT_MASK
}, {
	.reg = mmDISP_INTERRUPT_STATUS_CONTINUE,
	.vblank = DISP_INTERRUPT_STATUS_CONTINUE__LB_D2_VBLANK_INTERRUPT_MASK,
	.vline = DISP_INTERRUPT_STATUS_CONTINUE__LB_D2_VLINE_INTERRUPT_MASK,
	.hpd = DISP_INTERRUPT_STATUS_CONTINUE__DC_HPD2_INTERRUPT_MASK
}, {
	.reg = mmDISP_INTERRUPT_STATUS_CONTINUE2,
	.vblank = DISP_INTERRUPT_STATUS_CONTINUE2__LB_D3_VBLANK_INTERRUPT_MASK,
	.vline = DISP_INTERRUPT_STATUS_CONTINUE2__LB_D3_VLINE_INTERRUPT_MASK,
	.hpd = DISP_INTERRUPT_STATUS_CONTINUE2__DC_HPD3_INTERRUPT_MASK
}, {
	.reg = mmDISP_INTERRUPT_STATUS_CONTINUE3,
	.vblank = DISP_INTERRUPT_STATUS_CONTINUE3__LB_D4_VBLANK_INTERRUPT_MASK,
	.vline = DISP_INTERRUPT_STATUS_CONTINUE3__LB_D4_VLINE_INTERRUPT_MASK,
	.hpd = DISP_INTERRUPT_STATUS_CONTINUE3__DC_HPD4_INTERRUPT_MASK
}, {
	.reg = mmDISP_INTERRUPT_STATUS_CONTINUE4,
	.vblank = DISP_INTERRUPT_STATUS_CONTINUE4__LB_D5_VBLANK_INTERRUPT_MASK,
	.vline = DISP_INTERRUPT_STATUS_CONTINUE4__LB_D5_VLINE_INTERRUPT_MASK,
	.hpd = DISP_INTERRUPT_STATUS_CONTINUE4__DC_HPD5_INTERRUPT_MASK
}, {
	.reg = mmDISP_INTERRUPT_STATUS_CONTINUE5,
	.vblank = DISP_INTERRUPT_STATUS_CONTINUE5__LB_D6_VBLANK_INTERRUPT_MASK,
	.vline = DISP_INTERRUPT_STATUS_CONTINUE5__LB_D6_VLINE_INTERRUPT_MASK,
	.hpd = DISP_INTERRUPT_STATUS_CONTINUE5__DC_HPD6_INTERRUPT_MASK
} };

static const uint32_t hpd_int_control_offsets[6] = {
	mmDC_HPD1_INT_CONTROL,
	mmDC_HPD2_INT_CONTROL,
	mmDC_HPD3_INT_CONTROL,
	mmDC_HPD4_INT_CONTROL,
	mmDC_HPD5_INT_CONTROL,
	mmDC_HPD6_INT_CONTROL,
};

static u32 dce_v6_0_audio_endpt_rreg(struct amdgpu_device *adev,
				     u32 block_offset, u32 reg)
{
	unsigned long flags;
	u32 r;

	spin_lock_irqsave(&adev->audio_endpt_idx_lock, flags);
	WREG32(mmAZ_F0_CODEC_ENDPOINT_INDEX + block_offset, reg);
	r = RREG32(mmAZ_F0_CODEC_ENDPOINT_DATA + block_offset);
	spin_unlock_irqrestore(&adev->audio_endpt_idx_lock, flags);

	return r;
}

static void dce_v6_0_audio_endpt_wreg(struct amdgpu_device *adev,
				      u32 block_offset, u32 reg, u32 v)
{
	unsigned long flags;

	spin_lock_irqsave(&adev->audio_endpt_idx_lock, flags);
	WREG32(mmAZ_F0_CODEC_ENDPOINT_INDEX + block_offset, reg);
	WREG32(mmAZ_F0_CODEC_ENDPOINT_DATA + block_offset, v);
	spin_unlock_irqrestore(&adev->audio_endpt_idx_lock, flags);
}

static bool dce_v6_0_is_in_vblank(struct amdgpu_device *adev, int crtc)
{
	if (RREG32(mmCRTC_STATUS + crtc_offsets[crtc]) &
			CRTC_STATUS__CRTC_V_BLANK_MASK)
		return true;
	else
		return false;
}

static bool dce_v6_0_is_counter_moving(struct amdgpu_device *adev, int crtc)
{
	u32 pos1, pos2;

	pos1 = RREG32(mmCRTC_STATUS_POSITION + crtc_offsets[crtc]);
	pos2 = RREG32(mmCRTC_STATUS_POSITION + crtc_offsets[crtc]);

	if (pos1 != pos2)
		return true;
	else
		return false;
}

/**
 * dce4_wait_for_vblank - vblank wait asic callback.
 *
 * @adev: amdgpu_device pointer
 * @crtc: crtc to wait for vblank on
 *
 * Wait for vblank on the requested crtc (evergreen+).
 */
static void dce_v6_0_vblank_wait(struct amdgpu_device *adev, int crtc)
{
	unsigned i = 0;

	if (crtc >= adev->mode_info.num_crtc)
		return;

	if (!(RREG32(mmCRTC_CONTROL + crtc_offsets[crtc]) & CRTC_CONTROL__CRTC_MASTER_EN_MASK))
		return;

	/* depending on when we hit vblank, we may be close to active; if so,
	 * wait for another frame.
	 */
	while (dce_v6_0_is_in_vblank(adev, crtc)) {
		if (i++ % 100 == 0) {
			if (!dce_v6_0_is_counter_moving(adev, crtc))
				break;
		}
	}

	while (!dce_v6_0_is_in_vblank(adev, crtc)) {
		if (i++ % 100 == 0) {
			if (!dce_v6_0_is_counter_moving(adev, crtc))
				break;
		}
	}
}

static u32 dce_v6_0_vblank_get_counter(struct amdgpu_device *adev, int crtc)
{
	if (crtc >= adev->mode_info.num_crtc)
		return 0;
	else
		return RREG32(mmCRTC_STATUS_FRAME_COUNT + crtc_offsets[crtc]);
}

static void dce_v6_0_pageflip_interrupt_init(struct amdgpu_device *adev)
{
	unsigned i;

	/* Enable pflip interrupts */
	for (i = 0; i < adev->mode_info.num_crtc; i++)
		amdgpu_irq_get(adev, &adev->pageflip_irq, i);
}

static void dce_v6_0_pageflip_interrupt_fini(struct amdgpu_device *adev)
{
	unsigned i;

	/* Disable pflip interrupts */
	for (i = 0; i < adev->mode_info.num_crtc; i++)
		amdgpu_irq_put(adev, &adev->pageflip_irq, i);
}

/**
 * evergreen_page_flip - pageflip callback.
 *
 * @adev: amdgpu_device pointer
 * @crtc_id: crtc to cleanup pageflip on
 * @crtc_base: new address of the crtc (GPU MC address)
 *
 * Triggers the actual pageflip by updating the primary
 * surface base address (evergreen+).
 */
static void dce_v6_0_page_flip(struct amdgpu_device *adev, int crtc_id, u64 crtc_base)
{
	struct amdgpu_crtc *amdgpu_crtc = adev->mode_info.crtcs[crtc_id];

	/* update the scanout addresses */
	WREG32(mmGRPH_PRIMARY_SURFACE_ADDRESS_HIGH + amdgpu_crtc->crtc_offset,
	       upper_32_bits(crtc_base));
	WREG32(mmGRPH_PRIMARY_SURFACE_ADDRESS + amdgpu_crtc->crtc_offset,
	       (u32)crtc_base);
	/* post the write */
	RREG32(mmGRPH_PRIMARY_SURFACE_ADDRESS + amdgpu_crtc->crtc_offset);
}

/**
 * evergreen_page_flip_pending - check if page flip is still pending
 *
 * @adev: amdgpu_device pointer
 * @crtc_id: crtc to check
 *
 * Returns the current update pending status.
 */
bool dce_v6_0_page_flip_pending(struct amdgpu_device *adev, int crtc_id)
{
	struct amdgpu_crtc *amdgpu_crtc = adev->mode_info.crtcs[crtc_id];

	/* Return current update_pending status: */
	return !!(RREG32(mmGRPH_UPDATE + amdgpu_crtc->crtc_offset) &
		GRPH_UPDATE__GRPH_SURFACE_UPDATE_PENDING_MASK);
}

static int dce_v6_0_crtc_get_scanoutpos(struct amdgpu_device *adev, int crtc,
					u32 *vbl, u32 *position)
{
	if ((crtc < 0) || (crtc >= adev->mode_info.num_crtc))
		return -EINVAL;

	*vbl = RREG32(mmCRTC_V_BLANK_START_END + crtc_offsets[crtc]);
	*position = RREG32(mmCRTC_STATUS_POSITION + crtc_offsets[crtc]);

	return 0;
}

/**
 * evergreen_hpd_sense - hpd sense callback.
 *
 * @adev: amdgpu_device pointer
 * @hpd: hpd (hotplug detect) pin
 *
 * Checks if a digital monitor is connected (evergreen+).
 * Returns true if connected, false if not connected.
 */
static bool dce_v6_0_hpd_sense(struct amdgpu_device *adev, enum amdgpu_hpd_id hpd)
{
	bool connected = false;

	switch (hpd) {
	case AMDGPU_HPD_1:
		if (RREG32(mmDC_HPD1_INT_STATUS) & DC_HPD1_INT_STATUS__DC_HPD1_SENSE_MASK)
			connected = true;
		break;
	case AMDGPU_HPD_2:
		if (RREG32(mmDC_HPD2_INT_STATUS) & DC_HPD2_INT_STATUS__DC_HPD2_SENSE_MASK)
			connected = true;
		break;
	case AMDGPU_HPD_3:
		if (RREG32(mmDC_HPD3_INT_STATUS) & DC_HPD3_INT_STATUS__DC_HPD3_SENSE_MASK)
			connected = true;
		break;
	case AMDGPU_HPD_4:
		if (RREG32(mmDC_HPD4_INT_STATUS) & DC_HPD4_INT_STATUS__DC_HPD4_SENSE_MASK)
			connected = true;
		break;
	case AMDGPU_HPD_5:
		if (RREG32(mmDC_HPD5_INT_STATUS) & DC_HPD5_INT_STATUS__DC_HPD5_SENSE_MASK)
			connected = true;
		break;
	case AMDGPU_HPD_6:
		if (RREG32(mmDC_HPD6_INT_STATUS) & DC_HPD6_INT_STATUS__DC_HPD6_SENSE_MASK)
			connected = true;
		break;
	default:
		break;
	}

	return connected;
}

/**
 * dce_v6_0_hpd_set_polarity - hpd set polarity callback.
 *
 * @adev: amdgpu_device pointer
 * @hpd: hpd (hotplug detect) pin
 *
 * Set the polarity of the hpd pin (evergreen+).
 */
static void dce_v6_0_hpd_set_polarity(struct amdgpu_device *adev,
				      enum amdgpu_hpd_id hpd)
{
	u32 tmp;
	bool connected = dce_v6_0_hpd_sense(adev, hpd);

	switch (hpd) {
	case AMDGPU_HPD_1:
		tmp = RREG32(mmDC_HPD1_INT_CONTROL);
		if (connected)
			tmp &= ~DC_HPD1_INT_CONTROL__DC_HPD1_INT_POLARITY_MASK;
		else
			tmp |= DC_HPD1_INT_CONTROL__DC_HPD1_INT_POLARITY_MASK;
		WREG32(mmDC_HPD1_INT_CONTROL, tmp);
		break;
	case AMDGPU_HPD_2:
		tmp = RREG32(mmDC_HPD2_INT_CONTROL);
		if (connected)
			tmp &= ~DC_HPD2_INT_CONTROL__DC_HPD2_INT_POLARITY_MASK;
		else
			tmp |= DC_HPD2_INT_CONTROL__DC_HPD2_INT_POLARITY_MASK;
		WREG32(mmDC_HPD2_INT_CONTROL, tmp);
		break;
	case AMDGPU_HPD_3:
		tmp = RREG32(mmDC_HPD3_INT_CONTROL);
		if (connected)
			tmp &= ~DC_HPD3_INT_CONTROL__DC_HPD3_INT_POLARITY_MASK;
		else
			tmp |= DC_HPD3_INT_CONTROL__DC_HPD3_INT_POLARITY_MASK;
		WREG32(mmDC_HPD3_INT_CONTROL, tmp);
		break;
	case AMDGPU_HPD_4:
		tmp = RREG32(mmDC_HPD4_INT_CONTROL);
		if (connected)
			tmp &= ~DC_HPD4_INT_CONTROL__DC_HPD4_INT_POLARITY_MASK;
		else
			tmp |= DC_HPD4_INT_CONTROL__DC_HPD4_INT_POLARITY_MASK;
		WREG32(mmDC_HPD4_INT_CONTROL, tmp);
		break;
	case AMDGPU_HPD_5:
		tmp = RREG32(mmDC_HPD5_INT_CONTROL);
		if (connected)
			tmp &= ~DC_HPD5_INT_CONTROL__DC_HPD5_INT_POLARITY_MASK;
		else
			tmp |= DC_HPD5_INT_CONTROL__DC_HPD5_INT_POLARITY_MASK;
		WREG32(mmDC_HPD5_INT_CONTROL, tmp);
			break;
	case AMDGPU_HPD_6:
		tmp = RREG32(mmDC_HPD6_INT_CONTROL);
		if (connected)
			tmp &= ~DC_HPD6_INT_CONTROL__DC_HPD6_INT_POLARITY_MASK;
		else
			tmp |= DC_HPD6_INT_CONTROL__DC_HPD6_INT_POLARITY_MASK;
		WREG32(mmDC_HPD6_INT_CONTROL, tmp);
		break;
	default:
		break;
	}
}

/**
 * dce_v6_0_hpd_init - hpd setup callback.
 *
 * @adev: amdgpu_device pointer
 *
 * Setup the hpd pins used by the card (evergreen+).
 * Enable the pin, set the polarity, and enable the hpd interrupts.
 */
static void dce_v6_0_hpd_init(struct amdgpu_device *adev)
{
	struct drm_device *dev = adev->ddev;
	struct drm_connector *connector;
	u32 tmp = (0x9c4 << DC_HPD1_CONTROL__DC_HPD1_CONNECTION_TIMER__SHIFT) |
		(0xfa << DC_HPD1_CONTROL__DC_HPD1_RX_INT_TIMER__SHIFT) |
		DC_HPD1_CONTROL__DC_HPD1_EN_MASK;

	list_for_each_entry(connector, &dev->mode_config.connector_list, head) {
		struct amdgpu_connector *amdgpu_connector = to_amdgpu_connector(connector);

		if (connector->connector_type == DRM_MODE_CONNECTOR_eDP ||
		    connector->connector_type == DRM_MODE_CONNECTOR_LVDS) {
			/* don't try to enable hpd on eDP or LVDS avoid breaking the
			 * aux dp channel on imac and help (but not completely fix)
			 * https://bugzilla.redhat.com/show_bug.cgi?id=726143
			 * also avoid interrupt storms during dpms.
			 */
			continue;
		}
		switch (amdgpu_connector->hpd.hpd) {
		case AMDGPU_HPD_1:
			WREG32(mmDC_HPD1_CONTROL, tmp);
			break;
		case AMDGPU_HPD_2:
			WREG32(mmDC_HPD2_CONTROL, tmp);
			break;
		case AMDGPU_HPD_3:
			WREG32(mmDC_HPD3_CONTROL, tmp);
			break;
		case AMDGPU_HPD_4:
			WREG32(mmDC_HPD4_CONTROL, tmp);
			break;
		case AMDGPU_HPD_5:
			WREG32(mmDC_HPD5_CONTROL, tmp);
			break;
		case AMDGPU_HPD_6:
			WREG32(mmDC_HPD6_CONTROL, tmp);
			break;
		default:
			break;
		}
		dce_v6_0_hpd_set_polarity(adev, amdgpu_connector->hpd.hpd);
		amdgpu_irq_get(adev, &adev->hpd_irq, amdgpu_connector->hpd.hpd);
	}
}

/**
 * dce_v6_0_hpd_fini - hpd tear down callback.
 *
 * @adev: amdgpu_device pointer
 *
 * Tear down the hpd pins used by the card (evergreen+).
 * Disable the hpd interrupts.
 */
static void dce_v6_0_hpd_fini(struct amdgpu_device *adev)
{
	struct drm_device *dev = adev->ddev;
	struct drm_connector *connector;

	list_for_each_entry(connector, &dev->mode_config.connector_list, head) {
		struct amdgpu_connector *amdgpu_connector = to_amdgpu_connector(connector);

		switch (amdgpu_connector->hpd.hpd) {
		case AMDGPU_HPD_1:
			WREG32(mmDC_HPD1_CONTROL, 0);
			break;
		case AMDGPU_HPD_2:
			WREG32(mmDC_HPD2_CONTROL, 0);
			break;
		case AMDGPU_HPD_3:
			WREG32(mmDC_HPD3_CONTROL, 0);
			break;
		case AMDGPU_HPD_4:
			WREG32(mmDC_HPD4_CONTROL, 0);
			break;
		case AMDGPU_HPD_5:
			WREG32(mmDC_HPD5_CONTROL, 0);
			break;
		case AMDGPU_HPD_6:
			WREG32(mmDC_HPD6_CONTROL, 0);
			break;
		default:
			break;
		}
		amdgpu_irq_put(adev, &adev->hpd_irq, amdgpu_connector->hpd.hpd);
	}
}

static u32 dce_v6_0_hpd_get_gpio_reg(struct amdgpu_device *adev)
{
	return mmDC_GPIO_HPD_A;
}

static bool dce_v6_0_is_display_hung(struct amdgpu_device *adev)
{
	u32 crtc_hung = 0;
	u32 crtc_status[6];
	u32 i, j, tmp;

	for (i = 0; i < adev->mode_info.num_crtc; i++) {
		if (RREG32(mmCRTC_CONTROL + crtc_offsets[i]) & CRTC_CONTROL__CRTC_MASTER_EN_MASK) {
			crtc_status[i] = RREG32(mmCRTC_STATUS_HV_COUNT + crtc_offsets[i]);
			crtc_hung |= (1 << i);
		}
	}

	for (j = 0; j < 10; j++) {
		for (i = 0; i < adev->mode_info.num_crtc; i++) {
			if (crtc_hung & (1 << i)) {
				tmp = RREG32(mmCRTC_STATUS_HV_COUNT + crtc_offsets[i]);
				if (tmp != crtc_status[i])
					crtc_hung &= ~(1 << i);
			}
		}
		if (crtc_hung == 0)
			return false;
		udelay(100);
	}

	return true;
}

static void dce_v6_0_stop_mc_access(struct amdgpu_device *adev,
				    struct amdgpu_mode_mc_save *save)
{
	u32 crtc_enabled, tmp;
	int i;

	save->vga_render_control = RREG32(mmVGA_RENDER_CONTROL);
	save->vga_hdp_control = RREG32(mmVGA_HDP_CONTROL);

	/* disable VGA render */
	tmp = RREG32(mmVGA_RENDER_CONTROL);
	tmp = REG_SET_FIELD(tmp, VGA_RENDER_CONTROL, VGA_VSTATUS_CNTL, 0);
	WREG32(mmVGA_RENDER_CONTROL, tmp);

	/* blank the display controllers */
	for (i = 0; i < adev->mode_info.num_crtc; i++) {
		crtc_enabled = REG_GET_FIELD(RREG32(mmCRTC_CONTROL + crtc_offsets[i]),
					     CRTC_CONTROL, CRTC_MASTER_EN);
		if (crtc_enabled) {
#if 0
			u32 frame_count;
			int j;

			save->crtc_enabled[i] = true;
			tmp = RREG32(mmCRTC_BLANK_CONTROL + crtc_offsets[i]);
			if (REG_GET_FIELD(tmp, CRTC_BLANK_CONTROL, CRTC_BLANK_DATA_EN) == 0) {
				amdgpu_display_vblank_wait(adev, i);
				WREG32(mmCRTC_UPDATE_LOCK + crtc_offsets[i], 1);
				tmp = REG_SET_FIELD(tmp, CRTC_BLANK_CONTROL, CRTC_BLANK_DATA_EN, 1);
				WREG32(mmCRTC_BLANK_CONTROL + crtc_offsets[i], tmp);
				WREG32(mmCRTC_UPDATE_LOCK + crtc_offsets[i], 0);
			}
			/* wait for the next frame */
			frame_count = amdgpu_display_vblank_get_counter(adev, i);
			for (j = 0; j < adev->usec_timeout; j++) {
				if (amdgpu_display_vblank_get_counter(adev, i) != frame_count)
					break;
				udelay(1);
			}
			tmp = RREG32(mmGRPH_UPDATE + crtc_offsets[i]);
			if (REG_GET_FIELD(tmp, GRPH_UPDATE, GRPH_UPDATE_LOCK) == 0) {
				tmp = REG_SET_FIELD(tmp, GRPH_UPDATE, GRPH_UPDATE_LOCK, 1);
				WREG32(mmGRPH_UPDATE + crtc_offsets[i], tmp);
			}
			tmp = RREG32(mmMASTER_UPDATE_LOCK + crtc_offsets[i]);
			if (REG_GET_FIELD(tmp, MASTER_UPDATE_LOCK, MASTER_UPDATE_LOCK) == 0) {
				tmp = REG_SET_FIELD(tmp, MASTER_UPDATE_LOCK, MASTER_UPDATE_LOCK, 1);
				WREG32(mmMASTER_UPDATE_LOCK + crtc_offsets[i], tmp);
			}
#else
			/* XXX this is a hack to avoid strange behavior with EFI on certain systems */
			WREG32(mmCRTC_UPDATE_LOCK + crtc_offsets[i], 1);
			tmp = RREG32(mmCRTC_CONTROL + crtc_offsets[i]);
			tmp = REG_SET_FIELD(tmp, CRTC_CONTROL, CRTC_MASTER_EN, 0);
			WREG32(mmCRTC_CONTROL + crtc_offsets[i], tmp);
			WREG32(mmCRTC_UPDATE_LOCK + crtc_offsets[i], 0);
			save->crtc_enabled[i] = false;
			/* ***** */
#endif
		} else {
			save->crtc_enabled[i] = false;
		}
	}
}

static void dce_v6_0_resume_mc_access(struct amdgpu_device *adev,
				      struct amdgpu_mode_mc_save *save)
{
	u32 tmp, frame_count;
	int i, j;

	/* update crtc base addresses */
	for (i = 0; i < adev->mode_info.num_crtc; i++) {
		WREG32(mmGRPH_PRIMARY_SURFACE_ADDRESS_HIGH + crtc_offsets[i],
		       upper_32_bits(adev->mc.vram_start));
		WREG32(mmGRPH_SECONDARY_SURFACE_ADDRESS_HIGH + crtc_offsets[i],
		       upper_32_bits(adev->mc.vram_start));
		WREG32(mmGRPH_PRIMARY_SURFACE_ADDRESS + crtc_offsets[i],
		       (u32)adev->mc.vram_start);
		WREG32(mmGRPH_SECONDARY_SURFACE_ADDRESS + crtc_offsets[i],
		       (u32)adev->mc.vram_start);

		if (save->crtc_enabled[i]) {
			tmp = RREG32(mmMASTER_UPDATE_MODE + crtc_offsets[i]);
			if (REG_GET_FIELD(tmp, MASTER_UPDATE_MODE, MASTER_UPDATE_MODE) != 3) {
				tmp = REG_SET_FIELD(tmp, MASTER_UPDATE_MODE, MASTER_UPDATE_MODE, 3);
				WREG32(mmMASTER_UPDATE_MODE + crtc_offsets[i], tmp);
			}
			tmp = RREG32(mmGRPH_UPDATE + crtc_offsets[i]);
			if (REG_GET_FIELD(tmp, GRPH_UPDATE, GRPH_UPDATE_LOCK)) {
				tmp = REG_SET_FIELD(tmp, GRPH_UPDATE, GRPH_UPDATE_LOCK, 0);
				WREG32(mmGRPH_UPDATE + crtc_offsets[i], tmp);
			}
			tmp = RREG32(mmMASTER_UPDATE_LOCK + crtc_offsets[i]);
			if (REG_GET_FIELD(tmp, MASTER_UPDATE_LOCK, MASTER_UPDATE_LOCK)) {
				tmp = REG_SET_FIELD(tmp, MASTER_UPDATE_LOCK, MASTER_UPDATE_LOCK, 0);
				WREG32(mmMASTER_UPDATE_LOCK + crtc_offsets[i], tmp);
			}
			for (j = 0; j < adev->usec_timeout; j++) {
				tmp = RREG32(mmGRPH_UPDATE + crtc_offsets[i]);
				if (REG_GET_FIELD(tmp, GRPH_UPDATE, GRPH_SURFACE_UPDATE_PENDING) == 0)
					break;
				udelay(1);
			}
			tmp = RREG32(mmCRTC_BLANK_CONTROL + crtc_offsets[i]);
			tmp = REG_SET_FIELD(tmp, CRTC_BLANK_CONTROL, CRTC_BLANK_DATA_EN, 0);
			WREG32(mmCRTC_UPDATE_LOCK + crtc_offsets[i], 1);
			WREG32(mmCRTC_BLANK_CONTROL + crtc_offsets[i], tmp);
			WREG32(mmCRTC_UPDATE_LOCK + crtc_offsets[i], 0);
			/* wait for the next frame */
			frame_count = amdgpu_display_vblank_get_counter(adev, i);
			for (j = 0; j < adev->usec_timeout; j++) {
				if (amdgpu_display_vblank_get_counter(adev, i) != frame_count)
					break;
				udelay(1);
			}
		}
	}

	WREG32(mmVGA_MEMORY_BASE_ADDRESS_HIGH, upper_32_bits(adev->mc.vram_start));
	WREG32(mmVGA_MEMORY_BASE_ADDRESS, lower_32_bits(adev->mc.vram_start));

	/* Unlock vga access */
	WREG32(mmVGA_HDP_CONTROL, save->vga_hdp_control);
	mdelay(1);
	WREG32(mmVGA_RENDER_CONTROL, save->vga_render_control);
}

static void dce_v6_0_set_vga_render_state(struct amdgpu_device *adev,
					  bool render)
{
	u32 tmp;

	/* Lockout access through VGA aperture*/
	tmp = RREG32(mmVGA_HDP_CONTROL);
	if (render)
		tmp = REG_SET_FIELD(tmp, VGA_HDP_CONTROL, VGA_MEMORY_DISABLE, 0);
	else
		tmp = REG_SET_FIELD(tmp, VGA_HDP_CONTROL, VGA_MEMORY_DISABLE, 1);
	WREG32(mmVGA_HDP_CONTROL, tmp);

	/* disable VGA render */
	tmp = RREG32(mmVGA_RENDER_CONTROL);
	if (render)
		tmp = REG_SET_FIELD(tmp, VGA_RENDER_CONTROL, VGA_VSTATUS_CNTL, 1);
	else
		tmp = REG_SET_FIELD(tmp, VGA_RENDER_CONTROL, VGA_VSTATUS_CNTL, 0);
	WREG32(mmVGA_RENDER_CONTROL, tmp);
}

static void dce_v6_0_program_fmt(struct drm_encoder *encoder)
{
	struct drm_device *dev = encoder->dev;
	struct amdgpu_device *adev = dev->dev_private;
	struct amdgpu_encoder *amdgpu_encoder = to_amdgpu_encoder(encoder);
	struct amdgpu_crtc *amdgpu_crtc = to_amdgpu_crtc(encoder->crtc);
	struct drm_connector *connector = amdgpu_get_connector_for_encoder(encoder);
	int bpc = 0;
	u32 tmp = 0;
	enum amdgpu_connector_dither dither = AMDGPU_FMT_DITHER_DISABLE;

	if (connector) {
		struct amdgpu_connector *amdgpu_connector = to_amdgpu_connector(connector);
		bpc = amdgpu_connector_get_monitor_bpc(connector);
		dither = amdgpu_connector->dither;
	}

	/* LVDS/eDP FMT is set up by atom */
	if (amdgpu_encoder->devices & ATOM_DEVICE_LCD_SUPPORT)
		return;

	/* not needed for analog */
	if ((amdgpu_encoder->encoder_id == ENCODER_OBJECT_ID_INTERNAL_KLDSCP_DAC1) ||
	    (amdgpu_encoder->encoder_id == ENCODER_OBJECT_ID_INTERNAL_KLDSCP_DAC2))
		return;

	if (bpc == 0)
		return;

	switch (bpc) {
	case 6:
		if (dither == AMDGPU_FMT_DITHER_ENABLE)
			/* XXX sort out optimal dither settings */
			tmp |= (FMT_BIT_DEPTH_CONTROL__FMT_FRAME_RANDOM_ENABLE_MASK |
				FMT_BIT_DEPTH_CONTROL__FMT_HIGHPASS_RANDOM_ENABLE_MASK |
				FMT_BIT_DEPTH_CONTROL__FMT_SPATIAL_DITHER_EN_MASK |
				(0 << FMT_BIT_DEPTH_CONTROL__FMT_SPATIAL_DITHER_DEPTH__SHIFT));
		else
			tmp |= (FMT_BIT_DEPTH_CONTROL__FMT_TRUNCATE_EN_MASK |
			(0 << FMT_BIT_DEPTH_CONTROL__FMT_TRUNCATE_DEPTH__SHIFT));
		break;
	case 8:
		if (dither == AMDGPU_FMT_DITHER_ENABLE)
			/* XXX sort out optimal dither settings */
			tmp |= (FMT_BIT_DEPTH_CONTROL__FMT_FRAME_RANDOM_ENABLE_MASK |
				FMT_BIT_DEPTH_CONTROL__FMT_HIGHPASS_RANDOM_ENABLE_MASK |
				FMT_BIT_DEPTH_CONTROL__FMT_RGB_RANDOM_ENABLE_MASK |
				FMT_BIT_DEPTH_CONTROL__FMT_SPATIAL_DITHER_EN_MASK |
				(1 << FMT_BIT_DEPTH_CONTROL__FMT_SPATIAL_DITHER_DEPTH__SHIFT));
		else
			tmp |= (FMT_BIT_DEPTH_CONTROL__FMT_TRUNCATE_EN_MASK |
			(1 << FMT_BIT_DEPTH_CONTROL__FMT_TRUNCATE_DEPTH__SHIFT));
		break;
	case 10:
		if (dither == AMDGPU_FMT_DITHER_ENABLE)
			/* XXX sort out optimal dither settings */
			tmp |= (FMT_BIT_DEPTH_CONTROL__FMT_FRAME_RANDOM_ENABLE_MASK |
				FMT_BIT_DEPTH_CONTROL__FMT_HIGHPASS_RANDOM_ENABLE_MASK |
				FMT_BIT_DEPTH_CONTROL__FMT_RGB_RANDOM_ENABLE_MASK |
				FMT_BIT_DEPTH_CONTROL__FMT_SPATIAL_DITHER_EN_MASK |
				(2 << FMT_BIT_DEPTH_CONTROL__FMT_SPATIAL_DITHER_DEPTH__SHIFT));
		else
			tmp |= (FMT_BIT_DEPTH_CONTROL__FMT_TRUNCATE_EN_MASK |
			(2 << FMT_BIT_DEPTH_CONTROL__FMT_TRUNCATE_DEPTH__SHIFT));
		break;
	default:
		/* not needed */
		break;
	}

	WREG32(mmFMT_BIT_DEPTH_CONTROL + amdgpu_crtc->crtc_offset, tmp);
}

/* watermark setup */
static u32 dce_v6_0_line_buffer_adjust(struct amdgpu_device *adev,
				   struct amdgpu_crtc *amdgpu_crtc,
				   struct drm_display_mode *mode,
			   	   struct drm_display_mode *other_mode)
{
	u32 tmp, buffer_alloc, i;
	u32 pipe_offset = amdgpu_crtc->crtc_id * 0x20;
	/*
	 * Line Buffer Setup
	 * There are 3 line buffers, each one shared by 2 display controllers.
	 * DC_LB_MEMORY_SPLIT controls how that line buffer is shared between
	 * the display controllers.  The paritioning is done via one of four
	 * preset allocations specified in bits 21:20:
	 *  0 - half lb
	 *  2 - whole lb, other crtc must be disabled
	 */
	/* this can get tricky if we have two large displays on a paired group
	 * of crtcs.  Ideally for multiple large displays we'd assign them to
	 * non-linked crtcs for maximum line buffer allocation.
	 */
	if (amdgpu_crtc->base.enabled && mode) {
		if (other_mode) {
			tmp = 0; /* 1/2 */
			buffer_alloc = 1;
		} else {
			tmp = 2; /* whole */
			buffer_alloc = 2;
		}
	} else {
		tmp = 0;
		buffer_alloc = 0;
	}

	WREG32(mmDC_LB_MEMORY_SPLIT + amdgpu_crtc->crtc_offset,
	       REG_SET_FIELD(0, DC_LB_MEMORY_SPLIT, LB_MEMORY_CONFIG, tmp));

	WREG32(mmPIPE0_DMIF_BUFFER_CONTROL + pipe_offset,
	       REG_SET_FIELD(0, PIPE0_DMIF_BUFFER_CONTROL, DMIF_BUFFERS_ALLOCATED, buffer_alloc));
	for (i = 0; i < adev->usec_timeout; i++) {
		if (RREG32(mmPIPE0_DMIF_BUFFER_CONTROL + pipe_offset) &
		    PIPE0_DMIF_BUFFER_CONTROL__DMIF_BUFFERS_ALLOCATED_COMPLETED_MASK)
			break;
		udelay(1);
	}

	if (amdgpu_crtc->base.enabled && mode) {
		switch (tmp) {
		case 0:
		default:
			return 4096 * 2;
		case 2:
			return 8192 * 2;
		}
	}

	/* controller not enabled, so no lb used */
	return 0;
}

static u32 si_get_number_of_dram_channels(struct amdgpu_device *adev)
{
	u32 tmp = RREG32(mmMC_SHARED_CHMAP);

	switch (REG_GET_FIELD(tmp, MC_SHARED_CHMAP, NOOFCHAN)) {
	case 0:
	default:
		return 1;
	case 1:
		return 2;
	case 2:
		return 4;
	case 3:
		return 8;
	case 4:
		return 3;
	case 5:
		return 6;
	case 6:
		return 10;
	case 7:
		return 12;
	case 8:
		return 16;
	}
}

struct dce6_wm_params {
	u32 dram_channels; /* number of dram channels */
	u32 yclk;          /* bandwidth per dram data pin in kHz */
	u32 sclk;          /* engine clock in kHz */
	u32 disp_clk;      /* display clock in kHz */
	u32 src_width;     /* viewport width */
	u32 active_time;   /* active display time in ns */
	u32 blank_time;    /* blank time in ns */
	bool interlaced;    /* mode is interlaced */
	fixed20_12 vsc;    /* vertical scale ratio */
	u32 num_heads;     /* number of active crtcs */
	u32 bytes_per_pixel; /* bytes per pixel display + overlay */
	u32 lb_size;       /* line buffer allocated to pipe */
	u32 vtaps;         /* vertical scaler taps */
};

static u32 dce_v6_0_dram_bandwidth(struct dce6_wm_params *wm)
{
	/* Calculate raw DRAM Bandwidth */
	fixed20_12 dram_efficiency; /* 0.7 */
	fixed20_12 yclk, dram_channels, bandwidth;
	fixed20_12 a;

	a.full = dfixed_const(1000);
	yclk.full = dfixed_const(wm->yclk);
	yclk.full = dfixed_div(yclk, a);
	dram_channels.full = dfixed_const(wm->dram_channels * 4);
	a.full = dfixed_const(10);
	dram_efficiency.full = dfixed_const(7);
	dram_efficiency.full = dfixed_div(dram_efficiency, a);
	bandwidth.full = dfixed_mul(dram_channels, yclk);
	bandwidth.full = dfixed_mul(bandwidth, dram_efficiency);

	return dfixed_trunc(bandwidth);
}

static u32 dce_v6_0_dram_bandwidth_for_display(struct dce6_wm_params *wm)
{
	/* Calculate DRAM Bandwidth and the part allocated to display. */
	fixed20_12 disp_dram_allocation; /* 0.3 to 0.7 */
	fixed20_12 yclk, dram_channels, bandwidth;
	fixed20_12 a;

	a.full = dfixed_const(1000);
	yclk.full = dfixed_const(wm->yclk);
	yclk.full = dfixed_div(yclk, a);
	dram_channels.full = dfixed_const(wm->dram_channels * 4);
	a.full = dfixed_const(10);
	disp_dram_allocation.full = dfixed_const(3); /* XXX worse case value 0.3 */
	disp_dram_allocation.full = dfixed_div(disp_dram_allocation, a);
	bandwidth.full = dfixed_mul(dram_channels, yclk);
	bandwidth.full = dfixed_mul(bandwidth, disp_dram_allocation);

	return dfixed_trunc(bandwidth);
}

static u32 dce_v6_0_data_return_bandwidth(struct dce6_wm_params *wm)
{
	/* Calculate the display Data return Bandwidth */
	fixed20_12 return_efficiency; /* 0.8 */
	fixed20_12 sclk, bandwidth;
	fixed20_12 a;

	a.full = dfixed_const(1000);
	sclk.full = dfixed_const(wm->sclk);
	sclk.full = dfixed_div(sclk, a);
	a.full = dfixed_const(10);
	return_efficiency.full = dfixed_const(8);
	return_efficiency.full = dfixed_div(return_efficiency, a);
	a.full = dfixed_const(32);
	bandwidth.full = dfixed_mul(a, sclk);
	bandwidth.full = dfixed_mul(bandwidth, return_efficiency);

	return dfixed_trunc(bandwidth);
}

static u32 dce_v6_0_get_dmif_bytes_per_request(struct dce6_wm_params *wm)
{
	return 32;
}

static u32 dce_v6_0_dmif_request_bandwidth(struct dce6_wm_params *wm)
{
	/* Calculate the DMIF Request Bandwidth */
	fixed20_12 disp_clk_request_efficiency; /* 0.8 */
	fixed20_12 disp_clk, sclk, bandwidth;
	fixed20_12 a, b1, b2;
	u32 min_bandwidth;

	a.full = dfixed_const(1000);
	disp_clk.full = dfixed_const(wm->disp_clk);
	disp_clk.full = dfixed_div(disp_clk, a);
	a.full = dfixed_const(dce_v6_0_get_dmif_bytes_per_request(wm) / 2);
	b1.full = dfixed_mul(a, disp_clk);

	a.full = dfixed_const(1000);
	sclk.full = dfixed_const(wm->sclk);
	sclk.full = dfixed_div(sclk, a);
	a.full = dfixed_const(dce_v6_0_get_dmif_bytes_per_request(wm));
	b2.full = dfixed_mul(a, sclk);

	a.full = dfixed_const(10);
	disp_clk_request_efficiency.full = dfixed_const(8);
	disp_clk_request_efficiency.full = dfixed_div(disp_clk_request_efficiency, a);

	min_bandwidth = min(dfixed_trunc(b1), dfixed_trunc(b2));

	a.full = dfixed_const(min_bandwidth);
	bandwidth.full = dfixed_mul(a, disp_clk_request_efficiency);

	return dfixed_trunc(bandwidth);
}

static u32 dce_v6_0_available_bandwidth(struct dce6_wm_params *wm)
{
	/* Calculate the Available bandwidth. Display can use this temporarily but not in average. */
	u32 dram_bandwidth = dce_v6_0_dram_bandwidth(wm);
	u32 data_return_bandwidth = dce_v6_0_data_return_bandwidth(wm);
	u32 dmif_req_bandwidth = dce_v6_0_dmif_request_bandwidth(wm);

	return min(dram_bandwidth, min(data_return_bandwidth, dmif_req_bandwidth));
}

static u32 dce_v6_0_average_bandwidth(struct dce6_wm_params *wm)
{
	/* Calculate the display mode Average Bandwidth
	 * DisplayMode should contain the source and destination dimensions,
	 * timing, etc.
	 */
	fixed20_12 bpp;
	fixed20_12 line_time;
	fixed20_12 src_width;
	fixed20_12 bandwidth;
	fixed20_12 a;

	a.full = dfixed_const(1000);
	line_time.full = dfixed_const(wm->active_time + wm->blank_time);
	line_time.full = dfixed_div(line_time, a);
	bpp.full = dfixed_const(wm->bytes_per_pixel);
	src_width.full = dfixed_const(wm->src_width);
	bandwidth.full = dfixed_mul(src_width, bpp);
	bandwidth.full = dfixed_mul(bandwidth, wm->vsc);
	bandwidth.full = dfixed_div(bandwidth, line_time);

	return dfixed_trunc(bandwidth);
}

static u32 dce_v6_0_latency_watermark(struct dce6_wm_params *wm)
{
	/* First calcualte the latency in ns */
	u32 mc_latency = 2000; /* 2000 ns. */
	u32 available_bandwidth = dce_v6_0_available_bandwidth(wm);
	u32 worst_chunk_return_time = (512 * 8 * 1000) / available_bandwidth;
	u32 cursor_line_pair_return_time = (128 * 4 * 1000) / available_bandwidth;
	u32 dc_latency = 40000000 / wm->disp_clk; /* dc pipe latency */
	u32 other_heads_data_return_time = ((wm->num_heads + 1) * worst_chunk_return_time) +
		(wm->num_heads * cursor_line_pair_return_time);
	u32 latency = mc_latency + other_heads_data_return_time + dc_latency;
	u32 max_src_lines_per_dst_line, lb_fill_bw, line_fill_time;
	u32 tmp, dmif_size = 12288;
	fixed20_12 a, b, c;

	if (wm->num_heads == 0)
		return 0;

	a.full = dfixed_const(2);
	b.full = dfixed_const(1);
	if ((wm->vsc.full > a.full) ||
	    ((wm->vsc.full > b.full) && (wm->vtaps >= 3)) ||
	    (wm->vtaps >= 5) ||
	    ((wm->vsc.full >= a.full) && wm->interlaced))
		max_src_lines_per_dst_line = 4;
	else
		max_src_lines_per_dst_line = 2;

	a.full = dfixed_const(available_bandwidth);
	b.full = dfixed_const(wm->num_heads);
	a.full = dfixed_div(a, b);

	b.full = dfixed_const(mc_latency + 512);
	c.full = dfixed_const(wm->disp_clk);
	b.full = dfixed_div(b, c);

	c.full = dfixed_const(dmif_size);
	b.full = dfixed_div(c, b);

	tmp = min(dfixed_trunc(a), dfixed_trunc(b));

	b.full = dfixed_const(1000);
	c.full = dfixed_const(wm->disp_clk);
	b.full = dfixed_div(c, b);
	c.full = dfixed_const(wm->bytes_per_pixel);
	b.full = dfixed_mul(b, c);

	lb_fill_bw = min(tmp, dfixed_trunc(b));

	a.full = dfixed_const(max_src_lines_per_dst_line * wm->src_width * wm->bytes_per_pixel);
	b.full = dfixed_const(1000);
	c.full = dfixed_const(lb_fill_bw);
	b.full = dfixed_div(c, b);
	a.full = dfixed_div(a, b);
	line_fill_time = dfixed_trunc(a);

	if (line_fill_time < wm->active_time)
		return latency;
	else
		return latency + (line_fill_time - wm->active_time);

}

static bool dce_v6_0_average_bandwidth_vs_dram_bandwidth_for_display(struct dce6_wm_params *wm)
{
	if (dce_v6_0_average_bandwidth(wm) <=
	    (dce_v6_0_dram_bandwidth_for_display(wm) / wm->num_heads))
		return true;
	else
		return false;
};

static bool dce_v6_0_average_bandwidth_vs_available_bandwidth(struct dce6_wm_params *wm)
{
	if (dce_v6_0_average_bandwidth(wm) <=
	    (dce_v6_0_available_bandwidth(wm) / wm->num_heads))
		return true;
	else
		return false;
};

static bool dce_v6_0_check_latency_hiding(struct dce6_wm_params *wm)
{
	u32 lb_partitions = wm->lb_size / wm->src_width;
	u32 line_time = wm->active_time + wm->blank_time;
	u32 latency_tolerant_lines;
	u32 latency_hiding;
	fixed20_12 a;

	a.full = dfixed_const(1);
	if (wm->vsc.full > a.full)
		latency_tolerant_lines = 1;
	else {
		if (lb_partitions <= (wm->vtaps + 1))
			latency_tolerant_lines = 1;
		else
			latency_tolerant_lines = 2;
	}

	latency_hiding = (latency_tolerant_lines * line_time + wm->blank_time);

	if (dce_v6_0_latency_watermark(wm) <= latency_hiding)
		return true;
	else
		return false;
}

static void dce_v6_0_program_watermarks(struct amdgpu_device *adev,
					 struct amdgpu_crtc *amdgpu_crtc,
					 u32 lb_size, u32 num_heads)
{
	struct drm_display_mode *mode = &amdgpu_crtc->base.mode;
	struct dce6_wm_params wm_low, wm_high;
	u32 dram_channels;
	u32 pixel_period;
	u32 line_time = 0;
	u32 latency_watermark_a = 0, latency_watermark_b = 0;
	u32 priority_a_mark = 0, priority_b_mark = 0;
	u32 priority_a_cnt = PRIORITY_A_CNT__PRIORITY_OFF_MASK;
	u32 priority_b_cnt = PRIORITY_A_CNT__PRIORITY_OFF_MASK;
	u32 tmp, arb_control3;
	fixed20_12 a, b, c;

	if (amdgpu_crtc->base.enabled && num_heads && mode) {
		pixel_period = 1000000 / (u32)mode->clock;
		line_time = min((u32)mode->crtc_htotal * pixel_period, (u32)65535);
		priority_a_cnt = 0;
		priority_b_cnt = 0;

		dram_channels = si_get_number_of_dram_channels(adev);

		/* watermark for high clocks */
		if (adev->pm.dpm_enabled) {
			wm_high.yclk =
				amdgpu_dpm_get_mclk(adev, false) * 10;
			wm_high.sclk =
				amdgpu_dpm_get_sclk(adev, false) * 10;
		} else {
			wm_high.yclk = adev->pm.current_mclk * 10;
			wm_high.sclk = adev->pm.current_sclk * 10;
		}

		wm_high.disp_clk = mode->clock;
		wm_high.src_width = mode->crtc_hdisplay;
		wm_high.active_time = mode->crtc_hdisplay * pixel_period;
		wm_high.blank_time = line_time - wm_high.active_time;
		wm_high.interlaced = false;
		if (mode->flags & DRM_MODE_FLAG_INTERLACE)
			wm_high.interlaced = true;
		wm_high.vsc = amdgpu_crtc->vsc;
		wm_high.vtaps = 1;
		if (amdgpu_crtc->rmx_type != RMX_OFF)
			wm_high.vtaps = 2;
		wm_high.bytes_per_pixel = 4; /* XXX: get this from fb config */
		wm_high.lb_size = lb_size;
		wm_high.dram_channels = dram_channels;
		wm_high.num_heads = num_heads;

		/* watermark for low clocks */
		if (adev->pm.dpm_enabled) {
			wm_low.yclk =
				amdgpu_dpm_get_mclk(adev, true) * 10;
			wm_low.sclk =
				amdgpu_dpm_get_sclk(adev, true) * 10;
		} else {
			wm_low.yclk = adev->pm.current_mclk * 10;
			wm_low.sclk = adev->pm.current_sclk * 10;
		}

		wm_low.disp_clk = mode->clock;
		wm_low.src_width = mode->crtc_hdisplay;
		wm_low.active_time = mode->crtc_hdisplay * pixel_period;
		wm_low.blank_time = line_time - wm_low.active_time;
		wm_low.interlaced = false;
		if (mode->flags & DRM_MODE_FLAG_INTERLACE)
			wm_low.interlaced = true;
		wm_low.vsc = amdgpu_crtc->vsc;
		wm_low.vtaps = 1;
		if (amdgpu_crtc->rmx_type != RMX_OFF)
			wm_low.vtaps = 2;
		wm_low.bytes_per_pixel = 4; /* XXX: get this from fb config */
		wm_low.lb_size = lb_size;
		wm_low.dram_channels = dram_channels;
		wm_low.num_heads = num_heads;

		/* set for high clocks */
		latency_watermark_a = min(dce_v6_0_latency_watermark(&wm_high), (u32)65535);
		/* set for low clocks */
		latency_watermark_b = min(dce_v6_0_latency_watermark(&wm_low), (u32)65535);

		/* possibly force display priority to high */
		/* should really do this at mode validation time... */
		if (!dce_v6_0_average_bandwidth_vs_dram_bandwidth_for_display(&wm_high) ||
		    !dce_v6_0_average_bandwidth_vs_available_bandwidth(&wm_high) ||
		    !dce_v6_0_check_latency_hiding(&wm_high) ||
		    (adev->mode_info.disp_priority == 2)) {
			DRM_DEBUG_KMS("force priority to high\n");
			priority_a_cnt |= PRIORITY_A_CNT__PRIORITY_ALWAYS_ON_MASK;
			priority_b_cnt |= PRIORITY_A_CNT__PRIORITY_ALWAYS_ON_MASK;
		}
		if (!dce_v6_0_average_bandwidth_vs_dram_bandwidth_for_display(&wm_low) ||
		    !dce_v6_0_average_bandwidth_vs_available_bandwidth(&wm_low) ||
		    !dce_v6_0_check_latency_hiding(&wm_low) ||
		    (adev->mode_info.disp_priority == 2)) {
			DRM_DEBUG_KMS("force priority to high\n");
			priority_a_cnt |= PRIORITY_A_CNT__PRIORITY_ALWAYS_ON_MASK;
			priority_b_cnt |= PRIORITY_A_CNT__PRIORITY_ALWAYS_ON_MASK;
		}

		a.full = dfixed_const(1000);
		b.full = dfixed_const(mode->clock);
		b.full = dfixed_div(b, a);
		c.full = dfixed_const(latency_watermark_a);
		c.full = dfixed_mul(c, b);
		c.full = dfixed_mul(c, amdgpu_crtc->hsc);
		c.full = dfixed_div(c, a);
		a.full = dfixed_const(16);
		c.full = dfixed_div(c, a);
		priority_a_mark = dfixed_trunc(c);
		priority_a_cnt |= priority_a_mark & PRIORITY_A_CNT__PRIORITY_MARK_MASK;

		a.full = dfixed_const(1000);
		b.full = dfixed_const(mode->clock);
		b.full = dfixed_div(b, a);
		c.full = dfixed_const(latency_watermark_b);
		c.full = dfixed_mul(c, b);
		c.full = dfixed_mul(c, amdgpu_crtc->hsc);
		c.full = dfixed_div(c, a);
		a.full = dfixed_const(16);
		c.full = dfixed_div(c, a);
		priority_b_mark = dfixed_trunc(c);
		priority_b_cnt |= priority_b_mark & PRIORITY_A_CNT__PRIORITY_MARK_MASK;

		/* Save number of lines the linebuffer leads before the scanout */
		amdgpu_crtc->lb_vblank_lead_lines = DIV_ROUND_UP(lb_size, mode->crtc_hdisplay);
	}

	/* select wm A */
	arb_control3 = RREG32(mmDPG_PIPE_ARBITRATION_CONTROL3 + amdgpu_crtc->crtc_offset);
	tmp = arb_control3;
	tmp = REG_SET_FIELD(tmp, DPG_PIPE_ARBITRATION_CONTROL3, LATENCY_WATERMARK, 3);
	WREG32(mmDPG_PIPE_ARBITRATION_CONTROL3 + amdgpu_crtc->crtc_offset, tmp);

	tmp = REG_SET_FIELD(0, DPG_PIPE_LATENCY_CONTROL, LATENCY_LOW_WATERMARK, latency_watermark_a);
	tmp = REG_SET_FIELD(tmp, DPG_PIPE_LATENCY_CONTROL, LATENCY_HIGH_WATERMARK, line_time);
	WREG32(mmDPG_PIPE_LATENCY_CONTROL + amdgpu_crtc->crtc_offset, tmp);

	/* select wm B */
	tmp = RREG32(mmDPG_PIPE_ARBITRATION_CONTROL3 + amdgpu_crtc->crtc_offset);
	tmp = REG_SET_FIELD(tmp, DPG_PIPE_ARBITRATION_CONTROL3, LATENCY_WATERMARK, 2);
	WREG32(mmDPG_PIPE_ARBITRATION_CONTROL3 + amdgpu_crtc->crtc_offset, tmp);

	tmp = REG_SET_FIELD(0, DPG_PIPE_LATENCY_CONTROL, LATENCY_LOW_WATERMARK, latency_watermark_b);
	tmp = REG_SET_FIELD(tmp, DPG_PIPE_LATENCY_CONTROL, LATENCY_HIGH_WATERMARK, line_time);
	WREG32(mmDPG_PIPE_LATENCY_CONTROL + amdgpu_crtc->crtc_offset, tmp);

	/* restore original selection */
	WREG32(mmDPG_PIPE_ARBITRATION_CONTROL3 + amdgpu_crtc->crtc_offset, arb_control3);

	/* write the priority marks */
	WREG32(mmPRIORITY_A_CNT + amdgpu_crtc->crtc_offset, priority_a_cnt);
	WREG32(mmPRIORITY_B_CNT + amdgpu_crtc->crtc_offset, priority_b_cnt);

	/* save values for DPM */
	amdgpu_crtc->line_time = line_time;
	amdgpu_crtc->wm_high = latency_watermark_a;
	amdgpu_crtc->wm_low = latency_watermark_b;
}

void dce_v6_0_bandwidth_update(struct amdgpu_device *adev)
{
	struct drm_display_mode *mode0 = NULL;
	struct drm_display_mode *mode1 = NULL;
	u32 num_heads = 0, lb_size;
	int i;

	if (!adev->mode_info.mode_config_initialized)
		return;

	amdgpu_update_display_priority(adev);

	for (i = 0; i < adev->mode_info.num_crtc; i++) {
		if (adev->mode_info.crtcs[i]->base.enabled)
			num_heads++;
	}
	for (i = 0; i < adev->mode_info.num_crtc; i += 2) {
		mode0 = &adev->mode_info.crtcs[i]->base.mode;
		mode1 = &adev->mode_info.crtcs[i+1]->base.mode;
		lb_size = dce_v6_0_line_buffer_adjust(adev, adev->mode_info.crtcs[i], mode0, mode1);
		dce_v6_0_program_watermarks(adev, adev->mode_info.crtcs[i], lb_size, num_heads);
		lb_size = dce_v6_0_line_buffer_adjust(adev, adev->mode_info.crtcs[i+1], mode1, mode0);
		dce_v6_0_program_watermarks(adev, adev->mode_info.crtcs[i+1], lb_size, num_heads);
	}
}

static const u32 vga_control_regs[6] =
{
	mmD1VGA_CONTROL,
	mmD2VGA_CONTROL,
	mmD3VGA_CONTROL,
	mmD4VGA_CONTROL,
	mmD5VGA_CONTROL,
	mmD6VGA_CONTROL,
};

static void dce_v6_0_vga_enable(struct drm_crtc *crtc, bool enable)
{
	struct amdgpu_crtc *amdgpu_crtc = to_amdgpu_crtc(crtc);
	struct drm_device *dev = crtc->dev;
	struct amdgpu_device *adev = dev->dev_private;
	u32 vga_control;

	vga_control = RREG32(vga_control_regs[amdgpu_crtc->crtc_id]) & ~1;
	if (enable)
		WREG32(vga_control_regs[amdgpu_crtc->crtc_id], vga_control | 1);
	else
		WREG32(vga_control_regs[amdgpu_crtc->crtc_id], vga_control);
}

static void dce_v6_0_grph_enable(struct drm_crtc *crtc, bool enable)
{
	struct amdgpu_crtc *amdgpu_crtc = to_amdgpu_crtc(crtc);
	struct drm_device *dev = crtc->dev;
	struct amdgpu_device *adev = dev->dev_private;

	if (enable)
		WREG32(mmGRPH_ENABLE + amdgpu_crtc->crtc_offset, 1);
	else
		WREG32(mmGRPH_ENABLE + amdgpu_crtc->crtc_offset, 0);
}

static int dce_v6_0_crtc_do_set_base(struct drm_crtc *crtc,
				 struct drm_framebuffer *fb,
				 int x, int y, int atomic)
{
	struct amdgpu_crtc *amdgpu_crtc = to_amdgpu_crtc(crtc);
	struct drm_device *dev = crtc->dev;
	struct amdgpu_device *adev = dev->dev_private;
	struct amdgpu_framebuffer *amdgpu_fb;
	struct drm_framebuffer *target_fb;
	struct drm_gem_object *obj;
	struct amdgpu_bo *rbo;
	uint64_t fb_location, tiling_flags;
	uint32_t fb_format, fb_pitch_pixels;
	u32 fb_swap = REG_SET_FIELD(0, GRPH_SWAP_CONTROL, GRPH_ENDIAN_SWAP, GRPH_SWAP_CONTROL__GRPH_ENDIAN_NONE);
	u32 tmp, viewport_w, viewport_h;
	int r;
	bool bypass_lut = false;

	/* no fb bound */
	if (!atomic && !crtc->primary->fb) {
		DRM_DEBUG_KMS("No FB bound\n");
		return 0;
	}

	if (atomic) {
		amdgpu_fb = to_amdgpu_framebuffer(fb);
		target_fb = fb;
	}
	else {
		amdgpu_fb = to_amdgpu_framebuffer(crtc->primary->fb);
		target_fb = crtc->primary->fb;
	}

	/* If atomic, assume fb object is pinned & idle & fenced and
	 * just update base pointers
	 */
	obj = amdgpu_fb->obj;
	rbo = gem_to_amdgpu_bo(obj);
	r = amdgpu_bo_reserve(rbo, false);
	if (unlikely(r != 0))
		return r;

	if (atomic)
		fb_location = amdgpu_bo_gpu_offset(rbo);
	else {
		r = amdgpu_bo_pin(rbo, AMDGPU_GEM_DOMAIN_VRAM, &fb_location);
		if (unlikely(r != 0)) {
			amdgpu_bo_unreserve(rbo);
			return -EINVAL;
		}
	}

	amdgpu_bo_get_tiling_flags(rbo, &tiling_flags);
	amdgpu_bo_unreserve(rbo);

	switch (target_fb->pixel_format) {
	case DRM_FORMAT_C8:
		fb_format = REG_SET_FIELD(0, GRPH_CONTROL, GRPH_DEPTH, GRPH_CONTROL__GRPH_DEPTH_8BPP);
		fb_format = REG_SET_FIELD(fb_format, GRPH_CONTROL, GRPH_FORMAT, GRPH_CONTROL__GRPH_FORMAT_INDEXED);
		break;
	case DRM_FORMAT_XRGB4444:
	case DRM_FORMAT_ARGB4444:
		fb_format = REG_SET_FIELD(0, GRPH_CONTROL, GRPH_DEPTH, GRPH_CONTROL__GRPH_DEPTH_16BPP);
		fb_format = REG_SET_FIELD(fb_format, GRPH_CONTROL, GRPH_FORMAT, GRPH_CONTROL__GRPH_FORMAT_ARGB4444);

#ifdef __BIG_ENDIAN
		fb_swap = REG_SET_FIELD(0, GRPH_SWAP_CONTROL, GRPH_ENDIAN_SWAP, GRPH_CONTROL__GRPH_ENDIAN_8IN16);
#endif
		break;
	case DRM_FORMAT_XRGB1555:
	case DRM_FORMAT_ARGB1555:
		fb_format = REG_SET_FIELD(0, GRPH_CONTROL, GRPH_DEPTH, GRPH_CONTROL__GRPH_DEPTH_16BPP);
		fb_format = REG_SET_FIELD(fb_format, GRPH_CONTROL, GRPH_FORMAT, GRPH_CONTROL__GRPH_FORMAT_ARGB1555);
#ifdef __BIG_ENDIAN
		fb_swap = REG_SET_FIELD(0, GRPH_SWAP_CONTROL, GRPH_ENDIAN_SWAP, GRPH_CONTROL__GRPH_ENDIAN_8IN16);
#endif
		break;
	case DRM_FORMAT_BGRX5551:
	case DRM_FORMAT_BGRA5551:
		fb_format = REG_SET_FIELD(0, GRPH_CONTROL, GRPH_DEPTH, GRPH_CONTROL__GRPH_DEPTH_16BPP);
		fb_format = REG_SET_FIELD(fb_format, GRPH_CONTROL, GRPH_FORMAT, GRPH_CONTROL__GRPH_FORMAT_BGRA5551);
#ifdef __BIG_ENDIAN
		fb_swap = REG_SET_FIELD(0, GRPH_SWAP_CONTROL, GRPH_ENDIAN_SWAP, GRPH_CONTROL__GRPH_ENDIAN_8IN16);
#endif
		break;
	case DRM_FORMAT_RGB565:
		fb_format = REG_SET_FIELD(0, GRPH_CONTROL, GRPH_DEPTH, GRPH_CONTROL__GRPH_DEPTH_16BPP);
		fb_format = REG_SET_FIELD(fb_format, GRPH_CONTROL, GRPH_FORMAT, GRPH_CONTROL__GRPH_FORMAT_ARGB565);
#ifdef __BIG_ENDIAN
		fb_swap = REG_SET_FIELD(0, GRPH_SWAP_CONTROL, GRPH_ENDIAN_SWAP, GRPH_CONTROL__GRPH_ENDIAN_8IN16);
#endif
		break;
	case DRM_FORMAT_XRGB8888:
	case DRM_FORMAT_ARGB8888:
		fb_format = REG_SET_FIELD(0, GRPH_CONTROL, GRPH_DEPTH, GRPH_CONTROL__GRPH_DEPTH_32BPP);
		fb_format = REG_SET_FIELD(fb_format, GRPH_CONTROL, GRPH_FORMAT, GRPH_CONTROL__GRPH_FORMAT_ARGB8888);
#ifdef __BIG_ENDIAN
		fb_swap = REG_SET_FIELD(0, GRPH_SWAP_CONTROL, GRPH_ENDIAN_SWAP, GRPH_CONTROL__GRPH_ENDIAN_8IN32);
#endif
		break;
	case DRM_FORMAT_XRGB2101010:
	case DRM_FORMAT_ARGB2101010:
		fb_format = REG_SET_FIELD(0, GRPH_CONTROL, GRPH_DEPTH, GRPH_CONTROL__GRPH_DEPTH_32BPP);
		fb_format = REG_SET_FIELD(fb_format, GRPH_CONTROL, GRPH_FORMAT, GRPH_CONTROL__GRPH_FORMAT_ARGB2101010);
#ifdef __BIG_ENDIAN
		fb_swap = REG_SET_FIELD(0, GRPH_SWAP_CONTROL, GRPH_ENDIAN_SWAP, GRPH_CONTROL__GRPH_ENDIAN_8IN32);
#endif
		/* Greater 8 bpc fb needs to bypass hw-lut to retain precision */
		bypass_lut = true;
		break;
	case DRM_FORMAT_BGRX1010102:
	case DRM_FORMAT_BGRA1010102:
		fb_format = REG_SET_FIELD(0, GRPH_CONTROL, GRPH_DEPTH, GRPH_CONTROL__GRPH_DEPTH_32BPP);
		fb_format = REG_SET_FIELD(fb_format, GRPH_CONTROL, GRPH_FORMAT, GRPH_CONTROL__GRPH_FORMAT_BGRA1010102);
#ifdef __BIG_ENDIAN
		fb_swap = REG_SET_FIELD(0, GRPH_SWAP_CONTROL, GRPH_ENDIAN_SWAP, GRPH_CONTROL__GRPH_ENDIAN_8IN32);
#endif
		/* Greater 8 bpc fb needs to bypass hw-lut to retain precision */
		bypass_lut = true;
		break;
	default:
		DRM_ERROR("Unsupported screen format %s\n",
			  drm_get_format_name(target_fb->pixel_format));
		return -EINVAL;
	}

	if (AMDGPU_TILING_GET(tiling_flags, ARRAY_MODE) == GRPH_CONTROL__GRPH_ARRAY_2D_TILED_THIN1) {
		unsigned bankw, bankh, mtaspect, tile_split, num_banks;

		bankw = AMDGPU_TILING_GET(tiling_flags, BANK_WIDTH);
		bankh = AMDGPU_TILING_GET(tiling_flags, BANK_HEIGHT);
		mtaspect = AMDGPU_TILING_GET(tiling_flags, MACRO_TILE_ASPECT);
		tile_split = AMDGPU_TILING_GET(tiling_flags, TILE_SPLIT);
		num_banks = AMDGPU_TILING_GET(tiling_flags, NUM_BANKS);

		fb_format = REG_SET_FIELD(fb_format, GRPH_CONTROL, GRPH_NUM_BANKS, num_banks);
		fb_format = REG_SET_FIELD(fb_format, GRPH_CONTROL, GRPH_ARRAY_MODE, GRPH_CONTROL__GRPH_ARRAY_2D_TILED_THIN1);
		fb_format = REG_SET_FIELD(fb_format, GRPH_CONTROL, GRPH_TILE_SPLIT, tile_split);
		fb_format = REG_SET_FIELD(fb_format, GRPH_CONTROL, GRPH_BANK_WIDTH, bankw);
		fb_format = REG_SET_FIELD(fb_format, GRPH_CONTROL, GRPH_BANK_HEIGHT, bankh);
		fb_format = REG_SET_FIELD(fb_format, GRPH_CONTROL, GRPH_MACRO_TILE_ASPECT, mtaspect);
	} else if (AMDGPU_TILING_GET(tiling_flags, ARRAY_MODE) == GRPH_CONTROL__GRPH_ARRAY_1D_TILED_THIN1) {
		fb_format = REG_SET_FIELD(fb_format, GRPH_CONTROL, GRPH_ARRAY_MODE, GRPH_CONTROL__GRPH_ARRAY_1D_TILED_THIN1);
	}

	if ((adev->asic_type == CHIP_TAHITI) ||
	   (adev->asic_type == CHIP_PITCAIRN))
		fb_format |= SI_GRPH_PIPE_CONFIG(SI_ADDR_SURF_P8_32x32_8x16);
	else if ((adev->asic_type == CHIP_VERDE) ||
	 (adev->asic_type == CHIP_OLAND) ||
	 (adev->asic_type == CHIP_HAINAN)) /* for completeness.  HAINAN has no display hw */
		fb_format |= SI_GRPH_PIPE_CONFIG(SI_ADDR_SURF_P4_8x16);

	switch (amdgpu_crtc->crtc_id) {
		case 0:
			WREG32(mmD1VGA_CONTROL, 0);
			break;
		case 1:
			WREG32(mmD2VGA_CONTROL, 0);
			break;
		case 2:
			WREG32(mmD3VGA_CONTROL, 0);
			break;
		case 3:
			WREG32(mmD4VGA_CONTROL, 0);
			break;
		case 4:
			WREG32(mmD5VGA_CONTROL, 0);
			break;
		case 5:
			WREG32(mmD6VGA_CONTROL, 0);
			break;
		default:
			break;
	}

	WREG32(mmGRPH_PRIMARY_SURFACE_ADDRESS_HIGH + amdgpu_crtc->crtc_offset,
	       upper_32_bits(fb_location));
	WREG32(mmGRPH_SECONDARY_SURFACE_ADDRESS_HIGH + amdgpu_crtc->crtc_offset,
	       upper_32_bits(fb_location));
	WREG32(mmGRPH_PRIMARY_SURFACE_ADDRESS + amdgpu_crtc->crtc_offset,
	       (u32)fb_location & GRPH_SECONDARY_SURFACE_ADDRESS__GRPH_SURFACE_ADDRESS_MASK);
	WREG32(mmGRPH_SECONDARY_SURFACE_ADDRESS + amdgpu_crtc->crtc_offset,
	       (u32) fb_location & GRPH_SECONDARY_SURFACE_ADDRESS__GRPH_SURFACE_ADDRESS_MASK);
	WREG32(mmGRPH_CONTROL + amdgpu_crtc->crtc_offset, fb_format);
	WREG32(mmGRPH_SWAP_CONTROL + amdgpu_crtc->crtc_offset, fb_swap);

	/*
	 * The LUT only has 256 slots for indexing by a 8 bpc fb. Bypass the LUT
	 * for > 8 bpc scanout to avoid truncation of fb indices to 8 msb's, to
	 * retain the full precision throughout the pipeline.
	 */
	WREG32_P(mmGRPH_LUT_10BIT_BYPASS_CONTROL + amdgpu_crtc->crtc_offset,
		 (bypass_lut ? GRPH_LUT_10BIT_BYPASS_CONTROL__LUT_10BIT_BYPASS_EN_MASK : 0),
		 ~GRPH_LUT_10BIT_BYPASS_CONTROL__LUT_10BIT_BYPASS_EN_MASK);

	if (bypass_lut)
		DRM_DEBUG_KMS("Bypassing hardware LUT due to 10 bit fb scanout.\n");

	WREG32(mmGRPH_SURFACE_OFFSET_X + amdgpu_crtc->crtc_offset, 0);
	WREG32(mmGRPH_SURFACE_OFFSET_Y + amdgpu_crtc->crtc_offset, 0);
	WREG32(mmGRPH_X_START + amdgpu_crtc->crtc_offset, 0);
	WREG32(mmGRPH_Y_START + amdgpu_crtc->crtc_offset, 0);
	WREG32(mmGRPH_X_END + amdgpu_crtc->crtc_offset, target_fb->width);
	WREG32(mmGRPH_Y_END + amdgpu_crtc->crtc_offset, target_fb->height);

	fb_pitch_pixels = target_fb->pitches[0] / (target_fb->bits_per_pixel / 8);
	WREG32(mmGRPH_PITCH + amdgpu_crtc->crtc_offset, fb_pitch_pixels);
	WREG32(mmGRPH_ENABLE + amdgpu_crtc->crtc_offset, 1);

	WREG32(mmDESKTOP_HEIGHT + amdgpu_crtc->crtc_offset,
		       target_fb->height);
	x &= ~3;
	y &= ~1;
	WREG32(mmVIEWPORT_START + amdgpu_crtc->crtc_offset,
	       (x << 16) | y);
	viewport_w = crtc->mode.hdisplay;
	viewport_h = (crtc->mode.vdisplay + 1) & ~1;
	WREG32(mmVIEWPORT_SIZE + amdgpu_crtc->crtc_offset,
	       (viewport_w << 16) | viewport_h);

	/* pageflip setup */
	/* make sure flip is at vb rather than hb */
	tmp = RREG32(mmGRPH_FLIP_CONTROL + amdgpu_crtc->crtc_offset);
	tmp &= ~GRPH_FLIP_CONTROL__GRPH_SURFACE_UPDATE_H_RETRACE_EN_MASK;
	WREG32(mmGRPH_FLIP_CONTROL + amdgpu_crtc->crtc_offset, tmp);

	/* set pageflip to happen only at start of vblank interval (front porch) */
	WREG32(mmMASTER_UPDATE_MODE + amdgpu_crtc->crtc_offset, 3);

	if (!atomic && fb && fb != crtc->primary->fb) {
		amdgpu_fb = to_amdgpu_framebuffer(fb);
		rbo = gem_to_amdgpu_bo(amdgpu_fb->obj);
		r = amdgpu_bo_reserve(rbo, false);
		if (unlikely(r != 0))
			return r;
		amdgpu_bo_unpin(rbo);
		amdgpu_bo_unreserve(rbo);
	}

	/* Bytes per pixel may have changed */
	dce_v6_0_bandwidth_update(adev);

	return 0;
}

static void dce_v6_0_set_interleave(struct drm_crtc *crtc,
				    struct drm_display_mode *mode)
{
	struct drm_device *dev = crtc->dev;
	struct amdgpu_device *adev = dev->dev_private;
	struct amdgpu_crtc *amdgpu_crtc = to_amdgpu_crtc(crtc);

	if (mode->flags & DRM_MODE_FLAG_INTERLACE)
		WREG32(mmLB_DATA_FORMAT + amdgpu_crtc->crtc_offset,
		       LB_DATA_FORMAT__INTERLEAVE_EN_MASK);
	else
		WREG32(mmLB_DATA_FORMAT + amdgpu_crtc->crtc_offset, 0);
}

static void dce_v6_0_crtc_load_lut(struct drm_crtc *crtc)
{
	struct amdgpu_crtc *amdgpu_crtc = to_amdgpu_crtc(crtc);
	struct drm_device *dev = crtc->dev;
	struct amdgpu_device *adev = dev->dev_private;
	int i;

	DRM_DEBUG_KMS("%d\n", amdgpu_crtc->crtc_id);

	WREG32(mmNI_INPUT_CSC_CONTROL + amdgpu_crtc->crtc_offset,
	       (NI_INPUT_CSC_GRPH_MODE(NI_INPUT_CSC_BYPASS) |
		NI_INPUT_CSC_OVL_MODE(NI_INPUT_CSC_BYPASS)));
	WREG32(mmNI_PRESCALE_GRPH_CONTROL + amdgpu_crtc->crtc_offset,
	       NI_GRPH_PRESCALE_BYPASS);
	WREG32(mmNI_PRESCALE_OVL_CONTROL + amdgpu_crtc->crtc_offset,
	       NI_OVL_PRESCALE_BYPASS);
	WREG32(mmNI_INPUT_GAMMA_CONTROL + amdgpu_crtc->crtc_offset,
	       (NI_GRPH_INPUT_GAMMA_MODE(NI_INPUT_GAMMA_USE_LUT) |
		NI_OVL_INPUT_GAMMA_MODE(NI_INPUT_GAMMA_USE_LUT)));

	WREG32(mmDC_LUT_CONTROL + amdgpu_crtc->crtc_offset, 0);

	WREG32(mmDC_LUT_BLACK_OFFSET_BLUE + amdgpu_crtc->crtc_offset, 0);
	WREG32(mmDC_LUT_BLACK_OFFSET_GREEN + amdgpu_crtc->crtc_offset, 0);
	WREG32(mmDC_LUT_BLACK_OFFSET_RED + amdgpu_crtc->crtc_offset, 0);

	WREG32(mmDC_LUT_WHITE_OFFSET_BLUE + amdgpu_crtc->crtc_offset, 0xffff);
	WREG32(mmDC_LUT_WHITE_OFFSET_GREEN + amdgpu_crtc->crtc_offset, 0xffff);
	WREG32(mmDC_LUT_WHITE_OFFSET_RED + amdgpu_crtc->crtc_offset, 0xffff);

	WREG32(mmDC_LUT_RW_MODE + amdgpu_crtc->crtc_offset, 0);
	WREG32(mmDC_LUT_WRITE_EN_MASK + amdgpu_crtc->crtc_offset, 0x00000007);

	WREG32(mmDC_LUT_RW_INDEX + amdgpu_crtc->crtc_offset, 0);
	for (i = 0; i < 256; i++) {
		WREG32(mmDC_LUT_30_COLOR + amdgpu_crtc->crtc_offset,
		       (amdgpu_crtc->lut_r[i] << 20) |
		       (amdgpu_crtc->lut_g[i] << 10) |
		       (amdgpu_crtc->lut_b[i] << 0));
	}

	WREG32(mmNI_DEGAMMA_CONTROL + amdgpu_crtc->crtc_offset,
	       (NI_GRPH_DEGAMMA_MODE(NI_DEGAMMA_BYPASS) |
		NI_OVL_DEGAMMA_MODE(NI_DEGAMMA_BYPASS) |
		NI_ICON_DEGAMMA_MODE(NI_DEGAMMA_BYPASS) |
		NI_CURSOR_DEGAMMA_MODE(NI_DEGAMMA_BYPASS)));
	WREG32(mmNI_GAMUT_REMAP_CONTROL + amdgpu_crtc->crtc_offset,
	       (NI_GRPH_GAMUT_REMAP_MODE(NI_GAMUT_REMAP_BYPASS) |
		NI_OVL_GAMUT_REMAP_MODE(NI_GAMUT_REMAP_BYPASS)));
	WREG32(mmNI_REGAMMA_CONTROL + amdgpu_crtc->crtc_offset,
	       (NI_GRPH_REGAMMA_MODE(NI_REGAMMA_BYPASS) |
		NI_OVL_REGAMMA_MODE(NI_REGAMMA_BYPASS)));
	WREG32(mmNI_OUTPUT_CSC_CONTROL + amdgpu_crtc->crtc_offset,
	       (NI_OUTPUT_CSC_GRPH_MODE(NI_OUTPUT_CSC_BYPASS) |
		NI_OUTPUT_CSC_OVL_MODE(NI_OUTPUT_CSC_BYPASS)));
	/* XXX match this to the depth of the crtc fmt block, move to modeset? */
	WREG32((0x6940/4) + amdgpu_crtc->crtc_offset, 0);
}

static int dce_v6_0_pick_dig_encoder(struct drm_encoder *encoder)
{
	struct amdgpu_encoder *amdgpu_encoder = to_amdgpu_encoder(encoder);
	struct amdgpu_encoder_atom_dig *dig = amdgpu_encoder->enc_priv;

	switch (amdgpu_encoder->encoder_id) {
	case ENCODER_OBJECT_ID_INTERNAL_UNIPHY:
		if (dig->linkb)
			return 1;
		else
			return 0;
		break;
	case ENCODER_OBJECT_ID_INTERNAL_UNIPHY1:
		if (dig->linkb)
			return 3;
		else
			return 2;
		break;
	case ENCODER_OBJECT_ID_INTERNAL_UNIPHY2:
		if (dig->linkb)
			return 5;
		else
			return 4;
		break;
	case ENCODER_OBJECT_ID_INTERNAL_UNIPHY3:
		return 6;
		break;
	default:
		DRM_ERROR("invalid encoder_id: 0x%x\n", amdgpu_encoder->encoder_id);
		return 0;
	}
}

/**
 * amdgpu_atom_pick_pll - Allocate a PPLL for use by the crtc.
 *
 * @crtc: drm crtc
 *
 * Returns the PPLL (Pixel PLL) to be used by the crtc.  For DP monitors
 * a single PPLL can be used for all DP crtcs/encoders.  For non-DP
 * monitors a dedicated PPLL must be used.  If a particular board has
 * an external DP PLL, return ATOM_PPLL_INVALID to skip PLL programming
 * as there is no need to program the PLL itself.  If we are not able to
 * allocate a PLL, return ATOM_PPLL_INVALID to skip PLL programming to
 * avoid messing up an existing monitor.
 *
 * Asic specific PLL information
 *
 * DCE 6.1
 * - PPLL2 is only available to UNIPHYA (both DP and non-DP)
 * - PPLL0, PPLL1 are available for UNIPHYB/C/D/E/F (both DP and non-DP)
 *
 * DCE 6.0
 * - PPLL0 is available to all UNIPHY (DP only)
 * - PPLL1, PPLL2 are available for all UNIPHY (both DP and non-DP) and DAC
 *
 */
static int dce_v6_0_pick_pll(struct drm_crtc *crtc)
{
	struct amdgpu_crtc *amdgpu_crtc = to_amdgpu_crtc(crtc);
	struct drm_device *dev = crtc->dev;
	struct amdgpu_device *adev = dev->dev_private;
	struct amdgpu_encoder *amdgpu_encoder =
		to_amdgpu_encoder(amdgpu_crtc->encoder);
	u32 pll_in_use;
	int pll;

	if (adev->flags & AMD_IS_APU) {
		struct amdgpu_encoder_atom_dig *dig =
			amdgpu_encoder->enc_priv;

		if ((amdgpu_encoder->encoder_id == ENCODER_OBJECT_ID_INTERNAL_UNIPHY) &&
		    (dig->linkb == false))
			/* UNIPHY A uses PPLL2 */
			return ATOM_PPLL2;
		else if (ENCODER_MODE_IS_DP(amdgpu_atombios_encoder_get_encoder_mode(amdgpu_crtc->encoder))) {
			/* UNIPHY B/C/D/E/F */
			if (adev->clock.dp_extclk)
				/* skip PPLL programming if using ext clock */
				return ATOM_PPLL_INVALID;
			else {
				/* use the same PPLL for all DP monitors */
				pll = amdgpu_pll_get_shared_dp_ppll(crtc);
				if (pll != ATOM_PPLL_INVALID)
					return pll;
			}
		} else {
			/* use the same PPLL for all monitors with the same clock */
			pll = amdgpu_pll_get_shared_nondp_ppll(crtc);
			if (pll != ATOM_PPLL_INVALID)
				return pll;
		}
		/* UNIPHY B/C/D/E/F */
		pll_in_use = amdgpu_pll_get_use_mask(crtc);
		if (!(pll_in_use & (1 << ATOM_PPLL0)))
			return ATOM_PPLL0;
		if (!(pll_in_use & (1 << ATOM_PPLL1)))
			return ATOM_PPLL1;
		DRM_ERROR("unable to allocate a PPLL\n");
		return ATOM_PPLL_INVALID;
	} else {
		/* Don't share PLLs on DCE4.1 chips */
		if (ENCODER_MODE_IS_DP(amdgpu_atombios_encoder_get_encoder_mode(amdgpu_crtc->encoder))) {
			if (adev->clock.dp_extclk)
				/* skip PPLL programming if using ext clock */
				return ATOM_PPLL_INVALID;
		}
		pll_in_use = amdgpu_pll_get_use_mask(crtc);
		if (!(pll_in_use & (1 << ATOM_PPLL1)))
			return ATOM_PPLL1;
		if (!(pll_in_use & (1 << ATOM_PPLL2)))
			return ATOM_PPLL2;
		DRM_ERROR("unable to allocate a PPLL\n");
		return ATOM_PPLL_INVALID;
	}
}

static void dce_v6_0_lock_cursor(struct drm_crtc *crtc, bool lock)
{
	struct amdgpu_device *adev = crtc->dev->dev_private;
	struct amdgpu_crtc *amdgpu_crtc = to_amdgpu_crtc(crtc);
	uint32_t cur_lock;

	cur_lock = RREG32(mmCUR_UPDATE + amdgpu_crtc->crtc_offset);
	if (lock)
		cur_lock |= CUR_UPDATE__CURSOR_UPDATE_LOCK_MASK;
	else
		cur_lock &= ~CUR_UPDATE__CURSOR_UPDATE_LOCK_MASK;
	WREG32(mmCUR_UPDATE + amdgpu_crtc->crtc_offset, cur_lock);
}

static void dce_v6_0_hide_cursor(struct drm_crtc *crtc)
{
	struct amdgpu_crtc *amdgpu_crtc = to_amdgpu_crtc(crtc);
	struct amdgpu_device *adev = crtc->dev->dev_private;

	WREG32_IDX(mmCUR_CONTROL + amdgpu_crtc->crtc_offset,
		   (CUR_CONTROL__CURSOR_24_8_PRE_MULT << CUR_CONTROL__CURSOR_MODE__SHIFT) |
		   (CUR_CONTROL__CURSOR_URGENT_1_2 << CUR_CONTROL__CURSOR_URGENT_CONTROL__SHIFT));
}

static void dce_v6_0_show_cursor(struct drm_crtc *crtc)
{
	struct amdgpu_crtc *amdgpu_crtc = to_amdgpu_crtc(crtc);
	struct amdgpu_device *adev = crtc->dev->dev_private;

	WREG32(mmCUR_SURFACE_ADDRESS_HIGH + amdgpu_crtc->crtc_offset,
	       upper_32_bits(amdgpu_crtc->cursor_addr));
	WREG32(mmCUR_SURFACE_ADDRESS + amdgpu_crtc->crtc_offset,
	       lower_32_bits(amdgpu_crtc->cursor_addr));

	WREG32_IDX(mmCUR_CONTROL + amdgpu_crtc->crtc_offset,
		   CUR_CONTROL__CURSOR_EN_MASK |
		   (CUR_CONTROL__CURSOR_24_8_PRE_MULT << CUR_CONTROL__CURSOR_MODE__SHIFT) |
		   (CUR_CONTROL__CURSOR_URGENT_1_2 << CUR_CONTROL__CURSOR_URGENT_CONTROL__SHIFT));
}

static int dce_v6_0_cursor_move_locked(struct drm_crtc *crtc,
				       int x, int y)
{
	struct amdgpu_crtc *amdgpu_crtc = to_amdgpu_crtc(crtc);
	struct amdgpu_device *adev = crtc->dev->dev_private;
	int xorigin = 0, yorigin = 0;

	/* avivo cursor are offset into the total surface */
	x += crtc->x;
	y += crtc->y;
	DRM_DEBUG("x %d y %d c->x %d c->y %d\n", x, y, crtc->x, crtc->y);

	if (x < 0) {
		xorigin = min(-x, amdgpu_crtc->max_cursor_width - 1);
		x = 0;
	}
	if (y < 0) {
		yorigin = min(-y, amdgpu_crtc->max_cursor_height - 1);
		y = 0;
	}

	WREG32(mmCUR_POSITION + amdgpu_crtc->crtc_offset, (x << 16) | y);
	WREG32(mmCUR_HOT_SPOT + amdgpu_crtc->crtc_offset, (xorigin << 16) | yorigin);
	WREG32(mmCUR_SIZE + amdgpu_crtc->crtc_offset,
	       ((amdgpu_crtc->cursor_width - 1) << 16) | (amdgpu_crtc->cursor_height - 1));

	amdgpu_crtc->cursor_x = x;
	amdgpu_crtc->cursor_y = y;

	return 0;
}

static int dce_v6_0_crtc_cursor_move(struct drm_crtc *crtc,
				     int x, int y)
{
	int ret;

	dce_v6_0_lock_cursor(crtc, true);
	ret = dce_v6_0_cursor_move_locked(crtc, x, y);
	dce_v6_0_lock_cursor(crtc, false);

	return ret;
}

static int dce_v6_0_crtc_cursor_set2(struct drm_crtc *crtc,
				     struct drm_file *file_priv,
				     uint32_t handle,
				     uint32_t width,
				     uint32_t height,
				     int32_t hot_x,
				     int32_t hot_y)
{
	struct amdgpu_crtc *amdgpu_crtc = to_amdgpu_crtc(crtc);
	struct drm_gem_object *obj;
	struct amdgpu_bo *aobj;
	int ret;

	if (!handle) {
		/* turn off cursor */
		dce_v6_0_hide_cursor(crtc);
		obj = NULL;
		goto unpin;
	}

	if ((width > amdgpu_crtc->max_cursor_width) ||
	    (height > amdgpu_crtc->max_cursor_height)) {
		DRM_ERROR("bad cursor width or height %d x %d\n", width, height);
		return -EINVAL;
	}

	obj = drm_gem_object_lookup(crtc->dev, file_priv, handle);
	if (!obj) {
		DRM_ERROR("Cannot find cursor object %x for crtc %d\n", handle, amdgpu_crtc->crtc_id);
		return -ENOENT;
	}

	aobj = gem_to_amdgpu_bo(obj);
	ret = amdgpu_bo_reserve(aobj, false);
	if (ret != 0) {
		drm_gem_object_unreference_unlocked(obj);
		return ret;
	}

	ret = amdgpu_bo_pin(aobj, AMDGPU_GEM_DOMAIN_VRAM, &amdgpu_crtc->cursor_addr);
	amdgpu_bo_unreserve(aobj);
	if (ret) {
		DRM_ERROR("Failed to pin new cursor BO (%d)\n", ret);
		drm_gem_object_unreference_unlocked(obj);
		return ret;
	}

	amdgpu_crtc->cursor_width = width;
	amdgpu_crtc->cursor_height = height;

	dce_v6_0_lock_cursor(crtc, true);

	if (hot_x != amdgpu_crtc->cursor_hot_x ||
	    hot_y != amdgpu_crtc->cursor_hot_y) {
		int x, y;

		x = amdgpu_crtc->cursor_x + amdgpu_crtc->cursor_hot_x - hot_x;
		y = amdgpu_crtc->cursor_y + amdgpu_crtc->cursor_hot_y - hot_y;

		dce_v6_0_cursor_move_locked(crtc, x, y);

		amdgpu_crtc->cursor_hot_x = hot_x;
		amdgpu_crtc->cursor_hot_y = hot_y;
	}

	dce_v6_0_show_cursor(crtc);
	dce_v6_0_lock_cursor(crtc, false);

unpin:
	if (amdgpu_crtc->cursor_bo) {
		struct amdgpu_bo *aobj = gem_to_amdgpu_bo(amdgpu_crtc->cursor_bo);
		ret = amdgpu_bo_reserve(aobj, false);
		if (likely(ret == 0)) {
			amdgpu_bo_unpin(aobj);
			amdgpu_bo_unreserve(aobj);
		}
		drm_gem_object_unreference_unlocked(amdgpu_crtc->cursor_bo);
	}

	amdgpu_crtc->cursor_bo = obj;
	return 0;
}

static void dce_v6_0_cursor_reset(struct drm_crtc *crtc)
{
	struct amdgpu_crtc *amdgpu_crtc = to_amdgpu_crtc(crtc);

	if (amdgpu_crtc->cursor_bo) {
		dce_v6_0_lock_cursor(crtc, true);

		dce_v6_0_cursor_move_locked(crtc, amdgpu_crtc->cursor_x,
					    amdgpu_crtc->cursor_y);

		dce_v6_0_show_cursor(crtc);

		dce_v6_0_lock_cursor(crtc, false);
	}
}

static void dce_v6_0_crtc_gamma_set(struct drm_crtc *crtc, u16 *red, u16 *green,
				    u16 *blue, uint32_t start, uint32_t size)
{
	struct amdgpu_crtc *amdgpu_crtc = to_amdgpu_crtc(crtc);
	int end = (start + size > 256) ? 256 : start + size, i;

	/* userspace palettes are always correct as is */
	for (i = start; i < end; i++) {
		amdgpu_crtc->lut_r[i] = red[i] >> 6;
		amdgpu_crtc->lut_g[i] = green[i] >> 6;
		amdgpu_crtc->lut_b[i] = blue[i] >> 6;
	}
	dce_v6_0_crtc_load_lut(crtc);
}

static void dce_v6_0_crtc_destroy(struct drm_crtc *crtc)
{
	struct amdgpu_crtc *amdgpu_crtc = to_amdgpu_crtc(crtc);

	drm_crtc_cleanup(crtc);
	destroy_workqueue(amdgpu_crtc->pflip_queue);
	kfree(amdgpu_crtc);
}

static const struct drm_crtc_funcs dce_v6_0_crtc_funcs = {
	.cursor_set2 = dce_v6_0_crtc_cursor_set2,
	.cursor_move = dce_v6_0_crtc_cursor_move,
	.gamma_set = dce_v6_0_crtc_gamma_set,
	.set_config = amdgpu_crtc_set_config,
	.destroy = dce_v6_0_crtc_destroy,
	.page_flip = amdgpu_crtc_page_flip,
};

static void dce_v6_0_crtc_dpms(struct drm_crtc *crtc, int mode)
{
	struct drm_device *dev = crtc->dev;
	struct amdgpu_device *adev = dev->dev_private;
	struct amdgpu_crtc *amdgpu_crtc = to_amdgpu_crtc(crtc);
	unsigned type;

	switch (mode) {
	case DRM_MODE_DPMS_ON:
		amdgpu_crtc->enabled = true;
		amdgpu_atombios_crtc_enable(crtc, ATOM_ENABLE);
		dce_v6_0_vga_enable(crtc, true);
		amdgpu_atombios_crtc_blank(crtc, ATOM_DISABLE);
		dce_v6_0_vga_enable(crtc, false);
		/* Make sure VBLANK and PFLIP interrupts are still enabled */
		type = amdgpu_crtc_idx_to_irq_type(adev, amdgpu_crtc->crtc_id);
		amdgpu_irq_update(adev, &adev->crtc_irq, type);
		amdgpu_irq_update(adev, &adev->pageflip_irq, type);
		drm_vblank_post_modeset(dev, amdgpu_crtc->crtc_id);
		dce_v6_0_crtc_load_lut(crtc);
		break;
	case DRM_MODE_DPMS_STANDBY:
	case DRM_MODE_DPMS_SUSPEND:
	case DRM_MODE_DPMS_OFF:
		drm_vblank_pre_modeset(dev, amdgpu_crtc->crtc_id);
		if (amdgpu_crtc->enabled) {
			dce_v6_0_vga_enable(crtc, true);
			amdgpu_atombios_crtc_blank(crtc, ATOM_ENABLE);
			dce_v6_0_vga_enable(crtc, false);
		}
		amdgpu_atombios_crtc_enable(crtc, ATOM_DISABLE);
		amdgpu_crtc->enabled = false;
		break;
	}
	/* adjust pm to dpms */
	amdgpu_pm_compute_clocks(adev);
}

static void dce_v6_0_crtc_prepare(struct drm_crtc *crtc)
{
	/* disable crtc pair power gating before programming */
	amdgpu_atombios_crtc_powergate(crtc, ATOM_DISABLE);
	amdgpu_atombios_crtc_lock(crtc, ATOM_ENABLE);
	dce_v6_0_crtc_dpms(crtc, DRM_MODE_DPMS_OFF);
}

static void dce_v6_0_crtc_commit(struct drm_crtc *crtc)
{
	dce_v6_0_crtc_dpms(crtc, DRM_MODE_DPMS_ON);
	amdgpu_atombios_crtc_lock(crtc, ATOM_DISABLE);
}

static void dce_v6_0_crtc_disable(struct drm_crtc *crtc)
{
	struct amdgpu_crtc *amdgpu_crtc = to_amdgpu_crtc(crtc);
	struct drm_device *dev = crtc->dev;
	struct amdgpu_device *adev = dev->dev_private;
	struct amdgpu_atom_ss ss;
	int i;

	dce_v6_0_crtc_dpms(crtc, DRM_MODE_DPMS_OFF);
	if (crtc->primary->fb) {
		int r;
		struct amdgpu_framebuffer *amdgpu_fb;
		struct amdgpu_bo *rbo;

		amdgpu_fb = to_amdgpu_framebuffer(crtc->primary->fb);
		rbo = gem_to_amdgpu_bo(amdgpu_fb->obj);
		r = amdgpu_bo_reserve(rbo, false);
		if (unlikely(r))
			DRM_ERROR("failed to reserve rbo before unpin\n");
		else {
			amdgpu_bo_unpin(rbo);
			amdgpu_bo_unreserve(rbo);
		}
	}
	/* disable the GRPH */
	dce_v6_0_grph_enable(crtc, false);

	amdgpu_atombios_crtc_powergate(crtc, ATOM_ENABLE);

	for (i = 0; i < adev->mode_info.num_crtc; i++) {
		if (adev->mode_info.crtcs[i] &&
		    adev->mode_info.crtcs[i]->enabled &&
		    i != amdgpu_crtc->crtc_id &&
		    amdgpu_crtc->pll_id == adev->mode_info.crtcs[i]->pll_id) {
			/* one other crtc is using this pll don't turn
			 * off the pll
			 */
			goto done;
		}
	}

	switch (amdgpu_crtc->pll_id) {
	case ATOM_PPLL1:
	case ATOM_PPLL2:
		/* disable the ppll */
		amdgpu_atombios_crtc_program_pll(crtc, amdgpu_crtc->crtc_id, amdgpu_crtc->pll_id,
					  0, 0, ATOM_DISABLE, 0, 0, 0, 0, 0, false, &ss);
		break;
	case ATOM_PPLL0:
		/* disable the ppll */
		if ((adev->asic_type == CHIP_KAVERI) ||
		    (adev->asic_type == CHIP_BONAIRE) ||
		    (adev->asic_type == CHIP_HAWAII))
			amdgpu_atombios_crtc_program_pll(crtc, amdgpu_crtc->crtc_id, amdgpu_crtc->pll_id,
						  0, 0, ATOM_DISABLE, 0, 0, 0, 0, 0, false, &ss);
		break;
	default:
		break;
	}
done:
	amdgpu_crtc->pll_id = ATOM_PPLL_INVALID;
	amdgpu_crtc->adjusted_clock = 0;
	amdgpu_crtc->encoder = NULL;
	amdgpu_crtc->connector = NULL;
}

static int dce_v6_0_crtc_mode_set(struct drm_crtc *crtc,
				  struct drm_display_mode *mode,
				  struct drm_display_mode *adjusted_mode,
				  int x, int y, struct drm_framebuffer *old_fb)
{
	struct amdgpu_crtc *amdgpu_crtc = to_amdgpu_crtc(crtc);

	if (!amdgpu_crtc->adjusted_clock)
		return -EINVAL;

	amdgpu_atombios_crtc_set_pll(crtc, adjusted_mode);
	amdgpu_atombios_crtc_set_dtd_timing(crtc, adjusted_mode);
	dce_v6_0_crtc_do_set_base(crtc, old_fb, x, y, 0);
	amdgpu_atombios_crtc_overscan_setup(crtc, mode, adjusted_mode);
	amdgpu_atombios_crtc_scaler_setup(crtc);
	dce_v6_0_cursor_reset(crtc);
	/* update the hw version fpr dpm */
	amdgpu_crtc->hw_mode = *adjusted_mode;

	return 0;
}

static bool dce_v6_0_crtc_mode_fixup(struct drm_crtc *crtc,
				     const struct drm_display_mode *mode,
				     struct drm_display_mode *adjusted_mode)
{
	struct amdgpu_crtc *amdgpu_crtc = to_amdgpu_crtc(crtc);
	struct drm_device *dev = crtc->dev;
	struct drm_encoder *encoder;

	/* assign the encoder to the amdgpu crtc to avoid repeated lookups later */
	list_for_each_entry(encoder, &dev->mode_config.encoder_list, head) {
		if (encoder->crtc == crtc) {
			amdgpu_crtc->encoder = encoder;
			amdgpu_crtc->connector = amdgpu_get_connector_for_encoder(encoder);
			break;
		}
	}
	if ((amdgpu_crtc->encoder == NULL) || (amdgpu_crtc->connector == NULL)) {
		amdgpu_crtc->encoder = NULL;
		amdgpu_crtc->connector = NULL;
		return false;
	}
	if (!amdgpu_crtc_scaling_mode_fixup(crtc, mode, adjusted_mode))
		return false;
	if (amdgpu_atombios_crtc_prepare_pll(crtc, adjusted_mode))
		return false;
	/* pick pll */
	amdgpu_crtc->pll_id = dce_v6_0_pick_pll(crtc);
	/* if we can't get a PPLL for a non-DP encoder, fail */
	if ((amdgpu_crtc->pll_id == ATOM_PPLL_INVALID) &&
	    !ENCODER_MODE_IS_DP(amdgpu_atombios_encoder_get_encoder_mode(amdgpu_crtc->encoder)))
		return false;

	return true;
}

static int dce_v6_0_crtc_set_base(struct drm_crtc *crtc, int x, int y,
				  struct drm_framebuffer *old_fb)
{
	return dce_v6_0_crtc_do_set_base(crtc, old_fb, x, y, 0);
}

static int dce_v6_0_crtc_set_base_atomic(struct drm_crtc *crtc,
					 struct drm_framebuffer *fb,
					 int x, int y, enum mode_set_atomic state)
{
       return dce_v6_0_crtc_do_set_base(crtc, fb, x, y, 1);
}

static const struct drm_crtc_helper_funcs dce_v6_0_crtc_helper_funcs = {
	.dpms = dce_v6_0_crtc_dpms,
	.mode_fixup = dce_v6_0_crtc_mode_fixup,
	.mode_set = dce_v6_0_crtc_mode_set,
	.mode_set_base = dce_v6_0_crtc_set_base,
	.mode_set_base_atomic = dce_v6_0_crtc_set_base_atomic,
	.prepare = dce_v6_0_crtc_prepare,
	.commit = dce_v6_0_crtc_commit,
	.load_lut = dce_v6_0_crtc_load_lut,
	.disable = dce_v6_0_crtc_disable,
};

static int dce_v6_0_crtc_init(struct amdgpu_device *adev, int index)
{
	struct amdgpu_crtc *amdgpu_crtc;
	int i;

	amdgpu_crtc = kzalloc(sizeof(struct amdgpu_crtc) +
			      (AMDGPUFB_CONN_LIMIT * sizeof(struct drm_connector *)), GFP_KERNEL);
	if (amdgpu_crtc == NULL)
		return -ENOMEM;

	drm_crtc_init(adev->ddev, &amdgpu_crtc->base, &dce_v6_0_crtc_funcs);

	drm_mode_crtc_set_gamma_size(&amdgpu_crtc->base, 256);
	amdgpu_crtc->crtc_id = index;
	amdgpu_crtc->pflip_queue = create_singlethread_workqueue("amdgpu-pageflip-queue");
	adev->mode_info.crtcs[index] = amdgpu_crtc;

	amdgpu_crtc->max_cursor_width = CURSOR_WIDTH;
	amdgpu_crtc->max_cursor_height = CURSOR_HEIGHT;
	adev->ddev->mode_config.cursor_width = amdgpu_crtc->max_cursor_width;
	adev->ddev->mode_config.cursor_height = amdgpu_crtc->max_cursor_height;

	for (i = 0; i < 256; i++) {
		amdgpu_crtc->lut_r[i] = i << 2;
		amdgpu_crtc->lut_g[i] = i << 2;
		amdgpu_crtc->lut_b[i] = i << 2;
	}

	amdgpu_crtc->crtc_offset = crtc_offsets[amdgpu_crtc->crtc_id];

	amdgpu_crtc->pll_id = ATOM_PPLL_INVALID;
	amdgpu_crtc->adjusted_clock = 0;
	amdgpu_crtc->encoder = NULL;
	amdgpu_crtc->connector = NULL;
	drm_crtc_helper_add(&amdgpu_crtc->base, &dce_v6_0_crtc_helper_funcs);

	return 0;
}

static int dce_v6_0_early_init(void *handle)
{
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;

	adev->audio_endpt_rreg = &dce_v6_0_audio_endpt_rreg;
	adev->audio_endpt_wreg = &dce_v6_0_audio_endpt_wreg;

	switch (adev->asic_type) {
	case CHIP_TAHITI:
	case CHIP_PITCAIRN:
	case CHIP_VERDE:
		adev->mode_info.num_crtc = 6;
		adev->mode_info.num_hpd = 6;
		adev->mode_info.num_dig = 6;
		break;
	case CHIP_OLAND:
		adev->mode_info.num_crtc = 2;
		adev->mode_info.num_hpd = 2;
		adev->mode_info.num_dig = 2;
		break;
	case CHIP_HAINAN:
		adev->mode_info.num_crtc = 0;
		adev->mode_info.num_hpd = 0;
		adev->mode_info.num_dig = 0;
		break;
	default:
		/* FIXME: not supported yet */
		return -EINVAL;
	}

	dce_v6_0_set_display_funcs(adev);
	dce_v6_0_set_irq_funcs(adev);

	return 0;
}

static int dce_v6_0_sw_init(void *handle)
{
	int r, i;
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;

	for (i = 0; i < adev->mode_info.num_crtc; i++) {
		r = amdgpu_irq_add_id(adev, i + 1, &adev->crtc_irq);
		if (r)
			return r;
	}

	for (i = 8; i < 20; i += 2) {
		r = amdgpu_irq_add_id(adev, i, &adev->pageflip_irq);
		if (r)
			return r;
	}

	/* HPD hotplug */
	r = amdgpu_irq_add_id(adev, 42, &adev->hpd_irq);
	if (r)
		return r;

	adev->mode_info.mode_config_initialized = true;

	adev->ddev->mode_config.funcs = &amdgpu_mode_funcs;

	adev->ddev->mode_config.max_width = 16384;
	adev->ddev->mode_config.max_height = 16384;

	adev->ddev->mode_config.preferred_depth = 24;
	adev->ddev->mode_config.prefer_shadow = 1;

	adev->ddev->mode_config.fb_base = adev->mc.aper_base;

	r = amdgpu_modeset_create_props(adev);
	if (r)
		return r;

	adev->ddev->mode_config.max_width = 16384;
	adev->ddev->mode_config.max_height = 16384;

	/* allocate crtcs */
	for (i = 0; i < adev->mode_info.num_crtc; i++) {
		r = dce_v6_0_crtc_init(adev, i);
		if (r)
			return r;
	}

	if (amdgpu_atombios_get_connector_info_from_object_table(adev))
		amdgpu_print_display_setup(adev->ddev);
	else
		return -EINVAL;

	/* setup afmt */
	/*dce_v6_0_afmt_init(adev);

	r = dce_v6_0_audio_init(adev);
	if (r)
		return r;
	*/

	drm_kms_helper_poll_init(adev->ddev);

	return r;
}

static int dce_v6_0_sw_fini(void *handle)
{
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;

	kfree(adev->mode_info.bios_hardcoded_edid);

	drm_kms_helper_poll_fini(adev->ddev);

	/*dce_v6_0_audio_fini(adev);

	dce_v6_0_afmt_fini(adev);
	*/

	drm_mode_config_cleanup(adev->ddev);
	adev->mode_info.mode_config_initialized = false;

	return 0;
}

static int dce_v6_0_hw_init(void *handle)
{
	int i;
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;

	/* init dig PHYs, disp eng pll */
	amdgpu_atombios_encoder_init_dig(adev);
	amdgpu_atombios_crtc_set_disp_eng_pll(adev, adev->clock.default_dispclk);

	/* initialize hpd */
	dce_v6_0_hpd_init(adev);

	/*for (i = 0; i < adev->mode_info.audio.num_pins; i++) {
		dce_v6_0_audio_enable(adev, &adev->mode_info.audio.pin[i], false);
	}
	*/

	dce_v6_0_pageflip_interrupt_init(adev);

	return 0;
}

static int dce_v6_0_hw_fini(void *handle)
{
	int i;
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;

	dce_v6_0_hpd_fini(adev);

	/*for (i = 0; i < adev->mode_info.audio.num_pins; i++) {
		dce_v6_0_audio_enable(adev, &adev->mode_info.audio.pin[i], false);
	}
	*/

	dce_v6_0_pageflip_interrupt_fini(adev);

	return 0;
}

static int dce_v6_0_suspend(void *handle)
{
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;

	amdgpu_atombios_scratch_regs_save(adev);

	return dce_v6_0_hw_fini(handle);
}

static int dce_v6_0_resume(void *handle)
{
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;
	int ret;

	ret = dce_v6_0_hw_init(handle);

	amdgpu_atombios_scratch_regs_restore(adev);

	/* turn on the BL */
	if (adev->mode_info.bl_encoder) {
		u8 bl_level = amdgpu_display_backlight_get_level(adev,
								  adev->mode_info.bl_encoder);
		amdgpu_display_backlight_set_level(adev, adev->mode_info.bl_encoder,
						    bl_level);
	}

	return ret;
}

static bool dce_v6_0_is_idle(void *handle)
{
	return true;
}

static int dce_v6_0_wait_for_idle(void *handle)
{
	return 0;
}

static void dce_v6_0_print_status(void *handle)
{
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;

	dev_info(adev->dev, "DCE 6.x registers\n");
	/* XXX todo */
}

static int dce_v6_0_soft_reset(void *handle)
{
	u32 srbm_soft_reset = 0, tmp;
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;

	if (dce_v6_0_is_display_hung(adev))
		srbm_soft_reset |= SRBM_SOFT_RESET__SOFT_RESET_DC_MASK;

	if (srbm_soft_reset) {
		dce_v6_0_print_status((void *)adev);

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
		dce_v6_0_print_status((void *)adev);
	}
	return 0;
}

static void dce_v6_0_set_crtc_vblank_interrupt_state(struct amdgpu_device *adev,
						     int crtc,
						     enum amdgpu_interrupt_state state)
{
	u32 reg_block, lb_interrupt_mask;

	if (crtc >= adev->mode_info.num_crtc) {
		DRM_DEBUG("invalid crtc %d\n", crtc);
		return;
	}

	switch (crtc) {
	case 0:
		reg_block = CRTC0_REGISTER_OFFSET;
		break;
	case 1:
		reg_block = CRTC1_REGISTER_OFFSET;
		break;
	case 2:
		reg_block = CRTC2_REGISTER_OFFSET;
		break;
	case 3:
		reg_block = CRTC3_REGISTER_OFFSET;
		break;
	case 4:
		reg_block = CRTC4_REGISTER_OFFSET;
		break;
	case 5:
		reg_block = CRTC5_REGISTER_OFFSET;
		break;
	default:
		DRM_DEBUG("invalid crtc %d\n", crtc);
		return;
	}

	switch (state) {
	case AMDGPU_IRQ_STATE_DISABLE:
		lb_interrupt_mask = RREG32(mmLB_INT_MASK + reg_block);
		lb_interrupt_mask &= ~LB_INT_MASK__VBLANK_INT_MASK;
		WREG32(mmLB_INT_MASK + reg_block, lb_interrupt_mask);
		break;
	case AMDGPU_IRQ_STATE_ENABLE:
		lb_interrupt_mask = RREG32(mmLB_INT_MASK + reg_block);
		lb_interrupt_mask |= LB_INT_MASK__VBLANK_INT_MASK;
		WREG32(mmLB_INT_MASK + reg_block, lb_interrupt_mask);
		break;
	default:
		break;
	}
}

static void dce_v6_0_set_crtc_vline_interrupt_state(struct amdgpu_device *adev,
						    int crtc,
						    enum amdgpu_interrupt_state state)
{
	u32 reg_block, lb_interrupt_mask;

	if (crtc >= adev->mode_info.num_crtc) {
		DRM_DEBUG("invalid crtc %d\n", crtc);
		return;
	}

	switch (crtc) {
	case 0:
		reg_block = CRTC0_REGISTER_OFFSET;
		break;
	case 1:
		reg_block = CRTC1_REGISTER_OFFSET;
		break;
	case 2:
		reg_block = CRTC2_REGISTER_OFFSET;
		break;
	case 3:
		reg_block = CRTC3_REGISTER_OFFSET;
		break;
	case 4:
		reg_block = CRTC4_REGISTER_OFFSET;
		break;
	case 5:
		reg_block = CRTC5_REGISTER_OFFSET;
		break;
	default:
		DRM_DEBUG("invalid crtc %d\n", crtc);
		return;
	}

	switch (state) {
	case AMDGPU_IRQ_STATE_DISABLE:
		lb_interrupt_mask = RREG32(mmLB_INT_MASK + reg_block);
		lb_interrupt_mask &= ~LB_INT_MASK__VLINE_INT_MASK;
		WREG32(mmLB_INT_MASK + reg_block, lb_interrupt_mask);
		break;
	case AMDGPU_IRQ_STATE_ENABLE:
		lb_interrupt_mask = RREG32(mmLB_INT_MASK + reg_block);
		lb_interrupt_mask |= LB_INT_MASK__VLINE_INT_MASK;
		WREG32(mmLB_INT_MASK + reg_block, lb_interrupt_mask);
		break;
	default:
		break;
	}
}

static int dce_v6_0_set_crtc_interrupt_state(struct amdgpu_device *adev,
					     struct amdgpu_irq_src *src,
					     unsigned type,
					     enum amdgpu_interrupt_state state)
{
	switch (type) {
	case AMDGPU_CRTC_IRQ_VBLANK1:
		dce_v6_0_set_crtc_vblank_interrupt_state(adev, 0, state);
		break;
	case AMDGPU_CRTC_IRQ_VBLANK2:
		dce_v6_0_set_crtc_vblank_interrupt_state(adev, 1, state);
		break;
	case AMDGPU_CRTC_IRQ_VBLANK3:
		dce_v6_0_set_crtc_vblank_interrupt_state(adev, 2, state);
		break;
	case AMDGPU_CRTC_IRQ_VBLANK4:
		dce_v6_0_set_crtc_vblank_interrupt_state(adev, 3, state);
		break;
	case AMDGPU_CRTC_IRQ_VBLANK5:
		dce_v6_0_set_crtc_vblank_interrupt_state(adev, 4, state);
		break;
	case AMDGPU_CRTC_IRQ_VBLANK6:
		dce_v6_0_set_crtc_vblank_interrupt_state(adev, 5, state);
		break;
	case AMDGPU_CRTC_IRQ_VLINE1:
		dce_v6_0_set_crtc_vline_interrupt_state(adev, 0, state);
		break;
	case AMDGPU_CRTC_IRQ_VLINE2:
		dce_v6_0_set_crtc_vline_interrupt_state(adev, 1, state);
		break;
	case AMDGPU_CRTC_IRQ_VLINE3:
		dce_v6_0_set_crtc_vline_interrupt_state(adev, 2, state);
		break;
	case AMDGPU_CRTC_IRQ_VLINE4:
		dce_v6_0_set_crtc_vline_interrupt_state(adev, 3, state);
		break;
	case AMDGPU_CRTC_IRQ_VLINE5:
		dce_v6_0_set_crtc_vline_interrupt_state(adev, 4, state);
		break;
	case AMDGPU_CRTC_IRQ_VLINE6:
		dce_v6_0_set_crtc_vline_interrupt_state(adev, 5, state);
		break;
	default:
		break;
	}
	return 0;
}

static int dce_v6_0_crtc_irq(struct amdgpu_device *adev,
			     struct amdgpu_irq_src *source,
			     struct amdgpu_iv_entry *entry)
{
	unsigned crtc = entry->src_id - 1;
	uint32_t disp_int = RREG32(interrupt_status_offsets[crtc].reg);
	unsigned irq_type = amdgpu_crtc_idx_to_irq_type(adev, crtc);

	switch (entry->src_data) {
	case 0: /* vblank */
		if (disp_int & interrupt_status_offsets[crtc].vblank)
			WREG32(mmLB_VBLANK_STATUS + crtc_offsets[crtc], LB_VBLANK_STATUS__VBLANK_ACK_MASK);
		else
			DRM_DEBUG("IH: IH event w/o asserted irq bit?\n");

		if (amdgpu_irq_enabled(adev, source, irq_type)) {
			drm_handle_vblank(adev->ddev, crtc);
		}
		DRM_DEBUG("IH: D%d vblank\n", crtc + 1);

		break;
	case 1: /* vline */
		if (disp_int & interrupt_status_offsets[crtc].vline)
			WREG32(mmLB_VLINE_STATUS + crtc_offsets[crtc], LB_VLINE_STATUS__VLINE_ACK_MASK);
		else
			DRM_DEBUG("IH: IH event w/o asserted irq bit?\n");

		DRM_DEBUG("IH: D%d vline\n", crtc + 1);

		break;
	default:
		DRM_DEBUG("Unhandled interrupt: %d %d\n", entry->src_id, entry->src_data);
		break;
	}

	return 0;
}

static int dce_v6_0_set_pageflip_interrupt_state(struct amdgpu_device *adev,
						 struct amdgpu_irq_src *src,
						 unsigned type,
						 enum amdgpu_interrupt_state state)
{
	u32 reg;

	if (type >= adev->mode_info.num_crtc) {
		DRM_ERROR("invalid pageflip crtc %d\n", type);
		return -EINVAL;
	}

	reg = RREG32(mmGRPH_INT_CONTROL + crtc_offsets[type]);
	if (state == AMDGPU_IRQ_STATE_DISABLE)
		WREG32(mmGRPH_INT_CONTROL + crtc_offsets[type],
		       reg & ~GRPH_INT_CONTROL__GRPH_PFLIP_INT_MASK);
	else
		WREG32(mmGRPH_INT_CONTROL + crtc_offsets[type],
		       reg | GRPH_INT_CONTROL__GRPH_PFLIP_INT_MASK);

	return 0;
}

static int dce_v6_0_pageflip_irq(struct amdgpu_device *adev,
				struct amdgpu_irq_src *source,
				struct amdgpu_iv_entry *entry)
{
	unsigned long flags;
	unsigned crtc_id;
	struct amdgpu_crtc *amdgpu_crtc;
	struct amdgpu_flip_work *works;

	crtc_id = (entry->src_id - 8) >> 1;
	amdgpu_crtc = adev->mode_info.crtcs[crtc_id];

	if (crtc_id >= adev->mode_info.num_crtc) {
		DRM_ERROR("invalid pageflip crtc %d\n", crtc_id);
		return -EINVAL;
	}

	if (RREG32(mmGRPH_INT_STATUS + crtc_offsets[crtc_id]) &
	    GRPH_INT_STATUS__GRPH_PFLIP_INT_OCCURRED_MASK)
		WREG32(mmGRPH_INT_STATUS + crtc_offsets[crtc_id],
		       GRPH_INT_STATUS__GRPH_PFLIP_INT_CLEAR_MASK);

	/* IRQ could occur when in initial stage */
	if (amdgpu_crtc == NULL)
		return 0;

	spin_lock_irqsave(&adev->ddev->event_lock, flags);
	works = amdgpu_crtc->pflip_works;
	if (amdgpu_crtc->pflip_status != AMDGPU_FLIP_SUBMITTED){
		DRM_DEBUG_DRIVER("amdgpu_crtc->pflip_status = %d != "
						"AMDGPU_FLIP_SUBMITTED(%d)\n",
						amdgpu_crtc->pflip_status,
						AMDGPU_FLIP_SUBMITTED);
		spin_unlock_irqrestore(&adev->ddev->event_lock, flags);
		return 0;
	}

	/* page flip completed. clean up */
	amdgpu_crtc->pflip_status = AMDGPU_FLIP_NONE;
	amdgpu_crtc->pflip_works = NULL;

	/* wakeup usersapce */
	if (works->event)
		drm_send_vblank_event(adev->ddev, crtc_id, works->event);

	spin_unlock_irqrestore(&adev->ddev->event_lock, flags);

	drm_vblank_put(adev->ddev, amdgpu_crtc->crtc_id);
	queue_work(amdgpu_crtc->pflip_queue, &works->unpin_work);

	return 0;
}

static int dce_v6_0_set_hpd_interrupt_state(struct amdgpu_device *adev,
					    struct amdgpu_irq_src *src,
					    unsigned type,
					    enum amdgpu_interrupt_state state)
{
	u32 dc_hpd_int_cntl_reg, dc_hpd_int_cntl;

	switch (type) {
	case AMDGPU_HPD_1:
		dc_hpd_int_cntl_reg = mmDC_HPD1_INT_CONTROL;
		break;
	case AMDGPU_HPD_2:
		dc_hpd_int_cntl_reg = mmDC_HPD2_INT_CONTROL;
		break;
	case AMDGPU_HPD_3:
		dc_hpd_int_cntl_reg = mmDC_HPD3_INT_CONTROL;
		break;
	case AMDGPU_HPD_4:
		dc_hpd_int_cntl_reg = mmDC_HPD4_INT_CONTROL;
		break;
	case AMDGPU_HPD_5:
		dc_hpd_int_cntl_reg = mmDC_HPD5_INT_CONTROL;
		break;
	case AMDGPU_HPD_6:
		dc_hpd_int_cntl_reg = mmDC_HPD6_INT_CONTROL;
		break;
	default:
		DRM_DEBUG("invalid hdp %d\n", type);
		return 0;
	}

	switch (state) {
	case AMDGPU_IRQ_STATE_DISABLE:
		dc_hpd_int_cntl = RREG32(dc_hpd_int_cntl_reg);
		dc_hpd_int_cntl &= ~DC_HPD1_INT_CONTROL__DC_HPD1_INT_EN_MASK;
		WREG32(dc_hpd_int_cntl_reg, dc_hpd_int_cntl);
		break;
	case AMDGPU_IRQ_STATE_ENABLE:
		dc_hpd_int_cntl = RREG32(dc_hpd_int_cntl_reg);
		dc_hpd_int_cntl |= DC_HPD1_INT_CONTROL__DC_HPD1_INT_EN_MASK;
		WREG32(dc_hpd_int_cntl_reg, dc_hpd_int_cntl);
		break;
	default:
		break;
	}

	return 0;
}

static int dce_v6_0_hpd_irq(struct amdgpu_device *adev,
			    struct amdgpu_irq_src *source,
			    struct amdgpu_iv_entry *entry)
{
	uint32_t disp_int, mask, int_control, tmp;
	unsigned hpd;

	if (entry->src_data >= adev->mode_info.num_hpd) {
		DRM_DEBUG("Unhandled interrupt: %d %d\n", entry->src_id, entry->src_data);
		return 0;
	}

	hpd = entry->src_data;
	disp_int = RREG32(interrupt_status_offsets[hpd].reg);
	mask = interrupt_status_offsets[hpd].hpd;
	int_control = hpd_int_control_offsets[hpd];

	if (disp_int & mask) {
		tmp = RREG32(int_control);
		tmp |= DC_HPD1_INT_CONTROL__DC_HPD1_INT_ACK_MASK;
		WREG32(int_control, tmp);
		schedule_work(&adev->hotplug_work);
		DRM_DEBUG("IH: HPD%d\n", hpd + 1);
	}

	return 0;

}

static int dce_v6_0_set_clockgating_state(void *handle,
					  enum amd_clockgating_state state)
{
	return 0;
}

static int dce_v6_0_set_powergating_state(void *handle,
					  enum amd_powergating_state state)
{
	return 0;
}

const struct amd_ip_funcs dce_v6_0_ip_funcs = {
	.early_init = dce_v6_0_early_init,
	.late_init = NULL,
	.sw_init = dce_v6_0_sw_init,
	.sw_fini = dce_v6_0_sw_fini,
	.hw_init = dce_v6_0_hw_init,
	.hw_fini = dce_v6_0_hw_fini,
	.suspend = dce_v6_0_suspend,
	.resume = dce_v6_0_resume,
	.is_idle = dce_v6_0_is_idle,
	.wait_for_idle = dce_v6_0_wait_for_idle,
	.soft_reset = dce_v6_0_soft_reset,
	.print_status = dce_v6_0_print_status,
	.set_clockgating_state = dce_v6_0_set_clockgating_state,
	.set_powergating_state = dce_v6_0_set_powergating_state,
};

static void
dce_v6_0_encoder_mode_set(struct drm_encoder *encoder,
			  struct drm_display_mode *mode,
			  struct drm_display_mode *adjusted_mode)
{
	struct amdgpu_encoder *amdgpu_encoder = to_amdgpu_encoder(encoder);

	amdgpu_encoder->pixel_clock = adjusted_mode->clock;

	/* need to call this here rather than in prepare() since we need some crtc info */
	amdgpu_atombios_encoder_dpms(encoder, DRM_MODE_DPMS_OFF);

	/* set scaler clears this on some chips */
	dce_v6_0_set_interleave(encoder->crtc, mode);

	/*if (amdgpu_atombios_encoder_get_encoder_mode(encoder) == ATOM_ENCODER_MODE_HDMI) {
		dce_v6_0_afmt_enable(encoder, true);
		dce_v6_0_afmt_setmode(encoder, adjusted_mode);
	}*/
}

static void dce_v6_0_encoder_prepare(struct drm_encoder *encoder)
{
	struct amdgpu_device *adev = encoder->dev->dev_private;
	struct amdgpu_encoder *amdgpu_encoder = to_amdgpu_encoder(encoder);
	struct drm_connector *connector = amdgpu_get_connector_for_encoder(encoder);

	if ((amdgpu_encoder->active_device &
	     (ATOM_DEVICE_DFP_SUPPORT | ATOM_DEVICE_LCD_SUPPORT)) ||
	    (amdgpu_encoder_get_dp_bridge_encoder_id(encoder) !=
	     ENCODER_OBJECT_ID_NONE)) {
		struct amdgpu_encoder_atom_dig *dig = amdgpu_encoder->enc_priv;
		if (dig) {
			dig->dig_encoder = dce_v6_0_pick_dig_encoder(encoder);
			if (amdgpu_encoder->active_device & ATOM_DEVICE_DFP_SUPPORT)
				dig->afmt = adev->mode_info.afmt[dig->dig_encoder];
		}
	}

	amdgpu_atombios_scratch_regs_lock(adev, true);

	if (connector) {
		struct amdgpu_connector *amdgpu_connector = to_amdgpu_connector(connector);

		/* select the clock/data port if it uses a router */
		if (amdgpu_connector->router.cd_valid)
			amdgpu_i2c_router_select_cd_port(amdgpu_connector);

		/* turn eDP panel on for mode set */
		if (connector->connector_type == DRM_MODE_CONNECTOR_eDP)
			amdgpu_atombios_encoder_set_edp_panel_power(connector,
							     ATOM_TRANSMITTER_ACTION_POWER_ON);
	}

	/* this is needed for the pll/ss setup to work correctly in some cases */
	amdgpu_atombios_encoder_set_crtc_source(encoder);
	/* set up the FMT blocks */
	dce_v6_0_program_fmt(encoder);
}

static void dce_v6_0_encoder_commit(struct drm_encoder *encoder)
{
	struct drm_device *dev = encoder->dev;
	struct amdgpu_device *adev = dev->dev_private;

	/* need to call this here as we need the crtc set up */
	amdgpu_atombios_encoder_dpms(encoder, DRM_MODE_DPMS_ON);
	amdgpu_atombios_scratch_regs_lock(adev, false);
}

static void dce_v6_0_encoder_disable(struct drm_encoder *encoder)
{
	struct amdgpu_encoder *amdgpu_encoder = to_amdgpu_encoder(encoder);
	struct amdgpu_encoder_atom_dig *dig;

	amdgpu_atombios_encoder_dpms(encoder, DRM_MODE_DPMS_OFF);

	if (amdgpu_atombios_encoder_is_digital(encoder)) {
		/*if (amdgpu_atombios_encoder_get_encoder_mode(encoder) == ATOM_ENCODER_MODE_HDMI)
			dce_v6_0_afmt_enable(encoder, false);*/
		dig = amdgpu_encoder->enc_priv;
		dig->dig_encoder = -1;
	}
	amdgpu_encoder->active_device = 0;
}

/* these are handled by the primary encoders */
static void dce_v6_0_ext_prepare(struct drm_encoder *encoder)
{

}

static void dce_v6_0_ext_commit(struct drm_encoder *encoder)
{

}

static void
dce_v6_0_ext_mode_set(struct drm_encoder *encoder,
		      struct drm_display_mode *mode,
		      struct drm_display_mode *adjusted_mode)
{

}

static void dce_v6_0_ext_disable(struct drm_encoder *encoder)
{

}

static void
dce_v6_0_ext_dpms(struct drm_encoder *encoder, int mode)
{

}

static bool dce_v6_0_ext_mode_fixup(struct drm_encoder *encoder,
				    const struct drm_display_mode *mode,
				    struct drm_display_mode *adjusted_mode)
{
	return true;
}

static const struct drm_encoder_helper_funcs dce_v6_0_ext_helper_funcs = {
	.dpms = dce_v6_0_ext_dpms,
	.mode_fixup = dce_v6_0_ext_mode_fixup,
	.prepare = dce_v6_0_ext_prepare,
	.mode_set = dce_v6_0_ext_mode_set,
	.commit = dce_v6_0_ext_commit,
	.disable = dce_v6_0_ext_disable,
	/* no detect for TMDS/LVDS yet */
};

static const struct drm_encoder_helper_funcs dce_v6_0_dig_helper_funcs = {
	.dpms = amdgpu_atombios_encoder_dpms,
	.mode_fixup = amdgpu_atombios_encoder_mode_fixup,
	.prepare = dce_v6_0_encoder_prepare,
	.mode_set = dce_v6_0_encoder_mode_set,
	.commit = dce_v6_0_encoder_commit,
	.disable = dce_v6_0_encoder_disable,
	.detect = amdgpu_atombios_encoder_dig_detect,
};

static const struct drm_encoder_helper_funcs dce_v6_0_dac_helper_funcs = {
	.dpms = amdgpu_atombios_encoder_dpms,
	.mode_fixup = amdgpu_atombios_encoder_mode_fixup,
	.prepare = dce_v6_0_encoder_prepare,
	.mode_set = dce_v6_0_encoder_mode_set,
	.commit = dce_v6_0_encoder_commit,
	.detect = amdgpu_atombios_encoder_dac_detect,
};

static void dce_v6_0_encoder_destroy(struct drm_encoder *encoder)
{
	struct amdgpu_encoder *amdgpu_encoder = to_amdgpu_encoder(encoder);
	if (amdgpu_encoder->devices & (ATOM_DEVICE_LCD_SUPPORT))
		amdgpu_atombios_encoder_fini_backlight(amdgpu_encoder);
	kfree(amdgpu_encoder->enc_priv);
	drm_encoder_cleanup(encoder);
	kfree(amdgpu_encoder);
}

static const struct drm_encoder_funcs dce_v6_0_encoder_funcs = {
	.destroy = dce_v6_0_encoder_destroy,
};

static void dce_v6_0_encoder_add(struct amdgpu_device *adev,
				 uint32_t encoder_enum,
				 uint32_t supported_device,
				 u16 caps)
{
	struct drm_device *dev = adev->ddev;
	struct drm_encoder *encoder;
	struct amdgpu_encoder *amdgpu_encoder;

	/* see if we already added it */
	list_for_each_entry(encoder, &dev->mode_config.encoder_list, head) {
		amdgpu_encoder = to_amdgpu_encoder(encoder);
		if (amdgpu_encoder->encoder_enum == encoder_enum) {
			amdgpu_encoder->devices |= supported_device;
			return;
		}

	}

	/* add a new one */
	amdgpu_encoder = kzalloc(sizeof(struct amdgpu_encoder), GFP_KERNEL);
	if (!amdgpu_encoder)
		return;

	encoder = &amdgpu_encoder->base;
	switch (adev->mode_info.num_crtc) {
	case 1:
		encoder->possible_crtcs = 0x1;
		break;
	case 2:
	default:
		encoder->possible_crtcs = 0x3;
		break;
	case 4:
		encoder->possible_crtcs = 0xf;
		break;
	case 6:
		encoder->possible_crtcs = 0x3f;
		break;
	}

	amdgpu_encoder->enc_priv = NULL;

	amdgpu_encoder->encoder_enum = encoder_enum;
	amdgpu_encoder->encoder_id = (encoder_enum & OBJECT_ID_MASK) >> OBJECT_ID_SHIFT;
	amdgpu_encoder->devices = supported_device;
	amdgpu_encoder->rmx_type = RMX_OFF;
	amdgpu_encoder->underscan_type = UNDERSCAN_OFF;
	amdgpu_encoder->is_ext_encoder = false;
	amdgpu_encoder->caps = caps;

	switch (amdgpu_encoder->encoder_id) {
	case ENCODER_OBJECT_ID_INTERNAL_KLDSCP_DAC1:
	case ENCODER_OBJECT_ID_INTERNAL_KLDSCP_DAC2:
		drm_encoder_init(dev, encoder, &dce_v6_0_encoder_funcs,
				 DRM_MODE_ENCODER_DAC, NULL);
		drm_encoder_helper_add(encoder, &dce_v6_0_dac_helper_funcs);
		break;
	case ENCODER_OBJECT_ID_INTERNAL_KLDSCP_DVO1:
	case ENCODER_OBJECT_ID_INTERNAL_UNIPHY:
	case ENCODER_OBJECT_ID_INTERNAL_UNIPHY1:
	case ENCODER_OBJECT_ID_INTERNAL_UNIPHY2:
	case ENCODER_OBJECT_ID_INTERNAL_UNIPHY3:
		if (amdgpu_encoder->devices & (ATOM_DEVICE_LCD_SUPPORT)) {
			amdgpu_encoder->rmx_type = RMX_FULL;
			drm_encoder_init(dev, encoder, &dce_v6_0_encoder_funcs,
					 DRM_MODE_ENCODER_LVDS, NULL);
			amdgpu_encoder->enc_priv = amdgpu_atombios_encoder_get_lcd_info(amdgpu_encoder);
		} else if (amdgpu_encoder->devices & (ATOM_DEVICE_CRT_SUPPORT)) {
			drm_encoder_init(dev, encoder, &dce_v6_0_encoder_funcs,
					 DRM_MODE_ENCODER_DAC, NULL);
			amdgpu_encoder->enc_priv = amdgpu_atombios_encoder_get_dig_info(amdgpu_encoder);
		} else {
			drm_encoder_init(dev, encoder, &dce_v6_0_encoder_funcs,
					 DRM_MODE_ENCODER_TMDS, NULL);
			amdgpu_encoder->enc_priv = amdgpu_atombios_encoder_get_dig_info(amdgpu_encoder);
		}
		drm_encoder_helper_add(encoder, &dce_v6_0_dig_helper_funcs);
		break;
	case ENCODER_OBJECT_ID_SI170B:
	case ENCODER_OBJECT_ID_CH7303:
	case ENCODER_OBJECT_ID_EXTERNAL_SDVOA:
	case ENCODER_OBJECT_ID_EXTERNAL_SDVOB:
	case ENCODER_OBJECT_ID_TITFP513:
	case ENCODER_OBJECT_ID_VT1623:
	case ENCODER_OBJECT_ID_HDMI_SI1930:
	case ENCODER_OBJECT_ID_TRAVIS:
	case ENCODER_OBJECT_ID_NUTMEG:
		/* these are handled by the primary encoders */
		amdgpu_encoder->is_ext_encoder = true;
		if (amdgpu_encoder->devices & (ATOM_DEVICE_LCD_SUPPORT))
			drm_encoder_init(dev, encoder, &dce_v6_0_encoder_funcs,
					 DRM_MODE_ENCODER_LVDS, NULL);
		else if (amdgpu_encoder->devices & (ATOM_DEVICE_CRT_SUPPORT))
			drm_encoder_init(dev, encoder, &dce_v6_0_encoder_funcs,
					 DRM_MODE_ENCODER_DAC, NULL);
		else
			drm_encoder_init(dev, encoder, &dce_v6_0_encoder_funcs,
					 DRM_MODE_ENCODER_TMDS, NULL);
		drm_encoder_helper_add(encoder, &dce_v6_0_ext_helper_funcs);
		break;
	}
}

static const struct amdgpu_display_funcs dce_v6_0_display_funcs = {
	.set_vga_render_state = &dce_v6_0_set_vga_render_state,
	.bandwidth_update = &dce_v6_0_bandwidth_update,
	.vblank_get_counter = &dce_v6_0_vblank_get_counter,
	.vblank_wait = &dce_v6_0_vblank_wait,
	.is_display_hung = &dce_v6_0_is_display_hung,
	.backlight_set_level = &amdgpu_atombios_encoder_set_backlight_level,
	.backlight_get_level = &amdgpu_atombios_encoder_get_backlight_level,
	.hpd_sense = &dce_v6_0_hpd_sense,
	.hpd_set_polarity = &dce_v6_0_hpd_set_polarity,
	.hpd_get_gpio_reg = &dce_v6_0_hpd_get_gpio_reg,
	.page_flip = &dce_v6_0_page_flip,
	.page_flip_get_scanoutpos = &dce_v6_0_crtc_get_scanoutpos,
	.add_encoder = &dce_v6_0_encoder_add,
	.add_connector = &amdgpu_connector_add,
	.stop_mc_access = &dce_v6_0_stop_mc_access,
	.resume_mc_access = &dce_v6_0_resume_mc_access,
};

static void dce_v6_0_set_display_funcs(struct amdgpu_device *adev)
{
	if (adev->mode_info.funcs == NULL)
		adev->mode_info.funcs = &dce_v6_0_display_funcs;
}

static const struct amdgpu_irq_src_funcs dce_v6_0_crtc_irq_funcs = {
	.set = dce_v6_0_set_crtc_interrupt_state,
	.process = dce_v6_0_crtc_irq,
};

static const struct amdgpu_irq_src_funcs dce_v6_0_pageflip_irq_funcs = {
	.set = dce_v6_0_set_pageflip_interrupt_state,
	.process = dce_v6_0_pageflip_irq,
};

static const struct amdgpu_irq_src_funcs dce_v6_0_hpd_irq_funcs = {
	.set = dce_v6_0_set_hpd_interrupt_state,
	.process = dce_v6_0_hpd_irq,
};

static void dce_v6_0_set_irq_funcs(struct amdgpu_device *adev)
{
	adev->crtc_irq.num_types = adev->mode_info.num_crtc;
	adev->crtc_irq.funcs = &dce_v6_0_crtc_irq_funcs;

	adev->pageflip_irq.num_types = adev->mode_info.num_crtc;
	adev->pageflip_irq.funcs = &dce_v6_0_pageflip_irq_funcs;

	adev->hpd_irq.num_types = adev->mode_info.num_crtc;
	adev->hpd_irq.funcs = &dce_v6_0_hpd_irq_funcs;
}
