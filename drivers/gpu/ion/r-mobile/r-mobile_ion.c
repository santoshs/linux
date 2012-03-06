/*
 * drivers/gpu/r-mobile/r-mobile_ion.c
 *
 * Copyright (C) 2012 Renesas Electronics Corporation
 *
 * Based on drivers/gpu/tegra/tegra_ion.c,
 * Copyright (C) 2011 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/err.h>
#include <linux/ion.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <rtapi/system_memory.h>
#include "../ion_priv.h"

static struct ion_device *idev;
static int num_heaps;
static struct ion_heap **heaps;
static void *rtmem;
static unsigned int phys_addr;
static unsigned int map_size;
static unsigned int rtaddr;

struct ion_client *r_mobile_ion_client_create(unsigned int heap_mask,
					      const char *name)
{
	return ion_client_create(idev, heap_mask, name);
}
EXPORT_SYMBOL(r_mobile_ion_client_create);

static int r_mobile_ion_map_rtmem(unsigned int base, unsigned int size)
{
	int ret;
	system_mem_rt_map rt_map;
	system_mem_reg_phymem reg_phymem;

	/* map the video heap to RT-Domain */
	rt_map.handle = rtmem;
	rt_map.phys_addr = base;
	rt_map.map_size = size;
	ret = system_memory_rt_map(&rt_map);
	if (ret != SMAP_LIB_MEMORY_OK) {
		printk(KERN_ERR "%s: system_memory_rt_map error: %d\n",
		       __func__, ret);
		return -ENOMEM;
	}

	/* register the RT-mapped area */
	reg_phymem.handle = rtmem;
	reg_phymem.phys_addr = base;
	reg_phymem.map_size = size;
	reg_phymem.rtaddr = rt_map.rtaddr;
	ret = system_memory_reg_phymem(&reg_phymem);
	if (ret != SMAP_LIB_MEMORY_OK) {
		system_mem_rt_unmap rt_unmap;

		printk(KERN_ERR "%s: system_memory_reg_phymem error: %d\n",
		       __func__, ret);

		rt_unmap.handle = rtmem;
		rt_unmap.rtaddr = rt_map.rtaddr;
		rt_unmap.map_size = size;
		system_memory_rt_unmap(&rt_unmap);
		return -ENOMEM;
	}

	phys_addr = base;
	map_size = size;
	rtaddr = rt_map.rtaddr;

	return 0;
}

static int r_mobile_ion_probe(struct platform_device *pdev)
{
	struct ion_platform_data *pdata = pdev->dev.platform_data;
	int err;
	int i;
	system_mem_info_delete info_delete;

	num_heaps = pdata->nr;

	rtmem = system_memory_info_new();
	if (!rtmem) {
		printk(KERN_ERR "%s: system_memory_info_new error\n", __func__);
		return -ENOMEM;
	}

	heaps = kzalloc(sizeof(struct ion_heap *) * pdata->nr, GFP_KERNEL);
	if (!heaps) {
		err = -ENOMEM;
		goto err_heap;
	}

	idev = ion_device_create(NULL);
	if (IS_ERR_OR_NULL(idev)) {
		err = PTR_ERR(idev);
		goto err_dev;
	}

	/* create the heaps as specified in the board file */
	for (i = 0; i < num_heaps; i++) {
		struct ion_platform_heap *heap_data = &pdata->heaps[i];

		heaps[i] = ion_heap_create(heap_data);
		if (IS_ERR_OR_NULL(heaps[i])) {
			err = PTR_ERR(heaps[i]);
			goto err;
		}
		ion_device_add_heap(idev, heaps[i]);

		if (heap_data->id == ION_HEAP_VIDEO_ID) {
			err = r_mobile_ion_map_rtmem(heap_data->base,
						     heap_data->size);
			if (err)
				goto err;
		}
	}
	platform_set_drvdata(pdev, idev);
	return 0;
err:
	for (i = 0; i < num_heaps; i++) {
		if (heaps[i])
			ion_heap_destroy(heaps[i]);
	}
err_dev:
	kfree(heaps);
err_heap:
	info_delete.handle = rtmem;
	system_memory_info_delete(&info_delete);
	return err;
}

static int r_mobile_ion_remove(struct platform_device *pdev)
{
	struct ion_device *idev = platform_get_drvdata(pdev);
	int i;
	system_mem_unreg_phymem unreg_phymem;
	system_mem_rt_unmap rt_unmap;
	system_mem_info_delete info_delete;

	ion_device_destroy(idev);
	for (i = 0; i < num_heaps; i++)
		ion_heap_destroy(heaps[i]);
	kfree(heaps);

	/* unregister the RT-mapped area */
	unreg_phymem.handle = rtmem;
	unreg_phymem.phys_addr = phys_addr;
	unreg_phymem.map_size = map_size;
	unreg_phymem.rtaddr = rtaddr;
	system_memory_unreg_phymem(&unreg_phymem);

	/* unmap the video heap */
	rt_unmap.handle = rtmem;
	rt_unmap.rtaddr = rtaddr;
	rt_unmap.map_size = map_size;
	system_memory_rt_unmap(&rt_unmap);

	info_delete.handle = rtmem;
	system_memory_info_delete(&info_delete);
	return 0;
}

static struct platform_driver ion_driver = {
	.probe = r_mobile_ion_probe,
	.remove = r_mobile_ion_remove,
	.driver = { .name = "ion-r-mobile" }
};

static int __init ion_init(void)
{
	return platform_driver_register(&ion_driver);
}

static void __exit ion_exit(void)
{
	platform_driver_unregister(&ion_driver);
}

module_init(ion_init);
module_exit(ion_exit);

MODULE_AUTHOR("Renesas Electronics Corporation");
MODULE_LICENSE("GPL");
