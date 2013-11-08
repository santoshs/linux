#ifndef __ASM_ARCH_GPIO_KEY_H
#define __ASM_ARCH_GPIO_KEY_H

#include <linux/input.h>
#include <linux/gpio_keys.h>
#ifdef CONFIG_KEYBOARD_SH_KEYSC
extern struct platform_device keysc_device;
#endif
#define GPIO_KEY(c, g, d, w) \
	{.code = c, .gpio = g, .desc = d, .wakeup = w, .active_low = 1,\
	 .debounce_interval = 20}
extern struct platform_device gpio_key_device;
int gpio_key_init(int stm_select,
		struct platform_device **u2evm_devices_stm_sdhi0,
		int u2evm_devices_stm_sdhi0_size,
		struct platform_device **u2evm_devices_stm_sdhi1,
		int u2evm_devices_stm_sdhi1_size,
		struct platform_device **u2evm_devices_stm_none,
		int u2evm_devices_stm_none_size);
#endif /* __ASM_ARCH_GPIO_KEY_H */
