/*
 * Copyright (C) 2011 Renesas Electronics Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */

#include <config.h>
#include <version.h>

#include <asm/macro.h>
#include <asm/arch/macro.h>
#include <asm/arch/hardware.h>

/* WDT Symbols */
#define RWTCSRA0	(RWDT_BASE + 0x04)
#define SWTCSRA0	(SWDT_BASE + 0x04)

/* etc Symbols */
#define CCCR		(HPB_BASE + 0x101C)
#define SMGPIOTIME	(HPB_BASE + 0x1828)
#define SMCPGTIME	(HPB_BASE + 0x1858)
#define SMSYSCTIME	(HPB_BASE + 0x1878)
#define HPBCTRL1	(HPB_BASE + 0x1014)
#define HPBCTRL2	(HPB_BASE + 0x1018)
#define HPBCTRL4	(HPB_BASE + 0x1024)
#define HPBCTRL5	(HPB_BASE + 0x1028)
#define HPBCTRL6	(HPB_BASE + 0x1030)
#define HPBCTRL7	(HPB_BASE + 0x1034)

/* CPG Symbols */
#define FRQCRA		(CPG_BASE + 0x0000)
#define FRQCRB		(CPG_BASE + 0x0004)
#define FRQCRD		(CPG_BASE + 0x00E4)
#define VCLKCR1		(CPG_BASE + 0x0008)
#define VCLKCR2		(CPG_BASE + 0x000C)
#define VCLKCR3		(CPG_BASE + 0x001C)
#define VCLKCR4		(CPG_BASE + 0x0014)
#define ZBCKCR		(CPG_BASE + 0x0010)
#define SD0CKCR		(CPG_BASE + 0x0074)
#define SD1CKCR		(CPG_BASE + 0x0078)
#define FSIACKCR	(CPG_BASE + 0x0018)
#define FSIBCKCR	(CPG_BASE + 0x0090)
#define MPCKCR		(CPG_BASE + 0x0080)
#define SPUACKCR	(CPG_BASE + 0x0084)
#define HSICKCR		(CPG_BASE + 0x008C)
#define DSITCKCR	(CPG_BASE + 0x0060)
#define DSI0PCKCR	(CPG_BASE + 0x0064)
#define DSI0PHYCR	(CPG_BASE + 0x006C)
#define MPMODE		(CPG_BASE + 0x00CC)
#define PLLECR		(CPG_BASE + 0x00D0)
#define PLL0CR		(CPG_BASE + 0x00D8)
#define PLL1CR		(CPG_BASE + 0x0028)
#define PLL2CR		(CPG_BASE + 0x002C)
#define PLL3CR		(CPG_BASE + 0x00DC)
#define PLL0STPCR	(CPG_BASE + 0x00F0)
#define PLL1STPCR	(CPG_BASE + 0x00C8)
#define PLL2STPCR	(CPG_BASE + 0x00F8)
#define PLL3STPCR	(CPG_BASE + 0x00FC)
#define RMSTPCR0	(CPG_BASE + 0x0110)
#define RMSTPCR1	(CPG_BASE + 0x0114)
#define RMSTPCR2	(CPG_BASE + 0x0118)
#define RMSTPCR3	(CPG_BASE + 0x011C)
#define RMSTPCR4	(CPG_BASE + 0x0120)
#define RMSTPCR5	(CPG_BASE + 0x0124)
#define SMSTPCR0	(CPG_BASE + 0x0130)
#define SMSTPCR1	(CPG_BASE + 0x0134)
#define SMSTPCR2	(CPG_BASE + 0x0138)
#define SMSTPCR3	(CPG_BASE + 0x013C)
#define SMSTPCR4	(CPG_BASE + 0x0140)
#define SMSTPCR5	(CPG_BASE + 0x0144)
#define MMSTPCR0	(CPG_BASE + 0x0150)
#define MMSTPCR1	(CPG_BASE + 0x0154)
#define MMSTPCR2	(CPG_BASE + 0x0158)
#define MMSTPCR3	(CPG_BASE + 0x015C)
#define MMSTPCR4	(CPG_BASE + 0x0160)
#define MMSTPCR5	(CPG_BASE + 0x0164)
#define SRCR0		(CPG_BASE + 0x80A0)
#define SRCR1		(CPG_BASE + 0x80A8)
#define SRCR2		(CPG_BASE + 0x80B0)
#define SRCR3		(CPG_BASE + 0x80B8)
#define SRCR4		(CPG_BASE + 0x80BC)
#define SRCR5		(CPG_BASE + 0x80C4)
#define CKSCR		(CPG_BASE + 0x00C0)
#define VREFCR		(CPG_BASE + 0x00EC)
#define PCLKCR		(CPG_BASE + 0x1020)

/* SBSC1 Symbols */
#define SDCR0A		(SBSC1_BASE + 0x0008)
#define SDCR1A		(SBSC1_BASE + 0x000C)
#define SDCR0SA		(SBSC1_BASE + 0x0018)
#define SDCR1SA		(SBSC1_BASE + 0x001C)
#define SDWCRC0A	(SBSC1_BASE + 0x0040)
#define SDWCRC1A	(SBSC1_BASE + 0x0044)
#define SDWCRC2A	(SBSC1_BASE + 0x0064)
#define SDWCR00A	(SBSC1_BASE + 0x0048)
#define SDWCR01A	(SBSC1_BASE + 0x004C)
#define SDWCR10A	(SBSC1_BASE + 0x0050)
#define SDWCR11A	(SBSC1_BASE + 0x0054)
#define SDWCR2A		(SBSC1_BASE + 0x0060)
#define SDPDCR0A	(SBSC1_BASE + 0x0058)
#define EBMCRA		(SBSC1_BASE + 0x0014)
#define SDPCRA		(SBSC1_BASE + 0x0010)
#define RTCSRA		(SBSC1_BASE + 0x0020)
#define RTCNTA		(SBSC1_BASE + 0x0024)
#define RTCORA		(SBSC1_BASE + 0x0028)
#define RTCORHA		(SBSC1_BASE + 0x002C)
#define RTCORSA		(SBSC1_BASE + 0x0030)
#define RTCORSHA	(SBSC1_BASE + 0x0034)
#define RFCRA		(SBSC1_BASE + 0x0038)
#define BSTRCNTA	(SBSC1_BASE + 0x003C)
#define ZQCCRA		(SBSC1_BASE + 0x0068)
#define ZQRFCRA		(SBSC1_BASE + 0x006C)
#define SDMRA1		(0xFE500000)
#define SDMRA2		(0xFE5C0000)
#define SDMRA3		(0xFE504000)
#define SDMRACR0A	(SBSC1_BASE + 0x0084)
#define SDMRACR1A	(SBSC1_BASE + 0x0088)
#define SDMRTMPCRA	(SBSC1_BASE + 0x008C)
#define SDMRTMPMONA	(SBSC1_BASE + 0x0090)
#define SDMRTMPMSKA	(SBSC1_BASE + 0x0094)
#define SDMRTMPINTMSKA	(SBSC1_BASE + 0x0098)
#define SDGENCNTA	(SBSC1_BASE + 0x009C)
#define SDDRVCR0A	(SBSC1_BASE + 0x00B4)
#define SDPTDIVCR0A	(SBSC1_BASE + 0x00E0)
#define SDPTDIVCR1A	(SBSC1_BASE + 0x00E4)
#define SDPTDIVCR2A	(SBSC1_BASE + 0x00E8)
#define SDPTDIVCR3A	(SBSC1_BASE + 0x00E0)
#define SDPTDIVCR4A	(SBSC1_BASE + 0x00F0)
#define SDPTDIVCR5A	(SBSC1_BASE + 0x00F4)
#define SDPTDIVCR6A	(SBSC1_BASE + 0x00F8)
#define SDPTCR0A	(SBSC1_BASE + 0x0100)
#define SDPTCR1A	(SBSC1_BASE + 0x0104)
#define SDPTCR2A	(SBSC1_BASE + 0x0108)
#define SDPTCR3A	(SBSC1_BASE + 0x010C)
#define SDPTCR4A	(SBSC1_BASE + 0x0110)
#define SDPTCR5A	(SBSC1_BASE + 0x0114)
#define SDPTCR6A	(SBSC1_BASE + 0x0118)
#define SDPTCR7A	(SBSC1_BASE + 0x011C)
#define DLLCNT0A	(SBSC1_BASE + 0x0354)
#define SBSCMON0A	(SBSC1_BASE + 0x0358)
#define INTCR0A		(SBSC1_BASE + 0x0200)
#define INTCLR0A	(SBSC1_BASE + 0x0204)
#define INTCRS0A	(SBSC1_BASE + 0x0208)
#define INTCLRS0A	(SBSC1_BASE + 0x020C)
#define SDSCRM		(SBSC1_BASE + 0x0210)
#define SDSCRMLOCK	(SBSC1_BASE + 0x0214)
#define SDADMATCH0	(SBSC1_BASE + 0x0220)
#define SDADMATCH1	(SBSC1_BASE + 0x0224)
#define SDADMSK0	(SBSC1_BASE + 0x0228)
#define SDADMSK1	(SBSC1_BASE + 0x022C)
#define SDMONCON0	(SBSC1_BASE + 0x0230)
#define SDMONCON1	(SBSC1_BASE + 0x0234)

.globl lowlevel_init

lowlevel_init:
	/* Enable I-Cache */
	mov	r0, #0
	mcr	p15, 0, r0, c7, c5, 0		@ invalidate I cache
	mcr	p15, 0, r0, c7, c5, 6		@ flush BTAC/BTB
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, r0, #0x1800			@ ...IZ...........
	mcr	p15, 0, r0, c1, c0, 0

	/* WDT */
	write32		RWTCSRA0, 0xA5A5A507	@ halt
	write32		SWTCSRA0, 0xA5A5A507	@ halt

	/* MSTP - should be set up prior to CPG settings */
	write32		RMSTPCR0, 0xE46A8007	@ RT-CPU debug, ICB
	write32		RMSTPCR1, 0xFFFFDFFF	@ MERAM
	write32		RMSTPCR2, 0x3195E1FE
	write32		RMSTPCR3, 0xFFFFFFFF
	write32		RMSTPCR4, 0x800016CC
	write32		RMSTPCR5, 0xFFFFF1FF
	write32		SMSTPCR0, 0xE42F8087	@ INTCS
	write32		SMSTPCR1, 0xFFFFFFFF
	write32		SMSTPCR2, 0x3195E1FE
	write32		SMSTPCR3, 0xFFFFFFFF
	write32		SMSTPCR4, 0x800016CC
	write32		SMSTPCR5, 0xFBFFF07F	@ Public ROM, INTCM, IRQC
	write32		MMSTPCR0, 0xE46F8087
	write32		MMSTPCR1, 0xFFFFFFFF
	write32		MMSTPCR2, 0x3195E1FE
	write32		MMSTPCR3, 0xFFFFFFFF
	write32		MMSTPCR4, 0x800016CC
	write32		MMSTPCR5, 0xFFFFF1FF

	write32		SMGPIOTIME,	0x00000514
	write32		SMCPGTIME,	0x00000514
	write32		SMSYSCTIME,	0x00000514

	/* CPG */
#ifdef CONFIG_NOR_BOOT
	write32		PLLECR,		0x00000000
	polling		PLLECR,		0x00000F00, 0x00000000
#else
	write32		PLLECR,		0x00000009
	polling		PLLECR,		0x00000600, 0x00000000
#endif
	write32		FRQCRA,		0x00334530
	write32		FRQCRD,		0x00000000
	write32		FRQCRB,		0x80231350
	polling		FRQCRB,		0x80000000, 0x00000000
#ifdef CONFIG_NOR_BOOT
	write32		PLL0CR,		0x25000000	@ 26M x38 = 988M
#endif
	write32		PLL1CR,		0x17100000	@ 26M x48 = 1248M
	write32		PLL3CR,		0x13000000	@ 26M x20 = 520M
	write32		PLL0STPCR,	0x00080000
	write32		PLL1STPCR,	0x00012040
	write32		PLL3STPCR,	0x00012000
	write32		ZBCKCR,		0x00000002	@ PLL1 x1/2, x1/6, supplied
	write32		SD0CKCR,	0x00000180	@ PLL1 x1/13, x1/1
	write32		SD1CKCR,	0x00000180	@ PLL1 x1/13, x1/1
	write32		VCLKCR1,	0x00000119	@ PLL1 x1/2, x1/26
	write32		VCLKCR2,	0x00000119	@ PLL1 x1/2, x1/26
	write32		VCLKCR3,	0x00006100	@ main clock, x1/1
	write32		VCLKCR4,	0x00006100	@ main clock, x1/1
	write32		FSIACKCR,	0x0000013F	@ PLL1 x1/2, x1/64
	write32		FSIBCKCR,	0x0000013F	@ PLL1 x1/2, x1/64
	write32		MPCKCR,		0x00000F0C	@ PLL1 x1/2, x1/13
	write32		SPUACKCR,	0x00000105	@ PLL1 x1/2, x1/6
	write32		HSICKCR,	0x00000102	@ PLL1 x1/2, x1/6
	write32		DSITCKCR,	0x00000107	@ PLL1 x1/2, x1/8
	write32		DSI0PCKCR,	0x003F0317	@ PLL1 x1/2, x1/24
	write32		DSI0PHYCR,	0x00000000	@ ? (initial value)
	write32		MPMODE,		0x00000000	@ ? (initial value)
	write32		VREFCR,		0x0003028A	@ ? (initial value)
	write32		PCLKCR,		0x00000009	@ ZCLK x1/10
	write32		PLLECR,		0x0000000B
	polling		PLLECR,		0x00000B00, 0x00000B00

	/* SBSC1 */
	write32		HPBCTRL6,	0x00000002	@ address space setting
	write32		FRQCRD,		0x00000014	@ PLL3 x1/16
	setbits		FRQCRB,		0x80000000
	polling		FRQCRB,		0x80000000, 0x00000000
	write32		SDCR0A,		0xBCC9015A
	write32		SDCR1A,		0x0001005A
	write32		SDWCRC0A,	0x30654112
	write32		SDWCRC1A,	0x21106124
	write32		SDWCRC2A,	0x005D170C
	write32		SDWCR00A,	0x20A14404
	write32		SDWCR01A,	0x000F0108
	write32		SDWCR10A,	0x20A14404
	write32		SDWCR11A,	0x000F0108
	write32		SDDRVCR0A,	0x05555555
	write32		SDWCR2A,	0xF0000000
	setbits		SDPCRA,		0x00000080
	polling		SDPCRA,		0x00000080, 0x00000080
	write32		SDGENCNTA,	0x00001A0A	@ ???usec
	polling		SDGENCNTA,	0xFFFFFFFF, 0x00000000
	write32		SDMRACR0A,	0x0000003F
	write32		SDMRA1,		0x00000000
	write32		SDGENCNTA,	0x0000012D	@ ???usec
	polling		SDGENCNTA,	0xFFFFFFFF, 0x00000000
	write32		SDMRACR0A,	0x0000FF0A
	write32		SDMRA3,		0x00000000
	write32		SDGENCNTA,	0x00000022	@ ???usec
	polling		SDGENCNTA,	0xFFFFFFFF, 0x00000000
	write32		SDMRACR0A,	0x00002201	@ BL4
	write32		SDMRA1,		0x00000000
	write32		SDMRACR0A,	0x00000202	@ RL4/WL2
	write32		SDMRA1,		0x00000000
	write32		SDMRACR0A,	0x00000403	@ DS=4, 60-ohm
	write32		SDMRA1,		0x00000000
	write32		SDMRA2,		0x00000000
	write32		SDMRTMPCRA,	0x88800004
	write32		SDMRTMPMSKA,	0x00000004
	write32		RTCORA,		0xA55A0032
	write32		RTCORHA,	0xA55A000C
	write32		RTCSRA,		0xA55A2048
	setbits		SDCR0A,		0x00000800
	write32		ZQCCRA,		0xFFF20000

#ifdef CONFIG_NOR_BOOT
	/* Memory protection area */
#if 0
	write32		SDPTDIVCR0A,
	write32		SDPTDIVCR1A,
	write32		SDPTDIVCR2A,
	write32		SDPTDIVCR3A,
	write32		SDPTDIVCR4A,
	write32		SDPTDIVCR5A,
	write32		SDPTDIVCR6A,
#endif
	write32		SDPTCR0A,	0x00000000	@ clear SCR_FP
	write32		SDPTCR1A,	0x00000000
	write32		SDPTCR2A,	0x00000000
	write32		SDPTCR3A,	0x00000000
	write32		SDPTCR4A,	0x00000000
	write32		SDPTCR5A,	0x00000000
	write32		SDPTCR6A,	0x00000000
	write32		SDPTCR7A,	0x00000000
#endif

	write32		FRQCRD,		0x0000000B	@ PLL3 x1/2
	setbits		FRQCRB,		0x80000000
	polling		FRQCRB,		0x80000000, 0x00000000

	/* back to arch calling code */
	bx	lr			@ return to caller

	.pool
