/*
 * /drivers/sensor/bh1771glc/bh1771glc.c
 *
 * Copyright (C) 2011-2012 Renesas Mobile Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/wakelock.h>
#include <asm/io.h>
#include <linux/gpio.h>
/* #include <linux/pmic/pmic.h> */
#include <linux/bh1771glc.h>
#include <mach/irqs.h>
#include <mach/r8a73734.h>
#include "bh1771glc_local.h"

#define ROUND_FACTOR 100
/* prototype */
/* Common APIs: */
static int __init bh1771_init(void);
static void __exit bh1771_exit(void);
static int bh1771_i2c_suspend(struct device *dev);
static int bh1771_i2c_resume(struct device *dev);
static int bh1771_i2c_probe(struct i2c_client *client,  const struct i2c_device_id *devid);
static int bh1771_i2c_remove(struct i2c_client *client);
static int bh1771_i2c_read(unsigned char reg, unsigned char *val, int len);
static int bh1771_i2c_write(unsigned char reg, unsigned char *val);
static int bh1771_misc_open(struct inode *ip, struct file *fp);
static int bh1771_misc_release(struct inode *ip, struct file *fp);
static long bh1771_misc_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
static void bh1771_get_nv(unsigned long addr);
static int bh1771_hw_init(int *val);

/* Light APIs: */
static enum hrtimer_restart bh1771_als_timer_func(struct hrtimer *timer);
static void bh1771_als_work_func(struct work_struct *work);
static int bh1771_als_reset(void);
static int bh1771_als_resolution_mode(unsigned char val);
static int bh1771_als_sensitivity(unsigned char val);
static int bh1771_als_output_data_rate(unsigned char val);
static int bh1771_als_power_status(unsigned char val);

/* Proximity APIs: */
static enum hrtimer_restart bh1771_ps_timer_func(struct hrtimer *timer);
#ifdef PROXIMITY_INT
static irqreturn_t bh1771_ps_irq_handler(int irq, void *dev_id);
static int bh1771_ps_int_mode(unsigned char val);
static int bh1771_ps_int_th_h(unsigned char led_id, unsigned char val);
static int bh1771_ps_int_th_l(unsigned char led_id, unsigned char val);
static int bh1771_ps_int_hysteresis(unsigned char val);
static int bh1771_ps_persistence(unsigned char val);
#endif
static void bh1771_ps_poll_work_func(struct work_struct *work);
static void bh1771_ps_irq_work_func(struct work_struct *work);
static int bh1771_ps_power_status(unsigned char val);
static int bh1771_ps_led_current(unsigned char led_mode, const int *val);
static int bh1771_ps_measure_rate(unsigned char val);

/*Global variable*/
static struct bh1771_data	*bh1771_ginfo;
static struct i2c_client 	*g_client;
static struct workqueue_struct *workqueue;

static const struct i2c_device_id bh1771_id[] = {
	{BH1771_I2C_NAME, 0 },
	{}
};

static struct dev_pm_ops bh1771_pm_ops = {
	.suspend = bh1771_i2c_suspend,
	.resume = bh1771_i2c_resume,
};

static struct i2c_driver bh1771_i2c_driver = {
	.driver = {
		.name = BH1771_I2C_NAME,
		.owner = THIS_MODULE,
		.pm = &bh1771_pm_ops,
	},
	.probe = bh1771_i2c_probe,	
	.remove = bh1771_i2c_remove,
	.id_table = bh1771_id,
};

static struct file_operations bh1772_fops = {
	.owner = THIS_MODULE,
	.llseek = no_llseek,
	.open = bh1771_misc_open,
	.release = bh1771_misc_release,
	.unlocked_ioctl = bh1771_misc_ioctl,
};

static struct miscdevice bh1772_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = BH1771_I2C_NAME,
	.fops = &bh1772_fops,
};

static const int bh1771_setting_default[BH1771_SETTING_MAX] = {
	53,
	0,
	0,
	94,
	94,
	94,
	0,
	0,
	0,
	0,
	4,
	0,
	LEDCURRENT_50,
	LEDCURRENT_50,
	LEDCURRENT_50
};

static int bh1771_setting_nv[BH1771_SETTING_MAX];

const struct bh1771glc_output_rate odr_table_als[] = {
	{	100,		ALSRATE_100			},
	{	200,		ALSRATE_200			},
	{	500,		ALSRATE_500			},
	{	1000,		ALSRATE_1000		},
	{	2000,		ALSRATE_2000		},
};

const struct bh1771glc_output_rate odr_table_ps[] = {
	{	10,		PSRATE_10			},
	{	20,		PSRATE_20			},
	{	30,		PSRATE_30			},
	{	50,		PSRATE_50			},
	{	70,		PSRATE_70			},
	{	100,	PSRATE_100			},
	{	200,	PSRATE_200			},
	{	500,	PSRATE_500			},
	{	1000,	PSRATE_1000			},
	{	2000,	PSRATE_2000			},
};

/*************************************************************************
 *	name	=	bh1771_get_nv
 *	func	=	Get user setting values from non-volatile memory address
 *	input	=	unsigned long addr
 *	output	=	None
 *	return	=	None
 *************************************************************************/
static void bh1771_get_nv(unsigned long addr) 
{
	void *ptr;
	
	bh1771glc_log("NV address = 0x%lX \n", addr);

	ptr = ioremap(addr, sizeof(bh1771_setting_nv));
	memcpy(&bh1771_setting_nv[0], ptr, sizeof(bh1771_setting_nv));
	iounmap(ptr);
}

/*************************************************************************
 *	name	=	bh1771_i2c_read
 *	func	=	Read data from specified register address of BH1771GLC chip
 *	input	=	unsigned char reg, unsigned char *val, int len
 *	output	=	None
 *	return	=	0,	-EIO,	-EINVAL,	-ENODEV
 *************************************************************************/
static int bh1771_i2c_read(unsigned char reg, unsigned char *val, int len)
{
	int ret;
	struct i2c_msg msg[2];

	if (NULL == val) {
		bh1771glc_log("i2c null pointer error\n");
		return -EINVAL;
	}
	
	if (NULL == g_client->adapter) {
		bh1771glc_log("i2c null pointer error\n");
		return -ENODEV;
	}
	
	/* Initialization for all elements of i2c_msg structure */
	memset(&msg, 0, sizeof(struct i2c_msg));

	/* write start address */
	msg[0].addr  = g_client->addr;
    msg[0].flags = 0;
    msg[0].len   = 1;
    msg[0].buf   = (unsigned char *)&reg;
    
    /* read data */
    msg[1].addr  = g_client->addr;
    msg[1].flags = I2C_M_RD;
    msg[1].len   = len;
    msg[1].buf   = val;
	
	/* Set retry times. */
	g_client->adapter->retries = I2C_RETRIES;

	ret = i2c_transfer(g_client->adapter, msg, 2);
	if (ret != 2) {
		bh1771glc_log("i2c transfer error\n");
		return -EIO;
	}
	return 0;
}

/*************************************************************************
 *	name	=	bh1771_i2c_write
 *	func	=	Write (1 byte) data to specified register address of BH1771GLC chip
 *	input	=	unsigned char reg, unsigned char *val
 *	output	=	None
 *	return	=	0,	-EIO,	-EINVAL,	-ENODEV
 *************************************************************************/
static int bh1771_i2c_write(unsigned char reg, unsigned char *val)
{
	int ret;
	struct i2c_msg msg;
	unsigned char buf[2];

	if (NULL == val) {
		bh1771glc_log("i2c null pointer error\n");
		return -EINVAL;
	}

	if (NULL == g_client->adapter) {
		bh1771glc_log("i2c null pointer error\n");
		return -ENODEV;
	}

	/* Initialization for all elements of i2c_msg structure */
	memset(&msg, 0, sizeof(struct i2c_msg));
	
	buf[0] = reg;
	buf[1] = *val; 

	msg.addr = g_client->addr;
	msg.flags = I2C_M_WR;
	msg.len = 2;
	msg.buf = buf;

	/* Set retry times. */
	g_client->adapter->retries = I2C_RETRIES;

	ret = i2c_transfer(g_client->adapter, &msg, 1);
	if (ret != 1) {
		bh1771glc_log("i2c transfer error\n");
		return -EIO;
	}
	return 0;
}

/*************************************************************************
 *	name	=	bh1771_als_timer_func
 *	func	=	Handle timer polling interval for ALS device driver
 *	input	=	struct hrtimer *timer
 *	output	=	None
 *	return	=	HRTIMER_NORESTART
 *************************************************************************/
static enum hrtimer_restart bh1771_als_timer_func(struct hrtimer *timer) 
{
	if (atomic_read(&bh1771_ginfo->als_enable)) {
		queue_work(workqueue, &bh1771_ginfo->als_work);
	}
	return HRTIMER_NORESTART;
}

/*************************************************************************
 *	name	=	bh1771_ps_timer_func
 *	func	=	Handle timer polling interval for PS device driver
 *	input	=	struct hrtimer *timer
 *	output	=	None
 *	return	=	HRTIMER_NORESTART
 *************************************************************************/
static enum hrtimer_restart bh1771_ps_timer_func(struct hrtimer *timer) 
{
	if (atomic_read(&bh1771_ginfo->ps_enable)) {
		queue_work(workqueue, &bh1771_ginfo->ps_work_poll);
	}
	return HRTIMER_NORESTART;
}

/*************************************************************************
 *	name	=	bh1771_ps_poll_work_func
 *	func	=	Work queue callback for proximity sensor
 *	input	=	struct work_struct *work
 *	output	=	None
 *	return	=	None
 *************************************************************************/
static void bh1771_ps_poll_work_func(struct work_struct *work)
{
#ifndef PROXIMITY_INT
	int ret = 0;
	unsigned char  ps_data = 0;
#endif 
	mutex_lock(&bh1771_ginfo->lock);
	wake_lock(&bh1771_ginfo->wakelock);
	
	if (bh1771_ginfo->ps_delay > MAX_DELAY_TIME) {
		bh1771_ps_power_status(CTL_STANDALONE);
		msleep(125);
	}
	/* for polling machine only */
#ifndef PROXIMITY_INT		
	/* Read data from REG_PSDATA register and report it if reading is not fail*/
	ret = bh1771_i2c_read(REG_PSDATA, &ps_data, 1);
	if (ret >= 0) {
		input_report_abs(bh1771_ginfo->input_dev, EVENT_TYPE_PROXIMITY, (int)ps_data);
		bh1771glc_log("PS data: %d\n", ps_data); 
		bh1771_ginfo->input_dev->sync = 0;
		input_event(bh1771_ginfo->input_dev, EV_SYN, SYN_REPORT, 1);
	}

	if (bh1771_ginfo->ps_delay > MAX_DELAY_TIME) {
		bh1771_ps_power_status(CTL_STANDBY);
	}
	/* start high resolution timer */
	hrtimer_start(&bh1771_ginfo->ps_timer, ktime_set(0, bh1771_ginfo->ps_poll_interval * NSEC_PER_MSEC), HRTIMER_MODE_REL);
#endif 

	wake_unlock(&bh1771_ginfo->wakelock);
	mutex_unlock(&bh1771_ginfo->lock);
	return;
}

/*************************************************************************
 *	name	=	bh1771_ps_irq_work_func
 *	func	=	Work queue callback for proximity sensor
 *	input	=	struct work_struct *work
 *	output	=	None
 *	return	=	None
 *************************************************************************/
static void bh1771_ps_irq_work_func(struct work_struct *work)
{
	int           ret;
    unsigned char  ps_data;
	
	mutex_lock(&bh1771_ginfo->lock);
	wake_lock(&bh1771_ginfo->wakelock);
	
	ps_data  = 0;
	/* Read data from REG_PSDATA register and report it if reading is not fail*/
	ret = bh1771_i2c_read(REG_PSDATA, &ps_data, 1);
	if (ret >= 0) {
		input_report_abs(bh1771_ginfo->input_dev, EVENT_TYPE_PROXIMITY, (int)ps_data);		
		bh1771_ginfo->input_dev->sync = 0;
		input_event(bh1771_ginfo->input_dev, EV_SYN, SYN_REPORT, 1);
	}

	if (bh1771_ginfo->ps_delay > MAX_DELAY_TIME) {
		bh1771_ps_power_status(CTL_STANDBY);
		/* start high resolution timer */
		hrtimer_start(&bh1771_ginfo->ps_timer, ktime_set(0, bh1771_ginfo->ps_poll_interval * NSEC_PER_MSEC), HRTIMER_MODE_REL);
	}
	enable_irq(g_client->irq);

	wake_unlock(&bh1771_ginfo->wakelock);
	mutex_unlock(&bh1771_ginfo->lock);
	return;
}

/*************************************************************************
 *	name	=	bh1771_als_work_func
 *	func	=	Read ALS data and report data to HAL periodically
 *	input	=	struct work_struct *work
 *	output	=	None
 *	return	=	None
 *************************************************************************/
static void bh1771_als_work_func(struct work_struct *work)
{
	int     		result = 0;
    u8 				als_data[2]; /* data of ALS from sensor */
	u16				als_value;
	
	bh1771glc_log("Start \n");
	mutex_lock(&bh1771_ginfo->lock);
	wake_lock(&bh1771_ginfo->wakelock);
	
    if (bh1771_ginfo->als_delay > MAX_DELAY_TIME)
	{
		/* Power on Light */
		result = bh1771_als_power_status(CTL_STANDALONE);
		if (result < 0) 
		{
			bh1771glc_log("Light power-on failed\n");
		}
		msleep(110);
	}
	/* Read & report value to HAL */
    result = bh1771_i2c_read(REG_ALSDATA, als_data, 2);
			 
    if (result < 0)
	{
       bh1771glc_log("Read Light data error\n");       
    } 
	else 
	{
		als_value = (als_data[1] << 8) | als_data[0];		
		als_value = (u16) (((bh1771_ginfo->als_ill_per_count * als_value) + (50 * ROUND_FACTOR)) / (100 * ROUND_FACTOR));
		bh1771glc_log("ALS data: %d\n", als_value);
		input_report_abs(bh1771_ginfo->input_dev, EVENT_TYPE_LIGHT, (int)als_value);		
		bh1771_ginfo->input_dev->sync = 0;
		input_event(bh1771_ginfo->input_dev, EV_SYN, SYN_REPORT, 1);
    }
	if (bh1771_ginfo->als_delay > MAX_DELAY_TIME) {
		/* Power down Light */
		result = bh1771_als_power_status(CTL_STANDBY);
		if (result < 0) {
			bh1771glc_log("Light power-down failed\n");
		}
	}
	hrtimer_start(&bh1771_ginfo->als_timer, ktime_set(0, bh1771_ginfo->als_poll_interval * NSEC_PER_MSEC), HRTIMER_MODE_REL);
    
	wake_unlock(&bh1771_ginfo->wakelock);
	mutex_unlock(&bh1771_ginfo->lock);
	
	bh1771glc_log("End \n");
	return;
}


/*************************************************************************
 *	name	=	bh1771_i2c_probe
 *	func	=	Probe I2C slave device for PS and ALS
 *	input	=	struct i2c_client *client, const struct i2c_device_id *id
 *	output	=	None
 *	return	=	0,	-ENOMEM,	-EIO
 *************************************************************************/
static int bh1771_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret = 0;	
#ifdef PROXIMITY_INT
	int irq = 0;
#endif
	bh1771glc_log("Start \n");
	/* check i2c functionality */
	ret = i2c_check_functionality(client->adapter, I2C_FUNC_I2C);
	if (0 == ret) {
		bh1771glc_log("i2c_check_functionality error\n");
		return -EIO;
	}	
	/* allocate driver_data */
	bh1771_ginfo = kzalloc(sizeof(struct bh1771_data), GFP_KERNEL);
	if (NULL == bh1771_ginfo) {
		bh1771glc_log("kzalloc error\n");
		return -ENOMEM;
	}
	
	/* set client data*/
	g_client = client;
	
	/* init mutex */
	mutex_init(&bh1771_ginfo->lock);
	/* init wakelock(prevent suspend) */
	wake_lock_init(&bh1771_ginfo->wakelock,
		WAKE_LOCK_SUSPEND, "bh1771-wakelock");
		
	/* work queue settings */
	workqueue = create_singlethread_workqueue("workqueue");
	if (NULL == workqueue) {
		bh1771glc_log("create_singlethread_workqueue error\n");
		ret = -ENOMEM;
		goto error_create_workqueue;
	}
	/* Initialize work queue */
	INIT_WORK(&bh1771_ginfo->als_work, bh1771_als_work_func);
	INIT_WORK(&bh1771_ginfo->ps_work_poll, bh1771_ps_poll_work_func);
	INIT_WORK(&bh1771_ginfo->ps_work_irq, bh1771_ps_irq_work_func);
	
	mutex_lock(&bh1771_ginfo->lock);

	/* Initialize high resolution timer for polling of Light */
	hrtimer_init(&bh1771_ginfo->als_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	bh1771_ginfo->als_timer.function = bh1771_als_timer_func;

	/* Initialize high resolution timer for Proximity */
	hrtimer_init(&bh1771_ginfo->ps_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	bh1771_ginfo->ps_timer.function = bh1771_ps_timer_func;
#ifdef PROXIMITY_INT	
	/* INT settings */
	irq = g_client->irq;
	bh1771_ginfo->irq = -1;
	ret = request_irq(irq, bh1771_ps_irq_handler, 0, "bh1771_int", bh1771_ginfo);
	if (ret < 0) {
		bh1771glc_log("INT Settings failed\n");
		ret = -EIO;
		goto error_request_irq;
	}
	bh1771_ginfo->irq = irq;
	/* Disable IRQ */
	disable_irq_nosync(bh1771_ginfo->irq);
#endif
	/* Allocate input device */
	bh1771_ginfo->input_dev = input_allocate_device();
	if (NULL == bh1771_ginfo->input_dev) {
		bh1771glc_log("failed to allocate input device\n");
		ret = -ENOMEM;
		goto error_input_allocate_device;
	}
	/* Setting input device */
	bh1771_ginfo->input_dev->name = "bh1771_inputdev";
	set_bit(EV_ABS, bh1771_ginfo->input_dev->evbit);
	input_set_abs_params(bh1771_ginfo->input_dev, EVENT_TYPE_LIGHT, 0, ABS_MAX_VAL_ALS, 0, 0);
	input_set_abs_params(bh1771_ginfo->input_dev, EVENT_TYPE_PROXIMITY, 0, ABS_MAX_VAL_PS, 0, 0);
	
	/* Register the device */
	ret = input_register_device(bh1771_ginfo->input_dev);
	if (ret < 0) {
		bh1771glc_log("unable to register %s input device\n", bh1771_ginfo->input_dev->name);
		ret = -EIO;
		goto error_input_register_device;
	}

	/* misc device settings */
	ret = misc_register(&bh1772_device);
	if (ret < 0) {
		bh1771glc_log("misc_register failed\n");
		ret = -EIO;
		goto error_misc_register;
	}
	
	/* reset for sensor */
	ret = bh1771_als_reset();
	if (ret < 0) {
		bh1771glc_log("SW reset failed\n");
		ret = -EIO;
		goto error_hardware;
    }
	
	/* Initialize setting for registers with default values */
	ret = bh1771_hw_init((int *)&bh1771_setting_default[0]);
	if (ret < 0) {
		bh1771glc_log("hw_init failed\n");
		ret = -EIO;
		goto error_hardware;
	}

	/* maintain power-down mode before using sensor */
		/* Already power-down by SW reset */
		
	/* Initial settings for global variables of driver */
	atomic_set(&bh1771_ginfo->als_power_flg, 0);
	atomic_set(&bh1771_ginfo->als_enable, 0);
	atomic_set(&bh1771_ginfo->ps_power_flg, 0);
	atomic_set(&bh1771_ginfo->ps_enable, 0);	
	bh1771_ginfo->als_delay = 500; //500ms
	bh1771_ginfo->als_poll_interval = 500; //500ms
	bh1771_ginfo->ps_delay = 100; //100ms
	bh1771_ginfo->ps_poll_interval = 100; //100ms		
	
	mutex_unlock(&bh1771_ginfo->lock);
	bh1771glc_log("End \n");
	return 0;

error_hardware:
	misc_deregister(&bh1772_device);
	
error_misc_register:
	/* Unregister input device from input core */
	input_unregister_device(bh1771_ginfo->input_dev);
	
error_input_register_device:
	/* Release input device structure */
	input_free_device(bh1771_ginfo->input_dev);
	
error_input_allocate_device:
#ifdef PROXIMITY_INT
	free_irq(g_client->irq, bh1771_ginfo);
error_request_irq:	
#endif	
	mutex_unlock(&bh1771_ginfo->lock);
	destroy_workqueue(workqueue);
	
error_create_workqueue:
	wake_lock_destroy(&bh1771_ginfo->wakelock);
	kfree(bh1771_ginfo);
	bh1771_ginfo = NULL;
	
	return ret;
}

/*************************************************************************
 *	name	=	bh1771_i2c_remove
 *	func	=	Remove I2C slave device for PS and ALS
 *	input	=	struct i2c_client *client
 *	output	=	None
 *	return	=	0
 *************************************************************************/
static int bh1771_i2c_remove(struct i2c_client *client)
{	
	#ifdef PROXIMITY_INT
		free_irq(bh1771_ginfo->irq, bh1771_ginfo);
	#endif
		/* Destroy work queue structure */
		destroy_workqueue(workqueue);
		/* Unregister input device from input core */
		input_unregister_device(bh1771_ginfo->input_dev);
		/* Release input device structure */
		input_free_device(bh1771_ginfo->input_dev);
		/* Destroy wakelock */
		wake_lock_destroy(&bh1771_ginfo->wakelock);
		/* Unregister misc driver */
		misc_deregister(&bh1772_device);    
		/* Release the memory that storing driver data */
		kfree(bh1771_ginfo);
		bh1771_ginfo = NULL;
	
    return 0;
}



/*************************************************************************
 *	name	=	bh1771_i2c_suspend
 *	func	=	Disable polling mechanism and power down PS and ALS
 *	input	=	struct i2c_client *client, pm_message_t msg
 *	output	=	None
 *	return	=	0
 *************************************************************************/
static int bh1771_i2c_suspend(struct device *dev)
{		
	u8 als_enable = 0;
	u8 ps_enable = 0;
	
	bh1771glc_log("Start \n");
	mutex_lock(&bh1771_ginfo->lock);
	
	als_enable = atomic_read(&bh1771_ginfo->als_enable);
	ps_enable = atomic_read(&bh1771_ginfo->ps_enable);
	if ((0 == als_enable) && (0 == ps_enable)) 
	{
		mutex_unlock(&bh1771_ginfo->lock);
		return 0;
	}
	/*If Light already enabled */
	if (als_enable) {
		/* Cancel high resolution timer for polling of Light */
		hrtimer_cancel(&bh1771_ginfo->als_timer);
		/* Power down Light */
		bh1771_als_power_status(CTL_STANDBY);
	}
	/*If Proximity already enabled */
	if (ps_enable) {
#ifdef PROXIMITY_INT
			/* Disable IRQ */
		disable_irq_nosync(bh1771_ginfo->irq);		
		if (bh1771_ginfo->ps_delay <= MAX_DELAY_TIME) {
			/* Power down Proximity */
			bh1771_ps_power_status(CTL_STANDBY);
		} else {
			/* Cancel high resolution timer for standby of Proximity */
			hrtimer_cancel(&bh1771_ginfo->ps_timer);
		}
#else
		/* Cancel high resolution timer for polling of Proximity */
		hrtimer_cancel(&bh1771_ginfo->ps_timer);
		/* Power down Proximity */
		bh1771_ps_power_status(CTL_STANDBY);		
#endif				
	}	
	mutex_unlock(&bh1771_ginfo->lock);

	bh1771glc_log("End \n");
	return 0;
}

/*************************************************************************
 *	name	=	bh1771_i2c_resume
 *	func	=	Enable polling mechanism and power up PS and ALS for operation
 *	input	=	struct i2c_client *client
 *	output	=	None
 *	return	=	0
 *************************************************************************/
static int bh1771_i2c_resume(struct device *dev)
{
	int ret = 0;	
	u8 als_enable = 0;
	u8 ps_enable = 0;
	
	bh1771glc_log("Start \n");	
	mutex_lock(&bh1771_ginfo->lock);
	
	als_enable = atomic_read(&bh1771_ginfo->als_enable);
	ps_enable = atomic_read(&bh1771_ginfo->ps_enable);
	
	if ((0 == als_enable) && (0 == ps_enable))
	{
		mutex_unlock(&bh1771_ginfo->lock);
		return 0;
	}
	/*If Light is already enabled */
	if (als_enable) {
	/* Restart high resolution timer for polling of Light */
		hrtimer_restart(&bh1771_ginfo->als_timer);
		if (bh1771_ginfo->als_delay <= MAX_DELAY_TIME) {
			/* Power on Light */
			ret = bh1771_als_power_status(CTL_STANDALONE);
			if (ret < 0) {
				bh1771glc_log("Light resumes failed\n");
			}
		}
	}
	/*If Proximity is already enabled */
	if (ps_enable) {
#ifdef PROXIMITY_INT
		/* Enable IRQ */
		enable_irq(bh1771_ginfo->irq);
		if (bh1771_ginfo->ps_delay <= MAX_DELAY_TIME) {
			/* Power on Proximity */
			ret = bh1771_ps_power_status(CTL_STANDALONE);
			if (ret < 0) {
				bh1771glc_log("Proximity resumes failed\n");
			}
		} else {
			/* Restart high resolution timer for standby of Proximity */
			hrtimer_restart(&bh1771_ginfo->ps_timer);		
		}
#else
		/* Restart high resolution timer for polling of Proximity */
		hrtimer_restart(&bh1771_ginfo->ps_timer);
		if (bh1771_ginfo->ps_delay <= MAX_DELAY_TIME) {
			/* Power on Proximity */
			ret = bh1771_ps_power_status(CTL_STANDALONE);
			if (ret < 0) {
				bh1771glc_log("Proximity resumes failed\n");
			}
		}
#endif		
	}

	mutex_unlock(&bh1771_ginfo->lock);

	bh1771glc_log("End \n");
	return 0;
}

/*************************************************************************
 *	name	=	bh1771_ps_power_status
 *	func	=	Set operation mode (power status) for PS device driver
 *	input	=	unsigned char val
 *	output	=	None
 *	return	=	0,	-EIO,	-EINVAL
 *************************************************************************/
static int bh1771_ps_power_status(unsigned char val)
{
	int ret = 0;
    unsigned char ps_state = 0;
	
	if ((val != CTL_STANDBY) && (val != CTL_STANDALONE)) {
		return -EINVAL; 
	}
    /* write value to REG_PSCONTROL register via i2c */
	if (val == 0) {
		ps_state = 0x00;
		ret = bh1771_i2c_write(REG_PSCONTROL, &ps_state);
    } else {
		ps_state = 0x03;
		ret = bh1771_i2c_write(REG_PSCONTROL, &ps_state);
	}
	if (ret < 0) {
		bh1771glc_log("Set power status failed\n");
		return -EIO;
    }
	if (CTL_STANDBY == val) {
		atomic_set(&bh1771_ginfo->ps_power_flg, 0);
	} else {
		atomic_set(&bh1771_ginfo->ps_power_flg, 1);
	}
    return ret;
}

/*************************************************************************
 *	name	=	bh1771_als_power_status
 *	func	=	Set operation mode (power status) for ALS device driver
 *	input	=	unsigned char val
 *	output	=	None
 *	return	=	0,	-EIO,	-EINVAL
 *************************************************************************/
static int bh1771_als_power_status(unsigned char val)
{
	int result = 0;
    unsigned char value = 0;
	unsigned char current_val = 0;
	
	bh1771glc_log("Start \n");
	if ((val != CTL_STANDBY) && (val != CTL_STANDALONE)) {
		bh1771glc_log("Input invalid\n");
		return -EINVAL; 
	}
	/* Read value from ALS_CONTROL register */
	result = bh1771_i2c_read(REG_ALSCONTROL, &value, 1);
	if (result < 0) {
		bh1771glc_log("Read error\n");
		return -EIO;
    }
	current_val = value & 0x03;
	if (current_val == val) {
		return 0;
	}
	/* Update power status setting */
	value = (value & CLR_LOW2BIT) | val;
    /* Write register to REG_ALSCONTROL register via i2c */
	result = bh1771_i2c_write(REG_ALSCONTROL, &value);
    if (result < 0) {
		bh1771glc_log("Set power status failed\n");
		return -EIO;
    }
	else
	{
		if (CTL_STANDBY == val)
		{
			atomic_set(&bh1771_ginfo->als_power_flg, 0);
		}
		else if (CTL_STANDALONE == val)
		{
			atomic_set(&bh1771_ginfo->als_power_flg, 1);
		}
	}
	
	bh1771glc_log("End \n");
    return 0;
}

/*************************************************************************
 *	name	=	bh1771_hw_init
 *	func	=	Initialize hardware setting for bh1771glc chip set
 *	input	=	unsigned char *val
 *	output	=	None
 *	return	=	0,	-EIO,	-EINVAL
 *************************************************************************/
static int bh1771_hw_init(int *val)
{
	int ret = 0;
	int ps_pw_flg = 0;
	int als_pw_flg = 0;
	
	if (NULL == val) {
		return -EINVAL;
	}
	ps_pw_flg = atomic_read(&bh1771_ginfo->ps_power_flg);
	als_pw_flg = atomic_read(&bh1771_ginfo->als_power_flg);
	
	bh1771_als_power_status(CTL_STANDBY);
	bh1771_ps_power_status(CTL_STANDBY);
	ret = bh1771_als_sensitivity(val[0]);
	if (ret < 0) {
		bh1771glc_log("bh1771_hw_init : error value = 0x%x \n", ret);
		return ret;
	}
	
	ret = bh1771_als_resolution_mode(val[1]);
	if (ret < 0) {
		bh1771glc_log("bh1771_hw_init : error value = 0x%x \n", ret);
		return ret;
	}
	
#ifdef PROXIMITY_INT
	ret = bh1771_ps_int_mode(val[2]);
	if (ret < 0) {
		bh1771glc_log("bh1771_hw_init : error value = 0x%x \n", ret);
		return ret;
	}
	
	ret = bh1771_ps_int_th_h(0, val[3]);
	if (ret < 0) {
		bh1771glc_log("bh1771_hw_init : error value = 0x%x \n", ret);
		return ret;
	}
	
	ret = bh1771_ps_int_th_h(1, val[4]);
	if (ret < 0) {
		bh1771glc_log("bh1771_hw_init : error value = 0x%x \n", ret);
		return ret;
	}
	
	ret = bh1771_ps_int_th_h(2, val[5]);
	if (ret < 0) {
		bh1771glc_log("bh1771_hw_init : error value = 0x%x \n", ret);
		return ret;
	}
	
	ret = bh1771_ps_int_th_l(0, val[6]);
	if (ret < 0) {
		bh1771glc_log("bh1771_hw_init : error value = 0x%x \n", ret);
		return ret;
	}
	
	ret = bh1771_ps_int_th_l(1, val[7]);
	if (ret < 0) {
		bh1771glc_log("bh1771_hw_init : error value = 0x%x \n", ret);
		return ret;
	}
	
	ret = bh1771_ps_int_th_l(2, val[8]);
	if (ret < 0) {
		bh1771glc_log("bh1771_hw_init : error value = 0x%x \n", ret);
		return ret;
	}
	
	ret = bh1771_ps_int_hysteresis(val[9]);
	if (ret < 0) {
		bh1771glc_log("bh1771_hw_init : error value = 0x%x \n", ret);
		return ret;
	}
	
	ret = bh1771_ps_persistence(val[10]);
	if (ret < 0) {
		bh1771glc_log("bh1771_hw_init : error value = 0x%x \n", ret);
		return ret;
	}
#endif
	ret = bh1771_ps_led_current(val[11], (const int *)&val[12]);
	/* Restore power status for sensors */
	if (ENABLE == als_pw_flg) {
		bh1771_als_power_status(CTL_STANDALONE);
	}
	if (ENABLE == ps_pw_flg) {
		bh1771_ps_power_status(CTL_STANDALONE);
	}
	return ret;
}

#ifdef PROXIMITY_INT
/*************************************************************************
 *	name	=	bh1771_ps_irq_handler
 *	func	=	Handle interrupt processing
 *	input	=	int irq, void *dev_id
 *	output	=	None
 *	return	=	IRQ_HANDLED
 *************************************************************************/
static irqreturn_t bh1771_ps_irq_handler(int irq, void *dev_id)
{
	if (irq != -1) {
		disable_irq_nosync(irq);
		queue_work(workqueue, &bh1771_ginfo->ps_work_irq);
	}
	return IRQ_HANDLED;
}

/*************************************************************************
 *	name	=	bh1771_ps_int_mode
 *	func	=	Set interrupt mode for PS device driver
 *	input	=	unsigned char val
 *	output	=	None
 *	return	=	0,	-EIO,	-EINVAL
 *************************************************************************/
static int bh1771_ps_int_mode(unsigned char val)
{
	int ret = 0;
    unsigned char int_reg = 0;
	unsigned char current_val = 0;
	
	if (val < MODE_NONUSE || val > MODE_BOTH) {
		return -EINVAL;
	}
	disable_irq_nosync(g_client->irq);
	/* read value from REG_INTERRUPT register via i2c */
	ret = bh1771_i2c_read(REG_INTERRUPT, &int_reg, 1);
	if (ret < 0) {
	return ret;
	}
	current_val = int_reg & 0x03;
	if (current_val == val) {
		enable_irq(g_client->irq);
		return ret;
	}
	int_reg = (int_reg & CLR_LOW2BIT) | val;
	/* write value to REG_INTERRUPT register via i2c */
	ret = bh1771_i2c_write(REG_INTERRUPT, &int_reg);
	if (ret < 0) {
	return ret;
	}
	enable_irq(g_client->irq);
	return ret;
}

/*************************************************************************
 *	name	=	bh1771_ps_int_th_h
 *	func	=	Set interrupt high threshold for PS device driver
 *	input	=	unsigned char led_id, unsigned char val
 *	output	=	None
 *	return	=	0,	-EIO,	-EINVAL
 *************************************************************************/
static int bh1771_ps_int_th_h(unsigned char led_id, unsigned char val)
{
	int ret = 0;
	
	if (led_id < PS_LED1 || led_id > PS_LED3) {
		return -EINVAL;
	}
	if (val < 0 || val > 255) {
		return -EINVAL;
	}
    /* write value to REG_PSTH_H register via i2c */
	switch (led_id) {
	case PS_LED1:
		ret = bh1771_i2c_write(REG_PSTH_H_L1, &val);
		break;
	case PS_LED2:
		ret = bh1771_i2c_write(REG_PSTH_H_L2, &val);
		break;
	case PS_LED3:
		ret = bh1771_i2c_write(REG_PSTH_H_L3, &val);
		break;
	default:
		break;
	}
	return ret;
}

/*************************************************************************
 *	name	=	bh1771_ps_int_th_l
 *	func	=	Set interrupt low threshold for PS device driver
 *	input	=	unsigned char led_id, unsigned char val
 *	output	=	None
 *	return	=	0,	-EIO,	-EINVAL
 *************************************************************************/
static int bh1771_ps_int_th_l(unsigned char led_id, unsigned char val)
{
	int ret = 0;
	
	if (led_id < PS_LED1 || led_id > PS_LED3) {
		return -EINVAL;
	}
	if (val < 0 || val > 255) {
		return -EINVAL;
	}
    /* write value to REG_PSTH_L register via i2c */
	switch (led_id) {
	case PS_LED1:
		ret = bh1771_i2c_write(REG_PSTH_L_L1, &val);
		break;
	case PS_LED2:
		ret = bh1771_i2c_write(REG_PSTH_L_L2, &val);
		break;
	case PS_LED3:
		ret = bh1771_i2c_write(REG_PSTH_L_L3, &val);
		break;
	default:
		break;
	}
	return ret;
}

/*************************************************************************
 *	name	=	bh1771_ps_int_hysteresis
 *	func	=	Set interrupt hysteresis for PS device driver
 *	input	=	unsigned char val
 *	output	=	None
 *	return	=	0,	-EIO,	-EINVAL
 *************************************************************************/
static int bh1771_ps_int_hysteresis(unsigned char val)
{
	int ret = 0;
	unsigned char int_reg = 0;
	unsigned char current_val = 0;
	
	if (val < PSH_THLETH_ONLY || val > PSHL_THLETH_BOTH) {
		return -EINVAL;
	}
	/* read value from REG_INTERRUPT register via i2c */
	ret = bh1771_i2c_read(REG_INTERRUPT, &int_reg, 1);
	if (ret < 0) {
	return ret;
	}
	current_val = (int_reg >> 4) & 0x01;
	if (current_val == val) {
		return ret;
	}
	int_reg = (int_reg & 0XEF) | (val << 4);
    /* write value to REG_INTERRUPT register via i2c */
	ret = bh1771_i2c_write(REG_INTERRUPT, &int_reg);

	return ret;
}

/*************************************************************************
 *	name	=	bh1771_ps_persistence
 *	func	=	Set PS persistence
 *	input	=	unsigned char val
 *	output	=	None
 *	return	=	0,	-EIO,	-EINVAL
 *************************************************************************/
static int bh1771_ps_persistence(unsigned char val)
{
	int ret = 0;
	unsigned char per_reg = 0;
	unsigned char current_val = 0;
	if (val < 0 || val > 15) {
		return -EINVAL;
	}
	/* read value from REG_PERSISTENCE register via i2c */
	ret = bh1771_i2c_read(REG_PERSISTENCE, &per_reg, 1);
	if (ret < 0) {
	return ret;
	}
	current_val = per_reg & 0x0F;
	if (current_val == val) {
		return ret;
	}
	per_reg = (per_reg & 0XF0) | val;
    /* write value to REG_INTERRUPT register via i2c */
	ret = bh1771_i2c_write(REG_PERSISTENCE, &per_reg);

	return ret;
}

#endif
/*************************************************************************
 *	name	=	bh1771_ps_led_current
 *	func	=	Set PS led current consumption
 *	input	=	unsigned char led_mode, unsigned char *val
 *	output	=	None
 *	return	=	0,	-EIO,	-EINVAL
 *************************************************************************/
static int bh1771_ps_led_current(unsigned char led_mode, const int *val)
{
	int ret = 0;
	unsigned char led_reg = 0X00;

	if (led_mode < PS_LED_MODE1 || led_mode > PS_LED_MODE4) {
		return -EINVAL;
	}

	switch (led_mode) {
	case PS_LED_MODE1:
		if (*val < LEDCURRENT_5 || *val > LEDCURRENT_200) {
			return -EINVAL;
		}
		led_reg = *val;
		/* write value to REG_ILED register via i2c */
		ret = bh1771_i2c_write(REG_ILED, &led_reg);
		break;
		
	case PS_LED_MODE2:
		if (*val < LEDCURRENT_5 || *val > LEDCURRENT_200) {
			return -EINVAL;
		}
		if (*(val + 1) < LEDCURRENT_5 || *(val + 1) > LEDCURRENT_200) {
			return -EINVAL;
		}
		led_reg = 0X40 | (*val) | (*(val + 1) << 3);
		/* write value to REG_ILED register via i2c */
		ret = bh1771_i2c_write(REG_ILED, &led_reg);
		break;
		
	case PS_LED_MODE3:
		if (*val < LEDCURRENT_5 || *val > LEDCURRENT_200) {
			return -EINVAL;
		}
		if (*(val + 2) < LEDCURRENT_5 || *(val + 2) > LEDCURRENT_200) {
			return -EINVAL;
		}
		led_reg = 0x80 | (*val);
		/* write value to REG_ILED register via i2c */
		ret = bh1771_i2c_write(REG_ILED, &led_reg);
		if (ret < 0) {
			return ret;
		}
		led_reg = *(val + 2);
		/* write value to REG_ILED3 register via i2c */
		ret = bh1771_i2c_write(REG_ILED3, &led_reg);
		break;
		
	case PS_LED_MODE4:
		if (*val < LEDCURRENT_5 || *val > LEDCURRENT_200) {
			return -EINVAL;
		}
		if (*(val + 1) < LEDCURRENT_5 || *(val + 1) > LEDCURRENT_200) {
			return -EINVAL;
		}
		if (*(val + 2) < LEDCURRENT_5 || *(val + 2) > LEDCURRENT_200) {
			return -EINVAL;
		}
		led_reg = 0xC0 | (*val) | (*(val + 1) << 3);
		/* write value to REG_ILED register via i2c */
		ret = bh1771_i2c_write(REG_ILED, &led_reg);
		if (ret < 0) {
			return ret;
		}
		led_reg = *(val + 2);
		/* write value to REG_ILED3 register via i2c */
		ret = bh1771_i2c_write(REG_ILED3, &led_reg);
		break;
		
	default:
		break;
	}
	return ret;
}

/*************************************************************************
 *	name	=	bh1771_ps_measure_rate
 *	func	=	Set PS measure rate
 *	input	=	unsigned char val
 *	output	=	None
 *	return	=	0,	-EIO,	-EINVAL
 *************************************************************************/
static int bh1771_ps_measure_rate(unsigned char val)
{
	int ret = 0;
	unsigned char ps_meas_reg = 0x00;
	
	if (val < PSRATE_10 || val > PSRATE_2000) {
		return -EINVAL;
	}
	
	ps_meas_reg = ps_meas_reg | val;
	/* write value to REG_PSMEASRATE register via i2c */
	ret = bh1771_i2c_write(REG_PSMEASRATE, &ps_meas_reg);
		
	return ret;
}
/*************************************************************************
 *	name	=	bh1771_als_reset
 *	func	=	Reset BH1771 chip registers
 *	input	=	None
 *	output	=	None
 *	return	=	0,	-EIO,	-EINVAL
 *************************************************************************/
static int bh1771_als_reset(void)
{
	int result = 0;
	unsigned char value = 0;
	
	bh1771glc_log("Start \n");
	/* Update SW reset value */
	value = value | (1 << 2);
	result = bh1771_i2c_write(REG_ALSCONTROL, &value);
	if (result < 0) {
		bh1771glc_log("SW reset failed\n");
		return -EIO;
	}
	
	bh1771glc_log("End \n");
	return 0;
}
/*************************************************************************
 *	name	=	bh1771_als_resolution_mode
 *	func	=	Set resolution mode for ALS device driver
 *	input	=	unsigned char val
 *	output	=	None
 *	return	=	0,	-EIO,	-EINVAL
 *************************************************************************/
static int bh1771_als_resolution_mode(unsigned char val)
{
	int result = 0;
	unsigned char value = 0;
	unsigned char current_val = 0;
		
	bh1771glc_log("Start \n");
	if (val != ALSRES_HMODE && val != ALSRES_MMODE) {
		bh1771glc_log("Input invalid\n");
		return -EINVAL;
	}
	/* Read value from ALS_CONTROL register */
	result = bh1771_i2c_read(REG_ALSCONTROL, &value, 1);
	if (result < 0) {
		bh1771glc_log("Read error\n");
		return -EIO;
    }
	current_val = (value >> 3) & 0x01;
	if (current_val == val) {
		return 0;
	}
	/* Update resolution setting */
	value = (value & 0xF7) | (val << 3);
	/* Write resolution setting to ALS_CONTROL register */
	result = bh1771_i2c_write(REG_ALSCONTROL, &value);
	if (result < 0) {
		bh1771glc_log("Set ALS resolution mode failed\n");
		return -EIO;
	}
	
	bh1771glc_log("End \n");
	return 0;
}
/*************************************************************************
 *	name	=	bh1771_als_sensitivity
 *	func	=	Set sensitivity level for ALS device driver
 *	input	=	unsigned char val
 *	output	=	None
 *	return	=	0,	-EIO,	-EINVAL
 *************************************************************************/
static int bh1771_als_sensitivity(unsigned char val)
{
	int result = 0;	
	unsigned char current_val = 0;
	
	bh1771glc_log("Start \n");
	if (val < 0x18 || val > 0xFE) {
		bh1771glc_log("Input invalid\n");
		return -EINVAL;
	}
	/* Read value from ALS_SENSITIVITY register */
	result = bh1771_i2c_read(REG_ALSSENSITIVITY, &current_val, 1);
	if (result < 0) {
		bh1771glc_log("Read error\n");
		return -EIO;
    }
	if (current_val == val) {
		goto set_als_ill_per_count;
	}
	/* Write sensitivity level to ALS_SENSITIVITY register */
	result = bh1771_i2c_write(REG_ALSSENSITIVITY, &val);
	if (result < 0) {
		bh1771glc_log("Set ALS sensitivity failed\n");
		return -EIO;
    }
	
set_als_ill_per_count:
	/* Store illuminant per 1 count  */
	bh1771_ginfo->als_ill_per_count = (u16)((53 * 100 * ROUND_FACTOR)/val);
	
	bh1771glc_log("End \n");
	return 0;
}
/*************************************************************************
 *	name	=	bh1771_als_output_data_rate
 *	func	=	Set output data rate for ALS device driver
 *	input	=	unsigned char val
 *	output	=	None
 *	return	=	0,	-EIO,	-EINVAL
 *************************************************************************/
static int bh1771_als_output_data_rate(unsigned char val)
{
	int result = 0;
	unsigned char value = 0;
	unsigned char current_val = 0;
	
	bh1771glc_log("Start \n");
	if (val < ALSRATE_100 || val > ALSRATE_2000) {
		bh1771glc_log("Input invalid\n");
		return -EINVAL;
	}
	/* Read value from ALS_MEAS_RATE register */
	result = bh1771_i2c_read(REG_ALSMEASRATE, &value, 1);
	if (result < 0) {
		bh1771glc_log("Read error\n");
		return -EIO;
    }
	current_val = value & 0x07;
	if (current_val == val) {
		return 0;
	}
	/* Update odr setting */
	value = (value & 0xF8) | val;
	/* Write ODR setting to ALS_MEAS_RATE register */
	result = bh1771_i2c_write(REG_ALSMEASRATE, &value);
	if (result < 0) {
		bh1771glc_log("Set ALS odr failed\n");
		return -EIO;
    }
	
	bh1771glc_log("End \n");
    return 0;
}

/*************************************************************************
 *	name	=	bh1771_misc_open
 *	func	=	Open PS and ALS device drivers
 *	input	=	struct inode *ip, struct file *fp
 *	output	=	None
 *	return	=	0
 *************************************************************************/
static int bh1771_misc_open(struct inode *ip, struct file *fp)
{
	return 0;
}

/*************************************************************************
 *	name	=	bh1771_misc_release
 *	func	=	Release PS and ALS device drivers
 *	input	=	struct inode *ip, struct file *fp
 *	output	=	None
 *	return	=	0
 *************************************************************************/
static int bh1771_misc_release(struct inode *ip, struct file *fp)
{
	return 0;
}

/*************************************************************************
 *	name	=	bh1771_misc_ioctl
 *	func	=	IO control for PS and ALS device drivers
 *	input	=	struct file *filp, unsigned int cmd, unsigned long arg
 *	output	=	None
 *	return	=	0, -ENOTTY, -EFAULT, -EINVAL, -EIO
 *************************************************************************/
static long bh1771_misc_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *) arg;
	int ret = 0;
	unsigned char flag = 0;
	int delay_time = 0;
	int nv_addr = 0;
	int poll_interval = 0;
	int i = 0;
	
	if ((cmd != IOCTL_ALS_ENABLE) &&
	    (cmd != IOCTL_ALS_SET_DELAY) &&
	    (cmd != IOCTL_PS_ENABLE) &&
		(cmd != IOCTL_PS_SET_DELAY) &&
		(cmd != IOCTL_GET_NV_ADDRESS)) {
		bh1771glc_log("invalid ioctl cmd\n");
		return -ENOTTY;
	}

	mutex_lock(&bh1771_ginfo->lock);
	wake_lock(&bh1771_ginfo->wakelock);

	switch (cmd) {
	case IOCTL_ALS_ENABLE:
		bh1771glc_log("IOCTL_ALS_ENABLE\n");
		/* Get input value from user */
		if (copy_from_user(&flag, argp, sizeof(flag)) != 0) {
			bh1771glc_log("copy_from_user error\n");
			ret = -EFAULT;
			break;
		}
		
		if (ENABLE == flag) {
			/* ALS is already in activate state, return 0*/
			if (1 == atomic_read(&bh1771_ginfo->als_enable)) {
				ret = 0;
				break;
			}
			if (bh1771_ginfo->als_delay <= MAX_DELAY_TIME) {
				ret = bh1771_als_power_status(CTL_STANDALONE);
				if (ret < 0) {
					break;
				}
			}
			hrtimer_start(&bh1771_ginfo->als_timer, ktime_set(0, bh1771_ginfo->als_poll_interval * NSEC_PER_MSEC), HRTIMER_MODE_REL);
			/* set ALS enable flag*/
			atomic_set(&bh1771_ginfo->als_enable, 1);
		} else {
			if (DISABLE == flag) {
				/* ALS is already in deactive state, return 0*/
				if (0 == atomic_read(&bh1771_ginfo->als_enable)) {
					ret = 0;
					break;
				}
				ret = bh1771_als_power_status(CTL_STANDBY);
				if (ret < 0) {
					break;
				}
				hrtimer_cancel(&bh1771_ginfo->als_timer);
			} else {
				ret = -EINVAL;
				break;
			}
			atomic_set(&bh1771_ginfo->als_enable, 0);
		}
		ret = 0;
		break;

	case IOCTL_PS_ENABLE:
		bh1771glc_log("IOCTL_PS_ENABLE\n");
		/* Get input value from user */
		if (copy_from_user(&flag, argp, sizeof(flag)) != 0) {
			bh1771glc_log("copy_from_user error\n");
			ret = -EFAULT;
			break;
		}

		if (ENABLE == flag) {
			/* PS is already in activate state, return 0*/
			if (1 == atomic_read(&bh1771_ginfo->ps_enable)) {
				ret = 0;
				break;
			}
			if (bh1771_ginfo->ps_delay <= MAX_DELAY_TIME) {
				ret = bh1771_ps_power_status(CTL_STANDALONE);
				if (ret < 0) {
					break;
				}
			}
#ifdef PROXIMITY_INT
			enable_irq(g_client->irq);
			if (bh1771_ginfo->ps_delay <= MAX_DELAY_TIME) {
				atomic_set(&bh1771_ginfo->ps_enable, 1);
				ret = 0;
				break;
			}
#endif
			/* start high resolution timer */
			hrtimer_start(&bh1771_ginfo->ps_timer, ktime_set(0, bh1771_ginfo->ps_poll_interval * NSEC_PER_MSEC), HRTIMER_MODE_REL);
			/* set PS enable flag*/
			atomic_set(&bh1771_ginfo->ps_enable, 1);
		} else {
			if (DISABLE == flag) {
				/* PS is already in deactivate state, return 0*/
				if (0 == atomic_read(&bh1771_ginfo->ps_enable)) {
					ret = 0;
					break;
				}
				ret = bh1771_ps_power_status(CTL_STANDBY);
				if (ret < 0) {
					break;
				}
				hrtimer_cancel(&bh1771_ginfo->ps_timer);
#ifdef PROXIMITY_INT
				disable_irq_nosync(g_client->irq);
				if (bh1771_ginfo->ps_delay > MAX_DELAY_TIME) {
					hrtimer_cancel(&bh1771_ginfo->ps_timer);
				}
#endif
			} else {
				ret = -EINVAL;
				break;
			}
			atomic_set(&bh1771_ginfo->ps_enable, 0);
		}
		ret = 0;
		break;
		
	case IOCTL_ALS_SET_DELAY:
		bh1771glc_log("IOCTL_ALS_SET_DELAY\n");
		/* Get input value from user */
		if (copy_from_user(&delay_time, argp, sizeof(delay_time)) != 0) {
			bh1771glc_log("copy_from_user error\n");
			ret = -EFAULT;
			break;
		}

		if (delay_time > MAX_DELAY_TIME) {
			poll_interval = delay_time - MIN_DELAY_TIME;
		} else {
			if (delay_time < MIN_DELAY_TIME) {
				poll_interval = MIN_DELAY_TIME;
			} else {
				poll_interval = delay_time;
			}
		}
		
		/* if poll interval time > MAX_DELAY_TIME, set minimum out put data rate for ALS */
		if (poll_interval > MAX_DELAY_TIME) {
			ret = bh1771_als_output_data_rate(MIN_DELAY_TIME);
			if (ret < 0) {
				break;
			}
		} else { /* if poll interval time <= MAX_DELAY_TIME */
			/* search proximate output data rate then set for ALS */
			for (i = ARRAY_SIZE(odr_table_als) - 1; i > 0; i--) {
				if (odr_table_als[i].poll_rate_ms <= poll_interval)
					break;
			}
			ret = bh1771_als_output_data_rate(odr_table_als[i].mask);
			if (ret < 0) {
				break;
			}
		}
		
		/* processing if ALS is already activate before */
		if (1 == atomic_read(&bh1771_ginfo->als_enable)) {
			hrtimer_cancel(&bh1771_ginfo->als_timer);
			if (delay_time <= MAX_DELAY_TIME) {
				ret = bh1771_als_power_status(CTL_STANDALONE);
				if (ret < 0) {
					break;
				}
			} else {
				ret = bh1771_als_power_status(CTL_STANDBY);
				if (ret < 0) {
					break;
				}
			}
			/* start high resolution timer */
			hrtimer_start(&bh1771_ginfo->ps_timer, ktime_set(0, poll_interval * NSEC_PER_MSEC), HRTIMER_MODE_REL);
		}
		bh1771_ginfo->als_delay = delay_time;
		bh1771_ginfo->als_poll_interval = poll_interval;
		break;
		
	case IOCTL_PS_SET_DELAY:
		bh1771glc_log("IOCTL_PS_SET_DELAY\n");
		/* Get input value from user */
		if (copy_from_user(&delay_time, argp, sizeof(delay_time)) != 0) {
			bh1771glc_log("copy_from_user error\n");
			ret = -EFAULT;
			break;
		}

		if (delay_time > MAX_DELAY_TIME) {
			poll_interval = delay_time - MIN_DELAY_TIME;
		} else {
			if (delay_time < MIN_DELAY_TIME) {
				poll_interval = MIN_DELAY_TIME;
			} else {
				poll_interval = delay_time;
			}
		}
#ifdef PROXIMITY_INT
		if (1 == atomic_read(&bh1771_ginfo->ps_enable)) {
			if (bh1771_ginfo->ps_delay > MAX_DELAY_TIME) {
				hrtimer_cancel(&bh1771_ginfo->ps_timer);
			}
			if (delay_time > MAX_DELAY_TIME) {
				ret = bh1771_ps_power_status(CTL_STANDBY);
				if (ret < 0) {
					break;
				}
				ret = bh1771_ps_measure_rate(MIN_DELAY_TIME);
				if (ret < 0) {
					break;
				}
				/* start high resolution timer */
				hrtimer_start(&bh1771_ginfo->ps_timer, ktime_set(0, poll_interval * NSEC_PER_MSEC), HRTIMER_MODE_REL);
				/* Store input value to local variable for PS: ps_delay = input value */
				bh1771_ginfo->ps_delay = delay_time;
				/* calculate polling interval and store to global variable */
				bh1771_ginfo->ps_poll_interval = poll_interval;
				ret = 0;
				break;
			}
		}
#endif
		if (poll_interval > MAX_DELAY_TIME) {
			ret = bh1771_ps_measure_rate(MIN_DELAY_TIME);
			if (ret < 0) {
				break;
			}
		} else { /* if poll interval time <= MAX_DELAY_TIME */
			/* search proximate output data rate then set for ALS */
			for (i = ARRAY_SIZE(odr_table_ps) - 1; i > 0; i--) {
				if (odr_table_ps[i].poll_rate_ms <= poll_interval)
					break;
			}
			ret = bh1771_ps_measure_rate(odr_table_ps[i].mask);
			if (ret < 0) {
				break;
			}
		}
		
		if (1 == atomic_read(&bh1771_ginfo->ps_enable)) {
			hrtimer_cancel(&bh1771_ginfo->ps_timer);
			if (delay_time <= MAX_DELAY_TIME) {
				ret = bh1771_ps_power_status(CTL_STANDALONE);
				if (ret < 0) {
					break;
				}
			} else {
				ret = bh1771_ps_power_status(CTL_STANDBY);
				if (ret < 0) {
					break;
				}
			}
			/* start high resolution timer */
			hrtimer_start(&bh1771_ginfo->ps_timer, ktime_set(0, poll_interval * NSEC_PER_MSEC), HRTIMER_MODE_REL);
		}
#ifdef PROXIMITY_INT
		if (1 == atomic_read(&bh1771_ginfo->ps_enable)) {
			if (delay_time <= MAX_DELAY_TIME) {
				ret = bh1771_ps_power_status(CTL_STANDALONE);
				if (ret < 0) {
					break;
				}
			}
		}
#endif
		/* Store input value to local variable for PS: ps_delay = input value */
		bh1771_ginfo->ps_delay = delay_time;
		/* calculate polling interval and store to global variable */
		bh1771_ginfo->ps_poll_interval = poll_interval;
		break;
		
	case IOCTL_GET_NV_ADDRESS:
		bh1771glc_log("IOCTL_GET_NV_ADDRESS\n");
		/* Get input value from user */
		if (copy_from_user(&nv_addr, argp, sizeof(nv_addr)) != 0) {
			bh1771glc_log("copy_from_user error\n");
			ret = -EFAULT;
			break;
		}
		//Get NV settings from user input address and Set NV settings to ALS and PS register
		bh1771_get_nv(nv_addr);
		ret = bh1771_hw_init(bh1771_setting_nv);
		if (ret < 0) {
			break;
		}
		break;

	default:
		break;
	}

	wake_unlock(&bh1771_ginfo->wakelock);
	mutex_unlock(&bh1771_ginfo->lock);

	return ret;
}

/*************************************************************************
 *	name	=	bh1771_init
 *	func	=	Initialize I2C slave device for PS and ALS
 *	input	=	None
 *	output	=	None
 *	return	=	0,	-ENOTSUPP
 *************************************************************************/
static int __init bh1771_init(void)
{
	struct i2c_board_info i2c_info;
	struct i2c_adapter *adapt = NULL;	
	int ret = 0;
			
	bh1771glc_log("Start \n");
#ifdef PROXIMITY_INT
	bh1771glc_log("Interrupt is used\n");
	/* Set gpio for sensor */
	ret = gpio_request(GPIO_PORT108, NULL);         
	if (ret < 0) {
		return -ENOTSUPP;
	}	
	/* Set direction for GPIO_PORT108 */
	ret = gpio_direction_input(GPIO_PORT108);
	if (ret < 0) {
		ret = -ENOTSUPP;
		goto error_gpio;
	}	
#else
	bh1771glc_log("Polling is used\n");
#endif
	
#if 0
	/*Request PMIC to supply power for bh1771glc device*/
	ret = pmic_set_power_on(E_POWER_VANA_MM);
	if (ret < 0) 
	{
		bh1771glc_log("Request pmic error\n");
		ret = -ENOTSUPP;
		goto error_pmic;
	}
#endif

	/* Register bh1771glc driver to i2c bus */
	ret = i2c_add_driver(&bh1771_i2c_driver);
	if (0 != ret) 
	{
		bh1771glc_log("BH1771 can't add i2c driver\n");
		ret = -ENOTSUPP;
		goto error_i2c_add_driver;
	}
	/* Initialize i2c device information */
	memset(&i2c_info, 0 , sizeof(struct i2c_board_info));
	i2c_info.addr = BH1771_I2C_ADDRESS;
#ifdef PROXIMITY_INT
	i2c_info.irq = irqpin2irq(PROX_IRQ);
	i2c_info.flags = IORESOURCE_IRQ | IRQ_TYPE_EDGE_FALLING;
#endif
	strlcpy(i2c_info.type, BH1771_I2C_NAME, I2C_NAME_SIZE);
	/* Get i2c adapter */
	adapt = i2c_get_adapter(BH1771_I2C_CHANNEL);
	if (NULL == adapt) {
		bh1771glc_log("i2c_get_adapter error\n");
		ret = -ENOTSUPP;
		goto error_i2c_get_adapter;
	}
	
	/* Create i2c device */
	g_client = i2c_new_device(adapt, &i2c_info);
	if (NULL == g_client) {
		bh1771glc_log("i2c_new_device error\n");
		ret = -ENOTSUPP;
		goto error_i2c_new_device;
	}

	/* Register adapter to i2c bus */
	i2c_put_adapter(adapt);
	
	bh1771glc_log("End \n");
	return 0;
	
error_i2c_new_device:   
error_i2c_get_adapter:
	bh1771glc_log("error_i2c_client\n");	
	i2c_del_driver(&bh1771_i2c_driver);

error_i2c_add_driver:
#if 0
	pmic_set_power_off(E_POWER_VANA_MM);
	
error_pmic:
#endif

#ifdef PROXIMITY_INT
error_gpio:
	gpio_free(GPIO_PORT108);
#endif

	return ret;
}

/*************************************************************************
 *	name	=	bh1771_exit
 *	func	=	Finalize I2C slave device for PS and ALS
 *	input	=	None
 *	output	=	None
 *	return	=	None
 *************************************************************************/
static void __exit bh1771_exit(void)
{
	/*Remove i2c adapter to i2c bus*/
	i2c_del_driver(&bh1771_i2c_driver);
	
	/* unregister device */
	if (g_client != NULL) {
		i2c_unregister_device(g_client);       
	}
	
#ifdef PROXIMITY_INT
	gpio_free(GPIO_PORT108);
#endif	

#if 0
	/*Request PMIC not to supply power for bh1771glc device*/
	pmic_set_power_off(E_POWER_VANA_MM);
#endif
}

module_init(bh1771_init);
module_exit(bh1771_exit);

MODULE_AUTHOR("Renesas");
MODULE_DESCRIPTION("Proximity Sensor driver for bh1771");
MODULE_LICENSE("GPL v2");
