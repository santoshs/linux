/* common_set.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "com_type.h"
#include <common.h>

/********************************************************************************/
/*	Definitions																	*/
/********************************************************************************/
/* Standby Flag Register3 */
#define STBCHRB0_USB_DOWNLOAD		((uchar)(0x01))

/* CPG */
#define CPG_BASE					(0xE6150000ul)
#define CPG_FRQCRB					((volatile ulong*)(CPG_BASE + 0x0004))
#define CPG_PLL1CR					((volatile ulong*)(CPG_BASE + 0x0028))
#define CPG_PLLECR					((volatile ulong*)(CPG_BASE + 0x00D0))
#define CPG_PLL0CR					((volatile ulong*)(CPG_BASE + 0x00D8))
#define CPG_MPCKCR					((volatile ulong*)(CPG_BASE + 0x0080))

#define CPG_PLLECR_PLL3ST			(0x00000800)
#define CPG_PLLECR_PLL2ST			(0x00000400)
#define CPG_PLLECR_PLL1ST			(0x00000200)
#define CPG_PLLECR_PLL0ST			(0x00000100)
#define CPG_PLLECR_PLL3E			(0x00000008)
#define CPG_PLLECR_PLL2E			(0x00000004)
#define CPG_PLLECR_PLL1E			(0x00000002)
#define CPG_PLLECR_CLKOFF			(0x00000000)
#define CPG_PLLECR_CLKON_BOOT		(CPG_PLLECR_PLL1E | CPG_PLLECR_PLL0E)
#define CPG_PLLECR_STOFF			(CPG_PLLECR_PLL3ST | CPG_PLLECR_PLL2ST | CPG_PLLECR_PLL1ST | CPG_PLLECR_PLL0ST)	/* wait each bit is 0 */
#define CPG_PLLECR_STON_BOOT		(CPG_PLLECR_PLL1ST | CPG_PLLECR_PLL0ST)											/* wait each bit is 1 */

#define CPG_FRQCRB_KICK				(0x80000000)	/* wait each bit is 0 */
#define CPG_FRQCRB_BOOTCLK			(0x90231350)
#define CPG_PLL0CR_STC				(0x1E000000)	/* Multiplication Ratio : x31 */
#define CPG_PLL1CR_STC				(0x17100000)	/* Multiplication Ratio : x48 */
#define CPG_MPCKCR_CKSEL_EXTAL2	(0x00000080)


/**
 * SetUsbDownload - The processing to set to USB download
 * @return        - none
 */
void SetUsbDownload(void)
{
	/* Set Standby Flag Register0 */
	*STBCHRB0 |= STBCHRB0_USB_DOWNLOAD;
}


/**
 * SetBootClock - Boot clock setting processing
 * @return      - none
 */
void SetBootClock(void)
{
	ulong reg32;
	*CPG_PLLECR = CPG_PLLECR_CLKOFF;

	/* wait PLL status off */
	while (1)
	{
		reg32 = *CPG_PLLECR;
		if ((reg32 &= CPG_PLLECR_STOFF) == 0)
		{
			break;
		}
	}

	/* wait FRQCRB KICK bit is 0 */
	while (1)
	{
		reg32 = *CPG_FRQCRB;
		if ((reg32 &= CPG_FRQCRB_KICK) == 0)
		{
			break;
		}
	}

	if (CHIP_VERSION() == CHIP_RMU2_ES10) {
		*CPG_PLL0CR = CPG_PLL0CR_STC;
		*CPG_PLL1CR = CPG_PLL1CR_STC;
	} else {
		*CPG_PLL0CR = 0x2D000000;	// CPG_PLL0CR_X46
		*CPG_PLL1CR = 0x17100000;
	}

	*CPG_FRQCRB = CPG_FRQCRB_BOOTCLK;

	/* wait FRQCRB KICK bit is 0 */
	while (1)
	{
		reg32 = *CPG_FRQCRB;
		if ((reg32 &= CPG_FRQCRB_KICK) == 0)
		{
			break;
		}
	}
#if 0
	*CPG_PLLECR = CPG_PLLECR_CLKON_BOOT;
#endif /* 0 */
	*CPG_PLLECR = 0x0000000B;

	/* wait PLL status on */
	while (1)
	{
		reg32 = *CPG_PLLECR;
		if ((reg32 &= 0x00000B00) == 0x00000B00)
		{
			break;
		}
	}
}

/**
 * SetUsbBootClock - Usb Boot clock setting processing
 * @return      - none
 */
void SetUsbBootClock(void)
{
	SetBootClock();

	*CPG_MPCKCR |= CPG_MPCKCR_CKSEL_EXTAL2;
}
