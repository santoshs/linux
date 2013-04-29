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
#include <mach/common.h>
#include <mach/gpio.h>
#include <mach/r8a7373.h>

#ifdef CONFIG_PMIC_INTERFACE
#include <linux/pmic/pmic-tps80032.h>
#endif

#if defined (CONFIG_MPU_SENSORS_MPU6050B1)
#include <linux/mpu.h>
#endif

#if defined (CONFIG_MPU_SENSORS_MPU6500)
#include <linux/mpu_511.h>
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


#if defined(CONFIG_MPU_SENSORS_MPU6050B1) || defined(CONFIG_MPU_SENSORS_MPU6500) || defined(CONFIG_OPTICAL_TAOS_TRITON)
//static struct regulator *vsensor_1v8;
void sensor_power_on_vdd(int onoff)
{
	
}
#endif


#if defined(CONFIG_MPU_SENSORS_MPU6050B1) || defined(CONFIG_MPU_SENSORS_MPU6500)
void mpu_power_on(int onoff)
{
	sensor_power_on_vdd(onoff);
}
#endif

#if defined(CONFIG_OPTICAL_TAOS_TRITON)
int taos_power_on(bool onoff)
{
	//sensor_power_on_vdd(onoff);
	return 0;
}
#endif

#if defined(CONFIG_OPTICAL_TAOS_TRITON)
int taos_led_onoff(bool onoff)
{
        return 0;
}
#endif

#if defined(CONFIG_MPU_SENSORS_MPU6050B1)
struct mpu_platform_data mpu6050_data = {
	.int_config = 0x10,
	.orientation = {
        0, 1, 0,
        -1, 0, 0,
        0, 0, 1},
	.poweron = mpu_power_on,
	};
#endif
	
#if defined(CONFIG_MPU_SENSORS_MPU6500)
static struct mpu_platform_data mpu6500_data =
{
	.int_config  = 0x10,
	.level_shifter = 0,
	.orientation = {
        0, 1, 0,
        -1, 0, 0,
        0, 0, 1},
        .sec_slave_type = SECONDARY_SLAVE_TYPE_NONE,
        .key = {0xdd, 0x16, 0xcd, 0x7, 0xd9, 0xba, 0x97, 0x37, 
        0xce, 0xfe, 0x23, 0x90, 0xe1, 0x66, 0x2f, 0x32},
        .poweron = mpu_power_on,
	};
#endif

#if defined  (CONFIG_SENSORS_K3DH)
static struct k3dh_platform_data k3dh_platform_data = {
	.orientation = {
	-1, 0, 0,
	0, -1, 0,
	0, 0, 1},	      
};
#endif

#if defined  (CONFIG_SENSORS_HSCDTD006A) || defined(CONFIG_SENSORS_HSCDTD008A) 
static struct hscd_i2c_platform_data hscd_i2c_platform_data = {
	.orientation = {
	0, -1, 0,
	1, 0, 0,
	0, 0, 1},
};
#endif

#if defined  (CONFIG_SENSORS_GP2AP002)
#define PROXI_INT_GPIO_PIN      (108)
static struct gp2ap002_platform_data gp2ap002_platform_data = {
	.irq_gpio = PROXI_INT_GPIO_PIN,
	.irq = irqpin2irq(PROXI_INT_GPIO_PIN),        
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
#if defined(CONFIG_MPU_SENSORS_MPU6050B1)
	{
		I2C_BOARD_INFO("mpu6050", 0x68),
		.irq = irqpin2irq(46),
		.platform_data = &mpu6050_data,
	 },
#endif

#if defined (CONFIG_MPU_SENSORS_MPU6500)
	{
		I2C_BOARD_INFO("mpu6500", 0x68),
		.irq = irqpin2irq(46),
		.platform_data = &mpu6500_data,
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

#if defined(CONFIG_MPU_SENSORS_MPU6050B1) || defined(CONFIG_MPU_SENSORS_MPU6500)
void mpl_init(void)
{
	int rc = 0;
	rc = gpio_request(GPIO_PORT107, "MPUIRQ");
	if (rc < 0)
		pr_err("GPIO_MPU3050_INT gpio_request was failed\n");
	gpio_direction_input(GPIO_PORT107);
	gpio_pull_up_port(GPIO_PORT107);

	mpu_power_on(1);
}
#endif

void __init sensor_init(void)
{
	printk("%s : START\n", __func__);

#if defined(CONFIG_MPU_SENSORS_MPU6050B1) || defined(CONFIG_MPU_SENSORS_MPU6500)
	mpl_init();
#endif

	i2c_register_board_info(I2C_BUS_ID_SENSOR, i2c2_devices, ARRAY_SIZE(i2c2_devices));
	return;
}


