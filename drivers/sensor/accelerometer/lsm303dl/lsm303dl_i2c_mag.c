/* drivers/sensor/accelerometer/lsm303dl/lsm303dl_i2c_mag.c
 *
 * Copyright (C) 2012 Renesas Mobile Corporation. 
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
#include <linux/module.h>
#include <linux/wakelock.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <mach/irqs.h>
#include <mach/r8a73734.h>
#include <linux/lsm303dl.h>
#include "lsm303dl_local.h"


#ifdef KOTA_ENV
	#include <linux/pmic/pmic.h>
#endif

/************************************************
*   	Global variable definition section	 	*
************************************************/ 
static struct lsm303dl_mag_data *lsm303dl_mag_info = NULL;
static struct i2c_client *mag_client = NULL;

/*Output data rate looked-up table*/
static const struct lsm303dl_output_rate odr_table_mag[] = {
	{	5,		MAG_ODR_220			},
	{	13,		MAG_ODR_75			},
	{	34,		MAG_ODR_30			},
	{	67,		MAG_ODR_15			},
	{	134,	MAG_ODR_7_5			},
	{	334,	MAG_ODR_3_0			},
	{	667,	MAG_ODR_1_5			},
	{	1334,	MAG_ODR_0_75		},
};

/*Sensitivity unit looked-up table*/
static const struct lsm303dl_sens_table sens_table_mag[] = {
	{	0,			0,			0	},	/*Unused*/		
	{	1100,		1100,		980	},		
	{	855,		855,		760	},		
	{	670,		670,		600	},		
	{	450,		450,		400	},		
	{	400,		400,		355	},		
	{	330,		330,		295	},		
	{	230,		230,		205	}		
};

/*************************************************************************
 *	name	=	lsm303dl_mag_i2c_read
 *	func	=	Read data from specified register of magnetometer
 *	input	=	u8 reg, u8 *val, int len
 *	output	=	u8 *val
 *	return	=	0, -EIO, -EINVAL
 *************************************************************************/
static int lsm303dl_mag_i2c_read(u8 reg, u8 *val, int len)
{
	int 			ret = 0;
	int 			tries = 0;
	struct i2c_msg 	msg[2];
	
	/*Check input value*/
	if ((NULL == val) || (NULL == mag_client)) {
		lsm303dl_log("Read buffer is NULL\n");
		return -EINVAL;
	}
	
	/*Initialization for all elements of i2c_msg structure*/
	memset(msg, 0, sizeof(msg));
	
	msg[0].addr = mag_client->addr;
	msg[0].flags = 0;
	msg[0].len = 1;
	msg[0].buf = (u8 *)&reg;
	
	msg[1].addr = mag_client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = len;
	msg[1].buf = val;

	do {
		ret = i2c_transfer(mag_client->adapter, msg, 2);
		if (ret != 2) {
			msleep_interruptible(LSM303DL_I2C_RETRY_DELAY);
		}
	} while ((ret != 2) && (++tries < LSM303DL_I2C_RETRIES));

	if (ret != 2) {
		lsm303dl_log("Read transfer error\n");
		ret = -EIO;
	} else {
		ret = 0;
	}

	return ret;
}

/*************************************************************************
 *	name	=	lsm303dl_mag_i2c_write
 *	func	=	Write data to specified register of magnetometer
 *	input	=	u8 reg, u8 *val, int len
 *	output	=	None
 *	return	=	0, -EIO, -EINVAL
 *************************************************************************/
static int lsm303dl_mag_i2c_write(u8 reg, u8 *val, int len)
{
	int 			ret = 0;
	int 			tries = 0;
	struct i2c_msg 	msg;
	u8 				*data; 
	
	/*Check input value*/
	if ((NULL == val) || (NULL == mag_client)) {
		lsm303dl_log("Write buffer is NULL\n");
		return -EINVAL;
	}
	
	/*Allocate internal buffer*/
	data = kzalloc(len + 1, GFP_KERNEL);
	if (NULL == data) {
		lsm303dl_log("Allocate internal buffer error\n");
		return -EIO;
	}
	
	/*Initialization for all elements of i2c_msg structure*/
	memset(&msg, 0, sizeof(msg));
	
	data[0] = reg;
	memcpy((void *)&data[1], (void *)val, len);
	
	msg.addr = mag_client->addr;
	msg.flags = 0;
	msg.len = len + 1;
	msg.buf = data;

	/*Write data to a specific register address of accelerometer hardware via I2C bus*/
	do {
		ret = i2c_transfer(mag_client->adapter, &msg, 1);
		if (ret != 1) {
			msleep_interruptible(LSM303DL_I2C_RETRY_DELAY);
		}
	} while ((ret != 1) && (++tries < LSM303DL_I2C_RETRIES));
	
	if(ret != 1) {
		lsm303dl_log("Write transfer error\n");
		ret = -EIO;
	} else {
		ret = 0;
	}

	kfree(data);
	return ret;
}

/*************************************************************************
 *	name	=	lsm303dl_mag_set_sensitivity
 *	func	=	Change magnetometer sensitivity
 *	input	=	u8 val
 *	output	=	None
 *	return	=	0, -EIO, -EINVAL
 *************************************************************************/
static int lsm303dl_mag_set_sensitivity(u8 val)
{
	int 	ret 				= 0;
	u8 		reg_value			= 0;
	u8		current_setting 	= 0;
	
	if ((val < MAG_SENS_1_3) || (val > MAG_SENS_8_1)) {
		lsm303dl_log("Invalid input argument\n");
		return -EINVAL;
	}
	
	/*Read the content of CRB_REG_M register*/
	ret = lsm303dl_mag_i2c_read(CRB_REG_M, &reg_value, 1);
	if (ret < 0) {
		lsm303dl_log("Fail to read data from Magnetometer\n");
		return ret;
	}
	
	current_setting = (reg_value >> 5) & 0x07;
	if (val == current_setting)
	{
		lsm303dl_mag_info->sensitivity = val;
		return 0;
	}
	
	reg_value = (reg_value & 0x1F) | ((val << 5) & 0xE0);
	
	/*Write new value to CRB_REG_M register*/
	ret = lsm303dl_mag_i2c_write(CRB_REG_M, &reg_value, 1);
	if (ret < 0) {
		lsm303dl_log("Fail to write data to Magnetometer\n");
		return ret;
	}
	
	/*Save sensitivity level to global variable for further usage*/
	lsm303dl_mag_info->sensitivity = val;
	
	return 0;
}

/*************************************************************************
 *	name	=	lsm303dl_mag_power_status
 *	func	=	Power up/down magnetometer
 *	input	=	u8 val
 *	output	=	None
 *	return	=	0, -EIO, -EINVAL
 *************************************************************************/
static int lsm303dl_mag_power_status(u8 val)
{
	int 	ret 				= 0;
	u8 		reg_value			= 0;
	u8		current_setting 	= 0;
	
	if ((val != MAG_NORMAL) && (val != MAG_STANDBY )) {
		lsm303dl_log("Invalid input argument\n");
		return -EINVAL;
	}
	
	/*Read the content of MR_REG_M register*/
	ret = lsm303dl_mag_i2c_read(MR_REG_M, &reg_value, 1);
	if (ret < 0) {
		lsm303dl_log("Fail to read data from Magnetometer\n");
		return ret;
	}
	
	current_setting = reg_value & 0x03;
	if (val == current_setting)
	{
		return 0;
	}
	
	reg_value = (reg_value & 0xFC) | val;
	
	/*Write new value to MR_REG_M register*/
	ret = lsm303dl_mag_i2c_write(MR_REG_M, &reg_value, 1);
	if (ret < 0) {
		lsm303dl_log("Fail to write data to Magnetometer\n");
		return ret;
	}
	
	return 0;
}

/*************************************************************************
 *	name	=	lsm303dl_mag_set_odr
 *	func	=	Change magnetometer output data rate
 *	input	=	u32 val
 *	output	=	None
 *	return	=	0, -EIO
 *************************************************************************/
int lsm303dl_mag_set_odr(u32 val)
{
	int 	i 					= 0;
	int 	ret					= 0;
	u8 		activation_flg		= 0;
	u8 		reg_value			= 0;
	u8		odr				 	= 0;
	
	if (NULL == lsm303dl_mag_info) {
		lsm303dl_log("Invalid input argument\n");
		return -EINVAL;
	}
	
	/*Interrupt mechanism is used*/
	#ifdef LSM303DL_MAG_INT
	
		/*Get status of Magnetometer*/
		activation_flg = lsm303dl_get_sns_status(ID_MAG);
		if (MAG_ENABLE == activation_flg) {
		
			/*Previous time is greater than polling threshold*/
			if (lsm303dl_mag_info->delay > LSM303DL_POLL_THR) {
				hrtimer_cancel(&lsm303dl_mag_info->timer);
			}
			
			/*Input value is greater than polling threshold*/
			if (val > LSM303DL_POLL_THR) {
			
				/*Power down Magnetometer*/
				ret = lsm303dl_mag_power_status (MAG_STANDBY);
				if (ret < 0) {
					lsm303dl_log("Fail to power down Magnetometer\n");
					return -EIO;
				}
				
				/*Calculate delay time and store to global variable*/
				lsm303dl_mag_info->poll_interval = val - LSM303DL_ACTIVATE_TIME;
				
				/*Update input delay time to global variable*/
				lsm303dl_mag_info->delay = val;
				
				/*Read the content of CRA_REG_M register*/
				ret = lsm303dl_mag_i2c_read(CRA_REG_M, &reg_value, 1);
				if (ret < 0) {
					lsm303dl_log("Fail to read data from Magnetometer\n");
					return -EIO;
				}
	
				reg_value = (reg_value & 0xE3) | ((MAG_ODR_220 << 2) & 0x1C);
				
				/*Write new value to CRA_REG_M register*/
				ret = lsm303dl_mag_i2c_write(CRA_REG_M, &reg_value, 1);
				if (ret < 0) {
					lsm303dl_log("Fail to write data to Magnetometer\n");
					return -EIO;
				}
				
				hrtimer_start(	&lsm303dl_mag_info->timer, 
								ktime_set(0, lsm303dl_mag_info->poll_interval * NSEC_PER_MSEC), 
								HRTIMER_MODE_REL
							);
				
				return 0;
			}
		}
	#endif
	
	/*Find the output data rate in look-up table*/
	for (i = ARRAY_SIZE(odr_table_mag) -1; i > 0; i-- ) {
		if (odr_table_mag[i].poll_rate_ms <= val) {
			break;
		}
	}
	
	odr = odr_table_mag[i].mask;
	
	if (val > LSM303DL_POLL_THR) {
		odr = MAG_ODR_220;
	}
	
	/*Read the content of CRA_REG_M register*/
	ret = lsm303dl_mag_i2c_read(CRA_REG_M, &reg_value, 1);
	if (ret < 0) {
		lsm303dl_log("Fail to read data from Magnetometer\n");
		return -EIO;
	}

	reg_value = (reg_value & 0xE3) | ((odr << 2) & 0x1C);
	
	/*Write new value to CRA_REG_M register*/
	ret = lsm303dl_mag_i2c_write(CRA_REG_M, &reg_value, 1);
	if (ret < 0) {
		lsm303dl_log("Fail to write data to Magnetometer\n");
		return -EIO;
	}	
	
	/*Interrupt mechanism is used*/
	#ifdef LSM303DL_MAG_INT
		if (MAG_ENABLE == activation_flg) {
			if (lsm303dl_mag_info->delay > LSM303DL_POLL_THR) {
				if (val <= LSM303DL_POLL_THR) {
					
					/*Power on Magnetometer*/
					ret = lsm303dl_mag_power_status (MAG_NORMAL);
					if (ret < 0) {
						lsm303dl_log("Fail to power on Magnetometer\n");
						return -EIO;
					}
				}
			}
		}
	#endif
	
	/*Update input delay time to global variable*/
	lsm303dl_mag_info->delay = val;
	
	return 0;
}
EXPORT_SYMBOL(lsm303dl_mag_set_odr);

/*************************************************************************
 *	name	=	lsm303dl_mag_activate
 *	func	=	Activate/Deactivate Magnetometer
 *	input	=	u8 val
 *	output	=	None
 *	return	=	0, -EIO, -EINVAL
 *************************************************************************/
int lsm303dl_mag_activate(u8 val)
{
	int 	ret 			= 0;
	
	if ((val != MAG_ENABLE) && (val != MAG_DISABLE) ) {
		return -EINVAL;
	}
	
	if (NULL == lsm303dl_mag_info) {
		lsm303dl_log("Invalid input argument\n");
		return -EINVAL;
	}
	
	if (MAG_ENABLE == val) {
		
		/*Interrupt mechanism is used*/
		#ifdef LSM303DL_MAG_INT
		
			/*Enable interrupt processing*/
			enable_irq(lsm303dl_mag_info->drdy);
			
			if (lsm303dl_mag_info->delay > LSM303DL_POLL_THR) {
				hrtimer_start(	&lsm303dl_mag_info->timer, 
								ktime_set(0, lsm303dl_mag_info->poll_interval * NSEC_PER_MSEC), 
								HRTIMER_MODE_REL
							);
				return 0;
			}
		#endif
		
		/*Power on Magnetometer*/
		ret = lsm303dl_mag_power_status (MAG_NORMAL);
		
		return ret;
	} else {
		/*Interrupt mechanism is used*/
		#ifdef LSM303DL_MAG_INT
		
			/*Disable interrupt processing*/
			disable_irq(lsm303dl_mag_info->drdy);
			
			if (lsm303dl_mag_info->delay > LSM303DL_POLL_THR) {
				hrtimer_cancel(&lsm303dl_mag_info->timer);
				return 0;
			}
		#endif
		
		/*Power down Magnetometer*/
		ret = lsm303dl_mag_power_status (MAG_STANDBY);
		
		return ret;
	}
}
EXPORT_SYMBOL(lsm303dl_mag_activate);

/*************************************************************************
 *	name	=	lsm303dl_mag_hw_init
 *	func	=	Initialize hardware setting for magnetometer
 *	input	=	u8 *val
 *	output	=	None
 *	return	=	0, -EIO, -EINVAL
 *************************************************************************/
int lsm303dl_mag_hw_init(u8 *val)
{
	int ret = 0;
	
	if ((NULL == val) | (NULL == lsm303dl_mag_info)) {
		lsm303dl_log("Input value is NULL\n");
		return -EINVAL;
	}
	
	ret = lsm303dl_mag_set_sensitivity(val[0]);
	
	return ret;
}
EXPORT_SYMBOL(lsm303dl_mag_hw_init);

/*************************************************************************
 *	name	=	lsm303dl_mag_get_data
 *	func	=	Get magnetometer values from magnetometer device
 *	input	=	None
 *	output	=	s16 *data
 *	return	=	0, -EIO, -EINVAL
 *************************************************************************/
int lsm303dl_mag_get_data(s16 *data)
{
	int 	ret 			= 0;
	u8 		reg_value[6] 	= { 0 };
	s32		hw_data[3]		= { 0 };
	u8		idx				=	0;
	
	if (NULL == lsm303dl_mag_info) {
		lsm303dl_log("Invalid input argument\n");
		return -EINVAL;
	}
	idx	 = lsm303dl_mag_info->sensitivity;
	
	wake_lock(&lsm303dl_mag_info->wakelock);
	
	if (NULL == data) {
		lsm303dl_log("Input value is NULL\n");
		ret = -EINVAL;
		goto err;
	}
	
	/*Get x, y and z axis value from Magnetometer*/
	ret = lsm303dl_mag_i2c_read(OUT_X_H_M, reg_value, 6);
	if (ret < 0) {
		lsm303dl_log("Fail to read data from Magnetometer\n");
		goto err;
	}
	
	/*Calculate the actual value for magnetometer*/
	hw_data[0] = (int) (((reg_value[0]) << 8) | reg_value[1]);
	hw_data[1] = (int) (((reg_value[4]) << 8) | reg_value[5]);
	hw_data[2] = (int) (((reg_value[2]) << 8) | reg_value[3]);

	hw_data[0] = (hw_data[0] & 0x8000) ? (hw_data[0] | 0xFFFF0000) : (hw_data[0]);
	hw_data[1] = (hw_data[1] & 0x8000) ? (hw_data[1] | 0xFFFF0000) : (hw_data[1]);
	hw_data[2] = (hw_data[2] & 0x8000) ? (hw_data[2] | 0xFFFF0000) : (hw_data[2]);
	
	/*Adjust x-axis value based on sensitivity*/
	if (hw_data[0] != 0xF000) {
		data[0] = (hw_data[0] * 1000) / sens_table_mag[idx].x;
	} else {
		data[0] = 0x8000;
	}
	
	/*Adjust y-axis value based on sensitivity*/
	if (hw_data[1] != 0xF000) {
		data[1] = (hw_data[1] * 1000) / sens_table_mag[idx].y;
	} else {
		data[1] = 0x8000;
	}
	
	/*Adjust z-axis value based on sensitivity*/
	if (hw_data[2] != 0xF000) {
		data[2] = (hw_data[2] * 1000) / sens_table_mag[idx].z;
	} else {
		data[2] = 0x8000;
	}
	
err:
	wake_unlock(&lsm303dl_mag_info->wakelock);
	return ret;
}
EXPORT_SYMBOL(lsm303dl_mag_get_data);

#ifdef LSM303DL_MAG_INT
/*************************************************************************
 *	name	=	lsm303dl_mag_isr
 *	func	=	Handler for DRDY interrupt
 *	input	=	int irq, void *dev
 *	output	=	None
 *	return	=	IRQ_HANDLED
 *************************************************************************/
static irqreturn_t lsm303dl_mag_isr(int irq, void *dev)
{
	if (irq != -1) {
		disable_irq_nosync(irq);
		queue_work(lsm303dl_mag_info->drdy_workqueue, &lsm303dl_mag_info->drdy_work);
	}
	return IRQ_HANDLED;
}

/*************************************************************************
 *	name	=	lsm303dl_mag_irq_work_func
 *	func	=	Work function for getting magnetometer data and 
 *				reporting them to HAL when DRDY interrupt occurs
 *	input	=	struct work_struct *work
 *	output	=	None
 *	return	=	None
 *************************************************************************/
static void lsm303dl_mag_irq_work_func(struct work_struct *work)
{
	mutex_lock(&lsm303dl_mag_info->lock);
	
	/*Get x, y, z axis value and report to HAL layer*/
	lsm303dl_mag_report_values();
	
	if (lsm303dl_mag_info->delay > LSM303DL_POLL_THR) {
		lsm303dl_mag_power_status(MAG_STANDBY);
		hrtimer_start(	&lsm303dl_mag_info->timer, 
						ktime_set(0, lsm303dl_mag_info->poll_interval * NSEC_PER_MSEC), 
						HRTIMER_MODE_REL
					);
	}
	
	/*Enable interrupt processing*/
	enable_irq(lsm303dl_mag_info->drdy);
	mutex_unlock(&lsm303dl_mag_info->lock);
}

/*************************************************************************
 *	name	=	lsm303dl_mag_timer
 *	func	=	Handle timer interrupt for magnetometer
 *	input	=	struct hrtimer *timer
 *	output	=	None
 *	return	=	HRTIMER_NORESTART
 *************************************************************************/
static enum hrtimer_restart lsm303dl_mag_timer (struct hrtimer *timer)
{
	queue_work(lsm303dl_mag_info->drdy_workqueue, &lsm303dl_mag_info->drdy_work);
	return HRTIMER_NORESTART;
}

/*************************************************************************
 *	name	=	lsm303dl_mag_timer_work_func
 *	func	=	Work function for power Magnetometer device when timer 
 *				interrupt occurs
 *	input	=	struct work_struct *work
 *	output	=	None
 *	return	=	None
 *************************************************************************/
static void lsm303dl_mag_timer_work_func(struct work_struct *work)
{
	mutex_lock(&lsm303dl_mag_info->lock);
	
	if (lsm303dl_mag_info->delay > LSM303DL_POLL_THR) {
		lsm303dl_mag_power_status(MAG_NORMAL);
	}
	mutex_unlock(&lsm303dl_mag_info->lock);
}
#endif

/*************************************************************************
 *	name	=	lsm303dl_mag_suspend
 *	func	=	Suspend magnetometer device
 *	input	=	struct device *dev
 *	output	=	None
 *	return	=	0
 *************************************************************************/
static int lsm303dl_mag_suspend(struct device *dev)
{
	return 0;
}

/*************************************************************************
 *	name	=	lsm303dl_mag_resume
 *	func	=	Resume Magnetometer device
 *	input	=	struct device *dev
 *	output	=	None
 *	return	=	0
 *************************************************************************/
static int lsm303dl_mag_resume(struct device *dev)
{
	return 0;
}

/*************************************************************************
 *	name	=	lsm303dl_mag_i2c_probe
 *	func	=	Probe I2C slave device of magnetometer
 *	input	=	struct i2c_client *client, const struct i2c_device_id *devid
 *	output	=	None
 *	return	=	0
 *************************************************************************/
static int lsm303dl_mag_i2c_probe(struct i2c_client *client, const struct i2c_device_id *devid)
{
	int 	ret = 0;
	u8 		reg_default[3] = {0};
	
	lsm303dl_log("lsm303dl_mag_i2c_probe is called\n");
	
	/*Check functionalities of I2C adapter*/
	ret = i2c_check_functionality(client->adapter, I2C_FUNC_I2C);
	if (ret == 0) {
		lsm303dl_log("Magnetometer I2C is malfunction\n");
		return -EIO;
	}
	
	/*Allocate memory for lsm303dl_mag_data structure*/
	lsm303dl_mag_info = kzalloc(sizeof(struct lsm303dl_mag_data), GFP_KERNEL);
	if (NULL == lsm303dl_mag_info) {
		lsm303dl_log("Cannot allocate memmory for lsm303dl_mag_data structure\n");
		return -ENOMEM;
	}
	
	/*Initialize values for lsm303dl_mag_data structure*/
	lsm303dl_mag_info->sensitivity = 1;
	lsm303dl_mag_info->delay = 100;
	lsm303dl_mag_info->poll_interval = 100;
	
	mag_client = client;

	/*Initialize mutex and wake lock*/
	mutex_init(&lsm303dl_mag_info->lock);
	wake_lock_init(&lsm303dl_mag_info->wakelock,
			WAKE_LOCK_SUSPEND, "lsm303dl-mag-wakelock");
			
	mutex_lock(&lsm303dl_mag_info->lock);
	
	/*Initialize Magnetometer fixed and default setting*/
	reg_default[0] = 0x90;
	reg_default[1] = 0x20;
	reg_default[2] = 0x03;
	
	ret = lsm303dl_mag_i2c_write(CRA_REG_M, reg_default, 
					sizeof(reg_default)/sizeof(reg_default[0]));
	if (ret < 0) {
		lsm303dl_log("Cannot Initialize Magnetometer fixed and default setting\n");
		ret = -EIO;
		goto hw_init_err;
	}
	
	#ifdef LSM303DL_MAG_INT
		/*Initialize high resolution timer*/
		hrtimer_init(&lsm303dl_mag_info->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
		lsm303dl_mag_info->timer.function = lsm303dl_mag_timer;
		
		/*Create work queue structure for handling bottom-half interrupt*/
		lsm303dl_mag_info->drdy_workqueue = create_singlethread_workqueue("lsm303dl_mag_wq");
		if (NULL == lsm303dl_mag_info->drdy_workqueue) {
			lsm303dl_log("Cannot create work queue structure for Magnetometer\n");
			ret = -ENOMEM;
			goto hw_init_err;
		}
		
		INIT_WORK(&lsm303dl_mag_info->drdy_work, lsm303dl_mag_irq_work_func); 
		INIT_WORK(&lsm303dl_mag_info->timer_work, lsm303dl_mag_timer_work_func);		
		
		/*Register Magnetometer interrupt handler*/
		ret = request_irq(client->irq, lsm303dl_mag_isr, 0, "lsm303dl_mag_int", lsm303dl_mag_info);
		if (ret < 0) {
			lsm303dl_log("Cannot register Magnetometer interrupt handler\n");
			ret = -EIO;
			goto req_irq_err;
		}
		lsm303dl_mag_info->drdy = client->irq;
		disable_irq(lsm303dl_mag_info->drdy);
	#endif
	
	lsm303dl_mag_power_status(MAG_STANDBY);
	mutex_unlock(&lsm303dl_mag_info->lock);
	
	return 0;

#ifdef LSM303DL_MAG_INT
req_irq_err:	
	destroy_workqueue(lsm303dl_mag_info->drdy_workqueue);	
#endif	
	
hw_init_err:
	mutex_unlock(&lsm303dl_mag_info->lock);
	wake_lock_destroy(&lsm303dl_mag_info->wakelock);
	
	kfree(lsm303dl_mag_info);
	lsm303dl_mag_info = NULL;
	
	return ret;
}

/*************************************************************************
 *	name	=	lsm303dl_mag_i2c_remove
 *	func	=	Remove I2C slave device of magnetometer
 *	input	=	struct i2c_client *client
 *	output	=	None
 *	return	=	0
 *************************************************************************/
static int lsm303dl_mag_i2c_remove(struct i2c_client *client)
{
	#ifdef LSM303DL_MAG_INT
		destroy_workqueue(lsm303dl_mag_info->drdy_workqueue);
		free_irq(lsm303dl_mag_info->drdy, lsm303dl_mag_info);
	#endif	
	
	wake_lock_destroy(&lsm303dl_mag_info->wakelock);
	
	/*Release all allocated memory*/
	kfree(lsm303dl_mag_info);
	lsm303dl_mag_info = NULL;
	return 0;
}

/****************************************************
*   	I2C device id structure definition	 			*
*****************************************************/
static const struct i2c_device_id lsm303dl_mag_id[] = {
	{ 	LSM303DL_MAG_NAME, 		0 			},
	{ 										}
};

/****************************************************
*   	Power management structure definition	 	*
*****************************************************/ 
static struct dev_pm_ops lsm303dl_mag_pm_ops ={
	.suspend = lsm303dl_mag_suspend,
	.resume = lsm303dl_mag_resume,
};

/****************************************************
*   	I2C driver structure definition	 			*
*****************************************************/  
static struct i2c_driver mag_driver = {
	.probe     		= lsm303dl_mag_i2c_probe,
	.remove			= lsm303dl_mag_i2c_remove,
	.id_table  		= lsm303dl_mag_id,
	.driver			= 
	{
		.owner 		= THIS_MODULE,
		.name		= LSM303DL_MAG_NAME,
		.pm 		= &lsm303dl_mag_pm_ops,
	},
};

/*************************************************************************
 *	name	=	lsm303dl_mag_init
 *	func	=	Initialize I2C slave device of magnetometer
 *	input	=	None
 *	output	=	None
 *	return	=	0, -ENOTSUPP
 *************************************************************************/
static int __init lsm303dl_mag_init(void)
{
	int 					ret 		= 0;
	struct i2c_board_info 	i2c_info;
	
	struct i2c_adapter 		*adapter 	= NULL;
	struct i2c_client 		*client		= NULL;
	
	lsm303dl_log("lsm303dl_mag_init is called\n");

#ifdef KOTA_ENV
	/*Request PMIC to supply power for Magnetometer device*/
	ret = pmic_set_power_on(E_POWER_VANA_MM);
	if (ret < 0) {
		lsm303dl_log("Cannot request PMIC to supply power for Magnetometer device\n");
		return -ENOTSUPP;
	}
#endif

	#ifdef LSM303DL_MAG_INT
		/*GPIO is set here*/
		ret = gpio_request(GPIO_PORT109, NULL);         /*  Use the GPIO_PORT109 */
		if (ret < 0) {
			lsm303dl_log("Cannot request GPIO_PORT109 for magnetometer driver\n");
			ret = -ENOTSUPP;
			goto handle_power;
		}
		
		/* Set direction for GPIO_PORT109*/
		ret = gpio_direction_input(GPIO_PORT109);
		if (ret < 0) {
			lsm303dl_log("Cannot set direction of GPIO_PORT109 for magnetometer driver\n");
			ret = -ENOTSUPP;
			goto handle_gpio;
		}
	#endif

	/*Register magnetometer driver to I2C core*/
	ret = i2c_add_driver(&mag_driver);
	if (ret < 0) {
		lsm303dl_log("Cannot register magnetometer driver to I2C core\n");
		ret = -ENOTSUPP;
		goto handle_gpio;
	}
	
	/*Get I2C host adapter*/
	adapter = i2c_get_adapter(LSM303DL_I2C_BUS_NUMBER);
	if (NULL == adapter) {
		lsm303dl_log("Cannot get I2C host adapter\n");
		ret = -ENOTSUPP;
		goto handle_error;
	}
	
	/*Setup board information for I2C client*/
	memset(&i2c_info, 0, sizeof(struct i2c_board_info));
	i2c_info.addr = LSM303DL_I2C_ADDR_MAG;
	strlcpy(i2c_info.type, LSM303DL_MAG_NAME , I2C_NAME_SIZE); 
	
	#ifdef LSM303DL_MAG_INT
		i2c_info.irq = irqpin2irq(MAG_IRQ_NUMBER);
		i2c_info.flags = IORESOURCE_IRQ | IRQ_TYPE_EDGE_FALLING;	
	#endif	


	/*Initialize new i2c client device*/
	mag_client = i2c_new_device(adapter, &i2c_info);
	if (NULL == mag_client) {
		lsm303dl_log("Cannot initialize new i2c client device\n");
		ret = -ENOTSUPP;
		goto handle_error;
	}
	
	/*Put I2C adapter back to I2C core*/
	i2c_put_adapter(adapter);
	return 0;
	
handle_error: 
	/*Remove Magnetometer driver from I2C core*/
	i2c_del_driver(&mag_driver);
	
handle_gpio: 
#ifdef LSM303DL_MAG_INT
	gpio_free(GPIO_PORT109);
#endif

handle_power:
#ifdef KOTA_ENV
	pmic_set_power_off(E_POWER_VANA_MM);
#endif
	return ret;
}
 
/*************************************************************************
 *	name	=	lsm303dl_mag_exit
 *	func	=	Finalize I2C slave device of magnetometer
 *	input	=	None
 *	output	=	None
 *	return	=	None
 *************************************************************************/
static void __exit lsm303dl_mag_exit(void)
{
	/*Remove Magnetometer driver from I2C core*/
	i2c_del_driver(&mag_driver);
	
	/*Delete Magnetometer client from I2C core*/
	if (mag_client != NULL) {
		i2c_unregister_device(mag_client);
	}

#ifdef LSM303DL_MAG_INT
	gpio_free(GPIO_PORT109);
#endif
	
#ifdef KOTA_ENV
	/*Request PMIC not to supply power for Magnetometer device*/
	pmic_set_power_off(E_POWER_VANA_MM);
#endif
}
 
module_init(lsm303dl_mag_init);
module_exit(lsm303dl_mag_exit);

MODULE_DESCRIPTION("LSM303DL Magnetometer Driver");
MODULE_AUTHOR("Renesas");
MODULE_LICENSE("GPL v2");

