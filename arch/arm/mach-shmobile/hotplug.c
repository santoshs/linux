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

#ifdef CONFIG_MACH_U2EVM
#ifdef CONFIG_SUSPEND
#include <linux/suspend.h>
#include <mach/pm.h>
extern void jump_systemsuspend(void);
#endif /* CONFIG_SUSPEND */
#endif /* CONFIG_MACH_U2EVM */


int platform_cpu_kill(unsigned int cpu)
{
	return 1;
}

void platform_cpu_die(unsigned int cpu)
{
#ifdef CONFIG_MACH_U2EVM

#ifdef CONFIG_SUSPEND
	if (get_shmobile_suspend_state() & PM_SUSPEND_MEM) {
		/*
		 * cpu state is "shutdown mode" will transition
		 * in this function.
		 */
		jump_systemsuspend();
		return;
	}
#endif /* CONFIG_SUSPEND */

#endif /* CONFIG_MACH_U2EVM */
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
	/*
	 * we don't allow CPU 0 to be shutdown (it is still too special
	 * e.g. clock tick interrupts)
	 */
	return cpu == 0 ? -EPERM : 0;
}
