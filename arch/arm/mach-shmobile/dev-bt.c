#include <linux/platform_device.h>
#include <linux/broadcom/bcm-bt-rfkill.h>
#include <linux/broadcom/bcm-bt-lpm.h>
#ifdef CONFIG_BCM_BZHW
#include <linux/broadcom/bcm_bzhw.h>
#endif
#include <mach/dev-bt.h>
#include <mach/r8a7373.h>

static struct bcm_bt_rfkill_platform_data bcm_bt_rfkill_data = {
	.bcm_bt_rfkill_vreg_gpio = -1,
	.bcm_bt_rfkill_n_reset_gpio = -1,
};

static struct platform_device board_bcmbt_rfkill_device = {
	.name = "bcm-bt-rfkill",
	.id   = -1,
	.dev  = {
		.platform_data = &bcm_bt_rfkill_data,
	},
};

void __init add_bcmbt_rfkill_device(int vreg_gpio, int n_reset_gpio)
{
	int ret;
	bcm_bt_rfkill_data.bcm_bt_rfkill_vreg_gpio = vreg_gpio;
	bcm_bt_rfkill_data.bcm_bt_rfkill_n_reset_gpio = n_reset_gpio;
	ret = platform_device_register(&board_bcmbt_rfkill_device);
	if (ret)
		pr_err("%s failed to add bcm_bt_rfkill_data %d",
				__func__, ret);
}

#ifdef CONFIG_BCM_BZHW
static struct bcm_bzhw_platform_data bcm_bzhw_data = {
	.gpio_bt_wake   = -1,
	.gpio_host_wake = -1,
};

static struct platform_device board_bcm_bzhw_device = {
	.name = "bcm_bzhw",
	.id = -1,
	.dev = {
		.platform_data = &bcm_bzhw_data,
	},
};

void __init add_bcm_bzhw_device(int gpio_bt_wake, int gpio_host_wake)
{
	bcm_bzhw_data.bt_wake_gpio = gpio_bt_wake;
	bcm_bzhw_data.host_wake_gpio = gpio_host_wake;
	int ret = platform_device_register(&board_bcmbt_lpm_device);
	if (ret)
		pr_err("%s failed to add board_bcm_bzhw_device %d",
				__func__, ret);
}

#endif


static struct bcm_bt_lpm_platform_data brcm_bt_lpm_data = {
	.bt_wake_gpio = -1,
	.host_wake_gpio = -1,
};

static struct platform_device board_bcmbt_lpm_device = {
	.name = "bcm-bt-lpm",
	.id   = -1,
	.dev  = {
		.platform_data = &brcm_bt_lpm_data,
	},
};

void __init add_bcmbt_lpm_device(int gpio_bt_wake, int gpio_host_wake)
{
	int ret;
	brcm_bt_lpm_data.bt_wake_gpio = gpio_bt_wake;
	brcm_bt_lpm_data.host_wake_gpio = gpio_host_wake;
	ret = platform_device_register(&board_bcmbt_lpm_device);
	if (ret)
		pr_err("%s failed to add board_bcmbt_lpm_device %d",
				__func__, ret);
}
