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

#include <linux/hwspinlock.h>
#include <linux/delay.h>

#ifndef CONFIG_PM_HAS_SECURE
#include "pm_ram0.h"
#else /*CONFIG_PM_HAS_SECURE*/
#include "pm_ram0_tz.h"
#endif /*CONFIG_PM_HAS_SECURE*/
#include "pmRegisterDef.h"

#define ZB3_CLK_CORESTANDBY2	(65000)
#define SHMOBILE_MAX_STATES	4

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

	FIQ_DISABLE();

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
	FIQ_ENABLE();

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

	FIQ_DISABLE();

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
	FIQ_ENABLE();

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

	FIQ_DISABLE();
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
	FIQ_ENABLE();

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

	idle_log(">>>IN\n");

	FIQ_DISABLE();

	do_gettimeofday(&beforeTime);

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
		arch_idle(); /* WFI cpu_do_idle(); */
	}

	/* WakeUp State Notify */
	if (!state_notify_confirm())
		state_notify(PM_STATE_NOTIFY_WAKEUP);

	do_gettimeofday(&afterTime);

	idle_time = (afterTime.tv_sec - beforeTime.tv_sec) * 1000000
				+ (afterTime.tv_usec - beforeTime.tv_usec);

	local_irq_enable();
	FIQ_ENABLE();

	idle_log("<<<OUT idle_time[0x%x]\n", idle_time);

	return idle_time;
}

struct hwspinlock *pll_1_sem;

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
	if ((__raw_readl(CPG_MSTPSR1) & MSTPST1_PLL1) != MSTPST1_PLL1)
		goto set_pll1_c4;

	if ((__raw_readl(CPG_MSTPSR2) & MSTPST2_PLL1) != MSTPST2_PLL1)
		goto set_pll1_c4;

	if ((__raw_readl(CPG_MSTPSR3) & MSTPST3_PLL1) != MSTPST3_PLL1)
		goto set_pll1_c4;

	if ((__raw_readl(CPG_MSTPSR4) & MSTPST4_PLL1) == MSTPST4_PLL1)
		goto set_pll1_c4_skip;

set_pll1_c4:
	ret = hwspin_trylock_nospin(pll_1_sem);
	if (ret == 0) { /* Get sem OK */
		__raw_writel(__raw_readl(CPG_PLL1STPCR) | C4STP, CPG_PLL1STPCR);
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
		__raw_writel(__raw_readl(CPG_PLL1STPCR) &
					(~C4STP), CPG_PLL1STPCR);
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
#define FRQCRA_MSK (IFC_MASK | ZGFC_MASK | M3FC_MASK | \
						BFC_MASK | M1FC_MASK | M5FC_MASK)

/* FRQCRB */
#define KICK_BIT BIT(31)
#define ZTRFC_MASK (0xF << 20)
#define ZTFC_MASK (0xF << 16)
#define ZXFC_MASK (0xF << 12)
#define ZSFC_MASK (0xF << 8)
#define HPFC_MASK (0xF << 4)
#define FRQCRB_MSK (ZTRFC_MASK | ZTFC_MASK | ZXFC_MASK | \
						ZSFC_MASK | HPFC_MASK)

#define KICK_WAIT_INTERVAL_US	10

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
#define FRQCRA_CHANGE_CORE (IFC_CHANGE | ZGFC_CHANGE | M3FC_CHANGE | \
						BFC_CHANGE | M1FC_CHANGE | M5FC_CHANGE)
/* FRQCRB CHANGED */
#define ZTRFC_CHANGE (DIV_1_8 << 20)
#define ZTFC_CHANGE (DIV_1_12 << 16)
#define ZXFC_CHANGE (DIV_1_12 << 12)
#define ZSFC_CHANGE (DIV_1_12 << 8)
#define HPFC_CHANGE (DIV_1_24 << 4)
#define FRQCRB_CHANGE_CORE (ZTRFC_CHANGE | ZTFC_CHANGE | ZXFC_CHANGE | \
						ZSFC_CHANGE | HPFC_CHANGE)

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
int core_wait_kick(unsigned int time)
{
	unsigned int wait_time = time;

	while (0 < wait_time--) {
		if ((__raw_readl(CPG_FRQCRB) >> 31) == 0)
			break;
		shmobile_suspend_udelay(1);
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
int core_set_kick(unsigned int time)
{
	unsigned int wait_time = time;

	if ((wait_time <= 0) || (wait_time > KICK_WAIT_INTERVAL_US))
		wait_time = KICK_WAIT_INTERVAL_US;
	__raw_writel(BIT(31) | __raw_readl(CPG_FRQCRB), CPG_FRQCRB);

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
	current_value = __raw_readl(CPG_FRQCRA);
	if (freqA != (current_value & freqA_mask))
		freqA_change = 1;
	/* check if freqB change */
	current_value = __raw_readl(CPG_FRQCRB);
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
			__raw_writel(freqA | (__raw_readl(CPG_FRQCRA) &
						(~freqA_mask)),
					CPG_FRQCRA);
		if (zs_change || freqB_change)
			__raw_writel(freqB | (__raw_readl(CPG_FRQCRB) &
						(~freqB_mask)),
				CPG_FRQCRB);

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
EXPORT_SYMBOL(clock_update);


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

	do_gettimeofday(&beforeTime);

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
			freqA_save = __raw_readl(CPG_FRQCRA);
			freqB_save = __raw_readl(CPG_FRQCRB);
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
					clocks_ret = cpg_set_sbsc_freq(ZB3_CLK_CORESTANDBY2);
					if (clocks_ret < 0) {
						printk(KERN_INFO "[%s]: set ZB3 clocks FAILED\n",
							__func__);
					} else {
						printk(KERN_INFO "[%s]: set ZB3 clocks OK\n",
							__func__);
					}
				} else {
					printk(KERN_INFO "[%s]: Backup ZB3 clocks FAILED\n",
						__func__);
					clocks_ret = freqD_save;
				}
			}
			
#endif /*(defined ZB3_CLK_IDLE_ENABLE) && (defined ZB3_CLK_DFS_ENABLE)*/

#else /*!defined(PLL1_CAN_OFF)*/
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
#if (defined ZB3_CLK_IDLE_ENABLE) && (defined ZB3_CLK_DFS_ENABLE)
			chip_rev = shmobile_chip_rev();
			if (chip_rev > ES_REV_2_1) {
				freqD_save = suspend_ZB3_backup();
				if (freqD_save > 0) {
					clocks_ret = cpg_set_sbsc_freq(ZB3_CLK_CORESTANDBY2);
					if (clocks_ret < 0) {
						printk(KERN_INFO "[%s]: set ZB3 clocks FAILED\n",
							__func__);
					} else {
						printk(KERN_INFO "[%s]: set ZB3 clocks OK\n",
							__func__);
					}
				} else {
					printk(KERN_INFO "[%s]: Backup ZB3 clocks FAILED\n",
						__func__);
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
				if (clocks_ret < 0) {
					printk(KERN_INFO "[%s]: Restore ZB3 clocks FAILED\n",
							__func__);
				} else {
					printk(KERN_INFO "[%s]: Restore ZB3 clocks OK\n",
							__func__);
				}
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
		arch_idle(); /* WFI cpu_do_idle(); */
	}

finished_wakeup:
	/* WakeUp State Notify */
	if (!state_notify_confirm())
		state_notify(PM_STATE_NOTIFY_WAKEUP);

	do_gettimeofday(&afterTime);

	idle_time = (afterTime.tv_sec - beforeTime.tv_sec) * 1000000
				+ (afterTime.tv_usec - beforeTime.tv_usec);

	local_irq_enable();
	FIQ_ENABLE();

	idle_log("<<<OUT idle_time[0x%x]\n", idle_time);

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

	/* General HBP semaphore 0 + Dedicated (SW) semaphore #temp */
	pll_1_sem = hwspin_lock_request_specific(SMGP002);
	if (pll_1_sem == NULL) {
		printk(KERN_ERR "(%s:[%d])Unable to register hw_sem\n",
					__func__, __LINE__);
		return -EIO;
	}

	return 0;
}
device_initcall(shmobile_init_cpuidle);

