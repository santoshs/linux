/*
 * cp1_s23_tpu.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */
#ifndef __H_CP1_S23_TPU_
#define __H_CP1_S23_TPU_

/***************************************************************/
/* INCLUDE FILE                                                */
/***************************************************************/
#include "eos_system.h"				/* EOS-EVM-TP SYSTEM header */

/**********************************************************************/
/*** 16-Bit Timer Pulse Unit (TPU)                                  ***/
/*--------------------------------------------------------------------*/
/*** TPU_BADR:0xE6600000  Mirror:0xE7600000                         ***/
/**********************************************************************/
/* INDEX_23_TPU */
#define	U2R_TPU0_TSTR			(*(VU2 *)(TPU_BADR+0x0000))			/* Timer start register */
#define	U2R_TPU0_TCR0			(*(VU2 *)(TPU_BADR+0x0010))			/* Timer control register 0 */
#define	U2R_TPU0_TMDR0			(*(VU2 *)(TPU_BADR+0x0014))			/* Timer mode register 0 */
#define	U2R_TPU0_TIOR0			(*(VU2 *)(TPU_BADR+0x0018))			/* Timer I/O control register 0 */
#define	U2R_TPU0_TIER0			(*(VU2 *)(TPU_BADR+0x001C))			/* Timer interrupt enable register 0 */
#define	U2R_TPU0_TSR0			(*(VU2 *)(TPU_BADR+0x0020))			/* Timer status register 0 */
#define	U2R_TPU0_TCNT0			(*(VU2 *)(TPU_BADR+0x0024))			/* Timer counter 0 */
#define	U2R_TPU0_TGR0A			(*(VU2 *)(TPU_BADR+0x0028))			/* Timer general register 0A */
#define	U2R_TPU0_TGR0B			(*(VU2 *)(TPU_BADR+0x002C))			/* Timer general register 0B */
#define	U2R_TPU0_TGR0C			(*(VU2 *)(TPU_BADR+0x0030))			/* Timer general register 0C */
#define	U2R_TPU0_TGR0D			(*(VU2 *)(TPU_BADR+0x0034))			/* Timer general register 0D */
#define	U2R_TPU0_TCR1			(*(VU2 *)(TPU_BADR+0x0050))			/* Timer control register 1 */
#define	U2R_TPU0_TMDR1			(*(VU2 *)(TPU_BADR+0x0054))			/* Timer mode register 1 */
#define	U2R_TPU0_TIOR1			(*(VU2 *)(TPU_BADR+0x0058))			/* Timer I/O control register 1 */
#define	U2R_TPU0_TIER1			(*(VU2 *)(TPU_BADR+0x005C))			/* Timer interrupt enable register 1 */
#define	U2R_TPU0_TSR1			(*(VU2 *)(TPU_BADR+0x0060))			/* Timer status register 1 */
#define	U2R_TPU0_TCNT1			(*(VU2 *)(TPU_BADR+0x0064))			/* Timer counter 1 */
#define	U2R_TPU0_TGR1A			(*(VU2 *)(TPU_BADR+0x0068))			/* Timer general register 1A */
#define	U2R_TPU0_TGR1B			(*(VU2 *)(TPU_BADR+0x006C))			/* Timer general register 1B */
#define	U2R_TPU0_TGR1C			(*(VU2 *)(TPU_BADR+0x0070))			/* Timer general register 1C */
#define	U2R_TPU0_TGR1D			(*(VU2 *)(TPU_BADR+0x0074))			/* Timer general register 1D */
#define	U2R_TPU0_TCR2			(*(VU2 *)(TPU_BADR+0x0090))			/* Timer control register 2 */
#define	U2R_TPU0_TMDR2			(*(VU2 *)(TPU_BADR+0x0094))			/* Timer mode register 2 */
#define	U2R_TPU0_TIOR2			(*(VU2 *)(TPU_BADR+0x0098))			/* Timer I/O control register 2 */
#define	U2R_TPU0_TIER2			(*(VU2 *)(TPU_BADR+0x009C))			/* Timer interrupt enable register 2 */
#define	U2R_TPU0_TSR2			(*(VU2 *)(TPU_BADR+0x00A0))			/* Timer status register 2 */
#define	U2R_TPU0_TCNT2			(*(VU2 *)(TPU_BADR+0x00A4))			/* Timer counter 2 */
#define	U2R_TPU0_TGR2A			(*(VU2 *)(TPU_BADR+0x00A8))			/* Timer general register 2A */
#define	U2R_TPU0_TGR2B			(*(VU2 *)(TPU_BADR+0x00AC))			/* Timer general register 2B */
#define	U2R_TPU0_TGR2C			(*(VU2 *)(TPU_BADR+0x00B0))			/* Timer general register 2C */
#define	U2R_TPU0_TGR2D			(*(VU2 *)(TPU_BADR+0x00B4))			/* Timer general register 2D */
#define	U2R_TPU0_TCR3			(*(VU2 *)(TPU_BADR+0x00D0))			/* Timer control register 3 */
#define	U2R_TPU0_TMDR3			(*(VU2 *)(TPU_BADR+0x00D4))			/* Timer mode register 3 */
#define	U2R_TPU0_TIOR3			(*(VU2 *)(TPU_BADR+0x00D8))			/* Timer I/O control register 3 */
#define	U2R_TPU0_TIER3			(*(VU2 *)(TPU_BADR+0x00DC))			/* Timer interrupt enable register 3 */
#define	U2R_TPU0_TSR3			(*(VU2 *)(TPU_BADR+0x00E0))			/* Timer status register 3 */
#define	U2R_TPU0_TCNT3			(*(VU2 *)(TPU_BADR+0x00E4))			/* Timer counter 3 */
#define	U2R_TPU0_TGR3A			(*(VU2 *)(TPU_BADR+0x00E8))			/* Timer general register 3A */
#define	U2R_TPU0_TGR3B			(*(VU2 *)(TPU_BADR+0x00EC))			/* Timer general register 3B */
#define	U2R_TPU0_TGR3C			(*(VU2 *)(TPU_BADR+0x00F0))			/* Timer general register 3C */
#define	U2R_TPU0_TGR3D			(*(VU2 *)(TPU_BADR+0x00F4))			/* Timer general register 3D */
#define	U2R_TPU0_TMIR			(*(VU2 *)(TPU_BADR+0x0100))			/* Motor control configuration register */
#define	U2R_TPU0_TMRR			(*(VU2 *)(TPU_BADR+0x0104))			/* Motor deceleration (stop) transition configuration register */
#define	U2R_TPU0_TMSR			(*(VU2 *)(TPU_BADR+0x0108))			/* Motor control status register */
#define	U2R_TPU0_TMMPR0			(*(VU2 *)(TPU_BADR+0x0110))			/* Motor operation pattern storage register 0 */
#define	U2R_TPU0_TMMPR1			(*(VU2 *)(TPU_BADR+0x0114))			/* Motor operation pattern storage register 1 */
#define	U2R_TPU0_TMSPR0			(*(VU2 *)(TPU_BADR+0x0118))			/* Motor stop pattern storage register 0 */
#define	U2R_TPU0_TMSPR1			(*(VU2 *)(TPU_BADR+0x011C))			/* Motor stop pattern storage register 1 */
#define	U2R_TPU0_TMOPR			(*(VU2 *)(TPU_BADR+0x0120))			/* Motor output pattern storage register */
#define	U2R_TPU0_TMASR			(*(VU2 *)(TPU_BADR+0x0130))			/* Motor acceleration the number of steps register */
#define	U2R_TPU0_TMTSR			(*(VU2 *)(TPU_BADR+0x0134))			/* Motor normal the number of steps register */
#define	U2R_TPU0_TMRSR			(*(VU2 *)(TPU_BADR+0x0138))			/* Motor deceleration the number of steps register */
#define	U2R_TPU0_TMSCR			(*(VU2 *)(TPU_BADR+0x0140))			/* Motor control sequence counter register */
#define	U2R_TPU0_TMTCR			(*(VU2 *)(TPU_BADR+0x0144))			/* Motor control normal counter register */

#endif  /* __H_CP1_S23_TPU_ */

