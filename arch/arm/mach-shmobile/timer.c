/*
 * SH-Mobile Timer
 *
 * Copyright (C) 2010  Magnus Damm
 * Copyright (C) 2002 - 2009  Paul Mundt
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
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <asm/mach/time.h>
#include <mach/common.h>

static void __init shmobile_late_time_init(void)
{
	struct clk *clkp;
	unsigned long lpj;

	/*
	 * Make sure all compiled-in early timers register themselves.
	 *
	 * Run probe() for two "earlytimer" devices, these will be the
	 * clockevents and clocksource devices respectively. In the event
	 * that only a clockevents device is available, we -ENODEV on the
	 * clocksource and the jiffies clocksource is used transparently
	 * instead. No error handling is necessary here.
	 */
	early_platform_driver_register_all("earlytimer");
	early_platform_driver_probe("earlytimer", 1 + NR_CPUS, 0);

#ifdef CONFIG_SH_TIMER_CMT_ARM
	shmobile_clocksource_init();
	shmobile_clockevent_init();
#endif

	/*
	 * Calculate loops_per_jiffy using System-CPU frequency if it's
	 * available, to avoid time-consuming boot-time auto-calibration.
	 */
	clkp = clk_get(NULL, "z_clk");
	if (!IS_ERR(clkp)) {
		lpj = clk_get_rate(clkp) + HZ/2;
		do_div(lpj, HZ);
		lpj_fine = lpj;
		clk_put(clkp);
	}
}

static void __init shmobile_timer_init(void)
{
	late_time_init = shmobile_late_time_init;
}

struct sys_timer shmobile_timer = {
	.init		= shmobile_timer_init,
};
