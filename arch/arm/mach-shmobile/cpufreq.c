/*
 * arch/arm/mach-shmobile/cpufreq.c
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
#include <linux/types.h>
#include <linux/cpu.h>
#include <linux/cpufreq.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/earlysuspend.h>
#include <linux/slab.h>
#include <asm/system.h>
#include <mach/common.h>
#include <mach/pm.h>

#ifdef pr_fmt
#undef pr_fmt
#define pr_fmt(fmt) "dvfs[cpufreq.c<%d>]:" fmt, __LINE__
#endif

#define EXTAL1 26000
#define FREQ_MAX15 1456000
#define FREQ_MID_UPPER_LIMIT15 910000
#define FREQ_MID_LOWER_LIMIT15 455000
#define FREQ_MIN_UPPER_LIMIT15 364000
#define FREQ_MIN_LOWER_LIMIT15 273000
#define FREQ_MAX12 1196000
#define FREQ_MID_UPPER_LIMIT12 897000
#define FREQ_MID_LOWER_LIMIT12 448500
#define FREQ_MIN_UPPER_LIMIT12 373750
#define FREQ_MIN_LOWER_LIMIT12 299000

/* Clocks State */
enum clock_state {
	MODE_SUSPEND = 0,
	MODE_NORMAL,
	MODE_EARLY_SUSPEND,
	MODE_NUM
};
#define SUSPEND_CPUFREQ15 FREQ_MID_LOWER_LIMIT15	/* Suspend */
#define SUSPEND_CPUFREQ12 FREQ_MID_LOWER_LIMIT12
#define CORESTB_CPUFREQ CPUFREQ_ENTRY_INVALID   /* CoreStandby */
#define FREQ_TRANSITION_LATENCY  (CONFIG_SH_TRANSITION_LATENCY * NSEC_PER_USEC)

#define MAX_ZS_DIVRATE	DIV1_6
#define MAX_HP_DIVRATE	DIV1_12

/* For change sampling rate & down factor dynamically */
#define SAMPLING_RATE_DEF FREQ_TRANSITION_LATENCY
#define SAMPLING_RATE_LOW 500000
#define SAMPLING_DOWN_FACTOR_DEF 20
#define SAMPLING_DOWN_FACTOR_LOW 1

#define INIT_STATE	1
#define BACK_UP_STATE	2
#define NORMAL_STATE	3
#define STOP_STATE	4

#define HWREV_041	4

/* FIX me: need mock for bellow APIs
 *	   this should be disabled after got mock funcion
 */
#ifndef CONFIG_HOTPLUG_CPU_MGR
#define cpu_up_manager(x, y)	cpu_up(x)
#define cpu_down_manager(x, y)	cpu_down(x)
#endif /*CONFIG_HOTPLUG_CPU_MGR*/

/* Resource */
struct cpufreq_resource {
	bool used;
	atomic_t usage_count;
};

/* CPU info */
struct shmobile_cpuinfo {
	struct cpufreq_resource upper_lowspeed;
	struct cpufreq_resource highspeed;
	unsigned int freq;
	unsigned int req_rate[CONFIG_NR_CPUS];
	unsigned int limit_maxfrq;
	unsigned int freq_max;
	unsigned int freq_mid_upper_limit;
	unsigned int freq_mid_lower_limit;
	unsigned int freq_min_upper_limit;
	unsigned int freq_min_lower_limit;
	unsigned int freq_suspend;
	int sgx_flg;
	int clk_state;
	int scaling_locked;
	spinlock_t lock;
};

/**************** static ****************/
static int zclk12_flg;
#ifdef CONFIG_PM_DEBUG
int cpufreq_enabled = 1;
#endif /* CONFIG_PM_DEBUG */
#ifndef CPUFREQ_TEST_MODE
static
#endif /* CPUFREQ_TEST_MODE */
struct shmobile_cpuinfo	the_cpuinfo;
static struct cpufreq_frequency_table
/* ES1.0 */
main_freqtbl_es1_x[] = {
	{.index = 0, .frequency = 988000}, /* max		*/
	{.index = 1, .frequency = 494000}, /* mid		*/
	{.index = 2, .frequency = 247000}, /* low		*/
	{.index = 3, .frequency =  CPUFREQ_TABLE_END}
},
/* ES2.x */
main_freqtbl_es2_x[11],
*freq_table = NULL;

static struct {
	int pllratio;
	int div_rate;
	int waveform;
} zdiv_table15[] = {
	{ PLLx56, DIV1_1, LLx16_16},	/* 1,456 MHz	*/
	{ PLLx49, DIV1_1, LLx14_16},	/* 1,274 MHz	*/
	{ PLLx42, DIV1_1, LLx12_16},	/* 1,092 MHz	*/
	{ PLLx35, DIV1_1, LLx10_16},	/*   910 MHz	*/
	{ PLLx56, DIV1_2, LLx8_16},	/*   728 MHz	*/
	{ PLLx49, DIV1_2, LLx7_16},	/*   637 MHz	*/
	{ PLLx42, DIV1_2, LLx6_16},	/*   546 MHz	*/
	{ PLLx35, DIV1_2, LLx5_16},	/*   455 MHz	*/
	{ PLLx56, DIV1_4, LLx4_16},	/*   364 MHz	*/
	{ PLLx42, DIV1_4, LLx3_16},	/*   273 MHz	*/
},
zdiv_table12[] = {
	{ PLLx46, DIV1_1, LLx16_16},	/* 1196    MHz	*/
	{ PLLx46, DIV1_1, LLx14_16},	/* 1046.5  MHz	*/
	{ PLLx46, DIV1_1, LLx12_16},	/*  897    MHz	*/
	{ PLLx46, DIV1_1, LLx10_16},	/*  747.5  MHz	*/
	{ PLLx46, DIV1_2, LLx9_16},	/*  672.75 MHz	*/
	{ PLLx46, DIV1_2, LLx7_16},	/*  523.25 MHz	*/
	{ PLLx46, DIV1_2, LLx6_16},	/*  448.5  MHz	*/
	{ PLLx46, DIV1_2, LLx5_16},	/*  373.75 MHz	*/
	{ PLLx46, DIV1_4, LLx4_16},	/*  299    MHz	*/
},
*zdiv_table = NULL;

static int debug = 1;
#ifdef CONFIG_CPU_FREQ_DEFAULT_GOV_ONDEMAND
static int sampling_flag = INIT_STATE;
#else
static int sampling_flag = STOP_STATE;
#endif /* CONFIG_CPU_FREQ_DEFAULT_GOV_ONDEMAND */
module_param(debug, int, S_IRUGO | S_IWUSR | S_IWGRP);

#ifdef DYNAMIC_HOTPLUG_CPU
static void do_check_cpu(struct work_struct *work);
static DECLARE_DEFERRED_WORK(hlg_work, do_check_cpu);

/* FIX me: need to study more about below const */
#define DEF_MAX_REQ_NR		10
#define DEF_MAX_REQ_CHECK_NR	2
#define DEF_UNPLUG_THRESHOLD12	373750	/* KHz */
#define DEF_PLUG_THRESHOLD12	897000	/* KHz */
#define DEF_UNPLUG_THRESHOLD15	364000	/* KHz */
#define DEF_PLUG_THRESHOLD15	910000	/* KHz */
#define DEF_MAX_SAM_IN_US	(FREQ_TRANSITION_LATENCY * 2) /* 50ms * 2 */

static struct {
	unsigned int req_chk_nr;
	unsigned int req_his_freq[DEF_MAX_REQ_NR];
	unsigned int req_his_idx;
	unsigned int plug_threshold;
	unsigned int unplug_threshold;
	unsigned int sampling_rate;
	unsigned int hlg_enabled;
	struct mutex timer_mutex;
} hlg_config = {
	.req_chk_nr = DEF_MAX_REQ_CHECK_NR,
	.req_his_idx = 0,
	.plug_threshold = DEF_PLUG_THRESHOLD15,
	.unplug_threshold = DEF_UNPLUG_THRESHOLD15,
	.sampling_rate = DEF_MAX_SAM_IN_US,
	.hlg_enabled = 0
};

/*
 * pr_his_req: print requested frequency buffer
 *
 * Argument:
 *		None
 *
 * Return:
 *		None
 */
static inline void pr_his_req(void)
{
#ifdef DVFS_DEBUG_MODE
	int i = 0;
	int j = hlg_config.req_his_idx - 1;

	for (i = 0; i < DEF_MAX_REQ_NR; i++, j--) {
		if (j < 0)
			j = DEF_MAX_REQ_NR - 1;
		pr_log("[%2d]%07u\n", j, hlg_config.req_his_freq[j]);
	}
#endif
}

/*
 * static_governor: check if governor is validate
 *
 * Argument:
 *		none
 *
 * Return:
 *		0: The governor is validate
 *		1: The governor is invalidate
 */
static int static_governor(void)
{
	static const char * const governors[] = {
		"conservative", "ondemand", "interactive"
	};
	struct cpufreq_policy *policy = cpufreq_cpu_get(0);
	int i = 0;
	int len = ARRAY_SIZE(governors);

	if (!policy)
		return 1;

	for (i  = 0; i < len; i++) {
		if (strcmp(governors[i], policy->governor->name) == 0)
			return 0;
	}

	return 1;
}

/*
 * wakeup_nonboot_cpus: bring up nonboot-CPU which is off-line
 *
 * Argument:
 *		none
 *
 * Return:
 *		none
 */
static inline void wakeup_nonboot_cpus(void)
{
	unsigned int cpu;

	for_each_present_cpu(cpu) {
		if (!cpu_online(cpu)) {
			pr_his_req();
			pr_log("plug-in cpu<%d>\n", cpu);
			cpu_up_manager(cpu, DFS_HOTPLUG_ID);
		}
	}
}

/*
 * wakeup_nonboot_cpus: take down nonboot-CPU which is on-line
 *
 * Argument:
 *		none
 *
 * Return:
 *		none
 */
static inline void shutdown_nonboot_cpus(void)
{
	unsigned int cpu;

	for_each_online_cpu(cpu) {
		if (cpu) {
			pr_his_req();
			cpu_down_manager(cpu, DFS_HOTPLUG_ID);
			pr_log("plug-out cpu<%d> done\n", cpu);
		}
	}
}

/*
 * policy_update: Check policy for nonboot-CPU hotplug
 *
 * Argument:
 *		none
 *
 * Return:
 *		1: policy checking is unexpected for hotplug
 *			(if CPU is off-line, it need to be plug-in)
 *		0: policy checking is ok for hotplug
 */
static inline int policy_update(void)
{
	struct cpufreq_policy *policy = NULL;

	policy = cpufreq_cpu_get(0);
	/* by setting */
	if (policy && (policy->max < hlg_config.plug_threshold))
		return 1;

	if (policy && (policy->min > hlg_config.unplug_threshold))
		return 1;

	return 0;
}

/*
 * fixup_all_cpu_up: check all conditions for hotplug nonboot-CPU
 *			if any case occurs, the CPU need
*			 to be plug-in (if it's in off-line state)
 * Argument:
 *		none
 *
 * Return:
 *		1: condition is unexpected for hotplug
 *			(if CPU is off-line, it need to be plug-in)
 *		0: condition is ok for hotplug
 */
static inline int fixup_all_cpu_up(void)
{
	/* by demand */
	if (static_governor())
		return 1;

	if (the_cpuinfo.limit_maxfrq < hlg_config.plug_threshold)
		return 1;

	/* policy update */
	if (policy_update())
		return 1;

	/* by request */
	if (the_cpuinfo.scaling_locked &&
	   (the_cpuinfo.freq <= hlg_config.plug_threshold))
		return 1;

	/* by request */
	if (the_cpuinfo.upper_lowspeed.used &&
	   (the_cpuinfo.freq_min_upper_limit >= hlg_config.unplug_threshold))
		return 1;

	return 0;
}

/*
 * hotplug_nonboot_cpu: check and do hotplug nonboot-CPU
 * Argument:
 *		none
 *
 * Return:
 *		none
 */
static void hotplug_nonboot_cpu(void)
{
	unsigned long average = 0;
	int i = 0;
	int j = 0;

	/* normal mode */
	if (!hlg_config.hlg_enabled)
		return;

	/* max system cpu frequency is limitted under plug-inin
	 * threshold
	 */
	if (fixup_all_cpu_up()) {
		wakeup_nonboot_cpus();
		goto done;
	}

	/* check for unplugging nonboot cpus
	 *
	 * plug CPU if all of below conditions are satified
	 *	+ early suspend state
	 *	+ last hlg_config.req_chk_nr requested frequencies under
	 *	  hlg_config.unplug_threshold
	 */
	for (i = 0, j = hlg_config.req_his_idx - 1;
		i < hlg_config.req_chk_nr; i++, j--) {
		if (j < 0)
			j = DEF_MAX_REQ_NR - 1;

		if (hlg_config.req_his_freq[j] > hlg_config.unplug_threshold)
			break;
	}

	if (i >= hlg_config.req_chk_nr) {
		shutdown_nonboot_cpus();
		goto done;
	}

	/* check for plugging nonboot cpus
	 *
	 * plug CPU if all of below conditions are satified
	 *	+ early suspend state
	 *	+ average requested frequencies over hlg_config.plug_threshold
	 */
	for (i = 0, j = hlg_config.req_his_idx - 1;
		i < DEF_MAX_REQ_NR; i++, j--) {
		if (j < 0)
			j = DEF_MAX_REQ_NR - 1;

		average += hlg_config.req_his_freq[j];
	}

	average /= DEF_MAX_REQ_NR;
	if (average >= hlg_config.plug_threshold)
		wakeup_nonboot_cpus();
done:
	if (hlg_config.req_his_idx >= DEF_MAX_REQ_NR)
		hlg_config.req_his_idx = 0;

	hlg_config.req_his_freq[hlg_config.req_his_idx++] = the_cpuinfo.freq;
}

/*
 * do_check_cpu: add workqueue for hot plug nonboot-CPU
 * Argument:
 *		@work: check and do hotplug nonboot-CPU
 *
 * Return:
 *		none
 */
static void do_check_cpu(struct work_struct *work)
{
	/* check for hotplug-ing condition */
	mutex_lock(&hlg_config.timer_mutex);
	hotplug_nonboot_cpu();
	mutex_unlock(&hlg_config.timer_mutex);

	/* governor currectly used: conservative/ondemand/interractive?
	 * only active hotplug workqueue if one of these governors used
	 */
	if (!static_governor())
		schedule_delayed_work_on(0, &hlg_work,
			usecs_to_jiffies(hlg_config.sampling_rate));
	else
		wakeup_nonboot_cpus();
}
#endif /* DYNAMIC_HOTPLUG_CPU */

/*
 * find_target: find freq value in the freq table
 *		which is nearest the target freq
*		and return the freq table index
 * Argument:
 *		@table: freq table
 *		@freq: target freq
 *		@index: freq table index returned
 *
 * Return:
 *		0: Operation success
 *		negative: Operation fail
 */
static int find_target(struct cpufreq_frequency_table *table,
			unsigned int freq, unsigned int *index)
{
	struct cpufreq_policy *policy = NULL;

	policy = cpufreq_cpu_get(0);
	if (!policy)
		return -EINVAL;

	if (cpufreq_frequency_table_target(policy, table, freq,
		CPUFREQ_RELATION_H, index))
		return -EINVAL;

	return 0;
}

/*
 * __to_freq_level: convert from frequency to frequency level
 *
 * Argument:
 *		@freq: the frequency will be set
 *
 * Return:
 *		freq level
 */
static int __to_freq_level(unsigned int freq)
{
	int idx = -1;
	int found = -1;

	for (idx = 0; freq_table[idx].frequency != CPUFREQ_TABLE_END; idx++)
		if (freq_table[idx].frequency == freq)
			found = idx;
	return found;
}
#ifndef ZFREQ_MODE
/*
 * __to_div_rate: convert from frequency to divrate
 *
 * Argument:
 *		@freq: the frequency will be set
 *
 * Return:
 *		divrate
 */
static int __to_div_rate(unsigned int freq)
{
	int arr_size = 0;
	int idx = __to_freq_level(freq);

	arr_size = (int)ARRAY_SIZE(zdiv_table);
	if ((idx >= arr_size) || (idx < 0)) {
		pr_err("invalid parameter, frequency<%7u> index<%d>\n",
			freq, idx);
		return -EINVAL;
}

	return zdiv_table[idx].div_rate;
}
#endif
/*
 * __clk_get_rate: get the set of clocks
 *
 * Argument:
 * Return:
 *     address of element of array clocks_div_data
 *     NULL: if input parameters are invaid
 */
#ifndef CPUFREQ_TEST_MODE
static
#endif /* CPUFREQ_TEST_MODE */
int __clk_get_rate(struct clk_rate *rate)
{
	unsigned int target_freq = the_cpuinfo.freq;
	int clkmode = 0;
	int ret = 0;

	if (!rate) {
		pr_err("invalid parameter<NULL>\n");
		return -EINVAL;
	}

	clkmode = the_cpuinfo.clk_state;
	/* get the frequency mode */
	if (MODE_EARLY_SUSPEND == the_cpuinfo.clk_state) {
		if (target_freq <= the_cpuinfo.freq_mid_upper_limit)
			clkmode++;
		if (target_freq <= the_cpuinfo.freq_min_upper_limit)
			clkmode++;
	}

	/* get clocks setting according to clock mode */
	ret = pm_get_clock_mode(clkmode, rate);
	if (ret)
		pr_err("error! fail to get clock mode<%d>\n", clkmode);

	return ret;
}

/*
 * __notify_all_cpu: notify frequency change to all present CPUs
 *
 * Argument:
 *		@fold: the old frequency value
 *		@fnew: the new frequency value
 *		@cpu_nr: number of CPUS
 *		@flag: notification flag
 *
 * Return:
 *		none
 */
static void __notify_all_cpu(unsigned int fold, unsigned int fnew, int flag)
{
	struct cpufreq_freqs freqs;
	int i = 0;

	freqs.old = fold;
	freqs.new = fnew;
	for (i = 0; i < num_online_cpus(); i++) {
		freqs.cpu = i;
		cpufreq_notify_transition(&freqs, flag);
	}
}

/*
 * __set_rate: set SYS-CPU frequency
 *`
 * Argument:
 *		@freq: the frequency will be set
 *
 * Return:
 *     0: setting is normal
 *     negative: operation fail
 */
#ifndef CPUFREQ_TEST_MODE
static
#endif /* CPUFREQ_TEST_MODE */
int __set_rate(unsigned int freq)
{
#ifndef ZFREQ_MODE
	int div_rate = DIV1_1;
	int pllratio = PLLx56;
#endif /* ZFREQ_MODE */
	int freq_old = 0;
	int ret = 0;
	int level = 0;

	/* not allow higher frequency in emergency case(temp high) */
	freq = min(freq, the_cpuinfo.limit_maxfrq);
	if (freq == the_cpuinfo.freq)
		goto done;

	freq_old = the_cpuinfo.freq;
	level = __to_freq_level(freq);
	if (level < 0)
		return -EINVAL;
#ifndef ZFREQ_MODE
	div_rate = __to_div_rate(freq);
	if (div_rate < 0)
		return -EINVAL;

	ret = pm_set_syscpu_frequency(div_rate);
	if (!ret) {
		/* change PLL0 if need */
		pllratio = zdiv_table[level].pllratio;
		if (pm_get_pll_ratio(PLL0) != pllratio) {
			ret = pm_set_pll_ratio(PLL0, pllratio);
			if (ret)
				goto done;
		}

		the_cpuinfo.freq = freq;
		__notify_all_cpu(freq_old, freq, CPUFREQ_POSTCHANGE);
	}
#else /* ZFREQ_MODE */
	ret = pm_set_syscpu_frequency(zdiv_table[level].waveform);
	if (!ret) {
		the_cpuinfo.freq = freq;
		__notify_all_cpu(freq_old, freq, CPUFREQ_POSTCHANGE);
	}
#endif /* ZFREQ_MODE */
done:
	return ret;
}

static inline void __change_sampling_values(void)
{
	int ret = 0;
	static unsigned int downft = SAMPLING_DOWN_FACTOR_DEF;
	static unsigned int samrate = SAMPLING_RATE_DEF;

	if (STOP_STATE == sampling_flag) /* ondemand gov is stopped! */
		return;
	if (INIT_STATE == sampling_flag) {
		samplrate_downfact_change(SAMPLING_RATE_DEF,
					SAMPLING_DOWN_FACTOR_DEF,
					0);
		sampling_flag = NORMAL_STATE;
	}
	if ((the_cpuinfo.clk_state == MODE_EARLY_SUSPEND &&
		the_cpuinfo.freq <= the_cpuinfo.freq_min_upper_limit) ||
		the_cpuinfo.clk_state == MODE_SUSPEND) {
		if (NORMAL_STATE == sampling_flag) {/* Backup old values */
			samplrate_downfact_get(&samrate, &downft);
			sampling_flag = BACK_UP_STATE;
		}
		ret = samplrate_downfact_change(SAMPLING_RATE_LOW,
				SAMPLING_DOWN_FACTOR_LOW, 1);
	} else { /* Need to restore the previous values if any */
		if (BACK_UP_STATE == sampling_flag) {
			sampling_flag = NORMAL_STATE;
			ret = samplrate_downfact_change(samrate, downft, 0);
		}
	}
	if (ret)
		pr_err("%s()[%d]: error, samplrate_downfact_change(),"
			"ret<%d>\n", __func__, __LINE__, ret);
}

/*
 * __set_all_clocks: set SYS-CPU frequency and other clocks
 *`
 * Argument:
 *		@freq: the SYS frequency will be set
 *
 * Return:
 *     0: setting is normal
 *     negative: operation fail
 */
static inline int __set_all_clocks(unsigned int z_freq)
{
	int ret = 0;
	struct clk_rate rate;

	ret = __set_rate(z_freq);
	if (ret)
		return ret;

	/* get clocks setting based on new SYS-CPU clock */
	ret = __clk_get_rate(&rate);
	if (!ret)
		ret = pm_set_clocks(rate);
	__change_sampling_values();

	return ret;
}
/*
 * start_cpufreq: start dynamic frequency scaling, the SYS-CPU frequency
 * is changed automatically based on the system load.
 *
 * Argument:
 *		none
 *
 * Return: none
 */
void start_cpufreq(void)
{
	/* validate chip revision
	 * -> support revision 2.x and later
	 */
	if (validate()) {
		spin_lock(&the_cpuinfo.lock);

		if (atomic_dec_and_test(&the_cpuinfo.highspeed.usage_count))
			the_cpuinfo.highspeed.used = false;

		spin_unlock(&the_cpuinfo.lock);
	}
}
EXPORT_SYMBOL(start_cpufreq);

/*
 * stop_cpufreq: stop dynamic frequency scaling, the SYS-CPU frequency
 * is changed to maximum.
 *
 * Argument:
 *		none
 *
 * Return:
 *		0: normal
 *		negative: operation fail
 */
int stop_cpufreq(void)
{
	unsigned int freq_new = 0;
	struct cpufreq_policy *cur_policy = cpufreq_cpu_get(0);
	int ret = 0;

	/* validate chip revision
	 * -> support revision 2.x and later
	 */
	if (!validate())
		return ret;

	spin_lock(&the_cpuinfo.lock);
	/*
	 * emergency case, CPU is hot or system going to suspend,
	 * do not allow user to change frequency now
	 */
	if (MODE_SUSPEND == the_cpuinfo.clk_state) {
		spin_unlock(&the_cpuinfo.lock);
		return -EBUSY;
	}

	if (!atomic_read(&the_cpuinfo.highspeed.usage_count)) {
		the_cpuinfo.highspeed.used = true;
		/*
		 * skip setting hardware if the current SYS-CPU freq is
		 * MAX already
		 */
		if (the_cpuinfo.freq == the_cpuinfo.limit_maxfrq) {
			atomic_inc(&the_cpuinfo.highspeed.usage_count);
			spin_unlock(&the_cpuinfo.lock);
			return ret;
		}

		freq_new = the_cpuinfo.limit_maxfrq;
		ret = __set_all_clocks(freq_new);
		if (ret < 0) {
			the_cpuinfo.highspeed.used = false;
			spin_unlock(&the_cpuinfo.lock);
			return ret;
		}
	}

	atomic_inc(&the_cpuinfo.highspeed.usage_count);
	spin_unlock(&the_cpuinfo.lock);

	if (0 != freq_new) {
		/* this must be set for cpufreq_update_policy() */
		cur_policy->user_policy.max = freq_new;
		pr_log("update plicy->max\n");
		ret = cpufreq_update_policy(0);
	}
#ifdef DYNAMIC_HOTPLUG_CPU
	/* high-speed mode, need both CPUs online */
	if (the_cpuinfo.highspeed.used) {
		mutex_lock(&hlg_config.timer_mutex);
		wakeup_nonboot_cpus();
		mutex_unlock(&hlg_config.timer_mutex);
	}
#endif /* DYNAMIC_HOTPLUG_CPU */

	return ret;
}
EXPORT_SYMBOL(stop_cpufreq);

/*
 * disable_dfs_mode_min: prevent the CPU from running at minimum frequency,
 * in this case, the SYS-CPU only runs at middle frequency, or maximum
 * frequency.
 *
 * Argument:
 *		none
 *
 * Return: none
 */
void disable_dfs_mode_min(void)
{
	unsigned int freq_new = 0;
	int ret = 0;

	/* validate chip revision
	 * -> support revision 2.x and later
	 */
	if (!validate())
		return;

	spin_lock(&the_cpuinfo.lock);
	/*
	 * emergency case, CPU is hot, do not allow user to change
	 * frequency now
	 */
	if (the_cpuinfo.limit_maxfrq <= the_cpuinfo.freq_min_upper_limit)
		goto done;

	if (!atomic_read(&the_cpuinfo.upper_lowspeed.usage_count)) {
		if (the_cpuinfo.freq <= the_cpuinfo.freq_min_upper_limit) {
			freq_new = the_cpuinfo.freq_mid_lower_limit;
			ret = __set_all_clocks(freq_new);
			if (ret)
				goto done;
		}

		/* update flag */
		the_cpuinfo.upper_lowspeed.used = true;
	}

	atomic_inc(&the_cpuinfo.upper_lowspeed.usage_count);
done:
	spin_unlock(&the_cpuinfo.lock);
}
EXPORT_SYMBOL(disable_dfs_mode_min);

/*
 * enable_dfs_mode_min: allow the CPU to run at minimum frequency, in this case,
 * the SYS-CPU can run at minimum frequency, or middle frequency, or maximum
 * frequency.
 *
 * Argument:
 *		none
 *
 * Return:
 *		none
 */
void enable_dfs_mode_min(void)
{
	/* validate chip revision
	 * -> support revision 2.x and later
	 */
	if (validate()) {
		spin_lock(&the_cpuinfo.lock);

		if (atomic_dec_and_test
		   (&the_cpuinfo.upper_lowspeed.usage_count))
			the_cpuinfo.upper_lowspeed.used = false;

		spin_unlock(&the_cpuinfo.lock);
	}
}
EXPORT_SYMBOL(enable_dfs_mode_min);

/*
 * corestandby_cpufreq: change clocks in case of corestandby
 *
 * Argument:
 *		none
 * Return:
 *		0: normal
 *		negative: operation fail
 */
int corestandby_cpufreq(void)
{
	int ret = 0;

	/* validate chip revision
	 * -> support revision 2.x and later
	 */
	if (!validate())
		return ret;

	spin_lock(&the_cpuinfo.lock);
	if (atomic_read(&the_cpuinfo.highspeed.usage_count))
		goto done;

	if (CORESTB_CPUFREQ != CPUFREQ_ENTRY_INVALID) {
		/*
		 * emergency case, CPU is hot, do not allow user to change
		 * frequency now
		 */
		if (the_cpuinfo.limit_maxfrq <= CORESTB_CPUFREQ)
			goto done;

		ret = __set_rate(CORESTB_CPUFREQ);
		if (ret)
			pr_err("error! fail to do frequency setting\n");
	}
done:
	spin_unlock(&the_cpuinfo.lock);
	return ret;
}
EXPORT_SYMBOL(corestandby_cpufreq);

/*
 * suspend_cpufreq: change the SYS-CPU frequency to middle before entering
 * uspend state.
 * Argument:
 *		none
 *
 * Return:
 *		0: normal
 *		negative: operation fail
 */
int suspend_cpufreq(void)
{
	unsigned int freq_new = 0;
	int ret = 0;

	/* validate chip revision
	 * -> support revision 2.x and later
	 */
	if (!validate())
		return ret;

	spin_lock(&the_cpuinfo.lock);
	/*
	 * emergency case, CPU is hot, do not allow user to change
	 * frequency now
	 */
	if (the_cpuinfo.limit_maxfrq < the_cpuinfo.freq_suspend)
		goto done;

	the_cpuinfo.clk_state = MODE_SUSPEND;

	/* get the clock value for suspend state */
	freq_new = the_cpuinfo.freq_suspend;

	/* change SYS-CPU frequency */
	ret = __set_all_clocks(freq_new);
done:
	spin_unlock(&the_cpuinfo.lock);
#ifdef DYNAMIC_HOTPLUG_CPU
	/* dynamic hotplug cpu-core */
	mutex_lock(&hlg_config.timer_mutex);
	hlg_config.hlg_enabled = 0;
	mutex_unlock(&hlg_config.timer_mutex);
#endif /* DYNAMIC_HOTPLUG_CPU */

	return ret;
}
EXPORT_SYMBOL(suspend_cpufreq);

/*
 * resume_cpufreq: change the SYS-CPU and others frequency to middle
 * if the dfs has already been started.
 * Argument:
 *		none
 *
 * Return:
 *		0: normal
 *		negative: operation fail
 */
int resume_cpufreq(void)
{
	unsigned int freq_new = 0;
	int ret = 0;

	/* validate chip revision
	 * -> support revision 2.x and later
	 */
	if (!validate())
		return ret;

	spin_lock(&the_cpuinfo.lock);

	/* change to earlysuspend mode */
	the_cpuinfo.clk_state = MODE_EARLY_SUSPEND;
	freq_new = the_cpuinfo.freq;
	if (the_cpuinfo.highspeed.used)
		freq_new = the_cpuinfo.freq_max;

	ret = __set_all_clocks(freq_new);
	spin_unlock(&the_cpuinfo.lock);

#ifdef DYNAMIC_HOTPLUG_CPU
	/* dynamic hotplug cpu-core */
	mutex_lock(&hlg_config.timer_mutex);
	hlg_config.hlg_enabled = 1;
	mutex_unlock(&hlg_config.timer_mutex);
#endif /* DYNAMIC_HOTPLUG_CPU */

	return ret;
}
EXPORT_SYMBOL(resume_cpufreq);

/*
 * sgx_cpufreq: change the DFS mode to handle for SGX ON/OFF.
 *
 * Argument:
 *		@flag: the status of SGX module
 *			CPUFREQ_SGXON: the SGX is on
 *			CPUFREQ_SGXOFF: the SGX is off
 * Return:
 *		0: normal
 *		negative: operation fail
 */
int sgx_cpufreq(int flag)
{
	int ret = 0;

	/* validate chip revision
	 * -> support revision 2.x and later
	 */
	if (validate()) {
		spin_lock(&the_cpuinfo.lock);
		the_cpuinfo.sgx_flg = flag;
		spin_unlock(&the_cpuinfo.lock);
	}
	return ret;
}
EXPORT_SYMBOL(sgx_cpufreq);
/*
 * control_dfs_scaling: enable or disable dynamic frequency scaling.
 *
 * Argument:
 *		@enabled
 *			true: enable dynamic frequency scaling
 *			false: disable dynamic frequency scaling
 * Return:
 *		none
 */
void control_dfs_scaling(bool enabled)
{
	/* validate chip revision
	 * -> support revision 2.x and later
	 */
	if (validate()) {
		spin_lock(&the_cpuinfo.lock);
		the_cpuinfo.scaling_locked = (enabled) ? 0 : 1;

		if (enabled)
			the_cpuinfo.scaling_locked = 0;
		else
			the_cpuinfo.scaling_locked = 1;

		spin_unlock(&the_cpuinfo.lock);
	}
}
EXPORT_SYMBOL(control_dfs_scaling);

/*
 * suppress_clocks_change:enable ZS-phi change
 *
 * Argument:
 *		none
 *
 * Return:
 *		0: normal
 *		negative: operation fail
 */
int suppress_clocks_change(bool set_max)
{
	struct clk_rate clk_div;
	int ret = 0;

	/* validate chip revision
	 * -> support revision 2.x and later
	 */
	if (!validate() || (ES_REV_2_2 <= shmobile_chip_rev()))
		return ret;

	spin_lock(&the_cpuinfo.lock);
	if (set_max) {
		ret = __clk_get_rate(&clk_div);
		if (ret)
			goto done;

		/* restore to max */
		clk_div.zs_clk = MAX_ZS_DIVRATE;
		clk_div.hp_clk = MAX_HP_DIVRATE;
		ret = pm_set_clocks(clk_div);
	}

	if (!ret)
		ret = pm_disable_clock_change(ZSCLK | HPCLK);

done:
	spin_unlock(&the_cpuinfo.lock);
	return ret;
}
EXPORT_SYMBOL(suppress_clocks_change);

/*
 * unsuppress_clocks_change:disable ZS-phi change
 *
 * Argument:
 *		none
 *
 * Return:
 *		0: normal
 *		negative: operation fail
 */
void unsuppress_clocks_change(void)
{
	/* validate chip revision
	 * -> support revision 2.x and later
	 *
	 * (for later version (2.2), no need to W/A)
	 */
	if (validate() && (ES_REV_2_2 > shmobile_chip_rev())) {
		spin_lock(&the_cpuinfo.lock);
			if (!pm_enable_clock_change(ZSCLK | HPCLK))
				(void)__set_all_clocks(the_cpuinfo.freq);

		spin_unlock(&the_cpuinfo.lock);
	}
}
EXPORT_SYMBOL(unsuppress_clocks_change);

/*
 * limit_max_cpufreq: set max frequency
 *
 * Argument:
 *		@max: max frequency level to be limitted
 *
 * Return:
 *		0: normal
 *		negative: operation fail
 */
int limit_max_cpufreq(int max)
{
	int ret = 0;

	/* validate chip revision
	 * -> support revision 2.x and later
	 */
	if (!validate())
		return ret;

	spin_lock(&the_cpuinfo.lock);
	switch (max) {
	case LIMIT_NONE:
		the_cpuinfo.limit_maxfrq = the_cpuinfo.freq_max;
		break;
	case LIMIT_MID:
		the_cpuinfo.limit_maxfrq = the_cpuinfo.freq_mid_upper_limit;
		break;
	case LIMIT_LOW:
		the_cpuinfo.limit_maxfrq = the_cpuinfo.freq_min_upper_limit;
		break;
	default:
		spin_unlock(&the_cpuinfo.lock);
		return -EINVAL;
	}

	/* stop_cpufreq() may be called before, we need to update frequency */
	if (the_cpuinfo.highspeed.used ||
	   (the_cpuinfo.limit_maxfrq < the_cpuinfo.freq))
		ret = __set_all_clocks(the_cpuinfo.limit_maxfrq);

	spin_unlock(&the_cpuinfo.lock);
#ifdef DYNAMIC_HOTPLUG_CPU
	/* dynamic hotplug cpu-core */
	mutex_lock(&hlg_config.timer_mutex);
	if (hlg_config.hlg_enabled &&
	   (the_cpuinfo.limit_maxfrq < hlg_config.plug_threshold)) {
		pr_log("limit max %07u < plug threshold %07u",
			the_cpuinfo.limit_maxfrq, hlg_config.plug_threshold);
		wakeup_nonboot_cpus();
	}
	mutex_unlock(&hlg_config.timer_mutex);
#endif /* DYNAMIC_HOTPLUG_CPU */

	return ret;
}
EXPORT_SYMBOL(limit_max_cpufreq);

#ifdef CONFIG_PM_DEBUG
/*
 * control_cpufreq: runtime enable/disable cpufreq features
 *
 * Argument:
 *		@is_enable: enable flag: enable(1)/disable(0)
 *
 * Return:
 *		0: normal
 *		negative: operation fail
 */
int control_cpufreq(int is_enable)
{
	int ret = 0;

	if (!is_enable) {
		if (!cpufreq_enabled)
			goto done;

		ret = stop_cpufreq();
		if (ret)
			goto done;

		spin_lock(&the_cpuinfo.lock);
		cpufreq_enabled = 0;
		spin_unlock(&the_cpuinfo.lock);
		} else {
		if (!cpufreq_enabled) {
			spin_lock(&the_cpuinfo.lock);
			cpufreq_enabled = 1;
			spin_unlock(&the_cpuinfo.lock);

			start_cpufreq();
		}
	}
done:
	pr_log("cpufreq<%s>\n", (cpufreq_enabled) ? "on" : "off");
	return ret;
}
EXPORT_SYMBOL(control_cpufreq);

/*
 * is_cpufreq_enable: get runtime status
 *
 * Argument:
 *		None
 *
 * Return:
 *		0: disabled
 *		1: enabled
 */
int is_cpufreq_enable(void)
{
	return cpufreq_enabled;
}
EXPORT_SYMBOL(is_cpufreq_enable);
#endif /* CONFIG_PM_DEBUG */

#ifdef CONFIG_EARLYSUSPEND
/*
 * function: change clock state and set clocks, support for early suspend state
 * in system suspend.
 *
 * Argument:
 *		@h: the early_suspend interface
 *
 * Return:
 *		none
 */
#ifndef CPUFREQ_TEST_MODE
static
#endif /* CPUFREQ_TEST_MODE */
void shmobile_cpufreq_early_suspend(struct early_suspend *h)
{
	int ret = 0;

	/* validate chip revision
	 * -> support revision 2.x and later
	 */
	if (validate()) {
		spin_lock(&the_cpuinfo.lock);

		the_cpuinfo.clk_state = MODE_EARLY_SUSPEND;
		ret = __set_all_clocks(the_cpuinfo.freq);
		spin_unlock(&the_cpuinfo.lock);
#ifdef DYNAMIC_HOTPLUG_CPU
		/* dynamic hotplug cpu-core */
		mutex_lock(&hlg_config.timer_mutex);
		hlg_config.hlg_enabled = 1;
		mutex_unlock(&hlg_config.timer_mutex);
		schedule_delayed_work_on(0, &hlg_work,
			usecs_to_jiffies(hlg_config.sampling_rate));
#endif /* DYNAMIC_HOTPLUG_CPU */
	}
}

/*
 * shmobile_cpufreq_late_resume: change clock state and set clocks, support for
 * late resume state in system suspend.
 *
 * Argument:
 *		@h: the early_suspend interface
 *
 * Return:
 *		none
 */
#ifndef CPUFREQ_TEST_MODE
static
#endif /* CPUFREQ_TEST_MODE */
void shmobile_cpufreq_late_resume(struct early_suspend *h)
{
	/* validate chip revision
	 * -> support revision 2.x and later
	 */
	if (validate()) {
		spin_lock(&the_cpuinfo.lock);
		the_cpuinfo.clk_state = MODE_NORMAL;

		/* fixed: MAX frequency is used */
		if (the_cpuinfo.highspeed.used)
			(void)__set_all_clocks(the_cpuinfo.freq_max);
		else
			(void)__set_all_clocks(the_cpuinfo.freq);

		spin_unlock(&the_cpuinfo.lock);
#ifdef DYNAMIC_HOTPLUG_CPU
		/* dynamic hotplug cpu-core */
		mutex_lock(&hlg_config.timer_mutex);
		/* cancel workqueue from now on */
		cancel_delayed_work(&hlg_work);

		/* not allow hotplug now */
		hlg_config.hlg_enabled = 0;

		/* boot all secondaries cpu */
		wakeup_nonboot_cpus();
		mutex_unlock(&hlg_config.timer_mutex);
#endif /* DYNAMIC_HOTPLUG_CPU */
	}
}

static struct early_suspend shmobile_cpufreq_suspend = {
	.level = EARLY_SUSPEND_LEVEL_DISABLE_FB + 50,
	.suspend = shmobile_cpufreq_early_suspend,
	.resume  = shmobile_cpufreq_late_resume,
};
#endif /* CONFIG_EARLYSUSPEND */

/*
 * shmobile_cpufreq_verify: verify the limit frequency of the policy with the
 * limit frequency of the CPU.
 *
 * Argument:
 *		@policy: the input policy
 *
 * Return:
 *		0: normal
 *		negative: operation fail
 */
#ifndef CPUFREQ_TEST_MODE
static
#endif /* CPUFREQ_TEST_MODE */
int shmobile_cpufreq_verify(struct cpufreq_policy *policy)
{
	return cpufreq_frequency_table_verify(policy, freq_table);
}

/*
 * shmobile_cpufreq_getspeed: Retrieve the current frequency of a SYS-CPU.
 *
 * Argument:
 *		@cpu: the ID of CPU
 *
 * Return:
 *		the frequency value of input CPU.
 */
#ifndef CPUFREQ_TEST_MODE
static
#endif /* CPUFREQ_TEST_MODE */
unsigned int shmobile_cpufreq_getspeed(unsigned int cpu)
{
	return the_cpuinfo.freq;
}

/*
 * shmobile_cpufreq_target: judgle frequencies
 *
 * Argument:
 *		@policy: the policy
 *		@target_freq: the target frequency passed from CPUFreq framework
 *		@relation: not used
 *
 * Return:
 *		0: normal
 *		negative: operation fail
 */
#ifndef CPUFREQ_TEST_MODE
static
#endif /* CPUFREQ_TEST_MODE */
int shmobile_cpufreq_target(struct cpufreq_policy *policy,
	unsigned int target_freq, unsigned int relation)
{
	unsigned int old_freq = 0;
	unsigned int freq = ~0;
	int index = 0;
	int ret = 0;
	int cpu = 0;

	/* validate chip revision
	 * -> support revision 2.x and later
	 */
	if (!validate())
		return ret;

	spin_lock(&the_cpuinfo.lock);

	the_cpuinfo.req_rate[policy->cpu] = target_freq;
	/* only reduce the CPU frequency if all CPUs need to reduce */
	for_each_online_cpu(cpu)
		target_freq = max(the_cpuinfo.req_rate[cpu], target_freq);

	old_freq = the_cpuinfo.freq;
	ret = find_target(freq_table, target_freq, &index);
	if (ret)
		goto done;

	freq = freq_table[index].frequency;
	/* FREQ_MID_LOWER_LIMIT or upper will be used in case MIN frequency
	 * is suppressed
	 */
	if (the_cpuinfo.upper_lowspeed.used)
		freq = max((unsigned int)the_cpuinfo.freq_mid_lower_limit,
						freq);

	/* current frequency is set */
	if ((the_cpuinfo.freq == freq))
		goto done;

	/* frequency is not allowed to change */
	if ((the_cpuinfo.highspeed.used) || (the_cpuinfo.scaling_locked))
		goto done;

	/* suspend mode, frequency is not allowed to change */
	if (MODE_SUSPEND == the_cpuinfo.clk_state)
		goto done;

	ret = __set_all_clocks(freq);
done:
	spin_unlock(&the_cpuinfo.lock);

	/* the_cpuinfo.freq == freq when frequency changed */
	if ((the_cpuinfo.freq == freq) && debug)
		pr_info("[%07uKHz->%07uKHz]%s\n", old_freq, freq,
			(old_freq < freq) ? "^" : "v");

#ifdef DYNAMIC_HOTPLUG_CPU
	/* dynamic hotplug cpu-core */
	mutex_lock(&hlg_config.timer_mutex);

	/* hotplug is disabled before, do nothing */
	if (!hlg_config.hlg_enabled)
		goto end;

	/* need to cancel workqueue before do any setting */
	cancel_delayed_work(&hlg_work);

	/* dynamic frequency scalling governor is used, try to check hotplug
	 * condition
	 */
	if (!static_governor()) {
		if (!policy_update()) {
			hotplug_nonboot_cpu();

			/* schedule hotplug workqueue */
			schedule_delayed_work_on(0, &hlg_work,
				usecs_to_jiffies(hlg_config.sampling_rate));
			goto end;
		}
	}

	/* it's will be infinitive looping if call
	 * wakeup_nonboot_cpus() right here. So start
	 * workqueue with very short delay time instead.
	 */
	schedule_delayed_work_on(0, &hlg_work,
		usecs_to_jiffies(1 * HZ));
end:
	mutex_unlock(&hlg_config.timer_mutex);
#endif /* DYNAMIC_HOTPLUG_CPU */
	return ret;
}

/*
 * shmobile_cpufreq_init: initialize the DFS module.
 *
 * Argument:
 *		@policy: the policy will change the frequency.
 *
 * Return:
 *		0: normal initialization
 *		negative: operation fail
 */
#ifndef CPUFREQ_TEST_MODE
static
#endif /* CPUFREQ_TEST_MODE */
int shmobile_cpufreq_init(struct cpufreq_policy *policy)
{
	static unsigned int not_initialized = 1;
	int i = 0;
	int ret = 0;

	/* validate chip revision
	 * -> support revision 2.x and later
	 */
	if (!validate())
		return ret;

	if (!policy)
		return -EINVAL;

	/* init frequency table */
	if (shmobile_chip_rev() == ES_REV_1_0)
		freq_table = main_freqtbl_es1_x;
	else if (shmobile_chip_rev() >= ES_REV_2_0)
		freq_table = main_freqtbl_es2_x;
	else
		return -ENOTSUPP;


	/* for other governors which are used frequency table */
	cpufreq_frequency_table_get_attr(freq_table, policy->cpu);

	/* check whatever the driver has been not_initialized */
	if (!not_initialized) {
		/* the driver has already initialized. */
		ret = cpufreq_frequency_table_cpuinfo(policy, freq_table);
		if (ret)
			goto done;

		policy->cur = the_cpuinfo.freq;
		policy->governor = CPUFREQ_DEFAULT_GOVERNOR;
		policy->cpuinfo.transition_latency = FREQ_TRANSITION_LATENCY;
		/* policy sharing between dual CPUs */
		cpumask_copy(policy->cpus, &cpu_present_map);
		policy->shared_type = CPUFREQ_SHARED_TYPE_ALL;
		goto done;
	}

	spin_lock_init(&the_cpuinfo.lock);
#ifdef CONFIG_EARLYSUSPEND
	register_early_suspend(&shmobile_cpufreq_suspend);
#endif /* CONFIG_EARLYSUSPEND */
	the_cpuinfo.clk_state = MODE_NORMAL;
	the_cpuinfo.scaling_locked = 0;
	the_cpuinfo.highspeed.used = false;
	the_cpuinfo.upper_lowspeed.used = false;
	if (0 != zclk12_flg) {
		the_cpuinfo.freq_max = FREQ_MAX12;
		the_cpuinfo.freq_mid_upper_limit = FREQ_MID_UPPER_LIMIT12;
		the_cpuinfo.freq_mid_lower_limit = FREQ_MID_LOWER_LIMIT12;
		the_cpuinfo.freq_min_upper_limit = FREQ_MIN_UPPER_LIMIT12;
		the_cpuinfo.freq_min_lower_limit = FREQ_MIN_LOWER_LIMIT12;
		the_cpuinfo.freq_suspend = SUSPEND_CPUFREQ12;
	} else {
		the_cpuinfo.freq_max = FREQ_MAX15;
		the_cpuinfo.freq_mid_upper_limit = FREQ_MID_UPPER_LIMIT15;
		the_cpuinfo.freq_mid_lower_limit = FREQ_MID_LOWER_LIMIT15;
		the_cpuinfo.freq_min_upper_limit = FREQ_MIN_UPPER_LIMIT15;
		the_cpuinfo.freq_min_lower_limit = FREQ_MIN_LOWER_LIMIT15;
		the_cpuinfo.freq_suspend = SUSPEND_CPUFREQ15;
	}
	atomic_set(&the_cpuinfo.highspeed.usage_count, 0);
	atomic_set(&the_cpuinfo.upper_lowspeed.usage_count, 0);

	/*
	 * loader had set the frequency to MAX, already.
	 */
	the_cpuinfo.limit_maxfrq = the_cpuinfo.freq_max;
	the_cpuinfo.freq = pm_get_syscpu_frequency();
	ret = cpufreq_frequency_table_cpuinfo(policy, freq_table);
	if (ret)
		goto done;

	policy->cur = the_cpuinfo.freq;
	policy->governor = CPUFREQ_DEFAULT_GOVERNOR;
	policy->cpuinfo.transition_latency = FREQ_TRANSITION_LATENCY;
	/* policy sharing between dual CPUs */
	cpumask_copy(policy->cpus, &cpu_present_map);
	policy->shared_type = CPUFREQ_SHARED_TYPE_ALL;
	not_initialized--;
#ifdef DYNAMIC_HOTPLUG_CPU
	/* dynamic hotplug cpu-core */
	mutex_init(&hlg_config.timer_mutex);
	for (i = 0; i < DEF_MAX_REQ_NR; i++)
		hlg_config.req_his_freq[i] = the_cpuinfo.freq;
	if (0 != zclk12_flg) {
		hlg_config.plug_threshold = DEF_PLUG_THRESHOLD12;
		hlg_config.unplug_threshold = DEF_UNPLUG_THRESHOLD12;
	} else {
		hlg_config.plug_threshold = DEF_PLUG_THRESHOLD15;
		hlg_config.unplug_threshold = DEF_UNPLUG_THRESHOLD15;
	}
#endif /* DYNAMIC_HOTPLUG_CPU */

	for (i = 0; freq_table[i].frequency != CPUFREQ_TABLE_END; i++)
		pr_info("[%2d]:%7u KHz", i, freq_table[i].frequency);
done:
	return ret;
}

static struct cpufreq_driver shmobile_cpufreq_driver = {
	.flags		= CPUFREQ_STICKY,
	.verify		= shmobile_cpufreq_verify,
	.target		= shmobile_cpufreq_target,
	.get		= shmobile_cpufreq_getspeed,
	.init		= shmobile_cpufreq_init,
	.name		= "shmobile"
};

static int shmobile_policy_changed_notifier(struct notifier_block *nb,
			unsigned long type, void *data)
{
	struct cpufreq_policy *policy;

	if (CPUFREQ_NOTIFY != type)
		return 0;
	policy = (struct cpufreq_policy *)data;
	if (policy) {
		if (0 == strcmp(policy->governor->name, "ondemand")) {
			if (STOP_STATE != sampling_flag)
				return 0;
			/* when governor is changed from non-ondemand */
			sampling_flag = INIT_STATE;
			__change_sampling_values();
		} else {
			sampling_flag = STOP_STATE;
		}
	}
	return 0;
}
static struct notifier_block policy_notifier = {
	.notifier_call = shmobile_policy_changed_notifier,
};

/*
 * shmobile_cpu_init: register the cpufreq driver with the cpufreq
 * governor driver.
 *
 * Arguments:
 *		none.
 *
 * Return:
 *		0: normal registration
 *		negative: operation fail
 */
static int __init shmobile_cpu_init(void)
{
	int ret = 0;
	int i = 0;
	int pll0_ratio = 0;
	int arr_size = 0;
#if 0
	unsigned int hw_rev = 0;
#endif
#ifdef DVFS_DEBUG_MODE
	debug = 1;
#else  /* !DVFS_DEBUG_MODE */
	debug = 0;
#endif /* DVFS_DEBUG_MODE */
	/* build frequency table */
#if 0 /* Disable board rev checking */
	hw_rev = u2_get_board_rev();
	pr_info("------hw_rev = %d", hw_rev);
	if (hw_rev >= HWREV_041)
		zclk12_flg = 1;
	else
		zclk12_flg = 0;
#else
#if defined(CONFIG_BOARD_VERSION_V041) && !defined(CPUFREQ_OVERDRIVE)
	zclk12_flg = 1;
#else
	zclk12_flg = 0;
#endif /* CONFIG_BOARD_VERSION_V041 && CPUFREQ_OVERDRIVE */
#endif /* Disable board rev checking */
	if (0 != zclk12_flg) {
		pll0_ratio = PLLx46;
		zdiv_table = zdiv_table12;
		arr_size = (int)ARRAY_SIZE(zdiv_table12);
	} else {
		pll0_ratio = PLLx56;
		zdiv_table = zdiv_table15;
		arr_size = (int)ARRAY_SIZE(zdiv_table15);
	}

	if (pll0_ratio != pm_get_pll_ratio(PLL0)) {
		pr_info("Try to set PLL0 = x%d...", pll0_ratio);
		pm_set_pll_ratio(PLL0, pll0_ratio);
	}
	pr_info("----> PLL0 = x%d", pm_get_pll_ratio(PLL0));
	for (i = 0; i < arr_size; i++) {
		main_freqtbl_es2_x[i].index = i;
		main_freqtbl_es2_x[i].frequency =
		pll0_ratio * EXTAL1 * zdiv_table[i].waveform / 16;
	}
	main_freqtbl_es2_x[i].index = i;
	main_freqtbl_es2_x[i].frequency = CPUFREQ_TABLE_END;

	/* setup clocksuspend */
	ret = pm_setup_clock();
	if (ret)
		return ret;

	/* register cpufreq driver to cpufreq core */
	ret = cpufreq_register_driver(&shmobile_cpufreq_driver);
	if (ret)
		return ret;
	ret = cpufreq_register_notifier(&policy_notifier,
					CPUFREQ_POLICY_NOTIFIER);

	return ret;
}
/*
 * Append SYSFS interface for MAX/MIN frequency limitation control
 * path: /sys/power/{cpufreq_table, cpufreq_limit_max, cpufreq_limit_min}
 *	- cpufreq_table: listup all available frequencies
 *	- cpufreq_limit_max: maximum frequency current supported
 *	- cpufreq_limit_min: minimum frequency current supported
 */
static inline struct attribute *find_attr_by_name(struct attribute **attr,
				const char *name)
{
	/* lookup for an attibute(by name) from attribute list */
	while ((attr) && (*attr)) {
		if (strcmp(name, (*attr)->name) == 0)
			return *attr;
		attr++;
	}

	return NULL;
}
/*
 * show_available_freqs - show available frequencies
 * /sys/power/cpufreq_table
 */
static ssize_t show_available_freqs(struct kobject *kobj,
				struct kobj_attribute *attr, char *buf)
{
	unsigned int prev = 0;
	ssize_t count = 0;
	int i = 0;

	/* show all available freuencies */
	for (i = 0; (freq_table[i].frequency != CPUFREQ_TABLE_END); i++) {
		if (freq_table[i].frequency == prev)
			continue;
		count += sprintf(&buf[count], "%d ", freq_table[i].frequency);
		prev = freq_table[i].frequency;
	}
	count += sprintf(&buf[count], "\n");

	return count;
}
/* for cpufreq_limit_max & cpufreq_limit_min, create a shortcut from
 * default sysfs node (/sys/devices/system/cpu/cpu0/cpufreq/) so bellow
 * mapping table indicate the target name and alias which will be use
 * for creating shortcut.
 */
static struct {
	const char *target;
	const char *alias;
} attr_mapping[] = {
	{"scaling_max_freq", "cpufreq_max_limit"},
	{"scaling_min_freq", "cpufreq_min_limit"}
};

/*
 * cpufreq_max_limit/cpufreq_min_limit - show max/min frequencies
 * this handler is used for both MAX/MIN limit
 */
static ssize_t show_freq(struct kobject *kobj,
	struct kobj_attribute *attr, char *buf)
{
	unsigned int ret = -EINVAL;
	struct cpufreq_policy cur_policy;
	struct kobj_type *ktype = NULL;
	struct attribute *att = NULL;
	int i = 0;

	ret = cpufreq_get_policy(&cur_policy, 0);
	if (ret)
		return -EINVAL;

	ktype = get_ktype(&cur_policy.kobj);
	if (!ktype)
		return -EINVAL;

	for (i = 0; i < ARRAY_SIZE(attr_mapping); i++) {
		if (strcmp(attr->attr.name, attr_mapping[i].alias) == 0) {
			att = find_attr_by_name(ktype->default_attrs,
				attr_mapping[i].target);

			/* found the attibute, pass message to its ops */
			if (att && ktype->sysfs_ops && ktype->sysfs_ops->show)
					return ktype->sysfs_ops->show(
					&cur_policy.kobj, att, buf);
		}
	}

	return -EINVAL;
}

/*
 * cpufreq_max_limit/cpufreq_min_limit - store max/min frequencies
 * this handler is used for both MAX/MIN limit
 */
static ssize_t store_freq(struct kobject *kobj,
	struct kobj_attribute *attr, const char *buf, size_t count)
{
	struct cpufreq_policy *cur_policy = NULL;
	struct cpufreq_policy data;
	unsigned int freq = 0;
	int index = -1;
	int att_id = 0;
	int ret = -EINVAL;
	int i = 0;

	if (strcmp(attr->attr.name, "cpufreq_max_limit") == 0)
		att_id = 1;
	else if (strcmp(attr->attr.name, "cpufreq_min_limit") == 0)
		att_id = 0;
	else
		goto end;

	cur_policy = cpufreq_cpu_get(0);
	if (!cur_policy)
			goto end;

	memcpy(&data, cur_policy, sizeof(struct cpufreq_policy));
	if (sscanf(buf, "%d", &i) != 1)
			goto end;

	/*
	 * if input == -1 then restore original setting
	 * else apply new setting
	 */
	if (i >= 0) {
		if (sscanf(buf, "%d", &freq) != 1)
			goto end;

		if (the_cpuinfo.highspeed.used && att_id)
			freq = the_cpuinfo.limit_maxfrq;
	} else {
		/* restore original value
		 * will request new value which must be smaller than
		 * limit_maxfrq which set by other one.
		 */
		if (att_id)
			freq = the_cpuinfo.limit_maxfrq;
	}

	ret = cpufreq_frequency_table_cpuinfo(&data, freq_table);
	if (ret < 0)
		return ret;

	/* this must be set for cpufreq_update_policy() */
	if (att_id) {
		/* max limit
		 * for searching, need lowwer value compare with
		 * new one then CPUFREQ_RELATION_H will be used
		 */
		if (cpufreq_frequency_table_target(&data, freq_table, freq,
			CPUFREQ_RELATION_H, &index))
			return -EINVAL;
		cur_policy->user_policy.max = freq_table[index].frequency;
	} else {
		/* min limit
		 * for searching, need upper value compare with
		 * new one then CPUFREQ_RELATION_L will be used
		 */
		if (cpufreq_frequency_table_target(&data, freq_table, freq,
			CPUFREQ_RELATION_L, &index))
			return -EINVAL;
		cur_policy->user_policy.min = freq_table[index].frequency;
	}

	/* now, apply new value */
	pr_log("update plicy->max/policy->min\n");
	ret = cpufreq_update_policy(0);
end:
	return ret ? ret : count;
}

static ssize_t show_cur_freq(struct kobject *kobj,
	struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", the_cpuinfo.freq);
}

static struct kobj_attribute cpufreq_table_attribute =
	__ATTR(cpufreq_table, 0444, show_available_freqs, NULL);
static struct kobj_attribute cur_freq_attribute =
	__ATTR(cpufreq_cur_freq, 0444, show_cur_freq, NULL);
static struct kobj_attribute max_limit_attribute =
	__ATTR(cpufreq_max_limit, 0644, show_freq, store_freq);
static struct kobj_attribute min_limit_attribute =
	__ATTR(cpufreq_min_limit, 0644, show_freq, store_freq);
/*
 * Create a group of attributes so that can create and destroy them all
 * at once.
 */
static struct attribute *attrs[] = {
	&min_limit_attribute.attr,
	&max_limit_attribute.attr,
	&cpufreq_table_attribute.attr,
	&cur_freq_attribute.attr,
	NULL,	/* need to NULL terminate the list of attributes */
};

static struct attribute_group attr_group = {
	.attrs = attrs,
};

static int __init shmobile_sysfs_init(void)
{
	/* Create the files associated with power kobject */
	return sysfs_create_group(power_kobj, &attr_group);
}

module_init(shmobile_cpu_init);
late_initcall(shmobile_sysfs_init);

