/*
 * SMP support for R-Mobile / SH-Mobile
 *
 * Copyright (C) 2010  Magnus Damm
 * Copyright (C) 2011  Paul Mundt
 *
 * Based on vexpress, Copyright (C) 2002 ARM Ltd, All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/smp.h>
#include <linux/io.h>
#include <asm/hardware/gic.h>
#include <asm/mach-types.h>
#include <mach/common.h>

static unsigned int __init shmobile_smp_get_core_count(void)
{
#ifdef CONFIG_ARCH_SH73A0
	return sh73a0_get_core_count();
#endif

#ifdef CONFIG_ARCH_R8A7779
	return r8a7779_get_core_count();
#endif

#ifdef CONFIG_ARCH_R8A7373
	return r8a7373_get_core_count();
#endif

	return 1;
}

static void __init shmobile_smp_prepare_cpus(unsigned int max_cpus)
{
#ifdef CONFIG_ARCH_SH73A0
	sh73a0_smp_prepare_cpus();
#endif

#ifdef CONFIG_ARCH_R8A7779
	r8a7779_smp_prepare_cpus();
#endif

#ifdef CONFIG_ARCH_R8A7373
	r8a7373_smp_prepare_cpus(max_cpus);
#endif
}

int shmobile_platform_cpu_kill(unsigned int cpu)
{
#ifdef CONFIG_ARCH_R8A7779
	return r8a7779_platform_cpu_kill(cpu);
#endif

#ifdef CONFIG_ARCH_R8A7373
	return r8a7373_platform_cpu_kill(cpu);
#endif

	return 1;
}

int shmobile_platform_cpu_die(unsigned int cpu)
{
#ifdef CONFIG_ARCH_R8A7373
	return r8a7373_platform_cpu_die(cpu);
#endif

	return 1;
}

void __cpuinit platform_secondary_init(unsigned int cpu)
{
	trace_hardirqs_off();

#ifdef CONFIG_ARCH_SH73A0
	sh73a0_secondary_init(cpu);
#endif

#ifdef CONFIG_ARCH_R8A7779
	r8a7779_secondary_init(cpu);
#endif

#ifdef CONFIG_ARCH_R8A7373
	r8a7373_secondary_init(cpu);
#endif
}

int __cpuinit boot_secondary(unsigned int cpu, struct task_struct *idle)
{
#ifdef CONFIG_ARCH_SH73A0
	return sh73a0_boot_secondary(cpu);
#endif

#ifdef CONFIG_ARCH_R8A7779
	return r8a7779_boot_secondary(cpu);
#endif

#ifdef CONFIG_ARCH_R8A7373
	return r8a7373_boot_secondary(cpu);
#endif

	return -ENOSYS;
}

void __init smp_init_cpus(void)
{
	unsigned int ncores = shmobile_smp_get_core_count();
	unsigned int i;

	if (ncores > nr_cpu_ids) {
		pr_warn("SMP: %u cores greater than maximum (%u), clipping\n",
			ncores, nr_cpu_ids);
		ncores = nr_cpu_ids;
	}

	for (i = 0; i < ncores; i++)
		set_cpu_possible(i, true);

	set_smp_cross_call(gic_raise_softirq);
}

void __init platform_smp_prepare_cpus(unsigned int max_cpus)
{
	shmobile_smp_prepare_cpus(max_cpus);
}
