/* ************************************************************************* **
**                               Renesas                                     **
** ************************************************************************* */

/* *************************** COPYRIGHT INFORMATION *********************** **
** This program contains proprietary information that is a trade secret of   **
** Renesas and also is protected as an unpublished work under                **
** applicable Copyright laws. Recipient is to retain this program in         **
** confidence and is not permitted to use or make copies thereof other than  **
** as permitted in a written agreement with Renesas.                         **
**                                                                           **
** Copyright (C) 2010-2012 Renesas Electronics Corp.                         **
** All rights reserved.                                                      **
** ************************************************************************* */


/* ************************ HEADER (INCLUDE) SECTION *********************** */
#include "sec_hal_rt.h"
#include "sec_hal_rt_cmn.h"
#include "sec_hal_rt_trace.h"
#include "sec_serv_api.h"
#include "sec_dispatch.h"

#ifndef SEC_HAL_TEST_ISOLATION
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#else
#include <string.h>
#endif /* SEC_HAL_TEST_ISOLATION */


/* ***************** MACROS, CONSTANTS, COMPILATION FLAGS ****************** */
#ifdef LOCAL_DISP_FUNC
#error !!Local dispatcher function already defined!!
#else /* LOCAL_DISP_FUNC */
#define LOCAL_DISP_FUNC sec_dispatcher
#endif /* LOCAL_DISP_FUNC */

#ifdef LOCAL_DEFAULT_DISP_FLAGS
#error !!Local dispatcher default flags already defined!!
#else /* LOCAL_DEFAULT_DISP_FLAGS */
#define LOCAL_DEFAULT_DISP_FLAGS 0
#endif /* LOCAL_DEFAULT_DISP_FLAGS */

#ifdef LOCAL_DEFAULT_DISP_SPARE_PARAM
#error !!Local dispatcher spare param already defined!!
#else /* LOCAL_DEFAULT_DISP_SPARE_PARAM */
#define LOCAL_DEFAULT_DISP_SPARE_PARAM 0
#endif /* LOCAL_DEFAULT_DISP_SPARE_PARAM */

#ifdef LOCAL_DEFAULT_PERIODIC_TIMER_VALUE
#error !!Local timer value already defined!!
#else /* LOCAL_DEFAULT_PERIODIC_TIMER_VALUE */
#define LOCAL_DEFAULT_PERIODIC_TIMER_VALUE 60000
#endif /* LOCAL_DEFAULT_PERIODIC_TIMER_VALUE */

#ifdef LOCAL_DEFAULT_SIMLOCK_LEVEL_COUNT
#error !!Local simlock level count already defined!!
#else /* LOCAL_DEFAULT_SIMLOCK_LEVEL_COUNT */
#define LOCAL_DEFAULT_SIMLOCK_LEVEL_COUNT 5
#endif /* LOCAL_DEFAULT_SIMLOCK_LEVEL_COUNT */

#if (defined LOCAL_CPY_TO_CLIENT_FUNC || defined LOCAL_CPY_TO_CLIENT_FUNCPTR)
#error !!copy to client already defined, aborting!!
#else /* LOCAL_CPY_TO_CLIENT_FUNC */
#ifndef SEC_HAL_TEST_ISOLATION
/* ****************************************************************************
** Function name      : copy_to_userspace
** Description        : copy content to userspace, to user page.
** Parameters         :
** Return value       : number of bytes left uncopied.
** ***************************************************************************/
static unsigned long copy_to_userspace(
		void* dst,
		const void* src,
		unsigned long sz)
{
	return !copy_to_user((void __user *)dst, src, sz);
}
#define LOCAL_CPY_TO_CLIENT_FUNC    copy_to_userspace
#define LOCAL_CPY_TO_CLIENT_FUNCPTR &copy_to_userspace
#else /* SEC_HAL_TEST_ISOLATION */
#define LOCAL_CPY_TO_CLIENT_FUNC    !memcpy
#define LOCAL_CPY_TO_CLIENT_FUNCPTR &memcpy
#endif /* SEC_HAL_TEST_ISOLATION */
#endif /* LOCAL_CPY_TO_CLIENT_FUNC */

#if (defined LOCAL_CPY_TO_HW_FUNC || defined LOCAL_CPY_TO_HW_FUNCPTR)
#error !!copy to hw already defined, aborting!!
#else /* LOCAL_CPY_TO_IO_FUNC */
#ifndef SEC_HAL_TEST_ISOLATION
#define LOCAL_CPY_TO_HW_FUNC    memcpy_toio
#define LOCAL_CPY_TO_HW_FUNCPTR &memcpy_toio
#else
#define LOCAL_CPY_TO_HW_FUNC    memcpy
#define LOCAL_CPY_TO_HW_FUNCPTR &memcpy
#endif /* SEC_HAL_TEST_ISOLATION */
#endif /* LOCAL_CPY_TO_HW_FUNC */


/* empty unlock code for simlock */
const char* k_sec_hal_simlock_empty_code = "EMPTY";



/* ****************************** CODE SECTION ***************************** */
/* ****************************************************************************
** Function name      : sec_hal_rt_init
** Description        : initialize global/local resources.
** Parameters         : OUT/--- uint32_t *virt_wdt_upd_out, secure wdt expiration timeout value.
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** ***************************************************************************/
uint32_t sec_hal_rt_init(
		uint32_t *virt_wdt_upd_out)
{
	uint32_t sec_hal_status = SEC_HAL_RES_OK;
	uint32_t ssa_disp_status = 0x00;
	sec_msg_t *in_msg = NULL;
	sec_msg_t *out_msg = NULL;
	sec_msg_handle_t in_handle;
	sec_msg_handle_t out_handle;

	SEC_HAL_TRACE_ENTRY

	/* allocate memory, from ICRAM, for msgs to be sent to TZ */
	in_msg = sec_msg_alloc(
				&in_handle,
				0,
				SEC_MSG_OBJECT_ID_NONE,
				0,
				SEC_HAL_MSG_BYTE_ORDER);
	out_msg = sec_msg_alloc(
				&out_handle,
				0,
				SEC_MSG_OBJECT_ID_NONE,
				0,
				SEC_HAL_MSG_BYTE_ORDER);

	if (NULL == in_msg || NULL == out_msg)
	{
		SEC_HAL_TRACE("alloc failure, msgs not sent!")
		sec_hal_status = SEC_HAL_RES_FAIL;
	}
	else
	{
		/* call dispatcher */
		ssa_disp_status = LOCAL_DISP_FUNC(
							SEC_SERV_RUNTIME_INIT,
							LOCAL_DEFAULT_DISP_FLAGS,
							LOCAL_DEFAULT_DISP_SPARE_PARAM,
							SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
							SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));

		/* interpret the response */
		sec_hal_status = (
				SEC_ROM_RET_OK != ssa_disp_status ?
				SEC_HAL_RES_FAIL : sec_hal_status );
	}

	/* de-allocate msgs */
	sec_msg_free(out_msg);
	sec_msg_free(in_msg);

	if (sec_hal_status == SEC_HAL_RES_OK && NULL != virt_wdt_upd_out)
	{
		/* default timer value for the first period */
		*virt_wdt_upd_out = LOCAL_DEFAULT_PERIODIC_TIMER_VALUE;
	}

	SEC_HAL_TRACE_EXIT
	return sec_hal_status;
}

/* ****************************************************************************
** Function name      : sec_hal_rt_install_rpc_handler
** Description        : install rpc function to TZ.
** Parameters         : IN/--- sec_hal_rt_rpc_handler virt_func_ptr_in, virtual callback function address.
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** ***************************************************************************/
uint32_t sec_hal_rt_install_rpc_handler(
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

	SEC_HAL_TRACE_ENTRY

	if (NULL == virt_func_ptr_in)
	{
		SEC_HAL_TRACE_EXIT_INFO("!!null virt_func_ptr_in, aborting!!")
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
		SEC_HAL_TRACE("alloc failure, msgs not sent!")
		sec_hal_status = SEC_HAL_RES_FAIL;
	}
	else
	{
		/* write content to the input msg */
		sec_msg_param_write32(
				&in_handle,
				(uint32_t)virt_func_ptr_in,
				SEC_MSG_PARAM_ID_NONE);

		/* call dispatcher */
		ssa_disp_status = LOCAL_DISP_FUNC(
							SEC_SERV_RPC_ADDRESS,
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
			SEC_HAL_TRACE("op failed, INSTALL rpc handler failed!")
			SEC_HAL_TRACE_INT("ssa_disp_status", ssa_disp_status)
			SEC_HAL_TRACE_INT("sec_msg_status ", sec_msg_status)
			SEC_HAL_TRACE_INT("sec_serv_status", sec_serv_status)
			sec_hal_status = SEC_HAL_RES_FAIL;
		}
	}

	/* de-allocate msgs */
	sec_msg_free(out_msg);
	sec_msg_free(in_msg);

	SEC_HAL_TRACE_EXIT
	return sec_hal_status;
}

/* ****************************************************************************
** Function name      : sec_hal_rt_key_info_get
** Description        : request key information from the TZ side.
** Parameters         : OUT/--- sec_hal_key_info_t *key_info
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** ***************************************************************************/
uint32_t sec_hal_rt_key_info_get(
		sec_hal_key_info_t *user_key_info_out)
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

	SEC_HAL_TRACE_ENTRY

	if (NULL == user_key_info_out)
	{
		SEC_HAL_TRACE_EXIT_INFO("!!null user_key_info_out, aborting!!")
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
				sec_msg_param_size(SEC_HAL_KEY_INFO_SIZE);
	out_msg = sec_msg_alloc(
				&out_handle,
				msg_data_size,
				SEC_MSG_OBJECT_ID_NONE,
				0,
				SEC_HAL_MSG_BYTE_ORDER);

	if (NULL == in_msg || NULL == out_msg)
	{
		SEC_HAL_TRACE("alloc failure, msgs not sent!")
		sec_hal_status = SEC_HAL_RES_FAIL;
	}
	else
	{
		/* write content to the input msg */
		sec_msg_param_write32(
				&in_handle,
				SEC_HAL_KEY_INFO_SIZE,
				SEC_MSG_PARAM_ID_NONE);

		/* call dispatcher */
		ssa_disp_status = LOCAL_DISP_FUNC(
							SEC_SERV_KEY_INFO_REQUEST,
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
			SEC_HAL_TRACE("op failed, KEY INFO not available!")
			SEC_HAL_TRACE_INT("ssa_disp_status", ssa_disp_status)
			SEC_HAL_TRACE_INT("sec_msg_status ", sec_msg_status)
			SEC_HAL_TRACE_INT("sec_serv_status", sec_serv_status)
			sec_hal_status = SEC_HAL_RES_FAIL;
		}
		else
		{
			sec_msg_status = _sec_msg_param_read(LOCAL_CPY_TO_CLIENT_FUNCPTR,
								&out_handle,
								user_key_info_out,
								SEC_HAL_KEY_INFO_SIZE);
			if (SEC_MSG_STATUS_OK != sec_msg_status)
			{
				SEC_HAL_TRACE_INT("read failed! sec_msg_status", sec_msg_status)
				sec_hal_status = SEC_HAL_RES_FAIL;
			}
		}
	}

	/* de-allocate msgs */
	sec_msg_free(out_msg);
	sec_msg_free(in_msg);

	SEC_HAL_TRACE_EXIT
	return sec_hal_status;
}

/* ****************************************************************************
** Function name      : sec_hal_rt_cert_register
** Description        : register given certificate on TZ side.
** Parameters         : IN/--- void *cert
**                      IN/--- uint32_t cert_size
**                      OUT/ --- uint32_t *obj_id
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** ***************************************************************************/
uint32_t sec_hal_rt_cert_register(
		void *user_cert_in,
		uint32_t user_cert_size_in,
		uint32_t *user_obj_id_out)
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
	void *kernel_cert_in;

	SEC_HAL_TRACE_ENTRY

	if (NULL == user_cert_in)
	{
		SEC_HAL_TRACE_EXIT_INFO("!!null user_cert_in ptr, aborting!!")
		return SEC_HAL_RES_PARAM_ERROR;
	}
	if (0 == user_cert_size_in)
	{
		SEC_HAL_TRACE_EXIT_INFO("!!zero user_cert_size_in, aborting!!")
		return SEC_HAL_RES_PARAM_ERROR;
	}

	SEC_HAL_TRACE("user_cert_in: 0x%x", user_cert_in);
	SEC_HAL_TRACE("user_cert_size_in: %d", user_cert_size_in);

    kernel_cert_in = kmalloc(user_cert_size_in, GFP_KERNEL);
    copy_from_user( kernel_cert_in, user_cert_in, user_cert_size_in );
	SEC_HAL_TRACE("kernel_cert_in: 0x%x", kernel_cert_in);

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
		SEC_HAL_TRACE("alloc failure, msgs not sent!")
		sec_hal_status = SEC_HAL_RES_FAIL;
	}
	else
	{
		/* ensure that certificate is fully in work RAM memory */
		SEC_HAL_MEM_CACHE_CLEAN_FUNC(kernel_cert_in, user_cert_size_in);

		/* write content to the input msg */
		sec_msg_param_write32(
				&in_handle,
				(uint32_t)virt_to_phys(kernel_cert_in),
				SEC_MSG_PARAM_ID_NONE);

		/* call dispatcher */
		ssa_disp_status = LOCAL_DISP_FUNC(
							SEC_SERV_CERTIFICATE_REGISTER,
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
			SEC_HAL_TRACE("op failed, CERT register failed!")
			SEC_HAL_TRACE_INT("ssa_disp_status", ssa_disp_status)
			SEC_HAL_TRACE_INT("sec_msg_status ", sec_msg_status)
			SEC_HAL_TRACE_INT("sec_serv_status", sec_serv_status)
			sec_hal_status = SEC_HAL_RES_FAIL;
		}
	}

	/* de-allocate msgs */
	sec_msg_free(out_msg);
	sec_msg_free(in_msg);
	kfree(kernel_cert_in);

	SEC_HAL_TRACE_EXIT
	return sec_hal_status;
}

/* ****************************************************************************
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
** ***************************************************************************/
uint32_t sec_hal_rt_data_cert_register(
		void *user_cert_in,
		uint32_t user_cert_size_in,
		void *user_data_in,
		uint32_t user_data_size_in,
		uint32_t *user_id_ptr_out)
{
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
	SEC_HAL_TRACE_ENTRY

	if (NULL == user_cert_in)
	{
		SEC_HAL_TRACE_EXIT_INFO("!!null user_cert_in ptr, aborting!!")
		return SEC_HAL_RES_PARAM_ERROR;
	}
	if (0 == user_cert_size_in)
	{
		SEC_HAL_TRACE_EXIT_INFO("!!zero user_cert_size_in, aborting!!")
		return SEC_HAL_RES_PARAM_ERROR;
	}
	if (NULL == user_data_in)
	{
		SEC_HAL_TRACE_EXIT_INFO("!!null user_data_in ptr, aborting!!")
		return SEC_HAL_RES_PARAM_ERROR;
	}
	if (0 == user_data_size_in)
	{
		SEC_HAL_TRACE_EXIT_INFO("!!zero user_data_size_in, aborting!!")
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
		SEC_HAL_TRACE("alloc failure, msgs not sent!")
		sec_hal_status = SEC_HAL_RES_FAIL;
	}
	else
	{
		/* ensure that certificate and data are fully in RAM memory */
		SEC_HAL_MEM_CACHE_CLEAN_FUNC(user_cert_in, user_cert_size_in);
		SEC_HAL_MEM_CACHE_CLEAN_FUNC(user_data_in, user_data_size_in);

		/* write content to the input msg */
		sec_msg_param_write32(
				&in_handle,
				(uint32_t)virt_to_phys(user_cert_in),
				SEC_MSG_PARAM_ID_NONE);
		sec_msg_param_write32(
				&in_handle,
				(uint32_t)virt_to_phys(user_data_in),
				SEC_MSG_PARAM_ID_NONE);
		sec_msg_param_write32(&in_handle, user_data_size_in, SEC_MSG_PARAM_ID_NONE);

		/* call dispatcher */
		ssa_disp_status = LOCAL_DISP_FUNC(
							SEC_SERV_PROT_DATA_REGISTER,
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
			SEC_HAL_TRACE("op failed, data CERT register failed!")
			SEC_HAL_TRACE_INT("ssa_disp_status", ssa_disp_status)
			SEC_HAL_TRACE_INT("sec_msg_status ", sec_msg_status)
			SEC_HAL_TRACE_INT("sec_serv_status", sec_serv_status)
			sec_hal_status = SEC_HAL_RES_FAIL;
		}
		else if (NULL != user_id_ptr_out)
		{
			sec_msg_status = sec_msg_param_read32(&out_handle, &obj_id);
			if (SEC_MSG_STATUS_OK != sec_msg_status ||
				LOCAL_CPY_TO_CLIENT_FUNC(user_id_ptr_out, (void*)&obj_id, sizeof(uint32_t)))
			{
				SEC_HAL_TRACE_INT("read&copy failed! sec_msg_status", sec_msg_status)
				sec_hal_status = SEC_HAL_RES_FAIL;
			}
		}
	}

	/* de-allocate msgs */
	sec_msg_free(out_msg);
	sec_msg_free(in_msg);
	kfree(kernel_cert_in);
	kfree(kernel_data_in);

	SEC_HAL_TRACE_EXIT
	return sec_hal_status;
}

/* ****************************************************************************
** Function name      : sec_hal_rt_mac_address_get
** Description        : store requested MAC address on given userspace address.
** Parameters         : IN/--- sec_hal_mac_address_t *mac_addr
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** ***************************************************************************/
uint32_t sec_hal_rt_mac_address_get(
		uint32_t user_index_in,
		sec_hal_mac_address_t *user_mac_addr_out)
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

	SEC_HAL_TRACE_ENTRY

	if (NULL == user_mac_addr_out)
	{
		SEC_HAL_TRACE_EXIT_INFO("!!null user_mac_addr_out ptr, aborting!!")
		return SEC_HAL_RES_PARAM_ERROR;
	}

	/* allocate memory, from RAM, for msgs to be sent to TZ */
	msg_data_size = sec_msg_param_size(sizeof(uint32_t))*2;
	in_msg = sec_msg_alloc(
				&in_handle,
				msg_data_size,
				SEC_MSG_OBJECT_ID_NONE,
				0,
				SEC_HAL_MSG_BYTE_ORDER);
	msg_data_size = sec_msg_param_size(sizeof(sec_serv_status_t)) +
				sec_msg_param_size(SEC_HAL_MAC_SIZE);
	out_msg = sec_msg_alloc(
				&out_handle,
				msg_data_size,
				SEC_MSG_OBJECT_ID_NONE,
				0,
				SEC_HAL_MSG_BYTE_ORDER);

	if (NULL == in_msg || NULL == out_msg)
	{
		SEC_HAL_TRACE("alloc failure, msg not sent!")
		sec_hal_status = SEC_HAL_RES_FAIL;
	}
	else
	{
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
		ssa_disp_status = LOCAL_DISP_FUNC(
							SEC_SERV_MAC_ADDRESS_REQUEST,
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
			SEC_HAL_TRACE("op failed, MAC not available!")
			SEC_HAL_TRACE_INT("ssa_disp_status", ssa_disp_status)
			SEC_HAL_TRACE_INT("sec_msg_status ", sec_msg_status)
			SEC_HAL_TRACE_INT("sec_serv_status", sec_serv_status)
			sec_hal_status = SEC_HAL_RES_FAIL;
		}
		else
		{
			sec_msg_status = _sec_msg_param_read(LOCAL_CPY_TO_CLIENT_FUNCPTR,
								&out_handle,
								user_mac_addr_out,
								SEC_HAL_MAC_SIZE);
			if (SEC_MSG_STATUS_OK != sec_msg_status)
			{
				SEC_HAL_TRACE_INT("read failed! sec_msg_status", sec_msg_status)
				sec_hal_status = SEC_HAL_RES_FAIL;
			}
		}
	}

	/* de-allocate msg */
	sec_msg_free(out_msg);
	sec_msg_free(in_msg);

	SEC_HAL_TRACE_EXIT
	return sec_hal_status;
}

/* ****************************************************************************
** Function name      : sec_hal_rt_imei_get
** Description        : stores device's IMEI code to the given address.
** Parameters         : IN/--- sec_hal_imei_t *imei
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** ***************************************************************************/
uint32_t sec_hal_rt_imei_get(sec_hal_imei_t *user_imei_out)
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

	SEC_HAL_TRACE_ENTRY

	if (NULL == user_imei_out)
	{
		SEC_HAL_TRACE_EXIT_INFO("!!null user_imei_out ptr, aborting!!")
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
				sec_msg_param_size(SEC_HAL_MAX_IMEI_SIZE);
	out_msg = sec_msg_alloc(
				&out_handle,
				msg_data_size,
				SEC_MSG_OBJECT_ID_NONE,
				0,
				SEC_HAL_MSG_BYTE_ORDER);

	if (NULL == in_msg || NULL == out_msg)
	{
		SEC_HAL_TRACE("alloc failure, msg not sent!")
		sec_hal_status = SEC_HAL_RES_FAIL;
	}
	else
	{
		/* write content to the input msg */
		sec_msg_param_write32(
				&in_handle,
				SEC_HAL_MAX_IMEI_SIZE,
				SEC_MSG_PARAM_ID_NONE);

		/* call dispatcher */
		ssa_disp_status = LOCAL_DISP_FUNC(
							SEC_SERV_IMEI_REQUEST,
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
			SEC_HAL_TRACE("op failed, IMEI not available!")
			SEC_HAL_TRACE_INT("ssa_disp_status", ssa_disp_status)
			SEC_HAL_TRACE_INT("sec_msg_status ", sec_msg_status)
			SEC_HAL_TRACE_INT("sec_serv_status", sec_serv_status)
			sec_hal_status = SEC_HAL_RES_FAIL;
		}
		else
		{
			sec_msg_status = _sec_msg_param_read(
								LOCAL_CPY_TO_CLIENT_FUNCPTR,
								&out_handle,
								user_imei_out,
								SEC_HAL_MAX_IMEI_SIZE);
			if (SEC_MSG_STATUS_OK != sec_msg_status)
			{
				SEC_HAL_TRACE_INT("read failed! sec_msg_status", sec_msg_status)
				sec_hal_status = SEC_HAL_RES_FAIL;
			}
		}
	}

	/* de-allocate msg */
	sec_msg_free(out_msg);
	sec_msg_free(in_msg);

	SEC_HAL_TRACE_EXIT
	return sec_hal_status;
}

/* ****************************************************************************
** Function name      : sec_hal_rt_simlock_levels_open
** Description        : opens all simlock levels with given codes.
** Parameters         : IN/--- char levels_mask
**                    : IN/--- void *unlock_codes
**                    : OUT/--- uint32_t *post_lock_level_status
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** ***************************************************************************/
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

	SEC_HAL_TRACE_ENTRY

	if (0 == user_levels_mask_in)
	{
		SEC_HAL_TRACE_EXIT_INFO("!!no levels to-be-unlocked, aborting!!")
		return SEC_HAL_RES_PARAM_ERROR;
	}
	if (NULL == user_unlock_codes_in)
	{
		SEC_HAL_TRACE_EXIT_INFO("!!null user_unlock_codes_in ptr, aborting!!")
		return SEC_HAL_RES_PARAM_ERROR;
	}
	if (NULL == user_post_lock_level_status_out)
	{
		SEC_HAL_TRACE_EXIT_INFO("!!null user_post_lock_level_status_out ptr, aborting!!")
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
		SEC_HAL_TRACE("alloc failure, msg not sent!")
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
			ssa_disp_status = LOCAL_DISP_FUNC(
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
			SEC_HAL_TRACE("param error, SIMLOCK post_status not given!")
			sec_hal_status = SEC_HAL_RES_PARAM_ERROR;
		}
		else if (SEC_ROM_RET_OK != ssa_disp_status ||
				SEC_MSG_STATUS_OK != sec_msg_status ||
				SEC_SERV_STATUS_OK != sec_serv_status)
		{
			SEC_HAL_TRACE("op failed, SIMLOCK post_status not given!")
			SEC_HAL_TRACE_INT("ssa_disp_status", ssa_disp_status)
			SEC_HAL_TRACE_INT("sec_msg_status ", sec_msg_status)
			SEC_HAL_TRACE_INT("sec_serv_status", sec_serv_status)
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
				SEC_HAL_TRACE_INT("read failed! sec_msg_status", sec_msg_status)
			sec_hal_status = SEC_HAL_RES_FAIL;
			}
		}
	}

	/* de-allocate msgs */
	sec_msg_free(out_msg);
	sec_msg_free(in_msg);

	SEC_HAL_TRACE_EXIT
	return sec_hal_status;
}

/* ****************************************************************************
** Function name      : sec_hal_rt_simlock_level_open
** Description        : opens specific simlock level with the given code.
** Parameters         : IN/--- char *unlock_code
**                    : IN/--- uint8_t lock_level
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** ***************************************************************************/
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

	SEC_HAL_TRACE_ENTRY

	if (NULL == user_unlock_code_in)
	{
		SEC_HAL_TRACE_EXIT_INFO("!!no user_unlock_code_in, aborting!!")
		return SEC_HAL_RES_PARAM_ERROR;
	}
	if (0 == user_lock_level_in)
	{
		SEC_HAL_TRACE_EXIT_INFO("!!no user_lock_level_in, aborting!!")
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
		SEC_HAL_TRACE("alloc failure, msg not sent!")
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
		ssa_disp_status = LOCAL_DISP_FUNC(
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
			SEC_HAL_TRACE("op failed, SIMLOCK open failed!")
			SEC_HAL_TRACE_INT("ssa_disp_status", ssa_disp_status)
			SEC_HAL_TRACE_INT("sec_msg_status ", sec_msg_status)
			SEC_HAL_TRACE_INT("sec_serv_status", sec_serv_status)
			sec_hal_status = SEC_HAL_RES_FAIL;
		}
	}

	/* de-allocate msgs */
	sec_msg_free(out_msg);
	sec_msg_free(in_msg);

	SEC_HAL_TRACE_EXIT
	return sec_hal_status;
}

/* ****************************************************************************
** Function name      : sec_hal_rt_simlock_level_status_get
** Description        : retrieves device simlock levels state from TZ.
** Parameters         : OUT/--- uint32_t *lock_level_status
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** ***************************************************************************/
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

	SEC_HAL_TRACE_ENTRY

	if (NULL == user_lock_level_status_out)
	{
		SEC_HAL_TRACE_EXIT_INFO("!!null user_lock_level_status_out, aborting!!")
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
		SEC_HAL_TRACE("alloc failure, msg not sent!")
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
		ssa_disp_status = LOCAL_DISP_FUNC(
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
			SEC_HAL_TRACE("op failed, SIMLOCK level status not given!")
			SEC_HAL_TRACE_INT("ssa_disp_status", ssa_disp_status)
			SEC_HAL_TRACE_INT("sec_msg_status ", sec_msg_status)
			SEC_HAL_TRACE_INT("sec_serv_status", sec_serv_status)
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
				SEC_HAL_TRACE_INT("read failed! sec_msg_status", sec_msg_status)
				sec_hal_status = SEC_HAL_RES_FAIL;
			}
		}
	}

	/* de-allocate msgs */
	sec_msg_free(out_msg);
	sec_msg_free(in_msg);

	SEC_HAL_TRACE_EXIT
	return sec_hal_status;
}

/* ****************************************************************************
** Function name      : sec_hal_rt_auth_data_size_get
** Description        : retrieves device authentication data size from TZ.
** Parameters         : IN/--- uint32_t input_data_size
**                    : IN/--- uint32_t *input_data
**                    : OUT/--- uint32_t *auth_data_size
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** ***************************************************************************/
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

	SEC_HAL_TRACE_ENTRY

	if (NULL == user_input_data_in)
	{
		SEC_HAL_TRACE_EXIT_INFO("!!null user_input_data_in, aborting!!")
		return SEC_HAL_RES_PARAM_ERROR;
	}
	if (0 == user_input_data_size_in)
	{
		SEC_HAL_TRACE_EXIT_INFO("!!zero user_input_data_size_in, aborting!!")
		return SEC_HAL_RES_PARAM_ERROR;
	}
	if (NULL == user_auth_data_size_out)
	{
		SEC_HAL_TRACE_EXIT_INFO("!!null user_auth_data_size_out, aborting!!")
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
		SEC_HAL_TRACE("alloc failure, msg not sent!")
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
		ssa_disp_status = LOCAL_DISP_FUNC(
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
			SEC_HAL_TRACE("op failed, AUTH DATA SIZE not given!")
			SEC_HAL_TRACE_INT("ssa_disp_status", ssa_disp_status)
			SEC_HAL_TRACE_INT("sec_msg_status ", sec_msg_status)
			SEC_HAL_TRACE_INT("sec_serv_status", sec_serv_status)
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
				SEC_HAL_TRACE_INT("read failed! sec_msg_status", sec_msg_status)
				sec_hal_status = SEC_HAL_RES_FAIL;
			}
		}
	}

	/* de-allocate msgs */
	sec_msg_free(out_msg);
	sec_msg_free(in_msg);

	SEC_HAL_TRACE_EXIT
	return sec_hal_status;
}

/* ****************************************************************************
** Function name      : sec_hal_rt_auth_data_get
** Description        : retrieves device authentication data from TZ side.
** Parameters         : IN/--- uint32_t input_data_size
**                    : IN/--- uint32_t *input_data
**                    : IN/--- uint32_t auth_data_size
**                    : OUT/--- uint32_t *auth_data
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** ***************************************************************************/
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
	SEC_HAL_TRACE_ENTRY

	if (NULL == user_input_data_in)
	{
		SEC_HAL_TRACE_EXIT_INFO("!!zero user_input_data_in, aborting!!")
		return SEC_HAL_RES_PARAM_ERROR;
	}
	if (0 == user_input_data_size_in)
	{
		SEC_HAL_TRACE_EXIT_INFO("!!zero user_input_data_size_in, aborting!!")
		return SEC_HAL_RES_PARAM_ERROR;
	}
	if (NULL == user_auth_data_out)
	{
		SEC_HAL_TRACE_EXIT_INFO("!!null user_auth_data_out, aborting!!")
		return SEC_HAL_RES_PARAM_ERROR;
	}
	if (0 == user_auth_data_size_in)
	{
		SEC_HAL_TRACE_EXIT_INFO("!!zero user_auth_data_size_in, aborting!!")
		return SEC_HAL_RES_PARAM_ERROR;
	}
	SEC_HAL_TRACE("user_input_data_in: 0x%x", user_input_data_in);
	SEC_HAL_TRACE("user_input_data_size_in: %d", user_input_data_size_in);

    kernel_input_data_in = kmalloc(user_input_data_size_in, GFP_KERNEL);
    copy_from_user( kernel_input_data_in, user_input_data_in, user_input_data_size_in );
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
		SEC_HAL_TRACE("alloc failure, msgs not sent!")
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
		ssa_disp_status = LOCAL_DISP_FUNC(
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
			SEC_HAL_TRACE("op failed, AUTH DATA not available!")
			SEC_HAL_TRACE_INT("ssa_disp_status", ssa_disp_status)
			SEC_HAL_TRACE_INT("sec_msg_status ", sec_msg_status)
			SEC_HAL_TRACE_INT("sec_serv_status", sec_serv_status)
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
				SEC_HAL_TRACE_INT("read failed! sec_msg_status", sec_msg_status)
				sec_hal_status = SEC_HAL_RES_FAIL;
			}
		}
	}

	/* de-allocate msgs */
	sec_msg_free(out_msg);
	sec_msg_free(in_msg);
    kfree(kernel_input_data_in);
	SEC_HAL_TRACE_EXIT
	return sec_hal_status;
}

/* ****************************************************************************
** Function name      : sec_hal_rt_periodic_integrity_check
** Description        : makes a periodic check of SW cert protected files.
** Parameters         : OUT/--- uint32_t *sec_exp_time, timeout for next period
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** ***************************************************************************/
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

	SEC_HAL_TRACE_ENTRY

	if (NULL == sec_exp_time)
	{
		SEC_HAL_TRACE_EXIT_INFO("!!null sec_exp_time, aborting!!")
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
		SEC_HAL_TRACE("alloc failure, msgs not sent!")
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
		ssa_disp_status = LOCAL_DISP_FUNC(
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
			SEC_HAL_TRACE("op failed, INTEGRITY not confirmed!")
			SEC_HAL_TRACE_INT("ssa_disp_status", ssa_disp_status)
			SEC_HAL_TRACE_INT("sec_msg_status ", sec_msg_status)
			SEC_HAL_TRACE_INT("sec_serv_status", sec_serv_status)
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
				SEC_HAL_TRACE_INT("read failed! sec_msg_status", sec_msg_status)
				sec_hal_status = SEC_HAL_RES_FAIL;
			}
		}
	}

	/* de-allocate msgs */
	sec_msg_free(out_msg);
	sec_msg_free(in_msg);

	SEC_HAL_TRACE_EXIT
	return sec_hal_status;
}

/* ****************************************************************************
** Function name      : sec_hal_rt_selftest
** Description        : check if selftest was a success, or not.
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** ***************************************************************************/
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

	SEC_HAL_TRACE_ENTRY

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
		SEC_HAL_TRACE("alloc failure, msgs not sent!")
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
		ssa_disp_status = LOCAL_DISP_FUNC(
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
			SEC_HAL_TRACE("SELFTEST failed!")
			SEC_HAL_TRACE_INT("ssa_disp_status", ssa_disp_status)
			SEC_HAL_TRACE_INT("sec_msg_status ", sec_msg_status)
			SEC_HAL_TRACE_INT("sec_serv_status", sec_serv_status)
			sec_hal_status = SEC_HAL_RES_FAIL;
		}
	}

	/* de-allocate msgs */
	sec_msg_free(out_msg);
	sec_msg_free(in_msg);

	SEC_HAL_TRACE_EXIT
	return sec_hal_status;
}


/* ****************************************************************************
** Function name      : sec_hal_rt_public_cc42_key_init
** Description        : transmit public cc42 key init event to TZ.
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** ***************************************************************************/
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

	SEC_HAL_TRACE_ENTRY

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
		SEC_HAL_TRACE("alloc failure, msgs not sent!")
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
		ssa_disp_status = LOCAL_DISP_FUNC(
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
			SEC_HAL_TRACE("CC42 KEY INIT failed!")
			SEC_HAL_TRACE_INT("ssa_disp_status", ssa_disp_status)
			SEC_HAL_TRACE_INT("sec_msg_status ", sec_msg_status)
			SEC_HAL_TRACE_INT("sec_serv_status", sec_serv_status)
			sec_hal_status = SEC_HAL_RES_FAIL;
		}
	}

	/* de-allocate msgs */
	sec_msg_free(out_msg);
	sec_msg_free(in_msg);

	SEC_HAL_TRACE_EXIT
	return sec_hal_status;
}

/* ****************************************************************************
** Function name      : sec_hal_rt_a3sp_state_info
** Description        : transmit&receives public cc42 key init event to TZ.
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** ***************************************************************************/
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

	SEC_HAL_TRACE_ENTRY

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
		SEC_HAL_TRACE("alloc failure, msgs not sent!")
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
		ssa_disp_status = LOCAL_DISP_FUNC(
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
			SEC_HAL_TRACE("A3SP STATE INFO failed!")
			SEC_HAL_TRACE_INT("ssa_disp_status", ssa_disp_status)
			SEC_HAL_TRACE_INT("sec_msg_status ", sec_msg_status)
			SEC_HAL_TRACE_INT("sec_serv_status", sec_serv_status)
			sec_hal_status = SEC_HAL_RES_FAIL;
		}
		else
		{
			sec_msg_status = sec_msg_param_read32(&out_handle, virt_allowed_out);
			if (SEC_MSG_STATUS_OK != sec_msg_status)
			{
				SEC_HAL_TRACE_INT("read failed! sec_msg_status", sec_msg_status)
				sec_hal_status = SEC_HAL_RES_FAIL;
			}
		}
	}

	/* de-allocate msgs */
	sec_msg_free(out_msg);
	sec_msg_free(in_msg);

	SEC_HAL_TRACE_EXIT
	return sec_hal_status;
}

/* ****************************************************************************
** Function name      : sec_hal_rt_memcpy
** Description        : transmit memcpy request to TZ.
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** ***************************************************************************/
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

	SEC_HAL_TRACE_ENTRY

	if (NULL == phys_src_in)
	{
		SEC_HAL_TRACE_EXIT_INFO("!!null phys_src, aborting!!")
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
		SEC_HAL_TRACE("alloc failure, msgs not sent!")
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
		ssa_disp_status = LOCAL_DISP_FUNC(
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
			SEC_HAL_TRACE("MEMCPY failed!")
			SEC_HAL_TRACE_INT("ssa_disp_status", ssa_disp_status)
			SEC_HAL_TRACE_INT("sec_msg_status ", sec_msg_status)
			SEC_HAL_TRACE_INT("sec_serv_status", sec_serv_status)
			sec_hal_status = SEC_HAL_RES_FAIL;
		}
	}

	/* de-allocate msgs */
	sec_msg_free(out_msg);
	sec_msg_free(in_msg);

	SEC_HAL_TRACE_EXIT
	return sec_hal_status;
}

/* ****************************************************************************
** Function name      : sec_hal_rt_multicore_enable
** Description        : call SEC_SERV_MULTICORE_ENABLE from secure side
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** ***************************************************************************/
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

    SEC_HAL_TRACE_ENTRY

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
        SEC_HAL_TRACE("alloc failure, msgs not sent!")
        sec_hal_status = SEC_HAL_RES_FAIL;
    }
    else
    {
        /* write content to the input msg */
        sec_msg_param_write32(&in_handle,
                              sizeof(uint32_t),
                              SEC_MSG_PARAM_ID_NONE);

        /* call dispatcher */
        ssa_disp_status = LOCAL_DISP_FUNC(
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
            SEC_HAL_TRACE("multicore enable failed!")
            SEC_HAL_TRACE_INT("ssa_disp_status", ssa_disp_status)
            SEC_HAL_TRACE_INT("sec_msg_status ", sec_msg_status)
            SEC_HAL_TRACE_INT("sec_serv_status", sec_serv_status)
            sec_hal_status = SEC_HAL_RES_FAIL;
        }
    }

    /* de-allocate msgs */
    sec_msg_free(out_msg);
    sec_msg_free(in_msg);

    SEC_HAL_TRACE_EXIT
    return sec_hal_status;
}

/* ******************************** END ************************************ */
