/*
 * arch/arm/mach-shmobile/include/mach/board-loganlte.h
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

#ifndef __ASM_ARCH_BOARD_LOGANLTE_H
#define __ASM_ARCH_BOARD_LOGANLTE_H


/**
 * Camera
 */

/**
 * RLTE_BOARD_REV
 */
#define RLTE_BOARD_REV_0_0	0
#define RLTE_BOARD_REV_0_1	1
#define RLTE_BOARD_REV_0_2	2
/**
 * ION
 */
#ifdef CONFIG_ION_R_MOBILE_USE_VIDEO_HEAP
#define ION_HEAP_VIDEO_SIZE	(SZ_16M + SZ_2M)
#define ION_HEAP_VIDEO_ADDR	0x4AE00000
#endif

#endif /* __ASM_ARCH_BOARD_LOGANLTE_H */
