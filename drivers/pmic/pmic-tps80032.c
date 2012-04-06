/*
 * drivers/pmic/pmic-tps80032.c
 *
 * TPS80032 PMIC Driver
 *
 * Copyright (C) 2012 Renesas Mobile Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */
 
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/workqueue.h>
#include <linux/i2c.h>
#include <linux/io.h>
#include <linux/power_supply.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/timer.h>
#include <asm/atomic.h>
#include <mach/r8a73734.h>
#include <mach/common.h>
#include <linux/jiffies.h>
#include <linux/pmic/pmic.h>
#include <linux/pmic/pmic-tps80032.h>


#ifdef PMIC_DEBUG_ENABLE
#define PMIC_DEBUG_MSG(...) printk(KERN_DEBUG __VA_ARGS__)
#define PMIC_ERROR_MSG(...) printk(KERN_ERR __VA_ARGS__)
#else
#define PMIC_DEBUG_MSG(...) while(0)
#define PMIC_ERROR_MSG(...) while(0)
#endif

static void __iomem *virt_addr   = NULL;
static struct timer_list bat_timer;
static void tps80032_battery_timer_handler(unsigned long data);

struct tps80032_data {
	int device;
	int charger;
	int bat_volt;
	int bat_temp;
	int bat_over_volt;
	int bat_over_temp;
	int bat_capacity;
	int bat_presence;
	int stop_charge;
	int rscounter[RESOURCE_COUNTER_MAX];
	struct mutex smps1_lock;
	struct mutex smps2_lock;
	struct mutex smps3_lock;
	struct mutex smps4_lock;
	struct mutex smps5_lock;
	struct mutex ldo1_lock;
	struct mutex ldo2_lock;
	struct mutex ldo3_lock;
	struct mutex ldo4_lock;
	struct mutex ldo5_lock;
	struct mutex ldo6_lock;
	struct mutex ldo7_lock;
	struct mutex force_off_lock;
	struct mutex vbus_lock;
	struct mutex rscounter_lock;
	struct workqueue_struct *queue;
	struct i2c_client *client_power;
	struct i2c_client *client_battery;
	struct i2c_client *client_dvs;
	struct i2c_client *client_jtag;
	struct work_struct ext_work;
	struct work_struct bat_work;
	struct work_struct interrupt_work;
	struct work_struct resume_work;
	struct irq_chip		irq_chip;
	int	irq_base;
};

static struct tps80032_data *data = NULL;

/*
 * tps80032_init_timer: inital the battery timer
 * @data: The struct which handles the TPS80032 data.
 * return: void
 */
static void tps80032_init_timer(struct tps80032_data *data)
{
	PMIC_DEBUG_MSG(">>> %s start\n", __func__);
	
	init_timer(&bat_timer);
	bat_timer.function = tps80032_battery_timer_handler;
	bat_timer.data = (unsigned long)data;
	
	PMIC_DEBUG_MSG("%s end <<<\n", __func__);
}


/*
 * tps80032_battery_timer_handler: handle the battery timer interrupt
 * @data: 
 * return: void
 */
static void tps80032_battery_timer_handler(unsigned long data)
{
	struct tps80032_data *pdata = (struct tps80032_data*)data;
	
	PMIC_DEBUG_MSG(">>> %s start\n", __func__);
	
	/* Start timer to update battery info */
	queue_work(pdata->queue,&pdata->bat_work);
	bat_timer.expires  = jiffies + msecs_to_jiffies(CONST_TIMER_BATTERY_UPDATE);
	add_timer(&bat_timer);
	
	PMIC_DEBUG_MSG("%s end <<<\n", __func__);
}

/*
 * tps80032_irq_lock: handle the mutex for interrupt
 * @data: The struct irq_data
 * return: void
 */
static void tps80032_irq_lock(struct irq_data *data)
{
	return;
}

/*
 * tps80032_irq_enable: enable interrupt
 * @data: The struct irq_data
 * return: void
 */
static void tps80032_irq_enable(struct irq_data *data)
{
	return;
}

/*
 * tps80032_irq_disable: disable interrupt
 * @data: The struct irq_data
 * return: void
 */
static void tps80032_irq_disable(struct irq_data *data)
{
	return;
}

/*
 * tps80032_irq_sync_unlock: unclock mutex for interrupt
 * @data: The struct irq_data
 * return: void
 */
static void tps80032_irq_sync_unlock(struct irq_data *data)
{
	return;
}

/*
 * tps80032_interrupt_work: handle the PMIC interrupt
 * @work: The struct work.
 * return: void
 */
static void tps80032_interrupt_work(struct work_struct *work)
{
	int sts_c = 0;
	int ret;
	
	struct tps80032_data *data = container_of(work, struct tps80032_data, interrupt_work);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	/* Define the interrupt source */
	/* Read status interrupt C */
	ret = i2c_smbus_read_byte_data(data->client_battery, HW_REG_INT_STS_C);
	if (0 > ret) {
		PMIC_DEBUG_MSG("%s: i2c_smbus_read_byte_data failed err=%d\n",__func__,sts_c);
		goto exit;
	}
	
	/* Update value of interrupt register C */
	sts_c = ret & MSK_GET_INT_SRC;
	
	/* Clear interrupt source */
	ret = i2c_smbus_write_byte_data(data->client_battery, HW_REG_INT_STS_A, MSK_DISABLE);
	if (0 > ret) {
		PMIC_DEBUG_MSG("%s: i2c_smbus_write_byte_data failed err=%d\n",__func__,sts_c);
		goto exit;
	}	
	
	ret = i2c_smbus_write_byte_data(data->client_battery, HW_REG_INT_STS_B, MSK_DISABLE);
	if (0 > ret) {
		PMIC_DEBUG_MSG("%s: i2c_smbus_write_byte_data failed err=%d\n",__func__,sts_c);
		goto exit;
	}	
	
	ret = i2c_smbus_write_byte_data(data->client_battery, HW_REG_INT_STS_C, MSK_DISABLE);
	if (0 > ret) {
		PMIC_DEBUG_MSG("%s: i2c_smbus_write_byte_data failed err=%d\n",__func__,sts_c);
		goto exit;
	}

	/* Process interrupt source */
	/* interrupt source relate to battery */
	if ((MSK_BIT_6 == sts_c) || (MSK_BIT_4 == sts_c)) {
		queue_work(data->queue,&data->bat_work);
	} else if ((MSK_BIT_2 == sts_c) || (MSK_BIT_3 == sts_c)) {
		/* interrupt source relate to external device */
		queue_work(data->queue,&data->ext_work);
	}

	/* Notify when hava an interrupt signal */
	pmic_power_supply_changed(E_USB_STATUS_CHANGED|E_BATTERY_STATUS_CHANGED);

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);
	return;
	
exit:	
	return;
}



/*
 * tps80032_interrupt_handler: Interrupt handler for TPS80032
 * @irq: The interrupt id request
 * @dev_id: The device ID
 * return:
 *        irqreturn_t: the interrupt request handler.
 */
static irqreturn_t tps80032_interrupt_handler(int irq, void *dev_id)
{
	struct tps80032_data *data = (struct tps80032_data *)dev_id;

	PMIC_DEBUG_MSG("%s: irq=%d\n", __func__, irq);

	queue_work(data->queue,&data->interrupt_work);
	
	return IRQ_HANDLED;
}

/*
 * tps80032_check_state_valid: check the valid of power state 
 * @pstate: The power state.
 * return:
 * 			>=0: the valid power state
 * 			<0 : error code
 */
static int tps80032_check_state_valid(int pstate)
{
	int ret;

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	switch(pstate) {
		case E_POWER_OFF:
			ret = 0x00;
			break;
		case E_POWER_ON:
			ret = 0x01;
			break;
		case E_POWER_SLEEP:
			ret = -ENOTSUPP;
			break;
		default:
			ret = -EINVAL;
			break;
	}

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);
	return ret;
}

/*
 * tps80032_set_smps4_power_state: change power state for SMPS4
 * @dev: an i2c client
 * @pstate: The power state.
 * return:
 * 			=0: change successful
 * 			<0: change failed
 */
static int tps80032_set_smps4_power_state(struct device *dev, int pstate)
{
	int ret;
	int val;
	struct i2c_client *client = to_i2c_client(dev);
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	mutex_lock(&data->smps4_lock);
	ret = tps80032_check_state_valid(pstate);
	if (0 > ret) {
		goto exit;
	}

	pstate = ret; /* pstate now contains value corresponding to register value. */

	ret = i2c_smbus_read_byte_data(client, HW_REG_SMPS4_CFG_STATE);
	if (0 > ret) {
		goto exit;
	}

	if (pstate != (ret & MSK_POWER_STATE)) {
		val = ret & (~MSK_POWER_STATE);
		val |= pstate;
		ret = i2c_smbus_write_byte_data(client, HW_REG_SMPS4_CFG_STATE, val);
	} else {
		ret = 0;	/* Nothing to do */
	}

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);

exit:
	mutex_unlock(&data->smps4_lock);
	return ret;
}

/*
 * tps80032_set_ldo1_power_state: change power state for LDO1
 * @dev: an i2c client
 * @pstate: The power state.
 * return:
 * 			=0: change successful
 * 			<0: change failed
 */
static int tps80032_set_ldo1_power_state(struct device *dev, int pstate)
{
	int ret;
	int val;
	struct i2c_client *client = to_i2c_client(dev);
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	mutex_lock(&data->ldo1_lock);
	ret = tps80032_check_state_valid(pstate);

	if (0 > ret) {
		goto exit;
	}

	pstate = ret; /* pstate now contains value corresponding to register value. */

	ret = i2c_smbus_read_byte_data(client, HW_REG_LDO1_CFG_STATE);
	if (0 > ret) {
		goto exit;
	}

	if (pstate != (ret & MSK_POWER_STATE)) {
		val = ret & (~MSK_POWER_STATE);
		val |= pstate ;
		ret = i2c_smbus_write_byte_data(client, HW_REG_LDO1_CFG_STATE, val);
	} else {
		ret = 0;	/* Nothing to do */
	}

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);

exit:
	mutex_unlock(&data->ldo1_lock);
	return ret;
}

/*
 * tps80032_set_ldo5_power_state: change power state for LDO5
 * @dev: an i2c client
 * @pstate: The power state.
 * return:
 * 			=0: change successful
 * 			<0: change failed
 */
static int tps80032_set_ldo5_power_state(struct device *dev, int pstate)
{
	int ret;
	int val;
	struct i2c_client *client = to_i2c_client(dev);
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	mutex_lock(&data->ldo5_lock);
	ret = tps80032_check_state_valid(pstate);
	if (0 > ret) {
		goto exit;
	}

	pstate = ret; /* pstate now contains value corresponding to register value. */

	ret = i2c_smbus_read_byte_data(client, HW_REG_LDO5_CFG_STATE);
	if (0 > ret) {
		goto exit;
	}

	if (pstate != (ret & MSK_POWER_STATE)) {
		val = ret & (~MSK_POWER_STATE);
		val |= pstate;
		ret = i2c_smbus_write_byte_data(client, HW_REG_LDO5_CFG_STATE, val);
	} else {
		ret = 0;	/* Nothing to do */
	}

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);

exit:
	mutex_unlock(&data->ldo5_lock);
	return ret;
}

/*
 * tps80032_set_ldo7_power_state: change power state for LDO7
 * @dev: an i2c client
 * @pstate: The power state.
 * return:
 * 			=0: change successful
 * 			<0: change failed
 */
static int tps80032_set_ldo7_power_state(struct device *dev, int pstate)
{
	int ret;
	int val;
	struct i2c_client *client = to_i2c_client(dev);
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	mutex_lock(&data->ldo5_lock);
	ret = tps80032_check_state_valid(pstate);
	if (0 > ret) {
		goto exit;
	}

	pstate = ret; /* pstate now contains value corresponding to register value. */

	ret = i2c_smbus_read_byte_data(client, HW_REG_LDO7_CFG_STATE);
	if (0 > ret) {
		goto exit;
	}

	if (pstate != (ret & MSK_POWER_STATE)) {
		val = ret & (~MSK_POWER_STATE);
		val |= pstate;
		ret = i2c_smbus_write_byte_data(client, HW_REG_LDO7_CFG_STATE, val);
	} else {
		ret = 0;	/* Nothing to do */
	}

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);

exit:
	mutex_unlock(&data->ldo5_lock);
	return ret;
}


/*
 * tps80032_set_power_on: turn on a power resource
 * @dev: an i2c client
 * @resource: the resource to be turned on
 * return:
 *         =0: normal
 *         <0: error
 */
static int tps80032_set_power_on(struct device *dev, int resource)
{
	int ret = 0;
	struct i2c_client *client = to_i2c_client(dev);
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	mutex_lock(&data->rscounter_lock);
	if (data->rscounter[resource] == 0) {
		switch(resource) {
			case E_POWER_VCORE:
			case E_POWER_VIO2:
			case E_POWER_VIO1:
			case E_POWER_VCORE_RF:
			case E_POWER_VDIG_RF:
			case E_POWER_VDDR:
			case E_POWER_VMIPI:
			case E_POWER_VMMC:
			case E_POWER_ALL:
				ret = -ENOTSUPP;
				break;
			case E_POWER_VANA1_RF:
				ret = tps80032_set_smps4_power_state(dev, E_POWER_ON);
				break;
			case E_POWER_VIO_SD:
				ret = tps80032_set_ldo1_power_state(dev, E_POWER_ON);
				break;
			case E_POWER_VANA_MM:
				ret = tps80032_set_ldo5_power_state(dev, E_POWER_ON);
				break;
			case E_POWER_VUSIM1:
				ret = tps80032_set_ldo7_power_state(dev, E_POWER_ON);
				break;
			default:
				ret = -EINVAL;
				break;
		}
	}
	data->rscounter[resource] = data->rscounter[resource] + 1;
	mutex_unlock(&data->rscounter_lock);

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);
	return ret;
}



/*
 * tps80032_set_power_off: turn off a power resource
 * @dev: an i2c client
 * @resource: the resource to be turned off
 * return:
 *         =0: normal
 *         <0: error
 */
static int tps80032_set_power_off(struct device *dev, int resource)
{
	int ret = 0;
	struct i2c_client *client = to_i2c_client(dev);
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	mutex_lock(&data->rscounter_lock);
	if (data->rscounter[resource] > 0) {
		data->rscounter[resource] = data->rscounter[resource] - 1;
	}

	if (data->rscounter[resource] == 0) {
		switch(resource) {
			case E_POWER_VCORE:
			case E_POWER_VIO2:
			case E_POWER_VIO1:
			case E_POWER_VCORE_RF:
			case E_POWER_VDIG_RF:
			case E_POWER_VDDR:
			case E_POWER_VMIPI:
			case E_POWER_VMMC:
			case E_POWER_ALL:
				ret = - ENOTSUPP;
				break;
			case E_POWER_VANA1_RF:
				ret = tps80032_set_smps4_power_state(dev, E_POWER_OFF);
				break;
			case E_POWER_VIO_SD:
				ret = tps80032_set_ldo1_power_state(dev, E_POWER_OFF);
				break;
			case E_POWER_VANA_MM:
				ret = tps80032_set_ldo5_power_state(dev, E_POWER_OFF);
				break;
			case E_POWER_VUSIM1:
				ret = tps80032_set_ldo7_power_state(dev, E_POWER_OFF);
				break;
			default:
				ret = -EINVAL;
				break;
		}
	}
	mutex_unlock(&data->rscounter_lock);

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);
	return ret;
}


/*
 * tps80032_force_off_hw: force off PMIC hardware, including all power resources
 * @dev: an i2c client
 * return: void
*/
static void tps80032_force_off_hw(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	i2c_smbus_write_byte_data(client, HW_REG_PHOENIX_DEV_ON, MSK_BIT_0);

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);
}


/*
 * tps80032_force_power_off: force off power resources
 * @dev: an i2c client
 * @resource: the power resource to turn off
 * return:
 *         =0: normal
 *         <0: error
*/
static void tps80032_force_power_off(struct device *dev, int resource)
{
	int count = 0;
	struct i2c_client *client = to_i2c_client(dev);
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	/* Reset counter of power resource */
	mutex_lock(&data->rscounter_lock);
	if (resource == E_POWER_ALL) {
		while (count < RESOURCE_COUNTER_MAX) {
			data->rscounter[count] = 0;
			count = count + 1;
		}
	} else {
		data->rscounter[resource] = 0;
	}
	mutex_unlock(&data->rscounter_lock);

	switch(resource) {
		case E_POWER_VCORE:
		case E_POWER_VIO2:
		case E_POWER_VIO1:
		case E_POWER_VCORE_RF:
		case E_POWER_VDIG_RF:
		case E_POWER_VDDR:
		case E_POWER_VMIPI:
		case E_POWER_VMMC:
			break;
		case E_POWER_VANA1_RF:
			(void)tps80032_set_smps4_power_state(dev, E_POWER_OFF);
			break;
		case E_POWER_VIO_SD:
			(void)tps80032_set_ldo1_power_state(dev, E_POWER_OFF);
			break;
		case E_POWER_VANA_MM:
			(void)tps80032_set_ldo5_power_state(dev, E_POWER_OFF);
			break;
		case E_POWER_VUSIM1:
			(void)tps80032_set_ldo7_power_state(dev, E_POWER_OFF);
			break;
		case E_POWER_ALL:
			tps80032_force_off_hw(dev);
			break;
		default:
			break;
	}

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);
	return;
}

/*
 * tps80032_check_ldo_voltage_valid: check the valid of voltage for LDO resources
 * @voltage: The setting voltage
 * return:
 * 			>=0: the valid voltage
 * 			<0 : error code
 */
static int tps80032_check_ldo_voltage_valid(int voltage)
{
	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	if ((voltage >= E_SMPS_VOLTAGE_0_0000V) && (voltage <= E_SMPS_VOLTAGE_2_1000V)) {
		return -ENOTSUPP;
	}

	if ((voltage < E_SMPS_VOLTAGE_0_0000V) || (voltage > E_LDO_VOLTAGE_3_3000V)) {
		return -EINVAL;
	}

	voltage -= E_SMPS_VOLTAGE_2_1000V;

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);
	return voltage;
}


/*
 * tps80032_set_ldo1_voltage: set voltage for LDO1
 * @dev: an i2c client
 * @voltage: The setting voltage
 * return:
 * 			=0: change successful
 * 			<0: change failed
 */
static int tps80032_set_ldo1_voltage(struct device *dev, int voltage)
{
	int val;
	int bit;
	int ret;
	struct i2c_client *client = to_i2c_client(dev);
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	bit = tps80032_check_ldo_voltage_valid(voltage);
	if (-EINVAL == bit) {
		return -EINVAL;
	}

	if (-ENOTSUPP == bit) {
		return -ENOTSUPP;
	}

	mutex_lock(&data->ldo1_lock);
	ret = i2c_smbus_read_byte_data(client, HW_REG_LDO1_CFG_VOLTAGE);
	if (0 > ret) {
		goto exit;
	}
	if (bit != (ret & MSK_POWER_VOLTAGE)) {
		val = ret & ~MSK_POWER_VOLTAGE;
		val |= bit;
		ret = i2c_smbus_write_byte_data(client, HW_REG_LDO1_CFG_VOLTAGE,val);
	} else {
		ret = 0; /* Nothing to do */
	}

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);

exit:
	mutex_unlock(&data->ldo1_lock);
	return ret;
}

/*
 * tps80032_set_ldo2_voltage: set voltage for LDO2
 * @dev: an i2c client
 * @voltage: The setting voltage
 * return:
 * 			=0: change successful
 * 			<0: change failed
 */
static int tps80032_set_ldo2_voltage(struct device *dev, int voltage)
{
	int val;
	int bit;
	int ret;
	struct i2c_client *client = to_i2c_client(dev);
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	bit = tps80032_check_ldo_voltage_valid(voltage);
	if (-EINVAL == bit) {
		return -EINVAL;
	}

	if (-ENOTSUPP == bit) {
		return -ENOTSUPP;
	}

	mutex_lock(&data->ldo2_lock);
	ret = i2c_smbus_read_byte_data(client, HW_REG_LDO2_CFG_VOLTAGE);
	if (0 > ret) {
		goto exit;
	}
	if (bit != (ret & MSK_POWER_VOLTAGE)) {
		val = ret & ~MSK_POWER_VOLTAGE;
		val |= bit;
		ret = i2c_smbus_write_byte_data(client, HW_REG_LDO2_CFG_VOLTAGE,val);
	} else {
		ret = 0; /* Nothing to do */
	}

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);

exit:
	mutex_unlock(&data->ldo2_lock);
	return ret;
}

/*
 * tps80032_set_ldo4_voltage: set voltage for LDO4
 * @dev: an i2c client
 * @voltage: The setting voltage
 * return:
 * 			=0: change successful
 * 			<0: change failed
 */
static int tps80032_set_ldo4_voltage(struct device *dev, int voltage)
{
	int val;
	int bit;
	int ret;
	struct i2c_client *client = to_i2c_client(dev);
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	bit = tps80032_check_ldo_voltage_valid(voltage);
	if (-EINVAL == bit) {
		return -EINVAL;
	}

	if (-ENOTSUPP == bit) {
		return -ENOTSUPP;
	}

	mutex_lock(&data->ldo4_lock);
	ret = i2c_smbus_read_byte_data(client, HW_REG_LDO4_CFG_VOLTAGE);
	if (0 > ret) {
		goto exit;
	}
	if (bit != (ret & MSK_POWER_VOLTAGE)) {
		val = ret & ~MSK_POWER_VOLTAGE;
		val |= bit;
		ret = i2c_smbus_write_byte_data(client, HW_REG_LDO4_CFG_VOLTAGE,val);
	} else {
		ret = 0; /* Nothing to do */
	}

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);

exit:
	mutex_unlock(&data->ldo4_lock);
	return ret;
}

/*
 * tps80032_set_ldo5_voltage: set voltage for LDO5
 * @dev: an i2c client
 * @voltage: The setting voltage
 * return:
 * 			=0: change successful
 * 			<0: change failed
 */
static int tps80032_set_ldo5_voltage(struct device *dev, int voltage)
{
	int val;
	int bit;
	int ret;
	struct i2c_client *client = to_i2c_client(dev);
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	bit = tps80032_check_ldo_voltage_valid(voltage);
	if (bit == -EINVAL) {
		return -EINVAL;
	}

	if (-ENOTSUPP == bit) {
		return -ENOTSUPP;
	}

	mutex_lock(&data->ldo5_lock);
	ret = i2c_smbus_read_byte_data(client, HW_REG_LDO5_CFG_VOLTAGE);
	if (0 > ret) {
		goto exit;
	}
	if (bit != (ret & MSK_POWER_VOLTAGE)) {
		val = ret & ~MSK_POWER_VOLTAGE;
		val |= bit;
		ret = i2c_smbus_write_byte_data(client, HW_REG_LDO5_CFG_VOLTAGE,val);
	} else {
		ret = 0; /* Nothing to do */
	}

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);

exit:
	mutex_unlock(&data->ldo5_lock);
	return ret;
}

/*
 * tps80032_set_ldo6_voltage: set voltage for LDO6
 * @dev: an i2c client
 * @voltage: The setting voltage
 * return:
 * 			=0: change successful
 * 			<0: change failed
 */
static int tps80032_set_ldo6_voltage(struct device *dev, int voltage)
{
	int val;
	int bit;
	int ret;
	struct i2c_client *client = to_i2c_client(dev);
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	bit = tps80032_check_ldo_voltage_valid(voltage);
	if (bit == -EINVAL) {
		return -EINVAL;
	}

	if (-ENOTSUPP == bit) {
		return -ENOTSUPP;
	}

	mutex_lock(&data->ldo6_lock);
	ret = i2c_smbus_read_byte_data(client, HW_REG_LDO6_CFG_VOLTAGE);
	if (0 > ret) {
		goto exit;
	}
	if (bit != (ret & MSK_POWER_VOLTAGE)) {
		val = ret & ~MSK_POWER_VOLTAGE;
		val |= bit;
		ret = i2c_smbus_write_byte_data(client, HW_REG_LDO6_CFG_VOLTAGE,val);
	} else {
		ret = 0; /* Nothing to do */
	}

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);

exit:
	mutex_unlock(&data->ldo6_lock);
	return ret;
}

/*
 * tps80032_set_ldo7_voltage: set voltage for LDO7
 * @dev: an i2c client
 * @voltage: The setting voltage
 * return:
 * 			=0: change successful
 * 			<0: change failed
 */
static int tps80032_set_ldo7_voltage(struct device *dev, int voltage)
{
	int val;
	int bit;
	int ret;
	struct i2c_client *client = to_i2c_client(dev);
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	bit = tps80032_check_ldo_voltage_valid(voltage);
	if (bit == -EINVAL) {
		return -EINVAL;
	}

	if (-ENOTSUPP == bit) {
		return -ENOTSUPP;
	}

	mutex_lock(&data->ldo7_lock);
	ret = i2c_smbus_read_byte_data(client, HW_REG_LDO7_CFG_VOLTAGE);
	if (0 > ret) {
		goto exit;
	}
	if (bit != (ret & MSK_POWER_VOLTAGE)) {
		val = ret & ~MSK_POWER_VOLTAGE;
		val |= bit;
		ret = i2c_smbus_write_byte_data(client, HW_REG_LDO7_CFG_VOLTAGE,val);
	} else {
		ret = 0; /* Nothing to do */
	}

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);

exit:
	mutex_unlock(&data->ldo7_lock);
	return ret;
}



/*
 * tps80032_set_voltage: set voltage to a power resource
 * @dev: an i2c client
 * @resource: the resource to be set
 * @voltage: the specified voltage
 * return:
 *         =0: normal
 *         <0: error
 */
static int tps80032_set_voltage(struct device *dev, int resource, int voltage)
{
	int ret;

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	switch(resource) {
		case E_POWER_VCORE:
		case E_POWER_VIO2:
		case E_POWER_VIO1:
		case E_POWER_VANA1_RF:
		case E_POWER_VCORE_RF:
		case E_POWER_ALL:
		case E_POWER_VDDR:
			ret = -ENOTSUPP;
			break;
		case E_POWER_VIO_SD: /* LDO1 */
			ret = tps80032_set_ldo1_voltage(dev, voltage);
			break;
		case E_POWER_VDIG_RF: /* LDO2 */
			ret = tps80032_set_ldo2_voltage(dev, voltage);
			break;
		case E_POWER_VMIPI:   /* LDO4 */
			ret = tps80032_set_ldo4_voltage(dev, voltage);
			break;
		case E_POWER_VANA_MM: /* LDO5 */
			ret = tps80032_set_ldo5_voltage(dev, voltage);
			break;
		case E_POWER_VMMC:    /* LDO6 */
			ret = tps80032_set_ldo6_voltage(dev, voltage);
			break;
		case E_POWER_VUSIM1:  /* LDO7 */
			ret = tps80032_set_ldo7_voltage(dev, voltage);
			break;
		default:
			ret = -EINVAL;
		break;
	}

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);
	return ret;
}


/*
 * tps80032_set_smps1_power_mode: change power mode for SMPS1
 * @dev: an i2c client
 * @pstate: The setting state want to change power mode
 * @pmode: The setting mode
 * return:
 * 			=0: change successful
 * 			<0: change failed
 */
static int tps80032_set_smps1_power_mode(struct device *dev, int pstate, int pmode)
{
	int ret;
	int val;
	struct i2c_client *client = to_i2c_client(dev);
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	if (E_POWER_OFF == pstate) {
		ret = -ENOTSUPP;
		goto exit;
	}

	mutex_lock(&data->smps1_lock);
	ret = i2c_smbus_read_byte_data(client, HW_REG_SMPS1_CFG_TRANS);
	if (0 > ret) {
		goto exit;
	}

	switch(pmode) {
		case E_PMODE_OFF:
			if (pstate == E_POWER_ON) {
				ret = -ENOTSUPP;
				goto exit;
			} else if (pstate == E_POWER_SLEEP) {
				val = ret & ~MSK_BIT_2;
			} else {
				ret = -EINVAL;
				goto exit;
			}
			break;
		case E_SMPS_PMODE_AUTO:
			if (pstate == E_POWER_ON) {
				val = ret & ~MSK_BIT_1;
			} else if (pstate == E_POWER_SLEEP) {
				val = ret | MSK_BIT_2;
			} else if (pstate == (E_POWER_ON | E_POWER_SLEEP)) {
			/* only AUTO mode is able to be set AT ONCE in both ON state and SLEEP state, other modes are impossible */
				val = ret & ~MSK_BIT_1;
				val = ret | MSK_BIT_2;
			} else {
				ret = -EINVAL;
				goto exit;
			}
			break;
		case E_SMPS_PMODE_FORCE:
			if (pstate == E_POWER_ON) {
				val = ret | MSK_BIT_1;
			} else if (pstate == E_POWER_SLEEP) {
				ret = -ENOTSUPP;
				goto exit;
			} else {
				ret = -EINVAL;
				goto exit;
			}
			break;
			
		case E_LDO_PMODE_AMS:
		case E_LDO_PMODE_ACTIVE:
			ret = -ENOTSUPP;
			goto exit;
			break;
		default:
			ret = -EINVAL;
			goto exit;
	}

	ret = i2c_smbus_write_byte_data(client, HW_REG_SMPS1_CFG_TRANS, val);
	PMIC_DEBUG_MSG("%s end <<<\n", __func__);

exit:
	mutex_unlock(&data->smps1_lock);
	return ret;
}

/*
 * tps80032_set_smps2_power_mode: change power mode for SMPS2
 * @dev: an i2c client
 * @pstate: The setting state want to change power mode
 * @pmode: The setting mode
 * return:
 * 			=0: change successful
 * 			<0: change failed
 */
static int tps80032_set_smps2_power_mode(struct device *dev, int pstate, int pmode)
{
	int ret;
	int val;
	struct i2c_client *client = to_i2c_client(dev);
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	if (E_POWER_OFF == pstate) {
		ret = -ENOTSUPP;
		goto exit;
	}

	mutex_lock(&data->smps2_lock);
	ret = i2c_smbus_read_byte_data(client, HW_REG_SMPS2_CFG_TRANS);
	if (0 > ret) {
		goto exit;
	}

	switch(pmode) {
		case E_PMODE_OFF:
			if (pstate == E_POWER_ON) {
				ret = -ENOTSUPP;
				goto exit;
			} else if (pstate == E_POWER_SLEEP) {
				val = ret & ~MSK_BIT_2;
			} else {
				ret = -EINVAL;
				goto exit;
			}
			break;
		case E_SMPS_PMODE_AUTO:
			if (pstate == E_POWER_ON) {
				val = ret & ~MSK_BIT_1;
			} else if (pstate == E_POWER_SLEEP) {
				val = ret | MSK_BIT_2;
			} else if (pstate == (E_POWER_ON | E_POWER_SLEEP)) {
			/* only AUTO mode is able to be set AT ONCE in both ON state and SLEEP state, other modes are impossible */
				val = ret & ~MSK_BIT_1;
				val = ret | MSK_BIT_2;
			} else {
				ret = -EINVAL;
				goto exit;
			}
			break;
		case E_SMPS_PMODE_FORCE:
			if (pstate == E_POWER_ON) {
				val = ret | MSK_BIT_1;
			} else if (pstate == E_POWER_SLEEP) {
				ret = -ENOTSUPP;
				goto exit;
			} else {
				ret = -EINVAL;
				goto exit;
			}
			break;
		case E_LDO_PMODE_AMS:
		case E_LDO_PMODE_ACTIVE:
			ret =  -ENOTSUPP;
			goto exit;
			break;
		default:
			ret = -EINVAL;
			goto exit;
	}

	ret = i2c_smbus_write_byte_data(client, HW_REG_SMPS2_CFG_TRANS, val);
	PMIC_DEBUG_MSG("%s end <<<\n", __func__);

exit:
	mutex_unlock(&data->smps2_lock);
	return ret;
}

/*
 * tps80032_set_smps3_power_mode: change power mode for SMPS3
 * @dev: an i2c client
 * @pstate: The setting state want to change power mode
 * @pmode: The setting mode
 * return:
 * 			=0: change successful
 * 			<0: change failed
 */
static int tps80032_set_smps3_power_mode(struct device *dev, int pstate, int pmode)
{
	int ret;
	int val;
	struct i2c_client *client = to_i2c_client(dev);
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	if (E_POWER_OFF == pstate) {
		ret = -ENOTSUPP;
		goto exit;
	}

	mutex_lock(&data->smps3_lock);
	ret = i2c_smbus_read_byte_data(client, HW_REG_SMPS3_CFG_TRANS);
	if (0 > ret) {
		goto exit;
	}

	switch(pmode) {
		case E_PMODE_OFF:
			if (pstate == E_POWER_ON) {
				ret = -ENOTSUPP;
				goto exit;
			} else if (pstate == E_POWER_SLEEP) {
				val = ret & ~MSK_BIT_2;
			} else {
				ret = -EINVAL;
				goto exit;
			}
			break;
		case E_SMPS_PMODE_AUTO:
			if (pstate == E_POWER_ON) {
				val = ret & ~MSK_BIT_1;
			} else if (pstate == E_POWER_SLEEP) {
				val = ret | MSK_BIT_2;
			} else if (pstate == (E_POWER_ON | E_POWER_SLEEP)) {
			/* only AUTO mode is able to be set AT ONCE in both ON state and SLEEP state, other modes are impossible */
				val = ret & ~MSK_BIT_1;
				val = ret | MSK_BIT_2;
			} else {
				ret = -EINVAL;
				goto exit;
			}
			break;
		case E_SMPS_PMODE_FORCE:
			if (pstate == E_POWER_ON) {
				val = ret | MSK_BIT_1;
			} else if (pstate == E_POWER_SLEEP) {
				ret = -ENOTSUPP;
				goto exit;
			} else {
				ret = -EINVAL;
				goto exit;
			}
			break;
		case E_LDO_PMODE_AMS:
		case E_LDO_PMODE_ACTIVE:
			ret =  -ENOTSUPP;
			goto exit;
			break;
		default:
			ret = -EINVAL;
			goto exit;
	}

	ret = i2c_smbus_write_byte_data(client, HW_REG_SMPS3_CFG_TRANS, val);
	PMIC_DEBUG_MSG("%s end <<<\n", __func__);

exit:
	mutex_unlock(&data->smps3_lock);
	return ret;
}

/*
 * tps80032_set_smps4_power_mode: change power mode for SMPS4
 * @dev: an i2c client
 * @pstate: The setting state want to change power mode
 * @pmode: The setting mode
 * return:
 * 			=0: change successful
 * 			<0: change failed
 */
static int tps80032_set_smps4_power_mode(struct device *dev, int pstate, int pmode)
{
	int ret;
	int val;
	struct i2c_client *client = to_i2c_client(dev);
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	if (E_POWER_OFF == pstate) {
		ret = -ENOTSUPP;
		goto exit;
	}

	mutex_lock(&data->smps4_lock);
	ret = i2c_smbus_read_byte_data(client, HW_REG_SMPS4_CFG_TRANS);
	if (0 > ret) {
		goto exit;
	}

	switch(pmode) {
		case E_PMODE_OFF:
			if (pstate == E_POWER_ON) {
				ret = -ENOTSUPP;
				goto exit;
			} else if (pstate == E_POWER_SLEEP) {
				val = ret & ~MSK_BIT_2;
			} else {
				ret = -EINVAL;
				goto exit;
			}
			break;
		case E_SMPS_PMODE_AUTO:
			if (pstate == E_POWER_ON) {
				val = ret & ~MSK_BIT_1;
			} else if (pstate == E_POWER_SLEEP) {
				val = ret | MSK_BIT_2;
			} else if (pstate == (E_POWER_ON | E_POWER_SLEEP)) {
			/* only AUTO mode is able to be set AT ONCE in both ON state and SLEEP state, other modes are impossible */
				val = ret & ~MSK_BIT_1;
				val = ret | MSK_BIT_2;
			} else {
				ret = -EINVAL;
				goto exit;
			}
			break;
		case E_SMPS_PMODE_FORCE:
			if (pstate == E_POWER_ON) {
				val = ret | MSK_BIT_1;
			} else if (pstate == E_POWER_SLEEP) {
				ret = -ENOTSUPP;
				goto exit;
			} else {
				ret = -EINVAL;
				goto exit;
			}
			break;
		case E_LDO_PMODE_AMS:
		case E_LDO_PMODE_ACTIVE:
			ret =  -ENOTSUPP;
			goto exit;
			break;
		default:
			ret = -EINVAL;
			goto exit;
	}

	ret = i2c_smbus_write_byte_data(client, HW_REG_SMPS4_CFG_TRANS, val);
	PMIC_DEBUG_MSG("%s end <<<\n", __func__);

exit:
	mutex_unlock(&data->smps4_lock);
	return ret;
}

/*
 * tps80032_set_smps5_power_mode: change power mode for SMPS5
 * @dev: an i2c client
 * @pstate: The setting state want to change power mode
 * @pmode: The setting mode
 * return:
 * 			=0: change successful
 * 			<0: change failed
 */
static int tps80032_set_smps5_power_mode(struct device *dev, int pstate, int pmode)
{
	int ret;
	int val;
	struct i2c_client *client = to_i2c_client(dev);
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	if (E_POWER_OFF == pstate) {
		ret = -ENOTSUPP;
		goto exit;
	}

	mutex_lock(&data->smps5_lock);
	ret = i2c_smbus_read_byte_data(client, HW_REG_SMPS5_CFG_TRANS);
	if (0 > ret) {
		goto exit;
	}

	switch(pmode) {
		case E_PMODE_OFF:
			if (pstate == E_POWER_ON) {
				ret = -ENOTSUPP;
				goto exit;
			} else if (pstate == E_POWER_SLEEP) {
				val = ret & ~MSK_BIT_2;
			} else {
				ret = -EINVAL;
				goto exit;
			}
			break;
		case E_SMPS_PMODE_AUTO:
			if (pstate == E_POWER_ON) {
				val = ret & ~MSK_BIT_1;
			} else if (pstate == E_POWER_SLEEP) {
				val = ret | MSK_BIT_2;
			} else if (pstate == (E_POWER_ON | E_POWER_SLEEP)) {
			/* only AUTO mode is able to be set AT ONCE in both ON state and SLEEP state, other modes are impossible */
				val = ret & ~MSK_BIT_1;
				val = ret | MSK_BIT_2;
			} else {
				ret = -EINVAL;
				goto exit;
			}
			break;
		case E_SMPS_PMODE_FORCE:
			if (pstate == E_POWER_ON) {
				val = ret | MSK_BIT_1;
			} else if (pstate == E_POWER_SLEEP) {
				ret = -ENOTSUPP;
				goto exit;
			} else {
				ret = -EINVAL;
				goto exit;
			}
			break;
		case E_LDO_PMODE_AMS:
		case E_LDO_PMODE_ACTIVE:
			ret =  -ENOTSUPP;
			goto exit;
			break;
		default:
			ret = -EINVAL;
			goto exit;
	}

	ret = i2c_smbus_write_byte_data(client, HW_REG_SMPS5_CFG_TRANS, val);
	PMIC_DEBUG_MSG("%s end <<<\n", __func__);

exit:
	mutex_unlock(&data->smps5_lock);
	return ret;
}

/*
 * tps80032_set_ldo1_power_mode: change power mode for LDO1
 * @dev: an i2c client
 * @pstate: The setting state want to change power mode
 * @pmode: The setting mode
 * return:
 * 			=0: change successful
 * 			<0: change failed
 */
static int tps80032_set_ldo1_power_mode(struct device *dev, int pstate, int pmode)
{
	int ret;
	int val;
	struct i2c_client *client = to_i2c_client(dev);
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	if (E_POWER_OFF == pstate) {
		ret = -ENOTSUPP;
		goto exit;
	}

	mutex_lock(&data->ldo1_lock);
	ret = i2c_smbus_read_byte_data(client, HW_REG_LDO1_CFG_TRANS);
	if (0 > ret) {
		goto exit;
	}

	switch(pmode) {
		case E_PMODE_OFF:
			if (pstate == E_POWER_ON) {
				ret = -ENOTSUPP;
				goto exit;
			} else if (pstate == E_POWER_SLEEP) {
				val = ret & ~MSK_BIT_2;
			} else {
				ret = -EINVAL;
				goto exit;
			}
			break;
		case E_LDO_PMODE_AMS:
			if (pstate == E_POWER_ON) {
				val = ret & ~MSK_BIT_1;
			} else if (pstate == E_POWER_SLEEP) {
				val = ret | MSK_BIT_2;
			} else if (pstate == (E_POWER_ON | E_POWER_SLEEP)) {
			/* only AUTO mode is able to be set AT ONCE in both ON state and SLEEP state, other modes are impossible */
				val = ret & ~MSK_BIT_1;
				val = ret | MSK_BIT_2;
			} else {
				ret = -EINVAL;
				goto exit;
			}
			break;
		case E_LDO_PMODE_ACTIVE:
			if (pstate == E_POWER_ON) {
				val = ret | MSK_BIT_1;
			} else if (pstate == E_POWER_SLEEP) {
				ret = -ENOTSUPP;
				goto exit;
			} else {
				ret = -EINVAL;
				goto exit;
			}
			break;
		case E_SMPS_PMODE_AUTO:
		case E_SMPS_PMODE_FORCE:
			ret =  -ENOTSUPP;
			goto exit;
			break;
		default:
			ret = -EINVAL;
			goto exit;
	}

	ret = i2c_smbus_write_byte_data(client, HW_REG_LDO1_CFG_TRANS, val);
	PMIC_DEBUG_MSG("%s end <<<\n", __func__);

exit:
	mutex_unlock(&data->ldo1_lock);
	return ret;
}

/*
 * tps80032_set_ldo2_power_mode: change power mode for LDO2
 * @dev: an i2c client
 * @pstate: The setting state want to change power mode
 * @pmode: The setting mode
 * return:
 * 			=0: change successful
 * 			<0: change failed
 */
static int tps80032_set_ldo2_power_mode(struct device *dev, int pstate, int pmode)
{
	int ret;
	int val;
	struct i2c_client *client = to_i2c_client(dev);
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	if (E_POWER_OFF == pstate) {
		ret = -ENOTSUPP;
		goto exit;
	}

	mutex_lock(&data->ldo2_lock);
	ret = i2c_smbus_read_byte_data(client, HW_REG_LDO2_CFG_TRANS);
	if (0 > ret) {
		goto exit;
	}

	switch(pmode) {
		case E_PMODE_OFF:
			if (pstate == E_POWER_ON) {
				ret = -ENOTSUPP;
				goto exit;
			} else if (pstate == E_POWER_SLEEP) {
				val = ret & ~MSK_BIT_2;
			} else {
				ret = -EINVAL;
				goto exit;
			}
			break;
		case E_LDO_PMODE_AMS:
			if (pstate == E_POWER_ON) {
				val = ret & ~MSK_BIT_1;
			} else if (pstate == E_POWER_SLEEP) {
				val = ret | MSK_BIT_2;
			} else if (pstate == (E_POWER_ON | E_POWER_SLEEP)) {
			/* only AUTO mode is able to be set AT ONCE in both ON state and SLEEP state, other modes are impossible */
				val = ret & ~MSK_BIT_1;
				val = ret | MSK_BIT_2;
			} else {
				ret = -EINVAL;
				goto exit;
			}
			break;
		case E_LDO_PMODE_ACTIVE:
			if (pstate == E_POWER_ON) {
				val = ret | MSK_BIT_1;
			} else if (pstate == E_POWER_SLEEP) {
				ret = -ENOTSUPP;
				goto exit;
			} else {
				ret = -EINVAL;
				goto exit;
			}
			break;
		case E_SMPS_PMODE_AUTO:
		case E_SMPS_PMODE_FORCE:
			ret =  -ENOTSUPP;
			goto exit;
			break;
		default:
			ret = -EINVAL;
			goto exit;
	}

	ret = i2c_smbus_write_byte_data(client, HW_REG_LDO2_CFG_TRANS, val);
	PMIC_DEBUG_MSG("%s end <<<\n", __func__);

exit:
	mutex_unlock(&data->ldo2_lock);
	return ret;
}

/*
 * tps80032_set_ldo3_power_mode: change power mode for LDO3
 * @dev: an i2c client
 * @pstate: The setting state want to change power mode
 * @pmode: The setting mode
 * return:
 * 			=0: change successful
 * 			<0: change failed
 */
static int tps80032_set_ldo3_power_mode(struct device *dev, int pstate, int pmode)
{
	int ret;
	int val;
	struct i2c_client *client = to_i2c_client(dev);
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	if (E_POWER_OFF == pstate) {
		ret = -ENOTSUPP;
		goto exit;
	}

	mutex_lock(&data->ldo3_lock);
	ret = i2c_smbus_read_byte_data(client, HW_REG_LDO3_CFG_TRANS);
	if (0 > ret) {
		goto exit;
	}

	switch(pmode) {
		case E_PMODE_OFF:
			if (pstate == E_POWER_ON) {
				ret = -ENOTSUPP;
				goto exit;
			} else if (pstate == E_POWER_SLEEP) {
				val = ret & ~MSK_BIT_2;
			} else {
				ret = -EINVAL;
				goto exit;
			}
			break;
		case E_LDO_PMODE_AMS:
			if (pstate == E_POWER_ON) {
				val = ret & ~MSK_BIT_1;
			} else if (pstate == E_POWER_SLEEP) {
				val = ret | MSK_BIT_2;
			} else if (pstate == (E_POWER_ON | E_POWER_SLEEP)) {\
			/* only AUTO mode is able to be set AT ONCE in both ON state and SLEEP state, other modes are impossible */
				val = ret & ~MSK_BIT_1;
				val = ret | MSK_BIT_2;
			} else {
				ret = -EINVAL;
				goto exit;
			}
			break;
		case E_LDO_PMODE_ACTIVE:
			if (pstate == E_POWER_ON) {
				val = ret | MSK_BIT_1;
			} else if (pstate == E_POWER_SLEEP) {
				ret = -ENOTSUPP;
				goto exit;
			} else {
				ret = -EINVAL;
				goto exit;
			}
			break;
		case E_SMPS_PMODE_AUTO:
		case E_SMPS_PMODE_FORCE:
			ret =  -ENOTSUPP;
			goto exit;
			break;
		default:
			ret = -EINVAL;
			goto exit;
	}

	ret = i2c_smbus_write_byte_data(client, HW_REG_LDO3_CFG_TRANS, val);
	PMIC_DEBUG_MSG("%s end <<<\n", __func__);

exit:
	mutex_unlock(&data->ldo3_lock);
	return ret;
}

/*
 * tps80032_set_ldo4_power_mode: change power mode for LDO4
 * @dev: an i2c client
 * @pstate: The setting state want to change power mode
 * @pmode: The setting mode
 * return:
 * 			=0: change successful
 * 			<0: change failed
 */
static int tps80032_set_ldo4_power_mode(struct device *dev, int pstate, int pmode)
{
	int ret;
	int val;
	struct i2c_client *client = to_i2c_client(dev);
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	if (E_POWER_OFF == pstate) {
		ret = -ENOTSUPP;
		goto exit;
	}

	mutex_lock(&data->ldo4_lock);
	ret = i2c_smbus_read_byte_data(client, HW_REG_LDO4_CFG_TRANS);
	if (0 > ret) {
		goto exit;
	}

	switch(pmode) {
		case E_PMODE_OFF:
			if (pstate == E_POWER_ON) {
				ret = -ENOTSUPP;
				goto exit;
			} else if (pstate == E_POWER_SLEEP) {
				val = ret & ~MSK_BIT_2;
			} else {
				ret = -EINVAL;
				goto exit;
			}
			break;
		case E_LDO_PMODE_AMS:
			if (pstate == E_POWER_ON) {
				val = ret & ~MSK_BIT_1;
			} else if (pstate == E_POWER_SLEEP) {
				val = ret | MSK_BIT_2;
			} else if (pstate == (E_POWER_ON | E_POWER_SLEEP)) {
			/* only AUTO mode is able to be set AT ONCE in both ON state and SLEEP state, other modes are impossible */
				val = ret & ~MSK_BIT_1;
				val = ret | MSK_BIT_2;
			} else {
				ret = -EINVAL;
				goto exit;
			}
			break;
		case E_LDO_PMODE_ACTIVE:
			if (pstate == E_POWER_ON) {
				val = ret | MSK_BIT_1;
			} else if (pstate == E_POWER_SLEEP) {
				ret = -ENOTSUPP;
				goto exit;
			} else {
				ret = -EINVAL;
				goto exit;
			}
			break;
			
		case E_SMPS_PMODE_AUTO:
		case E_SMPS_PMODE_FORCE:
			ret =  -ENOTSUPP;
			goto exit;
			break;
		default:
			ret = -EINVAL;
			goto exit;
	}

	ret = i2c_smbus_write_byte_data(client, HW_REG_LDO4_CFG_TRANS, val);
	PMIC_DEBUG_MSG("%s end <<<\n", __func__);

exit:
	mutex_unlock(&data->ldo4_lock);
	return ret;
}

/*
 * tps80032_set_ldo5_power_mode: change power mode for LDO5
 * @dev: an i2c client
 * @pstate: The setting state want to change power mode
 * @pmode: The setting mode
 * return:
 * 			=0: change successful
 * 			<0: change failed
 */
static int tps80032_set_ldo5_power_mode(struct device *dev, int pstate, int pmode)
{
	int ret;
	int val;
	struct i2c_client *client = to_i2c_client(dev);
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	if (E_POWER_OFF == pstate) {
		ret = -ENOTSUPP;
		goto exit;
	}

	mutex_lock(&data->ldo5_lock);
	ret = i2c_smbus_read_byte_data(client, HW_REG_LDO5_CFG_TRANS);
	if (0 > ret) {
		goto exit;
	}

	switch(pmode) {
		case E_PMODE_OFF:
			if (pstate == E_POWER_ON) {
				ret = -ENOTSUPP;
				goto exit;
			} else if (pstate == E_POWER_SLEEP) {
				val = ret & ~MSK_BIT_2;
			} else {
				ret = -EINVAL;
				goto exit;
			}
			break;
		case E_LDO_PMODE_AMS:
			if (pstate == E_POWER_ON) {
				val = ret & ~MSK_BIT_1;
			} else if (pstate == E_POWER_SLEEP) {
				val = ret | MSK_BIT_2;
			} else if (pstate == (E_POWER_ON | E_POWER_SLEEP)) {
			/* only AUTO mode is able to be set AT ONCE in both ON state and SLEEP state, other modes are impossible */
				val = ret & ~MSK_BIT_1;
				val = ret | MSK_BIT_2;
			} else {
				ret = -EINVAL;
				goto exit;
			}
			break;
		case E_LDO_PMODE_ACTIVE:
			if (pstate == E_POWER_ON) {
				val = ret | MSK_BIT_1;
			} else if (pstate == E_POWER_SLEEP) {
				ret = -ENOTSUPP;
				goto exit;
			} else {
				ret = -EINVAL;
				goto exit;
			}
			break;
		case E_SMPS_PMODE_AUTO:
		case E_SMPS_PMODE_FORCE:
			ret =  -ENOTSUPP;
			goto exit;
			break;
		default:
			ret = -EINVAL;
			goto exit;
	}

	ret = i2c_smbus_write_byte_data(client, HW_REG_LDO5_CFG_TRANS, val);
	PMIC_DEBUG_MSG("%s end <<<\n", __func__);

exit:
	mutex_unlock(&data->ldo5_lock);
	return ret;
}

/*
 * tps80032_set_ldo6_power_mode: change power mode for LDO6
 * @dev: an i2c client
 * @pstate: The setting state want to change power mode
 * @pmode: The setting mode
 * return:
 * 			=0: change successful
 * 			<0: change failed
 */
static int tps80032_set_ldo6_power_mode(struct device *dev, int pstate, int pmode)
{
	int ret;
	int val;
	struct i2c_client *client = to_i2c_client(dev);
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	if (E_POWER_OFF == pstate) {
		ret = -ENOTSUPP;
		goto exit;
	}

	mutex_lock(&data->ldo6_lock);
	ret = i2c_smbus_read_byte_data(client, HW_REG_LDO6_CFG_TRANS);
	if (0 > ret) {
		goto exit;
	}

	switch(pmode) {
		case E_PMODE_OFF:
			if (pstate == E_POWER_ON) {
				ret = -ENOTSUPP;
				goto exit;
			} else if (pstate == E_POWER_SLEEP) {
				val = ret & ~MSK_BIT_2;
			} else {
				ret = -EINVAL;
				goto exit;
			}
			break;
		case E_LDO_PMODE_AMS:
			if (pstate == E_POWER_ON) {
				val = ret & ~MSK_BIT_1;
			} else if (pstate == E_POWER_SLEEP) {
				val = ret | MSK_BIT_2;
			} else if (pstate == (E_POWER_ON | E_POWER_SLEEP)) {
			/* only AUTO mode is able to be set AT ONCE in both ON state and SLEEP state, other modes are impossible */
				val = ret & ~MSK_BIT_1;
				val = ret | MSK_BIT_2;
			} else {
				ret = -EINVAL;
				goto exit;
			}
			break;
		case E_LDO_PMODE_ACTIVE:
			if (pstate == E_POWER_ON) {
				val = ret | MSK_BIT_1;
			} else if (pstate == E_POWER_SLEEP) {
				ret = -ENOTSUPP;
				goto exit;
			} else {
				ret = -EINVAL;
				goto exit;
			}
			break;
		case E_SMPS_PMODE_AUTO:
		case E_SMPS_PMODE_FORCE:
			ret =  -ENOTSUPP;
			goto exit;
			break;
		default:
			ret = -EINVAL;
			goto exit;
	}

	ret = i2c_smbus_write_byte_data(client, HW_REG_LDO6_CFG_TRANS, val);
	PMIC_DEBUG_MSG("%s end <<<\n", __func__);

exit:
	mutex_unlock(&data->ldo6_lock);
	return ret;
}

/*
 * tps80032_set_ldo7_power_mode: change power mode for LDO7
 * @dev: an i2c client
 * @pstate: The setting state want to change power mode
 * @pmode: The setting mode
 * return:
 * 			=0: change successful
 * 			<0: change failed
 */
static int tps80032_set_ldo7_power_mode(struct device *dev, int pstate, int pmode)
{
	int ret;
	int val;
	struct i2c_client *client = to_i2c_client(dev);
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	if (E_POWER_OFF == pstate) {
		ret = -ENOTSUPP;
		goto exit;
	}

	mutex_lock(&data->ldo7_lock);
	ret = i2c_smbus_read_byte_data(client, HW_REG_LDO7_CFG_TRANS);
	if (0 > ret) {
		goto exit;
	}

	switch(pmode) {
		case E_PMODE_OFF:
			if (pstate == E_POWER_ON) {
				ret = -ENOTSUPP;
				goto exit;
			} else if (pstate == E_POWER_SLEEP) {
				val = ret & ~MSK_BIT_2;
			} else {
				ret = -EINVAL;
				goto exit;
			}
			break;
		case E_LDO_PMODE_AMS:
			if (pstate == E_POWER_ON) {
				val = ret & ~MSK_BIT_1;
			} else if (pstate == E_POWER_SLEEP) {
				val = ret | MSK_BIT_2;
			} else if (pstate == (E_POWER_ON | E_POWER_SLEEP)) {
			/* only AUTO mode is able to be set AT ONCE in both ON state and SLEEP state, other modes are impossible */
				val = ret & ~MSK_BIT_1;
				val = ret | MSK_BIT_2;
			} else {
				ret = -EINVAL;
				goto exit;
			}
			break;
		case E_LDO_PMODE_ACTIVE:
			if (pstate == E_POWER_ON) {
				val = ret | MSK_BIT_1;
			} else if (pstate == E_POWER_SLEEP) {
				ret = -ENOTSUPP;
				goto exit;
			} else {
				ret = -EINVAL;
				goto exit;
			}
			break;
		case E_SMPS_PMODE_AUTO:
		case E_SMPS_PMODE_FORCE:
			ret =  -ENOTSUPP;
			goto exit;
			break;
		default:
			ret = -EINVAL;
			goto exit;
	}

	ret = i2c_smbus_write_byte_data(client, HW_REG_LDO7_CFG_TRANS, val);
	PMIC_DEBUG_MSG("%s end <<<\n", __func__);

exit:
	mutex_unlock(&data->ldo7_lock);
	return ret;
}


/*
 * tps80032_set_power_mode: set power mode to a power resource
 * @dev: an i2c client
 * @resource: the resource to be set
 * @pmode: the specified power mode
 * return:
 *         =0: normal
 *         <0: error
 */
static int tps80032_set_power_mode(struct device *dev, int resource, int pstate, int pmode)
{
	int ret;

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	switch(resource) {
		case E_POWER_VCORE:     /* SMPS1 */
			ret = tps80032_set_smps1_power_mode(dev, pstate, pmode);
			break;
		case E_POWER_VIO2:      /* SMPS2 */
			ret = tps80032_set_smps2_power_mode(dev, pstate, pmode);
			break;
		case E_POWER_VIO1:      /* SMPS3 */
			ret = tps80032_set_smps3_power_mode(dev, pstate, pmode);
			break;
		case E_POWER_VANA1_RF:  /* SMPS4 */
			ret = tps80032_set_smps4_power_mode(dev, pstate, pmode);
			break;
		case E_POWER_VCORE_RF:  /* SMPS5 */
			ret = tps80032_set_smps5_power_mode(dev, pstate, pmode);
			break;
		case E_POWER_VIO_SD: /* LDO1 */
			ret = tps80032_set_ldo1_power_mode(dev, pstate, pmode);
			break;
		case E_POWER_VDIG_RF: /* LDO2 */
			ret = tps80032_set_ldo2_power_mode(dev, pstate, pmode);
			break;
		case E_POWER_VDDR: /* LDO3 */
			ret = tps80032_set_ldo3_power_mode(dev, pstate, pmode);
			break;
		case E_POWER_VMIPI:   /* LDO4 */
			ret = tps80032_set_ldo4_power_mode(dev, pstate, pmode);
			break;
		case E_POWER_VANA_MM: /* LDO5 */
			ret = tps80032_set_ldo5_power_mode(dev, pstate, pmode);
			break;
		case E_POWER_VMMC:    /* LDO6 */
			ret = tps80032_set_ldo6_power_mode(dev, pstate, pmode);
			break;
		case E_POWER_VUSIM1:  /* LDO7 */
			ret = tps80032_set_ldo7_power_mode(dev, pstate, pmode);
			break;
		case E_POWER_ALL:
			ret = -ENOTSUPP;
			break;
		default:
			ret = -EINVAL;
		break;
	}

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);
	return ret;
}


/*
 * tps80032_set_vbus: enable or disable 5V USB OTG VBUS power supply
 * @dev: an i2c client
 * @enable:
 *          enable = 1: enable 5V VBUS power supply
 *          enable = 0: disable 5V VBUS power supply
 * return:
 *         =0: normal
 *         <0: error
 */
static int tps80032_set_vbus(struct device *dev, int enable)
{
	int ret;
	int val_mode;
	int val_dcdc;
	int val_boost;
	int cur_mode;
	int cur_dcdc;
	int cur_boost;
	struct i2c_client *client = to_i2c_client(dev);
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	mutex_lock(&data->vbus_lock);

	if ((enable != 0) && (enable != 1)) {
		ret = -EINVAL;
		goto exit;
	}

	/* read OPA_MODE bit */
	ret = i2c_smbus_read_byte_data(client, HW_REG_CHARGERUSB_CTRL1);
	if (0 > ret) {
		goto exit;
	}
	cur_mode = ret;
	
	/* read VSYS_SW_CTRL bit */
	ret = i2c_smbus_read_byte_data(client, HW_REG_CHARGERUSB_VSYSREG);
	if (0 > ret) {
		goto exit;
	}
	cur_dcdc = ret;
	
	
	/* read BST_HW_PR_DIS bit */
	ret = i2c_smbus_read_byte_data(client, HW_REG_CHARGERUSB_CTRL3);
	if (0 > ret) {
		goto exit;
	}
	cur_boost = ret;
	
	/* If current mode is boost mode and there's a charger mode request */
	if (enable == 0) {
		/* set charger mode */
		val_mode = cur_mode & ~MSK_BIT_6;
		/* Enable DC-DC Tracking-Mode */
		val_dcdc = cur_dcdc & ~MSK_BIT_7;
		/* Stop boost mode */
		val_boost= cur_boost & ~MSK_BIT_5;
	} else if (enable == 1) {
		/* Set boost mode */
		val_mode = cur_mode | MSK_BIT_6;
		/* Disable DC-DC Tracking-Mode */
		val_dcdc = cur_dcdc | MSK_BIT_7;
		/* Start boost mode */
		val_boost = cur_boost | MSK_BIT_5;
	}
	
	ret = i2c_smbus_write_byte_data(client, HW_REG_CHARGERUSB_CTRL1, val_mode);
	if (0 > ret) {
		goto exit;
	}
	
	ret = i2c_smbus_write_byte_data(client, HW_REG_CHARGERUSB_VSYSREG, val_dcdc);
	if (0 > ret) {
		goto exit;
	}

	ret = i2c_smbus_write_byte_data(client, HW_REG_CHARGERUSB_CTRL3, val_boost);
	if (0 > ret) {
		goto exit;
	}

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);
exit:
	mutex_unlock(&data->vbus_lock);
	return ret;
}

/*
 * tps80032_get_ext_device: get USB devices detected
 * @dev: an i2c client
 * return:
 *         >=0: the connected devices
 *         <0: error
*/
static int tps80032_get_ext_device(struct device *dev)
{
	int device;
	struct i2c_client *client = to_i2c_client(dev);
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	if (!data) {
		device = E_DEVICE_NONE;
	} else {
		device = data->device;
	}

	PMIC_DEBUG_MSG("%s: device=0x%x\n", __func__, device);
	return device;
}


/*
 * tps80032_check_bat_low_1: check the current capacity is low or critical and notify to PMIC interface
 * @old_cap: The previous battery capacity.
 * @new_cap: The current battery capacity.
 * return: void
 */
static void tps80032_check_bat_low_1(int old_cap, int new_cap)
{
	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	if (((THR_BAT_NORMAL <= old_cap) && (THR_BAT_NORMAL > new_cap))
		||((THR_BAT_LOW <= old_cap) && (THR_BAT_NORMAL > old_cap) 
				&& (THR_BAT_LOW > new_cap))) {
			/* Notify the battery change */
			pmic_power_supply_changed(E_BATTERY_STATUS_CHANGED);
	} else {
		/* Do nothing */
		return;
	}

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);
}


/*
 * tps80032_check_bat_low_2: check the current capacity is low or critical and notify to PMIC interface
 * @old: The old state of USB charger
 * @new: The new data of battery
 * return: void
 */
static void tps80032_check_bat_low_2(int old_charger, struct tps80032_data *new)
{
	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	/* If the previous state is charging and the current state is discharging*/
	if ((1 == old_charger) && (0 == new->charger) && (new->bat_capacity < THR_BAT_NORMAL)) {
		/* Notify the battery low  */
		pmic_power_supply_changed(E_BATTERY_STATUS_CHANGED);
	} else {
		/* Do nothing */
		return;
	}
	PMIC_DEBUG_MSG("%s end <<<\n", __func__);
}


/*
 * tps80032_gpadc_correct_temp: correct the battery temperature
 * @temp: the battery temperature
 * return:
 *        > 0: the battery temperature after correct
 */
int tps80032_gpadc_correct_temp(struct tps80032_data *data, int temp)
{
	int d1, d2;
	int offset, gain;
	int ret_trim1, ret_trim2, ret_trim3, ret_trim4;
	int sign_trim1, sign_trim2;
	int result;

	ret_trim1 = i2c_smbus_read_byte_data(data->client_jtag, HW_REG_GPADC_TRIM1);
	if (0 > ret_trim1) {
		result = ret_trim1;
		goto exit;
	}

	ret_trim2 = i2c_smbus_read_byte_data(data->client_jtag, HW_REG_GPADC_TRIM2);
	if (0 > ret_trim2) {
		result = ret_trim2;
		goto exit;
	}

	ret_trim3 = i2c_smbus_read_byte_data(data->client_jtag, HW_REG_GPADC_TRIM3);
	if (0 > ret_trim3) {
		result = ret_trim3;
		goto exit;
	}

	ret_trim4 = i2c_smbus_read_byte_data(data->client_jtag, HW_REG_GPADC_TRIM4);
	if (0 > ret_trim4) {
		result = ret_trim4;
		goto exit;
	}

	sign_trim1 = ret_trim1 & MSK_BIT_0;
	sign_trim2 = ret_trim2 & MSK_BIT_0;

	ret_trim1 = ret_trim1 & (MSK_BIT_2 | MSK_BIT_1);
	ret_trim2 = ret_trim2 & (MSK_BIT_2 | MSK_BIT_1);
	ret_trim3 = ret_trim3 & (MSK_BIT_4 | MSK_BIT_3 | MSK_BIT_2 | MSK_BIT_1 | MSK_BIT_0);
	ret_trim4 = ret_trim4 & (MSK_BIT_5 | MSK_BIT_4 | MSK_BIT_3 | MSK_BIT_2 | MSK_BIT_1 | MSK_BIT_0);

	if (0 == sign_trim1) {
		d1 = ret_trim3 * 4 + ret_trim1;
	} else {
		d1 = ret_trim3 * 4 + ret_trim1;
		d1 = -d1;
	}

	if (0 == sign_trim2) {
		d2 = ret_trim4 * 4 + ret_trim2;
	} else {
		d2 = ret_trim4 * 4 + ret_trim2;
		d2 = -d2;
	}

	gain = 1 + ((d2 - d1) / (CONST_X2 - CONST_X1));
	offset = d1 - (gain - 1) * CONST_X1;

	result = (temp - offset) / gain;

exit:	
	return result;
}

/*
 * tps80032_gpadc_correct_voltage: correct the battery voltage
 * @volt: the battery voltage
 * return:
 *        > 0: the battery voltage after correct
 */
int tps80032_gpadc_correct_voltage(struct tps80032_data *data, int volt)
{
	int d1, d2;
	int ret_temp1, ret_temp2;
	int ret_trim1, ret_trim2, ret_trim3, ret_trim4, ret_trim5, ret_trim6;
	int sign_trim1, sign_trim2, sign_trim5, sign_trim6;
	int offset, gain, result;

	ret_trim1 = i2c_smbus_read_byte_data(data->client_jtag, HW_REG_GPADC_TRIM1);
	if (0 > ret_trim1) {
		result = ret_trim1;
		goto exit;
	}

	ret_trim2 = i2c_smbus_read_byte_data(data->client_jtag, HW_REG_GPADC_TRIM2);
	if (0 > ret_trim2) {
		result = ret_trim2;
		goto exit;
	}

	ret_trim3 = i2c_smbus_read_byte_data(data->client_jtag, HW_REG_GPADC_TRIM3);
	if (0 > ret_trim3) {
		result = ret_trim3;
		goto exit;
	}

	ret_trim4 = i2c_smbus_read_byte_data(data->client_jtag, HW_REG_GPADC_TRIM4);
	if (0 > ret_trim4) {
		result = ret_trim4;
		goto exit;
	}

	ret_trim5 = i2c_smbus_read_byte_data(data->client_jtag, HW_REG_GPADC_TRIM5);
	if (0 > ret_trim1) {
		result = ret_trim5;
		goto exit;
	}

	ret_trim6 = i2c_smbus_read_byte_data(data->client_jtag, HW_REG_GPADC_TRIM6);
	if (0 > ret_trim1) {
		result = ret_trim6;
		goto exit;
	}

	sign_trim1 = ret_trim1 & MSK_BIT_0;
	sign_trim2 = ret_trim2 & MSK_BIT_0;
	sign_trim5 = ret_trim5 & MSK_BIT_0;
	sign_trim6 = ret_trim6 & MSK_BIT_0;

	ret_trim1 = ret_trim1 & (MSK_BIT_2 | MSK_BIT_1);
	ret_trim2 = ret_trim2 & (MSK_BIT_2 | MSK_BIT_1);
	ret_trim3 = ret_trim3 & (MSK_BIT_4 | MSK_BIT_3 | MSK_BIT_2 | MSK_BIT_1 | MSK_BIT_0);
	ret_trim4 = ret_trim4 & (MSK_BIT_5 | MSK_BIT_4 | MSK_BIT_3 | MSK_BIT_2 | MSK_BIT_1 | MSK_BIT_0);
	ret_trim5 = ret_trim5 & (MSK_BIT_6 | MSK_BIT_5 | MSK_BIT_4 | MSK_BIT_3 | MSK_BIT_2 | MSK_BIT_1);
	ret_trim6 = ret_trim6 & (MSK_BIT_7 | MSK_BIT_6 | MSK_BIT_5 | MSK_BIT_4 | MSK_BIT_3 | MSK_BIT_2 | MSK_BIT_1);

	ret_temp1 = ret_trim3 *4 + ret_trim1;
	ret_temp2 = ret_trim5;

	if (1 == sign_trim1) {
		ret_temp1 = -ret_temp1;
	}

	if (1 == sign_trim5) {
		ret_temp2 = -ret_temp2;
	}

	d1 = ret_temp1 + ret_temp2;

	ret_temp1 = ret_trim4 *4 + ret_trim2;
	ret_temp2 = ret_trim6;

	if (1 == sign_trim2) {
		ret_temp1 = -ret_temp1;
	}

	if (1 == sign_trim6) {
		ret_temp2 = -ret_temp2;
	}

	d2 = ret_temp1 + ret_temp2;

	gain = 1 + ((d2 - d1) / (CONST_X2 - CONST_X1));
	offset = d1 - (gain - 1) * CONST_X1;

	result = (volt - offset) / gain;

exit:
	return result;
}

/*
 * tps80032_read_bat_temp: read the battery temperature
 * @client: The I2C client device.
 * return:
 *        > 0: Battery temperature
 *        = 0: Error occurs
 */
int tps80032_read_bat_temp(struct i2c_client *client)
{
	int result = 0;
	int ret;
	int count_timer = 0;
	int ret_MSB, ret_LSB;

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	/*Set 5V scaler and other internal ADC reference */
	ret = i2c_smbus_write_byte_data(client, HW_REG_GPADC_CTRL, 0x6B);
	if (0 > ret) {
		goto exit;
	}

	/*Enable GPADC */
	ret = i2c_smbus_write_byte_data(client, HW_REG_TOGGLE1, 0x0E);
	if (0 > ret) {
		goto exit;
	}

	/*Select TEMP measurement channel */
	ret = i2c_smbus_write_byte_data(client, HW_REG_GPSELECT_ISB, 0x01);
	if (0 > ret) {
		goto exit;
	}

	/*Start GPADC */
	ret = i2c_smbus_write_byte_data(client, HW_REG_CTRL_P1, 0x08);
	if (0 > ret) {
		goto exit;
	}

	/*Wait for ADC interrupt */
	while (count_timer <= CONST_WAIT_TIME) {
		msleep(1);

		/* Check ADC intterupt bit */
		ret = i2c_smbus_read_byte_data(client, HW_REG_INT_STS_B);
		

		if (0 > ret) {
			goto exit;
		} else if (0 != (ret & MSK_BIT_5)) {
			/* Conversion finished */
			/* Clear interrupt source B */
			ret = i2c_smbus_write_byte_data(data->client_battery, HW_REG_INT_STS_B, MSK_DISABLE);
			if (0 > ret) {
				goto disable;
			}
			break;
		} else {
			count_timer++;
			/* Do nothing */
		}
	}

	if (CONST_WAIT_TIME < count_timer) {
		/* Time out */
		PMIC_DEBUG_MSG("%s: measurement conversion failed err=%d\n",__func__,ret);
		goto disable;
	}

	/*Read the VBAT conversion result */
	ret = i2c_smbus_read_byte_data(client, HW_REG_GPCH0_MSB);
	if (0 > ret) {
		goto exit;
	} else {
		ret_MSB = ret;
	}

	ret = i2c_smbus_read_byte_data(client, HW_REG_GPCH0_LSB);
	if (0 > ret) {
		goto exit;
	} else {
		ret_LSB = ret;
	}

	/*Correct the result */
	result = ((ret_MSB & 0x0F)<<8) | ret_LSB;

disable:
	/*Disable GPAD */
	ret = i2c_smbus_write_byte_data(client, HW_REG_TOGGLE1, 0x01);
	if (0 > ret) {
		goto exit;
	}

	/*Disable scaler */
	ret = i2c_smbus_write_byte_data(client, HW_REG_GPADC_CTRL, 0x00);
	if (0 > ret) {
		goto exit;
	}

	if (CONST_WAIT_TIME < count_timer) {
		result =  -1;
	}

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);
	return result;
exit:
	result = ret;
	return result;
}

/*
 * tps80032_read_bat_volt: read the battery voltage
 * @client: The I2C client device.
 * return:
 *        > 0: Battery voltage
 *        = 0: Error occurs
 */
int tps80032_read_bat_volt(struct i2c_client *client)
{
	int result = 0;
	int ret, count_timer = 0;
	int ret_MSB, ret_LSB;

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	/*Set 5V scaler and other internal ADC reference */
	ret = i2c_smbus_write_byte_data(client, HW_REG_GPADC_CTRL, 0x6B);
	if (0 > ret) {
		goto exit;
	}

	/*Set 5V scaler and enable VBAT */
	ret = i2c_smbus_write_byte_data(client, HW_REG_GPADC_CTRL2, 0x0C);
	if (0 > ret) {
		goto exit;
	}

	/*Enable GPADC */
	ret = i2c_smbus_write_byte_data(client, HW_REG_TOGGLE1, 0x0E);
	if (0 > ret) {
		goto exit;
	}

	/*Select VBAT measurement channel */
	ret = i2c_smbus_write_byte_data(client, HW_REG_GPSELECT_ISB, 0x12);
	if (0 > ret) {
		goto exit;
	}

	/*Start GPADC */
	ret = i2c_smbus_write_byte_data(client, HW_REG_CTRL_P1, 0x08);
	if (0 > ret) {
		goto exit;
	}

	/*Wait for ADC interrupt */
	while (count_timer <= CONST_WAIT_TIME) {
		msleep(1);

		/* Check ADC intterupt bit */
		ret = i2c_smbus_read_byte_data(client, HW_REG_INT_STS_B);
		

		if (0 > ret) {
			goto exit;
		} else if (0 != (ret & MSK_BIT_5)) {
			/* Conversion finished */
			/* Clear interrupt source B */
			ret = i2c_smbus_write_byte_data(data->client_battery, HW_REG_INT_STS_B, MSK_DISABLE);
			if (0 > ret) {
				goto disable;
			}
			break;
		} else {
			count_timer++;
			/* Do nothing */
		}
	}

	if (CONST_WAIT_TIME < count_timer) {
		/* Time out */
		PMIC_DEBUG_MSG("%s: measurement conversion failed err=%d\n",__func__,ret);
		goto disable;
	}

	/*Read the VBAT conversion result */
	ret = i2c_smbus_read_byte_data(client, HW_REG_GPCH0_MSB);
	if (0 > ret) {
		goto exit;
	} else {
		ret_MSB = ret;
	}

	ret = i2c_smbus_read_byte_data(client, HW_REG_GPCH0_LSB);
	if (0 > ret) {
		goto exit;
	} else {
		ret_LSB = ret;
	}

	/*Correct the result */
	result = ((ret_MSB & 0x0F)<<8) | ret_LSB;
	result = (result * 5000) / 4096;

disable:
	/*Disable GPAD */
	ret = i2c_smbus_write_byte_data(client, HW_REG_TOGGLE1, 0x01);
	if (0 > ret) {
		goto exit;
	}

	/*Disable scaler */
	ret = i2c_smbus_write_byte_data(client, HW_REG_GPADC_CTRL, 0x00);
	if (0 > ret) {
		goto exit;
	}

	/*Disable scaler */
	ret = i2c_smbus_write_byte_data(client, HW_REG_GPADC_CTRL2, 0x00);
	if (0 > ret) {
		goto exit;
	}

	if (CONST_WAIT_TIME < count_timer) {
		result =  -1;
	}

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);

	goto finish;
exit:
	result = ret;
finish:
	return result;
}


/*
 * tps80032_calc_bat_capacity: calculate the battery capacity
 * @client: The I2C client device.
 * return:
 *        > 0: Battery capacity
 *        = 0: Error occurs
 */
int tps80032_calc_bat_capacity(struct i2c_client *client)
{
	int ret;
	struct tps80032_data *data = i2c_get_clientdata(client);

	if (0 >= data->bat_volt) {
		return data->bat_volt;
	} else {
		if (3150 > data->bat_volt)
			ret = 4;
		else if ((3200 > data->bat_volt) && (3150 <= data->bat_volt))
			ret = 5;
		else if ((3300 > data->bat_volt) && (3200 <= data->bat_volt))
			ret = 20;
		else if ((3400 > data->bat_volt) && (3300 <= data->bat_volt))
			ret = 50;
		else if ((3500 > data->bat_volt) && (3400 <= data->bat_volt))
			ret = 75;
		else if ((3600 > data->bat_volt) && (3500 <= data->bat_volt))
			ret = 90;
		else if (3600 <= data->bat_volt)
			ret = 100;
	}

	return ret;
}

/*
 * tps80032_check_charger: enable/disable charger when the charger is present/not present
 * @data: The struct which handles the TPS80032 data.
 * return: void
 */
void tps80032_check_charger(struct tps80032_data *data)
{
	int ret, val;

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	/* Read the current value of CONTROLLER_CTRL1 register */
	ret = i2c_smbus_read_byte_data(data->client_battery, HW_REG_CONTROLLER_CTRL1);
	if (0 > ret) {
		PMIC_DEBUG_MSG("%s: i2c_smbus_read_byte_data failed err=%d\n",__func__,ret);
		return;
	}

	if (1 == data->charger) {
		/* Enable charger if charger is present*/
		/* Select charge source is USB */
		val = ret | MSK_BIT_4;
		val = val & (~MSK_BIT_3);
	} else {
		/* Disable charger if charger is not present*/
		val  = ret & (~MSK_BIT_4);
	}

	/* Set default value of stop_charge */
	data->stop_charge = 0;

	ret = i2c_smbus_write_byte_data(data->client_battery, HW_REG_CONTROLLER_CTRL1, val);
	if (0 > ret) {
		PMIC_DEBUG_MSG("%s: i2c_smbus_write_byte_data failed err=%d\n",__func__,ret);
		return;
	}

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);
	return;
}

/*
 * tps80032_bat_update: update all battery information
 * @data: The struct which handles the TPS80032 data.
 * return: void
 */
void tps80032_bat_update(struct tps80032_data *data)
{
	int ret, old_data, old_charger;
	int notify = 0;

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	/* Update all battery information */
	/* Read the state of USB charger */
	old_data = data->charger;
	old_charger = data->charger;
	ret = i2c_smbus_read_byte_data(data->client_battery, HW_REG_CONTROLLER_STAT1);
	if (0 > ret) {
		/* Do nothing */
	} else if (0 == (ret & MSK_BIT_2)) {
		data->charger = 0;
	} else {
		data->charger = 1;
	}

	/* check the change of usb charger */
	if (old_data != data->charger) {
		notify = 1;
		tps80032_check_charger(data);
	}

	/* Read the state of battery */
	ret = i2c_smbus_read_byte_data(data->client_battery, HW_REG_CONTROLLER_STAT1);
	if (0 > ret) {
		/* Do nothing */
	} else if (0 == (ret & MSK_BIT_1)) {
		data->bat_presence = 1;
	} else {
		data->bat_presence = 0;
	}

	/* Read the state of battery voltage is over or not */
	ret = i2c_smbus_read_byte_data(data->client_battery, HW_REG_CHARGERUSB_STATUS_INT1);
	if (0 > ret) {
		data->bat_over_volt = -1;
	} else if (0 == (ret & MSK_BIT_3)) {
		data->bat_over_volt = 0;
	} else {
		data->bat_over_volt = 1;
	}

	/* Read the state of battery temperature is over or not */
	ret = i2c_smbus_read_byte_data(data->client_battery, HW_REG_CONTROLLER_STAT1);
	if (0 > ret) {
		data->bat_over_temp = -1;
	} else if (0 == (ret & MSK_BIT_0)) {
		data->bat_over_temp = 0;
	} else {
		data->bat_over_temp = 1;
	}

	/* Get old value of battery voltage */
	old_data = data->bat_volt;

	/* Read the battery voltage */
	ret = tps80032_read_bat_volt(data->client_battery);

	/* Correct the battery voltage */
	if (0 < ret) {
		ret = tps80032_gpadc_correct_voltage(data, ret);
	}

	data->bat_volt = ret;

	/* check the change of battery capacity */
	if ((old_data != data->bat_volt) && (1 != notify)) {
		notify = 1;
	}

	/* Get old value of battery temperature */
	old_data = data->bat_temp;

	/* Read the battery temperature */
	ret = tps80032_read_bat_temp(data->client_battery);

	/* Correct the battery temp */
	if (0 < ret) {
		ret = tps80032_gpadc_correct_temp(data, ret);
	}

	data->bat_temp = ret;

	/* check the change of battery capacity */
	if ((old_data != data->bat_temp ) && (1 != notify)) {
		notify = 1;
	}

	/* Get old value of battery capacity */
	old_data = data->bat_capacity;

	/* Calculate the battery capacity */
	ret = tps80032_calc_bat_capacity(data->client_battery);

	/* Update battery capacity */
	data->bat_capacity = ret;
	/* check the battery capacity is low or critical */
	if (20 > ret) {
		tps80032_check_bat_low_1(old_data, ret);
		tps80032_check_bat_low_2(old_charger, data);
	}

	/* check the change of battery capacity */
	if ((old_data != data->bat_capacity) && (1 != notify)) {
		notify = 1;
	}

	/* Notify if there have any change */
	if (0 != notify) {
		pmic_power_supply_changed(E_USB_STATUS_CHANGED|E_BATTERY_STATUS_CHANGED);
	}

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);
	return;
}

/* WORK QUEUE */


/*
 * tps80032_ext_work: define the external device which is inserted and notify the presence of external device
 * @work: The struct work.
 * return: void
 */
static void tps80032_ext_work(struct work_struct *work)
{
	int ret;
	struct tps80032_data *data = container_of(work, struct tps80032_data, ext_work);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	ret = i2c_smbus_read_byte_data(data->client_battery, HW_REG_USB_ID_INT_SRC);
	if (0 > ret) {
		PMIC_DEBUG_MSG("%s: i2c_smbus_read_byte_data failed err=%d\n",__func__,ret);
		return;
	}

	ret &= MSK_GET_EXT_DEVICE;

	if (MSK_BIT_3 == ret) {
		data->device = E_DEVICE_ACA_ADEVICERID_A;
	} else if (MSK_BIT_2 == ret) {
		data->device = E_DEVICE_ACA_ADEVICERID_B;
	} else if (MSK_BIT_1 == ret) {
		data->device = E_DEVICE_ACA_ADEVICERID_C;
	} else {
		/* Do nothing */
	}

	pmic_ext_device_changed(data->device);
	PMIC_DEBUG_MSG("%s end <<<\n", __func__);

	return;
}


/*
 * tps80032_bat_work: update all battery information and notify the change of battery state
 * @work: The struct work.
 * return: void
 */
static void tps80032_bat_work(struct work_struct *work)
{
	struct tps80032_data *data = container_of(work, struct tps80032_data, bat_work);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	/* call bat_updat function */
	tps80032_bat_update(data);

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);

	return;
}

/*
 * tps80032_resume_work: restart battery timer
 * @work: The struct work.
 * return: void
 */
static void tps80032_resume_work(struct work_struct *work)
{
	struct tps80032_data *data = container_of(work, struct tps80032_data, resume_work);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	PMIC_DEBUG_MSG("%s: name=%s addr=0x%x\n", __func__, data->client_battery->name,data->client_battery->addr);

	/* Restart the battery timer */
	if (!timer_pending(&bat_timer)) {
		bat_timer.expires  = jiffies + msecs_to_jiffies(CONST_TIMER_BATTERY_UPDATE);
		add_timer(&bat_timer);
	}

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);
}


/*
 * tps80032_get_usb_online: get the presence of USB charger
 * @dev: The struct which handles the TPS80032 driver.
 * return:
 *        = 1: USB charger is presence
 *        = 0: USB charger is not presence
 */
static int tps80032_get_usb_online(struct device *dev)
{
	int ret;
	struct i2c_client *client = to_i2c_client(dev);
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	ret = data->charger;

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);
	return ret;
}

/*
 * tps80032_get_bat_status: get the status of battery
 * @dev: The struct which handles the TPS80032 driver.
 * return:
 *        POWER_SUPPLY_STATUS_FULL: the status of battery is full
 *        POWER_SUPPLY_STATUS_CHARGING: the status of battery is charging
 *        POWER_SUPPLY_STATUS_DISCHARGING: the status of battery is discharging
 *        POWER_SUPPLY_STATUS_NOT_CHARGING: the status of battery is not_discharging
 *        POWER_SUPPLY_STATUS_UNKNOWN:  fail to get battery device data
 */
static int tps80032_get_bat_status(struct device *dev)
{
	int ret;
	int ret_cap, ret_charger, ret_bat, ret_stop_charge;
	struct i2c_client *client = to_i2c_client(dev);
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	ret_cap = data->bat_capacity;
	ret_charger = data->charger;
	ret_bat = data->bat_presence;
	ret_stop_charge = data->stop_charge;

	if (0 > ret_cap) {
		/* return UNKNOWN status */
		ret = POWER_SUPPLY_STATUS_UNKNOWN;
	} else if (THR_BAT_FULL == ret_cap) {
		/* return Full status */
		ret = POWER_SUPPLY_STATUS_FULL;
	} else if ((((0 == ret_charger) && (1 == ret_bat))) || (1 == ret_stop_charge)) {
		/* return DISCHARGING status */
		ret = POWER_SUPPLY_STATUS_DISCHARGING;
	} else if ((1 == ret_charger) && (0 == ret_bat)) {
		/* return NOT_CHARGING status */
		ret = POWER_SUPPLY_STATUS_NOT_CHARGING;
	} else {
		/* return CHARGING status */
		ret = POWER_SUPPLY_STATUS_CHARGING;
	}

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);
	return ret;

}

/*
 * tps80032_get_bat_health: get the health of battery
 * @dev: The struct which handles the TPS80032 driver.
 * return:
 *        POWER_SUPPLY_HEALTH_OVERHEAT: the health of battery is overheat
 *        POWER_SUPPLY_HEALTH_OVERVOLTAGE: the health of battery is overvoltage
 *        POWER_SUPPLY_HEALTH_GOOD: the health of battery is good
 *        POWER_SUPPLY_HEALTH_COLD: the health of battery is cold
 *        POWER_SUPPLY_HEALTH_UNKNOWN:  fail to get battery device data
 */
static int tps80032_get_bat_health(struct device *dev)
{
	int ret;
	int ret_overvolt, ret_overtemp, ret_temp;
	struct i2c_client *client = to_i2c_client(dev);
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	ret_overvolt = data->bat_over_volt;
	ret_overtemp = data->bat_over_temp;
	ret_temp = data->bat_temp;

	if ((0 > ret_overvolt) || (0 > ret_overtemp) || (0 > ret_temp)) {
		/* return UNKNOWN health  */
		ret = POWER_SUPPLY_HEALTH_UNKNOWN;
	} else if ( 1 == ret_overvolt) {
		/* return OVERVOLTAGE health  */
		ret = POWER_SUPPLY_HEALTH_OVERVOLTAGE;
	} else if (1 == ret_overtemp) {
		/* return OVERHEAT health  */
		ret = POWER_SUPPLY_HEALTH_OVERHEAT;
	} else if (CONST_0C_DEGREE >= ret_temp) {
		/* return COLD health */
		ret = POWER_SUPPLY_HEALTH_COLD;
	} else {
		/* return GOOD health */
		ret = POWER_SUPPLY_HEALTH_GOOD;
	}

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);
	return ret;
}


/*
 * tps80032_get_bat_present: get the presence of battery
 * @dev: The struct which handles the TPS80032 driver.
 * return:
 *        = 1: Battery is presence
 *        = 0: Battery is not presence
 */
static int tps80032_get_bat_present(struct device *dev)
{
	int ret;
	struct i2c_client *client = to_i2c_client(dev);
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	ret = data->bat_presence;

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);

	/* return the presence of battery */
	return ret;
}

/*
 * tps80032_get_bat_temperature: get the battery temperature
 * @dev: The struct which handles the TPS80032 driver.
 * return:
 *        > 0: the battery temperature
 *        = 0: Error occurs
 */
static int tps80032_get_bat_temperature(struct device *dev)
{
	int ret;
	struct i2c_client *client = to_i2c_client(dev);
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	ret = data->bat_temp;

	if (0 > ret) {
		ret = 0;
	} else {
		/* Do nothing */
	}

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);
	return ret;
}

/*
 * tps80032_get_bat_capacity: get the remaining capacity in percent unit
 * @dev: The struct which handles the TPS80032 driver.
 * return:
 *        > 0: the remaining capacity in percent unit.
 *        = 0: Error occurs
 */
static int tps80032_get_bat_capacity(struct device *dev)
{
	int ret;
	struct i2c_client *client = to_i2c_client(dev);
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	ret = data->bat_capacity;

	if (0 > ret) {
		ret = 0;
	} else {
		/* Do nothing */
	}
	PMIC_DEBUG_MSG("%s end <<<\n", __func__);
	return ret;
}

/*
 * tps80032_get_bat_capacity_level: get the level of remaining capacity
 * @dev: The struct which handles the TPS80032 driver.
 * return:
 *        POWER_SUPPLY_CAPACITY_LEVEL_FULL : the capacity level is FULL 
 *        POWER_SUPPLY_CAPACITY_LEVEL_HIGH : the capacity level is HIGH 
 *        POWER_SUPPLY_CAPACITY_LEVEL_NORMAL : the capacity level is NORMAL
 *        POWER_SUPPLY_CAPACITY_LEVEL_LOW : the capacity level is LOW
 *        POWER_SUPPLY_CAPACITY_LEVEL_CRITICAL : the capacity level is CRITICAL
 *        POWER_SUPPLY_CAPACITY_LEVEL_UNKNOWN:  fail to get battery device data
 */

static int tps80032_get_bat_capacity_level(struct device *dev)
{
	int ret;
	int capacity;
	struct i2c_client *client = to_i2c_client(dev);
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	capacity = data->bat_capacity;

	if (0 > capacity) {
		/* return UNKNOWN level */
		ret = POWER_SUPPLY_CAPACITY_LEVEL_UNKNOWN;
	} else if (THR_BAT_FULL == capacity) {
		/* return FULL level */
		ret = POWER_SUPPLY_CAPACITY_LEVEL_FULL;
	} else if (THR_BAT_HIGH <= capacity) {
		/* return HIGH level */
		ret = POWER_SUPPLY_CAPACITY_LEVEL_HIGH;
	} else if (THR_BAT_NORMAL <= capacity) {
		/* return NORMAL level */
		ret = POWER_SUPPLY_CAPACITY_LEVEL_NORMAL;
	} else if (THR_BAT_LOW <= capacity) {
		/* return LOW level */
		ret = POWER_SUPPLY_CAPACITY_LEVEL_LOW;
	} else {
		/* return CRITICAL level */
		ret = POWER_SUPPLY_CAPACITY_LEVEL_CRITICAL;
	}

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);
	return ret;

}


/*
 * tps80032_get_bat_voltage: get the battery voltage
 * @dev: The struct which handles the TPS80032 driver.
 * return:
 *        > 0: the voltage value of battery.
 *        = 0: Error occurs
 */
static int tps80032_get_bat_voltage(struct device *dev)
{
	int ret;
	struct i2c_client *client = to_i2c_client(dev);
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	ret = data->bat_volt;

	if (0 > ret) {
		ret = 0;
	} else {
		/* Do nothing */
	}

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);
	return ret;
}


/*
 * tps80032_stop_charging: control the battery charging
 * @dev: The struct which handles the TPS80032 driver.
 * @stop: The control command to stop or restart
 * return:
 *        < 0: Error occurs
 *        = 0: Success
 */
static int tps80032_stop_charging(struct device *dev,int stop)
{
	int ret, cal;
	struct i2c_client *client = to_i2c_client(dev);
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	if (( 1 != stop) && (0 != stop)) {
		/* Return error */
		return -EINVAL;
	} else {
		/* read the current value of CONTROLLER_CTRL1 register */
		ret = i2c_smbus_read_byte_data(client, HW_REG_CONTROLLER_CTRL1);

		if (0 > ret) {
			return ret;
		} else if (((1 == stop) && (0 != (ret & MSK_BIT_4)))
				|| ((0 == stop) && (0 == (ret & MSK_BIT_4)))) {
			/* Do nothing */
		} else {
			if (0 == stop) {
				/* Disable charger */
				data->stop_charge = 1;
				cal  = ret & (~MSK_BIT_4);
			} else {
				/* Enable charger */
				data->stop_charge = 0;
				cal = ret | MSK_BIT_4;
			}

			/* write the new value of EN_CHARGER bit */
			ret = i2c_smbus_write_byte_data(client, HW_REG_CONTROLLER_CTRL1, cal);
			
			if (0 > ret) {
				return ret;
			}
		}
		PMIC_DEBUG_MSG("%s end <<<\n", __func__);
		return 0;
	}
}

/*
 * tps80032_correct_temp: change temperature unit from 0.1K to 0.1C
 * @temp: the battery temperature in 0.1K unit
 * return:
 *        the battery temperature in 0.1C unit
 */
static int tps80032_correct_temp(int temp)
{
	int ret = 0;

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	/* change temperature unit from 0.1K to 0.1C */
	ret = temp - CONST_CONVERT_TEMP;

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);
	return ret;	
}


/*
 * tps80032_correct_voltage: change voltage unit from mV to microV
 * @vol: the voltage value in mV unit 
 * return:
 *       >0: the voltage value in microV unit 
 */
static int tps80032_correct_voltage(int vol)
{
	int ret = 0;

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	/* change voltage unit from mV to microV */
	ret = vol*CONST_CONVERT_VOLT;

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);
	return ret;
}


/*
 * tps80032_correct_capacity: correct the battery capacity
 * @capacity: the voltage value in mV unit 
 * return:
 *       >0: the correct capacity
 */
static int tps80032_correct_capacity(int capacity)
{
	int ret = 0;

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	ret = capacity;

	if (THR_BAT_FULL < ret) {
		ret = THR_BAT_FULL;
	} else if (0 >= ret) {
		ret = CONST_BAT_MIN;
	}
	
	PMIC_DEBUG_MSG("%s end <<<\n", __func__);
	return ret;
}



/* COMMON FUNCTION */

/*
 * tps80032_read_register: read the current value of TPS80032 register
 * @dev: The struct which handles the TPS80032 driver.
 * @slave: The I2C slave address.
 * @addr: The address of register.
 * return:
 *        >= 0: Value of register
 *        < 0: Error occurs
 */
static int tps80032_read_register(struct device *dev, int slave, u8 addr)
{
	int ret;
	struct i2c_client *client = to_i2c_client(dev);
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	switch(slave) {
		case E_SLAVE_RTC_POWER:
			ret = i2c_smbus_read_byte_data(data->client_power, addr);
			break;
		case E_SLAVE_BATTERY:
			ret = i2c_smbus_read_byte_data(data->client_battery, addr);
			break;
		case E_SLAVE_DVS:
			ret = i2c_smbus_read_byte_data(data->client_dvs, addr);
			break;
		case E_SLAVE_JTAG:
			ret = i2c_smbus_read_byte_data(data->client_jtag, addr);
			break;
		default:
			ret = -EINVAL;
	}

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);
	return ret;
}

/*
 * tps80032_read_registers: read the current value of TPS80032 registers
 * @dev: The struct which handles the TPS80032 driver.
 * @slave: The I2C slave address.
 * @addr: The address of register.
 * @len: The number of registers is read.
 * @val: The values of registers.
 * return:
 *        = 0: Normal operation
 *        < 0: Error occurs
 */
static int tps80032_read_registers(struct device *dev, int slave, u8 addr, int len, u8 *val)
{
	int ret;
	struct i2c_client *client = to_i2c_client(dev);
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	switch(slave) {
		case E_SLAVE_RTC_POWER:
			ret = i2c_smbus_read_i2c_block_data(data->client_power, addr, len, val);
			break;
		case E_SLAVE_BATTERY:
			ret = i2c_smbus_read_i2c_block_data(data->client_battery, addr, len, val);
			break;
		case E_SLAVE_DVS:
			ret = i2c_smbus_read_i2c_block_data(data->client_dvs, addr, len, val);
			break;
		case E_SLAVE_JTAG:
			ret = i2c_smbus_read_i2c_block_data(data->client_jtag, addr, len, val);
			break;
		default:
			ret = -EINVAL;
	}

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);
	return ret;
}

/*
 * tps80032_write_register: write the value to TPS80032 register
 * @dev: The struct which handles the TPS80032 driver.
 * @slave: The I2C slave address.
 * @addr: The address of register.
 * @val: The value for setting to register.
 * return:
 *        = 0: Normal operation
 *        < 0: Error occurs
 */
static int tps80032_write_register(struct device *dev, int slave, u8 addr, u8 val)
{
	int ret;
	struct i2c_client *client = to_i2c_client(dev);
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	switch(slave) {
		case E_SLAVE_RTC_POWER:
			ret = i2c_smbus_write_byte_data(data->client_power, addr, val);
			break;
		case E_SLAVE_BATTERY:
			ret = i2c_smbus_write_byte_data(data->client_battery, addr, val);
			break;
		case E_SLAVE_DVS:
			ret = i2c_smbus_write_byte_data(data->client_dvs, addr, val);
			break;
		case E_SLAVE_JTAG:
			ret = i2c_smbus_write_byte_data(data->client_jtag, addr, val);
			break;
		default:
			ret = -EINVAL;
	}

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);
	return ret;
}

/*
 * tps80032_write_registers: write the values to TPS80032 registers
 * @dev: The struct which handles the TPS80032 driver.
 * @slave: The I2C slave address.
 * @addr: The address of register.
 * @len: The number of registers is wrote.
 * @val: The value for setting to register.
 * return:
 *        = 0: Normal operation
 *        < 0: Error occurs
 */
static int tps80032_write_registers(struct device *dev, int slave, u8 addr, int len, u8 *val)
{
	int ret;
	struct i2c_client *client = to_i2c_client(dev);
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	switch(slave) {
		case E_SLAVE_RTC_POWER:
			ret = i2c_smbus_write_i2c_block_data(data->client_power, addr, len, val);
			break;
		case E_SLAVE_BATTERY:
			ret = i2c_smbus_write_i2c_block_data(data->client_battery, addr, len, val);
			break;
		case E_SLAVE_DVS:
			ret = i2c_smbus_write_i2c_block_data(data->client_dvs, addr, len, val);
			break;
		case E_SLAVE_JTAG:
			ret = i2c_smbus_write_i2c_block_data(data->client_jtag, addr, len, val);
			break;
		default:
			ret = -EINVAL;
	}

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);
	return ret;
}
/*
 * tps80032_get_temp_status: read the current temperature of battery
 * @dev: The struct which handles the TPS80032 driver.
 * return:
 *        > 0: Battery temperature
 *        = 0: Error occurs
 */
static int tps80032_get_temp_status(struct device *dev)
{
	int ret;
	struct i2c_client *client = to_i2c_client(dev);
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	if (NULL != data) {
		ret = data->bat_temp;
	} else {
		ret = 0;
	}

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);
	return ret;
}

#ifdef PMIC_PT_TEST_ENABLE

/*
 * tps80032_get_batt_status: read the current temperature of battery
 * @dev: The struct which handles the TPS80032 driver.
 * return:
 *        > 0: Battery temperature
 *        = 0: Error occurs
 */
static int tps80032_get_batt_status(struct device *dev, int property)
{
	int ret;
	struct i2c_client *client = to_i2c_client(dev);
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	if (NULL != data) {
		switch (property) {
			case E_BATT_PROP_ONLINE:
				return tps80032_get_usb_online(&data->client_battery->dev);
			case E_BATT_PROP_STATUS:
				return tps80032_get_bat_status(&data->client_battery->dev);
			case E_BATT_PROP_HEALTH:
				return tps80032_get_bat_health(&data->client_battery->dev);
			case E_BATT_PROP_PRESENT:
				return tps80032_get_bat_present(&data->client_battery->dev);
			case E_BATT_PROP_TECHNOLOGY:
				return 0;		/* Not supported by TPS80032 */
			case E_BATT_PROP_CAPACITY:
				return tps80032_get_bat_capacity(&data->client_battery->dev);
			case E_BATT_PROP_VOLTAGE:
				return tps80032_get_bat_voltage(&data->client_battery->dev);
			case E_BATT_PROP_TEMP:
				return tps80032_get_bat_temperature(&data->client_battery->dev);
		}
	} else {
		ret = -1;
	}

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);
	return ret;
}

#endif

/* struct pmic_device_ops */
static struct pmic_device_ops tps80032_power_ops = {
	.set_power_on = tps80032_set_power_on,
	.set_power_off = tps80032_set_power_off,
	.set_voltage = tps80032_set_voltage,
	.set_power_mode = tps80032_set_power_mode,
	.force_power_off = tps80032_force_power_off,
	.get_ext_device = tps80032_get_ext_device,
	.read_register = tps80032_read_register,
	.read_registers = tps80032_read_registers,
	.write_register = tps80032_write_register,
	.write_registers = tps80032_write_registers,
	.get_temp_status = tps80032_get_temp_status,
#ifdef PMIC_PT_TEST_ENABLE
	.get_batt_status = tps80032_get_batt_status,
#endif
};

/* struct tps80032_vbus_ops */
static struct usb_otg_pmic_device_ops tps80032_vbus_ops = {
	.set_vbus = tps80032_set_vbus,
};

/* struct tps80032_power_battery_ops */
static struct pmic_battery_ops tps80032_power_battery_ops = {
	.get_usb_online = tps80032_get_usb_online,
	.get_bat_status = tps80032_get_bat_status,
	.get_bat_health = tps80032_get_bat_health,
	.get_bat_present = tps80032_get_bat_present,
	.get_bat_technology = NULL,
	.get_bat_capacity = tps80032_get_bat_capacity,
	.get_bat_capacity_level = tps80032_get_bat_capacity_level,
	.get_bat_temperature = tps80032_get_bat_temperature,
	.get_bat_voltage = tps80032_get_bat_voltage,
	.get_bat_time_to_empty = NULL,
	.get_bat_time_to_full = NULL,
	.stop_charging = tps80032_stop_charging,
};

/* struct tps80032_correct_ops */
struct battery_correct_ops tps80032_correct_ops = {
	.correct_capacity_func = tps80032_correct_capacity,
	.correct_time_to_empty_func = NULL,
	.correct_time_to_full_func = NULL,
	.correct_status_func = NULL,
	.correct_temp_func = tps80032_correct_temp,
	.correct_voltage_func = tps80032_correct_voltage,
};

/*
 * tps80032_power_suspend: power suppend event
 * @client: The I2C client device.
 * @mesg: The power message.
 * return:
 *        = 0: Normal operation
 */
static int tps80032_power_suspend(struct i2c_client *client, pm_message_t mesg)
{
	PMIC_DEBUG_MSG(">>> %s: name=%s addr=0x%x\n", __func__, client->name,client->addr);
	/* Disable interrupt of PMIC */
	disable_irq(irqpin2irq(CONST_INT_ID));
	del_timer_sync(&bat_timer);
	PMIC_DEBUG_MSG("%s end <<<\n", __func__);
	return 0;
}

/*
 * tps80032_power_resume: power resume event
 * @client: The I2C client device.
 * return:
 *        = 0: Normal operation
 */
static int tps80032_power_resume(struct i2c_client *client)
{
	struct tps80032_data *data = i2c_get_clientdata(client);

	PMIC_DEBUG_MSG(">>> %s: name=%s addr=0x%x\n", __func__, client->name,client->addr);

	/* Enable interrupt of PMIC */
	enable_irq(irqpin2irq(CONST_INT_ID));

	queue_work(data->queue,&data->resume_work);
	queue_work(data->queue,&data->bat_work);
	queue_work(data->queue,&data->ext_work);
	PMIC_DEBUG_MSG("%s end <<<\n", __func__);
	return 0;
}


/*
 * tps80032_init_power_hw: init hardware for power supply management
 * @data: The struct which handles the TPS80032 data.
 * return:
 *        = 0: Normal operation
 *        < 0: Error occurs
 */
int tps80032_init_power_hw(struct tps80032_data *data)
{
	int ret = 0;
	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	/* Turn on SMPS4 */
	/*
	ret = i2c_smbus_write_byte_data(data->client_power, HW_REG_SMPS4_CFG_STATE, tps80032_check_state_valid(E_POWER_ON));
	if (0 > ret) {
		goto init_power_hw_exit;
	} */

	/* Set default voltage (1.8V) for LDO1 */
	/*
	ret = i2c_smbus_write_byte_data(data->client_power, HW_REG_LDO1_CFG_VOLTAGE, tps80032_check_ldo_voltage_valid(E_LDO_VOLTAGE_1_8000V));
	if (0 > ret) {
		goto init_power_hw_exit;
	} */

	/* Set default voltage (2.9V) for LDO5 */
	/*
	ret = i2c_smbus_write_byte_data(data->client_power, HW_REG_LDO5_CFG_VOLTAGE, tps80032_check_ldo_voltage_valid(E_LDO_VOLTAGE_2_9000V));
	if (0 > ret) {
		goto init_power_hw_exit;
	} */

	/* Set default voltage (2.5V) for LDO7 */
	/*
	ret = i2c_smbus_write_byte_data(data->client_power, HW_REG_LDO7_CFG_VOLTAGE, tps80032_check_ldo_voltage_valid(E_LDO_VOLTAGE_2_5000V));
	if (0 > ret) {
		goto init_power_hw_exit;
	}*/

	/* Assign LDO6 and LDON into Group1 */
	/*
	ret = i2c_smbus_write_byte_data(data->client_power, HW_REG_PREQ1_RES_ASS_B, MSK_PREQ1_ASS);
	if (0 > ret) {
		goto init_power_hw_exit;
	} */
	
	PMIC_DEBUG_MSG("%s end <<<\n", __func__);
/* init_power_hw_exit: */
	return ret;
}

/*
 * tps80032_init_battery_hw: init hardware for battery management
 * @data: The struct which handles the TPS80032 data.
 * return:
 *        = 0: Normal operation
 *        < 0: Error occurs
 */
int tps80032_init_battery_hw(struct tps80032_data *data)
{
	int ret = 0;
	int val;

	PMIC_DEBUG_MSG(">>> %s start\n", __func__);

	/* Mask interrupt line signal A */
	ret = i2c_smbus_write_byte_data(data->client_battery, HW_REG_INT_MSK_LINE_STS_A, MSK_INT_LINE_A);
	if (0 > ret) {
		goto exit;
	}

	/* Mask interrupt line signal B */
	ret = i2c_smbus_write_byte_data(data->client_battery, HW_REG_INT_MSK_LINE_STS_B, MSK_INT_LINE_B);
	if (0 > ret) {
		goto exit;
	}

	/* Mask interrupt line signal C */
	ret = i2c_smbus_write_byte_data(data->client_battery, HW_REG_INT_MSK_LINE_STS_C, MSK_INT_LINE_C);
	if (0 > ret) {
		goto exit;
	}

	
	/* Mask interrupt signal A */
	ret = i2c_smbus_write_byte_data(data->client_battery, HW_REG_INT_MSK_STS_A, MSK_INT_SRC_A);
	if (0 > ret) {
		goto exit;
	}

	/* Mask interrupt signal B */
	ret = i2c_smbus_write_byte_data(data->client_battery, HW_REG_INT_MSK_STS_B, MSK_INT_SRC_B);
	if (0 > ret) {
		goto exit;
	}

	/* Mask interrupt signal C */
	ret = i2c_smbus_write_byte_data(data->client_battery, HW_REG_INT_MSK_STS_C, MSK_INT_SRC_C);
	if (0 > ret) {
		goto exit;
	}

	
	/* Enable Charge current termination interrupt */
	ret = i2c_smbus_read_byte_data(data->client_battery, HW_REG_CHARGERUSB_CTRL1);
	if (0 > ret) {
		goto exit;
	}

	ret |= MSK_BIT_4;

	/* Set 1 to TERM bit at CHARGERUSB_CTRL1 (0xE8) register */
	ret = i2c_smbus_write_byte_data(data->client_battery, HW_REG_CHARGERUSB_CTRL1, ret);
	if (0 > ret) {
		goto exit;
	}

	/* Enable charge one feature */
	ret = i2c_smbus_read_byte_data(data->client_battery, HW_REG_CHARGERUSB_CTRL3);
	if (0 > ret) {
		goto exit;
	}

	ret |= MSK_BIT_6;

	/* Set 1 to CHARGE_ONCE bit at CHARGERUSB_CTRL3 (0xEA) register */	
	ret = i2c_smbus_write_byte_data(data->client_battery, HW_REG_CHARGERUSB_CTRL3, ret);
	if (0 > ret) {
		goto exit;
	}
	
	/* Disable interrupt related to EXT_CHRG */	
	ret = i2c_smbus_write_byte_data(data->client_battery, HW_REG_CONTROLLER_INT_MASK, MSK_CONTROLLER_INT);
	if (0 > ret) {
		goto exit;
	}

	/* Clear all interrupt source */
	ret = i2c_smbus_write_byte_data(data->client_battery, HW_REG_INT_STS_A, MSK_DISABLE);
	if (0 > ret) {
		goto exit;
	}
	ret = i2c_smbus_write_byte_data(data->client_battery, HW_REG_INT_STS_B, MSK_DISABLE);
	if (0 > ret) {
		goto exit;
	}
	ret = i2c_smbus_write_byte_data(data->client_battery, HW_REG_INT_STS_C, MSK_DISABLE);
	if (0 > ret) {
		goto exit;
	}
	/* Enable charger if charger is present at the boot up of driver */
	ret = i2c_smbus_read_byte_data(data->client_battery, HW_REG_CONTROLLER_STAT1);
	if (0 > ret) {
		return ret;
	} else if (0 != (ret & MSK_BIT_2)) {
		ret = i2c_smbus_read_byte_data(data->client_battery, HW_REG_CONTROLLER_CTRL1);
		
		if (0 > ret) {
			return ret;
		}
		
		/* Enable charger and set VBUS is source charge */
		val = ret | MSK_BIT_4;
		val = val & (~MSK_BIT_3);

		ret = i2c_smbus_write_byte_data(data->client_battery, HW_REG_CONTROLLER_CTRL1, val);
		if (0 > ret) {
			return ret;
		}
	}
	PMIC_DEBUG_MSG("%s end <<<\n", __func__);
exit:
	return ret;
}


/*
 * tps80032_init_irq: init interrupt for PMIC driver
 * @data: The struct which handles the TPS80032 data.
 * @irq: The ID interrupt of I2C driver
 * @irq_base: The ID Base interrupt of PMIC driver
 * return:
 *        = 0: Normal operation
 *        < 0: Error occurs
 */
int tps80032_init_irq(struct tps80032_data *data, int irq, int irq_base)
{
	int i, ret;

	if (!irq_base) {
		return -EINVAL;
	}

	data->irq_base = irq_base;

	data->irq_chip.name = "tps80032-battery";
	data->irq_chip.irq_enable = tps80032_irq_enable;
	data->irq_chip.irq_disable = tps80032_irq_disable;
	data->irq_chip.irq_bus_lock = tps80032_irq_lock;
	data->irq_chip.irq_bus_sync_unlock = tps80032_irq_sync_unlock;

	for (i = 0; i < TPS80031_INT_NR; i++) {
		int __irq = i + data->irq_base;
		irq_set_chip_data(__irq, data);
		irq_set_chip_and_handler(__irq, &data->irq_chip, handle_simple_irq);
		irq_set_nested_thread(__irq, 1);
#ifdef CONFIG_ARM
		set_irq_flags(__irq, IRQF_VALID);
#endif
	}

	ret = request_threaded_irq(irq, NULL, tps80032_interrupt_handler, IRQF_ONESHOT, "tps80032", data);

	if (!ret) {
		enable_irq_wake(irq);
	}

	return ret;
}

/*
 * tps80032_power_probe: probe function for power supply management
 * @client: The I2C client device.
 * @id: The I2C ID.
 * return:
 *        = 0: Normal operation
 *        < 0: Error occurs
 */
static int tps80032_power_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret = 0;
	int count = 0;


	PMIC_DEBUG_MSG(">>> %s: name=%s addr=0x%x\n", __func__, client->name,client->addr);

	data = kzalloc(sizeof(*data), GFP_KERNEL);
	if (NULL == data) {
		PMIC_ERROR_MSG("%s:%d kzalloc failed err=%d\n",__func__,__LINE__,ret);
		return -ENOMEM;
	}

	/* init initial value */
	mutex_init(&data->smps1_lock);
	mutex_init(&data->smps2_lock);
	mutex_init(&data->smps3_lock);
	mutex_init(&data->smps4_lock);
	mutex_init(&data->smps5_lock);
	mutex_init(&data->ldo1_lock);
	mutex_init(&data->ldo2_lock);
	mutex_init(&data->ldo3_lock);
	mutex_init(&data->ldo4_lock);
	mutex_init(&data->ldo5_lock);
	mutex_init(&data->ldo6_lock);
	mutex_init(&data->ldo7_lock);
	mutex_init(&data->vbus_lock);
	mutex_init(&data->force_off_lock);
	mutex_init(&data->rscounter_lock);

	data->device = E_DEVICE_NONE;
	data->charger = 0;
	data->bat_volt = 0;
	data->bat_temp = 0;
	data->bat_over_volt = 0;
	data->bat_over_temp = 0;
	data->bat_capacity = 0;
	data->bat_presence = 0;

	while (count < RESOURCE_COUNTER_MAX) {
		data->rscounter[count] = 0;
		count = count + 1;
	}


	/* Set client power */
	data->client_power = client;

	/* Create work queue */
	data->queue  = create_singlethread_workqueue("tps80032_int_dev_queue");
	if (!data->queue) {
		ret = -ENOMEM;
		PMIC_ERROR_MSG("%s:%d create_singlethread_workqueue failed err=%d\n",__func__,__LINE__,ret);
		goto err_workqueue_alloc;
	}

	/* init work queue */
	INIT_WORK(&data->interrupt_work, tps80032_interrupt_work);
	INIT_WORK(&data->resume_work, tps80032_resume_work);
	INIT_WORK(&data->bat_work, tps80032_bat_work);
	INIT_WORK(&data->ext_work, tps80032_ext_work);

	i2c_set_clientdata(client, data);

	/*
	virt_addr = ioremap(MAP_BASE_NV + MAP_BASE_PMIC_NV, MAP_SIZE_PMIC_NV);
	if (!virt_addr) {
		ret = -ENOMEM;
		goto err_ioremap_virt;
	}
	*/

	/* Register into PMIC interface */
	ret = pmic_device_register(&client->dev,&tps80032_power_ops);
	if (0 > ret) {
		PMIC_ERROR_MSG("%s:%d pmic_device_register failed err=%d\n",__func__,__LINE__,ret);
		goto err_device_register;
	}

	/* Init hardware */
	ret = tps80032_init_power_hw(data);
	if (0 > ret) {
		PMIC_ERROR_MSG("%s:%d tps80032_init_power_hw failed err=%d\n",__func__,__LINE__,ret);
		goto err_init_hw;
	}

	/* Init IRQ*/
	ret = tps80032_init_irq(data, client->irq, IRQPIN_IRQ_BASE + 64);
	if (0 > ret) {
		PMIC_ERROR_MSG("%s:%d tps80032_init_irq failed err=%d\n",__func__,__LINE__,ret);
		goto err_request_irq;
	}
	
	PMIC_DEBUG_MSG("%s end <<<\n", __func__);
	return 0;
	
err_request_irq:
	free_irq(pint2irq(CONST_INT_ID),data);
err_init_hw:
	pmic_device_unregister(&client->dev);
err_device_register:
	iounmap(virt_addr);
	virt_addr = NULL;
/*err_ioremap_virt: */
	destroy_workqueue(data->queue);
err_workqueue_alloc:
	kfree(data);
	return ret;
}

/*
 * tps80032_power_remove: remove function for power supply management
 * @client: The I2C client device.
 * return:
 *        = 0: Normal operation
 */
static int tps80032_power_remove(struct i2c_client *client)
{
	struct tps80032_data *data = i2c_get_clientdata(client);

	gpio_free(GPIO_PORT28);		/* free GPIO_PORT28 */
	free_irq(pint2irq(CONST_INT_ID),data);	/* free interrupt */
	pmic_device_unregister(&client->dev);
	destroy_workqueue(data->queue);
	kfree(data);
	return 0;
}


/*
 * tps80032_battery_probe: probe function for battery management
 * @client: The I2C client device.
 * @id: The I2C ID.
 * return:
 *        = 0: Normal operation
 *        < 0: Error occurs
 */
static int tps80032_battery_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret=0;

	PMIC_DEBUG_MSG(">>> %s: name=%s addr=0x%x\n", __func__, client->name,client->addr);

	if (NULL == data) {
		PMIC_ERROR_MSG("%s:%d data is failed in location\n",__func__,__LINE__);
		return -ENOMEM;
	}

	/* Set client battery and data*/
	data->stop_charge = 0;
	data->client_battery = client;
	i2c_set_clientdata(client, data);

	/* Register into USB OTG VBUS interface */
	ret = usb_otg_pmic_device_register(&client->dev,&tps80032_vbus_ops);
	if (0 > ret) {
		PMIC_ERROR_MSG("%s:%d usb_otg_pmic_device_register failed err=%d\n",__func__,__LINE__,ret);
		goto err_device_USB_register;
	}

	/* Register battery device */
	ret = pmic_battery_device_register(&client->dev, &tps80032_power_battery_ops);
	if (0 > ret) {
		PMIC_ERROR_MSG("%s:%d pmic_battery_device_register failed err=%d\n",__func__,__LINE__,ret);
		goto err_battery_device_register;
	}

	/* Register correct device */
	ret = pmic_battery_register_correct_func(&tps80032_correct_ops);
	if (0 > ret) {
		PMIC_ERROR_MSG("%s:%d pmic_battery_register_correct_func failed err=%d\n",__func__,__LINE__,ret);
		goto err_correct_device_register;
	}

	/* Init hardware configuration*/
	ret = tps80032_init_battery_hw(data);
	if (0 > ret) {
		PMIC_ERROR_MSG("%s:%d tps80032_init_hw failed err=%d\n",__func__,__LINE__,ret);
		goto err_init_hw;
	}

	tps80032_init_timer(data);
	bat_timer.expires  = jiffies + msecs_to_jiffies(CONST_TIMER_BATTERY_UPDATE);
	add_timer(&bat_timer);

	/* Run bat_work() to update all battery information firstly */
	queue_work(data->queue, &data->bat_work);
	PMIC_DEBUG_MSG("%s end <<<\n", __func__);
	return 0;

err_init_hw:
	pmic_battery_unregister_correct_func();
err_correct_device_register:
	pmic_battery_device_unregister(&client->dev);
err_battery_device_register:
	usb_otg_pmic_device_unregister(&client->dev);
err_device_USB_register:
	return ret;
}

/*
 * tps80032_battery_remove: remove function for battery management
 * @client: The I2C client device.
 * return:
 *        = 0: Normal operation
 */
static int tps80032_battery_remove(struct i2c_client *client)
{
	pmic_battery_unregister_correct_func();
	pmic_battery_device_unregister(&client->dev);
	usb_otg_pmic_device_unregister(&client->dev);
	return 0;
}

/*
 * tps80032_dvs_probe: probe function for DVS driver
 * @client: The I2C client device.
 * @id: The I2C ID.
 * return:
 *        = 0: Normal operation
 *        < 0: Error occurs
 */
static int tps80032_dvs_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	PMIC_DEBUG_MSG(">>>%s: name=%s addr=0x%x\n", __func__, client->name,client->addr);

	if (NULL == data) {
		PMIC_ERROR_MSG("%s:%d data is failed in location\n",__func__,__LINE__);
		return -ENOMEM;
	}

	/* Set client dvs and data*/
	data->client_dvs = client;
	i2c_set_clientdata(client, data);

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);
	return 0;
}

/*
 * tps80032_dvs_remove: remove function for DVS driver
 * @client: The I2C client device.
 * return:
 *        = 0: Normal operation
 */
static int tps80032_dvs_remove(struct i2c_client *client)
{
	return 0;
}

/*
 * tps80032_jtag_probe: probe function for Jtag driver
 * @client: The I2C client device.
 * @id: The I2C ID.
 * return:
 *        = 0: Normal operation
 *        < 0: Error occurs
 */
static int tps80032_jtag_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	PMIC_DEBUG_MSG(">>>%s: name=%s addr=0x%x\n", __func__, client->name,client->addr);

	if (NULL == data) {
		PMIC_ERROR_MSG("%s:%d data is failed in location\n",__func__,__LINE__);
		return -ENOMEM;
	}

	/* Set client dvs and data*/
	data->client_jtag = client;
	i2c_set_clientdata(client, data);
	PMIC_DEBUG_MSG("%s end <<<\n", __func__);

	return 0;
}

/*
 * tps80032_jtag_remove: remove function for Jtag driver
 * @client: The I2C client device.
 * return:
 *        = 0: Normal operation
 */
static int tps80032_jtag_remove(struct i2c_client *client)
{
	return 0;
}

/* Struct I2C ID for power supply driver */
static const struct i2c_device_id tps80032_power_id[] = {
	{ "tps80032-power", 0 }, {{0}, 0},
};

/* Struct I2C ID for battery driver */
static const struct i2c_device_id tps80032_battery_id[] = {
	{ "tps80032-battery", 0 }, {{0}, 0},
};

/* Struct I2C ID for DVS driver */
static const struct i2c_device_id tps80032_dvs_id[] = {
	{ "tps80032-dvs", 0 }, {{0}, 0},
};

/* Struct I2C ID for JTAG driver */
static const struct i2c_device_id tps80032_jtag_id[] = {
	{ "tps80032-jtag", 0 }, {{0}, 0},
};

/* Struct I2C driver for power supply driver */
static struct i2c_driver tps80032_power_driver = {
	.driver = {
		.name	= "tps80032-power",
	},
	.probe		= tps80032_power_probe,
	.remove		= tps80032_power_remove,
	.id_table	= tps80032_power_id,
	.suspend	= tps80032_power_suspend,
	.resume		= tps80032_power_resume,
};

/* Struct I2C driver for battery driver */
static struct i2c_driver tps80032_battery_driver = {
	.driver = {
		.name	= "tps80032-battery",
	},
	.probe		= tps80032_battery_probe,
	.remove		= tps80032_battery_remove,
	.id_table	= tps80032_battery_id,
};

/* Struct I2C driver for DVS driver */
static struct i2c_driver tps80032_dvs_driver = {
	.driver = {
		.name	= "tps80032-dvs",
	},
	.probe		= tps80032_dvs_probe,
	.remove		= tps80032_dvs_remove,
	.id_table	= tps80032_dvs_id,
};

/* Struct I2C driver for JTAG driver */
static struct i2c_driver tps80032_jtag_driver = {
	.driver = {
		.name	= "tps80032-jtag",
	},
	.probe		= tps80032_jtag_probe,
	.remove		= tps80032_jtag_remove,
	.id_table	= tps80032_jtag_id,
};

/*
 * tps80032_power_init: Initialize all resource of this module
 * @void
 * return:
 *        = 0: Normal termination
 *        < 0: Error occurs
 */
static int __init tps80032_power_init(void)
{
	int ret;
	
	PMIC_DEBUG_MSG(">>> %s start\n", __func__);
	
	ret = i2c_add_driver(&tps80032_power_driver);
	if (0 != ret) {
		PMIC_ERROR_MSG("Unable to register tps80032_power driver\n");
		goto err_power_driver;
	}

	ret = i2c_add_driver(&tps80032_battery_driver);
	if (0 != ret) {
		PMIC_ERROR_MSG("Unable to register tps80032_battery driver\n");
		goto err_battery_driver;
	}

	ret = i2c_add_driver(&tps80032_dvs_driver);
	if (0 != ret) {
		PMIC_ERROR_MSG("Unable to register tps80032_dvs driver\n");
		goto err_dvs_driver;
	}

	ret = i2c_add_driver(&tps80032_jtag_driver);
	if (0 != ret) {
		PMIC_ERROR_MSG("Unable to register tps80032_jtag driver\n");
		goto err_jtag_driver;
	}

	PMIC_DEBUG_MSG("%s end <<<\n", __func__);

	return 0;

err_jtag_driver:
	i2c_del_driver(&tps80032_dvs_driver);
err_dvs_driver:
	i2c_del_driver(&tps80032_battery_driver);
err_battery_driver:
	i2c_del_driver(&tps80032_power_driver);
err_power_driver:
	return ret;
}

/*
 * tps80032_power_exit: free all resources of this module
 * @void
 * return: void
 */
static void __exit tps80032_power_exit(void)
{
	i2c_del_driver(&tps80032_power_driver);
	i2c_del_driver(&tps80032_battery_driver);
	i2c_del_driver(&tps80032_dvs_driver);
	i2c_del_driver(&tps80032_jtag_driver);
}

subsys_initcall(tps80032_power_init);
module_exit(tps80032_power_exit);

MODULE_DESCRIPTION("TPS80032 power department driver");
MODULE_LICENSE("GPL");