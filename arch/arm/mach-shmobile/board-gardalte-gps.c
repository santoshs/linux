/*
 * GPS GPIO control
 *
 *  Copyright (C) 2012 Samsung, Inc.
 *  Copyright (C) 2012 Google, Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#include <mach/board-gardalte.h>
#include <mach/dev-gps.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <mach/r8a7373.h>
#include <mach/gpio.h>
#include <mach/board.h>

#include <linux/sysfs.h>
#include <linux/proc_fs.h>

#ifdef CONFIG_PMIC_INTERFACE
#include <linux/pmic/pmic-tps80032.h>
#include <linux/pmic/pmic.h>
#endif
#include <linux/d2153/core.h>

struct class *gps_class;

#define FUNC2_MODE_SCIFB 0x02

static ssize_t GNSS_NRST_value_show(struct device *dev,
                                    struct device_attribute *attr, char *buf)
{
        int value = gpio_get_value(GPIO_PORT10);

        return sprintf(buf, "%d\n", value);
}

static ssize_t GNSS_NRST_value_store(struct device *dev,
                                     struct device_attribute *attr,
                                     const char *buf, size_t count)
{
        long value;
        int  ret;

        ret = strict_strtol(buf, 0, &value);
        if (ret < 0)
                return ret;

        if (u2_get_board_rev() >= 5) {
#ifdef CONFIG_MFD_D2153
                if (1 == value)
                        d2153_clk32k_enable(1);         /* on */
                else
                        d2153_clk32k_enable(0);         /* off */
#endif /* CONFIG_MFD_D2153 */
        } else {
#if defined(CONFIG_PMIC_INTERFACE)
                if (1 == value)
                        pmic_clk32k_enable(CLK32KG, TPS80032_STATE_ON);
                else
                        pmic_clk32k_enable(CLK32KG, TPS80032_STATE_OFF);
#endif /* CONFIG_PMIC_INTERFACE */
        }

        printk(KERN_ALERT "%s: %d\n", __func__, value);

        gpio_set_value(GPIO_PORT10, value);

        return count;
}

DEVICE_ATTR(value_nrst, 0644, GNSS_NRST_value_show, GNSS_NRST_value_store);

static ssize_t GNSS_EN_value_show(struct device *dev,
                                struct device_attribute *attr, char *buf)
{
        int value = gpio_get_value(GPIO_PORT11);

        return sprintf(buf, "%d\n", value);
}

static ssize_t GNSS_EN_value_store(struct device *dev,
                        struct device_attribute *attr,
                        const char *buf, size_t count)
{
        long value;
        int  ret;

        ret = strict_strtol(buf, 0, &value);
        if (ret < 0)
                return ret;

        printk(KERN_ALERT "%s: %d\n", __func__, value);

        gpio_set_value(GPIO_PORT11, value);

        return count;
}

DEVICE_ATTR(value_en, 0644, GNSS_EN_value_show, GNSS_EN_value_store);

static const struct attribute *GNSS_NRST_attrs[] = {
        &dev_attr_value_nrst.attr,
        NULL,
};

static const struct attribute_group GNSS_NRST_attr_group = {
        .attrs = (struct attribute **) GNSS_NRST_attrs,
};

static const struct attribute *GNSS_EN_attrs[] = {
        &dev_attr_value_en.attr,
        NULL,
};

static const struct attribute_group GNSS_EN_attr_group = {
        .attrs = (struct attribute **) GNSS_EN_attrs,
};


void gps_gpio_init(void)
{
	struct device *gps_dev;

	gps_class = class_create(THIS_MODULE, "gps");
	if (IS_ERR(gps_class)) {
		pr_err("Failed to create class(sec)!\n");
		return;
	}
	BUG_ON(!gps_class);

	gps_dev	= device_create(gps_class, NULL, 0,	NULL, "bcm4752");
	BUG_ON(!gps_dev);

	printk("gps_gpio_init!!");

	/* SCIFB1::UART mode & Function mode settings. */
	gpio_request(GPIO_FN_SCIFB1_RXD, NULL);
	gpio_pull_up_port(GPIO_PORT79);

	gpio_request(GPIO_FN_SCIFB1_TXD, NULL);
	gpio_pull_up_port(GPIO_PORT78);

	gpio_request(GPIO_FN_SCIFB1_CTS, NULL);
	gpio_pull_off_port(GPIO_PORT77);

	gpio_request(GPIO_FN_SCIFB1_RTS, NULL);
	gpio_pull_off_port(GPIO_PORT76);


	/* GPS Settings */
	gpio_request(GPIO_PORT11, "GPS_PWR_EN");
	gpio_pull_off_port(GPIO_PORT11);
	gpio_direction_output(GPIO_PORT11, 0);

	gpio_request(GPIO_PORT48, "GPS_ECLK_26M");
	gpio_pull_off_port(GPIO_PORT48);
	gpio_direction_output(GPIO_PORT48, 0);


	gpio_export(GPIO_PORT11, 1);
	gpio_export(GPIO_PORT48, 1);

	gpio_export_link(gps_dev, "GPS_PWR_EN", GPIO_PORT11);
	gpio_export_link(gps_dev, "GPS_ECLK_26M", GPIO_PORT48);

	printk("gps_gpio_init done!!");
}


