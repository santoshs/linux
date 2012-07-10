/*
 * Copyright (C) 2012 Renesas Mobile Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
*/
/**
 * @file
 * @brief Timer handling for EOS CPUs
 */

#include <config.h>
#include <common.h>
#include <init.h>
#include <clock.h>
#include <asm/io.h>
#include <mach/hardware-base.h>
#include <generated/mach-types.h>

/*
 * cmt_timer_start()
 * Input		:None
 * Output		:None
 * Return		:None
 */
static inline void cmt_timer_start(void)
{
	writel(1, CMSTR);
}

/*
 * cmt_timer_stop()
 * Input		:None
 * Output		:None
 * Return		:None
 */
static inline void cmt_timer_stop(void)
{
	writel(0, CMSTR);
}

/*
 * timer_init()
 * Input		:None
 * Output		:None
 * Return		:None
 */
static void timer_init(void)
{
	/* CMT1 */
	writel(readl(SMSTPCR3) & ~(1 << 29), SMSTPCR3); /* MSTP329 */
	
	writel(1 << 0, CMCLKE); /* CMT10 */
	cmt_timer_stop();
	writel(0x10f, CMCSR);	/* Free-run, DBGIVD, RCLK/1 */
	writel(0xffffffff, CMCOR);
	writel(0, CMCNT);

	/*
	 * Two counter input clock cycles are necessary before this
	 * register is either read or any modification of the value
	 * it holds is reflected in the LSI's actual operation.
	 */
	while (readl(CMCNT))
		;

	cmt_timer_start();

	return;
}

/*
 * r_mobile_cmt_clocksource_read()
 * Input		:None
 * Output		:None
 * Return		:None
 */
uint64_t r_mobile_cmt_clocksource_read(void)
{
	return readl(CMCNT);
}

static struct clocksource cs = {
	.read	= r_mobile_cmt_clocksource_read,
	.mask	= CLOCKSOURCE_MASK(32),
	.shift	= 0,
};

/*
 * clocksource_init()
 * Input		:None
 * Output		:None
 * Return		:None
 */
static int clocksource_init (void)
{
	timer_init();
	cs.mult = clocksource_hz2mult(TIMER_CLOCK, cs.shift);
	
	return init_clock(&cs);
}

core_initcall(clocksource_init);

static int cpu_info(void)
{
	unsigned int cccr, major, minor;

	cccr = readl(CCCR);
	major = ((cccr & 0xf0) >> 4) + 1;
	minor = cccr & 0x0f;

	printf("CPU: Renesas ES%d.%d\n", major, minor);

	return 0;
}
postconsole_initcall(cpu_info);
