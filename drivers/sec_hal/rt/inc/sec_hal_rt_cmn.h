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
** Copyright (C) 2010-2012 Renesas Electronics Corp.                       **
** All rights reserved.                                                    **
** *********************************************************************** */
#ifndef SEC_HAL_RT_CMN_H
#define SEC_HAL_RT_CMN_H

/* ***************** MACROS, CONSTANTS, COMPILATION FLAGS **************** */
/* Component level, constant, visible to all, macro defs                   */
/* *********************************************************************** */
#ifndef FALSE
#define FALSE 0
#endif /* FALSE */

#ifndef TRUE
#define TRUE 1
#endif /* TRUE */

#ifndef NULL
#define NULL (void*)0
#endif /* NULL */




/* component level, changes according to env, visible to all, macros       */
/* *********************************************************************** */
#if (defined SEC_HAL_MSG_BYTE_ORDER)
#error !!Byte order macro already defined!!
#else
#define SEC_HAL_MSG_BYTE_ORDER 0x00
#endif

#if (defined SEC_HAL_MEM_VIR2PHY_FUNC || defined SEC_HAL_MEM_PHY2VIR_FUNC)
#error !!virt to phy to virt memory conversion macros already defined!!
#else
#ifdef SEC_HAL_TEST_ISOLATION
#define SEC_HAL_MEM_VIR2PHY_FUNC(vir) vir
#define SEC_HAL_MEM_PHY2VIR_FUNC(phy) phy
#else
#include <asm/io.h>
/*#define SEC_HAL_MEM_VIR2PHY_FUNC(vir) virt_to_phys(vir)*/
/* TBD, own macro for ICRAM memory conversion, others are needed too */
#define SEC_HAL_MEM_VIR2PHY_FUNC(vir) \
sec_hal_virt_to_icram_phys((unsigned long)vir)
#define SEC_HAL_MEM_PHY2VIR_FUNC(phy) \
sec_hal_icram_phys_to_virt((unsigned long)phy)
#endif /* SEC_HAL_TEST_ISOLATION */
#endif

#if (defined SEC_HAL_MEM_CACHE_CLEAN_FUNC || \
     defined SEC_HAL_MEM_CACHE_FLUSH_FUNC)
#error !!Memory cache macros already defined!!
#else
#ifdef SEC_HAL_TEST_ISOLATION
#define SEC_HAL_MEM_CACHE_CLEAN_FUNC(ptr, size)
#define SEC_HAL_MEM_CACHE_FLUSH_FUNC(ptr, size)
#else
#include <asm/cacheflush.h>
#define SEC_HAL_MEM_CACHE_CLEAN_FUNC(ptr, size) clean_dcache_area(ptr, size)
#define SEC_HAL_MEM_CACHE_FLUSH_FUNC(ptr, size)
#endif /* SEC_HAL_TEST_ISOLATION */
#endif

#if (defined SEC_HAL_MEM_RAM_ADDR_START)
#error !!RAM start address already defined!!
#else
#define SEC_HAL_MEM_RAM_ADDR_START                        (0xE63A0000UL)
#endif

#if (defined SEC_HAL_MEM_RAM_MSG_AREA_ADDR_START)
#error !!RAM msg area start address already defined!!
#else

#define SEC_HAL_MEM_RAM_MSG_AREA_ADDR_START SEC_HAL_MEM_RAM_ADDR_START
#endif

#if (defined SEC_HAL_MEM_RAM_MSG_AREA_SIZE)
#error !!RAM msg area size already defined!!
#else
#define SEC_HAL_MEM_RAM_MSG_AREA_SIZE                     (4096)
#endif

unsigned long sec_hal_virt_to_icram_phys(unsigned long virt_addr);
unsigned long sec_hal_icram_phys_to_virt(unsigned long phys_addr);

/* ******************************** END ********************************** */
#endif /* SEC_HAL_RT_CMN_H */
