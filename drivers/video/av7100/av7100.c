/*
 * Copyright (C) ST-Ericsson AB 2010
 * Copyright (C) 2012 Renesas Mobile Corporation. 
 *
 * AV7100 driver
 *
 * Author: Per Persson <per.xb.persson@stericsson.com>
 * for ST-Ericsson.
 *
 * License terms: GNU General Public License (GPL), version 2.
 */


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/timer.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <mach/r8a73734.h>
#include "av7100_regs.h"
#include <video/av7100.h>
#include <video/hdmi.h>
#include "av7100_fw.h"
#include <rtapi/screen_display.h>
#include <rtapi/screen_common.h>

#define AV7100_INT_EVENT 0x1
#define AV7100_TIMER_INT_EVENT 0x2

#define AV7100_TIMER_INTERRUPT_POLLING_TIME 250

#define GPIO_AV7100_RSTN	        GPIO_PORT8	
#define AV7100_MASTER_CLOCK_TIMING	0x3
#define AV7100_ON_TIME			1
#define AV7100_DENC_OFF_TIME		0
#define AV7100_HDMI_OFF_TIME		3
#define AV7100_COMMAND_OFFSET		0x10
#define AV7100_COMMAND_MAX_LENGTH	0x81

#define AV7100_TE_LINE_NB_14	14
#define AV7100_TE_LINE_NB_17	17
#define AV7100_TE_LINE_NB_18	18
#define AV7100_TE_LINE_NB_21	21
#define AV7100_TE_LINE_NB_22	22
#define AV7100_TE_LINE_NB_30	30
#define AV7100_TE_LINE_NB_38	38
#define AV7100_TE_LINE_NB_40	40
#define AV7100_UI_X4_DEFAULT	6

#define HDMI_REQUEST_FOR_REVOCATION_LIST_INPUT 2
#define HDMI_CEC_MESSAGE_WRITE_BUFFER_SIZE 16
#define HDMI_HDCP_SEND_KEY_SIZE 7
#define HDMI_INFOFRAME_DATA_SIZE 28
#define HDMI_FUSE_AES_KEY_SIZE 16

#define HPDS_INVALID 0xF
#define CPDS_INVALID 0xF
#define CECRX_INVALID 0xF

#define REG_16_8_LSB(p)		((u8)(p & 0xFF))
#define REG_16_8_MSB(p)		((u8)((p & 0xFF00)>>8))
#define REG_32_8_MSB(p)		((u8)((p & 0xFF000000)>>24))
#define REG_32_8_MMSB(p)	((u8)((p & 0x00FF0000)>>16))
#define REG_32_8_MLSB(p)	((u8)((p & 0x0000FF00)>>8))
#define REG_32_8_LSB(p)		((u8)(p & 0x000000FF))
#define REG_10_8_MSB(p)		((u8)((p & 0x300)>>8))
#define REG_12_8_MSB(p)		((u8)((p & 0xf00)>>8))

DEFINE_MUTEX(av7100_hw_mutex);
#define LOCK_AV7100_HW mutex_lock(&av7100_hw_mutex)
#define UNLOCK_AV7100_HW mutex_unlock(&av7100_hw_mutex)

#define AV7100_DEBUG_EXTRA
/* Disable Timer polling for plugin which will enable interrupt */
#define AV7100_PLUGIN_DETECT_VIA_TIMER_INTERRUPTS
#define CEC_ADDR_OFFSET 3
#define AV7100_POWERON_WAITTIME1_MS 1
#define AV7100_POWERON_WAITTIME2_MS 2 


#define AV7100_DCS_FIRMWARE_DOWNLOAD_UCCMD 0Xdb
#define AV7100_DCS_WRITE_UCCMD		   0xdc
#define AV7100_DCS_EXEC_UCCMD		   0xdd
#define AV7100_DCS_WRITE_REGISTER_UCCMD	   0xe0
#define AV7100_DCS_READ_REGISTER	   0x06

struct av7100_config_t {
	struct i2c_client *client;
	struct i2c_device_id *id;
	struct av7100_video_input_format_cmd hdmi_video_input_cmd;
	struct av7100_audio_input_format_cmd hdmi_audio_input_cmd;
	struct av7100_video_output_format_cmd hdmi_video_output_cmd;
	struct av7100_video_scaling_format_cmd hdmi_video_scaling_cmd;
	struct av7100_color_space_conversion_format_cmd
					hdmi_color_space_conversion_cmd;
	struct av7100_cec_message_write_format_cmd
		hdmi_cec_message_write_cmd;
	struct av7100_cec_message_read_back_format_cmd
		hdmi_cec_message_read_back_cmd;
	struct av7100_denc_format_cmd hdmi_denc_cmd;
	struct av7100_hdmi_cmd hdmi_cmd;
	struct av7100_hdcp_send_key_format_cmd hdmi_hdcp_send_key_cmd;
	struct av7100_hdcp_management_format_cmd
		hdmi_hdcp_management_format_cmd;
	struct av7100_infoframes_format_cmd	hdmi_infoframes_cmd;
	struct av7100_edid_section_readback_format_cmd
		hdmi_edid_section_readback_cmd;
	struct av7100_pattern_generator_format_cmd hdmi_pattern_generator_cmd;
	struct av7100_fuse_aes_key_format_cmd hdmi_fuse_aes_key_cmd;
};

struct av7100_globals_t {
	int denc_off_time;/* 5 volt time */
	int hdmi_off_time;/* 5 volt time */
	int on_time;/* 5 volt time */
	u8 hpdm;/*stby_int_mask*/
	u8 cpdm;/*stby_int_mask*/
#ifdef AV7100_PLUGIN_DETECT_VIA_TIMER_INTERRUPTS
	u8 hpds_old;
	u8 cpds_old;
	u8 cecrx_old;
	u8 hdcps_old;
#endif
	enum interface_type if_type;

	void (*hdmi_ev_cb)(enum av7100_hdmi_event);
};

enum av7100_command_size {
	AV7100_COMMAND_VIDEO_INPUT_FORMAT_SIZE  = 0x17,
	AV7100_COMMAND_AUDIO_INPUT_FORMAT_SIZE  = 0x8,
	AV7100_COMMAND_VIDEO_OUTPUT_FORMAT_SIZE = 0x1E,
	AV7100_COMMAND_VIDEO_SCALING_FORMAT_SIZE = 0x11,
	AV7100_COMMAND_COLORSPACECONVERSION_SIZE = 0x1D,
	AV7100_COMMAND_CEC_MESSAGE_WRITE_SIZE = 0x12,
	AV7100_COMMAND_CEC_MESSAGE_READ_BACK_SIZE = 0x1,
	AV7100_COMMAND_DENC_SIZE = 0x6,
	AV7100_COMMAND_HDMI_SIZE = 0x4,
	AV7100_COMMAND_HDCP_SENDKEY_SIZE = 0xA,
	AV7100_COMMAND_HDCP_MANAGEMENT_SIZE = 0x4,
	AV7100_COMMAND_INFOFRAMES_SIZE = 0x21,
	AV7100_COMMAND_EDID_SECTION_READBACK_SIZE  = 0x3,
	AV7100_COMMAND_PATTERNGENERATOR_SIZE  = 0x4,
	AV7100_COMMAND_FUSE_AES_KEY_SIZE = 0x12,
};

static void clr_plug_status(enum av7100_plugin_status status);
static void set_plug_status(enum av7100_plugin_status status);
static void cec_rx(void);
static void hdcp_changed(void);
static int av7100_open(struct inode *inode, struct file *filp);
static int av7100_release(struct inode *inode, struct file *filp);
static int av7100_ioctl(struct inode *inode, struct file *file,
	unsigned int cmd, unsigned long arg);
extern ssize_t down_load(struct device *dev, struct device_attribute *attr,
			 char *buf);
static int __devinit av7100_probe(struct i2c_client *i2cClient,
	const struct i2c_device_id *id);
static int __devexit av7100_remove(struct i2c_client *i2cClient);
int register_read_internal(u8 offset, u8 *value);
extern int changeHDCPstatus(u8 hdcp_state);

/************************************************
*   	Global variable definition section	 	*
************************************************/ 

static struct av7100_config_t *av7100_config;
static struct av7100_status	g_av7100_status = {0};
#ifdef AV7100_PLUGIN_DETECT_VIA_TIMER_INTERRUPTS
static struct timer_list av7100_timer;
#endif
static wait_queue_head_t av7100_event;
static int av7100_flag = 0x0;
static struct av7100_globals_t *av7100_globals;
static u8 chip_version;
static char av7100_receivetab[AV7100_FW_SIZE];
struct device *av7100dev;

static const struct file_operations av7100_fops = {
	.owner =    THIS_MODULE,
	.open =     av7100_open,
	.release =  av7100_release,
};

static struct miscdevice av7100_miscdev = {
	MISC_DYNAMIC_MINOR,
	"av7100",
	&av7100_fops
};

static int av7100_suspend(struct i2c_client *client, pm_message_t message);
static int av7100_resume(struct i2c_client *client);

struct av7100_cea av7100_all_cea[29] = {
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
	"+",	0,	0,	0,	0,	0},/*Settings to be define*/
{ "5  CEA 6-7      480i (NTSC)          ",
	6,	525,	240,	44,	5,
	0,	"-",	858,	720,	12,	64,	10,	13513513,
	"-",	0,	0,	0,	0,	0},/*Settings to be define*/
{ "6  CEA 14-15    480p @ 60 Hz         ",
	14,	525,	480,	44,	5,
	0,	"-",	858,	720,	12,	64,	10,	27027000,
	"-",	0,	0,	0,	0,	0},/*Settings to be define*/
{ "7  CEA 16       1920x1080p @ 60 Hz   ",
	16,	1125,	1080,	36,	5,
	0,	"+",	1980,	1280,	440,	40,	10,	133650000,
	"+",	0,	0,	0,	0,	0},/*Settings to be define*/
{ "8  CEA 17-18    720x576p @ 50 Hz     ",
	17,	625,	576,	44,	5,
	0,	"-",	864,	720,	12,	64,	10,	27000000,
	"-",	0,	0,	0,	0,	0},/*Settings to be define*/
{ "9  CEA 19       1280x720p @ 50 Hz    ",
	19,	750,	720,	25,	5,
	0,	"+",	1980,	1280,	440,	40,	10,	74250000,
	"+",	0,	0,	0,	0,	0},/*Settings to be define*/
{ "10 CEA 20       1920 x 1080i @ 50 Hz ",
	20,	1125,	540,	20,	5,
	0,	"+",	2640,	1920,	528,	44,	10,	74250000,
	"+",	0,	0,	0,	0,	0},/*Settings to be define*/
{ "11 CEA 21-22    576i (PAL)           ",
	21,	625,	288,	44,	5,
	0,	"-",	1728,	1440,	12,	64,	10,	27000000,
	"-",	0,	0,	0,	0,	0},/*Settings to be define*/
{ "12 CEA 29/30    576p                 ",
	29,	625,	576,	44,	5,
	0,	"-",	864,	720,	12,	64,	10,	27000000,
	"-",	0,	0,	0,	0,	0},/*Settings to be define*/
{ "13 CEA 31       1080p 50Hz           ",
	31,	1125,	1080,	44,	5,
	0,	"-",	2640,	1920,	12,	64,	10,	148500000,
	"-",	0,	0,	0,	0,	0},/*Settings to be define*/
{ "14 CEA 32       1920x1080p @ 24 Hz   ",
	32,	1125,	1080,	36,	5,
	4,	"+",	2750,	1920,	660,	44,	153,	74250000,
	"+",	2844,	0x530,	6,	32,	1},/*RGB565*/
{ "15 CEA 33       1920x1080p @ 25 Hz   ",
	33,	1125,	1080,	36,	5,
	4,	"+",	2640,	1920,	528,	44,	10,	74250000,
	"+",	0,	0,	0,	0,	0},/*Settings to be define*/
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
	"+",	0,	0,	0,	0,	0},/*Settings to be define*/
{ "21 VESA 14      848x480  @ 60 Hz     ",
	114,	500,	480,	20,	5,
	0,	"+",	1056,	848,	24,	80,	10,	31680000,
	"-",	0,	0,	0,	0,	0},/*Settings to be define*/
{ "22 VESA 16      1024x768 @ 60 Hz     ",
	116,	806,	768,	38,	6,
	0,	"-",	1344,	1024,	24,	135,	10,	65000000,
	"-",	0,	0,	0,	0,	0},/*Settings to be define*/
{ "23 VESA 22      1280x768 @ 60 Hz     ",
	122,	802,	768,	34,	4,
	0,	"+",	1688,	1280,	48,	160,	10,	81250000,
	"-",	0,	0,	0,	0,	0},/*Settings to be define*/
{ "24 VESA 23      1280x768 @ 60 Hz     ",
	123,	798,	768,	30,	7,
	0,	"+",	1664,	1280,	64,	128,	10,	79500000,
	"-",	0,	0,	0,	0,	0},/*Settings to be define*/
{ "25 VESA 27      1280x800 @ 60 Hz     ",
	127,	823,	800,	23,	6,
	0,	"+",	1440,	1280,	48,	32,	10,	71000000,
	"+",	0,	0,	0,	0,	0},/*Settings to be define*/
{ "26 VESA 28      1280x800 @ 60 Hz     ",
	128,	831,	800,	31,	6,
	0,	"+",	1680,	1280,	72,	128,	10,	83500000,
	"-",	0,	0,	0,	0,	0},/*Settings to be define*/
{ "27 VESA 39      1360x768 @ 60 Hz     ",
	139,	790,	768,	22,	5,
	0,	"-",	1520,	1360,	48,	32,	10,	72000000,
	"+",	0,	0,	0,	0,	0},/*Settings to be define*/
{ "28 VESA 81      1360x768 @ 60 Hz     ",
	181,	798,	768,	30,	5,
	0,	"+",	1776,	1360,	72,	136,	10,	84750000,
	"-",	0,	0,	0,	0,	0} /*Settings to be define*/
};

const struct av7100_color_space_conversion_format_cmd col_cvt_identity = {
	.c0      = 0x0100,
	.c1      = 0x0000,
	.c2      = 0x0000,
	.c3      = 0x0000,
	.c4      = 0x0100,
	.c5      = 0x0000,
	.c6      = 0x0000,
	.c7      = 0x0000,
	.c8      = 0x0100,
	.aoffset = 0x0000,
	.boffset = 0x0000,
	.coffset = 0x0000,
	.lmax    = 0xff,
	.lmin    = 0x00,
	.cmax    = 0xff,
	.cmin    = 0x00,
};

const struct av7100_color_space_conversion_format_cmd
						col_cvt_identity_clamp_yuv = {
	.c0      = 0x0100,
	.c1      = 0x0000,
	.c2      = 0x0000,
	.c3      = 0x0000,
	.c4      = 0x0100,
	.c5      = 0x0000,
	.c6      = 0x0000,
	.c7      = 0x0000,
	.c8      = 0x0100,
	.aoffset = 0x0000,
	.boffset = 0x0000,
	.coffset = 0x0000,
	.lmax    = 0xeb,
	.lmin    = 0x10,
	.cmax    = 0xf0,
	.cmin    = 0x10,
};

const struct av7100_color_space_conversion_format_cmd col_cvt_yuv422_to_rgb = {
	.c0      = 0x00ba,
	.c1      = 0x007d,
	.c2      = 0x0000,
	.c3      = 0xffa1,
	.c4      = 0x007d,
	.c5      = 0xffd3,
	.c6      = 0x0000,
	.c7      = 0x007d,
	.c8      = 0x00eb,
	.aoffset = 0xff9b,
	.boffset = 0x003e,
	.coffset = 0xff82,
	.lmax    = 0xff,
	.lmin    = 0x00,
	.cmax    = 0xff,
	.cmin    = 0x00,
};

const struct av7100_color_space_conversion_format_cmd col_cvt_yuv422_to_denc = {
	.c0      = 0x0000,
	.c1      = 0x0000,
	.c2      = 0x0100,
	.c3      = 0x0000,
	.c4      = 0x0100,
	.c5      = 0x0000,
	.c6      = 0x0100,
	.c7      = 0x0000,
	.c8      = 0x0000,
	.aoffset = 0x0000,
	.boffset = 0x0000,
	.coffset = 0x0000,
	.lmax    = 0xeb,
	.lmin    = 0x10,
	.cmax    = 0xf0,
	.cmin    = 0x10,
};

const struct av7100_color_space_conversion_format_cmd col_cvt_rgb_to_denc = {
	.c0      = 0xffda,
	.c1      = 0xffb6,
	.c2      = 0x0070,
	.c3      = 0x0042,
	.c4      = 0x0081,
	.c5      = 0x0019,
	.c6      = 0x0070,
	.c7      = 0xffa2,
	.c8      = 0xffee,
	.aoffset = 0x007f,
	.boffset = 0x0010,
	.coffset = 0x007f,
	.lmax    = 0xff,
	.lmin    = 0x00,
	.cmax    = 0xff,
	.cmin    = 0x00,
};

static const struct i2c_device_id av7100_id[] = {
	{ "av7100", 0 },
	{ }
};

static struct i2c_driver av7100_driver = {
	.probe	= av7100_probe,
	.remove = av7100_remove,
#ifdef CONFIG_PM
	.suspend = av7100_suspend,
	.resume = av7100_resume,
#endif
	.driver = {
		.name	= "av7100",
	},
	.id_table	= av7100_id,
};

/****************************************************************************
*	name	=	av7100_timer_int
*	func	=	Initialize the timer for av7100.
*	input	=	unsigned long value
*	output	=	None
*	return	=	None
****************************************************************************/

#ifdef AV7100_PLUGIN_DETECT_VIA_TIMER_INTERRUPTS
static void av7100_timer_int(unsigned long value)
{
	av7100_flag |= AV7100_TIMER_INT_EVENT;
	wake_up_interruptible(&av7100_event);
	if (g_av7100_status.av7100_state >= AV7100_OPMODE_STANDBY) {
		av7100_timer.expires = jiffies +
			AV7100_TIMER_INTERRUPT_POLLING_TIME;
		add_timer(&av7100_timer);
	}
}
#endif

/****************************************************************************
*	name	=	av7100_thread
*	func	=	Thread function that waits until woken up by irq 
*			handler and perform register read/write whenever 
*			an interrupt arrives.
*	input	=	void *p
*	output	=	None
*	return	=	value of ret is returned.
****************************************************************************/
static int av7100_thread(void *p)
{
	u8 hpds = 0;
	u8 cpds = 0;
	u8 hdcps = 0;
	int ret = 0;
#ifdef AV7100_PLUGIN_DETECT_VIA_TIMER_INTERRUPTS
	u8 cecrx;
	u8 *hpds_old;
	u8 *cpds_old;
	u8 *cecrx_old;
	u8 *hdcps_old;
#else
	u8 sid = 0;
	u8 oni = 0;
	u8 hpdi = 0;
        u8 stby = 0;
        u8 mclkrng = 0;
#endif

	while (1) {
		wait_event_interruptible(av7100_event, (av7100_flag != 0));
#ifdef AV7100_PLUGIN_DETECT_VIA_TIMER_INTERRUPTS
		if ((av7100_flag & AV7100_TIMER_INT_EVENT) &&
			(g_av7100_status.av7100_state >=
				AV7100_OPMODE_STANDBY)) {
			hpds_old = &(av7100_globals->hpds_old);
			cpds_old = &(av7100_globals->cpds_old);
			cecrx_old = &(av7100_globals->cecrx_old);
			hdcps_old = &(av7100_globals->hdcps_old);
			/* STANDBY reg */
			if (av7100_reg_stby_r(
			        NULL,
				&hpds,
				NULL) != 0)
				dev_err(av7100dev, "av7100_reg_"
							"stby_r fail\n");

			/* TVout plugin change */
			if ((cpds == 1) && (cpds != *cpds_old)) {
				*cpds_old = 1;

				set_plug_status(AV7100_CVBS_PLUGIN);
			} else if ((cpds == 0) && (cpds != *cpds_old)) {
				*cpds_old = 0;

				clr_plug_status(AV7100_CVBS_PLUGIN);
			}

			/* HDMI plugin change */
			if ((hpds == 1) && (hpds != *hpds_old)) {
				*hpds_old = 1;
				set_plug_status(AV7100_HDMI_PLUGIN);
			} else if ((hpds == 0) && (hpds != *hpds_old)) {
				*hpds_old = 0;
				clr_plug_status(AV7100_HDMI_PLUGIN);
			}

			/* GENERAL_STATUS reg */
			if (av7100_reg_gen_status_r(
				&cecrx,
				NULL,
				NULL,
				NULL,
				&hdcps) != 0)
				dev_dbg(av7100dev, "av7100_reg_"
					"gen_status_r fail\n");
			else {
				if ((cecrx == 1) && (cecrx != *cecrx_old))
					/* Report CEC event */
					cec_rx();
				*cecrx_old = cecrx;

				if (hdcps != *hdcps_old)
					// Report HDCP status change even
    				changeHDCPstatus(hdcps);
				*hdcps_old = hdcps;
			}
		}
#else
		/* STANDBY_PENDING_INTERRUPT */
		ret = av7100_reg_stby_pend_int_r(
			&hpdi,
			&oni,
			&sid);

		if (ret)
			dev_dbg(av7100dev, "av7100_register_standby_"
				"pending_interrupt_read failed\n");

		if (hpdi | oni) {
			/* STANDBY */
			ret = av7100_reg_stby_r(
					&stby,
					&hpds,
					&mclkrng);
			if (ret)
				dev_dbg(av7100dev, "av7100_register_standby_"
					"read fail\n");
		}

        if (hpdi) {
			/* HDMI plugin change */
			if (hpds)
				set_plug_status(AV7100_HDMI_PLUGIN);
			else
				clr_plug_status(AV7100_HDMI_PLUGIN);
		}

		if (hpdi |  oni) {
			/* Clear pending interrupts */
			ret = av7100_reg_stby_pend_int_w(
					hpdi,
					oni);
			if (ret)
				dev_dbg(av7100dev, "av7100_register_standby_"
					"read fail\n");
		}
#endif
		av7100_flag = 0;
	}

	return ret;
}

/****************************************************************************
*	name	=	av7100_intr_handler
*	func	=	Handle interrupt for av7100
*	input	=	int irq,void *p
*	output	=	None
*	return	=	IRQ_HANDLED
****************************************************************************/
static irqreturn_t av7100_intr_handler(int irq, void *p)
{
	av7100_flag |= AV7100_INT_EVENT;
	wake_up_interruptible(&av7100_event);
	return IRQ_HANDLED;
}

/****************************************************************************
*	name	=	av7100_get_te_line_nb
*	func	=	Specify Tearing effect Lines per frame currently
*	input	=	enum av7100_output_CEA_VESA output_video_format
*	output	=	None
*	return	=	retval
****************************************************************************/
u16 av7100_get_te_line_nb(
	enum av7100_output_CEA_VESA output_video_format)
{
	u16 retval;

	switch (output_video_format) {
	case AV7100_CEA1_640X480P_59_94HZ:
	case AV7100_CEA2_3_720X480P_59_94HZ:
		retval = AV7100_TE_LINE_NB_30;
		break;

	case AV7100_CEA5_1920X1080I_60HZ:
	case AV7100_CEA6_7_NTSC_60HZ:
	case AV7100_CEA20_1920X1080I_50HZ:
		retval = AV7100_TE_LINE_NB_18;
		break;

	case AV7100_CEA4_1280X720P_60HZ:
		//retval = AV7100_TE_LINE_NB_21;
                retval = 0;
		break;

	case AV7100_CEA17_18_720X576P_50HZ:
		retval = AV7100_TE_LINE_NB_40;
		break;

	case AV7100_CEA19_1280X720P_50HZ:
		retval = AV7100_TE_LINE_NB_22;
		break;

	case AV7100_CEA21_22_576I_PAL_50HZ:
		/* Different values below come from LLD,
		 * TODO: check if this is really needed
		 * if not merge with AV7100_CEA6_7_NTSC_60HZ case
		 */
#ifdef CONFIG_AV7100_SDTV
		retval = AV7100_TE_LINE_NB_18;
#else
		retval = AV7100_TE_LINE_NB_17;
#endif
		break;

	case AV7100_CEA32_1920X1080P_24HZ:
	case AV7100_CEA33_1920X1080P_25HZ:
	case AV7100_CEA34_1920X1080P_30HZ:
		retval = AV7100_TE_LINE_NB_38;
		break;

	case AV7100_CEA60_1280X720P_24HZ:
	case AV7100_CEA62_1280X720P_30HZ:
		retval = AV7100_TE_LINE_NB_21;
		break;

	case AV7100_CEA14_15_480p_60HZ:
	case AV7100_VESA14_848X480P_60HZ:
	case AV7100_CEA61_1280X720P_25HZ:
	case AV7100_CEA16_1920X1080P_60HZ:
	case AV7100_CEA31_1920x1080P_50Hz:
	case AV7100_CEA29_30_576P_50HZ:
	case AV7100_VESA9_800X600P_60_32HZ:
	case AV7100_VESA16_1024X768P_60HZ:
	case AV7100_VESA22_1280X768P_59_99HZ:
	case AV7100_VESA23_1280X768P_59_87HZ:
	case AV7100_VESA27_1280X800P_59_91HZ:
	case AV7100_VESA28_1280X800P_59_81HZ:
	case AV7100_VESA39_1360X768P_60_02HZ:
	case AV7100_VESA81_1366X768P_59_79HZ:
	default:
		/* TODO */
		retval = AV7100_TE_LINE_NB_14;
		break;
	}

	return retval;
}

/****************************************************************************
*	name	=	av7100_get_ui_x4
*	func	=	Set value for ui_x4
*	input	=	enum av7100_output_CEA_VESA output_video_format
*	output	=	None
*	return	=	AV7100_UI_X4_DEFAULT
****************************************************************************/
u16 av7100_get_ui_x4(
	enum av7100_output_CEA_VESA output_video_format)
{
	return AV7100_UI_X4_DEFAULT;
}

/****************************************************************************
*	name	=	av7100_config_video_output_dep
*	func	=	Sets the default Input parameters based on 
* 			output format av7100 should have same input and 
* 			output formats so based on the format supported 
* 			by the HDMI sink the output and input parameters 
*			are configured called from av7100_conf_prep 
*			for output format
*	input	=	enum av7100_output_CEA_VESA	output_format
*	output	=	None
*	return	=	retval
****************************************************************************/
static int av7100_config_video_output_dep(enum av7100_output_CEA_VESA
	output_format)
{
	int retval = 0;
	union av7100_configuration config;
	/* video input */
	config.video_input_format.dsi_input_mode =
		AV7100_HDMI_DSI_COMMAND_MODE;
	config.video_input_format.input_pixel_format = AV7100_INPUT_PIX_RGB888;
	config.video_input_format.total_horizontal_pixel =
		av7100_all_cea[output_format].htotale;
	config.video_input_format.total_horizontal_active_pixel =
		av7100_all_cea[output_format].hactive;
	config.video_input_format.total_vertical_lines =
		av7100_all_cea[output_format].vtotale;
	config.video_input_format.total_vertical_active_lines =
		av7100_all_cea[output_format].vactive;

	switch (output_format) {
	case AV7100_CEA5_1920X1080I_60HZ:
	case AV7100_CEA20_1920X1080I_50HZ:
	case AV7100_CEA21_22_576I_PAL_50HZ:
	case AV7100_CEA6_7_NTSC_60HZ:
		config.video_input_format.video_mode =
			AV7100_VIDEO_INTERLACE;
		break;

	default:
		config.video_input_format.video_mode =
			AV7100_VIDEO_PROGRESSIVE;
		break;
	}

	config.video_input_format.nb_data_lane =
		AV7100_DATA_LANES_USED_4;
	config.video_input_format.nb_virtual_ch_command_mode = 0;
	config.video_input_format.nb_virtual_ch_video_mode = 0;
	config.video_input_format.ui_x4 = 7;
        /* Disable TE interrupts */
	config.video_input_format.TE_line_nb = 0; 
#ifdef CONFIG_AV7100_HWTRIG_I2SDAT3
	config.video_input_format.TE_config = AV7100_TE_GPIO_IT;
#else
	config.video_input_format.TE_config = AV7100_TE_OFF;
#endif
	config.video_input_format.master_clock_freq = 0;

	retval = av7100_conf_prep(
		AV7100_COMMAND_VIDEO_INPUT_FORMAT, &config);
	return retval;
}

/****************************************************************************
*	name	=	av7100_config_init
*	func	=	Initialises the av7100 configurations and sets 
* 			up default video/audio input, output and HDMI mode
*	input	=	void
*	output	=	None
*	return	=	retval
****************************************************************************/
static int av7100_config_init(void)
{
	int retval = 0;
	union av7100_configuration config;

	dev_dbg(av7100dev, "%s\n", __func__);

	av7100_config = kzalloc(sizeof(struct av7100_config_t), GFP_KERNEL);
	if (!av7100_config) {
		dev_err(av7100dev, "%s: Failed to allocate config\n", __func__);
		return AV7100_FAIL;
	}

	memset(&config, 0, sizeof(union av7100_configuration));
	memset(av7100_config, 0, sizeof(union av7100_configuration));

	/* Color conversion */
	config.color_space_conversion_format = col_cvt_identity;
	retval = av7100_conf_prep(
		AV7100_COMMAND_COLORSPACECONVERSION, &config);
	if (retval)
		return AV7100_FAIL;

	/* Video output */
	config.video_output_format.video_output_cea_vesa =
		AV7100_CEA4_1280X720P_60HZ;

	retval = av7100_conf_prep(
		AV7100_COMMAND_VIDEO_OUTPUT_FORMAT, &config);
	if (retval)
		return AV7100_FAIL;

	/* Video input */
	av7100_config_video_output_dep(
		config.video_output_format.video_output_cea_vesa);

	/* Pattern generator */
	config.pattern_generator_format.pattern_audio_mode =
		AV7100_PATTERN_AUDIO_OFF;
	config.pattern_generator_format.pattern_type =
		AV7100_PATTERN_GENERATOR;
	config.pattern_generator_format.pattern_video_format =
		AV7100_PATTERN_720P;
	retval = av7100_conf_prep(AV7100_COMMAND_PATTERNGENERATOR,
		&config);
	if (retval)
		return AV7100_FAIL;

	/* Audio input */
	config.audio_input_format.audio_input_if_format	=
		AV7100_AUDIO_I2SDELAYED_MODE;
	config.audio_input_format.i2s_input_nb = 1;
	config.audio_input_format.sample_audio_freq = AV7100_AUDIO_FREQ_44_1KHZ;
	config.audio_input_format.audio_word_lg = AV7100_AUDIO_16BITS;
	config.audio_input_format.audio_format = AV7100_AUDIO_LPCM_MODE;
	config.audio_input_format.audio_if_mode = AV7100_AUDIO_SLAVE;
	config.audio_input_format.audio_mute = AV7100_AUDIO_MUTE_DISABLE;
	retval = av7100_conf_prep(
		AV7100_COMMAND_AUDIO_INPUT_FORMAT, &config);
	if (retval)
		return AV7100_FAIL;

	/* HDMI mode */
	config.hdmi_format.hdmi_mode	= AV7100_HDMI_ON;
	config.hdmi_format.hdmi_format	= AV7100_HDMI;
	config.hdmi_format.dvi_format	= AV7100_DVI_CTRL_CTL0;
	retval = av7100_conf_prep(AV7100_COMMAND_HDMI, &config);
	if (retval)
		return AV7100_FAIL;

	/* EDID section readback */
	config.edid_section_readback_format.address = 0xA0;
	config.edid_section_readback_format.block_number = 0;
	retval = av7100_conf_prep(
		AV7100_COMMAND_EDID_SECTION_READBACK, &config);
	if (retval)
		return AV7100_FAIL;

	return retval;
}

/****************************************************************************
*	name	=	av7100_config_exit
*	func	=	Frees the av7100 config structure
*	input	=	void
*	output	=	None
*	return	=	void
****************************************************************************/
static void av7100_config_exit(void)
{
	dev_dbg(av7100dev, "%s\n", __func__);

	kfree(av7100_config);
	av7100_config = NULL;
}

/****************************************************************************
*	name	=	av7100_globals_init
*	func	=	Initialises the av7100 specific register 
*			and configuration settings
*	input	=	void
*	output	=	None
*	return	=	0,AV7100_FAIL
****************************************************************************/
static int av7100_globals_init(void)
{
	dev_dbg(av7100dev, "%s\n", __func__);

	av7100_globals = kzalloc(sizeof(struct av7100_globals_t), GFP_KERNEL);
	if (!av7100_globals) {
		dev_err(av7100dev, "%s: Alloc failure\n", __func__);
		return AV7100_FAIL;
	}
    /* TODO Remove DENC related code */
	av7100_globals->denc_off_time = AV7100_DENC_OFF_TIME;
	av7100_globals->hdmi_off_time = AV7100_HDMI_OFF_TIME;
	av7100_globals->on_time = AV7100_ON_TIME;
	av7100_globals->hpdm = AV7100_STANDBY_INTERRUPT_MASK_HPDM_LOW;
	av7100_globals->cpdm = AV7100_STANDBY_INTERRUPT_MASK_CPDM_LOW;
#ifdef AV7100_PLUGIN_DETECT_VIA_TIMER_INTERRUPTS
	av7100_globals->hpds_old = 0xf;
	av7100_globals->cpds_old = 0xf;
	av7100_globals->cecrx_old = 0xf;
	av7100_globals->hdcps_old = 0xf;
#endif
	av7100_globals->if_type = I2C_INTERFACE;
	return 0;
}

/****************************************************************************
*	name	=	av7100_globals_exit
*	func	=	Frees av7100_globals structure
*	input	=	void
*	output	=	None
*	return	=	void
****************************************************************************/
static void av7100_globals_exit(void)
{
	dev_dbg(av7100dev, "%s\n", __func__);

	kfree(av7100_globals);
	av7100_globals = NULL;
}

/****************************************************************************
*	name	=	clr_plug_status
*	func	=	Executes call back function when HDMI is PLUGGED IN
*	input	=	enum av7100_plugin_status status
*	output	=	None
*	return	=	void
****************************************************************************/ 
static void clr_plug_status(enum av7100_plugin_status status)
{
	g_av7100_status.av7100_plugin_status &= ~status;

	switch (status) {
	case AV7100_HDMI_PLUGIN:
		if (av7100_globals->hdmi_ev_cb)
			av7100_globals->hdmi_ev_cb(
					AV7100_HDMI_EVENT_HDMI_PLUGOUT);
		break;

	case AV7100_CVBS_PLUGIN:
		/* TODO */
		break;

	default:
		break;
	}
}

/****************************************************************************
*	name	=	set_plug_status
*	func	=	Executes call back function when HDMI is PLUGGED IN
*	input	=	enum av7100_plugin_status status
*	output	=	None
*	return	=	void
****************************************************************************/
static void set_plug_status(enum av7100_plugin_status status)
{
	g_av7100_status.av7100_plugin_status |= status;

	switch (status) {
	case AV7100_HDMI_PLUGIN:
		if (av7100_globals->hdmi_ev_cb)
			av7100_globals->hdmi_ev_cb(
					AV7100_HDMI_EVENT_HDMI_PLUGIN);
		break;

	case AV7100_CVBS_PLUGIN:
		/* TODO */
		break;

	default:
		break;
	}
}

/****************************************************************************
*	name	=	cec_rx
*	func	=	Executes callback function with AV7100_HDMI_EVENT_CEC 
* 			as parameter when it is in CEC mode
*	input	=	void
*	output	=	None
*	return	=	void
****************************************************************************/
static void cec_rx(void)
{
	if (av7100_globals->hdmi_ev_cb)
		av7100_globals->hdmi_ev_cb(AV7100_HDMI_EVENT_CEC);
}

/****************************************************************************
*	name	=	hdcp_changed
*	func	=	Executes callback function with AV7100_HDMI_EVENT_HDCP
* 			as parameter when HDCP event occurred
*	input	=	void
*	output	=	None
*	return	=	void
****************************************************************************/
static void hdcp_changed(void)
{
	if (av7100_globals->hdmi_ev_cb)
		av7100_globals->hdmi_ev_cb(AV7100_HDMI_EVENT_HDCP);
}

/****************************************************************************
*	name	=	av7100_set_state
*	func	=	Set state of av7100
*	input	=	enum av7100_operating_mode state
*	output	=	None
*	return	=	void
****************************************************************************/
static void av7100_set_state(enum av7100_operating_mode state)
{
	g_av7100_status.av7100_state = state;
	if (state <= AV7100_OPMODE_STANDBY) {
		clr_plug_status(AV7100_HDMI_PLUGIN);
		clr_plug_status(AV7100_CVBS_PLUGIN);
		g_av7100_status.hdmi_on = false;
	}
}

/****************************************************************************
*	name	=	write_single_byte
*	func	=	Write a single byte to av7100 through i2c interface
*	input	=	struct i2c_client *client, u8 reg,u8 data
*	output	=	None
*	return	=	ret
****************************************************************************/
static int write_single_byte(struct i2c_client *client, u8 reg,
	u8 data)
{
	int ret;

	ret = i2c_smbus_write_byte_data(client, reg, data);
	if (ret < 0){
		dev_err(av7100dev, "i2c smbus write byte failed in write_single_byte function\n");
		dev_dbg(av7100dev, "i2c smbus write byte failed\n");
	}

	return ret;
}

/****************************************************************************
*	name	=	read_single_byte
*	func	=	read single byte from av7100 through i2c interface
*	input	=	struct i2c_client *client, u8 reg, u8 *val
*	output	=	None
*	return	=	0,AV7100_FAIL
****************************************************************************/
static int read_single_byte(struct i2c_client *client, u8 reg, u8 *val)
{
	int value;

	value = i2c_smbus_read_byte_data(client, reg);
	if (value < 0) {
		dev_dbg(av7100dev, "i2c smbus read byte failed,read data = %x "
			"from offset:%x\n" , value, reg);
		dev_err(av7100dev, "read_single_byte i2c smbus read byte failed,read data = %x "
			"from offset:%x\n" , value, reg);
		return AV7100_FAIL;
	}

	*val = (u8) value;
	return 0;
}

/****************************************************************************
*	name	=	write_multi_byte
*	func	=	Write a multiple bytes to av7100 through i2c interface
*	input	=	struct i2c_client *client, u8 reg, u8 *buf, u8 nbytes
*	output	=	None
*	return	=	ret
****************************************************************************/
static int write_multi_byte(struct i2c_client *client, u8 reg,
		u8 *buf, u8 nbytes)
{
	int ret;

	ret = i2c_smbus_write_i2c_block_data(client, reg, nbytes, buf);
	if (ret < 0){
		dev_err(av7100dev, "i2c smbus write multi byte error in write_multi_byte fucntioni\n");
		dev_dbg(av7100dev, "i2c smbus write multi byte error\n");
	}

	return ret;
}

/****************************************************************************
*	name	=	write_register_value
*	func	=	Write register value using DSI interface
*	input	=	u32 uccmd, u8 reg, u8 data
*	output	=	None
*	return	=	ret, -1, -EINVAL,
****************************************************************************/
static int write_register_value(u32 uccmd, u8 reg, u8 data)
{
	int ret;
	screen_disp_write_dsi_short write_dsi_s;
#ifdef AV7100_DSI_COMMAND_USES	
	screen_disp_param disp_param;
#endif
        screen_disp_delete disp_delete;
        void *screen_handle;

	screen_handle = screen_display_new();
        if(screen_handle == NULL) {
		dev_dbg(av7100dev," DSI write_register_value screen_handle param == NULL exits\n");
                return -EINVAL;
	}

#ifdef AV7100_DSI_COMMAND_USES	
	disp_param.handle = screen_handle;
        disp_param.output_mode = RT_DISPLAY_HDMI;
        ret = screen_display_set_parameters(&disp_param);
        if (ret != SMAP_LIB_DISPLAY_OK) {
                dev_err(av7100dev,"disp_set_parameters err!\n");
                disp_delete.handle = screen_handle;
                screen_display_delete(&disp_delete);
		return -1;
        }
#endif

	write_dsi_s.handle      = screen_handle;
	write_dsi_s.data_id     = uccmd;
	write_dsi_s.reg_address = reg;
	write_dsi_s.write_data  = data;
	write_dsi_s.output_mode = RT_DISPLAY_HDMI;
	ret = screen_display_write_dsi_short_packet(&write_dsi_s);
	if (ret != SMAP_LIB_DISPLAY_OK) {
        	dev_err(av7100dev,"DSI disp_write_dsi_short 1 err!,ret = %d\n", ret);
        	disp_delete.handle = screen_handle;
        	screen_display_delete(&disp_delete);
		return -1;
	}

	return ret;
}

/****************************************************************************
*	name	=	firmware_download
*	func	=	Download AV7100 firmware using DSI Interface
*	input	=	u32 uccmd, char *fw_buff, int length
*	output	=	None
*	return	=	ret, -1, -ENINVAL
****************************************************************************/
static int firmware_download(u32 uccmd, char *fw_buff, int length)
{
        int ret;
	screen_disp_write_dsi_long write_dsi_l;
#ifdef AV7100_DSI_COMMAND_USES	
	screen_disp_param disp_param;
#endif
        screen_disp_delete disp_delete;
        void *screen_handle;
	
	screen_handle = screen_display_new();
        if(screen_handle == NULL) {
		dev_dbg(av7100dev,"DSI firmware download screen_handle param == NULL exits\n");
                return -EINVAL;
	}

#ifdef AV7100_DSI_COMMAND_USES	
	disp_param.handle = screen_handle;
        disp_param.output_mode = RT_DISPLAY_HDMI;
        ret = screen_display_set_parameters(&disp_param);
        if (ret != SMAP_LIB_DISPLAY_OK) {
                disp_delete.handle = screen_handle;
                screen_display_delete(&disp_delete);
		return -1;
        }
#endif

	write_dsi_l.handle	 = screen_handle;
	write_dsi_l.output_mode  = RT_DISPLAY_HDMI;
	write_dsi_l.data_id	 = uccmd;
	write_dsi_l.dummy 	 = 0;
	write_dsi_l.data_count   = length;
	write_dsi_l.dummy2	 = 0;
	write_dsi_l.write_data   = fw_buff;
	ret = screen_display_write_dsi_long_packet(&write_dsi_l);
	if (ret != SMAP_LIB_DISPLAY_OK) {	
		disp_delete.handle = screen_handle;
		screen_display_delete(&disp_delete);
		return -1;
	}


	return ret;
}

/****************************************************************************
*	name	=	write_uc_command
*	func	=	Write uc command using DSI interface
*	input	=	u8 uccmd, u8 *cmd_buffer, u32 cmd_length
*	output	=	None
*	return	=	ret, -1, -ENINVAL
****************************************************************************/
static int write_uc_command(u8 uccmd, u8 *cmd_buffer, u32 cmd_length)
{
	screen_disp_write_dsi_short write_dsi_s;
	screen_disp_write_dsi_long write_dsi_l;
#ifdef AV7100_DSI_COMMAND_USES	
	screen_disp_param disp_param;
#endif
        screen_disp_delete disp_delete;
        void *screen_handle;
        int ret;

	screen_handle = screen_display_new();
        if(screen_handle == NULL) {
		                return -EINVAL;
	}

#ifdef AV7100_DSI_COMMAND_USES	
	disp_param.handle = screen_handle;
        disp_param.output_mode = RT_DISPLAY_HDMI;
        ret = screen_display_set_parameters(&disp_param);
        if (ret != SMAP_LIB_DISPLAY_OK) {
                disp_delete.handle = screen_handle;
                screen_display_delete(&disp_delete);
		return -1;
        }
#endif

	write_dsi_s.handle      = screen_handle;
	write_dsi_s.data_id     = 0x05;
	write_dsi_s.reg_address = 0x11;
	write_dsi_s.write_data  = 0x00;
	write_dsi_s.output_mode = RT_DISPLAY_HDMI;
	ret = screen_display_write_dsi_short_packet(&write_dsi_s);
	if (ret != SMAP_LIB_DISPLAY_OK) {
        	disp_delete.handle = screen_handle;
        	screen_display_delete(&disp_delete);
		return -1;
	}

	/*wait 120ms */
	msleep(120);


	write_dsi_s.handle      = screen_handle;
	write_dsi_s.data_id     = 0x05;
	write_dsi_s.reg_address = 0x29;
	write_dsi_s.write_data  = 0x00;
	write_dsi_s.output_mode = RT_DISPLAY_HDMI;
	ret = screen_display_write_dsi_short_packet(&write_dsi_s);
	if (ret != SMAP_LIB_DISPLAY_OK) {
		disp_delete.handle = screen_handle;
		screen_display_delete(&disp_delete);
		return -1;
	}

	write_dsi_l.handle	 = screen_handle;
	write_dsi_l.output_mode  = RT_DISPLAY_HDMI;
	write_dsi_l.data_id	 = uccmd;
	write_dsi_l.dummy 	 = 0;
	write_dsi_l.data_count   = cmd_length;
	write_dsi_l.dummy2	 = 0;
	write_dsi_l.write_data   = cmd_buffer;
	ret = screen_display_write_dsi_long_packet(&write_dsi_l);
	if (ret != SMAP_LIB_DISPLAY_OK) {	
		disp_delete.handle = screen_handle;
		screen_display_delete(&disp_delete);
		return -1;
	}


	return ret;
}

/****************************************************************************
*	name	=	exec_uc_command
*	func	=	Execute uc command using DSI interface
*	input	=	u32 uccmd, enum av7100_command_type command_type
*	output	=	None
*	return	=	ret, -1, -EINVAL
****************************************************************************/
static int exec_uc_command(u32 uccmd, enum av7100_command_type command_type)
{
	screen_disp_write_dsi_short write_dsi_s;
#ifdef AV7100_DSI_COMMAND_USES	
	screen_disp_param disp_param;
#endif
	screen_disp_delete disp_delete;
	void *screen_handle;
	int ret;

	screen_handle = screen_display_new();
	if(screen_handle == NULL) {
        	return -EINVAL;
	}
	
#ifdef AV7100_DSI_COMMAND_USES	
	disp_param.handle = screen_handle;
        disp_param.output_mode = RT_DISPLAY_HDMI;
        ret = screen_display_set_parameters(&disp_param);
        if (ret != SMAP_LIB_DISPLAY_OK) {
                disp_delete.handle = screen_handle;
                screen_display_delete(&disp_delete);
		return -1;
        }
#endif

	write_dsi_s.handle	 = screen_handle;
	write_dsi_s.output_mode  = RT_DISPLAY_HDMI;
	write_dsi_s.data_id 	 = uccmd;
	write_dsi_s.reg_address  = command_type;
        /*write_dsi_s.write_data = command_type;*/
	ret = screen_display_write_dsi_short_packet(&write_dsi_s);
	if (ret != SMAP_LIB_DISPLAY_OK) {
		disp_delete.handle = screen_handle;
		screen_display_delete(&disp_delete);
		return -1;
	}

	return ret;
}
/****************************************************************************
*	name	=	read_register_value
*	func	=	Read values from register using DSI interface
*	input	=	u32 cmd, u8 reg, u8 *data
*	output	=	None
*	return	=	ret, -1, -EINVAL
****************************************************************************/
static int read_register_value(u32 cmd, u8 reg, u8 *data)
{
	screen_disp_read_dsi_short read_dsi_s;
#ifdef AV7100_DSI_COMMAND_USES	
	screen_disp_param disp_param;
#endif
	screen_disp_delete disp_delete;
	void *screen_handle;
	int ret;

	screen_handle = screen_display_new();
	if(screen_handle == NULL) {
		dev_dbg(av7100dev,"DSI write_start_read_address set param screen_handle == NULL exits");
        	return -EINVAL;
	}

#ifdef AV7100_DSI_COMMAND_USES	
	disp_param.handle = screen_handle;
        disp_param.output_mode = RT_DISPLAY_HDMI;
        ret = screen_display_set_parameters(&disp_param);
        if (ret != SMAP_LIB_DISPLAY_OK) {
                disp_delete.handle = screen_handle;
                screen_display_delete(&disp_delete);
		return -1;
        }
#endif

	read_dsi_s.handle	 = screen_handle;
	read_dsi_s.output_mode  = RT_DISPLAY_HDMI;
	read_dsi_s.dummy  = 0;
	read_dsi_s.data_id 	 = cmd;
	read_dsi_s.reg_address  = reg;
        read_dsi_s.write_data = 0;
        read_dsi_s.data_count= 1;
        read_dsi_s.read_data = data;
	ret = screen_display_read_dsi_short_packet(&read_dsi_s);
	if (ret != SMAP_LIB_DISPLAY_OK) {;
		disp_delete.handle = screen_handle;
		screen_display_delete(&disp_delete);
		return -1;
	}


	return ret;
}

/****************************************************************************
*	name	=	configuration_video_input_get
*	func	=	Gets the details of video input format from the 
* 			av7100_config global configuration into the 
* 			requested buffer
*	input	=	char *buffer,unsigned int *length
*	output	=	None
*	return	=	0, AV7100_FAIL
****************************************************************************/
static int configuration_video_input_get(char *buffer,
	unsigned int *length)
{
	if (!av7100_config)
		return AV7100_FAIL;

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

	*length = AV7100_COMMAND_VIDEO_INPUT_FORMAT_SIZE - 1;
	return 0;

}

/****************************************************************************
*	name	=	configuration_audio_input_get
*	func	=	Gets the details of audio input format from the 
* 			av7100_config global configuration into the 
* 			requested buffer
*	input	=	char *buffer,unsigned int *length
*	output	=	None
*	return	=	0, AV7100_FAIL
****************************************************************************/
static int configuration_audio_input_get(char *buffer,
	unsigned int *length)
{
	if (!av7100_config)
		return AV7100_FAIL;

	buffer[0] = av7100_config->hdmi_audio_input_cmd.
		audio_input_if_format;
	buffer[1] = av7100_config->hdmi_audio_input_cmd.i2s_input_nb;
	buffer[2] = av7100_config->hdmi_audio_input_cmd.sample_audio_freq;
	buffer[3] = av7100_config->hdmi_audio_input_cmd.audio_word_lg;
	buffer[4] = av7100_config->hdmi_audio_input_cmd.audio_format;
	buffer[5] = av7100_config->hdmi_audio_input_cmd.audio_if_mode;
	buffer[6] = av7100_config->hdmi_audio_input_cmd.audio_mute;

	*length = AV7100_COMMAND_AUDIO_INPUT_FORMAT_SIZE - 1;
	return 0;
}

/****************************************************************************
*	name	=	configuration_video_output_get
*	func	=	Gets the details of video output format from the 
* 			av7100_config global configuration into the 
* 			requested buffer
*	input	=	char *buffer,unsigned int *length
*	output	=	None
*	return	=	0, AV7100_FAIL
****************************************************************************/
static int configuration_video_output_get(char *buffer,
	unsigned int *length)
{
	if (!av7100_config)
		return AV7100_FAIL;

	buffer[0] = av7100_config->hdmi_video_output_cmd.
		video_output_cea_vesa;

	if (buffer[0] == AV7100_CUSTOM) {
		buffer[1] = av7100_config->hdmi_video_output_cmd.
			vsync_polarity;
		buffer[2] = av7100_config->hdmi_video_output_cmd.
			hsync_polarity;
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
			hdmi_video_output_cmd.
				total_vertical_active_in_half_lines);
		buffer[10] = REG_16_8_LSB(av7100_config->
			hdmi_video_output_cmd.
				total_vertical_active_in_half_lines);
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
		buffer[23] = av7100_config->
			hdmi_video_output_cmd.video_type;
		buffer[24] = av7100_config->
			hdmi_video_output_cmd.pixel_repeat;
		buffer[25] = REG_32_8_MSB(av7100_config->
			hdmi_video_output_cmd.pixel_clock_freq_Hz);
		buffer[26] = REG_32_8_MMSB(av7100_config->
			hdmi_video_output_cmd.pixel_clock_freq_Hz);
		buffer[27] = REG_32_8_MLSB(av7100_config->
			hdmi_video_output_cmd.pixel_clock_freq_Hz);
		buffer[28] = REG_32_8_LSB(av7100_config->
			hdmi_video_output_cmd.pixel_clock_freq_Hz);

		*length = AV7100_COMMAND_VIDEO_OUTPUT_FORMAT_SIZE - 1;
	} else {
		*length = 1;
	}

	return 0;
}

/****************************************************************************
*	name	=	configuration_video_scaling_get
*	func	=	Gets the details of video scaling format from the 
* 			av7100_config global configuration into the 
* 			requested buffer
*	input	=	char *buffer,unsigned int *length
*	output	=	None
*	return	=	0, AV7100_FAIL
****************************************************************************/
static int configuration_video_scaling_get(char *buffer,
	unsigned int *length)
{
	if (!av7100_config)
		return AV7100_FAIL;

	buffer[0] = REG_16_8_MSB(av7100_config->hdmi_video_scaling_cmd.
		h_start_in_pixel);
	buffer[1] = REG_16_8_LSB(av7100_config->hdmi_video_scaling_cmd.
		h_start_in_pixel);
	buffer[2] = REG_16_8_MSB(av7100_config->hdmi_video_scaling_cmd.
		h_stop_in_pixel);
	buffer[3] = REG_16_8_LSB(av7100_config->hdmi_video_scaling_cmd.
		h_stop_in_pixel);
	buffer[4] = REG_16_8_MSB(av7100_config->hdmi_video_scaling_cmd.
		v_start_in_line);
	buffer[5] = REG_16_8_LSB(av7100_config->hdmi_video_scaling_cmd.
		v_start_in_line);
	buffer[6] = REG_16_8_MSB(av7100_config->hdmi_video_scaling_cmd.
		v_stop_in_line);
	buffer[7] = REG_16_8_LSB(av7100_config->hdmi_video_scaling_cmd.
		v_stop_in_line);
	buffer[8] = REG_16_8_MSB(av7100_config->hdmi_video_scaling_cmd.
		h_start_out_pixel);
	buffer[9] = REG_16_8_LSB(av7100_config->hdmi_video_scaling_cmd
		.h_start_out_pixel);
	buffer[10] = REG_16_8_MSB(av7100_config->hdmi_video_scaling_cmd.
		h_stop_out_pixel);
	buffer[11] = REG_16_8_LSB(av7100_config->hdmi_video_scaling_cmd.
		h_stop_out_pixel);
	buffer[12] = REG_16_8_MSB(av7100_config->hdmi_video_scaling_cmd.
		v_start_out_line);
	buffer[13] = REG_16_8_LSB(av7100_config->hdmi_video_scaling_cmd.
		v_start_out_line);
	buffer[14] = REG_16_8_MSB(av7100_config->hdmi_video_scaling_cmd.
		v_stop_out_line);
	buffer[15] = REG_16_8_LSB(av7100_config->hdmi_video_scaling_cmd.
		v_stop_out_line);

	*length = AV7100_COMMAND_VIDEO_SCALING_FORMAT_SIZE - 1;
	return 0;
}

/****************************************************************************
*	name	=	configuration_colorspace_conversion_get
*	func	=	Gets the details of color space conversion from the 
* 			av7100_config global configuration into the 
* 			requested buffer
*	input	=	char *buffer,unsigned int *length
*	output	=	None
*	return	=	0, AV7100_FAIL
****************************************************************************/
static int configuration_colorspace_conversion_get(char *buffer,
	unsigned int *length)
{
	if (!av7100_config)
		return AV7100_FAIL;

	buffer[0] = REG_12_8_MSB(av7100_config->
		hdmi_color_space_conversion_cmd.c0);
	buffer[1] = REG_16_8_LSB(av7100_config->
		hdmi_color_space_conversion_cmd.c0);
	buffer[2] = REG_12_8_MSB(av7100_config->
		hdmi_color_space_conversion_cmd.c1);
	buffer[3] = REG_16_8_LSB(av7100_config->
		hdmi_color_space_conversion_cmd.c1);
	buffer[4] = REG_12_8_MSB(av7100_config->
		hdmi_color_space_conversion_cmd.c2);
	buffer[5] = REG_16_8_LSB(av7100_config->
		hdmi_color_space_conversion_cmd.c2);
	buffer[6] = REG_12_8_MSB(av7100_config->
		hdmi_color_space_conversion_cmd.c3);
	buffer[7] = REG_16_8_LSB(av7100_config->
		hdmi_color_space_conversion_cmd.c3);
	buffer[8] = REG_12_8_MSB(av7100_config->
		hdmi_color_space_conversion_cmd.c4);
	buffer[9] = REG_16_8_LSB(av7100_config->
		hdmi_color_space_conversion_cmd.c4);
	buffer[10] = REG_12_8_MSB(av7100_config->
		hdmi_color_space_conversion_cmd.c5);
	buffer[11] = REG_16_8_LSB(av7100_config->
		hdmi_color_space_conversion_cmd.c5);
	buffer[12] = REG_12_8_MSB(av7100_config->
		hdmi_color_space_conversion_cmd.c6);
	buffer[13] = REG_16_8_LSB(av7100_config->
		hdmi_color_space_conversion_cmd.c6);
	buffer[14] = REG_12_8_MSB(av7100_config->
		hdmi_color_space_conversion_cmd.c7);
	buffer[15] = REG_16_8_LSB(av7100_config->
		hdmi_color_space_conversion_cmd.c7);
	buffer[16] = REG_12_8_MSB(av7100_config->
		hdmi_color_space_conversion_cmd.c8);
	buffer[17] = REG_16_8_LSB(av7100_config->
		hdmi_color_space_conversion_cmd.c8);
	buffer[18] = REG_10_8_MSB(av7100_config->
		hdmi_color_space_conversion_cmd.aoffset);
	buffer[19] = REG_16_8_LSB(av7100_config->
		hdmi_color_space_conversion_cmd.aoffset);
	buffer[20] = REG_10_8_MSB(av7100_config->
		hdmi_color_space_conversion_cmd.boffset);
	buffer[21] = REG_16_8_LSB(av7100_config->
		hdmi_color_space_conversion_cmd.boffset);
	buffer[22] = REG_10_8_MSB(av7100_config->
		hdmi_color_space_conversion_cmd.coffset);
	buffer[23] = REG_16_8_LSB(av7100_config->
		hdmi_color_space_conversion_cmd.coffset);
	buffer[24] = av7100_config->hdmi_color_space_conversion_cmd.lmax;
	buffer[25] = av7100_config->hdmi_color_space_conversion_cmd.lmin;
	buffer[26] = av7100_config->hdmi_color_space_conversion_cmd.cmax;
	buffer[27] = av7100_config->hdmi_color_space_conversion_cmd.cmin;

	*length = AV7100_COMMAND_COLORSPACECONVERSION_SIZE - 1;
	return 0;
}

/****************************************************************************
*	name	=	configuration_cec_message_write_get
*	func	=	Gets the details of cec message written from the 
* 			av7100_config global configuration into the 
* 			requested buffer
*	input	=	char *buffer,unsigned int *length
*	output	=	None
*	return	=	0, AV7100_FAIL
****************************************************************************/
static int configuration_cec_message_write_get(char *buffer,
	unsigned int *length)
{
	if (!av7100_config)
		return AV7100_FAIL;

	buffer[0] = av7100_config->hdmi_cec_message_write_cmd.buffer_length;
	memcpy(&buffer[1], av7100_config->hdmi_cec_message_write_cmd.buffer,
		HDMI_CEC_MESSAGE_WRITE_BUFFER_SIZE);

	*length = AV7100_COMMAND_CEC_MESSAGE_WRITE_SIZE - 1;
	return 0;
}

/****************************************************************************
*	name	=	configuration_cec_message_read_get
*	func	=	Gets the details of cec message read from the 
* 			av7100_config global configuration into the 
* 			requested buffer
*	input	=	char *buffer,unsigned int *length
*	output	=	None
*	return	=	0, AV7100_FAIL
****************************************************************************/
static int configuration_cec_message_read_get(char *buffer,
	unsigned int *length)
{
	if (!av7100_config)
		return AV7100_FAIL;

	/* No buffer data */
	*length = AV7100_COMMAND_CEC_MESSAGE_READ_BACK_SIZE - 1;
	return 0;
}

/****************************************************************************
*	name	=	configuration_denc_get
*	func	=	Gets the details of denc configuration from the 
* 			av7100_config global configuration into the 
* 			requested buffer
*	input	=	char *buffer,unsigned int *length
*	output	=	None
*	return	=	0, AV7100_FAIL
****************************************************************************/
static int configuration_denc_get(char *buffer,
	unsigned int *length)
{
	if (!av7100_config)
		return AV7100_FAIL;

	buffer[0] = av7100_config->hdmi_denc_cmd.cvbs_video_format;
	buffer[1] = av7100_config->hdmi_denc_cmd.standard_selection;
	buffer[2] = av7100_config->hdmi_denc_cmd.on_off;
	buffer[3] = av7100_config->hdmi_denc_cmd.macrovision_on_off;
	buffer[4] = av7100_config->hdmi_denc_cmd.internal_generator;

	*length = AV7100_COMMAND_DENC_SIZE - 1;
	return 0;
}

/****************************************************************************
*	name	=	configuration_hdmi_get
*	func	=	Gets the details of hdmic configuration read from the 
* 			av7100_config global configuration into the 
* 			requested buffer
*	input	=	char *buffer,unsigned int *length
*	output	=	None
*	return	=	0, AV7100_FAIL
****************************************************************************/
static int configuration_hdmi_get(char *buffer, unsigned int *length)
{
	if (!av7100_config)
		return AV7100_FAIL;

	buffer[0] = av7100_config->hdmi_cmd.hdmi_mode;
	buffer[1] = av7100_config->hdmi_cmd.hdmi_format;
	buffer[2] = av7100_config->hdmi_cmd.dvi_format;

	*length = AV7100_COMMAND_HDMI_SIZE - 1;
	return 0;
}

/****************************************************************************
*	name	=	configuration_hdcp_sendkey_get
*	func	=	Gets the details of hdcp sendkey from the 
* 			av7100_config global configuration into the 
* 			requested buffer
*	input	=	char *buffer,unsigned int *length
*	output	=	None
*	return	=	0, AV7100_FAIL
****************************************************************************/
static int configuration_hdcp_sendkey_get(char *buffer,
	unsigned int *length)
{
	if (!av7100_config)
		return AV7100_FAIL;

	buffer[0] = av7100_config->hdmi_hdcp_send_key_cmd.key_number;
	memcpy(&buffer[1], av7100_config->hdmi_hdcp_send_key_cmd.data,
		av7100_config->hdmi_hdcp_send_key_cmd.data_len);

	*length = av7100_config->hdmi_hdcp_send_key_cmd.data_len + 1;
	return 0;
}

/****************************************************************************
*	name	=	configuration_hdcp_management_get
*	func	=	Gets the details of hdcp management configuration 
* 			from the av7100_config global configuration into the 
*			requested buffer
*	input	=	char *buffer,unsigned int *length
*	output	=	None
*	return	=	0, AV7100_FAIL
****************************************************************************/
static int configuration_hdcp_management_get(char *buffer,
	unsigned int *length)
{
	if (!av7100_config)
		return AV7100_FAIL;

	buffer[0] = av7100_config->hdmi_hdcp_management_format_cmd.req_type;
	buffer[1] = av7100_config->hdmi_hdcp_management_format_cmd.req_encr;
	buffer[2] = av7100_config->hdmi_hdcp_management_format_cmd.encr_use;

	*length = AV7100_COMMAND_HDCP_MANAGEMENT_SIZE - 1;
	return 0;
}

/****************************************************************************
*	name	=	configuration_infoframe_get
*	func	=	Gets the details of infoframes configuration from 
* 			the av7100_config global configuration into the 
* 			requested buffer
*	input	=	char *buffer,unsigned int *length
*	output	=	None
*	return	=	0, AV7100_FAIL
****************************************************************************/
static int configuration_infoframe_get(char *buffer,
	unsigned int *length)
{
	if (!av7100_config)
		return AV7100_FAIL;

	buffer[0] = av7100_config->hdmi_infoframes_cmd.type;
	buffer[1] = av7100_config->hdmi_infoframes_cmd.version;
	buffer[2] = av7100_config->hdmi_infoframes_cmd.length;
	buffer[3] = av7100_config->hdmi_infoframes_cmd.crc;
	memcpy(&buffer[4], av7100_config->hdmi_infoframes_cmd.data,
	HDMI_INFOFRAME_DATA_SIZE);

	*length = AV7100_COMMAND_INFOFRAMES_SIZE - 1;
	return 0;
}

/****************************************************************************
*	name	=	av7100_edid_section_readback_get
*	func	=	Gets the details of edid section readback command 
* 			from the av7100_config global configuration into the 
* 			requested buffer
*	input	=	char *buffer,unsigned int *length
*	output	=	None
*	return	=	0
****************************************************************************/
static int av7100_edid_section_readback_get(char *buffer, unsigned int *length)
{
	buffer[0] = av7100_config->hdmi_edid_section_readback_cmd.address;
	buffer[1] = av7100_config->hdmi_edid_section_readback_cmd.
		block_number;

	*length = AV7100_COMMAND_EDID_SECTION_READBACK_SIZE - 1;
	return 0;
}

/****************************************************************************
*	name	=	configuration_pattern_generator_get
*	func	=	Gets the details of pattern generator commnand from 
* 			the av7100_config global configuration into the 
*			requested buffer
*	input	=	char *buffer,unsigned int *length
*	output	=	None
*	return	=	0, AV7100_FAIL
****************************************************************************/
static int configuration_pattern_generator_get(char *buffer,
	unsigned int *length)
{
	if (!av7100_config)
		return AV7100_FAIL;

	buffer[0] = av7100_config->hdmi_pattern_generator_cmd.pattern_type;
	buffer[1] = av7100_config->hdmi_pattern_generator_cmd.
		pattern_video_format;
	buffer[2] = av7100_config->hdmi_pattern_generator_cmd.
		pattern_audio_mode;

	*length = AV7100_COMMAND_PATTERNGENERATOR_SIZE - 1;
	return 0;
}

/****************************************************************************
*	name	=	configuration_fuse_aes_key_get
*	func	=	Gets the details to fuse aes key  from 
*			the av7100_config global configuration into the 
*			requested buffer
*	input	=	char *buffer,unsigned int *length
*	output	=	None
*	return	=	0, AV7100_FAIL
****************************************************************************/
static int configuration_fuse_aes_key_get(char *buffer,
	unsigned int *length)
{
	if (!av7100_config)
		return AV7100_FAIL;

	buffer[0] = av7100_config->hdmi_fuse_aes_key_cmd.fuse_operation;
	memcpy(&buffer[1], av7100_config->hdmi_fuse_aes_key_cmd.key,
		HDMI_FUSE_AES_KEY_SIZE);

	*length = AV7100_COMMAND_FUSE_AES_KEY_SIZE - 1;
	return 0;
}

/****************************************************************************
*	name	=	get_command_return_data
*	func	=	This function gets the return value from av7100. 
* 			The return value might be a single byte or a array 
* 			of 8 bytes depending the command. 
*	input	=	struct i2c_client *i2c,enum av7100_command_type 
* 			command_type,u8 *command_buffer,u8 *buffer_length,
* 			u8 *buffer
*	output	=	None
*	return	=	retval
****************************************************************************/
static int get_command_return_data(struct i2c_client *i2c,
	enum av7100_command_type command_type,
	u8 *command_buffer,
	u8 *buffer_length,
	u8 *buffer)
{
	int retval = 0;
	char val;
	int index = 0;

	/* Get the first return byte */
	
	retval = register_read_internal(AV7100_COMMAND_OFFSET, &val);
	if (retval){
		goto get_command_return_data_fail1r;
	}

	if (val != (0x80 | command_type)) {
		retval = AV7100_FAIL;
		goto get_command_return_data_fail1v;
	}

	switch (command_type) {
	case AV7100_COMMAND_VIDEO_INPUT_FORMAT:
	case AV7100_COMMAND_AUDIO_INPUT_FORMAT:
	case AV7100_COMMAND_VIDEO_OUTPUT_FORMAT:
	case AV7100_COMMAND_VIDEO_SCALING_FORMAT:
	case AV7100_COMMAND_COLORSPACECONVERSION:
	case AV7100_COMMAND_CEC_MESSAGE_WRITE:
	case AV7100_COMMAND_DENC:
	case AV7100_COMMAND_HDMI:
	case AV7100_COMMAND_HDCP_SENDKEY:
	case AV7100_COMMAND_INFOFRAMES:
	case AV7100_COMMAND_PATTERNGENERATOR:
		/* Get the second return byte */

		retval = register_read_internal(AV7100_COMMAND_OFFSET + 1, &val);
		if (retval){
		dev_err(av7100dev,"In get_command_return_data function3\n");
			goto get_command_return_data_fail2r;
		}

		if (val) {
		dev_err(av7100dev,"In get_command_return_data function4\n");
			retval = AV7100_FAIL;
			goto get_command_return_data_fail2v;
		}
		break;

	case AV7100_COMMAND_CEC_MESSAGE_READ_BACK:
		if ((buffer == NULL) ||	(buffer_length == NULL)) {
			retval = AV7100_FAIL;
			goto get_command_return_data_fail;
		}

		/* Get the return buffer length */
		retval = register_read_internal(AV7100_COMMAND_OFFSET + CEC_ADDR_OFFSET, &val);
		if (retval)
			goto get_command_return_data_fail;

		/* TODO: buffer_length is always zero */
		/* *buffer_length = val;*/
		*buffer_length = val;

		if (*buffer_length >
			HDMI_CEC_READ_MAXSIZE) {
			*buffer_length = HDMI_CEC_READ_MAXSIZE;
		}

#ifdef AV7100_DEBUG_EXTRA
		dev_dbg(av7100dev, "return data: ");
#endif

		/* Get the return buffer */
		for (index = 0; index < *buffer_length; ++index) {
			retval = register_read_internal(AV7100_COMMAND_OFFSET + CEC_ADDR_OFFSET + 1
                                + index,
                                &val);
			if (retval) {
				*buffer_length = 0;
				goto get_command_return_data_fail;
			} else {
				*(buffer + index) = val;
#ifdef AV7100_DEBUG_EXTRA
				dev_dbg(av7100dev, "%02x ", *(buffer + index));
#endif
			}
		}
#ifdef AV7100_DEBUG_EXTRA
		dev_dbg(av7100dev, "\n");
#endif
		break;

	case AV7100_COMMAND_HDCP_MANAGEMENT:
		/* Get the second return byte */
		retval = register_read_internal(AV7100_COMMAND_OFFSET + 1, &val);
		if (retval) {
			goto get_command_return_data_fail2r;
		} else {
			/* Check the second return byte */
			if (val)
				goto get_command_return_data_fail2v;
		}

		if ((buffer == NULL) || (buffer_length == NULL))
			/* Ignore return data */
			break;

		/* Get the return buffer length */
		if (command_buffer[0] ==
			HDMI_REQUEST_FOR_REVOCATION_LIST_INPUT) {
			*buffer_length = 0x1F;
		} else {
			*buffer_length = 0x0;
		}

#ifdef AV7100_DEBUG_EXTRA
		dev_dbg(av7100dev, "return data: ");
#endif
		/* Get the return buffer */
		for (index = 0; index < *buffer_length; ++index) {
			retval = register_read_internal(AV7100_COMMAND_OFFSET + 2 + index,
                                &val);
			if (retval) {
				*buffer_length = 0;
				goto get_command_return_data_fail;
			} else {
				*(buffer + index) = val;
#ifdef AV7100_DEBUG_EXTRA
				dev_dbg(av7100dev, "%02x ", *(buffer + index));
#endif
			}
		}
#ifdef AV7100_DEBUG_EXTRA
		dev_dbg(av7100dev, "\n");
#endif
		break;

	case AV7100_COMMAND_EDID_SECTION_READBACK:
		if ((buffer == NULL) ||	(buffer_length == NULL)) {
			retval = AV7100_FAIL;
			goto get_command_return_data_fail;
		}

		/* Return buffer length is fixed */
		*buffer_length = 0x80;

#ifdef AV7100_DEBUG_EXTRA
		dev_dbg(av7100dev, "return data: ");
#endif
		/* Get the return buffer */
		for (index = 0; index < *buffer_length; ++index) {
			retval = register_read_internal(AV7100_COMMAND_OFFSET + 1 + index,
                                &val);
			if (retval) {
				*buffer_length = 0;
				goto get_command_return_data_fail;
			} else {
				*(buffer + index) = val;
#ifdef AV7100_DEBUG_EXTRA
				dev_dbg(av7100dev, "%02x ", *(buffer + index));
#endif
			}
		}
#ifdef AV7100_DEBUG_EXTRA
		dev_dbg(av7100dev, "\n");
#endif
		break;

	case AV7100_COMMAND_FUSE_AES_KEY:
		if ((buffer == NULL) ||	(buffer_length == NULL)) {
			retval = AV7100_FAIL;
			goto get_command_return_data_fail;
		}

		/* Get the second return byte */
		retval = register_read_internal(AV7100_COMMAND_OFFSET + 1, &val);
		if (retval)
			goto get_command_return_data_fail2r;

		/* Check the second return byte */
		if (val) {
			retval = AV7100_FAIL;
			goto get_command_return_data_fail2v;
		}

		/* Return buffer length is fixed */
		*buffer_length = 0x2;

		/* Get CRC */
		retval = register_read_internal(AV7100_COMMAND_OFFSET + 2, &val);
		if (retval)
			goto get_command_return_data_fail;

		*(buffer + 0) = val;
#ifdef AV7100_DEBUG_EXTRA
		dev_dbg(av7100dev, "CRC:%02x ", val);
#endif

		/* Get programmed status */
		retval = register_read_internal(AV7100_COMMAND_OFFSET + 3, &val);
		if (retval)
			goto get_command_return_data_fail;

		*(buffer + 1) = val;
#ifdef AV7100_DEBUG_EXTRA
		dev_dbg(av7100dev, "programmed:%02x ", val);
#endif
		break;

	default:
		retval = AV7100_INVALID_COMMAND;
		break;
	}

	return retval;
get_command_return_data_fail1r:
	dev_dbg(av7100dev, "%s Reading first return byte failed\n", __func__);
	return retval;
get_command_return_data_fail1v:
	dev_dbg(av7100dev, "%s First return byte is wrong:%x\n", __func__, val);
	return retval;
get_command_return_data_fail2r:
	dev_dbg(av7100dev, "%s Reading 2nd return byte failed\n", __func__);
	return retval;
get_command_return_data_fail2v:
	dev_dbg(av7100dev, "%s 2nd return byte is wrong:%x\n", __func__, val);
	return retval;
get_command_return_data_fail:
	dev_dbg(av7100dev, "%s FAIL\n", __func__);
	return retval;
}

/****************************************************************************
*	name	=	av7100_powerup1
*	func	=	av8100_powerup1 is used to power up AV7100 from 
* 			shutdown state to standby state
*	input	=	void
*	output	=	None
*	return	=	retval, -EFAULT
****************************************************************************/
static int av7100_powerup1(void)
{
	int retval = 0;



	/* POWERDWN ball is set to HIGH.*/
	gpio_set_value(GPIO_PORT8, 1);

    
	/* 1MS delay Figure 11 in AV7100 datasheet*/
	mdelay(AV7100_POWERON_WAITTIME1_MS);
        
	av7100_set_state(AV7100_OPMODE_STANDBY);

	/* Get chip version - Not documented in Av7100 datasheet but from 
	   reference code we come to know that chip version lies in Standby 
	   Pending Interrupt register
	*/
	retval = av7100_reg_stby_pend_int_r(NULL, NULL, &chip_version);
	if (retval) {
		dev_err(av7100dev, "Failed to read chip version\n");
		return -EFAULT;
	}

	switch (chip_version) {
	case AV7100_CHIPVER_1:
	case AV7100_CHIPVER_2:
		break;

	default:
		return -EFAULT;
		break;
	}
	return retval;
}

/****************************************************************************
*	name	=	av7100_powerup2
*	func	=	av8100_powerup2 is used to power up AV7100 from 
* 			standby state to scan state
*	input	=	void
*	output	=	None
*	return	=	retval, -EFAULT
****************************************************************************/
static int av7100_powerup2(void)
{
	int retval = 0;
	
    
	/* Master clock timing, runninng
	   STBY is set high here itself otherwise we observed
	   Plugdetect is not happening.*/
	retval = av7100_reg_stby_w(AV7100_STANDBY_STBY_HIGH, 
	                         AV7100_MASTER_CLOCK_TIMING);

	if (retval) {
		dev_err(av7100dev,
			"Failed to write the value to av7100 register\n");
		return -EFAULT;
	}
	/* ON time & OFF time on 5v HDMI plug detect */
	retval = av7100_reg_hdmi_5_volt_time_w(
			av7100_globals->hdmi_off_time,
			av7100_globals->on_time);
	if (retval) {
		dev_err(av7100dev,
			"Failed to write the value to av7100 register\n");
		return -EFAULT;
	}

	/* Need to wait before proceeding */
	mdelay(AV7100_POWERON_WAITTIME2_MS);

	av7100_set_state(AV7100_OPMODE_SCAN);
        /*printk("AV7100 powerup success\n");*/
	return retval;
}

/****************************************************************************
*	name	=	av7100_powerup2
*	func	=	Utility used by hdmi.c to power up av7100
*	input	=	void
*	output	=	None
*	return	=	0, -EFAULT
****************************************************************************/
int av7100_powerup(void)
{
	if (av7100_powerup1()) {
		dev_err(av7100dev, "av7100_powerup1 fail\n");
		return -EFAULT;
	}

	return av7100_powerup2();
}

/****************************************************************************
*	name	=	av7100_powerdown
*	func	=	Utility used by hdmi.c to power down av7100
*	input	=	void
*	output	=	None
*	return	=	0
****************************************************************************/
int av7100_powerdown(void)
{
	gpio_set_value(GPIO_PORT8, 0);
	av7100_set_state(AV7100_OPMODE_SHUTDOWN);

	return 0;
}

/****************************************************************************
*	name	=	av7100_download_firmware
*	func	=	Function that sets up AV7100 register for 
* 			firmware download and start firmware download using 
* 			I2C interface
*	input	=	char *fw_buff, int nbytes,enum interface_type if_type
*	output	=	None
*	return	=	retval, -EFAULT
****************************************************************************/
int av7100_download_firmware(char *fw_buff, int nbytes,
	enum interface_type if_type)
{
	int retval = 0;
	int temp = 0x0;
	int increment = 15;
	int index = 0;
	int size = 0x0;
	int tempnext = 0x0;
	char val = 0x0;
	char CheckSum = 0;
	int cnt = 10;
	struct i2c_client *i2c;
	u8 cecrec;
	u8 cectrx;
	u8 uc;
	u8 onuvb;
	u8 hdcps;
	u8 fdl;
	u8 hld;
	u8 wa;
	u8 ra;
	char tempbuff;

	if (!av7100_config) {
		retval = AV7100_FAIL;
		goto av7100_download_firmware_out;
	}
        
	if (fw_buff == NULL) {
		switch (chip_version) {
		case AV7100_CHIPVER_1:
			fw_buff = av7100_fw_buff;
			nbytes = AV7100_FW_SIZE;
			break;

		case AV7100_CHIPVER_2:
		default:
			fw_buff = av7100_fw_buff;
			nbytes = AV7100_FW_SIZE;
			break;
		}
	}

	i2c = av7100_config->client;
	/* Enable firmware download in General Control register*/
	retval = av7100_reg_gen_ctrl_w(
		AV7100_GENERAL_CONTROL_FDL_HIGH,
		AV7100_GENERAL_CONTROL_HLD_HIGH,
		AV7100_GENERAL_CONTROL_WA_LOW,
		AV7100_GENERAL_CONTROL_RA_LOW);
	if (retval) {
		dev_err(av7100dev,
			"Failed to write the value to av7100 register\n");
		return -EFAULT;
	}
    
	retval = av7100_reg_gen_ctrl_r(&fdl, &hld, &wa, &ra);
	if (retval) {
		dev_err(av7100dev,
			"Failed to read the value from av7100 register\n");
		return -EFAULT;
	} else {
		dev_err(av7100dev, "GENERAL_CONTROL_REG register fdl:%d "
			"hld:%d wa:%d ra:%d\n", fdl, hld, wa, ra);
	}

	LOCK_AV7100_HW;
    /* According to the Setting up AVX100 doc the increment 
	   of transfering firmware bytes is 0xf so the last bytes 
	   which is calculated from mod of 0xf is tansfered in the 
	   last */
	   
#if 0
	temp = nbytes % increment;
	for (size = 0; size < (nbytes-temp); size = size + increment,
		index += increment) {
		if (if_type == I2C_INTERFACE) {
			retval = write_multi_byte(i2c,
				AV7100_FIRMWARE_DOWNLOAD_ENTRY, fw_buff + size,
				increment);
			if (retval) {
				dev_dbg(av7100dev, "Failed to download the "
					"av7100 firmware\n");
				retval = -EFAULT;
				UNLOCK_AV7100_HW;
				goto av7100_download_firmware_out;
			}
		} else if (if_type == DSI_INTERFACE) {
                        retval = firmware_download(AV7100_DCS_FIRMWARE_DOWNLOAD_UCCMD, fw_buff + size, increment); 
                        if (retval) {
				dev_dbg(av7100dev,
					"DSI_INTERFACE is currently not supported\n");
                        	retval = -EFAULT;
				UNLOCK_AV7100_HW;
				goto av7100_download_firmware_out;
                	}
		} else {
			retval = AV7100_INVALID_INTERFACE;
			UNLOCK_AV7100_HW;
			goto av7100_download_firmware_out;
		}

		for (tempnext = size; tempnext < (increment+size); tempnext++)
			av7100_receivetab[tempnext] = fw_buff[tempnext];
	}

	/* Transfer last firmware bytes */
	if (if_type == I2C_INTERFACE) {
		retval = write_multi_byte(i2c,
			AV7100_FIRMWARE_DOWNLOAD_ENTRY, fw_buff + size, temp);
		if (retval) {
			dev_err(av7100dev,
				"Failed to download the av7100 firmware\n");
			retval = -EFAULT;
			UNLOCK_AV7100_HW;
			goto av7100_download_firmware_out;
		}
	} else if (if_type == DSI_INTERFACE) {
		
        	retval = firmware_download(AV7100_DCS_FIRMWARE_DOWNLOAD_UCCMD, fw_buff + size, temp);
                if (retval) {
			dev_dbg(av7100dev,
				"Failed to download the av7100 firmware\n");
			retval = -EFAULT;
			UNLOCK_AV7100_HW;
			goto av7100_download_firmware_out;
        	}
	} else {
		retval = AV7100_INVALID_INTERFACE;
		UNLOCK_AV7100_HW;
		goto av7100_download_firmware_out;
	}

	for (tempnext = size; tempnext < (size+temp); tempnext++)
		av7100_receivetab[tempnext] = fw_buff[tempnext];
	/* check transfer*/
	for (size = 0; size < nbytes; size++) {
		CheckSum = CheckSum ^ fw_buff[size];
		if (av7100_receivetab[size] != fw_buff[size]) {
			dev_err(av7100dev, ">Fw download fail....i=%d\n", size);
			dev_err(av7100dev, "Transm = %x, Receiv = %x\n",
				fw_buff[size], av7100_receivetab[size]);
		}
	}

#else
	for (size = 0; size < AV7100_FW_SIZE; size++) {
		retval = write_single_byte(i2c, AV7100_FIRMWARE_DOWNLOAD_ENTRY, fw_buff[size]);
		if (retval) {
			dev_err(av7100dev, "Failed to download the av7100 firmware\n");
			retval = -EFAULT;
			UNLOCK_AV7100_HW;
			goto av7100_download_firmware_out;
		}
	}
	retval = read_single_byte(i2c, AV7100_GENERAL_CONTROL, &tempbuff);
	if (retval) {
		dev_err(av7100dev, "Failed to download the av7100 firmware 2\n");
		retval = -EFAULT;
		UNLOCK_AV7100_HW;
		goto av7100_download_firmware_out;
	}
	dev_err(av7100dev, "GENERAL_CONTROL=0x%02x\n", tempbuff);

	tempbuff = 0x00;
	retval = write_single_byte(i2c, AV7100_GENERAL_CONTROL, &tempbuff);
	if (retval) {
		dev_err(av7100dev, "Failed to download the av7100 firmware 3\n");
		retval = -EFAULT;
		UNLOCK_AV7100_HW;
		goto av7100_download_firmware_out;
	}
	dev_err(av7100dev, "GENERAL_CONTROL=0x%02x\n", tempbuff);

#endif

#if 1
	for(size=0; size<0xffff; size++)
	{
		retval = read_single_byte(i2c, AV7100_GENERAL_CONTROL, &tempbuff);
		if (retval) {
			dev_err(av7100dev, "Failed to download the av7100 firmware 3\n");
			retval = -EFAULT;
			UNLOCK_AV7100_HW;
			goto av7100_download_firmware_out;
		}
		
		if ((tempbuff & 0x30) == 0x00) {
			dev_err(av7100dev, "GENERAL_CONTROL=0x%02x\n", tempbuff);
			dev_err(av7100dev, "firmware is running\n");
			break;

		}
	}

#endif

        UNLOCK_AV7100_HW;

#if 0
	retval = av7100_reg_fw_dl_entry_r(&val);
	if (retval) {
		dev_err(av7100dev,
			"Failed to read the value from the av7100 register\n");
		retval = -EFAULT;
		goto av7100_download_firmware_out;
	}

	dev_dbg(av7100dev, "CheckSum:%x,val:%x\n", CheckSum, val);

	if (CheckSum != val) {
		dev_err(av7100dev,
			">Fw downloading.... FAIL CheckSum issue\n");
		dev_err(av7100dev, "Checksum = %d\n", CheckSum);
		dev_err(av7100dev, "Checksum read: %d\n", val);
		retval = AV7100_FWDOWNLOAD_FAIL;
		goto av7100_download_firmware_out;
	} else {
		dev_err(av7100dev, ">Fw downloading.... success\n");
	}
#endif
	/* Set to idle mode By closing the Firmware download bit*/
	av7100_reg_gen_ctrl_w(AV7100_GENERAL_CONTROL_FDL_LOW,
		AV7100_GENERAL_CONTROL_HLD_LOW,	AV7100_GENERAL_CONTROL_WA_LOW,
		AV7100_GENERAL_CONTROL_RA_LOW);
	if (retval) {
		dev_dbg(av7100dev,
			"Failed to write the value to the av7100 register\n");
		retval = -EFAULT;
		goto av7100_download_firmware_out;
	}


       
	cnt = 3;
	retval = av7100_reg_gen_status_r(&cecrec, &cectrx, &uc,
		&onuvb, &hdcps);
	while ((retval == 0) && (uc != 0x1) && (cnt-- > 0)) {
		dev_dbg(av7100dev, "av7100 wait2\n");
		
		for (temp = 0; temp < 0xFFFFF; temp++)
			;

		retval = av7100_reg_gen_status_r(&cecrec, &cectrx,
			&uc, &onuvb, &hdcps);
	}
        if(cnt == 0) {
		dev_err(av7100dev,"Error UC is not yet ready to receive configuration\n");
        }
	
        if (retval)  {
		dev_dbg(av7100dev,
			"Failed to read the value from the av7100 register\n");
		retval = -EFAULT;
		goto av7100_download_firmware_out;
	}
	av7100_set_state(AV7100_OPMODE_INIT);

av7100_download_firmware_out:
	return retval;
}

/****************************************************************************
*	name	=	av7100_disable_interrupt
*	func	=	Disables interrupts for AV7100
*	input	=	void
*	output	=	None
*	return	=	retval, AV7100_FAIL, -EFAULT
****************************************************************************/
int av7100_disable_interrupt(void)
{
	int retval;
	struct i2c_client *i2c;

	if (!av7100_config) {
		retval = AV7100_FAIL;
		goto av7100_disable_interrupt_out;
	}

	i2c = av7100_config->client;
	retval = av7100_reg_stby_pend_int_w(
			AV7100_STANDBY_PENDING_INTERRUPT_HPDI_LOW,
			AV7100_STANDBY_PENDING_INTERRUPT_ONI_LOW);
	if (retval) {
		dev_err(av7100dev,
			"Failed to write the value to av7100 register\n");
		retval = -EFAULT;
		goto av7100_disable_interrupt_out;
	}

	retval = av7100_reg_gen_int_mask_w(
			AV7100_GENERAL_INTERRUPT_MASK_EOCM_HIGH,
			AV7100_GENERAL_INTERRUPT_MASK_VSIM_HIGH,
			AV7100_GENERAL_INTERRUPT_MASK_VSOM_HIGH,
			AV7100_GENERAL_INTERRUPT_MASK_CECM_HIGH,
			AV7100_GENERAL_INTERRUPT_MASK_HDCPM_HIGH,
			AV7100_GENERAL_INTERRUPT_MASK_UOVBM_HIGH);
	if (retval) {
		dev_err(av7100dev,
			"Failed to write the value to av7100 register\n");
		retval = -EFAULT;
		goto av7100_disable_interrupt_out;
	}
	retval = av7100_reg_stby_int_mask_w(
			AV7100_STANDBY_INTERRUPT_MASK_HPDM_LOW,
			AV7100_STANDBY_INTERRUPT_MASK_STBYGPIOCFG_INPUT,
			AV7100_STANDBY_INTERRUPT_MASK_IPOL_LOW);
	if (retval) {
		dev_err(av7100dev,
			"Failed to write the value to av7100 register\n");
		retval = -EFAULT;
		goto av7100_disable_interrupt_out;
	}

#ifdef AV7100_PLUGIN_DETECT_VIA_TIMER_INTERRUPTS
	del_timer(&av7100_timer);

	if (av7100_globals) {
		/* Reset to be able to detect changes */
		av7100_globals->hpds_old = HPDS_INVALID;
		av7100_globals->cpds_old = CPDS_INVALID;
		av7100_globals->cecrx_old = CECRX_INVALID;
	}
#endif


av7100_disable_interrupt_out:
	return retval;
}

/****************************************************************************
*	name	=	av7100_enable_interrupt
*	func	=	Enables interrupts for AV7100
*	input	=	void
*	output	=	None
*	return	=	retval, AV7100_FAIL, -EFAULT
****************************************************************************/
int av7100_enable_interrupt(void)
{
	int retval;
	struct i2c_client *i2c;
	if (!av7100_config) {
		retval = AV7100_FAIL;
		goto av7100_enable_interrupt_out;
	}

	i2c = av7100_config->client;

	if (retval) {
		dev_err(av7100dev,
			"Failed to write the value to av7100 register\n");
		retval = -EFAULT;
		goto av7100_enable_interrupt_out;
	}

	retval = av7100_reg_gen_int_mask_w(
			AV7100_GENERAL_INTERRUPT_MASK_EOCM_LOW,
			AV7100_GENERAL_INTERRUPT_MASK_VSIM_LOW,
			AV7100_GENERAL_INTERRUPT_MASK_VSOM_LOW,
			AV7100_GENERAL_INTERRUPT_MASK_CECM_LOW,
			AV7100_GENERAL_INTERRUPT_MASK_HDCPM_HIGH,
			AV7100_GENERAL_INTERRUPT_MASK_UOVBM_LOW);
	if (retval) {
		dev_err(av7100dev,
			"Failed to write the value to av7100 register\n");
		retval = -EFAULT;
		goto av7100_enable_interrupt_out;
	}

	/* Changes start here */

	retval = av7100_reg_stby_int_mask_w(
			AV7100_STANDBY_INTERRUPT_MASK_HPDM_HIGH,
			AV7100_STANDBY_INTERRUPT_MASK_STBYGPIOCFG_INPUT,
			AV7100_STANDBY_INTERRUPT_MASK_IPOL_HIGH);

	/* Changes end here */
	if (retval) {
		dev_err(av7100dev,
			"Failed to write the value to av7100 register\n");
		retval = -EFAULT;
		goto av7100_enable_interrupt_out;
	}

#ifdef AV7100_PLUGIN_DETECT_VIA_TIMER_INTERRUPTS
	init_timer(&av7100_timer);
	av7100_timer.expires = jiffies + AV7100_TIMER_INTERRUPT_POLLING_TIME;
	av7100_timer.function = av7100_timer_int;
	av7100_timer.data = 0;
	add_timer(&av7100_timer);
#endif

av7100_enable_interrupt_out:
	return retval;
}

/****************************************************************************
*	name	=	register_write_internal
*	func	=	Write values internal registers of AV7100
*	input	=	u8 offset, u8 value
*	output	=	None
*	return	=	retval, AV7100_FAIL, -EINVAL
****************************************************************************/
static int register_write_internal(u8 offset, u8 value)
{
	int retval;
	struct i2c_client *i2c;

	if (!av7100_config) {
		retval = AV7100_FAIL;
		goto av7100_register_write_out;
	}

	i2c = av7100_config->client;

	/* Write to register */
	if(av7100_globals->if_type == I2C_INTERFACE) { 
		retval = write_single_byte(i2c, offset, value);
		if (retval) {
			dev_dbg(av7100dev,
				"Failed to write the value to av7100 register\n");
			retval = -EFAULT;
        	}
        } else if (av7100_globals->if_type == DSI_INTERFACE) {
        //DSI 
		retval = write_register_value(AV7100_DCS_WRITE_REGISTER_UCCMD, offset, value);
		if (retval) {
			dev_dbg(av7100dev,
				"Failed to write the value to av7100 register\n");
			retval = -EFAULT;
		}
	} else {
		retval = AV7100_INVALID_INTERFACE;
		dev_dbg(av7100dev, "Invalid interface type\n");
	}	

av7100_register_write_out:
	return retval;
}

/****************************************************************************
*	name	=	av7100_reg_stby_w
*	func	=	Write standby register of AV7100
*	input	=	u8 cpd, u8 stby, u8 mclkrng
*	output	=	None
*	return	=	retval
****************************************************************************/
int av7100_reg_stby_w(
		u8 stby, u8 mclkrng)
{
	int retval;
	u8 val;

	LOCK_AV7100_HW;

	/* Set register value */
	val =AV7100_STANDBY_STBY(stby) |
		AV7100_STANDBY_MCLKRNG(mclkrng);

	/* Write to register */
	retval = register_write_internal(AV7100_STANDBY, val);
	UNLOCK_AV7100_HW;
	return retval;
}

/****************************************************************************
*	name	=	av7100_reg_hdmi_5_volt_time_w
*	func	=	Write hdmi 5V register of AV7100
*	input	=	u8 denc_off, u8 hdmi_off, u8 on
*	output	=	None
*	return	=	retval
****************************************************************************/
int av7100_reg_hdmi_5_volt_time_w(u8 hdmi_off, u8 on)
{
	int retval;
	u8 val;

	LOCK_AV7100_HW;

    /* HDMI_5_VOLT_TIME Register
      |SU_ON| SU_ON| SU_ON| SU_OFF| SU_OFF| SU_OFF|R | R |
	 */
	 
    val = AV7100_HDMI_5_VOLT_TIME_SU_OFF_TIME(hdmi_off) |
			AV7100_HDMI_5_VOLT_TIME_ON_TIME(on);

	/* Write to register */
	retval = register_write_internal(AV7100_HDMI_5_VOLT_TIME, val);

	/* Set vars */

	av7100_globals->hdmi_off_time = hdmi_off;
	av7100_globals->on_time = on;

	UNLOCK_AV7100_HW;
	return retval;
}

/****************************************************************************
*	name	=	av7100_reg_stby_int_mask_w
*	func	=	Write standby interrupt mask register of AV7100
*	input	=	u8 hpdm, u8 cpdm, u8 stbygpiocfg, u8 ipol
*	output	=	None
*	return	=	retval
****************************************************************************/
int av7100_reg_stby_int_mask_w(
		u8 hpdm, u8 stbygpiocfg, u8 ipol)
{
	int retval;
	u8 val;

	LOCK_AV7100_HW;

	/* Set register value */
	val = AV7100_STANDBY_INTERRUPT_MASK_HPDM(hpdm) |
		AV7100_STANDBY_INTERRUPT_MASK_STBYGPIOCFG(stbygpiocfg) |
		AV7100_STANDBY_INTERRUPT_MASK_IPOL(ipol);

	/* Write to register */
	retval = register_write_internal(AV7100_STANDBY_INTERRUPT_MASK, val);

	av7100_globals->hpdm = hpdm;

	UNLOCK_AV7100_HW;
	return retval;
}

/****************************************************************************
*	name	=	av7100_reg_stby_pend_int_w
*	func	=	Write standby pending interrupt register of AV7100
*	input	=	u8 hpdi, u8 cpdi, u8 oni
*	output	=	None
*	return	=	retval
****************************************************************************/
int av7100_reg_stby_pend_int_w(
		u8 hpdi, u8 oni)
{
	int retval;
	u8 val;

	LOCK_AV7100_HW;

	/* Set register value */
	val = AV7100_STANDBY_PENDING_INTERRUPT_HPDI(hpdi) |
		AV7100_STANDBY_PENDING_INTERRUPT_ONI(oni);

	/* Write to register */
	retval = register_write_internal(AV7100_STANDBY_PENDING_INTERRUPT, val);

	UNLOCK_AV7100_HW;
	return retval;
}

/****************************************************************************
*	name	=	av7100_reg_gen_int_mask_w
*	func	=	Write general interrupt mask register of AV7100
*	input	=	u8 eocm, u8 vsim, u8 vsom, u8 cecm, u8 hdcpm, 
* 			u8 uovbm, u8 tem
*	output	=	None
*	return	=	retval
****************************************************************************/
int av7100_reg_gen_int_mask_w(
		u8 eocm, u8 vsim, u8 vsom, u8 cecm, u8 hdcpm, u8 uovbm)
{
	int retval;
	u8 val;

	LOCK_AV7100_HW;

	/* Set register value */
	val = AV7100_GENERAL_INTERRUPT_MASK_EOCM(eocm) |
		AV7100_GENERAL_INTERRUPT_MASK_VSIM(vsim) |
		AV7100_GENERAL_INTERRUPT_MASK_VSOM(vsom) |
		AV7100_GENERAL_INTERRUPT_MASK_CECM(cecm) |
		AV7100_GENERAL_INTERRUPT_MASK_HDCPM(hdcpm) |
		AV7100_GENERAL_INTERRUPT_MASK_UOVBM(uovbm);

	/* Write to register */
	retval = register_write_internal(AV7100_GENERAL_INTERRUPT_MASK,	val);

	UNLOCK_AV7100_HW;
	return retval;
}

/****************************************************************************
*	name	=	av7100_reg_gen_int_w
*	func	=	Write general interrupt mask register of AV7100
*	input	=	u8 eoci, u8 vsii, u8 vsoi, u8 ceci, u8 hdcpi, 
* 			u8 uovbi
*	output	=	None
*	return	=	retval
****************************************************************************/
int av7100_reg_gen_int_w(
		u8 eoci, u8 vsii, u8 vsoi, u8 ceci,	u8 hdcpi, u8 uovbi)
{
	int retval;
	u8 val;

	LOCK_AV7100_HW;

	/* Set register value */
	val = AV7100_GENERAL_INTERRUPT_EOCI(eoci) |
		AV7100_GENERAL_INTERRUPT_VSII(vsii) |
		AV7100_GENERAL_INTERRUPT_VSOI(vsoi) |
		AV7100_GENERAL_INTERRUPT_CECI(ceci) |
		AV7100_GENERAL_INTERRUPT_HDCPI(hdcpi) |
		AV7100_GENERAL_INTERRUPT_UOVBI(uovbi);

	/* Write to register */
	retval = register_write_internal(AV7100_GENERAL_INTERRUPT, val);
	UNLOCK_AV7100_HW;
	return retval;
}

/****************************************************************************
*	name	=	av7100_reg_gpio_conf_w
*	func	=	Write general purpose input output configuration register 
*			of AV7100
*	input	=	u8 dat3dir, u8 dat3val, u8 dat2dir, u8 dat2val,	
*			u8 dat1dir, u8 dat1val, u8 ucdbg
*	output	=	None
*	return	=	retval
****************************************************************************/
int av7100_reg_gpio_conf_w(
		u8 dat3dir, u8 dat3val, u8 dat2dir, u8 dat2val,	u8 dat1dir,
		u8 dat1val, u8 ucdbg)
{
	int retval;
	u8 val;

	LOCK_AV7100_HW;

	/* Set register value */
	val = AV7100_GPIO_CONFIGURATION_DAT3DIR(dat3dir) |
		AV7100_GPIO_CONFIGURATION_DAT3VAL(dat3val) |
		AV7100_GPIO_CONFIGURATION_DAT2DIR(dat2dir) |
		AV7100_GPIO_CONFIGURATION_DAT2VAL(dat2val) |
		AV7100_GPIO_CONFIGURATION_DAT1DIR(dat1dir) |
		AV7100_GPIO_CONFIGURATION_DAT1VAL(dat1val) |
		AV7100_GPIO_CONFIGURATION_UCDBG(ucdbg);

	/* Write to register */
	retval = register_write_internal(AV7100_GPIO_CONFIGURATION, val);
	UNLOCK_AV7100_HW;
	return retval;
}

/****************************************************************************
*	name	=	av7100_reg_gen_ctrl_w
*	func	=	Write general control register of AV7100
*	input	=	u8 fdl, u8 hld, u8 wa, u8 ra
*	output	=	None
*	return	=	retval
****************************************************************************/
int av7100_reg_gen_ctrl_w(
		u8 fdl, u8 hld, u8 wa, u8 ra)
{
	int retval;
	u8 val;

	LOCK_AV7100_HW;

	/* Set register value */
	val = AV7100_GENERAL_CONTROL_FDL(fdl) |
		AV7100_GENERAL_CONTROL_HLD(hld) |
		AV7100_GENERAL_CONTROL_WA(wa) |
		AV7100_GENERAL_CONTROL_RA(ra);

	/* Write to register */
	retval = register_write_internal(AV7100_GENERAL_CONTROL, val);
	UNLOCK_AV7100_HW;
	return retval;
}

/****************************************************************************
*	name	=	av7100_reg_fw_dl_entry_w
*	func	=	Write firmware download entry register of AV7100
*	input	=	u8 mbyte_code_entry
*	output	=	None
*	return	=	retval
****************************************************************************/
int av7100_reg_fw_dl_entry_w(
	u8 mbyte_code_entry)
{
	int retval;
	u8 val;

	LOCK_AV7100_HW;

	/* Set register value */
	val = AV7100_FIRMWARE_DOWNLOAD_ENTRY_MBYTE_CODE_ENTRY(
		mbyte_code_entry);

	/* Write to register */
	retval = register_write_internal(AV7100_FIRMWARE_DOWNLOAD_ENTRY, val);
	UNLOCK_AV7100_HW;
	return retval;
}

/****************************************************************************
*	name	=	av7100_reg_w
*	func	=	Write general register of AV7100
*	input	=	u8 offset, u8 value
*	output	=	None
*	return	=	retval, -EFAULT
****************************************************************************/
int av7100_reg_w(
		u8 offset, u8 value)
{
	int retval = 0;
	struct i2c_client *i2c;

	LOCK_AV7100_HW;

	if (!av7100_config) {
		retval = AV7100_FAIL;
		goto av7100_reg_w_out;
	}

	i2c = av7100_config->client;

	/* Write to register */
	retval = write_single_byte(i2c, offset, value);
	if (retval) {
		dev_dbg(av7100dev,
			"Failed to write the value to av7100 register\n");
		retval = -EFAULT;
	}

av7100_reg_w_out:
	UNLOCK_AV7100_HW;
	return retval;
}

/****************************************************************************
*	name	=	register_read_internal
*	func	=	Read values from internal registers of AV7100
*	input	=	u8 offset, u8 *value
*	output	=	None
*	return	=	retval, AV7100_FAIL, -EFAULT
****************************************************************************/
int register_read_internal(u8 offset, u8 *value)
{
	int retval = 0;
	struct i2c_client *i2c;

	if (!av7100_config) {
		dev_err(av7100dev,"register_read_internal function 1\n");
		retval = AV7100_FAIL;
		goto av7100_register_read_out;
	}

	i2c = av7100_config->client;

	/* Read from register */
	if (av7100_globals->if_type == I2C_INTERFACE) {
		retval = read_single_byte(i2c, offset, value);
		if (retval)	{
			dev_dbg(av7100dev,
				"Failed to read the value from av7100 register\n");
			dev_err(av7100dev,
				"register_read_internal Failed to read the value from av7100 register\n");
			retval = -EFAULT;
			goto av7100_register_read_out;
		}
	} else if (av7100_globals->if_type == DSI_INTERFACE) {
		retval = read_register_value(AV7100_DCS_READ_REGISTER, offset, value);
		if (retval)	{
			dev_dbg(av7100dev,
				"Failed to read the value from av7100 register\n");
			retval = -EFAULT;
			goto av7100_register_read_out;
		}
	} else {
		retval = AV7100_INVALID_INTERFACE;
		dev_dbg(av7100dev, "Invalid interface type\n");
	}	

av7100_register_read_out:
	return retval;
}

/****************************************************************************
*	name	=	av7100_reg_stby_r
*	func	=	Read standby register of AV7100
*	input	=	u8 *cpd, u8 *stby, u8 *hpds, u8 *cpds, u8 *mclkrng
*	output	=	None
*	return	=	retval
****************************************************************************/
int av7100_reg_stby_r(
      u8 *stby, u8 *hpds,u8 *mclkrng)
{
	int retval;
	u8 val;

	LOCK_AV7100_HW;

	/* Read from register */
	retval = register_read_internal(AV7100_STANDBY, &val);

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

/****************************************************************************
*	name	=	av7100_reg_hdmi_5_volt_time_r
*	func	=	Read hdmi 5V register of AV7100
*	input	=	u8 *denc_off, u8 *hdmi_off_time, u8 *on_time
*	output	=	None
*	return	=	retval
****************************************************************************/
int av7100_reg_hdmi_5_volt_time_r(
		u8 *hdmi_off_time, u8 *on_time)
{
	int retval;
	u8 val;

	LOCK_AV7100_HW;

	/* Read from register */
	retval = register_read_internal(AV7100_HDMI_5_VOLT_TIME, &val);

	/* Set return params */
	if (chip_version == 1) {
		if (hdmi_off_time)
			*hdmi_off_time =
				AV7100_HDMI_5_VOLT_TIME_OFF_TIME_GET(val);
	} else {
		if (hdmi_off_time)
			*hdmi_off_time =
				AV7100_HDMI_5_VOLT_TIME_SU_OFF_TIME_GET(val);
	}

	if (on_time)
		*on_time = AV7100_HDMI_5_VOLT_TIME_ON_TIME_GET(val);

	UNLOCK_AV7100_HW;
	return retval;
}

/****************************************************************************
*	name	=	av7100_reg_stby_int_mask_r
*	func	=	Read standby interrupt mask register of AV7100
*	input	=	u8 *hpdm, u8 *cpdm, u8 *stbygpiocfg, u8 *ipol
*	output	=	None
*	return	=	retval
****************************************************************************/
int av7100_reg_stby_int_mask_r(
		u8 *hpdm,u8 *stbygpiocfg, u8 *ipol)
{
	int retval;
	u8 val;

	LOCK_AV7100_HW;

	/* Read from register */
	retval = register_read_internal(AV7100_STANDBY_INTERRUPT_MASK, &val);

	/* Set return params */
	if (hpdm)
		*hpdm = AV7100_STANDBY_INTERRUPT_MASK_HPDM_GET(val);
	if (stbygpiocfg)
		*stbygpiocfg =
			AV7100_STANDBY_INTERRUPT_MASK_STBYGPIOCFG_GET(val);
	if (ipol)
		*ipol = AV7100_STANDBY_INTERRUPT_MASK_IPOL_GET(val);

	UNLOCK_AV7100_HW;
	return retval;
}

/****************************************************************************
*	name	=	av7100_reg_stby_pend_int_w
*	func	=	Read standby pending interrupt register of AV7100
*	input	=	u8 *hpdi, u8 *cpdi, u8 *oni, u8 *sid
*	output	=	None
*	return	=	retval
****************************************************************************/
int av7100_reg_stby_pend_int_r(
		u8 *hpdi,u8 *oni, u8 *sid)
{
	int retval;
	u8 val;

	LOCK_AV7100_HW;

	/* Read from register */
	retval = register_read_internal(AV7100_STANDBY_PENDING_INTERRUPT,
			&val);

	/* Set return params */
	if (hpdi)
		*hpdi = AV7100_STANDBY_PENDING_INTERRUPT_HPDI_GET(val);
	if (oni)
		*oni = AV7100_STANDBY_PENDING_INTERRUPT_ONI_GET(val);
	if (sid)
		*sid = AV7100_STANDBY_PENDING_INTERRUPT_SID_GET(val);

	UNLOCK_AV7100_HW;
	return retval;
}

/****************************************************************************
*	name	=	av7100_reg_gen_int_mask_r
*	func	=	Read general interrupt mask register of AV7100
*	input	=	u8 *eocm, u8 *vsim, u8 *vsom, u8 *cecm, u8 *hdcpm, 
* 			u8 *uovbm, u8 *tem
*	output	=	None
*	return	=	retval
****************************************************************************/
int av7100_reg_gen_int_mask_r(
		u8 *eocm,
		u8 *vsim,
		u8 *vsom,
		u8 *cecm,
		u8 *hdcpm,
		u8 *uovbm
		)
{
	int retval;
	u8 val;

	LOCK_AV7100_HW;

	/* Read from register */
	retval = register_read_internal(AV7100_GENERAL_INTERRUPT_MASK, &val);

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

	UNLOCK_AV7100_HW;
	return retval;
}

/****************************************************************************
*	name	=	av7100_reg_gen_int_r
*	func	=	Read general interrupt mask register of AV7100
*	input	=	u8 *eoci, u8 *vsii, u8 *vsoi, u8 *ceci,	
* 			u8 *hdcpi, u8 *uovbi
*	output	=	None
*	return	=	retval
****************************************************************************/
int av7100_reg_gen_int_r(
		u8 *eoci,
		u8 *vsii,
		u8 *vsoi,
		u8 *ceci,
		u8 *hdcpi,
		u8 *uovbi
		)
{
	int retval;
	u8 val;

	LOCK_AV7100_HW;

	/* Read from register */
	retval = register_read_internal(AV7100_GENERAL_INTERRUPT, &val);

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

	UNLOCK_AV7100_HW;
	return retval;
}

/****************************************************************************
*	name	=	av7100_reg_gen_status_r
*	func	=	Read general status mask register of AV7100
*	input	=	u8 *cecrec,u8 *cectrx,u8 *uc,u8 *onuvb,u8 *hdcps
*	output	=	None
*	return	=	retval
****************************************************************************/
int av7100_reg_gen_status_r(
		u8 *cecrec,
		u8 *cectrx,
		u8 *uc,
		u8 *onuvb,
		u8 *hdcps)
{
	int retval;
	u8 val;

	LOCK_AV7100_HW;

	/* Read from register */
	retval = register_read_internal(AV7100_GENERAL_STATUS, &val);

	/* Set return params */
	if (cecrec)
		*cecrec	= AV7100_GENERAL_STATUS_CECREC_GET(val);
	if (cectrx)
		*cectrx	= AV7100_GENERAL_STATUS_CECTRX_GET(val);
	if (uc)
		*uc = AV7100_GENERAL_STATUS_UC_GET(val);
	if (onuvb)
		*onuvb = AV7100_GENERAL_STATUS_ONUVB_GET(val);
	if (hdcps)
		*hdcps = AV7100_GENERAL_STATUS_HDCPS_GET(val);

	UNLOCK_AV7100_HW;
	return retval;
}

/****************************************************************************
*	name	=	av7100_reg_gpio_conf_r
*	func	=	Read general purpose input output configuration 
* 			register of AV7100
*	input	=	u8 *dat3dir, u8 *dat3val, u8 *dat2dir, u8 *dat2val,	
*			u8 *dat1dir, u8 *dat1val, u8 *ucdbg
*	output	=	None
*	return	=	retval
****************************************************************************/
int av7100_reg_gpio_conf_r(
		u8 *dat3dir,
		u8 *dat3val,
		u8 *dat2dir,
		u8 *dat2val,
		u8 *dat1dir,
		u8 *dat1val,
		u8 *ucdbg)
{
	int retval;
	u8 val;

	LOCK_AV7100_HW;

	/* Read from register */
	retval = register_read_internal(AV7100_GPIO_CONFIGURATION, &val);

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
	if (ucdbg)
		*ucdbg = AV7100_GPIO_CONFIGURATION_UCDBG_GET(val);

	UNLOCK_AV7100_HW;
	return retval;
}

/****************************************************************************
*	name	=	av7100_reg_gen_ctrl_r
*	func	=	Read general control register of AV7100
*	input	=	u8 *fdl, u8 *hld, u8 *wa, u8 *ra
*	output	=	None
*	return	=	retval
****************************************************************************/
int av7100_reg_gen_ctrl_r(
		u8 *fdl,
		u8 *hld,
		u8 *wa,
		u8 *ra)
{
	int retval;
	u8 val;

	LOCK_AV7100_HW;

	/* Read from register */
	retval = register_read_internal(AV7100_GENERAL_CONTROL, &val);
	/* Set return params */
	if (fdl)
		*fdl = AV7100_GENERAL_CONTROL_FDL_GET(val);
	if (hld)
		*hld = AV7100_GENERAL_CONTROL_HLD_GET(val);
	if (wa)
		*wa = AV7100_GENERAL_CONTROL_WA_GET(val);
	if (ra)
		*ra = AV7100_GENERAL_CONTROL_RA_GET(val);

	UNLOCK_AV7100_HW;
	return retval;
}

/****************************************************************************
*	name	=	av7100_reg_fw_dl_entry_r
*	func	=	Read firmware download entry register of AV7100
*	input	=	u8 *mbyte_code_entry
*	output	=	None
*	return	=	retval
****************************************************************************/
int av7100_reg_fw_dl_entry_r(
	u8 *mbyte_code_entry)
{
	int retval;
	u8 val;

	LOCK_AV7100_HW;

	/* Read from register */
	retval = register_read_internal(AV7100_FIRMWARE_DOWNLOAD_ENTRY,	&val);

	/* Set return params */
	if (mbyte_code_entry)
		*mbyte_code_entry =
		AV7100_FIRMWARE_DOWNLOAD_ENTRY_MBYTE_CODE_ENTRY_GET(val);

	UNLOCK_AV7100_HW;
	return retval;
}

/****************************************************************************
*	name	=	av7100_reg_r
*	func	=	Read general register of AV7100
*	input	=	u8 offset, u8 *value
*	output	=	None
*	return	=	retval, AV7100-fail, -EFAULT
****************************************************************************/
int av7100_reg_r(
		u8 offset,
		u8 *value)
{
	int retval = 0;
	struct i2c_client *i2c;

	LOCK_AV7100_HW;

	if (!av7100_config) {
		retval = AV7100_FAIL;
		goto av7100_register_read_out;
	}

	i2c = av7100_config->client;

	/* Read from register */
	retval = read_single_byte(i2c, offset, value);
	if (retval)	{
		dev_dbg(av7100dev,
			"Failed to read the value from av7100 register\n");
		retval = -EFAULT;
		goto av7100_register_read_out;
	}

av7100_register_read_out:
	UNLOCK_AV7100_HW;
	return retval;
}

/****************************************************************************
*	name	=	av7100_conf_get
*	func	=	Read configurations data to the corresponding 
*			data struct depending on command type 
*	input	=	enum av7100_command_type command_type,
*			union av7100_configuration *config	
*	output	=	None
*	return	=	0, AV7100_FAIL
****************************************************************************/
int av7100_conf_get(enum av7100_command_type command_type,
	union av7100_configuration *config)
{
	if (!av7100_config || !config)
		return AV7100_FAIL;

	/* Put configuration data to the corresponding data struct depending
	 * on command type */
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

	case AV7100_COMMAND_VIDEO_SCALING_FORMAT:
		memcpy(&config->video_scaling_format,
			&av7100_config->hdmi_video_scaling_cmd,
			sizeof(struct av7100_video_scaling_format_cmd));
		break;

	case AV7100_COMMAND_COLORSPACECONVERSION:
		memcpy(&config->color_space_conversion_format,
			&av7100_config->hdmi_color_space_conversion_cmd,
			sizeof(struct
				av7100_color_space_conversion_format_cmd));
		break;

	case AV7100_COMMAND_CEC_MESSAGE_WRITE:
		memcpy(&config->cec_message_write_format,
			&av7100_config->hdmi_cec_message_write_cmd,
			sizeof(struct av7100_cec_message_write_format_cmd));
		break;

	case AV7100_COMMAND_CEC_MESSAGE_READ_BACK:
		memcpy(&config->cec_message_read_back_format,
			&av7100_config->hdmi_cec_message_read_back_cmd,
			sizeof(struct av7100_cec_message_read_back_format_cmd));
		break;

	case AV7100_COMMAND_DENC:
		memcpy(&config->denc_format, &av7100_config->hdmi_denc_cmd,
				sizeof(struct av7100_denc_format_cmd));
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
			sizeof(struct
				av7100_edid_section_readback_format_cmd));
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

/****************************************************************************
*	name	=	av7100_conf_prep
*	func	=	Copy configuration data to the corresponding 
*			data struct depending on command type 
*	input	=	enum av7100_command_type command_type,
* 			union av7100_configuration *config	
*	output	=	None
*	return	=	0, AV7100_FAIL
****************************************************************************/
int av7100_conf_prep(enum av7100_command_type command_type,
	union av7100_configuration *config)
{
	if (!av7100_config || !config)
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

	case AV7100_COMMAND_VIDEO_SCALING_FORMAT:
		memcpy(&av7100_config->hdmi_video_scaling_cmd,
			&config->video_scaling_format,
			sizeof(struct av7100_video_scaling_format_cmd));
		break;

	case AV7100_COMMAND_COLORSPACECONVERSION:
		memcpy(&av7100_config->hdmi_color_space_conversion_cmd,
			&config->color_space_conversion_format,
			sizeof(struct
				av7100_color_space_conversion_format_cmd));
		break;

	case AV7100_COMMAND_CEC_MESSAGE_WRITE:
		memcpy(&av7100_config->hdmi_cec_message_write_cmd,
			&config->cec_message_write_format,
			sizeof(struct av7100_cec_message_write_format_cmd));
		break;

	case AV7100_COMMAND_CEC_MESSAGE_READ_BACK:
		memcpy(&av7100_config->hdmi_cec_message_read_back_cmd,
			&config->cec_message_read_back_format,
			sizeof(struct av7100_cec_message_read_back_format_cmd));
		break;

	case AV7100_COMMAND_DENC:
		memcpy(&av7100_config->hdmi_denc_cmd, &config->denc_format,
				sizeof(struct av7100_denc_format_cmd));
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
			sizeof(struct
				av7100_edid_section_readback_format_cmd));
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
*	name	=	av7100_conf_w
*	func	=	Write configuration data to the corresponding 
*			data struct depending on command type 
*	input	=	enum av7100_command_type command_type,
			u8 *return_buffer_length,
			u8 *return_buffer, enum interface_type if_type	
*	output	=	None
*	return	=	retval, AV7100_FAIL
****************************************************************************/
int av7100_conf_w(enum av7100_command_type command_type,
	u8 *return_buffer_length,
	u8 *return_buffer, enum interface_type if_type)
{
	int retval = 0;
	u8 cmd_buffer[AV7100_COMMAND_MAX_LENGTH];
	u32 cmd_length = 0;
	struct i2c_client *i2c;

	if (return_buffer_length)
		*return_buffer_length = 0;

	if (!av7100_config)
		return AV7100_FAIL;

	i2c = av7100_config->client;

	memset(&cmd_buffer, 0x00, AV7100_COMMAND_MAX_LENGTH);

#define PRNK_MODE(_m) dev_err(av7100dev, "cmd: " #_m "\n");

	/* Fill the command buffer with configuration data */
	switch (command_type) {
	case AV7100_COMMAND_VIDEO_INPUT_FORMAT:
		PRNK_MODE(AV7100_COMMAND_VIDEO_INPUT_FORMAT);
		configuration_video_input_get(cmd_buffer, &cmd_length);
		break;

	case AV7100_COMMAND_AUDIO_INPUT_FORMAT:
		PRNK_MODE(AV7100_COMMAND_AUDIO_INPUT_FORMAT);
		configuration_audio_input_get(cmd_buffer, &cmd_length);
		break;

	case AV7100_COMMAND_VIDEO_OUTPUT_FORMAT:
		PRNK_MODE(AV7100_COMMAND_VIDEO_OUTPUT_FORMAT);
		configuration_video_output_get(cmd_buffer, &cmd_length);
		break;

	case AV7100_COMMAND_VIDEO_SCALING_FORMAT:
		PRNK_MODE(AV7100_COMMAND_VIDEO_SCALING_FORMAT);
		configuration_video_scaling_get(cmd_buffer,
			&cmd_length);
		break;

	case AV7100_COMMAND_COLORSPACECONVERSION:
		PRNK_MODE(AV7100_COMMAND_COLORSPACECONVERSION);
		configuration_colorspace_conversion_get(cmd_buffer,
			&cmd_length);
		break;

	case AV7100_COMMAND_CEC_MESSAGE_WRITE:
		PRNK_MODE(AV7100_COMMAND_CEC_MESSAGE_WRITE);
		configuration_cec_message_write_get(cmd_buffer,
			&cmd_length);
		break;

	case AV7100_COMMAND_CEC_MESSAGE_READ_BACK:
		PRNK_MODE(AV7100_COMMAND_CEC_MESSAGE_READ_BACK);
		configuration_cec_message_read_get(cmd_buffer,
			&cmd_length);
		break;

	case AV7100_COMMAND_DENC:
		PRNK_MODE(AV7100_COMMAND_DENC);
		configuration_denc_get(cmd_buffer, &cmd_length);
		break;

	case AV7100_COMMAND_HDMI:
		PRNK_MODE(AV7100_COMMAND_HDMI);
		configuration_hdmi_get(cmd_buffer, &cmd_length);
		break;

	case AV7100_COMMAND_HDCP_SENDKEY:
		PRNK_MODE(AV7100_COMMAND_HDCP_SENDKEY);
		configuration_hdcp_sendkey_get(cmd_buffer, &cmd_length);
		break;

	case AV7100_COMMAND_HDCP_MANAGEMENT:
		PRNK_MODE(AV7100_COMMAND_HDCP_MANAGEMENT);
		configuration_hdcp_management_get(cmd_buffer,
			&cmd_length);
		break;

	case AV7100_COMMAND_INFOFRAMES:
		PRNK_MODE(AV7100_COMMAND_INFOFRAMES);
		configuration_infoframe_get(cmd_buffer, &cmd_length);
		break;

	case AV7100_COMMAND_EDID_SECTION_READBACK:
		PRNK_MODE(AV7100_COMMAND_EDID_SECTION_READBACK);
		av7100_edid_section_readback_get(cmd_buffer, &cmd_length);
		break;

	case AV7100_COMMAND_PATTERNGENERATOR:
		PRNK_MODE(AV7100_COMMAND_PATTERNGENERATOR);
		configuration_pattern_generator_get(cmd_buffer,
			&cmd_length);
		break;

	case AV7100_COMMAND_FUSE_AES_KEY:
		PRNK_MODE(AV7100_COMMAND_FUSE_AES_KEY);
		configuration_fuse_aes_key_get(cmd_buffer, &cmd_length);
		break;

	default:
		dev_dbg(av7100dev, "Invalid command type\n");
		retval = AV7100_INVALID_COMMAND;
		break;
	}

	LOCK_AV7100_HW;

	if (if_type == I2C_INTERFACE) {
		/* Write the command buffer */
		retval = write_multi_byte(i2c,
			AV7100_COMMAND_OFFSET + 1, cmd_buffer, cmd_length);
		if (retval) {
			dev_err(av7100dev,"write_multi_byte fails in av7100_conf_w 1\n");
			UNLOCK_AV7100_HW;
			return retval;
		}

		/* Write the command */
		retval = write_single_byte(i2c, AV7100_COMMAND_OFFSET,
			command_type);
		if (retval) {
			dev_err(av7100dev,"write_single_byte fails in av7100_conf_w \n");
			UNLOCK_AV7100_HW;
			return retval;
		}

		mdelay(100);

		retval = get_command_return_data(i2c, command_type, cmd_buffer,
			return_buffer_length, return_buffer);
	} else if (if_type == DSI_INTERFACE) {
		int cnt = 0;
	
		/* Write the command buffer */
		retval = write_uc_command(AV7100_DCS_WRITE_UCCMD, cmd_buffer, cmd_length);
		if (retval) {
			UNLOCK_AV7100_HW;
			return retval;
		}

		/* Write the command */    
		retval = exec_uc_command(AV7100_DCS_EXEC_UCCMD, command_type);
		if (retval) {
			UNLOCK_AV7100_HW;
			return retval;
		}

		mdelay(100);

		retval = get_command_return_data(i2c, command_type, cmd_buffer,
			return_buffer_length, return_buffer);

	} else {
		retval = AV7100_INVALID_INTERFACE;
		dev_dbg(av7100dev, "Invalid command type\n");
	}

	if (command_type == AV7100_COMMAND_HDMI) {
		g_av7100_status.hdmi_on = ((av7100_config->hdmi_cmd.
			hdmi_mode == AV7100_HDMI_ON) &&
			(av7100_config->hdmi_cmd.hdmi_format == AV7100_HDMI));
		if(av7100_config->hdmi_cmd.hdmi_mode == AV7100_HDMI_ON)
		{
			av7100_set_state(AV7100_OPMODE_IDLE);
		} else {
		    av7100_set_state(AV7100_OPMODE_INIT);
		}
	}

	UNLOCK_AV7100_HW;
	return retval;
}

/****************************************************************************
*	name	=	av7100_conf_w_raw
*	func	=	Write command buffer using write_multi_byte function 
* 			and command using write_single_byte function
*	input	=	enum av7100_command_type command_type,
*			u8 buffer_length,u8 *buffer,
*			u8 *return_buffer_length,u8 *return_buffer
*	output	=	None
*	return	=	retval
****************************************************************************/
int av7100_conf_w_raw(enum av7100_command_type command_type,
	u8 buffer_length,
	u8 *buffer,
	u8 *return_buffer_length,
	u8 *return_buffer)
{
	int retval = 0;
	struct i2c_client *i2c;

	LOCK_AV7100_HW;

	if (return_buffer_length)
		*return_buffer_length = 0;

	if (!av7100_config) {
		retval = AV7100_FAIL;
		goto av7100_conf_w_raw_out;
	}

	i2c = av7100_config->client;

	/* Write the command buffer */
	retval = write_multi_byte(i2c,
		AV7100_COMMAND_OFFSET + 1, buffer, buffer_length);
	if (retval)
		goto av7100_conf_w_raw_out;

	/* Write the command */
	retval = write_single_byte(i2c, AV7100_COMMAND_OFFSET,
		command_type);
	if (retval)
		goto av7100_conf_w_raw_out;

	mdelay(100);

	retval = get_command_return_data(i2c, command_type, buffer,
		return_buffer_length, return_buffer);

av7100_conf_w_raw_out:
	UNLOCK_AV7100_HW;
	return retval;
}

/****************************************************************************
*	name	=	av7100_status_get
*	func	=	Returns status of AV7100
*	input	=	void
*	output	=	None
*	return	=	g_av7100_status
****************************************************************************/
struct av7100_status av7100_status_get(void)
{
	return g_av7100_status;
}

/****************************************************************************
*	name	=	av7100_video_output_format_get
*	func	=	Returns index of CEA_VESA format based on 
* 			xresolution,yresloution,htotal,vtotal pixelclock 
* 			and interlaced
*	input	=	int xres,int yres,int htot,int vtot,int pixelclk,
*			bool interlaced
*	output	=	None
*	return	=	index enum av7100_output_CEA_VESA
****************************************************************************/
enum av7100_output_CEA_VESA av7100_video_output_format_get(int xres,
	int yres,
	int htot,
	int vtot,
	int pixelclk,
	bool interlaced)
{
	enum av7100_output_CEA_VESA index = 1;
	int yres_div = !interlaced ? 1 : 2;
	int hres_div = 1;
	long freq1;
	long freq2;

	/*
	* 720_576_I need a divider for hact and htot since
	* these params need to be twice as large as expected in av7100_all_cea,
	* which is used as input parameter to video input config.
	*/
	if ((xres == 720) && (yres == 576) && (interlaced == true))
		hres_div = 2;

	freq1 = 1000000 / htot * 1000000 / vtot / pixelclk + 1;
	while (index < sizeof(av7100_all_cea)/sizeof(struct av7100_cea)) {
		freq2 = av7100_all_cea[index].frequence /
			av7100_all_cea[index].htotale /
			av7100_all_cea[index].vtotale;

		dev_dbg(av7100dev, "freq1:%ld freq2:%ld\n", freq1, freq2);
		if ((xres == av7100_all_cea[index].hactive / hres_div) &&
			(yres == av7100_all_cea[index].vactive * yres_div) &&
			(htot == av7100_all_cea[index].htotale / hres_div) &&
			(vtot == av7100_all_cea[index].vtotale) &&
			(abs(freq1 - freq2) < 2)) {
			goto av7100_video_output_format_get_out;
		}
		index++;
	}

av7100_video_output_format_get_out:
	dev_dbg(av7100dev, "av7100_video_output_format_get %d %d %d %d %d\n",
		xres, yres, htot, vtot, index);
	return index;
}

/****************************************************************************
*	name	=	av7100_hdmi_event_cb_set
*	func	=	Sets callback function for hdmi event
*	input	=	void (*hdmi_ev_cb)(enum av7100_hdmi_event)
*	output	=	None
*	return	=	void
****************************************************************************/
void av7100_hdmi_event_cb_set(void (*hdmi_ev_cb)(enum av7100_hdmi_event))
{
	if (av7100_globals)
		av7100_globals->hdmi_ev_cb = hdmi_ev_cb;
}

/****************************************************************************
*	name	=	av7100_ver_get
*	func	=	Returns chip version of AV7100
*	input	=	void
*	output	=	None
*	return	=	ret
****************************************************************************/
u8 av7100_ver_get(void)
{
	u8 ret;

	LOCK_AV7100_HW;
	ret = chip_version;
	UNLOCK_AV7100_HW;

	return ret;
}

/****************************************************************************
*	name	=	av7100_open
*	func	=	Open file operation function for AV7100
*	input	=	struct inode *inode, struct file *filp
*	output	=	None
*	return	=	0
****************************************************************************/
static int av7100_open(struct inode *inode, struct file *filp)
{
	dev_dbg(av7100dev, "av7100_open is called\n");
	return 0;
}

/****************************************************************************
*	name	=	av7100_release
*	func	=	Release file operation function for AV7100
*	input	=	struct inode *inode, struct file *filp
*	output	=	None
*	return	=	0
****************************************************************************/
static int av7100_release(struct inode *inode, struct file *filp)
{
	dev_dbg(av7100dev, "av7100_release is called\n");
	return 0;
}

/****************************************************************************
*	name	=	av7100_ioctl
*	func	=	IOCTL file operation function for AV7100
*	input	=	struct inode *inode, struct file *file,
			unsigned int cmd, unsigned long arg
*	output	=	None
*	return	=	0
****************************************************************************/
static int av7100_ioctl(struct inode *inode, struct file *file,
				unsigned int cmd, unsigned long arg)
{
	return 0;
}

/****************************************************************************
*	name	=	av7100_probe
*	func	=	Probe input device,requests ports for AV7100 and 
			configures AV7100
*	input	=	struct i2c_client *i2cClient,
			const struct i2c_device_id *id
*	output	=	None
*	return	=	ret
****************************************************************************/
static int __devinit av7100_probe(struct i2c_client *i2cClient,
	const struct i2c_device_id *id)
{
	int ret = 0;
	struct av7100_platform_data *pdata = i2cClient->dev.platform_data;

	av7100dev = &i2cClient->dev;

	dev_dbg(av7100dev, "%s\n", __func__);
	g_av7100_status.av7100_state = AV7100_OPMODE_SHUTDOWN;
	g_av7100_status.av7100_plugin_status = AV7100_PLUGIN_NONE;
	g_av7100_status.hdmi_on = false;

	ret = av7100_config_init();
	if (ret) {
		dev_info(av7100dev, "av7100_config_init failed\n");
		goto err;
	}

	ret = av7100_globals_init();
	if (ret) {
		dev_info(av7100dev, "av7100_globals_init failed\n");
		goto err;
	} 
	
	
	/* Request GPIO functions
	 * VCLK5 AV7100 Master Clock - GPIO_PORT219
	 * PWRDWN - GPIO_PORT8
	 * INT - GPIO_PORT15 */
	 
	/* MIPI HDMI */
	/* Port request for Master clock for AV7100 */
	ret = gpio_request(GPIO_PORT219, NULL); 
	if (ret)
		goto err_gpio_request_mclk;

	/* Port request for POWERDOWN of AV7100 */
	ret = gpio_request(GPIO_PORT8, NULL); 		
	if (ret)
		goto err_gpio_request_pwdown;

	/* Direction request for POWERDOWN Port */
	ret = gpio_direction_output(GPIO_PORT8, 0);
	if (ret)
		goto err_gpio_request_pwdown;

	/* Port request for INTERRUPT PIN of AV7100 */	

	ret = gpio_request(GPIO_PORT15, NULL);		
	if (ret)
		goto err_gpio_request_int;
	
	/* Direction request for INTERRUPT Port */
	ret = gpio_direction_input(GPIO_PORT15);
	if (ret)
		goto err_gpio_request_int;


	if (ret)
		goto err;



	if (!i2c_check_functionality(i2cClient->adapter,
		I2C_FUNC_SMBUS_BYTE_DATA |
		I2C_FUNC_SMBUS_READ_WORD_DATA)) {
		ret = -ENODEV;
		dev_err(av7100dev, "av7100 i2c_check_functionality failed\n");
		goto err;
	}
	init_waitqueue_head(&av7100_event);

	av7100_config->client = i2cClient;
	av7100_config->id = (struct i2c_device_id *) id;
	i2c_set_clientdata(i2cClient, av7100_config);

	kthread_run(av7100_thread, NULL, "av7100_thread");

	ret = request_irq(gpio_to_irq(GPIO_PORT15), av7100_intr_handler,
			IRQF_TRIGGER_RISING, "av7100", av7100_config);
	if (ret) {
		dev_err(av7100dev, "av7100_hw request_irq %d failed %d\n",
			GPIO_PORT15, ret);
		gpio_free(GPIO_PORT15);
		goto err;
	}

	
	/* Put av7100 in scan mode */

	if (av7100_powerup() != 0) {
                dev_err(av7100dev,"av7100 failed to power up \n");
		return -EIO;
	} 
        av7100_hdmi_event_cb_set(hdmi_event);
        av7100_enable_interrupt();

err:
	return ret;
err_gpio_request_mclk:
	av7100_globals_exit();

err_gpio_request_pwdown:
	gpio_free(GPIO_PORT219);

err_gpio_request_int:
	gpio_free(GPIO_PORT15);


}

/****************************************************************************
*	name	=	av7100_remove
*	func	=	Remove AV7100 input device
*	input	=	struct i2c_client *i2cClient,
*	output	=	None
*	return	=	0
****************************************************************************/
static int __devexit av7100_remove(struct i2c_client *i2cClient)
{
	dev_dbg(av7100dev, "%s\n", __func__);

	/* Free the interrupt request for GPIO PORT 15*/
	free_irq(gpio_to_irq(GPIO_PORT15), av7100_intr_handler);	

	/* Free the Port for INTERRUPT */
	gpio_free(GPIO_PORT15);

	/* Free the Port for POWERDOWN */
	gpio_free(GPIO_PORT8);

	/* Free the Port for CLOCK */
	gpio_free(GPIO_PORT219);		

	av7100_config_exit();
	av7100_globals_exit();

	return 0;
}


#ifdef CONFIG_PM
/****************************************************************************
*	name	=	av7100_suspend
*	func	=	Suspends AV7100 when HDMI cable is not plugged in
*	input	=	struct i2c_client *i2cClient,pm_message_t message
*	output	=	None
*	return	=	0
****************************************************************************/
static int av7100_suspend(struct i2c_client *client, pm_message_t message)
{
	/* device shall be suspended only when HDMI is not connected */
	av7100_disable_interrupt();
	av7100_powerdown();
	return 0;
}

/****************************************************************************
*	name	=	av7100_resume
*	func	=	Resumes AV7100 when HDMI cable is plugged in
*	input	=	struct i2c_client *client
*	output	=	None
*	return	=	0
****************************************************************************/
static int av7100_resume(struct i2c_client *client)
{
	/* device shall be suspended only when HDMI is not connected */
	av7100_powerup();
	av7100_enable_interrupt();
	return 0;
}
#else
# define av7100_suspend NULL
# define av7100_resume  NULL
#endif

/****************************************************************************
*	name	=	av7100_init
*	func	=	Register AV7100 device and AV7100 driver 
*			for platform-level devices
*	input	=	None
*	output	=	None
*	return	=	ret
****************************************************************************/
int av7100_init(void)
{
	int ret;

	pr_debug("%s\n", __func__);

	ret = i2c_add_driver(&av7100_driver);
	if (ret) {
		pr_err("av7100 i2c_add_driver failed\n");
		goto av7100_init_err;
	}

	ret = misc_register(&av7100_miscdev);
	if (ret) {
		pr_err("av7100 misc_register failed\n");
		goto av7100_init_err;
	}

	hdmi_init();

	return ret;

av7100_init_err:
	return ret;
}
module_init(av7100_init);

/****************************************************************************
*	name	=	av7100_exit
*	func	=	Deregister AV7100 driver and AV7100 device
*			for platform-level devices
*	input	=	None
*	output	=	None
*	return	=	None
****************************************************************************/
void av7100_exit(void)
{
	pr_debug("%s\n", __func__);

	hdmi_exit();

	misc_deregister(&av7100_miscdev);
	i2c_del_driver(&av7100_driver);
}
module_exit(av7100_exit);

MODULE_AUTHOR("Per Persson <per.xb.persson@stericsson.com>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("ST-Ericsson hdmi display driver");
