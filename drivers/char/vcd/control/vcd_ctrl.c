/* vcd_ctrl.c
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
#include "linux/vcd/vcd_common.h"
#include "linux/vcd/vcd_interface.h"
#include "linux/vcd/vcd_spuv.h"
#include "vcd_ctrl.h"
#include "vcd_ctrl_func.h"

/*
 * global variable declaration
 */
int g_vcd_ctrl_result;

/* ========================================================================= */
/* For AMHAL functions                                                       */
/* ========================================================================= */

/**
 * @brief	get msg buffer function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_ctrl_get_msg_buffer(void)
{
	vcd_pr_start_control_function();

	/* execute spuv function */
	g_vcd_ctrl_result = vcd_spuv_get_msg_buffer();

	vcd_pr_end_control_function("g_vcd_ctrl_result[%x].\n",
		g_vcd_ctrl_result);
	return;
}


/**
 * @brief	vcd start function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_ctrl_start_vcd(void)
{
	vcd_pr_start_control_function();

	/* check sequence */
	g_vcd_ctrl_result =
		vcd_ctrl_func_check_sequence(VCD_CTRL_FUNC_START_VCD);
	if (VCD_ERR_NONE != g_vcd_ctrl_result) {
		vcd_pr_control_info("check sequence[%d].\n",
			g_vcd_ctrl_result);
		g_vcd_ctrl_result =
			vcd_ctrl_func_convert_result(g_vcd_ctrl_result);
		goto rtn;
	}

	/* execute spuv function */
	g_vcd_ctrl_result = vcd_spuv_start_vcd();
	if (VCD_ERR_NONE != g_vcd_ctrl_result) {
		vcd_pr_err("start vcd error[%d].\n", g_vcd_ctrl_result);
		g_vcd_ctrl_result = VCD_ERR_SYSTEM;
		goto rtn;
	}

	/* update active status */
	vcd_ctrl_func_set_active_feature(VCD_CTRL_FUNC_FEATURE_VCD);

rtn:
	vcd_pr_end_control_function("g_vcd_ctrl_result[%d].\n",
		g_vcd_ctrl_result);
	return;
}


/**
 * @brief	vcd stop function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_ctrl_stop_vcd(void)
{
	vcd_pr_start_control_function();

	/* check sequence */
	g_vcd_ctrl_result =
		vcd_ctrl_func_check_sequence(VCD_CTRL_FUNC_STOP_VCD);
	if (VCD_ERR_NONE != g_vcd_ctrl_result) {
		vcd_pr_control_info("check sequence[%d].\n",
			g_vcd_ctrl_result);
		g_vcd_ctrl_result =
			vcd_ctrl_func_convert_result(g_vcd_ctrl_result);
		goto rtn;
	}

	/* execute spuv function */
	g_vcd_ctrl_result = vcd_spuv_stop_vcd();
	if (VCD_ERR_NONE != g_vcd_ctrl_result)
		vcd_pr_control_info("vcd_spuv_stop_vcd[%d].\n",
			g_vcd_ctrl_result);

	/* update result */
	g_vcd_ctrl_result = VCD_ERR_NONE;

	/* update active status */
	vcd_ctrl_func_unset_active_feature(~VCD_CTRL_FUNC_FEATURE_NONE);

rtn:
	vcd_pr_end_control_function("g_vcd_ctrl_result[%d].\n",
		g_vcd_ctrl_result);
	return;
}


/**
 * @brief	set hw param function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_ctrl_set_hw_param(void)
{
	vcd_pr_start_control_function();

	/* check sequence */
	g_vcd_ctrl_result =
		vcd_ctrl_func_check_sequence(VCD_CTRL_FUNC_HW_PARAM);
	if (VCD_ERR_NONE != g_vcd_ctrl_result) {
		vcd_pr_control_info("check sequence[%d].\n",
			g_vcd_ctrl_result);
		g_vcd_ctrl_result =
			vcd_ctrl_func_convert_result(g_vcd_ctrl_result);
		goto rtn;
	}

	/* execute spuv function */
	g_vcd_ctrl_result = vcd_spuv_set_hw_param();
	if (VCD_ERR_NONE != g_vcd_ctrl_result) {
		vcd_pr_err("set hw param error[%d].\n", g_vcd_ctrl_result);
		goto rtn;
	}

	/* update active status */
	vcd_ctrl_func_set_active_feature(VCD_CTRL_FUNC_FEATURE_HW_PARAM);

rtn:
	vcd_pr_end_control_function("g_vcd_ctrl_result[%d].\n",
		g_vcd_ctrl_result);
	return;
}


/**
 * @brief	start call function.
 *
 * @param	call_type	cs/1khz tone/pcm loopback/vif loopback.
 * @param	mode		loopback mode.
 *
 * @retval	none.
 */
void vcd_ctrl_start_call(int call_type, int mode)
{
	vcd_pr_start_control_function();

	/* check sequence */
	g_vcd_ctrl_result =
		vcd_ctrl_func_check_sequence(VCD_CTRL_FUNC_START_CALL);
	if (VCD_ERR_NONE != g_vcd_ctrl_result) {
		vcd_pr_control_info("check sequence[%d].\n",
			g_vcd_ctrl_result);
		g_vcd_ctrl_result =
			vcd_ctrl_func_convert_result(g_vcd_ctrl_result);
		goto rtn;
	}

	/* execute spuv function */
	if (VCD_CTRL_CALL_CS == call_type)
		g_vcd_ctrl_result = vcd_spuv_start_call();
	else if (VCD_CTRL_CALL_1KHZ == call_type)
		g_vcd_ctrl_result = vcd_spuv_start_1khz_tone();
	else if (VCD_CTRL_CALL_PCM == call_type)
		g_vcd_ctrl_result = vcd_spuv_start_pcm_loopback(mode);
	else if (VCD_CTRL_CALL_VIF == call_type)
		g_vcd_ctrl_result = vcd_spuv_start_bbif_loopback(mode);

	if (VCD_ERR_NONE != g_vcd_ctrl_result) {
		vcd_pr_err("start call error[%d].\n", g_vcd_ctrl_result);
		goto rtn;
	}

	/* update active status */
	vcd_ctrl_func_set_active_feature(VCD_CTRL_FUNC_FEATURE_CALL);

rtn:
	vcd_pr_end_control_function("g_vcd_ctrl_result[%d].\n",
		g_vcd_ctrl_result);
	return;
}


/**
 * @brief	stop call function.
 *
 * @param	call_type	cs/1khz tone/pcm loopback/vif loopback.
 *
 * @retval	none.
 */
void vcd_ctrl_stop_call(int call_type)
{
	vcd_pr_start_control_function();

	/* check sequence */
	g_vcd_ctrl_result =
		vcd_ctrl_func_check_sequence(VCD_CTRL_FUNC_STOP_CALL);
	if (VCD_ERR_NONE != g_vcd_ctrl_result) {
		vcd_pr_control_info("check sequence[%d].\n",
			g_vcd_ctrl_result);
		g_vcd_ctrl_result =
			vcd_ctrl_func_convert_result(g_vcd_ctrl_result);
		goto rtn;
	}

	/* execute spuv function */
	if (VCD_CTRL_CALL_CS == call_type)
		g_vcd_ctrl_result = vcd_spuv_stop_call();
	else if (VCD_CTRL_CALL_1KHZ == call_type)
		g_vcd_ctrl_result = vcd_spuv_stop_1khz_tone();
	else if (VCD_CTRL_CALL_PCM == call_type)
		g_vcd_ctrl_result = vcd_spuv_stop_pcm_loopback();
	else if (VCD_CTRL_CALL_VIF == call_type)
		g_vcd_ctrl_result = vcd_spuv_stop_bbif_loopback();

	if (VCD_ERR_NONE != g_vcd_ctrl_result) {
		vcd_pr_err("stop call error[%d].\n", g_vcd_ctrl_result);
		goto rtn;
	}

	/* update active status */
	vcd_ctrl_func_unset_active_feature(VCD_CTRL_FUNC_FEATURE_CALL);

rtn:
	vcd_pr_end_control_function("g_vcd_ctrl_result[%d].\n",
		g_vcd_ctrl_result);
	return;
}


/**
 * @brief	start tty/ctm function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_ctrl_start_tty_ctm(void)
{
	vcd_pr_start_control_function();

#ifdef __VCD_TTY_ENABLE__
	/* check sequence */
	g_vcd_ctrl_result =
		vcd_ctrl_func_check_sequence(VCD_CTRL_FUNC_START_TTY_CTM);
	if (VCD_ERR_NONE != g_vcd_ctrl_result) {
		vcd_pr_control_info("check sequence[%d].\n",
			g_vcd_ctrl_result);
		g_vcd_ctrl_result =
			vcd_ctrl_func_convert_result(g_vcd_ctrl_result);
		goto rtn;
	}

	/* execute spuv function */
	g_vcd_ctrl_result = vcd_spuv_start_tty_ctm();
	if (VCD_ERR_NONE != g_vcd_ctrl_result) {
		vcd_pr_err("tty config error[%d].\n", g_vcd_ctrl_result);
		goto rtn;
	}

	/* update active status */
	vcd_ctrl_func_set_active_feature(VCD_CTRL_FUNC_FEATURE_TTY_CTM);

#else /* __VCD_TTY_ENABLE__ */
	g_vcd_ctrl_result = VCD_ERR_NOT_SUPPORT;
	goto rtn;
#endif /* __VCD_TTY_ENABLE__ */
rtn:
	vcd_pr_end_control_function("g_vcd_ctrl_result[%d].\n",
		g_vcd_ctrl_result);
	return;
}


/**
 * @brief	stop tty/ctm function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_ctrl_stop_tty_ctm(void)
{
	vcd_pr_start_control_function();

#ifdef __VCD_TTY_ENABLE__
	/* check sequence */
	g_vcd_ctrl_result =
		vcd_ctrl_func_check_sequence(VCD_CTRL_FUNC_STOP_TTY_CTM);
	if (VCD_ERR_NONE != g_vcd_ctrl_result) {
		vcd_pr_control_info("check sequence[%d].\n",
			g_vcd_ctrl_result);
		g_vcd_ctrl_result =
			vcd_ctrl_func_convert_result(g_vcd_ctrl_result);
		goto rtn;
	}

	/* execute spuv function */
	g_vcd_ctrl_result = vcd_spuv_stop_tty_ctm();
	if (VCD_ERR_NONE != g_vcd_ctrl_result) {
		vcd_pr_err("tty config error[%d].\n", g_vcd_ctrl_result);
		goto rtn;
	}

	/* update active status */
	vcd_ctrl_func_unset_active_feature(VCD_CTRL_FUNC_FEATURE_TTY_CTM);

#else /* __VCD_TTY_ENABLE__ */
	g_vcd_ctrl_result = VCD_ERR_NOT_SUPPORT;
	goto rtn;
#endif /* __VCD_TTY_ENABLE__ */

rtn:
	vcd_pr_end_control_function("g_vcd_ctrl_result[%d].\n",
		g_vcd_ctrl_result);
	return;
}


/**
 * @brief	config tty/ctm function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_ctrl_config_tty_ctm(void)
{
	vcd_pr_start_control_function();

#ifdef __VCD_TTY_ENABLE__
	/* check sequence */
	g_vcd_ctrl_result =
		vcd_ctrl_func_check_sequence(VCD_CTRL_FUNC_CONFIG_TTY_CTM);
	if (VCD_ERR_NONE != g_vcd_ctrl_result) {
		vcd_pr_control_info("check sequence[%d].\n",
			g_vcd_ctrl_result);
		g_vcd_ctrl_result =
			vcd_ctrl_func_convert_result(g_vcd_ctrl_result);
		goto rtn;
	}

	/* execute spuv function */
	g_vcd_ctrl_result = vcd_spuv_config_tty_ctm();
	if (VCD_ERR_NONE != g_vcd_ctrl_result) {
		vcd_pr_err("tty config error[%d].\n", g_vcd_ctrl_result);
		goto rtn;
	}
#else /* __VCD_TTY_ENABLE__ */
	g_vcd_ctrl_result = VCD_ERR_NOT_SUPPORT;
	goto rtn;
#endif /* __VCD_TTY_ENABLE__ */

rtn:
	vcd_pr_end_control_function("g_vcd_ctrl_result[%d].\n",
		g_vcd_ctrl_result);
	return;
}


/**
 * @brief	set udata function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_ctrl_set_udata(void)
{
	vcd_pr_start_control_function();

	/* check sequence */
	g_vcd_ctrl_result =
		vcd_ctrl_func_check_sequence(VCD_CTRL_FUNC_SET_UDATA);
	if (VCD_ERR_NONE != g_vcd_ctrl_result) {
		vcd_pr_control_info("check sequence[%d].\n",
			g_vcd_ctrl_result);
		g_vcd_ctrl_result =
			vcd_ctrl_func_convert_result(g_vcd_ctrl_result);
		goto rtn;
	}

	/* execute spuv function */
	g_vcd_ctrl_result = vcd_spuv_set_udata();
	if (VCD_ERR_NONE != g_vcd_ctrl_result) {
		vcd_pr_err("set udata error[%d].\n", g_vcd_ctrl_result);
		goto rtn;
	}

rtn:
	vcd_pr_end_control_function("g_vcd_ctrl_result[%d].\n",
		g_vcd_ctrl_result);
	return;
}


/**
 * @brief	get status function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_ctrl_get_status(void)
{
	unsigned int active_feature = VCD_CTRL_FUNC_FEATURE_NONE;

	vcd_pr_start_control_function();

	/* operation state confirmation */
	active_feature = vcd_ctrl_func_get_active_feature();

	if (VCD_CTRL_FUNC_FEATURE_ERROR & active_feature) {
		g_vcd_ctrl_result = VCD_CTRL_STATUS_ERROR;
	} else if (VCD_CTRL_FUNC_FEATURE_CALL & active_feature) {
		if ((VCD_CTRL_FUNC_FEATURE_RECORD & active_feature) &&
			(VCD_CTRL_FUNC_FEATURE_PLAYBACK & active_feature)) {
			g_vcd_ctrl_result =
				VCD_CTRL_STATUS_CALL_RECORD_PLAYBACK;
		} else if (VCD_CTRL_FUNC_FEATURE_RECORD & active_feature) {
			g_vcd_ctrl_result = VCD_CTRL_STATUS_CALL_RECORD;
		} else if (VCD_CTRL_FUNC_FEATURE_PLAYBACK & active_feature) {
			g_vcd_ctrl_result = VCD_CTRL_STATUS_CALL_PLAYBACK;
		} else {
			g_vcd_ctrl_result = VCD_CTRL_STATUS_CALL;
		}
	} else if (VCD_CTRL_FUNC_FEATURE_RECORD & active_feature) {
		if (VCD_CTRL_FUNC_FEATURE_PLAYBACK & active_feature)
			g_vcd_ctrl_result = VCD_CTRL_STATUS_RECORD_PLAYBACK;
		else
			g_vcd_ctrl_result = VCD_CTRL_STATUS_RECORD;
	} else if (VCD_CTRL_FUNC_FEATURE_PLAYBACK & active_feature) {
		g_vcd_ctrl_result = VCD_CTRL_STATUS_PLAYBACK;
	} else if (VCD_CTRL_FUNC_FEATURE_HW_PARAM & active_feature) {
		g_vcd_ctrl_result = VCD_CTRL_STATUS_READY;
	} else if (VCD_CTRL_FUNC_FEATURE_VCD & active_feature) {
		g_vcd_ctrl_result = VCD_CTRL_STATUS_ACTIVE;
	} else {
		g_vcd_ctrl_result = VCD_CTRL_STATUS_STANDBY;
	}

	vcd_pr_end_control_function("g_vcd_ctrl_result[%d].\n",
		g_vcd_ctrl_result);
	return;
}


/**
 * @brief	get result function.
 *
 * @param	none.
 *
 * @retval	result	processing result.
 */
int vcd_ctrl_get_result(void)
{
	vcd_pr_start_control_function();

	vcd_pr_end_control_function("g_vcd_ctrl_result[%d].\n",
		g_vcd_ctrl_result);
	return g_vcd_ctrl_result;
}


/* ========================================================================= */
/* For Sound driver functions                                                */
/* ========================================================================= */

/**
 * @brief	start record function.
 *
 * @param[in]	option	pointer of record option structure.
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	others		result of called function.
 */
int vcd_ctrl_start_record(struct vcd_record_option *option)
{
	int ret = VCD_ERR_NONE;

	vcd_pr_start_control_function("option[%p].\n", option);
	vcd_pr_control_info("option.mode[%d].\n", option->mode);

	/* check sequence */
	ret = vcd_ctrl_func_check_sequence(VCD_CTRL_FUNC_START_RECORD);
	if (VCD_ERR_NONE != ret) {
		vcd_pr_control_info("check sequence[%d].\n", ret);
		ret = vcd_ctrl_func_convert_result(ret);
		goto rtn;
	}

	/* execute spuv function */
	ret = vcd_spuv_start_record(option);
	if (VCD_ERR_NONE != ret) {
		vcd_pr_err("start record error[%d].\n", ret);
		goto rtn;
	}

	/* update active status */
	vcd_ctrl_func_set_active_feature(VCD_CTRL_FUNC_FEATURE_RECORD);

rtn:
	vcd_pr_end_control_function("ret[%d].\n", ret);
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
int vcd_ctrl_stop_record(void)
{
	int ret = VCD_ERR_NONE;

	vcd_pr_start_control_function();

	/* check sequence */
	ret = vcd_ctrl_func_check_sequence(VCD_CTRL_FUNC_STOP_RECORD);
	if (VCD_ERR_NONE != ret) {
		vcd_pr_control_info("check sequence[%d].\n", ret);
		ret = vcd_ctrl_func_convert_result(ret);
		goto rtn;
	}

	/* execute spuv function */
	ret = vcd_spuv_stop_record();
	if (VCD_ERR_NONE != ret) {
		vcd_pr_err("stop record error[%d].\n", ret);
		/* update result */
		g_vcd_ctrl_result = VCD_ERR_NONE;
	}

	/* update active status */
	vcd_ctrl_func_unset_active_feature(VCD_CTRL_FUNC_FEATURE_RECORD);

rtn:
	vcd_pr_end_control_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	start playback function.
 *
 * @param[in]	option	pointer of playback option structure.
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	others		result of called function.
 */
int vcd_ctrl_start_playback(struct vcd_playback_option *option)
{
	int ret = VCD_ERR_NONE;

	vcd_pr_start_control_function("option[%p].\n", option);
	vcd_pr_control_info("option.mode[%d].\n", option->mode);

	/* check sequence */
	ret = vcd_ctrl_func_check_sequence(VCD_CTRL_FUNC_START_PLAYBACK);
	if (VCD_ERR_NONE != ret) {
		vcd_pr_control_info("check sequence[%d].\n", ret);
		ret = vcd_ctrl_func_convert_result(ret);
		goto rtn;
	}

	/* execute spuv function */
	ret = vcd_spuv_start_playback(option);
	if (VCD_ERR_NONE != ret) {
		vcd_pr_err("start playback error[%d].\n", ret);
		goto rtn;
	}

	/* update active status */
	vcd_ctrl_func_set_active_feature(VCD_CTRL_FUNC_FEATURE_PLAYBACK);

rtn:
	vcd_pr_end_control_function("ret[%d].\n", ret);
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
int vcd_ctrl_stop_playback(void)
{
	int ret = VCD_ERR_NONE;

	vcd_pr_start_control_function();

	/* check sequence */
	ret = vcd_ctrl_func_check_sequence(VCD_CTRL_FUNC_STOP_PLAYBACK);
	if (VCD_ERR_NONE != ret) {
		vcd_pr_control_info("check sequence[%d].\n", ret);
		ret = vcd_ctrl_func_convert_result(ret);
		goto rtn;
	}

	/* execute spuv function */
	ret = vcd_spuv_stop_playback();
	if (VCD_ERR_NONE != ret) {
		vcd_pr_err("stop playback error[%d].\n", ret);
		/* update result */
		g_vcd_ctrl_result = VCD_ERR_NONE;
	}

	/* update active status */
	vcd_ctrl_func_unset_active_feature(VCD_CTRL_FUNC_FEATURE_PLAYBACK);

rtn:
	vcd_pr_end_control_function("ret[%d].\n", ret);
	return ret;
}

/**
 * @brief	get record buffer function.
 *
 * @param[out]	info	pointer of record buffer info structure.
 *
 * @retval	none.
 */
void vcd_ctrl_get_record_buffer(struct vcd_record_buffer_info *info)
{
	vcd_pr_start_control_function();

	/* execute spuv function */
	vcd_spuv_get_record_buffer(info);

	vcd_pr_end_control_function();
	return;
}


/**
 * @brief	get playback buffer function.
 *
 * @param[out]	info	pointer of playback buffer info structure.
 *
 * @retval	none.
 */
void vcd_ctrl_get_playback_buffer(struct vcd_playback_buffer_info *info)
{
	vcd_pr_start_control_function();

	/* execute spuv function */
	vcd_spuv_get_playback_buffer(info);

	vcd_pr_end_control_function();
	return;
}


/* ========================================================================= */
/* For spuv functions                                                        */
/* ========================================================================= */

/**
 * @brief	rec_trigger notification function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_ctrl_rec_trigger(void)
{
	vcd_pr_start_control_function();

	/* notification buffer update */
	vcd_complete_buffer();

	vcd_pr_end_control_function();
	return;
}


/**
 * @brief	play_trigger notification function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_ctrl_play_trigger(void)
{
	vcd_pr_start_control_function();

	/* notification buffer update */
	vcd_beginning_buffer();

	vcd_pr_end_control_function();
	return;
}


/**
 * @brief	fw stop notification function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_ctrl_stop_fw(void)
{
	vcd_pr_start_control_function();

	/* update feature */
	vcd_ctrl_func_set_active_feature(VCD_CTRL_FUNC_FEATURE_ERROR);

	/* notification fw stop */
	vcd_stop_fw();

	vcd_pr_end_control_function();
	return;
}


/**
 * @brief	start clkgen timing notification function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_ctrl_start_clkgen(void)
{
	vcd_pr_start_control_function();

	/* notification start clkgen timing */
	vcd_start_clkgen();

	vcd_pr_end_control_function();
	return;
}


/* ========================================================================= */
/* Driver functions                                                          */
/* ========================================================================= */

/**
 * @brief	driver suspend function.
 *
 * @param	none.
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	VCD_ERR_BUSY	vcd is operating.
 */
int vcd_ctrl_suspend(void)
{
	int ret = VCD_ERR_NONE;

	vcd_pr_start_control_function();

	/* nop */

	vcd_pr_end_control_function("ret[%d].\n", ret);
	return ret;
}

/**
 * @brief	driver resume function.
 *
 * @param	none.
 *
 * @retval	VCD_ERR_NONE	successful.
 */
int vcd_ctrl_resume(void)
{
	int ret = VCD_ERR_NONE;

	vcd_pr_start_control_function();

	/* nop */

	vcd_pr_end_control_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	driver runtime suspend function.
 *
 * @param	none.
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	VCD_ERR_BUSY	vcd is operating.
 */
int vcd_ctrl_runtime_suspend(void)
{
	int ret = VCD_ERR_NONE;
	unsigned int active_feature = VCD_CTRL_FUNC_FEATURE_NONE;

	vcd_pr_start_control_function();

	/* operation state confirmation */
	active_feature = vcd_ctrl_func_get_active_feature();
	if (VCD_CTRL_FUNC_FEATURE_NONE != active_feature) {
		vcd_pr_control_info("vcd is operating.\n");
		ret = VCD_ERR_BUSY;
		goto rtn;
	}

rtn:
	vcd_pr_end_control_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	driver runtime resume function.
 *
 * @param	none.
 *
 * @retval	VCD_ERR_NONE	successful.
 */
int vcd_ctrl_runtime_resume(void)
{
	int ret = VCD_ERR_NONE;

	vcd_pr_start_control_function();

	/* nop */

	vcd_pr_end_control_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	driver probe function.
 *
 * @param	none.
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	others		result of called function.
 */
int vcd_ctrl_probe(void)
{
	int ret = VCD_ERR_NONE;

	vcd_pr_start_control_function();

	/* execute spuv function */
	ret = vcd_spuv_ioremap();
	if (VCD_ERR_NONE != ret)
		goto rtn;

	/* execute spuv function */
	ret = vcd_spuv_get_fw_buffer();
	if (VCD_ERR_NONE != ret)
		goto rtn;

	/* execute spuv function */
	ret = vcd_spuv_create_queue();
	if (VCD_ERR_NONE != ret)
		goto rtn;

	/* initialize */
	vcd_ctrl_func_initialize();
	g_vcd_ctrl_result = VCD_ERR_NONE;

rtn:
	vcd_pr_end_control_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	driver remove function.
 *
 * @param	none.
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	others		result of called function.
 */
int vcd_ctrl_remove(void)
{
	int ret = VCD_ERR_NONE;

	vcd_pr_start_control_function();

	/* execute spuv function */
	vcd_spuv_destroy_queue();

	/* execute spuv function */
	vcd_spuv_free_fw_buffer();

	/* execute spuv function */
	vcd_spuv_iounmap();

	vcd_pr_end_control_function("ret[%d].\n", ret);
	return ret;
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
void vcd_ctrl_dump_status(void)
{
	unsigned int active_feature = VCD_CTRL_FUNC_FEATURE_NONE;

	vcd_pr_start_control_function();

	/* operation state confirmation */
	active_feature = vcd_ctrl_func_get_active_feature();

	/* control status dump */
	vcd_pr_registers_dump("g_vcd_ctrl_active_feature      [%08x].\n",
		active_feature);

	/* execute spuv function */
	vcd_spuv_dump_status();

	vcd_pr_end_control_function();
	return;
}


/**
 * @brief	dump registers function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_ctrl_dump_registers(void)
{
	vcd_pr_start_control_function();

	/* execute spuv function */
	vcd_spuv_dump_registers();

	vcd_pr_end_control_function();
	return;
}


/**
 * @brief	dump cpg registers function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_ctrl_dump_cpg_registers(void)
{
	vcd_pr_start_control_function();

	/* execute spuv function */
	vcd_spuv_dump_cpg_registers();

	vcd_pr_end_control_function();
	return;
}


/**
 * @brief	dump crmu registers function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_ctrl_dump_crmu_registers(void)
{
	vcd_pr_start_control_function();

	/* execute spuv function */
	vcd_spuv_dump_crmu_registers();

	vcd_pr_end_control_function();
	return;
}


/**
 * @brief	dump gtu registers function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_ctrl_dump_gtu_registers(void)
{
	vcd_pr_start_control_function();

	/* execute spuv function */
	vcd_spuv_dump_gtu_registers();

	vcd_pr_end_control_function();
	return;
}


/**
 * @brief	dump voiceif registers function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_ctrl_dump_voiceif_registers(void)
{
	vcd_pr_start_control_function();

	/* execute spuv function */
	vcd_spuv_dump_voiceif_registers();

	vcd_pr_end_control_function();
	return;
}


/**
 * @brief	dump intcvo registers function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_ctrl_dump_intcvo_registers(void)
{
	vcd_pr_start_control_function();

	/* execute spuv function */
	vcd_spuv_dump_intcvo_registers();

	vcd_pr_end_control_function();
	return;
}


/**
 * @brief	dump spuv registers function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_ctrl_dump_spuv_registers(void)
{
	vcd_pr_start_control_function();

	/* execute spuv function */
	vcd_spuv_dump_spuv_registers();

	vcd_pr_end_control_function();
	return;
}


/**
 * @brief	dump dsp0 registers function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_ctrl_dump_dsp0_registers(void)
{
	vcd_pr_start_control_function();

	/* execute spuv function */
	vcd_spuv_dump_dsp0_registers();

	vcd_pr_end_control_function();
	return;
}
