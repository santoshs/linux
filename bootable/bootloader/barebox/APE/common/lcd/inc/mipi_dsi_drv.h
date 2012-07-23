/*
 * mipi_dsi_drv.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */
#ifndef __H_MIPI_DSI_DRV_
#define __H_MIPI_DSI_DRV_

/***************************************************************/
/* INCLUDE FILE                                                */
/***************************************************************/

/***************************************************************/
/* PUBLIC CONSTANT DEFINITON                                   */
/***************************************************************/
#define U1G_LCD_MAIN		(u1)( 0 )		/* MAIN LCD  */
#define U1G_LCD_SUB			(u1)( 1 )		/* SUB LCD   */

/***************************************************************/
/* PUBLIC TYPEDEFE ENUM                                        */
/***************************************************************/

/***************************************************************/
/* PUBLIC FUNCTION EXTERN                                      */
/***************************************************************/
extern void vog_mipi_dsi_init( u1 u1t_lcd_mode );
extern void vog_mipi_dsi_ena_vmode( void );
extern void vog_mipi_dsi_dis_vmode( void );
extern void vog_mipi_dsi_tr_spacket( u1 u1t_data_type, u1 u1t_data0, u1 u1t_data1 );

extern void vog_mipi_dsi_reg_set( u1 u1t_lcd_mode );
extern void vog_mipi_dsi_reg_get( u1* ptt_lcd_mode );
extern void vog_mipi_dsi_tr_lpacket( u1 u1t_data_type, u2 u2t_data_cnt, u1* ptt_data, u1 u1t_tr_speed );
extern void vog_mipi_dsi_draw_cmode( u4 addr );


#endif /* __H_MIPI_DSI_DRV_ */
