/*
 * OmniVision OV5645 sensor driver
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 *modify it under the terms of the GNU General Public License as
 *published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 *kind, whether express or implied; without even the implied warranty
 *of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/videodev2.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/regulator/consumer.h>
#include <linux/clk.h>
#include <mach/r8a7373.h>
#include <linux/i2c.h>
#include <linux/log2.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <media/v4l2-subdev.h>
#include <media/v4l2-chip-ident.h>
#include <media/soc_camera.h>
#include <media/sh_mobile_csi2.h>
#include <linux/videodev2_brcm.h>
#include "ov5645.h"

#ifdef CONFIG_VIDEO_ADP1653
#include "adp1653.h"
#endif

#ifdef CONFIG_VIDEO_AS3643
#include "as3643.h"
#endif

#if defined(CONFIG_MACH_HAWAII_GARNET_C_W68TK) || \
	defined(CONFIG_MACH_HAWAII_GARNET_C_W81)
#define CONFIG_VIDEO_SGM3410	1
#endif

#ifdef CONFIG_VIDEO_SGM3410
#define TORCH_EN (10)
#define FLASH_EN (11)


#endif


#define FLASH_TIMEOUT_MS	500
struct timer_list flash_timer;

#define CAM_LED_ON				(1)
#define CAM_LED_OFF				(0)
#define CAM_LED_MODE_PRE			(1<<1)
#define CAM_FLASH_ENSET     (GPIO_PORT99)
#define CAM_FLASH_FLEN      (GPIO_PORT100)

typedef struct touch_area v4l2_touch_area;

/* #define OV5645_DEBUG */

#define iprintk(format, arg...)	\
	printk(KERN_INFO"[%s]: "format"\n", __func__, ##arg)


#define OV5645_FLASH_THRESHHOLD		32


int OV5645_power(struct device *dev, int power_on)
{
	struct clk *vclk1_clk, *vclk2_clk;
	int iRet;
	struct regulator *regulator;
	dev_dbg(dev, "%s(): power_on=%d\n", __func__, power_on);

	vclk1_clk = clk_get(NULL, "vclk1_clk");
	if (IS_ERR(vclk1_clk)) {
		dev_err(dev, "clk_get(vclk1_clk) failed\n");
		return -1;
	}

	vclk2_clk = clk_get(NULL, "vclk2_clk");
	if (IS_ERR(vclk2_clk)) {
		dev_err(dev, "clk_get(vclk2_clk) failed\n");
		return -1;
	}

	if (power_on) {
		printk(KERN_ALERT "%s PowerON\n", __func__);
		sh_csi2_power(dev, power_on);
		gpio_set_value(GPIO_PORT3, 0); /* CAM_PWR_EN Low */
		gpio_set_value(GPIO_PORT16, 0); /* CAM1_RST_N */
		gpio_set_value(GPIO_PORT91, 0); /* CAM1_STBY */
		gpio_set_value(GPIO_PORT20, 0); /* CAM0_RST_N */
		gpio_set_value(GPIO_PORT45, 0); /* CAM0_STBY */

		/* CAM_AVDD_2V8  On */
		regulator = regulator_get(NULL, "cam_sensor_a");
		if (IS_ERR(regulator))
			return -1;
		regulator_enable(regulator);
		regulator_put(regulator);

		mdelay(2);
		/* CAM_VDDIO_1V8 On */
		regulator = regulator_get(NULL, "cam_sensor_io");
		if (IS_ERR(regulator))
			return -1;
		regulator_enable(regulator);
		regulator_put(regulator);

		mdelay(2);
		gpio_set_value(GPIO_PORT91, 1); /* CAM1_STBY */
		mdelay(2);

		iRet = clk_set_rate(vclk1_clk,
		clk_round_rate(vclk1_clk, 24000000));
		if (0 != iRet) {
			dev_err(dev,
			"clk_set_rate(vclk1_clk) failed (ret=%d)\n",
				iRet);
		}

		iRet = clk_enable(vclk1_clk);
		if (0 != iRet) {
			dev_err(dev, "clk_enable(vclk1_clk) failed (ret=%d)\n",
				iRet);
		}
		mdelay(3);

		gpio_set_value(GPIO_PORT16, 1); /* CAM1_RST_N */
		mdelay(2);

		gpio_set_value(GPIO_PORT91, 0); /* CAM1_STBY */
		mdelay(2);

		/* CAM_CORE_1V2  On */
		gpio_set_value(GPIO_PORT3, 1);
		mdelay(1);

		gpio_set_value(GPIO_PORT45, 1); /* CAM0_STBY */
		mdelay(1);

		gpio_set_value(GPIO_PORT20, 1); /* CAM0_RST_N Hi */
		udelay(70);
		/* 1ms */

		/* 5M_AF_2V8 On */
		regulator = regulator_get(NULL, "cam_af");
		if (IS_ERR(regulator))
			return -1;
		regulator_enable(regulator);
		regulator_put(regulator);

		printk(KERN_ALERT "%s PowerON fin\n", __func__);
		} else {
		printk(KERN_ALERT "%s PowerOFF\n", __func__);

		gpio_set_value(GPIO_PORT20, 0); /* CAM0_RST_N */
		mdelay(1);

		clk_disable(vclk1_clk);

		gpio_set_value(GPIO_PORT45, 0); /* CAM0_STBY */
		mdelay(1);

		gpio_set_value(GPIO_PORT16, 0); /* CAM1_RST_N */
		mdelay(1);

		/* CAM_CORE_1V2  Off */
		gpio_set_value(GPIO_PORT3, 0);
		mdelay(1);

		/* CAM_VDDIO_1V8 Off */
		regulator = regulator_get(NULL, "cam_sensor_io");
		if (IS_ERR(regulator))
			return -1;
		regulator_disable(regulator);
		regulator_put(regulator);
		mdelay(1);

		/* CAM_AVDD_2V8  Off */
		regulator = regulator_get(NULL, "cam_sensor_a");
		if (IS_ERR(regulator))
			return -1;
		regulator_disable(regulator);
		regulator_put(regulator);
		mdelay(1);

		/* 5M_AF_2V8 Off */
		regulator = regulator_get(NULL, "cam_af");
		if (IS_ERR(regulator))
			return -1;
		regulator_disable(regulator);
		regulator_put(regulator);
		sh_csi2_power(dev, power_on);
		printk(KERN_ALERT "%s PowerOFF fin\n", __func__);
	}

	clk_put(vclk1_clk);
	clk_put(vclk2_clk);

	return 0;
}


/*extern int hawaii_camera_AF_power(int on);*/

/* OV5645 has only one fixed colorspace per pixelcode */
struct ov5645_datafmt {
	enum v4l2_mbus_pixelcode code;
	enum v4l2_colorspace colorspace;
};

struct ov5645_timing_cfg {
	u16 x_addr_start;
	u16 y_addr_start;
	u16 x_addr_end;
	u16 y_addr_end;
	u16 h_output_size;
	u16 v_output_size;
	u16 h_total_size;
	u16 v_total_size;
	u16 isp_h_offset;
	u16 isp_v_offset;
	u8 h_odd_ss_inc;
	u8 h_even_ss_inc;
	u8 v_odd_ss_inc;
	u8 v_even_ss_inc;
	u8 out_mode_sel;
	u8 sclk_dividers;
	u8 sys_mipi_clk;

};

static const struct ov5645_datafmt ov5645_fmts[] = {
	/*
	 * Order important: first natively supported,
	 *second supported with a GPIO extender
	 */
	{V4L2_MBUS_FMT_UYVY8_2X8, V4L2_COLORSPACE_JPEG},
	{V4L2_MBUS_FMT_YUYV8_2X8, V4L2_COLORSPACE_JPEG},
/*	{V4L2_MBUS_FMT_JPEG_1X8, V4L2_COLORSPACE_JPEG}, */

};

enum ov5645_size {
	OV5645_SIZE_QVGA,	/*  320 x 240 */
	OV5645_SIZE_VGA,	/*  640 x 480 */
	OV5645_SIZE_720P,
	OV5645_SIZE_1280x960,	/*  1280 x 960 (1.2M) */
	OV5645_SIZE_UXGA,	/*  1600 x 1200 (2M) */
	OV5645_SIZE_QXGA,	/*  2048 x 1536 (3M) */
	OV5645_SIZE_5MP,
	OV5645_SIZE_LAST,
	OV5645_SIZE_MAX
};

enum cam_running_mode {
	CAM_RUNNING_MODE_NOTREADY,
	CAM_RUNNING_MODE_PREVIEW,
	CAM_RUNNING_MODE_CAPTURE,
	CAM_RUNNING_MODE_CAPTURE_DONE,
	CAM_RUNNING_MODE_RECORDING,
};
enum cam_running_mode runmode;

static const struct v4l2_frmsize_discrete ov5645_frmsizes[OV5645_SIZE_LAST] = {
	{320, 240},
	{640, 480},
	{1280, 720},
	{1280, 960},
	{1600, 1200},
	{2048, 1536},
	{2560, 1920},
};

/* Scalers to map image resolutions into AF 80x60 virtual viewfinder */
static const struct ov5645_af_zone_scale af_zone_scale[OV5645_SIZE_LAST] = {
	{4, 4},
	{8, 8},
	{16, 12},
	{16, 16},
	{20, 20},
	{26, 26},
	{32, 32},
};

/* Find a data format by a pixel code in an array */
static int ov5645_find_datafmt(enum v4l2_mbus_pixelcode code)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(ov5645_fmts); i++)
		if (ov5645_fmts[i].code == code)
			break;

	/* If not found, select latest */
	if (i >= ARRAY_SIZE(ov5645_fmts))
		i = ARRAY_SIZE(ov5645_fmts) - 1;

	return i;
}

/* Find a frame size in an array */
static int ov5645_find_framesize(u32 width, u32 height)
{
	int i;

	for (i = 0; i < OV5645_SIZE_LAST; i++) {
		if ((ov5645_frmsizes[i].width >= width) &&
		    (ov5645_frmsizes[i].height >= height))
			break;
	}

	/* If not found, select biggest */
	if (i >= OV5645_SIZE_LAST)
		i = OV5645_SIZE_LAST - 1;

	return i;
}

struct ov5645 {
	struct v4l2_subdev subdev;
	struct v4l2_subdev_sensor_interface_parms *plat_parms;
	int i_size;
	int i_fmt;
	int brightness;
	int contrast;
	int colorlevel;
	int sharpness;
	int saturation;
	int antibanding;
	int whitebalance;
	int framerate;
	int focus_mode;
	/*
	 * focus_status = 1 focusing
	 * focus_status = 0 focus cancelled or not focusing
	 */
	atomic_t focus_status;

	/*
	 * touch_focus holds number of valid touch focus areas. 0 = none
	 */
	int touch_focus;
	v4l2_touch_area touch_area[OV5645_MAX_FOCUS_AREAS];
	short flashmode;
	short fireflash;
};

static int ov5645_set_flash_mode(int mode, struct i2c_client *client);

/*static int flash_gpio_strobe(int);*/

static struct ov5645 *to_ov5645(const struct i2c_client *client)
{
	return container_of(i2c_get_clientdata(client), struct ov5645, subdev);
}

static const struct ov5645_timing_cfg timing_cfg_yuv[OV5645_SIZE_LAST] = {
	[OV5645_SIZE_QVGA] = {
			      /*  Timing control  2624 x 1952 --> 2592 x 1944 */
			      .x_addr_start = 16,
			      .y_addr_start = 4,
			      .x_addr_end = 2607,
			      .y_addr_end = 1947,
			      /*  Output image size */
			      .h_output_size = 320,
			      .v_output_size = 240,
			      /*  ISP Windowing size 1296 x 972 -->
				*  * 1280 x 960 */
				.isp_h_offset = 8,
			      .isp_v_offset = 6,
			      /*  Total size (+blanking) */
			      .h_total_size = 2200,
			      .v_total_size = 1280,
			      /*  Sensor Read Binning Enabled */
			      .h_odd_ss_inc = 3,
			      .h_even_ss_inc = 1,
			      .v_odd_ss_inc = 3,
			      .v_even_ss_inc = 1,
#ifdef CONFIG_MACH_HAWAII_GARNET
#ifdef CONFIG_MACH_HAWAII_GARNET_C_A18
			      .out_mode_sel = 0x07,
#else
			      .out_mode_sel = 0x01,
#endif
#else
			      .out_mode_sel = 0x07,
#endif
			      .sclk_dividers = 0x01,
			      .sys_mipi_clk = 0x11,
			      },
	[OV5645_SIZE_VGA] = {
			     /*  Timing control  2624 x 1952 --> 2592 x 1944 */
			     .x_addr_start = 16,
			     .y_addr_start = 4,
			     .x_addr_end = 2607,
			     .y_addr_end = 1947,
			     /*  Output image size */
			     .h_output_size = 640,
			     .v_output_size = 480,
			     /*  ISP Windowing size  1296 x 972 -->
				*  *  1280 x 960 */
				.isp_h_offset = 8,
			     .isp_v_offset = 6,
			     /*  Total size (+blanking) */
			     .h_total_size = 2200,
			     .v_total_size = 1280,
			     /*  Sensor Read Binning Enabled */
			     .h_odd_ss_inc = 3,
			     .h_even_ss_inc = 1,
			     .v_odd_ss_inc = 3,
			     .v_even_ss_inc = 1,
#ifdef CONFIG_MACH_HAWAII_GARNET
#ifdef CONFIG_MACH_HAWAII_GARNET_C_A18
			     .out_mode_sel = 0x07,
#else
			     .out_mode_sel = 0x01,
#endif
#else
			     .out_mode_sel = 0x07,
#endif
			     .sclk_dividers = 0x01,
			     .sys_mipi_clk = 0x11,
			     },
	[OV5645_SIZE_720P] = {
			      /*  Timing control  2624 x 1952 --> 2592 x 1944 */
			      .x_addr_start = 16,
			      .y_addr_start = 4,
			      .x_addr_end = 2607,
			      .y_addr_end = 1947,
			      /*  Output image size */
			      .h_output_size = 1280,
			      .v_output_size = 720,
			      /*  ISP Windowing size  1296 x 972 -->
				 *   1280 x 960 */
			      .isp_h_offset = 8,
			      .isp_v_offset = 6,
			      /*  Total size (+blanking) */
			      .h_total_size = 2200,
			      .v_total_size = 1280,
			      /*  Sensor Read Binning Enabled */
			      .h_odd_ss_inc = 3,
			      .h_even_ss_inc = 1,
			      .v_odd_ss_inc = 3,
			      .v_even_ss_inc = 1,
#ifdef CONFIG_MACH_HAWAII_GARNET
#ifdef CONFIG_MACH_HAWAII_GARNET_C_A18
			      .out_mode_sel = 0x07,
#else
			      .out_mode_sel = 0x01,
#endif
#else
			      .out_mode_sel = 0x07,
#endif
			      .sclk_dividers = 0x01,
			      .sys_mipi_clk = 0x11,
			      },
	[OV5645_SIZE_1280x960] = {
				  /*  Timing control  2624 x 1952 -->
					*   2592 x 1944 */
				  .x_addr_start = 16,
				  .y_addr_start = 4,
				  .x_addr_end = 2607,
				  .y_addr_end = 1947,
				  /*  Output image size */
				  .h_output_size = 1280,
				  .v_output_size = 960,
				  /*  ISP Windowing size  1296 x 972 -->
					*   1280 x 960 */
				  .isp_h_offset = 8,
				  .isp_v_offset = 6,
				  /*  Total size (+blanking) */
				  .h_total_size = 2200,
				  .v_total_size = 1280,
				  /*  Sensor Read Binning Enabled */
				  .h_odd_ss_inc = 3,
				  .h_even_ss_inc = 1,
				  .v_odd_ss_inc = 3,
				  .v_even_ss_inc = 1,
#ifdef CONFIG_MACH_HAWAII_GARNET
#ifdef CONFIG_MACH_HAWAII_GARNET_C_A18
				  .out_mode_sel = 0x07,
#else
				  .out_mode_sel = 0x01,
#endif
#else
				  .out_mode_sel = 0x07,
#endif
				  .sclk_dividers = 0x01,
				  .sys_mipi_clk = 0x11,
				  },
	[OV5645_SIZE_UXGA] = {
			      /*  Timing control  2624 x 1952 --> 2592 x 1944 */
			      .x_addr_start = 16,
			      .y_addr_start = 4,
			      .x_addr_end = 2607,
			      .y_addr_end = 1947,
			      /*  Output image size */
			      .h_output_size = 1600,
			      .v_output_size = 1200,
			      /*  ISP Windowing size  2592 x 1944 -->
				*   2560 x 1920 */
			      .isp_h_offset = 16,
			      .isp_v_offset = 12,
			      /*  Total size (+blanking) */
			      .h_total_size = 2844,
			      .v_total_size = 1968,
			      /*  Sensor Read Binning Disabled */
			      .h_odd_ss_inc = 1,
			      .h_even_ss_inc = 1,
			      .v_odd_ss_inc = 1,
			      .v_even_ss_inc = 1,
#ifdef CONFIG_MACH_HAWAII_GARNET
#ifdef CONFIG_MACH_HAWAII_GARNET_C_A18
			      .out_mode_sel = 0x06,
#else
			      .out_mode_sel = 0x00,
#endif
#else
			      .out_mode_sel = 0x06,
#endif
			      .sclk_dividers = 0x02,
			      .sys_mipi_clk = 0x12,
			      },
	[OV5645_SIZE_QXGA] = {
			      /*  Timing control  2624 x 1952 --> 2592 x 1944 */
			      .x_addr_start = 16,
			      .y_addr_start = 4,
			      .x_addr_end = 2607,
			      .y_addr_end = 1947,
			      /*  Output image size */
			      .h_output_size = 2048,
			      .v_output_size = 1536,
			      /*  ISP Windowing size  2592 x 1944 -->
				*   2560 x 1920 */
			      .isp_h_offset = 16,
			      .isp_v_offset = 12,
			      /*  Total size (+blanking) */
			      .h_total_size = 2844,
			      .v_total_size = 1968,
			      /*  Sensor Read Binning Enabled */
			      .h_odd_ss_inc = 1,
			      .h_even_ss_inc = 1,
			      .v_odd_ss_inc = 1,
			      .v_even_ss_inc = 1,
#ifdef CONFIG_MACH_HAWAII_GARNET
#ifdef CONFIG_MACH_HAWAII_GARNET_C_A18
			      .out_mode_sel = 0x06,
#else
			      .out_mode_sel = 0x00,
#endif
#else
			      .out_mode_sel = 0x06,
#endif
			      .sclk_dividers = 0x02,
			      .sys_mipi_clk = 0x12,
			      },
	[OV5645_SIZE_5MP] = {
			     /*  Timing control  2624 x 1952 --> 2592 x 1944 */
			     .x_addr_start = 16,
			     .y_addr_start = 4,
			     .x_addr_end = 2607,
			     .y_addr_end = 1947,
			     /*  Output image size */
			     .h_output_size = 2560,
			     .v_output_size = 1920,
			     /*  ISP Windowing size  2592 x 1944 -->
				*   2560 x 1920 */
			     .isp_h_offset = 16,
			     .isp_v_offset = 12,
			     /*  Total size (+blanking) */
			     .h_total_size = 2844,
			     .v_total_size = 1968,
			     /*  Sensor Read Binning Enabled */
			     .h_odd_ss_inc = 1,
			     .h_even_ss_inc = 1,
			     .v_odd_ss_inc = 1,
			     .v_even_ss_inc = 1,
#ifdef CONFIG_MACH_HAWAII_GARNET
#ifdef CONFIG_MACH_HAWAII_GARNET_C_A18
			     .out_mode_sel = 0x06,
#else
			     .out_mode_sel = 0x00,
#endif
#else
			     .out_mode_sel = 0x06,
#endif
			     .sclk_dividers = 0x02,
			     .sys_mipi_clk = 0x12,
			     },
};

static const struct ov5645_timing_cfg timing_cfg_jpeg[OV5645_SIZE_LAST] = {
	[OV5645_SIZE_QVGA] = {
			      /*  Timing control  2624 x 1952 --> 2592 x 1944 */
			      .x_addr_start = 16,
			      .y_addr_start = 4,
			      .x_addr_end = 2607,
			      .y_addr_end = 1947,
			      /*  Output image size */
			      .h_output_size = 320,
			      .v_output_size = 240,
			      /*  ISP Windowing size  2592 x 1944 -->
				*   2560 x 1920 */
			      .isp_h_offset = 16,
			      .isp_v_offset = 12,
			      /*  Total size (+blanking) */
			      .h_total_size = 2844,
			      .v_total_size = 1968,
			      /*  Sensor Read Binning Disabled */
			      .h_odd_ss_inc = 1,
			      .h_even_ss_inc = 1,
			      .v_odd_ss_inc = 1,
			      .v_even_ss_inc = 1,
			      .out_mode_sel = 0x26,
			      .sclk_dividers = 0x01,
			      .sys_mipi_clk = 0x12,
			      },
	[OV5645_SIZE_VGA] = {
			     /*  Timing control  2624 x 1952 --> 2592 x 1944 */
			     .x_addr_start = 16,
			     .y_addr_start = 4,
			     .x_addr_end = 2607,
			     .y_addr_end = 1947,
			     /*  Output image size */
			     .h_output_size = 640,
			     .v_output_size = 480,
			     /*  ISP Windowing size  2592 x 1944 -->
				*  *  2560 x 1920 */
				.isp_h_offset = 16,
			     .isp_v_offset = 12,
			     /*  Total size (+blanking) */
			     .h_total_size = 2844,
			     .v_total_size = 1968,
			     /*  Sensor Read Binning Disabled */
			     .h_odd_ss_inc = 1,
			     .h_even_ss_inc = 1,
			     .v_odd_ss_inc = 1,
			     .v_even_ss_inc = 1,
			     .out_mode_sel = 0x26,
			     .sclk_dividers = 0x01,
			     .sys_mipi_clk = 0x12,
			     },
	[OV5645_SIZE_720P] = {
			      /*  Timing control  2624 x 1952 --> 2592 x 1944 */
			      .x_addr_start = 16,
			      .y_addr_start = 4,
			      .x_addr_end = 2607,
			      .y_addr_end = 1947,
			      /*  Output image size */
			      .h_output_size = 1280,
			      .v_output_size = 720,
			      /*  ISP Windowing size  2592 x 1944 -->
				*  2560 x 1920 */
			      .isp_h_offset = 16,
			      .isp_v_offset = 12,
			      /*  Total size (+blanking) */
			      .h_total_size = 2844,
			      .v_total_size = 1968,
			      /*  Sensor Read Binning Disabled */
			      .h_odd_ss_inc = 1,
			      .h_even_ss_inc = 1,
			      .v_odd_ss_inc = 1,
			      .v_even_ss_inc = 1,
			      .out_mode_sel = 0x26,
			      .sclk_dividers = 0x01,
			      .sys_mipi_clk = 0x12,
			      },
	[OV5645_SIZE_1280x960] = {
				  /*  Timing control  2624 x 1952 -->
					*  2592 x 1944 */
				  .x_addr_start = 16,
				  .y_addr_start = 4,
				  .x_addr_end = 2607,
				  .y_addr_end = 1947,
				  /*  Output image size */
				  .h_output_size = 1280,
				  .v_output_size = 960,
				  /*  ISP Windowing size  2592 x 1944 -->
					*  2560 x 1920 */
				  .isp_h_offset = 16,
				  .isp_v_offset = 12,
				  /*  Total size (+blanking) */
				  .h_total_size = 2844,
				  .v_total_size = 1968,
				  /*  Sensor Read Binning Disabled */
				  .h_odd_ss_inc = 1,
				  .h_even_ss_inc = 1,
				  .v_odd_ss_inc = 1,
				  .v_even_ss_inc = 1,
				  .out_mode_sel = 0x26,
				  .sclk_dividers = 0x01,
				  .sys_mipi_clk = 0x12,
				  },
	[OV5645_SIZE_UXGA] = {
			      /*  Timing control  2624 x 1952 --> 2592 x 1944 */
			      .x_addr_start = 16,
			      .y_addr_start = 4,
			      .x_addr_end = 2607,
			      .y_addr_end = 1947,
			      /*  Output image size */
			      .h_output_size = 1600,
			      .v_output_size = 1200,
			      /*  ISP Windowing size  2592 x 1944 -->
				*  2560 x 1920 */
			      .isp_h_offset = 16,
			      .isp_v_offset = 12,
			      /*  Total size (+blanking) */
			      .h_total_size = 2844,
			      .v_total_size = 1968,
			      /*  Sensor Read Binning Disabled */
			      .h_odd_ss_inc = 1,
			      .h_even_ss_inc = 1,
			      .v_odd_ss_inc = 1,
			      .v_even_ss_inc = 1,
			      .out_mode_sel = 0x26,
			      .sclk_dividers = 0x01,
			      .sys_mipi_clk = 0x12,
			      },
	[OV5645_SIZE_QXGA] = {
			      /*  Timing control  2624 x 1952 --> 2592 x 1944 */
			      .x_addr_start = 16,
			      .y_addr_start = 4,
			      .x_addr_end = 2607,
			      .y_addr_end = 1947,
			      /*  Output image size */
			      .h_output_size = 2048,
			      .v_output_size = 1536,
			      /*  ISP Windowing size  2592 x 1944 -->
				*  2560 x 1920 */
			      .isp_h_offset = 16,
			      .isp_v_offset = 12,
			      /*  Total size (+blanking) */
			      .h_total_size = 2844,
			      .v_total_size = 1968,
			      /*  Sensor Read Binning Disabled */
			      .h_odd_ss_inc = 1,
			      .h_even_ss_inc = 1,
			      .v_odd_ss_inc = 1,
			      .v_even_ss_inc = 1,
			      .out_mode_sel = 0x26,
			      .sclk_dividers = 0x01,
			      .sys_mipi_clk = 0x12,
			      },
	[OV5645_SIZE_5MP] = {
			     /*  Timing control  2624 x 1952 --> 2592 x 1944 */
			     .x_addr_start = 16,
			     .y_addr_start = 4,
			     .x_addr_end = 2607,
			     .y_addr_end = 1947,
			     /* Output image size */
			     .h_output_size = 2560,
			     .v_output_size = 1920,
			     /*  ISP Windowing size  2592 x 1944 -->
				*  2560 x 1920 */
			     .isp_h_offset = 16,
			     .isp_v_offset = 12,
			     /* Total size (+blanking) */
			     .h_total_size = 2844,
			     .v_total_size = 1968,
			     /* Sensor Read Binning Disabled */
			     .h_odd_ss_inc = 1,
			     .h_even_ss_inc = 1,
			     .v_odd_ss_inc = 1,
			     .v_even_ss_inc = 1,
			     .out_mode_sel = 0x26,
			     .sclk_dividers = 0x01,
			     .sys_mipi_clk = 0x12,
			     },
};



static int ov5645_config_timing(struct i2c_client *client);

/* LED functions */
static void MIC2871_writeflash(char addr, char data)
{
	int i;
	/* send address */
	printk(KERN_ALERT "%s addr(%d) data(%d)\n", __func__, addr, data);
	for (i = 0; i < (addr + 1); i++) {
		gpio_set_value(CAM_FLASH_ENSET, 0);
		udelay(1);
		gpio_set_value(CAM_FLASH_ENSET, 1);
		udelay(1);
	}
	/* wait T lat */
	udelay(97);
	/* send data */
	for (i = 0; i < (data + 1); i++) {
		gpio_set_value(CAM_FLASH_ENSET, 0);
		udelay(1);
		gpio_set_value(CAM_FLASH_ENSET, 1);
		udelay(1);
	}
/* THis only needs to be 405us */
	/* wait T end */
	udelay(405);
}

int main_cam_ledcontrol(int light, int mode)
{
	unsigned long flags;
	spinlock_t lock;
	spin_lock_init(&lock);

	gpio_request(CAM_FLASH_ENSET, "camacq");
	gpio_request(CAM_FLASH_FLEN, "camacq");

	switch (light) {
	case CAM_LED_ON:

		spin_lock_irqsave(&lock, flags);
		gpio_set_value(CAM_FLASH_ENSET, 1);

		/* write "Disabled"(0) to LB_TH(4) */
		MIC2871_writeflash(4, 0);

		if (mode == CAM_LED_MODE_PRE) {
			/* write 56%(21) to TEN/TCUR(2) */
			MIC2871_writeflash(2, 21);
		} else {
			MIC2871_writeflash(5, 1);
		/* Register value 7 is the default for regiser 3,
		 * so no need to do this */
		/* MIC2871_write(3, 7); */
		/* Following is hte new case using registers only */
		/* write 100%(0) to FEN/FCUR(1) */
			MIC2871_writeflash(1, 16);
		}

		spin_unlock_irqrestore(&lock, flags);
		break;
	case CAM_LED_OFF:

		/* initailize falsh IC */
		gpio_set_value(CAM_FLASH_FLEN, 0);
		gpio_set_value(CAM_FLASH_ENSET, 0);
/* For SWI this only needs to be 400us */
		/* mdelay(1); */
		udelay(500);
		break;
	default:
		printk(KERN_ALERT "%s:not case %d", __func__, light);
		return -1;
		break;
	}
	gpio_free(CAM_FLASH_ENSET);
	gpio_free(CAM_FLASH_FLEN);

	return 0;
}


/**
 *ov5645_reg_read - Read a value from a register in an ov5645 sensor device
 *@client: i2c driver client structure
 *@reg: register address / offset
 *@val: stores the value that gets read
 *
 * Read a value from a register in an ov5645 sensor device.
 * The value is returned in 'val'.
 * Returns zero if successful, or non-zero otherwise.
 */
static int ov5645_reg_read(struct i2c_client *client, u16 reg, u8 *val)
{
	int ret;
	u8 data[2] = { 0 };
	struct i2c_msg msg = {
		.addr = client->addr,
		.flags = 0,
		.len = 2,
		.buf = data,
	};

	data[0] = (u8) (reg >> 8);
	data[1] = (u8) (reg & 0xff);

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret < 0)
		goto err;

	msg.flags = I2C_M_RD;
	msg.len = 1;
	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret < 0)
		goto err;

	*val = data[0];
	return 0;

err:
	dev_err(&client->dev, "Failed reading register 0x%02x!\n", reg);
	return ret;
}

/**
 * Write a value to a register in ov5645 sensor device.
 *@client: i2c driver client structure.
 *@reg: Address of the register to read value from.
 *@val: Value to be written to a specific register.
 * Returns zero if successful, or non-zero otherwise.
 */
static int ov5645_reg_write(struct i2c_client *client, u16 reg, u8 val)
{
	int ret;
	unsigned char data[3] = { (u8) (reg >> 8), (u8) (reg & 0xff), val };
	struct i2c_msg msg = {
		.addr = client->addr,
		.flags = 0,
		.len = 3,
		.buf = data,
	};

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret < 0) {
		dev_err(&client->dev, "Failed writing register 0x%02x!\n", reg);
		return ret;
	}

	return 0;
}

static const struct v4l2_queryctrl ov5645_controls[] = {
	{
	 .id = V4L2_CID_CAMERA_BRIGHTNESS,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "Brightness",
	 .minimum = EV_MINUS_2,
	 .maximum = EV_PLUS_2,
	 .step = 1,
	 .default_value = EV_DEFAULT,
	 },
	{
	 .id = V4L2_CID_CAMERA_CONTRAST,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "Contrast",
	 .minimum = CONTRAST_MINUS_1,
	 .maximum = CONTRAST_PLUS_1,
	 .step = 1,
	 .default_value = CONTRAST_DEFAULT,
	 },
#ifndef CONFIG_MACH_HAWAII_GARNET_C_W70
	{
	 .id = V4L2_CID_CAMERA_FLASH_MODE,
	 .type = V4L2_CTRL_TYPE_INTEGER,
#ifdef CONFIG_VIDEO_AS3643
	 .name = "AS3643-flash",
#endif
#ifdef CONFIG_VIDEO_ADP1653
	 .name = "ADP1653-flash",
#endif
#ifdef CONFIG_VIDEO_SGM3410
	 .name = "SGM3140-flash",
#endif
	 .minimum = FLASH_MODE_OFF,
	 .maximum = (1 << FLASH_MODE_OFF) | (1 << FLASH_MODE_ON) |
		(1 << FLASH_MODE_TORCH_OFF) | (1 << FLASH_MODE_TORCH_ON),
	 .step = 1,
	 .default_value = FLASH_MODE_OFF,
	 },
#endif
	{
	 .id = V4L2_CID_CAMERA_EFFECT,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "Color Effects",
	 .minimum = IMAGE_EFFECT_NONE,
	 .maximum = (1 << IMAGE_EFFECT_NONE | 1 << IMAGE_EFFECT_SEPIA |
		     1 << IMAGE_EFFECT_BNW | 1 << IMAGE_EFFECT_NEGATIVE),
	 .step = 1,
	 .default_value = IMAGE_EFFECT_NONE,
	 },
	{
	 .id = V4L2_CID_CAMERA_ANTI_BANDING,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "Anti Banding",
	 .minimum = ANTI_BANDING_AUTO,
	 .maximum = ANTI_BANDING_60HZ,
	 .step = 1,
	 .default_value = ANTI_BANDING_AUTO,
	 },
	{
	 .id = V4L2_CID_CAMERA_WHITE_BALANCE,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "White Balance",
	 .minimum = WHITE_BALANCE_AUTO,
	 .maximum = WHITE_BALANCE_FLUORESCENT,
	 .step = 1,
	 .default_value = WHITE_BALANCE_AUTO,
	 },
	{
	 .id = V4L2_CID_CAMERA_FRAME_RATE,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "Framerate control",
	 .minimum = FRAME_RATE_AUTO,
	 .maximum = (1 << FRAME_RATE_AUTO |
		     1 << FRAME_RATE_10 | 1 << FRAME_RATE_15 |
		     1 << FRAME_RATE_25 | 1 << FRAME_RATE_30),
	 .step = 1,
	 .default_value = FRAME_RATE_AUTO,
	 },
#ifndef CONFIG_MACH_HAWAII_GARNET_C_W70
	{
	 .id = V4L2_CID_CAMERA_FOCUS_MODE,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "Focus Modes",
	 .minimum = FOCUS_MODE_AUTO,
	 .maximum = (1 << FOCUS_MODE_AUTO),
	 .step = 1,
	 .default_value = FOCUS_MODE_AUTO,
	 },
#endif
	{
	 .id = V4L2_CID_CAMERA_SET_AUTO_FOCUS,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "AF start/stop",
	 .minimum = AUTO_FOCUS_OFF,
	 .maximum = AUTO_FOCUS_ON,
	 .step = 1,
	 .default_value = AUTO_FOCUS_OFF,
	 },
	{
	 .id = V4L2_CID_CAMERA_TOUCH_AF_AREA,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "Touch focus areas",
	 .minimum = 0,
	 .maximum = OV5645_MAX_FOCUS_AREAS,
	 .step = 1,
	 .default_value = 1,
	 },

};

/**
 * Initialize a list of ov5645 registers.
 * The list of registers is terminated by the pair of values
 *@client: i2c driver client structure.
 *@reglist[]: List of address of the registers to write data.
 * Returns zero if successful, or non-zero otherwise.
 */
static int ov5645_reg_writes(struct i2c_client *client,
			     const struct ov5645_reg reglist[])
{
	int err = 0, index;

	for (index = 0; ((reglist[index].reg != 0xFFFF) &&
		(err == 0)); index++) {
		err |=
		    ov5645_reg_write(client, reglist[index].reg,
				     reglist[index].val);
		/*  Check for Pause condition */
		if ((reglist[index + 1].reg == 0xFFFF)
		    && (reglist[index + 1].val != 0)) {
			msleep(reglist[index + 1].val);
			index += 1;
		}
	}
	return 0;
}

#ifdef OV5645_DEBUG
static int ov5645_reglist_compare(struct i2c_client *client,
				  const struct ov5645_reg reglist[])
{
	int err = 0, index;
	u8 reg;

	for (index = 0; ((reglist[index].reg != 0xFFFF) &&
		(err == 0)); index++) {
		err |= ov5645_reg_read(client, reglist[index].reg, &reg);
		if (reglist[index].val != reg) {
			iprintk("reg err:reg=0x%x val=0x%x rd=0x%x",
				reglist[index].reg, reglist[index].val, reg);
		}
		/*  Check for Pause condition */
		if ((reglist[index + 1].reg == 0xFFFF)
		    && (reglist[index + 1].val != 0)) {
			msleep(reglist[index + 1].val);
			index += 1;
		}
	}
	return 0;
}
#endif

/**
 * Write an array of data to ov5645 sensor device.
 *@client: i2c driver client structure.
 *@reg: Address of the register to read value from.
 *@data: pointer to data to be written starting at specific register.
 *@size: # of data to be written starting at specific register.
 * Returns zero if successful, or non-zero otherwise.
 */
static int ov5645_array_write(struct i2c_client *client,
			      const u8 *data, u16 size)
{
	int ret;
	struct i2c_msg msg = {
		.addr = client->addr,
		.flags = 0,
		.len = size,
		.buf = (u8 *) data,
	};

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret < 0) {
		dev_err(&client->dev, "Failed writing array to 0x%02x!\n",
			((data[0] << 8) | (data[1])));
		return ret;
	}

	return 0;
}

static int ov5645_af_ack(struct i2c_client *client, int num_trys)
{
	int ret = 0;
	u8 af_ack = 0;
	int i;


	for (i = 0; i < num_trys; i++) {
		ov5645_reg_read(client, OV5645_CMD_ACK, &af_ack);
		if (af_ack == 0)
			break;
		msleep(50);
	}
	if (af_ack != 0) {
		dev_dbg(&client->dev, "af ack failed\n");
		return OV5645_AF_FAIL;
	}
	return ret;
}

static int ov5645_af_fw_status(struct i2c_client *client)
{
	u8 af_st = 0;


	ov5645_reg_read(client, OV5645_CMD_FW_STATUS, &af_st);

	iprintk("status=0x%x", af_st);
	return (int)af_st;
}

static int ov5645_af_enable(struct i2c_client *client)
{
	int ret = 0;
	u8 af_st;
	int i;
	iprintk("ov5645_af_enable entry");

	/* hawaii_camera_AF_power(1); */
	msleep(20);

	ret = ov5645_reg_writes(client, ov5645_afpreinit_tbl);
	if (ret)
		return ret;

	ret = ov5645_array_write(client, ov5645_afinit_data,
				 sizeof(ov5645_afinit_data)
				 / sizeof(ov5645_afinit_data[0]));
	if (ret)
		return ret;

	ret = ov5645_reg_writes(client, ov5645_afpostinit_tbl);
	if (ret)
		return ret;

	msleep(20);

	for (i = 0; i < 30; i++) {
		ov5645_reg_read(client, OV5645_CMD_FW_STATUS, &af_st);
		if (af_st == 0x70)
			break;
		msleep(20);
	}
	iprintk("af_st check time %d", i);

	return ret;
}

static int ov5645_af_release(struct i2c_client *client)
{
	int ret = 0;


	ret = ov5645_reg_write(client, OV5645_CMD_ACK, 0x01);
	if (ret)
		return ret;
	ret = ov5645_reg_write(client, OV5645_CMD_MAIN, 0x08);
	if (ret)
		return ret;
	ov5645_af_fw_status(client);

	return ret;
}

static int ov5645_af_center(struct i2c_client *client)
{
	int ret = 0;


	ret = ov5645_reg_write(client, OV5645_CMD_ACK, 0x01);
	if (ret)
		return ret;
	ret = ov5645_reg_write(client, OV5645_CMD_MAIN, 0x80);
	if (ret)
		return ret;
	ret = ov5645_af_ack(client, 50);
	if (ret) {
		dev_dbg(&client->dev, "failed\n");
		return OV5645_AF_FAIL;
	}

	return ret;
}

static int ov5645_af_macro(struct i2c_client *client)
{
	int ret = 0;
	u8 reg;


	ret = ov5645_af_release(client);
	if (ret)
		return ret;
	/* move VCM all way out */
	ret = ov5645_reg_read(client, 0x3603, &reg);
	if (ret)
		return ret;
	reg &= ~(0x3f);
	ret = ov5645_reg_write(client, 0x3603, reg);
	if (ret)
		return ret;

	ret = ov5645_reg_read(client, 0x3602, &reg);
	if (ret)
		return ret;
	reg &= ~(0xf0);
	ret = ov5645_reg_write(client, 0x3602, reg);
	if (ret)
		return ret;

	/* set direct mode */
	ret = ov5645_reg_read(client, 0x3602, &reg);
	if (ret)
		return ret;
	reg &= ~(0x07);
	ret = ov5645_reg_write(client, 0x3602, reg);
	if (ret)
		return ret;

	return ret;
}

static int ov5645_af_infinity(struct i2c_client *client)
{
	int ret = 0;
	u8 reg;


	ret = ov5645_af_release(client);
	if (ret)
		return ret;
	/* move VCM all way in */
	ret = ov5645_reg_read(client, 0x3603, &reg);
	if (ret)
		return ret;
	reg |= 0x3f;
	ret = ov5645_reg_write(client, 0x3603, reg);
	if (ret)
		return ret;

	ret = ov5645_reg_read(client, 0x3602, &reg);
	if (ret)
		return ret;
	reg |= 0xf0;
	ret = ov5645_reg_write(client, 0x3602, reg);
	if (ret)
		return ret;

	/* set direct mode */
	ret = ov5645_reg_read(client, 0x3602, &reg);
	if (ret)
		return ret;
	reg &= ~(0x07);
	ret = ov5645_reg_write(client, 0x3602, reg);
	if (ret)
		return ret;

	return ret;
}

/* Set the touch area x,y in VVF coordinates*/
static int ov5645_af_touch(struct i2c_client *client)
{
	int ret = OV5645_AF_SUCCESS;
	struct ov5645 *ov5645 = to_ov5645(client);


	/* verify # zones correct */
	if (ov5645->touch_focus) {

		/* touch zone config */
		ret = ov5645_reg_write(client, 0x3024,
				       (u8) ov5645->touch_area[0].leftTopX);
		if (ret)
			return ret;
		ret = ov5645_reg_write(client, 0x3025,
				       (u8) ov5645->touch_area[0].leftTopY);
		if (ret)
			return ret;
		ret = ov5645_reg_write(client, OV5645_CMD_ACK, 0x01);
		if (ret)
			return ret;
		ret = ov5645_reg_write(client, OV5645_CMD_MAIN, 0x81);
		if (ret)
			return ret;
		ret = ov5645_af_ack(client, 50);
		if (ret) {
			dev_dbg(&client->dev, "zone config ack failed\n");
			return ret;
		}

	}

	iprintk(" exit");

	return ret;
}

/* Set the touch area, areas can overlap and
are givin in current sensor resolution coords */
static int ov5645_af_area(struct i2c_client *client)
{
	int ret = OV5645_AF_SUCCESS;
	struct ov5645 *ov5645 = to_ov5645(client);
	u8 weight[OV5645_MAX_FOCUS_AREAS];
	int i;


	/* verify # zones correct */
	if ((ov5645->touch_focus) &&
	    (ov5645->touch_focus <= OV5645_MAX_FOCUS_AREAS)) {

		iprintk("entry touch_focus %d", ov5645->touch_focus);

		/* enable zone config */
		ret = ov5645_reg_write(client, OV5645_CMD_ACK, 0x01);
		if (ret)
			return ret;
		ret = ov5645_reg_write(client, OV5645_CMD_MAIN, 0x8f);
		if (ret)
			return ret;
		ret = ov5645_af_ack(client, 50);
		if (ret) {
			dev_dbg(&client->dev, "zone config ack failed\n");
			return ret;
		}

		/* clear all zones */
		for (i = 0; i < OV5645_MAX_FOCUS_AREAS; i++)
			weight[i] = 0;

		/* write area to sensor */
		for (i = 0; i < ov5645->touch_focus; i++) {

			ret = ov5645_reg_write(client, 0x3024,
					       (u8) ov5645->
					       touch_area[i].leftTopX);
			if (ret)
				return ret;
			ret = ov5645_reg_write(client, 0x3025,
					       (u8) ov5645->
					       touch_area[i].leftTopY);
			if (ret)
				return ret;
			ret = ov5645_reg_write(client, 0x3026,
					       (u8) (ov5645->
						     touch_area[i].leftTopX +
						     ov5645->
						     touch_area
						     [i].rightBottomX));
			if (ret)
				return ret;
			ret = ov5645_reg_write(client, 0x3027,
					       (u8) (ov5645->
						     touch_area[i].leftTopY +
						     ov5645->
						     touch_area
						     [i].rightBottomY));
			if (ret)
				return ret;
			ret = ov5645_reg_write(client, OV5645_CMD_ACK, 0x01);
			if (ret)
				return ret;
			ret = ov5645_reg_write(client, OV5645_CMD_MAIN,
					       (0x90 + i));
			if (ret)
				return ret;
			ret = ov5645_af_ack(client, 50);
			if (ret) {
				dev_dbg(&client->dev, "zone update failed\n");
				return ret;
			}
			weight[i] = (u8) ov5645->touch_area[i].weight;
		}

		/* enable zone with weight */
		ret = ov5645_reg_write(client, 0x3024, weight[0]);
		if (ret)
			return ret;
		ret = ov5645_reg_write(client, 0x3025, weight[1]);
		if (ret)
			return ret;
		ret = ov5645_reg_write(client, 0x3026, weight[2]);
		if (ret)
			return ret;
		ret = ov5645_reg_write(client, 0x3027, weight[3]);
		if (ret)
			return ret;
		ret = ov5645_reg_write(client, 0x3028, weight[4]);
		if (ret)
			return ret;
		ret = ov5645_reg_write(client, OV5645_CMD_ACK, 0x01);
		if (ret)
			return ret;
		ret = ov5645_reg_write(client, OV5645_CMD_MAIN, 0x98);
		if (ret)
			return ret;
		ret = ov5645_af_ack(client, 50);
		if (ret) {
			dev_dbg(&client->dev, "weights failed\n");
			return ret;
		}

		/* launch zone configuration */
		ret = ov5645_reg_write(client, OV5645_CMD_ACK, 0x01);
		if (ret)
			return ret;
		ret = ov5645_reg_write(client, OV5645_CMD_MAIN, 0x9f);
		if (ret)
			return ret;
		ret = ov5645_af_ack(client, 50);
		if (ret) {
			dev_dbg(&client->dev, "launch failed\n");
			return ret;
		}
	}

	return ret;
}

/* Convert touch area from sensor resolution coords to ov5645 VVF zone */
static int ov5645_af_zone_conv(struct i2c_client *client,
			       v4l2_touch_area *zone_area, int zone)
{
	int ret = 0;
	u32 x0, y0, x1, y1, weight;
	struct ov5645 *ov5645 = to_ov5645(client);

	iprintk("entry ");

	/* Reset zone */
	ov5645->touch_area[zone].leftTopX = 0;
	ov5645->touch_area[zone].leftTopY = 0;
	ov5645->touch_area[zone].rightBottomX = 0;
	ov5645->touch_area[zone].rightBottomY = 0;
	ov5645->touch_area[zone].weight = 0;

	/* x y w h are in current sensor resolution dimensions */
	if (((u32) zone_area->leftTopX + (u32) zone_area->rightBottomX)
	    > ov5645_frmsizes[ov5645->i_size].width) {
		iprintk("zone width error: x=0x%x w=0x%x",
			zone_area->leftTopX, zone_area->rightBottomX);
		ret = -EINVAL;
		goto out;
	} else if (((u32) zone_area->leftTopY + (u32) zone_area->rightBottomY)
		   > ov5645_frmsizes[ov5645->i_size].height) {
		iprintk("zone height error: y=0x%x h=0x%x",
			zone_area->leftTopY, zone_area->rightBottomY);
		ret = -EINVAL;
		goto out;
	} else if ((u32) zone_area->weight > 1000) {

		iprintk("zone weight error: weight=0x%x", zone_area->weight);
		ret = -EINVAL;
		goto out;
	}

	/* conv area to sensor VVF zone */
	x0 = (u32) zone_area->leftTopX / af_zone_scale[ov5645->i_size].x_scale;
	if (x0 > (OV5645_AF_NORMALIZED_W - 8))
		x0 = (OV5645_AF_NORMALIZED_W - 8);
	x1 = ((u32) zone_area->leftTopX + (unsigned int)zone_area->rightBottomX)
	    / af_zone_scale[ov5645->i_size].x_scale;
	if (x1 > OV5645_AF_NORMALIZED_W)
		x1 = OV5645_AF_NORMALIZED_W;
	y0 = (u32) zone_area->leftTopY / af_zone_scale[ov5645->i_size].y_scale;
	if (y0 > (OV5645_AF_NORMALIZED_H - 8))
		y0 = (OV5645_AF_NORMALIZED_H - 8);
	y1 = ((u32) zone_area->leftTopY + (unsigned int)zone_area->rightBottomY)
	    / af_zone_scale[ov5645->i_size].y_scale;
	if (y1 > OV5645_AF_NORMALIZED_H)
		y1 = OV5645_AF_NORMALIZED_H;

	/* weight ranges from 1-1000 */
	/* Convert weight */
	weight = 0;
	if ((zone_area->weight > 0) && (zone_area->weight <= 125))
		weight = 1;
	else if ((zone_area->weight > 125) && (zone_area->weight <= 250))
		weight = 2;
	else if ((zone_area->weight > 250) && (zone_area->weight <= 375))
		weight = 3;
	else if ((zone_area->weight > 375) && (zone_area->weight <= 500))
		weight = 4;
	else if ((zone_area->weight > 500) && (zone_area->weight <= 625))
		weight = 5;
	else if ((zone_area->weight > 625) && (zone_area->weight <= 750))
		weight = 6;
	else if ((zone_area->weight > 750) && (zone_area->weight <= 875))
		weight = 7;
	else if (zone_area->weight > 875)
		weight = 8;

	/* Minimum zone size */
	if (((x1 - x0) >= 8) && ((y1 - y0) >= 8)) {

		ov5645->touch_area[zone].leftTopX = (int)x0;
		ov5645->touch_area[zone].leftTopY = (int)y0;
		ov5645->touch_area[zone].rightBottomX = (int)(x1 - x0);
		ov5645->touch_area[zone].rightBottomY = (int)(y1 - y0);
		ov5645->touch_area[zone].weight = (int)weight;

	} else {
		dev_dbg(&client->dev,
			"zone %d size failed: x0=%d x1=%d y0=%d y1=%d w=%d\n",
			zone, x0, x1, y0, y1, weight);
		ret = -EINVAL;
		goto out;
	}

out:

	return ret;
}

static int ov5645_af_status(struct i2c_client *client, int num_trys)
{
	int ret = OV5645_AF_SUCCESS;
	struct ov5645 *ov5645 = to_ov5645(client);
	int af_st = 0;
	u8 af_zone0, af_zone1, af_zone2, af_zone3, af_zone4;


	if (ov5645->focus_mode == FOCUS_MODE_AUTO) {
		/* Check if Focused */
		af_st = ov5645_af_fw_status(client);
		if (af_st != 0x10) {
			dev_dbg(&client->dev, "focus pending\n");
			ret = OV5645_AF_PENDING;
			goto out;
		}

		/* Check if Zones Focused */
		ov5645_reg_write(client, OV5645_CMD_ACK, 0x01);
		ov5645_reg_write(client, OV5645_CMD_MAIN, 0x07);

		ret = ov5645_af_ack(client, num_trys);
		if (ret) {
			dev_dbg(&client->dev, "zones ack failed\n");
			ret = OV5645_AF_FAIL;
			goto out;
		}

		ov5645_reg_read(client, 0x3024, &af_zone0);
		ov5645_reg_read(client, 0x3025, &af_zone1);
		ov5645_reg_read(client, 0x3026, &af_zone2);
		ov5645_reg_read(client, 0x3027, &af_zone3);
		ov5645_reg_read(client, 0x3028, &af_zone4);
		if ((af_zone0 != 0) && (af_zone1 != 0) && (af_zone2 != 0)
		    && (af_zone3 != 0) && (af_zone4 != 0)) {
			dev_dbg(&client->dev, "zones failed\n");
			ret = OV5645_AF_FAIL;
			iprintk("zones failed");
			goto out;
		}

	}

out:
	return ret;
}

/* For capture routines */
#define XVCLK 1300
static int AE_Target = 44;
static int AE_low, AE_high;
static int preview_sysclk, preview_HTS;

static int ov5645_get_sysclk(struct v4l2_subdev *sd)
{
	/*" calculate sysclk */
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	u8 val;
	int Multiplier, PreDiv, VCO, SysDiv, Pll_rdiv, Bit_div2x = 1, sclk_rdiv,
	    sysclk;

	int sclk_rdiv_map[] = { 1, 2, 4, 8 };

	ov5645_reg_read(client, 0x3034, &val);
	val &= 0x0F;
	if (val == 8 || val == 10)
		Bit_div2x = val / 2;
	ov5645_reg_read(client, 0x3035, &val);
	SysDiv = val >> 4;
	if (SysDiv == 0)
		SysDiv = 16;

	ov5645_reg_read(client, 0x3036, &val);
	Multiplier = val;

	ov5645_reg_read(client, 0x3037, &val);
	PreDiv = val & 0x0f;
	Pll_rdiv = ((val >> 4) & 0x01) + 1;

	ov5645_reg_read(client, 0x3108, &val);
	val &= 0x03;
	sclk_rdiv = sclk_rdiv_map[val];

	VCO = XVCLK * Multiplier / PreDiv;

	sysclk = VCO / SysDiv / Pll_rdiv * 2 / Bit_div2x / sclk_rdiv;

	return sysclk;
}

static int ov5645_get_HTS(struct v4l2_subdev *sd)
{
	/* read HTS from register settings */
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int HTS;
	u8 val;

	ov5645_reg_read(client, 0x380c, &val);
	HTS = val;
	ov5645_reg_read(client, 0x380d, &val);
	HTS = (HTS << 8) + val;

	return HTS;
}

static int ov5645_get_VTS(struct v4l2_subdev *sd)
{
	/* read VTS from register settings*/
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int VTS;
	u8 val;

	ov5645_reg_read(client, 0x380e, &val);
	VTS = val;
	ov5645_reg_read(client, 0x380f, &val);
	VTS = (VTS << 8) + val;

	return VTS;
}

static int ov5645_set_VTS(struct v4l2_subdev *sd, int VTS)
{
	/* write VTS to registers*/
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	u8 val;

	val = VTS & 0xFF;
	ov5645_reg_write(client, 0x380F, val);
	val = VTS >> 8;
	ov5645_reg_write(client, 0x380E, val);

	return 0;
}

static int ov5645_get_shutter(struct v4l2_subdev *sd)
{
	/* read shutter, in number of line period*/
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int shutter;
	u8 val;

	ov5645_reg_read(client, 0x3500, &val);
	shutter = (val & 0x0f);
	ov5645_reg_read(client, 0x3501, &val);
	shutter = (shutter << 8) + val;
	ov5645_reg_read(client, 0x3502, &val);
	shutter = (shutter << 4) + (val >> 4);

	return shutter;
}

static int ov5645_set_shutter(struct v4l2_subdev *sd, int shutter)
{
	/* write shutter, in number of line period*/
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	u8 val;

	shutter = shutter & 0xFFFF;
	val = (shutter & 0x0F) << 4;
	ov5645_reg_write(client, 0x3502, val);

	val = (shutter & 0xFFF) >> 4;
	ov5645_reg_write(client, 0x3501, val);

	val = shutter >> 12;
	ov5645_reg_write(client, 0x3500, val);

	return 0;
}

static int ov5645_get_exp_time(struct v4l2_subdev *sd)
{
	u8 line_l, val, etime = 0;
	line_l = ov5645_get_HTS(sd);
	val = ov5645_get_shutter(sd);
	etime = val * line_l;

	return etime;
}

static int ov5645_get_red_gain16(struct v4l2_subdev *sd)
{
	/* read gain, 16 = 1x*/
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	u16 gain16;
	u8 val;

	ov5645_reg_read(client, 0x3400, &val);
	gain16 = val & 0x0F;
	ov5645_reg_read(client, 0x3401, &val);
	gain16 = (gain16 << 8) + val;

	return gain16;
}

static int ov5645_set_red_gain16(struct v4l2_subdev *sd, int gain16)
{
	/* write gain, 16 = 1x*/
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	u8 val;
	gain16 = gain16 & 0xFFF;

	return 0;

	val = gain16 & 0xFF;
	ov5645_reg_write(client, 0x3401, val);

	val = gain16 >> 8;
	ov5645_reg_write(client, 0x3400, val);

	return 0;
}

static int ov5645_get_green_gain16(struct v4l2_subdev *sd)
{
	/* read gain, 16 = 1x */
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	u16 gain16;
	u8 val;

	ov5645_reg_read(client, 0x3402, &val);
	gain16 = val & 0x0F;
	ov5645_reg_read(client, 0x3403, &val);
	gain16 = (gain16 << 8) + val;

	return gain16;
}

static int ov5645_set_green_gain16(struct v4l2_subdev *sd, int gain16)
{
	/* write gain, 16 = 1x */
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	u8 val;
	gain16 = gain16 & 0xFFF;

	val = gain16 & 0xFF;
	ov5645_reg_write(client, 0x3403, val);

	val = gain16 >> 8;
	ov5645_reg_write(client, 0x3402, val);

	return 0;
}

static int ov5645_get_blue_gain16(struct v4l2_subdev *sd)
{
	/* read gain, 16 = 1x */
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	u16 gain16;
	u8 val;

	ov5645_reg_read(client, 0x3404, &val);
	gain16 = val & 0x0F;
	ov5645_reg_read(client, 0x3405, &val);
	gain16 = (gain16 << 8) + val;

	return gain16;
}

static int ov5645_set_blue_gain16(struct v4l2_subdev *sd, int gain16)
{
	/* write gain, 16 = 1x */
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	u8 val;
	gain16 = gain16 & 0xFFF;

	val = gain16 & 0xFF;
	ov5645_reg_write(client, 0x3405, val);

	val = gain16 >> 8;
	ov5645_reg_write(client, 0x3404, val);

	return 0;
}

static int ov5645_get_gain16(struct v4l2_subdev *sd)
{
	/* read gain, 16 = 1x */
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	u16 gain16;
	u8 val;

	ov5645_reg_read(client, 0x350A, &val);
	gain16 = val & 0x03;
	ov5645_reg_read(client, 0x350B, &val);
	gain16 = (gain16 << 8) + val;

	return gain16;
}

static int ov5645_set_gain16(struct v4l2_subdev *sd, int gain16)
{
	/* write gain, 16 = 1x */
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	u8 val;
	gain16 = gain16 & 0x3FF;

	val = gain16 & 0xFF;
	ov5645_reg_write(client, 0x350b, val);

	val = gain16 >> 8;
	ov5645_reg_write(client, 0x350a, val);

	return 0;
}

static int ov5645_get_banding(struct v4l2_subdev *sd)
{
	/* get banding filter value */
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	u8 val;
	int banding = 0;

	ov5645_reg_read(client, 0x3c01, &val);

	if (val & 0x80) {
		/* manual */
		ov5645_reg_read(client, 0x3c00, &val);
		if (val & 0x04) {
			/* 50Hz */
			banding = 50;
		} else {
			/* 60Hz*/
			banding = 60;
		}
	} else {
		/* auto */
		ov5645_reg_read(client, 0x3c0c, &val);
		if (val & 0x01) {
			/* 50Hz */
			banding = 50;
		} else {
			/* 60Hz */
		}
	}
	return banding;
}

static void ov5645_set_banding(struct v4l2_subdev *sd)
{
	int preview_VTS;
	int band_step60, max_band60, band_step50, max_band50;
	/*u8 val; */
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	/* read preview PCLK */
	preview_sysclk = ov5645_get_sysclk(sd);

	/* read preview HTS*/
	preview_HTS = ov5645_get_HTS(sd);

	/* read preview VTS */
	preview_VTS = ov5645_get_VTS(sd);
	printk(KERN_INFO "%s: preview_HTS=0x%x, VTS: 0x%x preview_sysclk=%ul\n",
	       __func__, preview_HTS, preview_VTS, preview_sysclk);

#if 0
	ov5645_reg_read(client, 0x3034, &val);
	printk(KERN_INFO "%s: [3034]=0x%x", __func__, val);
	ov5645_reg_read(client, 0x3035, &val);
	printk("  [3035]=0x%x", val);
	ov5645_reg_read(client, 0x3036, &val);
	printk("  [3036]=0x%x", val);
	ov5645_reg_read(client, 0x3037, &val);
	printk("  [3037]=0x%x", val);
	ov5645_reg_read(client, 0x3824, &val);
	printk("  [3824]=0x%x", val);
	ov5645_reg_read(client, 0x4837, &val);
	printk("  [4837]=0x%x\n", val);
#endif

	ov5645_reg_write(client, 0x3a02, (preview_VTS >> 8));
	ov5645_reg_write(client, 0x3a03, (preview_VTS & 0xff));
	ov5645_reg_write(client, 0x3a14, (preview_VTS >> 8));
	ov5645_reg_write(client, 0x3a15, (preview_VTS & 0xff));

	/* calculate banding filter */
	/* 60Hz */
	band_step60 = preview_sysclk * 100 / preview_HTS * 100 / 120;
	ov5645_reg_write(client, 0x3a0a, (band_step60 >> 8));
	ov5645_reg_write(client, 0x3a0b, (band_step60 & 0xff));

	max_band60 = (int)((preview_VTS - 4) / band_step60);
	ov5645_reg_write(client, 0x3a0d, max_band60);

	/* 50Hz */
	band_step50 = preview_sysclk * 100 / preview_HTS;
	ov5645_reg_write(client, 0x3a08, (band_step50 >> 8));
	ov5645_reg_write(client, 0x3a09, (band_step50 & 0xff));

	max_band50 = (int)((preview_VTS - 4) / band_step50);
	ov5645_reg_write(client, 0x3a0e, max_band50);
	printk(KERN_INFO
	       "%s: band_step60:0x%x max_band60:0x%x  band_step50:0x%x max_band50:0x%x\n",
	       __func__, band_step60, max_band60, band_step50, max_band50);
}

static void ov5645_set_night_mode(struct v4l2_subdev *sd, int night)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	u8 val;

	switch (night) {
	case 0:		/*Off */
		ov5645_reg_read(client, 0x3a00, &val);
		val &= 0xFB;	/* night mode off, bit[2] = 0 */
		ov5645_reg_write(client, 0x3a00, val);
		break;
	case 1:		/* On */
		ov5645_reg_read(client, 0x3a00, &val);
		val |= 0x04;	/* night mode on, bit[2] = 1 */
		ov5645_reg_write(client, 0x3a00, val);
		break;
	default:
		break;
	}
}

static int ov5645_set_AE_target(struct v4l2_subdev *sd, int target)
{
	/* stable in high */
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int fast_high, fast_low;
	AE_low = target * 23 / 25;	/* 0.92 */
	AE_high = target * 27 / 25;	/* 1.08 */

	fast_high = AE_high << 1;
	if (fast_high > 255)
		fast_high = 255;

	fast_low = AE_low >> 1;

	ov5645_reg_write(client, 0x3a0f, AE_high);
	ov5645_reg_write(client, 0x3a10, AE_low);
	ov5645_reg_write(client, 0x3a1b, AE_high);
	ov5645_reg_write(client, 0x3a1e, AE_low);
	ov5645_reg_write(client, 0x3a11, fast_high);
	ov5645_reg_write(client, 0x3a1f, fast_low);

	return 0;
}

static int ov5645_config_preview(struct v4l2_subdev *sd)
{
	int ret;
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	ov5645_reg_write(client, 0x3503, 0x00);

	ret = ov5645_reg_writes(client, hawaii_preview_init);
	/*ret = ov5645_config_timing(client);*/
	ov5645_set_banding(sd);
	ov5645_set_night_mode(sd, 0);
	ov5645_set_AE_target(sd, AE_Target);

	return ret;
}

static int ov5645_config_capture(struct v4l2_subdev *sd)
{
	int ret = 0;
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	int preview_shutter, preview_gain16;
	u8 average /*, preview_uv*/;
	unsigned int capture_shutter, capture_gain16;
	unsigned int red_gain16, green_gain16, blue_gain16;
	unsigned int capture_sysclk, capture_HTS, capture_VTS;
	unsigned int banding , capture_bandingfilter, capture_max_band;
	unsigned long capture_gain16_shutter;

	/* disable aec/agc */
	ov5645_reg_write(client, 0x3503, 0x03);

	/* read preview PCLK */
	preview_sysclk = ov5645_get_sysclk(sd);

	/* read preview HTS */
	preview_HTS = ov5645_get_HTS(sd);
	printk(KERN_INFO "%s: preview_HTS=0x%x, preview_sysclk=%ul\n",
	       __func__, preview_HTS, preview_sysclk);

	/* read preview shutter */
	preview_shutter = ov5645_get_shutter(sd);

	/* read preview gain */
	preview_gain16 = ov5645_get_gain16(sd);
	printk(KERN_INFO "%s: preview_shutter=0x%x, preview_gain16=0x%x\n",
	       __func__, preview_shutter, preview_gain16);
	/* ov5645_reg_read(client, 0x558c, &preview_uv); */
	red_gain16 = ov5645_get_red_gain16(sd);
	green_gain16 = ov5645_get_green_gain16(sd);
	blue_gain16 = ov5645_get_blue_gain16(sd);

	/* get average */
	ov5645_reg_read(client, 0x56a1, &average);
	printk(KERN_INFO "%s: preview avg=0x%x\n", __func__, average);

	/* turn off night mode for capture */
	ov5645_set_night_mode(sd, 0);

	/* Write capture setting */
	ov5645_reg_writes(client, hawaii_capture_init);
	ov5645_config_timing(client);

	/* read capture VTS */
	capture_VTS = ov5645_get_VTS(sd);
	capture_HTS = ov5645_get_HTS(sd);
	capture_sysclk = ov5645_get_sysclk(sd);
	printk(KERN_INFO
	       "%s: capture_VTS=0x%x, capture_HTS=0x%x, capture_sysclk=%ul\n",
	       __func__, capture_VTS, capture_HTS, capture_sysclk);
	/* calculate capture banding filter */
	banding = ov5645_get_banding(sd);
	if (banding == 60) {
		/* 60Hz */
		capture_bandingfilter =
		    capture_sysclk * 100 / capture_HTS * 100 / 120;
	} else {
		/* 50Hz */
		capture_bandingfilter = capture_sysclk * 100 / capture_HTS;
	}
	capture_max_band = (int)((capture_VTS - 4) / capture_bandingfilter);
	/* preview_shutter = preview_shutter * 5 / 4; */

	/* calculate capture shutter/gain16 */
	capture_gain16_shutter =
	    preview_gain16 * preview_shutter * capture_sysclk;
	if (average > AE_low && average < AE_high) {
		/* in stable range
		* printk("average0\n"); */
		capture_gain16_shutter =
		    capture_gain16_shutter / preview_sysclk * preview_HTS /
		    capture_HTS * AE_Target / average * 2;
	} else {
		/* printk("average1\n"); */
		capture_gain16_shutter =
		    capture_gain16_shutter / preview_sysclk * preview_HTS /
		    capture_HTS * 2;
	}

	/* gain to shutter */
	if (capture_gain16_shutter < (capture_bandingfilter * 16)) {
		/* shutter < 1/100
		 printk("gain0\n"); */
		capture_shutter = capture_gain16_shutter / 16;
		if (capture_shutter < 1)
			capture_shutter = 1;

		capture_gain16 = capture_gain16_shutter / capture_shutter;
		if (capture_gain16 < 16) {
			/* printk("gain00\n"); */
			capture_gain16 = 16;
		}
	} else {
		/* printk("gain1\n"); */
		if (capture_gain16_shutter >
		    (capture_bandingfilter * capture_max_band * 16)) {
			/* exposure reach max
			 printk("gain10\n"); */
			capture_shutter =
			    capture_bandingfilter * capture_max_band;
			capture_gain16 =
			    capture_gain16_shutter / capture_shutter;
		} else {
			/* 1/100 < capture_shutter =< max,
			*  capture_shutter = n/100
			*  printk("gain11\n"); */
			capture_shutter =
			    (int)(capture_gain16_shutter / 16 /
				  capture_bandingfilter) *
			    capture_bandingfilter;
			capture_gain16 =
			    capture_gain16_shutter / capture_shutter;
		}
	}

	/* write capture gain */
	red_gain16 = red_gain16 * 94 / 100;
	green_gain16 = green_gain16 * 100 / 100;
	blue_gain16 = blue_gain16 * 96 / 100;
	ov5645_set_red_gain16(sd, red_gain16);
	ov5645_set_green_gain16(sd, green_gain16);
	ov5645_set_blue_gain16(sd, blue_gain16);

	ov5645_set_gain16(sd, capture_gain16);

	printk(KERN_INFO "%s capture_gain16=0x%x\n", __func__, capture_gain16);

	/* write capture shutter
	capture_shutter = capture_shutter * 122 / 100; */
	printk(KERN_INFO "%s shutter=0x%x, capture_VTS=0x%x\n", __func__,
	       capture_shutter, capture_VTS);
	if (capture_shutter > (capture_VTS - 4)) {
		capture_VTS = capture_shutter + 4;
		ov5645_set_VTS(sd, capture_VTS);
	}

	ov5645_set_shutter(sd, capture_shutter);

	return ret;
}

static int ov5645_flash_control(struct i2c_client *client, int control)
{
	int ret = 0;

	main_cam_ledcontrol((control & CAM_LED_ON),
		(control & CAM_LED_MODE_PRE));

	return ret;
}

static void flash_timer_callback(unsigned long data)
{
	gpio_set_value(CAM_FLASH_FLEN, 0);
	gpio_set_value(CAM_FLASH_ENSET, 0);
}


static int ov5645_pre_flash(struct i2c_client *client)
{
	int ret = 0;
	struct ov5645 *ov5645 = to_ov5645(client);

	ov5645->fireflash = 0;
	if (FLASH_MODE_ON == ov5645->flashmode) {
		ret = ov5645_flash_control(client, ov5645->flashmode);
		ov5645->fireflash = 1;
	} else if (FLASH_MODE_AUTO == ov5645->flashmode) {
		u8 average = 0;
		ov5645_reg_read(client, 0x56a1, &average);
		if ((average & 0xFF) < OV5645_FLASH_THRESHHOLD) {
			ret = ov5645_flash_control(client, FLASH_MODE_ON);
			ov5645->fireflash = 1;
		}
	}
	if (1 == ov5645->fireflash)
		msleep(50);

	if (1 == ov5645->fireflash)
		mod_timer(&flash_timer,
			jiffies + msecs_to_jiffies(FLASH_TIMEOUT_MS));

	return ret;
}

static int ov5645_af_start(struct i2c_client *client)
{
	int ret = 0;
	struct ov5645 *ov5645 = to_ov5645(client);

	iprintk("entry focus_mode %d", ov5645->focus_mode);

	if (ov5645->focus_mode == FOCUS_MODE_MACRO) {
		/*
		 * FIXME: Can the af_area be set before af_macro, or does
		 * this need to be inside the af_macro func?
		 ret = ov5645_af_area(client);
		 */
		ret = ov5645_af_macro(client);
	} else if (ov5645->focus_mode == FOCUS_MODE_INFINITY)
		ret = ov5645_af_infinity(client);
	else {
		if (ov5645->touch_focus) {
			if (ov5645->touch_focus == 1)
				ret = ov5645_af_touch(client);
			else
				ret = ov5645_af_area(client);
		} else
			ret = ov5645_af_center(client);
		if (ret)
			return ret;
		ret = ov5645_reg_write(client, OV5645_CMD_ACK, 0x01);
		if (ret)
			return ret;
		ret = ov5645_reg_write(client, OV5645_CMD_MAIN, 0x03);
		if (ret)
			return ret;
	}

	return ret;
}

static int ov5645_config_timing(struct i2c_client *client)
{
	struct ov5645 *ov5645 = to_ov5645(client);
	int ret, i = ov5645->i_size;
	const struct ov5645_timing_cfg *timing_cfg;

return 0;

	printk(KERN_INFO "%s: code[0x%x] i:%d\n", __func__,
	       ov5645_fmts[ov5645->i_fmt].code, i);

	if (ov5645_fmts[ov5645->i_fmt].code == V4L2_MBUS_FMT_JPEG_1X8)
		timing_cfg = &timing_cfg_jpeg[i];
	else
		timing_cfg = &timing_cfg_yuv[i];

	ret = ov5645_reg_write(client,
			       0x3800,
			       (timing_cfg->x_addr_start & 0xFF00) >> 8);
	if (ret)
		return ret;

	ret = ov5645_reg_write(client, 0x3801, timing_cfg->x_addr_start & 0xFF);
	if (ret)
		return ret;

	ret = ov5645_reg_write(client,
			       0x3802,
			       (timing_cfg->y_addr_start & 0xFF00) >> 8);
	if (ret)
		return ret;

	ret = ov5645_reg_write(client, 0x3803, timing_cfg->y_addr_start & 0xFF);
	if (ret)
		return ret;

	ret = ov5645_reg_write(client,
			       0x3804, (timing_cfg->x_addr_end & 0xFF00) >> 8);
	if (ret)
		return ret;

	ret = ov5645_reg_write(client, 0x3805, timing_cfg->x_addr_end & 0xFF);
	if (ret)
		return ret;

	ret = ov5645_reg_write(client,
			       0x3806, (timing_cfg->y_addr_end & 0xFF00) >> 8);
	if (ret)
		return ret;

	ret = ov5645_reg_write(client, 0x3807, timing_cfg->y_addr_end & 0xFF);
	if (ret)
		return ret;

	ret = ov5645_reg_write(client,
			       0x3808,
			       (timing_cfg->h_output_size & 0xFF00) >> 8);
	if (ret)
		return ret;

	ret = ov5645_reg_write(client,
			       0x3809, timing_cfg->h_output_size & 0xFF);
	if (ret)
		return ret;

	ret = ov5645_reg_write(client,
			       0x380A,
			       (timing_cfg->v_output_size & 0xFF00) >> 8);
	if (ret)
		return ret;

	ret = ov5645_reg_write(client,
			       0x380B, timing_cfg->v_output_size & 0xFF);
	if (ret)
		return ret;

	ret = ov5645_reg_write(client,
			       0x380C,
			       (timing_cfg->h_total_size & 0xFF00) >> 8);
	if (ret)
		return ret;

	ret = ov5645_reg_write(client, 0x380D, timing_cfg->h_total_size & 0xFF);
	if (ret)
		return ret;

	ret = ov5645_reg_write(client,
			       0x380E,
			       (timing_cfg->v_total_size & 0xFF00) >> 8);
	if (ret)
		return ret;

	ret = ov5645_reg_write(client, 0x380F, timing_cfg->v_total_size & 0xFF);
	if (ret)
		return ret;

	ret = ov5645_reg_write(client,
			       0x3810,
			       (timing_cfg->isp_h_offset & 0xFF00) >> 8);
	if (ret)
		return ret;

	ret = ov5645_reg_write(client, 0x3811, timing_cfg->isp_h_offset & 0xFF);
	if (ret)
		return ret;

	ret = ov5645_reg_write(client,
			       0x3812,
			       (timing_cfg->isp_v_offset & 0xFF00) >> 8);
	if (ret)
		return ret;

	ret = ov5645_reg_write(client, 0x3813, timing_cfg->isp_v_offset & 0xFF);
	if (ret)
		return ret;

	ret = ov5645_reg_write(client,
			       0x3814,
			       ((timing_cfg->h_odd_ss_inc & 0xF) << 4) |
			       (timing_cfg->h_even_ss_inc & 0xF));
	if (ret)
		return ret;

	ret = ov5645_reg_write(client,
			       0x3815,
			       ((timing_cfg->v_odd_ss_inc & 0xF) << 4) |
			       (timing_cfg->v_even_ss_inc & 0xF));
	if (ret)
		return ret;

	ret = ov5645_reg_write(client, 0x3821, timing_cfg->out_mode_sel & 0xFF);
	if (ret)
		return ret;

	ret = ov5645_reg_write(client,
			       0x3108, timing_cfg->sclk_dividers & 0xFF);
	if (ret)
		return ret;

	ret = ov5645_reg_write(client, 0x3035, timing_cfg->sys_mipi_clk & 0xFF);
	if (ret)
		return ret;

	/* msleep(50); */

	return ret;
}

static int stream_mode = -1;
static int ov5645_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
/*	struct ov5645 *ov5645 = to_ov5645(client); */
	int ret = 0;
	printk(KERN_INFO "%s: enable:%d runmode:%d  stream_mode:%d\n",
	       __func__, enable, runmode, stream_mode);

/*	u8 reg; */

	if (enable == stream_mode)
		return ret;
	if (enable) {
		ov5645_reg_write(client, 0x4202, 0x00);
		msleep(50);
	} else {
		ov5645_reg_write(client, 0x4202, 0x0f);
	}
	stream_mode = enable;

	return ret;
}

static int afFWLoaded = -1;
static int initNeeded = -1;

#if 0
static int ov5645_set_bus_param(struct soc_camera_device *icd,
				unsigned long flags)
{
	/* TODO: Do the right thing here, and validate bus params */
	return 0;
}


static unsigned long ov5645_query_bus_param(struct soc_camera_device *icd)
{
	unsigned long flags = SOCAM_PCLK_SAMPLE_FALLING |
	    SOCAM_HSYNC_ACTIVE_HIGH | SOCAM_VSYNC_ACTIVE_HIGH |
	    SOCAM_DATA_ACTIVE_HIGH | SOCAM_MASTER;

	/* TODO: Do the right thing here, and validate bus params */

	flags |= SOCAM_DATAWIDTH_10;

	return flags;
}

/* static int afFWLoaded = -1; */
/* static int initNeeded = -1; */


static int ov5645_enum_input(struct soc_camera_device *icd,
			     struct v4l2_input *inp)
{
	struct soc_camera_link *icl = to_soc_camera_link(icd);
	struct v4l2_subdev_sensor_interface_parms *plat_parms;

	inp->type = V4L2_INPUT_TYPE_CAMERA;
	inp->std = V4L2_STD_UNKNOWN;
	strcpy(inp->name, "ov5645");

	if (icl && icl->priv) {

		plat_parms = icl->priv;
		inp->status = 0;

		if (plat_parms->orientation == V4L2_SUBDEV_SENSOR_PORTRAIT)
			inp->status |= V4L2_IN_ST_HFLIP;

		if (plat_parms->facing == V4L2_SUBDEV_SENSOR_BACK)
			inp->status |= V4L2_IN_ST_BACK;

	}
	stream_mode = -1;
	afFWLoaded = -1;
	initNeeded = 1;

	return 0;
}

#endif

static int ov5645_g_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov5645 *ov5645 = to_ov5645(client);

	mf->width = ov5645_frmsizes[ov5645->i_size].width;
	mf->height = ov5645_frmsizes[ov5645->i_size].height;
	mf->code = ov5645_fmts[ov5645->i_fmt].code;
	mf->colorspace = ov5645_fmts[ov5645->i_fmt].colorspace;
	mf->field = V4L2_FIELD_NONE;

	return 0;
}

static int ov5645_try_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
	int i_fmt;
	int i_size;

	i_fmt = ov5645_find_datafmt(mf->code);

	mf->code = ov5645_fmts[i_fmt].code;
	mf->colorspace = ov5645_fmts[i_fmt].colorspace;
	mf->field = V4L2_FIELD_NONE;

	i_size = ov5645_find_framesize(mf->width, mf->height);

	mf->width = ov5645_frmsizes[i_size].width;
	mf->height = ov5645_frmsizes[i_size].height;

	return 0;
}

static int ov5645_s_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov5645 *ov5645 = to_ov5645(client);
	int ret = 0;
/*	u8 reg; */

	ret = ov5645_try_fmt(sd, mf);
	if (ret < 0)
		return ret;

	ov5645->i_size = ov5645_find_framesize(mf->width, mf->height);
	ov5645->i_fmt = ov5645_find_datafmt(mf->code);

	/*To avoide reentry init sensor, remove from here       */
	if (initNeeded > 0) {
		ret = ov5645_reg_writes(client, hawaii_common_init);
		initNeeded = 0;
	}
	if (ret) {
		printk(KERN_ERR "Error configuring hawaii_common_init\n");
		return ret;
	}
	printk(KERN_INFO "%s: code:0x%x fmt[%d]\n", __func__,
	       ov5645_fmts[ov5645->i_fmt].code, ov5645->i_size);

	if (CAM_RUNNING_MODE_PREVIEW == runmode)
		ov5645_config_preview(sd);

	return ret;
}

static int ov5645_g_chip_ident(struct v4l2_subdev *sd,
			       struct v4l2_dbg_chip_ident *id)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (id->match.type != V4L2_CHIP_MATCH_I2C_ADDR)
		return -EINVAL;

	if (id->match.addr != client->addr)
		return -ENODEV;

	id->ident = V4L2_IDENT_OV5645;
	id->revision = 0;

	return 0;
}

/*
 * return value of this function should be
 * 0 == CAMERA_AF_STATUS_FOCUSED
 * 1 == CAMERA_AF_STATUS_FAILED
 * 2 == CAMERA_AF_STATUS_SEARCHING
 * 3 == CAMERA_AF_STATUS_CANCELLED
 * to keep consistent with auto_focus_result
 * in videodev2_brcm.h
 */
static int ov5645_get_af_status(struct i2c_client *client, int num_trys)
{
	int ret = OV5645_AF_PENDING;
	struct ov5645 *ov5645 = to_ov5645(client);

	if (atomic_read(&ov5645->focus_status)
	    == OV5645_FOCUSING) {
		ret = ov5645_af_status(client, num_trys);
		/*
		 * convert OV5645_AF_* to auto_focus_result
		 * in videodev2_brcm
		 */
		switch (ret) {
		case OV5645_AF_SUCCESS:
			ret = CAMERA_AF_STATUS_FOCUSED;
			break;
		case OV5645_AF_PENDING:
			ret = CAMERA_AF_STATUS_SEARCHING;
			break;
		case OV5645_AF_FAIL:
			ret = CAMERA_AF_STATUS_FAILED;
			break;
		default:
			ret = CAMERA_AF_STATUS_SEARCHING;
			break;
		}
	}
	if (atomic_read(&ov5645->focus_status)
	    == OV5645_NOT_FOCUSING) {
		ret = CAMERA_AF_STATUS_CANCELLED;	/* cancelled? */
	}
	if ((CAMERA_AF_STATUS_FOCUSED == ret) ||
	    (CAMERA_AF_STATUS_FAILED == ret))
		atomic_set(&ov5645->focus_status, OV5645_NOT_FOCUSING);

	return ret;
}

static int ov5645_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov5645 *ov5645 = to_ov5645(client);

	dev_dbg(&client->dev, "ov5645_g_ctrl\n");

	switch (ctrl->id) {
	case V4L2_CID_CAMERA_BRIGHTNESS:
		ctrl->value = ov5645->brightness;
		break;
	case V4L2_CID_CAMERA_CONTRAST:
		ctrl->value = ov5645->contrast;
		break;
	case V4L2_CID_CAMERA_EFFECT:
		ctrl->value = ov5645->colorlevel;
		break;
	case V4L2_CID_SATURATION:
		ctrl->value = ov5645->saturation;
		break;
	case V4L2_CID_SHARPNESS:
		ctrl->value = ov5645->sharpness;
		break;
	case V4L2_CID_CAMERA_ANTI_BANDING:
		ctrl->value = ov5645->antibanding;
		break;
	case V4L2_CID_CAMERA_WHITE_BALANCE:
		ctrl->value = ov5645->whitebalance;
		break;
	case V4L2_CID_CAMERA_FRAME_RATE:
		ctrl->value = ov5645->framerate;
		break;
	case V4L2_CID_CAMERA_FOCUS_MODE:
		ctrl->value = ov5645->focus_mode;
		break;
	case V4L2_CID_CAMERA_TOUCH_AF_AREA:
		ctrl->value = ov5645->touch_focus;
		break;
	case V4L2_CID_CAMERA_AUTO_FOCUS_RESULT:
		/*
		 * this is called from another thread to read AF status
		 */
		ctrl->value = ov5645_get_af_status(client, 100);
		ov5645->touch_focus = 0;
		break;
	case V4L2_CID_CAMERA_FLASH_MODE:
		ctrl->value = ov5645->flashmode;
		break;
	case V4L2_CID_CAMERA_EXP_TIME:
		/* This is called to get the exposure values */
		ctrl->value = ov5645_get_exp_time(sd);
		break;
	}

	return 0;
}

static int ov5645_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov5645 *ov5645 = to_ov5645(client);
	u8 ov_reg;
	int ret = 0;

	dev_dbg(&client->dev, "ov5645_s_ctrl\n");

	switch (ctrl->id) {
	case V4L2_CID_CAMERA_BRIGHTNESS:

		if (ctrl->value > EV_PLUS_2)
			return -EINVAL;

		ov5645->brightness = ctrl->value;
		switch (ov5645->brightness) {
		case EV_MINUS_2:
			ov5645_reg_write(client, 0x3a0f, 0x26);
			ov5645_reg_write(client, 0x3a10, 0x1f);
			ov5645_reg_write(client, 0x3a11, 0x4c);
			ov5645_reg_write(client, 0x3a1b, 0x26);
			ov5645_reg_write(client, 0x3a1e, 0x1f);
			ov5645_reg_write(client, 0x3a1f, 0x10);
			break;
		case EV_MINUS_1:
			ov5645_reg_write(client, 0x3a0f, 0x30);
			ov5645_reg_write(client, 0x3a10, 0x28);
			ov5645_reg_write(client, 0x3a11, 0x60);
			ov5645_reg_write(client, 0x3a1b, 0x30);
			ov5645_reg_write(client, 0x3a1e, 0x28);
			ov5645_reg_write(client, 0x3a1f, 0x14);
			break;
		case EV_PLUS_1:
			ov5645_reg_write(client, 0x3a0f, 0x40);
			ov5645_reg_write(client, 0x3a10, 0x38);
			ov5645_reg_write(client, 0x3a11, 0x80);
			ov5645_reg_write(client, 0x3a1b, 0x40);
			ov5645_reg_write(client, 0x3a1e, 0x38);
			ov5645_reg_write(client, 0x3a1f, 0x1c);
			break;
		case EV_PLUS_2:
			ov5645_reg_write(client, 0x3a0f, 0x48);
			ov5645_reg_write(client, 0x3a10, 0x40);
			ov5645_reg_write(client, 0x3a11, 0x90);
			ov5645_reg_write(client, 0x3a1b, 0x48);
			ov5645_reg_write(client, 0x3a1e, 0x40);
			ov5645_reg_write(client, 0x3a1f, 0x20);
			break;
		default:
			ov5645_reg_write(client, 0x3a0f, 0x38);
			ov5645_reg_write(client, 0x3a10, 0x30);
			ov5645_reg_write(client, 0x3a11, 0x70);
			ov5645_reg_write(client, 0x3a1b, 0x38);
			ov5645_reg_write(client, 0x3a1e, 0x30);
			ov5645_reg_write(client, 0x3a1f, 0x18);
			break;
		}
		if (ret)
			return ret;
		break;
	case V4L2_CID_CAMERA_CONTRAST:

		if (ctrl->value > CONTRAST_PLUS_1)
			return -EINVAL;

		ov5645->contrast = ctrl->value;
		switch (ov5645->contrast) {
		case CONTRAST_MINUS_1:

			break;
		case CONTRAST_PLUS_1:

			break;
		default:

			break;
		}
		if (ret)
			return ret;
		break;
	case V4L2_CID_CAMERA_EFFECT:

		if (ctrl->value > IMAGE_EFFECT_BNW)
			return -EINVAL;

		ov5645->colorlevel = ctrl->value;

		switch (ov5645->colorlevel) {
		case IMAGE_EFFECT_BNW:
			ov5645_reg_write(client, 0x3212, 0x03);
			ov5645_reg_write(client, 0x5580, 0x1e);
			ov5645_reg_write(client, 0x5583, 0x80);
			ov5645_reg_write(client, 0x5584, 0x80);
			ov5645_reg_write(client, 0x5003, 0x08);
			ov5645_reg_write(client, 0x3212, 0x13);
			ov5645_reg_write(client, 0x3212, 0xa3);
			break;
		case IMAGE_EFFECT_SEPIA:
			ov5645_reg_write(client, 0x3212, 0x03);
			ov5645_reg_write(client, 0x5580, 0x1e);
			ov5645_reg_write(client, 0x5583, 0x40);
			ov5645_reg_write(client, 0x5584, 0xa0);
			ov5645_reg_write(client, 0x5003, 0x08);
			ov5645_reg_write(client, 0x3212, 0x13);
			ov5645_reg_write(client, 0x3212, 0xa3);
			break;
		case IMAGE_EFFECT_NEGATIVE:
			ov5645_reg_write(client, 0x3212, 0x03);
			ov5645_reg_write(client, 0x5580, 0x46);
			ov5645_reg_write(client, 0x5583, 0x40);
			ov5645_reg_write(client, 0x5584, 0x30);
			ov5645_reg_write(client, 0x5003, 0x08);
			ov5645_reg_write(client, 0x3212, 0x13);
			ov5645_reg_write(client, 0x3212, 0xa3);
			break;
		default:
			ov5645_reg_write(client, 0x3212, 0x03);
			ov5645_reg_write(client, 0x5580, 0x06);
			ov5645_reg_write(client, 0x5583, 0x40);
			ov5645_reg_write(client, 0x5584, 0x30);
			ov5645_reg_write(client, 0x5003, 0x08);
			ov5645_reg_write(client, 0x3212, 0x13);
			ov5645_reg_write(client, 0x3212, 0xa3);
			break;
		}
		msleep(50);

		break;
	case V4L2_CID_SATURATION:

		if (ctrl->value > OV5645_SATURATION_MAX)
			return -EINVAL;

		ov5645->saturation = ctrl->value;
		switch (ov5645->saturation) {
		case OV5645_SATURATION_MIN:
			break;
		case OV5645_SATURATION_MAX:
			break;
		default:
			break;
		}
		if (ret)
			return ret;
		break;
	case V4L2_CID_SHARPNESS:

		if (ctrl->value > OV5645_SHARPNESS_MAX)
			return -EINVAL;

		ov5645->sharpness = ctrl->value;
		switch (ov5645->sharpness) {
		case OV5645_SHARPNESS_MIN:
			break;
		case OV5645_SHARPNESS_MAX:
			break;
		default:
			break;
		}
		if (ret)
			return ret;
		break;

	case V4L2_CID_CAMERA_ANTI_BANDING:

		if (ctrl->value > ANTI_BANDING_60HZ)
			return -EINVAL;

		ov5645->antibanding = ctrl->value;

		switch (ov5645->antibanding) {
		case ANTI_BANDING_50HZ:
			ov5645_reg_write(client, 0x3c00, 0x04);
			ov5645_reg_write(client, 0x3c01, 0xb4);

			ov5645_reg_read(client, 0x3a00, &ov_reg);
			ov_reg = ov_reg & 0x20;
			ov5645_reg_write(client, 0x3a00, ov_reg);

			break;
		case ANTI_BANDING_60HZ:
			ov5645_reg_write(client, 0x3c00, 0x00);
			ov5645_reg_write(client, 0x3c01, 0xb4);

			ov5645_reg_read(client, 0x3a00, &ov_reg);
			ov_reg = ov_reg & 0x20;
			ov5645_reg_write(client, 0x3a00, ov_reg);

			break;
		default:
			ov5645_reg_write(client, 0x3c01, 0x34);

			ov5645_reg_read(client, 0x3a00, &ov_reg);
			ov_reg = ov_reg & 0x20;
			ov5645_reg_write(client, 0x3a00, ov_reg);

			break;
		}
		if (ret)
			return ret;
		break;

	case V4L2_CID_CAMERA_WHITE_BALANCE:

		if (ctrl->value > WHITE_BALANCE_FLUORESCENT)
			return -EINVAL;

		ov5645->whitebalance = ctrl->value;

		switch (ov5645->whitebalance) {
		case WHITE_BALANCE_FLUORESCENT:
			ov5645_reg_write(client, 0x3212, 0x03);
			ov5645_reg_write(client, 0x3406, 0x01);
			ov5645_reg_write(client, 0x3400, 0x05);
			ov5645_reg_write(client, 0x3401, 0x48);
			ov5645_reg_write(client, 0x3402, 0x04);
			ov5645_reg_write(client, 0x3403, 0x00);
			ov5645_reg_write(client, 0x3404, 0x07);
			ov5645_reg_write(client, 0x3405, 0xcf);
			ov5645_reg_write(client, 0x3212, 0x13);
			ov5645_reg_write(client, 0x3212, 0xa3);
			break;
		case WHITE_BALANCE_SUNNY:
			ov5645_reg_write(client, 0x3212, 0x03);
			ov5645_reg_write(client, 0x3406, 0x01);
			ov5645_reg_write(client, 0x3400, 0x06);
			ov5645_reg_write(client, 0x3401, 0x1c);
			ov5645_reg_write(client, 0x3402, 0x04);
			ov5645_reg_write(client, 0x3403, 0x00);
			ov5645_reg_write(client, 0x3404, 0x04);
			ov5645_reg_write(client, 0x3405, 0xf3);
			ov5645_reg_write(client, 0x3212, 0x13);
			ov5645_reg_write(client, 0x3212, 0xa3);
			break;
		case WHITE_BALANCE_CLOUDY:
			ov5645_reg_write(client, 0x3212, 0x03);
			ov5645_reg_write(client, 0x3406, 0x01);
			ov5645_reg_write(client, 0x3400, 0x06);
			ov5645_reg_write(client, 0x3401, 0x48);
			ov5645_reg_write(client, 0x3402, 0x04);
			ov5645_reg_write(client, 0x3403, 0x00);
			ov5645_reg_write(client, 0x3404, 0x04);
			ov5645_reg_write(client, 0x3405, 0xd3);
			ov5645_reg_write(client, 0x3212, 0x13);
			ov5645_reg_write(client, 0x3212, 0xa3);
			break;
		case WHITE_BALANCE_TUNGSTEN:
			ov5645_reg_write(client, 0x3212, 0x03);
			ov5645_reg_write(client, 0x3406, 0x01);
			ov5645_reg_write(client, 0x3400, 0x04);
			ov5645_reg_write(client, 0x3401, 0x10);
			ov5645_reg_write(client, 0x3402, 0x04);
			ov5645_reg_write(client, 0x3403, 0x00);
			ov5645_reg_write(client, 0x3404, 0x08);
			ov5645_reg_write(client, 0x3405, 0x40);
			ov5645_reg_write(client, 0x3212, 0x13);
			ov5645_reg_write(client, 0x3212, 0xa3);
			break;
		default:  /*auto*/
			ov5645_reg_write(client, 0x3212, 0x03);
			ov5645_reg_write(client, 0x3406, 0x00);
			ov5645_reg_write(client, 0x3400, 0x04);
			ov5645_reg_write(client, 0x3401, 0x00);
			ov5645_reg_write(client, 0x3402, 0x04);
			ov5645_reg_write(client, 0x3403, 0x00);
			ov5645_reg_write(client, 0x3404, 0x04);
			ov5645_reg_write(client, 0x3405, 0x00);
			ov5645_reg_write(client, 0x3212, 0x13);
			ov5645_reg_write(client, 0x3212, 0xa3);
			break;
		}
		msleep(50);

		break;

	case V4L2_CID_CAMERA_FRAME_RATE:

		if (ctrl->value > FRAME_RATE_30)
			return -EINVAL;

		if ((ov5645->i_size < OV5645_SIZE_QVGA) ||
		    (ov5645->i_size > OV5645_SIZE_1280x960)) {
			if (ctrl->value == FRAME_RATE_30 ||
			    ctrl->value == FRAME_RATE_AUTO)
				return 0;
			else
				return -EINVAL;
		}

		ov5645->framerate = ctrl->value;
printk(KERN_INFO "ov5645 framerate = %d  ", ov5645->framerate);
		ov5645_reg_write(client, 0x4202, 0x0f);
		ov5645_reg_write(client, 0x3503, 0x03);
		msleep(50);
		switch (ov5645->framerate) {
		case FRAME_RATE_5:
			ret = ov5645_reg_writes(client, ov5645_fps_5);
			break;
		case FRAME_RATE_7:
			ret = ov5645_reg_writes(client, ov5645_fps_7);
			break;
		case FRAME_RATE_10:
			ret = ov5645_reg_writes(client, ov5645_fps_10);
			break;
		case FRAME_RATE_15:
			ret = ov5645_reg_writes(client, ov5645_fps_15);
			break;
		case FRAME_RATE_20:
			ret = ov5645_reg_writes(client, ov5645_fps_20);
			break;
		case FRAME_RATE_25:
			ret = ov5645_reg_writes(client, ov5645_fps_25);
			break;
		case FRAME_RATE_30:
			ret = ov5645_reg_writes(client, ov5645_fps_30);
		case FRAME_RATE_AUTO:
		default:
			break;
		}
		msleep(100);
		ov5645_reg_write(client, 0x4202, 0x00);
		if (ret)
			return ret;
		break;

	case V4L2_CID_CAMERA_FOCUS_MODE:

		if (ctrl->value > FOCUS_MODE_INFINITY)
			return -EINVAL;

		ov5645->focus_mode = ctrl->value;

		iprintk("set focus_mode %d", ov5645->focus_mode);

		/*
		 * Donot start the AF cycle here
		 * AF Start will be called later in
		 * V4L2_CID_CAMERA_SET_AUTO_FOCUS only for auto, macro mode
		 * it wont be called for infinity.
		 * Donot worry about resolution change for now.
		 * From userspace we set the resolution first
		 * and then set the focus mode.
		 */
		switch (ov5645->focus_mode) {
		case FOCUS_MODE_MACRO:
			/*
			 * set the table for macro mode
			 */
			break;
		case FOCUS_MODE_INFINITY:
			/*
			 * set the table for infinity
			 */
			ret = 0;
			break;
		default:
			ret = 0;
			break;
		}

		if (ret)
			return ret;
		break;

	case V4L2_CID_CAMERA_TOUCH_AF_AREA:


		if (ov5645->touch_focus < OV5645_MAX_FOCUS_AREAS) {
			v4l2_touch_area touch_area;
			if (copy_from_user(&touch_area,
					   (v4l2_touch_area *) ctrl->value,
					   sizeof(v4l2_touch_area)))
				return -EINVAL;

			iprintk("z=%d x=0x%x y=0x%x w=0x%x h=0x%x weight=0x%x",
				ov5645->touch_focus, touch_area.leftTopX,
				touch_area.leftTopY, touch_area.rightBottomX,
				touch_area.rightBottomY, touch_area.weight);

			ret = ov5645_af_zone_conv(client, &touch_area,
						  ov5645->touch_focus);
			if (ret == 0)
				ov5645->touch_focus++;
			ret = 0;

		} else
			dev_dbg(&client->dev,
				"Maximum touch focus areas already set\n");

		break;

	case V4L2_CID_CAMERA_SET_AUTO_FOCUS:

		if (ctrl->value > AUTO_FOCUS_ON)
			return -EINVAL;

		/* start and stop af cycle here */
		switch (ctrl->value) {

		case AUTO_FOCUS_OFF:

			if (atomic_read(&ov5645->focus_status)
			    == OV5645_FOCUSING) {
				ret = ov5645_af_release(client);
				atomic_set(&ov5645->focus_status,
					   OV5645_NOT_FOCUSING);
			}
			ov5645->touch_focus = 0;
			break;

		case AUTO_FOCUS_ON:
			if (1 != afFWLoaded) {
				ret = ov5645_af_enable(client);
				if (ret)
					return ret;
				afFWLoaded = 1;
			}
			/* check if preflash is needed */
			ret = ov5645_pre_flash(client);

			ret = ov5645_af_start(client);
			atomic_set(&ov5645->focus_status, OV5645_FOCUSING);
			break;

		}

		if (ret)
			return ret;
		break;
	case V4L2_CID_CAMERA_FLASH_MODE:
		ov5645_set_flash_mode(ctrl->value, client);
		break;

	case V4L2_CID_CAM_PREVIEW_ONOFF:
		{
			printk(KERN_INFO
			       "ov5645 PREVIEW_ONOFF:%d runmode = %d\n",
			       ctrl->value, runmode);
			if (ctrl->value)
				runmode = CAM_RUNNING_MODE_PREVIEW;
			else
				runmode = CAM_RUNNING_MODE_NOTREADY;
			break;
		}

	case V4L2_CID_CAM_CAPTURE:
		printk(KERN_INFO "ov5645 runmode = capture\n");
		runmode = CAM_RUNNING_MODE_CAPTURE;
		if (ov5645->fireflash) {
			ov5645_flash_control(client, FLASH_MODE_ON);
			msleep(50);

			mod_timer(&flash_timer,
				jiffies + msecs_to_jiffies(FLASH_TIMEOUT_MS));

		}
		ov5645_config_capture(sd);
		break;

	case V4L2_CID_CAM_CAPTURE_DONE:
		printk(KERN_INFO "ov5645 runmode = capture_done\n");
		runmode = CAM_RUNNING_MODE_CAPTURE_DONE;
		break;

	}

	return ret;
}

static int ov5645_set_flash_mode(int mode, struct i2c_client *client)
{
	int ret = 0;
	struct ov5645 *ov5645 = to_ov5645(client);

	switch (mode) {
	case FLASH_MODE_ON:
		ov5645->flashmode = mode;
		break;
	case FLASH_MODE_AUTO:
		ov5645->flashmode = mode;
		break;
	case FLASH_MODE_TORCH_ON:
	case FLASH_MODE_TORCH_OFF:
		ov5645_flash_control(client, mode);
		break;
	case FLASH_MODE_OFF:
	default:
		ov5645_flash_control(client, mode);
		ov5645->flashmode = mode;
		break;
	}

	return ret;
}

#if 0
static int flash_gpio_strobe(int on)
{
#ifdef CONFIG_VIDEO_ADP1653
	return adp1653_gpio_strobe(on);
#else
	return 0;
#endif
}
#endif
static long ov5645_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	int ret = 0;

	switch (cmd) {
	case VIDIOC_THUMB_SUPPORTED:
		{
			int *p = arg;
			*p = 0;	/* no we don't support thumbnail */
			break;
		}
	case VIDIOC_JPEG_G_PACKET_INFO:
		{
			struct v4l2_jpeg_packet_info *p =
			    (struct v4l2_jpeg_packet_info *)arg;
			p->padded = 0;
			p->packet_size = 0x400;
			break;
		}

	case VIDIOC_SENSOR_G_OPTICAL_INFO:
		{
			struct v4l2_sensor_optical_info *p =
			    (struct v4l2_sensor_optical_info *)arg;
			/* assuming 67.5 degree diagonal viewing angle */
			p->hor_angle.numerator = 5401;
			p->hor_angle.denominator = 100;
			p->ver_angle.numerator = 3608;
			p->ver_angle.denominator = 100;
			p->focus_distance[0] = 10;	/* near focus in cm */
			p->focus_distance[1] = 100;	/* optimal focus
							in cm */
			p->focus_distance[2] = -1;	/* infinity */
			p->focal_length.numerator = 342;
			p->focal_length.denominator = 100;
			break;
		}
	default:
		ret = -ENOIOCTLCMD;
		break;
	}
	return ret;
}


static int ov5645_queryctrl(struct v4l2_subdev *sd,
		struct v4l2_queryctrl *qc)
{

	int index = 0;

	for (index = 0; index < ARRAY_SIZE(ov5645_controls); index++) {
		if ((qc->id) == (ov5645_controls[index].id)) {
			qc->type = ov5645_controls[index].type;
			qc->minimum = ov5645_controls[index].minimum;
			qc->maximum = ov5645_controls[index].maximum;
			qc->step = ov5645_controls[index].step;
			qc->default_value =
				ov5645_controls[index].default_value;
			qc->flags = ov5645_controls[index].flags;
			strlcpy(qc->name, ov5645_controls[index].name,
				sizeof(qc->name));
			return 0;
		}
	}

	return -EINVAL;
}

#ifdef CONFIG_VIDEO_ADV_DEBUG
static int ov5645_g_register(struct v4l2_subdev *sd,
				struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (reg->match.type != V4L2_CHIP_MATCH_I2C_ADDR || reg->size > 2)
		return -EINVAL;

	if (reg->match.addr != client->addr)
		return -ENODEV;

	reg->size = 2;
	if (ov5645_reg_read(client, reg->reg, &reg->val))
		return -EIO return 0;
}

static int ov5645_s_register(struct v4l2_subdev *sd,
			     struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (reg->match.type != V4L2_CHIP_MATCH_I2C_ADDR || reg->size > 2)
		return -EINVAL;

	if (reg->match.addr != client->addr)
		return -ENODEV;

	if (ov5645_reg_write(client, reg->reg, reg->val))
		return -EIO;

	return 0;
}
#endif


/* The below needs to be discussed as this is added as a new functions
   currently commenting as it has needs the member ops in soc camera
	device stucture*/
/*static struct soc_camera_ops ov5645_ops = {
	.set_bus_param = ov5645_set_bus_param,
	.query_bus_param = ov5645_query_bus_param,
	.enum_input = ov5645_enum_input,
	.controls = ov5645_controls,
	.num_controls = ARRAY_SIZE(ov5645_controls),
};*/

static int ov5645_init(struct i2c_client *client)
{
	struct ov5645 *ov5645 = to_ov5645(client);
	int ret = 0;

	ret = ov5645_reg_writes(client, hawaii_common_init);
	if (ret)
		goto out;

#if defined(CONFIG_RHEA_CLOVER_ICS)
	/*Code turn off flash led */
	if (ov5645_reg_write(client, 0x3000, 0x00))
		goto out;
	if (ov5645_reg_write(client, 0x3004, 0xFF))
		goto out;
	if (ov5645_reg_write(client, 0x3016, 0x02))
		goto out;
	if (ov5645_reg_write(client, 0x3b07, 0x0A))
		goto out;
	if (ov5645_reg_write(client, 0x3b00, 0x03))
		goto out;
#endif

	/* Power Up, Start Streaming for AF Init */
	/* default brightness and contrast */
	ov5645->brightness = EV_DEFAULT;
	ov5645->contrast = CONTRAST_DEFAULT;
	ov5645->colorlevel = IMAGE_EFFECT_NONE;
	ov5645->antibanding = ANTI_BANDING_AUTO;
	ov5645->whitebalance = WHITE_BALANCE_AUTO;
	ov5645->framerate = FRAME_RATE_AUTO;
	ov5645->focus_mode = FOCUS_MODE_AUTO;
	ov5645->touch_focus = 0;
	atomic_set(&ov5645->focus_status, OV5645_NOT_FOCUSING);
	ov5645->flashmode = FLASH_MODE_OFF;
	ov5645->fireflash = 0;

	dev_dbg(&client->dev, "Sensor initialized\n");

out:
	return ret;
}

/*
 * Interface active, can use i2c. If it fails, it can indeed mean, that
 *this wasn't our capture interface, so, we wait for the right one
 */
static int ov5645_video_probe(struct soc_camera_device *icd,
			      struct i2c_client *client)
{
/*	unsigned long flags; */
	int ret = 0;
	u8 id_high, id_low;

	/*
	 * We must have a parent by now. And it cannot be a wrong one.
	 * So this entire test is completely redundant.
	 */
	#if 0
	if (!icd->dev.parent ||
	    to_soc_camera_host(icd->dev.parent)->nr != icd->iface)
		return -ENODEV;
    #endif

	ret = ov5645_reg_read(client, OV5645_CHIP_ID_HIGH, &id_high);
	ret += ov5645_reg_read(client, OV5645_CHIP_ID_LOW, &id_low);

	printk(KERN_INFO "OV5645 ID=0x%x%x\n", id_high, id_low);

	return ret;
}

static void ov5645_video_remove(struct soc_camera_device *icd)
{
	/*dev_dbg(&icd->dev, "Video removed: %p, %p\n",
		icd->dev.parent, icd->vdev);*/
}

static struct v4l2_subdev_core_ops ov5645_subdev_core_ops = {
	.g_chip_ident = ov5645_g_chip_ident,
	.g_ctrl = ov5645_g_ctrl,
	.s_ctrl = ov5645_s_ctrl,
	.ioctl = ov5645_ioctl,
	.queryctrl = ov5645_queryctrl,
#ifdef CONFIG_VIDEO_ADV_DEBUG
	.g_register = ov5645_g_register,
	.s_register = ov5645_s_register,
#endif
};

static int ov5645_enum_fmt(struct v4l2_subdev *sd, unsigned int index,
			   enum v4l2_mbus_pixelcode *code)
{
	if (index >= ARRAY_SIZE(ov5645_fmts))
		return -EINVAL;

	*code = ov5645_fmts[index].code;
	return 0;
}

static int ov5645_enum_framesizes(struct v4l2_subdev *sd,
				  struct v4l2_frmsizeenum *fsize)
{
	if (fsize->index >= OV5645_SIZE_LAST)
		return -EINVAL;

	fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
	fsize->pixel_format = V4L2_PIX_FMT_UYVY;

	fsize->discrete = ov5645_frmsizes[fsize->index];

	return 0;
}

/* we only support fixed frame rate */
static int ov5645_enum_frameintervals(struct v4l2_subdev *sd,
				      struct v4l2_frmivalenum *interval)
{
	int size;

	if (interval->index >= 1)
		return -EINVAL;

	interval->type = V4L2_FRMIVAL_TYPE_DISCRETE;

	size = ov5645_find_framesize(interval->width, interval->height);

	switch (size) {
	case OV5645_SIZE_5MP:
		interval->discrete.numerator = 2;
		interval->discrete.denominator = 15;
		break;
	case OV5645_SIZE_QXGA:
	case OV5645_SIZE_UXGA:
		interval->discrete.numerator = 1;
		interval->discrete.denominator = 15;
		break;
	case OV5645_SIZE_720P:
		interval->discrete.numerator = 1;
		interval->discrete.denominator = 0;
		break;
	case OV5645_SIZE_VGA:
	case OV5645_SIZE_QVGA:
	case OV5645_SIZE_1280x960:
	default:
		interval->discrete.numerator = 1;
		interval->discrete.denominator = 24;
		break;
	}
/*	printk(KERN_ERR"%s: width=%d height=%d fi=%d/%d\n", __func__,
			interval->width,
			interval->height, interval->discrete.numerator,
			interval->discrete.denominator);
			*/
	return 0;
}

static int ov5645_g_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov5645 *ov5645 = to_ov5645(client);
	struct v4l2_captureparm *cparm;

	if (param->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	cparm = &param->parm.capture;

	memset(param, 0, sizeof(*param));
	param->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	cparm->capability = V4L2_CAP_TIMEPERFRAME;

	switch (ov5645->i_size) {
	case OV5645_SIZE_5MP:
		cparm->timeperframe.numerator = 2;
		cparm->timeperframe.denominator = 15;
		break;
	case OV5645_SIZE_QXGA:
	case OV5645_SIZE_UXGA:
		cparm->timeperframe.numerator = 1;
		cparm->timeperframe.denominator = 15;
		break;
	case OV5645_SIZE_1280x960:
	case OV5645_SIZE_720P:
	case OV5645_SIZE_VGA:
	case OV5645_SIZE_QVGA:
	default:
		cparm->timeperframe.numerator = 1;
		cparm->timeperframe.denominator = 24;
		break;
	}

	return 0;
}

static int ov5645_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	/*
	 * FIXME: This just enforces the hardcoded framerates until this is
	 *flexible enough.
	 */
	return ov5645_g_parm(sd, param);
}

static struct v4l2_subdev_video_ops ov5645_subdev_video_ops = {
	.s_stream = ov5645_s_stream,
	.s_mbus_fmt = ov5645_s_fmt,
	.g_mbus_fmt = ov5645_g_fmt,
	.try_mbus_fmt = ov5645_try_fmt,
	.enum_mbus_fmt = ov5645_enum_fmt,
	.enum_mbus_fsizes = ov5645_enum_framesizes,
	.enum_framesizes = ov5645_enum_framesizes,
	.enum_frameintervals = ov5645_enum_frameintervals,
	.g_parm = ov5645_g_parm,
	.s_parm = ov5645_s_parm,
};

static int ov5645_g_skip_frames(struct v4l2_subdev *sd, u32 *frames)
{
	/* Quantity of initial bad frames to skip. Revisit. */
	/*Waitting for AWB stability,  avoid green color issue */
	*frames = 3;

	return 0;
}

#if 0
static int ov5645_g_interface_parms(struct v4l2_subdev *sd,
			struct v4l2_subdev_sensor_interface_parms *parms)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov5645 *ov5645 = to_ov5645(client);
	u8 sclk_dividers;

	if (!parms)
		return -EINVAL;

	parms->if_type = ov5645->plat_parms->if_type;
	parms->if_mode = ov5645->plat_parms->if_mode;
	parms->parms = ov5645->plat_parms->parms;

	parms->parms.serial.hs_term_time = 0x0c;
	parms->parms.serial.hs_settle_time = 2;

#if 0
	/* set the hs term time */
	if (ov5645_fmts[ov5645->i_fmt].code == V4L2_MBUS_FMT_JPEG_1X8)
		sclk_dividers = timing_cfg_jpeg[ov5645->i_size].sclk_dividers;
	else
		sclk_dividers = timing_cfg_yuv[ov5645->i_size].sclk_dividers;

	if (sclk_dividers == 0x01)
		parms->parms.serial.hs_term_time = 0x01;
	else
		parms->parms.serial.hs_term_time = 0x08;



	switch (ov5645->framerate) {
	case FRAME_RATE_5:
		parms->parms.serial.hs_settle_time = 9;
		break;
	case FRAME_RATE_7:
		parms->parms.serial.hs_settle_time = 6;
		break;
	case FRAME_RATE_10:
	case FRAME_RATE_15:
	case FRAME_RATE_25:
	case FRAME_RATE_30:
	case FRAME_RATE_AUTO:
	default:
		parms->parms.serial.hs_settle_time = 2;
		break;
	}
#endif

	return 0;
}

#endif

static struct v4l2_subdev_sensor_ops ov5645_subdev_sensor_ops = {
	.g_skip_frames = ov5645_g_skip_frames,
	/*.g_interface_parms = ov5645_g_interface_parms, */
};

static struct v4l2_subdev_ops ov5645_subdev_ops = {
	.core = &ov5645_subdev_core_ops,
	.video = &ov5645_subdev_video_ops,
	.sensor = &ov5645_subdev_sensor_ops,
};

static int ov5645_probe(struct i2c_client *client,
			const struct i2c_device_id *did)
{

	struct ov5645 *ov5645;
	struct soc_camera_device *icd = NULL;
	struct soc_camera_link *icl = client->dev.platform_data;
	int ret;

/*
	if (!icd) {
		dev_err(&client->dev, "OV5645: missing soc-camera data!\n");
		return -EINVAL;
	}

	icl = to_soc_camera_link(icd);
*/
	if (!icl) {
		dev_err(&client->dev, "OV5645 driver needs platform data\n");
		return -EINVAL;
	}


	if (!icl->priv) {
		dev_err(&client->dev,
			"OV5645 driver needs i/f platform data\n");
		return -EINVAL;
	}

	ov5645 = kzalloc(sizeof(struct ov5645), GFP_KERNEL);
	if (!ov5645)
		return -ENOMEM;

	v4l2_i2c_subdev_init(&ov5645->subdev, client, &ov5645_subdev_ops);

	/* Second stage probe - when a capture adapter is there */
	/* icd->ops = &ov5645_ops; */

	ov5645->i_size = OV5645_SIZE_VGA;
	ov5645->i_fmt = 0;	/* First format in the list */
	ov5645->plat_parms = icl->priv;

	ret = ov5645_video_probe(icd, client);
	if (ret) {
		/* icd->ops = NULL; */
		kfree(ov5645);
		return ret;
	}

	/* init the sensor here */
	ret = ov5645_init(client);
	if (ret) {
		dev_err(&client->dev, "Failed to initialize sensor\n");
		ret = -EINVAL;
	}

	init_timer(&flash_timer);
	setup_timer(&flash_timer, flash_timer_callback, 0);

	return ret;
}

static int ov5645_remove(struct i2c_client *client)
{
	struct ov5645 *ov5645 = to_ov5645(client);
	struct soc_camera_device *icd = client->dev.platform_data;

	/* icd->ops = NULL; */
	ov5645_video_remove(icd);
	client->driver = NULL;
	kfree(ov5645);

	return 0;
}

static const struct i2c_device_id ov5645_id[] = {
	{"OV5645", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, ov5645_id);

static struct i2c_driver ov5645_i2c_driver = {
	.driver = {
		   .name = "OV5645",
		   },
	.probe = ov5645_probe,
	.remove = ov5645_remove,
	.id_table = ov5645_id,
};

static int __init ov5645_mod_init(void)
{
	return i2c_add_driver(&ov5645_i2c_driver);
}

static void __exit ov5645_mod_exit(void)
{
	i2c_del_driver(&ov5645_i2c_driver);
}

module_init(ov5645_mod_init);
module_exit(ov5645_mod_exit);

MODULE_DESCRIPTION("OmniVision OV5645 Camera driver");
MODULE_AUTHOR("Sergio Aguirre <saaguirre@ti.com>");
MODULE_LICENSE("GPL v2");
