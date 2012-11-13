#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/mmc/host.h>
#include <linux/mmc/renesas_sdhi.h>
#include <linux/pmic/pmic.h>

#include <mach/r8a73734.h>
#include <mach/board-u2evm.h>

#define WLAN_GPIO_EN	GPIO_PORT260
#define WLAN_IRQ	GPIO_PORT98

static void sdhi0_set_pwr(struct platform_device *pdev, int state)
{
#ifdef CONFIG_PMIC_INTERFACE
	if (state) {
		printk(KERN_INFO "\n EOS2_BSP_SDHI : %s\n", __func__);
		pmic_set_power_on(E_POWER_VIO_SD);
		pmic_set_power_on(E_POWER_VMMC);
		__raw_writel(__raw_readl(MSEL3CR) | (1<<28), MSEL3CR);

	} else {
		printk(KERN_INFO "\n EOS2_BSP_SDHI : %s\n", __func__);
		__raw_writel(__raw_readl(MSEL3CR) & ~(1<<28), MSEL3CR);
		pmic_set_power_off(E_POWER_VIO_SD);
		pmic_set_power_off(E_POWER_VMMC);
	}
#endif
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

struct renesas_sdhi_platdata sdhi0_info = {
	.caps		= 0,
	.flags		= RENESAS_SDHI_SDCLK_OFFEN | RENESAS_SDHI_WP_DISABLE,
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

	printk(KERN_INFO "%s Powering %s wifi\n", __func__, (state ? "on" : "off"));

	if (state == power_state)
		return;
	power_state = state;

	if (state) {
		gpio_set_value(WLAN_GPIO_EN, 1);
		mdelay(15);
		gpio_set_value(WLAN_GPIO_EN, 0);
		mdelay(1);
		gpio_set_value(WLAN_GPIO_EN, 1);
		mdelay(70);
	} else {
		gpio_set_value(WLAN_GPIO_EN, 0);
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

static struct renesas_sdhi_dma sdhi1_dma = {
	.chan_tx = {
		.slave_id	= SHDMA_SLAVE_SDHI1_TX,
	},
	.chan_rx = {
		.slave_id	= SHDMA_SLAVE_SDHI1_RX,
	}
};

static struct renesas_sdhi_platdata sdhi1_info = {
	.caps		= MMC_CAP_SDIO_IRQ | MMC_CAP_POWER_OFF_CARD | MMC_CAP_NONREMOVABLE,
	.flags		= RENESAS_SDHI_WP_DISABLE,
	.dma		= &sdhi1_dma,
	.set_pwr		= sdhi1_set_pwr,
	.detect_irq		= 0,
	.detect_msec	= 0,
	.get_cd		= sdhi1_get_cd,
	.ocr			= MMC_VDD_165_195 | MMC_VDD_32_33 | MMC_VDD_33_34,
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
