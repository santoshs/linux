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
#include <linux/io.h>
#include <asm/cacheflush.h>
#include <mach/pm.h>

/* PLL2CR */
#define PLL2CR		IO_ADDRESS(0xE615002C)
#define PLL2CE_XOE	0x1

/* Restart Mode Setting*/
#define SBAR2		IO_ADDRESS(0xe6180060)
#define RESCNT2		IO_ADDRESS(0xE6188020)

/* Power Off Mode Setting */
#define POFFFLAG	0x80
#define STBCHRB2		IO_ADDRESS(0xE6180042)


#ifdef CONFIG_PMIC_INTERFACE
#include <linux/pmic/pmic.h>
#else
#define pmic_force_power_off(x)
#endif

/* #define DEBUG_POWEROFF */

#ifdef DEBUG_POWEROFF
	#define POWEROFF_PRINTK(fmt, arg...)  printk(fmt, ##arg)
#else
	#define POWEROFF_PRINTK(fmt, arg...)
#endif

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
	POWEROFF_PRINTK("Turn off SIM Reader\n");
	pmic_force_power_off(E_POWER_VUSIM1);
	POWEROFF_PRINTK("Turn off SD Host Interface\n");
	pmic_force_power_off(E_POWER_VIO_SD);
	POWEROFF_PRINTK("Turn off Sensors, Display and Touch module\n");
	pmic_force_power_off(E_POWER_VANA_MM);
}

/*
 *  U2EVM Restart
 */
static void shmobile_pm_restart(char mode, const char *cmd)
{
	u8 reg = 0;
	POWEROFF_PRINTK("%s\n", __func__);
	/* Flush the console to make sure all the relevant messages make it
	 * out to the console drivers */
	arm_machine_flush_console();
	/* Disable interrupts first */
	local_irq_disable();
	local_fiq_disable();
	/* Clear Power off flag */
	__raw_writeb(__raw_readb(STBCHRB2) & (~POFFFLAG), STBCHRB2);
	/*
	 * Tell the mm system that we are going to reboot -
	 * we may need it to insert some 1:1 mappings so that
	 * soft boot works.
	 */
	setup_mm_for_reboot();
	/* Clean and invalidate caches */
	flush_cache_all();

	reg = __raw_readb(STBCHR2); /* read STBCHR2 for debug */
	__raw_writeb((reg | APE_RESETLOG_PM_RESTART), STBCHR2); /* write STBCHR2 for debug */

	/* Turn off caching */
	cpu_proc_fin();
	/* Push out any further dirty data, and ensure cache is empty */
	flush_cache_all();
	/* The architecture specific reboot */
	#ifndef CONFIG_PM_HAS_SECURE
	__raw_writel(0, SBAR2);
	#endif
	__raw_writel(__raw_readl(RESCNT2) | (1 << 31), RESCNT2);
}

/*
 * Power off
 */
static void shmobile_pm_poweroff(void)
{
	u8 reg;
	POWEROFF_PRINTK("%s\n", __func__);	
#if 1
	/* Disable interrupts first */
	local_irq_disable();
	local_fiq_disable();
	/* Disable XTAL2 */
	__raw_writel(__raw_readl(PLL2CR) | PLL2CE_XOE, PLL2CR);
	/* Set Power off flag */
	__raw_writeb(__raw_readb(STBCHRB2) | POFFFLAG, STBCHRB2);
	/* Clean and invalidate caches */
	flush_cache_all();
	/* Turn off caching */
	cpu_proc_fin();
	/* Push out any further dirty data, and ensure cache is empty */
	flush_cache_all();

	/* Write STBCHR2 for debug */
	reg = __raw_readb(STBCHR2); /* read STBCHR2 for debug */
	__raw_writeb((reg | APE_RESETLOG_PM_POWEROFF), STBCHR2); /* write STBCHR2 for debug */

	/* The architecture specific reboot */
	#ifndef CONFIG_PM_HAS_SECURE
	__raw_writel(0, SBAR2);
	#endif
	__raw_writel(__raw_readl(RESCNT2) | (1 << 31), RESCNT2);
#else
	/* Turn off power of whole system */
	pmic_force_power_off(E_POWER_ALL);
#endif	
	/* Wait for power off */
	while (1);
}

/*
 * regist pm_power_off
 */
static int __init shmobile_init_poweroff(void)
{
	POWEROFF_PRINTK("%s\n", __func__);
	/* Register globally exported PM poweroff and restart */
	pm_power_off = shmobile_pm_poweroff;
	arm_pm_restart = shmobile_pm_restart;
	return 0;
}

module_init(shmobile_init_poweroff);
