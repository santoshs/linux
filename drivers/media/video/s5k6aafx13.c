/*
 * Driver for Samsung S5K6AAFX13 1.3M VT Camera
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/* for debug */
//#undef DEBUG
#define DEBUG 1
/* #define DEBUG */

#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/videodev2.h>
#include <linux/gpio.h>
#include <linux/clk.h>

#include <media/soc_camera.h>
#include <media/soc_mediabus.h>
#include <media/v4l2-subdev.h>
#include <media/v4l2-chip-ident.h>
#include <media/sh_mobile_csi2.h>

#include <mach/r8a73734.h>
#include <linux/sh_clk.h>
#include <linux/pmic/pmic-ncp6914.h>

#include <media/s5k6aafx13.h>

struct S5K6AAFX13_datafmt {
	enum v4l2_mbus_pixelcode	code;
	enum v4l2_colorspace		colorspace;
};

struct S5K6AAFX13 {
	struct v4l2_subdev		subdev;
	const struct S5K6AAFX13_datafmt	*fmt;
	unsigned int			width;
	unsigned int			height;
};

static const struct S5K6AAFX13_datafmt S5K6AAFX13_colour_fmts[] = {
	{V4L2_MBUS_FMT_SBGGR10_1X10,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_SGBRG10_1X10,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_SGRBG10_1X10,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_SRGGB10_1X10,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_UYVY8_2X8,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_VYUY8_2X8,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_YUYV8_2X8,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_YVYU8_2X8,	V4L2_COLORSPACE_SRGB},
};

int S5K6AAFX13_power(struct device *dev, int power_on)
{
	struct clk *vclk1_clk, *vclk2_clk;
	int iRet;
	unsigned long mclk_info = 0;

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
		gpio_direction_output(GPIO_PORT3, 0); /* CAM_PWR_EN */
		gpio_set_value(GPIO_PORT16, 0); /* CAM1_RST_N */
		gpio_set_value(GPIO_PORT91, 0); /* CAM1_STBY */
		gpio_set_value(GPIO_PORT20, 0); /* CAM0_RST_N */
		gpio_set_value(GPIO_PORT90, 0); /* CAM0_STBY */

		mdelay(10);
		/* 10ms */

		subPMIC_PowerOn(0x0);

//		/* CAM_CORE_1V2  On */
//		subPMIC_PinOnOff(0x0, 1);
//		mdelay(10);
		/* CAM_AVDD_2V8  On */
		subPMIC_PinOnOff(0x4, 1);
		mdelay(10);
		/* VT_DVDD_1V5   On */
		subPMIC_PinOnOff(0x1, 1);

		mdelay(10);
		/* CAM_VDDIO_1V8 On */
		subPMIC_PinOnOff(0x2, 1);
		mdelay(10);

		gpio_set_value(GPIO_PORT91, 1); /* CAM1_STBY */
		mdelay(10);

		/* MCLK Sub-Camera */
		iRet = clk_set_rate(vclk2_clk,
			clk_round_rate(vclk2_clk, 24000000));
		if (0 != iRet) {
			dev_err(dev,
				"clk_set_rate(vclk2_clk) failed (ret=%d)\n",
				iRet);
		}

		iRet = clk_enable(vclk2_clk);
		if (0 != iRet) {
			dev_err(dev, "clk_enable(vclk2_clk) failed (ret=%d)\n",
				iRet);
		}
		mdelay(10);

		gpio_set_value(GPIO_PORT16, 1); /* CAM1_RST_N */
//		mdelay(150);
//		gpio_set_value(GPIO_PORT91, 0); /* CAM1_STBY */
//		clk_disable(vclk2_clk);

		mdelay(10);

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

		mdelay(1);
		/* 1ms */

		gpio_set_value(GPIO_PORT20, 1); /* CAM0_RST_N Hi */
		mdelay(20);
		/* 20ms */

		/* 5M_AF_2V8 On */
		subPMIC_PinOnOff(0x3, 1);
		mdelay(20);
		clk_disable(vclk1_clk);

		printk(KERN_ALERT "%s PowerON fin\n", __func__);
	}
	else
	{
		printk(KERN_ALERT "%s PowerOFF\n", __func__);

		gpio_set_value(GPIO_PORT16, 0); /* CAM1_RST_N */
		mdelay(1);

		gpio_set_value(GPIO_PORT91, 0); /* CAM1_STBY */
		mdelay(1);

		clk_disable(vclk2_clk);

		mdelay(1);

		/* CAM_VDDIO_1V8 Off */
		subPMIC_PinOnOff(0x2, 0);
		mdelay(1);
		/* VT_DVDD_1V5   Off */
		subPMIC_PinOnOff(0x1, 0);
		mdelay(1);
		/* CAM_AVDD_2V8  Off */
		subPMIC_PinOnOff(0x4, 0);
		mdelay(1);
		/* CAM_CORE_1V2  Off */
//		subPMIC_PinOnOff(0x0, 0);
//		mdelay(1);

		gpio_direction_output(GPIO_PORT3, 0); /* CAM_PWR_EN Low */
		sh_csi2_power(dev, power_on);
		printk(KERN_ALERT "%s PowerOFF fin\n", __func__);
		
	}

	clk_put(vclk1_clk);
	clk_put(vclk2_clk);
	
	return 0;
}

static struct S5K6AAFX13 *to_S5K6AAFX13(const struct i2c_client *client)
{
	return container_of(i2c_get_clientdata(client), struct S5K6AAFX13, subdev);
}

/* Find a data format by a pixel code in an array */
static const struct S5K6AAFX13_datafmt *S5K6AAFX13_find_datafmt(enum v4l2_mbus_pixelcode code)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(S5K6AAFX13_colour_fmts); i++)
		if (S5K6AAFX13_colour_fmts[i].code == code)
			return S5K6AAFX13_colour_fmts + i;

	return NULL;
}

/* select nearest higher resolution for capture */
static void S5K6AAFX13_res_roundup(u32 *width, u32 *height)
{
	int i;
	int res_x[] = { 640, 1280, 1280, 1280 };
	int res_y[] = { 480, 720, 960, 1024 };

	for (i = 0; i < ARRAY_SIZE(res_x); i++) {
		if (res_x[i] >= *width && res_y[i] >= *height) {
			*width = res_x[i];
			*height = res_y[i];
			return;
		}
	}

	*width = res_x[3];
	*height = res_y[3];
}

static int S5K6AAFX13_try_fmt(struct v4l2_subdev *sd,
	       struct v4l2_mbus_framefmt *mf)
{
	const struct S5K6AAFX13_datafmt *fmt = S5K6AAFX13_find_datafmt(mf->code);

	dev_dbg(sd->v4l2_dev->dev, "%s(%u)\n", __func__, mf->code);

	if (!fmt) {
		mf->code	= S5K6AAFX13_colour_fmts[0].code;
		mf->colorspace	= S5K6AAFX13_colour_fmts[0].colorspace;
	}

	dev_dbg(sd->v4l2_dev->dev, "in: mf->width = %d, height = %d\n",
		mf->width, mf->height);
	S5K6AAFX13_res_roundup(&mf->width, &mf->height);
	dev_dbg(sd->v4l2_dev->dev, "out: mf->width = %d, height = %d\n",
		mf->width, mf->height);
	mf->field	= V4L2_FIELD_NONE;

	return 0;
}

static int S5K6AAFX13_s_fmt(struct v4l2_subdev *sd,
	     struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct S5K6AAFX13 *priv = to_S5K6AAFX13(client);

	dev_dbg(sd->v4l2_dev->dev, "%s(%u)\n", __func__, mf->code);

	/* MIPI CSI could have changed the format, double-check */
	if (!S5K6AAFX13_find_datafmt(mf->code)) {
		dev_err(sd->v4l2_dev->dev, "%s -EINVAL\n", __func__);
		return -EINVAL;
	}

	S5K6AAFX13_try_fmt(sd, mf);

	priv->fmt	= S5K6AAFX13_find_datafmt(mf->code);
	priv->width	= mf->width;
	priv->height	= mf->height;

	return 0;
}

static int S5K6AAFX13_g_fmt(struct v4l2_subdev *sd,
	     struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct S5K6AAFX13 *priv = to_S5K6AAFX13(client);

	const struct S5K6AAFX13_datafmt *fmt = priv->fmt;

	mf->code	= fmt->code;
	mf->colorspace	= fmt->colorspace;
	mf->width	= priv->width;
	mf->height	= priv->height;
	mf->field	= V4L2_FIELD_NONE;

	return 0;
}

static int S5K6AAFX13_g_crop(struct v4l2_subdev *sd, struct v4l2_crop *a)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct S5K6AAFX13 *priv = to_S5K6AAFX13(client);
	struct v4l2_rect *rect = &a->c;

	a->type		= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	rect->top	= 0;
	rect->left	= 0;
	rect->width	= priv->width;
	rect->height	= priv->height;

	return 0;
}

static int S5K6AAFX13_cropcap(struct v4l2_subdev *sd, struct v4l2_cropcap *a)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct S5K6AAFX13 *priv = to_S5K6AAFX13(client);

	a->bounds.left			= 0;
	a->bounds.top			= 0;
	a->bounds.width			= priv->width;
	a->bounds.height		= priv->height;
	dev_dbg(&client->dev, "crop: width = %d, height = %d\n",
		a->bounds.width, a->bounds.height);
	a->defrect			= a->bounds;
	a->type				= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	a->pixelaspect.numerator	= 1;
	a->pixelaspect.denominator	= 1;

	return 0;
}

static int S5K6AAFX13_enum_fmt(struct v4l2_subdev *sd, unsigned int index,
		enum v4l2_mbus_pixelcode *code)
{
	if ((unsigned int)index >= ARRAY_SIZE(S5K6AAFX13_colour_fmts))
		return -EINVAL;

	*code = S5K6AAFX13_colour_fmts[index].code;
	return 0;
}

static int S5K6AAFX13_g_chip_ident(struct v4l2_subdev *sd,
		    struct v4l2_dbg_chip_ident *id)
{
	id->ident	= V4L2_IDENT_S5K6AAFX13;
	id->revision	= 0;

	return 0;
}

static struct v4l2_subdev_video_ops S5K6AAFX13_subdev_video_ops = {
	.s_mbus_fmt	= S5K6AAFX13_s_fmt,
	.g_mbus_fmt	= S5K6AAFX13_g_fmt,
	.try_mbus_fmt	= S5K6AAFX13_try_fmt,
	.enum_mbus_fmt	= S5K6AAFX13_enum_fmt,
	.g_crop		= S5K6AAFX13_g_crop,
	.cropcap	= S5K6AAFX13_cropcap,
};

static struct v4l2_subdev_core_ops S5K6AAFX13_subdev_core_ops = {
	.g_chip_ident	= S5K6AAFX13_g_chip_ident,
};

static struct v4l2_subdev_ops S5K6AAFX13_subdev_ops = {
	.core	= &S5K6AAFX13_subdev_core_ops,
	.video	= &S5K6AAFX13_subdev_video_ops,
};

static unsigned long S5K6AAFX13_query_bus_param(struct soc_camera_device *icd)
{
	struct soc_camera_link *icl = to_soc_camera_link(icd);
	unsigned long flags = SOCAM_PCLK_SAMPLE_RISING | SOCAM_SLAVE |
		SOCAM_VSYNC_ACTIVE_HIGH | SOCAM_HSYNC_ACTIVE_HIGH |
		SOCAM_DATA_ACTIVE_HIGH;

	flags |= SOCAM_DATAWIDTH_8;

	return soc_camera_apply_sensor_flags(icl, flags);
}

static int S5K6AAFX13_set_bus_param(struct soc_camera_device *icd,
		     unsigned long flags)
{
	return 0;
}

static struct soc_camera_ops S5K6AAFX13_ops = {
	.query_bus_param	= S5K6AAFX13_query_bus_param,
	.set_bus_param		= S5K6AAFX13_set_bus_param,
};

static int S5K6AAFX13_probe(struct i2c_client *client,
			const struct i2c_device_id *did)
{
	struct S5K6AAFX13 *priv;
	struct soc_camera_device *icd = client->dev.platform_data;
	struct soc_camera_link *icl;
	int ret = 0;

	dev_dbg(&client->dev, "%s():\n", __func__);

	if (!icd) {
		dev_err(&client->dev, "S5K6AAFX13: missing soc-camera data!\n");
		return -EINVAL;
	}

	icl = to_soc_camera_link(icd);
	if (!icl) {
		dev_err(&client->dev, "S5K6AAFX13: missing platform data!\n");
		return -EINVAL;
	}

	priv = kzalloc(sizeof(struct S5K6AAFX13), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	v4l2_i2c_subdev_init(&priv->subdev, client, &S5K6AAFX13_subdev_ops);

	icd->ops	= &S5K6AAFX13_ops;
	priv->width	= 640;
	priv->height= 480;
	priv->fmt	= &S5K6AAFX13_colour_fmts[0];

	return ret;
}

static int S5K6AAFX13_remove(struct i2c_client *client)
{
	struct S5K6AAFX13 *priv = to_S5K6AAFX13(client);
	struct soc_camera_device *icd = client->dev.platform_data;
	struct v4l2_subdev *sd = i2c_get_clientdata(client);

	v4l2_device_unregister_subdev(sd);
	icd->ops = NULL;
	kfree(priv);

	return 0;
}

static const struct i2c_device_id S5K6AAFX13_id[] = {
	{ "S5K6AAFX13", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, S5K6AAFX13_id);

static struct i2c_driver S5K6AAFX13_i2c_driver = {
	.driver = {
		.name = "S5K6AAFX13",
	},
	.probe		= S5K6AAFX13_probe,
	.remove		= S5K6AAFX13_remove,
	.id_table	= S5K6AAFX13_id,
};

static int __init S5K6AAFX13_mod_init(void)
{
	printk(KERN_DEBUG "%s():\n", __func__);
	return i2c_add_driver(&S5K6AAFX13_i2c_driver);
}

static void __exit S5K6AAFX13_mod_exit(void)
{
	printk(KERN_DEBUG "%s():\n", __func__);
	i2c_del_driver(&S5K6AAFX13_i2c_driver);
}

module_init(S5K6AAFX13_mod_init);
module_exit(S5K6AAFX13_mod_exit);

MODULE_DESCRIPTION("Samsung S5K6AAFX13 Camera driver");
MODULE_AUTHOR("Renesas Mobile Corp.");
MODULE_LICENSE("GPL v2");
