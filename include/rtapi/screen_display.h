/*
 * drivers/char/rtapi/include/screen_display.h
 *     This file is screen display function.
 *
 * Copyright (C) 2011-2012 Renesas Electronics Corporation

 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
 
#ifndef __SCREEN_DISPLAY_H__
#define __SCREEN_DISPLAY_H__

/****************************************/
/* Fixed Number				*/
/****************************************/
/* LCD Type */
#define RT_DISPLAY_LCD1 (1)
#define RT_DISPLAY_LCD2 (2)
#define RT_DISPLAY_HDMI (3)
#define RT_DISPLAY_LCDHDMI (7)	/* #MU2DSP582 add */

/** Image format */
#define RT_DISPLAY_FORMAT_RGB565		(1)
#define RT_DISPLAY_FORMAT_ARGB8888		(2)

/** Refreshment mode */
#define RT_DISPLAY_REFRESH_ON			(1)
#define RT_DISPLAY_REFRESH_OFF			(2)

/** Display Buffer ID **/
#define RT_DISPLAY_BUFFER_A				(0)
#define RT_DISPLAY_BUFFER_B				(1)

/** HDMI Output Format */
#define	RT_DISPLAY_720_480P60			(1)
#define	RT_DISPLAY_1280_720P60			(2)
#define	RT_DISPLAY_1920_1080I60			(3)
#define	RT_DISPLAY_1920_1080P24			(4)
/* #MU2DSP582 mod -S- */
#define	RT_DISPLAY_720_576P50			(5)
#define	RT_DISPLAY_1280_720P50			(6)
#define	RT_DISPLAY_1920_1080P60			(11)
#define	RT_DISPLAY_1920_1080P50			(12)
#define	RT_DISPLAY_720_480P60A43		(13)
#define	RT_DISPLAY_720_576P50A43		(14)
/* #MU2DSP582 mod -E- */

/* #MU2DSP582 mod -S- */
/* Rotation type */
#define RT_DISPLAY_ROTATE_0				(1)
#define RT_DISPLAY_ROTATE_90			(2)
#define RT_DISPLAY_ROTATE_180			(3)
#define RT_DISPLAY_ROTATE_270			(4)
/* #MU2DSP582 mod -E- */

/** RT I/F Return code */
#define SMAP_LIB_DISPLAY_OK				(0)
#define SMAP_LIB_DISPLAY_NG				(-1)
#define SMAP_LIB_DISPLAY_PARAERR		(-2)
#define SMAP_LIB_DISPLAY_SEQERR			(-3)

/****************************************/
/* Structure				*/
/****************************************/
typedef struct
{
	unsigned int VCLKCR3; 
	unsigned int DSITCKCR;
	unsigned int DSI0PCKCR;
	unsigned int DSI0PHYCR;
	unsigned int SYSCONF;
	unsigned int TIMSET0;
	unsigned int DSICTRL;
	unsigned int VMCTR1;
	unsigned int VMCTR2;
	unsigned int VMLEN1;
	unsigned int VMLEN2;
	unsigned int VMLEN3;
	unsigned int VMLEN4;
	unsigned int DTCTR;
	unsigned int MLDHCNR;
	unsigned int MLDHSYNR;
	unsigned int MLDHAJR;
	unsigned int MLDVLNR;
	unsigned int MLDVSYNR;
	unsigned int MLDMT1R;		/* #MU2DSP381 */
	unsigned int LDDCKR;		/* #MU2DSP533 */
	unsigned int MLDDCKPAT1R;	/* #MU2DSP533 */
	unsigned int MLDDCKPAT2R;	/* #MU2DSP533 */
} screen_disp_lcd_if;

#ifndef _SCREEN_RECT_TYPE_
#define _SCREEN_RECT_TYPE_
typedef struct
{
	unsigned short x;
	unsigned short y;
	unsigned short width;
	unsigned short height;
} screen_rect;
#endif

typedef struct
{
	void*          handle;
	unsigned short output_mode;
	unsigned short dummy;
	unsigned int   address;
} screen_disp_get_address;

typedef struct
{
	void*            handle;
	unsigned short   output_mode;
	unsigned short   format;
	screen_rect      draw_rect;
	unsigned int     buffer_offset;
/* #MU2DSP582 mod -S- */
	unsigned short   rotate;
	unsigned short   dummy;
/* #MU2DSP582 mod -E- */
} screen_disp_draw;

typedef struct
{
	void*          handle;
	unsigned short output_mode;
} screen_disp_start_lcd;

typedef struct
{
	void*          handle;
	unsigned short output_mode;
} screen_disp_stop_lcd;

typedef struct
{
	void*          handle;
	unsigned short output_mode;
	unsigned short refresh_mode;
} screen_disp_set_lcd_refresh;

typedef struct
{
	void*        handle;
	unsigned int format;
	unsigned int background_color;
} screen_disp_start_hdmi;

typedef struct
{
	void* handle;
} screen_disp_stop_hdmi;

/* #MU2DSP222 add -S- */
typedef struct
{
	void*         handle;
	unsigned short output_mode;
	unsigned short dummy;
	unsigned char data_id;
	unsigned char reg_address;
	unsigned char	write_data;
	unsigned char	data_count;
	unsigned char*	read_data;
} screen_disp_read_dsi_short;
/* #MU2DSP222 add -E- */

typedef struct
{
	void*         handle;
	unsigned short output_mode;	/* #MU2DSP188 */
	unsigned char data_id;
	unsigned char reg_address;
	unsigned char write_data;
} screen_disp_write_dsi_short;

typedef struct
{
	void*          handle;
	unsigned short output_mode;	/* #MU2DSP188 */
	unsigned char  data_id;
	unsigned char  dummy;
	unsigned short data_count;
	unsigned short dummy2;		/* #MU2DSP188 */
	unsigned char* write_data;
} screen_disp_write_dsi_long;

typedef struct
{
	void*               handle;
	screen_disp_lcd_if* lcd_if_param;
	screen_disp_lcd_if* lcd_if_param_mask;
} screen_disp_set_lcd_if_param;

typedef struct
{
	void* handle;
} screen_disp_delete;

/****************************************/
/* function				*/
/****************************************/
/* Create handle */
extern void* screen_display_new
(
	void
);

/* Get address */
extern int screen_display_get_address
(
	screen_disp_get_address* address
);

/* Draw */
extern int screen_display_draw
(
	screen_disp_draw* disp_draw
);

/* Start LCD */
extern int screen_display_start_lcd
(
	screen_disp_start_lcd* start_lcd
);

/* Stop LCD */
extern int screen_display_stop_lcd
(
	screen_disp_stop_lcd* stop_lcd
);

/* Set LCD refresh mode */
extern int screen_display_set_lcd_refresh
(
	screen_disp_set_lcd_refresh* set_lcd_refresh
);

/* Start HDMI */
extern int screen_display_start_hdmi
(
	screen_disp_start_hdmi* start_hdmi
);

/* Stop HDMI */
extern int screen_display_stop_hdmi
(
	screen_disp_stop_hdmi* stop_hdmi
);

/* #MU2DSP222 add -S- */
/* Read LCD packet data */
extern int screen_display_read_dsi_short_packet
(
	screen_disp_read_dsi_short* read_dsi_s
);
/* #MU2DSP222 add -E- */


/* Write LCD packet data */
extern int screen_display_write_dsi_short_packet
(
	screen_disp_write_dsi_short* write_dsi_s
);

/* Write LCD packet data */
extern int screen_display_write_dsi_long_packet
(
	screen_disp_write_dsi_long* write_dsi_l
);

/* Set LCD parameters */
extern int screen_display_set_lcd_if_parameters
(
	screen_disp_set_lcd_if_param* set_lcd_if_param
);

/* Delete handle */
extern void screen_display_delete
(
	screen_disp_delete* disp_delete
);

#endif	/* __SCREEN_DISPLAY_H__ */
