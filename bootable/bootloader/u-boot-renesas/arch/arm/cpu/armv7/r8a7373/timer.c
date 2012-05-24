/*
 * Copyright (C) 2011 Renesas Electronics Corporation
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

#include <common.h>
#include <asm/io.h>

/* CMT10 */
#define CMCLKE		(CMT1_BASE + 0x1000)	/* 32-bit */
#define CMSTR		(CMT1_BASE + 0x00)	/* 32-bit */
#define CMCSR		(CMT1_BASE + 0x10)	/* 32-bit */
#define CMCNT		(CMT1_BASE + 0x14)	/* 32-bit */
#define CMCOR		(CMT1_BASE + 0x18)	/* 32-bit */

#define TIMER_CLOCK	32768

static ulong timestamp;
static ulong cmnext;

/* how many counter cycles in a jiffy */
#define CYCLES_PER_JIFFY \
	((TIMER_CLOCK + (CONFIG_SYS_HZ / 2)) / CONFIG_SYS_HZ)

static inline void cmt_timer_start(void)
{
	writel(1, CMSTR);
}

static inline void cmt_timer_stop(void)
{
	writel(0, CMSTR);
}

int timer_init(void)
{
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

	return 0;
}

void reset_timer(void)
{
	timestamp = 0;
	cmnext = readl(CMCNT) + CYCLES_PER_JIFFY;
}

ulong get_timer(ulong base)
{
	unsigned long cmcnt;

	/* Check to see if we have missed any timestamps */
	cmcnt = readl(CMCNT);
	while ((cmcnt - cmnext) < 0x7fffffff) {
		cmnext += CYCLES_PER_JIFFY;
		timestamp++;
	}

	return timestamp - base;
}

void set_timer(ulong t)
{
	timestamp = t;
	cmnext = readl(CMCNT) + CYCLES_PER_JIFFY;
}

void __udelay(unsigned long usec)
{
	ulong tmo = (usec * TIMER_CLOCK) / 1000000;

	if (!tmo)
		tmo = 1;

	tmo += readl(CMCNT);
	while ((tmo - readl(CMCNT)) < 0x7fffffff)
		;
}

/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On ARM it just returns the timer value.
 */
unsigned long long get_ticks(void)
{
	return get_timer(0);
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	return CONFIG_SYS_HZ;
}
