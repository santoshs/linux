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

static struct spa_temp_tb batt_temp_tb[] = {
	{869, -300},		/* -30 */
	{769, -200},		/* -20 */
	{643, -100},            /* -10 */
	{568, -50},		/* -5  */
	{509,   0},             /* 0   */
	{382,  100},            /* 10  */
	{275,  200},            /* 20  */
	{231,  250},            /* 25  */
	{196,  300},            /* 30  */
	{138,  400},            /* 40  */
	{95 ,  500},            /* 50  */
	{68 ,  600},            /* 60  */
	{54 ,  650},            /* 65  */
	{46 ,  700},		/* 70  */
	{34 ,  800},		/* 80  */
};

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
#if defined(CONFIG_MACH_U2EVM)
	.batt_temp_tb = &batt_temp_tb[0],
	.batt_temp_tb_len = ARRAY_SIZE(batt_temp_tb),
#elif defined(CONFIG_MACH_GARDALTE) || defined(CONFIG_MACH_LOGANLTE) \
		|| defined(CONFIG_MACH_LT02LTE)
	.batt_temp_tb = &batt_temp_tb_d2153[0],
	.batt_temp_tb_len = ARRAY_SIZE(batt_temp_tb_d2153),
#else
#error "ERROR: None of the boards are selected.!!!."
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

