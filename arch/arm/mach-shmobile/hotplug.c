/*
 * SMP support for R-Mobile / SH-Mobile
 *
 * Copyright (C) 2010  Magnus Damm
 * Copyright (C) 2012 Renesas Mobile Corporation
 *
 * Based on realview, Copyright (C) 2002 ARM Ltd, All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/smp.h>
#include <linux/delay.h>
#include <asm/cacheflush.h>
#include <mach/common.h>
#ifdef CONFIG_ARCH_R8A73734
#ifdef CONFIG_SUSPEND
#include <linux/suspend.h>
#endif /* CONFIG_SUSPEND */
#endif /* CONFIG_ARCH_R8A73734 */
#include <mach/pm.h>
static cpumask_t dead_cpus;
int platform_cpu_kill(unsigned int cpu)
{
#ifndef CONFIG_ARCH_R8A73734
	int cnt = 0;
	/* this will be executed on alive cpu,
	 * and it must be executed after the victim has been finished
	 */
	for (cnt = 0; cnt < 1000; cnt++) {
		if (cpumask_test_cpu(cpu, &dead_cpus))
			return shmobile_platform_cpu_kill(cpu);
		mdelay(1);
	}
#else
	return shmobile_platform_cpu_kill(cpu);
#endif
	return 1;
}

void platform_cpu_die(unsigned int cpu)
{
#ifdef CONFIG_ARM_TZ
	int ret = 0;
#endif
	flush_cache_all();
	dsb();

	/* notify platform_cpu_kill() that hardware shutdown is finished */
	cpumask_set_cpu(cpu, &dead_cpus);
#ifdef CONFIG_ARCH_R8A73734
	if (!shmobile_platform_cpu_die(cpu))
		return;
/* #ifdef CONFIG_SUSPEND */
	jump_systemsuspend();
	return;
/* #endif *//* CONFIG_SUSPEND */
#endif /* CONFIG_ARCH_R8A73734 */
	while (1) {
		/*
		 * here's the WFI
		 */
		asm(".word	0xe320f003\n"
		    :
		    :
		    : "memory", "cc");
	}
}

int platform_cpu_disable(unsigned int cpu)
{
	cpumask_clear_cpu(cpu, &dead_cpus);
	/*
	 * we don't allow CPU 0 to be shutdown (it is still too special
	 * e.g. clock tick interrupts)
	 */
	return cpu == 0 ? -EPERM : 0;
}
