/*
 * arch/arm/mach-shmobile/poweroff.c
 *
 * Copyright (C) 2012 Renesas Mobile Corporation
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

#include <linux/device.h>
#include <linux/string.h>

#if 0
#include <linux/io.h>
#include <asm/cacheflush.h>

/* PLL2CR */
#define PLL2CR		__io(0xE615002C)
#define PLL2CE_XOE	0x1

/* Recovery Mode setting */
#define BOOTFLAG		0x40
#define SIZE			0x40

#define STBCHRB2		__io(0xE6180042)
#define INTERNAL_RAM0	__io(0xE63A1FC0)
#endif


#ifdef CONFIG_PMIC_INTERFACE
#include <linux/pmic/pmic.h>
#else
#define pmic_force_power_off(x)
#endif

// #define DEBUG_POWEROFF

#ifdef DEBUG_POWEROFF
	#define POWEROFF_PRINTK(fmt, arg...)  printk(fmt,##arg)
#else
	#define POWEROFF_PRINTK(fmt, arg...)
#endif


void (*shmobile_arch_reset)(char mode, const char *cmd);

#if 0
/*
 *  shmobile_pm_set_recovery_mode: Set Recovery Mode
 *	@cmd: Special boot mode
 */
void shmobile_pm_set_recovery_mode(const char *cmd)
{
	POWEROFF_PRINTK("%s\n", __func__);
	local_irq_disable();
	
	if (cmd == NULL) {
		/* copy cmd to Inter Connect RAM0 */
		strncpy((void *)INTERNAL_RAM0, "", SIZE);
	} else {
		/* copy cmd to Inter Connect RAM0 */
	 	strncpy((void *)INTERNAL_RAM0, cmd, SIZE);
	}
	
	/* set boot flag */
	__raw_writeb(__raw_readb(STBCHRB2) | BOOTFLAG, STBCHRB2);
	
	local_irq_enable();
}
#endif

/*
 *  Stop peripheral devices
 */
void shmobile_pm_stop_peripheral_devices(void)
{
	POWEROFF_PRINTK("%s\n", __func__);
	
	POWEROFF_PRINTK("Turn off SIM Reader \n");
	pmic_force_power_off(E_POWER_VUSIM1);
	
	POWEROFF_PRINTK("Turn off SD Host Interface \n");
	pmic_force_power_off(E_POWER_VIO_SD);

	POWEROFF_PRINTK("Turn off Sensors, Display and Touch module \n");
	pmic_force_power_off(E_POWER_VANA_MM);
}

/*
 * Power off
 */
static void shmobile_pm_poweroff(void)
{
	POWEROFF_PRINTK("%s\n", __func__);

	/* Turn off power of whole system */
	pmic_force_power_off(E_POWER_ALL);
	
	/* Wait for power off */
	while (1) {};
}

/*
 * regist pm_power_off
 */
static int __init shmobile_init_poweroff(void)
{
	POWEROFF_PRINTK("%s\n", __func__);
	
	/* Register globally exported PM poweroff hook */
	pm_power_off = shmobile_pm_poweroff;
	
	return 0;
}

module_init(shmobile_init_poweroff);
