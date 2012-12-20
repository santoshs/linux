/*
 * Driver for Samsung DB8131 1.3M VT Camera
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/* for debug */
/* #undef DEBUG */
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

struct DB8131_datafmt {
	enum v4l2_mbus_pixelcode	code;
	enum v4l2_colorspace		colorspace;
};

struct DB8131 {
	struct v4l2_subdev		subdev;
	const struct DB8131_datafmt	*fmt;
	unsigned int			width;
	unsigned int			height;
};

static const struct DB8131_datafmt DB8131_colour_fmts[] = {
	{V4L2_MBUS_FMT_SBGGR10_1X10,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_SGBRG10_1X10,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_SGRBG10_1X10,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_SRGGB10_1X10,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_UYVY8_2X8,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_VYUY8_2X8,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_YUYV8_2X8,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_YVYU8_2X8,	V4L2_COLORSPACE_SRGB},
};

static struct DB8131 *to_DB8131(const struct i2c_client *client)
{
	return container_of(i2c_get_clientdata(client), struct DB8131, subdev);
}

/* Find a data format by a pixel code in an array */
static const struct DB8131_datafmt *DB8131_find_datafmt(
					enum v4l2_mbus_pixelcode code)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(DB8131_colour_fmts); i++)
		if (DB8131_colour_fmts[i].code == code)
			return DB8131_colour_fmts + i;

	return NULL;
}

/* select nearest higher resolution for capture */
static void DB8131_res_roundup(u32 *width, u32 *height)
{
	int i;
	int res_x[] = { 176, 320, 640, 1280};
	int res_y[] = { 144, 240, 480, 720};

	for (i = 0; i < ARRAY_SIZE(res_x); i++) {
		if (res_x[i] >= *width && res_y[i] >= *height) {
			*width = res_x[i];
			*height = res_y[i];
			return;
		}
	}

	*width = res_x[2];
	*height = res_y[2];
}

static int DB8131_try_fmt(struct v4l2_subdev *sd,
	       struct v4l2_mbus_framefmt *mf)
{
	const struct DB8131_datafmt *fmt = DB8131_find_datafmt(mf->code);

	dev_dbg(sd->v4l2_dev->dev, "%s(%u)\n", __func__, mf->code);

	if (!fmt) {
		mf->code	= DB8131_colour_fmts[0].code;
		mf->colorspace	= DB8131_colour_fmts[0].colorspace;
	}

	dev_dbg(sd->v4l2_dev->dev, "in: mf->width = %d, height = %d\n",
		mf->width, mf->height);
	DB8131_res_roundup(&mf->width, &mf->height);
	dev_dbg(sd->v4l2_dev->dev, "out: mf->width = %d, height = %d\n",
		mf->width, mf->height);
	mf->field	= V4L2_FIELD_NONE;

	return 0;
}

static int DB8131_s_fmt(struct v4l2_subdev *sd,
	     struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct DB8131 *priv = to_DB8131(client);

	dev_dbg(sd->v4l2_dev->dev, "%s(%u)\n", __func__, mf->code);

	/* MIPI CSI could have changed the format, double-check */
	if (!DB8131_find_datafmt(mf->code)) {
		dev_err(sd->v4l2_dev->dev, "%s -EINVAL\n", __func__);
		return -EINVAL;
	}

	DB8131_try_fmt(sd, mf);

	priv->fmt	= DB8131_find_datafmt(mf->code);
	priv->width	= mf->width;
	priv->height	= mf->height;

	return 0;
}

static int DB8131_g_fmt(struct v4l2_subdev *sd,
	     struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct DB8131 *priv = to_DB8131(client);

	const struct DB8131_datafmt *fmt = priv->fmt;

	mf->code	= fmt->code;
	mf->colorspace	= fmt->colorspace;
	mf->width	= priv->width;
	mf->height	= priv->height;
	mf->field	= V4L2_FIELD_NONE;

	return 0;
}

static int DB8131_g_crop(struct v4l2_subdev *sd, struct v4l2_crop *a)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct DB8131 *priv = to_DB8131(client);
	struct v4l2_rect *rect = &a->c;

	a->type		= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	rect->top	= 0;
	rect->left	= 0;
	rect->width	= priv->width;
	rect->height	= priv->height;

	return 0;
}

static int DB8131_cropcap(struct v4l2_subdev *sd, struct v4l2_cropcap *a)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct DB8131 *priv = to_DB8131(client);

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

static int DB8131_enum_fmt(struct v4l2_subdev *sd, unsigned int index,
		enum v4l2_mbus_pixelcode *code)
{
	if ((unsigned int)index >= ARRAY_SIZE(DB8131_colour_fmts))
		return -EINVAL;

	*code = DB8131_colour_fmts[index].code;
	return 0;
}

static int DB8131_g_chip_ident(struct v4l2_subdev *sd,
		    struct v4l2_dbg_chip_ident *id)
{
	id->ident	= V4L2_IDENT_DB8131;
	id->revision	= 0;

	return 0;
}

static struct v4l2_subdev_video_ops DB8131_subdev_video_ops = {
	.s_mbus_fmt	= DB8131_s_fmt,
	.g_mbus_fmt	= DB8131_g_fmt,
	.try_mbus_fmt	= DB8131_try_fmt,
	.enum_mbus_fmt	= DB8131_enum_fmt,
	.g_crop		= DB8131_g_crop,
	.cropcap	= DB8131_cropcap,
};

static struct v4l2_subdev_core_ops DB8131_subdev_core_ops = {
	.g_chip_ident	= DB8131_g_chip_ident,
};

static struct v4l2_subdev_ops DB8131_subdev_ops = {
	.core	= &DB8131_subdev_core_ops,
	.video	= &DB8131_subdev_video_ops,
};

static unsigned long DB8131_query_bus_param(struct soc_camera_device *icd)
{
	struct soc_camera_link *icl = to_soc_camera_link(icd);
	unsigned long flags = SOCAM_PCLK_SAMPLE_RISING | SOCAM_SLAVE |
		SOCAM_VSYNC_ACTIVE_HIGH | SOCAM_HSYNC_ACTIVE_HIGH |
		SOCAM_DATA_ACTIVE_HIGH;

	flags |= SOCAM_DATAWIDTH_8;

	return soc_camera_apply_sensor_flags(icl, flags);
}

static int DB8131_set_bus_param(struct soc_camera_device *icd,
		     unsigned long flags)
{
	return 0;
}

static struct soc_camera_ops DB8131_ops = {
	.query_bus_param	= DB8131_query_bus_param,
	.set_bus_param		= DB8131_set_bus_param,
};

static int DB8131_probe(struct i2c_client *client,
			const struct i2c_device_id *did)
{
	struct DB8131 *priv;
	struct soc_camera_device *icd = client->dev.platform_data;
	struct soc_camera_link *icl;
	int ret = 0;

	dev_dbg(&client->dev, "%s():\n", __func__);

	if (!icd) {
		dev_err(&client->dev, "DB8131: missing soc-camera data!\n");
		return -EINVAL;
	}

	icl = to_soc_camera_link(icd);
	if (!icl) {
		dev_err(&client->dev, "DB8131: missing platform data!\n");
		return -EINVAL;
	}

	priv = kzalloc(sizeof(struct DB8131), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	v4l2_i2c_subdev_init(&priv->subdev, client, &DB8131_subdev_ops);

	icd->ops	= &DB8131_ops;
	priv->width	= 640;
	priv->height	= 480;
	priv->fmt	= &DB8131_colour_fmts[0];


	{
		/* check i2c device */
		struct i2c_msg msg[2];
		unsigned char send_buf[2];
		unsigned char rcv_buf[2];

		msg[0].addr = client->addr;
		msg[0].flags = client->flags & I2C_M_TEN;
		msg[0].len = 2;
		msg[0].buf = (char *)send_buf;
		send_buf[0] = 0xFF;     /* device id = 0x6100 */
		send_buf[1] = 0xD0;

		msg[1].addr = client->addr;
		msg[1].flags = client->flags & I2C_M_TEN;
		msg[1].flags = I2C_M_RD;
		msg[1].len = 2;
		msg[1].buf = rcv_buf;

		printk(KERN_ALERT "%s :Slave Address 0x%x\n",
					__func__, msg[0].addr);
		ret = i2c_transfer(client->adapter, msg, 2);
		if (0 > ret) {
			printk(KERN_ALERT "%s :Read Error(%d)\n",
					__func__, ret);
		} else {
			printk(KERN_ALERT "%s :DB8131M OK(%02x, %02x)\n",
					__func__, rcv_buf[0], rcv_buf[1]);
		}
		ret = 0;
	}


	return ret;
}

static int DB8131_remove(struct i2c_client *client)
{
	struct DB8131 *priv = to_DB8131(client);
	struct soc_camera_device *icd = client->dev.platform_data;
	struct v4l2_subdev *sd = i2c_get_clientdata(client);

	v4l2_device_unregister_subdev(sd);
	icd->ops = NULL;
	kfree(priv);

	return 0;
}

static const struct i2c_device_id DB8131_id[] = {
	{ "DB8131", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, DB8131_id);

static struct i2c_driver DB8131_i2c_driver = {
	.driver = {
		.name = "DB8131",
	},
	.probe		= DB8131_probe,
	.remove		= DB8131_remove,
	.id_table	= DB8131_id,
};

static int __init DB8131_mod_init(void)
{
	printk(KERN_DEBUG "%s():\n", __func__);
	return i2c_add_driver(&DB8131_i2c_driver);
}

static void __exit DB8131_mod_exit(void)
{
	printk(KERN_DEBUG "%s():\n", __func__);
	i2c_del_driver(&DB8131_i2c_driver);
}

module_init(DB8131_mod_init);
module_exit(DB8131_mod_exit);

MODULE_DESCRIPTION("Samsung DB8131 Camera driver");
MODULE_AUTHOR("Renesas Mobile Corp.");
MODULE_LICENSE("GPL v2");

