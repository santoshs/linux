/*
 * /drivers/video/av7100/av7100.c
 *
 * Copyright (C) 2012 Renesas Mobile Corporation. 
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>

#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/timer.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/switch.h>

#include <mach/r8a73734.h>
#include <video/hdmi.h>

#include "av7100_regs.h"
#include "av7100.h"
#include "av7100_fw.h"
#include "hdmi_local.h"


DEFINE_MUTEX(av7100_hw_mutex);
#define LOCK_AV7100_HW mutex_lock(&av7100_hw_mutex)
#define UNLOCK_AV7100_HW mutex_unlock(&av7100_hw_mutex)

#define DSI_MAX_LENGTH		65535

/*DSI data type */
#define MIPI_DSI_DCS_LONG_WRITE						0x39
#define MIPI_DSI_DCS_SHORT_WRITE					0x05
#define MIPI_DSI_DCS_SHORT_WRITE_PARAM				0x15
#define MIPI_DSI_SET_MAXIMUM_RETURN_PACKET_SIZE		0x37
#define MIPI_DSI_DCS_READ							0x06
	
/* DCS command */
#define MIPI_DCS_READ_MEMORY_START			0x2E
#define MIPI_DCS_FIRMWARE_DOWNLOAD			0xDB
#define MIPI_DCS_WRITE_COMMAND				0xDC
#define MIPI_DCS_NEXT_FIELD_TYPE			0xDA
#define MIPI_DCS_EXEC_COMMAND				0xDD
#define MIPI_DCS_WRITE_START_READ_ADDRESS	0xDF
#define MIPI_DCS_WRITE_REGISTER_VALUE		0xE0

#define HDMI_FUSE_AES_KEY_SIZE 16

/************************************************
*   	Global variable definition section	 	*
************************************************/ 
static struct av7100_globals_t *av7100_globals;
static struct av7100_config_t *av7100_config;
static struct platform_device *pf_dev = NULL;


static const struct av7100_cea av7100_all_cea[29] = {
/* cea id
 *	cea_nr	vtot	vact	vsbpp	vslen
 *	vsfp	vpol	htot	hact	hbp	hslen	hfp	freq
 *	hpol	rld	bd	uix4	pm	pd */
{ "0  CUSTOM                            ",
	0,	0,	0,	0,	0,
	0,	"-",	800,	640,	16,	96,	10,	25200000,
	"-",	0,	0,	0,	0,	0},/*Settings to be defined*/
{ "1  CEA 1 VESA 4 640x480p @ 60 Hz     ",
	1,	525,	480,	33,	2,
	10,	"-",	800,	640,	49,	290,	146,	25200000,
	"-",	2438,	1270,	6,	32,	1},/*RGB888*/
{ "2  CEA 2 - 3    720x480p @ 60 Hz 4:3 ",
	2,	525,	480,	30,	6,
	9,	"-",	858,	720,	34,	130,	128,	27027000,
	"-",	1828,	0x3C0,	8,	24,	1},/*RGB565*/
{ "3  CEA 4        1280x720p @ 60 Hz    ",
	4,	750,	720,	20,	5,
	5,	"+",	1650,	1280,	114,	39,	228,	74250000,
	"+",	1706,	164,	6,	32,	1},/*RGB565*/
{ "4  CEA 5        1920x1080i @ 60 Hz   ",
	5,	1125,	540,	20,	5,
	0,	"+",	2200,	1920,	88,	44,	10,	74250000,
	"+",	0,	0,	0,	0,	0},
{ "5  CEA 6-7      480i (NTSC)          ",
	6,	525,	240,	44,	5,
	0,	"-",	858,	720,	12,	64,	10,	13513513,
	"-",	0,	0,	0,	0,	0},
{ "6  CEA 14-15    480p @ 60 Hz         ",
	14,	525,	480,	44,	5,
	0,	"-",	858,	720,	12,	64,	10,	27027000,
	"-",	0,	0,	0,	0,	0},
{ "7  CEA 16       1920x1080p @ 60 Hz   ",
	16,	1125,	1080,	36,	5,
	0,	"+",	1980,	1280,	440,	40,	10,	133650000,
	"+",	0,	0,	0,	0,	0},
{ "8  CEA 17-18    720x576p @ 50 Hz     ",
	17,	625,	576,	44,	5,
	0,	"-",	864,	720,	12,	64,	10,	27000000,
	"-",	0,	0,	0,	0,	0},
{ "9  CEA 19       1280x720p @ 50 Hz    ",
	19,	750,	720,	25,	5,
	0,	"+",	1980,	1280,	440,	40,	10,	74250000,
	"+",	0,	0,	0,	0,	0},
{ "10 CEA 20       1920 x 1080i @ 50 Hz ",
	20,	1125,	540,	20,	5,
	0,	"+",	2640,	1920,	528,	44,	10,	74250000,
	"+",	0,	0,	0,	0,	0},
{ "11 CEA 21-22    576i (PAL)           ",
	21,	625,	288,	44,	5,
	0,	"-",	1728,	1440,	12,	64,	10,	27000000,
	"-",	0,	0,	0,	0,	0},
{ "12 CEA 29/30    576p                 ",
	29,	625,	576,	44,	5,
	0,	"-",	864,	720,	12,	64,	10,	27000000,
	"-",	0,	0,	0,	0,	0},
{ "13 CEA 31       1080p 50Hz           ",
	31,	1125,	1080,	44,	5,
	0,	"-",	2640,	1920,	12,	64,	10,	148500000,
	"-",	0,	0,	0,	0,	0},
{ "14 CEA 32       1920x1080p @ 24 Hz   ",
	32,	1125,	1080,	36,	5,
	4,	"+",	2750,	1920,	660,	44,	153,	74250000,
	"+",	2844,	0x530,	6,	32,	1},/*RGB565*/
{ "15 CEA 33       1920x1080p @ 25 Hz   ",
	33,	1125,	1080,	36,	5,
	4,	"+",	2640,	1920,	528,	44,	10,	74250000,
	"+",	0,	0,	0,	0,	0},
{ "16 CEA 34       1920x1080p @ 30Hz    ",
	34,	1125,	1080,	36,	5,
	4,	"+",	2200,	1920,	91,	44,	153,	74250000,
	"+",	2275,	0xAB,	6,	32,	1},/*RGB565*/
{ "17 CEA 60       1280x720p @ 24 Hz    ",
	60,	750,	720,	20,	5,
	5,	"+",	3300,	1280,	284,	50,	2276,	59400000,
	"+",	4266,	0xAD0,	5,	32,	1},/*RGB565*/
{ "18 CEA 61       1280x720p @ 25 Hz    ",
	61,	750,	720,	20,	5,
	5,	"+",	3960,	1280,	228,	39,	2503,	74250000,
	"+",	4096,	0x500,	5,	32,	1},/*RGB565*/
{ "19 CEA 62       1280x720p @ 30 Hz    ",
	62,	750,	720,	20,	5,
	5,	"+",	3300,	1280,	228,	39,	1820,	74250000,
	"+",	3413,	0x770,	5,	32,	1},/*RGB565*/
{ "20 VESA 9       800x600 @ 60 Hz      ",
	109,	628,	600,	28,	4,
	0,	"+",	1056,	800,	40,	128,	10,	20782080,
	"+",	0,	0,	0,	0,	0},
{ "21 VESA 14      848x480  @ 60 Hz     ",
	114,	500,	480,	20,	5,
	0,	"+",	1056,	848,	24,	80,	10,	31680000,
	"-",	0,	0,	0,	0,	0},
{ "22 VESA 16      1024x768 @ 60 Hz     ",
	116,	806,	768,	38,	6,
	0,	"-",	1344,	1024,	24,	135,	10,	65000000,
	"-",	0,	0,	0,	0,	0},
{ "23 VESA 22      1280x768 @ 60 Hz     ",
	122,	802,	768,	34,	4,
	0,	"+",	1688,	1280,	48,	160,	10,	81250000,
	"-",	0,	0,	0,	0,	0},
{ "24 VESA 23      1280x768 @ 60 Hz     ",
	123,	798,	768,	30,	7,
	0,	"+",	1664,	1280,	64,	128,	10,	79500000,
	"-",	0,	0,	0,	0,	0},
{ "25 VESA 27      1280x800 @ 60 Hz     ",
	127,	823,	800,	23,	6,
	0,	"+",	1440,	1280,	48,	32,	10,	71000000,
	"+",	0,	0,	0,	0,	0},
{ "26 VESA 28      1280x800 @ 60 Hz     ",
	128,	831,	800,	31,	6,
	0,	"+",	1680,	1280,	72,	128,	10,	83500000,
	"-",	0,	0,	0,	0,	0},
{ "27 VESA 39      1360x768 @ 60 Hz     ",
	139,	790,	768,	22,	5,
	0,	"-",	1520,	1360,	48,	32,	10,	72000000,
	"+",	0,	0,	0,	0,	0},
{ "28 VESA 81      1360x768 @ 60 Hz     ",
	181,	798,	768,	30,	5,
	0,	"+",	1776,	1360,	72,	136,	10,	84750000,
	"-",	0,	0,	0,	0,	0} 
};

static const struct av7100_color_space_conversion_format_cmd 
	col_cvt_identity = {
	.c0			= 0x0100,
	.c1			= 0x0000,
	.c2			= 0x0000,
	.c3			= 0x0000,
	.c4			= 0x0100,
	.c5			= 0x0000,
	.c6			= 0x0000,
	.c7			= 0x0000,
	.c8			= 0x0100,
	.aoffset	= 0x0000,
	.boffset	= 0x0000,
	.coffset	= 0x0000,
	.lmax		= 0xff,
	.lmin		= 0x00,
	.cmax		= 0xff,
	.cmin		= 0x00,
};

static const struct av7100_color_space_conversion_format_cmd
						col_cvt_identity_clamp_yuv = {
	.c0			= 0x0100,
	.c1			= 0x0000,
	.c2			= 0x0000,
	.c3			= 0x0000,
	.c4			= 0x0100,
	.c5			= 0x0000,
	.c6			= 0x0000,
	.c7			= 0x0000,
	.c8			= 0x0100,
	.aoffset	= 0x0000,
	.boffset	= 0x0000,
	.coffset	= 0x0000,
	.lmax		= 0xeb,
	.lmin		= 0x10,
	.cmax		= 0xf0,
	.cmin		= 0x10,
};

static const struct av7100_color_space_conversion_format_cmd 
	col_cvt_yuv422_to_rgb = {
	.c0			= 0x00ba,
	.c1			= 0x007d,
	.c2			= 0x0000,
	.c3			= 0xffa1,
	.c4			= 0x007d,
	.c5			= 0xffd3,
	.c6			= 0x0000,
	.c7			= 0x007d,
	.c8			= 0x00eb,
	.aoffset	= 0xff9b,
	.boffset	= 0x003e,
	.coffset	= 0xff82,
	.lmax		= 0xff,
	.lmin		= 0x00,
	.cmax		= 0xff,
	.cmin		= 0x00,
};

/***********************************************************************
* THIS SECTION IS DUMMY PART THAT USED FOR dsi_write_command, dsi_read_command
* FUNCTIONS
***********************************************************************/

enum dsi_output_mode {
	RT_DISPLAY_LCD1 = 1,
	RT_DISPLAY_LCD2,
	RT_DISPLAY_HDMI
};

typedef struct
{
	void*         handle;
	unsigned char data_id;
	unsigned char reg_address;
	unsigned char output_mode;
	unsigned char write_data;
} screen_disp_write_dsi_short;

typedef struct
{
	void*          handle;
	unsigned char  data_id;
	unsigned char  dummy;
	unsigned short data_count;
	unsigned char output_mode;
	unsigned char* write_data;
} screen_disp_write_dsi_long;

typedef struct
{
	void*         handle;
	unsigned char data_id;
	unsigned char reg_address;
	unsigned char output_mode;
	unsigned char write_data;
	unsigned short data_count;
	unsigned char* read_data;
} screen_disp_read_dsi_short;

/* Write LCD packet data */
int screen_display_write_dsi_short_packet(screen_disp_write_dsi_short*
	write_dsi_s)
{
	return 0;
}

/* Write LCD packet data */
int screen_display_write_dsi_long_packet(screen_disp_write_dsi_long*
	write_dsi_l)
{
	return 0;
}

int screen_display_read_dsi_short_packet(screen_disp_read_dsi_short*
	read_dsi_s)
{
	return 0;
}

void* screen_display_new(void)
{
	return NULL;
}
/************************************************************************
* DUMMY END
*************************************************************************/



/****************************************************************************
*	name	=	dsi_write_command
*	func	=	Write packet data to AV7100 via DSI interface
*	input	=	u8 data_type, u8 *data, u16 len
*	output	=	None
*	return	=	0, -EFAULT, -EINVAL
****************************************************************************/
static int dsi_write_command(u8 data_type, u8 *data, u16 len)
{
	int retval = 0;
	screen_disp_write_dsi_long *write_dsi_l;
	screen_disp_write_dsi_short *write_dsi_s;
	
	if (!data)
		return -EINVAL;
	
	if (len <= 2) {
		write_dsi_s = kzalloc(sizeof(screen_disp_write_dsi_short),
			GFP_KERNEL);
		if (!write_dsi_s)
			return -EFAULT;
			
		write_dsi_s->handle = screen_display_new();
		write_dsi_s->output_mode = RT_DISPLAY_HDMI;
		write_dsi_s->data_id = data_type;
		write_dsi_s->reg_address = *data;
		write_dsi_s->write_data = (len == 1) ? 0 : *(data + 1);
		
		/* Send short command packet to DSI */
		retval = screen_display_write_dsi_short_packet(write_dsi_s);
		if (retval)
			retval = -EFAULT;
		
		kfree(write_dsi_s);
		write_dsi_s = NULL;
	} else {
		write_dsi_l = kzalloc(sizeof(screen_disp_write_dsi_long),
			GFP_KERNEL);
		if (!write_dsi_l)
			return -EFAULT;
	
		write_dsi_l->handle = screen_display_new();
		write_dsi_l->output_mode = RT_DISPLAY_HDMI;
		write_dsi_l->data_id = data_type;		/* Data type */
		write_dsi_l->data_count = len;		/* Length of data buffer */
		
		/* Address of first byte of DSI data buffer */
		write_dsi_l->write_data = (unsigned char *) data;
		
		/* Send long command packet to DSI */
		retval = screen_display_write_dsi_long_packet(write_dsi_l);
		if (retval)
			retval = -EFAULT;
		
		kfree(write_dsi_l);
		write_dsi_l = NULL;
	}
	return retval;
}

/****************************************************************************
*	name	=	dsi_read_command
*	func	=	Read data from AV7100 via DSI interface
*	input	=	u8 read_address, u16 len, u8 *data
*	output	=	u8 *data
*	return	=	0, -EFAULT, -EINVAL
****************************************************************************/
static int dsi_read_command(u8 read_address, u8 *data, u16 len)
{
	int retval = 0;
	screen_disp_read_dsi_short* read_dsi_s;
	
	if (!data || len > 255)
		return -EINVAL;

	read_dsi_s = kzalloc(sizeof(screen_disp_read_dsi_short), GFP_KERNEL);
	if (!read_dsi_s)
		return -EFAULT;
	
	read_dsi_s->handle = screen_display_new();
	read_dsi_s->output_mode = RT_DISPLAY_HDMI;
	read_dsi_s->data_id = MIPI_DSI_DCS_READ;
	read_dsi_s->reg_address = read_address;
	read_dsi_s->write_data = 0x0;
	read_dsi_s->data_count = len;
		
	read_dsi_s->read_data = (unsigned char *) data;
	
	retval = screen_display_read_dsi_short_packet(read_dsi_s);
	if (retval)
		retval = -EFAULT;
		
	kfree(read_dsi_s);
	read_dsi_s = NULL;
	return retval;
}

/****************************************************************************
*	name	=	register_read_internal
*	func	=	Read data from specified register on AV7100 chip
*	input	=	u8 offset, u8 *value
*	output	=	u8 *value
*	return	=	0, -EFAULT, -EINVAL
****************************************************************************/
static int register_read_internal(u8 offset, u8 *value)
{
	u8 retval = 0;
	
	if (!value)
		return -EINVAL;
	
	retval = dsi_read_command(offset, value, 1);
	if (retval)
		return -EFAULT;
	
	return 0;
}

/****************************************************************************
*	name	=	register_write_internal
*	func	=	Write single byte data to specified register
*				on AV7100 chip
*	input	=	u8 offset, u8 value
*	output	=	None
*	return	=	0, -EFAULT
****************************************************************************/
static int register_write_internal(u8 offset, u8 value)
{
	u8 data_type = 0;
	int retval = 0;
	u8 data[3];
	memset(&data, 0, 3);

	data_type = MIPI_DSI_DCS_LONG_WRITE;	/* DCS long write command */
	data[0] = MIPI_DCS_WRITE_REGISTER_VALUE;/* Write_register_value command */
	data[1] = offset;	/* Data1 = offset address of register */
	data[2] = value;	/* Data2 = value to be written to register */
	
	/* Send long command packet via DSI */
	retval = dsi_write_command(data_type, data, 3);
	if (retval)
		retval = -EFAULT;
	
	return retval;
}

/*****************************************************************************
 *	name	=	av7100_reg_stby_r
 *	func	=	Read specified values from 
 *			standby configuration register
 *	input	=	u8 *stby, u8 *hpds, u8 *mclkrng
 *	output	=	u8 *stby, u8 *hpds, u8 *mclkrng
 *	return	=	0, -EFAULT, -EINVAL
  *****************************************************************************/
int av7100_reg_stby_r(u8 *stby, u8 *hpds, u8 *mclkrng)
{
	int retval = 0;
	u8 val = 0;

	LOCK_AV7100_HW;

	/* Read from register */
	retval = register_read_internal(AV7100_STANDBY, &val);
	if (retval < 0) {
		UNLOCK_AV7100_HW;
		return -EFAULT;
	}
	/* Set return params */
	if (stby)
		*stby = AV7100_STANDBY_STBY_GET(val);
	if (hpds)
		*hpds = AV7100_STANDBY_HPDS_GET(val);
	if (mclkrng)
		*mclkrng = AV7100_STANDBY_MCLKRNG_GET(val);

	UNLOCK_AV7100_HW;
	return retval;
}

/*****************************************************************************
 *	name	=	av7100_reg_stby_w
 *	func	=	Write specified values to 
 *			standby configuration register
 *	input	=	u8 stby, u8 mclkrng
 *	output	=	None
 *	return	=	0, -EFAULT
 *****************************************************************************/
int av7100_reg_stby_w(u8 stby, u8 mclkrng)
{
	int retval = 0;
	u8 val = 0;

	LOCK_AV7100_HW;
	
	/* Set register value */
	val = AV7100_STANDBY_STBY(stby) | AV7100_STANDBY_MCLKRNG(mclkrng);
	
	/* Write to register */
	retval = register_write_internal(AV7100_STANDBY, val);
	
	UNLOCK_AV7100_HW;
	return retval;
}

/*****************************************************************************
 *	name	=	av7100_reg_hdmi_5_volt_time_r
 *	func	=	Read specified values from HDMI 5 Volt time register
 *	input	=	u8 *hdmi_off_time, u8 *hdmi_on_time
 *	output	=	u8 *hdmi_off_time, u8 *hdmi_on_time
 *	return	=	0, -EFAULT, -EINVAL
  *****************************************************************************/
int av7100_reg_hdmi_5_volt_time_r(u8 *hdmi_off_time, u8 *hdmi_on_time)
{
	int retval = 0;
	u8 val = 0;
	
	LOCK_AV7100_HW;

	/* Read from register */
	retval = register_read_internal(AV7100_HDMI_5_VOLT_TIME, &val);
	if (retval < 0) {
		UNLOCK_AV7100_HW;
		return -EFAULT;
	}
	
	/* Set return params */
	if (hdmi_off_time)
		*hdmi_off_time = AV7100_HDMI_5_VOLT_TIME_SU_OFF_TIME_GET(val);
	if (hdmi_on_time)
		*hdmi_on_time = AV7100_HDMI_5_VOLT_TIME_SU_ON_TIME_GET(val);

	UNLOCK_AV7100_HW;
	return retval;
}

/*****************************************************************************
 *	name	=	av7100_reg_hdmi_5_volt_time_w
 *	func	=	Write specified values to HDMI 5 Volt time register
 *	input	=	u8 hdmi_off_time, u8 hdmi_on_time
 *	output	=	None
 *	return	=	0, -EFAULT
  *****************************************************************************/
int av7100_reg_hdmi_5_volt_time_w(u8 hdmi_off_time, u8 hdmi_on_time)
{
	int retval = 0;
	u8 val = 0;

	LOCK_AV7100_HW;

	/* Set register value.*/
	
	val = AV7100_HDMI_5_VOLT_TIME_SU_OFF_TIME(hdmi_off_time) |
			AV7100_HDMI_5_VOLT_TIME_SU_ON_TIME(hdmi_on_time);

	/* Write to register */
	retval = register_write_internal(AV7100_HDMI_5_VOLT_TIME, val);

	/* Set vars */
	av7100_globals->hdmi_off_time = hdmi_off_time;
	av7100_globals->hdmi_on_time = hdmi_on_time;

	UNLOCK_AV7100_HW;
	return retval;
}

/*************************************************************************
*	name	=	av7100_reg_stby_int_mask_r
*	func	=	Read specified value from standby interrupt mask register
*	input	=	u8 *hpdm, u8 *stbygpiocfg, u8 *ipol, u8 *ccm, u8 *onm
*	output	=	u8 *hpdm, u8 *stbygpiocfg, u8 *ipol, u8 *ccm, u8 *onm
*	return	=	0, -EFAULT, -EINVAL
*************************************************************************/
int av7100_reg_stby_int_mask_r(u8 *hpdm, u8 *stbygpiocfg,
							   u8 *ipol, u8 *ccm, u8 *onm)
{
	int retval = 0;
	u8 val = 0;

	LOCK_AV7100_HW;

	/* Read from register */
	retval = register_read_internal(AV7100_STANDBY_INTERRUPT_MASK, &val);
	
	if (retval < 0) {
		UNLOCK_AV7100_HW;
		return -EFAULT;
	}

	/* Set return params */
	if (hpdm)
		*hpdm = AV7100_STANDBY_INTERRUPT_MASK_HPDM_GET(val);
	if (stbygpiocfg)
		*stbygpiocfg =
			AV7100_STANDBY_INTERRUPT_MASK_STBYGPIOCFG_GET(val);
	if (ipol)
		*ipol = AV7100_STANDBY_INTERRUPT_MASK_IPOL_GET(val);
	if (ccm)
		*ccm = AV7100_STANDBY_INTERRUPT_MASK_CCM_GET(val);
	if (onm)
		*onm = AV7100_STANDBY_INTERRUPT_MASK_ONM_GET(val);

	UNLOCK_AV7100_HW;
	return 0;
}

/*************************************************************************
*	name	=	av7100_reg_stby_int_mask_w
*	func	=	Write specified values to 
*					general interrupt mask register
*	input	=	u8 hpdm, u8 stbygpiocfg,
*				u8 ipol, u8 ccm, u8 onm
*	output	=	None
*	return	=	0, -EFAULT
*************************************************************************/
int av7100_reg_stby_int_mask_w(u8 hpdm, u8 stbygpiocfg,
				u8 ipol, u8 ccm, u8 onm)
{
	int retval = 0;
	u8 val = 0;
	
	LOCK_AV7100_HW;
	
	/* Set register value */
	val = AV7100_STANDBY_INTERRUPT_MASK_HPDM(hpdm) |
		AV7100_STANDBY_INTERRUPT_MASK_STBYGPIOCFG(stbygpiocfg) |
		AV7100_STANDBY_INTERRUPT_MASK_IPOL(ipol) |
		AV7100_STANDBY_INTERRUPT_MASK_CCM(ccm) |
		AV7100_STANDBY_INTERRUPT_MASK_ONM(onm);
		
	/* Write to register */
	retval = register_write_internal(AV7100_STANDBY_INTERRUPT_MASK, val);
	
	av7100_globals->hpdm = hpdm;	
	
	UNLOCK_AV7100_HW;
	return retval;
}

/*************************************************************************
*	name	=	av7100_reg_stby_pend_int_r
*	func	=	Read specified values from
*					standby pending interrupt register
*	input	=	u8 *hpdi, u8 *oni, u8 *ccrst, u8 *cci
*	output	=	u8 *hpdi, u8 *oni, u8 *ccrst, u8 *cci
*	return	=	0, -EFAULT, -EINVAL
*************************************************************************/
int av7100_reg_stby_pend_int_r(u8 *hpdi, u8 *oni, u8 *ccrst, u8 *cci)
{
	int retval = 0;
	u8 val = 0;

	LOCK_AV7100_HW;

	/* Read from standby pending interrupt register */
	retval = register_read_internal(AV7100_STANDBY_PENDING_INTERRUPT, &val);
	if (retval)
		goto err_register_read;

	/* Set return params */
	if (hpdi)
		*hpdi = AV7100_STANDBY_PENDING_INTERRUPT_HPDI_GET(val);
	if (oni)
		*oni = AV7100_STANDBY_PENDING_INTERRUPT_ONI_GET(val);
	if (ccrst)
		*ccrst = AV7100_STANDBY_PENDING_INTERRUPT_CCRST_GET(val);
	if (cci)
		*cci = AV7100_STANDBY_PENDING_INTERRUPT_CCI_GET(val);

err_register_read:
	UNLOCK_AV7100_HW;
	return retval;
}

/*************************************************************************
*	name	=	av7100_reg_stby_pend_int_w
*	func	=	Write specified values to 
*					standby pending interrupt register
*	input	=	u8 hpdi, u8 oni, u8 ccrst, u8 cci
*	output	=	None
*	return	=	0, -EFAULT
*************************************************************************/
int av7100_reg_stby_pend_int_w(u8 hpdi, u8 oni, u8 ccrst, u8 cci)
{
	int retval = 0;
	u8 val = 0;

	LOCK_AV7100_HW;

	/* Set register value */
	val = AV7100_STANDBY_PENDING_INTERRUPT_HPDI(hpdi) |
		AV7100_STANDBY_PENDING_INTERRUPT_ONI(oni) |
		AV7100_STANDBY_PENDING_INTERRUPT_CCRST(ccrst) |
		AV7100_STANDBY_PENDING_INTERRUPT_CCI(cci);

	/* Write to register */
	retval = register_write_internal(AV7100_STANDBY_PENDING_INTERRUPT, val);

	UNLOCK_AV7100_HW;
	return retval;
}

/*************************************************************************
 *	name	=	av7100_reg_gen_int_mask_r
 *	func	=	Read specified values from general interrupt mask register
 *	input	=	u8 *eocm, u8 *vsim, u8 *vsom, u8 *cecm, u8 *hdcpm, u8 *uovbm
 *	output	=	u8 *eocm, u8 *vsim, u8 *vsom, u8 *cecm, u8 *hdcpm, u8 *uovbm
 *	return	=	0, -EFAULT, -EINVAL
 *************************************************************************/
int av7100_reg_gen_int_mask_r(u8 *eocm, u8 *vsim, u8 *vsom,
							  u8 *cecm, u8 *hdcpm, u8 *uovbm)
{
	int retval = 0;
	u8 val = 0;

	LOCK_AV7100_HW;

	/* Read from general interrupt mask register */
	retval = register_read_internal(AV7100_GENERAL_INTERRUPT_MASK, &val);
	if (retval)
		goto err_register_read;
		
	/* Set return params */
	if (eocm)
		*eocm = AV7100_GENERAL_INTERRUPT_MASK_EOCM_GET(val);
	if (vsim)
		*vsim = AV7100_GENERAL_INTERRUPT_MASK_VSIM_GET(val);
	if (vsom)
		*vsom = AV7100_GENERAL_INTERRUPT_MASK_VSOM_GET(val);
	if (cecm)
		*cecm = AV7100_GENERAL_INTERRUPT_MASK_CECM_GET(val);
	if (hdcpm)
		*hdcpm = AV7100_GENERAL_INTERRUPT_MASK_HDCPM_GET(val);
	if (uovbm)
		*uovbm = AV7100_GENERAL_INTERRUPT_MASK_UOVBM_GET(val);

err_register_read:
	UNLOCK_AV7100_HW;
	return retval;
}

/*************************************************************************
 *	name	=	av7100_reg_gen_int_mask_w
 *	func	=	Write specified values to general interrupt mask register
 *	input	=	u8 eocm, u8 vsim, u8 vsom, u8 cecm, u8 hdcpm, u8 uovbm
 *	output	=	None
 *	return	=	0, -EFAULT
 *************************************************************************/
int av7100_reg_gen_int_mask_w(u8 eocm, u8 vsim, u8 vsom,
							  u8 cecm, u8 hdcpm, u8 uovbm)
{
	int retval = 0;
	u8 val = 0;

	LOCK_AV7100_HW;

	/* Set register value */
	val = AV7100_GENERAL_INTERRUPT_MASK_EOCM(eocm) |
		AV7100_GENERAL_INTERRUPT_MASK_VSIM(vsim) |
		AV7100_GENERAL_INTERRUPT_MASK_VSOM(vsom) |
		AV7100_GENERAL_INTERRUPT_MASK_CECM(cecm) |
		AV7100_GENERAL_INTERRUPT_MASK_HDCPM(hdcpm) |
		AV7100_GENERAL_INTERRUPT_MASK_UOVBM(uovbm);

	/* Write to general interrupt mask register */
	retval = register_write_internal(AV7100_GENERAL_INTERRUPT_MASK,	val);

	UNLOCK_AV7100_HW;
	return retval;
}

/*************************************************************************
 *	name	=	av7100_reg_gen_int_r
 *	func	=	Read specified values from general interrupt register
 *	input	=	u8 *eoci, u8 *vsii, u8 *vsoi, u8 *ceci, u8 *hdcpi, u8 *uovbi
 *	output	=	u8 *eoci, u8 *vsii, u8 *vsoi, u8 *ceci, u8 *hdcpi, u8 *uovbi
 *	return	=	0, -EFAULT, -EINVAL
 *************************************************************************/
int av7100_reg_gen_int_r(u8 *eoci, u8 *vsii, u8 *vsoi,
						 u8 *ceci, u8 *hdcpi, u8 *uovbi)
{
	int retval = 0;
	u8 val = 0;

	LOCK_AV7100_HW;

	/* Read from general interrupt register */
	retval = register_read_internal(AV7100_GENERAL_INTERRUPT, &val);
	if (retval)
		goto err_register_read;
	
	/* Set return params */
	if (eoci)
		*eoci = AV7100_GENERAL_INTERRUPT_EOCI_GET(val);
	if (vsii)
		*vsii = AV7100_GENERAL_INTERRUPT_VSII_GET(val);
	if (vsoi)
		*vsoi = AV7100_GENERAL_INTERRUPT_VSOI_GET(val);
	if (ceci)
		*ceci = AV7100_GENERAL_INTERRUPT_CECI_GET(val);
	if (hdcpi)
		*hdcpi = AV7100_GENERAL_INTERRUPT_HDCPI_GET(val);
	if (uovbi)
		*uovbi = AV7100_GENERAL_INTERRUPT_UOVBI_GET(val);

err_register_read:
	UNLOCK_AV7100_HW;
	return retval;
}

/*************************************************************************
 *	name	=	av7100_reg_gen_int_w
 *	func	=	Write specified values to general interrupt register
 *	input	=	u8 eoci, u8 vsii, u8 vsoi, u8 ceci, u8 hdcpi, u8 uovbi
 *	output	=	None
 *	return	=	0, -EFAULT
 *************************************************************************/
int av7100_reg_gen_int_w(u8 eoci, u8 vsii, u8 vsoi, 
						 u8 ceci, u8 hdcpi, u8 uovbi)
{
	int retval = 0;
	u8 val = 0;

	LOCK_AV7100_HW;

	/* Set register value */
	val = AV7100_GENERAL_INTERRUPT_EOCI(eoci) |
		AV7100_GENERAL_INTERRUPT_VSII(vsii) |
		AV7100_GENERAL_INTERRUPT_VSOI(vsoi) |
		AV7100_GENERAL_INTERRUPT_CECI(ceci) |
		AV7100_GENERAL_INTERRUPT_HDCPI(hdcpi) |
		AV7100_GENERAL_INTERRUPT_UOVBI(uovbi);

	/* Write to general interrupt register */
	retval = register_write_internal(AV7100_GENERAL_INTERRUPT, val);
	
	UNLOCK_AV7100_HW;
	return retval;
}

/****************************************************************************
*	name	=	av7100_reg_gen_status_r
*	func	=	Read specified values from general status register
*	input	=	u8 *cectxe, u8 *cecrx, u8 *cectx, u8 *uc, u8 *onuvb, u8 *hdcps
*	output	=	u8 *cectxe, u8 *cecrx, u8 *cectx, u8 *uc, u8 *onuvb, u8 *hdcps
*	return	=	0, -EFAULT, -EINVAL
****************************************************************************/
int av7100_reg_gen_status_r(u8 *cectxe, u8 *cecrx, u8 *cectx, u8 *uc,
	u8 *onuvb, u8 *hdcps)
{
	int retval = 0;
	u8 val = 0;

	LOCK_AV7100_HW;

	/* Read from register */
	retval = register_read_internal(AV7100_GENERAL_STATUS, &val);
	if (retval < 0)
		goto av7100_reg_gen_status_r_out;
		
	/* Set return params */
	if (cectxe)
		*cectxe	= AV7100_GENERAL_STATUS_CECTXE_GET(val);
	if (cecrx)
		*cecrx	= AV7100_GENERAL_STATUS_CECRX_GET(val);
	if (cectx)
		*cectx	= AV7100_GENERAL_STATUS_CECTX_GET(val);
	if (uc)
		*uc = AV7100_GENERAL_STATUS_UC_GET(val);
	if (onuvb)
		*onuvb = AV7100_GENERAL_STATUS_ONUVB_GET(val);
	if (hdcps)
		*hdcps = AV7100_GENERAL_STATUS_HDCPS_GET(val);

av7100_reg_gen_status_r_out:
	UNLOCK_AV7100_HW;
	return retval;
}

/****************************************************************************
*    name	=	av7100_reg_gpio_conf_r
*	func	=	Read specified values from GPIO configuration register
*	input	=	u8 *dat3dir, u8 *dat3val, u8 *dat2dir, u8 *dat2val,
*				u8 *dat1dir, u8 *dat1val
*	output	=	u8 *dat3dir, u8 *dat3val, u8 *dat2dir, u8 *dat2val,
*				u8 *dat1dir, u8 *dat1val
*	return	=	0, -EFAULT, -EINVAL
****************************************************************************/
int av7100_reg_gpio_conf_r(u8 *dat3dir, u8 *dat3val, u8 *dat2dir, u8 *dat2val,
	u8 *dat1dir, u8 *dat1val)
{
	int retval = 0;
	u8 val = 0;

	LOCK_AV7100_HW;

	/* Read from register */
	retval = register_read_internal(AV7100_GPIO_CONFIGURATION, &val);
	if (retval < 0)
		goto av7100_reg_gpio_conf_r_out;
		
	/* Set return params */
	if (dat3dir)
		*dat3dir = AV7100_GPIO_CONFIGURATION_DAT3DIR_GET(val);
	if (dat3val)
		*dat3val = AV7100_GPIO_CONFIGURATION_DAT3VAL_GET(val);
	if (dat2dir)
		*dat2dir = AV7100_GPIO_CONFIGURATION_DAT2DIR_GET(val);
	if (dat2val)
		*dat2val = AV7100_GPIO_CONFIGURATION_DAT2VAL_GET(val);
	if (dat1dir)
		*dat1dir = AV7100_GPIO_CONFIGURATION_DAT1DIR_GET(val);
	if (dat1val)
		*dat1val = AV7100_GPIO_CONFIGURATION_DAT1VAL_GET(val);
		
av7100_reg_gpio_conf_r_out:
	UNLOCK_AV7100_HW;
	return retval;
}

/****************************************************************************
*    name	=	av7100_reg_gpio_conf_w
*    func	=	Write specified values to GPIO configuration register
*    input	=	u8 dat3dir, u8 dat3val, u8 dat2dir, u8 dat2val,
*				u8 dat1dir, u8 dat1val
*    output	=	None
*    return	=	0, -EFAULT
****************************************************************************/
int av7100_reg_gpio_conf_w(u8 dat3dir, u8 dat3val, u8 dat2dir,
	u8 dat2val, u8 dat1dir, u8 dat1val)
{
	int retval = 0; /*Store the return value of register write function */
	u8 val = 0; 

	LOCK_AV7100_HW;
	
	/* Prepare the new value for GPIO configuration register */
	val = AV7100_GPIO_CONFIGURATION_DAT3DIR(dat3dir) |
		AV7100_GPIO_CONFIGURATION_DAT3VAL(dat3val) |
		AV7100_GPIO_CONFIGURATION_DAT2DIR(dat2dir) |
		AV7100_GPIO_CONFIGURATION_DAT2VAL(dat2val) |
		AV7100_GPIO_CONFIGURATION_DAT1DIR(dat1dir) |
		AV7100_GPIO_CONFIGURATION_DAT1VAL(dat1val);
	
	/* Write value to GPIO configuration register */
	retval = register_write_internal(AV7100_GPIO_CONFIGURATION, val);
	
	UNLOCK_AV7100_HW;
	return retval;
}

/****************************************************************************
*	name	=	av7100_reg_gen_ctrl_r
*	func	=	Read specified values from general control register
*	input	=	u8 *fdl, u8 *hld, u8 *wa, u8 *ra
*	output	=	u8 *fdl, u8 *hld, u8 *wa, u8 *ra
*	return	=	0, -EFAULT, -EINVAL
****************************************************************************/
int av7100_reg_gen_ctrl_r(u8 *fdl, u8 *hld, u8 *wa, u8 *ra)
{
	int retval = 0;
	u8 val = 0;

	LOCK_AV7100_HW;

	/* Read from register */
	retval = register_read_internal(AV7100_GENERAL_CONTROL, &val);
	if (retval < 0)
		goto av7100_reg_gen_ctrl_r_out;
	
	/* Set return params */
	if (fdl)
		*fdl = AV7100_GENERAL_CONTROL_FDL_GET(val);
	if (hld)
		*hld = AV7100_GENERAL_CONTROL_HLD_GET(val);
	if (wa)
		*wa = AV7100_GENERAL_CONTROL_WA_GET(val);
	if (ra)
		*ra = AV7100_GENERAL_CONTROL_RA_GET(val);

av7100_reg_gen_ctrl_r_out:
	UNLOCK_AV7100_HW;
	return retval;
}

/****************************************************************************
*	name	=	av7100_reg_gen_ctrl_w
*	func	=	Write specified values to general control register
*	input	=	u8 fdl, u8 hld, u8 wa, u8 ra
*	output	=	None
*	return	=	0, -EFAULT
****************************************************************************/
int av7100_reg_gen_ctrl_w(u8 fdl, u8 hld, u8 wa, u8 ra)
{
	int retval = 0; /*Store the return value of register write function */
	u8 val = 0;

	LOCK_AV7100_HW;

	/* Prepare the new value for general control register */
	val = AV7100_GENERAL_CONTROL_FDL(fdl) |
		AV7100_GENERAL_CONTROL_HLD(hld) |
		AV7100_GENERAL_CONTROL_WA(wa) |
		AV7100_GENERAL_CONTROL_RA(ra);

	/* Write value to general control register */
	retval = register_write_internal(AV7100_GENERAL_CONTROL, val);
	
	UNLOCK_AV7100_HW;
	return retval;
}

/****************************************************************************
*	name	=	av7100_reg_fw_dl_entry_r
*	func	=	Write specified values to firmware download entry register
*	input	=	None
*	output	=	u8 *mbyte_code_entry
*	return	=	0, -EFAULT, -EINVAL
****************************************************************************/
int av7100_reg_fw_dl_entry_r(u8 *mbyte_code_entry)
{
	int retval = 0;
	u8 val = 0;

	LOCK_AV7100_HW;

	/* Read from register */
	retval = register_read_internal(AV7100_FIRMWARE_DOWNLOAD_ENTRY, &val);
	if (retval < 0)
		goto av7100_reg_fw_dl_entry_r_out;
	
	/* Set return param */
	if (mbyte_code_entry)
		*mbyte_code_entry = 
			AV7100_FIRMWARE_DOWNLOAD_ENTRY_MBYTE_CODE_ENTRY_GET(val);

av7100_reg_fw_dl_entry_r_out:
	UNLOCK_AV7100_HW;
	return retval;
}

/*************************************************************************
 *	name	=	av7100_get_te_line_nb
 *	func	=	Get the tearing effect line number based on the video 
 *				output format
 *	input	=	enum av7100_output_cea_vesa output_video_format
 *	output	=	None
 *	return	=	14, 21, 22, 30, 38, 40
 *************************************************************************/
static u8 av7100_get_te_line_nb(
	enum av7100_output_cea_vesa output_video_format)
{
	u8 retval = 0;

	switch (output_video_format) {
	case AV7100_CEA2_3_720X480P_60HZ:
		retval = AV7100_TE_LINE_NB_30;
		break;

	case AV7100_CEA4_1280X720P_60HZ:
		retval = AV7100_TE_LINE_NB_21;
		break;

	case AV7100_CEA17_18_720X576P_50HZ:
		retval = AV7100_TE_LINE_NB_40;
		break;

	case AV7100_CEA19_1280X720P_50HZ:
		retval = AV7100_TE_LINE_NB_22;
		break;

	case AV7100_CEA34_1920X1080P_30HZ:
		retval = AV7100_TE_LINE_NB_38;
		break;

	case AV7100_CEA16_1920X1080P_60HZ:
	case AV7100_CEA31_1920x1080P_50Hz:
	default:
		retval = AV7100_TE_LINE_NB_14;
		break;
	}

	return retval;
}

/*************************************************************************
 *	name	=	av7100_conf_get
 *	func	=	Get configuration data based on command type
 *	input	=	enum av7100_command_type command_type, 
 *				union av7100_configuration *config
 *	output	=	union av7100_configuration *config
 *	return	=	0, AV7100_FAIL
 *************************************************************************/
int av7100_conf_get(enum av7100_command_type command_type, 
	union av7100_configuration *config)
{
	if (!config)
		return AV7100_FAIL;
	switch (command_type) {
	case AV7100_COMMAND_VIDEO_INPUT_FORMAT:
		memcpy(&config->video_input_format,
			&av7100_config->hdmi_video_input_cmd,
			sizeof(struct av7100_video_input_format_cmd));
		break;
		
	case AV7100_COMMAND_AUDIO_INPUT_FORMAT:
		memcpy(&config->audio_input_format,
			&av7100_config->hdmi_audio_input_cmd,
			sizeof(struct av7100_audio_input_format_cmd));
		break;
		
	case AV7100_COMMAND_VIDEO_OUTPUT_FORMAT:
		memcpy(&config->video_output_format,
			&av7100_config->hdmi_video_output_cmd,
			sizeof(struct av7100_video_output_format_cmd));
		break;
		
	case AV7100_COMMAND_COLORSPACECONVERSION:
		memcpy(&config->color_space_conversion_format,
			&av7100_config->hdmi_color_space_conversion_cmd,
			sizeof(struct av7100_color_space_conversion_format_cmd));
		break;
		
	case AV7100_COMMAND_HDMI:
		memcpy(&config->hdmi_format, &av7100_config->hdmi_cmd,
			sizeof(struct av7100_hdmi_cmd));
		break;
		
	case AV7100_COMMAND_HDCP_SENDKEY:
		memcpy(&config->hdcp_send_key_format,
			&av7100_config->hdmi_hdcp_send_key_cmd,
			sizeof(struct av7100_hdcp_send_key_format_cmd));
		break;
		
	case AV7100_COMMAND_HDCP_MANAGEMENT:
		memcpy(&config->hdcp_management_format,
			&av7100_config->hdmi_hdcp_management_format_cmd,
			sizeof(struct av7100_hdcp_management_format_cmd));
		break;
		
	case AV7100_COMMAND_INFOFRAMES:
		memcpy(&config->infoframes_format,
			&av7100_config->hdmi_infoframes_cmd,
			sizeof(struct av7100_infoframes_format_cmd));
		break;
		
	case AV7100_COMMAND_EDID_SECTION_READBACK:
		memcpy(&config->edid_section_readback_format,
			&av7100_config->hdmi_edid_section_readback_cmd,
			sizeof(struct av7100_edid_section_readback_format_cmd));
		break;
		
	case AV7100_COMMAND_PATTERNGENERATOR:
		memcpy(&config->pattern_generator_format,
			&av7100_config->hdmi_pattern_generator_cmd,
			sizeof(struct av7100_pattern_generator_format_cmd));
		break;
		
	case AV7100_COMMAND_FUSE_AES_KEY:
		memcpy(&config->fuse_aes_key_format,
			&av7100_config->hdmi_fuse_aes_key_cmd,
			sizeof(struct av7100_fuse_aes_key_format_cmd));
		break;
		
	default:
		return AV7100_FAIL;
		break;
	}
	return 0;
}

/*****************************************************************************
 *	name	=	configuration_video_input_get
 *	func	=	Get configuration of video input
 *	input	=	char *buffer, unsigned int *length
 *	output	=	char *buffer, unsigned int *length
 *	return	=	0,	-EINVAL
  *****************************************************************************/
static int configuration_video_input_get(char *buffer, unsigned int *length)
{
	if (!buffer || !length)
		return -EINVAL;
	buffer[0] = av7100_config->hdmi_video_input_cmd.dsi_input_mode;
	buffer[1] = av7100_config->hdmi_video_input_cmd.input_pixel_format;
	buffer[2] = REG_16_8_MSB(av7100_config->hdmi_video_input_cmd.
		total_horizontal_pixel);
	buffer[3] = REG_16_8_LSB(av7100_config->hdmi_video_input_cmd.
		total_horizontal_pixel);
	buffer[4] = REG_16_8_MSB(av7100_config->hdmi_video_input_cmd.
		total_horizontal_active_pixel);
	buffer[5] = REG_16_8_LSB(av7100_config->hdmi_video_input_cmd.
		total_horizontal_active_pixel);
	buffer[6] = REG_16_8_MSB(av7100_config->hdmi_video_input_cmd.
		total_vertical_lines);
	buffer[7] = REG_16_8_LSB(av7100_config->hdmi_video_input_cmd.
		total_vertical_lines);
	buffer[8] = REG_16_8_MSB(av7100_config->hdmi_video_input_cmd.
		total_vertical_active_lines);
	buffer[9] = REG_16_8_LSB(av7100_config->hdmi_video_input_cmd.
		total_vertical_active_lines);
	buffer[10] = av7100_config->hdmi_video_input_cmd.video_mode;
	buffer[11] = av7100_config->hdmi_video_input_cmd.nb_data_lane;
	buffer[12] = av7100_config->hdmi_video_input_cmd.
		nb_virtual_ch_command_mode;
	buffer[13] = av7100_config->hdmi_video_input_cmd.
		nb_virtual_ch_video_mode;
	buffer[14] = REG_16_8_MSB(av7100_config->hdmi_video_input_cmd.
		TE_line_nb);
	buffer[15] = REG_16_8_LSB(av7100_config->hdmi_video_input_cmd.
		TE_line_nb);
	buffer[16] = av7100_config->hdmi_video_input_cmd.TE_config;
	buffer[17] = REG_32_8_MSB(av7100_config->hdmi_video_input_cmd.
		master_clock_freq);
	buffer[18] = REG_32_8_MMSB(av7100_config->hdmi_video_input_cmd.
		master_clock_freq);
	buffer[19] = REG_32_8_MLSB(av7100_config->hdmi_video_input_cmd.
		master_clock_freq);
	buffer[20] = REG_32_8_LSB(av7100_config->hdmi_video_input_cmd.
		master_clock_freq);
	buffer[21] = av7100_config->hdmi_video_input_cmd.ui_x4;
	
	/* length = 22 bytes; */
	*length = AV7100_COMMAND_VIDEO_INPUT_FORMAT_SIZE - 1;
	return 0;
}

/*****************************************************************************
 *	name	=	configuration_video_output_get
 *	func	=	Get configuration of video output
 *	input	=	char *buffer
 *	output	=	char *buffer, unsigned int *length
 *	return	=	0, -EINVAL
  *****************************************************************************/
static int configuration_video_output_get(char *buffer, unsigned int *length)
{
	if (!buffer || !length)
		return -EINVAL;

	buffer[0] = av7100_config->hdmi_video_output_cmd.video_output_cea_vesa;

	if (buffer[0] == AV7100_CUSTOM) {
		buffer[1] = av7100_config->hdmi_video_output_cmd.vsync_polarity;
		buffer[2] = av7100_config->hdmi_video_output_cmd.hsync_polarity;
		buffer[3] = REG_16_8_MSB(av7100_config->
			hdmi_video_output_cmd.total_horizontal_pixel);
		buffer[4] = REG_16_8_LSB(av7100_config->
			hdmi_video_output_cmd.total_horizontal_pixel);
		buffer[5] = REG_16_8_MSB(av7100_config->
			hdmi_video_output_cmd.total_horizontal_active_pixel);
		buffer[6] = REG_16_8_LSB(av7100_config->
			hdmi_video_output_cmd.total_horizontal_active_pixel);
		buffer[7] = REG_16_8_MSB(av7100_config->
			hdmi_video_output_cmd.total_vertical_in_half_lines);
		buffer[8] = REG_16_8_LSB(av7100_config->
			hdmi_video_output_cmd.total_vertical_in_half_lines);
		buffer[9] = REG_16_8_MSB(av7100_config->
			hdmi_video_output_cmd.total_vertical_active_in_half_lines);
		buffer[10] = REG_16_8_LSB(av7100_config->
			hdmi_video_output_cmd.total_vertical_active_in_half_lines);
		buffer[11] = REG_16_8_MSB(av7100_config->
			hdmi_video_output_cmd.hsync_start_in_pixel);
		buffer[12] = REG_16_8_LSB(av7100_config->
			hdmi_video_output_cmd.hsync_start_in_pixel);
		buffer[13] = REG_16_8_MSB(av7100_config->
			hdmi_video_output_cmd.hsync_length_in_pixel);
		buffer[14] = REG_16_8_LSB(av7100_config->
			hdmi_video_output_cmd.hsync_length_in_pixel);
		buffer[15] = REG_16_8_MSB(av7100_config->
			hdmi_video_output_cmd.vsync_start_in_half_line);
		buffer[16] = REG_16_8_LSB(av7100_config->
			hdmi_video_output_cmd.vsync_start_in_half_line);
		buffer[17] = REG_16_8_MSB(av7100_config->
			hdmi_video_output_cmd.vsync_length_in_half_line);
		buffer[18] = REG_16_8_LSB(av7100_config->
			hdmi_video_output_cmd.vsync_length_in_half_line);
		buffer[19] = REG_16_8_MSB(av7100_config->
			hdmi_video_output_cmd.hor_video_start_pixel);
		buffer[20] = REG_16_8_LSB(av7100_config->
			hdmi_video_output_cmd.hor_video_start_pixel);
		buffer[21] = REG_16_8_MSB(av7100_config->
			hdmi_video_output_cmd.vert_video_start_pixel);
		buffer[22] = REG_16_8_LSB(av7100_config->
			hdmi_video_output_cmd.vert_video_start_pixel);
		buffer[23] = av7100_config->hdmi_video_output_cmd.video_type;
		buffer[24] = av7100_config->hdmi_video_output_cmd.pixel_repeat;
		buffer[25] = REG_32_8_MSB(av7100_config->
			hdmi_video_output_cmd.pixel_clock_freq_Hz);
		buffer[26] = REG_32_8_MMSB(av7100_config->
			hdmi_video_output_cmd.pixel_clock_freq_Hz);
		buffer[27] = REG_32_8_MLSB(av7100_config->
			hdmi_video_output_cmd.pixel_clock_freq_Hz);
		buffer[28] = REG_32_8_LSB(av7100_config->
			hdmi_video_output_cmd.pixel_clock_freq_Hz);
		
		/* length = 29 bytes; */
		*length = AV7100_COMMAND_VIDEO_OUTPUT_FORMAT_SIZE - 1;
	} else {
		*length = 1;
	}

	return 0;
}

/*****************************************************************************
 *	name	=	configuration_audio_input_get
 *	func	=	Get configuration of audio input
 *	input	=	char *buffer, unsigned int *length
 *	output	=	char *buffer, unsigned int *length
 *	return	=	0,	-EINVAL
  *****************************************************************************/
static int configuration_audio_input_get(char *buffer, unsigned int *length)
{
	if (!buffer || !length)
		return -EINVAL;
	buffer[0] = av7100_config->hdmi_audio_input_cmd.
		audio_input_if_format;
	buffer[1] = av7100_config->hdmi_audio_input_cmd.i2s_input_nb;
	buffer[2] = av7100_config->hdmi_audio_input_cmd.sample_audio_freq;
	buffer[3] = av7100_config->hdmi_audio_input_cmd.audio_word_lg;
	buffer[4] = av7100_config->hdmi_audio_input_cmd.audio_format;
	buffer[5] = av7100_config->hdmi_audio_input_cmd.audio_if_mode;
	buffer[6] = av7100_config->hdmi_audio_input_cmd.audio_mute;

	/* length = 7 bytes; */
	*length = AV7100_COMMAND_AUDIO_INPUT_FORMAT_SIZE - 1;
	return 0;
}

/*****************************************************************************
 *	name	=	configuration_colorspace_conversion_get
 *	func	=	Get configuration of colorspace conversion
 *	input	=	char *buffer, unsigned int *length
 *	output	=	char *buffer, unsigned int *length
 *	return	=	0,	-EINVAL
  *****************************************************************************/
static int configuration_colorspace_conversion_get(char *buffer,
						unsigned int *length)
{
	if (!buffer || !length)
		return -EINVAL;
	buffer[0] = REG_16_8_MSB(av7100_config->hdmi_color_space_conversion_cmd.c0);
	buffer[1] = REG_16_8_LSB(av7100_config->hdmi_color_space_conversion_cmd.c0);
	buffer[2] = REG_16_8_MSB(av7100_config->hdmi_color_space_conversion_cmd.c1);
	buffer[3] = REG_16_8_LSB(av7100_config->hdmi_color_space_conversion_cmd.c1);
	buffer[4] = REG_16_8_MSB(av7100_config->hdmi_color_space_conversion_cmd.c2);
	buffer[5] = REG_16_8_LSB(av7100_config->hdmi_color_space_conversion_cmd.c2);
	buffer[6] = REG_16_8_MSB(av7100_config->hdmi_color_space_conversion_cmd.c3);
	buffer[7] = REG_16_8_LSB(av7100_config->hdmi_color_space_conversion_cmd.c3);
	buffer[8] = REG_16_8_MSB(av7100_config->hdmi_color_space_conversion_cmd.c4);
	buffer[9] = REG_16_8_LSB(av7100_config->hdmi_color_space_conversion_cmd.c4);
	buffer[10] = REG_16_8_MSB(av7100_config->
		hdmi_color_space_conversion_cmd.c5);
	buffer[11] = REG_16_8_LSB(av7100_config->
		hdmi_color_space_conversion_cmd.c5);
	buffer[12] = REG_16_8_MSB(av7100_config->
		hdmi_color_space_conversion_cmd.c6);
	buffer[13] = REG_16_8_LSB(av7100_config->hdmi_color_space_conversion_cmd.c6);
	buffer[14] = REG_16_8_MSB(av7100_config->
		hdmi_color_space_conversion_cmd.c7);
	buffer[15] = REG_16_8_LSB(av7100_config->
		hdmi_color_space_conversion_cmd.c7);
	buffer[16] = REG_16_8_MSB(av7100_config->
		hdmi_color_space_conversion_cmd.c8);
	buffer[17] = REG_16_8_LSB(av7100_config->
		hdmi_color_space_conversion_cmd.c8);
	buffer[18] = REG_16_8_MSB(av7100_config->
		hdmi_color_space_conversion_cmd.aoffset);
	buffer[19] = REG_16_8_LSB(av7100_config->
		hdmi_color_space_conversion_cmd.aoffset);
	buffer[20] = REG_16_8_MSB(av7100_config->
		hdmi_color_space_conversion_cmd.boffset);
	buffer[21] = REG_16_8_LSB(av7100_config->
		hdmi_color_space_conversion_cmd.boffset);
	buffer[22] = REG_16_8_MSB(av7100_config->
		hdmi_color_space_conversion_cmd.coffset);
	buffer[23] = REG_16_8_LSB(av7100_config->
		hdmi_color_space_conversion_cmd.coffset);
	buffer[24] = av7100_config->hdmi_color_space_conversion_cmd.lmax;
	buffer[25] = av7100_config->hdmi_color_space_conversion_cmd.lmin;
	buffer[26] = av7100_config->hdmi_color_space_conversion_cmd.cmax;
	buffer[27] = av7100_config->hdmi_color_space_conversion_cmd.cmin;

	/* length = 28 */
	*length = AV7100_COMMAND_COLORSPACECONVERSION_SIZE - 1;
	return 0;
}

/*************************************************************************
 *	name	=	configuration_hdmi_get
 *	func	=	Get configuration of HDMI
 *	input	=	None
 *	output	=	char *buffer, unsigned int *length
 *	return	=	0, -EINVAL
 *************************************************************************/
static int configuration_hdmi_get(char *buffer, unsigned int *length)
{
	if (!buffer || !length)
		return -EINVAL;

	buffer[0] = av7100_config->hdmi_cmd.hdmi_mode;
	buffer[1] = av7100_config->hdmi_cmd.hdmi_format;
	buffer[2] = av7100_config->hdmi_cmd.dvi_format;

	/* length = 3 bytes; */
	*length = AV7100_COMMAND_HDMI_SIZE - 1;
	return 0;
}

/*************************************************************************
 *	name	=	configuration_fuse_aes_key_get
 *	func	=	Get configuration of fuse AES key
 *	input	=	None 
 *	output	=	char *buffer, unsigned int *length
 *	return	=	0, -EINVAL
 *************************************************************************/
static int configuration_fuse_aes_key_get(char *buffer, unsigned int *length)
{
	if (!buffer || !length)
		return -EINVAL;

	buffer[0] = av7100_config->hdmi_fuse_aes_key_cmd.fuse_operation;
	memcpy(&buffer[1], av7100_config->hdmi_fuse_aes_key_cmd.key,
		HDMI_FUSE_AES_KEY_SIZE);

	/* length = 17 bytes; */
	*length = AV7100_COMMAND_FUSE_AES_KEY_SIZE - 1;
	return 0;
}

/*****************************************************************************
 *	name	=	configuration_hdcp_sendkey_get
 *	func	=	Get configuration of HDCP send key
 *	input	=	char *buffer
 *	output	=	char *buffer, unsigned int *length
 *	return	=	0, -EINVAL
  *****************************************************************************/
static int configuration_hdcp_sendkey_get(char *buffer, unsigned int *length)
{
	if (!buffer || !length)
		return -EINVAL;

	buffer[0] = av7100_config->hdmi_hdcp_send_key_cmd.key_number;
	memcpy(&buffer[1], av7100_config->hdmi_hdcp_send_key_cmd.data,
		av7100_config->hdmi_hdcp_send_key_cmd.data_len);
	
	/* length = data_len + 1; */
	*length = av7100_config->hdmi_hdcp_send_key_cmd.data_len + 1;
	return 0;
}

/*****************************************************************************
 *	name	=	configuration_hdcp_management_get
 *	func	=	Get configuration of HDCP management key
 *	input	=	char *buffer
 *	output	=	char *buffer, unsigned int *length
 *	return	=	0, -EINVAL
  *****************************************************************************/
static int configuration_hdcp_management_get(char *buffer,
	unsigned int *length)
{
	if (!buffer || !length)
		return -EINVAL;

	buffer[0] = av7100_config->hdmi_hdcp_management_format_cmd.req_type;
	buffer[1] = av7100_config->hdmi_hdcp_management_format_cmd.req_encr;
	
	/* length = 2 bytes; */
	*length = AV7100_COMMAND_HDCP_MANAGEMENT_SIZE - 1;
	return 0;
}

/*****************************************************************************
 *	name	=	configuration_infoframe_get
 *	func	=	Get configuration of Infoframe
 *	input	=	char *buffer
 *	output	=	char *buffer, unsigned int *length
 *	return	=	0, -EINVAL
  *****************************************************************************/
static int configuration_infoframe_get(char *buffer, unsigned int *length)
{
	if (!buffer || !length)
		return -EINVAL;

	buffer[0] = av7100_config->hdmi_infoframes_cmd.type;
	buffer[1] = av7100_config->hdmi_infoframes_cmd.version;
	buffer[2] = av7100_config->hdmi_infoframes_cmd.length;
	buffer[3] = av7100_config->hdmi_infoframes_cmd.crc;
	memcpy(&buffer[4], av7100_config->hdmi_infoframes_cmd.data,
	HDMI_INFOFRAME_DATA_SIZE);

	/* length = 32 bytes; */
	*length = AV7100_COMMAND_INFOFRAMES_SIZE - 1;
	return 0;
}

/*****************************************************************************
 *	name	=	configuration_edid_section_readback_get
 *	func	=	Get configuration of EDID section read back
 *	input	=	char *buffer
 *	output	=	char *buffer, unsigned int *length
 *	return	=	0, -EINVAL
  *****************************************************************************/
static int configuration_edid_section_readback_get(char *buffer,
	unsigned int *length)
{
	if (!buffer || !length)
		return -EINVAL;
		
	buffer[0] = av7100_config->hdmi_edid_section_readback_cmd.address;
	buffer[1] = av7100_config->hdmi_edid_section_readback_cmd.block_number;

	/* length = 2 bytes; */
	*length = AV7100_COMMAND_EDID_SECTION_READBACK_SIZE - 1;
	return 0;
}

/*****************************************************************************
 *	name	=	configuration_pattern_generator_get
 *	func	=	Get configuration of pattern generator
 *	input	=	char *buffer
 *	output	=	char *buffer, unsigned int *length
 *	return	=	0, -EINVAL
  *****************************************************************************/
static int configuration_pattern_generator_get(char *buffer,
	unsigned int *length)
{
	if (!buffer || !length)
		return -EINVAL;

	buffer[0] = av7100_config->hdmi_pattern_generator_cmd.pattern_type;
	buffer[1] = av7100_config->hdmi_pattern_generator_cmd.
		pattern_video_format;
	buffer[2] = av7100_config->hdmi_pattern_generator_cmd.
		pattern_audio_mode;

	/* length = 3 bytes; */
	*length = AV7100_COMMAND_PATTERNGENERATOR_SIZE - 1;
	return 0;
}

/****************************************************************************
*	name	=	av7100_config_video_output_dep
*	func	=	Set video input configuration based on video output format
*	input	=	enum av7100_output_CEA_VESA output_format
*	output	=	None
*	return	=	0, AV7100_FAIL
****************************************************************************/
static int av7100_config_video_output_dep(enum av7100_output_cea_vesa
	output_format)
{
	int retval = 0;
	union av7100_configuration config;
	
	/* Prepare setting values for video input format */

	config.video_input_format.dsi_input_mode =
		AV7100_HDMI_DSI_COMMAND_MODE;
	config.video_input_format.input_pixel_format = AV7100_INPUT_PIX_RGB565;
	config.video_input_format.total_horizontal_pixel =
		av7100_all_cea[output_format].htotale;
	config.video_input_format.total_horizontal_active_pixel =
		av7100_all_cea[output_format].hactive;
	config.video_input_format.total_vertical_lines =
		av7100_all_cea[output_format].vtotale;
	config.video_input_format.total_vertical_active_lines =
		av7100_all_cea[output_format].vactive;
		
	config.video_input_format.video_mode = AV7100_VIDEO_PROGRESSIVE;
	
	config.video_input_format.nb_data_lane =
		AV7100_DATA_LANES_USED_4;
	config.video_input_format.nb_virtual_ch_command_mode = 0;
	config.video_input_format.nb_virtual_ch_video_mode = 0;
	config.video_input_format.ui_x4 = 6;
	config.video_input_format.TE_line_nb = av7100_get_te_line_nb(
		output_format);
		
	config.video_input_format.TE_config = AV7100_TE_IT_LINE;
	config.video_input_format.master_clock_freq = 0;
	
	retval = av7100_conf_prep(
		AV7100_COMMAND_VIDEO_INPUT_FORMAT, &config);
	
	return retval;
}

/****************************************************************************
*	name	=	av7100_conf_prep
*	func	=	Prepare configuration data to 
*				the corresponding data structure depending on command type
*	input	=	enum av7100_command_type command_type,
*				union av7100_configuration *config
*	output	=	union av7100_configuration *config
*	return	=	0, AV7100_FAIL
****************************************************************************/
int av7100_conf_prep(enum av7100_command_type command_type,
				union av7100_configuration *config)
{
	if (!config)
		return AV7100_FAIL;

	/* Put configuration data to the corresponding data struct depending
	 * on command type */
	switch (command_type) {
	case AV7100_COMMAND_VIDEO_INPUT_FORMAT:
		memcpy(&av7100_config->hdmi_video_input_cmd,
			&config->video_input_format,
			sizeof(struct av7100_video_input_format_cmd));
		break;

	case AV7100_COMMAND_AUDIO_INPUT_FORMAT:
		memcpy(&av7100_config->hdmi_audio_input_cmd,
			&config->audio_input_format,
			sizeof(struct av7100_audio_input_format_cmd));
		break;

	case AV7100_COMMAND_VIDEO_OUTPUT_FORMAT:
		memcpy(&av7100_config->hdmi_video_output_cmd,
			&config->video_output_format,
			sizeof(struct av7100_video_output_format_cmd));

		/* Set params that depend on video output */
		av7100_config_video_output_dep(av7100_config->
			hdmi_video_output_cmd.video_output_cea_vesa);
		break;

	case AV7100_COMMAND_COLORSPACECONVERSION:
		memcpy(&av7100_config->hdmi_color_space_conversion_cmd,
			&config->color_space_conversion_format,
			sizeof(struct av7100_color_space_conversion_format_cmd));
		break;

	case AV7100_COMMAND_HDMI:
		memcpy(&av7100_config->hdmi_cmd, &config->hdmi_format,
				sizeof(struct av7100_hdmi_cmd));
		break;

	case AV7100_COMMAND_HDCP_SENDKEY:
		memcpy(&av7100_config->hdmi_hdcp_send_key_cmd,
			&config->hdcp_send_key_format,
			sizeof(struct av7100_hdcp_send_key_format_cmd));
		break;

	case AV7100_COMMAND_HDCP_MANAGEMENT:
		memcpy(&av7100_config->hdmi_hdcp_management_format_cmd,
			&config->hdcp_management_format,
			sizeof(struct av7100_hdcp_management_format_cmd));
		break;

	case AV7100_COMMAND_INFOFRAMES:
		memcpy(&av7100_config->hdmi_infoframes_cmd,
			&config->infoframes_format,
			sizeof(struct av7100_infoframes_format_cmd));
		break;

	case AV7100_COMMAND_EDID_SECTION_READBACK:
		memcpy(&av7100_config->hdmi_edid_section_readback_cmd,
			&config->edid_section_readback_format,
			sizeof(struct av7100_edid_section_readback_format_cmd));
		break;

	case AV7100_COMMAND_PATTERNGENERATOR:
		memcpy(&av7100_config->hdmi_pattern_generator_cmd,
			&config->pattern_generator_format,
			sizeof(struct av7100_pattern_generator_format_cmd));
		break;

	case AV7100_COMMAND_FUSE_AES_KEY:
		memcpy(&av7100_config->hdmi_fuse_aes_key_cmd,
			&config->fuse_aes_key_format,
			sizeof(struct av7100_fuse_aes_key_format_cmd));
		break;

	default:
		return AV7100_FAIL;
		break;
	}
	return 0;
}

/****************************************************************************
*	name	=	get_command_return_data
*	func	=	Get return data when write command
*	input	=	enum av7100_command_type command_type, u8 *command_buffer,
*				u8 *buffer_length, u8 *buffer
*	output	=	u8 *buffer_length, u8 *buffer
*	return	=	0, -EFAULT, -EINVAL, AV7100_FAIL, AV7100_INVALID_COMMAND
****************************************************************************/
static int get_command_return_data(enum av7100_command_type command_type,
	u8 *command_buffer, u8 *buffer_length, u8 *buffer)
{
	int retval = 0;
	char val = 0;
	
	/* Get the first return byte at address 0x10 */
	retval = dsi_read_command(AV7100_COMMAND_OFFSET, &val, 1);
	if (retval)
		return -EFAULT;

	/* Check the configuration command */
	if (val != (0x80 | command_type))
		return AV7100_FAIL;

	
	switch (command_type) {
	case AV7100_COMMAND_VIDEO_INPUT_FORMAT:
	case AV7100_COMMAND_AUDIO_INPUT_FORMAT:
	case AV7100_COMMAND_VIDEO_OUTPUT_FORMAT:
	case AV7100_COMMAND_COLORSPACECONVERSION:
	case AV7100_COMMAND_HDMI:
	case AV7100_COMMAND_HDCP_SENDKEY:
	case AV7100_COMMAND_INFOFRAMES:
	case AV7100_COMMAND_PATTERNGENERATOR:
		/* Get the second return byte at address 0x11*/
		retval = dsi_read_command(AV7100_COMMAND_OFFSET + 1, &val, 1);
		if (retval)
			return -EFAULT;

		if (val)
			return AV7100_FAIL;

		break;
		
	case AV7100_COMMAND_HDCP_MANAGEMENT:
		/* Get the second return byte at address 0x11*/
		retval = dsi_read_command(AV7100_COMMAND_OFFSET + 1, &val, 1);
		if (retval)
			return -EFAULT;

		/* val = 0 means OK */
		if (val)
			return AV7100_FAIL;
			
		if (command_buffer == NULL)	
			return -EINVAL;		
			
		/* Prepare the return buffer length */
		if (command_buffer[0] == HDMI_REQUEST_FOR_REVOCATION_LIST_INPUT) {
		
			/* Check input buffer and input buffer_length */
			if ((buffer == NULL) ||	(buffer_length == NULL))
				return -EINVAL;
				
			*buffer_length = 0x06;
			
			/* Get the return buffer */
			retval = dsi_read_command(AV7100_COMMAND_OFFSET + 2, 
						buffer, *buffer_length);
			if (retval) {
				*buffer_length = 0;
				return -EFAULT;
			}
		}
	
		break;
	case AV7100_COMMAND_EDID_SECTION_READBACK:
		/* Check input buffer and input buffer_length */
		if ((buffer == NULL) ||	(buffer_length == NULL))
			return -EINVAL;

		/* Return buffer length is fixed */
		*buffer_length = 0x80;
		
		/* Get the return buffer */
		retval = dsi_read_command(AV7100_COMMAND_OFFSET + 1,
						buffer, *buffer_length);
		if (retval) {
			*buffer_length = 0;
			return -EFAULT;
		}
		
		break;
		
	case AV7100_COMMAND_FUSE_AES_KEY:
		/* Get the second return byte at address 0x11*/
		retval = dsi_read_command(AV7100_COMMAND_OFFSET + 1, &val, 1);
		if (retval)
			return -EFAULT;

		/* val = 0 means OK */
		if (val)
			return AV7100_FAIL;
	
		/* Check input buffer and input buffer_length */
		if ((buffer == NULL) ||	(buffer_length == NULL))
			return -EINVAL;
	
		/* Return buffer length is fixed */
		*buffer_length = 0x2;
	
		/* Get CRC and programmed status by reading at address 0x12 */
		retval = dsi_read_command(AV7100_COMMAND_OFFSET + 2,
						buffer, *buffer_length);
		if (retval) {
			*buffer_length = 0;
			return -EFAULT;
		}
	
		break;
		
	default:
		return AV7100_INVALID_COMMAND;
		break;
	}
	
	return 0;
}

/*************************************************************************
 *	name	=	av7100_conf_w
 *	func	=	Write configuration data to AV7100 and get return data
 *	input	=	enum av7100_command_type command_type
 *	output	=	u8 *return_buffer_length, u8 *return_buffer
 *	return	=	0, -14, 0x01, 0xFF
 *************************************************************************/
int av7100_conf_w(enum av7100_command_type command_type, 
				  u8 *return_buffer_length, u8 *return_buffer)
{
	int retval = 0;
	u8 cmd_buffer[AV7100_COMMAND_MAX_LENGTH];
	u8 dsi_buffer[AV7100_COMMAND_MAX_LENGTH + 1];
	u32 cmd_length = 0;

	if (return_buffer_length)
		*return_buffer_length = 0;

	memset(&cmd_buffer, 0x00, AV7100_COMMAND_MAX_LENGTH);

	/* Fill the command buffer with configuration data */
	switch (command_type) {
	case AV7100_COMMAND_VIDEO_INPUT_FORMAT:
		configuration_video_input_get(cmd_buffer, &cmd_length);
		break;

	case AV7100_COMMAND_AUDIO_INPUT_FORMAT:
		configuration_audio_input_get(cmd_buffer, &cmd_length);
		break;

	case AV7100_COMMAND_VIDEO_OUTPUT_FORMAT:
		configuration_video_output_get(cmd_buffer, &cmd_length);
		break;

	case AV7100_COMMAND_COLORSPACECONVERSION:
		configuration_colorspace_conversion_get(cmd_buffer, &cmd_length);
		break;

	case AV7100_COMMAND_HDMI:
		configuration_hdmi_get(cmd_buffer, &cmd_length);
		break;

	case AV7100_COMMAND_HDCP_SENDKEY:
		configuration_hdcp_sendkey_get(cmd_buffer, &cmd_length);
		break;

	case AV7100_COMMAND_HDCP_MANAGEMENT:
		configuration_hdcp_management_get(cmd_buffer, &cmd_length);
		break;

	case AV7100_COMMAND_INFOFRAMES:
		configuration_infoframe_get(cmd_buffer, &cmd_length);
		break;

	case AV7100_COMMAND_EDID_SECTION_READBACK:
		configuration_edid_section_readback_get(cmd_buffer, &cmd_length);
		break;

	case AV7100_COMMAND_PATTERNGENERATOR:
		configuration_pattern_generator_get(cmd_buffer, &cmd_length);
		break;

	case AV7100_COMMAND_FUSE_AES_KEY:
		configuration_fuse_aes_key_get(cmd_buffer, &cmd_length);
		break;

	default:
		return AV7100_INVALID_COMMAND;
		break;
	}

	LOCK_AV7100_HW;

	/* Write the data from @0x11 to @0xN in the UC memory space */
	dsi_buffer[0] = MIPI_DCS_WRITE_COMMAND;
	memcpy((void*)&dsi_buffer[1], (void *)&cmd_buffer[0], 
		   AV7100_COMMAND_MAX_LENGTH);
	/* Write the command buffer */
	retval = dsi_write_command(MIPI_DSI_DCS_LONG_WRITE, 
		dsi_buffer, cmd_length + 1);
	if (retval)
		goto err_write_data;

	/* Write the command number @0x10 in the UC memory space, thus starting 
	 * the execution of the command loaded by the write_uc_command */
	dsi_buffer[0] = MIPI_DCS_EXEC_COMMAND;
	dsi_buffer[1] = command_type;
	/* Write the command type */
	retval = dsi_write_command(MIPI_DSI_DCS_SHORT_WRITE_PARAM, 
		dsi_buffer, 2);
	if (retval)
		goto err_write_data;

	/* Delay 100ms to get return data */
	mdelay(100);

	retval = get_command_return_data(command_type, cmd_buffer,
		return_buffer_length, return_buffer);

err_write_data:
	UNLOCK_AV7100_HW;
	return retval;
}

/*************************************************************************
 *	name	=	av7100_hdmi_config
 *	func	=	Configure HDMI mode for AV7100
 *	input	=	u8 hdmi_mode
 *	output	=	None
 *	return	=	0, AV7100_FAIL, AV7100_INVALID_COMMAND, Other negative values
 *************************************************************************/
int av7100_hdmi_config(u8 hdmi_mode)
{
	union av7100_configuration config;
	int retval = 0;
	
	config.hdmi_format.hdmi_mode = hdmi_mode;
	config.hdmi_format.hdmi_format	= AV7100_HDMI;
	
	retval = av7100_conf_prep(AV7100_COMMAND_HDMI, &config);
	if (retval)
		return AV7100_FAIL;
	
	retval = av7100_conf_w(AV7100_COMMAND_HDMI, NULL, NULL);

	return retval;
} 

/****************************************************************************
*	name	=	av7100_set_state
*	func	=	Set current operation mode of AV7100 to
*				global structure for further usage
*	input	=	enum av7100_operating_mode state
*	output	=	None
*	return	=	None
****************************************************************************/
void av7100_set_state(enum av7100_operating_mode state)
{
	/* Set current operation mode of AV7100 to global structure */
	av7100_globals->av7100_state = state;
}

/****************************************************************************
*	name	=	av7100_get_status
*	func	=	Get the current status structure of AV7100
*	input	=	None
*	output	=	None
*	return	=	NULL, !NULL
****************************************************************************/
enum av7100_operating_mode av7100_get_state(void)
{
	return av7100_globals->av7100_state;
}

/*************************************************************************
 *	name	=	av7100_config_init
 *	func	=	Initialize some configurations for AV7100
 *	input	=	None
 *	output	=	None
 *	return	=	0, 0xFF
 *************************************************************************/
static int av7100_config_init(void)
{
	int retval = 0;
	union av7100_configuration config;

	/* Allocate memory for av7100_config global variable */
	av7100_config = kzalloc(sizeof(struct av7100_config_t), GFP_KERNEL);
	if (!av7100_config)
		return AV7100_FAIL;

	/* Initialize configuration data */
	memset(&config, 0, sizeof(union av7100_configuration));

	/* Prepare configuration for color conversion */
	config.color_space_conversion_format = col_cvt_identity;
	
	retval = av7100_conf_prep(AV7100_COMMAND_COLORSPACECONVERSION, &config);
	if (retval)
		goto err_conf_prep;

	/* Prepare configuration for video output */
	config.video_output_format.video_output_cea_vesa =
		AV7100_CEA34_1920X1080P_30HZ;

	retval = av7100_conf_prep(AV7100_COMMAND_VIDEO_OUTPUT_FORMAT, &config);
	if (retval)
		goto err_conf_prep;

	/* Prepare configuration for video input based on video output formats*/
	av7100_config_video_output_dep(
		config.video_output_format.video_output_cea_vesa);

	/* Prepare configuration for pattern generator */
	config.pattern_generator_format.pattern_audio_mode =
		AV7100_PATTERN_AUDIO_OFF;
	config.pattern_generator_format.pattern_type =
		AV7100_PATTERN_GENERATOR;
	config.pattern_generator_format.pattern_video_format =
		AV7100_PATTERN_720P;
	
	retval = av7100_conf_prep(AV7100_COMMAND_PATTERNGENERATOR, &config);
	if (retval)
		goto err_conf_prep;

	/* Prepare configuration for audio input */
	config.audio_input_format.audio_input_if_format	=
		AV7100_AUDIO_I2S_MODE;
	config.audio_input_format.i2s_input_nb = 1;
	config.audio_input_format.sample_audio_freq = AV7100_AUDIO_FREQ_48KHZ;
	config.audio_input_format.audio_word_lg = AV7100_AUDIO_16BITS;
	config.audio_input_format.audio_format = AV7100_AUDIO_LPCM_MODE;
	config.audio_input_format.audio_if_mode = AV7100_AUDIO_MASTER;
	config.audio_input_format.audio_mute = AV7100_AUDIO_MUTE_DISABLE;
	
	retval = av7100_conf_prep(AV7100_COMMAND_AUDIO_INPUT_FORMAT, &config);
	if (retval)
		goto err_conf_prep;

	/* Prepare configuration for HDMI mode */
	config.hdmi_format.hdmi_mode	= AV7100_HDMI_ON;
	config.hdmi_format.hdmi_format	= AV7100_HDMI;
	config.hdmi_format.dvi_format	= AV7100_DVI_CTRL_CTL0;
	
	retval = av7100_conf_prep(AV7100_COMMAND_HDMI, &config);
	if (retval)
		goto err_conf_prep;

	/* Prepare configuration for EDID section read-back */
	config.edid_section_readback_format.address = 0xA0;
	config.edid_section_readback_format.block_number = 0;
	
	retval = av7100_conf_prep(AV7100_COMMAND_EDID_SECTION_READBACK, &config);
	if (retval)
		goto err_conf_prep;

	return 0;
	
err_conf_prep:
	kfree(av7100_config);
	av7100_config = NULL;
	
	return AV7100_FAIL;
}

/*************************************************************************
 *	name	=	av7100_globals_init
 *	func	=	Allocate memory for global structure av7100_globals 
 *				and initialize its setting
 *	input	=	None
 *	output	=	None
 *	return	=	0, 0xFF
 *************************************************************************/
static int av7100_globals_init(void)
{
	av7100_globals = kzalloc(sizeof(struct av7100_globals_t), GFP_KERNEL);
	if (!av7100_globals)
		return AV7100_FAIL;

	av7100_globals->hdmi_off_time = AV7100_HDMI_OFF_TIME;
	av7100_globals->hdmi_on_time = AV7100_HDMI_ON_TIME;
	av7100_globals->hpdm = AV7100_STANDBY_INTERRUPT_MASK_HPDM_LOW;

	return 0;
}

/*************************************************************************
 *	name	=	av7100_config_exit
 *	func	=	Release memory of av7100_config variable
 *	input	=	None
 *	output	=	None
 *	return	=	None
 *************************************************************************/
static void av7100_config_exit(void)
{
	kfree(av7100_config);
	av7100_config = NULL;
}

/*************************************************************************
 *	name	=	av7100_globals_exit
 *	func	=	Release memory of av7100_globals variable
 *	input	=	None
 *	output	=	None
 *	return	=	None
 *************************************************************************/
static void av7100_globals_exit(void)
{
	kfree(av7100_globals);
	av7100_globals = NULL;
}

/*************************************************************************
 *	name	=	av7100_poweron
 *	func	=	Power on AV7100
 *	input	=	None
 *	output	=	None
 *	return	=	0, -EFAULT
 *************************************************************************/
int av7100_poweron(void)
{	
	int retval = 0;
	
	/* Set the POWERDWN ball to high */
	gpio_set_value(GPIO_PORT8, 1);
	
	/* Need to wait before proceeding */
	mdelay(1);
	
	/* Change operating mode to standby mode */
	av7100_set_state(AV7100_OPMODE_STANDBY);
	
	/* Set HDMI 5 volt time register to off mode */
	retval = av7100_reg_hdmi_5_volt_time_w(
			AV7100_HDMI_OFF_TIME_0MS,
			AV7100_HDMI_ON_TIME_0MS);
	if (retval)
		return -EFAULT;	
		
	/* Set standby pending interrupt and standby 
	interrupt mask for hot plug detection */
	retval = av7100_reg_stby_pend_int_w(
			AV7100_STANDBY_PENDING_INTERRUPT_HPDI_HIGH,
			AV7100_STANDBY_PENDING_INTERRUPT_ONI_LOW,
			AV7100_STANDBY_PENDING_INTERRUPT_CCRST_LOW,
			AV7100_STANDBY_PENDING_INTERRUPT_CCI_HIGH);
	if (retval)
		return -EFAULT;
	
	retval = av7100_reg_stby_int_mask_w(
			AV7100_STANDBY_INTERRUPT_MASK_HPDM_LOW,
			AV7100_STANDBY_INTERRUPT_MASK_STBYGPIOCFG_ALT,
			AV7100_STANDBY_INTERRUPT_MASK_IPOL_HIGH,
			AV7100_STANDBY_INTERRUPT_MASK_CCM_LOW,
			AV7100_STANDBY_INTERRUPT_MASK_ONM_HIGH); 
	if (retval)
		return -EFAULT;
		
	return retval;
}

/*************************************************************************
 *	name	=	av7100_poweroff
 *	func	=	Power down AV7100
 *	input	=	None
 *	output	=	None
 *	return	=	0
 *************************************************************************/
int av7100_poweroff(void)
{
	/* Set the POWERDWN ball to low */
	gpio_set_value(GPIO_PORT8, 0);
	
	/* Change operating mode to shutdown mode */
	av7100_set_state(AV7100_OPMODE_SHUTDOWN);
	return 0;
}

/****************************************************************************
*	name	=	av7100_download_firmware
*	func	=	Download firmware to AV7100
*	input	=	None
*	output	=	None
*	return	=	0, -EFAULT, AV7100_FWDOWNLOAD_FAIL, -ENOMEM
****************************************************************************/
int av7100_download_firmware(void)
{
	int retval = 0;
	int cnt = 3; /* Retry number used in reading UC from general status reg */
	u8 *dsi_buff;
	u8 uc = 0; /* Used in reading general control register */
	
	dsi_buff = kzalloc(AV7100_FW_SIZE + 1, GFP_KERNEL);
	if (!dsi_buff)
		return -ENOMEM;
	
	/* 	Write 0x20 to general control register to set device in hold
	*   RA | WA | HLD | FDL | 0 | 0 | 0 | 0
	*   0  | 0  | 1   | 0   | 0 | 0 | 0 | 0 */
	retval = av7100_reg_gen_ctrl_w(
		AV7100_GENERAL_CONTROL_FDL_LOW,
		AV7100_GENERAL_CONTROL_HLD_HIGH,
		AV7100_GENERAL_CONTROL_WA_LOW,
		AV7100_GENERAL_CONTROL_RA_LOW);
	if (retval) {
		retval = -EFAULT;
		goto av7100_download_firmware_end;
	}
		
	/* Write 0x0000002084AC to registers at address 0x09  */
	if (register_write_internal(0x09, 0xAC)) {
		retval = -EFAULT;
		goto av7100_download_firmware_end;
	}
	if (register_write_internal(0x0A, 0x84)) {
		retval = -EFAULT;
		goto av7100_download_firmware_end;
	}
	if (register_write_internal(0x0B, 0x20)) {
		retval = -EFAULT;
		goto av7100_download_firmware_end;
	}
	if (register_write_internal(0x0C, 0x00)) {
		retval = -EFAULT;
		goto av7100_download_firmware_end;
	}
	if (register_write_internal(0x0D, 0x00)) {
		retval = -EFAULT;
		goto av7100_download_firmware_end;
	}
	if (register_write_internal(0x0E, 0x00)) {
		retval = -EFAULT;
		goto av7100_download_firmware_end;
	}
	/* Write 0x60 to general control register to perform write access
	*  to internal registers 
	*  RA | WA | HLD | FDL | 0 | 0 | 0 | 0
	*  0  | 1  | 1   | 0   | 0 | 0 | 0 | 0 */
	retval = av7100_reg_gen_ctrl_w(
		AV7100_GENERAL_CONTROL_FDL_LOW,
		AV7100_GENERAL_CONTROL_HLD_HIGH,
		AV7100_GENERAL_CONTROL_WA_HIGH,
		AV7100_GENERAL_CONTROL_RA_LOW);
	if (retval) {
		retval = -EFAULT;
		goto av7100_download_firmware_end;
	}
	
	/* Write 0x04210724847C to registers at address 0x09  */
	if (register_write_internal(0x09, 0x7C)) {
		retval = -EFAULT;
		goto av7100_download_firmware_end;
	}
	if (register_write_internal(0x0A, 0x84)) {
		retval = -EFAULT;
		goto av7100_download_firmware_end;
	}
	if (register_write_internal(0x0B, 0x24)) {
		retval = -EFAULT;
		goto av7100_download_firmware_end;
	}
	if (register_write_internal(0x0C, 0x07)) {
		retval = -EFAULT;
		goto av7100_download_firmware_end;
	}
	if (register_write_internal(0x0D, 0x21)) {
		retval = -EFAULT;
		goto av7100_download_firmware_end;
	}
	if (register_write_internal(0x0E, 0x04)) {
		retval = -EFAULT;
		goto av7100_download_firmware_end;
	}
	/* Write 0x60 to general control register to perform write access
	*  to internal registers */
	retval = av7100_reg_gen_ctrl_w(
		AV7100_GENERAL_CONTROL_FDL_LOW,
		AV7100_GENERAL_CONTROL_HLD_HIGH,
		AV7100_GENERAL_CONTROL_WA_HIGH,
		AV7100_GENERAL_CONTROL_RA_LOW);
	if (retval) {
		retval = -EFAULT;
		goto av7100_download_firmware_end;
	}
		
	/* Write 0x00000081880C to registers at address 0x09 
	*        ---------1----   is number of DSI lane  */
	if (register_write_internal(0x09, 0x0C)) {
		retval = -EFAULT;
		goto av7100_download_firmware_end;
	}
	if (register_write_internal(0x0A, 0x88)) {
		retval = -EFAULT;
		goto av7100_download_firmware_end;
	}
	if (register_write_internal(0x0B, 0x81)) {
		retval = -EFAULT;
		goto av7100_download_firmware_end;
	}
	if (register_write_internal(0x0C, 0x00)) {
		retval = -EFAULT;
		goto av7100_download_firmware_end;
	}
	if (register_write_internal(0x0D, 0x00)) {
		retval = -EFAULT;
		goto av7100_download_firmware_end;
	}
	if (register_write_internal(0x0E, 0x00)) {
		retval = -EFAULT;
		goto av7100_download_firmware_end;
	}
	/* Write 0x60 to general control register to perform write access
	*  to internal registers */
	retval = av7100_reg_gen_ctrl_w(
		AV7100_GENERAL_CONTROL_FDL_LOW,
		AV7100_GENERAL_CONTROL_HLD_HIGH,
		AV7100_GENERAL_CONTROL_WA_HIGH,
		AV7100_GENERAL_CONTROL_RA_LOW);
	if (retval) {
		retval = -EFAULT;
		goto av7100_download_firmware_end;
	}
		
	/* Write 0x000000018800 to registers at address 0x09  */
	if (register_write_internal(0x09, 0x00)) {
		retval = -EFAULT;
		goto av7100_download_firmware_end;
	}
	if (register_write_internal(0x0A, 0x88)) {
		retval = -EFAULT;
		goto av7100_download_firmware_end;
	}
	if (register_write_internal(0x0B, 0x01)) {
		retval = -EFAULT;
		goto av7100_download_firmware_end;
	}
	if (register_write_internal(0x0C, 0x00)) {
		retval = -EFAULT;
		goto av7100_download_firmware_end;
	}
	if (register_write_internal(0x0D, 0x00)) {
		retval = -EFAULT;
		goto av7100_download_firmware_end;
	}
	if (register_write_internal(0x0E, 0x00)) {
		retval = -EFAULT;
		goto av7100_download_firmware_end;
	}
	/* Write 0x60 to general control register to perform write access
	*  to internal registers */
	retval = av7100_reg_gen_ctrl_w(
		AV7100_GENERAL_CONTROL_FDL_LOW,
		AV7100_GENERAL_CONTROL_HLD_HIGH,
		AV7100_GENERAL_CONTROL_WA_HIGH,
		AV7100_GENERAL_CONTROL_RA_LOW);
	if (retval) {
		retval = -EFAULT;
		goto av7100_download_firmware_end;
	}
		
	/***************** DOWNLOAD FIRMWARE *************************************/
	/* 	Setting bit FDL (Firmware download) initializes code pointer to origin
	*	and opens a direct path to store in code RAM data from MIPI
	*   RA | WA | HLD | FDL | 0 | 0 | 0 | 0
	*   0  | 0  | 1   | 1   | 0 | 0 | 0 | 0 */
	retval = av7100_reg_gen_ctrl_w(
		AV7100_GENERAL_CONTROL_FDL_HIGH,
		AV7100_GENERAL_CONTROL_HLD_HIGH,
		AV7100_GENERAL_CONTROL_WA_LOW,
		AV7100_GENERAL_CONTROL_RA_LOW);
	if (retval) {
		retval = -EFAULT;
		goto av7100_download_firmware_end;
	}
	
	/* Data0 = 0xDB: download firmware command  */
	dsi_buff[0] = MIPI_DCS_FIRMWARE_DOWNLOAD;	
	
	/* Copy bytes from firmware buffer to dsi buffer */
	memcpy(&dsi_buff[1], av7100_fw_buff, AV7100_FW_SIZE);
	
	/* Send long command packet to DSI */
	retval = dsi_write_command(MIPI_DSI_DCS_LONG_WRITE, &dsi_buff[0],
					AV7100_FW_SIZE + 1);
	if (retval) {
		retval = -AV7100_FWDOWNLOAD_FAIL;
		goto av7100_download_firmware_end;
	}
	
	/**************** Process after download complete ********************/
	/* 	Release FDL and HLD bits
	*	and opens a direct path to store in code RAM data from MIPI
	*   RA | WA | HLD | FDL | 0 | 0 | 0 | 0
	*   0  | 0  | 0   | 0   | 0 | 0 | 0 | 0 */
	retval = av7100_reg_gen_ctrl_w(
		AV7100_GENERAL_CONTROL_FDL_LOW,
		AV7100_GENERAL_CONTROL_HLD_LOW,
		AV7100_GENERAL_CONTROL_WA_LOW,
		AV7100_GENERAL_CONTROL_RA_LOW);
	if (retval) {
		retval = -EFAULT;
		goto av7100_download_firmware_end;
	}
	
	retval = av7100_reg_gen_status_r(NULL , NULL, NULL, &uc, NULL, NULL);
	/* cnt is the retry number (3)
	* read UC bit in general status register and check if UC is not 0x1, read 
	* register again and decrease the retry number (maximum 3 times)
	* retval to check whether the reading is successful or not
	*/
	while (((retval != 0) || (uc != 1)) && (cnt-- > 0)) {
		/* delay 3ms */
		mdelay(3);

		retval = av7100_reg_gen_status_r(NULL , NULL, NULL, &uc, NULL, NULL);
	}
	
	if (uc != 0x1)
		retval = -EFAULT;
	
	/* Error handling */
av7100_download_firmware_end:
	kfree(dsi_buff);
	dsi_buff = NULL;
	return retval;
}

/****************************************************************************
*	name	=	av7100_intr_handler
*	func	=	Interrupt handler of AV7100 device
*	input	=	int irq, void *dev
*	output	=	None
*	return	=	IRQ_HANDLED
****************************************************************************/
static irqreturn_t av7100_intr_handler(int irq, void *dev)
{
	queue_work(hdmi_dev->my_workq, &hdmi_dev->hdmi_work);
	return IRQ_HANDLED;
}

/****************************************************************************
*	name	=	av7100_probe
*	func	=	Probe av7100 input device
*	input	=	struct device *dev
*	output	=	None
*	return	=	0, -ENOTSUPP
****************************************************************************/
static int __devinit av7100_probe(struct device *dev)
{
#ifdef HDMI_RUN
	int ret = 0;
	int current_state = HPD_STATE_UNPLUGGED;
	
	ret = av7100_config_init();
	if (ret)
		goto err_config_init;
	
	ret = av7100_globals_init();
	if (ret)
		goto err_globals_init;
	
	av7100_globals->av7100_state = AV7100_OPMODE_SHUTDOWN;
	
	/* MIPI HDMI */
	ret = gpio_request(GPIO_PORT219, NULL); 	/* portVIO_CK05 MCLK */
	if (ret)
		goto err_gpio_request_mclk;

	ret = gpio_request(GPIO_PORT8, NULL); 		/* port8 POWERDOWN */
	if (ret)
		goto err_gpio_request_pwdown;

	ret = gpio_direction_output(GPIO_PORT8, 0);
	if (ret)
		goto err_gpio_request_pwdown;
	
	ret = gpio_request(GPIO_PORT15, NULL);		/* port15 INT */
	if (ret)
		goto err_gpio_request_int;
	
	ret = gpio_direction_input(GPIO_PORT15);
	if (ret)
		goto err_gpio_request_int;
	
	ret = hdmi_init();
	if (ret)
		goto err_hdmi_init;

	/* Request interrupt handler function for AV7100 to interrupt handler */
	ret = request_irq(gpio_to_irq(GPIO_PORT15), av7100_intr_handler,
			  IRQF_TRIGGER_RISING, "av7100_int", av7100_config);
	if (ret)
		goto err_request_irq;
	
	ret = av7100_poweron();
	if (ret)
		goto err_poweron;

	/* Perform initial detection */
	current_state = hdmi_get_current_hpd();
	if (current_state == HPD_STATE_PLUGGED) {
		hdmi_hpd_handler(HPD_STATE_PLUGGED);
	}
	
	/* DSI ULPS/ Standby ON */
	return 0;
	
err_poweron:
	free_irq(gpio_to_irq(GPIO_PORT15), av7100_intr_handler);
	
err_request_irq:
	hdmi_exit();
	
err_hdmi_init:
	gpio_free(GPIO_PORT15);
	
err_gpio_request_int:
	gpio_free(GPIO_PORT8);
	
err_gpio_request_pwdown:
	gpio_free(GPIO_PORT219);
	
err_gpio_request_mclk:
	av7100_globals_exit();
	
err_globals_init:
	av7100_config_exit();
	
err_config_init:
	return -ENOTSUPP;
#else
	printk(KERN_DEBUG "av7100_Probe\n");
	return 0;
#endif
}

/****************************************************************************
*	name	=	av7100_remove
*	func	=	Remove av7100 input device
*	input	=	struct device *dev
*	output	=	None
*	return	=	0
****************************************************************************/
static int av7100_remove(struct device *dev)
{
	av7100_poweroff();
	/* DSI ULPS/ Standby ON */
	free_irq(gpio_to_irq(GPIO_PORT15), av7100_intr_handler);
	
	hdmi_exit();
	
	gpio_free(GPIO_PORT15);
	gpio_free(GPIO_PORT8);
	gpio_free(GPIO_PORT219);
	
	av7100_globals_exit();
	av7100_config_exit();
	
	return 0;
}

/****************************************************************************
*	name	=	av7100_suspend
*	func	=	Suspend AV7100 device of HDMI driver
*	input	=	struct device *dev
*	output	=	None
*	return	=	0, -EFAULT
****************************************************************************/
static int av7100_suspend(struct device *dev)
{
#ifdef HDMI_RUN
	int ret = 0;
	
	/* Set DSI to ULPS/Standby ON to save power */
	
	/* Change operating state to SHUTDOWN mode */
	ret = av7100_poweroff();
	if (ret)
		return -EFAULT;
	
	/* Disable all interrupts */
	disable_irq(gpio_to_irq(GPIO_PORT15));
	disable_irq(hdmi_dev->irq);
	
	/* Write HPD unplugged state to switch class */
	switch_set_state(&hdmi_dev->sdev, HPD_STATE_UNPLUGGED);
	
	return 0;
#else
	printk(KERN_DEBUG "av7100_Suspend\n");
	return 0;
#endif

}

/****************************************************************************
*	name	=	av7100_resume
*	func	=	Resume AV7100 device of HDMI driver
*	input	=	struct device *dev
*	output	=	None
*	return	=	0, -EFAULT
****************************************************************************/
static int av7100_resume(struct device *dev)
{
#ifdef HDMI_RUN
	int ret = 0;
	
	/* Enable all interrupts */
	enable_irq(gpio_to_irq(GPIO_PORT15));
	enable_irq(hdmi_dev->irq);
	
	/* Change operating state to STANDBY mode */
	ret = av7100_poweron();
	if (ret)
		return -EFAULT;
	
	/* Handle current HDP state */
	hdmi_hpd_handler(hdmi_get_current_hpd());
	
	return 0;
#else
	printk(KERN_DEBUG "av7100_Resume\n");
	return 0;
#endif
}

static struct dev_pm_ops av7100_pm_ops ={
	.suspend 	= av7100_suspend,
	.resume 	= av7100_resume,
};

static struct platform_driver av7100_platform_driver = {
	.driver =
	{
		.owner 		= THIS_MODULE,
		.name		= AV7100_PF_NAME,
		.probe		= av7100_probe,
		.remove		= av7100_remove,
		.pm			= &av7100_pm_ops,
	},
};

/****************************************************************************
*	name	=	av7100_init
*	func	=	Register AV7100 device and AV7100 driver 
*					for platform-level devices
*	input	=	None
*	output	=	None
*	return	=	0, -ENOTSUPP
****************************************************************************/
static int __init av7100_init(void)
{
	int ret = 0;
	/* register a driver for platform-level devices */
	ret = platform_driver_register(&av7100_platform_driver);
	if (ret)
		return -ENOTSUPP;
	
	/* add a platform-level device */
	pf_dev = platform_device_register_simple(AV7100_PF_NAME, -1, NULL, 0);
	if (pf_dev == NULL) {
		platform_driver_unregister(&av7100_platform_driver);
		return -ENOTSUPP;
	}
	return 0;	
}

/****************************************************************************
*	name	=	av7100_exit
*	func	=	Deregister AV7100 driver and AV7100 device
*				for platform-level devices
*	input	=	None
*	output	=	None
*	return	=	None
****************************************************************************/
static void __exit av7100_exit(void)
{
	platform_device_unregister(pf_dev);
	platform_driver_unregister(&av7100_platform_driver);
}

module_init(av7100_init);
module_exit(av7100_exit);

MODULE_DESCRIPTION("HDMI AV7100 Driver");
MODULE_AUTHOR("Renesas");
MODULE_LICENSE("GPL v2");
