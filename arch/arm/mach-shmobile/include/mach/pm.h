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
#include <mach/r8a73734.h>
#include <linux/io.h>

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
extern void disablemmu(void);
extern void systemsuspend_cpu0_pa(void);
extern void systemsuspend_cpu1_pa(void);
extern void corestandby_pa(void);

#ifdef CONFIG_CPU_IDLE
void register_pm_state_notify(struct pm_state_notify *h);
void unregister_pm_state_notify(struct pm_state_notify *h);
void register_pm_state_notify_confirm(struct pm_state_notify_confirm *h);
void unregister_pm_state_notify_confirm(struct pm_state_notify_confirm *h);
unsigned int state_notify_confirm(void);
unsigned int state_notify(int state);
#ifdef CONFIG_PM_DEBUG
extern int control_cpuidle(int is_enable);
extern int is_cpuidle_enable(void);
#endif /* CONFIG_PM_DEBUG */
#else /*!CONFIG_CPU_IDLE*/
static inline void register_pm_state_notify(struct pm_state_notify *h) {}
static inline void unregister_pm_state_notify(struct pm_state_notify *h) {}
static inline void register_pm_state_notify_confirm(
	struct pm_state_notify_confirm *h) {}
static inline void unregister_pm_state_notify_confirm(
	struct pm_state_notify_confirm *h) {}
static inline unsigned int state_notify_confirm(void) { return 0; }
static inline unsigned int state_notify(int state) { return 0; }
#ifdef CONFIG_PM_DEBUG
static inline int control_cpuidle(int is_enable) { return 0; }
static inline int is_cpuidle_enable(void)  { return 0; }
#endif /* CONFIG_PM_DEBUG */
#endif /*CONFIG_CPU_IDLE*/


#define POWER_DOMAIN_COUNT_MAX	3
#define CONFIG_PM_RUNTIME_A3SG
#define CONFIG_PM_RUNTIME_A3SP
#define CONFIG_PM_RUNTIME_A3R
#define CONFIG_PM_RUNTIME_A4RM
#define CONFIG_PM_RUNTIME_A4MP

/*Value of power area (value is appropriate with SWUCR, SPDCR, PSTR registers)*/
#define POWER_A2SL					BIT(20)
#define POWER_A3SM					BIT(19)
#define POWER_A3SG					BIT(18)
#define POWER_A3SP					BIT(17)
#define POWER_C4					BIT(16)
#define POWER_A2RI					BIT(15)
#define POWER_A2RV					BIT(14)
#define POWER_A3R					BIT(13)
#define POWER_A4RM					BIT(12)
#define POWER_A4MP					BIT(8)
#define POWER_A4LC					BIT(6)
#define POWER_D4					BIT(1)
#define POWER_ALL					0x001FF142
#define POWER_NONE					0

#ifdef CONFIG_PDC
struct power_domain_info {
	struct device *devs[POWER_DOMAIN_COUNT_MAX];
	size_t cnt;
};
int power_domain_devices(const char *drv_name,
		struct device **dev, size_t *dev_cnt);

u64 power_down_count(unsigned int powerdomain);

void for_each_power_device(const struct device *dev,
		int (*iterator)(struct device *));
void power_domains_get_sync(const struct device *dev);
void power_domains_put_noidle(const struct device *dev);
struct power_domain_info *__to_pdi(const struct device *dev);
#ifdef CONFIG_PM_DEBUG
int control_pdc(int is_enable);
int is_pdc_enable(void);
#endif
#else /*!CONFIG_PDC*/
static inline int power_domain_devices(const char *drv_name,
		struct device **dev, size_t *dev_cnt) { return 0; }

static inline u64 power_down_count(unsigned int powerdomain) { return 0; }

static inline void for_each_power_device(const struct device *dev,
		int (*iterator)(struct device *)) {}
static inline void power_domains_get_sync(const struct device *dev) {}
static inline void power_domains_put_noidle(const struct device *dev) {}
#ifdef CONFIG_PM_DEBUG
static inline int control_pdc(int is_enable) { return 0; }
static inline int is_pdc_enable(void) { return 0; }
#endif
#endif  /*CONFIG_PDC*/

#ifdef CONFIG_SUSPEND
suspend_state_t get_shmobile_suspend_state(void);
void shwystatdm_regs_save(void);
void shwystatdm_regs_restore(void);
#ifdef CONFIG_PM_DEBUG
int control_systemsuspend(int is_enabled);
int is_systemsuspend_enable(void);
#else
static inline int control_systemsuspend(int is_enabled) { return 0; }
static inline int is_systemsuspend_enable(void) { return 0; }
#endif
#else /*!CONFIG_SUSPEND*/
static inline suspend_state_t get_shmobile_suspend_state(void) { return 0; }
static inline int control_systemsuspend(int is_enabled) { return 0; }
static inline int is_systemsuspend_enable(void) { return 0; }
static inline void shwystatdm_regs_save(void) { return 0;}
static inline void shwystatdm_regs_restore(void) { return 0;}
#endif /*CONFIG_SUSPEND*/


#ifdef CONFIG_PM_HAS_SECURE
extern uint32_t sec_hal_power_off(void);
extern uint32_t sec_hal_coma_entry(uint32_t mode, uint32_t freq,
	uint32_t wakeup_address, uint32_t context_save_address);
#else /* !CONFIG_PM_HAS_SECURE*/
static inline uint32_t sec_hal_power_off(void) { return 0; }
static inline uint32_t sec_hal_coma_entry(uint32_t mode, uint32_t freq,
	uint32_t wakeup_address, uint32_t context_save_address) { return 0; }
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
#define ES_REV_2_1	((1 << 2) | (1 << 0))
#define ES_REV_2X	ES_REV_2_1
#define ES_REV_ALL	(ES_REV_2X | ES_REV_1X)
#define CCCR	IO_ADDRESS(0xE600101C)

/*
 * Helper functions for getting chip revision
 */
static inline int shmobile_chip_rev(void)
{
	switch (system_rev & CHIP_VERSION_MASK) {
	case CHIP_VERSION_ES1_0:
		return ES_REV_1_0;
	case CHIP_VERSION_ES2_0:
		return ES_REV_2_0;
	case CHIP_VERSION_ES2_1:
		return ES_REV_2_1;
	default:
		break;
	}
	return ES_REV_2X;
}

/* SGX flags */
enum sgx_flg_type {
	CPUFREQ_SGXON  = 0,
	CPUFREQ_SGXOFF,
	CPUFREQ_SGXNUM
};

enum freq_mode {
	MODE_1 = 0,
	MODE_2,
	MODE_3,
	MODE_4,
	MODE_5,
	MODE_6,
	MODE_7,
	MODE_8,
	MODE_9,
	MODE_10,
	MODE_11,
	MODE_12,
	MODE_13,
	MODE_14,
	MODE_15
};

enum clk_type {
	I_CLK = 0,
	ZG_CLK,
	B_CLK,
	M1_CLK,
	M3_CLK,
	M5_CLK,
	Z_CLK,
	ZTR_CLK,
	ZT_CLK,
	ZX_CLK,
	HP_CLK,
	ZS_CLK,
	ZB_CLK,
	ZB3_CLK
};

enum clk_div {
	DIV1_1 = 0x00,
	DIV1_2,
	DIV1_3,
	DIV1_4,
	DIV1_5,
	DIV1_6,
	DIV1_7,
	DIV1_8,
	DIV1_12,
	DIV1_16,
	DIV1_18,
	DIV1_24,
	DIV1_32,
	DIV1_36,
	DIV1_48,
	DIV1_96
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
	enum clk_div i_clk;
	enum clk_div zg_clk;
	enum clk_div b_clk;
	enum clk_div m1_clk;
	enum clk_div m3_clk;
	enum clk_div m5_clk;
	enum clk_div z_clk;
	enum clk_div ztr_clk;
	enum clk_div zt_clk;
	enum clk_div zx_clk;
	enum clk_div hp_clk;
	enum clk_div zs_clk;
	enum clk_div zb_clk;
	enum clk_div zb3_clk;
	enum pll_ratio pll0;
};

enum limit_freq {
	LIMIT_NONE,		/* no limit		*/
	LIMIT_MID,		/* limit mid	*/
	LIMIT_LOW,		/* limit low	*/
};
enum {
	ZSCLK = BIT(1),
	HPCLK = BIT(2)
};
/* API use for Power Off module */
extern void setup_mm_for_reboot(void);
extern void arm_machine_flush_console(void);
#ifdef CONFIG_CPU_FREQ
/* overdrive mode enable flag */
#define SH_CPUFREQ_OVERDRIVE	1

/* verylow mode enable flag */
/* #define SH_CPUFREQ_VERYLOW	1 */
extern void start_cpufreq(void);
extern int stop_cpufreq(void);
extern bool cpufreq_compulsive_exec_get(void);
extern void cpufreq_compulsive_exec_clear(void);
extern void disable_dfs_mode_min(void);
extern void enable_dfs_mode_min(void);
extern int corestandby_cpufreq(void);
extern int suspend_cpufreq(void);
extern int resume_cpufreq(void);
extern int sgx_cpufreq(int flag);
extern void control_dfs_scaling(bool enabled);
extern int limit_max_cpufreq(int max);
extern int suppress_clocks_change(bool set_max);
extern void unsuppress_clocks_change(void);
#ifdef CONFIG_PM_DEBUG
extern int control_cpufreq(int is_enable);
extern int is_cpufreq_enable(void);
#endif
/* Internal API for CPUFreq driver only */
extern int pm_set_clocks(const struct clk_rate clk_div);
extern int pm_set_clock_mode(const int mode);
extern int pm_get_clock_mode(const int mode, struct clk_rate *rate);
extern int pm_set_syscpu_frequency(int div);
extern int pm_set_pll_ratio(int pll, unsigned int val);
extern int pm_get_pll_ratio(int pll);
extern int pm_setup_clock(void);
extern int pm_enable_clock_change(int clk);
extern int pm_disable_clock_change(int clk);
extern unsigned long pm_get_spinlock(void);
extern void pm_release_spinlock(unsigned long flag);
#else
static inline void start_cpufreq(void) {}
static inline int stop_cpufreq(void) { return 0; }
static inline void disable_dfs_mode_min(void) {}
static inline void enable_dfs_mode_min(void) {}
static inline int corestandby_cpufreq(void) { return 0; }
static inline int suspend_cpufreq(void) { return 0; }
static inline int resume_cpufreq(void) { return 0; }
static inline int sgx_cpufreq(int flag) { return 0; }
static inline void control_dfs_scaling(bool enabled) {}
static inline int limit_max_cpufreq(int max) { return 0; }
static inline int suppress_clocks_change(bool set_max) { return 0; }
static inline void unsuppress_clocks_change(void) {}
#ifdef CONFIG_PM_DEBUG
static inline int control_cpufreq(int is_enable) { return 0; }
static inline int is_cpufreq_enable(void) { return 0; }
#endif
/* Internal API for CPUFreq driver only */
static inline int pm_set_clocks(const struct clk_rate *clk_div) { return 0; }
static inline int pm_set_clock_mode(const int mode) { return 0; }
static inline int pm_get_clock_mode(const int mode, struct clk_rate *rate)
{return 0; }
static inline int pm_set_syscpu_frequency(int div) { return 0; }
static inline int pm_set_pll_ratio(int pll, unsigned int val)
{return -EINVAL; }
static inline int pm_get_pll_ratio(int pll) { return -EINVAL; }
static inline int pm_setup_clock(void) { return 0; }
static inline int pm_enable_clock_change(int clk) { return 0; }
static inline int pm_disable_clock_change(int clk) { return 0; }
static inline unsigned long pm_get_spinlock(void) { return 0; }
static inline void pm_release_spinlock(unsigned long flag) { }
#endif /* CONFIG_CPU_FREQ */
#endif /* __ASM_ARCH_PM_H */
