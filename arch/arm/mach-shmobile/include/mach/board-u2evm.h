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
#define GPIO_BASE	IO_ADDRESS(0xe6050000)

#ifndef __ASM_ARCH_BOARD_U2EVM_H
#define __ASM_ARCH_BOARD_U2EVM_H

/**
 * SDHI
 */
#define SDHI1_CLK_CR	IO_ADDRESS(0xE6052120)
#define SDHI1_D0_CR	IO_ADDRESS(0xE6052121)
#define SDHI1_D1_CR	IO_ADDRESS(0xE6052122)
#define SDHI1_D2_CR	IO_ADDRESS(0xE6052123)
#define SDHI1_D3_CR	IO_ADDRESS(0xE6052124)
#define SDHI1_CMD_CR	IO_ADDRESS(0xE6052125)

/**
 * MSEL3CR
 */
#define MSEL3CR		IO_ADDRESS(0xE6058020)

/**
 * SEC_RLTE_REV
 */
#define SEC_RLTE_REV0_0_0		0
#define SEC_RLTE_REV0_2_1		1
#define SEC_RLTE_REV0_2_2		2
#define SEC_RLTE_REV0_3_1		3
#define SEC_RLTE_REV0_4_0		4
/**
 * GPIO
 **/
#define GPIO_PULL_OFF	0x00
#define GPIO_PULL_DOWN	0x80
#define GPIO_PULL_UP	0xc0

/*Support for compatibility between ES1.0 and ES2.0*/
#define GPIO_PORTCR_ES1(n)	({				\
	((n) <  96) ? (GPIO_BASE + 0x0000 + (n)) :	\
	((n) < 128) ? (GPIO_BASE + 0x1000 + (n)) :	\
	((n) < 144) ? (GPIO_BASE + 0x1000 + (n)) :	\
	((n) < 192) ? 0 :				\
	((n) < 320) ? (GPIO_BASE + 0x2000 + (n)) :	\
	((n) < 328) ? (GPIO_BASE + 0x3000 + (n)) : 0; })

#define GPIO_PORTCR_ES2(n)	({				\
	((n) <  96) ? (GPIO_BASE + 0x0000 + (n)) :	\
	((n) < 128) ? (GPIO_BASE + 0x0000 + (n)) :	\
	((n) < 144) ? (GPIO_BASE + 0x1000 + (n)) :	\
	((n) < 192) ? 0 :				\
	((n) < 320) ? (GPIO_BASE + 0x2000 + (n)) :	\
	((n) < 328) ? (GPIO_BASE + 0x2000 + (n)) : 0; })
/**
 * CMT13
 */
#define CMSTR3		IO_ADDRESS(0xe6130300)
#define CMCSR3		IO_ADDRESS(0xe6130310)
#define CMCNT3		IO_ADDRESS(0xe6130314)
#define CMCOR3		IO_ADDRESS(0xe6130318)
#define CMCLKE		IO_ADDRESS(0xe6131000)

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
#endif /* __ASM_ARCH_BOARD_U2EVM_H*/
