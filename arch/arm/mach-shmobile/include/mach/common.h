#ifndef __ARCH_MACH_COMMON_H
#define __ARCH_MACH_COMMON_H

#include <mach/hardware.h>

extern unsigned int shmobile_rev(void);

#define U2_PRODUCT		0x00003E00 /* aka R8A7373 */
#define U2_VERSION_1_0		0x00003E00
#define U2_VERSION_2_0		0x00003E10
#define U2_VERSION_2_1		0x00003E11
#define U2_VERSION_2_2		0x00003E12
#define U2_VERSION_2_3		0x00003E13

#define APE6_PRODUCT		0x00003F00 /* aka R8A73A4 */

#define U3_PRODUCT		0x00004400 /* aka R8A73724 */

#define shmobile_product() (shmobile_rev() & CCCR_PRODUCT_MASK)
#define shmobile_is_u2() (shmobile_product() == U2_PRODUCT)
#define shmobile_is_u3() (shmobile_product() == U3_PRODUCT)
#define shmobile_is_ape6() (shmobile_product() == APE6_PRODUCT)

/* Handy test for old versions of a specific product that we need some sort
 * of workaround for - better than doing a straight "if (shmobile_rev() < x)".
 *
 * Returns true if we're on an older version of the specified product than
 * the specified version.
 * Returns false if we're on the specified version or newer, or it's a
 * different product.
 */
static inline int shmobile_is_older(unsigned int than)
{
	unsigned int rev = shmobile_rev();
	return rev < than
		&& (rev & CCCR_PRODUCT_MASK) == (than & CCCR_PRODUCT_MASK);
}

extern void shmobile_earlytimer_init(void);
extern void shmobile_calibrate_delay_early(void);
extern struct sys_timer shmobile_timer;
struct twd_local_timer;
extern void shmobile_setup_console(void);
extern void shmobile_secondary_vector(void);
extern int shmobile_platform_cpu_kill(unsigned int cpu);
extern int shmobile_platform_cpu_die(unsigned int cpu);
struct clk;
extern int shmobile_clk_init(void);
extern void shmobile_handle_irq_intc(struct pt_regs *);
extern void shmobile_handle_irq_gic(struct pt_regs *);
extern struct platform_suspend_ops shmobile_suspend_ops;
struct cpuidle_driver;
extern void (*shmobile_cpuidle_modes[])(void);
extern void (*shmobile_cpuidle_setup)(struct cpuidle_driver *drv);

extern void (*shmobile_clocksource_init)(void);
extern void shmobile_clockevent_init(void);

extern void sh7367_init_irq(void);
extern void sh7367_map_io(void);
extern void sh7367_add_early_devices(void);
extern void sh7367_add_standard_devices(void);
extern void sh7367_clock_init(void);
extern void sh7367_pinmux_init(void);
extern struct clk sh7367_extalb1_clk;
extern struct clk sh7367_extal2_clk;

extern void sh7377_init_irq(void);
extern void sh7377_map_io(void);
extern void sh7377_add_early_devices(void);
extern void sh7377_add_standard_devices(void);
extern void sh7377_clock_init(void);
extern void sh7377_pinmux_init(void);
extern struct clk sh7377_extalc1_clk;
extern struct clk sh7377_extal2_clk;

extern void sh7372_init_irq(void);
extern void sh7372_map_io(void);
extern void sh7372_add_early_devices(void);
extern void sh7372_add_standard_devices(void);
extern void sh7372_clock_init(void);
extern void sh7372_pinmux_init(void);
extern void sh7372_pm_init(void);
extern void sh7372_resume_core_standby_sysc(void);
extern int sh7372_do_idle_sysc(unsigned long sleep_mode);
extern struct clk sh7372_extal1_clk;
extern struct clk sh7372_extal2_clk;

extern void sh73a0_init_irq(void);
extern void sh73a0_map_io(void);
extern void sh73a0_add_early_devices(void);
extern void sh73a0_add_standard_devices(void);
extern void sh73a0_clock_init(void);
extern void sh73a0_pinmux_init(void);
extern struct clk sh73a0_extal1_clk;
extern struct clk sh73a0_extal2_clk;
extern struct clk sh73a0_extcki_clk;
extern struct clk sh73a0_extalr_clk;

extern void r8a73734_init_irq(void);
extern void r8a73734_add_early_devices(void);
extern void r8a73734_add_standard_devices(void);
extern void r8a73734_clock_init(void);
extern void r8a73734_pinmux_init(void);

extern unsigned int sh73a0_get_core_count(void);
extern void sh73a0_secondary_init(unsigned int cpu);
extern int sh73a0_boot_secondary(unsigned int cpu);
extern void sh73a0_smp_prepare_cpus(void);

extern void r8a7740_init_irq(void);
extern void r8a7740_map_io(void);
extern void r8a7740_add_early_devices(void);
extern void r8a7740_add_standard_devices(void);
extern void r8a7740_clock_init(u8 md_ck);
extern void r8a7740_pinmux_init(void);

extern void r8a7779_init_irq(void);
extern void r8a7779_map_io(void);
extern void r8a7779_add_early_devices(void);
extern void r8a7779_add_standard_devices(void);
extern void r8a7779_clock_init(void);
extern void r8a7779_pinmux_init(void);
extern void r8a7779_pm_init(void);

extern unsigned int r8a7779_get_core_count(void);
extern int r8a7779_platform_cpu_kill(unsigned int cpu);
extern void r8a7779_secondary_init(unsigned int cpu);
extern int r8a7779_boot_secondary(unsigned int cpu);
extern void r8a7779_smp_prepare_cpus(void);
extern void r8a7779_register_twd(void);

/* r8a7373 */
extern void r8a7373_init_irq(void);
extern void r8a7373_map_io(void);
extern void r8a7373_init_early(void);
extern void r8a7373_reserve(void);
extern void r8a7373_add_standard_devices(void);
extern void r8a7373_avoid_a2slpowerdown_afterL2sync(void);
extern void r8a7373_clock_init(void);
extern void r8a7373_pinmux_init(void);
extern void r8a7373_pm_init(void);
extern void r8a7373_enter_core_standby(void);

extern void sh_modify_register8(void __iomem *, u8, u8);
extern void sh_modify_register16(void __iomem *, u16, u16);
extern void sh_modify_register32(void __iomem *, u32, u32);

extern unsigned int r8a7373_get_core_count(void);
extern int r8a7373_platform_cpu_kill(unsigned int cpu);
extern int r8a7373_platform_cpu_die(unsigned int cpu);
extern void r8a7373_secondary_init(unsigned int cpu);
extern int r8a7373_boot_secondary(unsigned int cpu);
extern void r8a7373_smp_prepare_cpus(unsigned int max_cpus);
extern void r8a7373_register_twd(void);
extern void r8a7373_secondary_vector(void);
extern unsigned long r8a7373_secondary_vector_addr;
extern unsigned long r8a7373_secondary_vector_sz;
#endif /* __ARCH_MACH_COMMON_H */
