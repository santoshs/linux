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
#ifdef CONFIG_D2153_HW_TIMER
#include <linux/irq.h>
#include <mach/irqs.h>
#endif

#include <linux/io.h>
#include <mach/common.h>
#include <linux/jiffies.h>
#include <linux/spa_power.h>
#include <linux/spa_agent.h>
#include "linux/err.h"

#include <linux/d2153/core.h>
#include <linux/d2153/d2153_battery.h>
#include <linux/d2153/d2153_reg.h>

#ifdef CONFIG_ARCH_R8A7373
#include <mach/pm.h>
#endif /* CONFIG_ARCH_R8A7373 */

static const char __initdata d2153_battery_banner[] = \
	"D2153 Battery, (c) 2012 Dialog Semiconductor Ltd.\n";

/***************************************************************************
 Pre-definition
***************************************************************************/
#define FALSE								(0)
#define TRUE								(1)

#define ADC_RES_MASK_LSB					(0x0F)
#define ADC_RES_MASK_MSB					(0xF0)

#if USED_BATTERY_CAPACITY == BAT_CAPACITY_1800MA
#define ADC_VAL_100_PERCENT            		3645   // About 4280mV
#define CV_START_ADC            			3275   // About 4100mV
#define MAX_FULL_CHARGED_ADC				3768   // About 4340mV
#define ORIGN_CV_START_ADC					3686   // About 4300mV
#define ORIGN_FULL_CHARGED_ADC				3780   // About 4345mV

#define D2153_BAT_CHG_FRST_FULL_LVL         4310   // About EOC 160mA
#define D2153_BAT_CHG_BACK_FULL_LVL         4347   // About EOC 50mA

#define FIRST_VOLTAGE_DROP_ADC  			121

#define MAX_ADD_DIS_PERCENT_FOR_WEIGHT2   	27    // 30    // 35
#define MAX_ADD_DIS_PERCENT_FOR_WEIGHT1   	21    // 23    // 26 // 23
#define MAX_ADD_DIS_PERCENT_FOR_WEIGHT0_5   18    // 22 // 19  // 15
#define MAX_ADD_DIS_PERCENT_FOR_WEIGHT   	14    // 18 15
#define MIN_ADD_DIS_PERCENT_FOR_WEIGHT  	(-30)

#define LAST_CHARGING_WEIGHT      			380   // 900

#elif USED_BATTERY_CAPACITY == BAT_CAPACITY_2100MA
#define ADC_VAL_100_PERCENT            		3645   // About 4280mV
#define CV_START_ADC            			3275   // About 4100mV
#define MAX_FULL_CHARGED_ADC				3768   // About 4340mV
#define ORIGN_CV_START_ADC					3686   // About 4300mV
#define ORIGN_FULL_CHARGED_ADC				3780   // About 4345mV

#define D2153_BAT_CHG_FRST_FULL_LVL         4310   // About EOC 160mA
#define D2153_BAT_CHG_BACK_FULL_LVL         4340   // About EOC 50mA

#define FIRST_VOLTAGE_DROP_ADC  			121

#define MAX_ADD_DIS_PERCENT_FOR_WEIGHT2   	27    // 30    // 35
#define MAX_ADD_DIS_PERCENT_FOR_WEIGHT1   	21    // 23    // 26 // 23
#define MAX_ADD_DIS_PERCENT_FOR_WEIGHT0_5   18    // 22 // 19  // 15
#define MAX_ADD_DIS_PERCENT_FOR_WEIGHT   	14    // 18 15
#define MIN_ADD_DIS_PERCENT_FOR_WEIGHT  	(-30) // (-20) // (-63) // (-40)

#define LAST_CHARGING_WEIGHT      			450   // 900

#else
#define ADC_VAL_100_PERCENT            		3445
#define CV_START_ADC            			3338
#define MAX_FULL_CHARGED_ADC				3470
#define ORIGN_CV_START_ADC					3320
#define ORIGN_FULL_CHARGED_ADC				3480   // About 4345mV

#define D2153_BAT_CHG_FRST_FULL_LVL         4160   // About EOC 160mA
#define D2153_BAT_CHG_BACK_FULL_LVL         4185   // About EOC 60mA

#define FIRST_VOLTAGE_DROP_ADC  			165

#define MAX_ADD_DIS_PERCENT_FOR_WEIGHT2   	27    // 30    // 35
#define MAX_ADD_DIS_PERCENT_FOR_WEIGHT1   	21    // 23    // 26 // 23
#define MAX_ADD_DIS_PERCENT_FOR_WEIGHT0_5   18    // 22 // 19  // 15
#define MAX_ADD_DIS_PERCENT_FOR_WEIGHT   	14    // 18 15
#define MIN_ADD_DIS_PERCENT_FOR_WEIGHT  	(-30)

#define LAST_CHARGING_WEIGHT      			450   // 900

#endif /* USED_BATTERY_CAPACITY == BAT_CAPACITY_????MA */

#define FULL_CAPACITY						1000

#define NORM_NUM                			10000
#define MAX_WEIGHT							10000
#define MAX_DIS_OFFSET_FOR_WEIGHT2			300
#define MAX_DIS_OFFSET_FOR_WEIGHT1   		200
#define MAX_DIS_OFFSET_FOR_WEIGHT0_5   		150
#define MAX_DIS_OFFSET_FOR_WEIGHT			100
#define MIN_DIS_OFFSET_FOR_WEIGHT   		30

#define DISCHARGE_SLEEP_OFFSET              55    // 45
#define LAST_VOL_UP_PERCENT                 75

/* Static Function Prototype */
/* static void d2153_external_event_handler(int category, int event); */
static int  d2153_read_adc_in_auto(struct d2153_battery *pbat, adc_channel channel);
static int  d2153_read_adc_in_manual(struct d2153_battery *pbat, adc_channel channel);

static struct d2153_battery *gbat = NULL;
#ifdef CONFIG_D2153_HW_TIMER
static struct timeval suspend_time = {0, 0};
static struct timeval resume_time = {0, 0};
#endif

static u8  is_called_by_ticker = 0;
static u16 ACT_4P2V_ADC = 0;
static u16 ACT_3P4V_ADC = 0;

/* This array is for setting ADC_CONT register about each channel.*/
static struct adc_cont_in_auto adc_cont_inven[D2153_ADC_CHANNEL_MAX - 1] = {
	/* VBAT_S channel */
	[D2153_ADC_VOLTAGE] = {
		.adc_preset_val = 0,
		.adc_cont_val = (D2153_ADC_AUTO_EN_MASK | D2153_ADC_MODE_MASK
							| D2153_AUTO_VBAT_EN_MASK),
		.adc_msb_res = D2153_VDD_RES_VBAT_RES_REG,
		.adc_lsb_res = D2153_ADC_RES_AUTO1_REG,
		.adc_lsb_mask = ADC_RES_MASK_LSB,
	},
	/* TEMP_1 channel */
	[D2153_ADC_TEMPERATURE_1] = {
		.adc_preset_val = D2153_TEMP1_ISRC_EN_MASK,
		.adc_cont_val = (D2153_ADC_AUTO_EN_MASK | D2153_ADC_MODE_MASK),
		.adc_msb_res = D2153_TBAT1_RES_TEMP1_RES_REG,
		.adc_lsb_res = D2153_ADC_RES_AUTO1_REG,
		.adc_lsb_mask = ADC_RES_MASK_MSB,
	},
	/* TEMP_2 channel */
	[D2153_ADC_TEMPERATURE_2] = {
		.adc_preset_val =  D2153_TEMP2_ISRC_EN_MASK,
		.adc_cont_val = (D2153_ADC_AUTO_EN_MASK | D2153_ADC_MODE_MASK),
		.adc_msb_res = D2153_TBAT2_RES_TEMP2_RES_REG,
		.adc_lsb_res = D2153_ADC_RES_AUTO3_REG,
		.adc_lsb_mask = ADC_RES_MASK_LSB,
	},
	/* VF channel */
	[D2153_ADC_VF] = {
		.adc_preset_val = D2153_AD4_ISRC_ENVF_ISRC_EN_MASK,
		.adc_cont_val = (D2153_ADC_AUTO_EN_MASK | D2153_ADC_MODE_MASK
							| D2153_AUTO_VF_EN_MASK),
		.adc_msb_res = D2153_ADCIN4_RES_VF_RES_REG,
		.adc_lsb_res = D2153_ADC_RES_AUTO2_REG,
		.adc_lsb_mask = ADC_RES_MASK_LSB,
	},
	/* AIN channel */
	[D2153_ADC_AIN] = {
		.adc_preset_val = 0,
		.adc_cont_val = (D2153_ADC_AUTO_EN_MASK | D2153_ADC_MODE_MASK
							| D2153_AUTO_AIN_EN_MASK),
		.adc_msb_res = D2153_ADCIN5_RES_AIN_RES_REG,
		.adc_lsb_res = D2153_ADC_RES_AUTO2_REG,
		.adc_lsb_mask = ADC_RES_MASK_MSB
	},
};

/* Table for compensation of charge offset in CC mode */
static u16 initialize_charge_offset[] = {
	 18,  22,  26,  30,  34,   36,   40,   44,
};

static u16 initialize_charge_up_cc[] = {
	45
};

/* LUT for NCP15XW223 thermistor with 10uA current source selected */
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

static struct adc2vbat_lookuptbl adc2vbat_lut = {
	.adc	 = {1843, 1946, 2148, 2253, 2458, 2662, 2867, 2683, 3072, 3482,}, // ADC-12 input value
	.offset  = {   0,	 0,    0,	 0,    0,	 0,    0,	 0,    0,    0,}, // charging mode ADC offset
	.vbat	 = {3400, 3450, 3500, 3600, 3700, 3800, 3900, 4000, 4100, 4200,}, // VBAT (mV)
};

#if USED_BATTERY_CAPACITY == BAT_CAPACITY_1800MA
static struct adc2soc_lookuptbl adc2soc_lut = {
	.adc_ht  = {1843, 1906, 2056, 2213, 2310, 2396, 2563, 2627, 2688, 2762, 2920, 3069, 3249, 3458, ADC_VAL_100_PERCENT,}, // ADC input @ high temp
	.adc_rt  = {1843, 1906, 2056, 2213, 2310, 2396, 2563, 2627, 2688, 2762, 2920, 3069, 3249, 3458, ADC_VAL_100_PERCENT,}, // ADC input @ room temp
	.adc_rlt = {1843, 1906, 2056, 2213, 2310, 2396, 2563, 2627, 2688, 2762, 2920, 3069, 3249, 3458, ADC_VAL_100_PERCENT,}, // ADC input @ low temp(0)
	.adc_lt  = {1843, 1906, 2056, 2213, 2310, 2396, 2563, 2627, 2688, 2762, 2920, 3069, 3249, 3458, ADC_VAL_100_PERCENT,}, // ADC input @ low temp(0)
	.adc_lmt = {1843, 1906, 2056, 2213, 2310, 2396, 2563, 2627, 2688, 2762, 2920, 3069, 3249, 3458, ADC_VAL_100_PERCENT,}, // ADC input @ low mid temp(-10)
	.adc_llt = {1843, 1906, 2056, 2213, 2310, 2396, 2563, 2627, 2688, 2762, 2920, 3069, 3249, 3458, ADC_VAL_100_PERCENT,}, // ADC input @ low low temp(-20)
	.soc	 = {   0,	10,   30,	50,   70,  100,  200,  300,  400,  500,  600,  700,  800,  900, 1000,}, // SoC in %
};

//Discharging Weight(Room/Low/low low)          //     0,    1,     3,    5,    7,   10,   20,   30,   40,   50,   60,   70,   80,   90,  100
static u16 adc_weight_section_discharge[]       = {14100, 9100,  7585, 2418, 1235,  375,  101,   98,  142,  356,  355,  514,  769, 1228, 2495};
static u16 adc_weight_section_discharge_rlt[]   = {14100, 9100,  8985, 1585,  525,  352,  132,  119,  165,  375,  355,  514,  769, 1228, 2495};
static u16 adc_weight_section_discharge_lt[]    = {14100, 9100,  8985, 1585,  525,  352,  132,  119,  165,  375,  355,  514,  769, 1228, 2495};
static u16 adc_weight_section_discharge_lmt[]   = {14100, 9100,  8985, 1585,  525,  352,  132,  119,  165,  375,  355,  514,  769, 1228, 2495};
static u16 adc_weight_section_discharge_llt[]   = {19100, 9400, 10985, 1985,  480,  337,  132,  119,  165,  375,  355,  514,  769, 1228, 2495};

//Charging Weight(Room/Low/low low)             //     0,    1,     3,    5,    7,   10,   20,   30,   40,   50,   60,   70,   80,   90, 100
static u16 adc_weight_section_charge[]          = { 9700,  734,   585,  453,  321,  248,   95,   93,  105,  223,  219,  265,  281,  288, LAST_CHARGING_WEIGHT};
static u16 adc_weight_section_charge_rlt[]		= { 9700,  734,   488,  385,  225,  225,   95,   93,  105,  218,  216,  259,  281,  303, LAST_CHARGING_WEIGHT};
static u16 adc_weight_section_charge_lt[]		= { 9700,  734,   488,  385,  225,  225,   95,   93,  105,  218,  216,  259,  281,  303, LAST_CHARGING_WEIGHT};
static u16 adc_weight_section_charge_lmt[]		= { 9700,  734,   488,  385,  225,  225,   95,   93,  105,  218,  216,  259,  281,  303, LAST_CHARGING_WEIGHT};
static u16 adc_weight_section_charge_llt[]		= { 9700,  734,   488,  385,  225,  225,   95,   93,  105,  218,  216,  259,  281,  303, LAST_CHARGING_WEIGHT};

#elif USED_BATTERY_CAPACITY == BAT_CAPACITY_2100MA
static struct adc2soc_lookuptbl adc2soc_lut = {
	.adc_ht  = {1800, 1864, 1980, 2081, 2171, 2259, 2423, 2468, 2515, 2607, 2744, 2937, 3162, 3402, ADC_VAL_100_PERCENT,}, // ADC input @ high temp
	.adc_rt  = {1800, 1864, 1980, 2081, 2171, 2259, 2423, 2468, 2515, 2607, 2744, 2937, 3162, 3402, ADC_VAL_100_PERCENT,}, // ADC input @ room temp
	.adc_rlt = {1800, 1864, 1980, 2081, 2171, 2259, 2423, 2468, 2515, 2607, 2744, 2937, 3162, 3402, ADC_VAL_100_PERCENT,}, // ADC input @ low temp(0)
	.adc_lt  = {1800, 1864, 1980, 2081, 2171, 2259, 2423, 2468, 2515, 2607, 2744, 2937, 3162, 3402, ADC_VAL_100_PERCENT,}, // ADC input @ low temp(0)
	.adc_lmt = {1800, 1864, 1980, 2081, 2171, 2259, 2423, 2468, 2515, 2607, 2744, 2937, 3162, 3402, ADC_VAL_100_PERCENT,}, // ADC input @ low mid temp(-10)
	.adc_llt = {1800, 1864, 1980, 2081, 2171, 2259, 2423, 2468, 2515, 2607, 2744, 2937, 3162, 3402, ADC_VAL_100_PERCENT,}, // ADC input @ low low temp(-20)
	.soc	 = {   0,	10,   30,	50,   70,  100,  200,  300,  400,  500,  600,  700,  800,  900, 1000,}, // SoC in %
};

//Discharging Weight(Room/Low/low low)          //     0,    1,     3,    5,    7,   10,   20,   30,   40,   50,   60,   70,   80,   90,  100
static u16 adc_weight_section_discharge[]       = {14100, 4800,  3385,  965,  393,  425,  471,  485,  638,  683,  793,  885, 1002, 1328, 2495};
static u16 adc_weight_section_discharge_rlt[]   = {14100, 9100,  8985, 1585,  525,  352,  132,  119,  165,  375,  355,  514,  769, 1228, 2495};
static u16 adc_weight_section_discharge_lt[]    = {14100, 9100,  8985, 1585,  525,  352,  132,  119,  165,  375,  355,  514,  769, 1228, 2495};
static u16 adc_weight_section_discharge_lmt[]   = {14100, 9100,  8985, 1585,  525,  352,  132,  119,  165,  375,  355,  514,  769, 1228, 2495};
static u16 adc_weight_section_discharge_llt[]   = {19100, 9400, 10985, 1985,  480,  337,  132,  119,  165,  375,  355,  514,  769, 1228, 2495};

//Charging Weight(Room/Low/low low)             //     0,    1,     3,    5,    7,   10,   20,   30,   40,   50,   60,   70,   80,   90, 100
static u16 adc_weight_section_charge[]          = { 9700,  734,   473,  358,  215,  138,   56,   55,   96,  184,  242,  287,  299,  298, LAST_CHARGING_WEIGHT};
static u16 adc_weight_section_charge_rlt[]		= { 9700,  734,   488,  385,  225,  225,   95,   93,  105,  218,  216,  259,  281,  303, LAST_CHARGING_WEIGHT};
static u16 adc_weight_section_charge_lt[]		= { 9700,  734,   488,  385,  225,  225,   95,   93,  105,  218,  216,  259,  281,  303, LAST_CHARGING_WEIGHT};
static u16 adc_weight_section_charge_lmt[]		= { 9700,  734,   488,  385,  225,  225,   95,   93,  105,  218,  216,  259,  281,  303, LAST_CHARGING_WEIGHT};
static u16 adc_weight_section_charge_llt[]		= { 9700,  734,   488,  385,  225,  225,   95,   93,  105,  218,  216,  259,  281,  303, LAST_CHARGING_WEIGHT};

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
static u16 adc_weight_section_discharge_llt[]   = {2730, 1840,  710,  300,   70,   62,   55,   51,   63,   71,   73,   79,   85,  630};

//Charging Weight(Room/Low/low low)             //    0,    1,    3,    5,   10,   20,   30,   40,   50,   60,   70,   80,   90,  100
static u16 adc_weight_section_charge[]          = {7000, 5000, 2000,  500,  213,  130,   82,   86,  157,  263,  280,  310,  340,  600};
static u16 adc_weight_section_charge_rlt[]      = {7000, 5000, 2000,  500,  213,  130,   82,   86,  157,  263,  280,  310,  340,  600};
static u16 adc_weight_section_charge_lt[]       = {7000, 5000, 2000,  500,  213,  130,   82,   86,  157,  263,  280,  310,  340,  600};
static u16 adc_weight_section_charge_lmt[]      = {7000, 5000, 2000,  500,  213,  130,   82,   86,  157,  263,  280,  310,  340,  600};
static u16 adc_weight_section_charge_llt[]      = {7000, 5000, 2000,  500,  213,  130,   82,   86,  157,  263,  280,  310,  340,  600};

#endif /* USED_BATTERY_CAPACITY == BAT_CAPACITY_????MA */


static u16 adc2soc_lut_length = (u16)sizeof(adc2soc_lut.soc)/sizeof(u16);
static u16 adc2vbat_lut_length = (u16)sizeof(adc2vbat_lut.offset)/sizeof(u16);

#ifdef CONFIG_D2153_DEBUG_FEATURE
unsigned int d2153_attr_idx=0;

static ssize_t d2153_battery_attrs_show(struct device *pdev, struct device_attribute *attr, char *buf);

enum {
	D2153_PROP_SOC_LUT = 0,
	D2153_PROP_DISCHG_WEIGHT,
	D2153_PROP_CHG_WEIGHT,
	D2153_PROP_WEIGHT_OFFSET,
	D2153_PROP_BAT_CAPACITY,
	/* If you want to add a property,
	   then please add before "D2153_PROP_ALL" attributes */
	D2153_PROP_ALL,
};

#define D2153_PROP_MAX 			(D2153_PROP_ALL + 1)

static struct device_attribute d2153_battery_attrs[]=
{
	__ATTR(display_soc_lut, S_IRUGO, d2153_battery_attrs_show, NULL),
	__ATTR(display_dischg_weight, S_IRUGO, d2153_battery_attrs_show, NULL),
	__ATTR(display_chg_weight, S_IRUGO, d2153_battery_attrs_show, NULL),
	__ATTR(display_weight_offset, S_IRUGO, d2153_battery_attrs_show, NULL),
	__ATTR(display_capacity, S_IRUGO, d2153_battery_attrs_show, NULL),
	/* If you want to add a property,
	   then please add before "D2153_PROP_ALL" attributes */
	__ATTR(display_all_information, S_IRUGO, d2153_battery_attrs_show, NULL),
};

static ssize_t d2153_battery_attrs_show(struct device *pdev,
										struct device_attribute *attr,
										char *buf)
{
	u8 i = 0, length = 0;
	ssize_t count = 0;
	unsigned int view_all = 0;
	const ptrdiff_t off = attr - d2153_battery_attrs;

	switch(off) {
		case D2153_PROP_ALL:
			view_all=1;

		case D2153_PROP_SOC_LUT:
			count += scnprintf(buf + count, PAGE_SIZE - count,
									"\n## SOC Look up table ...\n");
			// High temperature
			count += scnprintf(buf + count, PAGE_SIZE - count,
									"adc2soc_lut.adc_ht  = {");
			for(i = 0; i < adc2soc_lut_length; i++) {
				count += scnprintf(buf + count, PAGE_SIZE - count,
									"%4d,", adc2soc_lut.adc_ht[i]);
			}
			count += scnprintf(buf + count, PAGE_SIZE - count, "}\n");

			// Room temperature
			count += scnprintf(buf + count, PAGE_SIZE - count,
									"adc2soc_lut.adc_rt  = {");
			for(i = 0; i < adc2soc_lut_length; i++) {
				count += scnprintf(buf + count, PAGE_SIZE - count,
									"%4d,", adc2soc_lut.adc_rt[i]);
			}
			count += scnprintf(buf + count, PAGE_SIZE - count, "}\n");

			// Room-low temperature
			count += scnprintf(buf + count, PAGE_SIZE - count,
									"adc2soc_lut.adc_rlt = {");
			for(i = 0; i < adc2soc_lut_length; i++) {
				count += scnprintf(buf + count, PAGE_SIZE - count,
									"%4d,", adc2soc_lut.adc_rlt[i]);
			}
			count += scnprintf(buf + count, PAGE_SIZE - count, "}\n");

			// Low temperature
			count += scnprintf(buf + count, PAGE_SIZE - count,
									"adc2soc_lut.adc_lt  = {");
			for(i = 0; i < adc2soc_lut_length; i++) {
				count += scnprintf(buf + count, PAGE_SIZE - count,
									"%4d,", adc2soc_lut.adc_lt[i]);
			}
			count += scnprintf(buf + count, PAGE_SIZE - count, "}\n");

			// Low-mid temperature
			count += scnprintf(buf + count, PAGE_SIZE - count,
									"adc2soc_lut.adc_lmt = {");
			for(i = 0; i < adc2soc_lut_length; i++) {
				count += scnprintf(buf + count, PAGE_SIZE - count,
									"%4d,", adc2soc_lut.adc_lmt[i]);
			}
			count += scnprintf(buf + count, PAGE_SIZE - count, "}\n");

			// Low-low temperature
			count += scnprintf(buf + count, PAGE_SIZE - count,
									"adc2soc_lut.adc_llt = {");
			for(i = 0; i < adc2soc_lut_length; i++) {
				count += scnprintf(buf + count, PAGE_SIZE - count,
									"%4d,", adc2soc_lut.adc_llt[i]);
			}
			count += scnprintf(buf + count, PAGE_SIZE - count, "}\n");

			if (!view_all) break;

		case D2153_PROP_DISCHG_WEIGHT:
			length = (u8)sizeof(adc_weight_section_discharge)/sizeof(u16);
			count += scnprintf(buf + count, PAGE_SIZE - count,
									"\n## Discharging weight table ...\n");

			// Discharge weight at high and room temperature
			count += scnprintf(buf + count, PAGE_SIZE - count,
									"adc_weight_section_discharge     = {");
			for(i = 0; i < length; i++) {
				count += scnprintf(buf + count, PAGE_SIZE - count,
									"%5d,", adc_weight_section_discharge[i]);
			}
			count += scnprintf(buf + count, PAGE_SIZE - count, "}\n");

			// Discharge weight at room-low temperature
			count += scnprintf(buf + count, PAGE_SIZE - count,
									"adc_weight_section_discharge_rlt = {");
			for(i = 0; i < length; i++) {
				count += scnprintf(buf + count, PAGE_SIZE - count,
									"%5d,", adc_weight_section_discharge_rlt[i]);
			}
			count += scnprintf(buf + count, PAGE_SIZE - count, "}\n");

			// Discharge weight at low temperature
			count += scnprintf(buf + count, PAGE_SIZE - count,
									"adc_weight_section_discharge_lt  = {");
			for(i = 0; i < length; i++) {
				count += scnprintf(buf + count, PAGE_SIZE - count,
									"%5d,", adc_weight_section_discharge_lt[i]);
			}
			count += scnprintf(buf + count, PAGE_SIZE - count, "}\n");

			// Discharge weight at low-mid temperature
			count += scnprintf(buf + count, PAGE_SIZE - count,
									"adc_weight_section_discharge_lmt = {");
			for(i = 0; i < length; i++) {
				count += scnprintf(buf + count, PAGE_SIZE - count,
									"%5d,", adc_weight_section_discharge_lmt[i]);
			}
			count += scnprintf(buf + count, PAGE_SIZE - count, "}\n");

			// Discharge weight at low-low temperature
			count += scnprintf(buf + count, PAGE_SIZE - count,
									"adc_weight_section_discharge_llt = {");
			for(i = 0; i < length; i++) {
				count += scnprintf(buf + count, PAGE_SIZE - count,
									"%5d,", adc_weight_section_discharge_llt[i]);
			}
			count += scnprintf(buf + count, PAGE_SIZE - count, "}\n");

			if (!view_all) break;

		case D2153_PROP_CHG_WEIGHT:
			length = (u8)sizeof(adc_weight_section_charge)/sizeof(u16);
			count += scnprintf(buf + count, PAGE_SIZE - count,
									"\n## Charging weight table ...\n");

			// Discharge weight at high and room temperature
			count += scnprintf(buf + count, PAGE_SIZE - count,
									"adc_weight_section_charge     = {");
			for(i = 0; i < length; i++) {
				count += scnprintf(buf + count, PAGE_SIZE - count,
									"%5d,", adc_weight_section_charge[i]);
			}
			count += scnprintf(buf + count, PAGE_SIZE - count, "}\n");

			// Discharge weight at room-low temperature
			count += scnprintf(buf + count, PAGE_SIZE - count,
									"adc_weight_section_charge_rlt = {");
			for(i = 0; i < length; i++) {
				count += scnprintf(buf + count, PAGE_SIZE - count,
									"%5d,", adc_weight_section_charge_rlt[i]);
			}
			count += scnprintf(buf + count, PAGE_SIZE - count, "}\n");

			// Discharge weight at low temperature
			count += scnprintf(buf + count, PAGE_SIZE - count,
									"adc_weight_section_charge_lt  = {");
			for(i = 0; i < length; i++) {
				count += scnprintf(buf + count, PAGE_SIZE - count,
									"%5d,", adc_weight_section_charge_lt[i]);
			}
			count += scnprintf(buf + count, PAGE_SIZE - count, "}\n");

			// Discharge weight at low-mid temperature
			count += scnprintf(buf + count, PAGE_SIZE - count,
									"adc_weight_section_charge_lmt = {");
			for(i = 0; i < length; i++) {
				count += scnprintf(buf + count, PAGE_SIZE - count,
									"%5d,", adc_weight_section_charge_lmt[i]);
			}
			count += scnprintf(buf + count, PAGE_SIZE - count, "}\n");

			// Discharge weight at low-low temperature
			count += scnprintf(buf + count, PAGE_SIZE - count,
									"adc_weight_section_charge_llt = {");
			for(i = 0; i < length; i++) {
				count += scnprintf(buf + count, PAGE_SIZE - count,
									"%5d,", adc_weight_section_charge_llt[i]);
			}
			count += scnprintf(buf + count, PAGE_SIZE - count, "}\n");

			if (!view_all) break;

		case D2153_PROP_WEIGHT_OFFSET:
			count += scnprintf(buf + count, PAGE_SIZE - count,
								"\n## Weight offset value ... \n");
			count += scnprintf(buf + count, PAGE_SIZE - count,
								"MAX_ADD_DIS_PERCENT_FOR_WEIGHT2   = %d\n",
								MAX_ADD_DIS_PERCENT_FOR_WEIGHT2);
			count += scnprintf(buf + count, PAGE_SIZE - count,
								"MAX_ADD_DIS_PERCENT_FOR_WEIGHT1   = %d\n",
								MAX_ADD_DIS_PERCENT_FOR_WEIGHT1);
			count += scnprintf(buf + count, PAGE_SIZE - count,
								"MAX_ADD_DIS_PERCENT_FOR_WEIGHT0_5 = %d\n",
								MAX_ADD_DIS_PERCENT_FOR_WEIGHT0_5);
			count += scnprintf(buf + count, PAGE_SIZE - count,
								"MAX_ADD_DIS_PERCENT_FOR_WEIGHT    = %d\n",
								MAX_ADD_DIS_PERCENT_FOR_WEIGHT);
			count += scnprintf(buf + count, PAGE_SIZE - count,
								"MIN_ADD_DIS_PERCENT_FOR_WEIGHT    = %d\n",
								MIN_ADD_DIS_PERCENT_FOR_WEIGHT);

			if (!view_all) break;

 		case D2153_PROP_BAT_CAPACITY:
			count += scnprintf(buf + count, PAGE_SIZE - count,
								"\n## Battery Capacity is  %d mAh\n",
								USED_BATTERY_CAPACITY);
			if (!view_all) break;

		default:
			break;
	}

	return count;
}


#endif /* CONFIG_D2153_DEBUG_FEATURE */


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

	return ret;
}


/*
 * Name : adc_to_soc_with_temp_compensat
 *
 */
u32 adc_to_soc_with_temp_compensat(u16 adc, u16 temp) {
	int sh, sl;

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


/*
 * Name : soc_to_adc_with_temp_compensat
 *
 */
u32 soc_to_adc_with_temp_compensat(u16 soc, u16 temp) {
	int sh, sl;

	if(temp < BAT_LOW_LOW_TEMPERATURE)
		temp = BAT_LOW_LOW_TEMPERATURE;
	else if(temp > BAT_HIGH_TEMPERATURE)
		temp = BAT_HIGH_TEMPERATURE;

	pr_info("%s. Parameter. SOC = %d. temp = %d\n", __func__, soc, temp);

	if((temp <= BAT_HIGH_TEMPERATURE) && (temp > BAT_ROOM_TEMPERATURE)) {
		sh = chk_lut(adc2soc_lut.soc, adc2soc_lut.adc_ht, soc, adc2soc_lut_length);
		sl = chk_lut(adc2soc_lut.soc, adc2soc_lut.adc_rt, soc, adc2soc_lut_length);
		sh = sl + (temp - BAT_ROOM_TEMPERATURE)*(sh - sl)
								/ (BAT_HIGH_TEMPERATURE - BAT_ROOM_TEMPERATURE);
	} else if((temp <= BAT_ROOM_TEMPERATURE) && (temp > BAT_ROOM_LOW_TEMPERATURE)) {
		sh = chk_lut(adc2soc_lut.soc, adc2soc_lut.adc_rt, soc, adc2soc_lut_length);
		sl = chk_lut(adc2soc_lut.soc, adc2soc_lut.adc_rlt, soc, adc2soc_lut_length);
		sh = sl + (temp - BAT_ROOM_LOW_TEMPERATURE)*(sh - sl)
								/ (BAT_ROOM_TEMPERATURE-BAT_ROOM_LOW_TEMPERATURE);
	} else if((temp <= BAT_ROOM_LOW_TEMPERATURE) && (temp > BAT_LOW_TEMPERATURE)) {
		sh = chk_lut(adc2soc_lut.soc, adc2soc_lut.adc_rlt, soc, adc2soc_lut_length);
		sl = chk_lut(adc2soc_lut.soc, adc2soc_lut.adc_lt, soc, adc2soc_lut_length);
		sh = sl + (temp - BAT_LOW_TEMPERATURE)*(sh - sl)
								/ (BAT_ROOM_LOW_TEMPERATURE-BAT_LOW_TEMPERATURE);
	} else if((temp <= BAT_LOW_TEMPERATURE) && (temp > BAT_LOW_MID_TEMPERATURE)) {
		sh = chk_lut(adc2soc_lut.soc, adc2soc_lut.adc_lt, soc, adc2soc_lut_length);
		sl = chk_lut(adc2soc_lut.soc, adc2soc_lut.adc_lmt, soc, adc2soc_lut_length);
		sh = sl + (temp - BAT_LOW_MID_TEMPERATURE)*(sh - sl)
								/ (BAT_LOW_TEMPERATURE-BAT_LOW_MID_TEMPERATURE);
	} else {
		sh = chk_lut(adc2soc_lut.soc, adc2soc_lut.adc_lmt,	soc, adc2soc_lut_length);
		sl = chk_lut(adc2soc_lut.soc, adc2soc_lut.adc_llt, soc, adc2soc_lut_length);
		sh = sl + (temp - BAT_LOW_LOW_TEMPERATURE)*(sh - sl)
								/ (BAT_LOW_MID_TEMPERATURE-BAT_LOW_LOW_TEMPERATURE);
	}

	return sh;
}



static u16 pre_soc = 0xffff;
/*
 * Name : soc_filter
 *
 */
u16 soc_filter(u16 new_soc, u8 is_charging) {
	u16 soc = new_soc;

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
int adc_to_degree_k(u16 adc) {
    return (chk_lut_temp(adc2temp_lut.adc, adc2temp_lut.temp, adc, temp_lut_length));
}

int degree_k2c(u16 k) {
	return (K2C(k));
}

/*
 * Name : get_adc_offset
 *
 */
int get_adc_offset(u16 adc) {
    return (chk_lut(adc2vbat_lut.adc, adc2vbat_lut.offset, adc, adc2vbat_lut_length));
}

/*
 * Name : adc_to_vbat
 *
 */
u16 adc_to_vbat(u16 adc, u8 is_charging) {
	u16 a = adc;

	if(is_charging)
		a = adc - get_adc_offset(adc); // deduct charging offset
	// return (chk_lut(adc2vbat_lut.adc, adc2vbat_lut.vbat, a, adc2vbat_lut_length));
	return (2500 + ((a * 2000) >> 12));
}

/*
 * Name : adc_to_soc
 * get SOC (@ room temperature) according ADC input
 */
int adc_to_soc(u16 adc, u8 is_charging) {

	u16 a = adc;

	if(is_charging)
		a = adc - get_adc_offset(adc); // deduct charging offset
	return (chk_lut(adc2soc_lut.adc_rt, adc2soc_lut.soc, a, adc2soc_lut_length));
}


/*
 * Name : do_interpolation
 */
int do_interpolation(int x0, int x1, int y0, int y1, int x)
{
	int y = 0;

	if(!(x1 - x0 )) {
		pr_err("%s. Divied by Zero. Plz check x1(%d), x0(%d) value \n",
				__func__, x1, x0);
		return 0;
	}

	y = y0 + (x - x0)*(y1 - y0)/(x1 - x0);
	pr_info("%s. Interpolated y_value is = %d\n", __func__, y);

	return y;
}


#define D2153_MAX_SOC_CHANGE_OFFSET		20

/*
 * Name : d2153_get_soc
 */
static int d2153_get_soc(struct d2153_battery *pbat)
{
	int soc;
	struct d2153_battery_data *pbat_data = NULL;

	if((pbat == NULL) || (!pbat->battery_data.volt_adc_init_done)) {
		pr_err("%s. Invalid parameter. \n", __func__);
		return -EINVAL;
	}
#ifdef CONFIG_D2153_BATTERY_DEBUG
	pr_info("%s. Getting SOC\n", __func__);
#endif
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
	else if(soc > FULL_CAPACITY) {
		soc = FULL_CAPACITY;
		if(pbat_data->virtual_battery_full == 1) {
			pbat_data->virtual_battery_full = 0;
			pbat_data->soc = FULL_CAPACITY;
		}
	}
#ifdef CONFIG_D2153_BATTERY_DEBUG
	pr_info("%s. 0. SOC = %d\n", __func__, soc);
#endif

	/* Don't allow soc goes up when battery is dicharged.
	 and also don't allow soc goes down when battey is charged. */
	if(pbat_data->is_charging != TRUE
		&& (soc > pbat_data->prev_soc && pbat_data->prev_soc )) {
		soc = pbat_data->prev_soc;
	}
#ifndef CONFIG_D2153_SOC_GO_DOWN_IN_CHG
	else if(pbat_data->is_charging
		&& (soc < pbat_data->prev_soc) && pbat_data->prev_soc) {
#if defined(CONFIG_D2153_BATTERY_DEBUG)
		pr_info("%s: is_charging = %d, soc = %d, prev soc = %d",
			__func__, pbat_data->is_charging,
			soc, pbat_data->prev_soc);
#endif
		soc = pbat_data->prev_soc;

	}
#endif
	pbat_data->soc = soc;

	d2153_reg_write(pbat->pd2153, D2153_GP_ID_2_REG, (0xFF & soc));
	d2153_reg_write(pbat->pd2153, D2153_GP_ID_3_REG, (0x0F & (soc>>8)));

	d2153_reg_write(pbat->pd2153, D2153_GP_ID_4_REG,
							(0xFF & pbat_data->average_volt_adc));
	d2153_reg_write(pbat->pd2153, D2153_GP_ID_5_REG,
							(0xF & (pbat_data->average_volt_adc>>8)));

	return soc;
}


/*
 * Name : d2153_get_weight_from_lookup
 */
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
			weight = weight + ((tempk-BAT_ROOM_LOW_TEMPERATURE)
				*(adc_weight_section_charge[i]-adc_weight_section_charge_rlt[i]))
							/(BAT_ROOM_TEMPERATURE-BAT_ROOM_LOW_TEMPERATURE);
		} else {
			weight=adc_weight_section_discharge_rlt[i];
			weight = weight + ((tempk-BAT_ROOM_LOW_TEMPERATURE)
				*(adc_weight_section_discharge[i]-adc_weight_section_discharge_rlt[i]))
							/(BAT_ROOM_TEMPERATURE-BAT_ROOM_LOW_TEMPERATURE);
		}
	} else if((tempk <= BAT_ROOM_LOW_TEMPERATURE) && (tempk > BAT_LOW_TEMPERATURE)) {
		if(is_charging) {
			if(average_adc < plut[0]) i = 0;

			weight = adc_weight_section_charge_lt[i];
			weight = weight + ((tempk-BAT_LOW_TEMPERATURE)
				*(adc_weight_section_charge_rlt[i]-adc_weight_section_charge_lt[i]))
								/(BAT_ROOM_LOW_TEMPERATURE-BAT_LOW_TEMPERATURE);
		} else {
			weight = adc_weight_section_discharge_lt[i];
			weight = weight + ((tempk-BAT_LOW_TEMPERATURE)
				*(adc_weight_section_discharge_rlt[i]-adc_weight_section_discharge_lt[i]))
								/(BAT_ROOM_LOW_TEMPERATURE-BAT_LOW_TEMPERATURE);
		}
	} else if((tempk <= BAT_LOW_TEMPERATURE) && (tempk > BAT_LOW_MID_TEMPERATURE)) {
		if(is_charging) {
			if(average_adc < plut[0]) i = 0;

			weight = adc_weight_section_charge_lmt[i];
			weight = weight + ((tempk-BAT_LOW_MID_TEMPERATURE)
				*(adc_weight_section_charge_lt[i]-adc_weight_section_charge_lmt[i]))
								/(BAT_LOW_TEMPERATURE-BAT_LOW_MID_TEMPERATURE);
		} else {
			weight = adc_weight_section_discharge_lmt[i];
			weight = weight + ((tempk-BAT_LOW_MID_TEMPERATURE)
				*(adc_weight_section_discharge_lt[i]-adc_weight_section_discharge_lmt[i]))
								/(BAT_LOW_TEMPERATURE-BAT_LOW_MID_TEMPERATURE);
		}
	} else {
		if(is_charging) {
			if(average_adc < plut[0]) i = 0;

			weight = adc_weight_section_charge_llt[i];
			weight = weight + ((tempk-BAT_LOW_LOW_TEMPERATURE)
				*(adc_weight_section_charge_lmt[i]-adc_weight_section_charge_llt[i]))
								/(BAT_LOW_MID_TEMPERATURE-BAT_LOW_LOW_TEMPERATURE);
		} else {
			weight = adc_weight_section_discharge_llt[i];
			weight = weight + ((tempk-BAT_LOW_LOW_TEMPERATURE)
				*(adc_weight_section_discharge_lmt[i]-adc_weight_section_discharge_llt[i]))
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

	mutex_lock(&pbat->meoc_lock);

	ret = d2153_get_adc_hwsem();
	if (ret < 0) {
			mutex_unlock(&pbat->meoc_lock);
			pr_err("%s:lock is already taken.\n", __func__);
			return -EBUSY;
	}

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
	msleep(3);

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
	d2153_put_adc_hwsem();
	mutex_unlock(&pbat->meoc_lock);

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

	mutex_lock(&pbat->meoc_lock);

	ret = d2153_get_adc_hwsem();
	if (ret < 0) {
			mutex_unlock(&pbat->meoc_lock);
			pr_err("%s:lock is already taken.\n", __func__);
			return -EBUSY;
	}

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

out:
	d2153_put_adc_hwsem();
	mutex_unlock(&pbat->meoc_lock);

	if(flag == FALSE) {
		pr_warn("%s. Failed manual ADC conversion. channel(%d)\n", __func__, channel);
		ret = -EIO;
	}

	return ret;
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

#define LCD_DIM_ADC			2168

extern int get_cable_type(void);

/*
 * Name : d2153_reset_sw_fuelgauge
 */
static int d2153_reset_sw_fuelgauge(struct d2153_battery *pbat)
{
	u8 i, j = 0;
	int read_adc = 0;
	u16 charge_offset[] = {100, 250};
	u16 drop_offset[] = {95, 43};
	u32 average_adc, sum_read_adc = 0;
	struct d2153_battery_data *pbatt_data = &pbat->battery_data;

	if(unlikely(!pbat || !pbatt_data)) {
		pr_err("%s. Invalid argument\n", __func__);
		return -EINVAL;
	}

	pr_info("++++++ Reset Software Fuelgauge +++++++++++\n");
	pbatt_data->volt_adc_init_done = FALSE;

	/* Initialize ADC buffer */
	memset(pbatt_data->voltage_adc, 0x0, ARRAY_SIZE(pbatt_data->voltage_adc));
	pbatt_data->sum_voltage_adc = 0;
	pbatt_data->soc = 0;
	pbatt_data->prev_soc = 0;

	/* Read VBAT_S ADC */
	for(i = 8, j = 0; i; i--) {
		read_adc = pbat->d2153_read_adc(pbat, D2153_ADC_VOLTAGE);
		if(pbatt_data->adc_res[D2153_ADC_VOLTAGE].is_adc_eoc) {
			read_adc = pbatt_data->adc_res[D2153_ADC_VOLTAGE].read_adc;
			//pr_info("%s. Read ADC %d : %d\n", __func__, i, read_adc);
			if(read_adc > 0) {
				sum_read_adc += read_adc;
				j++;
			}
		}
		msleep(10);
	}
	average_adc = read_adc = sum_read_adc / j;
	pr_info("%s. average = %d, j = %d \n", __func__, average_adc, j);

	/* To be compensated a read ADC */
	if(pbatt_data->is_charging) {
		int offset = 0;

		if(average_adc > ORIGN_CV_START_ADC ) {
			int X0, X, X1;
			int Y0, Y, Y1;

			X0 = ORIGN_CV_START_ADC; X1 = ORIGN_FULL_CHARGED_ADC;
			Y0 = 10;	Y1 = 100;
			X = average_adc;

			Y = Y0 + ((X - X0) * (Y1 - Y0)) / (X1 - X0);
			average_adc = X - (Y1 - Y);
		}
		else {
			int type = get_cable_type();

			//pr_info("[L%d] %s average_adc = %4d, charger type = %d \n",
			//			__LINE__, __func__, average_adc, type);
			if(type <= 1)
				offset = charge_offset[type];
			else
				offset = charge_offset[0];
			average_adc -= offset;
		}
	} else {
		if(average_adc >= LCD_DIM_ADC )
			average_adc += drop_offset[0];
		else
			average_adc += drop_offset[1];
	}

	pr_info("%s. average = %d\n", __func__, average_adc);
	/* Reset the buffer from using a read ADC */
	for(i = AVG_SIZE; i ; i--) {
		pbatt_data->voltage_adc[i-1] = average_adc;
		pbatt_data->sum_voltage_adc += average_adc;
	}

	pbatt_data->current_volt_adc = average_adc;

	pbatt_data->origin_volt_adc = read_adc;
	pbatt_data->average_volt_adc = pbatt_data->sum_voltage_adc >> AVG_SHIFT;
	pbatt_data->voltage_idx = (pbatt_data->voltage_idx+1) % AVG_SIZE;
	pbatt_data->current_voltage = adc_to_vbat(pbatt_data->current_volt_adc,
										 pbatt_data->is_charging);
	pbatt_data->average_voltage = adc_to_vbat(pbatt_data->average_volt_adc,
										 pbatt_data->is_charging);
	pbat->battery_data.volt_adc_init_done = TRUE;

	pr_info("%s. Average. ADC = %d, Voltage =  %d\n",
			__func__, pbatt_data->average_volt_adc, pbatt_data->average_voltage);

	return 0;
}


#ifdef CONFIG_SEC_CHARGING_FEATURE
extern struct spa_power_data spa_power_pdata;
extern int spa_event_handler(int evt, void *data);
#endif

/*
 * Name : d2153_read_voltage
 */
static int d2153_read_voltage(struct d2153_battery *pbat)
{
	int new_vol_adc = 0, base_weight, new_vol_orign;
	int offset_with_old, offset_with_new = 0;
	int ret = 0;
	static int calOffset_4P2, calOffset_3P4 = 0;
	int num_multi=0;
	struct d2153_battery_data *pbat_data = &pbat->battery_data;
	u16 offset_charging=0;
	int charging_index = 0;
	int charging_status;
	struct power_supply *ps = NULL;
	union power_supply_propval value;

	ps = power_supply_get_by_name(spa_power_pdata.charger_name);
	if (ps == NULL) {
		pr_err("%s. ps  yet to register\n", __func__);
		return -EINVAL;
	}
	ps->get_property(ps, POWER_SUPPLY_PROP_STATUS, &value);
	charging_status = value.intval;

	pbat_data->is_charging = charging_status;

#ifdef CONFIG_D2153_EOC_CTRL
	if (charging_status) {
		if(pbat_data->charger_ctrl_status == D2153_BAT_CHG_MAX)
			pbat_data->charger_ctrl_status = D2153_BAT_CHG_START;
	} else {
		if(value.intval != POWER_SUPPLY_STATUS_FULL)
			pbat_data->charger_ctrl_status = D2153_BAT_CHG_MAX;
	}
#endif

#if defined(CONFIG_D2153_BATTERY_DEBUG)
	pr_info("## %s. is_charging = %d, charger_ctrl_status = %d\n",
			__func__, pbat_data->is_charging,
			pbat_data->charger_ctrl_status);
#endif

	// Read voltage ADC
	ret = pbat->d2153_read_adc(pbat, D2153_ADC_VOLTAGE);
	if(ret < 0)
		return ret;

	 pr_info("%s:voltage ADC = 0x%x\n", __func__,
			pbat_data->adc_res[D2153_ADC_VOLTAGE].read_adc);
	// Getting calibration result.

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

				// Commented out.
				//if(pbat_data->average_volt_adc > CV_START_ADC)
				//	base_weight = base_weight + ((pbat_data->average_volt_adc
				//			- CV_START_ADC)*(LAST_CHARGING_WEIGHT-base_weight))
				//			/ ((ADC_VAL_100_PERCENT+offset) - CV_START_ADC);
				if(pbat_data->virtual_battery_full == 1)
					base_weight = MAX_WEIGHT;

				if(offset_with_new > 0) {
					pbat_data->sum_total_adc += (offset_with_new * base_weight);

					num_multi = pbat_data->sum_total_adc / NORM_NUM;
					if(num_multi > 0) {
						new_vol_adc = pbat_data->average_volt_adc + num_multi;
						pbat_data->sum_total_adc = pbat_data->sum_total_adc
													- (num_multi * NORM_NUM);
					} else {
						new_vol_adc = pbat_data->average_volt_adc;
					}
				}
#ifdef CONFIG_D2153_SOC_GO_DOWN_IN_CHG
				else {
					offset_with_new = -offset_with_new;

					base_weight = base_weight
						+ (base_weight*MIN_ADD_DIS_PERCENT_FOR_WEIGHT)/100;
#if defined(CONFIG_D2153_BATTERY_DEBUG)
					pr_info("Charging. Recalculated base_weight = %d, new_vol_adc = %d\n",
								base_weight, new_vol_adc);
#endif
					pbat_data->sum_total_adc -= (offset_with_new * base_weight);

					num_multi = pbat_data->sum_total_adc / NORM_NUM;
					if(num_multi < 0){
						new_vol_adc = pbat_data->average_volt_adc + num_multi;
						pbat_data->sum_total_adc = pbat_data->sum_total_adc
													- (num_multi * NORM_NUM);
					} else {
						new_vol_adc = pbat_data->average_volt_adc;
					}
#if defined(CONFIG_D2153_BATTERY_DEBUG)
					pr_info("Charging. Recalculated new_vol_adc = %d\n", new_vol_adc);
#endif
				}
#endif /* !CONFIG_D2153_SOC_GO_DOWN_IN_CHG */

				pbat_data->current_volt_adc = new_vol_adc;
				pbat_data->sum_voltage_adc += new_vol_adc;
				pbat_data->sum_voltage_adc -= pbat_data->average_volt_adc;
				pbat_data->voltage_adc[pbat_data->voltage_idx] = new_vol_adc;
			}
			else {
				pbat_data->virtual_battery_full = 0;

				// Case of Discharging.
				offset_with_new = pbat_data->average_volt_adc - new_vol_adc;
				offset_with_old = pbat_data->voltage_adc[pbat_data->voltage_idx]
								- pbat_data->average_volt_adc;

				if (is_called_by_ticker == 1) {
					pr_info("%s. is_called_by_ticker = %d,  base_weight = %d, offset_with_new = %d, offset_with_old = %d\n",
						__func__, is_called_by_ticker, base_weight, offset_with_new, offset_with_old);

					// Have to compensate the offset variable.
					// Because of, the offset is just for waking up from sleep.
					// Until waking up a handset, a certain load may be nothing over system.
					new_vol_adc = new_vol_adc + DISCHARGE_SLEEP_OFFSET;
					offset_with_new = pbat_data->average_volt_adc - new_vol_adc;

					if (offset_with_new > 100) {
						base_weight = (base_weight * 24 / 10);
					} else {
						base_weight = (base_weight * 18 / 10);
					}
					pr_info("##### base_weight = %d\n", base_weight);
				}

				pr_info("Discharging. offset_with_new = %d, offset_with_old = %d\n",
								offset_with_new, offset_with_old);

				if(offset_with_new > 0) {
					u8 which_condition = 0;
					int x1 = 0, x0 = 0, y1 = 0, y0 = 0, y = 0;

					pr_info("Discharging. base_weight = %d, new_vol_adc = %d\n", base_weight, new_vol_adc);

					// Battery was discharged by some reason.
					// So, ADC will be calculated again
					if(offset_with_new >= MAX_DIS_OFFSET_FOR_WEIGHT2) {
						base_weight = base_weight
							+ (base_weight*MAX_ADD_DIS_PERCENT_FOR_WEIGHT2)/100;
						which_condition = 0;
					} else if(offset_with_new >= MAX_DIS_OFFSET_FOR_WEIGHT1) {
						x1 = MAX_DIS_OFFSET_FOR_WEIGHT2;
						x0 = MAX_DIS_OFFSET_FOR_WEIGHT1;
						y1 = MAX_ADD_DIS_PERCENT_FOR_WEIGHT2;
						y0 = MAX_ADD_DIS_PERCENT_FOR_WEIGHT1;
						which_condition = 1;
					} else if(offset_with_new >= MAX_DIS_OFFSET_FOR_WEIGHT0_5) {
						x1 = MAX_DIS_OFFSET_FOR_WEIGHT1;
						x0 = MAX_DIS_OFFSET_FOR_WEIGHT0_5;
						y1 = MAX_ADD_DIS_PERCENT_FOR_WEIGHT1;
						y0 = MAX_ADD_DIS_PERCENT_FOR_WEIGHT0_5;
						which_condition = 2;
					} else if(offset_with_new >= MAX_DIS_OFFSET_FOR_WEIGHT) {
						x1 = MAX_DIS_OFFSET_FOR_WEIGHT0_5;
						x0 = MAX_DIS_OFFSET_FOR_WEIGHT;
						y1 = MAX_ADD_DIS_PERCENT_FOR_WEIGHT0_5;
						y0 = MAX_ADD_DIS_PERCENT_FOR_WEIGHT;
						which_condition = 3;
					} else if(offset_with_new < MIN_DIS_OFFSET_FOR_WEIGHT) {
						base_weight = base_weight
							+ (base_weight*MIN_ADD_DIS_PERCENT_FOR_WEIGHT)/100;
						which_condition = 4;
					} else {
						base_weight = base_weight + (base_weight
							* ( MAX_ADD_DIS_PERCENT_FOR_WEIGHT
							- (((MAX_DIS_OFFSET_FOR_WEIGHT - offset_with_new)
							* (MAX_ADD_DIS_PERCENT_FOR_WEIGHT
								- MIN_ADD_DIS_PERCENT_FOR_WEIGHT))
							/ (MAX_DIS_OFFSET_FOR_WEIGHT
							    - MIN_DIS_OFFSET_FOR_WEIGHT))))/100;
						which_condition = 5;
					}

					pr_info("%s. Discharging condition : %d\n", __func__, which_condition);
					if(which_condition >= 1 && which_condition <= 3) {
						y = do_interpolation(x0, x1, y0, y1, offset_with_new);
						base_weight = base_weight + (base_weight * y) / 100;
					}
					pbat_data->sum_total_adc -= (offset_with_new * base_weight);

					pr_info("Discharging. Recalculated base_weight = %d\n",
								base_weight);

					num_multi = pbat_data->sum_total_adc / NORM_NUM;
					if(num_multi < 0) {
						new_vol_adc = pbat_data->average_volt_adc + num_multi;
						pbat_data->sum_total_adc = pbat_data->sum_total_adc
													- (num_multi * NORM_NUM);
					} else {
						new_vol_adc = pbat_data->average_volt_adc;
					}
				} else {
					new_vol_adc = pbat_data->average_volt_adc;
				}

				if(is_called_by_ticker == 0) {
					pbat_data->current_volt_adc = new_vol_adc;
					pbat_data->sum_voltage_adc += new_vol_adc;
					pbat_data->sum_voltage_adc -=
								pbat_data->voltage_adc[pbat_data->voltage_idx];
					pbat_data->voltage_adc[pbat_data->voltage_idx] = new_vol_adc;
				} else {
					int i;
					for (i = AVG_SIZE / 3; i ; i--) {
						pbat_data->current_volt_adc = new_vol_adc;
						pbat_data->sum_voltage_adc += new_vol_adc;
						pbat_data->sum_voltage_adc -=
										pbat_data->voltage_adc[pbat_data->voltage_idx];
						pbat_data->voltage_adc[pbat_data->voltage_idx] = new_vol_adc;
						pbat_data->voltage_idx = (pbat_data->voltage_idx+1) % AVG_SIZE;
					}
#ifndef CONFIG_D2153_HW_TIMER
					is_called_by_ticker=0;
#endif
				}
			}
		} else {
			u8 i = 0;
			u8 res_msb, res_lsb, is_convert = 0;
			u32 capacity = 0, convert_vbat_adc = 0;
			int X1, X0;
			int Y1, Y0 = FIRST_VOLTAGE_DROP_ADC;
			int X = C2K(pbat_data->average_temperature);

			d2153_reg_read(pbat->pd2153, D2153_GP_ID_2_REG, &res_lsb);
			d2153_reg_read(pbat->pd2153, D2153_GP_ID_3_REG, &res_msb);
			capacity = (((res_msb & 0x0F) << 8) | (res_lsb & 0xFF));
			pr_info("%s. A read capacity is %d\n", __func__, capacity);

			if(capacity) {
				u32 vbat_adc = 0;
				if(capacity == FULL_CAPACITY) {
					d2153_reg_read(pbat->pd2153, D2153_GP_ID_4_REG, &res_lsb);
					d2153_reg_read(pbat->pd2153, D2153_GP_ID_5_REG, &res_msb);
					vbat_adc = (((res_msb & 0x0F) << 8) | (res_lsb & 0xFF));
					pr_info("[L%d] %s. Read vbat_adc is %d\n", __LINE__, __func__, vbat_adc);
				}
				// If VBAT_ADC is zero then, getting new_vol_adc from capacity(SOC)
				if(vbat_adc >= ADC_VAL_100_PERCENT) {
					convert_vbat_adc = vbat_adc;
				} else {
					convert_vbat_adc = soc_to_adc_with_temp_compensat(capacity,
											C2K(pbat_data->average_temperature));
					is_convert = TRUE;
				}
				pr_info("[L%4d]%s. read SOC = %d, convert_vbat_adc = %d, is_convert = %d\n",
						__LINE__, __func__, capacity, convert_vbat_adc, is_convert);
			}

			pbat->pd2153->average_vbat_init_adc =
									(pbat->pd2153->vbat_init_adc[0] +
										pbat->pd2153->vbat_init_adc[1] +
										pbat->pd2153->vbat_init_adc[2]) / 3;

			if(pbat_data->is_charging) {
					pr_info("[L%d] %s cc charging. new_vol_adc is %4d \n", __LINE__, __func__, new_vol_adc);
					offset = initialize_charge_up_cc[0];
					new_vol_adc = pbat->pd2153->average_vbat_init_adc - offset;
			} else {
				new_vol_adc = pbat->pd2153->average_vbat_init_adc;
				pr_info("[L%d] %s discharging new_vol_adc = %d	\n", __LINE__, __func__, new_vol_adc);

				Y0 = FIRST_VOLTAGE_DROP_ADC;
				if(C2K(pbat_data->average_temperature) <= BAT_LOW_LOW_TEMPERATURE) {
					//pr_info("### BAT_LOW_LOW_TEMPERATURE new ADC is %4d \n", new_vol_adc);
					new_vol_adc += (Y0 + 340);
				} else if(C2K(pbat_data->average_temperature) >= BAT_ROOM_TEMPERATURE) {
					new_vol_adc += Y0;
					//pr_info("### BAT_ROOM_TEMPERATURE new ADC is %4d \n", new_vol_adc);
				} else {
					if(C2K(pbat_data->average_temperature) <= BAT_LOW_MID_TEMPERATURE) {
						Y1 = Y0 + 115;	Y0 = Y0 + 340;
						X0 = BAT_LOW_LOW_TEMPERATURE;
						X1 = BAT_LOW_MID_TEMPERATURE;
					} else if(C2K(pbat_data->average_temperature) <= BAT_LOW_TEMPERATURE) {
						Y1 = Y0 + 60;	Y0 = Y0 + 115;
						X0 = BAT_LOW_MID_TEMPERATURE;
						X1 = BAT_LOW_TEMPERATURE;
					} else {
						Y1 = Y0 + 25;	Y0 = Y0 + 60;
						X0 = BAT_LOW_TEMPERATURE;
						X1 = BAT_ROOM_LOW_TEMPERATURE;
					}
					new_vol_adc = new_vol_adc + Y0
									+ ((X - X0) * (Y1 - Y0)) / (X1 - X0);
				}
			}

			pr_info("[L%d] %s Calculated new_vol_adc is %4d \n", __LINE__, __func__, new_vol_adc);
			if(((is_convert == TRUE) && (capacity < FULL_CAPACITY)
						&& (convert_vbat_adc >= 614))
				|| ((is_convert == FALSE) && (capacity == FULL_CAPACITY)
						&& (convert_vbat_adc >= ADC_VAL_100_PERCENT))) {
				new_vol_adc = convert_vbat_adc;
				pr_info("[L%d] %s. convert_vbat_adc is assigned to new_vol_adc\n", __LINE__, __func__);
			}

			if(new_vol_adc > MAX_FULL_CHARGED_ADC) {
				new_vol_adc = MAX_FULL_CHARGED_ADC;
				pr_info("%s. Set new_vol_adc to max. ADC value\n", __func__);
			}

			for(i = AVG_SIZE; i ; i--) {
				pbat_data->voltage_adc[i-1] = new_vol_adc;
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

	// To read a temperature ADC of BOARD
	ret = pbat->d2153_read_adc(pbat, D2153_ADC_TEMPERATURE_1);
	if(pbat_data->adc_res[D2153_ADC_TEMPERATURE_1].is_adc_eoc) {
		new_temp_adc = pbat_data->adc_res[D2153_ADC_TEMPERATURE_1].read_adc;

		pbat_data->current_temp_adc = new_temp_adc;

		if(pbat_data->temp_adc_init_done) {
			pbat_data->sum_temperature_adc += new_temp_adc;
			pbat_data->sum_temperature_adc -=
						pbat_data->temperature_adc[pbat_data->temperature_idx];
			pbat_data->temperature_adc[pbat_data->temperature_idx] = new_temp_adc;
		} else {
			u8 i;

			for(i = 0; i < AVG_SIZE; i++) {
				pbat_data->temperature_adc[i] = new_temp_adc;
				pbat_data->sum_temperature_adc += new_temp_adc;
			}
			pbat_data->temp_adc_init_done = TRUE;
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


/*
 * Name : d2153_check_bat_presence
 *	  Determine battery presence by reading TEMP1 ADC
 */
int d2153_check_bat_presence(unsigned int opt)
{
	int ret = 0;
	struct d2153_battery_data *pbat_data = &gbat->battery_data;
	unsigned int bat_check_adc;
	adc_channel bat_temp_adc =
			gbat->pd2153->pdata->pbat_platform->bat_temp_adc;

	/* Read temperature ADC
	* Channel : D2153_ADC_TEMPERATURE_1 -> TEMP_BOARD
	* Channel : D2153_ADC_TEMPERATURE_2 -> TEMP_RF
	*/

	/* To read a TEMP1 ADC */
	ret = gbat->d2153_read_adc(gbat, bat_temp_adc);
/*	ret = gbat->d2153_read_adc(gbat, D2153_ADC_TEMPERATURE_1); */
	if (ret < 0)
		return ret;
/*	bat_check_adc = pbat_data->adc_res[D2153_ADC_TEMPERATURE_1].read_adc; */
	bat_check_adc = pbat_data->adc_res[bat_temp_adc].read_adc;
	pr_info("%s:bat temp channel = %d,adc val = 0x%x\n", __func__,
						bat_temp_adc, bat_check_adc);
	if (bat_check_adc == 0xfff)
		return BATTERY_NOT_DETECTED;
	else
		return BATTERY_DETECTED;
}

/*
 * Name : d2153_get_rf_temperature
 */
int d2153_get_rf_temperature(void)
{
	u8 i, j, channel;
	int sum_temp_adc, ret = 0;
	struct d2153_battery *pbat = gbat;
	struct d2153_battery_data *pbat_data = &gbat->battery_data;

	if(pbat == NULL || pbat_data == NULL) {
		pr_err("%s. battery_data is NULL\n", __func__);
		return -EINVAL;
	}

	/* To read a temperature2 ADC */
	sum_temp_adc = 0;
	channel = D2153_ADC_TEMPERATURE_2;
	for(i = 10, j = 0; i; i--) {
		ret = pbat->d2153_read_adc(pbat, channel);
		if(ret == 0) {
			sum_temp_adc += pbat_data->adc_res[channel].read_adc;
			if(++j == 3)
				break;
		} else
			msleep(20);
	}
	if (j) {
		pbat_data->current_rf_temp_adc = (sum_temp_adc / j);
		pbat_data->current_rf_temperature =
		degree_k2c(adc_to_degree_k(pbat_data->current_rf_temp_adc));
#ifdef CONFIG_D2153_BATTERY_DEBUG
		pr_info("%s. RF_TEMP_ADC = %d, RF_TEMPERATURE = %3d.%d\n",
				__func__,
				pbat_data->current_rf_temp_adc,
				(pbat_data->current_rf_temperature/10),
				(pbat_data->current_rf_temperature%10));
#endif
		return pbat_data->current_rf_temperature;
	} else {
		pr_err("%s:ERROR in reading RF temperature.\n", __func__);
		return -EIO;
	}
 }
EXPORT_SYMBOL(d2153_get_rf_temperature);


/*
 * Name : d2153_battery_read_status
 */
int d2153_battery_read_status(int type)
{
	int val = 0;
	struct d2153_battery *pbat = NULL;

	if (gbat == NULL) {
		dlg_err("%s. driver data is NULL\n", __func__);
		return -EINVAL;
	}

	pbat = gbat;

	switch(type){
		case D2153_BATTERY_SOC:
			val = d2153_get_soc(pbat);
			//val = (val+5)/10;
			val = (val)/10;
			break;

		case D2153_BATTERY_CUR_VOLTAGE:
			val = pbat->battery_data.current_voltage;
			break;

		case D2153_BATTERY_AVG_VOLTAGE:
			val = pbat->battery_data.average_voltage;
			break;

		case D2153_BATTERY_VOLTAGE_NOW :
		{
			u8 ch = D2153_ADC_VOLTAGE;

			val = pbat->d2153_read_adc(pbat, ch);
			if(val < 0)
				return val;
			if(pbat->battery_data.adc_res[ch].is_adc_eoc) {
				val = adc_to_vbat(pbat->battery_data.adc_res[ch].read_adc, 0);
#ifdef CONFIG_D2153_BATTERY_DEBUG
				printk("%s: read adc to bat value = %d\n",__func__, val);
#endif
			} else {
				val = -EINVAL;
			}
			break;
		}

		case D2153_BATTERY_TEMP_HPA:
			val = d2153_get_rf_temperature();
			break;

		case D2153_BATTERY_TEMP_ADC:
			val = pbat->battery_data.average_temp_adc;
			break;

		case D2153_BATTERY_SLEEP_MONITOR:
			is_called_by_ticker = 1;
#ifdef CONFIG_D2153_HW_TIMER
			do_gettimeofday(&suspend_time);
			wake_lock(&pbat->battery_data.sleep_monitor_wakeup);
#else
			wake_lock_timeout(&pbat->battery_data.sleep_monitor_wakeup,
									D2153_SLEEP_MONITOR_WAKELOCK_TIME);
#endif
			cancel_delayed_work_sync(&pbat->monitor_temp_work);
			cancel_delayed_work_sync(&pbat->monitor_volt_work);
			schedule_delayed_work(&pbat->monitor_temp_work, 0);
			schedule_delayed_work(&pbat->monitor_volt_work, 0);
			break;
	}

	return val;
}
EXPORT_SYMBOL(d2153_battery_read_status);


/*
 * Name : d2153_battery_set_status
 */
int d2153_battery_set_status(int type, int status)
{
	int val = 0;
	struct d2153_battery *pbat = NULL;

	if(gbat == NULL) {
		dlg_err("%s. driver data is NULL\n", __func__);
		return -EINVAL;
	}

	pbat = gbat;

	switch(type){
		case D2153_STATUS_CHARGING :
			/* Discharging = 0, Charging = 1 */
			pbat->battery_data.is_charging = status;
			break;
		case D2153_RESET_SW_FG :
			/* Reset SW fuel gauge */
			cancel_delayed_work_sync(&pbat->monitor_volt_work);
			val = d2153_reset_sw_fuelgauge(pbat);
			schedule_delayed_work(&gbat->monitor_volt_work, 0);
			break;
		case D2153_STATUS_EOC_CTRL:
			pbat->battery_data.charger_ctrl_status = status;
			break;
		default :
			return -EINVAL;
	}

	return val;
}
EXPORT_SYMBOL(d2153_battery_set_status);
/*
 * Name: d2153_ctrl_fg - Reset fuel gauge
 */
static int d2153_ctrl_fg(void *data)
{
	int ret;
	ret = d2153_battery_set_status(D2153_RESET_SW_FG, 0);
	return ret;
}
/*
 * Name: d2153_get_voltage - Get battery voltage
 */
static int d2153_get_voltage(unsigned char opt)
{
	int volt;
	volt = d2153_battery_read_status(D2153_BATTERY_AVG_VOLTAGE);
	return volt;
}
/*
 * Name: d2153_get_capacity - Get battery SOC
 */
static int d2153_get_capacity(void)
{
	unsigned int bat_per;
	bat_per = d2153_battery_read_status(D2153_BATTERY_SOC);
	return bat_per;
}
/*
 * Name: d2153_get_temp - Read battery temperature
 */
static int d2153_get_temp(unsigned int opt)
{
	int temp;
	temp = d2153_battery_read_status(D2153_BATTERY_TEMP_ADC);
	return temp;
}

static void d2153_monitor_voltage_work(struct work_struct *work)
{
	int ret=0;
	struct d2153_battery *pbat = container_of(work, struct d2153_battery, monitor_volt_work.work);
	struct d2153_battery_data *pbat_data = &pbat->battery_data;

	if(unlikely(!pbat || !pbat_data)) {
		pr_err("%s. Invalid driver data\n", __func__);
		goto err_adc_read;
	}

	ret = d2153_read_voltage(pbat);
	if(ret < 0)
	{
		pr_err("%s. Read voltage ADC failure\n", __func__);
		goto err_adc_read;
	}

	if(pbat_data->is_charging == 0) {
		schedule_delayed_work(&pbat->monitor_volt_work, D2153_VOLTAGE_MONITOR_NORMAL);
	}
	else {
#ifdef CONFIG_D2153_EOC_CTRL
		if (pbat_data->volt_adc_init_done && pbat_data->is_charging) {
			struct power_supply *ps;
			union power_supply_propval value;

			ps = power_supply_get_by_name("battery");
			if (ps == NULL) {
				pr_err("%s. ps \"battery\" yet to register\n",
								__func__);
				goto err_adc_read;
			}

					pr_info("%s. Battery PROP_STATUS = %d\n", __func__, value.intval);

					ps->get_property(ps, POWER_SUPPLY_PROP_STATUS, &value);
					if( value.intval == POWER_SUPPLY_STATUS_FULL) {
						if(((pbat_data->charger_ctrl_status == D2153_BAT_RECHG_FULL)
							|| (pbat_data->charger_ctrl_status == D2153_BAT_CHG_BACKCHG_FULL))
							&& (pbat_data->average_voltage >= D2153_BAT_CHG_BACK_FULL_LVL)) {
							spa_event_handler(SPA_EVT_EOC, 0);
							pbat_data->charger_ctrl_status = D2153_BAT_RECHG_FULL;
							pr_info("%s. Recharging Done.(3) 2nd full > discharge > Recharge\n", __func__);
						} else if((pbat_data->charger_ctrl_status == D2153_BAT_CHG_FRST_FULL)
									&& (pbat_data->average_voltage >= D2153_BAT_CHG_BACK_FULL_LVL)) {
							pbat_data->charger_ctrl_status = D2153_BAT_CHG_BACKCHG_FULL;
							spa_event_handler(SPA_EVT_EOC, 0);
							pr_info("%s. Recharging Done.(4) 1st full > 2nd full\n", __func__);
						}
					} else {
						// Will stop charging when a voltage approach to first full charge level.
						if((pbat_data->charger_ctrl_status < D2153_BAT_CHG_FRST_FULL)
							&& (pbat_data->average_voltage >= D2153_BAT_CHG_FRST_FULL_LVL)) {
							spa_event_handler(SPA_EVT_EOC, 0);
							pbat_data->charger_ctrl_status = D2153_BAT_CHG_FRST_FULL;
							pr_info("%s. First charge done.(1)\n", __func__);
						} else if((pbat_data->charger_ctrl_status < D2153_BAT_CHG_FRST_FULL)
							&& (pbat_data->average_voltage >= D2153_BAT_CHG_BACK_FULL_LVL)) {
							spa_event_handler(SPA_EVT_EOC, 0);
							spa_event_handler(SPA_EVT_EOC, 0);
							pbat_data->charger_ctrl_status = D2153_BAT_CHG_BACKCHG_FULL;
							pr_info("%s. Fully charged.(2)(Back-charging done)\n", __func__);
						}
					}
				}
#endif
		schedule_delayed_work(&pbat->monitor_volt_work, D2153_VOLTAGE_MONITOR_FAST);
	}

#ifdef CONFIG_D2153_BATTERY_DEBUG
	pr_info("# SOC = %3d.%d %%, ADC(read) = %4d, ADC(avg) = %4d, Voltage(avg) = %4d mV, ADC(VF) = %4d\n",
				(pbat->battery_data.soc/10),
				(pbat->battery_data.soc%10),
				pbat->battery_data.origin_volt_adc,
				pbat->battery_data.average_volt_adc,
				pbat->battery_data.average_voltage,
				pbat->battery_data.vf_adc);
#endif
#ifdef CONFIG_D2153_HW_TIMER
	if(is_called_by_ticker ==1) {
		is_called_by_ticker=0;
		wake_unlock(&gbat->battery_data.sleep_monitor_wakeup);
	}
#endif
	return;

err_adc_read:
	schedule_delayed_work(&pbat->monitor_volt_work, D2153_VOLTAGE_MONITOR_START);
	return;
}


static void d2153_monitor_temperature_work(struct work_struct *work)
{
	struct d2153_battery *pbat = container_of(work, struct d2153_battery, monitor_temp_work.work);
	int ret;

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

#ifdef CONFIG_D2153_BATTERY_DEBUG
	pr_info("# TEMP_BOARD(ADC) = %4d, Board Temperauter(Celsius) = %3d.%d\n",
				pbat->battery_data.average_temp_adc,
				(pbat->battery_data.average_temperature/10),
				(pbat->battery_data.average_temperature%10));
#endif
	return ;
}


/*
 * Name : d2153_battery_start
 */
void d2153_battery_start(void)
{
	schedule_delayed_work(&gbat->monitor_volt_work, 0);
}
EXPORT_SYMBOL_GPL(d2153_battery_start);


/*
 * Name : d2153_battery_data_init
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
	pbat_data->is_charging = D2153_BATTERY_STATUS_MAX;
#ifdef CONFIG_D2153_EOC_CTRL
	pbat_data->charger_ctrl_status = D2153_BAT_CHG_MAX;
#endif
	wake_lock_init(&pbat_data->sleep_monitor_wakeup, WAKE_LOCK_SUSPEND, "sleep_monitor");

	return;
}

#ifdef CONFIG_D2153_HW_TIMER
#define CMT17_SPI			100U

static DEFINE_SPINLOCK(cmt_lock);
#define CONFIG_D2153_BAT_CMT_OVF 60*5

#define CMT_OVF		((256*CONFIG_D2153_BAT_CMT_OVF) - 2)

static inline u32 dec2hex(u32 dec)
{
	return dec;
}


/*
 * rmu2_cmt_start: start CMT
 * input: none
 * output: none
 * return: none
 */
static void d2153_battery_cmt_start(void)
{
	unsigned long flags, wrflg, i = 0;

	printk(KERN_INFO "START < %s >\n", __func__);
	dlg_info("< %s >CMCLKE=%08x\n", __func__, __raw_readl(CMCLKE));
	dlg_info("< %s >CMSTR17=%08x\n", __func__, __raw_readl(CMSTR17));
	dlg_info("< %s >CMCSR17=%08x\n", __func__, __raw_readl(CMCSR17));
	dlg_info("< %s >CMCNT17=%08x\n", __func__, __raw_readl(CMCNT17));
	dlg_info("< %s >CMCOR17=%08x\n", __func__, __raw_readl(CMCOR17));

	spin_lock_irqsave(&cmt_lock, flags);
	__raw_writel(__raw_readl(CMCLKE) | (1<<7), CMCLKE);
	spin_unlock_irqrestore(&cmt_lock, flags);

	mdelay(8);

	__raw_writel(0, CMSTR17);
	__raw_writel(0U, CMCNT17);
	__raw_writel(0x000000a6U, CMCSR17);	/* Int enable */
	__raw_writel(dec2hex(CMT_OVF), CMCOR17);

	do {
		wrflg = ((__raw_readl(CMCSR17) >> 13) & 0x1);
		i++;
	} while (wrflg != 0x00 && i < 0xffffffff);

	__raw_writel(1, CMSTR17);

	dlg_info("< %s >CMCLKE=%08x\n", __func__, __raw_readl(CMCLKE));
	dlg_info("< %s >CMSTR17=%08x\n", __func__, __raw_readl(CMSTR17));
	dlg_info("< %s >CMCSR17=%08x\n", __func__, __raw_readl(CMCSR17));
	dlg_info("< %s >CMCNT17=%08x\n", __func__, __raw_readl(CMCNT17));
	dlg_info("< %s >CMCOR17=%08x\n", __func__, __raw_readl(CMCOR17));
}

/*
 * rmu2_cmt_stop: stop CMT
 * input: none
 * output: none
 * return: none
 */
void d2153_battery_cmt_stop(void)
{
	unsigned long flags, wrflg, i = 0;

	printk(KERN_INFO "START < %s >\n", __func__);
	__raw_readl(CMCSR17);
	__raw_writel(0x00000186U, CMCSR17);	/* Int disable */
	__raw_writel(0U, CMCNT17);
	__raw_writel(0, CMSTR17);

	do {
		wrflg = ((__raw_readl(CMCSR17) >> 13) & 0x1);
		i++;
	} while (wrflg != 0x00 && i < 0xffffffff);

	mdelay(12);
	spin_lock_irqsave(&cmt_lock, flags);
	__raw_writel(__raw_readl(CMCLKE) & ~(1<<7), CMCLKE);
	spin_unlock_irqrestore(&cmt_lock, flags);
}

#if 0
/*
 * rmu2_cmt_clear: CMT counter clear
 * input: none
 * output: none
 * return: none
 */
static void d2153_battery_cmt_clear(void)
{

	int wrflg, i = 0;
	printk(KERN_INFO "START < %s >\n", __func__);
	__raw_writel(0, CMSTR17);	/* Stop counting */

	__raw_writel(0U, CMCNT17);	/* Clear the count value */

	do {
		wrflg = ((__raw_readl(CMCSR17) >> 13) & 0x1);
		i++;
	} while (wrflg != 0x00 && i < 0xffffffff);
	__raw_writel(1, CMSTR17);	/* Enable counting again */
}
#endif

/*
 * rmu2_cmt_irq: IRQ handler for CMT
 * input:
 *		@irq: interrupt number
 *		@dev_id: device ID
 * output: none
 * return:
 *		IRQ_HANDLED: irq handled
 */
static irqreturn_t d2153_battery_cmt_irq(int irq, void *dev_id)
{
	unsigned int reg_val = __raw_readl(CMCSR17);

	reg_val &= ~0x0000c000U;
	__raw_writel(reg_val, CMCSR17);

	printk(KERN_ERR "d2153_battery_cmt_irq!!!!!!!!!!!!!!!!!!..");

	d2153_battery_read_status(D2153_BATTERY_SLEEP_MONITOR);

	return IRQ_HANDLED;
}

/*
 * rmu2_cmt_init_irq: IRQ initialization handler for CMT
 * input: none
 * output: none
 * return: none
 */
static void d2153_battery_cmt_init_irq(void)
{
	int ret;
	unsigned int irq;

	pr_info(" < %s >\n", __func__);

	irq = gic_spi(CMT17_SPI);
	set_irq_flags(irq, IRQF_VALID);
	ret = request_threaded_irq(irq, NULL, d2153_battery_cmt_irq, IRQF_ONESHOT,
				"CMT17_RWDT0", (void *)irq);
	if (0 > ret) {
		pr_err("%s:%d request_irq failed err=%d\n",
				__func__, __LINE__, ret);
		free_irq(irq, (void *)irq);
		return;
	}

	enable_irq_wake(irq);
}
#endif /* CONFIG_D2153_HW_TIMER */

/*
 * Name : d2153_battery_probe
 */
static int d2153_battery_probe(struct platform_device *pdev)
{
	struct d2153 *d2153 = platform_get_drvdata(pdev);
	struct d2153_battery *pbat = &d2153->batt;
	int i;
	int ret = 0;

	pr_info("Start %s\n", __func__);

	if(unlikely(!pbat)) {
		pr_err("%s. Invalid platform data\n", __func__);
		return -EINVAL;
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

	// Start schedule of dealyed work for temperature.
	schedule_delayed_work(&pbat->monitor_temp_work, 0);

	device_init_wakeup(&pdev->dev, 1);


#ifdef CONFIG_D2153_DEBUG_FEATURE
	for (i = 0; i < D2153_PROP_MAX ; i++) {
		ret = device_create_file(&pdev->dev, &d2153_battery_attrs[i]);
		if (ret) {
			printk(KERN_ERR "Failed to create battery sysfs entries\n");
			return ret;
		}
	}
#endif

#ifdef CONFIG_D2153_HW_TIMER
	d2153_battery_cmt_init_irq();
#endif

	spa_agent_register(SPA_AGENT_GET_CAPACITY,
			(void *)d2153_get_capacity, "d2153_battery");
	spa_agent_register(SPA_AGENT_GET_TEMP,
			(void *)d2153_get_temp, "d2153_battery");
	spa_agent_register(SPA_AGENT_GET_VOLTAGE,
			(void *)d2153_get_voltage, "d2153-battery");
	spa_agent_register(SPA_AGENT_GET_BATT_PRESENCE_PMIC,
			(void *)d2153_check_bat_presence, "d2153-battery");
	spa_agent_register(SPA_AGENT_CTRL_FG,
			(void *)d2153_ctrl_fg, "d2153-battery");

	schedule_delayed_work(&pbat->monitor_volt_work, 0);

	pr_info("%s. End...\n", __func__);
	return ret;
}

/*
 * Name : d2153_battery_suspend
 */
static int d2153_battery_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct d2153_battery *pbat = platform_get_drvdata(pdev);
	struct d2153 *d2153 = pbat->pd2153;

	pr_info("%s. Enter\n", __func__);

	if(unlikely(!d2153)) {
		pr_err("%s. Invalid parameter\n", __func__);
		return -EINVAL;
	}

	cancel_delayed_work_sync(&pbat->monitor_temp_work);
	cancel_delayed_work_sync(&pbat->monitor_volt_work);

#ifdef CONFIG_PM_DEBUG
	if (pmdbg_get_enable_dump_suspend())
		pmdbg_pmic_dump_suspend(d2153);
#endif /* CONFIG_PM_DEBUG */

#ifdef CONFIG_D2153_HW_TIMER
	if(suspend_time.tv_sec == 0) {
		do_gettimeofday(&suspend_time);
		pr_info("##### suspend_time = %ld\n", suspend_time.tv_sec);
	}

	d2153_battery_cmt_start();
#endif /* CONFIG_D2153_HW_TIMER */
	pr_info("%s. Leave\n", __func__);

	return 0;
}


/*
 * Name : d2153_battery_resume
 */
static int d2153_battery_resume(struct platform_device *pdev)
{
	struct d2153_battery *pbat = platform_get_drvdata(pdev);
	struct d2153 *d2153 = pbat->pd2153;
#ifdef CONFIG_D2153_HW_TIMER
	u8 do_sampling = 0;
	unsigned long monitor_work_start = 0;
#endif

	pr_info("%s. Enter\n", __func__);

	if(unlikely(!d2153)) {
		pr_err("%s. Invalid parameter\n", __func__);
		return -EINVAL;
	}

#ifdef CONFIG_D2153_HW_TIMER
	d2153_battery_cmt_stop();
#endif
	// Start schedule of dealyed work for monitoring voltage and temperature.
	if(!is_called_by_ticker) {
#ifdef CONFIG_D2153_HW_TIMER
		do_gettimeofday(&resume_time);

		pr_info("##### suspend_time = %ld, resume_time = %ld\n",
					suspend_time.tv_sec, resume_time.tv_sec);
		if((resume_time.tv_sec - suspend_time.tv_sec) > 10) {
			memset(&suspend_time, 0, sizeof(struct timeval));
			do_sampling = 1;
			pr_info("###### Sampling voltage & temperature ADC\n");
		}

		if(do_sampling) {
			monitor_work_start = 0;

			wake_lock_timeout(&pbat->battery_data.sleep_monitor_wakeup,
										D2153_SLEEP_MONITOR_WAKELOCK_TIME);
		}
		else {
			monitor_work_start = 1 * HZ;
		}
		schedule_delayed_work(&pbat->monitor_temp_work, monitor_work_start);
		schedule_delayed_work(&pbat->monitor_volt_work, monitor_work_start);
#else
		schedule_delayed_work(&pbat->monitor_temp_work, 0);
		schedule_delayed_work(&pbat->monitor_volt_work, 0);
#endif
	}

	pr_info("%s. Leave\n", __func__);

	return 0;
}


/*
 * Name : d2153_battery_remove
 */
static int d2153_battery_remove(struct platform_device *pdev)
{
	struct d2153_battery *pbat = platform_get_drvdata(pdev);
	struct d2153 *d2153 = pbat->pd2153;
	u8 i;

	if(unlikely(!d2153)) {
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

	d2153_put_adc_hwsem();

#ifdef CONFIG_D2153_DEBUG_FEATURE
	for (i = 0; i < D2153_PROP_MAX ; i++) {
		device_remove_file(&pdev->dev, &d2153_battery_attrs[i]);
	}
#endif

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
	printk(d2153_battery_banner);
	return platform_driver_register(&d2153_battery_driver);
}
subsys_initcall(d2153_battery_init);

static void __exit d2153_battery_exit(void)
{
	flush_scheduled_work();
	platform_driver_unregister(&d2153_battery_driver);
}
module_exit(d2153_battery_exit);

MODULE_AUTHOR("Dialog Semiconductor Ltd. < eric.jeong@diasemi.com >");
MODULE_DESCRIPTION("Battery driver for the Dialog D2153 PMIC");
MODULE_LICENSE("GPL");
MODULE_ALIAS("Power supply : d2153-battery");
