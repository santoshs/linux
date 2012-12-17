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
#define SEC_HAL_TRACE_LOCAL_DISABLE
#include "sec_hal_rt.h"
#include "sec_hal_rt_cmn.h"
#include "sec_serv_api.h"
#include "sec_msg.h"
#include "sec_dispatch.h"
#include <linux/module.h>

#ifndef SEC_HAL_TEST_ISOLATION
#include <linux/string.h>
#include <linux/uaccess.h>
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

/* PM start */

/* Mode */
enum mode {
	SEC_HAL_INITIAL_STATE = 0,
	SEC_HAL_SUSPEND_REQ = 1,
	SEC_HAL_CORE_STANDBY_REQ = 2,
	SEC_HAL_SLEEP = 3,
	NR_MODES
};

static uint16_t msg_data_size[CONFIG_NR_CPUS] ;
static sec_msg_t *in_msg[CONFIG_NR_CPUS] = {NULL};
static sec_msg_t *out_msg[CONFIG_NR_CPUS] = {NULL};
static sec_msg_handle_t in_handle[CONFIG_NR_CPUS];
static sec_msg_handle_t out_handle[CONFIG_NR_CPUS];


uint32_t sec_hal_pm_coma_entry_init(void)
{
	uint32_t ret = SEC_HAL_RES_OK;
	uint32_t cpu;
	
	for_each_possible_cpu(cpu) {
		msg_data_size[cpu] = sec_msg_param_size(sizeof(uint32_t))*4;
		in_msg[cpu] = sec_msg_alloc(
					&in_handle[cpu],
					msg_data_size[cpu],
					SEC_MSG_OBJECT_ID_NONE,
					0,
					SEC_HAL_MSG_BYTE_ORDER);
		msg_data_size[cpu] = sec_msg_param_size(sizeof(sec_serv_status_t)) +
					sec_msg_param_size(sizeof(uint32_t));
		out_msg[cpu] = sec_msg_alloc(
					&out_handle[cpu],
					msg_data_size[cpu],
					SEC_MSG_OBJECT_ID_NONE,
					0,
					SEC_HAL_MSG_BYTE_ORDER);

		if (NULL == in_msg[cpu] || NULL == out_msg[cpu]){
			ret = SEC_HAL_RES_FAIL;
		}
	}

	return ret;

}
EXPORT_SYMBOL(sec_hal_pm_coma_entry_init);
/* PM end */

/* ****************************** CODE SECTION *************************** */

/* **************************************************************************
 * EXPORTs, Power Management
 * *************************************************************************/

/* **************************************************************************
** Function name      : sec_hal_pm_coma_entry
** Description        : transmit power control event and params to TZ.
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** *************************************************************************/
uint32_t sec_hal_pm_coma_entry (uint32_t mode, uint32_t wakeup_address,
                                uint32_t pll0, uint32_t zclk)
{
    
	uint32_t sec_hal_status = SEC_HAL_RES_OK;
    uint32_t sec_msg_status = SEC_MSG_STATUS_OK;
    uint32_t ssa_disp_status = 0x00;
    uint32_t sec_serv_status = 0x00;
	uint32_t cpu = 0x0;

	/* SEC_HAL_TRACE_ENTRY */
#if 0
    uint16_t msg_data_size;
    sec_msg_t *in_msg = NULL;
    sec_msg_t *out_msg = NULL;
    sec_msg_handle_t in_handle;
    sec_msg_handle_t out_handle;

	SEC_HAL_TRACE_ENTRY

	/* allocate memory, from ICRAM, for msgs to be sent to TZ */
	msg_data_size = sec_msg_param_size(sizeof(uint32_t))*4;
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
#endif

		cpu = smp_processor_id();
		sec_msg_open(&in_handle[cpu], in_msg[cpu]);
		sec_msg_open(&out_handle[cpu], out_msg[cpu]);
		/* write content to the input msg */
		sec_msg_param_write32(
				&in_handle[cpu],
				/* &in_handle, */
				mode,
				SEC_MSG_PARAM_ID_NONE);
		sec_msg_param_write32(
				&in_handle[cpu],
				/* &in_handle, */
				wakeup_address,
				SEC_MSG_PARAM_ID_NONE);
		sec_msg_param_write32(
				&in_handle[cpu],
				/* &in_handle, */
				pll0,
				SEC_MSG_PARAM_ID_NONE);
		sec_msg_param_write32(
				&in_handle[cpu],
				/* &in_handle, */
				zclk,
				SEC_MSG_PARAM_ID_NONE);

		/* call dispatcher */
		ssa_disp_status = LOCAL_DISP_FUNC(
							SEC_SERV_COMA_ENTRY,
							LOCAL_DEFAULT_DISP_FLAGS,
							LOCAL_DEFAULT_DISP_SPARE_PARAM,
							SEC_HAL_MEM_VIR2PHY_FUNC(out_msg[cpu]),
							/* SEC_HAL_MEM_VIR2PHY_FUNC(out_msg), */
							SEC_HAL_MEM_VIR2PHY_FUNC(in_msg[cpu]));
							/* SEC_HAL_MEM_VIR2PHY_FUNC(in_msg)); */

		/* interpret the response */
		sec_msg_status = sec_msg_param_read32(&out_handle[cpu], &sec_serv_status);
		/* sec_msg_status = sec_msg_param_read32(&out_handle, &sec_serv_status); */
		if (SEC_ROM_RET_OK != ssa_disp_status ||
			SEC_MSG_STATUS_OK != sec_msg_status ||
			SEC_SERV_STATUS_OK != sec_serv_status)
		{
			sec_hal_status = SEC_HAL_RES_FAIL;
		}
#if 0
	}

	/* de-allocate msgs */
	sec_msg_free(out_msg);
	sec_msg_free(in_msg);

	SEC_HAL_TRACE_EXIT
#endif
	/* SEC_HAL_TRACE_EXIT */
	printk(KERN_INFO "[%s] is returned!! ", __func__);
	return sec_hal_status;
}
/* made available for other kernel entities */
EXPORT_SYMBOL(sec_hal_pm_coma_entry);

/* **************************************************************************
** Function name      : sec_hal_pm_coma_cpu_off
** Description        : transmit coma cpu off event to TZ.
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** *************************************************************************/
uint32_t sec_hal_pm_coma_cpu_off(void)
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
							SEC_SERV_COMA_CPU_OFF,
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
			sec_hal_status = SEC_HAL_RES_FAIL;
		}
	}

	/* de-allocate msgs */
	sec_msg_free(out_msg);
	sec_msg_free(in_msg);

	return sec_hal_status;
}
/* made available for other kernel entities */
EXPORT_SYMBOL(sec_hal_pm_coma_cpu_off);

/* **************************************************************************
** Function name      : sec_hal_pm_poweroff
** Description        : transmit poweroff event to TZ.
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** *************************************************************************/
uint32_t sec_hal_pm_poweroff(void)
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
							SEC_SERV_POWER_OFF,
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
			sec_hal_status = SEC_HAL_RES_FAIL;
		}
	}

	/* de-allocate msgs */
	sec_msg_free(out_msg);
	sec_msg_free(in_msg);

	return sec_hal_status;
}

/* made available for other kernel entities */
EXPORT_SYMBOL(sec_hal_pm_poweroff);


/* **************************************************************************
** Function name      : sec_hal_pm_public_cc42_key_init
** Description        : transmit public cc42 key init event to TZ.
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** *************************************************************************/
uint32_t sec_hal_pm_public_cc42_key_init(void)
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
			sec_hal_status = SEC_HAL_RES_FAIL;
		}
	}

	/* de-allocate msgs */
	sec_msg_free(out_msg);
	sec_msg_free(in_msg);

	return sec_hal_status;
}

/* **************************************************************************
** Function name      : sec_hal_pm_a3sp_state_request
** Description        : transmit&receives public cc42 key init event to TZ.
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** *************************************************************************/
uint32_t sec_hal_pm_a3sp_state_request(uint32_t state)
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
		sec_hal_status = SEC_HAL_RES_FAIL;
	}
	else
	{
		/* write content to the input msg */
		sec_msg_param_write32(
				&in_handle,
				state,
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
			sec_hal_status = SEC_HAL_RES_FAIL;
		}
		else
		{
			/* Call was succesful, no actions */
		}
	}

	/* de-allocate msgs */
	sec_msg_free(out_msg);
	sec_msg_free(in_msg);

	return sec_hal_status;
}

/* ****************************************************************************
** Function name      : sec_hal_pm_l2_enable
** Description        : share l2cache control spinlock with secure side.
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** ***************************************************************************/
uint32_t sec_hal_pm_l2_enable(uint32_t spinlock_phys_addr)
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

    /* allocate memory, from ICRAM, for msgs to be sent to TZ */
    msg_data_size = sec_msg_param_size(sizeof(uint32_t))*3;
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

    if (!in_msg || !out_msg) {
        sec_hal_status = SEC_HAL_RES_FAIL;
    }
    else {
        /* write content to the input msg */
        sec_msg_param_write32(
                &in_handle,
                1, /* nonzero == enable */
                SEC_MSG_PARAM_ID_NONE);
        sec_msg_param_write32(
                &in_handle,
                L2_SPINLOCK_TYPE_DEFAULT,
                SEC_MSG_PARAM_ID_NONE);
        sec_msg_param_write32(
                &in_handle,
                spinlock_phys_addr,
                SEC_MSG_PARAM_ID_NONE);

        /* call dispatcher */
        ssa_disp_status = LOCAL_DISP_FUNC(
                            SEC_SERV_L2_CACHE_CONTROL,
                            LOCAL_DEFAULT_DISP_FLAGS,
                            LOCAL_DEFAULT_DISP_SPARE_PARAM,
                            SEC_HAL_MEM_VIR2PHY_FUNC(out_msg),
                            SEC_HAL_MEM_VIR2PHY_FUNC(in_msg));

        /* interpret the response */
        sec_msg_status = sec_msg_param_read32(&out_handle, &sec_serv_status);
        if (SEC_ROM_RET_OK != ssa_disp_status ||
            SEC_MSG_STATUS_OK != sec_msg_status ||
            SEC_SERV_STATUS_OK != sec_serv_status) {
            sec_hal_status = SEC_HAL_RES_FAIL;
        }
    }

    /* de-allocate msgs */
    sec_msg_free(out_msg);
    sec_msg_free(in_msg);

    return sec_hal_status;
}
/* made available for other kernel entities */
EXPORT_SYMBOL(sec_hal_pm_l2_enable);

/* ******************************** END ********************************** */
