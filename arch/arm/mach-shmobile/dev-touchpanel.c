/*
 * arch/arm/mach-shmobile/dev-touchpanel.c
 *
 * Copyright (C) 2013 Renesas Mobile Corporation
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
 *
*/
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/d2153/pmic.h>
#include <linux/regulator/consumer.h>
#include <mach/irqs.h>

#include <linux/i2c/bcmtch15xxx.h>
#include <mach/bcmtch15xxx_settings.h>

#define BCMTCH15XXX_TSC_NAME	"bcmtch15xxx"

/* Touch Panel auto detection */
static struct i2c_client *tsp_detector_i2c_client;

static struct i2c_board_info i2c4_devices_imagis[] = {
	{
		I2C_BOARD_INFO("IST30XX", 0xA0>>1),
		.irq = irqpin2irq(32),
	},
};

static struct i2c_board_info i2c4_devices_zinitix[] = {
	{
		I2C_BOARD_INFO("zinitix_touch", 0x20),
		.irq = irqpin2irq(32),
	},
};

static int __devinit tsp_detector_probe(struct i2c_client *client,
		const struct i2c_device_id * id)
{
	int ret=0;
	struct i2c_adapter *adap = client->adapter;
	struct regulator *touch_regulator;
	unsigned short addr_list_touch[] = { 0x20, I2C_CLIENT_END };


	touch_regulator = regulator_get(NULL, "vtsp_3v");
	if(IS_ERR(touch_regulator)){
		printk(KERN_ERR "failed to get regulator for Touch Panel");
		return -ENODEV;
	}
	regulator_set_voltage(touch_regulator, 3000000, 3000000); /* 3.0V */
	regulator_enable(touch_regulator);
	msleep(20);

	if ((tsp_detector_i2c_client = i2c_new_probed_device(adap,	&i2c4_devices_zinitix[0], addr_list_touch, NULL)))
	{
		printk("Touch Panel: Zinitix BT432\n");

	} else {
		tsp_detector_i2c_client = i2c_new_device(adap,
						&i2c4_devices_imagis[0]);
		printk("Touch Panel: Imagis IST30XX\n");
	}

	regulator_disable(touch_regulator);
	regulator_put(touch_regulator);
	return ret;
}

static int tsp_detector_remove(struct i2c_client *client)
{
	i2c_unregister_device(tsp_detector_i2c_client);
	tsp_detector_i2c_client = NULL;
	return 0;
}

static struct i2c_device_id tsp_detector_idtable[] = {
	{ "tsp_detector", 0 },
	{},
};

struct i2c_driver tsp_detector_driver = {
	.driver = {
		.name = "tsp_detector",
	},
	.probe 		= tsp_detector_probe,
	.remove 	= tsp_detector_remove,
	.id_table 	= tsp_detector_idtable,
};

#if 0
static int BCMTCH_TSP_PowerOnOff(bool on)
{
/* PLACE TOUCH CONTROLLER REGULATOR CODE HERE – SEE STEP 6 */
}
#endif

static struct bcmtch_platform_data bcmtch15xxx_i2c_platform_data = {
	.i2c_bus_id       = BCMTCH_HW_I2C_BUS_ID,
	.i2c_addr_spm = BCMTCH_HW_I2C_ADDR_SPM,
	.i2c_addr_sys   = BCMTCH_HW_I2C_ADDR_SYS,

	.gpio_interrupt_pin       = BCMTCH_HW_GPIO_INTERRUPT_PIN,
	.gpio_interrupt_trigger = BCMTCH_HW_GPIO_INTERRUPT_TRIGGER,

	.gpio_reset_pin           = BCMTCH_HW_GPIO_RESET_PIN,
	.gpio_reset_polarity   = BCMTCH_HW_GPIO_RESET_POLARITY,
	.gpio_reset_time_ms = BCMTCH_HW_GPIO_RESET_TIME_MS,

#if 0
	.ext_button_count = BCMTCH_BUTTON_COUNT,
	.ext_button_map   = bcmtch_button_map,

	.axis_orientation_flag =
		((BCMTCH_HW_AXIS_ REVERSE _X << BCMTCH_AXIS_FLAG_X_BIT_POS)
		|(BCMTCH_HW_AXIS_ REVERSE _Y << BCMTCH_AXIS_FLAG_Y_BIT_POS)
		|(BCMTCH_HW_AXIS_SWAP_X_Y << BCMTCH_AXIS_FLAG_X_Y_BIT_POS)),

	.bcmtch_on = BCMTCH_TSP_PowerOnOff,
#endif
};

static struct i2c_board_info __initdata bcmtch15xxx_i2c_boardinfo[] = {
	{
		I2C_BOARD_INFO(BCMTCH15XXX_TSC_NAME, BCMTCH_HW_I2C_ADDR_SPM),
		.irq = irqpin2irq(BCMTCH_HW_GPIO_INTERRUPT_PIN),
		.platform_data = &bcmtch15xxx_i2c_platform_data,
	},
};

void __init tsp_bcmtch15xxx_init(void)
{
	printk(KERN_INFO "%s: [TSP] init\n", __func__);
	i2c_register_board_info(bcmtch15xxx_i2c_platform_data.i2c_bus_id,
		bcmtch15xxx_i2c_boardinfo,
		ARRAY_SIZE(bcmtch15xxx_i2c_boardinfo));
}
