/*
 * lcd_drv.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */
#ifndef __H_LCD_DRV_
#define __H_LCD_DRV_

/***************************************************************/
/* INCLUDE FILE                                                */
/***************************************************************/

/***************************************************************/
/* PUBLIC CONSTANT DEFINITON                                   */
/***************************************************************/
#define U2G_DISP_XMIN			(u2)( 0 )
#define U2G_DISP_XMAX			(u2)( 540 )
#define U2G_DISP_YMIN			(u2)( 0 )
#define U2G_DISP_YMAX			(u2)( 960 )


#define U2G_LCD_ONELINE_OFFSET	( U2G_DISP_XMAX * 2 )

/* LCD Color */
#define U2G_COLOR_RED			(u2)( 0xF800 )
#define U2G_COLOR_GREEN			(u2)( 0x07E0 )
#define U2G_COLOR_BLUE			(u2)( 0x001F )
#define U2G_COLOR_WHITE			(u2)( 0xFFFF )
#define U2G_COLOR_BLACK			(u2)( 0x0000 )
#define U2G_COLOR_YELLOW		(u2)( 0xFFE0 )
#define U2G_COLOR_CYAN			(u2)( 0x07FF )
#define U2G_COLOR_MAGENTA		(u2)( 0xF81F )
#define U2G_COLOR_ORANGE		(u2)( 0xFC00 )
#define U2G_COLOR_NAVY			(u2)( 0x0010 )
#define U2G_COLOR_MOS_GREEN		(u2)( 0x0400 )



#define U1G_LCD_MAIN			(u1)( 0 )		/* MAIN LCD  */
#define U1G_LCD_SUB				(u1)( 1 )		/* SUB LCD   */

#define U4R_RWUCR				(*(VU4 *)(0xE6180010))	

/***************************************************************/
/* PUBLIC TYPEDEF STRUCT UNION ENUM                            */
/***************************************************************/
/* LCD Structure */
typedef struct point
{
	s2			s2_p_x;
	s2			s2_p_y;
} ST_POINT;

typedef struct lcd_dot
{
	ST_POINT	st_point;
	u2			u2_color;
} ST_LCD_DOT;

typedef struct lcd_rect
{
	ST_POINT	st_left_top;
	ST_POINT	st_right_bottom;
	u2			u2_color;
} ST_LCD_RECT;

typedef struct lcd_bmp
{
	ST_POINT	st_left_top;
	s2			s2_x_width;
	s2			s2_y_height;
	s2*			pt_data;
} ST_LCD_BMP;

typedef struct data_rect
{
	s2			s2_l_x;
	s2			s2_l_y;
	s2			s2_r_x;
	s2			s2_r_y;
	u2			u2_color;
} ST_DATA_RECT;

/***************************************************************/
/* PUBLIC VARIABLE EXTERN                                      */
/***************************************************************/


/***************************************************************/
/* PUBLIC FUNCTION EXTERN                                      */
/***************************************************************/
extern void vog_lcd_display_on( u2* ptt_vram_sta_addr, u1 u1t_lcd_mode );
extern void vog_lcd_display_off( u1 u1t_lcd_mode );
extern void vog_lcd_init_for_panel( u2* vram_sta_addr, u1 u1t_lcd_mode );

extern void vog_lcdc_to_dsi_start( u2* ptt_vram_sta_addr, u1 u1t_lcd_mode );
extern void vog_lcdc_to_stop( u1 u1t_lcd_mode );
extern void vog_lcdc_int_wait( void );

void vog_lcd_display_on( u2* ptt_vram_sta_addr, u1 u1t_lcd_mode );
void vog_lcd_display_off( u1 u1t_lcd_mode );
void vog_lcd_init_for_panel( u2* vram_sta_addr, u1 u1t_lcd_mode );

void vog_lcdc_int_wait( void );
void vog_lcd_display_draw_cmode( u4 vram_addr );

#endif /*  __H_LCD_DRV_ */

