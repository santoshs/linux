/*
 * arch/arm/mach-shmobile/cpuidle.c
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
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/cpuidle.h>
#include <asm/proc-fns.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <mach/system.h>
#include <mach/pm.h>
#include <linux/wakelock.h>
#include <linux/spinlock_types.h>
#include <linux/cpu.h>

#ifndef CONFIG_PM_HAS_SECURE
#include "pm_ram0.h"
#else /*CONFIG_PM_HAS_SECURE*/
#include "pm_ram0_tz.h"
#endif /*CONFIG_PM_HAS_SECURE*/
#include "pmRegisterDef.h"

#define SHMOBILE_MAX_STATES	4

#define DISPLAY_LOG 0

#ifdef CONFIG_PM_HAS_SECURE
static int sec_hal_fail_cpu0;
module_param(sec_hal_fail_cpu0, int, S_IRUGO | S_IWUSR | S_IWGRP);

static int sec_hal_fail_cpu1;
module_param(sec_hal_fail_cpu1, int, S_IRUGO | S_IWUSR | S_IWGRP);
#endif /*CONFIG_PM_HAS_SECURE*/

static DEFINE_SPINLOCK(clock_lock);

static DEFINE_PER_CPU(struct cpuidle_device, shmobile_cpuidle_device);

unsigned int *cpu0BackupArea;
unsigned int *cpu1BackupArea;


static struct cpuidle_driver shmobile_idle_driver = {
	.name =			"shmobile_idle",
	.owner =		THIS_MODULE,
};


/*
 * ********************************************************************
 *     Drivers interface
 * ********************************************************************
 */
static LIST_HEAD(state_notify_list);
static LIST_HEAD(state_notify_confirm_list);
static DEFINE_MUTEX(state_notify_lock);
static DEFINE_MUTEX(state_notify_confirm_lock);

/*
 * register_pm_state_notify: registers a notification callback function
 * for PM state
 * @h: A callback function address to be notified PM state of AP-System domain
 */
void register_pm_state_notify(struct pm_state_notify *h)
{
	mutex_lock(&state_notify_lock);
	list_add_tail(&h->link, &state_notify_list);
	mutex_unlock(&state_notify_lock);
}
EXPORT_SYMBOL(register_pm_state_notify);

/*
 * unregister_pm_state_notify: unregisters a notification callback function
 * for PM state.
 * @h: A callback function address to be notified PM state of AP-System domain
 */
void unregister_pm_state_notify(struct pm_state_notify *h)
{
	mutex_lock(&state_notify_lock);
	list_del_init(&h->link);
	mutex_unlock(&state_notify_lock);
}
EXPORT_SYMBOL(unregister_pm_state_notify);

/*
 * register_pm_state_notify_confirm: registers a callback function to be
 * confirmed whether a notification is necessary when PM state is changed
 * @h: A callback function address to be confirmed PM state of AP-System domain
 */
void register_pm_state_notify_confirm(struct pm_state_notify_confirm *h)
{
	mutex_lock(&state_notify_confirm_lock);
	list_add_tail(&h->link, &state_notify_confirm_list);
	mutex_unlock(&state_notify_confirm_lock);
}
EXPORT_SYMBOL(register_pm_state_notify_confirm);

/*
 * unregister_pm_state_notify_confirm: unregisters a confirmation
 * callback function of a notification.
 * @h: A callback function address to be confirmed PM state of AP-System domain
 */
void unregister_pm_state_notify_confirm(struct pm_state_notify_confirm *h)
{
	mutex_lock(&state_notify_confirm_lock);
	list_del_init(&h->link);
	mutex_unlock(&state_notify_confirm_lock);
}
EXPORT_SYMBOL(unregister_pm_state_notify_confirm);

/*
 * state_notify: notify the state.
 * @state: the state
 * return:
 *		0: successful
 *
 * The caller must call after irq disabled
 */
unsigned int state_notify(int state)
{
	struct pm_state_notify *pos;
	int ret;

	list_for_each_entry(pos, &state_notify_list, link) {
		if (pos->notify != NULL)
			ret = pos->notify(state);
	}
	return 0;
}

/*
 * state_notify_confirm: confirm for the state to be nofified.
 * return:
 *		0: successful
 *		Otherwise: error of confirmation callback function.
 *
 * The caller must call after irq disabled
 */
unsigned int state_notify_confirm(void)
{
	struct pm_state_notify_confirm *pos;
	int error = 0;

	list_for_each_entry(pos, &state_notify_confirm_list, link) {
		if (pos->confirm != NULL)
			error = pos->confirm();
		if (error) {
			pr_debug("pm state_notify: skip notify %s\n",
				pos->name ? pos->name : "No Name");
			goto End;
		}
	}
End:
	return error;
}


/*
 * shmobile_enter_wfi_debug: executes idle PM for a CPU - WFI state
 * @dev: the target CPU
 * @state: the state
 * return:
 *		int: the idle duration
 */
static int shmobile_enter_wfi_debug(struct cpuidle_device *dev,
				struct cpuidle_state *state)
{
	struct timeval beforeTime, afterTime;
	int idle_time;
#ifndef CONFIG_PM_HAS_SECURE
	local_fiq_disable();
#endif
	do_gettimeofday(&beforeTime);

	/* Sleep State Notify */
	if (!state_notify_confirm())
		state_notify(PM_STATE_NOTIFY_SLEEP);

	arch_idle();		/* WFI cpu_do_idle(); */

	/* WakeUp State Notify */
	if (!state_notify_confirm())
		state_notify(PM_STATE_NOTIFY_WAKEUP);

	do_gettimeofday(&afterTime);

	idle_time = (afterTime.tv_sec - beforeTime.tv_sec) * 1000000
				+ (afterTime.tv_usec - beforeTime.tv_usec);

	local_irq_enable();
#ifndef CONFIG_PM_HAS_SECURE
	local_fiq_enable();
#endif
	return idle_time;
}

/*
 * shmobile_enter_wfi: executes idle PM for a CPU - WFI state
 * @dev: the target CPU
 * @state: the state
 * return:
 *		int: the idle duration
 */
static int shmobile_enter_wfi(struct cpuidle_device *dev,
				struct cpuidle_state *state)
{
	struct timeval beforeTime, afterTime;
	int idle_time;
#ifndef CONFIG_PM_HAS_SECURE
	local_fiq_disable();
#endif
	do_gettimeofday(&beforeTime);

	/* Sleep State Notify */
	if (!state_notify_confirm())
		state_notify(PM_STATE_NOTIFY_SLEEP);

	/* Transition to WFI setting	*/
	start_wfi();

	/* WakeUp State Notify */
	if (!state_notify_confirm())
		state_notify(PM_STATE_NOTIFY_WAKEUP);

	do_gettimeofday(&afterTime);

	idle_time = (afterTime.tv_sec - beforeTime.tv_sec) * 1000000
				+ (afterTime.tv_usec - beforeTime.tv_usec);

	local_irq_enable();
#ifndef CONFIG_PM_HAS_SECURE
	local_fiq_enable();
#endif
	return idle_time;
}

/*
 * shmobile_enter_wfi_lowfreq: executes idle PM for a CPU - WFI(low-freq) state
 * @dev: the target CPU
 * @state: the state
 * return:
 *		int: the idle duration
 */
static int shmobile_enter_wfi_lowfreq(struct cpuidle_device *dev,
				struct cpuidle_state *state)
{
	struct timeval beforeTime, afterTime;
	int idle_time;
#ifndef CONFIG_PM_HAS_SECURE
	local_fiq_disable();
#endif
	do_gettimeofday(&beforeTime);

	/* Sleep State Notify */
	if (!state_notify_confirm())
		state_notify(PM_STATE_NOTIFY_SLEEP_LOWFREQ);

	/* Transition to WFI standby with low-frequency setting	*/
	start_wfi2();

	/* WakeUp State Notify */
	if (!state_notify_confirm())
		state_notify(PM_STATE_NOTIFY_WAKEUP);

	do_gettimeofday(&afterTime);

	idle_time = (afterTime.tv_sec - beforeTime.tv_sec) * 1000000
				+ (afterTime.tv_usec - beforeTime.tv_usec);

	local_irq_enable();
#ifndef CONFIG_PM_HAS_SECURE
	local_fiq_enable();
#endif
	return idle_time;
}

/*
 * shmobile_enter_corestandby: executes idle PM for a CPU - CoreStandby state
 * @dev: the target CPU
 * @state: the state
 * return:
 *		int: the idle duration
 */
static int shmobile_enter_corestandby(struct cpuidle_device *dev,
						struct cpuidle_state *state)
{
	struct timeval beforeTime, afterTime;
	int idle_time;
	long wakelock;
	int sec_ret = 0;
	int cpuid = smp_processor_id();

#if DISPLAY_LOG
	printk(KERN_INFO "Standby IN  %d\n", cpuid);
#endif
#ifndef CONFIG_PM_HAS_SECURE
	local_fiq_disable();
#endif
	do_gettimeofday(&beforeTime);

	/* Core Standby wakelock check */
	wakelock = has_wake_lock(WAKE_LOCK_IDLE);
	if (!wakelock) {
		/* Core Standby State Notify */
		if (!state_notify_confirm())
			state_notify(PM_STATE_NOTIFY_CORESTANDBY);
#ifdef CORESTANDBY_DFS
		corestandby_cpufreq();
#endif
#ifdef CONFIG_PM_HAS_SECURE
		if (cpuid == 0)
			__raw_writel(0, ram0SecHalReturnCpu0);
		else
			__raw_writel(0, ram0SecHalReturnCpu1);
#endif /*CONFIG_PM_HAS_SECURE*/

		start_corestandby(); /* CoreStandby(A1SL0 or A1SL1 Off) */
#ifdef CONFIG_PM_HAS_SECURE
		if (cpuid == 0)
			sec_ret = __raw_readl(ram0SecHalReturnCpu0);
		else
			sec_ret = __raw_readl(ram0SecHalReturnCpu1);
		if (sec_ret) {
			if (cpuid == 0)
				sec_hal_fail_cpu0++;
			else
				sec_hal_fail_cpu1++;
		}
#endif /*CONFIG_PM_HAS_SECURE*/

	} else {
#if DISPLAY_LOG
		printk(KERN_INFO "Core-Standby %d (WAKELOCK)", cpuid);
#endif
		/* Sleep State Notify */
		if (!state_notify_confirm())
			state_notify(PM_STATE_NOTIFY_SLEEP);
		arch_idle(); /* WFI cpu_do_idle(); */
	}

	/* WakeUp State Notify */
	if (!state_notify_confirm())
		state_notify(PM_STATE_NOTIFY_WAKEUP);

	do_gettimeofday(&afterTime);

	idle_time = (afterTime.tv_sec - beforeTime.tv_sec) * 1000000
				+ (afterTime.tv_usec - beforeTime.tv_usec);

	local_irq_enable();
#ifndef CONFIG_PM_HAS_SECURE
	local_fiq_enable();
#endif
#if DISPLAY_LOG
	printk(KERN_INFO "Standby OUT %d IDLE=0x%x\n", cpuid, idle_time);
#endif

	return idle_time;
}

/*
 * corestandby_2_pll1_condition_set
 *
 * return:
 *		0: successful
 *		-1: C4 is set to PLL1 stop condition.
 */
static int corestandby_2_pll1_condition_set(void)
{
	if ((__raw_readl(CPG_MSTPSR1) & MSTPST1_PLL1) != MSTPST1_PLL1)
		goto set_pll1_c4;

	if ((__raw_readl(CPG_MSTPSR2) & MSTPST2_PLL1) != MSTPST2_PLL1)
		goto set_pll1_c4;

	if ((__raw_readl(CPG_MSTPSR3) & MSTPST3_PLL1) != MSTPST3_PLL1)
		goto set_pll1_c4;

	if ((__raw_readl(CPG_MSTPSR4) & MSTPST4_PLL1) == MSTPST4_PLL1)
		goto set_pll1_c4_skip;

set_pll1_c4:
	__raw_writel(__raw_readl(CPG_PLL1STPCR) | C4STP, CPG_PLL1STPCR);
	return -1;

set_pll1_c4_skip:
	return 0;
}

static void corestandby_2_pll1_condition_at_wakeup(void)
{
	__raw_writel(__raw_readl(CPG_PLL1STPCR) & (~C4STP), CPG_PLL1STPCR);
}

#define POWER_BBPLLST					BIT(7)
#define POWER_BBPLLOFF					BIT(7)

/*
 * corestandby_2_pll1_will_be_off_check
 *
 * return:
 *		0: successful
 *		-1: PLL1 cannot be off
 */
static int corestandby_2_pll1_will_be_off_check(void)
{
	/* pll1 condition is successful */
	if (corestandby_2_pll1_condition_set() < 0)
		return -1;

	/* A3R off ? */
	if (__raw_readl(PSTR) & POWER_A3R)
		return -1;

	/* Bit7(BBPLLST) of SYSC.PSTR==0 ? */
	if (__raw_readl(PSTR) & POWER_BBPLLST)
		return -1;

	return 0;
}

/*
 * shmobile_enter_corestandby_2:
 * Executes idle PM for a CPU - CoreStandby_2 state
 * @dev: the target CPU
 * @state: the state
 * return:
 *		int: the idle duration
 */
static int shmobile_enter_corestandby_2(struct cpuidle_device *dev,
						struct cpuidle_state *state)
{
	struct timeval beforeTime, afterTime;
	int idle_time;
	long wakelock;

	int cpuid = smp_processor_id();
	unsigned int dr_WUPSFAC;
	struct clk_rate before_corestandby_clocks, corestandby_clocks;
	int clocks_ret, clocks_changed = 0;
#if DISPLAY_LOG
	printk(KERN_INFO "Standby-2 IN  %d\n", cpuid);
#endif
#ifndef CONFIG_PM_HAS_SECURE
	local_fiq_disable();
#endif
	do_gettimeofday(&beforeTime);

	/* Core Standby wakelock check */
	wakelock = has_wake_lock(WAKE_LOCK_IDLE);
	if (!wakelock) {
		/* Core Standby State Notify */
		if (!state_notify_confirm())
			state_notify(PM_STATE_NOTIFY_CORESTANDBY_2);
#ifdef CORESTANDBY_DFS
		corestandby_cpufreq();
#endif
		if ((cpuid == 0) &&
		(__raw_readl(ram0Cpu1Status) == CPUSTATUS_HOTPLUG)) {
#ifdef CONFIG_PM_HAS_SECURE
			__raw_writel(0, ram0SecHalReturnCpu0);
#endif

#ifdef PLL1_CAN_OFF
			/* PLL1 is sure to be off ? */
			if (corestandby_2_pll1_will_be_off_check() < 0)
				goto skip_clock_change;

			/* clock table */
			clocks_ret = cpg_get_freq(&before_corestandby_clocks);
			if (clocks_ret < 0)
				goto skip_clock_change;

			(void)memcpy(&corestandby_clocks,
			&before_corestandby_clocks, sizeof(struct clk_rate));

			corestandby_clocks.i_clk = DIV1_6;
			corestandby_clocks.zg_clk = DIV1_4;
			corestandby_clocks.m3_clk = DIV1_8;
			corestandby_clocks.b_clk = DIV1_12;
			corestandby_clocks.m1_clk = DIV1_6;
			corestandby_clocks.m5_clk = DIV1_8;

			corestandby_clocks.ztr_clk = DIV1_4;
			corestandby_clocks.zt_clk = DIV1_6;
			corestandby_clocks.zx_clk = DIV1_6;
			corestandby_clocks.zs_clk = DIV1_6;
			corestandby_clocks.hp_clk = DIV1_12;
			/* set clocks */
			clocks_ret =
			corestandby_pm_set_clocks(corestandby_clocks);
			if (clocks_ret < 0)
				printk(KERN_INFO "[%s]: set clocks FAILED\n", \
				__func__);

			clocks_changed = 1;

skip_clock_change:
			/* end clock table */
#endif
			start_corestandby_2(); /* CoreStandby(A2SL Off) */

#ifdef PLL1_CAN_OFF
			/* update pll1 stop condition without C4 */
			corestandby_2_pll1_condition_at_wakeup();

			/* restore clocks */
			if (clocks_changed) {
				clocks_ret =
				corestandby_pm_set_clocks(
				before_corestandby_clocks);
				if (clocks_ret < 0)
					printk(KERN_INFO
					"[%s]: restore clocks FAILED\n ", \
					__func__);
			}
#endif
			dr_WUPSFAC = __raw_readl(WUPSFAC);
#if 0	/* for debug */
			if (dr_WUPSFAC)
				printk(KERN_INFO "[%s] is wake-up 0x%08X. ", \
						__func__, dr_WUPSFAC);
#endif
#ifdef CONFIG_PM_HAS_SECURE
			if (0 != __raw_readl(ram0SecHalReturnCpu0))
				sec_hal_fail_cpu0++;
#endif
		} else {
#ifdef CONFIG_PM_HAS_SECURE
			__raw_writel(0, ram0SecHalReturnCpu1);
#endif
			start_corestandby();
#ifdef CONFIG_PM_HAS_SECURE
			if (0 != __raw_readl(ram0SecHalReturnCpu1))
				sec_hal_fail_cpu1++;
#endif
		}
	} else {
#if DISPLAY_LOG
		printk(KERN_INFO "Core-Standby %d (WAKELOCK)", cpuid);
#endif
		/* Sleep State Notify */
		if (!state_notify_confirm())
			state_notify(PM_STATE_NOTIFY_SLEEP);
		arch_idle(); /* WFI cpu_do_idle(); */
	}

	/* WakeUp State Notify */
	if (!state_notify_confirm())
		state_notify(PM_STATE_NOTIFY_WAKEUP);

	do_gettimeofday(&afterTime);

	idle_time = (afterTime.tv_sec - beforeTime.tv_sec) * 1000000
				+ (afterTime.tv_usec - beforeTime.tv_usec);

	local_irq_enable();
#ifndef CONFIG_PM_HAS_SECURE
	local_fiq_enable();
#endif

#if DISPLAY_LOG
	printk(KERN_INFO "Standby-2 OUT %d IDLE=0x%x\n", cpuid, idle_time);
#endif

	return idle_time;
}

#ifdef CONFIG_PM_DEBUG

#define SHMOBILE_MAX_STATES_DEBUG	1

static int is_enable_cpuidle = 1; /* Status of CPU's idle PM */
static DEFINE_SPINLOCK(cpuidle_debug_lock);

/*
 * control_cpuidle: Enable/Disable of CPU's idle PM
 * @is_enable: input value to control Enable/Disable
 *				0: Disable, 1: Enable
 * return:
 *		0: successful
 *		-EINVAL: Invalid argument
 */
int control_cpuidle(int is_enable)
{
	struct cpuidle_device *device;
	unsigned long flags;
	int cpu;
	int error = 0;

	spin_lock_irqsave(&cpuidle_debug_lock, flags);
	switch (is_enable) {

	case 0:
		if (!is_enable_cpuidle)
			break; /* Already disabled */
		for_each_possible_cpu(cpu) {
			device = &per_cpu(shmobile_cpuidle_device, cpu);
			/* Let the governor work/statistic correct info */
			device->state_count = SHMOBILE_MAX_STATES_DEBUG;
			/* Make sure that only WFI state is running */
			device->states[0].enter = shmobile_enter_wfi_debug;
			device->states[1].enter = shmobile_enter_wfi_debug;
			device->states[2].enter = shmobile_enter_wfi_debug;
			device->states[3].enter = shmobile_enter_wfi_debug;
		}
		is_enable_cpuidle = is_enable;
		break;

	case 1:
		if (is_enable_cpuidle)
			break; /* Already enabled */
		for_each_possible_cpu(cpu) {
			device = &per_cpu(shmobile_cpuidle_device, cpu);
			/* Restore to original CPU's idle PM */
			device->state_count = SHMOBILE_MAX_STATES;
			device->states[0].enter = shmobile_enter_wfi;
			device->states[1].enter = shmobile_enter_wfi_lowfreq;
			device->states[2].enter = shmobile_enter_corestandby;
			device->states[3].enter = shmobile_enter_corestandby_2;

		}
		is_enable_cpuidle = is_enable;
		break;

	default:
		printk(KERN_INFO "control_cpuidle: Invalid argument\n");
		error = -EINVAL; /* Invalid argument */
		break;
	}
	spin_unlock_irqrestore(&cpuidle_debug_lock, flags);

	return error;
}
EXPORT_SYMBOL(control_cpuidle);

/*
 * is_cpuidle_enable: Status of CPU's idle PM
 * return:
 *		0: Disable
 *		1: Enable
 */
int is_cpuidle_enable(void)
{
	return is_enable_cpuidle;
}
EXPORT_SYMBOL(is_cpuidle_enable);

#endif


/*
 * shmobile_init_cpuidle: Initialization of CPU's idle PM
 * return:
 *		0: successful
 *		-EIO: failed ioremap, or failed registering a CPU's idle PM
 */
static int shmobile_init_cpuidle(void)
{
	struct cpuidle_device *device;
	unsigned int smstpcr5_val;
	unsigned int mstpsr5_val;
	unsigned int pstr_val;
	unsigned long flags;
	int count;
	void __iomem *map = NULL;
	int chip_rev;
	unsigned long cpuidle_spinlock;

	/* Chip revision */
	chip_rev = shmobile_chip_rev();

	spin_lock_irqsave(&clock_lock, flags);
	/* Internal RAM0 Module Clock ON */
	if (chip_rev < ES_REV_2_0) {
		mstpsr5_val = __raw_readl(CPG_MSTPSR5);
		if (0 != (mstpsr5_val & MSTPST527)) {
			smstpcr5_val = __raw_readl(CPG_SMSTPCR5);
			__raw_writel((smstpcr5_val & (~MSTP527)), CPG_SMSTPCR5);
			do {
				mstpsr5_val = __raw_readl(CPG_MSTPSR5);
			} while (mstpsr5_val & MSTPST527);
		}
	} else {
	/* W/A of errata ES2 E0263 */
		mstpsr5_val = __raw_readl(CPG_MSTPSR5);
		if (0 != (mstpsr5_val & (MSTPST527 | MSTPST529))) {
			smstpcr5_val = __raw_readl(CPG_SMSTPCR5);
			__raw_writel((smstpcr5_val & (~(MSTP527 | MSTP529)))
							, CPG_SMSTPCR5);
			do {
				mstpsr5_val = __raw_readl(CPG_MSTPSR5);
			} while (mstpsr5_val & (MSTPST527 | MSTPST529));
		}
	}
#ifndef CONFIG_PM_HAS_SECURE
	/* Internal RAM1 Module Clock ON */
	mstpsr5_val = __raw_readl(CPG_MSTPSR5);
	if (0 != (mstpsr5_val & MSTPST528)) {
		smstpcr5_val = __raw_readl(CPG_SMSTPCR5);
		__raw_writel((smstpcr5_val & (~MSTP528)), CPG_SMSTPCR5);
		do {
			mstpsr5_val = __raw_readl(CPG_MSTPSR5);
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
	if (chip_rev < ES_REV_2_0)
		cpuidle_spinlock = 0x47BDF000;
	else
		cpuidle_spinlock = 0x44000000;

	map = ioremap_nocache(cpuidle_spinlock,
							0x00000400/*1k*/);
	if (map != NULL) {
		__raw_writel((unsigned long)map, __io(ram0SpinLockVA));
		__raw_writel(cpuidle_spinlock,
						__io(ram0SpinLockPA));
		__raw_writel((unsigned long)0x0, __io(map));
	} else {
		printk(KERN_ERR "shmobile_init_cpuidle: Failed ioremap\n");
		return -EIO;
	}

#ifndef CONFIG_PM_HAS_SECURE
	__raw_writel((unsigned long)0x0, __io(ram0CPU0SpinLock));
	__raw_writel((unsigned long)0x0, __io(ram0CPU1SpinLock));
	/* Errata(ECR0285) */
	if (chip_rev <= ES_REV_2_1)
		__raw_writel((unsigned long)0x0, __io(ram0ES_2_2_AndAfter));
	else
		__raw_writel((unsigned long)0x1, __io(ram0ES_2_2_AndAfter));
#endif
	/* Initialize internal setting */
	__raw_writel((unsigned long)CPUSTATUS_RUN, __io(ram0Cpu0Status));
	__raw_writel((unsigned long)CPUSTATUS_RUN, __io(ram0Cpu1Status));

#ifdef CONFIG_PM_HAS_SECURE

	/* Initialize sec_hal allocation */
	sec_hal_pm_coma_entry_init();

	/* Initialize internal setting */
	__raw_writel((unsigned long)(&sec_hal_pm_coma_entry),
					__io(ram0SecHalCommaEntry));
	__raw_writel((unsigned long)0x0, __io(ram0ZClockFlag));
#endif

#ifndef CONFIG_PM_SMP
	/* Temporary solution for Kernel in Secure */
#ifndef CONFIG_PM_HAS_SECURE
	__raw_writel(0, __io(SBAR2));
#endif

	__raw_writel((unsigned long)0x0, __io(APARMBAREA)); /* 4k */
#endif
#ifndef CONFIG_PM_HAS_SECURE
	/* Copy the source code internal RAM1 */
	(void)memcpy((void *)ram1ArmVector,
				(void *)&ArmVector,
				fsArmVector);

	(void)memcpy((void *)ram1PM_Spin_Lock,
		(void *)&PM_Spin_Lock,
		fsPM_Spin_Lock);

	(void)memcpy((void *)ram1PM_Spin_Unlock,
		(void *)&PM_Spin_Unlock,
		fsPM_Spin_Unlock);

	(void)memcpy((void *)ram1DisableMMU,
				(void *)&disablemmu,
				fsDisableMMU);

	(void)memcpy((void *)ram1RestoreArmRegisterPA,
				(void *)&restore_arm_register_pa,
				fsRestoreArmRegisterPA);

	(void)memcpy((void *)ram1RestoreCommonRegister,
				(void *)&restore_common_register,
				fsRestoreCommonRegister);

	(void)memcpy((void *)ram1SysPowerDown,
				(void *)&sys_powerdown,
				fsSysPowerDown);

	(void)memcpy((void *)ram1SysPowerUp,
				(void *)&sys_powerup,
				fsSysPowerUp);

	(void)memcpy((void *)ram1SetClockSystemSuspend,
				(void *)&setclock_systemsuspend,
				fsSetClockSystemSuspend);

	(void)memcpy((void *)ram1SystemSuspendCPU0PA,
				(void *)&systemsuspend_cpu0_pa,
				fsSystemSuspendCPU0PA);

	(void)memcpy((void *)ram1CoreStandbyPA,
				(void *)&corestandby_pa,
				fsCoreStandbyPA);

	(void)memcpy((void *)ram1CoreStandbyPA2,
				(void *)&corestandby_pa_2,
				fsCoreStandbyPA2);

	(void)memcpy((void *)ram1SystemSuspendCPU1PA,
				(void *)&systemsuspend_cpu1_pa,
				fsSystemSuspendCPU1PA);

	(void)memcpy((void *)ram1corestandby_down_status,
				(void *)&corestandby_down_status,
				fscorestandby_down_status);

	(void)memcpy((void *)ram1corestandby_up_status,
				(void *)&corestandby_up_status,
				fscorestandby_up_status);

	(void)memcpy((void *)ram1xtal_though,
				(void *)&xtal_though,
				fsxtal_though);
#if 0
	(void)memcpy((void *)ram1xtal_though_restore,
				(void *)&xtal_though_restore,
				fsxtal_though_restore);
#endif
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

	(void)memcpy((void *)ram0xtal_though,
				(void *)&xtal_though,
				fsxtal_though);

	(void)memcpy((void *)ram0SysPowerDown,
				(void *)&sys_powerdown,
				fsSysPowerDown);

	(void)memcpy((void *)ram0SysPowerUp,
				(void *)&sys_powerup,
				fsSysPowerUp);

	(void)memcpy((void *)ram0SetClockSystemSuspend,
				(void *)&setclock_systemsuspend,
				fsSetClockSystemSuspend);
#endif /* CONFIG_PM_HAS_SECURE */

	/* Idle function register */
	cpuidle_register_driver(&shmobile_idle_driver);
		printk(KERN_INFO "[%s] No. idle state: %d\n", \
				__func__, SHMOBILE_MAX_STATES);

	for_each_possible_cpu(count) {
		device = &per_cpu(shmobile_cpuidle_device, count);
		device->state_count = SHMOBILE_MAX_STATES;
		device->cpu = count;

		/* WFI state */
		strcpy(device->states[0].name, "WFI");
		strcpy(device->states[0].desc, "Wait for interrupt");
		device->states[0].enter = shmobile_enter_wfi;
		device->states[0].exit_latency = 1;
		device->states[0].target_residency = 1;
		device->states[0].flags = CPUIDLE_FLAG_TIME_VALID;

		/* WFI(low-freq) state */
		strcpy(device->states[1].name, "WFI(low-freq)");
		strcpy(device->states[1].desc, "Wait for interrupt(low-freq)");
		device->states[1].enter = shmobile_enter_wfi_lowfreq;
		device->states[1].exit_latency = 100;
		device->states[1].target_residency = 1;
		device->states[1].flags = CPUIDLE_FLAG_TIME_VALID;

		/* CoreStandby state */
		strcpy(device->states[2].name, "CoreStandby");
		strcpy(device->states[2].desc, "Core Standby");
		device->states[2].enter = shmobile_enter_corestandby;
		device->states[2].exit_latency = 300;
		device->states[2].target_residency = 500;
		device->states[2].flags = CPUIDLE_FLAG_TIME_VALID;

		/* CoreStandby state */
		strcpy(device->states[3].name, "CoreStandby_2");
		strcpy(device->states[3].desc, "Core Standby 2");
		device->states[3].enter = shmobile_enter_corestandby_2;
		device->states[3].exit_latency = 400;
		device->states[3].target_residency = 600;
		device->states[3].flags = CPUIDLE_FLAG_TIME_VALID;

		if (cpuidle_register_device(device)) {
			printk(KERN_ERR "shmobile_init_cpuidle: "
				"Failed registering\n");
			return -EIO;
		}
	}

	/* - set the Xtal though(PLL0 ON) mode to LPCKCR */
	__raw_writel(CPG_LPCKCR_26MHz, CPG_LPCKCR);
	/* - set PLL0 stop conditon to A2SL state by CPG.PLL0STPCR */
	__raw_writel(A2SLSTP, CPG_PLL0STPCR);
#ifdef PLL1_CAN_OFF
	/* - set PLL1 stop conditon to A2SL, A3R state by CPG.PLL1STPCR */
	__raw_writel(PLL1STPCR_DEFALT, CPG_PLL1STPCR);
#else
	__raw_writel(PLL1STPCR_DEFALT | C4STP, CPG_PLL1STPCR);
#endif
	do {
		__raw_writel(POWER_BBPLLOFF, SPDCR);
		pstr_val = __raw_readl(PSTR);
	} while (pstr_val & POWER_BBPLLST);

	/* - set Wake up factor unmask to GIC.CPU0 by SYS.WUPSMSK */
	__raw_writel((__raw_readl(WUPSMSK) &  ~(1 << 28)), WUPSMSK);

	return 0;
}
device_initcall(shmobile_init_cpuidle);

