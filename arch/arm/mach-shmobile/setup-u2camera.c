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
#ifdef CONFIG_SOC_CAMERA_ISX012
#include <media/isx012.h>
#endif
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

#if defined(CONFIG_MACH_U2EVM)	/* Guardian */
#if defined(CONFIG_SOC_CAMERA_IMX175) && \
	defined(CONFIG_SOC_CAMERA_S5K6AAFX13) && \
	defined(CONFIG_SOC_CAMERA_ISX012) && \
	defined(CONFIG_SOC_CAMERA_DB8131)	/* Select by board Rev */
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

static struct i2c_board_info i2c_cameras_rev5[] = {
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

static struct soc_camera_link camera_links_rev5[] = {
	{
		.bus_id			= 0,
		.board_info		= &i2c_cameras_rev5[0],
		.i2c_adapter_id	= 1,
		.module_name	= "ISX012",
		.power			= ISX012_power,
	},
	{
		.bus_id			= 1,
		.board_info		= &i2c_cameras_rev5[1],
		.i2c_adapter_id	= 1,
		.module_name	= "DB8131",
		.power			= DB8131_power,
	},
};
#else	/* Select by board Rev */
struct i2c_board_info i2c_cameras[] = {
/* Rear Camera */
#if defined(CONFIG_SOC_CAMERA_IMX175)
	{
		I2C_BOARD_INFO("IMX175", 0x1A),
	},
#endif
#if defined(CONFIG_SOC_CAMERA_ISX012)
	{
		I2C_BOARD_INFO("ISX012", 0x3D),
	},
#endif
/* Front Camera */
#if defined(CONFIG_SOC_CAMERA_S5K6AAFX13)
	{
		I2C_BOARD_INFO("S5K6AAFX13", 0x3C), /* 0x78, 0x5A, 0x45 */
	},
#endif
#if defined(CONFIG_SOC_CAMERA_DB8131)
	{
		I2C_BOARD_INFO("DB8131", 0x45), /* TODO::HYCHO 0x45(0x8A>>1) */
	},
#endif
};

struct soc_camera_link camera_links[] = {
/* Rear Camera */
#if defined(CONFIG_SOC_CAMERA_IMX175)
	{
		.bus_id			= 0,
		.board_info		= &i2c_cameras[0],
		.i2c_adapter_id	= 1,
		.module_name	= "IMX175",
		.power			= IMX175_power,
	},
#endif
#if defined(CONFIG_SOC_CAMERA_ISX012)
	{
		.bus_id			= 0,
		.board_info		= &i2c_cameras[0],
		.i2c_adapter_id	= 1,
		.module_name	= "ISX012",
		.power			= ISX012_power,
	},
#endif
/* Front Camera */
#if defined(CONFIG_SOC_CAMERA_S5K6AAFX13)
	{
		.bus_id			= 1,
		.board_info		= &i2c_cameras[1],
		.i2c_adapter_id	= 1,
		.module_name	= "S5K6AAFX13",
		.power			= S5K6AAFX13_power,
	},
#endif
#if defined(CONFIG_SOC_CAMERA_DB8131)
	{
		.bus_id			= 1,
		.board_info		= &i2c_cameras[1],
		.i2c_adapter_id	= 1,
		.module_name	= "DB8131",
		.power			= DB8131_power,
	},
#endif
};
#endif	/* Select by board Rev */
#endif	/* Guardian */

#if defined(CONFIG_MACH_GARDALTE) || \
	defined(CONFIG_MACH_LOGANLTE) /* Gardalte, Logan */

#if defined(CONFIG_SOC_CAMERA_S5K4ECGX) && \
	defined(CONFIG_SOC_CAMERA_SR030PC50) /* Select by board Rev */
struct i2c_board_info i2c_cameras[] = {
	{
		I2C_BOARD_INFO("S5K4ECGX", 0x1A),
	},
	{
		I2C_BOARD_INFO("SR030PC50", 0x3C), /* 0x78, 0x5A, 0x45 */
	},
};

static struct i2c_board_info i2c_cameras_rev6[] = {
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

static struct soc_camera_link camera_links_rev6[] = {
	{
		.bus_id			= 0,
		.board_info		= &i2c_cameras_rev6[0],
		.i2c_adapter_id	= 1,
		.module_name	= "S5K4ECGX",
		.power			= S5K4ECGX_power,
	},
	{
		.bus_id			= 1,
		.board_info		= &i2c_cameras_rev6[1],
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
#endif	/* Gardalte, Logan */

#if defined(CONFIG_MACH_LT02LTE) /* LT02LTE */
#if defined(CONFIG_SOC_CAMERA_SR352) && \
	defined(CONFIG_SOC_CAMERA_SR130PC20)	/* Select by board Rev */
static struct i2c_board_info i2c_cameras[] = {
	{
		I2C_BOARD_INFO("SR352", 0x20),
	},
	{
		I2C_BOARD_INFO("SR130PC20", 0x28), /* TODO::HYCHO (0x41>>1) */
	},
};
static struct soc_camera_link camera_links[] = {
	{
		.bus_id			= 0,
		.board_info		= &i2c_cameras[0],
		.i2c_adapter_id		= 1,
		.module_name		= "SR352",
		.power			= SR352_power,
	},
	{
		.bus_id			= 1,
		.board_info		= &i2c_cameras[1],
		.i2c_adapter_id	= 1,
		.module_name		= "SR130PC20",
		.power			= SR130PC20_power,
	},
};
#else	/* Select by board Rev */
struct i2c_board_info i2c_cameras[] = {
	/* Rear Camera */
#if defined(CONFIG_SOC_CAMERA_SR352)
	{
		I2C_BOARD_INFO("SR352", 0x20),
	},
#endif
	/* Front Camera */
#if defined(CONFIG_SOC_CAMERA_SR130PC20)
	{
		I2C_BOARD_INFO("SR130PC20", 0x28), /* TODO::HYCHO (0x41>>1) */
	},
#endif
};

struct soc_camera_link camera_links[] = {
	/* Rear Camera */
#if defined(CONFIG_SOC_CAMERA_SR352)
	{
		.bus_id			= 0,
		.board_info		= &i2c_cameras[0],
		.i2c_adapter_id		= 1,
		.module_name		= "SR352",
		.power			= SR352_power,
	},
#endif
	/* Front Camera */
#if defined(CONFIG_SOC_CAMERA_SR130PC20)
	{
		.bus_id			= 1,
		.board_info		= &i2c_cameras[1],
		.i2c_adapter_id	= 1,
		.module_name	= "SR130PC20",
		.power			= SR130PC20_power,
	},
#endif
};
#endif	/* Select by board Rev */
#endif	/* LT02LTE */

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
#if defined(CONFIG_MACH_U2EVM)	/* Guardian */
	if (RLTE_BOARD_REV_0_4_1 <= u2_board_rev) {
		gpio_request(GPIO_PORT45, NULL);
		gpio_direction_output(GPIO_PORT45, 0);	/* CAM0_STBY */
	}
#endif	/* Guardian */
#if defined(CONFIG_MACH_GARDALTE) || defined(CONFIG_MACH_LOGANLTE) \
	|| defined(CONFIG_MACH_LT02LTE) /* Gardalte, Logan, lt02lte */
 /* Gardalte, Logan */
	gpio_request(GPIO_PORT45, NULL);
	gpio_direction_output(GPIO_PORT45, 0);	/* CAM0_STBY */
#endif	/* Gardalte, Logan */

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

#if defined(CONFIG_MACH_U2EVM)	/* Guardian */
#if defined(CONFIG_SOC_CAMERA_IMX175) && \
	defined(CONFIG_SOC_CAMERA_S5K6AAFX13) && \
	defined(CONFIG_SOC_CAMERA_ISX012) && \
	defined(CONFIG_SOC_CAMERA_DB8131)	/* Select by board Rev */
	if (RLTE_BOARD_REV_0_4_1 == u2_board_rev) {
		csi20_info.clients[0].lanes = 0x3;
		camera_devices[0].dev.platform_data = &camera_links_rev4[0];
		camera_devices[1].dev.platform_data = &camera_links_rev4[1];
		camera_links_rev4[0].priv = &csi20_info;
		camera_links_rev4[1].priv = &csi21_info;
	} else if (RLTE_BOARD_REV_0_5_0 <= u2_board_rev) {
		csi20_info.clients[0].lanes = 0x3;
		camera_devices[0].dev.platform_data = &camera_links_rev5[0];
		camera_devices[1].dev.platform_data = &camera_links_rev5[1];
		camera_links_rev5[0].priv = &csi20_info;
		camera_links_rev5[1].priv = &csi21_info;
	}
#else	/* Select by board Rev */
#if defined(CONFIG_SOC_CAMERA_ISX012)
	csi20_info.clients[0].lanes = 0x3;
#endif
	camera_devices[0].dev.platform_data = &camera_links[0];
	camera_devices[1].dev.platform_data = &camera_links[1];
	camera_links[0].priv = &csi20_info;
	camera_links[1].priv = &csi21_info;
#endif	/* Select by board Rev */
#endif	/* Guardian */

#if defined(CONFIG_MACH_GARDALTE) || \
	defined(CONFIG_MACH_LOGANLTE) /* Gardalte, Logan */
#if defined(CONFIG_SOC_CAMERA_S5K4ECGX) && \
	defined(CONFIG_SOC_CAMERA_SR030PC50) /* Select by board Rev */
	csi20_info.clients[0].lanes = 0x3;
	camera_devices[0].dev.platform_data = &camera_links_rev6[0];
	camera_devices[1].dev.platform_data = &camera_links_rev6[1];
	camera_links_rev6[0].priv = &csi20_info;
	camera_links_rev6[1].priv = &csi21_info;
#else	/* Select by board Rev */
#if defined(CONFIG_SOC_CAMERA_S5K4ECGX)
	csi20_info.clients[0].lanes = 0x3;
#endif
#endif	/* Select by board Rev */
#endif	/* Gardalte, Logan */

#if defined(CONFIG_MACH_LT02LTE) /* lt02lte */
#if defined(CONFIG_SOC_CAMERA_SR200PC20M) && \
	defined(CONFIG_SOC_CAMERA_SR130PC20) /* Select by board Rev */
	csi20_info.clients[0].lanes = 0x1;
	camera_devices[0].dev.platform_data = &camera_links_rev7[0];
	camera_devices[1].dev.platform_data = &camera_links_rev7[1];
	camera_links_rev7[0].priv = &csi20_info;
	camera_links_rev7[1].priv = &csi21_info;
#else	/* Select by board Rev */
#if defined(CONFIG_SOC_CAMERA_SR200PC20M)
	csi20_info.clients[0].lanes = 0x1;
#endif
	camera_devices[0].dev.platform_data = &camera_links[0];
	camera_devices[1].dev.platform_data = &camera_links[1];
	camera_links[0].priv = &csi20_info;
	camera_links[1].priv = &csi21_info;
#endif	/* Select by board Rev */

#if defined(CONFIG_SOC_CAMERA_SR352) && \
	defined(CONFIG_SOC_CAMERA_SR130PC20) /* Select by board Rev */
	csi20_info.clients[0].lanes = 0x1;
	camera_devices[0].dev.platform_data = &camera_links[0];
	camera_devices[1].dev.platform_data = &camera_links[1];
	camera_links[0].priv = &csi20_info;
	camera_links[1].priv = &csi21_info;
#else	/* Select by board Rev */
#if defined(CONFIG_SOC_CAMERA_SR352)
	csi20_info.clients[0].lanes = 0x1;
#endif
	camera_devices[0].dev.platform_data = &camera_links[0];
	camera_devices[1].dev.platform_data = &camera_links[1];
	camera_links[0].priv = &csi20_info;
	camera_links[1].priv = &csi21_info;
#endif	/* Select by board Rev */
#endif	/* lr02lte */

	return 0;
}

/* CAM0 Power function */
#if defined(CONFIG_SOC_CAMERA_IMX175)
int IMX175_power(struct device *dev, int power_on)
{
	struct clk *vclk1_clk, *vclk2_clk;
	int iRet;
#if defined(CONFIG_MFD_D2153)
	struct regulator *regulator;
#endif /* CONFIG_MFD_D2153 */

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

#if defined(CONFIG_MACH_U2EVM)
		if (u2_get_board_rev() >= RLTE_BOARD_REV_0_4_1) {
			gpio_set_value(GPIO_PORT45, 0); /* CAM0_STBY */
		}
#endif
#if defined(CONFIG_MACH_GARDALTE) || defined(CONFIG_MACH_LOGANLTE)\
	|| defined(CONFIG_MACH_LT02LTE)
		gpio_set_value(GPIO_PORT45, 0); /* CAM0_STBY */
#endif

		mdelay(10);
		/* 10ms */
#if defined(CONFIG_MACH_U2EVM)
		if (u2_get_board_rev() >= RLTE_BOARD_REV_0_5_0) {
#if defined(CONFIG_MFD_D2153)
			/* cam_sensor_core_1.2V */

			mdelay(1);
			/* cam_sensor_a2.8 */
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
#endif /* CONFIG_MFD_D2153 */
		} else {
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
		}
#endif
#if defined(CONFIG_MACH_GARDALTE) || defined(CONFIG_MACH_LOGANLTE) \
		|| defined(CONFIG_MACH_LT02LTE) /* garda, logan, lt02lte */
#if defined(CONFIG_MFD_D2153)
		/* cam_sensor_core_1.2V */

		mdelay(1);
		/* cam_sensor_a2.8 */
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
#endif /* CONFIG_MFD_D2153 */

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
#if defined(CONFIG_MACH_U2EVM)
		if (u2_get_board_rev() >= RLTE_BOARD_REV_0_5_0) {
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

			/* cam_sensor_a2.8 */
			regulator = regulator_get(NULL, "cam_sensor_a");
			if (IS_ERR(regulator))
				return -1;

			regulator_disable(regulator);
			regulator_put(regulator);

			mdelay(1);

			/* off cam core 1.2 */
#endif /* CONFIG_MFD_D2153 */
		} else {

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
		}
#endif
#if defined(CONFIG_MACH_GARDALTE) || defined(CONFIG_MACH_LOGANLTE)\
	|| defined(CONFIG_MACH_LT02LTE)
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

		/* cam_sensor_a2.8 */
		regulator = regulator_get(NULL, "cam_sensor_a");
		if (IS_ERR(regulator))
			return -1;

		regulator_disable(regulator);
		regulator_put(regulator);

		mdelay(1);

		/* off cam core 1.2 */
#endif /* CONFIG_MFD_D2153 */
#endif
		gpio_set_value(GPIO_PORT3, 0); /* CAM_PWR_EN Low */
		sh_csi2_power(dev, power_on);
		printk(KERN_ALERT "%s PowerOFF fin\n", __func__);
	}

	clk_put(vclk1_clk);
	clk_put(vclk2_clk);

	return 0;
}
#endif

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
#else
		subPMIC_PowerOn(0x0);

		/* CAM_CORE_1V2  On */
		subPMIC_PinOnOff(0x0, 1);
		/* CAM_AVDD_2V8  On */
		subPMIC_PinOnOff(0x4, 1);
		/* VT_DVDD_1V5   On */
		subPMIC_PinOnOff(0x1, 1);
		/* CAM_VDDIO_1V8 On */
		subPMIC_PinOnOff(0x2, 1);
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
#else
		subPMIC_PinOnOff(0x3, 1);
#endif

		printk(KERN_ALERT "%s PowerON fin\n", __func__);
	} else {
		printk(KERN_ALERT "%s PowerOFF\n", __func__);

		gpio_set_value(GPIO_PORT20, 0); /* CAM0_RST_N */
		mdelay(1);

		clk_disable(vclk1_clk);

		gpio_set_value(GPIO_PORT45, 0); /* CAM0_STBY */
		mdelay(1);

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

#endif

#define CAM_FLASH_ENSET     (GPIO_PORT99)
#define CAM_FLASH_FLEN      (GPIO_PORT100)

#if defined(CONFIG_FLASH_AAT1274)
#if !defined(CONFIG_BOARD_VERSION_GARDA) || !defined(CONFIG_MACH_LOGANLTE)\
	|| defined(CONFIG_MACH_LT02LTE)
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
#endif
#endif
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
	udelay(100);
	/* send data */
	for (i = 0; i < (data + 1); i++) {
		gpio_set_value(CAM_FLASH_ENSET, 0);
		udelay(1);
		gpio_set_value(CAM_FLASH_ENSET, 1);
		udelay(1);
	}
	/* wait T end */
	udelay(500);
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
		/* wait T end */
		udelay(500);

		/* write "Disabled"(0) to LB_TH(4) */
		MIC2871_write(4, 0);

		if (mode == SH_RCU_LED_MODE_PRE) {
			/* write 56%(5) to FEN/FCUR(1) */
			/* MIC2871_write(1, 5); */

			/* write 56%(21) to TEN/TCUR(2) */
			MIC2871_write(2, 21);
		} else {
			MIC2871_write(5, 1);
			MIC2871_write(3, 7);
			/* write 100%(0) to FEN/FCUR(1) */
			MIC2871_write(1, 0);
		}

		/* enable */
		gpio_set_value(CAM_FLASH_FLEN, 1);

		spin_unlock_irqrestore(&lock, flags);
		break;
	case SH_RCU_LED_OFF:

		/* initailize falsh IC */
		gpio_set_value(CAM_FLASH_FLEN, 0);
		gpio_set_value(CAM_FLASH_ENSET, 0);
		mdelay(1);
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

/* CAM1 Power function */
#if defined(CONFIG_SOC_CAMERA_S5K6AAFX13)
int S5K6AAFX13_power(struct device *dev, int power_on)
{
	struct clk *vclk1_clk, *vclk2_clk;
	int iRet;
#if defined(CONFIG_MFD_D2153)
	struct regulator *regulator;
#endif /* CONFIG_MFD_D2153 */

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
#if defined(CONFIG_MACH_U2EVM)
		if (u2_get_board_rev() >= RLTE_BOARD_REV_0_4_1) {
			gpio_set_value(GPIO_PORT45, 0); /* CAM0_STBY */
		}
#endif
#if defined(CONFIG_MACH_GARDALTE) || defined(CONFIG_MACH_LOGANLTE)\
	|| defined(CONFIG_MACH_LT02LTE)
		gpio_set_value(GPIO_PORT45, 0); /* CAM0_STBY */
#endif

		mdelay(10);
		/* 10ms */
#if defined(CONFIG_MACH_U2EVM)
		if (u2_get_board_rev() >= RLTE_BOARD_REV_0_5_0) {
#if defined(CONFIG_MFD_D2153)

			/* cam_sensor_a2.8 */
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

#endif /* CONFIG_MFD_D2153 */
		} else {
			subPMIC_PowerOn(0x0);

			/* CAM_CORE_1V2  On */
			/*		subPMIC_PinOnOff(0x0, 1); */
			/*		mdelay(10); */
			/* CAM_AVDD_2V8  On */
			subPMIC_PinOnOff(0x4, 1);
			mdelay(10);
			/* VT_DVDD_1V5   On */
			subPMIC_PinOnOff(0x1, 1);

			mdelay(10);
			/* CAM_VDDIO_1V8 On */
			subPMIC_PinOnOff(0x2, 1);
			mdelay(10);
		}
#endif
#if defined(CONFIG_MACH_GARDALTE) || defined(CONFIG_MACH_LOGANLTE)\
	|| defined(CONFIG_MACH_LT02LTE)
#if defined(CONFIG_MFD_D2153)

		/* cam_sensor_a2.8 */
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

#endif /* CONFIG_MFD_D2153 */
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

		gpio_set_value(GPIO_PORT20, 1); /* CAM0_RST_N Hi */
		mdelay(20);
		/* 20ms */
#if defined(CONFIG_MACH_U2EVM)
		if (u2_get_board_rev() >= RLTE_BOARD_REV_0_5_0) {
#if defined(CONFIG_MFD_D2153)
			regulator = regulator_get(NULL, "cam_af");
			if (IS_ERR(regulator))
				return -1;

			regulator_enable(regulator);
			regulator_put(regulator);
#endif /* CONFIG_MFD_D2153 */
		} else {
			/* 5M_AF_2V8 On */
			subPMIC_PinOnOff(0x3, 1);
		}
#endif
#if defined(CONFIG_MACH_GARDALTE) || defined(CONFIG_MACH_LOGANLTE)\
	|| defined(CONFIG_MACH_LT02LTE)
#if defined(CONFIG_MFD_D2153)
		regulator = regulator_get(NULL, "cam_af");
		if (IS_ERR(regulator))
			return -1;

		regulator_enable(regulator);
		regulator_put(regulator);
#endif /* CONFIG_MFD_D2153 */
#endif
		mdelay(20);
		clk_disable(vclk1_clk);

		printk(KERN_ALERT "%s PowerON fin\n", __func__);
	} else {
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
#if defined(CONFIG_MACH_U2EVM)
		if (u2_get_board_rev() >= RLTE_BOARD_REV_0_5_0) {
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

			/* cam_sensor_a2.8 */
			regulator = regulator_get(NULL, "cam_sensor_a");
			if (IS_ERR(regulator))
				return -1;

			regulator_disable(regulator);
			regulator_put(regulator);

			mdelay(1);
#endif /* CONFIG_MFD_D2153 */
		} else {

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
		}
#endif
#if defined(CONFIG_MACH_GARDALTE) || defined(CONFIG_MACH_LOGANLTE)\
	|| defined(CONFIG_MACH_LT02LTE)
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

		/* cam_sensor_a2.8 */
		regulator = regulator_get(NULL, "cam_sensor_a");
		if (IS_ERR(regulator))
			return -1;

		regulator_disable(regulator);
		regulator_put(regulator);

		mdelay(1);
#endif /* CONFIG_MFD_D2153 */
#endif
		gpio_set_value(GPIO_PORT3, 0); /* CAM_PWR_EN Low */
		sh_csi2_power(dev, power_on);
		printk(KERN_ALERT "%s PowerOFF fin\n", __func__);

	}

	clk_put(vclk1_clk);
	clk_put(vclk2_clk);

	return 0;
}
#endif

#if defined(CONFIG_SOC_CAMERA_ISX012)
int ISX012_power(struct device *dev, int power_on)
{
	struct clk *vclk1_clk, *vclk2_clk;
	int iRet;
#if defined(CONFIG_MFD_D2153)
	struct regulator *regulator;
#endif /* CONFIG_MFD_D2153 */
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
#if defined(CONFIG_MACH_U2EVM)
		if (u2_get_board_rev() >= RLTE_BOARD_REV_0_4_1) {
			gpio_set_value(GPIO_PORT45, 0); /* CAM0_STBY */
		}
#endif
#if defined(CONFIG_MACH_GARDALTE) || defined(CONFIG_MACH_LOGANLTE)\
	|| defined(CONFIG_MACH_LT02LTE)
		gpio_set_value(GPIO_PORT45, 0); /* CAM0_STBY */
#endif

		mdelay(10);
		/* 10ms */
#if defined(CONFIG_MACH_U2EVM)
		if (u2_get_board_rev() >= RLTE_BOARD_REV_0_5_0) {
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
#endif /* CONFIG_MFD_D2153 */
		} else {
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
		}
#endif
#if defined(CONFIG_MACH_GARDALTE) || defined(CONFIG_MACH_LOGANLTE) /* Gardalte, Logan */
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
#endif /* CONFIG_MFD_D2153 */
#endif /* Gardalte, Logan */
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

#if defined(CONFIG_MACH_U2EVM)
		if (u2_get_board_rev() >= RLTE_BOARD_REV_0_4_1) {
			gpio_set_value(GPIO_PORT45, 1); /* CAM0_STBY */
		}
#endif
#if defined(CONFIG_MACH_GARDALTE) || defined(CONFIG_MACH_LOGANLTE)\
	|| defined(CONFIG_MACH_LT02LTE)
			gpio_set_value(GPIO_PORT45, 1); /* CAM0_STBY */
#endif
		mdelay(20);

		/* 5M_AF_2V8 On */
#if defined(CONFIG_MACH_U2EVM)
		if (u2_get_board_rev() >= RLTE_BOARD_REV_0_5_0) {
#if defined(CONFIG_MFD_D2153)
			regulator = regulator_get(NULL, "cam_af");
			if (IS_ERR(regulator))
				return -1;
			regulator_enable(regulator);
			regulator_put(regulator);
#endif /* CONFIG_MFD_D2153 */
		} else {
			subPMIC_PinOnOff(0x3, 1);
		}
#endif
#if defined(CONFIG_MACH_GARDALTE) || defined(CONFIG_MACH_LOGANLTE)\
	|| defined(CONFIG_MACH_LT02LTE)
#if defined(CONFIG_MFD_D2153)
		regulator = regulator_get(NULL, "cam_af");
		if (IS_ERR(regulator))
			return -1;
		regulator_enable(regulator);
		regulator_put(regulator);
#endif /* CONFIG_MFD_D2153 */
#endif
		mdelay(20);

		printk(KERN_ALERT "%s PowerON fin\n", __func__);
	} else {
		printk(KERN_ALERT "%s PowerOFF\n", __func__);

		gpio_set_value(GPIO_PORT20, 0); /* CAM0_RST_N */
#if defined(CONFIG_MACH_U2EVM)
		if (u2_get_board_rev() >= RLTE_BOARD_REV_0_4_1) {
			gpio_set_value(GPIO_PORT45, 0); /* CAM0_STBY */
		}
#endif
#if defined(CONFIG_MACH_GARDALTE) || defined(CONFIG_MACH_LOGANLTE)\
	|| defined(CONFIG_MACH_LT02LTE)
		gpio_set_value(GPIO_PORT45, 0); /* CAM0_STBY */
#endif
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
#if defined(CONFIG_MACH_U2EVM)
		if (u2_get_board_rev() >= RLTE_BOARD_REV_0_5_0) {
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
#endif /* CONFIG_MFD_D2153 */
		} else {
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
		}
#endif
#if defined(CONFIG_MACH_GARDALTE) || defined(CONFIG_MACH_LOGANLTE) \
		|| defined(CONFIG_MACH_LOGANLTE) /* Gardalte, Logan, lt02lte */

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
#endif /* CONFIG_MFD_D2153 */
#endif
		sh_csi2_power(dev, power_on);
		printk(KERN_ALERT "%s PowerOFF fin\n", __func__);
	}

	clk_put(vclk1_clk);
	clk_put(vclk2_clk);

	return 0;
}
#endif

/* CAM1 Power function */
#if defined(CONFIG_SOC_CAMERA_DB8131)
int DB8131_power(struct device *dev, int power_on)
{
	struct clk *vclk1_clk, *vclk2_clk;
	int iRet;
#if defined(CONFIG_MFD_D2153)
	struct regulator *regulator;
#endif /* CONFIG_MFD_D2153 */

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
#if defined(CONFIG_MACH_U2EVM)
		if (u2_get_board_rev() >= RLTE_BOARD_REV_0_4_1) {
			gpio_set_value(GPIO_PORT45, 0); /* CAM0_STBY */
		}
#endif
#if defined(CONFIG_MACH_GARDALTE) || defined(CONFIG_MACH_LOGANLTE)\
	|| defined(CONFIG_MACH_LT02LTE)
		gpio_set_value(GPIO_PORT45, 0); /* CAM0_STBY */
#endif

		mdelay(10);
		/* 10ms */
#if defined(CONFIG_MACH_U2EVM)
		if (u2_get_board_rev() >= RLTE_BOARD_REV_0_5_0) {
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
#endif /* CONFIG_MFD_D2153 */
		} else {
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
		}
#endif
#if defined(CONFIG_MACH_GARDALTE) || defined(CONFIG_MACH_LOGANLTE)\
	|| defined(CONFIG_MACH_LT02LTE)
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
#endif /* CONFIG_MFD_D2153 */
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
#if defined(CONFIG_MACH_U2EVM)
		if (u2_get_board_rev() >= RLTE_BOARD_REV_0_5_0) {
#if defined(CONFIG_MFD_D2153)
			/* 5M_AF_2V8 On */
			regulator = regulator_get(NULL, "cam_af");
			if (IS_ERR(regulator))
				return -1;
			regulator_enable(regulator);
			regulator_put(regulator);
#endif /* CONFIG_MFD_D2153 */
		} else {
			/* 5M_AF_2V8 On */
			subPMIC_PinOnOff(0x3, 1);
		}
#endif
#if defined(CONFIG_MACH_GARDALTE) || defined(CONFIG_MACH_LOGANLTE)\
	|| defined(CONFIG_MACH_LT02LTE
#if defined(CONFIG_MFD_D2153)
		/* 5M_AF_2V8 On */
		regulator = regulator_get(NULL, "cam_af");
		if (IS_ERR(regulator))
			return -1;
		regulator_enable(regulator);
		regulator_put(regulator);
#endif /* CONFIG_MFD_D2153 */
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
#if defined(CONFIG_MACH_U2EVM)
		if (u2_get_board_rev() >= RLTE_BOARD_REV_0_5_0) {
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
			/* cam_sensor_a2.8 */
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
#endif /* CONFIG_MFD_D2153 */
		} else {
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

		}
#endif
#if defined(CONFIG_MACH_GARDALTE) || defined(CONFIG_MACH_LOGANLTE)\
	|| defined(CONFIG_MACH_LT02LTE)
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
		/* cam_sensor_a2.8 */
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
#endif /* CONFIG_MFD_D2153 */
#endif
		gpio_set_value(GPIO_PORT3, 0); /* CAM_PWR_EN Low */
		sh_csi2_power(dev, power_on);
		printk(KERN_ALERT "%s PowerOFF fin\n", __func__);

	}

	clk_put(vclk1_clk);
	clk_put(vclk2_clk);

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

#endif
#if defined(CONFIG_SOC_CAMERA_SR200PC20M)
int SR200PC20M_power(struct device *dev, int power_on)
{
	struct clk *vclk1_clk, *vclk2_clk;
	int iRet;
#if defined(CONFIG_MFD_D2153)
	struct regulator *regulator_io;
	struct regulator *regulator_a;
	struct regulator *regulator_core;
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
		/* gpio_set_value(GPIO_PORT3, 0); */ /* CAM_PWR_EN Low */
		gpio_set_value(GPIO_PORT16, 0); /* CAM1_RST_N */
		gpio_set_value(GPIO_PORT91, 0); /* CAM1_STBY */
		gpio_set_value(GPIO_PORT20, 0); /* CAM0_RST_N */
		gpio_set_value(GPIO_PORT45, 0); /* CAM0_STBY */
		mdelay(10);
		/* 10ms */

#if defined(CONFIG_MFD_D2153)
#if 0
		/* CAM_CORE_1V2  On */
		gpio_set_value(GPIO_PORT3, 1);
		mdelay(1);
#endif

		/* CAM_VDDIO_1V8 Get */
		regulator_io = regulator_get(NULL, "cam_sensor_io");
		if (IS_ERR(regulator_io)) {
			dev_err(dev,
				"regulator_get(cam_sensor_io) failed\n");
			return -1;
		}
		regulator_set_voltage(regulator_io, 1800000, 1800000);
		/* CAM_AVDD_2V8  Get */
		regulator_a = regulator_get(NULL, "cam_sensor_a");
		if (IS_ERR(regulator_a)) {
			dev_err(dev,
				"regulator_get(cam_sensor_a) failed\n");
			return -1;
		}
		regulator_set_voltage(regulator_a, 2800000, 2800000);
		/* VT_DVDD_1V5   Get */
		regulator_core = regulator_get(NULL, "cam_core");
		if (IS_ERR(regulator_core)) {
			dev_err(dev,
				"regulator_get(cam_core) failed\n");
			return -1;
		}
		regulator_set_voltage(regulator_core, 1800000, 1800000);

		dev_err(dev, "regulator_enable s\n");
		/* CAM_VDDIO_1V8 enable */
		regulator_enable(regulator_io);
		/* CAM_AVDD_2V8  enable */
		regulator_enable(regulator_a);
		/* VT_DVDD_1V5   enable */
		regulator_enable(regulator_core);
		dev_err(dev, "regulator_enable e\n");

		/* Regulator free */
		regulator_put(regulator_io);
		regulator_put(regulator_a);
		regulator_put(regulator_core);
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

		gpio_set_value(GPIO_PORT45, 1); /* CAM0_STBY */
		mdelay(30);

		gpio_set_value(GPIO_PORT20, 1); /* CAM0_RST_N Hi */
		mdelay(660);

		/* 5M_AF_2V8 On */
#if defined(CONFIG_MFD_D2153)
#if 0
		regulator = regulator_get(NULL, "cam_af");
		if (IS_ERR(regulator))
			return -1;
		regulator_enable(regulator);
		regulator_put(regulator);
#endif
#else
		subPMIC_PinOnOff(0x3, 1);
#endif
		mdelay(20);

		printk(KERN_ALERT "%s PowerON fin\n", __func__);
	} else {
		printk(KERN_ALERT "%s PowerOFF\n", __func__);

		mdelay(660);

		gpio_set_value(GPIO_PORT20, 0); /* CAM0_RST_N */
		mdelay(10);

		clk_disable(vclk1_clk);

		gpio_set_value(GPIO_PORT45, 0); /* CAM0_STBY */
		mdelay(1);

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
		/* VT_DVDD_1V5   Off */
		regulator_core = regulator_get(NULL, "cam_core");
		if (IS_ERR(regulator_core)) {
			dev_err(dev,
				"regulator_get(cam_core) failed\n");
			return -1;
		}

		/* CAM_AVDD_2V8  Off */
		regulator_a = regulator_get(NULL, "cam_sensor_a");
		if (IS_ERR(regulator_a)) {
			dev_err(dev,
				"regulator_get(cam_sensor_a) failed\n");
			return -1;
		}

		/* CAM_VDDIO_1V8 Off */
		regulator_io = regulator_get(NULL, "cam_sensor_io");
		if (IS_ERR(regulator_io)) {
			dev_err(dev,
				"regulator_get(cam_sensor_io) failed\n");
			return -1;
		}

		dev_err(dev, "regulator_disable s\n");
		/* VT_DVDD_1V5   disable */
		regulator_disable(regulator_core);
		/* CAM_AVDD_2V8  disable */
		regulator_disable(regulator_a);
		/* CAM_VDDIO_1V8 disable */
		regulator_disable(regulator_io);
		dev_err(dev, "regulator_disable e\n");

		/* Regulator free */
		regulator_put(regulator_core);
		regulator_put(regulator_a);
		regulator_put(regulator_io);

#if 0
		gpio_set_value(GPIO_PORT3, 0);
		mdelay(1);

		/* 5M_AF_2V8 Off */
		regulator = regulator_get(NULL, "cam_af");
		if (IS_ERR(regulator))
			return -1;
		regulator_disable(regulator);
		regulator_put(regulator);
#endif
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
#if 0
		gpio_set_value(GPIO_PORT3, 0); /* CAM_PWR_EN Low */
#endif
#endif
		sh_csi2_power(dev, power_on);
		printk(KERN_ALERT "%s PowerOFF fin\n", __func__);
	}

	clk_put(vclk1_clk);
	clk_put(vclk2_clk);

	return 0;
}
#endif
#if defined(CONFIG_SOC_CAMERA_SR352)
int SR352_power(struct device *dev, int power_on)
{
	struct clk *vclk1_clk;
	int iRet;
#if defined(CONFIG_MFD_D2153)
	struct regulator *regulator_io;
	struct regulator *regulator_a;
	struct regulator *regulator_core_0;
	struct regulator *regulator_core_1; 
#endif
	dev_dbg(dev, "%s(): power_on=%d\n", __func__, power_on);

	vclk1_clk = clk_get(NULL, "vclk1_clk");
	if (IS_ERR(vclk1_clk)) {
		dev_err(dev, "clk_get(vclk1_clk) failed\n");
		return -1;
	}

	if (power_on) {
		printk(KERN_ALERT "%s PowerON\n", __func__);
		sh_csi2_power(dev, power_on);
		/* gpio_set_value(GPIO_PORT3, 0); *//* CAM_PWR_EN Low */
		gpio_set_value(GPIO_PORT16, 0); /* CAM1_RST_N */
		gpio_set_value(GPIO_PORT91, 0); /* CAM1_STBY */
		gpio_set_value(GPIO_PORT20, 0); /* CAM0_RST_N */
		gpio_set_value(GPIO_PORT45, 0); /* CAM0_STBY */
		mdelay(10);
		/* 10ms */

#if defined(CONFIG_MFD_D2153)
#if 0
		/* CAM_CORE_1V2  On */
		gpio_set_value(GPIO_PORT3, 1);
		mdelay(1);
#endif

		/* CAM_VDDIO_1V8 Get */
		regulator_io = regulator_get(NULL, "cam_sensor_io");
		if (IS_ERR(regulator_io)) {
			dev_err(dev,
				"regulator_get(cam_sensor_io) failed\n");
			return -1;
		}
		regulator_set_voltage(regulator_io, 1800000, 1800000);
		/* CAM_AVDD_2V8  Get */
		regulator_a = regulator_get(NULL, "cam_sensor_a");
		if (IS_ERR(regulator_a)) {
			dev_err(dev,
				"regulator_get(cam_sensor_a) failed\n");
			return -1;
		}
		regulator_set_voltage(regulator_a, 2800000, 2800000);
		/* CAM_CORE_1_1V8 (VT)	 Get */
		regulator_core_1 = regulator_get(NULL, "cam_core_1");
		if (IS_ERR(regulator_core_1)) {
		   dev_err(dev,
			   "regulator_get(cam_core_1) failed\n");
		   return -1;
		}
		regulator_set_voltage(regulator_core_1, 1800000, 1800000);
		/* CAM_CORE_0_1V2 (MAIN)  Get */
		regulator_core_0 = regulator_get(NULL, "cam_core_0");
		if (IS_ERR(regulator_core_0)) {
			dev_err(dev,
				"regulator_get(cam_core_0) failed\n");
			return -1;
		}
		regulator_set_voltage(regulator_core_0, 1200000, 1200000);

		dev_err(dev, "regulator_enable s\n");
		/* CAM_VDDIO_1V8 enable */
		regulator_enable(regulator_io);
		/* CAM_AVDD_2V8  enable */
		regulator_enable(regulator_a);
		/* CAM_CORE_1_1V8 (VT)	enable */
		regulator_enable(regulator_core_1);
		/* CAM_CORE_0_1V2 (MAIN)  enable */
		regulator_enable(regulator_core_0);
		dev_err(dev, "regulator_enable e\n");

		/* Regulator free */
		regulator_put(regulator_io);
		regulator_put(regulator_a);
		regulator_put(regulator_core_1);
		regulator_put(regulator_core_0);
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

		mdelay(5);

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

		gpio_set_value(GPIO_PORT45, 1); /* CAM0_STBY */
		mdelay(20);

		gpio_set_value(GPIO_PORT20, 1); /* CAM0_RST_N Hi */

		mdelay(10);

		printk(KERN_ALERT "%s PowerON fin\n", __func__);
	} else {
		printk(KERN_ALERT "%s PowerOFF\n", __func__);

		gpio_set_value(GPIO_PORT20, 0); /* CAM0_RST_N */
		mdelay(10);

		clk_disable(vclk1_clk);
		mdelay(1);

		gpio_set_value(GPIO_PORT45, 0); /* CAM0_STBY */
		mdelay(1);

#if defined(CONFIG_MFD_D2153)
		/* CAM_CORE_0_1V2 (MAIN)  off */
		regulator_core_0 = regulator_get(NULL, "cam_core_0");
		if (IS_ERR(regulator_core_0)) {
			dev_err(dev,
				"regulator_get(cam_core) failed\n");
			return -1;
		}

		/* CAM_CORE_1_1V8 (VT)	off */
		regulator_core_1 = regulator_get(NULL, "cam_core_1");
		if (IS_ERR(regulator_core_1)) {
			dev_err(dev,
			   "regulator_get(cam_core_1) failed\n");
			return -1;
		}

		/* CAM_AVDD_2V8  Off */
		regulator_a = regulator_get(NULL, "cam_sensor_a");
		if (IS_ERR(regulator_a)) {
			dev_err(dev,
				"regulator_get(cam_sensor_a) failed\n");
			return -1;
		}

		/* CAM_VDDIO_1V8 Off */
		regulator_io = regulator_get(NULL, "cam_sensor_io");
		if (IS_ERR(regulator_io)) {
			dev_err(dev,
				"regulator_get(cam_sensor_io) failed\n");
			return -1;
		}

		dev_err(dev, "regulator_disable s\n");
		/* CAM_CORE_0_1V2 (MAIN) disable */
		regulator_disable(regulator_core_0);
		/* CAM_CORE_1_1V8 (VT) disable */
		regulator_disable(regulator_core_1);
		/* CAM_AVDD_2V8  disable */
		regulator_disable(regulator_a);
		/* CAM_VDDIO_1V8 disable */
		regulator_disable(regulator_io);
		dev_err(dev, "regulator_disable e\n");

		/* Regulator free */
		regulator_put(regulator_core_0);
		regulator_put(regulator_core_1);	
		regulator_put(regulator_a);
		regulator_put(regulator_io);

#if 0
		gpio_set_value(GPIO_PORT3, 0);
		mdelay(1);

		/* 5M_AF_2V8 Off */
		regulator = regulator_get(NULL, "cam_af");
		if (IS_ERR(regulator))
			return -1;
		regulator_disable(regulator);
		regulator_put(regulator);
#endif
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
#if 0
		gpio_set_value(GPIO_PORT3, 0); /* CAM_PWR_EN Low */
#endif
#endif
		sh_csi2_power(dev, power_on);
		printk(KERN_ALERT "%s PowerOFF fin\n", __func__);
	}

	clk_put(vclk1_clk);

	return 0;
}

#endif
#if defined(CONFIG_SOC_CAMERA_SR130PC20)
int SR130PC20_power(struct device *dev, int power_on)
{
	struct clk *vclk2_clk;
	int iRet;
	struct regulator *regulator_io;
	struct regulator *regulator_a;
	struct regulator *regulator_core_0;
	struct regulator *regulator_core_1; 

	vclk2_clk = clk_get(NULL, "vclk2_clk");
	if (IS_ERR(vclk2_clk)) {
		dev_err(dev, "clk_get(vclk2_clk) failed\n");
		return -1;
	}

	if (power_on) {
		printk(KERN_ALERT "%s PowerON\n", __func__);

	/*
	[SXGA ON] CAM_A2.8V(LDO12) / CAM_IO_1.8V(LDO17) / CAM_CORE_1.8V(LDO14)
	VDD:I(1.8v) -> VDD:A(2.8v) -> VDD:D(1.8v)
	CAM ENB/RESET Low -> SXGA ENB Low(OFF) -> CAM Power High ->
	SXGA_ENB HIGH(ON) -> MCLK 24MHz On -> Delay 10ms -> SXGA RESET High ->
	Delay 100 cycle -> SXGA_ENB LOW(OFF) -> Main CAM Hi-Z Sequence ->
	SXGA_ENB_HIGH(ON) -> Delay 30ms -> SXGA RESET Toggle -> SXGA INIT CODE
	*/

		sh_csi2_power(dev, power_on);
		gpio_set_value(GPIO_PORT3, 0); /* CAM_PWR_EN */
		gpio_set_value(GPIO_PORT16, 0); /* CAM1_RST_N */
		gpio_set_value(GPIO_PORT91, 0); /* CAM1_ENB */
		gpio_set_value(GPIO_PORT20, 0); /* CAM0_RST_N */
		gpio_set_value(GPIO_PORT45, 0); /* CAM0_STBY */

		mdelay(10);

		/* CAM_VDDIO_1V8 On */
		regulator_io = regulator_get(NULL, "cam_sensor_io");
		if (IS_ERR(regulator_io))
			return -1;
		regulator_set_voltage(regulator_io, 1800000, 1800000);
		
		regulator_enable(regulator_io);
		regulator_put(regulator_io);
		mdelay(1);

		/* CAM_AVDD_2V8  On */
		regulator_a = regulator_get(NULL, "cam_sensor_a");
		if (IS_ERR(regulator_a))
			return -1;
		regulator_set_voltage(regulator_a, 2800000, 2800000);
		
		regulator_enable(regulator_a);
		regulator_put(regulator_a);
		mdelay(1);

		/* CAM_CORE_1_1V8 (VT)	 On */
		regulator_core_1 = regulator_get(NULL, "cam_core_1");
		if (IS_ERR(regulator_core_1))
			return -1;
		regulator_set_voltage(regulator_core_1, 1800000, 1800000);

		regulator_enable(regulator_core_1);
		regulator_put(regulator_core_1);
		mdelay(1);

		/* CAM_CORE_0_1V2 (MAIN)  On */
		regulator_core_0 = regulator_get(NULL, "cam_core_0");
		if (IS_ERR(regulator_core_0)) {
			dev_err(dev,
				"regulator_get(cam_core_0) failed\n");
			return -1;
		}
		regulator_set_voltage(regulator_core_0, 1200000, 1200000);
		regulator_enable(regulator_core_0);
		regulator_put(regulator_core_0);		

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
		gpio_set_value(GPIO_PORT91, 1); /* CAM1_ENB */
		mdelay(50);

		gpio_set_value(GPIO_PORT16, 1); /* CAM1_RST_N */
		mdelay(10);

		printk(KERN_ALERT "%s PowerON fin\n", __func__);
	} else {
		printk(KERN_ALERT "%s PowerOFF\n", __func__);

	/*
	[Main CAM OFF]
	Main CAM Power OFF Sequence -> Sleep command->SXGA RESET LOW(OFF) ->
	MCLK OFF -> SXGA_STDBY LOW(OFF) -> VDD:C Low(OFF) ->
	VDD:A Low(OFF) -> VDD:I Low(OFF)
	*/

	/* Main CAM Power-off sequence */

		gpio_set_value(GPIO_PORT16, 0); /* CAM1_RST_N */
		mdelay(1);

		clk_disable(vclk2_clk);

		gpio_set_value(GPIO_PORT91, 0); /* CAM1_ENB */
		mdelay(1);

		/* CAM_CORE_0_1V2 (MAIN)  Off */
		regulator_core_0 = regulator_get(NULL, "cam_core_0");
		if (IS_ERR(regulator_core_0))
			return -1;
		regulator_disable(regulator_core_0);
		regulator_put(regulator_core_0);
		mdelay(1);

		/* CAM_CORE_1_1V8 (VT)	 Off */
		regulator_core_1 = regulator_get(NULL, "cam_core_1");
		if (IS_ERR(regulator_core_1))
			return -1;
		regulator_disable(regulator_core_1);
		regulator_put(regulator_core_1);
		mdelay(1);

		/* CAM_AVDD_2V8  Off */
		regulator_a = regulator_get(NULL, "cam_sensor_a");
		if (IS_ERR(regulator_a))
			return -1;
		regulator_disable(regulator_a);
		regulator_put(regulator_a);
		mdelay(1);

		/* CAM_VDDIO_1V8  Off */
		/* cam_sensor_a2.8 */
		regulator_io = regulator_get(NULL, "cam_sensor_io");
		if (IS_ERR(regulator_io))
			return -1;
		regulator_disable(regulator_io);
		regulator_put(regulator_io);
		mdelay(1);

		sh_csi2_power(dev, power_on);
		printk(KERN_ALERT "%s PowerOFF fin\n", __func__);

	}

	clk_put(vclk2_clk);

	return 0;
}

#endif
