/*
 * rmu2_rwdt.h
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

#ifndef _LINUX_RWDT_H
#define _LINUX_RWDT_H

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
#endif	/* __KERNEL__ */

/* define macro declaration for IOCTL command numbers */
#define RWDT_MAGIC	'r'
#define IOCTL_RWDT_SOFT_RESET	_IO(RWDT_MAGIC, 1)

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

#ifndef IO_ADDRESS
#define IO_ADDRESS(x)	x
#endif	/* IO_ADDRESS */

#ifdef CONFIG_RWDT_DEBUG
#define RWDT_DEBUG(fmt, ...)	printk(KERN_DEBUG "" fmt, ##__VA_ARGS__)
#else /* CONFIG_RWDT_DEBUG */
#define RWDT_DEBUG(fmt, ...)
#endif /* CONFIG_RWDT_DEBUG */

/* register address define */
#define STBCHRB1		IO_ADDRESS(0xE6180041U)
#define SYSC_RESCNT2		IO_ADDRESS(0xE6188020U)
#define RWDT_BASE		IO_ADDRESS(0xE6020000U)
#define REG_SIZE		0xCU
#define RWTCNT_OFFSET           0x0U
#define RWTCSRA			0x4U
#define RWTCSRB			0x8U

/* register mask define */
#define RESCSR_HEADER		0xA5A5A500U
#define RESCNT_CLEAR_DATA	0x5A5AFF00U
#define RESCNT2_RWD0A_MASK	0x00003000U
#define RESCNT2_PRES_MASK	0x80000000U
#define RWTCSRA_TME_MASK	0x80U
#define RWTCSRA_WOVF_MASK	0x10U
#define RWTCSRA_WOVFE_MASK	0x08U
#define RWTCSRA_CSK0_MASK	0x07U
#define RWDT_SPI		141U

/* wait time define */
#define WRFLG_WAITTIME		214000	/* [nsec] 7RCLK */

/* default starting cpu number */
#define DEFAULT_CPU_NUMBER	0U

int rmu2_rwdt_cntclear(void);
int rmu2_rwdt_stop(void);
void rmu2_rwdt_software_reset(void);
#endif  /* _LINUX_RWDT_H */

/* End of File */

