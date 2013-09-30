/*
 * arch/arm/mach-shmobile/pm.c
 *
 * Copyright (C) 2012 Renesas Mobile Corporation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 * */

#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <mach/common.h>
#include <mach/gpio.h>
#include <mach/r8a7373.h>
#include <mach/irqs.h>
#include <mach/dev-sensor.h>

#include <linux/lsm303dl.h>
#include <linux/interrupt.h>
#include <linux/irq.h>

static struct lsm303dl_platform_data lsm303dl_platdata = {

};

static struct platform_device lsm303dl_device = {
	.name           = "lsm303dl",
	.dev            = {
	.platform_data  = &lsm303dl_platdata,
	},
};

static struct lsm303dl_acc_port_info lsm303dl_acc_platdata = {
	.lsm303dl_acc_port = GPIO_PORT110,
};

static struct lsm303dl_mag_port_info lsm303dl_mag_platdata = {
	.lsm303dl_mag_port = GPIO_PORT109,
};

static struct i2c_board_info i2c2_devices[] = {
	{
		I2C_BOARD_INFO("l3gd20", 0x6b),
	},

	{
		I2C_BOARD_INFO("lsm303dl_acc", 0x19),
		.platform_data = &lsm303dl_acc_platdata,
		.irq = R8A7373_IRQC_IRQ(0x31),
		.flags = IORESOURCE_IRQ | IRQ_TYPE_EDGE_FALLING,
	},
	{
		I2C_BOARD_INFO("lsm303dl_mag", 0x1E),
		.platform_data = &lsm303dl_mag_platdata,
		.irq = R8A7373_IRQC_IRQ(0x30),
		.flags = IORESOURCE_IRQ | IRQ_TYPE_EDGE_FALLING,
	},
};

void __init amethyst_board_sensor_init(void)
{
	printk("%s : START\n", __func__);

	platform_device_register(&lsm303dl_device);
	i2c_register_board_info(2, i2c2_devices, ARRAY_SIZE(i2c2_devices));
	return;
}
