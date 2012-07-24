/*
 * cp1_m05_tmu.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */
#ifndef __H_CP1_M5_TMU_
#define __H_CP1_M5_TMU_

/***************************************************************/
/* INCLUDE FILE                                                */
/***************************************************************/
#include "eos_system.h"				/* EOS-EVM-TP SYSTEM header */

/***************************************************************/
/* PUBLIC CONSTANT DEFINITON                                   */
/*-------------------------------------------------------------*/
/* Timer Unit(TMU)                                           */
/*-------------------------------------------------------------*/
/* TMU1_BADR:0xFFF60000                                        */
/* TMU2_BADR:0xFFF70000                                        */
/***************************************************************/
/* INDEX_M05_TMU */
#define U1R_TSTR1				(*(VU1 *)(TMU1_BADR+0x0004))			/* Timer start register 1 */
#define U4R_TCOR1_0				(*(VU4 *)(TMU1_BADR+0x0008))			/* Timer constant register1_0 */
#define U4R_TCNT1_0				(*(VU4 *)(TMU1_BADR+0x000C))			/* Timer counter1_0 */
#define U2R_TCR1_0				(*(VU2 *)(TMU1_BADR+0x0010))			/* Timer control register1_0 */
#define U4R_TCOR1_1				(*(VU4 *)(TMU1_BADR+0x0014))			/* Timer constant register1_1 */
#define U4R_TCNT1_1				(*(VU4 *)(TMU1_BADR+0x0018))			/* Timer counter1_1 */
#define U2R_TCR1_1				(*(VU2 *)(TMU1_BADR+0x001C))			/* Timer control register1_1 */
#define U4R_TCOR1_2				(*(VU4 *)(TMU1_BADR+0x0020))			/* Timer constant register1_2 */
#define U4R_TCNT1_2				(*(VU4 *)(TMU1_BADR+0x0024))			/* Timer counter1_2 */
#define U2R_TCR1_2				(*(VU2 *)(TMU1_BADR+0x0028))			/* Timer control register1_2 */
#define U1R_TSTR2				(*(VU1 *)(TMU2_BADR+0x0004))			/* Timer start register 2 */
#define U4R_TCOR2_0				(*(VU4 *)(TMU2_BADR+0x0008))			/* Timer constant register2_0 */
#define U4R_TCNT2_0				(*(VU4 *)(TMU2_BADR+0x000C))			/* Timer counter2_0 */
#define U2R_TCR2_0				(*(VU2 *)(TMU2_BADR+0x0010))			/* Timer control register2_0 */
#define U4R_TCOR2_1				(*(VU4 *)(TMU2_BADR+0x0014))			/* Timer constant register2_1 */
#define U4R_TCNT2_1				(*(VU4 *)(TMU2_BADR+0x0018))			/* Timer counter2_1 */
#define U2R_TCR2_1				(*(VU2 *)(TMU2_BADR+0x001C))			/* Timer control register2_1 */
#define U4R_TCOR2_2				(*(VU4 *)(TMU2_BADR+0x0020))			/* Timer constant register2_2 */
#define U4R_TCNT2_2				(*(VU4 *)(TMU2_BADR+0x0024))			/* Timer counter2_2 */
#define U2R_TCR2_2				(*(VU2 *)(TMU2_BADR+0x0028))			/* Timer control register2_2 */

/*------------------------------------*/
/* CH0                                */
/*------------------------------------*/
#define	U1R_TSTR0				(*(VU1 *)(TMU1_BADR+0x0004))		/* Timer Start register0 */
#define	U4R_TCOR0_0				(*(VU4 *)(TMU1_BADR+0x0008))		/* Timer Constant register 0_0 */
#define	U4R_TCNT0_0				(*(VU4 *)(TMU1_BADR+0x000C))		/* Timer Counter register  0_0 */
#define	U2R_TCR0_0				(*(VU2 *)(TMU1_BADR+0x0010))		/* Timer Control register  0_0 */
#define	U4R_TCOR0_1				(*(VU4 *)(TMU1_BADR+0x0014))		/* Timer Constant register 0_1 */
#define	U4R_TCNT0_1				(*(VU4 *)(TMU1_BADR+0x0018))		/* Timer Counter register  0_1 */
#define	U2R_TCR0_1				(*(VU2 *)(TMU1_BADR+0x001C))		/* Timer Control register  0_1 */
#define	U4R_TCOR0_2				(*(VU4 *)(TMU1_BADR+0x0020))		/* Timer Constant register 0_2 */
#define	U4R_TCNT0_2				(*(VU4 *)(TMU1_BADR+0x0024))		/* Timer Counter register  0_2 */
#define	U2R_TCR0_2				(*(VU2 *)(TMU1_BADR+0x0028))		/* Timer Control register  0_2 */


#endif  /* __H_CP1_M5_TMU_ */

