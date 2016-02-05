#ifndef __OTHER_SI_ENUM_H__
#define __OTHER_SI_ENUM_H__

typedef enum VGT_CACHE_INVALIDATION {
	VC_ONLY                                            = 0,
	TC_ONLY                                            = 1,
	VC_AND_TC                                          = 2,
	NO_AUTO                                            = 0,
	ES_AUTO                                            = 1,
	GS_AUTO                                            = 2,
	ES_AND_GS_AUTO                                     = 3,
} VGT_CACHE_INVALIDATION;
typedef enum PACKET3_NOP {
	GDS_PARTITION_BASE                                 = 2,
	CE_PARTITION_BASE                                  = 3,
} PACKET3_NOP;
typedef enum PACKET3_SET_CONFIG_REG {
	PACKET3_SET_CONFIG_REG_START                       = 0x00008000,
	PACKET3_SET_CONFIG_REG_END                         = 0x0000b000,
	PACKET3_SET_CONTEXT_REG_START                      = 0x00028000,
	PACKET3_SET_CONTEXT_REG_END                        = 0x00029000,
} PACKET3_SET_CONFIG_REG;
typedef enum PACKET3_SET_RESOURCE_INDIRECT {
	PACKET3_SET_SH_REG_START                           = 0x0000b000,
	PACKET3_SET_SH_REG_END                             = 0x0000c000,
} PACKET3_SET_RESOURCE_INDIRECT;

#endif /*__OTHER_SI_ENUM_H__*/
