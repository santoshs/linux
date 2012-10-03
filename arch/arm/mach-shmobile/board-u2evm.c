/*
 * u2evm board support
 *
 * Copyright (C) 2012  Renesas Electronics Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/gpio.h>
#include <linux/hwspinlock.h>
#include <linux/smsc911x.h>
#include <linux/mmc/host.h>
#include <linux/mmc/renesas_mmcif.h>
#include <linux/mmc/renesas_sdhi.h>
#include <linux/input.h>
#include <linux/input/sh_keysc.h>
#include <linux/gpio_keys.h>
#include <linux/platform_data/leds-renesas-tpu.h>
#include <linux/mfd/tps80031.h>
#include <linux/i2c/atmel_mxt_ts.h>
#include <linux/regulator/tps80031-regulator.h>
#include <linux/regulator/fixed.h>
#include <linux/usb/r8a66597.h>
#include <linux/videodev2.h>
#include <video/sh_mobile_lcdc.h>
#include <video/sh_mipi_dsi.h>
#include <mach/hardware.h>
#include <mach/irqs.h>
#include <mach/common.h>
#include <mach/r8a7373.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/hardware/gic.h>

#define GPIO_BASE	0xe6050000
#define GPIO_PORTCR(n)	({				\
	((n) < 128) ? (GPIO_BASE + 0x0000 + (n)) :	\
	((n) < 198) ? (GPIO_BASE + 0x1000 + (n)) :	\
	((n) < 328) ? (GPIO_BASE + 0x2000 + (n)) : 0; })

#define ENT_TPS80031_IRQ_BASE	(R8A7373_IRQC_BASE + 64)

#define SRCR2		0xe61580b0
#define SRCR3		0xe61580b8

/* Ether */
static struct resource smsc9220_resources[] = {
	{
		.start	= 0x00080000,
		.end	= 0x00080000 + SZ_64K - 1,
		.flags	= IORESOURCE_MEM,
	}, {
		.start	= R8A7373_IRQC_IRQ(41),
		.flags	= IORESOURCE_IRQ | IRQ_TYPE_LEVEL_LOW,
	},
};

static struct smsc911x_platform_config smsc9220_platdata = {
	.flags		= SMSC911X_USE_32BIT,
	.phy_interface	= PHY_INTERFACE_MODE_MII,
	.irq_polarity	= SMSC911X_IRQ_POLARITY_ACTIVE_LOW,
	.irq_type	= SMSC911X_IRQ_TYPE_PUSH_PULL,
};

static struct platform_device eth_device = {
	.name		= "smsc911x",
	.id		= 0,
	.dev	= {
		.platform_data = &smsc9220_platdata,
	},
	.resource	= smsc9220_resources,
	.num_resources	= ARRAY_SIZE(smsc9220_resources),
};

static struct regulator_consumer_supply dummy_supplies[] = {
	REGULATOR_SUPPLY("vddvario", "smsc911x.0"),
	REGULATOR_SUPPLY("vdd33a", "smsc911x.0"),
};

/* USBHS */
static void usbhs_module_reset(void)
{
	unsigned long flags;

	hwspin_lock_timeout_irqsave(r8a7373_hwlock_cpg, 1000, &flags);

	__raw_writel(__raw_readl(SRCR2) | (1 << 14), SRCR2); /* USBHS-DMAC0 */
	__raw_writel(__raw_readl(SRCR3) | (1 << 22), SRCR3); /* USBHS */

	udelay(50); /* wait for at least one EXTALR cycle */

	__raw_writel(__raw_readl(SRCR2) & ~(1 << 14), SRCR2);
	__raw_writel(__raw_readl(SRCR3) & ~(1 << 22), SRCR3);

	hwspin_unlock_irqrestore(r8a7373_hwlock_cpg, &flags);
}

static struct r8a66597_platdata usbhs_func_data = {
	.module_start	= usbhs_module_reset,
	.module_stop	= usbhs_module_reset,
	.on_chip	= 1,
	.buswait	= 5,
	.max_bufnum	= 0xff,
	.dmac		= 1,
};

static struct resource usbhs_resources[] = {
	[0] = {
		.start	= 0xe6890000,
		.end	= 0xe6890150 - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= gic_spi(87) /* USBULPI */,
		.flags	= IORESOURCE_IRQ,
	},
	[2] = {
		.name	= "dmac",
		.start	= 0xe68a0000,
		.end	= 0xe68a0064 - 1,
		.flags	= IORESOURCE_MEM,
	},
	[3] = {
		.start	= gic_spi(85) /* USBHSDMAC1 */,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device usbhs_func_device = {
	.name	= "r8a66597_udc",
	.id	= 0,
	.dev = {
		.dma_mask		= NULL,
		.coherent_dma_mask	= DMA_BIT_MASK(32),
		.platform_data		= &usbhs_func_data,
	},
	.num_resources	= ARRAY_SIZE(usbhs_resources),
	.resource	= usbhs_resources,
};

/* MMCIF */
static void mmcif_set_pwr(struct platform_device *pdev, int state)
{
#ifdef CONFIG_PMIC_INTERFACE
	pmic_set_power_on(E_POWER_VMMC);
#endif
}

static void mmcif_down_pwr(struct platform_device *pdev)
{
#ifdef CONFIG_PMIC_INTERFACE
	pmic_set_power_off(E_POWER_VMMC);
#endif
}

static struct sh_mmcif_plat_data renesas_mmcif_plat = {
	.sup_pclk	= 0,
	.ocr		= MMC_VDD_165_195 | MMC_VDD_32_33 | MMC_VDD_33_34,
	.caps		= MMC_CAP_4_BIT_DATA | MMC_CAP_8_BIT_DATA |
			  MMC_CAP_1_8V_DDR | MMC_CAP_UHS_DDR50 |
			  MMC_CAP_NONREMOVABLE,
	.set_pwr	= mmcif_set_pwr,
	.down_pwr	= mmcif_down_pwr,
	.slave_id_tx	= SHDMA_SLAVE_MMCIF0_TX,
	.slave_id_rx	= SHDMA_SLAVE_MMCIF0_RX,
	.max_clk	= 26000000,
};

static struct resource renesas_mmcif_resources[] = {
	[0] = {
		.name	= "MMCIF",
		.start	= 0xe6bd0000,
		.end	= 0xe6bd00ff,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= gic_spi(122),
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device mmcif_device = {
	.name		= "renesas_mmcif",
	.id		= 0,
	.dev		= {
		.platform_data	= &renesas_mmcif_plat,
	},
	.num_resources	= ARRAY_SIZE(renesas_mmcif_resources),
	.resource	= renesas_mmcif_resources,
};

/* SDHI0 */
static void sdhi0_set_pwr(struct platform_device *pdev, int state)
{
	;
}

static int sdhi0_get_cd(struct platform_device *pdev)
{
	return gpio_get_value(GPIO_PORT327) ? 0 : 1;
}

#define SDHI0_EXT_ACC	0xee1000e4
#define SDHI0_DMACR	0xee108000

static void sdhi0_set_dma(struct platform_device *pdev, int size)
{
	static void __iomem *dmacr, *ext_acc;
	u32 val, val2;

	if (!dmacr)
		dmacr = ioremap_nocache(SDHI0_DMACR, 4);
	if (!ext_acc)
		ext_acc = ioremap_nocache(SDHI0_EXT_ACC, 4);

	switch (size) {
	case 32:
		val = 0x30;
		val2 = 1;
		break;
	case 16:
		val = 0x03;
		val2 = 1;
		break;
	default:
		val = 0x00;
		val2 = 0;
		break;
	}
	__raw_writew(val, dmacr);
	__raw_writew(val2, ext_acc);
}

static struct renesas_sdhi_platdata sdhi0_info = {
	.caps		= 0,
	.flags		= RENESAS_SDHI_SDCLK_OFFEN | RENESAS_SDHI_WP_DISABLE
				| RENESAS_SDHI_DMA_SLAVE_CONFIG,
	.slave_id_tx	= SHDMA_SLAVE_SDHI0_TX,
	.slave_id_rx	= SHDMA_SLAVE_SDHI0_RX,
	.set_pwr	= sdhi0_set_pwr,
	.detect_irq	= R8A7373_IRQC_IRQ(50),
	.detect_msec	= 0,
	.get_cd		= sdhi0_get_cd,
	.set_dma	= sdhi0_set_dma,
};

static struct resource sdhi0_resources[] = {
	[0] = {
		.name	= "SDHI0",
		.start	= 0xee100000,
		.end	= 0xee1000ff,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= gic_spi(118),
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device sdhi0_device = {
	.name		= "renesas_sdhi",
	.id		= 0,
	.dev		= {
		.platform_data	= &sdhi0_info,
	},
	.num_resources	= ARRAY_SIZE(sdhi0_resources),
	.resource	= sdhi0_resources,
};

/* SDHI1 */
static void sdhi1_set_pwr(struct platform_device *pdev, int state)
{
	;
}

#define SDHI1_VOLTAGE (MMC_VDD_165_195 | MMC_VDD_20_21 | MMC_VDD_21_22 \
			| MMC_VDD_22_23 | MMC_VDD_23_24 | MMC_VDD_24_25 \
			| MMC_VDD_25_26 | MMC_VDD_26_27 | MMC_VDD_27_28 \
			| MMC_VDD_28_29 | MMC_VDD_29_30 | MMC_VDD_30_31 \
			| MMC_VDD_31_32 | MMC_VDD_32_33 | MMC_VDD_33_34 \
			| MMC_VDD_34_35 | MMC_VDD_35_36)

static struct renesas_sdhi_platdata sdhi1_info = {
	.caps		= MMC_CAP_SDIO_IRQ | MMC_CAP_POWER_OFF_CARD \
				| MMC_CAP_NONREMOVABLE | MMC_PM_KEEP_POWER,
	.flags		= RENESAS_SDHI_SDCLK_OFFEN | RENESAS_SDHI_WP_DISABLE,
	.slave_id_tx	= SHDMA_SLAVE_SDHI1_TX,
	.slave_id_rx	= SHDMA_SLAVE_SDHI1_RX,
	.set_pwr	= sdhi1_set_pwr,
};

static struct resource sdhi1_resources[] = {
	[0] = {
		.name	= "SDHI1",
		.start	= 0xee120000,
		.end	= 0xee1200ff,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= gic_spi(119),
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device sdhi1_device = {
	.name		= "renesas_sdhi",
	.id		= 1,
	.dev		= {
		.platform_data	= &sdhi1_info,
	},
	.num_resources	= ARRAY_SIZE(sdhi1_resources),
	.resource	= sdhi1_resources,
};

/* SDHI2 */
static void sdhi2_set_pwr(struct platform_device *pdev, int state)
{
	;
}

static struct renesas_sdhi_platdata sdhi2_info = {
	.caps		= MMC_CAP_SDIO_IRQ | MMC_CAP_POWER_OFF_CARD \
				| MMC_CAP_NONREMOVABLE | MMC_PM_KEEP_POWER,
	.flags		= RENESAS_SDHI_SDCLK_OFFEN | RENESAS_SDHI_WP_DISABLE,
	.slave_id_tx	= SHDMA_SLAVE_SDHI2_TX,
	.slave_id_rx	= SHDMA_SLAVE_SDHI2_RX,
	.set_pwr	= sdhi2_set_pwr,
};

static struct resource sdhi2_resources[] = {
	[0] = {
		.name	= "SDHI2",
		.start	= 0xee140000,
		.end	= 0xee1400ff,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= gic_spi(120),
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device sdhi2_device = {
	.name		= "renesas_sdhi",
	.id		= 2,
	.dev		= {
		.platform_data	= &sdhi2_info,
	},
	.num_resources	= ARRAY_SIZE(sdhi2_resources),
	.resource	= sdhi2_resources,
};

/* KEYSC */
static struct sh_keysc_info keysc_platdata = {
	.mode		= SH_KEYSC_MODE_6,
	.scan_timing	= 3,
	.delay		= 100,
	.automode	= 1,
	.keycodes	= {
		KEY_NUMERIC_STAR, KEY_0, KEY_NUMERIC_POUND, 0, 0, 0, 0, 0,
		KEY_7, KEY_8, KEY_9, 0, KEY_DOWN, 0, 0, 0,
		KEY_4, KEY_5, KEY_6, KEY_LEFT, KEY_ENTER, KEY_RIGHT, 0, 0,
		KEY_1, KEY_2, KEY_3, 0, KEY_UP, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	},
};

static struct resource keysc_resources[] = {
	[0] = {
		.name	= "KEYSC",
		.start	= 0xe61b0000,
		.end	= 0xe61b0098 - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= gic_spi(71),
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device keysc_device = {
	.name		= "sh_keysc",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(keysc_resources),
	.resource	= keysc_resources,
	.dev		= {
		.platform_data	= &keysc_platdata,
	},
};

/* GPIO-KEY */
#define GPIO_KEY(c, g, d, w) \
	{ .code = c, .gpio = g, .desc = d, .wakeup = w, .active_low = 1, \
	  .debounce_interval = 20 }

static struct gpio_keys_button gpio_buttons[] = {
	GPIO_KEY(KEY_POWER, GPIO_PORT24, "Power", 1),
	GPIO_KEY(KEY_MENU, GPIO_PORT25, "Menu", 1),
	GPIO_KEY(KEY_HOME, GPIO_PORT26, "Home", 1),
	GPIO_KEY(KEY_BACK, GPIO_PORT27, "Back", 1),
	GPIO_KEY(KEY_VOLUMEUP, GPIO_PORT1, "+", 0),
	GPIO_KEY(KEY_VOLUMEDOWN, GPIO_PORT2, "-", 0),
};

static struct gpio_keys_platform_data gpio_key_info = {
	.buttons	= gpio_buttons,
	.nbuttons	= ARRAY_SIZE(gpio_buttons),
	.rep		= 0,
};

static struct platform_device gpio_key_device = {
	.name	= "gpio-keys",
	.id	= -1,
	.dev	= {
		.platform_data	= &gpio_key_info,
	},
};

/* LCDC0 */
static const struct fb_videomode lcdc0_modes[] = {
	{
		.name		= "WVGA",
		.xres		= 480,
		.yres		= 864,
		.left_margin	= 16,
		.right_margin	= 1000,
		.hsync_len	= 16,
		.upper_margin	= 1,
		.lower_margin	= 4,
		.vsync_len	= 2,
		.sync		= FB_SYNC_VERT_HIGH_ACT | FB_SYNC_HOR_HIGH_ACT,
	},
};

static struct platform_device mipidsi0_device;

static struct sh_mobile_lcdc_info lcdc_info = {
	.clock_source	= LCDC_CLK_PERIPHERAL,

	/* LCDC0 */
	.ch[0] = {
		.chan = LCDC_CHAN_MAINLCD,
		.interface_type = RGB24,
		.clock_divider = 1,
		.flags = LCDC_FLAGS_DWPOL,
		.fourcc = V4L2_PIX_FMT_BGR32,
		.lcd_modes = lcdc0_modes,
		.num_modes = ARRAY_SIZE(lcdc0_modes),
		.panel_cfg = {
			.width	= 44,
			.height	= 79,
		},
		.tx_dev = &mipidsi0_device,
	},
};

static struct resource lcdc_resources[] = {
	[0] = {
		.name	= "LCDC",
		.start	= 0xfe940000,
		.end	= 0xfe943fff,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= intcs_evt2irq(0x580),
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device lcdc_device = {
	.name		= "sh_mobile_lcdc_fb",
	.num_resources	= ARRAY_SIZE(lcdc_resources),
	.resource	= lcdc_resources,
	.dev	= {
		.platform_data = &lcdc_info,
		.coherent_dma_mask = DMA_BIT_MASK(32),
	},
};

/* MIPI-DSI0 */
static struct resource mipidsi0_resources[] = {
	[0] = {
		.start	= 0xfeab0000,
		.end	= 0xfeab3fff,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= 0xfeab4000,
		.end	= 0xfeab7fff,
		.flags	= IORESOURCE_MEM,
	},
};

static int sh_mipi_set_dot_clock(struct platform_device *pdev,
				 void __iomem *base,
				 int enable)
{
#if 0
	struct clk *pck;
	int ret = 0;

	pck = clk_get(&pdev->dev, "dsip_clk");
	if (IS_ERR(pck)) {
		ret = PTR_ERR(pck);
		goto sh_mipi_set_dot_clock_pck_err;
	}

	if (enable) {
		clk_set_rate(pck, clk_round_rate(pck, 24000000));
		clk_enable(pck);
	} else {
		clk_disable(pck);
	}

	clk_put(pck);
sh_mipi_set_dot_clock_pck_err:
	return ret;
#else
	/*
	 * FIXME - do not use .set_dot_clock to start DSIP clock for now,
	 * to get Suspend-to-RAM to work.  Will revisit later.
	 */
	return 0;
#endif
}

static struct sh_mipi_dsi_info mipidsi0_info = {
	.data_format	= MIPI_RGB888,
	.lane		= 2,
	.lcd_chan	= &lcdc_info.ch[0],
	.vsynw_offset	= 20,
	.clksrc		= 1,
	.flags		= SH_MIPI_DSI_HSABM,
	.set_dot_clock	= sh_mipi_set_dot_clock,
};

static struct platform_device mipidsi0_device = {
	.name		= "sh-mipi-dsi",
	.num_resources	= ARRAY_SIZE(mipidsi0_resources),
	.resource	= mipidsi0_resources,
	.id		= 0,
	.dev	= {
		.platform_data	= &mipidsi0_info,
	},
};

/* TPU03 backlight */
static struct led_renesas_tpu_config tpu3_info = {
	.name		= "lcd-backlight",
	.pin_gpio_fn	= GPIO_FN_TPU0TO3,
	.pin_gpio	= GPIO_PORT39,
	.channel_offset	= 0x00d0,
	.timer_bit	= 3,
	.max_brightness	= LED_FULL,
	.init_brightness = LED_FULL,
	.refresh_rate	= 2000,
};

static struct resource tpu3_resources[] = {
	[0] = {
		.start	= 0xe66000d0,
		.end	= 0xe66000ff,
		.flags	= IORESOURCE_MEM,
	},
};

static struct platform_device tpu3_device = {
	.name		= "leds-renesas-tpu",
	.num_resources	= ARRAY_SIZE(tpu3_resources),
	.resource	= tpu3_resources,
	.id		= 3,
	.dev		= {
		.platform_data	= &tpu3_info,
	},
};

static struct platform_device *u2evm_devices[] __initdata = {
	&usbhs_func_device,
	&eth_device,
	&keysc_device,
	&mmcif_device,
	&sdhi0_device,
	&sdhi1_device,
	&sdhi2_device,
	&gpio_key_device,
	&lcdc_device,
	&mipidsi0_device,
	&tpu3_device,
};

/* I2C devices */
static struct regulator_consumer_supply tps80031_ldo5_supply[] = {
	REGULATOR_SUPPLY("vdd_touch", NULL),
};

#define TPS_PDATA_INIT(_id, _minmv, _maxmv, _supply_reg, _always_on,	\
	_boot_on, _apply_uv, _init_uV, _init_enable, _init_apply,	\
	_flags, _delay)						\
	static struct tps80031_regulator_platform_data pdata_##_id = {	\
		.regulator = {						\
			.constraints = {				\
				.min_uV = (_minmv)*1000,		\
				.max_uV = (_maxmv)*1000,		\
				.valid_modes_mask = (REGULATOR_MODE_NORMAL |  \
						REGULATOR_MODE_STANDBY),      \
				.valid_ops_mask = (REGULATOR_CHANGE_MODE |    \
						REGULATOR_CHANGE_STATUS |     \
						REGULATOR_CHANGE_VOLTAGE),    \
				.always_on = _always_on,		\
				.boot_on = _boot_on,			\
				.apply_uV = _apply_uv,			\
			},						\
			.num_consumer_supplies =			\
				ARRAY_SIZE(tps80031_##_id##_supply),	\
			.consumer_supplies = tps80031_##_id##_supply,	\
			.supply_regulator = _supply_reg,		\
		},							\
		.init_uV = _init_uV * 1000,				\
		.init_enable = _init_enable,				\
		.init_apply = _init_apply,				\
		.flags = _flags,					\
		.delay_us = _delay,					\
	}

TPS_PDATA_INIT(ldo5, 1000, 3300, 0, 0, 0, 0, 2700, 0, 1, 0, 0);

static struct tps80031_rtc_platform_data rtc_data = {
	.irq = ENT_TPS80031_IRQ_BASE + TPS80031_INT_RTC_ALARM,
	.time = {
		.tm_year = 2012,
		.tm_mon = 0,
		.tm_mday = 1,
		.tm_hour = 1,
		.tm_min = 2,
		.tm_sec = 3,
	},
};

#define TPS_REG(_id, _data)				\
	{						\
		.id	= TPS80031_ID_##_id,		\
		.name	= "tps80031-regulator",		\
		.platform_data = &pdata_##_data,	\
	}

#define TPS_RTC()				\
	{					\
		.id	= 0,			\
		.name	= "rtc_tps80031",	\
		.platform_data = &rtc_data,	\
	}

static struct tps80031_subdev_info tps80031_devs[] = {
	TPS_RTC(),
	TPS_REG(LDO5, ldo5),
};

static struct tps80031_platform_data tps_platform = {
	.num_subdevs	= ARRAY_SIZE(tps80031_devs),
	.subdevs	= tps80031_devs,
	.irq_base	= ENT_TPS80031_IRQ_BASE,
};

static struct i2c_board_info __initdata i2c0_devices[] = {
	{
		I2C_BOARD_INFO("tps80032", 0x4A),
		.irq		= R8A7373_IRQC_IRQ(28),
		.platform_data	= &tps_platform,
	},
};

static struct regulator *mxt224_regulator;

static void mxt224_set_power(int on)
{
	if (!mxt224_regulator)
		mxt224_regulator = regulator_get(NULL, "vdd_touch");

	if (mxt224_regulator) {
		if (on)
			regulator_enable(mxt224_regulator);
		else
			regulator_disable(mxt224_regulator);
	}
}

static int mxt224_read_chg(void)
{
	return gpio_get_value(GPIO_PORT32);
}

static struct mxt_platform_data mxt224_platform_data = {
	.x_line		= 19,
	.y_line		= 11,
	.x_size		= 864,
	.y_size		= 480,
	.blen		= 0x21,
	.threshold	= 0x28,
	.voltage	= 1825000,
	.orient		= MXT_DIAGONAL,
	.irqflags	= IRQF_TRIGGER_FALLING,
	.set_pwr	= mxt224_set_power,
	.read_chg	= mxt224_read_chg,
};

static struct i2c_board_info i2c4_devices[] = {
	{
		I2C_BOARD_INFO("atmel_mxt_ts", 0x4b),
		.platform_data = &mxt224_platform_data,
		.irq	= R8A7373_IRQC_IRQ(32),
	},
};

#define DSI0PHYCR	0xe615006c

static void __init u2evm_init(void)
{
	struct clk *dsip_clk;

	r8a7373_pinmux_init();

	/* SCIFA0 */
	gpio_request(GPIO_FN_SCIFA0_TXD, NULL);
	gpio_request(GPIO_FN_SCIFA0_RXD, NULL);

	/* SCIFB0 */
	gpio_request(GPIO_FN_SCIFB0_TXD, NULL);
	gpio_request(GPIO_FN_SCIFB0_RXD, NULL);
	gpio_request(GPIO_FN_SCIFB0_CTS_, NULL);
	gpio_request(GPIO_FN_SCIFB0_RTS_, NULL);

	/* enable KEYSC */
	gpio_request(GPIO_FN_KEYIN0, NULL);
	gpio_request(GPIO_FN_KEYIN1, NULL);
	gpio_request(GPIO_FN_KEYIN2, NULL);
	gpio_request(GPIO_FN_KEYIN3, NULL);
	gpio_request(GPIO_FN_KEYIN4, NULL);
	gpio_request(GPIO_FN_KEYIN5, NULL);
	gpio_request(GPIO_FN_KEYIN6, NULL);
	gpio_request(GPIO_FN_KEYOUT0, NULL);
	gpio_request(GPIO_FN_KEYOUT1, NULL);
	gpio_request(GPIO_FN_KEYOUT2, NULL);
	gpio_request(GPIO_FN_KEYOUT3, NULL);
	gpio_request(GPIO_FN_KEYOUT4, NULL);
	gpio_request(GPIO_FN_KEYOUT5, NULL);
	gpio_request(GPIO_FN_KEYOUT6, NULL);
	gpio_pull_up(GPIO_PORTCR(44));
	gpio_pull_up(GPIO_PORTCR(45));
	gpio_pull_up(GPIO_PORTCR(46));
	gpio_pull_up(GPIO_PORTCR(47));
	gpio_pull_up(GPIO_PORTCR(48));
	gpio_pull_up(GPIO_PORTCR(96));
	gpio_pull_up(GPIO_PORTCR(97));

	/* gpio_key */
	gpio_pull_up(GPIO_PORTCR(24));
	gpio_pull_up(GPIO_PORTCR(25));
	gpio_pull_up(GPIO_PORTCR(26));
	gpio_pull_up(GPIO_PORTCR(27));
	gpio_pull_up(GPIO_PORTCR(1));
	gpio_pull_up(GPIO_PORTCR(2));

	/* MMC0 */
	gpio_request(GPIO_FN_MMCCLK0, NULL);
	gpio_request(GPIO_FN_MMCD0_0, NULL);
	gpio_request(GPIO_FN_MMCD0_1, NULL);
	gpio_request(GPIO_FN_MMCD0_2, NULL);
	gpio_request(GPIO_FN_MMCD0_3, NULL);
	gpio_request(GPIO_FN_MMCD0_4, NULL);
	gpio_request(GPIO_FN_MMCD0_5, NULL);
	gpio_request(GPIO_FN_MMCD0_6, NULL);
	gpio_request(GPIO_FN_MMCD0_7, NULL);
	gpio_request(GPIO_FN_MMCCMD0, NULL);

	/* SDHI0 */
	gpio_request(GPIO_FN_SDHID0_0, NULL);
	gpio_request(GPIO_FN_SDHID0_1, NULL);
	gpio_request(GPIO_FN_SDHID0_2, NULL);
	gpio_request(GPIO_FN_SDHID0_3, NULL);
	gpio_request(GPIO_FN_SDHICMD0, NULL);
	gpio_request(GPIO_FN_SDHIWP0, NULL);
	gpio_request(GPIO_FN_SDHICLK0, NULL);
	gpio_request(GPIO_PORT327, NULL);
	gpio_direction_input(GPIO_PORT327);
	irq_set_irq_type(gpio_to_irq(GPIO_PORT327), IRQ_TYPE_EDGE_BOTH);
	gpio_set_debounce(GPIO_PORT327, 1000);	/* 1msec */

	/* SDHI1 */
	gpio_request(GPIO_FN_SDHID1_0, NULL);
	gpio_request(GPIO_FN_SDHID1_1, NULL);
	gpio_request(GPIO_FN_SDHID1_2, NULL);
	gpio_request(GPIO_FN_SDHID1_3, NULL);
	gpio_request(GPIO_FN_SDHICMD1, NULL);
	gpio_request(GPIO_FN_SDHICLK1, NULL);

	/* SDHI2 */
	gpio_request(GPIO_FN_SDHID2_0, NULL);
	gpio_request(GPIO_FN_SDHID2_1, NULL);
	gpio_request(GPIO_FN_SDHID2_2, NULL);
	gpio_request(GPIO_FN_SDHID2_3, NULL);
	gpio_request(GPIO_FN_SDHICMD2, NULL);
	gpio_request(GPIO_FN_SDHICLK2, NULL);

	/* I2C */
	gpio_request(GPIO_FN_I2C_SCL0H, NULL);
	gpio_request(GPIO_FN_I2C_SDA0H, NULL);
	gpio_request(GPIO_FN_I2C_SCL1H, NULL);
	gpio_request(GPIO_FN_I2C_SDA1H, NULL);

	/* LCD */
	gpio_request(GPIO_PORT31, NULL);
	gpio_direction_output(GPIO_PORT31, 1); /* unreset */

	/* MIPI-DSI clock setup */
	__raw_writel(0x2a83900D, DSI0PHYCR);
	/* FIXME - start DSIP clock without .set_dot_clock help */
	dsip_clk = clk_get_sys("sh-mipi-dsi.0", "dsip_clk");
	if (!IS_ERR(dsip_clk)) {
		clk_set_rate(dsip_clk, clk_round_rate(dsip_clk, 24000000));
		clk_enable(dsip_clk);
		clk_put(dsip_clk);
	}

	/* PMIC */
	gpio_request(GPIO_PORT0, NULL);	/* MSECURE */
	gpio_direction_output(GPIO_PORT0, 1);
	gpio_request(GPIO_PORT28, NULL);
	gpio_direction_input(GPIO_PORT28);
	irq_set_irq_type(gpio_to_irq(GPIO_PORT28), IRQ_TYPE_LEVEL_LOW);

	/* Ethernet */
#ifdef CONFIG_SMSC911X
	clk_enable(clk_get(NULL, "zb_clk"));
#endif
	gpio_request(GPIO_PORT97, NULL);
	gpio_direction_input(GPIO_PORT97); /* for IRQ */
	gpio_request(GPIO_PORT105, NULL);
	gpio_direction_output(GPIO_PORT105, 1); /* release NRESET */
	regulator_register_fixed(0, dummy_supplies, ARRAY_SIZE(dummy_supplies));

	/* Touch */
	gpio_request(GPIO_PORT30, NULL);
	gpio_direction_output(GPIO_PORT30, 1);
	gpio_request(GPIO_PORT32, NULL);
	gpio_direction_input(GPIO_PORT32);
	gpio_pull_up(GPIO_PORTCR(32));

	/* USBHS */
	gpio_request(GPIO_FN_ULPI_DATA0, NULL);
	gpio_request(GPIO_FN_ULPI_DATA1, NULL);
	gpio_request(GPIO_FN_ULPI_DATA2, NULL);
	gpio_request(GPIO_FN_ULPI_DATA3, NULL);
	gpio_request(GPIO_FN_ULPI_DATA4, NULL);
	gpio_request(GPIO_FN_ULPI_DATA5, NULL);
	gpio_request(GPIO_FN_ULPI_DATA6, NULL);
	gpio_request(GPIO_FN_ULPI_DATA7, NULL);
	gpio_request(GPIO_FN_ULPI_CLK, NULL);
	gpio_request(GPIO_FN_ULPI_STP, NULL);
	gpio_request(GPIO_FN_ULPI_DIR, NULL);
	gpio_request(GPIO_FN_ULPI_NXT, NULL);

	/* TUSB1211 */
	gpio_request(GPIO_PORT131, NULL);
	gpio_direction_output(GPIO_PORT131, 0);
	udelay(100); /* assert RESET_N (minimum pulse width 100 usecs) */
	gpio_direction_output(GPIO_PORT131, 1);

	/* start supplying VIO_CKO3@26MHz to REFCLK */
	gpio_request(GPIO_FN_VIO_CKO3, NULL);
	clk_enable(clk_get(NULL, "vclk3_clk"));

	r8a7373_add_standard_devices();
	platform_add_devices(u2evm_devices, ARRAY_SIZE(u2evm_devices));

	r8a7373_hwlock_gpio = hwspin_lock_request_specific(SMGPIO);
	r8a7373_hwlock_cpg = hwspin_lock_request_specific(SMCPG);
	r8a7373_hwlock_sysc = hwspin_lock_request_specific(SMSYSC);
	pinmux_hwspinlock_init(r8a7373_hwlock_gpio);

	i2c_register_board_info(0, i2c0_devices, ARRAY_SIZE(i2c0_devices));
	i2c_register_board_info(4, i2c4_devices, ARRAY_SIZE(i2c4_devices));

	r8a7373_add_device_to_domain(&r8a7373_a3sp, &usbhs_func_device);
	r8a7373_add_device_to_domain(&r8a7373_a3sp, &mmcif_device);
	r8a7373_add_device_to_domain(&r8a7373_a3sp, &sdhi0_device);
	r8a7373_add_device_to_domain(&r8a7373_a3sp, &sdhi1_device);
	r8a7373_add_device_to_domain(&r8a7373_a3sp, &sdhi2_device);
	r8a7373_add_device_to_domain(&r8a7373_a3sp, &tpu3_device);

	pm_clk_remove(&mmcif_device.dev, NULL);
	pm_clk_remove(&sdhi0_device.dev, NULL);
	pm_clk_remove(&sdhi1_device.dev, NULL);
	pm_clk_remove(&sdhi2_device.dev, NULL);
}

#define SBAR2		IOMEM(0xe6180060)
#define RESCNT2		IOMEM(0xe6188020)

void u2evm_restart(char mode, const char *cmd)
{
	__raw_writel(0, SBAR2);
	__raw_writel(__raw_readl(RESCNT2) | (1 << 31), RESCNT2);
}

static void __init u2evm_reserve(void)
{
	;
}

MACHINE_START(U2EVM, "u2evm")
	.reserve	= u2evm_reserve,
	.map_io		= r8a7373_map_io,
	.init_early	= r8a7373_init_early,
	.nr_irqs	= NR_IRQS_LEGACY,
	.init_irq	= r8a7373_init_irq,
	.handle_irq	= gic_handle_irq,
	.init_machine	= u2evm_init,
	.timer		= &shmobile_timer,
	.restart	= u2evm_restart,
MACHINE_END
