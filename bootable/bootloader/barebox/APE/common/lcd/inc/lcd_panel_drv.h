/*
 * lcd_panel_drv.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */
#ifndef __H_LCD_PANEL_DRV_
#define __H_LCD_PANEL_DRV_

/***************************************************************/
/* INCLUDE FILE                                                */
/***************************************************************/

/***************************************************************/
/* PUBLIC CONSTANT DEFINITON                                   */
/***************************************************************/

/***************************************************************/
/* PUBLIC TYPEDEF STRUCT UNION ENUM                            */
/***************************************************************/

/***************************************************************/
/* PUBLIC VARIABLE EXTERN                                      */
/***************************************************************/

/***************************************************************/
/* PUBLIC FUNCTION EXTERN                                      */
/***************************************************************/
extern void vog_lcd_panel_init( u1 u1t_lcd_mode );
extern void vog_lcd_panel_display_on( u1 u1t_lcd_mode );
extern void vog_lcd_panel_display_off( u1 u1t_lcd_mode );
extern void vog_lcd_panel_read( u1 u1t_lcd_mode );
extern void vog_lcd_time_on( u2 u2t_disp_x, u2 u2t_disp_y, u4 u4t_date, u4 u4t_time );

extern void vos_lcd_backlight_on( void );
extern void vos_lcd_backlight_off( void );
#endif /* __H_LCD_PANEL_DRV_ */
