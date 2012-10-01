/*
 * drivers/sec_hal/rt/src/sec_hal_cmn.c
 *
 * Copyright (c) 2012, Renesas Mobile Corporation.
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
/*#include "sec_msg.h"*/
#include "sec_serv_api.h"
#include "sec_dispatch.h"
#include "sec_hal_res.h"
#include "sec_hal_rt_cmn.h"
#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/system.h>


#define LOCAL_MSG_BYTE_ORDER              SEC_MSG_BYTE_ORDER_LE
#define LOCAL_DEFAULT_DISP_FLAGS          0
#define LOCAL_DEFAULT_DISP_SPARE_PARAM    0

#ifdef CONFIG_ARM_TZ_SEC_INTEGRATION
#define __TRACE_ENTRY printk(KERN_INFO "%s>\n", __FUNCTION__)
#define __TRACE_EXIT  printk(KERN_INFO "%s<\n", __FUNCTION__)
/*#define __TRACE(...)  printk(KERN_WARNING #__VA_ARGS__)*/
#define __TRACE(fmt, args...) printk(KERN_DEBUG   fmt, ##args)
#else /* CONFIG_ARM_TZ_SEC_INTEGRATION */
#define __TRACE_ENTRY do{}while(0)
#define __TRACE_EXIT  do{}while(0)
/*#define __TRACE(...)  printk(KERN_WARNING #__VA_ARGS__)*/
#define __TRACE(fmt, args...) printk(KERN_DEBUG   fmt, ##args)
#endif /* CONFIG_ARM_TZ_SEC_INTEGRATION */

/* **********************************************************************
** Function name      : sec_memcpy
** Description        : request memcpy to TZ protected memory areas.
**                      Physical buffers should be cache cleaned before this
**                      operation, since RAM is directly read. Otherwise
**                      unwanted failures can occur.
** Parameters         : IN/--- uint32_t phys_dst
**                      IN/--- uint32_t phys_src
**                      IN/--- uint32_t size
** Return value       : uint32
**                      ==SEC_API_RES_OK operation successful
**                      failure otherwise.
** *********************************************************************/
uint32_t sec_hal_memcpy(uint32_t phys_dst,
                        uint32_t phys_src,
                        uint32_t size)
{
    uint32_t sec_api_status = SEC_HAL_RES_OK;
    uint32_t sec_msg_status = SEC_MSG_STATUS_OK;
    uint32_t ssa_disp_status = 0x00;
    uint32_t sec_serv_status = 0x00;
    uint16_t msg_data_size;
    sec_msg_t *in_msg = NULL;
    sec_msg_t *out_msg = NULL;
    sec_msg_handle_t in_handle;
    sec_msg_handle_t out_handle;

    __TRACE_ENTRY;

    /* allocate memory for msgs to be sent to TZ */
    msg_data_size = sec_msg_param_size(sizeof(uint32_t))*3;
    in_msg = sec_msg_alloc(&in_handle,
                msg_data_size,
                SEC_MSG_OBJECT_ID_NONE,
                0,
                LOCAL_MSG_BYTE_ORDER);
    msg_data_size = sec_msg_param_size(sizeof(sec_serv_status_t)) +
                sec_msg_param_size(sizeof(uint32_t));
    out_msg = sec_msg_alloc(&out_handle,
                msg_data_size,
                SEC_MSG_OBJECT_ID_NONE,
                0,
                LOCAL_MSG_BYTE_ORDER);

    do{
        if (!in_msg || !out_msg){
            __TRACE("alloc failure, msgs not sent, aborting!");
            sec_api_status = SEC_HAL_RES_FAIL;
            break;
        }

        /* write content to the input msg */
        if (sec_msg_param_write32(&in_handle,
                          phys_dst,
                          SEC_MSG_PARAM_ID_NONE) != SEC_MSG_STATUS_OK){
            __TRACE("phys_dst write error, aborting!");
            sec_api_status = SEC_HAL_RES_FAIL;
            break;
        }
        if (sec_msg_param_write32(&in_handle,
                          phys_src,
                          SEC_MSG_PARAM_ID_NONE) != SEC_MSG_STATUS_OK){
            __TRACE("phys_src write error, aborting!");
            sec_api_status = SEC_HAL_RES_FAIL;
            break;
        }
        if (sec_msg_param_write32(&in_handle,
                          size,
                          SEC_MSG_PARAM_ID_NONE) != SEC_MSG_STATUS_OK){
            __TRACE("size write error, aborting!");
            sec_api_status = SEC_HAL_RES_FAIL;
            break;
        }
        /* write memory barrier, to ensure that all write ops completed */
        wmb();

        /* call dispatcher */
        ssa_disp_status = pub2sec_dispatcher(SEC_SERV_MEMCPY_REQUEST,
                                LOCAL_DEFAULT_DISP_FLAGS,
                                LOCAL_DEFAULT_DISP_SPARE_PARAM,
            			SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
                               	SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));

        /* interpret the response */
        sec_msg_status = sec_msg_param_read32(&out_handle, &sec_serv_status);
        /* read memory barrier, to ensure that all read ops completed */
        rmb();
        if (SEC_ROM_RET_OK != ssa_disp_status ||
        SEC_MSG_STATUS_OK != sec_msg_status ||
        SEC_SERV_STATUS_OK != sec_serv_status){
            __TRACE("failure! disp==%d, msg==%d, serv==%d\n",
                     ssa_disp_status, sec_msg_status, sec_serv_status);
            sec_api_status = SEC_HAL_RES_FAIL;
            break;
        }
    }while (0);

    /* de-allocate msgs */
    sec_msg_free(out_msg);
    sec_msg_free(in_msg);

    __TRACE_EXIT;
    return sec_api_status;
}
EXPORT_SYMBOL(sec_hal_memcpy); /* made available for other kernel entities */

/* **********************************************************************
** Function name      : sec_authenticate
** Description        : Authenticate memory area with given SWCERT.
**                      Physical buffers should be cache cleaned before this
**                      operation, since RAM is directly read. Otherwise
**                      unwanted failures can occur.
** Parameters         : IN/--- uint32_t phys_cert_addr
**                      IN/--- uint32_t cert_size
**                      OUT/--- uint32_t *obj_id, object id if successful
** Return value       : uint32
**                      ==SEC_API_RES_OK operation successful
**                      failure otherwise.
** *********************************************************************/
uint32_t sec_hal_authenticate(uint32_t phys_cert_addr,
                              uint32_t cert_size,
                              uint32_t *obj_id)
{
    uint32_t sec_api_status = SEC_HAL_RES_OK;
    uint32_t sec_msg_status = SEC_MSG_STATUS_OK;
    uint32_t ssa_disp_status = 0x00;
    uint32_t sec_serv_status = 0x00;
    uint16_t msg_data_size;
    sec_msg_t *in_msg = NULL;
    sec_msg_t *out_msg = NULL;
    sec_msg_handle_t in_handle;
    sec_msg_handle_t out_handle;

    __TRACE_ENTRY;

    /* allocate memory for msgs to be sent to TZ */
    msg_data_size = sec_msg_param_size(sizeof(uint32_t));
    in_msg = sec_msg_alloc(&in_handle,
                msg_data_size,
                SEC_MSG_OBJECT_ID_NONE,
                0,
                LOCAL_MSG_BYTE_ORDER);
    msg_data_size = sec_msg_param_size(sizeof(sec_serv_status_t)) +
                sec_msg_param_size(sizeof(uint32_t));
    out_msg = sec_msg_alloc(
                &out_handle,
                msg_data_size,
                SEC_MSG_OBJECT_ID_NONE,
                0,
                LOCAL_MSG_BYTE_ORDER);

    do{
        if (!in_msg || !out_msg){
            __TRACE("alloc failure, msgs not sent, aborting!");
            sec_api_status = SEC_HAL_RES_FAIL;
            break;
        }

        /* write content to the input msg */
        if (sec_msg_param_write32(&in_handle,
                          phys_cert_addr,
                          SEC_MSG_PARAM_ID_NONE) != SEC_MSG_STATUS_OK){
            __TRACE("phys_cert_addr write error, aborting!");
            sec_api_status = SEC_HAL_RES_FAIL;
            break;
        }
        /* write memory barrier, to ensure that all write ops completed */
        wmb();

        /* call dispatcher */
        ssa_disp_status = pub2sec_dispatcher(SEC_SERV_CERTIFICATE_REGISTER,
                                LOCAL_DEFAULT_DISP_FLAGS,
                                LOCAL_DEFAULT_DISP_SPARE_PARAM,
				SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
                               	SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));

        /* interpret the response */
        sec_msg_status = sec_msg_param_read32(&out_handle, &sec_serv_status);
        /* read memory barrier, to ensure that all read ops completed */
        rmb();
        if (SEC_ROM_RET_OK != ssa_disp_status ||
        SEC_MSG_STATUS_OK != sec_msg_status ||
        SEC_SERV_STATUS_OK != sec_serv_status){
            __TRACE("failure! disp==%d, msg==%d, serv==%d\n",
            ssa_disp_status, sec_msg_status, sec_serv_status);
            sec_api_status = SEC_HAL_RES_FAIL;
            break;
        }

        if (obj_id){
            sec_msg_status = sec_msg_param_read32(&out_handle, obj_id);
            /* read memory barrier, to ensure that all read ops completed */
            rmb();

            if (sec_msg_status != sec_msg_status){
                __TRACE("failed to read objid! msg==%d\n", sec_msg_status);
            }
        }
    }while (0);

    /* de-allocate msgs */
    sec_msg_free(out_msg);
    sec_msg_free(in_msg);

    __TRACE_EXIT;
    return sec_api_status;
}
/* made available for other kernel entities */
EXPORT_SYMBOL(sec_hal_authenticate);

