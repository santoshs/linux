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

#define OVERDRIVE_DEFAULT	1
#define ENABLE_OVERDRIVE_CMDLINE "overdrive=true"
#define DISABLE_OVERDRIVE_CMDLINE "overdrive=false"
#define OVERDRIVE_FREQ	1456000
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
	FREQ_LEV_HIGH,
	FREQ_LEV_MID,
	FREQ_LEV_MIN,
	FREQ_LEV_EXMIN,
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
#ifdef SH_CPUFREQ_OVERDRIVE
#define PLL0_RATIO_MAX	56
#else
#define PLL0_RATIO_MAX	46
#endif /* SH_CPUFREQ_OVERDRIVE */
#define PLL0_MAIN_CLK	26000

#define MAX_ZS_DIVRATE	DIV1_6
#define MAX_HP_DIVRATE	DIV1_12

#define FREQ_MIN_UPPER_LIMIT 299000
#define SAMPLING_RATE_DEF 50000
#define SAMPLING_RATE_LOW 500000
#define SAMPLING_DOWN_FACTOR_DEF 20
#define SAMPLING_DOWN_FACTOR_LOW 1

#define INIT_STATE	1
#define BACK_UP_STATE	2
#define NORMAL_STATE	3
#define STOP_STATE	4

struct cpufreq_resource {
	bool used;
	atomic_t usage_count;
};

struct shmobile_cpuinfo {
	struct cpufreq_resource upper_lowspeed;
	struct cpufreq_resource highspeed;
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
static unsigned int sys_rev;
#ifndef CPUFREQ_TEST_MODE
static
#endif /* CPUFREQ_TEST_MODE */
struct shmobile_cpuinfo	the_cpuinfo;
/* ES1.0 */
static struct cpufreq_frequency_table
main_freqtbl_es1_x[] = {
	{.index = 0, .frequency = 988000}, /* max		*/
	{.index = 1, .frequency = 988000}, /* max		*/
	{.index = 2, .frequency = 494000}, /* mid		*/
	{.index = 3, .frequency = 247000}, /* low		*/
	{.index = 4, .frequency =  CPUFREQ_TABLE_END}
},
main_freqtbl_es2_x[] = {
	{.index = 0, .frequency = 1456000},
	{.index = 1, .frequency = 1196000},	/* high			*/
	{.index = 2, .frequency =  598000},	/* mid			*/
	{.index = 3, .frequency =  299000},	/* low			*/
#ifdef SH_CPUFREQ_VERYLOW
	{.index = 4, .frequency =  199333},	/* extra low	*/
#else
	{.index = 4, .frequency =  299000},	/* low			*/
#endif /* SH_CPUFREQ_VERYLOW */
	{.index = 5, .frequency =  CPUFREQ_TABLE_END}
},
*freq_table = NULL;

/* divrate table */
static int main_divtable[] = {
	DIV1_1, DIV1_1, DIV1_2, DIV1_4
#ifdef SH_CPUFREQ_VERYLOW
	, DIV1_6
#else
	, DIV1_4
#endif /* SH_CPUFREQ_VERYLOW */
};

static unsigned int bk_cpufreq;
static int log_freq_change;
static int sampling_flag = INIT_STATE;
module_param(log_freq_change, int, S_IRUGO | S_IWUSR | S_IWGRP);

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
	int found = FREQ_LEV_NUM;

	for (idx = 0; freq_table[idx].frequency != CPUFREQ_TABLE_END; idx++)
		if (freq_table[idx].frequency == freq)
			found = idx;

	pr_log("%s()[%d]: got frequency level<%u>\n",
		__func__, __LINE__, found);
	return found;
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
 * Return:
 *     address of element of array clocks_div_data
 *     NULL: if input parameters are invaid
 */
#ifndef CPUFREQ_TEST_MODE
static
#endif /* CPUFREQ_TEST_MODE */
int __clk_get_rate(struct clk_rate *rate)
{
	int level = (int)FREQ_LEV_MAX;
	int freq_mode = 0;
	int lv_num = FREQ_LEV_NUM;

	int clk_state = the_cpuinfo.clk_state;
	unsigned int freq = the_cpuinfo.freq;
	int ret = 0;

	/* Check for invalid parameters */
	if ((clk_state < 0) || (clk_state >= MODE_NUM)) {
		/* invalid clk_state */
		pr_err("%s()[%d]: error! invalid clock ratio\n",
			__func__, __LINE__);
		return -EINVAL;
	}
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
		if (shmobile_chip_rev() <= ES_REV_1_0) {
			if (level == FREQ_LEV_MAX)
				level = level + 1;
			freq_mode = (lv_num - 2) * clk_state + level;
		} else {
			/* ES2.x */
			if (MODE_EARLY_SUSPEND == clk_state) {
				if (level == FREQ_LEV_EXMIN)
					level = FREQ_LEV_MIN;
				if (level == FREQ_LEV_MAX)
					level = FREQ_LEV_HIGH;
				freq_mode = clk_state * lv_num + level;
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
	int freq_old = 0;
	int ret = 0;

	/* not allow higher frequency in emergency case(temp high) */
	freq = min(freq, the_cpuinfo.limit_maxfrq);
	if (freq == the_cpuinfo.freq) {
		pr_log("%s()[%d]: frequency not changed, ret<%d>\n",
			__func__, __LINE__, ret);
		return ret;
	}
	freq_old = the_cpuinfo.freq;
#if 0
	/* for overdrive mode, pll0 is x56, other is x46 */
	if (shmobile_chip_rev() >= ES_REV_2_0) {
		if (OVERDRIVE_FREQ == freq)
			pllratio = PLLx56;
		else
			pllratio = PLLx46;
	}
#endif
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
		/* for overdrive mode, pll0 is x56, other is x46 */
		if (shmobile_chip_rev() >= ES_REV_2_0) {
			if (OVERDRIVE_FREQ == freq)
				pllratio = PLLx56;
			else
				pllratio = PLLx46;
		}

		/* change PLL0 if need */
		if (pm_get_pll_ratio(PLL0) != pllratio) {
			ret = pm_set_pll_ratio(PLL0, pllratio);
			if (ret) {
				pr_err("%s()[%d]: error<%d>! set pll0 ratio<%d>\n",
					__func__, __LINE__, ret, pllratio);
				return ret;
			}
		}

		the_cpuinfo.freq = freq;
		__notify_all_cpu(freq_old, freq, CPUFREQ_POSTCHANGE);
	}

	return ret;
}
static inline void __change_sampling_values(void)
{
	int ret = 0;
	static unsigned int downft = SAMPLING_DOWN_FACTOR_DEF;
	static unsigned int samrate = SAMPLING_RATE_DEF;

	if (STOP_STATE == sampling_flag)
		return;
	if (INIT_STATE == sampling_flag) {
		samplrate_downfact_change(SAMPLING_RATE_DEF,
					SAMPLING_DOWN_FACTOR_DEF,
					0);
		sampling_flag = NORMAL_STATE;
	}
	if ((the_cpuinfo.clk_state == MODE_EARLY_SUSPEND &&
		the_cpuinfo.freq <= FREQ_MIN_UPPER_LIMIT) ||
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

	if ((MODE_EARLY_SUSPEND == the_cpuinfo.clk_state)
		&& (shmobile_chip_rev() >= ES_REV_2_0)) {
		if (z_freq == freq_table[FREQ_LEV_MAX].frequency)
			z_freq = freq_table[FREQ_LEV_HIGH].frequency;
#ifdef SH_CPUFREQ_VERYLOW
		if (z_freq == freq_table[FREQ_LEV_EXMIN].frequency)
			z_freq = freq_table[FREQ_LEV_MIN].frequency;
#endif
	}

	ret = __set_rate(z_freq);
	if (ret) {
		pr_log("%s()[%d]: error, __set_rate(), ret<%d>\n",
			__func__, __LINE__, ret);
		return ret;
	}
	__change_sampling_values();
	ret = __clk_get_rate(&rate);
	if (0 == ret) {
		ret = pm_set_clocks(rate);
		if (ret)
			pr_log("%s()[%d]: error, pm_set_clocks(), ret<%d>\n",
				__func__, __LINE__, ret);
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
	if (!cpufreq_enabled)
		return;
#endif /* CONFIG_PM_DEBUG */
	pr_log("%s()[%d]: start scaling/begin, cpufreq flag<%d>,\n",
		__func__, __LINE__, the_cpuinfo.highspeed.used);
	spin_lock(&the_cpuinfo.lock);

	if (atomic_dec_and_test(&the_cpuinfo.highspeed.usage_count))
		the_cpuinfo.highspeed.used = false;

	spin_unlock(&the_cpuinfo.lock);

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
	unsigned int freq_new = 0;
	int ret = 0;

#ifdef CONFIG_PM_DEBUG
	if (!cpufreq_enabled)
		return ret;
#endif /* CONFIG_PM_DEBUG */
	pr_log("%s()[%d]: stop scaling/begin, cpufreq flag<%d>,\n",
		__func__, __LINE__, the_cpuinfo.highspeed.used);
	spin_lock(&the_cpuinfo.lock);

	/*
	 * emergency case, CPU is hot or system going to suspend,
	 * we do not allow user to change frequency now
	 */
	if (MODE_SUSPEND == the_cpuinfo.clk_state) {
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
		if (the_cpuinfo.freq == the_cpuinfo.limit_maxfrq) {
			atomic_inc(&the_cpuinfo.highspeed.usage_count);
			spin_unlock(&the_cpuinfo.lock);
			return ret;
		}

		freq_new = the_cpuinfo.limit_maxfrq;
		ret = __set_all_clocks(freq_new);
		if (ret < 0) {
			pr_err("%s()[%d]: error<%d>! __set_all_clocks(%u)\n",
				__func__, __LINE__, ret, freq_new);
			the_cpuinfo.highspeed.used = false;
			spin_unlock(&the_cpuinfo.lock);
			return ret;
		}
	}

	atomic_inc(&the_cpuinfo.highspeed.usage_count);
	spin_unlock(&the_cpuinfo.lock);
	pr_log("%s()[%d]: stop scaling[cnt:%d]/end,cpufreq flag<%d>, ret<%d>\n",
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
	unsigned int freq_new = 0;
	int ret = 0;
#ifdef CONFIG_PM_DEBUG
	if (!cpufreq_enabled)
		return;
#endif /* CONFIG_PM_DEBUG */
	pr_log("%s()[%d]: disable low-level(MIN)/begin, upper lowspeed<%d>,\n",
		__func__, __LINE__, the_cpuinfo.upper_lowspeed.used);
	spin_lock(&the_cpuinfo.lock);

	/*
	 * emergency case, CPU is hot, we do not allow user to change
	 * frequency now
	 */
	if (the_cpuinfo.limit_maxfrq <= freq_table[FREQ_LEV_MIN].frequency) {
		pr_err("%s()[%d] CPU hot, can not change frequency right now!\n",
			__func__, __LINE__);
	} else if (!atomic_read(&the_cpuinfo.upper_lowspeed.usage_count)) {
		the_cpuinfo.upper_lowspeed.used = true;
		if (the_cpuinfo.freq <= freq_table[FREQ_LEV_MIN].frequency) {
			freq_new = freq_table[FREQ_LEV_MID].frequency;
			ret = __set_all_clocks(freq_new);
			if (ret < 0) {
				pr_err("%s()[%d]: error<%d>! __set_all_clocks(%u)\n",
					__func__, __LINE__, ret, freq_new);
				the_cpuinfo.upper_lowspeed.used = false;
				spin_unlock(&the_cpuinfo.lock);
				return;
			}
		}
	}
	atomic_inc(&the_cpuinfo.upper_lowspeed.usage_count);
	spin_unlock(&the_cpuinfo.lock);
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
	if (!cpufreq_enabled)
		return;
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
	int idx = CORESTB_CPUFREQ;
	int ret = 0;

#ifdef CONFIG_PM_DEBUG
	if (!cpufreq_enabled)
		return ret;
#endif /* CONFIG_PM_DEBUG */
	/* pr_log("%s()[%d]: cpufreq standby/begin\n", __func__, __LINE__); */
	spin_lock(&the_cpuinfo.lock);

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
	unsigned int freq_new = 0;
	int ret = 0;

#ifdef CONFIG_PM_DEBUG
	if (!cpufreq_enabled)
		return ret;
#endif /* CONFIG_PM_DEBUG */
	pr_log("%s()[%d]: suspend cpufreq/begin\n", __func__, __LINE__);
	spin_lock(&the_cpuinfo.lock);

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
	/* Get the clock value for suspend state */
	freq_new = freq_table[SUSPEND_CPUFREQ].frequency;
	/* Change SYS-CPU frequency */
	ret = __set_all_clocks(freq_new);
	if (ret < 0) {
		pr_err("%s()[%d]: error<%d>! __set_all_clocks(%u)\n",
			__func__, __LINE__, ret, freq_new);
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
	unsigned int freq_new = 0;
	int ret = 0;

#ifdef CONFIG_PM_DEBUG
	if (!cpufreq_enabled)
		return ret;
#endif /* CONFIG_PM_DEBUG */
	pr_log("%s()[%d]: resume from suspend/begin\n", __func__, __LINE__);

	spin_lock(&the_cpuinfo.lock);

	/* change to earlysuspend mode */
	the_cpuinfo.clk_state = MODE_EARLY_SUSPEND;
	freq_new = the_cpuinfo.freq;
	if (the_cpuinfo.highspeed.used)
		freq_new = freq_table[FREQ_LEV_MAX].frequency;

	ret = __set_all_clocks(freq_new);
	if (ret < 0) {
		pr_err("%s()[%d]: error<%d>! __set_all_clocks<%u>/end\n",
				__func__, __LINE__, ret, freq_new);
		ret = -ETIMEDOUT;
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
	if (!cpufreq_enabled)
		return ret;
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
	if (!cpufreq_enabled)
		return;
#endif /* CONFIG_PM_DEBUG */
	pr_log("%s()[%d]: begin\n", __func__, __LINE__);
	spin_lock(&the_cpuinfo.lock);
	the_cpuinfo.scaling_locked = (enabled) ? 0 : 1;
	if (enabled) {
		the_cpuinfo.scaling_locked = 0;
		if (bk_cpufreq > freq_table[FREQ_LEV_HIGH].frequency)
			__set_all_clocks(freq_table[FREQ_LEV_MAX].frequency);
		else if (bk_cpufreq < freq_table[FREQ_LEV_MIN].frequency)
			__set_all_clocks(freq_table[FREQ_LEV_EXMIN].frequency);
	} else {
		bk_cpufreq = the_cpuinfo.freq;
		the_cpuinfo.scaling_locked = 1;
	}
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

	pr_log("%s()[%d]: begin\n", __func__, __LINE__);
	if (ES_REV_2_2 <= shmobile_chip_rev())
		return ret;
#ifdef CONFIG_PM_DEBUG
	if (!cpufreq_enabled)
		return ret;
#endif /* CONFIG_PM_DEBUG */
	spin_lock(&the_cpuinfo.lock);
	if (set_max) {
		ret = __clk_get_rate(&clk_div);
		if (ret) {
			spin_unlock(&the_cpuinfo.lock);
			return ret;
		}
		/* restore to max */
		clk_div.zs_clk = MAX_ZS_DIVRATE;
		clk_div.hp_clk = MAX_HP_DIVRATE;
		ret = pm_set_clocks(clk_div);
	}
	if (!ret)
		ret = pm_disable_clock_change(ZSCLK | HPCLK);
	spin_unlock(&the_cpuinfo.lock);

	pr_log("%s()[%d]: end\n", __func__, __LINE__);
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
	int ret = 0;

	pr_log("%s()[%d]: begin\n", __func__, __LINE__);
	if (ES_REV_2_2 <= shmobile_chip_rev())
		return;
#ifdef CONFIG_PM_DEBUG
	if (!cpufreq_enabled)
		return;
#endif /* CONFIG_PM_DEBUG */
	spin_lock(&the_cpuinfo.lock);
	ret = pm_enable_clock_change(ZSCLK | HPCLK);
	/* restore to normal operation */
	if (!ret) {
		ret = __set_all_clocks(the_cpuinfo.freq);
		if (ret < 0)
			pr_err("%s()[%d]: error<%d>! __set_all_clocks(%u)\n",
				__func__, __LINE__, ret, the_cpuinfo.freq);
	}

	spin_unlock(&the_cpuinfo.lock);
	pr_log("%s()[%d]: end\n", __func__, __LINE__);
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

#ifdef CONFIG_PM_DEBUG
	if (!cpufreq_enabled)
		return ret;
#endif /* CONFIG_PM_DEBUG */
	pr_log("%s()[%d]: begin\n", __func__, __LINE__);
	spin_lock(&the_cpuinfo.lock);

	switch (max) {
	case LIMIT_NONE:
		the_cpuinfo.limit_maxfrq = freq_table[FREQ_LEV_MAX].frequency;
		break;
	case LIMIT_MID:
		the_cpuinfo.limit_maxfrq = freq_table[FREQ_LEV_MID].frequency;
		break;
	case LIMIT_LOW:
		the_cpuinfo.limit_maxfrq = freq_table[FREQ_LEV_MIN].frequency;
		break;
	default:
		spin_unlock(&the_cpuinfo.lock);
		return -EINVAL;
	}

	if (the_cpuinfo.highspeed.used ||
		(the_cpuinfo.limit_maxfrq < the_cpuinfo.freq)
		|| bk_cpufreq >= the_cpuinfo.limit_maxfrq) {
		ret = __set_all_clocks(the_cpuinfo.limit_maxfrq);
		if (ret < 0)
			pr_err("%s()[%d]: error<%d>! __set_all_clocks(%u)\n",
				__func__, __LINE__, ret,
				the_cpuinfo.limit_maxfrq);
	} else
		pr_err("%s()[%d]: Frequency is not change(%u)\n",
			__func__, __LINE__, the_cpuinfo.freq);

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

	if (!is_enable) {
		if (!cpufreq_enabled) {
			pr_log("%s()[%d]: cpufreq is disabled already\n",
			__func__, __LINE__);
			return 0;
		}

		ret = stop_cpufreq();
		if (ret) {
			pr_err("%s()[%d]: error<%d>! stop_cpufreq\n",
				__func__, __LINE__, ret);
			return ret;
		}

		spin_lock(&the_cpuinfo.lock);
		cpufreq_enabled = 0;
		spin_unlock(&the_cpuinfo.lock);
		pr_log("%s()[%d]: cpufreq is disabled\n", __func__, __LINE__);
	} else {
		if (cpufreq_enabled) {
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
	int ret = 0;

#ifdef CONFIG_PM_DEBUG
	if (!cpufreq_enabled)
		return;
#endif /* CONFIG_PM_DEBUG */
	pr_log("%s()[%d]: begin\n", __func__, __LINE__);
	spin_lock(&the_cpuinfo.lock);

	the_cpuinfo.clk_state = MODE_EARLY_SUSPEND;
	ret = __set_all_clocks(the_cpuinfo.freq);
	if (ret < 0)
		pr_err("%s()[%d]: error<%d>! __set_all_clocks(%u)\n",
			__func__, __LINE__, ret, the_cpuinfo.freq);
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
	int ret = 0;
	unsigned int freq_new = 0;

#ifdef CONFIG_PM_DEBUG
	if (!cpufreq_enabled)
		return;
#endif /* CONFIG_PM_DEBUG */
	pr_log("%s()[%d]: begin\n", __func__, __LINE__);
	spin_lock(&the_cpuinfo.lock);
	the_cpuinfo.clk_state = MODE_NORMAL;

	if (the_cpuinfo.highspeed.used ||
		(bk_cpufreq > freq_table[FREQ_LEV_HIGH].frequency))
		freq_new = freq_table[FREQ_LEV_MAX].frequency;
	else if (bk_cpufreq < freq_table[FREQ_LEV_MIN].frequency)
		freq_new = freq_table[FREQ_LEV_EXMIN].frequency;
	else
		freq_new = the_cpuinfo.freq;

	ret = __set_all_clocks(freq_new);
	if (ret < 0)
		pr_err("%s()[%d]: error<%d>! __set_all_clocks(%u)\n",
			__func__, __LINE__, ret, the_cpuinfo.freq);
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
	struct cpufreq_frequency_table *ptr = NULL;
	unsigned int old_freq = 0;
	unsigned int freq = ~0;
	int seleted_level = 0;
	int ret = 0;
	int cpu = 0;

#ifdef CONFIG_PM_DEBUG
	if (!cpufreq_enabled)
		return ret;
#endif /* CONFIG_PM_DEBUG */
	/* pr_log("%s()[%d]: CPU<%d> target<%u>/begin\n", __func__, __LINE__,
		policy->cpu, target_freq); */

	spin_lock(&the_cpuinfo.lock);

	the_cpuinfo.req_rate[policy->cpu] = target_freq;
	/* only reduce the CPU frequency if all CPUs need to reduce */
	for_each_online_cpu(cpu) {
		if (cpu != policy->cpu)
			target_freq = max(the_cpuinfo.req_rate[cpu],
				target_freq);
	}

	old_freq = the_cpuinfo.freq;
	if (shmobile_chip_rev() <= ES_REV_1_0)
		seleted_level = FREQ_LEV_NUM - 2;
	else
		seleted_level = FREQ_LEV_NUM - 1;

	while (seleted_level > FREQ_LEV_MAX) {
		ptr = &freq_table[seleted_level - 1];
		if (target_freq <= (ptr->frequency * DOWN_THRESHOLD / 100))
			break;
		seleted_level--;
	}

	freq = freq_table[seleted_level].frequency;

	if ((the_cpuinfo.upper_lowspeed.used) &&
		(freq <= freq_table[FREQ_LEV_MIN].frequency)) {
		freq = freq_table[FREQ_LEV_MID].frequency;
	}


	if ((the_cpuinfo.freq == freq) ||
		(the_cpuinfo.highspeed.used) ||
		(the_cpuinfo.scaling_locked ||
		(MODE_SUSPEND == the_cpuinfo.clk_state))) {
		spin_unlock(&the_cpuinfo.lock);
		pr_log("%s()[%d]: the frequency is not changed. ret<%d>\n",
				__func__, __LINE__, ret);
		return ret;
	}

	ret = __set_all_clocks(freq);
	bk_cpufreq = the_cpuinfo.freq;
	if (ret < 0) {
		pr_err("%s()[%d]: error<%d>! __set_all_clocks(%u)\n",
		__func__, __LINE__, ret, freq);
		spin_unlock(&the_cpuinfo.lock);
		return ret;
	}

	spin_unlock(&the_cpuinfo.lock);
	/* pr_info("CPU<%d> frequency chage<%u>, set<%u>/end, ret<%d>\n",
		policy->cpu, target_freq, freq, ret); */
	if ((freq == the_cpuinfo.freq) && log_freq_change)
		pr_info("SYS-CPU<%d> clk[%uKHz->%uKHz]\n", policy->cpu,
			old_freq, freq);

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
	unsigned int freq = 0;
	static unsigned int init_flag = 1;
	int i = 0;
	int ret = 0;

#ifdef CONFIG_PM_DEBUG
	if (!cpufreq_enabled)
		return ret;
#endif /* CONFIG_PM_DEBUG */
	pr_info("%s()[%d]: init cpufreq driver/start\n", __func__, __LINE__);
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
		/* policy sharing between dual CPUs */
		cpumask_copy(policy->cpus, &cpu_present_map);
		policy->shared_type = CPUFREQ_SHARED_TYPE_ALL;
		pr_log("%s()[%d]: init cpufreq driver/end, current freq<%u>\n",
			__func__, __LINE__, the_cpuinfo.freq);
		return ret;
	}
#ifdef CONFIG_EARLYSUSPEND
	register_early_suspend(&shmobile_cpufreq_suspend);
#endif /* CONFIG_EARLYSUSPEND */
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
	the_cpuinfo.limit_maxfrq = freq;
	the_cpuinfo.freq = pm_get_syscpu_frequency();
	ret = cpufreq_frequency_table_cpuinfo(policy, freq_table);
	if (ret < 0) {
		pr_err("%s()[%d]: error<%d>! main frequency table is invalid\n",
			__func__, __LINE__,	ret);
		return ret;
	}
	policy->cur = the_cpuinfo.freq;
	policy->governor = CPUFREQ_DEFAULT_GOVERNOR;
	policy->cpuinfo.transition_latency = FREQ_TRANSITION_LATENCY;
	/* policy sharing between dual CPUs */
	cpumask_copy(policy->cpus, &cpu_present_map);
	policy->shared_type = CPUFREQ_SHARED_TYPE_ALL;
	init_flag--;
	bk_cpufreq = 0;
	log_freq_change = 0;
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

static int shmobile_policy_changed_notifier(struct notifier_block *nb,
			unsigned long type, void *data)
{
	struct cpufreq_policy *policy;

	if (CPUFREQ_NOTIFY != type)
		return 0;
	policy = (struct cpufreq_policy *)data;
	if (policy)
		if (0 == strcmp(policy->governor->name, "ondemand")) {
			if (STOP_STATE != sampling_flag)
				return 0;
			/* when governor is changed from non-ondemand */
			sampling_flag = INIT_STATE;
			__change_sampling_values();
		} else {
			sampling_flag = STOP_STATE;
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

	sys_rev = shmobile_chip_rev();
#ifdef OVERDRIVE_DEFAULT /* The overdrive mode IS supported by default */
	/* users can disable by command "overdrive=false" */
	if (strstr(&boot_command_line[0], DISABLE_OVERDRIVE_CMDLINE))
		main_freqtbl_es2_x[0].frequency =
			main_freqtbl_es2_x[1].frequency;
#else /* the overdrive mode IS NOT supported by default */
	/* users can enable by command "overdrive=true" */
	if (strstr(&boot_command_line[0], ENABLE_OVERDRIVE_CMDLINE))
		main_freqtbl_es2_x[0].frequency = OVERDRIVE_FREQ;
	else
		main_freqtbl_es2_x[0].frequency =
			main_freqtbl_es2_x[1].frequency;
#endif /* OVERDRIVE_DEFAULT */
	ret = pm_setup_clock();
	if (ret)
		return ret;

	pr_log("%s()[%d]: register cpufreq driver\n", __func__, __LINE__);
	ret = cpufreq_register_driver(&shmobile_cpufreq_driver);
	if (ret)
		return ret;
	ret = cpufreq_register_notifier(&policy_notifier,
					CPUFREQ_POLICY_NOTIFIER);
	return ret;
}


static inline struct attribute *find_attr_by_name(struct attribute **attr,
				const char *name)
{
	while ((attr) && (*attr)) {
		if (strcmp(name, (*attr)->name) == 0)
			return *attr;
		attr++;
	}

	return NULL;
}
/*
 * show_available_freqs - show available frequencies
 */
static ssize_t show_available_freqs(struct kobject *kobj,
				struct kobj_attribute *attr, char *buf)
{
	unsigned int prev = 0;
	ssize_t count = 0;
	int i = 0;

	for (i = 0; (freq_table[i].frequency != CPUFREQ_TABLE_END); i++) {
		if (freq_table[i].frequency == prev)
			continue;
		count += sprintf(&buf[count], "%d ", freq_table[i].frequency);
		prev = freq_table[i].frequency;
	}
	count += sprintf(&buf[count], "\n");

	return count;
}

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
				struct kobj_attribute *attr,
				char *buf)
{
	unsigned int ret = -EINVAL;
	struct cpufreq_policy new_policy;
	struct kobj_type *ktype = NULL;
	struct attribute *att = NULL;
	int i = 0;

	ret = cpufreq_get_policy(&new_policy, 0);
	if (ret)
		return -EINVAL;

	ktype = get_ktype(&new_policy.kobj);
	if (!ktype)
		return -EINVAL;

	for (i = 0; i < ARRAY_SIZE(attr_mapping); i++) {
		if (strcmp(attr->attr.name, attr_mapping[i].alias) == 0) {
			att = find_attr_by_name(ktype->default_attrs,
				attr_mapping[i].target);
			if (att && ktype->sysfs_ops &&
				ktype->sysfs_ops->show)
					return ktype->sysfs_ops->show(
							&new_policy.kobj,
							att, buf);
		}
	}

	return -EINVAL;
}

/*
 * cpufreq_max_limit/cpufreq_min_limit - store max/min frequencies
 * this handler is used for both MAX/MIN limit
 */
static ssize_t store_freq(struct kobject *kobj,
				struct kobj_attribute *attr,
				const char *buf, size_t count)
{
	struct cpufreq_policy *new_policy = NULL;
	struct kobj_type *ktype = NULL;
	struct attribute *att = NULL;
	int ret = -EINVAL;
	int i = 0;
	int freq = 0;

	new_policy = cpufreq_cpu_get(0);
	if (!new_policy) {
		pr_err("fail to get policy\n");
		goto end;
	}

	if (sscanf(buf, "%d", &freq) != 1) {
		pr_err("fail to get param\n");
		goto end;
	}

	/* restore original setting? */
	if (freq == -1) {
		struct cpufreq_frequency_table *ftable = NULL;

		ftable = cpufreq_frequency_get_table(0);
		if (!ftable) {
			pr_err("fail to get frequency table\n");
			goto end;
		}

		ret = cpufreq_frequency_table_cpuinfo(new_policy, ftable);
		if (ret) {
			pr_err("fail to update policy\n");
			goto end;
		}

		/* this must be set for cpufreq_update_policy() */
		if (strcmp(attr->attr.name, "cpufreq_max_limit") == 0)
			new_policy->user_policy.max = new_policy->max;
		else if (strcmp(attr->attr.name, "cpufreq_min_limit") == 0)
			new_policy->user_policy.min = new_policy->min;
		else
			goto end;

		ret = cpufreq_update_policy(0);
		goto end;
	}

	ktype = get_ktype(&new_policy->kobj);
	if (!ktype)
		goto end;

	for (i = 0; i < ARRAY_SIZE(attr_mapping); i++) {
		if (strcmp(attr->attr.name, attr_mapping[i].alias) == 0) {
			att = find_attr_by_name(ktype->default_attrs,
				attr_mapping[i].target);
			if (att && ktype->sysfs_ops &&
				ktype->sysfs_ops->store)
					ret = ktype->sysfs_ops->store(
						&new_policy->kobj,
						att, buf, count);
			break;
		}
	}

end:
	return ret ? ret : count;
}

static struct kobj_attribute cpufreq_table_attribute =
	__ATTR(cpufreq_table, 0444, show_available_freqs, NULL);
static struct kobj_attribute max_limit_attribute =
	__ATTR(cpufreq_max_limit, 0644, show_freq, store_freq);
static struct kobj_attribute min_limit_attribute =
	__ATTR(cpufreq_min_limit, 0644, show_freq, store_freq);
/*
 * Create a group of attributes so that we can create and destroy them all
 * at once.
 */
static struct attribute *attrs[] = {
	&min_limit_attribute.attr,
	&max_limit_attribute.attr,
	&cpufreq_table_attribute.attr,
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

