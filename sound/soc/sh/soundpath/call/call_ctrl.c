/* call_ctrl.c
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


#define __CALL_CTRL_NO_EXTERN__

#include <linux/jiffies.h>
#include <asm/atomic.h>
#include <sound/soundpath/soundpath.h>
#include <sound/soundpath/call_extern.h>

#include "call_ctrl.h"

/*
 *
 * GLOBAL DATA Definitions
 *
 */

/* Playback buf address from Vocoder */
static u_int g_call_play_data_addr[DATA_SIDE_MAX] = {0, 0};
/* Record buf address from Vocoder */
static u_int g_call_rec_data_addr[DATA_SIDE_MAX] = {0, 0};
/* Stream information from linux kernel */
static struct snd_pcm_substream *g_call_substream[SNDP_PCM_DIRECTION_MAX];
/* Data information for Playback and Capture */
static struct call_pcm_info g_call_pcm_info[SNDP_PCM_DIRECTION_MAX];

/* Call status */
static enum call_status g_status;
/* Uplink status */
static bool g_call_play_uplink;
/* Dummy record flag */
static bool g_call_dummy_rec;
/* Callback Pointer */
static callback_func g_call_sndp_stop_fw;

/* Work queue */
struct workqueue_struct	*g_call_queue_in;

/* Temporary area information(Play) */
static u8 g_call_playback_buff[VCD_PLAYBACK_BUFFER_SIZE];
static long g_call_playback_len;

/* Temporary area information(Rec) */
static u8 g_call_record_buff[VCD_RECORD_BUFFER_SIZE];
static long g_call_record_len;
static long g_call_dummy_record_len;

#ifdef DEBUG
static bool g_call_cb_debug[SNDP_PCM_DIRECTION_MAX] = { true, true };
#endif


/*!
   @brief Speech + Playback Start function

   @param[in]	substream	PCM substream structure
   @param[out]	none

   @retval	0		Successful
   @retval	-EPERM		Not ready
 */
int call_playback_start(struct snd_pcm_substream *substream)
{
	/* Local variable declaration */
	int				ret = ERROR_NONE;
	struct vcd_playback_option	vcd_ply_opt;
	struct vcd_playback_buffer_info	vcd_ply_buf;

	sndp_log_debug_func("start\n");

	/* Status update */
	g_status |= PLAY_STATUS;

	/* Playback data information initialize */
	call_pcm_info_init(substream, PLAY_STATUS);

	/* Get playback buffer */
	sndp_log_debug("vcd_execute() cmd=VCD_COMMAND_GET_PLAYBACK_BUFFER\n");
	ret = call_vcd_execute((int)VCD_COMMAND_GET_PLAYBACK_BUFFER,
			       (void *)&vcd_ply_buf);
	if (ERROR_NONE != ret) {
		sndp_log_err("Error COMMAND_GET_PLAYBACK_BUFFER(code=%d)\n",
			     ret);
		g_call_pcm_info[SNDP_PCM_OUT].byte_offset = 16;
		/* Status update */
		g_status &= ~PLAY_STATUS;
		return -EPERM;
	}

	/* Data pointer setting */
	g_call_play_data_addr[DATA_SIDE_0] =
		(u_int)vcd_ply_buf.playback_buffer[DATA_SIDE_0];
	g_call_play_data_addr[DATA_SIDE_1] =
		(u_int)vcd_ply_buf.playback_buffer[DATA_SIDE_1];

	/* Play mode set */
	if (0 == g_sndp_mode) {
		/*
		 * DownLink
		 * UL : Speech Enable,  Playback Disable
		 * DL : Speech Enable,  Playback Enable
		 */
		if (false == g_call_play_uplink) {
			vcd_ply_opt.mode = VCD_PLAYBACK_MODE_0;
		/*
		 * UpLink
		 * UL : Speech Disable, Playback Enable
		 * DL : Speech Disable, Playback Disable
		 */
		} else {
			vcd_ply_opt.mode = VCD_PLAYBACK_MODE_1;
		}
	/* For debug, Force mode change */
	} else {
		vcd_ply_opt.mode = g_sndp_mode - 1;
	}

	/* Callback pointer set */
	vcd_ply_opt.beginning_buffer = call_playback_cb;

	/* Play data set */
	call_playback_data_set();

	/* Start speech playback */
	sndp_log_debug("vcd_execute() cmd=VCD_COMMAND_START_PLAYBACK\n");

	ret = call_vcd_execute((int)VCD_COMMAND_START_PLAYBACK,
			       (void *)&vcd_ply_opt);

	if (ERROR_NONE != ret) {
		sndp_log_err("VCD_COMMAND_START_PLAYBACK return (code=%d)\n",
			     ret);
		g_call_pcm_info[SNDP_PCM_OUT].byte_offset = 16;
		/* Status update */
		g_status &= ~PLAY_STATUS;
		ret = -EPERM;
	}

	sndp_log_debug_func("end\n");
	return ret;
}


/*!
   @brief Speech + Playback Stop function

   @param[in]	none
   @param[out]	none

   @retval	none
 */
void call_playback_stop(void)
{
	sndp_log_debug_func("start\n");

#ifdef DEBUG
	g_call_cb_debug[SNDP_PCM_OUT] = true;
#endif

	/* Stop speech playback */
	sndp_log_debug("vcd_execute() cmd=VCD_COMMAND_STOP_PLAYBACK\n");
	(void)call_vcd_execute((int)VCD_COMMAND_STOP_PLAYBACK, NULL);

	/* If cross Vocoder callback, Data information clear */
	g_call_playback_len = 0;
	g_call_pcm_info[SNDP_PCM_OUT].byte_offset = 0;

	/* Status update */
	g_status &= ~PLAY_STATUS;

	sndp_log_debug_func("end\n");
}


/*!
   @brief Speech + Record Start function

   @param[in]	substream	PCM substream structure
   @param[out]	none

   @retval	0		Successful
   @retval	-EPERM		Not ready
 */
int call_record_start(struct snd_pcm_substream *substream)
{
	/* Local variable declaration */
	int				ret = ERROR_NONE;
	struct vcd_record_option	vcd_rec_opt;
	struct vcd_record_buffer_info	vcd_rec_buf;

	sndp_log_debug_func("start\n");

	/* Status update */
	g_status |= REC_STATUS;

	/* Record information clear */
	call_pcm_info_init(substream, REC_STATUS);

	/* Get record buffer */
	sndp_log_debug("vcd_execute() cmd=VCD_COMMAND_GET_RECORD_BUFFER\n");
	ret = call_vcd_execute((int)VCD_COMMAND_GET_RECORD_BUFFER,
			       (void *)&vcd_rec_buf);
	if (ERROR_NONE != ret) {
		sndp_log_err("Error COMMAND_GET_RECORD_BUFFER(code=%d)\n",
			     ret);
		/* Status update */
		g_status &= ~REC_STATUS;
		return -EPERM;
	}

	/* Data pointer setting */
	g_call_rec_data_addr[DATA_SIDE_0] =
		(u_int)vcd_rec_buf.record_buffer[DATA_SIDE_0];
	g_call_rec_data_addr[DATA_SIDE_1] =
		(u_int)vcd_rec_buf.record_buffer[DATA_SIDE_1];

	/*
	 * Rec mode set
	 * UL : Enable, DL : Enable
	 */
	vcd_rec_opt.mode = VCD_RECORD_MODE_0;

	/* Callback pointer set */
	vcd_rec_opt.complete_buffer = call_record_cb;

	/* Start speech record */
	sndp_log_debug("vcd_execute() cmd=VCD_COMMAND_START_RECORD\n");

	ret = call_vcd_execute((int)VCD_COMMAND_START_RECORD,
			       (void *)&vcd_rec_opt);

	if (ERROR_NONE != ret) {
		sndp_log_err("Error COMMAND_START_RECORD(code=%d)\n", ret);
		/* Status update */
		g_status &= ~REC_STATUS;
		ret = -EPERM;
	}

	sndp_log_debug_func("end\n");
	return ret;
}


/*!
   @brief Speech + Record Stop function

   @param[in]	none
   @param[out]	none

   @retval	none
 */
void call_record_stop(void)
{
	sndp_log_debug_func("start\n");

#ifdef DEBUG
	g_call_cb_debug[SNDP_PCM_IN] = true;
#endif

	/* Dummy status check */
	if (g_call_dummy_rec) {
		g_call_dummy_rec = false;
		wake_up_interruptible(&g_call_wait_in);
		sndp_log_debug("DUMMY_REC OFF [%02x]\n", g_call_dummy_rec);
		return;
	}

	/* Stop speech record */
	sndp_log_debug("vcd_execute() cmd=VCD_COMMAND_STOP_RECORD\n");
	(void)call_vcd_execute((int)VCD_COMMAND_STOP_RECORD, NULL);

	/* If cross Vocoder callback, Data information clear */
	g_call_record_len = 0;
	g_call_pcm_info[SNDP_PCM_IN].byte_offset = 0;

	/* Status update */
	g_status &= ~REC_STATUS;

	sndp_log_debug_func("end\n");
}


/*!
   @brief Speech + Playback/Record buffer offset return function

   @param[in]	none
   @param[out]	none

   @retval	PCM data buffer offset
 */
snd_pcm_uframes_t call_pcmdata_pointer(struct snd_pcm_substream *substream)
{
	long location = g_call_pcm_info[substream->stream].byte_offset;

#ifdef DEBUG
	/* If offset < 16Byte, Reduce the amount of log. */
	if ((g_call_pcm_info[substream->stream].buffer_len - 16) <=
	    location) {
		sndp_log_debug("%ld frames (%ld bytes)\n",
			bytes_to_frames(substream->runtime, location),
			location);
	}

#endif
	return bytes_to_frames(substream->runtime, location);
}


/*!
   @brief VOCODER Set Callback function for VCD Watch

   @param[in]	Callback function
   @param[out]	none

   @retval	0	Successful
 */
int call_regist_watch(callback_func callback, callback_func_clk callback_clk)
{
	/* Local variable declaration */
	int ret = ERROR_NONE;

	sndp_log_debug_func("start\n");

	/* Set callback function */
	g_call_sndp_stop_fw = callback;

	/* Set callback for Vocoder */
	sndp_log_debug("vcd_execute() cmd=VCD_COMMAND_WATCH_STOP_FW\n");
	ret = call_vcd_execute((int)VCD_COMMAND_WATCH_STOP_FW,
			       (void *)call_watch_stop_fw_cb);
	if (ERROR_NONE > ret) {
		sndp_log_err("Error COMMAND_WATCH_STOP_FW(code=%d)\n", ret);
		ret = -EPERM;
	} else {
		/* Set callback for Vocoder */
		sndp_log_debug(
			"vcd_execute() cmd=VCD_COMMAND_WATCH_START_CLKGEN\n");
		ret = call_vcd_execute((int)VCD_COMMAND_WATCH_START_CLKGEN,
				       (void *)callback_clk);
		if (ERROR_NONE > ret) {
			sndp_log_err(
			"Error VCD_COMMAND_WATCH_START_CLKGEN(code=%d)\n", ret);
			ret = -EPERM;
		}
	}

	sndp_log_debug_func("end\n");
	return ret;
}


/*!
   @brief Record dummy change function

   @param[in]	none
   @param[out]	none

   @retval	none
 */
void call_change_dummy_rec(void)
{
	/* Dummy rec ON */
	g_call_dummy_rec = true;
	g_call_dummy_record_len = g_call_record_len;
	queue_work(g_call_queue_in, &g_call_work_in);

	/* If Record status ON */
	if (REC_STATUS & g_status)
		g_status &= ~REC_STATUS;
}


/*!
   @brief UpLink status set function

   @param[in]	UpLink flag
   @param[out]	none

   @retval	none
 */
void call_set_play_uplink(bool flag)
{
	g_call_play_uplink = flag;
}


/*!
   @brief UpLink status get function

   @param[in]	none
   @param[out]	none

   @retval	g_call_play_uplink	UpLink flag
 */
int call_read_play_uplink_state(void)
{
	return g_call_play_uplink;
}


/*!
   @brief create work queue

   @param[in]	none
   @param[out]	none

   @retval	0		Successful
   @retval	-ENOMEM		create_workqueue error.
 */
int call_create_workque(void)
{
	sndp_log_debug_func("start\n");

	/* create work queue */
	g_call_queue_in = create_singlethread_workqueue("sndp_queue_in");
	if (NULL == g_call_queue_in) {
		sndp_log_err("in queue create error.\n");
		return -ENOMEM;
	}

	sndp_log_debug_func("end\n");
	return ERROR_NONE;
}


/*!
   @brief destroy work queue

   @param[in]	none
   @param[out]	none

   @retval	none
 */
void call_destroy_workque(void)
{
	sndp_log_debug_func("start\n");

	/* destroy work queue */
	if (NULL != g_call_queue_in) {
		destroy_workqueue(g_call_queue_in);
		g_call_queue_in = NULL;
	}

	sndp_log_debug_func("end\n");
}


/*!
   @brief PCM information initialization function

   @param[in]	substream	PCM substream structure
   @param[in]	kind		Direction(playback/capture)
   @param[out]	none

   @retval	none
 */
static void call_pcm_info_init(
	struct snd_pcm_substream *substream,
	enum call_status kind)
{
	/* Local variable declaration */
	int			direction;
	struct snd_pcm_runtime	*runtime = substream->runtime;

	sndp_log_debug_func("start\n");

	direction = (PLAY_STATUS == kind) ? SNDP_PCM_OUT : SNDP_PCM_IN;

	/* Set substream */
	g_call_substream[direction] = substream;

	/* Pcm information clear */
	g_call_pcm_info[direction].buffer_len =
		runtime->buffer_size * runtime->frame_bits / 8;

	g_call_pcm_info[direction].byte_offset = 0;

	g_call_pcm_info[direction].next_pd_side = DATA_SIDE_0;

	g_call_pcm_info[direction].period_len =
		frames_to_bytes(runtime, runtime->period_size);

	g_call_pcm_info[direction].period = 0;

	sndp_log_debug("buffer_len %ld (buffer_size %ld) period_len %d\n",
		g_call_pcm_info[direction].buffer_len,
		runtime->buffer_size,
		g_call_pcm_info[direction].period_len);

	sndp_log_debug_func("end\n");
}


/*!
   @brief Vocoder API call function

   @param[in]	Vocoder api command
   @param[in]	Vocoder api parameter
   @param[out]	none

   @retval	Vocoder api result
 */
static int call_vcd_execute(int command, void *arg)
{
	/* Local variable declaration */
	int				ret = ERROR_NONE;
	struct vcd_execute_command	vcd_exec;

	sndp_log_debug_func("start\n");

	/* Vocoder parameter set */
	vcd_exec.command = command;
	vcd_exec.arg = arg;

	/* Vocoder API call */
	ret = vcd_execute(&vcd_exec);

	sndp_log_debug_func("end\n");
	return ret;
}


/*!
   @brief Speech + Playback Data set function

   @param[in]	none
   @param[out]	none

   @retval	none
 */
static void call_playback_data_set(void)
{
	/* Local variable declaration */
	long			call_playback_len = 0;
	struct call_pcm_info	*pcm_info = &g_call_pcm_info[SNDP_PCM_OUT];
	int			next_pd_side = pcm_info->next_pd_side;
	struct snd_pcm_runtime	*runtime;

	/* If Playback status OFF */
	if (!(PLAY_STATUS & g_status)) {
		sndp_log_info("Playback not ready");
		return;
	}

	/* If process had been driver closed. */
	if (NULL == g_call_substream[SNDP_PCM_OUT]->runtime) {
		sndp_log_info("runtime is NULL\n");
		return;
	}

	runtime = g_call_substream[SNDP_PCM_OUT]->runtime;

	if (NULL == runtime->dma_area) {
		sndp_log_info("dma_area is NULL\n");
		return;
	}

	/* Copy the data from temporary area. */
	if (0 < g_call_playback_len) {
		/* Set data */
		memcpy((void *)g_call_play_data_addr[next_pd_side],
		       (void *)g_call_playback_buff,
		       g_call_playback_len);

		/* Clear temporary area. */
		memset(g_call_playback_buff, '\0', g_call_playback_len);
		/* Clear temporary information. */
		pcm_info->period = 0;
		pcm_info->byte_offset = g_call_playback_len;
		call_playback_len = g_call_playback_len;
		g_call_playback_len = 0;
	}

	/* Set data, If Buffer <= Data */
	if ((VCD_PLAYBACK_BUFFER_SIZE - call_playback_len) <=
		(pcm_info->buffer_len - pcm_info->byte_offset)) {
		/* Set data */
		memcpy((void *)(g_call_play_data_addr[next_pd_side] +
							call_playback_len),
			runtime->dma_area + pcm_info->byte_offset,
			VCD_PLAYBACK_BUFFER_SIZE - call_playback_len);

		/* Offset update */
		pcm_info->byte_offset += VCD_PLAYBACK_BUFFER_SIZE;

		/* If Buffer > Data */
		if (VCD_PLAYBACK_BUFFER_SIZE >
			(pcm_info->buffer_len - pcm_info->byte_offset)) {
			/*
			 * Length of the temporary area
			 * to update information.
			 */
			g_call_playback_len =
				pcm_info->buffer_len - pcm_info->byte_offset;

			/* Copy the data to temporary area. */
			memcpy((void *)g_call_playback_buff,
				runtime->dma_area + pcm_info->byte_offset,
				g_call_playback_len);

			/* Offset update */
			pcm_info->byte_offset += g_call_playback_len;
		}
		/* sndp_log_debug(
			"then : byte_offset %ld  g_call_playback_len %ld\n",
			 pcm_info->byte_offset,
			g_call_playback_len); */

	/* Set data, If Buffer > Data */
	} else {
		/* 0 padding */
		memset((void *)(g_call_play_data_addr[next_pd_side] +
							call_playback_len),
			'\0',
			VCD_PLAYBACK_BUFFER_SIZE - call_playback_len);

		/* Set data */
		memcpy((void *)(g_call_play_data_addr[next_pd_side] +
							call_playback_len),
			runtime->dma_area + pcm_info->byte_offset,
			pcm_info->buffer_len - pcm_info->byte_offset);

		/* Offset update */
		pcm_info->byte_offset +=
			(pcm_info->buffer_len - pcm_info->byte_offset);
		/* sndp_log_debug("else : g_byte_offset %ld\n", g_byte_offset); */
	}

	/* If it reaches the period of data */
	if ((0 < call_playback_len) ||
	    (pcm_info->byte_offset >=
		(pcm_info->period_len * (pcm_info->period + 1)))) {

		/* Period update */
		pcm_info->period =
			pcm_info->byte_offset / pcm_info->period_len;

		if (runtime->periods == pcm_info->period) {
			/* Update again, the period */
			pcm_info->period = 0;
			pcm_info->byte_offset = 0;
		}

		/* Notification period to ALSA */
		snd_pcm_period_elapsed(g_call_substream[SNDP_PCM_OUT]);
	}

	/* Set next position */
	pcm_info->next_pd_side = (DATA_SIDE_0 == pcm_info->next_pd_side) ?
						DATA_SIDE_1 : DATA_SIDE_0;
}


/*!
   @brief Speech + Record Data set function

   @param[in]	none
   @param[out]	none

   @retval	none
 */
static void call_record_data_set(void)
{
	/* Local variable declaration */
	struct call_pcm_info	*pcm_info = &g_call_pcm_info[SNDP_PCM_IN];
	int			next_pd_side = pcm_info->next_pd_side;
	struct snd_pcm_runtime	*runtime;

	/* If Record status OFF */
	if (!(REC_STATUS & g_status)) {
		sndp_log_info("Record not ready");
		return;
	}

	/* If process had been driver closed. */
	if (NULL == g_call_substream[SNDP_PCM_IN]->runtime) {
		sndp_log_info("runtime is NULL\n");
		return;
	}

	runtime = g_call_substream[SNDP_PCM_IN]->runtime;

	if (NULL == runtime->dma_area) {
		sndp_log_info("dma_area is NULL\n");
		return;
	}

	/* Get data, If Buffer >= Data */
	if ((g_call_record_len + VCD_RECORD_BUFFER_SIZE) <=
		(pcm_info->buffer_len - pcm_info->byte_offset)) {
		/* Copy the data from temporary area. */
		if (0 < g_call_record_len) {
			/* Get data */
			memcpy((runtime->dma_area + pcm_info->byte_offset),
			       (void *)g_call_record_buff,
			       g_call_record_len);

			/* Clear temporary area and information. */
			memset(g_call_record_buff, '\0', g_call_record_len);
			g_call_record_len = 0;
		}

		/* Get data */
		memcpy((void *)(runtime->dma_area +
				pcm_info->byte_offset +
				g_call_record_len),
		       (void *)g_call_rec_data_addr[next_pd_side],
		       VCD_RECORD_BUFFER_SIZE);

		/* Offset update */
		pcm_info->byte_offset +=
			(g_call_record_len + VCD_RECORD_BUFFER_SIZE);
		/* sndp_log_debug(
			"then : byte_offset %ld  g_call_record_len %ld\n",
			pcm_info->byte_offset,
			(g_call_record_len + VCD_RECORD_BUFFER_SIZE)); */

	/* Get data, If Buffer < Data */
	} else {
		/* Get data */
		memcpy((void *)(runtime->dma_area + pcm_info->byte_offset),
		       (void *)g_call_rec_data_addr[next_pd_side],
		       pcm_info->buffer_len - pcm_info->byte_offset);

		/* Offset update */
		pcm_info->byte_offset +=
			(pcm_info->buffer_len - pcm_info->byte_offset);

		/* Length of the temporary area to update information. */
		g_call_record_len = VCD_RECORD_BUFFER_SIZE -
			(pcm_info->buffer_len - pcm_info->byte_offset);

		/* Copy the data to temporary area. */
		memcpy(g_call_record_buff,
		       (void *)(g_call_rec_data_addr[next_pd_side] +
			       (pcm_info->buffer_len - pcm_info->byte_offset)),
		       g_call_record_len);
	}

	/* If it reaches the period of data */
	if ((0 < g_call_record_len) ||
	    (pcm_info->byte_offset >=
		(pcm_info->period_len * (pcm_info->period + 1)))) {

		/* Period update */
		pcm_info->period =
			pcm_info->byte_offset / pcm_info->period_len;

		if (runtime->periods == pcm_info->period) {
			/* Update again, the period */
			pcm_info->period = 0;
			pcm_info->byte_offset = 0;
		}

		/* Notification period to ALSA */
		snd_pcm_period_elapsed(g_call_substream[SNDP_PCM_IN]);
	}

	/* Set next position */
	pcm_info->next_pd_side = (DATA_SIDE_0 == pcm_info->next_pd_side) ?
						DATA_SIDE_1 : DATA_SIDE_0;
}


/*!
   @brief Speech + Playback Callback function

   @param[in]	none
   @param[out]	none

   @retval	none
 */
static void call_playback_cb(void)
{
#ifdef DEBUG
	if (g_call_cb_debug[SNDP_PCM_OUT]) {
		sndp_log_info("first call for vocoder\n");
		g_call_cb_debug[SNDP_PCM_OUT] = false;
	}
#endif
	/* Data set */
	call_playback_data_set();
}


/*!
   @brief Speech + Record Callback function

   @param[in]	none
   @param[out]	none

   @retval	none
 */
static void call_record_cb(void)
{
#ifdef DEBUG
	if (g_call_cb_debug[SNDP_PCM_IN]) {
		sndp_log_info("first call for vocoder\n");
		g_call_cb_debug[SNDP_PCM_IN] = false;
	}
#endif
	/* Data get */
	call_record_data_set();
}


/*!
   @brief VCD_COMMAND_WATCH_STOP_FW callback function

   @param[in]	none
   @param[out]	none

   @retval	none
 */
static void call_watch_stop_fw_cb(void)
{
	sndp_log_debug("VCD is close <incall>\n");

	/* Callback to soundpathlogical */
	g_call_sndp_stop_fw(0);
}


/*!
   @brief Work queue Dammy Record process function

   @param[in]	Work queue
   @param[out]	none

   @retval	none
 */
static void call_work_dummy_rec(struct work_struct *work)
{
	/* Local variable declaration */
	long wait_ret;
	struct call_pcm_info	*pcm_info = &g_call_pcm_info[SNDP_PCM_IN];
	struct snd_pcm_runtime	*runtime;

	/* sndp_log_debug_func("start\n"); */

	/* From work queue */
	if (NULL != work) {
		/* 20msec wait */
		wait_ret = wait_event_interruptible_timeout(
				g_call_wait_in,
				!(g_call_dummy_rec),
				msecs_to_jiffies(CALL_WAIT_TIME));

		/* Status check */
		if ((!(g_call_dummy_rec)) || (0 != wait_ret)) {
			sndp_log_debug("dummy flag %02x\n", g_call_dummy_rec);
			return;
		}
	}

	/* If process had been driver closed. */
	if (NULL == g_call_substream[SNDP_PCM_IN]->runtime) {
		sndp_log_info("runtime is NULL\n");
		return;
	}

	runtime = g_call_substream[SNDP_PCM_IN]->runtime;

	if (NULL == runtime->dma_area) {
		sndp_log_info("dma_area is NULL\n");
		return;
	}

	/* Get data, If Buffer >= Data */
	if ((g_call_dummy_record_len + VCD_RECORD_BUFFER_SIZE) <=
		(pcm_info->buffer_len - pcm_info->byte_offset)) {
		/* Copy the data from temporary area. */
		if (0 < g_call_dummy_record_len) {
			/* Get 0 padding data */
			memset((runtime->dma_area + pcm_info->byte_offset),
			       0,
			       g_call_dummy_record_len);

			/* Clear temporary information. */
			g_call_dummy_record_len = 0;
		}

		/* Get 0 padding data */
		memset((void *)(runtime->dma_area +
				pcm_info->byte_offset +
				g_call_dummy_record_len),
		       0,
		       VCD_RECORD_BUFFER_SIZE);

		/* Offset update */
		pcm_info->byte_offset +=
			(g_call_dummy_record_len + VCD_RECORD_BUFFER_SIZE);

	/* Get data, If Buffer < Data */
	} else {
		/* Get 0 padding data */
		memset((void *)(runtime->dma_area + pcm_info->byte_offset),
		       0,
		       pcm_info->buffer_len - pcm_info->byte_offset);

		/* Offset update */
		pcm_info->byte_offset +=
			(pcm_info->buffer_len - pcm_info->byte_offset);

		/* Length of the temporary area to update information. */
		g_call_dummy_record_len = VCD_RECORD_BUFFER_SIZE -
			(pcm_info->buffer_len - pcm_info->byte_offset);
	}

	/* If it reaches the period of data */
	if ((0 < g_call_dummy_record_len) ||
	    (pcm_info->byte_offset >=
		(pcm_info->period_len * (pcm_info->period + 1)))) {

		/* Period update */
		pcm_info->period =
			pcm_info->byte_offset / pcm_info->period_len;

		if (runtime->periods == pcm_info->period) {
			/* Update again, the period */
			pcm_info->period = 0;
			pcm_info->byte_offset = 0;
		}

		/* Notification period to ALSA */
		snd_pcm_period_elapsed(g_call_substream[SNDP_PCM_IN]);
	}

	/* Work queue, Queuing again */
	if (g_call_dummy_rec)
		queue_work(g_call_queue_in, &g_call_work_in);

	/* sndp_log_debug_func("end\n"); */
}


