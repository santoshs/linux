/* *********************************************************************** **
**                               Renesas                                   **
** *********************************************************************** */

/* *************************** COPYRIGHT INFORMATION ********************* **
** This program contains proprietary information that is a trade secret of **
** Renesas and also is protected as an unpublished work under              **
** applicable Copyright laws. Recipient is to retain this program in       **
** confidence and is not permitted to use or make copies thereof other than**
** as permitted in a written agreement with Renesas.                       **
**                                                                         **
** Copyright (C) 2012 Renesas Electronics Corp.                            **
** All rights reserved.                                                    **
** *********************************************************************** */
#include "sec_dispatch.h"
#include "sec_hal_rt_cmn.h"
#include <stdarg.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/spinlock_types.h>
#include <linux/completion.h>
#include <linux/ioport.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/percpu.h>
#include <asm/cacheflush.h>


#define SEC_ROM_ICACHE_ENABLE_MASK              0x00000001
#define SEC_ROM_DCACHE_ENABLE_MASK              0x00000002
#define SEC_ROM_IRQ_ENABLE_MASK                 0x00000004
#define SEC_ROM_FIQ_ENABLE_MASK                 0x00000008
#define SEC_ROM_UL2_CACHE_ENABLE_MASK           0x00000010

#define MODE_USER                               0x10
#define MODE_FIQ                                0x11
#define MODE_IRQ                                0x12
#define MODE_SVC                                0x13
#define MODE_ABORT                              0x17
#define MODE_UNDEF                              0x1B
#define MODE_SYSTEM                             0x1F
#define MODE_MONITOR                            0x16
#define MODE_CLR_MASK                           0x1F
#define ARM_THUMB_MODE_MASK                     0x20
#define FIQ_IRQ_MASK                            0xC0
#define FIQ_MASK                                0x40
#define IRQ_MASK                                0x80

uint32_t hw_sec_rom_pub_bridge(uint32_t appl_id, uint32_t flags, va_list *);
/* ****************************************************************************
** Function name      : pub2sec_dispatcher
** Description        :
** Parameters         :
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** ***************************************************************************/
uint32_t pub2sec_dispatcher(uint32_t appl_id, uint32_t flags, ...)
{
    uint32_t return_value, pub_cpsr;
    va_list ap;

    /* Read current CPSR */
    __asm__ __volatile__("MRS %0, CPSR" : "=r"(pub_cpsr));

    /* FIQ won't be enabled in secure mode if FIQ is currently disabled */
    if (pub_cpsr & FIQ_MASK) {
        flags &= ~SEC_ROM_FIQ_ENABLE_MASK;
    }

    /* IRQ won't be enabled in secure mode if IRQ is currently disabled */
    if (pub_cpsr & IRQ_MASK) {
        flags &= ~SEC_ROM_IRQ_ENABLE_MASK;
    }

    va_start(ap, flags);
    /* 'Address of va_list' convention is inherited from previous projects */
    return_value = hw_sec_rom_pub_bridge(appl_id, flags, &ap);
    va_end(ap);

    return return_value;
}


/* MMU */
void* hw_mmu_physical_address_get(void* arg)
{
    va_list* va_ptr = (va_list*)arg;
    va_ptr->__ap = virt_to_phys((void*)va_ptr->__ap); 
    return (void*)virt_to_phys(arg);
}


/* D-Cache */
/* < Clean D cache(s) */
#define HW_ARM_DCACHE_OP_CLEAN                0
/* < Invalidate D cache(s) */
#define HW_ARM_DCACHE_OP_INVALIDATE           1
/* < Clean & invalidate D cache(s) */
#define HW_ARM_DCACHE_OP_CLEAN_INVALIDATE     2
/* < Operate up to point of unification level (meaning L1 in R_Mobile_U) */
#define HW_ARM_DCACHE_LEVEL_POU               0
void hw_arm_dcache_maintenance(uint32_t op, uint32_t lvl)
{
    switch(op)
    {
        case HW_ARM_DCACHE_OP_CLEAN:
        case HW_ARM_DCACHE_OP_INVALIDATE:
        case HW_ARM_DCACHE_OP_CLEAN_INVALIDATE:
        default: break;
    }
}


/* L2-Cache */
void hw_arm_l2_cache_area_clean(void * virt_addr, int32_t size)
{
    __cpuc_flush_dcache_area(virt_addr, size);
}


/* ******************************** END ************************************ */
