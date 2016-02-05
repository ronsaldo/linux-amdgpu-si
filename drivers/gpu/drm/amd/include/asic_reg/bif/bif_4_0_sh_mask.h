#ifndef __BIF_4_0_SH_MASK_H__
#define __BIF_4_0_SH_MASK_H__

#define INTERRUPT_CNTL__IH_DUMMY_RD_OVERRIDE_MASK                             0x00000001
#define INTERRUPT_CNTL__IH_DUMMY_RD_OVERRIDE__SHIFT                           0
#define INTERRUPT_CNTL__GEN_IH_INT_EN_MASK                                    0x00000100
#define INTERRUPT_CNTL__GEN_IH_INT_EN__SHIFT                                  8
#define INTERRUPT_CNTL__IH_DUMMY_RD_EN_MASK                                   0x00000002
#define INTERRUPT_CNTL__IH_DUMMY_RD_EN__SHIFT                                 1
#define INTERRUPT_CNTL__IH_REQ_NONSNOOP_EN_MASK                               0x00000008
#define INTERRUPT_CNTL__IH_REQ_NONSNOOP_EN__SHIFT                             3
#define BIF_FB_EN__FB_WRITE_EN_MASK                                           0x00000002
#define BIF_FB_EN__FB_WRITE_EN__SHIFT                                         1
#define BIF_FB_EN__FB_READ_EN_MASK                                            0x00000001
#define BIF_FB_EN__FB_READ_EN__SHIFT                                          0
#define INT_MASK__VBLANK_INT_MASK                                             (1 << 0)
#define INT_MASK__VBLANK_INT__SHIFT                                           0
#define INT_MASK__VLINE_INT_MASK                                              (1 << 4)
#define INT_MASK__VLINE_INT__SHIFT                                            4
#define GRPH_INT_STATUS__GRPH_PFLIP_INT_OCCURRED_MASK                         0x00000001
#define GRPH_INT_STATUS__GRPH_PFLIP_INT_OCCURRED__SHIFT                       0
#define GRPH_INT_STATUS__GRPH_PFLIP_INT_CLEAR_MASK                            0x00000100
#define GRPH_INT_STATUS__GRPH_PFLIP_INT_CLEAR__SHIFT                          8
#define GRPH_INT_CONTROL__GRPH_PFLIP_INT_TYPE_MASK                            0x00000100
#define GRPH_INT_CONTROL__GRPH_PFLIP_INT_TYPE__SHIFT                          8
#define GRPH_INT_CONTROL__GRPH_PFLIP_INT_MASK                                 (1 << 0)
#define GRPH_INT_CONTROL__GRPH_PFLIP_INT__SHIFT                               0
#define PB0_PIF_CNTL__LS2_EXIT_TIME_MASK                                      (0x7 << 17)
#define PB0_PIF_CNTL__LS2_EXIT_TIME__SHIFT                                    17
#define PB0_PIF_PAIRING__MULTI_PIF_MASK                                       0x02000000
#define PB0_PIF_PAIRING__MULTI_PIF__SHIFT                                     25
#define PB0_PIF_PWRDOWN_0__PLL_RAMP_UP_TIME_0_MASK                            (0x7 << 24)
#define PB0_PIF_PWRDOWN_0__PLL_RAMP_UP_TIME_0__SHIFT                          24
#define PB0_PIF_PWRDOWN_0__PLL_POWER_STATE_IN_OFF_0_MASK                      (0x7 << 10)
#define PB0_PIF_PWRDOWN_0__PLL_POWER_STATE_IN_OFF_0__SHIFT                    10
#define PB0_PIF_PWRDOWN_0__PLL_POWER_STATE_IN_TXS2_0_MASK                     (0x7 << 7)
#define PB0_PIF_PWRDOWN_0__PLL_POWER_STATE_IN_TXS2_0__SHIFT                   7
#define PB0_PIF_PWRDOWN_1__PLL_POWER_STATE_IN_OFF_1_MASK                      (0x7 << 10)
#define PB0_PIF_PWRDOWN_1__PLL_POWER_STATE_IN_OFF_1__SHIFT                    10
#define PB0_PIF_PWRDOWN_1__PLL_RAMP_UP_TIME_1_MASK                            (0x7 << 24)
#define PB0_PIF_PWRDOWN_1__PLL_RAMP_UP_TIME_1__SHIFT                          24
#define PB0_PIF_PWRDOWN_1__PLL_POWER_STATE_IN_TXS2_1_MASK                     (0x7 << 7)
#define PB0_PIF_PWRDOWN_1__PLL_POWER_STATE_IN_TXS2_1__SHIFT                   7
#define PB0_PIF_PWRDOWN_2__PLL_POWER_STATE_IN_TXS2_2_MASK                     (0x7 << 7)
#define PB0_PIF_PWRDOWN_2__PLL_POWER_STATE_IN_TXS2_2__SHIFT                   7
#define PB0_PIF_PWRDOWN_2__PLL_RAMP_UP_TIME_2_MASK                            (0x7 << 24)
#define PB0_PIF_PWRDOWN_2__PLL_RAMP_UP_TIME_2__SHIFT                          24
#define PB0_PIF_PWRDOWN_2__PLL_POWER_STATE_IN_OFF_2_MASK                      (0x7 << 10)
#define PB0_PIF_PWRDOWN_2__PLL_POWER_STATE_IN_OFF_2__SHIFT                    10
#define PB0_PIF_PWRDOWN_3__PLL_POWER_STATE_IN_TXS2_3_MASK                     (0x7 << 7)
#define PB0_PIF_PWRDOWN_3__PLL_POWER_STATE_IN_TXS2_3__SHIFT                   7
#define PB0_PIF_PWRDOWN_3__PLL_POWER_STATE_IN_OFF_3_MASK                      (0x7 << 10)
#define PB0_PIF_PWRDOWN_3__PLL_POWER_STATE_IN_OFF_3__SHIFT                    10
#define PB0_PIF_PWRDOWN_3__PLL_RAMP_UP_TIME_3_MASK                            (0x7 << 24)
#define PB0_PIF_PWRDOWN_3__PLL_RAMP_UP_TIME_3__SHIFT                          24
#define PCIE_CNTL2__SLV_MEM_AGGRESSIVE_LS_EN_MASK                             0x00020000
#define PCIE_CNTL2__SLV_MEM_AGGRESSIVE_LS_EN__SHIFT                           17
#define PCIE_CNTL2__SLV_MEM_LS_EN_MASK                                        0x00010000
#define PCIE_CNTL2__SLV_MEM_LS_EN__SHIFT                                      16
#define PCIE_CNTL2__MST_MEM_LS_EN_MASK                                        0x00040000
#define PCIE_CNTL2__MST_MEM_LS_EN__SHIFT                                      18
#define PCIE_CNTL2__REPLAY_MEM_LS_EN_MASK                                     0x00080000
#define PCIE_CNTL2__REPLAY_MEM_LS_EN__SHIFT                                   19
#define PCIE_LC_STATUS1__LC_REVERSE_RCVR_MASK                                 0x00000001
#define PCIE_LC_STATUS1__LC_REVERSE_RCVR__SHIFT                               0
#define PCIE_LC_STATUS1__LC_DETECTED_LINK_WIDTH_MASK                          (0x7 << 5)
#define PCIE_LC_STATUS1__LC_DETECTED_LINK_WIDTH__SHIFT                        5
#define PCIE_LC_STATUS1__LC_OPERATING_LINK_WIDTH_MASK                         (0x7 << 2)
#define PCIE_LC_STATUS1__LC_OPERATING_LINK_WIDTH__SHIFT                       2
#define PCIE_LC_STATUS1__LC_REVERSE_XMIT_MASK                                 0x00000002
#define PCIE_LC_STATUS1__LC_REVERSE_XMIT__SHIFT                               1
#define PCIE_P_CNTL__P_IGNORE_EDB_ERR_MASK                                    0x00000040
#define PCIE_P_CNTL__P_IGNORE_EDB_ERR__SHIFT                                  6
#define PCIE_LC_CNTL__LC_L1_INACTIVITY_MASK                                   (0xf << 12)
#define PCIE_LC_CNTL__LC_L1_INACTIVITY__SHIFT                                 12
#define PCIE_LC_CNTL__LC_PMI_TO_L1_DIS_MASK                                   0x00010000
#define PCIE_LC_CNTL__LC_PMI_TO_L1_DIS__SHIFT                                 16
#define PCIE_LC_CNTL__LC_ASPM_TO_L1_DIS_MASK                                  0x01000000
#define PCIE_LC_CNTL__LC_ASPM_TO_L1_DIS__SHIFT                                24
#define PCIE_LC_CNTL__LC_L0S_INACTIVITY_MASK                                  (0xf << 8)
#define PCIE_LC_CNTL__LC_L0S_INACTIVITY__SHIFT                                8
#define PCIE_LC_LINK_WIDTH_CNTL__LC_RECONFIG_NOW_MASK                         0x00000100
#define PCIE_LC_LINK_WIDTH_CNTL__LC_RECONFIG_NOW__SHIFT                       8
#define PCIE_LC_LINK_WIDTH_CNTL__LC_UPCONFIGURE_SUPPORT_MASK                  0x00001000
#define PCIE_LC_LINK_WIDTH_CNTL__LC_UPCONFIGURE_SUPPORT__SHIFT                12
#define PCIE_LC_LINK_WIDTH_CNTL__LC_LINK_WIDTH_RD_MASK                        0x70
#define PCIE_LC_LINK_WIDTH_CNTL__LC_LINK_WIDTH_RD__SHIFT                      4
#define PCIE_LC_LINK_WIDTH_CNTL__LC_UPCONFIGURE_DIS_MASK                      0x00002000
#define PCIE_LC_LINK_WIDTH_CNTL__LC_UPCONFIGURE_DIS__SHIFT                    13
#define PCIE_LC_LINK_WIDTH_CNTL__LC_SHORT_RECONFIG_EN_MASK                    0x00000800
#define PCIE_LC_LINK_WIDTH_CNTL__LC_SHORT_RECONFIG_EN__SHIFT                  11
#define PCIE_LC_LINK_WIDTH_CNTL__LC_RENEGOTIATE_EN_MASK                       0x00000400
#define PCIE_LC_LINK_WIDTH_CNTL__LC_RENEGOTIATE_EN__SHIFT                     10
#define PCIE_LC_LINK_WIDTH_CNTL__LC_RECONFIG_ARC_MISSING_ESCAPE_MASK          0x00000080
#define PCIE_LC_LINK_WIDTH_CNTL__LC_RECONFIG_ARC_MISSING_ESCAPE__SHIFT        7
#define PCIE_LC_LINK_WIDTH_CNTL__LC_LINK_WIDTH_MASK                           0x7
#define PCIE_LC_LINK_WIDTH_CNTL__LC_LINK_WIDTH__SHIFT                         0
#define PCIE_LC_LINK_WIDTH_CNTL__LC_DYN_LANES_PWR_STATE_MASK                  (0x3 << 21)
#define PCIE_LC_LINK_WIDTH_CNTL__LC_DYN_LANES_PWR_STATE__SHIFT                21
#define PCIE_LC_LINK_WIDTH_CNTL__LC_RENEGOTIATION_SUPPORT_MASK                0x00000200
#define PCIE_LC_LINK_WIDTH_CNTL__LC_RENEGOTIATION_SUPPORT__SHIFT              9
#define PCIE_LC_N_FTS_CNTL__LC_XMIT_N_FTS_OVERRIDE_EN_MASK                    0x00000100
#define PCIE_LC_N_FTS_CNTL__LC_XMIT_N_FTS_OVERRIDE_EN__SHIFT                  8
#define PCIE_LC_N_FTS_CNTL__LC_N_FTS_MASK                                     (0xff << 24)
#define PCIE_LC_N_FTS_CNTL__LC_N_FTS__SHIFT                                   24
#define PCIE_LC_N_FTS_CNTL__LC_XMIT_N_FTS_MASK                                (0xff << 0)
#define PCIE_LC_N_FTS_CNTL__LC_XMIT_N_FTS__SHIFT                              0
#define PCIE_LC_SPEED_CNTL__LC_GEN3_EN_STRAP_MASK                             0x00000002
#define PCIE_LC_SPEED_CNTL__LC_GEN3_EN_STRAP__SHIFT                           1
#define PCIE_LC_SPEED_CNTL__LC_FORCE_DIS_HW_SPEED_CHANGE_MASK                 0x00000100
#define PCIE_LC_SPEED_CNTL__LC_FORCE_DIS_HW_SPEED_CHANGE__SHIFT               8
#define PCIE_LC_SPEED_CNTL__LC_CLR_FAILED_SPD_CHANGE_CNT_MASK                 0x00010000
#define PCIE_LC_SPEED_CNTL__LC_CLR_FAILED_SPD_CHANGE_CNT__SHIFT               16
#define PCIE_LC_SPEED_CNTL__LC_OTHER_SIDE_SUPPORTS_GEN3_MASK                  0x00200000
#define PCIE_LC_SPEED_CNTL__LC_OTHER_SIDE_SUPPORTS_GEN3__SHIFT                21
#define PCIE_LC_SPEED_CNTL__LC_CURRENT_DATA_RATE_MASK                         (0x3 << 13) 
#define PCIE_LC_SPEED_CNTL__LC_CURRENT_DATA_RATE__SHIFT                       13
#define PCIE_LC_SPEED_CNTL__LC_INITIATE_LINK_SPEED_CHANGE_MASK                0x00000200
#define PCIE_LC_SPEED_CNTL__LC_INITIATE_LINK_SPEED_CHANGE__SHIFT              9
#define PCIE_LC_SPEED_CNTL__LC_OTHER_SIDE_EVER_SENT_GEN2_MASK                 0x00040000
#define PCIE_LC_SPEED_CNTL__LC_OTHER_SIDE_EVER_SENT_GEN2__SHIFT               18
#define PCIE_LC_SPEED_CNTL__LC_FORCE_DIS_SW_SPEED_CHANGE_MASK                 0x00000040
#define PCIE_LC_SPEED_CNTL__LC_FORCE_DIS_SW_SPEED_CHANGE__SHIFT               6
#define PCIE_LC_SPEED_CNTL__LC_FORCE_EN_HW_SPEED_CHANGE_MASK                  0x00000080
#define PCIE_LC_SPEED_CNTL__LC_FORCE_EN_HW_SPEED_CHANGE__SHIFT                7
#define PCIE_LC_SPEED_CNTL__LC_FORCE_EN_SW_SPEED_CHANGE_MASK                  0x00000020
#define PCIE_LC_SPEED_CNTL__LC_FORCE_EN_SW_SPEED_CHANGE__SHIFT                5
#define PCIE_LC_SPEED_CNTL__LC_OTHER_SIDE_EVER_SENT_GEN3_MASK                 0x00100000
#define PCIE_LC_SPEED_CNTL__LC_OTHER_SIDE_EVER_SENT_GEN3__SHIFT               20
#define PCIE_LC_SPEED_CNTL__LC_TARGET_LINK_SPEED_OVERRIDE_MASK                (0x3 << 3)
#define PCIE_LC_SPEED_CNTL__LC_TARGET_LINK_SPEED_OVERRIDE__SHIFT              3
#define PCIE_LC_SPEED_CNTL__LC_TARGET_LINK_SPEED_OVERRIDE_EN_MASK             0x00000004
#define PCIE_LC_SPEED_CNTL__LC_TARGET_LINK_SPEED_OVERRIDE_EN__SHIFT           2
#define PCIE_LC_SPEED_CNTL__LC_OTHER_SIDE_SUPPORTS_GEN2_MASK                  0x00080000
#define PCIE_LC_SPEED_CNTL__LC_OTHER_SIDE_SUPPORTS_GEN2__SHIFT                19
#define PCIE_LC_SPEED_CNTL__LC_GEN2_EN_STRAP_MASK                             0x00000001
#define PCIE_LC_SPEED_CNTL__LC_GEN2_EN_STRAP__SHIFT                           0
#define PCIE_LC_SPEED_CNTL__LC_SPEED_CHANGE_ATTEMPTS_ALLOWED_MASK             (0x3 << 10)
#define PCIE_LC_SPEED_CNTL__LC_SPEED_CHANGE_ATTEMPTS_ALLOWED__SHIFT           10
#define PCIE_LC_CNTL2__LC_ALLOW_PDWN_IN_L23_MASK                              0x00040000
#define PCIE_LC_CNTL2__LC_ALLOW_PDWN_IN_L23__SHIFT                            18
#define PCIE_LC_CNTL2__LC_ALLOW_PDWN_IN_L1_MASK                               0x00020000
#define PCIE_LC_CNTL2__LC_ALLOW_PDWN_IN_L1__SHIFT                             17
#define PCIE_LC_CNTL3__LC_GO_TO_RECOVERY_MASK                                 0x40000000
#define PCIE_LC_CNTL3__LC_GO_TO_RECOVERY__SHIFT                               30
#define PCIE_LC_CNTL4__LC_REDO_EQ_MASK                                        0x00000020
#define PCIE_LC_CNTL4__LC_REDO_EQ__SHIFT                                      5
#define PCIE_LC_CNTL4__LC_SET_QUIESCE_MASK                                    0x00002000
#define PCIE_LC_CNTL4__LC_SET_QUIESCE__SHIFT                                  13
#define ROM_CNTL__SCK_PRESCALE_CRYSTAL_CLK_MASK                               (0xf << 28)
#define ROM_CNTL__SCK_PRESCALE_CRYSTAL_CLK__SHIFT                             28
#define ROM_CNTL__SCK_OVERWRITE_MASK                                          0x00000002
#define ROM_CNTL__SCK_OVERWRITE__SHIFT                                        1
#define BUS_CNTL__VGA_COHE_SPEC_TIMER_DIS_MASK                                0x00000200
#define BUS_CNTL__VGA_COHE_SPEC_TIMER_DIS__SHIFT                              9
#define BUS_CNTL__BIOS_ROM_DIS_MASK                                           0x00000002
#define BUS_CNTL__BIOS_ROM_DIS__SHIFT                                         1

#endif /*__BIF_4_0_SH_MASK_H__*/
