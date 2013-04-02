/*
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
 */

#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <mach/common.h>
#include <mach/gpio.h>
#include <mach/r8a7373.h>
#include <mach/irqs.h>

#include <linux/pm.h>
#include <linux/regulator/consumer.h>

#ifdef CONFIG_PMIC_INTERFACE
#include <linux/pmic/pmic-tps80032.h>
#include <linux/mfd/tps80031.h>
#include <linux/pmic/pmic.h>
#include <mach/setup-u2tps80032.h>
#include <linux/regulator/tps80031-regulator.h>
#endif

#ifdef CONFIG_MFD_D2153
#include <linux/d2153/core.h>
#include <linux/d2153/pmic.h>
#include <linux/d2153/d2153_battery.h>
#endif

#if defined(CONFIG_INPUT_MPU6050) || defined(CONFIG_INPUT_MPU6500)
#include <linux/mpu6k_input.h>
#endif

#if defined  (CONFIG_SENSORS_K3DH)
#include <linux/k3dh_dev.h>
#endif

#if defined  (CONFIG_SENSORS_HSCDTD006A) || defined(CONFIG_SENSORS_HSCDTD008A) 
#include <linux/hscd_i2c_dev.h>
#endif

#if defined (CONFIG_SENSORS_GP2AP002)
#include <linux/gp2ap002_dev.h>
#endif

#if defined (CONFIG_OPTICAL_TAOS_TRITON)
#include <linux/i2c/taos.h>
#endif

#define I2C_BUS_ID_SENSOR	  2

#if defined(CONFIG_OPTICAL_TAOS_TRITON)
int taos_power_on(bool onoff)
{
	return 0;
}
#endif

#if defined(CONFIG_OPTICAL_TAOS_TRITON)
int taos_led_onoff(bool onoff)
{
	return 0;
}
#endif

#if defined(CONFIG_INPUT_MPU6050) || defined(CONFIG_INPUT_MPU6500)
static void sensors_regulator_on(bool onoff)
{

}

static struct mpu6k_input_platform_data mpu6k_pdata = {
	.power_on = sensors_regulator_on,
	.orientation = {
		0, 1, 0,
		-1, 0, 0,
		0, 0, 1},
	.acc_cal_path = "/efs/calibration_data",
	.gyro_cal_path = "/efs/gyro_cal_data",
};
#endif


#if defined  (CONFIG_SENSORS_K3DH)
static struct k3dh_platform_data k3dh_platform_data = {
	.orientation = {
#if defined (CONFIG_MACH_LOGANLTE)
	1, 0, 0,
	0, 1, 0,
	0, 0, 1},
#else
	-1, 0, 0,
	0, -1, 0,
	0, 0, 1},
#endif
};
#endif

#if defined  (CONFIG_SENSORS_HSCDTD006A) || defined(CONFIG_SENSORS_HSCDTD008A) 
static struct hscd_i2c_platform_data hscd_i2c_platform_data = {
	.orientation = {
#if defined (CONFIG_MACH_LOGANLTE)
	1, 0, 0,
	0, 1, 0,
	0, 0, 1},
#else
	0, -1, 0,
	1, 0, 0,
	0, 0, 1},
#endif	
};
#endif

#if defined  (CONFIG_SENSORS_GP2AP002)
static void gp2ap002_power_onoff(bool onoff)
{
	if (onoff) {
#if defined (CONFIG_MACH_GARDALTE)
		if (u2_get_board_rev() == 1) {  //REV0.1
			if (gpio_request(21, "PROXY_LED_EN"))
				printk(KERN_ERR "[GP2A] Proximity Request GPIO_21 failed!\n");
			else
				printk(KERN_ERR "[GP2A] Proximity Request GPIO_21 Sucess!\n");
			gpio_direction_output(21 , 1);
			printk(KERN_INFO "[GP2A] gpio_get_value of GPIO(21) is %d\n", gpio_get_value(21));
		}
#endif
	}
}

static void gp2ap002_led_onoff(bool onoff)
{
#if defined (CONFIG_MACH_GARDALTE)
	if (u2_get_board_rev() == 2) {  //REV0.2
#endif	    
		struct regulator *led_regulator = NULL;
		int ret=0;

		if (onoff) {
			led_regulator = regulator_get(NULL, "sensor_led_3v");
			if (IS_ERR(led_regulator)){
				printk(KERN_ERR "[GP2A] can not get prox_regulator (SENSOR_LED_3.0V) \n");
			} else {
				ret = regulator_set_voltage(led_regulator,3000000,3000000);
				printk(KERN_INFO "[GP2A] regulator_set_voltage : %d\n", ret);
				ret = regulator_enable(led_regulator);
				printk(KERN_INFO "[GP2A] regulator_enable : %d\n", ret);
				regulator_put(led_regulator);
				mdelay(5);
			}
		} else {
			led_regulator = regulator_get(NULL, "sensor_led_3v");
			ret = regulator_disable(led_regulator); 
			printk(KERN_INFO "[GP2A] regulator_disable : %d\n", ret);
			regulator_put(led_regulator);
		}
#if defined (CONFIG_MACH_GARDALTE) 
	}
#endif
}

#define PROXI_INT_GPIO_PIN 108
static struct gp2ap002_platform_data gp2ap002_platform_data = {
	.power_on = gp2ap002_power_onoff,
	.led_on = gp2ap002_led_onoff,
	.irq_gpio = PROXI_INT_GPIO_PIN,
	.irq = irqpin2irq(46),
};
#endif

#if defined(CONFIG_OPTICAL_TAOS_TRITON)
#define GPIO_ALS_INT	 108
static struct taos_platform_data taos_pdata = {
	.power	= taos_power_on,
	.led_on	= taos_led_onoff,
	.als_int = GPIO_ALS_INT,
	.prox_thresh_hi = 650,
	.prox_thresh_low = 510,
	.als_time = 0xED,
	.intr_filter = 0x33,
	.prox_pulsecnt = 0x08,
	.prox_gain = 0x28,
	.coef_atime = 50,
	.ga = 117,
	.coef_a = 1000,
	.coef_b = 1600,
	.coef_c = 660,
	.coef_d = 1250,
};
#endif

static struct i2c_board_info __initdata i2c2_devices[] = {

#if defined(CONFIG_INPUT_MPU6050)
	{
		I2C_BOARD_INFO("mpu6050_input", 0x68),
		.platform_data = &mpu6k_pdata,
		.irq =  irqpin2irq(46),
	},
#endif

#if defined(CONFIG_INPUT_MPU6500)
	{
		I2C_BOARD_INFO("mpu6500_input", 0x62), /*dummy address*/
		.platform_data = &mpu6k_pdata,
		.irq =  irqpin2irq(46),
	},
#endif

#if defined (CONFIG_INPUT_YAS_SENSORS)
	{
		I2C_BOARD_INFO("geomagnetic", 0x2e),
	},
#endif

#if defined  (CONFIG_SENSORS_K3DH)
	{
		I2C_BOARD_INFO("k3dh", 0x19),
		.platform_data = &k3dh_platform_data,                        
	},
#endif

#if defined  (CONFIG_SENSORS_HSCDTD006A) || defined(CONFIG_SENSORS_HSCDTD008A) 
	{
		I2C_BOARD_INFO("hscd_i2c", 0x0c),
		.platform_data = &hscd_i2c_platform_data,               
	},
 #endif

#if defined  (CONFIG_SENSORS_GP2AP002)
	{
		I2C_BOARD_INFO("gp2ap002", 0x44),
		.platform_data = &gp2ap002_platform_data,            
	},
#endif

#if defined(CONFIG_OPTICAL_TAOS_TRITON)
	{
		I2C_BOARD_INFO("taos", 0x39),
		.platform_data = &taos_pdata,
	},
#endif
};

#if defined(CONFIG_INPUT_MPU6050) || defined(CONFIG_INPUT_MPU6500)
void mpl_init(void)
{
	int rc = 0;
	rc = gpio_request(GPIO_PORT107, "MPUIRQ");
	if (rc < 0)
		pr_err("GPIO_MPU3050_INT gpio_request was failed\n");
	gpio_direction_input(GPIO_PORT107);
	gpio_pull_up_port(GPIO_PORT107);
}
#endif

void __init board_sensor_init(void)
{
	printk("%s : START\n", __func__);

#if defined(CONFIG_INPUT_MPU6050) || defined(CONFIG_INPUT_MPU6500)
	mpl_init();
#endif

	i2c_register_board_info(I2C_BUS_ID_SENSOR, i2c2_devices, ARRAY_SIZE(i2c2_devices));
	return;
}
