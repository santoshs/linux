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
#else /*CONFIG_PM_HAS_SECURE*/
#include "pm_ram0_tz.h"
#endif /*CONFIG_PM_HAS_SECURE*/
#include "pmRegisterDef.h"

#define ZB3_CLK_CORESTANDBY2	(130000)

#define DISPLAY_LOG 0

#if DISPLAY_LOG
#define idle_log(fmt, ...) printk(KERN_INFO "[%s] line[%d] cpu[%d] " fmt,\
		__func__, __LINE__, smp_processor_id(), ##__VA_ARGS__)
#else
#define idle_log(fmt, ...)
#endif

#ifndef CONFIG_PM_HAS_SECURE
#define FIQ_ENABLE()	local_fiq_enable()
#define FIQ_DISABLE()	local_fiq_disable()
#else
#define FIQ_ENABLE()
#define FIQ_DISABLE()
#endif

#ifdef CONFIG_PM_HAS_SECURE
static int sec_hal_fail_cpu0;
module_param(sec_hal_fail_cpu0, int, S_IRUGO | S_IWUSR | S_IWGRP);

static int sec_hal_fail_cpu1;
module_param(sec_hal_fail_cpu1, int, S_IRUGO | S_IWUSR | S_IWGRP);
#endif /*CONFIG_PM_HAS_SECURE*/

static int get_sem_fail_ebusy;
module_param(get_sem_fail_ebusy, int, S_IRUGO | S_IWUSR | S_IWGRP);
static int get_sem_fail_einval;
module_param(get_sem_fail_einval, int, S_IRUGO | S_IWUSR | S_IWGRP);

static DEFINE_PER_CPU(struct cpuidle_device, shmobile_cpuidle_device);

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

static int shmobile_enter_wfi_debug(struct cpuidle_device *dev,
	struct cpuidle_driver *drv, int index);
static int shmobile_enter_wfi(struct cpuidle_device *dev,
	struct cpuidle_driver *drv, int index);
static int shmobile_enter_wfi_lowfreq(struct cpuidle_device *dev,
	struct cpuidle_driver *drv, int index);
static int shmobile_enter_corestandby(struct cpuidle_device *dev,
	struct cpuidle_driver *drv, int index);
static int shmobile_enter_corestandby_2(struct cpuidle_device *dev,
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
			.enter			= shmobile_enter_wfi,
			/*.enter		= shmobile_enter_wfi_lowfreq,*/
			.exit_latency		= 100,
			.target_residency	= 1,
			.flags		= CPUIDLE_FLAG_TIME_VALID,
			.name		= "WFI(low-freq)",
			.desc = "Wait for interrupt(lowfreq)",
		},
		[SHMOBILE_STATE_3] = {
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
		[SHMOBILE_STATE_4] = {
			.enter			= shmobile_enter_corestandby_2,
			.exit_latency		= 400,
			.target_residency	= 600,
			.flags		= CPUIDLE_FLAG_TIME_VALID,
			.name		= "CoreStandby_2",
			.desc = "Core Standby 2",
		},
	},
};

/*
 * shmobile_enter_wfi_debug: executes idle PM for a CPU - WFI state
 * @dev: cpuidle device for this cpu
 * @drv: cpuidle driver for this cpu
 * @index: index into drv->states of the state to enter
 * return:
 *		int: index into drv->states of the state to exit
 */
static int shmobile_enter_wfi_debug(struct cpuidle_device *dev,
	struct cpuidle_driver *drv, int index)
{
	ktime_t time_start, time_end;
	s64 diff;

	FIQ_DISABLE();

	time_start = ktime_get();

	/* Sleep State Notify */
	if (!state_notify_confirm())
		state_notify(PM_STATE_NOTIFY_SLEEP);

	cpu_do_idle();		/* WFI cpu_do_idle(); */

	/* WakeUp State Notify */
	if (!state_notify_confirm())
		state_notify(PM_STATE_NOTIFY_WAKEUP);

	time_end = ktime_get();

	local_irq_enable();
	FIQ_ENABLE();

	diff = ktime_to_us(ktime_sub(time_end, time_start));
	if (diff > INT_MAX)
		diff = INT_MAX;

	dev->last_residency = (int) diff;

	return index;

}

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

	FIQ_DISABLE();

	time_start = ktime_get();

	/* Sleep State Notify */
	if (!state_notify_confirm())
		state_notify(PM_STATE_NOTIFY_SLEEP);

	/* Transition to WFI setting	*/
	start_wfi();

	/* WakeUp State Notify */
	if (!state_notify_confirm())
		state_notify(PM_STATE_NOTIFY_WAKEUP);

	time_end = ktime_get();

	local_irq_enable();
	FIQ_ENABLE();

	diff = ktime_to_us(ktime_sub(time_end, time_start));
	if (diff > INT_MAX)
		diff = INT_MAX;

	dev->last_residency = (int) diff;

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

	FIQ_DISABLE();

	time_start = ktime_get();

	/* Sleep State Notify */
	if (!state_notify_confirm())
		state_notify(PM_STATE_NOTIFY_SLEEP_LOWFREQ);

	/* Transition to WFI standby with low-frequency setting	*/
	start_wfi2();

	/* WakeUp State Notify */
	if (!state_notify_confirm())
		state_notify(PM_STATE_NOTIFY_WAKEUP);

	time_end = ktime_get();

	local_irq_enable();
	FIQ_ENABLE();

	diff = ktime_to_us(ktime_sub(time_end, time_start));
	if (diff > INT_MAX)
		diff = INT_MAX;

	dev->last_residency = (int) diff;

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

	idle_log(">>>IN\n");

	FIQ_DISABLE();

	time_start = ktime_get();

	/* Core Standby wakelock check */
	wakelock = has_wake_lock(WAKE_LOCK_IDLE);
	if (!wakelock) {
		/* Core Standby State Notify */
		if (!state_notify_confirm())
			state_notify(PM_STATE_NOTIFY_CORESTANDBY);

		start_corestandby(); /* CoreStandby(A1SL0 or A1SL1 Off) */

	} else {

		idle_log(">>>IN (WAKELOCK)\n");

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
	FIQ_ENABLE();

	diff = ktime_to_us(ktime_sub(time_end, time_start));
	if (diff > INT_MAX)
		diff = INT_MAX;

	dev->last_residency = (int) diff;

	idle_log("<<<OUT idle_time[0x%x]\n", idle_time);

	return index;
}


/*
 * pll1_condition_set
 *
 * return:
 *		0: successful
 *		-1: C4 is set to PLL1 stop condition.
 *		-2: set C4 failed.
 */
static int pll1_condition_set(void)
{
	int ret = 0;
	if ((__raw_readl(MSTPSR1) & MSTPST1_PLL1) != MSTPST1_PLL1)
		goto set_pll1_c4;

	if ((__raw_readl(MSTPSR2) & MSTPST2_PLL1) != MSTPST2_PLL1)
		goto set_pll1_c4;

	if ((__raw_readl(MSTPSR3) & MSTPST3_PLL1) != MSTPST3_PLL1)
		goto set_pll1_c4;

	if ((__raw_readl(MSTPSR4) & MSTPST4_PLL1) == MSTPST4_PLL1)
		goto set_pll1_c4_skip;

set_pll1_c4:
	ret = hwspin_trylock_nospin(pll_1_sem);
	if (ret == 0) { /* Get sem OK */
		__raw_writel(__raw_readl(PLL1STPCR) | C4STP, PLL1STPCR);
		hwspin_unlock_nospin(pll_1_sem);
		ret = -1;
	} else if (ret == -EBUSY) {
		get_sem_fail_ebusy++;
		ret = -2;
	} else if (ret == -EINVAL) {
		get_sem_fail_einval++;
		ret = -2;
	} else { /* Never come !!! */
		printk(KERN_ERR "[%s]:hwspin_unlock_nospin() spec NG\n"
				, __func__);
		ret = -2;
	}

set_pll1_c4_skip:
	return ret;
}

static void pll1_condition_at_wakeup(void)
{
	int ret;
	ret = hwspin_trylock_nospin(pll_1_sem);
	if (ret == 0) { /* Get sem OK */
		__raw_writel(__raw_readl(PLL1STPCR) &
					(~C4STP), PLL1STPCR);
		hwspin_unlock_nospin(pll_1_sem);
	} else if (ret == -EBUSY)
		get_sem_fail_ebusy++;
	else if (ret == -EINVAL)
		get_sem_fail_einval++;
	else /* Never come !!! */
		printk(KERN_ERR "[%s]:hwspin_unlock_nospin() spec NG\n",
				__func__);
}

#define POWER_BBPLLST					BIT(7)
#define POWER_BBPLLOFF					BIT(7)

/*
 * pll1_will_be_off_check
 *
 * return:
 *		0: successful
 *		-1: PLL1 cannot be off
 *		-2: Set C4 failed
 */
static int pll1_will_be_off_check(void)
{
	int ret;
	/* pll1 condition is successful */
	ret = pll1_condition_set();
	if (ret < 0)
		return ret;

	/* A3R or Bit7(BBPLLST) is not off */
	if (__raw_readl(PSTR) & (POWER_A3R | POWER_BBPLLST))
		ret = -1;

	return ret;
}

/* FRQCRA */
#define IFC_MASK (0xF << 20)
#define ZGFC_MASK (0xF << 16)
#define M3FC_MASK (0xF << 12)
#define BFC_MASK (0xF << 8)
#define M1FC_MASK (0xF << 4)
#define M5FC_MASK 0xF
#define FRQCRA_MSK (IFC_MASK | ZGFC_MASK | \
				M3FC_MASK | BFC_MASK | M1FC_MASK | M5FC_MASK)

/* FRQCRB */
#define KICK_BIT BIT(31)
#define ZTRFC_MASK (0xF << 20)
#define ZTFC_MASK (0xF << 16)
#define ZXFC_MASK (0xF << 12)
#define ZSFC_MASK (0xF << 8)
#define HPFC_MASK (0xF << 4)
#define FRQCRB_MSK (ZTRFC_MASK | ZTFC_MASK | ZXFC_MASK | \
						ZSFC_MASK | HPFC_MASK)

#define		DIV_1_1		-1
#define		DIV_1_2		0x0
#define		DIV_1_3		0x1
#define		DIV_1_4		0x2
#define		DIV_1_5		-1
#define		DIV_1_6		0x3
#define		DIV_1_7		-1
#define		DIV_1_8		0x4
#define		DIV_1_12	0x5
#define		DIV_1_16	0x6
#define		DIV_1_18	0x7
#define		DIV_1_24	0x8
#define		DIV_1_32	-1
#define		DIV_1_36	0xa
#define		DIV_1_48	0xb
#define		DIV_1_96	-1

/* clocks table change for corestandby 2
FRQCRA
I:		1/6
ZG:		1/4
M3:		1/8
B:		1/12
M1:		1/6
M5:		1/8

FRQCRB:
ZTR:	1/4
ZT:		1/6
ZX:		1/6
ZS:		1/6
HP:		1/12
*/

/* FRQCRA CHANGED */
#define IFC_CHANGE		(DIV_1_6 << 20)
#define ZGFC_CHANGE		(DIV_1_4 << 16)
#define M3FC_CHANGE		(DIV_1_16 << 12)
#define BFC_CHANGE		(DIV_1_24 << 8)
#define M1FC_CHANGE		(DIV_1_12 << 4)
#define M5FC_CHANGE		DIV_1_16
#define FRQCRA_CHANGE_CORE (IFC_CHANGE | ZGFC_CHANGE | \
			M3FC_CHANGE | BFC_CHANGE | M1FC_CHANGE | M5FC_CHANGE)
/* FRQCRB CHANGED */
#define ZTRFC_CHANGE (DIV_1_8 << 20)
#define ZTFC_CHANGE (DIV_1_12 << 16)
#define ZXFC_CHANGE (DIV_1_12 << 12)
#define ZSFC_CHANGE (DIV_1_12 << 8)
#define HPFC_CHANGE (DIV_1_24 << 4)
#define FRQCRB_CHANGE_CORE (ZTRFC_CHANGE | ZTFC_CHANGE | ZXFC_CHANGE | \
						ZSFC_CHANGE | HPFC_CHANGE)

/*
 * shmobile_enter_corestandby: executes idle PM for a CPU - Corestandby state
 * @dev: cpuidle device for this cpu
 * @drv: cpuidle driver for this cpu
 * @index: index into drv->states of the state to enter
 * return:
 *		int: index into drv->states of the state to exit
 */

static int shmobile_enter_corestandby_2(struct cpuidle_device *dev,
	struct cpuidle_driver *drv, int index)
{
	ktime_t time_start, time_end;
	s64 diff;
	long wakelock;
	unsigned int dr_WUPSFAC;
	int clocks_ret, clocks_changed = 0;
	unsigned int freqA_save;
	unsigned int freqB_save;
#if (defined ZB3_CLK_IDLE_ENABLE) && (defined ZB3_CLK_DFS_ENABLE)
	unsigned int freqD_save = 0;
	int chip_rev;
#endif /*(defined ZB3_CLK_IDLE_ENABLE) && (defined ZB3_CLK_DFS_ENABLE)*/
	int ret;
	int cpuid = smp_processor_id();

	idle_log(">>>IN\n");

	FIQ_DISABLE();

	time_start = ktime_get();

	/* Core Standby wakelock check */
	wakelock = has_wake_lock(WAKE_LOCK_IDLE);
	if (!wakelock) {
		/* Core Standby State Notify */
		if (!state_notify_confirm())
			state_notify(PM_STATE_NOTIFY_CORESTANDBY_2);

		if ((cpuid == 0) &&
		(__raw_readl(ram0Cpu1Status) == CPUSTATUS_HOTPLUG)) {

#ifdef PLL1_CAN_OFF
			/* PLL1 is sure to be off ? */
			ret = pll1_will_be_off_check();
			if (ret == 0)
				goto clock_change;
			else if (ret == -2)
				/* Have to handle case C4 is set,
				but cannot due to sem fail */
				goto out;
			else /* (ret == -1) */
					/* PLL1 cannot off,
					no need to change clocks */
					goto skip_clock_change;

clock_change:
			/* backup freqs before change */
			freqA_save = __raw_readl(FRQCRA);
			freqB_save = __raw_readl(FRQCRB);
			/* set clocks */
			clocks_ret = clock_update(FRQCRA_CHANGE_CORE,
				FRQCRA_MSK, FRQCRB_CHANGE_CORE, FRQCRB_MSK);
			if (clocks_ret < 0) {
				printk(KERN_INFO "[%s]: set clocks FAILED\n", \
				__func__);
				goto skip_clock_change;
			}

			clocks_changed = 1;

#if (defined ZB3_CLK_IDLE_ENABLE) && (defined ZB3_CLK_DFS_ENABLE)
			chip_rev = shmobile_chip_rev();
			if (chip_rev > ES_REV_2_1) {
				freqD_save = suspend_ZB3_backup();
				if (freqD_save > 0) {
					clocks_ret =
					cpg_set_sbsc_freq(ZB3_CLK_CORESTANDBY2);
					/* if (clocks_ret < 0) {
						printk
						(KERN_INFO
						"[%s]: set ZB3 clocks FAILED\n",
							__func__);
					} else {
						printk
						(KERN_INFO
						"[%s]: set ZB3 clocks OK\n",
							__func__);
					} */
				} else {
					/* printk
					(KERN_INFO
					"[%s]: Backup ZB3 clocks FAILED\n",
						__func__); */
					clocks_ret = freqD_save;
				}
			}

#endif /*(defined ZB3_CLK_IDLE_ENABLE) && (defined ZB3_CLK_DFS_ENABLE)*/

#else /*!defined(PLL1_CAN_OFF)*/
#if (defined ZB3_CLK_IDLE_ENABLE) && (defined ZB3_CLK_DFS_ENABLE)
			chip_rev = shmobile_chip_rev();
			if (chip_rev > ES_REV_2_1) {
				freqD_save = suspend_ZB3_backup();
				if (freqD_save > 0) {
					clocks_ret =
					cpg_set_sbsc_freq(ZB3_CLK_CORESTANDBY2);
					/* if (clocks_ret < 0) {
						printk
						(KERN_INFO
						"[%s]: set ZB3 clocks FAILED\n",
							__func__);
					} else {
						printk
						(KERN_INFO
						"[%s]: set ZB3 clocks OK\n",
							__func__);
					}*/
				} else {
					/*printk
					(KERN_INFO
					"[%s]: Backup ZB3 clocks FAILED\n",
						__func__);*/
					clocks_ret = freqD_save;
				}
			}

#endif /*(defined ZB3_CLK_IDLE_ENABLE) && (defined ZB3_CLK_DFS_ENABLE)*/
#endif

skip_clock_change:
			/* end clock table */

			start_corestandby_2(); /* CoreStandby(A2SL Off) */

#if (defined ZB3_CLK_IDLE_ENABLE) && (defined ZB3_CLK_DFS_ENABLE)
			if ((chip_rev > ES_REV_2_1) && (freqD_save > 0)) {
				clocks_ret = cpg_set_sbsc_freq(freqD_save);
				/*if (clocks_ret < 0) {
					printk
					(KERN_INFO
					"[%s]: Restore ZB3 clocks FAILED\n",
						__func__);
				} else {
					printk
					(KERN_INFO
					"[%s]: Restore ZB3 clocks OK\n",
						__func__);
				}*/
			}
#endif /*(defined ZB3_CLK_IDLE_ENABLE) && (defined ZB3_CLK_DFS_ENABLE)*/

#ifdef PLL1_CAN_OFF
			/* update pll1 stop condition without C4 */
			pll1_condition_at_wakeup();

			/* restore clocks */
			if (clocks_changed) {
				clocks_ret = clock_update(freqA_save,
					FRQCRA_MSK, freqB_save, FRQCRB_MSK);
				if (clocks_ret < 0)
					printk(KERN_INFO "[%s]: restore clocks FAILED\n ",
					__func__);
			}
#endif
			dr_WUPSFAC = __raw_readl(WUPSFAC);
#if 0	/* for debug */
			if (dr_WUPSFAC)
				printk(KERN_INFO "[%s] is wake-up 0x%08X. ", \
						__func__, dr_WUPSFAC);
#endif

			goto finished_wakeup;

		}

/* #ifdef PLL1_CAN_OFF */
out: /* go to corestandby for power consumption */
/* #endif */

		start_corestandby();

	} else { /* idle wakelock is used */

		idle_log(">>>IN (WAKELOCK)\n");

		/* Sleep State Notify */
		if (!state_notify_confirm())
			state_notify(PM_STATE_NOTIFY_SLEEP);
		cpu_do_idle(); /* WFI cpu_do_idle(); */
	}

finished_wakeup:

	/* WakeUp State Notify */
	if (!state_notify_confirm())
		state_notify(PM_STATE_NOTIFY_WAKEUP);

	time_end = ktime_get();

	local_irq_enable();
	FIQ_ENABLE();

	diff = ktime_to_us(ktime_sub(time_end, time_start));
	if (diff > INT_MAX)
		diff = INT_MAX;

	dev->last_residency = (int) diff;

	idle_log("<<<OUT idle_time[0x%x]\n", idle_time);

	return index;
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
	unsigned long flags;
	int error = 0;
	struct cpuidle_driver *drv = &shmobile_cpuidle_driver;

	spin_lock_irqsave(&cpuidle_debug_lock, flags);
	switch (is_enable) {

	case 0:
		if (!is_enable_cpuidle)
			break; /* Already disabled */
		/* Let the governor work/statistic correct info */
		drv->state_count = SHMOBILE_MAX_STATES_DEBUG;
		/* Make sure that only WFI state is running */
		drv->states[0].enter = shmobile_enter_wfi_debug;
		drv->states[1].enter = shmobile_enter_wfi_debug;
		drv->states[2].enter = shmobile_enter_wfi_debug;
		drv->states[3].enter = shmobile_enter_wfi_debug;
		is_enable_cpuidle = is_enable;
		break;

	case 1:
		if (is_enable_cpuidle)
			break; /* Already enabled */
		/* Restore to original CPU's idle PM */
		drv->state_count = SHMOBILE_MAX_STATES;
		drv->states[0].enter = shmobile_enter_wfi;
		/*drv->states[1].enter = shmobile_enter_wfi_lowfreq;*/
		drv->states[1].enter = shmobile_enter_wfi;
		drv->states[2].enter = shmobile_enter_corestandby;
		drv->states[3].enter = shmobile_enter_corestandby_2;
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
	struct cpuidle_device *dev;
	struct cpuidle_driver *drv = &shmobile_cpuidle_driver;
	unsigned int cpu;
	int ret;

	ret = shmobile_init_pm();
	if (ret != 0)
		return ret;

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
#ifdef CONFIG_PM_BOOT_SYSFS
		is_enable_cpuidle = 0;
		/* Make sure that only WFI state is running */
		drv->states[0].enter = shmobile_enter_wfi_debug;
		drv->states[1].enter = shmobile_enter_wfi_debug;
		drv->states[2].enter = shmobile_enter_wfi_debug;
		drv->states[3].enter = shmobile_enter_wfi_debug;
#endif
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

	/* - set the legacy mode to LPCKCR */
	__raw_writel(CPG_LPCKCR_LEGACY, CPG_LPCKCR);
	/* - set PLL0 stop conditon to A2SL state by CPG.PLL0STPCR */
	__raw_writel(A2SLSTP, PLL0STPCR);

	/* - set Wake up factor unmask to GIC.CPU0 by SYS.WUPSMSK */
	__raw_writel((__raw_readl(WUPSMSK) &  ~(1 << 28)), WUPSMSK);

	return 0;
}
device_initcall(shmobile_init_cpuidle);

