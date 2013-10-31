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
	unsigned short inx = 0;
	struct gpio_keys_platform_data *pdata = dev->platform_data;
	for (inx = 0; inx < pdata->nbuttons; inx++)
		gpio_pull_up_port(pdata->buttons[inx].gpio);
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

int gpio_key_init(int stm_select,
		unsigned int u2_board_rev,
		struct platform_device **u2evm_devices_stm_sdhi0,
		int u2evm_devices_stm_sdhi0_size,
		struct platform_device **u2evm_devices_stm_sdhi1,
		int u2evm_devices_stm_sdhi1_size,
		struct platform_device **u2evm_devices_stm_none,
		int u2evm_devices_stm_none_size) {

	struct platform_device **p_dev;
	int p_dev_cnt;
	switch (stm_select) {
	case 0:
			p_dev = u2evm_devices_stm_sdhi0;
			p_dev_cnt = u2evm_devices_stm_sdhi0_size;
			break;
	case 1:
			p_dev = u2evm_devices_stm_sdhi1;
			p_dev_cnt = u2evm_devices_stm_sdhi1_size;
			break;
	default:
			p_dev = u2evm_devices_stm_none;
			p_dev_cnt = u2evm_devices_stm_none_size;
			break;
	}
	platform_add_devices(p_dev, p_dev_cnt);

	return 0;
}
