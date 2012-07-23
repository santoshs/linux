/*
 * timer_drv.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */
/***************************************************************/
/* INCLUDE FILE                                                */
/***************************************************************/
#include "eos_csw.h"					/* EOS CSW Header file */

#include "eos_stdio.h"					/* EOS-Standard Header */
#include "cp1_c01_cpg.h"				/* CPG REG Header      */
#include "cp1_m05_tmu.h"				/* TMU REG Header      */

#include "timer_drv.h"					/* Timer function header    */

/***************************************************************/
/* STATIC CONSTANT DEFINE                                      */
/***************************************************************/
/* if timer clock MP clk/4 = 48.0MHz/4	*/
#define U4S_COUNT_1MS	(0x00002EE0)    /* Number of CLK/1MS(1Count=83.3ns 1ms = 1000000ns) */
#define U4S_COUNT_1US	(0x0000000C)    /* Number of CLK/1MS(1Count=83.3ns 1us = 1000ns) */

#define U4S_TMU0_ON		(u4)0xFDFFFFFF	/* MSTPCR1 TMU0CLK ON  */
#define U4S_TMU0_OFF	(u4)0x02000000	/* MSTPCR1 TMU0CLK OFF */
#define U1S_STR0_ON		(u1)0x01		/* TSTR STR0 ON */
#define U1S_STR0_OFF	(u1)0xFE		/* TSTR STR0 OFF */
#define U2S_TCR_SET		(u2)0x0000		/* TCR Set value */
#define U2S_UNF			(u2)0x0100		/* UNF bit */
#define U2S_CLR_UNF		(u2)0xFEFF		/* UNF bit Clear */

/***************************************************************/
/* STATIC TYPEDEF ENUM                                         */
/***************************************************************/

/***************************************************************/
/* STATIC FUNCTION PROTOTYPE                                   */
/***************************************************************/

/***************************************************************/
/* PUBLIC FUNCTION PROTOTYPE                                   */
/***************************************************************/
void vog_timer_wait( u4 u4t_time, u1 u1t_option );
void vog_timer_ms_wait( u4 u4t_time );
void vog_timer_us_wait( u4 u4t_time );

/*************************************************************************************************/
/* Function name   : vog_timer_wait( u4 u4t_time, u1 u1t_option )                                    */
/* Input     : u4t_time(Wait time), u1t_option(0:MS_TIMER 1:US_TIMER)       		                 */
/* Return   : void                                              	                                 */
/* Processing : TimerWait polling								        	                         */
/* Date     : TMU0 Timer0 is used                                     	                             */
/*************************************************************************************************/
void vog_timer_wait( u4 u4t_time, u1 u1t_option )
{
	u4 u4t_cnt_time = U4G_ZERO;

	/* Setting of CPGA(Clock supply) */
	U4R_SMSTPCR1 &= U4S_TMU0_ON;	/* MSTPCR2 ;TMU0 ON */

	U1R_TSTR0   &= U1S_STR0_OFF;	/* Timer0 stop */
	U2R_TCR0_0   = U2S_TCR_SET;		/* clock=MP clk/4 = 48MHz/4 */


	if( U1G_TIME_MS == u1t_option )
	{
		/* The timer count value is operated with MS. */
		u4t_cnt_time = u4t_time * U4S_COUNT_1MS;
	}else
	{
		/* The timer count value is operated with US. */
		u4t_cnt_time = u4t_time * U4S_COUNT_1US;
	}

	/* Setting of timer count value */
	U4R_TCOR0_0 = u4t_cnt_time;
	U4R_TCNT0_0 = u4t_cnt_time;

	U1R_TSTR0  |= U1S_STR0_ON;		/* Timer0 start */

	/* Timer end waiting */
	while( !(U2R_TCR0_0 & U2S_UNF) );

	U1R_TSTR0  &= U1S_STR0_OFF;		/* Timer0 stop */
	U2R_TCR0_0 &= U2S_CLR_UNF;		/* UNF bit Clear */

}


/*************************************************************************************************/
/* Function name   : vog_timer_ms_wait( u4 u4t_time )                                                   */
/* Input     : u4t_time(Wait time)                                                                 */
/* Return   : void                                                                               */
/* Processing : Timer Wait polling                                     */
/*************************************************************************************************/
void vog_timer_ms_wait( u4 u4t_time )
{
	u4 u4t_cnt_time = U4G_ZERO;

	/* Setting of CPGA(Clock supply) */
	U4R_SMSTPCR1 &= U4S_TMU0_ON;	/* MSTPCR2 ;TMU0 ON */

	U1R_TSTR0   &= U1S_STR0_OFF;	/* Timer0 stop */
	U2R_TCR0_0   = U2S_TCR_SET;		/* clock=MP clk/4 = 48MHz/4 */

	/* The timer count value is operated with MS. */
	u4t_cnt_time = u4t_time * U4S_COUNT_1MS;

	/* Setting of timer count value */
	U4R_TCOR0_0 = u4t_cnt_time;
	U4R_TCNT0_0 = u4t_cnt_time;

	U1R_TSTR0  |= U1S_STR0_ON;		/* Timer0 start */

	/* Timer end waiting */
	while( !(U2R_TCR0_0 & U2S_UNF) );

	U1R_TSTR0  &= U1S_STR0_OFF;		/* Timer0 stop */
	U2R_TCR0_0 &= U2S_CLR_UNF;		/* UNF bit Clear */

}

/*************************************************************************************************/
/* Function name   : vog_timer_wait( u4 u4t_time )                                                      */
/* Input     : u4t_time(Wait time)                                                                 */
/* Return   : void                                                                               */
/* Processing : Timer Wait polling                                     */
/*************************************************************************************************/
void vog_timer_us_wait( u4 u4t_time )
{
	u4 u4t_cnt_time = U4G_ZERO;

	/* Setting of CPGA(Clock supply) */
	U4R_SMSTPCR1 &= U4S_TMU0_ON;	/* MSTPCR2 ;TMU0 ON */

	U1R_TSTR0   &= U1S_STR0_OFF;	/* Timer0 stop */
	U2R_TCR0_0   = U2S_TCR_SET;		/* clock=SUB clk/4 = 48MHz/4 */

	/* The timer count value is operated with US. */
	u4t_cnt_time = u4t_time * U4S_COUNT_1US;

	/* Setting of timer count value */
	U4R_TCOR0_0 = u4t_cnt_time;
	U4R_TCNT0_0 = u4t_cnt_time;

	U1R_TSTR0  |= U1S_STR0_ON;		/* Timer0 start */

	/* Timer end waiting */
	while( !(U2R_TCR0_0 & U2S_UNF) );

	U1R_TSTR0  &= U1S_STR0_OFF;		/* Timer0 stop */
	U2R_TCR0_0 &= U2S_CLR_UNF;		/* UNF bit Clear */

}

