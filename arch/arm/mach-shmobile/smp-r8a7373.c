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
#include <asm/cacheflush.h>

#include "pm-r8a7373.h"

static bool init_flag = false;
DEFINE_MUTEX(smp_idle_lock);

#ifdef CONFIG_HAVE_ARM_TWD
static DEFINE_TWD_LOCAL_TIMER(twd_local_timer, 0xF0000600, 29);

void __init r8a7373_register_twd(void)
{
	twd_local_timer_register(&twd_local_timer);
}
#endif

static void __init r8a7373_smp_init_cpus(void)
{
	shmobile_scu_base = ioremap(SCU_BASE_PHYS, PAGE_SIZE);

	shmobile_smp_init_cpus(scu_get_core_count(shmobile_scu_base));
}

void __cpuinit r8a7373_secondary_init(unsigned int cpu)
{
	if (init_flag) {
		mutex_unlock(&smp_idle_lock);
	} else {
		init_flag = true;
	}
}

int __cpuinit r8a7373_boot_secondary(unsigned int cpu, struct task_struct *idle)
{
	unsigned long status;
	cpu = cpu_logical_map(cpu);

	if (init_flag)
		mutex_lock(&smp_idle_lock);

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
	scu_enable(shmobile_scu_base);

	__raw_writel(0, SBAR2);

	/* Map the reset vector (in headsmp-scu.S) */
	__raw_writel(0, APARMBAREA);      /* 4k */
	__raw_writel(virt_to_phys(shmobile_secondary_vector_scu), SBAR);

	/* enable cache coherency on booting CPU */
	scu_power_mode(shmobile_scu_base, SCU_PM_NORMAL);
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
#ifdef IDLE_WAKELOCK_THING
	.smp_secondary_init	= r8a7373_secondary_init,
#endif
#ifdef CONFIG_HOTPLUG_CPU
	.cpu_kill		= r8a7373_cpu_kill,
	.cpu_die		= r8a7373_cpu_die,
	.cpu_disable		= r8a7373_cpu_disable,
#endif
};
