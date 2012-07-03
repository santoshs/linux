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

#ifdef CONFIG_PMIC_INTERFACE
#include <linux/pmic/pmic.h>
#endif

#include <linux/mfd/tps80031.h>
#include <linux/spi/sh_msiof.h>
#include <linux/i2c/atmel_mxt_ts.h>
#include <linux/regulator/tps80031-regulator.h>
#include <linux/usb/r8a66597.h>
#include <linux/ion.h>
#include <linux/memblock.h>
#include <sound/sh_fsi.h>
#include <linux/tpu_pwm.h>
#include <linux/tpu_pwm_board.h>
#include <linux/pcm2pwm.h>
#include <linux/thermal_sensor/ths_kernel.h>
#include <media/sh_mobile_rcu.h>
#include <media/soc_camera.h>
#include <media/soc_camera_platform.h>
#include <media/sh_mobile_csi2.h>
#include <linux/sh_clk.h>
#include <media/v4l2-subdev.h>
#include <linux/pmic/pmic-ncp6914.h>
#include <media/isx012.h>

#ifdef CONFIG_KEYBOARD_CYPRESS_TOUCH
#include <linux/i2c/touchkey_i2c.h>
#endif

#include <linux/mmcoops.h>

#define CLASHLOG_R_LOCAL_VER_LOCATE		0x4C801000
#define CLASHLOG_R_LOCAL_VER_LENGTH       	32

#define CRASHLOG_KMSG_LOCATE			0x4C801020
#define CRASHLOG_LOGCAT_MAIN_LOCATE		0x4C801030
#define CRASHLOG_LOGCAT_EVENT_LOCATE	0x4C801040
#define CRASHLOG_LOGCAT_RADIO_LOCATE	0x4C801050
#define CRASHLOG_LOGCAT_SYSTEM_LOCATE	0x4C801060

#define TMPLOG_ADDRESS 0x4C821200
#define TMPLOG_SIZE    0x00040000
#define RMC_LOCAL_VERSION "150612"		// ddmmyy (release time)
char *tmplog_nocache_address = NULL;

static void crashlog_r_local_ver_write(void);
static void crashlog_reset_log_write(void);
static void crashlog_init_tmplog(void); 

#define SRCR2		IO_ADDRESS(0xe61580b0)
#define SRCR3		IO_ADDRESS(0xe61580b8)

#define GPIO_PULL_OFF	0x00
#define GPIO_PULL_DOWN	0x80
#define GPIO_PULL_UP	0xc0

#define SDHI1_CLK_CR IO_ADDRESS(0xE6052120)
#define SDHI1_D0_CR IO_ADDRESS(0xE6052121)
#define SDHI1_D1_CR IO_ADDRESS(0xE6052122)
#define SDHI1_D2_CR IO_ADDRESS(0xE6052123)
#define SDHI1_D3_CR IO_ADDRESS(0xE6052124)
#define SDHI1_CMD_CR IO_ADDRESS(0xE6052125)

/*Support for compatibility between ES1.0 and ES2.0*/
#define GPIO_BASE	IO_ADDRESS(0xe6050000)
#define GPIO_PORTCR_ES1(n)	({				\
	((n) <  96) ? (GPIO_BASE + 0x0000 + (n)) :	\
	((n) < 128) ? (GPIO_BASE + 0x1000 + (n)) :	\
	((n) < 144) ? (GPIO_BASE + 0x1000 + (n)) :	\
	((n) < 192) ? 0 :				\
	((n) < 320) ? (GPIO_BASE + 0x2000 + (n)) :	\
	((n) < 328) ? (GPIO_BASE + 0x3000 + (n)) : 0; })

#define GPIO_PORTCR_ES2(n)	({				\
	((n) <  96) ? (GPIO_BASE + 0x0000 + (n)) :	\
	((n) < 128) ? (GPIO_BASE + 0x0000 + (n)) :	\
	((n) < 144) ? (GPIO_BASE + 0x1000 + (n)) :	\
	((n) < 192) ? 0 :				\
	((n) < 320) ? (GPIO_BASE + 0x2000 + (n)) :	\
	((n) < 328) ? (GPIO_BASE + 0x2000 + (n)) : 0; })

#define ENT_TPS80031_IRQ_BASE	(IRQPIN_IRQ_BASE + 64)

static void gpio_pull(u32 addr, int type)
{
	u8 data = __raw_readb(addr);

	data &= ~0xc0;
	data |= type;

	__raw_writeb(data, addr);
}

static DEFINE_SPINLOCK(io_lock);//for modify register

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

#ifdef CONFIG_KEYBOARD_SH_KEYSC
/* KEYSC */
static struct sh_keysc_info keysc_platdata = {
	.mode		= SH_KEYSC_MODE_6,
	.scan_timing	= 3,
	.delay		= 100,
	.wakeup		= 1,	
	.automode	= 1,
	.flags		= WA_EOS_E132_KEYSC,
	.keycodes	= {
		227, KEY_0, 228,
		0, 0, 0, 0, 0,
		KEY_7, KEY_8, KEY_9,
		0, KEY_DOWN, 0, 0, 0,
		KEY_4, KEY_5, KEY_6,
		KEY_LEFT, KEY_ENTER, KEY_RIGHT, 0, 0,
		KEY_1, KEY_2, KEY_3,
		0, KEY_UP, 0, 0, 0,
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
		.start	= gic_spi(101),
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

void (*shmobile_arch_reset)(char mode, const char *cmd);

static int is_vbus_powered(void) {
	int val = 0;
	int val1 = 0;
	int count = 10;

	/* Extract bit VBSTS in INTSTS0 register */
	val = __raw_readw(IO_ADDRESS(0xE6890040)) & 0x80;

	while (--count){
		msleep(1);
		val1 = __raw_readw(IO_ADDRESS(0xE6890040)) & 0x80;
		if (val != val1)
		{
			count = 10;
			val = val1;
		}
	}

	printk ("Value of Status register INTSTS0: %x \n", __raw_readw(IO_ADDRESS(0xE6890040)));
	printk("VBUS val = %d\n", val1);

	return val1>>7;

}

#define PHYFUNCTR	IO_ADDRESS(0xe6890104) /* 16-bit */

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
#ifdef CONFIG_USB_OTG
#define SYSSTS	IO_ADDRESS(0xe6890004) /* 16-bit */
#define PHYOTGCTR	IO_ADDRESS(0xe689010a) /* 16-bit */
	__raw_writew(__raw_readw(PHYOTGCTR) | (1 << 8), PHYOTGCTR); /* IDPULLUP */
	msleep(50);
#endif
}

static struct r8a66597_platdata usbhs_func_data = {
	.is_vbus_powered = is_vbus_powered,
	.module_start	= usbhs_module_reset,
	.on_chip	= 1,
	.buswait	= 5,
	.max_bufnum	= 0xff,
#ifdef CONFIG_PMIC_INTERFACE
	.vbus_irq	= ENT_TPS80031_IRQ_BASE + TPS80031_INT_VBUS,
#else
	.vbus_irq	= ENT_TPS80031_IRQ_BASE + TPS80031_INT_VBUS_DET,
#endif
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
#ifdef CONFIG_USB_R8A66597_HCD
static void usb_host_port_power(int port, int power)
{
#ifdef CONFIG_PMIC_INTERFACE
	if (power) {
		pmic_set_vbus(1);
	} else {
		pmic_set_vbus(0);
	}
#endif
}
static struct r8a66597_platdata usb_host_data = {
	.module_start	= usbhs_module_reset,
	.on_chip = 1,
	.port_power = usb_host_port_power,
};

static struct resource usb_host_resources[] = {
	[0] = {
		.name	= "USBHS",
		.start	= 0xe6890000,
		.end	= 0xe689014b,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= gic_spi(87), /* USBULPI */
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device usb_host_device = {
	.name		= "r8a66597_hcd",
	.id		= 0,
	.dev = {
		.platform_data		= &usb_host_data,
		.dma_mask		= NULL,
		.coherent_dma_mask	= 0xffffffff,
	},
	.num_resources	= ARRAY_SIZE(usb_host_resources),
	.resource	= usb_host_resources,
};
#endif /*CONFIG_USB_R8A66597_HCD*/
#ifdef CONFIG_USB_OTG
/*TUSB1211 OTG*/
static struct resource tusb1211_resource[] = {
	[0] = {
		.name	= "tusb1211_resource",
		.start	= 0xe6890000,
		.end	= 0xe689014b,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.name	= "INT_ID",
		.start	= ENT_TPS80031_IRQ_BASE + TPS80031_INT_ID,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device tusb1211_device = {
	.name = "tusb1211_driver",
	.id = 0,
	.num_resources = ARRAY_SIZE (tusb1211_resource),
	.resource = tusb1211_resource,
};
#endif /*CONFIG_USB_OTG*/

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
			  MMC_CAP_1_8V_DDR | MMC_CAP_UHS_DDR50 |
			  MMC_CAP_NONREMOVABLE,
	.dma		= &sh_mmcif_dma,
	.max_clk	= 25000000,
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
	.local_version	= MMCOOPS_LOCAL_VERSION,
	.soft_version	= RMC_LOCAL_VERSION,
	/*512 byte blocks */
	.start		= MMCOOPS_START_OFFSET,
	.size		= MMCOOPS_LOG_SIZE
};

static struct platform_device mmcoops_device = {
	.name   = "mmcoops",
	.dev    = {
		.platform_data  = &mmcoops_info,
	},
};

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

static struct platform_device sdhi0_device = {
	.name		= "renesas_sdhi",
	.id		= 0,
	.dev		= {
		.platform_data	= &sdhi0_info,
	},
	.num_resources	= ARRAY_SIZE(sdhi0_resources),
	.resource	= sdhi0_resources,
};

#if 0

static void sdhi1_set_pwr(struct platform_device *pdev, int state)
{
   static int power_state;

   printk("Powering %s wifi\n", (state ? "on" : "off"));

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
   return 1;//return gpio_get_value(GPIO_PORT327) ? 0 : 1;
}

static struct renesas_sdhi_dma sdhi1_dma = {
	.chan_tx = {
		.slave_id	= SHDMA_SLAVE_SDHI1_TX,
	},
	.chan_rx = {
		.slave_id	= SHDMA_SLAVE_SDHI1_RX,
	}
};

#define SDHI1_VOLTAGE  MMC_VDD_165_195 | MMC_VDD_20_21 | MMC_VDD_21_22 | MMC_VDD_22_23 | MMC_VDD_23_24 | MMC_VDD_24_25 | MMC_VDD_25_26 | MMC_VDD_26_27 | MMC_VDD_27_28 | MMC_VDD_28_29 | MMC_VDD_29_30 | MMC_VDD_30_31 | MMC_VDD_31_32 | MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_34_35 | MMC_VDD_35_36 

static struct renesas_sdhi_platdata sdhi1_info = {
	.caps		= MMC_CAP_SDIO_IRQ | MMC_CAP_POWER_OFF_CARD | MMC_CAP_NONREMOVABLE | MMC_PM_KEEP_POWER,
	.flags		= RENESAS_SDHI_SDCLK_OFFEN | RENESAS_SDHI_WP_DISABLE,
 	.dma		= &sdhi1_dma,
 	.set_pwr	= sdhi1_set_pwr,
	.detect_irq	= 0, 
	.detect_msec	= 0,
	.get_cd		= sdhi1_get_cd,
	.ocr		= MMC_VDD_165_195,//SDHI1_VOLTAGE,
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

static void sdhi2_set_pwr(struct platform_device *pdev, int state)
{
	;
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
	.ocr		= MMC_VDD_165_195, //SDHI2_VOLTAGE,
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

static struct sh_fsi_platform_info fsi_info = {
	.port_flags = SH_FSI_OUT_SLAVE_MODE |
	              SH_FSI_IN_SLAVE_MODE	|
		          SH_FSI_BRS_INV		|
		          SH_FSI_OFMT(I2S)		|
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



#define GPIO_KEY(c, g, d) \
	{.code=c, .gpio=g, .desc=d, .wakeup=0, .active_low=1,\
	 .debounce_interval=20}

static struct gpio_keys_button gpio_buttons[] = {
#ifndef CONFIG_PMIC_INTERFACE
	GPIO_KEY(KEY_POWER, GPIO_PORT24, "Power"),
#endif
#if 0
	GPIO_KEY(KEY_MENU, GPIO_PORT25, "Menu"),
	GPIO_KEY(KEY_HOMEPAGE, GPIO_PORT26, "Home"),
	GPIO_KEY(KEY_BACK, GPIO_PORT27, "Back"),
	GPIO_KEY(KEY_VOLUMEUP, GPIO_PORT1, "+"),
	GPIO_KEY(KEY_VOLUMEDOWN, GPIO_PORT2, "-"),
#endif
	GPIO_KEY(KEY_HOMEPAGE, GPIO_PORT45, "Home"),
	GPIO_KEY(KEY_VOLUMEUP, GPIO_PORT46, "+"),
	GPIO_KEY(KEY_VOLUMEDOWN, GPIO_PORT47, "-"),
};

static int gpio_key_enable(struct device *dev)
{
	if((system_rev & 0xFF) == 0x00)
	{
		#ifndef CONFIG_PMIC_INTERFACE
			gpio_pull(GPIO_PORTCR_ES1(24), GPIO_PULL_UP);
		#endif
		gpio_pull(GPIO_PORTCR_ES1(25), GPIO_PULL_UP);
		gpio_pull(GPIO_PORTCR_ES1(26), GPIO_PULL_UP);
		gpio_pull(GPIO_PORTCR_ES1(27), GPIO_PULL_UP);
		gpio_pull(GPIO_PORTCR_ES1(1), GPIO_PULL_UP);
		gpio_pull(GPIO_PORTCR_ES1(2), GPIO_PULL_UP);
	}
	else
	{
		#ifndef CONFIG_PMIC_INTERFACE
			gpio_pull(GPIO_PORTCR_ES2(24), GPIO_PULL_UP);
		#endif
		gpio_pull(GPIO_PORTCR_ES2(25), GPIO_PULL_UP);
		gpio_pull(GPIO_PORTCR_ES2(26), GPIO_PULL_UP);
		gpio_pull(GPIO_PORTCR_ES2(27), GPIO_PULL_UP);
		gpio_pull(GPIO_PORTCR_ES2(1), GPIO_PULL_UP);
		gpio_pull(GPIO_PORTCR_ES2(2), GPIO_PULL_UP);
	}
	return 0;
}

static struct gpio_keys_platform_data gpio_key_info = {
	.buttons	= gpio_buttons,
	.nbuttons	= ARRAY_SIZE(gpio_buttons),
	.rep		= 0,
	.enable		= gpio_key_enable,
	.poll_interval	= 50,
};

static struct platform_device gpio_key_device = {
	.name	= "gpio-keys-polled",
	.id	= -1,
	.dev	= {
		.platform_data	= &gpio_key_info,
	},
};

static const struct fb_videomode lcdc0_modes[] = {
	{
/*		.name		= "WVGA",*/
/*		.xres		= 480,*/
/*		.yres		= 800,*/
		.name		= "qHD",
		.xres		= 540,
		.yres		= 960,
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

static struct resource	tpu_resources[] = {
	[TPU_MODULE_0] = {
		.name	= "tpu0_map",
		.start	= 0xe6600000,
		.end	= 0xe6600200,
		.flags	= IORESOURCE_MEM,
	},
};

static struct port_info
	tpu_pwm_pfc[TPU_MODULE_MAX][TPU_CHANNEL_MAX] = {
	[TPU_MODULE_0] = {
		[TPU_CHANNEL_0]	= {
			.port_func	= GPIO_FN_TPUTO0,
			.func_name	= "pwm-tpu0to0",
		},
		[TPU_CHANNEL_1]	= {
			.port_func	= GPIO_FN_TPUTO1,
			.func_name	= "pwm-tpu0to1",
		},
		[TPU_CHANNEL_2]	= {
			.port_func	= GPIO_FN_TPUTO2,
			.func_name	= "pwm-tpu0to2",
		},
		[TPU_CHANNEL_3]	= {
			.port_func	= GPIO_FN_TPUTO3,
			.func_name	= "pwm-tpu0to3",
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

/* PCM2PWM */
static struct resource pcm2pwm_resource = {
	.name	= "pcm2pwm_map",
	.start	= 0xEC380000,
	.end	= 0xEC380090,
	.flags	= IORESOURCE_MEM,
};
static struct platform_device pcm2pwm_device = {
	.name			= "pcm2pwm-renesas-sh_mobile",

	.id				= 1,
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

#define ION_HEAP_VIDEO_SIZE	(SZ_32M - SZ_4M)
#define ION_HEAP_VIDEO_ADDR	(0x4bc00000 - ION_HEAP_VIDEO_SIZE)

static struct ion_platform_data u2evm_ion_data = {
	.nr = 3,
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
			.id = ION_HEAP_VIDEO_ID,
			.name = "video-cam",
			.base = ION_HEAP_VIDEO_ADDR,
			.size = ION_HEAP_VIDEO_SIZE,
		},
	},
};

static struct platform_device u2evm_ion_device = {
	.name = "ion-r-mobile",
	.id = -1,
	.dev = {
		.platform_data = &u2evm_ion_data,
	},
};

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

static int EOSMAIN_power0(struct device *dev, int power_on)
{
	struct clk *vclk1_clk;
	int iRet;

	dev_dbg(dev, "%s(): power_on=%d\n", __func__, power_on);

	vclk1_clk = clk_get(NULL, "vclk1_clk");
	if (IS_ERR(vclk1_clk)) {
		dev_err(dev, "clk_get(vclk1_clk) failed\n");
		return -1;
	}

	if (power_on) {
		printk(KERN_ALERT "%s : PowerON\n", __func__);
		sh_csi2_power(dev, power_on);
		gpio_direction_output(GPIO_PORT5, 1); /* VDIG ON */
		gpio_direction_output(GPIO_PORT3, 1); /* VANA ON */
		gpio_direction_output(GPIO_PORT4, 1); /* VANA ON SUB */
		mdelay(5);

		iRet = clk_set_rate(vclk1_clk,
				clk_round_rate(vclk1_clk, 24000000));
		if (0 != iRet) {
			dev_err(dev,
			"clk_set_rate(vclk1_clk) failed (ret=%d)\n", iRet);
		}

		iRet = clk_enable(vclk1_clk);
		if (0 != iRet) {
			dev_err(dev,
			"clk_enable(vclk1_clk) failed (ret=%d)\n", iRet);
		}

		mdelay(100);	/* 0ms */
		gpio_set_value(GPIO_PORT20, 0);		/* assert RESET */
		mdelay(100);	/* 0ms */
		gpio_set_value(GPIO_PORT90, 1);		/* turn on POWER */
		mdelay(100);	/* 1ms */
		gpio_set_value(GPIO_PORT20, 1);		/* deassert RESET */
		mdelay(100);	/* 20ms */

		gpio_set_value(GPIO_PORT16, 0);		/* assert RESET SUB */
		mdelay(100);	/* 0ms */
		gpio_set_value(GPIO_PORT91, 1);		/* POWER off SUB */
		mdelay(100);	/* 0ms */
	} else {
		printk(KERN_ALERT "%s : PowerOFF\n", __func__);
		gpio_set_value(GPIO_PORT20, 0);		/* assert RESET */
		mdelay(100);	/* 0ms */
		clk_disable(vclk1_clk);
		mdelay(100);	/* 0ms */
		gpio_set_value(GPIO_PORT90, 0);		/* POWER off */
		mdelay(100);	/* 0ms */

		gpio_direction_output(GPIO_PORT4, 0); /* VANA OFF SUB */
		gpio_direction_output(GPIO_PORT3, 0); /* VANA OFF */
		gpio_direction_output(GPIO_PORT5, 0); /* VDIG OFF */
		sh_csi2_power(dev, power_on);
	}

	clk_put(vclk1_clk);

	return 0;
}

static int dummy_camera_power(struct device *dev, int power_on)
{
	dev_dbg(dev, "%s(): power_on=%d\n", __func__, power_on);

	return 0;
}

static struct i2c_board_info i2c_cameras[] = {
	{
		I2C_BOARD_INFO("EOSCAMERA", 0x1A),
	},
	{
		I2C_BOARD_INFO("dummy_camera", 0),
	},
};

static struct soc_camera_link camera_links[] = {
	{
		.bus_id			= 0,
		.board_info		= &i2c_cameras[0],
		.i2c_adapter_id	= 1,
		.module_name	= "EOSCAMERA",
		.power			= EOSMAIN_power0,
	},
	{
		.bus_id			= 1,
		.board_info		= &i2c_cameras[1],
		.i2c_adapter_id	= 1,
		.module_name	= "dummy_camera",
		.power			= dummy_camera_power,
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
		.lanes		= 3,
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
		.lanes		= 3,
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

/* THREE optional u2evm_devices pointer lists for initializing the platform devices */
/* For different STM muxing options 0, 1, or None, as given by boot_command_line parameter stm=0/1/n */

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
	&fsi_device,
	&fsi_b_device,
	&gpio_key_device,
	&lcdc_device,
	&mfis_device,
	&mipidsi0_device,
//	&tpu_devices[TPU_MODULE_0],
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
	&sdhi0_device, // STM Trace muxed over SDHI0 SD-Card interface, coming by special SD-Card adapter to FIDO
//	&sdhi1_device,
	&fsi_device,
	&fsi_b_device,
	&gpio_key_device,
	&lcdc_device,
	&mfis_device,
	&mipidsi0_device,
//	&tpu_devices[TPU_MODULE_0],
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
//	&sdhi1_device,
	&fsi_device,
	&fsi_b_device,
	&gpio_key_device,
	&lcdc_device,
	&mfis_device,
	&mipidsi0_device,
//	&tpu_devices[TPU_MODULE_0],
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
		.name	= "rtc_tps80032",	\
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
#else
	{
		I2C_BOARD_INFO("tps80032", 0x4A),
		.irq		= irqpin2irq(28),
		.platform_data	= &tps_platform,
	},
#endif
};

#ifdef CONFIG_KEYBOARD_CYPRESS_TOUCH
static struct i2c_board_info i2c_touchkey[];

static void touchkey_init_hw(void)
{
	/* To do as follows
	gpio_request(GPIO_3_TOUCH_INT, "3_TOUCH_INT");
	setpull(GPIO_3_TOUCH_INT, S3C_GPIO_PULL_NONE);
	register_interrupt(GPIO_3_TOUCH_INT);
	gpio_direction_input(GPIO_3_TOUCH_INT);

	i2c_touchkey[0].irq = gpio_to_irq(GPIO_3_TOUCH_INT);
	irq_set_irq_type(gpio_to_irq(GPIO_3_TOUCH_INT), IRQF_TRIGGER_FALLING);
	gpio_cfgpin(GPIO_3_TOUCH_INT, S3C_GPIO_SFN(0xf));
	*/
#if 0
	gpio_request(GPIO_PORT104, NULL);
	gpio_direction_input(GPIO_PORT104);
	gpio_pull(GPIO_PORTCR(104), GPIO_PULL_UP);
	i2c_touchkey[0].irq = irqpin2irq(43);
	irq_set_irq_type(irqpin2irq(43), IRQF_TRIGGER_FALLING);
#endif
}

static int touchkey_suspend(void)
{
	struct regulator *regulator;
#if 0
	regulator = regulator_get(NULL, TK_REGULATOR_NAME);
	if (IS_ERR(regulator))
		return 0;
	if (regulator_is_enabled(regulator))
		regulator_force_disable(regulator);

	regulator_put(regulator);
#endif
	return 1;
}

static int touchkey_resume(void)
{
	struct regulator *regulator;
#if 0
	regulator = regulator_get(NULL, TK_REGULATOR_NAME);
	if (IS_ERR(regulator))
		return 0;
	regulator_enable(regulator);
	regulator_put(regulator);
#endif
	return 1;
}

static int touchkey_power_on(bool on)
{
	int ret;

	if (on) {
		/* To do to power on */		
	}
	else {
		/* To do to power off */		
	}

	if (on)
		ret = touchkey_resume();
	else
		ret = touchkey_suspend();

	return ret;
}

static int touchkey_led_power_on(bool on)
{
	if (on) {
		/* To do to led power on */		
	}
	else {
		/* To do to led power off */		
	}
	
	return 1;
}

static struct touchkey_platform_data touchkey_pdata = {
	.gpio_sda = NULL,	/* To do to set gpio */
	.gpio_scl = NULL,	/* To do to set gpio */
	.gpio_int = NULL,	/* To do to set gpio */
	.init_platform_hw = touchkey_init_hw,
	.suspend = touchkey_suspend,
	.resume = touchkey_resume,
	.power_on = touchkey_power_on,
	.led_power_on = touchkey_led_power_on,
};
#endif /*CONFIG_KEYBOARD_CYPRESS_TOUCH*/

static struct i2c_board_info i2c_touchkey[] = {
#ifdef CONFIG_KEYBOARD_CYPRESS_TOUCH
	{
		I2C_BOARD_INFO("sec_touchkey", 0x20),
		.platform_data = &touchkey_pdata,
		.irq = irqpin2irq(43),
	},
#endif
};


#ifndef CONFIG_PMIC_INTERFACE
	static struct regulator *mxt224_regulator;
#endif

static void mxt224_set_power(int on)
{
#ifdef CONFIG_PMIC_INTERFACE
	pmic_set_power_on(E_POWER_VANA_MM);
#else
	if (!mxt224_regulator)
		mxt224_regulator = regulator_get(NULL, "vdd_touch");

	if (mxt224_regulator) {
		if (on)
			regulator_enable(mxt224_regulator);
		else
			regulator_disable(mxt224_regulator);
	}
#endif
}

static int mxt224_read_chg(void)
{
	return gpio_get_value(GPIO_PORT32);
}

static struct mxt_platform_data mxt224_platform_data = {
	.x_line		= 19,
	.y_line		= 11,
	.x_size		= 800,
	.y_size		= 480,
	.blen		= 0x21,
	.threshold	= 0x28,
	.voltage	= 1825000,
	.orient		= MXT_DIAGONAL,
	.irqflags	= IRQF_TRIGGER_FALLING,
	.set_pwr	= NULL ,//mxt224_set_power,
	.read_chg	= mxt224_read_chg,
};

static struct i2c_board_info i2c4_devices[] = {
	{
		I2C_BOARD_INFO("atmel_mxt_ts", 0x4a),
		.platform_data = &mxt224_platform_data,
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
#if 0
	{
		I2C_BOARD_INFO("led", 0x74),
	},
	{
		I2C_BOARD_INFO("flash", 0x30),
	},
#endif
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
	{
	        I2C_BOARD_INFO("av7100", 0x70),
	},
};
#endif
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


#define IRQC0_CONFIG_00		IO_ADDRESS(0xe61c0180)
#define IRQC1_CONFIG_00		IO_ADDRESS(0xe61c0380)
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
	u8 reg = __raw_readb(STBCHR2);
	__raw_writeb((reg | APE_RESETLOG_U2EVM_RESTART), STBCHR2); // write STBCHR2 for debug

	__raw_writel(0, SBAR2);
	__raw_writel(__raw_readl(RESCNT2) | (1 << 31), RESCNT2);
}

 
 
#define DSI0PHYCR	IO_ADDRESS(0xe615006c)

static void __init u2evm_init(void)
{
	char *cp=&boot_command_line[0];
	int ci;
	int stm_select=1;	// Shall tell how to route STM traces.
				// Taken from boot_command_line[] parameters.
				// stm=# will set parameter, if '0' or '1' then as number, otherwise -1.
				// -1 = NONE, i.e. SDHI1 and SDHI0 are free for other functions.
				//  0 = SDHI0 used for STM traces. SD-Card not enabled.
				//  1 = SDHI1 used for STM traces. WLAN not enabled. [DEFAULT if stm boot para not defined]
	if (cp[0] && cp[1] && cp[2] && cp[3] && cp[4]) {
		for (ci=4; cp[ci]; ci++) {
			if (cp[ci-4] == 's' &&
			    cp[ci-3] == 't' &&
			    cp[ci-2] == 'm' &&
			    cp[ci-1] == '=') {
				switch (cp[ci]) {
					case '0': stm_select =  0; break;
					case '1': stm_select =  1; break;
					default:  stm_select = -1; break;
				}
				break;
			}
		}
	}
	printk("stm_select=%d\n", stm_select);

	r8a73734_pinmux_init();
	if((system_rev & 0xFF) != 0x00)      /*ES1_ECR0208*/
	{
#define GPIO_DRVCR_SD0	((volatile ushort *)(0xE6050000ul + 0x818E))
#define GPIO_DRVCR_SIM1	((volatile ushort *)(0xE6050000ul + 0x8192))
#define GPIO_DRVCR_SIM2	((volatile ushort *)(0xE6050000ul + 0x8194))

		*GPIO_DRVCR_SD0 = 0x0023;
		*GPIO_DRVCR_SIM1 = 0x0023;
		*GPIO_DRVCR_SIM2 = 0x0023;
	}
	shmobile_arch_reset = u2evm_restart;

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

if((system_rev & 0xFF) == 0x00) /*ES1.0*/
{	
	gpio_pull(GPIO_PORTCR_ES1(44), GPIO_PULL_UP);
	gpio_pull(GPIO_PORTCR_ES1(45), GPIO_PULL_UP);
	gpio_pull(GPIO_PORTCR_ES1(46), GPIO_PULL_UP);
	gpio_pull(GPIO_PORTCR_ES1(47), GPIO_PULL_UP);
	gpio_pull(GPIO_PORTCR_ES1(48), GPIO_PULL_UP);
	gpio_pull(GPIO_PORTCR_ES1(96), GPIO_PULL_UP);
	gpio_pull(GPIO_PORTCR_ES1(97), GPIO_PULL_UP);
}
else /*ES2.0*/
{
	gpio_pull(GPIO_PORTCR_ES2(44), GPIO_PULL_UP);
	gpio_pull(GPIO_PORTCR_ES2(45), GPIO_PULL_UP);
	gpio_pull(GPIO_PORTCR_ES2(46), GPIO_PULL_UP);
	gpio_pull(GPIO_PORTCR_ES2(47), GPIO_PULL_UP);
	gpio_pull(GPIO_PORTCR_ES2(48), GPIO_PULL_UP);
	gpio_pull(GPIO_PORTCR_ES2(96), GPIO_PULL_UP);
	gpio_pull(GPIO_PORTCR_ES2(97), GPIO_PULL_UP);
}
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
	
	gpio_request(GPIO_PORT29, NULL); //Enabled eMMC LDO
	gpio_direction_output(GPIO_PORT29, 1);

	if (0 != stm_select) {
		/* If STM Traces go to SDHI1 or NOWHERE, then SDHI0 can be used for SD-Card */
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

#define MSEL3CR		IO_ADDRESS(0xE6058020)
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
if((system_rev & 0xFF) == 0x00) /*ES1.0*/
        gpio_pull(GPIO_PORTCR_ES1(293), GPIO_PULL_UP);
else /*ES2.0*/
	gpio_pull(GPIO_PORTCR_ES2(293), GPIO_PULL_UP);

	}
	
	if (0 == stm_select) {
        /* SDHI0 used for STMSIDI */
//        gpio_request(GPIO_PORT324, NULL);
//        gpio_direction_input(GPIO_PORT324);
        gpio_request(GPIO_FN_STMSIDI_1, NULL);
if((system_rev & 0xFF) == 0x00) /*ES1.0*/
        gpio_pull(GPIO_PORTCR_ES1(324), GPIO_PULL_UP);
else /*ES2.0*/
	gpio_pull(GPIO_PORTCR_ES2(324), GPIO_PULL_UP);

	}

        if (-1 != stm_select) {
          int i;
          volatile unsigned long dummy_read;
#if 0 // NOT neede any more with new FIDO SW version Fido.1.9.5.36.edge_aligned_stpv2
          // Lower CPG Frequency Control Register B (BRQCRB) ZTRFC clock by divider  control because STM clock was 76.8MHZ, too high, now it is about 38.4MHz
#define BRQCRB		IO_ADDRESS(0xE6150004)
	  __raw_writel((__raw_readl(BRQCRB) & 0x7F0FFFFF) | 0x80400000, BRQCRB); // Set KICK bit and set ZTRFC[3:0] to 0100, i.e. x 1/8 divider for System CPU Debugging and Trace Clock Frequenct Division Ratio
#endif

#define DBGREG9		IO_ADDRESS(0xE6100040)
	  __raw_writel(0x0000a501, DBGREG9); /* Key register */
	  __raw_writel(0x0000a501, DBGREG9); /* Key register, must write twice! */

          for(i=0; i<0x10; i++);

#define DBGREG1		IO_ADDRESS(0xE6100020)
	  if (1 == stm_select) {
          __raw_writel((__raw_readl(DBGREG1) & 0xFFDFFFFF) | (1<<20), DBGREG1);
		// Clear STMSEL[1], i.e. select STMSIDI to BB side.
		// Set   STMSEL[0], i.e. select SDHI1/STM*_2 as output/in port for STM
	  }
	  
	  if (0 == stm_select) {
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
                                                        (0 << 6) |              /*    Reserved      ´                            */
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
	    irq_set_irq_type(irqpin2irq(42), IRQ_TYPE_EDGE_FALLING);
	    irqc_set_chattering(42, 0x01);  /* 1msec */
	

	}

	/* touch key */
	gpio_request(GPIO_PORT104, NULL);
	gpio_direction_input(GPIO_PORT104);
if((system_rev & 0xFF) == 0x00) /*ES1.0*/
{	
	gpio_pull(GPIO_PORTCR_ES1(104), GPIO_PULL_UP);
}
else /*ES2.0*/
{
	gpio_pull(GPIO_PORTCR_ES2(104), GPIO_PULL_UP);
}

	/* I2C */
	gpio_request(GPIO_FN_I2C_SCL0H, NULL);
	gpio_request(GPIO_FN_I2C_SDA0H, NULL);
	gpio_request(GPIO_FN_I2C_SCL1H, NULL);
	gpio_request(GPIO_FN_I2C_SDA1H, NULL);

	/* PMIC */
	gpio_request(GPIO_PORT0, NULL);	/* MSECURE */
	gpio_direction_output(GPIO_PORT0, 1);
	gpio_request(GPIO_PORT28, NULL);
	gpio_direction_input(GPIO_PORT28);

	#ifdef CONFIG_PMIC_INTERFACE
		irq_set_irq_type(irqpin2irq(28), IRQ_TYPE_EDGE_FALLING);
	#else
		irq_set_irq_type(irqpin2irq(28), IRQ_TYPE_LEVEL_LOW);
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
	gpio_direction_output(GPIO_PORT30, 1);
	
	/* Touch */
	gpio_request(GPIO_PORT32, NULL);
	gpio_direction_input(GPIO_PORT32);
if((system_rev & 0xFF) == 0x00) /*ES1.0*/
	gpio_pull(GPIO_PORTCR_ES1(32), GPIO_PULL_UP);
else /*ES2.0*/
	gpio_pull(GPIO_PORTCR_ES2(32), GPIO_PULL_UP);

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

	if (1 != stm_select) {
		/*configure Ports for SDHI1*/
		*((volatile u8 *)SDHI1_D0_CR) = 0xC1;
		*((volatile u8 *)SDHI1_D1_CR) = 0xC1;
		*((volatile u8 *)SDHI1_D2_CR) = 0xC1;
		*((volatile u8 *)SDHI1_D3_CR) = 0xC1;
		*((volatile u8 *)SDHI1_CMD_CR) = 0xC1;
	}

	/* enable sound */
	gpio_request(GPIO_FN_FSIAISLD, "sound");
	gpio_request(GPIO_FN_FSIAOBT, "sound");
	gpio_request(GPIO_FN_FSIAOLR, "sound");
	gpio_request(GPIO_FN_FSIAOSLD, "sound");

	gpio_request(GPIO_FN_FSIBISLD, "sound");
	gpio_request(GPIO_FN_FSIBOBT, "sound");
	gpio_request(GPIO_FN_FSIBOLR, "sound");
	gpio_request(GPIO_FN_FSIBOSLD, "sound");

#ifdef CONFIG_CACHE_L2X0
	/*
	 * [30] Early BRESP enable
	 * [27] Non-secure interrupt access control
	 * [26] Non-secure lockdown enable
	 * [22] Shared attribute override enable
	 * [19:17] Way-size: b010 = 32KB
	 * [16] Accosiativity: 0 = 8-way
	 */
	if ((system_rev & 0xFF) == 0x00)
	{
		l2x0_init(__io(IO_ADDRESS(0xf0100000)), 0x4c440000, 0x820f0fff);
	}
	else
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
	if (0x00003E00 == system_rev) {
		/* ES1.0 */
		printk(KERN_ALERT "Camera ISP ES version switch (ES1)\n");
		csi21_device.resource = csi21_resources_es1;
		csi21_device.num_resources = ARRAY_SIZE(csi21_resources_es1);
		csi21_info.flags |= SH_CSI2_MULTI;
		csi21_info.cmod_name = csi20_info.cmod_name;
		rcu0_device.resource = rcu0_resources_es1;
		rcu1_device.resource = rcu1_resources_es1;
		rcu1_device.num_resources = ARRAY_SIZE(rcu1_resources_es1);
		sh_mobile_rcu1_info.mod_name = sh_mobile_rcu0_info.mod_name;
	} else
		printk(KERN_ALERT "Camera ISP ES version switch (ES2)\n");
}

#if 0
	gpio_request(GPIO_PORT39, NULL);
	gpio_direction_output(GPIO_PORT39, 0);
#endif
	r8a73734_add_standard_devices();

	switch (stm_select) {
		case 0:
			platform_add_devices(u2evm_devices_stm_sdhi0,
				ARRAY_SIZE(u2evm_devices_stm_sdhi0));
			break;
		case 1:
			platform_add_devices(u2evm_devices_stm_sdhi1,
				ARRAY_SIZE(u2evm_devices_stm_sdhi1));
			break;
		default:
			platform_add_devices(u2evm_devices_stm_none,
				ARRAY_SIZE(u2evm_devices_stm_none));
			break;
	}

	r8a73734_hwlock_gpio = hwspin_lock_request_specific(SMGPIO);
	r8a73734_hwlock_cpg = hwspin_lock_request_specific(SMCPG);
	r8a73734_hwlock_sysc = hwspin_lock_request_specific(SMSYSC);

	i2c_register_board_info(0, i2c0_devices, ARRAY_SIZE(i2c0_devices));
	i2c_register_board_info(4, i2c4_devices, ARRAY_SIZE(i2c4_devices));
#if 0
        if (0x00003E00 == system_rev) {
            i2c_register_board_info(6, i2cm_devices, ARRAY_SIZE(i2cm_devices));
        } else {
            i2c_register_board_info(6, i2cm_devices_es2, ARRAY_SIZE(i2cm_devices_es2));
        }
#endif
	crashlog_r_local_ver_write();
	crashlog_reset_log_write();
	crashlog_init_tmplog();
	i2c_register_board_info(10, i2c_touchkey, ARRAY_SIZE(i2c_touchkey)); //For TOUCHKEY

	i2c_register_board_info(9, i2c9gpio_devices, ARRAY_SIZE(i2c9gpio_devices));
	i2c_register_board_info(6, i2cm_devices, ARRAY_SIZE(i2cm_devices));
}

#ifdef ARCH_HAS_READ_CURRENT_TIMER

/* CMT13 */
#define CMSTR3		IO_ADDRESS(0xe6130300)
#define CMCSR3		IO_ADDRESS(0xe6130310)
#define CMCNT3		IO_ADDRESS(0xe6130314)
#define CMCOR3		IO_ADDRESS(0xe6130318)
#define CMCLKE		IO_ADDRESS(0xe6131000)

static DEFINE_SPINLOCK(cmt_lock);

int read_current_timer(unsigned long *timer_val)
{
	*timer_val = __raw_readl(CMCNT3);
	return 0;
}

static int __init setup_current_timer(void)
{
	struct clk *clk;
	unsigned long lpj, flags;

	clk = clk_get(NULL, "currtimer");
	if (IS_ERR(clk))
		return PTR_ERR(clk);
	clk_enable(clk);

	lpj = clk_get_rate(clk) + HZ/2;
	do_div(lpj, HZ);
	lpj_fine = lpj;

	spin_lock_irqsave(&cmt_lock, flags);
	__raw_writel(__raw_readl(CMCLKE) | (1 << 3), CMCLKE);
	spin_unlock_irqrestore(&cmt_lock, flags);

	__raw_writel(0, CMSTR3);
	__raw_writel(0x10b, CMCSR3); /* Free-running, DBGIVD, CKS=3 */
	__raw_writel(0xffffffff, CMCOR3);
	__raw_writel(0, CMCNT3);
	while (__raw_readl(CMCNT3) != 0)
		cpu_relax();
	__raw_writel(1, CMSTR3);

	pr_info("Current timer started (lpj=%lu)\n", lpj);

	/*
	 * TODO: Current timer (CMT1) MSTP bit vs. Suspend-to-RAM
	 *
	 * We don't have proper suspend/resume operations implemented yet
	 * for the current timer (CMT1), so there is no guarantee that CMT1
	 * module is functional when timer-based udelay() is used.  Thus
	 * we need to enable CMT1 MSTP clock here, and if possible, would
	 * like to leave it enabled forever.
	 *
	 * On the other hand, CMT1 should be halted during Suspend-to-RAM
	 * state to minimize power consumption.
	 *
	 * To solve the problem, we make the following assumptions:
	 *
	 * 1) udelay() is not used from now (time_init()) until
	 *    late_time_init() or calibrate_delay() completes
	 *
	 * 2) timer-based udelay() is functional as long as clocksource is
	 *    available
	 *
	 * and disable CMT1 MSTP clock here not to increment CMT1 usecount.
	 */
	clk_disable(clk);
	clk_put(clk);
	return 0;
}

#endif /* ARCH_HAS_READ_CURRENT_TIMER */

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

static void crashlog_r_local_ver_write()
{
	void __iomem * adr = 0;
	void __iomem * adr_bak = 0;
	char	version_name[CLASHLOG_R_LOCAL_VER_LENGTH];
	int	revision_version_length;
	unsigned char i;
	
	adr = ioremap(CLASHLOG_R_LOCAL_VER_LOCATE, CLASHLOG_R_LOCAL_VER_LENGTH); 
	adr_bak = adr;
	if (adr) 
	{
		revision_version_length=strlen(linux_banner);

		snprintf(mmcoops_info.soft_version,CLASHLOG_R_LOCAL_VER_LENGTH,"%s %s",RMC_LOCAL_VERSION,(linux_banner + revision_version_length - 25) );

		strncpy(version_name , RMC_LOCAL_VERSION , CLASHLOG_R_LOCAL_VER_LENGTH);

		for(i=0 ; i<CLASHLOG_R_LOCAL_VER_LENGTH ; i++){
			__raw_writeb(version_name[i], adr);
			adr++;
		}

		iounmap(adr_bak);
	}
}

extern unsigned long log_buf_address;
extern unsigned long log_buf_len_address;
extern unsigned long log_end_address;
extern unsigned long logged_chars_address;

extern unsigned long log_main_buffer_address;
extern unsigned long log_main_size_address;
extern unsigned long log_main_w_off_address;
extern unsigned long log_main_head_address;

extern unsigned long log_events_buffer_address;
extern unsigned long log_events_size_address;
extern unsigned long log_events_w_off_address;
extern unsigned long log_events_head_address;

extern unsigned long log_radio_buffer_address;
extern unsigned long log_radio_size_address;
extern unsigned long log_radio_w_off_address;
extern unsigned long log_radio_head_address;

extern unsigned long log_system_buffer_address;
extern unsigned long log_system_size_address;
extern unsigned long log_system_w_off_address;
extern unsigned long log_system_head_address;

static void crashlog_reset_log_write()
{
	void __iomem * adr = 0;
	u8 	reg = 0;

/* kmsg */
/*	printk(KERN_ERR "log_buf_address=0x%08x\n", log_buf_address); */
	adr = ioremap(CRASHLOG_KMSG_LOCATE, 16);
	__raw_writel(log_buf_address, adr);
	__raw_writel(log_buf_len_address, adr + 4);
	__raw_writel(log_end_address, adr + 8);
	__raw_writel(logged_chars_address, adr + 12);
	iounmap(adr);

/* log_cat_main */
/*	printk(KERN_ERR "log_main_buffer_address=0x%08x\n", log_main_buffer_address); */
	adr = ioremap(CRASHLOG_LOGCAT_MAIN_LOCATE, 16);
	__raw_writel(log_main_buffer_address, adr);
	__raw_writel(log_main_size_address, adr + 4);
	__raw_writel(log_main_w_off_address, adr + 8);
	__raw_writel(log_main_head_address, adr + 12);
	iounmap(adr);

/* log_cat_events */
/*	printk(KERN_ERR "log_events_buffer_address=0x%08x\n", log_events_buffer_address); */
	adr = ioremap(CRASHLOG_LOGCAT_EVENT_LOCATE, 16);
	__raw_writel(log_events_buffer_address, adr);
	__raw_writel(log_events_size_address, adr + 4);
	__raw_writel(log_events_w_off_address, adr + 8);
	__raw_writel(log_events_head_address, adr + 12);
	iounmap(adr);

/* log_cat_radio */
/*	printk(KERN_ERR "log_radio_buffer_address=0x%08x\n", log_radio_buffer_address); */
	adr = ioremap(CRASHLOG_LOGCAT_RADIO_LOCATE, 16);
	__raw_writel(log_radio_buffer_address, adr);
	__raw_writel(log_radio_size_address, adr + 4);
	__raw_writel(log_radio_w_off_address, adr + 8);
	__raw_writel(log_radio_head_address, adr + 12);
	iounmap(adr);

/* log_cat_system */
	adr = ioremap(CRASHLOG_LOGCAT_SYSTEM_LOCATE, 16);
	__raw_writel(log_system_buffer_address, adr);
	__raw_writel(log_system_size_address, adr + 4);
	__raw_writel(log_system_w_off_address, adr + 8);
	__raw_writel(log_system_head_address, adr + 12);
	iounmap(adr);

	reg = __raw_readb(STBCHR2);
	__raw_writeb((reg | APE_RESETLOG_INIT_COMPLETE), STBCHR2);	/* andriod init */

/*Developer option to debug Reset Log*/
     /*	reg = __raw_readb(STBCHR3);*/
     /*	__raw_writeb((reg | APE_RESETLOG_DEBUG), STBCHR3);*/
}

static void crashlog_init_tmplog(void)
{
	if (request_mem_region(TMPLOG_ADDRESS, TMPLOG_SIZE, "tmplog-nocache"))
	{
		tmplog_nocache_address = (char *)ioremap_nocache(TMPLOG_ADDRESS, TMPLOG_SIZE);
		memcpy(tmplog_nocache_address, "CrashLog Temporary Area" , 24);
	}

	/*	reg = __raw_readb(STBCHR3);*/
	/*	__raw_writeb((reg | APE_RESETLOG_TMPLOG_END), STBCHR3); */ // write STBCHR3 for debug	

	return;
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
