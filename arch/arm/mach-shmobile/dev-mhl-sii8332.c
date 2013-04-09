#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/sii8332_platform.h>
#include <mach/irqs.h>
#include <mach/r8a7373.h>

/*#include "midas.h"*/

#include <rtapi/screen_display.h>

#ifdef CONFIG_SAMSUNG_MHL

//#define SFEATURE_MDT_FEATURE
#define GPIO_MHL_SDA_1_8V GPIO_PORT15
#define GPIO_MHL_SCL_1_8V GPIO_PORT14
#define GPIO_MHL_INT      GPIO_PORT109
#define MHL_INT 48
#define GPIO_MHL_EN      GPIO_PORT102
#define GPIO_MHL_RST      GPIO_PORT39
#define I2C_BUS_ID_MHL	  15
#define GPIO_VBUS_EN GPIO_PORT25


static struct i2c_gpio_platform_data mhl_i2c_gpio_data = {
	.sda_pin    = GPIO_MHL_SDA_1_8V,
	.scl_pin    = GPIO_MHL_SCL_1_8V,
	.udelay  = 5,
	
};

static struct platform_device mhl_i2c_gpio_device = {
        .name       = "i2c-gpio",
        .id     = I2C_BUS_ID_MHL,
        .dev        = {
            .platform_data  = &mhl_i2c_gpio_data,
        },
};



static void sii9234_cfg_gpio(void)
{
	int gpio, rc;
	
	printk(KERN_INFO "%s()\n", __func__);
	
	gpio = GPIO_MHL_INT;
	rc = gpio_request(gpio, "MHL_INT");
	if (rc < 0) {
		printk(KERN_ERR "unable to request GPIO pin %d\n", gpio);
		return;
	}
	gpio_direction_input(gpio);
	
	gpio = GPIO_MHL_EN;
	rc = gpio_request(gpio, "HDMI_EN");
	if (rc < 0) {
		printk(KERN_ERR "unable to request GPIO pin %d\n", gpio);
		return;
	}
	gpio_direction_output(gpio, 0);

	gpio = GPIO_MHL_RST;
	rc = gpio_request(gpio, "MHL_RST");
	if (rc < 0) {
		printk(KERN_ERR "unable to request GPIO pin %d\n", gpio);
		return;
	}
	gpio_direction_output(gpio, 1);

}

extern void sii9234_power_onoff(bool on)
{
	printk(KERN_INFO "%s(%d)\n", __func__, on);

	if (on) {
		gpio_set_value(GPIO_MHL_EN, 1);
	} else {

		gpio_set_value(GPIO_MHL_RST, 0);
		usleep_range(10000, 20000);
		gpio_set_value(GPIO_MHL_RST, 1);

		gpio_set_value(GPIO_MHL_EN, 0);
	}
}
EXPORT_SYMBOL(sii9234_power_onoff);

extern void sii9234_reset(void)
{
	printk(KERN_INFO "%s()\n", __func__);

	gpio_set_value(GPIO_MHL_RST, 0);
	usleep_range(10000, 20000);
	gpio_set_value(GPIO_MHL_RST, 1);
}
EXPORT_SYMBOL(sii9234_reset);

static void mhl_usb_switch_control(bool on)
{


	printk(KERN_INFO "%s() [MHL] USB path change : %s\n",
	       __func__, on ? "MHL" : "USB");

}



static struct mhl_platform_data sii9234_pdata = {
	.mhl_int = MHL_INT,
	.mhl_rst = GPIO_MHL_RST,
};

static struct i2c_board_info __initdata i2c_devs_sii9234[] = {
	{
		I2C_BOARD_INFO("SIMG72", 0x72>>1),
		.platform_data = &sii9234_pdata,
		.irq = R8A7373_IRQC_IRQ(MHL_INT),
	},
	{
		I2C_BOARD_INFO("SIMG7A", 0x7A>>1),
		.platform_data = &sii9234_pdata,
		.irq = R8A7373_IRQC_IRQ(MHL_INT),
	},
	{
		I2C_BOARD_INFO("SIMG92", 0x92>>1),
		.platform_data = &sii9234_pdata,
		.irq = R8A7373_IRQC_IRQ(MHL_INT),
	},
	{
		I2C_BOARD_INFO("SIMG9A", 0x9A>>1),
		.platform_data = &sii9234_pdata,
		.irq = R8A7373_IRQC_IRQ(MHL_INT),
	},
	{
		I2C_BOARD_INFO("SIMGC8", 0xC8>>1),
		.platform_data = &sii9234_pdata,
		.irq = R8A7373_IRQC_IRQ(MHL_INT),
	},
};

static struct i2c_board_info i2c_dev_hdmi_ddc __initdata = {
	I2C_BOARD_INFO("s5p_ddc", (0x74 >> 1)),
};

void __init mhl_init(void)
{
	int ret;
	printk("%s : START", __func__);

	ret = i2c_register_board_info(I2C_BUS_ID_MHL, i2c_devs_sii9234,
			ARRAY_SIZE(i2c_devs_sii9234));

	if (ret < 0) {
		printk(KERN_ERR "[MHL] adding i2c fail - nodevice\n");
		return;
	}

	sii9234_cfg_gpio();

	sii9234_power_onoff(1);

	sii9234_reset();


	return;
}




#endif
