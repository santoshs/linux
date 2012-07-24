/* cpg.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef __CPG_H__
#define __CPG_H__

#include "com_type.h"
#include "compile_option.h"

/* Constant of CPU clock */
typedef enum {
	CPU_CLOCK_1456MHZ,         /* CPU clock 1456MHz - ES2.0 only */
	CPU_CLOCK_1196MHZ,         /* CPU clock 1196MHz - ES2.0 only */
	CPU_CLOCK_988MHZ,          /* CPU clock 988MHz  - ES1.0 only */
	CPU_CLOCK_26MHZ,		   /* CPU clock 26MHz */
	CPU_CLOCK_104MHZ,		   /* CPU clock 104MHz */
} CPU_CLOCK;


/* CPG register address */
#define CPG_BASE		(0xE6150000ul)
#define CPG_FRQCRA		((volatile ulong *)(CPG_BASE + 0x0000))
#define CPG_FRQCRB		((volatile ulong *)(CPG_BASE + 0x0004))
#define CPG_VCLKCR1		((volatile ulong *)(CPG_BASE + 0x0008))
#define CPG_VCLKCR2		((volatile ulong *)(CPG_BASE + 0x000C))
#define CPG_ZBCKCR		((volatile ulong *)(CPG_BASE + 0x0010))
#define CPG_FSIACKCR	((volatile ulong *)(CPG_BASE + 0x0018))
#define CPG_VCLKCR3		((volatile ulong *)(CPG_BASE + 0x0014))
#define CPG_VCLKCR4		((volatile ulong *)(CPG_BASE + 0x001C))
#define CPG_VCLKCR5		((volatile ulong *)(CPG_BASE + 0x0034))
#define CPG_PLL1CR		((volatile ulong *)(CPG_BASE + 0x0028))
#define CPG_PLL2CR		((volatile ulong *)(CPG_BASE + 0x002C))
#define CPG_DSITCKCR	((volatile ulong *)(CPG_BASE + 0x0060))
#define CPG_DSI0PCKCR	((volatile ulong *)(CPG_BASE + 0x0064))
#define CPG_DSI1PCKCR	((volatile ulong *)(CPG_BASE + 0x0068))
#define CPG_DSI0PHYCR	((volatile ulong *)(CPG_BASE + 0x006C))
#define CPG_SD0CKCR		((volatile ulong *)(CPG_BASE + 0x0074))
#define CPG_SD1CKCR		((volatile ulong *)(CPG_BASE + 0x0078))
#define CPG_MPCKCR		((volatile ulong *)(CPG_BASE + 0x0080))
#define CPG_SPUACKCR	((volatile ulong *)(CPG_BASE + 0x0084))
#define CPG_SPU2VCKCR	((volatile ulong *)(CPG_BASE + 0x0094))
#define CPG_SLIMBCKCR	((volatile ulong *)(CPG_BASE + 0x0088))
#define CPG_HSICKCR		((volatile ulong *)(CPG_BASE + 0x008C))
#define CPG_FSIBCKCR	((volatile ulong *)(CPG_BASE + 0x0090))
#define CPG_M4CKCR		((volatile ulong *)(CPG_BASE + 0x0098))
#define CPG_PLL1STPCR	((volatile ulong *)(CPG_BASE + 0x00C8))
#define CPG_MPMODE		((volatile ulong *)(CPG_BASE + 0x00CC))
#define CPG_PLLECR		((volatile ulong *)(CPG_BASE + 0x00D0))
#define CPG_PLL0CR		((volatile ulong *)(CPG_BASE + 0x00D8))
#define CPG_PLL3CR		((volatile ulong *)(CPG_BASE + 0x00DC))
#define CPG_FRQCRD		((volatile ulong *)(CPG_BASE + 0x00E4))
#define CPG_VREFCR		((volatile ulong *)(CPG_BASE + 0x00EC))
#define CPG_PLL0STPCR	((volatile ulong *)(CPG_BASE + 0x00F0))
#define CPG_PLL2STPCR	((volatile ulong *)(CPG_BASE + 0x00F8))
#define CPG_PLL3STPCR	((volatile ulong *)(CPG_BASE + 0x00FC))
#define CPG_RMSTPCR0	((volatile ulong *)(CPG_BASE + 0x0110))
#define CPG_RMSTPCR1	((volatile ulong *)(CPG_BASE + 0x0114))
#define CPG_RMSTPCR2	((volatile ulong *)(CPG_BASE + 0x0118))
#define CPG_RMSTPCR3	((volatile ulong *)(CPG_BASE + 0x011C))
#define CPG_RMSTPCR4	((volatile ulong *)(CPG_BASE + 0x0120))
#define CPG_RMSTPCR5	((volatile ulong *)(CPG_BASE + 0x0124))
#define CPG_RMSTPCR6	((volatile ulong *)(CPG_BASE + 0x0128))
#define CPG_SMSTPCR0	((volatile ulong *)(CPG_BASE + 0x0130))
#define CPG_SMSTPCR1	((volatile ulong *)(CPG_BASE + 0x0134))
#define CPG_SMSTPCR2	((volatile ulong *)(CPG_BASE + 0x0138))
#define CPG_SMSTPCR3	((volatile ulong *)(CPG_BASE + 0x013C))
#define CPG_SMSTPCR4	((volatile ulong *)(CPG_BASE + 0x0140))
#define CPG_SMSTPCR5	((volatile ulong *)(CPG_BASE + 0x0144))
#define CPG_SMSTPCR6	((volatile ulong *)(CPG_BASE + 0x0148))

#define CPG_MMSTPCR0	((volatile ulong *)(CPG_BASE + 0x0150))
#define CPG_MMSTPCR1	((volatile ulong *)(CPG_BASE + 0x0154))
#define CPG_MMSTPCR2	((volatile ulong *)(CPG_BASE + 0x0158))
#define CPG_MMSTPCR3	((volatile ulong *)(CPG_BASE + 0x015C))
#define CPG_MMSTPCR4	((volatile ulong *)(CPG_BASE + 0x0160))
#define CPG_MMSTPCR5	((volatile ulong *)(CPG_BASE + 0x0164))
#define CPG_MMSTPCR6	((volatile ulong *)(CPG_BASE + 0x0168))

#define CPG_MSTPSR0		((volatile ulong *)(CPG_BASE + 0x0030))
#define CPG_MSTPSR1		((volatile ulong *)(CPG_BASE + 0x0038))
#define CPG_MSTPSR2		((volatile ulong *)(CPG_BASE + 0x0040))
#define CPG_MSTPSR3		((volatile ulong *)(CPG_BASE + 0x0048))
#define CPG_MSTPSR4		((volatile ulong *)(CPG_BASE + 0x004C))
#define CPG_MSTPSR5		((volatile ulong *)(CPG_BASE + 0x003C))

/* Bit mask defination for UBC*/
#define CPG_MSTP017  	(0x00020000)

#define CPG_PCLKCR		((volatile ulong *)(CPG_BASE + 0x1020))
#define CPG_SRCR0		((volatile ulong *)(CPG_BASE + 0x80A0))
#define CPG_SRCR1		((volatile ulong *)(CPG_BASE + 0x80A8))
#define CPG_SRCR2		((volatile ulong *)(CPG_BASE + 0x80B0))
#define CPG_SRCR3		((volatile ulong *)(CPG_BASE + 0x80B8))
#define CPG_SRCR4		((volatile ulong *)(CPG_BASE + 0x80BC))

/* System-CPU power control threshold */
#define CPG_SPCTR		((volatile ulong *)(CPG_BASE + 0x01A4))
#define CPG_SPCR2		((volatile ulong *)(CPG_BASE + 0x0158))	
/* System-CPU Power Control Max mode */
#define CPG_SPCMMR		((volatile ulong *)(CPG_BASE + 0x01AC))
/* System-CPU Power Control Delta mode */
#define CPG_SPCDMR		((volatile ulong *)(CPG_BASE + 0x01B0))
/* value setting */
#define CPG_SPCTR_VALUE		0x11AC0699
#define CPG_SPCR2_VALUE		0xE7FC63FF
#define CPG_SPCMMR_VALUE	0xFFFFFFFF
#define CPG_SPCDMR_VALUE	0xBFE2E10F

/* CPG register setting value */
#define CPG_PLLECR_PLL3ST		(0x00000800)
#define CPG_PLLECR_PLL2ST		(0x00000400)
#define CPG_PLLECR_PLL1ST		(0x00000200)
#define CPG_PLLECR_PLL0ST		(0x00000100)
#define CPG_PLLECR_PLL3E		(0x00000008)
#define CPG_PLLECR_PLL2E		(0x00000004)
#define CPG_PLLECR_PLL1E		(0x00000002)
#define CPG_PLLECR_PLL0E		(0x00000001)
#define CPG_PLLECR_PLL22E		(0x00000010)

#define CPG_PLLECR_CLKOFF		(0x00000008)	
#define CPG_PLLECR_CLKON		(CPG_PLLECR_PLL3E | CPG_PLLECR_PLL1E | CPG_PLLECR_PLL0E)
#define CPG_PLLECR_STOFF		(CPG_PLLECR_PLL2ST | CPG_PLLECR_PLL1ST | CPG_PLLECR_PLL0ST)	/* wait each bit is 0 - no SBSC */
#define CPG_PLLECR_STON			(CPG_PLLECR_PLL1ST | CPG_PLLECR_PLL0ST)						/* wait each bit is 1 */

#define CPG_FRQCRA_1456MHZ		(0x00324534)
#define CPG_FRQCRA_1196MHZ		(0x00324534)
#define CPG_FRQCRA_988MHZ		(0x00334530)
#define CPG_FRQCRA_104MHZ       (0x001345C0)
#define CPG_FRQCRA_26MHZ        (0x001345C0)

#define CPG_FRQCRB_DATA			(0x00231350)
#define CPG_FRQCRB_KICK			(0x80000000)	/* Wait each bit is 0 */
#define CPG_FRQCRD_260MHz		(0x0000000B)
#define CPG_FRQCRD_32MHz		(0x00000014)
#define CPG_FRQCRD_DIV2			(0x00008000)
#define CPG_FRQCRD_DIV32		(0x00000016)

/*check PCLK */
#define CPG_PCLKCR_1456MHZ		(0x0000000C)
#define CPG_PCLKCR_1196MHZ		(0x0000000A)
#define CPG_PCLKCR_988MHZ		(0x00000009)
#define CPG_PCLKCR_104MHZ       (0x00000003)
#define CPG_PCLKCR_26MHZ        (0x00000003)

#define CPG_SETCLOCK_208MHZ     (0x00000102)
#define CPG_SETCLOCK_156MHZ     (0x00000103)
#define CPG_SETCLOCK_104MHZ     (0x00000105)
#define CPG_SETCLOCK_78MHZ     	(0x00000107)
#define CPG_SETCLOCK_26MHZ      (0x00000117)
#define CPG_SETCLOCK_24MHZ      (0x00000119)
#define CPG_SETCLOCK_9800KHZ    (0x0000013F)


#define CPG_VCLKCR4_DATA		(0x0000012F)

#define CPG_ZBCKCR_BSC			(0x00000002)
#define CPG_SD0CKCR_DATA		(0x00000080)
#define CPG_SD1CKCR_DATA		(0x00000180)
#define CPG_FSIACKCR_DATA		(0x0000003F)
#define CPG_FSIBCKCR_DATA		(0x0000013F)	/* disable FSIB clock */
#define CPG_MPCKCR_48MHz		(0x0000000C)
#define CPG_SPUACKCR_DATA		(0x00000003)
#define CPG_HSICKCR_DATA		(0x00000002)
#define CPG_DSITCKCR_DATA		(0x00000007)
#define CPG_DSI0PCKCR_DATA		(0x00000317)
#define CPG_DSI0PHYCR_DATA		(0x2A80000D)
#define CPG_DSI1PCKCR_DATA		(0x0000110D)

#define CPG_M4CKCR_DATA			(0x00000189)
#define CPG_PLL0CR_X56			(0x37000000)
#define CPG_PLL0CR_X46			(0x2D000000)
#define CPG_PLL0CR_X38			(0x25000000)
#define CPG_PLL0CR_X4			(0x03000000)
#define CPG_PLL0CR_X1			(0x00000000)

#define CPG_PLL1CR_DATA			(0x17100000)	/* 1248MHz */
#define CPG_PLL2CR_DATA			(0x27000000)
#define CPG_PLL3CR_1040MHZ		(0x27000000)
#define CPG_PLL3CR_780MHZ		(0x1D000000)
#define CPG_PLL3CR_520MHZ		(0x13000000)
#define CPG_PLL0STPCR_DATA		(0x00080000)
#define CPG_PLL1STPCR_DATA		(0x00012040)
#define CPG_PLL2STPCR_DATA		(0x00012000)
#define CPG_PLL3STPCR_DATA		(0x00010000)
#define CPG_MPMODE_DATA			(0x00000000)	/* EXESMSK=1 */
#define CPG_VREFCR_DATA			(0x0003028A)

/* Boot_Init */
#define CPG_MSTPCR_DFT_DATA		(0xFFFFFFFF)

#define CPG_RMSTPCR0_DATA		(0xE4608003)
#define CPG_RMSTPCR1_DATA		(0xFFFFDFFF)
#define CPG_RMSTPCR2_DATA		(0x3197E3FE)
#define CPG_RMSTPCR4_DATA		(0x9F803ECC)
#define CPG_RMSTPCR5_DATA		(0xFFFFF1FF)

#define CPG_SMSTPCR0_DATA		(0xE42F8087)
#define CPG_SMSTPCR1_DATA		(0xFFFFF7FF)
#define CPG_SMSTPCR2_DATA		(0x3197E3FE)
#define CPG_SMSTPCR4_DATA		(0x908038CC)
#define CPG_SMSTPCR5_DATA		(0xFBFFF07F)

#define CPG_MMSTPCR0_DATA		(0xE46F8087)
#define CPG_MMSTPCR2_DATA		(0x3197E3FE)
#define CPG_MMSTPCR4_DATA		(0x9F803ECC)
#define CPG_MMSTPCR5_DATA		(0xFFFFF1FF)

#define CPG_SRCR2_ALL			(0x00000000)
#define CPG_SMSTPCR2_218		(0x00040000)	/* SYS-DMAC */
#define CPG_SRCR2_218			(0x00040000)	/* SYS-DMAC */
#define CPG_SMSTPCR3_USB		(0x00400000)	/* SMSTPCR3 - Module Stop Status 322 (Controls clock supply to USB) */
#define CPG_SRCR3_USB			(0x00400000)	/* SRCR3 - Software Reset bit 322 (Issues the reset to USB) */

/* I2C setting */
#define CPG_SMSTPCR1_116		(0x00010000)	/* SMSTPCR1 - Module Stop bit 116 (Controls clock supply to IIC0) */
#define CPG_SRCR1_SRT116		(0x00010000)	/* SRCR1 - Software Reset bit 116 (Issues the reset to IIC0) */
#define CPG_SMSTPCR3_323		(0x00800000)	/* SMSTPCR3 - Module Stop bit 323 (Controls clock supply to IIC1) */
#define CPG_SRCR3_SRT323		(0x00800000)	/* SRCR3 - Software Reset bit 323 (Issues the reset to IIC1) */
#define CPG_SMSTPCR0_001		(0x00000002)	/* SMSTPCR0 - Module Stop bit 001 (Controls clock supply to IIC2) */
#define CPG_SRCR0_SRT001		(0x00000002)	/* SRCR0 - Software Reset bit 001 (Issues the reset to IIC2) */
#define CPG_SMSTPCR4_410		(0x00000400)	/* SMSTPCR4 - Module Stop bit 410 (Controls clock supply to IIC4) */
#define CPG_SRCR4_SRT410		(0x00000400)	/* SRCR4 - Software Reset bit 410 (Issues the reset to IIC4) */
#define CPG_SMSTPCR4_409		(0x00002000)	/* SMSTPCR1 - Module Stop bit 409 (Controls clock supply to IIC5) */
#define CPG_SRCR4_SRT409		(0x00002000)	/* SRCR1 - Software Reset bit 409 (Issues the reset to IIC5) */
#define CPG_SMSTPCR4_412		(0x00001000)	/* SMSTPCR1 - Module Stop bit 412 (Controls clock supply to IICMS) */
#define CPG_SRCR4_SRT412		(0x00001000)	/* SRCR1 - Software Reset bit 412 (Issues the reset to IICMS) */

/* Temporary for debug */
#define CPG_MSTP019		        (0x00080000)	/* H-UDI */
#define CPG_MSTP018		        (0x00040000)	/* RT_CPU debug module1 */

/* HPB register address */
#define HPB_BASE			(0xE6000000ul)
#define HPB_HPBCTRL1		((volatile ulong *)(HPB_BASE + 0x1014))
#define HPB_HPBCTRL2		((volatile ulong *)(HPB_BASE + 0x1018))
#define HPB_HPBCTRL4		((volatile ulong *)(HPB_BASE + 0x1024))
#define HPB_HPBCTRL5		((volatile ulong *)(HPB_BASE + 0x1028))
#define HPB_HPBCTRL7		((volatile ulong *)(HPB_BASE + 0x1034))

#define HPB_SMGPIOTIME		((volatile ulong *)(HPB_BASE + 0x1828))
#define HPB_SMCMT2TIME		((volatile ulong *)(HPB_BASE + 0x1848))
#define HPB_SMCPGATIME		((volatile ulong *)(HPB_BASE + 0x1858))
#define HPB_SMSYSCTIME		((volatile ulong *)(HPB_BASE + 0x1878))


/* HPB register setting value */
#define HPB_HPBCTRL1_DATA		(0x0FFF0FFF)
#define HPB_HPBCTRL2_DATA		(0x0FFFFFFF)
#define HPB_HPBCTRL4_DATA		(0x0FFF00FF)
#define HPB_HPBCTRL5_DATA		(0x00000000)
#define HPB_HPBCTRL7_DATA		(0xFFFFFFFF)
/* Semaphore time-out is supposed to be 100us */
#define HPB_SMGPIOTIME_DATA		(0x00000514)
#define HPB_SMCMT2TIME_DATA		(0x00000514)
#define HPB_SMCPGATIME_DATA		(0x00000514)
#define HPB_SMSYSCTIME_DATA		(0x00000514)

/* The setting value of boot */
#define CPG_SMSTPCR1_IIC0	(1 << 16)
#define CPG_SMSTPCR2_SCIFA0	(1 << 4)
#define CPG_SMSTPCR3_CMT1	(1 << 29)
#define CPG_SMSTPCR3_SDHI0	(1 << 14)
#define CPG_SMSTPCR3_MMCIF	(1 << 15)

/* Mask Clock Stop */
#define CPG_CLOCK_STOP (0x00000100)

/* Function Prototypes */
void CPG_Set_Clock(CPU_CLOCK clock);
/* void CPG_Stop_Clock(void); */
void CPG_InterconnectRAM_Enable(void);
void CPG_2nd_Setting(void);
void CPG_Stop_VCK3clock(void);


#endif /* __CPG_H__ */
