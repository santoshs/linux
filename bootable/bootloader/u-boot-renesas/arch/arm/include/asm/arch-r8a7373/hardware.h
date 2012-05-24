/*
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

#ifndef __ASM_ARCH_R8A7373_HARDWARE_H
#define __ASM_ARCH_R8A7373_HARDWARE_H

/*
 * I/O Addresses
 */

#define MERAM_BASE	0xE5580000

#define HPB_BASE	0xE6000000

#define RWDT_BASE	0xE6020000

#define SWDT_BASE	0xE6030000

#define GPIO_BASE	0xE6050000

#define CMT1_BASE	0xE6130000

#define CPG_BASE	0xE6150000

#define MMCIF_BASE	0xE6BD0000

#define SCIF0_BASE	0xE6450000

#define SDHI0_BASE	0xEE100000

#define SBSC1_BASE	0xFE400000

#define BSC_BASE	0xFEC10000

/* GPIO */
#define PORTL031_000DR		(GPIO_BASE + 0x4000)
#define PORTL031_000DSR		(GPIO_BASE + 0x4100)
#define PORTL031_000DCR		(GPIO_BASE + 0x4200)

#endif /* __ASM_ARCH_R8A7373_HARDWARE_H */
