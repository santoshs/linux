/*
 * arch/arm/mach-shmobile/include/mach/crashlog.h
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

#ifndef __ARCH_SHMOBILE_CRASHLOG_H__
#define __ARCH_SHMOBILE_CRASHLOG_H__

#include <mach/memory-r8a7373.h>

#define CRASHLOG_R_LOCAL_VER_LOCATE             (SDRAM_CRASHLOG_START_ADDR + 0)
#define CRASHLOG_R_LOCAL_VER_LENGTH             32

#define CRASHLOG_KMSG_LOCATE		(SDRAM_CRASHLOG_START_ADDR + 0x20)
#define CRASHLOG_LOGCAT_MAIN_LOCATE	(SDRAM_CRASHLOG_START_ADDR + 0x30)
#define CRASHLOG_LOGCAT_EVENT_LOCATE	(SDRAM_CRASHLOG_START_ADDR + 0x40)
#define CRASHLOG_LOGCAT_RADIO_LOCATE	(SDRAM_CRASHLOG_START_ADDR + 0x50)
#define CRASHLOG_LOGCAT_SYSTEM_LOCATE	(SDRAM_CRASHLOG_START_ADDR + 0x60)

#if 1 /*TODO: replace with config option */
extern void set_log_info(unsigned long store_addr,
		unsigned long buffer_addr, unsigned long buffer_size_addr,
		unsigned long w_off_add, unsigned long head_addr);
#else
static inline void set_log_info(unsigned long store_addr,
		unsigned long buffer_addr, unsigned long buffer_size_addr,
		unsigned long w_off_add, unsigned long head_addr){}
#endif

void crashlog_kmsg_init(void);

#endif /* __ARCH_SHMOBILE_CRASHLOG_H__ */
