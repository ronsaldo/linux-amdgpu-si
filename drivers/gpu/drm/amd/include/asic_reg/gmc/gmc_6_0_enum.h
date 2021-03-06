#ifndef __GMC_6_0_ENUM_H__
#define __GMC_6_0_ENUM_H__

typedef enum VM_L2_CNTL2 {
	VM_L2_CNTL2__INVALIDATE_PTE_AND_PDE_CACHES         = 0,
	VM_L2_CNTL2__INVALIDATE_ONLY_PTE_CACHES            = 1,
	VM_L2_CNTL2__INVALIDATE_ONLY_PDE_CACHES            = 2,
} VM_L2_CNTL2;
typedef enum MC_SEQ_MISC0 {
	MC_SEQ_MISC0__MC_SEQ_MISC0_VEN_ID_VALUE            = 3,
	MC_SEQ_MISC0__MC_SEQ_MISC0_REV_ID_VALUE            = 1,
	MC_SEQ_MISC0__MC_SEQ_MISC0_GDDR5_VALUE             = 5,
} MC_SEQ_MISC0;

#endif /*__GMC_6_0_ENUM_H__*/
