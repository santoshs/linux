/* lcd_api.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef __LCD_API_H__
#define __LCD_API_H__

#include "com_type.h"

/*****************************************************************************
; Error Code
******************************************************************************/
#define LCD_SUCCESS				( 0)	/* Success                           */
#define LCD_ERR_INIT			(-1)	/* Initialize Failed                 */
#define LCD_ERR_NOT_INIT		(-2)	/* Not Initialized                   */
#define LCD_ERR_PARAM			(-3)	/* Parameter Error                   */
#define LCD_ERR_STRING_LENGTH	(-4)	/* String Length Error               */
#define LCD_ERR_NOT_LCD_BOARD	(-5)	/* The LCD board is not attached     */


/*****************************************************************************
; ThreeColor LED Color
******************************************************************************/
#define LED_COLOR_1				(uchar)( 0x01 )
#define LED_COLOR_2				(uchar)( 0x02 )
#define LED_COLOR_3				(uchar)( 0x03 )
#define LED_COLOR_4				(uchar)( 0x04 )
#define LED_COLOR_5				(uchar)( 0x05 )
#define LED_COLOR_6				(uchar)( 0x06 )
#define LED_COLOR_7				(uchar)( 0x07 )

/*****************************************************************************
; LCD Color
******************************************************************************/
#define LCD_COLOR_BLACK			(ulong)( 0x00000000 )
#define LCD_COLOR_RED			(ulong)( 0x00FF0000 )
#define LCD_COLOR_GREEN			(ulong)( 0x0000FF00 )
#define LCD_COLOR_BLUE			(ulong)( 0x000000FF )
#define LCD_COLOR_MAGENTA		(ulong)( LCD_COLOR_RED | LCD_COLOR_BLUE )
#define LCD_COLOR_YELLOW		(ulong)( LCD_COLOR_RED | LCD_COLOR_GREEN )
#define LCD_COLOR_CYAN			(ulong)( LCD_COLOR_GREEN | LCD_COLOR_BLUE )
#define LCD_COLOR_WHITE			(ulong)( LCD_COLOR_RED | LCD_COLOR_GREEN | LCD_COLOR_BLUE )

#define BMP_HEADER 0x4D42

/*****************************************************************************
; API Prototypes
******************************************************************************/
RC LCD_Init(void);
RC LCD_Check_Board(void);
void LCD_Clear(ulong color);
void LCD_Set_Text_color(ulong color);
RC LCD_Draw_Cmode(void);
RC LCD_Display_On(void);
RC LCD_Display_Off(void);
RC LCD_Backlight_On(void);
RC LCD_Backlight_Off(void);
RC LCD_Print(ulong p_x, ulong p_y, const char* pStr);
RC LCD_BMP_Draw(ulong set_x, ulong set_y, ulong set_w, ulong set_h, ulong img_addr);




#endif /* __LCD_API_H__ */
