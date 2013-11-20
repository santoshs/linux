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
void __init u2_add_gpio_key_devices(struct gpio_keys_button *gpio_buttons,
		int nbuttons);
#endif /* __ASM_ARCH_GPIO_KEY_H */
