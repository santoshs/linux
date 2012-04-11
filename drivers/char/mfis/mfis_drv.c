/*
 * This file is MFIS driver function.
 *
 * Copyright (C) 2011 Renesas Electronics Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/err.h>
#include <linux/pm_runtime.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <rtapi/system_standby.h>
#include "mfis_private.h"

#if RTPM_PF_CUSTOM
#include <mach/pm.h>
#endif

static struct clk *mfi_clk_data;

static int mfis_drv_probe(struct platform_device *pdev)
{

	int ret = -1;
	struct clk *clk_data;
#if RTPM_PF_CUSTOM
	char domain_name[] = "av-domain";
	char *dev_name;
	struct device *dev_img;
	size_t dev_cnt;
#endif

	clk_data = clk_get(NULL, "mp_clk");
	if (IS_ERR(clk_data)) {
		pr_err("cannot get clock \"%s\"\n", "mp_clk");
		return PTR_ERR(clk_data);
	}
	clk_enable(clk_data);
	clk_put(clk_data);
	
	
	pm_runtime_enable(&pdev->dev);
	
	mfi_clk_data = clk_get(NULL, "mfis");
	if (IS_ERR(mfi_clk_data)) {
		pr_err("cannot get clock \"%s\"\n", "mfis");
		return PTR_ERR(clk_data);
	}
	clk_enable(mfi_clk_data);
	
#if RTPM_PF_CUSTOM
	dev_name = domain_name;	
	ret = power_domain_devices(dev_name, &dev_img, &dev_cnt);
	if (!ret) {
		ret = pm_runtime_get_sync(dev_img);
		if (0 > ret) {
			return -1;
		}
	}
	else {
		return -1;
	}
	
#endif
	
	ret = pm_runtime_get_sync(&pdev->dev);
	if (0 > ret) {
		return -1;
	}
	
	return 0;
}

static int mfis_drv_remove(struct platform_device *pdev)
{

	struct clk *clk_data;
#if RTPM_PF_CUSTOM
	int ret = -1;
	char domain_name[] = "av-domain";
	char *dev_name;
	struct device *dev_img;
	size_t dev_cnt;
	
	dev_name = domain_name;	
	ret = power_domain_devices(dev_name, &dev_img, &dev_cnt);
	if (!ret) {
		ret = 	pm_runtime_put_sync(dev_img);
		if (!ret) {
		}
		else {
			return -1;
		}
	}
	
#endif
	
	pm_runtime_disable(&pdev->dev);
	
	clk_disable(mfi_clk_data);
	clk_put(mfi_clk_data);
	
	clk_data = clk_get(NULL, "mp_clk");
	if (IS_ERR(clk_data)) {
		pr_err("cannot get clock \"%s\"\n", "mp_clk");
		return PTR_ERR(clk_data);
	}
	clk_disable(clk_data);
	clk_put(clk_data);
	
	
#if RTPM_PF_CUSTOM
	return ret;
#else 
	return 0;
#endif

}

static int mfis_suspend_noirq(struct device *dev)
{

	int ret = -1;
	struct clk *clk_data;
#if RTPM_PF_CUSTOM
	char domain_name[] = "av-domain";
	char *dev_name;
	struct device *dev_img;
	size_t dev_cnt;
#endif

	ret = system_rt_standby();
	if (ret == SMAP_LIB_STANDBY_OK) {
	}
	else {
		return -1;
	}
	
#if RTPM_PF_CUSTOM		
	dev_name = domain_name;
	ret = power_domain_devices(dev_name, &dev_img, &dev_cnt);
	if (!ret) {
		ret = 	pm_runtime_put_sync(dev_img);
		if (!ret) {
		}
		else {
			return -1;
		}
	}
	else {
		return -1;
	}
#endif

	ret = pm_runtime_put_sync(dev);
	if (!ret) {
	}
	else {
		return -1;
	}

	clk_disable(mfi_clk_data);

	clk_data = clk_get(NULL, "mp_clk");
	if (IS_ERR(clk_data)) {
		pr_err("cannot get clock \"%s\"\n", "mp_clk");
		return PTR_ERR(clk_data);
	}
	clk_disable(clk_data);
	clk_put(clk_data);

	return 0;
}

static int mfis_resume_noirq(struct device *dev)
{

	int ret = -1;
	struct clk *clk_data;
#if RTPM_PF_CUSTOM

	char domain_name[] = "av-domain";
	char *dev_name;
	struct device *dev_img;
	size_t dev_cnt;
#endif

	clk_data = clk_get(NULL, "mp_clk");
	if (IS_ERR(clk_data)) {
		pr_err("cannot get clock \"%s\"\n", "mp_clk");
		return PTR_ERR(clk_data);
	}
	clk_enable(clk_data);
	clk_put(clk_data);
	
	
	ret = pm_runtime_get_sync(dev);
	if (0 > ret) {
		return -1;
	}
	clk_enable(mfi_clk_data);
	
#if RTPM_PF_CUSTOM
	dev_name = domain_name;	
	ret = power_domain_devices(dev_name, &dev_img, &dev_cnt);
	if (!ret) {
		ret = pm_runtime_get_sync(dev_img);
		if (0 > ret) {
			return -1;
		}
	}
	else {
		return -1;
	}
#endif
	
	ret = system_rt_active();
	if (ret == SMAP_LIB_STANDBY_OK) {
	}
	else {
		return -1;
	}

	return 0;
}


static int mfis_runtime_nop(struct device *dev)
{
	/* nop */
	return 0;
}


static const struct dev_pm_ops mfis_dev_pm_ops = {
	.suspend_noirq   = mfis_suspend_noirq,
	.resume_noirq    = mfis_resume_noirq,
	.runtime_suspend = mfis_runtime_nop,
	.runtime_resume  = mfis_runtime_nop,
};

static struct platform_driver mfis_driver = {
	.driver		= {
		.name		= "mfis",
		.owner		= THIS_MODULE,
		.pm		= &mfis_dev_pm_ops,
	},
	.probe		= mfis_drv_probe,
	.remove		= mfis_drv_remove,
};

static int __init mfis_drv_adap_init(void)
{
	return platform_driver_register(&mfis_driver);
}

static void __exit mfis_drv_adap_exit(void)
{
	platform_driver_unregister(&mfis_driver);
}

module_init(mfis_drv_adap_init);
module_exit(mfis_drv_adap_exit);

MODULE_DESCRIPTION("SuperH Mobile MFIS driver");
MODULE_AUTHOR("Renesas Electronics Corporation.");
MODULE_LICENSE("GPL v2");


