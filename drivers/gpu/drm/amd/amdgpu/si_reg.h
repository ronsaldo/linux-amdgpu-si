#ifndef __SI_REG_H__

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

#endif
