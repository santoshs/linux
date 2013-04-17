/*
 * arch/arm/mach-shmobile/setup-u2spa.c
 *
 * Device initialization for spa_power and spa_agent.c
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
#include <linux/delay.h>
#include <linux/wakelock.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/spa_power.h>
#include <linux/spa_agent.h>
#include <linux/platform_device.h>

#if defined(CONFIG_CHARGER_SMB358)
#include <linux/smb358_charger.h>

static struct smb358_platform_data smb358_info = {
	.irq = GPIO_SMB358_INT,
};

struct platform_device smb358_charger =  {
	.name		= "smb358",
	.id		= CHARGER_I2C_BUS_ID,
	.dev		= {
		.platform_data = &smb358_info,
	},
};
#endif

#if defined(CONFIG_STC3115_FUELGAUGE)
#include <linux/power_supply.h>
#include <linux/stc3115_battery.h>

#ifdef CONFIG_CHARGER_RT9532
extern int rt9532_get_charger_online(void);
#endif

static int Temperature_fn(void)
{
	return 25;
}

struct stc311x_platform_data stc3115_data = {
	.battery_online = NULL,
#ifdef CONFIG_CHARGER_RT9532
	.charger_online = rt9532_get_charger_online,
	/* used in stc311x_get_status()*/
#else
	.charger_online = NULL,		/* used in stc311x_get_status()*/
#endif
	.charger_enable = NULL,		/* used in stc311x_get_status()*/
	.power_supply_register = NULL,
	.power_supply_unregister = NULL,

	.Vmode = 0,		/*REG_MODE, BIT_VMODE 1=Voltage mode, 0=mixed mode */
	.Alm_SOC = 15,		/* SOC alm level % */
	.Alm_Vbat = 3400,	/* Vbat alm level mV */
	.CC_cnf = 241,      /* nominal CC_cnf, coming from battery characterisation*/
	.VM_cnf = 295,      /* nominal VM cnf , coming from battery characterisation*/
	.Cnom = 1200,       /* nominal capacity in mAh, coming from battery characterisation*/
	.Rsense = 10,		/* sense resistor mOhms */
	.RelaxCurrent = 100, /* current for relaxation in mA (< C/20) */
	.Adaptive = 1,		/* 1=Adaptive mode enabled, 0=Adaptive mode disabled */

	.CapDerating[6] = 277,   /* capacity derating in 0.1%, for temp = -20°C */
	.CapDerating[5] = 82,   /* capacity derating in 0.1%, for temp = -10°C */
	.CapDerating[4] = 23,   /* capacity derating in 0.1%, for temp = 0°C */
	.CapDerating[3] = 19,   /* capacity derating in 0.1%, for temp = 10°C */
	.CapDerating[2] = 0,   /* capacity derating in 0.1%, for temp = 25°C */
	.CapDerating[1] = 0,   /* capacity derating in 0.1%, for temp = 40°C */
	.CapDerating[0] = 0,   /* capacity derating in 0.1%, for temp = 60°C */

	.OCVOffset[15] = -22,    /* OCV curve adjustment */
	.OCVOffset[14] = -9,   /* OCV curve adjustment */
	.OCVOffset[13] = -15,    /* OCV curve adjustment */
	.OCVOffset[12] = -2,    /* OCV curve adjustment */
	.OCVOffset[11] = 0,	/* OCV curve adjustment */
	.OCVOffset[10] = -2,    /* OCV curve adjustment */
	.OCVOffset[9] = -26,     /* OCV curve adjustment */
	.OCVOffset[8] = -6,      /* OCV curve adjustment */
	.OCVOffset[7] = -7,      /* OCV curve adjustment */
	.OCVOffset[6] = -14,    /* OCV curve adjustment */
	.OCVOffset[5] = -23,    /* OCV curve adjustment */
	.OCVOffset[4] = -46,     /* OCV curve adjustment */
	.OCVOffset[3] = -27,    /* OCV curve adjustment */
	.OCVOffset[2] = -34,     /* OCV curve adjustment */
	.OCVOffset[1] = -125,    /* OCV curve adjustment */
	.OCVOffset[0] = -68,     /* OCV curve adjustment */

	.OCVOffset2[15] = -58,    /* OCV curve adjustment */
	.OCVOffset2[14] = -37,   /* OCV curve adjustment */
	.OCVOffset2[13] = -21,    /* OCV curve adjustment */
	.OCVOffset2[12] = -14,    /* OCV curve adjustment */
	.OCVOffset2[11] = -6,    /* OCV curve adjustment */
	.OCVOffset2[10] = -16,    /* OCV curve adjustment */
	.OCVOffset2[9] = -6,     /* OCV curve adjustment */
	.OCVOffset2[8] = 4,      /* OCV curve adjustment */
	.OCVOffset2[7] = 9,      /* OCV curve adjustment */
	.OCVOffset2[6] = 11,    /* OCV curve adjustment */
	.OCVOffset2[5] = 24,    /* OCV curve adjustment */
	.OCVOffset2[4] = 7,     /* OCV curve adjustment */
	.OCVOffset2[3] = 28,    /* OCV curve adjustment */
	.OCVOffset2[2] = 89,     /* OCV curve adjustment */
	.OCVOffset2[1] = 94,    /* OCV curve adjustment */
	.OCVOffset2[0] = 0,     /* OCV curve adjustment */

	/*if the application temperature data is preferred than the STC3115 temperature */
	.ExternalTemperature = Temperature_fn,	/*External temperature fonction, return C */
	.ForceExternalTemperature = 0,	/* 1=External temperature, 0=STC3115 temperature */

};
#endif

/* Samsung charging feature
 +++ for board files, it may contain changeable values */
static struct spa_temp_tb batt_temp_tb_d2153[] = {
	{2300, -300},            /* -30 */
	{2144, -200},            /* -20 */
	{1341, -100},            /* -10 */
	{1072,  -50},            /* -5 */
	{865,     0},            /* 0   */
	{577,   100},            /* 10  */
	{400,   200},            /* 20  */
	{334,   250},            /* 25  */
	{285,   300},            /* 30  */
	{199,   400},            /* 40  */
	{143,   500},            /* 50  */
	{106,   600},            /* 60  */
	{ 93,   650},            /* 65  */
	{ 83,   700},            /* 70  */
	{ 75 ,  800},            /* 80  */
};


#if defined(CONFIG_MACH_LT02LTE)
struct spa_power_data spa_power_pdata = {
	.charger_name = "spa_agent_chrg",/*"smb358-charger" to be used if no GED*/
	.eoc_current = 180,
	.recharge_voltage = 4180,
	.charging_cur_usb = 500,
	.charging_cur_wall = 2000,
	.suspend_temp_hot = 600,
	.recovery_temp_hot = 400,
	.suspend_temp_cold = -50,
	.recovery_temp_cold = 0,
	.charge_timer_limit = CHARGE_TIMER_6HOUR,
	.batt_temp_tb = &batt_temp_tb_d2153[0],
	.batt_temp_tb_len = ARRAY_SIZE(batt_temp_tb_d2153),
#else
static struct spa_power_data spa_power_pdata = {
	/* GED changes are not yet pulled in charger driver.
	 * Hence we aree using smb328a */
	.charger_name = "smb328a-charger",/*"spa_agent_chrg",*/
	.eoc_current = 100,
	.recharge_voltage = 4150,
	.charging_cur_usb = 500,
	.charging_cur_wall = 800,
	.suspend_temp_hot = 600,
	.recovery_temp_hot = 400,
	.suspend_temp_cold = -50,
	.recovery_temp_cold = 0,
	.charge_timer_limit = CHARGE_TIMER_6HOUR,
#if defined(CONFIG_MACH_GARDALTE) || defined(CONFIG_MACH_LOGANLTE)
	.batt_temp_tb = &batt_temp_tb_d2153[0],
	.batt_temp_tb_len = ARRAY_SIZE(batt_temp_tb_d2153),
#else
#error "ERROR: None of the boards are selected.!!!."
#endif
#endif

};

static struct platform_device spa_power_device = {
	.name = "spa_power",
	.id = -1,
	.dev.platform_data = &spa_power_pdata,
};

static struct platform_device spa_agent_device = {
	.name = "spa_agent",
	.id = -1,
};

int init_spa_power(void)
{
	int ret = 0;
	ret = platform_device_register(&spa_agent_device);
	if (ret < 0)
		return ret;

	ret = platform_device_register(&spa_power_device);
	if (ret < 0)
		return ret;

	return 0;
}
/* End of FIle */

