#include <linux/platform_device.h>
#include <linux/broadcom/bcm-bt-rfkill.h>
#include <linux/broadcom/bcm-bt-lpm.h>
#ifdef CONFIG_BCM_BZHW
#include <linux/broadcom/bcm_bzhw.h>
#endif
#include <mach/dev-bt.h>
#include <mach/r8a7373.h>

void __init add_bcmbt_rfkill_device(int vreg_gpio, int n_reset_gpio)
{
	struct bcm_bt_rfkill_platform_data data = {
		.bcm_bt_rfkill_vreg_gpio = vreg_gpio,
		.bcm_bt_rfkill_n_reset_gpio = n_reset_gpio,
	};
	int ret = platform_device_register_data(NULL, "bcm-bt-rfkill", -1,
						&data, sizeof data);
	if (ret)
		pr_err("failed to register bcm-bt-rfkill device %d", ret);
}

#ifdef CONFIG_BCM_BZHW
void __init add_bcm_bzhw_device(int gpio_bt_wake, int gpio_host_wake)
{
	struct bcm_bzhw_platform_data data = {
		.gpio_bt_wake   = gpio_bt_wake,
		.gpio_host_wake = gpio_host_wake,
	};
	int ret = platform_device_register_data(NULL, "bcm_bzhw", -1,
						&data, sizeof data);
	if (ret)
		pr_err("failed to register bcm_bzhw device %d", ret);
}

#endif


void __init add_bcmbt_lpm_device(int gpio_bt_wake, int gpio_host_wake)
{
	struct bcm_bt_lpm_platform_data data = {
		.bt_wake_gpio = gpio_bt_wake,
		.host_wake_gpio = gpio_host_wake,
	};
	int ret = platform_device_register_data(NULL, "bcm-bt-lpm", -1,
						&data, sizeof data);
	if (ret)
		pr_err("failed to register bcm-bt-lpm device %d", ret);
}
