/*
 * SMP support for R-Mobile / SH-Mobile - local timer portion
 *
 * Copyright (C) 2010  Magnus Damm
 *
 * Based on vexpress, Copyright (C) 2002 ARM Ltd, All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/smp.h>
#include <linux/clockchips.h>
#include <asm/localtimer.h>

/*
 * Setup the local clock events for a CPU.
 */
#ifdef CONFIG_HAVE_ARM_TWD
int __cpuinit local_timer_setup(struct clock_event_device *evt)
{
	evt->irq = 29;
	twd_timer_setup(evt);
	return 0;
}
#else /* CONFIG_HAVE_ARM_TWD */

extern int cmt_timer_setup(struct clock_event_device *clk);

int local_timer_ack(void)
{
	return 1;
}

int __cpuinit local_timer_setup(struct clock_event_device *evt)
{
	return cmt_timer_setup(evt);
}

#endif /* CONFIG_HAVE_ARM_TWD */
