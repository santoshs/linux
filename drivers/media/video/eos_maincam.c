/*
 * Driver for EOS Main Camera Image Sensor
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
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

#include <media/sh_mobile_csi2.h>

struct EOS_datafmt {
	enum v4l2_mbus_pixelcode	code;
	enum v4l2_colorspace		colorspace;
};

struct EOS {
	struct v4l2_subdev		subdev;
	const struct EOS_datafmt	*fmt;
	unsigned int			width;
	unsigned int			height;
};

static const struct EOS_datafmt EOS_colour_fmts[] = {
	{V4L2_MBUS_FMT_SBGGR10_1X10,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_SGBRG10_1X10,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_SGRBG10_1X10,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_SRGGB10_1X10,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_UYVY8_2X8,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_VYUY8_2X8,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_YUYV8_2X8,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_YVYU8_2X8,	V4L2_COLORSPACE_SRGB},
};

/* camera device settings */
extern struct soc_camera_ops OV8820_ops;
extern struct v4l2_subdev_ops OV8820_subdev_ops;
#ifdef CONFIG_SOC_CAMERA_IMX081
extern struct soc_camera_ops IMX081_ops;
extern struct v4l2_subdev_ops IMX081_subdev_ops;
#endif

static struct EOS *
to_EOS(const struct i2c_client *client)
{
	return container_of(i2c_get_clientdata(client), struct EOS, subdev);
}

static int EOS_probe(struct i2c_client *client,
			const struct i2c_device_id *did)
{
	struct EOS *priv;
	struct soc_camera_device *icd = client->dev.platform_data;
	struct soc_camera_link *icl;
	int ret = 0;
	struct sh_csi2_pdata *csi_info;

	struct i2c_msg msg[2];
	unsigned char send_buf[2];
	unsigned char rcv_buf[2];

	dev_dbg(&client->dev, "%s():\n", __func__);

	if (!icd) {
		dev_err(&client->dev, "EOS: missing soc-camera data!\n");
		return -EINVAL;
	}

	icl = to_soc_camera_link(icd);
	if (!icl) {
		dev_err(&client->dev, "EOS: missing platform data!\n");
		return -EINVAL;
	}

	priv = kzalloc(sizeof(struct EOS), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

#ifdef CONFIG_SOC_CAMERA_IMX081
	/* check main camera connect IMX081 */
	csi_info = (struct sh_csi2_pdata *) icl->priv;
	printk(KERN_ALERT "%s :main camera connect check\n", __func__);

	msg[0].addr = client->addr;
	msg[0].flags = client->flags & I2C_M_TEN;
	msg[0].len = 2;
	msg[0].buf = (char *)send_buf;
	send_buf[0] = 0x30;	/* BLKLEVEL */
	send_buf[1] = 0x32;

	msg[1].addr = client->addr;
	msg[1].flags = client->flags & I2C_M_TEN;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 1;
	msg[1].buf = rcv_buf;

	printk(KERN_ALERT "%s :Slave Address 0x%x\n", __func__, msg[0].addr);
	ret = i2c_transfer(client->adapter, msg, 2);
	if (0 > ret) {
#endif
		/* check main camera connect OV8820 */
		msg[0].addr = 0x36;
		msg[0].flags = client->flags & I2C_M_TEN;
		msg[0].len = 2;
		msg[0].buf = (char *) send_buf;
		send_buf[0] = 0x30; /* Chip ID High Byte */
		send_buf[1] = 0x0A;

		msg[1].addr = 0x36;
		msg[1].flags = client->flags & I2C_M_TEN;
		msg[1].flags |= I2C_M_RD;
		msg[1].len = 1;
		msg[1].buf = rcv_buf;

		printk(KERN_ALERT "%s :Slave Address 0x%x\n", __func__,
			msg[0].addr);
		ret = i2c_transfer(client->adapter, msg, 2);
		if ((0 <= ret) && (0x88 == rcv_buf[0])) {
			printk(KERN_ALERT "%s :OV8820 connect\n", __func__);
			v4l2_i2c_subdev_init(&priv->subdev, client,
				&OV8820_subdev_ops);

			icd->ops = &OV8820_ops;
			priv->width = 640;
			priv->height = 480;
			priv->fmt = &EOS_colour_fmts[0];
			ret = 0;

		} else {
			printk(KERN_ALERT "%s : not connect main camera\n",
				__func__);
			return ret;
		}
#ifdef CONFIG_SOC_CAMERA_IMX081
	} else {
		printk(KERN_ALERT "%s :IMX081 connect\n", __func__);
		v4l2_i2c_subdev_init(&priv->subdev, client, &IMX081_subdev_ops);

		icd->ops = &IMX081_ops;
		priv->width = 640;
		priv->height = 480;
		priv->fmt = &EOS_colour_fmts[0];
		ret = 0;
		if (csi_info)
			csi_info->clients->lanes = 0xF;
	}
#endif

	return ret;
}

static int EOS_remove(struct i2c_client *client)
{
	struct EOS *priv = to_EOS(client);
	struct soc_camera_device *icd = client->dev.platform_data;
	struct v4l2_subdev *sd = i2c_get_clientdata(client);

	v4l2_device_unregister_subdev(sd);
	icd->ops = NULL;
	kfree(priv);

	return 0;
}

static const struct i2c_device_id EOS_id[] = {
	{ "EOSCAMERA", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, EOS_id);

static struct i2c_driver EOS_i2c_driver = {
	.driver = {
		.name = "EOSCAMERA",
	},
	.probe		= EOS_probe,
	.remove		= EOS_remove,
	.id_table	= EOS_id,
};

static int __init EOS_mod_init(void)
{
	printk(KERN_DEBUG "%s():\n", __func__);
	return i2c_add_driver(&EOS_i2c_driver);
}

static void __exit EOS_mod_exit(void)
{
	printk(KERN_DEBUG "%s():\n", __func__);
	i2c_del_driver(&EOS_i2c_driver);
}

module_init(EOS_mod_init);
module_exit(EOS_mod_exit);

MODULE_DESCRIPTION("EOS Main Camera driver");
MODULE_AUTHOR("Renesas Mobile Corp.");
MODULE_LICENSE("GPL v2");
