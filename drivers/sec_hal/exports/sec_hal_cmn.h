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
#ifndef SEC_HAL_CMN_H
#define SEC_HAL_CMN_H

#include <linux/types.h>

#define SEC_HAL_CMN_RES_OK          0x00000000
#define SEC_HAL_CMN_RES_FAIL        0x00000010
#define SEC_HAL_CMN_RES_PARAM_ERROR 0x00000020


typedef struct {
	uint32_t hw_reset_type;
	uint32_t reason;
	uint32_t link_register;
	uint32_t interrupt_addr;
} sec_reset_info;


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
uint32_t
sec_hal_memcpy(
		uint32_t phys_dst,
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
uint32_t
sec_hal_authenticate(
		uint32_t phys_cert_addr,
		uint32_t cert_size,
		uint32_t *obj_id);

#define AREA_ID_FRAMEBUFFER  0
#define AREA_ID_OMX          1
#define AREA_ID_HDMI         2
/* ****************************************************************************
** Function name      : sec_hal_memfw_attr_reserve
** Description        : Reserve memory firewalling attributes from secure env.
** Parameters         : IN/--- uint32_t area_id, FRAMEBUFF/HDMI/OMX identifier.
**                      IN/--- uint32_t phys_start_addr, start of prot. area.
**                      OUT/--- uint32_t phys_end_addr, end of prot. area.
** Return value       : uint32
**                      ==SEC_HAL_CMN_RES_OK operation successful
**                      failure otherwise.
** ***************************************************************************/
uint32_t
sec_hal_memfw_attr_reserve(
		uint32_t area_id,
		uint32_t phys_start_addr,
		uint32_t phys_end_addr);

/* ****************************************************************************
** Function name      : sec_hal_memfw_attr_free
** Description        : Free resources previously reserved for memory fw.
** Parameters         : IN/--- uint32_t area_id, FRAMEBUFF/HDMI/OMX identifier.
** Return value       : uint32
**                      ==SEC_HAL_CMN_RES_OK operation successful
**                      failure otherwise.
** ***************************************************************************/
uint32_t
sec_hal_memfw_attr_free(
		uint32_t area_id);

/* ****************************************************************************
** Function name      : sec_hal_reset_info_get
** Description        : Get secure reset information
** Parameters         : IN/--- sec_reset_info * reset_info, pointer where reset
                        information will be updated.
** Return value       : uint32
**                      ==SEC_HAL_CMN_RES_OK operation successful
**                      failure otherwise.
** ***************************************************************************/
uint32_t
sec_hal_reset_info_get(
		sec_reset_info *reset_info);

/* ****************************************************************************
** Function name      : sec_hal_dbg_reg_set
** Description        : Set DBGREG values to TZ.
**                      Will write registers back for assertion.
**                      Some regs/bit configuration are not allowed, thus
**                      assertion required by requestor.
** Parameters         : IN/OUT--- uint32_t *dbgreg1, pointer to in/out value.
**                      IN/OUT--- uint32_t *dbgreg2, pointer to in/out value.
**                      IN/OUT--- uint32_t *dbgreg3, pointer to in/out value.
** Return value       : uint32
**                      ==SEC_HAL_CMN_RES_OK operation successful
**                      failure otherwise.
** ***************************************************************************/
uint32_t
sec_hal_dbg_reg_set(
		uint32_t *dbgreg1,
		uint32_t *dbgreg2,
		uint32_t *dbgreg3);

#endif /* SEC_HAL_CMN_H */
