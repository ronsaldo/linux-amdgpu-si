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
#include "clearstate_si.h"
#include "si_blit_shaders.h"

#include "gfx_v6_0.h"

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

#include "uvd/uvd_3_2_d.h"

#define GFX6_NUM_GFX_RINGS 1
#define GFX6_NUM_COMPUTE_RINGS 2

static void gfx_v6_0_set_ring_funcs(struct amdgpu_device *adev);
static void gfx_v6_0_set_irq_funcs(struct amdgpu_device *adev);
static void gfx_v6_0_set_gds_init(struct amdgpu_device *adev);

MODULE_FIRMWARE("radeon/tahiti_pfp.bin");
MODULE_FIRMWARE("radeon/tahiti_me.bin");
MODULE_FIRMWARE("radeon/tahiti_ce.bin");
MODULE_FIRMWARE("radeon/tahiti_mc.bin");
MODULE_FIRMWARE("radeon/tahiti_rlc.bin");
MODULE_FIRMWARE("radeon/tahiti_smc.bin");

MODULE_FIRMWARE("radeon/pitcairn_pfp.bin");
MODULE_FIRMWARE("radeon/pitcairn_me.bin");
MODULE_FIRMWARE("radeon/pitcairn_ce.bin");
MODULE_FIRMWARE("radeon/pitcairn_mc.bin");
MODULE_FIRMWARE("radeon/pitcairn_rlc.bin");
MODULE_FIRMWARE("radeon/pitcairn_smc.bin");

MODULE_FIRMWARE("radeon/verde_pfp.bin");
MODULE_FIRMWARE("radeon/verde_me.bin");
MODULE_FIRMWARE("radeon/verde_ce.bin");
MODULE_FIRMWARE("radeon/verde_mc.bin");
MODULE_FIRMWARE("radeon/verde_rlc.bin");
MODULE_FIRMWARE("radeon/verde_smc.bin");

MODULE_FIRMWARE("radeon/oland_pfp.bin");
MODULE_FIRMWARE("radeon/oland_me.bin");
MODULE_FIRMWARE("radeon/oland_ce.bin");
MODULE_FIRMWARE("radeon/oland_mc.bin");
MODULE_FIRMWARE("radeon/oland_rlc.bin");
MODULE_FIRMWARE("radeon/oland_smc.bin");

MODULE_FIRMWARE("radeon/hainan_pfp.bin");
MODULE_FIRMWARE("radeon/hainan_me.bin");
MODULE_FIRMWARE("radeon/hainan_ce.bin");
MODULE_FIRMWARE("radeon/hainan_mc.bin");
MODULE_FIRMWARE("radeon/hainan_rlc.bin");
MODULE_FIRMWARE("radeon/hainan_smc.bin");

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


static u32 gfx_v6_0_get_csb_size(struct amdgpu_device *adev);
static void gfx_v6_0_get_csb_buffer(struct amdgpu_device *adev, volatile u32 *buffer);
static void gfx_v6_0_init_pg(struct amdgpu_device *adev);

/*
 * Core functions
 */
/**
 * gfx_v6_0_init_microcode - load ucode images from disk
 *
 * @adev: amdgpu_device pointer
 *
 * Use the firmware interface to load the ucode images into
 * the driver (not loaded into hw).
 * Returns 0 on success, error on failure.
 */
static int gfx_v6_0_init_microcode(struct amdgpu_device *adev)
{
	const char *chip_name;
	char fw_name[30];
	int err;

	DRM_DEBUG("\n");

	switch (adev->asic_type) {
	case CHIP_TAHITI:
		chip_name = "tahiti";
		break;
	case CHIP_PITCAIRN:
        chip_name = "pitcairn";
		break;
	case CHIP_VERDE:
        chip_name = "verde";
        break;
	case CHIP_OLAND:
        chip_name = "oland";
        break;
    case CHIP_HAINAN:
        chip_name = "hainan";
        break;
	default: BUG();
	}


	snprintf(fw_name, sizeof(fw_name), "radeon/%s_pfp.bin", chip_name);
	err = request_firmware(&adev->gfx.pfp_fw, fw_name, adev->dev);
	if (err)
		goto out;
	err = amdgpu_ucode_validate(adev->gfx.pfp_fw);
	if (err)
		goto out;

	snprintf(fw_name, sizeof(fw_name), "radeon/%s_me.bin", chip_name);
	err = request_firmware(&adev->gfx.me_fw, fw_name, adev->dev);
	if (err)
		goto out;
	err = amdgpu_ucode_validate(adev->gfx.me_fw);
	if (err)
		goto out;

	snprintf(fw_name, sizeof(fw_name), "radeon/%s_ce.bin", chip_name);
	err = request_firmware(&adev->gfx.ce_fw, fw_name, adev->dev);
	if (err)
		goto out;
	err = amdgpu_ucode_validate(adev->gfx.ce_fw);
	if (err)
		goto out;

	snprintf(fw_name, sizeof(fw_name), "radeon/%s_rlc.bin", chip_name);
	err = request_firmware(&adev->gfx.rlc_fw, fw_name, adev->dev);
	if (err)
		goto out;
	err = amdgpu_ucode_validate(adev->gfx.rlc_fw);

out:
	if (err) {
		printk(KERN_ERR
		       "gfx6: Failed to load firmware \"%s\"\n",
		       fw_name);
		release_firmware(adev->gfx.pfp_fw);
		adev->gfx.pfp_fw = NULL;
		release_firmware(adev->gfx.me_fw);
		adev->gfx.me_fw = NULL;
		release_firmware(adev->gfx.ce_fw);
		adev->gfx.ce_fw = NULL;
		release_firmware(adev->gfx.rlc_fw);
		adev->gfx.rlc_fw = NULL;
	}
	return err;
}

static void gfx_v6_0_tiling_mode_table_init(struct amdgpu_device *adev)
{
	const u32 num_tile_mode_states = 32;
	u32 reg_offset, gb_tile_moden, split_equal_to_row_size;

	switch (adev->gfx.config.mem_row_size_in_kb) {
	case 1:
		split_equal_to_row_size = ADDR_SURF_TILE_SPLIT_1KB;
		break;
	case 2:
	default:
		split_equal_to_row_size = ADDR_SURF_TILE_SPLIT_2KB;
		break;
	case 4:
		split_equal_to_row_size = ADDR_SURF_TILE_SPLIT_4KB;
		break;
	}

	if ((adev->asic_type == CHIP_TAHITI) ||
	    (adev->asic_type == CHIP_PITCAIRN)) {
		for (reg_offset = 0; reg_offset < num_tile_mode_states; reg_offset++) {
			switch (reg_offset) {
			case 0:  /* non-AA compressed depth or any compressed stencil */
				gb_tile_moden = (ARRAY_MODE(ARRAY_2D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_DEPTH_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P8_32x32_8x16) |
						 TILE_SPLIT(ADDR_SURF_TILE_SPLIT_64B) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_4) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_2));
				break;
			case 1:  /* 2xAA/4xAA compressed depth only */
				gb_tile_moden = (ARRAY_MODE(ARRAY_2D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_DEPTH_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P8_32x32_8x16) |
						 TILE_SPLIT(ADDR_SURF_TILE_SPLIT_128B) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_4) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_2));
				break;
			case 2:  /* 8xAA compressed depth only */
				gb_tile_moden = (ARRAY_MODE(ARRAY_2D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_DEPTH_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P8_32x32_8x16) |
						 TILE_SPLIT(ADDR_SURF_TILE_SPLIT_256B) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_4) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_2));
				break;
			case 3:  /* 2xAA/4xAA compressed depth with stencil (for depth buffer) */
				gb_tile_moden = (ARRAY_MODE(ARRAY_2D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_DEPTH_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P8_32x32_8x16) |
						 TILE_SPLIT(ADDR_SURF_TILE_SPLIT_128B) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_4) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_2));
				break;
			case 4:  /* Maps w/ a dimension less than the 2D macro-tile dimensions (for mipmapped depth textures) */
				gb_tile_moden = (ARRAY_MODE(ARRAY_1D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_DEPTH_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P8_32x32_8x16) |
						 TILE_SPLIT(ADDR_SURF_TILE_SPLIT_64B) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_2) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_2));
				break;
			case 5:  /* Uncompressed 16bpp depth - and stencil buffer allocated with it */
				gb_tile_moden = (ARRAY_MODE(ARRAY_2D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_DEPTH_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P8_32x32_8x16) |
						 TILE_SPLIT(split_equal_to_row_size) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_2) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_2));
				break;
			case 6:  /* Uncompressed 32bpp depth - and stencil buffer allocated with it */
				gb_tile_moden = (ARRAY_MODE(ARRAY_2D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_DEPTH_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P8_32x32_8x16) |
						 TILE_SPLIT(split_equal_to_row_size) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_1) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_1));
				break;
			case 7:  /* Uncompressed 8bpp stencil without depth (drivers typically do not use) */
				gb_tile_moden = (ARRAY_MODE(ARRAY_2D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_DEPTH_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P8_32x32_8x16) |
						 TILE_SPLIT(split_equal_to_row_size) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_4) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_2));
				break;
			case 8:  /* 1D and 1D Array Surfaces */
				gb_tile_moden = (ARRAY_MODE(ARRAY_LINEAR_ALIGNED) |
						 MICRO_TILE_MODE(ADDR_SURF_DISPLAY_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P8_32x32_8x16) |
						 TILE_SPLIT(ADDR_SURF_TILE_SPLIT_64B) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_2) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_2));
				break;
			case 9:  /* Displayable maps. */
				gb_tile_moden = (ARRAY_MODE(ARRAY_1D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_DISPLAY_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P8_32x32_8x16) |
						 TILE_SPLIT(ADDR_SURF_TILE_SPLIT_64B) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_2) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_2));
				break;
			case 10:  /* Display 8bpp. */
				gb_tile_moden = (ARRAY_MODE(ARRAY_2D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_DISPLAY_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P8_32x32_8x16) |
						 TILE_SPLIT(ADDR_SURF_TILE_SPLIT_256B) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_4) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_2));
				break;
			case 11:  /* Display 16bpp. */
				gb_tile_moden = (ARRAY_MODE(ARRAY_2D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_DISPLAY_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P8_32x32_8x16) |
						 TILE_SPLIT(ADDR_SURF_TILE_SPLIT_256B) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_2) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_2));
				break;
			case 12:  /* Display 32bpp. */
				gb_tile_moden = (ARRAY_MODE(ARRAY_2D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_DISPLAY_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P8_32x32_8x16) |
						 TILE_SPLIT(ADDR_SURF_TILE_SPLIT_512B) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_1) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_1));
				break;
			case 13:  /* Thin. */
				gb_tile_moden = (ARRAY_MODE(ARRAY_1D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_THIN_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P8_32x32_8x16) |
						 TILE_SPLIT(ADDR_SURF_TILE_SPLIT_64B) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_2) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_2));
				break;
			case 14:  /* Thin 8 bpp. */
				gb_tile_moden = (ARRAY_MODE(ARRAY_2D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_THIN_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P8_32x32_8x16) |
						 TILE_SPLIT(ADDR_SURF_TILE_SPLIT_256B) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_4) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_1));
				break;
			case 15:  /* Thin 16 bpp. */
				gb_tile_moden = (ARRAY_MODE(ARRAY_2D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_THIN_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P8_32x32_8x16) |
						 TILE_SPLIT(ADDR_SURF_TILE_SPLIT_256B) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_2) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_1));
				break;
			case 16:  /* Thin 32 bpp. */
				gb_tile_moden = (ARRAY_MODE(ARRAY_2D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_THIN_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P8_32x32_8x16) |
						 TILE_SPLIT(ADDR_SURF_TILE_SPLIT_512B) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_1) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_1));
				break;
			case 17:  /* Thin 64 bpp. */
				gb_tile_moden = (ARRAY_MODE(ARRAY_2D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_THIN_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P8_32x32_8x16) |
						 TILE_SPLIT(split_equal_to_row_size) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_1) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_1));
				break;
			case 21:  /* 8 bpp PRT. */
				gb_tile_moden = (ARRAY_MODE(ARRAY_2D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_THIN_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P8_32x32_8x16) |
						 TILE_SPLIT(ADDR_SURF_TILE_SPLIT_256B) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_2) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_4) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_2));
				break;
			case 22:  /* 16 bpp PRT */
				gb_tile_moden = (ARRAY_MODE(ARRAY_2D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_THIN_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P8_32x32_8x16) |
						 TILE_SPLIT(ADDR_SURF_TILE_SPLIT_256B) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_4) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_4));
				break;
			case 23:  /* 32 bpp PRT */
				gb_tile_moden = (ARRAY_MODE(ARRAY_2D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_THIN_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P8_32x32_8x16) |
						 TILE_SPLIT(ADDR_SURF_TILE_SPLIT_256B) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_2) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_2));
				break;
			case 24:  /* 64 bpp PRT */
				gb_tile_moden = (ARRAY_MODE(ARRAY_2D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_THIN_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P8_32x32_8x16) |
						 TILE_SPLIT(ADDR_SURF_TILE_SPLIT_512B) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_1) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_2));
				break;
			case 25:  /* 128 bpp PRT */
				gb_tile_moden = (ARRAY_MODE(ARRAY_2D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_THIN_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P8_32x32_8x16) |
						 TILE_SPLIT(ADDR_SURF_TILE_SPLIT_1KB) |
						 NUM_BANKS(ADDR_SURF_8_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_1) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_1));
				break;
			default:
				gb_tile_moden = 0;
				break;
			}
			adev->gfx.config.tile_mode_array[reg_offset] = gb_tile_moden;
			WREG32(mmGB_TILE_MODE0 + reg_offset, gb_tile_moden);
		}
	} else if ((adev->asic_type == CHIP_VERDE) ||
		   (adev->asic_type == CHIP_OLAND) ||
		   (adev->asic_type == CHIP_HAINAN)) {
		for (reg_offset = 0; reg_offset < num_tile_mode_states; reg_offset++) {
			switch (reg_offset) {
			case 0:  /* non-AA compressed depth or any compressed stencil */
				gb_tile_moden = (ARRAY_MODE(ARRAY_2D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_DEPTH_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P4_8x16) |
						 TILE_SPLIT(ADDR_SURF_TILE_SPLIT_64B) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_4) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_4));
				break;
			case 1:  /* 2xAA/4xAA compressed depth only */
				gb_tile_moden = (ARRAY_MODE(ARRAY_2D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_DEPTH_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P4_8x16) |
						 TILE_SPLIT(ADDR_SURF_TILE_SPLIT_128B) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_4) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_4));
				break;
			case 2:  /* 8xAA compressed depth only */
				gb_tile_moden = (ARRAY_MODE(ARRAY_2D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_DEPTH_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P4_8x16) |
						 TILE_SPLIT(ADDR_SURF_TILE_SPLIT_256B) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_4) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_4));
				break;
			case 3:  /* 2xAA/4xAA compressed depth with stencil (for depth buffer) */
				gb_tile_moden = (ARRAY_MODE(ARRAY_2D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_DEPTH_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P4_8x16) |
						 TILE_SPLIT(ADDR_SURF_TILE_SPLIT_128B) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_4) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_4));
				break;
			case 4:  /* Maps w/ a dimension less than the 2D macro-tile dimensions (for mipmapped depth textures) */
				gb_tile_moden = (ARRAY_MODE(ARRAY_1D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_DEPTH_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P4_8x16) |
						 TILE_SPLIT(ADDR_SURF_TILE_SPLIT_64B) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_2) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_2));
				break;
			case 5:  /* Uncompressed 16bpp depth - and stencil buffer allocated with it */
				gb_tile_moden = (ARRAY_MODE(ARRAY_2D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_DEPTH_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P4_8x16) |
						 TILE_SPLIT(split_equal_to_row_size) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_2) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_2));
				break;
			case 6:  /* Uncompressed 32bpp depth - and stencil buffer allocated with it */
				gb_tile_moden = (ARRAY_MODE(ARRAY_2D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_DEPTH_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P4_8x16) |
						 TILE_SPLIT(split_equal_to_row_size) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_1) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_2));
				break;
			case 7:  /* Uncompressed 8bpp stencil without depth (drivers typically do not use) */
				gb_tile_moden = (ARRAY_MODE(ARRAY_2D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_DEPTH_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P4_8x16) |
						 TILE_SPLIT(split_equal_to_row_size) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_4) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_4));
				break;
			case 8:  /* 1D and 1D Array Surfaces */
				gb_tile_moden = (ARRAY_MODE(ARRAY_LINEAR_ALIGNED) |
						 MICRO_TILE_MODE(ADDR_SURF_DISPLAY_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P4_8x16) |
						 TILE_SPLIT(ADDR_SURF_TILE_SPLIT_64B) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_2) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_2));
				break;
			case 9:  /* Displayable maps. */
				gb_tile_moden = (ARRAY_MODE(ARRAY_1D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_DISPLAY_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P4_8x16) |
						 TILE_SPLIT(ADDR_SURF_TILE_SPLIT_64B) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_2) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_2));
				break;
			case 10:  /* Display 8bpp. */
				gb_tile_moden = (ARRAY_MODE(ARRAY_2D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_DISPLAY_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P4_8x16) |
						 TILE_SPLIT(ADDR_SURF_TILE_SPLIT_256B) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_4) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_4));
				break;
			case 11:  /* Display 16bpp. */
				gb_tile_moden = (ARRAY_MODE(ARRAY_2D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_DISPLAY_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P4_8x16) |
						 TILE_SPLIT(ADDR_SURF_TILE_SPLIT_256B) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_2) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_2));
				break;
			case 12:  /* Display 32bpp. */
				gb_tile_moden = (ARRAY_MODE(ARRAY_2D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_DISPLAY_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P4_8x16) |
						 TILE_SPLIT(ADDR_SURF_TILE_SPLIT_512B) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_1) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_2));
				break;
			case 13:  /* Thin. */
				gb_tile_moden = (ARRAY_MODE(ARRAY_1D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_THIN_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P4_8x16) |
						 TILE_SPLIT(ADDR_SURF_TILE_SPLIT_64B) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_2) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_2));
				break;
			case 14:  /* Thin 8 bpp. */
				gb_tile_moden = (ARRAY_MODE(ARRAY_2D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_THIN_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P4_8x16) |
						 TILE_SPLIT(ADDR_SURF_TILE_SPLIT_256B) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_4) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_2));
				break;
			case 15:  /* Thin 16 bpp. */
				gb_tile_moden = (ARRAY_MODE(ARRAY_2D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_THIN_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P4_8x16) |
						 TILE_SPLIT(ADDR_SURF_TILE_SPLIT_256B) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_2) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_2));
				break;
			case 16:  /* Thin 32 bpp. */
				gb_tile_moden = (ARRAY_MODE(ARRAY_2D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_THIN_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P4_8x16) |
						 TILE_SPLIT(ADDR_SURF_TILE_SPLIT_512B) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_1) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_2));
				break;
			case 17:  /* Thin 64 bpp. */
				gb_tile_moden = (ARRAY_MODE(ARRAY_2D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_THIN_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P4_8x16) |
						 TILE_SPLIT(split_equal_to_row_size) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_1) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_2));
				break;
			case 21:  /* 8 bpp PRT. */
				gb_tile_moden = (ARRAY_MODE(ARRAY_2D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_THIN_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P8_32x32_8x16) |
						 TILE_SPLIT(ADDR_SURF_TILE_SPLIT_256B) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_2) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_4) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_2));
				break;
			case 22:  /* 16 bpp PRT */
				gb_tile_moden = (ARRAY_MODE(ARRAY_2D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_THIN_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P8_32x32_8x16) |
						 TILE_SPLIT(ADDR_SURF_TILE_SPLIT_256B) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_4) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_4));
				break;
			case 23:  /* 32 bpp PRT */
				gb_tile_moden = (ARRAY_MODE(ARRAY_2D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_THIN_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P8_32x32_8x16) |
						 TILE_SPLIT(ADDR_SURF_TILE_SPLIT_256B) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_2) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_2));
				break;
			case 24:  /* 64 bpp PRT */
				gb_tile_moden = (ARRAY_MODE(ARRAY_2D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_THIN_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P8_32x32_8x16) |
						 TILE_SPLIT(ADDR_SURF_TILE_SPLIT_512B) |
						 NUM_BANKS(ADDR_SURF_16_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_1) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_2));
				break;
			case 25:  /* 128 bpp PRT */
				gb_tile_moden = (ARRAY_MODE(ARRAY_2D_TILED_THIN1) |
						 MICRO_TILE_MODE(ADDR_SURF_THIN_MICRO_TILING) |
						 PIPE_CONFIG(ADDR_SURF_P8_32x32_8x16) |
						 TILE_SPLIT(ADDR_SURF_TILE_SPLIT_1KB) |
						 NUM_BANKS(ADDR_SURF_8_BANK) |
						 BANK_WIDTH(ADDR_SURF_BANK_WIDTH_1) |
						 BANK_HEIGHT(ADDR_SURF_BANK_HEIGHT_1) |
						 MACRO_TILE_ASPECT(ADDR_SURF_MACRO_ASPECT_1));
				break;
			default:
				gb_tile_moden = 0;
				break;
			}
			adev->gfx.config.tile_mode_array[reg_offset] = gb_tile_moden;
			WREG32(mmGB_TILE_MODE0 + reg_offset, gb_tile_moden);
		}
	} else
		DRM_ERROR("unknown asic: 0x%x\n", adev->asic_type);
}

/**
 * gfx_v6_0_select_se_sh - select which SE, SH to address
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

static u32 gfx_v6_0_create_bitmask(u32 bit_width)
{
	u32 i, mask = 0;

	for (i = 0; i < bit_width; i++) {
		mask <<= 1;
		mask |= 1;
	}
	return mask;
}

static u32 gfx_v6_0_get_rb_disabled(struct amdgpu_device *adev,
			      u32 max_rb_num_per_se,
			      u32 sh_per_se)
{
	u32 data, mask;

	data = RREG32(mmCC_RB_BACKEND_DISABLE);
	if (data & 1)
		data &= GC_USER_RB_BACKEND_DISABLE__BACKEND_DISABLE_MASK;
	else
		data = 0;
	data |= RREG32(mmGC_USER_RB_BACKEND_DISABLE);

	data >>= GC_USER_RB_BACKEND_DISABLE__BACKEND_DISABLE__SHIFT;

	mask = gfx_v6_0_create_bitmask(max_rb_num_per_se / sh_per_se);

	return data & mask;
}

static void gfx_v6_0_setup_rb(struct amdgpu_device *adev,
			u32 se_num, u32 sh_per_se,
			u32 max_rb_num_per_se)
{
	int i, j;
	u32 data, mask;
	u32 disabled_rbs = 0;
	u32 enabled_rbs = 0;

	mutex_lock(&adev->grbm_idx_mutex);
	for (i = 0; i < se_num; i++) {
		for (j = 0; j < sh_per_se; j++) {
			gfx_v6_0_select_se_sh(adev, i, j);
			data = gfx_v6_0_get_rb_disabled(adev, max_rb_num_per_se, sh_per_se);
			disabled_rbs |= data << ((i * sh_per_se + j) * TAHITI_RB_BITMAP_WIDTH_PER_SH);
		}
	}
	gfx_v6_0_select_se_sh(adev, 0xffffffff, 0xffffffff);
	mutex_unlock(&adev->grbm_idx_mutex);

	mask = 1;
	for (i = 0; i < max_rb_num_per_se * se_num; i++) {
		if (!(disabled_rbs & mask))
			enabled_rbs |= mask;
		mask <<= 1;
	}

	adev->gfx.config.backend_enable_mask = enabled_rbs;

	mutex_lock(&adev->grbm_idx_mutex);
	for (i = 0; i < se_num; i++) {
		gfx_v6_0_select_se_sh(adev, i, 0xffffffff);
		data = 0;
		for (j = 0; j < sh_per_se; j++) {
			switch (enabled_rbs & 3) {
			case 1:
				data |= (PA_SC_RASTER_CONFIG__RASTER_CONFIG_RB_MAP_0 << (i * sh_per_se + j) * 2);
				break;
			case 2:
				data |= (PA_SC_RASTER_CONFIG__RASTER_CONFIG_RB_MAP_3 << (i * sh_per_se + j) * 2);
				break;
			case 3:
			default:
				data |= (PA_SC_RASTER_CONFIG__RASTER_CONFIG_RB_MAP_2 << (i * sh_per_se + j) * 2);
				break;
			}
			enabled_rbs >>= 2;
		}
		WREG32(mmPA_SC_RASTER_CONFIG, data);
	}
	gfx_v6_0_select_se_sh(adev, 0xffffffff, 0xffffffff);
	mutex_unlock(&adev->grbm_idx_mutex);
}

static u32 gfx_v6_0_get_cu_enabled(struct amdgpu_device *adev, u32 cu_per_sh)
{
	u32 data, mask;

	data = RREG32(mmCC_GC_SHADER_ARRAY_CONFIG);
	if (data & 1)
		data &= CC_GC_SHADER_ARRAY_CONFIG__INACTIVE_CUS_MASK;
	else
		data = 0;
	data |= RREG32(mmGC_USER_SHADER_ARRAY_CONFIG);

	data >>= CC_GC_SHADER_ARRAY_CONFIG__INACTIVE_CUS__SHIFT;

	mask = gfx_v6_0_create_bitmask(cu_per_sh);

	return ~data & mask;
}

static void gfx_v6_0_setup_spi(struct amdgpu_device *adev,
			 u32 se_num, u32 sh_per_se,
			 u32 cu_per_sh)
{
	int i, j, k;
	u32 data, mask, active_cu;

	mutex_lock(&adev->grbm_idx_mutex);
	for (i = 0; i < se_num; i++) {
		for (j = 0; j < sh_per_se; j++) {
			gfx_v6_0_select_se_sh(adev, i, j);
			data = RREG32(mmSPI_STATIC_THREAD_MGMT_3);
			active_cu = gfx_v6_0_get_cu_enabled(adev, cu_per_sh);

			mask = 1;
			for (k = 0; k < 16; k++) {
				mask <<= k;
				if (active_cu & mask) {
					data &= ~mask;
					WREG32(mmSPI_STATIC_THREAD_MGMT_3, data);
					break;
				}
			}
		}
	}
	gfx_v6_0_select_se_sh(adev, 0xffffffff, 0xffffffff);
	mutex_unlock(&adev->grbm_idx_mutex);
}

static void gfx_v6_0_gpu_init(struct amdgpu_device *adev)
{
	u32 gb_addr_config = 0;
	u32 mc_shared_chmap, mc_arb_ramcfg;
	u32 sx_debug_1;
	u32 hdp_host_path_cntl;
	u32 tmp;
	int i, j;

	switch (adev->asic_type) {
	case CHIP_TAHITI:
		adev->gfx.config.max_shader_engines = 2;
		adev->gfx.config.max_tile_pipes = 12;
		adev->gfx.config.max_cu_per_sh = 8;
		adev->gfx.config.max_sh_per_se = 2;
		adev->gfx.config.max_backends_per_se = 4;
		adev->gfx.config.max_texture_channel_caches = 12;
		adev->gfx.config.max_gprs = 256;
		adev->gfx.config.max_gs_threads = 32;
		adev->gfx.config.max_hw_contexts = 8;

		adev->gfx.config.sc_prim_fifo_size_frontend = 0x20;
		adev->gfx.config.sc_prim_fifo_size_backend = 0x100;
		adev->gfx.config.sc_hiz_tile_fifo_size = 0x30;
		adev->gfx.config.sc_earlyz_tile_fifo_size = 0x130;
		gb_addr_config = TAHITI_GB_ADDR_CONFIG_GOLDEN;
		break;
	case CHIP_PITCAIRN:
		adev->gfx.config.max_shader_engines = 2;
		adev->gfx.config.max_tile_pipes = 8;
		adev->gfx.config.max_cu_per_sh = 5;
		adev->gfx.config.max_sh_per_se = 2;
		adev->gfx.config.max_backends_per_se = 4;
		adev->gfx.config.max_texture_channel_caches = 8;
		adev->gfx.config.max_gprs = 256;
		adev->gfx.config.max_gs_threads = 32;
		adev->gfx.config.max_hw_contexts = 8;

		adev->gfx.config.sc_prim_fifo_size_frontend = 0x20;
		adev->gfx.config.sc_prim_fifo_size_backend = 0x100;
		adev->gfx.config.sc_hiz_tile_fifo_size = 0x30;
		adev->gfx.config.sc_earlyz_tile_fifo_size = 0x130;
		gb_addr_config = TAHITI_GB_ADDR_CONFIG_GOLDEN;
		break;
	case CHIP_VERDE:
	default:
		adev->gfx.config.max_shader_engines = 1;
		adev->gfx.config.max_tile_pipes = 4;
		adev->gfx.config.max_cu_per_sh = 5;
		adev->gfx.config.max_sh_per_se = 2;
		adev->gfx.config.max_backends_per_se = 4;
		adev->gfx.config.max_texture_channel_caches = 4;
		adev->gfx.config.max_gprs = 256;
		adev->gfx.config.max_gs_threads = 32;
		adev->gfx.config.max_hw_contexts = 8;

		adev->gfx.config.sc_prim_fifo_size_frontend = 0x20;
		adev->gfx.config.sc_prim_fifo_size_backend = 0x40;
		adev->gfx.config.sc_hiz_tile_fifo_size = 0x30;
		adev->gfx.config.sc_earlyz_tile_fifo_size = 0x130;
		gb_addr_config = VERDE_GB_ADDR_CONFIG_GOLDEN;
		break;
	case CHIP_OLAND:
		adev->gfx.config.max_shader_engines = 1;
		adev->gfx.config.max_tile_pipes = 4;
		adev->gfx.config.max_cu_per_sh = 6;
		adev->gfx.config.max_sh_per_se = 1;
		adev->gfx.config.max_backends_per_se = 2;
		adev->gfx.config.max_texture_channel_caches = 4;
		adev->gfx.config.max_gprs = 256;
		adev->gfx.config.max_gs_threads = 16;
		adev->gfx.config.max_hw_contexts = 8;

		adev->gfx.config.sc_prim_fifo_size_frontend = 0x20;
		adev->gfx.config.sc_prim_fifo_size_backend = 0x40;
		adev->gfx.config.sc_hiz_tile_fifo_size = 0x30;
		adev->gfx.config.sc_earlyz_tile_fifo_size = 0x130;
		gb_addr_config = VERDE_GB_ADDR_CONFIG_GOLDEN;
		break;
	case CHIP_HAINAN:
		adev->gfx.config.max_shader_engines = 1;
		adev->gfx.config.max_tile_pipes = 4;
		adev->gfx.config.max_cu_per_sh = 5;
		adev->gfx.config.max_sh_per_se = 1;
		adev->gfx.config.max_backends_per_se = 1;
		adev->gfx.config.max_texture_channel_caches = 2;
		adev->gfx.config.max_gprs = 256;
		adev->gfx.config.max_gs_threads = 16;
		adev->gfx.config.max_hw_contexts = 8;

		adev->gfx.config.sc_prim_fifo_size_frontend = 0x20;
		adev->gfx.config.sc_prim_fifo_size_backend = 0x40;
		adev->gfx.config.sc_hiz_tile_fifo_size = 0x30;
		adev->gfx.config.sc_earlyz_tile_fifo_size = 0x130;
		gb_addr_config = HAINAN_GB_ADDR_CONFIG_GOLDEN;
		break;
	}

	WREG32(mmGRBM_CNTL, 0xff);
	WREG32(mmSRBM_INT_CNTL, 1);
	WREG32(mmSRBM_INT_ACK, 1);

	mc_shared_chmap = RREG32(mmMC_SHARED_CHMAP);
	adev->gfx.config.mc_arb_ramcfg = RREG32(mmMC_ARB_RAMCFG);
	mc_arb_ramcfg = adev->gfx.config.mc_arb_ramcfg;

	adev->gfx.config.num_tile_pipes = adev->gfx.config.max_tile_pipes;
	adev->gfx.config.mem_max_burst_length_bytes = 256;
	tmp = (mc_arb_ramcfg & MC_ARB_RAMCFG__NOOFCOLS_MASK) >> MC_ARB_RAMCFG__NOOFCOLS__SHIFT;
	adev->gfx.config.mem_row_size_in_kb = (4 * (1 << (8 + tmp))) / 1024;
	if (adev->gfx.config.mem_row_size_in_kb > 4)
		adev->gfx.config.mem_row_size_in_kb = 4;
	/* XXX use MC settings? */
	adev->gfx.config.shader_engine_tile_size = 32;
	adev->gfx.config.num_gpus = 1;
	adev->gfx.config.multi_gpu_tile_size = 64;

	/* fix up row size */
	gb_addr_config &= ~GB_ADDR_CONFIG__ROW_SIZE_MASK;
	switch (adev->gfx.config.mem_row_size_in_kb) {
	case 1:
	default:
		gb_addr_config = REG_SET_FIELD(gb_addr_config, GB_ADDR_CONFIG, ROW_SIZE, 0);
		break;
	case 2:
		gb_addr_config = REG_SET_FIELD(gb_addr_config, GB_ADDR_CONFIG, ROW_SIZE, 1);
		break;
	case 4:
		gb_addr_config = REG_SET_FIELD(gb_addr_config, GB_ADDR_CONFIG, ROW_SIZE, 2);
		break;
	}
	adev->gfx.config.gb_addr_config = gb_addr_config;

	WREG32(mmGB_ADDR_CONFIG, gb_addr_config);
	WREG32(mmHDP_ADDR_CONFIG, gb_addr_config);
	WREG32(mmDMIF_ADDR_CONFIG, gb_addr_config);
	WREG32(mmDMIF_ADDR_CALC, gb_addr_config);
	WREG32(mmDMA_TILING_CONFIG + DMA0_REGISTER_OFFSET, gb_addr_config);
	WREG32(mmDMA_TILING_CONFIG + DMA1_REGISTER_OFFSET, gb_addr_config);
	if (adev->has_uvd) {
		WREG32(mmUVD_UDEC_ADDR_CONFIG, gb_addr_config);
		WREG32(mmUVD_UDEC_DB_ADDR_CONFIG, gb_addr_config);
		WREG32(mmUVD_UDEC_DBW_ADDR_CONFIG, gb_addr_config);
	}

	gfx_v6_0_tiling_mode_table_init(adev);

	gfx_v6_0_setup_rb(adev, adev->gfx.config.max_shader_engines,
		    adev->gfx.config.max_sh_per_se,
		    adev->gfx.config.max_backends_per_se);

	gfx_v6_0_setup_spi(adev, adev->gfx.config.max_shader_engines,
		     adev->gfx.config.max_sh_per_se,
		     adev->gfx.config.max_cu_per_sh);

	/* set HW defaults for 3D engine */
	tmp = REG_SET_FIELD(0, CP_QUEUE_THRESHOLDS, ROQ_IB1_START, 0x16);
	tmp = REG_SET_FIELD(tmp, CP_QUEUE_THRESHOLDS, ROQ_IB2_START, 0x2b);
	WREG32(mmCP_QUEUE_THRESHOLDS, tmp);
	tmp = REG_SET_FIELD(0, CP_MEQ_THRESHOLDS, MEQ1_START, 0x30);
	tmp = REG_SET_FIELD(tmp, CP_MEQ_THRESHOLDS, MEQ2_START, 0x60);
	WREG32(mmCP_MEQ_THRESHOLDS, tmp);

	sx_debug_1 = RREG32(mmSX_DEBUG_1);
	WREG32(mmSX_DEBUG_1, sx_debug_1);

	tmp = REG_SET_FIELD(0, SPI_CONFIG_CNTL_1, VTX_DONE_DELAY, 4);
	WREG32(mmSPI_CONFIG_CNTL_1, tmp);

	tmp = REG_SET_FIELD(0, PA_SC_FIFO_SIZE, SC_FRONTEND_PRIM_FIFO_SIZE, adev->gfx.config.sc_prim_fifo_size_frontend);
	tmp = REG_SET_FIELD(tmp, PA_SC_FIFO_SIZE, SC_BACKEND_PRIM_FIFO_SIZE, adev->gfx.config.sc_prim_fifo_size_backend);
	tmp = REG_SET_FIELD(tmp, PA_SC_FIFO_SIZE, SC_HIZ_TILE_FIFO_SIZE, adev->gfx.config.sc_hiz_tile_fifo_size);
	tmp = REG_SET_FIELD(tmp, PA_SC_FIFO_SIZE, SC_EARLYZ_TILE_FIFO_SIZE, adev->gfx.config.sc_earlyz_tile_fifo_size);

	WREG32(mmVGT_NUM_INSTANCES, 1);

	WREG32(mmCP_PERFMON_CNTL, 0);

	WREG32(mmSQ_CONFIG, 0);

	tmp = REG_SET_FIELD(0, PA_SC_FORCE_EOV_MAX_CNTS, FORCE_EOV_MAX_CLK_CNT, 4095);
	tmp = REG_SET_FIELD(tmp, PA_SC_FORCE_EOV_MAX_CNTS, FORCE_EOV_MAX_REZ_CNT, 255);
	WREG32(mmPA_SC_FORCE_EOV_MAX_CNTS, tmp);

	tmp = REG_SET_FIELD(0, VGT_CACHE_INVALIDATION, CACHE_INVALIDATION, VGT_CACHE_INVALIDATION__VC_AND_TC);
	tmp = REG_SET_FIELD(tmp, VGT_CACHE_INVALIDATION, AUTO_INVLD_EN, VGT_CACHE_INVALIDATION__ES_AND_GS_AUTO);
	WREG32(mmVGT_CACHE_INVALIDATION, tmp);

	WREG32(mmVGT_GS_VERTEX_REUSE, 16);
	WREG32(mmPA_SC_LINE_STIPPLE_STATE, 0);

	WREG32(mmCB_PERFCOUNTER0_SELECT0, 0);
	WREG32(mmCB_PERFCOUNTER0_SELECT1, 0);
	WREG32(mmCB_PERFCOUNTER1_SELECT0, 0);
	WREG32(mmCB_PERFCOUNTER1_SELECT1, 0);
	WREG32(mmCB_PERFCOUNTER2_SELECT0, 0);
	WREG32(mmCB_PERFCOUNTER2_SELECT1, 0);
	WREG32(mmCB_PERFCOUNTER3_SELECT0, 0);
	WREG32(mmCB_PERFCOUNTER3_SELECT1, 0);

	tmp = RREG32(mmHDP_MISC_CNTL);
	tmp |= HDP_MISC_CNTL__HDP_FLUSH_INVALIDATE_CACHE_MASK;
	WREG32(mmHDP_MISC_CNTL, tmp);

	hdp_host_path_cntl = RREG32(mmHDP_HOST_PATH_CNTL);
	WREG32(mmHDP_HOST_PATH_CNTL, hdp_host_path_cntl);

	tmp = REG_SET_FIELD(0, PA_CL_ENHANCE, CLIP_VTX_REORDER_ENA, 1);
	tmp = REG_SET_FIELD(0, PA_CL_ENHANCE, NUM_CLIP_SEQ, 3);
	WREG32(mmPA_CL_ENHANCE, tmp);

	udelay(50);
}

/*
 * GPU scratch registers helpers function.
 */
/**
 * gfx_v6_0_scratch_init - setup driver info for CP scratch regs
 *
 * @adev: amdgpu_device pointer
 *
 * Set up the number and offset of the CP scratch registers.
 * NOTE: use of CP scratch registers is a legacy inferface and
 * is not used by default on newer asics (r6xx+).  On newer asics,
 * memory buffers are used for fences rather than scratch regs.
 */
static void gfx_v6_0_scratch_init(struct amdgpu_device *adev)
{
	int i;

	adev->gfx.scratch.num_reg = 7;
	adev->gfx.scratch.reg_base = mmSCRATCH_REG0;
	for (i = 0; i < adev->gfx.scratch.num_reg; i++) {
		adev->gfx.scratch.free[i] = true;
		adev->gfx.scratch.reg[i] = adev->gfx.scratch.reg_base + i;
	}
}

/**
 * gfx_v6_0_ring_test_ring - basic gfx ring test
 *
 * @adev: amdgpu_device pointer
 * @ring: amdgpu_ring structure holding ring information
 *
 * Allocate a scratch register and write to it using the gfx ring (SI).
 * Provides a basic gfx ring test to verify that the ring is working.
 * Used by gfx_v6_0_cp_gfx_resume();
 * Returns 0 on success, error on failure.
 */
static int gfx_v6_0_ring_test_ring(struct amdgpu_ring *ring)
{
	struct amdgpu_device *adev = ring->adev;
	uint32_t scratch;
	uint32_t tmp = 0;
	unsigned i;
	int r;

	/* DEBUG */ printk(KERN_ALERT "CP_INT_CNTL_RING0 0x%08x\n", RREG32(mmCP_INT_CNTL_RING0));
	r = amdgpu_gfx_scratch_get(adev, &scratch);
	if (r) {
		DRM_ERROR("amdgpu: cp failed to get scratch reg (%d).\n", r);
		return r;
	}
	WREG32(scratch, 0xCAFEDEAD);
	r = amdgpu_ring_lock(ring, 3);
	if (r) {
		DRM_ERROR("amdgpu: cp failed to lock ring %d (%d).\n", ring->idx, r);
		amdgpu_gfx_scratch_free(adev, scratch);
		return r;
	}
	amdgpu_ring_write(ring, PACKET3(PACKET3_SET_CONFIG_REG, 1));
	amdgpu_ring_write(ring, (scratch - PACKET3_SET_CONFIG_REG_START));
	amdgpu_ring_write(ring, 0xDEADBEEF);

	amdgpu_ring_unlock_commit(ring);

	for (i = 0; i < adev->usec_timeout; i++) {
		tmp = RREG32(scratch);
		if (tmp == 0xDEADBEEF)
			break;
		DRM_UDELAY(1);
	}
	if (i < adev->usec_timeout) {
		DRM_INFO("ring test on %d succeeded in %d usecs\n", ring->idx, i);
	} else {
		DRM_ERROR("amdgpu: ring %d test failed (scratch(0x%04X)=0x%08X)\n",
			  ring->idx, scratch, tmp);
		r = -EINVAL;
	}
	amdgpu_gfx_scratch_free(adev, scratch);
	return r;
}

/**
 * gfx_v6_0_ring_emit_fence_gfx - emit a fence on the gfx ring
 *
 * @adev: amdgpu_device pointer
 * @fence: amdgpu fence object
 *
 * Emits a fence sequnce number on the gfx ring and flushes
 * GPU caches.
 */
static void gfx_v6_0_ring_emit_fence_gfx(struct amdgpu_ring *ring, u64 addr,
					 u64 seq, unsigned flags)
{
	bool write64bit = flags & AMDGPU_FENCE_FLAG_64BIT;
	bool int_sel = flags & AMDGPU_FENCE_FLAG_INT;

	/* flush read cache over gart */
	amdgpu_ring_write(ring, PACKET3(PACKET3_SET_CONFIG_REG, 1));
	amdgpu_ring_write(ring, (mmCP_COHER_CNTL2 - PACKET3_SET_CONFIG_REG_START));
	amdgpu_ring_write(ring, 0);
	amdgpu_ring_write(ring, PACKET3(PACKET3_SURFACE_SYNC, 3));
	amdgpu_ring_write(ring, PACKET3_TCL1_ACTION_ENA |
			  PACKET3_TC_ACTION_ENA |
			  PACKET3_SH_KCACHE_ACTION_ENA |
			  PACKET3_SH_ICACHE_ACTION_ENA);
	amdgpu_ring_write(ring, 0xFFFFFFFF);
	amdgpu_ring_write(ring, 0);
	amdgpu_ring_write(ring, 10);

	/* EVENT_WRITE_EOP - flush caches, send int */
	amdgpu_ring_write(ring, PACKET3(PACKET3_EVENT_WRITE_EOP, 4));
	amdgpu_ring_write(ring, EVENT_TYPE(VGT_EVENT_INITIATOR__CACHE_FLUSH_AND_INV_TS_EVENT_MASK) | EVENT_INDEX(5));
	amdgpu_ring_write(ring, lower_32_bits(addr));
	amdgpu_ring_write(ring, (upper_32_bits(addr) & 0xff) | DATA_SEL(write64bit ? 2 : 1) | INT_SEL(int_sel ? 2 : 0));
	amdgpu_ring_write(ring, lower_32_bits(seq));
	amdgpu_ring_write(ring, upper_32_bits(seq));
}

/**
 * gfx_v6_0_ring_emit_fence_compute - emit a fence on the compute ring
 *
 * @adev: amdgpu_device pointer
 * @fence: amdgpu fence object
 *
 * Emits a fence sequnce number on the compute ring and flushes
 * GPU caches.
 */
static void gfx_v6_0_ring_emit_fence_compute(struct amdgpu_ring *ring,
					     u64 addr, u64 seq,
					     unsigned flags)
{
	bool write64bit = flags & AMDGPU_FENCE_FLAG_64BIT;
	bool int_sel = flags & AMDGPU_FENCE_FLAG_INT;

	/* EVENT_WRITE_EOP - flush caches, send int */
	amdgpu_ring_write(ring, PACKET3(PACKET3_EVENT_WRITE_EOP, 4));
	amdgpu_ring_write(ring, EVENT_TYPE(VGT_EVENT_INITIATOR__CACHE_FLUSH_AND_INV_TS_EVENT_MASK) | EVENT_INDEX(5));
	amdgpu_ring_write(ring, lower_32_bits(addr));
	amdgpu_ring_write(ring, (upper_32_bits(addr) & 0xff) | DATA_SEL(write64bit ? 2 : 1) | INT_SEL(int_sel ? 2 : 0));
	amdgpu_ring_write(ring, lower_32_bits(seq));
	amdgpu_ring_write(ring, upper_32_bits(seq));
}

/**
 * gfx_v6_0_ring_emit_semaphore - emit a semaphore on the CP ring
 *
 * @ring: amdgpu ring buffer object
 * @semaphore: amdgpu semaphore object
 * @emit_wait: Is this a sempahore wait?
 *
 * Emits a semaphore signal/wait packet to the CP ring and prevents the PFP
 * from running ahead of semaphore waits.
 */
static bool gfx_v6_0_ring_emit_semaphore(struct amdgpu_ring *ring,
					 struct amdgpu_semaphore *semaphore,
					 bool emit_wait)
{
	uint64_t addr = semaphore->gpu_addr;
	unsigned sel = emit_wait ? PACKET3_SEM_SEL_WAIT : PACKET3_SEM_SEL_SIGNAL;

	amdgpu_ring_write(ring, PACKET3(PACKET3_MEM_SEMAPHORE, 1));
	amdgpu_ring_write(ring, addr & 0xffffffff);
	amdgpu_ring_write(ring, (upper_32_bits(addr) & 0xff) | sel);

	if (emit_wait && (ring->type == AMDGPU_RING_TYPE_GFX)) {
		/* Prevent the PFP from running ahead of the semaphore wait */
		amdgpu_ring_write(ring, PACKET3(PACKET3_PFP_SYNC_ME, 0));
		amdgpu_ring_write(ring, 0x0);
	}

	return true;
}

/*
 * IB stuff
 */
/**
 * gfx_v6_0_ring_emit_ib - emit an IB (Indirect Buffer) on the ring
 *
 * @ring: amdgpu_ring structure holding ring information
 * @ib: amdgpu indirect buffer object
 *
 * Emits an DE (drawing engine) or CE (constant engine) IB
 * on the gfx ring.  IBs are usually generated by userspace
 * acceleration drivers and submitted to the kernel for
 * sheduling on the ring.  This function schedules the IB
 * on the gfx ring for execution by the GPU.
 */
static void gfx_v6_0_ring_emit_ib_gfx(struct amdgpu_ring *ring,
				  struct amdgpu_ib *ib)
{
	bool need_ctx_switch = ring->current_ctx != ib->ctx;
	u32 header, control = 0;
	unsigned vm_id = ib->vm ? (ib->vm->ids[ring->idx].id) : 0;
	u32 next_rptr = ring->wptr + 5;

	/* DEBUG */printk(KERN_ALERT "ib_gfx 0x%016llX %d ib->length_dw %d %d\n", ib->gpu_addr, vm_id, ib->length_dw, (int)(ib->flags & AMDGPU_IB_FLAG_CE));

	/* drop the CE preamble IB for the same context */
	if ((ib->flags & AMDGPU_IB_FLAG_PREAMBLE) && !need_ctx_switch)
		return;

	if (need_ctx_switch)
		next_rptr += 2;

	next_rptr += 4;
	if ((ib->flags & AMDGPU_IB_FLAG_CE) == 0)
		next_rptr += 8;

	amdgpu_ring_write(ring, PACKET3(PACKET3_WRITE_DATA, 3));
	amdgpu_ring_write(ring, WRITE_DATA_DST_SEL(5) | WR_CONFIRM);
	amdgpu_ring_write(ring, ring->next_rptr_gpu_addr & 0xfffffffc);
	amdgpu_ring_write(ring, upper_32_bits(ring->next_rptr_gpu_addr) & 0xffffffff);
	amdgpu_ring_write(ring, next_rptr);

	/* insert SWITCH_BUFFER packet before first IB in the ring frame */
	if (need_ctx_switch) {
		amdgpu_ring_write(ring, PACKET3(PACKET3_SWITCH_BUFFER, 0));
		amdgpu_ring_write(ring, 0);
	}

	if (ib->flags & AMDGPU_IB_FLAG_CE)
		header = PACKET3(PACKET3_INDIRECT_BUFFER_CONST, 2);
	else
		header = PACKET3(PACKET3_INDIRECT_BUFFER, 2);

	control |= ib->length_dw |
		(ib->vm ? (ib->vm->ids[ring->idx].id << 24) : 0);

	amdgpu_ring_write(ring, header);
	amdgpu_ring_write(ring,
#ifdef __BIG_ENDIAN
			  (2 << 0) |
#endif
			  (ib->gpu_addr & 0xFFFFFFFC));
	amdgpu_ring_write(ring, upper_32_bits(ib->gpu_addr) & 0xFFFF);
	amdgpu_ring_write(ring, control);

	if ((ib->flags & AMDGPU_IB_FLAG_CE) == 8) {
		/* flush read cache over gart for this vmid */
		amdgpu_ring_write(ring, PACKET3(PACKET3_SET_CONFIG_REG, 1));
		amdgpu_ring_write(ring, (mmCP_COHER_CNTL2 - PACKET3_SET_CONFIG_REG_START));
		amdgpu_ring_write(ring, vm_id);
		amdgpu_ring_write(ring, PACKET3(PACKET3_SURFACE_SYNC, 3));
		amdgpu_ring_write(ring, PACKET3_TCL1_ACTION_ENA |
				  PACKET3_TC_ACTION_ENA |
				  PACKET3_SH_KCACHE_ACTION_ENA |
				  PACKET3_SH_ICACHE_ACTION_ENA);
		amdgpu_ring_write(ring, 0xFFFFFFFF);
		amdgpu_ring_write(ring, 0);
		amdgpu_ring_write(ring, 10); /* poll interval */
	}
}

static void gfx_v6_0_ring_emit_ib_compute(struct amdgpu_ring *ring,
				  struct amdgpu_ib *ib)
{
	u32 header, control = 0;
	u32 next_rptr = ring->wptr + 5;

	next_rptr += 4;
	amdgpu_ring_write(ring, PACKET3(PACKET3_WRITE_DATA, 3));
	amdgpu_ring_write(ring, WRITE_DATA_DST_SEL(5) | WR_CONFIRM);
	amdgpu_ring_write(ring, ring->next_rptr_gpu_addr & 0xfffffffc);
	amdgpu_ring_write(ring, upper_32_bits(ring->next_rptr_gpu_addr) & 0xffffffff);
	amdgpu_ring_write(ring, next_rptr);

	header = PACKET3(PACKET3_INDIRECT_BUFFER, 2);

	control |= ib->length_dw |
			   (ib->vm ? (ib->vm->ids[ring->idx].id << 24) : 0);

	amdgpu_ring_write(ring, header);
	amdgpu_ring_write(ring,
#ifdef __BIG_ENDIAN
					  (2 << 0) |
#endif
					  (ib->gpu_addr & 0xFFFFFFFC));
	amdgpu_ring_write(ring, upper_32_bits(ib->gpu_addr) & 0xFFFF);
	amdgpu_ring_write(ring, control);
}

/**
 * gfx_v6_0_ring_test_ib - basic ring IB test
 *
 * @ring: amdgpu_ring structure holding ring information
 *
 * Allocate an IB and execute it on the gfx ring (SI).
 * Provides a basic gfx ring test to verify that IBs are working.
 * Returns 0 on success, error on failure.
 */
static int gfx_v6_0_ring_test_ib(struct amdgpu_ring *ring)
{
	struct amdgpu_device *adev = ring->adev;
	struct amdgpu_ib ib;
	struct fence *f = NULL;
	uint32_t scratch;
	uint32_t tmp = 0;
	unsigned i;
	int r;

	r = amdgpu_gfx_scratch_get(adev, &scratch);
	if (r) {
		DRM_ERROR("amdgpu: failed to get scratch reg (%d).\n", r);
		return r;
	}
	WREG32(scratch, 0xCAFEDEAD);
	memset(&ib, 0, sizeof(ib));
	r = amdgpu_ib_get(ring, NULL, 256, &ib);
	if (r) {
		DRM_ERROR("amdgpu: failed to get ib (%d).\n", r);
		goto err1;
	}
	ib.ptr[0] = PACKET3(PACKET3_SET_CONFIG_REG, 1);
	ib.ptr[1] = ((scratch - PACKET3_SET_CONFIG_REG_START));
	ib.ptr[2] = 0xDEADBEEF;
	ib.length_dw = 3;

	r = amdgpu_sched_ib_submit_kernel_helper(adev, ring, &ib, 1, NULL,
						 AMDGPU_FENCE_OWNER_UNDEFINED,
						 &f);
	if (r)
		goto err2;

	r = fence_wait(f, false);
	if (r) {
		DRM_ERROR("amdgpu: fence wait failed (%d).\n", r);
		goto err2;
	}
	for (i = 0; i < adev->usec_timeout; i++) {
		tmp = RREG32(scratch);
		if (tmp == 0xDEADBEEF)
			break;
		DRM_UDELAY(1);
	}
	if (i < adev->usec_timeout) {
		DRM_INFO("ib test on ring %d succeeded in %u usecs\n",
			 ring->idx, i);
		goto err2;
	} else {
		DRM_ERROR("amdgpu: ib test failed (scratch(0x%04X)=0x%08X)\n",
			  scratch, tmp);
		r = -EINVAL;
	}

err2:
	fence_put(f);
	amdgpu_ib_free(adev, &ib);
err1:
	amdgpu_gfx_scratch_free(adev, scratch);
	return r;
}

/**
 * gfx_v6_0_cp_gfx_enable - enable/disable the gfx CP MEs
 *
 * @adev: amdgpu_device pointer
 * @enable: enable or disable the MEs
 *
 * Halts or unhalts the gfx MEs.
 */
static void gfx_v6_0_cp_gfx_enable(struct amdgpu_device *adev, bool enable)
{
	int i;

	if (enable) {
		WREG32(mmCP_ME_CNTL, 0);
	} else {
		WREG32(mmCP_ME_CNTL, (CP_ME_CNTL__CP_ME_HALT_MASK | CP_ME_CNTL__CP_PFP_HALT_MASK | CP_ME_CNTL__CP_CE_HALT_MASK));
		WREG32(mmSCRATCH_UMSK, 0);
		for (i = 0; i < adev->gfx.num_gfx_rings; i++)
			adev->gfx.gfx_ring[i].ready = false;
		for (i = 0; i < adev->gfx.num_compute_rings; i++)
			adev->gfx.compute_ring[i].ready = false;
	}
	udelay(50);
}

/**
 * gfx_v6_0_cp_gfx_load_microcode - load the gfx CP ME ucode
 *
 * @adev: amdgpu_device pointer
 *
 * Loads the gfx PFP, ME, and CE ucode.
 * Returns 0 for success, -EINVAL if the ucode is not available.
 */
static int gfx_v6_0_cp_gfx_load_microcode(struct amdgpu_device *adev)
{
	const struct gfx_firmware_header_v1_0 *pfp_hdr;
	const struct gfx_firmware_header_v1_0 *ce_hdr;
	const struct gfx_firmware_header_v1_0 *me_hdr;
	const __le32 *fw_data;
	unsigned i, fw_size;

	if (!adev->gfx.me_fw || !adev->gfx.pfp_fw || !adev->gfx.ce_fw)
		return -EINVAL;

	pfp_hdr = (const struct gfx_firmware_header_v1_0 *)adev->gfx.pfp_fw->data;
	ce_hdr = (const struct gfx_firmware_header_v1_0 *)adev->gfx.ce_fw->data;
	me_hdr = (const struct gfx_firmware_header_v1_0 *)adev->gfx.me_fw->data;

	amdgpu_ucode_print_gfx_hdr(&pfp_hdr->header);
	amdgpu_ucode_print_gfx_hdr(&ce_hdr->header);
	amdgpu_ucode_print_gfx_hdr(&me_hdr->header);
	adev->gfx.pfp_fw_version = le32_to_cpu(pfp_hdr->header.ucode_version);
	adev->gfx.ce_fw_version = le32_to_cpu(ce_hdr->header.ucode_version);
	adev->gfx.me_fw_version = le32_to_cpu(me_hdr->header.ucode_version);
	adev->gfx.me_feature_version = le32_to_cpu(me_hdr->ucode_feature_version);
	adev->gfx.ce_feature_version = le32_to_cpu(ce_hdr->ucode_feature_version);
	adev->gfx.pfp_feature_version = le32_to_cpu(pfp_hdr->ucode_feature_version);

	gfx_v6_0_cp_gfx_enable(adev, false);

	/* PFP */
	fw_data = (const __le32 *)
		(adev->gfx.pfp_fw->data +
		 le32_to_cpu(pfp_hdr->header.ucode_array_offset_bytes));
	fw_size = le32_to_cpu(pfp_hdr->header.ucode_size_bytes) / 4;
	WREG32(mmCP_PFP_UCODE_ADDR, 0);
	for (i = 0; i < fw_size; i++)
		WREG32(mmCP_PFP_UCODE_DATA, le32_to_cpup(fw_data++));

	/* CE */
	fw_data = (const __le32 *)
		(adev->gfx.ce_fw->data +
		 le32_to_cpu(ce_hdr->header.ucode_array_offset_bytes));
	fw_size = le32_to_cpu(ce_hdr->header.ucode_size_bytes) / 4;
	WREG32(mmCP_CE_UCODE_ADDR, 0);
	for (i = 0; i < fw_size; i++)
		WREG32(mmCP_CE_UCODE_DATA, le32_to_cpup(fw_data++));

	/* ME */
	fw_data = (const __le32 *)
		(adev->gfx.me_fw->data +
		 le32_to_cpu(me_hdr->header.ucode_array_offset_bytes));
	fw_size = le32_to_cpu(me_hdr->header.ucode_size_bytes) / 4;
	WREG32(mmCP_ME_RAM_WADDR, 0);
	for (i = 0; i < fw_size; i++)
		WREG32(mmCP_ME_RAM_DATA, le32_to_cpup(fw_data++));

	WREG32(mmCP_PFP_UCODE_ADDR, 0);
	WREG32(mmCP_CE_UCODE_ADDR, 0);
	WREG32(mmCP_ME_RAM_WADDR, 0);
	WREG32(mmCP_ME_RAM_RADDR, 0);

	return 0;
}

/**
 * gfx_v6_0_cp_gfx_start - start the gfx ring
 *
 * @adev: amdgpu_device pointer
 *
 * Enables the ring and loads the clear state context and other
 * packets required to init the ring.
 * Returns 0 for success, error for failure.
 */
static int gfx_v6_0_cp_gfx_start(struct amdgpu_device *adev)
{
	struct amdgpu_ring *ring = &adev->gfx.gfx_ring[0];
	int r, i;

	gfx_v6_0_cp_gfx_enable(adev, true);

	r = amdgpu_ring_lock(ring, 7 + 4);
	if (r) {
		DRM_ERROR("amdgpu: cp failed to lock ring (%d).\n", r);
		return r;
	}

	/* init the CP */
	amdgpu_ring_write(ring, PACKET3(PACKET3_ME_INITIALIZE, 5));
	amdgpu_ring_write(ring, 0x1);
	amdgpu_ring_write(ring, 0x0);
	amdgpu_ring_write(ring, adev->gfx.config.max_hw_contexts - 1);
	amdgpu_ring_write(ring, PACKET3_ME_INITIALIZE_DEVICE_ID(1));
	amdgpu_ring_write(ring, 0);
	amdgpu_ring_write(ring, 0);

	/* init the CE partitions. */
	amdgpu_ring_write(ring, PACKET3(PACKET3_SET_BASE, 2));
	amdgpu_ring_write(ring, PACKET3_BASE_INDEX(CE_PARTITION_BASE));
	amdgpu_ring_write(ring, 0xc000);
	amdgpu_ring_write(ring, 0xe000);
	amdgpu_ring_unlock_commit(ring);

	r = amdgpu_ring_lock(ring, si_default_size + 10);
	if (r) {
		DRM_ERROR("amdgpu: cp failed to lock ring (%d).\n", r);
		return r;
	}

	/* clear state buffer */
	amdgpu_ring_write(ring, PACKET3(PACKET3_PREAMBLE_CNTL, 0));
	amdgpu_ring_write(ring, PACKET3_PREAMBLE_BEGIN_CLEAR_STATE);

	for (i = 0; i < si_default_size; i++)
		amdgpu_ring_write(ring, si_default_state[i]);

	amdgpu_ring_write(ring, PACKET3(PACKET3_PREAMBLE_CNTL, 0));
	amdgpu_ring_write(ring, PACKET3_PREAMBLE_END_CLEAR_STATE);

	amdgpu_ring_write(ring, PACKET3(PACKET3_CLEAR_STATE, 0));
	amdgpu_ring_write(ring, 0);

	amdgpu_ring_write(ring, PACKET3(PACKET3_SET_CONTEXT_REG, 2));
	amdgpu_ring_write(ring, 0x00000316);
	amdgpu_ring_write(ring, 0x0000000e); /* VGT_VERTEX_REUSE_BLOCK_CNTL */
	amdgpu_ring_write(ring, 0x00000010); /* VGT_OUT_DEALLOC_CNTL */

	amdgpu_ring_unlock_commit(ring);

	for(i = 0; i < adev->gfx.num_gfx_rings; ++i)
	{
		ring = &adev->gfx.gfx_ring[i];
		r = amdgpu_ring_lock(ring, 2);

		/* clear the compute context state */
		amdgpu_ring_write(ring, PACKET3_COMPUTE(PACKET3_CLEAR_STATE, 0));
		amdgpu_ring_write(ring, 0);

		amdgpu_ring_unlock_commit(ring);
	}

	for(i = 0; i < adev->gfx.num_compute_rings; ++i)
	{
		ring = &adev->gfx.compute_ring[i];
		r = amdgpu_ring_lock(ring, 2);

		/* clear the compute context state */
		amdgpu_ring_write(ring, PACKET3_COMPUTE(PACKET3_CLEAR_STATE, 0));
		amdgpu_ring_write(ring, 0);

		amdgpu_ring_unlock_commit(ring);
	}
	return 0;
}

/**
 * gfx_v6_0_cp_gfx_resume - setup the gfx ring buffer registers
 *
 * @adev: amdgpu_device pointer
 *
 * Program the location and size of the gfx ring buffer
 * and test it to make sure it's working.
 * Returns 0 for success, error for failure.
 */
static int gfx_v6_0_cp_gfx_resume(struct amdgpu_device *adev)
{
	struct amdgpu_ring *ring;
	u32 tmp;
	u32 rb_bufsz;
	u64 rb_addr, rptr_addr;
	int r;

	WREG32(mmCP_SEM_WAIT_TIMER, 0x0);
	WREG32(mmCP_SEM_INCOMPLETE_TIMER_CNTL, 0x0);

	/* Set the write pointer delay */
	WREG32(mmCP_RB_WPTR_DELAY, 0);

	/* set the RB to use vmid 0 */
	WREG32(mmCP_DEBUG, 0);
	WREG32(mmSCRATCH_ADDR, 0);

	/* ring 0 - compute and gfx */
	/* Set ring buffer size */
	ring = &adev->gfx.gfx_ring[0];
	rb_bufsz = order_base_2(ring->ring_size / 8);
	tmp = (order_base_2(AMDGPU_GPU_PAGE_SIZE/8) << 8) | rb_bufsz;
#ifdef __BIG_ENDIAN
	tmp |= 2 << CP_RB0_CNTL__BUF_SWAP__SHIFT;
#endif
	WREG32(mmCP_RB0_CNTL, tmp);

	/* Initialize the ring buffer's read and write pointers */
	WREG32(mmCP_RB0_CNTL, tmp | CP_RB0_CNTL__RB_RPTR_WR_ENA_MASK);
	ring->wptr = 0;
	WREG32(mmCP_RB0_WPTR, ring->wptr);

	/* set the wb address wether it's enabled or not */
	rptr_addr = adev->wb.gpu_addr + (ring->rptr_offs * 4);
	WREG32(mmCP_RB0_RPTR_ADDR, lower_32_bits(rptr_addr));
	WREG32(mmCP_RB0_RPTR_ADDR_HI, upper_32_bits(rptr_addr) & 0xFF);

	/* Scratch shadowing not supported */
	WREG32(mmSCRATCH_UMSK, 0);

	mdelay(1);
	WREG32(mmCP_RB0_CNTL, tmp);

	rb_addr = ring->gpu_addr >> 8;
	WREG32(mmCP_RB0_BASE, rb_addr);

	/* ring 1 - compute */
	/* Set ring buffer size */
	ring = &adev->gfx.compute_ring[0];
	rb_bufsz = order_base_2(ring->ring_size / 8);
	tmp = (order_base_2(AMDGPU_GPU_PAGE_SIZE/8) << 8) | rb_bufsz;
#ifdef __BIG_ENDIAN
	tmp |= 2 << CP_RB0_CNTL__BUF_SWAP__SHIFT;
#endif
	WREG32(mmCP_RB1_CNTL, tmp);

	/* Initialize the ring buffer's read and write pointers */
	WREG32(mmCP_RB1_CNTL, tmp | CP_RB0_CNTL__RB_RPTR_WR_ENA_MASK);
	ring->wptr = 0;
	WREG32(mmCP_RB1_WPTR, ring->wptr);

	/* set the wb address wether it's enabled or not */
	rptr_addr = adev->wb.gpu_addr + (ring->rptr_offs * 4);
	WREG32(mmCP_RB1_RPTR_ADDR, lower_32_bits(rptr_addr));
	WREG32(mmCP_RB1_RPTR_ADDR_HI, upper_32_bits(rptr_addr) & 0xFF);

	mdelay(1);
	WREG32(mmCP_RB1_CNTL, tmp);

	rb_addr = ring->gpu_addr >> 8;
	WREG32(mmCP_RB1_BASE, rb_addr);

	/* ring 2 - compute */
	/* Set ring buffer size */
	ring = &adev->gfx.compute_ring[1];
	rb_bufsz = order_base_2(ring->ring_size / 8);
	tmp = (order_base_2(AMDGPU_GPU_PAGE_SIZE/8) << 8) | rb_bufsz;
#ifdef __BIG_ENDIAN
	tmp |= 2 << CP_RB0_CNTL__BUF_SWAP__SHIFT;
#endif
	WREG32(mmCP_RB2_CNTL, tmp);

	/* Initialize the ring buffer's read and write pointers */
	WREG32(mmCP_RB2_CNTL, tmp | CP_RB0_CNTL__RB_RPTR_WR_ENA_MASK);
	ring->wptr = 0;
	WREG32(mmCP_RB2_WPTR, ring->wptr);

	/* set the wb address wether it's enabled or not */
	rptr_addr = adev->wb.gpu_addr + (ring->rptr_offs * 4);
	WREG32(mmCP_RB2_RPTR_ADDR, lower_32_bits(rptr_addr));
	WREG32(mmCP_RB2_RPTR_ADDR_HI, upper_32_bits(rptr_addr) & 0xFF);

	mdelay(1);
	WREG32(mmCP_RB2_CNTL, tmp);

	rb_addr = ring->gpu_addr >> 8;
	WREG32(mmCP_RB2_BASE, rb_addr);

	/* start the ring */
	gfx_v6_0_cp_gfx_start(adev);

	adev->gfx.gfx_ring[0].ready = true;
	adev->gfx.compute_ring[0].ready = true;
	adev->gfx.compute_ring[1].ready = true;
	r = amdgpu_ring_test_ring(&adev->gfx.gfx_ring[0]);
	if (r) {
		ring->ready = false;
		adev->gfx.compute_ring[0].ready = false;
		adev->gfx.compute_ring[1].ready = false;
		return r;
	}

	r = amdgpu_ring_test_ring(&adev->gfx.compute_ring[0]);
	if (r) {
		adev->gfx.compute_ring[0].ready = false;
		adev->gfx.compute_ring[1].ready = false;
		return r;
	}

	r = amdgpu_ring_test_ring(&adev->gfx.compute_ring[1]);
	if (r) {
		adev->gfx.compute_ring[1].ready = false;
		return r;
	}

	return 0;
}

static u32 gfx_v6_0_ring_get_rptr_gfx(struct amdgpu_ring *ring)
{
	u32 rptr;

	rptr = ring->adev->wb.wb[ring->rptr_offs];

	return rptr;
}

static u32 gfx_v6_0_ring_get_wptr_gfx(struct amdgpu_ring *ring)
{
	struct amdgpu_device *adev = ring->adev;
	u32 wptr;

	wptr = RREG32(mmCP_RB0_WPTR);

	return wptr;
}

static void gfx_v6_0_ring_set_wptr_gfx(struct amdgpu_ring *ring)
{
	struct amdgpu_device *adev = ring->adev;

	WREG32(mmCP_RB0_WPTR, ring->wptr);
	(void)RREG32(mmCP_RB0_WPTR);
}

static u32 gfx_v6_0_ring_get_rptr_compute(struct amdgpu_ring *ring)
{
	u32 rptr;

	rptr = ring->adev->wb.wb[ring->rptr_offs];

	return rptr;
}

static u32 gfx_v6_0_ring_get_wptr_compute(struct amdgpu_ring *ring)
{
	u32 wptr;
	struct amdgpu_device *adev = ring->adev;

	u32 reg = (ring == &adev->gfx.compute_ring[0]) ? mmCP_RB1_WPTR : mmCP_RB2_WPTR;
	wptr = RREG32(reg);

	return wptr;
}

static void gfx_v6_0_ring_set_wptr_compute(struct amdgpu_ring *ring)
{
	struct amdgpu_device *adev = ring->adev;

	u32 reg = (ring == &adev->gfx.compute_ring[0]) ? mmCP_RB1_WPTR : mmCP_RB2_WPTR;
	WREG32(reg, ring->wptr);
	(void)RREG32(reg);
}

static void gfx_v6_0_cp_enable(struct amdgpu_device *adev, bool enable)
{
	gfx_v6_0_cp_gfx_enable(adev, enable);
}

static int gfx_v6_0_cp_load_microcode(struct amdgpu_device *adev)
{
	int r;

	r = gfx_v6_0_cp_gfx_load_microcode(adev);
	if (r)
		return r;

	return 0;
}

static void gfx_v6_0_enable_gui_idle_interrupt(struct amdgpu_device *adev,
					       bool enable)
{
	u32 tmp = RREG32(mmCP_INT_CNTL_RING0);

	if (enable)
		tmp |= (CP_INT_CNTL_RING2__CNTX_BUSY_INT_ENABLE_MASK |
				CP_INT_CNTL_RING2__CNTX_EMPTY_INT_ENABLE_MASK);
	else
		tmp &= ~(CP_INT_CNTL_RING2__CNTX_BUSY_INT_ENABLE_MASK |
				CP_INT_CNTL_RING2__CNTX_EMPTY_INT_ENABLE_MASK);
	WREG32(mmCP_INT_CNTL_RING0, tmp);
}

static int gfx_v6_0_cp_resume(struct amdgpu_device *adev)
{
	int r;

	gfx_v6_0_enable_gui_idle_interrupt(adev, false);

	r = gfx_v6_0_cp_load_microcode(adev);
	if (r)
		return r;

	r = gfx_v6_0_cp_gfx_resume(adev);
	if (r)
		return r;

	gfx_v6_0_enable_gui_idle_interrupt(adev, true);

	return 0;
}

/*
 * vm
 * VMID 0 is the physical GPU addresses as used by the kernel.
 * VMIDs 1-15 are used for userspace clients and are handled
 * by the amdgpu vm/hsa code.
 */
/**
 * gfx_v6_0_ring_emit_vm_flush - cik vm flush using the CP
 *
 * @adev: amdgpu_device pointer
 *
 * Update the page table base and flush the VM TLB
 * using the CP (SI).
 */
static void gfx_v6_0_ring_emit_vm_flush(struct amdgpu_ring *ring,
					unsigned vm_id, uint64_t pd_addr)
{
	printk(KERN_ALERT "emit gfx vm flush %d 0x%016llX\n", vm_id, pd_addr);

	amdgpu_ring_write(ring, PACKET3(PACKET3_WRITE_DATA, 3));
	amdgpu_ring_write(ring, (WRITE_DATA_ENGINE_SEL(1) |
				 WRITE_DATA_DST_SEL(0)));
	if (vm_id < 8) {
		amdgpu_ring_write(ring,
				  (mmVM_CONTEXT0_PAGE_TABLE_BASE_ADDR + vm_id));
	} else {
		amdgpu_ring_write(ring,
				  (mmVM_CONTEXT8_PAGE_TABLE_BASE_ADDR + vm_id - 8));
	}
	amdgpu_ring_write(ring, 0);
	amdgpu_ring_write(ring, pd_addr >> 12);

	/* flush hdp cache */
	amdgpu_ring_write(ring, PACKET3(PACKET3_WRITE_DATA, 3));
	amdgpu_ring_write(ring, (WRITE_DATA_ENGINE_SEL(1) |
				 WRITE_DATA_DST_SEL(0)));
	amdgpu_ring_write(ring, mmHDP_MEM_COHERENCY_FLUSH_CNTL);
	amdgpu_ring_write(ring, 0);
	amdgpu_ring_write(ring, 0x1);

	/* bits 0-15 are the VM contexts0-15 */
	amdgpu_ring_write(ring, PACKET3(PACKET3_WRITE_DATA, 3));
	amdgpu_ring_write(ring, (WRITE_DATA_ENGINE_SEL(1) |
				 WRITE_DATA_DST_SEL(0)));
	amdgpu_ring_write(ring, mmVM_INVALIDATE_REQUEST);
	amdgpu_ring_write(ring, 0);
	amdgpu_ring_write(ring, 1 << vm_id);

	/* wait for the invalidate to complete */
	amdgpu_ring_write(ring, PACKET3(PACKET3_WAIT_REG_MEM, 5));
	amdgpu_ring_write(ring, (WAIT_REG_MEM_FUNCTION(0) |  /* always */
				 WAIT_REG_MEM_ENGINE(0))); /* me */
	amdgpu_ring_write(ring, mmVM_INVALIDATE_REQUEST);
	amdgpu_ring_write(ring, 0);
	amdgpu_ring_write(ring, 0); /* ref */
	amdgpu_ring_write(ring, 0); /* mask */
	amdgpu_ring_write(ring, 0x20); /* poll interval */

	/* sync PFP to ME, otherwise we might get invalid PFP reads */
	amdgpu_ring_write(ring, PACKET3(PACKET3_PFP_SYNC_ME, 0));
	amdgpu_ring_write(ring, 0x0);
}

/*
 * RLC
 * The RLC is a multi-purpose microengine that handles a
 * variety of functions.
 */
static void gfx_v6_0_rlc_fini(struct amdgpu_device *adev)
{
	int r;

	/* save restore block */
	if (adev->gfx.rlc.save_restore_obj) {
		r = amdgpu_bo_reserve(adev->gfx.rlc.save_restore_obj, false);
		if (unlikely(r != 0))
			dev_warn(adev->dev, "(%d) reserve RLC sr bo failed\n", r);
		amdgpu_bo_unpin(adev->gfx.rlc.save_restore_obj);
		amdgpu_bo_unreserve(adev->gfx.rlc.save_restore_obj);

		amdgpu_bo_unref(&adev->gfx.rlc.save_restore_obj);
		adev->gfx.rlc.save_restore_obj = NULL;
	}

	/* clear state block */
	if (adev->gfx.rlc.clear_state_obj) {
		r = amdgpu_bo_reserve(adev->gfx.rlc.clear_state_obj, false);
		if (unlikely(r != 0))
			dev_warn(adev->dev, "(%d) reserve RLC c bo failed\n", r);
		amdgpu_bo_unpin(adev->gfx.rlc.clear_state_obj);
		amdgpu_bo_unreserve(adev->gfx.rlc.clear_state_obj);

		amdgpu_bo_unref(&adev->gfx.rlc.clear_state_obj);
		adev->gfx.rlc.clear_state_obj = NULL;
	}

	/* clear state block */
	if (adev->gfx.rlc.cp_table_obj) {
		r = amdgpu_bo_reserve(adev->gfx.rlc.cp_table_obj, false);
		if (unlikely(r != 0))
			dev_warn(adev->dev, "(%d) reserve RLC cp table bo failed\n", r);
		amdgpu_bo_unpin(adev->gfx.rlc.cp_table_obj);
		amdgpu_bo_unreserve(adev->gfx.rlc.cp_table_obj);

		amdgpu_bo_unref(&adev->gfx.rlc.cp_table_obj);
		adev->gfx.rlc.cp_table_obj = NULL;
	}
}


static int gfx_v6_0_rlc_init(struct amdgpu_device *adev)
{
	const u32 *src_ptr;
	volatile u32 *dst_ptr;
	u32 dws, i;
	u64 reg_list_mc_addr;
	const struct cs_section_def *cs_data;
	int r;

	/* allocate rlc buffers */
	adev->gfx.rlc.cs_data = si_cs_data;

	src_ptr = adev->gfx.rlc.reg_list;
	dws = adev->gfx.rlc.reg_list_size;

	cs_data = adev->gfx.rlc.cs_data;

	if (src_ptr) {
		/* save restore block */
		if (adev->gfx.rlc.save_restore_obj == NULL) {
			r = amdgpu_bo_create(adev, dws * 4, PAGE_SIZE, true,
					     AMDGPU_GEM_DOMAIN_VRAM,
					     AMDGPU_GEM_CREATE_CPU_ACCESS_REQUIRED,
					     NULL, NULL,
					     &adev->gfx.rlc.save_restore_obj);
			if (r) {
				dev_warn(adev->dev, "(%d) create RLC sr bo failed\n", r);
				return r;
			}
		}

		r = amdgpu_bo_reserve(adev->gfx.rlc.save_restore_obj, false);
		if (unlikely(r != 0)) {
			gfx_v6_0_rlc_fini(adev);
			return r;
		}
		r = amdgpu_bo_pin(adev->gfx.rlc.save_restore_obj, AMDGPU_GEM_DOMAIN_VRAM,
				  &adev->gfx.rlc.save_restore_gpu_addr);
		if (r) {
			amdgpu_bo_unreserve(adev->gfx.rlc.save_restore_obj);
			dev_warn(adev->dev, "(%d) pin RLC sr bo failed\n", r);
			gfx_v6_0_rlc_fini(adev);
			return r;
		}

		r = amdgpu_bo_kmap(adev->gfx.rlc.save_restore_obj, (void **)&adev->gfx.rlc.sr_ptr);
		if (r) {
			dev_warn(adev->dev, "(%d) map RLC sr bo failed\n", r);
			gfx_v6_0_rlc_fini(adev);
			return r;
		}
		/* write the sr buffer */
		dst_ptr = adev->gfx.rlc.sr_ptr;
		for (i = 0; i < adev->gfx.rlc.reg_list_size; i++)
			dst_ptr[i] = cpu_to_le32(src_ptr[i]);
		amdgpu_bo_kunmap(adev->gfx.rlc.save_restore_obj);
		amdgpu_bo_unreserve(adev->gfx.rlc.save_restore_obj);
	}

	if (cs_data) {
		/* clear state block */
		adev->gfx.rlc.clear_state_size = gfx_v6_0_get_csb_size(adev);
		dws = adev->gfx.rlc.clear_state_size + (256 / 4);

		if (adev->gfx.rlc.clear_state_obj == NULL) {
			r = amdgpu_bo_create(adev, dws * 4, PAGE_SIZE, true,
					     AMDGPU_GEM_DOMAIN_VRAM,
					     AMDGPU_GEM_CREATE_CPU_ACCESS_REQUIRED,
					     NULL, NULL,
					     &adev->gfx.rlc.clear_state_obj);
			if (r) {
				dev_warn(adev->dev, "(%d) create RLC c bo failed\n", r);
				gfx_v6_0_rlc_fini(adev);
				return r;
			}
		}
		r = amdgpu_bo_reserve(adev->gfx.rlc.clear_state_obj, false);
		if (unlikely(r != 0)) {
			gfx_v6_0_rlc_fini(adev);
			return r;
		}
		r = amdgpu_bo_pin(adev->gfx.rlc.clear_state_obj, AMDGPU_GEM_DOMAIN_VRAM,
				  &adev->gfx.rlc.clear_state_gpu_addr);
		if (r) {
			amdgpu_bo_unreserve(adev->gfx.rlc.clear_state_obj);
			dev_warn(adev->dev, "(%d) pin RLC c bo failed\n", r);
			gfx_v6_0_rlc_fini(adev);
			return r;
		}

		r = amdgpu_bo_kmap(adev->gfx.rlc.clear_state_obj, (void **)&adev->gfx.rlc.cs_ptr);
		if (r) {
			dev_warn(adev->dev, "(%d) map RLC c bo failed\n", r);
			gfx_v6_0_rlc_fini(adev);
			return r;
		}
		/* set up the cs buffer */
		dst_ptr = adev->gfx.rlc.cs_ptr;
		reg_list_mc_addr = adev->gfx.rlc.clear_state_gpu_addr + 256;
		dst_ptr[0] = cpu_to_le32(upper_32_bits(reg_list_mc_addr));
		dst_ptr[1] = cpu_to_le32(lower_32_bits(reg_list_mc_addr));
		dst_ptr[2] = cpu_to_le32(adev->gfx.rlc.clear_state_size);
		gfx_v6_0_get_csb_buffer(adev, &dst_ptr[(256/4)]);

		amdgpu_bo_kunmap(adev->gfx.rlc.clear_state_obj);
		amdgpu_bo_unreserve(adev->gfx.rlc.clear_state_obj);
	}

	return 0;
}

static void gfx_v6_0_enable_lbpw(struct amdgpu_device *adev, bool enable)
{
	u32 tmp;

	tmp = RREG32(mmRLC_LB_CNTL);
	if (enable)
		tmp |= RLC_LB_CNTL__LOAD_BALANCE_ENABLE_MASK;
	else
		tmp &= ~RLC_LB_CNTL__LOAD_BALANCE_ENABLE_MASK;
	WREG32(mmRLC_LB_CNTL, tmp);

	if (!enable) {
		mutex_lock(&adev->grbm_idx_mutex);
		gfx_v6_0_select_se_sh(adev, 0xffffffff, 0xffffffff);
		WREG32(mmSPI_LB_CU_MASK, 0x00ff);
		mutex_unlock(&adev->grbm_idx_mutex);
	}
}

static void gfx_v6_0_wait_for_rlc_serdes(struct amdgpu_device *adev)
{
	int i;

	for (i = 0; i < adev->usec_timeout; i++) {
		if (RREG32(mmRLC_SERDES_MASTER_BUSY_0) == 0)
			break;
		udelay(1);
	}

	for (i = 0; i < adev->usec_timeout; i++) {
		if (RREG32(mmRLC_SERDES_MASTER_BUSY_1) == 0)
			break;
		udelay(1);
	}
}

static void gfx_v6_0_update_rlc(struct amdgpu_device *adev, u32 rlc)
{
	u32 tmp;

	tmp = RREG32(mmRLC_CNTL);
	if (tmp != rlc)
		WREG32(mmRLC_CNTL, rlc);
}

static u32 gfx_v6_0_halt_rlc(struct amdgpu_device *adev)
{
	u32 data, orig;

	orig = data = RREG32(mmRLC_CNTL);

	if (data & RLC_CNTL__RLC_ENABLE_MASK) {

		data &= ~RLC_CNTL__RLC_ENABLE_MASK;
		WREG32(mmRLC_CNTL, data);

		gfx_v6_0_wait_for_rlc_serdes(adev);
	}

	return orig;
}

void gfx_v6_0_enter_rlc_safe_mode(struct amdgpu_device *adev)
{
	/* TODO: Should I do something ?*/
}

void gfx_v6_0_exit_rlc_safe_mode(struct amdgpu_device *adev)
{
	/* TODO: Should I do something ?*/
}

/**
 * gfx_v6_0_rlc_stop - stop the RLC ME
 *
 * @adev: amdgpu_device pointer
 *
 * Halt the RLC ME (MicroEngine) (SI).
 */
void gfx_v6_0_rlc_stop(struct amdgpu_device *adev)
{
	WREG32(mmRLC_CNTL, 0);

	gfx_v6_0_enable_gui_idle_interrupt(adev, false);

	gfx_v6_0_wait_for_rlc_serdes(adev);
}

/**
 * gfx_v6_0_rlc_start - start the RLC ME
 *
 * @adev: amdgpu_device pointer
 *
 * Unhalt the RLC ME (MicroEngine) (SI).
 */
static void gfx_v6_0_rlc_start(struct amdgpu_device *adev)
{
	WREG32(mmRLC_CNTL, RLC_CNTL__RLC_ENABLE_MASK);

	gfx_v6_0_enable_gui_idle_interrupt(adev, true);

	udelay(50);
}

static void gfx_v6_0_rlc_reset(struct amdgpu_device *adev)
{
	u32 tmp = RREG32(mmGRBM_SOFT_RESET);

	tmp |= GRBM_SOFT_RESET__SOFT_RESET_RLC_MASK;
	WREG32(mmGRBM_SOFT_RESET, tmp);
	udelay(50);
	tmp &= ~GRBM_SOFT_RESET__SOFT_RESET_RLC_MASK;
	WREG32(mmGRBM_SOFT_RESET, tmp);
	udelay(50);
}

/**
 * gfx_v6_0_rlc_resume - setup the RLC hw
 *
 * @adev: amdgpu_device pointer
 *
 * Initialize the RLC registers, load the ucode,
 * and start the RLC (SI).
 * Returns 0 for success, -EINVAL if the ucode is not available.
 */
static int gfx_v6_0_rlc_resume(struct amdgpu_device *adev)
{
	const struct rlc_firmware_header_v1_0 *hdr;
	const __le32 *fw_data;
	unsigned i, fw_size;
	u32 tmp;

	if (!adev->gfx.rlc_fw)
		return -EINVAL;

	hdr = (const struct rlc_firmware_header_v1_0 *)adev->gfx.rlc_fw->data;
	amdgpu_ucode_print_rlc_hdr(&hdr->header);
	adev->gfx.rlc_fw_version = le32_to_cpu(hdr->header.ucode_version);
	adev->gfx.rlc_feature_version = le32_to_cpu(
					hdr->ucode_feature_version);

	gfx_v6_0_rlc_stop(adev);

	/* disable CG */
	tmp = RREG32(mmRLC_CGCG_CGLS_CTRL) & 0xfffffffc;
	WREG32(mmRLC_CGCG_CGLS_CTRL, tmp);

	gfx_v6_0_rlc_reset(adev);

	gfx_v6_0_init_pg(adev);

	WREG32(mmRLC_RL_BASE, 0);
	WREG32(mmRLC_RL_SIZE, 0);
	WREG32(mmRLC_LB_CNTR_INIT, 0);
	WREG32(mmRLC_LB_CNTR_MAX, 0x00008000);

	mutex_lock(&adev->grbm_idx_mutex);
	gfx_v6_0_select_se_sh(adev, 0xffffffff, 0xffffffff);
	WREG32(mmRLC_LB_INIT_CU_MASK, 0xffffffff);
	WREG32(mmRLC_LB_CNTL, 0x80000004);
	mutex_unlock(&adev->grbm_idx_mutex);

	WREG32(mmRLC_MC_CNTL, 0);
	WREG32(mmRLC_UCODE_CNTL, 0);

	fw_data = (const __le32 *)
		(adev->gfx.rlc_fw->data + le32_to_cpu(hdr->header.ucode_array_offset_bytes));
	fw_size = le32_to_cpu(hdr->header.ucode_size_bytes) / 4;
	WREG32(mmRLC_UCODE_ADDR, 0);
	for (i = 0; i < fw_size; i++) {
		WREG32(mmRLC_UCODE_ADDR, i);
		WREG32(mmRLC_UCODE_DATA, le32_to_cpup(fw_data++));
	}
	WREG32(mmRLC_UCODE_ADDR, 0);

	/* XXX - find out what chips support lbpw */
	gfx_v6_0_enable_lbpw(adev, false);

	gfx_v6_0_rlc_start(adev);

	return 0;
}

static void gfx_v6_0_enable_cgcg(struct amdgpu_device *adev, bool enable)
{
	u32 data, orig, tmp, tmp2;

	orig = data = RREG32(mmRLC_CGCG_CGLS_CTRL);

	if (enable && (adev->cg_flags & AMDGPU_CG_SUPPORT_GFX_CGCG)) {
		gfx_v6_0_enable_gui_idle_interrupt(adev, true);

		tmp = gfx_v6_0_halt_rlc(adev);

		WREG32(mmRLC_SERDES_WR_MASTER_MASK_0, 0xffffffff);
		WREG32(mmRLC_SERDES_WR_MASTER_MASK_1, 0xffffffff);
		WREG32(mmRLC_SERDES_WR_CTRL, 0x00b000ff);

		gfx_v6_0_wait_for_rlc_serdes(adev);

		gfx_v6_0_update_rlc(adev, tmp);

		WREG32(mmRLC_SERDES_WR_CTRL, 0x007000ff);

		data |= RLC_CGCG_CGLS_CTRL__CGCG_EN_MASK | RLC_CGCG_CGLS_CTRL__CGLS_EN_MASK;
	} else {
		gfx_v6_0_enable_gui_idle_interrupt(adev, false);

		RREG32(mmCB_CGTT_SCLK_CTRL);
		RREG32(mmCB_CGTT_SCLK_CTRL);
		RREG32(mmCB_CGTT_SCLK_CTRL);
		RREG32(mmCB_CGTT_SCLK_CTRL);

		data &= ~(RLC_CGCG_CGLS_CTRL__CGCG_EN_MASK | RLC_CGCG_CGLS_CTRL__CGLS_EN_MASK);
	}

	if (orig != data)
		WREG32(mmRLC_CGCG_CGLS_CTRL, data);

}

static void gfx_v6_0_enable_mgcg(struct amdgpu_device *adev, bool enable)
{
	u32 data, orig, tmp = 0;

	if (enable && (adev->cg_flags & AMDGPU_CG_SUPPORT_GFX_MGCG)) {
		orig = data = RREG32(mmCGTS_SM_CTRL_REG);
		data = 0x96940200;
		if (orig != data)
			WREG32(mmCGTS_SM_CTRL_REG, data);

		if (adev->cg_flags & AMDGPU_CG_SUPPORT_GFX_CP_LS) {
			orig = data = RREG32(mmCP_MEM_SLP_CNTL);
			data |= CP_MEM_SLP_CNTL__CP_MEM_LS_EN_MASK;
			if (orig != data)
				WREG32(mmCP_MEM_SLP_CNTL, data);
		}

		orig = data = RREG32(mmRLC_CGTT_MGCG_OVERRIDE);
		data &= 0xffffffc0;
		if (orig != data)
			WREG32(mmRLC_CGTT_MGCG_OVERRIDE, data);

		tmp = gfx_v6_0_halt_rlc(adev);

		mutex_lock(&adev->grbm_idx_mutex);
		gfx_v6_0_select_se_sh(adev, 0xffffffff, 0xffffffff);
		WREG32(mmRLC_SERDES_WR_MASTER_MASK_0, 0xffffffff);
		WREG32(mmRLC_SERDES_WR_MASTER_MASK_1, 0xffffffff);
		WREG32(mmRLC_SERDES_WR_CTRL, 0x00d000ff);
		mutex_unlock(&adev->grbm_idx_mutex);

		gfx_v6_0_update_rlc(adev, tmp);
	} else {
		orig = data = RREG32(mmRLC_CGTT_MGCG_OVERRIDE);
		data |= 0x00000003;
		if (orig != data)
			WREG32(mmRLC_CGTT_MGCG_OVERRIDE, data);

		data = RREG32(mmCP_MEM_SLP_CNTL);
		if (data & CP_MEM_SLP_CNTL__CP_MEM_LS_EN_MASK) {
			data &= ~CP_MEM_SLP_CNTL__CP_MEM_LS_EN_MASK;
			WREG32(mmCP_MEM_SLP_CNTL, data);
		}

		orig = data = RREG32(mmCGTS_SM_CTRL_REG);
		data |= CGTS_SM_CTRL_REG__OVERRIDE_MASK | CGTS_SM_CTRL_REG__LS_OVERRIDE_MASK;
		if (orig != data)
			WREG32(mmCGTS_SM_CTRL_REG, data);

		tmp = gfx_v6_0_halt_rlc(adev);

		mutex_lock(&adev->grbm_idx_mutex);
		gfx_v6_0_select_se_sh(adev, 0xffffffff, 0xffffffff);
		WREG32(mmRLC_SERDES_WR_MASTER_MASK_0, 0xffffffff);
		WREG32(mmRLC_SERDES_WR_MASTER_MASK_1, 0xffffffff);
		WREG32(mmRLC_SERDES_WR_CTRL, 0x00e000ff);
		mutex_unlock(&adev->grbm_idx_mutex);

		gfx_v6_0_update_rlc(adev, tmp);
	}
}

static void gfx_v6_0_update_cg(struct amdgpu_device *adev,
			       bool enable)
{
	gfx_v6_0_enable_gui_idle_interrupt(adev, false);
	/* order matters! */
	if (enable) {
		gfx_v6_0_enable_mgcg(adev, true);
		gfx_v6_0_enable_cgcg(adev, true);
	} else {
		gfx_v6_0_enable_cgcg(adev, false);
		gfx_v6_0_enable_mgcg(adev, false);
	}
	gfx_v6_0_enable_gui_idle_interrupt(adev, true);
}

static void gfx_v6_0_enable_gfx_cgpg(struct amdgpu_device *adev,
			       bool enable)
{
	u32 tmp;

	if (enable && (adev->pg_flags & AMDGPU_PG_SUPPORT_GFX_PG)) {
		tmp = REG_SET_FIELD(0, RLC_TTOP_D, RLC_PUD, 0x10);
		tmp = REG_SET_FIELD(tmp, RLC_TTOP_D, RLC_PDD, 0x10);
		tmp = REG_SET_FIELD(tmp, RLC_TTOP_D, RLC_TTPD, 0x10);
		tmp = REG_SET_FIELD(tmp, RLC_TTOP_D, RLC_MSD, 0x10);
		WREG32(mmRLC_TTOP_D, tmp);

		tmp = RREG32(mmRLC_PG_CNTL);
		tmp |= RLC_PG_CNTL__GFX_PG_ENABLE_MASK;
		WREG32(mmRLC_PG_CNTL, tmp);

		tmp = RREG32(mmRLC_AUTO_PG_CTRL);
		tmp |= RLC_AUTO_PG_CTRL__AUTO_PG_EN_MASK;
		WREG32(mmRLC_AUTO_PG_CTRL, tmp);
	} else {
		tmp = RREG32(mmRLC_AUTO_PG_CTRL);
		tmp &= ~RLC_AUTO_PG_CTRL__AUTO_PG_EN_MASK;
		WREG32(mmRLC_AUTO_PG_CTRL, tmp);

		tmp = RREG32(mmDB_RENDER_CONTROL);
	}
}

static u32 gfx_v6_0_get_cu_active_bitmap(struct amdgpu_device *adev,
					 u32 se, u32 sh)
{
	u32 mask = 0, tmp, tmp1;
	int i;

	gfx_v6_0_select_se_sh(adev, se, sh);
	tmp = RREG32(mmCC_GC_SHADER_ARRAY_CONFIG);
	tmp1 = RREG32(mmGC_USER_SHADER_ARRAY_CONFIG);
	gfx_v6_0_select_se_sh(adev, 0xffffffff, 0xffffffff);

	tmp &= 0xffff0000;

	tmp |= tmp1;
	tmp >>= 16;

	for (i = 0; i < adev->gfx.config.max_cu_per_sh; i ++) {
		mask <<= 1;
		mask |= 1;
	}

	return (~tmp) & mask;
}

static void gfx_v6_0_init_ao_cu_mask(struct amdgpu_device *adev)
{
	uint32_t tmp, active_cu_number;
	struct amdgpu_cu_info cu_info;

	gfx_v6_0_get_cu_info(adev, &cu_info);
	tmp = cu_info.ao_cu_mask;
	active_cu_number = cu_info.number;

	WREG32(mmRLC_PG_AO_CU_MASK, tmp);

	tmp = RREG32(mmRLC_MAX_PG_CU);
	tmp = REG_SET_FIELD(tmp, RLC_MAX_PG_CU, MAX_PU_CU, active_cu_number);
	WREG32(mmRLC_MAX_PG_CU, tmp);
}

static void gfx_v6_0_init_gfx_cgpg(struct amdgpu_device *adev)
{
	u32 tmp;

	WREG32(mmRLC_SAVE_AND_RESTORE_BASE, adev->gfx.rlc.save_restore_gpu_addr >> 8);

	tmp = RREG32(mmRLC_PG_CNTL);
	tmp |= RLC_PG_CNTL__GFX_PG_SRC_MASK;
	WREG32(mmRLC_PG_CNTL, tmp);

	WREG32(mmRLC_CLEAR_STATE_RESTORE_BASE, adev->gfx.rlc.clear_state_gpu_addr >> 8);

	tmp = RREG32(mmRLC_AUTO_PG_CTRL);
	tmp = REG_SET_FIELD(tmp, RLC_AUTO_PG_CTRL, GRBM_REG_SGIT, 0x700);
	tmp = REG_SET_FIELD(tmp, RLC_AUTO_PG_CTRL, PG_AFTER_GRBM_REG_ST, 0);
	WREG32(mmRLC_AUTO_PG_CTRL, tmp);
}

static void gfx_v6_0_update_gfx_pg(struct amdgpu_device *adev, bool enable)
{
	gfx_v6_0_enable_gfx_cgpg(adev, enable);
}

static u32 gfx_v6_0_get_csb_size(struct amdgpu_device *adev)
{
	u32 count = 0;
	const struct cs_section_def *sect = NULL;
	const struct cs_extent_def *ext = NULL;

	if (adev->gfx.rlc.cs_data == NULL)
		return 0;

	/* begin clear state */
	count += 2;
	/* context control state */
	count += 3;

	for (sect = adev->gfx.rlc.cs_data; sect->section != NULL; ++sect) {
		for (ext = sect->section; ext->extent != NULL; ++ext) {
			if (sect->id == SECT_CONTEXT)
				count += 2 + ext->reg_count;
			else
				return 0;
		}
	}

	/* pa_sc_raster_config */
	count += 3;
	/* end clear state */
	count += 2;
	/* clear state */
	count += 2;

	return count;
}

static void gfx_v6_0_get_csb_buffer(struct amdgpu_device *adev, volatile u32 *buffer)
{
	u32 count = 0, i;
	const struct cs_section_def *sect = NULL;
	const struct cs_extent_def *ext = NULL;

	if (adev->gfx.rlc.cs_data == NULL)
		return;
	if (buffer == NULL)
		return;

	buffer[count++] = cpu_to_le32(PACKET3(PACKET3_PREAMBLE_CNTL, 0));
	buffer[count++] = cpu_to_le32(PACKET3_PREAMBLE_BEGIN_CLEAR_STATE);

	buffer[count++] = cpu_to_le32(PACKET3(PACKET3_CONTEXT_CONTROL, 1));
	buffer[count++] = cpu_to_le32(0x80000000);
	buffer[count++] = cpu_to_le32(0x80000000);

	for (sect = adev->gfx.rlc.cs_data; sect->section != NULL; ++sect) {
		for (ext = sect->section; ext->extent != NULL; ++ext) {
			if (sect->id == SECT_CONTEXT) {
				buffer[count++] =
					cpu_to_le32(PACKET3(PACKET3_SET_CONTEXT_REG, ext->reg_count));
				buffer[count++] = cpu_to_le32(ext->reg_index - 0xa000);
				for (i = 0; i < ext->reg_count; i++)
					buffer[count++] = cpu_to_le32(ext->extent[i]);
			} else {
				return;
			}
		}
	}

	buffer[count++] = cpu_to_le32(PACKET3(PACKET3_SET_CONTEXT_REG, 1));
	buffer[count++] = cpu_to_le32(mmPA_SC_RASTER_CONFIG - PACKET3_SET_CONTEXT_REG_START);
	switch (adev->asic_type) {
	case CHIP_TAHITI:
	case CHIP_PITCAIRN:
		buffer[count++] = cpu_to_le32(0x2a00126a);
		break;
	case CHIP_VERDE:
		buffer[count++] = cpu_to_le32(0x0000124a);
		break;
	case CHIP_OLAND:
		buffer[count++] = cpu_to_le32(0x00000082);
		break;
	case CHIP_HAINAN:
		buffer[count++] = cpu_to_le32(0x00000000);
		break;
	default:
		buffer[count++] = cpu_to_le32(0x00000000);
		break;
	}

	buffer[count++] = cpu_to_le32(PACKET3(PACKET3_PREAMBLE_CNTL, 0));
	buffer[count++] = cpu_to_le32(PACKET3_PREAMBLE_END_CLEAR_STATE);

	buffer[count++] = cpu_to_le32(PACKET3(PACKET3_CLEAR_STATE, 0));
	buffer[count++] = cpu_to_le32(0);
}

static void gfx_v6_0_init_pg(struct amdgpu_device *adev)
{
	if (adev->pg_flags & (AMDGPU_PG_SUPPORT_GFX_PG |
			      AMDGPU_PG_SUPPORT_GFX_SMG |
			      AMDGPU_PG_SUPPORT_GFX_DMG |
			      AMDGPU_PG_SUPPORT_CP |
			      AMDGPU_PG_SUPPORT_GDS |
			      AMDGPU_PG_SUPPORT_RLC_SMU_HS)) {
		if (adev->pg_flags & AMDGPU_PG_SUPPORT_GFX_PG) {
			gfx_v6_0_init_gfx_cgpg(adev);
		}
		gfx_v6_0_init_ao_cu_mask(adev);
		gfx_v6_0_update_gfx_pg(adev, true);
	}
}

static void gfx_v6_0_fini_pg(struct amdgpu_device *adev)
{
	if (adev->pg_flags & (AMDGPU_PG_SUPPORT_GFX_PG |
			      AMDGPU_PG_SUPPORT_GFX_SMG |
			      AMDGPU_PG_SUPPORT_GFX_DMG |
			      AMDGPU_PG_SUPPORT_CP |
			      AMDGPU_PG_SUPPORT_GDS |
			      AMDGPU_PG_SUPPORT_RLC_SMU_HS)) {
		gfx_v6_0_update_gfx_pg(adev, false);
	}
}

/**
 * si_get_gpu_clock_counter - return GPU clock counter snapshot
 *
 * @rdev: radeon_device pointer
 *
 * Fetches a GPU clock counter snapshot (SI).
 * Returns the 64 bit clock counter snapshot.
 */
uint64_t gfx_v6_0_get_gpu_clock_counter(struct amdgpu_device *adev)
{
	uint64_t clock;

	mutex_lock(&adev->gfx.gpu_clock_mutex);
	WREG32(mmRLC_CAPTURE_GPU_CLOCK_COUNT, 1);
	clock = (uint64_t)RREG32(mmRLC_GPU_CLOCK_COUNT_LSB) |
	        ((uint64_t)RREG32(mmRLC_GPU_CLOCK_COUNT_MSB) << 32ULL);
	mutex_unlock(&adev->gfx.gpu_clock_mutex);
	return clock;
}

static int gfx_v6_0_early_init(void *handle)
{
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;

	adev->gfx.num_gfx_rings = GFX6_NUM_GFX_RINGS;
	adev->gfx.num_compute_rings = GFX6_NUM_COMPUTE_RINGS;
	gfx_v6_0_set_ring_funcs(adev);
	gfx_v6_0_set_irq_funcs(adev);
	gfx_v6_0_set_gds_init(adev);

	return 0;
}

static int gfx_v6_0_sw_init(void *handle)
{
	struct amdgpu_ring *ring;
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;
	int i, r;

	/* EOP Event */
	r = amdgpu_irq_add_id(adev, 181, &adev->gfx.eop_irq);
	if (r)
		return r;

	gfx_v6_0_scratch_init(adev);

	r = gfx_v6_0_init_microcode(adev);
	if (r) {
		DRM_ERROR("Failed to load gfx firmware!\n");
		return r;
	}

	r = gfx_v6_0_rlc_init(adev);
	if (r) {
		DRM_ERROR("Failed to init rlc BOs!\n");
		return r;
	}

	for (i = 0; i < adev->gfx.num_gfx_rings; i++) {
		ring = &adev->gfx.gfx_ring[i];
		ring->ring_obj = NULL;
		sprintf(ring->name, "gfx");
		r = amdgpu_ring_init(adev, ring, 1024 * 1024,
				     PACKET2(0), 0xf,
				     &adev->gfx.eop_irq, AMDGPU_CP_IRQ_GFX_EOP,
				     AMDGPU_RING_TYPE_GFX);
		if (r)
			return r;
	}

	/* set up the compute queues */
	for (i = 0; i < adev->gfx.num_compute_rings; i++) {
		unsigned irq_type;
		ring = &adev->gfx.compute_ring[i];
		ring->ring_obj = NULL;
		sprintf(ring->name, "comp %d", i);
		irq_type = AMDGPU_CP_IRQ_COMPUTE_MEC1_PIPE0_EOP + i;
		r = amdgpu_ring_init(adev, ring, 1024 * 1024,
				     PACKET2(0), 0xf,
				     &adev->gfx.eop_irq, irq_type,
				     AMDGPU_RING_TYPE_COMPUTE);
		if (r)
			return r;
	}

	/* reserve GDS, GWS and OA resource for gfx */
	/* DEBUG */ printk(KERN_ALERT "SI GFX reserve gds\n");
	r = amdgpu_bo_create(adev, adev->gds.mem.gfx_partition_size,
			PAGE_SIZE, true,
			AMDGPU_GEM_DOMAIN_GDS, 0,
			NULL, NULL, &adev->gds.gds_gfx_bo);
	if (r)
		return r;

	r = amdgpu_bo_create(adev, adev->gds.gws.gfx_partition_size,
		PAGE_SIZE, true,
		AMDGPU_GEM_DOMAIN_GWS, 0,
		NULL, NULL, &adev->gds.gws_gfx_bo);
	if (r)
		return r;

	r = amdgpu_bo_create(adev, adev->gds.oa.gfx_partition_size,
			PAGE_SIZE, true,
			AMDGPU_GEM_DOMAIN_OA, 0,
			NULL, NULL, &adev->gds.oa_gfx_bo);
	if (r)
		return r;

	return r;
}

static int gfx_v6_0_sw_fini(void *handle)
{
	int i;
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;

	amdgpu_bo_unref(&adev->gds.oa_gfx_bo);
	amdgpu_bo_unref(&adev->gds.gws_gfx_bo);
	amdgpu_bo_unref(&adev->gds.gds_gfx_bo);

	for (i = 0; i < adev->gfx.num_gfx_rings; i++)
		amdgpu_ring_fini(&adev->gfx.gfx_ring[i]);
	for (i = 0; i < adev->gfx.num_compute_rings; i++)
		amdgpu_ring_fini(&adev->gfx.compute_ring[i]);

	gfx_v6_0_rlc_fini(adev);

	return 0;
}

static int gfx_v6_0_hw_init(void *handle)
{
	int r;
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;
	/* DEBUG */ printk(KERN_ALERT "SI GFX hw_init\n");

	gfx_v6_0_gpu_init(adev);

	/* init rlc */
	r = gfx_v6_0_rlc_resume(adev);
	if (r)
		return r;

	r = gfx_v6_0_cp_resume(adev);
	if (r)
		return r;

	adev->gfx.ce_ram_size = 0x8000;

	return r;
}

static int gfx_v6_0_hw_fini(void *handle)
{
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;

	gfx_v6_0_cp_enable(adev, false);
	gfx_v6_0_rlc_stop(adev);
	gfx_v6_0_fini_pg(adev);

	return 0;
}

static int gfx_v6_0_suspend(void *handle)
{
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;

	return gfx_v6_0_hw_fini(adev);
}

static int gfx_v6_0_resume(void *handle)
{
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;

	return gfx_v6_0_hw_init(adev);
}

static bool gfx_v6_0_is_idle(void *handle)
{
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;

	if (RREG32(mmGRBM_STATUS) & GRBM_STATUS__GUI_ACTIVE_MASK)
		return false;
	else
		return true;
}

static int gfx_v6_0_wait_for_idle(void *handle)
{
	unsigned i;
	u32 tmp;
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;

	for (i = 0; i < adev->usec_timeout; i++) {
		/* read MC_STATUS */
		tmp = RREG32(mmGRBM_STATUS) & GRBM_STATUS__GUI_ACTIVE_MASK;

		if (!tmp)
			return 0;
		udelay(1);
	}
	return -ETIMEDOUT;
}

static void gfx_v6_0_print_status(void *handle)
{
	int i;
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;

	dev_info(adev->dev, "GFX 6.x registers\n");
	dev_info(adev->dev, "  GRBM_STATUS=0x%08X\n",
		RREG32(mmGRBM_STATUS));
}

static int gfx_v6_0_soft_reset(void *handle)
{
	u32 grbm_soft_reset = 0, srbm_soft_reset = 0;
	u32 tmp;
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;

	/* GRBM_STATUS */
	tmp = RREG32(mmGRBM_STATUS);
	if (tmp & (GRBM_STATUS__PA_BUSY_MASK | GRBM_STATUS__SC_BUSY_MASK |
		   GRBM_STATUS__BCI_BUSY_MASK | GRBM_STATUS__SX_BUSY_MASK |
		   GRBM_STATUS__TA_BUSY_MASK | GRBM_STATUS__VGT_BUSY_MASK |
		   GRBM_STATUS__DB_BUSY_MASK | GRBM_STATUS__CB_BUSY_MASK |
		   GRBM_STATUS__GDS_BUSY_MASK | GRBM_STATUS__SPI_BUSY_MASK |
		   GRBM_STATUS__IA_BUSY_MASK | GRBM_STATUS__IA_BUSY_NO_DMA_MASK))
		grbm_soft_reset |= GRBM_SOFT_RESET__SOFT_RESET_CP_MASK |
			GRBM_SOFT_RESET__SOFT_RESET_VGT_MASK;

	if (tmp & (GRBM_STATUS__CP_BUSY_MASK | GRBM_STATUS__CP_COHERENCY_BUSY_MASK)) {
		grbm_soft_reset |= GRBM_SOFT_RESET__SOFT_RESET_CP_MASK;
		srbm_soft_reset |= SRBM_SOFT_RESET__SOFT_RESET_GRBM_MASK;
	}

	/* GRBM_STATUS2 */
	tmp = RREG32(mmGRBM_STATUS2);
	if (tmp & GRBM_STATUS2__RLC_BUSY_MASK)
		grbm_soft_reset |= GRBM_SOFT_RESET__SOFT_RESET_RLC_MASK;

	/* SRBM_STATUS */
	tmp = RREG32(mmSRBM_STATUS);
	if (tmp & SRBM_STATUS__GRBM_RQ_PENDING_MASK)
		srbm_soft_reset |= SRBM_SOFT_RESET__SOFT_RESET_GRBM_MASK;

	if (grbm_soft_reset || srbm_soft_reset) {
		gfx_v6_0_print_status((void *)adev);
		/* disable CG/PG */
		gfx_v6_0_fini_pg(adev);
		gfx_v6_0_update_cg(adev, false);

		/* stop the rlc */
		gfx_v6_0_rlc_stop(adev);

		/* Disable GFX parsing/prefetching */
		WREG32(mmCP_ME_CNTL, CP_ME_CNTL__CP_ME_HALT_MASK | CP_ME_CNTL__CP_PFP_HALT_MASK | CP_ME_CNTL__CP_CE_HALT_MASK);

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
		gfx_v6_0_print_status((void *)adev);
	}
	return 0;
}

static void gfx_v6_0_set_gfx_eop_interrupt_state(struct amdgpu_device *adev,
						 enum amdgpu_interrupt_state state)
{
	u32 cp_int_cntl;

	switch (state) {
	case AMDGPU_IRQ_STATE_DISABLE:
		/* DEBUG */ printk(KERN_ALERT "eop irq0 disable\n");
		cp_int_cntl = RREG32(mmCP_INT_CNTL_RING0);
		cp_int_cntl &= ~(CP_INT_CNTL_RING2__TIME_STAMP_INT_ENABLE_MASK) ;
		WREG32(mmCP_INT_CNTL_RING0, cp_int_cntl);
		break;
	case AMDGPU_IRQ_STATE_ENABLE:
		/* DEBUG */ printk(KERN_ALERT "eop irq0 enable\n");
		cp_int_cntl = RREG32(mmCP_INT_CNTL_RING0);
		cp_int_cntl |= CP_INT_CNTL_RING2__TIME_STAMP_INT_ENABLE_MASK;
		WREG32(mmCP_INT_CNTL_RING0, cp_int_cntl);
		break;
	default:
		break;
	}
}

static void gfx_v6_0_set_compute_eop_interrupt_state(struct amdgpu_device *adev,
						     int ring,
						     enum amdgpu_interrupt_state state)
{
	u32 int_cntl, int_cntl_reg;

	switch (ring) {
	case 0:
		int_cntl_reg = mmCP_INT_CNTL_RING1;
		break;
	case 1:
		int_cntl_reg = mmCP_INT_CNTL_RING2;
		break;
	default:
		DRM_DEBUG("invalid ring %d\n", ring);
		return;
	}


	switch (state) {
	case AMDGPU_IRQ_STATE_DISABLE:
		/* DEBUG */ printk(KERN_ALERT "eop compute irq %d disable\n", ring);
		int_cntl = RREG32(int_cntl_reg);
		int_cntl &= ~CP_INT_CNTL_RING2__TIME_STAMP_INT_ENABLE_MASK;
		WREG32(int_cntl_reg, int_cntl);
		break;
	case AMDGPU_IRQ_STATE_ENABLE:
		/* DEBUG */ printk(KERN_ALERT "eop compute irq %d enable\n", ring);
		int_cntl = RREG32(int_cntl_reg);
		int_cntl |= CP_INT_CNTL_RING2__TIME_STAMP_INT_ENABLE_MASK;
		WREG32(int_cntl_reg, int_cntl);
		break;
	default:
		break;
	}
}

static int gfx_v6_0_set_eop_interrupt_state(struct amdgpu_device *adev,
					    struct amdgpu_irq_src *src,
					    unsigned type,
					    enum amdgpu_interrupt_state state)
{
	switch (type) {
	case AMDGPU_CP_IRQ_GFX_EOP:
		gfx_v6_0_set_gfx_eop_interrupt_state(adev, state);
		break;
	case AMDGPU_CP_IRQ_COMPUTE_MEC1_PIPE0_EOP:
		gfx_v6_0_set_compute_eop_interrupt_state(adev, 0, state);
		break;
	case AMDGPU_CP_IRQ_COMPUTE_MEC1_PIPE1_EOP:
		gfx_v6_0_set_compute_eop_interrupt_state(adev, 1, state);
		break;
	default:
		break;
	}
	return 0;
}

static int gfx_v6_0_eop_irq(struct amdgpu_device *adev,
			    struct amdgpu_irq_src *source,
			    struct amdgpu_iv_entry *entry)
{
	int i;

	DRM_DEBUG("IH: CP EOP\n");
	/* DEBUG */ printk(KERN_ALERT "EOP from ring %d\n", entry->ring_id);
	switch (entry->ring_id) {
	case 0:
		amdgpu_fence_process(&adev->gfx.gfx_ring[0]);
		break;
	case 1:
		amdgpu_fence_process(&adev->gfx.compute_ring[0]);
		break;
	case 2:
		amdgpu_fence_process(&adev->gfx.compute_ring[1]);
		break;
	}
	return 0;
}

static int gfx_v6_0_set_clockgating_state(void *handle,
					  enum amd_clockgating_state state)
{
	bool gate = false;
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;

	if (state == AMD_CG_STATE_GATE)
		gate = true;

	gfx_v6_0_enable_gui_idle_interrupt(adev, false);
	/* order matters! */
	if (gate) {
		gfx_v6_0_enable_mgcg(adev, true);
		gfx_v6_0_enable_cgcg(adev, true);
	} else {
		gfx_v6_0_enable_cgcg(adev, false);
		gfx_v6_0_enable_mgcg(adev, false);
	}
	gfx_v6_0_enable_gui_idle_interrupt(adev, true);

	return 0;
}

static int gfx_v6_0_set_powergating_state(void *handle,
					  enum amd_powergating_state state)
{
	bool gate = false;
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;

	if (state == AMD_PG_STATE_GATE)
		gate = true;

	if (adev->pg_flags & (AMDGPU_PG_SUPPORT_GFX_PG |
			      AMDGPU_PG_SUPPORT_GFX_SMG |
			      AMDGPU_PG_SUPPORT_GFX_DMG |
			      AMDGPU_PG_SUPPORT_CP |
			      AMDGPU_PG_SUPPORT_GDS |
			      AMDGPU_PG_SUPPORT_RLC_SMU_HS)) {
		gfx_v6_0_update_gfx_pg(adev, gate);
	}

	return 0;
}

const struct amd_ip_funcs gfx_v6_0_ip_funcs = {
	.early_init = gfx_v6_0_early_init,
	.late_init = NULL,
	.sw_init = gfx_v6_0_sw_init,
	.sw_fini = gfx_v6_0_sw_fini,
	.hw_init = gfx_v6_0_hw_init,
	.hw_fini = gfx_v6_0_hw_fini,
	.suspend = gfx_v6_0_suspend,
	.resume = gfx_v6_0_resume,
	.is_idle = gfx_v6_0_is_idle,
	.wait_for_idle = gfx_v6_0_wait_for_idle,
	.soft_reset = gfx_v6_0_soft_reset,
	.print_status = gfx_v6_0_print_status,
	.set_clockgating_state = gfx_v6_0_set_clockgating_state,
	.set_powergating_state = gfx_v6_0_set_powergating_state,
};

static const struct amdgpu_ring_funcs gfx_v6_0_ring_funcs_gfx = {
	.get_rptr = gfx_v6_0_ring_get_rptr_gfx,
	.get_wptr = gfx_v6_0_ring_get_wptr_gfx,
	.set_wptr = gfx_v6_0_ring_set_wptr_gfx,
	.parse_cs = NULL,
	.emit_ib = gfx_v6_0_ring_emit_ib_gfx,
	.emit_fence = gfx_v6_0_ring_emit_fence_gfx,
	.emit_semaphore = gfx_v6_0_ring_emit_semaphore,
	.emit_vm_flush = gfx_v6_0_ring_emit_vm_flush,
	.emit_gds_switch = NULL,
	.emit_hdp_flush = NULL,
	.test_ring = gfx_v6_0_ring_test_ring,
	.test_ib = gfx_v6_0_ring_test_ib,
	.insert_nop = amdgpu_ring_insert_nop,
};

static const struct amdgpu_ring_funcs gfx_v6_0_ring_funcs_compute = {
	.get_rptr = gfx_v6_0_ring_get_rptr_compute,
	.get_wptr = gfx_v6_0_ring_get_wptr_compute,
	.set_wptr = gfx_v6_0_ring_set_wptr_compute,
	.parse_cs = NULL,
	.emit_ib = gfx_v6_0_ring_emit_ib_compute,
	.emit_fence = gfx_v6_0_ring_emit_fence_compute,
	.emit_semaphore = gfx_v6_0_ring_emit_semaphore,
	.emit_vm_flush = gfx_v6_0_ring_emit_vm_flush,
	.emit_gds_switch = NULL,
	.emit_hdp_flush = NULL,
	.test_ring = gfx_v6_0_ring_test_ring,
	.test_ib = gfx_v6_0_ring_test_ib,
	.insert_nop = amdgpu_ring_insert_nop,
};

static void gfx_v6_0_set_ring_funcs(struct amdgpu_device *adev)
{
	int i;

	for (i = 0; i < adev->gfx.num_gfx_rings; i++)
		adev->gfx.gfx_ring[i].funcs = &gfx_v6_0_ring_funcs_gfx;
	for (i = 0; i < adev->gfx.num_compute_rings; i++)
		adev->gfx.compute_ring[i].funcs = &gfx_v6_0_ring_funcs_compute;
}

static const struct amdgpu_irq_src_funcs gfx_v6_0_eop_irq_funcs = {
	.set = gfx_v6_0_set_eop_interrupt_state,
	.process = gfx_v6_0_eop_irq,
};

static void gfx_v6_0_set_irq_funcs(struct amdgpu_device *adev)
{
	adev->gfx.eop_irq.num_types = 3;
	adev->gfx.eop_irq.funcs = &gfx_v6_0_eop_irq_funcs;
}

int gfx_v6_0_get_cu_info(struct amdgpu_device *adev, struct amdgpu_cu_info *cu_info)
{
	int i, j, k, counter, active_cu_number = 0;
	u32 mask, bitmap, ao_bitmap, ao_cu_mask = 0;

	if (!adev || !cu_info)
		return -EINVAL;

	mutex_lock(&adev->grbm_idx_mutex);
	for (i = 0; i < adev->gfx.config.max_shader_engines; i++) {
		for (j = 0; j < adev->gfx.config.max_sh_per_se; j++) {
			mask = 1;
			ao_bitmap = 0;
			counter = 0;
			bitmap = gfx_v6_0_get_cu_active_bitmap(adev, i, j);
			cu_info->bitmap[i][j] = bitmap;

			for (k = 0; k < adev->gfx.config.max_cu_per_sh; k ++) {
				if (bitmap & mask) {
					if (counter < 2)
						ao_bitmap |= mask;
					counter ++;
				}
				mask <<= 1;
			}
			active_cu_number += counter;
			ao_cu_mask |= (ao_bitmap << (i * 16 + j * 8));
		}
	}

	cu_info->number = active_cu_number;
	cu_info->ao_cu_mask = ao_cu_mask;
	mutex_unlock(&adev->grbm_idx_mutex);
	return 0;
}

static void gfx_v6_0_set_gds_init(struct amdgpu_device *adev)
{
	/* init asci gds info */
	adev->gds.mem.total_size = 64*1024;
	adev->gds.gws.total_size = 64;
	adev->gds.oa.total_size = 16;

	adev->gds.mem.gfx_partition_size = 4096;
	adev->gds.mem.cs_partition_size = 4096;

	adev->gds.gws.gfx_partition_size = 4;
	adev->gds.gws.cs_partition_size = 4;

	adev->gds.oa.gfx_partition_size = 4;
}
