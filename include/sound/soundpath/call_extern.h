/* call_extern.h
 *
 * Copyright (C) 2011 Renesas Mobile Corp.
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


#ifndef __CALL_EXTERN_H__
#define __CALL_EXTERN_H__

#include <linux/kernel.h>

#ifdef __CALL_CTRL_NO_EXTERN__
#define CALL_CTRL_NO_EXTERN
#else
#define CALL_CTRL_NO_EXTERN	extern
#endif

typedef void (*callback_func)(u_int);
typedef void (*callback_func_clk)(void);

/* Call initialization function */
CALL_CTRL_NO_EXTERN void call_init(
	callback_func rat_cb,
	callback_func control_cb);
/* Speech Start function */
CALL_CTRL_NO_EXTERN void call_speech_start(void);
/* Speech Stop function */
CALL_CTRL_NO_EXTERN void call_speech_stop(void);
/* Speech Force Stop function */
CALL_CTRL_NO_EXTERN void call_force_stop(void);
/* Speech + Playback Start function */
CALL_CTRL_NO_EXTERN int call_playback_start(
	struct snd_pcm_substream *substream);
/* Speech + Playback Stop function */
CALL_CTRL_NO_EXTERN void call_playback_stop(void);
/* Speech + Record Start function */
CALL_CTRL_NO_EXTERN int call_record_start(struct snd_pcm_substream *substream);
/* Speech + Record Stop function */
CALL_CTRL_NO_EXTERN void call_record_stop(void);
/* Speech + Playback/Record buffer offset return function */
CALL_CTRL_NO_EXTERN snd_pcm_uframes_t call_pcmdata_pointer(
	struct snd_pcm_substream *substream);
/* VOCODER Set VQA mode function */
CALL_CTRL_NO_EXTERN int call_set_vqa(u_int value);
/* VOCODER Set Callback function for VCD Watch function */
CALL_CTRL_NO_EXTERN int call_regist_watch(
			callback_func callback, callback_func_clk callback_clk);
/* Record dummy change function */
CALL_CTRL_NO_EXTERN void call_change_dummy_rec(void);
CALL_CTRL_NO_EXTERN void call_change_dummy_play(void);
/* Speech UpLink status set function */
CALL_CTRL_NO_EXTERN void call_set_play_uplink(bool flag);
/* Speech UpLink status get function */
CALL_CTRL_NO_EXTERN int call_read_play_uplink_state(void);
/* Create work queue function */
CALL_CTRL_NO_EXTERN int call_create_workque(void);
/* Destroy work queue function */
CALL_CTRL_NO_EXTERN void call_destroy_workque(void);

#endif /* __CALL_EXTERN_H__ */

