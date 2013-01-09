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

#define ARCH_NR_GPIOS 1024
#include <linux/sh_pfc.h>

#ifdef CONFIG_GPIOLIB

#define GPIO_BASE	IO_ADDRESS(0xe6050000)
/* gpio address calculation */
#define GPIO_PORTCR(n) ({ \
((n) < 96) ? (GPIO_BASE + 0x0000 + (n)) : \
((n) < 128) ? (GPIO_BASE + 0x0000 + (n) +  \
	((system_rev&(~0x3E00)) ? 0 : 0x1000)) : \
((n) < 144) ? (GPIO_BASE + 0x1000 + (n)) : \
((n) < 192) ? 0 : \
((n) < 320) ? (GPIO_BASE + 0x2000 + (n)) : \
((n) < 328) ? (GPIO_BASE + 0x2000 + (n) + \
	((system_rev&(~0x3E00)) ? 0 : 0x1000)) : 0; })


/* GPIO Settings - PULMD (Pull OFF/Pull DOWN/Pull UP) */
#define PORTn_CR_PULL_NOT_SET	-1
#define PORTn_CR_PULL_OFF	0
#define PORTn_CR_PULL_DOWN	1
#define PORTn_CR_PULL_UP	2

/* GPIO Settings - IE/OE */
#define PORTn_CR_DIRECTION_NOT_SET	(-1)
#define PORTn_CR_DIRECTION_NONE		0
#define PORTn_CR_DIRECTION_OUTPUT	1
#define PORTn_CR_DIRECTION_INPUT	2

/* GPIO Settings - Output data level High/Low */
#define PORTn_OUTPUT_LEVEL_NOT_SET	(-1)
#define PORTn_OUTPUT_LEVEL_LOW		0
#define PORTn_OUTPUT_LEVEL_HIGH		1

/* GPIO Mask Values */
#define GPIO_DIRECTION_NONE	0xcf
#define GPIO_PULL_OFF		0x3f
#define GPIO_PULL_UP		0xc0
#define GPIO_PULL_DOWN		0x80
#define GPIO_BIDIRECTION	0x30
#define INPUT		0x20
#define OUTPUT		0x10
#define FUNCTION_0	0x00
#define FUNCTION_1	0x01

struct portn_gpio_setting {
	u32	port_fn;	/* Pin function select*/
	s32	pull;		/* Pull Off/Down/Up */
	s32 direction;  /* Input/Output direction */
	/* It become enable only when direction is output. */
	s32 output_level; 
};

struct portn_gpio_setting_info {
	u32	flag;		/* 0: no change required on suspend 
					   1: change required on suspend*/
	u32	port;		/* GPIO port number */
	/* GPIO settings to be retained on resume */
	struct portn_gpio_setting active; 
	/* GPIO settings on suspending state */
	struct portn_gpio_setting inactive; 
};

static inline int gpio_get_value(unsigned gpio)
{
	return __gpio_get_value(gpio);
}

static inline void gpio_set_value(unsigned gpio, int value)
{
	__gpio_set_value(gpio, value);
}

static inline int gpio_cansleep(unsigned gpio)
{
	return __gpio_cansleep(gpio);
}

static inline int gpio_to_irq(unsigned gpio)
{
	return __gpio_to_irq(gpio);
}

static inline int irq_to_gpio(unsigned int irq)
{
	return -ENOSYS;
}

extern void gpio_direction_none_port(int gpio);
extern void gpio_pull_off_port(int gpio);
extern void gpio_pull_up_port(int gpio);
extern void gpio_pull_down_port(int gpio);
extern void gpio_bidirection_port(int gpio);

#endif /* CONFIG_GPIOLIB */

#endif /* __ASM_ARCH_GPIO_H */
