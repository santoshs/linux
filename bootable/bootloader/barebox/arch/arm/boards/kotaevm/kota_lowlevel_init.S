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

#include <config.h>
#include <mach/hardware-base.h>

/* WDT */
#define RWTCSRA0	0xE6020004
#define SWTCSRA0	0xE6030004

#ifdef CONFIG_MACH_DO_LOWLEVEL_INIT
/* assembly macro */
.macro	write32, addr, data
	ldr	r4, =\addr
	ldr	r5, =\data
	str	r5, [r4]
.endm

.macro	setbits addr, mask
	ldr	r2, =\addr
	ldr	r3, =\mask
	ldr	r1, [r2, #0]
	orr	r3, r1, r3
	str	r3, [r2, #0]
.endm

.macro	clearbits addr, mask
	ldr	r2, =\addr
	ldr	r3, =\mask
	ldr	r1, [r2, #0]
	bic	r3, r1, r3
	str	r3, [r2, #0]
.endm

.macro	polling addr, mask, comp
	ldr	r0, =\addr
	ldr	r1, =\mask
	ldr	r2, =\comp
2:
	ldr	r3, [r0, #0]
	and	r3, r1, r3
	cmp	r2, r3
	bne	2b
.endm

.globl board_init_lowlevel

board_init_lowlevel:
	/* Enable I-Cache */
	mov	r0, #0
	mcr	p15, 0, r0, c7, c5, 0		@ invalidate I cache
	mcr	p15, 0, r0, c7, c5, 6		@ flush BTAC/BTB
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, r0, #0x1800			@ ...IZ...........
	mcr	p15, 0, r0, c1, c0, 0
	
	/* WDT */
	write32		RWTCSRA0, 0xA5A5A507	@ halt
	write32		SWTCSRA0, 0xA5A5A507	@ halt

	/* back to arch calling code */
	bx	lr			@ return to caller

	.pool

#endif /* CONFIG_MACH_DO_LOWLEVEL_INIT */
