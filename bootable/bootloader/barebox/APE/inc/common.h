/* common.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef __COMMON_H__
#define __COMMON_H__

#include "com_type.h"

#define ENBLE_SERCUE_RWDT_CLR
/************************************************************************************************/
/*	Definitions																					*/
/************************************************************************************************/

/* Bit Bitmask */
#define BIT31									(ulong)(0x80000000)
#define BIT17									(ulong)(0x00020000)
#define BIT7									(ulong)(0x00000080)
#define BIT6									(ulong)(0x00000040)
#define BIT5									(ulong)(0x00000020)
#define BIT4									(ulong)(0x00000010)
#define BIT3									(ulong)(0x00000008)
#define BIT2									(ulong)(0x00000004)
#define BIT1									(ulong)(0x00000002)
#define BIT0									(ulong)(0x00000001)

/* write RWTCNT, value is 0xA5A55Axx, align is 32bit. read RWCNT align is 16bit. */
#define RWTCNT				(0xE6020000)
#define RWTCSRA				((volatile ulong*)0xE6020004)
#define RWTCNT_W			(volatile ulong*)RWTCNT
#define RWTCNT_R			(volatile uchar*)RWTCNT

/* RWDT secure */
#define RWTCNT_SEC				(0xE6030000)
#define RWTCSRA_SEC				((volatile ulong*)0xE6030004)
#define RWTCNT_W_SEC			(volatile ulong*)RWTCNT_SEC
#define RWTCNT_R_SEC			(volatile uchar*)RWTCNT_SEC

/* RWDT data */
#define RWDT_CLEAR			((ulong)0x5A5A0000)
#define RWDT_DISABLE		((ulong)0xA5A5A500)
#define TME					((ulong)0x00000080)

/* Common chip code register */
#define CHIP_VER_REG_CCCR	((volatile ulong*)0xE600101C)
#define CHIP_RMU2_ES10		(0x00003E00)
#define CHIP_RMU2_ES20		(0x00003E10)
#define CHIP_RMU2_ES21		(0x00003E11)
#define CHIP_RMU2_ES22		(0x00003E12)

/* Write protection area */
#define EMMC_PROTECT_AREA_ST 	(0x10000)	/* eMMC write protection area start sector address */
#define EMMC_PROTECT_AREA_ED 	(0x109FFF)	/* eMMC write protection area end  sector address 	*/

/* Standby Flag Register */
#define STBCHR0							((volatile uchar *)(0xE6180000))
#define STBCHR1							((volatile uchar *)(0xE6180001))
#define STBCHR2							((volatile uchar *)(0xE6180002))
#define STBCHR3							((volatile uchar *)(0xE6180003))
#define STBCHRB0						((volatile uchar *)(0xE6180040))
#define STBCHRB1						((volatile uchar *)(0xE6180041))
#define STBCHRB2						((volatile uchar *)(0xE6180042))

/* SYS Wakeup control register */
#define SWUCR							((volatile ulong *)(0xE6180014))
#define SWBCR							((volatile ulong *)(0xE6180204))
#define PSTR							((volatile ulong *)(0xE6180080))

/* Reset status register */
#define SRSTFR							((volatile ulong *)(0xE61800B4))

/* Reset Control Register */
#define RESCNT							((volatile ulong *)(0xE618801C))

/* Non-Volatile Memory boot flag */
/* BEGIN: CR1040: QA1040: Clean up source code which accesses the eMMC directly */
#define BOOTFLAG_SDRAM_ADDR				0x47FBFF80
#define BOOTFLAG_SDRAM_SIZE				0x80
#define BOOTFLAG_SDRAM_OFFSET			0x01
/* END: CR1040: QA1040: Clean up source code which accesses the eMMC directly */
/* BEGIN: CR722: Apply GPT */
#define STBCHRB1_BOOT_PARTITION2_USED	(uchar)(0x10)
#define BOOTFLAG_ADDR					((*STBCHRB1 & STBCHRB1_BOOT_PARTITION2_USED) == 0 ? (0xF00000000003A000ull):(0xE00000000003A000ull))
// #define BOOTFLAG_ADDR					((uint64)(0x0000000000040000))
#define BOOTFLAG_SIZE					(12)
/* END: CR722: Apply GPT */

/************************************************************************************************/
/*	Macro																						*/
/************************************************************************************************/
#ifdef ENBLE_SERCUE_RWDT_CLR
#define WDT_CLEAR()						\
			({							\
			*RWTCNT_W = RWDT_CLEAR;		\
			*RWTCNT_W_SEC = RWDT_CLEAR;	\
			})
#else

#define WDT_CLEAR()						\
			({							\
			*RWTCNT_W = RWDT_CLEAR;		\
			})
#endif /*ENBLE_SERCUE_RWDT_CLR*/


/* Disable RCLK Watchdog Timer */
#define WDT_DISABLE()				\
		({							\
			ulong data;			\
			data = *RWTCSRA;		\
			data |= RWDT_DISABLE;	\
			data &= ~TME;			\
			*RWTCSRA = data;		\
		})
#define WDT_CNT()			(*RWTCNT_R)							/* Get RCLK Watchdog Timer Counter */
#define CHIP_VERSION()		(*CHIP_VER_REG_CCCR & 0x0000FFFF)	/* Get CP1/CP1+ Chip Version */


/************************************************************************************************/
/*	Bit mask																					*/
/************************************************************************************************/
#define STBCHRB1_FIRST_DOMAIN_COMPLETE	(uchar)(0x80)
#define STBCHRB2_PWROFF					(uchar)(0x80)
#define SRSTFR_RESCNT2					(uint)(0x21)
/* SYS Wakeup control register */
#define PSTR_MASK						0x1FF1443

/************************************************************************************************/
/*	Global Data																					*/
/************************************************************************************************/


/************************************************************************************************/
/*	Prototypes																					*/
/************************************************************************************************/
void ChangeFlashingMode(void);
void Reset(void);

void SetUsbDownload(void);
/* void SetBootClock(void); */
void SetUsbBootClock(void);

/************************************************************************************************/
/*	for debug																					*/
/************************************************************************************************/


#endif /* __COMMON_H__ */

