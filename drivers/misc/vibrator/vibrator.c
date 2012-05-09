/*
 * /drivers/misc/vibrator/vibrator.c
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
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/hrtimer.h>
#include <mach/r8a73734.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/tpu_pwm.h>
#include <linux/pcm2pwm.h>
#include "../../../drivers/staging/android/timed_output.h"

/************************  MACRO   ****************************************/
#define TPU_CHANNEL				"TPU0TO0"
#define CLK_SOURCE_CP			0 /* CP clock Common Peripheral*/
#define VIB_OVERDRV_TIME		40	/* Over-drive duration of DC motor (ms) */
#define VIB_DUTY_OVERDRV_ON		0	/* Duty cycle when turning on DC motor */
#define VIB_DUTY_VIBRATE		0x0100		/* Duty cycle when vibrating */
#define VIB_CYCLE				0x0400		/* Value of TGRB to set the cycle */

#define SAMPLE_NUM				133
#define VIB_CNT_LINEAR			1024
#define VIB_CNT_OFF				1600
#define VIB_CNT_75				4
#define VIB_CNT_ON				1
#define USER_TURN_OFF			0
#define USER_TURN_ON			1
#define HRTIMER_REQUEST			2
#define HIGH					1
#define LOW						0

/*********************** FUNCTION PROTOTYPE   ********************************/
#if !defined(VIB_PCM2PWM) /* Default is using TPU */
/* Functions to control TPU */
extern int tpu_pwm_open(const char *channel, int prescaler, void **handle);
extern int tpu_pwm_close(void *handle);
extern int tpu_pwm_enable(void *handle, enum tpu_pwm_state state,
							int duty, int cycle);
#else
/* Functions to control PCM2PWM */
extern int pcm2pwm_open(void);
extern int pcm2pwm_close(void);
extern int pcm2pwm_enable(enum pcm2pwm_request_state state, const void *src,
							unsigned int data_sz, u16 cnt);
#endif /* !defined(VIB_PCM2PWM */


static int __init init_vibrator(void);
extern void vibrator_enable(struct timed_output_dev *sdev, int timeout);
static int vib_get_time(struct timed_output_dev *sdev);
static enum hrtimer_restart vib_timer_function(struct hrtimer *timer);

#if !defined(VIB_DCMOTOR) /* In case use Linear Motor */
static void vibrator_work_turn_off(struct work_struct *work);
static void vibrator_work_turn_on(struct work_struct *work);
#else /* In case use DC Motor */
static void vibrator_work_func(struct work_struct *work);
#endif /* !defined(VIB_DCMOTOR) */

struct vibrator_single_request {
	struct list_head node;
	struct work_struct work;
	int object_request;
	int request_state;
	int timeout;
};

enum vibration_state {
	NO_VIBRATE,	/* Status when there is no operation of DC motor */
	TURN_ON,	/* Status when DC motor begins the operation */
	VIBRATE,	/* Status when DC motor is in stable operation */
	TURN_OFF	/* Status when DC motor finishes the operation */
};

/*********************** GLOBAL VARIABLE  ************************************/
static struct hrtimer vib_timer;
static struct timed_output_dev vib_device; /* Vibrator device object */
static int vib_state = NO_VIBRATE;
static struct workqueue_struct *vibrator_single_workqueue;

#if defined(VIB_DCMOTOR) /* In case use DC Motor */
static spinlock_t vibrator_spinlock;
static int vib_duration; /* Vibrate duration in millisecond */
static LIST_HEAD(vibrator_list_head);
/* DC Motor*/
static const unsigned short const_pcm[SAMPLE_NUM] = {3,	3,	3,	3,	3,
								3,	3,	3,	3,	3,	3,	3,	3,	3,	3,
								3,	3,	3,	3,	3,	3,	3,	3,	3,	3,
								3,	3,	3,	3,	3,	3,	3,	3,	3,	3,
								3,	3,	3,	3,	3,	3,	3,	3,	3,	3,
								3,	3,	3,	3,	3,	3,	3,	3,	3,	3,
								3,	3,	3,	3,	3,	3,	3,	3,	3,	3,
								3,	3,	3,	3,	3,	3,	3,	3,	3,	3,
								3,	3,	3,	3,	3,	3,	3,	3,	3,	3,
								3,	3,	3,	3,	3,	3,	3,	3,	3,	3,
								3,	3,	3,	3,	3,	3,	3,	3,	3,	3,
								3,	3,	3,	3,	3,	3,	3,	3,	3,	3,
								3,	3,	3,	3,	3,	3,	3,	3,	3,	3,
								3,	3,	3,	3,	3,	3,	3,  3};
#else /* Linear Motor */
struct work_struct vibrator_work_on_pcm2pwm;
struct work_struct vibrator_work_off_pcm2pwm;
/* sine wave PCM data (Linear Motor) */
static const unsigned short wave_pcm[SAMPLE_NUM] = {0x0200, 0x020C, 0x0218,
					0x0223, 0x022F, 0x023B, 0x0247, 0x0252, 0x025E, 0x0269,
					0x0273, 0x027E, 0x0288, 0x0292, 0x029C, 0x02A5, 0x02AE,
					0x02B7, 0x02BF, 0x02C7, 0x02CE, 0x02D5, 0x02DB, 0x02E1,
					0x02E7, 0x02EC, 0x02F0, 0x02F4, 0x02F7, 0x02FA, 0x02FC,
					0x02FE, 0x02FF, 0x02FF, 0x02FF, 0x02FF, 0x02FE, 0x02FC,
					0x02FA, 0x02F7, 0x02F3, 0x02EF, 0x02EB, 0x02E6, 0x02E1,
					0x02DB, 0x02D4, 0x02CD, 0x02C6, 0x02BE, 0x02B6, 0x02AD,
					0x02A4, 0x029B, 0x0291, 0x0287, 0x027C, 0x0272, 0x0267,
					0x025C, 0x0250, 0x0245, 0x0239, 0x022D, 0x0222, 0x0216,
					0x020A, 0x01FE, 0x01F2, 0x01E6, 0x01DA, 0x01CE, 0x01C2,
					0x01B6, 0x01AB, 0x01A0, 0x0195, 0x018A, 0x017F, 0x0175,
					0x016B, 0x0161, 0x0158, 0x014F, 0x0147, 0x013F, 0x0137,
					0x0130, 0x0129, 0x0123, 0x011D, 0x0117, 0x0113, 0x010E,
					0x010B, 0x0107, 0x0105, 0x0103, 0x0101, 0x0100, 0x0100,
					0x0100, 0x0100, 0x0102, 0x0103, 0x0106, 0x0109, 0x010C,
					0x0110, 0x0115, 0x011A, 0x011F, 0x0125, 0x012C, 0x0133,
					0x013A, 0x0142, 0x014B, 0x0153, 0x015D, 0x0166, 0x0170,
					0x017A, 0x0184, 0x018F, 0x019A, 0x01A5, 0x01B0, 0x01BC,
					0x01C8, 0x01D3, 0x01DF, 0x01EB};
#endif /* defined(VIB_DCMOTOR) */


/*********************** FUNCTION IMPLEMENT   ********************************/

/*
* vib_get_time: Get the current number of milliseconds remaining on the timer
* @sdev		: Pointer to structure timed_output_dev
* return	: the current number of milliseconds remaining on the timer
* Note		: This function use for TPU and PCM2PWM driver
*/
static int vib_get_time(struct timed_output_dev *sdev)
{
	if (hrtimer_active(&vib_timer)) {
		ktime_t r = hrtimer_get_remaining(&vib_timer);
		struct timeval t = ktime_to_timeval(r);
		return t.tv_sec * 1000 + t.tv_usec / 1000;
	}
	return 0;
}

#if defined(VIB_DCMOTOR)
/*=============================================================================
*							DC MOTOR
=============================================================================*/
/*
* vibrator_enable: Handle request Turn off vibration,
*					or turn on vibration for a specified duration
* @sdev		: Pointer to structure timed_output_dev
* @timeout	: The duration of vibration
* return	: the current number of milliseconds remaining on the timer
*/
extern void vibrator_enable(struct timed_output_dev *sdev, int timeout)
{
	struct vibrator_single_request *request;
	unsigned long flags;
	if (timeout < 0) {
		printk(KERN_ERR "[VIBRATOR ERR]  vibrator_enable():timeout value is negative = %d\n", timeout);
	}
	if (vib_state != TURN_OFF) {
		request = (struct vibrator_single_request *)kzalloc(sizeof(struct vibrator_single_request), GFP_KERNEL);
		if (NULL == request) {
			printk(KERN_ERR "[VIBRATOR ERR]  vibrator_enable():failed to allocate memory\n");
			return;
		}
		if (!timeout) {
			request->object_request = USER_TURN_OFF;
		} else if (timeout > 0) {
			request->object_request = USER_TURN_ON;
		}
		request->timeout = timeout;
		INIT_WORK(&request->work, vibrator_work_func);
		spin_lock_irqsave(&vibrator_spinlock, flags);
		list_add_tail(&request->node, &vibrator_list_head);
		queue_work(vibrator_single_workqueue, &request->work);
		spin_unlock_irqrestore(&vibrator_spinlock, flags);
	} else {
		/* Get the latest request during TURN_OFF state */
		spin_lock_irqsave(&vibrator_spinlock, flags);
		vib_duration = timeout;
		spin_unlock_irqrestore(&vibrator_spinlock, flags);
	}
}

/*
* vib_timer_function: Timer expiry callback function
* @timer	:
* return	:
*				HRTIMER_NORESTART	: Timer is not restarted (actual value: 0)
*				HRTIMER_RESTART		: Timer must be restarted (actual value: 1)
*/
static enum hrtimer_restart vib_timer_function(struct hrtimer *timer)
{
	struct vibrator_single_request *request;

	request = (struct vibrator_single_request *)kzalloc(sizeof(struct vibrator_single_request), GFP_KERNEL);
	if (request == NULL) {
		printk(KERN_ERR "[VIBRATOR ERR]  vib_timer_function():failed to allocate memory\n");
		return HRTIMER_NORESTART;
	}

	/* identify this request by HRTIMER */
	request->object_request = HRTIMER_REQUEST;

	if (TURN_ON == vib_state) {
		/* request change to VIBRATE state */
		request->request_state = VIBRATE;
		list_add_tail(&request->node, &vibrator_list_head);
		INIT_WORK(&request->work, vibrator_work_func);
		queue_work(vibrator_single_workqueue, &request->work);
	} else if (VIBRATE == vib_state) {
		/* request change to TURN_OFF state */
		request->request_state = TURN_OFF;
		list_add_tail(&request->node, &vibrator_list_head);
		INIT_WORK(&request->work, vibrator_work_func);
		queue_work(vibrator_single_workqueue, &request->work);
	} else if (TURN_OFF == vib_state) {
		/* request change to NO_VIBRATE state */
		request->request_state = NO_VIBRATE;
		list_add_tail(&request->node, &vibrator_list_head);
		INIT_WORK(&request->work, vibrator_work_func);
		queue_work(vibrator_single_workqueue, &request->work);
	} else {
		/* vib_state in another statenot in above state */
		printk(KERN_ERR "[VIBRATOR ERR]  vib_timer_function():wrong request: vib_state = %d\n", vib_state);
		kfree(request);
	}
	return HRTIMER_NORESTART;
}

#if !defined(VIB_PCM2PWM) /* Case using TPU */
/*
* vibrator_work_func: work function to turn on/off tpu
* @work		: pointer to work_struct object
* return	: None
* note		: This function use for TPU driver with DC Motor
*/
static void vibrator_work_func(struct work_struct *work)
{
	struct vibrator_single_request *request;
	static void *tpu_handle;
	int ret;

	if (list_empty(&vibrator_list_head)) {
		printk(KERN_ERR "[VIBRATOR ERR] vibrator_work_func: list is emtpy\n");
		return;
	}

	spin_lock(&vibrator_spinlock);
	/* get first node in list */
	request = list_first_entry(&vibrator_list_head,
								struct vibrator_single_request, node);
	list_del(&request->node);
	spin_unlock(&vibrator_spinlock);

	if (HRTIMER_REQUEST == request->object_request) {
		/* processing hrtimer request */
		if ((TURN_ON == vib_state) && (VIBRATE == request->request_state)) {
			/* set tpu to duty cycle is 75% */
			ret = tpu_pwm_enable(tpu_handle, TPU_PWM_START,
									VIB_DUTY_VIBRATE, VIB_CYCLE);
			if (ret) {
				printk(KERN_ERR "[VIBRATOR ERR] vibrator_work_func: call tpu_pwm_enable error = %d, state = %d\n", ret, request->request_state);
				tpu_pwm_close(tpu_handle);
				tpu_handle = NULL;
				kfree(request);
				return;
			}

			spin_lock(&vibrator_spinlock);
			vib_state = VIBRATE;

			/* vibrate in timeout */
			hrtimer_start(&vib_timer, ktime_set(vib_duration / 1000,
								(vib_duration % 1000) * 1000000),
								HRTIMER_MODE_REL);
			vib_duration = 0;
			spin_unlock(&vibrator_spinlock);
		} else if ((VIBRATE == vib_state)
					&& (TURN_OFF == request->request_state)) {
			/* turn off and close tpu */
			ret = tpu_pwm_enable(tpu_handle, TPU_PWM_STOP, 0, 0);
			if (ret) {
				printk(KERN_ERR "[VIBRATOR ERR] vibrator_work_func: call tpu_pwm_enable error = %d, state = %d\n", ret, request->request_state);
				tpu_pwm_close(tpu_handle);
				tpu_handle = NULL;
				kfree(request);
				return;
			}

			ret = tpu_pwm_close(tpu_handle);
			if (ret) {
				printk(KERN_ERR "[VIBRATOR ERR] vibrator_work_func: call tpu_pwm_close error = %d\n", ret);
				tpu_handle = NULL;
				kfree(request);
				return;
			}
			tpu_handle = NULL;
			spin_lock(&vibrator_spinlock);
			vib_state = TURN_OFF;
			spin_unlock(&vibrator_spinlock);
			/* start hrtimer for 40ms */
			hrtimer_start(&vib_timer, ktime_set(VIB_OVERDRV_TIME / 1000,
								(VIB_OVERDRV_TIME % 1000) * 1000000),
								HRTIMER_MODE_REL);

		} else if ((TURN_OFF == vib_state)
					&& (NO_VIBRATE == request->request_state)) {
			spin_lock(&vibrator_spinlock);
			if (vib_duration > 0) {
				struct vibrator_single_request *request_on;
				request_on = (struct vibrator_single_request *)kzalloc(sizeof(struct vibrator_single_request), GFP_KERNEL);
				if (NULL == request_on) {
					printk(KERN_ERR "[VIBRATOR ERR]  vibrator_work_func():failed to allocate memory\n");
					kfree(request);
					spin_unlock(&vibrator_spinlock);
					return;
				}
				request_on->object_request = USER_TURN_ON;
				request_on->timeout = vib_duration;
				list_add_tail(&request_on->node, &vibrator_list_head);
				/* send request to bottom-half */
				INIT_WORK(&request_on->work, vibrator_work_func);
				queue_work(vibrator_single_workqueue, &request_on->work);
			}
			vib_state = NO_VIBRATE;
			spin_unlock(&vibrator_spinlock);

			ret = gpio_direction_output(GPIO_PORT226, LOW);
			if (ret) {
				printk(KERN_ERR "[VIBRATOR ERR] vibrator_work_func(): gpio_direction_output is failed\n");
				return;
			}
		}
	} else if (USER_TURN_ON == request->object_request) {
		/* User request to turn ON*/
		if (NO_VIBRATE == vib_state) {
			hrtimer_cancel(&vib_timer);

			ret = tpu_pwm_open(TPU_CHANNEL, CLK_SOURCE_CP, &tpu_handle);
			if (ret) {
				printk("[VIBRATOR ERR] vibrator_work_func(): call tpu_pwm_open error = %d\n", ret);
				kfree(request);
				return;
			}

			ret = tpu_pwm_enable(tpu_handle, TPU_PWM_START,
									VIB_DUTY_OVERDRV_ON, VIB_CYCLE);
			if (ret) {
				printk("[VIBRATOR ERR] vibrator_work_func(): call tpu_pwm_enable error = %d, state = %d\n", ret, request->request_state);
				tpu_pwm_close(tpu_handle);
				kfree(request);
				tpu_handle = NULL;
				return;
			}

			ret = gpio_direction_output(GPIO_PORT226, HIGH);
			if (ret) {
				printk(KERN_ERR "[VIBRATOR ERR] vibrator_work_func(): gpio_direction_output is failed\n");
				return;
			}
			spin_lock(&vibrator_spinlock);
			vib_state = TURN_ON;
			vib_duration = request->timeout;
			spin_unlock(&vibrator_spinlock);
			hrtimer_start(&vib_timer, ktime_set(VIB_OVERDRV_TIME / 1000,
							(VIB_OVERDRV_TIME % 1000) * 1000000),
							HRTIMER_MODE_REL);
		}
	} else if (USER_TURN_OFF == request->object_request) {
		/* User request to turn off */
		spin_lock(&vibrator_spinlock);
		vib_duration = 0;
		spin_unlock(&vibrator_spinlock);
		if (TURN_ON == vib_state || VIBRATE == vib_state) {
			/* turn off and close tpu */
			hrtimer_cancel(&vib_timer);
			ret = tpu_pwm_enable(tpu_handle, TPU_PWM_STOP, 0, 0);
			if (ret) {
				tpu_pwm_close(tpu_handle);
				tpu_handle = NULL;
				kfree(request);
				return;
			}
			ret = tpu_pwm_close(tpu_handle);
			if (ret) {
				printk(KERN_ERR "[VIBRATOR ERR] vibrator_work_func(): call tpu_pwm_close error = %d\n", ret);
				tpu_handle = NULL;
				kfree(request);
				return;
			}
			tpu_handle = NULL;

			spin_lock(&vibrator_spinlock);
			vib_state = TURN_OFF;
			spin_unlock(&vibrator_spinlock);
			hrtimer_start(&vib_timer, ktime_set(VIB_OVERDRV_TIME / 1000,
							(VIB_OVERDRV_TIME % 1000) * 1000000),
							HRTIMER_MODE_REL);
		}
	}
	kfree(request);
}
#else /* use PCM2PWM  defined(VIB_PCM2PWM)*/
/*
* vibrator_work_func: work function to turn on/off pcm2pwm
* @work		: pointer to work_struct object
* return	: None
* note		: This function use for PCM2PWM driver with DC Motor
*/
static void vibrator_work_func(struct work_struct *work)
{
	struct vibrator_single_request *request;
	static  void *tpu_handle;
	int ret;

	if (list_empty(&vibrator_list_head)) {
		printk(KERN_ERR "[VIBRATOR ERR] vibrator_work_func: list is emtpy\n");
		return;
	}

	spin_lock(&vibrator_spinlock);
	/* get first node in list */
	request = list_first_entry(&vibrator_list_head,
								struct vibrator_single_request, node);

	list_del(&request->node);
	spin_unlock(&vibrator_spinlock);

	if (HRTIMER_REQUEST == request->object_request) {
		/* processing hrtimer request */
		if ((TURN_ON == vib_state) && (VIBRATE == request->request_state)) {
			/* set pcm2pwm to duty cycle is 75% */
			ret = pcm2pwm_enable(START_PCM2PWM, const_pcm, SAMPLE_NUM,
									VIB_CNT_75);
			if (ret) {
				printk(KERN_ERR "[VIBRATOR ERR] vibrator_work_func(): call tpu_pwm_enable error = %d, state = %d\n", ret, request->request_state);
				pcm2pwm_close();
				kfree(request);
				return;
			}
			spin_lock(&vibrator_spinlock);
			vib_state = VIBRATE;

			/* vibrate in timeout */
			hrtimer_start(&vib_timer, ktime_set(vib_duration / 1000,
								(vib_duration % 1000) * 1000000),
								HRTIMER_MODE_REL);
			vib_duration = 0;
			spin_unlock(&vibrator_spinlock);
		} else if ((VIBRATE == vib_state)
					&& (TURN_OFF == request->request_state)) {
			/* turn off and close pcm2pwm */
			ret = pcm2pwm_enable(STOP_PCM2PWM, const_pcm,
									SAMPLE_NUM, VIB_CNT_OFF);
			if (ret) {
				printk(KERN_ERR "[VIBRATOR ERR] vibrator_work_func(): call tpu_pwm_enable error = %d, state = %d\n", ret, request->request_state);
				pcm2pwm_close();
				kfree(request);
				return;
			}

			ret = pcm2pwm_close();
			if (ret) {
				printk(KERN_ERR "[VIBRATOR ERR] vibrator_work_func(): call pcm2pwm_close() error = %d\n", ret);
				kfree(request);
				return;
			}

			tpu_handle = NULL;
			spin_lock(&vibrator_spinlock);
			vib_state = TURN_OFF;
			spin_unlock(&vibrator_spinlock);
			/* start hrtimer for 40ms */
			hrtimer_start(&vib_timer, ktime_set(VIB_OVERDRV_TIME / 1000,
							(VIB_OVERDRV_TIME % 1000) * 1000000),
							HRTIMER_MODE_REL);

		} else if ((TURN_OFF == vib_state)
					&& (NO_VIBRATE == request->request_state)) {
			spin_lock(&vibrator_spinlock);
			if (vib_duration > 0) {
				struct vibrator_single_request *request_on;
				request_on = (struct vibrator_single_request *)kzalloc(sizeof(struct vibrator_single_request), GFP_KERNEL);
				if (NULL == request_on) {
					printk(KERN_ERR "[VIBRATOR ERR]  vibrator_work_func():failed to allocate memory\n");
					kfree(request);
					return;
				}
				request_on->object_request = USER_TURN_ON;
				request_on->timeout = vib_duration;
				list_add_tail(&request_on->node, &vibrator_list_head);
				/* send request to top-half */
				INIT_WORK(&request->work, vibrator_work_func);
				queue_work(vibrator_single_workqueue, &request->work);
			}

			vib_state = NO_VIBRATE;
			spin_unlock(&vibrator_spinlock);
			
			ret = gpio_direction_output(GPIO_PORT226, LOW);
			if (ret) {
				printk(KERN_ERR "[VIBRATOR ERR] vibrator_work_func(): gpio_direction_output is failed\n");
				return;
			}

		} else {
			printk(KERN_ERR "[VIBRATOR ERR] vibrator_work_func(): hrtimer request in error vib_state = %d\n", vib_state);
		}
	} else if (USER_TURN_ON == request->object_request) {
		/* User request to turn ON*/
		if (NO_VIBRATE == vib_state) {
			hrtimer_cancel(&vib_timer);

			ret = gpio_direction_output(GPIO_PORT226, HIGH);
			if (ret) {
				printk(KERN_ERR "[VIBRATOR ERR] vibrator_work_func(): gpio_direction_output is failed\n");
				return;
			}

			ret = pcm2pwm_open();
			if (ret) {
				printk(KERN_ERR "[VIBRATOR ERR] vibrator_work_func(): call tpu_pwm_open error = %d\n", ret);
				kfree(request);
				return;
			}

			ret = pcm2pwm_enable(START_PCM2PWM,
									const_pcm,
									SAMPLE_NUM,
									VIB_CNT_ON);

			if (ret) {
				printk(KERN_ERR "[VIBRATOR ERR] vibrator_work_func(): call tpu_pwm_enable error = %d, state = %d\n", ret, request->request_state);
				pcm2pwm_close();
				kfree(request);
				return;
			}

			spin_lock(&vibrator_spinlock);
			vib_state = TURN_ON;
			vib_duration = request->timeout;
			spin_unlock(&vibrator_spinlock);
			hrtimer_start(&vib_timer, ktime_set(VIB_OVERDRV_TIME / 1000,
							(VIB_OVERDRV_TIME % 1000) * 1000000),
							HRTIMER_MODE_REL);
		}
	} else if (USER_TURN_OFF == request->object_request) {
		/* user request to turn off */
		spin_lock(&vibrator_spinlock);
		vib_duration = 0;
		spin_unlock(&vibrator_spinlock);
		if ((TURN_ON == vib_state) || (VIBRATE == vib_state)) {
			/* Stop and close pcm2pwm */
			hrtimer_cancel(&vib_timer);
			ret = pcm2pwm_enable(STOP_PCM2PWM,
									const_pcm,
									SAMPLE_NUM,
									VIB_CNT_OFF);

			if (ret) {
				pcm2pwm_close();
				kfree(request);
				return;
			}

			ret = pcm2pwm_close();
			if (ret) {
				printk(KERN_ERR "[VIBRATOR ERR] vibrator_work_func(): call tpu_pwm_close error = %d\n", ret);
				kfree(request);
				return;
			}
			tpu_handle = NULL;

			spin_lock(&vibrator_spinlock);
			vib_state = TURN_OFF;
			spin_unlock(&vibrator_spinlock);

			hrtimer_start(&vib_timer,
							ktime_set(VIB_OVERDRV_TIME / 1000,
							(VIB_OVERDRV_TIME % 1000) * 1000000),
							HRTIMER_MODE_REL);
		}
	}
	kfree(request);
}
#endif

/*
* init_vibrator: init vibrator driver
* return:
			0: if no error
			other value if error
*/
static int __init init_vibrator(void)
{
	int ret = 0;
	/* Initializing vibrator device object */
	vib_device.name = "vibrator";
	vib_device.enable = vibrator_enable;
	vib_device.get_time = vib_get_time;
#if !defined(VIB_PCM2PWM) /* use TPU driver */
	ret = gpio_request(GPIO_PORT36, NULL);
	if (ret) {
		printk(KERN_ERR "[VIBRATOR ERR] init_vibrator(): request GPIO_PORT36 error\n");
		return ret;
	}

	ret = gpio_direction_output(GPIO_PORT36, LOW);
	if (ret) {
		printk(KERN_ERR "[VIBRATOR ERR] init_vibrator(): gpio_direction_output GPIO_PORT36 error\n");
		return ret;
	}
#else /* Use PCM2PWM driver */
	ret = gpio_request(GPIO_PORT228, NULL);
	if (ret) {
		printk(KERN_ERR "[VIBRATOR ERR] init_vibrator(): request GPIO_PORT228 error\n");
		return ret;
	}

	ret = gpio_direction_output(GPIO_PORT228, LOW);
	if (ret) {
		printk(KERN_ERR "[VIBRATOR ERR] init_vibrator(): gpio_direction_output GPIO_PORT228 error\n");
		return ret;
	}
#endif /* !defined(VIB_PCM2PWM) */

	ret = gpio_request(GPIO_PORT226, NULL);
	if (ret) {
		printk(KERN_ERR "[VIBRATOR ERR] init_vibrator(): request GPIO_PORT226 error\n");
		return ret;
	}

	ret = gpio_direction_output(GPIO_PORT226, LOW);
	if (ret) {
		printk(KERN_ERR "[VIBRATOR ERR] init_vibrator(): gpio_direction_output GPIO_PORT226 error\n");
		return ret;
	}

	/* Register vibrator to timed_output device */
	ret = timed_output_dev_register(&vib_device);
	if (ret) {
		printk(KERN_ERR "[VIBRATOR ERR]init_vibrator ()regist timed_output_dev failded\n");
		return ret;
	}

	vibrator_single_workqueue =
						create_singlethread_workqueue("vibrator_workqueue");

	if (NULL == vibrator_single_workqueue) {
		printk(KERN_ERR "[VIBRATOR ERR] init_vibrator(): create workqueue error\n");
		return -ENOMEM;
	}

	/* init spinlock */
	spin_lock_init(&vibrator_spinlock);

	/* init hrtimer */
	hrtimer_init(&vib_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	/* Specify timer expiry callback function */
	vib_timer.function = vib_timer_function;
	vib_state = NO_VIBRATE;

	/* init static variable */
	vib_duration = 0;
	return ret;
}

#else /* !defined(VIB_DCMOTOR) */
/*=============================================================================
*						PCM2PWM driver with LINEAR MOTOR
=============================================================================*/

/*
* vibrator_work_turn_off: work function to turn ON pcm2pwm
* @work		: pointer to work_struct object
* return	: None
*/
static void vibrator_work_turn_on(struct work_struct *work)
{
	int ret;
	if (NO_VIBRATE == vib_state) {
		ret = pcm2pwm_open();
		if (ret) {
			printk(KERN_ERR "[VIBRATOR ERR] vibrator_work_turn_on(): call pcm2pwm_open error = %d\n", ret);
			return;
		}
		ret = pcm2pwm_enable(START_PCM2PWM,
								wave_pcm,
								SAMPLE_NUM,
								VIB_CNT_LINEAR);
		if (ret) {
			printk(KERN_ERR "[VIBRATOR ERR] vibrator_work_turn_on(): call pcm2pwm_enable error = %d\n", ret);
			pcm2pwm_close();
			return;
		}


		ret = gpio_direction_output(GPIO_PORT226, HIGH);
		if (ret) {
			printk(KERN_ERR "[VIBRATOR ERR] vibrator_work_turn_on(): gpio_direction_output GPIO_PORT226 error\n");
			return;
		}

		vib_state = VIBRATE;
	}
}

/*
* vibrator_work_turn_off: work function to turn off pcm2pwm
* @work		: pointer to work_struct object
* return	: None
*/
static void vibrator_work_turn_off(struct work_struct *work)
{
	int ret;
	if (VIBRATE == vib_state) {
		ret = pcm2pwm_enable(STOP_PCM2PWM,
								wave_pcm,
								SAMPLE_NUM,
								VIB_CNT_LINEAR);
		if (ret) {
			printk(KERN_ERR "[VIBRATOR ERR] vibrator_work_turn_off(): call pcm2pwm_enable error = %d\n", ret);
			pcm2pwm_close();
			return;
		}

		ret = pcm2pwm_close();
		if (ret) {
			printk(KERN_ERR "[VIBRATOR ERR] vibrator_work_turn_off(): call pcm2pwm_close error = %d\n", ret);
			return;
		}

		vib_state = NO_VIBRATE;

		ret = gpio_direction_output(GPIO_PORT226, LOW);
		if (ret) {
			printk(KERN_ERR "[VIBRATOR ERR] vibrator_work_turn_off(): gpio_direction_output GPIO_PORT226 error\n");
			return;
		}
	}
}

/*
* vib_timer_function: Timer expiry callback function
* @timer:
* return:
*        HRTIMER_NORESTART	: Timer is not restarted (actual value: 0)
*        HRTIMER_RESTART	: Timer must be restarted (actual value: 1)
*/
static enum hrtimer_restart vib_timer_function(struct hrtimer *timer)
{
	/* request to turn off */
	queue_work(vibrator_single_workqueue, &vibrator_work_off_pcm2pwm);
	return HRTIMER_NORESTART;
}

/*
* vibrator_enable: Handle request Turn off vibration
*					or turn on vibration for a specified duration
* @sdev		: Pointer to structure timed_output_dev
* @timeout	: The duration of vibration
* return	: the current number of milliseconds remaining on the timer
*/
extern void vibrator_enable(struct timed_output_dev *sdev, int timeout)
{
	hrtimer_cancel(&vib_timer);
	if (timeout < 0) {
		printk(KERN_ERR "[VIBRATOR ERR] vibrator_enable(): timeout is < 0\n");
		return;
	}
	if (!timeout) {
		/* request to turn off */
		queue_work(vibrator_single_workqueue, &vibrator_work_off_pcm2pwm);
	} else {
		/* request to turn on */
		queue_work(vibrator_single_workqueue, &vibrator_work_on_pcm2pwm);
		/* atfer timeout milisecond,
		* the callback function of hrtimer will request to turn off
		*/
		hrtimer_start(&vib_timer, ktime_set(timeout / 1000,
						(timeout % 1000) * 1000000), HRTIMER_MODE_REL);
	}
}


/*
* init_vibrator: init vibrator driver
* return:
			0: if no error
			other value if error
*/
static int __init init_vibrator(void)
{
	int ret = 0;
	/* Initializing vibrator device object */
	vib_device.name = "vibrator";
	vib_device.enable = vibrator_enable;
	vib_device.get_time = vib_get_time;

	ret = gpio_request(GPIO_PORT228, NULL);
	if (ret) {
		printk(KERN_ERR "[VIBRATOR ERR] init_vibrator(): request GPIO_PORT36 error\n");
		return ret;
	}

	ret = gpio_direction_output(GPIO_PORT228, LOW);
	if (ret) {
		printk(KERN_ERR "[VIBRATOR ERR] init_vibrator(): gpio_direction_output GPIO_PORT36 error\n");
		return ret;
	}

	ret = gpio_request(GPIO_PORT226, NULL);
	if (ret) {
		printk(KERN_ERR "[VIBRATOR ERR] init_vibrator(): request GPIO_PORT226 error\n");
		return ret;
	}

	ret = gpio_direction_output(GPIO_PORT226, LOW);
	if (ret) {
		printk(KERN_ERR "[VIBRATOR ERR] init_vibrator(): gpio_direction_output GPIO_PORT226 error\n");
		return ret;
	}

	/* Register vibrator to timed_output device */
	ret = timed_output_dev_register(&vib_device);
	if (ret) {
		printk(KERN_ERR "[VIBRATOR ERR]init_vibrator(): regist timed_output_dev failed\n");
		return ret;
	}

	/* create vibrator single thread workqueue */
	vibrator_single_workqueue =
						create_singlethread_workqueue("vibrator_workqueue");

	if (NULL == vibrator_single_workqueue) {
		printk(KERN_ERR "[VIBRATOR ERR] init_vibrator(): create workqueue error\n");
		return -ENOMEM;
	}

	/* init work function to hande turn-on/turn-off request */
	INIT_WORK(&vibrator_work_on_pcm2pwm, vibrator_work_turn_on);
	INIT_WORK(&vibrator_work_off_pcm2pwm, vibrator_work_turn_off);

	/* init hrtimer */
	hrtimer_init(&vib_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	/* Specify timer expiry callback function */
	vib_timer.function = vib_timer_function;
	vib_state = NO_VIBRATE;
	return ret;
}
#endif

device_initcall_sync(init_vibrator);

MODULE_AUTHOR("Renesas Mobile");
MODULE_DESCRIPTION("timed output vibrator device");
MODULE_LICENSE("GPL v2");
