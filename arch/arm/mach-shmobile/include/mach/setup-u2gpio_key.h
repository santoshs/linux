#ifndef __ASM_ARCH_GPIO_KEY_H
#define __ASM_ARCH_GPIO_KEY_H

#ifdef CONFIG_KEYBOARD_SH_KEYSC
extern struct platform_device keysc_device;
#endif
extern struct platform_device gpio_key_device;
extern void gpio_pull(u32 addr, int type);
extern void gpio_direction_none_port(int gpio);
extern void gpio_pull_off_port(int gpio);
extern void gpio_pull_up_port(int gpio);
extern void gpio_pull_down_port(int gpio);
int gpio_key_init(int stm_select,
		unsigned int u2_board_rev,
		int sec_rlte_hw_rev,
		struct platform_device **u2evm_devices_stm_sdhi0,
		int u2evm_devices_stm_sdhi0_size,
		struct platform_device **u2evm_devices_stm_sdhi1,
		int u2evm_devices_stm_sdhi1_size,
		struct platform_device **u2evm_devices_stm_none,
		int u2evm_devices_stm_none_size);
#endif /* __ASM_ARCH_GPIO_KEY_H */
