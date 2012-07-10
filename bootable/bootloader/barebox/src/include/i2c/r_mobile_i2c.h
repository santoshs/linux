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

#ifndef __R_MOBILE_I2C_H__
#define __R_MOBILE_I2C_H__

#include <clock.h>
#include <common.h>
#include <driver.h>
#include <init.h>
#include <malloc.h>
#include <types.h>
#include <xfuncs.h>

#include <linux/err.h>

#include <asm/io.h>
#include <i2c/i2c.h>
#include <mach/hardware-base.h>


/* HP clock */
typedef enum {
    I2C_HPCLK_52MHZ = 0,        /* HP clock 52MHz */
    I2C_HPCLK_104MHZ            /* HP clock 104MHz */
} I2C_HPCLK;


/* I2C(IIC0) register address */
#define ICDR0	(I2C0_BASE + 0x0000)		/* I2C bus dara register 2 */
#define ICCR0	(I2C0_BASE + 0x0004)		/* I2C bus control register 2 */
#define ICSR0	(I2C0_BASE + 0x0008)		/* I2C bus status register 2 */
#define ICIC0	(I2C0_BASE + 0x000C)		/* I2C interruption control register 2 */
#define ICCL0	(I2C0_BASE + 0x0010)		/* I2C clock control register low 2 */
#define ICCH0	(I2C0_BASE + 0x0014)		/* I2C clock control register high 2 */

#define SMSTPCR1_116		(0x00010000)	/* SMSTPCR1 - Module Stop bit 116 (Controls clock supply to IIC0) */
#define SRCR1_SRT116		(0x00010000)	/* SRCR1 - Software Reset bit 116 (Issues the reset to IIC0) */
#define NORMAL_SPEED		100000 /* FAST_SPEED 400000 */

/* Function Prototypes */
void i2c_init(void);
void i2c_set_Hp(I2C_HPCLK hpclk);
int i2c_write(uchar slave_addr, uchar register_addr, uchar w_data);
int i2c_read(uchar slave_addr, uchar register_addr, uchar* r_data);

struct r_mobile_mobile_i2c_data {
	struct device *dev;
	void __iomem *reg;
	struct i2c_adapter adapter;

	void 			*base;
	struct resource		*ioarea;
	u32			speed;		/* Speed of bus in Khz */
};

#endif /* __R_MOBILE_I2C_H__ */