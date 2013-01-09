#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/hwspinlock.h>
#include <mach/common.h>
#include <mach/hardware.h>
#include <mach/setup-u2usb.h>
#include <asm/hardware/cache-l2x0.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/time.h>
#include <linux/smsc911x.h>
#include <linux/mmc/host.h>
#include <linux/mmc/sh_mmcif.h>
#include <video/sh_mobile_lcdc.h>
#include <linux/platform_data/leds-renesas-tpu.h>
#include <mach/board-u2evm.h>
#ifdef CONFIG_PMIC_INTERFACE
#include <linux/pmic/pmic-tps80032.h>
#endif
#ifdef CONFIG_MFD_D2153
#include <linux/d2153/core.h>
#include <linux/d2153/pmic.h>
#include <linux/d2153/d2153_battery.h>
#endif
#ifdef CONFIG_PMIC_INTERFACE
#include <linux/mfd/tps80031.h>
#include <linux/pmic/pmic.h>
#include <mach/setup-u2tps80032.h>
#include <linux/regulator/tps80031-regulator.h>
#endif
#include <linux/spi/sh_msiof.h>
#include <linux/i2c/atmel_mxt_ts.h>
#include <linux/usb/r8a66597.h>
#include <linux/ion.h>
#include <linux/memblock.h>
#include <sound/sh_fsi.h>
#include <linux/tpu_pwm.h>
#include <linux/tpu_pwm_board.h>
#include <linux/pcm2pwm.h>
#include <linux/vibrator.h>
#include <linux/thermal_sensor/ths_kernel.h>
#include <media/sh_mobile_rcu.h>
#include <media/soc_camera.h>
#include <media/soc_camera_platform.h>
#include <media/sh_mobile_csi2.h>
#include <linux/sh_clk.h>
#include <media/v4l2-subdev.h>
#include "board-renesas_wifi.h"
#include <linux/pmic/pmic-ncp6914.h>
#ifdef CONFIG_SOC_CAMERA_ISX012
#include <media/isx012.h>
#endif
#include <linux/sysfs.h>
#include <linux/proc_fs.h>
#if defined(CONFIG_USB_SWITCH_TSU6712)
#include <linux/tsu6712.h>
#endif
#ifdef CONFIG_MPU_SENSORS_MPU6050B1
#include <linux/mpu.h>
#endif
#ifdef CONFIG_INPUT_YAS_SENSORS
#include <linux/yas.h>
#endif
#ifdef CONFIG_YAS_ACC_MULTI_SUPPORT
#include <linux/yas_accel.h>
#endif
#ifdef CONFIG_OPTICAL_GP2AP020A00F
#include <linux/i2c/gp2ap020.h>
#endif
#ifdef CONFIG_OPTICAL_TAOS_TRITON
#include <linux/i2c/taos.h>
#endif
#include <mach/setup-u2touchkey.h>
#include <mach/setup-u2mxt224.h>


#include <linux/mmcoops.h>	/*crashlog.h is also included with this*/
#include <asm/io.h>
#if defined(CONFIG_RENESAS_BT)
#include <mach/board-u2evm-renesas-bt.h>
#endif
#if defined(CONFIG_RENESAS_NFC)
#ifdef CONFIG_PN544_NFC
#include <linux/i2c-gpio.h>
#include <linux/nfc/pn544.h>
#endif
#endif
#if defined(CONFIG_SAMSUNG_MHL)
#include "board_mhl_sii8332.c"
#include <linux/sii8332_platform.h>
#endif
#ifdef CONFIG_USB_OTG
#include <linux/usb/tusb1211.h>
#endif
#if defined(CONFIG_VIBRATOR_ISA1000A)
#include <mach/setup-u2vibrator.h>
#endif
#if defined(CONFIG_SEC_CHARGING_FEATURE)
#include <mach/setup-u2smb328a.h>
#endif
#ifdef ARCH_HAS_READ_CURRENT_TIMER
#include <mach/setup-u2current_timer.h>
#endif
#include <mach/setup-u2sdhi.h>
#include <mach/board-u2evm.h>
#include <mach/setup-u2gpio_key.h>
static int check_sec_rlte_hw_rev(void);
#if defined(CONFIG_SEC_DEBUG_INFORM_IOTABLE)
#include <mach/sec_debug.h>
#include <mach/sec_debug_inform.h>
#endif

#include <sound/a2220.h>
#include <linux/leds-ktd253ehd.h>

#if defined(CONFIG_MPU_SENSORS_MPU6050B1)
static void mpu_power_on(int onoff);
#endif

#ifdef CONFIG_INPUT_YAS_SENSORS
static void yas_power_on(int onoff);
#endif
#if defined(CONFIG_OPTICAL_GP2AP020A00F)
static void gp2a_power_on(int onoff);
#endif
#if defined(CONFIG_MPU_SENSORS_MPU6050B1) || \
	defined(CONFIG_OPTICAL_TAOS_TRITON) || \
	defined(CONFIG_OPTICAL_GP2AP020A00F) || \
	defined(CONFIG_INPUT_YAS_SENSORS)
enum {
	SNS_PWR_OFF,
	SNS_PWR_ON,
	SNS_PWR_KEEP
};
#endif



#if defined(CONFIG_MPU_SENSORS_MPU6050B1) || \
	defined(CONFIG_OPTICAL_TAOS_TRITON) || \
	defined(CONFIG_OPTICAL_GP2AP020A00F) || \
	defined(CONFIG_INPUT_YAS_SENSORS)
static void sensor_power_on_vdd(int);
#endif





#define ENT_TPS80031_IRQ_BASE	(IRQPIN_IRQ_BASE + 64)
#define ENT_TPS80032_IRQ_BASE	(IRQPIN_IRQ_BASE + 64)

static DEFINE_SPINLOCK(io_lock);//for modify register

#if defined(CONFIG_RENESAS_GPS)
struct class *gps_class;
#endif

/*===================*/
/*  modify register  */
/*===================*/
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

/*===================*/
/*  get board rev */
/*===================*/
unsigned int u2_board_rev;
unsigned int u2_get_board_rev()
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


static struct resource smsc9220_resources[] = {
	{
		.start	= 0x00080000,
		.end	= 0x00080000 + SZ_64K - 1,
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


void (*shmobile_arch_reset)(char mode, const char *cmd);

/* MMCIF */
static struct sh_mmcif_dma sh_mmcif_dma = {
	.chan_priv_rx	= {
		.slave_id	= SHDMA_SLAVE_MMCIF0_RX,
	},
	.chan_priv_tx	= {
		.slave_id	= SHDMA_SLAVE_MMCIF0_TX,
	},
};


#if defined(CONFIG_MFD_D2153)
static struct regulator *emmc_regulator;

void d2153_mmcif_pwr_control(int onoff)
{
	int ret;

	printk(KERN_EMERG "%s %s\n", __func__, (onoff) ? "on" : "off");
	if(emmc_regulator == NULL)
	{
		printk(" %s, %d \n", __func__, __LINE__ );			
		emmc_regulator = regulator_get(NULL, "vmmc"); 
		if(IS_ERR(emmc_regulator)){
			printk("can not get vmmc regulator\n");
			return;
		}
	}

	if(onoff==1)
	{
		printk(" %s, %d vmmc On\n", __func__, __LINE__ );	

		//if(!regulator_is_enabled(touch_regulator))
		{
			printk(" %s, %d \n", __func__, __LINE__ );	
			//ret = regulator_set_voltage(emmc_regulator,3300000,3300000);
			//printk("regulator_set_voltage ret = %d \n", ret);       			
			ret = regulator_enable(emmc_regulator);
			printk("regulator_enable ret = %d \n", ret);       			
		}
	}
	else
	{
		printk("%s, %d vmmc Off\n", __func__, __LINE__ );	

		//if(regulator_is_enabled(touch_regulator))
		{
			ret = regulator_disable(emmc_regulator);
			printk("regulator_disable ret = %d \n", ret);			
		}
	}

}
#endif


static void mmcif_set_pwr(struct platform_device *pdev, int state)
{
	if (u2_get_board_rev() >= 5) {
#if defined(CONFIG_MFD_D2153)
		d2153_mmcif_pwr_control(1);
#endif /* CONFIG_MFD_D2153 */
	}else{
#if defined(CONFIG_PMIC_INTERFACE)
		gpio_set_value(GPIO_PORT227, 1);
#endif /* CONFIG_PMIC_INTERFACE */
	}
}

static void mmcif_down_pwr(struct platform_device *pdev)
{
	if (u2_get_board_rev() >= 5) {
#if defined(CONFIG_MFD_D2153)
		d2153_mmcif_pwr_control(0);
#endif /* CONFIG_MFD_D2153 */
	}else{
#if defined(CONFIG_PMIC_INTERFACE)
		gpio_set_value(GPIO_PORT227, 0);
#endif /* CONFIG_PMIC_INTERFACE */
	}
}


static struct sh_mmcif_plat_data sh_mmcif_plat = {
	.sup_pclk	= 0,
	.ocr		= MMC_VDD_165_195 | MMC_VDD_32_33 | MMC_VDD_33_34,
	.caps		= MMC_CAP_4_BIT_DATA | MMC_CAP_8_BIT_DATA |
			  MMC_CAP_1_8V_DDR | MMC_CAP_UHS_DDR50 |
			  MMC_CAP_NONREMOVABLE,
	.set_pwr	= mmcif_set_pwr,
	.down_pwr	= mmcif_down_pwr,
	.dma		= &sh_mmcif_dma,
	.max_clk	= 26000000,
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

static struct mmcoops_platform_data mmcoops_info = {
	.pdev		= &sh_mmcif_device,
	.record_size	= MMCOOPS_RECORD_SIZE,
	.kmsg_size	= MMCOOPS_KMSG_SIZE,
	.logcat_main_size	= MMCOOPS_LOGCAT_MAIN_SIZE,
	.logcat_system_size	= MMCOOPS_LOGCAT_SYSTEM_SIZE,
	.logcat_radio_size	= MMCOOPS_LOGCAT_RADIO_SIZE,
	.logcat_events_size	= MMCOOPS_LOGCAT_EVENTS_SIZE,
	.kmsg_size_ddr			= MMCOOPS_KMSG_SIZE_DDR,
	.logcat_main_size_ddr	= MMCOOPS_LOGCAT_MAIN_SIZE_DDR,
	.logcat_system_size_ddr	= MMCOOPS_LOGCAT_SYSTEM_SIZE_DDR,
	.logcat_radio_size_ddr	= MMCOOPS_LOGCAT_RADIO_SIZE_DDR,
	.logcat_events_size_ddr	= MMCOOPS_LOGCAT_EVENTS_SIZE_DDR,
	.local_version	= MMCOOPS_LOCAL_VERSION,
	.soft_version	= RMC_LOCAL_VERSION,
	/*512 byte blocks */
	.start		= MMCOOPS_START_OFFSET,
	.start_ddr	= MMCOOPS_START_OFFSET_DDR,
	.size		= MMCOOPS_LOG_SIZE
};

static struct platform_device mmcoops_device = {
	.name   = "mmcoops",
	.dev    = {
		.platform_data  = &mmcoops_info,
	},
};

static struct sh_fsi_platform_info fsi_info = {
	.port_flags = SH_FSI_OUT_SLAVE_MODE |
		SH_FSI_IN_SLAVE_MODE	|
		SH_FSI_BRS_INV		|
		SH_FSI_OFMT(I2S)	|
		SH_FSI_IFMT(I2S),
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
	.port_flags = SH_FSI_BRM_INV		|
		       SH_FSI_LRM_INV		|
		       SH_FSI_OFMT(I2S)	|
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
	.name		= "sh_fsi2",
	.id		= 1,
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

static struct ktd253ehd_led_platform_data ktd253ehd_led_info = {
	.gpio_port = GPIO_PORT47,
};

static struct platform_device led_backlight_device = {
	.name		= "ktd253ehd_led",
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
	.id                     = 0,
	.resource       = mfis_resources,
	.num_resources  = ARRAY_SIZE(mfis_resources),
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
			.port_fn		= GPIO_FN_TPUTO0,/*Func 3*/
			.pull			= PORTn_CR_PULL_DOWN,
			.direction		= PORTn_CR_DIRECTION_NOT_SET,
			.output_level	= PORTn_OUTPUT_LEVEL_NOT_SET,
		},
		/* GPIO settings to be set at suspend state */
		.inactive = {
			.port_fn		= GPIO_PORT36, /*Func 0*/
			.pull			= PORTn_CR_PULL_OFF,
			.direction		= PORTn_CR_DIRECTION_OUTPUT,
			.output_level	= PORTn_OUTPUT_LEVEL_LOW,
		}
	},
};

static struct port_info
	tpu_pwm_pfc[TPU_MODULE_MAX][TPU_CHANNEL_MAX] = {
	[TPU_MODULE_0] = {
		[TPU_CHANNEL_0]	= {
			.port_func	= GPIO_FN_TPUTO0,
			.func_name	= "pwm-tpu0to0",
			.port_count = ARRAY_SIZE(tpu0_gpio_setting_info),
			.tpu_gpio_setting_info	= &tpu0_gpio_setting_info,
		},
		[TPU_CHANNEL_1]	= {
			.port_func	= GPIO_FN_TPUTO1,
			.func_name	= "pwm-tpu0to1",
			.port_count = 0,
			.tpu_gpio_setting_info	= NULL,
		},
		[TPU_CHANNEL_2]	= {
			.port_func	= GPIO_FN_TPUTO2,
			.func_name	= "pwm-tpu0to2",
			.port_count = 0,
			.tpu_gpio_setting_info	= NULL,
		},
		[TPU_CHANNEL_3]	= {
			.port_func	= GPIO_FN_TPUTO3,
			.func_name	= "pwm-tpu0to3",
			.port_count = 0,
			.tpu_gpio_setting_info	= NULL,
		},
	},
};

static struct platform_device	tpu_devices[] = {
	{
		.name	= "tpu-renesas-sh_mobile",
		.id		= TPU_MODULE_0,
		.dev	= {
			.platform_data = &tpu_pwm_pfc[TPU_MODULE_0],
		},
		.num_resources	= 1,
		.resource		= &tpu_resources[TPU_MODULE_0],
	},
};

static struct vibrator_port_info vibrator_platdata = {
	.pcm2pwm_port	= GPIO_PORT228 ,
	.vibrator_port	= GPIO_PORT226 ,
	.tpu_port	= GPIO_PORT36 ,
};

static struct platform_device vibrator_device = {
        .name           = "vibrator-renesas-sh_mobile",
        .id             = -1,
        .dev            = {
                .platform_data  = &vibrator_platdata,
        },       
};

/* PCM2PWM */
static struct pcm2pwm_port_info pcm2pwm_platdata = {
	.port_func	= GPIO_FN_PWMO,
	.func_name	= "PWMO",
};

static struct resource pcm2pwm_resource = {
	.name	= "pcm2pwm_map",
	.start	= 0xEC380000,
	.end	= 0xEC380090,
	.flags	= IORESOURCE_MEM,
};
static struct platform_device pcm2pwm_device = {
	.name			= "pcm2pwm-renesas-sh_mobile",

	.id				= 1,
	.dev	= {
		.platform_data = &pcm2pwm_platdata,
	},
	.num_resources 	= 1,
	.resource		= &pcm2pwm_resource,
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

#define ION_HEAP_CAMERA_SIZE	(SZ_16M + SZ_2M)
#define ION_HEAP_CAMERA_ADDR	0x46600000
#define ION_HEAP_GPU_SIZE	SZ_4M
#define ION_HEAP_GPU_ADDR	0x48400000
#ifdef CONFIG_ION_R_MOBILE_USE_VIDEO_HEAP
#define ION_HEAP_VIDEO_SIZE	(SZ_16M + SZ_2M)
#define ION_HEAP_VIDEO_ADDR	0x4AE00000
#endif

static struct ion_platform_data u2evm_ion_data = {
#ifdef CONFIG_ION_R_MOBILE_USE_VIDEO_HEAP
	.nr = 5,
#else
	.nr = 4,
#endif
	.heaps = {
		{
			.type = ION_HEAP_TYPE_SYSTEM,
			.id = ION_HEAP_SYSTEM_ID,
			.name = "system",
		},
		{
			.type = ION_HEAP_TYPE_SYSTEM_CONTIG,
			.id = ION_HEAP_SYSTEM_CONTIG_ID,
			.name = "system-contig",
		},
		{
			.type = ION_HEAP_TYPE_CARVEOUT,
			.id = ION_HEAP_CAMERA_ID,
			.name = "camera",
			.base = ION_HEAP_CAMERA_ADDR,
			.size = ION_HEAP_CAMERA_SIZE,
		},
		{
			.type = ION_HEAP_TYPE_CARVEOUT,
			.id = ION_HEAP_GPU_ID,
			.name = "gpu",
			.base = ION_HEAP_GPU_ADDR,
			.size = ION_HEAP_GPU_SIZE,
		},
#ifdef CONFIG_ION_R_MOBILE_USE_VIDEO_HEAP
		{
			.type = ION_HEAP_TYPE_CARVEOUT,
			.id = ION_HEAP_VIDEO_ID,
			.name = "video",
			.base = ION_HEAP_VIDEO_ADDR,
			.size = ION_HEAP_VIDEO_SIZE,
		},
#endif
	},
};

static struct platform_device u2evm_ion_device = {
	.name = "ion-r-mobile",
	.id = -1,
	.dev = {
		.platform_data = &u2evm_ion_data,
	},
};
/* I2C */
#ifdef CONFIG_PMIC_INTERFACE
static struct tps80031_32kclock_plat_data tps_clk = {
        /*All 32k always on*/
        .en_clk32kao = 1,
        .en_clk32kg = 1,
        .en_clk32kaudio = 1,
};
#endif /* CONFIG_PMIC_INTERFACE */
#if defined(CONFIG_MPU_SENSORS_MPU6050B1) || defined(CONFIG_INPUT_YAS_SENSORS) || \
	defined(CONFIG_OPTICAL_TAOS_TRITON) || defined(CONFIG_OPTICAL_GP2AP020A00F)

#if defined (CONFIG_MACH_U2EVM_SR_REV021) || defined (CONFIG_MACH_U2EVM_SR_REV022)
static int gpiosensor = -1;
#else
static struct regulator *vsensor_3V;
#endif

static void sensor_power_on_vdd(int onoff)
{
       pr_err("%s: start ",__func__);

#if defined (CONFIG_MACH_U2EVM_SR_REV021) || defined (CONFIG_MACH_U2EVM_SR_REV022)

      if(gpiosensor < 0) {
          gpiosensor = gpio_request(GPIO_PORT9, "SENSOR_LDO");
	    if (gpiosensor < 0)
          {
		pr_err("SENSOR_LDO_EN gpio_request was failed\n");
             return;
           }
	gpio_pull_off_port(GPIO_PORT9);
      }

	if (onoff == SNS_PWR_ON) {
            gpio_direction_output(GPIO_PORT9,1);
            pr_err("%s: power on ",__func__);
	} else if ((onoff == SNS_PWR_OFF)) {
            //gpio_direction_output(GPIO_PORT9,0); // TEMP  fix for "inv_i2c_read error"
            pr_err("%s: power off ",__func__);
	}
#else
	int ret;

       pr_err("%s: start ",__func__);

	if (!vsensor_3V) {
		if (u2_get_board_rev() >= 5) {
#if defined(CONFIG_MFD_D2153)
			vsensor_3V = regulator_get(NULL, "sensor_3v");
#endif /* CONFIG_MFD_D2153 */
		} else {
#if defined(CONFIG_PMIC_INTERFACE)
			vsensor_3V = regulator_get(NULL, "vdd_touch");
#endif /* CONFIG_PMIC_INTERFACE */
		}
		pr_err("%s: regulator_get ",__func__);
		if (IS_ERR(vsensor_3V))
			return ;

		ret = regulator_set_voltage(vsensor_3V, 3000000, 3000000);
		if (ret)
			pr_err("%s: error vsensor_3V setting voltage ret=%d\n",__func__, ret);
	}

	if (onoff == SNS_PWR_ON) {
		ret = regulator_enable(vsensor_3V);
		pr_err("%s: regulator_enable ",__func__);
		if (ret)
			pr_err("%s: error enabling regulator\n", __func__);
	} else if ((onoff == SNS_PWR_OFF)) {
		if (regulator_is_enabled(vsensor_3V)) {
			ret = regulator_disable(vsensor_3V);
			if (ret)
				pr_err("%s: error vsensor_3V enabling regulator\n",__func__);
		}
	}
#endif
}
#endif


#if defined(CONFIG_MPU_SENSORS_MPU6050B1)
static void mpu_power_on(int onoff)
{
	sensor_power_on_vdd(onoff);
}
#endif

#ifdef CONFIG_INPUT_YAS_SENSORS
static void yas_power_on(int onoff)
{
	sensor_power_on_vdd(onoff);
}
#endif

#if defined(CONFIG_OPTICAL_GP2AP020A00F)
static void gp2a_power_on(int onoff)
{
	sensor_power_on_vdd(onoff);
}
#endif

#if defined(CONFIG_OPTICAL_TAOS_TRITON)
static void taos_power_on(int onoff)
{
	sensor_power_on_vdd(onoff);
}
#endif

#if defined(CONFIG_OPTICAL_GP2A) || defined(CONFIG_OPTICAL_GP2AP020A00F)
static void gp2a_led_onoff(int onoff)
{

}
#endif

#if defined(CONFIG_OPTICAL_TAOS_TRITON)
static void taos_led_onoff(int onoff)
{

}
#endif

#ifdef CONFIG_MPU_SENSORS_MPU6050B1
struct mpu_platform_data mpu6050_data = {
	.int_config = 0x10,
	.orientation = {-1, 0, 0,
			0, -1, 0,
			0, 0, 1},
	.poweron = mpu_power_on,
	};
	/* compass */
static struct ext_slave_platform_data inv_mpu_yas530_data = {
	.bus            = EXT_SLAVE_BUS_PRIMARY,
	.orientation = {1, 0, 0,
			0, 1, 0,
			0, 0, 1},
	};
#endif


#if defined(CONFIG_OPTICAL_GP2AP020A00F)
#define GPIO_ALS_INT	 108
static struct gp2a_platform_data opt_gp2a_data = {
	.gp2a_led_on	= gp2a_led_onoff,
	.power_on = gp2a_power_on,
	.p_out = GPIO_ALS_INT,
	.addr = 0x72>>1,
	.version = 0,
};

static struct platform_device opt_gp2a = {
	.name = "gp2a-opt",
	.id = -1,
	.dev        = {
		.platform_data  = &opt_gp2a_data,
	},
};
#endif

#if defined(CONFIG_OPTICAL_TAOS_TRITON)
#define GPIO_ALS_INT	 108
static void taos_power_on(int onoff);
static void taos_led_onoff(int onoff);

static struct taos_platform_data taos_pdata = {
	.power	= taos_power_on,
	.led_on	=	taos_led_onoff,
	.als_int = GPIO_ALS_INT,
	.prox_thresh_hi = 650,
	.prox_thresh_low = 510,
	.als_time = 0xED,
	.intr_filter = 0x33,
	.prox_pulsecnt = 0x08,
	.prox_gain = 0x28,
	.coef_atime = 50,
	.ga = 117,
	.coef_a = 1000,
	.coef_b = 1600,
	.coef_c = 660,
	.coef_d = 1250,
};
#endif

#if defined(CONFIG_INPUT_YAS_SENSORS)
static struct platform_device yas532_orient_device = {
	.name			= "orientation",
};
#endif


static struct i2c_board_info __initdata i2c2_devices[] = {
#ifdef CONFIG_MPU_SENSORS_MPU6050B1
	{
		I2C_BOARD_INFO("mpu6050", 0x68),
		.irq = irqpin2irq(46),
		.platform_data = &mpu6050_data,
	 },
#endif
#ifdef CONFIG_INPUT_YAS_SENSORS
	{
#ifdef CONFIG_YAS_ACC_MULTI_SUPPORT
		I2C_BOARD_INFO("accelerometer", 0x18),
		.platform_data = &accel_pdata,
#else
		I2C_BOARD_INFO("accelerometer", 0x19),
#endif
	},
	{
		I2C_BOARD_INFO("geomagnetic", 0x2e),
	},
#endif
#if defined(CONFIG_OPTICAL_GP2AP020A00F)
	{
		I2C_BOARD_INFO("gp2a", 0x72>>1),
	},
#elif defined(CONFIG_OPTICAL_TAOS_TRITON)
	{
		I2C_BOARD_INFO("taos", 0x39),
		.platform_data = &taos_pdata,
	},
#endif
};

#if defined(CONFIG_BATTERY_BQ27425)
#define BQ27425_ADDRESS (0xAA >> 1)
#define GPIO_FG_INT 44
#endif

#if defined(CONFIG_CHARGER_SMB328A)
#define SMB328A_ADDRESS (0x69 >> 1)
#define GPIO_CHG_INT 19
#endif

#if defined(CONFIG_RENESAS_GPS)

static ssize_t GNSS_NRST_value_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	int value = gpio_get_value(GPIO_PORT10);

	return sprintf(buf, "%d\n", value);
}

#ifdef CONFIG_MFD_D2153
extern void d2153_clk32k_enable(int onoff);
#endif /* CONFIG_MFD_D2153 */

static ssize_t GNSS_NRST_value_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	long value;
	int  ret;

	ret = strict_strtol(buf, 0, &value);
	if (ret < 0)
		return ret;

	if (u2_get_board_rev() >= 5) {
#ifdef CONFIG_MFD_D2153
		if (1 == value)
			d2153_clk32k_enable(1);		// on
		else
			d2153_clk32k_enable(0);		// off
#endif /* CONFIG_MFD_D2153 */
	} else {
#if defined(CONFIG_PMIC_INTERFACE)
		if (1 == value)
			pmic_clk32k_enable(CLK32KG, TPS80032_STATE_ON);
		else
			pmic_clk32k_enable(CLK32KG, TPS80032_STATE_OFF);
#endif /* CONFIG_PMIC_INTERFACE */
	}

	printk(KERN_ALERT "%s: %d\n", __func__, value);

	gpio_set_value(GPIO_PORT10, value);

	return count;
}

DEVICE_ATTR(value_nrst, 0644, GNSS_NRST_value_show, GNSS_NRST_value_store);

static ssize_t GNSS_EN_value_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	int value = gpio_get_value(GPIO_PORT11);

	return sprintf(buf, "%d\n", value);
}

static ssize_t GNSS_EN_value_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	long value;
	int  ret;

	ret = strict_strtol(buf, 0, &value);
	if (ret < 0)
		return ret;

	printk(KERN_ALERT "%s: %d\n", __func__, value);

	gpio_set_value(GPIO_PORT11, value);

	return count;
}

DEVICE_ATTR(value_en, 0644, GNSS_EN_value_show, GNSS_EN_value_store);

static const struct attribute *GNSS_NRST_attrs[] = {
	&dev_attr_value_nrst.attr,
	NULL,
};

static const struct attribute_group GNSS_NRST_attr_group = {
	.attrs = (struct attribute **) GNSS_NRST_attrs,
};

static const struct attribute *GNSS_EN_attrs[] = {
	&dev_attr_value_en.attr,
	NULL,
};

static const struct attribute_group GNSS_EN_attr_group = {
	.attrs = (struct attribute **) GNSS_EN_attrs,
};

static void gps_gpio_init(void)
{
	struct device *gps_dev;
	struct device *gnss_nrst_dev;
	struct device *gnss_en_dev;
	int    status = -EINVAL;

	gps_class = class_create(THIS_MODULE, "gps");
	if (IS_ERR(gps_class)) {
		pr_err("Failed to create class(sec)!\n");
		return PTR_ERR(gps_class);
	}
	BUG_ON(!gps_class);

	gps_dev = device_create(gps_class, NULL, 0, NULL, "device_gps");
	BUG_ON(!gps_dev);

	gnss_nrst_dev = device_create(gps_class, gps_dev, 0, NULL, "GNSS_NRST");
	BUG_ON(!gnss_nrst_dev);

	gnss_en_dev = device_create(gps_class, gps_dev, 0, NULL, "GNSS_EN");
	BUG_ON(!gnss_en_dev);

	status = sysfs_create_group(&gnss_nrst_dev->kobj,
				    &GNSS_NRST_attr_group);

	if (status)
		pr_debug("%s: status for sysfs_create_group %d\n",
			 __func__, status);

	status = sysfs_create_group(&gnss_en_dev->kobj, &GNSS_EN_attr_group);

	if (status)
		pr_debug("%s: status for sysfs_create_group %d\n",
			 __func__, status);

	printk(KERN_ALERT "gps_gpio_init!!");

		gpio_request(GPIO_FN_SCIFB1_RXD, NULL);
		gpio_pull_up_port(GPIO_PORT79);
		gpio_request(GPIO_FN_SCIFB1_TXD, NULL);
		gpio_pull_off_port(GPIO_PORT78);
		gpio_request(GPIO_FN_SCIFB1_CTS, NULL);
		gpio_pull_up_port(GPIO_PORT77);
		gpio_request(GPIO_FN_SCIFB1_RTS, NULL);
		gpio_pull_off_port(GPIO_PORT76);

		/* GPS Settings */
		gpio_request(GPIO_PORT10, "GNSS_NRST");
		gpio_pull_off_port(GPIO_PORT10);
		gpio_direction_output(GPIO_PORT10, 0);

		gpio_request(GPIO_PORT11, "GNSS_EN");
		gpio_pull_off_port(GPIO_PORT11);
		gpio_direction_output(GPIO_PORT11, 0);

	printk("gps_gpio_init done!!\n");
}
#endif

#if defined(CONFIG_USB_SWITCH_TSU6712)
#define TSU6712_ADDRESS (0x4A >> 1)
#define GPIO_MUS_INT 41

static struct tsu6712_platform_data tsu6712_pdata = {
};
#endif
static struct i2c_board_info __initdata i2c3_devices[] = {
#if defined(CONFIG_BATTERY_BQ27425)
    {
                I2C_BOARD_INFO("bq27425", BQ27425_ADDRESS),
                .irq            = irqpin2irq(GPIO_FG_INT),
        },
#endif
#if defined(CONFIG_USB_SWITCH_TSU6712)
    {
		I2C_BOARD_INFO("tsu6712", TSU6712_ADDRESS),
			.platform_data = NULL,
			.irq            = irqpin2irq(GPIO_MUS_INT),
    },
#endif
#if defined(CONFIG_CHARGER_SMB328A)
    {
		I2C_BOARD_INFO("smb328a", SMB328A_ADDRESS),
/*			.platform_data = &tsu6712_pdata,*/
/*            .irq            = irqpin2irq(GPIO_CHG_INT),*/
    },
#endif
};
#if defined(CONFIG_MPU_SENSORS_MPU6050B1)
static void mpl_init(void)
{
	int rc = 0;
	rc = gpio_request(GPIO_PORT107, "MPUIRQ");
	if (rc < 0)
		pr_err("GPIO_MPU3050_INT gpio_request was failed\n");
	gpio_direction_input(GPIO_PORT107);
	gpio_pull_up_port(GPIO_PORT107);
}
#endif

/* << Add for Thermal Sensor driver*/
static struct thermal_sensor_data ths_platdata[] = {
	/* THS0 */
	{
	.current_mode		= E_NORMAL_1,	/* Normal 1 operation */
	.last_mode			= E_NORMAL_1,	/* Normal 1 operation */
	},
	
	/* THS1 */
	{
	.current_mode		= E_NORMAL_1,	/* Normal 1 operation */
	.last_mode			= E_NORMAL_1,	/* Normal 1 operation */
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
		.start	= gic_spi(73),		/* SPI# of THS is 73 */
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
/* >> End Add for Thermal Sensor driver*/

/* CAM0 Power function */
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
		gpio_set_value(GPIO_PORT90, 0); /* CAM0_STBY */
		mdelay(10);
		/* 10ms */

		if (u2_get_board_rev() >= 5) {
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

		if (u2_get_board_rev() >= 5) {
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

		//spin_lock(&bl_ctrl_lock);
		if (mode == SH_RCU_LED_MODE_PRE) // temp, torch mode
			{
			/* initailize flash IC */
			gpio_direction_output(CAM_FLASH_ENSET, 0);
			gpio_direction_output(CAM_FLASH_FLEN, 0);
			mdelay(1);
			// to enter a shutdown mode
			/* set to movie mode */
			for (i = 0; i < 3; i++) {
				udelay(1);
				gpio_direction_output(CAM_FLASH_ENSET, 1);
				udelay(1);
				gpio_direction_output(CAM_FLASH_ENSET, 0);
			}
			gpio_direction_output(CAM_FLASH_ENSET, 1);
		} else {
			// CamacqTraceErr("WINGI AAAAAAAAAAAAAAAAAAA");
			/* initailize flash IC */
			gpio_direction_output(CAM_FLASH_ENSET, 0);
			gpio_direction_output(CAM_FLASH_FLEN, 0);
			mdelay(1);
			// to enter a shutdown mode
			// FLEN high
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
		//spin_unlock(&bl_ctrl_lock);
		break;
	case SH_RCU_LED_OFF: {
		gpio_request(CAM_FLASH_ENSET, "ledflash");
		gpio_request(CAM_FLASH_FLEN, "ledflash");
		/* initailize falsh IC */
		gpio_direction_output(CAM_FLASH_ENSET, 0);
		gpio_direction_output(CAM_FLASH_FLEN, 0);
		mdelay(1);
		// to enter a shutdown mode
		gpio_free(CAM_FLASH_ENSET);
		gpio_free(CAM_FLASH_FLEN);
	}
		break;
	default:
		printk(KERN_ALERT "%s:not case %d",__func__, light);
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
		gpio_set_value(GPIO_PORT90, 0); /* CAM0_STBY */

		mdelay(10);
		/* 10ms */
		if (u2_get_board_rev() >= 5) {
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
		
#endif /* CONFIG_MFD_D2153 */
		} else {
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
		}

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

		if (u2_get_board_rev() >= 5) {
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

		if (u2_get_board_rev() >= 5) {
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
//		subPMIC_PinOnOff(0x0, 0);
//		mdelay(1);
		}
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
		gpio_set_value(GPIO_PORT90, 0); /* CAM0_STBY */
		mdelay(10);
		/* 10ms */

		if (u2_get_board_rev() >= 5) {
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
		if (u2_get_board_rev() >= 5) {
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

		if (u2_get_board_rev() >= 5) {
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
		gpio_set_value(GPIO_PORT90, 0); /* CAM0_STBY */

		mdelay(10);
		/* 10ms */

		if (u2_get_board_rev() >= 5) {
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

		if (u2_get_board_rev() >= 5) {
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

		if (u2_get_board_rev() >= 5) {
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

		gpio_set_value(GPIO_PORT3, 0); /* CAM_PWR_EN Low */
		sh_csi2_power(dev, power_on);
		printk(KERN_ALERT "%s PowerOFF fin\n", __func__);

	}

	clk_put(vclk1_clk);
	clk_put(vclk2_clk);

	return 0;
}

static struct i2c_board_info i2c_cameras[] = {
	{
		I2C_BOARD_INFO("IMX175", 0x1A),
	},
	{
		I2C_BOARD_INFO("S5K6AAFX13", 0x3C), /* 0x78(3C),0x5A(2D),0x45 */
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

static struct soc_camera_link camera_links[] = {
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

static struct platform_device camera_devices[] = {
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

static struct sh_csi2_client_config csi20_clients[] = {
	{
		.phy		= SH_CSI2_PHY_MAIN,
		.lanes		= 0xF,
		.channel	= 0,
		.pdev		= &camera_devices[0],
	},
};

static struct sh_csi2_pdata csi20_info = {
	.type		= SH_CSI2C,
	.clients	= csi20_clients,
	.num_clients	= ARRAY_SIZE(csi20_clients),
	.flags		= SH_CSI2_ECC | SH_CSI2_CRC,
	.ipr		= 0x24,
	.ipr_set	= (0x0001 << 8),
	.imcr		= 0x1D0,
	.imcr_set	= (0x01 << 2),
	.priv		= NULL,
	.cmod_name	= "csi20",
};

static struct resource csi20_resources[] = {
	[0] = {
		.name	= "CSI20",
		.start	= 0xfeaa0000,
		.end	= 0xfeaa0fff,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= intcs_evt2irq(0x17a0),
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device csi20_device = {
	.name   = "sh-mobile-csi2",
	.id     = 0,
	.num_resources	= ARRAY_SIZE(csi20_resources),
	.resource	= csi20_resources, /* ES1 is CSI21 connect */
	.dev    = {
		.platform_data = &csi20_info,
	},
};

static struct sh_csi2_client_config csi21_clients[] = {
	{
		.phy		= SH_CSI2_PHY_SUB,
		.lanes		= 1,
		.channel	= 0,
		.pdev		= &camera_devices[1],
	},
};

static struct sh_csi2_pdata csi21_info = {
	.type		= SH_CSI2C,
	.clients	= csi21_clients,
	.num_clients	= ARRAY_SIZE(csi21_clients),
	.flags		= SH_CSI2_ECC | SH_CSI2_CRC,
	.ipr		= 0x44,
	.ipr_set	= (0x0001 << 0),
	.imcr		= 0x1E0,
	.imcr_set	= (0x01 << 0),
	.priv		= NULL,
	.cmod_name	= "csi21",
};

static struct resource csi21_resources[] = {
	[0] = {
		.name	= "CSI21",
		.start	= 0xfeaa8000,
		.end	= 0xfeaa8fff,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= intcs_evt2irq(0x1be0),
		.flags  = IORESOURCE_IRQ,
	},
};

static struct resource csi21_resources_es1[] = {
	[0] = {
		.name	= "CSI21",
		.start	= 0xfeaa0000,
		.end	= 0xfeaa0fff,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= intcs_evt2irq(0x17a0),
		.flags  = IORESOURCE_IRQ,
	},
};

static struct platform_device csi21_device = {
	.name   = "sh-mobile-csi2",
	.id     = 1,
	.num_resources	= ARRAY_SIZE(csi21_resources),
	.resource	= csi21_resources,
	.dev    = {
		.platform_data = &csi21_info,
	},
};

static struct sh_mobile_rcu_info sh_mobile_rcu0_info = {
	.flags		= 0,
	.csi2_dev	= &csi20_device.dev,
	.mod_name	= "rcu0",
	.led		= main_cam_led,
};

static struct resource rcu0_resources[] = {
	[0] = {
		.name	= "RCU0",
		.start	= 0xfe910000,
		.end	= 0xfe91022b,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= gic_spi(83),
		.flags	= IORESOURCE_IRQ,
	},
	[2] = {
		/* place holder for contiguous memory */
	},
};

static struct resource rcu0_resources_es1[] = {
	[0] = {
		.name	= "RCU0",
		.start	= 0xfe910000,
		.end	= 0xfe91022b,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= intcs_evt2irq(0x1de0),
		.flags	= IORESOURCE_IRQ,
	},
	[2] = {
		/* place holder for contiguous memory */
	},
};

static struct platform_device rcu0_device = {
	.name			= "sh_mobile_rcu",
	.id				= 0, /* "rcu0" clock */
	.num_resources	= ARRAY_SIZE(rcu0_resources),
	.resource		= rcu0_resources,	/* ES1 is RCU0 connect */
	.dev = {
		.platform_data	= &sh_mobile_rcu0_info,
	},
};

static struct sh_mobile_rcu_info sh_mobile_rcu1_info = {
	.flags		= 0,
	.csi2_dev	= &csi21_device.dev,
	.mod_name	= "rcu1",
};

static struct resource rcu1_resources[] = {
	[0] = {
		.name	= "RCU1",
		.start	= 0xfe914000,
		.end	= 0xfe91422b,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= gic_spi(84),
		.flags	= IORESOURCE_IRQ,
	},
	[2] = {
		/* place holder for contiguous memory */
	},
};

static struct resource rcu1_resources_es1[] = {
	[0] = {
		.name	= "RCU1",
		.start	= 0xfe910000,
		.end	= 0xfe91022b,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= intcs_evt2irq(0x1de0),
		.flags	= IORESOURCE_IRQ,
	},
	[2] = {
		/* place holder for contiguous memory */
	},
};

static struct platform_device rcu1_device = {
	.name			= "sh_mobile_rcu",
	.id				= 1, /* "rcu1" clock */
	.num_resources	= ARRAY_SIZE(rcu1_resources),
	.resource		= rcu1_resources, /* ES1 is RCU0 connect */
	.dev	= {
		.platform_data	= &sh_mobile_rcu1_info,
	},
};

#if defined(CONFIG_RENESAS_NFC)
#ifdef CONFIG_PN544_NFC  

#define NFC_EN_GPIO         GPIO_PORT12
#define NFC_IRQ_GPIO        GPIO_PORT13
#define NFC_FIRM_GPIO       GPIO_PORT101
#define NFC_I2C_SDA_GPIO	GPIO_PORT274      
#define NFC_I2C_SCL_GPIO	GPIO_PORT273
#define NFC_I2C_BUS_ID		(8) 

static struct i2c_gpio_platform_data pn544_i2c_gpio_data = {
	.sda_pin = NFC_I2C_SDA_GPIO,
	.scl_pin =  NFC_I2C_SCL_GPIO,
 	.udelay = 1,  
};

static struct platform_device pn544_i2c_gpio_device = {
 	.name = "i2c-gpio",
 	.id = NFC_I2C_BUS_ID,
 	.dev = {
	.platform_data  = &pn544_i2c_gpio_data,
	},
};

static struct pn544_i2c_platform_data pn544_pdata = {
 	.irq_gpio 	= NFC_IRQ_GPIO,
 	.ven_gpio = NFC_EN_GPIO,
 	.firm_gpio = NFC_FIRM_GPIO,
};
 
static struct i2c_board_info pn544_info[] __initdata = {
{
	I2C_BOARD_INFO("pn544", 0x2b),
	.irq = irqpin2irq(NFC_IRQ_GPIO),
	.platform_data = &pn544_pdata,
 	},
};

#endif
#endif

static struct resource mdm_reset_resources[] = {
	[0] = {
		.name	= "MODEM_RESET",
		.start	= 0xE6190000,
		.end	= 0xE61900FF,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= gic_spi(219),  // EPMU_int1
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device mdm_reset_device = {
	.name		= "rmc_wgm_reset_int",
	.id		= 0,
	/*.dev		= {
		.platform_data	= &sdhi0_info,
	},*/
	.num_resources	= ARRAY_SIZE(mdm_reset_resources),
	.resource	= mdm_reset_resources,
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

struct a2220_platform_data  u2evm_a2220_data = {
	.a2220_hw_init = NULL,
	.gpio_reset = GPIO_PORT44,
	.gpio_wakeup = GPIO_PORT26,
};

/* THREE optional u2evm_devices pointer lists for initializing the platform devices */
/* For different STM muxing options 0, 1, or None, as given by boot_command_line parameter stm=0/1/n */

static struct platform_device *u2evm_devices_stm_sdhi1_d2153[] __initdata = {
	
	&usbhs_func_device_d2153,
#ifdef CONFIG_USB_R8A66597_HCD
	&usb_host_device,
#endif
#ifdef CONFIG_USB_OTG
	&tusb1211_device,
#endif
//	&eth_device,
#ifdef CONFIG_KEYBOARD_SH_KEYSC
	&keysc_device,
#endif
	&sh_mmcif_device,
	&mmcoops_device,
	&sdhi0_device,
//	&sdhi1_device, // STM Trace muxed over SDHI1 WLAN interface, coming from 34-pint MIPI cable to FIDO
#if defined(CONFIG_RENESAS_BT)
	&bcm4334_bluetooth_device,
#endif
	&fsi_device,
	&fsi_b_device,
	&gpio_key_device,
	&lcdc_device,
	&led_backlight_device,
	&mfis_device,
//	&tpu_devices[TPU_MODULE_0],
	&mdm_reset_device,
	&vibrator_device,
//	&pcm2pwm_device,
#ifdef CONFIG_SPI_SH_MSIOF
	&sh_msiof0_device,
#endif
// #ifdef CONFIG_ION_R_MOBILE
	&u2evm_ion_device,
// #endif
	&thermal_sensor_device,
	&csi20_device,
	&csi21_device,

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

static struct platform_device *u2evm_devices_stm_sdhi1[] __initdata = {
	
	&usbhs_func_device,
#ifdef CONFIG_USB_R8A66597_HCD
	&usb_host_device,
#endif
#ifdef CONFIG_USB_OTG
	&tusb1211_device,
#endif
//	&eth_device,
#ifdef CONFIG_KEYBOARD_SH_KEYSC
	&keysc_device,
#endif
	&sh_mmcif_device,
	&mmcoops_device,
	&sdhi0_device,
//	&sdhi1_device, // STM Trace muxed over SDHI1 WLAN interface, coming from 34-pint MIPI cable to FIDO
#if defined(CONFIG_RENESAS_BT)
	&bcm4334_bluetooth_device,
#endif
	&fsi_device,
	&fsi_b_device,
	&gpio_key_device,
	&lcdc_device,
	&led_backlight_device,
	&mfis_device,
//	&tpu_devices[TPU_MODULE_0],
	&mdm_reset_device,
	&vibrator_device,
//	&pcm2pwm_device,
#ifdef CONFIG_SPI_SH_MSIOF
	&sh_msiof0_device,
#endif
// #ifdef CONFIG_ION_R_MOBILE
	&u2evm_ion_device,
// #endif
	&thermal_sensor_device,
	&csi20_device,
	&csi21_device,

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

static struct platform_device *u2evm_devices_stm_sdhi0_d2153[] __initdata = {
	
	&usbhs_func_device_d2153,
#ifdef CONFIG_USB_R8A66597_HCD
	&usb_host_device,
#endif
#ifdef CONFIG_USB_OTG
	&tusb1211_device,
#endif
//	&eth_device,
#ifdef CONFIG_KEYBOARD_SH_KEYSC
	&keysc_device,
#endif
	&sh_mmcif_device,
	&mmcoops_device,
//	&sdhi0_device, // STM Trace muxed over SDHI0 SD-Card interface, coming by special SD-Card adapter to FIDO
	&sdhi1_device,
#if defined(CONFIG_RENESAS_BT)
	&bcm4334_bluetooth_device,
#endif
	&fsi_device,
	&fsi_b_device,
	&gpio_key_device,
	&lcdc_device,
	&led_backlight_device,
	&mfis_device,
	&tpu_devices[TPU_MODULE_0],
	&mdm_reset_device,
	&vibrator_device,
//	&pcm2pwm_device,
#ifdef CONFIG_SPI_SH_MSIOF
	&sh_msiof0_device,
#endif
// #ifdef CONFIG_ION_R_MOBILE // BUG ? Testing -- Tommi
	&u2evm_ion_device,
// #endif
	&thermal_sensor_device,
	&csi20_device,
	&csi21_device,

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
#if defined(CONFIG_OPTICAL_GP2A) ||	defined(CONFIG_OPTICAL_GP2AP020A00F)
	&opt_gp2a,
#endif
#if defined(CONFIG_INPUT_YAS_SENSORS)
	&yas532_orient_device,
#endif
};

static struct platform_device *u2evm_devices_stm_sdhi0[] __initdata = {
	
	&usbhs_func_device,
#ifdef CONFIG_USB_R8A66597_HCD
	&usb_host_device,
#endif
#ifdef CONFIG_USB_OTG
	&tusb1211_device,
#endif
//	&eth_device,
#ifdef CONFIG_KEYBOARD_SH_KEYSC
	&keysc_device,
#endif
	&sh_mmcif_device,
	&mmcoops_device,
//	&sdhi0_device, // STM Trace muxed over SDHI0 SD-Card interface, coming by special SD-Card adapter to FIDO
	&sdhi1_device,
#if defined(CONFIG_RENESAS_BT)
	&bcm4334_bluetooth_device,
#endif
	&fsi_device,
	&fsi_b_device,
	&gpio_key_device,
	&lcdc_device,
	&led_backlight_device,
	&mfis_device,
	&tpu_devices[TPU_MODULE_0],
	&mdm_reset_device,
	&vibrator_device,
//	&pcm2pwm_device,
#ifdef CONFIG_SPI_SH_MSIOF
	&sh_msiof0_device,
#endif
// #ifdef CONFIG_ION_R_MOBILE // BUG ? Testing -- Tommi
	&u2evm_ion_device,
// #endif
	&thermal_sensor_device,
	&csi20_device,
	&csi21_device,

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
#if defined(CONFIG_OPTICAL_GP2A) ||	defined(CONFIG_OPTICAL_GP2AP020A00F)
	&opt_gp2a,
#endif
#if defined(CONFIG_INPUT_YAS_SENSORS)
	&yas532_orient_device,
#endif
};

static struct platform_device *u2evm_devices_stm_none_d2153[] __initdata = {
	
	&usbhs_func_device_d2153,
#ifdef CONFIG_USB_R8A66597_HCD
	&usb_host_device,
#endif
#ifdef CONFIG_USB_OTG
	&tusb1211_device,
#endif
//	&eth_device,
#ifdef CONFIG_KEYBOARD_SH_KEYSC
	&keysc_device,
#endif
	&sh_mmcif_device,
	&mmcoops_device,
	&sdhi0_device,
	&sdhi1_device,
#if defined(CONFIG_RENESAS_BT)
	&bcm4334_bluetooth_device,
#endif
	&fsi_device,
	&fsi_b_device,
	&gpio_key_device,
	&lcdc_device,
	&led_backlight_device,
	&mfis_device,
    &tpu_devices[TPU_MODULE_0],
	&mdm_reset_device,
	&vibrator_device,
//	&pcm2pwm_device,
#ifdef CONFIG_SPI_SH_MSIOF
	&sh_msiof0_device,
#endif
	&u2evm_ion_device,
	&thermal_sensor_device,
	&csi20_device,
	&csi21_device,

	&rcu0_device,
	&rcu1_device,

	&camera_devices[0],
	&camera_devices[1],
#if defined(CONFIG_RENESAS_NFC)
#ifdef CONFIG_PN544_NFC
        &pn544_i2c_gpio_device,
#endif
#endif
};

static struct platform_device *u2evm_devices_stm_none[] __initdata = {
	
	&usbhs_func_device,
#ifdef CONFIG_USB_R8A66597_HCD
	&usb_host_device,
#endif
#ifdef CONFIG_USB_OTG
	&tusb1211_device,
#endif
//	&eth_device,
#ifdef CONFIG_KEYBOARD_SH_KEYSC
	&keysc_device,
#endif
	&sh_mmcif_device,
	&mmcoops_device,
	&sdhi0_device,
	&sdhi1_device,
#if defined(CONFIG_RENESAS_BT)
	&bcm4334_bluetooth_device,
#endif
	&fsi_device,
	&fsi_b_device,
	&gpio_key_device,
	&lcdc_device,
	&led_backlight_device,
	&mfis_device,
    &tpu_devices[TPU_MODULE_0],
	&mdm_reset_device,
	&vibrator_device,
//	&pcm2pwm_device,
#ifdef CONFIG_SPI_SH_MSIOF
	&sh_msiof0_device,
#endif
	&u2evm_ion_device,
	&thermal_sensor_device,
	&csi20_device,
	&csi21_device,

	&rcu0_device,
	&rcu1_device,

	&camera_devices[0],
	&camera_devices[1],
#if defined(CONFIG_RENESAS_NFC)
#ifdef CONFIG_PN544_NFC
        &pn544_i2c_gpio_device,
#endif
#endif
};

/* I2C */

#if defined(CONFIG_MFD_D2153)

#define mV_to_uV(v)                 ((v) * 1000)
#define uV_to_mV(v)                 ((v) / 1000)
#define MAX_MILLI_VOLT              (3300)


/* D2153 DC-DCs */
// BUCK1
static struct regulator_consumer_supply d2153_buck1_supplies[] = {
	REGULATOR_SUPPLY("vcore", NULL),
};

static struct regulator_init_data d2153_buck1 = {
	.constraints = {
		.min_uV = D2153_BUCK1_VOLT_LOWER,
		.max_uV = D2153_BUCK1_VOLT_UPPER,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
		.always_on = 1,
	},
	.num_consumer_supplies = ARRAY_SIZE(d2153_buck1_supplies),
	.consumer_supplies = d2153_buck1_supplies,
};

// BUCK2
static struct regulator_consumer_supply d2153_buck2_supplies[] = {
	REGULATOR_SUPPLY("vio2", NULL),
};

static struct regulator_init_data d2153_buck2 = {
	.constraints = {
		.min_uV = D2153_BUCK2_VOLT_LOWER,
		.max_uV = D2153_BUCK2_VOLT_UPPER,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
		.always_on = 1,
	},
	.num_consumer_supplies = ARRAY_SIZE(d2153_buck2_supplies),
	.consumer_supplies = d2153_buck2_supplies,
};

// BUCK3
static struct regulator_consumer_supply d2153_buck3_supplies[] = {
	REGULATOR_SUPPLY("vio1", NULL),
};

static struct regulator_init_data d2153_buck3 = {
	.constraints = {
		.min_uV = D2153_BUCK3_VOLT_LOWER,
		.max_uV = D2153_BUCK3_VOLT_UPPER,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
		.always_on = 1,
	},
	.num_consumer_supplies = ARRAY_SIZE(d2153_buck3_supplies),
	.consumer_supplies = d2153_buck3_supplies,
};

// BUCK4
static struct regulator_consumer_supply d2153_buck4_supplies[] = {
	REGULATOR_SUPPLY("vcore_rf", NULL),
};

static struct regulator_init_data d2153_buck4 = {
	.constraints = {
		.min_uV = D2153_BUCK4_VOLT_LOWER,
		.max_uV = D2153_BUCK4_VOLT_UPPER,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_STATUS,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
		.always_on = 1,
	},
	.num_consumer_supplies = ARRAY_SIZE(d2153_buck4_supplies),
	.consumer_supplies = d2153_buck4_supplies,
};

// BUCK5
static struct regulator_consumer_supply d2153_buck5_supplies[] = {
	REGULATOR_SUPPLY("vana1_rf", NULL),
};

static struct regulator_init_data d2153_buck5 = {
	.constraints = {
		.min_uV = D2153_BUCK5_VOLT_LOWER,
		.max_uV = D2153_BUCK5_VOLT_UPPER,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_STATUS,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
		.always_on = 1,
	},
	.num_consumer_supplies = ARRAY_SIZE(d2153_buck5_supplies),
	.consumer_supplies = d2153_buck5_supplies,
};

// BUCK6
static struct regulator_consumer_supply d2153_buck6_supplies[] = {
	REGULATOR_SUPPLY("vpam", NULL),
};

static struct regulator_init_data d2153_buck6 = {
	.constraints = {
		.min_uV = D2153_BUCK6_VOLT_LOWER,
		.max_uV = D2153_BUCK6_VOLT_UPPER,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_STATUS,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
		.always_on = 1,
	},
	.num_consumer_supplies = ARRAY_SIZE(d2153_buck6_supplies),
	.consumer_supplies = d2153_buck6_supplies,
};

/* D2153 LDOs */
// LDO1
__weak struct regulator_consumer_supply d2153_ldo1_supplies[] = {
	REGULATOR_SUPPLY("vdig_rf", NULL),	// VDIG_RF
};

static struct regulator_init_data d2153_ldo1 = {
	.constraints = {
		.min_uV = D2153_LDO1_VOLT_LOWER,
		.max_uV = D2153_LDO1_VOLT_UPPER,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
		.always_on = 0,
	},
	.num_consumer_supplies = ARRAY_SIZE(d2153_ldo1_supplies),
	.consumer_supplies = d2153_ldo1_supplies,
};

// LDO2
__weak struct regulator_consumer_supply d2153_ldo2_supplies[] = {
	REGULATOR_SUPPLY("vdd_mhl", NULL),	// VDDR
};

static struct regulator_init_data d2153_ldo2 = {
	.constraints = {
		.min_uV = D2153_LDO2_VOLT_LOWER,
		.max_uV = D2153_LDO2_VOLT_UPPER,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
//		.always_on = 1,
	},
	.num_consumer_supplies = ARRAY_SIZE(d2153_ldo2_supplies),
	.consumer_supplies = d2153_ldo2_supplies,
};

// LDO3
__weak struct regulator_consumer_supply d2153_ldo3_supplies[] = {
	REGULATOR_SUPPLY("vmmc", NULL),	// VMMC
};

static struct regulator_init_data d2153_ldo3 = {
	.constraints = {
		.min_uV = D2153_LDO3_VOLT_LOWER,
		.max_uV = D2153_LDO3_VOLT_UPPER,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
//		.always_on = 1,
	},
	.num_consumer_supplies = ARRAY_SIZE(d2153_ldo3_supplies),
	.consumer_supplies = d2153_ldo3_supplies,
};

// LDO4
__weak struct regulator_consumer_supply d2153_ldo4_supplies[] = {
	REGULATOR_SUPPLY("vregtcxo", NULL),	// VVCTCXO
};

static struct regulator_init_data d2153_ldo4 = {
	.constraints = {
		.min_uV = D2153_LDO4_VOLT_LOWER,
		.max_uV = D2153_LDO4_VOLT_UPPER,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
//		.always_on = 1,
	},
	.num_consumer_supplies = ARRAY_SIZE(d2153_ldo4_supplies),
	.consumer_supplies = d2153_ldo4_supplies,
};

// LDO5
__weak struct regulator_consumer_supply d2153_ldo5_supplies[] = {
	REGULATOR_SUPPLY("vmipi", NULL),	// VMIPI
};

static struct regulator_init_data d2153_ldo5 = {
	.constraints = {
		.min_uV = D2153_LDO5_VOLT_LOWER,
		.max_uV = D2153_LDO5_VOLT_UPPER,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
//		.always_on = 1,
	},
	.num_consumer_supplies = ARRAY_SIZE(d2153_ldo5_supplies),
	.consumer_supplies = d2153_ldo5_supplies,
};

// LDO6
__weak struct regulator_consumer_supply d2153_ldo6_supplies[] = {
	REGULATOR_SUPPLY("vusim1", NULL),	// VUSIM1
};

static struct regulator_init_data d2153_ldo6 = {
	.constraints = {
		.min_uV = D2153_LDO6_VOLT_LOWER,
		.max_uV = D2153_LDO6_VOLT_UPPER,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
		.always_on = 1,
	},
	.num_consumer_supplies = ARRAY_SIZE(d2153_ldo6_supplies),
	.consumer_supplies = d2153_ldo6_supplies,
};

// LDO7
__weak struct regulator_consumer_supply d2153_ldo7_supplies[] = {
	REGULATOR_SUPPLY("sensor_3v", NULL),	// SENSOR
};

static struct regulator_init_data d2153_ldo7 = {
	.constraints = {
		.min_uV = D2153_LDO7_VOLT_LOWER,
		.max_uV = D2153_LDO7_VOLT_UPPER,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
//		.always_on = 1,
	},
	.num_consumer_supplies = ARRAY_SIZE(d2153_ldo7_supplies),
	.consumer_supplies = d2153_ldo7_supplies,
};

// LDO8
__weak struct regulator_consumer_supply d2153_ldo8_supplies[] = {
	REGULATOR_SUPPLY("vlcd_3v", NULL),	// VLCD_3V0
};

static struct regulator_init_data d2153_ldo8 = {
	.constraints = {
		.min_uV = D2153_LDO8_VOLT_LOWER,
		.max_uV = D2153_LDO8_VOLT_UPPER,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
//		.always_on = 1,
	},
	.num_consumer_supplies = ARRAY_SIZE(d2153_ldo8_supplies),
	.consumer_supplies = d2153_ldo8_supplies,
};

// LDO9
__weak struct regulator_consumer_supply d2153_ldo9_supplies[] = {
	REGULATOR_SUPPLY("vlcd_1v8", NULL),	// VLDO_1V8
};

static struct regulator_init_data d2153_ldo9 = {
	.constraints = {
		.min_uV = D2153_LDO9_VOLT_LOWER,
		.max_uV = D2153_LDO9_VOLT_UPPER,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
//		.always_on = 1,
	},
	.num_consumer_supplies = ARRAY_SIZE(d2153_ldo9_supplies),
	.consumer_supplies = d2153_ldo9_supplies,
};

// LDO10
__weak struct regulator_consumer_supply d2153_ldo10_supplies[] = {
	REGULATOR_SUPPLY("vio_sd", NULL),	// VIO_SD
};

static struct regulator_init_data d2153_ldo10 = {
	.constraints = {
		.min_uV = D2153_LDO10_VOLT_LOWER,
		.max_uV = D2153_LDO10_VOLT_UPPER,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
		.boot_on = 0,
	},
	.num_consumer_supplies = ARRAY_SIZE(d2153_ldo10_supplies),
	.consumer_supplies = d2153_ldo10_supplies,
};

// LDO11
__weak struct regulator_consumer_supply d2153_ldo11_supplies[] = {
	REGULATOR_SUPPLY("key_led", NULL),	// key led
};

static struct regulator_init_data d2153_ldo11 = {
	.constraints = {
		.min_uV = D2153_LDO11_VOLT_LOWER,
		.max_uV = D2153_LDO11_VOLT_UPPER,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
//		.always_on = 1,
	},
	.num_consumer_supplies = ARRAY_SIZE(d2153_ldo11_supplies),
	.consumer_supplies = d2153_ldo11_supplies,
};

// LDO12
__weak struct regulator_consumer_supply d2153_ldo12_supplies[] = {
	REGULATOR_SUPPLY("cam_sensor_a", NULL),	// cam_sensor_a
};

static struct regulator_init_data d2153_ldo12 = {
	.constraints = {
		.min_uV = D2153_LDO12_VOLT_LOWER,
		.max_uV = D2153_LDO12_VOLT_UPPER,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
//		.always_on = 1,
	},
	.num_consumer_supplies = ARRAY_SIZE(d2153_ldo12_supplies),
	.consumer_supplies = d2153_ldo12_supplies,
};

// LDO13
__weak struct regulator_consumer_supply d2153_ldo13_supplies[] = {
	REGULATOR_SUPPLY("cam_af", NULL),	// cam_af
};

static struct regulator_init_data d2153_ldo13 = {
	.constraints = {
		.min_uV = D2153_LDO13_VOLT_LOWER,
		.max_uV = D2153_LDO13_VOLT_UPPER,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
//		.always_on = 1,
	},
	.num_consumer_supplies = ARRAY_SIZE(d2153_ldo13_supplies),
	.consumer_supplies = d2153_ldo13_supplies,
};

// LDO14
__weak struct regulator_consumer_supply d2153_ldo14_supplies[] = {
	REGULATOR_SUPPLY("vt_cam", NULL),	// vt_cam
};

static struct regulator_init_data d2153_ldo14 = {
	.constraints = {
		.min_uV = D2153_LDO14_VOLT_LOWER,
		.max_uV = D2153_LDO14_VOLT_UPPER,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
//		.always_on = 1,
	},
	.num_consumer_supplies = ARRAY_SIZE(d2153_ldo14_supplies),
	.consumer_supplies = d2153_ldo14_supplies,
};

// LDO15
__weak struct regulator_consumer_supply d2153_ldo15_supplies[] = {
	REGULATOR_SUPPLY("vsd", NULL),	// VSD
};

static struct regulator_init_data d2153_ldo15 = {
	.constraints = {
		.min_uV = D2153_LDO15_VOLT_LOWER,
		.max_uV = D2153_LDO15_VOLT_UPPER,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
		.boot_on = 0,
	},
	.num_consumer_supplies = ARRAY_SIZE(d2153_ldo15_supplies),
	.consumer_supplies = d2153_ldo15_supplies,
};

// LDO16
__weak struct regulator_consumer_supply d2153_ldo16_supplies[] = {
	REGULATOR_SUPPLY("none", NULL),	// none
};

static struct regulator_init_data d2153_ldo16 = {
	.constraints = {
		.min_uV = D2153_LDO16_VOLT_LOWER,
		.max_uV = D2153_LDO16_VOLT_UPPER,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
	},
	.num_consumer_supplies = ARRAY_SIZE(d2153_ldo16_supplies),
	.consumer_supplies = d2153_ldo16_supplies,
};

// LDO17
__weak struct regulator_consumer_supply d2153_ldo17_supplies[] = {
	REGULATOR_SUPPLY("cam_sensor_io", NULL),	// cam_sensor_io
};

static struct regulator_init_data d2153_ldo17 = {
	.constraints = {
		.min_uV = D2153_LDO17_VOLT_LOWER,
		.max_uV = D2153_LDO17_VOLT_UPPER,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
	},
	.num_consumer_supplies = ARRAY_SIZE(d2153_ldo17_supplies),
	.consumer_supplies = d2153_ldo17_supplies,
};

// LDO18
__weak struct regulator_consumer_supply d2153_ldo18_supplies[] = {
	REGULATOR_SUPPLY("vrf_ana_high", NULL),	// vrf_ana_high
};

static struct regulator_init_data d2153_ldo18 = {
	.constraints = {
		.min_uV = D2153_LDO18_VOLT_LOWER,
		.max_uV = D2153_LDO18_VOLT_UPPER,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
	},
	.num_consumer_supplies = ARRAY_SIZE(d2153_ldo18_supplies),
	.consumer_supplies = d2153_ldo18_supplies,
};

// LDO19
__weak struct regulator_consumer_supply d2153_ldo19_supplies[] = {
	REGULATOR_SUPPLY("vtsp_3v", NULL),	// vtsp_3v
};

static struct regulator_init_data d2153_ldo19 = {
	.constraints = {
		.min_uV = D2153_LDO19_VOLT_LOWER,
		.max_uV = D2153_LDO19_VOLT_UPPER,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
	},
	.num_consumer_supplies = ARRAY_SIZE(d2153_ldo19_supplies),
	.consumer_supplies = d2153_ldo19_supplies,
};

// LDO20
__weak struct regulator_consumer_supply d2153_ldo20_supplies[] = {
	REGULATOR_SUPPLY("sensor_1v8", NULL),	// sensor_1v8
};

static struct regulator_init_data d2153_ldo20 = {
	.constraints = {
		.min_uV = D2153_LDO19_VOLT_LOWER,
		.max_uV = D2153_LDO19_VOLT_UPPER,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
	},
	.num_consumer_supplies = ARRAY_SIZE(d2153_ldo20_supplies),
	.consumer_supplies = d2153_ldo20_supplies,
};

// LDO_AUD_1
__weak struct regulator_consumer_supply d2153_ldoaud1_supplies[] = {
	REGULATOR_SUPPLY("aud1", NULL),	// aud1
};

static struct regulator_init_data d2153_ldoaud1 = {
	.constraints = {
		.min_uV = D2153_LDOAUD1_VOLT_LOWER,
		.max_uV = D2153_LDOAUD1_VOLT_UPPER,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
	},
	.num_consumer_supplies = ARRAY_SIZE(d2153_ldoaud1_supplies),
	.consumer_supplies = d2153_ldoaud1_supplies,
};

// LDO_AUD_2
__weak struct regulator_consumer_supply d2153_ldoaud2_supplies[] = {
	REGULATOR_SUPPLY("aud2", NULL),	// aud2
};

static struct regulator_init_data d2153_ldoaud2 = {
	.constraints = {
		.min_uV = D2153_LDOAUD2_VOLT_LOWER,
		.max_uV = D2153_LDOAUD2_VOLT_UPPER,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
	},
	.num_consumer_supplies = ARRAY_SIZE(d2153_ldoaud2_supplies),
	.consumer_supplies = d2153_ldoaud2_supplies,
};


static struct d2153_regl_init_data d2153_regulators_init_data[D2153_NUMBER_OF_REGULATORS] = {
	[D2153_BUCK_1] = { D2153_BUCK_1,  &d2153_buck1 },
	[D2153_BUCK_2] = { D2153_BUCK_2,  &d2153_buck2 },
	[D2153_BUCK_3] = { D2153_BUCK_3,  &d2153_buck3 },
	[D2153_BUCK_4] = { D2153_BUCK_4,  &d2153_buck4 },
	[D2153_BUCK_5] = { D2153_BUCK_5,  &d2153_buck5 },
	[D2153_BUCK_6] = { D2153_BUCK_6,  &d2153_buck6 },

	[D2153_LDO_1]  = { D2153_LDO_1, &d2153_ldo1 },
	[D2153_LDO_2]  = { D2153_LDO_2, &d2153_ldo2 },
	[D2153_LDO_3]  = { D2153_LDO_3, &d2153_ldo3 },
	[D2153_LDO_4]  = { D2153_LDO_4, &d2153_ldo4 },
	[D2153_LDO_5]  = { D2153_LDO_5, &d2153_ldo5 },
	[D2153_LDO_6]  = { D2153_LDO_6, &d2153_ldo6 },
	[D2153_LDO_7]  = { D2153_LDO_7, &d2153_ldo7 },
	[D2153_LDO_8]  = { D2153_LDO_8, &d2153_ldo8 },
	[D2153_LDO_9]  = { D2153_LDO_9, &d2153_ldo9 },
	[D2153_LDO_10] = { D2153_LDO_10, &d2153_ldo10 },
	[D2153_LDO_11] = { D2153_LDO_11, &d2153_ldo11 },
	[D2153_LDO_12] = { D2153_LDO_12, &d2153_ldo12 },
	[D2153_LDO_13] = { D2153_LDO_13, &d2153_ldo13 },
	[D2153_LDO_14] = { D2153_LDO_14, &d2153_ldo14 },
	[D2153_LDO_15] = { D2153_LDO_15, &d2153_ldo15 },
	[D2153_LDO_16] = { D2153_LDO_16, &d2153_ldo16 },
	[D2153_LDO_17] = { D2153_LDO_17, &d2153_ldo17 },
	[D2153_LDO_18] = { D2153_LDO_15, &d2153_ldo18 },
	[D2153_LDO_19] = { D2153_LDO_16, &d2153_ldo19 },
	[D2153_LDO_20] = { D2153_LDO_17, &d2153_ldo20 },
	
	[D2153_LDO_AUD1] = { D2153_LDO_AUD1, &d2153_ldoaud1 },
	[D2153_LDO_AUD2] = { D2153_LDO_AUD2, &d2153_ldoaud2 },
};

struct d2153_battery_platform_data pbat_pdata = {
       .battery_technology = POWER_SUPPLY_TECHNOLOGY_LION,
       .battery_capacity = 1300,
       .vf_lower    = 250,
       .vf_upper = 510,
};


struct d2153_platform_data d2153_pdata = {	
	.pbat_platform  = &pbat_pdata,
	.regulator_data = &d2153_regulators_init_data[0],
	.regl_map = {
		/*
		 *		Define initial MCTL value of EOS2 with D2153
		 *	
		 *	[ LDO ]	0x0 : Off	[ BUCK 2,3,4]	0x0 : Off
		 *			0x1 : On					0x1 : On
		 *			0x2 : Sleep - LPM			0x2 : Sleep(Force PFM mode) - LPM
		 *			0x3 : n/a				0x3 : n/a
		 *
		 *	[ BUCK 1 ]	0x0 : Off
		 *				0x1 : On 	(reference VBUCK1     reg[0x002E])
		 *				0x2 : Sleep 	(reference VBUCK1_RET reg[0x0061])
		 *				0x3 : On 	(reference VBUCK1_TUR reg[0x0062])
		 *
		 *		
		 * ---------------------------------------------------------------	
		 * [PC1|PC2]	11	|	10	|	01	|	00
		 * ---------------------------------------------------------------
		 *	[MCTL]	M3	|	M2	|	M1	|	M0
		 * ---------------------------------------------------------------
		 *	0xDE :	11		01		11		10	(TUR, ON , TUR, LPM)
		 *	0xCD :	11		00		11		01	(TUR, OFF, TUR, ON )
		 *
		 *	0x00 :	00		00		00		00	(OFF, OFF, OFF, OFF)
		 *	0x66 :	01		10		01		10	(ON , LPM, ON , LPM)
		 *	0x44 :	01		00		01		00	(ON , OFF, ON , OFF)
		 * ---------------------------------------------------------------
		 *
		 * NEVIS use M3 and M0
		*/
#if 0
		D2153_MCTL_MODE_INIT(D2153_BUCK_1, 0xDE, D2153_REGULATOR_LPM_IN_DSM),	// VCORE_1.125
		D2153_MCTL_MODE_INIT(D2153_BUCK_2, 0x56, D2153_REGULATOR_MAX), // VIO2_1.225V 
		D2153_MCTL_MODE_INIT(D2153_BUCK_3, 0x56, D2153_REGULATOR_MAX), // VIO1_1.825V 
		D2153_MCTL_MODE_INIT(D2153_BUCK_4, 0x56, D2153_REGULATOR_MAX),	// VCORE_RF_1.25V.
		D2153_MCTL_MODE_INIT(D2153_BUCK_5, 0x56, D2153_REGULATOR_MAX), // VANA1_RF_1.525V - TSR
		D2153_MCTL_MODE_INIT(D2153_BUCK_6, 0x56, D2153_REGULATOR_MAX),	// VPAM_3.3V - used.
		
		D2153_MCTL_MODE_INIT(D2153_LDO_1,  0x56, D2153_REGULATOR_MAX),	//VDIG_RF_1.1
		D2153_MCTL_MODE_INIT(D2153_LDO_2,  0x00, D2153_REGULATOR_OFF_IN_DSM),	// VDD_MHL_1.2V
		D2153_MCTL_MODE_INIT(D2153_LDO_3,  0x56, D2153_REGULATOR_MAX),	// VMMC_2.85V
		D2153_MCTL_MODE_INIT(D2153_LDO_4,  0x56, D2153_REGULATOR_MAX),	// VREG_TCXO_1.8V
		D2153_MCTL_MODE_INIT(D2153_LDO_5,  0x56, D2153_REGULATOR_MAX),	// VMIPI_1.8V
		D2153_MCTL_MODE_INIT(D2153_LDO_6,  0x56, D2153_REGULATOR_MAX),	// VUSIM1_1.8V
		D2153_MCTL_MODE_INIT(D2153_LDO_7,  0x00, D2153_REGULATOR_OFF_IN_DSM),	// SENSOR_3V
		D2153_MCTL_MODE_INIT(D2153_LDO_8,  0x00, D2153_REGULATOR_OFF_IN_DSM),	// VLCD_3.0V
		D2153_MCTL_MODE_INIT(D2153_LDO_9,  0x00, D2153_REGULATOR_OFF_IN_DSM),	// VLCD_1.8V
		D2153_MCTL_MODE_INIT(D2153_LDO_10, 0x00, D2153_REGULATOR_OFF_IN_DSM),	// VIO_SD_2.85V
		D2153_MCTL_MODE_INIT(D2153_LDO_11, 0x00, D2153_REGULATOR_OFF_IN_DSM),	// KEY_LED_3.3V
		D2153_MCTL_MODE_INIT(D2153_LDO_12, 0x00, D2153_REGULATOR_OFF_IN_DSM),	// CAM_SENSOR_A2.8V
		D2153_MCTL_MODE_INIT(D2153_LDO_13, 0x00, D2153_REGULATOR_OFF_IN_DSM),	// CAM_AF_2.8V
		D2153_MCTL_MODE_INIT(D2153_LDO_14, 0x00, D2153_REGULATOR_OFF_IN_DSM),	// VT_CAM_1.2V
		D2153_MCTL_MODE_INIT(D2153_LDO_15, 0x00, D2153_REGULATOR_OFF_IN_DSM),	// VSD_2.85V
		D2153_MCTL_MODE_INIT(D2153_LDO_16, 0x00, D2153_REGULATOR_OFF_IN_DSM),	// VTCXO_D_1V8
		D2153_MCTL_MODE_INIT(D2153_LDO_17, 0x00, D2153_REGULATOR_OFF_IN_DSM),	// CAM_SENSOR_IO_1.8V
		D2153_MCTL_MODE_INIT(D2153_LDO_18, 0x56, D2153_REGULATOR_MAX), // VRF_ANA_HIGH_2.8V
		D2153_MCTL_MODE_INIT(D2153_LDO_19, 0x00, D2153_REGULATOR_OFF_IN_DSM), // VTSP_A3.0V
		D2153_MCTL_MODE_INIT(D2153_LDO_20, 0x00, D2153_REGULATOR_OFF_IN_DSM), // SENSOR_1.8V
		D2153_MCTL_MODE_INIT(D2153_LDO_AUD1, 0x00, D2153_REGULATOR_OFF_IN_DSM),	// LDO_AUD1 1.8
		D2153_MCTL_MODE_INIT(D2153_LDO_AUD2, 0x00, D2153_REGULATOR_OFF_IN_DSM), // LDO_AUD2 3.3
#elif 1	// Default setting for sleep event. - Recommended by RMC
		D2153_MCTL_MODE_INIT(D2153_BUCK_1, 0xDE, D2153_REGULATOR_LPM_IN_DSM),	// VCORE_1.125
		D2153_MCTL_MODE_INIT(D2153_BUCK_2, 0x56, D2153_REGULATOR_LPM_IN_DSM), // VIO2_1.225V 
		D2153_MCTL_MODE_INIT(D2153_BUCK_3, 0x56, D2153_REGULATOR_LPM_IN_DSM), // VIO1_1.825V 
		D2153_MCTL_MODE_INIT(D2153_BUCK_4, 0x56, D2153_REGULATOR_LPM_IN_DSM),	// VCORE_RF_1.25V.
		D2153_MCTL_MODE_INIT(D2153_BUCK_5, 0x00, D2153_REGULATOR_MAX), // VANA1_RF_1.525V - TSR
		D2153_MCTL_MODE_INIT(D2153_BUCK_6, 0x54, D2153_REGULATOR_OFF_IN_DSM),	// VPAM_3.3V - used.

		D2153_MCTL_MODE_INIT(D2153_LDO_1,  0x56, D2153_REGULATOR_LPM_IN_DSM),	//VDIG_RF_1.1
		D2153_MCTL_MODE_INIT(D2153_LDO_2,  0x54, D2153_REGULATOR_OFF_IN_DSM),	// VDD_MHL_1.2V
		D2153_MCTL_MODE_INIT(D2153_LDO_3,  0x54/*0x56*/, D2153_REGULATOR_OFF_IN_DSM),	// VMMC_2.85V
		D2153_MCTL_MODE_INIT(D2153_LDO_4,  0x54, D2153_REGULATOR_OFF_IN_DSM),	// VREG_TCXO_1.8V
		D2153_MCTL_MODE_INIT(D2153_LDO_5,  0x56, D2153_REGULATOR_LPM_IN_DSM),	// VMIPI_1.8V
		D2153_MCTL_MODE_INIT(D2153_LDO_6,  0x56, D2153_REGULATOR_LPM_IN_DSM),	// VUSIM1_1.8V
		D2153_MCTL_MODE_INIT(D2153_LDO_7,  0x54, D2153_REGULATOR_OFF_IN_DSM),	// SENSOR_3V
		D2153_MCTL_MODE_INIT(D2153_LDO_8,  0x54, D2153_REGULATOR_OFF_IN_DSM),	// VLCD_3.0V
		D2153_MCTL_MODE_INIT(D2153_LDO_9,  0x54, D2153_REGULATOR_OFF_IN_DSM),	// VLCD_1.8V
		D2153_MCTL_MODE_INIT(D2153_LDO_10, 0x54, D2153_REGULATOR_OFF_IN_DSM),	// VIO_SD_2.85V
		D2153_MCTL_MODE_INIT(D2153_LDO_11, 0x54, D2153_REGULATOR_OFF_IN_DSM),	// KEY_LED_3.3V
		D2153_MCTL_MODE_INIT(D2153_LDO_12, 0x54, D2153_REGULATOR_OFF_IN_DSM),	// CAM_SENSOR_A2.8V
		D2153_MCTL_MODE_INIT(D2153_LDO_13, 0x54, D2153_REGULATOR_OFF_IN_DSM),	// CAM_AF_2.8V
		D2153_MCTL_MODE_INIT(D2153_LDO_14, 0x54, D2153_REGULATOR_OFF_IN_DSM),	// VT_CAM_1.2V
		D2153_MCTL_MODE_INIT(D2153_LDO_15, 0x54, D2153_REGULATOR_OFF_IN_DSM),	// VSD_2.85V
		D2153_MCTL_MODE_INIT(D2153_LDO_16, 0x54, D2153_REGULATOR_OFF_IN_DSM),	// VTCXO_D_1V8
		D2153_MCTL_MODE_INIT(D2153_LDO_17, 0x54, D2153_REGULATOR_OFF_IN_DSM),	// CAM_SENSOR_IO_1.8V
		D2153_MCTL_MODE_INIT(D2153_LDO_18, 0x00, D2153_REGULATOR_MAX), // VRF_ANA_HIGH_2.8V
		D2153_MCTL_MODE_INIT(D2153_LDO_19, 0x00, D2153_REGULATOR_OFF_IN_DSM), // VTSP_A3.0V
		D2153_MCTL_MODE_INIT(D2153_LDO_20, 0x54, D2153_REGULATOR_OFF_IN_DSM), // SENSOR_1.8V
		D2153_MCTL_MODE_INIT(D2153_LDO_AUD1, 0x56, D2153_REGULATOR_LPM_IN_DSM), // LDO_AUD1 1.8
		D2153_MCTL_MODE_INIT(D2153_LDO_AUD2, 0x56, D2153_REGULATOR_LPM_IN_DSM), // LDO_AUD2 3.3

#elif 1	// Default setting on DLG side [ TEST ONLY ]
		D2153_MCTL_MODE_INIT(D2153_BUCK_1, 0xDE, D2153_REGULATOR_LPM_IN_DSM),	// VCORE_1.125
		D2153_MCTL_MODE_INIT(D2153_BUCK_2, 0x56, D2153_REGULATOR_LPM_IN_DSM), // VIO2_1.225V 
		D2153_MCTL_MODE_INIT(D2153_BUCK_3, 0x56, D2153_REGULATOR_LPM_IN_DSM), // VIO1_1.825V 
		D2153_MCTL_MODE_INIT(D2153_BUCK_4, 0x56, D2153_REGULATOR_LPM_IN_DSM),	// VCORE_RF_1.25V.
		D2153_MCTL_MODE_INIT(D2153_BUCK_5, 0x00, D2153_REGULATOR_MAX), // VANA1_RF_1.525V - TSR
		D2153_MCTL_MODE_INIT(D2153_BUCK_6, 0x54, D2153_REGULATOR_OFF_IN_DSM),	// VPAM_3.3V - used.

		D2153_MCTL_MODE_INIT(D2153_LDO_1,  0x56, D2153_REGULATOR_LPM_IN_DSM),	//VDIG_RF_1.1
		D2153_MCTL_MODE_INIT(D2153_LDO_2,  0x54, D2153_REGULATOR_OFF_IN_DSM),	// VDD_MHL_1.2V
		D2153_MCTL_MODE_INIT(D2153_LDO_3,  0x56, D2153_REGULATOR_OFF_IN_DSM),	// VMMC_2.85V		--> [OFF -> LPM]
		D2153_MCTL_MODE_INIT(D2153_LDO_4,  0x54, D2153_REGULATOR_OFF_IN_DSM),	// VREG_TCXO_1.8V
		D2153_MCTL_MODE_INIT(D2153_LDO_5,  0x56, D2153_REGULATOR_LPM_IN_DSM),	// VMIPI_1.8V
		D2153_MCTL_MODE_INIT(D2153_LDO_6,  0x56, D2153_REGULATOR_LPM_IN_DSM),	// VUSIM1_1.8V
		D2153_MCTL_MODE_INIT(D2153_LDO_7,  0x54, D2153_REGULATOR_OFF_IN_DSM),	// SENSOR_3V
		D2153_MCTL_MODE_INIT(D2153_LDO_8,  0x54, D2153_REGULATOR_OFF_IN_DSM),	// VLCD_3.0V
		D2153_MCTL_MODE_INIT(D2153_LDO_9,  0x54, D2153_REGULATOR_OFF_IN_DSM),	// VLCD_1.8V
		D2153_MCTL_MODE_INIT(D2153_LDO_10, 0x54, D2153_REGULATOR_OFF_IN_DSM),	// VIO_SD_2.85V
		D2153_MCTL_MODE_INIT(D2153_LDO_11, 0x54, D2153_REGULATOR_OFF_IN_DSM),	// KEY_LED_3.3V
		D2153_MCTL_MODE_INIT(D2153_LDO_12, 0x54, D2153_REGULATOR_OFF_IN_DSM),	// CAM_SENSOR_A2.8V
		D2153_MCTL_MODE_INIT(D2153_LDO_13, 0x54, D2153_REGULATOR_OFF_IN_DSM),	// CAM_AF_2.8V
		D2153_MCTL_MODE_INIT(D2153_LDO_14, 0x54, D2153_REGULATOR_OFF_IN_DSM),	// VT_CAM_1.2V
		D2153_MCTL_MODE_INIT(D2153_LDO_15, 0x54, D2153_REGULATOR_OFF_IN_DSM),	// VSD_2.85V
		D2153_MCTL_MODE_INIT(D2153_LDO_16, 0x54, D2153_REGULATOR_OFF_IN_DSM),	// VTCXO_D_1V8
		D2153_MCTL_MODE_INIT(D2153_LDO_17, 0x54, D2153_REGULATOR_OFF_IN_DSM),	// CAM_SENSOR_IO_1.8V
		D2153_MCTL_MODE_INIT(D2153_LDO_18, 0x00, D2153_REGULATOR_MAX), // VRF_ANA_HIGH_2.8V
		D2153_MCTL_MODE_INIT(D2153_LDO_19, 0x00, D2153_REGULATOR_OFF_IN_DSM), // VTSP_A3.0V
		D2153_MCTL_MODE_INIT(D2153_LDO_20, 0x54, D2153_REGULATOR_OFF_IN_DSM), // SENSOR_1.8V
		D2153_MCTL_MODE_INIT(D2153_LDO_AUD1, 0x56, D2153_REGULATOR_LPM_IN_DSM), // LDO_AUD1 1.8
		D2153_MCTL_MODE_INIT(D2153_LDO_AUD2, 0x56, D2153_REGULATOR_LPM_IN_DSM), // LDO_AUD2 3.3

#else	// Current setting on 26 Nov 2012
		D2153_MCTL_MODE_INIT(D2153_BUCK_1, 0xDE, D2153_REGULATOR_LPM_IN_DSM),	// VCORE_1.125
		D2153_MCTL_MODE_INIT(D2153_BUCK_2, 0x56, D2153_REGULATOR_MAX), // VIO2_1.225V 
		D2153_MCTL_MODE_INIT(D2153_BUCK_3, 0x56, D2153_REGULATOR_MAX), // VIO1_1.825V 
		D2153_MCTL_MODE_INIT(D2153_BUCK_4, 0x56, D2153_REGULATOR_MAX),	// VCORE_RF_1.25V.
		D2153_MCTL_MODE_INIT(D2153_BUCK_5, 0x00, D2153_REGULATOR_MAX), // VANA1_RF_1.525V - TSR
		D2153_MCTL_MODE_INIT(D2153_BUCK_6, 0x56, D2153_REGULATOR_MAX),	// VPAM_3.3V - used.
		
		D2153_MCTL_MODE_INIT(D2153_LDO_1,  0x56, D2153_REGULATOR_MAX),	//VDIG_RF_1.1
		D2153_MCTL_MODE_INIT(D2153_LDO_2,  0x56, D2153_REGULATOR_OFF_IN_DSM),	// VDD_MHL_1.2V
		D2153_MCTL_MODE_INIT(D2153_LDO_3,  0x56, D2153_REGULATOR_MAX),	// VMMC_2.85V
		D2153_MCTL_MODE_INIT(D2153_LDO_4,  0x56, D2153_REGULATOR_MAX),	// VREG_TCXO_1.8V
		D2153_MCTL_MODE_INIT(D2153_LDO_5,  0x56, D2153_REGULATOR_MAX),	// VMIPI_1.8V
		D2153_MCTL_MODE_INIT(D2153_LDO_6,  0x56, D2153_REGULATOR_MAX),	// VUSIM1_1.8V
		D2153_MCTL_MODE_INIT(D2153_LDO_7,  0x56, D2153_REGULATOR_OFF_IN_DSM),	// SENSOR_3V
		D2153_MCTL_MODE_INIT(D2153_LDO_8,  0x56, D2153_REGULATOR_OFF_IN_DSM),	// VLCD_3.0V
		D2153_MCTL_MODE_INIT(D2153_LDO_9,  0x56, D2153_REGULATOR_OFF_IN_DSM),	// VLCD_1.8V
		D2153_MCTL_MODE_INIT(D2153_LDO_10, 0x56, D2153_REGULATOR_OFF_IN_DSM),	// VIO_SD_2.85V
		D2153_MCTL_MODE_INIT(D2153_LDO_11, 0x56, D2153_REGULATOR_OFF_IN_DSM),	// KEY_LED_3.3V
		D2153_MCTL_MODE_INIT(D2153_LDO_12, 0x56, D2153_REGULATOR_OFF_IN_DSM),	// CAM_SENSOR_A2.8V
		D2153_MCTL_MODE_INIT(D2153_LDO_13, 0x56, D2153_REGULATOR_OFF_IN_DSM),	// CAM_AF_2.8V
		D2153_MCTL_MODE_INIT(D2153_LDO_14, 0x56, D2153_REGULATOR_OFF_IN_DSM),	// VT_CAM_1.2V
		D2153_MCTL_MODE_INIT(D2153_LDO_15, 0x56, D2153_REGULATOR_OFF_IN_DSM),	// VSD_2.85V
		D2153_MCTL_MODE_INIT(D2153_LDO_16, 0x56, D2153_REGULATOR_OFF_IN_DSM),	// VTCXO_D_1V8
		D2153_MCTL_MODE_INIT(D2153_LDO_17, 0x56, D2153_REGULATOR_OFF_IN_DSM),	// CAM_SENSOR_IO_1.8V
		D2153_MCTL_MODE_INIT(D2153_LDO_18, 0x00, D2153_REGULATOR_MAX), // VRF_ANA_HIGH_2.8V
		D2153_MCTL_MODE_INIT(D2153_LDO_19, 0x56, D2153_REGULATOR_OFF_IN_DSM), // VTSP_A3.0V
		D2153_MCTL_MODE_INIT(D2153_LDO_20, 0x56, D2153_REGULATOR_OFF_IN_DSM), // SENSOR_1.8V
		D2153_MCTL_MODE_INIT(D2153_LDO_AUD1, 0x56, D2153_REGULATOR_LPM_IN_DSM),	// LDO_AUD1 1.8
		D2153_MCTL_MODE_INIT(D2153_LDO_AUD2, 0x56, D2153_REGULATOR_LPM_IN_DSM), // LDO_AUD2 3.3
#endif
	},
};
#endif /* CONFIG_MFD_D2153 */

static struct i2c_board_info __initdata i2c0_devices_d2153[] = {
#if defined(CONFIG_MFD_D2153)
	{
		// for D2153 PMIC driver
		I2C_BOARD_INFO("d2153", D2153_PMIC_I2C_ADDR),
		.platform_data = &d2153_pdata,
		.irq = irqpin2irq(28),
	},
#if 0 // register below I2C at mfd
	{
		// for D2153 Audio Code driver
		// TODO: DLG. eric. Have to check the codec address for I2C interface
		I2C_BOARD_INFO("d2153-codec", D2153_CODEC_I2C_ADDR),
		.platform_data = NULL,
	},
	{
		// for D2153 AAD driver
		// TODO: DLG. eric. Have to check.
		I2C_BOARD_INFO("d2153-aad", D2153_AAD_I2C_ADDR),
		.platform_data = NULL,
	},
#endif /* 0 */	
#endif /* CONFIG_MFD_D2153 */
};

static struct i2c_board_info __initdata i2c0_devices[] = {
#ifdef CONFIG_PMIC_INTERFACE
	{
  		I2C_BOARD_INFO("tps80032-power", 0x48),
		.platform_data = &tps_platform,
  	},
  	{
  		I2C_BOARD_INFO("tps80032-battery", 0x49),
		.irq = irqpin2irq(28),
  	},
  	{
  		I2C_BOARD_INFO("tps80032-dvs", 0x12),
  	},
  	{
  		I2C_BOARD_INFO("tps80032-jtag", 0x4A),
  	},
#else /* CONFIG_PMIC_INTERFACE */
	{
		I2C_BOARD_INFO("tps80032", 0x4A),
		.irq		= irqpin2irq(28),
		.platform_data	= &tps_platform,
	},
#endif /* CONFIG_PMIC_INTERFACE */
};

static struct i2c_board_info i2c4_devices[] = {
	{
		I2C_BOARD_INFO("atmel_mxt_ts", 0x4a),
		.platform_data = &mxt224_platform_data,
		.irq	= irqpin2irq(32),
	},
	{
		I2C_BOARD_INFO("sec_touch", 0x48),
		.irq	= irqpin2irq(32),
	},
};

static struct NCP6914_platform_data ncp6914info= {
	.subpmu_pwron_gpio = GPIO_PORT3,
};

static struct i2c_board_info i2c9gpio_devices[] = {
	{
		I2C_BOARD_INFO("ncp6914", 0x10),//address 20/21
		.irq	= irqpin2irq(5),
		.platform_data = &ncp6914info,
	},
};

static struct i2c_board_info i2cm_devices_d2153[] = {
	{
		I2C_BOARD_INFO("audience_a2220", 0x3E),
		.platform_data = &u2evm_a2220_data,
	},
};

static struct i2c_board_info i2cm_devices[] = {
	{
                I2C_BOARD_INFO("max98090", 0x10),
                .irq            = irqpin2irq(34),
    },
    {
                I2C_BOARD_INFO("max97236", 0x40),
                .irq            = irqpin2irq(34),
    },
    {
                I2C_BOARD_INFO("wm1811", 0x1a),
                .irq            = irqpin2irq(24),
    },
	{
		I2C_BOARD_INFO("audience_a2220", 0x3E),
		.platform_data = &u2evm_a2220_data,
	},
#if 0
#ifdef CONFIG_SND_FSI_WM1811_MODULE
	{
		I2C_BOARD_INFO("wm1811", 0x1a),
		.irq	= irqpin2irq(24),
	},
#endif /* CONFIG_SND_FSI_WM1811_MODULE */
#endif
#if 0
	{
		I2C_BOARD_INFO("led", 0x74),
	},
	{
		I2C_BOARD_INFO("flash", 0x30),
	},
#endif /* 0 */
};
#if 0
i2c_board_info i2cm_devices_es2[] = {
        {
                I2C_BOARD_INFO("max98090", 0x10),
                .irq            = irqpin2irq(34),
        },
        {
                I2C_BOARD_INFO("max97236", 0x40),
                .irq            = irqpin2irq(34),
        },
#if 0
	{
	        I2C_BOARD_INFO("led", 0x74),
	},
	{
	        I2C_BOARD_INFO("flash", 0x30),
	},
#endif
//	{
//	        I2C_BOARD_INFO("av7100", 0x70),
//	},
};
#endif


static struct platform_device *gpio_i2c_devices[] __initdata = {
#if defined(CONFIG_SAMSUNG_MHL)
	&mhl_i2c_gpio_device,
#endif	
};

static struct map_desc u2evm_io_desc[] __initdata = {
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

static void __init u2evm_map_io(void)
{
	iotable_init(u2evm_io_desc, ARRAY_SIZE(u2evm_io_desc));
#if defined(CONFIG_SEC_DEBUG_INFORM_IOTABLE)
	sec_debug_init();
#endif
	r8a73734_add_early_devices();
	shmobile_setup_console();
}

void __init u2evm_init_irq(void)
{
	r8a73734_init_irq();
}

#ifdef CONFIG_U2_STM_ETR_TO_SDRAM
static int wait_for_coresight_access_lock(u32 base)
{
        int retval = -1;
        int timeout = 512;
        int i;
        __raw_writel(0xc5acce55, base + 0xFB0); /* Lock Access */
        for (i = 0; i < timeout && retval; i++) {
                if ((__raw_readl(base + 0xFB4) & 2) == 0) retval = 0;
        }
        printk("wait_for_coresight_access_lock %d\n", retval);
        return retval;
}
#endif


static void irqc_set_chattering(int pin, int timing)
{
	u32 val;
	u32 *reg;

	reg = (pin >= 32) ? (u32 *)IRQC1_CONFIG_00 : (u32 *)IRQC0_CONFIG_00;
	reg += (pin & 0x1f);

	val = __raw_readl(reg) & ~0x80ff0000;
	__raw_writel(val | (timing << 16) | (1 << 31), reg);
}

#define SBAR2		__io(IO_ADDRESS(0xe6180060))
#define RESCNT2		__io(IO_ADDRESS(0xe6188020))

void u2evm_restart(char mode, const char *cmd)
{
	printk(KERN_INFO "%s\n", __func__);
	shmobile_do_restart(mode, cmd, APE_RESETLOG_U2EVM_RESTART);
}
int sec_rlte_hw_rev;

 
 
#define STBCHRB3		0xE6180043
/* SBSC register address */
#define SBSC_BASE		(0xFE000000U)
#define SBSC_SDMRA_DONE		(0x00000000)
#define SBSC_SDMRACR1A_ZQ	(0x0000560A)
#define CPG_PLL3CR		IO_ADDRESS(CPG_BASE + 0x00DC)
#define CPG_PLLECR		IO_ADDRESS(CPG_BASE + 0x00D0)
#define CPG_PLL3CR_1040MHZ	(0x27000000)
#define CPG_PLLECR_PLL3ST	(0x00000800)
static void __iomem *sbsc_sdmracr1a;

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

static void __init u2evm_init(void)
{
	char *cp=&boot_command_line[0];
	int ci;
	int pub_stm_select=-1;
	int stm_select=-1;	// Shall tell how to route STM traces.
				// Taken from boot_command_line[] parameters.
				// stm=# will set parameter, if '0' or '1' then as number, otherwise -1.
				// -1 = NONE, i.e. SDHI1 and SDHI0 are free for other functions.
				//  0 = SDHI0 used for STM traces. SD-Card not enabled.
				//  1 = SDHI1 used for STM traces. WLAN not enabled. [DEFAULT if stm boot para not defined]
	void __iomem *sbsc_sdmra_28200 = 0;
	void __iomem *sbsc_sdmra_38200 = 0;

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

	/* For case that Secure ISSW has selected debug mode already! */
#define DBGREG1		IO_ADDRESS(0xE6100020)

	printk("sec stm_select=%d\n", stm_select);

	/* pub_stm_select = stm_select;*/
	if (stm_select >= 0) { /* Only if Secure side allows debugging */
		if (cp[0] && cp[1] && cp[2] && cp[3] && cp[4]) {
			for (ci=4; cp[ci]; ci++) {
				if (cp[ci-4] == 's' &&
				    cp[ci-3] == 't' &&
				    cp[ci-2] == 'm' &&
				    cp[ci-1] == '=') {
					switch (cp[ci]) {
						case '0': pub_stm_select =  0; break;
						case '1': pub_stm_select =  1; break;
						default:  pub_stm_select = -1; stm_select = -1; break;
					}
					break;
 				}
 			}
 		}
 	}

	if (-1 == pub_stm_select) stm_select = -1;

	printk("pub_stm_select=%d\n", pub_stm_select);

	r8a73734_pinmux_init();
	r8a73734_add_standard_devices();

	r8a73734_hwlock_gpio = hwspin_lock_request_specific(SMGPIO);
	r8a73734_hwlock_cpg = hwspin_lock_request_specific(SMCPG);
	r8a73734_hwlock_sysc = hwspin_lock_request_specific(SMSYSC);
	pinmux_hwspinlock_init(r8a73734_hwlock_gpio);

#ifdef CONFIG_ARM_TZ
	if((system_rev & 0xFFFF) >= 0x3E12) /* ES2.02 and onwards */
	{
		printk(KERN_DEBUG "ES2.02 on later with TZ enabled\n");
		pub_stm_select = 0; /* Can't override secure \
				side by public side any more */
	} else {
		printk(KERN_DEBUG "ES2.01 or earlier or TZ enabled\n");
		if (stm_select != pub_stm_select) {
			stm_select = pub_stm_select;
			pub_stm_select = 1; /* Override secure \
					side by public side */
		} else {
			pub_stm_select = 0; /* Both secure and public agree.\
					No need to change HW setup */
		}
	}
#else
		printk(KERN_DEBUG "ES2.01 or earlier or TZ disabled\n");
		if (stm_select != pub_stm_select) {
			stm_select = pub_stm_select;
			pub_stm_select = 1; /* Override secure \
						side by public side */
		} else {
			pub_stm_select = 0; /* Both secure and public agree. \
						No need to change HW setup */
		}
#endif
		printk(KERN_DEBUG "final stm_select=%d\n", stm_select);


	if(((system_rev & 0xFFFF)>>4) >= 0x3E1)
	{

		*GPIO_DRVCR_SD0 = 0x0023;
		*GPIO_DRVCR_SIM1 = 0x0023;
		*GPIO_DRVCR_SIM2 = 0x0023;
	}
	shmobile_arch_reset = u2evm_restart;
	sec_rlte_hw_rev = check_sec_rlte_hw_rev();
	printk(KERN_INFO "%s hw rev : %d \n", __func__, sec_rlte_hw_rev);

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

	create_proc_read_entry("board_revision", 0444, NULL,
					u2_read_board_rev, NULL);

	/* SCIFA0 */
	gpio_request(GPIO_FN_SCIFA0_TXD, NULL);
	gpio_request(GPIO_FN_SCIFA0_RXD, NULL);

	/* SCIFB0 */
	gpio_request(GPIO_FN_SCIFB0_TXD, NULL);
	gpio_request(GPIO_FN_SCIFB0_RXD, NULL);
	gpio_request(GPIO_FN_SCIFB0_CTS_, NULL);
	gpio_request(GPIO_FN_SCIFB0_RTS_, NULL);

	// Config SCIFB0 with PU on RX and CTS pins
	*((volatile u8 *)0xE6050025) = 0x81;
	*((volatile u8 *)0xE6050026) = 0xC1;
	*((volatile u8 *)0xE6051089) = 0x81;
	*((volatile u8 *)0xE605108A) = 0xC1;

	gpio_pull_up_port(GPIO_PORT138); /* RX PU */
	gpio_pull_down_port(GPIO_PORT137); /* TX PD */
	gpio_pull_up_port(GPIO_PORT38); /* CTS PU */
	gpio_pull_down_port(GPIO_PORT37); /* RTS PD */


#ifdef CONFIG_KEYBOARD_SH_KEYSC
	/* enable KEYSC */
	gpio_request(GPIO_FN_KEYIN0, NULL);
	gpio_request(GPIO_FN_KEYIN1, NULL);
	gpio_request(GPIO_FN_KEYIN2, NULL);
	gpio_request(GPIO_FN_KEYIN3, NULL);
	gpio_request(GPIO_FN_KEYIN4, NULL);
	gpio_request(GPIO_FN_KEYIN5, NULL);
	gpio_request(GPIO_FN_KEYIN6, NULL);
//	gpio_request(GPIO_FN_KEYOUT0, NULL);
//	gpio_request(GPIO_FN_KEYOUT1, NULL);
//	gpio_request(GPIO_FN_KEYOUT2, NULL);
//	gpio_request(GPIO_FN_KEYOUT3, NULL);
//	gpio_request(GPIO_FN_KEYOUT4, NULL);
//	gpio_request(GPIO_FN_KEYOUT5, NULL);
//	gpio_request(GPIO_FN_KEYOUT6, NULL);


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
	
	/* Disable GPIO Enable at initialization */

	// MHL enable
	/*gpio_request(GPIO_PORT102, NULL);
	gpio_direction_output(GPIO_PORT102, 0);*/
	
	// ===== CWS GPIO =====
	
	// GPS Reset
	gpio_request(GPIO_PORT10, NULL);
	gpio_direction_output(GPIO_PORT10, 0);
	
	// GPS Enable
	gpio_request(GPIO_PORT11, NULL);
	gpio_direction_output(GPIO_PORT11, 0);

	// NFC Enable
	//gpio_request(GPIO_PORT12, NULL);
	//gpio_direction_output(GPIO_PORT12, 0);
#if defined(CONFIG_RENESAS_NFC)
	// NFC Firmware
	gpio_request(GPIO_PORT101, NULL);
	gpio_direction_output(GPIO_PORT101, 0);
#endif
	// WLAN Enable
	//gpio_request(GPIO_PORT260, NULL);
	//gpio_direction_output(GPIO_PORT260, 0);
	
	// BT Enable
	//gpio_request(GPIO_PORT268, NULL);
	//gpio_direction_output(GPIO_PORT268, 0);
	
	// GPS Enable
	/*gpio_request(GPIO_PORT11, NULL);
	gpio_direction_output(GPIO_PORT11, 0);*/
	
	// ===== Misc GPIO =====

	// MAIN MIC LDO Enable
	gpio_request(GPIO_PORT8, NULL);
	gpio_direction_output(GPIO_PORT8, 0);
	
	// Sensor LDO Enable
	gpio_request(GPIO_PORT9, NULL);
	gpio_direction_output(GPIO_PORT9, 0);

	// MHL enable
	//gpio_request(GPIO_PORT102, NULL);  /* commented as suggested by  */
	//gpio_direction_output(GPIO_PORT102, 0);

	/* End */

	gpio_direction_none_port(GPIO_PORT309);

	if (0 != stm_select) {
		/* If STM Traces go to SDHI1 or NOWHERE, then SDHI0 can be used for SD-Card */
		/* SDHI0 */
		gpio_request(GPIO_FN_SDHID0_0, NULL);
		gpio_request(GPIO_FN_SDHID0_1, NULL);
		gpio_request(GPIO_FN_SDHID0_2, NULL);
		gpio_request(GPIO_FN_SDHID0_3, NULL);
		gpio_request(GPIO_FN_SDHICMD0, NULL);
		gpio_request(GPIO_FN_SDHIWP0, NULL);
		gpio_direction_none_port(GPIO_PORT326);
		gpio_request(GPIO_FN_SDHICLK0, NULL);
		gpio_request(GPIO_PORT327, NULL);
		gpio_direction_input(GPIO_PORT327);
		irq_set_irq_type(irqpin2irq(50), IRQ_TYPE_EDGE_BOTH);
		gpio_set_debounce(GPIO_PORT327, 1000);	/* 1msec */
	}

#if 0
	/* ONLY FOR HSI CROSS COUPLING */
        /* TODO: Add HSI pinmux and direction etc control for X-coupling */
        /* TODO: CHECK if any conflict arises, whether X-coupling can be used also wit SHM and EOS Android */
	gpio_request(GPIO_FN_HSI_RX_FLAG, NULL);
 /* ... */
	gpio_requset(GPIO_FN_HSI_TX_WAKE, NULL);
#endif

	if (1 == stm_select) {
	/* FIRST, CONFIGURE STM CLK AND DATA PINMUX */

        /* SDHI1 used for STM Data, STM Clock, and STM SIDI */
//        gpio_request(GPIO_PORT288, NULL);
//        gpio_direction_output(GPIO_PORT288, 0);
        gpio_request(GPIO_FN_STMCLK_2, NULL);

//        gpio_request(GPIO_PORT289, NULL);
//        gpio_direction_output(GPIO_PORT289, 0);
        gpio_request(GPIO_FN_STMDATA0_2, NULL); 

//        gpio_request(GPIO_PORT290, NULL);
//        gpio_direction_output(GPIO_PORT290, 0);
        gpio_request(GPIO_FN_STMDATA1_2, NULL);

//        gpio_request(GPIO_PORT291, NULL);
//        gpio_direction_output(GPIO_PORT291, 0);
        gpio_request(GPIO_FN_STMDATA2_2, NULL);

//        gpio_request(GPIO_PORT292, NULL);
//        gpio_direction_output(GPIO_PORT292, 0);
        gpio_request(GPIO_FN_STMDATA3_2, NULL);

	} else if (0 == stm_select) {
	/* FIRST, CONFIGURE STM CLK AND DATA PINMUX using SDHI0 as port */

        /* SDHI0 used for STM Data, STM Clock */
//        gpio_request(GPIO_PORT326, NULL);
//        gpio_direction_output(GPIO_PORT326, 0);
        gpio_request(GPIO_FN_STMCLK_1, NULL);

//        gpio_request(GPIO_PORT320, NULL);
//        gpio_direction_output(GPIO_PORT320, 0);
        gpio_request(GPIO_FN_STMDATA0_1, NULL); 

//        gpio_request(GPIO_PORT321, NULL);
//        gpio_direction_output(GPIO_PORT321, 0);
        gpio_request(GPIO_FN_STMDATA1_1, NULL);

//        gpio_request(GPIO_PORT322, NULL);
//        gpio_direction_output(GPIO_PORT322, 0);
        gpio_request(GPIO_FN_STMDATA2_1, NULL);

//        gpio_request(GPIO_PORT323, NULL);
//        gpio_direction_output(GPIO_PORT323, 0);
        gpio_request(GPIO_FN_STMDATA3_1, NULL);

//        *PORTCR(324) = 0x03; //STMCMD0

	}


/*      Module function select register 3 (MSEL3CR/MSEL03CR)  at 0xE6058020 
 *        Write bit 28 up to enable SDHIx STMSIDI power
 *          bits [31:20] All 0, R, Reserved.
 *          bit  28      MSEL28, Initial value 0, R/W, IO power supply of terminal SDHI when SD is transmitted.
 *                       0=IO power OFF, 1=IO power ON
 *          bits [27:16] All 0, R, Reserved.
 *          bit  15      MSEL15, Initial value 0, R/W, Debug monitor function Setting.
 *                       0=Use KEYSC pins for debug monitor function.
 *                       1=Use BSC pins for debug monitor function.
 *          bits [14:4]  All 0, R, Reserved.
 *          bit  3       MSEL3, Initial value 0, R/W, IC_DP/IC_DM Output Enable Control.
 *                       0=Output Disable, 1=Depends on ICUSB Controller. Set 0 before "power down sequence".
 *          bit  2       0, R, Reserved.
 *          bits [1:0]   MSEL[1:0], Initial value 00, R/W, Select HSI.
 *                       0x=Internal connect Port xxx(HSI) shall set func0.
 *                       10=HSI0 select.
 *                       11=HSIB select.
 */

	/* SECOND, ENABLE TERMINAL POWER FOR STM CLK AND DATA PINS */


	__raw_writel(__raw_readl(MSEL3CR) | (1<<27), MSEL3CR); /* ES2.0: SIM powers */

	if (-1 != stm_select) {	
		__raw_writel(__raw_readl(MSEL3CR) | (1<<28), MSEL3CR);
	}


	/* THIRD, PINMUX STM SIDI (i,e, return channel) MUX FOR BB/MODEM */
	/* ALSO, CONFIGURE SYS-(TRACE) FUNNEL-STM, and SYS-TPIU-STM */

	if (1 == stm_select) {
	/* SDHI1 used for STMSIDI */
//        gpio_request(GPIO_PORT293, NULL);
//        gpio_direction_input(GPIO_PORT293);
        gpio_request(GPIO_FN_STMSIDI_2, NULL);
	gpio_pull_up_port(GPIO_PORT293);

	}
	
	if (0 == stm_select) {
        /* SDHI0 used for STMSIDI */
//        gpio_request(GPIO_PORT324, NULL);
//        gpio_direction_input(GPIO_PORT324);
        gpio_request(GPIO_FN_STMSIDI_1, NULL);
	gpio_pull_up_port(GPIO_PORT324);

	}

        if (-1 != stm_select) {
          int i;
          volatile unsigned long dummy_read;
#if 0 // NOT neede any more with new FIDO SW version Fido.1.9.5.36.edge_aligned_stpv2
	/* Lower CPG Frequency Control Register B (FRQCRB) ZTRFC clock by divider*/
	/*control because STM clock was 76.8MHZ, too high, now it is about 38.4MHz*/
	__raw_writel((__raw_readl(FRQCRB) & 0x7F0FFFFF) | 0x80400000, FRQCRB);
	/* Set KICK bit and set ZTRFC[3:0] to 0100, i.e. x 1/8 divider for System*/
	/*CPU Debugging and Trace Clock Frequenct Division Ratio*/
#endif

#define DBGREG9		IO_ADDRESS(0xE6100040)
	  if (pub_stm_select) {
		  __raw_writel(0x0000a501, DBGREG9); /* Key register */
		  __raw_writel(0x0000a501, DBGREG9); /* Key register, must write twice! */
	  }


          for(i=0; i<0x10; i++);

/* #define DBGREG1		IO_ADDRESS(0xE6100020) */ /* Defined already above */
	  if ((1 == stm_select) && pub_stm_select) {
          __raw_writel((__raw_readl(DBGREG1) & 0xFFDFFFFF) | (1<<20), DBGREG1);
		// Clear STMSEL[1], i.e. select STMSIDI to BB side.
		// Set   STMSEL[0], i.e. select SDHI1/STM*_2 as output/in port for STM
	  }

	  if ((0 == stm_select) && pub_stm_select) {
          __raw_writel((__raw_readl(DBGREG1) & 0xFFCFFFFF), DBGREG1);
		// Clear STMSEL[1], i.e. select STMSIDI to BB side.
		// Clear STMSEL[0], i.e. select SDHI0/STM*_1 as output/in port for STM
	  }

          for(i=0; i<0x10; i++);

#define SYS_TRACE_FUNNEL_STM_BASE	IO_ADDRESS(0xE6F8B000)
          /* Configure SYS-(Trace) Funnel-STM @ 0xE6F8B000 */
	  // TODO: check if delays and double writing really needed or not?
	  __raw_writel(0xc5acce55, SYS_TRACE_FUNNEL_STM_BASE + 0xFB0); // Lock Access
          for(i=0; i<0xF0; i++);
	  __raw_writel(     0x302, SYS_TRACE_FUNNEL_STM_BASE + 0x000); // Enable only Slave port 1, i.e. Modem top-level funnel for STM, 0x303 for APE also
          for(i=0; i<0xF0; i++);
	  __raw_writel(0xc5acce55, SYS_TRACE_FUNNEL_STM_BASE + 0xFB0); // Lock Access
          for(i=0; i<0x10; i++);
	  __raw_writel(     0x302, SYS_TRACE_FUNNEL_STM_BASE + 0x000); // Enable only Slave port 1, i.e. Modem top-level funnel for STM, 0x303 for APE also
          for(i=0; i<0xF0; i++);
        
          /* Configure SYS-TPIU-STM @ 0xE6F8A000 */
#define SYS_TPIU_STM_BASE	IO_ADDRESS(0xE6F8A000)
	  __raw_writel(0xc5acce55, SYS_TPIU_STM_BASE + 0xFB0); // Lock Access
	  __raw_writel(       0x8, SYS_TPIU_STM_BASE + 0x004); // 0x8 means Current Port Size 4-bits wide (TRACEDATA0-3 all set)
	  __raw_writel(     0x112, SYS_TPIU_STM_BASE + 0x304); // Formatter and Flush control
          dummy_read = __raw_readl(SYS_TPIU_STM_BASE + 0x304); // Formatter and Flush control
	  __raw_writel(     0x162, SYS_TPIU_STM_BASE + 0x304); // Formatter and Flush control
          dummy_read = __raw_readl(SYS_TPIU_STM_BASE + 0x304); // Formatter and Flush control
#if 0 // STM Walking ones test mode, only for testing timing, not for normal trace operation!
	  __raw_writel(0x00020001, SYS_TPIU_STM_BASE + 0x204); // STM Walking ones test mode
#endif
        }


#ifdef CONFIG_U2_STM_ETR_TO_SDRAM
        if (1) {
                int i;
                /*
                EOS2 Modem STM Trace to SDRAM through ETR -- Configuration in Short
                ===================================================================
                SUMMARY OF MODEM STM TRACE FLOW, CONFIGURATION IN REVERSE ORDER:
                ----------------------------------------------------------------
                1) Modem   CoreSight / WGEM STM          @ inside WGEM  - Enable traces
                2) System  CoreSight / SYS Funnel STM    @ 0xE6F 8B 000 - Enable Port #1 "From STM-ATB Modem"
                3) System  CoreSight / SYS Trace Funnel  @ 0xE6F 84 000 - Enable Port #2 "From Sys-Funnel-STM"
                4) HostCPU CoreSight / CPU Trace Funnel  @ 0xE6F A4 000 - Enable Port #4 "From Sys-Trace-Funnel"
                5) HostCPU CoreSight / ETF               @ 0xE6F A1 000 - configure FIFO mode
                6) HostCPU CoreSight / ETR configuration @ 0xE6F A5 000 - configure Circular buffer mode, SDRAM write buffer size and start address, etc.
                7) System  CoreSight / SYS-TPIU-STM      @ 0xE6F 8A 000 - set to 32-bit mode to avoid unnecessary stall
                8) HostCPU CoreSight / CPU-TPIU          @ 0xE6F A3 000 - set to 32-bit mode to avoid unnecessary stall
                9) System  CoreSight / SYS-TPIU          @ 0xE6F 83 000 - set to 32-bit mode to avoid unnecessary stall

                DETAILED CONFIGURATION REGISTER WRITES:
                ---------------------------------------
                */

                __raw_writel(0x0000a501, DBGREG9); /* Key register */
                __raw_writel(0x0000a501, DBGREG9); /* Key register, must write twice! */


                /* <<<<<< - 9 - System CoreSight  / SYS-TPIU     to 32-bit mode >>>>>> */

#define SYS_TPIU_BASE       IO_ADDRESS(0xE6F83000)
                wait_for_coresight_access_lock(SYS_TPIU_BASE);
#if 1
                __raw_writel((1<<(16-1)), SYS_TPIU_BASE + 0x004);               /* Current Port Size 4-bits wide to avoid stall */
#else
                __raw_writel((1<<(32-1)), SYS_TPIU_BASE + 0x004);               /* Current Port Size 32-bits wide to avoid stall */
#endif
       
                /* <<<<<< - 8 - HostCPU CoreSight / CPU-TPIU     to 32-bit mode >>>>>> */

#define CPU_TPIU_BASE       IO_ADDRESS(0xE6FA3000)
                wait_for_coresight_access_lock(CPU_TPIU_BASE);
#if 1
                __raw_writel((1<<(16-1)), CPU_TPIU_BASE + 0x004);               /* Current Port Size 16-bits wide to avoid stall */
#else
                __raw_writel((1<<(32-1)), CPU_TPIU_BASE + 0x004);               /* Current Port Size 32-bits wide to avoid stall */
#endif
                /* <<<<<< - 7 - System CoreSight  / SYS-TPIU-STM to 32-bit mode >>>>>> */

#define SYS_TPIU_STM_BASE       IO_ADDRESS(0xE6F8A000)
                wait_for_coresight_access_lock(SYS_TPIU_STM_BASE);
#if 1
                __raw_writel((1<<(4-1)), SYS_TPIU_STM_BASE + 0x004);    /* Current Port Size 16-bits wide to avoid stall */
#else
                __raw_writel((1<<(32-1)), SYS_TPIU_STM_BASE + 0x004);   /* Current Port Size 32-bits wide to avoid stall */
#endif

                /* <<<<<< - 6 - HostCPU CoreSight / ETR configuration >>>>>>
                For ARM Specification of this HW block, see CoreSight Trace Memory Controller Technical Reference Manual
                SW Registers of ETR are same as ETF in different HW configuration
                */

#define CPU_ETR_BASE       IO_ADDRESS(0xE6FA5000)

                wait_for_coresight_access_lock(CPU_ETR_BASE);
                __raw_writel(0, CPU_ETR_BASE + 0x020);                  /* CTL Control: 0 */
                __raw_writel(0, CPU_ETR_BASE + 0x028);                  /* MODE: Circular buffer */
                __raw_writel(3, CPU_ETR_BASE + 0x304);                  /* FFCR: Formatting enabled */

                __raw_writel(
                                                (       (3 << 8) |              /*    WrBurstLen, 0 = 1, 1 = 2, ..., 15 = 16     */
                                                        (0 << 7) |              /*    0 = Single buffer, 1 = ScatterGather       */
                                                        (0 << 6) |              /*    Reserved                                   */
                                                        (0 << 5) |              /*    CacheCtrlBit3 No write alloc / write alloc */
                                                        (0 << 4) |              /*    CacheCtrlBit2 No read alloc / read alloc   */
                                                        (1 << 3) |              /*    CacheCtrlBit1 Non-cacheable  / Cacheable   */
                                                        (1 << 2) |              /*    CacheCtrlBit0 Non-bufferable / Bufferable  */
                                                        (1 << 1) |              /*    ProtCtrlBit1  Secure / Non-secure          */
                                                        (1 << 0)                /*    ProtCtrlBit0  Normal / Privileged          */
                                                ),
                                        CPU_ETR_BASE + 0x110); /* AXICTL: Set as commented above */

                __raw_writel(0, CPU_ETR_BASE + 0x034);                  /* BUFWM Buffer Level Water Mark: 0 */
                __raw_writel(0, CPU_ETR_BASE + 0x018);                  /* RWP RAM Writer Pointer: 0 */
                __raw_writel(0, CPU_ETR_BASE + 0x03C);                  /* RWP RAM Writer Pointer High: 0 */
                __raw_writel(0x45801000, CPU_ETR_BASE + 0x118);         /* DBALO Data Buffer Address Low: 0x 4580 10000 */
                __raw_writel(0, CPU_ETR_BASE + 0x11C);                  /* DBAHI Data Buffer Address High: 0 */
                __raw_writel(((39*1024*1024  + 764*1024)/ 4), CPU_ETR_BASE + 0x004); /* RSZ RAM Size Register: 39MB + 764 kB */
                __raw_writel(1, CPU_ETR_BASE + 0x020);                  /* CTL Control: 1 */

                /* <<<<<< - 5 - HostCPU CoreSight / ETF - configuration to FIFO mode >>>>>>
                For ARM Specification of this HW block, see CoreSight Trace Memory Controller Technical Reference Manual
                */

#define CPU_ETF_BASE       IO_ADDRESS(0xE6FA1000)
                wait_for_coresight_access_lock(CPU_ETF_BASE);
                __raw_writel(0, CPU_ETF_BASE + 0x020);                  /* CTL Control: TraceCaptEn OFF ==> Disabled */
                __raw_writel(2, CPU_ETF_BASE + 0x028);                  /* MODE: FIFO */
                __raw_writel(3, CPU_ETF_BASE + 0x304);                  /* FFCR Formatter and Flush Control Register: Formatting enabled */
                __raw_writel(0, CPU_ETF_BASE + 0x034);                  /* BUFWM Buffer Level Water Mark: 0 */
                __raw_writel(1, CPU_ETF_BASE + 0x020);                  /* CTL Control: TraceCaptEn ON ==> Running */

                /* <<<<<< - 4 - HostCPU CoreSight / CPU Trace Funnel - Enable Port #3 "From Sys-Trace-Funnel" >>>>>> */

#define CPU_TRACE_FUNNEL_BASE       IO_ADDRESS(0xE6FA4000)
                wait_for_coresight_access_lock(CPU_TRACE_FUNNEL_BASE);
                __raw_writel((0x300 | (1<<4)), CPU_TRACE_FUNNEL_BASE + 0x000);  /* Enable only Slave port 4, i.e. From Sys-Trace-Funnel */

                /* <<<<<< - 3 - System CoreSight / SYS Trace Funnel - Enable Port #2 "From Sys-Funnel-STM" >>>>>> */

#define SYS_TRACE_FUNNEL_BASE       IO_ADDRESS(0xE6F84000)
                wait_for_coresight_access_lock(SYS_TRACE_FUNNEL_BASE);
                __raw_writel((0x300 | (1<<2)), SYS_TRACE_FUNNEL_BASE + 0x000);  // Enable only Slave port 2, i.e. From Sys-Funnel-STM

                /* <<<<<< - 2 - System CoreSight / SYS Funnel STM - Enable Port #1 "From STM-ATB Modem" >>>>>> */

#define SYS_TRACE_FUNNEL_STM_BASE       IO_ADDRESS(0xE6F8B000)
                wait_for_coresight_access_lock(SYS_TRACE_FUNNEL_STM_BASE);
                __raw_writel((0x300 | (1<<1)), SYS_TRACE_FUNNEL_STM_BASE + 0x000);      /* Enable only Slave port 1, i.e. Modem top-level funnel for STM */

                /* <<<<<< - 1 - Modem CoreSight / WGEM STM - Enable traces >>>>>>
                This happens inside WGEM L2 TCM vector boot code
                */
        }
#endif /* CONFIG_U2_STM_ETR_TO_SDRAM */

	if (1 != stm_select) {
		/* SDHI1 */
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
		// move gpio request to board-renesas_wifi.c
		
		/* WLAN Init API call */
#ifdef CONFIG_BRCM_UNIFIED_DHD_SUPPORT
		printk(KERN_ERR "Calling WLAN_INIT!\n");

	 	renesas_wlan_init();
		printk(KERN_ERR "DONE WLAN_INIT!\n");
#endif	
		/* add the SDIO device */
		//board_add_sdio_devices();
	}

	/* touch key Interupt */
	gpio_request(GPIO_PORT104, NULL);
	gpio_direction_input(GPIO_PORT104);

#if 0
	if (u2_get_board_rev() >= 5) {		// PORT227(CS0_N) pin is not connected.
#ifndef CONFIG_MFD_D2153		// PORT227(CS0_N) pin is not connected.
#endif
		/* emmc ldo enable*/
		gpio_request(GPIO_PORT227, NULL);
		gpio_direction_output(GPIO_PORT227, 1);
#if 0
#endif
	}
#endif


	gpio_pull_up_port(GPIO_PORT104);

	/* I2C */
	gpio_request(GPIO_FN_I2C_SCL0H, NULL);
	gpio_request(GPIO_FN_I2C_SDA0H, NULL);

	if (u2_board_rev >= 4) {
		gpio_pull_off_port(GPIO_PORT84);
		gpio_pull_off_port(GPIO_PORT85);
	}

	gpio_request(GPIO_FN_I2C_SCL1H, NULL);
	gpio_request(GPIO_FN_I2C_SDA1H, NULL);

	/* PMIC */
	gpio_request(GPIO_PORT0, NULL);	/* MSECURE */
	gpio_direction_output(GPIO_PORT0, 1);
	gpio_request(GPIO_PORT28, NULL);
	gpio_direction_input(GPIO_PORT28);

	if (u2_get_board_rev() >= 5) {
#if defined(CONFIG_MFD_D2153)
		irq_set_irq_type(irqpin2irq(28), IRQ_TYPE_LEVEL_LOW);
#endif /* CONFIG_MFD_D2153 */
	} else {
#if defined(CONFIG_PMIC_INTERFACE)
		irq_set_irq_type(irqpin2irq(28), IRQ_TYPE_EDGE_FALLING);
#else  /* CONFIG_PMIC_INTERFACE */
		irq_set_irq_type(irqpin2irq(28), IRQ_TYPE_LEVEL_LOW);
#endif /* CONFIG_PMIC_INTERFACE */
	}

#if defined(CONFIG_USB_SWITCH_TSU6712)
   gpio_request(GPIO_PORT97, NULL);
   gpio_direction_input(GPIO_PORT97);
	gpio_pull_up_port(GPIO_PORT97);
#endif

#if defined(CONFIG_CHARGER_SMB328A)
	if(SEC_RLTE_REV0_2_2 == sec_rlte_hw_rev)
	{
	   gpio_request(GPIO_PORT103, NULL);
	   gpio_direction_input(GPIO_PORT103);
	   gpio_pull_up_port(GPIO_PORT103);
	}
	else
	{
	   gpio_request(GPIO_PORT19, NULL);
	   gpio_direction_input(GPIO_PORT19);
	   gpio_pull_up_port(GPIO_PORT19);
	}
#endif

#if defined(CONFIG_BATTERY_BQ27425)
   gpio_request(GPIO_PORT105, NULL);
   gpio_direction_input(GPIO_PORT105);
	gpio_pull_up_port(GPIO_PORT105);
#endif

#if 0
	/* Ethernet */
	gpio_request(GPIO_PORT97, NULL);
	gpio_direction_input(GPIO_PORT97); /* for IRQ */
	gpio_request(GPIO_PORT105, NULL);
	gpio_direction_output(GPIO_PORT105, 1); /* release NRESET */
#endif	
		/*TSP LDO Enable*/
	gpio_request(GPIO_PORT30, NULL);
	if(u2_board_rev >= 4)
		gpio_direction_output(GPIO_PORT30, 0);
	else
		gpio_direction_output(GPIO_PORT30, 1);
	/* Touch */
	gpio_request(GPIO_PORT32, NULL);
	gpio_direction_input(GPIO_PORT32);
	if (u2_get_board_rev() >= 4)
		gpio_pull_off_port(GPIO_PORT32);
	else
		gpio_pull_up_port(GPIO_PORT32);

	USBGpio_init();

#ifdef CONFIG_SPI_SH_MSIOF
	/* enable MSIOF0 */
	gpio_request(GPIO_FN_MSIOF0_TXD, NULL);
	gpio_request(GPIO_FN_MSIOF0_SYNC, NULL);
	gpio_request(GPIO_FN_MSIOF0_SCK, NULL);
	gpio_request(GPIO_FN_MSIOF0_RXD, NULL);
#endif

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

#ifdef CONFIG_CACHE_L2X0
	/*
	 * [30] Early BRESP enable
	 * [27] Non-secure interrupt access control
	 * [26] Non-secure lockdown enable
	 * [22] Shared attribute override enable
	 * [19:17] Way-size: b010 = 32KB
	 * [16] Accosiativity: 0 = 8-way
	 */
	if((system_rev & 0xFFFF) == 0x3E00)
	{
		l2x0_init(__io(IO_ADDRESS(0xf0100000)), 0x4c440000, 0x820f0fff);
	}
	else if(((system_rev & 0xFFFF)>>4) >= 0x3E1)
	{
		/*The L2Cache is resized to 512 KB*/
		l2x0_init(__io(IO_ADDRESS(0xf0100000)), 0x4c460000, 0x820f0fff);
	}
#endif

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
	if((system_rev & 0xFFFF) == 0x3E00) {
		printk(KERN_ALERT "Camera ISP ES version switch (ES1)\n");
		csi21_device.resource = csi21_resources_es1;
		csi21_device.num_resources = ARRAY_SIZE(csi21_resources_es1);
		csi21_info.flags |= SH_CSI2_MULTI;
		csi21_info.cmod_name = csi20_info.cmod_name;
		rcu0_device.resource = rcu0_resources_es1;
		rcu1_device.resource = rcu1_resources_es1;
		rcu1_device.num_resources = ARRAY_SIZE(rcu1_resources_es1);
		sh_mobile_rcu1_info.mod_name = sh_mobile_rcu0_info.mod_name;
	} else if(((system_rev & 0xFFFF)>>4) >= 0x3E1)
		printk(KERN_ALERT "Camera ISP ES version switch (ES2)\n");
	if ((1 != u2_get_board_rev()) && (2 != u2_get_board_rev()) &&
		(3 != u2_get_board_rev())) {
		csi20_clients[0].lanes = 0x3;
		camera_devices[0].dev.platform_data = &camera_links_rev4[0];
		camera_devices[1].dev.platform_data = &camera_links_rev4[1];
		camera_links_rev4[0].priv = &csi20_info;
		camera_links_rev4[1].priv = &csi21_info;
	}
}

#if 0
	gpio_request(GPIO_PORT39, NULL);
	gpio_direction_output(GPIO_PORT39, 0);
#endif
	if(u2_get_board_rev() >= 5) {
		gpio_key_init(stm_select,
			u2_board_rev,
			sec_rlte_hw_rev,
			u2evm_devices_stm_sdhi0_d2153,
			ARRAY_SIZE(u2evm_devices_stm_sdhi0_d2153),
			u2evm_devices_stm_sdhi1_d2153,
			ARRAY_SIZE(u2evm_devices_stm_sdhi1_d2153),
			u2evm_devices_stm_none_d2153,
			ARRAY_SIZE(u2evm_devices_stm_none_d2153));
	} else {
		gpio_key_init(stm_select,
			u2_board_rev,
			sec_rlte_hw_rev,
			u2evm_devices_stm_sdhi0,
			ARRAY_SIZE(u2evm_devices_stm_sdhi0),
			u2evm_devices_stm_sdhi1,
			ARRAY_SIZE(u2evm_devices_stm_sdhi1),
			u2evm_devices_stm_none,
			ARRAY_SIZE(u2evm_devices_stm_none));
	}

#if defined(CONFIG_MPU_SENSORS_MPU6050B1)
	mpl_init();
#endif

	if(u2_get_board_rev() < 4)
		touchkey_init_hw();

#if defined (CONFIG_SAMSUNG_MHL)
	board_mhl_init();
#endif
	if(u2_get_board_rev() >= 5) {
		i2c_register_board_info(0, i2c0_devices_d2153, ARRAY_SIZE(i2c0_devices_d2153));
	} else {
		i2c_register_board_info(0, i2c0_devices, ARRAY_SIZE(i2c0_devices));
	}
	i2c_register_board_info(2, i2c2_devices, ARRAY_SIZE(i2c2_devices));
	i2c_register_board_info(3, i2c3_devices, ARRAY_SIZE(i2c3_devices));
	i2c_register_board_info(4, i2c4_devices, ARRAY_SIZE(i2c4_devices));
	if(u2_get_board_rev() >= 5) {
		i2c_register_board_info(6, i2cm_devices_d2153, ARRAY_SIZE(i2cm_devices_d2153));
	}else{
		i2c_register_board_info(6, i2cm_devices, ARRAY_SIZE(i2cm_devices));
	}

	/* GPS Init */
#if defined(CONFIG_RENESAS_GPS)
	gps_gpio_init();
#endif
	if(u2_get_board_rev() < 4)
		touchkey_i2c_register_board_info(10);
platform_add_devices(gpio_i2c_devices, ARRAY_SIZE(gpio_i2c_devices));	
	#if defined(CONFIG_VIBRATOR_ISA1000A)
    isa1000_vibrator_init();
#endif

#if defined(CONFIG_SEC_CHARGING_FEATURE)
	spa_power_init();
#endif
	printk(KERN_DEBUG "%s\n", __func__);
	crashlog_r_local_ver_write(mmcoops_info.soft_version);
	crashlog_reset_log_write();
	crashlog_init_tmplog();

	if (u2_get_board_rev() <= 4) {
		i2c_register_board_info(9, i2c9gpio_devices, ARRAY_SIZE(i2c9gpio_devices));
	}
#if defined(CONFIG_RENESAS_NFC)
#ifdef CONFIG_PN544_NFC
	i2c_register_board_info(8, pn544_info, ARRAY_SIZE(pn544_info)); 
#endif
#endif
}

static void __init u2evm_timer_init(void)
{
	r8a73734_clock_init();
	shmobile_timer.init();
#ifdef ARCH_HAS_READ_CURRENT_TIMER
	if (!setup_current_timer())
		set_delay_fn(read_current_timer_delay_loop);
#endif
}

struct sys_timer u2evm_timer = {
	.init	= u2evm_timer_init,
};


static void __init u2evm_reserve(void)
{
	int i;
	int ret;

	for (i = 0; i < u2evm_ion_data.nr; i++) {
		if (u2evm_ion_data.heaps[i].type == ION_HEAP_TYPE_CARVEOUT) {
			ret = memblock_remove(u2evm_ion_data.heaps[i].base,
					      u2evm_ion_data.heaps[i].size);
			if (ret)
				pr_err("memblock remove of %x@%lx failed\n",
				       u2evm_ion_data.heaps[i].size,
				       u2evm_ion_data.heaps[i].base);
		}
	}
}

static int check_sec_rlte_hw_rev(void)
{
	int rev0, rev1, rev2, rev3;

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

	return (rev3 << 3 | rev2 << 2 | rev1 << 1 | rev0);
}

MACHINE_START(U2EVM, "u2evm")
	.map_io		= u2evm_map_io,
	.init_irq	= u2evm_init_irq,
	.handle_irq	= shmobile_handle_irq_gic,
	.init_machine	= u2evm_init,
	.timer		= &u2evm_timer,
	.restart	= u2evm_restart,
	.reserve	= u2evm_reserve,
MACHINE_END
