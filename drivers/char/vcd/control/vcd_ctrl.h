/* vcd_ctrl.h
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
#ifndef __VCD_CTRL_H__
#define __VCD_CTRL_H__

/*
 * constant definition
 */
/* call type */
#define VCD_CTRL_CALL_CS	0
#define VCD_CTRL_CALL_1KHZ	1
#define VCD_CTRL_CALL_PCM	2
#define VCD_CTRL_CALL_VIF	3

/* vcd status */
#define VCD_CTRL_STATUS_STANDBY			\
			VCD_CTRL_STATUS_NOT_START
#define VCD_CTRL_STATUS_ACTIVE			\
			VCD_CTRL_STATUS_STARTED_DSP
#define VCD_CTRL_STATUS_READY			\
			VCD_CTRL_STATUS_STARTED_DSP
#define VCD_CTRL_STATUS_RECORD			\
			VCD_CTRL_STATUS_STARTED_RECORD_PLAYBACK
#define VCD_CTRL_STATUS_PLAYBACK		\
			VCD_CTRL_STATUS_STARTED_RECORD_PLAYBACK
#define VCD_CTRL_STATUS_RECORD_PLAYBACK		\
			VCD_CTRL_STATUS_STARTED_RECORD_PLAYBACK
#define VCD_CTRL_STATUS_CALL			\
			VCD_CTRL_STATUS_STARTED_CALL
#define VCD_CTRL_STATUS_CALL_RECORD		\
			VCD_CTRL_STATUS_STARTED_CALL_RECORD_PLAYBACK
#define VCD_CTRL_STATUS_CALL_PLAYBACK		\
			VCD_CTRL_STATUS_STARTED_CALL_RECORD_PLAYBACK
#define VCD_CTRL_STATUS_CALL_RECORD_PLAYBACK	\
			VCD_CTRL_STATUS_STARTED_CALL_RECORD_PLAYBACK
#define VCD_CTRL_STATUS_TTY_CTM			\
			VCD_CTRL_STATUS_STARTED_TTY_CTM
#define VCD_CTRL_STATUS_ERROR			\
			VCD_CTRL_STATUS_ERROR_OCCURS


/*
 * define macro declaration
 */


/*
 * enum declaration
 */
enum VCD_CTRL_STATUS {
	VCD_CTRL_STATUS_ERROR_OCCURS = -1,
	VCD_CTRL_STATUS_NOT_START = 0,
	VCD_CTRL_STATUS_STARTED_DSP,
	VCD_CTRL_STATUS_STARTED_RECORD_PLAYBACK,
	VCD_CTRL_STATUS_STARTED_CALL,
	VCD_CTRL_STATUS_STARTED_CALL_RECORD_PLAYBACK,
	VCD_CTRL_STATUS_STARTED_TTY_CTM,
};


/*
 * structure declaration
 */


/*
 * table declaration
 */


/*
 * prototype declaration
 */

/* For AMHAL functions */
void vcd_ctrl_get_msg_buffer(void);
void vcd_ctrl_start_vcd(void);
void vcd_ctrl_stop_vcd(void);
void vcd_ctrl_set_hw_param(void);
void vcd_ctrl_start_call(int call_type, int mode);
void vcd_ctrl_stop_call(int call_type);
void vcd_ctrl_start_tty_ctm(void);
void vcd_ctrl_stop_tty_ctm(void);
void vcd_ctrl_config_tty_ctm(void);
void vcd_ctrl_set_udata(void);
void vcd_ctrl_get_status(void);
int vcd_ctrl_get_result(void);

/* For Sound driver functions */
int vcd_ctrl_start_record(struct vcd_record_option *option);
int vcd_ctrl_stop_record(void);
int vcd_ctrl_start_playback(struct vcd_playback_option *option);
int vcd_ctrl_stop_playback(void);
void vcd_ctrl_get_record_buffer(struct vcd_record_buffer_info *info);
void vcd_ctrl_get_playback_buffer(struct vcd_playback_buffer_info *info);

/* For spuv functions */
void vcd_ctrl_rec_trigger(void);
void vcd_ctrl_play_trigger(void);
void vcd_ctrl_stop_fw(void);
void vcd_ctrl_start_clkgen(void);

/* Driver functions */
int vcd_ctrl_suspend(void);
int vcd_ctrl_resume(void);
int vcd_ctrl_runtime_suspend(void);
int vcd_ctrl_runtime_resume(void);
int vcd_ctrl_probe(void);
int vcd_ctrl_remove(void);

/* Dump functions */
void vcd_ctrl_dump_status(void);
void vcd_ctrl_dump_registers(void);
void vcd_ctrl_dump_cpg_registers(void);
void vcd_ctrl_dump_crmu_registers(void);
void vcd_ctrl_dump_gtu_registers(void);
void vcd_ctrl_dump_voiceif_registers(void);
void vcd_ctrl_dump_intcvo_registers(void);
void vcd_ctrl_dump_spuv_registers(void);
void vcd_ctrl_dump_dsp0_registers(void);

#endif /* __VCD_CTRL_H__ */
