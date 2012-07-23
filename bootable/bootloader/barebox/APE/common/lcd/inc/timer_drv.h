/*
 * timer_drv.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */
#ifndef __H_TIMER_DRV_
#define __H_TIMER_DRV_

/***************************************************************/
/* INCLUDE FILE                                                */
/***************************************************************/

/***************************************************************/
/* PUBLIC CONSTANT DEFINITON                                   */
/***************************************************************/
#define U1G_TIME_MS	(0)
#define U1G_TIME_US	(1)

/***************************************************************/
/* PUBLIC TYPEDEFE ENUM                                        */
/***************************************************************/

/***************************************************************/
/* PUBLIC FUNCTION EXTERN                                      */
/***************************************************************/
extern void vog_timer_wait( u4 u4t_time, u1 u1t_option );
extern void vog_timer_ms_wait( u4 u4t_time );
extern void vog_timer_us_wait( u4 u4t_time );

#endif /* H_TIMER_DRV */
