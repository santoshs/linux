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

#include <mach/r8a7373.h>
#include <linux/sh_pfc.h>
#include <linux/io.h>

#ifdef CONFIG_GPIOLIB

#define GPIO_BASE      IO_ADDRESS(0xe6050000)
/* gpio address calculation */
#define GPIO_PORTCR(n) ({		 \
((n) < 96) ? (GPIO_BASE + 0x0000 + (n)) : \
((n) < 128) ? (GPIO_BASE + 0x0000 + (n)) :  \
((n) < 144) ? (GPIO_BASE + 0x1000 + (n)) : \
((n) < 192) ? 0 : \
((n) < 320) ? (GPIO_BASE + 0x2000 + (n)) : \
((n) < 328) ? (GPIO_BASE + 0x2000 + (n)) : 0; })

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
#define PORTn_CR_BI_DIRECTIONAL		3

/* GPIO Settings - Output data level High/Low */
#define PORTn_OUTPUT_LEVEL_NOT_SET      (-1)
#define PORTn_OUTPUT_LEVEL_LOW          0
#define PORTn_OUTPUT_LEVEL_HIGH         1

/* GPIO Mask Values */
#define GPIO_DIRECTION_NONE     0xcf
#define GPIO_PULL_OFF           0x3f
#define GPIO_PULL_UP            0xc0
#define GPIO_PULL_DOWN          0x80
#define GPIO_BIDIRECTION        0x30
#define INPUT           0x20
#define OUTPUT          0x10
#define FUNCTION_0      0x00
#define FUNCTION_1      0x01

struct portn_gpio_setting {
	u32	port_fn;	/* Pin function select*/
	s32	pull;		/* Pull Off/Down/Up */
	s32	direction;	/* Input/Output direction */
	/* It become enable only when direction is output. */
	s32 output_level;
};

struct portn_gpio_setting_info {
	u32	flag;	/* 0: no change required on suspend
			   1: change required on suspend*/
	u32	port;	/* GPIO port number */
	/* GPIO settings to be retained on resume */
	struct portn_gpio_setting active;
	/* GPIO settings on suspending state */
	struct portn_gpio_setting inactive;
};
static int unused_gpios_garda_rev1[] = {
		GPIO_PORT0, GPIO_PORT4, GPIO_PORT5, GPIO_PORT8,
		GPIO_PORT9, GPIO_PORT10, GPIO_PORT14, GPIO_PORT15,
		GPIO_PORT17, GPIO_PORT22, GPIO_PORT23, GPIO_PORT24,
		GPIO_PORT25, GPIO_PORT30, GPIO_PORT34, GPIO_PORT35,
		GPIO_PORT39, GPIO_PORT40, GPIO_PORT41, GPIO_PORT42,
		GPIO_PORT43, GPIO_PORT46, GPIO_PORT64, GPIO_PORT65,
		GPIO_PORT66, GPIO_PORT70, GPIO_PORT71, GPIO_PORT80,
		GPIO_PORT81, GPIO_PORT82, GPIO_PORT83, GPIO_PORT86,
		GPIO_PORT87, GPIO_PORT88, GPIO_PORT89, GPIO_PORT90,
		GPIO_PORT96, GPIO_PORT102, GPIO_PORT103, GPIO_PORT104,
		GPIO_PORT105, GPIO_PORT107, GPIO_PORT109, GPIO_PORT131,
		GPIO_PORT140, GPIO_PORT141, GPIO_PORT142, GPIO_PORT198,
		GPIO_PORT199, GPIO_PORT200, GPIO_PORT201, GPIO_PORT202,
		GPIO_PORT219, GPIO_PORT224, GPIO_PORT225, GPIO_PORT227,
		GPIO_PORT228, GPIO_PORT229, GPIO_PORT230, GPIO_PORT231,
		GPIO_PORT232, GPIO_PORT233, GPIO_PORT234, GPIO_PORT235,
		GPIO_PORT236, GPIO_PORT237, GPIO_PORT238, GPIO_PORT239,
		GPIO_PORT240, GPIO_PORT241, GPIO_PORT242, GPIO_PORT243,
		GPIO_PORT244, GPIO_PORT245, GPIO_PORT246, GPIO_PORT247,
		GPIO_PORT248, GPIO_PORT249, GPIO_PORT250, GPIO_PORT251,
		GPIO_PORT252, GPIO_PORT253, GPIO_PORT254, GPIO_PORT255,
		GPIO_PORT256, GPIO_PORT257, GPIO_PORT258, GPIO_PORT259,
		GPIO_PORT271, GPIO_PORT275, GPIO_PORT276, GPIO_PORT277,
		GPIO_PORT294, GPIO_PORT295, GPIO_PORT296, GPIO_PORT297,
		GPIO_PORT298, GPIO_PORT299, GPIO_PORT311, GPIO_PORT312,
		GPIO_PORT325,
};
static int unused_gpios_garda_rev2[] = {
		GPIO_PORT0, GPIO_PORT4, GPIO_PORT5, GPIO_PORT8,
		GPIO_PORT9, GPIO_PORT10, GPIO_PORT14, GPIO_PORT15,
		GPIO_PORT17, GPIO_PORT22, GPIO_PORT23, GPIO_PORT24,
		GPIO_PORT25, GPIO_PORT29, GPIO_PORT30, GPIO_PORT34,
		GPIO_PORT35, GPIO_PORT39, GPIO_PORT40, GPIO_PORT41,
		GPIO_PORT42, GPIO_PORT43, GPIO_PORT64, GPIO_PORT65,
		GPIO_PORT66, GPIO_PORT70, GPIO_PORT71, GPIO_PORT80,
		GPIO_PORT81, GPIO_PORT82, GPIO_PORT83, GPIO_PORT86,
		GPIO_PORT87, GPIO_PORT88, GPIO_PORT89, GPIO_PORT90,
		GPIO_PORT96, GPIO_PORT102, GPIO_PORT103, GPIO_PORT104,
		GPIO_PORT105, GPIO_PORT107, GPIO_PORT109, GPIO_PORT140,
		GPIO_PORT141, GPIO_PORT142, GPIO_PORT198, GPIO_PORT199,
		GPIO_PORT200, GPIO_PORT201, GPIO_PORT202, GPIO_PORT219,
		GPIO_PORT224, GPIO_PORT225, GPIO_PORT227, GPIO_PORT228,
		GPIO_PORT229, GPIO_PORT230, GPIO_PORT231, GPIO_PORT232,
		GPIO_PORT233, GPIO_PORT234, GPIO_PORT235, GPIO_PORT236,
		GPIO_PORT237, GPIO_PORT238, GPIO_PORT239, GPIO_PORT240,
		GPIO_PORT241, GPIO_PORT242, GPIO_PORT243, GPIO_PORT244,
		GPIO_PORT245, GPIO_PORT246, GPIO_PORT247, GPIO_PORT248,
		GPIO_PORT249, GPIO_PORT250, GPIO_PORT251, GPIO_PORT252,
		GPIO_PORT253, GPIO_PORT254, GPIO_PORT255, GPIO_PORT256,
		GPIO_PORT257, GPIO_PORT258, GPIO_PORT259, GPIO_PORT271,
		GPIO_PORT275, GPIO_PORT276, GPIO_PORT277, GPIO_PORT294,
		GPIO_PORT295, GPIO_PORT296, GPIO_PORT297, GPIO_PORT298,
		GPIO_PORT299, GPIO_PORT311, GPIO_PORT312, GPIO_PORT325,
};

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

extern void gpio_direction_none_port(int gpio);
extern void gpio_pull_off_port(int gpio);
extern void gpio_pull_up_port(int gpio);
extern void gpio_pull_down_port(int gpio);
extern void gpio_bidirection_port(int gpio);
extern void unused_gpio_port_init(int gpio);
extern void gpio_set_portncr_value(unsigned int port_count,
	struct portn_gpio_setting_info *gpio_setting_info, int suspend_mode);

#endif /* __ASM_ARCH_GPIO_H */
