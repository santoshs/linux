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
 * @brief ARM PL310 handling
 */

#include "hw_arm.h"
#include "pl310.h"

PL310_STR    pl310;

#define L2              pl310

static void hw_l2_sync(volatile uint32_t * syncreg);
static void hw_l2_all_sets(volatile uint32_t *reg, volatile uint32_t * syncreg);
static void hw_l2_op(void * virtual_address, int32_t size, volatile uint32_t * reg, volatile uint32_t * syncreg);

#define L2_LINE_LENGTH  32

#define L2_ALL_SET_MASK 0xFF	/* 8 sets */

void* hw_mmu_physical_address_get(void *virtual_address)
{
	return virtual_address;
}

void hw_arm_l2_cache_clean_invalidate(void)
{
      hw_l2_all_sets(&L2.CleanAndInvalidateByWay, &L2.CacheSync);
}

uint32_t hw_l2_status_get(void)
{
  hw_l2_sync(&L2.CacheSync);
  return L2.Control;
}

void hw_arm_l2_cache_clean(void)
{
  hw_l2_all_sets(&L2.CleanByWay, &L2.CacheSync);
}

void hw_arm_l2_cache_invalidate(void)
{
  hw_l2_all_sets(&L2.InvalidateByWay, &L2.CacheSync);
}

static void hw_l2_all_sets(volatile uint32_t *reg, volatile uint32_t *syncreg)
{
  hw_l2_sync(syncreg);

  *reg = L2_ALL_SET_MASK;

  hw_l2_sync(syncreg);

  while(L2_ALL_SET_MASK & *reg)
    {
      ;
    }
}

static void hw_l2_op(void * virtual_address, int32_t size, volatile uint32_t * reg, volatile uint32_t * syncreg)
{
  void * phy;

  phy = hw_mmu_physical_address_get(virtual_address);

  if ((uint32_t)-1 != (uint32_t)phy)
    {
      if (((uint32_t)phy & (L2_LINE_LENGTH-1)))
        {
          size += (uint32_t)phy & (L2_LINE_LENGTH-1);
        }

      phy = (void*)((uint32_t)phy & ~(L2_LINE_LENGTH-1));

      if (!size)
        {
          size++;
        }

      hw_l2_sync(syncreg);

      for(;
          size > 0;
          size -= L2_LINE_LENGTH,
            phy = (void*)((uint32_t)phy + L2_LINE_LENGTH))
        {
          *reg = (uint32_t)phy;
        }

      hw_l2_sync(syncreg);
    }

}

void hw_arm_l2_cache_area_invalidate(void * virtual_address, int32_t size)
{
  hw_l2_op(virtual_address,
           size,
           &L2.InvalidateLineByPA, &L2.CacheSync);
}

void hw_arm_l2_cache_area_clean(void * virtual_address, int32_t size)
{
  hw_l2_op(virtual_address,
           size, 
           &L2.CleanLineByPA,
           &L2.CacheSync);
}

void hw_arm_l2_cache_area_clean_invalidate(void * virtual_address, int32_t size)
{
  hw_l2_op(virtual_address,
           size,
           &L2.CleanAndInvalidateLineByPA,
           &L2.CacheSync);
}

static void hw_l2_sync(volatile uint32_t * syncreg)
{

  uint32_t x;

  for(x = 0;
      x < 10000;
      x++)
    {
      hw_arm_write_buffer_drain();
  
      if (!(1 & *syncreg))
        {
          break;
        }
    }
}

