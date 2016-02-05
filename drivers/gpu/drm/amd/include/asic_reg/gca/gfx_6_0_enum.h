#ifndef __GFX_6_0_ENUM_H__
#define __GFX_6_0_ENUM_H__

typedef enum GB_TILE_MODE0 {
	ADDR_SURF_DISPLAY_MICRO_TILING                     = 0,
	ADDR_SURF_THIN_MICRO_TILING                        = 1,
	ADDR_SURF_DEPTH_MICRO_TILING                       = 2,
	ARRAY_LINEAR_GENERAL                               = 0,
	ARRAY_LINEAR_ALIGNED                               = 1,
	ARRAY_1D_TILED_THIN1                               = 2,
	ARRAY_2D_TILED_THIN1                               = 4,
	ADDR_SURF_P2                                       = 0,
	ADDR_SURF_TILE_SPLIT_64B                           = 0,
	ADDR_SURF_TILE_SPLIT_128B                          = 1,
	ADDR_SURF_TILE_SPLIT_256B                          = 2,
	ADDR_SURF_TILE_SPLIT_512B                          = 3,
	ADDR_SURF_TILE_SPLIT_1KB                           = 4,
	ADDR_SURF_TILE_SPLIT_2KB                           = 5,
	ADDR_SURF_TILE_SPLIT_4KB                           = 6,
	ADDR_SURF_BANK_WIDTH_1                             = 0,
	ADDR_SURF_BANK_WIDTH_2                             = 1,
	ADDR_SURF_BANK_WIDTH_4                             = 2,
	ADDR_SURF_BANK_WIDTH_8                             = 3,
	ADDR_SURF_BANK_HEIGHT_1                            = 0,
	ADDR_SURF_BANK_HEIGHT_2                            = 1,
	ADDR_SURF_BANK_HEIGHT_4                            = 2,
	ADDR_SURF_BANK_HEIGHT_8                            = 3,
	ADDR_SURF_MACRO_ASPECT_1                           = 0,
	ADDR_SURF_MACRO_ASPECT_2                           = 1,
	ADDR_SURF_MACRO_ASPECT_4                           = 2,
	ADDR_SURF_MACRO_ASPECT_8                           = 3,
	ADDR_SURF_2_BANK                                   = 0,
	ADDR_SURF_4_BANK                                   = 1,
	ADDR_SURF_8_BANK                                   = 2,
	ADDR_SURF_16_BANK                                  = 3,
} GB_TILE_MODE0;
typedef enum PA_SC_RASTER_CONFIG {
	RASTER_CONFIG_RB_MAP_0                             = 0,
	RASTER_CONFIG_RB_MAP_1                             = 1,
	RASTER_CONFIG_RB_MAP_2                             = 2,
	RASTER_CONFIG_RB_MAP_3                             = 3,
} PA_SC_RASTER_CONFIG;

#endif /*__GFX_6_0_ENUM_H__*/
