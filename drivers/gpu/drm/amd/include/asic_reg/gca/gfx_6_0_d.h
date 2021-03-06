#ifndef __GFX_6_0_D_H__
#define __GFX_6_0_D_H__

#define mmCC_SYS_RB_BACKEND_DISABLE                                             0x3a0
#define mmGC_USER_SYS_RB_BACKEND_DISABLE                                        0x3a1
#define mmGRBM_CNTL                                                             0x2000
#define mmGRBM_STATUS2                                                          0x2002
#define mmGRBM_STATUS                                                           0x2004
#define mmGRBM_STATUS_SE0                                                       0x2005
#define mmGRBM_STATUS_SE1                                                       0x2006
#define mmGRBM_SOFT_RESET                                                       0x2008
#define mmGRBM_GFX_INDEX                                                        0x200b
#define mmGRBM_INT_CNTL                                                         0x2018
#define mmCP_STRMOUT_CNTL                                                       0x213f
#define mmSCRATCH_REG0                                                          0x2140
#define mmSCRATCH_REG1                                                          0x2141
#define mmSCRATCH_REG2                                                          0x2142
#define mmSCRATCH_REG3                                                          0x2143
#define mmSCRATCH_REG4                                                          0x2144
#define mmSCRATCH_REG5                                                          0x2145
#define mmSCRATCH_REG6                                                          0x2146
#define mmSCRATCH_REG7                                                          0x2147
#define mmSCRATCH_UMSK                                                          0x2150
#define mmSCRATCH_ADDR                                                          0x2151
#define mmCP_SEM_WAIT_TIMER                                                     0x216f
#define mmCP_SEM_INCOMPLETE_TIMER_CNTL                                          0x2172
#define mmCP_ME_CNTL                                                            0x21b6
#define mmCP_COHER_CNTL2                                                        0x217a
#define mmCP_RB2_RPTR                                                           0x21be
#define mmCP_RB1_RPTR                                                           0x21bf
#define mmCP_RB0_RPTR                                                           0x21c0
#define mmCP_RB_WPTR_DELAY                                                      0x21c1
#define mmCP_QUEUE_THRESHOLDS                                                   0x21d8
#define mmCP_MEQ_THRESHOLDS                                                     0x21d9
#define mmCP_PERFMON_CNTL                                                       0x21ff
#define mmVGT_VTX_VECT_EJECT_REG                                                0x222c
#define mmVGT_CACHE_INVALIDATION                                                0x2231
#define mmVGT_ESGS_RING_SIZE                                                    0x2232
#define mmVGT_GSVS_RING_SIZE                                                    0x2233
#define mmVGT_GS_VERTEX_REUSE                                                   0x2235
#define mmVGT_PRIMITIVE_TYPE                                                    0x2256
#define mmVGT_INDEX_TYPE                                                        0x2257
#define mmVGT_NUM_INDICES                                                       0x225c
#define mmVGT_NUM_INSTANCES                                                     0x225d
#define mmVGT_TF_RING_SIZE                                                      0x2262
#define mmVGT_HS_OFFCHIP_PARAM                                                  0x226c
#define mmVGT_TF_MEMORY_BASE                                                    0x226e
#define mmCC_GC_SHADER_ARRAY_CONFIG                                             0x226f
#define mmGC_USER_SHADER_ARRAY_CONFIG                                           0x2270
#define mmPA_CL_ENHANCE                                                         0x2285
#define mmPA_SU_LINE_STIPPLE_VALUE                                              0x2298
#define mmPA_SC_LINE_STIPPLE_STATE                                              0x22c4
#define mmPA_SC_FORCE_EOV_MAX_CNTS                                              0x22c9
#define mmPA_SC_FIFO_SIZE                                                       0x22f3
#define mmPA_SC_ENHANCE                                                         0x22fc
#define mmSQ_CONFIG                                                             0x2300
#define mmSQ_POWER_THROTTLE                                                     0x2396
#define mmSQ_POWER_THROTTLE2                                                    0x2397
#define mmSX_DEBUG_1                                                            0x2418
#define mmSPI_STATIC_THREAD_MGMT_1                                              0x2438
#define mmSPI_STATIC_THREAD_MGMT_2                                              0x2439
#define mmSPI_STATIC_THREAD_MGMT_3                                              0x243a
#define mmSPI_PS_MAX_WAVE_ID                                                    0x243b
#define mmSPI_CONFIG_CNTL                                                       0x2440
#define mmSPI_CONFIG_CNTL_1                                                     0x244f
#define mmCGTS_TCC_DISABLE                                                      0x2452
#define mmCGTS_USER_TCC_DISABLE                                                 0x2453
#define mmCGTS_SM_CTRL_REG                                                      0x2454
#define mmSPI_LB_CU_MASK                                                        0x24d5
#define mmCC_RB_BACKEND_DISABLE                                                 0x263d
#define mmGB_ADDR_CONFIG                                                        0x263e
#define mmGB_TILE_MODE0                                                         0x2644
#define mmGB_TILE_MODE1                                                         0x2645
#define mmGB_TILE_MODE2                                                         0x2646
#define mmGB_TILE_MODE3                                                         0x2647
#define mmGB_TILE_MODE4                                                         0x2648
#define mmGB_TILE_MODE5                                                         0x2649
#define mmGB_TILE_MODE6                                                         0x264a
#define mmGB_TILE_MODE7                                                         0x264b
#define mmGB_TILE_MODE8                                                         0x264c
#define mmGB_TILE_MODE9                                                         0x264d
#define mmGB_TILE_MODE10                                                        0x264e
#define mmGB_TILE_MODE11                                                        0x264f
#define mmGB_TILE_MODE12                                                        0x2650
#define mmGB_TILE_MODE13                                                        0x2651
#define mmGB_TILE_MODE14                                                        0x2652
#define mmGB_TILE_MODE15                                                        0x2653
#define mmGB_TILE_MODE16                                                        0x2654
#define mmGB_TILE_MODE17                                                        0x2655
#define mmGB_TILE_MODE18                                                        0x2656
#define mmGB_TILE_MODE19                                                        0x2657
#define mmGB_TILE_MODE20                                                        0x2658
#define mmGB_TILE_MODE21                                                        0x2659
#define mmGB_TILE_MODE22                                                        0x265a
#define mmGB_TILE_MODE23                                                        0x265b
#define mmGB_TILE_MODE24                                                        0x265c
#define mmGB_TILE_MODE25                                                        0x265d
#define mmGB_TILE_MODE26                                                        0x265e
#define mmGB_TILE_MODE27                                                        0x265f
#define mmGB_TILE_MODE28                                                        0x2660
#define mmGB_TILE_MODE29                                                        0x2661
#define mmGB_TILE_MODE30                                                        0x2662
#define mmGB_TILE_MODE31                                                        0x2663
#define mmCB_PERFCOUNTER0_SELECT0                                               0x2688
#define mmCB_PERFCOUNTER0_SELECT1                                               0x2689
#define mmCB_PERFCOUNTER1_SELECT0                                               0x268a
#define mmCB_PERFCOUNTER1_SELECT1                                               0x268b
#define mmCB_PERFCOUNTER2_SELECT0                                               0x268c
#define mmCB_PERFCOUNTER2_SELECT1                                               0x268d
#define mmCB_PERFCOUNTER3_SELECT0                                               0x268e
#define mmCB_PERFCOUNTER3_SELECT1                                               0x268f
#define mmCB_CGTT_SCLK_CTRL                                                     0x2698
#define mmGC_USER_RB_BACKEND_DISABLE                                            0x26df
#define mmCP_RB0_BASE                                                           0x3040
#define mmCP_RB0_CNTL                                                           0x3041
#define mmCP_RB0_RPTR_ADDR                                                      0x3043
#define mmCP_RB0_RPTR_ADDR_HI                                                   0x3044
#define mmCP_RB0_WPTR                                                           0x3045
#define mmCP_PFP_UCODE_ADDR                                                     0x3054
#define mmCP_PFP_UCODE_DATA                                                     0x3055
#define mmCP_ME_RAM_RADDR                                                       0x3056
#define mmCP_ME_RAM_WADDR                                                       0x3057
#define mmCP_ME_RAM_DATA                                                        0x3058
#define mmCP_CE_UCODE_ADDR                                                      0x305a
#define mmCP_CE_UCODE_DATA                                                      0x305b
#define mmCP_RB1_BASE                                                           0x3060
#define mmCP_RB1_CNTL                                                           0x3061
#define mmCP_RB1_RPTR_ADDR                                                      0x3062
#define mmCP_RB1_RPTR_ADDR_HI                                                   0x3063
#define mmCP_RB1_WPTR                                                           0x3064
#define mmCP_RB2_BASE                                                           0x3065
#define mmCP_RB2_CNTL                                                           0x3066
#define mmCP_RB2_RPTR_ADDR                                                      0x3067
#define mmCP_RB2_RPTR_ADDR_HI                                                   0x3068
#define mmCP_RB2_WPTR                                                           0x3069
#define mmCP_INT_CNTL_RING0                                                     0x306a
#define mmCP_INT_CNTL_RING1                                                     0x306b
#define mmCP_INT_CNTL_RING2                                                     0x306c
#define mmCP_INT_STATUS_RING0                                                   0x306d
#define mmCP_INT_STATUS_RING1                                                   0x306e
#define mmCP_INT_STATUS_RING2                                                   0x306f
#define mmCP_MEM_SLP_CNTL                                                       0x3079
#define mmCP_DEBUG                                                              0x307f
#define mmRLC_CNTL                                                              0x30c0
#define mmRLC_RL_BASE                                                           0x30c1
#define mmRLC_RL_SIZE                                                           0x30c2
#define mmRLC_LB_CNTL                                                           0x30c3
#define mmRLC_SAVE_AND_RESTORE_BASE                                             0x30c4
#define mmRLC_LB_CNTR_MAX                                                       0x30c5
#define mmRLC_LB_CNTR_INIT                                                      0x30c6
#define mmRLC_CLEAR_STATE_RESTORE_BASE                                          0x30c8
#define mmRLC_UCODE_ADDR                                                        0x30cb
#define mmRLC_UCODE_DATA                                                        0x30cc
#define mmRLC_GPU_CLOCK_COUNT_LSB                                               0x30ce
#define mmRLC_GPU_CLOCK_COUNT_MSB                                               0x30cf
#define mmRLC_CAPTURE_GPU_CLOCK_COUNT                                           0x30d0
#define mmRLC_MC_CNTL                                                           0x30d1
#define mmRLC_UCODE_CNTL                                                        0x30d2
#define mmRLC_STAT                                                              0x30d3
#define mmRLC_PG_CNTL                                                           0x30d7
#define mmRLC_CGTT_MGCG_OVERRIDE                                                0x3100
#define mmRLC_CGCG_CGLS_CTRL                                                    0x3101
#define mmRLC_TTOP_D                                                            0x3105
#define mmRLC_LB_INIT_CU_MASK                                                   0x3107
#define mmRLC_PG_AO_CU_MASK                                                     0x310b
#define mmRLC_MAX_PG_CU                                                         0x310c
#define mmRLC_AUTO_PG_CTRL                                                      0x310d
#define mmRLC_SERDES_WR_MASTER_MASK_0                                           0x3115
#define mmRLC_SERDES_WR_MASTER_MASK_1                                           0x3116
#define mmRLC_SERDES_WR_CTRL                                                    0x3117
#define mmRLC_SERDES_MASTER_BUSY_0                                              0x3119
#define mmRLC_SERDES_MASTER_BUSY_1                                              0x311a
#define mmRLC_GCPM_GENERAL_3                                                    0x311e
#define mmDB_RENDER_CONTROL                                                     0xa000
#define mmDB_DEPTH_INFO                                                         0xa00f
#define mmPA_SC_RASTER_CONFIG                                                   0xa0d4
#define mmVGT_EVENT_INITIATOR                                                   0xa2a4
#define mmGB_BACKEND_MAP                                                        0x263f
#define mmCP_STALLED_STAT1                                                      0x219d
#define mmCP_STALLED_STAT2                                                      0x219e
#define mmCP_BUSY_STAT                                                          0x219f
#define mmCP_STAT                                                               0x21a0

#endif /*__GFX_6_0_D_H__*/
