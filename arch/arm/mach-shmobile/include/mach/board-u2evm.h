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
#include <mach/r8a73734.h>
#include <mach/gpio.h>


/**
 * SEC_RLTE_REV
 */
#define SEC_RLTE_REV0_0_0		0
#define SEC_RLTE_REV0_2_1		1
#define SEC_RLTE_REV0_2_2		2
#define SEC_RLTE_REV0_3_1		3
#define SEC_RLTE_REV0_4_0		4

/**
 * CMT13
 */

/**
 * Crash log configurations
 */
#define TMPLOG_ADDRESS	IO_ADDRESS(0x44801200)
#define TMPLOG_SIZE	IO_ADDRESS(0x00040000)
#define RMC_LOCAL_VERSION "150612"		/* ddmmyy (release time)*/

#ifndef CONFIG_IRQ_TRACE
#define TMPLOG_ADDRESS	IO_ADDRESS(0x44801200)
#define TMPLOG_SIZE	IO_ADDRESS(0x00040000)
#endif

#define CRASHLOG_R_LOCAL_VER_LOCATE	IO_ADDRESS(0x44801000)
#define CRASHLOG_R_LOCAL_VER_LENGTH		32

#define CRASHLOG_KMSG_LOCATE		IO_ADDRESS(0x44801020)
#define CRASHLOG_LOGCAT_MAIN_LOCATE	IO_ADDRESS(0x44801030)
#define CRASHLOG_LOGCAT_EVENT_LOCATE	IO_ADDRESS(0x44801040)
#define CRASHLOG_LOGCAT_RADIO_LOCATE	IO_ADDRESS(0x44801050)
#define CRASHLOG_LOGCAT_SYSTEM_LOCATE	IO_ADDRESS(0x44801060)

#ifdef ARCH_HAS_READ_CURRENT_TIMER
	extern spinlock_t       sh_cmt_lock; /* arch/arm/mach-shmobile/sh_cmt.c */
#endif
extern void shmobile_do_restart(char mode, const char *cmd, u32 debug_mode);
#endif /* __ASM_ARCH_BOARD_U2EVM_H*/
