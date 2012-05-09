/* 
 * arch/arm/mach-shmobile/ramdefrag/ram_defrag_internal.h
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

/* Internal declaration of RAM defragmentation (global variable, prototype, etc.) */
#ifndef __ARCH_RAM_DEFRAG_INTERNAL_H__
#define __ARCH_RAM_DEFRAG_INTERNAL_H__

#include <linux/types.h> /* ssize_t */
#include <mach/ram_defrag.h>

static int page_check(struct page *page);

#endif /*__ARCH_RAM_DEFRAG_INTERNAL_H__*/
