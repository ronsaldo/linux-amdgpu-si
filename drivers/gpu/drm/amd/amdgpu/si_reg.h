/*
 * Copyright 2010 Advanced Micro Devices, Inc.
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
 * Authors: Alex Deucher
 */

#ifndef __SI_REG_H__

/* max cursor sizes (in pixels) */
#define CURSOR_WIDTH 64
#define CURSOR_HEIGHT 64

#define SI_WB_IH_WPTR_OFFSET   2048

/* Registers taken from r600d.h and r600_reg.h */
#define	mmSI_PORT_INDEX					(0x0038/4)
#define	mmSI_PORT_DATA					(0x003C/4)

#define mmSI_UVD_CTX_INDEX                  0xf4a0/4
#define mmSI_UVD_CTX_DATA                   (0xf4a4/4)

/* Registers taken from evergreen_reg.h */
#define mmSI_PIF_PHY0_INDEX                        (0x8/4)
#define mmSI_PIF_PHY0_DATA                         (0xc/4)
#define mmSI_PIF_PHY1_INDEX                        (0x10/4)
#define mmSI_PIF_PHY1_DATA                         (0x14/4)

#define mmSI_CG_IND_ADDR                           (0x8f8/4)
#define mmSI_CG_IND_DATA                           (0x8fc/4)

#define DMA0_REGISTER_OFFSET                              0x0 /* not a register */
#define DMA1_REGISTER_OFFSET                              0x200 /* not a register */
#define DMA_MAX_INSTANCE                                  2

/* interruption */
#       define IH_MC_SWAP(x)                              ((x) << 1)
#       define MC_WRREQ_CREDIT(x)                         ((x) << 15)
#       define MC_WR_CLEAN_CNT(x)                         ((x) << 20)
#       define MC_VMID(x)                                 ((x) << 25)

/* display controller offsets used for crtc/cur/lut/grph/viewport/etc. */
#define CRTC0_REGISTER_OFFSET                 ((0x6df0 - 0x6df0)/4)
#define CRTC1_REGISTER_OFFSET                 ((0x79f0 - 0x6df0)/4)
#define CRTC2_REGISTER_OFFSET                 ((0x105f0 - 0x6df0)/4)
#define CRTC3_REGISTER_OFFSET                 ((0x111f0 - 0x6df0)/4)
#define CRTC4_REGISTER_OFFSET                 ((0x11df0 - 0x6df0)/4)
#define CRTC5_REGISTER_OFFSET                 ((0x129f0 - 0x6df0)/4)

/* SI */
#define mmSI_DC_GPIO_HPD_MASK                      (0x65b0 / 4)
#define mmSI_DC_GPIO_HPD_A                         (0x65b4 / 4)
#define mmSI_DC_GPIO_HPD_EN                        (0x65b8 / 4)
#define mmSI_DC_GPIO_HPD_Y                         (0x65bc / 4)

#define mmSI_GRPH_CONTROL                          (0x6804 /4)
#       define SI_GRPH_DEPTH(x)                  (((x) & 0x3) << 0)
#       define SI_GRPH_DEPTH_8BPP                0
#       define SI_GRPH_DEPTH_16BPP               1
#       define SI_GRPH_DEPTH_32BPP               2
#       define SI_GRPH_NUM_BANKS(x)              (((x) & 0x3) << 2)
#       define SI_ADDR_SURF_2_BANK               0
#       define SI_ADDR_SURF_4_BANK               1
#       define SI_ADDR_SURF_8_BANK               2
#       define SI_ADDR_SURF_16_BANK              3
#       define SI_GRPH_Z(x)                      (((x) & 0x3) << 4)
#       define SI_GRPH_BANK_WIDTH(x)             (((x) & 0x3) << 6)
#       define SI_ADDR_SURF_BANK_WIDTH_1         0
#       define SI_ADDR_SURF_BANK_WIDTH_2         1
#       define SI_ADDR_SURF_BANK_WIDTH_4         2
#       define SI_ADDR_SURF_BANK_WIDTH_8         3
#       define SI_GRPH_FORMAT(x)                 (((x) & 0x7) << 8)
/* 8 BPP */
#       define SI_GRPH_FORMAT_INDEXED            0
/* 16 BPP */
#       define SI_GRPH_FORMAT_ARGB1555           0
#       define SI_GRPH_FORMAT_ARGB565            1
#       define SI_GRPH_FORMAT_ARGB4444           2
#       define SI_GRPH_FORMAT_AI88               3
#       define SI_GRPH_FORMAT_MONO16             4
#       define SI_GRPH_FORMAT_BGRA5551           5
/* 32 BPP */
#       define SI_GRPH_FORMAT_ARGB8888           0
#       define SI_GRPH_FORMAT_ARGB2101010        1
#       define SI_GRPH_FORMAT_32BPP_DIG          2
#       define SI_GRPH_FORMAT_8B_ARGB2101010     3
#       define SI_GRPH_FORMAT_BGRA1010102        4
#       define SI_GRPH_FORMAT_8B_BGRA1010102     5
#       define SI_GRPH_FORMAT_RGB111110          6
#       define SI_GRPH_FORMAT_BGR101111          7
#       define SI_GRPH_BANK_HEIGHT(x)            (((x) & 0x3) << 11)
#       define SI_ADDR_SURF_BANK_HEIGHT_1        0
#       define SI_ADDR_SURF_BANK_HEIGHT_2        1
#       define SI_ADDR_SURF_BANK_HEIGHT_4        2
#       define SI_ADDR_SURF_BANK_HEIGHT_8        3
#       define SI_GRPH_TILE_SPLIT(x)             (((x) & 0x7) << 13)
#       define SI_ADDR_SURF_TILE_SPLIT_64B       0
#       define SI_ADDR_SURF_TILE_SPLIT_128B      1
#       define SI_ADDR_SURF_TILE_SPLIT_256B      2
#       define SI_ADDR_SURF_TILE_SPLIT_512B      3
#       define SI_ADDR_SURF_TILE_SPLIT_1KB       4
#       define SI_ADDR_SURF_TILE_SPLIT_2KB       5
#       define SI_ADDR_SURF_TILE_SPLIT_4KB       6
#       define SI_GRPH_MACRO_TILE_ASPECT(x)      (((x) & 0x3) << 18)
#       define SI_ADDR_SURF_MACRO_TILE_ASPECT_1  0
#       define SI_ADDR_SURF_MACRO_TILE_ASPECT_2  1
#       define SI_ADDR_SURF_MACRO_TILE_ASPECT_4  2
#       define SI_ADDR_SURF_MACRO_TILE_ASPECT_8  3
#       define SI_GRPH_ARRAY_MODE(x)             (((x) & 0x7) << 20)
#       define SI_GRPH_ARRAY_LINEAR_GENERAL      0
#       define SI_GRPH_ARRAY_LINEAR_ALIGNED      1
#       define SI_GRPH_ARRAY_1D_TILED_THIN1      2
#       define SI_GRPH_ARRAY_2D_TILED_THIN1      4
#       define SI_GRPH_PIPE_CONFIG(x)		 (((x) & 0x1f) << 24)
#       define SI_ADDR_SURF_P2			 0
#       define SI_ADDR_SURF_P4_8x16		 4
#       define SI_ADDR_SURF_P4_16x16		 5
#       define SI_ADDR_SURF_P4_16x32		 6
#       define SI_ADDR_SURF_P4_32x32		 7
#       define SI_ADDR_SURF_P8_16x16_8x16	 8
#       define SI_ADDR_SURF_P8_16x32_8x16	 9
#       define SI_ADDR_SURF_P8_32x32_8x16	 10
#       define SI_ADDR_SURF_P8_16x32_16x16	 11
#       define SI_ADDR_SURF_P8_32x32_16x16	 12
#       define SI_ADDR_SURF_P8_32x32_16x32	 13
#       define SI_ADDR_SURF_P8_32x64_32x32	 14

/* registers taken from ni_reg.h*/
#define mmNI_INPUT_GAMMA_CONTROL                         (0x6840 / 4)
#       define NI_GRPH_INPUT_GAMMA_MODE(x)             (((x) & 0x3) << 0)
#       define NI_INPUT_GAMMA_USE_LUT                  0
#       define NI_INPUT_GAMMA_BYPASS                   1
#       define NI_INPUT_GAMMA_SRGB_24                  2
#       define NI_INPUT_GAMMA_XVYCC_222                3
#       define NI_OVL_INPUT_GAMMA_MODE(x)              (((x) & 0x3) << 4)

#define mmNI_PRESCALE_GRPH_CONTROL                       (0x68b4 / 4)
#       define NI_GRPH_PRESCALE_BYPASS                 (1 << 4)

#define mmNI_PRESCALE_OVL_CONTROL                        (0x68c4 / 4)
#       define NI_OVL_PRESCALE_BYPASS                  (1 << 4)

#define mmNI_INPUT_CSC_CONTROL                           (0x68d4 / 4)
#       define NI_INPUT_CSC_GRPH_MODE(x)               (((x) & 0x3) << 0)
#       define NI_INPUT_CSC_BYPASS                     0
#       define NI_INPUT_CSC_PROG_COEFF                 1
#       define NI_INPUT_CSC_PROG_SHARED_MATRIXA        2
#       define NI_INPUT_CSC_OVL_MODE(x)                (((x) & 0x3) << 4)

#define mmNI_OUTPUT_CSC_CONTROL                          (0x68f0 / 4)
#       define NI_OUTPUT_CSC_GRPH_MODE(x)              (((x) & 0x7) << 0)
#       define NI_OUTPUT_CSC_BYPASS                    0
#       define NI_OUTPUT_CSC_TV_RGB                    1
#       define NI_OUTPUT_CSC_YCBCR_601                 2
#       define NI_OUTPUT_CSC_YCBCR_709                 3
#       define NI_OUTPUT_CSC_PROG_COEFF                4
#       define NI_OUTPUT_CSC_PROG_SHARED_MATRIXB       5
#       define NI_OUTPUT_CSC_OVL_MODE(x)               (((x) & 0x7) << 4)

#define mmNI_DEGAMMA_CONTROL                             (0x6960 / 4)
#       define NI_GRPH_DEGAMMA_MODE(x)                 (((x) & 0x3) << 0)
#       define NI_DEGAMMA_BYPASS                       0
#       define NI_DEGAMMA_SRGB_24                      1
#       define NI_DEGAMMA_XVYCC_222                    2
#       define NI_OVL_DEGAMMA_MODE(x)                  (((x) & 0x3) << 4)
#       define NI_ICON_DEGAMMA_MODE(x)                 (((x) & 0x3) << 8)
#       define NI_CURSOR_DEGAMMA_MODE(x)               (((x) & 0x3) << 12)

#define mmNI_GAMUT_REMAP_CONTROL                         (0x6964 / 4)
#       define NI_GRPH_GAMUT_REMAP_MODE(x)             (((x) & 0x3) << 0)
#       define NI_GAMUT_REMAP_BYPASS                   0
#       define NI_GAMUT_REMAP_PROG_COEFF               1
#       define NI_GAMUT_REMAP_PROG_SHARED_MATRIXA      2
#       define NI_GAMUT_REMAP_PROG_SHARED_MATRIXB      3
#       define NI_OVL_GAMUT_REMAP_MODE(x)              (((x) & 0x3) << 4)

#define mmNI_REGAMMA_CONTROL                             (0x6a80 / 4)
#       define NI_GRPH_REGAMMA_MODE(x)                 (((x) & 0x7) << 0)
#       define NI_REGAMMA_BYPASS                       0
#       define NI_REGAMMA_SRGB_24                      1
#       define NI_REGAMMA_XVYCC_222                    2
#       define NI_REGAMMA_PROG_A                       3
#       define NI_REGAMMA_PROG_B                       4
#       define NI_OVL_REGAMMA_MODE(x)                  (((x) & 0x7) << 4)

#define mmNI_DP_MSE_LINK_TIMING                          (0x73a0 / 4)
#	define NI_DP_MSE_LINK_FRAME			(((x) & 0x3ff) << 0)
#	define NI_DP_MSE_LINK_LINE                      (((x) & 0x3) << 16)

#define mmNI_DP_MSE_MISC_CNTL                            (0x736c / 4)
#       define NI_DP_MSE_BLANK_CODE                    (((x) & 0x1) << 0)
#       define NI_DP_MSE_TIMESTAMP_MODE                (((x) & 0x1) << 4)
#       define NI_DP_MSE_ZERO_ENCODER                  (((x) & 0x1) << 8)

#define mmNI_DP_MSE_RATE_CNTL                            (0x7384 / 4)
#       define NI_DP_MSE_RATE_Y(x)                   (((x) & 0x3ffffff) << 0)
#       define NI_DP_MSE_RATE_X(x)                   (((x) & 0x3f) << 26)

#define mmNI_DP_MSE_RATE_UPDATE                          (0x738c / 4)

#define mmNI_DP_MSE_SAT0                                 (0x7390 / 4)
#       define NI_DP_MSE_SAT_SRC0(x)                   (((x) & 0x7) << 0)
#       define NI_DP_MSE_SAT_SLOT_COUNT0(x)            (((x) & 0x3f) << 8)
#       define NI_DP_MSE_SAT_SRC1(x)                   (((x) & 0x7) << 16)
#       define NI_DP_MSE_SAT_SLOT_COUNT1(x)            (((x) & 0x3f) << 24)

#define mmNI_DP_MSE_SAT1                                 (0x7394 / 4)

#define mmNI_DP_MSE_SAT2                                 (0x7398 / 4)

#define mmNI_DP_MSE_SAT_UPDATE                           (0x739c / 4)

#define mmNI_DIG_BE_CNTL                                 (0x7140 / 4)
#       define NI_DIG_FE_SOURCE_SELECT(x)              (((x) & 0x7f) << 8)
#       define NI_DIG_FE_DIG_MODE(x)                   (((x) & 0x7) << 16)
#       define NI_DIG_MODE_DP_SST                      0
#       define NI_DIG_MODE_LVDS                        1
#       define NI_DIG_MODE_TMDS_DVI                    2
#       define NI_DIG_MODE_TMDS_HDMI                   3
#       define NI_DIG_MODE_DP_MST                      5
#       define NI_DIG_HPD_SELECT(x)                    (((x) & 0x7) << 28)

#define mmNI_DIG_FE_CNTL                                 (0x7000 / 4)
#       define NI_DIG_SOURCE_SELECT(x)                 (((x) & 0x3) << 0)
#       define NI_DIG_STEREOSYNC_SELECT(x)             (((x) & 0x3) << 4)
#       define NI_DIG_STEREOSYNC_GATE_EN(x)            (((x) & 0x1) << 8)
#       define NI_DIG_DUAL_LINK_ENABLE(x)              (((x) & 0x1) << 16)
#       define NI_DIG_SWAP(x)                          (((x) & 0x1) << 18)
#       define NI_DIG_SYMCLK_FE_ON                     (0x1 << 24)

#define DMA_PACKET(cmd, t, s, n)	((((cmd) & 0xF) << 28) |	\
					 (((t) & 0x1) << 23) |		\
					 (((s) & 0x1) << 22) |		\
					 (((n) & 0xFFFFF) << 0))

#define SI_DMA_PACKET(cmd, b, t, s, n)	((((cmd) & 0xF) << 28) |	\
					 (((b) & 0x1) << 26) |		\
					 (((t) & 0x1) << 23) |		\
					 (((s) & 0x1) << 22) |		\
					 (((n) & 0xFFFFF) << 0))

#define DMA_IB_PACKET(cmd, vmid, n)	((((cmd) & 0xF) << 28) |	\
					 (((vmid) & 0xF) << 20) |	\
					 (((n) & 0xFFFFF) << 0))

#define DMA_PTE_PDE_PACKET(n)		((2 << 28) |			\
					 (1 << 26) |			\
					 (1 << 21) |			\
					 (((n) & 0xFFFFF) << 0))

/* async DMA Packet types */
#define	DMA_PACKET_WRITE				  0x2
#define	DMA_PACKET_COPY					  0x3
#define	DMA_PACKET_INDIRECT_BUFFER			  0x4
#define	DMA_PACKET_SEMAPHORE				  0x5
#define	DMA_PACKET_FENCE				  0x6
#define	DMA_PACKET_TRAP					  0x7
#define	DMA_PACKET_SRBM_WRITE				  0x9
#define	DMA_PACKET_CONSTANT_FILL			  0xd
#define	DMA_PACKET_POLL_REG_MEM				  0xe
#define	DMA_PACKET_NOP					  0xf

#endif
