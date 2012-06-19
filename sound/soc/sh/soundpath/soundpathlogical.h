/* soundpathlogical.h
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

#ifndef __SOUNDPATHLOGICAL_H__
#define __SOUNDPATHLOGICAL_H__

#include <linux/kernel.h>

/*
 *
 * EXTERN Declarations
 *
 */
/* Trigger stop parameter setting for FSI */
extern void fsi_set_trigger_stop(
	struct snd_pcm_substream *substream,
	bool flag);
/* FSI PM RUNTIME control */
extern void fsi_set_run_time(
	void *sndp_fsi_suspend,
	void *sndp_fsi_resume);
/* MAXIM DAI functions */
#if 0
extern max98090_ctl_dai_func_t g_max98090_ctrl_dai_func;
#endif

extern void fsi_set_slave(const bool slave);


/*
 *
 * PROTOTYPE Declarations
 *
 */

/* Proc read for sndp (DEBUG LOG) */
static int sndp_proc_read(
	char *page,
	char **start,
	off_t offset,
	int count,
	int *eof,
	void *data);
/* Proc write for sndp (DEBUG LOG) */
static int sndp_proc_write(
	struct file *filp,
	const char *buffer,
	unsigned long count,
	void *data);
/* Proc read for Register dump (DEBUG LOG) */
static int sndp_proc_reg_dump_read(
	char *page,
	char **start,
	off_t offset,
	int count,
	int *eof,
	void *data);
/* Proc write for Register dump (DEBUG LOG) */
static int sndp_proc_reg_dump_write(
	struct file *filp,
	const char *buffer,
	unsigned long count,
	void *data);

/* SOC INFO */
static int sndp_soc_info(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_info *uinfo);
/* SOC GET */
static int sndp_soc_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol);
/* SOC PUT */
static int sndp_soc_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol);

/* SOC functions */
static int sndp_soc_get_voice_out_volume(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol);
static int sndp_soc_put_voice_out_volume(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol);
static int sndp_soc_capture_volume(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol);
static int sndp_soc_get_capture_mute(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol);
static int sndp_soc_put_capture_mute(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol);

/* FSI control functions */
static int sndp_fsi_startup(
	struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai);
static void sndp_fsi_shutdown(
	struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai);
static int sndp_fsi_hw_params(
	struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params,
	struct snd_soc_dai *dai);
static int sndp_fsi_trigger(
	struct snd_pcm_substream *substream,
	int cmd,
	struct snd_soc_dai *dai);
static int sndp_fsi_set_fmt(struct snd_soc_dai *dai, u_int fmt);
static snd_pcm_uframes_t sndp_fsi_pointer(
	struct snd_pcm_substream *substream);
static int sndp_fsi_suspend(struct device *dev);
static int sndp_fsi_resume(struct device *dev);

/* MAXIM control functions */
#if 0	/* TODO */
static int sndp_maxim_startup(
	struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai);
static void sndp_maxim_shutdown(
	struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai);
static int sndp_maxim_hw_params(
	struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params,
	struct snd_soc_dai *dai);
#endif	/* TODO */

/* PATH Switchging / Back-out control functions */
static void sndp_path_switching(const u_int uiValue);
static void sndp_path_backout(const u_int uiValue);

/* Trigger process for Call */
static void sndp_call_trigger(
	struct snd_pcm_substream *substream,
	int cmd,
	struct snd_soc_dai *dai,
	u_int value);

/* Next set device type, to identify */
static u_long sndp_get_next_devices(const u_int uiValue);

/* Work queue processing for call start */
static void sndp_work_voice_start(struct work_struct *work);
/* Work queue processing for call stop */
static void sndp_work_voice_stop(struct work_struct *work);
/* Work queue processing for voice call device change */
static void sndp_work_voice_dev_chg(struct work_struct *work);
/* Work queue processing for device change (not voice call) */
static void sndp_work_normal_dev_chg(struct work_struct *work);

/* Work queue processing for Playback start */
static void sndp_work_maxim_play_start(struct work_struct *work);
/* Work queue processing for Capture start */
static void sndp_work_maxim_capture_start(struct work_struct *work);
/* Work queue processing for Playback stop */
static void sndp_work_maxim_play_stop(struct work_struct *work);
/* Work queue processing for Capture stop */
static void sndp_work_maxim_capture_stop(struct work_struct *work);

/* Work queue processing for Start during a call playback */
static void sndp_work_call_playback_start(struct work_struct *work);
/* Work queue processing for Start during a call capture */
static void sndp_work_call_capture_start(struct work_struct *work);
/* Work queue processing for Stop during a call playback */
static void sndp_work_call_playback_stop(struct work_struct *work);
/* Work queue processing for Stop during a call capture */
static void sndp_work_call_capture_stop(struct work_struct *work);

/* VCD_COMMAND_WATCH_STOP_FW registration process */
static void sndp_regist_watch(void);
/* Work queue processing for VCD_COMMAND_WATCH_STOP_FW process */
static void sndp_work_watch_stop_fw(struct work_struct *work);

/* Work queue processing for FM Radio start */
static void sndp_work_fm_radio_start(struct work_struct *work);
/* Work queue processing for FM Radio stop */
static void sndp_work_fm_radio_stop(struct work_struct *work);


/* MAXIM start / stop control functions */
static void sndp_maxim_work_start(const int direction);
static void sndp_maxim_work_stop(
	struct work_struct *work,
	const int direction);

/* Voice stop and Normal device change */
/* (Post-processing of this sndp_work_call_capture_stop()) */
static void sndp_after_of_work_call_capture_stop(
	const u_int iInValue,
	const u_int iOutValue);

/* Watch stop Firmware notification callback function */
static void sndp_watch_stop_fw_cb(u_int uiNop);

/* Wake up callback function */
static void sndp_watch_clk_cb(void);

/* Subfunction of the sndp_work_voice_dev_chg() */
/* AudioLSI -> BT-SCO */
static int sndp_work_voice_dev_chg_max98090_to_bt(
	const u_int old_value,
	const u_int new_value);
/* BT-SCO -> AudioLSI */
static int sndp_work_voice_dev_chg_bt_to_max98090(
	const u_int old_value,
	const u_int new_value);
/* AudioLSI ->AudioLSI */
static int sndp_work_voice_dev_chg_in_max98090(
	const u_int old_value,
	const u_int new_value);

/*
 *
 * MACRO Declarations
 *
 */

/* SOC Interface */
#define SNDPDRV_SOC_ENUM_EXT(xname, xenum)			\
{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER,			\
	.name = xname,						\
	.info = sndp_soc_info,					\
	.get = sndp_soc_get,					\
	.put = sndp_soc_put,					\
	.private_value = (unsigned long)&xenum			\
}

#define SNDPDRV_SOC_SINGLE(xname, reg, shift, max, invert, xget, xput)	\
{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER,				\
	.name = xname,							\
	.info = snd_soc_info_volsw,					\
	.get = xget,							\
	.put = xput,							\
	.private_value = SOC_SINGLE_VALUE(reg, shift, max, invert)	\
}

/* for informs of data receiving */
#define LOG_INIT_CYCLE_COUNT(stream)	g_sndp_log_cycle_counter[stream] = 0
#define LOG_GET_CYCLE_COUNT_MAX()	(g_sndp_log_level >> 16)

/* SoundDriver Status SET/GET */
#define SET_SNDP_STATUS(stream, set_status)	\
	g_sndp_main[stream].status = set_status
#define GET_SNDP_STATUS(stream)		g_sndp_main[stream].status

/* Old value (PCM) SET/GET */
#define SET_OLD_VALUE(stream, value)	g_sndp_main[stream].old_value = value
#define GET_OLD_VALUE(stream)		g_sndp_main[stream].old_value

/* For Stop trigger conditions */
#define SNDP_STOP_TRIGGER_CHECK(idx)					\
	(g_sndp_stop_trigger_condition[idx] & (0x0000ffff << (idx * 16)))

#define SNDP_STOP_TRIGGER_INIT_SET(idx)					\
	(g_sndp_stop_trigger_condition[idx] &=				\
	(g_sndp_stop_trigger_condition[idx] & (0xffff0000 >> (idx * 16))))


/*
 *
 * DEFINE Definitions
 *
 */
#define SNDPDRV_VOICE_VOL_MAX	(25)	/* Volume MAX value */

#define SNDP_WAIT_MAX	(3000)		/* TRIGGER STOP Wait time max */

/* Wait time max for wait queue for start voice process wake up */
#define SNDP_WATCH_CLK_TIME_OUT		(1000)

/*
 *
 * STRUCTURE Definitions
 *
 */

/* Process ID, when mode change by soc_put. */
enum sndp_proc_e {
	SNDP_PROC_NO			= 0x0000, /* No action               */
	SNDP_PROC_SHUTDOWN		= 0x0001, /* HW Shutdown(FSI, MAXIM) */
	SNDP_PROC_MAXIM_START		= 0x0002, /* MAXIM setting           */
	SNDP_PROC_CALL_START		= 0x0004, /* Call on                 */
	SNDP_PROC_CALL_STOP		= 0x0008, /* Disconnect              */
	SNDP_PROC_WATCH_STOP_FW		= 0x0010, /* Stop watch VCD          */
	SNDP_PROC_DEV_CHANGE		= 0x0020, /* Device change for call  */
};

/* Status */
enum sndp_stat_e {
	SNDP_STAT_NOT_CHG = 0,		/* 0: No transition           */
	SNDP_STAT_NORMAL,		/* 1: NORMAL                  */
	SNDP_STAT_RINGTONE,		/* 2: RINGTONE                */
	SNDP_STAT_IN_CALL,		/* 3: IN_CALL                 */
	SNDP_STAT_IN_CALL_PLAY,		/* 4: Recording during a call */
	SNDP_STAT_IN_CALL_CAP,		/* 5: Capturing during a call */
	SNDP_STAT_IN_COMM,		/* 6: INCOMMUNICATION         */
};

/* Power status */
enum sndp_power_status {
	SNDP_POWER_INIT = 0,	/* 0: Initial */
	SNDP_POWER_RESUME,	/* 1: Resume  */
	SNDP_POWER_SUSPEND,	/* 2: Suspend */
};

/* Running state of the Playback or Capture */
enum sndp_play_rec_state {
	E_IDLE = 0,		/* Idle */
	E_PLAY,			/* Running Playback process */
	E_CAP,			/* Running Capture process  */
};

/* Routing type of the stream, in during a call */
enum sndp_stream_route_type {
	E_ROUTE_NORMAL = 0,	/* Normal route */
	E_ROUTE_PLAY_CHANGED,	/* Playback path, switched to the FSI */
	E_ROUTE_CAP_DUMMY,	/* Started the dummy recording */
};

/* Stop trigger conditions */
enum sndp_stop_trigger_condition {
	SNDP_STOP_TRIGGER_INIT		= 0x00000000,
	SNDP_STOP_TRIGGER_PLAYBACK	= 0x00000001,
	SNDP_STOP_TRIGGER_CAPTURE	= 0x00010000,
	SNDP_STOP_TRIGGER_VOICE		= 0x00020000,
};

/* Function pointer typedef declarations */
typedef int (*sndp_dai_startup)(struct snd_pcm_substream *,
				struct snd_soc_dai *);
typedef void (*sndp_dai_shutdown_fsi)(struct snd_pcm_substream *,
				      struct snd_soc_dai *);
typedef void (*sndp_dai_shutdown_maxim)(struct snd_soc_dai *, u_int);
typedef int (*sndp_dai_trigger)(struct snd_pcm_substream *,
				int,
				struct snd_soc_dai *);
typedef int (*sndp_dai_hw_params)(struct snd_pcm_substream *,
				  struct snd_pcm_hw_params *,
				  struct snd_soc_dai *);
typedef snd_pcm_uframes_t (*sndp_pointer)(struct snd_pcm_substream *substream);
typedef int (*sndp_dai_set_fmt)(struct snd_soc_dai *, u_int);

/* Function table */
struct sndp_dai_func {
	sndp_dai_startup	fsi_startup;
	sndp_dai_shutdown_fsi	fsi_shutdown;
	sndp_dai_trigger	fsi_trigger;
	sndp_dai_set_fmt	fsi_set_fmt;
	sndp_dai_hw_params	fsi_hw_params;
	sndp_pointer		fsi_pointer;
	sndp_dai_startup	maxim_startup;
	sndp_dai_shutdown_maxim	maxim_shutdown;
	sndp_dai_hw_params	maxim_hw_params[SNDP_PCM_DIRECTION_MAX];
};

/* ARGs table */
struct sndp_arg {
	struct snd_pcm_substream	*fsi_substream;
	struct snd_soc_dai		*fsi_dai;
	struct snd_pcm_hw_params	fsi_params;
	struct snd_pcm_substream	*maxim_substream;
	struct snd_soc_dai		*maxim_dai;
	struct snd_pcm_hw_params	maxim_params;
};

/* Stop processing table */
struct sndp_stop {
	struct snd_pcm_substream	fsi_substream;
	struct snd_soc_dai		fsi_dai;
};

/* Main processing table */
struct sndp_main {
	struct sndp_arg		arg;		/* ARGs               */
	u_int			status;		/* SoundDriver status */
	u_int			old_value;	/* Old value (PCM)    */
};

/* Mode transition table */
struct sndp_mode_trans {
	u_int next_proc;		/* Implementation processing  */
	u_int next_status;		/* After the transition state */
};

/* Work queue processing table */
struct sndp_work_info {
	struct work_struct		work;		 /* Work             */
	struct snd_pcm_substream	*save_substream; /* Substream        */
	struct sndp_stop		stop;		 /* for Stop process */
	u_int				new_value;	 /* PCM value (NEW)  */
	u_int				old_value;	 /* PCM value (OLD)  */
};

/* Wait queue for start voice process wake up */
static DECLARE_WAIT_QUEUE_HEAD(g_watch_clk_queue);

/* SOUND_TEST */
#ifdef SOUND_TEST
extern void fsi_set_callback(callback_function func);
static void sndp_fsi_interrupt(void);
#endif
/* SOUND_TEST */

#endif /* __SOUNDPATHLOGICAL_H__ */


