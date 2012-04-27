/* drivers/sensor/accelerometer/lsm303dl/lsm303dl_i2c_acc.c
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
#include <mach/r8a73734.h>
#include <mach/irqs.h>
#include <linux/lsm303dl.h>
#include "lsm303dl_local.h"

#ifdef KOTA_ENV
	#include <linux/pmic/pmic.h>
#endif

/************************************************
*   	Global variable definition section	 	*
************************************************/ 
static struct lsm303dl_acc_data *lsm303dl_acc_info = NULL;
static struct i2c_client *acc_client = NULL;

/*Output data rate looked-up table*/
static const struct lsm303dl_output_rate odr_table_acc[] = {
	{	1,		ACC_ODR_5376		},
	{	3,		ACC_ODR_400			},
	{	5,		ACC_ODR_200			},
	{	10,		ACC_ODR_100			},
	{	20,		ACC_ODR_50			},
	{	40,		ACC_ODR_25			},
	{	100,	ACC_ODR_10			},
	{	1000,	ACC_ODR_1			},
};

/*Sensitivity unit looked-up table*/
static const struct lsm303dl_sens_table sens_table_acc[] = {
	{	1,			1,		1	},	
	{	2,			2,		2	},		
	{	4,			4,		4	},		
	{	12,			12,		12	}	
};

/*Threshold LSB value*/
static const u8 thres_lsb_val[] = 
{ 
	16, 
	31, 
	63, 
	125 
};

/*Default setting for accelerometer*/
static const u8 lsm303dl_acc_setting_default[MAX_SETTING_ACC] = 
{
	HPF_RESET_READ_FILTER,		/*High pass filter*/
	ACC_SENSITIVITY_EXTREME,	/*Sensitivity*/
	ACC_ENABLE					/*High resolution output*/
};

/*************************************************************************
 *	name	=	lsm303dl_acc_i2c_read
 *	func	=	Read data from specified register of accelerometer
 *	input	=	u8 reg, u8 *val, int len
 *	output	=	u8 *val
 *	return	=	0, -EIO, -EINVAL
 *************************************************************************/
static int lsm303dl_acc_i2c_read(u8 reg, u8 *val, int len)
{
	int 			ret 		= 0;
	int 			tries 		= 0;
	u8				reg_addr 	= 0;
	
	struct i2c_msg 	msg[2];
	
	/*Check input value*/
	if ((NULL == val) || (NULL == acc_client)) {
		lsm303dl_log("Read buffer is NULL\n");
		return -EINVAL;
	}
	
	if (len > 1) {
		reg_addr = reg | ACC_I2C_AUTO_INCREMENT;
	} else {
		reg_addr = reg;
	}
	
	/*Initialization for all elements of i2c_msg structure*/
	memset(msg, 0, sizeof(msg));
	
	msg[0].addr = acc_client->addr;
	msg[0].flags = 0;
	msg[0].len = 1;
	msg[0].buf = (u8 *)&reg_addr;
	
	msg[1].addr = acc_client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = len;
	msg[1].buf = val;

	do {
		ret = i2c_transfer(acc_client->adapter, msg, 2);
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
 *	name	=	lsm303dl_acc_i2c_write
 *	func	=	Write data to specified register of accelerometer
 *	input	=	u8 reg, u8 *val, int len
 *	output	=	None
 *	return	=	0, -EIO, -EINVAL
 *************************************************************************/
static int lsm303dl_acc_i2c_write(u8 reg, u8 *val, int len)
{
	int 			ret = 0;
	int 			tries = 0;
	struct i2c_msg 	msg;
	u8 				*data; 
	
	/*Check input value*/
	if ((NULL == val) || (NULL == acc_client)) {
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
	
	if (len > 1) {
		data[0] = reg | ACC_I2C_AUTO_INCREMENT;
	} else {
		data[0] = reg;
	}
	memcpy((void *)&data[1], (void *)val, len);
	
	msg.addr = acc_client->addr;
	msg.flags = 0;
	msg.len = len + 1;
	msg.buf = data;

	/*Write data to a specific register address of accelerometer hardware via I2C bus*/
	do {
		ret = i2c_transfer(acc_client->adapter, &msg, 1);
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
 *	name	=	lsm303dl_acc_set_hpf
 *	func	=	Change Accelerometer high pass filter status
 *	input	=	u8 val
 *	output	=	None
 *	return	=	0, -EIO, -EINVAL
 *************************************************************************/
static int lsm303dl_acc_set_hpf(u8 val)
{
	int 	ret 				= 0;
	u8 		reg_value			= 0;
	u8		current_setting 	= 0;
	
	if ((val < HPF_RESET_READ_FILTER) || (val > HPF_AUTORESET_ON_INTERRUPT)) {
		lsm303dl_log("Invalid input argument\n");
		return -EINVAL;
	}
	
	/*Read the content of CTRL_REG2_A register*/
	ret = lsm303dl_acc_i2c_read(CTRL_REG2_A, &reg_value, 1);
	if (ret < 0) {
		lsm303dl_log("Fail to read data from Accelerometer\n");
		return ret;
	}
	
	current_setting = (reg_value >> 6) & 0x03;
	if (val == current_setting) {
		return 0;
	}
	
	reg_value = (reg_value & 0x3F) | ((val << 6) & 0xC0);
	
	/*Write new value to CTRL_REG2_A register*/
	ret = lsm303dl_acc_i2c_write(CTRL_REG2_A, &reg_value, 1);
	if (ret < 0) {
		lsm303dl_log("Fail to write data to Accelerometer\n");
		return ret;
	}
	
	return 0;
}

/*************************************************************************
 *	name	=	lsm303dl_acc_set_high_res
 *	func	=	Enable/Disable Accelerometer high resolution output
 *	input	=	u8 val
 *	output	=	None
 *	return	=	0, -EIO, -EINVAL
 *************************************************************************/
static int lsm303dl_acc_set_high_res(u8 val)
{
	int 	ret 				= 0;
	u8 		reg_value			= 0;
	u8		current_setting 	= 0;
	
	if ((val != ACC_ENABLE) && (val != ACC_DISABLE)) {
		lsm303dl_log("Invalid input argument (val = %d)\n", val);
		return -EINVAL;
	}
	
	/*Read the content of CTRL_REG4_A register*/
	ret = lsm303dl_acc_i2c_read(CTRL_REG4_A, &reg_value, 1);
	if (ret < 0) {
		lsm303dl_log("Fail to read data from Accelerometer\n");
		return ret;
	}
	
	current_setting = (reg_value >> 3) & 0x01;
	if (val == current_setting) {
		return 0;
	}
	
	reg_value = (reg_value & 0xF7) | ((val << 3) & 0x08);
	
	/*Write new value to CTRL_REG4_A register*/
	ret = lsm303dl_acc_i2c_write(CTRL_REG4_A, &reg_value, 1);
	if (ret < 0) {
		lsm303dl_log("Fail to write data to Accelerometer\n");
		return ret;
	}
	
	return 0;
}

/*************************************************************************
 *	name	=	lsm303dl_acc_set_sensitivity
 *	func	=	Change accelerometer sensitivity
 *	input	=	u8 val
 *	output	=	None
 *	return	=	0, -EIO, -EINVAL
 *************************************************************************/
static int lsm303dl_acc_set_sensitivity(u8 val)
{
	int 	ret 				= 0;
	u8 		reg_value			= 0;
	u8		current_setting 	= 0;
	
	if ((val < ACC_SENSITIVITY_EXTREME) || (val > ACC_SENSITIVITY_LOW)) {
		lsm303dl_log("Invalid input argument\n");
		return -EINVAL;
	}
	
	/*Read the content of CTRL_REG4_A register*/
	ret = lsm303dl_acc_i2c_read(CTRL_REG4_A, &reg_value, 1);
	if (ret < 0) {
		lsm303dl_log("Fail to read data from Accelerometer\n");
		return ret;
	}
	
	current_setting = (reg_value >> 4) & 0x03;
	if (val == current_setting) {
		lsm303dl_acc_info->sensitivity = val;
		return 0;
	}
	
	reg_value = (reg_value & 0xCF) | ((val << 4) & 0x30);
	
	/*Write new value to CTRL_REG4_A register*/
	ret = lsm303dl_acc_i2c_write(CTRL_REG4_A, &reg_value, 1);
	if (ret < 0) {
		lsm303dl_log("Fail to write data to Accelerometer\n");
		return ret;
	}
	
	/*Save sensitivity level to global variable for further usage*/
	lsm303dl_acc_info->sensitivity = val;
	
	#ifdef LSM303DL_ACC_INT1
		/*INT1_THS_A*/
		reg_value = thres_lsb_val[val];
		ret = lsm303dl_acc_i2c_write(INT1_THS_A, &reg_value, 1);
		if (ret < 0) {
			lsm303dl_log("Cannot configure for INT1_THS_A register\n");
			return ret;
		}
	#endif
	
	return 0;
}

/*************************************************************************
 *	name	=	lsm303dl_acc_power_status
 *	func	=	Power up/down accelerometer
 *	input	=	u8 val
 *	output	=	None
 *	return	=	0, -EIO, -EINVAL
 *************************************************************************/
static int lsm303dl_acc_power_status(u8 val)
{
	int 	ret 				= 0;
	u8 		reg_value			= 0;
	u8		current_setting 	= 0;
	u8		current_odr		 	= 0;
	
	if ((val != ACC_NORMAL) && (val != ACC_STANDBY)) {
		lsm303dl_log("Invalid input argument\n");
		return -EINVAL;
	}
	
	/*Read the content of CTRL_REG1_A register*/
	ret = lsm303dl_acc_i2c_read(CTRL_REG1_A, &reg_value, 1);
	if (ret < 0) {
		lsm303dl_log("Fail to read data from Accelerometer\n");
		return ret;
	}
	
	current_setting = (reg_value >> 3) & 0x01;
	if (val == current_setting) {
		return 0;
	}
	
	if (ACC_STANDBY == val) {
		/*Get odr from "read buffer"*/
		current_odr = (reg_value >> 4) & 0x0F;
		reg_value = 0x08;
	} else {
		reg_value = 0x07 | (lsm303dl_acc_info->odr << 4);
	}
	
	/*Write new value to CTRL_REG1_A register*/
	ret = lsm303dl_acc_i2c_write(CTRL_REG1_A, &reg_value, 1);
	if (ret < 0) {
		lsm303dl_log("Fail to write data to Accelerometer\n");
		return ret;
	}
	
	if (ACC_STANDBY == val) {
		lsm303dl_acc_info->odr = current_odr;
	}
	
	return 0;
}

/*************************************************************************
 *	name	=	lsm303dl_acc_set_odr
 *	func	=	Change accelerometer output data rate
 *	input	=	u32 val
 *	output	=	None
 *	return	=	0, -EIO
 *************************************************************************/
int lsm303dl_acc_set_odr(u32 val)
{
	int 	i 					= 0;
	int 	ret					= 0;
	u8 		activation_flg		= 0;
	u8 		reg_value			= 0;
	u8		odr					= 0;
	
	if (NULL == lsm303dl_acc_info)
	{
		lsm303dl_log("Invalid input argument\n");
		return -EINVAL;
	}
	
	/*Interrupt mechanism is used*/
	#ifdef LSM303DL_ACC_INT1
	
		/*Get status of Accelerometer*/
		activation_flg = lsm303dl_get_sns_status(ID_ACC);
		if (ACC_ENABLE == activation_flg) {
		
			/*Previous time is greater than polling threshold*/
			if (lsm303dl_acc_info->delay > LSM303DL_POLL_THR) {
				hrtimer_cancel(&lsm303dl_acc_info->timer);
			}
			
			/*Input value is greater than polling threshold*/
			if (val > LSM303DL_POLL_THR) {
			
				/*Power down Accelerometer*/
				ret = lsm303dl_acc_power_status (ACC_STANDBY);
				if (ret < 0) {
					lsm303dl_log("Fail to power down Accelerometer\n");
					return -EIO;
				}
				
				/*Calculate delay time and store to global variable*/
				lsm303dl_acc_info->poll_interval = val - LSM303DL_ACTIVATE_TIME;
				
				/*Update input delay time to global variable*/
				lsm303dl_acc_info->delay = val;
				
				/*Update minimum output data rate to global variable*/
				lsm303dl_acc_info->odr = ACC_ODR_5376;
				
				hrtimer_start(	&lsm303dl_acc_info->timer, 
								ktime_set(0, lsm303dl_acc_info->poll_interval * NSEC_PER_MSEC), 
								HRTIMER_MODE_REL
							);
				
				return 0;
			}
		}
	#endif
	
	/*Find the output data rate in look-up table*/
	for (i = ARRAY_SIZE(odr_table_acc) -1; i > 0; i-- ) {
		if (odr_table_acc[i].poll_rate_ms <= val) {
			break;
		}
	}
	
	odr = odr_table_acc[i].mask;
	
	if (val > LSM303DL_POLL_THR) {
		odr = ACC_ODR_5376;
	}
	
	/*Read the content of CTRL_REG1_A register*/
	ret = lsm303dl_acc_i2c_read(CTRL_REG1_A, &reg_value, 1);
	if (ret < 0) {
		lsm303dl_log("Fail to read data from Accelerometer\n");
		return -EIO;
	}

	reg_value = (reg_value & 0x0F) | ((odr << 4) & 0xF0);
	
	/*Write new value to CTRL_REG1_A register*/
	ret = lsm303dl_acc_i2c_write(CTRL_REG1_A, &reg_value, 1);
	if (ret < 0) {
		lsm303dl_log("Fail to write data to Accelerometer\n");
		return -EIO;
	}
	
	/*Update output data rate to global variable*/
	lsm303dl_acc_info->odr = odr;
	
	/*Interrupt mechanism is used*/
	#ifdef LSM303DL_ACC_INT1
		if (ACC_ENABLE == activation_flg) {
			if (lsm303dl_acc_info->delay > LSM303DL_POLL_THR) {
				if (val <= LSM303DL_POLL_THR) {
					
					/*Power on Accelerometer*/
					ret = lsm303dl_acc_power_status (ACC_NORMAL);
					if (ret < 0) {
						lsm303dl_log("Fail to power on Accelerometer\n");
						return -EIO;
					}
				}
			}
		}
	#endif
	
	/*Update input delay time to global variable*/
	lsm303dl_acc_info->delay = val;
	
	if ((lsm303dl_acc_info->delay > 100) && (lsm303dl_acc_info->delay < 1000)) {
		lsm303dl_acc_info->report_ignore_cnt = val/100;
	} else {
		lsm303dl_acc_info->report_ignore_cnt = 1;
	}
	
	return 0;
}
EXPORT_SYMBOL(lsm303dl_acc_set_odr);

/*************************************************************************
 *	name	=	lsm303dl_acc_activate
 *	func	=	Activate/Deactivate Accelerometer
 *	input	=	u8 val
 *	output	=	None
 *	return	=	0, -EIO, -EINVAL
 *************************************************************************/
int lsm303dl_acc_activate(u8 val)
{
	int 	ret 			= 0;
	
	if ((val != ACC_ENABLE) && (val != ACC_DISABLE) ) {
		return -EINVAL;
	}
	
	if (NULL == lsm303dl_acc_info) {
		lsm303dl_log("Invalid input argument\n");
		return -EINVAL;
	}
	
	if (ACC_ENABLE == val) {
		
		/*Interrupt mechanism is used*/
		#ifdef LSM303DL_ACC_INT1
		
			/*Enable interrupt processing*/
			enable_irq(lsm303dl_acc_info->irq1);
			
			if (lsm303dl_acc_info->delay > LSM303DL_POLL_THR) {
				hrtimer_start(	&lsm303dl_acc_info->timer, 
								ktime_set(0, lsm303dl_acc_info->poll_interval * NSEC_PER_MSEC), 
								HRTIMER_MODE_REL
							);
				return 0;
			}
		#endif
		
		/*Power on Accelerometer*/
		ret = lsm303dl_acc_power_status (ACC_NORMAL);
		
		return ret;
	} else {
		/*Interrupt mechanism is used*/
		#ifdef LSM303DL_ACC_INT1
		
			/*Disable interrupt processing*/
			disable_irq(lsm303dl_acc_info->irq1);
			
			if (lsm303dl_acc_info->delay > LSM303DL_POLL_THR) {
				hrtimer_cancel(&lsm303dl_acc_info->timer);
				return 0;
			}
		#endif
		
		/*Power down Accelerometer*/
		ret = lsm303dl_acc_power_status (ACC_STANDBY);
		
		return ret;
	}
}
EXPORT_SYMBOL(lsm303dl_acc_activate);

/*************************************************************************
 *	name	=	lsm303dl_acc_hw_init
 *	func	=	Initialize hardware setting for accelerometer
 *	input	=	u8 *val
 *	output	=	None
 *	return	=	0, -EIO, -EINVAL
 *************************************************************************/
int lsm303dl_acc_hw_init(u8 *val)
{
	int ret = 0;
	
	if ((NULL == val) | (NULL == lsm303dl_acc_info)) {
		lsm303dl_log("Input value is NULL\n");
		return -EINVAL;
	}
	
	/*Set high pass filter*/
	ret = lsm303dl_acc_set_hpf(val[0]);
	if (ret < 0) {
		lsm303dl_log("Fail to set high pass filter for Accelerometer\n");
		return ret;
	}
	
	/*Set sensitivity*/
	ret = lsm303dl_acc_set_sensitivity(val[1]);
	if (ret < 0) {
		lsm303dl_log("Fail to set sensitivity for Accelerometer\n");
		return ret;
	}
	
	/*Set high resolution output mode*/
	ret = lsm303dl_acc_set_high_res(val[2]);
	if (ret < 0) {
		lsm303dl_log("Fail to set high resolution output mode for Accelerometer\n");
		return ret;
	}
	
	return 0;
}
EXPORT_SYMBOL(lsm303dl_acc_hw_init);

/*************************************************************************
 *	name	=	lsm303dl_acc_get_data
 *	func	=	Get accelerometer values from accelerometer device
 *	input	=	None
 *	output	=	s16 *data
 *	return	=	0, -EIO, -EINVAL
 *************************************************************************/
int lsm303dl_acc_get_data(s16 *data)
{
	int 	ret 			= 0;
	u8 		reg_value[6] 	= { 0 };
	s32		hw_data[3]		= { 0 };
	u8		idx				=	0;
	
	if (NULL == lsm303dl_acc_info) {
		lsm303dl_log("Invalid input argument\n");
		return -EINVAL;
	}
	
	idx = lsm303dl_acc_info->sensitivity;
	
	wake_lock(&lsm303dl_acc_info->wakelock);
	
	if (NULL == data) {
		lsm303dl_log("Input value is NULL\n");
		ret = -EINVAL;
		goto err;
	}
	
	/*Get x, y and z axis value from Accelerometer*/
	ret = lsm303dl_acc_i2c_read(OUT_X_L_A, reg_value, 6);
	if (ret < 0) {
		lsm303dl_log("Fail to read data from Accelerometer\n");
		goto err;
	}
	
	/*Calculate the actual value for accelerometer*/
	hw_data[0] = (((s16) ((reg_value[1] << 8) | reg_value[0])) >> 4);
	hw_data[1] = (((s16) ((reg_value[3] << 8) | reg_value[2])) >> 4);
	hw_data[2] = (((s16) ((reg_value[5] << 8) | reg_value[4])) >> 4);
	
	/*Adjust x, y, z-axis value based on sensitivity*/
	data[0] = hw_data[0] * sens_table_acc[idx].x;
	data[1] = hw_data[1] * sens_table_acc[idx].y;
	data[2] = hw_data[2] * sens_table_acc[idx].z;
	
err:
	wake_unlock(&lsm303dl_acc_info->wakelock);
	return ret;
}
EXPORT_SYMBOL(lsm303dl_acc_get_data);

#ifdef LSM303DL_ACC_INT1
/*************************************************************************
 *	name	=	lsm303dl_acc_isr
 *	func	=	Handler for INT1 interrupt
 *	input	=	int irq, void *dev
 *	output	=	None
 *	return	=	IRQ_HANDLED
 *************************************************************************/
static irqreturn_t lsm303dl_acc_isr(int irq, void *dev)
{
	if (irq != -1) {
		disable_irq_nosync(irq);
		queue_work(lsm303dl_acc_info->irq1_workqueue, &lsm303dl_acc_info->irq1_work);
	}
	return IRQ_HANDLED;
}

/*************************************************************************
 *	name	=	lsm303dl_acc_irq_work_func
 *	func	=	Work function for getting accelerometer data and 
 *				reporting them to HAL when INT1 interrupt occurs
 *	input	=	struct work_struct *work
 *	output	=	None
 *	return	=	None
 *************************************************************************/
static void lsm303dl_acc_irq_work_func(struct work_struct *work)
{
	mutex_lock(&lsm303dl_acc_info->lock);
	
	lsm303dl_acc_info->report_ignore_cnt--;
	
	if (lsm303dl_acc_info->report_ignore_cnt < 1) {
		/*Get x, y, z axis value and report to HAL layer*/
		lsm303dl_acc_report_values();
		
		/*Calculate the number of ignored time in case of long polling interval*/
		if ((lsm303dl_acc_info->delay > 100) && (lsm303dl_acc_info->delay < 1000)) {
			lsm303dl_acc_info->report_ignore_cnt = lsm303dl_acc_info->delay/100;
		} else{
			lsm303dl_acc_info->report_ignore_cnt = 1;
		}
	}
	
	if (lsm303dl_acc_info->delay > LSM303DL_POLL_THR) {
		lsm303dl_acc_power_status(ACC_STANDBY);
		hrtimer_start(	&lsm303dl_acc_info->timer, 
						ktime_set(0, lsm303dl_acc_info->poll_interval * NSEC_PER_MSEC), 
						HRTIMER_MODE_REL
					);
	}
	
	/*Enable interrupt processing*/
	enable_irq(lsm303dl_acc_info->irq1);
	mutex_unlock(&lsm303dl_acc_info->lock);
}

/*************************************************************************
 *	name	=	lsm303dl_acc_timer
 *	func	=	Handle timer interrupt for accelerometer
 *	input	=	struct hrtimer *timer
 *	output	=	None
 *	return	=	HRTIMER_NORESTART
 *************************************************************************/
static enum hrtimer_restart lsm303dl_acc_timer (struct hrtimer *timer)
{
	queue_work(lsm303dl_acc_info->irq1_workqueue, &lsm303dl_acc_info->irq1_work);
	return HRTIMER_NORESTART;
}

/*************************************************************************
 *	name	=	lsm303dl_acc_timer_work_func
 *	func	=	Work function for power Accelerometer device when timer 
 *				interrupt occurs
 *	input	=	struct work_struct *work
 *	output	=	None
 *	return	=	None
 *************************************************************************/
static void lsm303dl_acc_timer_work_func(struct work_struct *work)
{
	mutex_lock(&lsm303dl_acc_info->lock);
	
	if (lsm303dl_acc_info->delay > LSM303DL_POLL_THR) {
		lsm303dl_acc_power_status(ACC_NORMAL);
	}
	mutex_unlock(&lsm303dl_acc_info->lock);
}
#endif

/*************************************************************************
 *	name	=	lsm303dl_acc_suspend
 *	func	=	Suspend Accelerometer device
 *	input	=	struct device *dev
 *	output	=	None
 *	return	=	0
 *************************************************************************/
static int lsm303dl_acc_suspend(struct device *dev)
{
	return 0;
}

/*************************************************************************
 *	name	=	lsm303dl_acc_resume
 *	func	=	Resume Accelerometer device
 *	input	=	struct device *dev
 *	output	=	None
 *	return	=	0
 *************************************************************************/
static int lsm303dl_acc_resume(struct device *dev)
{
	int activation_flg = 0;
	
	lsm303dl_log("lsm303dl_acc_resume is called\n");
	mutex_lock(&lsm303dl_acc_info->lock);
	
	/*Get status of Accelerometer*/
	//activation_flg = lsm303dl_get_sns_status(ID_ACC);
	if (ACC_ENABLE == activation_flg) {
		lsm303dl_acc_activate(ACC_ENABLE);
	}
	
	mutex_unlock(&lsm303dl_acc_info->lock);
	return 0;
}

/*************************************************************************
 *	name	=	lsm303dl_acc_i2c_probe
 *	func	=	Probe I2C slave device of accelerometer
 *	input	=	struct i2c_client *client, const struct i2c_device_id *devid
 *	output	=	None
 *	return	=	0
 *************************************************************************/
static int lsm303dl_acc_i2c_probe(struct i2c_client *client, const struct i2c_device_id *devid)
{
	int 	ret = 0;
	u8 		reg_default[5] = {0};
	
	lsm303dl_log("lsm303dl_acc_i2c_probe is called\n");
	
	/*Check functionalities of I2C adapter*/
	ret = i2c_check_functionality(client->adapter, I2C_FUNC_I2C);
	if (ret == 0) {
		lsm303dl_log("Accelerometer I2C is malfunction\n");
		return -EIO;
	}
	
	/*Allocate memory for lsm303dl_acc_data structure*/
	lsm303dl_acc_info = kzalloc(sizeof(struct lsm303dl_acc_data), GFP_KERNEL);
	if (NULL == lsm303dl_acc_info) {
		lsm303dl_log("Cannot allocate memmory for lsm303dl_acc_data structure\n");
		return -ENOMEM;
	}
	
	/*Initialize values for lsm303dl_acc_data structure*/
	lsm303dl_acc_info->sensitivity 			= 0;
	lsm303dl_acc_info->odr					= 2;
	lsm303dl_acc_info->delay 				= 100;
	lsm303dl_acc_info->poll_interval 		= 100;
	lsm303dl_acc_info->report_ignore_cnt 	= 0;
	
	acc_client = client;

	/*Initialize mutex and wake lock*/
	mutex_init(&lsm303dl_acc_info->lock);
	wake_lock_init(&lsm303dl_acc_info->wakelock,
			WAKE_LOCK_SUSPEND, "lsm303dl-acc-wakelock");
			
	mutex_lock(&lsm303dl_acc_info->lock);
	
	/*Initialize Accelerometer fixed setting*/
	/*CTRL_REG1_A: ODR: 100 ms, low power and disable xyz axis*/
	reg_default[0] = 0x28;
	ret = lsm303dl_acc_i2c_write(CTRL_REG1_A, reg_default, 1);
	if (ret < 0) {
		lsm303dl_log("Cannot configure for CTRL_REG1_A\n");
		ret = -EIO;
		goto hw_init_err;
	}
	
	/*FIFO_CTRL_REG_A*/
	reg_default[0] = 0x00;
	ret = lsm303dl_acc_i2c_write(FIFO_CTRL_REG_A, reg_default, 1);
	if (ret < 0) {
		lsm303dl_log("Cannot configure for FIFO_CTRL_REG_A\n");
		ret = -EIO;
		goto hw_init_err;
	}

	#ifdef LSM303DL_ACC_INT1	
		reg_default[0] = 0x00; /*CTRL_REG2_A*/
		reg_default[1] = 0x10; /*CTRL_REG3_A*/
		reg_default[2] = 0x80; /*CTRL_REG4_A*/
		reg_default[3] = 0x00; /*CTRL_REG5_A*/
		reg_default[4] = 0x00; /*CTRL_REG6_A*/
		
		ret = lsm303dl_acc_i2c_write(CTRL_REG2_A, reg_default, 5);
		if (ret < 0) {
			lsm303dl_log("Cannot configure for CTRL_REG2_A->CTRL_REG6_A register\n");
			ret = -EIO;
			goto hw_init_err;
		}
		
		reg_default[0] = 0xBF; /*INT1_CFG_A*/
		ret = lsm303dl_acc_i2c_write(INT1_CFG_A, reg_default, 1);
		if (ret < 0) {
			lsm303dl_log("Cannot configure for INT1_CFG_A register\n");
			ret = -EIO;
			goto hw_init_err;
		}
		
		reg_default[0] = thres_lsb_val[lsm303dl_acc_info->sensitivity]; /*INT1_THS_A*/
		reg_default[1] = 0x01; /*INT1_DURATION_A*/
		ret = lsm303dl_acc_i2c_write(INT1_THS_A, reg_default, 2);
		if (ret < 0) {
			lsm303dl_log("Cannot configure for INT1_THS_A and INT1_DURATION_A register\n");
			ret = -EIO;
			goto hw_init_err;
		}
	#endif
	
	/*Initialize Accelerometer default setting*/
	ret = lsm303dl_acc_hw_init((u8 *)&lsm303dl_acc_setting_default[0]);
	if (ret < 0) {
		lsm303dl_log("Cannot initialize Accelerometer default setting\n");
		ret = -EIO;
		goto hw_init_err;
	}
	
	#ifdef LSM303DL_ACC_INT1	
		/*Initialize high resolution timer*/
		hrtimer_init(&lsm303dl_acc_info->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
		lsm303dl_acc_info->timer.function = lsm303dl_acc_timer;
		
		/*Create work queue structure for handling bottom-half interrupt*/
		lsm303dl_acc_info->irq1_workqueue = create_singlethread_workqueue("lsm303dl_acc_wq");
		if (NULL == lsm303dl_acc_info->irq1_workqueue) {
			lsm303dl_log("Cannot create work queue structure for Accelerometer\n");
			ret = -ENOMEM;
			goto hw_init_err;
		}
		
		INIT_WORK(&lsm303dl_acc_info->irq1_work, lsm303dl_acc_irq_work_func); 
		INIT_WORK(&lsm303dl_acc_info->timer_work, lsm303dl_acc_timer_work_func);		
		
		/*Register Accelerometer interrupt handler*/
		ret = request_irq(client->irq, lsm303dl_acc_isr, 0, "lsm303dl_acc_int", lsm303dl_acc_info);
		if (ret < 0) {
			lsm303dl_log("Cannot register Accelerometer interrupt handler\n");
			ret = -EIO;
			goto req_irq_err;
		}
		lsm303dl_acc_info->irq1 = client->irq;
		disable_irq(lsm303dl_acc_info->irq1);
	#endif
	
	/*Change to standby mode*/
	lsm303dl_acc_power_status(ACC_STANDBY);
	mutex_unlock(&lsm303dl_acc_info->lock);
	
	return 0;

#ifdef LSM303DL_ACC_INT1
req_irq_err:	
	destroy_workqueue(lsm303dl_acc_info->irq1_workqueue);	
#endif	
	
hw_init_err:
	mutex_unlock(&lsm303dl_acc_info->lock);
	wake_lock_destroy(&lsm303dl_acc_info->wakelock);
	
	kfree(lsm303dl_acc_info);
	lsm303dl_acc_info = NULL;
	
	return ret;
}

/*************************************************************************
 *	name	=	lsm303dl_acc_i2c_remove
 *	func	=	Remove I2C slave device of accelerometer
 *	input	=	struct i2c_client *client
 *	output	=	None
 *	return	=	0
 *************************************************************************/
static int lsm303dl_acc_i2c_remove(struct i2c_client *client)
{
	#ifdef LSM303DL_ACC_INT1
		destroy_workqueue(lsm303dl_acc_info->irq1_workqueue);
		free_irq(lsm303dl_acc_info->irq1, lsm303dl_acc_info);
	#endif	
	
	wake_lock_destroy(&lsm303dl_acc_info->wakelock);
	kfree(lsm303dl_acc_info);
	lsm303dl_acc_info = NULL;
	return 0;
}

/****************************************************
*   	I2C device id structure definition	 			*
*****************************************************/
static const struct i2c_device_id lsm303dl_acc_id[] = {
	{ 	LSM303DL_ACC_NAME, 		0 			},
	{ 										}
};

/****************************************************
*   	Power management structure definition	 	*
*****************************************************/ 
static struct dev_pm_ops lsm303dl_acc_pm_ops ={
	.suspend = lsm303dl_acc_suspend,
	.resume = lsm303dl_acc_resume,
};

/****************************************************
*   	I2C driver structure definition	 			*
*****************************************************/  
static struct i2c_driver acc_driver = {
	.probe     		= lsm303dl_acc_i2c_probe,
	.remove			= lsm303dl_acc_i2c_remove,
	.id_table  		= lsm303dl_acc_id,
	.driver			= 
	{
		.owner 		= THIS_MODULE,
		.name		= LSM303DL_ACC_NAME,
		.pm 		= &lsm303dl_acc_pm_ops,
	},
};

/*************************************************************************
 *	name	=	lsm303dl_acc_init
 *	func	=	Initialize I2C slave device of accelerometer
 *	input	=	None
 *	output	=	None
 *	return	=	0, -ENOTSUPP
 *************************************************************************/
static int __init lsm303dl_acc_init(void)
{
	int 					ret 		= 0;
	struct i2c_board_info 	i2c_info;
	
	struct i2c_adapter 		*adapter 	= NULL;
	
	lsm303dl_log("lsm303dl_acc_init is called\n");
	
#ifdef KOTA_ENV
	/*Request PMIC to supply power for Accelerometer device*/
	ret = pmic_set_power_on(E_POWER_VANA_MM);
	if (ret < 0) {
		lsm303dl_log("Cannot request PMIC to supply power for Accelerometer device\n");
		return -ENOTSUPP;
	}
#endif
	
	#ifdef LSM303DL_ACC_INT1
		/*  Use the GPIO_PORT110 */
		ret = gpio_request(GPIO_PORT110, NULL);         
		if (ret < 0) {
			lsm303dl_log("Cannot request GPIO_PORT110 for accelerometer driver\n");
			ret = -ENOTSUPP;
			goto handle_power;
		}
		
		/* Set direction for GPIO_PORT110 */
		ret = gpio_direction_input(GPIO_PORT110);
		if (ret < 0) {
			lsm303dl_log("Cannot set direction of GPIO_PORT110 for accelerometer driver\n");
			ret = -ENOTSUPP;
			goto handle_gpio;
		}
	#endif
	
	/*Register accelerometer driver to I2C core*/
	ret = i2c_add_driver(&acc_driver);
	if (ret < 0) {
		lsm303dl_log("Cannot register accelerometer driver to I2C core\n");
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
	i2c_info.addr = LSM303DL_I2C_ADDR_ACC;
	strlcpy(i2c_info.type, LSM303DL_ACC_NAME , I2C_NAME_SIZE); 
	
	#ifdef LSM303DL_ACC_INT1
		i2c_info.irq = irqpin2irq(ACC_IRQ_NUMBER);
		i2c_info.flags = IORESOURCE_IRQ | IRQ_TYPE_EDGE_FALLING;
	#endif	

	/*Initialize new i2c client device*/
	acc_client = i2c_new_device(adapter, &i2c_info);
	if (NULL == acc_client) {
		lsm303dl_log("Cannot initialize new i2c client device\n");
		ret = -ENOTSUPP;
		goto handle_error;
	}
	
	/*Put I2C adapter back to I2C core*/
	i2c_put_adapter(adapter);
	return 0;
	

handle_error: 	
	/*Remove accelerometer driver from I2C core*/
	i2c_del_driver(&acc_driver);
	
handle_gpio: 
#ifdef LSM303DL_ACC_INT1
	gpio_free(GPIO_PORT110);
#endif

handle_power:

#ifdef KOTA_ENV
	pmic_set_power_off(E_POWER_VANA_MM);
#endif	
	return ret;
}

/*************************************************************************
 *	name	=	lsm303dl_acc_exit
 *	func	=	Finalize I2C slave device of accelerometer
 *	input	=	None
 *	output	=	None
 *	return	=	None
 *************************************************************************/
static void __exit lsm303dl_acc_exit(void)
{
	/*Remove accelerometer driver from I2C core*/
	i2c_del_driver(&acc_driver);
	
	/*Delete accelerometer client from I2C core*/
	if (acc_client != NULL) {
		i2c_unregister_device(acc_client);
	}

#ifdef LSM303DL_ACC_INT1
	gpio_free(GPIO_PORT109);
#endif

#ifdef KOTA_ENV
	/*Request PMIC not to supply power for Accelerometer device*/
	pmic_set_power_off(E_POWER_VANA_MM);
#endif
}
 
module_init(lsm303dl_acc_init);
module_exit(lsm303dl_acc_exit);

MODULE_DESCRIPTION("LSM303DL Accelerometer Driver");
MODULE_AUTHOR("Renesas");
MODULE_LICENSE("GPL v2");

