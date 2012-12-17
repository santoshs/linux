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
#include "sec_hal_tee.h"
#include "sec_hal_rt_trace.h"
#include "sec_serv_api.h"
#include "sec_dispatch.h"
#include "sec_msg.h"

#ifndef SEC_HAL_TEST_ISOLATION
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#else
#include <string.h>
#endif /* SEC_HAL_TEST_ISOLATION */


/* ***************** MACROS, CONSTANTS, COMPILATION FLAGS **************** */
#ifdef LOCAL_DISP_FUNC
#error !!Local dispatcher function already defined!!
#else /* LOCAL_DISP_FUNC */
#define LOCAL_DISP_FUNC pub2sec_dispatcher
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


#if (defined LOCAL_CPY_TO_CLIENT_FUNC || defined LOCAL_CPY_TO_CLIENT_FUNCPTR)
#error !!copy to client already defined, aborting!!
#else /* LOCAL_CPY_TO_CLIENT_FUNC */
#ifndef SEC_HAL_TEST_ISOLATION
/* **************************************************************************
** Function name      : copy_to_userspace
** Description        : copy content to userspace, to user page.
** Parameters         :
** Return value       : number of bytes left uncopied.
** *************************************************************************/
static unsigned long copy_to_userspace(
		void* dst,
		const void* src,
		unsigned long sz)
{
	return copy_to_user((void __user *)dst, src, sz);
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

/* From sec_hal_dev.c*/
unsigned long sec_hal_memory_tablewalk(unsigned long virt_addr);

uint32_t sec_hal_tee_initialize_context( const char* name, TEEC_Context* kernel_context)
    {
	uint32_t sec_hal_status = TEEC_SUCCESS;
	uint32_t sec_msg_status = SEC_MSG_STATUS_OK;
	uint32_t ssa_disp_status = 0x00;
	uint32_t sec_serv_status = 0x00;
	uint16_t msg_data_size;

	sec_msg_t *in_msg = NULL;
	sec_msg_t *out_msg = NULL;
	sec_msg_handle_t in_handle;
	sec_msg_handle_t out_handle;
    uint32_t* kernel_name = NULL;
    uint32_t name_length;

	SEC_HAL_TRACE_ENTRY();

    if( name != NULL)
        {
        name_length = strlen(name) + 1;
        kernel_name = kmalloc(name_length, GFP_KERNEL);
        copy_from_user( kernel_name, name, name_length );
        SEC_HAL_TRACE("kernel_name: %s", kernel_name);
        }


	/* allocate memory, from ICRAM, for msgs to be sent to TZ */
	msg_data_size = (sec_msg_param_size(sizeof(uint32_t)))*3;
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
		sec_hal_status = TEEC_ERROR_GENERIC;
	}
	else
	{
		/* write content to the input msg */
        /* 1 ) string size */
        /* 2 ) name */
        /* 3 ) context */
		sec_msg_param_write32(
				&in_handle,
				name_length,
				SEC_MSG_PARAM_ID_NONE);

		sec_msg_param_write32(
				&in_handle,
				kernel_name != NULL ? (uint32_t)virt_to_phys(kernel_name) : NULL,
				SEC_MSG_PARAM_ID_NONE);

		sec_msg_param_write32(
				&in_handle,
				kernel_context != NULL ? (uint32_t)virt_to_phys(kernel_context) : NULL,
				SEC_MSG_PARAM_ID_NONE);

		/* call dispatcher */
		ssa_disp_status = LOCAL_DISP_FUNC(
							SEC_SERV_TEEC_InitializeContext,
							LOCAL_DEFAULT_DISP_FLAGS,
							LOCAL_DEFAULT_DISP_SPARE_PARAM,
							SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
							SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));

		/* interpret the response */
		sec_msg_status = sec_msg_param_read32(&out_handle, &sec_serv_status);
		if (SEC_ROM_RET_OK != ssa_disp_status ||
			SEC_MSG_STATUS_OK != sec_msg_status ||
			TEEC_SUCCESS != sec_serv_status)
		{
			SEC_HAL_TRACE("op failed, initialize tee context failed!");
			SEC_HAL_TRACE("ssa_disp_status 0x%x", ssa_disp_status);
			SEC_HAL_TRACE("sec_msg_status 0x%x", sec_msg_status);
			SEC_HAL_TRACE("sec_serv_status 0x%x", sec_serv_status);
			sec_hal_status = sec_serv_status;
		}
	}

	/* de-allocate msgs */
	sec_msg_free(out_msg);
	sec_msg_free(in_msg);

    if(kernel_name !=NULL)
        {
        kfree(kernel_name);
        }
	SEC_HAL_TRACE_EXIT();
    return sec_hal_status;
    }

uint32_t sec_hal_tee_finalize_context( TEEC_Context* kernel_context)
    {
	uint32_t sec_hal_status = TEEC_SUCCESS;
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
	msg_data_size = (sec_msg_param_size(sizeof(uint32_t)));
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
		sec_hal_status = TEEC_ERROR_GENERIC;
	}
	else
	{
		/* ensure that certificate is fully in work RAM memory */
		SEC_HAL_MEM_CACHE_CLEAN_FUNC(kernel_context, sizeof(TEEC_Context));

		/* write content to the input msg */
        /* 1 ) context */
		sec_msg_param_write32(
				&in_handle,
				kernel_context != NULL ? (uint32_t)virt_to_phys(kernel_context) : NULL,
				SEC_MSG_PARAM_ID_NONE);

		/* call dispatcher */
		ssa_disp_status = LOCAL_DISP_FUNC(
							SEC_SERV_TEEC_FinalizeContext,
							LOCAL_DEFAULT_DISP_FLAGS,
							LOCAL_DEFAULT_DISP_SPARE_PARAM,
							SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
							SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));

		/* interpret the response */
		sec_msg_status = sec_msg_param_read32(&out_handle, &sec_serv_status);
		if (SEC_ROM_RET_OK != ssa_disp_status ||
			TEEC_SUCCESS != sec_serv_status)
		{
			SEC_HAL_TRACE("op failed, finalize tee context failed!");
			SEC_HAL_TRACE("ssa_disp_status 0x%x", ssa_disp_status);
			SEC_HAL_TRACE("sec_serv_status 0x%x", sec_serv_status);
			sec_hal_status = sec_serv_status;
		}
	}

	/* de-allocate msgs */
	sec_msg_free(out_msg);
	sec_msg_free(in_msg);


	SEC_HAL_TRACE_EXIT();
    return sec_hal_status;
    }

uint32_t sec_hal_tee_open_session( TEEC_Context* kernel_context,
                          TEEC_Session* kernel_session,
                          const TEEC_UUID* destination,
                          uint32_t connectionMethod,
                          const void* connectionData,
                          TEEC_Operation* operation,
                          uint32_t* returnOrigin)
    {
	uint32_t sec_hal_status = TEEC_SUCCESS;
	uint32_t sec_msg_status = SEC_MSG_STATUS_OK;
	uint32_t ssa_disp_status = 0x00;
	uint32_t sec_serv_status = 0x00;
	uint16_t msg_data_size;

	sec_msg_t *in_msg = NULL;
	sec_msg_t *out_msg = NULL;
	sec_msg_handle_t in_handle;
	sec_msg_handle_t out_handle;

    TEEC_UUID*      kernel_destination = NULL;
    uint32_t*       kernel_connectionData = NULL;
    TEEC_Operation* kernel_operation = NULL;
    uint32_t*       kernel_returnOrigin = NULL;

	SEC_HAL_TRACE_ENTRY();

    if( destination==NULL )
        {
        SEC_HAL_TRACE("Error: invalid parameters, destination==NULL");
        SEC_HAL_TRACE_EXIT();
        return SEC_HAL_RES_FAIL;
        }

	SEC_HAL_TRACE("kernel_context: 0x%x", kernel_context);

    kernel_destination = kmalloc(sizeof(TEEC_UUID), GFP_KERNEL);
    copy_from_user( kernel_destination, destination, sizeof(TEEC_UUID) );
	SEC_HAL_TRACE("kernel_destination: 0x%x", kernel_destination);

    if(connectionMethod != TEEC_LOGIN_USER)
        {
        if(connectionData !=NULL)
            {
            kernel_connectionData = kmalloc(sizeof(uint32_t), GFP_KERNEL);
            copy_from_user( kernel_connectionData, connectionData, sizeof(uint32_t) );
            SEC_HAL_TRACE("kernel_connectionData: 0x%x", kernel_connectionData);
            SEC_HAL_MEM_CACHE_CLEAN_FUNC(connectionData, sizeof(uint32_t));
            }
        }

    if(operation !=NULL)
        {
        kernel_operation = kmalloc(sizeof(TEEC_Operation), GFP_KERNEL);
        copy_from_user( kernel_operation, operation, sizeof(TEEC_Operation) );
        SEC_HAL_TRACE("kernel_operation: 0x%x", kernel_operation);
        sec_hal_tee_convert_memrefs(NULL,kernel_operation,1);
        SEC_HAL_MEM_CACHE_CLEAN_FUNC(operation, sizeof(TEEC_Operation));
        }


    if(returnOrigin !=NULL)
        {
        kernel_returnOrigin = kmalloc(sizeof(uint32_t), GFP_KERNEL);
        copy_from_user( kernel_returnOrigin, returnOrigin, sizeof(uint32_t));
        SEC_HAL_TRACE("kernel_returnOrigin: 0x%x", kernel_returnOrigin);
        SEC_HAL_MEM_CACHE_CLEAN_FUNC(returnOrigin, sizeof(uint32_t));
        }


	/* allocate memory, from ICRAM, for msgs to be sent to TZ */
	msg_data_size = (sec_msg_param_size(sizeof(uint32_t))) * 7;
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
		sec_hal_status = TEEC_ERROR_GENERIC;
	}
	else
	{
		/* ensure that certificate is fully in work RAM memory */
		SEC_HAL_MEM_CACHE_CLEAN_FUNC(kernel_context, sizeof(TEEC_Context));
		SEC_HAL_MEM_CACHE_CLEAN_FUNC(kernel_session, sizeof(TEEC_Session));
		SEC_HAL_MEM_CACHE_CLEAN_FUNC(kernel_destination, sizeof(TEEC_UUID));


		/* write content to the input msg */
        /* 1 ) context */
        /* 2 ) session */
        /* 3 ) destination */
        /* 4 ) connectionMethod */
        /* 5 ) connectionData (NULL) */
        /* 6 ) operation (NULL) */
        /* 7 ) returnOrigin (NULL) */


		sec_msg_param_write32(
				&in_handle,
				kernel_context != NULL ? (uint32_t)virt_to_phys(kernel_context) : NULL,
				SEC_MSG_PARAM_ID_NONE);

		sec_msg_param_write32(
				&in_handle,
				kernel_session != NULL ? (uint32_t)virt_to_phys(kernel_session) : NULL,
				SEC_MSG_PARAM_ID_NONE);

		sec_msg_param_write32(
				&in_handle,
				kernel_destination != NULL ? (uint32_t)virt_to_phys(kernel_destination) : NULL,
				SEC_MSG_PARAM_ID_NONE);

		sec_msg_param_write32(
				&in_handle,
				connectionMethod,
				SEC_MSG_PARAM_ID_NONE);

		sec_msg_param_write32(
				&in_handle,
				kernel_connectionData != NULL ? (uint32_t)virt_to_phys(kernel_connectionData) : NULL,
				SEC_MSG_PARAM_ID_NONE);

		sec_msg_param_write32(
				&in_handle,
				kernel_operation != NULL ? (uint32_t)virt_to_phys(kernel_operation) : NULL,
				SEC_MSG_PARAM_ID_NONE);

		sec_msg_param_write32(
				&in_handle,
				kernel_returnOrigin != NULL ? (uint32_t)virt_to_phys(kernel_returnOrigin) : NULL,
				SEC_MSG_PARAM_ID_NONE);

		/* call dispatcher */
		ssa_disp_status = LOCAL_DISP_FUNC(
							SEC_SERV_TEEC_OpenSession,
							LOCAL_DEFAULT_DISP_FLAGS,
							LOCAL_DEFAULT_DISP_SPARE_PARAM,
							SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
							SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));

		/* interpret the response */
		sec_msg_status = sec_msg_param_read32(&out_handle, &sec_serv_status);
		if (SEC_ROM_RET_OK != ssa_disp_status ||
			SEC_MSG_STATUS_OK != sec_msg_status ||
			TEEC_SUCCESS != sec_serv_status)
		{
			SEC_HAL_TRACE("op failed, open tee session failed!");
			SEC_HAL_TRACE("ssa_disp_status 0x%x", ssa_disp_status);
			SEC_HAL_TRACE("sec_msg_status 0x%x", sec_msg_status);
			SEC_HAL_TRACE("sec_serv_status 0x%x", sec_serv_status);
			sec_hal_status = sec_serv_status;
		}
	}

	/* de-allocate msgs */
	sec_msg_free(out_msg);
	sec_msg_free(in_msg);

    if(kernel_destination !=NULL)
        {
        kfree(kernel_destination);
        }
    if(kernel_connectionData !=NULL)
        {
        kfree(kernel_connectionData);
        }
    if(kernel_operation !=NULL)
        {
        sec_hal_tee_convert_memrefs(operation,kernel_operation,0);
        /*Is copy to user needed?? NO!*/
        kfree(kernel_operation);
        }
    if(kernel_returnOrigin !=NULL)
        {
        kfree(kernel_returnOrigin);
        }

	SEC_HAL_TRACE_EXIT();
    return sec_hal_status;
    }

uint32_t  sec_hal_tee_close_session( TEEC_Session* kernel_session )
    {
	uint32_t sec_hal_status = TEEC_SUCCESS;
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
	msg_data_size = (sec_msg_param_size(sizeof(uint32_t)));
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
		sec_hal_status = TEEC_ERROR_GENERIC;
	}
	else
	{
		/* write content to the input msg */
        /* 1 ) session */

		sec_msg_param_write32(
				&in_handle,
				kernel_session != NULL ? (uint32_t)virt_to_phys(kernel_session) : NULL,
				SEC_MSG_PARAM_ID_NONE);

		/* call dispatcher */
		ssa_disp_status = LOCAL_DISP_FUNC(
							SEC_SERV_TEEC_CloseSession,
							LOCAL_DEFAULT_DISP_FLAGS,
							LOCAL_DEFAULT_DISP_SPARE_PARAM,
							SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
							SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));

		/* interpret the response */
		sec_msg_status = sec_msg_param_read32(&out_handle, &sec_serv_status);
		if (SEC_ROM_RET_OK != ssa_disp_status ||
			TEEC_SUCCESS != sec_serv_status)
		{
			SEC_HAL_TRACE("op failed, close tee session failed!");
			SEC_HAL_TRACE("ssa_disp_status 0x%x", ssa_disp_status);
			SEC_HAL_TRACE("sec_serv_status 0x%x", sec_serv_status);
			sec_hal_status = sec_serv_status;
		}
	}

	/* de-allocate msgs */
	sec_msg_free(out_msg);
	sec_msg_free(in_msg);

	SEC_HAL_TRACE_EXIT();
    return sec_hal_status;
    }

uint32_t sec_hal_tee_invoke_command( TEEC_Session* kernel_session,
                        uint32_t commandID,
                        TEEC_Operation* operation,
                        uint32_t* returnOrigin)
    {
	uint32_t sec_hal_status = TEEC_SUCCESS;
	uint32_t sec_msg_status = SEC_MSG_STATUS_OK;
	uint32_t ssa_disp_status = 0x00;
	uint32_t sec_serv_status = 0x00;
	uint16_t msg_data_size;

	sec_msg_t *in_msg = NULL;
	sec_msg_t *out_msg = NULL;
	sec_msg_handle_t in_handle;
	sec_msg_handle_t out_handle;
    TEEC_Operation* kernel_operation = NULL;
    uint32_t*       kernel_returnOrigin = NULL;

	SEC_HAL_TRACE_ENTRY();

    if(operation !=NULL)
        {
        kernel_operation = kmalloc(sizeof(TEEC_Operation), GFP_KERNEL);
        copy_from_user( kernel_operation, operation, sizeof(TEEC_Operation) );
        SEC_HAL_TRACE("kernel_operation: 0x%x", kernel_operation);
        sec_hal_tee_convert_memrefs(NULL,kernel_operation,1);
        SEC_HAL_MEM_CACHE_CLEAN_FUNC(kernel_operation, sizeof(TEEC_Operation));
        }

    if(returnOrigin !=NULL)
        {
        kernel_returnOrigin = kmalloc(sizeof(uint32_t), GFP_KERNEL);
        copy_from_user( kernel_returnOrigin, returnOrigin, sizeof(uint32_t));
        SEC_HAL_TRACE("kernel_returnOrigin: 0x%x", kernel_returnOrigin);
        SEC_HAL_MEM_CACHE_CLEAN_FUNC(returnOrigin, sizeof(uint32_t));
        }


	/* allocate memory, from ICRAM, for msgs to be sent to TZ */
	msg_data_size = (sec_msg_param_size(sizeof(uint32_t))) * 4;
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
		sec_hal_status = TEEC_ERROR_GENERIC;
	}
	else
	{

		/* write content to the input msg */
        /* 1 ) session */
        /* 2 ) commandID */
        /* 3 ) operation */
        /* 4 ) returnOrigin */


		sec_msg_param_write32(
				&in_handle,
				kernel_session != NULL ? (uint32_t)virt_to_phys(kernel_session) : NULL,
				SEC_MSG_PARAM_ID_NONE);

		sec_msg_param_write32(
				&in_handle,
				commandID,
				SEC_MSG_PARAM_ID_NONE);

		sec_msg_param_write32(
				&in_handle,
				kernel_operation != NULL ? (uint32_t)virt_to_phys(kernel_operation) : NULL,
				SEC_MSG_PARAM_ID_NONE);

		sec_msg_param_write32(
				&in_handle,
				kernel_returnOrigin != NULL ? (uint32_t)virt_to_phys(kernel_returnOrigin) : NULL,
				SEC_MSG_PARAM_ID_NONE);

		/* call dispatcher */
		ssa_disp_status = LOCAL_DISP_FUNC(
							SEC_SERV_TEEC_InvokeCommand,
							LOCAL_DEFAULT_DISP_FLAGS,
							LOCAL_DEFAULT_DISP_SPARE_PARAM,
							SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
							SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));

		/* interpret the response */
		sec_msg_status = sec_msg_param_read32(&out_handle, &sec_serv_status);
		if (SEC_ROM_RET_OK != ssa_disp_status ||
			SEC_MSG_STATUS_OK != sec_msg_status ||
			TEEC_SUCCESS != sec_serv_status)
		{
			SEC_HAL_TRACE("op failed, invoke command failed!");
			SEC_HAL_TRACE("ssa_disp_status 0x%x", ssa_disp_status);
			SEC_HAL_TRACE("sec_msg_status 0x%x", sec_msg_status);
			SEC_HAL_TRACE("sec_serv_status 0x%x", sec_serv_status);
			sec_hal_status = sec_serv_status;
		}
	}

	/* de-allocate msgs */
	sec_msg_free(out_msg);
	sec_msg_free(in_msg);

#if 0
    copy_to_user( session, kernel_session, sizeof(TEEC_Session) );
#endif

    /* TBD needs change, update only the value parameters */
#if 0
    copy_to_user( operation, kernel_operation, sizeof(TEEC_Operation) );
#endif

    if(kernel_operation !=NULL)
        {
        sec_hal_tee_convert_memrefs(operation,kernel_operation,0);
        /*Is copy to user needed??*/
        kfree(kernel_operation);
        }
    if(kernel_returnOrigin !=NULL)
        {
        kfree(kernel_returnOrigin);
        copy_to_user( returnOrigin, kernel_returnOrigin, sizeof(uint32_t) );
        }

	SEC_HAL_TRACE_EXIT();
    return sec_hal_status;
    }


uint32_t sec_hal_tee_register_shared_memory_area( TEEC_Context* kernel_context, TEEC_SharedMemory* kernel_shmem)
    {
	uint32_t sec_hal_status = TEEC_SUCCESS;
	uint32_t sec_msg_status = SEC_MSG_STATUS_OK;
	uint32_t ssa_disp_status = 0x00;
	uint32_t sec_serv_status = 0x00;
	uint16_t msg_data_size;

	sec_msg_t *in_msg = NULL;
	sec_msg_t *out_msg = NULL;
	sec_msg_handle_t in_handle;
	sec_msg_handle_t out_handle;
	SEC_HAL_TRACE_ENTRY();

    if(kernel_context ==NULL)
        {
        SEC_HAL_TRACE("context is null");
        sec_hal_status = TEEC_ERROR_BAD_PARAMETERS;
        return sec_hal_status;
        }

    if(kernel_shmem ==NULL)
        {
        SEC_HAL_TRACE("kernel_shmem is null");
        sec_hal_status = TEEC_ERROR_BAD_PARAMETERS;
        return sec_hal_status;
        }

	/* allocate memory, from ICRAM, for msgs to be sent to TZ */
	msg_data_size = (sec_msg_param_size(sizeof(uint32_t)))*2;
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
		sec_hal_status = TEEC_ERROR_GENERIC;
	}
	else
	{
		/* write content to the input msg */
        /* 1 ) context */
        /* 2 ) shared memory */

		SEC_HAL_MEM_CACHE_CLEAN_FUNC(kernel_context, sizeof(TEEC_Context));
		SEC_HAL_MEM_CACHE_CLEAN_FUNC(kernel_shmem, sizeof(TEEC_SharedMemory));

		sec_msg_param_write32(
				&in_handle,
				kernel_context != NULL ? (uint32_t)virt_to_phys(kernel_context) : NULL,
				SEC_MSG_PARAM_ID_NONE);

		sec_msg_param_write32(
				&in_handle,
				kernel_shmem != NULL ? (uint32_t)virt_to_phys(kernel_shmem) : NULL,
				SEC_MSG_PARAM_ID_NONE);

		/* call dispatcher */
		ssa_disp_status = LOCAL_DISP_FUNC(
							SEC_SERV_TEEC_RegisterSharedMemory,
							LOCAL_DEFAULT_DISP_FLAGS,
							LOCAL_DEFAULT_DISP_SPARE_PARAM,
							SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
							SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));

		/* interpret the response */
		sec_msg_status = sec_msg_param_read32(&out_handle, &sec_serv_status);
		if (SEC_ROM_RET_OK != ssa_disp_status ||
			SEC_MSG_STATUS_OK != sec_msg_status ||
			TEEC_SUCCESS != sec_serv_status)
		{
			SEC_HAL_TRACE("op failed, registering shared memory address failed!");
			SEC_HAL_TRACE("ssa_disp_status 0x%x", ssa_disp_status);
			SEC_HAL_TRACE("sec_msg_status 0x%x", sec_msg_status);
			SEC_HAL_TRACE("sec_serv_status 0x%x", sec_serv_status);
			sec_hal_status = sec_serv_status;
		}
	}

	/* de-allocate msgs */
	sec_msg_free(out_msg);
	sec_msg_free(in_msg);

    SEC_HAL_TRACE_EXIT();
    return sec_hal_status;
    }


uint32_t sec_hal_tee_release_shared_memory_area(TEEC_SharedMemory* kernel_shmem)
    {
	uint32_t sec_hal_status = TEEC_SUCCESS;
	uint32_t ssa_disp_status = 0x00;
	uint32_t sec_serv_status = 0x00;
	uint16_t msg_data_size;

	sec_msg_t *in_msg = NULL;
	sec_msg_t *out_msg = NULL;
	sec_msg_handle_t in_handle;
	sec_msg_handle_t out_handle;
	SEC_HAL_TRACE_ENTRY();


	/* allocate memory, from ICRAM, for msgs to be sent to TZ */
	msg_data_size = (sec_msg_param_size(sizeof(uint32_t)))*1;
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
		sec_hal_status = TEEC_ERROR_GENERIC;
	}
	else
	{
		/* write content to the input msg */
        /* 1 ) shared memory pointer */

		SEC_HAL_MEM_CACHE_CLEAN_FUNC(kernel_shmem, sizeof(TEEC_SharedMemory));

		sec_msg_param_write32(
				&in_handle,
				kernel_shmem != NULL ? (uint32_t)virt_to_phys(kernel_shmem) : NULL,
				SEC_MSG_PARAM_ID_NONE);

		/* call dispatcher */
		ssa_disp_status = LOCAL_DISP_FUNC(
							SEC_SERV_TEEC_ReleaseSharedMemory,
							LOCAL_DEFAULT_DISP_FLAGS,
							LOCAL_DEFAULT_DISP_SPARE_PARAM,
							SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
							SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));

		/* interpret the response */
		if (SEC_ROM_RET_OK != ssa_disp_status ||
			TEEC_SUCCESS != sec_serv_status)
		{
			SEC_HAL_TRACE("op failed, releasing shared memory buffer 0x%x failed!",kernel_shmem->buffer);
			SEC_HAL_TRACE("ssa_disp_status 0x%x", ssa_disp_status);
			SEC_HAL_TRACE("sec_serv_status 0x%x", sec_serv_status);
			sec_hal_status = sec_serv_status;
		}
	}

	/* de-allocate msgs */
	sec_msg_free(out_msg);
	sec_msg_free(in_msg);

    SEC_HAL_TRACE_EXIT();
    return sec_hal_status;
    }


uint32_t sec_hal_tee_allocate_TA_memory_workaround()
    {
	uint32_t sec_hal_status = TEEC_SUCCESS;
	uint32_t sec_msg_status = SEC_MSG_STATUS_OK;
	uint32_t ssa_disp_status = 0x00;
	uint32_t sec_serv_status = 0x00;
	uint16_t msg_data_size;

	sec_msg_t *in_msg = NULL;
	sec_msg_t *out_msg = NULL;
	sec_msg_handle_t in_handle;
	sec_msg_handle_t out_handle;


    uint32_t size = 1024*1024*3;
    uint32_t * memarea;
    unsigned long physical;
    int i;

    SEC_HAL_TRACE_ENTRY();

    memarea=kmalloc(size, GFP_KERNEL);

#if 0
    for (i=0; i<(size/sizeof(int)); i+=2)
        {
        memarea[i]=(0xdead<<16) +i;
        memarea[i+1]=(0xbeef<<16) + i;
        }

    for (i=0; i<size/sizeof(int); i+=1)
        {
        SEC_HAL_TRACE("0x%08x", memarea[i]);
        }
#endif

    physical=(unsigned long)virt_to_phys(memarea);
	/* allocate memory, from ICRAM, for msgs to be sent to TZ */
	msg_data_size = (sec_msg_param_size(sizeof(uint32_t)))*2;
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
		sec_hal_status = TEEC_ERROR_GENERIC;
	}
	else
	{
		/* write content to the input msg */
        /* 1 ) physical address */
        /* 2 ) memory size */

		sec_msg_param_write32(
				&in_handle,
				(uint32_t)physical,
				SEC_MSG_PARAM_ID_NONE);

		sec_msg_param_write32(
				&in_handle,
				size,
				SEC_MSG_PARAM_ID_NONE);


		/* call dispatcher */
		ssa_disp_status = LOCAL_DISP_FUNC(
							SEC_SERV_TEE_DMEM_SDRAM_WORKAROUND,
							LOCAL_DEFAULT_DISP_FLAGS,
							LOCAL_DEFAULT_DISP_SPARE_PARAM,
							SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
							SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));

		/* interpret the response */
		sec_msg_status = sec_msg_param_read32(&out_handle, &sec_serv_status);
		if (SEC_ROM_RET_OK != ssa_disp_status ||
			TEEC_SUCCESS != sec_serv_status)
		{
			SEC_HAL_TRACE("op failed, allocate TA memory workaround failed, physical 0x%x",physical);
			SEC_HAL_TRACE("ssa_disp_status 0x%x", ssa_disp_status);
			SEC_HAL_TRACE("sec_serv_status 0x%x", sec_serv_status);
			sec_hal_status = sec_serv_status;
		}
	}

	/* de-allocate msgs */
	sec_msg_free(out_msg);
	sec_msg_free(in_msg);

    SEC_HAL_TRACE_EXIT();
    return sec_hal_status;
    }

#if 0
uint32_t sec_hal_tee_get_param_type(uint32_t index, uint32_t types)
    {
	SEC_HAL_TRACE_ENTRY();
    SEC_HAL_TRACE("index: 0x%x", index);
    SEC_HAL_TRACE("types: 0x%x", types);
	SEC_HAL_TRACE_EXIT();
#if 0
    if(index==0)
        {
        return ((types >>24) & 0x000000FF);
        }
    if(index==1)
        {
        return ((types >>16) & 0x000000FF);
        }
    if(index==2)
        {
        return ((types >>8) & 0x000000FF);
        }
    if(index==3)
        {
        return ((types & 0x000000FF);
        }
#endif

    return ((types>>(24 - (8 * index))) & 0x000000FF);
    }
#endif

uint32_t  sec_hal_tee_convert_memrefs( TEEC_Operation* user_operation,
                                       TEEC_Operation* operation,
                                       uint32_t direction )
    {
    uint32_t i;

	SEC_HAL_TRACE_ENTRY();

    SEC_HAL_TRACE("operation->paramTypes: 0x%x", operation->paramTypes);
    SEC_HAL_TRACE("direction: 0x%x", direction);

    for(i=0;i<4;i++)
        {
        uint32_t param_type;
        SEC_HAL_TRACE("i: 0x%x", i);

/*        param_type = sec_hal_tee_get_param_type(i,operation->paramTypes);*/
        param_type = TEEC_PARAM_TYPE_GET(operation->paramTypes, i);

        SEC_HAL_TRACE("param_type: 0x%x", param_type);

        if( param_type == TEEC_MEMREF_WHOLE ||
             param_type == TEEC_MEMREF_PARTIAL_INPUT ||
             param_type == TEEC_MEMREF_PARTIAL_OUTPUT ||
             param_type == TEEC_MEMREF_PARTIAL_INOUT)
            {
            if(direction == 1)
                {
                TEEC_SharedMemory *kernelSM;
                kernelSM = kmalloc(sizeof(TEEC_SharedMemory), GFP_KERNEL);
                copy_from_user( kernelSM, operation->params[i].memref.parent, sizeof(TEEC_SharedMemory) );
        		SEC_HAL_MEM_CACHE_CLEAN_FUNC(kernelSM, sizeof(TEEC_SharedMemory));
        		SEC_HAL_MEM_CACHE_CLEAN_FUNC(kernelSM->buffer, kernelSM->size);
                SEC_HAL_TRACE("operation->params[%d].memref.parent.buffer: 0x%x",  i, operation->params[i].memref.parent->buffer);
                SEC_HAL_TRACE("operation->params[%d].memref.parent: 0x%x",  i, operation->params[i].memref.parent);

                operation->params[i].memref.parent = kernelSM;
                SEC_HAL_TRACE("after copy: operation->params[%d].memref.parent: 0x%x",  i, operation->params[i].memref.parent);
                operation->params[i].memref.parent->buffer = sec_hal_memory_tablewalk(operation->params[i].memref.parent->buffer);
                SEC_HAL_TRACE("after conversion: operation->params[%d].memref.parent->buffer: 0x%x", i,  operation->params[i].memref.parent->buffer);
                operation->params[i].memref.parent =  sec_hal_memory_tablewalk(operation->params[i].memref.parent);
                SEC_HAL_TRACE("after conversion: operation->params[%d].memref.parent: 0x%x",  i, operation->params[i].memref.parent);

                }
            else
                {
#if 0
                SEC_HAL_TRACE("PHYS: operation->params[%d].memref.parent: 0x%x",  i, operation->params[i].memref.parent);
                operation->params[i].memref.parent =  phys_to_virt(operation->params[i].memref.parent);
                SEC_HAL_TRACE("VIRT: operation->params[%d].memref.parent: 0x%x",  i, operation->params[i].memref.parent);
                SEC_HAL_TRACE("PHYS:operation->params[%d].memref.parent.buffer: 0x%x",  i, operation->params[i].memref.parent->buffer);
                operation->params[i].memref.parent->buffer = phys_to_virt(operation->params[i].memref.parent->buffer);
                SEC_HAL_TRACE("VIRT:operation->params[%d].memref.parent.buffer: 0x%x",  i, operation->params[i].memref.parent->buffer);
#endif
                operation->params[i].memref.parent =  phys_to_virt(operation->params[i].memref.parent);
                SEC_HAL_TRACE("kfree for 0x%x",  i, operation->params[i].memref.parent);
                kfree(operation->params[i].memref.parent);
                }
            }
        else if( param_type == TEEC_MEMREF_TEMP_INPUT ||
                 param_type == TEEC_MEMREF_TEMP_OUTPUT ||
                 param_type == TEEC_MEMREF_TEMP_INOUT )
            {
            if(direction == 1)
                {
                SEC_HAL_TRACE("operation->params[%d].tmpref.buffer: 0x%x",  i, operation->params[i].tmpref.buffer);
                operation->params[i].tmpref.buffer = virt_to_phys(operation->params[i].tmpref.buffer);
                SEC_HAL_TRACE("after conversion: operation->params[%d].tmpref.buffer: 0x%x",  i, operation->params[i].tmpref.buffer);
                }
            else
                {
                SEC_HAL_TRACE("PHYS:operation->params[%d].tmpref.buffer: 0x%x",  i, operation->params[i].tmpref.buffer);
                operation->params[i].tmpref.buffer = phys_to_virt(operation->params[i].tmpref.buffer);
                SEC_HAL_TRACE("VIRT:operation->params[%d].tmpref.buffer: 0x%x",  i, operation->params[i].tmpref.buffer);
                }
            }
        else if( param_type == TEEC_VALUE_INPUT ||
                 param_type == TEEC_VALUE_OUTPUT ||
                 param_type == TEEC_VALUE_INOUT )
            {
            if(direction == 1)
                {
                }
            else
                {
                user_operation->params[i].value.a = operation->params[i].value.a;
                user_operation->params[i].value.b = operation->params[i].value.b;
                }
            }
        }
	SEC_HAL_TRACE_EXIT();
    }



/* ******************************** END ********************************** */
