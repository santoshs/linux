/* sbsc.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef __SBSC_H__
#define __SBSC_H__

#include "com_type.h"


/* SBSC register address */
#define SBSC_BASE			(0xFE000000ul)
#define SBSC_SDPHYTCR1		((volatile ulong *)(0xFE401004))
#define SBSC_SDCR0A			((volatile ulong *)(SBSC_BASE + 0x400008))
#define SBSC_SDCR1A			((volatile ulong *)(SBSC_BASE + 0x40000C))
#define SBSC_SDPCRA			((volatile ulong *)(SBSC_BASE + 0x400010))
#define SBSC_RTCSRA			((volatile ulong *)(SBSC_BASE + 0x400020))
#define SBSC_RTCORA			((volatile ulong *)(SBSC_BASE + 0x400028))
#define SBSC_RTCORHA		((volatile ulong *)(SBSC_BASE + 0x40002C))
#define SBSC_SDWCRC0A		((volatile ulong *)(SBSC_BASE + 0x400040))
#define SBSC_SDWCRC1A		((volatile ulong *)(SBSC_BASE + 0x400044))
#define SBSC_SDWCR00A		((volatile ulong *)(SBSC_BASE + 0x400048))
#define SBSC_SDWCR01A		((volatile ulong *)(SBSC_BASE + 0x40004C))
#define SBSC_SDWCR10A		((volatile ulong *)(SBSC_BASE + 0x400050))
#define SBSC_SDWCR11A		((volatile ulong *)(SBSC_BASE + 0x400054))
#define SBSC_SDPDCR0A		((volatile ulong *)(SBSC_BASE + 0x400058))
#define SBSC_SDWCR2A		((volatile ulong *)(SBSC_BASE + 0x400060))
#define SBSC_SDWCRC2A		((volatile ulong *)(SBSC_BASE + 0x400064))
#define SBSC_ZQCCRA			((volatile ulong *)(SBSC_BASE + 0x400068))
#define SBSC_SDMRACR0A		((volatile ulong *)(SBSC_BASE + 0x400084))
#define SBSC_SDMRTMPCRA		((volatile ulong *)(SBSC_BASE + 0x40008C))
#define SBSC_SDMRTMPMSKA	((volatile ulong *)(SBSC_BASE + 0x400094))
#define SBSC_SDGENCNTA		((volatile ulong *)(SBSC_BASE + 0x40009C))
#define SBSC_SDDRVCR0A		((volatile ulong *)(SBSC_BASE + 0x4000B4))
#define SBSC_DLLCNT0A		((volatile ulong *)(SBSC_BASE + 0x400354))
#define SBSC_SDMRA_00000	((volatile ulong *)(SBSC_BASE + 0x500000))
#define SBSC_SDMRA_04000	((volatile ulong *)(SBSC_BASE + 0x504000))
#define SBSC_SDMRA_C0000	((volatile ulong *)(SBSC_BASE + 0x5C0000))

#define SBSC_SDPTCR0A		((volatile ulong *)(SBSC_BASE + 0x400100))
#define SBSC_SDPTCR1A		((volatile ulong *)(SBSC_BASE + 0x400104))
#define SBSC_SDPTCR2A		((volatile ulong *)(SBSC_BASE + 0x400108))
#define SBSC_SDPTCR3A		((volatile ulong *)(SBSC_BASE + 0x40010C))
#define SBSC_SDPTCR4A		((volatile ulong *)(SBSC_BASE + 0x400110))
#define SBSC_SDPTCR5A		((volatile ulong *)(SBSC_BASE + 0x400114))
#define SBSC_SDPTCR6A		((volatile ulong *)(SBSC_BASE + 0x400118))
#define SBSC_SDPTCR7A		((volatile ulong *)(SBSC_BASE + 0x40011C))
#define SBSC_SAPRICR0		((volatile ulong *)(SBSC_BASE + 0x400240))
#define SHBMCTR2			((volatile ulong *)(0xE6244010))

/* SBSC register setting value(Initialize) */
#define SBSC_DLLCNT0A_DATA			(0x00000002)
#define SBSC_SDGENCNTA_100NS		(0x00000004)
#define SBSC_SDPHYTCR1_DATA			(0x9A005802)	
#define SBSC_SDCR0A_DATA			(0xBCC9015A)	/* 4Gbit setting with COL=10bits */
#define SBSC_SDCR1A_DATA			(0x0001005A)
#define SBSC_SDWCRC0A_DATA			(0x30654112)
#define SBSC_SDWCRC0A_DATA_520		(0x70A84115)
#define SBSC_SDWCRC0A_DATA_390		(0xD0874114)
#define SBSC_SDWCRC1A_DATA			(0x21106124)
#define SBSC_SDWCRC1A_DATA_520		(0x00BB2E19)
#define SBSC_SDWCRC1A_DATA_390		(0x32189B36)
#define SBSC_SDWCRC2A_DATA			(0x005D170C)
#define SBSC_SDWCRC2A_DATA_520		(0x4320CD48)
#define SBSC_SDWCRC2A_DATA_390		(0x008C2313)
#define SBSC_SDWCR00A_DATA			(0x20A14404)
#define SBSC_SDWCR00A_DATA_520		(0x51528909)
#define SBSC_SDWCR00A_DATA_390		(0x31020707)
#define SBSC_SDWCR01A_DATA			(0x000F0108)
#define SBSC_SDWCR01A_DATA_520		(0x001F030E)
#define SBSC_SDWCR01A_DATA_390		(0x0017020B)
#define SBSC_SDWCR10A_DATA			(0x20A14404)
#define SBSC_SDWCR10A_DATA_520		(0x51528909)
#define SBSC_SDWCR10A_DATA_390		(0x31020707)
#define SBSC_SDWCR11A_DATA			(0x000F0108)
#define SBSC_SDWCR11A_DATA_520		(0x001F030E)
#define SBSC_SDWCR11A_DATA_390		(0x0017020B)

#define SBSC_SDPDCR0A_PL3DLLCNT_OFF	(0xFF000100)
#define SBSC_SDDRVCR0A_DATA			(0x05555555)
#define SBSC_SDWCR2A_DATA			(0xF0000000)
#define SBSC_SDPCRA_DATA			(0x00000080)
#define SBSC_SDPCRA_DATA_CKELV		(0x000000100)
#define SBSC_SDGENCNTA_200US		(0x00001A0A)
#define SBSC_SDMRACR0A_DATA1		(0x00000602)
#define SBSC_SDMRACR0A_DATA_520		(0x0000602)
#define SBSC_SDMRACR0A_DATA_390		(0x0000402)
#define SBSC_SDMRACR0A_RESET		(0x0000003F)
#define SBSC_SDMRACR0A_ZQ_CAL		(0x0000FF0A)
#define SBSC_SDMRACR0A_MR1_SET		(0x00002201)
#define SBSC_SDMRACR0A_MR2_SET		(0x00000202)
#define SBSC_SDMRACR0A_MR3_SET		(0x00000403)
#define SBSC_SDMRA_DONE				(0x00000000)
#define SBSC_SDGENCNTA_10US			(0x0000012D)
#define SBSC_SDGENCNTA_1US			(0x00000022)
#define SBSC_SDMRTMPCRA_DATA		(0x88800004)
#define SBSC_SDMRTMPMSKA_DATA		(0x00000004)
#define SBSC_RTCORA_DATA			(0xA55A0032)
#define SBSC_RTCORHA_DATA			(0xA55A000C)
#define SBSC_RTCSRA_DATA			(0xA55A2048)
#define SBSC_SDCR0A_RFSH			(0x00000800)
#define SBSC_SDCR0A_PDOWN3			(0x0000C000)
#define SBSC_SDCR0A_BIT31			(0x80000000)
#define SBSC_SDCR1A_RMODE			(0x00000400)
#define SBSC_SDCR1A_PDOWN3			(0x0000C000)
#define SBSC_ZQCCRA_DATA			(0xFFF20000)

#define SBSC_SDPTCR_A_DATA			(0x00000800)
#define SBSC_SDPTCR_SCR_FP			(0x00040000)

/* SW semaphore (SD-RAM) */
#define SW_SEMAPHORE_1				((volatile ulong *)(0x47FBFC00))	


/* Function Prototypes */
void SBSC_Init_ES10(void);
void SBSC_Init_520Mhz(void);
void SBSC_Init_390Mhz(void);
void SBSC_Init(void);

void SBSC_SW_Semaphore(void);


#endif /* __SBSC_H__ */
