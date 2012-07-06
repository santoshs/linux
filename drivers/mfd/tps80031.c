/*
 * driver/mfd/tps80031.c
 *
 * Core driver for TI TPS80031
 *
 * Copyright (C) 2011 NVIDIA Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/i2c.h>

#include <linux/mfd/core.h>
#include <linux/mfd/tps80031.h>

/* interrupt related registers */
#define TPS80031_INT_STS_A		0xD0
#define TPS80031_INT_STS_B		0xD1
#define TPS80031_INT_STS_C		0xD2
#define TPS80031_INT_MSK_LINE_A		0xD3
#define TPS80031_INT_MSK_LINE_B		0xD4
#define TPS80031_INT_MSK_LINE_C		0xD5
#define TPS80031_INT_MSK_STS_A		0xD6
#define TPS80031_INT_MSK_STS_B		0xD7
#define TPS80031_INT_MSK_STS_C		0xD8

/* Version number related register */
#define TPS80031_JTAGVERNUM		0x87

/* External control register */
#define REGEN1_BASE_ADD		0xAE
#define REGEN2_BASE_ADD		0xB1
#define SYSEN_BASE_ADD		0xB4

/* device control registers */
#define TPS80031_PHOENIX_DEV_ON	0x25
#define DEVOFF	1

#define CLK32KAO_BASE_ADD	0xBA
#define CLK32KG_BASE_ADD	0xBD
#define CLK32KAUDIO_BASE_ADD	0xC0

#define EXT_CONTROL_CFG_TRANS 0
#define EXT_CONTROL_CFG_STATE 1

#define STATE_OFF	0x00
#define STATE_ON	0x01
#define STATE_MASK	0x03

#define TPS_NUM_SLAVES	4

static u8 pmc_ext_control_base[] = {
	REGEN1_BASE_ADD,
	REGEN2_BASE_ADD,
	SYSEN_BASE_ADD,
};

struct tps80031_irq_data {
	u8	mask_reg;
	u8	mask_mask;
};

#define TPS80031_IRQ(_reg, _mask)				\
	{							\
		.mask_reg = (TPS80031_INT_MSK_LINE_##_reg) -	\
				TPS80031_INT_MSK_LINE_A,	\
		.mask_mask = (_mask),				\
	}

static const struct tps80031_irq_data tps80031_irqs[] = {

	[TPS80031_INT_PWRON]		= TPS80031_IRQ(A, 0),
	[TPS80031_INT_RPWRON]		= TPS80031_IRQ(A, 1),
	[TPS80031_INT_SYS_VLOW]		= TPS80031_IRQ(A, 2),
	[TPS80031_INT_RTC_ALARM]	= TPS80031_IRQ(A, 3),
	[TPS80031_INT_RTC_PERIOD]	= TPS80031_IRQ(A, 4),
	[TPS80031_INT_HOT_DIE]		= TPS80031_IRQ(A, 5),
	[TPS80031_INT_VXX_SHORT]	= TPS80031_IRQ(A, 6),
	[TPS80031_INT_SPDURATION]	= TPS80031_IRQ(A, 7),
	[TPS80031_INT_WATCHDOG]		= TPS80031_IRQ(B, 0),
	[TPS80031_INT_BAT]		= TPS80031_IRQ(B, 1),
	[TPS80031_INT_SIM]		= TPS80031_IRQ(B, 2),
	[TPS80031_INT_MMC]		= TPS80031_IRQ(B, 3),
	[TPS80031_INT_RES]		= TPS80031_IRQ(B, 4),
	[TPS80031_INT_GPADC_RT]		= TPS80031_IRQ(B, 5),
	[TPS80031_INT_GPADC_SW2_EOC]	= TPS80031_IRQ(B, 6),
	[TPS80031_INT_CC_AUTOCAL]	= TPS80031_IRQ(B, 7),
	[TPS80031_INT_ID_WKUP]		= TPS80031_IRQ(C, 0),
	[TPS80031_INT_VBUSS_WKUP]	= TPS80031_IRQ(C, 1),
	[TPS80031_INT_ID]		= TPS80031_IRQ(C, 2),
	[TPS80031_INT_VBUS]		= TPS80031_IRQ(C, 3),
	[TPS80031_INT_CHRG_CTRL]	= TPS80031_IRQ(C, 4),
	[TPS80031_INT_EXT_CHRG]		= TPS80031_IRQ(C, 5),
	[TPS80031_INT_INT_CHRG]		= TPS80031_IRQ(C, 6),
	[TPS80031_INT_RES2]		= TPS80031_IRQ(C, 7),
};

/* Structure for TPS80031 Slaves */
struct tps80031_client {
	struct i2c_client *client;
	struct mutex lock;
	u8 addr;
};

struct tps80031 {
	struct device		*dev;

	struct gpio_chip	gpio;
	struct irq_chip		irq_chip;
	struct mutex		irq_lock;
	int			irq_base;
	u32			irq_en;
	u8			mask_cache[3];
	u8			mask_reg[3];
	struct tps80031_client	tps_clients[TPS_NUM_SLAVES];
};

static inline int __tps80031_read(struct i2c_client *client,
				  int reg, uint8_t *val)
{
	int ret;

	ret = i2c_smbus_read_byte_data(client, reg);
	if (ret < 0) {
		dev_err(&client->dev,
			"failed reading from addr 0x%02x, reg 0x%02x\n",
			client->addr, reg);
		return ret;
	}

	*val = (uint8_t)ret;

	return 0;
}

static inline int __tps80031_reads(struct i2c_client *client, int reg,
				int len, uint8_t *val)
{
	int ret;

	ret = i2c_smbus_read_i2c_block_data(client, reg, len, val);
	if (ret < 0) {
		dev_err(&client->dev,
			"failed reading from addr 0x%02x, reg	0x%02x\n",
			 client->addr, reg);
		return ret;
	}

	return 0;
}

static inline int __tps80031_write(struct i2c_client *client,
				 int reg, uint8_t val)
{
	int ret;
	ret = i2c_smbus_write_byte_data(client, reg, val);
	if (ret < 0) {
		dev_err(&client->dev,
			"failed writing 0x%02x to 0x%02x\n", val, reg);
		return ret;
	}

	return 0;
}

static inline int __tps80031_writes(struct i2c_client *client, int reg,
				  int len, uint8_t *val)
{
	int ret;

	ret = i2c_smbus_write_i2c_block_data(client, reg, len, val);
	if (ret < 0) {
		dev_err(&client->dev, "failed writings to 0x%02x\n", reg);
		return ret;
	}

	return 0;
}

int tps80031_write(struct device *dev, int sid, int reg, uint8_t val)
{
	struct tps80031 *tps80031 = dev_get_drvdata(dev);
	struct tps80031_client *tps = &tps80031->tps_clients[sid];
	int ret;

	mutex_lock(&tps->lock);
	ret = __tps80031_write(tps->client, reg, val);
	mutex_unlock(&tps->lock);

	return ret;
}
EXPORT_SYMBOL_GPL(tps80031_write);

int tps80031_writes(struct device *dev, int sid, int reg, int len, uint8_t *val)
{
	struct tps80031 *tps80031 = dev_get_drvdata(dev);
	struct tps80031_client *tps = &tps80031->tps_clients[sid];
	int ret;

	mutex_lock(&tps->lock);
	ret = __tps80031_writes(tps->client, reg, len, val);
	mutex_unlock(&tps->lock);

	return ret;
}
EXPORT_SYMBOL_GPL(tps80031_writes);

int tps80031_read(struct device *dev, int sid, int reg, uint8_t *val)
{
	struct tps80031 *tps80031 = dev_get_drvdata(dev);
	struct tps80031_client *tps = &tps80031->tps_clients[sid];

	return __tps80031_read(tps->client, reg, val);
}
EXPORT_SYMBOL_GPL(tps80031_read);

int tps80031_reads(struct device *dev, int sid, int reg, int len, uint8_t *val)
{
	struct tps80031 *tps80031 = dev_get_drvdata(dev);
	struct tps80031_client *tps = &tps80031->tps_clients[sid];

	return __tps80031_reads(tps->client, reg, len, val);
}
EXPORT_SYMBOL_GPL(tps80031_reads);

int tps80031_set_bits(struct device *dev, int sid, int reg, uint8_t bit_mask)
{
	struct tps80031 *tps80031 = dev_get_drvdata(dev);
	struct tps80031_client *tps = &tps80031->tps_clients[sid];
	uint8_t reg_val;
	int ret = 0;

	mutex_lock(&tps->lock);

	ret = __tps80031_read(tps->client, reg, &reg_val);
	if (ret)
		goto out;

	if ((reg_val & bit_mask) != bit_mask) {
		reg_val |= bit_mask;
		ret = __tps80031_write(tps->client, reg, reg_val);
	}
out:
	mutex_unlock(&tps->lock);
	return ret;
}
EXPORT_SYMBOL_GPL(tps80031_set_bits);

int tps80031_clr_bits(struct device *dev, int sid, int reg, uint8_t bit_mask)
{
	struct tps80031 *tps80031 = dev_get_drvdata(dev);
	struct tps80031_client *tps = &tps80031->tps_clients[sid];
	uint8_t reg_val;
	int ret = 0;

	mutex_lock(&tps->lock);

	ret = __tps80031_read(tps->client, reg, &reg_val);
	if (ret)
		goto out;

	if (reg_val & bit_mask) {
		reg_val &= ~bit_mask;
		ret = __tps80031_write(tps->client, reg, reg_val);
	}
out:
	mutex_unlock(&tps->lock);
	return ret;
}
EXPORT_SYMBOL_GPL(tps80031_clr_bits);

int tps80031_update(struct device *dev, int sid, int reg, uint8_t val,
		uint8_t mask)
{
	struct tps80031 *tps80031 = dev_get_drvdata(dev);
	struct tps80031_client *tps = &tps80031->tps_clients[sid];
	uint8_t reg_val;
	int ret = 0;

	mutex_lock(&tps->lock);

	ret = __tps80031_read(tps->client, reg, &reg_val);
	if (ret)
		goto out;

	if ((reg_val & mask) != val) {
		reg_val = (reg_val & ~mask) | (val & mask);
		ret = __tps80031_write(tps->client, reg, reg_val);
	}
out:
	mutex_unlock(&tps->lock);
	return ret;
}
EXPORT_SYMBOL_GPL(tps80031_update);

int tps80031_force_update(struct device *dev, int sid, int reg, uint8_t val,
			  uint8_t mask)
{
	struct tps80031 *tps80031 = dev_get_drvdata(dev);
	struct tps80031_client *tps = &tps80031->tps_clients[sid];
	uint8_t reg_val;
	int ret = 0;

	mutex_lock(&tps->lock);

	ret = __tps80031_read(tps->client, reg, &reg_val);
	if (ret)
		goto out;

	reg_val = (reg_val & ~mask) | (val & mask);
	ret = __tps80031_write(tps->client, reg, reg_val);

out:
	mutex_unlock(&tps->lock);
	return ret;
}
EXPORT_SYMBOL_GPL(tps80031_force_update);

static struct tps80031 *tps80031_dev;
int tps80031_power_off(void)
{
	struct tps80031_client *tps = &tps80031_dev->tps_clients[SLAVE_ID1];

	if (!tps->client)
		return -EINVAL;
	dev_info(&tps->client->dev, "switching off PMU\n");
	return __tps80031_write(tps->client, TPS80031_PHOENIX_DEV_ON, DEVOFF);
}

static int tps80031_gpio_get(struct gpio_chip *gc, unsigned offset)
{
	struct tps80031 *tps80031 = container_of(gc, struct tps80031, gpio);
	struct tps80031_client *tps = &tps80031->tps_clients[SLAVE_ID1];
	uint8_t state;
	uint8_t trans;
	int ret;

	ret = __tps80031_read(tps->client,
			pmc_ext_control_base[offset] +
				EXT_CONTROL_CFG_STATE, &state);
	if (ret)
		return ret;

	if (state != 0) {
		ret = __tps80031_read(tps->client,
				pmc_ext_control_base[offset] +
					EXT_CONTROL_CFG_TRANS, &trans);
		if (ret)
			return ret;
		return trans & 0x1;
	}
	return 0;
}

static void tps80031_gpio_set(struct gpio_chip *gc, unsigned offset,
			int value)
{
	struct tps80031 *tps80031 = container_of(gc, struct tps80031, gpio);

	tps80031_update(tps80031->dev, SLAVE_ID1,
		pmc_ext_control_base[offset] + EXT_CONTROL_CFG_TRANS,
			value, 0x1);
}

static int tps80031_gpio_input(struct gpio_chip *gc, unsigned offset)
{
	return -EIO;
}

static int tps80031_gpio_output(struct gpio_chip *gc, unsigned offset,
				int value)
{
	tps80031_gpio_set(gc, offset, value);
	return 0;
}

static int tps80031_gpio_enable(struct gpio_chip *gc, unsigned offset)
{
	struct tps80031 *tps80031 = container_of(gc, struct tps80031, gpio);
	int ret;

	ret = tps80031_update(tps80031->dev, SLAVE_ID1,
		pmc_ext_control_base[offset] + EXT_CONTROL_CFG_STATE,
						STATE_ON, STATE_MASK);
	if (ret)
		return ret;

	return tps80031_write(tps80031->dev, SLAVE_ID1,
		pmc_ext_control_base[offset] + EXT_CONTROL_CFG_TRANS, 0x0);
}

static void tps80031_gpio_disable(struct gpio_chip *gc, unsigned offset)
{
	struct tps80031 *tps80031 = container_of(gc, struct tps80031, gpio);
	tps80031_update(tps80031->dev, SLAVE_ID1,
		pmc_ext_control_base[offset] + EXT_CONTROL_CFG_STATE,
						STATE_OFF, STATE_MASK);
}

static void tps80031_gpio_init(struct tps80031 *tps80031,
			struct tps80031_platform_data *pdata)
{
	int ret;
	int gpio_base = pdata->gpio_base;
	struct tps80031_client *tps = &tps80031->tps_clients[SLAVE_ID1];

	if (gpio_base <= 0)
		return;

	tps80031->gpio.owner		= THIS_MODULE;
	tps80031->gpio.label		= tps->client->name;
	tps80031->gpio.dev		= tps80031->dev;
	tps80031->gpio.base		= gpio_base;
	tps80031->gpio.ngpio		= 3;
	tps80031->gpio.can_sleep	= 1;

	tps80031->gpio.request		= tps80031_gpio_enable;
	tps80031->gpio.free		= tps80031_gpio_disable;
	tps80031->gpio.direction_input	= tps80031_gpio_input;
	tps80031->gpio.direction_output	= tps80031_gpio_output;
	tps80031->gpio.set		= tps80031_gpio_set;
	tps80031->gpio.get		= tps80031_gpio_get;

	ret = gpiochip_add(&tps80031->gpio);
	if (ret)
		dev_warn(tps80031->dev, "GPIO registration failed: %d\n", ret);
}

static int __remove_subdev(struct device *dev, void *unused)
{
	platform_device_unregister(to_platform_device(dev));
	return 0;
}

static int tps80031_remove_subdevs(struct tps80031 *tps80031)
{
	return device_for_each_child(tps80031->dev, NULL, __remove_subdev);
}

static void tps80031_irq_lock(struct irq_data *data)
{
	struct tps80031 *tps80031 = irq_data_get_irq_chip_data(data);

	mutex_lock(&tps80031->irq_lock);
}

static void tps80031_irq_enable(struct irq_data *data)
{
	struct tps80031 *tps80031 = irq_data_get_irq_chip_data(data);
	unsigned int __irq = data->irq - tps80031->irq_base;
	const struct tps80031_irq_data *irq_data = &tps80031_irqs[__irq];

	tps80031->mask_reg[irq_data->mask_reg] &= ~(1 << irq_data->mask_mask);
	tps80031->irq_en |= (1 << __irq);
}

static void tps80031_irq_disable(struct irq_data *data)
{
	struct tps80031 *tps80031 = irq_data_get_irq_chip_data(data);

	unsigned int __irq = data->irq - tps80031->irq_base;
	const struct tps80031_irq_data *irq_data = &tps80031_irqs[__irq];

	tps80031->mask_reg[irq_data->mask_reg] |= (1 << irq_data->mask_mask);
	tps80031->irq_en &= ~(1 << __irq);
}

static void tps80031_irq_sync_unlock(struct irq_data *data)
{
	struct tps80031 *tps80031 = irq_data_get_irq_chip_data(data);
	int i;

	for (i = 0; i < ARRAY_SIZE(tps80031->mask_reg); i++) {
		if (tps80031->mask_reg[i] != tps80031->mask_cache[i]) {
			if (!WARN_ON(tps80031_write(tps80031->dev, SLAVE_ID2,
						TPS80031_INT_MSK_LINE_A + i,
						tps80031->mask_reg[i])))
				if (!WARN_ON(tps80031_write(tps80031->dev,
						SLAVE_ID2,
						TPS80031_INT_MSK_STS_A + i,
						tps80031->mask_reg[i])))
					tps80031->mask_cache[i] =
							tps80031->mask_reg[i];
		}
	}

	mutex_unlock(&tps80031->irq_lock);
}

static irqreturn_t tps80031_irq(int irq, void *data)
{
	struct tps80031 *tps80031 = data;
	int ret = 0;
	u8 tmp[3];
	u32 acks;
	int i;

	for (i = 0; i < 3; i++) {
		ret = tps80031_read(tps80031->dev, SLAVE_ID2,
			TPS80031_INT_STS_A + i,	&tmp[i]);
		if (ret < 0) {
			dev_err(tps80031->dev, "failed to read interrupt "
							"status\n");
			return IRQ_NONE;
		}
		if (tmp[i]) {
			ret = tps80031_write(tps80031->dev, SLAVE_ID2,
					TPS80031_INT_STS_A + i, tmp[i]);
			if (ret < 0) {
				dev_err(tps80031->dev, "failed to write "
							"interrupt status\n");
				return IRQ_NONE;
			}
		}
	}
	acks = (tmp[2] << 16) | (tmp[1] << 8) | tmp[0];
	while (acks) {
		i = __ffs(acks);
		if (tps80031->irq_en & (1 << i))
			handle_nested_irq(tps80031->irq_base + i);
		acks &= ~(1 << i);
	}
	return IRQ_HANDLED;
}

static int __devinit tps80031_irq_init(struct tps80031 *tps80031, int irq,
				int irq_base)
{
	int i, ret;

	if (!irq_base) {
		dev_warn(tps80031->dev, "No interrupt support on IRQ base\n");
		return -EINVAL;
	}

	mutex_init(&tps80031->irq_lock);

	for (i = 0; i < 3; i++) {
		tps80031->mask_reg[i] = 0xFF;
		tps80031->mask_cache[i] = tps80031->mask_reg[i];
		tps80031_write(tps80031->dev, SLAVE_ID2,
					TPS80031_INT_MSK_LINE_A + i,
					tps80031->mask_cache[i]);
		tps80031_write(tps80031->dev, SLAVE_ID2,
					TPS80031_INT_MSK_STS_A + i, 0xFF);
		tps80031_write(tps80031->dev, SLAVE_ID2,
					TPS80031_INT_STS_A + i, 0xFF);
	}

	tps80031->irq_base = irq_base;

	tps80031->irq_chip.name = "tps80031";
	tps80031->irq_chip.irq_enable = tps80031_irq_enable;
	tps80031->irq_chip.irq_disable = tps80031_irq_disable;
	tps80031->irq_chip.irq_bus_lock = tps80031_irq_lock;
	tps80031->irq_chip.irq_bus_sync_unlock = tps80031_irq_sync_unlock;

	for (i = 0; i < ARRAY_SIZE(tps80031_irqs); i++) {
		int __irq = i + tps80031->irq_base;
		irq_set_chip_data(__irq, tps80031);
		irq_set_chip_and_handler(__irq, &tps80031->irq_chip,
					 handle_simple_irq);
		irq_set_nested_thread(__irq, 1);
#ifdef CONFIG_ARM
		irq_set_status_flags(__irq, IRQF_VALID);
#endif
	}

	ret = request_threaded_irq(irq, NULL, tps80031_irq, IRQF_ONESHOT,
				"tps80031", tps80031);
	if (!ret) {
		device_init_wakeup(tps80031->dev, 1);
		enable_irq_wake(irq);
	}

	return ret;
}

static void tps80031_clk32k_enable(struct tps80031 *tps80031, int base_add)
{
	int ret;
	ret = tps80031_update(tps80031->dev, SLAVE_ID1,
			base_add + EXT_CONTROL_CFG_STATE, STATE_ON, STATE_MASK);
	if (!ret)
		ret = tps80031_update(tps80031->dev, SLAVE_ID1,
				base_add + EXT_CONTROL_CFG_TRANS,
				STATE_ON, STATE_MASK);
	if (ret < 0)
		dev_err(tps80031->dev, "Error in updating clock register\n");
}

static void tps80031_clk32k_init(struct tps80031 *tps80031,
			struct tps80031_platform_data *pdata)
{
	struct tps80031_32kclock_plat_data *clk32k_pdata;

	if (!(pdata && pdata->clk32k_pdata))
		return;

	clk32k_pdata = pdata->clk32k_pdata;
	if (clk32k_pdata->en_clk32kao)
		tps80031_clk32k_enable(tps80031, CLK32KAO_BASE_ADD);

	if (clk32k_pdata->en_clk32kg)
		tps80031_clk32k_enable(tps80031, CLK32KG_BASE_ADD);

	if (clk32k_pdata->en_clk32kaudio)
		tps80031_clk32k_enable(tps80031, CLK32KAUDIO_BASE_ADD);
}

static int __devinit tps80031_add_subdevs(struct tps80031 *tps80031,
					  struct tps80031_platform_data *pdata)
{
	struct tps80031_subdev_info *subdev;
	struct platform_device *pdev;
	int i, ret = 0;

	for (i = 0; i < pdata->num_subdevs; i++) {
		subdev = &pdata->subdevs[i];

		pdev = platform_device_alloc(subdev->name, subdev->id);

		pdev->dev.parent = tps80031->dev;
		pdev->dev.platform_data = subdev->platform_data;

		ret = platform_device_add(pdev);
		if (ret)
			goto failed;
	}
	return 0;

failed:
	tps80031_remove_subdevs(tps80031);
	return ret;
}

#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/seq_file.h>
static void print_regs(const char *header, struct seq_file *s,
		int sid, int start_offset, int end_offset)
{
	struct tps80031 *tps80031 = s->private;
	struct tps80031_client *tps = &tps80031->tps_clients[sid];
	uint8_t reg_val;
	int i;
	int ret;

	seq_printf(s, "%s\n", header);
	for (i = start_offset; i <= end_offset; ++i) {
		ret = __tps80031_read(tps->client, i, &reg_val);
		if (ret >= 0)
			seq_printf(s, "Addr = 0x%02x Reg 0x%02x Value 0x%02x\n",
						tps->client->addr, i, reg_val);
	}
	seq_printf(s, "------------------\n");
}

static int dbg_tps_show(struct seq_file *s, void *unused)
{
	seq_printf(s, "TPS80031 Registers\n");
	seq_printf(s, "------------------\n");
	print_regs("VIO Regs",       s, SLAVE_ID1, 0x47, 0x49);
	print_regs("VIO Regs",       s, SLAVE_ID0, 0x49, 0x4A);
	print_regs("SMPS1 Regs",     s, SLAVE_ID1, 0x53, 0x54);
	print_regs("SMPS1 Regs",     s, SLAVE_ID0, 0x55, 0x56);
	print_regs("SMPS1 Regs",     s, SLAVE_ID1, 0x57, 0x57);
	print_regs("SMPS2 Regs",     s, SLAVE_ID1, 0x59, 0x5B);
	print_regs("SMPS2 Regs",     s, SLAVE_ID0, 0x5B, 0x5C);
	print_regs("SMPS2 Regs",     s, SLAVE_ID1, 0x5C, 0x5D);
	print_regs("SMPS3 Regs",     s, SLAVE_ID1, 0x65, 0x68);
	print_regs("SMPS4 Regs",     s, SLAVE_ID1, 0x41, 0x44);
	print_regs("VANA Regs",      s, SLAVE_ID1, 0x81, 0x83);
	print_regs("VRTC Regs",      s, SLAVE_ID1, 0xC3, 0xC4);
	print_regs("LDO1 Regs",      s, SLAVE_ID1, 0x9D, 0x9F);
	print_regs("LDO2 Regs",      s, SLAVE_ID1, 0x85, 0x87);
	print_regs("LDO3 Regs",      s, SLAVE_ID1, 0x8D, 0x8F);
	print_regs("LDO4 Regs",      s, SLAVE_ID1, 0x89, 0x8B);
	print_regs("LDO5 Regs",      s, SLAVE_ID1, 0x99, 0x9B);
	print_regs("LDO6 Regs",      s, SLAVE_ID1, 0x91, 0x93);
	print_regs("LDO7 Regs",      s, SLAVE_ID1, 0xA5, 0xA7);
	print_regs("LDOUSB Regs",    s, SLAVE_ID1, 0xA1, 0xA3);
	print_regs("LDOLN Regs",     s, SLAVE_ID1, 0x95, 0x97);
	print_regs("REGEN1 Regs",    s, SLAVE_ID1, 0xAE, 0xAF);
	print_regs("REGEN2 Regs",    s, SLAVE_ID1, 0xB1, 0xB2);
	print_regs("SYSEN Regs",     s, SLAVE_ID1, 0xB4, 0xB5);
	print_regs("CLK32KAO Regs",  s, SLAVE_ID1, 0xBA, 0xBB);
	print_regs("CLK32KG Regs",   s, SLAVE_ID1, 0xBD, 0xBE);
	print_regs("CLK32KAUD Regs", s, SLAVE_ID1, 0xC0, 0xC1);
	print_regs("INT Regs",       s, SLAVE_ID2, 0xD0, 0xD8);
	print_regs("PREQ Regs",      s, SLAVE_ID1, 0xD7, 0xDF);
	print_regs("MASK_PH Regs",   s, SLAVE_ID1, 0x20, 0x21);
	print_regs("VERNUM Regs",    s, SLAVE_ID1, 0x87, 0x87);
	return 0;
}

static int dbg_tps_open(struct inode *inode, struct file *file)
{
	return single_open(file, dbg_tps_show, inode->i_private);
}

static const struct file_operations debug_fops = {
	.open		= dbg_tps_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static void __init tps80031_debuginit(struct tps80031 *tps)
{
	(void)debugfs_create_file("tps80031", S_IRUGO, NULL,
			tps, &debug_fops);
}
#else
static void __init tps80031_debuginit(struct tps80031 *tpsi)
{
	return;
}
#endif

static int __devexit tps80031_i2c_remove(struct i2c_client *client)
{
	struct tps80031 *tps80031 = i2c_get_clientdata(client);
	int i;

	if (client->irq)
		free_irq(client->irq, tps80031);

	if (tps80031->gpio.owner != NULL)
		if (gpiochip_remove(&tps80031->gpio) < 0)
			dev_err(&client->dev, "Error in removing the gpio driver\n");

	for (i = 0; i < TPS_NUM_SLAVES; i++) {
		struct tps80031_client *tps = &tps80031->tps_clients[i];
		if (tps->client && tps->client != client)
			i2c_unregister_device(tps->client);
		tps80031->tps_clients[i].client = NULL;
		mutex_destroy(&tps->lock);
	}

	kfree(tps80031);
	return 0;
}

static int __devinit tps80031_i2c_probe(struct i2c_client *client,
					const struct i2c_device_id *id)
{
	struct tps80031_platform_data *pdata = client->dev.platform_data;
	struct tps80031 *tps80031;
	struct tps80031_client *tps;
	int ret;
	int i;

	if (!pdata) {
		dev_err(&client->dev, "tps80031 requires platform data\n");
		return -ENOTSUPP;
	}

	ret = i2c_smbus_read_byte_data(client, TPS80031_JTAGVERNUM);
	if (ret < 0) {
		dev_err(&client->dev, "Silicon version number read"
				" failed: %d\n", ret);
		return -EIO;
	}

	dev_info(&client->dev, "VERNUM is %02x\n", ret);

	tps80031 = kzalloc(sizeof(struct tps80031), GFP_KERNEL);
	if (tps80031 == NULL)
		return -ENOMEM;

	tps80031->dev = &client->dev;
	i2c_set_clientdata(client, tps80031);

	/* Set up slaves */
	tps80031->tps_clients[SLAVE_ID0].addr = I2C_ID0_ADDR;
	tps80031->tps_clients[SLAVE_ID1].addr = I2C_ID1_ADDR;
	tps80031->tps_clients[SLAVE_ID2].addr = I2C_ID2_ADDR;
	tps80031->tps_clients[SLAVE_ID3].addr = I2C_ID3_ADDR;
	for (i = 0; i < TPS_NUM_SLAVES; i++) {
		tps = &tps80031->tps_clients[i];
		if (tps->addr == client->addr)
			tps->client = client;
		else
			tps->client = i2c_new_dummy(client->adapter,
						tps->addr);
		if (!tps->client) {
			dev_err(&client->dev, "can't attach client %d\n", i);
			ret = -ENOMEM;
			goto fail;
		}
		i2c_set_clientdata(tps->client, tps80031);
		mutex_init(&tps->lock);
	}

	if (client->irq) {
		ret = tps80031_irq_init(tps80031, client->irq,
					pdata->irq_base);
		if (ret) {
			dev_err(&client->dev, "IRQ init failed: %d\n", ret);
			goto fail;
		}
	}

	ret = tps80031_add_subdevs(tps80031, pdata);
	if (ret) {
		dev_err(&client->dev, "add devices failed: %d\n", ret);
		goto fail;
	}

	tps80031_gpio_init(tps80031, pdata);

	tps80031_clk32k_init(tps80031, pdata);

	tps80031_debuginit(tps80031);

	tps80031_dev = tps80031;

	return 0;

fail:
	tps80031_i2c_remove(client);
	return ret;
}

#ifdef CONFIG_PM
static int tps80031_i2c_suspend(struct i2c_client *client, pm_message_t state)
{
	if (client->irq)
		disable_irq(client->irq);
	return 0;
}

static int tps80031_i2c_resume(struct i2c_client *client)
{
	if (client->irq)
		enable_irq(client->irq);
	return 0;
}
#endif


static const struct i2c_device_id tps80031_id_table[] = {
	{ "tps80031", 0 },
	{ },
};
MODULE_DEVICE_TABLE(i2c, tps80031_id_table);

static struct i2c_driver tps80031_driver = {
	.driver	= {
		.name	= "tps80031",
		.owner	= THIS_MODULE,
	},
	.probe		= tps80031_i2c_probe,
	.remove		= __devexit_p(tps80031_i2c_remove),
#ifdef CONFIG_PM
	.suspend	= tps80031_i2c_suspend,
	.resume		= tps80031_i2c_resume,
#endif
	.id_table	= tps80031_id_table,
};

static int __init tps80031_init(void)
{
	return i2c_add_driver(&tps80031_driver);
}
subsys_initcall(tps80031_init);

static void __exit tps80031_exit(void)
{
	i2c_del_driver(&tps80031_driver);
}
module_exit(tps80031_exit);

MODULE_DESCRIPTION("TPS80031 core driver");
MODULE_LICENSE("GPL");
