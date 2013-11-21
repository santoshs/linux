/*
 * Copyright (C) 2013 Renesas Mobile Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */

#ifndef __ASM_ARCH_BOARD_CONFIG_H
#define __ASM_ARCH_BOARD_CONFIG_H

#include <linux/platform_device.h>
#include <linux/mmc/renesas_mmcif.h>
#include <mach/setup-u2sdhi.h>
#include <mach/setup-u2stm.h>
#include <mach/crashlog.h>
#include <sound/sh_fsi.h>
#include <linux/platform_data/fsi_d2153_pdata.h>
#include <linux/spi/sh_msiof.h>
#include <linux/thermal_sensor/ths_kernel.h>
#include <mach/setup-u2csi2.h>
#include <mach/setup-u2gpio_key.h>
#include <linux/tpu_pwm_board.h>

#include <mach/r8a7373.h>
#include <mach/gpio.h>

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

/* For different STM muxing options 0, 1, or None, as given by
 * boot_command_line parameter stm=0/1/n
 */

static struct sh_mmcif_plat_data renesas_mmcif_plat = {
	.sup_pclk	= 0,
	.ocr		= MMC_VDD_165_195 | MMC_VDD_32_33 | MMC_VDD_33_34,
	.caps		= MMC_CAP_4_BIT_DATA | MMC_CAP_8_BIT_DATA |
		MMC_CAP_1_8V_DDR | MMC_CAP_UHS_DDR50 | MMC_CAP_NONREMOVABLE,
	.set_pwr	= mmcif_set_pwr,
	.down_pwr	= mmcif_down_pwr,
	.slave_id_tx	= SHDMA_SLAVE_MMCIF0_TX,
	.slave_id_rx	= SHDMA_SLAVE_MMCIF0_RX,
	.max_clk	= 52000000,
};

static struct platform_device mmcif_device = {
	.name		= "renesas_mmcif",
	.id		= 0,
	.dev		= {
		.platform_data	= &renesas_mmcif_plat,
	},
	.resource		= renesas_mmcif_resources,
	.num_resources	= ARRAY_SIZE(renesas_mmcif_resources),
};

static struct resource fsi_resources[] = {
	[0] = {
		.name	= "FSI",
		.start	= 0xec230000,
		.end	= 0xec230500 - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start  = gic_spi(146),
		.flags  = IORESOURCE_IRQ,
	},
};

static struct sh_fsi_platform_info fsi_info = {
	.port_flags =	SH_FSI_OUT_SLAVE_MODE | SH_FSI_IN_SLAVE_MODE |
			SH_FSI_BRS_INV | SH_FSI_OFMT(I2S) | SH_FSI_IFMT(I2S),
};

static struct platform_device fsi_device = {
	.name		= "sh_fsi2",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(fsi_resources),
	.resource	= fsi_resources,
	.dev	= {
		.platform_data	= &fsi_info,
	},
};

static struct resource fsi_b_resources[] = {
	[0] = {
		.name	= "FSI",
		.start	= 0xec230000,
		.end	= 0xec230500 - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= gic_spi(146),
		.flags	= IORESOURCE_IRQ,
	},
};

static struct sh_fsi_platform_info fsi_b_info = {
	.port_flags = SH_FSI_BRM_INV | SH_FSI_LRM_INV | SH_FSI_OFMT(I2S) |
		SH_FSI_IFMT(I2S),
	.always_slave	= 1,
};

static struct platform_device fsi_b_device = {
	.name			= "sh_fsi2",
	.id				= 1,
	.num_resources	= ARRAY_SIZE(fsi_b_resources),
	.resource	= fsi_b_resources,
	.dev	= {
		.platform_data	= &fsi_b_info,
	},
};

static struct fsi_d2153_platform_data audio_pdata = {
	.hp_spk_path_en		= false,
	.private_data		= NULL,
};

static struct platform_device audio_device = {
	.name	= "fsi-snd-d2153",
	.id	= 0,
	.dev	= {
		.platform_data  = &audio_pdata,
	},
};

static struct platform_device sh_fsi_wireless_transciever_device = {
		.name = "sh_fsi_wireless_transciever",
		.id = 0,
};

static struct resource lcdc_resources[] = {
	[0] = {
		.name	= "LCDC",
		.start	= 0xe61c0000,
		.end	= 0xe61c2fff,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= gic_spi(64),
		.flags	= IORESOURCE_IRQ,
	},
	[2] = {
		.name	= "panel_irq_port",
		.start	= GPIO_PORT27,
		.end	= GPIO_PORT27,
		.flags	= IORESOURCE_MEM,
	},
	[3] = {
		.name	= "panel_esd_irq_port",
		.start	= GPIO_PORT6,
		.end	= GPIO_PORT6,
		.flags	= IORESOURCE_MEM,
	},
};

static struct sh_mobile_lcdc_info lcdc_info = {
	.clock_source	= LCDC_CLK_PERIPHERAL,
	/* LCDC0 */
	.ch[0] = {
		.chan = LCDC_CHAN_MAINLCD,
#ifdef CONFIG_FB_SH_MOBILE_RGB888
		.bpp = 24,
#else
		.bpp = 32,
#endif
		.panelreset_gpio = GPIO_PORT31,
		.paneldsi_irq = 33,
	},
};

static struct platform_device lcdc_device = {
	.name		= "sh_mobile_lcdc_fb",
	.num_resources	= ARRAY_SIZE(lcdc_resources),
	.resource	= lcdc_resources,
	.dev	= {
		.platform_data		= &lcdc_info,
		.coherent_dma_mask	= DMA_BIT_MASK(32),
	},
};

static struct resource mfis_resources[] = {
	[0] = {
		.name   = "MFIS",
		.start  = gic_spi(126),
		.flags  = IORESOURCE_IRQ,
	},
};

static struct platform_device mfis_device = {
	.name           = "mfis",
	.id             = 0,
	.resource       = mfis_resources,
	.num_resources  = ARRAY_SIZE(mfis_resources),
};

static struct resource mdm_reset_resources[] = {
	[0] = {
		.name	= "MODEM_RESET",
		.start	= 0xE6190000,
		.end	= 0xE61900FF,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= gic_spi(219), /* EPMU_int1 */
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device mdm_reset_device = {
	.name		= "rmc_wgm_reset_int",
	.id			= 0,
	.resource	= mdm_reset_resources,
	.num_resources	= ARRAY_SIZE(mdm_reset_resources),
};

#ifdef CONFIG_SPI_SH_MSIOF
/* SPI */
static struct sh_msiof_spi_info sh_msiof0_info = {
	.rx_fifo_override = 256,
	.num_chipselect = 1,
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
	.name = "spi_sh_msiof",
	.id   = 0,
	.dev  = {
		.platform_data  = &sh_msiof0_info,
	},
	.num_resources  = ARRAY_SIZE(sh_msiof0_resources),
	.resource       = sh_msiof0_resources,
};
#endif

/*  Add for Thermal Sensor driver*/
static struct thermal_sensor_data ths_platdata[] = {
	/* THS0 */
	{
		/* Normal 1 operation */
		.current_mode	= E_NORMAL_1,
		/* Normal 1 operation */
		.last_mode		= E_NORMAL_1,
	},

	/* THS1 */
	{
		/* Normal 1 operation */
		.current_mode	= E_NORMAL_1,
		/* Normal 1 operation */
		.last_mode		= E_NORMAL_1,
	},
};

static struct resource ths_resources[] = {
	[0] = {
		.name	= "THS",
		.start	= 0xe61F0000,
		.end	= 0xe61F0238 - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= gic_spi(73), /* SPI# of THS is 73 */
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device thermal_sensor_device = {
	.name			= "thermal_sensor",
	.id				= 0,
	.num_resources	= ARRAY_SIZE(ths_resources),
	.resource	= ths_resources,
	.dev		= {
		.platform_data	= &ths_platdata,
	},
};
/* End Add for Thermal Sensor driver */

static struct resource	tpu_resources[] = {
	[TPU_MODULE_0] = {
		.name	= "tpu0_map",
		.start	= 0xe6600000,
		.end	= 0xe6600200,
		.flags	= IORESOURCE_MEM,
	},
};

/* GPIO Settings */
static struct portn_gpio_setting_info_tpu tpu0_gpio_setting_info[] = {
	[0] = { /* TPU CHANNEL */
		.flag = 0,
		.port = GPIO_PORT36,
		/* GPIO settings to be retained at resume state */
		.active = {
			/* GPIO_FN_TPUTO0 ,*//*Func 3*/
			.port_fn	= GPIO_FN_PORT36_TPU0TO0,
			.pull		= PORTn_CR_PULL_DOWN,
			.direction	= PORTn_CR_DIRECTION_NOT_SET,
			.output_level	= PORTn_OUTPUT_LEVEL_NOT_SET,
		},
		/* GPIO settings to be set at suspend state */
		.inactive = {
			.port_fn	= GPIO_PORT36, /*Func 0*/
			.pull		= PORTn_CR_PULL_DOWN,
			.direction	= PORTn_CR_DIRECTION_INPUT,
		}
	},
};

static struct port_info
tpu_pwm_pfc[TPU_MODULE_MAX][TPU_CHANNEL_MAX] = {
	[TPU_MODULE_0] = {
		[TPU_CHANNEL_0]	= {
			/* GPIO_FN_TPUTO0,*/
			.port_func	=  GPIO_PORT36,
			.func_name	= "pwm-tpu0to0",
			.port_count	= ARRAY_SIZE(tpu0_gpio_setting_info),
			.tpu_gpio_setting_info	= tpu0_gpio_setting_info,
		},
		[TPU_CHANNEL_1]	= {
			.port_func	=  GPIO_FN_TPU0TO1,
			.func_name	= "pwm-tpu0to1",
			.port_count	= 0,
			.tpu_gpio_setting_info	= NULL,
		},
		[TPU_CHANNEL_2]	= {
			.port_func	=  GPIO_FN_TPU0TO2,
			.func_name	= "pwm-tpu0to2",
			.port_count	= 0,
			.tpu_gpio_setting_info	= NULL,
		},
		[TPU_CHANNEL_3]	= {
			.port_func	=  GPIO_FN_TPU0TO3,
			.func_name	= "pwm-tpu0to3",
			.port_count	= 0,
			.tpu_gpio_setting_info	= NULL,
		},
	},
};

static struct platform_device	tpu_devices[] = {
	{
		.name		= "tpu-renesas-sh_mobile",
		.id		= TPU_MODULE_0,
		.num_resources	= 1,
		.resource	= &tpu_resources[TPU_MODULE_0],
		.dev	= {
			.platform_data = &tpu_pwm_pfc[TPU_MODULE_0],
		},
	},
};


static struct platform_device *u2_devices[] __initdata = {
	&mmcif_device,
#if defined(CONFIG_U2_SDHI)
	&sdhi1_device,
#endif
	&fsi_device,
	&fsi_b_device,
	&audio_device,
	&sh_fsi_wireless_transciever_device,
	&gpio_key_device,
	&lcdc_device,
	&mfis_device,
	&tpu_devices[TPU_MODULE_0],
	&mdm_reset_device,
#ifdef CONFIG_SPI_SH_MSIOF
	&sh_msiof0_device,
#endif
	&thermal_sensor_device,
};

#endif
