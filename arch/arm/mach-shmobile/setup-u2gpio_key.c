#include <asm/system.h>
#include <mach/r8a7373.h>
#include <mach/gpio.h>
#include <mach/irqs.h>
#include <mach/setup-u2gpio_key.h>
#include <mach/common.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/input/sh_keysc.h>
#include <linux/platform_device.h>

#ifdef CONFIG_KEYBOARD_SH_KEYSC
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

struct platform_device keysc_device = {
	.name		= "sh_keysc",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(keysc_resources),
	.resource	= keysc_resources,
	.dev		= {
		.platform_data	= &keysc_platdata,
	},
};
#endif

static int gpio_key_enable(struct device *dev)
{
	gpio_pull_up_port(GPIO_PORT18);
	gpio_pull_up_port(GPIO_PORT1);
	gpio_pull_up_port(GPIO_PORT2);
	return 0;
}

static struct gpio_keys_platform_data gpio_key_info = {
	.buttons	= NULL,
	.nbuttons	= 0,
	.rep		= 0,
	.enable		= gpio_key_enable,
};

struct platform_device gpio_key_device = {
	.name	= "gpio-keys",
	.id	= -1,
	.dev	= {
		.platform_data	= &gpio_key_info,
	},
};
