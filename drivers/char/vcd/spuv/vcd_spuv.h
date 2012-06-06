/* vcd_spuv.h
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
#ifndef __VCD_SPUV_H__
#define __VCD_SPUV_H__

/*
 * constant definition
 */
/* spuv status */
#define VCD_SPUV_STATUS_NONE			0x00000000
#define VCD_SPUV_STATUS_WAIT_ACK		0x00000001
#define VCD_SPUV_STATUS_WAIT_REQ		0x00000002
#define VCD_SPUV_STATUS_NEED_ACK		0x00000004
#define VCD_SPUV_STATUS_SYSTEM_ERROR		0x00000008

/* spuv reply kind */
#define VCD_SPUV_REPLY_REQ			0x00000001
#define VCD_SPUV_REPLY_ACK			0x00000002

/* spuv AINTCLR value */
#define VCD_SPUV_AINTCLR_REQ			0x00000001
#define VCD_SPUV_AINTCLR_ACK			0x00000002

/* spuv IEVENTC value */
#define VCD_SPUV_IEVENTC			0x00001111

/* spuv fw interface id */
#define VCD_SPUV_INTERFACE_ID			0x5C
#define VCD_SPUV_INTERFACE_ID_ACK_ONLY		0

/* spuv fw record/playback gain */
#define VCD_SPUV_GAIN_DISABLE			0
#define VCD_SPUV_GAIN_ENABLE			10

/* spuv fw message id (to spuv) */
#define VCD_SPUV_HW_PARAMETERS_IND		0x04
#define VCD_SPUV_ACTIVE_REQ			0x10
#define VCD_SPUV_SPEECH_START_REQ		0x11
#define VCD_SPUV_SPEECH_STOP_REQ		0x12
#define VCD_SPUV_VOICE_RECORDING_START_REQ	0x13
#define VCD_SPUV_VOICE_RECORDING_STOP_REQ	0x14
#define VCD_SPUV_VOICE_PLAYING_START_REQ	0x15
#define VCD_SPUV_VOICE_PLAYING_STOP_REQ		0x16
#define VCD_SPUV_1KHZ_TONE_START_REQ		0x17
#define VCD_SPUV_1KHZ_TONE_STOP_REQ		0x18
#define VCD_SPUV_PCM_LOOPBACK_START_REQ		0x19
#define VCD_SPUV_PCM_LOOPBACK_STOP_REQ		0x1A
#define VCD_SPUV_BBIF_LOOPBACK_START_REQ	0x1B
#define VCD_SPUV_BBIF_LOOPBACK_STOP_REQ		0x1C
#define VCD_SPUV_TRACE_SELECT_REQ		0x1D
#define VCD_SPUV_UDATA_REQ			0x1E
#define VCD_SPUV_TTY_CTM_START_REQ		0x40
#define VCD_SPUV_TTY_CTM_STOP_REQ		0x41
#define VCD_SPUV_TTY_CTM_CONFIG_REQ		0x42

/* spuv fw message length (to spuv) */
#define VCD_SPUV_HW_PARAMETERS_LENGTH		5
#define VCD_SPUV_ACTIVE_LENGTH			2
#define VCD_SPUV_SPEECH_START_LENGTH		3
#define VCD_SPUV_SPEECH_STOP_LENGTH		2
#define VCD_SPUV_VOICE_RECORDING_START_LENGTH	7
#define VCD_SPUV_VOICE_RECORDING_STOP_LENGTH	2
#define VCD_SPUV_VOICE_PLAYING_START_LENGTH	9
#define VCD_SPUV_VOICE_PLAYING_STOP_LENGTH	2
#define VCD_SPUV_1KHZ_TONE_START_LENGTH		2
#define VCD_SPUV_1KHZ_TONE_STOP_LENGTH		2
#define VCD_SPUV_PCM_LOOPBACK_START_LENGTH	3
#define VCD_SPUV_PCM_LOOPBACK_STOP_LENGTH	2
#define VCD_SPUV_BBIF_LOOPBACK_START_LENGTH	3
#define VCD_SPUV_BBIF_LOOPBACK_STOP_LENGTH	2
#define VCD_SPUV_TRACE_SELECT_LENGTH		3
#define VCD_SPUV_TTY_CTM_START_LENGTH		6
#define VCD_SPUV_TTY_CTM_STOP_LENGTH		2
#define VCD_SPUV_TTY_CTM_CONFIG_LENGTH		4
#define VCD_SPUV_UDATA_LENGTH			3

/* spuv fw message id (from spuv) */
#define VCD_SPUV_BOOT_COMPLETE_IND		0x00
#define VCD_SPUV_SYSTEM_ERROR_IND		0x01
#define VCD_SPUV_SYSTEM_INFO_IND		0x02
#define VCD_SPUV_TRIGGER_PLAY_IND		0x03
#define VCD_SPUV_TRIGGER_REC_IND		0x05
#define VCD_SPUV_ACTIVE_CNF			0x20
#define VCD_SPUV_SPEECH_START_CNF		0x21
#define VCD_SPUV_SPEECH_STOP_CNF		0x22
#define VCD_SPUV_VOICE_RECORDING_START_CNF	0x23
#define VCD_SPUV_VOICE_RECORDING_STOP_CNF	0x24
#define VCD_SPUV_VOICE_PLAYING_START_CNF	0x25
#define VCD_SPUV_VOICE_PLAYING_STOP_CNF		0x26
#define VCD_SPUV_1KHZ_TONE_START_CNF		0x27
#define VCD_SPUV_1KHZ_TONE_STOP_CNF		0x28
#define VCD_SPUV_PCM_LOOPBACK_START_CNF		0x29
#define VCD_SPUV_PCM_LOOPBACK_STOP_CNF		0x2A
#define VCD_SPUV_BBIF_LOOPBACK_START_CNF	0x2B
#define VCD_SPUV_BBIF_LOOPBACK_STOP_CNF		0x2C
#define VCD_SPUV_TRACE_SELECT_CNF		0x2D
#define VCD_SPUV_UDATA_IND			0x2E
#define VCD_SPUV_TTY_CTM_START_CNF		0x50
#define VCD_SPUV_TTY_CTM_STOP_CNF		0x51
#define VCD_SPUV_TTY_CTM_CONFIG_CNF		0x52

/* spuv fw message log */
#define VCD_SPUV_HW_PARAMETERS_IND_LOG		\
		"V --> F : SPUV_HW_PARAMETERS_IND.\n"
#define VCD_SPUV_ACTIVE_REQ_LOG			\
		"V --> F : SPUV_ACTIVE_REQ.\n"
#define VCD_SPUV_SPEECH_START_REQ_LOG		\
		"V --> F : SPUV_SPEECH_START_REQ.\n"
#define VCD_SPUV_SPEECH_STOP_REQ_LOG		\
		"V --> F : SPUV_SPEECH_STOP_REQ.\n"
#define VCD_SPUV_VOICE_RECORDING_START_REQ_LOG	\
		"V --> F : SPUV_VOICE_RECORDING_START_REQ.\n"
#define VCD_SPUV_VOICE_RECORDING_STOP_REQ_LOG	\
		"V --> F : SPUV_VOICE_RECORDING_STOP_REQ.\n"
#define VCD_SPUV_VOICE_PLAYING_START_REQ_LOG	\
		"V --> F : SPUV_VOICE_PLAYING_START_REQ.\n"
#define VCD_SPUV_VOICE_PLAYING_STOP_REQ_LOG	\
		"V --> F : SPUV_VOICE_PLAYING_STOP_REQ.\n"
#define VCD_SPUV_1KHZ_TONE_START_REQ_LOG	\
		"V --> F : SPUV_1KHZ_TONE_START_REQ.\n"
#define VCD_SPUV_1KHZ_TONE_STOP_REQ_LOG		\
		"V --> F : SPUV_1KHZ_TONE_STOP_REQ.\n"
#define VCD_SPUV_PCM_LOOPBACK_START_REQ_LOG	\
		"V --> F : SPUV_PCM_LOOPBACK_START_REQ.\n"
#define VCD_SPUV_PCM_LOOPBACK_STOP_REQ_LOG	\
		"V --> F : SPUV_PCM_LOOPBACK_STOP_REQ.\n"
#define VCD_SPUV_BBIF_LOOPBACK_START_REQ_LOG	\
		"V --> F : SPUV_BBIF_LOOPBACK_START_REQ.\n"
#define VCD_SPUV_BBIF_LOOPBACK_STOP_REQ_LOG	\
		"V --> F : SPUV_BBIF_LOOPBACK_STOP_REQ.\n"
#define VCD_SPUV_TRACE_SELECT_REQ_LOG		\
		"V --> F : SPUV_TRACE_SELECT_REQ.\n"
#define VCD_SPUV_UDATA_REQ_LOG			\
		"V --> F : SPUV_UDATA_REQ.\n"
#define VCD_SPUV_TTY_CTM_START_REQ_LOG		\
		"V --> F : SPUV_TTY_CTM_START_REQ.\n"
#define VCD_SPUV_TTY_CTM_STOP_REQ_LOG		\
		"V --> F : SPUV_TTY_CTM_STOP_REQ.\n"
#define VCD_SPUV_TTY_CTM_CONFIG_REQ_LOG		\
		"V --> F : SPUV_TTY_CTM_CONFIG_REQ.\n"
#define VCD_SPUV_BOOT_COMPLETE_IND_LOG		\
		"V <-- F : SPUV_BOOT_COMPLETE_IND.\n"
#define VCD_SPUV_SYSTEM_ERROR_IND_LOG		\
		"V <-- F : SPUV_SYSTEM_ERROR_IND.\n"
#define VCD_SPUV_SYSTEM_INFO_IND_LOG		\
		"V <-- F : SPUV_SYSTEM_INFO_IND.\n"
#define VCD_SPUV_TRIGGER_PLAY_IND_LOG		\
		"V <-- F : SPUV_TRIGGER_PLAY_IND.\n"
#define VCD_SPUV_TRIGGER_REC_IND_LOG		\
		"V <-- F : SPUV_TRIGGER_REC_IND.\n"
#define VCD_SPUV_ACTIVE_CNF_LOG			\
		"V <-- F : SPUV_ACTIVE_CNF.\n"
#define VCD_SPUV_SPEECH_START_CNF_LOG		\
		"V <-- F : SPUV_SPEECH_START_CNF.\n"
#define VCD_SPUV_SPEECH_STOP_CNF_LOG		\
		"V <-- F : SPUV_SPEECH_STOP_CNF.\n"
#define VCD_SPUV_VOICE_RECORDING_START_CNF_LOG	\
		"V <-- F : SPUV_VOICE_RECORDING_START_CNF.\n"
#define VCD_SPUV_VOICE_RECORDING_STOP_CNF_LOG	\
		"V <-- F : SPUV_VOICE_RECORDING_STOP_CNF.\n"
#define VCD_SPUV_VOICE_PLAYING_START_CNF_LOG	\
		"V <-- F : SPUV_VOICE_PLAYING_START_CNF.\n"
#define VCD_SPUV_VOICE_PLAYING_STOP_CNF_LOG	\
		"V <-- F : SPUV_VOICE_PLAYING_STOP_CNF.\n"
#define VCD_SPUV_1KHZ_TONE_START_CNF_LOG	\
		"V <-- F : SPUV_1KHZ_TONE_START_CNF.\n"
#define VCD_SPUV_1KHZ_TONE_STOP_CNF_LOG		\
		"V <-- F : SPUV_1KHZ_TONE_STOP_CNF.\n"
#define VCD_SPUV_PCM_LOOPBACK_START_CNF_LOG	\
		"V <-- F : SPUV_PCM_LOOPBACK_START_CNF.\n"
#define VCD_SPUV_PCM_LOOPBACK_STOP_CNF_LOG	\
		"V <-- F : SPUV_PCM_LOOPBACK_STOP_CNF.\n"
#define VCD_SPUV_BBIF_LOOPBACK_START_CNF_LOG	\
		"V <-- F : SPUV_BBIF_LOOPBACK_START_CNF.\n"
#define VCD_SPUV_BBIF_LOOPBACK_STOP_CNF_LOG	\
		"V <-- F : SPUV_BBIF_LOOPBACK_STOP_CNF.\n"
#define VCD_SPUV_TRACE_SELECT_CNF_LOG		\
		"V <-- F : SPUV_TRACE_SELECT_CNF.\n"
#define VCD_SPUV_UDATA_IND_LOG			\
		"V <-- F : SPUV_UDATA_IND.\n"
#define VCD_SPUV_TTY_CTM_START_CNF_LOG		\
		"V <-- F : SPUV_TTY_CTM_START_CNF.\n"
#define VCD_SPUV_TTY_CTM_STOP_CNF_LOG		\
		"V <-- F : SPUV_TTY_CTM_STOP_CNF.\n"
#define VCD_SPUV_TTY_CTM_CONFIG_CNF_LOG		\
		"V <-- F : SPUV_TTY_CTM_CONFIG_CNF.\n"

/* spuv fw result */
#define VCD_SPUV_FW_RESULT_SUCCESS		0
#define VCD_SPUV_FW_RESULT_ERROR		1

/*
 * define macro declaration
 */
#define VCD_SPUV_SPI_NO				gic_spi(71)


/*
 * enum declaration
 */
enum VCD_SPUV_VALIDITY {
	VCD_SPUV_DISABLE = 0,
	VCD_SPUV_ENABLE
};


/*
 * structure declaration
 */
struct vcd_spuv_workqueue {
	struct work_struct work;
};

struct vcd_spuv_info {
	unsigned int status;
	unsigned int wait_fw_if_id;
	unsigned int wait_fw_msg_id;
	int fw_result;
};


/*
 * table declaration
 */


/*
 * prototype declaration
 */

/* Internal public functions */
int vcd_spuv_ioremap(void);
void vcd_spuv_iounmap(void);
int vcd_spuv_get_fw_buffer(void);
void vcd_spuv_free_fw_buffer(void);
int vcd_spuv_get_msg_buffer(void);
int vcd_spuv_start_vcd(void);
int vcd_spuv_stop_vcd(void);
int vcd_spuv_set_hw_param(void);
int vcd_spuv_start_call(void);
int vcd_spuv_stop_call(void);
int vcd_spuv_start_tty_ctm(void);
int vcd_spuv_stop_tty_ctm(void);
int vcd_spuv_config_tty_ctm(void);
int vcd_spuv_set_udata(void);
int vcd_spuv_start_record(struct vcd_record_option *option);
int vcd_spuv_stop_record(void);
int vcd_spuv_start_playback(struct vcd_playback_option *option);
int vcd_spuv_stop_playback(void);
void vcd_spuv_get_record_buffer(struct vcd_record_buffer_info *info);
void vcd_spuv_get_playback_buffer(struct vcd_playback_buffer_info *info);

int vcd_spuv_start_1khz_tone(void);
int vcd_spuv_stop_1khz_tone(void);
int vcd_spuv_start_pcm_loopback(int mode);
int vcd_spuv_stop_pcm_loopback(void);
int vcd_spuv_start_bbif_loopback(int mode);
int vcd_spuv_stop_bbif_loopback(void);
int vcd_spuv_set_trace_select(void);

/* Interrupt functions */
static int vcd_spuv_request_irq(void);
static void vcd_spuv_free_irq(void);
static int vcd_spuv_set_irq(int validity);
static irqreturn_t vcd_spuv_irq_handler(int irq, void *dev_id);

/* Queue functions */
int vcd_spuv_create_queue(void);
void vcd_spuv_destroy_queue(void);
static void vcd_spuv_set_schedule(void);
static void vcd_spuv_interrupt_ack(struct work_struct *work);
static void vcd_spuv_interrupt_req(struct work_struct *work);
static void vcd_spuv_rec_trigger(struct work_struct *work);
static void vcd_spuv_play_trigger(struct work_struct *work);
static void vcd_spuv_system_error(struct work_struct *work);
static void vcd_spuv_interface_log(unsigned int msg);

/* FW info functions */
static void vcd_spuv_set_wait_fw_info(unsigned int fw_id, unsigned int msg_id);
static void vcd_spuv_check_wait_fw_info(unsigned int fw_id,
			unsigned int msg_id, unsigned int result);

/* Status functions */
static unsigned int vcd_spuv_get_status(void);
static void vcd_spuv_set_status(unsigned int status);
static void vcd_spuv_unset_status(unsigned int status);
static int vcd_spuv_check_result(void);

/* Dump functions */
void vcd_spuv_dump_status(void);
void vcd_spuv_dump_registers(void);
void vcd_spuv_dump_cpg_registers(void);
void vcd_spuv_dump_crmu_registers(void);
void vcd_spuv_dump_gtu_registers(void);
void vcd_spuv_dump_voiceif_registers(void);
void vcd_spuv_dump_intcvo_registers(void);
void vcd_spuv_dump_spuv_registers(void);
void vcd_spuv_dump_dsp0_registers(void);

#endif /* __VCD_SPUV_H__ */
