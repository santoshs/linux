/*
 * SMP support for R-Mobile / SH-Mobile - r8a7373 portion
 *
 * Copyright (C) 2010  Magnus Damm
 * Copyright (C) 2010  Takashi Yoshii
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
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/smp.h>
#include <linux/spinlock.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/sh_clk.h>
#include <mach/common.h>
#include <asm/smp_plat.h>
#include <asm/smp_scu.h>
#include <asm/smp_twd.h>
#include <linux/irqchip/arm-gic.h>
#include <mach/pm.h>
#include <linux/delay.h>
#if 0
/* TODO - check if replacement for idle wakelock functionality is required */
#include <linux/wakelock.h>
#endif
#include <asm/cacheflush.h>

#include "pm-r8a7373.h"

static bool init_flag = false;
#if 0
static struct wake_lock smp_idle_wakelock;
#endif

static void __iomem *scu_base_addr(void)
{
	return SCU_BASE;
}

static DEFINE_SPINLOCK(scu_lock);

#ifdef CONFIG_HAVE_ARM_TWD
static DEFINE_TWD_LOCAL_TIMER(twd_local_timer, 0xF0000600, 29);

void __init r8a7373_register_twd(void)
{
	twd_local_timer_register(&twd_local_timer);
}
#endif

static unsigned int __init r8a7373_get_core_count(void)
{
	void __iomem *scu_base = scu_base_addr();

	return scu_get_core_count(scu_base);
}

static void __init r8a7373_smp_init_cpus(void)
{
	unsigned int ncores = r8a7373_get_core_count();
	unsigned int i;

	if (ncores > nr_cpu_ids) {
		pr_warn("SMP: %u cores greater than maximum (%u), clipping\n",
			ncores, nr_cpu_ids);
		ncores = nr_cpu_ids;
	}

	for (i = 0; i < ncores; i++)
		set_cpu_possible(i, true);
}

static void modify_scu_cpu_psr(unsigned long set, unsigned long clr)
{
	void __iomem *scu_base = scu_base_addr();
	unsigned long tmp;

	spin_lock(&scu_lock);
	tmp = __raw_readl(scu_base + 8);
	tmp &= ~clr;
	tmp |= set;
	spin_unlock(&scu_lock);

	/* disable cache coherency after releasing the lock */
	__raw_writel(tmp, scu_base + 8);
}

void __cpuinit r8a7373_secondary_init(unsigned int cpu)
{
	if (init_flag) {
#if 0
		/* merge: TODO: remove wakelock??*/
		wake_unlock(&smp_idle_wakelock);
#endif
	} else {
		init_flag = true;
	}
}

int __cpuinit r8a7373_boot_secondary(unsigned int cpu, struct task_struct *idle)
{
	unsigned long status;
	cpu = cpu_logical_map(cpu);

#if 0
	/* XXX Idle wake locks don't exist any more - if this is necessary,
	 * find a replacement.
	 */
	if (!init_flag) {
		wake_lock_init(&smp_idle_wakelock, WAKE_LOCK_IDLE, "smp Idle");
	}
	else {
		wake_lock(&smp_idle_wakelock);
	}
#endif

	/* enable cache coherency */
	status = (__raw_readl(SCPUSTR) >> (4 * cpu)) & 3;

	if (status == 3) {
		__raw_writel(1 << cpu, WUPCR); /* wake up */
	} else if (status == 0) {
		printk(KERN_NOTICE "CPU%d is SRESETed\n", cpu);
		__raw_writel(1 << cpu, SRESCR); /* reset */
	} else {
		printk(KERN_NOTICE "CPU%d has illegal status %08lx\n",\
				cpu, status);
		__raw_writel(1 << cpu, WUPCR); /* wake up */
		__raw_writel(1 << cpu, SRESCR); /* reset */
	}

	return 0;
}

void __init r8a7373_smp_prepare_cpus(unsigned int max_cpus)
{
	int cpu = cpu_logical_map(0);

	scu_enable(scu_base_addr());

	__raw_writel(0, SBAR2);

	/* Map the reset vector (in headsmp.S) */
	__raw_writel(0, APARMBAREA);      /* 4k */
	__raw_writel(virt_to_phys(shmobile_secondary_vector), SBAR);

	/* enable cache coherency on all CPU */
	for (cpu = 0; cpu < max_cpus; cpu++)
		modify_scu_cpu_psr(0, 3 << (cpu_logical_map(cpu) * 8));
}

#ifdef CONFIG_HOTPLUG_CPU
static int r8a7373_cpu_kill(unsigned int cpu)
{
	unsigned long status;
	int timeout = 1000;

	/* If panic is in progress, do NOT kill cpu */
	u8 reg = __raw_readb(STBCHR2);
	if (reg & APE_RESETLOG_PANIC_START)
		return 1;

	/* skip powerdown check */
	if ((system_state == SYSTEM_RESTART) ||
		(system_state == SYSTEM_HALT) ||
		(system_state == SYSTEM_POWER_OFF))
		return 1;

	cpu = cpu_logical_map(cpu);

	while (0 < timeout) {
		timeout--;
		status = __raw_readl(SCPUSTR);
		if (((status >> (4 * cpu)) & 2) == 2) {
			pr_debug("CPUSTR:0x%08lx\n", status);
			return 1;
		}
		mdelay(1);
	}
	panic("timeout r8a7373_cpu_kill block\n");
	return 1;
}

static void r8a7373_cpu_die(unsigned int cpu)
{
	u8 reg;

	pr_debug("smp-r8a7373: smp_cpu_die is called\n");
	/* hardware shutdown code running on the CPU that is being offlined */
	flush_cache_all();
	dsb();

	/* disable gic cpu_if */
	__raw_writel(0, GIC_CPU_BASE + GIC_CPU_CTRL);

	/* If PANIC is in progress, do NOT call jump_systemsuspend(); */
	reg = __raw_readb(STBCHR2);
	if (reg & APE_RESETLOG_PANIC_START)
		return;

#ifdef CONFIG_SUSPEND
	jump_systemsuspend();
#endif
}

static int r8a7373_cpu_disable(unsigned int cpu)
{
	/*
	 * we don't allow CPU 0 to be shutdown (it is still too special
	 * e.g. clock tick interrupts)
	 */
	return cpu == 0 ? -EPERM : 0;
}
#endif

struct smp_operations r8a7373_smp_ops  __initdata = {
	.smp_init_cpus		= r8a7373_smp_init_cpus,
	.smp_prepare_cpus	= r8a7373_smp_prepare_cpus,
	.smp_boot_secondary	= r8a7373_boot_secondary,
	.smp_secondary_init	= r8a7373_secondary_init,
#ifdef CONFIG_HOTPLUG_CPU
	.cpu_kill		= r8a7373_cpu_kill,
	.cpu_die		= r8a7373_cpu_die,
	.cpu_disable		= r8a7373_cpu_disable,
#endif
};
