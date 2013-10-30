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
extern void shmobile_timer_init(void);
extern void shmobile_setup_delay(unsigned int max_cpu_core_mhz,
			 unsigned int mult, unsigned int div);
struct twd_local_timer;
extern void shmobile_setup_console(void);
extern void shmobile_secondary_vector(void);
extern void shmobile_secondary_vector_scu(void);
struct clk;
extern int shmobile_clk_init(void);
extern void shmobile_handle_irq_intc(struct pt_regs *);
extern struct platform_suspend_ops shmobile_suspend_ops;
struct cpuidle_driver;
extern void shmobile_cpuidle_set_driver(struct cpuidle_driver *drv);

#ifdef CONFIG_SUSPEND
int shmobile_suspend_init(void);
#else
static inline int shmobile_suspend_init(void) { return 0; }
#endif

#ifdef CONFIG_CPU_IDLE
int shmobile_cpuidle_init(void);
#else
static inline int shmobile_cpuidle_init(void) { return 0; }
#endif

extern void __iomem *shmobile_scu_base;
extern void shmobile_smp_init_cpus(unsigned int ncores);

static inline void __init shmobile_init_late(void)
{
	shmobile_suspend_init();
	shmobile_cpuidle_init();
}

extern void sh_modify_register8(void __iomem *, u8, u8);
extern void sh_modify_register16(void __iomem *, u16, u16);
extern void sh_modify_register32(void __iomem *, u32, u32);

#endif /* __ARCH_MACH_COMMON_H */
