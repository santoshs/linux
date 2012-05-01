/* drivers/sensor/accelerometer/lsm303dl/lsm303dl_input.c
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
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/wakelock.h>
#include <linux/delay.h>
#include <linux/workqueue.h> 
#include <linux/hrtimer.h> 
#include <linux/device.h> 
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/lsm303dl.h>
#include "lsm303dl_local.h"

/************************************************
*   	Global variable definition section	 	*
************************************************/ 
static struct lsm303dl_input *lsm303dl_info = NULL;
static struct platform_device *pdev = NULL;

/*************************************************************************
 *	name	=	lsm303dl_misc_open
 *	func	=	Open lsm303dl input device node
 *	input	=	struct inode *ip, struct file *fp
 *	output	=	None
 *	return	=	0, -EBUSY
 *************************************************************************/
static int lsm303dl_misc_open(struct inode *ip, struct file *fp)
{
	return 0;
}

/*************************************************************************
 *	name	=	lsm303dl_misc_release
 *	func	=	Release lsm303dl input device node
 *	input	=	struct inode *ip, struct file *fp
 *	output	=	None
 *	return	=	0
 *************************************************************************/
static int lsm303dl_misc_release(struct inode *ip, struct file *fp)
{
	return 0;
}

/*************************************************************************
 *	name	=	lsm303dl_misc_ioctl
 *	func	=	Control lsm303dl input device node
 *	input	=	struct file *filp, unsigned int cmd, unsigned long arg
 *	output	=	None
 *	return	=	0, -ENOTTY, -EFAULT, -EINVAL, -EIO
 *************************************************************************/
static long lsm303dl_misc_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int 			ret 			= 0;
	unsigned int 	usr_val 		= 0;
	int 			activate_acc 	= 0;
	int 			activate_mag 	= 0;
	int 			acc_delay		= 0;
	int 			mag_delay		= 0;

	void __user 	*argp	 		= (void __user *)arg;
	void 			*ptr			= NULL;			
	
	int				nv_val[MAX_SETTING_LSM303DL]	=	{0};
	
	mutex_lock(&lsm303dl_info->lock);
	wake_lock(&lsm303dl_info->wakelock);
	
	/*Get value from user*/
	ret = copy_from_user(&usr_val, argp, sizeof(usr_val));
	if (ret != 0) {
		lsm303dl_log("Invalid user address!\n");
		wake_unlock(&lsm303dl_info->wakelock);
		mutex_unlock(&lsm303dl_info->lock);
		return -EFAULT;
	}
	
	switch (cmd) {
	
	/*Activate/Deactivate Accelerometer*/
	case IOCTL_ACC_ENABLE:
		#ifdef LSM303DL_ACC_INT1
			lsm303dl_log("Interrupt mechanism is used in accelerometer\n");
		#else
			lsm303dl_log("Polling mechanism is used in accelerometer\n");
		#endif
		
		activate_acc = atomic_read(&lsm303dl_info->acc_enable);
		
		/*Activate Accelerometer*/
		if (ACC_ENABLE == usr_val) {
			
			/*Accelerometer has already activated*/
			if (ACC_ENABLE == activate_acc) {
				ret = 0;
				break;
			}
			
			/*User delay time is less than or equal to maximum polling threshold*/
			if (lsm303dl_info->delay <= LSM303DL_POLL_THR) {
				/*Activate accelerometer*/
				ret = lsm303dl_acc_activate(ACC_ENABLE);
				if (ret < 0) {
					lsm303dl_log("Cannot activate accelerometer\n");
					ret = -EIO;
					break;
				}
			}
			
			/*Polling mechanism is used in Accelerometer or Magnetometer*/
			#if ((!defined LSM303DL_ACC_INT1) || (!defined LSM303DL_MAG_INT))
				activate_mag = atomic_read(&lsm303dl_info->mag_enable);
				
				/*Magnetometer is not activated*/
				if (MAG_DISABLE == activate_mag) {
					hrtimer_start(	&lsm303dl_info->timer, 
									ktime_set(0, lsm303dl_info->poll_interval * NSEC_PER_MSEC), 
									HRTIMER_MODE_REL
								);
				}
			#endif
		} else if (ACC_DISABLE == usr_val) {	/*Deactivate Accelerometer*/
			
			/*Accelerometer has already deactivated*/
			if (ACC_DISABLE == activate_acc) {
				ret = 0;
				break;
			}
			
			/*Deactivate accelerometer*/
			ret = lsm303dl_acc_activate(ACC_DISABLE);
			if (ret < 0) {
				lsm303dl_log("Cannot deactivate accelerometer\n");
				ret = -EIO;
				break;
			}
			
			/*Polling mechanism is used in Accelerometer or Magnetometer*/
			#if ((!defined LSM303DL_ACC_INT1) || (!defined LSM303DL_MAG_INT))
				activate_mag = atomic_read(&lsm303dl_info->mag_enable);
				
				/*Magnetometer is not activated*/
				if (MAG_DISABLE == activate_mag) {
					hrtimer_cancel(&lsm303dl_info->timer);
				}
			#endif
		} else { /*Input value from user is invalid*/
			lsm303dl_log("Invaild input value from user\n");
			ret =  -EINVAL;
			break;
		}
		
		/*Store acceleromter status*/
		atomic_set(&lsm303dl_info->acc_enable, usr_val);
		break;
	
	/*Activate/Deactivate Magnetometer*/
	case IOCTL_MAG_ENABLE:
	
		#ifdef LSM303DL_MAG_INT
			lsm303dl_log("Interrupt mechanism is used in Magnetometer\n");
		#else
			lsm303dl_log("Polling mechanism is used in Magnetometer\n");
		#endif
		
		activate_mag = atomic_read(&lsm303dl_info->mag_enable);
		
		/*Activate Magnetometer*/
		if (MAG_ENABLE == usr_val) {
			
			/*Magnetometer has already activated*/
			if (MAG_ENABLE == activate_mag) {
				ret = 0;
				break;
			}
			
			/*User delay time is less than or equal to maximum polling threshold*/
			if (lsm303dl_info->delay <= LSM303DL_POLL_THR) {
				/*Activate Magnetometer*/
				ret = lsm303dl_mag_activate(MAG_ENABLE);
				if (ret < 0) {
					lsm303dl_log("Cannot activate Magnetometer\n");
					ret = -EIO;
					break;
				}
			}
			
			/*Polling mechanism is used in Accelerometer or Magnetometer*/
			#if ((!defined LSM303DL_ACC_INT1) || (!defined LSM303DL_MAG_INT))
				activate_acc = atomic_read(&lsm303dl_info->acc_enable);
				
				/*Accelerometer is not activated*/
				if (ACC_DISABLE == activate_acc) {
					hrtimer_start(	&lsm303dl_info->timer, 
									ktime_set(0, lsm303dl_info->poll_interval * NSEC_PER_MSEC), 
									HRTIMER_MODE_REL
								);
				}
			#endif
		} else if (MAG_DISABLE == usr_val) {	/*Deactivate Magnetometer*/
			
			/*Magnetometer has already deactivated*/
			if (MAG_DISABLE == activate_mag) {
				ret = 0;
				break;
			}
			
			/*Deactivate Magnetometer*/
			ret = lsm303dl_mag_activate(MAG_DISABLE);
			if (ret < 0) {
				lsm303dl_log("Cannot deactivate Magnetometer\n");
				ret = -EIO;
				break;
			}
			
			/*Polling mechanism is used in Accelerometer or Magnetometer*/
			#if ((!defined LSM303DL_ACC_INT1) || (!defined LSM303DL_MAG_INT))
				activate_acc = atomic_read(&lsm303dl_info->acc_enable);
				
				/*Accelerometer is not activated*/
				if (ACC_DISABLE == activate_acc) {
					hrtimer_cancel(&lsm303dl_info->timer);
				}
			#endif
		} else { /*Input value from user is invalid*/
			lsm303dl_log("Invalid input value from user\n");
			ret =  -EINVAL;
			break;
		}
		
		/*Store Magnetometer status*/
		atomic_set(&lsm303dl_info->mag_enable, usr_val);
		break;
	
	/*Set delay time for Accelerometer and Magnetometer*/
	case IOCTL_SET_DELAY:
		acc_delay = usr_val;
		mag_delay = usr_val;
		
		/*Polling mechanism is used in Accelerometer or Magnetometer*/
		#if ((!defined LSM303DL_ACC_INT1) || (!defined LSM303DL_MAG_INT))
			
			/*Input value is greater than maximum polling threshold*/
			if(usr_val > LSM303DL_POLL_THR) {
				lsm303dl_info->poll_interval = usr_val - LSM303DL_ACTIVATE_TIME;
			
				/*Polling mechanism is used in Accelerometer*/
				#ifndef LSM303DL_ACC_INT1
					acc_delay = ACC_POLLING_INTERVAL_MIN;
				#endif

				/*Polling mechanism is used in Magnetometer*/
				#ifndef LSM303DL_MAG_INT
					mag_delay = MAG_POLLING_INTERVAL_MIN;
				#endif
			} else if (usr_val < LSM303DL_POLL_MIN) {
				lsm303dl_info->poll_interval = LSM303DL_POLL_MIN;
			} else {
				lsm303dl_info->poll_interval = usr_val;
			}
		#endif
		
		/*Write output data rate to accelerometer register*/
		ret = lsm303dl_acc_set_odr(acc_delay);
		if (ret < 0) {
			lsm303dl_log("Cannot set odr for accelerometer\n");
			ret = -EIO;
			break;
		}
		
		/*Write output data rate to magnetometer register*/
		ret = lsm303dl_mag_set_odr(mag_delay);
		if (ret < 0) {
			lsm303dl_log("Cannot set odr for magnetometer\n");
			ret = -EIO;
			break;
		}
		
		/*Polling mechanism is used in Accelerometer or Magnetometer*/
		#if ((!defined LSM303DL_ACC_INT1) || (!defined LSM303DL_MAG_INT))
			activate_acc = atomic_read(&lsm303dl_info->acc_enable);
			activate_mag = atomic_read(&lsm303dl_info->mag_enable);
	
			/*Accelerometer or Magnetometer is activated before*/
			if ((ACC_ENABLE == activate_acc) || (MAG_ENABLE == activate_mag)) { 
				hrtimer_cancel(&lsm303dl_info->timer);
				if (usr_val > LSM303DL_POLL_THR) {
					/*Accelerometer has already activated*/
					if (ACC_ENABLE == activate_acc) {
						/*Deactivate Accelerometer*/
						ret = lsm303dl_acc_activate(ACC_DISABLE);
						if (ret < 0) {
							lsm303dl_log("Cannot deactivate accelerometer\n");
							ret = -EIO;
							break;
						}
					}
					
					/*Magnetometer has already activated*/
					if (MAG_ENABLE == activate_mag) {
						/*Deactivate Magnetometer*/
						ret = lsm303dl_mag_activate(MAG_DISABLE);
						if (ret < 0) {
							lsm303dl_log("Cannot deactivate magnetometer\n");
							ret = -EIO;
							break;
						}
					}
					
				} else {
					/*Accelerometer has already activated*/
					if (ACC_ENABLE == activate_acc) {
						/*Activate Accelerometer*/
						ret = lsm303dl_acc_activate(ACC_ENABLE);
						if (ret < 0) {
							lsm303dl_log("Cannot activate accelerometer\n");
							ret = -EIO;
							break;
						}
					}
					
					/*Accelerometer has already activated*/
					if (MAG_ENABLE == activate_mag) {
						/*Activate Magnetometer*/
						ret = lsm303dl_mag_activate(MAG_ENABLE);
						if (ret < 0) {
							lsm303dl_log("Cannot activate magnetometer\n");
							ret = -EIO;
							break;
						}
					}
				}
				/*Start polling*/
				hrtimer_start(	&lsm303dl_info->timer, 
								ktime_set(0, lsm303dl_info->poll_interval * NSEC_PER_MSEC), 
								HRTIMER_MODE_REL
							);
			}
		lsm303dl_info->delay = usr_val;	
		#endif
		
		break;
	
	/*Configure Accelerometer and Magnetometer based on setting on NV memory*/
	case IOCTL_GET_NV_ADDRESS:
		
		/*Get NV setting from NV memory*/
		ptr = ioremap(usr_val, sizeof(nv_val));
		memcpy(&nv_val[0], ptr, sizeof(nv_val));
		iounmap(ptr);
		
		/*Write NV setting to accelerometer register*/
		ret = lsm303dl_acc_hw_init((u8 *)&nv_val[ACC_HPF]);
		if (ret < 0) {
			lsm303dl_log("Cannot initialize accelerometer based on NV settings\n");
			break;
		}
		
		/*Write NV setting to magnetometer register*/
		ret = lsm303dl_mag_hw_init((u8 *)&nv_val[MAG_SENS]);
		if (ret < 0) {
			lsm303dl_log("Cannot initialize magnetometer based on NV settings\n");
		}
		break;
		
	/*Unsupported command*/
	default:
		lsm303dl_log("Unknown command\n");
		ret = -ENOTTY;
		break;
	}
	
	wake_unlock(&lsm303dl_info->wakelock);
	mutex_unlock(&lsm303dl_info->lock);
	return ret;
}

/*************************************************************************
 *	name	=	lsm303dl_acc_report_values
 *	func	=	Read accelerometer values and report them to HAL
 *	input	=	None
 *	output	=	None
 *	return	=	None
 *************************************************************************/
void lsm303dl_acc_report_values(void)
{
	s16 	xyz[3] 	= {0};
	int 	ret 	= 0;
	
	/*Get value of x, y and z axis from accelerometer*/
	ret = lsm303dl_acc_get_data(xyz);
	if (0 == ret) {
		/*Report x, y, z value to HAL*/
		input_report_abs(lsm303dl_info->input_dev, EVENT_TYPE_ACCEL_X, xyz[0]);
		input_report_abs(lsm303dl_info->input_dev, EVENT_TYPE_ACCEL_Y, xyz[1]);
		input_report_abs(lsm303dl_info->input_dev, EVENT_TYPE_ACCEL_Z, xyz[2]);
		
//		lsm303dl_log("xyz[0] = %d\n", xyz[0]);
//		lsm303dl_log("xyz[1] = %d\n", xyz[1]);
//		lsm303dl_log("xyz[2] = %d\n", xyz[2]);
		
		/*Mark this event for accelerometer and inform
		 *to input core that value report is completed */
		lsm303dl_info->input_dev->sync = 0;
		input_event(lsm303dl_info->input_dev, EV_SYN, SYN_REPORT, LSM303DL_ACC_REPORT_ID);
	}
}
EXPORT_SYMBOL(lsm303dl_acc_report_values);

/*************************************************************************
 *	name	=	lsm303dl_mag_report_values
 *	func	=	Read Magnetometer values and report them to HAL
 *	input	=	None
 *	output	=	None
 *	return	=	None
 *************************************************************************/
void lsm303dl_mag_report_values(void)
{
	s16 	xyz[3] 	= {0};
	int 	ret 	= 0;
	
	/*Get value of x, y and z axis from magnetometer*/
	ret = lsm303dl_mag_get_data(xyz);
	if (0 == ret) {
		/*Report x, y, z value to HAL*/
		input_report_abs(lsm303dl_info->input_dev, EVENT_TYPE_MAGV_X, xyz[0]);
		input_report_abs(lsm303dl_info->input_dev, EVENT_TYPE_MAGV_Y, xyz[1]);
		input_report_abs(lsm303dl_info->input_dev, EVENT_TYPE_MAGV_Z, xyz[2]);
		
//		lsm303dl_log("xyz[0] = %d\n", xyz[0]);
//		lsm303dl_log("xyz[1] = %d\n", xyz[1]);
//		lsm303dl_log("xyz[2] = %d\n", xyz[2]);
		
		/*Mark this event for magnetometer and inform
		 *to input core that value report is completed */
		lsm303dl_info->input_dev->sync = 0;
		input_event(lsm303dl_info->input_dev, EV_SYN, SYN_REPORT, LSM303DL_MAG_REPORT_ID);
	}
}
EXPORT_SYMBOL(lsm303dl_mag_report_values);

/*************************************************************************
 *	name	=	lsm303dl_get_sns_status
 *	func	=	Get sensor status corresponding to sensor ID
 *	input	=	u8 sns_id
 *	output	=	None
 *	return	=	-EINVAL, DISABLE, ENABLE 
 *************************************************************************/
int lsm303dl_get_sns_status(u8 sns_id)
{
	if ((sns_id != ID_ACC) && (sns_id !=ID_MAG)) {
		return -EINVAL;
	}
	
	/*Sensor ID is accelerometer*/
	if (ID_ACC == sns_id) {
		return atomic_read(&lsm303dl_info->acc_enable);
	} else { /*Sensor ID is magnetometer*/
		return atomic_read(&lsm303dl_info->mag_enable);
	}
}
EXPORT_SYMBOL(lsm303dl_get_sns_status);


/*Polling mechanism is used in Accelerometer or Magnetometer*/
#if ((!defined LSM303DL_ACC_INT1) || (!defined LSM303DL_MAG_INT))
/*************************************************************************
 *	name	=	lsm303dl_timer_func
 *	func	=	Handle timer interrupt for lsm303dl input device
 *	input	=	struct hrtimer *timer
 *	output	=	None
 *	return	=	HRTIMER_NORESTART
 *************************************************************************/
static enum hrtimer_restart lsm303dl_timer_func (struct hrtimer *timer)
{
	int activate_acc = 0;
	int activate_mag = 0;
	
	/*Get the value of lsm303dl_info->acc_enable*/
	activate_acc = atomic_read(&lsm303dl_info->acc_enable);
	
	/*Get the value of lsm303dl_info->mag_enable*/
	activate_mag = atomic_read(&lsm303dl_info->mag_enable);
	
	if ((ACC_ENABLE == activate_acc) || (MAG_ENABLE == activate_mag)) { 	
		queue_work(lsm303dl_info->poll_workqueue, &lsm303dl_info->work_func);
	}
	
	return HRTIMER_NORESTART;
}

/*************************************************************************
 *	name	=	lsm303dl_work_func
 *	func	=	Read accelerometer and magnetometer values and report them to HAL periodically
 *	input	=	struct work_struct *work
 *	output	=	None
 *	return	=	None
 *************************************************************************/
static void lsm303dl_work_func(struct work_struct *work)
{
	int activate_acc = 0;
	int activate_mag = 0;
	
	mutex_lock(&lsm303dl_info->lock);
	wake_lock(&lsm303dl_info->wakelock);
	
	/*Get the value of lsm303dl_info->acc_enable*/
	activate_acc = atomic_read(&lsm303dl_info->acc_enable);
	
	/*Get the value of lsm303dl_info->mag_enable*/
	activate_mag = atomic_read(&lsm303dl_info->mag_enable);
	
	if (lsm303dl_info->delay > LSM303DL_POLL_THR) {
		/*Polling mechanism is used in Accelerometer*/
		#ifndef LSM303DL_ACC_INT1
			if (ACC_ENABLE == activate_acc) {
				/*Activate accelerometer*/
				lsm303dl_acc_activate(ACC_ENABLE);
			}
		#endif
		
		/*Polling mechanism is used in Magnetometer*/
		#ifndef LSM303DL_MAG_INT
			if (MAG_ENABLE == activate_mag) {
				/*Activate magnetometer*/
				lsm303dl_mag_activate(MAG_ENABLE);
			}
		#endif
		
		msleep(LSM303DL_ACTIVATE_TIME);
	}
	
	/*Polling mechanism is used in Accelerometer*/
	#ifndef LSM303DL_ACC_INT1
		if (ACC_ENABLE == activate_acc) {
			/*Read accelerometer values and report them to HAL*/
			lsm303dl_acc_report_values();
		}
	#endif
	
	/*Polling mechanism is used in Magnetometer*/
	#ifndef LSM303DL_MAG_INT
		if (MAG_ENABLE == activate_mag) {
			/*Read magnetometer values and report them to HAL*/
			lsm303dl_mag_report_values();
		}
	#endif
	
	hrtimer_start(	&lsm303dl_info->timer, 
					ktime_set(0, lsm303dl_info->poll_interval * NSEC_PER_MSEC), 
					HRTIMER_MODE_REL
				);
	
	if (lsm303dl_info->delay > LSM303DL_POLL_THR) {
		/*Polling mechanism is used in Accelerometer*/
		#ifndef LSM303DL_ACC_INT1
			if (ACC_ENABLE == activate_acc) {
				/*Activate accelerometer*/
				lsm303dl_acc_activate(ACC_DISABLE);
			}
		#endif
		
		/*Polling mechanism is used in Magnetometer*/
		#ifndef LSM303DL_MAG_INT
			if (MAG_ENABLE == activate_mag) {
				/*Activate magnetometer*/
				lsm303dl_mag_activate(MAG_DISABLE);
			}
		#endif
	}
	
	wake_unlock(&lsm303dl_info->wakelock);
	mutex_unlock(&lsm303dl_info->lock);
}
#endif

/*************************************************************************
 *	name	=	lsm303dl_platform_suspend
 *	func	=	Cancel polling mechanism and suspend lsm303dl input device
 *	input	=	struct device *dev
 *	output	=	None
 *	return	=	0
 *************************************************************************/
static int lsm303dl_platform_suspend(struct device *dev)
{
	int activate_acc = 0;
	int activate_mag = 0;
	
	mutex_lock(&lsm303dl_info->lock);
	
	/*Get the value of lsm303dl_info->acc_enable*/
	activate_acc = atomic_read(&lsm303dl_info->acc_enable);
	
	/*Get the value of lsm303dl_info->mag_enable*/
	activate_mag = atomic_read(&lsm303dl_info->mag_enable);

/*Polling mechanism is used in Accelerometer or Magnetometer*/
#if ((!defined LSM303DL_ACC_INT1) || (!defined LSM303DL_MAG_INT))
	/*Accelerometer or Magnetometer is activated before*/
	if ((ACC_ENABLE == activate_acc) || (MAG_ENABLE == activate_mag)) { 
		hrtimer_cancel(&lsm303dl_info->timer);
	}
#endif

	if (ACC_ENABLE == activate_acc) {
		/*De-activate accelerometer*/
		lsm303dl_acc_activate(ACC_DISABLE);
	}

	if (MAG_ENABLE == activate_mag) {
		/*De-activate magnetometer*/
		lsm303dl_mag_activate(MAG_DISABLE);
	}

	mutex_unlock(&lsm303dl_info->lock);
	return 0;
}

/*************************************************************************
 *	name	=	lsm303dl_platform_resume
 *	func	=	Restart polling mechanism and resume lsm303dl input device
 *	input	=	struct device *dev
 *	output	=	None
 *	return	=	0
 *************************************************************************/
static int lsm303dl_platform_resume(struct device *dev)
{
	int activate_acc = 0;
	int activate_mag = 0;
	
	mutex_lock(&lsm303dl_info->lock);
	
	/*Get the value of lsm303dl_info->acc_enable*/
	activate_acc = atomic_read(&lsm303dl_info->acc_enable);
	
	/*Get the value of lsm303dl_info->mag_enable*/
	activate_mag = atomic_read(&lsm303dl_info->mag_enable);

/*Polling mechanism is used in Accelerometer or Magnetometer*/
#if ((!defined LSM303DL_ACC_INT1) || (!defined LSM303DL_MAG_INT))
	/*Accelerometer or Magnetometer is activated before*/
	if ((ACC_ENABLE == activate_acc) || (MAG_ENABLE == activate_mag)) { 
		hrtimer_restart(&lsm303dl_info->timer);
	}
#endif

	if (ACC_ENABLE == activate_acc) {
		/*Activate accelerometer*/
		lsm303dl_acc_activate(ACC_ENABLE);
	}

	if (MAG_ENABLE == activate_mag) {
		/*Activate magnetometer*/
		lsm303dl_mag_activate(MAG_ENABLE);
	}

	mutex_unlock(&lsm303dl_info->lock);
	return 0;
}

/****************************************************
*   	File operation structure definition	     	*
*****************************************************/ 
static struct file_operations lsm303dl_fops = {
	.owner          = THIS_MODULE,
	.llseek			= no_llseek,
	.open           = lsm303dl_misc_open,
	.release        = lsm303dl_misc_release,
	.unlocked_ioctl = lsm303dl_misc_ioctl
};

/****************************************************
*   Miscellaneous device structure definition	 	*
*****************************************************/ 
static struct miscdevice lsm303dl_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = LSM303DL_DRIVER_NAME,
	.fops = &lsm303dl_fops
};

/*************************************************************************
 *	name	=	lsm303dl_platform_probe
 *	func	=	Probe lsm303dl input device
 *	input	=	struct device *dev
 *	output	=	None
 *	return	=	0, -ENOMEM, -EIO
 *************************************************************************/
static int lsm303dl_platform_probe(struct device *dev)
{
	int ret = 0;
	
	/*Allocate memory for lsm303dl_input structure*/
	lsm303dl_info = kzalloc(sizeof(struct lsm303dl_input), GFP_KERNEL);
	if (NULL == lsm303dl_info) {
		lsm303dl_log("Cannot allocate memory for lsm303dl_input structure\n");
		return -ENOMEM;
	}
	
	/*Initialize value for all members of lsm303dl_input  structure*/
	atomic_set(&lsm303dl_info->acc_enable, 0);
	atomic_set(&lsm303dl_info->mag_enable, 0);
	atomic_set(&lsm303dl_info->available, 0);
	lsm303dl_info->delay = LSM303DL_INTERVAL_DEFAUT;
	lsm303dl_info->poll_interval = LSM303DL_INTERVAL_DEFAUT;
	
	/*Initialize mutex and wake lock*/
	mutex_init(&lsm303dl_info->lock);
	wake_lock_init(&lsm303dl_info->wakelock,
			WAKE_LOCK_SUSPEND, "lsm303dl-wakelock");
			
	mutex_lock(&lsm303dl_info->lock);
	
/*Polling mechanism is used in Accelerometer or Magnetometer*/
#if ((!defined LSM303DL_ACC_INT1) || (!defined LSM303DL_MAG_INT))
	/*Initialize high resolution timer*/
	hrtimer_init(&lsm303dl_info->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	lsm303dl_info->timer.function = lsm303dl_timer_func;
	
	/*Create work queue structure for handling bottom half timer interrupt*/
	lsm303dl_info->poll_workqueue = create_singlethread_workqueue("lsm303dl_wq");
	if (NULL == lsm303dl_info->poll_workqueue) {
		lsm303dl_log("Cannot allocate memory for workqueue\n");
		ret = -ENOMEM;
		goto workqueue_err;
	}
	
	INIT_WORK(&lsm303dl_info->work_func, lsm303dl_work_func); 
#endif
	
	/*Allocate input device for reporting value to HAL layer*/
	lsm303dl_info->input_dev = input_allocate_device();
	if (NULL == lsm303dl_info->input_dev) {
		lsm303dl_log("Cannot allocate memory for input device\n");
		ret = -ENOMEM;	
		goto input_alloc_err;
	}	
	lsm303dl_info->input_dev->name = LSM303DL_DRIVER_NAME;
	set_bit(EV_ABS, lsm303dl_info->input_dev->evbit);
	
	/*Set parameters for 3-axis event type of accelerometer*/
	input_set_abs_params(lsm303dl_info->input_dev, EVENT_TYPE_ACCEL_X, -LSM303DL_MAX_RANGE_ACC, 
						LSM303DL_MAX_RANGE_ACC, LSM303DL_INPUT_FLAT, LSM303DL_INPUT_FUZZ);
						
	input_set_abs_params(lsm303dl_info->input_dev, EVENT_TYPE_ACCEL_Y, -LSM303DL_MAX_RANGE_ACC, 
						LSM303DL_MAX_RANGE_ACC, LSM303DL_INPUT_FLAT, LSM303DL_INPUT_FUZZ);
						
	input_set_abs_params(lsm303dl_info->input_dev, EVENT_TYPE_ACCEL_Z, -LSM303DL_MAX_RANGE_ACC, 
						LSM303DL_MAX_RANGE_ACC, LSM303DL_INPUT_FLAT, LSM303DL_INPUT_FUZZ);
						
	/*Set parameters for 3-axis event type of magnetometer*/
	input_set_abs_params(lsm303dl_info->input_dev, EVENT_TYPE_MAGV_X, -LSM303DL_MAX_RANGE_MAG, 
						LSM303DL_MAX_RANGE_MAG, LSM303DL_INPUT_FLAT, LSM303DL_INPUT_FUZZ);
						
	input_set_abs_params(lsm303dl_info->input_dev, EVENT_TYPE_MAGV_Y, -LSM303DL_MAX_RANGE_MAG, 
						LSM303DL_MAX_RANGE_MAG, LSM303DL_INPUT_FLAT, LSM303DL_INPUT_FUZZ);
						
	input_set_abs_params(lsm303dl_info->input_dev, EVENT_TYPE_MAGV_Z, -LSM303DL_MAX_RANGE_MAG, 
						LSM303DL_MAX_RANGE_MAG, LSM303DL_INPUT_FLAT, LSM303DL_INPUT_FUZZ);
	
	/*Register input device to input core*/
	ret = input_register_device(lsm303dl_info->input_dev);
	if (ret < 0) {
		lsm303dl_log("Cannot register input device\n");
		ret = -EIO;
		goto input_register_err;	
	}
	
	/*Register lsm303dl input driver as miscellaneous driver*/
	ret = misc_register(&lsm303dl_device);
	if (ret < 0) {
		lsm303dl_log("Cannot register misc device\n");
		ret = -EIO;		
		goto misc_register_err;	
	}
	
	mutex_unlock(&lsm303dl_info->lock);
	return 0;
	
/*Error handling*/
misc_register_err:
	input_unregister_device(lsm303dl_info->input_dev);
	
input_register_err:
	input_free_device(lsm303dl_info->input_dev);
	
input_alloc_err:
	/*Polling mechanism is used in Accelerometer or Magnetometer*/
	#if ((!defined LSM303DL_ACC_INT1) || (!defined LSM303DL_MAG_INT))
		destroy_workqueue(lsm303dl_info->poll_workqueue);	
workqueue_err:
	#endif	
	
	mutex_unlock(&lsm303dl_info->lock);
	wake_lock_destroy(&lsm303dl_info->wakelock);
	kfree(lsm303dl_info);
	lsm303dl_info = NULL;	
	return ret;
}

/*************************************************************************
 *	name	=	lsm303dl_platform_remove
 *	func	=	Remove lsm303dl input device
 *	input	=	struct device *dev
 *	output	=	None
 *	return	=	0
 *************************************************************************/
static int lsm303dl_platform_remove(struct device *dev)
{
	/*Unregister lsm303dl input driver as miscellaneous driver*/
	misc_deregister(&lsm303dl_device);
	
	/*Unregister lsm303dl input device from input core*/
	input_unregister_device(lsm303dl_info->input_dev);
	
	/*Release input device structure*/
	input_free_device(lsm303dl_info->input_dev);
	
	/*Polling mechanism is used in Accelerometer or Magnetometer*/
	#if ((!defined LSM303DL_ACC_INT1) || (!defined LSM303DL_MAG_INT))
		destroy_workqueue(lsm303dl_info->poll_workqueue);	
	#endif	
	
	/*Destroy wakelock*/
	wake_lock_destroy(&lsm303dl_info->wakelock);
	
	/*Release memory for lsm303dl_input structure*/
	kfree(lsm303dl_info);
	lsm303dl_info = NULL;
	
	return 0;
}

/****************************************************
*   	Power management structure definition	 	*
*****************************************************/ 
static struct dev_pm_ops lsm303dl_pm_ops ={
	.suspend = lsm303dl_platform_suspend,
	.resume = lsm303dl_platform_resume,
};

/****************************************************
*   	Platform driver structure definition	 	*
*****************************************************/  
static struct platform_driver lsm303dl_pf_driver =
{
	.driver =
	{
		.owner 		= THIS_MODULE,
		.name 		= LSM303DL_DRIVER_NAME,
		.pm 		= &lsm303dl_pm_ops,
		.probe 		= lsm303dl_platform_probe,
		.remove 	= lsm303dl_platform_remove,
	},
};

/*************************************************************************
 *	name	=	lsm303dl_init
 *	func	=	Initialize lsm303dl input device
 *	input	=	None
 *	output	=	None
 *	return	=	0, -ENOTSUPP
 *************************************************************************/
static int __init lsm303dl_init(void)
{
	int ret = 0;
	
	/*Register lsm303dl input driver to platform driver*/
	ret = platform_driver_register(&lsm303dl_pf_driver);
	if (ret < 0) {
		lsm303dl_log("Cannot register platform driver\n");
		return -ENOTSUPP;
	}
	
	/*Register lsm303dl input device to platform device*/
	pdev = platform_device_register_simple(LSM303DL_DRIVER_NAME, -1, NULL, 0);
	if (NULL == pdev) {
		lsm303dl_log("Cannot register platform device\n");
		/*Deregister lsm303dl input driver from platform driver*/
		platform_driver_unregister(&lsm303dl_pf_driver);
		return -ENOTSUPP;
	}
	
	return 0;
}
 
/*************************************************************************
 *	name	=	lsm303dl_exit
 *	func	=	Finalize lsm303dl input device
 *	input	=	None
 *	output	=	None
 *	return	=	None
 *************************************************************************/
static void __exit lsm303dl_exit(void)
{
	/*Unregister lsm303dl input device from platform device*/
	platform_device_unregister(pdev);
	
	/*Deregister lsm303dl input driver from platform driver*/
	platform_driver_unregister(&lsm303dl_pf_driver);
}
 
module_init(lsm303dl_init);
module_exit(lsm303dl_exit);

MODULE_DESCRIPTION("LSM303DL Input Driver");
MODULE_AUTHOR("Renesas");
MODULE_LICENSE("GPL v2");
 
