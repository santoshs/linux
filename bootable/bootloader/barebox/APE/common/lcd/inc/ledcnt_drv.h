/*
 * ledcnt_drv.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */
#ifndef __H_LEDCNT_DRV_
#define __H_LEDCNT_DRV_

/***************************************************************/
/* INCLUDE FILE                                                */
/***************************************************************/

/***************************************************************/
/* PUBLIC CONSTANT DEFINITON                                   */
/***************************************************************/

/***************************************************************/
/* PUBLIC TYPEDEFE ENUM                                        */
/***************************************************************/

/***************************************************************/
/* PUBLIC FUNCTION EXTERN                                      */
/***************************************************************/
extern void vog_ledcnt_xreset( void );
extern void vos_ledcnt_start( void );
extern void vog_ledcnt_soft_reset( void );
extern void vog_ledcnt_backlight_on( void );
extern void vog_ledcnt_backlight_off( void );

#endif /* __H_LEDCNT_DRV_ */
