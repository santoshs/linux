/*
 * arch/arm/mach-shmobile/board-gardalte.c
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
/*******************************************************************************
	Board file to support GARDA Products
*******************************************************************************/
#include <asm/hardware/gic.h>
#include <linux/clk.h>
#include <linux/mmcoops.h>
#include <linux/dma-mapping.h>
#include <mach/irqs.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/hwspinlock.h>
#include <mach/common.h>
#include <mach/hardware.h>
#include <mach/r8a7373.h>
#include <mach/setup-u2usb.h>
#include <asm/hardware/cache-l2x0.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/time.h>
#include <linux/mmc/host.h>
#include <linux/mmc/renesas_mmcif.h>
#include <video/sh_mobile_lcdc.h>
#include <linux/platform_data/leds-renesas-tpu.h>
#include <mach/board-gardalte.h>
#include <mach/poweroff.h>
#ifdef CONFIG_MFD_D2153
#include <linux/d2153/core.h>
#include <linux/d2153/pmic.h>
#include <linux/d2153/d2153_battery.h>
#endif
#include <linux/spi/sh_msiof.h>
#include <linux/i2c/atmel_mxt_ts.h>
#include <linux/usb/r8a66597.h>
#include <linux/memblock.h>
#include <sound/sh_fsi.h>
#include <linux/platform_data/fsi_d2153_pdata.h>
#include <linux/tpu_pwm.h>
#include <linux/tpu_pwm_board.h>
#include <linux/pcm2pwm.h>
#include <linux/vibrator.h>
#include <linux/thermal_sensor/ths_kernel.h>
#include <linux/sh_clk.h>
#include <media/v4l2-subdev.h>
#include "board-renesas_wifi.h"
#include <linux/pmic/pmic-ncp6914.h>
#include <linux/ktd259b_bl.h>
#include <mach/setup-u2rcu.h>
#include <mach/setup-u2csi2.h>
#include <mach/setup-u2camera.h>
#include <mach/setup-u2ion.h>
#include <linux/sysfs.h>
#include <linux/proc_fs.h>
#include <linux/mmcoops.h>	
#include <asm/io.h>
#if defined(CONFIG_RENESAS_BT)
#include <mach/board-gardalte-renesas-bt.h>
#endif
#if defined(CONFIG_RENESAS_GPS)
#include <mach/board-gardalte-renesas-gps.h>
#endif
#if defined(CONFIG_RENESAS_NFC)
#ifdef CONFIG_PN544_NFC
#include <mach/board-gardalte-renesas-nfc.h>
#endif
#endif
#if defined(CONFIG_SAMSUNG_MHL)
#include <mach/board-gardalte-mhl.h>
#include <mach/board_edid.h>
#endif
#ifdef CONFIG_USB_OTG
#include <linux/usb/tusb1211.h>
#endif
#ifdef ARCH_HAS_READ_CURRENT_TIMER
#include <mach/setup-u2current_timer.h>
#endif
#include <mach/setup-u2sdhi.h>
#include <mach/setup-u2gpio_key.h>
#include <mach/setup-u2touchkey.h>
#include <mach/setup-u2mxt224.h>
#include <linux/pmic/pmic-ncp6914.h>
#include <linux/i2c-gpio.h>
#ifdef CONFIG_USB_OTG
#include <linux/usb/tusb1211.h>
#endif
#include <mach/board.h>
#include <mach/crashlog.h>
#if defined(CONFIG_GPS_CSR_GSD5T)
#include <mach/board-gardalte-gps.h>
#endif
#include <linux/mmcoops.h>
#if defined(CONFIG_SEC_DEBUG_INFORM_IOTABLE)
#include <mach/sec_debug.h>
#include <mach/sec_debug_inform.h>
#endif
#include <sound/a2220.h>
#include <linux/leds-ktd253ehd.h>
#ifdef CONFIG_BOARD_VERSION_GARDA
#include <linux/leds-regulator.h>
#endif /* CONFIG_BOARD_VERSION_GARDA */
#if (defined(CONFIG_BCM_RFKILL) || defined(CONFIG_BCM_RFKILL_MODULE))
#include <linux/broadcom/bcmbt_rfkill.h>
#endif
#ifdef CONFIG_BCM_BT_LPM
#include <linux/broadcom/bcmbt_lpm.h>
#endif
#ifdef CONFIG_BCM_BZHW
#include <linux/broadcom/bcm_bzhw.h>
#endif
#if defined(CONFIG_BCMI2CNFC)
#include <linux/bcmi2cnfc.h>
#endif
#ifdef CONFIG_NFC_PN547
#include <linux/nfc/pn547.h>
#endif /* CONFIG_NFC_PN547	*/
#if defined(CONFIG_SAMSUNG_SENSOR)
#include <mach/board-gardalte-sensor.h>
#endif
#if defined(CONFIG_GPS_CSR_GSD5T)
#include <mach/board-gardalte-gps.h>
#endif
#if defined(CONFIG_BCMI2CNFC) || defined(CONFIG_NFC_PN547)
#include <mach/board-gardalte-nfc.h>
#endif
#define ENT_TPS80031_IRQ_BASE	(IRQPIN_IRQ_BASE + 64)


#define STBCHRB3	0xE6180043
#define PHYFUNCTR	IO_ADDRESS(0xe6890104) /* 16-bit */

/* SBSC register address */
#define SBSC_BASE			(0xFE000000U)
#define SBSC_SDMRA_DONE			(0x00000000)
#define SBSC_SDMRACR1A_ZQ		(0x0000560A)
#define CPG_PLL3CR_1040MHZ		(0x27000000)
#define CPG_PLLECR_PLL3ST		(0x00000800)
#define CPG_BASE                (0xE6150000U)
#define CPG_PLL3CR			IO_ADDRESS(CPG_BASE + 0x00DC)
#define CPG_PLLECR			IO_ADDRESS(CPG_BASE + 0x00D0)
#define DBGREG1 IO_ADDRESS(0xE6100020)
#define DBGREG9         IO_ADDRESS(0xE6100040)
#define SYS_TRACE_FUNNEL_STM_BASE       IO_ADDRESS(0xE6F8B000)
#define SYS_TPIU_STM_BASE	IO_ADDRESS(0xE6F8A000)

/* Lock used while modifying register */
static DEFINE_SPINLOCK(io_lock);

static int check_sec_rlte_hw_rev(void);
void (*shmobile_arch_reset)(char mode, const char *cmd);

int sec_rlte_hw_rev;
unsigned int u2_board_rev;
static void __iomem *sbsc_sdmracr1a;

//extern struct d2153_pdata d2153_pdata;

void sh_modify_register8(unsigned int addr, u8 clear, u8 set)
{
	unsigned long flags;
	u8 val;
	spin_lock_irqsave(&io_lock, flags);
	val = *(volatile u8 *)addr;
	val &= ~clear;
	val |= set;
	*(volatile u8 *)addr = val;
	spin_unlock_irqrestore(&io_lock, flags);
}
EXPORT_SYMBOL_GPL(sh_modify_register8);

void sh_modify_register16(unsigned int addr, u16 clear, u16 set)
{
	unsigned long flags;
	u16 val;
	spin_lock_irqsave(&io_lock, flags);
	val = *(volatile u16 *)addr;
	val &= ~clear;
	val |= set;
	*(volatile u16 *)addr = val;
	spin_unlock_irqrestore(&io_lock, flags);
}
EXPORT_SYMBOL_GPL(sh_modify_register16);

void sh_modify_register32(unsigned int addr, u32 clear, u32 set)
{
	unsigned long flags;
	u32 val;
	spin_lock_irqsave(&io_lock, flags);
	val = *(volatile u32 *)addr;
	val &= ~clear;
	val |= set;
	*(volatile u32 *)addr = val;
	spin_unlock_irqrestore(&io_lock, flags);
}
EXPORT_SYMBOL_GPL(sh_modify_register32);

unsigned int u2_get_board_rev(void)
{
	return u2_board_rev;
}
EXPORT_SYMBOL_GPL(u2_get_board_rev);

static int u2_read_board_rev(char *page, char **start, off_t off,
		int count, int *eof, void *data)
{
	count = snprintf(page, count, "%x", u2_board_rev);
	return count;
}

/* MMCIF */
/*static struct sh_mmcif_dma sh_mmcif_dma = {
	.chan_priv_rx	= {
		.slave_id	= SHDMA_SLAVE_MMCIF0_RX,
	},
	.chan_priv_tx	= {
		.slave_id	= SHDMA_SLAVE_MMCIF0_TX,
	},
};*/

#if defined(CONFIG_MFD_D2153)
static struct regulator *emmc_regulator;

void d2153_mmcif_pwr_control(int onoff)
{
	int ret;

	printk(KERN_EMERG "%s %s\n", __func__, (onoff) ? "on" : "off");

	if(emmc_regulator == NULL) {
		printk(" %s, %d \n", __func__, __LINE__ );			
		emmc_regulator = regulator_get(NULL, "vmmc"); 
		if(IS_ERR(emmc_regulator)){
			printk("can not get vmmc regulator\n");
			return;
		}
	}

	if(onoff==1) {
		printk(" %s, %d vmmc On\n", __func__, __LINE__ );	
		printk(" %s, %d \n", __func__, __LINE__ );
		ret = regulator_enable(emmc_regulator);
		printk("regulator_enable ret = %d \n", ret);	
	}else {
		printk("%s, %d vmmc Off\n", __func__, __LINE__ );	
		ret = regulator_disable(emmc_regulator);
		printk("regulator_disable ret = %d \n", ret);			
	}
}
#endif


static void mmcif_set_pwr(struct platform_device *pdev, int state)
{
#if defined(CONFIG_MFD_D2153)
	d2153_mmcif_pwr_control(1);
#endif /* CONFIG_MFD_D2153 */
}

static void mmcif_down_pwr(struct platform_device *pdev)
{
#if defined(CONFIG_MFD_D2153)
	d2153_mmcif_pwr_control(0);
#endif /* CONFIG_MFD_D2153 */
}


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
	.resource		= renesas_mmcif_resources,
	.num_resources	= ARRAY_SIZE(renesas_mmcif_resources),
};

static struct mmcoops_platform_data mmcoops_info = {
#ifdef CONFIG_CRASHLOG_EMMC
	.pdev			= &mmcif_device,
	.start			= MMCOOPS_START_OFFSET,
	.size			= MMCOOPS_LOG_SIZE,
	.record_size		= MMCOOPS_RECORD_SIZE,
	.kmsg_size		= MMCOOPS_KMSG_SIZE,
	.logcat_main_size	= MMCOOPS_LOGCAT_MAIN_SIZE,
	.logcat_system_size	= MMCOOPS_LOGCAT_SYSTEM_SIZE,
	.logcat_radio_size	= MMCOOPS_LOGCAT_RADIO_SIZE,
	.logcat_events_size	= MMCOOPS_LOGCAT_EVENTS_SIZE,
#else
	.start			= MMCOOPS_START_OFFSET_DDR,
	.size			= MMCOOPS_LOG_SIZE_DDR,
	.record_size		= MMCOOPS_RECORD_SIZE_DDR,
	.kmsg_size		= MMCOOPS_KMSG_SIZE_DDR,
	.logcat_main_size	= MMCOOPS_LOGCAT_MAIN_SIZE_DDR,
	.logcat_system_size	= MMCOOPS_LOGCAT_SYSTEM_SIZE_DDR,
	.logcat_radio_size	= MMCOOPS_LOGCAT_RADIO_SIZE_DDR,
	.logcat_events_size	= MMCOOPS_LOGCAT_EVENTS_SIZE_DDR,
#endif
	.local_version		= MMCOOPS_LOCAL_VERSION,
	.soft_version		= RMC_LOCAL_VERSION,
	/*512 byte blocks */
};

static struct platform_device mmcoops_device = {
	.name   = "mmcoops",
	.dev    = {
		.platform_data  = &mmcoops_info,
	},
};

static struct fsi_d2153_platform_data gardalte_audio_pdata = {
	.gpio_spkr_en		= -1,
	.gpio_hp_det		= GPIO_PORT24,
	.gpio_hp_mute		= -1,
	.gpio_int_mic_en	= -1,
	.gpio_ext_mic_en	= -1,
	.private_data		= NULL,
};

static struct platform_device gardalte_audio_device = {
	.name	= "fsi-snd-d2153",
	.id	= 0,
	.dev	= {
		.platform_data  = &gardalte_audio_pdata,
	},
};

static struct sh_fsi_platform_info fsi_info = {
	.port_flags = 	SH_FSI_OUT_SLAVE_MODE | SH_FSI_IN_SLAVE_MODE	|
		SH_FSI_BRS_INV | SH_FSI_OFMT(I2S) |	SH_FSI_IFMT(I2S),
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

static struct platform_device fsi_device = {
	.name		= "sh_fsi2",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(fsi_resources),
	.resource	= fsi_resources,
	.dev	= {
		.platform_data	= &fsi_info,
	},
};

static struct sh_fsi_platform_info fsi_b_info = {
	.port_flags = SH_FSI_BRM_INV | SH_FSI_LRM_INV | SH_FSI_OFMT(I2S) | 
		SH_FSI_IFMT(I2S),
	.always_slave	= 1,
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

static struct platform_device fsi_b_device = {
	.name			= "sh_fsi2",
	.id				= 1,
	.num_resources	= ARRAY_SIZE(fsi_b_resources),
	.resource	= fsi_b_resources,
	.dev	= {
		.platform_data	= &fsi_b_info,
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
		.name	= "panel_power_port",
		.start	= GPIO_PORT89,
		.end	= GPIO_PORT89,
		.flags	= IORESOURCE_MEM,
	},
	[3] = {
		.name	= "panel_irq_port",
		.start	= GPIO_PORT27,
		.end	= GPIO_PORT27,
		.flags	= IORESOURCE_MEM,
	},
};

static struct platform_device lcdc_device = {
	.name			= "sh_mobile_lcdc_fb",
	.num_resources	= ARRAY_SIZE(lcdc_resources),
	.resource		= lcdc_resources,
	.dev	= {
		.platform_data  	= &lcdc_info,
		.coherent_dma_mask 	= DMA_BIT_MASK(32),
	},
};

static struct ktd253ehd_led_platform_data ktd253ehd_led_info = {
	.gpio_port = GPIO_PORT47,
};

static struct platform_device led_backlight_device = {
	.name	= "ktd253ehd_led",
	.dev	= {
		.platform_data  = &ktd253ehd_led_info,
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
		.flag = 1,
		.port = GPIO_PORT36,
		/* GPIO settings to be retained at resume state */
		.active = {
			.port_fn	= GPIO_FN_PORT36_TPU0TO0,//GPIO_FN_TPUTO0,/*Func 3*/
			.pull		= PORTn_CR_PULL_DOWN,
			.direction	= PORTn_CR_DIRECTION_NOT_SET,
			.output_level	= PORTn_OUTPUT_LEVEL_NOT_SET,
		},
		/* GPIO settings to be set at suspend state */
		.inactive = {
			.port_fn	= GPIO_PORT36, /*Func 0*/
			.pull		= PORTn_CR_PULL_OFF,
			.direction	= PORTn_CR_DIRECTION_OUTPUT,
			.output_level	= PORTn_OUTPUT_LEVEL_LOW,
		}
	},
};

static struct port_info
tpu_pwm_pfc[TPU_MODULE_MAX][TPU_CHANNEL_MAX] = {
	[TPU_MODULE_0] = {
		[TPU_CHANNEL_0]	= {
			.port_func	=  GPIO_FN_PORT36_TPU0TO0, //GPIO_FN_TPUTO0,
			.func_name	= "pwm-tpu0to0",
			.port_count	= ARRAY_SIZE(tpu0_gpio_setting_info),
			.tpu_gpio_setting_info	= tpu0_gpio_setting_info,
		},
		[TPU_CHANNEL_1]	= {
			.port_func	=  GPIO_FN_TPU0TO1,//GPIO_FN_TPUTO1,
			.func_name	= "pwm-tpu0to1",
			.port_count 	= 0,
			.tpu_gpio_setting_info	= NULL,
		},
		[TPU_CHANNEL_2]	= {
			.port_func	=  GPIO_FN_TPU0TO2,//GPIO_FN_TPUTO2,
			.func_name	= "pwm-tpu0to2",
			.port_count 	= 0,
			.tpu_gpio_setting_info	= NULL,
		},
		[TPU_CHANNEL_3]	= {
			.port_func	=  GPIO_FN_TPU0TO3,//GPIO_FN_TPUTO3,
			.func_name	= "pwm-tpu0to3",
			.port_count 	= 0,
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

#if (defined(CONFIG_BCM_RFKILL) || defined(CONFIG_BCM_RFKILL_MODULE))
#define BCMBT_VREG_GPIO       (GPIO_PORT268)
#define BCMBT_N_RESET_GPIO    (-1)
/* clk32 */
#define BCMBT_AUX0_GPIO       (-1)  
/* UARTB_SEL */
#define BCMBT_AUX1_GPIO       (-1)    

static struct bcmbt_rfkill_platform_data board_bcmbt_rfkill_cfg = {
	.vreg_gpio 		= BCMBT_VREG_GPIO,
	.n_reset_gpio 	= BCMBT_N_RESET_GPIO,
	/* CLK32 */
	.aux0_gpio 		= BCMBT_AUX0_GPIO,  
	/* UARTB_SEL, probably not required */
	.aux1_gpio 		= BCMBT_AUX1_GPIO,  
};

static struct platform_device board_bcmbt_rfkill_device = {
	.name 	= "bcmbt-rfkill",
	.id 	= -1,
	.dev 	= {
		.platform_data=&board_bcmbt_rfkill_cfg,
	},
};
#endif

#ifdef CONFIG_BCM_BZHW

#define GPIO_BT_WAKE 	GPIO_PORT262
#define GPIO_HOST_WAKE 	GPIO_PORT272

static struct bcm_bzhw_platform_data bcm_bzhw_data = {
	.gpio_bt_wake 	= GPIO_BT_WAKE,
	.gpio_host_wake = GPIO_HOST_WAKE,
};

static struct platform_device board_bcm_bzhw_device = {
	.name = "bcm_bzhw",
	.id = -1,
	.dev = {
		.platform_data = &bcm_bzhw_data,
	},
};
#endif

#ifdef CONFIG_BCM_BT_LPM
#define GPIO_BT_WAKE   6 
#define GPIO_HOST_WAKE 14

static struct bcm_bt_lpm_platform_data brcm_bt_lpm_data = {
	.gpio_bt_wake = GPIO_BT_WAKE,
	.gpio_host_wake = GPIO_HOST_WAKE,
};

static struct platform_device board_bcmbt_lpm_device = {
	.name 	= "bcmbt-lpm",
	.id 	= -1,
	.dev 	= {
		.platform_data=&brcm_bt_lpm_data,
	},
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

static struct resource stm_res[] = {
	[0] = {
		.name	= "stm_ctrl",
		.start	= 0xe6f89000,
		.end	= 0xe6f89fff,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.name	= "stm_ports",
		.start	= 0xe9000000,
		.end	= 0xe9000fff,
		.flags	= IORESOURCE_MEM,
	},
	[2] = {
		.name	= "funnel",
		.start	= 0xe6f8b000,
		.end	= 0xe6f8bfff,
		.flags	= IORESOURCE_MEM,
	},
};

static struct platform_device stm_device = {
	.name = "stm",
	.num_resources	= ARRAY_SIZE(stm_res),
	.resource	= stm_res,
};

struct a2220_platform_data  gardalte_a2220_data = {
	.a2220_hw_init = NULL,
	.gpio_reset = GPIO_PORT44,
	.gpio_wakeup = GPIO_PORT26,
};

/* THREE optional gardalte_devices pointer lists for initializing the platform
 * devices 
 */

/* For different STM muxing options 0, 1, or None, as given by 
 * boot_command_line parameter stm=0/1/n 
 */
static struct platform_device *gardalte_devices_stm_sdhi1_d2153[] __initdata = {

	&usbhs_func_device_d2153,
#ifdef CONFIG_USB_R8A66597_HCD
	&usb_host_device,
#endif
#ifdef CONFIG_USB_OTG
	&tusb1211_device,
#endif
	&mmcif_device,
	&mmcoops_device,
	&sdhi0_device,
#if defined(CONFIG_RENESAS_BT)
	&bcm4334_bluetooth_device,
#endif
	&fsi_device,
	&fsi_b_device,
	&gardalte_audio_device,
	&gpio_key_device,
	&lcdc_device,
	&mfis_device,
	&mdm_reset_device,
#ifdef CONFIG_SPI_SH_MSIOF
	&sh_msiof0_device,
#endif
	&u2evm_ion_device,
#if (defined(CONFIG_BCM_RFKILL) || defined(CONFIG_BCM_RFKILL_MODULE))
	&board_bcmbt_rfkill_device,
#endif

#ifdef CONFIG_BCM_BZHW
	&board_bcm_bzhw_device,
#endif

#ifdef CONFIG_BCM_BT_LPM
	&board_bcmbt_lpm_device,
#endif
	&thermal_sensor_device,

	&rcu0_device,
	&rcu1_device,

	&camera_devices[0],
	&camera_devices[1],
	&stm_device,
#if defined(CONFIG_RENESAS_NFC)
#ifdef CONFIG_PN544_NFC
	&pn544_i2c_gpio_device,
#endif
#endif
};

static struct platform_device *gardalte_devices_stm_sdhi0_d2153[] __initdata = {

	&usbhs_func_device_d2153,
#ifdef CONFIG_USB_R8A66597_HCD
	&usb_host_device,
#endif
#ifdef CONFIG_USB_OTG
	&tusb1211_device,
#endif
	&mmcif_device,
	&mmcoops_device,
	&sdhi1_device,
#if defined(CONFIG_RENESAS_BT)
	&bcm4334_bluetooth_device,
#endif
	&fsi_device,
	&fsi_b_device,
	&gardalte_audio_device,
	&gpio_key_device, 
	&lcdc_device,
	&mfis_device,
	&tpu_devices[TPU_MODULE_0],
	&mdm_reset_device,
#ifdef CONFIG_SPI_SH_MSIOF
	&sh_msiof0_device,
#endif
	&u2evm_ion_device,
#if (defined(CONFIG_BCM_RFKILL) || defined(CONFIG_BCM_RFKILL_MODULE))
	&board_bcmbt_rfkill_device,
#endif
#ifdef CONFIG_BCM_BZHW
	&board_bcm_bzhw_device,
#endif

#ifdef CONFIG_BCM_BT_LPM
	&board_bcmbt_lpm_device,
#endif
	&thermal_sensor_device,

	&rcu0_device,
	&rcu1_device,

	&camera_devices[0],
	&camera_devices[1],
	&stm_device,
#if defined(CONFIG_RENESAS_NFC)
#ifdef CONFIG_PN544_NFC
	&pn544_i2c_gpio_device,
#endif
#endif
};

static struct platform_device *gardalte_devices_stm_none_d2153[] __initdata = {

	&usbhs_func_device_d2153,
#ifdef CONFIG_USB_R8A66597_HCD
	&usb_host_device,
#endif
#ifdef CONFIG_USB_OTG
	&tusb1211_device,
#endif
	&mmcif_device,
	&mmcoops_device,
	&sdhi0_device,
	&sdhi1_device,
#if defined(CONFIG_RENESAS_BT)
	&bcm4334_bluetooth_device,
#endif
	&fsi_device,
	&fsi_b_device,
	&gardalte_audio_device,
	&gpio_key_device,
	&lcdc_device,
	&mfis_device,
	&tpu_devices[TPU_MODULE_0],
	&mdm_reset_device,
#ifdef CONFIG_SPI_SH_MSIOF
	&sh_msiof0_device,
#endif
	&u2evm_ion_device,
#if (defined(CONFIG_BCM_RFKILL) || defined(CONFIG_BCM_RFKILL_MODULE))
	&board_bcmbt_rfkill_device,
#endif
#ifdef CONFIG_BCM_BZHW
	&board_bcm_bzhw_device,
#endif
#ifdef CONFIG_BCM_BT_LPM
	&board_bcmbt_lpm_device,
#endif
	&thermal_sensor_device,

	&rcu0_device,
	&rcu1_device,

	&camera_devices[0],
	&camera_devices[1],
#if defined(CONFIG_RENESAS_NFC)
#ifdef CONFIG_PN544_NFC
	&pn544_i2c_gpio_device,
#endif
#endif
#if defined(CONFIG_SAMSUNG_MHL)
	&mhl_i2c_gpio_device,
#endif	
};

/* I2C */
static struct i2c_board_info __initdata i2c0_devices_d2153[] = {
#if defined(CONFIG_MFD_D2153)
	{
		/* for D2153 PMIC driver */
		I2C_BOARD_INFO("d2153", D2153_PMIC_I2C_ADDR),
		.platform_data = &d2153_pdata,
		.irq = irqpin2irq(28),
	},
#endif /* CONFIG_MFD_D2153 */
};

static struct platform_ktd259b_backlight_data bcm_ktd259b_backlight_data = {
	.max_brightness = 255,
	.dft_brightness = 143,
	.ctrl_pin = 47,
};

static struct platform_device bcm_backlight_devices = {
	.name	= "panel",
	.id 	= -1,
	.dev 	= {
		.platform_data = &bcm_ktd259b_backlight_data,
	},
};

/* Rhea Ray specific platform devices */
static struct platform_device *guardian__plat_devices[] __initdata = {
	&bcm_backlight_devices,
};

static struct i2c_board_info i2c4_devices_melfas[] = {
	{
		I2C_BOARD_INFO("sec_touch", 0x48),
		.irq = irqpin2irq(32),
	},
};

#ifdef CONFIG_BOARD_VERSION_GARDA
static struct i2c_board_info i2c4_devices_imagis[] = {
	{
		I2C_BOARD_INFO("IST30XX", 0xA0>>1),
		.irq = irqpin2irq(32),
	},
};

/* Touch Panel auto detection for Garda_Kyle (Melfas and Imagis) */
static struct i2c_client *tsp_detector_i2c_client;

static int __devinit tsp_detector_probe(struct i2c_client *client,
		const struct i2c_device_id * id)
{
	int ret=0;
	struct i2c_adapter *adap = client->adapter;
	struct regulator *touch_regulator;
	unsigned short addr_list_melfas[] = { 0x48, I2C_CLIENT_END };

	touch_regulator = regulator_get(NULL, "vtsp_3v");
	if(IS_ERR(touch_regulator)){
		printk(KERN_ERR "failed to get regulator for Touch Panel");
		return -ENODEV;
	}
	regulator_set_voltage(touch_regulator, 3000000, 3000000); /* 3.0V */
	regulator_enable(touch_regulator);
	msleep(20);

	if ( (tsp_detector_i2c_client = 
				i2c_new_probed_device(adap,&i2c4_devices_melfas[0], addr_list_melfas, NULL))){
		printk(KERN_INFO "Touch Panel: Melfas MMS-13X\n");
	} else {
		tsp_detector_i2c_client = i2c_new_device(adap, &i2c4_devices_imagis[0]);
		printk(KERN_INFO "Touch Panel: Imagis IST30XX\n");
	}

	regulator_disable(touch_regulator);
	regulator_put(touch_regulator);
	return ret;
}

static int tsp_detector_remove(struct i2c_client *client)
{
	i2c_unregister_device(tsp_detector_i2c_client);
	tsp_detector_i2c_client = NULL;
	return 0;
}

static struct i2c_device_id tsp_detector_idtable[] = {
	{ "tsp_detector", 0 },
	{},
};
static struct i2c_driver tsp_detector_driver = {
	.driver = {
		.name = "tsp_detector",
	},
	.probe 		= tsp_detector_probe,
	.remove 	= tsp_detector_remove,
	.id_table 	= tsp_detector_idtable,
};

static struct i2c_board_info i2c4_devices_tsp_detector[] = {
	{
		I2C_BOARD_INFO("tsp_detector", 0x7f),
	},
};

static struct led_regulator_platform_data key_backlight_data = {
	.name   = "button-backlight",
};

static struct platform_device key_backlight_device = {
	.name = "leds-regulator",
	.id   = 0,
	.dev  = {
		.platform_data = &key_backlight_data,
	},
};
#endif /* CONFIG_BOARD_VERSION_GARDA */

static struct i2c_board_info i2cm_devices_d2153[] = {
	{
		I2C_BOARD_INFO("audience_a2220", 0x3E),
		.platform_data = &gardalte_a2220_data,
	},
};


static struct platform_device *gpio_i2c_devices[] __initdata = {
};

static struct map_desc gardalte_io_desc[] __initdata = {
	{
		.virtual	= 0xe6000000,
		.pfn		= __phys_to_pfn(0xe6000000),
		.length		= SZ_256M,
		.type		= MT_DEVICE
	},
	{
		/*
		 * Create 4 MiB of virtual address hole within a big 1:1 map
		 * requested above, which is dedicated for the RT-CPU driver.
		 *
		 * According to the hardware manuals, physical 0xefc00000
		 * space is reserved for Router and a data abort error will
		 * be generated if access is made there.  So this partial
		 * mapping change won't be a problem.
		 */
		.virtual        = 0xefc00000,
		.pfn            = __phys_to_pfn(0xffc00000),
		.length         = SZ_4M,
		.type           = MT_DEVICE
	},
#if defined(CONFIG_SEC_DEBUG_INFORM_IOTABLE)
	{
		.virtual        = SEC_DEBUG_INFORM_VIRT,
		.pfn            = __phys_to_pfn(SEC_DEBUG_INFORM_PHYS),
		.length         = SZ_4K,
		.type           = MT_UNCACHED,
	},
#endif
};

static void __init gardalte_map_io(void)
{
	iotable_init(gardalte_io_desc, ARRAY_SIZE(gardalte_io_desc));

#if defined(CONFIG_SEC_DEBUG_INFORM_IOTABLE)
	sec_debug_init();
#endif

	//r8a7373_add_early_devices();
        r8a7373_init_early();
	shmobile_setup_console();
}

void __init gardalte_init_irq(void)
{
	r8a7373_init_irq();
}

#ifdef CONFIG_U2_STM_ETR_TO_SDRAM
static int wait_for_coresight_access_lock(u32 base)
{
	int i = 0x00;
	int retval = -1;
	int timeout = 512;
	/* Lock Access */
	__raw_writel(0xc5acce55, base + 0xFB0); 
	for (i = 0; i < timeout && retval; i++) {
		if ((__raw_readl(base + 0xFB4) & 2) == 0)
			retval = 0;
	}
	printk("wait_for_coresight_access_lock %d\n", retval);
	return retval;
}
#endif

void gardalte_restart(char mode, const char *cmd)
{
	printk(KERN_INFO "%s\n", __func__);
	shmobile_do_restart(mode, cmd, APE_RESETLOG_U2EVM_RESTART);
}
static void SBSC_Init_520Mhz(void)
{
	unsigned long work;

	printk(KERN_ALERT "START < %s >\n", __func__);

	/* Check PLL3 status */
	work = __raw_readl(CPG_PLLECR);
	if (!(work & CPG_PLLECR_PLL3ST)) {
		printk(KERN_ALERT "CPG_PLLECR_PLL3ST is 0\n");
		return;
	}

	/* Set PLL3 = 1040 Mhz*/
	__raw_writel(CPG_PLL3CR_1040MHZ, CPG_PLL3CR);

	/* Wait PLL3 status on */
	while (1) {
		work = __raw_readl(CPG_PLLECR);
		work &= CPG_PLLECR_PLL3ST;
		if (work == CPG_PLLECR_PLL3ST)
			break;
	}

	/* Dummy read */
	__raw_readl(sbsc_sdmracr1a);
}

/* For PA devices */
#include <setup-u2pa.c>

static void __init gardalte_init(void)
{
	char *cp=&boot_command_line[0];
	int ci;
	unsigned int i = 0x00;
	volatile uint32_t val;
	volatile unsigned long dummy_read;
	int pub_stm_select =-1;
	int stm_select =-1;	
	void __iomem *sbsc_sdmra_28200 = 0;
	void __iomem *sbsc_sdmra_38200 = 0;
        printk(KERN_ERR "boot command line is\n");
	/* 	Shall tell how to route STM traces.
		Taken from boot_command_line[] parameters.
		stm=# will set parameter, if '0' or '1' then as number, otherwise -1.
		-1 = NONE, i.e. SDHI1 and SDHI0 are free for other functions.
		0 = SDHI0 used for STM traces. SD-Card not enabled.
		1 = SDHI1 used for STM traces. WLAN not enabled. [DEFAULT if stm boot para not defined]
		*/	
	/* ES2.02 / LPDDR2 ZQ Calibration Issue WA */

	u8 reg8 = __raw_readb(STBCHRB3);

	if ((reg8 & 0x80) && ((system_rev & 0xFFFF) >= 0x3E12)) {
		printk(KERN_ALERT "< %s >Apply for ZQ calibration\n", __func__);
		printk(KERN_ALERT "< %s > Before CPG_PLL3CR 0x%8x\n",
				__func__, __raw_readl(CPG_PLL3CR));
		sbsc_sdmracr1a   = ioremap(SBSC_BASE + 0x400088, 0x4);
		sbsc_sdmra_28200 = ioremap(SBSC_BASE + 0x528200, 0x4);
		sbsc_sdmra_38200 = ioremap(SBSC_BASE + 0x538200, 0x4);
		if (sbsc_sdmracr1a && sbsc_sdmra_28200 && sbsc_sdmra_38200) {
			SBSC_Init_520Mhz();
			__raw_writel(SBSC_SDMRACR1A_ZQ, sbsc_sdmracr1a);
			__raw_writel(SBSC_SDMRA_DONE, sbsc_sdmra_28200);
			__raw_writel(SBSC_SDMRA_DONE, sbsc_sdmra_38200);
		} else {
			printk(KERN_ERR "%s: ioremap failed.\n", __func__);
		}
		printk(KERN_ALERT "< %s > After CPG_PLL3CR 0x%8x\n",
				__func__, __raw_readl(CPG_PLL3CR));
		if(sbsc_sdmracr1a)
			iounmap(sbsc_sdmracr1a);
		if(sbsc_sdmra_28200)
			iounmap(sbsc_sdmra_28200);
		if(sbsc_sdmra_38200)
			iounmap(sbsc_sdmra_38200);
	}


	val = __raw_readl(DBGREG1);
	if ((val & (1 << 29)) == 0) {
		stm_select = -1;
	} else {
		if ((val & (1 << 20)) == 0) {
			stm_select = 0;
		} else {
			stm_select = 1;
		}
	}

	printk("sec stm_select=%d\n", stm_select);

	/* pub_stm_select = stm_select;*/
	/* Only if Secure side allows debugging */

	if (stm_select >= 0) { 
		if (cp[0] && cp[1] && cp[2] && cp[3] && cp[4]) {
			for (ci=4; cp[ci]; ci++) {
				if (cp[ci-4] == 's' && cp[ci-3] == 't' && cp[ci-2] == 'm' && 
						cp[ci-1] == '=') {

					switch (cp[ci]) {
						case '0': 
							pub_stm_select =  0; 
							break;
						case '1': 
							pub_stm_select =  1;
							break;
						default:  
							pub_stm_select = -1;
							stm_select = -1;
							break;
					}
					break;
				}
			}
		}
	}

	if (-1 == pub_stm_select)
		stm_select = -1;

	printk("pub_stm_select=%d\n", pub_stm_select);

	r8a7373_pinmux_init();

#if 0
	/* set board version */
	u2_board_rev = 0;
	gpio_request(GPIO_PORT75, NULL);
	gpio_direction_input(GPIO_PORT75);
	u2_board_rev |= gpio_get_value(GPIO_PORT75) << 3;
	gpio_request(GPIO_PORT74, NULL);
	gpio_direction_input(GPIO_PORT74);
	u2_board_rev |= gpio_get_value(GPIO_PORT74) << 2;
	gpio_request(GPIO_PORT73, NULL);
	gpio_direction_input(GPIO_PORT73);
	u2_board_rev |= gpio_get_value(GPIO_PORT73) << 1;
	gpio_request(GPIO_PORT72, NULL);
	gpio_direction_input(GPIO_PORT72);
	u2_board_rev |= gpio_get_value(GPIO_PORT72);
#endif
	/* This is hack to make driver using get_u2_board_rev() work fine 
 	   should be either removed or modified to get Garda HW rev 
	 */	
	u2_board_rev = 5;
	/* Temporary workaround */
	if (u2_board_rev == 1)
		u2_board_rev = RLTE_BOARD_REV_0_1;

	create_proc_read_entry("board_revision", 0444, NULL,u2_read_board_rev,
			NULL);

	r8a7373_add_standard_devices();

	r8a7373_hwlock_gpio = hwspin_lock_request_specific(SMGPIO);
	r8a7373_hwlock_cpg = hwspin_lock_request_specific(SMCPG);
	r8a7373_hwlock_sysc = hwspin_lock_request_specific(SMSYSC);
	pinmux_hwspinlock_init(r8a7373_hwlock_gpio);

#ifdef CONFIG_ARM_TZ
	/* ES2.02 and onwards */
	if((system_rev & 0xFFFF) >= 0x3E12) 
	{
		printk(KERN_DEBUG "ES2.02 on later with TZ enabled\n");
		/* Can't override secure side by public side any more */
		pub_stm_select = 0; 
	} else {
		printk(KERN_DEBUG "ES2.01 or earlier or TZ enabled\n");
		if (stm_select != pub_stm_select) {
			stm_select = pub_stm_select;
			/* Override secure side by public side */
			pub_stm_select = 1; 
		} else {
			/* Both secure and public agree.No need to change HW setup */
			pub_stm_select = 0; 
		}
	}
#else
	printk(KERN_DEBUG "ES2.01 or earlier or TZ disabled\n");
	if (stm_select != pub_stm_select) {
		stm_select = pub_stm_select;
		/* Override secure side by public side */
		pub_stm_select = 1; 
	} else {
		/* Both secure and public agree.No need to change HW setup */
		pub_stm_select = 0;
	}
#endif
	printk(KERN_DEBUG "final stm_select=%d\n", stm_select);

	if(((system_rev & 0xFFFF)>>4) >= 0x3E1)
	{

		*GPIO_DRVCR_SD0 = 0x0023;
		*GPIO_DRVCR_SIM1 = 0x0023;
		*GPIO_DRVCR_SIM2 = 0x0023;
	}
	shmobile_arch_reset = gardalte_restart;

	sec_rlte_hw_rev = check_sec_rlte_hw_rev();
	printk(KERN_INFO "%s hw rev : %d \n", __func__, sec_rlte_hw_rev);

	/* SCIFA0 */
	gpio_request(GPIO_FN_SCIFA0_TXD, NULL);
	gpio_request(GPIO_FN_SCIFA0_RXD, NULL);

	/* Bluetooth UART settings (ttySC4) */

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

	gpio_pull_up_port(GPIO_PORT44);
	gpio_pull_up_port(GPIO_PORT45);
	gpio_pull_up_port(GPIO_PORT46);
	gpio_pull_up_port(GPIO_PORT47);
	gpio_pull_up_port(GPIO_PORT48);
	gpio_pull_up_port(GPIO_PORT96);
	gpio_pull_up_port(GPIO_PORT97);
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
	gpio_request(GPIO_FN_MMCRST, NULL);

	/* Disable GPIO Enable at initialization */

	/* ===== CWS GPIO ===== */

	/* GPS Reset */
	gpio_request(GPIO_PORT10, NULL);
	gpio_direction_output(GPIO_PORT10, 0);

	/* GPS Enable */
	gpio_request(GPIO_PORT11, NULL);
	gpio_direction_output(GPIO_PORT11, 0);

#if defined(CONFIG_RENESAS_NFC)
	nfc_gpio_init();
#endif

	/* MAIN MIC LDO Enable */
	gpio_request(GPIO_PORT8, NULL);
	gpio_direction_output(GPIO_PORT8, 0);

	gpio_direction_none_port(GPIO_PORT309);

	if (0 != stm_select) {
		/* If STM Traces go to SDHI1 or NOWHERE, then SDHI0 can be used for SD-Card */

		/* SDHI0 */
		gpio_request(GPIO_FN_SDHID0_0, NULL);
		gpio_request(GPIO_FN_SDHID0_1, NULL);
		gpio_request(GPIO_FN_SDHID0_2, NULL);
		gpio_request(GPIO_FN_SDHID0_3, NULL);
		gpio_request(GPIO_FN_SDHICMD0, NULL);
		gpio_direction_none_port(GPIO_PORT326);
		gpio_request(GPIO_FN_SDHICLK0, NULL);
		gpio_request(GPIO_PORT327, NULL);
		gpio_direction_input(GPIO_PORT327);
		irq_set_irq_type(irqpin2irq(50), IRQ_TYPE_EDGE_BOTH);
		gpio_set_debounce(GPIO_PORT327, 1000);	/* 1msec */
	}

	if (1 == stm_select) {
		/* FIRST, CONFIGURE STM CLK AND DATA PINMUX */
		/* SDHI1 used for STM Data, STM Clock, and STM SIDI */
		gpio_request(GPIO_FN_STMCLK_2, NULL);
		gpio_request(GPIO_FN_STMDATA0_2, NULL); 
		gpio_request(GPIO_FN_STMDATA1_2, NULL);
		gpio_request(GPIO_FN_STMDATA2_2, NULL);
		gpio_request(GPIO_FN_STMDATA3_2, NULL);

	} else if (0 == stm_select) {
		/* FIRST, CONFIGURE STM CLK AND DATA PINMUX using SDHI0 as port */
		/* SDHI0 used for STM Data, STM Clock */
		gpio_request(GPIO_FN_STMCLK_1, NULL);
		gpio_request(GPIO_FN_STMDATA0_1, NULL); 
		gpio_request(GPIO_FN_STMDATA1_1, NULL);
		gpio_request(GPIO_FN_STMDATA2_1, NULL);
		gpio_request(GPIO_FN_STMDATA3_1, NULL);

	}

	/* 	Module function select register 3 (MSEL3CR/MSEL03CR)  at 0xE6058020 
		Write bit 28 up to enable SDHIx STMSIDI power

		bits [31:20] All 0, R, Reserved
		bit  28		MSEL28, Initial value 0, R/W, IO power supply of 
		terminal SDHI when SD is transmitted.
		0=IO power OFF, 1=IO power ON
		bits [27:16] All 0, R, Reserved.
		bit  15     MSEL15, Initial value 0, R/W, Debug monitor function Setting.
		0 = Use KEYSC pins for debug monitor function
		1 = Use BSC pins for debug monitor function.
		bits [14:4]  All 0, R, Reserved
		bit  3   	MSEL3, Initial value 0, R/W, IC_DP/IC_DM Output Enable Control.
		0 = Output Disable, 1 = Depends on ICUSB Controller. 
		Set 0 before "power down sequence".
		bit  2		0, R, Reserved.
		bits [1:0]   MSEL[1:0], Initial value 00, R/W, Select HSI
		0x = Internal connect Port xxx(HSI) shall set func0.
		10 = HSI0 select.
		11 = HSIB select.
		*/

	/* SECOND, ENABLE TERMINAL POWER FOR STM CLK AND DATA PINS */
	__raw_writel(__raw_readl(MSEL3CR) | (1<<27), MSEL3CR); /* ES2.0: SIM powers */

	if (-1 != stm_select) {	
		__raw_writel(__raw_readl(MSEL3CR) | (1<<28), MSEL3CR);
	}

	/* THIRD, PINMUX STM SIDI (i,e, return channel) MUX FOR BB/MODEM */
	/* ALSO, CONFIGURE SYS-(TRACE) FUNNEL-STM, and SYS-TPIU-STM */

	if (1 == stm_select) {
		gpio_request(GPIO_FN_STMSIDI_2, NULL);
		gpio_pull_up_port(GPIO_PORT293);
	}

	if (0 == stm_select) {
		gpio_request(GPIO_FN_STMSIDI_1, NULL);
		gpio_pull_up_port(GPIO_PORT324);

	}
	if (-1 != stm_select) {
		if (pub_stm_select) {
			/* Key register */
			__raw_writel(0x0000a501, DBGREG9); 
			/* Key register, must write twice! */
			__raw_writel(0x0000a501, DBGREG9); 
		}

		for(i=0; i<0x10; i++);

		if ((1 == stm_select) && pub_stm_select) {
			__raw_writel((__raw_readl(DBGREG1) & 0xFFDFFFFF) | (1<<20), DBGREG1);
		}

		if ((0 == stm_select) && pub_stm_select) {
			__raw_writel((__raw_readl(DBGREG1) & 0xFFCFFFFF), DBGREG1);
		}

		for(i=0; i<0x10; i++);

		/* Configure SYS-(Trace) Funnel-STM @ 0xE6F8B000 */
		/* TODO: check if delays and double writing really needed or not? */
		/* Lock Access */
		__raw_writel(0xc5acce55, SYS_TRACE_FUNNEL_STM_BASE + 0xFB0);

		for(i=0; i<0xF0; i++);

		/* 	Enable only Slave port 1, i.e. Modem top-level funnel for STM, 0x303
			for APE also 
			*/
		__raw_writel(     0x302, SYS_TRACE_FUNNEL_STM_BASE + 0x000);

		for(i=0; i<0xF0; i++);
		/* Lock Access */
		__raw_writel(0xc5acce55, SYS_TRACE_FUNNEL_STM_BASE + 0xFB0); 

		for(i=0; i<0x10; i++);

		/* 	Enable only Slave port 1, i.e. Modem top-level funnel for STM, 0x303
			for APE also 
			*/
		__raw_writel(     0x302, SYS_TRACE_FUNNEL_STM_BASE + 0x000); 

		for(i=0; i<0xF0; i++);

		/* Configure SYS-TPIU-STM @ 0xE6F8A000 */

		/* Lock Access */
		__raw_writel(0xc5acce55, SYS_TPIU_STM_BASE + 0xFB0); 

		/* 0x8 means Current Port Size 4-bits wide (TRACEDATA0-3 all set) */
		__raw_writel(0x8, SYS_TPIU_STM_BASE + 0x004); 

		/* Formatter and Flush control */
		__raw_writel(0x112, SYS_TPIU_STM_BASE + 0x304); 
		dummy_read = __raw_readl(SYS_TPIU_STM_BASE + 0x304);

		/* Formatter and Flush control */
		__raw_writel(0x162, SYS_TPIU_STM_BASE + 0x304);
		dummy_read = __raw_readl(SYS_TPIU_STM_BASE + 0x304); 
	}

#ifdef CONFIG_U2_STM_ETR_TO_SDRAM
	/*
	   EOS2 Modem STM Trace to SDRAM through ETR -- Configuration in Short
	   ===================================================================
	   SUMMARY OF MODEM STM TRACE FLOW, CONFIGURATION IN REVERSE ORDER:
	   ----------------------------------------------------------------
	   1) Modem   CoreSight / WGEM STM          
	   @ inside WGEM  - Enable traces
	   2) System  CoreSight / SYS Funnel STM    
	   @ 0xE6F 8B 000 - Enable Port #1 "From STM-ATB Modem"
	   3) System  CoreSight / SYS Trace Funnel  
	   @ 0xE6F 84 000 - Enable Port #2 "From Sys-Funnel-STM"
	   4) HostCPU CoreSight / CPU Trace Funnel  
	   @ 0xE6F A4 000 - Enable Port #4 "From Sys-Trace-Funnel"
	   5) HostCPU CoreSight / ETF               
	   @ 0xE6F A1 000 - configure FIFO mode
	   6) HostCPU CoreSight / ETR configuration 
	   @ 0xE6F A5 000 - configure Circular buffer mode, SDRAM write buffer
	   size and start address, etc.
	   7) System  CoreSight / SYS-TPIU-STM      
	   @ 0xE6F 8A 000 - set to 32-bit mode to avoid unnecessary stall
	   8) HostCPU CoreSight / CPU-TPIU          
	   @ 0xE6F A3 000 - set to 32-bit mode to avoid unnecessary stall
	   9) System  CoreSight / SYS-TPIU          
	   @ 0xE6F 83 000 - set to 32-bit mode to avoid unnecessary stall

	   DETAILED CONFIGURATION REGISTER WRITES:
	   ---------------------------------------
	*/

	/* Key register */
	__raw_writel(0x0000a501, DBGREG9); 

	/* Key register, must write twice! */
	__raw_writel(0x0000a501, DBGREG9);

	/* 9 - System CoreSight/SYS-TPIU to 32-bit mode */

	wait_for_coresight_access_lock(SYS_TPIU_BASE);
#if 1
	/* Current Port Size 4-bits wide to avoid stall */
	__raw_writel((1<<(16-1)), SYS_TPIU_BASE + 0x004);
#else
	/* Current Port Size 32-bits wide to avoid stall */
	__raw_writel((1<<(32-1)), SYS_TPIU_BASE + 0x004);
#endif

	/* 8 - HostCPU CoreSight / CPU-TPIU  to 32-bit mode  */

	wait_for_coresight_access_lock(CPU_TPIU_BASE);
#if 1
	/* Current Port Size 16-bits wide to avoid stall */
	__raw_writel((1<<(16-1)), CPU_TPIU_BASE + 0x004); 
#else
	/* Current Port Size 32-bits wide to avoid stall */
	__raw_writel((1<<(32-1)), CPU_TPIU_BASE + 0x004); 
#endif

	/* 7 - System CoreSight  / SYS-TPIU-STM to 32-bit mode */

	wait_for_coresight_access_lock(SYS_TPIU_STM_BASE);
#if 1
	/* Current Port Size 16-bits wide to avoid stall */
	__raw_writel((1<<(4-1)), SYS_TPIU_STM_BASE + 0x004);   
#else
	/* Current Port Size 32-bits wide to avoid stall */
	__raw_writel((1<<(32-1)), SYS_TPIU_STM_BASE + 0x004);
#endif

	/* 	6 HostCPU CoreSight / ETR configuration  For ARM Specification of 
		this HW block, see CoreSight Trace Memory Controller Technical Reference
		Manual SW Registers of ETR are same as ETF in different HW configuration
		*/
	wait_for_coresight_access_lock(CPU_ETR_BASE);
	__raw_writel(0, CPU_ETR_BASE + 0x020);  /* CTL Control: 0 */
	__raw_writel(0, CPU_ETR_BASE + 0x028);  /* MODE: Circular buffer */
	__raw_writel(3, CPU_ETR_BASE + 0x304);  /* FFCR: Formatting enabled */

	/*
	   (3 << 8)  WrBurstLen, 0 = 1, 1 = 2, ..., 15 = 16     
	   (0 << 7)  0 = Single buffer, 1 = ScatterGather      
	   (0 << 6)  Reserved                                  
	   (0 << 5)  CacheCtrlBit3 No write alloc / write alloc
	   (0 << 4)  CacheCtrlBit2 No read alloc / read alloc  
	   (1 << 3)  CacheCtrlBit1 Non-cacheable  / Cacheable  
	   (1 << 2)  CacheCtrlBit0 Non-bufferable / Bufferable 
	   (1 << 1)  ProtCtrlBit1  Secure / Non-secure        
	   (1 << 0)  ProtCtrlBit0  Normal / Privileged 
	   */
	i = 0x00;
	i =	((3 << 8) | (0 << 7) | (0 << 6) | (0 << 5) | (0 << 4) | (1 << 3) |
			(1 << 2) | (1 << 1) | (1 << 0));
	/* AXICTL: Set as commented above */  
	__raw_writel(i,CPU_ETR_BASE + 0x110); 

	/* BUFWM Buffer Level Water Mark: 0 */
	__raw_writel(0, CPU_ETR_BASE + 0x034); 
	/* RWP RAM Writer Pointer: 0 */
	__raw_writel(0, CPU_ETR_BASE + 0x018); 
	/* RWP RAM Writer Pointer High: 0 */
	__raw_writel(0, CPU_ETR_BASE + 0x03C);
	/* DBALO Data Buffer Address Low: 0x 4580 10000 */
	__raw_writel(0x45801000, CPU_ETR_BASE + 0x118); 
	/* DBAHI Data Buffer Address High: 0 */
	__raw_writel(0, CPU_ETR_BASE + 0x11C); 
	/* RSZ RAM Size Register: 39MB + 764 kB */
	__raw_writel(((39*1024*1024  + 764*1024)/ 4), CPU_ETR_BASE + 0x004); 
	/* CTL Control: 1 */
	__raw_writel(1, CPU_ETR_BASE + 0x020);

	/* 	5 - HostCPU CoreSight / ETF - configuration to FIFO mode  For ARM 
		specification of this HW block, see CoreSight Trace Memory Controller 
		Technical Reference Manual
		*/
	wait_for_coresight_access_lock(CPU_ETF_BASE);
	/* CTL Control: TraceCaptEn OFF ==> Disabled */
	__raw_writel(0, CPU_ETF_BASE + 0x020); 

	/* MODE: FIFO */
	__raw_writel(2, CPU_ETF_BASE + 0x028); 
	/* FFCR Formatter and Flush Control Register: Formatting enabled */
	__raw_writel(3, CPU_ETF_BASE + 0x304); 
	/* BUFWM Buffer Level Water Mark: 0 */
	__raw_writel(0, CPU_ETF_BASE + 0x034);
	/* CTL Control: TraceCaptEn ON ==> Running */
	__raw_writel(1, CPU_ETF_BASE + 0x020);
	/* 4 HostCPU CoreSight / CPU Trace Funnel - Enable Port #3 
	   "From Sys-Trace-Funnel" */

	wait_for_coresight_access_lock(CPU_TRACE_FUNNEL_BASE);
	/* Enable only Slave port 4, i.e. From Sys-Trace-Funnel */
	__raw_writel((0x300 | (1<<4)), CPU_TRACE_FUNNEL_BASE + 0x000); 

	/* 	3 - System CoreSight / SYS Trace Funnel - Enable Port #2 
		"From Sys-Funnel-STM" 
		*/
	wait_for_coresight_access_lock(SYS_TRACE_FUNNEL_BASE);
	/* Enable only Slave port 2, i.e. From Sys-Funnel-STM */
	__raw_writel((0x300 | (1<<2)), SYS_TRACE_FUNNEL_BASE + 0x000);  

	/* 	2 - System CoreSight / SYS Funnel STM - Enable Port #1 
		"From STM-ATB Modem" 
		*/
	wait_for_coresight_access_lock(SYS_TRACE_FUNNEL_STM_BASE);
	/* Enable only Slave port 1, i.e. Modem top-level funnel for STM */
	__raw_writel((0x300 | (1<<1)), SYS_TRACE_FUNNEL_STM_BASE + 0x000);      

	/* 	1 - Modem CoreSight / WGEM STM - Enable traces This happens inside WGEM 
		L2 TCM vector boot code
		*/

#endif /* CONFIG_U2_STM_ETR_TO_SDRAM */

	/* WLAN Init and SDIO device call */
	if (1 != stm_select) {
		/* SDHI1 for WLAN */
		gpio_request(GPIO_FN_SDHID1_0, NULL);
		gpio_request(GPIO_FN_SDHID1_1, NULL);
		gpio_request(GPIO_FN_SDHID1_2, NULL);
		gpio_request(GPIO_FN_SDHID1_3, NULL);
		gpio_request(GPIO_FN_SDHICMD1, NULL);
		gpio_request(GPIO_FN_SDHICLK1, NULL);

		gpio_pull_up_port(GPIO_PORT293);
		gpio_pull_up_port(GPIO_PORT292);
		gpio_pull_up_port(GPIO_PORT291);
		gpio_pull_up_port(GPIO_PORT290);
		gpio_pull_up_port(GPIO_PORT289);
		/* move gpio request to board-renesas_wifi.c */

		/* WLAN Init API call */
#ifdef CONFIG_BRCM_UNIFIED_DHD_SUPPORT
		printk(KERN_ERR "Calling WLAN_INIT!\n");
		renesas_wlan_init();
		printk(KERN_ERR "DONE WLAN_INIT!\n");
#endif	
		/* add the SDIO device */
	}

	/* touch key Interupt */
	gpio_request(GPIO_PORT104, NULL);
	gpio_direction_input(GPIO_PORT104);

	gpio_pull_up_port(GPIO_PORT104);
	/* I2C */
	gpio_request(GPIO_FN_I2C_SCL0H, NULL);
	gpio_request(GPIO_FN_I2C_SDA0H, NULL);

	gpio_pull_off_port(GPIO_PORT84);
	gpio_pull_off_port(GPIO_PORT85);

	gpio_request(GPIO_PORT28, NULL);
	gpio_direction_input(GPIO_PORT28);

#if defined(CONFIG_MFD_D2153)
	irq_set_irq_type(irqpin2irq(28), IRQ_TYPE_LEVEL_LOW);
#endif /* CONFIG_MFD_D2153 */


	/* Touch */
	gpio_request(GPIO_PORT32, NULL);
	gpio_direction_input(GPIO_PORT32);
	gpio_pull_up_port(GPIO_PORT32);

	USBGpio_init();

	/* enable sound */
	gpio_request(GPIO_FN_FSIAISLD, "sound");
	gpio_request(GPIO_FN_FSIAOBT, "sound");
	gpio_request(GPIO_FN_FSIAOLR, "sound");
	gpio_request(GPIO_FN_FSIAOSLD, "sound");

	gpio_request(GPIO_FN_FSIBISLD, "sound");
	gpio_request(GPIO_FN_FSIBOBT, "sound");
	gpio_request(GPIO_FN_FSIBOLR, "sound");
	gpio_request(GPIO_FN_FSIBOSLD, "sound");

	gpio_request(GPIO_PORT24, NULL);
	gpio_direction_input(GPIO_PORT24);
	gpio_pull_down_port(GPIO_PORT24);

#ifndef CONFIG_ARM_TZ
	r8a7373_l2cache_init();
#endif

	camera_init(u2_board_rev);
	gpio_key_init(stm_select,u2_board_rev,sec_rlte_hw_rev,
			gardalte_devices_stm_sdhi0_d2153,
			ARRAY_SIZE(gardalte_devices_stm_sdhi0_d2153),
			gardalte_devices_stm_sdhi1_d2153,
			ARRAY_SIZE(gardalte_devices_stm_sdhi1_d2153),
			gardalte_devices_stm_none_d2153,
			ARRAY_SIZE(gardalte_devices_stm_none_d2153));

	platform_device_register(&led_backlight_device);

#if defined(CONFIG_SAMSUNG_MHL)
	board_mhl_init();
	board_edid_init();
#endif

	i2c_register_board_info(0, i2c0_devices_d2153, ARRAY_SIZE(i2c0_devices_d2153));

#if defined(CONFIG_SAMSUNG_SENSOR)
	board_sensor_init();
#endif

	i2c_register_board_info(3, i2c3_devices, ARRAY_SIZE(i2c3_devices));

#ifdef CONFIG_NFC_PN547
	i2c_register_board_info(8, PN547_info, pn547_info_size()); 
#endif

#ifdef CONFIG_BOARD_VERSION_GARDA
	/* Touch Panel auto detection */
	i2c_add_driver(&tsp_detector_driver);
	i2c_register_board_info(4, i2c4_devices_tsp_detector, ARRAY_SIZE(i2c4_devices_tsp_detector));
	platform_device_register(&key_backlight_device);
#else /* CONFIG_BOARD_VERSION_GARDA */

	i2c_register_board_info(4, i2c4_devices_melfas, ARRAY_SIZE(i2c4_devices_melfas));
#endif /* CONFIG_BOARD_VERSION_GARDA */

	//i2c_register_board_info(6, i2cm_devices_d2153, ARRAY_SIZE(i2cm_devices_d2153));
	i2c_register_board_info(8, i2cm_devices_d2153, ARRAY_SIZE(i2cm_devices_d2153));

#if defined(CONFIG_GPS_CSR_GSD5T)
	/* GPS Init */
	gps_gpio_init();
#endif

	platform_add_devices(gpio_i2c_devices, ARRAY_SIZE(gpio_i2c_devices));	
	platform_add_devices(guardian__plat_devices, ARRAY_SIZE(guardian__plat_devices));

	/* PA devices init */
	PA_devices_init();

	printk(KERN_DEBUG "%s\n", __func__);
	crashlog_r_local_ver_write(mmcoops_info.soft_version);
	crashlog_reset_log_write();
	crashlog_init_tmplog();
}

static void __init gardalte_timer_init(void)
{
	r8a7373_clock_init();
	shmobile_timer.init();
#ifdef ARCH_HAS_READ_CURRENT_TIMER
	if (!setup_current_timer())
		set_delay_fn(read_current_timer_delay_loop);
#endif
}

struct sys_timer gardalte_timer = {
	.init	= gardalte_timer_init,
};

static void __init gardalte_reserve(void)
{
	u2evm_ion_adjust();

#if defined(CONFIG_SEC_DEBUG)
	sec_debug_magic_init();
#endif
}

static int check_sec_rlte_hw_rev(void)
{
	int rev0, rev1, rev2, rev3;
	int tmp_board_rev;

	gpio_request(GPIO_PORT72, "HW_REV0");
	gpio_request(GPIO_PORT73, "HW_REV1");
	gpio_request(GPIO_PORT74, "HW_REV2");
	gpio_request(GPIO_PORT75, "HW_REV3");

	gpio_direction_input(GPIO_PORT72);
	gpio_direction_input(GPIO_PORT73);
	gpio_direction_input(GPIO_PORT74);
	gpio_direction_input(GPIO_PORT75);

	rev0 = gpio_get_value(GPIO_PORT72);
	rev1 = gpio_get_value(GPIO_PORT73);
	rev2 = gpio_get_value(GPIO_PORT74);
	rev3 = gpio_get_value(GPIO_PORT75);
	/* This line is commented as Temporary as a workaround */
	/*return (rev3 << 3 | rev2 << 2 | rev1 << 1 | rev0);*/
	tmp_board_rev = (rev3 << 3 | rev2 << 2 | rev1 << 1 | rev0);
	if (tmp_board_rev == 1)
		return RLTE_BOARD_REV_0_1;
	return tmp_board_rev;
}

MACHINE_START(U2EVM, "u2evm")
	.map_io			= r8a7373_map_io,//gardalte_map_io,
	.init_irq		= r8a7373_init_irq,//gardalte_init_irq,
        .init_early             = r8a7373_init_early,
        .nr_irqs                = NR_IRQS_LEGACY,
	.handle_irq		= gic_handle_irq,
	.init_machine		= gardalte_init,
	.timer			= &shmobile_timer,//&gardalte_timer,
	.restart		= gardalte_restart,
	.reserve		= gardalte_reserve,
MACHINE_END
