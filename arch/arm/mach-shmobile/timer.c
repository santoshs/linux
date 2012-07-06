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
#include <asm/mach/time.h>

static void __init shmobile_late_time_init(void)
{
	struct clk *clkp;
	unsigned long lpj;

	if (lpj_fine)
		return; /* seems to be set up for the current timer */

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

void __init shmobile_earlytimer_init(void)
{
	late_time_init = shmobile_late_time_init;
}

static void __init shmobile_timer_init(void)
{
}

struct sys_timer shmobile_timer = {
	.init		= shmobile_timer_init,
};
