/*
 * drivers/sec_hal/exports/sec_hal_cmn.h
 *
 * Copyright (c) 2012, Renesas Mobile Corporation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include <linux/types.h>

#define SEC_HAL_RES_OK          0x00000000
#define SEC_HAL_RES_FAIL        0x00000010
#define SEC_HAL_RES_PARAM_ERROR 0x00000020

/* **************************************************************************
** Function name      : sec_hal_memcpy
** Description        : request memcpy to TZ protected memory areas.
**                      Physical buffers should be cache cleaned before this
**                      operation, since RAM is directly read. Otherwise
**                      unwanted failures can occur.
** Parameters         : IN/--- uint32_t phys_dst
**                      IN/--- uint32_t phys_src
**                      IN/--- uint32_t size
** Return value       : uint32
**                      ==SEC_HAL_RES_OK operation successful
**                      failure otherwise.
** *************************************************************************/
uint32_t sec_hal_memcpy(uint32_t phys_dst,
                        uint32_t phys_src,
                        uint32_t size);

/* **************************************************************************
** Function name      : sec_hal_authenticate
** Description        : Authenticate memory area with given SWCERT.
**                      Physical buffers should be cache cleaned before this
**                      operation, since RAM is directly read. Otherwise
**                      unwanted failures can occur.
** Parameters         : IN/--- uint32_t phys_cert_addr
**                      IN/--- uint32_t cert_size
**                      OUT/--- uint32_t *obj_id, object id if successful
** Return value       : uint32
**                      ==SEC_HAL_RES_OK operation successful
**                      failure otherwise.
** *************************************************************************/
uint32_t sec_hal_authenticate(uint32_t phys_cert_addr,
                              uint32_t cert_size,
                              uint32_t *obj_id);

