/* ************************************************************************* **
**                               Renesas                                     **
** ************************************************************************* */

/* *************************** COPYRIGHT INFORMATION *********************** **
** This program contains proprietary information that is a trade secret of   **
** Renesas and also is protected as an unpublished work under                **
** applicable Copyright laws. Recipient is to retain this program in         **
** confidence and is not permitted to use or make copies thereof other than  **
** as permitted in a written agreement with Renesas.                         **
**                                                                           **
** Copyright (C) 2012 Renesas Electronics Corp.                              **
** All rights reserved.                                                      **
** ************************************************************************* */
#include "sec_dispatch.h"

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
** Function name      : sec_dispatcher
** Description        :
** Parameters         :
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** ***************************************************************************/
uint32_t sec_dispatcher(uint32_t appl_id, uint32_t flags, ...)
{
    va_list ap;
    uint32_t return_value;
    uint32_t pub_cpsr;

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
#if 1
    return_value = hw_sec_rom_pub_bridge(appl_id, flags, &ap);  /* 'Address of va_list' convention is inherited from previous projects */
#endif
    va_end(ap);

    return return_value;
}
