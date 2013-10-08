/*
 * arch/arm/mach-shmobile/setup-u2camera.c
 *
 * Copyright (C) 2013 Renesas Mobile Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <mach/r8a7373.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/pmic/pmic-ncp6914.h>
#include <mach/common.h>
#include <media/sh_mobile_csi2.h>
#include <media/sh_mobile_rcu.h>
#include <media/soc_camera.h>
#include <media/soc_camera_platform.h>
#ifdef CONFIG_MFD_D2153
#include <linux/d2153/core.h>
#include <linux/d2153/pmic.h>
#include <linux/d2153/d2153_battery.h>
#endif
#include <mach/board.h>
#include <mach/setup-u2camera.h>
#include <mach/setup-u2csi2.h>
#include <mach/setup-u2rcu.h>
#include <mach/setup-u2ion.h>

#if defined(CONFIG_SOC_CAMERA_S5K4ECGX) && \
	defined(CONFIG_SOC_CAMERA_SR030PC50) /* Select by board Rev */
struct i2c_board_info i2c_cameras[] = {
	{
		I2C_BOARD_INFO("S5K4ECGX", 0x56),
	},
	{
		I2C_BOARD_INFO("SR030PC50", 0x30), /* TODO::HYCHO (0x61>>1) */
	},
};

struct soc_camera_link camera_links[] = {
	{
		.bus_id			= 0,
		.board_info		= &i2c_cameras[0],
		.i2c_adapter_id	= 1,
		.module_name	= "S5K4ECGX",
		.power			= S5K4ECGX_power,
	},
	{
		.bus_id			= 1,
		.board_info		= &i2c_cameras[1],
		.i2c_adapter_id	= 1,
		.module_name	= "SR030PC50",
		.power			= SR030PC50_power,
	},
};
#else	/* Select by board Rev */
struct i2c_board_info i2c_cameras[] = {
	/* Rear Camera */
#if defined(CONFIG_SOC_CAMERA_S5K4ECGX)
	{
		I2C_BOARD_INFO("S5K4ECGX", 0x56),
	},
#endif
	/* Front Camera */
#if defined(CONFIG_SOC_CAMERA_SR030PC50)
	{
		I2C_BOARD_INFO("SR030PC50", 0x30), /* TODO::HYCHO (0x61>>1) */
	},
#endif
};

struct soc_camera_link camera_links[] = {
	/* Rear Camera */
#if defined(CONFIG_SOC_CAMERA_S5K4ECGX)
	{
		.bus_id			= 0,
		.board_info		= &i2c_cameras[0],
		.i2c_adapter_id	= 1,
		.module_name	= "S5K4ECGX",
		.power			= S5K4ECGX_power,
	},
#endif
	/* Front Camera */
#if defined(CONFIG_SOC_CAMERA_SR030PC50)
	{
		.bus_id			= 1,
		.board_info		= &i2c_cameras[1],
		.i2c_adapter_id	= 1,
		.module_name	= "SR030PC50",
		.power			= SR030PC50_power,
	},
#endif
};
#endif	/* Select by board Rev */


struct platform_device camera_devices[] = {
	{
		.name	= "soc-camera-pdrv",
		.id		= 0,
		.dev	= {
			.platform_data = &camera_links[0],
		},
	},
	{
		.name	= "soc-camera-pdrv",
		.id	=	1,
		.dev	= {
			.platform_data = &camera_links[1],
		},
	},
};

#if defined(CONFIG_FLASH_RT8547)
static unsigned char reg_value[4] = /*for RT8547 flash */
{
	0x03,
	0x12,
	0x02,
	0x0f,
};
#endif

int camera_init(void)
{
	struct clk *vclk1_clk;
	struct clk *pll1_div2_clk;
	int iRet;

	gpio_request(GPIO_PORT3, NULL);
	gpio_direction_output(GPIO_PORT3, 0);	/* CAM_PWR_EN */
	gpio_request(GPIO_PORT20, NULL);
	gpio_direction_output(GPIO_PORT20, 0);	/* CAM0_RST_N */
	gpio_request(GPIO_PORT45, NULL);
	gpio_direction_output(GPIO_PORT45, 0);	/* CAM0_STBY */

	pll1_div2_clk = clk_get(NULL, "pll1_div2_clk");
	if (IS_ERR(pll1_div2_clk))
		printk(KERN_ERR "clk_get(pll1_div2_clk) failed\n");

	vclk1_clk = clk_get(NULL, "vclk1_clk");
	if (IS_ERR(vclk1_clk))
		printk(KERN_ERR "clk_get(vclk1_clk) failed\n");

	iRet = clk_set_parent(vclk1_clk, pll1_div2_clk);
	if (0 != iRet) {
		printk(KERN_ERR
		"clk_set_parent(vclk1_clk) failed (ret=%d)\n", iRet);
	}

	clk_put(vclk1_clk);
	clk_put(pll1_div2_clk);

	/* Camera ES version convert */
	camera_links[0].priv = &csi20_info;
	camera_links[1].priv = &csi21_info;
	printk(KERN_ALERT "Camera ISP ES version switch (ES2)\n");

#if defined(CONFIG_SOC_CAMERA_S5K4ECGX) && \
	defined(CONFIG_SOC_CAMERA_SR030PC50) /* Select by board Rev */
	csi20_info.clients[0].lanes = 0x3;
#else	/* Select by board Rev */
#if defined(CONFIG_SOC_CAMERA_S5K4ECGX)
	csi20_info.clients[0].lanes = 0x3;
#endif
#endif	/* Select by board Rev */
	return 0;
}

/* CAM0 Power function */
#if defined(CONFIG_SOC_CAMERA_S5K4ECGX)
int S5K4ECGX_power(struct device *dev, int power_on)
{
	struct clk *vclk1_clk, *vclk2_clk;
	int iRet;
#if defined(CONFIG_MFD_D2153)
	struct regulator *regulator;
#endif
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

#if defined(CONFIG_MFD_D2153)
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
#endif

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
#if defined(CONFIG_MFD_D2153)
		regulator = regulator_get(NULL, "cam_af");
		if (IS_ERR(regulator))
			return -1;
		regulator_enable(regulator);
		regulator_put(regulator);
#endif

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

#if defined(CONFIG_MFD_D2153)
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
#endif
		sh_csi2_power(dev, power_on);
		printk(KERN_ALERT "%s PowerOFF fin\n", __func__);
	}

	clk_put(vclk1_clk);
	clk_put(vclk2_clk);

	return 0;
}
#endif

#define CAM_FLASH_ENSET     (GPIO_PORT99)
#define CAM_FLASH_FLEN      (GPIO_PORT100)
#define RT8547_ADDR	0x99
#define LONG_DELAY	9
#define SHORT_DELAY	4
#define START_DELAY	10

#if defined(CONFIG_FLASH_MIC2871)
static void MIC2871_write(char addr, char data)
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

int main_cam_led(int light, int mode)
{
	unsigned long flags;
	spinlock_t lock;
	spin_lock_init(&lock);

	gpio_request(CAM_FLASH_ENSET, "camacq");
	gpio_request(CAM_FLASH_FLEN, "camacq");

	switch (light) {
	case SH_RCU_LED_ON:

		spin_lock_irqsave(&lock, flags);
		gpio_set_value(CAM_FLASH_ENSET, 1);

		/* write "Disabled"(0) to LB_TH(4) */
		MIC2871_write(4, 0);

		if (mode == SH_RCU_LED_MODE_PRE) {
			/* write 56%(21) to TEN/TCUR(2) */
			MIC2871_write(2, 21);
		} else {
			MIC2871_write(5, 1);
/* Register value 7 is the default for regiser 3, so no need to do this */
			/* MIC2871_write(3, 7); */
#if 0	/* Old case */
/* IF you use the FEN pin,then there is no need to program this register
						(FCUR default is 100%) */
			/* write 100%(0) to FEN/FCUR(1) */
			MIC2871_write(1, 0);

			/* Noneed to do this for torch mode */
			/* enable */
			gpio_set_value(CAM_FLASH_FLEN, 1);
#else	/* Following is hte new case using registers only */
			/* write 100%(0) to FEN/FCUR(1) */
			MIC2871_write(1, 16);
#endif
		}

		spin_unlock_irqrestore(&lock, flags);
		break;
	case SH_RCU_LED_OFF:

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
#endif

static inline int camdrv_ss_RT8547_flash_send_bit(unsigned char bit)
{
	if (bit > 0) {
		gpio_set_value(CAM_FLASH_ENSET, 0);
		udelay(SHORT_DELAY);
		gpio_set_value(CAM_FLASH_ENSET, 1);
		udelay(LONG_DELAY);
	} else {
		gpio_set_value(CAM_FLASH_ENSET, 0);
		udelay(LONG_DELAY);
		gpio_set_value(CAM_FLASH_ENSET, 1);
		udelay(SHORT_DELAY);
	}
	return 0;
}

static inline int camdrv_ss_RT8547_flash_send_byte(unsigned char byte)
{
	int i;
	/* send order is high bit to low bit */
	for (i = 7; i >= 0; i--)
		camdrv_ss_RT8547_flash_send_bit(byte&(0x1<<i));
	return 0;
}

static inline int camdrv_ss_RT8547_flash_send_special_byte(unsigned char byte)
{
	int i;
	/* only send three bit for register address */
	for (i = 2; i >= 0; i--)
		camdrv_ss_RT8547_flash_send_bit(byte&(0x1<<i));
	return 0;
}

static inline int camdrv_ss_RT8547_flash_start_xfer(void)
{
	gpio_set_value(CAM_FLASH_ENSET, 1);
	udelay(START_DELAY);
	return 0;
}

static inline int camdrv_ss_RT8547_flash_stop_xfer(void)
{
	/* redundant 1 bit as the stop condition */
	camdrv_ss_RT8547_flash_send_bit(1);
	return 0;
}

#if defined(CONFIG_FLASH_RT8547)
static int camdrv_ss_RT8547_flash_send_data(int reg, unsigned char data)
{

	unsigned long flags;
	unsigned char xfer_data[3]; /* 0: adddr, 1: reg, 2: reg data */
	spinlock_t lock;
	spin_lock_init(&lock);
	xfer_data[0] = RT8547_ADDR;
	xfer_data[1] = (unsigned char)reg;
	xfer_data[2] = (unsigned char)data;
	/*CAM_INFO_PRINTK( "rt8547-> 0: 0x%02x, 1: 0x%02x, 2: 0x%02x\n",
			* xfer_data[0], xfer_data[1], xfer_data[2]));*/
	spin_lock_irqsave(&lock, flags);

	camdrv_ss_RT8547_flash_start_xfer();
	/*send order is high bit to low bit */
	camdrv_ss_RT8547_flash_send_byte(xfer_data[0]);
	camdrv_ss_RT8547_flash_send_special_byte(xfer_data[1]);
	camdrv_ss_RT8547_flash_send_byte(xfer_data[2]);

	camdrv_ss_RT8547_flash_stop_xfer();

	spin_unlock_irqrestore(&lock, flags);

	/* write back to reg array */
	reg_value[reg-1] = data;

	return 0;
}

#define LED_MODE_MASK	    0x10
int main_cam_led(int light, int mode)
{
	int reg;
	unsigned char reg_val;

	gpio_request(CAM_FLASH_ENSET, "camacq");
	gpio_request(CAM_FLASH_FLEN, "camacq");

	switch (light) {
	case SH_RCU_LED_ON:

		if (mode == SH_RCU_LED_MODE_PRE) {
			gpio_set_value(CAM_FLASH_FLEN, 0);
			gpio_set_value(CAM_FLASH_ENSET, 0);

			/* set Low Vin Protection */
			reg = 0x01;
			reg_val = reg_value[reg-1];
			camdrv_ss_RT8547_flash_send_data(reg, reg_val);

			/* set torch current & set torch mode */
			reg = 0x03;
			reg_val = (reg_value[reg-1] | LED_MODE_MASK);
			camdrv_ss_RT8547_flash_send_data(reg, reg_val);
			/* set ctlen high & flashen high*/
			gpio_set_value(CAM_FLASH_ENSET, 1);
			gpio_set_value(CAM_FLASH_FLEN, 1);
			} else {
			gpio_set_value(CAM_FLASH_FLEN, 0);
			gpio_set_value(CAM_FLASH_ENSET, 0);

			/*set Low Vin Protection */
			reg = 0x01;
			reg_val = reg_value[reg-1];
			camdrv_ss_RT8547_flash_send_data(reg, reg_val);

			/* set strobe current */
			reg = 0x02;
			reg_val = reg_value[reg-1];
			camdrv_ss_RT8547_flash_send_data(reg, reg_val);
			/* set strobe mode */
			reg = 0x03;
			reg_val = (reg_value[reg-1] & ~LED_MODE_MASK);
			camdrv_ss_RT8547_flash_send_data(reg, reg_val);
			/* set strobe timing to maximum time 0.5s */
			reg = 0x04;
			reg_val = reg_value[reg-1];
			camdrv_ss_RT8547_flash_send_data(reg, reg_val);
			/* set ctlen high & flashen high */
			gpio_set_value(CAM_FLASH_ENSET, 1);
			gpio_set_value(CAM_FLASH_FLEN, 1);
		}
		break;
	case SH_RCU_LED_OFF:

		gpio_set_value(CAM_FLASH_FLEN, 0);
		gpio_set_value(CAM_FLASH_ENSET, 0);
		break;
	default:
		gpio_set_value(CAM_FLASH_FLEN, 0);
		gpio_set_value(CAM_FLASH_ENSET, 0);
		udelay(500);
		break;
	}
	gpio_free(CAM_FLASH_ENSET);
	gpio_free(CAM_FLASH_FLEN);

	return 0;
}
#endif

/* CAM1 Power function */
#if defined(CONFIG_SOC_CAMERA_SR030PC50)
int SR030PC50_power(struct device *dev, int power_on)
{
	struct clk *vclk1_clk, *vclk2_clk;
	int iRet;
#if defined(CONFIG_MFD_D2153)
	struct regulator *regulator;
#endif

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

#if defined(CONFIG_MFD_D2153)
		/* CAM_AVDD_2V8  On */
		regulator = regulator_get(NULL, "cam_sensor_a");
		if (IS_ERR(regulator))
			return -1;
		regulator_enable(regulator);
		regulator_put(regulator);
		mdelay(1);

		/* CAM_VDDIO_1V8 On */
		regulator = regulator_get(NULL, "cam_sensor_io");
		if (IS_ERR(regulator))
			return -1;
		regulator_enable(regulator);
		regulator_put(regulator);
		mdelay(1);
#endif

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

#if defined(CONFIG_MFD_D2153)
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
#endif
		sh_csi2_power(dev, power_on);
		printk(KERN_ALERT "%s PowerOFF fin\n", __func__);
	}

	clk_put(vclk1_clk);
	clk_put(vclk2_clk);

	return 0;
}

#endif


