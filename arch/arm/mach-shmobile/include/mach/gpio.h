/*
 * Generic GPIO API and pinmux table support
 *
 * Copyright (c) 2008  Magnus Damm
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */
#ifndef __ASM_ARCH_GPIO_H
#define __ASM_ARCH_GPIO_H

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/sh_pfc.h>
#include <linux/io.h>

#ifdef CONFIG_GPIOLIB

static inline int irq_to_gpio(unsigned int irq)
{
	return -ENOSYS;
}

#else

#define __ARM_GPIOLIB_COMPLEX

#endif /* CONFIG_GPIOLIB */

/*
 * GPIO API supplement
 *
 * Generic GPIO library lacks of some functions; it doesn't provide any
 * APIs to specify pull-up or pull-down of the port, nor an API to disable
 * both the input and the output of the port.  These supplementary APIs
 * are to fill in the missing piece of generic GPIO APIs.
 *
 * Note that these APIs are supposed to be used _after_ primary port
 * configuration of each port has been done using generic GPIO APIs.
 */

static inline void gpio_direction_none(unsigned long reg)
{
	__raw_writeb(0, reg);
}

static inline void gpio_pull_off(unsigned long reg)
{
	__raw_writeb(__raw_readb(reg) & 0x3f, reg);
}

static inline void gpio_pull_up(unsigned long reg)
{
	__raw_writeb((__raw_readb(reg) & 0x3f) | 0xc0, reg);
}

static inline void gpio_pull_down(unsigned long reg)
{
	__raw_writeb((__raw_readb(reg) & 0x3f) | 0x80, reg);
}

extern void gpio_direction_none_port(int gpio);
extern void gpio_pull_off_port(int gpio);
extern void gpio_pull_up_port(int gpio);
extern void gpio_pull_down_port(int gpio);

#endif /* __ASM_ARCH_GPIO_H */
