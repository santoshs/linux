/*
 * (C) Copyright 2010 Renesas Solutions Corp.
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

#ifndef __ASM_ARCH_R8A7373_MACRO_H__
#define __ASM_ARCH_R8A7373_MACRO_H__

#ifdef __ASSEMBLY__

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

#endif /* __ASSEMBLY__ */
#endif /* __ASM_ARCH_R8A7373_MACRO_H__ */
