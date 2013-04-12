/*
 * arch/arm/mach-shmobile/pm.c
 *
 * Copyright (C) 2012 Renesas Mobile Corporation.
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
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/cpuidle.h>
#include <asm/proc-fns.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <mach/system.h>
#include <mach/pm.h>
#include <mach/memory-r8a7373.h>
#include <linux/wakelock.h>
#include <linux/spinlock_types.h>
#include <linux/cpu.h>
#include <linux/delay.h>
#include <memlog/memlog.h>

#ifndef CONFIG_PM_HAS_SECURE
#include "pm_ram0.h"
#else /*CONFIG_PM_HAS_SECURE*/
#include "pm_ram0_tz.h"
#endif /*CONFIG_PM_HAS_SECURE*/
#include "pmRegisterDef.h"
#include <mach/sbsc.h>

DEFINE_SPINLOCK(clock_lock);

unsigned int *cpu0BackupArea;
unsigned int *cpu1BackupArea;
int chip_rev;
unsigned int is_suspend_request;
#define KICK_WAIT_INTERVAL_US	10
#define ZSFC_MASK (0xF << 8)

/*
 * core_wait_kick: wait for KICK bit change
 *
 * Arguments:
 *		@time: wait time.
 *
 * Return:
 *		0: successful
 * -EBUSY: unsuccessful
 */
int core_wait_kick(int time)
{
	int wait_time = time;

	while (0 < wait_time--) {
		if ((__raw_readl(FRQCRB) >> 31) == 0)
			break;
		udelay(1);
	}

	return (wait_time <= 0) ? -EBUSY : 0;
}
/*
 * core_set_kick: set and wait for KICK bit cleared
 *
 * Arguments:
 *		@time: wait time.
 *
 * Return:
 *		0: successful
 *		-EBUSY: operation fail
 */
int core_set_kick(int time)
{
	int wait_time = time;

	if ((wait_time <= 0) || (wait_time > KICK_WAIT_INTERVAL_US))
		wait_time = KICK_WAIT_INTERVAL_US;
	__raw_writel(BIT(31) | __raw_readl(FRQCRB), FRQCRB);

	return core_wait_kick(wait_time);
}


/*
 * clock_update
 *
 * Arguments:
 *		@freqA: value of freqA need to be changed.
 *		@freqB: value of freqB need to be changed.
 *		@freqA_mask: mask of freqA need to be changed.
 *		@freqB_mask: mask of freqB need to be changed.
 *
 * Return:
 *		0: successful
 * -EBUSY: - operation fail of KICK bit
 *           OR the hwspinlock was already taken
 * -EINVAL: @hwlock is invalid.
 */

int clock_update(unsigned int freqA, unsigned int freqA_mask,
				unsigned int freqB, unsigned int freqB_mask)
{
	unsigned int current_value;
	int ret;
	int freqA_change = 0;
	int freqB_change = 0;
	int zs_change = 0;

	/* check if freqA change */
	current_value = __raw_readl(FRQCRA);
	if (freqA != (current_value & freqA_mask))
		freqA_change = 1;
	/* check if freqB change */
	current_value = __raw_readl(FRQCRB);
	if ((freqB & ZSFC_MASK) != (current_value & ZSFC_MASK)) {
                zs_change = 1;
		ret = hwspin_trylock_nospin(gen_sem1); /* ZS_CLK_SEM */
		if (ret) {
			printk(KERN_INFO "[%s:%d] fail to get hwsem, ret:%d\n",
				__func__, __LINE__, ret);
			return ret;
		}
	} else if (freqB != (current_value & freqB_mask))
		freqB_change = 1;

	/* wait for KICK bit change if any */
	ret = core_wait_kick(KICK_WAIT_INTERVAL_US);
	if (ret) {
		printk(KERN_INFO "[%s:%d] fail KICK bit, ret:%d\n",
			__func__, __LINE__, ret);
		if (zs_change)
			hwspin_unlock_nospin(gen_sem1);
		return ret;
	}

	if (freqA_change || zs_change || freqB_change) {
		/* FRQCRA_B_SEM */
		ret = hwspin_trylock_nospin(sw_cpg_lock);
		if (ret) {
			printk(KERN_INFO "[%s:%d] fail to get hwsem, ret:%d\n",
				__func__, __LINE__, ret);
			if (zs_change)
				hwspin_unlock_nospin(gen_sem1);
			return ret;
		}
		/* update value change */
		if (freqA_change)
			__raw_writel(freqA | (__raw_readl(FRQCRA) &
						(~freqA_mask)),
					FRQCRA);
		if (zs_change || freqB_change)
			__raw_writel(freqB | (__raw_readl(FRQCRB) &
						(~freqB_mask)),
				FRQCRB);

		/* set and wait for KICK bit changed */
		ret = core_set_kick(KICK_WAIT_INTERVAL_US);
		if (ret) {
			printk(KERN_INFO "[%s:%d] fail KICK bit, ret:%d\n",
				__func__, __LINE__, ret);
			if (zs_change)
				hwspin_unlock_nospin(gen_sem1);
			hwspin_unlock_nospin(sw_cpg_lock);
			return ret;
		}

		/* Release SEM */
		if (zs_change)
			hwspin_unlock_nospin(gen_sem1);
		hwspin_unlock_nospin(sw_cpg_lock);
	}

	/* successful change,...*/
	return ret;
}

#define PLL3CR_MASK		0x3F000000

unsigned int suspend_ZB3_backup(void)
{
	unsigned int zb3_clk = 0;
	zb3_clk = shmobile_get_ape_req_freq();
	return zb3_clk;
}


/*
 * shmobile_init_pm: Initialization of CPU's idle & suspend PM
 * return:
 *	0: successful
 *	-EIO: failed ioremap, or failed registering a CPU's idle & suspend PM
 */
int shmobile_init_pm(void)
{
	unsigned int smstpcr5_val;
	unsigned int mstpsr5_val;
	unsigned long flags;
	void __iomem *map = NULL;
	unsigned long cpuidle_spinlock;
#ifdef CONFIG_MEMLOG
	int i;
#endif
	is_suspend_request = 0;
		/* Chip revision */
	chip_rev = shmobile_chip_rev();

	spin_lock_irqsave(&clock_lock, flags);
	/* Internal RAM0 Module Clock ON */
	/* W/A of errata ES2 E0263 */
	mstpsr5_val = __raw_readl(MSTPSR5);
	if (0 != (mstpsr5_val & (MSTPST527 | MSTPST529))) {
		smstpcr5_val = __raw_readl(SMSTPCR5);
		__raw_writel((smstpcr5_val & (~(MSTP527 | MSTP529)))
						, SMSTPCR5);
		do {
			mstpsr5_val = __raw_readl(MSTPSR5);
		} while (mstpsr5_val & (MSTPST527 | MSTPST529));
	}

#ifndef CONFIG_PM_HAS_SECURE
	/* Internal RAM1 Module Clock ON */
	mstpsr5_val = __raw_readl(MSTPSR5);
	if (0 != (mstpsr5_val & MSTPST528)) {
		smstpcr5_val = __raw_readl(SMSTPCR5);
		__raw_writel((smstpcr5_val & (~MSTP528)), SMSTPCR5);
		do {
			mstpsr5_val = __raw_readl(MSTPSR5);
		} while (mstpsr5_val & MSTPST528);
	}
#endif
	spin_unlock_irqrestore(&clock_lock, flags);
	/* Allocate CPU0 back up area */
	cpu0BackupArea = kmalloc(saveCpuRegisterAreaSize, GFP_KERNEL);
	if (cpu0BackupArea == NULL)
		printk(KERN_ERR "shmobile_init_cpuidle: "
			"Failed Allocate CPU0 back up area\n");
	else
		__raw_writel((unsigned int)cpu0BackupArea,
						ram0Cpu0RegisterArea);

	/* Allocate CPU1 back up area */
	cpu1BackupArea = kmalloc(saveCpuRegisterAreaSize, GFP_KERNEL);
	if (cpu1BackupArea == NULL)
		printk(KERN_ERR "shmobile_init_cpuidle: "
			"Failed Allocate CPU1 back up area\n");
	else
		__raw_writel((unsigned int)cpu1BackupArea,
						ram0Cpu1RegisterArea);

	/* Initialize SpinLock setting */
	cpuidle_spinlock = SDRAM_NON_SECURE_SPINLOCK_START_ADDR;

	map = ioremap_nocache(cpuidle_spinlock,
							0x00000400/*1k*/);
	if (map != NULL) {
		__raw_writel((unsigned long)map, IOMEM(ram0SpinLockVA));
		__raw_writel(cpuidle_spinlock,
						IOMEM(ram0SpinLockPA));
		__raw_writel((unsigned long)0x0, IOMEM(map));
	} else {
		printk(KERN_ERR "shmobile_init_cpuidle: Failed ioremap\n");
		return -EIO;
	}

	__raw_writel((unsigned long)0x0, IOMEM(ram0CPU0SpinLock));
	__raw_writel((unsigned long)0x0, IOMEM(ram0CPU1SpinLock));
	/* Errata(ECR0285) */
	if (chip_rev <= ES_REV_2_1)
		__raw_writel((unsigned long)0x0, IOMEM(ram0ES_2_2_AndAfter));
	else if (chip_rev == ES_REV_2_2)
		__raw_writel((unsigned long)0x1, IOMEM(ram0ES_2_2_AndAfter));
	else
		__raw_writel((unsigned long)0x2, IOMEM(ram0ES_2_2_AndAfter));

	/* Initialize internal setting */
	__raw_writel((unsigned long)CPUSTATUS_RUN, IOMEM(ram0Cpu0Status));
	__raw_writel((unsigned long)CPUSTATUS_RUN, IOMEM(ram0Cpu1Status));

#ifdef CONFIG_MEMLOG
	/* Initialize memlog pm setting (ioremap 2k)*/
	map = ioremap_nocache((unsigned long)(
		MEMLOG_ADDRESS + CPU0_PM_START_INDEX),
		CPU0_PM_SIZE + CPU1_PM_SIZE);
	if (map != NULL) {
		__raw_writel((unsigned long)map, IOMEM(ram0MemlogPmAddressVA));
		__raw_writel((unsigned long)(
			MEMLOG_ADDRESS + CPU0_PM_START_INDEX),
			IOMEM(ram0MemlogPmAddressPAPhys));
		for (i = 0 ; i <  ((CPU0_PM_SIZE + CPU1_PM_SIZE) >> 2); i++) {
			__raw_writel((unsigned long)0x0, IOMEM(map));
			map++;
		}
	} else {
		printk(KERN_ERR "shmobile_init_pm: Failed ioremap\n");
		return -EIO;
	}
#endif
#ifdef CONFIG_PM_HAS_SECURE

	/* Initialize sec_hal allocation */
	sec_hal_pm_coma_entry_init();

	/* Initialize internal setting */
	__raw_writel((unsigned long)(&sec_hal_pm_coma_entry),
					IOMEM(ram0SecHalCommaEntry));
	__raw_writel((unsigned long)0x0, IOMEM(ram0ZClockFlag));
#endif

#ifndef CONFIG_PM_SMP
	/* Temporary solution for Kernel in Secure */
#ifndef CONFIG_PM_HAS_SECURE
	__raw_writel(0, IOMEM(SBAR2));
#endif

	__raw_writel((unsigned long)0x0, IOMEM(APARMBAREA)); /* 4k */
#endif
#ifndef CONFIG_PM_HAS_SECURE
	/* Copy the source code internal RAM1 */
	(void)memcpy((void *)secramArmVector,
				(void *)&ArmVector,
				fsArmVector);

	(void)memcpy((void *)secramPM_Spin_Lock,
		(void *)&PM_Spin_Lock,
		fsPM_Spin_Lock);

	(void)memcpy((void *)secramPM_Spin_Unlock,
		(void *)&PM_Spin_Unlock,
		fsPM_Spin_Unlock);

	(void)memcpy((void *)secramDisableMMU,
				(void *)&disablemmu,
				fsDisableMMU);

	(void)memcpy((void *)secramRestoreArmRegisterPA,
				(void *)&restore_arm_register_pa,
				fsRestoreArmRegisterPA);

	(void)memcpy((void *)secramRestoreCommonRegister,
				(void *)&restore_common_register,
				fsRestoreCommonRegister);

	(void)memcpy((void *)secramSysPowerDown,
				(void *)&sys_powerdown,
				fsSysPowerDown);

	(void)memcpy((void *)secramSysPowerUp,
				(void *)&sys_powerup,
				fsSysPowerUp);

	(void)memcpy((void *)secramSystemSuspendCPU0PA,
				(void *)&systemsuspend_cpu0_pa,
				fsSystemSuspendCPU0PA);

	(void)memcpy((void *)secramCoreStandbyPA,
				(void *)&corestandby_pa,
				fsCoreStandbyPA);

	(void)memcpy((void *)secramCoreStandbyPA2,
				(void *)&corestandby_pa_2,
				fsCoreStandbyPA2);

	(void)memcpy((void *)secramSystemSuspendCPU1PA,
				(void *)&systemsuspend_cpu1_pa,
				fsSystemSuspendCPU1PA);

	(void)memcpy((void *)secramcorestandby_down_status,
				(void *)&corestandby_down_status,
				fscorestandby_down_status);

	(void)memcpy((void *)secramcorestandby_up_status,
				(void *)&corestandby_up_status,
				fscorestandby_up_status);

	(void)memcpy((void *)secramMemoryLogPm,
				(void *)&memory_log_pm,
				fsMemoryLogPm);
#else /*CONFIG_PM_HAS_SECURE*/
	/* Copy the source code internal RAM0 */
	(void)memcpy((void *)ram0ArmVector,
				(void *)&ArmVector,
				fsArmVector);

	(void)memcpy((void *)ram0CoreStandby,
				(void *)&corestandby,
				fsCoreStandby);

	(void)memcpy((void *)ram0CoreStandby_2,
				(void *)&corestandby_2,
				fsCoreStandby_2);

	(void)memcpy((void *)ram0SystemSuspend,
				(void *)&systemsuspend,
				fsSystemSuspend);

	(void)memcpy((void *)ram0SaveArmRegister,
				(void *)&save_arm_register,
				fsSaveArmRegister);

	(void)memcpy((void *)ram0RestoreArmRegisterPA,
				(void *)&restore_arm_register_pa,
				fsRestoreArmRegisterPA);

	(void)memcpy((void *)ram0RestoreArmRegisterVA,
				(void *)&restore_arm_register_va,
				fsRestoreArmRegisterVA);

	(void)memcpy((void *)ram0SaveArmCommonRegister,
				(void *)&save_arm_common_register,
				fsSaveArmCommonRegister);

	(void)memcpy((void *)ram0RestoreArmCommonRegister,
				(void *)&restore_arm_common_register,
				fsRestoreArmCommonRegister);

	(void)memcpy((void *)ram0PM_Spin_Lock,
		(void *)&PM_Spin_Lock,
		fsPM_Spin_Lock);

	(void)memcpy((void *)ram0PM_Spin_Unlock,
		(void *)&PM_Spin_Unlock,
		fsPM_Spin_Unlock);

	(void)memcpy((void *)ram0SysPowerDown,
				(void *)&sys_powerdown,
				fsSysPowerDown);

	(void)memcpy((void *)ram0SysPowerUp,
				(void *)&sys_powerup,
				fsSysPowerUp);
	(void)memcpy((void *)ram0MemoryLogPm,
				(void *)&memory_log_pm,
				fsMemoryLogPm);
#endif /* CONFIG_PM_HAS_SECURE */

	/* - set PLL1 stop conditon to A2SL, A3R, C4 state by CPG.PLL1STPCR */
	__raw_writel(PLL1STPCR_DEFALT, PLL1STPCR);

	return 0;
}

