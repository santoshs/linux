/* 
 *	lcd_api.c 
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "lcd.h"
#include "lcd_common.h"
#include "lcd_api.h"
#include "lcd_drv.h"
#include "lcd_panel_drv.h"

#include "com_type.h"
#include "common.h"
#include "string.h"
#include "ledcnt_drv.h"
#include "timer_drv.h"
#include "ledcnt_drv.h"



/***************************************************************/
/* STATIC CONSTANT DEFINE                                      */
/***************************************************************/
uchar gLcdInit = FALSE;
static uchar gLcdDisplayON = FALSE;
static uchar gLcdBacklightON = FALSE;
static ulong gBackground = LCD_COLOR_WHITE;
static ulong gTextColor = LCD_COLOR_RED;


/****************************************************************************************************/
/* Function name   : LCD_Init(void)                                                              	*/
/* Input    : void                                                                               	*/
/* Return   : LCD_SUCCESS      : Success															*/
/*            LCD_ERR_INIT     : Initialize Failed													*/                                                                                
/* Processing : LCD initialize processing                                                       	*/
/****************************************************************************************************/
RC LCD_Init(void)
{
	/* Setting LCD Flags */
	gLcdInit = TRUE;
	gLcdDisplayON = FALSE;
	gLcdBacklightON = FALSE;
	
	return LCD_SUCCESS;
}

/****************************************************************************************************/
/* Function name   : LCD_Clear(void)                                                              	*/
/* Input    : void                                                                               	*/
/* Return   : void      																			*/
/* Processing : LCD clear processing                                                       			*/
/****************************************************************************************************/
void LCD_Clear(ulong color)
{
	ulong i;
	
	/* Sets a LCD VRAM Address */
	u1* lcd_vram_addr = (u1*)LCD_VRAM_BUFF_RGB;
	
	u1 cr = (u1)((color>>16)&0xFF);
	u1 cg = (u1)((color>>8)&0xFF);
	u1 cb = (u1)((color)&0xFF);
	/* LCD VRAM area is filled in specified color */
	for(i=0; i < LCD_VRAM_MAX_LEN; i++)
	{
		lcd_vram_addr[i*3  ] = cr;
		lcd_vram_addr[i*3+1] = cg;
		lcd_vram_addr[i*3+2] = cb;
	}
	
	gBackground = color;
}

/****************************************************************************************************/
/* Function name   : LCD_Set_Text_color(ulong color)                                               	*/
/* Input    : color                                                                               	*/
/* Return   : void      																			*/
/* Processing : Set text color		                                                       			*/
/****************************************************************************************************/
void LCD_Set_Text_color(ulong color)
{
	gTextColor = color;
}

/****************************************************************************************************/
/* Function name   : LCD_Draw_Cmode(void)                                                          	*/
/* Input    : void                                                                               	*/
/* Return   : LCD_SUCCESS      : Success															*/
/*            LCD_ERR_NOT_INIT : Not Initialized      												*/
/* Processing : LCD Draw Command Mode processing                                           			*/
/****************************************************************************************************/
RC LCD_Draw_Cmode(void)
{
	if(gLcdInit != TRUE && gLcdDisplayON != TRUE)
	{
		return LCD_ERR_NOT_INIT;
	}
	
	/* Send Draw Command */
	vog_lcd_display_draw_cmode(LCD_VRAM_BUFF_RGB);
	
	return LCD_SUCCESS;
}

/****************************************************************************************************/
/* Function name   : LCD_Display_On(void)                                                          	*/
/* Input    : void                                                                               	*/
/* Return   : LCD_SUCCESS      : Success															*/
/*            LCD_ERR_NOT_INIT : Not Initialized      												*/
/* Processing : LCD display on processing		                                           			*/
/****************************************************************************************************/
RC LCD_Display_On(void)
{
	
	if(gLcdInit != TRUE)
	{
		return LCD_ERR_NOT_INIT;
	}
	
	/* Check LCD flag */
	if(gLcdDisplayON != TRUE)
	{
		/* LCD panel ON */
		vog_lcd_display_on((u2* )LCD_VRAM_BUFF_RGB, U1G_LCD_MAIN);
	}
	/* Set LCD flag */
	gLcdDisplayON = TRUE;
	
	return LCD_SUCCESS;
}

/****************************************************************************************************/
/* Function name   : LCD_Display_Off(void)                                                          	*/
/* Input    : void                                                                               	*/
/* Return   : LCD_SUCCESS      : Success															*/
/*            LCD_ERR_NOT_INIT : Not Initialized      												*/
/* Processing : LCD display off processing		                                           			*/
/****************************************************************************************************/
RC LCD_Display_Off(void)
{
	
	if(gLcdInit != TRUE)
	{
		return LCD_ERR_NOT_INIT;
	}
	
	/* Check LCD flag */
	if(gLcdDisplayON != FALSE)
	{
		/* LCD panel OFF */
		vog_lcd_display_off(U1G_LCD_MAIN);
	}
	/* Set LCD flag */
	gLcdDisplayON = FALSE;
	
	return LCD_SUCCESS;
}

/****************************************************************************************************/
/* Function name   : LCD_Backlight_On(void)                                                        	*/
/* Input    : void                                                                               	*/
/* Return   : LCD_SUCCESS      : Success															*/
/*            LCD_ERR_NOT_INIT : Not Initialized      												*/
/* Processing : LCD backlight on processing		                                           			*/
/****************************************************************************************************/
RC LCD_Backlight_On(void)
{
	if(gLcdInit != TRUE)
	{
		return LCD_ERR_NOT_INIT;
	}
	
	/* Check LCD flag */
	if(gLcdBacklightON != TRUE)
	{
		/* LCD backlight ON */
		vos_lcd_backlight_on();
	}
	
	/* Set LCD flag */
	gLcdBacklightON = TRUE;
	
	return LCD_SUCCESS;
}

/****************************************************************************************************/
/* Function name   : LCD_Backlight_Off(void)                                                      	*/
/* Input    : void                                                                               	*/
/* Return   : LCD_SUCCESS      : Success															*/
/*            LCD_ERR_NOT_INIT : Not Initialized      												*/
/* Processing : LCD backlight off processing		                                      			*/
/****************************************************************************************************/
RC LCD_Backlight_Off(void)
{
	if(gLcdInit != TRUE)
	{
		return LCD_ERR_NOT_INIT;
	}
	
	/* Check LCD flag */
	if(gLcdBacklightON != FALSE)
	{
		/* LCD backlight OFF */
		vos_lcd_backlight_off();
	}
	
	/* Set LCD flag */
	gLcdBacklightON = FALSE;
	
	return LCD_SUCCESS;
}

/****************************************************************************************************/
/* Function name   : LCD_PrintFont(void)                                      	                	*/
/* Input    : void                                                                               	*/
/* Return   : LCD_SUCCESS   : Success																*/
/*            LCD_ERR_PARAM : Parameter Error      													*/
/* Processing : LCD print Font processing		                       		            			*/
/****************************************************************************************************/
RC LCD_PrintFont(ulong set_x, ulong set_y, ulong set_w, ulong set_h, const uchar* pBuff)
{
	ulong pixel_num;
	u4 pixel_color;
	ulong point_x;
	ulong point_y;

	ulong buff_idx;
	ulong w_max_byte;
	ulong axis_y, axis_x_byte, axis_x_bit;
	
	/* Calculates a LCD End Dot Address of VRAM */
	buff_idx = (LCD_DISP_STRIDE * (set_y + (set_h - 1))) + set_x + (set_w - 1);
	
	/* Check Parameter */
	if(pBuff == 0 || set_w <= 0 || set_h <= 0 || buff_idx >= LCD_VRAM_MAX_LEN)
	{
		return LCD_ERR_PARAM;
	}
	
	/* Sets a LCD VRAM Address */
	u1* lcd_vram_addr = (u1*)LCD_VRAM_BUFF_RGB;
	
	/* Calculates a X-axis MAX byte-count */
	w_max_byte = (set_w + LCD_CARRY_BITS) / LCD_MAX_BIT;
	
	/* Y-axis count */
	for(axis_y = 0; axis_y < set_h; axis_y++)
	{
		/* Calculates a buffer index */
		buff_idx = w_max_byte * axis_y;
		/* Calculates a Y-axis start Address */
		point_y = LCD_DISP_STRIDE * (axis_y + set_y);
		
		/* X-axis Byte count */
		for(axis_x_byte = 0; axis_x_byte < w_max_byte; axis_x_byte++)
		{
			/* Calculates a X-axis start Address */
			point_x = set_x + (LCD_MAX_BIT * axis_x_byte);
			
			/* X-axis Bit count */
			for(axis_x_bit = 0; axis_x_bit < LCD_MAX_BIT; axis_x_bit++)
			{
				/* judges whether the bit is 1 */
				if(pBuff[buff_idx + axis_x_byte] & (LCD_CHECK_BIT >> axis_x_bit))
				{
					/* Sets a font color to the text color */
					pixel_color = gTextColor;
				}
				else
				{
					/* Sets a font color to the background color */
					pixel_color = gBackground;
				}
				
				/* Calculates a LCD Dot Address of VRAM */
				pixel_num = (point_x + axis_x_bit + point_y);
				
				/* Sets a LCD Dot Color  */
				lcd_vram_addr[pixel_num*3] = (u1)((pixel_color>>16)&0xFF);
				lcd_vram_addr[pixel_num*3 + 1] = (u1)((pixel_color>>8)&0xFF);
				lcd_vram_addr[pixel_num*3 + 2] = (u1)(pixel_color&0xFF);
			}
		}
	}
	
	
	
	return LCD_SUCCESS;
}

/****************************************************************************************************/
/* Function name   : LCD_Check_Board(void)                                                     		*/
/* Input    : void                                                                               	*/
/* Return   : LCD_SUCCESS           : Success 														*/
/*            LCD_ERR_NOT_INIT      : Not Initialized												*/
/* Processing : LCD print Font processing															*/
/****************************************************************************************************/
RC LCD_Check_Board(void)
{
	if(gLcdInit != TRUE)
	{
		return LCD_ERR_NOT_INIT;
	}
	
	return LCD_SUCCESS;
}


