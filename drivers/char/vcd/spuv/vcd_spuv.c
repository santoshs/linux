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
#include <linux/kthread.h>
#include <linux/slab.h>
#include <mach/pm.h>

#include "linux/vcd/vcd_common.h"
#include "linux/vcd/vcd_control.h"
#include "vcd_spuv.h"
#include "vcd_spuv_func.h"


/*
 * global variable declaration
 */
struct vcd_spuv_info g_vcd_spuv_info;

static struct vcd_spuv_workqueue  *g_vcd_spuv_work_queue;
static struct vcd_spuv_work       g_vcd_spuv_interrupt_ack;
static struct vcd_spuv_work       g_vcd_spuv_interrupt_req;

static struct vcd_spuv_workqueue  *g_vcd_spuv_notify_queue;
static struct vcd_spuv_work       g_vcd_spuv_rec_trigger;
static struct vcd_spuv_work       g_vcd_spuv_play_trigger;
static struct vcd_spuv_work       g_vcd_spuv_system_error;
static struct vcd_spuv_work       g_vcd_spuv_udata_ind;


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
 * @brief	initialize register function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_spuv_init_register(void)
{
	vcd_pr_start_spuv_function();

	vcd_spuv_func_set_hpb_register();
	vcd_spuv_func_set_cpg_register();

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

	vcd_spuv_func_cacheflush(
		SPUV_FUNC_SDRAM_PROC_MSG_BUFFER,
		(PAGE_SIZE*2));

	vcd_pr_end_spuv_function("buf_addr[%d].\n", buf_addr);
	return buf_addr;
}


/**
 * @brief	get asyncrnous return area function.
 *
 * @param	none.
 *
 * @retval	buf_addr	msg buffer physical address.
 */
int vcd_spuv_get_async_area(void)
{
	int buf_addr = 0;

	vcd_pr_start_spuv_function();

	vcd_spuv_func_initialize();

	buf_addr = (int)__get_free_pages(GFP_KERNEL, 1);

	vcd_pr_end_spuv_function("buf_addr lo[0x%08x] buf_addr ph[0x%08x].\n",
		buf_addr, (unsigned int)__pa(buf_addr));
	return buf_addr;
}


/**
 * @brief	get asyncrnous return area function.
 *
 * @param	adr	asyncrnous address.
 *
 * @retval	ret	result.
 */
int vcd_spuv_free_async_area(unsigned int adr)
{
	int ret = 0;

	vcd_pr_start_spuv_function();

	if (0 != adr)
		free_pages((unsigned long)adr, 1);

	vcd_pr_end_spuv_function("ret[%d].\n", ret);
	return ret;
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

	/* register dump */
	vcd_spuv_dump_registers();

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

	/* ending on clkgen is notified */
	vcd_ctrl_stop_clkgen();

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
	vcd_spuv_func_cacheflush((unsigned int)proc_param, PAGE_SIZE);

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
	int param_num = VCD_SPUV_SPEECH_START_LENGTH;

	vcd_pr_start_spuv_function();

	/* set status */
	vcd_spuv_set_status(VCD_SPUV_STATUS_WAIT_ACK |
				VCD_SPUV_STATUS_WAIT_REQ);

	/* set expected response*/
	vcd_spuv_set_wait_fw_info(VCD_SPUV_INTERFACE_ID,
				VCD_SPUV_SPEECH_START_CNF);

	/* flush cache */
	vcd_spuv_func_cacheflush((unsigned int)proc_param, PAGE_SIZE);

	/* set parameter */
	param[0] = VCD_SPUV_INTERFACE_ID;
	param[1] = VCD_SPUV_SPEECH_START_REQ;
	param[2] = proc_param[0];
	if (VCD_CALL_TYPE_VOIP == proc_param[0]) {
		param[3] = 0;
		param[4] = ((vcd_spuv_func_sdram_logical_to_physical(
			SPUV_FUNC_SDRAM_VOIP_DL_TEMP_BUFFER_0) >> 16) &
			0x0000FFFF);
		param[5] = (vcd_spuv_func_sdram_logical_to_physical(
			SPUV_FUNC_SDRAM_VOIP_DL_TEMP_BUFFER_0) & 0x0000FFFF);
		param[6] = ((vcd_spuv_func_sdram_logical_to_physical(
			SPUV_FUNC_SDRAM_VOIP_UL_TEMP_BUFFER_0) >> 16) &
			0x0000FFFF);
		param[7] = (vcd_spuv_func_sdram_logical_to_physical(
			SPUV_FUNC_SDRAM_VOIP_UL_TEMP_BUFFER_0) & 0x0000FFFF);

		/* set parameter num */
		param_num = VCD_SPUV_VOIP_SPEECH_START_LENGTH;
		/* initialize VoIP buffer ID */
		vcd_spuv_func_init_voip_ul_buffer_id();
		vcd_spuv_func_init_voip_dl_buffer_id();

		/* patch start */
		proc_param[1] = 44100;
		proc_param[2] = 44100;
		/* patch end */

		/* SRC initialize */
		/* UL : proc_param[1], DL : proc_param[2], spuv : 16kHz */
		vcd_spuv_func_resampler_init(
			proc_param[1],
			proc_param[2],
			16000);
	}

	/* wait path set */
	vcd_ctrl_wait_path();

	/* output msg log */
	vcd_spuv_interface_log(param[1]);

	/* send message */
	vcd_spuv_func_send_msg(param, param_num);

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
	vcd_spuv_func_cacheflush((unsigned int)proc_param, PAGE_SIZE);

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
	return;
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
	return;
}


/**
 * @brief	get VoIP UL buffer function.
 *
 * @param	info	buffer information.
 *
 * @retval	none.
 */
void vcd_spuv_get_voip_ul_buffer(struct vcd_voip_ul_buffer_info *info)
{
	vcd_pr_start_spuv_function("info[%p].\n", info);

	/* set buffer address */
	info->voip_ul_buffer[0] =
		(unsigned int *)SPUV_FUNC_SDRAM_VOIP_UL_BUFFER_0;
	info->voip_ul_buffer[1] =
		(unsigned int *)SPUV_FUNC_SDRAM_VOIP_UL_BUFFER_1;

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	get VoIP DL buffer function.
 *
 * @param	info	buffer information.
 *
 * @retval	none.
 */
void vcd_spuv_get_voip_dl_buffer(struct vcd_voip_dl_buffer_info *info)
{
	vcd_pr_start_spuv_function("info[%p].\n", info);

	/* set buffer address */
	info->voip_dl_buffer[0] =
		(unsigned int *)SPUV_FUNC_SDRAM_VOIP_DL_BUFFER_0;
	info->voip_dl_buffer[1] =
		(unsigned int *)SPUV_FUNC_SDRAM_VOIP_DL_BUFFER_1;

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	initialize record buffer ID function. (for VoIP)
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_spuv_init_record_buffer_id(void)
{
	vcd_pr_start_spuv_function();

	vcd_spuv_func_init_record_buffer_id();

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	initialize playback buffer ID function. (for VoIP)
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_spuv_init_playback_buffer_id(void)
{
	vcd_pr_start_spuv_function();

	vcd_spuv_func_init_playback_buffer_id();

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	set VoIP UL buffer ID function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_spuv_update_voip_ul_buffer_id(void)
{
	vcd_pr_start_spuv_function();

	vcd_spuv_func_set_voip_ul_buffer_id();

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	set VoIP DL buffer ID function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_spuv_update_voip_dl_buffer_id(void)
{
	vcd_pr_start_spuv_function();

	vcd_spuv_func_set_voip_dl_buffer_id();

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	VoIP UL function.
 *
 * @param[out]	buf_size	buffer size.
 *
 * @retval	none.
 */
void vcd_spuv_voip_ul(unsigned int *buf_size)
{
	vcd_pr_start_spuv_function();

	vcd_spuv_func_voip_ul(buf_size);

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	VoIP DL function.
 *
 * @param[out]	buf_size	buffer size.
 *
 * @retval	none.
 */
void vcd_spuv_voip_dl(unsigned int *buf_size)
{
	vcd_pr_start_spuv_function();

	vcd_spuv_func_voip_dl(buf_size);

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	VoIP UL playback function.
 *
 * @param	mode	playback mode
 *
 * @retval	none.
 */
void vcd_spuv_voip_ul_playback(unsigned int mode)
{
	vcd_pr_start_spuv_function("mode[%d].\n", mode);

	if (VCD_PLAYBACK_MODE_0 == mode) {
		/* nop */
		vcd_spuv_func_voip_ul_playback_mode0();
	} else if (VCD_PLAYBACK_MODE_1 == mode) {
		/* overwrite playback data */
		vcd_spuv_func_voip_ul_playback_mode1();
	} else {
		/* overwrite no voice data */
		vcd_spuv_func_voip_ul_playback_mode2();
	}

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	VoIP DL playback function.
 *
 * @param	mode	playback mode
 *
 * @retval	none.
 */
void vcd_spuv_voip_dl_playback(unsigned int mode)
{
	vcd_pr_start_spuv_function("mode[%d].\n", mode);

	if (VCD_PLAYBACK_MODE_0 == mode) {
		/* mixing "DL voice data" and "playback data" */
		vcd_spuv_func_voip_dl_playback_mode0();
	} else if (VCD_PLAYBACK_MODE_1 == mode) {
		/* overwrite no voice data */
		vcd_spuv_func_voip_dl_playback_mode1();
	} else {
		/* overwrite playback data */
		vcd_spuv_func_voip_dl_playback_mode2();
	}

	vcd_pr_end_spuv_function();
	return;
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
	if (VCD_LOOPBACK_MODE_DELAY == mode)
		param[3] = VCD_SPUV_LOOPBACK_DELAY_500;
	else
		param[3] = VCD_SPUV_LOOPBACK_DELAY_0;

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


/**
 * @brief	get call type function.
 *
 * @param	none.
 *
 * @retval	call_type	call type (CS/VoIP).
 */
int vcd_spuv_get_call_type(void)
{
	int call_type = VCD_CALL_TYPE_CS;
	int *proc_param = (int *)SPUV_FUNC_SDRAM_PROC_MSG_BUFFER;

	vcd_pr_start_spuv_function();

	/* flush cache */
	vcd_spuv_func_cacheflush((unsigned int)proc_param, PAGE_SIZE);
	if (0x03000000 & g_vcd_log_level)/* VoIP Loopback [SRC] */
		*proc_param = VCD_CALL_TYPE_VOIP;

	if (VCD_CALL_TYPE_CS == *proc_param) {
		call_type = VCD_CALL_TYPE_CS;
	} else if (VCD_CALL_TYPE_VOIP == *proc_param) {
		call_type = VCD_CALL_TYPE_VOIP;
	} else if (VCD_CALL_TYPE_VOLTE == *proc_param) {
		call_type = VCD_CALL_TYPE_VOLTE;
	} else {
		vcd_pr_err("Unknown call type[%d].\n", *proc_param);
		call_type = VCD_CALL_TYPE_CS;
	}
	vcd_pr_end_spuv_function("call_type[%d].\n", call_type);
	return call_type;
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
		vcd_pr_spuv_debug("execute request_irq().\n");
		ret = request_irq(VCD_SPUV_SPI_NO, vcd_spuv_irq_handler,
				0, "SPU2V DSP", NULL);
		if (VCD_ERR_NONE == ret)
			g_vcd_spuv_info.irq_status = VCD_SPUV_ENABLE;
		break;
	case VCD_SPUV_DISABLE:
		/* Unbind SPUV interrupt */
		if (VCD_SPUV_ENABLE == g_vcd_spuv_info.irq_status) {
			vcd_pr_spuv_debug("execute free_irq().\n");
			free_irq(VCD_SPUV_SPI_NO, NULL);
			g_vcd_spuv_info.irq_status = VCD_SPUV_DISABLE;
		}
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

	vcd_pr_start_irq_function("irq[%d],dev_id[%p].\n", irq, dev_id);

	/* interrupt masked */
	vcd_spuv_func_set_register(
		(VCD_SPUV_FUNC_AMSGIT_MSG_REQ | VCD_SPUV_FUNC_AMSGIT_MSG_ACK),
		SPUV_FUNC_RW_32_AINTMASK);

	/* read AINTSTS register */
	vcd_spuv_func_register(SPUV_FUNC_RO_32_AINTSTS, aintsts);

	/* set AINTCLR register */
	vcd_spuv_func_set_register(aintsts, SPUV_FUNC_RW_32_AINTCLR);
	udelay(1);
	vcd_spuv_func_set_register(aintsts, SPUV_FUNC_RW_32_AINTCLR);

	/* generate IEVENTC interrupt */
	vcd_spuv_func_register(SPUV_FUNC_RW_32_IEVENTC, ieventc);
	vcd_spuv_func_set_register((~ieventc & VCD_SPUV_IEVENTC),
					SPUV_FUNC_RW_32_IEVENTC);

	/* interrupt masked */
	vcd_spuv_func_set_register(
		VCD_SPUV_FUNC_DISABLE,
		SPUV_FUNC_RW_32_AINTMASK);

	/* entry queue */
	if (VCD_SPUV_AINT_ACK & aintsts) {
		vcd_pr_irq_debug("interrupt Ack.\n");
		vcd_spuv_workqueue_enqueue(g_vcd_spuv_work_queue,
					&g_vcd_spuv_interrupt_ack);
	}

	if (VCD_SPUV_AINT_REQ & aintsts) {
		vcd_pr_irq_debug("interrupt Req.\n");
		vcd_spuv_workqueue_enqueue(g_vcd_spuv_work_queue,
					&g_vcd_spuv_interrupt_req);
	}

	vcd_pr_end_irq_function("return IRQ_HANDLED.\n");
	return IRQ_HANDLED;
}


/* ========================================================================= */
/* Queue functions                                                           */
/* ========================================================================= */

/**
 * @brief	queue work initialize function.
 *
 * @param[in]	work	queue work.
 * @param[in]	func	queue function.
 *
 * @retval	none.
 */
static void vcd_spuv_work_initialize(
	struct vcd_spuv_work *work, void (*func)(void))
{
	vcd_pr_start_spuv_function("work[%p]func[%p]", work, func);

	INIT_LIST_HEAD(&work->link);
	work->func = func;
	work->status = 0;

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	destroy workqueue function.
 *
 * @param[in]	wq	workqueue.
 *
 * @retval	none.
 */
static void vcd_spuv_workqueue_destroy(struct vcd_spuv_workqueue *wq)
{
	unsigned long flags;

	vcd_pr_start_spuv_function("wq[%p]", wq);

	if (wq == NULL) {
		/* report error */
	} else {
		/* request task stop */
		if (wq->task)
			kthread_stop(wq->task);

		/* wakeup pending thread */
		spin_lock_irqsave(&wq->lock, flags);

		while (!list_empty(&wq->top)) {
			struct list_head *list;
			struct vcd_spuv_work *work = NULL;

			list_for_each(list, &wq->top)
			{
				work = list_entry(list,
					struct vcd_spuv_work, link);
				break;
			}
			if (work) {
				work->status = 1;
				list_del_init(&work->link);
			}
		}
		spin_unlock_irqrestore(&wq->lock, flags);

		wake_up_interruptible_all(&wq->wait);

		kfree(wq);
	}

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	workqueue thread function.
 *
 * @param[in]	arg	workqueue.
 *
 * @retval	VCD_ERR_NONE.
 */
static inline int vcd_spuv_workqueue_thread(void *arg)
{
	struct vcd_spuv_workqueue *wq = (struct vcd_spuv_workqueue *)arg;
	unsigned long flags;

	vcd_pr_start_spuv_function("arg[%p]", arg);

	/* set schedule */
	vcd_spuv_set_schedule();

	/* dev->th_events already initialized 0. */
	while (!kthread_should_stop()) {
		struct vcd_spuv_work *work = NULL;
		void   (*func)(void);

		wait_event_interruptible(wq->wait, !list_empty(&wq->top));

		if (kthread_should_stop())
			break;

		spin_lock_irqsave(&wq->lock, flags);
		while (!list_empty(&wq->top)) {
			work = list_first_entry(&wq->top,
				struct vcd_spuv_work, link);

			func = work->func;
			work_clear_pending(work);
			list_del_init(&work->link);
			spin_unlock_irqrestore(&wq->lock, flags);

			(*func)();

			spin_lock_irqsave(&wq->lock, flags);
			work->status = 1;
			wake_up_all(&wq->finish);
		}
		spin_unlock_irqrestore(&wq->lock, flags);
	}

	vcd_pr_end_spuv_function();
	return VCD_ERR_NONE;
}


/**
 * @brief	create workqueue function.
 *
 * @param[in]	taskname	queue name.
 *
 * @retval	wq		workqueue.
 */
static struct vcd_spuv_workqueue *vcd_spuv_workqueue_create(char *taskname)
{
	struct vcd_spuv_workqueue *wq;

	vcd_pr_start_spuv_function();

	wq = kmalloc(sizeof(*wq), GFP_KERNEL);

	if (wq == NULL) {
		vcd_pr_err("kmalloc error.\n");
	} else {
		memset(wq, 0, sizeof(*wq));

		INIT_LIST_HEAD(&wq->top);
		spin_lock_init(&wq->lock);
		init_waitqueue_head(&wq->wait);
		init_waitqueue_head(&wq->finish);

		wq->task = kthread_run(vcd_spuv_workqueue_thread,
				     wq,
				     taskname);
		if (IS_ERR(wq->task)) {
			kfree(wq);
			wq = NULL;
		}
	}

	vcd_pr_end_spuv_function("wq[%p]", wq);
	return wq;
}


/**
 * @brief	enqueue workqueue function.
 *
 * @param[in]	wq	workqueue.
 * @param[in]	work	queue work.
 *
 * @retval	none.
 */
static void vcd_spuv_workqueue_enqueue(
	struct vcd_spuv_workqueue *wq, struct vcd_spuv_work *work)
{
	unsigned long flags;

	vcd_pr_start_spuv_function("wq[%p]work[%p]", wq, work);

	if (wq && work) {
		spin_lock_irqsave(&wq->lock, flags);
		if (!test_and_set_bit(
			WORK_STRUCT_PENDING_BIT, work_data_bits(work))) {
			if (!list_empty(&work->link))
				vcd_pr_err("sequence violation. type1.\n");
			list_add_tail(&work->link, &wq->top);
		} else {
			vcd_pr_err("sequence violation. type2.\n");
		}
		spin_unlock_irqrestore(&wq->lock, flags);
		wake_up_interruptible(&wq->wait);
	} else {
		vcd_pr_err("parameter error. wq[%p]work[%p].\n", wq, work);
	}

	vcd_pr_end_spuv_function();
	return;
}


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
			vcd_spuv_workqueue_create("vcd_spuv_work_queue");
	if (NULL == g_vcd_spuv_work_queue) {
		vcd_pr_err("queue create error.\n");
		ret = VCD_ERR_NOMEMORY;
	} else {
		vcd_spuv_work_initialize(&g_vcd_spuv_interrupt_ack,
						vcd_spuv_interrupt_ack);
		vcd_spuv_work_initialize(&g_vcd_spuv_interrupt_req,
						vcd_spuv_interrupt_req);
	}

	/* queue create for notify */
	g_vcd_spuv_notify_queue =
			vcd_spuv_workqueue_create("vcd_spuv_notify_queue");
	if (NULL == g_vcd_spuv_notify_queue) {
		vcd_pr_err("notify queue create error.\n");
		ret = VCD_ERR_NOMEMORY;
	} else {
		vcd_spuv_work_initialize(&g_vcd_spuv_rec_trigger,
						vcd_spuv_rec_trigger);
		vcd_spuv_work_initialize(&g_vcd_spuv_play_trigger,
						vcd_spuv_play_trigger);
		vcd_spuv_work_initialize(&g_vcd_spuv_system_error,
						vcd_spuv_system_error);
		vcd_spuv_work_initialize(&g_vcd_spuv_udata_ind,
						vcd_spuv_udata_ind);
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

	vcd_spuv_workqueue_destroy(g_vcd_spuv_work_queue);
	vcd_spuv_workqueue_destroy(g_vcd_spuv_notify_queue);

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
 * @param	none.
 *
 * @retval	none.
 */
static void vcd_spuv_interrupt_ack(void)
{
	int ret = VCD_ERR_NONE;

	vcd_pr_start_spuv_function();

	/* set schedule */
	vcd_spuv_set_schedule();

	vcd_pr_if_spuv("[VCD <- SPUV ] : ACK\n");

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
 * @param	none.
 *
 * @retval	none.
 */
static void vcd_spuv_interrupt_req(void)
{
	int ret = VCD_ERR_NONE;
	int i = 0;
	int is_ack_log_enable = VCD_SPUV_FUNC_ENABLE;
	unsigned int *fw_req = (int *)SPUV_FUNC_SDRAM_FW_RESULT_BUFFER;
	unsigned int spuv_status = VCD_SPUV_STATUS_NONE;
	unsigned int *latest_sys_info =
		(unsigned int *)SPUV_FUNC_SDRAM_SYSTEM_INFO_BUFFER;
	unsigned int *rcv_msg_buf =
		(unsigned int *)(SPUV_FUNC_SDRAM_PROC_MSG_BUFFER + PAGE_SIZE);

	vcd_pr_start_spuv_function();

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
		/* status update */
		vcd_spuv_set_status(VCD_SPUV_STATUS_SYSTEM_ERROR);
		/* copy SYSTEM_INFO_IND data length */
		rcv_msg_buf[0] = latest_sys_info[0];
		/* copy latest SYSTEM_INFO_IND data to MSG_BUFFER */
		if (0 < rcv_msg_buf[0])
			memcpy(
				(void *)&rcv_msg_buf[1],
				(void *)&latest_sys_info[1],
				(sizeof(unsigned int) * rcv_msg_buf[0])
			);

		/* notify SYSTEM_ERROR_IND */
		vcd_spuv_workqueue_enqueue(g_vcd_spuv_notify_queue,
						&g_vcd_spuv_system_error);
		break;
	case VCD_SPUV_SYSTEM_INFO_IND:
		/* copy SYSTEM_INFO_IND data length */
		latest_sys_info[0] = fw_req[2];
		/* copy latest SYSTEM_INFO_IND data */
		if (0 < latest_sys_info[0])
			memcpy(
				(void *)&latest_sys_info[1],
				(void *)&fw_req[3],
				(sizeof(unsigned int) * latest_sys_info[0])
			);
		/* SYSTEM_INFO_IND data length output */
		vcd_pr_spuv_info(
			"system info length[%d].\n",
			latest_sys_info[0]
		);
		/* SYSTEM_INFO_IND data output */
		for (i = 0; i < latest_sys_info[0]; i++)
			vcd_pr_spuv_debug(
				"system info[%d][%x].\n",
				i, latest_sys_info[1+i]
			);
		break;
	case VCD_SPUV_UDATA_IND:
		/* copy UDATA_IND data length */
		rcv_msg_buf[0] = fw_req[2];
		/* copy UDATA_IND data */
		if (0 < rcv_msg_buf[0])
			memcpy(
				(void *)&rcv_msg_buf[1],
				(void *)&fw_req[3],
				(sizeof(unsigned int) * rcv_msg_buf[0])
			);
		/* notify UDATA_IND */
		vcd_spuv_workqueue_enqueue(g_vcd_spuv_notify_queue,
						&g_vcd_spuv_udata_ind);
		break;
	case VCD_SPUV_TRIGGER_REC_IND:
		vcd_spuv_workqueue_enqueue(g_vcd_spuv_notify_queue,
						&g_vcd_spuv_rec_trigger);
		break;
	case VCD_SPUV_TRIGGER_PLAY_IND:
		vcd_spuv_workqueue_enqueue(g_vcd_spuv_notify_queue,
						&g_vcd_spuv_play_trigger);
		break;
	default:
		/* get status */
		spuv_status = vcd_spuv_get_status();
		if (!(VCD_SPUV_STATUS_WAIT_REQ & spuv_status)) {
			/* status update */
			vcd_spuv_set_status(VCD_SPUV_STATUS_SYSTEM_ERROR);
			/* notification fw stop */
			vcd_ctrl_stop_fw();
			break;
		}

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
	is_ack_log_enable = vcd_spuv_is_log_enable(fw_req[1]);
	vcd_spuv_func_send_ack(is_ack_log_enable);
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
 * @param	none.
 *
 * @retval	none.
 */
static void vcd_spuv_rec_trigger(void)
{
	vcd_pr_start_spuv_function();

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
 * @param	none.
 *
 * @retval	none.
 */
static void vcd_spuv_play_trigger(void)
{
	vcd_pr_start_spuv_function();

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
 * @param	none.
 *
 * @retval	none.
 */
static void vcd_spuv_system_error(void)
{
	vcd_pr_start_spuv_function();

	/* set schedule */
	vcd_spuv_set_schedule();

	vcd_pr_err("system error occured.\n");

	/* notification fw stop */
	vcd_ctrl_stop_fw();

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	queue out UDATA function.
 *
 * @param	none.
 *
 * @retval	none.
 */
static void vcd_spuv_udata_ind(void)
{
	vcd_pr_start_spuv_function();

	/* set schedule */
	vcd_spuv_set_schedule();

	/* notification fw stop */
	vcd_ctrl_udata_ind();

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	ack log output check function.
 *
 * @param[in]	msg	output msg type.
 *
 * @retval	is_ack_log_enable	log ON/OFF.
 */
static int vcd_spuv_is_log_enable(unsigned int msg)
{
	int is_ack_log_enable = VCD_SPUV_FUNC_DISABLE;

	switch (msg) {
	case VCD_SPUV_SYSTEM_INFO_IND:
		if (g_vcd_log_level & VCD_LOG_ON_SYSTEM_INFO_IND)
			is_ack_log_enable = VCD_SPUV_FUNC_ENABLE;
		break;
	case VCD_SPUV_TRIGGER_PLAY_IND:
		if (g_vcd_log_level & VCD_LOG_ON_TRIGGER_PLAY_IND)
			is_ack_log_enable = VCD_SPUV_FUNC_ENABLE;
		break;
	case VCD_SPUV_TRIGGER_REC_IND:
		if (g_vcd_log_level & VCD_LOG_ON_TRIGGER_REC_IND)
			is_ack_log_enable = VCD_SPUV_FUNC_ENABLE;
		break;
	case VCD_SPUV_UDATA_IND:
		if (g_vcd_log_level & VCD_LOG_ON_UDATA_IND)
			is_ack_log_enable = VCD_SPUV_FUNC_ENABLE;
		break;
	default:
		is_ack_log_enable = VCD_SPUV_FUNC_ENABLE;
		break;
	}

	return is_ack_log_enable;
}


/**
 * @brief	spuv if log output function.
 *
 * @param[in]	msg	output msg type.
 *
 * @retval	none.
 */
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
	case VCD_SPUV_BOOT_COMPLETE_IND:
		vcd_pr_if_spuv(VCD_SPUV_BOOT_COMPLETE_IND_LOG);
		break;
	case VCD_SPUV_SYSTEM_ERROR_IND:
		vcd_pr_if_spuv(VCD_SPUV_SYSTEM_ERROR_IND_LOG);
		break;
	case VCD_SPUV_SYSTEM_INFO_IND:
		vcd_pr_if_spuv_system_info_ind(VCD_SPUV_SYSTEM_INFO_IND_LOG);
		break;
	case VCD_SPUV_TRIGGER_PLAY_IND:
		vcd_pr_if_spuv_trigger_play_ind(VCD_SPUV_TRIGGER_PLAY_IND_LOG);
		break;
	case VCD_SPUV_TRIGGER_REC_IND:
		vcd_pr_if_spuv_trigger_rec_ind(VCD_SPUV_TRIGGER_REC_IND_LOG);
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
		vcd_pr_if_spuv_udata_ind(VCD_SPUV_UDATA_IND_LOG);
		break;
	default:
		vcd_pr_if_spuv("unkown msg[%x].\n", msg);
		break;
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
 * @brief	check wait fw information function.
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

	if (VCD_ERR_NONE != g_vcd_spuv_info.fw_result) {
		vcd_pr_err(
			"Expect:fw_id[0x%08x]msg_id[0x%08x].\n",
			g_vcd_spuv_info.wait_fw_if_id,
			g_vcd_spuv_info.wait_fw_msg_id);
		vcd_pr_err(
			"Result:fw_id[0x%08x]msg_id[0x%08x]result[0x%08x].\n",
			fw_id, msg_id, result);
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
		vcd_pr_if_spuv("[VCD <- SPUV ] : TIME OUT\n");
		vcd_pr_err("firmware time out occured.\n");
		/* update status */
		vcd_spuv_set_status(VCD_SPUV_STATUS_SYSTEM_ERROR);
		/* update result */
		g_vcd_spuv_info.fw_result = VCD_ERR_FW_TIME_OUT;
		/* fw stop notification */
		vcd_ctrl_stop_fw();
	} else if (VCD_ERR_NONE != g_vcd_spuv_info.fw_result) {
		vcd_pr_err("firmware result is not success.\n");
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

	vcd_spuv_func_dump_hpb_registers();
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
 * @brief	dump HPB registers function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_spuv_dump_hpb_registers(void)
{
	vcd_pr_start_spuv_function();

	vcd_spuv_func_dump_hpb_registers();

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


/**
 * @brief	dump memories function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_spuv_dump_memories(void)
{
	vcd_pr_start_spuv_function();

	vcd_spuv_func_dump_pram0_memory();
	vcd_spuv_func_dump_xram0_memory();
	vcd_spuv_func_dump_yram0_memory();
	vcd_spuv_func_dump_dspio_memory();
	vcd_spuv_func_dump_sdram_static_area_memory();
	vcd_spuv_func_dump_fw_static_buffer_memory();

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	dump pram0 memory function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_spuv_dump_pram0_memory(void)
{
	vcd_pr_start_spuv_function();

	vcd_spuv_func_dump_pram0_memory();

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	dump xram0 memory function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_spuv_dump_xram0_memory(void)
{
	vcd_pr_start_spuv_function();

	vcd_spuv_func_dump_xram0_memory();

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	dump yram0 memory function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_spuv_dump_yram0_memory(void)
{
	vcd_pr_start_spuv_function();

	vcd_spuv_func_dump_yram0_memory();

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	dump dspio memory function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_spuv_dump_dspio_memory(void)
{
	vcd_pr_start_spuv_function();

	vcd_spuv_func_dump_dspio_memory();

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	dump sdram static area memory function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_spuv_dump_sdram_static_area_memory(void)
{
	vcd_pr_start_spuv_function();

	vcd_spuv_func_dump_sdram_static_area_memory();

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	dump fw static buffer memory function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_spuv_dump_fw_static_buffer_memory(void)
{
	vcd_pr_start_spuv_function();

	vcd_spuv_func_dump_fw_static_buffer_memory();

	vcd_pr_end_spuv_function();
	return;
}
