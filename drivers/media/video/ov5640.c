/*
 * Driver for Omnivision OV5640 CMOS Image Sensor
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 * Copyright (C) 2011, Renesas Solutions Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/* for debug */
#undef DEBUG
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

#include <mach/r8a73734.h>

struct OV5640_datafmt {
	enum v4l2_mbus_pixelcode	code;
	enum v4l2_colorspace		colorspace;
};

struct OV5640 {
	struct v4l2_subdev		subdev;
	const struct OV5640_datafmt	*fmt;
	unsigned int			width;
	unsigned int			height;
};

static const struct OV5640_datafmt OV5640_colour_fmts[] = {
	{V4L2_MBUS_FMT_SBGGR10_1X10,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_SGBRG10_1X10,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_SGRBG10_1X10,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_SRGGB10_1X10,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_UYVY8_2X8,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_VYUY8_2X8,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_YUYV8_2X8,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_YVYU8_2X8,	V4L2_COLORSPACE_SRGB},
};

static struct OV5640 *
to_OV5640(const struct i2c_client *client)
{
	return container_of(i2c_get_clientdata(client), struct OV5640, subdev);
}

/* Find a data format by a pixel code in an array */
static const struct OV5640_datafmt *
OV5640_find_datafmt(enum v4l2_mbus_pixelcode code)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(OV5640_colour_fmts); i++)
		if (OV5640_colour_fmts[i].code == code)
			return OV5640_colour_fmts + i;

	return NULL;
}

/* select nearest higher resolution for capture */
static void OV5640_res_roundup(u32 *width, u32 *height)
{
	int i;
	enum {          VGA, HD,   UXGA, FHD,  PIX5M };
	int res_x[] = { 640, 1280, 1600, 1920, 2560  };
	int res_y[] = { 480, 720,  1200, 1080, 1920  };

	for (i = 0; i < ARRAY_SIZE(res_x); i++) {
		if (res_x[i] >= *width && res_y[i] >= *height) {
			*width = res_x[i];
			*height = res_y[i];
			return;
		}
	}

	*width = res_x[PIX5M];
	*height = res_y[PIX5M];
}

static int
OV5640_try_fmt(struct v4l2_subdev *sd,
	       struct v4l2_mbus_framefmt *mf)
{
	const struct OV5640_datafmt *fmt = OV5640_find_datafmt(mf->code);

	dev_dbg(sd->v4l2_dev->dev, "%s(%u)\n", __func__, mf->code);

	if (!fmt) {
		mf->code	= OV5640_colour_fmts[0].code;
		mf->colorspace	= OV5640_colour_fmts[0].colorspace;
	}

	dev_dbg(sd->v4l2_dev->dev, "in: mf->width = %d, height = %d\n",
		mf->width, mf->height);
	OV5640_res_roundup(&mf->width, &mf->height);
	dev_dbg(sd->v4l2_dev->dev, "out: mf->width = %d, height = %d\n",
		mf->width, mf->height);
	mf->field	= V4L2_FIELD_NONE;

	return 0;
}

static int
OV5640_s_fmt(struct v4l2_subdev *sd,
	     struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct OV5640 *priv = to_OV5640(client);

	dev_dbg(sd->v4l2_dev->dev, "%s(%u)\n", __func__, mf->code);

	/* MIPI CSI could have changed the format, double-check */
	if (!OV5640_find_datafmt(mf->code)) {
		dev_err(sd->v4l2_dev->dev, "%s -EINVAL\n", __func__);
		return -EINVAL;
	}

	OV5640_try_fmt(sd, mf);

	priv->fmt	= OV5640_find_datafmt(mf->code);
	priv->width	= mf->width;
	priv->height	= mf->height;

	return 0;
}

static int
OV5640_g_fmt(struct v4l2_subdev *sd,
	     struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct OV5640 *priv = to_OV5640(client);

	const struct OV5640_datafmt *fmt = priv->fmt;

	mf->code	= fmt->code;
	mf->colorspace	= fmt->colorspace;
	mf->width	= priv->width;
	mf->height	= priv->height;
	mf->field	= V4L2_FIELD_NONE;

	return 0;
}

static int
OV5640_g_crop(struct v4l2_subdev *sd, struct v4l2_crop *a)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct OV5640 *priv = to_OV5640(client);
	struct v4l2_rect *rect = &a->c;

	a->type		= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	rect->top	= 0;
	rect->left	= 0;
	rect->width	= priv->width;
	rect->height	= priv->height;

	return 0;
}

static int
OV5640_cropcap(struct v4l2_subdev *sd, struct v4l2_cropcap *a)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct OV5640 *priv = to_OV5640(client);

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

static int
OV5640_enum_fmt(struct v4l2_subdev *sd, unsigned int index,
		enum v4l2_mbus_pixelcode *code)
{
	if ((unsigned int)index >= ARRAY_SIZE(OV5640_colour_fmts))
		return -EINVAL;

	*code = OV5640_colour_fmts[index].code;
	return 0;
}

static int
OV5640_g_chip_ident(struct v4l2_subdev *sd,
		    struct v4l2_dbg_chip_ident *id)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (id->match.type != V4L2_CHIP_MATCH_I2C_ADDR)
		return -EINVAL;

	if (id->match.addr != client->addr)
		return -ENODEV;

	id->ident	= V4L2_IDENT_OV5640;
	id->revision	= 0;

	return 0;
}

static struct v4l2_subdev_video_ops OV5640_subdev_video_ops = {
	.s_mbus_fmt	= OV5640_s_fmt,
	.g_mbus_fmt	= OV5640_g_fmt,
	.try_mbus_fmt	= OV5640_try_fmt,
	.enum_mbus_fmt	= OV5640_enum_fmt,
	.g_crop		= OV5640_g_crop,
	.cropcap	= OV5640_cropcap,
};

static struct v4l2_subdev_core_ops OV5640_subdev_core_ops = {
	.g_chip_ident	= OV5640_g_chip_ident,
};

static struct v4l2_subdev_ops OV5640_subdev_ops = {
	.core	= &OV5640_subdev_core_ops,
	.video	= &OV5640_subdev_video_ops,
};

static unsigned long
OV5640_query_bus_param(struct soc_camera_device *icd)
{
	struct soc_camera_link *icl = to_soc_camera_link(icd);
	unsigned long flags = SOCAM_PCLK_SAMPLE_RISING | SOCAM_MASTER |
		SOCAM_VSYNC_ACTIVE_HIGH | SOCAM_HSYNC_ACTIVE_HIGH |
		SOCAM_DATA_ACTIVE_HIGH;

	flags |= SOCAM_DATAWIDTH_8;

	return soc_camera_apply_sensor_flags(icl, flags);
}

static int
OV5640_set_bus_param(struct soc_camera_device *icd,
		     unsigned long flags)
{
	return 0;
}

static struct soc_camera_ops OV5640_ops = {
	.query_bus_param	= OV5640_query_bus_param,
	.set_bus_param		= OV5640_set_bus_param,
};

static int OV5640_probe(struct i2c_client *client,
			const struct i2c_device_id *did)
{
	struct OV5640 *priv;
	struct soc_camera_device *icd = client->dev.platform_data;
	struct soc_camera_link *icl;
	int ret = 0;

	dev_dbg(&client->dev, "%s():\n", __func__);

	if (!icd) {
		dev_err(&client->dev, "OV5640: missing soc-camera data!\n");
		return -EINVAL;
	}

	icl = to_soc_camera_link(icd);
	if (!icl) {
		dev_err(&client->dev, "OV5640: missing platform data!\n");
		return -EINVAL;
	}

	priv = kzalloc(sizeof(struct OV5640), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	v4l2_i2c_subdev_init(&priv->subdev, client, &OV5640_subdev_ops);

	icd->ops	= &OV5640_ops;
	priv->width	= 640;
	priv->height	= 480;
	priv->fmt	= &OV5640_colour_fmts[0];

	return ret;
}

static int OV5640_remove(struct i2c_client *client)
{
	struct OV5640 *priv = to_OV5640(client);
	struct soc_camera_device *icd = client->dev.platform_data;
	struct v4l2_subdev *sd = i2c_get_clientdata(client);

	v4l2_device_unregister_subdev(sd);
	icd->ops = NULL;
	kfree(priv);

	return 0;
}

static const struct i2c_device_id OV5640_id[] = {
	{ "OV5640", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, OV5640_id);

static struct i2c_driver OV5640_i2c_driver = {
	.driver = {
		.name = "OV5640",
	},
	.probe		= OV5640_probe,
	.remove		= OV5640_remove,
	.id_table	= OV5640_id,
};

static int __init OV5640_mod_init(void)
{
	printk(KERN_DEBUG "%s():\n", __func__);
	return i2c_add_driver(&OV5640_i2c_driver);
}

static void __exit OV5640_mod_exit(void)
{
	printk(KERN_DEBUG "%s():\n", __func__);
	i2c_del_driver(&OV5640_i2c_driver);
}

module_init(OV5640_mod_init);
module_exit(OV5640_mod_exit);

MODULE_DESCRIPTION("Omnivision OV5640 Camera driver");
MODULE_AUTHOR("Renesas Mobile Corp.");
MODULE_LICENSE("GPL v2");
