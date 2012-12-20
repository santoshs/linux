#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/pmic/pmic-ncp6914.h>

#include <media/sh_mobile_csi2.h>
#include <media/sh_mobile_rcu.h>
#include <media/soc_camera.h>
#include <media/soc_camera_platform.h>
#ifdef CONFIG_SOC_CAMERA_ISX012
#include <media/isx012.h>
#endif
#ifdef CONFIG_MFD_D2153
#include <linux/d2153/core.h>
#include <linux/d2153/pmic.h>
#include <linux/d2153/d2153_battery.h>
#endif
#include <mach/board-u2evm.h>
#include <mach/setup-u2camera.h>
#include <mach/setup-u2csi2.h>
#include <mach/setup-u2rcu.h>
#include <mach/setup-u2ion.h>
#include <mach/r8a73734.h>

struct i2c_board_info i2c_cameras[] = {
	{
		I2C_BOARD_INFO("IMX175", 0x1A),
	},
	{
		I2C_BOARD_INFO("S5K6AAFX13", 0x3C), /* 0x78, 0x5A, 0x45 */
	},
};

static struct i2c_board_info i2c_cameras_rev4[] = {
	{
		I2C_BOARD_INFO("ISX012", 0x3D),
	},
	{
		I2C_BOARD_INFO("DB8131", 0x45), /* TODO::HYCHO 0x45(0x8A>>1) */
	},
};

struct soc_camera_link camera_links[] = {
	{
		.bus_id			= 0,
		.board_info		= &i2c_cameras[0],
		.i2c_adapter_id	= 1,
		.module_name	= "IMX175",
		.power			= IMX175_power,
	},
	{
		.bus_id			= 1,
		.board_info		= &i2c_cameras[1],
		.i2c_adapter_id	= 1,
		.module_name	= "S5K6AAFX13",
		.power			= S5K6AAFX13_power,
	},
};

static struct soc_camera_link camera_links_rev4[] = {
	{
		.bus_id			= 0,
		.board_info		= &i2c_cameras_rev4[0],
		.i2c_adapter_id	= 1,
		.module_name	= "ISX012",
		.power			= ISX012_power,
	},
	{
		.bus_id			= 1,
		.board_info		= &i2c_cameras_rev4[1],
		.i2c_adapter_id	= 1,
		.module_name	= "DB8131",
		.power			= DB8131_power,
	},
};

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

int camera_init(unsigned int u2_board_rev)
{
	struct clk *vclk1_clk;
	struct clk *pll1_div2_clk;
	int iRet;

	gpio_request(GPIO_PORT3, NULL);
	gpio_direction_output(GPIO_PORT3, 0);	/* CAM_PWR_EN */
	gpio_request(GPIO_PORT20, NULL);
	gpio_direction_output(GPIO_PORT20, 0);	/* CAM0_RST_N */
	gpio_request(GPIO_PORT90, NULL);
	gpio_direction_output(GPIO_PORT90, 0);	/* CAM0_STBY */

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
	if ((system_rev & 0xFFFF) == 0x3E00) {
		printk(KERN_ALERT "Camera ISP ES version switch (ES1)\n");
		csi21_device.resource = csi21_resources_es1;
		csi21_device.num_resources = csi21_resources_es1_size();
		csi21_info.flags |= SH_CSI2_MULTI;
		csi21_info.cmod_name = csi20_info.cmod_name;
		rcu0_device.resource = rcu0_resources_es1;
		rcu1_device.resource = rcu1_resources_es1;
		rcu1_device.num_resources = rcu1_resources_es1_size();
		sh_mobile_rcu1_info.mod_name = sh_mobile_rcu0_info.mod_name;
	} else if (((system_rev & 0xFFFF)>>4) >= 0x3E1)
		printk(KERN_ALERT "Camera ISP ES version switch (ES2)\n");
	if ((1 != u2_board_rev) && (2 != u2_board_rev) &&
		(3 != u2_board_rev)) {
		csi20_info.clients[0].lanes = 0x3;
		camera_devices[0].dev.platform_data = &camera_links_rev4[0];
		camera_devices[1].dev.platform_data = &camera_links_rev4[1];
		camera_links_rev4[0].priv = &csi20_info;
		camera_links_rev4[1].priv = &csi21_info;
	}

	return 0;
}

/* CAM0 Power function */
int IMX175_power(struct device *dev, int power_on)
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

		gpio_set_value(GPIO_PORT3, 1); /* CAM_PWR_EN */
		gpio_set_value(GPIO_PORT16, 0); /* CAM1_RST_N */
		gpio_set_value(GPIO_PORT91, 0); /* CAM1_STBY */
		gpio_set_value(GPIO_PORT20, 0); /* CAM0_RST_N */
		gpio_set_value(GPIO_PORT90, 0); /* CAM0_STBY */
		mdelay(10);
		/* 10ms */

#if defined(CONFIG_MFD_D2153)
		//cam_sensor_core_1.2V

		mdelay(1);
		//cam_sensor_a2.8
		regulator = regulator_get(NULL, "cam_sensor_a");
		if (IS_ERR(regulator))
			return -1;

		regulator_enable(regulator);
		regulator_put(regulator);

		mdelay(1);
		
		regulator = regulator_get(NULL, "vt_cam");
		if (IS_ERR(regulator))
			return -1;
		
		regulator_enable(regulator);		
		regulator_put(regulator);
		mdelay(1);
		
		regulator = regulator_get(NULL, "cam_af");
		if (IS_ERR(regulator))
			return -1;
		
		regulator_enable(regulator);
		regulator_put(regulator);
		mdelay(1);

		regulator = regulator_get(NULL, "cam_sensor_io");
		if (IS_ERR(regulator))
			return -1;
		
		regulator_enable(regulator);
		regulator_put(regulator);
		mdelay(1);

#else
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
		/* 5M_AF_2V8 On */
		subPMIC_PinOnOff(0x3, 1);
		mdelay(1);
		/* CAM_VDDIO_1V8 On */
		subPMIC_PinOnOff(0x2, 1);
		mdelay(1);
#endif
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

		printk(KERN_ALERT "%s PowerON fin\n", __func__);
	} else {
		printk(KERN_ALERT "%s PowerOFF\n", __func__);

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

#if defined(CONFIG_MFD_D2153)

		regulator = regulator_get(NULL, "cam_sensor_io");
		if (IS_ERR(regulator))
			return -1;

		regulator_disable(regulator);
		regulator_put(regulator);
		mdelay(1);

		regulator = regulator_get(NULL, "vt_cam");
		if (IS_ERR(regulator))
			return -1;

		regulator_disable(regulator);		
		regulator_put(regulator);
		mdelay(1);

		//cam_sensor_a2.8
		regulator = regulator_get(NULL, "cam_sensor_a");
		if (IS_ERR(regulator))
			return -1;

		regulator_disable(regulator);
		regulator_put(regulator);

		mdelay(1);	

		//off cam core 1.2
#else

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
#endif
		gpio_set_value(GPIO_PORT3, 0); /* CAM_PWR_EN Low */
		sh_csi2_power(dev, power_on);
		printk(KERN_ALERT "%s PowerOFF fin\n", __func__);
	}

	clk_put(vclk1_clk);
	clk_put(vclk2_clk);

	return 0;
}

#define CAM_FLASH_ENSET     (GPIO_PORT99)
#define CAM_FLASH_FLEN      (GPIO_PORT100)
int main_cam_led(int light, int mode)
{
	int i = 0;

	switch (light) {
	case SH_RCU_LED_ON:
		gpio_request(CAM_FLASH_ENSET, "camacq");
		gpio_request(CAM_FLASH_FLEN, "camacq");

		/* spin_lock(&bl_ctrl_lock); */
		if (mode == SH_RCU_LED_MODE_PRE) { /* temp, torch mode */
			/* initailize flash IC */
			gpio_direction_output(CAM_FLASH_ENSET, 0);
			gpio_direction_output(CAM_FLASH_FLEN, 0);
			mdelay(1);
			/* to enter a shutdown mode */
			/* set to movie mode */
			for (i = 0; i < 3; i++) {
				udelay(1);
				gpio_direction_output(CAM_FLASH_ENSET, 1);
				udelay(1);
				gpio_direction_output(CAM_FLASH_ENSET, 0);
			}
			gpio_direction_output(CAM_FLASH_ENSET, 1);
		} else {
			/* CamacqTraceErr("WINGI AAAAAAAAAAAAAAAAAAA"); */
			/* initailize flash IC */
			gpio_direction_output(CAM_FLASH_ENSET, 0);
			gpio_direction_output(CAM_FLASH_FLEN, 0);
			mdelay(1);
			/* to enter a shutdown mode */
			/* FLEN high */
			gpio_direction_output(CAM_FLASH_FLEN, 1);
			udelay(100);
			/* set to movie mode */
			for (i = 0; i < 4; i++) {
				udelay(1);
				gpio_direction_output(CAM_FLASH_ENSET, 1);
				udelay(1);
				gpio_direction_output(CAM_FLASH_ENSET, 0);
			}
			gpio_direction_output(CAM_FLASH_ENSET, 1);
			mdelay(1);
		}
		gpio_free(CAM_FLASH_ENSET);
		gpio_free(CAM_FLASH_FLEN);
		/* spin_unlock(&bl_ctrl_lock); */
		break;
	case SH_RCU_LED_OFF: {
		gpio_request(CAM_FLASH_ENSET, "ledflash");
		gpio_request(CAM_FLASH_FLEN, "ledflash");
		/* initailize falsh IC */
		gpio_direction_output(CAM_FLASH_ENSET, 0);
		gpio_direction_output(CAM_FLASH_FLEN, 0);
		mdelay(1);
		/* to enter a shutdown mode */
		gpio_free(CAM_FLASH_ENSET);
		gpio_free(CAM_FLASH_FLEN);
	}
		break;
	default:
		printk(KERN_ALERT "%s:not case %d", __func__, light);
		return -1;
		break;
	}
	return 0;
}

/* CAM1 Power function */
int S5K6AAFX13_power(struct device *dev, int power_on)
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
		gpio_set_value(GPIO_PORT3, 1); /* CAM_PWR_EN */
		gpio_set_value(GPIO_PORT16, 0); /* CAM1_RST_N */
		gpio_set_value(GPIO_PORT91, 0); /* CAM1_STBY */
		gpio_set_value(GPIO_PORT20, 0); /* CAM0_RST_N */
		gpio_set_value(GPIO_PORT90, 0); /* CAM0_STBY */

		mdelay(10);
		/* 10ms */
#if defined(CONFIG_MFD_D2153)

		//cam_sensor_a2.8
		regulator = regulator_get(NULL, "cam_sensor_a");
		if (IS_ERR(regulator))
			return -1;

		regulator_enable(regulator);
		regulator_put(regulator);
		mdelay(1);
		
		regulator = regulator_get(NULL, "vt_cam");
		if (IS_ERR(regulator))
			return -1;
		
		regulator_enable(regulator);		
		regulator_put(regulator);
		mdelay(1);
		
		regulator = regulator_get(NULL, "cam_sensor_io");
		if (IS_ERR(regulator))
			return -1;
		
		regulator_enable(regulator);
		regulator_put(regulator);
		mdelay(1);
		
#else
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
#endif

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

#if defined(CONFIG_MFD_D2153)
		regulator = regulator_get(NULL, "cam_af");
		if (IS_ERR(regulator))
			return -1;
		
		regulator_enable(regulator);
		regulator_put(regulator);
#else
		/* 5M_AF_2V8 On */
		subPMIC_PinOnOff(0x3, 1);
#endif
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

		gpio_set_value(GPIO_PORT20, 0); /* CAM0_RST_N */
		mdelay(1);

		clk_disable(vclk2_clk);

		gpio_set_value(GPIO_PORT20, 0); /* CAM0_RST_N */
		mdelay(1);

#if defined(CONFIG_MFD_D2153)		
		regulator = regulator_get(NULL, "cam_sensor_io");

		if (IS_ERR(regulator))
			return -1;

		regulator_disable(regulator);
		regulator_put(regulator);
		mdelay(1);

		regulator = regulator_get(NULL, "vt_cam");
		if (IS_ERR(regulator))
			return -1;

		regulator_disable(regulator);		
		regulator_put(regulator);
		mdelay(1);

		//cam_sensor_a2.8
		regulator = regulator_get(NULL, "cam_sensor_a");
		if (IS_ERR(regulator))
			return -1;

		regulator_disable(regulator);
		regulator_put(regulator);

		mdelay(1);	
#else

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
#endif
		gpio_set_value(GPIO_PORT3, 0); /* CAM_PWR_EN Low */
		sh_csi2_power(dev, power_on);
		printk(KERN_ALERT "%s PowerOFF fin\n", __func__);

	}

	clk_put(vclk1_clk);
	clk_put(vclk2_clk);

	return 0;
}

int ISX012_power(struct device *dev, int power_on)
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
		gpio_set_value(GPIO_PORT90, 0); /* CAM0_STBY */
		mdelay(10);
		/* 10ms */

#if defined(CONFIG_MFD_D2153)
		/* CAM_CORE_1V2  On */
		gpio_set_value(GPIO_PORT3, 1);
		mdelay(1);

		/* CAM_AVDD_2V8  On */
		regulator = regulator_get(NULL, "cam_sensor_a");
		if (IS_ERR(regulator))
			return -1;
		regulator_enable(regulator);
		regulator_put(regulator);
		mdelay(1);

		/* VT_DVDD_1V5   On */
		regulator = regulator_get(NULL, "vt_cam");
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
#else
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
#endif

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

		ISX012_pll_init();

		gpio_set_value(GPIO_PORT90, 1); /* CAM0_STBY */
		mdelay(20);

		/* 5M_AF_2V8 On */
#if defined(CONFIG_MFD_D2153)
		regulator = regulator_get(NULL, "cam_af");
		if (IS_ERR(regulator))
			return -1;
		regulator_enable(regulator);
		regulator_put(regulator);
#else
		subPMIC_PinOnOff(0x3, 1);
#endif
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

#if defined(CONFIG_MFD_D2153)
		/* CAM_VDDIO_1V8 Off */
		regulator = regulator_get(NULL, "cam_sensor_io");
		if (IS_ERR(regulator))
			return -1;
		regulator_disable(regulator);
		regulator_put(regulator);
		mdelay(1);

		/* VT_DVDD_1V5   Off */
		regulator = regulator_get(NULL, "vt_cam");
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

		/* CAM_CORE_1V2  Off */
		gpio_set_value(GPIO_PORT3, 0);
		mdelay(1);

		/* 5M_AF_2V8 Off */
		regulator = regulator_get(NULL, "cam_af");
		if (IS_ERR(regulator))
			return -1;
		regulator_disable(regulator);
		regulator_put(regulator);
#else
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

		gpio_set_value(GPIO_PORT3, 0); /* CAM_PWR_EN Low */
#endif
		sh_csi2_power(dev, power_on);
		printk(KERN_ALERT "%s PowerOFF fin\n", __func__);
	}

	clk_put(vclk1_clk);
	clk_put(vclk2_clk);

	return 0;
}

/* CAM1 Power function */
int DB8131_power(struct device *dev, int power_on)
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
		gpio_set_value(GPIO_PORT3, 1); /* CAM_PWR_EN */
		gpio_set_value(GPIO_PORT16, 0); /* CAM1_RST_N */
		gpio_set_value(GPIO_PORT91, 0); /* TODO::HYCHO CAM1_CEN */
		gpio_set_value(GPIO_PORT20, 0); /* CAM0_RST_N */
		gpio_set_value(GPIO_PORT90, 0); /* CAM0_STBY */

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

		/* VT_DVDD_1V5   On */
		regulator = regulator_get(NULL, "vt_cam");
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
#else
		subPMIC_PowerOn(0x0);

		/* CAM_CORE_1V2  On */
		/* subPMIC_PinOnOff(0x0, 1); */
		/* mdelay(10); */
		/* CAM_AVDD_2V8  On */
		subPMIC_PinOnOff(0x4, 1);
		mdelay(10);
		/* VT_DVDD_1V5   On */
		subPMIC_PinOnOff(0x1, 1);

		mdelay(10);
		/* CAM_VDDIO_1V8 On */
		subPMIC_PinOnOff(0x2, 1);
		mdelay(10);
#endif

		gpio_set_value(GPIO_PORT91, 1); /* CAM1_CEN */
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
		 /* mdelay(150); */
		 /* gpio_set_value(GPIO_PORT91, 0); *//* CAM1_STBY */
		 /* clk_disable(vclk2_clk); */

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

		/* gpio_set_value(GPIO_PORT20, 1); *//* CAM0_RST_N Hi */
		/* mdelay(20); */
		/* 20ms */

#if defined(CONFIG_MFD_D2153)
		/* 5M_AF_2V8 On */
		regulator = regulator_get(NULL, "cam_af");
		if (IS_ERR(regulator))
			return -1;
		regulator_enable(regulator);
		regulator_put(regulator);
#else
		/* 5M_AF_2V8 On */
		subPMIC_PinOnOff(0x3, 1);
#endif
		mdelay(20);
		clk_disable(vclk1_clk);

		printk(KERN_ALERT "%s PowerON fin\n", __func__);
	} else {
		printk(KERN_ALERT "%s PowerOFF\n", __func__);

		gpio_set_value(GPIO_PORT91, 0); /* CAM1_CEN */
		mdelay(1);

		gpio_set_value(GPIO_PORT16, 0); /* CAM1_RST_N */
		mdelay(1);

		gpio_set_value(GPIO_PORT20, 0); /* CAM0_RST_N */
		mdelay(1);

		clk_disable(vclk2_clk);

		gpio_set_value(GPIO_PORT20, 0); /* CAM0_RST_N */
		mdelay(1);

#if defined(CONFIG_MFD_D2153)
		/* CAM_VDDIO_1V8 Off */
		regulator = regulator_get(NULL, "cam_sensor_io");
		if (IS_ERR(regulator))
			return -1;
		regulator_disable(regulator);
		regulator_put(regulator);
		mdelay(1);

		/* VT_DVDD_1V5   Off */
		regulator = regulator_get(NULL, "vt_cam");
		if (IS_ERR(regulator))
			return -1;
		regulator_disable(regulator);
		regulator_put(regulator);
		mdelay(1);

		/* CAM_AVDD_2V8  Off */
		//cam_sensor_a2.8
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
#else
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
		/* subPMIC_PinOnOff(0x0, 0); */
		/* mdelay(1); */

		gpio_set_value(GPIO_PORT3, 0); /* CAM_PWR_EN Low */
#endif
		sh_csi2_power(dev, power_on);
		printk(KERN_ALERT "%s PowerOFF fin\n", __func__);

	}

	clk_put(vclk1_clk);
	clk_put(vclk2_clk);

	return 0;
}

