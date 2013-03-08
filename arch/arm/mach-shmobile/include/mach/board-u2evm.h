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
 * SEC_RLTE_REV
 */
#define SEC_RLTE_REV0_0_0		0
#define SEC_RLTE_REV0_2_1		1
#define SEC_RLTE_REV0_2_2		2
#define SEC_RLTE_REV0_3_1		3
#define SEC_RLTE_REV0_4_0		4

/**
 * ION
 */
#define ION_HEAP_CAMERA_SIZE	(SZ_16M + SZ_2M)
#define ION_HEAP_CAMERA_ADDR	0x46600000
#define ION_HEAP_GPU_SIZE	SZ_4M
#define ION_HEAP_GPU_ADDR	0x48400000
#ifdef CONFIG_ION_R_MOBILE_USE_VIDEO_HEAP
#define ION_HEAP_VIDEO_SIZE	(SZ_16M + SZ_2M)
#define ION_HEAP_VIDEO_ADDR	0x4AE00000
#endif
#endif /* __ASM_ARCH_BOARD_U2EVM_H*/
