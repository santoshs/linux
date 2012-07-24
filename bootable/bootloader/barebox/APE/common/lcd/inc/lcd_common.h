/* lcd_common.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef __LCD_COMMON_H__
#define __LCD_COMMON_H__

#include "eos_system.h"
/***************************************************************/
/* INCLUDE FILE                                                */
/***************************************************************/

#define U4R_RMSTPCR0			(*(VU4 *)(CPGA_BADR+0x0110))		/* Realtime module stop control register 0 */
#define U4R_RMSTPCR1			(*(VU4 *)(CPGA_BADR+0x0114))		/* Realtime module stop control register 1 */
#define U4R_RMSTPCR2			(*(VU4 *)(CPGA_BADR+0x0118))		/* Realtime module stop control register 2 */
#define U4R_RMSTPCR3			(*(VU4 *)(CPGA_BADR+0x011C))		/* Realtime module stop control register 3 */
#define U4R_RMSTPCR4			(*(VU4 *)(CPGA_BADR+0x0120))		/* Realtime module stop control register 4 */
#define U4R_RMSTPCR5			(*(VU4 *)(CPGA_BADR+0x0124))		/* Realtime module stop control register 5 */
#define U4R_SMSTPCR0			(*(VU4 *)(CPGA_BADR+0x0130))		/* System module stop control register 0 */
#define U4R_SMSTPCR1			(*(VU4 *)(CPGA_BADR+0x0134))		/* System module stop control register 1 */
#define U4R_SMSTPCR2			(*(VU4 *)(CPGA_BADR+0x0138))		/* System module stop control register 2 */
#define U4R_SMSTPCR3			(*(VU4 *)(CPGA_BADR+0x013C))		/* System module stop control register 3 */
#define U4R_SMSTPCR4			(*(VU4 *)(CPGA_BADR+0x0140))		/* System module stop control register 4 */
#define U4R_SMSTPCR5			(*(VU4 *)(CPGA_BADR+0x0144))		/* System module stop control register 4 */

#define U4R_SRCR0				(*(VU4 *)(CPGA_BADR+0x80A0))		/* Software reset register 0 */
#define U4R_SRCR1				(*(VU4 *)(CPGA_BADR+0x80A8))		/* Software reset register 1 */
#define U4R_SRCR2				(*(VU4 *)(CPGA_BADR+0x80B0))		/* Software reset register 2 */
#define U4R_SRCR3				(*(VU4 *)(CPGA_BADR+0x80B8))		/* Software reset register 3 */
#define U4R_SRCR4				(*(VU4 *)(CPGA_BADR+0x80BC))		/* Software reset register 4 */


/**********************************************************************/
/*** CPGA                                                           ***/
/*--------------------------------------------------------------------*/
/*** CPGA_BADR:0xE6150000                                           ***/
/**********************************************************************/

/***************************************************************/
/* PUBLIC CONSTANT DEFINITON                                   */
/***************************************************************/
#define U1G_LCD_MAIN			(u1)( 0 )		/* MAIN LCD    */
#define U1G_LCD_SUB				(u1)( 1 )		/* SUB LCD     */

#define LCD_BOARD_ATTACHED		(u4)( 0 )		/* The LCD board is attached     */
#define LCD_BOARD_NOT_ATTACHED	(u4)( 1 )		/* The LCD board is not attached */


#endif /* __LCD_COMMON_H__ */
