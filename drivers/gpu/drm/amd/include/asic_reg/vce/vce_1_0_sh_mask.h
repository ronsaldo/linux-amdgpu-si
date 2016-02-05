#ifndef __VCE_1_0_SH_MASK_H__
#define __VCE_1_0_SH_MASK_H__

#define VCE_VCPU_CNTL__VCE_CLK_EN_MASK                                        0x00000001
#define VCE_VCPU_CNTL__VCE_CLK_EN__SHIFT                                      0
#define VCE_SOFT_RESET__VCE_ECPU_SOFT_RESET_MASK                              0x00000001
#define VCE_SOFT_RESET__VCE_ECPU_SOFT_RESET__SHIFT                            0
#define VCE_SOFT_RESET__VCE_FME_SOFT_RESET_MASK                               0x00000004
#define VCE_SOFT_RESET__VCE_FME_SOFT_RESET__SHIFT                             2
#define VCE_CLOCK_GATING_A__CGC_DYN_CLOCK_MODE_MASK                           0x00010000
#define VCE_CLOCK_GATING_A__CGC_DYN_CLOCK_MODE__SHIFT                         16
#define VCE_FW_REG_STATUS__VCE_FW_REG_STATUS_DONE_MASK                        0x00000800
#define VCE_FW_REG_STATUS__VCE_FW_REG_STATUS_DONE__SHIFT                      11
#define VCE_FW_REG_STATUS__VCE_FW_REG_STATUS_BUSY_MASK                        0x00000001
#define VCE_FW_REG_STATUS__VCE_FW_REG_STATUS_BUSY__SHIFT                      0
#define VCE_FW_REG_STATUS__VCE_FW_REG_STATUS_PASS_MASK                        0x00000008
#define VCE_FW_REG_STATUS__VCE_FW_REG_STATUS_PASS__SHIFT                      3

#endif /*__VCE_1_0_SH_MASK_H__*/
