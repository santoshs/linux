/* lcd.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef	__LCD_H__
#define	__LCD_H__

#include "com_type.h"

/********************************************************************************/
/*	Definitions																	*/
/********************************************************************************/

#define LED_WAIT				(0x02000000)



#define TIME_MS					(0)
#define TIME_US					(1)



#define LCD_FONT_WIDTH			(24)
#define LCD_FONT_HIGH			(54)
#define LCD_FONT_OFFSET			(0x20)
#define LCD_FONT_OTHER			(0x5f)

#define LCD_FONT_INDEX_MIN		(0x00)
#define LCD_FONT_INDEX_MAX		(0x5e)
#define LCD_STR_LEN_MAX			(20)



#define LCD_CHECK_BIT			(0x80)
#define LCD_MAX_BIT 			(8)
#define LCD_CARRY_BITS			(LCD_MAX_BIT - 1)
#define LCD_DISP_XMAX			(540)
#define LCD_DISP_YMAX			(960)
#define LCD_DISP_STRIDE			(544)
#define LCD_VRAM_MAX_LEN		(LCD_DISP_STRIDE * LCD_DISP_YMAX)
#define LCD_RGB888_ONE_PIXEL		(3)

/* BEGIN: CR994: Re-allocate SDRAM to reserve more free memory */
#define LCD_VRAM_BUFF_RGB		(0x5C000000) /* Frame Buffer */
/* END: CR994: Re-allocate SDRAM to reserve more free memory */

#define LCD_FONT_MAX			(0x60)

extern uchar gLcdInit;


RC LCD_PrintFont(ulong set_x, ulong set_y, ulong set_w, ulong set_h, const uchar* pBuff);

#endif // __LCD_H__
