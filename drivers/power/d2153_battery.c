/*
 * Battery driver for Dialog D2153
 *   
 * Copyright(c) 2012 Dialog Semiconductor Ltd.
 *  
 * Author: Dialog Semiconductor Ltd. D. Chen, A Austin, E Jeong
 *  
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/errno.h>
#include <linux/power_supply.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/i2c.h>
#include <linux/proc_fs.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/wakelock.h>
#include <linux/gpio.h>

#include <linux/atomic.h>
#include <mach/r8a73734.h>
#include <linux/io.h>
#include <mach/common.h>
#include <linux/jiffies.h>
#include <linux/hwspinlock.h>
#include <linux/kthread.h>
#include <linux/spa_power.h>

#include "linux/err.h"	// test only

#include <linux/d2153/core.h>
#include <linux/d2153/d2153_battery.h>
#include <linux/d2153/d2153_reg.h>

static const char __initdata d2153_battery_banner[] = \
    "D2153 Battery, (c) 2012 Dialog Semiconductor Ltd.\n";

/***************************************************************************
 Pre-definition
***************************************************************************/
#define FALSE								(0)
#define TRUE								(1)

#define DETACHED							(0)
#define ATTACHED							(1)

#define ADC_RES_MASK_LSB					(0x0F)
#define ADC_RES_MASK_MSB					(0xF0)

#if USED_BATTERY_CAPACITY == BAT_CAPACITY_1800MA
#define ADC_VAL_100_PERCENT            		3670 //3665  
#else
#define ADC_VAL_100_PERCENT            		3445
#endif

#if USED_BATTERY_CAPACITY == BAT_CAPACITY_1800MA
#define CV_START_ADC            			3400
#else
#define CV_START_ADC            			3338
#endif

#define FIRST_VOLTAGE_DROP_ADC  			180 //100 //80
#define NORM_NUM                			10000
#define MAX_DIS_OFFSET_FOR_WEIGHT   		200
#define MIN_DIS_OFFSET_FOR_WEIGHT   		30
#define MAX_ADD_DIS_PERCENT_FOR_WEIGHT   	15
#define MIN_ADD_DIS_PERCENT_FOR_WEIGHT  	(-40)

#define MAX_CHA_OFFSET_FOR_WEIGHT   		300
#define MIN_CHA_OFFSET_FOR_WEIGHT   		150
#define MAX_ADD_CHA_PERCENT_FOR_WEIGHT   	10
#define MIN_ADD_CHA_PERCENT_FOR_WEIGHT  	(0)
#define DISCHARGE_SLEEP_OFFSET              20
#define LAST_VOL_UP_PERCENT                 75
#define LAST_CHARGING_WEIGHT      			400

//////////////////////////////////////////////////////////////////////////////
//    Static Function Prototype
//////////////////////////////////////////////////////////////////////////////
//static void d2153_external_event_handler(int category, int event);
static int  d2153_read_adc_in_auto(struct d2153_battery *pbat, adc_channel channel);
static int  d2153_read_adc_in_manual(struct d2153_battery *pbat, adc_channel channel);
static void d2153_sleep_monitor(struct d2153_battery *pbat);

//////////////////////////////////////////////////////////////////////////////
//    Static Variable Declaration
//////////////////////////////////////////////////////////////////////////////
struct hwspinlock *r8a73734_hwlock_pmic_d2153;
static wait_queue_head_t d2153_modem_reset_event;
static struct task_struct *d2153_modem_reset_thread;
static atomic_t modem_reset_handing = ATOMIC_INIT(0);

static struct d2153_battery *gbat = NULL;
static u8  is_called_by_ticker = 0;
static u16 ACT_4P2V_ADC = 0;
static u16 ACT_3P4V_ADC = 0;


// This array is for setting ADC_CONT register about each channel.
static struct adc_cont_in_auto adc_cont_inven[D2153_ADC_CHANNEL_MAX - 1] = {
	// VBAT_S channel
	[D2153_ADC_VOLTAGE] = {
		.adc_preset_val = 0,
		.adc_cont_val = (D2153_ADC_AUTO_EN_MASK | D2153_ADC_MODE_MASK 
							| D2153_AUTO_VBAT_EN_MASK),
		.adc_msb_res = D2153_VDD_RES_VBAT_RES_REG,
		.adc_lsb_res = D2153_ADC_RES_AUTO1_REG,
		.adc_lsb_mask = ADC_RES_MASK_LSB,
	},
	// TEMP_1 channel
	[D2153_ADC_TEMPERATURE_1] = {
		.adc_preset_val = D2153_TEMP1_ISRC_EN_BIT,   // 10uA Current Source enabled
		.adc_cont_val = (D2153_ADC_AUTO_EN_MASK | D2153_ADC_MODE_MASK 
							| D2153_TEMP1_ISRC_EN_MASK),   // 10uA Current Source enabled
		.adc_msb_res = D2153_TBAT1_RES_TEMP1_RES_REG,
		.adc_lsb_res = D2153_ADC_RES_AUTO1_REG,
		.adc_lsb_mask = ADC_RES_MASK_MSB,
	},
	// TEMP_2 channel
	[D2153_ADC_TEMPERATURE_2] = {
		.adc_preset_val =  D2153_TEMP2_ISRC_EN_BIT,   // 10uA Current Source enabled
		.adc_cont_val = (D2153_ADC_AUTO_EN_MASK | D2153_ADC_MODE_MASK 
							| D2153_TEMP2_ISRC_EN_MASK ),   // 10uA Current source enabled
		.adc_msb_res = D2153_TBAT2_RES_TEMP2_RES_REG,
		.adc_lsb_res = D2153_ADC_RES_AUTO3_REG,
		.adc_lsb_mask = ADC_RES_MASK_LSB,
	},
	// VF channel
	[D2153_ADC_VF] = {
		.adc_preset_val = D2153_AUTO_VF_EN_BIT,      // 10uA Current Source enabled
		.adc_cont_val = (D2153_ADC_AUTO_EN_MASK | D2153_ADC_MODE_MASK 
							| D2153_AD4_ISRC_ENVF_ISRC_EN_MASK | D2153_AUTO_VF_EN_MASK),
		.adc_msb_res = D2153_ADCIN4_RES_VF_RES_REG,
		.adc_lsb_res = D2153_ADC_RES_AUTO2_REG,
		.adc_lsb_mask = ADC_RES_MASK_LSB,
	},
	// AIN channel
	[D2153_ADC_AIN] = {
		.adc_preset_val = 0,
		.adc_cont_val = (D2153_ADC_AUTO_EN_MASK | D2153_ADC_MODE_MASK
							| D2153_AD4_ISRC_ENVF_ISRC_EN_MASK),
		.adc_msb_res = D2153_ADCIN5_RES_AIN_RES_REG,
		.adc_lsb_res = D2153_ADC_RES_AUTO2_REG,
		.adc_lsb_mask = ADC_RES_MASK_MSB
	},
};

// Table for compensation of charge offset in CC mode
static u16 initialize_charge_offset[] = {
//  500, 600, 700, 800, 900, 1000, 1100, 1200,
	 18,  22,  26,  30,  34,   36,   40,   44,
};

static u16 initialize_charge_up_cc[] = {
// 500, 600, 700, 800, 900, 1000, 1100, 1200,
   215, 275, 335, 390, 450,  510,  570,  630,
};

// LUT for NCP15XW223 thermistor with 10uA current source selected
static struct adc2temp_lookuptbl adc2temp_lut = {
	// Case of NCP03XH223
	.adc  = {  // ADC-12 input value
		2144,      1691,      1341,      1072,     865,      793,      703,
		577,       480,       400,       334,      285,      239,      199,
		179,       168,       143,       124,      106,      98,       93,
		88, 
	},
	.temp = {	// temperature (degree K)
		C2K(-200), C2K(-150), C2K(-100), C2K(-50), C2K(0),	 C2K(20),  C2K(50),
		C2K(100),  C2K(150),  C2K(200),  C2K(250), C2K(300), C2K(350), C2K(400),
		C2K(430),  C2K(450),  C2K(500),  C2K(550), C2K(600), C2K(630), C2K(650),
		C2K(670),
	},
};

static u16 temp_lut_length = (u16)sizeof(adc2temp_lut.adc)/sizeof(u16);

// adc = (vbat-2500)/2000*2^12
// vbat (mV) = 2500 + adc*2000/2^12
static struct adc2vbat_lookuptbl adc2vbat_lut = {
#if 1
	.adc	 = {1843, 1946, 2148, 2253, 2458, 2662, 2867, 2683, 3072, 3482,}, // ADC-12 input value
	.offset  = {   0,	 0,    0,	 0,    0,	 0,    0,	 0,    0,    0,}, // charging mode ADC offset
	.vbat	 = {3400, 3450, 3500, 3600, 3700, 3800, 3900, 4000, 4100, 4200,}, // VBAT (mV)
#else
    .adc     = {1843, 1946, 2148, 2253, 2458, 2662, 2867, 2683, 3072, 3482,}, // ADC-12 input value
    .offset  = {   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,}, // charging mode ADC offset
    .vbat    = {3400, 3450, 3500, 3600, 3700, 3800, 3900, 4000, 4100, 4200,}, // VBAT (mV)
#endif
};

#if USED_BATTERY_CAPACITY == BAT_CAPACITY_1800MA
static struct adc2soc_lookuptbl adc2soc_lut = {
	.adc_ht  = {1800, 1900, 2150, 2351, 2467, 2605, 2662, 2711, 2750, 2897, 3031, 3233, 3430, ADC_VAL_100_PERCENT,}, // ADC input @ high temp
	.adc_rt  = {1800, 1900, 2150, 2351, 2467, 2605, 2662, 2711, 2750, 2897, 3031, 3233, 3430, ADC_VAL_100_PERCENT,}, // ADC input @ room temp
	.adc_rlt = {1800, 1860, 2038, 2236, 2367, 2481, 2551, 2602, 2650, 2750, 2901, 3038, 3190, 3390,}, // ADC input @ low temp(0)
	.adc_lt  = {1800, 1854, 2000, 2135, 2270, 2390, 2455, 2575, 2645, 2740, 2880, 3020, 3160, 3320,}, // ADC input @ low temp(0)
	.adc_lmt = {1800, 1853, 1985, 2113, 2243, 2361, 2428, 2538, 2610, 2705, 2840, 2985, 3125, 3260,}, // ADC input @ low mid temp(-10)
	.adc_llt = {1800, 1850, 1978, 2105, 2235, 2342, 2405, 2510, 2595, 2680, 2786, 2930, 3040, 3160,}, // ADC input @ low low temp(-20)
	.soc	 = {   0,	10,   30,	50,  100,  200,  300,  400,  500,  600,  700,  800,  900, 1000,}, // SoC in %
};           

//Discharging Weight(Room/Low/low low)          //    0,    1,    3,    5,   10,   20,   30,   40,   50,   60,   70,   80,   90,  100
static u16 adc_weight_section_discharge[]		= {5000, 4000, 3000,  500,	200,   55,	 50,   40,	190,  180,	290,  369,	492, 1000};
static u16 adc_weight_section_discharge_rlt[]	= {3640, 2740,	966,  466,	140,  120,	 93,   80,	128,  151,	150,  176,	186,  780};
static u16 adc_weight_section_discharge_lt[]	= {3200, 2120,	860,  356,	111,   90,	 68,   64,	 96,  106,	110,  130,	139,  710};
static u16 adc_weight_section_discharge_lmt[]	= {2920, 1850,	756,  326,	 94,   79,	 65,   57,	 81,   96,	 99,  121,	128,  670};
static u16 adc_weight_section_discharge_llt[]	= {2730, 1840,	710,  300,	 70,   62,	 55,   51,	 63,   71,	 73,   79,	 85,  630};   // 20121113. will be tested

//Charging Weight(Room/Low/low low)             //    0,    1,    3,    5,   10,   20,   30,   40,   50,   60,   70,   80,   90,  100
static u16 adc_weight_section_charge[]			= {7000, 1500,  700,  200,	155,   60,	 55,   45,	165,  170,	200,  230,	230,  LAST_CHARGING_WEIGHT};
static u16 adc_weight_section_charge_rlt[]		= {7000, 1700, 1000,  225,	155,   60,	 55,   45,	170,  170,	200,  230,	230,  LAST_CHARGING_WEIGHT};
static u16 adc_weight_section_charge_lt[]		= {7000, 1700, 1000,  225,	155,   60,	 55,   45,	170,  170,	200,  230,	230,  LAST_CHARGING_WEIGHT};
static u16 adc_weight_section_charge_lmt[]		= {7000, 1700, 1000,  225,	155,   60,	 55,   45,	170,  170,	200,  230,	230,  LAST_CHARGING_WEIGHT};
static u16 adc_weight_section_charge_llt[]		= {7000, 1700, 1000,  225,	155,   60,	 55,   45,	170,  170,	200,  230,	230,  LAST_CHARGING_WEIGHT};

//Charging Offset                               //    0,    1,    3,    5,   10,   20,   30,   40,   50,   60,   70,   80,   90,  100
static u16 adc_diff_charge[]                    = {  60,   60,  200,  210,  225,  225,  248,  240,  235,  220,  175,  165,  165,    0};
#else
static struct adc2soc_lookuptbl adc2soc_lut = {
	.adc_ht  = {1800, 1870, 2060, 2270, 2400, 2510, 2585, 2635, 2685, 2781, 2933, 3064, 3230, ADC_VAL_100_PERCENT,}, // ADC input @ high temp
	.adc_rt  = {1800, 1870, 2060, 2270, 2400, 2510, 2585, 2635, 2685, 2781, 2933, 3064, 3230, ADC_VAL_100_PERCENT,}, // ADC input @ room temp
	.adc_rlt = {1800, 1860, 2038, 2236, 2367, 2481, 2551, 2602, 2650, 2750, 2901, 3038, 3190, 3390,               }, // ADC input @ low temp(0)
	.adc_lt  = {1800, 1854, 2000, 2135, 2270, 2390, 2455, 2575, 2645, 2740, 2880, 3020, 3160, 3320,               }, // ADC input @ low temp(0)
	.adc_lmt = {1800, 1853, 1985, 2113, 2243, 2361, 2428, 2538, 2610, 2705, 2840, 2985, 3125, 3260,               }, // ADC input @ low mid temp(-10)
	.adc_llt = {1800, 1850, 1978, 2105, 2235, 2342, 2405, 2510, 2595, 2680, 2786, 2930, 3040, 3160,               }, // ADC input @ low low temp(-20)
	.soc	 = {   0,	10,   30,	50,  100,  200,  300,  400,	 500,  600,	 700,  800,	 900, 1000,               }, // SoC in %
};

//Discharging Weight(Room/Low/low low)          //    0,    1,    3,    5,   10,   20,   30,   40,   50,   60,   70,   80,   90,  100
static u16 adc_weight_section_discharge[]       = {5000, 3500, 2000,  600,  260,  220,  115,  112,  280,  450,  420,  600,  800, 1000};
static u16 adc_weight_section_discharge_rlt[]   = {3640, 2740,  966,  466,  140,  120,   93,   80,  128,  151,  150,  176,  186,  780};
static u16 adc_weight_section_discharge_lt[]    = {3200, 2120,  860,  356,  111,   90,   68,   64,   96,  106,  110,  130,  139,  710};
static u16 adc_weight_section_discharge_lmt[]   = {2920, 1850,  756,  326,   94,   79,   65,   57,   81,   96,   99,  121,  128,  670};
static u16 adc_weight_section_discharge_llt[]   = {2730, 1840,  710,  300,   70,   62,   55,   51,   63,   71,   73,   79,   85,  630};   // 20121113. will be tested

//Charging Weight(Room/Low/low low)             //    0,    1,    3,    5,   10,   20,   30,   40,   50,   60,   70,   80,   90,  100
static u16 adc_weight_section_charge[]          = {7000, 5000, 2000,  500,  213,  130,   82,   86,  157,  263,  280,  310,  340,  600};
static u16 adc_weight_section_charge_rlt[]      = {7000, 5000, 2000,  500,  213,  130,   82,   86,  157,  263,  280,  310,  340,  600};
static u16 adc_weight_section_charge_lt[]       = {7000, 5000, 2000,  500,  213,  130,   82,   86,  157,  263,  280,  310,  340,  600};
static u16 adc_weight_section_charge_lmt[]      = {7000, 5000, 2000,  500,  213,  130,   82,   86,  157,  263,  280,  310,  340,  600};
static u16 adc_weight_section_charge_llt[]      = {7000, 5000, 2000,  500,  213,  130,   82,   86,  157,  263,  280,  310,  340,  600};

//Charging Offset                               //    0,    1,    3,    5,   10,   20,   30,   40,   50,   60,   70,   80,   90,  100
static u16 adc_diff_charge[]                    = {  60,   60,  200,  210,  225,  225,  248,  240,  235,  220,  175,  165,  165,    0};
#endif /* BATTERY_CAPACITY_1300mAH */


static u16 adc2soc_lut_length = (u16)sizeof(adc2soc_lut.soc)/sizeof(u16);
static u16 adc2vbat_lut_length = (u16)sizeof(adc2vbat_lut.offset)/sizeof(u16);

/*
 * d2153_get_hw_sem_timeout() - lock an hwspinlock with timeout limit
 * @hwlock: the hwspinlock to be locked
 * @timeout: timeout value in msecs
 */
static int d2153_get_hwsem_timeout(struct hwspinlock *hwlock,
		unsigned int time_out)
{
	int ret;
	unsigned long expire;

	expire = msecs_to_jiffies(time_out) + jiffies;

	for (;;) {
		/* Try to take the hwspinlock */
		ret = hwspin_trylock_nospin(hwlock);
		if (ret == 0)
			break;

		/*
		 * The lock is already taken, try again
		 */
		if (time_is_before_eq_jiffies(expire))
			return -ETIMEDOUT;

		/*
		 * Wait 1 millisecond for another round
		 */
		mdelay(1);
	}

	return ret;
}

/*
 * d2153_force_release_hwsem() - force to release hw semaphore
 * @hwsem_id: Hardware semaphore ID
		0x01: AP Realtime side
 *		0x40: AP System side
 *		0x93: Baseband side
 * return: void
 */
static void d2153_force_release_hwsem(u8 hwsem_id)
{
	void *ptr;
	u32 value = 0;
	unsigned long expire = msecs_to_jiffies(5) + jiffies;

	/*Check input hwsem_id*/
	switch (hwsem_id) {
	case RT_CPU_SIDE:
	case SYS_CPU_SIDE:
	case BB_CPU_SIDE:
		break;
	default:
		return;
	}

	ptr = ioremap(0xe6001830, 4);
	value = ioread32(ptr);
	iounmap(ptr);

	dlg_info("%s: ID (0x%x) is using HW semaphore\n", \
			__func__, value >> 24);

	if ((value >> 24) != hwsem_id)
		return;

	/*enable master access*/
	ptr = ioremap(0xE6001604, 4);
	for (;;) {

		/* Try to enable master access */
		iowrite32(0xC0000000, ptr);
		value = ioread32(ptr);
		if (value == 0xC0000000) {
			iounmap(ptr);
			break;
		}

		/*
		 * Cannot enable master access, try again
		 */
		if (time_is_before_eq_jiffies(expire)) {
			iounmap(ptr);
			return;
		}

		/*
		 * Wait 50 nanosecond for another round
		 */
		ndelay(50);
	}

	/*Force clear HW sem*/
	expire = msecs_to_jiffies(5) + jiffies;

	ptr = ioremap(0xe6001830, 4);
	for (;;) {
		/* Try to force clear HW sem */
		iowrite32(0, ptr);
		value = ioread32(ptr);
		if (value == 0x0) {
			iounmap(ptr);
			dlg_err(
		"%s: Force to release HW sem from ID 0x%x) successful\n",
				__func__, hwsem_id);
			break;
		}

		/*
		 * Cannot force clear HW sem, try again
		 */
		if (time_is_before_eq_jiffies(expire)) {
			iounmap(ptr);
			dlg_err(
				"%s: Fail to release HW sem from ID (0x%x)\n",
					__func__, hwsem_id);
			break;
		}

		/*
		 * Wait 50 nanosecond for another round
		 */
		ndelay(50);
	}

	/*Disable master access*/
	expire = msecs_to_jiffies(5) + jiffies;
	ptr = ioremap(0xE6001604, 4);
	for (;;) {
		/* Try to disable master access */
		iowrite32(0, ptr);
		value = ioread32(ptr);
		if (value == 0x0) {
			iounmap(ptr);
			break;
		}

		/*
		 * Cannot disable master access, try again
		 */
		if (time_is_before_eq_jiffies(expire)) {
			iounmap(ptr);
			return;
		}

		/*
		 * Wait 50 nanosecond for another round
		 */
		ndelay(50);
	}
}

/*
 * d2153_force_release_swsem() - force to release sw semaphore
 * @swsem_id: Software semaphore ID
		0x01: AP Realtime side
 *		0x40: AP System side
 *		0x93: Baseband side
 * return: void
 */
static void d2153_force_release_swsem(u8 swsem_id)
{
	u32 lock_id;
	unsigned long expire = msecs_to_jiffies(10) + jiffies;

	/*Check input swsem_id*/
	switch (swsem_id) {
	case RT_CPU_SIDE:
	case SYS_CPU_SIDE:
	case BB_CPU_SIDE:
		break;
	default:
		return;
	}

	/* Check which CPU (Real time or Baseband or System) is using SW sem*/
	lock_id = hwspin_get_lock_id_nospin(r8a73734_hwlock_pmic_d2153);

	dlg_info("%s: ID (0x%x) is using SW semaphore\n", \
				__func__, lock_id);

	if (lock_id != swsem_id)
		return;

	for (;;) {
		/* Try to force to unlock SW sem*/
		hwspin_unlock_nospin(r8a73734_hwlock_pmic_d2153);
		lock_id = hwspin_get_lock_id_nospin(r8a73734_hwlock_pmic_d2153);
		if (lock_id == 0) {
			dlg_err(
		"%s: Forcing to release SW sem from ID (0x%x) is successful\n",
				__func__, swsem_id);
			break;
		}

		/*
		 * Cannot force to unlock SW sem, try again
		 */
		if (time_is_before_eq_jiffies(expire)) {
			dlg_err(
				"%s: Fail to release HW sem from ID (0x%x)\n",
					__func__, swsem_id);
			return;
		}

		/*
		 * Wait 100 nanosecond for another round
		 */
		ndelay(100);
	}

}

/*
 * d2153_handle_modem_reset: Handle modem reset
 * return: void
 */
void d2153_handle_modem_reset(void)
{
	if(u2_get_board_rev() <= 4) {
		dlg_info("%s is called on old Board revision. error\n", __func__);
		return;
	}
	atomic_set(&modem_reset_handing, 1);
	wake_up_interruptible(&d2153_modem_reset_event);
}
EXPORT_SYMBOL(d2153_handle_modem_reset);

/*
 * d2153_modem_reset: start thread to handle modem reset
 * @ptr:
 * return: 0
 */
static int d2153_modem_thread(void *ptr)
{
	while (!kthread_should_stop()) {
		wait_event_interruptible(d2153_modem_reset_event,
					atomic_read(&modem_reset_handing));

		d2153_force_release_hwsem(BB_CPU_SIDE);
		d2153_force_release_swsem(BB_CPU_SIDE);

		atomic_set(&modem_reset_handing, 0);
	}
	return 0;
}

/* 
 * Name : chk_lut
 *
 */
static int chk_lut (u16* x, u16* y, u16 v, u16 l) {
	int i;
	//u32 ret;
	int ret;

	if (v < x[0])
		ret = y[0];
	else if (v >= x[l-1])
		ret = y[l-1]; 
	else {          
		for (i = 1; i < l; i++) {          
			if (v < x[i])               
				break;      
		}       
		ret = y[i-1];       
		ret = ret + ((v-x[i-1])*(y[i]-y[i-1]))/(x[i]-x[i-1]);   
	}   
	//return (u16) ret;
	return ret;
}

/* 
 * Name : chk_lut_temp
 * return : The return value is Kelvin degree
 */
static int chk_lut_temp (u16* x, u16* y, u16 v, u16 l) {
	int i, ret;

	if (v >= x[0])
		ret = y[0];
	else if (v < x[l-1])
		ret = y[l-1]; 
	else {			
		for (i=1; i < l; i++) { 		 
			if (v > x[i])				
				break;		
		}		
		ret = y[i-1];		
		ret = ret + ((v-x[i-1])*(y[i]-y[i-1]))/(x[i]-x[i-1]);	
	}

	//pr_info("%s. Result (%d)\n", __func__, ret);
	
	return ret;
}


/* 
 * Name : adc_to_soc_with_temp_compensat
 *
 */
u32 adc_to_soc_with_temp_compensat(u16 adc, u16 temp) {	
	int sh, sl;

	//pr_info("%s. adc = %d. temp = %d\n", __func__, adc, temp);

	if(u2_get_board_rev() <= 4) {
		dlg_info("%s is called on old Board revision. error\n", __func__);
		return 0;
	}
	if(temp < BAT_LOW_LOW_TEMPERATURE)
		temp = BAT_LOW_LOW_TEMPERATURE;
	else if(temp > BAT_HIGH_TEMPERATURE)
		temp = BAT_HIGH_TEMPERATURE;

	if((temp <= BAT_HIGH_TEMPERATURE) && (temp > BAT_ROOM_TEMPERATURE)) {
		sh = chk_lut(adc2soc_lut.adc_ht, adc2soc_lut.soc, adc, adc2soc_lut_length);    
		sl = chk_lut(adc2soc_lut.adc_rt, adc2soc_lut.soc, adc, adc2soc_lut_length);
		sh = sl + (temp - BAT_ROOM_TEMPERATURE)*(sh - sl)
								/ (BAT_HIGH_TEMPERATURE - BAT_ROOM_TEMPERATURE);
	} else if((temp <= BAT_ROOM_TEMPERATURE) && (temp > BAT_ROOM_LOW_TEMPERATURE)) {
		sh = chk_lut(adc2soc_lut.adc_rt, adc2soc_lut.soc, adc, adc2soc_lut_length); 	   
		sl = chk_lut(adc2soc_lut.adc_rlt, adc2soc_lut.soc, adc, adc2soc_lut_length); 	   
		sh = sl + (temp - BAT_ROOM_LOW_TEMPERATURE)*(sh - sl)
								/ (BAT_ROOM_TEMPERATURE-BAT_ROOM_LOW_TEMPERATURE);
	} else if((temp <= BAT_ROOM_LOW_TEMPERATURE) && (temp > BAT_LOW_TEMPERATURE)) {
		sh = chk_lut(adc2soc_lut.adc_rlt, adc2soc_lut.soc, adc, adc2soc_lut_length); 	   
		sl = chk_lut(adc2soc_lut.adc_lt, adc2soc_lut.soc, adc, adc2soc_lut_length); 	   
		sh = sl + (temp - BAT_LOW_TEMPERATURE)*(sh - sl)
								/ (BAT_ROOM_LOW_TEMPERATURE-BAT_LOW_TEMPERATURE);
	} else if((temp <= BAT_LOW_TEMPERATURE) && (temp > BAT_LOW_MID_TEMPERATURE)) {
		sh = chk_lut(adc2soc_lut.adc_lt, adc2soc_lut.soc, adc, adc2soc_lut_length); 	   
		sl = chk_lut(adc2soc_lut.adc_lmt, adc2soc_lut.soc, adc, adc2soc_lut_length);		
		sh = sl + (temp - BAT_LOW_MID_TEMPERATURE)*(sh - sl)
								/ (BAT_LOW_TEMPERATURE-BAT_LOW_MID_TEMPERATURE);	
	} else {		
		sh = chk_lut(adc2soc_lut.adc_lmt, adc2soc_lut.soc,	adc, adc2soc_lut_length);		 
		sl = chk_lut(adc2soc_lut.adc_llt, adc2soc_lut.soc, adc, adc2soc_lut_length);
		sh = sl + (temp - BAT_LOW_LOW_TEMPERATURE)*(sh - sl)
								/ (BAT_LOW_MID_TEMPERATURE-BAT_LOW_LOW_TEMPERATURE); 
	}

	return sh;
}


static u16 pre_soc = 0xffff;
u16 soc_filter(u16 new_soc, u8 is_charging) {
	u16 soc = new_soc;

	if(u2_get_board_rev() <= 4) {
		dlg_info("%s is called on old Board revision. error\n", __func__);
		return 0;
	}
	if(pre_soc == 0xffff)
		pre_soc = soc;
	else {
		if( soc > pre_soc)
		{
			if(is_charging)
			{
				if(soc <= pre_soc + 2)
					pre_soc = soc;
				else {
					soc = pre_soc + 1;
					pre_soc = soc;
				}
			} else
				soc = pre_soc; //in discharge, SoC never goes up
		} else {
			if(soc >= pre_soc - 2)
				pre_soc = soc;
			else {
				soc = pre_soc - 1;
				pre_soc = soc;
			}
		}
	}
	return (soc);
}


/* 
 * Name : adc_to_degree
 *
 */
//u16 adc_to_degree_k(u16 adc) {
int adc_to_degree_k(u16 adc) {

	if(u2_get_board_rev() <= 4) {
		dlg_info("%s is called on old Board revision. error\n", __func__);
		return 0;
	}
    return (chk_lut_temp(adc2temp_lut.adc, adc2temp_lut.temp, adc, temp_lut_length));
}

int degree_k2c(u16 k) {
	if(u2_get_board_rev() <= 4) {
		dlg_info("%s is called on old Board revision. error\n", __func__);
		return 0;
	}
	return (K2C(k));
}

/* 
 * Name : get_adc_offset
 *
 */
//u16 get_adc_offset(u16 adc) {	
int get_adc_offset(u16 adc) {	

	if(u2_get_board_rev() <= 4) {
		dlg_info("%s is called on old Board revision. error\n", __func__);
		return 0;
	}
    return (chk_lut(adc2vbat_lut.adc, adc2vbat_lut.offset, adc, adc2vbat_lut_length));
}

/* 
 * Name : adc_to_vbat
 *
 */
u16 adc_to_vbat(u16 adc, u8 is_charging) {    
	u16 a = adc;

	if(u2_get_board_rev() <= 4) {
		dlg_info("%s is called on old Board revision. error\n", __func__);
		return 0;
	}
	if(is_charging)
		a = adc - get_adc_offset(adc); // deduct charging offset
	// return (chk_lut(adc2vbat_lut.adc, adc2vbat_lut.vbat, a, adc2vbat_lut_length));
	return (2500 + ((a * 2000) >> 12));
}

/* 
 * Name : adc_to_soc
 * get SOC (@ room temperature) according ADC input
 */
//u16 adc_to_soc(u16 adc, u8 is_charger) { 
int adc_to_soc(u16 adc, u8 is_charging) { 

	u16 a = adc;

	if(u2_get_board_rev() <= 4) {
		dlg_info("%s is called on old Board revision. error\n", __func__);
		return 0;
	}
	if(is_charging)
		a = adc - get_adc_offset(adc); // deduct charging offset
	return (chk_lut(adc2soc_lut.adc_rt, adc2soc_lut.soc, a, adc2soc_lut_length));
}


/* 
 * Name : d2153_get_current_voltage
 */
static int d2153_get_current_voltage(struct d2153_battery *pbat)
{
	int current_voltage;

	if(unlikely(!pbat)) {
		pr_err("%s. Invalid driver data\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&pbat->lock);
	current_voltage = pbat->battery_data.current_voltage;
	mutex_unlock(&pbat->lock);

	return current_voltage;
}


/* 
 * Name : d2153_get_average_voltage
 */
static int d2153_get_average_voltage(struct d2153_battery *pbat)
{
	int average_voltage;
	
	if(unlikely(!pbat)) {
		pr_err("%s. Invalid driver data\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&pbat->lock);
	average_voltage = pbat->battery_data.average_voltage;
	mutex_unlock(&pbat->lock);

	return average_voltage;
}


/* 
 * Name : d2153_get_average_temperature
 */
static int d2153_get_average_temperature(struct d2153_battery *pbat)
{
	int average_temperature;
	
	if(unlikely(!pbat)) {
		pr_err("%s. Invalid driver data\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&pbat->lock);
	average_temperature = pbat->battery_data.average_temperature;
	mutex_unlock(&pbat->lock);

	return average_temperature;
}


/* 
 * Name : d2153_get_average_temperature_adc
 */
static int d2153_get_average_temperature_adc(struct d2153_battery *pbat)
{
	int average_temperature_adc;
	
	if(unlikely(!pbat)) {
		pr_err("%s. Invalid driver data\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&pbat->lock);
	average_temperature_adc = pbat->battery_data.average_temp_adc;
	mutex_unlock(&pbat->lock);

	return average_temperature_adc;
}


/* 
 * Name : d2153_get_soc
 */
static int d2153_get_soc(struct d2153_battery *pbat)
{
	//u8 is_charger = 0;
	int soc;
 	struct d2153_battery_data *pbat_data = NULL;

	if(pbat == NULL) {
		pr_err("%s. Invalid parameter. \n", __func__);
		return 0;
	}

	pbat_data = &pbat->battery_data;

	if(pbat_data->soc)
		pbat_data->prev_soc = pbat_data->soc;

	soc = adc_to_soc_with_temp_compensat(pbat_data->average_volt_adc, 
										C2K(pbat_data->average_temperature));
	if(soc <= 0) {
		pbat_data->soc = 0;
		if(pbat_data->current_voltage >= BAT_POWER_OFF_VOLTAGE
			|| (pbat_data->is_charging == TRUE)) {
			soc = 10;
		}
	}
	else if(soc >= 1000) {
		soc = 1000;
	}

	// Don't allow soc goes up when battery is dicharged.
	// and also don't allow soc goes down when battey is charged.
	if(pbat_data->is_charging != TRUE 
		&& (soc > pbat_data->prev_soc && pbat_data->prev_soc )) {
		soc = pbat_data->prev_soc;
	}
	else if(pbat_data->is_charging
		&& (soc < pbat_data->prev_soc) && pbat_data->prev_soc) {
		soc = pbat_data->prev_soc;
	}
	pbat_data->soc = soc;

	return soc;
}


/* 
 * Name : d2153_set_adc_mode
 * get resistance (ohm) of VF from ADC input, using 10uA current source
 */ 
static u32 d2153_get_vf_ohm (u16 adc) {
	u32 ohm;
	ohm = (2500 * adc * 100000); // R = 2.5*adc/(10*10^-6)/2^D2153_ADC_RESOLUTION
	ohm >>= D2153_ADC_RESOLUTION;
	ohm /= 1000;
	return (ohm);
}

static u16 d2153_get_target_adc_from_lookup_at_charging(u16 tempk, u16 average_adc, u8 is_charging)
{
	u8 i = 0;
	u16 *plut = NULL;
	int diff;

	if (tempk < BAT_LOW_LOW_TEMPERATURE)		
		plut = &adc2soc_lut.adc_llt[0];
	else if (tempk > BAT_ROOM_TEMPERATURE)
		plut = &adc2soc_lut.adc_rt[0];
	else if (tempk > BAT_ROOM_LOW_TEMPERATURE)
		plut = &adc2soc_lut.adc_rlt[0];
	else
		plut = &adc2soc_lut.adc_lt[0];	
	
	for(i = adc2soc_lut_length - 1; i; i--) {
		if(plut[i] <= average_adc)
			break;
	}
	diff = adc_diff_charge[i] + ((average_adc - plut[i])*(adc_diff_charge[i+1]-adc_diff_charge[i])) 
				/ (plut[i+1] - plut[i]);
	
	if(diff < 0)
	{
		pr_info ("Diff can NEVER be less than 0!");
		diff = 0;
	}
	return (u16)diff;
}

static u16 d2153_get_weight_from_lookup(u16 tempk, u16 average_adc, u8 is_charging)
{
	u8 i = 0;
	u16 *plut = NULL;
	int weight = 0;

	// Sanity check.
	if (tempk < BAT_LOW_LOW_TEMPERATURE)		
		tempk = BAT_LOW_LOW_TEMPERATURE;
	else if (tempk > BAT_HIGH_TEMPERATURE)
		tempk = BAT_HIGH_TEMPERATURE;

	// Get the SOC look-up table
	if(tempk >= BAT_HIGH_TEMPERATURE) {
		plut = &adc2soc_lut.adc_ht[0];
	} else if(tempk < BAT_HIGH_TEMPERATURE && tempk >= BAT_ROOM_TEMPERATURE) {
		plut = &adc2soc_lut.adc_rt[0];
	} else if(tempk < BAT_ROOM_TEMPERATURE && tempk >= BAT_ROOM_LOW_TEMPERATURE) {
		plut = &adc2soc_lut.adc_rlt[0];
	} else if (tempk < BAT_ROOM_LOW_TEMPERATURE && tempk >= BAT_LOW_TEMPERATURE) {
		plut = &adc2soc_lut.adc_lt[0];
	} else if(tempk < BAT_LOW_TEMPERATURE && tempk >= BAT_LOW_MID_TEMPERATURE) {
		plut = &adc2soc_lut.adc_lmt[0];
	} else 
		plut = &adc2soc_lut.adc_llt[0];

	for(i = adc2soc_lut_length - 1; i; i--) {
		if(plut[i] <= average_adc)
			break;
	}

	if((tempk <= BAT_HIGH_TEMPERATURE) && (tempk > BAT_ROOM_TEMPERATURE)) {  
		if(is_charging) {
			if(average_adc < plut[0]) {
				// under 1% -> fast charging
				weight = adc_weight_section_charge[0];
			} else
				weight = adc_weight_section_charge[i];
		} else
			weight = adc_weight_section_discharge[i];
	} else if((tempk <= BAT_ROOM_TEMPERATURE) && (tempk > BAT_ROOM_LOW_TEMPERATURE)) {
		if(is_charging) {
			if(average_adc < plut[0]) i = 0;
		
			weight=adc_weight_section_charge_rlt[i];
			weight = weight + ((tempk-BAT_ROOM_LOW_TEMPERATURE)*(adc_weight_section_charge[i]-adc_weight_section_charge_rlt[i]))
								/(BAT_ROOM_TEMPERATURE-BAT_ROOM_LOW_TEMPERATURE);
		} else {
			weight=adc_weight_section_discharge_rlt[i];
			weight = weight + ((tempk-BAT_ROOM_LOW_TEMPERATURE)*(adc_weight_section_discharge[i]-adc_weight_section_discharge_rlt[i]))
								/(BAT_ROOM_TEMPERATURE-BAT_ROOM_LOW_TEMPERATURE); 
		}
	} else if((tempk <= BAT_ROOM_LOW_TEMPERATURE) && (tempk > BAT_LOW_TEMPERATURE)) {
		if(is_charging) {
			if(average_adc < plut[0]) i = 0;
		
			weight = adc_weight_section_charge_lt[i];
			weight = weight + ((tempk-BAT_LOW_TEMPERATURE)*(adc_weight_section_charge_rlt[i]-adc_weight_section_charge_lt[i]))
								/(BAT_ROOM_LOW_TEMPERATURE-BAT_LOW_TEMPERATURE);
		} else {
			weight = adc_weight_section_discharge_lt[i];
			weight = weight + ((tempk-BAT_LOW_TEMPERATURE)*(adc_weight_section_discharge_rlt[i]-adc_weight_section_discharge_lt[i]))
								/(BAT_ROOM_LOW_TEMPERATURE-BAT_LOW_TEMPERATURE); 
		}
	} else if((tempk <= BAT_LOW_TEMPERATURE) && (tempk > BAT_LOW_MID_TEMPERATURE)) {
		if(is_charging) {
			if(average_adc < plut[0]) i = 0;
		
			weight = adc_weight_section_charge_lmt[i];
			weight = weight + ((tempk-BAT_LOW_MID_TEMPERATURE)*(adc_weight_section_charge_lt[i]-adc_weight_section_charge_lmt[i]))
								/(BAT_LOW_TEMPERATURE-BAT_LOW_MID_TEMPERATURE);
		} else {
			weight = adc_weight_section_discharge_lmt[i];
			weight = weight + ((tempk-BAT_LOW_MID_TEMPERATURE)*(adc_weight_section_discharge_lt[i]-adc_weight_section_discharge_lmt[i]))
								/(BAT_LOW_TEMPERATURE-BAT_LOW_MID_TEMPERATURE); 
		}
	} else {		
		if(is_charging) {
			if(average_adc < plut[0]) i = 0;

			weight = adc_weight_section_charge_llt[i];
			weight = weight + ((tempk-BAT_LOW_LOW_TEMPERATURE)*(adc_weight_section_charge_lmt[i]-adc_weight_section_charge_llt[i]))
								/(BAT_LOW_MID_TEMPERATURE-BAT_LOW_LOW_TEMPERATURE); 
		} else {
			weight = adc_weight_section_discharge_llt[i];
			weight = weight + ((tempk-BAT_LOW_LOW_TEMPERATURE)*(adc_weight_section_discharge_lmt[i]-adc_weight_section_discharge_llt[i]))
								/(BAT_LOW_MID_TEMPERATURE-BAT_LOW_LOW_TEMPERATURE); 
		}
	}

	return weight;	

}


/* 
 * Name : d2153_set_adc_mode
 */
static int d2153_set_adc_mode(struct d2153_battery *pbat, adc_mode type)
{
	if(unlikely(!pbat)) {
		pr_err("%s. Invalid parameter.\n", __func__);
		return -EINVAL;
	}

	if(pbat->adc_mode != type)
	{
		if(type == D2153_ADC_IN_AUTO) {
			pbat->d2153_read_adc = d2153_read_adc_in_auto;
			pbat->adc_mode = D2153_ADC_IN_AUTO;
		}
		else if(type == D2153_ADC_IN_MANUAL) {
			pbat->d2153_read_adc = d2153_read_adc_in_manual;
			pbat->adc_mode = D2153_ADC_IN_MANUAL;
		}
	}
	else {
		pr_info("%s: ADC mode is same before was set \n", __func__);
	}

	return 0;
}


/* 
 * Name : d2153_read_adc_in_auto
 * Desc : Read ADC raw data for each channel.
 * Param : 
 *    - d2153 : 
 *    - channel : voltage, temperature 1, temperature 2, VF and TJUNC* Name : d2153_set_end_of_charge
 */
static int d2153_read_adc_in_auto(struct d2153_battery *pbat, adc_channel channel)
{
	u8 msb_res, lsb_res;
	int ret = 0;
	struct d2153_battery_data *pbat_data = &pbat->battery_data;
	struct d2153 *d2153 = pbat->pd2153;

	if(unlikely(!pbat || !pbat_data || !d2153)) {
		pr_err("%s. Invalid argument\n", __func__);
		return -EINVAL;
	}


	// The valid channel is from ADC_VOLTAGE to ADC_AIN in auto mode.
	if(channel >= D2153_ADC_CHANNEL_MAX - 1) {
		pr_err("%s. Invalid channel(%d) in auto mode\n", __func__, channel);
		return -EINVAL;
	}

	ret = d2153_get_hwsem_timeout(r8a73734_hwlock_pmic_d2153, CONST_HPB_WAIT);
	
	if (ret < 0) {
			pr_err("%s:lock is already taken.\n", __func__);
			return -EBUSY;
	}
	
	mutex_lock(&pbat->meoc_lock);

	pbat_data->adc_res[channel].is_adc_eoc = FALSE;
	pbat_data->adc_res[channel].read_adc = 0;

	// Set ADC_CONT register to select a channel.
	if(adc_cont_inven[channel].adc_preset_val) {
		ret = d2153_reg_write(d2153, D2153_ADC_CONT_REG, adc_cont_inven[channel].adc_preset_val);
		msleep(1);
		ret |= d2153_set_bits(d2153, D2153_ADC_CONT_REG, adc_cont_inven[channel].adc_cont_val);
		if(ret < 0)
			goto out;
	} else {
		ret = d2153_reg_write(d2153, D2153_ADC_CONT_REG, adc_cont_inven[channel].adc_cont_val);
		if(ret < 0)
			goto out;
	}
	msleep(2);

	// Read result register for requested adc channel
	ret = d2153_reg_read(d2153, adc_cont_inven[channel].adc_msb_res, &msb_res);
	ret |= d2153_reg_read(d2153, adc_cont_inven[channel].adc_lsb_res, &lsb_res);
	lsb_res &= adc_cont_inven[channel].adc_lsb_mask;
	if((ret = d2153_reg_write(d2153, D2153_ADC_CONT_REG, 0x00)) < 0)
		goto out;

	// Make ADC result
	pbat_data->adc_res[channel].is_adc_eoc = TRUE;
	pbat_data->adc_res[channel].read_adc =
		((msb_res << 4) | (lsb_res >> 
			(adc_cont_inven[channel].adc_lsb_mask == ADC_RES_MASK_MSB ? 4 : 0)));

out:
	mutex_unlock(&pbat->meoc_lock);
	
	hwspin_unlock_nospin(r8a73734_hwlock_pmic_d2153);
	
	return ret;
}


/* 
 * Name : d2153_read_adc_in_manual
 */
static int d2153_read_adc_in_manual(struct d2153_battery *pbat, adc_channel channel)
{
	u8 mux_sel, flag = FALSE;
	int ret, retries = D2153_MANUAL_READ_RETRIES;
	struct d2153_battery_data *pbat_data = &pbat->battery_data;
	struct d2153 *d2153 = pbat->pd2153;

	ret = d2153_get_hwsem_timeout(r8a73734_hwlock_pmic_d2153, CONST_HPB_WAIT);
	
	if (ret < 0) {
			pr_err("%s:lock is already taken.\n", __func__);
			return -EBUSY;
	}
	
	mutex_lock(&pbat->meoc_lock);

	pbat_data->adc_res[channel].is_adc_eoc = FALSE;
	pbat_data->adc_res[channel].read_adc = 0;

	switch(channel) {
		case D2153_ADC_VOLTAGE:
			mux_sel = D2153_MUXSEL_VBAT;
			break;
		case D2153_ADC_TEMPERATURE_1:
			mux_sel = D2153_MUXSEL_TEMP1;
			break;
		case D2153_ADC_TEMPERATURE_2:
			mux_sel = D2153_MUXSEL_TEMP2;
			break;
		case D2153_ADC_VF:
			mux_sel = D2153_MUXSEL_VF;
			break;
		case D2153_ADC_TJUNC:
			mux_sel = D2153_MUXSEL_TJUNC;
			break;
		default :
			pr_err("%s. Invalid channel(%d) \n", __func__, channel);
			ret = -EINVAL;
			goto out;
	}

	mux_sel |= D2153_MAN_CONV_MASK;
	if((ret = d2153_reg_write(d2153, D2153_ADC_MAN_REG, mux_sel)) < 0)
		goto out;

	do {
		schedule_timeout_interruptible(msecs_to_jiffies(1));
		flag = pbat_data->adc_res[channel].is_adc_eoc;
	} while(retries-- && (flag == FALSE));

	if(flag == FALSE) {
		pr_warn("%s. Failed manual ADC conversion. channel(%d)\n", __func__, channel);
		ret = -EIO;
	}

out:
	mutex_unlock(&pbat->meoc_lock);
	
	hwspin_unlock_nospin(r8a73734_hwlock_pmic_d2153);
	
	return ret;    
}


/* 
 * Name : d2153_check_offset_limits
 */
static void d2153_check_offset_limits(int *A, int *B)
{
	if(*A > D2153_CAL_MAX_OFFSET)
		*A = D2153_CAL_MAX_OFFSET;
	else if(*A < -D2153_CAL_MAX_OFFSET)
		*A = -D2153_CAL_MAX_OFFSET;

	if(*B > D2153_CAL_MAX_OFFSET)
		*B = D2153_CAL_MAX_OFFSET;
	else if(*B < -D2153_CAL_MAX_OFFSET)
		*B = -D2153_CAL_MAX_OFFSET;

	return;
}


/* 
 * Name : d2153_get_calibration_offset
 */
static int d2153_get_calibration_offset(int voltage, int y1, int y0)
{
	int x1 = D2153_CAL_HIGH_VOLT, x0 = D2153_CAL_LOW_VOLT;
	int x = voltage, y = 0;

	y = y0 + ((x-x0)*y1 - (x-x0)*y0) / (x1-x0);

	return y;
}


/* 
 * Name : d2153_read_voltage
 */
#ifdef CONFIG_SEC_CHARGING_FEATURE
extern struct spa_power_data spa_power_pdata;
#endif
static int d2153_read_voltage(struct d2153_battery *pbat,struct power_supply *ps)
{
	int new_vol_adc = 0, base_weight,new_vol_orign;
	int offset_with_old, offset_with_new = 0;
	int ret = 0;
	static int calOffset_4P2, calOffset_3P4 = 0;
	int num_multi=0;
	struct d2153_battery_data *pbat_data = &pbat->battery_data;
	u16 offset_charging=0;
	union power_supply_propval charger_type;
	int charging_index;

	ps->get_property(ps, POWER_SUPPLY_PROP_TYPE, &charger_type);

	if(charger_type.intval== POWER_SUPPLY_TYPE_USB)  {
#ifdef CONFIG_SEC_CHARGING_FEATURE
		pbat_data->is_charging=1;
		charging_index=(spa_power_pdata.charging_cur_wall-500)/100;
#else  /* CONFIG_SEC_CHARGING_FEATURE */
		pbat_data->is_charging=0;
		charging_index=0;
#endif /* CONFIG_SEC_CHARGING_FEATURE */
		pr_info("Charging mode USB charger connected \n");
	}
	else if(charger_type.intval== POWER_SUPPLY_TYPE_USB_DCP)
	{
#ifdef CONFIG_SEC_CHARGING_FEATURE
		pbat_data->is_charging=1;
		charging_index=(spa_power_pdata.charging_cur_wall-500)/100;
#else  /* CONFIG_SEC_CHARGING_FEATURE */
		pbat_data->is_charging=0;
		charging_index=0;
#endif /* CONFIG_SEC_CHARGING_FEATURE */
		pr_info("Charging mode Wall charger connected \n");
	}
	else {
		pbat_data->is_charging=0;
		charging_index=0;
		pr_info("Discharging mode \n");
	}
	
	// Read voltage ADC
	ret = pbat->d2153_read_adc(pbat, D2153_ADC_VOLTAGE);

	if(ret < 0)
		return ret;
	// Getting calibration result.
#if 0
	if(ACT_4P2V_ADC == 0 && SYSPARM_GetIsInitialized()) {
		ACT_4P2V_ADC = SYSPARM_GetActual4p2VoltReading();
		ACT_3P4V_ADC = SYSPARM_GetActualLowVoltReading();

		if(ACT_4P2V_ADC && ACT_3P4V_ADC) {
			calOffset_4P2 = D2153_BASE_4P2V_ADC - ACT_4P2V_ADC;
			calOffset_3P4 = D2153_BASE_3P4V_ADC - ACT_3P4V_ADC;

			d2153_check_offset_limits(&calOffset_4P2, &calOffset_3P4);
		}
	}
#endif

	if(pbat_data->is_charging)
	{			
		offset_charging = initialize_charge_offset[charging_index];

		adc2soc_lut.adc_ht[ADC2SOC_LUT_SIZE-1] = ADC_VAL_100_PERCENT+offset_charging;
		adc2soc_lut.adc_rt[ADC2SOC_LUT_SIZE-1] = ADC_VAL_100_PERCENT+offset_charging;
		adc2soc_lut.adc_rlt[ADC2SOC_LUT_SIZE-1] = 3390+offset_charging;
		adc2soc_lut.adc_lt[ADC2SOC_LUT_SIZE-1] = 3320+offset_charging;
		adc2soc_lut.adc_lmt[ADC2SOC_LUT_SIZE-1] = 3260+offset_charging;
		adc2soc_lut.adc_llt[ADC2SOC_LUT_SIZE-1] = 3160+offset_charging;

	} else {
		adc2soc_lut.adc_ht[ADC2SOC_LUT_SIZE-1] = ADC_VAL_100_PERCENT;
		adc2soc_lut.adc_rt[ADC2SOC_LUT_SIZE-1] = ADC_VAL_100_PERCENT;
		adc2soc_lut.adc_rlt[ADC2SOC_LUT_SIZE-1] = 3390;
		adc2soc_lut.adc_lt[ADC2SOC_LUT_SIZE-1] = 3320;
		adc2soc_lut.adc_lmt[ADC2SOC_LUT_SIZE-1] = 3260;
		adc2soc_lut.adc_llt[ADC2SOC_LUT_SIZE-1] = 3160;
	}

	
	if(pbat_data->adc_res[D2153_ADC_VOLTAGE].is_adc_eoc) {
		int offset = 0;

		new_vol_orign = new_vol_adc = pbat_data->adc_res[D2153_ADC_VOLTAGE].read_adc;

		// To be made a new VBAT_S ADC by interpolation with calibration result.
		if(ACT_4P2V_ADC != 0 && ACT_3P4V_ADC != 0) {
			if(calOffset_4P2 && calOffset_3P4) {
				offset = d2153_get_calibration_offset(pbat_data->average_voltage, 
														calOffset_4P2, 
														calOffset_3P4);
			}
			pr_info("%s. new_vol_adc = %d, offset = %d new_vol_adc + offset = %d \n", 
							__func__, new_vol_adc, offset, (new_vol_adc + offset));
			new_vol_adc = new_vol_adc + offset;
		}
		
		if(pbat->battery_data.volt_adc_init_done) {

			//battery_status = d2153_get_battery_status(pbat);

			base_weight = d2153_get_weight_from_lookup(
											C2K(pbat_data->average_temperature),
											pbat_data->average_volt_adc,
											pbat_data->is_charging);

			if(pbat_data->is_charging) {

				offset_with_new = new_vol_adc - pbat_data->average_volt_adc; 
				// Case of Charging
				// The battery may be discharged, even if a charger is attached.

				if(pbat_data->average_volt_adc > CV_START_ADC)
					base_weight = base_weight + ((pbat_data->average_volt_adc 
							- CV_START_ADC)*(LAST_CHARGING_WEIGHT-base_weight))
							/ ((ADC_VAL_100_PERCENT+offset) - CV_START_ADC);
				
				if(offset_with_new > 0) {
					pbat_data->sum_total_adc += (offset_with_new * base_weight);

					num_multi = pbat_data->sum_total_adc / NORM_NUM;
					if(num_multi > 0) {						
						new_vol_adc = pbat_data->average_volt_adc + num_multi;
						pbat_data->sum_total_adc = pbat_data->sum_total_adc - (num_multi*NORM_NUM);
					}
					else
						new_vol_adc = pbat_data->average_volt_adc;
				}
				else
					new_vol_adc = pbat_data->average_volt_adc;

				pbat_data->current_volt_adc = new_vol_adc;
				pbat_data->sum_voltage_adc += new_vol_adc;
				pbat_data->sum_voltage_adc -= pbat_data->average_volt_adc; //pbat_data->voltage_adc[pbat_data->voltage_idx];
				pbat_data->voltage_adc[pbat_data->voltage_idx] = new_vol_adc;
			}
			else {				
				// Case of Discharging.
				offset_with_new = pbat_data->average_volt_adc - new_vol_adc;
				offset_with_old = pbat_data->voltage_adc[pbat_data->voltage_idx] 
								- pbat_data->average_volt_adc;

				if(is_called_by_ticker ==1)	
				{
					new_vol_adc = new_vol_adc + DISCHARGE_SLEEP_OFFSET;
					pr_info("##### is_called_by_ticker = %d, base_weight = %d\n",
								is_called_by_ticker, base_weight);
					if(offset_with_new > 100) {
						base_weight = (base_weight * 28 / 10);
					} else {
						base_weight = (base_weight * 26 / 10);
					}
					pr_info("##### base_weight = %d\n", base_weight);
				}


				if(offset_with_new > 0) {
					// Battery was discharged by some reason. 
					// So, ADC will be calculated again

					if(offset_with_new > MAX_DIS_OFFSET_FOR_WEIGHT) {
						base_weight = base_weight 
							+ (base_weight*MAX_ADD_DIS_PERCENT_FOR_WEIGHT)/100;
						pbat_data->sum_total_adc -= (offset_with_new * base_weight);
					}
					else if(offset_with_new < MIN_DIS_OFFSET_FOR_WEIGHT) {
						base_weight = base_weight 
							+ (base_weight*MIN_ADD_DIS_PERCENT_FOR_WEIGHT)/100;
						pbat_data->sum_total_adc -= (offset_with_new * base_weight);
					}					
					else {
						base_weight = base_weight + (base_weight 
							* ( MAX_ADD_DIS_PERCENT_FOR_WEIGHT 
							- (((MAX_DIS_OFFSET_FOR_WEIGHT - offset_with_new)
							*(MAX_ADD_DIS_PERCENT_FOR_WEIGHT-MIN_ADD_DIS_PERCENT_FOR_WEIGHT))
							/(MAX_DIS_OFFSET_FOR_WEIGHT-MIN_DIS_OFFSET_FOR_WEIGHT))))/100;
						pbat_data->sum_total_adc -= (offset_with_new * base_weight);						
					}		
					
					num_multi = pbat_data->sum_total_adc / NORM_NUM; //minus

					if(num_multi < 0){
						new_vol_adc = pbat_data->average_volt_adc + num_multi;
						pbat_data->sum_total_adc = pbat_data->sum_total_adc - (num_multi*NORM_NUM);
					}
					else
						new_vol_adc = pbat_data->average_volt_adc;					
				}

				if(is_called_by_ticker ==0)			
				{
					pbat_data->current_volt_adc = new_vol_adc;
					pbat_data->sum_voltage_adc += new_vol_adc;
					pbat_data->sum_voltage_adc -= 
									pbat_data->voltage_adc[pbat_data->voltage_idx];
					pbat_data->voltage_adc[pbat_data->voltage_idx] = new_vol_adc;
				}
				else
				{
					int i;
					
					for(i=0 ; i < 16; i++)
					{				
						pbat_data->current_volt_adc = new_vol_adc;
						pbat_data->sum_voltage_adc += new_vol_adc;
						pbat_data->sum_voltage_adc -= 
										pbat_data->voltage_adc[pbat_data->voltage_idx];
						pbat_data->voltage_adc[pbat_data->voltage_idx] = new_vol_adc;	
						pbat_data->voltage_idx = (pbat_data->voltage_idx+1) % AVG_SIZE;
					}

					is_called_by_ticker=0;
				}
			}
		}
		else {
			u8 i = 0;
			u8 res_msb, res_lsb;
			int result=0,result_vol_adc=0,result_temp_adc=0;
			int X1, X0;
			int Y1, Y0 = FIRST_VOLTAGE_DROP_ADC;
			int X = C2K(pbat_data->average_temperature);

			d2153_reg_read(pbat->pd2153, D2153_GP_ID_4_REG, &res_msb);
			d2153_reg_read(pbat->pd2153, D2153_GP_ID_5_REG, &res_lsb);
			if(res_msb != 0 || res_lsb != 0) {
				result_temp_adc = (((res_msb&0xF) << 8) | (res_lsb & 0xFF));
			}

			result = result_temp_adc - pbat_data->average_temp_adc;

			if(result < 0)
				result=-result;
			
			if( result < 1000) //in case the difference of temp is under 10C.
			{				
				d2153_reg_read(pbat->pd2153, D2153_GP_ID_3_REG, &res_msb);
				d2153_reg_read(pbat->pd2153, D2153_GP_ID_2_REG, &res_lsb);
				if(res_msb != 0 || res_lsb != 0)
				{
					result_vol_adc = (((res_msb&0xF) << 8) | (res_lsb & 0xFF));
				}
			}
			
			//new_vol_adc += 80; //52.5mV -> 40mV
			if(C2K(pbat_data->average_temperature) <= BAT_LOW_LOW_TEMPERATURE) {
				new_vol_adc += (Y0 + 330);
			} else if(C2K(pbat_data->average_temperature) >= BAT_ROOM_TEMPERATURE) {
				new_vol_adc += Y0;
			} else {
				if(C2K(pbat_data->average_temperature) <= BAT_LOW_MID_TEMPERATURE) {
					Y1 = Y0 + 105;	Y0 = Y0 + 330;
					X0 = BAT_LOW_LOW_TEMPERATURE;
					X1 = BAT_LOW_MID_TEMPERATURE;
				} else if(C2K(pbat_data->average_temperature) <= BAT_LOW_TEMPERATURE) {
					Y1 = Y0 + 45;	Y0 = Y0 + 105;
					X0 = BAT_LOW_MID_TEMPERATURE;
					X1 = BAT_LOW_TEMPERATURE;
				} else {
					Y1 = Y0 + 15;	Y0 = Y0 + 45;
					X0 = BAT_LOW_TEMPERATURE;
					X1 = BAT_ROOM_LOW_TEMPERATURE;
				}
				new_vol_adc = new_vol_adc + Y0 
					+ ((X - X0) * Y1 - (X - X0) * Y0) / (X1 - X0);
				pr_info("### else Calculated new ADC is %4d \n", new_vol_adc);
			}

			// 1. read general_register MSB and LSB of voltage ADC which was stored at bootloader
			// 2. set to new_temp_adc from general register
			// 3. make 12Bits ADC raw data.
			d2153_reg_read(pbat->pd2153, D2153_GP_ID_0_REG, &res_msb);
			d2153_reg_read(pbat->pd2153, D2153_GP_ID_1_REG, &res_lsb);

			result = ((res_msb << 4) | (res_lsb & 0xF));

			// Conpensate charging current.
			if(pbat_data->is_charging) {
				union power_supply_propval is_cv_charging;	
								
				ps->get_property(ps, POWER_SUPPLY_PROP_CHARGE_STATUS, &is_cv_charging);
								
				is_cv_charging.intval= (is_cv_charging.intval & 0x4);
				
				pr_info("CV =%d offset_charging = %d index=%d\n",is_cv_charging.intval, offset_charging,charging_index);

				if(is_cv_charging.intval && new_vol_adc > (CV_START_ADC)) {

					offset=initialize_charge_up_cc[charging_index]
						+(initialize_charge_offset[charging_index]
						-initialize_charge_offset[charging_index]);						
					///185 + (x-12)
					
					if(new_vol_adc > 3600){ //4195mV
						new_vol_adc =new_vol_adc -80;
					}
					else {	
						//usb = adc - 185 -40 + 185(%)
						//wall = adc - (185+22) -80 + (185+22)(%)
						new_vol_adc =new_vol_adc -offset-80 +offset*((new_vol_adc - CV_START_ADC)
							/ ( 3600 - CV_START_ADC));		
					}
				}
				else 
				{
					offset=initialize_charge_up_cc[charging_index];
					// Charging CC					
					new_vol_adc -= offset;
				}

			}

			result=result_vol_adc - new_vol_adc;

			if(result < 0)
				result=-result;
			
			if(result < 200) //50mV
				new_vol_adc=(result_vol_adc*70+new_vol_adc*30)/100;
				
			for(i = 0; i < AVG_SIZE; i++) {
				pbat_data->voltage_adc[i] = new_vol_adc;
				pbat_data->sum_voltage_adc += new_vol_adc;
			}
			
			pbat_data->current_volt_adc = new_vol_adc;
			pbat->battery_data.volt_adc_init_done = TRUE;
		}

		pbat_data->origin_volt_adc = new_vol_orign;
		pbat_data->average_volt_adc = pbat_data->sum_voltage_adc >> AVG_SHIFT;
		pbat_data->voltage_idx = (pbat_data->voltage_idx+1) % AVG_SIZE;
		pbat_data->current_voltage = adc_to_vbat(pbat_data->current_volt_adc,
											 pbat_data->is_charging);
		pbat_data->average_voltage = adc_to_vbat(pbat_data->average_volt_adc,
											 pbat_data->is_charging);

		pr_info("# SOC = %3d.%d %%, Weight = %4d, ADC(oV) = %4d, ADC(aV) = %4d, OFST = %4d, Voltage = %4d mV, ADC(T) = %4d, ADC(VF) = %4d, Temp = %3d.%d \n",
					(pbat->battery_data.soc/10),
					(pbat->battery_data.soc%10),
					base_weight,
					pbat->battery_data.origin_volt_adc,
					pbat->battery_data.average_volt_adc,
					offset_with_new,
					pbat->battery_data.average_voltage, 
					pbat->battery_data.average_temp_adc,
					pbat->battery_data.vf_adc,
					(pbat->battery_data.average_temperature/10),
					(pbat->battery_data.average_temperature%10));
	}
	else {
		pr_err("%s. Voltage ADC read failure \n", __func__);
		ret = -EIO;
	}

	return ret;
}


/*
 * Name : d2153_read_temperature
 */
static int d2153_read_temperature(struct d2153_battery *pbat)
{
	u16 new_temp_adc = 0;
	int ret = 0;
	struct d2153_battery_data *pbat_data = &pbat->battery_data;

	/* Read temperature ADC
	 * Channel : D2153_ADC_TEMPERATURE_1 -> TEMP_BOARD
	 * Channel : D2153_ADC_TEMPERATURE_2 -> TEMP_RF
	 */

	ret = pbat->d2153_read_adc(pbat, D2153_ADC_TEMPERATURE_1);

	if(pbat_data->adc_res[D2153_ADC_TEMPERATURE_1].is_adc_eoc) {
		new_temp_adc = pbat_data->adc_res[D2153_ADC_TEMPERATURE_1].read_adc;


		pbat_data->current_temp_adc = new_temp_adc;

		if(pbat_data->temp_adc_init_done) {
			pbat_data->sum_temperature_adc += new_temp_adc;
			pbat_data->sum_temperature_adc -= 
						pbat_data->temperature_adc[pbat_data->temperature_idx];
			pbat_data->temperature_adc[pbat_data->temperature_idx] = new_temp_adc;
		}
		else {
			u8 i = 0;

			for(i = 0; i < AVG_SIZE; i++) {
				pbat_data->temperature_adc[i] = new_temp_adc;
				pbat_data->sum_temperature_adc += new_temp_adc;
			}

			pbat->battery_data.temp_adc_init_done = TRUE;
		}

		pbat_data->average_temp_adc =
								pbat_data->sum_temperature_adc >> AVG_SHIFT;
		pbat_data->temperature_idx = (pbat_data->temperature_idx+1) % AVG_SIZE;
		pbat_data->average_temperature = 
					degree_k2c(adc_to_degree_k(pbat_data->average_temp_adc));
		pbat_data->current_temperature = 
									degree_k2c(adc_to_degree_k(new_temp_adc)); 

	}
	else {
		pr_err("%s. Temperature ADC read failed \n", __func__);
		ret = -EIO;
	}

	return ret;
}


/******************************************************************************
    Interrupt Handler
******************************************************************************/
/* 
 * Name : d2153_battery_tbat2_handler
 */
static irqreturn_t d2153_battery_tbat2_handler(int irq, void *data)
{
	struct d2153_battery *pbat = (struct d2153_battery *)data;

	if(unlikely(!pbat)) {
		pr_err("%s. Invalid driver data\n", __func__);
		return -EINVAL;
	}

	//pr_warn("WARNING !!! Temperature 2\n"); 

	return IRQ_HANDLED;
}


/* 
 * Name : d2153_battery_vdd_low_handler
 */
static irqreturn_t d2153_battery_vdd_low_handler(int irq, void *data)
{
	struct d2153_battery *pbat = (struct d2153_battery *)data;

	if(unlikely(!pbat)) {
		pr_err("%s. Invalid driver data\n", __func__);
		return -EINVAL;
	}

	//pr_warn("WARNING !!! Low Battery... \n");
	

	return IRQ_HANDLED;
}


/* 
 * Name : d2153_battery_vdd_mon_handler
 */
static irqreturn_t d2153_battery_vdd_mon_handler(int irq, void *data)
{
	struct d2153_battery *pbat = (struct d2153_battery *)data;

	if(unlikely(!pbat)) {
		pr_err("%s. Invalid driver data\n", __func__);
		return -EINVAL;
	}

	//pr_warn("WARNING !!! Invalid Battery inserted. \n");
	

	return IRQ_HANDLED;
}


/* 
 * Name : d2153_battery_adceom_handler
 */
static irqreturn_t d2153_battery_adceom_handler(int irq, void *data)
{
	u8 read_msb, read_lsb, channel;
	int ret = 0;
	struct d2153_battery *pbat = (struct d2153_battery *)data;
	struct d2153 *d2153 = NULL;

	if(unlikely(!pbat)) {
		pr_err("%s. Invalid driver data\n", __func__);
		return -EINVAL;
	}

	d2153 = pbat->pd2153;
	
	/* A manual ADC has 12 bit resolution */
	ret = d2153_reg_read(d2153, D2153_ADC_RES_H_REG, &read_msb);
	ret |= d2153_reg_read(d2153, D2153_ADC_RES_L_REG, &read_lsb);
	ret |= d2153_reg_read(d2153, D2153_ADC_MAN_REG, &channel);
	
	channel = (channel & 0xF);
	
	switch(channel) {
		case D2153_MUXSEL_VBAT:
			channel = D2153_ADC_VOLTAGE;
			break;
		case D2153_MUXSEL_TEMP1:
			channel = D2153_ADC_TEMPERATURE_1;
			break;
		case D2153_MUXSEL_TEMP2:
			channel = D2153_ADC_TEMPERATURE_2;
			break;
		case D2153_MUXSEL_VF:
			channel = D2153_ADC_VF;
			break;
		case D2153_MUXSEL_TJUNC:
			channel = D2153_ADC_TJUNC;
			break;
		default :
			pr_err("%s. Invalid channel(%d) \n", __func__, channel);
			goto out;
	}

	pbat->battery_data.adc_res[channel].is_adc_eoc = TRUE;
	pbat->battery_data.adc_res[channel].read_adc = 
						((read_msb << 4) | (read_lsb & ADC_RES_MASK_LSB));

out:
	//pr_info("%s. Manual ADC (%d) = %d\n", 
	//			__func__, channel,
	//			pbat->battery_data.adc_res[channel].read_adc);

	return IRQ_HANDLED;
}


/* 
 * Name : d2153_sleep_monitor
 */
static void d2153_sleep_monitor(struct d2153_battery *pbat)
{
	schedule_delayed_work(&pbat->sleep_monitor_work, 0);

	return;
}


/* 
 * Name : d2153_monitor_voltage_work
 */

int d2153_battery_read_status(int type)
{
	int val=0;

	if(u2_get_board_rev() <= 4) {
		dlg_info("%s is called on old Board revision. error\n", __func__);
		return 0;
	}
	switch(type){
		case D2153_BATTERY_SOC:
			val=d2153_get_soc(gbat);
			val=(val+5)/10;
			break;

		case D2153_BATTERY_CUR_VOLTAGE:
			val=gbat->battery_data.current_voltage;
			break;
 
		case D2153_BATTERY_TEMP_ADC:
			val=gbat->battery_data.current_temp_adc;
			break;
		case D2153_BATTERY_SLEEP_MONITOR:
			is_called_by_ticker=1;
			wake_lock_timeout(&gbat->battery_data.sleep_monitor_wakeup, 
									D2153_SLEEP_MONITOR_WAKELOCK_TIME);
			cancel_delayed_work_sync(&gbat->monitor_temp_work);
			cancel_delayed_work_sync(&gbat->monitor_volt_work);
			schedule_delayed_work(&gbat->monitor_temp_work, 0);
			schedule_delayed_work(&gbat->monitor_volt_work, 0);
			break;
	}
	
	return val;
}
EXPORT_SYMBOL(d2153_battery_read_status);

static void d2153_monitor_voltage_work(struct work_struct *work)
{
	int ret=0; 
	struct d2153_battery *pbat = container_of(work, struct d2153_battery, monitor_volt_work.work);
	struct d2153_battery_data *pbat_data = &pbat->battery_data;
	struct power_supply *ps;
	
	if(unlikely(!pbat || !pbat_data)) {
		pr_err("%s. Invalid driver data\n", __func__);
		goto err_adc_read;
	}
#ifdef CONFIG_SEC_CHARGING_FEATURE
	ps = power_supply_get_by_name(spa_power_pdata.charger_name);
#else
	ps = NULL;
#endif
	if(ps == NULL){
		pr_info("spa is not registered yet !!!");
		schedule_delayed_work(&pbat->monitor_volt_work, D2153_VOLTAGE_MONITOR_START);
		return;
	}
	
	ret = d2153_read_voltage(pbat,ps);
	if(ret < 0)
	{
		pr_err("%s. Read voltage ADC failure\n", __func__);
		goto err_adc_read;
	}

	if(pbat_data->is_charging ==0) {
		schedule_delayed_work(&pbat->monitor_volt_work, D2153_VOLTAGE_MONITOR_NORMAL);
	}
	else {
		schedule_delayed_work(&pbat->monitor_volt_work, D2153_VOLTAGE_MONITOR_FAST);
	}
	return;

err_adc_read:
	schedule_delayed_work(&pbat->monitor_volt_work, D2153_VOLTAGE_MONITOR_START);
	return;
}


static void d2153_monitor_temperature_work(struct work_struct *work)
{
	struct d2153_battery *pbat = container_of(work, struct d2153_battery, monitor_temp_work.work);
	int ret = 0;

	ret = d2153_read_temperature(pbat);
	if(ret < 0) {
		pr_err("%s. Failed to read_temperature\n", __func__);
		schedule_delayed_work(&pbat->monitor_temp_work, D2153_TEMPERATURE_MONITOR_FAST);
		return;
	}

	if(pbat->battery_data.temp_adc_init_done) {
		schedule_delayed_work(&pbat->monitor_temp_work, D2153_TEMPERATURE_MONITOR_NORMAL);
	}
	else {
		schedule_delayed_work(&pbat->monitor_temp_work, D2153_TEMPERATURE_MONITOR_FAST);
	}

	return ;
}


#if 0
/* 
 * Name : d2153_info_notify_work
 */
static void d2153_info_notify_work(struct work_struct *work)
{
	struct d2153_battery *pbat = container_of(work, 
												struct d2153_battery, 
												info_notify_work.work);

	power_supply_changed(&pbat->battery);	
	schedule_delayed_work(&pbat->info_notify_work, D2153_NOTIFY_INTERVAL);
}


/* 
 * Name : d2153_charge_timer_work
 */
static void d2153_charge_timer_work(struct work_struct *work)
{
	struct d2153_battery *pbat = container_of(work, struct d2153_battery, charge_timer_work.work);

	pr_info("%s. Start\n", __func__);

	d2153_set_end_of_charge(pbat, BAT_END_OF_CHARGE_BY_TIMER);
	d2153_set_battery_status(pbat, POWER_SUPPLY_STATUS_FULL);
	d2153_stop_charge(pbat, BAT_END_OF_CHARGE_BY_TIMER);

	pbat->recharge_start_timer.expires = jiffies + BAT_RECHARGE_CHECK_TIMER_30SEC;
	add_timer(&pbat->recharge_start_timer);

	return;
}


/* 
 * Name : d2153_recharge_start_timer_work
 */
static void d2153_recharge_start_timer_work(struct work_struct *work)
{
	u8 end_of_charge = 0;	
	struct d2153_battery *pbat = container_of(work, 
												struct d2153_battery, 
												recharge_start_timer_work.work);

	pr_info("%s. Start\n", __func__);

	if(d2153_check_end_of_charge(pbat, BAT_END_OF_CHARGE_BY_TIMER) == 0)
	{
		end_of_charge = d2153_clear_end_of_charge(pbat, BAT_END_OF_CHARGE_BY_TIMER);
		if(end_of_charge == BAT_END_OF_CHARGE_NONE)
		{
			if((d2153_get_battery_status(pbat) == POWER_SUPPLY_STATUS_FULL) 
				&& (d2153_get_average_voltage(pbat) < BAT_CHARGING_RESTART_VOLTAGE))
			{
				pr_info("%s. Restart charge. Voltage is lower than %04d mV\n", 
									__func__, BAT_CHARGING_RESTART_VOLTAGE);
				d2153_start_charge(pbat, BAT_CHARGE_RESTART_TIMER);
			}
			else
			{
				pr_info("%s. set BAT_END_OF_CHARGE_BY_FULL. Voltage is higher than %04d mV\n", 
								__func__, BAT_CHARGING_RESTART_VOLTAGE);
				d2153_set_end_of_charge(pbat, BAT_END_OF_CHARGE_BY_FULL);	
			}
		}
		else {
			pr_info("%s. Can't restart charge. The reason why is %d\n", __func__, end_of_charge);	
		}
	}
	else {
		pr_info("%s. SPA_END_OF_CHARGE_BY_TIMER had been cleared by other reason\n", __func__); 
	}
}


/* 
 * Name : d2153_sleep_monitor_work
 */
static void d2153_sleep_monitor_work(struct work_struct *work)
{
	struct d2153_battery *pbat = container_of(work, struct d2153_battery, 
												sleep_monitor_work.work);

	is_called_by_ticker = 1;
	wake_lock_timeout(&pbat->battery_data.sleep_monitor_wakeup, 
									D2153_SLEEP_MONITOR_WAKELOCK_TIME);
	pr_info("%s. Start. Ticker was set to 1\n", __func__);
	if(schedule_delayed_work(&pbat->monitor_volt_work, 0) == 0) {
		cancel_delayed_work_sync(&pbat->monitor_volt_work);
		schedule_delayed_work(&pbat->monitor_volt_work, 0);
	}
	if(schedule_delayed_work(&pbat->monitor_temp_work, 0) == 0) {
		cancel_delayed_work_sync(&pbat->monitor_temp_work);
		schedule_delayed_work(&pbat->monitor_temp_work, 0);
	}
	if(schedule_delayed_work(&pbat->info_notify_work, 0) == 0) {
		cancel_delayed_work_sync(&pbat->info_notify_work);
		schedule_delayed_work(&pbat->info_notify_work, 0);
	}

	return ;	
}
#endif

void d2153_battery_start(void)
{
	if(u2_get_board_rev() <= 4) {
		dlg_info("%s is called on old Board revision. error\n", __func__);
		return;
	}
	schedule_delayed_work(&gbat->monitor_volt_work, 0);
}
EXPORT_SYMBOL_GPL(d2153_battery_start);

/* 
 * Name : d2153_battery_init
 */
static void d2153_battery_data_init(struct d2153_battery *pbat)
{
	struct d2153_battery_data *pbat_data = &pbat->battery_data;

	if(unlikely(!pbat_data)) {
		pr_err("%s. Invalid platform data\n", __func__);
		return;
	}

	pbat->adc_mode = D2153_ADC_MODE_MAX;

	pbat_data->sum_total_adc = 0;
	pbat_data->vdd_hwmon_level = 0;
	pbat_data->volt_adc_init_done = FALSE;
	pbat_data->temp_adc_init_done = FALSE;
	pbat_data->battery_present = TRUE;
	wake_lock_init(&pbat_data->sleep_monitor_wakeup, WAKE_LOCK_SUSPEND, "sleep_monitor");		

	return;
}


/* 
 * Name : d2153_battery_probe
 */
static __devinit int d2153_battery_probe(struct platform_device *pdev)
{
	struct d2153 *d2153 = platform_get_drvdata(pdev);
	struct d2153_battery *pbat = &d2153->batt;
	int ret;

	pr_info("Start %s\n", __func__);

	if(unlikely(!d2153 || !pbat)) {
		pr_err("%s. Invalid platform data\n", __func__);
		return -EINVAL;
	}

	r8a73734_hwlock_pmic_d2153 = hwspin_lock_request_specific(SMGP000);
	if (r8a73734_hwlock_pmic_d2153 == NULL) {
		pr_err("%s Unable to register hw spinlock for pmic driver\n",__func__);
		return -EIO;
	}
	
	gbat = pbat;
	pbat->pd2153 = d2153;

	// Initialize a resource locking
	mutex_init(&pbat->lock);
	mutex_init(&pbat->api_lock);
	mutex_init(&pbat->meoc_lock);

	// Store a driver data structure to platform.
	platform_set_drvdata(pdev, pbat);

	d2153_battery_data_init(pbat);
	d2153_set_adc_mode(pbat, D2153_ADC_IN_AUTO);
	// Disable 50uA current source in Manual ctrl register
	d2153_reg_write(d2153, D2153_ADC_MAN_REG, 0x00);

	INIT_DELAYED_WORK(&pbat->monitor_volt_work, d2153_monitor_voltage_work);
	INIT_DELAYED_WORK(&pbat->monitor_temp_work, d2153_monitor_temperature_work);
#if 0	
	INIT_DELAYED_WORK(&pbat->info_notify_work, d2153_info_notify_work);
	INIT_DELAYED_WORK(&pbat->charge_timer_work, d2153_charge_timer_work);
	INIT_DELAYED_WORK(&pbat->recharge_start_timer_work, d2153_recharge_start_timer_work);
	INIT_DELAYED_WORK(&pbat->sleep_monitor_work, d2153_sleep_monitor_work);
#endif	

	// Start schedule of dealyed work for temperature.
	schedule_delayed_work(&pbat->monitor_temp_work, 0);

	device_init_wakeup(&pdev->dev, 1);	

	/* Init thread to handle modem reset */
	init_waitqueue_head(&d2153_modem_reset_event);
	d2153_modem_reset_thread = kthread_run(d2153_modem_thread,
					NULL, "d2153_modem_reset_thread");
	if (NULL == d2153_modem_reset_thread) {
		ret = -ENOMEM;
		dlg_err("%s:%d d2153_modem_reset_thread failed\n",\
				__func__, __LINE__);
		goto err_default;
	}
	
	pr_info("%s. End...\n", __func__);

	return 0;

err_default:
	kfree(pbat);

	return ret;

}


/*
 * Name : d2153_battery_suspend
 */
static int d2153_battery_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct d2153_battery *pbat = platform_get_drvdata(pdev);
	struct d2153 *d2153 = pbat->pd2153;
//	int ret;

	pr_info("%s. Enter\n", __func__);

	if(unlikely(!pbat || !d2153)) {
		pr_err("%s. Invalid parameter\n", __func__);
		return -EINVAL;
	}

	//ret = d2153_reg_write(d2153, D2153_BUCK_A_REG, 0x9A);//force pwm mode
	
//	cancel_delayed_work(&pbat->info_notify_work);
	cancel_delayed_work(&pbat->monitor_temp_work);
	cancel_delayed_work(&pbat->monitor_volt_work);

	pr_info("%s. Leave\n", __func__);
	
	return 0;
}


/*
 * Name : d2153_battery_resume
 */
static int d2153_battery_resume(struct platform_device *pdev, pm_message_t state)
{
	struct d2153_battery *pbat = platform_get_drvdata(pdev);
	struct d2153 *d2153 = pbat->pd2153;
//	int ret;

	pr_info("%s. Enter\n", __func__);

	if(unlikely(!pbat || !d2153)) {
		pr_err("%s. Invalid parameter\n", __func__);
		return -EINVAL;
	}

	//ret = d2153_reg_write(d2153, D2153_BUCK_A_REG, 0x99); // auto mode
	
	// Start schedule of dealyed work for monitoring voltage and temperature.
	if(!is_called_by_ticker) {
//		wake_lock_timeout(&pbat->battery_data.sleep_monitor_wakeup, 
//										D2153_SLEEP_MONITOR_WAKELOCK_TIME);
		schedule_delayed_work(&pbat->monitor_temp_work, 0);
		schedule_delayed_work(&pbat->monitor_volt_work, 0);
	}

	pr_info("%s. Leave\n", __func__);

	return 0;
}


/*
 * Name : d2153_battery_remove
 */
static __devexit int d2153_battery_remove(struct platform_device *pdev)
{
	struct d2153_battery *pbat = platform_get_drvdata(pdev);
	struct d2153 *d2153 = pbat->pd2153;
//	int i;

	if(unlikely(!pbat || !d2153)) {
		pr_err("%s. Invalid parameter\n", __func__);
		return -EINVAL;
	}

	// Free IRQ
#ifdef D2153_REG_EOM_IRQ
	d2153_free_irq(d2153, D2153_IRQ_EADCEOM);
#endif /* D2153_REG_EOM_IRQ */
#ifdef D2153_REG_VDD_MON_IRQ
	d2153_free_irq(d2153, D2153_IRQ_EVDD_MON);
#endif /* D2153_REG_VDD_MON_IRQ */
#ifdef D2153_REG_VDD_LOW_IRQ
	d2153_free_irq(d2153, D2153_IRQ_EVDD_LOW);
#endif /* D2153_REG_VDD_LOW_IRQ */
#ifdef D2153_REG_TBAT2_IRQ
	d2153_free_irq(d2153, D2153_IRQ_ETBAT2);
#endif /* D2153_REG_TBAT2_IRQ */

	hwspin_lock_free(r8a73734_hwlock_pmic_d2153);

	return 0;
}

static struct platform_driver d2153_battery_driver = {
	.probe    = d2153_battery_probe,
	.suspend  = d2153_battery_suspend,
	.resume   = d2153_battery_resume,
	.remove   = d2153_battery_remove,
	.driver   = {
		.name  = "d2153-battery",
		.owner = THIS_MODULE,
    },
};

static int __init d2153_battery_init(void)
{
	if(u2_get_board_rev() <= 4) {
		dlg_info("%s is called on old Board revision. error\n", __func__);
		return 0;
	}
	printk(d2153_battery_banner);
	return platform_driver_register(&d2153_battery_driver);
}
//module_init(d2153_battery_init);
subsys_initcall(d2153_battery_init);



static void __exit d2153_battery_exit(void)
{
	if(u2_get_board_rev() <= 4) {
		dlg_info("%s is called on old Board revision. error\n", __func__);
		return;
	}
	flush_scheduled_work();
	platform_driver_unregister(&d2153_battery_driver);
}
module_exit(d2153_battery_exit);


MODULE_AUTHOR("Dialog Semiconductor Ltd. < eric.jeong@diasemi.com >");
MODULE_DESCRIPTION("Battery driver for the Dialog D2153 PMIC");
MODULE_LICENSE("GPL");
MODULE_ALIAS("Power supply : d2153-battery");

