/*
 * arch/arm/mach-shmobile/include/mach/pm.h
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
#ifndef __ASM_ARCH_PM_H
#define __ASM_ARCH_PM_H __FILE__
#include <linux/suspend.h>
#include <mach/vmalloc.h>

/*
 * PM state interface
 */
/*
 * @notify the AP from the BB to the callback function, system
 * sleep/CoreStandby/suspend and restore it to the state transition is used to
 * notify the AP.
 * Return 0 if it succeeds, nonzero otherwise.
 */
#define PM_STATE_NOTIFY_SLEEP			1
#define PM_STATE_NOTIFY_SLEEP_LOWFREQ	2
#define PM_STATE_NOTIFY_CORESTANDBY		3
#define PM_STATE_NOTIFY_WAKEUP			4
#define PM_STATE_NOTIFY_SUSPEND			5
#define PM_STATE_NOTIFY_RESUME			6
struct pm_state_notify {
	struct list_head link;
	const char *name;
	unsigned int (*notify)(int state);
};

/*
 * @confirm the AP from the BB to the callback function, the system
 * sleep/CoreStandby/suspend state whether the transition can be used to
 * confirm the AP.
 * It returns 0 if possible, otherwise nonzero.
 */
struct pm_state_notify_confirm {
	struct list_head link;
	const char *name;
	unsigned int (*confirm)(void);
};

#ifdef CONFIG_CPU_IDLE

void register_pm_state_notify(struct pm_state_notify *h);
void unregister_pm_state_notify(struct pm_state_notify *h);
void register_pm_state_notify_confirm(struct pm_state_notify_confirm *h);
void unregister_pm_state_notify_confirm(struct pm_state_notify_confirm *h);
unsigned int state_notify_confirm(void);
unsigned int state_notify(int state);

#else /*!CONFIG_CPU_IDLE*/

static inline void register_pm_state_notify(struct pm_state_notify *h){}
static inline void unregister_pm_state_notify(struct pm_state_notify *h){}
static inline void register_pm_state_notify_confirm(struct pm_state_notify_confirm *h){}
static inline void unregister_pm_state_notify_confirm(struct pm_state_notify_confirm *h){}
static inline unsigned int state_notify_confirm(void){return 0;}
static inline unsigned int state_notify(int state){return 0;}

#endif /*CONFIG_CPU_IDLE*/

#define CPUIDLE_SPINLOCK		0x47BDF000

#define POWER_DOMAIN_COUNT_MAX	3

#define CONFIG_PM_RUNTIME_A3SG
#define CONFIG_PM_RUNTIME_A3SP
#define CONFIG_PM_RUNTIME_A3R
#define CONFIG_PM_RUNTIME_A4RM
#define CONFIG_PM_RUNTIME_A4MP

#ifdef CONFIG_PDC
int power_domain_devices(const char *drv_name,
		struct device **dev, size_t *dev_cnt);

u64 power_down_count(unsigned int powerdomain);

void for_each_power_device(const char *name,
		int (*iterator)(struct device*));
void power_domains_get_sync(const char *name);
void power_domains_put_noidle(const char *name);

#else /*!CONFIG_PDC*/
static inline int power_domain_devices(const char *drv_name,
		struct device **dev, size_t *dev_cnt){return 0;}

static inline u64 power_down_count(unsigned int powerdomain){return 0;}

static inline void for_each_power_device(const char *name,
		int (*iterator)(struct device*)){}
static inline void power_domains_get_sync(const char *name){}
static inline void power_domains_put_noidle(const char *name){}
#endif  /*CONFIG_PDC*/

#ifdef CONFIG_SUSPEND
suspend_state_t get_shmobile_suspend_state(void);
#else /*!CONFIG_SUSPEND*/
static inline suspend_state_t get_shmobile_suspend_state(void){return 0;}
#endif /*CONFIG_SUSPEND*/

#undef IO_ADDRESS
#if VMALLOC_END > 0xe6000000UL

/*
 * io_address
 * 0xe6000000 -> 0xf6000000
 * 0xf0000000 -> 0xf7000000
 */
#define IO_BASE	0xf6000000
#define IO_ADDRESS(x) ((((x) & 0x10000000)>>4)  | ((x) & 0x00ffffff) | IO_BASE)

#else /*VMALLOC_END <= 0xe6000000UL*/

#define IO_ADDRESS(x)	(x)

#endif /*VMALLOC_END*/

#ifdef CONFIG_PM_HAS_SECURE
extern uint32_t sec_hal_power_off(void);
extern uint32_t sec_hal_coma_entry(uint32_t mode, uint32_t freq, uint32_t wakeup_address, uint32_t context_save_address);
#else /* !CONFIG_PM_HAS_SECURE*/
static inline uint32_t sec_hal_power_off(void){return 0;}
static inline uint32_t sec_hal_coma_entry(uint32_t mode, uint32_t freq, uint32_t wakeup_address, uint32_t context_save_address){return 0;}
#endif /*CONFIG_PM_HAS_SECURE*/

/* HPB Phys:0xE6000000 + 0x101C */
#define HPBCCCR				0xE600101C
#define CHIP_VERSION_ES1_0	0x00003E00
#define CHIP_VERSION_ES2_0	0x00003E10
#define CHIP_VERSION_ES2_1	0x00003E11

#define CHIP_VERSION_MASK	0x0000FFFF
#define ES_REV_1_0	(1 << 0)
#define ES_REV_1_1	(1 << 1)
#define ES_REV_1X	(ES_REV_1_1 | ES_REV_1_0)
#define ES_REV_2_0	(1 << 2)
#define ES_REV_2_1	(1 << 2) | (1 << 0)
#define ES_REV_2X	ES_REV_2_1
#define ES_REV_ALL	(ES_REV_2X | ES_REV_1X)

/* #ifdef CONFIG_U2_ES1
#undef CONFIG_U2_ES1
#endif */

#ifndef CONFIG_U2_ES1
#ifndef CONFIG_U2_ES2
#define CONFIG_U2_ES2
#endif
#endif
extern int shmobile_chip_rev(void);

/* SGX flags */
enum sgx_flg_type {
	CPUFREQ_SGXON  = 0,
	CPUFREQ_SGXOFF,
	CPUFREQ_SGXNUM
};
enum freq_mode {
	MODE_1 = 0,	/* Normal, SGX on,  CPU:MAX 		*/
	MODE_2,		/* Normal, SGX on,  CPU:MID 		*/
	MODE_3,		/* Normal, SGX on,  CPU:MIN 		*/
	MODE_4,		/* Normal, SGX off, CPU:MAX 		*/
	MODE_5,		/* Normal, SGX off, CPU:MID 		*/
	MODE_6,		/* Normal, SGX off, CPU:MIN 		*/

	MODE_7,		/* Earlysuspend, SGX on,  CPU:MAX 	*/
	MODE_8,		/* Earlysuspend, SGX on,  CPU:MID 	*/
	MODE_9,		/* Earlysuspend, SGX on,  CPU:MIN 	*/
	MODE_10,	/* Earlysuspend, SGX off, CPU:MAX 	*/
	MODE_11,	/* Earlysuspend, SGX off, CPU:MID 	*/
	MODE_12,	/* Earlysuspend, SGX off, CPU:MIN 	*/
	MODE_13	/* Suspend, SGX on/off, CPU:MID 	*/
};
enum clk_type {
	I_CLK = 0,		/* RT-CPU core clock 			*/
	ZG_CLK,			/* SGX clock 					*/
	B_CLK,			/* Media clock 					*/
	M1_CLK,			/* VPU clock  					*/
	M3_CLK,			/* Video clock 					*/
	Z_CLK,			/* Sysem-CPU core clock 		*/
	ZTR_CLK,		/* Trace clock 					*/
	ZT_CLK,			/* Debug Trace bus clock 		*/
	ZX_CLK,			/* SHwy bus bridge clock 		*/
	HP_CLK,			/* High-speed Peripheral clock 	*/
	ZS_CLK,			/* SHwy clock 					*/
	ZB_CLK,			/* BSC clock 					*/
	ZB3_CLK			/* SBSC bus clock 				*/
};
enum clk_div {
	DIV1_1 = 0x00,	/* x1/1  */
	DIV1_2,			/* x1/2  */
	DIV1_3,			/* x1/3  */
	DIV1_4,			/* x1/4  */
	DIV1_5,			/* x1/5  */
	DIV1_6,			/* x1/6  */
	DIV1_7,			/* x1/7  */
	DIV1_8,			/* x1/8  */
	DIV1_12,		/* x1/12 */
	DIV1_16,		/* x1/16 */
	DIV1_18,		/* x1/18 */
	DIV1_24,		/* x1/24 */
	DIV1_32,		/* x1/32 */
	DIV1_36,		/* x1/36 */
	DIV1_48,		/* x1/48 */
	DIV1_96,		/* x1/96 */
	DIV1_0			/* x1/96 */
};
enum pll_type {
	PLL0 = 0x00,
	PLL1,
	PLL2,
	PLL3
};
enum pll_ratio {
	PLLx38 = 38,
	PLLx46 = 46,
	PLLx56 = 56
};
struct clk_rate {
	enum clk_div i_clk;		/* RT-CPU core clock 			*/
	enum clk_div zg_clk;	/* SGX clock 					*/
	enum clk_div b_clk;		/* Media clock 					*/
	enum clk_div m1_clk;	/* VPU clock  					*/
	enum clk_div m3_clk;	/* Video clock 					*/
	enum clk_div z_clk;		/* Sysem-CPU core clock 		*/
	enum clk_div ztr_clk;	/* Trace clock 					*/
	enum clk_div zt_clk;	/* Debug Trace bus clock 		*/
	enum clk_div zx_clk;	/* SHwy bus bridge clock 		*/
	enum clk_div hp_clk;	/* High-speed Peripheral clock 	*/
	enum clk_div zs_clk;	/* SHwy clock 					*/
	enum clk_div zb_clk;	/* BSC clock 					*/
	enum clk_div zb3_clk;	/* SBSC bus clock 				*/
#ifdef CONFIG_U2_ES2
	enum pll_ratio pll0;	/* Pll0 ratio 					*/
#endif /* CONFIG_U2_ES2 */
};
#ifdef CONFIG_CPU_FREQ
extern void start_cpufreq( void );
extern int stop_cpufreq( void );
extern bool cpufreq_compulsive_exec_get( void );
extern void cpufreq_compulsive_exec_clear( void );
extern void disable_dfs_mode_min( void );
extern void enable_dfs_mode_min( void );
extern int corestandby_cpufreq( void );
extern int suspend_cpufreq( void );
extern int resume_cpufreq( void );
extern int sgx_cpufreq( int flag );
extern void control_dfs_scaling(bool enabled);
/* Internal API for CPUFreq driver only */
extern int pm_set_clocks(const struct clk_rate *clk_div);
extern int pm_set_clock_mode(const int mode);
extern int pm_get_clock_mode(const int mode, struct clk_rate* rate);
extern int pm_set_syscpu_frequency(enum clk_div z_clk);
extern int pm_set_pll_ratio(enum pll_type pll, unsigned int val);
extern int pm_get_pll_ratio(enum pll_type pll);
#else
static inline void start_cpufreq( void ){}
static inline int stop_cpufreq( void ){return 0;}
static inline bool cpufreq_compulsive_exec_get( void ){return false;}
static inline void cpufreq_compulsive_exec_clear( void ){}
static inline void disable_dfs_mode_min( void ){}
static inline void enable_dfs_mode_min( void ){}
static inline int corestandby_cpufreq( void ){return 0;}
static inline int suspend_cpufreq( void ){return 0;}
static inline int resume_cpufreq( void ){return 0;}
static inline int sgx_cpufreq( int flag ){return 0;}
static inline void control_dfs_scaling(bool enabled){}
/* Internal API for CPUFreq driver only */
static inline int pm_set_clocks(const struct clk_rate *clk_div){return 0;}
static inline int pm_set_clock_mode(const int mode){return 0;}
static inline int pm_get_clock_mode(const int mode, struct clk_rate* rate)
{return 0;}
static inline int pm_set_syscpu_frequency(enum clk_div z_clk){return 0;}
static inline int pm_set_pll_ratio(enum pll_type pll, unsigned int val)
{return -EINVAL;}
static inline int pm_get_pll_ratio(enum pll_type pll){return -EINVAL;}
#endif /* CONFIG_CPU_FREQ */
#endif /* __ASM_ARCH_PM_H */
