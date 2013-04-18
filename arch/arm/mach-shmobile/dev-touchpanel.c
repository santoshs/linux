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


/* Touch Panel auto detection for Garda_Kyle (Melfas and Imagis) */
static struct i2c_client *tsp_detector_i2c_client;

struct i2c_board_info i2c4_devices_melfas[] = {
	{
		I2C_BOARD_INFO("sec_touch", 0x48),
		.irq = irqpin2irq(32),
	},
};

struct i2c_board_info i2c4_devices_imagis[] = {
	{
		I2C_BOARD_INFO("IST30XX", 0xA0>>1),
		.irq = irqpin2irq(32),
	},
};

struct i2c_board_info i2c4_devices_zinitix[] = {
	{
		I2C_BOARD_INFO("zinitix_touch", 0x40>>1),
		.irq = irqpin2irq(32),
	},
};

static int __devinit tsp_detector_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	int ret = 0;
	struct i2c_adapter *adap = client->adapter;
	struct regulator *touch_regulator;
	unsigned short addr_list_melfas[] = { 0x48, I2C_CLIENT_END };
	unsigned short addr_list_zinitix[] = { 0x40>>1, I2C_CLIENT_END };

	touch_regulator = regulator_get(NULL, "vtsp_3v");
	if (IS_ERR(touch_regulator)) {
		printk(KERN_ERR "failed to get regulator for Touch Panel");
		return -ENODEV;
	}
	regulator_set_voltage(touch_regulator, 3000000, 3000000); /* 3.0V */
	regulator_enable(touch_regulator);
	msleep(20);

	tsp_detector_i2c_client = i2c_new_probed_device(adap,
						&i2c4_devices_melfas[0],
						addr_list_melfas, NULL);
	if (tsp_detector_i2c_client) {
		printk(KERN_INFO "Touch Panel: Melfas MMS-13X\n");
	} else {
		tsp_detector_i2c_client = i2c_new_probed_device(adap,
						&i2c4_devices_zinitix[0],
						addr_list_zinitix, NULL) ;
		if (tsp_detector_i2c_client) {
			printk(KERN_INFO "Touch Panel: Zinitix BT432\n");
		} else {
			tsp_detector_i2c_client = i2c_new_device(adap,
						&i2c4_devices_imagis[0]);
			printk(KERN_INFO "Touch Panel: Imagis IST30XX\n");
		}
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
	.probe	= tsp_detector_probe,
	.remove	= tsp_detector_remove,
	.id_table	= tsp_detector_idtable,
};
