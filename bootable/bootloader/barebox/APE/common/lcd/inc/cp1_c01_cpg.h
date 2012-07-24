/*
 * cp1_c01_cpg.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */
#ifndef __H_CP1_C1_CPG_
#define __H_CP1_C1_CPG_

/***************************************************************/
/* INCLUDE FILE                                                */
/***************************************************************/
#include "eos_system.h"				/* EOS-EVM-TP SYSTEM header */

/***************************************************************/
/* PUBLIC CONSTANT DEFINITON                                   */
/*-------------------------------------------------------------*/
/* Clock Pulse Generator (CPG)                                */
/*-------------------------------------------------------------*/
/* CPG_BADR:0xE6150000                                         */
/***************************************************************/
/* INDEX_C01_CPG */
#define U4R_FRQCRA				(*(VU4 *)(CPG_BADR+0x0000))			/* frequency control registerA */
#define U4R_FRQCRB				(*(VU4 *)(CPG_BADR+0x0004))			/* frequency control registerB */
#define U4R_FRQCRD				(*(VU4 *)(CPG_BADR+0x00E4))			/* frequency control registerD */
#define U4R_VCLKCR1				(*(VU4 *)(CPG_BADR+0x0008))			/* Video clock frequency control register1 */
#define U4R_VCLKCR2				(*(VU4 *)(CPG_BADR+0x000C))			/* Video clock frequency control register2 */
#define U4R_VCLKCR3				(*(VU4 *)(CPG_BADR+0x001C))			/* Video clock frequency control register3 */
#define U4R_VCLKCR4				(*(VU4 *)(CPG_BADR+0x0014))			/* Video clock frequency control register4 */
#define U4R_ZBCKCR				(*(VU4 *)(CPG_BADR+0x0010))			/* BSC clock frequency control register */
#define U4R_SD0CKCR				(*(VU4 *)(CPG_BADR+0x0074))			/* SDHI0 clock frequency control register */
#define U4R_SD1CKCR				(*(VU4 *)(CPG_BADR+0x0078))			/* SDHI1 clock frequency control register */
#define U4R_SD2CKCR				(*(VU4 *)(CPG_BADR+0x007C))			/* SDHI2 clock frequency control register */
#define U4R_FSIACKCR			(*(VU4 *)(CPG_BADR+0x0018))			/* FSIA clock frequency control register */
#define U4R_FSIBCKCR			(*(VU4 *)(CPG_BADR+0x0090))			/* FSIB clock frequency control register */
#define U4R_MPCKCR				(*(VU4 *)(CPG_BADR+0x0080))			/* MP clock frequency control register */
#define U4R_SPUACKCR			(*(VU4 *)(CPG_BADR+0x0084))			/* SPUA clock frequency control register */
#define U4R_SLIMBCKCR			(*(VU4 *)(CPG_BADR+0x0088))			/* SLIMB clock frequency control register */
#define U4R_HSICKCR				(*(VU4 *)(CPG_BADR+0x008C))			/* HSI clock frequency control register */
#define U4R_DSITCKCR			(*(VU4 *)(CPG_BADR+0x0060))			/* MIPI DSI TX Link clock frequency control register */
#define U4R_DSI0PCKCR			(*(VU4 *)(CPG_BADR+0x0064))			/* MIPI DSI PHY0 clock frequency control register */
#define U4R_DSI0PHYCR			(*(VU4 *)(CPG_BADR+0x006C))			/* DSI PHY0 PLL control register */
#define U4R_MPMODE				(*(VU4 *)(CPG_BADR+0x00CC))			/* Music player mode register */
#define U4R_RTSTBCR				(*(VU4 *)(CPG_BADR+0x0020))			/* Realtime standby control register */
#define U4R_SYSTBCR				(*(VU4 *)(CPG_BADR+0x0024))			/* System standby control register */
#define U4R_PLLECR				(*(VU4 *)(CPG_BADR+0x00D0))			/* PLL Enable Register */
#define U4R_PLL0CR				(*(VU4 *)(CPG_BADR+0x00D8))			/* PLL0 control register */
#define U4R_PLL1CR				(*(VU4 *)(CPG_BADR+0x0028))			/* PLL1 control register */
#define U4R_PLL2CR				(*(VU4 *)(CPG_BADR+0x002C))			/* PLL2 control register */
#define U4R_PLL3CR				(*(VU4 *)(CPG_BADR+0x00DC))			/* PLL3 control register */
#define U4R_PLL0STPCR			(*(VU4 *)(CPG_BADR+0x00F0))			/* PLL0 stop condition register */
#define U4R_PLL1STPCR			(*(VU4 *)(CPG_BADR+0x00C8))			/* PLL1 stop condition register */
#define U4R_PLL2STPCR			(*(VU4 *)(CPG_BADR+0x00F8))			/* PLL2 stop condition register */
#define U4R_PLL3STPCR			(*(VU4 *)(CPG_BADR+0x00FC))			/* PLL3 stop condition register */
#define U4R_MSTPSR0				(*(VU4 *)(CPG_BADR+0x0030))			/* Module stop status register0 */
#define U4R_MSTPSR1				(*(VU4 *)(CPG_BADR+0x0038))			/* Module stop status register1 */
#define U4R_MSTPSR2				(*(VU4 *)(CPG_BADR+0x0040))			/* Module stop status register2 */
#define U4R_MSTPSR3				(*(VU4 *)(CPG_BADR+0x0048))			/* Module stop status register3 */
#define U4R_MSTPSR4				(*(VU4 *)(CPG_BADR+0x004C))			/* Module stop status register4 */
#define U4R_MSTPSR5				(*(VU4 *)(CPG_BADR+0x003C))			/* Module stop status register5 */
#define U4R_RMSTPCR0			(*(VU4 *)(CPG_BADR+0x0110))			/* Realtime module stop control register0 */
#define U4R_RMSTPCR1			(*(VU4 *)(CPG_BADR+0x0114))			/* Realtime module stop control register1 */
#define U4R_RMSTPCR2			(*(VU4 *)(CPG_BADR+0x0118))			/* Realtime module stop control register2 */
#define U4R_RMSTPCR3			(*(VU4 *)(CPG_BADR+0x011C))			/* Realtime module stop control register3 */
#define U4R_RMSTPCR4			(*(VU4 *)(CPG_BADR+0x0120))			/* Realtime module stop control register4 */
#define U4R_RMSTPCR5			(*(VU4 *)(CPG_BADR+0x0124))			/* Realtime module stop control register5 */
#define U4R_SMSTPCR0			(*(VU4 *)(CPG_BADR+0x0130))			/* System module stop control register0 */
#define U4R_SMSTPCR1			(*(VU4 *)(CPG_BADR+0x0134))			/* System module stop control register1 */
#define U4R_SMSTPCR2			(*(VU4 *)(CPG_BADR+0x0138))			/* System module stop control register2 */
#define U4R_SMSTPCR3			(*(VU4 *)(CPG_BADR+0x013C))			/* System module stop control register3 */
#define U4R_SMSTPCR4			(*(VU4 *)(CPG_BADR+0x0140))			/* System module stop control register4 */
#define U4R_SMSTPCR5			(*(VU4 *)(CPG_BADR+0x0144))			/* System module stop control register5 */
#define U4R_MMSTPCR0			(*(VU4 *)(CPG_BADR+0x0150))			/* Modem module stop control register0 */
#define U4R_MMSTPCR1			(*(VU4 *)(CPG_BADR+0x0154))			/* Modem module stop control register1 */
#define U4R_MMSTPCR2			(*(VU4 *)(CPG_BADR+0x0158))			/* Modem module stop control register2 */
#define U4R_MMSTPCR3			(*(VU4 *)(CPG_BADR+0x015C))			/* Modem module stop control register3 */
#define U4R_MMSTPCR4			(*(VU4 *)(CPG_BADR+0x0160))			/* Modem module stop control register4 */
#define U4R_MMSTPCR5			(*(VU4 *)(CPG_BADR+0x0164))			/* Modem module stop control register5 */
#define U4R_SRCR0				(*(VU4 *)(CPG_BADR+0x80A0))			/* Software reset register0 */
#define U4R_SRCR1				(*(VU4 *)(CPG_BADR+0x80A8))			/* Software reset register1 */
#define U4R_SRCR2				(*(VU4 *)(CPG_BADR+0x80B0))			/* Software reset register2 */
#define U4R_SRCR3				(*(VU4 *)(CPG_BADR+0x80B8))			/* Software reset register3 */
#define U4R_SRCR4				(*(VU4 *)(CPG_BADR+0x80BC))			/* Software reset register4 */
#define U4R_SRCR5				(*(VU4 *)(CPG_BADR+0x80C4))			/* Software reset register5 */
#define U4R_ASTAT				(*(VU4 *)(CPG_BADR+0x0054))			/* Status register */
#define U4R_CKSCR				(*(VU4 *)(CPG_BADR+0x00C0))			/* Clock source control register */
#define U4R_SEQMON				(*(VU4 *)(CPG_BADR+0x0108))			/* Sequence Monitor Register */
#define U4R_VREFCR				(*(VU4 *)(CPG_BADR+0x00EC))			/* VREF control register */
#define U4R_WUPCR				(*(VU4 *)(CPG_BADR+0x1010))			/* System-CPU Wake Up Control Register */
#define U4R_SRESCR				(*(VU4 *)(CPG_BADR+0x1018))			/* System-CPU Software Reset Control Register */
#define U4R_PCLKCR				(*(VU4 *)(CPG_BADR+0x1020))			/* System-CPU PERIPHCLK Control Register */
#define U4R_PSTR				(*(VU4 *)(CPG_BADR+0x1040))			/* System-CPU Power Status Register */
#define U4R_CPU0RFR				(*(VU4 *)(CPG_BADR+0x1104))			/* System-CPU CPU0 Reset Flag Register */
#define U4R_CPU1RFR				(*(VU4 *)(CPG_BADR+0x1114))			/* System-CPU CPU1 Reset Flag Register */
#define U4R_SPCTR				(*(VU4 *)(CPG_BADR+0x01A4))			/* System-CPU Power Control Threshold register */
#define U4R_SPCMMR				(*(VU4 *)(CPG_BADR+0x01AC))			/* System-CPU Power Control Max mode register */
#define U4R_SPCDMR				(*(VU4 *)(CPG_BADR+0x01B0))			/* System-CPU Power Control Delta mode register */

#endif  /* __H_CP1_C1_CPG_ */

