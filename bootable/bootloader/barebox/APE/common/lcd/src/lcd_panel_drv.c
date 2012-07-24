/*
 * lcd_panel_drv.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */
/***************************************************************/
/* INCLUDE FILE                                                */
/***************************************************************/
#include "string.h"
#include "eos_csw.h"					/* EOS CSW Header file */

#include "eos_stdio.h"					/* EOS Standard Header */
#include "cp1_c01_cpg.h"
#include "cp1_s23_tpu.h"

#include "port_io_drv.h"
#include "timer_drv.h"
#include "mipi_dsi_drv.h"

#include "lcd_panel_drv.h"
#include "lcd.h"

/***************************************************************/
/* STATIC CONSTANT DEFINE                                      */
/***************************************************************/
/* Data Type */
#define DT_DCS_WRITE_NO_PARAM	(u1)( 0x05 )
#define DT_DCS_WRITE_1_PARAM	(u1)( 0x15 )
#define U1S_GENERIC_LONG_WRITE	(u1)( 0x39 )
#define U2S_LCD_PANEL_RST_PORT	(u2)( 31 )
#define U2S_LCD_PANEL_TE_PORT	(u2)( 33 )
#define U2S_LCD_PANEL_POWER		(u2)( 89 )
#define U1S_PORT_OE				(u1)( 0x10 )

#define U2S_MLCD_WLED_EN_PORT	(u2)( 39 )
#define U2S_SLCD_WLED_EN_PORT	(u2)( 228 )
#define U1S_PWM_FUNCTION		(u1)( 0x13 )
#define U2S_TPU0_3_EN_SET		(u2)( 0x0008 )
#define U2S_TPU0_CTRL_SET		(u2)( 0x0040 )
#define U2S_TPU0_CNT_CLEAR		(u2)( 0x0000 )
#define U2S_TPU0_PWM_MODE		(u2)( 0x0002 )
#define U2S_TPU0_PWM_BASE		(u2)( 0x1000 )
#define U2S_TPU0_PWM_LOW		(u2)( 0x0800 )
#define U2S_TPU0TO3_ON			(u2)( 0x0002 )

#define U1S_DATA_COUNT			(u1)( 0 )
#define U1S_DATA_TR_SP			(u1)( 1 )
#define U1S_DATA_ST_NUM			(u1)( 2 )

#define U1S_HSSEL_LOW_POWER		(u1)( 0 )
#define U1S_HSSEL_HIGH_SPEED	(u1)( 1 )

#define MIPI_DSI_DCS_LONG_WRITE		(0x39)
#define MIPI_DSI_DCS_SHORT_WRITE_PARAM	(0x15)
#define MIPI_DSI_DCS_SHORT_WRITE	(0x05)
#define MIPI_DSI_DELAY			(0x00)
#define MIPI_DSI_BLACK			(0x01)
#define MIPI_DSI_END			(0xFF)


/***************************************************************/
/* STATIC TYPEDEF                                              */
/***************************************************************/
struct _s6e39a0x02_cmdset {
	unsigned char cmd;
	const unsigned char * const data;
	int size;
};

/***************************************************************/
/* STATIC VARIABLE                                             */
/***************************************************************/
/**
 * LCD command tables
 */
const unsigned char data_to_send_01[] = { 0xf0, 0x5a, 0x5a };
const unsigned char data_to_send_02[] = { 0xf1, 0x5a, 0x5a };
const unsigned char data_to_send_03[] = { 0xfc, 0x5a, 0x5a };
const unsigned char data_to_send_04[] = { 0xfa, 0x02, 0x58, 0x42, 0x56, 0xaa, 0xc8, 0xae, 0xb5, 0xc1, 0xbe, 0xb4, 0xc0, 0xb2, 0x93, 0x9f, 0x93, 0xa6, 0xad, 0xa2, 0x00, 0xe9, 0x00, 0xdb, 0x01, 0x0f };
const unsigned char data_to_send_05[] = { 0xfa, 0x03 };
const unsigned char data_to_send_06[] = { 0xf8, 0x27, 0x27, 0x08, 0x08, 0x4e, 0xaa, 0x5e, 0x8a, 0x10, 0x3f, 0x10, 0x10, 0x00 };
const unsigned char data_to_send_07[] = { 0xf7, 0x03 };
const unsigned char data_to_send_08[] = { 0xb3, 0x63, 0x02, 0xc3, 0x32, 0xff };
const unsigned char data_to_send_09[] = { 0xf6, 0x00, 0x84, 0x09 };
const unsigned char data_to_send_10[] = { 0xb0, 0x09 };
const unsigned char data_to_send_11[] = { 0xd5, 0x64 };
const unsigned char data_to_send_12[] = { 0xb0, 0x0b };
const unsigned char data_to_send_13[] = { 0xd5, 0xa4, 0x7e, 0x20 };
const unsigned char data_to_send_14[] = { 0xb0, 0x08 };
const unsigned char data_to_send_15[] = { 0xfd, 0xf8 };
const unsigned char data_to_send_16[] = { 0xb0, 0x01 };
const unsigned char data_to_send_17[] = { 0xf2, 0x07 };
const unsigned char data_to_send_18[] = { 0xb0, 0x04 };
const unsigned char data_to_send_19[] = { 0xf2, 0x4d };
const unsigned char data_to_send_20[] = { 0xb1, 0x01, 0x00, 0x16 };
const unsigned char data_to_send_21[] = { 0xb2, 0x15, 0x15, 0x15, 0x15 };
const unsigned char data_to_send_22[] = { 0x11, 0x00 };	/* Sleep Out */
const unsigned char data_to_send_23[] = { 0x2a, 0x00, 0x00, 0x02, 0x57 };
const unsigned char data_to_send_24[] = { 0x2b, 0x00, 0x00, 0x03, 0xff };
const unsigned char data_to_send_25[] = { 0x2c, 0x00 };
const unsigned char data_to_send_26[] = { 0x35, 0x00 };
const unsigned char data_to_send_27[] = { 0x2a, 0x00, 0x1e, 0x02, 0x39 };	/* panel size is qHD 540x960 */
const unsigned char data_to_send_28[] = { 0x2b, 0x00, 0x00, 0x03, 0xbf };	/* panel size is qHD 540x960 */
const unsigned char data_to_send_29[] = { 0xd1, 0x8a };
const unsigned char data_to_send_30[] = { 0x29, 0x00 };	/* Display On */

const struct _s6e39a0x02_cmdset s6e39a0x02_cmdset[]= {
	{MIPI_DSI_DCS_LONG_WRITE,	data_to_send_01,	sizeof(data_to_send_01)	},
	{MIPI_DSI_DCS_LONG_WRITE,	data_to_send_02,	sizeof(data_to_send_02)	},
	{MIPI_DSI_DCS_LONG_WRITE,	data_to_send_03,	sizeof(data_to_send_03)	},
	{MIPI_DSI_DCS_LONG_WRITE,	data_to_send_04,	sizeof(data_to_send_04)	},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM,data_to_send_05,	sizeof(data_to_send_05)	},
	{MIPI_DSI_DCS_LONG_WRITE,	data_to_send_06,	sizeof(data_to_send_06)	},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM,data_to_send_07,	sizeof(data_to_send_07)	},
	{MIPI_DSI_DCS_LONG_WRITE,	data_to_send_08,	sizeof(data_to_send_08)	},
	{MIPI_DSI_DCS_LONG_WRITE,	data_to_send_09,	sizeof(data_to_send_09)	},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM,data_to_send_10,	sizeof(data_to_send_10)	},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM,data_to_send_11,	sizeof(data_to_send_11)	},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM,data_to_send_12,	sizeof(data_to_send_12)	},
	{MIPI_DSI_DCS_LONG_WRITE,	data_to_send_13,	sizeof(data_to_send_13)	},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM,data_to_send_14,	sizeof(data_to_send_14)	},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM,data_to_send_15,	sizeof(data_to_send_15)	},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM,data_to_send_16,	sizeof(data_to_send_16)	},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM,data_to_send_17,	sizeof(data_to_send_17)	},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM,data_to_send_18,	sizeof(data_to_send_18)	},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM,data_to_send_19,	sizeof(data_to_send_19)	},
	{MIPI_DSI_DCS_LONG_WRITE,	data_to_send_20,	sizeof(data_to_send_20)	},
	{MIPI_DSI_DCS_LONG_WRITE,	data_to_send_21,	sizeof(data_to_send_21)	},
	{MIPI_DSI_DCS_SHORT_WRITE,	data_to_send_22,	sizeof(data_to_send_22)	},
	{MIPI_DSI_DELAY,		NULL,			120			},
	{MIPI_DSI_DCS_LONG_WRITE,	data_to_send_23,	sizeof(data_to_send_23)	},
	{MIPI_DSI_DCS_LONG_WRITE,	data_to_send_24,	sizeof(data_to_send_24)	},
	{MIPI_DSI_DCS_SHORT_WRITE,	data_to_send_25,	sizeof(data_to_send_25)	},
	{MIPI_DSI_DELAY,		NULL,			20			},
	{MIPI_DSI_DCS_SHORT_WRITE,	data_to_send_26,	sizeof(data_to_send_26)	},
	{MIPI_DSI_DCS_LONG_WRITE,	data_to_send_27,	sizeof(data_to_send_27)	},
	{MIPI_DSI_DCS_LONG_WRITE,	data_to_send_28,	sizeof(data_to_send_28)	},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM,data_to_send_29,	sizeof(data_to_send_29)	},
/*	{MIPI_DSI_BLACK,		NULL,			0			},*/
/*	{MIPI_DSI_DCS_SHORT_WRITE,	data_to_send_30,	sizeof(data_to_send_30)	},*/
	{MIPI_DSI_END,			NULL,			0			},
};


/***************************************************************/
/* STATIC FUNCTION PROTOTYPE                                   */
/***************************************************************/
static void vos_lcd_panel_reset( u1 u1t_lcd_mode );
static void vos_lcd_first_set( void );

/***************************************************************/
/* PUBLIC FUNCTION PROTOTYPE                                   */
/***************************************************************/
void vos_lcd_backlight_on( void );
void vos_lcd_backlight_off( void );
/*************************************************************************************************/
/* function : lcd_panel_init( int lcd_mode )                                                     */
/* parameter: int lcd_mode(0:LCD_MAIN, 1:LCD_SUB)                                                */
/* return   : void                                                                               */
/* outline  : Initialize LCD panel                                                               */
/*************************************************************************************************/
void vog_lcd_panel_init( u1 u1t_lcd_mode )
{

	/* LCD PANEL RESET SET */
	vos_lcd_panel_reset( u1t_lcd_mode );

	/* LCD setting(LCDC0) */
	vog_mipi_dsi_reg_set( u1t_lcd_mode );

	/* 120ms Wait  */
	vog_timer_wait( 120, U1G_TIME_MS );

	/* LCD first data setting */
	vos_lcd_first_set();

	return;
}


/*************************************************************************************************/
/* function : lcd_panel_reset( int lcd_mode )                                                    */
/* parameter: int lcd_mode(0:LCD_MAIN, 1:LCD_SUB)                                                */
/* return   : void                                                                               */
/* outline  : LCD panel reset (GPIO control)                             */
/*************************************************************************************************/
static void vos_lcd_panel_reset( u1 u1t_lcd_mode )
{
	/* LCD power enable */
	vog_port_st_set( U2S_LCD_PANEL_POWER, U1S_PORT_OE );
	vog_port_set( U2S_LCD_PANEL_POWER, U1G_PORT_LO );

	/* LCD RESET PIN OUTPUT SET */
	vog_port_st_set( U2S_LCD_PANEL_RST_PORT, U1S_PORT_OE );

	/* LCD PANEL RESET CREAR */
	vog_port_set( U2S_LCD_PANEL_RST_PORT, U1G_PORT_HI );

	/* 5ms Wait */
	vog_timer_wait( 5, U1G_TIME_MS );

	/* LCD PANEL RESET */
	vog_port_set( U2S_LCD_PANEL_RST_PORT, U1G_PORT_LO );

	/* 5ms Wait */
	vog_timer_wait( 5, U1G_TIME_MS );

	/* Do not Unreset here ( move to vos_lcd_first_set ) */
#if 0
	/* LCD PANEL RESET CREAR */
	vog_port_set( U2S_LCD_PANEL_RST_PORT, U1G_PORT_HI );

	/* 15ms Wait */
	vog_timer_wait( 25, U1G_TIME_MS );
#endif /* 0 */
	return;
}

/*************************************************************************************************/
/* function : lcd_panel_display_ON( int lcd_mode )                                               */
/* parameter: int lcd_mode(0:LCD_MAIN, 1:LCD_SUB)                                                */
/* return   : void                                                                               */
/* outline  : LCD Panel display ON                                                               */
/*************************************************************************************************/
void vog_lcd_panel_display_on( u1 u1t_lcd_mode )
{
	/* set_display_on */
	vog_mipi_dsi_tr_spacket( DT_DCS_WRITE_1_PARAM, 0x29, 0x00 );	/* CMD=0x29 */

	vog_mipi_dsi_tr_spacket( DT_DCS_WRITE_1_PARAM, 0x51, 0xFF );	/* CMD=0x51 */

	vog_mipi_dsi_tr_spacket( DT_DCS_WRITE_1_PARAM, 0x53, 0x25 );	/* CMD=0x53 */

	return;
}

/*************************************************************************************************/
/* function : vos_lcd_first_set( void )                                                          */
/* parameter: void                                                                               */
/* return   : void                                                                               */
/* outline  : LCD Panel display INIT set                                                         */
/*************************************************************************************************/
static void vos_lcd_first_set( void )
{
	/* LDO enable */
	vog_port_set( U2S_LCD_PANEL_POWER, U1G_PORT_HI );

	vog_timer_wait(25, U1G_TIME_MS);
	vog_port_set( U2S_LCD_PANEL_RST_PORT, U1G_PORT_HI );
	vog_timer_wait(10, U1G_TIME_MS);

	int loop = 0;
	while(0 <= loop) {
		switch(s6e39a0x02_cmdset[loop].cmd){
		case MIPI_DSI_DCS_LONG_WRITE:
			vog_mipi_dsi_tr_lpacket(
				MIPI_DSI_DCS_LONG_WRITE,
				s6e39a0x02_cmdset[loop].size,
				(unsigned char*)s6e39a0x02_cmdset[loop].data,
				U1S_HSSEL_HIGH_SPEED);
			break;
		case MIPI_DSI_DCS_SHORT_WRITE_PARAM:
			vog_mipi_dsi_tr_spacket(
				MIPI_DSI_DCS_SHORT_WRITE_PARAM,
				s6e39a0x02_cmdset[loop].data[0],
				s6e39a0x02_cmdset[loop].data[1]);
			break;
		case MIPI_DSI_DCS_SHORT_WRITE:
			vog_mipi_dsi_tr_spacket(
				MIPI_DSI_DCS_SHORT_WRITE,
				s6e39a0x02_cmdset[loop].data[0],
				0);
			break;
		case MIPI_DSI_DELAY:
			vog_timer_wait(s6e39a0x02_cmdset[loop].size, U1G_TIME_MS);
			break;
		case MIPI_DSI_BLACK:
			{
				int line_num;
				int line_size = LCD_DISP_XMAX * LCD_RGB888_ONE_PIXEL + 1;
				static unsigned char line_data[LCD_DISP_XMAX * LCD_RGB888_ONE_PIXEL + 1];
				memset(line_data, 0, line_size);
				*line_data = 0x2C;
				vog_mipi_dsi_tr_lpacket(
					MIPI_DSI_DCS_LONG_WRITE,
					line_size,
					line_data,
					U1S_HSSEL_HIGH_SPEED);
				for (line_num=0; line_num < LCD_DISP_YMAX; line_num++) {
					*line_data = 0x3C;
					vog_mipi_dsi_tr_lpacket(
						MIPI_DSI_DCS_LONG_WRITE,
						line_size,
						line_data,
						U1S_HSSEL_HIGH_SPEED);
				}
			}
			break;
		case MIPI_DSI_END:
		default:
			loop = -2;
			break;
		}
		loop++;
	}

	return;
}

/*************************************************************************************************/
/* function : lcd_panel_display_OFF( int lcd_mode )                                              */
/* parameter: int lcd_mode(0:LCD_MAIN, 1:LCD_SUB)                                                */
/* return   : void                                                                               */
/* outline  : LCD Panel display OFF                                                              */
/*************************************************************************************************/
void vog_lcd_panel_display_off( u1 u1t_lcd_mode )
{
	/* Backlight of panel OFF */
	vos_lcd_backlight_off();

	/* set_display_off */
	vog_mipi_dsi_tr_spacket( DT_DCS_WRITE_NO_PARAM, 0x28, 0x00 );	/* CMD=0x28 */

	/* enter_sleep_mode */
	vog_mipi_dsi_tr_spacket( DT_DCS_WRITE_NO_PARAM, 0x10, 0x00 );	/* CMD=0x10 */

	return;
}


/*************************************************************************************************/
/* function : void lcd_panel_read( int lcd_mode )                                                */
/* parameter: int lcd_mode(0:LCD_MAIN, 1:LCD_SUB)                                                */
/* return   : void                                                                               */
/* outline  : LCD panel read                                                             */
/*************************************************************************************************/
void vog_lcd_panel_read( u1 u1t_lcd_mode )
{
}


/*************************************************************************************************/
/* function : lcd_Back_LED_ON( void )                                                            */
/* parameter: void                                                                               */
/* return   : void                                                                               */
/* outline  : The backlight is ON (Use TPU0TO3).                                               */
/*************************************************************************************************/
void vos_lcd_backlight_on( void )
{
	/* In AMOLED display case, backlight control is unnecessary. */
#if 0
	/*------------------------*/
	/* TPU Setting            */
	/*------------------------*/
	/* TPU0 operates */

	/* Timer counter STOP */
	U2R_TPU0_TSTR &= ~( U2S_TPU0_3_EN_SET );
	/* TPU0_TGRB3 counter clear, count clock = CPclk/1. */
	U2R_TPU0_TCR3 = U2S_TPU0_CTRL_SET;
	/* TPU0_TGRA3=>TPU0TO3=1 */
	U2R_TPU0_TIOR3 = U2S_TPU0TO3_ON;
	/* TGRB3 value setting */
	U2R_TPU0_TGR3B = U2S_TPU0_PWM_BASE;
	/* TGRA3 value setting DUTY50%  */
	U2R_TPU0_TGR3A = U2S_TPU0_PWM_LOW;
	/* PWM mode */
	U2R_TPU0_TMDR3 = U2S_TPU0_PWM_MODE;
	/* TPU0TO3 setting */
	vog_port_st_set( U2S_MLCD_WLED_EN_PORT, U1S_PWM_FUNCTION );

	/* Timer count count initialization */
	U2R_TPU0_TCNT3 = U2S_TPU0_CNT_CLEAR;
	/* Timer counter START(PWM OUTPUT) */
	U2R_TPU0_TSTR |= U2S_TPU0_3_EN_SET;
#endif
	return;
}

/*************************************************************************************************/
/* function : lcd_Back_LED_OFF( void )                                                           */
/* parameter: void                                                                               */
/* return   : void                                                                               */
/* outline  : lcd Back LED OFF                                                           */
/*************************************************************************************************/
void vos_lcd_backlight_off( void )
{
#if 0
	/* Timer counter STOP */
	U2R_TPU0_TSTR &= ~( U2S_TPU0_3_EN_SET );
	/* TPU0_TGRB3 counter clear, count clock = CPclk/1. */
	U2R_TPU0_TCR3 = U2S_TPU0_CTRL_SET;
	/* TPU0_TGRA3=>TPU0TO3=1 */
	U2R_TPU0_TIOR3 = U2S_TPU0TO3_ON;
	/* TGRB3 setting value  */
	U2R_TPU0_TGR3B = U2S_TPU0_PWM_BASE;
	/* TGRA3 setting value  DUTY 0%Setting */
	U2R_TPU0_TGR3A = U2S_TPU0_PWM_BASE;
	/* PWM mode */
	U2R_TPU0_TMDR3 = U2S_TPU0_PWM_MODE;
	/* TPU0TO3 setting */
	vog_port_st_set( U2S_MLCD_WLED_EN_PORT, U1S_PWM_FUNCTION );

	/* Timer count count initialization */
	U2R_TPU0_TCNT3 = U2S_TPU0_CNT_CLEAR;
	/* Timer counter START(PWM OUTPUT) */
	U2R_TPU0_TSTR |= U2S_TPU0_3_EN_SET;
#endif
	return;
}

