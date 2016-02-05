#ifndef __VCE_1_0_D_H__
#define __VCE_1_0_D_H__

#define mmVCE_STATUS                                                            0x8001
#define mmVCE_VCPU_CNTL                                                         0x8005
#define mmVCE_VCPU_CACHE_OFFSET0                                                0x8009
#define mmVCE_VCPU_CACHE_SIZE0                                                  0x800a
#define mmVCE_VCPU_CACHE_OFFSET1                                                0x800b
#define mmVCE_VCPU_CACHE_SIZE1                                                  0x800c
#define mmVCE_VCPU_CACHE_OFFSET2                                                0x800d
#define mmVCE_VCPU_CACHE_SIZE2                                                  0x800e
#define mmVCE_VCPU_SCRATCH7                                                     0x8037
#define mmVCE_SOFT_RESET                                                        0x8048
#define mmVCE_RB_BASE_LO2                                                       0x805b
#define mmVCE_RB_BASE_HI2                                                       0x805c
#define mmVCE_RB_SIZE2                                                          0x805d
#define mmVCE_RB_RPTR2                                                          0x805e
#define mmVCE_RB_WPTR2                                                          0x805f
#define mmVCE_RB_BASE_LO                                                        0x8060
#define mmVCE_RB_BASE_HI                                                        0x8061
#define mmVCE_RB_SIZE                                                           0x8062
#define mmVCE_RB_RPTR                                                           0x8063
#define mmVCE_RB_WPTR                                                           0x8064
#define mmVCE_CLOCK_GATING_A                                                    0x80be
#define mmVCE_CLOCK_GATING_B                                                    0x80bf
#define mmVCE_UENC_CLOCK_GATING                                                 0x816f
#define mmVCE_UENC_REG_CLOCK_GATING                                             0x8170
#define mmVCE_FW_REG_STATUS                                                     0x8384
#define mmVCE_LMI_FW_START_KEYSEL                                               0x8386
#define mmVCE_LMI_FW_PERIODIC_CTRL                                              0x8388
#define mmVCE_LMI_CTRL2                                                         0x839d
#define mmVCE_LMI_CTRL                                                          0x83a6
#define mmVCE_LMI_VM_CTRL                                                       0x83a8
#define mmVCE_LMI_SWAP_CNTL                                                     0x83ad
#define mmVCE_LMI_SWAP_CNTL1                                                    0x83ae
#define mmVCE_LMI_CACHE_CTRL                                                    0x83bd
#define mmVCE_CMD_NO_OP                                                         0x0
#define mmVCE_CMD_TRAP                                                          0x1

#endif /*__VCE_1_0_D_H__*/
