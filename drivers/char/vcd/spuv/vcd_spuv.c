/* vcd_spuv.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <mach/pm.h>

#include "linux/vcd/vcd_common.h"
#include "linux/vcd/vcd_control.h"
#include "vcd_spuv.h"
#include "vcd_spuv_func.h"


/*
 * global variable declaration
 */
struct vcd_spuv_info g_vcd_spuv_info;

struct workqueue_struct *g_vcd_spuv_work_queue;
struct vcd_spuv_workqueue g_vcd_spuv_interrupt_ack;
struct vcd_spuv_workqueue g_vcd_spuv_interrupt_req;

struct workqueue_struct *g_vcd_spuv_notify_queue;
struct vcd_spuv_workqueue g_vcd_spuv_rec_trigger;
struct vcd_spuv_workqueue g_vcd_spuv_play_trigger;
struct vcd_spuv_workqueue g_vcd_spuv_system_error;


/* ========================================================================= */
/* Internal public functions                                                 */
/* ========================================================================= */

/**
 * @brief	driver ioremap function.
 *
 * @param	none.
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	others		result of called function.
 */
int vcd_spuv_ioremap(void)
{
	int ret = VCD_ERR_NONE;

	vcd_pr_start_spuv_function();

	ret = vcd_spuv_func_ioremap();
	if (VCD_ERR_NONE != ret)
		vcd_spuv_func_iounmap();

	vcd_pr_end_spuv_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	driver iounmap function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_spuv_iounmap(void)
{
	vcd_pr_start_spuv_function();

	vcd_spuv_func_iounmap();

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	get firmware buffer function.
 *
 * @param	none.
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	others		result of called function.
 */
int vcd_spuv_get_fw_buffer(void)
{
	int ret = VCD_ERR_NONE;

	vcd_pr_start_spuv_function();

	ret = vcd_spuv_func_get_fw_buffer();
	if (VCD_ERR_NONE != ret)
		vcd_spuv_func_free_fw_buffer();

	vcd_pr_end_spuv_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	free firmware buffer function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_spuv_free_fw_buffer(void)
{
	vcd_pr_start_spuv_function();

	vcd_spuv_func_free_fw_buffer();

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	get msg buffer function.
 *
 * @param	none.
 *
 * @retval	buf_addr	msg buffer physical address.
 */
int vcd_spuv_get_msg_buffer(void)
{
	int buf_addr = 0;

	vcd_pr_start_spuv_function();

	vcd_spuv_func_initialize();

	buf_addr = SPUV_FUNC_SDRAM_AREA_TOP_PHY +
		(SPUV_FUNC_SDRAM_PROC_MSG_BUFFER - SPUV_FUNC_SDRAM_AREA_TOP);

	vcd_spuv_func_cacheflush();

	vcd_pr_end_spuv_function("buf_addr[%d].\n", buf_addr);
	return buf_addr;
}


/**
 * @brief	start vcd function.
 *
 * @param	none.
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	others		result of called function.
 */
int vcd_spuv_start_vcd(void)
{
	int ret = VCD_ERR_NONE;

	vcd_pr_start_spuv_function();

	memset(&g_vcd_spuv_info, 0, sizeof(struct vcd_spuv_info));


	/* set power supply */
	ret = vcd_spuv_func_control_power_supply(
					VCD_SPUV_FUNC_ENABLE);

	/* beginning on clkgen is notified */
	/* regardless of the execution result. */
	vcd_ctrl_start_clkgen();

	if (VCD_ERR_NONE != ret) {
		vcd_pr_err("power supply error[%d].\n", ret);
		goto err_rtn;
	}

	/* set firmware */
	ret = vcd_spuv_func_set_fw();
	if (VCD_ERR_NONE != ret) {
		vcd_pr_err("set firmware error[%d].\n", ret);
		goto err_rtn;
	}

	/* interrupt start */
	ret = vcd_spuv_request_irq();
	if (VCD_ERR_NONE != ret) {
		vcd_pr_err("request irq error[%d].\n", ret);
		goto err_rtn;
	}

	/* set status */
	vcd_spuv_set_status(VCD_SPUV_STATUS_WAIT_REQ);

	vcd_spuv_set_wait_fw_info(VCD_SPUV_INTERFACE_ID,
				VCD_SPUV_BOOT_COMPLETE_IND);

	/* core reset */
	vcd_spuv_func_dsp_core_reset();

	/* check result */
	ret = vcd_spuv_check_result();
	if (VCD_ERR_NONE == ret)
		goto rtn;

	/* error route */
	vcd_spuv_free_irq();

err_rtn:
	vcd_spuv_func_control_power_supply(VCD_SPUV_FUNC_DISABLE);
	vcd_spuv_func_release_firmware();
	memset(&g_vcd_spuv_info, 0, sizeof(struct vcd_spuv_info));
rtn:
	vcd_pr_end_spuv_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	stop vcd function.
 *
 * @param	none.
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	others		result of called function.
 */
int vcd_spuv_stop_vcd()
{
	int ret = VCD_ERR_NONE;

	vcd_pr_start_spuv_function();

	/* interrupt end */
	vcd_spuv_free_irq();

	/* power supply off */
	ret = vcd_spuv_func_control_power_supply(VCD_SPUV_FUNC_DISABLE);
	if (VCD_ERR_NONE != ret)
		vcd_pr_err("power supply error[%d].\n", ret);

	vcd_spuv_func_release_firmware();

	memset(&g_vcd_spuv_info, 0, sizeof(struct vcd_spuv_info));

	vcd_pr_end_spuv_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	set hw param function.
 *
 * @param	none.
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	others		result of called function.
 */
int vcd_spuv_set_hw_param(void)
{
	int ret = VCD_ERR_NONE;
	int *param = (int *)SPUV_FUNC_SDRAM_SPUV_MSG_BUFFER;
	int *proc_param = (int *)SPUV_FUNC_SDRAM_PROC_MSG_BUFFER;

	vcd_pr_start_spuv_function();

	/* set status */
	vcd_spuv_set_status(VCD_SPUV_STATUS_WAIT_ACK);

	/* set expected response*/
	vcd_spuv_set_wait_fw_info(VCD_SPUV_INTERFACE_ID_ACK_ONLY,
				VCD_SPUV_HW_PARAMETERS_IND);

	/* flush cache */
	vcd_spuv_func_cacheflush();

	/* set parameter */
	param[0] = VCD_SPUV_INTERFACE_ID;
	param[1] = VCD_SPUV_HW_PARAMETERS_IND;
	param[2] = proc_param[0];
	param[3] = proc_param[1];
	param[4] = proc_param[2];

	/* output msg log */
	vcd_spuv_interface_log(param[1]);

	/* send message */
	vcd_spuv_func_send_msg(param, VCD_SPUV_HW_PARAMETERS_LENGTH);

	/* check result */
	ret = vcd_spuv_check_result();

	vcd_pr_end_spuv_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	start call function.
 *
 * @param	none.
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	others		result of called function.
 */
int vcd_spuv_start_call(void)
{
	int ret = VCD_ERR_NONE;
	int *param = (int *)SPUV_FUNC_SDRAM_SPUV_MSG_BUFFER;
	int *proc_param = (int *)SPUV_FUNC_SDRAM_PROC_MSG_BUFFER;

	vcd_pr_start_spuv_function();

	/* set status */
	vcd_spuv_set_status(VCD_SPUV_STATUS_WAIT_ACK |
				VCD_SPUV_STATUS_WAIT_REQ);

	/* set expected response*/
	vcd_spuv_set_wait_fw_info(VCD_SPUV_INTERFACE_ID,
				VCD_SPUV_SPEECH_START_CNF);

	/* flush cache */
	vcd_spuv_func_cacheflush();

	/* set parameter */
	param[0] = VCD_SPUV_INTERFACE_ID;
	param[1] = VCD_SPUV_SPEECH_START_REQ;
	param[2] = proc_param[0];

	/* output msg log */
	vcd_spuv_interface_log(param[1]);

	/* send message */
	vcd_spuv_func_send_msg(param, VCD_SPUV_SPEECH_START_LENGTH);

	/* check result */
	ret = vcd_spuv_check_result();

	vcd_pr_end_spuv_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	stop call function.
 *
 * @param	none.
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	others		result of called function.
 */
int vcd_spuv_stop_call(void)
{
	int ret = VCD_ERR_NONE;
	int *param = (int *)SPUV_FUNC_SDRAM_SPUV_MSG_BUFFER;

	vcd_pr_start_spuv_function();

	/* set status */
	vcd_spuv_set_status(VCD_SPUV_STATUS_WAIT_ACK |
				VCD_SPUV_STATUS_WAIT_REQ);

	/* set expected response*/
	vcd_spuv_set_wait_fw_info(VCD_SPUV_INTERFACE_ID,
				VCD_SPUV_SPEECH_STOP_CNF);

	/* set parameter */
	param[0] = VCD_SPUV_INTERFACE_ID;
	param[1] = VCD_SPUV_SPEECH_STOP_REQ;

	/* output msg log */
	vcd_spuv_interface_log(param[1]);

	/* send message */
	vcd_spuv_func_send_msg(param, VCD_SPUV_SPEECH_STOP_LENGTH);

	/* check result */
	ret = vcd_spuv_check_result();

	vcd_pr_end_spuv_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	start tty/ctm function.
 *
 * @param	none.
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	others		result of called function.
 */
int vcd_spuv_start_tty_ctm(void)
{
	int ret = VCD_ERR_NONE;
	int *param = (int *)SPUV_FUNC_SDRAM_SPUV_MSG_BUFFER;

	vcd_pr_start_spuv_function();

	/* set status */
	vcd_spuv_set_status(VCD_SPUV_STATUS_WAIT_ACK |
				VCD_SPUV_STATUS_WAIT_REQ);

	/* set expected response*/
	vcd_spuv_set_wait_fw_info(VCD_SPUV_INTERFACE_ID,
				VCD_SPUV_TTY_CTM_START_CNF);

	/* flush cache */
	vcd_spuv_func_cacheflush();

	/* set parameter */
	param[0] = VCD_SPUV_INTERFACE_ID;
	param[1] = VCD_SPUV_TTY_CTM_START_REQ;
#if 0
	param[2] = 0;
	param[3] = 0;
	param[4] = 0;
	param[5] = 0;
#endif

	/* output msg log */
	vcd_spuv_interface_log(param[1]);

	/* send message */
	vcd_spuv_func_send_msg(param, VCD_SPUV_TTY_CTM_START_LENGTH);

	/* check result */
	ret = vcd_spuv_check_result();

	vcd_pr_end_spuv_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	stop tty/ctm function.
 *
 * @param	none.
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	others		result of called function.
 */
int vcd_spuv_stop_tty_ctm(void)
{
	int ret = VCD_ERR_NONE;
	int *param = (int *)SPUV_FUNC_SDRAM_SPUV_MSG_BUFFER;

	vcd_pr_start_spuv_function();

	/* set status */
	vcd_spuv_set_status(VCD_SPUV_STATUS_WAIT_ACK |
				VCD_SPUV_STATUS_WAIT_REQ);

	/* set expected response*/
	vcd_spuv_set_wait_fw_info(VCD_SPUV_INTERFACE_ID,
				VCD_SPUV_TTY_CTM_STOP_CNF);

	/* set parameter */
	param[0] = VCD_SPUV_INTERFACE_ID;
	param[1] = VCD_SPUV_TTY_CTM_STOP_REQ;

	/* output msg log */
	vcd_spuv_interface_log(param[1]);

	/* send message */
	vcd_spuv_func_send_msg(param, VCD_SPUV_TTY_CTM_STOP_LENGTH);

	/* check result */
	ret = vcd_spuv_check_result();

	vcd_pr_end_spuv_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	config tty/ctm function.
 *
 * @param	none.
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	others		result of called function.
 */
int vcd_spuv_config_tty_ctm(void)
{
	int ret = VCD_ERR_NONE;
	int *param = (int *)SPUV_FUNC_SDRAM_SPUV_MSG_BUFFER;
	int *proc_param = (int *)SPUV_FUNC_SDRAM_PROC_MSG_BUFFER;

	vcd_pr_start_spuv_function();

	/* set status */
	vcd_spuv_set_status(VCD_SPUV_STATUS_WAIT_ACK |
				VCD_SPUV_STATUS_WAIT_REQ);

	/* set expected response*/
	vcd_spuv_set_wait_fw_info(VCD_SPUV_INTERFACE_ID,
				VCD_SPUV_TTY_CTM_CONFIG_CNF);

	/* flush cache */
	vcd_spuv_func_cacheflush();

	/* set parameter */
	param[0] = VCD_SPUV_INTERFACE_ID;
	param[1] = VCD_SPUV_TTY_CTM_CONFIG_REQ;
	param[2] = proc_param[0];
	param[3] = proc_param[1];

	/* output msg log */
	vcd_spuv_interface_log(param[1]);

	/* send message */
	vcd_spuv_func_send_msg(param, VCD_SPUV_TTY_CTM_CONFIG_LENGTH);

	/* check result */
	ret = vcd_spuv_check_result();

	vcd_pr_end_spuv_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	set udata function.
 *
 * @param	none.
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	others		result of called function.
 */
int vcd_spuv_set_udata(void)
{
	int ret = VCD_ERR_NONE;
	int *param = (int *)SPUV_FUNC_SDRAM_SPUV_MSG_BUFFER;
	int *proc_param = (int *)SPUV_FUNC_SDRAM_PROC_MSG_BUFFER;

	vcd_pr_start_spuv_function();

	/* set status */
	vcd_spuv_set_status(VCD_SPUV_STATUS_WAIT_ACK);

	/* set expected response*/
	vcd_spuv_set_wait_fw_info(VCD_SPUV_INTERFACE_ID_ACK_ONLY,
				VCD_SPUV_UDATA_REQ);

	/* flush cache */
	vcd_spuv_func_cacheflush();

	/* set parameter */
	param[0] = VCD_SPUV_INTERFACE_ID;
	param[1] = VCD_SPUV_UDATA_REQ;
	param[2] = proc_param[0];
	memcpy((void *)&param[3], (void *)&proc_param[1],
				 sizeof(int)*proc_param[0]);

	/* output msg log */
	vcd_spuv_interface_log(param[1]);

	/* send message */
	vcd_spuv_func_send_msg(param, VCD_SPUV_UDATA_LENGTH+param[2]);

	/* check result */
	ret = vcd_spuv_check_result();

	vcd_pr_end_spuv_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	start record function.
 *
 * @param	option		record mode.
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	others		result of called function.
 */
int vcd_spuv_start_record(struct vcd_record_option *option)
{
	int ret = VCD_ERR_NONE;
	int *param = (int *)SPUV_FUNC_SDRAM_SPUV_MSG_BUFFER;
	int dl_gain = VCD_SPUV_GAIN_DISABLE;
	int ul_gain = VCD_SPUV_GAIN_DISABLE;

	vcd_pr_start_spuv_function("option[%p].\n", option);

	/* set status */
	vcd_spuv_set_status(VCD_SPUV_STATUS_WAIT_ACK |
				VCD_SPUV_STATUS_WAIT_REQ);

	/* set expected response*/
	vcd_spuv_set_wait_fw_info(VCD_SPUV_INTERFACE_ID,
				VCD_SPUV_VOICE_RECORDING_START_CNF);

	/* set parameter */
	switch (option->mode) {
	case VCD_RECORD_MODE_0:
		dl_gain = VCD_SPUV_GAIN_ENABLE;
		ul_gain = VCD_SPUV_GAIN_ENABLE;
		break;
	case VCD_RECORD_MODE_1:
		dl_gain = VCD_SPUV_GAIN_ENABLE;
		break;
	case VCD_RECORD_MODE_2:
		ul_gain = VCD_SPUV_GAIN_ENABLE;
		break;
	default:
		/* impossible route */
		break;
	}

	param[0] = VCD_SPUV_INTERFACE_ID;
	param[1] = VCD_SPUV_VOICE_RECORDING_START_REQ;
	param[2] = 0;
	param[3] = ((vcd_spuv_func_sdram_logical_to_physical(
			SPUV_FUNC_SDRAM_RECORD_BUFFER_0) >> 16) & 0x0000FFFF);
	param[4] = (vcd_spuv_func_sdram_logical_to_physical(
			SPUV_FUNC_SDRAM_RECORD_BUFFER_0) & 0x0000FFFF);
	param[5] = dl_gain;
	param[6] = ul_gain;

	/* output msg log */
	vcd_spuv_interface_log(param[1]);

	/* send message */
	vcd_spuv_func_send_msg(param, VCD_SPUV_VOICE_RECORDING_START_LENGTH);

	/* check result */
	ret = vcd_spuv_check_result();

	vcd_pr_end_spuv_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	stop record function.
 *
 * @param	none.
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	others		result of called function.
 */
int vcd_spuv_stop_record(void)
{
	int ret = VCD_ERR_NONE;
	int *param = (int *)SPUV_FUNC_SDRAM_SPUV_MSG_BUFFER;

	vcd_pr_start_spuv_function();

	/* set status */
	vcd_spuv_set_status(VCD_SPUV_STATUS_WAIT_ACK |
				VCD_SPUV_STATUS_WAIT_REQ);

	/* set expected response*/
	vcd_spuv_set_wait_fw_info(VCD_SPUV_INTERFACE_ID,
				VCD_SPUV_VOICE_RECORDING_STOP_CNF);

	/* set parameter */
	param[0] = VCD_SPUV_INTERFACE_ID;
	param[1] = VCD_SPUV_VOICE_RECORDING_STOP_REQ;

	/* output msg log */
	vcd_spuv_interface_log(param[1]);

	/* send message */
	vcd_spuv_func_send_msg(param, VCD_SPUV_VOICE_RECORDING_STOP_LENGTH);

	/* check result */
	ret = vcd_spuv_check_result();

	vcd_pr_end_spuv_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	start playback function.
 *
 * @param	option		playback mode.
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	others		result of called function.
 */
int vcd_spuv_start_playback(struct vcd_playback_option *option)
{
	int ret = VCD_ERR_NONE;
	int *param = (int *)SPUV_FUNC_SDRAM_SPUV_MSG_BUFFER;
	int dl_speech_gain = VCD_SPUV_GAIN_DISABLE;
	int dl_playback_gain = VCD_SPUV_GAIN_DISABLE;
	int ul_speech_gain = VCD_SPUV_GAIN_DISABLE;
	int ul_playback_gain = VCD_SPUV_GAIN_DISABLE;

	vcd_pr_start_spuv_function("option[%p].\n", option);

	/* set status */
	vcd_spuv_set_status(VCD_SPUV_STATUS_WAIT_ACK |
				VCD_SPUV_STATUS_WAIT_REQ);

	/* set expected response*/
	vcd_spuv_set_wait_fw_info(VCD_SPUV_INTERFACE_ID,
				VCD_SPUV_VOICE_PLAYING_START_CNF);

	/* set parameter */
	switch (option->mode) {
	case VCD_PLAYBACK_MODE_0:
		dl_speech_gain = VCD_SPUV_GAIN_ENABLE;
		dl_playback_gain = VCD_SPUV_GAIN_ENABLE;
		ul_speech_gain = VCD_SPUV_GAIN_ENABLE;
		break;
	case VCD_PLAYBACK_MODE_1:
		ul_playback_gain = VCD_SPUV_GAIN_ENABLE;
		break;
	case VCD_PLAYBACK_MODE_2:
		dl_playback_gain = VCD_SPUV_GAIN_ENABLE;
		break;
	default:
		/* impossible route */
		break;
	}

	param[0] = VCD_SPUV_INTERFACE_ID;
	param[1] = VCD_SPUV_VOICE_PLAYING_START_REQ;
	param[2] = 0;
	param[3] = ((vcd_spuv_func_sdram_logical_to_physical(
			SPUV_FUNC_SDRAM_PLAYBACK_BUFFER_0) >> 16) &
			0x0000FFFF);
	param[4] = (vcd_spuv_func_sdram_logical_to_physical(
			SPUV_FUNC_SDRAM_PLAYBACK_BUFFER_0) & 0x0000FFFF);
	param[5] = dl_speech_gain;
	param[6] = dl_playback_gain;
	param[7] = ul_speech_gain;
	param[8] = ul_playback_gain;

	/* output msg log */
	vcd_spuv_interface_log(param[1]);

	/* send message */
	vcd_spuv_func_send_msg(param, VCD_SPUV_VOICE_PLAYING_START_LENGTH);

	/* check result */
	ret = vcd_spuv_check_result();

	vcd_pr_end_spuv_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	stop playback function.
 *
 * @param	none.
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	others		result of called function.
 */
int vcd_spuv_stop_playback(void)
{
	int ret = VCD_ERR_NONE;
	int *param = (int *)SPUV_FUNC_SDRAM_SPUV_MSG_BUFFER;

	vcd_pr_start_spuv_function();

	/* set status */
	vcd_spuv_set_status(VCD_SPUV_STATUS_WAIT_ACK |
				VCD_SPUV_STATUS_WAIT_REQ);

	/* set expected response*/
	vcd_spuv_set_wait_fw_info(VCD_SPUV_INTERFACE_ID,
				VCD_SPUV_VOICE_PLAYING_STOP_CNF);

	/* set parameter */
	param[0] = VCD_SPUV_INTERFACE_ID;
	param[1] = VCD_SPUV_VOICE_PLAYING_STOP_REQ;

	/* output msg log */
	vcd_spuv_interface_log(param[1]);

	/* send message */
	vcd_spuv_func_send_msg(param, VCD_SPUV_VOICE_PLAYING_STOP_LENGTH);

	/* check result */
	ret = vcd_spuv_check_result();

	vcd_pr_end_spuv_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	get record buffer function.
 *
 * @param	info	buffer information.
 *
 * @retval	none.
 */
void vcd_spuv_get_record_buffer(struct vcd_record_buffer_info *info)
{
	vcd_pr_start_spuv_function("info[%p].\n", info);

	/* set buffer address */
	info->record_buffer[0] =
		(unsigned int *)SPUV_FUNC_SDRAM_RECORD_BUFFER_0;
	info->record_buffer[1] =
		(unsigned int *)SPUV_FUNC_SDRAM_RECORD_BUFFER_1;

	vcd_pr_end_spuv_function();
}


/**
 * @brief	get playback buffer function.
 *
 * @param	info	buffer information.
 *
 * @retval	none.
 */
void vcd_spuv_get_playback_buffer(struct vcd_playback_buffer_info *info)
{
	vcd_pr_start_spuv_function("info[%p].\n", info);

	/* set buffer address */
	info->playback_buffer[0] =
		(unsigned int *)SPUV_FUNC_SDRAM_PLAYBACK_BUFFER_0;
	info->playback_buffer[1] =
		(unsigned int *)SPUV_FUNC_SDRAM_PLAYBACK_BUFFER_1;

	vcd_pr_end_spuv_function();
}


/**
 * @brief	start 1khz tone function.
 *
 * @param	none.
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	others		result of called function.
 */
int vcd_spuv_start_1khz_tone(void)
{
	int ret = VCD_ERR_NONE;
	int *param = (int *)SPUV_FUNC_SDRAM_SPUV_MSG_BUFFER;

	vcd_pr_start_spuv_function();

	/* set status */
	vcd_spuv_set_status(VCD_SPUV_STATUS_WAIT_ACK |
				VCD_SPUV_STATUS_WAIT_REQ);

	/* set expected response*/
	vcd_spuv_set_wait_fw_info(VCD_SPUV_INTERFACE_ID,
				VCD_SPUV_1KHZ_TONE_START_CNF);

	/* set parameter */
	param[0] = VCD_SPUV_INTERFACE_ID;
	param[1] = VCD_SPUV_1KHZ_TONE_START_REQ;

	/* output msg log */
	vcd_spuv_interface_log(param[1]);

	/* send message */
	vcd_spuv_func_send_msg(param, VCD_SPUV_1KHZ_TONE_START_LENGTH);

	/* check result */
	ret = vcd_spuv_check_result();

	vcd_pr_end_spuv_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	stop 1khz tone function.
 *
 * @param	none.
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	others		result of called function.
 */
int vcd_spuv_stop_1khz_tone(void)
{
	int ret = VCD_ERR_NONE;
	int *param = (int *)SPUV_FUNC_SDRAM_SPUV_MSG_BUFFER;

	vcd_pr_start_spuv_function();

	/* set status */
	vcd_spuv_set_status(VCD_SPUV_STATUS_WAIT_ACK |
				VCD_SPUV_STATUS_WAIT_REQ);

	/* set expected response*/
	vcd_spuv_set_wait_fw_info(VCD_SPUV_INTERFACE_ID,
				VCD_SPUV_1KHZ_TONE_STOP_CNF);

	/* set parameter */
	param[0] = VCD_SPUV_INTERFACE_ID;
	param[1] = VCD_SPUV_1KHZ_TONE_STOP_REQ;

	/* output msg log */
	vcd_spuv_interface_log(param[1]);

	/* send message */
	vcd_spuv_func_send_msg(param, VCD_SPUV_1KHZ_TONE_STOP_LENGTH);

	/* check result */
	ret = vcd_spuv_check_result();

	vcd_pr_end_spuv_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	start pcm loopback function.
 *
 * @param	mode		loopback mode.
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	others		result of called function.
 */
int vcd_spuv_start_pcm_loopback(int mode)
{
	int ret = VCD_ERR_NONE;
	int *param = (int *)SPUV_FUNC_SDRAM_SPUV_MSG_BUFFER;

	vcd_pr_start_spuv_function("mode[%d]", mode);

	/* set status */
	vcd_spuv_set_status(VCD_SPUV_STATUS_WAIT_ACK |
				VCD_SPUV_STATUS_WAIT_REQ);

	/* set expected response*/
	vcd_spuv_set_wait_fw_info(VCD_SPUV_INTERFACE_ID,
				VCD_SPUV_PCM_LOOPBACK_START_CNF);

	/* set parameter */
	param[0] = VCD_SPUV_INTERFACE_ID;
	param[1] = VCD_SPUV_PCM_LOOPBACK_START_REQ;
	param[2] = mode;

	/* output msg log */
	vcd_spuv_interface_log(param[1]);

	/* send message */
	vcd_spuv_func_send_msg(param, VCD_SPUV_PCM_LOOPBACK_START_LENGTH);

	/* check result */
	ret = vcd_spuv_check_result();

	vcd_pr_end_spuv_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	stop pcm loopback function.
 *
 * @param	none.
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	others		result of called function.
 */
int vcd_spuv_stop_pcm_loopback(void)
{
	int ret = VCD_ERR_NONE;
	int *param = (int *)SPUV_FUNC_SDRAM_SPUV_MSG_BUFFER;

	vcd_pr_start_spuv_function();

	/* set status */
	vcd_spuv_set_status(VCD_SPUV_STATUS_WAIT_ACK |
				VCD_SPUV_STATUS_WAIT_REQ);

	/* set expected response*/
	vcd_spuv_set_wait_fw_info(VCD_SPUV_INTERFACE_ID,
				VCD_SPUV_PCM_LOOPBACK_STOP_CNF);

	/* set parameter */
	param[0] = VCD_SPUV_INTERFACE_ID;
	param[1] = VCD_SPUV_PCM_LOOPBACK_STOP_REQ;

	/* output msg log */
	vcd_spuv_interface_log(param[1]);

	/* send message */
	vcd_spuv_func_send_msg(param, VCD_SPUV_PCM_LOOPBACK_STOP_LENGTH);

	/* check result */
	ret = vcd_spuv_check_result();

	vcd_pr_end_spuv_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	start bbif loopback function.
 *
 * @param	mode		loopback mode.
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	others		result of called function.
 */
int vcd_spuv_start_bbif_loopback(int mode)
{
	int ret = VCD_ERR_NONE;
	int *param = (int *)SPUV_FUNC_SDRAM_SPUV_MSG_BUFFER;

	vcd_pr_start_spuv_function("mode[%d].\n", mode);

	/* set status */
	vcd_spuv_set_status(VCD_SPUV_STATUS_WAIT_ACK |
				VCD_SPUV_STATUS_WAIT_REQ);

	/* set expected response*/
	vcd_spuv_set_wait_fw_info(VCD_SPUV_INTERFACE_ID,
				VCD_SPUV_BBIF_LOOPBACK_START_CNF);

	/* set parameter */
	param[0] = VCD_SPUV_INTERFACE_ID;
	param[1] = VCD_SPUV_BBIF_LOOPBACK_START_REQ;
	param[2] = mode;

	/* output msg log */
	vcd_spuv_interface_log(param[1]);

	/* send message */
	vcd_spuv_func_send_msg(param, VCD_SPUV_BBIF_LOOPBACK_START_LENGTH);

	/* check result */
	ret = vcd_spuv_check_result();

	vcd_pr_end_spuv_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	stop bbif loopback function.
 *
 * @param	none.
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	others		result of called function.
 */
int vcd_spuv_stop_bbif_loopback(void)
{
	int ret = VCD_ERR_NONE;
	int *param = (int *)SPUV_FUNC_SDRAM_SPUV_MSG_BUFFER;

	vcd_pr_start_spuv_function();

	/* set status */
	vcd_spuv_set_status(VCD_SPUV_STATUS_WAIT_ACK |
				VCD_SPUV_STATUS_WAIT_REQ);

	/* set expected response*/
	vcd_spuv_set_wait_fw_info(VCD_SPUV_INTERFACE_ID,
				VCD_SPUV_BBIF_LOOPBACK_STOP_CNF);

	/* set parameter */
	param[0] = VCD_SPUV_INTERFACE_ID;
	param[1] = VCD_SPUV_BBIF_LOOPBACK_STOP_REQ;

	/* output msg log */
	vcd_spuv_interface_log(param[1]);

	/* send message */
	vcd_spuv_func_send_msg(param, VCD_SPUV_BBIF_LOOPBACK_STOP_LENGTH);

	/* check result */
	ret = vcd_spuv_check_result();

	vcd_pr_end_spuv_function("ret[%d].\n", ret);
	return ret;
}


int vcd_spuv_set_trace_select(void)
{
	int ret = VCD_ERR_NONE;
	vcd_pr_start_spuv_function();
	vcd_pr_end_spuv_function("ret[%d].\n", ret);
	return ret;
}


/* ========================================================================= */
/* Interrupt functions                                                       */
/* ========================================================================= */

/**
 * @brief	request irq function.
 *
 * @param	none.
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	others		result of called function.
 */
static int vcd_spuv_request_irq(void)
{
	int ret = VCD_ERR_NONE;

	vcd_pr_start_spuv_function();

	ret = vcd_spuv_set_irq(VCD_SPUV_ENABLE);
	if (VCD_ERR_NONE != ret)
		vcd_pr_err("vcd_spuv_func_set_irq ret[%d].\n", ret);

	vcd_pr_end_spuv_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	request irq function.
 *
 * @param	none.
 *
 * @retval	none.
 */
static void vcd_spuv_free_irq(void)
{
	int ret = VCD_ERR_NONE;

	vcd_pr_start_spuv_function();

	ret = vcd_spuv_set_irq(VCD_SPUV_DISABLE);
	if (VCD_ERR_NONE != ret)
		/* unlikely circumstance */
		vcd_pr_err("vcd_spuv_func_set_irq ret[%d].\n", ret);

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	irq setting function.
 *
 * @param	validity	enable/disable.
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	VCD_ERR_PARAM	invalid argument.
 * @retval	others		result of called function.
 */
static int vcd_spuv_set_irq(int validity)
{
	int ret = VCD_ERR_NONE;

	vcd_pr_start_spuv_function("validity[%d].\n", validity);

	switch (validity) {
	case VCD_SPUV_ENABLE:
		/* Bind SPUV interrupt */
		vcd_pr_spuv_info("execute request_irq().\n");
		ret = request_irq(VCD_SPUV_SPI_NO, vcd_spuv_irq_handler,
				0, "SPU2V DSP", NULL);
		break;
	case VCD_SPUV_DISABLE:
		/* Unbind SPUV interrupt */
		vcd_pr_spuv_info("execute free_irq().\n");
		free_irq(VCD_SPUV_SPI_NO, NULL);
		break;
	default:
		/* unlikely circumstance */
		vcd_pr_err("parameter error. validity[%d].\n", validity);
		ret = VCD_ERR_PARAM;
		break;
	}

	vcd_pr_end_spuv_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	irq handler function.
 *
 * @param[in]	irq	irq number.
 * @param[in]	dev_id	device id.
 *
 * @retval	IRQ_HANDLED.
 */
static irqreturn_t vcd_spuv_irq_handler(int irq, void *dev_id)
{
	unsigned int aintsts = 0;
	unsigned int ieventc = 0;
	unsigned int clear_bit = 0;

	vcd_pr_start_spuv_function("irq[%d],dev_id[%p].\n", irq, dev_id);

	/* read AINTSTS register */
	vcd_spuv_func_register(SPUV_FUNC_RO_32_AINTSTS, aintsts);

	if (VCD_SPUV_REPLY_ACK & aintsts) {
		vcd_pr_spuv_info("interrupt Ack.\n");
		clear_bit |= VCD_SPUV_AINTCLR_ACK;

		/* entry queue */
		queue_work(g_vcd_spuv_work_queue,
			&g_vcd_spuv_interrupt_ack.work);
	}

	if (VCD_SPUV_REPLY_REQ & aintsts) {
		vcd_pr_spuv_info("interrupt Req.\n");
		clear_bit |= VCD_SPUV_AINTCLR_REQ;

		/* entry queue */
		queue_work(g_vcd_spuv_work_queue,
			&g_vcd_spuv_interrupt_req.work);
	}

	/* set AINTCLR register */
	vcd_spuv_func_set_register(clear_bit, SPUV_FUNC_RW_32_AINTCLR);
	udelay(1);
	vcd_spuv_func_set_register(clear_bit, SPUV_FUNC_RW_32_AINTCLR);

	/* generate IEVENTC interrupt */
	vcd_spuv_func_register(SPUV_FUNC_RW_32_IEVENTC, ieventc);
	vcd_spuv_func_set_register((~ieventc & VCD_SPUV_IEVENTC),
					SPUV_FUNC_RW_32_IEVENTC);

	vcd_pr_end_spuv_function("return IRQ_HANDLED.\n");
	return IRQ_HANDLED;
}


/* ========================================================================= */
/* Queue functions                                                           */
/* ========================================================================= */

/**
 * @brief	create queue function.
 *
 * @param	none.
 *
 * @retval	VCD_ERR_NONE		successful.
 * @retval	VCD_ERR_NOMEMORY	create_workqueue error.
 */
int vcd_spuv_create_queue(void)
{
	int ret = VCD_ERR_NONE;

	vcd_pr_start_spuv_function();

	/* queue create for work */
	g_vcd_spuv_work_queue =
			create_singlethread_workqueue("vcd_spuv_work_queue");
	if (NULL == g_vcd_spuv_work_queue) {
		vcd_pr_err("queue create error.\n");
		ret = VCD_ERR_NOMEMORY;
	} else {
		INIT_WORK(&g_vcd_spuv_interrupt_ack.work,
					vcd_spuv_interrupt_ack);
		INIT_WORK(&g_vcd_spuv_interrupt_req.work,
					vcd_spuv_interrupt_req);
	}

	/* queue create for notify */
	g_vcd_spuv_notify_queue =
			create_singlethread_workqueue("vcd_spuv_notify_queue");
	if (NULL == g_vcd_spuv_notify_queue) {
		vcd_pr_err("notify queue create error.\n");
		ret = VCD_ERR_NOMEMORY;
	} else {
		INIT_WORK(&g_vcd_spuv_rec_trigger.work,
						vcd_spuv_rec_trigger);
		INIT_WORK(&g_vcd_spuv_play_trigger.work,
						vcd_spuv_play_trigger);
		INIT_WORK(&g_vcd_spuv_system_error.work,
						vcd_spuv_system_error);
	}

	vcd_pr_end_spuv_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	destroy queue function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_spuv_destroy_queue(void)
{

	vcd_pr_start_spuv_function();

	destroy_workqueue(g_vcd_spuv_work_queue);
	destroy_workqueue(g_vcd_spuv_notify_queue);

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	set scheduler function.
 *
 * @param	none.
 *
 * @retval	none.
 */
static void vcd_spuv_set_schedule(void)
{
	struct sched_param param = { .sched_priority = 75 };

	vcd_pr_start_spuv_function();

	sched_setscheduler(current, SCHED_FIFO, &param);

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	queue out interrupt_ack function.
 *
 * @param[in]	*work	work struct.
 *
 * @retval	none.
 */
static void vcd_spuv_interrupt_ack(struct work_struct *work)
{
	int ret = VCD_ERR_NONE;

	vcd_pr_start_spuv_function("work[%p].\n", work);

	/* set schedule */
	vcd_spuv_set_schedule();

	vcd_pr_if_spuv("V <-- F : ACK.\n");

	/* check power supply */
	ret = vcd_spuv_func_check_power_supply();
	if (VCD_SPUV_FUNC_DISABLE == ret)
		goto rtn;

	/* unset status */
	vcd_spuv_unset_status(VCD_SPUV_STATUS_WAIT_ACK);

	/* check status */
	ret = vcd_spuv_get_status();
	if (VCD_SPUV_STATUS_NONE == ret)
		/* end wait */
		vcd_spuv_func_end_wait();


rtn:
	vcd_pr_end_if_spuv();
	return;
}


/**
 * @brief	queue out interrupt_req function.
 *
 * @param[in]	*work	work struct.
 *
 * @retval	none.
 */
static void vcd_spuv_interrupt_req(struct work_struct *work)
{
	int ret = VCD_ERR_NONE;
	int i = 0;
	unsigned int *fw_req = (int *)SPUV_FUNC_SDRAM_FW_RESULT_BUFFER;

	vcd_pr_start_spuv_function("work[%p].\n", work);

	/* set schedule */
	vcd_spuv_set_schedule();

	/* check power supply */
	ret = vcd_spuv_func_check_power_supply();
	if (VCD_SPUV_FUNC_DISABLE == ret)
		goto rtn;

	/* check status */
	if (VCD_SPUV_STATUS_SYSTEM_ERROR & vcd_spuv_get_status())
		goto rtn;

	/* status update */
	vcd_spuv_set_status(VCD_SPUV_STATUS_NEED_ACK);

	/* get request details */
	vcd_spuv_func_get_fw_request();

	/* output msg log */
	vcd_spuv_interface_log(fw_req[1]);

	switch (fw_req[1]) {
	case VCD_SPUV_SYSTEM_ERROR_IND:
		vcd_spuv_set_status(VCD_SPUV_STATUS_SYSTEM_ERROR);
		queue_work(g_vcd_spuv_notify_queue,
			&g_vcd_spuv_system_error.work);
		break;
	case VCD_SPUV_SYSTEM_INFO_IND:
		vcd_pr_spuv_info("system info length[%d].\n", fw_req[2]);
		for (i = 3; i < (fw_req[2] + 3); i++)
			vcd_pr_spuv_debug(
				"system info[%d][%x].\n", i, fw_req[i]);
		break;
	case VCD_SPUV_UDATA_IND:
		break;
	case VCD_SPUV_TRIGGER_REC_IND:
		queue_work(g_vcd_spuv_notify_queue,
			&g_vcd_spuv_rec_trigger.work);
		break;
	case VCD_SPUV_TRIGGER_PLAY_IND:
		queue_work(g_vcd_spuv_notify_queue,
			&g_vcd_spuv_play_trigger.work);
		break;
	default:
		/* status update */
		vcd_spuv_unset_status(VCD_SPUV_STATUS_WAIT_REQ);
		/* check result */
		vcd_spuv_check_wait_fw_info(fw_req[0], fw_req[1], fw_req[2]);
		if (VCD_SPUV_BOOT_COMPLETE_IND == fw_req[1]) {
			vcd_pr_spuv_info("SPUV version is.\n");
			for (i = 2; i < 10; i++)
				vcd_pr_spuv_info("[%x].\n", fw_req[i]);
		}
		break;
	}

	/* send ack message */
	vcd_spuv_func_send_ack();
	vcd_spuv_unset_status(VCD_SPUV_STATUS_NEED_ACK);

	/* check status */
	ret = vcd_spuv_get_status();
	if ((VCD_SPUV_STATUS_NONE == ret) ||
			(VCD_SPUV_STATUS_SYSTEM_ERROR & ret))
		/* end wait */
		vcd_spuv_func_end_wait();

rtn:
	vcd_pr_end_if_spuv();
	return;
}


/**
 * @brief	queue out rec_trigger function.
 *
 * @param[in]	*work	work struct.
 *
 * @retval	none.
 */
static void vcd_spuv_rec_trigger(struct work_struct *work)
{
	vcd_pr_start_spuv_function("work[%p].\n", work);

	/* set schedule */
	vcd_spuv_set_schedule();

	/* notification buffer update */
	vcd_ctrl_rec_trigger();

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	queue out play_trigger function.
 *
 * @param[in]	*work	work struct.
 *
 * @retval	none.
 */
static void vcd_spuv_play_trigger(struct work_struct *work)
{
	vcd_pr_start_spuv_function("work[%p].\n", work);

	/* set schedule */
	vcd_spuv_set_schedule();

	/* notification buffer update */
	vcd_ctrl_play_trigger();

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	queue out system_error function.
 *
 * @param[in]	*work	work struct.
 *
 * @retval	none.
 */
static void vcd_spuv_system_error(struct work_struct *work)
{
	vcd_pr_start_spuv_function("work[%p].\n", work);

	/* set schedule */
	vcd_spuv_set_schedule();

	/* notification fw stop */
	vcd_ctrl_stop_fw();

	vcd_pr_end_spuv_function();
	return;
}


static void vcd_spuv_interface_log(unsigned int msg)
{
	switch (msg) {
	case VCD_SPUV_HW_PARAMETERS_IND:
		vcd_pr_if_spuv(VCD_SPUV_HW_PARAMETERS_IND_LOG);
		break;
	case VCD_SPUV_ACTIVE_REQ:
		vcd_pr_if_spuv(VCD_SPUV_ACTIVE_REQ_LOG);
		break;
	case VCD_SPUV_SPEECH_START_REQ:
		vcd_pr_if_spuv(VCD_SPUV_SPEECH_START_REQ_LOG);
		break;
	case VCD_SPUV_SPEECH_STOP_REQ:
		vcd_pr_if_spuv(VCD_SPUV_SPEECH_STOP_REQ_LOG);
		break;
	case VCD_SPUV_VOICE_RECORDING_START_REQ:
		vcd_pr_if_spuv(VCD_SPUV_VOICE_RECORDING_START_REQ_LOG);
		break;
	case VCD_SPUV_VOICE_RECORDING_STOP_REQ:
		vcd_pr_if_spuv(VCD_SPUV_VOICE_RECORDING_STOP_REQ_LOG);
		break;
	case VCD_SPUV_VOICE_PLAYING_START_REQ:
		vcd_pr_if_spuv(VCD_SPUV_VOICE_PLAYING_START_REQ_LOG);
		break;
	case VCD_SPUV_VOICE_PLAYING_STOP_REQ:
		vcd_pr_if_spuv(VCD_SPUV_VOICE_PLAYING_STOP_REQ_LOG);
		break;
	case VCD_SPUV_1KHZ_TONE_START_REQ:
		vcd_pr_if_spuv(VCD_SPUV_1KHZ_TONE_START_REQ_LOG);
		break;
	case VCD_SPUV_1KHZ_TONE_STOP_REQ:
		vcd_pr_if_spuv(VCD_SPUV_1KHZ_TONE_STOP_REQ_LOG);
		break;
	case VCD_SPUV_PCM_LOOPBACK_START_REQ:
		vcd_pr_if_spuv(VCD_SPUV_PCM_LOOPBACK_START_REQ_LOG);
		break;
	case VCD_SPUV_PCM_LOOPBACK_STOP_REQ:
		vcd_pr_if_spuv(VCD_SPUV_PCM_LOOPBACK_STOP_REQ_LOG);
		break;
	case VCD_SPUV_BBIF_LOOPBACK_START_REQ:
		vcd_pr_if_spuv(VCD_SPUV_BBIF_LOOPBACK_START_REQ_LOG);
		break;
	case VCD_SPUV_BBIF_LOOPBACK_STOP_REQ:
		vcd_pr_if_spuv(VCD_SPUV_BBIF_LOOPBACK_STOP_REQ_LOG);
		break;
	case VCD_SPUV_TRACE_SELECT_REQ:
		vcd_pr_if_spuv(VCD_SPUV_TRACE_SELECT_REQ_LOG);
		break;
	case VCD_SPUV_UDATA_REQ:
		vcd_pr_if_spuv(VCD_SPUV_UDATA_REQ_LOG);
		break;
	case VCD_SPUV_TTY_CTM_START_REQ:
		vcd_pr_if_spuv(VCD_SPUV_TTY_CTM_START_REQ_LOG);
		break;
	case VCD_SPUV_TTY_CTM_STOP_REQ:
		vcd_pr_if_spuv(VCD_SPUV_TTY_CTM_STOP_REQ_LOG);
		break;
	case VCD_SPUV_TTY_CTM_CONFIG_REQ:
		vcd_pr_if_spuv(VCD_SPUV_TTY_CTM_CONFIG_REQ_LOG);
		break;
	case VCD_SPUV_BOOT_COMPLETE_IND:
		vcd_pr_if_spuv(VCD_SPUV_BOOT_COMPLETE_IND_LOG);
		break;
	case VCD_SPUV_SYSTEM_ERROR_IND:
		vcd_pr_if_spuv(VCD_SPUV_SYSTEM_ERROR_IND_LOG);
		break;
	case VCD_SPUV_SYSTEM_INFO_IND:
		vcd_pr_if_spuv(VCD_SPUV_SYSTEM_INFO_IND_LOG);
		break;
	case VCD_SPUV_TRIGGER_PLAY_IND:
		vcd_pr_if_spuv(VCD_SPUV_TRIGGER_PLAY_IND_LOG);
		break;
	case VCD_SPUV_TRIGGER_REC_IND:
		vcd_pr_if_spuv(VCD_SPUV_TRIGGER_REC_IND_LOG);
		break;
	case VCD_SPUV_ACTIVE_CNF:
		vcd_pr_if_spuv(VCD_SPUV_ACTIVE_CNF_LOG);
		break;
	case VCD_SPUV_SPEECH_START_CNF:
		vcd_pr_if_spuv(VCD_SPUV_SPEECH_START_CNF_LOG);
		break;
	case VCD_SPUV_SPEECH_STOP_CNF:
		vcd_pr_if_spuv(VCD_SPUV_SPEECH_STOP_CNF_LOG);
		break;
	case VCD_SPUV_VOICE_RECORDING_START_CNF:
		vcd_pr_if_spuv(VCD_SPUV_VOICE_RECORDING_START_CNF_LOG);
		break;
	case VCD_SPUV_VOICE_RECORDING_STOP_CNF:
		vcd_pr_if_spuv(VCD_SPUV_VOICE_RECORDING_STOP_CNF_LOG);
		break;
	case VCD_SPUV_VOICE_PLAYING_START_CNF:
		vcd_pr_if_spuv(VCD_SPUV_VOICE_PLAYING_START_CNF_LOG);
		break;
	case VCD_SPUV_VOICE_PLAYING_STOP_CNF:
		vcd_pr_if_spuv(VCD_SPUV_VOICE_PLAYING_STOP_CNF_LOG);
		break;
	case VCD_SPUV_1KHZ_TONE_START_CNF:
		vcd_pr_if_spuv(VCD_SPUV_1KHZ_TONE_START_CNF_LOG);
		break;
	case VCD_SPUV_1KHZ_TONE_STOP_CNF:
		vcd_pr_if_spuv(VCD_SPUV_1KHZ_TONE_STOP_CNF_LOG);
		break;
	case VCD_SPUV_PCM_LOOPBACK_START_CNF:
		vcd_pr_if_spuv(VCD_SPUV_PCM_LOOPBACK_START_CNF_LOG);
		break;
	case VCD_SPUV_PCM_LOOPBACK_STOP_CNF:
		vcd_pr_if_spuv(VCD_SPUV_PCM_LOOPBACK_STOP_CNF_LOG);
		break;
	case VCD_SPUV_BBIF_LOOPBACK_START_CNF:
		vcd_pr_if_spuv(VCD_SPUV_BBIF_LOOPBACK_START_CNF_LOG);
		break;
	case VCD_SPUV_BBIF_LOOPBACK_STOP_CNF:
		vcd_pr_if_spuv(VCD_SPUV_BBIF_LOOPBACK_STOP_CNF_LOG);
		break;
	case VCD_SPUV_TRACE_SELECT_CNF:
		vcd_pr_if_spuv(VCD_SPUV_TRACE_SELECT_CNF_LOG);
		break;
	case VCD_SPUV_UDATA_IND:
		vcd_pr_if_spuv(VCD_SPUV_UDATA_IND_LOG);
		break;
	case VCD_SPUV_TTY_CTM_START_CNF:
		vcd_pr_if_spuv(VCD_SPUV_TTY_CTM_START_CNF_LOG);
		break;
	case VCD_SPUV_TTY_CTM_STOP_CNF:
		vcd_pr_if_spuv(VCD_SPUV_TTY_CTM_STOP_CNF_LOG);
		break;
	case VCD_SPUV_TTY_CTM_CONFIG_CNF:
		vcd_pr_if_spuv(VCD_SPUV_TTY_CTM_CONFIG_CNF_LOG);
		break;
	default:
		vcd_pr_if_spuv("unkown msg[%x].\n", msg);
	}

	return;
}


/* ========================================================================= */
/* FW info functions                                                         */
/* ========================================================================= */

/**
 * @brief	stop playback function.
 *
 * @param	fw_id	interface id.
 * @param	msg_id	message id.
 *
 * @retval	none.
 */
static void vcd_spuv_set_wait_fw_info(unsigned int fw_id, unsigned int msg_id)
{
	vcd_pr_start_spuv_function("fw_id[%x],msg_id[%x].\n", fw_id, msg_id);

	g_vcd_spuv_info.wait_fw_if_id = fw_id;
	g_vcd_spuv_info.wait_fw_msg_id = msg_id;
	g_vcd_spuv_info.fw_result = VCD_ERR_NONE;

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	stop playback function.
 *
 * @param	fw_id	interface id.
 * @param	msg_id	message id.
 * @param	result	fw result.
 *
 * @retval	none.
 */
static void vcd_spuv_check_wait_fw_info(unsigned int fw_id, unsigned int msg_id,
							unsigned int result)
{
	vcd_pr_start_spuv_function("fw_id[%x], msg_id[%x], result[%x].\n",
		fw_id, msg_id, result);

	if (VCD_SPUV_BOOT_COMPLETE_IND == msg_id) {
		if ((g_vcd_spuv_info.wait_fw_if_id != fw_id) ||
		(g_vcd_spuv_info.wait_fw_msg_id != msg_id)) {
			g_vcd_spuv_info.fw_result = VCD_ERR_SYSTEM;
		}
	} else if ((g_vcd_spuv_info.wait_fw_if_id != fw_id) ||
		(g_vcd_spuv_info.wait_fw_msg_id != msg_id) ||
		(VCD_SPUV_FW_RESULT_SUCCESS != result)) {
		g_vcd_spuv_info.fw_result = VCD_ERR_SYSTEM;
	}

	vcd_pr_end_spuv_function();
	return;
}


/* ========================================================================= */
/* Status functions                                                          */
/* ========================================================================= */

/**
 * @brief	get spuv status function.
 *
 * @param	none.
 *
 * @retval	active feature.
 */
static unsigned int vcd_spuv_get_status(void)
{
	vcd_pr_start_spuv_function();
	vcd_pr_end_spuv_function("g_vcd_spuv_info.status[0x%08x].\n",
		g_vcd_spuv_info.status);
	return g_vcd_spuv_info.status;
}


/**
 * @brief	set spuv status function.
 *
 * @param	status	set status.
 *
 * @retval	none.
 */
static void vcd_spuv_set_status(unsigned int status)
{
	vcd_pr_start_spuv_function("status[0x%08x].\n", status);

	vcd_pr_status_change("g_vcd_spuv_info.status[0x%08x] -> [0x%08x].\n",
		g_vcd_spuv_info.status, (g_vcd_spuv_info.status | status));
	g_vcd_spuv_info.status = g_vcd_spuv_info.status | status;

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	unset spuv status function.
 *
 * @param[in]	status	unset status.
 *
 * @retval	none.
 */
static void vcd_spuv_unset_status(unsigned int status)
{
	vcd_pr_start_spuv_function("status[0x%08x].\n", status);

	vcd_pr_status_change("g_vcd_spuv_info.status[0x%08x] -> [0x%08x].\n",
		g_vcd_spuv_info.status, (g_vcd_spuv_info.status & ~status));
	g_vcd_spuv_info.status = g_vcd_spuv_info.status & ~status;

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	check result function.
 *
 * @param	none.
 *
 * @retval	VCD_ERR_NONE		successful.
 * @retval	VCD_ERR_FW_TIME_OUT	timeout occurred.
 * @retval	VCD_ERR_SYSTEM		other error.
 */
static int vcd_spuv_check_result(void)
{
	vcd_pr_start_spuv_function();

	if (VCD_SPUV_STATUS_SYSTEM_ERROR & g_vcd_spuv_info.status) {
		/* update result */
		g_vcd_spuv_info.fw_result = VCD_ERR_SYSTEM;
	} else if ((VCD_SPUV_STATUS_WAIT_ACK & g_vcd_spuv_info.status) ||
		(VCD_SPUV_STATUS_WAIT_REQ & g_vcd_spuv_info.status)) {
		vcd_pr_if_spuv("V <-- F : TIME OUT.\n");
		/* update status */
		vcd_spuv_set_status(VCD_SPUV_STATUS_SYSTEM_ERROR);
		/* update result */
		g_vcd_spuv_info.fw_result = VCD_ERR_FW_TIME_OUT;
		/* fw stop notification */
		vcd_ctrl_stop_fw();
	} else if (VCD_ERR_NONE != g_vcd_spuv_info.fw_result) {
		/* update status */
		vcd_spuv_set_status(VCD_SPUV_STATUS_SYSTEM_ERROR);
		/* update result */
		g_vcd_spuv_info.fw_result = VCD_ERR_SYSTEM;
		/* fw stop notification */
		vcd_ctrl_stop_fw();
	}

	vcd_pr_end_spuv_function("ret[%d].\n", g_vcd_spuv_info.fw_result);
	return g_vcd_spuv_info.fw_result;
}


/* ========================================================================= */
/* Dump functions                                                            */
/* ========================================================================= */

/**
 * @brief	dump status function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_spuv_dump_status(void)
{
	vcd_pr_start_spuv_function();

	vcd_pr_registers_dump("g_vcd_spuv_info.status         [%08x].\n",
		vcd_spuv_get_status());
	vcd_pr_registers_dump("g_vcd_spuv_info.wait_fw_if_id  [%08x].\n",
		g_vcd_spuv_info.wait_fw_if_id);
	vcd_pr_registers_dump("g_vcd_spuv_info.wait_fw_msg_id [%08x].\n",
		g_vcd_spuv_info.wait_fw_msg_id);
	vcd_pr_registers_dump("g_vcd_spuv_info.fw_result      [%08x].\n",
		g_vcd_spuv_info.fw_result);

	vcd_pr_end_spuv_function();
	return;
}

/**
 * @brief	dump related registers function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_spuv_dump_registers(void)
{
	vcd_pr_start_spuv_function();

	vcd_spuv_func_dump_cpg_registers();
	vcd_spuv_func_dump_crmu_registers();
	vcd_spuv_func_dump_gtu_registers();
	vcd_spuv_func_dump_voiceif_registers();
	vcd_spuv_func_dump_intcvo_registers();
	vcd_spuv_func_dump_spuv_registers();
	vcd_spuv_func_dump_dsp0_registers();

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	dump CPG registers function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_spuv_dump_cpg_registers(void)
{
	vcd_pr_start_spuv_function();

	vcd_spuv_func_dump_cpg_registers();

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	dump CRMU registers function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_spuv_dump_crmu_registers(void)
{
	vcd_pr_start_spuv_function();

	vcd_spuv_func_dump_crmu_registers();

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	dump GTU registers function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_spuv_dump_gtu_registers(void)
{
	vcd_pr_start_spuv_function();

	vcd_spuv_func_dump_gtu_registers();

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	dump VOICEIF registers function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_spuv_dump_voiceif_registers(void)
{
	vcd_pr_start_spuv_function();

	vcd_spuv_func_dump_voiceif_registers();

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	dump INTCVO registers function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_spuv_dump_intcvo_registers(void)
{
	vcd_pr_start_spuv_function();

	vcd_spuv_func_dump_intcvo_registers();

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	dump spuv registers function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_spuv_dump_spuv_registers(void)
{
	vcd_pr_start_spuv_function();

	vcd_spuv_func_dump_spuv_registers();

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	dump dsp0 registers function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_spuv_dump_dsp0_registers(void)
{
	vcd_pr_start_spuv_function();

	vcd_spuv_func_dump_dsp0_registers();

	vcd_pr_end_spuv_function();
	return;
}
