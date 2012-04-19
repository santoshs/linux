/*
 * This file is MFIS driver function.
 *
 * Copyright (C) 2011-2012 Renesas Electronics Corporation
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
#include <linux/semaphore.h>
#include <rtapi/system_standby.h>
#include "mfis_private.h"

#if RTPM_PF_CUSTOM
#include <mach/pm.h>
#endif

static struct clk *mfi_clk_data;
static struct clk *clk_data;
static struct semaphore a3r_power_sem;

#if EARLYSUSPEND_STANDBY
#include <linux/earlysuspend.h>
#include <mach/irqs.h>
#define	GIC_MFI			(126)
#define	IRQ_MFI			(gic_spi(GIC_MFI))
#define	REG_SYSC_PSTR		((unsigned long)0xE6180080)
#define	REG_CPGA_MSTPSR0	((unsigned long)0xE6150030)
#define	POWER_A3R		((unsigned long)0x00002000)
#define	CLOCK_TLB_IC_OC	((unsigned long)0xE0000000)

struct mfis_early_suspend_tbl {
	struct device *dev;
	struct early_suspend early_suspend;
};

static struct platform_device *pdev_tbl;
static unsigned long early_suspend_phase_flag;
static unsigned long late_resume_phase_flag;
static struct semaphore mfis_sem;

#define CYCLE_STANDBY 1
#else
#define CYCLE_STANDBY 0
#endif /* EARLYSUSPEND_STANDBY */

#if CYCLE_STANDBY
#include <linux/mfis.h>
#define STANDBY_RETRY_INTERVAL 100

struct workqueue_struct	*mfis_wq;
struct delayed_work		standby_work;
unsigned int			standby_work_on;
#endif /* CYCLE_STANDBY */


#if CYCLE_STANDBY
static void mfis_standby_work(struct work_struct *work)
{
	int ret;

	standby_work_on = 0;
	
	ret = mfis_drv_suspend();
	if (ret != 0)
	{
		if ((down_interruptible(&mfis_sem))) {
			printk(KERN_ALERT "[%s] Semaphore acquisition error!!\n",__func__);
			return;
		}
		
		standby_work_on = 1;
		queue_delayed_work(
			mfis_wq,
			&standby_work,
			msecs_to_jiffies(STANDBY_RETRY_INTERVAL));
		
		up(&mfis_sem);
	}
	
	return;
}
#endif /* CYCLE_STANDBY */

static int mfis_suspend_noirq(struct device *dev)
{

	int ret = -1;
#if RTPM_PF_CUSTOM
	char domain_name[] = "av-domain";
	char *dev_name;
	struct device *dev_img;
	size_t dev_cnt;
#endif

#if CYCLE_STANDBY
	if (standby_work_on)
	{
		if ((down_interruptible(&mfis_sem))) {
			printk(KERN_ALERT "[%s] Semaphore acquisition error!!\n",__func__);
			return -1;
		}
		
		cancel_delayed_work_sync(&standby_work);
		standby_work_on = 0;
		
		up(&mfis_sem);
	}
#endif /* CYCLE_STANDBY */

	down(&a3r_power_sem);

#if (EARLYSUSPEND_STANDBY == 1) && (RTPM_PF_CUSTOM == 1)
	if (POWER_A3R & inl(REG_SYSC_PSTR)) {
		dev_name = domain_name;
		ret = power_domain_devices(dev_name, &dev_img, &dev_cnt);
		if (((early_suspend_phase_flag)&&(1==atomic_read(&dev_img->power.usage_count))) || (!early_suspend_phase_flag)) {
#endif /* EARLYSUSPEND_STANDBY */
			ret = system_rt_standby();
			if (ret == SMAP_LIB_STANDBY_OK) {
			}
			else {
				up(&a3r_power_sem);
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
					up(&a3r_power_sem);
					return -1;
				}
			}
			else {
				up(&a3r_power_sem);
				return -1;
			}
#endif

			ret = pm_runtime_put_sync(dev);
			if (!ret) {
			}
			else {
				up(&a3r_power_sem);
				return -1;
			}
			clk_disable(mfi_clk_data);
			clk_disable(clk_data);
#if (EARLYSUSPEND_STANDBY == 1) && (RTPM_PF_CUSTOM == 1)
		}
	}
#endif /* EARLYSUSPEND_STANDBY */

	up(&a3r_power_sem);

	return 0;
}

static int mfis_resume_noirq(struct device *dev)
{

	int ret = -1;
#if RTPM_PF_CUSTOM

	char domain_name[] = "av-domain";
	char *dev_name;
	struct device *dev_img;
	size_t dev_cnt;
#endif

	down(&a3r_power_sem);

#if EARLYSUSPEND_STANDBY
	if ( CLOCK_TLB_IC_OC == ( inl(REG_CPGA_MSTPSR0) & CLOCK_TLB_IC_OC ) ) {
#endif /* EARLYSUSPEND_STANDBY */
		clk_enable(clk_data);

		ret = pm_runtime_get_sync(dev);
		if (0 > ret) {
			up(&a3r_power_sem);
			return -1;
		}
		clk_enable(mfi_clk_data);
	
#if RTPM_PF_CUSTOM
		dev_name = domain_name;	
		ret = power_domain_devices(dev_name, &dev_img, &dev_cnt);
		if (!ret) {
			ret = pm_runtime_get_sync(dev_img);
			if (0 > ret) {
				up(&a3r_power_sem);
				return -1;
			}
		}
		else {
			up(&a3r_power_sem);
			return -1;
		}
#endif

		ret = system_rt_active();
		if (ret == SMAP_LIB_STANDBY_OK) {
		}
		else {
			up(&a3r_power_sem);
			return -1;
		}
#if EARLYSUSPEND_STANDBY
	}
#endif /* EARLYSUSPEND_STANDBY */

	up(&a3r_power_sem);

	return 0;
}

#if EARLYSUSPEND_STANDBY
static void mfis_drv_early_suspend(struct early_suspend *h)
{
	struct mfis_early_suspend_tbl *p_tbl;
	int ret;

	p_tbl = container_of(h, struct mfis_early_suspend_tbl, early_suspend);

	early_suspend_phase_flag = 1;

	ret = mfis_suspend_noirq(p_tbl->dev);
#if CYCLE_STANDBY
	if (ret != 0)
	{
		standby_work_on = 1;
		
		queue_delayed_work(
			mfis_wq,
			&standby_work,
			msecs_to_jiffies(STANDBY_RETRY_INTERVAL));
	}
#endif /* CYCLE_STANDBY */

	early_suspend_phase_flag = 0;

	return;
}

static void mfis_drv_late_resume(struct early_suspend *h)
{
	struct mfis_early_suspend_tbl *p_tbl;

#if CYCLE_STANDBY
	if (standby_work_on)
	{
		if ((down_interruptible(&mfis_sem))) {
			printk(KERN_ALERT "[%s] Semaphore acquisition error!!\n",__func__);
			return;
		}
		
		cancel_delayed_work_sync(&standby_work);
		standby_work_on = 0;
		
		up(&mfis_sem);
	}
#endif /* CYCLE_STANDBY */

	p_tbl = container_of(h, struct mfis_early_suspend_tbl, early_suspend);

	late_resume_phase_flag = 1;

	mfis_resume_noirq(p_tbl->dev);

	late_resume_phase_flag = 0;

	return;
}
#endif /* EARLYSUSPEND_STANDBY */

static int mfis_drv_probe(struct platform_device *pdev)
{
	
	int ret = -1;
#if RTPM_PF_CUSTOM
	char domain_name[] = "av-domain";
	char *dev_name;
	struct device *dev_img;
	size_t dev_cnt;
#endif
#if EARLYSUSPEND_STANDBY
	struct mfis_early_suspend_tbl *p_tbl;
	early_suspend_phase_flag = 0;
	late_resume_phase_flag = 0;
#endif /* EARLYSUSPEND_STANDBY */
	
	clk_data = clk_get(NULL, "mp_clk");
	if (IS_ERR(clk_data)) {
		pr_err("cannot get clock \"%s\"\n", "mp_clk");
		return PTR_ERR(clk_data);
	}
	clk_enable(clk_data);
	
	pm_runtime_enable(&pdev->dev);

	mfi_clk_data = clk_get(NULL, "mfis");
	if (IS_ERR(mfi_clk_data)) {
		pr_err("cannot get clock \"%s\"\n", "mfis");
		return PTR_ERR(mfi_clk_data);
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
	
#if CYCLE_STANDBY
	mfis_wq = create_singlethread_workqueue("mfis_wq");
	
	INIT_DELAYED_WORK(&standby_work, mfis_standby_work);
	
	standby_work_on = 0;
#endif /* CYCLE_STANDBY */
	
#if EARLYSUSPEND_STANDBY
	p_tbl = kzalloc(sizeof(*p_tbl), GFP_KERNEL);
	if (p_tbl == NULL) {
		pr_err("cannot alloc area \n");
		return -1;
	}
	platform_set_drvdata(pdev, p_tbl);

	sema_init(&mfis_sem,1);
	sema_init(&a3r_power_sem,1);
	p_tbl->dev						= &pdev->dev;
	p_tbl->early_suspend.level		= (EARLY_SUSPEND_LEVEL_DISABLE_FB + 1);
	p_tbl->early_suspend.suspend	= mfis_drv_early_suspend;
	p_tbl->early_suspend.resume		= mfis_drv_late_resume;
	register_early_suspend(&p_tbl->early_suspend);
	pdev_tbl = pdev;
#endif /* EARLYSUSPEND_STANDBY */

	return 0;
}

static int mfis_drv_remove(struct platform_device *pdev)
{

#if EARLYSUSPEND_STANDBY
	struct mfis_early_suspend_tbl *p_tbl;
#endif /* EARLYSUSPEND_STANDBY */
#if RTPM_PF_CUSTOM
	int ret = -1;
	char domain_name[] = "av-domain";
	char *dev_name;
	struct device *dev_img;
	size_t dev_cnt;
#endif

#if EARLYSUSPEND_STANDBY
	p_tbl = platform_get_drvdata(pdev);
	unregister_early_suspend(&p_tbl->early_suspend);
	kfree(p_tbl);
#endif /* EARLYSUSPEND_STANDBY */

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
	
#endif

	pm_runtime_disable(&pdev->dev);

	clk_disable(mfi_clk_data);
	clk_put(mfi_clk_data);

	clk_disable(clk_data);
	clk_put(clk_data);
	
#if CYCLE_STANDBY
	destroy_workqueue(mfis_wq);
#endif /* CYCLE_STANDBY */
	
#if RTPM_PF_CUSTOM
	return ret;
#else 
	return 0;
#endif

}

#if EARLYSUSPEND_STANDBY
int mfis_drv_suspend(void)
{
	struct mfis_early_suspend_tbl *p_tbl;
	int ret = 0;

	if ((down_interruptible(&mfis_sem))) {
		printk(KERN_ALERT "[%s] Semaphore acquisition error!!\n",__func__);
		return -1;
	}

	if (POWER_A3R & inl(REG_SYSC_PSTR)) {
		p_tbl = platform_get_drvdata(pdev_tbl);

		early_suspend_phase_flag = 1;

		ret = mfis_suspend_noirq(p_tbl->dev);

		early_suspend_phase_flag = 0;
	}

	up(&mfis_sem);
	
	return ret;
}
EXPORT_SYMBOL(mfis_drv_suspend);

int mfis_drv_resume(void)
{
	struct mfis_early_suspend_tbl *p_tbl;
	int ret = 0;

	if ((down_interruptible(&mfis_sem))) {
		printk(KERN_ALERT "[%s] Semaphore acquisition error!!\n",__func__);
		return -1;
	}

	if (!(POWER_A3R & inl(REG_SYSC_PSTR))) {
		p_tbl = platform_get_drvdata(pdev_tbl);

		late_resume_phase_flag = 1;

		ret = mfis_resume_noirq(p_tbl->dev);

		late_resume_phase_flag = 0;
	}

	up(&mfis_sem);
	
	return ret;
}
EXPORT_SYMBOL(mfis_drv_resume);

#else /* EARLYSUSPEND_STANDBY */
int mfis_drv_suspend(void)
{
	return 0;
}
EXPORT_SYMBOL(mfis_drv_suspend);

int mfis_drv_resume(void)
{
	return 0;
}
EXPORT_SYMBOL(mfis_drv_resume);
#endif /* EARLYSUSPEND_STANDBY */

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


