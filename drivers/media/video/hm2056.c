/*
 * OmniVision hm2056 sensor driver
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
#include <mach/gpio.h>
#include <linux/i2c.h>
#include <linux/log2.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <media/v4l2-subdev.h>
#include <media/v4l2-chip-ident.h>
#include <media/soc_camera.h>
#include <media/sh_mobile_csi2.h>
#include <linux/videodev2_brcm.h>
#include "hm2056.h"

#ifdef CONFIG_VIDEO_ADP1653
#include "adp1653.h"
#endif

#ifdef CONFIG_VIDEO_AS3643
#include "as3643.h"
#endif

/* #define hm2056_DEBUG */

#define iprintk(format, arg...)	\
	printk(KERN_INFO"[%s]: "format"\n", __func__, ##arg)

static int initialized;

/* hm2056 has only one fixed colorspace per pixelcode */
struct hm2056_datafmt {
	enum v4l2_mbus_pixelcode code;
	enum v4l2_colorspace colorspace;
};

struct hm2056_timing_cfg {
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


static const struct hm2056_datafmt hm2056_fmts[] = {
	/*
	 * Order important: first natively supported,
	 *second supported with a GPIO extender
	 */
	{V4L2_MBUS_FMT_UYVY8_2X8, V4L2_COLORSPACE_JPEG},
	{V4L2_MBUS_FMT_YUYV8_2X8, V4L2_COLORSPACE_JPEG},
/*	{V4L2_MBUS_FMT_JPEG_1X8, V4L2_COLORSPACE_JPEG}, */

};

enum hm2056_size {
	hm2056_SIZE_QVGA,	/*  320 x 240 */
	hm2056_SIZE_VGA,	/*  640 x 480 */
	hm2056_SIZE_1280x1024,	/*  1280 x 960 (1.2M) */
	hm2056_SIZE_UXGA,	/*  1600 x 1200 (2M) */
	hm2056_SIZE_LAST,
	hm2056_SIZE_MAX
};

static enum cam_running_mode {
	CAM_RUNNING_MODE_NOTREADY,
	CAM_RUNNING_MODE_PREVIEW,
	CAM_RUNNING_MODE_CAPTURE,
	CAM_RUNNING_MODE_CAPTURE_DONE,
	CAM_RUNNING_MODE_RECORDING,
} runmode;

static const struct v4l2_frmsize_discrete hm2056_frmsizes[hm2056_SIZE_LAST] = {
	{320, 240},
	{640, 480},
	{1280, 1024},
	{1600, 1200},
};

/* Power function for HM2056 */
int HM2056_power(struct device *dev, int power_on)
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


/* Find a data format by a pixel code in an array */
static int hm2056_find_datafmt(enum v4l2_mbus_pixelcode code)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(hm2056_fmts); i++)
		if (hm2056_fmts[i].code == code)
			break;

	/* If not found, select latest */
	if (i >= ARRAY_SIZE(hm2056_fmts))
		i = ARRAY_SIZE(hm2056_fmts) - 1;

	return i;
}

/* Find a frame size in an array */
static int hm2056_find_framesize(u32 width, u32 height)
{
	int i;

	for (i = 0; i < hm2056_SIZE_LAST; i++) {
		if ((hm2056_frmsizes[i].width >= width) &&
		    (hm2056_frmsizes[i].height >= height))
			break;
	}

	/* If not found, select biggest */
	if (i >= hm2056_SIZE_LAST)
		i = hm2056_SIZE_LAST - 1;

	return i;
}

struct hm2056 {
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
	int flashmode;
};

static struct hm2056 *to_hm2056(const struct i2c_client *client)
{
	return container_of(i2c_get_clientdata(client), struct hm2056, subdev);
}

static const struct hm2056_timing_cfg timing_cfg_yuv[hm2056_SIZE_LAST] = {
	[hm2056_SIZE_QVGA] = {
			/*  Timing control  2624 x 1952 --> 2592 x 1944 */
			      .x_addr_start = 16,
			      .y_addr_start = 4,
			      .x_addr_end = 2607,
			      .y_addr_end = 1947,
			/*  Output image size */
			      .h_output_size = 320,
			      .v_output_size = 240,
			/*  ISP Windowing size 1296 x 972 --> 1280 x 960 */
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
				  .out_mode_sel = 0x01,
#else
			      .out_mode_sel = 0x07,
#endif
			      .sclk_dividers = 0x01,
			      .sys_mipi_clk = 0x11,
			       },
	[hm2056_SIZE_VGA] = {
			/*  Timing control  2624 x 1952 --> 2592 x 1944 */
			      .x_addr_start = 16,
			      .y_addr_start = 4,
			      .x_addr_end = 2607,
			      .y_addr_end = 1947,
			/*  Output image size */
			      .h_output_size = 640,
			      .v_output_size = 480,
			/*  ISP Windowing size  1296 x 972 --> 1280 x 960 */
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
				  .out_mode_sel = 0x01,
#else
			      .out_mode_sel = 0x07,
#endif
			      .sclk_dividers = 0x01,
			      .sys_mipi_clk = 0x11,
			       },
	[hm2056_SIZE_1280x1024] = {
			/*  Timing control  2624 x 1952 --> 2592 x 1944 */
			      .x_addr_start = 16,
			      .y_addr_start = 4,
			      .x_addr_end = 2607,
			      .y_addr_end = 1947,
			/*  Output image size */
			      .h_output_size = 1280,
			      .v_output_size = 960,
			/*  ISP Windowing size  1296 x 972 --> 1280 x 960 */
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
				  .out_mode_sel = 0x01,
#else
			      .out_mode_sel = 0x07,
#endif
			      .sclk_dividers = 0x01,
			      .sys_mipi_clk = 0x11,
			       },
	[hm2056_SIZE_UXGA] = {
			/*  Timing control  2624 x 1952 --> 2592 x 1944 */
			      .x_addr_start = 16,
			      .y_addr_start = 4,
			      .x_addr_end = 2607,
			      .y_addr_end = 1947,
			/*  Output image size */
			      .h_output_size = 1600,
			      .v_output_size = 1200,
			/*  ISP Windowing size	2592 x 1944 --> 2560 x 1920 */
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
				  .out_mode_sel = 0x00,
#else
			      .out_mode_sel = 0x06,
#endif
			      .sclk_dividers = 0x01,
			      .sys_mipi_clk = 0x12,
			      },

};


/**
 *hm2056_reg_read - Read a value from a register in an hm2056 sensor device
 *@client: i2c driver client structure
 *@reg: register address / offset
 *@val: stores the value that gets read
 *
 * Read a value from a register in an hm2056 sensor device.
 * The value is returned in 'val'.
 * Returns zero if successful, or non-zero otherwise.
 */
static int hm2056_reg_read(struct i2c_client *client, u16 reg, u8 *val)
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
 * Write a value to a register in hm2056 sensor device.
 *@client: i2c driver client structure.
 *@reg: Address of the register to read value from.
 *@val: Value to be written to a specific register.
 * Returns zero if successful, or non-zero otherwise.
 */
static int hm2056_reg_write(struct i2c_client *client, u16 reg, u8 val)
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
		dev_err(&client->dev,
		"Failed writing register 0x%04xvat 0x%x !\n", reg, val);
		return ret;
	}

	return 0;
}

static const struct v4l2_queryctrl hm2056_controls[] = {
	{
	 .id = V4L2_CID_CAMERA_BRIGHTNESS,
	 .type = V4L2_CTRL_TYPE_INTEGER,
	 .name = "Brightness",
	 .minimum = EV_MINUS_1,
	 .maximum = EV_PLUS_1,
	 .step = 1,
	 .default_value = EV_DEFAULT,
	 },

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
	 .maximum = (1 << FRAME_RATE_AUTO | 1 << FRAME_RATE_15 |
				1 << FRAME_RATE_30),
	 .step = 1,
	 .default_value = FRAME_RATE_AUTO,
	},

};

/**
 * Initialize a list of hm2056 registers.
 * The list of registers is terminated by the pair of values
 *@client: i2c driver client structure.
 *@reglist[]: List of address of the registers to write data.
 * Returns zero if successful, or non-zero otherwise.
 */
static int hm2056_reg_writes(struct i2c_client *client,
			     const struct hm2056_reg reglist[])
{
	int err = 0, index;

	for (index = 0; ((reglist[index].reg != 0xffff) &&
			(err == 0)); index++) {
		err |= hm2056_reg_write(client,
				reglist[index].reg, reglist[index].val);
		/*  Check for Pause condition */
		if ((reglist[index + 1].reg == DELAY_SEQ)
			&& (reglist[index + 1].val != 0)) {
			msleep(reglist[index + 1].val);
			index += 1;
		}
	}
	return 0;
}

#ifdef hm2056_DEBUG
static int hm2056_reglist_compare(struct i2c_client *client,
				  const struct hm2056_reg reglist[])
{
	int err = 0, index;
	u8 reg;

	for (index = 0; ((reglist[index].reg != 0xFFFF) &&
			(err == 0)); index++) {
		err |= hm2056_reg_read(client, reglist[index].reg, &reg);
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

static int hm2056_config_timing(struct i2c_client *client)
{
	struct hm2056 *hm2056 = to_hm2056(client);
	int ret = 0, i = hm2056->i_size;
	const struct hm2056_timing_cfg *timing_cfg;

	timing_cfg = &timing_cfg_yuv[i];


    /* TODO: Do the right thing here, and validate mipi timing params */

	return ret;
}

static int hm2056_capture_setting(struct v4l2_subdev *sd)
{
	int ret;
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	ret = hm2056_reg_writes(client, hawaii_capture_init);

	printk(KERN_INFO "hm2056 capture_init_common!");

	return ret;
}


static int hm2056_config_preview(struct v4l2_subdev *sd)
{
	int ret;
	struct i2c_client *client = v4l2_get_subdevdata(sd);


	if (initialized == 0) {
		ret = hm2056_reg_writes(client, hawaii_common_init);
		initialized = 1;
		msleep(100);
	}

	ret = hm2056_reg_writes(client, hawaii_preview_init);

	return ret;
}

static int hm2056_config_capture(struct v4l2_subdev *sd)
{
	int ret = 0;


	hm2056_capture_setting(sd);

	msleep(100);

	return ret;
}


static int stream_mode = -1;
static int hm2056_s_stream(struct v4l2_subdev *sd, int enable)
{
	int ret = 0;
	printk(KERN_INFO "%s: enable:%d runmode:%d  stream_mode:%d\n",
	       __func__, enable, runmode, stream_mode);

	if (enable == stream_mode)
		return ret;


	stream_mode = enable;

	return ret;
}

static int hm2056_g_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct hm2056 *hm2056 = to_hm2056(client);

	mf->width = hm2056_frmsizes[hm2056->i_size].width;
	mf->height = hm2056_frmsizes[hm2056->i_size].height;
	mf->code = hm2056_fmts[hm2056->i_fmt].code;
	mf->colorspace = hm2056_fmts[hm2056->i_fmt].colorspace;
	mf->field = V4L2_FIELD_NONE;

	return 0;
}

static int hm2056_try_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
	int i_fmt;
	int i_size;

	i_fmt = hm2056_find_datafmt(mf->code);

	mf->code = hm2056_fmts[i_fmt].code;
	mf->colorspace = hm2056_fmts[i_fmt].colorspace;
	mf->field = V4L2_FIELD_NONE;

	i_size = hm2056_find_framesize(mf->width, mf->height);

	mf->width = hm2056_frmsizes[i_size].width;
	mf->height = hm2056_frmsizes[i_size].height;

	return 0;
}

static int hm2056_s_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct hm2056 *hm2056 = to_hm2056(client);
	int ret = 0;

	ret = hm2056_try_fmt(sd, mf);
	if (ret < 0)
		return ret;

	hm2056->i_size = hm2056_find_framesize(mf->width, mf->height);
	hm2056->i_fmt = hm2056_find_datafmt(mf->code);

	/*To avoide reentry init sensor, remove from here*/

	/*printk(KERN_INFO "%s: code:0x%x fmt[%d] runmode %d\n", __FUNCTION__,
	* hm2056_fmts[hm2056->i_fmt].code, hm2056->i_size,runmode); */

	if (CAM_RUNNING_MODE_PREVIEW == runmode) {
		hm2056_config_preview(sd);
		hm2056_config_timing(client);
	}

	return ret;
}

static int hm2056_g_chip_ident(struct v4l2_subdev *sd,
			       struct v4l2_dbg_chip_ident *id)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (id->match.type != V4L2_CHIP_MATCH_I2C_ADDR)
		return -EINVAL;

	if (id->match.addr != client->addr)
		return -ENODEV;

	id->ident = V4L2_IDENT_HM2056;
	id->revision = 0;

	return 0;
}

static int hm2056_get_tline(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	u8 val1, val2;
	u16 colblnk = 0;
	unsigned short actcol, ldelay;
	u8 pixelclock = 1;/*TBD*/

	actcol = 810;
	ldelay = 275;
	hm2056_reg_read(client, 0x0012, &val1);
	hm2056_reg_read(client, 0x0013, &val2);
	colblnk = (val1 << 4) + val2 + actcol + ldelay;
	colblnk = colblnk * pixelclock;
	return colblnk;
}

static int hm2056_get_integ(struct v4l2_subdev *sd)
{
	/* read shutter, in number of line period*/
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	u16 integ;
	u8 val;
	hm2056_reg_read(client, 0x0015, &val);
	integ = val & 0x0F;
	hm2056_reg_read(client, 0x0016, &val);
	integ = (integ << 8) + val;
	return integ;
}


static int hm2056_get_exp_time(struct v4l2_subdev *sd)
{
	unsigned short val1, val2, etime;
	val1 = hm2056_get_integ(sd);
	val2 = hm2056_get_tline(sd);
	etime = val1 * val2;
	return etime;
}


static int hm2056_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct hm2056 *hm2056 = to_hm2056(client);

	dev_dbg(&client->dev, "hm2056_g_ctrl\n");

	switch (ctrl->id) {
	case V4L2_CID_CAMERA_BRIGHTNESS:
		ctrl->value = hm2056->brightness;
		break;
	case V4L2_CID_CAMERA_CONTRAST:
		ctrl->value = hm2056->contrast;
		break;
	case V4L2_CID_CAMERA_EFFECT:
		ctrl->value = hm2056->colorlevel;
		break;
	case V4L2_CID_SATURATION:
		ctrl->value = hm2056->saturation;
		break;
	case V4L2_CID_SHARPNESS:
		ctrl->value = hm2056->sharpness;
		break;
	case V4L2_CID_CAMERA_ANTI_BANDING:
		ctrl->value = hm2056->antibanding;
		break;
	case V4L2_CID_CAMERA_WHITE_BALANCE:
		ctrl->value = hm2056->whitebalance;
		break;
	case V4L2_CID_CAMERA_FRAME_RATE:
		ctrl->value = hm2056->framerate;
		break;
	case V4L2_CID_CAMERA_FLASH_MODE:
		ctrl->value = hm2056->flashmode;
		break;
	case V4L2_CID_CAMERA_EXP_TIME:
		ctrl->value = hm2056_get_exp_time(sd);
		break;
	}

	return 0;
}

static int hm2056_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct hm2056 *hm2056 = to_hm2056(client);
	int ret = 0;

	dev_dbg(&client->dev, "hm2056_s_ctrl\n");

	switch (ctrl->id) {
	case V4L2_CID_CAMERA_BRIGHTNESS:

		if (ctrl->value > EV_PLUS_2)
			return -EINVAL;

		hm2056->brightness = ctrl->value;
		switch (hm2056->brightness) {
		case EV_MINUS_2:
			ret = hm2056_reg_writes(client,
					hm2056_brightness_lv0_tbl);
			break;
		case EV_MINUS_1:
			ret = hm2056_reg_writes(client,
					hm2056_brightness_lv1_tbl);
			break;
		case EV_PLUS_1:
			ret = hm2056_reg_writes(client,
					hm2056_brightness_lv3_tbl);
			break;
		case EV_PLUS_2:
			ret = hm2056_reg_writes(client,
					hm2056_brightness_lv4_tbl);
			break;
		default:
			ret = hm2056_reg_writes(client,
					hm2056_brightness_lv2_default_tbl);
			break;
		}
		if (ret)
			return ret;

		break;
	case V4L2_CID_CAMERA_CONTRAST:


		break;
	case V4L2_CID_CAMERA_EFFECT:
		if (ctrl->value > IMAGE_EFFECT_BNW)
			return -EINVAL;

		hm2056->colorlevel = ctrl->value;

		switch (hm2056->colorlevel) {
		case IMAGE_EFFECT_BNW:
			ret = hm2056_reg_writes(client, hm2056_effect_bw_tbl);
			break;
		case IMAGE_EFFECT_SEPIA:
			ret = hm2056_reg_writes(client,
						hm2056_effect_sepia_tbl);
			break;
		case IMAGE_EFFECT_NEGATIVE:
			ret = hm2056_reg_writes(client,
						hm2056_effect_negative_tbl);
			break;
		default:
			ret = hm2056_reg_writes(client,
						hm2056_effect_normal_tbl);
			break;
		}
		if (ret)
			return ret;

		break;
	case V4L2_CID_SATURATION:


		break;
	case V4L2_CID_SHARPNESS:


		break;

	case V4L2_CID_CAMERA_ANTI_BANDING:


		break;

	case V4L2_CID_CAMERA_WHITE_BALANCE:
		if (ctrl->value > WHITE_BALANCE_FLUORESCENT)
			return -EINVAL;

		hm2056->whitebalance = ctrl->value;

		switch (hm2056->whitebalance) {
		case WHITE_BALANCE_FLUORESCENT:
			ret = hm2056_reg_writes(client, hm2056_wb_fluorescent);
			break;
		case WHITE_BALANCE_SUNNY:
			ret = hm2056_reg_writes(client, hm2056_wb_daylight);
			break;
		case WHITE_BALANCE_CLOUDY:
			ret = hm2056_reg_writes(client, hm2056_wb_cloudy);
			break;
		case WHITE_BALANCE_TUNGSTEN:
			ret = hm2056_reg_writes(client, hm2056_wb_tungsten);
			break;
		default:
			ret = hm2056_reg_writes(client, hm2056_wb_def);
			break;
		}
		if (ret) {
			printk(KERN_ERR "Some error in AWB\n");
			return ret;
		}

		break;

	case V4L2_CID_CAMERA_FRAME_RATE:


		break;

	case V4L2_CID_CAMERA_FLASH_MODE:

		break;

	case V4L2_CID_CAM_PREVIEW_ONOFF:
		{
			printk(KERN_INFO
			       "hm2056 PREVIEW_ONOFF:%d runmode = %d\n",
			       ctrl->value, runmode);
			if (ctrl->value) {
				runmode = CAM_RUNNING_MODE_PREVIEW;
			} else {
			    initialized = 0;
				runmode = CAM_RUNNING_MODE_NOTREADY;
			}

			break;
		}

	case V4L2_CID_CAM_CAPTURE:
		printk(KERN_INFO "hm2056 runmode = capture\n");
		runmode = CAM_RUNNING_MODE_CAPTURE;
		hm2056_config_capture(sd);
		break;

	case V4L2_CID_CAM_CAPTURE_DONE:
		printk(KERN_INFO "hm2056 runmode = capture_done\n");
		runmode = CAM_RUNNING_MODE_CAPTURE_DONE;
		break;

	}

	return ret;
}


static long hm2056_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
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
			p->focus_distance[0] = 10;	/*near focus in cm */
			p->focus_distance[1] = 100;	/*optimal focus in cm*/
			p->focus_distance[2] = -1;	/*infinity*/
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

#ifdef CONFIG_VIDEO_ADV_DEBUG
static int hm2056_g_register(struct v4l2_subdev *sd,
			     struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (reg->match.type != V4L2_CHIP_MATCH_I2C_ADDR || reg->size > 2)
		return -EINVAL;

	if (reg->match.addr != client->addr)
		return -ENODEV;

	reg->size = 2;
	if (hm2056_reg_read(client, reg->reg, &reg->val))
		return -EIO return 0;

}

static int hm2056_s_register(struct v4l2_subdev *sd,
			     struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (reg->match.type != V4L2_CHIP_MATCH_I2C_ADDR || reg->size > 2)
		return -EINVAL;

	if (reg->match.addr != client->addr)
		return -ENODEV;

	if (hm2056_reg_write(client, reg->reg, reg->val))
		return -EIO;


	return 0;
}
#endif

static int hm2056_init(struct i2c_client *client)
{
	struct hm2056 *hm2056 = to_hm2056(client);
	int ret = 0;

	hm2056->brightness = EV_DEFAULT;
	hm2056->contrast = CONTRAST_DEFAULT;
	hm2056->colorlevel = IMAGE_EFFECT_NONE;
	hm2056->antibanding = ANTI_BANDING_AUTO;
	hm2056->whitebalance = WHITE_BALANCE_AUTO;
	hm2056->framerate = FRAME_RATE_AUTO;

	printk(KERN_ERR"Sensor initialized\n");

	return ret;
}

/*
 * Interface active, can use i2c. If it fails, it can indeed mean, that
 *this wasn't our capture interface, so, we wait for the right one
 */
static int hm2056_video_probe(struct soc_camera_device *icd,
			      struct i2c_client *client)
{
	int ret = 0;
	u8 id_high, id_low;

	/*
	 * We must have a parent by now. And it cannot be a wrong one.
	 * So this entire test is completely redundant.
	 */
	/*if (!icd->dev.parent ||
	to_soc_camera_host(icd->dev.parent)->nr != icd->iface)
	return -ENODEV;dev field is not part of soc_camera_device on 3.4*/

	ret = hm2056_reg_read(client, HM2056_CHIP_ID_HIGH, &id_high);
	ret += hm2056_reg_read(client, HM2056_CHIP_ID_LOW, &id_low);
	if (ret) {
		printk(KERN_ERR
		"Failure to detect HM2056 chip for I2c failure\n");
		goto out;
	}

	printk(KERN_ERR"-hm2056_video_probe- 0x%x%x detected ---\n",
	id_high, id_low);

	/* TODO: Do something like hm2056_init */

out:
	return ret;
}

static void hm2056_video_remove(struct soc_camera_device *icd)
{
	/*dev_dbg(&icd->dev, "Video removed: %p, %p\n",
	icd->dev.parent, icd->vdev);commented because dev is not not
	part soc_camera_device on 3.4 verison*/
}

static int hm2056_queryctrl(struct v4l2_subdev *sd,
		struct v4l2_queryctrl *qc)
{
	int index = 0;
	for (index = 0; (index) < (ARRAY_SIZE(hm2056_controls)); index++) {
		if ((qc->id) == (hm2056_controls[index].id)) {
			qc->type = hm2056_controls[index].type;
			qc->minimum = hm2056_controls[index].minimum;
			qc->maximum = hm2056_controls[index].maximum;
			qc->step = hm2056_controls[index].step;
			qc->default_value =
				hm2056_controls[index].default_value;
			qc->flags = hm2056_controls[index].flags;
			strlcpy(qc->name, hm2056_controls[index].name,
			sizeof(qc->name));
			return 0;
		}
	}
	return -EINVAL;
}

static struct v4l2_subdev_core_ops hm2056_subdev_core_ops = {
	.g_chip_ident = hm2056_g_chip_ident,
	.g_ctrl = hm2056_g_ctrl,
	.s_ctrl = hm2056_s_ctrl,
	.ioctl = hm2056_ioctl,
	.queryctrl = hm2056_queryctrl,
#ifdef CONFIG_VIDEO_ADV_DEBUG
	.g_register = hm2056_g_register,
	.s_register = hm2056_s_register,
#endif
};

static int hm2056_enum_fmt(struct v4l2_subdev *sd, unsigned int index,
			   enum v4l2_mbus_pixelcode *code)
{
	if (index >= ARRAY_SIZE(hm2056_fmts))
		return -EINVAL;

	*code = hm2056_fmts[index].code;
	return 0;
}

static int hm2056_enum_framesizes(struct v4l2_subdev *sd,
				  struct v4l2_frmsizeenum *fsize)
{
	if (fsize->index >= hm2056_SIZE_LAST)
		return -EINVAL;

	fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
	fsize->pixel_format = V4L2_PIX_FMT_UYVY;

	fsize->discrete = hm2056_frmsizes[fsize->index];

	return 0;
}

/* we only support fixed frame rate */
static int hm2056_enum_frameintervals(struct v4l2_subdev *sd,
				      struct v4l2_frmivalenum *interval)
{
	int size;

	if (interval->index >= 1)
		return -EINVAL;

	interval->type = V4L2_FRMIVAL_TYPE_DISCRETE;

	size = hm2056_find_framesize(interval->width, interval->height);

	switch (size) {
	case hm2056_SIZE_UXGA:
		interval->discrete.numerator = 1;
		interval->discrete.denominator = 15;
		break;
	case hm2056_SIZE_VGA:
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

static int hm2056_g_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct hm2056 *hm2056 = to_hm2056(client);
	struct v4l2_captureparm *cparm;

	if (param->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	cparm = &param->parm.capture;

	memset(param, 0, sizeof(*param));
	param->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	cparm->capability = V4L2_CAP_TIMEPERFRAME;

	switch (hm2056->i_size) {
	case hm2056_SIZE_UXGA:
		cparm->timeperframe.numerator = 1;
		cparm->timeperframe.denominator = 15;
		break;
	case hm2056_SIZE_VGA:
	default:
		cparm->timeperframe.numerator = 1;
		cparm->timeperframe.denominator = 24;
		break;
	}

	return 0;
}

static int hm2056_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	/*
	 * FIXME: This just enforces the hardcoded framerates until this is
	 *flexible enough.
	 */
	return hm2056_g_parm(sd, param);
}

static struct v4l2_subdev_video_ops hm2056_subdev_video_ops = {
	.s_stream = hm2056_s_stream,
	.s_mbus_fmt = hm2056_s_fmt,
	.g_mbus_fmt = hm2056_g_fmt,
	.try_mbus_fmt = hm2056_try_fmt,
	.enum_mbus_fmt = hm2056_enum_fmt,
	.enum_mbus_fsizes = hm2056_enum_framesizes,
	.enum_framesizes = hm2056_enum_framesizes,
	.enum_frameintervals = hm2056_enum_frameintervals,
	.g_parm = hm2056_g_parm,
	.s_parm = hm2056_s_parm,
};

static int hm2056_g_skip_frames(struct v4l2_subdev *sd, u32 *frames)
{
	/* Quantity of initial bad frames to skip. Revisit. */
	/*Waitting for AWB stability,  avoid green color issue */
	*frames = 5;

	return 0;
}

static struct v4l2_subdev_sensor_ops hm2056_subdev_sensor_ops = {
	.g_skip_frames = hm2056_g_skip_frames,
};

static struct v4l2_subdev_ops hm2056_subdev_ops = {
	.core = &hm2056_subdev_core_ops,
	.video = &hm2056_subdev_video_ops,
	.sensor = &hm2056_subdev_sensor_ops,
};

static int hm2056_probe(struct i2c_client *client,
			const struct i2c_device_id *did)
{
	struct hm2056 *hm2056;
	struct soc_camera_device *icd = NULL;
	struct soc_camera_link *icl = client->dev.platform_data;
	int ret;

	printk(KERN_ERR"hm2056 probe start\n");
	client->addr = (0x48>>1);
	#if 0
	printk(KERN_ERR"hm2056 i2c addr is 0x%x\n", client->addr);
	if (!icd) {
		dev_err(&client->dev, "hm2056: missing soc-camera data!\n");
		return -EINVAL;
	}

	icl = to_soc_camera_link(icd);
	#endif
	if (!icl) {
		dev_err(&client->dev, "hm2056 driver needs platform data\n");
		return -EINVAL;
	}

	if (!icl->priv) {
		dev_err(&client->dev,
			"hm2056 driver needs i/f platform data\n");
		return -EINVAL;
	}

	hm2056 = kzalloc(sizeof(struct hm2056), GFP_KERNEL);
	if (!hm2056)
		return -ENOMEM;

	v4l2_i2c_subdev_init(&hm2056->subdev, client, &hm2056_subdev_ops);

	hm2056->i_size = hm2056_SIZE_VGA;
	hm2056->i_fmt = 0;	/* First format in the list */
	hm2056->plat_parms = icl->priv;
	ret = hm2056_video_probe(icd, client);
	if (ret) {
		kfree(hm2056);
		return ret;
	}

	/* init the sensor here */
	ret = hm2056_init(client);
	if (ret) {
		dev_err(&client->dev, "Failed to initialize sensor\n");
		ret = -EINVAL;
	}
	printk(KERN_ERR"hm2056 probe sucess end\n");


	return ret;
}

static int hm2056_remove(struct i2c_client *client)
{
	struct hm2056 *hm2056 = to_hm2056(client);
	struct soc_camera_device *icd = client->dev.platform_data;
	hm2056_video_remove(icd);
	client->driver = NULL;
	kfree(hm2056);


	return 0;
}

static const struct i2c_device_id hm2056_id[] = {
	{"HM2056", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, hm2056_id);

static struct i2c_driver hm2056_i2c_driver = {
	.driver = {
		   .name = "HM2056",
		   },
	.probe = hm2056_probe,
	.remove = hm2056_remove,
	.id_table = hm2056_id,
};

static int __init hm2056_mod_init(void)
{
	return i2c_add_driver(&hm2056_i2c_driver);
}

static void __exit hm2056_mod_exit(void)
{
	i2c_del_driver(&hm2056_i2c_driver);
}

module_init(hm2056_mod_init);
module_exit(hm2056_mod_exit);

MODULE_DESCRIPTION(" hm2056 Camera driver");
MODULE_AUTHOR("Sergio Aguirre <saaguirre@ti.com>");
MODULE_LICENSE("GPL v2");
