#ifndef __SMU_6_0_SH_MASK_H__
#define __SMU_6_0_SH_MASK_H__

#define SMC_IND_ACCESS_CNTL__AUTO_INCREMENT_IND_0_MASK                        0x00000001
#define SMC_IND_ACCESS_CNTL__AUTO_INCREMENT_IND_0__SHIFT                      0
#define SMC_SYSCON_RESET_CNTL__RST_REG_MASK                                   0x00000001
#define SMC_SYSCON_RESET_CNTL__RST_REG__SHIFT                                 0
#define SMC_SYSCON_CLOCK_CNTL_0__CKEN_MASK                                    0x01000000
#define SMC_SYSCON_CLOCK_CNTL_0__CKEN__SHIFT                                  24
#define SMC_SYSCON_CLOCK_CNTL_0__CK_DISABLE_MASK                              0x00000001
#define SMC_SYSCON_CLOCK_CNTL_0__CK_DISABLE__SHIFT                            0
#define CG_SPLL_FUNC_CNTL__SPLL_PDIV_A_MASK                                   (0x7f << 20)
#define CG_SPLL_FUNC_CNTL__SPLL_PDIV_A__SHIFT                                 20
#define CG_SPLL_FUNC_CNTL__SPLL_RESET_MASK                                    0x00000001
#define CG_SPLL_FUNC_CNTL__SPLL_RESET__SHIFT                                  0
#define CG_SPLL_FUNC_CNTL__SPLL_BYPASS_EN_MASK                                0x00000008
#define CG_SPLL_FUNC_CNTL__SPLL_BYPASS_EN__SHIFT                              3
#define CG_SPLL_FUNC_CNTL__SPLL_REF_DIV_MASK                                  (0x3f << 4)
#define CG_SPLL_FUNC_CNTL__SPLL_REF_DIV__SHIFT                                4
#define CG_SPLL_FUNC_CNTL__SPLL_SLEEP_MASK                                    0x00000002
#define CG_SPLL_FUNC_CNTL__SPLL_SLEEP__SHIFT                                  1
#define CG_SPLL_FUNC_CNTL_2__SCLK_MUX_SEL_MASK                                (0x1ff << 0)
#define CG_SPLL_FUNC_CNTL_2__SCLK_MUX_SEL__SHIFT                              0
#define CG_SPLL_FUNC_CNTL_2__SCLK_MUX_UPDATE_MASK                             0x04000000
#define CG_SPLL_FUNC_CNTL_2__SCLK_MUX_UPDATE__SHIFT                           26
#define CG_SPLL_FUNC_CNTL_2__SPLL_CTLREQ_CHG_MASK                             0x00800000
#define CG_SPLL_FUNC_CNTL_2__SPLL_CTLREQ_CHG__SHIFT                           23
#define CG_SPLL_FUNC_CNTL_3__SPLL_DITHEN_MASK                                 0x10000000
#define CG_SPLL_FUNC_CNTL_3__SPLL_DITHEN__SHIFT                               28
#define CG_SPLL_FUNC_CNTL_3__SPLL_FB_DIV_MASK                                 (0x3ffffff << 0)
#define CG_SPLL_FUNC_CNTL_3__SPLL_FB_DIV__SHIFT                               0
#define SPLL_STATUS__SPLL_CHG_STATUS_MASK                                     0x00000002
#define SPLL_STATUS__SPLL_CHG_STATUS__SHIFT                                   1
#define SPLL_CNTL_MODE__SPLL_SW_DIR_CONTROL_MASK                              0x00000001
#define SPLL_CNTL_MODE__SPLL_SW_DIR_CONTROL__SHIFT                            0
#define SPLL_CNTL_MODE__SPLL_REFCLK_SEL_MASK                                  (3 << 26)
#define SPLL_CNTL_MODE__SPLL_REFCLK_SEL__SHIFT                                26
#define CG_SPLL_SPREAD_SPECTRUM__SSEN_MASK                                    0x00000001
#define CG_SPLL_SPREAD_SPECTRUM__SSEN__SHIFT                                  0
#define CG_SPLL_SPREAD_SPECTRUM__CLK_S_MASK                                   (0xfff << 4)
#define CG_SPLL_SPREAD_SPECTRUM__CLK_S__SHIFT                                 4
#define CG_SPLL_SPREAD_SPECTRUM_2__CLK_V_MASK                                 (0x3ffffff << 0)
#define CG_SPLL_SPREAD_SPECTRUM_2__CLK_V__SHIFT                               0
#define CG_SPLL_AUTOSCALE_CNTL__AUTOSCALE_ON_SS_CLEAR_MASK                    0x00000200
#define CG_SPLL_AUTOSCALE_CNTL__AUTOSCALE_ON_SS_CLEAR__SHIFT                  9
#define CG_UPLL_FUNC_CNTL__UPLL_BYPASS_EN_MASK                                0x00000004
#define CG_UPLL_FUNC_CNTL__UPLL_BYPASS_EN__SHIFT                              2
#define CG_UPLL_FUNC_CNTL__UPLL_RESET_MASK                                    0x00000001
#define CG_UPLL_FUNC_CNTL__UPLL_RESET__SHIFT                                  0
#define CG_UPLL_FUNC_CNTL__UPLL_REF_DIV_MASK                                  0x003F0000
#define CG_UPLL_FUNC_CNTL__UPLL_REF_DIV__SHIFT                                16
#define CG_UPLL_FUNC_CNTL__UPLL_VCO_MODE_MASK                                 0x00000600
#define CG_UPLL_FUNC_CNTL__UPLL_VCO_MODE__SHIFT                               9
#define CG_UPLL_FUNC_CNTL__UPLL_CTLACK_MASK                                   0x40000000
#define CG_UPLL_FUNC_CNTL__UPLL_CTLACK__SHIFT                                 30
#define CG_UPLL_FUNC_CNTL__UPLL_CTLREQ_MASK                                   0x00000008
#define CG_UPLL_FUNC_CNTL__UPLL_CTLREQ__SHIFT                                 3
#define CG_UPLL_FUNC_CNTL__UPLL_CTLACK2_MASK                                  0x80000000
#define CG_UPLL_FUNC_CNTL__UPLL_CTLACK2__SHIFT                                31
#define CG_UPLL_FUNC_CNTL__UPLL_SLEEP_MASK                                    0x00000002
#define CG_UPLL_FUNC_CNTL__UPLL_SLEEP__SHIFT                                  1
#define CG_UPLL_FUNC_CNTL_2__VCLK_SRC_SEL_MASK                                0x01F00000
#define CG_UPLL_FUNC_CNTL_2__VCLK_SRC_SEL__SHIFT                              20
#define CG_UPLL_FUNC_CNTL_2__DCLK_SRC_SEL_MASK                                0x3E000000
#define CG_UPLL_FUNC_CNTL_2__DCLK_SRC_SEL__SHIFT                              25
#define CG_UPLL_FUNC_CNTL_2__UPLL_PDIV_A_MASK                                 0x0000007F
#define CG_UPLL_FUNC_CNTL_2__UPLL_PDIV_A__SHIFT                               0
#define CG_UPLL_FUNC_CNTL_2__UPLL_PDIV_B_MASK                                 0x00007F00
#define CG_UPLL_FUNC_CNTL_2__UPLL_PDIV_B__SHIFT                               8
#define CG_UPLL_FUNC_CNTL_3__UPLL_FB_DIV_MASK                                 0x01FFFFFF
#define CG_UPLL_FUNC_CNTL_3__UPLL_FB_DIV__SHIFT                               0
#define CG_UPLL_FUNC_CNTL_4__UPLL_SPARE_ISPARE9_MASK                          0x00020000
#define CG_UPLL_FUNC_CNTL_4__UPLL_SPARE_ISPARE9__SHIFT                        16
#define CG_UPLL_FUNC_CNTL_5__RESET_ANTI_MUX_MASK                              0x00000200
#define CG_UPLL_FUNC_CNTL_5__RESET_ANTI_MUX__SHIFT                            9
#define CG_UPLL_SPREAD_SPECTRUM__SSEN_MASK                                    0x00000001
#define CG_UPLL_SPREAD_SPECTRUM__SSEN__SHIFT                                  0
#define MPLL_BYPASSCLK_SEL__MPLL_CLKOUT_SEL_MASK                              0xFF00
#define MPLL_BYPASSCLK_SEL__MPLL_CLKOUT_SEL__SHIFT                            8
#define CG_CLKPIN_CNTL__BCLK_AS_XCLK_MASK                                     0x00000004
#define CG_CLKPIN_CNTL__BCLK_AS_XCLK__SHIFT                                   2
#define CG_CLKPIN_CNTL__XTALIN_DIVIDE_MASK                                    0x00000002
#define CG_CLKPIN_CNTL__XTALIN_DIVIDE__SHIFT                                  1
#define CG_CLKPIN_CNTL_2__FORCE_BIF_REFCLK_EN_MASK                            0x00000008
#define CG_CLKPIN_CNTL_2__FORCE_BIF_REFCLK_EN__SHIFT                          3
#define CG_CLKPIN_CNTL_2__MUX_TCLK_TO_XCLK_MASK                               0x00000100
#define CG_CLKPIN_CNTL_2__MUX_TCLK_TO_XCLK__SHIFT                             8
#define THM_CLK_CNTL__TMON_CLK_SEL_MASK                                       0xFF00
#define THM_CLK_CNTL__TMON_CLK_SEL__SHIFT                                     8
#define THM_CLK_CNTL__CMON_CLK_SEL_MASK                                       0xFF
#define THM_CLK_CNTL__CMON_CLK_SEL__SHIFT                                     0
#define MISC_CLK_CNTL__DEEP_SLEEP_CLK_SEL_MASK                                0xFF
#define MISC_CLK_CNTL__DEEP_SLEEP_CLK_SEL__SHIFT                              0
#define MISC_CLK_CNTL__ZCLK_SEL_MASK                                          0xFF00
#define MISC_CLK_CNTL__ZCLK_SEL__SHIFT                                        8
#define CG_THERMAL_CTRL__DPM_EVENT_SRC_MASK                                   (7 << 0)
#define CG_THERMAL_CTRL__DPM_EVENT_SRC__SHIFT                                 0
#define CG_THERMAL_CTRL__DIG_THERM_DPM_MASK                                   0x003FC000
#define CG_THERMAL_CTRL__DIG_THERM_DPM__SHIFT                                 14
#define CG_THERMAL_STATUS__FDO_PWM_DUTY_MASK                                  (0xff << 9)
#define CG_THERMAL_STATUS__FDO_PWM_DUTY__SHIFT                                9
#define CG_THERMAL_INT__DIG_THERM_INTH_MASK                                   0x0000FF00
#define CG_THERMAL_INT__DIG_THERM_INTH__SHIFT                                 8
#define CG_THERMAL_INT__THERM_INT_MASK_LOW_MASK                               0x02000000
#define CG_THERMAL_INT__THERM_INT_MASK_LOW__SHIFT                             25
#define CG_THERMAL_INT__DIG_THERM_INTL_MASK                                   0x00FF0000
#define CG_THERMAL_INT__DIG_THERM_INTL__SHIFT                                 16
#define CG_THERMAL_INT__THERM_INT_MASK_HIGH_MASK                              0x01000000
#define CG_THERMAL_INT__THERM_INT_MASK_HIGH__SHIFT                            24
#define CG_MULT_THERMAL_CTRL__TEMP_SEL_MASK                                   (0xff << 20)
#define CG_MULT_THERMAL_CTRL__TEMP_SEL__SHIFT                                 20
#define CG_MULT_THERMAL_STATUS__ASIC_MAX_TEMP_MASK                            0x000001ff
#define CG_MULT_THERMAL_STATUS__ASIC_MAX_TEMP__SHIFT                          0
#define CG_MULT_THERMAL_STATUS__CTF_TEMP_MASK                                 0x0003fe00
#define CG_MULT_THERMAL_STATUS__CTF_TEMP__SHIFT                               9
#define CG_FDO_CTRL0__FDO_STATIC_DUTY_MASK                                    0x000000FF
#define CG_FDO_CTRL0__FDO_STATIC_DUTY__SHIFT                                  0
#define CG_FDO_CTRL1__FMAX_DUTY100_MASK                                       0x000000FF
#define CG_FDO_CTRL1__FMAX_DUTY100__SHIFT                                     0
#define CG_FDO_CTRL2__TACH_PWM_RESP_RATE_MASK                                 (0x7f << 25)
#define CG_FDO_CTRL2__TACH_PWM_RESP_RATE__SHIFT                               25
#define CG_FDO_CTRL2__FDO_PWM_MODE_MASK                                       (7 << 11)
#define CG_FDO_CTRL2__FDO_PWM_MODE__SHIFT                                     11
#define CG_FDO_CTRL2__TMIN_MASK                                               0x000000FF
#define CG_FDO_CTRL2__TMIN__SHIFT                                             0
#define CG_TACH_CTRL__EDGE_PER_REV_MASK                                       (0x7 << 0)
#define CG_TACH_CTRL__EDGE_PER_REV__SHIFT                                     0
#define CG_TACH_CTRL__TARGET_PERIOD_MASK                                      0xfffffff8
#define CG_TACH_CTRL__TARGET_PERIOD__SHIFT                                    3
#define CG_TACH_STATUS__TACH_PERIOD_MASK                                      0xffffffff
#define CG_TACH_STATUS__TACH_PERIOD__SHIFT                                    0
#define GENERAL_PWRMGT__THERMAL_PROTECTION_TYPE_MASK                          0x00000008
#define GENERAL_PWRMGT__THERMAL_PROTECTION_TYPE__SHIFT                        3
#define GENERAL_PWRMGT__THERMAL_PROTECTION_DIS_MASK                           0x00000004
#define GENERAL_PWRMGT__THERMAL_PROTECTION_DIS__SHIFT                         2
#define GENERAL_PWRMGT__VOLT_PWRMGT_EN_MASK                                   0x00000400
#define GENERAL_PWRMGT__VOLT_PWRMGT_EN__SHIFT                                 10
#define GENERAL_PWRMGT__GLOBAL_PWRMGT_EN_MASK                                 0x00000001
#define GENERAL_PWRMGT__GLOBAL_PWRMGT_EN__SHIFT                               0
#define GENERAL_PWRMGT__SW_SMIO_INDEX_MASK                                    (1 << 6)
#define GENERAL_PWRMGT__SW_SMIO_INDEX__SHIFT                                  6
#define GENERAL_PWRMGT__DYN_SPREAD_SPECTRUM_EN_MASK                           0x00800000
#define GENERAL_PWRMGT__DYN_SPREAD_SPECTRUM_EN__SHIFT                         23
#define GENERAL_PWRMGT__STATIC_PM_EN_MASK                                     0x00000002
#define GENERAL_PWRMGT__STATIC_PM_EN__SHIFT                                   1
#define SCLK_PWRMGT_CNTL__DYN_LIGHT_SLEEP_EN_MASK                             0x00004000
#define SCLK_PWRMGT_CNTL__DYN_LIGHT_SLEEP_EN__SHIFT                           14
#define SCLK_PWRMGT_CNTL__GFX_CLK_FORCE_OFF_MASK                              0x00000400
#define SCLK_PWRMGT_CNTL__GFX_CLK_FORCE_OFF__SHIFT                            10
#define SCLK_PWRMGT_CNTL__GFX_CLK_REQUEST_OFF_MASK                            0x00000200
#define SCLK_PWRMGT_CNTL__GFX_CLK_REQUEST_OFF__SHIFT                          9
#define SCLK_PWRMGT_CNTL__FIR_TREND_MODE_MASK                                 0x00000040
#define SCLK_PWRMGT_CNTL__FIR_TREND_MODE__SHIFT                               6
#define SCLK_PWRMGT_CNTL__GFX_CLK_OFF_ACPI_D2_MASK                            0x00001000
#define SCLK_PWRMGT_CNTL__GFX_CLK_OFF_ACPI_D2__SHIFT                          12
#define SCLK_PWRMGT_CNTL__GFX_CLK_OFF_ACPI_D3_MASK                            0x00002000
#define SCLK_PWRMGT_CNTL__GFX_CLK_OFF_ACPI_D3__SHIFT                          13
#define SCLK_PWRMGT_CNTL__SCLK_PWRMGT_OFF_MASK                                0x00000001
#define SCLK_PWRMGT_CNTL__SCLK_PWRMGT_OFF__SHIFT                              0
#define SCLK_PWRMGT_CNTL__DYN_GFX_CLK_OFF_EN_MASK                             0x00000080
#define SCLK_PWRMGT_CNTL__DYN_GFX_CLK_OFF_EN__SHIFT                           7
#define SCLK_PWRMGT_CNTL__GFX_CLK_OFF_ACPI_D1_MASK                            0x00000800
#define SCLK_PWRMGT_CNTL__GFX_CLK_OFF_ACPI_D1__SHIFT                          11
#define SCLK_PWRMGT_CNTL__FIR_FORCE_TREND_SEL_MASK                            0x00000020
#define SCLK_PWRMGT_CNTL__FIR_FORCE_TREND_SEL__SHIFT                          5
#define SCLK_PWRMGT_CNTL__SCLK_LOW_D1_MASK                                    0x00000002
#define SCLK_PWRMGT_CNTL__SCLK_LOW_D1__SHIFT                                  1
#define SCLK_PWRMGT_CNTL__GFX_CLK_FORCE_ON_MASK                               0x00000100
#define SCLK_PWRMGT_CNTL__GFX_CLK_FORCE_ON__SHIFT                             8
#define SCLK_PWRMGT_CNTL__FIR_RESET_MASK                                      0x00000010
#define SCLK_PWRMGT_CNTL__FIR_RESET__SHIFT                                    4
#define TARGET_AND_CURRENT_PROFILE_INDEX__CURRENT_STATE_INDEX_MASK            (0xf << 4)
#define TARGET_AND_CURRENT_PROFILE_INDEX__CURRENT_STATE_INDEX__SHIFT          4
#define CG_FFCT_0__UTC_0_MASK                                                 (0x3ff << 0)
#define CG_FFCT_0__UTC_0__SHIFT                                               0
#define CG_FFCT_0__DTC_0_MASK                                                 (0x3ff << 10)
#define CG_FFCT_0__DTC_0__SHIFT                                               10
#define CG_BSP__BSU_MASK                                                      (0xf << 16)
#define CG_BSP__BSU__SHIFT                                                    16
#define CG_BSP__BSP_MASK                                                      (0xffff << 0)
#define CG_BSP__BSP__SHIFT                                                    0
#define CG_AT__CG_R_MASK                                                      (0xffff << 0)
#define CG_AT__CG_R__SHIFT                                                    0
#define CG_AT__CG_L_MASK                                                      (0xffff << 16)
#define CG_AT__CG_L__SHIFT                                                    16
#define CG_GIT__CG_GICST_MASK                                                 (0xffff << 0)
#define CG_GIT__CG_GICST__SHIFT                                               0
#define CG_GIT__CG_GIPOT_MASK                                                 (0xffff << 16)
#define CG_GIT__CG_GIPOT__SHIFT                                               16
#define CG_SSP__SST_MASK                                                      (0xffff << 0)
#define CG_SSP__SST__SHIFT                                                    0
#define CG_SSP__SSTU_MASK                                                     (0xf << 16)
#define CG_SSP__SSTU__SHIFT                                                   16
#define CG_DISPLAY_GAP_CNTL__VBI_TIMER_COUNT_MASK                             (0x3fff << 4)
#define CG_DISPLAY_GAP_CNTL__VBI_TIMER_COUNT__SHIFT                           4
#define CG_DISPLAY_GAP_CNTL__DISP1_GAP_MASK                                   (3 << 0)
#define CG_DISPLAY_GAP_CNTL__DISP1_GAP__SHIFT                                 0
#define CG_DISPLAY_GAP_CNTL__DISP2_GAP_MASK                                   (3 << 2)
#define CG_DISPLAY_GAP_CNTL__DISP2_GAP__SHIFT                                 2
#define CG_DISPLAY_GAP_CNTL__DISP1_GAP_MCHG_MASK                              (3 << 24)
#define CG_DISPLAY_GAP_CNTL__DISP1_GAP_MCHG__SHIFT                            24
#define CG_DISPLAY_GAP_CNTL__DISP2_GAP_MCHG_MASK                              (3 << 26)
#define CG_DISPLAY_GAP_CNTL__DISP2_GAP_MCHG__SHIFT                            26
#define CG_DISPLAY_GAP_CNTL__VBI_TIMER_UNIT_MASK                              (7 << 20)
#define CG_DISPLAY_GAP_CNTL__VBI_TIMER_UNIT__SHIFT                            7
#define CG_CAC_CTRL__CAC_WINDOW_MASK                                          0x00ffffff
#define CG_CAC_CTRL__CAC_WINDOW__SHIFT                                        0
#define MPLL_CNTL_MODE__MPLL_MCLK_SEL_MASK                                    0x00000800
#define MPLL_CNTL_MODE__MPLL_MCLK_SEL__SHIFT                                  11
#define MPLL_FUNC_CNTL__BWCTRL_MASK                                           (0xff << 20)
#define MPLL_FUNC_CNTL__BWCTRL__SHIFT                                         20
#define MPLL_FUNC_CNTL_1__VCO_MODE_MASK                                       (3 << 0)
#define MPLL_FUNC_CNTL_1__VCO_MODE__SHIFT                                     0
#define MPLL_FUNC_CNTL_1__CLKFRAC_MASK                                        (0xfff << 4)
#define MPLL_FUNC_CNTL_1__CLKFRAC__SHIFT                                      4
#define MPLL_FUNC_CNTL_1__CLKF_MASK                                           (0xfff << 16)
#define MPLL_FUNC_CNTL_1__CLKF__SHIFT                                         16
#define MPLL_AD_FUNC_CNTL__YCLK_POST_DIV_MASK                                 (7 << 0)
#define MPLL_AD_FUNC_CNTL__YCLK_POST_DIV__SHIFT                               0
#define MPLL_DQ_FUNC_CNTL__YCLK_SEL_MASK                                      (1 << 4)
#define MPLL_DQ_FUNC_CNTL__YCLK_SEL__SHIFT                                    4
#define MPLL_SS1__CLKV_MASK                                                   (0x3ffffff << 0)
#define MPLL_SS1__CLKV__SHIFT                                                 0
#define MPLL_SS2__CLKS_MASK                                                   (0xfff << 0)
#define MPLL_SS2__CLKS__SHIFT                                                 0
#define CG_VCEPLL_FUNC_CNTL__VCEPLL_RESET_MASK                                0x00000001
#define CG_VCEPLL_FUNC_CNTL__VCEPLL_RESET__SHIFT                              0
#define CG_VCEPLL_FUNC_CNTL__VCEPLL_REF_DIV_MASK                              0x003F0000
#define CG_VCEPLL_FUNC_CNTL__VCEPLL_REF_DIV__SHIFT                            16
#define CG_VCEPLL_FUNC_CNTL__VCEPLL_VCO_MODE_MASK                             0x00000600
#define CG_VCEPLL_FUNC_CNTL__VCEPLL_VCO_MODE__SHIFT                           9
#define CG_VCEPLL_FUNC_CNTL__VCEPLL_CTLACK_MASK                               0x40000000
#define CG_VCEPLL_FUNC_CNTL__VCEPLL_CTLACK__SHIFT                             30
#define CG_VCEPLL_FUNC_CNTL__VCEPLL_CTLACK2_MASK                              0x80000000
#define CG_VCEPLL_FUNC_CNTL__VCEPLL_CTLACK2__SHIFT                            31
#define CG_VCEPLL_FUNC_CNTL__VCEPLL_SLEEP_MASK                                0x00000002
#define CG_VCEPLL_FUNC_CNTL__VCEPLL_SLEEP__SHIFT                              1
#define CG_VCEPLL_FUNC_CNTL__VCEPLL_BYPASS_EN_MASK                            0x00000004
#define CG_VCEPLL_FUNC_CNTL__VCEPLL_BYPASS_EN__SHIFT                          2
#define CG_VCEPLL_FUNC_CNTL__VCEPLL_CTLREQ_MASK                               0x00000008
#define CG_VCEPLL_FUNC_CNTL__VCEPLL_CTLREQ__SHIFT                             3
#define CG_VCEPLL_FUNC_CNTL_2__EVCLK_SRC_SEL_MASK                             0x01F00000
#define CG_VCEPLL_FUNC_CNTL_2__EVCLK_SRC_SEL__SHIFT                           20
#define CG_VCEPLL_FUNC_CNTL_2__ECCLK_SRC_SEL_MASK                             0x3E000000
#define CG_VCEPLL_FUNC_CNTL_2__ECCLK_SRC_SEL__SHIFT                           25
#define CG_VCEPLL_FUNC_CNTL_2__VCEPLL_PDIV_A_MASK                             0x0000007F
#define CG_VCEPLL_FUNC_CNTL_2__VCEPLL_PDIV_A__SHIFT                           0
#define CG_VCEPLL_FUNC_CNTL_2__VCEPLL_PDIV_B_MASK                             0x00007F00
#define CG_VCEPLL_FUNC_CNTL_2__VCEPLL_PDIV_B__SHIFT                           8
#define CG_VCEPLL_FUNC_CNTL_3__VCEPLL_FB_DIV_MASK                             0x01FFFFFF
#define CG_VCEPLL_FUNC_CNTL_3__VCEPLL_FB_DIV__SHIFT                           0
#define CG_VCEPLL_FUNC_CNTL_5__RESET_ANTI_MUX_MASK                            0x00000200
#define CG_VCEPLL_FUNC_CNTL_5__RESET_ANTI_MUX__SHIFT                          9
#define CG_VCEPLL_SPREAD_SPECTRUM__VCEPLL_SSEN_MASK                           0x00000001
#define CG_VCEPLL_SPREAD_SPECTRUM__VCEPLL_SSEN__SHIFT                         0

#endif /*__SMU_6_0_SH_MASK_H__*/
