#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/hwspinlock.h>
#include <mach/common.h>
#include <mach/hardware.h>
#include <mach/r8a73734.h>
#include <asm/hardware/cache-l2x0.h>
#include <linux/gpio_keys.h>
#include <linux/usb/r8a66597.h>
#include <mach/setup-u2usb.h>
#include <linux/mfd/tps80031.h>
#include <linux/dma-mapping.h>
#if defined(CONFIG_SAMSUNG_MHL)
#include <linux/sii8332_platform.h>
#endif
#ifdef CONFIG_USB_OTG
#include <linux/usb/tusb1211.h>
#endif
#ifdef CONFIG_PMIC_INTERFACE
#include <linux/pmic/pmic.h>
#include <linux/pmic/pmic-tps80032.h>
#endif
#include <mach/r8a73734.h>
#define ENT_TPS80031_IRQ_BASE	(IRQPIN_IRQ_BASE + 64)
#define ENT_TPS80032_IRQ_BASE	(IRQPIN_IRQ_BASE + 64)
static int is_vbus_powered(void)
{
	int val = 0;
	int val1 = 0;
	int count = 10;

	/* Extract bit VBSTS in INTSTS0 register */
	val = __raw_readw(IO_ADDRESS(0xE6890040)) & 0x80;

	while (--count) {
		msleep(1);
		val1 = __raw_readw(IO_ADDRESS(0xE6890040)) & 0x80;
		if (val != val1) {
			count = 10;
			val = val1;
		}
	}

	printk(KERN_INFO "Value of Status register INTSTS0: %x\n",
			__raw_readw(IO_ADDRESS(0xE6890040)));
/*Chaitanya*/
#if defined(CONFIG_SAMSUNG_MHL)
	isvbus_powered_mhl(val1);
#endif
/*Chaitanya*/
	return val1>>7;
}

#define LOCK_TIME_OUT_MS 1000
static void usbhs_module_reset(void)
{
	unsigned long flags;
	int ret;

	ret = hwspin_lock_timeout_irqsave(r8a73734_hwlock_cpg,
		LOCK_TIME_OUT_MS, &flags);
	if (ret < 0)
		printk(KERN_INFO "Can't lock hwlock_cpg\n");
	else {
		__raw_writel(__raw_readl(SRCR2) |
				(1 << 14), SRCR2); /* USBHS-DMAC */
		__raw_writel(__raw_readl(SRCR3) | (1 << 22), SRCR3); /* USBHS */
		hwspin_unlock_irqrestore(r8a73734_hwlock_cpg, &flags);
	}
	udelay(50); /* wait for at least one EXTALR cycle */
	ret = hwspin_lock_timeout_irqsave(r8a73734_hwlock_cpg,
		LOCK_TIME_OUT_MS, &flags);
	if (ret < 0)
		printk(KERN_INFO "Can't lock hwlock_cpg\n");
	else {
		__raw_writel(__raw_readl(SRCR2) & ~(1 << 14), SRCR2);
		__raw_writel(__raw_readl(SRCR3) & ~(1 << 22), SRCR3);
		hwspin_unlock_irqrestore(r8a73734_hwlock_cpg, &flags);
	}
	/* wait for SuspendM bit being cleared by hardware */
	while (!(__raw_readw(PHYFUNCTR) & (1 << 14))) /* SUSMON */
			;

	__raw_writew(__raw_readw(PHYFUNCTR) |
		(1 << 13), PHYFUNCTR); /* PRESET */
	while (__raw_readw(PHYFUNCTR) & (1 << 13))
			;
#ifdef CONFIG_USB_OTG
#define SYSSTS	IO_ADDRESS(0xe6890004) /* 16-bit */
	__raw_writew(__raw_readw(PHYOTGCTR) |
		(1 << 8), PHYOTGCTR); /* IDPULLUP */
	msleep(50);
#endif
}

static struct r8a66597_gpio_setting_info r8a66597_gpio_setting_info[] = {
	[0] = {
		.flag = 1,
		.port = GPIO_PORT203,
		.active = {
			.port_mux 	= GPIO_FN_ULPI_DATA0,
			.pull 		= R8A66597_PULL_DOWN,
			.direction	= R8A66597_DIRECTION_NOT_SET,
			.out_level	= R8A66597_OUT_LEVEL_NOT_SET,
		},
		.deactive = {
			.port_mux 	= GPIO_PORT203,
			.pull 		= R8A66597_PULL_OFF,
			.direction	= R8A66597_DIRECTION_NONE,
			.out_level	= R8A66597_OUT_LEVEL_NOT_SET,
		}                                   
	},
	[1] = {
		.flag = 1,
		.port = GPIO_PORT204,
		.active = {
			.port_mux 	= GPIO_FN_ULPI_DATA1,
			.pull 		= R8A66597_PULL_DOWN,
			.direction	= R8A66597_DIRECTION_NOT_SET,
			.out_level	= R8A66597_OUT_LEVEL_NOT_SET,
		},
		.deactive = {
			.port_mux 	= GPIO_PORT204,
			.pull 		= R8A66597_PULL_OFF,
			.direction	= R8A66597_DIRECTION_NONE,
			.out_level	= R8A66597_OUT_LEVEL_NOT_SET,
		}                                   
	},
	[2] = {
		.flag = 1,
		.port = GPIO_PORT205,
		.active = {
			.port_mux 	= GPIO_FN_ULPI_DATA2,
			.pull 		= R8A66597_PULL_DOWN,
			.direction	= R8A66597_DIRECTION_NOT_SET,
			.out_level	= R8A66597_OUT_LEVEL_NOT_SET,
		},
		.deactive = {
			.port_mux 	= GPIO_PORT205,
			.pull 		= R8A66597_PULL_OFF,
			.direction	= R8A66597_DIRECTION_NONE,
			.out_level	= R8A66597_OUT_LEVEL_NOT_SET,
		}                                   
	},
	[3] = {
		.flag = 1,
		.port = GPIO_PORT206,
		.active = {
			.port_mux 	= GPIO_FN_ULPI_DATA3,
			.pull 		= R8A66597_PULL_DOWN,
			.direction	= R8A66597_DIRECTION_NOT_SET,
			.out_level	= R8A66597_OUT_LEVEL_NOT_SET,
		},
		.deactive = {
			.port_mux 	= GPIO_PORT206,
			.pull 		= R8A66597_PULL_OFF,
			.direction	= R8A66597_DIRECTION_NONE,
			.out_level	= R8A66597_OUT_LEVEL_NOT_SET,
		}                                   
	},
	[4] = {
		.flag = 1,
		.port = GPIO_PORT207,
		.active = {
			.port_mux 	= GPIO_FN_ULPI_DATA4,
			.pull 		= R8A66597_PULL_DOWN,
			.direction	= R8A66597_DIRECTION_NOT_SET,
			.out_level	= R8A66597_OUT_LEVEL_NOT_SET,
		},
		.deactive = {
			.port_mux 	= GPIO_PORT207,
			.pull 		= R8A66597_PULL_OFF,
			.direction	= R8A66597_DIRECTION_NONE,
			.out_level	= R8A66597_OUT_LEVEL_NOT_SET,
		}                                   
	},
	[5] = {
		.flag = 1,
		.port = GPIO_PORT208,
		.active = {
			.port_mux 	= GPIO_FN_ULPI_DATA5,
			.pull 		= R8A66597_PULL_DOWN,
			.direction	= R8A66597_DIRECTION_NOT_SET,
			.out_level	= R8A66597_OUT_LEVEL_NOT_SET,
		},
		.deactive = {
			.port_mux 	= GPIO_PORT208,
			.pull 		= R8A66597_PULL_OFF,
			.direction	= R8A66597_DIRECTION_NONE,
			.out_level	= R8A66597_OUT_LEVEL_NOT_SET,
		}                                   
	},
	[6] = {
		.flag = 1,
		.port = GPIO_PORT209,
		.active = {
			.port_mux 	= GPIO_FN_ULPI_DATA6,
			.pull 		= R8A66597_PULL_DOWN,
			.direction	= R8A66597_DIRECTION_NOT_SET,
			.out_level	= R8A66597_OUT_LEVEL_NOT_SET,
		},
		.deactive = {
			.port_mux 	= GPIO_PORT209,
			.pull 		= R8A66597_PULL_OFF,
			.direction	= R8A66597_DIRECTION_NONE,
			.out_level	= R8A66597_OUT_LEVEL_NOT_SET,
		}                                   
	},
	[7] = {
		.flag = 1,
		.port = GPIO_PORT210,
		.active = {
			.port_mux 	= GPIO_FN_ULPI_DATA7,
			.pull 		= R8A66597_PULL_DOWN,
			.direction	= R8A66597_DIRECTION_NOT_SET,
			.out_level	= R8A66597_OUT_LEVEL_NOT_SET,
		},
		.deactive = {
			.port_mux 	= GPIO_PORT210,
			.pull 		= R8A66597_PULL_OFF,
			.direction	= R8A66597_DIRECTION_NONE,
			.out_level	= R8A66597_OUT_LEVEL_NOT_SET,
		}                                   
	},
	[8] = {
		.flag = 1,
		.port = GPIO_PORT211,
		.active = {
			.port_mux 	= GPIO_FN_ULPI_CLK,
			.pull 		= R8A66597_PULL_OFF,
			.direction	= R8A66597_DIRECTION_NOT_SET,
			.out_level	= R8A66597_OUT_LEVEL_NOT_SET,
		},
		.deactive = {
			.port_mux 	= GPIO_PORT211,
			.pull 		= R8A66597_PULL_OFF,
			.direction	= R8A66597_DIRECTION_NONE,
			.out_level	= R8A66597_OUT_LEVEL_NOT_SET,
		}                                   
	},
	[9] = {
		.flag = 1,
		.port = GPIO_PORT212,
		.active = {
			.port_mux 	= GPIO_FN_ULPI_STP,
			.pull 		= R8A66597_PULL_UP,
			.direction	= R8A66597_DIRECTION_NOT_SET,
			.out_level	= R8A66597_OUT_LEVEL_NOT_SET,
		},
		.deactive = {
			.port_mux 	= GPIO_PORT212,
			.pull 		= R8A66597_PULL_OFF,
			.direction	= R8A66597_DIRECTION_NONE,
			.out_level	= R8A66597_OUT_LEVEL_NOT_SET,
		}                                   
	},
	[10] = {
		.flag = 1,
		.port = GPIO_PORT213,
		.active = {
			.port_mux 	= GPIO_FN_ULPI_DIR,
			.pull 		= R8A66597_PULL_DOWN,
			.direction	= R8A66597_DIRECTION_NOT_SET,
			.out_level	= R8A66597_OUT_LEVEL_NOT_SET,
		},
		.deactive = {
			.port_mux 	= GPIO_PORT213,
			.pull 		= R8A66597_PULL_OFF,
			.direction	= R8A66597_DIRECTION_NONE,
			.out_level	= R8A66597_OUT_LEVEL_NOT_SET,
		}                                   
	},
	[11] = {
		.flag = 1,
		.port = GPIO_PORT214,
		.active = {
			.port_mux 	= GPIO_FN_ULPI_NXT,
			.pull 		= R8A66597_PULL_DOWN,
			.direction	= R8A66597_DIRECTION_NOT_SET,
			.out_level	= R8A66597_OUT_LEVEL_NOT_SET,
		},
		.deactive = {
			.port_mux 	= GPIO_PORT214,
			.pull 		= R8A66597_PULL_OFF,
			.direction	= R8A66597_DIRECTION_NONE,
			.out_level	= R8A66597_OUT_LEVEL_NOT_SET,
		}                                   
	},
	[12] = {
		.flag = 1,
		.port = GPIO_PORT217,
		.active = {
			.port_mux 	= GPIO_FN_VIO_CKO3,
			.pull 		= R8A66597_PULL_OFF,
			.direction	= R8A66597_DIRECTION_OUTPUT,
			.out_level	= R8A66597_OUT_LEVEL_HI,
		},
		.deactive = {
			.port_mux 	= GPIO_PORT217,
			.pull 		= R8A66597_PULL_OFF,
			.direction	= R8A66597_DIRECTION_NONE,
			.out_level	= R8A66597_OUT_LEVEL_NOT_SET,
		}                                   
	},
};

static struct r8a66597_platdata usbhs_func_data_d2153 = {
	.is_vbus_powered = is_vbus_powered,
	.module_start	= usbhs_module_reset,
	.on_chip	= 1,
	.buswait	= 5,
	.max_bufnum	= 0xff,
	.vbus_irq	= ENT_TPS80031_IRQ_BASE + 19,
	.pin_gpio_1_fn	= GPIO_PORT130,
	.pin_gpio_1		= GPIO_PORT130,
	.pin_gpio_2_fn	= GPIO_PORT131,
	.pin_gpio_2		= GPIO_PORT131,
	.port_cnt		= ARRAY_SIZE(r8a66597_gpio_setting_info),
	.gpio_setting_info	= &r8a66597_gpio_setting_info,
};

static struct r8a66597_platdata usbhs_func_data = {
	.is_vbus_powered = is_vbus_powered,
	.module_start	= usbhs_module_reset,
	.on_chip	= 1,
	.buswait	= 5,
	.max_bufnum	= 0xff,
#ifdef CONFIG_PMIC_INTERFACE
	.vbus_irq	= ENT_TPS80031_IRQ_BASE + TPS80031_INT_VBUS,
#else  /* CONFIG_PMIC_INTERFACE */
	.vbus_irq	= ENT_TPS80031_IRQ_BASE + TPS80031_INT_VBUS_DET,
#endif /* CONFIG_PMIC_INTERFACE */
	.pin_gpio_1_fn	= GPIO_PORT130,
	.pin_gpio_1		= GPIO_PORT130,
	.pin_gpio_2_fn	= GPIO_PORT131,
	.pin_gpio_2		= GPIO_PORT131,
	.port_cnt		= ARRAY_SIZE(r8a66597_gpio_setting_info),
	.gpio_setting_info	= &r8a66597_gpio_setting_info,
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

struct platform_device usbhs_func_device_d2153 = {
	.name	= "r8a66597_udc",
	.id	= 0,
	.dev = {
		.dma_mask		= NULL,
		.coherent_dma_mask	= DMA_BIT_MASK(32),
		.platform_data		= &usbhs_func_data_d2153,
	},
	.num_resources	= ARRAY_SIZE(usbhs_resources),
	.resource	= usbhs_resources,
};

struct platform_device usbhs_func_device = {
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
	if (power)
		pmic_set_vbus(1);
	else
		pmic_set_vbus(0);
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

struct platform_device usb_host_device = {
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
static struct tusb1211_platform_data tusb1211_data = {
	.module_start = usbhs_module_reset,
};
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

struct platform_device tusb1211_device = {
	.name = "tusb1211_driver",
	.id = 0,
	.dev = {
		.platform_data = &tusb1211_data,
	},
	.num_resources = ARRAY_SIZE(tusb1211_resource),
	.resource = tusb1211_resource,
};
#endif /*CONFIG_USB_OTG*/

void __init USBGpio_init(void)
{
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
	if (u2_get_board_rev() < 4) {
		gpio_request(GPIO_PORT131, NULL);
		gpio_direction_output(GPIO_PORT131, 0);
		udelay(100); /* assert RESET_N (min pulse width 100 usecs) */
		gpio_direction_output(GPIO_PORT131, 1);
	}

        gpio_request(GPIO_PORT130, NULL);
        gpio_direction_output(GPIO_PORT130, 1);

	/* start supplying VIO_CKO3@26MHz to REFCLK */
	gpio_request(GPIO_FN_VIO_CKO3, NULL);
	clk_enable(clk_get(NULL, "vclk3_clk"));
}
