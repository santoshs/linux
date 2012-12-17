/* *********************************************************************** **
**                               Renesas                                   **
** *********************************************************************** */

/* *************************** COPYRIGHT INFORMATION ********************* **
** This program contains proprietary information that is a trade secret of **
** Renesas and also is protected as an unpublished work under              **
** applicable Copyright laws. Recipient is to retain this program in       **
** confidence and is not permitted to use or make copies thereof other than**
** as permitted in a written agreement with Renesas.                       **
**                                                                         **
** Copyright (C) 2010-2012 Renesas Electronics Corp.                       **
** All rights reserved.                                                    **
** *********************************************************************** */


/* ************************ HEADER (INCLUDE) SECTION ********************* */

#define SEC_HAL_TRACE_LOCAL_ENABLE

#include "sec_hal_rt.h"
#include "sec_hal_rt_cmn.h"
#include "sec_hal_rt_trace.h"
#include "sec_serv_api.h"
#include "sec_msg.h"
#include "sec_dispatch.h"

#ifndef SEC_HAL_TEST_ISOLATION
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#else
#include <string.h>
#endif /* SEC_HAL_TEST_ISOLATION */


/* ***************** MACROS, CONSTANTS, COMPILATION FLAGS **************** */
#define LOCAL_DISP                         pub2sec_dispatcher
#define LOCAL_DEFAULT_DISP_FLAGS           0
#define LOCAL_DEFAULT_DISP_SPARE_PARAM     0
#define LOCAL_WMB()                        wmb()
#define LOCAL_RMB()                        rmb()
#define LOCAL_DEFAULT_PERIODIC_TIMER_VALUE 60000
#define LOCAL_DEFAULT_SIMLOCK_LEVEL_COUNT  5

#define LOCAL_CPY_TO_CLIENT_FUNC    copy_to_user
#define LOCAL_CPY_TO_CLIENT_FUNCPTR &copy_to_user

#ifdef SEC_STORAGE_SELFTEST_ENABLE

#include "sec_hal_dev_ioctl.h"

#define RPC_SUCCESS                      0

/* structure copied from secure, but only file_handle & error are used */
typedef struct {
  uint32_t filesystem;
  uint64_t file_handle;
  uint64_t io_position;
  uint32_t state;
  uint32_t eof;
  uint32_t error;
} sec_sto_file_t;

sec_sto_file_t fstream[11];

sec_hal_rt_rpc_handler rpch = NULL;


/* function prototypes needed in secure storage testing */
sec_sto_file_t *ssfopen(const char *filename, const char *mode, int index);
size_t          ssfread(void *buffer, size_t size, size_t nitems, sec_sto_file_t *stream);
size_t          ssfwrite(void *buffer, size_t size, size_t nitems, sec_sto_file_t *stream);
uint32_t        ssfclose(sec_sto_file_t *file);
size_t          ssfsize(sec_sto_file_t *file);
uint32_t        ssfgetroot(uint32_t *rhandle);
uint32_t        ssfmv(const char *from, const char *to);
uint32_t        ssfrm(const char *filename);
#endif


/* empty unlock code for simlock */
const char* k_sec_hal_simlock_empty_code = "EMPTY";



/* ****************************** CODE SECTION *************************** */
/* **************************************************************************
** Function name      : sec_hal_rt_init
** Description        : initialize global/local resources.
** Parameters         : OUT/--- sec_hal_init_info_t *runtime_int,
**                      version information and reset register values.
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** *************************************************************************/
uint32_t
sec_hal_rt_init(
    sec_hal_init_info_t *runtime_init)
{
    uint32_t sec_hal_st = SEC_HAL_RES_OK;
    uint32_t sec_msg_st = SEC_MSG_STATUS_OK;
    uint32_t ssa_disp_st = 0x00;
    uint32_t sec_serv_st = 0x00;
    uint16_t size;
    sec_msg_t *in_msg = NULL;
    sec_msg_t *out_msg = NULL;
    sec_msg_handle_t in_handle;
    sec_msg_handle_t out_handle;

    SEC_HAL_TRACE_ENTRY();

    /* allocate memory, from ICRAM, for msgs to be sent to TZ */
    size = sec_msg_param_size(sizeof(uint32_t))*2;
    in_msg = sec_msg_alloc(&in_handle, size, SEC_MSG_OBJECT_ID_NONE,
                0, SEC_HAL_MSG_BYTE_ORDER);
    size = sec_msg_param_size(sizeof(sec_serv_status_t)) +
            sec_msg_param_size(sizeof(uint32_t))*2 +
            sec_msg_param_size(sizeof(uint64_t)) +
            sec_msg_param_size(sizeof(uint32_t)*SEC_HAL_RESCNT_COUNT);
    out_msg = sec_msg_alloc(&out_handle, size, SEC_MSG_OBJECT_ID_NONE,
                0, SEC_HAL_MSG_BYTE_ORDER);

    do {
        if (!in_msg || !out_msg) {
            SEC_HAL_TRACE("alloc failure, msgs not sent!");
            sec_hal_st = SEC_HAL_RES_FAIL;
            break;
        }

        /* write content to the input msg */
        if (sec_msg_param_write32(&in_handle,
               sizeof(uint64_t),
               SEC_MSG_PARAM_ID_NONE) != SEC_MSG_STATUS_OK) {
            SEC_HAL_TRACE("commit_id_length write error, aborting!");
            sec_hal_st = SEC_HAL_RES_FAIL;
            break;
        }
        if (sec_msg_param_write32(&in_handle,
               sizeof(uint32_t)*SEC_HAL_RESCNT_COUNT,
               SEC_MSG_PARAM_ID_NONE) != SEC_MSG_STATUS_OK) {
            SEC_HAL_TRACE("reset_info_length write error, aborting!");
            sec_hal_st = SEC_HAL_RES_FAIL;
            break;
        }
        LOCAL_WMB();


        /* call dispatcher */
        ssa_disp_st = LOCAL_DISP(SEC_SERV_RUNTIME_INIT,
                LOCAL_DEFAULT_DISP_FLAGS,
                LOCAL_DEFAULT_DISP_SPARE_PARAM,
                SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
                SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));

        /* interpret the response(s) */
        sec_msg_st = sec_msg_param_read32(&out_handle, &sec_serv_st);
        LOCAL_RMB();
        if (SEC_ROM_RET_OK != ssa_disp_st ||
            SEC_MSG_STATUS_OK != sec_msg_st ||
            SEC_SERV_STATUS_OK != sec_serv_st) {
            SEC_HAL_TRACE("failure! disp==%d, msg==%d, serv==%d",
                    ssa_disp_st, sec_msg_st, sec_serv_st);
            sec_hal_st = SEC_HAL_RES_FAIL;
            break;
        }

        if (!runtime_init) {
            break; /* runtime_init arg not given, can exit now. */
        }

        sec_msg_st = sec_msg_param_read32(&out_handle, (uint32_t*)&size);
        LOCAL_RMB();
        if (SEC_MSG_STATUS_OK != sec_msg_st ||
            sec_msg_param_read(&out_handle,
                &(runtime_init->commit_id),
                size < sizeof(uint64_t) ?
                size : sizeof(uint64_t)) != SEC_MSG_STATUS_OK) {
            SEC_HAL_TRACE("commit_id read error, aborting!");
            sec_hal_st = SEC_HAL_RES_FAIL;
            break;
        }
        sec_msg_st = sec_msg_param_read32(&out_handle, (uint32_t*)&size);
        LOCAL_RMB();
        if (SEC_MSG_STATUS_OK != sec_msg_st ||
            sec_msg_param_read(&out_handle,
                &(runtime_init->reset_info[0]),
                size < sizeof(uint32_t)*SEC_HAL_RESCNT_COUNT ?
                size : sizeof(uint32_t)*SEC_HAL_RESCNT_COUNT)
                    != SEC_MSG_STATUS_OK) {
            SEC_HAL_TRACE("reset_info read error, aborting!");
            sec_hal_st = SEC_HAL_RES_FAIL;
            break;

        }
        LOCAL_RMB(); /* to ensure last read being completed */
    } while(0);

    /* de-allocate msgs */
    sec_msg_free(out_msg);
    sec_msg_free(in_msg);


    SEC_HAL_TRACE_EXIT();
    return sec_hal_st;
}

/* **************************************************************************
** Function name      : sec_hal_rt_install_rpc_handler
** Description        : install rpc function to TZ.
** Parameters         : IN/--- sec_hal_rt_rpc_handler virt_func_ptr_in
**                      virtual callback function address.
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** *************************************************************************/
uint32_t
sec_hal_rt_install_rpc_handler(
    sec_hal_rt_rpc_handler virt_func_ptr_in)
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

    if (NULL == virt_func_ptr_in) {
        SEC_HAL_TRACE_EXIT_INFO("!!null virt_func_ptr_in, aborting!!");
        return SEC_HAL_RES_PARAM_ERROR;
    }

    /* allocate memory, from ICRAM, for msgs to be sent to TZ */
    msg_data_size = sec_msg_param_size(sizeof(uint32_t));
    in_msg = sec_msg_alloc(&in_handle, msg_data_size,
            SEC_MSG_OBJECT_ID_NONE, 0, SEC_HAL_MSG_BYTE_ORDER);
    msg_data_size = sec_msg_param_size(sizeof(sec_serv_status_t)) +
            sec_msg_param_size(sizeof(uint32_t));
    out_msg = sec_msg_alloc(&out_handle, msg_data_size,
            SEC_MSG_OBJECT_ID_NONE, 0, SEC_HAL_MSG_BYTE_ORDER);

    if (NULL == in_msg || NULL == out_msg) {
        SEC_HAL_TRACE("alloc failure, msgs not sent!");
        sec_hal_status = SEC_HAL_RES_FAIL;
    }
    else {
        /* write content to the input msg */
        sec_msg_param_write32(
                &in_handle,
                (uint32_t)virt_func_ptr_in,
                SEC_MSG_PARAM_ID_NONE);

        /* call dispatcher */
        ssa_disp_status = LOCAL_DISP(SEC_SERV_RPC_ADDRESS,
                LOCAL_DEFAULT_DISP_FLAGS,
                LOCAL_DEFAULT_DISP_SPARE_PARAM,
                SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
                SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));

        /* interpret the response */
        sec_msg_status = sec_msg_param_read32(&out_handle, &sec_serv_status);
        if (SEC_ROM_RET_OK != ssa_disp_status ||
            SEC_MSG_STATUS_OK != sec_msg_status ||
            SEC_SERV_STATUS_OK != sec_serv_status) {
            SEC_HAL_TRACE("op failed, INSTALL rpc handler failed!");
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
** Function name      : sec_hal_rt_key_info_get
** Description        : request key information from the TZ side.
** Parameters         : OUT/--- sec_hal_key_info_t *key_info
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** *************************************************************************/
uint32_t
sec_hal_rt_key_info_get(sec_hal_key_info_t *user_key_info_out) {

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

    if (NULL == user_key_info_out) {
        SEC_HAL_TRACE_EXIT_INFO("!!null user_key_info_out, aborting!!");
        return SEC_HAL_RES_PARAM_ERROR;
    }

    /* allocate memory, from ICRAM, for msgs to be sent to TZ */
    msg_data_size = sec_msg_param_size(sizeof(uint32_t));
    in_msg = sec_msg_alloc(&in_handle, msg_data_size,
            SEC_MSG_OBJECT_ID_NONE, 0, SEC_HAL_MSG_BYTE_ORDER);
    msg_data_size = sec_msg_param_size(sizeof(sec_serv_status_t)) +
            sec_msg_param_size(SEC_HAL_KEY_INFO_SIZE);
    out_msg = sec_msg_alloc(&out_handle, msg_data_size,
            SEC_MSG_OBJECT_ID_NONE, 0, SEC_HAL_MSG_BYTE_ORDER);

    if (NULL == in_msg || NULL == out_msg) {
        SEC_HAL_TRACE("alloc failure, msgs not sent!");
        sec_hal_status = SEC_HAL_RES_FAIL;
    }
    else {
        /* write content to the input msg */
        sec_msg_param_write32(
                &in_handle,
                SEC_HAL_KEY_INFO_SIZE,
                SEC_MSG_PARAM_ID_NONE);

        /* call dispatcher */
        ssa_disp_status = LOCAL_DISP(
                SEC_SERV_KEY_INFO_REQUEST,
                LOCAL_DEFAULT_DISP_FLAGS,
                LOCAL_DEFAULT_DISP_SPARE_PARAM,
                SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
                SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));

        /* interpret the response */
        sec_msg_status = sec_msg_param_read32(&out_handle, &sec_serv_status);
        if (SEC_ROM_RET_OK != ssa_disp_status ||
            SEC_MSG_STATUS_OK != sec_msg_status ||
            SEC_SERV_STATUS_OK != sec_serv_status) {
            SEC_HAL_TRACE("op failed, KEY INFO not available!");
            SEC_HAL_TRACE_INT("ssa_disp_status", ssa_disp_status);
            SEC_HAL_TRACE_INT("sec_msg_status ", sec_msg_status);
            SEC_HAL_TRACE_INT("sec_serv_status", sec_serv_status);
            sec_hal_status = SEC_HAL_RES_FAIL;
        }
        else {
            sec_msg_status = _sec_msg_param_read(LOCAL_CPY_TO_CLIENT_FUNCPTR,
                    &out_handle, user_key_info_out, SEC_HAL_KEY_INFO_SIZE);
            if (SEC_MSG_STATUS_OK != sec_msg_status) {
                SEC_HAL_TRACE_INT("read failed! sec_msg_status", sec_msg_status);
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
** Function name      : sec_hal_rt_cert_register
** Description        : register given certificate on TZ side.
** Parameters         : IN/--- void *cert
**                      IN/--- uint32_t cert_size
**                      OUT/ --- uint32_t *obj_id
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** *************************************************************************/
uint32_t
sec_hal_rt_cert_register(void *user_cert_in,
                         uint32_t user_cert_size_in,
                         uint32_t *user_obj_id_out) {

    uint32_t sec_hal_status = SEC_HAL_RES_OK;
    uint32_t sec_msg_status = SEC_MSG_STATUS_OK;
    uint32_t ssa_disp_status = 0x00;
    uint32_t sec_serv_status = 0x00;
    uint16_t msg_data_size;
    sec_msg_t *in_msg = NULL;
    sec_msg_t *out_msg = NULL;
    sec_msg_handle_t in_handle;
    sec_msg_handle_t out_handle;
    void *kernel_cert_in;

    SEC_HAL_TRACE_ENTRY();

    if (NULL == user_cert_in) {
        SEC_HAL_TRACE_EXIT_INFO("!!null user_cert_in ptr, aborting!!");
        return SEC_HAL_RES_PARAM_ERROR;
    }
    if (0 == user_cert_size_in) {
        SEC_HAL_TRACE_EXIT_INFO("!!zero user_cert_size_in, aborting!!");
        return SEC_HAL_RES_PARAM_ERROR;
    }

    SEC_HAL_TRACE("user_cert_in: 0x%x", user_cert_in);
    SEC_HAL_TRACE("user_cert_size_in: %d", user_cert_size_in);

    kernel_cert_in = kmalloc(user_cert_size_in, GFP_KERNEL);
    copy_from_user(kernel_cert_in, user_cert_in, user_cert_size_in);
    SEC_HAL_TRACE("kernel_cert_in: 0x%x", kernel_cert_in);

    /* allocate memory, from ICRAM, for msgs to be sent to TZ */
    msg_data_size = sec_msg_param_size(sizeof(uint32_t));
    in_msg = sec_msg_alloc(&in_handle, msg_data_size,
            SEC_MSG_OBJECT_ID_NONE, 0, SEC_HAL_MSG_BYTE_ORDER);
    msg_data_size = sec_msg_param_size(sizeof(sec_serv_status_t)) +
            sec_msg_param_size(sizeof(uint32_t));
    out_msg = sec_msg_alloc(&out_handle, msg_data_size,
            SEC_MSG_OBJECT_ID_NONE, 0, SEC_HAL_MSG_BYTE_ORDER);

    if (NULL == in_msg || NULL == out_msg) {
        SEC_HAL_TRACE("alloc failure, msgs not sent!");
        sec_hal_status = SEC_HAL_RES_FAIL;
    }
    else {
        /* ensure that certificate is fully in work RAM memory */
        SEC_HAL_MEM_CACHE_CLEAN_FUNC(kernel_cert_in, user_cert_size_in);

        /* write content to the input msg */
        sec_msg_param_write32(
                &in_handle,
                (uint32_t)virt_to_phys(kernel_cert_in),
                SEC_MSG_PARAM_ID_NONE);

        /* call dispatcher */
        ssa_disp_status = LOCAL_DISP(
                SEC_SERV_CERTIFICATE_REGISTER,
                LOCAL_DEFAULT_DISP_FLAGS,
                LOCAL_DEFAULT_DISP_SPARE_PARAM,
                SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
                SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));

        /* interpret the response */
        sec_msg_status = sec_msg_param_read32(&out_handle, &sec_serv_status);
        if (SEC_ROM_RET_OK != ssa_disp_status ||
            SEC_MSG_STATUS_OK != sec_msg_status ||
            SEC_SERV_STATUS_OK != sec_serv_status) {
            SEC_HAL_TRACE("op failed, CERT register failed!");
            SEC_HAL_TRACE_INT("ssa_disp_status", ssa_disp_status);
            SEC_HAL_TRACE_INT("sec_msg_status ", sec_msg_status);
            SEC_HAL_TRACE_INT("sec_serv_status", sec_serv_status);
            sec_hal_status = SEC_HAL_RES_FAIL;
        }
    }

    /* de-allocate msgs */
    sec_msg_free(out_msg);
    sec_msg_free(in_msg);
    kfree(kernel_cert_in);

    SEC_HAL_TRACE_EXIT();
    return sec_hal_status;
}

/* **************************************************************************
** Function name      : sec_hal_rt_data_cert_register
** Description        : register given data certificate on TZ side,
**                      temporary data certificate.
** Parameters         : IN/--- void *cert
**                      IN/--- uint32_t cert_size
**                      IN/--- void *data
**                      IN/--- uint32_t data_size
**                      OUT/ --- uint32_t *id_ptr, ptr where to copy obj_id.
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** *************************************************************************/
uint32_t
sec_hal_rt_data_cert_register(void *user_cert_in,
                              uint32_t user_cert_size_in,
                              void *user_data_in,
                              uint32_t user_data_size_in,
                              uint32_t *user_id_ptr_out) {

    uint32_t sec_hal_status = SEC_HAL_RES_OK;
    uint32_t sec_msg_status = SEC_MSG_STATUS_OK;
    uint32_t ssa_disp_status = 0x00;
    uint32_t sec_serv_status = 0x00;
    uint32_t obj_id = 0x00;
    uint16_t msg_data_size;
    sec_msg_t *in_msg = NULL;
    sec_msg_t *out_msg = NULL;
    sec_msg_handle_t in_handle;
    sec_msg_handle_t out_handle;
    void *kernel_cert_in;
    void *kernel_data_in;

    SEC_HAL_TRACE_ENTRY();

    if (NULL == user_cert_in) {
        SEC_HAL_TRACE_EXIT_INFO("!!null user_cert_in ptr, aborting!!");
        return SEC_HAL_RES_PARAM_ERROR;
    }
    if (0 == user_cert_size_in) {
        SEC_HAL_TRACE_EXIT_INFO("!!zero user_cert_size_in, aborting!!");
        return SEC_HAL_RES_PARAM_ERROR;
    }
    if (NULL == user_data_in) {
        SEC_HAL_TRACE_EXIT_INFO("!!null user_data_in ptr, aborting!!");
        return SEC_HAL_RES_PARAM_ERROR;
    }
    if (0 == user_data_size_in) {
        SEC_HAL_TRACE_EXIT_INFO("!!zero user_data_size_in, aborting!!");
        return SEC_HAL_RES_PARAM_ERROR;
    }

    SEC_HAL_TRACE("user_cert_in: 0x%x", user_cert_in);
    SEC_HAL_TRACE("user_cert_size_in: %d", user_cert_size_in);
    SEC_HAL_TRACE("user_data_in: 0x%x", user_data_in);
    SEC_HAL_TRACE("user_data_size_in: %d", user_data_size_in);

    kernel_cert_in = kmalloc(user_cert_size_in, GFP_KERNEL);
    copy_from_user( kernel_cert_in, user_cert_in, user_cert_size_in );
    kernel_data_in = kmalloc(user_data_size_in, GFP_KERNEL);
    copy_from_user( kernel_data_in, user_data_in, user_data_size_in );

    SEC_HAL_TRACE("kernel_cert_in: 0x%x", kernel_cert_in);
    SEC_HAL_TRACE("kernel_data_in: 0x%x", kernel_data_in);

    /* allocate memory, from ICRAM, for msgs to be sent to TZ */
    msg_data_size = sec_msg_param_size(sizeof(uint32_t))*3;
    in_msg = sec_msg_alloc(&in_handle, msg_data_size,
            SEC_MSG_OBJECT_ID_NONE, 0, SEC_HAL_MSG_BYTE_ORDER);
    msg_data_size = sec_msg_param_size(sizeof(sec_serv_status_t)) +
            sec_msg_param_size(sizeof(uint32_t));
    out_msg = sec_msg_alloc(&out_handle, msg_data_size,
            SEC_MSG_OBJECT_ID_NONE, 0, SEC_HAL_MSG_BYTE_ORDER);

    if (NULL == in_msg || NULL == out_msg) {
        SEC_HAL_TRACE("alloc failure, msgs not sent!");
        sec_hal_status = SEC_HAL_RES_FAIL;
    }
    else {
        /* ensure that certificate and data are fully in RAM memory */
        SEC_HAL_MEM_CACHE_CLEAN_FUNC(kernel_cert_in, user_cert_size_in);
        SEC_HAL_MEM_CACHE_CLEAN_FUNC(kernel_data_in, user_data_size_in);

        /* write content to the input msg */
        sec_msg_param_write32(
                &in_handle,
                (uint32_t)virt_to_phys(kernel_cert_in),
                SEC_MSG_PARAM_ID_NONE);
        sec_msg_param_write32(
                &in_handle,
                (uint32_t)virt_to_phys(kernel_data_in),
                SEC_MSG_PARAM_ID_NONE);
        sec_msg_param_write32(&in_handle,
                user_data_size_in,
                SEC_MSG_PARAM_ID_NONE);

        /* call dispatcher */
        ssa_disp_status = LOCAL_DISP(
                SEC_SERV_PROT_DATA_REGISTER,
                LOCAL_DEFAULT_DISP_FLAGS,
                LOCAL_DEFAULT_DISP_SPARE_PARAM,
                SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
                SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));

        /* interpret the response */
        sec_msg_status = sec_msg_param_read32(&out_handle, &sec_serv_status);
        if (SEC_ROM_RET_OK != ssa_disp_status ||
            SEC_MSG_STATUS_OK != sec_msg_status ||
            SEC_SERV_STATUS_OK != sec_serv_status) {
            SEC_HAL_TRACE("op failed, data CERT register failed!");
            SEC_HAL_TRACE_INT("ssa_disp_status", ssa_disp_status);
            SEC_HAL_TRACE_INT("sec_msg_status ", sec_msg_status);
            SEC_HAL_TRACE_INT("sec_serv_status", sec_serv_status);
            sec_hal_status = SEC_HAL_RES_FAIL;
        }
        else if (NULL != user_id_ptr_out) {
            sec_msg_status = sec_msg_param_read32(&out_handle, &obj_id);
            if (SEC_MSG_STATUS_OK != sec_msg_status ||
                LOCAL_CPY_TO_CLIENT_FUNC(user_id_ptr_out, (void*)&obj_id, sizeof(uint32_t))) {
                SEC_HAL_TRACE_INT("read&copy failed! sec_msg_status", sec_msg_status);
                sec_hal_status = SEC_HAL_RES_FAIL;
            }
        }
    }

    /* de-allocate msgs */
    sec_msg_free(out_msg);
    sec_msg_free(in_msg);
    kfree(kernel_cert_in);
    kfree(kernel_data_in);

    SEC_HAL_TRACE_EXIT();
    return sec_hal_status;
}

/* **************************************************************************
** Function name      : sec_hal_rt_mac_address_get
** Description        : store requested MAC address on given userspace addr.
** Parameters         : IN/--- sec_hal_mac_address_t *mac_addr
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** *************************************************************************/
uint32_t
sec_hal_rt_mac_address_get(uint32_t user_index_in,
                           sec_hal_mac_address_t *user_mac_addr_out) {

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

    if (NULL == user_mac_addr_out) {
        SEC_HAL_TRACE_EXIT_INFO("!!null user_mac_addr_out ptr, aborting!!");
        return SEC_HAL_RES_PARAM_ERROR;
    }

    /* allocate memory, from RAM, for msgs to be sent to TZ */
    msg_data_size = sec_msg_param_size(sizeof(uint32_t))*2;
    in_msg = sec_msg_alloc(&in_handle, msg_data_size,
            SEC_MSG_OBJECT_ID_NONE, 0, SEC_HAL_MSG_BYTE_ORDER);
    msg_data_size = sec_msg_param_size(sizeof(sec_serv_status_t)) +
            sec_msg_param_size(SEC_HAL_MAC_SIZE);
    out_msg = sec_msg_alloc(&out_handle, msg_data_size,
            SEC_MSG_OBJECT_ID_NONE, 0, SEC_HAL_MSG_BYTE_ORDER);

    if (NULL == in_msg || NULL == out_msg) {
        SEC_HAL_TRACE("alloc failure, msg not sent!");
        sec_hal_status = SEC_HAL_RES_FAIL;
    }
    else {
        /* write content to the input msg */
        sec_msg_param_write32(
                &in_handle,
                SEC_HAL_MAC_SIZE,
                SEC_MSG_PARAM_ID_NONE);
        sec_msg_param_write32(
                &in_handle,
                user_index_in,
                SEC_MSG_PARAM_ID_NONE);

        /* call dispatcher */
        ssa_disp_status = LOCAL_DISP(
                SEC_SERV_MAC_ADDRESS_REQUEST,
                LOCAL_DEFAULT_DISP_FLAGS,
                LOCAL_DEFAULT_DISP_SPARE_PARAM,
                SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
                SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));

        /* interpret the response */
        sec_msg_status = sec_msg_param_read32(&out_handle, &sec_serv_status);
        if (SEC_ROM_RET_OK != ssa_disp_status ||
            SEC_MSG_STATUS_OK != sec_msg_status ||
            SEC_SERV_STATUS_OK != sec_serv_status) {
            SEC_HAL_TRACE("op failed, MAC not available!");
            SEC_HAL_TRACE_INT("ssa_disp_status", ssa_disp_status);
            SEC_HAL_TRACE_INT("sec_msg_status ", sec_msg_status);
            SEC_HAL_TRACE_INT("sec_serv_status", sec_serv_status);
            sec_hal_status = SEC_HAL_RES_FAIL;
        }
        else {
            sec_msg_status = _sec_msg_param_read(LOCAL_CPY_TO_CLIENT_FUNCPTR,
                        &out_handle,
                        user_mac_addr_out,
                        SEC_HAL_MAC_SIZE);
            if (SEC_MSG_STATUS_OK != sec_msg_status) {
                SEC_HAL_TRACE_INT("read failed! sec_msg_status", sec_msg_status);
                sec_hal_status = SEC_HAL_RES_FAIL;
            }
        }
    }

    /* de-allocate msg */
    sec_msg_free(out_msg);
    sec_msg_free(in_msg);

    SEC_HAL_TRACE_EXIT();
    return sec_hal_status;
}


/* **************************************************************************
** Function name      : sec_hal_rt_imei_get
** Description        : stores device's IMEI code to the given address.
** Parameters         : IN/--- sec_hal_imei_t *imei
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** *************************************************************************/
uint32_t
sec_hal_rt_imei_get(sec_hal_imei_t *user_imei_out){

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

    if (NULL == user_imei_out) {
        SEC_HAL_TRACE_EXIT_INFO("!!null user_imei_out ptr, aborting!!");
        return SEC_HAL_RES_PARAM_ERROR;
    }

    /* allocate memory, from ICRAM, for msgs to be sent to TZ */
    msg_data_size = sec_msg_param_size(sizeof(uint32_t));
    in_msg = sec_msg_alloc(&in_handle, msg_data_size,
            SEC_MSG_OBJECT_ID_NONE, 0, SEC_HAL_MSG_BYTE_ORDER);
    msg_data_size = sec_msg_param_size(sizeof(sec_serv_status_t)) +
            sec_msg_param_size(SEC_HAL_MAX_IMEI_SIZE);
    out_msg = sec_msg_alloc(&out_handle, msg_data_size,
            SEC_MSG_OBJECT_ID_NONE, 0, SEC_HAL_MSG_BYTE_ORDER);

    if (NULL == in_msg || NULL == out_msg) {
        SEC_HAL_TRACE("alloc failure, msg not sent!");
        sec_hal_status = SEC_HAL_RES_FAIL;
    }
    else {
        /* write content to the input msg */
        sec_msg_param_write32(
                &in_handle,
                SEC_HAL_MAX_IMEI_SIZE,
                SEC_MSG_PARAM_ID_NONE);

        /* call dispatcher */
        ssa_disp_status = LOCAL_DISP(
                SEC_SERV_IMEI_REQUEST,
                LOCAL_DEFAULT_DISP_FLAGS,
                LOCAL_DEFAULT_DISP_SPARE_PARAM,
                SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
                SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));

        /* interpret the response */
        sec_msg_status = sec_msg_param_read32(&out_handle, &sec_serv_status);
        if (SEC_ROM_RET_OK != ssa_disp_status ||
            SEC_MSG_STATUS_OK != sec_msg_status ||
            SEC_SERV_STATUS_OK != sec_serv_status) {
            SEC_HAL_TRACE("op failed, IMEI not available!");
            SEC_HAL_TRACE_INT("ssa_disp_status", ssa_disp_status);
            SEC_HAL_TRACE_INT("sec_msg_status ", sec_msg_status);
            SEC_HAL_TRACE_INT("sec_serv_status", sec_serv_status);
            sec_hal_status = SEC_HAL_RES_FAIL;
        }
        else {
            sec_msg_status = _sec_msg_param_read(
                    LOCAL_CPY_TO_CLIENT_FUNCPTR,
                    &out_handle,
                    user_imei_out,
                    SEC_HAL_MAX_IMEI_SIZE);
            if (SEC_MSG_STATUS_OK != sec_msg_status) {
                SEC_HAL_TRACE_INT("read failed! sec_msg_status", sec_msg_status);
                sec_hal_status = SEC_HAL_RES_FAIL;
            }
        }
    }

    /* de-allocate msg */
    sec_msg_free(out_msg);
    sec_msg_free(in_msg);

    SEC_HAL_TRACE_EXIT();
    return sec_hal_status;
}

/* **************************************************************************
** Function name      : sec_hal_rt_simlock_levels_open
** Description        : opens all simlock levels with given codes.
** Parameters         : IN/--- char levels_mask
**                    : IN/--- void *unlock_codes
**                    : OUT/--- uint32_t *post_lock_level_status
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** *************************************************************************/
uint32_t sec_hal_rt_simlock_levels_open(
		uint32_t user_levels_mask_in,
		void* user_unlock_codes_in,
		uint32_t* user_post_lock_level_status_out)
{
	uint32_t sec_hal_status = SEC_HAL_RES_OK;
	uint32_t sec_msg_status = SEC_MSG_STATUS_OK;
	uint32_t ssa_disp_status = 0x00;
	uint32_t sec_serv_status = 0x00;
	uint16_t siml_levels;
	uint16_t msg_data_size;
	char* string_ptr = NULL;
	sec_msg_t *in_msg = NULL;
	sec_msg_t *out_msg = NULL;
	sec_msg_handle_t in_handle;
	sec_msg_handle_t out_handle;

	SEC_HAL_TRACE_ENTRY();

	if (0 == user_levels_mask_in)
	{
		SEC_HAL_TRACE_EXIT_INFO("!!no levels to-be-unlocked, aborting!!");
		return SEC_HAL_RES_PARAM_ERROR;
	}
	if (NULL == user_unlock_codes_in)
	{
		SEC_HAL_TRACE_EXIT_INFO("!!null user_unlock_codes_in ptr, aborting!!");
		return SEC_HAL_RES_PARAM_ERROR;
	}
	if (NULL == user_post_lock_level_status_out)
	{
		SEC_HAL_TRACE_EXIT_INFO("!!null user_post_lock_level_status_out ptr, aborting!!");
		return SEC_HAL_RES_PARAM_ERROR;
	}

	/* allocate memory, from ICRAM, for msgs to be sent to TZ */
	msg_data_size = sec_msg_param_size(SEC_HAL_MAX_SIMLOCK_CODE_LENGTH)*LOCAL_DEFAULT_SIMLOCK_LEVEL_COUNT;
	in_msg = sec_msg_alloc(
				&in_handle,
				msg_data_size,
				SEC_MSG_OBJECT_ID_NONE,
				0,
				SEC_HAL_MSG_BYTE_ORDER);
	msg_data_size = sec_msg_param_size(sizeof(sec_serv_status_t)) +
				sec_msg_param_size(sizeof(uint32_t));
	out_msg = sec_msg_alloc(
				&out_handle,
				msg_data_size,
				SEC_MSG_OBJECT_ID_NONE,
				0,
				SEC_HAL_MSG_BYTE_ORDER);

	if (NULL == in_msg || NULL == out_msg)
	{
		SEC_HAL_TRACE("alloc failure, msg not sent!");
		sec_hal_status = SEC_HAL_RES_FAIL;
	}
	else
	{
		/* write content to the input msg */
		siml_levels = 0;
		msg_data_size = 0;
		string_ptr = user_unlock_codes_in;
		while (siml_levels < LOCAL_DEFAULT_SIMLOCK_LEVEL_COUNT &&
				SEC_MSG_STATUS_OK == sec_msg_status)
		{
			string_ptr = string_ptr + msg_data_size;
			msg_data_size = strlen(string_ptr) + 1;
			if (msg_data_size > SEC_HAL_MAX_SIMLOCK_CODE_LENGTH)
			{
				sec_msg_status = SEC_MSG_STATUS_PARAM_OUT_OF_RANGE;
			}
			else if ((user_levels_mask_in>>siml_levels)&0x1)
			{
				sec_msg_status = sec_msg_param_write(
									&in_handle,
									string_ptr,
									msg_data_size,
									SEC_MSG_PARAM_ID_NONE);
			}
			else
			{
				sec_msg_status = sec_msg_param_write(
									&in_handle,
									k_sec_hal_simlock_empty_code,
									strlen(k_sec_hal_simlock_empty_code)+1,
									SEC_MSG_PARAM_ID_NONE);
			}
			siml_levels++;
		}

		if (SEC_MSG_STATUS_OK == sec_msg_status)
		{
			/* call dispatcher */
			ssa_disp_status = LOCAL_DISP(
								SEC_SERV_SIMLOCK_CHECK_LOCKS,
								LOCAL_DEFAULT_DISP_FLAGS,
								LOCAL_DEFAULT_DISP_SPARE_PARAM,
								SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
								SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));

			/* interpret the response */
			sec_msg_status = sec_msg_param_read32(&out_handle, &sec_serv_status);
		}

		if (SEC_MSG_STATUS_PARAM_OUT_OF_RANGE == sec_msg_status)
		{
			SEC_HAL_TRACE("param error, SIMLOCK post_status not given!");
			sec_hal_status = SEC_HAL_RES_PARAM_ERROR;
		}
		else if (SEC_ROM_RET_OK != ssa_disp_status ||
				SEC_MSG_STATUS_OK != sec_msg_status ||
				SEC_SERV_STATUS_OK != sec_serv_status)
		{
			SEC_HAL_TRACE("op failed, SIMLOCK post_status not given!");
			SEC_HAL_TRACE_INT("ssa_disp_status", ssa_disp_status);
			SEC_HAL_TRACE_INT("sec_msg_status ", sec_msg_status);
			SEC_HAL_TRACE_INT("sec_serv_status", sec_serv_status);
			sec_hal_status = SEC_HAL_RES_FAIL;
		}
		else
		{
			sec_msg_status = _sec_msg_param_read(
								LOCAL_CPY_TO_CLIENT_FUNCPTR,
								&out_handle,
								user_post_lock_level_status_out,
								sizeof(uint32_t));
			if (SEC_MSG_STATUS_OK != sec_msg_status)
			{
				SEC_HAL_TRACE_INT("read failed! sec_msg_status", sec_msg_status);
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
** Function name      : sec_hal_rt_simlock_level_open
** Description        : opens specific simlock level with the given code.
** Parameters         : IN/--- char *unlock_code
**                    : IN/--- uint8_t lock_level
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** *************************************************************************/
uint32_t sec_hal_rt_simlock_level_open(
		char *user_unlock_code_in,
		uint8_t user_lock_level_in)
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

	if (NULL == user_unlock_code_in)
	{
		SEC_HAL_TRACE_EXIT_INFO("!!no user_unlock_code_in, aborting!!");
		return SEC_HAL_RES_PARAM_ERROR;
	}
	if (0 == user_lock_level_in)
	{
		SEC_HAL_TRACE_EXIT_INFO("!!no user_lock_level_in, aborting!!");
		return SEC_HAL_RES_PARAM_ERROR;
	}

	/* allocate memory, from ICRAM, for msgs to be sent to TZ */
	msg_data_size = sec_msg_param_size(sizeof(uint8_t)) +
				sec_msg_param_size(SEC_HAL_MAX_SIMLOCK_CODE_LENGTH);
	in_msg = sec_msg_alloc(
				&in_handle,
				msg_data_size,
				SEC_MSG_OBJECT_ID_NONE,
				0,
				SEC_HAL_MSG_BYTE_ORDER);
	msg_data_size = sec_msg_param_size(sizeof(sec_serv_status_t));
	out_msg = sec_msg_alloc(
				&out_handle,
				msg_data_size,
				SEC_MSG_OBJECT_ID_NONE,
				0,
				SEC_HAL_MSG_BYTE_ORDER);

	if (NULL == in_msg || NULL == out_msg)
	{
		SEC_HAL_TRACE("alloc failure, msg not sent!");
		sec_hal_status = SEC_HAL_RES_FAIL;
	}
	else
	{
		/* write content to the input msg */
		sec_msg_param_write8(
				&in_handle,
				user_lock_level_in,
				SEC_MSG_PARAM_ID_NONE);
		sec_msg_param_write(
				&in_handle,
				user_unlock_code_in,
				SEC_HAL_MAX_SIMLOCK_CODE_LENGTH,
				SEC_MSG_PARAM_ID_NONE);

		/* call dispatcher */
		ssa_disp_status = LOCAL_DISP(
							SEC_SERV_SIMLOCK_CHECK_ONE_LOCK,
							LOCAL_DEFAULT_DISP_FLAGS,
							LOCAL_DEFAULT_DISP_SPARE_PARAM,
							SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
							SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));

		/* interpret the response */
		sec_msg_status = sec_msg_param_read32(&out_handle, &sec_serv_status);
		if (SEC_ROM_RET_OK != ssa_disp_status ||
			SEC_MSG_STATUS_OK != sec_msg_status ||
			SEC_SERV_STATUS_OK != sec_serv_status)
		{
			SEC_HAL_TRACE("op failed, SIMLOCK open failed!");
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
** Function name      : sec_hal_rt_simlock_level_status_get
** Description        : retrieves device simlock levels state from TZ.
** Parameters         : OUT/--- uint32_t *lock_level_status
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** *************************************************************************/
uint32_t sec_hal_rt_simlock_level_status_get(
		uint32_t *user_lock_level_status_out)
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

	if (NULL == user_lock_level_status_out)
	{
		SEC_HAL_TRACE_EXIT_INFO("!!null user_lock_level_status_out, aborting!!");
		return SEC_HAL_RES_PARAM_ERROR;
	}

	/* allocate memory, from ICRAM, for msgs to be sent to TZ */
	msg_data_size = sec_msg_param_size(sizeof(uint32_t));
	in_msg = sec_msg_alloc(
				&in_handle,msg_data_size,
				SEC_MSG_OBJECT_ID_NONE,
				0,
				SEC_HAL_MSG_BYTE_ORDER);
	msg_data_size = sec_msg_param_size(sizeof(sec_serv_status_t)) +
				sec_msg_param_size(sizeof(uint32_t));
	out_msg = sec_msg_alloc(
				&out_handle,
				msg_data_size,
				SEC_MSG_OBJECT_ID_NONE,
				0,
				SEC_HAL_MSG_BYTE_ORDER);

	if (NULL == in_msg || NULL == out_msg)
	{
		SEC_HAL_TRACE("alloc failure, msg not sent!");
		sec_hal_status = SEC_HAL_RES_FAIL;
	}
	else
	{
		/* write content to the input msg */
		sec_msg_param_write32(
				&in_handle,
				sizeof(uint32_t),
				SEC_MSG_PARAM_ID_NONE);

		/* call dispatcher */
		ssa_disp_status = LOCAL_DISP(
							SEC_SERV_SIMLOCK_GET_STATE,
							LOCAL_DEFAULT_DISP_FLAGS,
							LOCAL_DEFAULT_DISP_SPARE_PARAM,
							SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
							SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));

		/* interpret the response */
		sec_msg_status = sec_msg_param_read32(&out_handle, &sec_serv_status);
		if (SEC_ROM_RET_OK != ssa_disp_status ||
			SEC_MSG_STATUS_OK != sec_msg_status ||
			SEC_SERV_STATUS_OK != sec_serv_status)
		{
			SEC_HAL_TRACE("op failed, SIMLOCK level status not given!");
			SEC_HAL_TRACE_INT("ssa_disp_status", ssa_disp_status);
			SEC_HAL_TRACE_INT("sec_msg_status ", sec_msg_status);
			SEC_HAL_TRACE_INT("sec_serv_status", sec_serv_status);
			sec_hal_status = SEC_HAL_RES_FAIL;
		}
		else
		{
			sec_msg_status = _sec_msg_param_read(
								LOCAL_CPY_TO_CLIENT_FUNCPTR,
								&out_handle,
								user_lock_level_status_out,
								sizeof(uint32_t));
			if (SEC_MSG_STATUS_OK != sec_msg_status)
			{
				SEC_HAL_TRACE_INT("read failed! sec_msg_status", sec_msg_status);
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
** Function name      : sec_hal_rt_auth_data_size_get
** Description        : retrieves device authentication data size from TZ.
** Parameters         : IN/--- uint32_t input_data_size
**                    : IN/--- uint32_t *input_data
**                    : OUT/--- uint32_t *auth_data_size
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** *************************************************************************/
uint32_t sec_hal_rt_auth_data_size_get(
		void* user_input_data_in,
		uint32_t user_input_data_size_in,
		uint32_t* user_auth_data_size_out)
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

	if (NULL == user_input_data_in)
	{
		SEC_HAL_TRACE_EXIT_INFO("!!null user_input_data_in, aborting!!");
		return SEC_HAL_RES_PARAM_ERROR;
	}
	if (0 == user_input_data_size_in)
	{
		SEC_HAL_TRACE_EXIT_INFO("!!zero user_input_data_size_in, aborting!!");
		return SEC_HAL_RES_PARAM_ERROR;
	}
	if (NULL == user_auth_data_size_out)
	{
		SEC_HAL_TRACE_EXIT_INFO("!!null user_auth_data_size_out, aborting!!");
		return SEC_HAL_RES_PARAM_ERROR;
	}

	/* allocate memory, from ICRAM, for msgs to be sent to TZ */
	msg_data_size = sec_msg_param_size(sizeof(uint32_t)) +
				sec_msg_param_size(user_input_data_size_in);
	in_msg = sec_msg_alloc(
				&in_handle,
				msg_data_size,
				SEC_MSG_OBJECT_ID_NONE,
				0,
				SEC_HAL_MSG_BYTE_ORDER);
	msg_data_size = sec_msg_param_size(sizeof(sec_serv_status_t)) +
				sec_msg_param_size(sizeof(uint32_t));
	out_msg = sec_msg_alloc(
				&out_handle,
				msg_data_size,
				SEC_MSG_OBJECT_ID_NONE,
				0,
				SEC_HAL_MSG_BYTE_ORDER);

	if (NULL == in_msg || NULL == out_msg)
	{
		SEC_HAL_TRACE("alloc failure, msg not sent!");
		sec_hal_status = SEC_HAL_RES_FAIL;
	}
	else
	{
		/* write content to the input msg */
		sec_msg_param_write32(
				&in_handle,
				user_input_data_size_in,
				SEC_MSG_PARAM_ID_NONE);
		sec_msg_param_write(
				&in_handle,
				user_input_data_in,
				user_input_data_size_in,
				SEC_MSG_PARAM_ID_NONE);

		/* call dispatcher */
		ssa_disp_status = LOCAL_DISP(
							SEC_SERV_DEVICE_AUTH_DATA_SIZE_REQUEST,
							LOCAL_DEFAULT_DISP_FLAGS,
							LOCAL_DEFAULT_DISP_SPARE_PARAM,
							SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
							SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));

		/* interpret the response */
		sec_msg_status = sec_msg_param_read32(&out_handle, &sec_serv_status);
		if (SEC_ROM_RET_OK != ssa_disp_status ||
			SEC_MSG_STATUS_OK != sec_msg_status ||
			SEC_SERV_STATUS_OK != sec_serv_status)
		{
			SEC_HAL_TRACE("op failed, AUTH DATA SIZE not given!");
			SEC_HAL_TRACE_INT("ssa_disp_status", ssa_disp_status);
			SEC_HAL_TRACE_INT("sec_msg_status ", sec_msg_status);
			SEC_HAL_TRACE_INT("sec_serv_status", sec_serv_status);
			sec_hal_status = SEC_HAL_RES_FAIL;
		}
		else
		{
			sec_msg_status = _sec_msg_param_read(
								LOCAL_CPY_TO_CLIENT_FUNCPTR,
								&out_handle,
								user_auth_data_size_out,
								sizeof(uint32_t));
			if (SEC_MSG_STATUS_OK != sec_msg_status)
			{
				SEC_HAL_TRACE_INT("read failed! sec_msg_status", sec_msg_status);
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
** Function name      : sec_hal_rt_auth_data_get
** Description        : retrieves device authentication data from TZ side.
** Parameters         : IN/--- uint32_t input_data_size
**                    : IN/--- uint32_t *input_data
**                    : IN/--- uint32_t auth_data_size
**                    : OUT/--- uint32_t *auth_data
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** *************************************************************************/
uint32_t sec_hal_rt_auth_data_get(
		void *user_input_data_in,
		uint32_t user_input_data_size_in,
		void *user_auth_data_out,
		uint32_t user_auth_data_size_in)
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
	void *kernel_input_data_in;

	SEC_HAL_TRACE_ENTRY();

	if (NULL == user_input_data_in)
	{
		SEC_HAL_TRACE_EXIT_INFO("!!zero user_input_data_in, aborting!!");
		return SEC_HAL_RES_PARAM_ERROR;
	}
	if (0 == user_input_data_size_in)
	{
		SEC_HAL_TRACE_EXIT_INFO("!!zero user_input_data_size_in, aborting!!");
		return SEC_HAL_RES_PARAM_ERROR;
	}
	if (NULL == user_auth_data_out)
	{
		SEC_HAL_TRACE_EXIT_INFO("!!null user_auth_data_out, aborting!!");
		return SEC_HAL_RES_PARAM_ERROR;
	}
	if (0 == user_auth_data_size_in)
	{
		SEC_HAL_TRACE_EXIT_INFO("!!zero user_auth_data_size_in, aborting!!");
		return SEC_HAL_RES_PARAM_ERROR;
	}
	SEC_HAL_TRACE("user_input_data_in: 0x%x", user_input_data_in);
	SEC_HAL_TRACE("user_input_data_size_in: %d", user_input_data_size_in);

    kernel_input_data_in = kmalloc(user_input_data_size_in, GFP_KERNEL);
    copy_from_user( kernel_input_data_in, user_input_data_in,
                    user_input_data_size_in );
	SEC_HAL_TRACE("kernel_input_data_in: 0x%x", kernel_input_data_in);

	/* allocate memory, from ICRAM, for msgs to be sent to TZ */
	msg_data_size = sec_msg_param_size(sizeof(uint32_t))*2 +
				sec_msg_param_size(user_input_data_size_in);
	SEC_HAL_TRACE("msg_data_size: %d", msg_data_size);
	in_msg = sec_msg_alloc(
				&in_handle,
				msg_data_size,
				SEC_MSG_OBJECT_ID_NONE,
				0,
				SEC_HAL_MSG_BYTE_ORDER);
	SEC_HAL_TRACE("in_msg: %d", in_msg);

	msg_data_size = sec_msg_param_size(sizeof(sec_serv_status_t)) +
				sec_msg_param_size(user_auth_data_size_in);
	out_msg = sec_msg_alloc(
				&out_handle,
				msg_data_size,
				SEC_MSG_OBJECT_ID_NONE,
				0,
				SEC_HAL_MSG_BYTE_ORDER);

	if (NULL == in_msg || NULL == out_msg)
	{
		SEC_HAL_TRACE("alloc failure, msgs not sent!");
		sec_hal_status = SEC_HAL_RES_FAIL;
	}
	else
	{
		/* write content to the input msg */
		sec_msg_param_write32(
				&in_handle,
				user_input_data_size_in,
				SEC_MSG_PARAM_ID_NONE);
		sec_msg_param_write(
				&in_handle,
				kernel_input_data_in,
				user_input_data_size_in,
				SEC_MSG_PARAM_ID_NONE);
		sec_msg_param_write32(
				&in_handle,
				user_auth_data_size_in,
				SEC_MSG_PARAM_ID_NONE);

		/* call dispatcher */
		ssa_disp_status = LOCAL_DISP(
							SEC_SERV_DEVICE_AUTH_DATA_REQUEST,
							LOCAL_DEFAULT_DISP_FLAGS,
							LOCAL_DEFAULT_DISP_SPARE_PARAM,
							SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
							SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));

		/* interpret the response */
		sec_msg_status = sec_msg_param_read32(&out_handle, &sec_serv_status);
		if (SEC_ROM_RET_OK != ssa_disp_status ||
			SEC_MSG_STATUS_OK != sec_msg_status ||
			SEC_SERV_STATUS_OK != sec_serv_status)
		{
			SEC_HAL_TRACE("op failed, AUTH DATA not available!");
			SEC_HAL_TRACE_INT("ssa_disp_status", ssa_disp_status);
			SEC_HAL_TRACE_INT("sec_msg_status ", sec_msg_status);
			SEC_HAL_TRACE_INT("sec_serv_status", sec_serv_status);
			sec_hal_status = SEC_HAL_RES_FAIL;
		}
		else
		{
			sec_msg_status = _sec_msg_param_read(
								LOCAL_CPY_TO_CLIENT_FUNCPTR,
								&out_handle,
								user_auth_data_out,
								user_auth_data_size_in);
			if (SEC_MSG_STATUS_OK != sec_msg_status)
			{
				SEC_HAL_TRACE_INT("read failed! sec_msg_status", sec_msg_status);
				sec_hal_status = SEC_HAL_RES_FAIL;
			}
		}
	}

	/* de-allocate msgs */
	sec_msg_free(out_msg);
	sec_msg_free(in_msg);
    kfree(kernel_input_data_in);

	SEC_HAL_TRACE_EXIT();
	return sec_hal_status;
}

/* **************************************************************************
** Function name      : sec_hal_rt_periodic_integrity_check
** Description        : makes a periodic check of SW cert protected files.
** Parameters         : OUT/--- uint32_t *sec_exp_time
**                      timeout for next period
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** *************************************************************************/
uint32_t sec_hal_rt_periodic_integrity_check(uint32_t *sec_exp_time)
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

	if (NULL == sec_exp_time)
	{
		SEC_HAL_TRACE_EXIT_INFO("!!null sec_exp_time, aborting!!");
		return SEC_HAL_RES_PARAM_ERROR;
	}

	/* allocate memory, from ICRAM, for msgs to be sent to TZ */
	msg_data_size = sec_msg_param_size(sizeof(uint32_t));
	in_msg = sec_msg_alloc(
				&in_handle,
				msg_data_size,
				SEC_MSG_OBJECT_ID_NONE,
				0,
				SEC_HAL_MSG_BYTE_ORDER);
	msg_data_size = sec_msg_param_size(sizeof(sec_serv_status_t)) +
				sec_msg_param_size(sizeof(uint32_t));
	out_msg = sec_msg_alloc(
				&out_handle,
				msg_data_size,
				SEC_MSG_OBJECT_ID_NONE,
				0,
				SEC_HAL_MSG_BYTE_ORDER);

	if (NULL == in_msg || NULL == out_msg)
	{
		SEC_HAL_TRACE("alloc failure, msgs not sent!");
		sec_hal_status = SEC_HAL_RES_FAIL;
	}
	else
	{
		/* write content to the input msg */
		sec_msg_param_write32(
				&in_handle,
				sizeof(uint32_t),
				SEC_MSG_PARAM_ID_NONE);

		/* call dispatcher */
		ssa_disp_status = LOCAL_DISP(
							SEC_SERV_INTEGRITY_CHECK,
							LOCAL_DEFAULT_DISP_FLAGS,
							LOCAL_DEFAULT_DISP_SPARE_PARAM,
							SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
							SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));

		/* interpret the response */
		sec_msg_status = sec_msg_param_read32(&out_handle, &sec_serv_status);
		if (SEC_ROM_RET_OK != ssa_disp_status ||
			SEC_MSG_STATUS_OK != sec_msg_status ||
			SEC_SERV_STATUS_OK != sec_serv_status)
		{
			SEC_HAL_TRACE("op failed, INTEGRITY not confirmed!");
			SEC_HAL_TRACE_INT("ssa_disp_status", ssa_disp_status);
			SEC_HAL_TRACE_INT("sec_msg_status ", sec_msg_status);
			SEC_HAL_TRACE_INT("sec_serv_status", sec_serv_status);
			sec_hal_status = SEC_HAL_RES_FAIL;
		}
		else
		{
			sec_msg_status = _sec_msg_param_read(
								LOCAL_CPY_TO_CLIENT_FUNCPTR,
								&out_handle,
								sec_exp_time,
								sizeof(uint32_t));
			if (SEC_MSG_STATUS_OK != sec_msg_status)
			{
				SEC_HAL_TRACE_INT("read failed! sec_msg_status", sec_msg_status);
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
** Function name      : sec_hal_rt_selftest
** Description        : check if selftest was a success, or not.
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** *************************************************************************/
uint32_t sec_hal_rt_selftest(void)
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
	in_msg = sec_msg_alloc(
				&in_handle,
				msg_data_size,
				SEC_MSG_OBJECT_ID_NONE,
				0,
				SEC_HAL_MSG_BYTE_ORDER);
	msg_data_size = sec_msg_param_size(sizeof(sec_serv_status_t)) +
				sec_msg_param_size(sizeof(uint32_t));
	out_msg = sec_msg_alloc(
				&out_handle,
				msg_data_size,
				SEC_MSG_OBJECT_ID_NONE,
				0,
				SEC_HAL_MSG_BYTE_ORDER);

	if (NULL == in_msg || NULL == out_msg)
	{
		SEC_HAL_TRACE("alloc failure, msgs not sent!");
		sec_hal_status = SEC_HAL_RES_FAIL;
	}
	else
	{
		/* write content to the input msg */
		sec_msg_param_write32(
				&in_handle,
				sizeof(uint32_t),
				SEC_MSG_PARAM_ID_NONE);

		/* call dispatcher */
		ssa_disp_status = LOCAL_DISP(
							SEC_SERV_SELFTEST,
							LOCAL_DEFAULT_DISP_FLAGS,
							LOCAL_DEFAULT_DISP_SPARE_PARAM,
							SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
							SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));

		/* interpret the response */
		sec_msg_status = sec_msg_param_read32(&out_handle, &sec_serv_status);
		if (SEC_ROM_RET_OK != ssa_disp_status ||
			SEC_MSG_STATUS_OK != sec_msg_status ||
			SEC_SERV_STATUS_OK != sec_serv_status)
		{
			SEC_HAL_TRACE("SELFTEST failed!");
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
** Function name      : sec_hal_rt_public_cc42_key_init
** Description        : transmit public cc42 key init event to TZ.
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** *************************************************************************/
uint32_t sec_hal_rt_public_cc42_key_init(void)
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
	in_msg = sec_msg_alloc(
				&in_handle,
				msg_data_size,
				SEC_MSG_OBJECT_ID_NONE,
				0,
				SEC_HAL_MSG_BYTE_ORDER);
	msg_data_size = sec_msg_param_size(sizeof(sec_serv_status_t)) +
				sec_msg_param_size(sizeof(uint32_t));
	out_msg = sec_msg_alloc(
				&out_handle,
				msg_data_size,
				SEC_MSG_OBJECT_ID_NONE,
				0,
				SEC_HAL_MSG_BYTE_ORDER);

	if (NULL == in_msg || NULL == out_msg)
	{
		SEC_HAL_TRACE("alloc failure, msgs not sent!");
		sec_hal_status = SEC_HAL_RES_FAIL;
	}
	else
	{
		/* write content to the input msg */
		sec_msg_param_write32(
				&in_handle,
				sizeof(uint32_t),
				SEC_MSG_PARAM_ID_NONE);

		/* call dispatcher */
		ssa_disp_status = LOCAL_DISP(
							SEC_SERV_PUBLIC_CC42_KEY_INIT,
							LOCAL_DEFAULT_DISP_FLAGS,
							LOCAL_DEFAULT_DISP_SPARE_PARAM,
							SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
							SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));

		/* interpret the response */
		sec_msg_status = sec_msg_param_read32(&out_handle, &sec_serv_status);
		if (SEC_ROM_RET_OK != ssa_disp_status ||
			SEC_MSG_STATUS_OK != sec_msg_status ||
			SEC_SERV_STATUS_OK != sec_serv_status)
		{
			SEC_HAL_TRACE("CC42 KEY INIT failed!");
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
** Function name      : sec_hal_rt_a3sp_state_info
** Description        : transmit&receives public cc42 key init event to TZ.
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** *************************************************************************/
uint32_t sec_hal_rt_a3sp_state_info(uint32_t request_in, uint32_t *virt_allowed_out)
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
	in_msg = sec_msg_alloc(
				&in_handle,
				msg_data_size,
				SEC_MSG_OBJECT_ID_NONE,
				0,
				SEC_HAL_MSG_BYTE_ORDER);
	msg_data_size = sec_msg_param_size(sizeof(sec_serv_status_t)) +
				sec_msg_param_size(sizeof(uint32_t));
	out_msg = sec_msg_alloc(
				&out_handle,
				msg_data_size,
				SEC_MSG_OBJECT_ID_NONE,
				0,
				SEC_HAL_MSG_BYTE_ORDER);

	if (NULL == in_msg || NULL == out_msg)
	{
		SEC_HAL_TRACE("alloc failure, msgs not sent!");
		sec_hal_status = SEC_HAL_RES_FAIL;
	}
	else
	{
		/* write content to the input msg */
		sec_msg_param_write32(
				&in_handle,
				request_in,
				SEC_MSG_PARAM_ID_NONE);

		/* call dispatcher */
		ssa_disp_status = LOCAL_DISP(
							SEC_SERV_A3SP_STATE_INFO,
							LOCAL_DEFAULT_DISP_FLAGS,
							LOCAL_DEFAULT_DISP_SPARE_PARAM,
							SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
							SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));

		/* interpret the response */
		sec_msg_status = sec_msg_param_read32(&out_handle, &sec_serv_status);
		if (SEC_ROM_RET_OK != ssa_disp_status ||
			SEC_MSG_STATUS_OK != sec_msg_status ||
			SEC_SERV_STATUS_OK != sec_serv_status)
		{
			SEC_HAL_TRACE("A3SP STATE INFO failed!");
			SEC_HAL_TRACE_INT("ssa_disp_status", ssa_disp_status);
			SEC_HAL_TRACE_INT("sec_msg_status ", sec_msg_status);
			SEC_HAL_TRACE_INT("sec_serv_status", sec_serv_status);
			sec_hal_status = SEC_HAL_RES_FAIL;
		}
		else
		{
			sec_msg_status = sec_msg_param_read32(&out_handle, virt_allowed_out);
			if (SEC_MSG_STATUS_OK != sec_msg_status)
			{
				SEC_HAL_TRACE_INT("read failed! sec_msg_status", sec_msg_status);
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
** Function name      : sec_hal_rt_memcpy
** Description        : transmit memcpy request to TZ.
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** *************************************************************************/
uint32_t sec_hal_rt_memcpy(void* phys_dst_in, void* phys_src_in, uint32_t size_in)
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

	if (NULL == phys_src_in)
	{
		SEC_HAL_TRACE_EXIT_INFO("!!null phys_src, aborting!!");
		return SEC_HAL_RES_PARAM_ERROR;
	}

	/* allocate memory, from RAM, for msgs to be sent to TZ */
	msg_data_size = sec_msg_param_size(sizeof(uint32_t))*3;
	in_msg = sec_msg_alloc(
				&in_handle,
				msg_data_size,
				SEC_MSG_OBJECT_ID_NONE,
				0,
				SEC_HAL_MSG_BYTE_ORDER);
	msg_data_size = sec_msg_param_size(sizeof(sec_serv_status_t)) +
				sec_msg_param_size(sizeof(uint32_t));
	out_msg = sec_msg_alloc(
				&out_handle,
				msg_data_size,
				SEC_MSG_OBJECT_ID_NONE,
				0,
				SEC_HAL_MSG_BYTE_ORDER);

	if (NULL == in_msg || NULL == out_msg)
	{
		SEC_HAL_TRACE("alloc failure, msgs not sent!");
		sec_hal_status = SEC_HAL_RES_FAIL;
	}
	else
	{
		/* ensure that data is fully in work RAM memory */
		SEC_HAL_MEM_CACHE_CLEAN_FUNC(phys_src_in, size_in);

		/* write content to the input msg */
		sec_msg_param_write32(
				&in_handle,
				(uint32_t)phys_dst_in,
				SEC_MSG_PARAM_ID_NONE);
		sec_msg_param_write32(
				&in_handle,
				(uint32_t)phys_src_in,
				SEC_MSG_PARAM_ID_NONE);
		sec_msg_param_write32(
				&in_handle,
				size_in,
				SEC_MSG_PARAM_ID_NONE);

		/* call dispatcher */
		ssa_disp_status = LOCAL_DISP(
							SEC_SERV_MEMCPY_REQUEST,
							LOCAL_DEFAULT_DISP_FLAGS,
							LOCAL_DEFAULT_DISP_SPARE_PARAM,
							SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
							SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));

		/* interpret the response */
		sec_msg_status = sec_msg_param_read32(&out_handle, &sec_serv_status);
		if (SEC_ROM_RET_OK != ssa_disp_status ||
			SEC_MSG_STATUS_OK != sec_msg_status ||
			SEC_SERV_STATUS_OK != sec_serv_status)
		{
			SEC_HAL_TRACE("MEMCPY failed!");
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
** Function name      : sec_hal_rt_multicore_enable
** Description        : call SEC_SERV_MULTICORE_ENABLE from secure side
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** *************************************************************************/
uint32_t sec_hal_rt_multicore_enable(void)
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

    /* allocate memory, from RAM, for msgs to be sent to TZ */
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

    if (NULL == in_msg || NULL == out_msg)
    {
        SEC_HAL_TRACE("alloc failure, msgs not sent!");
        sec_hal_status = SEC_HAL_RES_FAIL;
    }
    else
    {
        /* write content to the input msg */
        sec_msg_param_write32(&in_handle,
                              sizeof(uint32_t),
                              SEC_MSG_PARAM_ID_NONE);

        /* call dispatcher */
        ssa_disp_status = LOCAL_DISP(
                                SEC_SERV_MULTICORE_ENABLE,
                                LOCAL_DEFAULT_DISP_FLAGS,
                                LOCAL_DEFAULT_DISP_SPARE_PARAM,
                                SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
                                SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));

        /* interpret the response */
        sec_msg_status = sec_msg_param_read32(&out_handle, &sec_serv_status);
        if (SEC_ROM_RET_OK != ssa_disp_status ||
            SEC_MSG_STATUS_OK != sec_msg_status ||
            SEC_SERV_STATUS_OK != sec_serv_status)
        {
            SEC_HAL_TRACE("multicore enable failed!");
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

#ifdef SEC_STORAGE_DS_TEST_ENABLE
/* TEMPORARY, for testing purposes, send SEC_SERV_SIMU_DS0/1_TEST messages to Demo (DS) */

/* **************************************************************************
** Function name      : sec_hal_rt_simu_ds_test
** Description        : send SEC_SERV_SIMU_DS0/1_TEST messages to Demo (DS).
** Parameters         : IN/--- DS0 or DS1 / w or wo params, zero, one
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** *************************************************************************/
uint32_t sec_hal_rt_simu_ds_test(uint32_t dsnum, uint32_t param, uint32_t zero, uint32_t one)
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
    msg_data_size = sec_msg_param_size(sizeof(uint32_t)) +
                    sec_msg_param_size(sizeof(uint32_t));

    in_msg = sec_msg_alloc(&in_handle, msg_data_size,
            SEC_MSG_OBJECT_ID_NONE, 0, SEC_HAL_MSG_BYTE_ORDER);

    /* assuming that Demo only returns status after test sequence */
    msg_data_size = sec_msg_param_size(sizeof(sec_serv_status_t));

    out_msg = sec_msg_alloc(&out_handle, msg_data_size,
            SEC_MSG_OBJECT_ID_NONE, 0, SEC_HAL_MSG_BYTE_ORDER);

    if (NULL == in_msg || NULL == out_msg)
    {
        SEC_HAL_TRACE("Alloc failure, msg not sent!");
        sec_hal_status = SEC_HAL_RES_FAIL;
    }
    else
    {
        if (param == 1) { /* SEND DS0/1 TEST msg with parameters 'zero' and 'one' */
            SEC_HAL_TRACE("Parameters: (%d, %d)", zero, one);
            /* write content to the input msg */
            sec_msg_param_write32(&in_handle, zero, SEC_MSG_PARAM_ID_NONE);
            sec_msg_param_write32(&in_handle, one, SEC_MSG_PARAM_ID_NONE);
        }
        else {
            SEC_HAL_TRACE("No parameters. (%d)", zero);
        }

        if (dsnum == 0) {
            SEC_HAL_TRACE("Calling dispatcher - msg: 0x%x", SEC_SERV_SIMU_DS0_TEST);
            /* call dispatcher */
            ssa_disp_status = LOCAL_DISP(SEC_SERV_SIMU_DS0_TEST,
                    LOCAL_DEFAULT_DISP_FLAGS,
                    LOCAL_DEFAULT_DISP_SPARE_PARAM,
                    SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
                    SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));
        }
        else {
            SEC_HAL_TRACE("Calling dispatcher - msg: 0x%x", SEC_SERV_SIMU_DS1_TEST);
            /* call dispatcher */
            ssa_disp_status = LOCAL_DISP(
                    SEC_SERV_SIMU_DS1_TEST,
                    LOCAL_DEFAULT_DISP_FLAGS,
                    LOCAL_DEFAULT_DISP_SPARE_PARAM,
                    SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
                    SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));
        }

        /* interpret the response */
        sec_msg_status = sec_msg_param_read32(&out_handle, &sec_serv_status);
        if (SEC_ROM_RET_OK != ssa_disp_status ||
                SEC_MSG_STATUS_OK != sec_msg_status ||
                SEC_SERV_STATUS_OK != sec_serv_status) {
            SEC_HAL_TRACE("Fail, D-E-M-O !");
            SEC_HAL_TRACE_INT("ssa_disp_status", ssa_disp_status);
            SEC_HAL_TRACE_INT("sec_msg_status ", sec_msg_status);
            SEC_HAL_TRACE_INT("sec_serv_status", sec_serv_status);
            sec_hal_status = SEC_HAL_RES_FAIL;
            sec_hal_status |= ssa_disp_status <<  8;
            sec_hal_status |= sec_msg_status  << 16;
            sec_hal_status |= sec_serv_status << 24;
        }
        else {
            /* do nothing */
            SEC_HAL_TRACE("DEMO returned STATUS_OK!");
        }
    }
    /* de-allocate msg */
    sec_msg_free(out_msg);
    sec_msg_free(in_msg);

    SEC_HAL_TRACE_EXIT();
    return sec_hal_status;
}
#endif

#ifdef SEC_STORAGE_SELFTEST_ENABLE
/* **************************************************************************
** Function name      : sec_hal_rt_sec_storage_selftest
**
** Description        : Test secure storage. Emulates security side by
**                      writing messages according to security message API
**                      into ICRAM and calling directly rpc_handler.
**
**                      NOTE: THIS TEST CODE ASSUMES THAT SECURE STORAGE
**                            RINGBUFFER SIZE IS 8.
**
** Parameters         : rpc_func_ptr (pointer to rpc function)
**                      testcase (0-14, see explanations below)
**
**                      tc0:  Run all test cases
**
**                      Normal operation:
**                      tc1:  Read file1 - if not found, Create and Write file1
**                            -> File1 in Sec Storage and RBUF. Handle to it exists.
**                      tc2:  Repeat step1 for file2..file8
**                            -> Files1..8 in Sec Storage. RBUF is full. Handles 1..8 exist and valid.
**                      tc3:  Repeat step1 for file9..file11
**                            -> Files 1&2&3 lost from RBUF. Files are still in Sec Storage. Handles 1..3 no more valid.
**                      tc4:  Repeat step1 for file2
**                            -> File2 added to RBUF again, new handle to it is got.
**                      tc5:  Read all files (file1..file11) and compare to initial contents.
**                            -> All files Read from Sec Storage should be equal to write buffer contents.
**                      tc6:  Size of file3 and file10
**                            -> Size returned should be equal to original filesize (in w_buff)
**
**                      Unsupported RPC's:
**                      tc7:  Root & Move & Remove RPC calls
**                            -> NOT_SUPPORTED returned
**
**                      Error cases:
**                      tc8:  Read/Write/Size w OLD handle
**                            -> Error code returned (STALE)
**                      tc9:  Read/Write/Size w NULL handle and handle > last_handle
**                            -> Error code returned (STALE)
**                      tc10: Read/Size for a file which is in RBUF, but has been wiped off from Sec Storage
**                            -> Error code returned (NOT FOUND)
**                      tc11: Read/Write with length > 3kB
**                            -> Error code returned
**                      tc12: Read for a file, read length is set too large (larger than file)
**                            -> Error code returned (EOF)
**                      tc13: Lookup/Create with illegal filename(s)
**                            -> Error code returned (INVALID_NAME)
**                      tc14: File Write while no more space in Sec Storage
**                            -> Error code returned (NO_SPACE)
**
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** *************************************************************************/
uint32_t sec_hal_rt_sec_storage_selftest(sec_hal_rt_rpc_handler rpc_func_ptr, uint32_t testcase)
{
    static sec_sto_file_t *filestr[11];

    const char filenam[11][13] =  {"SFile.nb1",
                                   "SFile.nb2",
                                   "SFile.nb3",
                                   "SFile.nb4",
                                   "SFile.nb5",
                                   "SFile.nb6",
                                   "xFileS07.txt",
                                   "xFileS08.txt",
                                   "xFileS09.txt",
                                   "xFileS10.txt",
                                   "xFileS11.txt"};
    char *w_buff, *r_buff;
    char *very_long_fname, *large_file;
    static size_t filelen[11];
    static size_t readlen[11];
    uint32_t test_status[15];
    uint32_t tc, testvalue, roothandle;
    uint64_t tmphandle;
    static int init_done = 0;
    int i, x;

    SEC_HAL_TRACE_ENTRY();

    if (NULL == rpc_func_ptr) {
        SEC_HAL_TRACE_EXIT_INFO("!!null rpc_func_ptr, aborting!!");
        return SEC_HAL_RES_PARAM_ERROR;
    }

    /* ------------------- Initialize ------------------- */
    rpch = rpc_func_ptr;

    if (!init_done) {
        /* allocate mem buffers (these are never freed) */
        w_buff = kmalloc(11*99, GFP_KERNEL);
        r_buff = kmalloc(11*99, GFP_KERNEL);
        very_long_fname = kmalloc(256, GFP_KERNEL);
	large_file = kmalloc(3333, GFP_KERNEL);
        /* check if kmalloc failed */
        if (!w_buff || !r_buff || !very_long_fname || !large_file) {
            SEC_HAL_TRACE_EXIT_INFO("!!kmalloc failed, aborting!!");
            return SEC_HAL_RES_PARAM_ERROR;
        }
        /* initialize write buffers */
        strcpy(w_buff+(0*99),  "SecStorage File Number  1. TOP\n\0");
        strcpy(w_buff+(1*99),  "SecStorage File Number  2. SECRET\n\0");
        strcpy(w_buff+(2*99),  "SecStorage File Number  3. CODE\n\0");
        strcpy(w_buff+(3*99),  "SecStorage File Number  4. IS\n\0");
        strcpy(w_buff+(4*99),  "SecStorage File Number  5. HIDDEN\n\0");
        strcpy(w_buff+(5*99),  "SecStorage File Number  6. INSIDE\n\0");
        strcpy(w_buff+(6*99),  "SecStorage File Number  7. THESE\n\0");
        strcpy(w_buff+(7*99),  "SecStorage File Number  8. FILES\n\0");
        strcpy(w_buff+(8*99),  "SecStorage File Number  9. READY\n\0");
        strcpy(w_buff+(9*99),  "SecStorage File Number 10. STEADY\n\0");
        strcpy(w_buff+(10*99), "SecStorage File Number 11. GO\n\0");
        /* initialize read buffers and other tables */
	for (i = 0; i < 11; i++) {
            strcpy(r_buff+(i*99), "Empty slot\n\0");
            filelen[i] = strlen(w_buff+(i*99));
            readlen[i] = 0;
            filestr[i] = NULL;
            init_done = 1;
        }
        /* initialize rest of buffers */
        strcpy(very_long_fname, "there_is_1a_limit_h2ow_long_f3ilename_c4an_be_and5_this_is_6too_much_7\
                                 there_is_1a_limit_h2ow_long_f3ilename_c4an_be_and5_this_is_6too_much_7.txt");
        strcpy(large_file, "this is the beginning of a large file...");
    }

    for (i = 0; i < 15; i++) {
        test_status[i] = 0; /* STATUS OK */
    }
    /* ------------------- End of Initialize ------------------- */

    /* ------------------- Test cases ------------------- */

/*                      tc1:  Read file1 - if not found, Create and Write file1
**                            -> File1 in Sec Storage and RBUF. Handle to it exists.*/
    if (testcase == 0 || testcase == 1) {
        tc = 1;
        if ((filestr[0] = ssfopen(filenam[0], "r", 0))) {
            readlen[0] = ssfread(r_buff, 1, filelen[0], filestr[0]);
            if (readlen[0] != filelen[0]) test_status[tc] = 1; /* failed if read length does not match */
            ssfclose(filestr[0]);
        }
        else {
            if ((filestr[0] = ssfopen(filenam[0], "w", 0))) {
                ssfwrite((void *)w_buff, 1, filelen[0], filestr[0]);
                ssfclose(filestr[0]);
            }
            else {
                SEC_HAL_TRACE("Fail/TC1: Fileopen failed. File %d.", 0);
                SEC_HAL_TRACE_EXIT();
                return tc;
            }
        }

        /* check if tc failed */
        if (filestr[0]->file_handle != 1) test_status[tc] = 1;
        if (filestr[0]->error) test_status[tc] = 1;

        /* if failed return right away */
        if (test_status[tc]) {
            SEC_HAL_TRACE("Fail/TC1: File 1. Error %d. Handle %d.", filestr[0]->error, filestr[0]->file_handle);
            SEC_HAL_TRACE_EXIT();
            return tc;
        }
        SEC_HAL_TRACE("PASSED/TC1");
    }
/*                      tc2:  Repeat step1 for file2..file8
**                            -> Files1..8 in Sec Storage. RBUF is full. Handles 1..8 exist and valid.*/
    if (testcase == 0 || testcase == 2) {
        tc = 2;
        for (i = 1; i < 8; i++) {
            if ((filestr[i] = ssfopen(filenam[i], "r", i))) {
                readlen[i] = ssfread(r_buff+(i*99), 1, filelen[i], filestr[i]);
                if (readlen[i] != filelen[i]) test_status[tc] = 1;
                ssfclose(filestr[i]);
            }
            else {
                if ((filestr[i] = ssfopen(filenam[i], "w", i))) {
                    ssfwrite((void *)(w_buff+(i*99)), 1, filelen[i], filestr[i]);
                    ssfclose(filestr[i]);
                }
                else {
                    SEC_HAL_TRACE("Fail/TC2: Fileopen failed. File %d.", i);
                    SEC_HAL_TRACE_EXIT();
                    return tc;
                }
            }

            /* check if tc failed */
            if (filestr[i]->file_handle != (i+1)) test_status[tc] = 1;
            if (filestr[i]->error) test_status[tc] = 1;

            /* if failed return right away */
            if (test_status[tc]) {
                SEC_HAL_TRACE("Fail/TC2: File %d. Error %d. Handle %d.", i+1, filestr[i]->error, filestr[i]->file_handle);
                SEC_HAL_TRACE_EXIT();
                return tc;
            }
        }
        SEC_HAL_TRACE("PASSED/TC2");
    }
/*                      tc3:  Repeat step1 for file9..file11
**                            -> Files 1&2&3 lost from RBUF. Files are still in Sec Storage. Handles 1..3 no more valid.*/
    if (testcase == 0 || testcase == 3) {
        tc = 3;
        for (i = 8; i < 11; i++) {
            if ((filestr[i] = ssfopen(filenam[i], "r", i))) {
                readlen[i] = ssfread(r_buff+(i*99), 1, filelen[i], filestr[i]);
                if (readlen[i] != filelen[i]) test_status[tc] = 1;
                ssfclose(filestr[i]);
            }
            else {
                if ((filestr[i] = ssfopen(filenam[i], "w", i))) {
                    ssfwrite((void *)(w_buff+(i*99)), 1, filelen[i], filestr[i]);
                    ssfclose(filestr[i]);
                }
                else {
                    SEC_HAL_TRACE("Fail/TC3: Fileopen failed. File %d.", i);
                    SEC_HAL_TRACE_EXIT();
                    return tc;
                }
            }

            /* check if tc failed */
            if (filestr[i]->file_handle != (i+1)) test_status[tc] = 1;
            if (filestr[i]->error) test_status[tc] = 1;

            /* if failed return right away */
            if (test_status[tc]) {
                SEC_HAL_TRACE("Fail/TC3: File %d. Error %d. Handle %d.", i+1, filestr[i]->error, filestr[i]->file_handle);
                SEC_HAL_TRACE_EXIT();
                return tc;
            }
        }
        SEC_HAL_TRACE("PASSED/TC3");
    }
/*                      tc4:  Read file2
**                            -> File2 added to RBUF again, new handle to it is got.*/
    if (testcase == 0 || testcase == 4) {
        tc = 4;
        if ((filestr[1] = ssfopen(filenam[1], "r", 1))) {
            readlen[1] = ssfread(r_buff+(1*99), 1, filelen[1], filestr[1]);
            if (readlen[1] != filelen[1]) test_status[tc] = 1; /* failed if read length does not match */
            ssfclose(filestr[1]);
        }
        else {
            SEC_HAL_TRACE("Fail/TC4: Fileopen failed. File %d.", 2);
            SEC_HAL_TRACE_EXIT();
            return tc;
        }

        /* check if tc failed */
        if (filestr[1]->file_handle != 12) test_status[tc] = 1; /* Handle should be 12 now */
        if (filestr[1]->error) test_status[tc] = 1;

        /* if failed return right away */
        if (test_status[tc]) {
            SEC_HAL_TRACE("Fail/TC4: File 2. Error %d. Handle %d.", filestr[1]->error, filestr[1]->file_handle);
            SEC_HAL_TRACE_EXIT();
            return tc;
        }
        SEC_HAL_TRACE("PASSED/TC4");
    }
/*                      tc5:  Read all files (file1..file11) and compare to initial contents.
**                            -> All files Read from Sec Storage should be equal to write buffer contents.*/
    if (testcase == 0 || testcase == 5) {
        tc = 5;
        for (i = 10; i >= 0; i--) {
            if ((filestr[i] = ssfopen(filenam[i], "r", i))) {
                readlen[i] = ssfread(r_buff+(i*99), 1, filelen[i], filestr[i]);
                if (readlen[i] != filelen[i]) test_status[tc] = 1;
                ssfclose(filestr[i]);
            }
            else {
                SEC_HAL_TRACE("Fail/TC5: Fileopen failed. File %d.", i+1);
                SEC_HAL_TRACE_EXIT();
                return tc;
            }
        }
        /* check the results */
        for (i = 0; i < 11; i++) {
            int handles[11] = {15,12,14,13,5,6,7,8,9,10,11}; /* 5,6,7 are STALE, other handles are valid */
            /* check if tc failed */
            if (filestr[i]->file_handle != handles[i]) test_status[tc] = 1;
            if (filestr[i]->error) test_status[tc] = 1;
            for (x = 0; x < filelen[i] ; x++) {
                if (r_buff[i*99+x] != w_buff[i*99+x]) test_status[tc] = 1;
            }
            /* if failed return right away */
            if (test_status[tc]) {
                SEC_HAL_TRACE("Fail/TC5: File %d. Error %d. Handle %d.", i+1, filestr[i]->error, filestr[i]->file_handle);
                SEC_HAL_TRACE_EXIT();
                return tc;
            }
        }
        SEC_HAL_TRACE("PASSED/TC5");
    }
/*                      tc6:  Size of file3 and file10
**                            -> Size returned should be equal to original filesize (in w_buff) */
    if (testcase == 0 || testcase == 6) {
        tc = 6;

        if ((filestr[2] == 0) || (filestr[9] == 0)) { SEC_HAL_TRACE("Fail/TC6: Null pointer."); SEC_HAL_TRACE_EXIT(); return tc; }
        
        testvalue = ssfsize(filestr[2]);
        if (testvalue != filelen[2]) { test_status[tc] = 1; SEC_HAL_TRACE("Fail/TC6: File3 size %d != %d", testvalue, filelen[2]); }

        testvalue = ssfsize(filestr[9]);
        if (testvalue != filelen[9]) { test_status[tc] = 1; SEC_HAL_TRACE("Fail/TC6: File10 size %d != %d", testvalue, filelen[9]); }

        /* if failed return right away */
        if (test_status[tc]) {
            SEC_HAL_TRACE_EXIT();
            return tc;
        }
        SEC_HAL_TRACE("PASSED/TC6");
    }
    
/*                      tc7:  Root & Move & Remove RPC calls
**                            -> NOT_SUPPORTED returned*/
    if (testcase == 0 || testcase == 7) {
        tc = 7;

        testvalue = ssfgetroot(&roothandle); /* Should return NOT_SUPPORTED, Root is not supported */
        if (testvalue != SFS_STATUS_NOT_SUPPORTED) { test_status[tc] = 1; SEC_HAL_TRACE("Fail/TC7: Root returned %d != 3", testvalue); }

        testvalue = ssfmv(filenam[4], "newfile1.txt"); /* Should return NOT_SUPPORTED, Move is not supported */
        if (testvalue != SFS_STATUS_NOT_SUPPORTED) { test_status[tc] = 1; SEC_HAL_TRACE("Fail/TC7: Move returned %d != 3", testvalue); }

        testvalue = ssfrm(filenam[5]); /* Should return NOT_SUPPORTED, Remove is not supported */
        if (testvalue != SFS_STATUS_NOT_SUPPORTED) { test_status[tc] = 1; SEC_HAL_TRACE("Fail/TC7: Remove returned %d != 3", testvalue); }
	
        /* if failed return right away */
        if (test_status[tc]) {
            SEC_HAL_TRACE_EXIT();
            return tc;
        }
        SEC_HAL_TRACE("PASSED/TC7");
    }

/*                      tc8:  Read/Write/Size w OLD handle
**                            -> Error code returned (STALE)*/
    if (testcase == 0 || testcase == 8) {	
        tc = 8;

        if ((filestr[0] == 0)) { SEC_HAL_TRACE("Fail/TC8: Null pointer."); SEC_HAL_TRACE_EXIT(); return tc; }

        /* save original handle */
        tmphandle = filestr[0]->file_handle;

        /* with handle = 1 which is no longer valid */
        filestr[0]->file_handle = 1;

        /* these all should fail and return 0, error code should be STALE */
        testvalue = ssfread(r_buff, 1, filelen[0], filestr[0]);
        if (testvalue || (filestr[0]->error != SFS_STATUS_STALE)) test_status[tc] = 1;
        testvalue = ssfwrite((void *)w_buff, 1, filelen[0], filestr[0]);
        if (testvalue || (filestr[0]->error != SFS_STATUS_STALE)) test_status[tc] = 1;
        testvalue = ssfsize(filestr[0]);
        if (testvalue || (filestr[0]->error != SFS_STATUS_STALE)) test_status[tc] = 1;

        /* restore original handle */
        filestr[0]->file_handle = tmphandle;
        
        /* if failed return right away */
        if (test_status[tc]) {
            SEC_HAL_TRACE("Fail/TC8: Error code %d.", filestr[0]->error);
            SEC_HAL_TRACE_EXIT();
            return tc;
        }
        SEC_HAL_TRACE("PASSED/TC8");
    }

/*                      tc9:  Read/Write/Size w NULL handle and handle > last_handle
**                            -> Error code returned (STALE)*/
    if (testcase == 0 || testcase == 9) {
        tc = 9;

        if ((filestr[0] == 0)) { SEC_HAL_TRACE("Fail/TC9: Null pointer."); SEC_HAL_TRACE_EXIT(); return tc; }

        /* save original handle */
        tmphandle = filestr[0]->file_handle;

        /* first with handle = 0 */
        filestr[0]->file_handle = 0;

        /* these all should fail and return 0, error code should be STALE */
        testvalue = ssfread(r_buff, 1, filelen[0], filestr[0]);
        if (testvalue || (filestr[0]->error != SFS_STATUS_STALE)) test_status[tc] = 1;
        testvalue = ssfwrite((void *)w_buff, 1, filelen[0], filestr[0]);
        if (testvalue || (filestr[0]->error != SFS_STATUS_STALE)) test_status[tc] = 1;
        testvalue = ssfsize(filestr[0]);
        if (testvalue || (filestr[0]->error != SFS_STATUS_STALE)) test_status[tc] = 1;

        /* with handle > latest(=biggest) handle */
        filestr[0]->file_handle = 9999;

        /* these all should also fail and return 0, error code should be STALE */
        testvalue = ssfread(r_buff, 1, filelen[0], filestr[0]);
        if (testvalue || (filestr[0]->error != SFS_STATUS_STALE)) test_status[tc] = 1;
        testvalue = ssfwrite((void *)w_buff, 1, filelen[0], filestr[0]);
        if (testvalue || (filestr[0]->error != SFS_STATUS_STALE)) test_status[tc] = 1;
        testvalue = ssfsize(filestr[0]); /* Size with handle 9999 causes file9 to be renamed, this is needed for tc10 */
        if (testvalue || (filestr[0]->error != SFS_STATUS_STALE)) test_status[tc] = 1;

        /* restore original handle */
        filestr[0]->file_handle = tmphandle;
        
        /* if failed return right away */
        if (test_status[tc]) {
            SEC_HAL_TRACE("Fail/TC9: Error code %d.", filestr[0]->error);
            SEC_HAL_TRACE_EXIT();
            return tc;
        }
        SEC_HAL_TRACE("PASSED/TC9");
    }

/*                      tc10: Read/Size for a file which is in RBUF, but has been wiped off from Sec Storage
**                            -> Error code returned (NOT FOUND)*/
    if (testcase == 0 || testcase == 10) {
        tc = 10;
        
        /* File9 has been renamed at the end of tc9 */

        /* these all should fail and return 0, error code should be NOT_FOUND */
        testvalue = ssfread(r_buff+(8*99), 1, filelen[8], filestr[8]);
        if (testvalue || (filestr[8]->error != SFS_STATUS_NOT_FOUND)) {
            test_status[tc] = 1;
            SEC_HAL_TRACE("Fail/TC10(1): Error code %d. Length read %d. Test_status %d.", filestr[8]->error, testvalue, test_status[tc]);
        }
        testvalue = ssfsize(filestr[8]);
        if (testvalue || (filestr[8]->error != SFS_STATUS_NOT_FOUND)) {
            test_status[tc] = 1;
            SEC_HAL_TRACE("Fail/TC10(2): Error code %d. Size %d. Test_status %d.", filestr[8]->error, testvalue, test_status[tc]);
        }

        /* if failed return right away */
        if (test_status[tc]) {
            SEC_HAL_TRACE_EXIT();
            return tc;
        }
        SEC_HAL_TRACE("PASSED/TC10");
    }

/*                      tc11: Read/Write with length > 3kB
**                            -> Error code returned*/
    if (testcase == 0 || testcase == 11) {
        tc = 11;

        if ((filestr[10] = ssfopen("large_file_3333.abc", "w", 10))) {
            ssfwrite((void *)large_file, 1, 3333, filestr[10]);

            /* check if write failed */
            if (filestr[10]->error != SFS_STATUS_INVALID_CALL) { /* should return INVALID_CALL */
                test_status[tc] = 1;
                SEC_HAL_TRACE("Fail/TC11(1): Error code %d.", filestr[10]->error);
            }

            testvalue = ssfread(large_file, 1, 3333, filestr[10]);

            /* check if read failed */
            if (filestr[10]->error != SFS_STATUS_INVALID_CALL) { /* should return INVALID_CALL */
                test_status[tc] = 1;
                SEC_HAL_TRACE("Fail/TC11(2): Error code %d.", filestr[10]->error);
            }
        }
        else {
            test_status[tc] = 1;
            SEC_HAL_TRACE("TC11: File open failed.");
        }

        /* if failed return right away */
        if (test_status[tc]) {
            SEC_HAL_TRACE_EXIT();
            return tc;
        }

        SEC_HAL_TRACE("PASSED/TC11");
    }

/*                      tc12: Read for a file, read length is set too large (larger than file)
**                            -> Error code returned (EOF)*/
    if (testcase == 0 || testcase == 12) {
        tc = 12;

        if ((filestr[2] == 0)) { SEC_HAL_TRACE("Fail/TC12: Null pointer."); SEC_HAL_TRACE_EXIT(); return tc; }

        /* Try to read 10 bytes too much from file3 */
        testvalue = ssfread(r_buff+(2*99), 1, (filelen[2]+10), filestr[2]);
        /* check if failed */
        if (testvalue != filelen[2]) test_status[tc] = 1; /* should read full file eventhough length was set too big */
        /* check that first file was correctly read */
        if (filestr[2]->error != SFS_STATUS_END_OF_FILE) test_status[tc] = 1; /* should return EOF */
        for (x = 0; x < filelen[2] ; x++) { /* file should have been read correctly */
            if (r_buff[2*99+x] != w_buff[2*99+x]) test_status[tc] = 1;
        }
        if (test_status[tc]) {
            SEC_HAL_TRACE("Fail/TC12(1): Error code %d. Length read %d.", filestr[2]->error, testvalue);
        }

        /* try to read 15 bytes from empty file which was created in test case 11 */
        testvalue = ssfread(large_file, 1, 15, filestr[10]);
        /* check if failed */
        if (testvalue != 0) test_status[tc] = 1; /* should not read anything since empty file */
        /* check that correct status was returned from read operation */
        if (filestr[10]->error != SFS_STATUS_END_OF_FILE) { /* should return EOF */
            test_status[tc] = 1;
            SEC_HAL_TRACE("Fail/TC12(2): Error code %d. Length read %d.", filestr[10]->error, testvalue);
        }

        /* if failed return right away */
        if (test_status[tc]) {
            SEC_HAL_TRACE_EXIT();
            return tc;
        }
        SEC_HAL_TRACE("PASSED/TC12");
    }

/*                      tc13: Lookup/Create with illegal filename(s) 
**                            -> Error code returned (INVALID_NAME)*/
    if (testcase == 0 || testcase == 13) {
        tc = 13;

        /* filestr[10] is used but not rewritten, so null pointer check needs to be done only once */
        if ((filestr[10] == 0)) { SEC_HAL_TRACE("Fail/TC13: Null pointer."); SEC_HAL_TRACE_EXIT(); return tc; }

        /* too long filename */	
        ssfopen(very_long_fname, "w", 10);

        /* check status */
        if (filestr[10]->error != SFS_STATUS_INVALID_NAME) { /* should be INVALID_NAME */
            test_status[tc] = 1;
            SEC_HAL_TRACE("Fail/TC13(1): Error code %d.", filestr[10]->error);
        }


        /* filename with illegal characters - only '/' is illegal */	
        ssfopen("File/ill.txt", "w", 10);

        /* check status */
        if (filestr[10]->error != SFS_STATUS_INVALID_NAME) { /* should be INVALID_NAME */
            test_status[tc] = 1;
            SEC_HAL_TRACE("Fail/TC13(2): Error code %d.", filestr[10]->error);
        }

        /* filename ending with '/' */	
        ssfopen("AnotherDir/", "w", 10);

        /* check status */
        if (filestr[10]->error != SFS_STATUS_INVALID_NAME) { /* should be INVALID_NAME */
            test_status[tc] = 1;
            SEC_HAL_TRACE("Fail/TC13(3): Error code %d.", filestr[10]->error);
        }

        /* if failed return right away */
        if (test_status[tc]) {
            SEC_HAL_TRACE_EXIT();
            return tc;
        }
        SEC_HAL_TRACE("PASSED/TC13");
    }

/*                      tc14: File Write while no more space in Sec Storage
**                            -> Error code returned (NO_SPACE)*/
    if (testcase == 0 || testcase == 14) {
        tc = 14;

        if ((filestr[2] == 0)) { SEC_HAL_TRACE("Fail/TC14: Null pointer."); SEC_HAL_TRACE_EXIT(); return tc; }

        /* save original handle */
        tmphandle = filestr[2]->file_handle;

        /* handle 19999 is used to pretend no-space situation */
        filestr[2]->file_handle = 19999;

        /* Try to write to file3 while there is no space in filesystem (eMMC) */
        testvalue = ssfwrite(w_buff+(2*99), 1, filelen[2], filestr[2]);

        /* restore original handle */
        filestr[0]->file_handle = tmphandle;

        /* check if failed */
        if (testvalue) test_status[tc] = 1; /* write should return 0 */
        /* check that correct error code was returned */
        if (filestr[2]->error != SFS_STATUS_NO_SPACE) test_status[tc] = 1; /* should return NO_SPACE */

        /* if failed return right away */
        if (test_status[tc]) {
            SEC_HAL_TRACE("Fail/TC14: Error code %d. Write %d. Test_status %d.", filestr[2]->error, testvalue, test_status[tc]);
            SEC_HAL_TRACE_EXIT();
            return tc;
        }
        SEC_HAL_TRACE("PASSED/TC14");
    }

    /* ------------------- End of Test cases ------------------- */

    /* Overall test status, if all test cases passed, return value will be 0 */
    for (i = 1; i < 15; i++)
        test_status[0] |= test_status[i] << i;

    SEC_HAL_TRACE_EXIT();
    return test_status[0];
}


/* ssfopen - fileopen for secure storage testing purposes

   if mode is read 'r' then LOOKUP 
   if mode is write 'w' then first try LOOKUP, if it fails (file does not exist) then CREATE & LOOKUP
   if mode is something else return NULL (failure) */
sec_sto_file_t *ssfopen(const char *filename, const char *mode, int index)
{
    uint32_t status = SFS_STATUS_OK;
    uint32_t sec_msg_status = SEC_MSG_STATUS_OK;
    uint32_t rpc_status = RPC_SUCCESS;
    uint16_t msg_size;
    sec_msg_t *in_msg = NULL;
    sec_msg_t *out_msg = NULL;
    sec_msg_handle_t in_handle;
    sec_msg_handle_t out_handle;

    uint32_t openmode = 0;
    uint32_t file_h = 0;
    uint32_t namelen = 0;
    sec_sto_file_t *stream = NULL;

    if (filename == NULL || mode == NULL)
    {
        SEC_HAL_TRACE("Null pointer!");
        return NULL;
    }

    if ((index < 0) || (index >= 11))
    {
        SEC_HAL_TRACE("Index not in range 0...11!");
        return NULL;
    }

    if (strchr(mode, 'w') != NULL)
    {
        openmode = 1;
    }
    else if (strchr(mode, 'r') != NULL)
    {
        openmode = 2;
    }

    if (openmode == 0)
    {
        SEC_HAL_TRACE("Mode not read or write!");
        return NULL;
    }

    stream = &fstream[index];
    stream->error = 0;
    stream->file_handle = 0;

    namelen = strlen(filename);
    namelen += 1;

/*LOOKUP*/
    /* allocate memory, from ICRAM, for msgs to be sent to RPC_HANDLER */
    msg_size = sec_msg_param_size(sizeof(uint32_t)) +
               sec_msg_param_size(sizeof(uint32_t)) +
               sec_msg_param_size(namelen);

    in_msg = sec_msg_alloc(&in_handle, msg_size, SEC_MSG_OBJECT_ID_NONE, 0, SEC_HAL_MSG_BYTE_ORDER);
				
    msg_size = sec_msg_param_size(sizeof(uint32_t)) +
               sec_msg_param_size(sizeof(uint32_t));
				
    out_msg = sec_msg_alloc(&out_handle, msg_size, SEC_MSG_OBJECT_ID_NONE, 0, SEC_HAL_MSG_BYTE_ORDER);

    if (NULL == in_msg || NULL == out_msg)
    {
        SEC_HAL_TRACE("Alloc failure, msg not sent!");
        return NULL;
    }
    else
    {
        /* write content to the input msg */
        sec_msg_param_write32(&in_handle, 0, SEC_MSG_PARAM_ID_NONE);
        sec_msg_param_write32(&in_handle, namelen, SEC_MSG_PARAM_ID_NONE);
        sec_msg_param_write(&in_handle, filename, namelen, SEC_MSG_PARAM_ID_NONE);

        rpc_status = rpch(SEC_HAL_RPC_FS_LOOKUP, SEC_HAL_MEM_VIR2PHY_FUNC(out_msg), SEC_HAL_MEM_VIR2PHY_FUNC(in_msg), 0, 0);

        /* interpret the response */
        sec_msg_status = sec_msg_param_read32(&out_handle, &status);
        if (status == SFS_STATUS_OK)
        {
            sec_msg_status = sec_msg_param_read32(&out_handle, &file_h);
        }
        if ((RPC_SUCCESS != rpc_status) || (SEC_MSG_STATUS_OK != sec_msg_status))
        {
            SEC_HAL_TRACE("Fail, LOOKUP!");
            SEC_HAL_TRACE_INT("rpc_status", rpc_status);
            SEC_HAL_TRACE_INT("sec_msg_status ", sec_msg_status);
            SEC_HAL_TRACE_INT("status", status);
        }
        sec_msg_free(in_msg);
        sec_msg_free(out_msg);
    }
/*END OF LOOKUP*/

    stream->error = status;

    if ((status != SFS_STATUS_OK) && (status != SFS_STATUS_NOT_FOUND))
    {
        SEC_HAL_TRACE("LOOKUP status %d", status);
        return NULL;
    }

    if ((status == SFS_STATUS_NOT_FOUND) && (openmode == 2)) /* read mode and file not found */
    {
        return NULL;
    }

    if ((openmode == 1) && !file_h)
    {
/*CREATE*/
        /* allocate memory, from ICRAM, for msgs to be sent to RPC_HANDLER */
        msg_size = sec_msg_param_size(sizeof(uint32_t)) +
                   sec_msg_param_size(sizeof(uint32_t)) +
                   sec_msg_param_size(namelen);

        in_msg = sec_msg_alloc(&in_handle, msg_size, SEC_MSG_OBJECT_ID_NONE, 0, SEC_HAL_MSG_BYTE_ORDER);

        msg_size = sec_msg_param_size(sizeof(uint32_t));

        out_msg = sec_msg_alloc(&out_handle, msg_size, SEC_MSG_OBJECT_ID_NONE, 0, SEC_HAL_MSG_BYTE_ORDER);

        if ((NULL == in_msg) || (NULL == out_msg))
        {
            SEC_HAL_TRACE("Alloc failure, msg not sent!");
            return NULL;
        }
        else
        {
            /* write content to the input msg */
            sec_msg_param_write32(&in_handle, 0, SEC_MSG_PARAM_ID_NONE);
            sec_msg_param_write32(&in_handle, namelen, SEC_MSG_PARAM_ID_NONE);
            sec_msg_param_write(&in_handle, filename, namelen, SEC_MSG_PARAM_ID_NONE);

            rpc_status = rpch(SEC_HAL_RPC_FS_CREATE, SEC_HAL_MEM_VIR2PHY_FUNC(out_msg), SEC_HAL_MEM_VIR2PHY_FUNC(in_msg), 0, 0);
		
            /* interpret the response */
            sec_msg_status = sec_msg_param_read32(&out_handle, &status);

            if ((RPC_SUCCESS != rpc_status) || (SEC_MSG_STATUS_OK != sec_msg_status))
            {
                SEC_HAL_TRACE("Fail, CREATE!");
                SEC_HAL_TRACE_INT("rpc_status", rpc_status);
                SEC_HAL_TRACE_INT("sec_msg_status ", sec_msg_status);
                SEC_HAL_TRACE_INT("status", status);
            }
            sec_msg_free(in_msg);
            sec_msg_free(out_msg);
        }
/*END OF CREATE*/

        if (status != SFS_STATUS_OK)
        {
            SEC_HAL_TRACE("CREATE status %d", status);
            stream->error = status;
            return NULL;
        }

/*LOOKUP*/
        /* allocate memory, from ICRAM, for msgs to be sent to RPC_HANDLER */
        msg_size = sec_msg_param_size(sizeof(uint32_t)) +
                   sec_msg_param_size(sizeof(uint32_t)) +
                   sec_msg_param_size(namelen);

        in_msg = sec_msg_alloc(&in_handle, msg_size, SEC_MSG_OBJECT_ID_NONE, 0, SEC_HAL_MSG_BYTE_ORDER);

        msg_size = sec_msg_param_size(sizeof(uint32_t)) +
                   sec_msg_param_size(sizeof(uint32_t));

        out_msg = sec_msg_alloc(&out_handle, msg_size, SEC_MSG_OBJECT_ID_NONE, 0, SEC_HAL_MSG_BYTE_ORDER);

        if ((NULL == in_msg) || (NULL == out_msg))
        {
            SEC_HAL_TRACE("Alloc failure, msg not sent!");
            return NULL;
        }
        else
        {
            /* write content to the input msg */
            sec_msg_param_write32(&in_handle, 0, SEC_MSG_PARAM_ID_NONE);
            sec_msg_param_write32(&in_handle, namelen, SEC_MSG_PARAM_ID_NONE);
            sec_msg_param_write(&in_handle, filename, namelen, SEC_MSG_PARAM_ID_NONE);

            rpc_status = rpch(SEC_HAL_RPC_FS_LOOKUP, SEC_HAL_MEM_VIR2PHY_FUNC(out_msg), SEC_HAL_MEM_VIR2PHY_FUNC(in_msg), 0, 0);
		
            /* interpret the response */
            sec_msg_status = sec_msg_param_read32(&out_handle, &status);
            if (status == SFS_STATUS_OK)
            {
                sec_msg_status = sec_msg_param_read32(&out_handle, &file_h);
            }
            if ((RPC_SUCCESS != rpc_status) || (SEC_MSG_STATUS_OK != sec_msg_status))
            {
                SEC_HAL_TRACE("Fail, LOOKUP!");
                SEC_HAL_TRACE_INT("rpc_status", rpc_status);
                SEC_HAL_TRACE_INT("sec_msg_status ", sec_msg_status);
                SEC_HAL_TRACE_INT("status", status);
            }
            sec_msg_free(in_msg);
            sec_msg_free(out_msg);
        }
/*END OF LOOKUP*/
        stream->error = status;

        if (status != SFS_STATUS_OK)
        {
            SEC_HAL_TRACE("LOOKUP status %d", status);
            stream->error = status;
            return NULL;
        }
    }

    stream->file_handle = file_h;

    return stream;
}


/* ssfread - fileread for secure storage testing purposes
 */
size_t ssfread(void *buf, size_t size, size_t nitems, sec_sto_file_t *stream)
{
    uint32_t status = SFS_STATUS_OK;
    uint32_t sec_msg_status = SEC_MSG_STATUS_OK;
    uint32_t rpc_status = RPC_SUCCESS;
    uint16_t msg_size;
    sec_msg_t *in_msg = NULL;
    sec_msg_t *out_msg = NULL;
    sec_msg_handle_t in_handle;
    sec_msg_handle_t out_handle;

    uint32_t buf_size = size * nitems;
    uint32_t length_read = 0;

    if (!stream || !buf || !buf_size)
    {
        SEC_HAL_TRACE("Null pointer!");
        return 0;
    }

/*READ*/
    /* allocate memory, from ICRAM, for msgs to be sent to RPC_HANDLER */
    msg_size = sec_msg_param_size(sizeof(uint32_t)) +
               sec_msg_param_size(sizeof(uint64_t)) +
               sec_msg_param_size(sizeof(uint32_t));

    in_msg = sec_msg_alloc(&in_handle, msg_size, SEC_MSG_OBJECT_ID_NONE, 0, SEC_HAL_MSG_BYTE_ORDER);

    msg_size = sec_msg_param_size(sizeof(uint32_t)) +
               sec_msg_param_size(sizeof(uint32_t)) +
               sec_msg_param_size(buf_size);

    out_msg = sec_msg_alloc(&out_handle, msg_size, SEC_MSG_OBJECT_ID_NONE, 0, SEC_HAL_MSG_BYTE_ORDER);

    if (NULL == in_msg || NULL == out_msg)
    {
        SEC_HAL_TRACE("Alloc failure, msg not sent!");
        return 0;
    }
    else
    {
        /* write content to the input msg */
        sec_msg_param_write32(&in_handle, stream->file_handle, SEC_MSG_PARAM_ID_NONE);
        sec_msg_param_write64(&in_handle, 0, SEC_MSG_PARAM_ID_NONE);
        sec_msg_param_write32(&in_handle, buf_size, SEC_MSG_PARAM_ID_NONE);

        rpc_status = rpch(SEC_HAL_RPC_FS_READ, SEC_HAL_MEM_VIR2PHY_FUNC(out_msg), SEC_HAL_MEM_VIR2PHY_FUNC(in_msg), 0, 0);

        /* interpret the response */
        sec_msg_status = sec_msg_param_read32(&out_handle, &status);
        if ((status == SFS_STATUS_OK) || (status == SFS_STATUS_END_OF_FILE))
        {
            sec_msg_status = sec_msg_param_read32(&out_handle, &length_read);
            sec_msg_status = sec_msg_param_read(&out_handle, buf, (uint16_t)length_read);
        }		

        if ((RPC_SUCCESS != rpc_status) || (SEC_MSG_STATUS_OK != sec_msg_status))
        {
            SEC_HAL_TRACE("Fail, READ!");
            SEC_HAL_TRACE_INT("rpc_status", rpc_status);
            SEC_HAL_TRACE_INT("sec_msg_status ", sec_msg_status);
            SEC_HAL_TRACE_INT("status", status);
        }
        sec_msg_free(in_msg);
        sec_msg_free(out_msg);
    }
/*END OF READ*/

    if (status != SFS_STATUS_OK)
    {
        SEC_HAL_TRACE("READ status %d", status);
        stream->error = status;
        if (status != SFS_STATUS_END_OF_FILE) {
            return 0;
        }
    }

    return (size_t)length_read;
}

/* ssfwrite - filewrite for secure storage testing purposes
 */
size_t ssfwrite(void *buf, size_t size, size_t nitems, sec_sto_file_t *stream)
{
    uint32_t status = SFS_STATUS_OK;
    uint32_t sec_msg_status = SEC_MSG_STATUS_OK;
    uint32_t rpc_status = RPC_SUCCESS;
    uint16_t msg_size;
    sec_msg_t *in_msg = NULL;
    sec_msg_t *out_msg = NULL;
    sec_msg_handle_t in_handle;
    sec_msg_handle_t out_handle;

    uint32_t buf_size = size * nitems;

    if (!stream || !buf || buf_size == 0)
    {
        SEC_HAL_TRACE("Null pointer!");
        return 0;
    }

/*WRITE*/
    /* allocate memory, from ICRAM, for msgs to be sent to RPC_HANDLER */
    msg_size = sec_msg_param_size(sizeof(uint32_t)) +
               sec_msg_param_size(sizeof(uint64_t)) +
               sec_msg_param_size(sizeof(uint32_t)) +
               sec_msg_param_size(buf_size);

    in_msg = sec_msg_alloc(&in_handle, msg_size, SEC_MSG_OBJECT_ID_NONE, 0, SEC_HAL_MSG_BYTE_ORDER);

    msg_size = sec_msg_param_size(sizeof(uint32_t));

    out_msg = sec_msg_alloc(&out_handle, msg_size, SEC_MSG_OBJECT_ID_NONE, 0, SEC_HAL_MSG_BYTE_ORDER);

    if ((NULL == in_msg) || (NULL == out_msg))
    {
        SEC_HAL_TRACE("Alloc failure, msg not sent!");
        return 0;
    }
    else
    {
        /* write content to the input msg */
        sec_msg_param_write32(&in_handle, stream->file_handle, SEC_MSG_PARAM_ID_NONE);
        sec_msg_param_write64(&in_handle, 0, SEC_MSG_PARAM_ID_NONE);
        sec_msg_param_write32(&in_handle, buf_size, SEC_MSG_PARAM_ID_NONE);
        sec_msg_param_write(&in_handle, buf, (uint16_t)buf_size, SEC_MSG_PARAM_ID_NONE);

        rpc_status = rpch(SEC_HAL_RPC_FS_WRITE, SEC_HAL_MEM_VIR2PHY_FUNC(out_msg), SEC_HAL_MEM_VIR2PHY_FUNC(in_msg), 0, 0);

        /* interpret the response */
        sec_msg_status = sec_msg_param_read32(&out_handle, &status);

        if ((RPC_SUCCESS != rpc_status) || (SEC_MSG_STATUS_OK != sec_msg_status))
        {
            SEC_HAL_TRACE("Fail, WRITE!");
            SEC_HAL_TRACE_INT("rpc_status", rpc_status);
            SEC_HAL_TRACE_INT("sec_msg_status ", sec_msg_status);
            SEC_HAL_TRACE_INT("status", status);
        }
        sec_msg_free(in_msg);
        sec_msg_free(out_msg);
    }
/*END OF WRITE*/

    if (status != SFS_STATUS_OK)
    {
        SEC_HAL_TRACE("WRITE status %d", status);
        stream->error = status;
        return 0;
    }

    return 1;
}

/* ssfclose - fileclose for secure storage testing purposes
 */
uint32_t ssfclose(sec_sto_file_t *file)
{
    if (!file)
    {
        SEC_HAL_TRACE("Null pointer!");
        return 0;
    }

    return 0;
}

/* ssfsize - filesize for secure storage testing purposes
 */
size_t ssfsize(sec_sto_file_t *file)
{
    uint32_t status = SFS_STATUS_OK;
    uint32_t sec_msg_status = SEC_MSG_STATUS_OK;
    uint32_t rpc_status = RPC_SUCCESS;
    uint16_t msg_size;
    sec_msg_t *in_msg = NULL;
    sec_msg_t *out_msg = NULL;
    sec_msg_handle_t in_handle;
    sec_msg_handle_t out_handle;

    uint64_t filesize = 0;

    if (!file)
    {
        SEC_HAL_TRACE("Null pointer!");
        return 0;
    }

/*SIZE*/
    /* allocate memory, from ICRAM, for msgs to be sent to RPC_HANDLER */
    msg_size = sec_msg_param_size(sizeof(uint32_t));

    in_msg = sec_msg_alloc(&in_handle, msg_size, SEC_MSG_OBJECT_ID_NONE, 0, SEC_HAL_MSG_BYTE_ORDER);

    msg_size = sec_msg_param_size(sizeof(uint32_t)) +
               sec_msg_param_size(sizeof(uint64_t));

    out_msg = sec_msg_alloc(&out_handle, msg_size, SEC_MSG_OBJECT_ID_NONE, 0, SEC_HAL_MSG_BYTE_ORDER);

    if ((NULL == in_msg) || (NULL == out_msg))
    {
        SEC_HAL_TRACE("Alloc failure, msg not sent!");
        return 0;
    }
    else
    {
        /* write content to the input msg */
        sec_msg_param_write32(&in_handle, file->file_handle, SEC_MSG_PARAM_ID_NONE);

        rpc_status = rpch(SEC_HAL_RPC_FS_SIZE, SEC_HAL_MEM_VIR2PHY_FUNC(out_msg), SEC_HAL_MEM_VIR2PHY_FUNC(in_msg), 0, 0);

        /* interpret the response */
        sec_msg_status = sec_msg_param_read32(&out_handle, &status);
        if (status == SFS_STATUS_OK)
        {
            sec_msg_status = sec_msg_param_read64(&out_handle, &filesize);
        }		

	if ((RPC_SUCCESS != rpc_status) || (SEC_MSG_STATUS_OK != sec_msg_status))
        {
            SEC_HAL_TRACE("Fail, SIZE!");
            SEC_HAL_TRACE_INT("rpc_status", rpc_status);
            SEC_HAL_TRACE_INT("sec_msg_status ", sec_msg_status);
            SEC_HAL_TRACE_INT("status", status);
        }
        sec_msg_free(in_msg);
        sec_msg_free(out_msg);
    }
/*END OF SIZE*/

    if (status != SFS_STATUS_OK)
    {
        SEC_HAL_TRACE("SIZE status %d", status);
        file->error = status;
        return 0;
    }

    return (size_t)filesize;
}

uint32_t ssfgetroot(uint32_t *rhandle)
{
    uint32_t status = SFS_STATUS_OK;
    uint32_t sec_msg_status = SEC_MSG_STATUS_OK;
    uint32_t rpc_status = RPC_SUCCESS;
    uint16_t msg_size;
    sec_msg_t *out_msg = NULL;
    sec_msg_handle_t out_handle;

    if (!rhandle)
    {
        SEC_HAL_TRACE("Null pointer!");
        return 555;
    }

/*ROOT*/
    /* allocate memory, from ICRAM, for msgs to be sent to RPC_HANDLER */
    /* Root has no in_msg at all */
    msg_size = sec_msg_param_size(sizeof(uint32_t)) +
               sec_msg_param_size(sizeof(uint32_t));

    out_msg = sec_msg_alloc(&out_handle, msg_size, SEC_MSG_OBJECT_ID_NONE, 0, SEC_HAL_MSG_BYTE_ORDER);

    if (NULL == out_msg)
    {
        SEC_HAL_TRACE("Alloc failure, msg not sent!");
        return 555;
    }
    else
    {
        rpc_status = rpch(SEC_HAL_RPC_FS_ROOT, SEC_HAL_MEM_VIR2PHY_FUNC(out_msg), 0, 0, 0);

        /* interpret the response */
        sec_msg_status = sec_msg_param_read32(&out_handle, &status);
        if (status == SFS_STATUS_OK)
        {
            sec_msg_status = sec_msg_param_read32(&out_handle, rhandle);
        }
        if ((RPC_SUCCESS != rpc_status) || (SEC_MSG_STATUS_OK != sec_msg_status))
        {
            SEC_HAL_TRACE("Fail, ROOT!");
            SEC_HAL_TRACE_INT("rpc_status", rpc_status);
            SEC_HAL_TRACE_INT("sec_msg_status ", sec_msg_status);
            SEC_HAL_TRACE_INT("status", status);
        }
        sec_msg_free(out_msg);
    }
/*END OF ROOT*/

    if (status != SFS_STATUS_OK)
    {
        SEC_HAL_TRACE("ROOT status %d", status);
        *rhandle = 0;
    }

    return status;
}

uint32_t ssfmv(const char *from, const char *to)
{
    uint32_t status = SFS_STATUS_OK;
    uint32_t sec_msg_status = SEC_MSG_STATUS_OK;
    uint32_t rpc_status = RPC_SUCCESS;
    uint16_t msg_size;
    sec_msg_t *in_msg = NULL;
    sec_msg_t *out_msg = NULL;
    sec_msg_handle_t in_handle;
    sec_msg_handle_t out_handle;

    uint32_t namelen_f = 0;
    uint32_t namelen_t = 0;

    if (!from || !to)
    {
        SEC_HAL_TRACE("Null pointer!");
        return 555;
    }

    namelen_f = strlen(from);
    namelen_f += 1;
    namelen_t = strlen(to);
    namelen_t += 1;

/*MOVE*/
    /* allocate memory, from ICRAM, for msgs to be sent to RPC_HANDLER */
    msg_size = sec_msg_param_size(sizeof(uint32_t)) +
               sec_msg_param_size(sizeof(uint32_t)) +
               sec_msg_param_size(namelen_f) +
               sec_msg_param_size(sizeof(uint32_t)) +
               sec_msg_param_size(sizeof(uint32_t)) +
               sec_msg_param_size(namelen_t);

    in_msg = sec_msg_alloc(&in_handle, msg_size, SEC_MSG_OBJECT_ID_NONE, 0, SEC_HAL_MSG_BYTE_ORDER);

    msg_size = sec_msg_param_size(sizeof(uint32_t));

    out_msg = sec_msg_alloc(&out_handle, msg_size, SEC_MSG_OBJECT_ID_NONE, 0, SEC_HAL_MSG_BYTE_ORDER);

    if ((NULL == in_msg) || (NULL == out_msg))
    {
        SEC_HAL_TRACE("Alloc failure, msg not sent!");
        return 555;
    }
    else
    {
        /* write content to the input msg */
        sec_msg_param_write32(&in_handle, 0, SEC_MSG_PARAM_ID_NONE);
        sec_msg_param_write32(&in_handle, namelen_f, SEC_MSG_PARAM_ID_NONE);
        sec_msg_param_write(&in_handle, from, namelen_f, SEC_MSG_PARAM_ID_NONE);
        sec_msg_param_write32(&in_handle, 0, SEC_MSG_PARAM_ID_NONE);
        sec_msg_param_write32(&in_handle, namelen_t, SEC_MSG_PARAM_ID_NONE);
        sec_msg_param_write(&in_handle, to, namelen_t, SEC_MSG_PARAM_ID_NONE);

        rpc_status = rpch(SEC_HAL_RPC_FS_MOVE, SEC_HAL_MEM_VIR2PHY_FUNC(out_msg), SEC_HAL_MEM_VIR2PHY_FUNC(in_msg), 0, 0);

        /* interpret the response */
        sec_msg_status = sec_msg_param_read32(&out_handle, &status);

        if ((RPC_SUCCESS != rpc_status) || (SEC_MSG_STATUS_OK != sec_msg_status))
        {
            SEC_HAL_TRACE("Fail, MOVE!");
            SEC_HAL_TRACE_INT("rpc_status", rpc_status);
            SEC_HAL_TRACE_INT("sec_msg_status ", sec_msg_status);
            SEC_HAL_TRACE_INT("status", status);
        }
        sec_msg_free(in_msg);
        sec_msg_free(out_msg);
    }
/*END OF MOVE*/

    if (status != SFS_STATUS_OK)
    {
        SEC_HAL_TRACE("MOVE status %d", status);
    }

    return status;
}

uint32_t ssfrm(const char *filename)
{
    uint32_t status = SFS_STATUS_OK;
    uint32_t sec_msg_status = SEC_MSG_STATUS_OK;
    uint32_t rpc_status = RPC_SUCCESS;
    uint16_t msg_size;
    sec_msg_t *in_msg = NULL;
    sec_msg_t *out_msg = NULL;
    sec_msg_handle_t in_handle;
    sec_msg_handle_t out_handle;

    uint32_t namelen = 0;

    if (!filename)
    {
        SEC_HAL_TRACE("Null pointer!");
        return 555;
    }

    namelen = strlen(filename);
    namelen += 1;

/*REMOVE*/
    /* allocate memory, from ICRAM, for msgs to be sent to RPC_HANDLER */
    msg_size = sec_msg_param_size(sizeof(uint32_t)) +
               sec_msg_param_size(sizeof(uint32_t)) +
               sec_msg_param_size(namelen);

    in_msg = sec_msg_alloc(&in_handle, msg_size, SEC_MSG_OBJECT_ID_NONE, 0, SEC_HAL_MSG_BYTE_ORDER);

    msg_size = sec_msg_param_size(sizeof(uint32_t));

    out_msg = sec_msg_alloc(&out_handle, msg_size, SEC_MSG_OBJECT_ID_NONE, 0, SEC_HAL_MSG_BYTE_ORDER);

    if ((NULL == in_msg) || (NULL == out_msg))
    {
        SEC_HAL_TRACE("Alloc failure, msg not sent!");
        return 555;
    }
    else
    {
        /* write content to the input msg */
        sec_msg_param_write32(&in_handle, 0, SEC_MSG_PARAM_ID_NONE);
        sec_msg_param_write32(&in_handle, namelen, SEC_MSG_PARAM_ID_NONE);
        sec_msg_param_write(&in_handle, filename, namelen, SEC_MSG_PARAM_ID_NONE);

        rpc_status = rpch(SEC_HAL_RPC_FS_REMOVE, SEC_HAL_MEM_VIR2PHY_FUNC(out_msg), SEC_HAL_MEM_VIR2PHY_FUNC(in_msg), 0, 0);

        /* interpret the response */
        sec_msg_status = sec_msg_param_read32(&out_handle, &status);

        if ((RPC_SUCCESS != rpc_status) || (SEC_MSG_STATUS_OK != sec_msg_status))
        {
            SEC_HAL_TRACE("Fail, REMOVE!");
            SEC_HAL_TRACE_INT("rpc_status", rpc_status);
            SEC_HAL_TRACE_INT("sec_msg_status ", sec_msg_status);
            SEC_HAL_TRACE_INT("status", status);
	}
        sec_msg_free(in_msg);
        sec_msg_free(out_msg);
    }
/*END OF REMOVE*/

    if (status != SFS_STATUS_OK)
    {
        SEC_HAL_TRACE("REMOVE status %d", status);
    }

    return status;
}

#endif

/* ******************************** END ********************************** */
