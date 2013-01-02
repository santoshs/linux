/*
*  SMB328A-charger.c
*  SMB328A charger interface driver
*
*  Copyright (C) 2012 Samsung Electronics
*
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/power_supply.h>
#include <linux/regulator/machine.h>
#include <linux/d2153/d2153_battery.h>
#include <linux/bq27425.h>
#include <linux/tsu6712.h>
#include <linux/pmic/pmic.h>
#include <linux/spa_power.h>

#include <mach/common.h>

/* Register define */
#define SMB328A_INPUT_AND_CHARGE_CURRENTS	0x00
#define SMB328A_CURRENT_TERMINATION			0x01
#define SMB328A_FLOAT_VOLTAGE				0x02
#define SMB328A_FUNCTION_CONTROL_A1			0x03
#define SMB328A_FUNCTION_CONTROL_A2			0x04
#define SMB328A_FUNCTION_CONTROL_B			0x05
#define SMB328A_OTG_PWR_AND_LDO_CONTROL	0x06
#define SMB328A_VARIOUS_CONTROL_FUNCTION_A	0x07
#define SMB328A_CELL_TEMPERATURE_MONITOR	0x08
#define SMB328A_INTERRUPT_SIGNAL_SELECTION	0x09
#define SMB328A_I2C_BUS_SLAVE_ADDRESS		0x0A

#define SMB328A_CLEAR_IRQ					0x30
#define SMB328A_COMMAND						0x31
#define SMB328A_INTERRUPT_STATUS_A			0x32
#define SMB328A_BATTERY_CHARGING_STATUS_A	0x33
#define SMB328A_INTERRUPT_STATUS_B			0x34
#define SMB328A_BATTERY_CHARGING_STATUS_B	0x35
#define SMB328A_BATTERY_CHARGING_STATUS_C	0x36
#define SMB328A_INTERRUPT_STATUS_C			0x37
#define SMB328A_BATTERY_CHARGING_STATUS_D	0x38
#define SMB328A_AUTOMATIC_INPUT_CURRENT_LIMMIT_STATUS	0x39

enum {
	BAT_NOT_DETECTED,
	BAT_DETECTED
};

enum {
	CHG_MODE_NONE,
	CHG_MODE_AC,
	CHG_MODE_USB
};

struct smb328a_chip {
	struct i2c_client		*client;
	struct power_supply		psy_bat;
    struct work_struct      work;
	struct smb328a_platform_data	*pdata;
	int chg_mode;
};

static struct smb328a_chip *smb_charger = NULL;

static enum power_supply_property smb328a_battery_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_TYPE,
	POWER_SUPPLY_PROP_BATT_TEMP_ADC,
};

extern int pmic_get_temp_status(void);
extern int pmic_read_battery_status(int property);

static int smb328a_write_reg(struct i2c_client *client, int reg, u8 value)
{
	int ret;

	ret = i2c_smbus_write_byte_data(client, reg, value);

	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	return ret;
}

static int smb328a_read_reg(struct i2c_client *client, int reg)
{
	int ret;

	ret = i2c_smbus_read_byte_data(client, reg);

	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	return ret;
}

static void smb328a_print_reg(struct i2c_client *client, int reg)
{
	u8 data = 0;

	data = i2c_smbus_read_byte_data(client, reg);

	if (data < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, data);
	else
		printk("%s : reg (0x%x) = 0x%x\n", __func__, reg, data);
}

static void smb328a_print_all_regs(struct i2c_client *client)
{
	smb328a_print_reg(client, 0x31);
	smb328a_print_reg(client, 0x32);
	smb328a_print_reg(client, 0x33);
	smb328a_print_reg(client, 0x34);
	smb328a_print_reg(client, 0x35);
	smb328a_print_reg(client, 0x36);
	smb328a_print_reg(client, 0x37);
	smb328a_print_reg(client, 0x38);
	smb328a_print_reg(client, 0x39);
	smb328a_print_reg(client, 0x00);
	smb328a_print_reg(client, 0x01);
	smb328a_print_reg(client, 0x02);
	smb328a_print_reg(client, 0x03);
	smb328a_print_reg(client, 0x04);
	smb328a_print_reg(client, 0x05);
	smb328a_print_reg(client, 0x06);
	smb328a_print_reg(client, 0x07);
	smb328a_print_reg(client, 0x08);
	smb328a_print_reg(client, 0x09);
	smb328a_print_reg(client, 0x0a);
}

static void smb328a_allow_volatile_writes(struct i2c_client *client)
{
	int val;
	u8 data;

	val = smb328a_read_reg(client, SMB328A_COMMAND);
	if ((val >= 0) && !(val&0x80)) {
		data = (u8)val;
		printk("%s : reg (0x%x) = 0x%x\n", __func__, SMB328A_COMMAND, data);
		data |= (0x1 << 7);
		if (smb328a_write_reg(client, SMB328A_COMMAND, data) < 0)
			printk("%s : error!\n", __func__);
		val = smb328a_read_reg(client, SMB328A_COMMAND);
		if (val >= 0) {
			data = (u8)data;
			printk("%s : => reg (0x%x) = 0x%x\n", __func__, SMB328A_COMMAND, data);
		}
	}
}

static void smb328a_set_command_reg(struct i2c_client *client)
{
	int val;
	u8 data;

	val = smb328a_read_reg(client, SMB328A_COMMAND);
	if (val >= 0) {
		data = (u8)val;
		printk("%s : reg (0x%x) = 0x%x\n", __func__, SMB328A_COMMAND, data);
		data = 0xad;
		if (smb328a_write_reg(client, SMB328A_COMMAND, data) < 0)
			printk("%s : error!\n", __func__);
		val = smb328a_read_reg(client, SMB328A_COMMAND);
		if (val >= 0) {
			data = (u8)data;
			printk("%s : => reg (0x%x) = 0x%x\n", __func__, SMB328A_COMMAND, data);
		}
	}
}

static void smb328a_charger_function_conrol(struct i2c_client *client, int chg_current)
{
	int val;
	u8 data;

	smb328a_allow_volatile_writes(client);

	val = smb328a_read_reg(client, SMB328A_INPUT_AND_CHARGE_CURRENTS);
	if (val >= 0) {
		data = (u8)val;
		printk("%s : reg (0x%x) = 0x%x\n", __func__, SMB328A_INPUT_AND_CHARGE_CURRENTS, data);
		data &= 0x1F;
		data |= (((chg_current / 100) - 5) << 5);
		if (smb328a_write_reg(client, SMB328A_INPUT_AND_CHARGE_CURRENTS, data) < 0)
			printk("%s : error!\n", __func__);
		val = smb328a_read_reg(client, SMB328A_INPUT_AND_CHARGE_CURRENTS);
		if (val >= 0) {
			data = (u8)val;
			printk("%s : => reg (0x%x) = 0x%x\n", __func__, SMB328A_INPUT_AND_CHARGE_CURRENTS, data);
		}
	}

	val = smb328a_read_reg(client, SMB328A_CURRENT_TERMINATION);
	if (val >= 0) {
		data = (u8)val;
		printk("%s : reg (0x%x) = 0x%x\n", __func__, SMB328A_CURRENT_TERMINATION, data);
		data = 0xf4;

		if (smb328a_write_reg(client, SMB328A_CURRENT_TERMINATION, data) < 0)
			printk("%s : error!\n", __func__);

		val = smb328a_read_reg(client, SMB328A_CURRENT_TERMINATION);
		if (val >= 0) {
			data = (u8)val;
			printk("%s : => reg (0x%x) = 0x%x\n", __func__, SMB328A_CURRENT_TERMINATION, data);
		}
	}

	val = smb328a_read_reg(client, SMB328A_FLOAT_VOLTAGE);
	if (val >= 0) {
		data = (u8)val;
		printk("%s : reg (0x%x) = 0x%x\n", __func__, SMB328A_FLOAT_VOLTAGE, data);
		if (u2_get_board_rev() >= 5) {
			if (data != 0xd4) {
				data = 0xd4; /* 4.3V float voltage */
				if (smb328a_write_reg(client, SMB328A_FLOAT_VOLTAGE, data) < 0)
					printk("%s : error!\n", __func__);
				val = smb328a_read_reg(client, SMB328A_FLOAT_VOLTAGE);
				if (val >= 0) {
					data = (u8)val;
					printk("%s : => reg (0x%x) = 0x%x\n", __func__, SMB328A_FLOAT_VOLTAGE, data);
				}
			}
		} else {
			if (data != 0xca) {
				data = 0xca; /* 4.2V float voltage */
				if (smb328a_write_reg(client, SMB328A_FLOAT_VOLTAGE, data) < 0)
					printk("%s : error!\n", __func__);
				val = smb328a_read_reg(client, SMB328A_FLOAT_VOLTAGE);
				if (val >= 0) {
					data = (u8)val;
					printk("%s : => reg (0x%x) = 0x%x\n", __func__, SMB328A_FLOAT_VOLTAGE, data);
				}
			}
		}
	}

	val = smb328a_read_reg(client, SMB328A_FUNCTION_CONTROL_A1);
	if (val >= 0) {
		data = (u8)val;
		printk("%s : reg (0x%x) = 0x%x\n", __func__, SMB328A_FUNCTION_CONTROL_A1, data);

		if (data != 0xc2) {
			data = 0xc2; // changed pre-charge to Fast < charge Voltage Threshold 2.6V->2.2V
			if (smb328a_write_reg(client, SMB328A_FUNCTION_CONTROL_A1, data) < 0)
				printk("%s : error!\n", __func__);
			val = smb328a_read_reg(client, SMB328A_FUNCTION_CONTROL_A1);
			if (val >= 0) {
				data = (u8)val;
				printk("%s : => reg (0x%x) = 0x%x\n", __func__, SMB328A_FUNCTION_CONTROL_A1, data);
			}
		}
	}

	val = smb328a_read_reg(client, SMB328A_FUNCTION_CONTROL_A2);
	if (val >= 0) {
		data = (u8)val;
		printk("%s : reg (0x%x) = 0x%x\n", __func__, SMB328A_FUNCTION_CONTROL_A2, data);
		if (data != 0x4D) {
			data = 0x4D;
			if (smb328a_write_reg(client, SMB328A_FUNCTION_CONTROL_A2, data) < 0)
				printk("%s : error!\n", __func__);
			val = smb328a_read_reg(client, SMB328A_FUNCTION_CONTROL_A2);
			if (val >= 0) {
				data = (u8)val;
				printk("%s : => reg (0x%x) = 0x%x\n", __func__, SMB328A_FUNCTION_CONTROL_A2, data);
			}
		}
	}

	val = smb328a_read_reg(client, SMB328A_FUNCTION_CONTROL_B);
	if (val >= 0) {
		data = (u8)val;
		printk("%s : reg (0x%x) = 0x%x\n", __func__, SMB328A_FUNCTION_CONTROL_B, data);
		if (data != 0x0) {
			data = 0x0;
			if (smb328a_write_reg(client, SMB328A_FUNCTION_CONTROL_B, data) < 0)
				printk("%s : error!\n", __func__);
			val = smb328a_read_reg(client, SMB328A_FUNCTION_CONTROL_B);
			if (val >= 0) {
				data = (u8)val;
				printk("%s : => reg (0x%x) = 0x%x\n", __func__, SMB328A_FUNCTION_CONTROL_B, data);
			}
		}
	}

	val = smb328a_read_reg(client, SMB328A_OTG_PWR_AND_LDO_CONTROL);
	if (val >= 0) {
		data = (u8)val;
		printk("%s : reg (0x%x) = 0x%x\n", __func__, SMB328A_OTG_PWR_AND_LDO_CONTROL, data);
		if (data != 0xc5) {
			data = 0xc5;
			if (smb328a_write_reg(client, SMB328A_OTG_PWR_AND_LDO_CONTROL, data) < 0)
				printk("%s : error!\n", __func__);
			val = smb328a_read_reg(client, SMB328A_OTG_PWR_AND_LDO_CONTROL);
			if (val >= 0) {
				data = (u8)val;
				printk("%s : => reg (0x%x) = 0x%x\n", __func__, SMB328A_OTG_PWR_AND_LDO_CONTROL, data);
			}
		}
	}

	val = smb328a_read_reg(client, SMB328A_VARIOUS_CONTROL_FUNCTION_A);
	if (val >= 0) {
		data = (u8)val;
		printk("%s : reg (0x%x) = 0x%x\n", __func__, SMB328A_VARIOUS_CONTROL_FUNCTION_A, data);
		if (data != 0x96) { /* this can be changed with top-off setting */
			data = 0x96;
			if (smb328a_write_reg(client, SMB328A_VARIOUS_CONTROL_FUNCTION_A, data) < 0)
				printk("%s : error!\n", __func__);
			val = smb328a_read_reg(client, SMB328A_VARIOUS_CONTROL_FUNCTION_A);
			if (val >= 0) {
				data = (u8)val;
				printk("%s : => reg (0x%x) = 0x%x\n", __func__, SMB328A_VARIOUS_CONTROL_FUNCTION_A, data);
			}
		}
	}

	val = smb328a_read_reg(client, SMB328A_CELL_TEMPERATURE_MONITOR);
	if (val >= 0) {
		data = (u8)val;
		printk("%s : reg (0x%x) = 0x%x\n", __func__, SMB328A_CELL_TEMPERATURE_MONITOR, data);
		if (data != 0x0) {
			data = 0x0;
			if (smb328a_write_reg(client, SMB328A_CELL_TEMPERATURE_MONITOR, data) < 0)
				printk("%s : error!\n", __func__);
			val = smb328a_read_reg(client, SMB328A_CELL_TEMPERATURE_MONITOR);
			if (val >= 0) {
				data = (u8)val;
				printk("%s : => reg (0x%x) = 0x%x\n", __func__, SMB328A_CELL_TEMPERATURE_MONITOR, data);
			}
		}
	}

	val = smb328a_read_reg(client, SMB328A_INTERRUPT_SIGNAL_SELECTION);
	if (val >= 0) {
		data = (u8)val;
		printk("%s : reg (0x%x) = 0x%x\n", __func__, SMB328A_INTERRUPT_SIGNAL_SELECTION, data);
		if (data != 0x10) {
			data = 0x10;
			if (smb328a_write_reg(client, SMB328A_INTERRUPT_SIGNAL_SELECTION, data) < 0)
				printk("%s : error!\n", __func__);
			val = smb328a_read_reg(client, SMB328A_INTERRUPT_SIGNAL_SELECTION);
			if (val >= 0) {
				data = (u8)val;
				printk("%s : => reg (0x%x) = 0x%x\n", __func__, SMB328A_INTERRUPT_SIGNAL_SELECTION, data);
			}
		}
	}
	val = smb328a_read_reg(client, SMB328A_I2C_BUS_SLAVE_ADDRESS);
	if (val >= 0) {
		data = (u8)val;
		printk("%s : reg (0x%x) = 0x%x\n", __func__, SMB328A_I2C_BUS_SLAVE_ADDRESS, data);
		if (data != 0x69) {
			data = 0x69;
			if (smb328a_write_reg(client, SMB328A_I2C_BUS_SLAVE_ADDRESS, data) < 0)
				printk("%s : error!\n", __func__);
			val = smb328a_read_reg(client, SMB328A_I2C_BUS_SLAVE_ADDRESS);
			if (val >= 0) {
				data = (u8)val;
				printk("%s : => reg (0x%x) = 0x%x\n", __func__, SMB328A_I2C_BUS_SLAVE_ADDRESS, data);
			}
		}
	}
}

static int smb328a_check_charging_status(struct i2c_client *client)
{
	int val;
	u8 data = 0;
	int ret = -1;

	//printk("%s : \n", __func__);

	val = smb328a_read_reg(client, SMB328A_BATTERY_CHARGING_STATUS_C);
	if (val >= 0) {
		data = (u8)val;
		printk("%s : reg (0x%x) = 0x%x\n", __func__, SMB328A_BATTERY_CHARGING_STATUS_C, data);

		ret = (data&(0x3<<1))>>1;
		printk("%s : status = 0x%x\n", __func__, data);
	}

	return ret;
}

static bool smb328a_check_is_charging(struct i2c_client *client)
{
	int val;
	u8 data = 0;
	bool ret = false;

	//printk("%s : \n", __func__);

	val = smb328a_read_reg(client, SMB328A_BATTERY_CHARGING_STATUS_C);
	if (val >= 0) {
		data = (u8)val;
		printk("%s : reg (0x%x) = 0x%x\n", __func__, SMB328A_BATTERY_CHARGING_STATUS_C, data);

		if (data&0x1)
			ret = true; /* charger enabled */
	}

	return ret;
}

static bool smb328a_check_bat_full(struct i2c_client *client)
{
	int val;
	u8 data = 0;
	bool ret = false;

	val = smb328a_read_reg(client, SMB328A_BATTERY_CHARGING_STATUS_C);
	if (val >= 0) {
		data = (u8)val;
		printk("%s : reg (0x%x) = 0x%x\n", __func__, SMB328A_BATTERY_CHARGING_STATUS_C, data);

		if (data&(0x1<<6))
			ret = true; /* full */
	}

	return ret;
}

/* vf check */
static bool smb328a_check_bat_missing(struct i2c_client *client)
{
	int val;
	u8 data = 0;
	bool ret = false;

	//printk("%s : \n", __func__);

	val = smb328a_read_reg(client, SMB328A_BATTERY_CHARGING_STATUS_B);
	if (val >= 0) {
		data = (u8)val;
		printk("%s : reg (0x%x) = 0x%x\n", __func__, SMB328A_BATTERY_CHARGING_STATUS_B, data);

		if (data&0x1) {
			printk("%s : vf is open, reg (0x%x) = 0x%x\n", __func__, SMB328A_BATTERY_CHARGING_STATUS_B, data);
			ret = true; /* missing battery */
		}
	}

	return ret;
}

/* whether valid dcin or not */
static bool smb328a_check_vdcin(struct i2c_client *client)
{
	int val;
	u8 data = 0;
	bool ret = false;

	val = smb328a_read_reg(client, SMB328A_BATTERY_CHARGING_STATUS_A);
	if (val >= 0) {
		data = (u8)val;
		printk("%s : reg (0x%x) = 0x%x\n", __func__, SMB328A_BATTERY_CHARGING_STATUS_A, data);

		if (data&(0x1<<1))
			ret = true;
	}

	return ret;
}

static bool smb328a_check_bmd_disabled(struct i2c_client *client)
{
	int val;
	u8 data = 0;
	bool ret = false;

	//printk("%s : \n", __func__);

	val = smb328a_read_reg(client, SMB328A_FUNCTION_CONTROL_B);
	if (val >= 0) {
		data = (u8)val;
		//printk("%s : reg (0x%x) = 0x%x\n", __func__, SMB328A_FUNCTION_CONTROL_B, data);

		if (data&(0x1<<7)) {
			ret = true;
			printk("%s : return ture : reg(0x%x)=0x%x (0x%x)\n", __func__,
				SMB328A_FUNCTION_CONTROL_B, data, data&(0x1<<7));
		}
	}

	val = smb328a_read_reg(client, SMB328A_OTG_PWR_AND_LDO_CONTROL);
	if (val >= 0) {
		data = (u8)val;
		//printk("%s : reg (0x%x) = 0x%x\n", __func__, SMB328A_OTG_PWR_AND_LDO_CONTROL, data);

		if ((data&(0x1<<7))==0) {
			ret = true;
			printk("%s : return ture : reg(0x%x)=0x%x (0x%x)\n", __func__,
				SMB328A_OTG_PWR_AND_LDO_CONTROL, data, data&(0x1<<7));
		}
	}

	return ret;
}

static int smb328a_chg_get_property(struct power_supply *psy,
	enum power_supply_property psp,
union power_supply_propval *val)
{
	struct smb328a_chip *chip = container_of(psy,
	struct smb328a_chip, psy_bat);

	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		if (smb328a_check_vdcin(chip->client))
			val->intval = 0; // Good
		else
			val->intval = 1; // OVP occurs!
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		if (smb328a_check_bat_missing(chip->client))
			val->intval = BAT_NOT_DETECTED;
		else
			val->intval = BAT_DETECTED;
		break;
	case POWER_SUPPLY_PROP_CAPACITY    :
		{
			unsigned int bat_per = 1;
			if (u2_get_board_rev() >= 5) {
				bat_per= d2153_battery_read_status(D2153_BATTERY_SOC);
			} else {
				get_bq27425_battery_data(BQ27425_REG_SOC, &bat_per);
			}
			val->intval = bat_per;
		}
		break;
	case POWER_SUPPLY_PROP_TYPE:
		{
			int type = get_cable_type();
			printk("%s, %d\n", __func__, type);
			switch(type)
			{
			case CABLE_TYPE_USB:
				val->intval = POWER_SUPPLY_TYPE_USB;
				break;
			case CABLE_TYPE_AC:
				val->intval = POWER_SUPPLY_TYPE_USB_DCP;
				break;
			default :
				val->intval = POWER_SUPPLY_TYPE_BATTERY;
				break;
			}
		}
		break;
	case POWER_SUPPLY_PROP_ONLINE:
		//printk("%s : check bmd available\n", __func__);
		/* check VF check available */
		if (smb328a_check_bmd_disabled(chip->client))
			val->intval = 1;
		else
			val->intval = 0;
		//printk("smb328a_check_bmd_disabled is %d\n", val->intval);
		break;
	case POWER_SUPPLY_PROP_CHARGE_FULL:
		if (smb328a_check_bat_full(chip->client))
			val->intval = 1;
		else
			val->intval = 0;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		if (u2_get_board_rev() >= 5) {
			val->intval = d2153_battery_read_status(D2153_BATTERY_CUR_VOLTAGE);
		} else {
			val->intval = pmic_read_battery_status(E_BATT_PROP_VOLTAGE);
		}
		break;
	case POWER_SUPPLY_PROP_BATT_TEMP_ADC:
		if (u2_get_board_rev() >= 5) {
			val->intval = d2153_battery_read_status(D2153_BATTERY_TEMP_ADC);
		} else {
			val->intval = pmic_get_temp_status();
		}
		break;
	case POWER_SUPPLY_PROP_CHARGE_STATUS:
		if (u2_get_board_rev() >= 5) {
			val->intval=smb328a_check_charging_status(chip->client);
			break;
		}
		/* no need
		case POWER_SUPPLY_PROP_CHARGE_TYPE:
		switch (smb328a_check_charging_status(chip->client)) {
		case 0:
		val->intval = POWER_SUPPLY_CHARGE_TYPE_NONE;
		break;
		case 1:
		val->intval = POWER_SUPPLY_CHARGE_TYPE_UNKNOWN;
		break;
		case 2:
		val->intval = POWER_SUPPLY_CHARGE_TYPE_FAST;
		break;
		case 3:
		val->intval = POWER_SUPPLY_CHARGE_TYPE_TRICKLE;
		break;
		default:
		printk("get charge type error!\n");
		return -EINVAL;
		}
		break;
		case POWER_SUPPLY_PROP_CHARGE_NOW:
		if (smb328a_check_is_charging(chip->client))
		val->intval = 1;
		else
		val->intval = 0;
		break;
		*/
	default:
		return -EINVAL;
	}
	return 0;
}

static int smb328a_set_top_off(struct i2c_client *client, int set_val)
{
	int val;
	u8 data;

	printk("%s : \n", __func__);

	smb328a_allow_volatile_writes(client);

	val = smb328a_read_reg(client, SMB328A_INPUT_AND_CHARGE_CURRENTS);
	if (val >= 0) {
		data = (u8)val;
		printk("%s : reg (0x%x) = 0x%x\n", __func__, SMB328A_INPUT_AND_CHARGE_CURRENTS, data);
		data &= 0xF8;
		data |= ((set_val / 25) - 1);
		if (smb328a_write_reg(client, SMB328A_INPUT_AND_CHARGE_CURRENTS, data) < 0) {
			printk("%s : error!\n", __func__);
			return -1;
		}
		data = smb328a_read_reg(client, SMB328A_INPUT_AND_CHARGE_CURRENTS);
		printk("%s : => reg (0x%x) = 0x%x\n", __func__, SMB328A_INPUT_AND_CHARGE_CURRENTS, data);
	}

	val = smb328a_read_reg(client, SMB328A_VARIOUS_CONTROL_FUNCTION_A);
	if (val >= 0) {
		data = (u8)val;
		printk("%s : reg (0x%x) = 0x%x\n", __func__, SMB328A_VARIOUS_CONTROL_FUNCTION_A, data);
		data &= 0x1F;
		data |= (((set_val / 25) - 1) << 5);
		if (smb328a_write_reg(client, SMB328A_VARIOUS_CONTROL_FUNCTION_A, data) < 0) {
			printk("%s : error!\n", __func__);
			return -1;
		}
		data = smb328a_read_reg(client, SMB328A_VARIOUS_CONTROL_FUNCTION_A);
		printk("%s : => reg (0x%x) = 0x%x\n", __func__, SMB328A_VARIOUS_CONTROL_FUNCTION_A, data);
	}

	return 0;
}

static int smb328a_set_charging_current(struct i2c_client *client, int chg_current)
{
	struct smb328a_chip *chip = i2c_get_clientdata(client);
    int cable_type = get_cable_type();

	if (cable_type == CABLE_TYPE_USB)
		chip->chg_mode = CHG_MODE_USB;
	else if(cable_type == CABLE_TYPE_AC)
		chip->chg_mode = CHG_MODE_AC;
	else
	    chip->chg_mode = CHG_MODE_NONE;

	return 0;
}

static int smb328a_enable_otg(struct i2c_client *client)
{
	int val;
	u8 data;

	printk("%s : \n", __func__);val = smb328a_read_reg(client, SMB328A_COMMAND);
	if (val >= 0) {
		data = (u8)val;

		printk("%s : reg (0x%x) = 0x%x\n", __func__, SMB328A_COMMAND, data);
		data = 0x80;
		if (smb328a_write_reg(client, SMB328A_COMMAND, data) < 0) {
			printk("%s : error!\n", __func__);
			return -1;
		}
		msleep(100);

		data = smb328a_read_reg(client, SMB328A_COMMAND);
		printk("%s : => reg (0x%x) = 0x%x\n", __func__, SMB328A_COMMAND, data);

	}

	val = smb328a_read_reg(client, SMB328A_FUNCTION_CONTROL_B);
	if (val >= 0) {
		data = (u8)val;
		printk("%s : reg (0x%x) = 0x%x\n", __func__, SMB328A_FUNCTION_CONTROL_B, data);
		data = 0x0C;
		if (smb328a_write_reg(client, SMB328A_FUNCTION_CONTROL_B, data) < 0) {
			printk("%s : error!\n", __func__);
			return -1;
		}
		msleep(100);
		data = smb328a_read_reg(client, SMB328A_FUNCTION_CONTROL_B);
		printk("%s : => reg (0x%x) = 0x%x\n", __func__, SMB328A_FUNCTION_CONTROL_B, data);

	}

	val = smb328a_read_reg(client, SMB328A_COMMAND);
	if (val >= 0) {
		data = (u8)val;
		if (data != 0x9a)
		{
			printk("%s : reg (0x%x) = 0x%x\n", __func__, SMB328A_COMMAND, data);
			data = 0x9a;
			if (smb328a_write_reg(client, SMB328A_COMMAND, data) < 0) {
				printk("%s : error!\n", __func__);
				return -1;
			}
			msleep(100);

			data = smb328a_read_reg(client, SMB328A_COMMAND);
			printk("%s : => reg (0x%x) = 0x%x\n", __func__, SMB328A_COMMAND, data);
		}
	}
	return 0;
}


static int smb328a_disable_otg(struct i2c_client *client)
{
	int val;
	u8 data;

	printk("%s : \n", __func__);

	//	fsa9480_otg_detach();

	val = smb328a_read_reg(client, SMB328A_FUNCTION_CONTROL_B);
	if (val >= 0) {
		data = (u8)val;
		printk("%s : reg (0x%x) = 0x%x\n", __func__, SMB328A_FUNCTION_CONTROL_B, data);

		data = 0x0C;

		if (smb328a_write_reg(client, SMB328A_FUNCTION_CONTROL_B, data) < 0) {
			printk("%s : error!\n", __func__);
			return -1;
		}

		msleep(100);

		data = smb328a_read_reg(client, SMB328A_FUNCTION_CONTROL_B);
		printk("%s : => reg (0x%x) = 0x%x\n", __func__, SMB328A_FUNCTION_CONTROL_B, data);
	}

	val = smb328a_read_reg(client, SMB328A_COMMAND);
	if (val >= 0) {
		data = (u8)val;
		printk("%s : reg (0x%x) = 0x%x\n", __func__, SMB328A_COMMAND, data);
		data = 0x98;
		if (smb328a_write_reg(client, SMB328A_COMMAND, data) < 0) {
			printk("%s : error!\n", __func__);
			return -1;
		}
		msleep(100);
		data = smb328a_read_reg(client, SMB328A_COMMAND);
		printk("%s : => reg (0x%x) = 0x%x\n", __func__, SMB328A_COMMAND, data);
	}

	return 0;
}



void smb328a_otg_enable_disable(int onoff, int cable)
{
	struct i2c_client *client = smb_charger->client;

	if (onoff)
	{
		// smb328a_charger_function_conrol(client, cable);  kbj temp
		smb328a_enable_otg(client);
	}
	else
		smb328a_disable_otg(client);
}
EXPORT_SYMBOL(smb328a_otg_enable_disable);


static void smb328a_ldo_disable(struct i2c_client *client)
{
	int val;
	u8 data;

	printk("%s : \n", __func__);

	smb328a_allow_volatile_writes(client);

	val = smb328a_read_reg(client, SMB328A_OTG_PWR_AND_LDO_CONTROL);
	if (val >= 0) {
		data = (u8)val;
		printk("%s : reg (0x%x) = 0x%x\n", __func__, SMB328A_OTG_PWR_AND_LDO_CONTROL, data);

		data |= (0x1 << 5);
		if (smb328a_write_reg(client, SMB328A_OTG_PWR_AND_LDO_CONTROL, data) < 0)
			printk("%s : error!\n", __func__);
		val = smb328a_read_reg(client, SMB328A_OTG_PWR_AND_LDO_CONTROL);
		if (val >= 0) {
			data = (u8)val;
			printk("%s : => reg (0x%x) = 0x%x\n", __func__, SMB328A_OTG_PWR_AND_LDO_CONTROL, data);
		}
	}
}

static int smb328a_enable_charging(struct i2c_client *client)
{
	int val;
	u8 data;
	struct smb328a_chip *chip = i2c_get_clientdata(client);

	val = smb328a_read_reg(client, SMB328A_COMMAND);
	if (val >= 0) {
		data = (u8)val;
		printk("%s : reg (0x%x) = 0x%x\n", __func__, SMB328A_COMMAND, data);
		//data &= ~(0x1 << 4); /* "0" turn off the charger */
		if (chip->chg_mode == CHG_MODE_AC)
			data = 0x8C;
		else if (chip->chg_mode == CHG_MODE_USB)
			data = 0x88;
		else
			data = 0x98;

		if (smb328a_write_reg(client, SMB328A_COMMAND, data) < 0) {
			printk("%s : error!\n", __func__);
			return -1;
		}
		data = smb328a_read_reg(client, SMB328A_COMMAND);
		printk("%s : => reg (0x%x) = 0x%x\n", __func__, SMB328A_COMMAND, data);
	}

	return 0;
}

static int smb328a_disable_charging(struct i2c_client *client)
{
	int val;
	u8 data;

	val = smb328a_read_reg(client, SMB328A_COMMAND);
	if (val >= 0) {
		data = (u8)val;
		printk("%s : reg (0x%x) = 0x%x\n", __func__, SMB328A_COMMAND, data);
		//data |= (0x1 << 4); /* "1" turn off the charger */
		data = 0x98;
		if (smb328a_write_reg(client, SMB328A_COMMAND, data) < 0) {
			printk("%s : error!\n", __func__);
			return -1;
		}
		data = smb328a_read_reg(client, SMB328A_COMMAND);
		printk("%s : => reg (0x%x) = 0x%x\n", __func__, SMB328A_COMMAND, data);
	}

	return 0;
}

static int smb328a_chg_set_property(struct power_supply *psy,
	enum power_supply_property psp,
	const union power_supply_propval *val)
{
	struct smb328a_chip *chip = container_of(psy,
	struct smb328a_chip, psy_bat);
	int ret = 0;
	int validval = 0;

	switch (psp) {
	case POWER_SUPPLY_PROP_CURRENT_NOW: /* step1) Set charging current */
		validval = val->intval;
		if (val->intval < 500 || val->intval > 1200) {
			validval = 500; //min current
		}

		smb328a_set_command_reg(chip->client);
		smb328a_charger_function_conrol(chip->client, validval);
		ret = smb328a_set_charging_current(chip->client, validval);
		break;

	case POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN: // step2) Set top-off current
		validval = val->intval;
		if (val->intval < 25 || val->intval > 200) {
			validval = 200; //max top-off
		}
		ret = smb328a_set_top_off(chip->client, validval);
		break;

	case POWER_SUPPLY_PROP_STATUS:	/* step3) Enable/Disable charging */
		if (val->intval == POWER_SUPPLY_STATUS_CHARGING) {
			if (chip->chg_mode == CHG_MODE_AC)
				smb328a_ldo_disable(chip->client);
			ret = smb328a_enable_charging(chip->client);
		} else {
			ret = smb328a_disable_charging(chip->client);
		}
		//smb328a_print_all_regs(chip->client);
		break;
		/*	case POWER_SUPPLY_PROP_OTG:
		if (val->intval == POWER_SUPPLY_CAPACITY_OTG_ENABLE)
		{
		smb328a_charger_function_conrol(chip->client, val->intval);
		ret = smb328a_enable_otg(chip->client);
		}
		else
		ret = smb328a_disable_otg(chip->client);
		break;*/
	default:
		return -EINVAL;
	}
	return ret;
}

static void smb328a_work_func(struct work_struct *work)
{
    struct smb328a_chip *p = container_of(work, struct smb328a_chip, work);
    pr_info("%s\n", __func__);

    if(!p)
    {
        pr_info("%s: smb328a_chip is NULL\n", __func__);
        goto error;
    }

    if(smb328a_check_bat_full(p->client))
    {
        pr_info("%s: EOC\n", __func__);
        spa_event_handler(SPA_EVT_EOC, 0);
    }

error:
    smb328a_write_reg(p->client, SMB328A_CLEAR_IRQ, 1)  ;
}

static irqreturn_t smb328a_irq_handler(int irq, void *data)
{
    struct smb328a_chip *p = (struct smb328a_chip *)data;

    schedule_work(&(p->work));

	return IRQ_HANDLED;
}

static int smb328a_irq_init(struct i2c_client *client)
{
	int ret = 0;

	if (client->irq) {
		ret = request_irq(client->irq, smb328a_irq_handler,
            (IRQF_TRIGGER_FALLING | IRQF_DISABLED | IRQF_NO_SUSPEND), "smb328a_charger", smb_charger);

		if (ret) {
			pr_err("%s: failed to reqeust IRQ\n", __func__);
			return ret;
		}
	}
    else
        pr_err("%s: SMB328A IRQ is NULL\n", __func__);

	return ret;
}

static int __devinit smb328a_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct smb328a_chip *chip;
	int ret;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE))
		return -EIO;

	pr_info("%s: SMB328A driver Loading! \n", __func__);

	chip = kzalloc(sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	smb_charger = chip;

	chip->client = client;

	i2c_set_clientdata(client, chip);

	chip->psy_bat.name = "smb328a-charger",
		chip->psy_bat.properties = smb328a_battery_props,
		chip->psy_bat.num_properties = ARRAY_SIZE(smb328a_battery_props),
		chip->psy_bat.get_property = smb328a_chg_get_property,
		chip->psy_bat.set_property = smb328a_chg_set_property,
		ret = power_supply_register(&client->dev, &chip->psy_bat);
	if (ret) {
		pr_err("Failed to register power supply psy_bat\n");
		goto err_kfree;
	}

    INIT_WORK(&(chip->work), smb328a_work_func);

	chip->chg_mode = CHG_MODE_NONE;

    smb328a_irq_init(client);

	if (u2_get_board_rev() >= 5) {
		d2153_battery_start();
	}
	return 0;

err_kfree:
	kfree(chip);
	return ret;
}

static int __devexit smb328a_remove(struct i2c_client *client)
{
	struct smb328a_chip *chip = i2c_get_clientdata(client);

	power_supply_unregister(&chip->psy_bat);
	kfree(chip);
	return 0;
}

static int smb328a_suspend(struct i2c_client *client,
	pm_message_t state)
{
	return 0;
}

static int smb328a_resume(struct i2c_client *client)
{
	return 0;
}

static const struct i2c_device_id smb328a_id[] = {
	{ "smb328a", 0 },
	{ }
};

static struct i2c_driver smb328a_i2c_driver = {
	.driver	= {
		.name	= "smb328a",
	},
	.probe		= smb328a_probe,
	.remove		= __devexit_p(smb328a_remove),
	.suspend	= smb328a_suspend,
	.resume		= smb328a_resume,
	.id_table	= smb328a_id,
};

static int __init smb328a_init(void)
{
	return i2c_add_driver(&smb328a_i2c_driver);
}

static void __exit smb328a_exit(void)
{
	i2c_del_driver(&smb328a_i2c_driver);
}

module_init(smb328a_init);
module_exit(smb328a_exit);

MODULE_DESCRIPTION("SMB328A charger control driver");
MODULE_AUTHOR("SAMSUNG");
MODULE_LICENSE("GPL");
