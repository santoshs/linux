/*
 * Driver for Samsung SR030PC50 VGA Camera
 *
 * Copyright (C) 2012-2013 Renesas Mobile Corp.
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
#include <linux/v4l2-mediabus.h>
#include <linux/module.h>
#include <linux/videodev2.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <linux/d2153/core.h>

#include <media/soc_camera.h>
#include <media/v4l2-subdev.h>
#include <media/v4l2-chip-ident.h>
#include <media/v4l2-ctrls.h>
#include <media/sh_mobile_csi2.h>

#include <mach/r8a7373.h>

/* CAM1 Power function */
int SR030PC50_power(struct device *dev, int power_on)
{
	struct clk *vclk1_clk, *vclk2_clk;
	int iRet;
	struct regulator *regulator;

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
		gpio_set_value(GPIO_PORT3, 0); /* CAM_PWR_EN */
		gpio_set_value(GPIO_PORT16, 0); /* CAM1_RST_N */
		gpio_set_value(GPIO_PORT91, 0); /* TODO::HYCHO CAM1_CEN */
		gpio_set_value(GPIO_PORT20, 0); /* CAM0_RST_N */
		gpio_set_value(GPIO_PORT45, 0); /* CAM0_STBY */

		mdelay(10);
		/* 10ms */

		/* CAM_AVDD_2V8  On */
		regulator = regulator_get(NULL, "cam_sensor_a");
		if (IS_ERR(regulator))
			return -1;
		if (regulator_enable(regulator))
			dev_warn(dev, "Could not enable regulator\n");
		regulator_put(regulator);
		mdelay(1);

		/* CAM_VDDIO_1V8 On */
		regulator = regulator_get(NULL, "cam_sensor_io");
		if (IS_ERR(regulator))
			return -1;
		if (regulator_enable(regulator))
			dev_warn(dev, "Could not enable regulator\n");
		regulator_put(regulator);
		mdelay(1);

		/* CAM_CORE_1V2  On */
		gpio_set_value(GPIO_PORT3, 1);
		mdelay(2);

		/* CAM_CORE_1V2  Off */
		gpio_set_value(GPIO_PORT3, 0);
		mdelay(4);

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

		gpio_set_value(GPIO_PORT91, 1); /* CAM1_STBY */
		mdelay(4);

		gpio_set_value(GPIO_PORT16, 1); /* CAM1_RST_N */
		mdelay(100);

		printk(KERN_ALERT "%s PowerON fin\n", __func__);
	} else {
		printk(KERN_ALERT "%s PowerOFF\n", __func__);

		gpio_set_value(GPIO_PORT16, 0); /* CAM1_RST_N */
		mdelay(1);

		gpio_set_value(GPIO_PORT91, 0); /* CAM1_STBY */
		mdelay(2);

		clk_disable(vclk2_clk);
		mdelay(1);

		/* CAM_VDDIO_1V8 Off */
		regulator = regulator_get(NULL, "cam_sensor_io");
		if (IS_ERR(regulator))
			return -1;
		regulator_disable(regulator);
		regulator_put(regulator);
		mdelay(1);

		/* CAM_AVDD_2V8  Off */
		/* cam_sensor_a2.8 */
		regulator = regulator_get(NULL, "cam_sensor_a");
		if (IS_ERR(regulator))
			return -1;
		regulator_disable(regulator);
		regulator_put(regulator);
		mdelay(1);

		/* CAM_CORE_1V2  Off */
		gpio_set_value(GPIO_PORT3, 0);
		mdelay(1);
		sh_csi2_power(dev, power_on);
		printk(KERN_ALERT "%s PowerOFF fin\n", __func__);

	}

	clk_put(vclk1_clk);
	clk_put(vclk2_clk);

	return 0;
}

static int sr030pc50_s_power(struct v4l2_subdev *sd, int on)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct soc_camera_subdev_desc *ssdd = soc_camera_i2c_to_desc(client);
	int ret;

	if (on) {
		ret = soc_camera_power_on(&client->dev, ssdd);
		if (ret < 0)
			return ret;
	} else{
		ret = soc_camera_power_off(&client->dev, ssdd);
		if (ret < 0)
			return ret;
	}

	return 0;
}

static ssize_t subcamtype_SR030PC50_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	char *sensorname = "SR030PC50";
	return sprintf(buf, "%s\n", sensorname);
}

static ssize_t subcamfw_SR030PC50_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	char *sensorfw = "SR030PC50 N";
	return sprintf(buf, "%s\n", sensorfw);
}

static DEVICE_ATTR(front_camtype, S_IRUGO, subcamtype_SR030PC50_show, NULL);
static DEVICE_ATTR(front_camfw, S_IRUGO, subcamfw_SR030PC50_show, NULL);

struct SR030PC50_datafmt {
	enum v4l2_mbus_pixelcode	code;
	enum v4l2_colorspace		colorspace;
};

struct SR030PC50 {
	struct v4l2_subdev		subdev;
	struct v4l2_ctrl_handler hdl;
	const struct SR030PC50_datafmt	*fmt;
	unsigned int			width;
	unsigned int			height;
};

static const struct SR030PC50_datafmt SR030PC50_colour_fmts[] = {
	{V4L2_MBUS_FMT_SBGGR10_1X10,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_SGBRG10_1X10,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_SGRBG10_1X10,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_SRGGB10_1X10,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_UYVY8_2X8,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_VYUY8_2X8,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_YUYV8_2X8,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_YVYU8_2X8,	V4L2_COLORSPACE_SRGB},
};

static inline int sr030pc50_read(struct i2c_client *client,
	unsigned char subaddr, unsigned char *data)
{
	unsigned char buf[2];
	int err = 0;
	struct i2c_msg msg = {client->addr, 0, 1, buf};

	buf[0] = subaddr;

	err = i2c_transfer(client->adapter, &msg, 1);
	if (unlikely(err < 0))
		dev_err(&client->dev, "%s: %d register read fail\n",
							__func__, __LINE__);

	msg.flags = I2C_M_RD;
	msg.len = 1;

	err = i2c_transfer(client->adapter, &msg, 1);
	if (unlikely(err < 0))
		dev_err(&client->dev, "%s: %d register read fail\n",
							__func__, __LINE__);

/*	dev_err(&client->dev, "\n\n\n%X %X\n\n\n", buf[0], buf[1]); */

	*data = buf[0];

	return err;
}

static inline int sr030pc50_write(struct i2c_client *client,
						unsigned char packet[])
{

	int err = 0;
	int retry_count = 5;

	struct i2c_msg msg = {
		.addr   = client->addr,
		.flags  = 0,
		.buf    = (char *)packet,
		.len    = 2,
	};

	if (!client->adapter) {
		dev_err(&client->dev,
			"%s: can't search i2c client adapter\n", __func__);
		return -EIO;
	}

	while (retry_count--) {
		/**(unsigned short *)buf = cpu_to_be16(packet);*/
		err = i2c_transfer(client->adapter, &msg, 1);
		if (likely(err == 1))
			break;
		mdelay(10);
	}

	if (unlikely(err < 0)) {
		dev_err(&client->dev, "%s: 0x%08x write failed err = %d\n",
					__func__, (unsigned int)packet, err);
		return err;
	}

	return (err != 1) ? -1 : 0;
}

static struct SR030PC50 *to_SR030PC50(const struct i2c_client *client)
{
	return container_of(i2c_get_clientdata(client),
				struct SR030PC50, subdev);
}

/* Find a data format by a pixel code in an array */
static const struct SR030PC50_datafmt *SR030PC50_find_datafmt(
					enum v4l2_mbus_pixelcode code)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(SR030PC50_colour_fmts); i++)
		if (SR030PC50_colour_fmts[i].code == code)
			return SR030PC50_colour_fmts + i;

	return NULL;
}

/* select nearest higher resolution for capture */
static void SR030PC50_res_roundup(u32 *width, u32 *height)
{
	int i;
	int res_x[] = { 320, 640};
	int res_y[] = { 240, 480};

	for (i = 0; i < ARRAY_SIZE(res_x); i++) {
		if (res_x[i] >= *width && res_y[i] >= *height) {
			*width = res_x[i];
			*height = res_y[i];
			return;
		}
	}

	*width = res_x[1];
	*height = res_y[1];
}

static int SR030PC50_try_fmt(struct v4l2_subdev *sd,
	       struct v4l2_mbus_framefmt *mf)
{
	const struct SR030PC50_datafmt *fmt = SR030PC50_find_datafmt(mf->code);

	dev_dbg(sd->v4l2_dev->dev, "%s(%u)\n", __func__, mf->code);

	if (!fmt) {
		mf->code	= SR030PC50_colour_fmts[0].code;
		mf->colorspace	= SR030PC50_colour_fmts[0].colorspace;
	}

	dev_dbg(sd->v4l2_dev->dev, "in: mf->width = %d, height = %d\n",
		mf->width, mf->height);
	SR030PC50_res_roundup(&mf->width, &mf->height);
	dev_dbg(sd->v4l2_dev->dev, "out: mf->width = %d, height = %d\n",
		mf->width, mf->height);
	mf->field	= V4L2_FIELD_NONE;

	return 0;
}

static int SR030PC50_s_fmt(struct v4l2_subdev *sd,
	     struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct SR030PC50 *priv = to_SR030PC50(client);

	dev_dbg(sd->v4l2_dev->dev, "%s(%u)\n", __func__, mf->code);

	/* MIPI CSI could have changed the format, double-check */
	if (!SR030PC50_find_datafmt(mf->code)) {
		dev_err(sd->v4l2_dev->dev, "%s -EINVAL\n", __func__);
		return -EINVAL;
	}

	SR030PC50_try_fmt(sd, mf);

	priv->fmt	= SR030PC50_find_datafmt(mf->code);
	priv->width	= mf->width;
	priv->height	= mf->height;

	return 0;
}

static int SR030PC50_g_fmt(struct v4l2_subdev *sd,
	     struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct SR030PC50 *priv = to_SR030PC50(client);

	const struct SR030PC50_datafmt *fmt = priv->fmt;

	mf->code	= fmt->code;
	mf->colorspace	= fmt->colorspace;
	mf->width	= priv->width;
	mf->height	= priv->height;
	mf->field	= V4L2_FIELD_NONE;

	return 0;
}

static int SR030PC50_g_crop(struct v4l2_subdev *sd, struct v4l2_crop *a)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct SR030PC50 *priv = to_SR030PC50(client);
	struct v4l2_rect *rect = &a->c;

	a->type		= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	rect->top	= 0;
	rect->left	= 0;
	rect->width	= priv->width;
	rect->height	= priv->height;

	return 0;
}

static int SR030PC50_cropcap(struct v4l2_subdev *sd, struct v4l2_cropcap *a)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct SR030PC50 *priv = to_SR030PC50(client);

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

static int SR030PC50_enum_fmt(struct v4l2_subdev *sd, unsigned int index,
		enum v4l2_mbus_pixelcode *code)
{
	if ((unsigned int)index >= ARRAY_SIZE(SR030PC50_colour_fmts))
		return -EINVAL;

	*code = SR030PC50_colour_fmts[index].code;
	return 0;
}

static int SR030PC50_g_chip_ident(struct v4l2_subdev *sd,
		    struct v4l2_dbg_chip_ident *id)
{
	/* check i2c device */
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	unsigned char rcv_buf[1];
	unsigned char send_buf[2];
	int ret = 0;

	send_buf[0] = 0x03;
	send_buf[1] = 0x00;
	sr030pc50_write(client, send_buf); /* Page 0 */
	ret = sr030pc50_read(client, 0x04, rcv_buf);
	/* device id = P0(0x00) address 0x04 = 0xB8 */

	if (0 > ret) {
		dev_err(&client->dev, "%s :Read Error(%d)\n", __func__, ret);
		id->ident = V4L2_IDENT_NONE;
	} else {
		dev_dbg(&client->dev, "%s :SR030PC50OKOK(%02x)\n", __func__,
			rcv_buf[0]);
		id->ident = V4L2_IDENT_SR030PC50;
	}
	id->revision = 0;

	return 0;
}

static int SR030PC50_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	switch (ctrl->id) {
	case V4L2_CID_GET_TUNING:
#ifdef CONFIG_SOC_CAMERA_SR030PC50_TUNING
		ctrl->value = 1;
#else
		ctrl->value = 0;
#endif
		/* no break */
	default:
		return 0;
	}
	return -ENOIOCTLCMD;
}

/* Request bus settings on camera side */
static int SR030PC50_g_mbus_config(struct v4l2_subdev *sd,
				struct v4l2_mbus_config *cfg)
{
	cfg->type = V4L2_MBUS_CSI2;
	cfg->flags = V4L2_MBUS_CSI2_1_LANE |
		V4L2_MBUS_CSI2_CHANNEL_0 |
		V4L2_MBUS_CSI2_CONTINUOUS_CLOCK;

	return 0;
}

/* Alter bus settings on camera side */
static int SR030PC50_s_mbus_config(struct v4l2_subdev *sd,
				const struct v4l2_mbus_config *cfg)
{
	return 0;
}

static struct v4l2_subdev_video_ops SR030PC50_subdev_video_ops = {
	.s_mbus_fmt	= SR030PC50_s_fmt,
	.g_mbus_fmt	= SR030PC50_g_fmt,
	.try_mbus_fmt	= SR030PC50_try_fmt,
	.enum_mbus_fmt	= SR030PC50_enum_fmt,
	.g_crop		= SR030PC50_g_crop,
	.cropcap	= SR030PC50_cropcap,
	.g_mbus_config	= SR030PC50_g_mbus_config,
	.s_mbus_config	= SR030PC50_s_mbus_config,
};

static struct v4l2_subdev_core_ops SR030PC50_subdev_core_ops = {
	.s_power	= sr030pc50_s_power,
	.g_chip_ident	= SR030PC50_g_chip_ident,
	.g_ctrl		= SR030PC50_g_ctrl,
};

static struct v4l2_subdev_ops SR030PC50_subdev_ops = {
	.core	= &SR030PC50_subdev_core_ops,
	.video	= &SR030PC50_subdev_video_ops,
};

static int SR030PC50_s_ctrl(struct v4l2_ctrl *ctrl)
{
	return 0;
}

static struct v4l2_ctrl_ops SR030PC50_ctrl_ops = {
	.s_ctrl = SR030PC50_s_ctrl,
};

static int SR030PC50_probe(struct i2c_client *client,
			const struct i2c_device_id *did)
{
	struct SR030PC50 *priv;
	struct soc_camera_subdev_desc *sdesc = soc_camera_i2c_to_desc(client);
	int ret = 0;

	dev_dbg(&client->dev, "%s():\n", __func__);

	if (!sdesc) {
		dev_err(&client->dev, "SR030PC50: missing platform data!\n");
		return -EINVAL;
	}

	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		dev_err(&client->dev,
			"SR030PC50: Failed to allocate memory for private data!\n");
		return -ENOMEM;
	}

	v4l2_i2c_subdev_init(&priv->subdev, client, &SR030PC50_subdev_ops);
	v4l2_ctrl_handler_init(&priv->hdl, 4);
	v4l2_ctrl_new_std(&priv->hdl, &SR030PC50_ctrl_ops,
			V4L2_CID_VFLIP, 0, 1, 1, 0);
	v4l2_ctrl_new_std(&priv->hdl, &SR030PC50_ctrl_ops,
			V4L2_CID_HFLIP, 0, 1, 1, 0);
	v4l2_ctrl_new_std(&priv->hdl, &SR030PC50_ctrl_ops,
			V4L2_CID_GAIN, 0, 127, 1, 66);
	v4l2_ctrl_new_std(&priv->hdl, &SR030PC50_ctrl_ops,
			V4L2_CID_AUTO_WHITE_BALANCE, 0, 1, 1, 1);
	priv->subdev.ctrl_handler = &priv->hdl;
	if (priv->hdl.error) {
		int err = priv->hdl.error;

		kfree(priv);
		return err;
	}

	priv->width	= 640;
	priv->height	= 480;
	priv->fmt	= &SR030PC50_colour_fmts[0];
	ret = v4l2_ctrl_handler_setup(&priv->hdl);
	if (0 > ret) {
		dev_err(&client->dev,
			"SR030PC50: v4l2_ctrl_handler_setup Error(%d)\n", ret);
		kfree(priv);
		return ret;
	}

	if (cam_class_init == false) {
		dev_dbg(&client->dev,
			"Start create class for factory test mode !\n");
		camera_class = class_create(THIS_MODULE, "camera");
		cam_class_init = true;
	}

	if (camera_class) {
		dev_dbg(&client->dev, "Create Sub camera device !\n");

		sec_sub_cam_dev = device_create(camera_class,
						NULL, 0, NULL, "front");
		if (IS_ERR(sec_sub_cam_dev)) {
			dev_err(&client->dev,
				"Failed to create device(sec_sub_cam_dev)!\n");
		}

		if (device_create_file(sec_sub_cam_dev,
					&dev_attr_front_camtype) < 0) {
			dev_err(&client->dev,
				"failed to create sub camera device file, %s\n",
				dev_attr_front_camtype.attr.name);
		}
		if (device_create_file(sec_sub_cam_dev,
					&dev_attr_front_camfw) < 0) {
			dev_err(&client->dev,
				"failed to create sub camera device file, %s\n",
				dev_attr_front_camfw.attr.name);
		}
	}

	return ret;
}

static int SR030PC50_remove(struct i2c_client *client)
{
	struct SR030PC50 *priv = to_SR030PC50(client);
	struct soc_camera_subdev_desc *sdesc = soc_camera_i2c_to_desc(client);

	v4l2_device_unregister_subdev(&priv->subdev);
	if (sdesc->free_bus)
		sdesc->free_bus(sdesc);
	v4l2_ctrl_handler_free(&priv->hdl);
	kfree(priv);

	return 0;
}

static const struct i2c_device_id SR030PC50_id[] = {
	{ "SR030PC50", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, SR030PC50_id);

static struct i2c_driver SR030PC50_i2c_driver = {
	.driver = {
		.name = "SR030PC50",
	},
	.probe		= SR030PC50_probe,
	.remove		= SR030PC50_remove,
	.id_table	= SR030PC50_id,
};

module_i2c_driver(SR030PC50_i2c_driver);

MODULE_DESCRIPTION("Samsung SR030PC50 Camera driver");
MODULE_AUTHOR("Renesas Mobile Corp.");
MODULE_LICENSE("GPL v2");

