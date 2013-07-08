/*
 * rmu2_cmt15.h
 *
 * Copyright (C) 2013 Renesas Mobile Corporation
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

#ifndef _RMC_CMT15_H__
#define _RMC_CMT15_H__

#ifdef CONFIG_RWDT_CMT15_TEST
void rmu2_cmt_loop(void *info);

/* Various nasty things we can do to the system to test the watchdog and
 * CMT timer. Example: "echo 8 > /proc/proc_watch_entry"
 */
enum crash_type {
	TEST_NORMAL = 0,		/* Normal operation, watchdog kicked */
	TEST_NO_KICK = 1,		/* Normal system, watchdog not kicked */
	TEST_LOOP = 2,			/* Infinite loop (1 CPU) */
	TEST_PREEMPT_LOOP = 3,		/* Infinite loop (1 CPU, preempt off) */

	/* Infinite loop (all CPUs, preempt off)*/
	TEST_LOOP_ALL = 4,
	TEST_IRQOFF_LOOP = 5,		/* IRQ-off infinite loop (1 CPU) */
	TEST_IRQOFF_LOOP_ALL = 6,	/* IRQ-off infinite loop (all CPUs) */
	TEST_WORKQUEUE_LOOP = 7,	/* Infinite loop in 1 workqueue */

	/* Infinite loop in IRQ handler (all CPUs) */
	TEST_IRQHANDLER_LOOP = 8,
	TEST_FIQOFF_LOOP = 9,		/* FIQ+IRQ-off infinite loop (1 CPU) */

	/* FIQ+IRQ-off on 1 CPU, IRQ-off on others */
	TEST_FIQOFF_1_LOOP_ALL = 10,

	/* FIQ+IRQ-off infinite loop (all CPUs) */
	TEST_FIQOFF_LOOP_ALL = 11,
};

extern int test_mode;
#endif

#ifdef CONFIG_RMU2_CMT15
void cpg_check_check(void);
void rmu2_cmt_stop(void);
void rmu2_cmt_clear(void);
#else
static inline void cpg_check_check(void)
{
}
static inline void rmu2_cmt_stop(void)
{
}
static inline void rmu2_cmt_clear(void)
{
}
#endif

#endif /* _RMC_CMT15_H */
