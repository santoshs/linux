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

#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/poll.h>
#include <linux/mm.h>
#include <linux/ioctl.h>
#include <asm/page.h>
#else
#include <sys/ioctl.h>
#endif  /* __KERNEL__ */

#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/smp.h>
#include <linux/sched.h>
#include <linux/clk.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <mach/common.h>
#include <mach/irqs.h>
#include <mach/r8a7373.h>

#ifdef CONFIG_RWDT_CMT15_TEST
void loop(void *info);

/* Various nasty things we can do to the system to test the watchdog and
 * CMT timer. Example: "echo 8 > /proc/proc_watch_entry"
 */
static int test_mode;

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
#endif

#ifndef IO_ADDRESS
#define IO_ADDRESS(x)   x
#endif  /* IO_ADDRESS */

#define CONFIG_GIC_NS_CMT

/* Macro definition */
#define CPG_CHECK_REG		IO_ADDRESS(0xE61503D0U)
#define CPG_CHECK_STATUS	IO_ADDRESS(0xE61503DCU)
#define CPG_CHECK_MODULES	IO_ADDRESS(0xE6150440U)

#ifdef CONFIG_GIC_NS_CMT
#define CMSTR15			IO_ADDRESS(0xE6130500U)
#define CMCSR15			IO_ADDRESS(0xE6130510U)
#define CMCNT15			IO_ADDRESS(0xE6130514U)
#define CMCOR15			IO_ADDRESS(0xE6130518U)
#define CMT15_SPI		98U

#define ICD_ISR0 0xF0001080
#define ICD_IPR0 0xF0001400
#define ICD_IPTR0 0xf0001800

/* FIQ handle excecute panic before RWDT request CPU reset system */
#define CMT_OVF			((256*CONFIG_RMU2_RWDT_CMT_OVF)/1000 - 2)

static inline u32 dec2hex(u32 dec)
{
	return dec;
}
#endif  /* CONFIG_GIC_NS_CMT */

void cpg_check_check(void);
void rmu2_cmt_stop(void);
void rmu2_cmt_clear(void);

#endif /* _RMC_CMT15_H */
