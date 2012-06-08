/*  SMC Platform Specific Configuration header for Linux Kernel
*   Copyright © Renesas Mobile Corporation 2011. All rights reserved
*
*   This material, including documentation and any related source code
*   and information, is protected by copyright controlled by Renesas.
*   All rights are reserved. Copying, including reproducing, storing,
*   adapting, translating and modifying, including decompiling or
*   reverse engineering, any or all of this material requires the prior
*   written consent of Renesas. This material also contains
*   confidential information, which may not be disclosed to others
*   without the prior written consent of Renesas.
*/
#if 0
/*
Change history:

Version:       1    21-Dec-2011     Heikki Siikaluoma
Status:        draft
Description :  File created
-------------------------------------------------------------------------------
*/
#endif


#ifndef SMC_CONF_PLATFORM_H
#define SMC_CONF_PLATFORM_H

    /* ===============================================
     * Define Linux Kernel Specific data types for SMC
     */

#include <linux/interrupt.h>
#include <linux/irq.h>
#include <asm/signal.h>


    // EOS2 registers
    // TODO Use own Include file
    //
#define HPB_BASE    0xE6000000
#define CCCR        (HPB_BASE + 0x101C)

#define SMC_WPMCIF_EPMU_BASE        0xE6190000
#define SMC_WPMCIF_EPMU_ACC_CR      (SMC_WPMCIF_EPMU_BASE + 0x0004)

#ifdef SMC_CONF_PM_APE_HOST_ACCESS_REQ_ENABLED
  #if(SMC_CONF_PM_APE_HOST_ACCESS_REQ_ENABLED==TRUE)

    // TODO Add Global spinlock
    #define SMC_HOST_ACCESS_WAKEUP(spinlock)  { if( spinlock != NULL ) SMC_LOCK_IRQ( spinlock );            \
                                                __raw_writel(0x00000002, SMC_WPMCIF_EPMU_ACC_CR);           \
                                               (void)__raw_readl(SMC_WPMCIF_EPMU_ACC_CR);                   \
                                               while (0x00000003 != __raw_readl(SMC_WPMCIF_EPMU_ACC_CR)) {} \
                                              }


    #define SMC_HOST_ACCESS_SLEEP(spinlock)   { __raw_writel(0x00000000, SMC_WPMCIF_EPMU_ACC_CR);  \
                                               (void)__raw_readl(SMC_WPMCIF_EPMU_ACC_CR);          \
                                               if( spinlock != NULL ) SMC_UNLOCK_IRQ( spinlock );       \
                                              }

  #else
    #define SMC_HOST_ACCESS_WAKEUP(spinlock)
    #define SMC_HOST_ACCESS_SLEEP(spinlock)
  #endif
#else
  #error "SMC_CONF_PM_APE_HOST_ACCESS_REQ_ENABLED --- NOT DEFINED"
#endif

#define SMC_EOS_ASIC_ES10   0x10
#define SMC_EOS_ASIC_ES20   0x20

#define SMC_SIGNAL_TYPE_INTGEN         (SMC_SIGNAL_TYPE_INTERRUPT + SMC_SIGNAL_TYPE_PRIVATE_START + 0x01)  /* 0x03000001 */
#define SMC_SIGNAL_TYPE_INTCBB         (SMC_SIGNAL_TYPE_INTERRUPT + SMC_SIGNAL_TYPE_PRIVATE_START + 0x02)  /* 0x03000002 */
#define SMC_SIGNAL_TYPE_INT_WGM_GENOUT (SMC_SIGNAL_TYPE_INTERRUPT + SMC_SIGNAL_TYPE_PRIVATE_START + 0x04)  /* 0x03000004 */
#define SMC_SIGNAL_TYPE_INT_RESOURCE   (SMC_SIGNAL_TYPE_INTERRUPT + SMC_SIGNAL_TYPE_PRIVATE_START + 0x06)  /* 0x03000006 */

    /*
     * Data type for SMC signals
     */
typedef struct _smc_signal_t
{
    uint32_t interrupt_id;
    uint32_t signal_type;
    uint32_t peripheral_address;
    uint8_t  address_remapped;

} smc_signal_t;


typedef struct
{
  volatile uint32_t OUTPUT;        /* output := <>              input := output */
  volatile uint32_t SET;           /* output := output OR <>    input := None   */
  volatile uint32_t CLEAR;         /* output := output AND ~<>  input := None   */
  volatile uint32_t TOGGLE;        /* output := output XOR <>   input := None   */
} GOP001_STR;


/*
 * Shared memory remapping from physical to virtual address
 */

#include <asm/irq.h>

#define SMC_SHM_IOREMAP( address, size )                        ioremap( (long unsigned int)address, size )

#define SMC_HW_ARM_MEMORY_SYNC(startaddress)

#define SMC_SHM_WRITE32( target_address, value )                 __raw_writel( value, ((void __iomem *)(target_address)) )
#define SMC_SHM_READ32( source_address )                         __raw_readl( ((void __iomem *)source_address) )

#define SMC_SHM_CACHE_INVALIDATE( start_address, end_address )
#define SMC_SHM_CACHE_CLEAN( start_address, end_address )        { __raw_readl( ((void __iomem *)(start_address)) ); __raw_readl( ((void __iomem *)(end_address)) );  }


#define  SMC_LOCK( lock )        { SMC_TRACE_PRINTF_LOCK("lock: 0x%08X...", (uint32_t)lock); spin_lock( &(lock->mr_lock)); }
#define  SMC_UNLOCK( lock )      { spin_unlock( &(lock->mr_lock)); SMC_TRACE_PRINTF_LOCK("unlock: 0x%08X", (uint32_t)lock); }
#define  SMC_LOCK_IRQ( lock )    { SMC_TRACE_PRINTF_LOCK("lock irq save: 0x%08X...", (uint32_t)lock); spin_lock_irqsave( &lock->mr_lock, lock->flags); }
#define  SMC_UNLOCK_IRQ( lock )  { spin_unlock_irqrestore( &lock->mr_lock, lock->flags); SMC_TRACE_PRINTF_LOCK("unlock irq save: 0x%08X...", (uint32_t)lock); }

    /*
     * Data type for SMC locking
     */
#include <linux/spinlock.h>

typedef struct _smc_lock_t
{
    spinlock_t    mr_lock;
    unsigned long flags;   /* For IRQ lock */

} smc_lock_t;

    /*
     * Data type for SMC semaphore
     */
typedef struct _smc_semaphore_t
{
    struct semaphore* sem;

} smc_semaphore_t;


    /*
     * Data type for SMC timer
     */
typedef struct _smc_timer_t
{
    uint32_t           period_us;
    struct timer_list* smc_timer_list;
    uint32_t           timer_data;

    unsigned long      prev_jiffies;

} smc_timer_t;


uint16_t smc_asic_version_get(void);

#endif

/* EOF */
