/*
 * cp1_m12_dsi.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */
#ifndef __H_CP1_M12_DSI_
#define __H_CP1_M12_DSI_

/***************************************************************/
/* INCLUDE FILE                                                */
/***************************************************************/
#include "eos_system.h"				/* EOS-EVM-TP SYSTEM header */

/***************************************************************/
/* PUBLIC CONSTANT DEFINITON                                   */
/*-------------------------------------------------------------*/
/* Display Serial Interface(DSI)                             */
/*-------------------------------------------------------------*/
/* DSI_BADR:0xFEAB0000                                         */
/***************************************************************/
/* INDEX_M12_DSI */
#define	U4R_SYSCTRL				(*(VU4 *)(DSI_BADR+0x0000))			/* System Control Register */
#define	U4R_SYSCONF				(*(VU4 *)(DSI_BADR+0x0004))			/* System Configuration Register */
#define	U4R_TIMSET0				(*(VU4 *)(DSI_BADR+0x0008))			/* Transition Timing Parameter Setting Register0 */
#define	U4R_PTYPESET0			(*(VU4 *)(DSI_BADR+0x0010))			/* Transmission Packet Type Setting Register0 */
#define	U4R_PTYPESET1			(*(VU4 *)(DSI_BADR+0x0014))			/* Transmission Packet Type Setting Register1 */
#define	U4R_RESREQSET0			(*(VU4 *)(DSI_BADR+0x0018))			/* Response Request Setting Register0 */
#define	U4R_RESREQSET1			(*(VU4 *)(DSI_BADR+0x001C))			/* Response Request Setting Register1 */
#define	U4R_HSTTOVSET			(*(VU4 *)(DSI_BADR+0x0020))			/* HS Transmission Time-out Setting Register */
#define	U4R_LPRTOVSET			(*(VU4 *)(DSI_BADR+0x0024))			/* LP Reception Time-out Setting Register */
#define	U4R_TATOVSET			(*(VU4 *)(DSI_BADR+0x0028))			/* Turn Around Time-out Setting Register */
#define	U4R_PRTOVSET			(*(VU4 *)(DSI_BADR+0x002C))			/* Peripheral Reset Time-out Value Setting Register */
#define	U4R_DSICTRL				(*(VU4 *)(DSI_BADR+0x0030))			/* DSI-LINK Control Register */
#define	U4R_RPTYPESET0			(*(VU4 *)(DSI_BADR+0x0038))			/* Reception Packet Type Setting Register0 */
#define	U4R_RPTYPESET1			(*(VU4 *)(DSI_BADR+0x003C))			/* Reception Packet Type Setting Register1 */
#define	U4R_DSIS				(*(VU4 *)(DSI_BADR+0x0040))			/* DSI-LINK Status Register */
#define	U4R_DSIINT				(*(VU4 *)(DSI_BADR+0x0050))			/* DSI-LINK Interrupt Status Register */
#define	U4R_DSIINTE				(*(VU4 *)(DSI_BADR+0x0060))			/* DSI-LINK Interrupt Enable Register */
#define	U4R_PHYCTRL0			(*(VU4 *)(DSI_BADR+0x0070))			/* PHY Control Register0 */
#define	U4R_PHYCTRL1			(*(VU4 *)(DSI_BADR+0x0074))			/* PHY Control Register1 */
#define	U4R_TIMSET1				(*(VU4 *)(DSI_BADR+0x007C))			/* Transition Timing Parameter Setting Register1 */
#define	U4R_DTCTR				(*(VU4 *)(DSI_BADR+0x4000))			/* DSI-L-Bridge Control Register */
#define	U4R_DTST				(*(VU4 *)(DSI_BADR+0x4004))			/* DSI-L-Bridge Status Register */
#define	U4R_DTSTCL				(*(VU4 *)(DSI_BADR+0x4008))			/* DSI-L-Bridge Status Clear Register */
#define	U4R_DTIRQEN				(*(VU4 *)(DSI_BADR+0x400C))			/* DSI-L-Bridge Interrupt Enable Register */
#define	U4R_VMCTR1				(*(VU4 *)(DSI_BADR+0x4020))			/* Video Mode Control Register1 */
#define	U4R_VMCTR2				(*(VU4 *)(DSI_BADR+0x4024))			/* Video Mode Control Register2 */
#define	U4R_VMLEN1				(*(VU4 *)(DSI_BADR+0x4028))			/* Video Mode Data Length Register1 */
#define	U4R_VMLEN2				(*(VU4 *)(DSI_BADR+0x402C))			/* Video Mode Data Length Register2 */
#define	U4R_VMLEN3				(*(VU4 *)(DSI_BADR+0x4030))			/* Video Mode Data Length Register3 */
#define	U4R_VMLEN4				(*(VU4 *)(DSI_BADR+0x4034))			/* Video Mode Data Length Register4 */
#define	U4R_VMSDAT1				(*(VU4 *)(DSI_BADR+0x4038))			/* Video Mode Short Packet Data Register1 */
#define	U4R_VMSDAT2				(*(VU4 *)(DSI_BADR+0x403C))			/* Video Mode Short Packet Data Register2 */
#define	U4R_CMRCTR				(*(VU4 *)(DSI_BADR+0x4040))			/* Command Mode Reception Control Register */
#define	U4R_CMRDAT				(*(VU4 *)(DSI_BADR+0x4044))			/* Command Mode Reception Data Register */
#define	U4R_CMRHEAD				(*(VU4 *)(DSI_BADR+0x4048))			/* Command Mode Reception Header Register */
#define	U4R_CMRTEREQ			(*(VU4 *)(DSI_BADR+0x404C))			/* Tearing Effect Request Register */
#define	U4R_CMTLNGREQ0			(*(VU4 *)(DSI_BADR+0x4060))			/* Command Mode Transmission Long Packet Request CH0 Register */
#define	U4R_CMTLNGREQ1			(*(VU4 *)(DSI_BADR+0x4064))			/* Command Mode Transmission Long Packet Request CH1 Register */
#define	U4R_CMTLNGREQ2			(*(VU4 *)(DSI_BADR+0x4068))			/* Command Mode Transmission Long Packet Request CH2 Register */
#define	U4R_CMTLNGREQ3			(*(VU4 *)(DSI_BADR+0x406C))			/* Command Mode Transmission Long Packet Request CH3 Register */
#define	U4R_CMTSRTREQ			(*(VU4 *)(DSI_BADR+0x4070))			/* Command Mode Transmission Short Packet Request Register */
#define	U4R_CMTIADR0			(*(VU4 *)(DSI_BADR+0x4080))			/* Command Mode Transmission Initiator Address CH0 Register */
#define	U4R_CMTIADR1			(*(VU4 *)(DSI_BADR+0x4084))			/* Command Mode Transmission Initiator Address CH1 Register */
#define	U4R_CMTIADR2			(*(VU4 *)(DSI_BADR+0x4088))			/* Command Mode Transmission Initiator Address CH2 Register */
#define	U4R_CMTIADR3			(*(VU4 *)(DSI_BADR+0x408C))			/* Command Mode Transmission Initiator Address CH3 Register */
#define	U4R_CMTITTL0			(*(VU4 *)(DSI_BADR+0x4090))			/* Command Mode Transmission Initiator Total Data CH0 Register */
#define	U4R_CMTITTL1			(*(VU4 *)(DSI_BADR+0x4094))			/* Command Mode Transmission Initiator Total Data CH1 Register */
#define	U4R_CMTITTL2			(*(VU4 *)(DSI_BADR+0x4098))			/* Command Mode Transmission Initiator Total Data CH2 Register */
#define	U4R_CMTITTL3			(*(VU4 *)(DSI_BADR+0x409C))			/* Command Mode Transmission Initiator Total Data CH3 Register */
#define	U4R_CMTIADRI0			(*(VU4 *)(DSI_BADR+0x40A0))			/* Command Mode Transmission Initiator Address Increment CH0 Register */
#define	U4R_CMTIADRI1			(*(VU4 *)(DSI_BADR+0x40A4))			/* Command Mode Transmission Initiator Address Increment CH1 Register */
#define	U4R_CMTIADRI2			(*(VU4 *)(DSI_BADR+0x40A8))			/* Command Mode Transmission Initiator Address Increment CH2 Register */
#define	U4R_CMTIADRI3			(*(VU4 *)(DSI_BADR+0x40AC))			/* Command Mode Transmission Initiator Address Increment CH3 Register */
#define	U4R_CMTIRN0				(*(VU4 *)(DSI_BADR+0x40B0))			/* Command Mode Transmission Initiator Repeat Number CH0 Register */
#define	U4R_CMTIRN1				(*(VU4 *)(DSI_BADR+0x40B4))			/* Command Mode Transmission Initiator Repeat Number CH1Register */
#define	U4R_CMTIRN2				(*(VU4 *)(DSI_BADR+0x40B8))			/* Command Mode Transmission Initiator Repeat Number CH2 Register */
#define	U4R_CMTIRN3				(*(VU4 *)(DSI_BADR+0x40BC))			/* Command Mode Transmission Initiator Repeat Number CH3 Register */
#define	U4R_CMTLNGCTR0			(*(VU4 *)(DSI_BADR+0x40C0))			/* Command Mode Transmission Long Packet Control CH0 Register */
#define	U4R_CMTLNGCTR1			(*(VU4 *)(DSI_BADR+0x40C4))			/* Command Mode Transmission Long Packet Control CH1 Register */
#define	U4R_CMTLNGCTR2			(*(VU4 *)(DSI_BADR+0x40C8))			/* Command Mode Transmission Long Packet Control CH2 Register */
#define	U4R_CMTLNGCTR3			(*(VU4 *)(DSI_BADR+0x40CC))			/* Command Mode Transmission Long Packet Control CH3 Register */
#define	U4R_CMTSRTCTR			(*(VU4 *)(DSI_BADR+0x40D0))			/* Command Mode Transmission Short Packet Control Register */
#define	U4R_CMTLNGDT0			(*(VU4 *)(DSI_BADR+0x40E0))			/* Command Mode Transmission Long Packet Data Type CH0 Register */
#define	U4R_CMTLNGDT1			(*(VU4 *)(DSI_BADR+0x40E4))			/* Command Mode Transmission Long Packet Data Type CH1 Register */
#define	U4R_CMTLNGDT2			(*(VU4 *)(DSI_BADR+0x40E8))			/* Command Mode Transmission Long Packet Data Type CH2 Register */
#define	U4R_CMTLNGDT3			(*(VU4 *)(DSI_BADR+0x40EC))			/* Command Mode Transmission Long Packet Data  Type CH3 Register */
#define	U4R_CMTLNGMEM0			(*(VU4 *)(DSI_BADR+0x40F0))			/* Command Mode Transmission Long Packet Memory Write CH0 Register */
#define	U4R_CMTLNGMEM1			(*(VU4 *)(DSI_BADR+0x40F4))			/* Command Mode Transmission Long Packet Memory Write CH1 Register */
#define	U4R_CMTLNGMEM2			(*(VU4 *)(DSI_BADR+0x40F8))			/* Command Mode Transmission Long Packet Memory Write CH2 Register */
#define	U4R_CMTLNGMEM3			(*(VU4 *)(DSI_BADR+0x40FC))			/* Command Mode Transmission Long Packet Memory Write CH3 Register */

#endif  /* __H_CP1_M12_DSI_ */

