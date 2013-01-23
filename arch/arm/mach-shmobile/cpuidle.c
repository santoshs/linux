/*
 * CPUIdle support code for SH-Mobile ARM
 * arch/arm/mach-shmobile/cpuidle.c
 *
 * Copyright (C) 2011 Magnus Damm
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

#include <linux/pm.h>
#include <linux/cpuidle.h>
#include <linux/suspend.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/io.h>
#include <mach/pm.h>
#include <linux/slab.h>
#include <mach/system.h>
#include <linux/wakelock.h>
#include <linux/spinlock_types.h>
#include <linux/cpu.h>

#ifndef CONFIG_PM_HAS_SECURE
#include "pm_ram0.h"
#else
#include "pm_ram0_tz.h"
#endif
#include "pmRegisterDef.h"



#define DISPLAY_LOG 0

#ifdef CONFIG_PM_HAS_SECURE
static int sec_hal_fail_cpu0;
module_param(sec_hal_fail_cpu0, int, S_IRUGO | S_IWUSR | S_IWGRP);

static int sec_hal_fail_cpu1;
module_param(sec_hal_fail_cpu1, int, S_IRUGO | S_IWUSR | S_IWGRP);
#endif


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
 * ********************************************************************
 *     Drivers interface end.
 * ********************************************************************
 */

/* states */
enum state {
	SHMOBILE_STATE_1 = 0,
	SHMOBILE_STATE_2 = 1,
	SHMOBILE_STATE_3 = 2,
	SHMOBILE_STATE_4 = 3,
	SHMOBILE_MAX_STATES = 4
};

static int shmobile_enter_wfi(struct cpuidle_device *dev,
	struct cpuidle_driver *drv, int index);
static int shmobile_enter_wfi_lowfreq(struct cpuidle_device *dev,
	struct cpuidle_driver *drv, int index);
static int shmobile_enter_wfi_lowfreq2(struct cpuidle_device *dev,
	struct cpuidle_driver *drv, int index);
static int shmobile_enter_corestandby(struct cpuidle_device *dev,
	struct cpuidle_driver *drv, int index);

static DEFINE_PER_CPU(struct cpuidle_device, shmobile_cpuidle_dev);

struct cpuidle_driver shmobile_cpuidle_driver = {
	.name = "shmobile_cpuidle",
	.owner = THIS_MODULE,
	.state_count = SHMOBILE_MAX_STATES,
	.states = {
		[SHMOBILE_STATE_1] = {
			.enter			= shmobile_enter_wfi,
			.exit_latency		= 1,
			.target_residency	= 1,
			.flags			= CPUIDLE_FLAG_TIME_VALID,
			.name			= "WFI",
			.desc			= "Wait for interrupt",
		},
		[SHMOBILE_STATE_2] = {
			.enter			= shmobile_enter_wfi_lowfreq,
			.exit_latency		= 100,
			.target_residency	= 1,
			.flags		= CPUIDLE_FLAG_TIME_VALID,
			.name		= "WFI(low-freq)",
			.desc = "Wait for interrupt(lowfreq)",
		},
		[SHMOBILE_STATE_3] = {
			.enter			= shmobile_enter_wfi_lowfreq2,
			.exit_latency		= 200,
			.target_residency	= 100,
			.flags		= CPUIDLE_FLAG_TIME_VALID,
			.name		= "WFI(low-freq2)",
			.desc = "Wait for interrupt(low-freq2)",
		},
		[SHMOBILE_STATE_4] = {
			.enter			= shmobile_enter_corestandby,
			.exit_latency		= 300,
			.target_residency	= 500,
#ifdef CONFIG_ARCH_NEEDS_CPU_IDLE_COUPLED
			.flags		= CPUIDLE_FLAG_TIME_VALID |
					CPUIDLE_FLAG_COUPLED,
#else
			.flags			= CPUIDLE_FLAG_TIME_VALID,
#endif
			.name			= "CoreStandby",
			.desc			= "Core Standby",
		},
	},
};


/*
 * shmobile_enter_wfi: executes idle PM for a CPU - WFI state
 * @dev: cpuidle device for this cpu
 * @drv: cpuidle driver for this cpu
 * @index: index into drv->states of the state to enter
 * return:
 *		int: index into drv->states of the state to exit
 */
static int shmobile_enter_wfi(struct cpuidle_device *dev,
	struct cpuidle_driver *drv, int index)
{
	ktime_t time_start, time_end;
	s64 diff;

#if DISPLAY_LOG
	int cpuid = smp_processor_id();
	printk(KERN_INFO "WFI IN %d\n", cpuid);
#endif
#ifndef CONFIG_PM_HAS_SECURE
	local_fiq_disable();
#endif
	time_start = ktime_get();

	/* Sleep State Notify */
	if (!state_notify_confirm())
		state_notify(PM_STATE_NOTIFY_SLEEP);

	arch_idle();		/* WFI cpu_do_idle(); */

	/* WakeUp State Notify */
	if (!state_notify_confirm())
		state_notify(PM_STATE_NOTIFY_WAKEUP);

	time_end = ktime_get();

	local_irq_enable();
#ifndef CONFIG_PM_HAS_SECURE
	local_fiq_enable();
#endif
	diff = ktime_to_us(ktime_sub(time_end, time_start));
	if (diff > INT_MAX)
		diff = INT_MAX;

	dev->last_residency = (int) diff;

#if DISPLAY_LOG
	printk(KERN_INFO "WFI OUT %d IDLE=%d\n", cpuid, (int) diff);
#endif

	return index;

}

/*
 * shmobile_enter_wfi_lowfreq: executes idle PM for a CPU - WFI(low-freq) state
 * @dev: cpuidle device for this cpu
 * @drv: cpuidle driver for this cpu
 * @index: index into drv->states of the state to enter
 * return:
 *		int: index into drv->states of the state to exit
 */
static int shmobile_enter_wfi_lowfreq(struct cpuidle_device *dev,
	struct cpuidle_driver *drv, int index)
{
	ktime_t time_start, time_end;
	s64 diff;

#if DISPLAY_LOG
	int cpuid = smp_processor_id();
	printk(KERN_INFO "WFI-low IN %d\n", cpuid);
#endif

#ifndef CONFIG_PM_HAS_SECURE
	local_fiq_disable();
#endif

	time_start = ktime_get();

	/* Sleep State Notify */
	if (!state_notify_confirm())
		state_notify(PM_STATE_NOTIFY_SLEEP_LOWFREQ);

	/* Transition to WFI standby with low-frequency setting	*/
#ifndef CONFIG_PM_HAS_SECURE
	start_wfi();
#else
	arch_idle();		/* WFI cpu_do_idle(); */
#endif

	/* WakeUp State Notify */
	if (!state_notify_confirm())
		state_notify(PM_STATE_NOTIFY_WAKEUP);

	time_end = ktime_get();

	local_irq_enable();
#ifndef CONFIG_PM_HAS_SECURE
	local_fiq_enable();
#endif
	diff = ktime_to_us(ktime_sub(time_end, time_start));
	if (diff > INT_MAX)
		diff = INT_MAX;

	dev->last_residency = (int) diff;

#if DISPLAY_LOG
	printk(KERN_INFO "WFI-low OUT %d IDLE=%d\n", cpuid, (int) diff);
#endif

	return index;
}

/*
 * shmobile_enter_wfi_lowfreq2: executes idle PM for a CPU-WFI(low-freq2) state
 * @dev: cpuidle device for this cpu
 * @drv: cpuidle driver for this cpu
 * @index: index into drv->states of the state to enter
 * return:
 *		int: index into drv->states of the state to exit
 */
static int shmobile_enter_wfi_lowfreq2(struct cpuidle_device *dev,
	struct cpuidle_driver *drv, int index)
{
	ktime_t time_start, time_end;
	s64 diff;

#if DISPLAY_LOG
	int cpuid = smp_processor_id();
	printk(KERN_INFO "WFI-low2 IN %d\n", cpuid);
#endif
#ifndef CONFIG_PM_HAS_SECURE
	local_fiq_disable();
#endif
	time_start = ktime_get();

	/* Sleep State Notify */
	if (!state_notify_confirm())
		state_notify(PM_STATE_NOTIFY_SLEEP_LOWFREQ2);

#ifndef CONFIG_PM_HAS_SECURE
	/* Transition to WFI standby with low-frequency-2 setting	*/
	start_wfi2();
#else
	arch_idle();		/* WFI cpu_do_idle(); */
#endif

	/* WakeUp State Notify */
	if (!state_notify_confirm())
		state_notify(PM_STATE_NOTIFY_WAKEUP);

	time_end = ktime_get();

	local_irq_enable();
#ifndef CONFIG_PM_HAS_SECURE
	local_fiq_enable();
#endif
	diff = ktime_to_us(ktime_sub(time_end, time_start));
	if (diff > INT_MAX)
		diff = INT_MAX;

	dev->last_residency = (int) diff;

#if DISPLAY_LOG
	printk(KERN_INFO "WFI-low2 OUT %d IDLE=%d\n", cpuid, (int) diff);
#endif

	return index;
}

/*
 * shmobile_enter_corestandby: executes idle PM for a CPU - Corestandby state
 * @dev: cpuidle device for this cpu
 * @drv: cpuidle driver for this cpu
 * @index: index into drv->states of the state to enter
 * return:
 *		int: index into drv->states of the state to exit
 */
static int shmobile_enter_corestandby(struct cpuidle_device *dev,
	struct cpuidle_driver *drv, int index)
{
	ktime_t time_start, time_end;
	s64 diff;
	long wakelock;
	int cpuid = smp_processor_id();
#if DISPLAY_LOG
	printk(KERN_INFO "Standby IN  %d\n", cpuid);
#endif
#ifndef CONFIG_PM_HAS_SECURE
	local_fiq_disable();
#endif

	time_start = ktime_get();

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
#endif
		start_corestandby(); /* CoreStandby(A1SL0 or A1SL1 Off) */
#ifdef CONFIG_PM_HAS_SECURE
		if (cpuid == 0) {
			if (0 != __raw_readl(ram0SecHalReturnCpu0))
				sec_hal_fail_cpu0++;
		} else {
			if (0 != __raw_readl(ram0SecHalReturnCpu1))
				sec_hal_fail_cpu1++;
		}
#endif
	} else {
#if DISPLAY_LOG
		printk(KERN_INFO "Core-Standby %d (WAKELOCK)\n", cpuid);
#endif
		/* Sleep State Notify */
		if (!state_notify_confirm())
			state_notify(PM_STATE_NOTIFY_SLEEP);
		cpu_do_idle(); /* WFI cpu_do_idle(); */
	}

	/* WakeUp State Notify */
	if (!state_notify_confirm())
		state_notify(PM_STATE_NOTIFY_WAKEUP);

	time_end = ktime_get();

	local_irq_enable();
#ifndef CONFIG_PM_HAS_SECURE
	local_fiq_enable();
#endif
	diff = ktime_to_us(ktime_sub(time_end, time_start));
	if (diff > INT_MAX)
		diff = INT_MAX;

	dev->last_residency = (int) diff;

#if DISPLAY_LOG
	printk(KERN_INFO "Standby OUT %d IDLE=%d\n", cpuid, (int) diff);
#endif

	return index;
}


#ifdef CONFIG_PM_DEBUG

static int is_enable_cpuidle = 1; /* Status of CPU's idle PM */
spinlock_t cpuidle_debug_lock;

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
	unsigned long flags;
	unsigned int states_disable;
	int error = 0;
	struct cpuidle_driver *drv = &shmobile_cpuidle_driver;

	spin_lock_irqsave(&cpuidle_debug_lock, flags);
	switch (is_enable) {

	case 0:
		if (!is_enable_cpuidle)
			break; /* Already disabled */
		/* Only WFI state is running */
		for (states_disable = SHMOBILE_STATE_2;
			states_disable < SHMOBILE_MAX_STATES; states_disable++)
			drv->states[states_disable].disable = 1;

		is_enable_cpuidle = is_enable;
		break;

	case 1:
		if (is_enable_cpuidle)
			break; /* Already enabled */
		/* Restore to original CPU's idle PM */
		for (states_disable = SHMOBILE_STATE_2;
			states_disable < SHMOBILE_MAX_STATES; states_disable++)
			drv->states[states_disable].disable = 0;

		is_enable_cpuidle = is_enable;
		break;

	default:
		printk(KERN_INFO "%s: Invalid argument\n", __func__);
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

spinlock_t clock_lock;

/*
 * cpuidle_coupled_cpu_notify - notifier called during hotplug transitions
 * @nfb: notifier block
 * @action: hotplug transition
 * @hcpu: target cpu number
 *
 * Called when a cpu is brought on or offline using hotplug.
 * updates the CPU's status appropriately
 */
static int __cpuinit
cpu_callback(struct notifier_block *nfb, unsigned long action, void *hcpu)
{
	unsigned long flags;
	int cpu = (unsigned long)hcpu;

	switch (action) {

	case CPU_ONLINE:
	case CPU_ONLINE_FROZEN:
		spin_lock_irqsave(&clock_lock, flags);
		if (cpu)
			__raw_writel((unsigned long)CPUSTATUS_RUN
						, IOMEM(ram0Cpu1Status));
		else
			__raw_writel((unsigned long)CPUSTATUS_RUN
						, IOMEM(ram0Cpu0Status));
		spin_unlock_irqrestore(&clock_lock, flags);
		break;

	case CPU_DEAD:
	case CPU_DEAD_FROZEN:
		spin_lock_irqsave(&clock_lock, flags);
		if (cpu)
			__raw_writel((unsigned long)CPUSTATUS_SHUTDOWN
						, IOMEM(ram0Cpu1Status));
		else
			__raw_writel((unsigned long)CPUSTATUS_SHUTDOWN
						, IOMEM(ram0Cpu0Status));
		spin_unlock_irqrestore(&clock_lock, flags);
		break;

	default:
		break;
	}

	return NOTIFY_OK;
}

static struct notifier_block __cpuinitdata cpu_nfb = {
	.notifier_call = cpu_callback
};


unsigned int *cpu0BackupArea;
unsigned int *cpu1BackupArea;
struct copy_source_code {
		void *dest;
		const void *source;
		size_t size;
	} internal_ram[] = {
#ifndef CONFIG_PM_HAS_SECURE
		[0] = {
			.dest = (void *)ram1ArmVector,
			.source = (const void *)&ArmVector,
			.size = (size_t)fsArmVector,
		},
		[1] = {
			.dest = (void *)ram1DisableMMU,
			.source = (const void *)&disablemmu,
			.size = (size_t)fsDisableMMU,
		},
		[2] = {
			.dest = (void *)ram1RestoreArmRegisterPA,
			.source = (const void *)&restore_arm_register_pa,
			.size = (size_t)fsRestoreArmRegisterPA,
		},
		[3] = {
			.dest = (void *)ram1RestoreCommonRegister,
			.source = (const void *)&restore_common_register,
			.size = (size_t)fsRestoreCommonRegister,
		},
		[4] = {
			.dest = (void *)ram1SysPowerDown,
			.source = (const void *)&sys_powerdown,
			.size = (size_t)fsSysPowerDown,
		},
		[5] = {
			.dest = (void *)ram1SysPowerUp,
			.source = (const void *)&sys_powerup,
			.size = (size_t)fsSysPowerUp,
		},
		[6] = {
			.dest = (void *)ram1SetClockSystemSuspend,
			.source = (const void *)&setclock_systemsuspend,
			.size = (size_t)fsSetClockSystemSuspend,
		},
		[7] = {
			.dest = (void *)ram1SystemSuspendCPU0PA,
			.source = (const void *)&systemsuspend_cpu0_pa,
			.size = (size_t)fsSystemSuspendCPU0PA,
		},
		[8] = {
			.dest = (void *)ram1CoreStandbyPA,
			.source = (const void *)&corestandby_pa,
			.size = (size_t)fsCoreStandbyPA,
		},
		[9] = {
			.dest = (void *)ram1SystemSuspendCPU1PA,
			.source = (const void *)&systemsuspend_cpu1_pa,
			.size = (size_t)fsSystemSuspendCPU1PA,
		},
#else
		[0] = {
			.dest = (void *)ram0ArmVector,
			.source = (const void *)&ArmVector,
			.size = (size_t)fsArmVector,
		},
		[1] = {
			.dest = (void *)ram0CoreStandby,
			.source = (const void *)&corestandby,
			.size = (size_t)fsCoreStandby,
		},
		[2] = {
			.dest = (void *)ram0SystemSuspend,
			.source = (const void *)&systemsuspend,
			.size = (size_t)fsSystemSuspend,
		},
		[3] = {
			.dest = (void *)ram0SaveArmRegister,
			.source = (const void *)&save_arm_register,
			.size = (size_t)fsSaveArmRegister,
		},
		[4] = {
			.dest = (void *)ram0RestoreArmRegisterPA,
			.source = (const void *)&restore_arm_register_pa,
			.size = (size_t)fsRestoreArmRegisterPA,
		},
		[5] = {
			.dest = (void *)ram0RestoreArmRegisterVA,
			.source = (const void *)&restore_arm_register_va,
			.size = (size_t)fsRestoreArmRegisterVA,
		},
		[6] = {
			.dest = (void *)ram0SaveArmCommonRegister,
			.source = (const void *)&save_arm_common_register,
			.size = (size_t)fsSaveArmCommonRegister,
		},
		[7] = {
			.dest = (void *)ram0RestoreArmCommonRegister,
			.source = (const void *)&restore_arm_common_register,
			.size = (size_t)fsRestoreArmCommonRegister,
		},
		[8] = {
			.dest = (void *)ram0SysPowerDown,
			.source = (const void *)&sys_powerdown,
			.size = (size_t)fsSysPowerDown,
		},
		[9] = {
			.dest = (void *)ram0SysPowerUp,
			.source = (const void *)&sys_powerup,
			.size = (size_t)fsSysPowerUp,
		},
		[10] = {
			.dest = (void *)ram0SetClockSystemSuspend,
			.source = (const void *)&setclock_systemsuspend,
			.size = (size_t)fsSetClockSystemSuspend,
		},
#endif /* CONFIG_PM_HAS_SECURE */
	};

#define	copy_source_from_index(index) \
			((void *)memcpy(internal_ram[index].dest, \
			internal_ram[index].source, \
			internal_ram[index].size))

/*
 * shmobile_init_cpuidle: Initialization of CPU's idle PM
 * return:
 *		0: successful
 *		-EIO: failed ioremap, or failed registering a CPU's idle PM
 */
static int shmobile_init_cpuidle(void)
{
	struct cpuidle_device *dev;
	struct cpuidle_driver *drv = &shmobile_cpuidle_driver;
	unsigned int cpu;
	unsigned long flags;
	int ret;
	unsigned int smstpcr5_val;
	unsigned int mstpsr5_val;
	int chip_rev;
	int index;
	unsigned long cpuidle_spinlock;
	void __iomem *map = NULL;

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
	/* Internal RAM1 Module Clock ON */
	mstpsr5_val = __raw_readl(CPG_MSTPSR5);
	if (0 != (mstpsr5_val & MSTPST528)) {
		smstpcr5_val = __raw_readl(CPG_SMSTPCR5);
		__raw_writel((smstpcr5_val & (~MSTP528)), CPG_SMSTPCR5);
		do {
			mstpsr5_val = __raw_readl(CPG_MSTPSR5);
		} while (mstpsr5_val & MSTPST528);
	}
	spin_unlock_irqrestore(&clock_lock, flags);
	/* Allocate CPU0 back up area */
	cpu0BackupArea = kmalloc(saveCpuRegisterAreaSize, GFP_KERNEL);
	if (cpu0BackupArea == NULL)
		printk(KERN_ERR "%s: Failed Allocate CPU0 back up area\n",
				__func__);
	else
		__raw_writel((unsigned int)cpu0BackupArea,
						IOMEM(ram0Cpu0RegisterArea));

	/* Allocate CPU1 back up area */
	cpu1BackupArea = kmalloc(saveCpuRegisterAreaSize, GFP_KERNEL);
	if (cpu1BackupArea == NULL)
		printk(KERN_ERR "%s: Failed Allocate CPU1 back up area\n",
				__func__);
	else
		__raw_writel((unsigned int)cpu1BackupArea,
						IOMEM(ram0Cpu1RegisterArea));

	/* Initialize SpinLock setting */
	if (chip_rev < ES_REV_2_0)
		cpuidle_spinlock = 0x47BDF000;
	else
		cpuidle_spinlock = 0x44000000;

	map = ioremap_nocache(cpuidle_spinlock,
							0x00000400/*1k*/);
	if (map != NULL) {
		__raw_writel((unsigned long)map,
						IOMEM(ram0SpinLockVA));
		__raw_writel(cpuidle_spinlock,
						IOMEM(ram0SpinLockPA));
		__raw_writel((unsigned long)0x0, map);
	} else {
		printk(KERN_ERR "%s: Failed ioremap\n", __func__);
		return -EIO;
	}
#ifndef CONFIG_PM_HAS_SECURE
	__raw_writel((unsigned long)0x0, IOMEM(ram0CPU0SpinLock));
	__raw_writel((unsigned long)0x0, IOMEM(ram0CPU1SpinLock));
	/* Errata(ECR0285) */
	if (chip_rev <= ES_REV_2_1)
		__raw_writel((unsigned long)0x0,
				IOMEM(ram0ES_2_2_AndAfter));
	else
		__raw_writel((unsigned long)0x1,
				IOMEM(ram0ES_2_2_AndAfter));
#endif
	/* Initialize internal setting */
	__raw_writel((unsigned long)CPUSTATUS_RUN,
					IOMEM(ram0Cpu0Status));
	__raw_writel((unsigned long)CPUSTATUS_RUN,
					IOMEM(ram0Cpu1Status));
	__raw_writel((unsigned long)0x0,
					IOMEM(ram0CpuClock));


#ifdef CONFIG_PM_HAS_SECURE
	/* Initialize sec_hal allocation */
	sec_hal_pm_coma_entry_init();
	__raw_writel((unsigned long)(&sec_hal_pm_coma_entry),
					IOMEM(ram0SecHalCommaEntry));
	__raw_writel((unsigned long)0x0, IOMEM(ram0ZClockFlag));
#endif

#ifndef CONFIG_PM_SMP
	/* Temporary solution for Kernel in Secure */
#ifdef CONFIG_PM_HAS_SECURE
	__raw_writel(0, IOMEM(SBAR2));
#endif
	__raw_writel((unsigned long)0x0,
			IOMEM(APARMBAREA)); /* 4k */
#endif

	/* Copy the source code internal RAM1 */
	for (index = 0; index < ARRAY_SIZE(internal_ram); index++)
		copy_source_from_index(index);

	ret = cpuidle_register_driver(drv);
	if (ret) {
		printk(KERN_ERR "%s: driver registration failed\n",
				__func__);
		return ret;
	}

	for_each_possible_cpu(cpu) {
		dev = &per_cpu(shmobile_cpuidle_dev, cpu);
		dev->cpu = cpu;

		dev->state_count = drv->state_count;
#ifdef CONFIG_ARCH_NEEDS_CPU_IDLE_COUPLED
		dev->safe_state_index = SHMOBILE_STATE_1;
		cpumask_set_cpu(cpu, &dev->coupled_cpus);
#endif
		ret = cpuidle_register_device(dev);
		if (ret) {
			printk(KERN_ERR "%s:[CPU%u] device registration failed\n",
				__func__, cpu);
			return ret;
		}
	}

	/* Register hotplug notifier. */
	if (0 != register_cpu_notifier(&cpu_nfb))
		printk(KERN_ERR "%s: Failed registering CPUHotplug\n",
				__func__);

	return 0;
}
device_initcall(shmobile_init_cpuidle);

