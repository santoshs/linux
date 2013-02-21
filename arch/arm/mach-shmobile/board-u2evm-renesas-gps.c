#include <mach/board-u2evm.h>
#include <mach/board-u2evm-renesas-gps.h>
#include <mach/r8a73734.h>
#include <mach/gpio.h>
#ifdef CONFIG_PMIC_INTERFACE
#include <linux/pmic/pmic-tps80032.h>
#include <linux/pmic/pmic.h>
#endif
#include <linux/d2153/core.h>


struct class *gps_class;

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
        struct device *gnss_nrst_dev;
        struct device *gnss_en_dev;
        int    status = -EINVAL;

        gps_class = class_create(THIS_MODULE, "gps");
        if (IS_ERR(gps_class)) {
                pr_err("Failed to create class(sec)!\n");
                return PTR_ERR(gps_class);
        }
        BUG_ON(!gps_class);

        gps_dev = device_create(gps_class, NULL, 0, NULL, "device_gps");
        BUG_ON(!gps_dev);

        gnss_nrst_dev = device_create(gps_class, gps_dev, 0, NULL, "GNSS_NRST");
        BUG_ON(!gnss_nrst_dev);

        gnss_en_dev = device_create(gps_class, gps_dev, 0, NULL, "GNSS_EN");
        BUG_ON(!gnss_en_dev);

        status = sysfs_create_group(&gnss_nrst_dev->kobj,
                                    &GNSS_NRST_attr_group);

        if (status)
                pr_debug("%s: status for sysfs_create_group %d\n",
                         __func__, status);

        status = sysfs_create_group(&gnss_en_dev->kobj, &GNSS_EN_attr_group);

        if (status)
                pr_debug("%s: status for sysfs_create_group %d\n",
                         __func__, status);

        printk(KERN_ALERT "gps_gpio_init!!");

                gpio_request(GPIO_FN_SCIFB1_RXD, NULL);
                gpio_pull_up_port(GPIO_PORT79);
                gpio_request(GPIO_FN_SCIFB1_TXD, NULL);
                gpio_pull_off_port(GPIO_PORT78);
                gpio_request(GPIO_FN_SCIFB1_CTS, NULL);
                gpio_pull_up_port(GPIO_PORT77);
                gpio_request(GPIO_FN_SCIFB1_RTS, NULL);
                gpio_pull_off_port(GPIO_PORT76);

                /* GPS Settings */
                gpio_request(GPIO_PORT10, "GNSS_NRST");
                gpio_pull_off_port(GPIO_PORT10);
                gpio_direction_output(GPIO_PORT10, 0);

                gpio_request(GPIO_PORT11, "GNSS_EN");
                gpio_pull_off_port(GPIO_PORT11);
                gpio_direction_output(GPIO_PORT11, 0);

        printk("gps_gpio_init done!!\n");
}
