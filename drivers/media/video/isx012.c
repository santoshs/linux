/*
 * Driver for SONY ISX012 CMOS Image Sensor
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
#include <linux/sh_clk.h>
#include <linux/pmic/pmic-ncp6914.h>

#include <media/isx012.h>

typedef struct stSonyData_t {
	u16 usRegs;
	u16 usData;
	u8 ucLen;
} _stSonyData;

struct ISX012_datafmt {
	enum v4l2_mbus_pixelcode	code;
	enum v4l2_colorspace		colorspace;
};

struct ISX012 {
	struct v4l2_subdev		subdev;
	const struct ISX012_datafmt	*fmt;
	unsigned int			width;
	unsigned int			height;
};

static const struct ISX012_datafmt ISX012_colour_fmts[] = {
	{V4L2_MBUS_FMT_SBGGR10_1X10,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_SGBRG10_1X10,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_SGRBG10_1X10,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_SRGGB10_1X10,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_UYVY8_2X8,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_VYUY8_2X8,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_YUYV8_2X8,	V4L2_COLORSPACE_SRGB},
	{V4L2_MBUS_FMT_YVYU8_2X8,	V4L2_COLORSPACE_SRGB},
};

static int
_ISX012_i2c_send(struct i2c_client *client, const u8 * data, int len)
{
	int ret = 0;
	if (len <= 0) {
		printk(KERN_ERR "%s(): invalid length %d", __func__, len);
		return -EINVAL;
	}

	ret = i2c_master_send(client, data, len);
	if (ret < 0) {
		printk(KERN_ERR "Failed to send %d bytes to NCP6914 [errno=%d]",
			len, ret);
	} else if (ret != len) {
		printk(
			KERN_ERR "Failed to send exactly %d bytes to NCP6914 (send %d)",
			len, ret);
		ret = -EIO;
	} else {
		ret = 0;
	}
	return ret;
}

int CamacqExtWriteI2cLists_Sony(struct i2c_client *pClient,
				const void *pvArg, int iResType)
{
	int iRet = 0;
	u32 uiCnt = 0;
	u8 rgucWriteRegs[4] = { 0, };
	_stSonyData *pstRegLists = 0;

	if (iResType == 0) {
		pstRegLists = (_stSonyData *) pvArg;
		{
			if (pstRegLists[uiCnt].ucLen == 0x02) {
				rgucWriteRegs[0] =
					(u8) ((pstRegLists[uiCnt].usRegs >> 8)
						& 0xFF);
				rgucWriteRegs[1] =
					(u8) (pstRegLists[uiCnt].usRegs & 0xFF);
				rgucWriteRegs[2] =
					(u8) (pstRegLists[uiCnt].usData & 0xFF);
				rgucWriteRegs[3] =
					(u8) (pstRegLists[uiCnt].usData >> 8
						& 0xFF);
			} else if (pstRegLists[uiCnt].ucLen == 0x01
				|| pstRegLists[uiCnt].ucLen == 0x03) {
				rgucWriteRegs[0] =
					(u8) ((pstRegLists[uiCnt].usRegs >> 8)
						& 0xFF);
				rgucWriteRegs[1] =
					(u8) (pstRegLists[uiCnt].usRegs & 0xFF);
				rgucWriteRegs[2] =
					(u8) (pstRegLists[uiCnt].usData & 0xFF);
				rgucWriteRegs[3] = 0x00;
			} else if (pstRegLists[uiCnt].ucLen == 0xFF) {
				rgucWriteRegs[0] =
					(u8) ((pstRegLists[uiCnt].usRegs >> 8)
						& 0xFF);
				rgucWriteRegs[1] =
					(u8) (pstRegLists[uiCnt].usRegs & 0xFF);
				rgucWriteRegs[2] =
					(u8) (pstRegLists[uiCnt].usData & 0xFF);
				rgucWriteRegs[3] =
					(u8) (pstRegLists[uiCnt].usData >> 8
						& 0xFF);
			} else {
				printk(KERN_ALERT "%s :Unexpected value!!",
					__func__);
				return iRet;
			}

			while (rgucWriteRegs[0] != 0xFF
				|| rgucWriteRegs[1] != 0xFF
				|| rgucWriteRegs[2] != 0xFF) {
				if (pstRegLists[uiCnt].ucLen != 0x03) {
					iRet = _ISX012_i2c_send(pClient,
						rgucWriteRegs,
						2 + pstRegLists[uiCnt].ucLen);
					if (iRet < 0) {
						printk(
							KERN_ALERT "%s :write failed ===============break!!!!!!!!!!!!!!",
							__func__);
						iRet = -1;
						return iRet;
					}

				} else { /* 0x03 is delay */
					mdelay(rgucWriteRegs[2]);
					printk(
						KERN_ALERT "%s :setfile delay :  %d", __func__,
						 rgucWriteRegs[2]);
				}

				uiCnt++;

				if (pstRegLists[uiCnt].ucLen == 0x02) {
					rgucWriteRegs[0] =
						(u8) ((pstRegLists[uiCnt].usRegs
							>> 8) & 0xFF);
					rgucWriteRegs[1] =
						(u8) (pstRegLists[uiCnt].usRegs
							& 0xFF);

					rgucWriteRegs[2] =
						(u8) (pstRegLists[uiCnt].usData
							& 0xFF);
					rgucWriteRegs[3] =
						(u8) (pstRegLists[uiCnt].usData
							>> 8 & 0xFF);
				} else if (pstRegLists[uiCnt].ucLen == 0x01
					|| pstRegLists[uiCnt].ucLen == 0x03) {
					rgucWriteRegs[0] =
						(u8) ((pstRegLists[uiCnt].usRegs
							>> 8) & 0xFF);
					rgucWriteRegs[1] =
						(u8) (pstRegLists[uiCnt].usRegs
							& 0xFF);
					rgucWriteRegs[2] =
						(u8) (pstRegLists[uiCnt].usData
							& 0xFF);
					rgucWriteRegs[3] = 0x00;
				} else if (pstRegLists[uiCnt].ucLen == 0xFF) {
					rgucWriteRegs[0] =
						(u8) ((pstRegLists[uiCnt].usRegs
							>> 8) & 0xFF);
					rgucWriteRegs[1] =
						(u8) (pstRegLists[uiCnt].usRegs
							& 0xFF);
					rgucWriteRegs[2] =
						(u8) (pstRegLists[uiCnt].usData
							& 0xFF);
					rgucWriteRegs[3] =
						(u8) (pstRegLists[uiCnt].usData
							>> 8 & 0xFF);
				} else {
					printk(
						KERN_ALERT "%s :Unexpected value!!", __func__);
					return iRet;
				}
			}

		}
	}

	return iRet;
}
/* ISX012-0            */
/* MIPI 2LANE      648 */
/*        PLL   648MHz */
/*        DCK       81 */
/* inifile             */
/* size address data   */
_stSonyData ISX012_Pll_Setting_2[] = {
{0x0007, 0x01, 0x01}, /* PLL_CKSEL : PLL 648MHz */
{0x0008, 0x03, 0x01}, /* SRCCK_DIV : 1/8 frequency */

{0x0004, 0x03, 0x01}, /* I2C_ADR_SEL 2: 0x3C MIPI selected,
		       * 3:0x3D MIPI selected */
{0x5008, 0x00, 0x01}, /* ENDIAN_SEL : 0:Little Endian */
{0x6DA8, 0x01, 0x01}, /* SHD_CoEF (OTP shading ON flag) */
{0x6DA9, 0x09, 0x01}, /* WHITE_CTRL */
{0x6DCB, 0x22, 0x01}, /* YGAM_CONFIG2 : */

{0x00C4, 0x11, 0x01}, /* VIF_CLKCONFIG1 : VIFSEL and VIFDIV setting value with
		       * full frame pixel setting for other then JPG */
{0x00C5, 0x11, 0x01}, /* VIF_CLKCONFIG2 : VIFSEL and VIFDIV setting value with
		       * 1/2 sub-sampling setting for other then JPG */
{0x00C6, 0x11, 0x01}, /* VIF_CLKCONFIG3 : VIFSEL and VIFDIV setting value with
		       * 1/4 sub-sampling setting for other then JPG */
{0x00C7, 0x11, 0x01}, /* VIF_CLKCONFIG4 : VIFSEL and VIFDIV setting value with
		       * 1/8 sub-sampling setting for other then JPG */
{0x00C8, 0x11, 0x01}, /* VIF_CLKCONFIG5 : VIFSEL and VIFDIV setting value with
		       * full frame pixel setting for JPG mode */
{0x00C9, 0x11, 0x01}, /* VIF_CLKCONFIG6 : VIFSEL and VIFDIV setting value with
		       * 1/2 sub-sampling setting for JPG mode */
{0x00CA, 0x11, 0x01}, /* VIF_CLKCONFIG7 : VIFSEL and VIFDIV setting value with
		       * 1/4 sub-sampling setting for JPG mode */
{0x018C, 0x0000, 0x02}, /* VADJ_SENS_1_1 : VMAX adjustment value for full frame
			 * pixel */
{0x018E, 0x0000, 0x02}, /* VADJ_SENS_1_2  : VMAX adjustment value for 1/2
			 * sub-sampling */
{0x0190, 0x0000, 0x02}, /* VADJ_SENS_1_4 : VMAX adjustment value for 1/4
			 * sub-sampling */
{0x0192, 0x0000, 0x02}, /* VADJ_SENS_1_8 : VMAX adjustment value for 1/8
			 * sub-sampling */
{0x0194, 0x0000, 0x02}, /* VADJ_SENS_HD_1_1 : VMAX adjustment value for HD full
			 * frame pixel */
{0x0196, 0x0000, 0x02}, /* VADJ_SENS_HD_1_2 : VMAX adjustment value for HD 1/2
			 * sub-sampling */
{0x6A16, 0x0400, 0x02}, /* FLC_OPD_HEIGHT_NORMAL_1_1 : Detection window vertical
			 * size with all 32 windows for FLC full frame pixel */
{0x6A18, 0x03C0, 0x02}, /* FLC_OPD_HEIGHT_NORMAL_1_2 : Detection window vertical
			 * size with all 32 windows for FLC 1/2 sub-sampling */
{0x6A1A, 0x01E0, 0x02}, /* FLC_OPD_HEIGHT_NORMAL_1_4 : Detection window vertical
			 * size with all 32 windows for FLC 1/4 sub-sampling */
{0x6A1C, 0x00E0, 0x02}, /* FLC_OPD_HEIGHT_NORMAL_1_8 : Detection window vertical
			 * size with all 32 windows for FLC 1/8 sub-sampling */
{0x6A1E, 0x0400, 0x02}, /* FLC_OPD_HEIGHT_HD_1_1 : Detection window vertical
			 * size with all 32 windows for FLC HD full frame pixel
			 */
{0x6A20, 0x02C0, 0x02}, /* FLC_OPD_HEIGHT_HD_1_2 : Detection window vertical
			 * size with all 32 windows for FLC HD 1/2 sub-sampling
			 */
{0x0016, 0x0010, 0x02}, /* GPIO_FUNCSEL : GPIO setting */
{0x5C01, 0x00, 0x01}, /* RGLANESEL : Select 1Lane or 2Lane */

{0x5C04, 0x06, 0x01}, /* RGTLPX                    : 0x5C04   0x4  ->  0x6 */
{0x5C05, 0x05, 0x01}, /* RGTCLKPREPARE             : 0x5C05   0x3  ->  0x5 */
{0x5C06, 0x14, 0x01}, /* RGTCLKZERO                : */
{0x5C07, 0x02, 0x01}, /* RGTCLKPRE                 : */
{0x5C08, 0x0D, 0x01}, /* RGTCLKPOST                : 0x5C08   0x11 ->  0xD */
{0x5C09, 0x07, 0x01}, /* RGTCLKTRAIL               : 0x5C09   0x5  ->  0x7 */
{0x5C0A, 0x0A, 0x01}, /* RGTHSEXIT                 : 0x5C0A   0x7  ->  0xA */
{0x5C0B, 0x05, 0x01}, /* RGTHSPREPARE              : 0x5C0B   0x3  ->  0x5 */
{0x5C0C, 0x08, 0x01}, /* RGTHSZERO                 : 0x5C0C   0x7  ->  0x8 */
{0x5C0D, 0x07, 0x01}, /* RGTHSTRAIL                : 0x5C0D   0x5  ->  0x7 */

{0x0009, 0x01, 0x01}, /* EXT_PLL_CKSEL       : PLL 648MHz */
{0x00D0, 0x11, 0x01}, /* VIF_CLKCONFIG_EXT1  : VIFSEL and VIFDIV setting value
		       * with full frame pixel setting for JPG and interleave
		       * mode */
{0x00D1, 0x11, 0x01}, /* VIF_CLKCONFIG_EXT2  : VIFSEL and VIFDIV setting value
		       * with 1/2 sub-sampling setting for JPG and interleave
		       * mode */
{0x00D4, 0x11, 0x01}, /* VIF_CLKCONFIG_EXT5  : VIFSEL and VIFDIV setting value
		       * with full frame pixel setting for JPG mode */
{0x00D5, 0x11, 0x01}, /* VIF_CLKCONFIG_EXT6  : VIFSEL and VIFDIV setting value
		       * with 1/2 sub-sampling setting for JPG mode */
{0x00D8, 0x11, 0x01}, /* VIF_CLKCONFIG_EXT9  : VIFSEL and VIFDIV setting value
		       * with full frame pixel setting for other than JPG */
{0x00D9, 0x11, 0x01}, /* VIF_CLKCONFIG_EXT10 : VIFSEL and VIFDIV setting value
		       * with 1/2 sub-sampling setting for other than JPG */

/* init Preview setting */
{0x0089, 0x00, 0x01}, /* OUTFMT_MONI */
{0x0090, 0x0280, 0x02}, /* HSIZE_MONI : 640 */
{0x0096, 0x01E0, 0x02}, /* VSIZE_MONI : 480 */
{0x0083, 0x01, 0x01}, /* SENSMODE_MONI */
{0x0086, 0x02, 0x01}, /* FPSTYPE_MONI */
{0x0081, 0x00, 0x01}, /* MODESEL */
{0x0082, 0x01, 0x01}, /* MONI_REFRESH */

/* jpeg setting */
/* Apex40 is not Jpeg Capture */

/* Fast mode setting */
{0x500A, 0x00, 0x01}, /* FAST_MODECHG_EN */
{0x500B, 0x01, 0x01}, /* FAST_SHT_MODE_SEL */
{0x500C, 0x00FA, 0x02}, /* FAST_SHT_LIMIT_COUNT */

/* Select sensor inversion link control */
{0x501A, 0x00, 0x01}, /* SENS_REVERSE_CTRL */

/* shading */
{0x6DBC, 0x03, 0x01}, /* WHITE_EDGE_MAX : */
{0x6DF6, 0xFF, 0x01}, /* WHITE_SHD_JUDGE_BODY_COLOR_RATIO : */
{0x6DF7, 0xF0, 0x01}, /* WHITE_SHD_JUDGE_RED_RATIO : */
{0x6DAD, 0x0C, 0x01}, /* WHITE_OFSET1_UP : */
{0x6DAE, 0x0C, 0x01}, /* WHITE_OFSET1_DOWN : */
{0x6DAF, 0x11, 0x01}, /* WHITE_OFSET1_RIGHT : */
{0x6DB0, 0x1B, 0x01}, /* WHITE_OFSET1_LEFT : */
{0x6DB1, 0x0D, 0x01}, /* WHITE_OFSET2_UP : */
{0x6DB2, 0x13, 0x01}, /* WHITE_OFSET2_DOWN : */
{0x6DB3, 0x11, 0x01}, /* WHITE_OFSET2_RIGHT : */
{0x6DB4, 0x17, 0x01}, /* WHITE_OFSET2_LEFT : */

/* addtional code */
{0xF200, 0xB9B9, 0x02},
{0xF202, 0x4E12, 0x02},
{0xF204, 0x6055, 0x02},
{0xF206, 0x008B, 0x02},
{0xF208, 0xF177, 0x02},
{0xF20A, 0xFA70, 0x02},
{0xF20C, 0x0000, 0x02},
{0xF20E, 0x0000, 0x02},
{0xF210, 0x0000, 0x02},
{0xF212, 0x0000, 0x02},
{0xF214, 0x0000, 0x02},
{0xF216, 0x0000, 0x02},
{0xF218, 0x0000, 0x02},
{0xF21A, 0x0000, 0x02},
{0xF21C, 0x0000, 0x02},
{0xF21E, 0x0000, 0x02},
{0xF220, 0x0000, 0x02},
{0xF222, 0x0000, 0x02},
{0xF224, 0x0000, 0x02},
{0xF226, 0x0000, 0x02},
{0xF228, 0x0000, 0x02},
{0xF22A, 0x0000, 0x02},
{0xF22C, 0x0000, 0x02},
{0xF22E, 0x0000, 0x02},
{0xF230, 0x0000, 0x02},
{0xF232, 0x0000, 0x02},
{0xF234, 0x0000, 0x02},
{0xF236, 0x0000, 0x02},
{0xF238, 0x0000, 0x02},
{0xF23A, 0x0000, 0x02},
{0xF23C, 0x0000, 0x02},
{0xF23E, 0x0000, 0x02},
{0xF240, 0x0000, 0x02},
{0xF242, 0x0000, 0x02},
{0xF244, 0xB47E, 0x02},
{0xF246, 0x4808, 0x02},
{0xF248, 0x7800, 0x02},
{0xF24A, 0x07C0, 0x02},
{0xF24C, 0x0FC0, 0x02},
{0xF24E, 0xF687, 0x02},
{0xF250, 0xF8ED, 0x02},
{0xF252, 0xF68E, 0x02},
{0xF254, 0xFE2B, 0x02},
{0xF256, 0xF688, 0x02},
{0xF258, 0xFF6B, 0x02},
{0xF25A, 0xF693, 0x02},
{0xF25C, 0xFB6B, 0x02},
{0xF25E, 0xF687, 0x02},
{0xF260, 0xF947, 0x02},
{0xF262, 0xBC7E, 0x02},
{0xF264, 0xF688, 0x02},
{0xF266, 0xFD8F, 0x02},
{0xF268, 0x239C, 0x02},
{0xF26A, 0x0018, 0x02},


{0x0006, 0x16, 0x01}, /* INCK_SET : 24MHz */
{0xFFFF, 0xFF, 0xFF}
};



int ISX012_power0(struct device *dev, int power_on)
{
	struct clk *vclk1_clk, *vclk2_clk;
	int iRet;
	struct i2c_adapter *adapter;
	struct i2c_board_info info = { I2C_BOARD_INFO("ISX012_KERN", 0x3D), };
	struct i2c_client *pClient;

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
		gpio_direction_output(GPIO_PORT3, 0); /* CAM_PWR_EN */
		gpio_set_value(GPIO_PORT16, 0); /* CAM1_RST_N */
		gpio_set_value(GPIO_PORT91, 0); /* CAM1_STBY */
		gpio_set_value(GPIO_PORT20, 0); /* CAM0_RST_N */
		gpio_set_value(GPIO_PORT90, 0); /* CAM0_STBY */
		mdelay(10);
		/* 10ms */

		subPMIC_PowerOn(0x0);

		/* CAM_CORE_1V2  On */
		subPMIC_PinOnOff(0x0, 1);
		mdelay(1);
		/* CAM_AVDD_2V8  On */
		subPMIC_PinOnOff(0x4, 1);
		mdelay(1);
		/* VT_DVDD_1V5   On */
		subPMIC_PinOnOff(0x1, 1);
		mdelay(1);
		/* CAM_VDDIO_1V8 On */
		subPMIC_PinOnOff(0x2, 1);
		mdelay(1);

		gpio_set_value(GPIO_PORT91, 1); /* CAM1_STBY */
		udelay(50);

		/* MCLK Sub-Camera */
		iRet = clk_set_rate(vclk2_clk,
			clk_round_rate(vclk2_clk, 12000000));
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
		mdelay(150);
		gpio_set_value(GPIO_PORT91, 0); /* CAM1_STBY */
		clk_disable(vclk2_clk);

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

		adapter = i2c_get_adapter(1);
		if (!adapter) {
			printk(KERN_ALERT"%s :adapter get NG\n", __func__);
			return -1;
		}
		pClient = i2c_new_device(adapter, &info);
		if (!pClient) {
			printk(KERN_ALERT"%s :pClient get NG\n", __func__);
			return -1;
		}

		CamacqExtWriteI2cLists_Sony(pClient, ISX012_Pll_Setting_2, 0);

		i2c_unregister_device(pClient);
		i2c_put_adapter(adapter);

		gpio_set_value(GPIO_PORT90, 1); /* CAM0_STBY */
		mdelay(20);

		/* 5M_AF_2V8 On */
		subPMIC_PinOnOff(0x3, 1);
		mdelay(20);

		printk(KERN_ALERT "%s PowerON fin\n", __func__);
	} else {
		printk(KERN_ALERT "%s PowerOFF\n", __func__);

		gpio_set_value(GPIO_PORT90, 0); /* CAM0_STBY */
		mdelay(1);

		gpio_set_value(GPIO_PORT20, 0); /* CAM0_RST_N */
		mdelay(1);

		clk_disable(vclk1_clk);

		iRet = clk_enable(vclk2_clk);
		if (0 != iRet) {
			dev_err(dev, "clk_enable(vclk2_clk) failed (ret=%d)\n",
				iRet);
		}
		mdelay(1);

		gpio_set_value(GPIO_PORT91, 1); /* CAM1_STBY */
		mdelay(1);
		gpio_set_value(GPIO_PORT16, 0); /* CAM1_RST_N */
		mdelay(1);
		clk_disable(vclk2_clk);
		mdelay(1);
		gpio_set_value(GPIO_PORT91, 0); /* CAM1_STBY */

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
		subPMIC_PinOnOff(0x0, 0);
		mdelay(1);

		gpio_direction_output(GPIO_PORT3, 0); /* CAM_PWR_EN Low */
		printk(KERN_ALERT "%s PowerOFF fin\n", __func__);
	}

	clk_put(vclk1_clk);
	clk_put(vclk2_clk);

	return 0;
}


static struct ISX012 *
to_ISX012(const struct i2c_client *client)
{
	return container_of(i2c_get_clientdata(client), struct ISX012, subdev);
}

/* Find a data format by a pixel code in an array */
static const struct ISX012_datafmt *
ISX012_find_datafmt(enum v4l2_mbus_pixelcode code)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(ISX012_colour_fmts); i++)
		if (ISX012_colour_fmts[i].code == code)
			return ISX012_colour_fmts + i;

	return NULL;
}

/* select nearest higher resolution for capture */
static void ISX012_res_roundup(u32 *width, u32 *height)
{
	int i;
	enum {
		VGA, HD, UXGA, FHD, PIX5M, PIX8M
	};
	int res_x[] = { 640, 1280, 1600, 1920, 2560, 3272 };
	int res_y[] = { 480, 720, 1200, 1080, 1920, 2456 };

	for (i = 0; i < ARRAY_SIZE(res_x); i++) {
		if (res_x[i] >= *width && res_y[i] >= *height) {
			*width = res_x[i];
			*height = res_y[i];
			return;
		}
	}

	*width = res_x[PIX8M];
	*height = res_y[PIX8M];
}

static int
ISX012_try_fmt(struct v4l2_subdev *sd,
	       struct v4l2_mbus_framefmt *mf)
{
	const struct ISX012_datafmt *fmt = ISX012_find_datafmt(mf->code);

	dev_dbg(sd->v4l2_dev->dev, "%s(%u)\n", __func__, mf->code);

	if (!fmt) {
		mf->code	= ISX012_colour_fmts[0].code;
		mf->colorspace	= ISX012_colour_fmts[0].colorspace;
	}

	dev_dbg(sd->v4l2_dev->dev, "in: mf->width = %d, height = %d\n",
		mf->width, mf->height);
	ISX012_res_roundup(&mf->width, &mf->height);
	dev_dbg(sd->v4l2_dev->dev, "out: mf->width = %d, height = %d\n",
		mf->width, mf->height);
	mf->field	= V4L2_FIELD_NONE;

	return 0;
}

static int
ISX012_s_fmt(struct v4l2_subdev *sd,
	     struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ISX012 *priv = to_ISX012(client);

	dev_dbg(sd->v4l2_dev->dev, "%s(%u)\n", __func__, mf->code);

	/* MIPI CSI could have changed the format, double-check */
	if (!ISX012_find_datafmt(mf->code)) {
		dev_err(sd->v4l2_dev->dev, "%s -EINVAL\n", __func__);
		return -EINVAL;
	}

	ISX012_try_fmt(sd, mf);

	priv->fmt	= ISX012_find_datafmt(mf->code);
	priv->width	= mf->width;
	priv->height	= mf->height;

	return 0;
}

static int
ISX012_g_fmt(struct v4l2_subdev *sd,
	     struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ISX012 *priv = to_ISX012(client);

	const struct ISX012_datafmt *fmt = priv->fmt;

	mf->code	= fmt->code;
	mf->colorspace	= fmt->colorspace;
	mf->width	= priv->width;
	mf->height	= priv->height;
	mf->field	= V4L2_FIELD_NONE;

	return 0;
}

static int
ISX012_g_crop(struct v4l2_subdev *sd, struct v4l2_crop *a)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ISX012 *priv = to_ISX012(client);
	struct v4l2_rect *rect = &a->c;

	a->type		= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	rect->top	= 0;
	rect->left	= 0;
	rect->width	= priv->width;
	rect->height	= priv->height;

	return 0;
}

static int
ISX012_cropcap(struct v4l2_subdev *sd, struct v4l2_cropcap *a)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ISX012 *priv = to_ISX012(client);

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
ISX012_enum_fmt(struct v4l2_subdev *sd, unsigned int index,
		enum v4l2_mbus_pixelcode *code)
{
	if ((unsigned int)index >= ARRAY_SIZE(ISX012_colour_fmts))
		return -EINVAL;

	*code = ISX012_colour_fmts[index].code;
	return 0;
}

static int
ISX012_g_chip_ident(struct v4l2_subdev *sd,
		    struct v4l2_dbg_chip_ident *id)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (id->match.type != V4L2_CHIP_MATCH_I2C_ADDR)
		return -EINVAL;

	if (id->match.addr != client->addr)
		return -ENODEV;

	id->ident	= V4L2_IDENT_ISX012;
	id->revision	= 0;

	return 0;
}

static struct v4l2_subdev_video_ops ISX012_subdev_video_ops = {
	.s_mbus_fmt	= ISX012_s_fmt,
	.g_mbus_fmt	= ISX012_g_fmt,
	.try_mbus_fmt	= ISX012_try_fmt,
	.enum_mbus_fmt	= ISX012_enum_fmt,
	.g_crop		= ISX012_g_crop,
	.cropcap	= ISX012_cropcap,
};

static struct v4l2_subdev_core_ops ISX012_subdev_core_ops = {
	.g_chip_ident	= ISX012_g_chip_ident,
};

static struct v4l2_subdev_ops ISX012_subdev_ops = {
	.core	= &ISX012_subdev_core_ops,
	.video	= &ISX012_subdev_video_ops,
};

static unsigned long
ISX012_query_bus_param(struct soc_camera_device *icd)
{
	struct soc_camera_link *icl = to_soc_camera_link(icd);
	unsigned long flags = SOCAM_PCLK_SAMPLE_RISING | SOCAM_MASTER |
		SOCAM_VSYNC_ACTIVE_HIGH | SOCAM_HSYNC_ACTIVE_HIGH |
		SOCAM_DATA_ACTIVE_HIGH;

	flags |= SOCAM_DATAWIDTH_8;

	return soc_camera_apply_sensor_flags(icl, flags);
}

static int
ISX012_set_bus_param(struct soc_camera_device *icd,
		     unsigned long flags)
{
	return 0;
}

static struct soc_camera_ops ISX012_ops = {
	.query_bus_param	= ISX012_query_bus_param,
	.set_bus_param		= ISX012_set_bus_param,
};

static int ISX012_probe(struct i2c_client *client,
			const struct i2c_device_id *did)
{
	struct ISX012 *priv;
	struct soc_camera_device *icd = client->dev.platform_data;
	struct soc_camera_link *icl;
	int ret = 0;

	dev_dbg(&client->dev, "%s():\n", __func__);

	if (!icd) {
		dev_err(&client->dev, "ISX012: missing soc-camera data!\n");
		return -EINVAL;
	}

	icl = to_soc_camera_link(icd);
	if (!icl) {
		dev_err(&client->dev, "ISX012: missing platform data!\n");
		return -EINVAL;
	}

	priv = kzalloc(sizeof(struct ISX012), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	v4l2_i2c_subdev_init(&priv->subdev, client, &ISX012_subdev_ops);

	icd->ops	= &ISX012_ops;
	priv->width	= 640;
	priv->height	= 480;
	priv->fmt	= &ISX012_colour_fmts[0];

	return ret;
}

static int ISX012_remove(struct i2c_client *client)
{
	struct ISX012 *priv = to_ISX012(client);
	struct soc_camera_device *icd = client->dev.platform_data;
	struct v4l2_subdev *sd = i2c_get_clientdata(client);

	v4l2_device_unregister_subdev(sd);
	icd->ops = NULL;
	kfree(priv);

	return 0;
}

static const struct i2c_device_id ISX012_id[] = {
	{ "ISX012", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, ISX012_id);

static struct i2c_driver ISX012_i2c_driver = {
	.driver = {
		.name = "ISX012",
	},
	.probe		= ISX012_probe,
	.remove		= ISX012_remove,
	.id_table	= ISX012_id,
};

static int __init ISX012_mod_init(void)
{
	printk(KERN_DEBUG "%s():\n", __func__);
	return i2c_add_driver(&ISX012_i2c_driver);
}

static void __exit ISX012_mod_exit(void)
{
	printk(KERN_DEBUG "%s():\n", __func__);
	i2c_del_driver(&ISX012_i2c_driver);
}

module_init(ISX012_mod_init);
module_exit(ISX012_mod_exit);

MODULE_DESCRIPTION("SONY ISX012 Camera driver");
MODULE_AUTHOR("Renesas Mobile Corp.");
MODULE_LICENSE("GPL v2");
