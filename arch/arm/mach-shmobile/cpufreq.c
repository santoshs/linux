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
#define pr_log(fmt,...)	printk(KERN_INFO pr_fmt(fmt), ##__VA_ARGS__)
#else
#define pr_log(fmt,...)
#endif /* CPUFREQ_DEBUG_ENABLE */
/* #define CPUFREQ_TEST_MODE	1 */
/* MAIN Table */
enum cpu_freq_level {
#ifdef CONFIG_U2_ES2
	FREQ_LEV_MAX = 0,
	FREQ_LEV_HIGH,
#else
	FREQ_LEV_MAX = 0,
#endif
	FREQ_LEV_MID,
	FREQ_LEV_MIN,
	FREQ_LEV_NUM
};

/* Clocks State */
enum clock_mode {
	MODE_NORMAL = 0,
	MODE_EARLY_SUSPEND,
	MODE_SUSPEND,
	MODE_NUM
};

/*
 * MAX  : FREQ_LEV_MAX
 * MID  : FREQ_LEV_MID
 * MIN  : FREQ_LEV_MIN
 * NULL : 0xFFFFFFFF
*/
#define EX1_TABLE_VAL FREQ_LEV_MID /* Suspend */
#define EX2_TABLE_VAL 0xFFFFFFFF   /* CoreStandby */
#define FREQ_TRANSITION_LATENCY  (CONFIG_SH_TRANSITION_LATENCY * NSEC_PER_USEC)
/* PLL0 */
#define PLL0_RATIO_MIN			35
#define PLL0_RATIO_MAX			46
#define PLL0_MAIN_CLK			26000

struct cpufreq_resource {
	bool used;
	atomic_t usage_count;
};

struct shmobile_cpuinfo {
	struct cpufreq_frequency_table *curtbl_main;
	struct cpufreq_frequency_table *curtbl_ex1;
	struct cpufreq_frequency_table *curtbl_ex2;
	enum clock_mode clk_state;
	unsigned int without_dvfs_flg;
	unsigned int suspend_mode;
	enum sgx_flg_type sgx_flg;
	unsigned int freq;
	struct cpufreq_resource cpufreq;
	struct cpufreq_resource highspeed;
	spinlock_t lock;
};

/**************** static ****************/
#ifndef CPUFREQ_TEST_MODE
static
#endif /* CPUFREQ_TEST_MODE */
struct shmobile_cpuinfo	cpu_info;
static struct cpufreq_frequency_table main_freqtbl[FREQ_LEV_NUM + 1];
static struct cpufreq_frequency_table sgxon_tbl[FREQ_LEV_NUM];
static struct cpufreq_frequency_table sgxoff_tbl[FREQ_LEV_NUM];
/*
 * __to_freq_level: convert from frequency to frequency level
 *
 * Argument:
 *		@freq: the frequency will be set
 *
 * Return:
 *		freq level
 */
static int __to_freq_level( unsigned int freq )
{
	int idx = 0;

	for (idx = 0; idx < FREQ_LEV_NUM; idx++) {
		if(freq == main_freqtbl[idx].frequency) {
			break;
		}
	}
	pr_log("%s()[%d]: got frequency level<%u>\n", __func__, __LINE__, idx);
	return idx;
}

/*
 * __to_div_rate: convert from frequency to divrate
 *
 * Argument:
 * 		@freq: the frequency will be set
 *
 * Return:
 *		divrate
 */
static int __to_div_rate( unsigned int freq )
{
#ifdef CONFIG_U2_ES2
	if ((freq == main_freqtbl[FREQ_LEV_MAX].frequency) ||
		(freq == main_freqtbl[FREQ_LEV_HIGH].frequency)){
		return (int)DIV1_1;
	}
#endif /* CONFIG_U2_ES2 */
	if (freq == main_freqtbl[FREQ_LEV_MAX].frequency){
		return (int)DIV1_1;
	} else if (freq == main_freqtbl[FREQ_LEV_MID].frequency) {
		return (int)DIV1_2;
	} else if (freq == main_freqtbl[FREQ_LEV_MIN].frequency) {
		return (int)DIV1_4;
	} else {
		pr_err("%s()[%d]: error! invalid frequency<%u>\n", __func__, __LINE__,
			freq);
		return (int)DIV1_0;
	}
}

/*
 * __clk_get_rate: get the set of clocks
 *
 * Argument:
 * 		@clk_state: clock state either MODE_NORMAL/MODE_EARLY_SUSPEND
 * 		@sgx_type: power state of SGX, either CPUFREQ_SGXON/CPUFREQ_SGXOFF
 * 		@freq: current frequency
 *
 * Return:
 *     address of element of array clocks_div_data
 *     NULL: if input parameters are invaid
 */
#ifndef CPUFREQ_TEST_MODE
static
#endif /* CPUFREQ_TEST_MODE */
struct clk_rate* __clk_get_rate( enum clock_mode clk_state,
										enum sgx_flg_type sgx_type,
										unsigned int freq)
{
	struct clk_rate *rate = NULL;
	int level = (int)FREQ_LEV_MAX;
	int freq_mode = 0;
	int lv_num = FREQ_LEV_NUM;
#ifdef CONFIG_U2_ES1
	int sgx_num = CPUFREQ_SGXNUM;
#endif /* CONFIG_U2_ES1 */

	/* Check for invalid parameters */
	if ((clk_state < 0) || (clk_state >= MODE_NUM)) {
		/* Invalid clk_state */
		pr_err("%s()[%d]: error! invalid clock ratio\n",	__func__, __LINE__);
		return NULL;
	}
	if ((sgx_type < 0) || (sgx_type >= CPUFREQ_SGXNUM)) {
		/* Invalid sgx_type */
		return NULL;
	}
	level = __to_freq_level(freq);
	if (FREQ_LEV_NUM == level) {
		/* Invalid frequency level */
		pr_err("%s()[%d]: error! invalid frequency level\n", 
			__func__, __LINE__);
		return NULL;
	}
	/* Get the frequency mode */
#ifdef CONFIG_U2_ES1
	if (MODE_SUSPEND == clk_state) {
		/* suspend state */
		freq_mode = (int)MODE_13;
	} else {
		freq_mode = clk_state * sgx_num * lv_num + sgx_type * lv_num + level;
	}
#else
	if (MODE_SUSPEND == clk_state) {
		/* suspend state */
		freq_mode = (int)MODE_8;
	} else {
		freq_mode = clk_state * lv_num + level;
	}
#endif /* CONFIG_U2_ES2 */
	rate = kmalloc(sizeof(*rate), GFP_KERNEL);
	if(rate == NULL) {
		pr_err("%s()[%d]: error! out-of-memory\n",	__func__, __LINE__);
		return NULL;
	}
	
	if (pm_get_clock_mode(freq_mode, rate)) {
		pr_err("%s()[%d]: error! get clock mode<%d>\n",	__func__, __LINE__,
			freq_mode);
		return NULL;
	}

	return rate;
}

/*
 * __notify_frequency_change: notify frequency change to all present CPUs
 *
 * Argument:
 * 		@fold: the old frequency value
 *		&fnew: the new frequency value
 * 		@cpu_nr: number of CPUS
 * 		@flag: notification flag
 *
 * Return:
 *		none
 */
static void __notify_frequency_change(unsigned int fold, unsigned int fnew,
									  int cpu_nr, int flag)
{
	struct cpufreq_freqs freqs = {0, 0, 0};
	int i = 0;

	freqs.old = fold;
	freqs.new = fnew;
	for (i = 0; i < cpu_nr; i++) {
		freqs.cpu = i;
		cpufreq_notify_transition(&freqs, flag);
	}
}

/*
 * shmobile_set_frequency: set SYS-CPU frequency
 *
 * Argument:
 * 		@freq: the frequency will be set
 *
 * Return:
 *     0: setting is normal
 *     -EINVAL: if input parameter is invaid
 *     -ETIMEDOUT: if the setting take a time out.
 */
#ifndef CPUFREQ_TEST_MODE
static
#endif /* CPUFREQ_TEST_MODE */
int shmobile_set_frequency( unsigned int freq )
{
	int div_rate = (int)DIV1_1;
	int ret = 0;

	if (freq == cpu_info.freq) {
		pr_log("%s()[%d]: frequency not changed, ret<%d>\n", 
			__func__, __LINE__, ret);
		return ret;
	}
#ifdef CONFIG_U2_ES2
	if(__to_freq_level(freq) == FREQ_LEV_MAX) {
		if(pm_get_pll_ratio(PLL0) != PLLx56) {
			/* change pll0 -> x46 */
			ret = pm_set_pll_ratio(PLL0, PLLx56);
			if(ret) {
				pr_err("%s()[%d]: error<%d>! set pll0 ratio<%d>\n",	__func__, 
					__LINE__, ret, PLLx56);
				return ret;
			}
		}
	} else if(pm_get_pll_ratio(PLL0) != PLLx46) {
		/* change pll0 -> x46*/
		ret = pm_set_pll_ratio(PLL0, PLLx46);
		if(ret) {
			pr_err("%s()[%d]: error<%d>! set pll0 ratio<%d>\n",	__func__, 
				__LINE__, ret, PLLx46);
			return ret;
		}
	}
#endif /* CONFIG_U2_ES2 */
	div_rate = __to_div_rate(freq);
	if (div_rate == DIV1_0) {
		pr_err("%s()[%d]: error<%d>! frequency not support\n",	__func__, 
			__LINE__, -EINVAL);
		return -EINVAL;
	}
	ret = pm_set_syscpu_frequency((enum clk_div)div_rate);
	if (0 != ret) {
		pr_err("%s()[%d]: error<%d>! set divrate<0x%x>\n",	__func__, 
			__LINE__, ret, div_rate);
	} else {
#ifdef CONFIG_SMP
		pr_log("SYS-CPU-SMP Core[%d]'s clk[%uKHz->%uKHz]\n", 
			smp_processor_id(), cpu_info.freq, freq);
#else
		pr_log("SYS-CPU Core[%d]'s clk[%uKHz->%uKHz]\n", 0,
			cpu_info.freq, freq);
#endif /* CONFIG_SMP */
		cpu_info.freq = freq;
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
void start_cpufreq( void )
{
	pr_log("%s()[%d]: start scaling/begin, cpufreq flag<%d>,\n",
		__func__, __LINE__, cpu_info.cpufreq.used);
	spin_lock(&cpu_info.lock);
	if (atomic_dec_and_test(&cpu_info.cpufreq.usage_count)) {
		cpu_info.cpufreq.used = false;
	}
	spin_unlock(&cpu_info.lock);
	pr_log("%s()[%d]: start scaling/end, cpufreq flag<%d>,\n",
		__func__, __LINE__, cpu_info.cpufreq.used);
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
 * 		0: normal
 *		-ETIMEOUT: The operation takes timeout
 */
int stop_cpufreq( void )
{
	struct clk_rate *clk_div = NULL;
	unsigned int freq_old = 0;
	unsigned int freq_new = 0;
	int ret = 0;

	pr_log("%s()[%d]: stop scaling/begin, cpufreq flag<%d>,\n",
		__func__, __LINE__, cpu_info.cpufreq.used);
	spin_lock(&cpu_info.lock);
	if (0 == atomic_read(&cpu_info.cpufreq.usage_count)) {
		cpu_info.cpufreq.used = true;
		/*Skip setting hardware if the current SYS-CPU freq is MAX already*/
		if (__to_freq_level(cpu_info.freq) == FREQ_LEV_MAX) {
			atomic_inc(&cpu_info.cpufreq.usage_count);
			spin_unlock(&cpu_info.lock);
			return ret;
		}
		freq_old = cpu_info.freq;
		freq_new = cpu_info.curtbl_main[FREQ_LEV_MAX].frequency;
		ret = shmobile_set_frequency(freq_new);
		if (0 != ret) {
			pr_err("%s()[%d]: error<%d>! set cpu frequency[%u]\n", __func__,
				__LINE__, ret, freq_new);
			cpu_info.cpufreq.used = false;
			spin_unlock(&cpu_info.lock);
			return ret;
		} else {
			__notify_frequency_change(freq_old, freq_new, NR_CPUS,
				CPUFREQ_POSTCHANGE);
			clk_div = __clk_get_rate(cpu_info.clk_state, cpu_info.sgx_flg,
				freq_new);
		}
	}
	atomic_inc(&cpu_info.cpufreq.usage_count);
	spin_unlock(&cpu_info.lock);

	if (NULL != clk_div) {
		ret = pm_set_clocks(clk_div);
		kfree(clk_div);
	}
	pr_log("%s()[%d]: stop scaling/end,cpufreq flag<%d>, ret<%d>\n",
		__func__, __LINE__, cpu_info.cpufreq.used, ret);
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
void disable_dfs_mode_min( void )
{
	struct clk_rate *clk_div = NULL;
	unsigned int freq_old = 0;
	unsigned int freq_new = 0;
	int ret = 0;

	pr_log("%s()[%d]: disable low-level(MIN)/begin, highspeed<%d>,\n",
		__func__, __LINE__, cpu_info.highspeed.used);
	spin_lock(&cpu_info.lock);
	if (atomic_read(&cpu_info.highspeed.usage_count) == 0) {
		cpu_info.highspeed.used = true;
		if (__to_freq_level(cpu_info.freq) == FREQ_LEV_MIN) {
			freq_old = cpu_info.freq;
			freq_new = cpu_info.curtbl_main[FREQ_LEV_MID].frequency;
			ret = shmobile_set_frequency(freq_new);
			if (ret) {
				pr_err("%s()[%d]: error<%d>! set cpu frequency[%u]\n", __func__,
					__LINE__, ret, freq_new);
				cpu_info.highspeed.used = false;
				spin_unlock(&cpu_info.lock);
				return;
			} else {
				__notify_frequency_change(freq_old, freq_new, NR_CPUS,
					CPUFREQ_POSTCHANGE);
				clk_div = __clk_get_rate(cpu_info.clk_state, cpu_info.sgx_flg,
					cpu_info.freq);
			}
		}
	}
    atomic_inc(&cpu_info.highspeed.usage_count);
	spin_unlock(&cpu_info.lock);

	if (clk_div) {
		ret = pm_set_clocks(clk_div);
		kfree(clk_div);
	}
	pr_log("%s()[%d]: disable low-level(MIN)/end, highspeed<%d>, ret<%d>\n",
		__func__, __LINE__, cpu_info.highspeed.used, ret);
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
void enable_dfs_mode_min( void )
{
	pr_log("%s()[%d]: enable low-level(MIN)/begin, highspeed<%d>,\n",
		__func__, __LINE__, cpu_info.highspeed.used);
	spin_lock(&cpu_info.lock);
	if (atomic_dec_and_test(&cpu_info.highspeed.usage_count)) {
		cpu_info.highspeed.used = false;
	}
	spin_unlock(&cpu_info.lock);
	pr_log("%s()[%d]: enable low-level(MIN)/end, highspeed<%d>\n",
		__func__, __LINE__,	cpu_info.highspeed.used);
}
EXPORT_SYMBOL(enable_dfs_mode_min);

/*
 * corestandby_cpufreq: change clocks in case of corestandby
 *
 * Argument:
 *		none
 * Return:
 *		0: normal
 *		-ETIMEDOUT: operation fail
 */
int corestandby_cpufreq( void )
{
	int ret = 0;

	pr_log("%s()[%d]: cpufreq standby/begin\n", __func__, __LINE__);
	spin_lock(&cpu_info.lock);
	if (atomic_read(&cpu_info.cpufreq.usage_count) > 0) {
		spin_unlock(&cpu_info.lock);
		pr_log("%s()[%d]: cpufreq standby/end, ret<%d>\n", __func__, __LINE__,
			ret);
		return ret;
	}
	if (NULL != cpu_info.curtbl_ex2) {
		ret = shmobile_set_frequency(cpu_info.curtbl_ex2->frequency);
		if (ret) {
			pr_err("%s()[%d]: error<%d>! set cpu frequency[%u]\n", __func__,
				__LINE__, ret, cpu_info.curtbl_ex2->frequency);
		}
	}
	spin_unlock(&cpu_info.lock);
	pr_log("%s()[%d]: cpufreq standby/end, ret<%d>\n", __func__, __LINE__, ret);

	return ret;
}
EXPORT_SYMBOL(corestandby_cpufreq);

/*
 * suspend_cpufreq: Change the SYS-CPU frequency to middle before entering
 * 					Suspend state.
 * Argument:
 *		none
 *
 * Return:
 *		0: normal
 *		-ETIMEDOUT: the operation takes timeout
 */
int suspend_cpufreq(void)
{
	struct clk_rate *clk_div = NULL;
	unsigned int freq_old = 0;
	unsigned int freq_new = 0;
	int ret = 0;

	pr_log("%s()[%d]: suspend cpufreq/begin\n", __func__, __LINE__);
	spin_lock(&cpu_info.lock);
	cpu_info.suspend_mode = 1;
	cpu_info.clk_state = MODE_SUSPEND;
	if (NULL != cpu_info.curtbl_ex1) {
		freq_old = cpu_info.freq;
		/* Get the clock value for suspend state */
		freq_new = cpu_info.curtbl_ex1->frequency;
		/* Change SYS-CPU frequency */
		ret = shmobile_set_frequency(freq_new);
		if (0 != ret) {
			pr_err("%s()[%d]: error<%d>! set cpu frequency[%u]\n", __func__,
				__LINE__, ret, freq_new);
			spin_unlock(&cpu_info.lock);
			return -ETIMEDOUT;
		}
		/* Notify to the system */
		__notify_frequency_change(freq_old, freq_new, NR_CPUS,
			CPUFREQ_POSTCHANGE);
	}
	clk_div = __clk_get_rate(cpu_info.clk_state, cpu_info.sgx_flg,
		cpu_info.freq);
	if (NULL != clk_div) {
		ret = pm_set_clocks(clk_div);
		if (0 != ret) {
			pr_err("%s()[%d]: error<%d>! set clocks\n", __func__, __LINE__,
				ret);
			ret = -ETIMEDOUT;
		}
		/* Free the memory of clk_div */
		kfree(clk_div);
	}
	spin_unlock(&cpu_info.lock);
	pr_log("%s()[%d]: suspend cpufreq/end, ret<%d>\n", __func__, __LINE__, ret);
	return ret;
}
EXPORT_SYMBOL(suspend_cpufreq);

/*
 * resume_cpufreq: Change the SYS-CPU and others frequency to middle
 * 				   if the dfs has already been started.
 * Argument:
 *		none
 *
 * Return:
 *		0: normal
 *		-ETIMEDOUT: the operation takes timeout
 */
int resume_cpufreq(void)
{
	struct clk_rate *clk_div = NULL;
	unsigned int freq_old = 0;
	unsigned int freq_new = 0;
	int ret = 0;

	pr_log("%s()[%d]: resume from suspend/begin\n", __func__, __LINE__);
	spin_lock(&cpu_info.lock);
	cpu_info.suspend_mode = 0;
	cpu_info.clk_state = MODE_EARLY_SUSPEND;
	if (cpu_info.cpufreq.used) {
		freq_old = cpu_info.freq;
		ret = shmobile_set_frequency(
					cpu_info.curtbl_main[FREQ_LEV_MAX].frequency);
		if (0 != ret) {
			pr_err("%s()[%d]: error<%d>! set cpu frequency<%u>/end\n", __func__,
				__LINE__, ret, cpu_info.curtbl_main[FREQ_LEV_MAX].frequency);
			spin_unlock(&cpu_info.lock);
			return -ETIMEDOUT;
		}
		freq_new = cpu_info.curtbl_main[FREQ_LEV_MAX].frequency;
		/* Notify to the system */
		__notify_frequency_change(freq_old, freq_new, NR_CPUS,
			CPUFREQ_POSTCHANGE);
	}
	clk_div = __clk_get_rate(cpu_info.clk_state, cpu_info.sgx_flg,
								cpu_info.freq);
	if (clk_div) {
		ret = pm_set_clocks(clk_div);
		if (0 != ret) {
			pr_err("%s()[%d]: error<%d>! set clocks/end\n", __func__, __LINE__,
				ret);
			ret = -ETIMEDOUT;
		}
		/* Free the memory of clk_div */
		kfree(clk_div);
	}
	spin_unlock(&cpu_info.lock);
	pr_log("%s()[%d]: resume from suspend/end, ret<%d>\n", __func__, __LINE__,
		ret);
	return ret;
}
EXPORT_SYMBOL(resume_cpufreq);

/*
 * sgx_cpufreq: change the DFS mode to handle for SGX ON/OFF.
 *
 * Argument:
 * 		@flag: 	The status of SGX module
 *        		CPUFREQ_SGXON: the SGX is on
 *        		CPUFREQ_SGXOFF: the SGX is off
 * Return:
 *		0: normal
 *		-ETIMEDOUT: the operation takes timeout
 *		-EINVAL: The tlag value is invalid
 */
int sgx_cpufreq(int flag)
{
	struct clk_rate *clk_div = NULL;
	int ret = 0;

	pr_log("%s()[%d]: begin\n", __func__, __LINE__);
	spin_lock(&cpu_info.lock);
	if (CPUFREQ_SGXON == flag) {
		cpu_info.curtbl_main = sgxon_tbl;
	} else if (CPUFREQ_SGXOFF == flag) {
		cpu_info.curtbl_main = sgxoff_tbl;
	} else {
		pr_err("%s()[%d]: error<%d>! invalid parameter/end\n", __func__, 
			__LINE__, ret);
		spin_unlock(&cpu_info.lock);
		return -EINVAL;
	}
	if ((EX1_TABLE_VAL >= FREQ_LEV_NUM) || (EX1_TABLE_VAL < 0)) {
		pr_log("%s()[%d]: EX1_TABLE_VAL:%d\n", __func__, __LINE__,
			EX1_TABLE_VAL);
		cpu_info.curtbl_ex1 = NULL;
	} else {
		cpu_info.curtbl_ex1 = cpu_info.curtbl_main + EX1_TABLE_VAL;
	}
	if ((EX2_TABLE_VAL >= FREQ_LEV_NUM) || (EX2_TABLE_VAL < 0)) {
		pr_log("%s()[%d]: EX2_TABLE_VAL:%d\n", __func__, __LINE__,
			EX2_TABLE_VAL);
		cpu_info.curtbl_ex2 = NULL;
	} else {
		cpu_info.curtbl_ex2 = cpu_info.curtbl_main + EX2_TABLE_VAL;
	}
	cpu_info.sgx_flg = (enum sgx_flg_type)flag;
	if (cpu_info.cpufreq.used) {
		/* The cpufreq driver has been started. */
		clk_div = __clk_get_rate(cpu_info.clk_state, cpu_info.sgx_flg,
								cpu_info.freq);
		if (NULL != clk_div) {
			ret = pm_set_clocks(clk_div);
			/* Free the memory of clk_div */
			kfree(clk_div);
			if (ret) {
				pr_log("%s()[%d]: error<%d>! set clocks/end\n", __func__, __LINE__, 
					ret);
				ret = -ETIMEDOUT;
			}
		}
	}
	spin_unlock(&cpu_info.lock);
	pr_log("%s()[%d]: end, sgx_flg<%d>\n", __func__, __LINE__, cpu_info.sgx_flg);
	return ret;
}
EXPORT_SYMBOL(sgx_cpufreq);

/*
 * control_dfs_scaling: enable or disable dynamic frequency scaling.
 *
 * Argument:
 * 		@enabled
 *			true: enable dynamic frequency scaling
 *			false: disable dynamic frequency scaling
 * Return:
 *		none
 */
void control_dfs_scaling(bool enabled)
{
	pr_log("%s()[%d]: begin\n", __func__, __LINE__);
	spin_lock(&cpu_info.lock);
	cpu_info.without_dvfs_flg = (enabled)?0:1;
	spin_unlock(&cpu_info.lock);
	pr_log("%s()[%d]: end/without_dvfs_flg<%d>\n", __func__, __LINE__,
		cpu_info.without_dvfs_flg);
}
EXPORT_SYMBOL(control_dfs_scaling);

#ifdef CONFIG_EARLYSUSPEND
/*
 * function: change clock state and set clocks, support for early suspend state
 * in system suspend.
 *
 * Argument:
 *		@h: the early_suspend interface
 *
 * Return:
 * 		none
 */
#ifndef CPUFREQ_TEST_MODE
static 
#endif
void shmobile_cpufreq_early_suspend( struct early_suspend *h )
{
	struct clk_rate *clk_div = NULL;
	int ret = 0;

	pr_log("%s()[%d]: begin\n", __func__, __LINE__);
	spin_lock(&cpu_info.lock);

	cpu_info.clk_state = MODE_EARLY_SUSPEND;
	clk_div = __clk_get_rate(cpu_info.clk_state, cpu_info.sgx_flg,
		cpu_info.freq );

	spin_unlock(&cpu_info.lock);

	if (NULL != clk_div) {
		ret = pm_set_clocks(clk_div);
		kfree(clk_div);
		if (0 != ret) {
			pr_log("[%s](%d) pm_set_clocks() err ret:%d",__func__,__LINE__,
				ret);
		}
	} else {
		pr_err("[%s](%d) __clk_get_rate() err",__func__,__LINE__);
	}
	pr_log("%s()[%d]: end\n", __func__, __LINE__);
}

/*
 * shmobile_cpufreq_late_resume: change clock state and set clocks, support for
 * 								 late resume state in system suspend.
 *
 * Argument:
 *		structure @early_suspend: the early_suspend interface
 *
 * Return:
 * 		none
 */
#ifndef CPUFREQ_TEST_MODE
static 
#endif
void shmobile_cpufreq_late_resume( struct early_suspend *h )
{
	struct clk_rate *clk_div = NULL;
	int ret = 0;

	pr_log("%s()[%d]: begin\n", __func__, __LINE__);
	spin_lock(&cpu_info.lock);
	cpu_info.clk_state = MODE_NORMAL;
	clk_div = __clk_get_rate(cpu_info.clk_state, cpu_info.sgx_flg,
		cpu_info.freq );
	spin_unlock(&cpu_info.lock);
	if (clk_div != NULL) {
		ret = pm_set_clocks(clk_div);
		kfree(clk_div);
		if (0 != ret) {
			pr_log("[%s](%d) pm_set_clocks() err ret:%d",__func__,__LINE__,
				ret);
		}
	} else {
		pr_err("[%s](%d) __clk_get_rate() err",__func__,__LINE__);
	}
	pr_log("%s()[%d]: end\n", __func__, __LINE__);
}

static struct early_suspend shmobile_cpufreq_suspend = {
	.level = EARLY_SUSPEND_LEVEL_DISABLE_FB+50,
	.suspend = shmobile_cpufreq_early_suspend,
	.resume  = shmobile_cpufreq_late_resume,
};
#endif /* CONFIG_EARLYSUSPEND */

/*
 * shmobile_cpufreq_verify: verify the limit frequency of the policy with the
 * 							limit frequency of the CPU.
 *
 * Argument:
 *		@policy: the input policy
 *
 * Return:
 * 		0: normal
 *		-EINVAL: the policy->cpu is invalid
 */
static int shmobile_cpufreq_verify(struct cpufreq_policy *policy)
{
	pr_log("%s()[%d]: verify frequency table/begin\n", __func__, __LINE__);
	return cpufreq_frequency_table_verify(policy, main_freqtbl);
}

/*
 * shmobile_cpufreq_getspeed: Retrieve the current frequency of a SYS-CPU.
 *
 * Argument:
 * 		@cpu: the ID of CPU
 *
 * Return:
 *		the frequency value of input CPU.
 */
static unsigned int shmobile_cpufreq_getspeed(unsigned int cpu)
{
	pr_log("%s()[%d]: freq[%u]\n", __func__, __LINE__, cpu_info.freq);
	return cpu_info.freq;
}
#ifdef CONFIG_U2_ES1 /* For ES1.x*/
#define DOWN_TO_MIN_THRESHOLD	90
#define DOWN_TO_MID_THRESHOLD	90
#define UP_TO_MID_THRESHOLD		90
#else
#define DOWN_THRESHOLD			90
#endif
/*
 * shmobile_cpufreq_target: judgle frequencies
 *
 * Argument:
 * 		@policy: the policy
 * 		@target_freq: the target frequency passed from CPUFreq framework
 * 		@relation: not used
 *
 * Return:
 *     0: normal
 *     -EINVAL: if input parameter is invaid
 *     -ETIMEDOUT: if the setting take a time out.
 */
#ifndef CPUFREQ_TEST_MODE
static
#endif /* CPUFREQ_TEST_MODE */
int shmobile_cpufreq_target(struct cpufreq_policy *policy,
									unsigned int target_freq,
									unsigned int relation)
{
	struct cpufreq_freqs freqs;
	struct clk_rate *clk_div;
	unsigned int freq = ~0;
	int ret = 0;
#ifdef CONFIG_U2_ES2 /* For ES2.x*/
	int seleted_level = 0;
#endif

	pr_log("%s()[%d]: CPU<%d> target<%u>/begin\n", __func__, __LINE__, 
		policy->cpu, target_freq);
	if(cpu_info.freq == target_freq) {
		pr_log("%s()[%d]: The frequency is not changed. ret<%d>\n", 
				__func__, __LINE__, ret);
		return ret;
	}
	spin_lock(&cpu_info.lock);
	freqs.cpu = policy->cpu;
	freqs.old = cpu_info.freq;
	if (true == cpu_info.cpufreq.used) { /* The driver has been stopped */
		freqs.new = main_freqtbl[FREQ_LEV_MAX].frequency;
		__notify_frequency_change(freqs.old, freqs.new, NR_CPUS,
			CPUFREQ_POSTCHANGE);
		spin_unlock(&cpu_info.lock);
		return ret;
	}

	if ((cpu_info.without_dvfs_flg == 1) || (cpu_info.suspend_mode == 1)) {
		/* Frequency scaling is paused or system is running in suspend mode */
		spin_unlock(&cpu_info.lock);
		pr_log("%s()[%d]: frequency not changed/end, ret<%d>\n", __func__, 
			__LINE__, ret);
		return ret;
	}
#ifdef CONFIG_U2_ES2 /* For ES 2*/
	seleted_level = FREQ_LEV_MIN;
	while (seleted_level-- > FREQ_LEV_MAX) {
		/* The policy to choose freq:
		 * 		The target is in the range [low_level, high_level * 90%]
		 *		-> Decide to low_level.
		*/
		if (target_freq <= (main_freqtbl[seleted_level - 1].frequency *
							DOWN_THRESHOLD / 100)) {
			/* When target <= [DOWN_THRESHOLD% * (Higher Level)],
				decide to current level*/
			freq = main_freqtbl[seleted_level].frequency;
			goto next;
		}
	}
	if (target_freq < main_freqtbl[FREQ_LEV_MAX].frequency) {
		/* When MAX90% < target < MAX , keep current freq */
		freq = cpu_info.freq;
		goto next;
	}
#else /* For ES 1*/
	if (target_freq <= ((main_freqtbl[FREQ_LEV_MID].frequency
						* DOWN_TO_MIN_THRESHOLD) / 100)) {
		/* When target <= [MID90%] , decide the MIN */
		freq = main_freqtbl[FREQ_LEV_MIN].frequency;
		goto next;
	}

	if ((target_freq > ((main_freqtbl[FREQ_LEV_MID].frequency
						* UP_TO_MID_THRESHOLD) / 100))
		&& (target_freq < main_freqtbl[FREQ_LEV_MID].frequency)) {
		/* When [MID90%] < target < MID , decide the MID */
		freq = main_freqtbl[FREQ_LEV_MID].frequency;
		goto next;
	}

	if ((target_freq >= (main_freqtbl[FREQ_LEV_MID].frequency))
		&& (target_freq <= ((main_freqtbl[FREQ_LEV_MAX].frequency
							* DOWN_TO_MID_THRESHOLD) / 100))) {
		/* When MID <= target <= [MAX90%] , decide the MID */
		freq = main_freqtbl[FREQ_LEV_MID].frequency;
		goto next;
	}

	if ((target_freq > ((main_freqtbl[FREQ_LEV_MAX].frequency
						* DOWN_TO_MID_THRESHOLD) / 100))
		&& (target_freq < main_freqtbl[FREQ_LEV_MAX].frequency)) {
		/* When MID90% < target < MAX , keep current freq */
		freq = cpu_info.freq;
		goto next;
	}
#endif /* CONFIG_U2_ES2 */
	/* When over the MAX , decide the MAX */
	freq = main_freqtbl[FREQ_LEV_MAX].frequency;

next:
	if ((cpu_info.highspeed.used)
		&& (freq == main_freqtbl[FREQ_LEV_MIN].frequency)) {
		/* If the cpu is prevented from MIN frequency level
		 * && The new freq is MIN -> change new freq to MID 
		 */
		freq = main_freqtbl[FREQ_LEV_MID].frequency;
		freqs.new = main_freqtbl[FREQ_LEV_MID].frequency;
	}

	if(cpu_info.freq == freq) {
		pr_log("%s()[%d]: frequency not changed/end, ret<%d>\n", __func__, 
			__LINE__, ret);
		spin_unlock(&cpu_info.lock);
		return ret;
	}

	ret = shmobile_set_frequency(freq);
	if (0 != ret) {
		pr_err("%s()[%d]: error<%d>! set cpu frequency<%u>/end\n", __func__,
			__LINE__, ret, freq);
		spin_unlock(&cpu_info.lock);
		return ret;
	}

	/* only do when frequency is really changed */
	if(freq == cpu_info.freq) {
		__notify_frequency_change(freqs.old, freq, NR_CPUS,
			CPUFREQ_POSTCHANGE);
		clk_div = __clk_get_rate(cpu_info.clk_state, cpu_info.sgx_flg,
			cpu_info.freq );
		if (NULL == clk_div) {
			spin_unlock(&cpu_info.lock);
			pr_err("%s()[%d]: error<%d>! fail to get clocks ratio/end\n", 
				__func__, __LINE__, -EINVAL);
			return -EINVAL;
		}
		spin_unlock(&cpu_info.lock);
		ret = pm_set_clocks(clk_div);
		pr_log("%s()[%d]: pm_set_clocks(), ret<%d>\n", __func__, __LINE__, ret);
		kfree(clk_div);
	} else {
		spin_unlock(&cpu_info.lock);
	}

	pr_log("%s()[%d]: CPU<%d> target<%u>, set<%u>/end, ret<%d>\n", __func__, 
		__LINE__, policy->cpu, target_freq, freq, ret);
	return ret;
}

/*
 * shmobile_cpufreq_init: Initialize the DFS module.
 *
 * Argument:
 * 		@policy:	is the policy will change the frequency.
 *
 * Return:
 *		0: normal initialization
 *    < 0: error
 */
 #ifndef CPUFREQ_TEST_MODE
static
#endif /* CPUFREQ_TEST_MODE */
int shmobile_cpufreq_init(struct cpufreq_policy *policy)
{
	unsigned int freq = 0;
	unsigned int stc_val = 0;
	unsigned int pll0_freq = 0;
	static int init_flag = 0;
	int ret = 0;
	int i = 0;

	pr_log("%s()[%d]: init cpufreq driver/start\n", __func__, __LINE__);
	if (!policy) {
		return -EINVAL;
	}
	if (0 != init_flag) { /* The driver has already initialized. */
		pr_log("%s()[%d]: the driver has already initialized\n",
			__func__, __LINE__);
		ret = cpufreq_frequency_table_cpuinfo(policy, main_freqtbl);
		if (0 != ret) {
			pr_err("%s()[%d]: error<%d>! the main frequency table is invalid!",
				__func__, __LINE__, ret);
			return ret;
		}
		policy->cur = cpu_info.freq;
		policy->governor = CPUFREQ_DEFAULT_GOVERNOR;
		policy->cpuinfo.transition_latency = FREQ_TRANSITION_LATENCY;
		pr_log("%s()[%d]: init cpufreq driver/end, current freq<%u>\n",
			__func__, __LINE__, cpu_info.freq);
		return ret;
	}
#ifdef CONFIG_EARLYSUSPEND
	register_early_suspend(&shmobile_cpufreq_suspend);
#endif

#ifdef CONFIG_U2_ES2
	ret = pm_set_pll_ratio(PLL0, PLLx46);
#else
	ret = pm_set_pll_ratio(PLL0, PLLx38);
#endif
	if(ret) {
		pr_err("%s()[%d]: error<%d>! can not set PLL0 ratio<%u>",
			__func__, __LINE__,	ret, stc_val);
	}
	stc_val = pm_get_pll_ratio(PLL0);
	if ((stc_val < PLL0_RATIO_MIN) || (stc_val > PLL0_RATIO_MAX)) {
		pr_err("%s()[%d]: error<%d>! STC<0x%x> supported out-of-range\n",
			__func__, __LINE__,	-EINVAL, stc_val);
		return -EINVAL;
	}
	pll0_freq = PLL0_MAIN_CLK * stc_val;
	/* Initialize all frequency tables for SYS-CPU */
#ifdef CONFIG_U2_ES2
	main_freqtbl[0].index = 0;
	main_freqtbl[0].frequency = PLL0_MAIN_CLK * 56;
	sgxon_tbl[0].index = 0;
	sgxon_tbl[0].frequency = PLL0_MAIN_CLK * 56;
	sgxoff_tbl[0].index = 0;
	sgxoff_tbl[0].frequency = PLL0_MAIN_CLK * 56;
	pr_log("%s()[%d]: freq[%d]:%uHz\n", __func__, __LINE__,	0,
		PLL0_MAIN_CLK * 56);
	for (i = 1; i < FREQ_LEV_NUM; i++) {
#else
	for (i = 0; i < FREQ_LEV_NUM; i++) {
#endif
		pr_log("%s()[%d]: freq[%d]:%uHz\n", __func__, __LINE__,	i, pll0_freq);
		main_freqtbl[i].index = i;
		main_freqtbl[i].frequency = pll0_freq;
		sgxon_tbl[i].index = i;
		sgxon_tbl[i].frequency = pll0_freq;
		sgxoff_tbl[i].index = i;
		sgxoff_tbl[i].frequency = pll0_freq;
		pll0_freq >>= 1;
	}
	/* The last element of main frequency table */
	main_freqtbl[FREQ_LEV_NUM].index = (int)FREQ_LEV_NUM;
	main_freqtbl[FREQ_LEV_NUM].frequency = CPUFREQ_TABLE_END;
	/* Initialize all variables and flags */
	cpu_info.sgx_flg = CPUFREQ_SGXOFF;
	cpu_info.curtbl_main = sgxoff_tbl;
	if ((EX1_TABLE_VAL >= FREQ_LEV_NUM) || (EX1_TABLE_VAL < 0)) {
		pr_log("%s()[%d]: EX1_TABLE_VAL:%d\n", __func__, __LINE__,
			EX1_TABLE_VAL);
		cpu_info.curtbl_ex1 = NULL;
	} else {
		cpu_info.curtbl_ex1 = cpu_info.curtbl_main + EX1_TABLE_VAL;
	}
	if ((EX2_TABLE_VAL >= FREQ_LEV_NUM) || (EX2_TABLE_VAL < 0)) {
		pr_log("%s()[%d]: EX2_TABLE_VAL:%d\n", __func__, __LINE__,
			EX2_TABLE_VAL);
		cpu_info.curtbl_ex2 = NULL;
	} else {
		cpu_info.curtbl_ex2 = cpu_info.curtbl_main + EX2_TABLE_VAL;
	}
	cpu_info.clk_state = MODE_NORMAL;
	cpu_info.without_dvfs_flg = 0;
	cpu_info.suspend_mode = 0;
	cpu_info.cpufreq.used = false;
	cpu_info.highspeed.used = false;
	atomic_set(&cpu_info.cpufreq.usage_count, 0);
	atomic_set(&cpu_info.highspeed.usage_count, 0);

#ifdef CONFIG_U2_ES2
	freq = sgxoff_tbl[FREQ_LEV_HIGH].frequency;
#else
	freq = sgxoff_tbl[FREQ_LEV_MAX].frequency;
#endif
	ret = shmobile_set_frequency(freq); /* Change SYS-CPU frequency to MAX. */
	if (0 != ret) {
		pr_err("%s()[%d]: error<%d>! freq[%u] is not set\n", __func__, __LINE__,
			ret, freq);
		return ret;
	}
	ret = cpufreq_frequency_table_cpuinfo(policy, main_freqtbl);
	if (0 != ret) {
		pr_err("%s()[%d]: error<%d>! main frequency table is invalid\n",
			__func__, __LINE__,	ret);
		return ret;
	}
	policy->cur = cpu_info.freq;
	policy->governor = CPUFREQ_DEFAULT_GOVERNOR;
	policy->cpuinfo.transition_latency = FREQ_TRANSITION_LATENCY;
	init_flag = 1;
	pr_log("%s()[%d]: init cpufreq driver/end, ret<%d>\n", __func__, __LINE__,
		ret);
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
 * shmobile_cpu_init: 	Register the cpufreq driver with the cpufreq
 * 						governor driver.
 *
 * Arguments:
 *		none.
 *
 * Return:
 *		0: normal registration
 *	  < 0: error
 */
static int __init shmobile_cpu_init(void)
{
	int ret = 0;

	pr_log("%s()[%d]: register cpufreq driver\n", __func__, __LINE__);
	ret = cpufreq_register_driver(&shmobile_cpufreq_driver);
	return ret;
}
module_init(shmobile_cpu_init);
