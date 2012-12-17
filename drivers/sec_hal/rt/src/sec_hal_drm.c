/* *********************************************************************** **
**                               Renesas                                   **
** *********************************************************************** */

/* ************************** COPYRIGHT INFORMATION ********************** **
** This program contains proprietary information that is a trade secret of **
** Renesas and also is protected as an unpublished work under              **
** applicable Copyright laws. Recipient is to retain this program in       **
** confidence and is not permitted to use or make copies thereof other than**
** as permitted in a written agreement with Renesas.                       **
**                                                                         **
** Copyright (C) 2012 Renesas Electronics Corp.                            **
** All rights reserved.                                                    **
** *********************************************************************** */
#include "sec_hal_drm.h"
#include "sec_hal_res.h"
#include "sec_hal_rt_cmn.h"
#define SEC_HAL_TRACE_LOCAL_DISABLE
#include "sec_hal_rt_trace.h"
#include "sec_serv_api.h"
#include "sec_msg.h"
#include "sec_dispatch.h"
#include <linux/string.h>
#include <linux/uaccess.h>

#define LOCAL_DISP                     pub2sec_dispatcher
#define LOCAL_DEFAULT_DISP_FLAGS       0
#define LOCAL_DEFAULT_DISP_SPARE_PARAM 0
#define LOCAL_WMB()                    wmb()
#define LOCAL_RMB()                    rmb()
#define LOCAL_IV_SIZE                  16


/* **************************************************************************
** Function name      : sec_hal_drm_enter
** Description        : 
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** *************************************************************************/
uint32_t
sec_hal_drm_enter(uint32_t thr_id,
                  uint32_t *session_id)
{
    uint32_t sec_hal_status = SEC_HAL_RES_OK;
    uint32_t sec_msg_status = SEC_MSG_STATUS_OK;
    uint32_t ssa_disp_status = 0x00;
    uint32_t sec_serv_status = 0x00;
    uint16_t msg_data_size;
    sec_msg_t *in_msg = NULL;
    sec_msg_t *out_msg = NULL;
    sec_msg_handle_t in_handle;
    sec_msg_handle_t out_handle;

    SEC_HAL_TRACE_ENTRY();

    /* allocate memory, from ICRAM, for msgs to be sent to TZ */
    msg_data_size = sec_msg_param_size(sizeof(uint32_t));
    in_msg = sec_msg_alloc(&in_handle,
                           msg_data_size,
                           SEC_MSG_OBJECT_ID_NONE,
                           0,
                           SEC_HAL_MSG_BYTE_ORDER);
    msg_data_size = sec_msg_param_size(sizeof(sec_serv_status_t)) +
                        sec_msg_param_size(sizeof(uint32_t));
    out_msg = sec_msg_alloc(&out_handle,
                            msg_data_size,
                            SEC_MSG_OBJECT_ID_NONE,
                            0,
                            SEC_HAL_MSG_BYTE_ORDER);

    if (!in_msg || !out_msg) {
        SEC_HAL_TRACE("alloc failure, msgs not sent!");
        sec_hal_status = SEC_HAL_RES_FAIL;
    }
    else {
        /* write content to the input msg */
        sec_msg_param_write32(
                &in_handle,
                thr_id,
                SEC_MSG_PARAM_ID_NONE);
        LOCAL_WMB();

        /* call dispatcher */
        ssa_disp_status = LOCAL_DISP(
                            SEC_SERV_DRM_OEMCRYPTO_SESSION_INIT,
                            LOCAL_DEFAULT_DISP_FLAGS,
                            LOCAL_DEFAULT_DISP_SPARE_PARAM,
                            SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
                            SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));

        /* interpret the response */
        sec_msg_status = sec_msg_param_read32(&out_handle, &sec_serv_status);
        LOCAL_RMB();
        if (SEC_ROM_RET_OK != ssa_disp_status ||
            SEC_MSG_STATUS_OK != sec_msg_status ||
            SEC_SERV_STATUS_OK != sec_serv_status) {
            SEC_HAL_TRACE("SESSION_INIT failed!");
            SEC_HAL_TRACE_INT("ssa_disp_status", ssa_disp_status);
            SEC_HAL_TRACE_INT("sec_msg_status ", sec_msg_status);
            SEC_HAL_TRACE_INT("sec_serv_status", sec_serv_status);
            sec_hal_status = SEC_HAL_RES_FAIL;
        }
        else {
            sec_msg_status = sec_msg_param_read32(&out_handle, session_id);
            LOCAL_RMB();
            if (SEC_MSG_STATUS_OK != sec_msg_status) {
                SEC_HAL_TRACE_INT("sec_msg_status ", sec_msg_status);
                sec_hal_status = SEC_HAL_RES_FAIL;
            }
        }
    }

    /* de-allocate msgs */
    sec_msg_free(out_msg);
    sec_msg_free(in_msg);

    SEC_HAL_TRACE_EXIT();
    return sec_hal_status;
}

/* **************************************************************************
** Function name      : sec_hal_drm_exit
** Description        : 
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** *************************************************************************/
uint32_t
sec_hal_drm_exit(uint32_t id_cnt,
                 uint32_t session_ids[])
{
    uint32_t sec_hal_status = SEC_HAL_RES_OK;
    uint32_t sec_msg_status = SEC_MSG_STATUS_OK;
    uint32_t ssa_disp_status = 0x00;
    uint32_t sec_serv_status = 0x00;
    uint16_t msg_data_size;
    sec_msg_t *in_msg = NULL;
    sec_msg_t *out_msg = NULL;
    sec_msg_handle_t in_handle;
    sec_msg_handle_t out_handle;

    SEC_HAL_TRACE_ENTRY();

    /* allocate memory, from ICRAM, for msgs to be sent to TZ */
    msg_data_size = sec_msg_param_size(sizeof(uint32_t))*(1+id_cnt);
    in_msg = sec_msg_alloc(&in_handle,
                           msg_data_size,
                           SEC_MSG_OBJECT_ID_NONE,
                           0,
                           SEC_HAL_MSG_BYTE_ORDER);
    msg_data_size = sec_msg_param_size(sizeof(sec_serv_status_t));
    out_msg = sec_msg_alloc(&out_handle,
                            msg_data_size,
                            SEC_MSG_OBJECT_ID_NONE,
                            0,
                            SEC_HAL_MSG_BYTE_ORDER);

    if (!in_msg || !out_msg) {
        SEC_HAL_TRACE("alloc failure, msgs not sent!");
        sec_hal_status = SEC_HAL_RES_FAIL;
    }
    else {
        sec_msg_status = sec_msg_param_write32(
                    &in_handle,
                    id_cnt,
                    SEC_MSG_PARAM_ID_NONE);
        /* write content to the input msg */
        while (SEC_MSG_STATUS_OK == sec_msg_status && id_cnt) {
            sec_msg_status = sec_msg_param_write32(
                        &in_handle,
                        session_ids[--id_cnt],
                        SEC_MSG_PARAM_ID_NONE);
        }
        LOCAL_WMB();

        /* call dispatcher */
        ssa_disp_status = LOCAL_DISP(
                            SEC_SERV_DRM_OEMCRYPTO_SESSION_TERMINATE,
                            LOCAL_DEFAULT_DISP_FLAGS,
                            LOCAL_DEFAULT_DISP_SPARE_PARAM,
                            SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
                            SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));

        /* interpret the response */
        sec_msg_status = sec_msg_param_read32(&out_handle, &sec_serv_status);
        LOCAL_RMB();
        if (SEC_ROM_RET_OK != ssa_disp_status ||
            SEC_MSG_STATUS_OK != sec_msg_status ||
            SEC_SERV_STATUS_OK != sec_serv_status) {
            SEC_HAL_TRACE("SESSION_TERMINATE failed!");
            SEC_HAL_TRACE_INT("ssa_disp_status", ssa_disp_status);
            SEC_HAL_TRACE_INT("sec_msg_status ", sec_msg_status);
            SEC_HAL_TRACE_INT("sec_serv_status", sec_serv_status);
            sec_hal_status = SEC_HAL_RES_FAIL;
        }
    }

    /* de-allocate msgs */
    sec_msg_free(out_msg);
    sec_msg_free(in_msg);

    SEC_HAL_TRACE_EXIT();
    return sec_hal_status;
}

/* **************************************************************************
** Function name      : sec_hal_drm_set_entit_key
** Description        :
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** *************************************************************************/
uint32_t
sec_hal_drm_set_entit_key(uint32_t session_id,
                          uint32_t key_len,
                          uint8_t *key)
{
    uint32_t sec_hal_status = SEC_HAL_RES_OK;
    uint32_t sec_msg_status[2] = {SEC_MSG_STATUS_OK};
    uint32_t ssa_disp_status = 0x00;
    uint32_t sec_serv_status = 0x00;
    uint32_t sec_drm_status = 0x00;
    uint16_t msg_data_size;
    sec_msg_t *in_msg = NULL;
    sec_msg_t *out_msg = NULL;
    sec_msg_handle_t in_handle;
    sec_msg_handle_t out_handle;

    SEC_HAL_TRACE_ENTRY();

    /* allocate memory, from ICRAM, for msgs to be sent to TZ */
    msg_data_size = sec_msg_param_size(sizeof(uint32_t))*2 + 
                        sec_msg_param_size(key_len);
    in_msg = sec_msg_alloc(&in_handle,
                           msg_data_size,
                           SEC_MSG_OBJECT_ID_NONE,
                           0,
                           SEC_HAL_MSG_BYTE_ORDER);
    msg_data_size = sec_msg_param_size(sizeof(sec_serv_status_t)) +
                        sec_msg_param_size(sizeof(uint32_t));
    out_msg = sec_msg_alloc(&out_handle,
                            msg_data_size,
                            SEC_MSG_OBJECT_ID_NONE,
                            0,
                            SEC_HAL_MSG_BYTE_ORDER);

    if (!in_msg || !out_msg) {
        SEC_HAL_TRACE("alloc failure, msgs not sent!");
        sec_hal_status = SEC_HAL_RES_FAIL;
    }
    else {
        /* write content to the input msg */
        sec_msg_param_write32(&in_handle,
                              session_id,
                              SEC_MSG_PARAM_ID_NONE);
        sec_msg_param_write32(&in_handle,
                              key_len,
                              SEC_MSG_PARAM_ID_NONE);
        sec_msg_param_write(&in_handle,
                            key,
                            key_len,
                            SEC_MSG_PARAM_ID_NONE);
        LOCAL_WMB();

        /* call dispatcher */
        ssa_disp_status = LOCAL_DISP(
                            SEC_SERV_DRM_OEMCRYPTO_SET_ENTITLEMENT_KEY,
                            LOCAL_DEFAULT_DISP_FLAGS,
                            LOCAL_DEFAULT_DISP_SPARE_PARAM,
                            SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
                            SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));

        /* interpret the response */
        sec_msg_status[0] = sec_msg_param_read32(&out_handle, &sec_serv_status);
        sec_msg_status[1] = sec_msg_param_read32(&out_handle, &sec_drm_status);
        LOCAL_RMB();
        if (SEC_ROM_RET_OK != ssa_disp_status ||
            SEC_MSG_STATUS_OK != sec_msg_status[0] ||
            SEC_MSG_STATUS_OK != sec_msg_status[1] ||
            SEC_SERV_STATUS_OK != sec_serv_status) {
            SEC_HAL_TRACE("SET_ENTIT_KEY failed!");
            SEC_HAL_TRACE_INT("ssa_disp_status", ssa_disp_status);
            SEC_HAL_TRACE_INT("sec_msg_status0", sec_msg_status[0]);
            SEC_HAL_TRACE_INT("sec_msg_status1", sec_msg_status[1]);
            SEC_HAL_TRACE_INT("sec_serv_status", sec_serv_status);
            sec_hal_status = SEC_HAL_RES_FAIL;
        }
    }

    /* de-allocate msgs */
    sec_msg_free(out_msg);
    sec_msg_free(in_msg);

    SEC_HAL_TRACE_EXIT();
    return (SEC_HAL_RES_OK == sec_hal_status ?
            sec_drm_status : sec_hal_status);
}


/* **************************************************************************
** Function name      : sec_hal_drm_derive_cw
** Description        :
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** *************************************************************************/
uint32_t
sec_hal_drm_derive_cw(uint32_t session_id,
                      uint32_t ecm_len,
                      uint8_t *ecm,
                      uint32_t *flags)
{
    uint32_t sec_hal_status = SEC_HAL_RES_OK;
    uint32_t sec_msg_status[2] = {SEC_MSG_STATUS_OK};
    uint32_t ssa_disp_status = 0x00;
    uint32_t sec_serv_status = 0x00;
    uint32_t sec_drm_status = 0x00;
    uint16_t msg_data_size;
    sec_msg_t *in_msg = NULL;
    sec_msg_t *out_msg = NULL;
    sec_msg_handle_t in_handle;
    sec_msg_handle_t out_handle;

    SEC_HAL_TRACE_ENTRY();

    /* allocate memory, from ICRAM, for msgs to be sent to TZ */
    msg_data_size = sec_msg_param_size(sizeof(uint32_t))*2 + 
                        sec_msg_param_size(ecm_len);
    in_msg = sec_msg_alloc(&in_handle,
                           msg_data_size,
                           SEC_MSG_OBJECT_ID_NONE,
                           0,
                           SEC_HAL_MSG_BYTE_ORDER);
    msg_data_size = sec_msg_param_size(sizeof(sec_serv_status_t)) +
                        sec_msg_param_size(sizeof(uint32_t))*2;
    out_msg = sec_msg_alloc(&out_handle,
                            msg_data_size,
                            SEC_MSG_OBJECT_ID_NONE,
                            0,
                            SEC_HAL_MSG_BYTE_ORDER);

    if (!in_msg || !out_msg) {
        SEC_HAL_TRACE("alloc failure, msgs not sent!");
        sec_hal_status = SEC_HAL_RES_FAIL;
    }
    else {
        /* write content to the input msg */
        sec_msg_param_write32(&in_handle,
                              session_id,
                              SEC_MSG_PARAM_ID_NONE);
        sec_msg_param_write32(&in_handle,
                              ecm_len,
                              SEC_MSG_PARAM_ID_NONE);
        sec_msg_param_write(&in_handle,
                            ecm,
                            ecm_len,
                            SEC_MSG_PARAM_ID_NONE);
        LOCAL_WMB();

        /* call dispatcher */
        ssa_disp_status = LOCAL_DISP(
                            SEC_SERV_DRM_OEMCRYPTO_DERIVE_CONTROL_WORD,
                            LOCAL_DEFAULT_DISP_FLAGS,
                            LOCAL_DEFAULT_DISP_SPARE_PARAM,
                            SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
                            SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));

        /* interpret the response */
        sec_msg_status[0] = sec_msg_param_read32(&out_handle, &sec_serv_status);
        sec_msg_status[1] = sec_msg_param_read32(&out_handle, &sec_drm_status);
        LOCAL_RMB();
        if (SEC_ROM_RET_OK != ssa_disp_status ||
            SEC_MSG_STATUS_OK != sec_msg_status[0] ||
            SEC_MSG_STATUS_OK != sec_msg_status[1] ||
            SEC_SERV_STATUS_OK != sec_serv_status) {
            SEC_HAL_TRACE("DERIVE_CW failed!");
            SEC_HAL_TRACE_INT("ssa_disp_status", ssa_disp_status);
            SEC_HAL_TRACE_INT("sec_msg_status0", sec_msg_status[0]);
            SEC_HAL_TRACE_INT("sec_msg_status1", sec_msg_status[1]);
            SEC_HAL_TRACE_INT("sec_serv_status", sec_serv_status);
            sec_hal_status = SEC_HAL_RES_FAIL;
        }
        else {
             sec_msg_status[0] = sec_msg_param_read32(&out_handle, flags);
             if (SEC_MSG_STATUS_OK != sec_msg_status[0]) {
                sec_hal_status = SEC_HAL_RES_FAIL;
             }
        }
    }

    /* de-allocate msgs */
    sec_msg_free(out_msg);
    sec_msg_free(in_msg);

    SEC_HAL_TRACE_EXIT();
    return (SEC_HAL_RES_OK == sec_hal_status ?
            sec_drm_status : sec_hal_status);
}


/* **************************************************************************
** Function name      : sec_hal_drm_decrypt_video
** Description        :
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** *************************************************************************/
uint32_t
sec_hal_drm_decrypt_video(uint32_t session_id,
                          uint8_t *iv_in,
                          uint32_t iv_len,
                          uint32_t input_len,
                          uint32_t input_phys_addr,
                          uint32_t output_phys_addr,
                          uint32_t output_offset,
                          uint32_t *output_len,
                          uint8_t *iv_out)
{
    uint32_t sec_hal_status = SEC_HAL_RES_OK;
    uint32_t sec_msg_status[2] = {SEC_MSG_STATUS_OK};
    uint32_t ssa_disp_status = 0x00;
    uint32_t sec_serv_status = 0x00;
    uint32_t sec_drm_status = 0x00;
    uint16_t msg_data_size;
    sec_msg_t *in_msg = NULL;
    sec_msg_t *out_msg = NULL;
    sec_msg_handle_t in_handle;
    sec_msg_handle_t out_handle;

    SEC_HAL_TRACE_ENTRY();

    /* allocate memory, from ICRAM, for msgs to be sent to TZ */
    msg_data_size = sec_msg_param_size(sizeof(uint32_t))*6 +
                        sec_msg_param_size(LOCAL_IV_SIZE);
    in_msg = sec_msg_alloc(&in_handle,
                           msg_data_size,
                           SEC_MSG_OBJECT_ID_NONE,
                           0,
                           SEC_HAL_MSG_BYTE_ORDER);
    msg_data_size = sec_msg_param_size(sizeof(sec_serv_status_t)) +
                        sec_msg_param_size(sizeof(uint32_t))*2 +
                        sec_msg_param_size(LOCAL_IV_SIZE);
    out_msg = sec_msg_alloc(&out_handle,
                            msg_data_size,
                            SEC_MSG_OBJECT_ID_NONE,
                            0,
                            SEC_HAL_MSG_BYTE_ORDER);

    if (!in_msg || !out_msg) {
        SEC_HAL_TRACE("alloc failure, msgs not sent!");
        sec_hal_status = SEC_HAL_RES_FAIL;
    }
    else {
        /* write content to the input msg */
        sec_msg_param_write32(&in_handle,
                              session_id,
                              SEC_MSG_PARAM_ID_NONE);
        sec_msg_param_write32(&in_handle,
                              iv_len,
                              SEC_MSG_PARAM_ID_NONE);
        sec_msg_param_write(&in_handle,
                            iv_in,
                            LOCAL_IV_SIZE,
                            SEC_MSG_PARAM_ID_NONE);
        sec_msg_param_write32(&in_handle,
                              input_len,
                              SEC_MSG_PARAM_ID_NONE);
        sec_msg_param_write32(&in_handle,
                              input_phys_addr,
                              SEC_MSG_PARAM_ID_NONE);
        sec_msg_param_write32(&in_handle,
                              output_phys_addr,
                              SEC_MSG_PARAM_ID_NONE);
        sec_msg_param_write32(&in_handle,
                              output_offset,
                              SEC_MSG_PARAM_ID_NONE);
        LOCAL_WMB();

        /* call dispatcher */
        ssa_disp_status = LOCAL_DISP(
                            SEC_SERV_DRM_OEMCRYPTO_DECRYPT_VIDEO,
                            LOCAL_DEFAULT_DISP_FLAGS,
                            LOCAL_DEFAULT_DISP_SPARE_PARAM,
                            SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
                            SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));

        /* interpret the response */
        sec_msg_status[0] = sec_msg_param_read32(&out_handle, &sec_serv_status);
        sec_msg_status[1] = sec_msg_param_read32(&out_handle, &sec_drm_status);
        LOCAL_RMB();
        if (SEC_ROM_RET_OK != ssa_disp_status ||
            SEC_MSG_STATUS_OK != sec_msg_status[0] ||
            SEC_MSG_STATUS_OK != sec_msg_status[1] ||
            SEC_SERV_STATUS_OK != sec_serv_status) {
            SEC_HAL_TRACE("DECRYPT_VIDEO failed!");
            SEC_HAL_TRACE_INT("ssa_disp_status", ssa_disp_status);
            SEC_HAL_TRACE_INT("sec_msg_status0", sec_msg_status[0]);
            SEC_HAL_TRACE_INT("sec_msg_status1", sec_msg_status[1]);
            SEC_HAL_TRACE_INT("sec_serv_status", sec_serv_status);
            sec_hal_status = SEC_HAL_RES_FAIL;
        }
        else {
            sec_msg_status[0] = sec_msg_param_read32(&out_handle, output_len);
            sec_msg_status[1] = sec_msg_param_read(&out_handle, iv_out, LOCAL_IV_SIZE);
            LOCAL_RMB();
            if (SEC_MSG_STATUS_OK != sec_msg_status[0] ||
                SEC_MSG_STATUS_OK != sec_msg_status[1]) {
                SEC_HAL_TRACE("read failed!");
                SEC_HAL_TRACE_INT("sec_msg_status0", sec_msg_status[0]);
                SEC_HAL_TRACE_INT("sec_msg_status1", sec_msg_status[1]);
                sec_hal_status = SEC_HAL_RES_FAIL;
            }
        }
    }

    /* de-allocate msgs */
    sec_msg_free(out_msg);
    sec_msg_free(in_msg);

    SEC_HAL_TRACE_EXIT();
    return (sec_hal_status == SEC_HAL_RES_OK ?
            sec_drm_status : sec_hal_status);
}



/* **************************************************************************
** Function name      : sec_hal_drm_decrypt_audio
** Description        :
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** *************************************************************************/
uint32_t
sec_hal_drm_decrypt_audio(uint32_t session_id,
                          uint8_t *iv_in,
                          uint32_t iv_len,
                          uint32_t input_len,
                          uint32_t input_phys_addr,
                          uint32_t output_phys_addr,
                          uint32_t *output_len,
                          uint8_t *iv_out)
{
    uint32_t sec_hal_status = SEC_HAL_RES_OK;
    uint32_t sec_msg_status[2] = {SEC_MSG_STATUS_OK};
    uint32_t ssa_disp_status = 0x00;
    uint32_t sec_serv_status = 0x00;
    uint32_t sec_drm_status = 0x00;
    uint16_t msg_data_size;
    sec_msg_t *in_msg = NULL;
    sec_msg_t *out_msg = NULL;
    sec_msg_handle_t in_handle;
    sec_msg_handle_t out_handle;

    SEC_HAL_TRACE_ENTRY();

    /* allocate memory, from ICRAM, for msgs to be sent to TZ */
    msg_data_size = sec_msg_param_size(sizeof(uint32_t))*5 +
                        sec_msg_param_size(LOCAL_IV_SIZE);
    in_msg = sec_msg_alloc(&in_handle,
                           msg_data_size,
                           SEC_MSG_OBJECT_ID_NONE,
                           0,
                           SEC_HAL_MSG_BYTE_ORDER);
    msg_data_size = sec_msg_param_size(sizeof(sec_serv_status_t)) +
                        sec_msg_param_size(sizeof(uint32_t))*2 +
                        sec_msg_param_size(LOCAL_IV_SIZE);
    out_msg = sec_msg_alloc(&out_handle,
                            msg_data_size,
                            SEC_MSG_OBJECT_ID_NONE,
                            0,
                            SEC_HAL_MSG_BYTE_ORDER);

    if (!in_msg || !out_msg) {
        SEC_HAL_TRACE("alloc failure, msgs not sent!");
        sec_hal_status = SEC_HAL_RES_FAIL;
    }
    else {
        /* write content to the input msg */
        sec_msg_param_write32(&in_handle,
                              session_id,
                              SEC_MSG_PARAM_ID_NONE);
        sec_msg_param_write32(&in_handle,
                              iv_len,
                              SEC_MSG_PARAM_ID_NONE);
        sec_msg_param_write(&in_handle,
                            iv_in,
                            LOCAL_IV_SIZE,
                            SEC_MSG_PARAM_ID_NONE);
        sec_msg_param_write32(&in_handle,
                              input_len,
                              SEC_MSG_PARAM_ID_NONE);
        sec_msg_param_write32(&in_handle,
                              input_phys_addr,
                              SEC_MSG_PARAM_ID_NONE);
        sec_msg_param_write32(&in_handle,
                              output_phys_addr,
                              SEC_MSG_PARAM_ID_NONE);
        LOCAL_WMB();

        /* call dispatcher */
        ssa_disp_status = LOCAL_DISP(
                            SEC_SERV_DRM_OEMCRYPTO_DECRYPT_AUDIO,
                            LOCAL_DEFAULT_DISP_FLAGS,
                            LOCAL_DEFAULT_DISP_SPARE_PARAM,
                            SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
                            SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));

        /* interpret the response */
        sec_msg_status[0] = sec_msg_param_read32(&out_handle, &sec_serv_status);
        sec_msg_status[1] = sec_msg_param_read32(&out_handle, &sec_drm_status);
        LOCAL_RMB();
        if (SEC_ROM_RET_OK != ssa_disp_status ||
            SEC_MSG_STATUS_OK != sec_msg_status[0] ||
            SEC_MSG_STATUS_OK != sec_msg_status[1] ||
            SEC_SERV_STATUS_OK != sec_serv_status) {
            SEC_HAL_TRACE("DECRYPT_AUDIO failed!");
            SEC_HAL_TRACE_INT("ssa_disp_status", ssa_disp_status);
            SEC_HAL_TRACE_INT("sec_msg_status0", sec_msg_status[0]);
            SEC_HAL_TRACE_INT("sec_msg_status1", sec_msg_status[1]);
            SEC_HAL_TRACE_INT("sec_serv_status", sec_serv_status);
            sec_hal_status = SEC_HAL_RES_FAIL;
        }
        else {
            sec_msg_status[0] = sec_msg_param_read32(&out_handle, output_len);
            sec_msg_status[1] = sec_msg_param_read(&out_handle, iv_out, LOCAL_IV_SIZE);
            LOCAL_RMB();
            if (SEC_MSG_STATUS_OK != sec_msg_status[0] ||
                SEC_MSG_STATUS_OK != sec_msg_status[1]) {
                SEC_HAL_TRACE("read failed!");
                SEC_HAL_TRACE_INT("sec_msg_status0", sec_msg_status[0]);
                SEC_HAL_TRACE_INT("sec_msg_status1", sec_msg_status[1]);
                sec_hal_status = SEC_HAL_RES_FAIL;
            }
        }
    }

    /* de-allocate msgs */
    sec_msg_free(out_msg);
    sec_msg_free(in_msg);

    SEC_HAL_TRACE_EXIT();
    return (SEC_HAL_RES_OK == sec_hal_status ?
            sec_drm_status : sec_hal_status);
}

/* **************************************************************************
** Function name      : sec_hal_drm_wrap_keybox
** Description        :
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** *************************************************************************/
uint32_t
sec_hal_drm_wrap_keybox(uint8_t *keybox,
                        uint32_t keybox_size,
                        uint8_t *wrapped_keybox,
                        uint32_t *wrapped_keybox_size,
                        uint8_t *transport_key,
                        uint32_t transport_key_size)
{
    uint32_t sec_hal_st = SEC_HAL_RES_OK;
    uint32_t sec_msg_st[2] = {SEC_MSG_STATUS_OK};
    uint32_t ssa_disp_st = 0x00;
    uint32_t sec_serv_st = 0x00;
    uint32_t sec_drm_st = 0x00;
    uint16_t msg_data_size, buffer_size;
    sec_msg_t *in_msg = NULL;
    sec_msg_t *out_msg = NULL;
    sec_msg_handle_t in_handle;
    sec_msg_handle_t out_handle;

    SEC_HAL_TRACE_ENTRY();

    buffer_size = wrapped_keybox ? sec_msg_param_size(keybox_size*2) : 0;
    /* allocate memory, from ICRAM, for msgs to be sent to TZ */
    msg_data_size = sec_msg_param_size(sizeof(uint32_t))*2 +
                        sec_msg_param_size(keybox_size) +
                        sec_msg_param_size(transport_key_size);
    in_msg = sec_msg_alloc(&in_handle,
                           msg_data_size,
                           SEC_MSG_OBJECT_ID_NONE,
                           0,
                           SEC_HAL_MSG_BYTE_ORDER);
    msg_data_size = sec_msg_param_size(sizeof(sec_serv_status_t)) +
                        sec_msg_param_size(sizeof(uint32_t))*2 +
                        buffer_size;
    out_msg = sec_msg_alloc(&out_handle,
                            msg_data_size,
                            SEC_MSG_OBJECT_ID_NONE,
                            0,
                            SEC_HAL_MSG_BYTE_ORDER);

    if (!in_msg || !out_msg) {
        SEC_HAL_TRACE("alloc failure, msgs not sent!");
        sec_hal_st = SEC_HAL_RES_FAIL;
    }
    else {
        /* write content to the input msg */
        sec_msg_param_write32(&in_handle,
                              keybox_size,
                              SEC_MSG_PARAM_ID_NONE);
        sec_msg_param_write(&in_handle,
                            keybox,
                            keybox_size,
                            SEC_MSG_PARAM_ID_NONE);
        sec_msg_param_write32(&in_handle,
                              buffer_size,
                              SEC_MSG_PARAM_ID_NONE);

        /* optional transport key */
        if (transport_key_size) {
            sec_msg_param_write32(&in_handle,
                                  transport_key_size,
                                  SEC_MSG_PARAM_ID_NONE);
            sec_msg_param_write(&in_handle,
                                keybox,
                                keybox_size,
                                SEC_MSG_PARAM_ID_NONE);
        }
        LOCAL_WMB();

        /* call dispatcher */
        ssa_disp_st = LOCAL_DISP(SEC_SERV_DRM_OEMCRYPTO_WRAP_KEYBOX,
                            LOCAL_DEFAULT_DISP_FLAGS,
                            LOCAL_DEFAULT_DISP_SPARE_PARAM,
                            SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
                            SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));

        /* interpret the response */
        sec_msg_st[0] = sec_msg_param_read32(&out_handle, &sec_serv_st);
        sec_msg_st[1] = sec_msg_param_read32(&out_handle, &sec_drm_st);
        LOCAL_RMB();
        if (SEC_ROM_RET_OK != ssa_disp_st ||
            SEC_MSG_STATUS_OK != sec_msg_st[0] ||
            SEC_MSG_STATUS_OK != sec_msg_st[1] ||
            SEC_SERV_STATUS_OK != sec_serv_st) {
            SEC_HAL_TRACE("WRAP_KEYBOX failed!");
            SEC_HAL_TRACE_INT("ssa_disp_st", ssa_disp_st);
            SEC_HAL_TRACE_INT("sec_msg_st0", sec_msg_st[0]);
            SEC_HAL_TRACE_INT("sec_msg_st1", sec_msg_st[1]);
            SEC_HAL_TRACE_INT("sec_serv_st", sec_serv_st);
            sec_hal_st = SEC_HAL_RES_FAIL;
        }
        else {
            sec_msg_st[0] = sec_msg_param_read32(&out_handle,
                                            wrapped_keybox_size);
            if (wrapped_keybox && *wrapped_keybox_size) {
                sec_msg_st[1] = sec_msg_param_read(&out_handle,
                                                wrapped_keybox,
                                                (uint16_t)*wrapped_keybox_size);
            }
            LOCAL_RMB();
            if (SEC_MSG_STATUS_OK != sec_msg_st[0] ||
                SEC_MSG_STATUS_OK != sec_msg_st[1]) {
                SEC_HAL_TRACE("read failed!");
                sec_hal_st = SEC_HAL_RES_FAIL;
            }

        }
   }

    /* de-allocate msgs */
    sec_msg_free(out_msg);
    sec_msg_free(in_msg);

    SEC_HAL_TRACE_EXIT();
    return (sec_hal_st == SEC_HAL_RES_OK ? sec_drm_st : sec_hal_st);
}

/* **************************************************************************
** Function name      : sec_hal_drm_install_keybox
** Description        :
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** *************************************************************************/
uint32_t
sec_hal_drm_install_keybox(uint8_t *keybox,
                           uint32_t keybox_size)
{
    uint32_t sec_hal_st = SEC_HAL_RES_OK;
    uint32_t sec_msg_st[2] = {SEC_MSG_STATUS_OK};
    uint32_t ssa_disp_st = 0x00;
    uint32_t sec_serv_st = 0x00;
    uint32_t sec_drm_st = 0x00;
    uint16_t msg_data_size;
    sec_msg_t *in_msg = NULL;
    sec_msg_t *out_msg = NULL;
    sec_msg_handle_t in_handle;
    sec_msg_handle_t out_handle;

    SEC_HAL_TRACE_ENTRY();

    /* allocate memory, from ICRAM, for msgs to be sent to TZ */
    msg_data_size = sec_msg_param_size(sizeof(uint32_t)) +
                        sec_msg_param_size(keybox_size);
    in_msg = sec_msg_alloc(&in_handle,
                           msg_data_size,
                           SEC_MSG_OBJECT_ID_NONE,
                           0,
                           SEC_HAL_MSG_BYTE_ORDER);
    msg_data_size = sec_msg_param_size(sizeof(sec_serv_status_t)) +
                        sec_msg_param_size(sizeof(uint32_t));
    out_msg = sec_msg_alloc(&out_handle,
                            msg_data_size,
                            SEC_MSG_OBJECT_ID_NONE,
                            0,
                            SEC_HAL_MSG_BYTE_ORDER);

    if (!in_msg || !out_msg) {
        SEC_HAL_TRACE("alloc failure, msgs not sent!");
        sec_hal_st = SEC_HAL_RES_FAIL;
    }
    else {
        /* write content to the input msg */
        sec_msg_param_write32(&in_handle,
                              keybox_size,
                              SEC_MSG_PARAM_ID_NONE);
        sec_msg_param_write(&in_handle,
                            keybox,
                            keybox_size,
                            SEC_MSG_PARAM_ID_NONE);
        LOCAL_WMB();

        /* call dispatcher */
        ssa_disp_st = LOCAL_DISP(SEC_SERV_DRM_OEMCRYPTO_INSTALL_KEYBOX,
                            LOCAL_DEFAULT_DISP_FLAGS,
                            LOCAL_DEFAULT_DISP_SPARE_PARAM,
                            SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
                            SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));

        /* interpret the response */
        sec_msg_st[0] = sec_msg_param_read32(&out_handle, &sec_serv_st);
        sec_msg_st[1] = sec_msg_param_read32(&out_handle, &sec_drm_st);
        LOCAL_RMB();
        if (SEC_ROM_RET_OK != ssa_disp_st ||
            SEC_MSG_STATUS_OK != sec_msg_st[0] ||
            SEC_MSG_STATUS_OK != sec_msg_st[1] ||
            SEC_SERV_STATUS_OK != sec_serv_st) {
            SEC_HAL_TRACE("INSTALL_KEYBOX failed!");
            SEC_HAL_TRACE_INT("ssa_disp_st", ssa_disp_st);
            SEC_HAL_TRACE_INT("sec_msg_st0", sec_msg_st[0]);
            SEC_HAL_TRACE_INT("sec_msg_st1", sec_msg_st[1]);
            SEC_HAL_TRACE_INT("sec_serv_st", sec_serv_st);
            sec_hal_st = SEC_HAL_RES_FAIL;
        }
   }

    /* de-allocate msgs */
    sec_msg_free(out_msg);
    sec_msg_free(in_msg);

    SEC_HAL_TRACE_EXIT();
    return (sec_hal_st == SEC_HAL_RES_OK ? sec_drm_st : sec_hal_st);
}


/* **************************************************************************
** Function name      : sec_hal_drm_is_keybox_valid
** Description        :
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** *************************************************************************/
uint32_t
sec_hal_drm_is_keybox_valid(void)
{
    uint32_t sec_hal_status = SEC_HAL_RES_OK;
    uint32_t sec_msg_status[2] = {SEC_MSG_STATUS_OK};
    uint32_t ssa_disp_status = 0x00;
    uint32_t sec_serv_status = 0x00;
    uint32_t sec_drm_status = 0x00;
    uint16_t msg_data_size;
    sec_msg_t *in_msg = NULL;
    sec_msg_t *out_msg = NULL;
    sec_msg_handle_t in_handle;
    sec_msg_handle_t out_handle;

    SEC_HAL_TRACE_ENTRY();

    /* allocate memory, from ICRAM, for msgs to be sent to TZ */
    msg_data_size = sec_msg_param_size(sizeof(uint32_t));
    in_msg = sec_msg_alloc(&in_handle,
                           msg_data_size,
                           SEC_MSG_OBJECT_ID_NONE,
                           0,
                           SEC_HAL_MSG_BYTE_ORDER);
    msg_data_size = sec_msg_param_size(sizeof(sec_serv_status_t)) +
                        sec_msg_param_size(sizeof(uint32_t));
    out_msg = sec_msg_alloc(&out_handle,
                            msg_data_size,
                            SEC_MSG_OBJECT_ID_NONE,
                            0,
                            SEC_HAL_MSG_BYTE_ORDER);

    if (!in_msg || !out_msg) {
        SEC_HAL_TRACE("alloc failure, msgs not sent!");
        sec_hal_status = SEC_HAL_RES_FAIL;
    }
    else {
        /* write content to the input msg */
        sec_msg_param_write32(&in_handle,
                              sizeof(uint32_t),
                              SEC_MSG_PARAM_ID_NONE);
        LOCAL_WMB();

        /* call dispatcher */
        ssa_disp_status = LOCAL_DISP(
                            SEC_SERV_DRM_OEMCRYPTO_IS_KEYBOX_VALID,
                            LOCAL_DEFAULT_DISP_FLAGS,
                            LOCAL_DEFAULT_DISP_SPARE_PARAM,
                            SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
                            SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));

        /* interpret the response */
        sec_msg_status[0] = sec_msg_param_read32(&out_handle, &sec_serv_status);
        sec_msg_status[1] = sec_msg_param_read32(&out_handle, &sec_drm_status);
        LOCAL_RMB();
        if (SEC_ROM_RET_OK != ssa_disp_status ||
            SEC_MSG_STATUS_OK != sec_msg_status[0] ||
            SEC_MSG_STATUS_OK != sec_msg_status[1] ||
            SEC_SERV_STATUS_OK != sec_serv_status) {
            SEC_HAL_TRACE("IS_KEYBOX_VALID failed!");
            SEC_HAL_TRACE_INT("ssa_disp_status", ssa_disp_status);
            SEC_HAL_TRACE_INT("sec_msg_status0", sec_msg_status[0]);
            SEC_HAL_TRACE_INT("sec_msg_status1", sec_msg_status[1]);
            SEC_HAL_TRACE_INT("sec_serv_status", sec_serv_status);
            sec_hal_status = SEC_HAL_RES_FAIL;
        }
   }

    /* de-allocate msgs */
    sec_msg_free(out_msg);
    sec_msg_free(in_msg);

    SEC_HAL_TRACE_INT("valid", !sec_drm_status);
    SEC_HAL_TRACE_EXIT();
    return (sec_hal_status == SEC_HAL_RES_OK ? sec_drm_status : sec_hal_status);
}

/* **************************************************************************
** Function name      : sec_hal_drm_get_random
** Description        : returns requested number of random number from TZ.
** Parameters         : IN/--- uint32_t size
**                      OUT/ --- uint8_t *random, ptr to array for random.
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** *************************************************************************/
uint32_t
sec_hal_drm_get_random(uint32_t size,
                       uint8_t *random)
{
    uint32_t sec_hal_status = SEC_HAL_RES_OK;
    uint32_t sec_msg_status[2] = {SEC_MSG_STATUS_OK};
    uint32_t ssa_disp_status = 0x00;
    uint32_t sec_serv_status = 0x00;
    uint32_t sec_drm_status = 0x00;
    uint16_t msg_data_size;
    sec_msg_t *in_msg = NULL;
    sec_msg_t *out_msg = NULL;
    sec_msg_handle_t in_handle;
    sec_msg_handle_t out_handle;

    SEC_HAL_TRACE_ENTRY();

    /* allocate memory, from RAM, for msgs to be sent to TZ */
    msg_data_size = sec_msg_param_size(sizeof(uint32_t));
    in_msg = sec_msg_alloc(&in_handle,
                           msg_data_size,
                           SEC_MSG_OBJECT_ID_NONE,
                           0,
                           SEC_HAL_MSG_BYTE_ORDER);
    msg_data_size = sec_msg_param_size(sizeof(sec_serv_status_t)) +
                    sec_msg_param_size(sizeof(uint32_t)) +
                    sec_msg_param_size(size);
    out_msg = sec_msg_alloc(&out_handle,
                            msg_data_size,
                            SEC_MSG_OBJECT_ID_NONE,
                            0,
                            SEC_HAL_MSG_BYTE_ORDER);

    if (!in_msg || !out_msg) {
        SEC_HAL_TRACE("alloc failure, msg not sent!");
        sec_hal_status = SEC_HAL_RES_FAIL;
    }
    else {
        /* write content to the input msg */
        sec_msg_param_write32(
                &in_handle,
                size,
                SEC_MSG_PARAM_ID_NONE);
        LOCAL_WMB();

        /* call dispatcher */
        ssa_disp_status = LOCAL_DISP(
                                SEC_SERV_DRM_OEMCRYPTO_GET_RANDOM,
                                LOCAL_DEFAULT_DISP_FLAGS,
                                LOCAL_DEFAULT_DISP_SPARE_PARAM,
                                SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
                                SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));

        /* interpret the response */
        sec_msg_status[0] = sec_msg_param_read32(&out_handle, &sec_serv_status);
        sec_msg_status[1] = sec_msg_param_read32(&out_handle, &sec_drm_status); 
        LOCAL_RMB();
        if (SEC_ROM_RET_OK != ssa_disp_status ||
            SEC_MSG_STATUS_OK != sec_msg_status[0] ||
            SEC_MSG_STATUS_OK != sec_msg_status[1] ||
            SEC_SERV_STATUS_OK != sec_serv_status) {
            SEC_HAL_TRACE("GET RANDOM failed!");
            SEC_HAL_TRACE_INT("ssa_disp_status", ssa_disp_status);
            SEC_HAL_TRACE_INT("sec_msg_status0", sec_msg_status[0]);
            SEC_HAL_TRACE_INT("sec_msg_status1", sec_msg_status[1]);
            SEC_HAL_TRACE_INT("sec_serv_status", sec_serv_status);
            sec_hal_status = SEC_HAL_RES_FAIL;
        }
        else
        {
            sec_msg_status[0] = sec_msg_param_read(&out_handle, random, size);
            LOCAL_RMB();
            if (SEC_MSG_STATUS_OK != sec_msg_status[0]) {
                SEC_HAL_TRACE("read failed!");
                sec_hal_status = SEC_HAL_RES_FAIL;
            }
        }
    }

    /* de-allocate msg */
    sec_msg_free(out_msg);
    sec_msg_free(in_msg);

    SEC_HAL_TRACE_EXIT();
    return (SEC_HAL_RES_OK == sec_hal_status ?
            sec_drm_status : sec_hal_status);
}

/* **************************************************************************
** Function name      : sec_hal_drm_get_device_id
** Description        :
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** *************************************************************************/
uint32_t
sec_hal_drm_get_device_id(uint32_t id_len,
                          uint8_t *id,
                          uint32_t *out_size)
{
    uint32_t sec_hal_status = SEC_HAL_RES_OK;
    uint32_t sec_msg_status[2] = {SEC_MSG_STATUS_OK};
    uint32_t ssa_disp_status = 0x00;
    uint32_t sec_serv_status = 0x00;
    uint32_t sec_drm_status = 0x00;
    uint16_t msg_data_size;
    sec_msg_t *in_msg = NULL;
    sec_msg_t *out_msg = NULL;
    sec_msg_handle_t in_handle;
    sec_msg_handle_t out_handle;

    SEC_HAL_TRACE_ENTRY();

    /* allocate memory, from ICRAM, for msgs to be sent to TZ */
    msg_data_size = sec_msg_param_size(sizeof(uint32_t));
    in_msg = sec_msg_alloc(&in_handle,
                           msg_data_size,
                           SEC_MSG_OBJECT_ID_NONE,
                           0,
                           SEC_HAL_MSG_BYTE_ORDER);
    msg_data_size = sec_msg_param_size(sizeof(sec_serv_status_t)) +
                        sec_msg_param_size(sizeof(uint32_t))*2 +
                        sec_msg_param_size(id_len);
    out_msg = sec_msg_alloc(&out_handle,
                            msg_data_size,
                            SEC_MSG_OBJECT_ID_NONE,
                            0,
                            SEC_HAL_MSG_BYTE_ORDER);

    if (!in_msg || !out_msg) {
        SEC_HAL_TRACE("alloc failure, msgs not sent!");
        sec_hal_status = SEC_HAL_RES_FAIL;
    }
    else {
        /* write content to the input msg */
        sec_msg_param_write32(&in_handle,
                              id_len,
                              SEC_MSG_PARAM_ID_NONE);
        LOCAL_WMB();

        /* call dispatcher */
        ssa_disp_status = LOCAL_DISP(
                            SEC_SERV_DRM_OEMCRYPTO_GET_DEVICE_ID,
                            LOCAL_DEFAULT_DISP_FLAGS,
                            LOCAL_DEFAULT_DISP_SPARE_PARAM,
                            SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
                            SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));

        /* interpret the response */
        sec_msg_status[0] = sec_msg_param_read32(&out_handle, &sec_serv_status);
        sec_msg_status[1] = sec_msg_param_read32(&out_handle, &sec_drm_status);
        LOCAL_RMB();
        if (SEC_ROM_RET_OK != ssa_disp_status ||
            SEC_MSG_STATUS_OK != sec_msg_status[0] ||
            SEC_MSG_STATUS_OK != sec_msg_status[1] ||
            SEC_SERV_STATUS_OK != sec_serv_status) {
            SEC_HAL_TRACE("GET_DEVICE_ID failed!");
            SEC_HAL_TRACE_INT("ssa_disp_status", ssa_disp_status);
            SEC_HAL_TRACE_INT("sec_msg_status0", sec_msg_status[0]);
            SEC_HAL_TRACE_INT("sec_msg_status1", sec_msg_status[1]);
            SEC_HAL_TRACE_INT("sec_serv_status", sec_serv_status);
            sec_hal_status = SEC_HAL_RES_FAIL;
        }
        else {
            sec_msg_status[0] = sec_msg_param_read32(&out_handle, out_size);
            sec_msg_status[1] = sec_msg_param_read(&out_handle, id, *out_size);
            LOCAL_RMB();
            if (SEC_MSG_STATUS_OK != sec_msg_status[0] ||
                SEC_MSG_STATUS_OK != sec_msg_status[1]) {
                SEC_HAL_TRACE("read failed!");
                SEC_HAL_TRACE_INT("sec_msg_status0", sec_msg_status[0]);
                SEC_HAL_TRACE_INT("sec_msg_status1", sec_msg_status[1]);
                SEC_HAL_TRACE_INT("id_len         ", id_len);
                SEC_HAL_TRACE_INT("*out_size      ", *out_size);
                sec_hal_status = SEC_HAL_RES_FAIL;
            }
        }
    }

    /* de-allocate msgs */
    sec_msg_free(out_msg);
    sec_msg_free(in_msg);

    SEC_HAL_TRACE_EXIT();
    return (sec_hal_status == SEC_HAL_RES_OK ?
            sec_drm_status : sec_hal_status);
}

/* **************************************************************************
** Function name      : sec_hal_drm_get_key_data
** Description        :
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** *************************************************************************/
uint32_t
sec_hal_drm_get_key_data(uint32_t key_data_len,
                         uint8_t *key_data,
                         uint32_t *out_size)
{
    uint32_t sec_hal_status = SEC_HAL_RES_OK;
    uint32_t sec_msg_status[2] = {SEC_MSG_STATUS_OK};
    uint32_t ssa_disp_status = 0x00;
    uint32_t sec_serv_status = 0x00;
    uint32_t sec_drm_status = 0x00;
    uint16_t msg_data_size;
    sec_msg_t *in_msg = NULL;
    sec_msg_t *out_msg = NULL;
    sec_msg_handle_t in_handle;
    sec_msg_handle_t out_handle;

    SEC_HAL_TRACE_ENTRY();

    /* allocate memory, from ICRAM, for msgs to be sent to TZ */
    msg_data_size = sec_msg_param_size(sizeof(uint32_t));
    in_msg = sec_msg_alloc(&in_handle,
                           msg_data_size,
                           SEC_MSG_OBJECT_ID_NONE,
                           0,
                           SEC_HAL_MSG_BYTE_ORDER);
    msg_data_size = sec_msg_param_size(sizeof(sec_serv_status_t)) +
                        sec_msg_param_size(sizeof(uint32_t))*2 +
                        sec_msg_param_size(key_data_len);
    out_msg = sec_msg_alloc(&out_handle,
                            msg_data_size,
                            SEC_MSG_OBJECT_ID_NONE,
                            0,
                            SEC_HAL_MSG_BYTE_ORDER);

    if (!in_msg || !out_msg) {
        SEC_HAL_TRACE("alloc failure, msgs not sent!");
        sec_hal_status = SEC_HAL_RES_FAIL;
    }
    else {
        /* write content to the input msg */
        sec_msg_param_write32(&in_handle,
                              key_data_len,
                              SEC_MSG_PARAM_ID_NONE);
        LOCAL_WMB();

        /* call dispatcher */
        ssa_disp_status = LOCAL_DISP(
                            SEC_SERV_DRM_OEMCRYPTO_GET_KEYDATA,
                            LOCAL_DEFAULT_DISP_FLAGS,
                            LOCAL_DEFAULT_DISP_SPARE_PARAM,
                            SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
                            SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));

        /* interpret the response */
        sec_msg_status[0] = sec_msg_param_read32(&out_handle, &sec_serv_status);
        sec_msg_status[1] = sec_msg_param_read32(&out_handle, &sec_drm_status);
        LOCAL_RMB();
        if (SEC_ROM_RET_OK != ssa_disp_status ||
            SEC_MSG_STATUS_OK != sec_msg_status[0] ||
            SEC_MSG_STATUS_OK != sec_msg_status[1] ||
            SEC_SERV_STATUS_OK != sec_serv_status) {
            SEC_HAL_TRACE("GET_DEVICE_ID failed!");
            SEC_HAL_TRACE_INT("ssa_disp_status", ssa_disp_status);
            SEC_HAL_TRACE_INT("sec_msg_status0", sec_msg_status[0]);
            SEC_HAL_TRACE_INT("sec_msg_status1", sec_msg_status[1]);
            SEC_HAL_TRACE_INT("sec_serv_status", sec_serv_status);
            sec_hal_status = SEC_HAL_RES_FAIL;
        }
        else {
            sec_msg_status[0] = sec_msg_param_read32(&out_handle, out_size);
            sec_msg_status[1] = sec_msg_param_read(&out_handle, key_data, *out_size);
            LOCAL_RMB();
            if (SEC_MSG_STATUS_OK != sec_msg_status[0] ||
                SEC_MSG_STATUS_OK != sec_msg_status[1]) {
                SEC_HAL_TRACE("read failed!");
                SEC_HAL_TRACE_INT("sec_msg_status0", sec_msg_status[0]);
                SEC_HAL_TRACE_INT("sec_msg_status1", sec_msg_status[1]);
                SEC_HAL_TRACE_INT("key_data_len   ", key_data_len);
                SEC_HAL_TRACE_INT("*out_size      ", *out_size);
                sec_hal_status = SEC_HAL_RES_FAIL;
            }
        }
    }

    /* de-allocate msgs */
    sec_msg_free(out_msg);
    sec_msg_free(in_msg);

    SEC_HAL_TRACE_EXIT();
    return (sec_hal_status == SEC_HAL_RES_OK ?
            sec_drm_status : sec_hal_status);
}


