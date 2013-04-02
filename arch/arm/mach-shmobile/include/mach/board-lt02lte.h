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

#ifndef __ASM_ARCH_BOARD_H
#define __ASM_ARCH_BOARD_H

#if defined(CONFIG_BATTERY_BQ27425)
#define BQ27425_ADDRESS (0xAA >> 1)
#define GPIO_FG_INT 44
#endif

#define GPIO_CHG_INT 19

#if defined(CONFIG_CHARGER_SMB328A)
#define SMB328A_ADDRESS (0xA9 >> 1)		//for smb 327
#endif

#if defined(CONFIG_CHARGER_SMB358)
#define CHARGER_I2C_SLAVE_ADDRESS (0xD4 >> 1)		//for smb 358
#endif

#if defined(CONFIG_USB_SWITCH_TSU6712)
#define TSU6712_ADDRESS (0x4A >> 1)  // mUSB_temp_20130308
#endif

#if defined(CONFIG_USB_SWITCH_TSU6712) || \
    defined(CONFIG_RT8973) || defined(CONFIG_RT8969)
#define GPIO_MUS_INT 41
#endif

/**
 * ION
 */
#define ION_HEAP_CAMERA_SIZE (SZ_16M + SZ_2M)
#define ION_HEAP_CAMERA_ADDR 0x46600000

#define ION_HEAP_GPU_SIZE (SZ_4M + SZ_2M)
#define ION_HEAP_GPU_ADDR 0x48400000

#ifdef CONFIG_ION_R_MOBILE_USE_VIDEO_HEAP
#define ION_HEAP_VIDEO_SIZE (SZ_16M + SZ_2M)
#define ION_HEAP_VIDEO_ADDR 0x4AE00000
#endif

#endif // __ASM_ARCH_BOARD_LT02LTE_H
