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

#include "pm_ram0.h"
#include "pmRegisterDef.h"

#define SHMOBILE_MAX_STATES	3
#define NUM_OF_CPU			2

#define DISPLAY_LOG 0


spinlock_t clock_lock;

extern int start_corestandby(void);
extern void ArmVector(void);
extern void corestandby(void);
extern void systemsuspend(void);
extern void save_arm_register(void);
extern void restore_arm_register_pa(void);
extern void restore_arm_register_va(void);
extern void save_arm_common_register(void);
extern void restore_arm_common_register(void);
extern void save_common_register(void);
extern void restore_common_register(void);
extern void sys_powerdown(void);
extern void sys_powerup(void);
extern void setclock_systemsuspend(void);
extern void start_wfi(void);
#ifdef VMALLOC_EXPAND
extern void disablemmu(void);
extern void systemsuspend_cpu0_pa(void);
extern void systemsuspend_cpu1_pa(void);
extern void corestandby_pa(void);
#endif /* VMALLOC_EXPAND */

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
 * register_pm_state_notify: registers a notification callback function for PM state
 * @h: A callback function address to be notified PM state of AP-System domain.
 */
void register_pm_state_notify(struct pm_state_notify *h)
{
	mutex_lock(&state_notify_lock);
	list_add_tail(&h->link, &state_notify_list);
	mutex_unlock(&state_notify_lock);
}
EXPORT_SYMBOL(register_pm_state_notify);

/*
 * unregister_pm_state_notify: unregisters a notification callback function for PM state.
 * @h: A callback function address to be notified PM state of AP-System domain.
 */
void unregister_pm_state_notify(struct pm_state_notify *h)
{
	mutex_lock(&state_notify_lock);
	list_del_init(&h->link);
	mutex_unlock(&state_notify_lock);
}
EXPORT_SYMBOL(unregister_pm_state_notify);

/*
 * register_pm_state_notify_confirm: registers a callback function to be confirmed 
 * whether a notification is necessary or not when PM state is changed.
 * @h: A callback function address to be confirmed PM state of AP-System domain.
 */
void register_pm_state_notify_confirm(struct pm_state_notify_confirm *h)
{
	mutex_lock(&state_notify_confirm_lock);
	list_add_tail(&h->link, &state_notify_confirm_list);
	mutex_unlock(&state_notify_confirm_lock);
}
EXPORT_SYMBOL(register_pm_state_notify_confirm);

/*
 * unregister_pm_state_notify_confirm: unregisters a confirmation callback function of a notification.
 * @h: A callback function address to be confirmed PM state of AP-System domain.
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
 * 		0: successful
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
 * 		0: successful
 * 		Otherwise: error of confirmation callback function.
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
 * shmobile_enter_wfi: executes idle PM for a CPU - WFI state
 * @dev: the target CPU
 * @state: the state
 * return: 
 *		int: the idle duration
 */
static int shmobile_enter_wfi(struct cpuidle_device *dev, struct cpuidle_state *state)
{
	struct timeval beforeTime, afterTime;
	int idle_time;

	local_fiq_disable();

	do_gettimeofday(&beforeTime);

	/* Sleep State Notify */
	if(!state_notify_confirm())
	{
		state_notify(PM_STATE_NOTIFY_SLEEP);
	}

	arch_idle();		/* WFI cpu_do_idle(); */

	/* WakeUp State Notify */
	if(!state_notify_confirm())
	{
		state_notify(PM_STATE_NOTIFY_WAKEUP);
	}

	do_gettimeofday(&afterTime);

	idle_time = (afterTime.tv_sec - beforeTime.tv_sec) * 1000000
				+ (afterTime.tv_usec - beforeTime.tv_usec);

	local_irq_enable();
	local_fiq_enable();

	return idle_time;
}

/*
 * shmobile_enter_wfi_lowfreq: executes idle PM for a CPU - WFI(low-freq) state
 * @dev: the target CPU
 * @state: the state
 * return: 
 * 		int: the idle duration
 */
static int shmobile_enter_wfi_lowfreq(struct cpuidle_device *dev, struct cpuidle_state *state)
{
	struct timeval beforeTime, afterTime;
	int idle_time;

	local_fiq_disable();

	do_gettimeofday(&beforeTime);

	/* Sleep State Notify */
	if(!state_notify_confirm())
	{
		state_notify(PM_STATE_NOTIFY_SLEEP_LOWFREQ);
	}

	/* Transition to WFI standby with low-frequency setting	*/
	start_wfi();

	/* WakeUp State Notify */
	if(!state_notify_confirm())
	{
		state_notify(PM_STATE_NOTIFY_WAKEUP);
	}

	do_gettimeofday(&afterTime);

	idle_time = (afterTime.tv_sec - beforeTime.tv_sec) * 1000000
				+ (afterTime.tv_usec - beforeTime.tv_usec);

	local_irq_enable();
	local_fiq_enable();

	return idle_time;
}

/*
 * shmobile_enter_corestandby: executes idle PM for a CPU - CoreStandby state
 * @dev: the target CPU
 * @state: the state
 * return: 
 * 		int: the idle duration
 */
static int shmobile_enter_corestandby(struct cpuidle_device *dev, struct cpuidle_state *state)
{
	struct timeval beforeTime, afterTime;
	int idle_time;
	long wakelock;

#if DISPLAY_LOG
	int cpuid = smp_processor_id();
	printk(KERN_INFO "Standby IN  %d\n", cpuid);
#endif

	local_fiq_disable();

	do_gettimeofday(&beforeTime);

	/* Core Standby wakelock check */
	wakelock = has_wake_lock(WAKE_LOCK_IDLE);
	if (!wakelock) {
		/* Core Standby State Notify */
		if (!state_notify_confirm())
			state_notify(PM_STATE_NOTIFY_CORESTANDBY);
		corestandby_cpufreq();
		start_corestandby();								/* CoreStandby(A1SL0 or A1SL1 Off) */
	} else {
#if DISPLAY_LOG
		printk(KERN_INFO "Core-Standby %d (WAKELOCK)", cpuid);
#endif
		/* Sleep State Notify */
		if (!state_notify_confirm())
			state_notify(PM_STATE_NOTIFY_SLEEP);
		arch_idle();										/* WFI cpu_do_idle(); */
	}

	/* WakeUp State Notify */
	if(!state_notify_confirm())
	{
		state_notify(PM_STATE_NOTIFY_WAKEUP);
	}

	do_gettimeofday(&afterTime);

	idle_time = (afterTime.tv_sec - beforeTime.tv_sec) * 1000000
				+ (afterTime.tv_usec - beforeTime.tv_usec);

	local_irq_enable();
	local_fiq_enable();

#if DISPLAY_LOG
	printk(KERN_INFO "Standby OUT %d  IDLE=0x%x\n", cpuid,idle_time);
#endif

	return idle_time;
}

/*
 * shmobile_init_cpuidle: Initialization of CPU's idle PM
 * return: 
 * 		0: successful
 * 		-EIO: failed ioremap, or failed registering a CPU's idle PM
 */
static int shmobile_init_cpuidle(void)
{
	struct cpuidle_device *device;
	unsigned int smstpcr5_val;
	unsigned int mstpsr5_val;
	unsigned long flags;
	unsigned int count;
	void __iomem *map = NULL;

	/* Internal RAM0 Module Clock ON */
	spin_lock_irqsave(&clock_lock, flags);
	mstpsr5_val = __raw_readl(CPG_MSTPSR5);
	if (0 != (mstpsr5_val & MSTPST527)) {
	
		smstpcr5_val = __raw_readl(CPG_SMSTPCR5); 
		__raw_writel((smstpcr5_val & (~MSTP527)), CPG_SMSTPCR5);

		do {
			mstpsr5_val = __raw_readl(CPG_MSTPSR5);
		} while (mstpsr5_val & MSTPST527);
	}
	spin_unlock_irqrestore(&clock_lock, flags);
	/* Allocate CPU0 back up area */
	cpu0BackupArea = kmalloc(saveCpuRegisterAreaSize, GFP_KERNEL);
	if (cpu0BackupArea == NULL) {
		printk(KERN_ERR "shmobile_init_cpuidle: Failed Allocate CPU0 back up area\n");
	} else {
		__raw_writel((unsigned int)cpu0BackupArea, ram0Cpu0RegisterArea);
	}

	/* Allocate CPU1 back up area */
	cpu1BackupArea = kmalloc(saveCpuRegisterAreaSize, GFP_KERNEL);
	if (cpu1BackupArea == NULL) {
		printk(KERN_ERR "shmobile_init_cpuidle: Failed Allocate CPU1 back up area\n");
	} else {
		__raw_writel((unsigned int)cpu1BackupArea, ram0Cpu1RegisterArea);
	}

	/* Initialize SpinLock setting */
	map = ioremap_nocache((unsigned long)CPUIDLE_SPINLOCK, 0x00000400/*1k*/);
	if (map != NULL) {
		__raw_writel((unsigned long)map, __io(ram0SpinLockVA));
		__raw_writel((unsigned long)CPUIDLE_SPINLOCK, __io(ram0SpinLockPA));
		__raw_writel((unsigned long)0x0, __io(map));
	} else {
		printk(KERN_ERR "shmobile_init_cpuidle: Failed ioremap\n");
		return -EIO;
	}

	/* Initialize internal setting */
	__raw_writel((unsigned long)CPUSTATUS_RUN, __io(ram0Cpu0Status));
	__raw_writel((unsigned long)CPUSTATUS_RUN, __io(ram0Cpu1Status));
	__raw_writel((unsigned long)0x0, __io(ram0CpuClock));

	/* Copy the source code internal RAM0 */
	(void)memcpy((void *)ram0ArmVector,						(void *)&ArmVector,						fsArmVector					);
	(void)memcpy((void *)ram0CoreStandby,					(void *)&corestandby,					fsCoreStandby				);
	(void)memcpy((void *)ram0SystemSuspend,					(void *)&systemsuspend,					fsSystemSuspend				);
	(void)memcpy((void *)ram0SaveArmRegister,				(void *)&save_arm_register,				fsSaveArmRegister			);
	(void)memcpy((void *)ram0RestoreArmRegisterPA,			(void *)&restore_arm_register_pa,		fsRestoreArmRegisterPA		);
	(void)memcpy((void *)ram0RestoreArmRegisterVA,			(void *)&restore_arm_register_va,		fsRestoreArmRegisterVA		);
	(void)memcpy((void *)ram0SaveArmCommonRegister,			(void *)&save_arm_common_register,		fsSaveArmCommonRegister		);
	(void)memcpy((void *)ram0RestoreArmCommonRegister,		(void *)&restore_arm_common_register,	fsRestoreArmCommonRegister	);
	(void)memcpy((void *)ram0SaveCommonRegister,			(void *)&save_common_register,			fsSaveCommonRegister		);
	(void)memcpy((void *)ram0RestoreCommonRegister,			(void *)&restore_common_register,		fsRestoreCommonRegister		);
	(void)memcpy((void *)ram0SysPowerDown,					(void *)&sys_powerdown,					fsSysPowerDown				);
	(void)memcpy((void *)ram0SysPowerUp,					(void *)&sys_powerup,					fsSysPowerUp				);
	(void)memcpy((void *)ram0SetClockSystemSuspend,			(void *)&setclock_systemsuspend,		fsSetClockSystemSuspend		);

#ifdef VMALLOC_EXPAND
	(void)memcpy((void *)ram0SystemSuspendCPU0PA,			(void *)&systemsuspend_cpu0_pa,			fsSystemSuspendCPU0PA		);
	(void)memcpy((void *)ram0CoreStandbyPA		,			(void *)&corestandby_pa,				fsCoreStandbyPA				);
	(void)memcpy((void *)ram0DisableMMU,					(void *)&disablemmu,					fsDisableMMU				);
	(void)memcpy((void *)ram0SystemSuspendCPU1PA,			(void *)&systemsuspend_cpu1_pa,			fsSystemSuspendCPU1PA		);
#endif /* VMALLOC_EXPAND */

	/* Idle function register */
	cpuidle_register_driver(&shmobile_idle_driver);

	for (count = 0; count < NUM_OF_CPU; count++) {
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
		device->states[2].enter = shmobile_enter_wfi_lowfreq;
		device->states[2].exit_latency = 300;
		device->states[2].target_residency = 500;
		device->states[2].flags = CPUIDLE_FLAG_TIME_VALID;

		if (cpuidle_register_device(device)) {
			printk(KERN_ERR "shmobile_init_cpuidle: Failed registering\n");
			return -EIO;
		}
	}

	return 0;
}
device_initcall(shmobile_init_cpuidle);

