/*
 * arch/arm/mach-shmobile/include/mach/board-u2evm.h
 *
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
/**
 * GPIO port configurations
 */
#ifndef __ASM_ARCH_BOARD_U2EVM_H
#define __ASM_ARCH_BOARD_U2EVM_H


/**
 * RLTE_BOARD_REV
 */
#define RLTE_BOARD_REV_0_1_0		0
#define RLTE_BOARD_REV_0_1_1		0
#define RLTE_BOARD_REV_0_1_2		0
#define RLTE_BOARD_REV_0_2_1		1
#define RLTE_BOARD_REV_0_2_2		2
#define RLTE_BOARD_REV_0_2_3		2
#define RLTE_BOARD_REV_0_2_4		2
#define RLTE_BOARD_REV_0_3_1		3
#define RLTE_BOARD_REV_0_3_2		3
#define RLTE_BOARD_REV_0_4_1		4
#define RLTE_BOARD_REV_0_5_0		5

/**
 * ION
 */
#define ION_HEAP_GPU_SIZE	(SZ_4M + SZ_1M)
#define ION_HEAP_GPU_ADDR	0x46600000
#ifdef CONFIG_ION_R_MOBILE_USE_VIDEO_HEAP
#define ION_HEAP_VIDEO_SIZE	(SZ_16M + SZ_2M)
#define ION_HEAP_VIDEO_ADDR	0x4AE00000
#endif
#endif /* __ASM_ARCH_BOARD_U2EVM_H*/
