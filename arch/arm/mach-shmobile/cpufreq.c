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
#include <mach/pm.h>

#ifdef pr_fmt
#undef pr_fmt
#define pr_fmt(fmt) "[DFS] - " fmt
#endif
/* #define CPUFREQ_DEBUG_ENABLE	1 */
#ifdef CPUFREQ_DEBUG_ENABLE
#define pr_log(fmt, ...)	printk(KERN_INFO pr_fmt(fmt), ##__VA_ARGS__)
#else /* !CPUFREQ_DEBUG_ENABLE */
#define pr_log(fmt, ...)
#endif /*   */
/* #define CPUFREQ_TEST_MODE	1 */

/* MAIN Table */
enum cpu_freq_level {
	FREQ_LEV_MAX = 0,
#ifdef SH_CPUFREQ_OVERDRIVE
	FREQ_LEV_HIGH,
#endif /* SH_CPUFREQ_OVERDRIVE */
	FREQ_LEV_MID,
	FREQ_LEV_MIN,
#ifdef SH_CPUFREQ_VERYLOW
	FREQ_LEV_EXMIN,
#endif /* SH_CPUFREQ_VERYLOW */
	FREQ_LEV_NUM
};

/* Clocks State */
enum clock_state {
	MODE_NORMAL = 0,
	MODE_EARLY_SUSPEND,
	MODE_SUSPEND,
	MODE_NUM
};
#define SUSPEND_CPUFREQ FREQ_LEV_MID		/* Suspend */
#define CORESTB_CPUFREQ CPUFREQ_TABLE_END   /* CoreStandby */
#define FREQ_TRANSITION_LATENCY  (CONFIG_SH_TRANSITION_LATENCY * NSEC_PER_USEC)
/* PLL0 */
#define PLL0_RATIO_MIN	35
#define PLL0_RATIO_MAX	56
#define PLL0_MAIN_CLK	26000

#define MAX_ZS_DIVRATE	DIV1_6
#define MAX_HP_DIVRATE	DIV1_12

struct cpufreq_resource {
	bool used;
	atomic_t usage_count;
};

struct shmobile_cpuinfo {
	struct cpufreq_resource upper_lowspeed;
	struct cpufreq_resource highspeed;
	struct cpufreq_policy *policy;
	unsigned int freq;
	unsigned int req_rate[CONFIG_NR_CPUS];
	unsigned int limit_maxfrq;
	int sgx_flg;
	int clk_state;
	int scaling_locked;
	spinlock_t lock;
};

/**************** static ****************/
#ifdef CONFIG_PM_DEBUG
int cpufreq_enabled = 1;
#endif /* CONFIG_PM_DEBUG */
static int target_cpu = -1;
#ifndef CPUFREQ_TEST_MODE
static
#endif /* CPUFREQ_TEST_MODE */
struct shmobile_cpuinfo	the_cpuinfo;
/* ES1.0 */
static struct cpufreq_frequency_table main_freqtbl_es1_x[] = {
	{.index = 0, .frequency = 988000}, /* max		*/
	{.index = 1, .frequency = 494000}, /* mid		*/
	{.index = 2, .frequency = 247000}, /* low		*/
#ifdef SH_CPUFREQ_VERYLOW
	{.index = 3, .frequency = 164666}, /* extra low	*/
#endif /* SH_CPUFREQ_VERYLOW */
	{.index = 4, .frequency =  CPUFREQ_TABLE_END}
};

/* ES2.0 */
static struct cpufreq_frequency_table main_freqtbl_es2_x[] = {
#ifdef SH_CPUFREQ_OVERDRIVE
	{.index = 0, .frequency = 1456000},	/* max			*/
	{.index = 1, .frequency = 1196000},	/* high			*/
	{.index = 2, .frequency =  598000},	/* mid			*/
	{.index = 3, .frequency =  299000},	/* low			*/
#ifdef SH_CPUFREQ_VERYLOW
	{.index = 4, .frequency =  199333},	/* extra low	*/
#endif /* SH_CPUFREQ_VERYLOW */
	{.index = 5, .frequency =  CPUFREQ_TABLE_END}
#else /* !SH_CPUFREQ_OVERDRIVE */
	{.index = 0, .frequency = 1196000},	/* max			*/
	{.index = 1, .frequency =  598000},	/* mid			*/
	{.index = 2, .frequency =  299000},	/* low			*/
#ifdef SH_CPUFREQ_VERYLOW
	{.index = 3, .frequency =  199333},	/* extra low	*/
#endif /* SH_CPUFREQ_VERYLOW */
	{.index = 4, .frequency =  CPUFREQ_TABLE_END}
#endif /* SH_CPUFREQ_OVERDRIVE */
};

/* divrate table */
static int main_divtable[] = {
#ifdef SH_CPUFREQ_OVERDRIVE
	DIV1_1, DIV1_1, DIV1_2, DIV1_4
#else /* !SH_CPUFREQ_OVERDRIVE */
	DIV1_1, DIV1_2, DIV1_4
#endif /* SH_CPUFREQ_OVERDRIVE */
#ifdef SH_CPUFREQ_VERYLOW
	, DIV1_6
#endif /* SH_CPUFREQ_VERYLOW */
};

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
	struct cpufreq_frequency_table *freq_table = NULL;
	int idx = 0;

	/* get the frequency table */
	freq_table = cpufreq_frequency_get_table(the_cpuinfo.policy->cpu);
	if (!freq_table)
		return -EINVAL;

	cpufreq_frequency_table_target(the_cpuinfo.policy, freq_table,
		freq, CPUFREQ_RELATION_H, &idx);
	pr_log("%s()[%d]: got frequency level<%u>\n", __func__, __LINE__, idx);
	return idx;
}

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
	int idx = __to_freq_level(freq);

	if ((idx >= ARRAY_SIZE(main_divtable)) || (idx < 0))
		return -EINVAL;

	return main_divtable[idx];
}

/*
 * __clk_get_rate: get the set of clocks
 *
 * Argument:
 *		@clk_state: clock state
 *		@sgx_type: power state of SGX
 *		@freq: current frequency
 *
 * Return:
 *     address of element of array clocks_div_data
 *     NULL: if input parameters are invaid
 */
#ifndef CPUFREQ_TEST_MODE
static
#endif /* CPUFREQ_TEST_MODE */
int __clk_get_rate(struct clk_rate *rate, int clk_state,
	int sgx_type, unsigned int freq)
{
	int level = (int)FREQ_LEV_MAX;
	int freq_mode = 0;
	int lv_num = FREQ_LEV_NUM;
	int sgx_num = CPUFREQ_SGXNUM;
	int ret = 0;

	/* Check for invalid parameters */
	if ((clk_state < 0) || (clk_state >= MODE_NUM)) {
		/* invalid clk_state */
		pr_err("%s()[%d]: error! invalid clock ratio\n",
			__func__, __LINE__);
		return -EINVAL;
	}

	if ((shmobile_chip_rev() < ES_REV_2_0) &&
		((sgx_type < 0) || (sgx_type >= CPUFREQ_SGXNUM)))
		return -EINVAL;

	level = __to_freq_level(freq);
	if (FREQ_LEV_NUM == level) {
		/* Invalid frequency level */
		pr_err("%s()[%d]: error! invalid frequency level\n",
			__func__, __LINE__);
		return -EINVAL;
	}

	/* Get the frequency mode */
	if (MODE_SUSPEND == clk_state) {
		freq_mode = 0;
	} else {
		/* ES1.x */
		if (shmobile_chip_rev() < ES_REV_2_0) {
			freq_mode = clk_state * sgx_num * lv_num +
				sgx_type * lv_num + level + 1;
		} else {
			/* ES2.x */
			if (MODE_EARLY_SUSPEND == clk_state) {
#ifdef SH_CPUFREQ_VERYLOW
				if (level == FREQ_LEV_EXMIN)
					level = FREQ_LEV_MIN;
#endif /* SH_CPUFREQ_VERYLOW */
#ifdef SH_CPUFREQ_OVERDRIVE
				freq_mode = clk_state * lv_num + level;
#else /* !SH_CPUFREQ_OVERDRIVE */
				freq_mode = clk_state * lv_num + level + 1;
#endif /* SH_CPUFREQ_OVERDRIVE */
			} else {
				freq_mode = clk_state * lv_num + level + 1;
			}
		}
	}

	if (!rate) {
		pr_err("%s()[%d]: error! out-of-memory\n",
			__func__, __LINE__);
		return -ENOMEM;
	}

	pr_log("%s()[%d]: freq mode.%d\n", __func__, __LINE__, freq_mode);
	if (pm_get_clock_mode(freq_mode, rate)) {
		pr_err("%s()[%d]: error! get clock mode<%d>\n",
			__func__, __LINE__, freq_mode);
		return -EINVAL;
	}

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
 * __update_policy: update policy for all online except current one
 *
 * Argument:
 *		None
 *
 * Return:
 *		none
 */
static void __update_policy(void)
{
	struct cpufreq_policy policy;
	const char *gov = "conservative";
	int i = 0;
	
	/* suspend mode, only CPU0 alive */
	if (the_cpuinfo.clk_state == MODE_SUSPEND)
		return;
	
	/* notify all others about frequency change, reuqest them allocmem
	 * update current frequency
	 */
	for (i = 0; i < num_online_cpus(); i++) {
		if (!cpufreq_get_policy(&policy, i)) {
			/* only conservative governor */
			if (!strcmp(gov, policy.governor->name) &&
				(target_cpu != i))
				(void)cpufreq_update_policy(i);
		}
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
	int div_rate = (int)DIV1_1;
	int pllratio = PLLx38;
	int ret = 0;

	if (freq == the_cpuinfo.freq) {
		pr_log("%s()[%d]: frequency not changed, ret<%d>\n",
			__func__, __LINE__, ret);
		return ret;
	}

	/* for overdrive mode, pll0 is x56, other is x46 */
	if (shmobile_chip_rev() >= ES_REV_2_0) {
#ifdef SH_CPUFREQ_OVERDRIVE
		pllratio = PLLx56;
#else /* !SH_CPUFREQ_OVERDRIVE */
		pllratio = PLLx46;
#endif /* SH_CPUFREQ_OVERDRIVE */
	}

	/* change PLL0 if need */
	if ((__to_freq_level(freq) == FREQ_LEV_MAX) &&
		(pm_get_pll_ratio(PLL0) != pllratio)) {
		ret = pm_set_pll_ratio(PLL0, pllratio);
		if (ret) {
			pr_err("%s()[%d]: error<%d>! set pll0 ratio<%d>\n",
				__func__, __LINE__, ret, pllratio);
			return ret;
		}
	}

	div_rate = __to_div_rate(freq);
	if (div_rate < 0) {
		pr_err("%s()[%d]: error<%d>! frequency not support\n",
			__func__, __LINE__, -EINVAL);
		return -EINVAL;
	}

	ret = pm_set_syscpu_frequency(div_rate);
	if (ret) {
		pr_err("%s()[%d]: error<%d>! set divrate<0x%x>\n",
			__func__, __LINE__, ret, div_rate);
	} else {
		pr_info("SYS-CPU<%d> clk[%uKHz->%uKHz]\n",
			((target_cpu >= 0) ? target_cpu : smp_processor_id()),
			the_cpuinfo.freq, freq);
		the_cpuinfo.freq = freq;

		/* update all cpus with new frequency */
		__update_policy();
	}

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
#ifdef CONFIG_PM_DEBUG
	if (!cpufreq_enabled) {
		/* runtime disabled */
		return;
	}
#endif /* CONFIG_PM_DEBUG */
	pr_log("%s()[%d]: start scaling/begin, cpufreq flag<%d>,\n",
		__func__, __LINE__, the_cpuinfo.highspeed.used);
	spin_lock(&the_cpuinfo.lock);

	if (atomic_dec_and_test(&the_cpuinfo.highspeed.usage_count))
		the_cpuinfo.highspeed.used = false;

	spin_unlock(&the_cpuinfo.lock);
	/* start governor */
	if (!the_cpuinfo.highspeed.used) {
		/* update all cpus with new frequency */
		__update_policy();
	}
	pr_log("%s()[%d]: start scaling/end, cpufreq flag<%d>,\n",
		__func__, __LINE__, the_cpuinfo.highspeed.used);
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
	struct clk_rate clk_div;
	struct cpufreq_frequency_table *freq_table = NULL;
	unsigned int freq_old = 0;
	unsigned int freq_new = 0;
	int ret = 0;

#ifdef CONFIG_PM_DEBUG
	if (!cpufreq_enabled) {
		/* runtime disabled */
		return ret;
	}
#endif /* CONFIG_PM_DEBUG */
	pr_log("%s()[%d]: stop scaling/begin, cpufreq flag<%d>,\n",
		__func__, __LINE__, the_cpuinfo.highspeed.used);
	spin_lock(&the_cpuinfo.lock);

	/* get the frequency table */
	freq_table = cpufreq_frequency_get_table(the_cpuinfo.policy->cpu);
	if (!freq_table) {
		spin_unlock(&the_cpuinfo.lock);
		return -ENOMEM;
	}
	/*
	 * emergency case, CPU is hot or system going to suspend,
	 * we do not allow user to change frequency now
	 */
	if ((the_cpuinfo.limit_maxfrq != freq_table[FREQ_LEV_MAX].frequency) ||
		(MODE_SUSPEND == the_cpuinfo.clk_state)) {
		spin_unlock(&the_cpuinfo.lock);
		pr_err("%s()[%d] can not change frequency right now!\n",
			__func__, __LINE__);
		return -EBUSY;
	}

	if (!atomic_read(&the_cpuinfo.highspeed.usage_count)) {
		the_cpuinfo.highspeed.used = true;
		/*
		 * skip setting hardware if the current SYS-CPU freq is
		 * MAX already
		 */
		if (__to_freq_level(the_cpuinfo.freq) == FREQ_LEV_MAX) {
			atomic_inc(&the_cpuinfo.highspeed.usage_count);
			spin_unlock(&the_cpuinfo.lock);
			return ret;
		}

		freq_old = the_cpuinfo.freq;
		freq_new = freq_table[FREQ_LEV_MAX].frequency;
		ret = __set_rate(freq_new);
		if (ret < 0) {
			pr_err("%s()[%d]: error<%d>! set cpu frequency[%u]\n",
				__func__, __LINE__, ret, freq_new);
			the_cpuinfo.highspeed.used = false;
			spin_unlock(&the_cpuinfo.lock);
			return ret;
		} else {
			__notify_all_cpu(freq_old, freq_new,
				CPUFREQ_POSTCHANGE);

			ret = __clk_get_rate(&clk_div, the_cpuinfo.clk_state,
				the_cpuinfo.sgx_flg, freq_new);
			if (!ret)
				ret = pm_set_clocks(clk_div);
		}
	}

	atomic_inc(&the_cpuinfo.highspeed.usage_count);
	spin_unlock(&the_cpuinfo.lock);
	pr_log("%s()[%d]: stop scaling[cnd:%d]/end,cpufreq flag<%d>, ret<%d>\n",
		__func__, __LINE__,
		atomic_read(&the_cpuinfo.highspeed.usage_count),
		the_cpuinfo.highspeed.used, ret);

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
	struct clk_rate clk_div;
	struct cpufreq_frequency_table *freq_table = NULL;
	unsigned int freq_old = 0;
	unsigned int freq_new = 0;
	int ret = 0;
#ifdef CONFIG_PM_DEBUG
	if (!cpufreq_enabled) {
		/* runtime disabled */
		return;
	}
#endif /* CONFIG_PM_DEBUG */
	pr_log("%s()[%d]: disable low-level(MIN)/begin, upper lowspeed<%d>,\n",
		__func__, __LINE__, the_cpuinfo.upper_lowspeed.used);
	spin_lock(&the_cpuinfo.lock);

	/* get the frequency table */
	freq_table = cpufreq_frequency_get_table(the_cpuinfo.policy->cpu);
	if (!freq_table) {
		spin_unlock(&the_cpuinfo.lock);
		goto end;
	}
	/*
	 * emergency case, CPU is hot, we do not allow user to change
	 * frequency now
	 */
	if (the_cpuinfo.limit_maxfrq <= freq_table[FREQ_LEV_MIN].frequency) {
		spin_unlock(&the_cpuinfo.lock);
		pr_err("%s()[%d] CPU hot, can not change frequency right now!\n",
			__func__, __LINE__);
		goto end;
	}

	if (!atomic_read(&the_cpuinfo.upper_lowspeed.usage_count)) {
		the_cpuinfo.upper_lowspeed.used = true;
		if (__to_freq_level(the_cpuinfo.freq) >= FREQ_LEV_MIN) {
			freq_old = the_cpuinfo.freq;
			freq_new = freq_table[FREQ_LEV_MID].frequency;

			ret = __set_rate(freq_new);
			if (ret < 0) {
				pr_err("%s()[%d]: error<%d>! set cpu frequency[%u]\n",
					__func__, __LINE__, ret, freq_new);
				the_cpuinfo.upper_lowspeed.used = false;
				spin_unlock(&the_cpuinfo.lock);
				goto end;
			} else {
				__notify_all_cpu(freq_old, freq_new,
					CPUFREQ_POSTCHANGE);

				ret = __clk_get_rate(&clk_div,
					the_cpuinfo.clk_state,
					the_cpuinfo.sgx_flg, the_cpuinfo.freq);
				if (!ret)
					ret = pm_set_clocks(clk_div);
			}
		}
	}

	atomic_inc(&the_cpuinfo.upper_lowspeed.usage_count);
	spin_unlock(&the_cpuinfo.lock);
end:
	pr_log("%s()[%d]: disable low-level(MIN)/end, upper lowspeed<%d>\n",
		__func__, __LINE__, the_cpuinfo.upper_lowspeed.used);
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
#ifdef CONFIG_PM_DEBUG
	if (!cpufreq_enabled) {
		/* runtime disabled */
		return;
	}
#endif /* CONFIG_PM_DEBUG */
	pr_log("%s()[%d]: enable low-level(MIN)/begin, highspeed<%d>,\n",
		__func__, __LINE__, the_cpuinfo.upper_lowspeed.used);
	spin_lock(&the_cpuinfo.lock);

	if (atomic_dec_and_test(&the_cpuinfo.upper_lowspeed.usage_count))
		the_cpuinfo.upper_lowspeed.used = false;

	spin_unlock(&the_cpuinfo.lock);
	pr_log("%s()[%d]: enable low-level(MIN)/end, highspeed<%d>\n",
		__func__, __LINE__,	the_cpuinfo.upper_lowspeed.used);
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
	struct cpufreq_frequency_table *freq_table = NULL;
	int idx = CORESTB_CPUFREQ;
	int ret = 0;

#ifdef CONFIG_PM_DEBUG
	if (!cpufreq_enabled) {
		/* runtime disabled */
		return ret;
	}
#endif /* CONFIG_PM_DEBUG */
	/* pr_log("%s()[%d]: cpufreq standby/begin\n", __func__, __LINE__); */
	spin_lock(&the_cpuinfo.lock);

	/* get the frequency table */
	freq_table = cpufreq_frequency_get_table(the_cpuinfo.policy->cpu);
	if (!freq_table) {
		spin_unlock(&the_cpuinfo.lock);
		return -EINVAL;
	}

	if (atomic_read(&the_cpuinfo.highspeed.usage_count)) {
		spin_unlock(&the_cpuinfo.lock);
		/* pr_log("%s()[%d]: cpufreq standby/end, ret<%d>\n",
		 *	__func__, __LINE__, ret);
		 */
		return ret;
	}

	if (idx != CPUFREQ_TABLE_END) {
		/*
		 * emergency case, CPU is hot, we do not allow user to change
		 * frequency now
		 */
		if (the_cpuinfo.limit_maxfrq <= freq_table[idx].frequency) {
			spin_unlock(&the_cpuinfo.lock);
			pr_err("%s()[%d] CPU hot, can not change frequency right now!\n",
				__func__, __LINE__);
			return ret;
		}

		ret = __set_rate(freq_table[idx].frequency);
		if (ret < 0)
			pr_err("%s()[%d]: error<%d>! set cpu frequency[%u]\n",
				__func__, __LINE__, ret,
				freq_table[idx].frequency);
	}
	spin_unlock(&the_cpuinfo.lock);
	/* pr_log("%s()[%d]: cpufreq standby/end, ret<%d>\n",
	 *	__func__, __LINE__, ret);
	 */

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
	struct clk_rate clk_div;
	struct cpufreq_frequency_table *freq_table = NULL;
	unsigned int freq_old = 0;
	unsigned int freq_new = 0;
	int ret = 0;

#ifdef CONFIG_PM_DEBUG
	if (!cpufreq_enabled) {
		/* runtime disabled */
		return ret;
	}
#endif /* CONFIG_PM_DEBUG */
	pr_log("%s()[%d]: suspend cpufreq/begin\n", __func__, __LINE__);
	spin_lock(&the_cpuinfo.lock);

	/* get the frequency table */
	freq_table = cpufreq_frequency_get_table(the_cpuinfo.policy->cpu);
	if (!freq_table) {
		spin_unlock(&the_cpuinfo.lock);
		return -EINVAL;
	}
	/*
	 * emergency case, CPU is hot, we do not allow user to change
	 * frequency now
	 */
	if (the_cpuinfo.limit_maxfrq < freq_table[SUSPEND_CPUFREQ].frequency) {
		spin_unlock(&the_cpuinfo.lock);
		pr_err("%s()[%d] CPU hot, can not change frequency right now!\n",
			__func__, __LINE__);
		return -EBUSY;
	}

	the_cpuinfo.clk_state = MODE_SUSPEND;
	freq_old = the_cpuinfo.freq;
	/* Get the clock value for suspend state */
	freq_new = freq_table[SUSPEND_CPUFREQ].frequency;
	/* Change SYS-CPU frequency */
	ret = __set_rate(freq_new);
	if (ret < 0) {
		pr_err("%s()[%d]: error<%d>! set cpu frequency[%u]\n",
			__func__, __LINE__, ret, freq_new);
		spin_unlock(&the_cpuinfo.lock);
		return -ETIMEDOUT;
	}
	/* notify to the system */
	__notify_all_cpu(freq_old, freq_new,
		CPUFREQ_POSTCHANGE);

	ret = __clk_get_rate(&clk_div, the_cpuinfo.clk_state,
		the_cpuinfo.sgx_flg, the_cpuinfo.freq);
	if (!ret) {
		ret = pm_set_clocks(clk_div);
		if (ret < 0) {
			pr_err("%s()[%d]: error<%d>! set clocks\n",
				__func__, __LINE__, ret);
			ret = -ETIMEDOUT;
		}
	}
	spin_unlock(&the_cpuinfo.lock);
	pr_log("%s()[%d]: suspend cpufreq/end, ret<%d>\n",
		__func__, __LINE__, ret);
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
	struct clk_rate clk_div;
	struct cpufreq_frequency_table *freq_table = NULL;
	unsigned int freq_old = 0;
	unsigned int freq_new = 0;
	int ret = 0;

#ifdef CONFIG_PM_DEBUG
	if (!cpufreq_enabled) {
		/* runtime disabled */
		return ret;
	}
#endif /* CONFIG_PM_DEBUG */
	pr_log("%s()[%d]: resume from suspend/begin\n", __func__, __LINE__);
	spin_lock(&the_cpuinfo.lock);

	/* get the frequency table */
	freq_table = cpufreq_frequency_get_table(the_cpuinfo.policy->cpu);
	if (!freq_table) {
		spin_unlock(&the_cpuinfo.lock);
		return -EINVAL;
	}

	/* change to earlysuspend mode */
	the_cpuinfo.clk_state = MODE_EARLY_SUSPEND;

	if ((the_cpuinfo.highspeed.used) &&
		(the_cpuinfo.limit_maxfrq >=
		freq_table[FREQ_LEV_MAX].frequency)) {
		freq_old = the_cpuinfo.freq;
		freq_new = freq_table[FREQ_LEV_MAX].frequency;
		ret = __set_rate(freq_new);
		if (ret < 0) {
			pr_err("%s()[%d]: error<%d>! set cpu frequency<%u>/end\n",
				__func__, __LINE__, ret, freq_new);
			spin_unlock(&the_cpuinfo.lock);
			return -ETIMEDOUT;
		}
		/* notify to the system about the frequency change */
		__notify_all_cpu(freq_old, freq_new,
			CPUFREQ_POSTCHANGE);
	}

	ret = __clk_get_rate(&clk_div, the_cpuinfo.clk_state,
		the_cpuinfo.sgx_flg, the_cpuinfo.freq);
	if (!ret) {
		ret = pm_set_clocks(clk_div);
		if (ret < 0) {
			pr_err("%s()[%d]: error<%d>! set clocks/end\n",
				__func__, __LINE__, ret);
			ret = -ETIMEDOUT;
		}
	}
	spin_unlock(&the_cpuinfo.lock);
	pr_log("%s()[%d]: resume from suspend/end, ret<%d>\n",
		__func__, __LINE__, ret);
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

#ifdef CONFIG_PM_DEBUG
	if (!cpufreq_enabled) {
		/* runtime disabled */
		return ret;
	}
#endif /* CONFIG_PM_DEBUG */
	spin_lock(&the_cpuinfo.lock);
	the_cpuinfo.sgx_flg = flag;
	spin_unlock(&the_cpuinfo.lock);
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
#ifdef CONFIG_PM_DEBUG
	if (!cpufreq_enabled) {
		/* runtime disabled */
		return;
	}
#endif /* CONFIG_PM_DEBUG */
	pr_log("%s()[%d]: begin\n", __func__, __LINE__);
	spin_lock(&the_cpuinfo.lock);
	the_cpuinfo.scaling_locked = (enabled) ? 0 : 1;
	spin_unlock(&the_cpuinfo.lock);
	pr_log("%s()[%d]: end/scaling_locked<%d>\n", __func__, __LINE__,
		the_cpuinfo.scaling_locked);
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

#ifdef CONFIG_PM_DEBUG
	if (!cpufreq_enabled) {
		/* runtime disabled */
		return ret;
	}
#endif /* CONFIG_PM_DEBUG */
	spin_lock(&the_cpuinfo.lock);
	if (set_max) {
		ret = __clk_get_rate(&clk_div, the_cpuinfo.clk_state,
				the_cpuinfo.sgx_flg, the_cpuinfo.freq);
		if (ret) {
			spin_unlock(&the_cpuinfo.lock);
			return ret;
		}
		/* restore to max */
			clk_div.zs_clk = MAX_ZS_DIVRATE;
			clk_div.hp_clk = MAX_HP_DIVRATE;

		ret = pm_set_clocks(clk_div);
		if (!ret)
			ret = pm_disable_clock_change(ZSCLK | HPCLK);
	}
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
	struct clk_rate clk_div;
	int ret = 0;

#ifdef CONFIG_PM_DEBUG
	if (!cpufreq_enabled) {
		/* runtime disabled */
		return;
	}
#endif /* CONFIG_PM_DEBUG */
	spin_lock(&the_cpuinfo.lock);
	ret = pm_enable_clock_change(ZSCLK | HPCLK);	
	/* restore to normal operation */
	if (!ret) {
	ret = __clk_get_rate(&clk_div, the_cpuinfo.clk_state,
			the_cpuinfo.sgx_flg, the_cpuinfo.freq);
	if (ret) {
		spin_unlock(&the_cpuinfo.lock);
			pr_err("%s()[%d]: __clk_get_rate fail\n",
				__func__, __LINE__);
		return;
	}

	ret = pm_set_clocks(clk_div);
	}

	spin_unlock(&the_cpuinfo.lock);
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
	struct cpufreq_frequency_table *freq_table = NULL;

	struct clk_rate clk_div;
	int ret = 0;

#ifdef CONFIG_PM_DEBUG
	if (!cpufreq_enabled) {
		/* runtime disabled */
		return ret;
	}
#endif /* CONFIG_PM_DEBUG */
	pr_log("%s()[%d]: begin\n", __func__, __LINE__);
	spin_lock(&the_cpuinfo.lock);

	/* get the frequency table */
	freq_table = cpufreq_frequency_get_table(the_cpuinfo.policy->cpu);
	if (!freq_table) {
		spin_unlock(&the_cpuinfo.lock);
		return -EINVAL;
	}

	switch (max) {
	case LIMIT_NONE:
		the_cpuinfo.limit_maxfrq = freq_table[FREQ_LEV_MAX].frequency;
		/*
		 * user may call stop before
		 */
		if (the_cpuinfo.highspeed.used) {
			ret = __set_rate(the_cpuinfo.limit_maxfrq);
			if (ret) {
			spin_unlock(&the_cpuinfo.lock);
			return ret;
		}
			ret = __clk_get_rate(&clk_div, the_cpuinfo.clk_state,
			the_cpuinfo.sgx_flg, the_cpuinfo.freq);

			if (!ret) {
				ret = pm_set_clocks(clk_div);
				if (ret)
					pr_log("[%s](%d) error! ret:%d",
						__func__, __LINE__,	ret);
			}

			goto set_end;
		}
		break;
	case LIMIT_MID:
		the_cpuinfo.limit_maxfrq = freq_table[FREQ_LEV_MID].frequency;
		break;
	case LIMIT_LOW:
		the_cpuinfo.limit_maxfrq = freq_table[FREQ_LEV_MIN].frequency;
		break;
	default:
		return -EINVAL;
	}

	/*
	 * current frequency may upper limitation
	 */
	if (the_cpuinfo.limit_maxfrq < the_cpuinfo.freq) {
		ret = __set_rate(the_cpuinfo.limit_maxfrq);
		if (ret) {
			spin_unlock(&the_cpuinfo.lock);
			return ret;
		}

		ret = __clk_get_rate(&clk_div, the_cpuinfo.clk_state,
			the_cpuinfo.sgx_flg, the_cpuinfo.freq);

		if (!ret) {
			ret = pm_set_clocks(clk_div);
			if (ret < 0)
				pr_log("[%s](%d) pm_set_clocks() err ret:%d",
					__func__, __LINE__,	ret);
		}
	} else {
		pr_log("%s()[%d]: max<%u>, curr<%u> not change\n",
			__func__, __LINE__, the_cpuinfo.limit_maxfrq,
			the_cpuinfo.freq);
	}

set_end:
	spin_unlock(&the_cpuinfo.lock);
	pr_log("%s()[%d]: end\n", __func__, __LINE__);
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
	
	if(!is_enable) {
		if(!cpufreq_enabled) {
			pr_log("%s()[%d]: cpufreq is disabled already\n",
			__func__, __LINE__);
			return 0;
		}
		
		ret = stop_cpufreq();
		if(ret) {
			pr_err("%s()[%d]: error<%d>! stop_cpufreq \n",
				__func__, __LINE__, ret);
			return ret;
		}
		
		spin_lock(&the_cpuinfo.lock);
		cpufreq_enabled = 0;
		spin_unlock(&the_cpuinfo.lock);
		pr_log("%s()[%d]: cpufreq is disabled\n", __func__, __LINE__);
	} else {
		if(cpufreq_enabled) {
			pr_log("%s()[%d]: cpufreq is enabled already\n",
				__func__, __LINE__);
			return ret;
		} else {
			spin_lock(&the_cpuinfo.lock);
			cpufreq_enabled = 1;
			spin_unlock(&the_cpuinfo.lock);	
			
			start_cpufreq();		
			pr_log("%s()[%d]: cpufreq is enabled\n",
				__func__, __LINE__);
		}
	}
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
	struct clk_rate clk_div;
	int ret = 0;

#ifdef CONFIG_PM_DEBUG
	if (!cpufreq_enabled) {
		/* runtime disabled */
		return;
	}
#endif /* CONFIG_PM_DEBUG */
	pr_log("%s()[%d]: begin\n", __func__, __LINE__);
	spin_lock(&the_cpuinfo.lock);

	the_cpuinfo.clk_state = MODE_EARLY_SUSPEND;
	ret = __clk_get_rate(&clk_div, the_cpuinfo.clk_state,
		the_cpuinfo.sgx_flg, the_cpuinfo.freq);

	if (!ret) {
		ret = pm_set_clocks(clk_div);
		if (ret < 0)
			pr_log("[%s](%d) pm_set_clocks() err ret:%d",
				__func__, __LINE__,	ret);
	} else {
		pr_err("[%s](%d) __clk_get_rate() err", __func__, __LINE__);
	}
	spin_unlock(&the_cpuinfo.lock);
	pr_log("%s()[%d]: end\n", __func__, __LINE__);
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
	struct clk_rate clk_div;
	int ret = 0;

#ifdef CONFIG_PM_DEBUG
	if (!cpufreq_enabled) {
		/* runtime disabled */
		return;
	}
#endif /* CONFIG_PM_DEBUG */
	pr_log("%s()[%d]: begin\n", __func__, __LINE__);
	spin_lock(&the_cpuinfo.lock);
	the_cpuinfo.clk_state = MODE_NORMAL;
	ret = __clk_get_rate(&clk_div, the_cpuinfo.clk_state,
		the_cpuinfo.sgx_flg, the_cpuinfo.freq);

	if (!ret) {
		ret = pm_set_clocks(clk_div);
		if (ret < 0) {
			pr_log("[%s](%d) pm_set_clocks() err ret:%d",
				__func__, __LINE__, ret);
		}
	} else {
		pr_err("[%s](%d) __clk_get_rate() err", __func__, __LINE__);
	}
	spin_unlock(&the_cpuinfo.lock);
	pr_log("%s()[%d]: end\n", __func__, __LINE__);
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
	struct cpufreq_frequency_table *freq_table = NULL;

	/* get the frequency table */
	freq_table = cpufreq_frequency_get_table(the_cpuinfo.policy->cpu);
	if (!freq_table)
		return -EINVAL;

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
	pr_log("%s()[%d]: CPU[%d], freq[%u]\n",
		__func__, __LINE__, cpu, the_cpuinfo.freq);
	return the_cpuinfo.freq;
}
#define DOWN_THRESHOLD			90
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
	struct cpufreq_frequency_table *freq_table = NULL;
	struct cpufreq_freqs freqs;
	struct clk_rate clk_div;
	struct cpufreq_frequency_table *ptr = NULL;
	unsigned int freq = ~0;
	int ret = 0;
	int seleted_level = 0;
	int cpu = 0;

#ifdef CONFIG_PM_DEBUG
	if (!cpufreq_enabled) {
		/* runtime disabled */
		return ret;
	}
#endif /* CONFIG_PM_DEBUG */
	/* pr_log("%s()[%d]: CPU<%d> target<%u>/begin\n", __func__, __LINE__,
		policy->cpu, target_freq); */
	spin_lock(&the_cpuinfo.lock);

	/* get the frequency table */
	freq_table = cpufreq_frequency_get_table(policy->cpu);
	if (!freq_table) {
		spin_unlock(&the_cpuinfo.lock);
		return -EINVAL;
	}

	the_cpuinfo.req_rate[policy->cpu] = target_freq;
	/* only reduce the CPU frequency if all CPUs need to reduce */
	for_each_online_cpu(cpu) {
		if (cpu != policy->cpu)
			target_freq = max(the_cpuinfo.req_rate[cpu],
				target_freq);
	}

	freqs.cpu = policy->cpu;
	freqs.old = the_cpuinfo.freq;
	seleted_level = FREQ_LEV_NUM - 1;
	if (shmobile_chip_rev() <= ES_REV_1_0) {
#ifdef SH_CPUFREQ_VERYLOW
		/* verrylow must not be set in ES1.x */
		seleted_level = FREQ_LEV_NUM - 2;
#endif /* SH_CPUFREQ_VERYLOW */
	}

	while (seleted_level > FREQ_LEV_MAX) {
		ptr = &freq_table[seleted_level - 1];
		freq = freq_table[seleted_level].frequency;
		if (target_freq <= ptr->frequency * DOWN_THRESHOLD / 100)
			goto next;
		seleted_level--;
	}

	if (target_freq < freq_table[FREQ_LEV_MAX].frequency) {
		freq = the_cpuinfo.freq;
		goto next;
	}

	freq = freq_table[FREQ_LEV_MAX].frequency;
next:
	if ((the_cpuinfo.upper_lowspeed.used) &&
		(freq <= freq_table[FREQ_LEV_MIN].frequency)) {
		freq = freq_table[FREQ_LEV_MID].frequency;
		freqs.new = freq_table[FREQ_LEV_MID].frequency;
	}

	/*
	 * user set max frequency level but we still not change the
	 * main frequency table so we skip here
	 */
	if ((the_cpuinfo.freq == freq) ||
		(freq > the_cpuinfo.limit_maxfrq) ||
		(the_cpuinfo.highspeed.used) ||
		(the_cpuinfo.scaling_locked ||
		(MODE_SUSPEND == the_cpuinfo.clk_state))) {
		spin_unlock(&the_cpuinfo.lock);
		pr_log("%s()[%d]: the frequency is not changed. ret<%d>\n",
				__func__, __LINE__, ret);
		return ret;
	}

	target_cpu = policy->cpu;
	ret = __set_rate(freq);
	target_cpu = -1;
	if (ret < 0) {
		pr_err("%s()[%d]: error<%d>! set cpu frequency<%u>/end\n",
		__func__, __LINE__, ret, freq);
		spin_unlock(&the_cpuinfo.lock);
		return ret;
	}

	/* only do when frequency is really changed */
	if (freq == the_cpuinfo.freq) {
		__notify_all_cpu(freqs.old, freq,
			CPUFREQ_POSTCHANGE);
		ret = __clk_get_rate(&clk_div, the_cpuinfo.clk_state,
			the_cpuinfo.sgx_flg, the_cpuinfo.freq);
		if (!ret)
			ret = pm_set_clocks(clk_div);

		pr_log("%s()[%d]: pm_set_clocks(), ret<%d>\n",
			__func__, __LINE__, ret);
	}

	spin_unlock(&the_cpuinfo.lock);
	pr_log("%s()[%d]: CPU<%d> target<%u>, set<%u>/end, ret<%d>\n",
		__func__, __LINE__, policy->cpu, target_freq, freq, ret);

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
	struct cpufreq_frequency_table *freq_table = NULL;
	unsigned int freq = 0;
	unsigned int stc_val = 0;
	static unsigned int init_flag = 1;
	int i = 0;
	int ret = 0;

#ifdef CONFIG_PM_DEBUG
	if (!cpufreq_enabled) {
		/* runtime disabled */
		return ret;
	}
#endif /* CONFIG_PM_DEBUG */
	pr_info("%s()[%d]: init cpufreq driver/start\n", __func__, __LINE__);
	if (!policy)
		return -EINVAL;

	if (shmobile_chip_rev() <= ES_REV_1_0) {
#ifdef SH_CPUFREQ_OVERDRIVE
		/* ES1.x does not support OVERDRIVE mode */
		return -ENOTSUPP;
#endif /* SH_CPUFREQ_OVERDRIVE */
		freq_table = main_freqtbl_es1_x;
	} else if (shmobile_chip_rev() >= ES_REV_2_0) {
		freq_table = main_freqtbl_es2_x;
	} else {
		return -ENOTSUPP;
	}

	/* for other governors which are used frequency table */
	cpufreq_frequency_table_get_attr(freq_table, policy->cpu);

	/* check whatever the driver has been initialized */
	if (0 >= init_flag) { /* The driver has already initialized. */
		pr_log("%s()[%d]: the driver has already initialized\n",
			__func__, __LINE__);
		ret = cpufreq_frequency_table_cpuinfo(policy, freq_table);
		if (ret < 0) {
			pr_err("%s()[%d]: error<%d>! the main frequency table is invalid!",
				__func__, __LINE__, ret);
			return ret;
		}
		policy->cur = the_cpuinfo.freq;
		policy->governor = CPUFREQ_DEFAULT_GOVERNOR;
		policy->cpuinfo.transition_latency = FREQ_TRANSITION_LATENCY;
		pr_log("%s()[%d]: init cpufreq driver/end, current freq<%u>\n",
			__func__, __LINE__, the_cpuinfo.freq);
		return ret;
	}
#ifdef CONFIG_EARLYSUSPEND
	register_early_suspend(&shmobile_cpufreq_suspend);
#endif /* CONFIG_EARLYSUSPEND */
	if (ret < 0)
		pr_err("%s()[%d]: error<%d>! can not set PLL0 ratio<%u>",
			__func__, __LINE__,	ret, stc_val);

	stc_val = pm_get_pll_ratio(PLL0);
	if ((stc_val < PLL0_RATIO_MIN) || (stc_val > PLL0_RATIO_MAX)) {
		pr_err("%s()[%d]: error<%d>! STC<0x%x> supported out-of-range\n",
			__func__, __LINE__,	-EINVAL, stc_val);
		return -EINVAL;
	}
	the_cpuinfo.clk_state = MODE_NORMAL;
	the_cpuinfo.scaling_locked = 0;
	the_cpuinfo.highspeed.used = false;
	the_cpuinfo.upper_lowspeed.used = false;
	atomic_set(&the_cpuinfo.highspeed.usage_count, 0);
	atomic_set(&the_cpuinfo.upper_lowspeed.usage_count, 0);

	/* the max frequency is set at boot time */
	freq = freq_table[0].frequency;
	/*
	 * loader had set the frequency to MAX, already.
	 */
	the_cpuinfo.limit_maxfrq = the_cpuinfo.freq = freq;
	ret = cpufreq_frequency_table_cpuinfo(policy, freq_table);
	if (ret < 0) {
		pr_err("%s()[%d]: error<%d>! main frequency table is invalid\n",
			__func__, __LINE__,	ret);
		return ret;
	}
	policy->cur = the_cpuinfo.freq;
	policy->governor = CPUFREQ_DEFAULT_GOVERNOR;
	policy->cpuinfo.transition_latency = FREQ_TRANSITION_LATENCY;
	the_cpuinfo.policy = policy;
	init_flag--;

	for (i = 0; freq_table[i].frequency != CPUFREQ_TABLE_END; i++)
		pr_info("[%d]:%8u KHz", i, freq_table[i].frequency);

	pr_info("%s()[%d]: init cpufreq driver/end, ret<%d>\n",
		__func__, __LINE__, ret);
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

	ret = pm_setup_clock();
	if (ret)
		return ret;

	pr_log("%s()[%d]: register cpufreq driver\n", __func__, __LINE__);
	ret = cpufreq_register_driver(&shmobile_cpufreq_driver);

	return ret;
}
module_init(shmobile_cpu_init);
