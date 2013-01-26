#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/mmc/host.h>
#include <linux/mmc/renesas_sdhi.h>
#include <linux/pmic/pmic.h>
#include <linux/regulator/consumer.h>

#include <mach/common.h>
#include <mach/r8a7373.h>
#include <mach/irqs.h>
#include <mach/board-u2evm.h>

#define WLAN_GPIO_EN	GPIO_PORT260
#define WLAN_IRQ	GPIO_PORT98
#define VSD_VDCORE_DELAY 50

static void sdhi0_set_pwr(struct platform_device *pdev, int state)
{
	if (u2_get_board_rev() >= 5) {
		struct regulator *regulator;		

		if(state) {
			printk("\n EOS2_BSP_SDHI : %s\n",__func__);

			regulator = regulator_get(NULL, "vio_sd");
			if (IS_ERR(regulator))
				return -1;

			regulator_enable(regulator);

			regulator_put(regulator);

			regulator = regulator_get(NULL, "vsd");
			if (IS_ERR(regulator))
				return ;

			regulator_enable(regulator);

			regulator_put(regulator);

			__raw_writel(__raw_readl(MSEL3CR) | (1<<28), MSEL3CR);

		} else {
			printk("\n EOS2_BSP_SDHI : %s\n",__func__);
			__raw_writel(__raw_readl(MSEL3CR) & ~(1<<28), MSEL3CR);		

			regulator = regulator_get(NULL, "vio_sd");
			if (IS_ERR(regulator))
				return -1;

			regulator_disable(regulator);

			regulator_put(regulator);

			regulator = regulator_get(NULL, "vsd");
			if (IS_ERR(regulator))
				return ;

			regulator_disable(regulator);

			regulator_put(regulator);
		}
	} else {
#ifdef CONFIG_PMIC_INTERFACE
		if (state) {
			pmic_set_power_on(E_POWER_VIO_SD);
			pmic_set_power_on(E_POWER_VMMC);
			__raw_writel(__raw_readl(MSEL3CR) | (1<<28), MSEL3CR);

		} else {
			__raw_writel(__raw_readl(MSEL3CR) & ~(1<<28), MSEL3CR);
			pmic_set_power_off(E_POWER_VIO_SD);
			mdelay(VSD_VDCORE_DELAY);
			pmic_set_power_off(E_POWER_VMMC);
		}
#endif  /* CONFIG_PMIC_INTERFACE */
	}
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

static struct renesas_sdhi_gpio_setting_info sdhi0_gpio_setting_info[] = {
	[0] = {
		.flag = 1,
		.port = GPIO_PORT327,
		.active = {
			.port_mux 	= GPIO_PORT327,
			.pull 		= RENESAS_SDHI_PULL_OFF,
			.direction	= RENESAS_SDHI_DIRECTION_INPUT,
			.out_level	= RENESAS_SDHI_OUT_LEVEL_NOT_SET,
		},
		.deactive = {
			.port_mux 	= GPIO_FN_SDHICD0,
			.pull 		= RENESAS_SDHI_PULL_UP,
			.direction	= RENESAS_SDHI_DIRECTION_NOT_SET,
			.out_level	= RENESAS_SDHI_OUT_LEVEL_NOT_SET,
		}
	},
};

struct renesas_sdhi_platdata sdhi0_info = {
	.caps			= 0,
	.flags			= RENESAS_SDHI_SDCLK_OFFEN |
					RENESAS_SDHI_WP_DISABLE |
					RENESAS_SDHI_DMA_SLAVE_CONFIG,
	.slave_id_tx		= SHDMA_SLAVE_SDHI0_TX,
	.slave_id_rx		= SHDMA_SLAVE_SDHI0_RX,
	.set_pwr		= sdhi0_set_pwr,
	.detect_irq		= R8A7373_IRQC_IRQ(50),
	.detect_msec		= 0,
	.get_cd			= sdhi0_get_cd,
	.set_dma		= sdhi0_set_dma,
	.port_cnt		= ARRAY_SIZE(sdhi0_gpio_setting_info),
	.gpio_setting_info	= &sdhi0_gpio_setting_info,
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

struct platform_device sdhi0_device = {
	.name		= "renesas_sdhi",
	.id		= 0,
	.dev		= {
		.platform_data	= &sdhi0_info,
	},
	.num_resources	= ARRAY_SIZE(sdhi0_resources),
	.resource	= sdhi0_resources,
};

static void sdhi1_set_pwr(struct platform_device *pdev, int state)
{
	static int power_state;

	printk(KERN_ALERT "%s: %s\n", __func__, (state ? "on" : "off"));

	if (state != power_state) {
		power_state = state;
		gpio_set_value(GPIO_PORT260, state);
	}
}

static int sdhi1_get_cd(struct platform_device *pdev)
{
	/*
	In SSG , SDHI1 channel is using wlan .
	For wlan case its non removable card
	thats we dont have external interrupt.
	For that reason we returning  1. */
	return 1;/*return gpio_get_value(GPIO_PORT327) ? 0 : 1;*/
}

#define SDHI1_VOLTAGE (MMC_VDD_165_195 | MMC_VDD_20_21 | MMC_VDD_21_22 \
			| MMC_VDD_22_23 | MMC_VDD_23_24 | MMC_VDD_24_25 \
			| MMC_VDD_25_26 | MMC_VDD_26_27 | MMC_VDD_27_28 \
			| MMC_VDD_28_29 | MMC_VDD_29_30 | MMC_VDD_30_31 \
			| MMC_VDD_31_32 | MMC_VDD_32_33 | MMC_VDD_33_34 \
			| MMC_VDD_34_35 | MMC_VDD_35_36)

static struct renesas_sdhi_platdata sdhi1_info = {
	.caps		= MMC_CAP_SDIO_IRQ | MMC_CAP_NONREMOVABLE | MMC_CAP_4_BIT_DATA |
			  MMC_CAP_POWER_OFF_CARD | MMC_CAP_DISABLE,
	.pm_caps	= MMC_PM_KEEP_POWER | MMC_PM_IGNORE_PM_NOTIFY,
	.flags		= RENESAS_SDHI_SDCLK_OFFEN,
	.slave_id_tx	= SHDMA_SLAVE_SDHI1_TX,
	.slave_id_rx	= SHDMA_SLAVE_SDHI1_RX,
	.set_pwr	= sdhi1_set_pwr,
	.detect_irq	= 0,
	.detect_msec	= 0,
	.get_cd		= sdhi1_get_cd,
	.ocr		= MMC_VDD_165_195 | MMC_VDD_32_33 | MMC_VDD_33_34,
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

struct platform_device sdhi1_device = {
	.name		= "renesas_sdhi",
	.id		= 1,
	.dev		= {
		.platform_data	= &sdhi1_info,
	},
	.num_resources	= ARRAY_SIZE(sdhi1_resources),
	.resource	= sdhi1_resources,
};

#if 0
static void sdhi2_set_pwr(struct platform_device *pdev, int state)
{
	;/* TODO*/
}

static int sdhi2_get_cd(struct platform_device *pdev)
{
	return 1;
}

static struct renesas_sdhi_dma sdhi2_dma = {
	.chan_tx = {
		.slave_id	= SHDMA_SLAVE_SDHI2_TX,
	},
	.chan_rx = {
		.slave_id	= SHDMA_SLAVE_SDHI2_RX,
	}
};

static struct renesas_sdhi_platdata sdhi2_info = {
	.caps		= MMC_CAP_SDIO_IRQ | MMC_CAP_POWER_OFF_CARD | MMC_CAP_NONREMOVABLE | MMC_PM_KEEP_POWER,
	.flags		= RENESAS_SDHI_SDCLK_OFFEN | RENESAS_SDHI_WP_DISABLE,
	.dma		= &sdhi2_dma,
	.set_pwr	= sdhi2_set_pwr,
	.detect_irq	= 0,
	.detect_msec	= 0,
	.get_cd		= sdhi2_get_cd,
	.ocr		= MMC_VDD_165_195, /*SDHI2_VOLTAGE,In SSG we are using only SDHI0 and SDHI1 channel.As there is no external interface on Kota/SSG w.r.t SDHI2.
									The present value is the default setting made this will alter according to the external interface mapped for the SDHI Bus.*/
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
#endif
