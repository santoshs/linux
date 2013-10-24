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
#include <mach/board.h>
#include <mach/setup-u2camera.h>
#include <mach/setup-u2csi2.h>
#include <mach/setup-u2rcu.h>


#ifdef CONFIG_MACH_LOGANLTE
static struct i2c_board_info i2c_cameras[] = {
	{
		I2C_BOARD_INFO("S5K4ECGX", 0x56),
	},
	{
		I2C_BOARD_INFO("SR030PC50", 0x30), /* TODO::HYCHO (0x61>>1) */
	},
};

static struct soc_camera_link camera_links[] = {
	{
		.bus_id                 = 0,
		.board_info             = &i2c_cameras[0],
		.i2c_adapter_id = 1,
		.module_name    = "S5K4ECGX",
		.power                  = S5K4ECGX_power,
	},
	{
		.bus_id                 = 1,
		.board_info             = &i2c_cameras[1],
		.i2c_adapter_id = 1,
		.module_name    = "SR030PC50",
		.power                  = SR030PC50_power,
	},
};
#endif
#ifdef CONFIG_MACH_AMETHYST
static struct i2c_board_info i2c_cameras[] = {
	{
		I2C_BOARD_INFO("OV5645", 0x20),
	},
	{
		I2C_BOARD_INFO("HM2056", 0x28), /* TODO::HYCHO (0x61>>1) */
	},
};

static struct soc_camera_link camera_links[] = {
	{
		.bus_id                 = 0,
		.board_info             = &i2c_cameras[0],
		.i2c_adapter_id = 1,
		.module_name    = "OV5645",
		.power                  = OV5645_power,
	},
	{
		.bus_id                 = 1,
		.board_info             = &i2c_cameras[1],
		.i2c_adapter_id = 1,
		.module_name    = "HM2056",
		.power                  = HM2056_power,
	},
};
#endif

struct platform_device camera_devices[] = {
	{
		.name   = "soc-camera-pdrv",
		.id             = 0,
		.dev    = {
		.platform_data = &camera_links[0],
		},
	},
	{
		.name   = "soc-camera-pdrv",
		.id     =       1,
		.dev    = {
		.platform_data = &camera_links[1],
		},
	},
};


int camera_init(void)
{
	struct clk *vclk1_clk;
	struct clk *pll1_div2_clk;
	int iRet;

	gpio_request(GPIO_PORT3, NULL);
	gpio_direction_output(GPIO_PORT3, 0);   /* CAM_PWR_EN */
	gpio_request(GPIO_PORT20, NULL);
	gpio_direction_output(GPIO_PORT20, 0);  /* CAM0_RST_N */
	gpio_request(GPIO_PORT45, NULL);
	gpio_direction_output(GPIO_PORT45, 0);  /* CAM0_STBY */

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

	camera_links[0].priv = &csi20_info;
	camera_links[1].priv = &csi21_info;
	printk(KERN_ALERT "Camera ISP ES version switch (ES2)\n");
	csi20_info.clients[0].lanes = 0x3;

	clk_put(vclk1_clk);
	clk_put(pll1_div2_clk);

	return 0;
}

#define CAM_FLASH_ENSET     (GPIO_PORT99)
#define CAM_FLASH_FLEN      (GPIO_PORT100)

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
