#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <mach/common.h>
#include <mach/hardware.h>
#include <mach/r8a73734.h>
#include <asm/hardware/cache-l2x0.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/time.h>
#include <linux/smsc911x.h>
#include <linux/mmc/host.h>
#include <linux/mmc/sh_mmcif.h>
#include <linux/mmc/renesas_sdhi.h>
#include <linux/input.h>
#include <linux/input/sh_keysc.h>
#include <linux/gpio_keys.h>
#include <video/sh_mobile_lcdc.h>
#include <video/sh_mipi_dsi.h>
#include <linux/platform_data/leds-renesas-tpu.h>
#include <linux/mfd/tps80031.h>
#include <linux/spi/sh_msiof.h>
#include <linux/i2c/atmel_mxt_ts.h>
#include <linux/regulator/tps80031-regulator.h>
#include <linux/usb/r8a66597.h>

#define SRCR2		0xe61580b0
#define SRCR3		0xe61580b8

#define GPIO_PULL_OFF	0x00
#define GPIO_PULL_DOWN	0x80
#define GPIO_PULL_UP	0xc0

#define GPIO_BASE	0xe6050000
#define GPIO_PORTCR(n)	({				\
	((n) <  96) ? (GPIO_BASE + 0x0000 + (n)) :	\
	((n) < 128) ? (GPIO_BASE + 0x1000 + (n)) :	\
	((n) < 144) ? (GPIO_BASE + 0x1000 + (n)) :	\
	((n) < 192) ? 0 :				\
	((n) < 320) ? (GPIO_BASE + 0x2000 + (n)) :	\
	((n) < 328) ? (GPIO_BASE + 0x3000 + (n)) : 0; })

#define ENT_TPS80031_IRQ_BASE	(IRQPIN_IRQ_BASE + 64)

static void gpio_pull(u32 addr, int type)
{
	u8 data = __raw_readb(addr);

	data &= ~0xc0;
	data |= type;

	__raw_writeb(data, addr);
}

static struct resource smsc9220_resources[] = {
	{
		.start	= 0x10000000,
		.end	= 0x10000000 + SZ_64K - 1,
		.flags	= IORESOURCE_MEM,
	}, {
		.start	= irqpin2irq(41),
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

#ifdef CONFIG_KEYBOARD_SH_KEYSC
/* KEYSC */
static struct sh_keysc_info keysc_platdata = {
	.mode		= SH_KEYSC_MODE_6,
	.scan_timing	= 3,
	.delay		= 100,
	.keycodes	= {
		KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G,
		KEY_H, KEY_I, KEY_J, KEY_K, KEY_L, KEY_M, KEY_N,
		KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U,
		KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z, KEY_HOME, KEY_SLEEP,
		KEY_SPACE, KEY_9, KEY_6, KEY_3, KEY_WAKEUP, KEY_RIGHT, \
		KEY_COFFEE,
		KEY_0, KEY_8, KEY_5, KEY_2, KEY_DOWN, KEY_ENTER, KEY_UP,
		KEY_KPASTERISK, KEY_7, KEY_4, KEY_1, KEY_STOP, KEY_LEFT, \
		KEY_COMPUTER,
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
#endif

/* USBHS */
static int is_vbus_powered(void)
{
	return 1; /* always powered */
}

#define PHYFUNCTR	0xe6890104 /* 16-bit */

static void usbhs_module_reset(void)
{
	__raw_writel(__raw_readl(SRCR2) | (1 << 14), SRCR2); /* USBHS-DMAC */
	__raw_writel(__raw_readl(SRCR3) | (1 << 22), SRCR3); /* USBHS */
	udelay(50); /* wait for at least one EXTALR cycle */
	__raw_writel(__raw_readl(SRCR2) & ~(1 << 14), SRCR2);
	__raw_writel(__raw_readl(SRCR3) & ~(1 << 22), SRCR3);

	/* wait for SuspendM bit being cleared by hardware */
	while (!(__raw_readw(PHYFUNCTR) & (1 << 14))) /* SUSMON */
			;

	__raw_writew(__raw_readw(PHYFUNCTR) | (1 << 13), PHYFUNCTR); /* PRESET */
	while (__raw_readw(PHYFUNCTR) & (1 << 13))
			;
}

static struct r8a66597_platdata usbhs_func_data = {
	.is_vbus_powered = is_vbus_powered,
	.module_start	= usbhs_module_reset,
	.on_chip	= 1,
	.buswait	= 5,
	.max_bufnum	= 0xff,
	.vbus_irq	= ENT_TPS80031_IRQ_BASE + TPS80031_INT_VBUS_DET,
};

static struct resource usbhs_resources[] = {
	[0] = {
		.name	= "USBHS",
		.start	= 0xe6890000,
		.end	= 0xe6890150 - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= gic_spi(87) /* USBULPI */,
		.flags	= IORESOURCE_IRQ,
	},
	[2] = {
		.name	= "USBHS-DMA",
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
static struct sh_mmcif_dma sh_mmcif_dma = {
	.chan_priv_rx	= {
		.slave_id	= SHDMA_SLAVE_MMCIF0_RX,
	},
	.chan_priv_tx	= {
		.slave_id	= SHDMA_SLAVE_MMCIF0_TX,
	},
};

static struct sh_mmcif_plat_data sh_mmcif_plat = {
	.sup_pclk	= 0,
	.ocr		= MMC_VDD_165_195 | MMC_VDD_32_33 | MMC_VDD_33_34,
	.caps		= MMC_CAP_4_BIT_DATA | MMC_CAP_8_BIT_DATA |
			  MMC_CAP_NONREMOVABLE,
	.dma		= &sh_mmcif_dma,
};

static struct resource sh_mmcif_resources[] = {
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

static struct platform_device sh_mmcif_device = {
	.name		= "sh_mmcif",
	.id		= 0,
	.dev		= {
		.platform_data	= &sh_mmcif_plat,
	},
	.num_resources	= ARRAY_SIZE(sh_mmcif_resources),
	.resource	= sh_mmcif_resources,
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

static struct renesas_sdhi_dma sdhi0_dma = {
	.chan_tx = {
		.slave_id	= SHDMA_SLAVE_SDHI0_TX,
	},
	.chan_rx = {
		.slave_id	= SHDMA_SLAVE_SDHI0_RX,
	}
};

static struct renesas_sdhi_platdata sdhi0_info = {
	.caps		= 0,
	.flags		= RENESAS_SDHI_SDCLK_OFFEN,
	.dma		= &sdhi0_dma,
	.set_pwr	= sdhi0_set_pwr,
	.detect_irq	= irqpin2irq(50),
	.detect_msec	= 0,
	.get_cd		= sdhi0_get_cd,
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

#define GPIO_KEY(c, g, d) \
	{.code=c, .gpio=g, .desc=d, .wakeup=1, .active_low=1}

static struct gpio_keys_button gpio_buttons[] = {
	GPIO_KEY(KEY_POWER, GPIO_PORT24, "Power"),
	GPIO_KEY(KEY_MENU, GPIO_PORT25, "Menu"),
	GPIO_KEY(KEY_HOME, GPIO_PORT26, "Home"),
	GPIO_KEY(KEY_BACK, GPIO_PORT27, "Back"),
	GPIO_KEY(KEY_VOLUMEUP, GPIO_PORT1, "+"),
	GPIO_KEY(KEY_VOLUMEDOWN, GPIO_PORT2, "-"),
};

static int gpio_key_enable(struct device *dev)
{
	gpio_pull(GPIO_PORTCR(24), GPIO_PULL_UP);
	gpio_pull(GPIO_PORTCR(25), GPIO_PULL_UP);
	gpio_pull(GPIO_PORTCR(26), GPIO_PULL_UP);
	gpio_pull(GPIO_PORTCR(27), GPIO_PULL_UP);
	gpio_pull(GPIO_PORTCR(1), GPIO_PULL_UP);
	gpio_pull(GPIO_PORTCR(2), GPIO_PULL_UP);
	return 0;
}

static struct gpio_keys_platform_data gpio_key_info = {
	.buttons	= gpio_buttons,
	.nbuttons	= ARRAY_SIZE(gpio_buttons),
	.rep		= 0,
	.enable		= gpio_key_enable,
};

static struct platform_device gpio_key_device = {
	.name	= "gpio-keys",
	.id	= -1,
	.dev	= {
		.platform_data	= &gpio_key_info,
	},
};

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

static struct sh_mobile_lcdc_info lcdc_info = {
	.clock_source	= LCDC_CLK_PERIPHERAL,

	/* LCDC0 */
	.ch[0] = {
		.chan = LCDC_CHAN_MAINLCD,
		.bpp = 32,
		.interface_type		= RGB24,
		.clock_divider		= 1,
		.flags			= LCDC_FLAGS_DWPOL,
		.lcd_cfg = lcdc0_modes,
		.num_cfg = ARRAY_SIZE(lcdc0_modes),
		.lcd_size_cfg = {
			.width	= 44,
			.height	= 79,
		},
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
		.platform_data  = &lcdc_info,
		.coherent_dma_mask = DMA_BIT_MASK(32),
	},
};

static struct resource mipidsi0_resources[] = {
	[0] = {
		.start  = 0xfeab0000,
		.end    = 0xfeab3fff,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = 0xfeab4000,
		.end    = 0xfeab7fff,
		.flags  = IORESOURCE_MEM,
	},
};

static struct sh_mipi_dsi_info mipidsi0_info = {
	.data_format	= MIPI_RGB888,
	.lcd_chan	= &lcdc_info.ch[0],
	.vsynw_offset	= 20,
	.clksrc		= 1,
	.flags		= SH_MIPI_DSI_HSABM,
};

static struct platform_device mipidsi0_device = {
	.name           = "sh-mipi-dsi",
	.num_resources  = ARRAY_SIZE(mipidsi0_resources),
	.resource       = mipidsi0_resources,
	.id             = 0,
	.dev	= {
		.platform_data	= &mipidsi0_info,
	},
};

static struct led_renesas_tpu_config tpu3_info = {
	.name		= "lcd-backlight",
	.pin_gpio_fn	= GPIO_FN_TPUTO3,
	.pin_gpio	= GPIO_PORT39,
	.channel_offset	= 0x00d0,
	.timer_bit	= 3,
	.max_brightness = LED_FULL,
	.init_brightness = LED_FULL,
	.refresh_rate	= 2000,
};

static struct resource tpu3_resources[] = {
	[0] = {
		.start  = 0xe66000d0,
		.end    = 0xe66000ff,
		.flags  = IORESOURCE_MEM,
	},
};

static struct platform_device tpu3_device = {
	.name		= "leds-renesas-tpu",
	.num_resources  = ARRAY_SIZE(tpu3_resources),
	.resource       = tpu3_resources,
	.id		= 3,
	.dev		= {
		.platform_data	= &tpu3_info,
	},
};

#ifdef CONFIG_SPI_SH_MSIOF
/* SPI */
static struct sh_msiof_spi_info sh_msiof0_info = {
        .rx_fifo_override       = 256,
        .num_chipselect         = 1,
};

static struct resource sh_msiof0_resources[] = {
        [0] = {
                .start  = 0xe6e20000,
                .end    = 0xe6e20064 - 1,
                .flags  = IORESOURCE_MEM,
        },
        [1] = {
                .start  = gic_spi(109),
                .flags  = IORESOURCE_IRQ,
        },
};

static struct platform_device sh_msiof0_device = {
        .name           = "spi_sh_msiof",
        .id             = 0,
        .dev            = {
                .platform_data  = &sh_msiof0_info,
        },
        .num_resources  = ARRAY_SIZE(sh_msiof0_resources),
        .resource       = sh_msiof0_resources,
};
#endif

static struct platform_device *u2evm_devices[] __initdata = {
	&usbhs_func_device,
	&eth_device,
#ifdef CONFIG_KEYBOARD_SH_KEYSC
	&keysc_device,
#endif
	&sh_mmcif_device,
	&sdhi0_device,
	&gpio_key_device,
	&lcdc_device,
	&mipidsi0_device,
	&tpu3_device,
#ifdef CONFIG_SPI_SH_MSIOF
	&sh_msiof0_device,
#endif
};

/* I2C */

static struct regulator_consumer_supply tps80031_ldo5_supply[] = {
	REGULATOR_SUPPLY("vdd_touch", NULL),
};

#define TPS_PDATA_INIT(_id, _minmv, _maxmv, _supply_reg, _always_on,	\
	_boot_on, _apply_uv, _init_uV, _init_enable, _init_apply, 	\
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
		.init_uV =  _init_uV * 1000,				\
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
		.id	 = TPS80031_ID_##_id,		\
		.name   = "tps80031-regulator",		\
		.platform_data  = &pdata_##_data,	\
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
		.irq		= irqpin2irq(28),
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
		.irq	= irqpin2irq(32),
	},
};

static struct map_desc u2evm_io_desc[] __initdata = {
	{
		.virtual	= 0xe6000000,
		.pfn		= __phys_to_pfn(0xe6000000),
		.length		= 256 << 20,
		.type		= MT_DEVICE_NONSHARED
	},
};

static void __init u2evm_map_io(void)
{
	iotable_init(u2evm_io_desc, ARRAY_SIZE(u2evm_io_desc));
	r8a73734_add_early_devices();
	shmobile_setup_console();
}

void __init u2evm_init_irq(void)
{
	r8a73734_init_irq();
}

#define IRQC0_CONFIG_00		0xe61c0180
#define IRQC1_CONFIG_00		0xe61c0380
static void irqc_set_chattering(int pin, int timing)
{
	u32 val;
	u32 *reg;

	reg = (pin >= 32) ? (u32 *)IRQC1_CONFIG_00 : (u32 *)IRQC0_CONFIG_00;
	reg += (pin & 0x1f);

	val = __raw_readl(reg) & ~0x80ff0000;
	__raw_writel(val | (timing << 16) | (1 << 31), reg);
}

#define DSI0PHYCR	0xe615006c
static void __init u2evm_init(void)
{
	r8a73734_pinmux_init();

	/* SCIFA0 */
	gpio_request(GPIO_FN_SCIFA0_TXD, NULL);
	gpio_request(GPIO_FN_SCIFA0_RXD, NULL);

	/* SCIFB0 */
	gpio_request(GPIO_FN_SCIFB0_TXD, NULL);
	gpio_request(GPIO_FN_SCIFB0_RXD, NULL);
	gpio_request(GPIO_FN_SCIFB0_CTS_, NULL);
	gpio_request(GPIO_FN_SCIFB0_RTS_, NULL);

#ifdef CONFIG_KEYBOARD_SH_KEYSC
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
#endif

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
	irq_set_irq_type(irqpin2irq(50), IRQ_TYPE_EDGE_BOTH);
	irqc_set_chattering(50, 0x01);	/* 1msec */

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

	/* PMIC */
	gpio_request(GPIO_PORT0, NULL);	/* MSECURE */
	gpio_direction_output(GPIO_PORT0, 1);
	gpio_request(GPIO_PORT28, NULL);
	gpio_direction_input(GPIO_PORT28);
	irq_set_irq_type(irqpin2irq(28), IRQ_TYPE_LEVEL_LOW);

	/* Ethernet */
	gpio_request(GPIO_PORT97, NULL);
	gpio_direction_input(GPIO_PORT97); /* for IRQ */
	gpio_request(GPIO_PORT105, NULL);
	gpio_direction_output(GPIO_PORT105, 1); /* release NRESET */

	/* Touch */
	gpio_request(GPIO_PORT30, NULL);
	gpio_direction_output(GPIO_PORT30, 1);
	gpio_request(GPIO_PORT32, NULL);
	gpio_direction_input(GPIO_PORT32);
	gpio_pull(GPIO_PORTCR(32), GPIO_PULL_UP);

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

#ifdef CONFIG_SPI_SH_MSIOF
	/* enable MSIOF0 */
	gpio_request(GPIO_FN_MSIOF0_TXD, NULL);
	gpio_request(GPIO_FN_MSIOF0_TSYNC, NULL);
	gpio_request(GPIO_FN_MSIOF0_SCK, NULL);
	gpio_request(GPIO_FN_MSIOF0_RXD, NULL);
#endif

#ifdef CONFIG_CACHE_L2X0
	/*
	 * [30] Early BRESP enable
	 * [27] Non-secure interrupt access control
	 * [26] Non-secure lockdown enable
	 * [22] Shared attribute override enable
	 * [19:17] Way-size: b010 = 32KB
	 * [16] Accosiativity: 0 = 8-way
	 */
	l2x0_init(__io(0xf0100000), 0x4c440000, 0x820f0fff);
#endif
	r8a73734_add_standard_devices();
	platform_add_devices(u2evm_devices, ARRAY_SIZE(u2evm_devices));

	i2c_register_board_info(0, i2c0_devices, ARRAY_SIZE(i2c0_devices));
	i2c_register_board_info(4, i2c4_devices, ARRAY_SIZE(i2c4_devices));
}

static void __init u2evm_timer_init(void)
{
	r8a73734_clock_init();
	shmobile_timer.init();
}

struct sys_timer u2evm_timer = {
	.init	= u2evm_timer_init,
};

MACHINE_START(U2EVM, "u2evm")
	.map_io		= u2evm_map_io,
	.init_irq	= u2evm_init_irq,
	.handle_irq	= shmobile_handle_irq_gic,
	.init_machine	= u2evm_init,
	.timer		= &u2evm_timer,
MACHINE_END
