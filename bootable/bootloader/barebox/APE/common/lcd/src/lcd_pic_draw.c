/* 
 *	lcd_pic_draw.c 
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */
#include "com_type.h"
#include "lcd_api.h"
#include "lcd_common.h"
#include "lcd.h"
#include "string.h"
#include "timer_drv.h"


#define BMP_HEADER_SIZE 0x36


/********************************************************************************************************/
/* Function name   : LCD_BMP_Draw(ulong set_x, ulong set_y, ulong set_w, ulong set_h, ulong img_addr)	*/
/* Input    : set_x, set_y, set_w, set_h, img_addr														*/                                                                              
/* Return   : LCD_SUCCESS           : Success 															*/
/*            LCD_ERR_NOT_INIT      : Not Initialized													*/
/*            LCD_ERR_PARAM         : Parameter Error													*/
/* Processing : LCD print Font processing																*/
/********************************************************************************************************/
RC LCD_BMP_Draw(ulong set_x, ulong set_y, ulong set_w, ulong set_h, ulong img_addr)
{

	ulong axis_y, axis_x, point_y, pixel_num;
	short bmp_header;
	u1* img = (u1*)(img_addr + BMP_HEADER_SIZE) ; 
	
	/* Check LCD Init flag */
	if(gLcdInit != TRUE)
	{
		return LCD_ERR_NOT_INIT;
	}
	
	/* Sets a LCD VRAM Address */
	u1* lcd_vram_addr = (u1*)LCD_VRAM_BUFF_RGB;
	
	/* Check bitmap header */
	bmp_header = *((short *)img_addr);
	if (bmp_header != BMP_HEADER )
	{
		return LCD_ERR_PARAM;
	}
	
	/* Check Parameter */
	if(img_addr == 0 ||
	   set_w <= 0 ||
	   set_h <= 0 ||
	   (set_x + set_w) > LCD_DISP_XMAX ||
	   (set_y + set_h) > LCD_DISP_YMAX    )
	{
		return LCD_ERR_PARAM;
	}
	
	/* Image Draw */
	/* Y-axis count */
	for(axis_y = (set_h + set_y); axis_y > set_y; axis_y--)
	{
		/* Calculates a Y-axis start Address */
		point_y = LCD_DISP_STRIDE * (axis_y - 1);
		
		/* X-axis count */
		for(axis_x = set_x; axis_x < (set_w + set_x); axis_x++)
		{
			/* Calculates a Draw Address */
			pixel_num = point_y + axis_x;
			
			/* Set a LCD Dot Color */
			lcd_vram_addr[pixel_num*3+2] = *img++;
			lcd_vram_addr[pixel_num*3+1] = *img++;
			lcd_vram_addr[pixel_num*3  ] = *img++;
			
		}
	}
	
	return LCD_SUCCESS;
}

