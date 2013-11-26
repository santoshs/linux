/*
 * Copyright (C) 2013 Broadcom Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <mach/gpio.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/skbuff.h>
#include "wlan_perf.h"
#include <linux/bcm_wlan.h>

static struct sk_buff *wlan_static_skb[WLAN_SKB_BUF_NUM];
static struct wlan_sdio_kona_cfg *wlan_cfg;
#if DEV_TREE
static const struct of_device_id wlan_sdio_driver[] = {
	{ .compatible = "brcm,wlan-sdio" },
	{}
};
MODULE_DEVICE_TABLE(of, wlan_sdio_driver);
#endif
struct wlan_mem_prealloc {
	void *mem_ptr;
	size_t size;
};

static struct wlan_mem_prealloc wlan_mem_array[PREALLOC_WLAN_SEC_NUM] = {
	{NULL, (WLAN_SECTION_SIZE_0 + PREALLOC_WLAN_SECTION_HEADER)},
	{NULL, (WLAN_SECTION_SIZE_1 + PREALLOC_WLAN_SECTION_HEADER)},
	{NULL, (WLAN_SECTION_SIZE_2 + PREALLOC_WLAN_SECTION_HEADER)},
	{NULL, (WLAN_SECTION_SIZE_3 + PREALLOC_WLAN_SECTION_HEADER)}
};

static void *wlan_static_scan_buf0;
static void *wlan_static_scan_buf1;

/* wlan_sdio_deallocate_memory
	Clear all statically assigned buffers
*/
static int wlan_sdio_deallocate_memory(void)
{
	int buff_count, count;

	pr_debug("%s: Enter\n", __func__);

	for (buff_count = 0; buff_count < NUM_BUF_1PAGE; buff_count++)
		dev_kfree_skb(wlan_static_skb[buff_count]);

	for (; buff_count < (NUM_BUF_2PAGE + NUM_BUF_1PAGE); buff_count++)
		dev_kfree_skb(wlan_static_skb[buff_count]);

	dev_kfree_skb(wlan_static_skb[buff_count]);

	for (count = 0; count < ARRAY_SIZE(wlan_mem_array); count++)
		kfree(wlan_mem_array[count].mem_ptr);

	kfree(wlan_static_scan_buf0);
	kfree(wlan_static_scan_buf1);

	pr_debug("%s: wlan MEM de-Allocated\n", __func__);
	return 0;
}

/* wlan_sdio_allocate_memory
	Allocate static buffers for data and DHD ioctl
*/
static int wlan_sdio_allocate_memory(void)
{
	int buff_count, count;
	int j;

	pr_debug("%s: Enter\n", __func__);

	for (buff_count = 0; buff_count < NUM_BUF_1PAGE; buff_count++) {
		wlan_static_skb[buff_count] =
			dev_alloc_skb(DHD_SKB_1PAGE_BUFSIZE);
		if (!wlan_static_skb[buff_count])
			goto err_skb_alloc;
	}

	for (; buff_count < (NUM_BUF_2PAGE + NUM_BUF_1PAGE); buff_count++) {
		wlan_static_skb[buff_count] =
			dev_alloc_skb(DHD_SKB_2PAGE_BUFSIZE);
		if (!wlan_static_skb[buff_count])
			goto err_skb_alloc;
	}

	wlan_static_skb[buff_count] = dev_alloc_skb(DHD_SKB_4PAGE_BUFSIZE);
	if (!wlan_static_skb[buff_count])
		goto err_skb_alloc;

	for (count = 0; count < ARRAY_SIZE(wlan_mem_array); count++) {
		wlan_mem_array[count].mem_ptr =
			kmalloc(wlan_mem_array[count].size, GFP_KERNEL);
		if (!wlan_mem_array[count].mem_ptr)
			goto err_mem_alloc;
	}
	wlan_static_scan_buf0 = kmalloc(SCAN_BUFFER, GFP_KERNEL);
	if (!wlan_static_scan_buf0)
		goto err_mem_alloc;
	wlan_static_scan_buf1 = kmalloc(SCAN_BUFFER, GFP_KERNEL);
	if (!wlan_static_scan_buf1) {
		kfree(wlan_static_scan_buf0);
		goto err_mem_alloc;
	}
	pr_debug("%s: wlan MEM Allocated\n", __func__);

	goto success;

err_mem_alloc:
	pr_err("%s: Failed to mem_alloc for WLAN\n",
			__func__);
	for (j = 0; j < count; j++)
		kfree(wlan_mem_array[j].mem_ptr);

err_skb_alloc:
	pr_err("%s: Failed to skb_alloc for WLAN\n",
			__func__);
	for (j = 0; j < buff_count; j++)
		dev_kfree_skb(wlan_static_skb[j]);

	return -ENOMEM;
success:
	pr_debug("%s: wlan MEM Allocated\n",
			__func__);
	return 0;
}
/* wlan_sdio_performance_power
	Enable disable wlan chipset
*/
static int wlan_sdio_performance_power(int onoff)
{
	pr_debug("%s: ENTRY onoff=%d\n",
			__func__, onoff);

	if (onoff) {
		pr_debug("%s: calling wlan_sdio_power_up\n",
			__func__);
		return wlan_sdio_power_up(wlan_cfg, onoff);
	} else {
		pr_debug("%s: calling wlan_sdio_power_down\n",
			__func__);
		return wlan_sdio_power_down(wlan_cfg);
	}
}

/* wlan_sdio_performance_mem_prealloc
	assign preallocated buffer pointers to DHD driver
*/
static void *wlan_sdio_performance_mem_prealloc(int section, unsigned long size)
{
	if (section == PREALLOC_WLAN_SEC_NUM)
		return wlan_static_skb;
	if (section == WLAN_STATIC_SCAN_BUF0)
		return wlan_static_scan_buf0;
	if (section == WLAN_STATIC_SCAN_BUF1)
		return wlan_static_scan_buf1;
	if ((section < 0) || (section > PREALLOC_WLAN_SEC_NUM))
		return NULL;

	if (wlan_mem_array[section].size < size)
		return NULL;

	return wlan_mem_array[section].mem_ptr;
}
/* wlan_sdio_performance_reset
	not used, will be modified in future revision
*/
static int wlan_sdio_performance_reset(int on)
{
	pr_debug("%s: Enter\n", __func__);
	return 0;
}
/* wlan_sdio_performance_carddetect
	Route card emulate API from DHD driver to sdhci
*/
static int wlan_sdio_performance_carddetect(int insert)
{
	pr_debug("%s: Enter\n", __func__);
#if BRCM_SDIO
	wlan_sdio_card_emulate(wlan_cfg, insert);
#endif
	return 0;
}
/*  wlan_sdio_performance_get_mac_addr
*/
static int wlan_sdio_performance_get_mac_addr(unsigned char *buf)
{
	u8 wlan_mac_addr[6] = {0, 0x90, 0x4c, 0, 0, 0};
	u32 rand_mac;
	pr_debug("%s: Enter\n", __func__);

	prandom_seed((unsigned int) jiffies);
	rand_mac = prandom_u32();

	wlan_mac_addr[3] = (u8)rand_mac;
	wlan_mac_addr[4] = (u8)(rand_mac >> 8);
	wlan_mac_addr[5] = (u8)(rand_mac >> 16);
	memcpy(buf, wlan_mac_addr, sizeof(wlan_mac_addr));

	return 0;
}
/*  wlan_sdio_performance_country_code
	not used, will be modified in future revision
*/
static void *wlan_sdio_performance_country_code(char *ccode)
{
	pr_debug("%s: Enter\n", __func__);
	return 0;
}
/*	wifi_platform_data
	Platform data needed for DHD functionality
*/
static struct wifi_platform_data wlan_sdio_performance_control = {
	.set_power = wlan_sdio_performance_power,
	.set_reset = wlan_sdio_performance_reset,
	.set_carddetect = wlan_sdio_performance_carddetect,
	.mem_prealloc = wlan_sdio_performance_mem_prealloc,
	.get_mac_addr = wlan_sdio_performance_get_mac_addr,
	.get_country_code = wlan_sdio_performance_country_code,
};
/* wlan IRQ resource
*/
static struct resource wlan_sdio_performance_resource[] = {
	[0] = {
	       .name = "bcmdhd_wlan_irq",
	       .start = -1,
	       .end = -1,
	       .flags = IORESOURCE_IRQ |  IORESOURCE_IRQ_LOWEDGE
	       /* IORESOURCE_IRQ_HIGHLEVEL */  | IORESOURCE_IRQ_SHAREABLE,
	       },
};
/*platform device -- DHD driver
*/
static struct platform_device wlan_sdio_performance_device = {
	.name = "bcmdhd_wlan",
	.id = 2,
	.resource = wlan_sdio_performance_resource,
	.num_resources = ARRAY_SIZE(wlan_sdio_performance_resource),
	.dev = {
		.platform_data = &wlan_sdio_performance_control,
		},
};
/* wlan_sdio_parse_dt
	parse wlan device structure from device tree
*/
static struct wlan_sdio_kona_cfg *wlan_sdio_parse_dt(
			struct platform_device *pdev)
{
	struct wlan_sdio_kona_cfg *cfg = NULL;
#if DEV_TREE
	struct device_node *np = pdev->dev.of_node;
	pr_debug("%s: Enter\n", __func__);
	if (!np)
		return NULL;
#endif
		cfg = devm_kzalloc(&pdev->dev, sizeof(*cfg), GFP_KERNEL);
	if (!cfg) {
		pr_err("%s: Can't allocate platform cfg\n",
			__func__);
		return NULL;
	}
#if DEV_TREE
	cfg->bcm_wlan_host_wake =
		of_get_named_gpio(np, "bcm-wlan-host-wake", 0);
	cfg->bcm_wlan_reset =
		of_get_named_gpio(np, "bcm-wlan-reset", 0);
#else
	cfg->bcm_wlan_host_wake =
		((struct wlan_plat_data*)(pdev->dev.platform_data))->host_wake_gpio;
	cfg->bcm_wlan_reset =
		((struct wlan_plat_data*)(pdev->dev.platform_data))->wl_reset_gpio;

#endif
	return cfg;
}
/* wlan_sdio_performance_probe
	probe function for wlan device
*/
static int wlan_sdio_performance_probe(struct platform_device *pdev)
{
#if DEV_TREE
	const struct of_device_id *match = NULL;
#endif
	int ret = 0;
	pr_debug("Enter\n");
#if DEV_TREE
	match = of_match_device(wlan_sdio_driver, &pdev->dev);
	if (!match) {
		pr_err("%s: No matcing device found\n",
			__func__);
		return -ENODEV;
	}
#endif
	wlan_cfg = wlan_sdio_parse_dt(pdev);
	if (!wlan_cfg) {
		pr_err("%s: parsing device tree failed\n",
			__func__);
		return -ENODEV;
	}
	pr_debug("%s: cfg is host wake = %d reset = %d\n",
		__func__,
		wlan_cfg->bcm_wlan_host_wake, wlan_cfg->bcm_wlan_reset);

	ret = wlan_sdio_allocate_memory();
	if (ret != 0)
		return ret;
	*(volatile unsigned char __force  *)(WLAN_OOB_IRQ_CR) = 0xA0;
	ret = gpio_to_irq(wlan_cfg->bcm_wlan_host_wake);
	if (ret < 0)
		goto exit;
	wlan_sdio_performance_device.resource->start = ret;
	wlan_sdio_performance_device.resource->end = ret;

	ret = platform_device_register(&wlan_sdio_performance_device);
	if (ret) {
		pr_err("%s: Failed to register device\n",
			__func__);
		goto exit;
	}
	return 0;
exit:
	wlan_sdio_deallocate_memory();
	return ret;
}
/* wlan_sdio_performance_remove
	remove wlan device
*/
static int wlan_sdio_performance_remove(struct platform_device *pdev)
{
	pr_debug("%s: Enter\n", __func__);
	if (!wlan_sdio_deallocate_memory())
		pr_err("%s: failed to deallocate memory\n",
			__func__);
	return 0;
}
/* platform driver
	platform driver struct for wlan device
*/
static struct platform_driver wlan_sdio_perf_driver = {
	.driver		= {
		.name	= "bcm-wlan",
		.owner	= THIS_MODULE,
#if DEV_TREE
		.of_match_table = of_match_ptr(wlan_sdio_driver),
#endif
	},
	.probe		= wlan_sdio_performance_probe,
	.remove		= wlan_sdio_performance_remove,
};
/* wlan_sdio_driver_init
	Initialization function for wlan perf driver
*/
static int __init wlan_sdio_driver_init(void)
{
	pr_debug("%s: Enter\n", __func__);
	return platform_driver_register(&wlan_sdio_perf_driver);
}
/* wlan_sdio_driver_exit
	Exit function for wlan perf driver
*/
static void __exit wlan_sdio_driver_exit(void)
{
		platform_driver_unregister(&wlan_sdio_perf_driver);
}
module_init(wlan_sdio_driver_init);
module_exit(wlan_sdio_driver_exit);

MODULE_DESCRIPTION("Wlan Driver for Broadcom Kona platform");
MODULE_AUTHOR("Broadcom");
MODULE_LICENSE("GPL v2");
