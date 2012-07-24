/* 
 *  Copyright © Renesas Mobile Corporation 2010 . All rights reserved     
 * 
 *  This material, including documentation and any related source code and information, is protected by 
 *  copyright controlled by Renesas. All rights are reserved. Copying, including reproducing, storing, adapting,
 *  translating and modifying, including decompiling or reverse engineering, any or all of this material
 *  requires the prior written consent of Renesas. This material also contains confidential information, which
 *  may not be disclosed to others without the prior written consent of Renesas.                                                              
 */

/**
 * @file
 * @brief ARM CP15 functionality used by secure<->public world communication
 */ 

#ifndef __CACHE_MMU_IF_H__
#define __CACHE_MMU_IF_H__

#include "sec_type.h"

#define HW_ARM_DCACHE_OP_CLEAN                0  /* Clean D cache(s) */
#define HW_ARM_DCACHE_OP_INVALIDATE           1  /* Invalidate D cache(s) */
#define HW_ARM_DCACHE_OP_CLEAN_INVALIDATE     2  /* Clean & invalidate D cache(s) */

#define HW_ARM_DCACHE_LEVEL_POU               0  /* Operate up to point of unification level (meaning L1 in R_Mobile_U) */

/**
 * @brief Cache handling
 * @param[in]
 *    operation  one of HW_ARM_DCACHE_OP_CLEAN HW_ARM_DCACHE_OP_INVALIDATE HW_ARM_DCACHE_OP_CLEAN_INVALIDATE
 * @param[in]
 *    level      one of HW_ARM_DCACHE_LEVEL_POU
 */
 
void   hw_arm_dcache_maintenance(uint32_t operation, uint32_t level);

/** 
 * @brief Virtual to physical address conversion
 *
 *    This function converts from virtual to physical address in current world (secure/non-secure). If MMU is not
 *    enabled, returns the input parameter unmodified.
 *
 * @param[in]
 *    Virtual address
 *
 * @retval physical address
 */
 
void * hw_mmu_physical_address_get(void*);

#endif /* __CACHE_MMU_IF_H__ */
