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
#include <linux/kernel.h>
#include <linux/delay.h>
#include <mach/r8a7373.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <mach/common.h>
#include <media/sh_mobile_csi2.h>
#include <media/sh_mobile_rcu.h>
#include <media/soc_camera.h>
#include <media/soc_camera_platform.h>
#include <media/s5k4ecgx.h>
#include <media/sr030pc50.h>
#include <mach/setup-u2camera.h>
#include <mach/setup-u2csi2.h>
#include <mach/setup-u2rcu.h>

static struct i2c_board_info s5k4ecgx_i2c_camera = {
	I2C_BOARD_INFO("S5K4ECGX", 0x56),
};

static struct i2c_board_info sr030pc50_i2c_camera = {
	I2C_BOARD_INFO("SR030PC50", 0x30),
};

static struct i2c_board_info ov5645_i2c_camera = {
	I2C_BOARD_INFO("OV5645", 0x3C),
};

static struct i2c_board_info hm2056_i2c_camera = {
	I2C_BOARD_INFO("HM2056", 0x24),
};

static struct soc_camera_desc primary_camera_link = {
	.subdev_desc = {
		.drv_priv	= &csi20_info,
	},
	.host_desc = {
		.bus_id		= 0,
		.i2c_adapter_id	= 1,
	},
};

static struct soc_camera_desc secondary_camera_link = {
	.subdev_desc = {
		.drv_priv		= &csi21_info,
	},
	.host_desc = {
		.bus_id		= 1,
		.i2c_adapter_id	= 1,
	}
};

static struct platform_device primary_camera_device = {
	.name   = "soc-camera-pdrv",
	.id     = 0,
	.dev    = {
		.platform_data = &primary_camera_link,
	},
};

static struct platform_device secondary_camera_device = {
	.name   = "soc-camera-pdrv",
	.id     = 1,
	.dev    = {
		.platform_data = &secondary_camera_link,
	},
};

void add_s5k4ecgx_primary_camera(void)
{
	primary_camera_link.host_desc.board_info = &s5k4ecgx_i2c_camera;
	primary_camera_link.host_desc.module_name = "S5K4ECGX";
	primary_camera_link.subdev_desc.power = S5K4ECGX_power;
}

void add_ov5645_primary_camera(void)
{
	primary_camera_link.host_desc.board_info = &ov5645_i2c_camera;
	primary_camera_link.host_desc.module_name = "OV5645";
	primary_camera_link.subdev_desc.power = OV5645_power;
}

void add_sr030pc50_secondary_camera(void)
{
	secondary_camera_link.host_desc.board_info = &sr030pc50_i2c_camera;
	secondary_camera_link.host_desc.module_name = "SR030PC50";
	secondary_camera_link.subdev_desc.power = SR030PC50_power;
}

void add_hm2056_secondary_camera(void)
{
	secondary_camera_link.host_desc.board_info = &hm2056_i2c_camera;
	secondary_camera_link.host_desc.module_name = "HM2056";
	secondary_camera_link.subdev_desc.power = HM2056_power;
}

int camera_init(int gpio_cam_pwr_en, int gpio_cam_rst_n, int gpio_cam_stby)
{
	struct clk *vclk1_clk;
	struct clk *pll1_div2_clk;
	int ret;

	gpio_request(gpio_cam_pwr_en, NULL);
	gpio_direction_output(gpio_cam_pwr_en, 0);   /* CAM_PWR_EN */
	gpio_request(gpio_cam_rst_n, NULL);
	gpio_direction_output(gpio_cam_rst_n, 0);  /* CAM0_RST_N */
	gpio_request(gpio_cam_stby, NULL);
	gpio_direction_output(gpio_cam_stby, 0);  /* CAM0_STBY */

	pll1_div2_clk = clk_get(NULL, "pll1_div2_clk");
	if (IS_ERR(pll1_div2_clk))
		pr_err("clk_get(pll1_div2_clk) failed\n");

	vclk1_clk = clk_get(NULL, "vclk1_clk");
	if (IS_ERR(vclk1_clk))
		pr_err("clk_get(vclk1_clk) failed\n");

	ret = clk_set_parent(vclk1_clk, pll1_div2_clk);
	if (0 != ret) {
		pr_err("clk_set_parent(vclk1_clk) failed (ret=%d)\n", ret);
	}

	pr_info("Camera ISP ES version switch (ES2)\n");
	csi20_info.clients[0].lanes = 0x3;
	csi20_info.clients[0].pdev = &primary_camera_device;
	csi21_info.clients[0].pdev = &secondary_camera_device;

	clk_put(vclk1_clk);
	clk_put(pll1_div2_clk);

	ret = platform_device_register(&primary_camera_device);
	if (ret)
		pr_err("%s failed to add primary_camera_device %d",
				__func__, ret);

	ret = platform_device_register(&secondary_camera_device);
	if (ret)
		pr_err("%s failed to add secondary_camera_device %d",
				__func__, ret);
	return 0;
}

static int mic2871_flash_enset;
static int mic2871_flash_flen;
/* TODO: need to move to flash driver */
static void MIC2871_write(char addr, char data)
{
	int i;
	/* send address */
	printk(KERN_ALERT "%s addr(%d) data(%d)\n", __func__, addr, data);
	for (i = 0; i < (addr + 1); i++) {
		gpio_set_value(mic2871_flash_enset, 0);
		udelay(1);
		gpio_set_value(mic2871_flash_enset, 1);
		udelay(1);
	}
	/* wait T lat */
	udelay(97);
	/* send data */
	for (i = 0; i < (data + 1); i++) {
		gpio_set_value(mic2871_flash_enset, 0);
		udelay(1);
		gpio_set_value(mic2871_flash_enset, 1);
		udelay(1);
	}
/* THis only needs to be 405us */
	/* wait T end */
	udelay(405);
}

static int mic2871_led(int light, int mode)
{
	unsigned long flags;
	spinlock_t lock;
	spin_lock_init(&lock);

	gpio_request(mic2871_flash_enset, "camacq");
	gpio_request(mic2871_flash_flen, "camacq");

	switch (light) {
	case SH_RCU_LED_ON:

		spin_lock_irqsave(&lock, flags);
		gpio_set_value(mic2871_flash_enset, 1);

		/* write "Disabled"(0) to LB_TH(4) */
		MIC2871_write(4, 0);

		if (mode == SH_RCU_LED_MODE_PRE) {
			/* write 56%(21) to TEN/TCUR(2) */
			MIC2871_write(2, 21);
		} else {
			MIC2871_write(5, 1);
			/* write 100%(0) to FEN/FCUR(1) */
			MIC2871_write(1, 16);
		}

		spin_unlock_irqrestore(&lock, flags);
		break;
	case SH_RCU_LED_OFF:

		/* initailize falsh IC */
		gpio_set_value(mic2871_flash_flen, 0);
		gpio_set_value(mic2871_flash_enset, 0);

		/* For SWI this only needs to be 400us */
		/* mdelay(1); */
		udelay(500);
		break;
	default:
		printk(KERN_ALERT "%s:not case %d", __func__, light);
		return -1;
		break;
	}
	gpio_free(mic2871_flash_enset);
	gpio_free(mic2871_flash_flen);

	return 0;
}

void add_primary_cam_flash_mic2871(int gpio_cam_flash_enset,
		int gpio_cam_flash_flen)
{
	mic2871_flash_enset = gpio_cam_flash_enset;
	mic2871_flash_flen = gpio_cam_flash_flen;
	sh_mobile_rcu0_info.led = mic2871_led;
}



static int rt8547_flash_enset;
static int rt8547_flash_flen;
/*TODO: if these values changes, then add these as arguments to
 * add_primary_cam_flash_rt8547 */
#define RT8547_ADDR	0x99
#define LONG_DELAY	9
#define SHORT_DELAY	4
#define START_DELAY	10

static unsigned char reg_value[4] = /*for RT8547 flash */
{
	0x03,
	0x12,
	0x02,
	0x0f,
};

static inline int camdrv_ss_RT8547_flash_send_bit(unsigned char bit)
{
	if (bit > 0) {
		gpio_set_value(rt8547_flash_enset, 0);
		udelay(SHORT_DELAY);
		gpio_set_value(rt8547_flash_enset, 1);
		udelay(LONG_DELAY);
	} else {
		gpio_set_value(rt8547_flash_enset, 0);
		udelay(LONG_DELAY);
		gpio_set_value(rt8547_flash_enset, 1);
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
	gpio_set_value(rt8547_flash_enset, 1);
	udelay(START_DELAY);
	return 0;
}

static inline int camdrv_ss_RT8547_flash_stop_xfer(void)
{
	/* redundant 1 bit as the stop condition */
	camdrv_ss_RT8547_flash_send_bit(1);
	return 0;
}

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
static int rt8547_led(int light, int mode)
{
	int reg;
	unsigned char reg_val;

	gpio_request(rt8547_flash_enset, "camacq");
	gpio_request(rt8547_flash_flen, "camacq");

	switch (light) {
	case SH_RCU_LED_ON:

		if (mode == SH_RCU_LED_MODE_PRE) {
			gpio_set_value(rt8547_flash_flen, 0);
			gpio_set_value(rt8547_flash_enset, 0);

			/* set Low Vin Protection */
			reg = 0x01;
			reg_val = reg_value[reg-1];
			camdrv_ss_RT8547_flash_send_data(reg, reg_val);

			/* set torch current & set torch mode */
			reg = 0x03;
			reg_val = (reg_value[reg-1] | LED_MODE_MASK);
			camdrv_ss_RT8547_flash_send_data(reg, reg_val);
			/* set ctlen high & flashen high*/
			gpio_set_value(rt8547_flash_enset, 1);
			gpio_set_value(rt8547_flash_flen, 1);
			} else {
			gpio_set_value(rt8547_flash_flen, 0);
			gpio_set_value(rt8547_flash_enset, 0);

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
			gpio_set_value(rt8547_flash_enset, 1);
			gpio_set_value(rt8547_flash_flen, 1);
		}
		break;
	case SH_RCU_LED_OFF:

		gpio_set_value(rt8547_flash_flen, 0);
		gpio_set_value(rt8547_flash_enset, 0);
		break;
	default:
		gpio_set_value(rt8547_flash_flen, 0);
		gpio_set_value(rt8547_flash_enset, 0);
		udelay(500);
		break;
	}
	gpio_free(rt8547_flash_enset);
	gpio_free(rt8547_flash_flen);

	return 0;
}

void add_primary_cam_flash_rt8547(int gpio_cam_flash_enset,
		int gpio_cam_flash_flen)
{
	rt8547_flash_enset = gpio_cam_flash_enset;
	rt8547_flash_flen = gpio_cam_flash_flen;
	sh_mobile_rcu0_info.led = rt8547_led;
}
