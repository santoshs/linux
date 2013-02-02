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
#include <asm/hardware/gic.h>
#include <mach/pm.h>
#include <linux/delay.h>

#include "pm-r8a7373.h"


static void __iomem *scu_base_addr(void)
{
	return IOMEM(0xf0000000);
}

static DEFINE_SPINLOCK(scu_lock);

#ifdef CONFIG_HAVE_ARM_TWD
static DEFINE_TWD_LOCAL_TIMER(twd_local_timer, 0xf0000600, 29);

void __init r8a7373_register_twd(void)
{
	twd_local_timer_register(&twd_local_timer);
}
#endif

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

unsigned int __init r8a7373_get_core_count(void)
{
	void __iomem *scu_base = scu_base_addr();

	return scu_get_core_count(scu_base);
}

int r8a7373_platform_cpu_kill(unsigned int cpu)
{
	/* cpu = cpu_logical_map(cpu); */

	/* disable cache coherency */
	/* modify_scu_cpu_psr(3 << (cpu * 8), 0); */

	if (system_state == SYSTEM_RESTART ||
		system_state == SYSTEM_HALT) {
		cpu = cpu_logical_map(cpu);
		modify_scu_cpu_psr(3 << (cpu * 8), 0);
		goto abort_checking;
	}
	pr_debug("SCUSTAT:0x%x\n", __raw_readl(scu_base_addr() + 8));
	while ((((__raw_readl(scu_base_addr() + 8)) >> (8 * cpu)) & 3) != 3)
		mdelay(1);
abort_checking:
	pr_debug("SCUSTAT:0x%x\n", __raw_readl(scu_base_addr() + 8));

	return 1;
}

int r8a7373_platform_cpu_die(unsigned int cpu)
{
	pr_debug("smp-r8a7373: smp_cpu_die is called\n");
	/* disable gic cpu_if */
	__raw_writel(0, IOMEM(0xf0000100 + GIC_CPU_CTRL));

	return 1;
}

void __cpuinit r8a7373_secondary_init(unsigned int cpu)
{
	gic_secondary_init(0);
}

int __cpuinit r8a7373_boot_secondary(unsigned int cpu)
{
	cpu = cpu_logical_map(cpu);

	/* enable cache coherency */
	modify_scu_cpu_psr(0, 3 << (cpu * 8));

	if (((__raw_readl(IOMEM(SCPUSTR)) >> (4 * cpu)) & 3) == 3)
		__raw_writel(1 << cpu, IOMEM(WUPCR));	/* wake up */
	else
		__raw_writel(1 << cpu, IOMEM(SRESCR));	/* reset */

	return 0;
}

void __init r8a7373_smp_prepare_cpus(void)
{
	int cpu = cpu_logical_map(0);

	scu_enable(scu_base_addr());

	__raw_writel(0, IOMEM(SBAR2));

	/* Map the reset vector (in headsmp.S) */
	__raw_writel(0, IOMEM(APARMBAREA));      /* 4k */
	__raw_writel(__pa(shmobile_secondary_vector), IOMEM(SBAR));

	/* enable cache coherency on CPU0 */
	modify_scu_cpu_psr(0, 3 << (cpu * 8));
}
