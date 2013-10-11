/*
 * drivers/sec_hal/rt/inc/sec_dispatch.h
 *
 * Copyright (c) 2012-2013, Renesas Mobile Corporation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#ifndef SEC_DISPATCH_H
#define SEC_DISPATCH_H

#include <linux/types.h>


uint32_t raw_pub2sec_dispatcher(uint32_t appl_id, uint32_t flags, ...);

uint32_t pub2sec_dispatcher(uint32_t appl_id, uint32_t flags, ...);


#endif /* SEC_DISPATCH_H */

