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
#include <linux/platform_data/leds-renesas-tpu.h>

#ifdef CONFIG_PMIC_INTERFACE
#include <linux/pmic/pmic.h>
#include <linux/pmic/pmic-tps80032.h>
#endif

#ifdef CONFIG_VIBRATOR_ISA1000A
#include <linux/isa1000a_haptic.h>
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
#include <linux/vibrator.h>
#include <linux/thermal_sensor/ths_kernel.h>
#include <media/sh_mobile_rcu.h>
#include <media/soc_camera.h>
#include <media/soc_camera_platform.h>
#include <media/sh_mobile_csi2.h>
#include <linux/sh_clk.h>
#include <media/v4l2-subdev.h>
#include <linux/pmic/pmic-ncp6914.h>
#ifdef CONFIG_SOC_CAMERA_ISX012
#include <media/isx012.h>
#endif

#include <linux/sysfs.h>
#include <linux/proc_fs.h>
#if defined(CONFIG_SEC_CHARGING_FEATURE)
#include <linux/spa_power.h>
#endif
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
#ifdef CONFIG_KEYBOARD_CYPRESS_TOUCH
#include <linux/i2c/touchkey_i2c.h>
#endif


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

#include <sound/a2220.h>

static int check_sec_rlte_hw_rev(void);
#if defined(CONFIG_SEC_DEBUG_INFORM_IOTABLE)
#include <mach/sec_debug.h>
#include <mach/sec_debug_inform.h>
#endif

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

#define ENT_TPS80032_IRQ_BASE	(IRQPIN_IRQ_BASE + 64)

static void gpio_pull(u32 addr, int type)
{
	u8 data = __raw_readb(addr);

	data &= ~0xc0;
	data |= type;

	__raw_writeb(data, addr);
}

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
static unsigned int u2_board_rev;
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

void gpio_direction_none_port(int gpio)
{

	__raw_writeb(__raw_readb(GPIO_PORTCR_ES2(gpio)) & 0xcf,
	GPIO_PORTCR_ES2(gpio));
}
EXPORT_SYMBOL_GPL(gpio_direction_none_port);

void gpio_pull_off_port(int gpio)
{
	__raw_writeb(__raw_readb(GPIO_PORTCR_ES2(gpio)) & 0x3f,
	GPIO_PORTCR_ES2(gpio));
}
EXPORT_SYMBOL_GPL(gpio_pull_off_port);

void gpio_pull_up_port(int gpio)
{
	__raw_writeb((__raw_readb(GPIO_PORTCR_ES2(gpio)) & 0x3f) | 0xc0,
	GPIO_PORTCR_ES2(gpio));
}
EXPORT_SYMBOL_GPL(gpio_pull_up_port);

void gpio_pull_down_port(int gpio)
{
	__raw_writeb((__raw_readb(GPIO_PORTCR_ES2(gpio)) & 0x3f) | 0x80,
	GPIO_PORTCR_ES2(gpio));
}
EXPORT_SYMBOL_GPL(gpio_pull_down_port);

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
//Chaitanya
	#if defined(CONFIG_SAMSUNG_MHL)
	isvbus_powered_mhl(val1);
	#endif
//Chaitanya
	return val1>>7;

}

#define PHYFUNCTR	IO_ADDRESS(0xe6890104) /* 16-bit */

#define LOCK_TIME_OUT_MS 1000
static void usbhs_module_reset(void)
{
	unsigned long flags;
	int ret; 
 
	ret = hwspin_lock_timeout_irqsave(r8a73734_hwlock_cpg, LOCK_TIME_OUT_MS, &flags);
	if (ret < 0) {
		printk("Can't lock hwlock_cpg\n");
	} else {
		__raw_writel(__raw_readl(SRCR2) | (1 << 14), SRCR2); /* USBHS-DMAC */
		__raw_writel(__raw_readl(SRCR3) | (1 << 22), SRCR3); /* USBHS */
		hwspin_unlock_irqrestore(r8a73734_hwlock_cpg, &flags);
	}
	udelay(50); /* wait for at least one EXTALR cycle */
	ret = hwspin_lock_timeout_irqsave(r8a73734_hwlock_cpg, LOCK_TIME_OUT_MS, &flags);
	if (ret < 0) {
		printk("Can't lock hwlock_cpg\n");
	} else {
		__raw_writel(__raw_readl(SRCR2) & ~(1 << 14), SRCR2);
		__raw_writel(__raw_readl(SRCR3) & ~(1 << 22), SRCR3);
		hwspin_unlock_irqrestore(r8a73734_hwlock_cpg, &flags);
	}

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
	.pin_gpio_1_fn 	= GPIO_PORT130,
	.pin_gpio_1 	= GPIO_PORT130,
#ifndef CONFIG_BOARD_VERSION_V041
	.pin_gpio_2_fn	= GPIO_PORT131,
	.pin_gpio_2		= GPIO_PORT131,
#endif
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

static struct platform_device tusb1211_device = {
	.name = "tusb1211_driver",
	.id = 0,
	.dev = {
		.platform_data = &tusb1211_data,
	},
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

static void mmcif_set_pwr(struct platform_device *pdev, int state)
{
#ifdef CONFIG_PMIC_INTERFACE
	printk(" \n EOS2_BSP_MMCIF_PMIC : %s\n",__func__);
	gpio_set_value(GPIO_PORT227, 1);
	printk("\n GPIO_PORTCR_ES2(227) : %x\n",__raw_readl(0xe60520e3));
#endif
}

static void mmcif_down_pwr(struct platform_device *pdev)
{
#ifdef CONFIG_PMIC_INTERFACE
	printk(" \n EOS2_BSP_MMCIF_PMIC : %s\n",__func__);
	gpio_set_value(GPIO_PORT227, 0);
	printk("\n GPIO_PORTCR_ES2(227) : %x\n",__raw_readl(0xe60520e3));
#endif

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

#define MSEL3CR		IO_ADDRESS(0xE6058020)

static void sdhi0_set_pwr(struct platform_device *pdev, int state)
{
#ifdef CONFIG_PMIC_INTERFACE
	if(state)
	{
		printk("\n EOS2_BSP_SDHI : %s\n",__func__);
		pmic_set_power_on(E_POWER_VIO_SD);
		pmic_set_power_on(E_POWER_VMMC);
		__raw_writel(__raw_readl(MSEL3CR) | (1<<28), MSEL3CR);

	}
	else
	{
		printk("\n EOS2_BSP_SDHI : %s\n",__func__);
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

static struct renesas_sdhi_platdata sdhi0_info = {
	.caps				= 0,
	.flags				= RENESAS_SDHI_SDCLK_OFFEN | 
						RENESAS_SDHI_WP_DISABLE,
	.dma				= &sdhi0_dma,
	.set_pwr			= sdhi0_set_pwr,
	.detect_irq			= irqpin2irq(50),
	.detect_msec		= 0,
	.get_cd				= sdhi0_get_cd,
	.port_cnt			= ARRAY_SIZE(sdhi0_gpio_setting_info),
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


#define GPIO_KEY(c, g, d, w) \
	{.code = c, .gpio = g, .desc = d, .wakeup = w, .active_low = 1,\
	 .debounce_interval = 20}

static struct gpio_keys_button gpio_buttons_polled[] = {
#ifndef CONFIG_PMIC_INTERFACE
	GPIO_KEY(KEY_POWER,      GPIO_PORT24, "Power", 0),
#endif
	GPIO_KEY(KEY_HOMEPAGE,   GPIO_PORT45, "Home",  0),
	GPIO_KEY(KEY_VOLUMEUP,   GPIO_PORT46, "+",     0),
	GPIO_KEY(KEY_VOLUMEDOWN, GPIO_PORT47, "-",     0),
};

static struct gpio_keys_button gpio_buttons[] = {
#ifndef CONFIG_PMIC_INTERFACE
	GPIO_KEY(KEY_POWER,      GPIO_PORT24, "Power", 1),
#endif
	GPIO_KEY(KEY_HOMEPAGE,   GPIO_PORT18, "Home",  1),
	GPIO_KEY(KEY_VOLUMEUP,   GPIO_PORT1,  "+",     1),
	GPIO_KEY(KEY_VOLUMEDOWN, GPIO_PORT2,  "-",     1),
};

static int gpio_key_enable(struct device *dev)
{
	if((system_rev & 0xFFFF) == 0x3E00)
	{
		#ifndef CONFIG_PMIC_INTERFACE
			gpio_pull(GPIO_PORTCR_ES1(24), GPIO_PULL_UP);
		#endif
		gpio_pull(GPIO_PORTCR_ES1(25), GPIO_PULL_UP);
		gpio_pull(GPIO_PORTCR_ES1(26), GPIO_PULL_UP);
		gpio_pull(GPIO_PORTCR_ES1(27), GPIO_PULL_UP);
		gpio_pull(GPIO_PORTCR_ES1(1), GPIO_PULL_UP);
		gpio_pull(GPIO_PORTCR_ES1(2), GPIO_PULL_UP);
		gpio_pull(GPIO_PORTCR_ES1(45), GPIO_PULL_UP);
		gpio_pull(GPIO_PORTCR_ES1(46), GPIO_PULL_UP);
		gpio_pull(GPIO_PORTCR_ES1(47), GPIO_PULL_UP);
	}
	else if(((system_rev & 0xFFFF)>>4) >= 0x3E1)
	{
		#ifndef CONFIG_PMIC_INTERFACE
			gpio_pull(GPIO_PORTCR_ES2(24), GPIO_PULL_UP);
		#endif
		gpio_pull(GPIO_PORTCR_ES2(25), GPIO_PULL_UP);
		gpio_pull(GPIO_PORTCR_ES2(26), GPIO_PULL_UP);
		gpio_pull(GPIO_PORTCR_ES2(27), GPIO_PULL_UP);
		gpio_pull(GPIO_PORTCR_ES2(1), GPIO_PULL_UP);
		gpio_pull(GPIO_PORTCR_ES2(2), GPIO_PULL_UP);
		gpio_pull(GPIO_PORTCR_ES1(45), GPIO_PULL_UP);
		gpio_pull(GPIO_PORTCR_ES1(46), GPIO_PULL_UP);
		gpio_pull(GPIO_PORTCR_ES1(47), GPIO_PULL_UP);
	}
	return 0;
}

static struct gpio_keys_platform_data gpio_key_polled_info = {
	.buttons	= gpio_buttons_polled,
	.nbuttons	= ARRAY_SIZE(gpio_buttons),
	.rep		= 0,
	.enable		= gpio_key_enable,
	.poll_interval	= 50,
};

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

static struct platform_device gpio_key_polled_device = {
	.name	= "gpio-keys-polled",
	.id	= -1,
	.dev	= {
		.platform_data	= &gpio_key_polled_info,
	},
};

static const struct fb_videomode lcdc0_modes[] = {
	{
#if defined(CONFIG_FB_R_MOBILE_S6E39A0X02)
		.name		= "qHD",
		.xres		= 540,
		.yres		= 960,
#else
		.name		= "WVGA",
		.xres		= 480,
		.yres		= 800,
#endif
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
#if defined(CONFIG_VIBRATOR_ISA1000A)

/******************************************************/
/*
 * ISA1000 HAPTIC MOTOR Driver IC.
 * MOTOR Resonance frequency: 205HZ.
 * Input PWM Frequency: 205 * 128 = 26240 HZ.
 * PWM_period_ns = 1000000000/26240 = 38109.
 * PWM Enable GPIO number = 189.
*/
/******************************************************/

#define GPIO_MOTOR_EN	GPIO_PORT226

static int isa1000_enable(bool en)
{
	return gpio_direction_output(GPIO_MOTOR_EN, en);
}

static struct platform_isa1000_vibrator_data isa1000_vibrator_data =
{
	.gpio_en	= isa1000_enable,
	.pwm_name	= "TPU0TO0",
	.pwm_duty	= 542,
	.pwm_period_ns	= 580, //0x244, VIB_CYC
	.polarity	= 0,
	.regulator_id	= "vibldo_uc",
};

static struct platform_device isa1000_device =
{
	.name     = "isa1000-vibrator",
	.id       = -1,
	.dev      =
		{
		.platform_data = &isa1000_vibrator_data,
	},
};

static void isa1000_gpio_init(void)
{
	int gpio;

	gpio = GPIO_MOTOR_EN;
	gpio_request(gpio, "MOTOR_EN");
	gpio_direction_output(gpio, 1);
	gpio_export(gpio, 0);
}

static void __init isa1000_vibrator_init(void)
{
	isa1000_gpio_init();
	platform_device_register(&isa1000_device);
}
#endif


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
#define ION_HEAP_CAMERA_ADDR	0x48800000
#define ION_HEAP_GPU_SIZE	SZ_4M
#define ION_HEAP_GPU_ADDR	0x49A00000
#ifdef CONFIG_ION_R_MOBILE_USE_VIDEO_HEAP
#define ION_HEAP_VIDEO_SIZE	(SZ_16M + SZ_2M)
#define ION_HEAP_VIDEO_ADDR	0x56C00000
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

static struct tps80031_32kclock_plat_data tps_clk = {
        /*All 32k always on*/
        .en_clk32kao = 1,
        .en_clk32kg = 1,
        .en_clk32kaudio = 1,
};

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
           gpio_pull(GPIO_PORTCR_ES2(9), GPIO_PULL_OFF);
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
		vsensor_3V = regulator_get(NULL, "vdd_touch");
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

#if defined(CONFIG_SEC_CHARGING_FEATURE)
// Samsung charging feature
// +++ for board files, it may contain changeable values
struct spa_temp_tb batt_temp_tb[]=
{
	{869, -300},            /* -30 */
	{769, -200},			/* -20 */
	{643, -100},                    /* -10 */
	{568, -50},				/* -5 */
	{509,   0},                    /* 0   */
	{382,  100},                    /* 10  */
	{275,  200},                    /* 20  */
	{231,  250},                    /* 25  */
	{196,  300},                    /* 30  */
	{138,  400},                    /* 40  */
	{95 ,  500},                    /* 50  */
	{68 ,  600},                    /* 60  */
	{54 ,  650},                    /* 65  */
	{46 ,  700},            /* 70  */
	{34 ,  800},            /* 80  */
};

struct spa_power_data spa_power_pdata=
{
	.charger_name = "smb328a-charger",
	.eoc_current=100,
	.recharge_voltage=4150,
	.charging_cur_usb=500,
	.charging_cur_wall=800,
	.suspend_temp_hot=600,
	.recovery_temp_hot=400,
	.suspend_temp_cold=-50,
	.recovery_temp_cold=0,
	.charge_timer_limit=CHARGE_TIMER_6HOUR,
	.batt_temp_tb=&batt_temp_tb[0],
	.batt_temp_tb_len=ARRAY_SIZE(batt_temp_tb),
};
EXPORT_SYMBOL(spa_power_pdata);

static struct platform_device spa_power_device=
{
	.name = "spa_power",
	.id=-1,
	.dev.platform_data = &spa_power_pdata,
};
// --- for board files
#endif

#if defined(CONFIG_BATTERY_BQ27425)
#define BQ27425_ADDRESS (0xAA >> 1)
#define GPIO_FG_INT 44
#endif

#if defined(CONFIG_CHARGER_SMB328A)
#define SMB328A_ADDRESS (0x69 >> 1)
#define GPIO_CHG_INT 19
#endif

#if defined(CONFIG_RENESAS_GPS)
#define FUNC2_MODE_SCIFB 0x02

static ssize_t GNSS_NRST_value_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	int value = gpio_get_value(GPIO_PORT10);

	return sprintf(buf, "%d\n", value);
}

static ssize_t GNSS_NRST_value_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	long value;
	int  ret;

	ret = strict_strtol(buf, 0, &value);
	if (ret < 0)
		return ret;

#ifdef CONFIG_PMIC_INTERFACE

	if (1 == value)
		pmic_clk32k_enable(CLK32KG, TPS80032_STATE_ON);
	else
		pmic_clk32k_enable(CLK32KG, TPS80032_STATE_OFF);
#endif

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

	if ((system_rev & 0xFF) == 0x00) { /*ES1.0*/
		/* SCIFB1::UART mode & Function mode settings. */
		gpio_request(GPIO_FN_SCIFB1_RXD, NULL);
		gpio_pull(GPIO_PORTCR_ES1(79), GPIO_PULL_UP|FUNC2_MODE_SCIFB);
		gpio_request(GPIO_FN_SCIFB1_TXD, NULL);
		gpio_pull(GPIO_PORTCR_ES1(78), GPIO_PULL_OFF|FUNC2_MODE_SCIFB);
		gpio_request(GPIO_FN_SCIFB1_CTS, NULL);
		gpio_pull(GPIO_PORTCR_ES1(77), GPIO_PULL_UP|FUNC2_MODE_SCIFB);
		gpio_request(GPIO_FN_SCIFB1_RTS, NULL);
		gpio_pull(GPIO_PORTCR_ES1(76), GPIO_PULL_OFF|FUNC2_MODE_SCIFB);

		/* GPS Settings */
		gpio_request(GPIO_PORT10, "GNSS_NRST");
		gpio_pull(GPIO_PORTCR_ES1(10), GPIO_PULL_OFF);
		gpio_direction_output(GPIO_PORT10, 1);

		gpio_request(GPIO_PORT11, "GNSS_EN");
		gpio_pull(GPIO_PORTCR_ES1(11), GPIO_PULL_OFF);
		gpio_direction_output(GPIO_PORT11, 0);
	} else { /* ES2.0*/
		gpio_request(GPIO_FN_SCIFB1_RXD, NULL);
		gpio_pull(GPIO_PORTCR_ES2(79), GPIO_PULL_UP|FUNC2_MODE_SCIFB);
		gpio_request(GPIO_FN_SCIFB1_TXD, NULL);
		gpio_pull(GPIO_PORTCR_ES2(78), GPIO_PULL_OFF|FUNC2_MODE_SCIFB);
		gpio_request(GPIO_FN_SCIFB1_CTS, NULL);
		gpio_pull(GPIO_PORTCR_ES2(77), GPIO_PULL_UP|FUNC2_MODE_SCIFB);
		gpio_request(GPIO_FN_SCIFB1_RTS, NULL);
		gpio_pull(GPIO_PORTCR_ES2(76), GPIO_PULL_OFF|FUNC2_MODE_SCIFB);

		/* GPS Settings */
		gpio_request(GPIO_PORT10, "GNSS_NRST");
		gpio_pull(GPIO_PORTCR_ES2(10), GPIO_PULL_OFF);
		gpio_direction_output(GPIO_PORT10, 0);

		gpio_request(GPIO_PORT11, "GNSS_EN");
		gpio_pull(GPIO_PORTCR_ES2(11), GPIO_PULL_OFF);
		gpio_direction_output(GPIO_PORT11, 0);
	}

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
			.platform_data = &tsu6712_pdata,
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
	gpio_pull(GPIO_PORTCR_ES1(107), GPIO_PULL_UP);
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

		/* 5M_AF_2V8 On */
		subPMIC_PinOnOff(0x3, 1);
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
		gpio_direction_output(GPIO_PORT3, 0); /* CAM_PWR_EN */
		gpio_set_value(GPIO_PORT16, 0); /* CAM1_RST_N */
		gpio_set_value(GPIO_PORT91, 0); /* CAM1_STBY */
		gpio_set_value(GPIO_PORT20, 0); /* CAM0_RST_N */
		gpio_set_value(GPIO_PORT90, 0); /* CAM0_STBY */
		mdelay(10);
		/* 10ms */

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
		subPMIC_PinOnOff(0x3, 1);
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

		gpio_direction_output(GPIO_PORT3, 0); /* CAM_PWR_EN Low */
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

		/* 5M_AF_2V8 On */
		subPMIC_PinOnOff(0x3, 1);
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
		I2C_BOARD_INFO("ISX012", 0x1A),
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
#if defined(CONFIG_RENESAS_BT)
	&bcm4334_bluetooth_device,
#endif
	&fsi_device,
	&fsi_b_device,
	&gpio_key_device,
	&lcdc_device,
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
//	&sdhi1_device,
#if defined(CONFIG_RENESAS_BT)
	&bcm4334_bluetooth_device,
#endif
	&fsi_device,
	&fsi_b_device,
	&gpio_key_device,
	&lcdc_device,
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
#if defined(CONFIG_RENESAS_BT)
	&bcm4334_bluetooth_device,
#endif
	&fsi_device,
	&fsi_b_device,
	&gpio_key_device,
	&lcdc_device,
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

static struct tps80032_subdev_info tps80032_devs[] = {
	TPS_RTC(),
	TPS_REG(LDO5, ldo5),
};
#define PORTCR0			IO_ADDRESS(0xE6050000)
#define PORTCR28		IO_ADDRESS(0xE605001C)
#define PORTCR35		IO_ADDRESS(0xE6050023)
#define PORTCR141		IO_ADDRESS(0xE605108D)
#define PORTCR202		IO_ADDRESS(0xE60520CA)

static u8 tps80032_get_portcr_value(u32 addr)
{
	return __raw_readb(addr);
}

static void tps80032_set_portcr_value(u8 value, u32 addr)
{
	__raw_writeb(value, addr);
}
static struct tps80032_platform_data tps_platform = {
	.pin_gpio	= {GPIO_PORT0, GPIO_PORT28,
				GPIO_PORT35, GPIO_PORT141,
				GPIO_PORT202},
	.pin_gpio_fn	= {GPIO_PORT0, GPIO_PORT28,
				GPIO_PORT35, GPIO_PORT141,
				GPIO_PORT202},
	.portcr		= {PORTCR0, PORTCR28,
					PORTCR35, PORTCR141,
					PORTCR202},
	.get_portcr_value = tps80032_get_portcr_value,
	.set_portcr_value = tps80032_set_portcr_value,
	.num_subdevs	= ARRAY_SIZE(tps80032_devs),
	.subdevs	= tps80032_devs,
	.irq_base	= ENT_TPS80032_IRQ_BASE,
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
#if defined (CONFIG_MACH_U2EVM_SR_REV021) || defined (CONFIG_MACH_U2EVM_SR_REV022)

	gpio_request(GPIO_PORT29, "TCKEY_LDO");
	gpio_pull(GPIO_PORTCR_ES2(29), GPIO_PULL_OFF);
    gpio_direction_output(GPIO_PORT29,0);
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
#if defined (CONFIG_MACH_U2EVM_SR_REV021) || defined (CONFIG_MACH_U2EVM_SR_REV022)
	gpio_direction_output(GPIO_PORT29,1);
#endif
	}
	else {
		/* To do to power off */		
#if defined (CONFIG_MACH_U2EVM_SR_REV021) || defined (CONFIG_MACH_U2EVM_SR_REV022)
	gpio_direction_output(GPIO_PORT29,0);
#endif
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
#define TCKEY_SDA 27
#define TCKEY_SCL 26
static struct touchkey_platform_data touchkey_pdata = {
	.gpio_sda = TCKEY_SDA,	/* To do to set gpio */
	.gpio_scl = TCKEY_SCL,	/* To do to set gpio */
	.gpio_int = NULL,	/* To do to set gpio */
	.init_platform_hw = touchkey_init_hw,
	.suspend = touchkey_suspend,
	.resume = touchkey_resume,
	.power_on = touchkey_power_on,
	.led_power_on = touchkey_led_power_on,
};


static struct i2c_board_info i2c_touchkey[] = {
#ifdef CONFIG_KEYBOARD_CYPRESS_TOUCH
	{
		I2C_BOARD_INFO("sec_touchkey", 0x20),
		.platform_data = &touchkey_pdata,
		.irq = irqpin2irq(43),
	},

};
#endif /*CONFIG_KEYBOARD_CYPRESS_TOUCH*/
#endif
#ifndef CONFIG_PMIC_INTERFACE
	static struct regulator *mxt224_regulator;
#endif

static void mxt224_set_power(int on)
{
#ifdef CONFIG_PMIC_INTERFACE
//	pmic_set_power_on(E_POWER_VANA_MM);
	if(on)
	{
		gpio_set_value(GPIO_PORT29, 1);
		gpio_set_value(GPIO_PORT30, 1);
	}
	else
	{
		gpio_set_value(GPIO_PORT29, 0 );
		gpio_set_value(GPIO_PORT30, 0 );		
	}
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
	.set_pwr	= mxt224_set_power,
	.read_chg	= mxt224_read_chg,
};

struct a2220_platform_data  u2evm_a2220_data = {
	.a2220_hw_init = NULL,
	.gpio_reset = GPIO_PORT44,
	.gpio_wakeup = GPIO_PORT26,
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
	{
		I2C_BOARD_INFO("audience_a2220", 0x3E),
		.platform_data = &u2evm_a2220_data,
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

#define DSI0PHYCR	IO_ADDRESS(0xe615006c)
#define SBAR2		__io(IO_ADDRESS(0xe6180060))
#define RESCNT2		__io(IO_ADDRESS(0xe6188020))

#define SEC_RLTE_REV0_0_0		0
#define SEC_RLTE_REV0_2_1		1
#define SEC_RLTE_REV0_2_2		2
#define SEC_RLTE_REV0_3_1		3
#define SEC_RLTE_REV0_4_0		4

void u2evm_restart(char mode, const char *cmd)
{
	u8 reg = __raw_readb(STBCHR2);
	__raw_writeb((reg | APE_RESETLOG_U2EVM_RESTART), STBCHR2); // write STBCHR2 for debug

	__raw_writel(0, SBAR2);
	__raw_writel(__raw_readl(RESCNT2) | (1 << 31), RESCNT2);
}
int sec_rlte_hw_rev;

 
 
#define STBCHRB3		0xE6180043
/* SBSC register address */
#define SBSC_BASE		(0xFE000000U)
#define SBSC_SDMRA_DONE		(0x00000000)
#define SBSC_SDMRACR1A_ZQ	(0x0000560A)
/* CPG register address */
#define CPG_BASE		(0xE6150000U)
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
	struct platform_device **p_dev;
	int p_dev_cnt;
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
	{
		volatile uint32_t val;
		
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
	}

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
#else
	if(0) /* Without TZ, kernel may override also on ES2.02 and onwards */
	{
#endif
		printk("ES2.02 on later with TZ enabled\n");
		pub_stm_select = 0; /* Can't override secure side by public side any more */
	} else {
		printk("ES2.01 or earlier or TZ disabled\n");
		if (stm_select != pub_stm_select) {
			stm_select = pub_stm_select;
			pub_stm_select = 1; /* Override secure side by public side */
		} else {
			pub_stm_select = 0; /* Both secure and public agree. No need to change HW setup */
		}
	}

	printk("final stm_select=%d\n", stm_select);


	if(((system_rev & 0xFFFF)>>4) >= 0x3E1)
	{
#define GPIO_DRVCR_SD0	((volatile ushort *)(0xE6050000ul + 0x818E))
#define GPIO_DRVCR_SIM1	((volatile ushort *)(0xE6050000ul + 0x8192))
#define GPIO_DRVCR_SIM2	((volatile ushort *)(0xE6050000ul + 0x8194))

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

	create_proc_read_entry("board_revision", 0400, NULL,
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

	if((system_rev & 0xFF) == 0x00) /*ES1.0*/
	{
		gpio_pull(GPIO_PORTCR_ES1(138), GPIO_PULL_UP); /* RX PU */
		gpio_pull(GPIO_PORTCR_ES1(137), GPIO_PULL_DOWN); /* TX PD */
		gpio_pull(GPIO_PORTCR_ES1(38), GPIO_PULL_UP); /* CTS PU */
		gpio_pull(GPIO_PORTCR_ES1(37), GPIO_PULL_DOWN); /* RTS PD */
	} else
	{
		gpio_pull(GPIO_PORTCR_ES2(138), GPIO_PULL_UP); /* RX PU */
		gpio_pull(GPIO_PORTCR_ES2(137), GPIO_PULL_DOWN); /* TX PD */
		gpio_pull(GPIO_PORTCR_ES2(38), GPIO_PULL_UP); /* CTS PU */
		gpio_pull(GPIO_PORTCR_ES2(37), GPIO_PULL_DOWN); /* RTS PD */
	}

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


if((system_rev & 0xFFFF) == 0x3E00)
{	
	gpio_pull(GPIO_PORTCR_ES1(44), GPIO_PULL_UP);
	gpio_pull(GPIO_PORTCR_ES1(45), GPIO_PULL_UP);
	gpio_pull(GPIO_PORTCR_ES1(46), GPIO_PULL_UP);
	gpio_pull(GPIO_PORTCR_ES1(47), GPIO_PULL_UP);
	gpio_pull(GPIO_PORTCR_ES1(48), GPIO_PULL_UP);
	gpio_pull(GPIO_PORTCR_ES1(96), GPIO_PULL_UP);
	gpio_pull(GPIO_PORTCR_ES1(97), GPIO_PULL_UP);
}
else if(((system_rev & 0xFFFF)>>4) >= 0x3E1)
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
	gpio_request(GPIO_PORT260, NULL);
	gpio_direction_output(GPIO_PORT260, 0);
	
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
if((system_rev & 0xFFFF) == 0x3E00)
        gpio_pull(GPIO_PORTCR_ES1(293), GPIO_PULL_UP);
else if((system_rev & 0xFFFF>>4) >= 0x3E1)
	gpio_pull(GPIO_PORTCR_ES2(293), GPIO_PULL_UP);

	}
	
	if (0 == stm_select) {
        /* SDHI0 used for STMSIDI */
//        gpio_request(GPIO_PORT324, NULL);
//        gpio_direction_input(GPIO_PORT324);
        gpio_request(GPIO_FN_STMSIDI_1, NULL);
if((system_rev & 0xFFFF) == 0x3E00)
        gpio_pull(GPIO_PORTCR_ES1(324), GPIO_PULL_UP);
else if(((system_rev & 0xFFFF)>>4) >= 0x3E1)
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
	    irq_set_irq_type(irqpin2irq(42), IRQ_TYPE_EDGE_FALLING);
	    irqc_set_chattering(42, 0x01);  /* 1msec */
	

	}

	/* touch key Interupt */
	gpio_request(GPIO_PORT104, NULL);
	gpio_direction_input(GPIO_PORT104);
	/* emmc ldo enable*/
	gpio_request(GPIO_PORT227, NULL);
	gpio_direction_output(GPIO_PORT227, 1);




if((system_rev & 0xFFFF) == 0x3E00)

{	
	gpio_pull(GPIO_PORTCR_ES1(104), GPIO_PULL_UP);
}
else if(((system_rev & 0xFFFF)>>4) >= 0x3E1)
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

#if defined(CONFIG_USB_SWITCH_TSU6712)
   gpio_request(GPIO_PORT97, NULL);
   gpio_direction_input(GPIO_PORT97);
   gpio_pull(GPIO_PORTCR_ES2(97), GPIO_PULL_UP);
#endif
#if defined(CONFIG_CHARGER_SMB328A)
	if(SEC_RLTE_REV0_2_2 == sec_rlte_hw_rev)
	{
	   gpio_request(GPIO_PORT103, NULL);
	   gpio_direction_input(GPIO_PORT103);
	   gpio_pull(GPIO_PORTCR_ES2(103), GPIO_PULL_UP);
	}
	else if(SEC_RLTE_REV0_3_1 == sec_rlte_hw_rev)
	{
	   gpio_request(GPIO_PORT19, NULL);
	   gpio_direction_input(GPIO_PORT19);
	   gpio_pull(GPIO_PORTCR_ES2(19), GPIO_PULL_UP);
	}
	else
	{
	   gpio_request(GPIO_PORT19, NULL);
	   gpio_direction_input(GPIO_PORT19);
	   gpio_pull(GPIO_PORTCR_ES2(19), GPIO_PULL_UP);
	}
#endif

#if defined(CONFIG_BATTERY_BQ27425)
   gpio_request(GPIO_PORT105, NULL);
   gpio_direction_input(GPIO_PORT105);
   gpio_pull(GPIO_PORTCR_ES2(105), GPIO_PULL_UP);
#endif
#if defined(CONFIG_CHARGER_SMB328A)
	if(SEC_RLTE_REV0_2_2 == sec_rlte_hw_rev)
	{
	   gpio_request(GPIO_PORT103, NULL);
	   gpio_direction_input(GPIO_PORT103);
	   gpio_pull(GPIO_PORTCR_ES2(103), GPIO_PULL_UP);
	}
	else if(SEC_RLTE_REV0_3_1 == sec_rlte_hw_rev)
	{
	   gpio_request(GPIO_PORT19, NULL);
	   gpio_direction_input(GPIO_PORT19);
	   gpio_pull(GPIO_PORTCR_ES2(19), GPIO_PULL_UP);
	}
	else
	{
	   gpio_request(GPIO_PORT19, NULL);
	   gpio_direction_input(GPIO_PORT19);
	   gpio_pull(GPIO_PORTCR_ES2(19), GPIO_PULL_UP);
	}
#endif

#if defined(CONFIG_BATTERY_BQ27425)
   gpio_request(GPIO_PORT105, NULL);
   gpio_direction_input(GPIO_PORT105);
   gpio_pull(GPIO_PORTCR_ES2(105), GPIO_PULL_UP);
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
if((system_rev & 0xFFFF) == 0x3E00)
	gpio_pull(GPIO_PORTCR_ES1(32), GPIO_PULL_UP);
else if(((system_rev & 0xFFFF)>>4) >= 0x3E1)
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
#ifndef CONFIG_BOARD_VERSION_V041
	gpio_request(GPIO_PORT131, NULL);
	gpio_direction_output(GPIO_PORT131, 0);
	udelay(100); /* assert RESET_N (minimum pulse width 100 usecs) */
	gpio_direction_output(GPIO_PORT131, 1);
#endif

	gpio_request(GPIO_PORT130, NULL);
	gpio_direction_output(GPIO_PORT130, 1);

	/* start supplying VIO_CKO3@26MHz to REFCLK */
	gpio_request(GPIO_FN_VIO_CKO3, NULL);
	clk_enable(clk_get(NULL, "vclk3_clk"));

#ifdef CONFIG_SPI_SH_MSIOF
	/* enable MSIOF0 */
	gpio_request(GPIO_FN_MSIOF0_TXD, NULL);
	gpio_request(GPIO_FN_MSIOF0_SYNC, NULL);
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

	gpio_request(GPIO_PORT24, NULL);
	gpio_direction_input(GPIO_PORT24);
	gpio_pull(GPIO_PORTCR_ES2(24), GPIO_PULL_DOWN);

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

	switch (stm_select) {
		case 0:
			p_dev = u2evm_devices_stm_sdhi0;
			p_dev_cnt = ARRAY_SIZE(u2evm_devices_stm_sdhi0);
			break;
		case 1:
			p_dev = u2evm_devices_stm_sdhi1;
			p_dev_cnt = ARRAY_SIZE(u2evm_devices_stm_sdhi1);
			break;
		default:
			p_dev = u2evm_devices_stm_none;
			p_dev_cnt = ARRAY_SIZE(u2evm_devices_stm_none);
			break;
	}

	if (u2_board_rev < 3) {
		int i;
		for (i = 0; i < p_dev_cnt; i++) {
			if (strncmp(p_dev[i]->name, "gpio-keys", 9) == 0) {
				p_dev[i] = &gpio_key_polled_device;
				break;
			}
		}
	}
	platform_add_devices(p_dev, p_dev_cnt);
#if defined(CONFIG_MPU_SENSORS_MPU6050B1)
	mpl_init();
#endif

#if defined (CONFIG_KEYBOARD_CYPRESS_TOUCH)
	touchkey_init_hw();
#endif

#if defined (CONFIG_SAMSUNG_MHL)
	board_mhl_init();
#endif

	i2c_register_board_info(0, i2c0_devices, ARRAY_SIZE(i2c0_devices));
	i2c_register_board_info(2, i2c2_devices, ARRAY_SIZE(i2c2_devices));
	i2c_register_board_info(3, i2c3_devices, ARRAY_SIZE(i2c3_devices));
	i2c_register_board_info(4, i2c4_devices, ARRAY_SIZE(i2c4_devices));
            i2c_register_board_info(6, i2cm_devices, ARRAY_SIZE(i2cm_devices));
#ifdef CONFIG_PN544_NFC
	i2c_register_board_info(8, pn544_info, ARRAY_SIZE(pn544_info)); //PATCH CPN
#endif	

	/* GPS Init */
#if defined(CONFIG_RENESAS_GPS)
	gps_gpio_init();
#endif

platform_add_devices(gpio_i2c_devices, ARRAY_SIZE(gpio_i2c_devices));	
	#if defined(CONFIG_VIBRATOR_ISA1000A)
    isa1000_vibrator_init();
#endif

#if defined(CONFIG_SEC_CHARGING_FEATURE)
	platform_device_register(&spa_power_device);
#endif
	printk(KERN_DEBUG "%s \n", __func__);
	crashlog_r_local_ver_write(mmcoops_info.soft_version);
	crashlog_reset_log_write();
	crashlog_init_tmplog();
	i2c_register_board_info(10, i2c_touchkey, ARRAY_SIZE(i2c_touchkey)); //For TOUCHKEY

	i2c_register_board_info(9, i2c9gpio_devices, ARRAY_SIZE(i2c9gpio_devices));
	i2c_register_board_info(6, i2cm_devices, ARRAY_SIZE(i2cm_devices));
#if defined(CONFIG_RENESAS_NFC)
#ifdef CONFIG_PN544_NFC
	i2c_register_board_info(8, pn544_info, ARRAY_SIZE(pn544_info)); 
#endif
#endif

/* Tentative workaround to set thermal sensor idle. To be removed when thermal sensor driver is enabled */
__raw_writel((__raw_readl(__io(0xE61F012C)) | 0x00000300), __io(0xE61F012C)); 
__raw_writel((__raw_readl(__io(0xE61F022C)) | 0x00000300), __io(0xE61F022C));
}

#ifdef ARCH_HAS_READ_CURRENT_TIMER

/* CMT13 */
#define CMSTR3		IO_ADDRESS(0xe6130300)
#define CMCSR3		IO_ADDRESS(0xe6130310)
#define CMCNT3		IO_ADDRESS(0xe6130314)
#define CMCOR3		IO_ADDRESS(0xe6130318)
#define CMCLKE		IO_ADDRESS(0xe6131000)

extern spinlock_t	sh_cmt_lock; /* arch/arm/mach-shmobile/sh_cmt.c */

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

	spin_lock_irqsave(&sh_cmt_lock, flags);
	__raw_writel(__raw_readl(CMCLKE) | (1 << 3), CMCLKE);
	spin_unlock_irqrestore(&sh_cmt_lock, flags);

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
