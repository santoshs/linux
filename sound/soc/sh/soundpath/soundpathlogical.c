/* soundpathlogical.c
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

#define __SOUNDPATHLOGICAL_NO_EXTERN__

#include <linux/gpio.h>
#include <linux/proc_fs.h>
#include <linux/pm_runtime.h>
#include <linux/stat.h>
#include <mach/pm.h>

/* #include <linux/vcd/vcd.h> */

#include <sound/soundpath/soundpath.h>
#include <sound/soundpath/common_extern.h>
#include <sound/soundpath/scuw_extern.h>
#include <sound/soundpath/fsi_extern.h>
#include <sound/soundpath/clkgen_extern.h>
#include <sound/soundpath/call_extern.h>
#include <sound/sh_fsi.h>
#include "soundpathlogical.h"

/*
 *
 * DEFINE Definitions
 *
 */


/*
 *
 * Global Data Declarations
 *
 */

/* Operation mode string */
static const char *const g_sndpdrv_op_modes_texts[] = {
	"PCM"
};

/* for SoC */
static const struct soc_enum g_sndpdrv_op_modes_enum
	= SOC_ENUM_SINGLE(0,
			  0,
			  ARRAY_SIZE(g_sndpdrv_op_modes_texts),
			  g_sndpdrv_op_modes_texts);

/* for Kcontrol */
static struct snd_kcontrol_new g_sndpdrv_controls[] = {
	SNDPDRV_SOC_ENUM_EXT("Path", g_sndpdrv_op_modes_enum),
	SNDPDRV_SOC_SINGLE("Earpiece Volume" , 0, 0, SNDPDRV_VOICE_VOL_MAX, 0, sndp_soc_get_voice_out_volume, sndp_soc_put_voice_out_volume),
	SNDPDRV_SOC_SINGLE("Speaker Volume"  , 0, 0, SNDPDRV_VOICE_VOL_MAX, 0, sndp_soc_get_voice_out_volume, sndp_soc_put_voice_out_volume),
	SNDPDRV_SOC_SINGLE("Headphone Volume", 0, 0, SNDPDRV_VOICE_VOL_MAX, 0, sndp_soc_get_voice_out_volume, sndp_soc_put_voice_out_volume),
	SNDPDRV_SOC_SINGLE("Bluetooth Volume", 0, 0, SNDPDRV_VOICE_VOL_MAX, 0, sndp_soc_get_voice_out_volume, sndp_soc_put_voice_out_volume),
	SNDPDRV_SOC_SINGLE("Capture Volume"  , 0, 0, SNDPDRV_VOICE_VOL_MAX, 0, sndp_soc_capture_volume, sndp_soc_capture_volume),
	SNDPDRV_SOC_SINGLE("Capture Switch"  , 0, 0, 1, 0, sndp_soc_get_capture_mute, sndp_soc_put_capture_mute),
};

/* Mode change table */
const struct sndp_mode_trans g_sndp_mode_map[SNDP_MODE_MAX][SNDP_MODE_MAX] = {
	[SNDP_MODE_INIT][SNDP_MODE_NORMAL]  =	{SNDP_PROC_NO,
							SNDP_MODE_NORMAL},
	[SNDP_MODE_INIT][SNDP_MODE_RING]    =	{SNDP_PROC_NO,
							SNDP_STAT_RINGTONE},
	[SNDP_MODE_INIT][SNDP_MODE_INCALL]  =	{SNDP_PROC_CALL_START,
							SNDP_STAT_IN_CALL},
	[SNDP_MODE_INIT][SNDP_MODE_INCOMM]  =	{SNDP_PROC_INCOMM_START,
							SNDP_STAT_IN_COMM},
	[SNDP_MODE_NORMAL][SNDP_MODE_NORMAL] =	{SNDP_PROC_NO,
							SNDP_STAT_NOT_CHG},
	[SNDP_MODE_NORMAL][SNDP_MODE_RING]   =	{SNDP_PROC_NO,
							SNDP_STAT_RINGTONE},
	[SNDP_MODE_NORMAL][SNDP_MODE_INCALL] =	{SNDP_PROC_CALL_START,
							SNDP_STAT_IN_CALL},
	[SNDP_MODE_NORMAL][SNDP_MODE_INCOMM] =	{SNDP_PROC_INCOMM_START,
							SNDP_STAT_IN_COMM},
	[SNDP_MODE_RING][SNDP_MODE_NORMAL]   =	{SNDP_PROC_NO,
							SNDP_STAT_NORMAL},
	[SNDP_MODE_RING][SNDP_MODE_RING]     =	{SNDP_PROC_NO,
							SNDP_STAT_NOT_CHG},
	[SNDP_MODE_RING][SNDP_MODE_INCALL]   =	{SNDP_PROC_CALL_START,
							SNDP_STAT_IN_CALL},
	[SNDP_MODE_RING][SNDP_MODE_INCOMM]   =	{SNDP_PROC_INCOMM_START,
							SNDP_STAT_IN_COMM},
	[SNDP_MODE_INCALL][SNDP_MODE_NORMAL] =	{SNDP_PROC_CALL_STOP,
							SNDP_STAT_NORMAL},
	[SNDP_MODE_INCALL][SNDP_MODE_RING]   =	{SNDP_PROC_CALL_STOP,
							SNDP_STAT_RINGTONE},
	[SNDP_MODE_INCALL][SNDP_MODE_INCALL] =	{SNDP_PROC_DEV_CHANGE,
							SNDP_STAT_NOT_CHG},
	[SNDP_MODE_INCALL][SNDP_MODE_INCOMM] =	{SNDP_PROC_CALL_STOP |
						 SNDP_PROC_INCOMM_START,
							SNDP_STAT_IN_COMM},
	[SNDP_MODE_INCOMM][SNDP_MODE_NORMAL] =	{SNDP_PROC_INCOMM_STOP,
							SNDP_STAT_NORMAL},
	[SNDP_MODE_INCOMM][SNDP_MODE_RING]   =	{SNDP_PROC_INCOMM_STOP,
							SNDP_STAT_RINGTONE},
	[SNDP_MODE_INCOMM][SNDP_MODE_INCALL] =	{SNDP_PROC_INCOMM_STOP |
						 SNDP_PROC_CALL_START,
							SNDP_STAT_IN_CALL},
	[SNDP_MODE_INCOMM][SNDP_MODE_INCOMM] =	{SNDP_PROC_DEV_CHANGE,
							SNDP_STAT_NOT_CHG},
};

/* Function table */
static struct sndp_dai_func g_sndp_dai_func = {
	.fsi_startup		= NULL,
	.fsi_shutdown		= NULL,
	.fsi_trigger		= NULL,
	.fsi_set_fmt		= NULL,
	.fsi_hw_params		= NULL,
	.fsi_pointer		= NULL,
	.fsi_hw_free		= NULL,
};

/* Main Process table */
/* 0:Playback/1:Capture */
struct sndp_main g_sndp_main[SNDP_PCM_DIRECTION_MAX];

/* FSI format */
static u_int g_sndp_fsi_format;

/* Work queue processing table */
/* Voice start */
static struct sndp_work_info g_sndp_work_voice_start;
/* Voice stop */
static struct sndp_work_info g_sndp_work_voice_stop;
/* Device change for voice */
static struct sndp_work_info g_sndp_work_voice_dev_chg;
/* Playback start for soundpath */
static struct sndp_work_info g_sndp_work_play_start;
/* Capture start for soundpath */
static struct sndp_work_info g_sndp_work_capture_start;
/* Playback stop for soundpath */
static struct sndp_work_info g_sndp_work_play_stop;
/* Capture stop for soundpath */
static struct sndp_work_info g_sndp_work_capture_stop;
/* Start during a call playback */
static struct sndp_work_info g_sndp_work_call_playback_start;
/* Start during a call capture */
static struct sndp_work_info g_sndp_work_call_capture_start;
/* Stop during a call playback */
static struct sndp_work_info g_sndp_work_call_playback_stop;
/* Stop during a call capture */
static struct sndp_work_info g_sndp_work_call_capture_stop;
/* Start during a fm playback */
static struct sndp_work_info g_sndp_work_fm_playback_start;
/* Start during a fm capture */
static struct sndp_work_info g_sndp_work_fm_capture_start;
/* Stop during a fm playback */
static struct sndp_work_info g_sndp_work_fm_playback_stop;
/* Stop during a fm capture */
static struct sndp_work_info g_sndp_work_fm_capture_stop;
/* VCD_COMMAND_WATCH_STOP_FW process */
static struct sndp_work_info g_sndp_work_watch_stop_fw;
/* FM Radio start */
static struct sndp_work_info g_sndp_work_fm_radio_start;
/* FM Radio stop */
static struct sndp_work_info g_sndp_work_fm_radio_stop;
/* Start during a playback incommunication */
static struct sndp_work_info g_sndp_work_call_playback_incomm_start;
/* Start during a capture incommunication */
static struct sndp_work_info g_sndp_work_call_capture_incomm_start;
/* Stop during a playback incommunication */
static struct sndp_work_info g_sndp_work_call_playback_incomm_stop;
/* Stop during a capture incommunication */
static struct sndp_work_info g_sndp_work_call_capture_incomm_stop;

/* Playback incommunication start */
static struct sndp_work_info g_sndp_work_play_incomm_start;
/* Playback incommunication stop */
static struct sndp_work_info g_sndp_work_play_incomm_stop;
/* Capture incommunication start */
static struct sndp_work_info g_sndp_work_capture_incomm_start;
/* Capture incommunication stop */
static struct sndp_work_info g_sndp_work_capture_incomm_stop;


/* for Power control */
static int g_sndp_power_status = SNDP_POWER_INIT;
static struct device *g_sndp_power_domain;
static size_t g_sndp_power_domain_count;

/* for Wake Lock */
DEFINE_SPINLOCK(lock);
static u_int g_wake_lock_count;

/* for Proc control */
static struct proc_dir_entry *g_sndp_parent;

/* for Register Dump Log */
static int g_sndp_now_direction = SNDP_PCM_DIRECTION_MAX;

/* Running state of the Playback or Capture */
static int g_sndp_playrec_flg = E_IDLE;
static int g_sndp_incomm_playrec_flg = E_IDLE;

/* Routing type of the stream, in during a call */
int g_sndp_stream_route = SNDP_ROUTE_NORMAL;

/* for Stop Trigger conditions */
u_int g_sndp_stop_trigger_condition[SNDP_PCM_DIRECTION_MAX];

/* for FSI DAI OPS */
static struct snd_soc_dai_ops sndp_fsi_dai_ops = {
	.startup    = sndp_fsi_startup,
	.shutdown   = sndp_fsi_shutdown,
	.trigger    = sndp_fsi_trigger,
	.set_fmt    = sndp_fsi_set_fmt,
	.hw_params  = sndp_fsi_hw_params,
};

struct sndp_codec_info g_sndp_codec_info;
EXPORT_SYMBOL(g_sndp_codec_info);

/* pointer receive log cycle counter */
u_int g_sndp_log_cycle_counter[SNDP_PCM_DIRECTION_MAX];

/* for wait path */
DECLARE_WAIT_QUEUE_HEAD(g_sndp_start_call_queue);
u_int g_sndp_start_call_wait;

#if defined(DEBUG) && defined(__PRN_SNDP__)
/* Status */
static const struct sndp_pcm_name_suffix status_list[] = {
	{ SNDP_STAT_NOT_CHG,		"SNDP_STAT_NOT_CHG"		},
	{ SNDP_STAT_NORMAL,		"SNDP_STAT_NORMAL"		},
	{ SNDP_STAT_RINGTONE,		"SNDP_STAT_RINGTONE"		},
	{ SNDP_STAT_IN_CALL,		"SNDP_STAT_IN_CALL"		},
	{ SNDP_STAT_IN_CALL_PLAY,	"SNDP_STAT_IN_CALL_PLAY"	},
	{ SNDP_STAT_IN_CALL_CAP,	"SNDP_STAT_IN_CALL_CAP"		},
	{ SNDP_STAT_IN_COMM,		"SNDP_STAT_IN_COMM"		},
};
#endif

static int g_call_playback_stop;
static int g_fm_playback_stop;

/*!
   @brief Print Log informs of data receiving

   @param[in]	stream		Direction type
   @param[in]	frame		Frame information
   @param[out]	none

   @retval	none
 */
#if defined(DEBUG) && defined(__PRN_SNDP__)
static inline void sndp_log_data_rcv_indicator(
	const int stream,
	snd_pcm_uframes_t frame)
{
	/* Log level check */
	if (LOG_DEBUG_PRINT > LOG_BYTE_LOW(g_sndp_log_level))
		return;

	/* Receive data notification log is not output */
	if (0 == LOG_GET_CYCLE_COUNT_MAX())
		return;

	/* Specified number of cycles */
	if (LOG_GET_CYCLE_COUNT_MAX() == g_sndp_log_cycle_counter[stream]) {
		/* Initialize the cycle counter */
		LOG_INIT_CYCLE_COUNT(stream);
	}

	/* Receive data notification log is output */
	if (0 == g_sndp_log_cycle_counter[stream]) {
		if (SNDP_PCM_OUT == stream)
			sndp_log_debug("OUT : Pointer [%ld]\n", frame);
		else
			sndp_log_debug("IN  : Pointer [%ld]\n", frame);
	}

	g_sndp_log_cycle_counter[stream]++;
}
#else
#define sndp_log_data_rcv_indicator(int stream, snd_pcm_uframes_t frame) \
	do { } while (0)
#endif


/*!
   @brief Print Status change

   @param[in]	uiBefore	Before status
   @param[in]	uiAfter		After status
   @param[out]	none

   @retval	none
 */

#if defined(DEBUG) && defined(__PRN_SNDP__)

static inline void sndp_print_status_change(
	const u_int uiBefore,
	const u_int uiAfter)
{
	sndp_log_info("STATUS CHANGE: %s(%d) -> %s(%d)\n",
		((SNDP_STAT_NOT_CHG <= uiBefore) &&
		 (SNDP_STAT_IN_COMM >= uiBefore)) ?
			status_list[uiBefore].suffix : "***",
		uiBefore,
		((SNDP_STAT_NOT_CHG <= uiAfter) &&
		 (SNDP_STAT_IN_COMM >= uiAfter)) ?
			status_list[uiAfter].suffix : "***",
		uiAfter);
}
#else
#define sndp_print_status_change(const u_int uiBefore, const u_int uiAfter) \
	do { } while (0)
#endif


/*!
   @brief Next set device type, to identify

   @param[in]	uiValue		PCM value
   @param[out]	none

   @retval	Next set devices
 */
static u_long sndp_get_next_devices(const u_int uiValue)
{
	int	iRet = ERROR_NONE;
	u_long	ulTmpNextDev = g_sndp_codec_info.dev_none;
	u_int	uiDev;


	sndp_log_debug_func("start uiValue[0x%08X]\n", uiValue);

	/* OUTPUT side */
	if (SNDP_PCM_OUT == SNDP_GET_DIRECTION_VAL(uiValue)) {

		/* Get now enabled devices */
		if (NULL != g_sndp_codec_info.get_device) {
			iRet = g_sndp_codec_info.get_device(&ulTmpNextDev);
			if (ERROR_NONE != iRet)
				sndp_log_err("get_device error(code=%d)\n",
					iRet);
		}

		/* Init to Playback side */
		ulTmpNextDev &= ~g_sndp_codec_info.out_dev_all;

		/* Check OUTPUT device */
		uiDev = SNDP_GET_DEVICE_VAL(uiValue);

		/* [OUT]Speaker */
		if (SNDP_SPEAKER & uiDev)
			ulTmpNextDev |= g_sndp_codec_info.dev_playback_speaker;

		/* [OUT]Earpiece */
		if (SNDP_EARPIECE & uiDev)
			ulTmpNextDev |= g_sndp_codec_info.dev_playback_earpiece;

		/* [OUT]Headphone */
		if (SNDP_WIREDHEADPHONE & uiDev)
			ulTmpNextDev |=
				g_sndp_codec_info.dev_playback_headphones;

		/* [OUT]Headset */
		if (SNDP_WIREDHEADSET & uiDev)
			ulTmpNextDev |=
				g_sndp_codec_info.dev_playback_headphones;

		/* IN_CALL mode check */
		if (SNDP_MODE_INCALL == SNDP_GET_MODE_VAL(uiValue)) {

			/* Init to Capture side */
			ulTmpNextDev &= ~g_sndp_codec_info.in_dev_all;

			/*
			 * To identify the INPUT device,
			 * to check the OUTPUT device.
			 */
			if (SNDP_WIREDHEADSET & uiDev) {
				/*
				 * [OUT]Including the Headset,
				 * [IN]Headset MIC
				 */
				ulTmpNextDev |=
				g_sndp_codec_info.dev_capture_headset_mic;
			} else if (!(SNDP_BLUETOOTHSCO & uiDev)) {
				/*
				 * [OUT]Dosn't include the Headset, (not BT)
				 * [IN]Built-in MIC
				 */
				ulTmpNextDev |=
					g_sndp_codec_info.dev_capture_mic;
			} else {
				/*
				 * [OUT]Bluetooth SCO,
				 * [IN]Bluetooth SCO
				 */
				/* No process */
			}
		}
	/* INPUT side */
	} else {
		/* Get now enabled devices */
		if (NULL != g_sndp_codec_info.get_device) {
			iRet = g_sndp_codec_info.get_device(&ulTmpNextDev);
			if (ERROR_NONE != iRet)
				sndp_log_err("get_device error(code=%d)\n",
					     iRet);
		}

		/* Init to Capture side */
		ulTmpNextDev &= ~g_sndp_codec_info.in_dev_all;

		/* Check INPUT device */
		uiDev = SNDP_GET_DEVICE_VAL(uiValue);

		/* [IN]Built-in MIC */
		if (SNDP_BUILTIN_MIC & uiDev)
			ulTmpNextDev |= g_sndp_codec_info.dev_capture_mic;

		/* [IN]Headset MIC */
		if (SNDP_WIREDHEADSET & uiDev)
			ulTmpNextDev |=
				g_sndp_codec_info.dev_capture_headset_mic;
	}

	sndp_log_debug_func("end ulNextDev[0x%08lX]\n", ulTmpNextDev);

	return ulTmpNextDev;
}


/*!
   @brief Wake lock control

   @param[in]	kind		Wake lock kind
   @param[out]	none

   @retval	none
 */
void sndp_wake_lock(const enum sndp_wake_lock_kind kind)
{
	sndp_log_debug_func("start kind[%d] count[%d]\n",
			    kind,
			    g_wake_lock_count);

	spin_lock(&lock);

	switch (kind) {
	/* for Lock */
	case E_LOCK:
		if (0 == g_wake_lock_count)
			wake_lock(&g_sndp_wake_lock_suspend);

		g_wake_lock_count++;
		break;

	/* for UnLock */
	case E_UNLOCK:
		if (0 < g_wake_lock_count) {
			g_wake_lock_count--;
			if (0 == g_wake_lock_count)
				wake_unlock(&g_sndp_wake_lock_suspend);
		}
		break;

	/* for UnLock(Forced) */
	case E_FORCE_UNLOCK:
		if (0 < g_wake_lock_count) {
			wake_unlock(&g_sndp_wake_lock_suspend);
			g_wake_lock_count = 0;
		}
		break;

	default:
		sndp_log_err("kind error[%d]\n", kind);
		break;
	}

	spin_unlock(&lock);

	sndp_log_debug_func("end lock_count[%d]\n", g_wake_lock_count);
}


/*!
   @brief Sound path driver init function (from module_init)

   @param[in]	fsi_port_dai		DAI for FSI(CPU DAI)
   @param[in]	fsi_soc_platform	Structure for FSI SoC platform
   @param[out]	none

   @retval	0			Successful
   @retval	-EFAULT			Other error
 */
int sndp_init(struct snd_soc_dai_driver *fsi_port_dai_driver,
	      struct snd_soc_platform_driver *fsi_soc_platform)
{
	int			iRet = -EFAULT;
	int			iCnt = 0;
	struct proc_dir_entry	*entry = NULL;
	struct proc_dir_entry	*reg_dump_entry = NULL;

	sndp_log_debug_func("start\n");

	g_sndp_power_domain = NULL;
	/* Main Process table init */
	for (iCnt = 0; iCnt < SNDP_PCM_DIRECTION_MAX; iCnt++) {
		memset(&g_sndp_main[iCnt].arg, 0, sizeof(struct sndp_arg));
		g_sndp_main[iCnt].status = SNDP_STAT_NORMAL;
		g_sndp_main[iCnt].old_value = SNDP_VALUE_INIT;
		g_sndp_log_cycle_counter[iCnt] = 0;
	}

	/* FSI fotmat */
	g_sndp_fsi_format = 0;

	/* for Proc control */
	g_sndp_parent = proc_mkdir(SNDP_DRV_NAME, NULL);
	if (NULL != g_sndp_parent) {
		/* create file for log level entry */
		entry = create_proc_entry(LOG_LEVEL,
					  S_IRUGO | S_IWUGO,
					  g_sndp_parent);
		if (NULL != entry) {
			entry->read_proc  = sndp_proc_read;
			entry->write_proc = sndp_proc_write;
		} else {
			sndp_log_always_err("create_proc_entry(LOG) failed\n");
			goto mkproc_sub_err;
		}

		/* create file for register dump */
		reg_dump_entry = create_proc_entry(SNDP_REG_DUMP,
						   S_IRUGO | S_IWUGO,
						   g_sndp_parent);
		if (NULL != reg_dump_entry) {
			reg_dump_entry->read_proc = sndp_proc_reg_dump_read;
			reg_dump_entry->write_proc = sndp_proc_reg_dump_write;
		} else {
			sndp_log_always_err("create_proc_entry(REG) failed\n");
			goto mkproc_sub_err;
		}

	} else {
		sndp_log_always_err("create failed for proc parrent\n");
		goto mkproc_err;
	}

	/* for Log output */
	g_sndp_log_level = LOG_NO_PRINT;

	/* Initializing work queue processing */
	INIT_WORK(&g_sndp_work_voice_start.work,
		  sndp_work_voice_start);
	INIT_WORK(&g_sndp_work_voice_stop.work,
		  sndp_work_voice_stop);
	INIT_WORK(&g_sndp_work_voice_dev_chg.work,
		  sndp_work_voice_dev_chg);
	INIT_WORK(&g_sndp_work_play_start.work,
		  sndp_work_play_start);
	INIT_WORK(&g_sndp_work_capture_start.work,
		  sndp_work_capture_start);
	INIT_WORK(&g_sndp_work_play_stop.work,
		  sndp_work_play_stop);
	INIT_WORK(&g_sndp_work_capture_stop.work,
		  sndp_work_capture_stop);
	INIT_WORK(&g_sndp_work_call_playback_start.work,
		  sndp_work_call_playback_start);
	INIT_WORK(&g_sndp_work_call_capture_start.work,
		  sndp_work_call_capture_start);
	INIT_WORK(&g_sndp_work_call_playback_stop.work,
		  sndp_work_call_playback_stop);
	INIT_WORK(&g_sndp_work_call_capture_stop.work,
		  sndp_work_call_capture_stop);
	INIT_WORK(&g_sndp_work_fm_playback_start.work,
		  sndp_work_fm_playback_start);
	INIT_WORK(&g_sndp_work_fm_capture_start.work,
		  sndp_work_fm_capture_start);
	INIT_WORK(&g_sndp_work_fm_playback_stop.work,
		  sndp_work_fm_playback_stop);
	INIT_WORK(&g_sndp_work_fm_capture_stop.work,
		  sndp_work_fm_capture_stop);
	INIT_WORK(&g_sndp_work_watch_stop_fw.work,
		  sndp_work_watch_stop_fw);
	INIT_WORK(&g_sndp_work_fm_radio_start.work,
		  sndp_work_fm_radio_start);
	INIT_WORK(&g_sndp_work_fm_radio_stop.work,
		  sndp_work_fm_radio_stop);
	INIT_WORK(&g_sndp_work_play_incomm_start.work,
		  sndp_work_play_incomm_start);
	INIT_WORK(&g_sndp_work_play_incomm_stop.work,
		  sndp_work_play_incomm_stop);
	INIT_WORK(&g_sndp_work_capture_incomm_start.work,
		  sndp_work_capture_incomm_start);
	INIT_WORK(&g_sndp_work_capture_incomm_stop.work,
		  sndp_work_capture_incomm_stop);
	INIT_WORK(&g_sndp_work_call_playback_incomm_start.work,
		  sndp_work_call_playback_incomm_start);
	INIT_WORK(&g_sndp_work_call_capture_incomm_start.work,
		  sndp_work_call_capture_incomm_start);
	INIT_WORK(&g_sndp_work_call_playback_incomm_stop.work,
		  sndp_work_call_playback_incomm_stop);
	INIT_WORK(&g_sndp_work_call_capture_incomm_stop.work,
		  sndp_work_call_capture_incomm_stop);

	atomic_set(&g_sndp_watch_start_clk, 0);
	atomic_set(&g_sndp_watch_stop_clk, 0);

	/* Get buffer for incommunication */
	call_get_incomm_buffer();
	/*
	 * VCD_COMMAND_WATCH_STOP_FW registration
	 */
	sndp_regist_watch();

	/* To initialize the flag waiting for the trigger stop processing. */
	for (iCnt = 0; SNDP_PCM_DIRECTION_MAX > iCnt; iCnt++)
		g_sndp_stop_trigger_condition[iCnt] = SNDP_STOP_TRIGGER_INIT;

	/* To save the volume value specified by the APL
	 * (Assumption that the AudioLSI driver already started)
	 */
	if (NULL != g_sndp_codec_info.set_volum) {
		iRet = g_sndp_codec_info.set_volum(
				g_sndp_codec_info.out_dev_all,
				g_sndp_codec_info.codec_valume);
		if (ERROR_NONE != iRet) {
			sndp_log_always_err("set volume[%d] error\n",
					    g_sndp_codec_info.codec_valume);
			goto set_volume_err;
		}
	}

	/* To save the MIC mute value specified by the APL
	 * (Assumption that the AudioLSI driver already started)
	 */
	if (NULL != g_sndp_codec_info.set_mute) {
		iRet = g_sndp_codec_info.set_mute(
				g_sndp_codec_info.mute_disable);
		if (ERROR_NONE != iRet) {
			sndp_log_always_err("set mute[%d] error\n",
					    g_sndp_codec_info.mute_disable);
			goto set_mute_err;
		}
	}

	/* ioremap */
	iRet = common_ioremap();
	if (ERROR_NONE != iRet)
		goto ioremap_err;

	/* FSI master for ES 2.0 over */
	if ((system_rev & 0xffff) >= 0x3E10)
		common_set_fsi2cr(STAT_ON);

	/* Replaced of function pointers. */
	g_sndp_dai_func.fsi_startup = fsi_port_dai_driver->ops->startup;
	g_sndp_dai_func.fsi_shutdown = fsi_port_dai_driver->ops->shutdown;
	g_sndp_dai_func.fsi_trigger = fsi_port_dai_driver->ops->trigger;
	g_sndp_dai_func.fsi_set_fmt = fsi_port_dai_driver->ops->set_fmt;
	g_sndp_dai_func.fsi_hw_params = fsi_port_dai_driver->ops->hw_params;
	g_sndp_dai_func.fsi_pointer = fsi_soc_platform->ops->pointer;
	g_sndp_dai_func.fsi_hw_free = fsi_soc_platform->ops->hw_free;

	fsi_port_dai_driver[SNDP_PCM_PORTA].ops = &sndp_fsi_dai_ops;
	fsi_port_dai_driver[SNDP_PCM_PORTB].ops = &sndp_fsi_dai_ops;

	fsi_soc_platform->ops->pointer = sndp_fsi_pointer;
	fsi_soc_platform->ops->hw_free = sndp_fsi_hw_free;
	fsi_set_run_time(sndp_fsi_suspend, sndp_fsi_resume);

	/* SoC control */
	if (NULL != g_sndp_codec_info.set_soc_controls) {
		g_sndp_codec_info.set_soc_controls(
			g_sndpdrv_controls,
			ARRAY_SIZE(g_sndpdrv_controls));
	}

	/* Power domain setting */
	iRet = power_domain_devices("snd-soc-fsi",
				    &g_sndp_power_domain,
				    &g_sndp_power_domain_count);
	if (ERROR_NONE != iRet) {
		sndp_log_err("Modules not found ... [iRet=%d]\n", iRet);
		goto power_domain_err;
	}

	/* RuntimePM */
	pm_runtime_enable(g_sndp_power_domain);
	iRet = pm_runtime_resume(g_sndp_power_domain);
	if (iRet < 0) {
		sndp_log_err("pm_runtime_resume failed\n");
		goto power_domain_err;
	}

	/* create work queue for call */
	iRet = call_create_workque();
	if (ERROR_NONE != iRet) {
		sndp_log_err("work queue create error.\n");
		goto workque_create_err;
	}

	/* create work queue */
	g_sndp_queue_main = create_singlethread_workqueue("sndp_queue_main");
	if (NULL == g_sndp_queue_main) {
		sndp_log_err("Queue create error.\n");
		iRet = -ENOMEM;
		goto workque_create_err;
	}

	/* Initialize device setting for AudioLSI */
	if (NULL != g_sndp_codec_info.set_device) {
		iRet = g_sndp_codec_info.set_device(
			g_sndp_codec_info.dev_none,
			SNDP_VALUE_INIT,
			g_sndp_codec_info.power_on);
		sndp_log_debug("initial set_device\n");
		if (ERROR_NONE != iRet) {
			sndp_log_err("set device error(code=%d)\n", iRet);
			goto set_dev_err;
		}
	}

	/* Wake lock init */
	wake_lock_init(&g_sndp_wake_lock_suspend,
		       WAKE_LOCK_SUSPEND,
		       "snd-soc-fsi");

	/* wait queue init */
	init_waitqueue_head(&g_sndp_stop_wait);

	sndp_log_debug_func("end\n");
	return ERROR_NONE;

set_volume_err:
set_mute_err:
set_dev_err:
ioremap_err:
mkproc_sub_err:
/*
add_control_err:	TODO
*/
workque_create_err:
	remove_proc_entry(SNDP_DRV_NAME, NULL);
power_domain_err:
	if (g_sndp_power_domain) {
		pm_runtime_disable(g_sndp_power_domain);
		g_sndp_power_domain = NULL;
	}
mkproc_err:
	return iRet;

}


/*!
   @brief Sound path driver exit function (from module_exit)

   @param[in]	none
   @param[out]	none

   @retval	none
 */
void sndp_exit(void)
{
	/* iounmap */
	common_iounmap();

	/* RuntimePM */
	if (g_sndp_power_domain) {
		pm_runtime_disable(g_sndp_power_domain);
		g_sndp_power_domain = NULL;
	}

	/* Proc entry remove */
	if (NULL != g_sndp_parent)
		remove_proc_entry(SNDP_DRV_NAME, NULL);

	/* Destroy work queue for call */
	call_destroy_workque();

	/* Destroy work queue */
	if (NULL != g_sndp_queue_main) {
		destroy_workqueue(g_sndp_queue_main);
		g_sndp_queue_main = NULL;
	}

	/* Wake lock destroy */
	wake_lock_destroy(&g_sndp_wake_lock_suspend);
}


/*!
   @brief PROC read function

   @param[out]	page	Page data
   @param[-]	start	Not use
   @param[-]	offset	Not use
   @param[-]	count	Not use
   @param[out]	eof	EOF flag
   @param[-]	data	Not use

   @retval	read data length
 */
static int sndp_proc_read(
	char *page,
	char **start,
	off_t offset,
	int count,
	int *eof,
	void *data)
{
	int	iLen = sprintf(page, "0x%08x\n", (int)g_sndp_log_level);

	*eof = 1;
	return iLen;
}


/*!
   @brief PROC write function

   @param[-]	filp	Not use
   @param[in]	buffer	Write data
   @param[in]	count	Data length
   @param[-]	data	Not use

   @retval		Write data count
 */
static int sndp_proc_write(
	struct file *filp,
	const char *buffer,
	unsigned long count,
	void *data)
{
	unsigned long long int	uiIn;
	static unsigned char	proc_buf[32];

	memset(proc_buf, 0, sizeof(proc_buf));
	/* copy, from user */
	if (copy_from_user(proc_buf, (void __user *)buffer, count)) {
		sndp_log_err("copy_from_user failed.\n");
		return -EFAULT;
	}

	if (kstrtoull(proc_buf, 0, &uiIn)) {
		sndp_log_err("kstrtoull error\n");
		return -EFAULT;
	}

	g_sndp_log_level = (u_int)uiIn & LOG_LEVEL_MAX;
	LOG_INIT_CYCLE_COUNT(SNDP_PCM_OUT);

	return count;
}


/*!
   @brief PROC read function for register dump

   @param[out]  page    Page data
   @param[-]    start   Not use
   @param[-]    offset  Not use
   @param[-]    count   Not use
   @param[out]  eof     EOF flag
   @param[-]    data    Not use

   @retval      0
 */
static int sndp_proc_reg_dump_read(
	char *page,
	char **start,
	off_t offset,
	int count,
	int *eof,
	void *data)
{
	return ERROR_NONE;
}


/*!
   @brief PROC write function for register dump

   @param[-]	filp	Not use
   @param[in]	buffer	Write data
   @param[in]	count	Data length
   @param[-]	data	Not use

   @retval	Write data length
 */
static int sndp_proc_reg_dump_write(
	struct file *filp,
	const char *buffer,
	unsigned long count,
	void *data)
{
	int			iRet = ERROR_NONE;
	u_long			ulIn;
	u_long			log_level_back = 0;
	static unsigned char	proc_buf[32];

	sndp_log_debug_func("start\n");

	log_level_back = LOG_BIT_REG_DUMP & g_sndp_log_level;
	g_sndp_log_level |= LOG_BIT_REG_DUMP;

	memset(proc_buf, 0, sizeof(proc_buf));
	/* copy, from user */
	if (copy_from_user(proc_buf, (void __user *)buffer, count)) {
		iRet = -EFAULT;
		sndp_log_err("copy_from_user failed.\n");
		return iRet;
	}

	iRet = kstrtoul(proc_buf, 0, &ulIn);
	if (0 != iRet) {
		sndp_log_err("kstrtoul error(ret=%d)\n", iRet);
		return iRet;
	}

	sndp_log_reg_dump("ulIn : 0x%08lx\n", ulIn);

	g_sndp_mode = ((u_int)ulIn & 0xffff0000) >> 16;
	ulIn &= 0xffff;

	/* for FSI */
	if ((REG_DUMP_ALL == ulIn) || (REG_DUMP_FSI & ulIn)) {
		if (SNDP_PCM_DIRECTION_MAX != g_sndp_now_direction)
			fsi_reg_dump(GET_OLD_VALUE(g_sndp_now_direction));
		else
			sndp_log_reg_dump("FSI register is not ready.\n");
	}

	/* for CLKGEN */
	if ((REG_DUMP_ALL == ulIn) || (REG_DUMP_CLKGEN & ulIn)) {
		if (SNDP_PCM_DIRECTION_MAX != g_sndp_now_direction)
			clkgen_reg_dump();
		else
			sndp_log_reg_dump("CLKGEN register is not ready.\n");
	}

	/* for SCUW */
	if ((REG_DUMP_ALL == ulIn) || (REG_DUMP_SCUW & ulIn)) {
		if ((SNDP_MODE_INCALL == SNDP_GET_MODE_VAL(GET_OLD_VALUE(SNDP_PCM_OUT))) ||
			(SNDP_MODE_INCOMM == SNDP_GET_MODE_VAL(GET_OLD_VALUE(SNDP_PCM_OUT)))) {
			if ((SNDP_PCM_DIRECTION_MAX != g_sndp_now_direction) &&
				(!(SNDP_ROUTE_PLAY_CHANGED & g_sndp_stream_route)))
				scuw_reg_dump();
			else
				sndp_log_reg_dump("SCUW register is not ready.\n");
		}
	}

	g_sndp_log_level &= ~LOG_BIT_REG_DUMP | log_level_back;

	sndp_log_debug_func("%s out\n", __func__);

	return count;
}


/*!
   @brief Element information check function

   @param[in]	kcontrol	Kcontrol structure
   @param[out]	uinfo		Element information structure

   @retval	0		Successful
 */
static int sndp_soc_info(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_info *uinfo)
{
	struct soc_enum	*e = (struct soc_enum *)kcontrol->private_value;


	uinfo->type = SNDRV_CTL_ELEM_TYPE_ENUMERATED;
	uinfo->count = 1;
	uinfo->value.enumerated.items = e->max;

	if (uinfo->value.enumerated.item > e->max - 1)
		uinfo->value.enumerated.item = e->max - 1;

	strcpy(uinfo->value.enumerated.name,
	       e->texts[uinfo->value.enumerated.item]);

	return ERROR_NONE;
}


/*!
   @brief GET callback function for hooks control

   @param[-]	kcontrol	Not use
   @param[-]	ucontrol	Not use

   @retval	0		Successful
 */
static int sndp_soc_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	return ERROR_NONE;
}


/*!
   @brief PUT callback function for hooks control

   @param[-]	kcontrol	Not use
   @param[in]	ucontrol	Element data

   @retval	0		Successful
   @retval	-EINVAL		Invalid argument
 */
static int sndp_soc_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	char cPcm[SNDP_PCM_NAME_MAX_LEN];


	int	iRet = ERROR_NONE;
	u_int	uiValue;
	u_int	uiProcess;
	u_int	uiDirection;
	u_int	uiMode;
	u_int	uiNextStatus;
	u_int	uiSaveStatus;
	u_int	uiOldValue;
	u_int	old_mode;

	sndp_log_debug_func("start\n");

	/* Invalid error */
	if (NULL == ucontrol)
		return -EINVAL;

	/* Gets the value of PCM */
	uiValue = ucontrol->value.enumerated.item[0];

	/* Show the PCM name */
	memset(cPcm, '\0', sizeof(cPcm));
	sndp_pcm_name_generate(uiValue, cPcm);
	sndp_log_info("PCM: %s [0x%08X]\n", cPcm, uiValue);

	/* Gets the direction (Playback/Capture) */
	uiDirection = SNDP_GET_DIRECTION_VAL(uiValue);

	/* Get Old Value */
	uiOldValue = GET_OLD_VALUE(uiDirection);
	old_mode   = SNDP_GET_MODE_VAL(uiOldValue);
#if 0
	/* Uplink only/Downlink only/Both Uplink and Downlink */
	if (SNDP_PLAYBACK_UPLINK_INCALL == uiValue) {
		/* Uplink only */
		call_set_play_link(true, false);
		maxim_voice_link_mute(MUTE_ON, MUTE_OFF);
		return ERROR_NONE;
	} else if (SNDP_PLAYBACK_DOWNLINK_INCALL == uiValue) {
		/* Downlink only */
		call_set_play_link(false, true);
		maxim_voice_link_mute(MUTE_OFF, MUTE_ON);
		return ERROR_NONE;
	} else {
		/* Both Uplink and Downlink */
		call_set_play_link(false, false);
		maxim_voice_link_mute(MUTE_OFF, MUTE_OFF);
		if (uiValue == uiOldValue)
			return ERROR_NONE;
	}
#endif
	/* Gets the mode (NORMAL/RINGTONE/INCALL/INCOMMUNICATION) */
	uiMode = SNDP_GET_MODE_VAL(uiValue);

	/* Identify the process for changing modes (Old mode -> New mode) */
	uiProcess = g_sndp_mode_map[old_mode][uiMode].next_proc;

	/* Identify the next status for changing modes */
	uiNextStatus = g_sndp_mode_map[old_mode][uiMode].next_status;

	/* Save now status */
	uiSaveStatus = GET_SNDP_STATUS(uiDirection);

	/* Status Change */
	if (SNDP_STAT_NOT_CHG != uiNextStatus) {
		/* Change to new Status */
		sndp_print_status_change(uiSaveStatus, uiNextStatus);
		SET_SNDP_STATUS(uiDirection, uiNextStatus);
	}

	/* Direction check */
	if (SNDP_PCM_OUT == uiDirection) {
		/* FM Radio stop process */
		if ((SNDP_VALUE_INIT != uiOldValue) &&
		    (SNDP_FM_RADIO_RX & SNDP_GET_DEVICE_VAL(uiOldValue)) &&
		    (!(SNDP_FM_RADIO_RX & SNDP_GET_DEVICE_VAL(uiValue)))) {
			g_sndp_now_direction = SNDP_PCM_DIRECTION_MAX;

			/* Wake Lock */
			sndp_wake_lock(E_LOCK);

			/* Stop Capture running */
			g_sndp_playrec_flg &= ~E_PLAY;

			/* Registered in the work queue for FM Radio stop */
			g_sndp_work_fm_radio_stop.old_value = uiOldValue;

			queue_work(g_sndp_queue_main,
				   &g_sndp_work_fm_radio_stop.work);
		}

		/* FM Radio start process */
		if ((SNDP_FM_RADIO_RX & SNDP_GET_DEVICE_VAL(uiValue)) &&
		    !(SNDP_FM_RADIO_RX & SNDP_GET_DEVICE_VAL(uiOldValue))) {
			g_sndp_now_direction = SNDP_PCM_OUT;

			/* Wake Lock */
			sndp_wake_lock(E_LOCK);

			/* Running Capture */
			g_sndp_playrec_flg |= E_PLAY;

			/* Registered in the work queue for FM Radio start */
			g_sndp_work_fm_radio_start.new_value = uiValue;
			queue_work(g_sndp_queue_main,
					&g_sndp_work_fm_radio_start.work);

			/* Saving the type of PCM */
			SET_OLD_VALUE(uiDirection, uiValue);
			sndp_log_debug_func("end\n");
			return ERROR_NONE;
		}
	}

	/* Direction check */
	if ((SNDP_PCM_IN == uiDirection) &&
	    (SNDP_MODE_INCOMM != uiMode) &&
	    (SNDP_MODE_INCOMM != old_mode)) {
		/* FM Radio stop process */
		if ((SNDP_VALUE_INIT != uiOldValue) &&
		    (SNDP_FM_RADIO_RX & SNDP_GET_DEVICE_VAL(uiOldValue))) {
			/* Wake Lock */
			sndp_wake_lock(E_LOCK);

			/* Stop Capture running */
			g_sndp_playrec_flg &= ~E_CAP;

			/* Registered in the work queue for FM Radio stop */
			g_sndp_work_fm_radio_stop.old_value = uiOldValue;

			queue_work(g_sndp_queue_main,
				   &g_sndp_work_fm_radio_stop.work);
		}

		/* FM Radio start process */
		if (SNDP_FM_RADIO_RX & SNDP_GET_DEVICE_VAL(uiValue)) {
			/* Wake Lock */
			sndp_wake_lock(E_LOCK);

			/* Running Capture */
			g_sndp_playrec_flg |= E_CAP;

			/* Registered in the work queue for FM Radio start */
			g_sndp_work_fm_radio_start.new_value = uiValue;
			queue_work(g_sndp_queue_main,
					&g_sndp_work_fm_radio_start.work);
		}

		/* Saving the type of PCM */
		SET_OLD_VALUE(uiDirection, uiValue);
		sndp_log_debug_func("end\n");
		return ERROR_NONE;
	}

	/* for Register dump debug */
	g_sndp_now_direction =
		((SNDP_MODE_INCALL == uiMode) || (SNDP_MODE_INCOMM == uiMode)) ?
		SNDP_PCM_OUT : SNDP_PCM_DIRECTION_MAX;

	 /* for Register dump debug */
	 g_sndp_now_direction =
	    ((SNDP_MODE_INCALL == uiMode) || (SNDP_MODE_INCOMM == uiMode)) ?
	    SNDP_PCM_OUT : SNDP_PCM_DIRECTION_MAX;

	/* Processing for each process */
	/* SNDP_PROC_CALL_STOP */
	if (uiProcess & SNDP_PROC_CALL_STOP) {
		/* Call + Recording is not running */
		if (!(g_status & REC_STATUS)) {

			/* Wake Lock */
			sndp_wake_lock(E_LOCK);

			/* Registered in the work queue for call stop */
			g_sndp_work_voice_stop.old_value = uiOldValue;

			queue_work(g_sndp_queue_main,
				   &g_sndp_work_voice_stop.work);

			g_sndp_now_direction = SNDP_PCM_DIRECTION_MAX;
		}
	}

	/* Processing for each process */
	if (uiProcess & SNDP_PROC_INCOMM_STOP) {

			/* Wake Lock */
			sndp_wake_lock(E_LOCK);

			if (SNDP_PCM_OUT == uiDirection) {
				/* Registered in the work queue for play incomm stop */
				g_sndp_work_play_incomm_stop.old_value = uiOldValue;

				queue_work(g_sndp_queue_main,
					   &g_sndp_work_play_incomm_stop.work);
			} else {
				/* Registered in the work queue for capture incomm stop */
				g_sndp_work_capture_incomm_stop.old_value = uiOldValue;

				queue_work(g_sndp_queue_main,
					   &g_sndp_work_capture_incomm_stop.work);
			}
	}

	/* SNDP_PROC_CALL_START */
	if (uiProcess & SNDP_PROC_CALL_START) {
		/* for Register dump debug */
		g_sndp_now_direction = SNDP_PCM_OUT;

		/* Enable the power domain */
		iRet = pm_runtime_get_sync(g_sndp_power_domain);
		if (!(0 == iRet || 1 == iRet)) {  /* 0:success 1:active */
			sndp_log_err("modules power on error[iRet=%d]\n",
				     iRet);

			/* Revert the status */
			sndp_print_status_change(GET_SNDP_STATUS(uiDirection),
						 uiSaveStatus);
			SET_SNDP_STATUS(uiDirection, uiSaveStatus);
			return iRet;
		}

		/* Wake Lock */
		sndp_wake_lock(E_LOCK);

		/* Registered in the work queue for call start */
		g_sndp_work_voice_start.new_value = uiValue;
		queue_work(g_sndp_queue_main, &g_sndp_work_voice_start.work);
	}

	if (uiProcess & SNDP_PROC_INCOMM_START) {
		if (((SNDP_PCM_OUT == uiDirection) &&
		     (SNDP_MODE_INCOMM != SNDP_GET_MODE_VAL(GET_OLD_VALUE(SNDP_PCM_IN)))) ||
		    ((SNDP_PCM_IN == uiDirection) &&
		     (SNDP_MODE_INCOMM != SNDP_GET_MODE_VAL(GET_OLD_VALUE(SNDP_PCM_OUT))))) {

			/* Enable the power domain */
			iRet = pm_runtime_get_sync(g_sndp_power_domain);
			if (!(0 == iRet || 1 == iRet)) {  /* 0:success 1:active */
				sndp_log_err("modules power on error[iRet=%d]\n",
					     iRet);

				/* Revert the status */
				sndp_print_status_change(GET_SNDP_STATUS(uiDirection),
							 uiSaveStatus);
				SET_SNDP_STATUS(uiDirection, uiSaveStatus);
				return iRet;
			}
		}
		/* Wake Lock */
		sndp_wake_lock(E_LOCK);

		if (SNDP_PCM_OUT == uiDirection) {
			/* Registered in the work queue for play incomm stop */
			g_sndp_work_play_incomm_start.new_value = uiValue;

			queue_work(g_sndp_queue_main,
				   &g_sndp_work_play_incomm_start.work);
		} else {
			/* Registered in the work queue for capture incomm stop */
			g_sndp_work_capture_incomm_start.new_value = uiValue;

			queue_work(g_sndp_queue_main,
				   &g_sndp_work_capture_incomm_start.work);
		}
	}

	/* SNDP_PROC_DEV_CHANGE */
	if (uiProcess & SNDP_PROC_DEV_CHANGE) {
		/* Modes other than the INIT */
		if (SNDP_VALUE_INIT != uiOldValue) {
			/* Change the device type */
			if ((SNDP_GET_DEVICE_VAL(uiOldValue) !=
					SNDP_GET_DEVICE_VAL(uiValue))) {

				/* Wake Lock */
				sndp_wake_lock(E_LOCK);

				/*
				 * Registered in the work queue for
				 * voice call device change
				 */
				sndp_log_debug("uiValue %08x  old_value %08x\n",
						uiValue, uiOldValue);

				g_sndp_work_voice_dev_chg.new_value =
								uiValue;

				g_sndp_work_voice_dev_chg.old_value =
								uiOldValue;

				queue_work(g_sndp_queue_main,
					   &g_sndp_work_voice_dev_chg.work);
			}
		}
	}

	/* Saving the type of PCM */
	SET_OLD_VALUE(uiDirection, uiValue);

	sndp_log_debug_func("end\n");
	return ERROR_NONE;
}


/*!
   @brief GET callback function for hooks control(Volume setting)

   @param[-]	kcontrol	Not use
   @param[in]	ucontrol	Element data

   @retval	0		Successful
   @retval	-EIO		kernel-side error
 */
static int sndp_soc_get_voice_out_volume(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	int	iRet = ERROR_NONE;
	u_int	uiVal = 0;


	/* Get the current volume */
	if (NULL != g_sndp_codec_info.get_volume) {
		iRet = g_sndp_codec_info.get_volume(
				g_sndp_codec_info.out_dev_all,
				&uiVal);
		if (ERROR_NONE > iRet) {
			sndp_log_err("get volume error(code=%d)\n", iRet);
			return iRet;
		}
	}

	/* Return the current settings */
	ucontrol->value.enumerated.item[0] = uiVal;

	return iRet;
}


/*!
   @brief PUT callback function for hooks control(Volume setting)

   @param[-]	kcontrol	Not use
   @param[in]	ucontrol	Element data

   @retval	0		Successful
   @retval	-EINVAL		Invalid argument
 */
static int sndp_soc_put_voice_out_volume(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	int		iRet = ERROR_NONE;
	u_int	uiVal = ucontrol->value.enumerated.item[0];


	/* Set the Volume value */
	if (NULL != g_sndp_codec_info.set_volum) {
		iRet = g_sndp_codec_info.set_volum(
				g_sndp_codec_info.out_dev_all,
				uiVal);
		if (ERROR_NONE != iRet) {
			sndp_log_err("set volume error(code=%d)\n", iRet);
			return iRet;
		}
	}

	return iRet;
}


/*!
   @brief callback function for hooks control(Volume setting)

   @param[-]	kcontrol	Not use
   @param[in]	ucontrol	Element data

   @retval	0		Successful
 */
static int sndp_soc_capture_volume(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	/* Not processing */
	return ERROR_NONE;
}


/*!
   @brief GET callback function for hooks control(Mute setting)

   @param[-]	kcontrol	Not use
   @param[in]	ucontrol	Element data

   @retval	0		Successful
 */
static int sndp_soc_get_capture_mute(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	int	iRet = ERROR_NONE;
	u_int	uiVal = 0;


	/* Get the current Mute status */
	if (NULL != g_sndp_codec_info.get_mute) {
		iRet = g_sndp_codec_info.get_mute(&uiVal);
		if (ERROR_NONE != iRet) {
			sndp_log_err("get mute error(code=%d)\n", iRet);
			return iRet;
		}
	}

	/* Return the current settings */
	ucontrol->value.enumerated.item[0] = !(uiVal);

	return iRet;
}


/*!
   @brief PUT callback function for hooks control(Mute setting)

   @param[-]	kcontrol	Not use
   @param[in]	ucontrol	Element data

   @retval	0		Successful
   @retval	-EINVAL		Invalid argument
 */
static int sndp_soc_put_capture_mute(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	int	iRet = ERROR_NONE;
	u_int	uiVal = ucontrol->value.enumerated.item[0];

	/* Set the Mute status */
	if (NULL != g_sndp_codec_info.set_mute) {
		iRet = g_sndp_codec_info.set_mute(uiVal);
		if (ERROR_NONE != iRet) {
			sndp_log_err("set mute error(code=%d)\n", iRet);
			return iRet;
		}
	}

	return iRet;
}


/*!
   @brief System suspend function

   @param[in]	dev	Device
   @param[out]	none

   @retval	ERROR_NONE
 */
static int sndp_fsi_suspend(struct device *dev)
{
	int	iRet = ERROR_NONE;
	u_int	iInDev = SNDP_NO_DEVICE;
	u_int	iOutDev = SNDP_NO_DEVICE;

	sndp_log_info("start\n");

	/* Device get from old_value */
	if (SNDP_VALUE_INIT != GET_OLD_VALUE(SNDP_PCM_IN))
		iInDev = SNDP_GET_DEVICE_VAL(GET_OLD_VALUE(SNDP_PCM_IN));
	if (SNDP_VALUE_INIT != GET_OLD_VALUE(SNDP_PCM_OUT))
		iOutDev = SNDP_GET_DEVICE_VAL(GET_OLD_VALUE(SNDP_PCM_OUT));

	sndp_log_debug("InDev[0x%08X] OutDev[0x%08X]\n",
			iInDev, iOutDev);

	/* Otherwise only IN_CALL, for processing and Other than FM playback */
	if ((SNDP_MODE_INCALL !=
		SNDP_GET_MODE_VAL(GET_OLD_VALUE(SNDP_PCM_OUT))) &&
	    ((!(SNDP_FM_RADIO_RX & iInDev)) &&
	     (!(SNDP_FM_RADIO_RX & iOutDev)))) {
		if (SNDP_POWER_SUSPEND != g_sndp_power_status) {
			/*
			 * Transition to SUSPEND,
			 * status of Audio Lsi (Disable all devices)
			 */
			if (NULL != g_sndp_codec_info.set_device) {
				iRet = g_sndp_codec_info.set_device(
					g_sndp_codec_info.dev_none,
					SNDP_VALUE_INIT,
					g_sndp_codec_info.power_off);
				sndp_log_debug("set_device all 0\n");
				if (ERROR_NONE != iRet)
					sndp_log_err(
						"set device error(code=%d)\n",
						iRet);
			}

			g_sndp_power_status = SNDP_POWER_SUSPEND;
		}
	}

	sndp_log_info("end\n");

	return ERROR_NONE;
}


/*!
   @brief System resume function

   @param[in]	dev	Device
   @param[out]	none

   @retval	ERROR_NONE
 */
static int sndp_fsi_resume(struct device *dev)
{
	int	iRet = ERROR_NONE;
	u_long	ulSetDevice = g_sndp_codec_info.dev_none;
	u_int	iInDev = SNDP_NO_DEVICE;
	u_int	iOutDev = SNDP_NO_DEVICE;

	sndp_log_info("start\n");

	/* Device get from old_value */
	if (SNDP_VALUE_INIT != GET_OLD_VALUE(SNDP_PCM_IN))
		iInDev = SNDP_GET_DEVICE_VAL(GET_OLD_VALUE(SNDP_PCM_IN));
	if (SNDP_VALUE_INIT != GET_OLD_VALUE(SNDP_PCM_OUT))
		iOutDev = SNDP_GET_DEVICE_VAL(GET_OLD_VALUE(SNDP_PCM_OUT));

	sndp_log_debug("InDev[0x%08X] OutDev[0x%08X]\n",
			iInDev, iOutDev);

	/* Otherwise only IN_CALL, for processing and Other than FM playback */
	if ((SNDP_MODE_INCALL !=
		SNDP_GET_MODE_VAL(GET_OLD_VALUE(SNDP_PCM_OUT))) &&
	    ((!(SNDP_FM_RADIO_RX & iInDev)) &&
	     (!(SNDP_FM_RADIO_RX & iOutDev)))) {
		if (SNDP_POWER_RESUME != g_sndp_power_status) {
			/* Transition to RESUME, status of Audio Lsi */
			ulSetDevice =
			sndp_get_next_devices(GET_OLD_VALUE(SNDP_PCM_OUT));

			if (NULL != g_sndp_codec_info.set_device) {
				iRet = g_sndp_codec_info.set_device(
						g_sndp_codec_info.dev_none,
						GET_OLD_VALUE(SNDP_PCM_OUT),
						g_sndp_codec_info.power_on);
				sndp_log_debug("set_device 0x%08lX\n",
						ulSetDevice);
				if (ERROR_NONE != iRet)
					sndp_log_err(
						"set device error(code=%d)\n",
						iRet);
			}

			g_sndp_power_status = SNDP_POWER_RESUME;
		}
	}

	sndp_log_info("end\n");

	return ERROR_NONE;
}


/*!
   @brief FSI startup function

   @param[in]	substream	PCM substream structure
   @param[in]	dai		Digital audio interface structure

   @retval	0		Successful
 */
static int sndp_fsi_startup(
	struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	int	iRet = ERROR_NONE;


	sndp_log_debug_func("start\n");

	sndp_log_info("substream->stream = %d(%s)  old_value = 0x%08X\n",
		substream->stream,
		(SNDP_PCM_OUT == substream->stream) ? "PLAYBACK" : "CAPTURE",
		GET_OLD_VALUE(substream->stream));

	/* Playback or Capture, than the Not processing */
	if ((SNDP_PCM_OUT != substream->stream) &&
		(SNDP_PCM_IN != substream->stream)) {
		return iRet;
	}

	/* To store information about Substream and DAI */
	g_sndp_main[substream->stream].arg.fsi_substream = substream;
	g_sndp_main[substream->stream].arg.fsi_dai = dai;

	sndp_log_debug_func("end\n");
	return iRet;
}


/*!
   @brief FSI shutdown function

   @param[in]	substream	PCM substream structure
   @param[in]	dai		Digital audio interface structure

   @retval	none
 */
static void sndp_fsi_shutdown(
	struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	sndp_log_debug_func("start\n");

	sndp_log_info("substream->stream = %d(%s)  old_value = 0x%08X\n",
		substream->stream,
		(SNDP_PCM_OUT == substream->stream) ? "PLAYBACK" : "CAPTURE",
		GET_OLD_VALUE(substream->stream));

	sndp_log_info("g_sndp_stop_trigger_condition[%d] = 0x%08x\n",
		substream->stream,
		g_sndp_stop_trigger_condition[substream->stream]);

	/* Playback or Capture, than the Not processing */
	if ((SNDP_PCM_OUT != substream->stream) &&
	    (SNDP_PCM_IN != substream->stream)) {
		return;
	}

	sndp_log_debug("val set\n");

	/* To store information about Substream and DAI */
	g_sndp_main[substream->stream].arg.fsi_substream = substream;
	g_sndp_main[substream->stream].arg.fsi_dai = dai;

	sndp_log_debug_func("end\n");
}


/*!
   @brief FSI trigger function

   @param[in]	substream	PCM substream structure
   @param[in]	cmd		Trigger command type
				(SNDRV_PCM_TRIGGER_START/
				 SNDRV_PCM_TRIGGER_STOP)
   @param[in]	dai		Digital audio interface structure

   @retval	0		Successful
 */
static int sndp_fsi_trigger(
	struct snd_pcm_substream *substream,
	int cmd,
	struct snd_soc_dai *dai)
{
	int			iRet = ERROR_NONE;
	struct sndp_stop	*stop;
	struct sndp_arg		*arg;
	char			cPcm[SNDP_PCM_NAME_MAX_LEN];
	struct snd_pcm_runtime	*runtime = substream->runtime;

	sndp_log_debug_func("start\n");

	sndp_log_info("stream = %d(%s)  old_value = 0x%08X  cmd = %s\n",
		substream->stream,
		(SNDP_PCM_OUT == substream->stream) ? "PLAYBACK" : "CAPTURE",
		GET_OLD_VALUE(substream->stream),
		(SNDRV_PCM_TRIGGER_START == cmd) ?
					"TRIGGER_START" : "TRIGGER_STOP");

	/* Playback or Capture, than the Not processing */
	if ((SNDP_PCM_OUT != substream->stream) &&
		(SNDP_PCM_IN != substream->stream)) {
		return iRet;
	}

	/* Initialize the cycle counter */
	LOG_INIT_CYCLE_COUNT(substream->stream);

	/* Checking Mode */
	switch (SNDP_GET_MODE_VAL(GET_OLD_VALUE(substream->stream))) {
	case SNDP_MODE_INCALL:
		/* Call process route */
		sndp_call_trigger(substream,
				  cmd,
				  dai,
				  GET_OLD_VALUE(substream->stream));
		break;
	case SNDP_MODE_INCOMM:
		/* VoIP process route */
		sndp_incomm_trigger(substream,
				  cmd,
				  dai,
				  GET_OLD_VALUE(substream->stream));
		break;
	default:
		if (SNDP_FM_RADIO_RX &
		    SNDP_GET_DEVICE_VAL(GET_OLD_VALUE(substream->stream))) {
			sndp_fm_trigger(substream,
					  cmd,
					  dai,
					  GET_OLD_VALUE(substream->stream));
		} else {
			/* MM Playback or MM Capture process route */
			switch (cmd) {
			case SNDRV_PCM_TRIGGER_START:	/* TRIGGER_START */
				sndp_log_info("#Trigger start[MM]\n");

				/* Display the name of PCM */
				sndp_pcm_name_generate(
					GET_OLD_VALUE(substream->stream), cPcm);
				sndp_log_info("PCM: %s [0x%08X]\n",
					cPcm, GET_OLD_VALUE(substream->stream));

				sndp_log_debug("buff_size %ld  period_size %ld"
						"periods %d  frame_bits %d\n",
						runtime->buffer_size,
						runtime->period_size,
						runtime->periods,
						runtime->frame_bits);

				/* Wake Lock */
				sndp_wake_lock(E_LOCK);

				/* A work queue processing */
				if (SNDP_PCM_OUT == substream->stream) {
					queue_work(g_sndp_queue_main,
						&g_sndp_work_play_start.work);

					/* for Register dump debug */
					g_sndp_now_direction = SNDP_PCM_OUT;
				} else {
					queue_work(g_sndp_queue_main,
						&g_sndp_work_capture_start.work);

					/* for Register dump debug */
					g_sndp_now_direction = SNDP_PCM_IN;
				}
				break;

			case SNDRV_PCM_TRIGGER_STOP:	/* TRIGGER_STOP */
				sndp_log_info("#Trigger stop[MM]\n");

				arg = &g_sndp_main[substream->stream].arg;
				/* FSI trigger stop process */
				fsi_set_trigger_stop(arg->fsi_substream,
						     false);

				/* Init register dump log flag for debug */
				g_sndp_now_direction = SNDP_PCM_DIRECTION_MAX;

				/* A work queue processing */
				if (SNDP_PCM_OUT == substream->stream) {
					g_sndp_stop_trigger_condition[SNDP_PCM_OUT] |=
						SNDP_STOP_TRIGGER_PLAYBACK;

					stop = &g_sndp_work_play_stop.stop;

					stop->fsi_substream =
							*arg->fsi_substream;

					stop->fsi_dai = *arg->fsi_dai;

					queue_work(g_sndp_queue_main,
						&g_sndp_work_play_stop.work);
				} else {
					g_sndp_stop_trigger_condition[SNDP_PCM_IN] |=
						SNDP_STOP_TRIGGER_CAPTURE;

					stop = &g_sndp_work_capture_stop.stop;

					stop->fsi_substream =
							*arg->fsi_substream;

					stop->fsi_dai = *arg->fsi_dai;

					queue_work(g_sndp_queue_main,
						&g_sndp_work_capture_stop.work);
				}
				break;
			default:
				sndp_log_debug("playback trigger none.\n");
				break;
			}
		}
	}

	sndp_log_debug_func("end[%s %s]\n",
		(SNDP_PCM_OUT == substream->stream) ? "PLAYBACK" : "CAPTURE",
		(SNDRV_PCM_TRIGGER_START == cmd) ?
					"TRIGGER_START" : "TRIGGER_STOP");
	return iRet;
}


/*!
   @brief FSI Set format function

   @param[in]	dai	Digital audio interface structure
   @param[in]	fmt	Format type

   @retval	0	Successful
 */
static int sndp_fsi_set_fmt(struct snd_soc_dai *dai, u_int fmt)
{
	sndp_log_debug_func("start\n");
	sndp_log_debug("fmt = 0x%08X\n", fmt);

	/* To store information about Hardware parameters */
	g_sndp_fsi_format = fmt;

	sndp_log_debug_func("end\n");
	return ERROR_NONE;
}


/*!
   @brief FSI HW Parameters function

   @param[in]	substream	PCM substream structure
   @param[in]	params		HW Parameters structure
   @param[in]	dai		Digital audio interface structure

   @retval	0		Successful
 */
static int sndp_fsi_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params,
	struct snd_soc_dai *dai)
{
	sndp_log_debug_func("start\n");
	sndp_log_debug("substream->stream = %d  params = %d\n",
			substream->stream, params_rate(params));

	/* To store information about Hardware parameters */
	g_sndp_main[substream->stream].arg.fsi_params = *params;

	sndp_log_debug_func("end\n");
	return ERROR_NONE;
}


/*!
   @brief FSI pointer function

   @param[in]	substream	PCM substream structure

   @retval	Data position
 */
static snd_pcm_uframes_t sndp_fsi_pointer(struct snd_pcm_substream *substream)
{
	snd_pcm_uframes_t	iRet = 0;

	if (SNDP_MODE_INCALL ==
		SNDP_GET_MODE_VAL(GET_OLD_VALUE(substream->stream))) {
		/* Not in a call */

		/* VCD is dead */
		if ((SNDP_ROUTE_PLAY_CHANGED & g_sndp_stream_route) &&
		    (SNDP_PCM_OUT == substream->stream)) {
			iRet = g_sndp_dai_func.fsi_pointer(substream);

		/* VCD is alive */
		} else {
			if (g_call_playback_stop)
				iRet = 0;
			else
				iRet = call_pcmdata_pointer(substream);
		}
	} else if (SNDP_MODE_INCOMM ==
		SNDP_GET_MODE_VAL(GET_OLD_VALUE(substream->stream))) {
		if (((SNDP_PCM_OUT == substream->stream) &&
			(g_status & PLAY_INCOMM_STATUS)) ||
			((SNDP_PCM_IN == substream->stream) &&
			(g_status & REC_INCOMM_STATUS)))
			iRet = call_incomm_pcmdata_pointer(substream);
		else
			iRet = 0;
	} else {
		/* During a call */
		iRet = g_sndp_dai_func.fsi_pointer(substream);
	}

	sndp_log_data_rcv_indicator(substream->stream, iRet);
	return iRet;
}

static int sndp_fsi_hw_free(struct snd_pcm_substream *substream)
{
	int			ret;

	sndp_log_debug_func("start\n");

	ret = wait_event_interruptible_timeout(
		g_sndp_stop_wait, !SNDP_STOP_TRIGGER_CHECK(substream->stream),
		msecs_to_jiffies(SNDP_WAIT_MAX));

	SNDP_STOP_TRIGGER_INIT_SET(substream->stream);

	sndp_log_debug("TRIGGER_STOP had been waiting to complete.\n");

	ret = g_sndp_dai_func.fsi_hw_free(substream);

	sndp_log_debug_func("end\n");

	return ret;
}

/*!
   @brief During a call trigger function

   @param[in]	substream	PCM substream structure
   @param[in]	cmd		Trigger command type
				(SNDRV_PCM_TRIGGER_START/
				 SNDRV_PCM_TRIGGER_STOP)
   @param[in]	dai		Digital audio interface structure
   @param[in]	value		PCM value

   @retval	none
 */
static void sndp_call_trigger(
	struct snd_pcm_substream *substream,
	int cmd,
	struct snd_soc_dai *dai,
	u_int value)
{
	sndp_log_debug_func("start\n");

	/* Branch processing for each command (TRIGGER_START/TRIGGER_STOP) */
	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:	/* TRIGGER_START */
		sndp_log_debug("call_%s_start\n",
				(SNDP_PCM_OUT == substream->stream) ?
						"playback" : "record");

		/* Wake Lock */
		sndp_wake_lock(E_LOCK);

		/* For during a call playback */
		if (SNDP_PCM_OUT == substream->stream) {
			/*
			 * Status change
			 * (from SNDP_STAT_IN_CALL to SNDP_STAT_IN_CALL_PLAY)
			 */
			sndp_print_status_change(GET_SNDP_STATUS(SNDP_PCM_OUT),
						 SNDP_STAT_IN_CALL_PLAY);

			SET_SNDP_STATUS(SNDP_PCM_OUT, SNDP_STAT_IN_CALL_PLAY);

			/*
			 * To register a work queue to start processing
			 * during a call playback
			 */
			g_sndp_work_call_playback_start.save_substream =
								substream;

			queue_work(g_sndp_queue_main,
				   &g_sndp_work_call_playback_start.work);

		/* For during a call capture */
		} else {
			/*
			 * Status change
			 * (from SNDP_STAT_IN_CALL to SNDP_STAT_IN_CALL_CAP)
			 */
			sndp_print_status_change(GET_SNDP_STATUS(SNDP_PCM_IN),
						 SNDP_STAT_IN_CALL_CAP);

			SET_SNDP_STATUS(SNDP_PCM_IN, SNDP_STAT_IN_CALL_CAP);

			/*
			 * To register a work queue to start processing
			 * during a call capture
			 */
			g_sndp_work_call_capture_start.save_substream =
								substream;

			queue_work(g_sndp_queue_main,
				   &g_sndp_work_call_capture_start.work);
		}
		break;
	case SNDRV_PCM_TRIGGER_STOP:	/* TRIGGER_STOP */
		sndp_log_debug("call_%s_stop\n",
				(SNDP_PCM_OUT == substream->stream) ?
						"playback" : "record");

		/* For during a call playback */
		if (SNDP_PCM_OUT == substream->stream) {
			if (SNDP_ROUTE_PLAY_CHANGED & g_sndp_stream_route)
				fsi_set_trigger_stop(substream, false);
			else
				g_call_playback_stop = true;

			/*
			 * Status change
			 * (from SNDP_STAT_IN_CALL_PLAY to SNDP_STAT_IN_CALL)
			 */
			sndp_print_status_change(GET_SNDP_STATUS(SNDP_PCM_OUT),
						 SNDP_STAT_IN_CALL);

			SET_SNDP_STATUS(SNDP_PCM_OUT, SNDP_STAT_IN_CALL);

			/*
			 * To register a work queue to stop processing
			 * during a call playback
			 */
			g_sndp_stop_trigger_condition[SNDP_PCM_OUT] |=
						SNDP_STOP_TRIGGER_PLAYBACK;

			g_sndp_work_call_playback_stop.stop.fsi_substream =
								*substream;

			g_sndp_work_call_playback_stop.stop.fsi_dai = *dai;

			queue_work(g_sndp_queue_main,
				   &g_sndp_work_call_playback_stop.work);

		/* For during a call capture */
		} else {
			/*
			 * Status change
			 * (from SNDP_STAT_IN_CALL_CAP to SNDP_STAT_IN_CALL)
			 */
			sndp_print_status_change(GET_SNDP_STATUS(SNDP_PCM_IN),
						 SNDP_STAT_IN_CALL);

			SET_SNDP_STATUS(SNDP_PCM_IN, SNDP_STAT_IN_CALL);

			/* To register a work queue to stop */
			/* processing During a call capture */
			g_sndp_stop_trigger_condition[SNDP_PCM_IN] |=
			(SNDP_STOP_TRIGGER_CAPTURE | SNDP_STOP_TRIGGER_VOICE);
			queue_work(g_sndp_queue_main,
				   &g_sndp_work_call_capture_stop.work);
		}
		break;

	default:
		sndp_log_debug("Trigger none.\n");
		break;
	}

	sndp_log_debug_func("end[%s %s]\n",
		(SNDP_PCM_OUT == substream->stream) ?
					"PLAYBACK" : "CAPTURE",
		(SNDRV_PCM_TRIGGER_START == cmd) ?
					"TRIGGER_START" : "TRIGGER_STOP");
}


/*!
   @brief During a fm trigger function

   @param[in]	substream	PCM substream structure
   @param[in]	cmd		Trigger command type
				(SNDRV_PCM_TRIGGER_START/
				 SNDRV_PCM_TRIGGER_STOP)
   @param[in]	dai		Digital audio interface structure
   @param[in]	value		PCM value

   @retval	none
 */
static void sndp_fm_trigger(
	struct snd_pcm_substream *substream,
	int cmd,
	struct snd_soc_dai *dai,
	u_int value)
{
	struct snd_pcm_runtime	*runtime = substream->runtime;

	sndp_log_debug_func("start\n");

	/* Branch processing for each command (TRIGGER_START/TRIGGER_STOP) */
	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:	/* TRIGGER_START */
		sndp_log_debug("fm_%s_start\n",
				(SNDP_PCM_OUT == substream->stream) ?
						"playback" : "record");

		/* Wake Lock */
		sndp_wake_lock(E_LOCK);

		/* For during a fm playback */
		if (SNDP_PCM_OUT == substream->stream) {

			sndp_log_debug("buffer_size %ld  period_size %ld  "
				"periods %d  frame_bits %d\n",
				runtime->buffer_size, runtime->period_size,
				runtime->periods, runtime->frame_bits);
			/*
			 * To register a work queue to start processing
			 * during a fm playback
			 */
			g_sndp_work_fm_playback_start.save_substream =
								substream;

			queue_work(g_sndp_queue_main,
				   &g_sndp_work_fm_playback_start.work);

		/* For during a fm capture */
		} else {

			g_sndp_work_fm_capture_start.save_substream =
								substream;

			queue_work(g_sndp_queue_main,
				   &g_sndp_work_fm_capture_start.work);
		}
		break;
	case SNDRV_PCM_TRIGGER_STOP:	/* TRIGGER_STOP */
		sndp_log_debug("fm_%s_stop\n",
				(SNDP_PCM_OUT == substream->stream) ?
						"playback" : "record");

		/* For during a fm playback */
		if (SNDP_PCM_OUT == substream->stream) {
			if (SNDP_ROUTE_PLAY_CHANGED & g_sndp_stream_route)
				fsi_set_trigger_stop(substream, false);
			else
				g_fm_playback_stop = true;

			/*
			 * To register a work queue to stop processing
			 * during a fm playback
			 */
			g_sndp_stop_trigger_condition[SNDP_PCM_OUT] |=
						SNDP_STOP_TRIGGER_PLAYBACK;

			g_sndp_work_fm_playback_stop.stop.fsi_substream =
								*substream;

			g_sndp_work_fm_playback_stop.stop.fsi_dai = *dai;

			queue_work(g_sndp_queue_main,
				   &g_sndp_work_fm_playback_stop.work);

		/* For during a fm capture */
		} else {

			/* To register a work queue to stop */
			/* processing During a fm capture */
			g_sndp_stop_trigger_condition[SNDP_PCM_IN] |=
			(SNDP_STOP_TRIGGER_CAPTURE);
			queue_work(g_sndp_queue_main,
				   &g_sndp_work_fm_capture_stop.work);
		}
		break;

	default:
		sndp_log_debug("Trigger none.\n");
		break;
	}

	sndp_log_debug_func("end[%s %s]\n",
		(SNDP_PCM_OUT == substream->stream) ?
					"PLAYBACK" : "CAPTURE",
		(SNDRV_PCM_TRIGGER_START == cmd) ?
					"TRIGGER_START" : "TRIGGER_STOP");
}


/*!
   @brief During a VoIP trigger function

   @param[in]	substream	PCM substream structure
   @param[in]	cmd		Trigger command type
				(SNDRV_PCM_TRIGGER_START/
				 SNDRV_PCM_TRIGGER_STOP)
   @param[in]	dai		Digital audio interface structure
   @param[in]	value		PCM value

   @retval	none
 */
static void sndp_incomm_trigger(
	struct snd_pcm_substream *substream,
	int cmd,
	struct snd_soc_dai *dai,
	u_int value)
{
	char			cPcm[SNDP_PCM_NAME_MAX_LEN];
	struct snd_pcm_runtime	*runtime = substream->runtime;

	sndp_log_debug_func("start\n");

	/* Branch processing for each command (TRIGGER_START/TRIGGER_STOP) */
	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:	/* TRIGGER_START */
		/* Display the name of PCM */
		sndp_pcm_name_generate(
			GET_OLD_VALUE(substream->stream), cPcm);
		sndp_log_info("PCM: %s [0x%08X]\n",
			cPcm, GET_OLD_VALUE(substream->stream));

		sndp_log_debug("buffer_size %ld  period_size %ld  "
			"periods %d  frame_bits %d\n",
			runtime->buffer_size, runtime->period_size,
			runtime->periods, runtime->frame_bits);

		sndp_log_debug("call_%s_incomm_start\n",
				(SNDP_PCM_OUT == substream->stream) ?
						"playback" : "record");

		/* Wake Lock */
		sndp_wake_lock(E_LOCK);

		/* For during a call playback */
		if (SNDP_PCM_OUT == substream->stream) {
			/*
			 * To register a work queue to start processing
			 * during a playback incommunication
			 */
			g_sndp_work_call_playback_incomm_start.save_substream =
								substream;

			queue_work(g_sndp_queue_main,
				   &g_sndp_work_call_playback_incomm_start.work);

		/* For during a call capture */
		} else {
			/*
			 * To register a work queue to start processing
			 * during a capture incommunication
			 */
			g_sndp_work_call_capture_incomm_start.save_substream =
								substream;

			queue_work(g_sndp_queue_main,
				   &g_sndp_work_call_capture_incomm_start.work);
		}
		break;
	case SNDRV_PCM_TRIGGER_STOP:	/* TRIGGER_STOP */
		sndp_log_debug("call_%s_incomm_stop\n",
				(SNDP_PCM_OUT == substream->stream) ?
						"playback" : "record");

		/* For during a playback incommunication */
		if (SNDP_PCM_OUT == substream->stream) {
			/*
			 * To register a work queue to stop processing
			 * during a call playback
			 */
			g_sndp_stop_trigger_condition[SNDP_PCM_OUT] |=
						SNDP_STOP_TRIGGER_PLAYBACK;

			g_sndp_work_call_playback_incomm_stop.stop.fsi_substream =
								*substream;

			g_sndp_work_call_playback_incomm_stop.stop.fsi_dai =
								*dai;

			queue_work(g_sndp_queue_main,
				   &g_sndp_work_call_playback_incomm_stop.work);

		/* For during a capture incommunication */
		} else {
			/* To register a work queue to stop */
			/* processing During a call capture */
			g_sndp_stop_trigger_condition[SNDP_PCM_IN] |=
						SNDP_STOP_TRIGGER_CAPTURE;
			queue_work(g_sndp_queue_main,
				   &g_sndp_work_call_capture_incomm_stop.work);
		}
		break;

	default:
		sndp_log_debug("Trigger none.\n");
		break;
	}

	sndp_log_debug_func("end[%s %s]\n",
		(SNDP_PCM_OUT == substream->stream) ?
					"PLAYBACK" : "CAPTURE",
		(SNDRV_PCM_TRIGGER_START == cmd) ?
					"TRIGGER_START" : "TRIGGER_STOP");
}


/*!
   @brief Work queue function for Voice Start

   @param[in]	work	work queue structure
   @param[out]	none

   @retval	none
 */
static void sndp_work_voice_start(struct work_struct *work)
{
	int			iRet = ERROR_NONE;
	u_long			ulSetDevice = g_sndp_codec_info.dev_none;
	struct sndp_work_info	*wp = NULL;


	sndp_log_debug_func("start\n");

	/* Initialization of the firmware starting notice receiving flag */
	atomic_set(&g_call_watch_start_fw, 0);

	/* FSI master for ES 2.0 over */
	if ((system_rev & 0xffff) >= 0x3E10)
		common_set_fsi2cr(STAT_OFF);

	/* To get a work queue structure */
	wp = container_of((void *)work, struct sndp_work_info, work);

	/* set device  */
	ulSetDevice = sndp_get_next_devices(wp->new_value);
	if (NULL != g_sndp_codec_info.set_device) {
		iRet = g_sndp_codec_info.set_device(
				ulSetDevice,
				wp->new_value,
				g_sndp_codec_info.power_on);
		if (ERROR_NONE != iRet) {
			sndp_log_err("set device error (code=%d)\n", iRet);
			goto start_err;
		}
	}

	/* start SCUW */
	iRet = scuw_start(wp->new_value);
	if (ERROR_NONE != iRet) {
		sndp_log_err("scuw start error(code=%d)\n", iRet);
		goto start_err;
	}

	/* start FSI */
	iRet = fsi_start(wp->new_value);
	if (ERROR_NONE != iRet) {
		sndp_log_err("fsi start error(code=%d)\n", iRet);
		goto start_err;
	}

	wait_event_interruptible_timeout(
		g_watch_start_clk_queue, atomic_read(&g_sndp_watch_start_clk),
		msecs_to_jiffies(SNDP_WATCH_CLK_TIME_OUT));
	atomic_set(&g_sndp_watch_start_clk, 0);

	/* start CLKGEN */
	iRet = clkgen_start(wp->new_value, 0);
	if (ERROR_NONE != iRet) {
		sndp_log_err("clkgen start error(code=%d)\n", iRet);
		goto start_err;
	}

	/* Set to ENABLE the speaker amp */
	if (SNDP_SPEAKER & SNDP_GET_DEVICE_VAL(wp->new_value)) {
		if (NULL != g_sndp_codec_info.set_speaker_amp) {
			iRet = g_sndp_codec_info.set_speaker_amp(
					g_sndp_codec_info.speaker_enable);
			if (ERROR_NONE != iRet) {
				sndp_log_err(
					"speaker_amp ENABLE error(code=%d)\n",
					iRet);
				goto start_err;
			}
		}
	}

start_err:
	g_sndp_start_call_wait = 1;
	wake_up_interruptible(&g_sndp_start_call_queue);
	/* Wake Unlock */
	sndp_wake_lock(E_UNLOCK);

	sndp_log_debug_func("end\n");
}


/*!
   @brief Work queue function for Voice Stop

   @param[in]	work	work queue structure
   @param[out]	none

   @retval	none
 */
static void sndp_work_voice_stop(struct work_struct *work)
{
	int			iRet = ERROR_NONE;
	struct sndp_work_info	*wp = NULL;


	sndp_log_debug_func("start\n");

	/* To get a work queue structure */
	wp = container_of((void *)work, struct sndp_work_info, work);

	/* Set to DISABLE the speaker amp */
	if (SNDP_SPEAKER & SNDP_GET_DEVICE_VAL(wp->old_value)) {
		if (NULL != g_sndp_codec_info.set_speaker_amp) {
			iRet = g_sndp_codec_info.set_speaker_amp(
					g_sndp_codec_info.speaker_disable);
			if (ERROR_NONE != iRet)
				sndp_log_err(
					"speaker_amp DISABLE error(code=%d)\n",
					iRet);
		}
	}

	/* stop SCUW */
	scuw_stop();

	/* stop FSI */
	fsi_stop();

	/* stop CLKGEN */
	clkgen_stop();

	/* AudioLSI device all stop */
	if (NULL != g_sndp_codec_info.set_device) {
		iRet = g_sndp_codec_info.set_device(
				g_sndp_codec_info.dev_none,
				SNDP_VALUE_INIT,
				g_sndp_codec_info.power_on);
		if (ERROR_NONE != iRet)
			sndp_log_err("set device error (code=%d)\n",
				     iRet);
	}

	/* Disable the power domain */
	iRet = pm_runtime_put_sync(g_sndp_power_domain);
	if (ERROR_NONE != iRet)
		sndp_log_debug("modules power off iRet=%d\n", iRet);

	/* FSI master for ES 2.0 over */
	if ((system_rev & 0xffff) >= 0x3E10)
		common_set_fsi2cr(STAT_ON);

	/* Wake Force Unlock */
	sndp_wake_lock(E_FORCE_UNLOCK);

	/* Trigger stop control flag update */
	g_sndp_stop_trigger_condition[SNDP_PCM_IN] &=
					~SNDP_STOP_TRIGGER_VOICE;

	/* Return from the waiting of the shutdown */
	wake_up_interruptible(&g_sndp_stop_wait);

	sndp_log_debug_func("end\n");
}


/*!
   @brief Work queue function for Device change (IN_CALL)

   @param[in]	work	work queue structure
   @param[out]	none

   @retval	none
 */
static void sndp_work_voice_dev_chg(struct work_struct *work)
{
	int			iRet = ERROR_NONE;
	struct sndp_work_info	*wp = NULL;
	u_int			old_dev = SNDP_NO_DEVICE;
	u_int			new_dev = SNDP_NO_DEVICE;


	sndp_log_debug_func("start\n");

	/* To get a work queue structure */
	wp = container_of((void *)work, struct sndp_work_info, work);

	/* get Device type */
	old_dev = SNDP_GET_DEVICE_VAL(wp->old_value);
	new_dev = SNDP_GET_DEVICE_VAL(wp->new_value);

	/* AudioLSI -> BT(SCO) */
	if ((SNDP_BLUETOOTHSCO != old_dev) &&
	    (SNDP_BLUETOOTHSCO == new_dev)) {
		iRet = sndp_work_voice_dev_chg_audioic_to_bt(
			wp->old_value, wp->new_value);
		if (ERROR_NONE != iRet)
			sndp_log_err(
				"voice_dev_chg_to_bt error(code=%d)\n", iRet);
	/* BT(SCO) -> AudioLSI */
	} else if ((SNDP_BLUETOOTHSCO == old_dev) &&
		  (SNDP_BLUETOOTHSCO != new_dev)) {
		iRet = sndp_work_voice_dev_chg_bt_to_audioic(
			wp->old_value, wp->new_value);
		if (ERROR_NONE != iRet)
			sndp_log_err(
				"voice_dev_chg_bt_to_audioic error(code=%d)\n",
				iRet);
	/* AudioLSI -> AudioLSI */
	} else if ((SNDP_BLUETOOTHSCO != old_dev) &&
		  (SNDP_BLUETOOTHSCO != new_dev)) {
		iRet = sndp_work_voice_dev_chg_in_audioic(
			wp->old_value, wp->new_value);
		if (ERROR_NONE != iRet)
			sndp_log_err(
				"voice_dev_chg_in_audioic error(code=%d)\n",
				iRet);
	/* BT(SCO) -> BT(SCO) */
	} else {
		/* Without processing */
	}

	/* Wake Unlock */
	sndp_wake_lock(E_UNLOCK);

	sndp_log_debug_func("end\n");
}


/*!
   @brief Device change AudioLSI -> BT-SCO (IN_CALL)
	  Subfunction of the sndp_work_voice_dev_chg()

   @param[in]	old_value	last PCM value
   @param[in]	new_value	new PCM value
   @param[out]

   @retval	0		Successful
 */
static int sndp_work_voice_dev_chg_audioic_to_bt(
	const u_int old_value,
	const u_int new_value)
{
	int	iRet = ERROR_NONE;


	sndp_log_debug_func("start\n");

	/* Set to DISABLE the speaker amp */
	if (SNDP_SPEAKER & SNDP_GET_DEVICE_VAL(old_value)) {
		if (NULL != g_sndp_codec_info.set_speaker_amp) {
			iRet = g_sndp_codec_info.set_speaker_amp(
					g_sndp_codec_info.speaker_disable);
			if (ERROR_NONE != iRet)
				sndp_log_err(
					"speaker_amp DISABLE error(code=%d)\n",
					iRet);
		}
	}

	/* stop SCUW */
	scuw_stop();

	/* stop FSI */
	fsi_stop();

	/* stop CLKGEN */
	clkgen_stop();

	/* AudioLSI device all stop */
	if (NULL != g_sndp_codec_info.set_device) {
		iRet = g_sndp_codec_info.set_device(
				g_sndp_codec_info.dev_none,
				SNDP_VALUE_INIT,
				g_sndp_codec_info.power_on);
		if (ERROR_NONE != iRet)
			sndp_log_err("set device error (code=%d)\n",
				     iRet);
	}

	/* start SCUW */
	iRet = scuw_start(new_value);
	if (ERROR_NONE != iRet)
		sndp_log_err("scuw start error(code=%d)\n", iRet);

	/* start FSI */
	iRet = fsi_start(new_value);
	if (ERROR_NONE != iRet)
		sndp_log_err("fsi start error(code=%d)\n", iRet);

	/* start CLKGEN */
	iRet = clkgen_start(new_value, 0);
	if (ERROR_NONE != iRet)
		sndp_log_err("clkgen start error(code=%d)\n", iRet);

	sndp_log_debug_func("end\n");

	return ERROR_NONE;
}


/*!
   @brief Device change BT-SCO -> AudioLSI (IN_CALL)
	  Subfunction of the sndp_work_voice_dev_chg()

   @param[in]	old_value	last PCM value
   @param[in]	new_value	new PCM value
   @param[out]

   @retval	0		Successful
 */
static int sndp_work_voice_dev_chg_bt_to_audioic(
	const u_int old_value,
	const u_int new_value)
{
	int	iRet = ERROR_NONE;
	u_long	ulSetDevice = g_sndp_codec_info.dev_none;


	sndp_log_debug_func("start\n");

	/* stop SCUW */
	scuw_stop();

	/* stop FSI */
	fsi_stop();

	/* stop CLKGEN */
	clkgen_stop();

	/* start SCUW */
	iRet = scuw_start(new_value);
	if (ERROR_NONE != iRet)
		sndp_log_err("scuw start error(code=%d)\n", iRet);

	/* start FSI */
	iRet = fsi_start(new_value);
	if (ERROR_NONE != iRet)
		sndp_log_err("fsi start error(code=%d)\n", iRet);

	/* start CLKGEN */
	iRet = clkgen_start(new_value, 0);
	if (ERROR_NONE != iRet)
		sndp_log_err("clkgen start error(code=%d)\n", iRet);

	/* device setting */
	ulSetDevice = sndp_get_next_devices(new_value);
	if (NULL != g_sndp_codec_info.set_device) {
		iRet = g_sndp_codec_info.set_device(
				ulSetDevice,
				new_value,
				g_sndp_codec_info.power_on);
		if (ERROR_NONE != iRet)
			sndp_log_err("set device error (code=%d)\n",
				     iRet);
	}

	/* Set to ENABLE the speaker amp */
	if (SNDP_SPEAKER & SNDP_GET_DEVICE_VAL(new_value)) {
		if (NULL != g_sndp_codec_info.set_speaker_amp) {
			iRet = g_sndp_codec_info.set_speaker_amp(
					g_sndp_codec_info.speaker_enable);
			if (ERROR_NONE != iRet)
				sndp_log_err(
					"speaker_amp ENABLE error(code=%d)\n",
					iRet);
		}
	}

	sndp_log_debug_func("end\n");

	return ERROR_NONE;
}


/*!
   @brief Device change AudioLSI -> AudioLSI (IN_CALL)
	  Subfunction of the sndp_work_voice_dev_chg()

   @param[in]	old_value	last PCM value
   @param[in]	new_value	new PCM value
   @param[out]

   @retval	0		Successful
 */
static int sndp_work_voice_dev_chg_in_audioic(
	const u_int old_value,
	const u_int new_value)
{
	int	iRet = ERROR_NONE;
	u_long	ulSetDevice = g_sndp_codec_info.dev_none;


	sndp_log_debug_func("start\n");

	/* device change */
	if (new_value != old_value) {
		ulSetDevice = sndp_get_next_devices(new_value);
		if (NULL != g_sndp_codec_info.set_device) {
			iRet = g_sndp_codec_info.set_device(
					ulSetDevice,
					new_value,
					g_sndp_codec_info.power_on);
			if (ERROR_NONE != iRet)
				sndp_log_err("set device error (code=%d)\n",
					     iRet);
		}
	}

	/* Set to ENABLE/DISABLE the speaker amp */
	if (NULL != g_sndp_codec_info.set_speaker_amp) {
		if (SNDP_SPEAKER & SNDP_GET_DEVICE_VAL(new_value))
			iRet = g_sndp_codec_info.set_speaker_amp(
					g_sndp_codec_info.speaker_enable);
		else
			iRet = g_sndp_codec_info.set_speaker_amp(
					g_sndp_codec_info.speaker_disable);

		if (ERROR_NONE != iRet)
			sndp_log_err(
			"set_speaker_amp %s error(code=%d)\n",
			(SNDP_SPEAKER & SNDP_GET_DEVICE_VAL(new_value)) ?
			"ENABLE" : "DISABLE", iRet);
	}
	sndp_log_debug_func("end\n");

	return ERROR_NONE;
}


/*!
   @brief Work queue function for SoundPath Setting(Playback)

   @param[in]	work	work queue structure
   @param[out]	none

   @retval	none
 */
static void sndp_work_play_start(struct work_struct *work)
{
	sndp_log_debug_func("start\n");

	/* Running Playback */
	g_sndp_playrec_flg |= E_PLAY;

	/* To register a work queue to start processing Playback */
	sndp_work_start(SNDP_PCM_OUT);

	sndp_log_debug_func("end\n");
}


/*!
   @brief Work queue function for SoundPath Setting(Capture)

   @param[in]	work	work queue structure
   @param[out]	none

   @retval	none
 */
static void sndp_work_capture_start(struct work_struct *work)
{
	sndp_log_debug_func("start\n");

	/* Running Capture */
	g_sndp_playrec_flg |= E_CAP;

	/* To register a work queue to start processing Capture */
	sndp_work_start(SNDP_PCM_IN);

	sndp_log_debug_func("end\n");
}


/*!
   @brief Work queue function for SoundPath Stop(Playback)

   @param[in]	work	work queue structure
   @param[out]	none

   @retval	none
 */
static void sndp_work_play_stop(struct work_struct *work)
{
	sndp_log_debug_func("start\n");

	/* Stop Playback runnning */
	g_sndp_playrec_flg &= ~E_PLAY;

	/* To register a work queue to stop processing Playback */
	sndp_work_stop(work, SNDP_PCM_OUT);

	/* Reset a Trigger stop status flag */
	g_sndp_stop_trigger_condition[SNDP_PCM_OUT] &=
					~SNDP_STOP_TRIGGER_PLAYBACK;

	/* Wake up main process */
	wake_up_interruptible(&g_sndp_stop_wait);

	sndp_log_debug_func("end\n");
}


/*!
   @brief Work queue function for SoundPath Stop(Capture)

   @param[in]	work	work queue structure
   @param[out]	none

   @retval	none
 */
static void sndp_work_capture_stop(struct work_struct *work)
{
	sndp_log_debug_func("start\n");

	/* Stop Capture running */
	g_sndp_playrec_flg &= ~E_CAP;

	/* To register a work queue to stop processing Capture */
	sndp_work_stop(work, SNDP_PCM_IN);

	/* Reset a Trigger stop status flag */
	g_sndp_stop_trigger_condition[SNDP_PCM_IN] &=
					~SNDP_STOP_TRIGGER_CAPTURE;

	/* Wake up main process */
	wake_up_interruptible(&g_sndp_stop_wait);

	sndp_log_debug_func("end\n");
}


/*!
   @brief Work queue function for SoundPath Setting(Playback incommunication)

   @param[in]	work	work queue structure
   @param[out]	none

   @retval	none
 */
static void sndp_work_play_incomm_start(struct work_struct *work)
{
	int			ret = ERROR_NONE;
	u_long			set_device = g_sndp_codec_info.dev_none;
	struct sndp_work_info	*wp = NULL;

	sndp_log_debug_func("start\n");
	/* Standby restraint */

	/* Running Playback */
	g_sndp_incomm_playrec_flg |= E_PLAY;

	/* To get a work queue structure */
	wp = container_of((void *)work, struct sndp_work_info, work);

	/* device change */
	set_device = sndp_get_next_devices(wp->new_value);
	if (NULL != g_sndp_codec_info.set_device) {
		ret = g_sndp_codec_info.set_device(
				set_device,
				wp->new_value,
				g_sndp_codec_info.power_on);
		if (ERROR_NONE != ret)
			sndp_log_err("set device error (code=%d)\n", ret);
	}

	if (!(E_CAP & g_sndp_incomm_playrec_flg)) {
		/* To register a work queue to start processing Playback */
		sndp_work_incomm_start(wp->new_value);
	} 

	/* Set to ENABLE the speaker amp */
	if (SNDP_SPEAKER & SNDP_GET_DEVICE_VAL(wp->new_value)) {
		if (NULL != g_sndp_codec_info.set_speaker_amp) {
			ret = g_sndp_codec_info.set_speaker_amp(
				g_sndp_codec_info.speaker_enable);
			if (ERROR_NONE != ret) {
				sndp_log_err(
					"speaker_amp ENABLE error(code=%d)\n",
					ret);
			}
		}
	}

	/* Wake Unlock */
	sndp_wake_lock(E_UNLOCK);

	sndp_log_debug_func("end\n");
}


/*!
   @brief Work queue function for SoundPath Stop(Playback incommunication)

   @param[in]	work	work queue structure
   @param[out]	none

   @retval	none
 */
static void sndp_work_play_incomm_stop(struct work_struct *work)
{
	int			ret = ERROR_NONE;
	u_long			set_device = g_sndp_codec_info.dev_none;
	struct sndp_work_info	*wp = NULL;

	sndp_log_debug_func("start\n");

	/* Stop Playback runnning */
	g_sndp_incomm_playrec_flg &= ~E_PLAY;

	if (!g_sndp_incomm_playrec_flg) {
		wp = container_of((void *)work, struct sndp_work_info, work);

		/* To register a work queue to stop processing Playback */
		sndp_work_incomm_stop();
	}

	if (NULL != g_sndp_codec_info.get_device) {
		ret = g_sndp_codec_info.get_device(&set_device);
		if (ERROR_NONE != ret)
			sndp_log_err("get_device error(code=%d)\n", ret);
	}
	
	/* Init to Playback side */
	set_device &= ~g_sndp_codec_info.out_dev_all;

	/* set Device */
	if (NULL != g_sndp_codec_info.set_device) {
		ret = g_sndp_codec_info.set_device(
				set_device,
				SNDP_VALUE_INIT,
				g_sndp_codec_info.power_on);
		if (ERROR_NONE != ret)
			sndp_log_err("set_device error (code=%d)\n", ret);
	}

	/* Reset a Trigger stop status flag */
	g_sndp_stop_trigger_condition[SNDP_PCM_OUT] &=
					~SNDP_STOP_TRIGGER_PLAYBACK;

	/* Wake up main process */
	wake_up_interruptible(&g_sndp_stop_wait);

	sndp_log_debug_func("end\n");
}


/*!
   @brief Work queue function for SoundPath Setting(Capture incommunication)

   @param[in]	work	work queue structure
   @param[out]	none

   @retval	none
 */
static void sndp_work_capture_incomm_start(struct work_struct *work)
{
	int			ret = ERROR_NONE;
	u_long			set_device = g_sndp_codec_info.dev_none;
	struct sndp_work_info	*wp = NULL;

	sndp_log_debug_func("start\n");

	/* Running Capture */
	g_sndp_incomm_playrec_flg |= E_CAP;

	/* To get a work queue structure */
	wp = container_of((void *)work, struct sndp_work_info, work);

	/* device change */
	set_device = sndp_get_next_devices(wp->new_value);
	if (NULL != g_sndp_codec_info.set_device) {
		ret = g_sndp_codec_info.set_device(
				set_device,
				wp->new_value,
				g_sndp_codec_info.power_on);
		if (ERROR_NONE != ret)
			sndp_log_err("set device error (code=%d)\n", ret);
	}

	if (!(E_PLAY & g_sndp_incomm_playrec_flg)) {
		/* To register a work queue to start processing Capture */
		sndp_work_incomm_start(wp->new_value);
	}

	/* Wake Unlock */
	sndp_wake_lock(E_UNLOCK);

	sndp_log_debug_func("end\n");
}


/*!
   @brief Work queue function for SoundPath Stop(Capture incommunication)

   @param[in]	work	work queue structure
   @param[out]	none

   @retval	none
 */
static void sndp_work_capture_incomm_stop(struct work_struct *work)
{
	int			ret = ERROR_NONE;
	u_long			set_device = g_sndp_codec_info.dev_none;
	struct sndp_work_info	*wp = NULL;

	sndp_log_debug_func("start\n");

	/* Stop Capture running */
	g_sndp_incomm_playrec_flg &= ~E_CAP;

	if (!g_sndp_incomm_playrec_flg) {
		wp = container_of((void *)work, struct sndp_work_info, work);

		/* To register a work queue to stop processing Playback */
		sndp_work_incomm_stop();
	}

	if (NULL != g_sndp_codec_info.get_device) {
		ret = g_sndp_codec_info.get_device(&set_device);
		if (ERROR_NONE != ret)
			sndp_log_err("get_device error(code=%d)\n", ret);
	}
	
	/* Init to Capture side */
	set_device &= ~g_sndp_codec_info.in_dev_all;

	/* set Device */
	if (NULL != g_sndp_codec_info.set_device) {
		ret = g_sndp_codec_info.set_device(
				set_device,
				SNDP_VALUE_INIT,
				g_sndp_codec_info.power_on);
		if (ERROR_NONE != ret)
			sndp_log_err("set_device error (code=%d)\n", ret);
	}


	/* Reset a Trigger stop status flag */
	g_sndp_stop_trigger_condition[SNDP_PCM_IN] &=
					~SNDP_STOP_TRIGGER_CAPTURE;

	/* Wake up main process */
	wake_up_interruptible(&g_sndp_stop_wait);

	sndp_log_debug_func("end\n");
}


/*!
   @brief Work queue function for Incommunication Start

   @param[in]	work	work queue structure
   @param[out]	none

   @retval	none
 */
static void sndp_work_incomm_start(const u_int new_value)
{
	int	ret = ERROR_NONE;

	sndp_log_debug_func("start\n");

	/* FSI master for ES 2.0 over */
	if ((system_rev & 0xffff) >= 0x3E10)
		common_set_fsi2cr(STAT_OFF);

	/* start SCUW */
	ret = scuw_start(new_value);
	if (ERROR_NONE != ret) {
		sndp_log_err("scuw start error(code=%d)\n", ret);
		goto start_err;
	}

	/* start FSI */
	ret = fsi_start(new_value);
	if (ERROR_NONE != ret) {
		sndp_log_err("fsi start error(code=%d)\n", ret);
		goto start_err;
	}

	wait_event_interruptible_timeout(
		g_watch_start_clk_queue, atomic_read(&g_sndp_watch_start_clk),
		msecs_to_jiffies(SNDP_WATCH_CLK_TIME_OUT));
	atomic_set(&g_sndp_watch_start_clk, 0);

	/* start CLKGEN */
	ret = clkgen_start(new_value, 0);
	if (ERROR_NONE != ret) {
		sndp_log_err("clkgen start error(code=%d)\n", ret);
		goto start_err;
	}

start_err:
	g_sndp_start_call_wait = 1;
	wake_up_interruptible(&g_sndp_start_call_queue);

	sndp_log_debug_func("end\n");
}


/*!
   @brief Work queue function for Incommunication Stop

   @param[in]	none
   @param[out]	none

   @retval	none
 */
static void sndp_work_incomm_stop(void)
{
	int	ret = ERROR_NONE;

	sndp_log_debug_func("start\n");

	wait_event_interruptible_timeout(
		g_watch_stop_clk_queue, atomic_read(&g_sndp_watch_stop_clk),
		msecs_to_jiffies(SNDP_WATCH_CLK_TIME_OUT));
	atomic_set(&g_sndp_watch_stop_clk, 0);

	/* stop SCUW */
	scuw_stop();

	/* stop FSI */
	fsi_stop();

	/* stop CLKGEN */
	clkgen_stop();

	/* Disable the power domain */
	ret = pm_runtime_put_sync(g_sndp_power_domain);
	if (ERROR_NONE != ret)
		sndp_log_debug("modules power off iRet=%d\n", ret);

	/* FSI master for ES 2.0 over */
	if ((system_rev & 0xffff) >= 0x3E10)
		common_set_fsi2cr(STAT_ON);

	/* Wake Force Unlock */
	sndp_wake_lock(E_FORCE_UNLOCK);

	sndp_log_debug_func("end\n");
}


/*!
   @brief Switching path of the sound during a Call + Playback

   @param[in]	none
   @param[out]	none

   @retval	none
 */
void sndp_call_playback_normal(void)
{
	sndp_log_debug_func("start\n");

	if (!(SNDP_ROUTE_PLAY_CHANGED & g_sndp_stream_route)) {
		sndp_path_switching(GET_OLD_VALUE(SNDP_PCM_OUT));
		g_sndp_stream_route |= SNDP_ROUTE_PLAY_CHANGED;
	}

	sndp_log_debug_func("end\n");
}

/*!
   @brief Work queue function for Start during a call playback

   @param[in]	work	work queue structure
   @param[out]	none

   @retval	none
 */
static void sndp_work_call_playback_start(struct work_struct *work)
{
	int			iRet = ERROR_NONE;
	struct sndp_work_info	*wp = NULL;


	sndp_log_debug_func("start\n");

	/* To get a work queue structure */
	wp = container_of((void *)work, struct sndp_work_info, work);

	/* Call + Playback start request */
	iRet = call_playback_start(wp->save_substream);

	if (ERROR_NONE != iRet)
		/* Switching path of the sound during a Call + Playback */
		sndp_call_playback_normal();

	sndp_log_debug_func("end\n");
}


/*!
   @brief Work queue function for Start during a call capture

   @param[in]	work	work queue structure
   @param[out]	none

   @retval	none
 */
static void sndp_work_call_capture_start(struct work_struct *work)
{
	int			iRet = ERROR_NONE;
	struct sndp_work_info	*wp = NULL;


	sndp_log_debug_func("start\n");

	/* To get a work queue structure */
	wp = container_of((void *)work, struct sndp_work_info, work);

	/* Call + Capture start request */
	iRet = call_record_start(wp->save_substream);

	if (ERROR_NONE != iRet) {
		/* Dummy capture start */
		if (!(SNDP_ROUTE_CAP_DUMMY & g_sndp_stream_route)) {
			call_change_dummy_rec();
			g_sndp_stream_route |= SNDP_ROUTE_CAP_DUMMY;
		}
	}

	sndp_log_debug_func("end\n");
}


/*!
   @brief Work queue function for Stop during a call playback

   @param[in]	work	work queue structure
   @param[out]	none

   @retval	none
 */
static void sndp_work_call_playback_stop(struct work_struct *work)
{
	struct sndp_work_info	*wp = NULL;


	sndp_log_debug_func("start\n");

	/* To get a work queue structure */
	wp = container_of((void *)work, struct sndp_work_info, work);

	/* Call + Playback stop request */
	call_playback_stop();

	/* Back-out path of the sound during a Call + Playback */
	if (SNDP_ROUTE_PLAY_CHANGED & g_sndp_stream_route) {
		sndp_path_backout(GET_OLD_VALUE(SNDP_PCM_OUT));
		g_sndp_stream_route &= ~SNDP_ROUTE_PLAY_CHANGED;
	}

	g_call_playback_stop = false;

	sndp_wake_lock(E_UNLOCK);

	/* Reset a Trigger stop status flag */
	g_sndp_stop_trigger_condition[SNDP_PCM_OUT] &=
					~SNDP_STOP_TRIGGER_PLAYBACK;

	/* Wake up main process */
	wake_up_interruptible(&g_sndp_stop_wait);

	sndp_log_debug_func("end\n");
}


/*!
   @brief Work queue function for Stop during a call capture

   @param[in]	work	work queue structure
   @param[out]	none

   @retval	none
 */
static void sndp_work_call_capture_stop(struct work_struct *work)
{
	u_int in_old_val = GET_OLD_VALUE(SNDP_PCM_IN);
	u_int out_old_val = GET_OLD_VALUE(SNDP_PCM_OUT);


	sndp_log_debug_func("start\n");

	if (SNDP_ROUTE_CAP_DUMMY & g_sndp_stream_route) {
		/* Dummy capture stop */
		g_sndp_stream_route &= ~SNDP_ROUTE_CAP_DUMMY;
	}

	/* Call + Capture stop request */
	call_record_stop();

	sndp_wake_lock(E_UNLOCK);

	/* Reset a Trigger stop status flag */
	g_sndp_stop_trigger_condition[SNDP_PCM_IN] &=
					~SNDP_STOP_TRIGGER_CAPTURE;

	/* If the state already NORMAL Playback side */
	if ((!(SNDP_ROUTE_PLAY_CHANGED & g_sndp_stream_route)) &&
	    (SNDP_MODE_INCALL == SNDP_GET_MODE_VAL(in_old_val)) &&
	    (SNDP_MODE_INCALL != SNDP_GET_MODE_VAL(out_old_val))) {

		/*
		 * Voice stop and Normal device change
		 * (Post-processing of this function)
		 */
		sndp_after_of_work_call_capture_stop(in_old_val, out_old_val);

	} else {
		/* Reset a Trigger stop status flag */
		g_sndp_stop_trigger_condition[SNDP_PCM_IN] &=
						~SNDP_STOP_TRIGGER_VOICE;

		/* Wake up main process */
		wake_up_interruptible(&g_sndp_stop_wait);
	}

	sndp_log_debug_func("end\n");
}


/*!
   @brief Work queue function for Start during a fm playback

   @param[in]	work	work queue structure
   @param[out]	none

   @retval	none
 */
static void sndp_work_fm_playback_start(struct work_struct *work)
{
	sndp_log_debug_func("start\n");

	/* Running Playback */
	g_sndp_playrec_flg |= E_PLAY;

	/* To register a work queue to start processing Playback */
	sndp_fm_work_start(SNDP_PCM_OUT);

	sndp_log_debug_func("end\n");
}


/*!
   @brief Work queue function for Start during a fm capture

   @param[in]	work	work queue structure
   @param[out]	none

   @retval	none
 */
static void sndp_work_fm_capture_start(struct work_struct *work)
{

	sndp_log_debug_func("start\n");

	/* Running Capture */
	g_sndp_playrec_flg |= E_CAP;

	/* To register a work queue to start processing Capture */
	sndp_fm_work_start(SNDP_PCM_IN);

	sndp_log_debug_func("end\n");

}


/*!
   @brief Work queue function for Stop during a fm playback

   @param[in]	work	work queue structure
   @param[out]	none

   @retval	none
 */
static void sndp_work_fm_playback_stop(struct work_struct *work)
{
	sndp_log_debug_func("start\n");

	/* Stop Playback runnning */
	g_sndp_playrec_flg &= ~E_PLAY;

	/* To register a work queue to stop processing Playback */
	sndp_fm_work_stop(work, SNDP_PCM_OUT);

	/* Reset a Trigger stop status flag */
	g_sndp_stop_trigger_condition[SNDP_PCM_OUT] &=
					~SNDP_STOP_TRIGGER_PLAYBACK;

	/* Wake up main process */
	wake_up_interruptible(&g_sndp_stop_wait);

	sndp_log_debug_func("end\n");
}


/*!
   @brief Work queue function for Stop during a fm capture

   @param[in]	work	work queue structure
   @param[out]	none

   @retval	none
 */
static void sndp_work_fm_capture_stop(struct work_struct *work)
{
	sndp_log_debug_func("start\n");

	/* Stop Capture running */
	g_sndp_playrec_flg &= ~E_CAP;

	/* To register a work queue to stop processing Capture */
	sndp_fm_work_stop(work, SNDP_PCM_IN);

	/* Reset a Trigger stop status flag */
	g_sndp_stop_trigger_condition[SNDP_PCM_IN] &=
					~SNDP_STOP_TRIGGER_CAPTURE;

	/* Wake up main process */
	wake_up_interruptible(&g_sndp_stop_wait);

	sndp_log_debug_func("end\n");
}


/*!
   @brief Work queue function for Start during a playback incommunication

   @param[in]	work	work queue structure
   @param[out]	none

   @retval	none
 */
static void sndp_work_call_playback_incomm_start(struct work_struct *work)
{
	struct sndp_work_info	*wp = NULL;


	sndp_log_debug_func("start\n");

	/* To get a work queue structure */
	wp = container_of((void *)work, struct sndp_work_info, work);

	/*  Playback incommunication start request */
	call_playback_incomm_start(wp->save_substream);

	sndp_log_debug_func("end\n");
}


/*!
   @brief Work queue function for Start during a capture incommunication

   @param[in]	work	work queue structure
   @param[out]	none

   @retval	none
 */
static void sndp_work_call_capture_incomm_start(struct work_struct *work)
{
	struct sndp_work_info	*wp = NULL;


	sndp_log_debug_func("start\n");

	/* To get a work queue structure */
	wp = container_of((void *)work, struct sndp_work_info, work);

	/* Capture incommunication start request */
	call_record_incomm_start(wp->save_substream);

	sndp_log_debug_func("end\n");
}


/*!
   @brief Work queue function for Stop during a playback incommunication

   @param[in]	work	work queue structure
   @param[out]	none

   @retval	none
 */
static void sndp_work_call_playback_incomm_stop(struct work_struct *work)
{
	struct sndp_work_info	*wp = NULL;


	sndp_log_debug_func("start\n");

	/* To get a work queue structure */
	wp = container_of((void *)work, struct sndp_work_info, work);

	/* Playback incommunication stop request */
	call_playback_incomm_stop();

	/* Reset a Trigger stop status flag */
	g_sndp_stop_trigger_condition[SNDP_PCM_OUT] &=
					~SNDP_STOP_TRIGGER_PLAYBACK;

	/* Wake up main process */
	wake_up_interruptible(&g_sndp_stop_wait);

	sndp_log_debug_func("end\n");
}


/*!
   @brief Work queue function for Stop during a capture incommunication

   @param[in]	work	work queue structure
   @param[out]	none

   @retval	none
 */
static void sndp_work_call_capture_incomm_stop(struct work_struct *work)
{
	sndp_log_debug_func("start\n");


	/* Capture incommunication stop request */
	call_record_incomm_stop();

	/* Reset a Trigger stop status flag */
	g_sndp_stop_trigger_condition[SNDP_PCM_IN] &=
					~SNDP_STOP_TRIGGER_CAPTURE;

	/* Wake up main process */
	wake_up_interruptible(&g_sndp_stop_wait);

	sndp_log_debug_func("end\n");
}


/*!
   @brief VCD_COMMAND_WATCH_STOP_FW registration process

   @param[in]	none
   @param[out]	none

   @retval	none
 */
static void sndp_regist_watch(void)
{
	int	iRet = ERROR_NONE;
	struct call_vcd_callback	func;


	sndp_log_debug_func("start\n");

	/* Standby restraint */
	func.callback_start_fw = sndp_watch_start_fw_cb;
	func.callback_stop_fw = sndp_watch_stop_fw_cb;
	func.callback_start_clk = sndp_watch_start_clk_cb;
	func.callback_stop_clk = sndp_watch_stop_clk_cb;
	func.callback_wait_path = sndp_wait_path_cb;

	iRet = call_regist_watch(&func);
	if (ERROR_NONE  != iRet)
		sndp_log_err("VCD watch command set error(code=%d)\n", iRet);

	sndp_log_debug_func("end\n");
}


/*!
   @brief Work queue processing for VCD_COMMAND_WATCH_STOP_FW process

   @param[in]	work	work queue structure
   @param[out]	none

   @retval	none
 */
static void sndp_work_watch_stop_fw(struct work_struct *work)
{
	sndp_log_debug_func("start\n");

	/* During a Call + Capture */
	if (SNDP_STAT_IN_CALL_CAP == GET_SNDP_STATUS(SNDP_PCM_IN)) {
		/* Dummy capture start */
		if (!(SNDP_ROUTE_CAP_DUMMY & g_sndp_stream_route)) {
			call_change_dummy_rec();
			g_sndp_stream_route |= SNDP_ROUTE_CAP_DUMMY;
		}
	}

	/* During a Call + Playback */
	if (SNDP_STAT_IN_CALL_PLAY == GET_SNDP_STATUS(SNDP_PCM_OUT)) {
		/* Switching path of the sound during a Call + Playback */
		if (!(SNDP_ROUTE_PLAY_CHANGED & g_sndp_stream_route)) {
			call_change_dummy_play();
		}
	}

	sndp_log_debug_func("end\n");
}


/*!
   @brief Watch start Firmware notification callback function

   @param[in]	none
   @param[out]	none

   @retval	none
 */
static void sndp_watch_start_fw_cb(void)
{
	sndp_log_debug_func("start\n");
	sndp_log_debug("FW was started <incall or incommunication>\n");

	atomic_set(&g_call_watch_start_fw, 1);

	sndp_log_debug_func("end\n");
}


/*!
   @brief Wait set path notification callback function

   @param[in]	none
   @param[out]	none

   @retval	none
 */
static void sndp_wait_path_cb(void)
{
	sndp_log_debug_func("start\n");

	/*
	 * Registered in the work queue for
	 * VCD_COMMAND_WAIT_PATH process
	 */
	wait_event_interruptible(g_sndp_start_call_queue,
				g_sndp_start_call_wait);

	g_sndp_start_call_wait = 0;

	sndp_log_debug_func("end\n");
}


/*!
   @brief Watch stop Firmware notification callback function

   @param[in]	none
   @param[out]	none

   @retval	none
 */
static void sndp_watch_stop_fw_cb(void)
{
	sndp_log_debug_func("start\n");
	sndp_log_debug("VCD is close <incall or incommunication>\n");

	/*
	 * Registered in the work queue for
	 * VCD_COMMAND_WATCH_STOP_FW process
	 */
	queue_work(g_sndp_queue_main, &g_sndp_work_watch_stop_fw.work);

	sndp_log_debug_func("end\n");
}


/*!
   @brief Work queue function for FM Radio start

   @param[in]	work	work queue structure
   @param[out]	none

   @retval	none
 */
static void sndp_work_fm_radio_start(struct work_struct *work)
{
	int			iRet = ERROR_NONE;
	u_long			ulSetDevice = g_sndp_codec_info.dev_none;
	struct sndp_work_info	*wp = NULL;


	sndp_log_debug_func("start\n");

	/* To get a work queue structure */
	wp = container_of((void *)work, struct sndp_work_info, work);

	/* set device */
	ulSetDevice = sndp_get_next_devices(wp->new_value);
	if (NULL != g_sndp_codec_info.set_device) {
		iRet = g_sndp_codec_info.set_device(
				ulSetDevice,
				wp->new_value,
				g_sndp_codec_info.power_on);
		if (ERROR_NONE != iRet)
			sndp_log_err("set device error (code=%d)\n", iRet);
	}

	/* Enable the power domain */
	if ((E_PLAY | E_CAP) != g_sndp_playrec_flg) {
		iRet = pm_runtime_get_sync(g_sndp_power_domain);
		/* 0:success 1:active */
		if (!(0 == iRet || 1 == iRet)) {
			sndp_log_err("module power on err[iRet=%d]\n", iRet);
		} else {
			/* CPG soft reset */
			fsi_soft_reset();
		}

		/* FSI master for ES 2.0 over */
		if ((system_rev & 0xffff) >= 0x3E10)
			common_set_pll22(wp->new_value, STAT_ON);
	}


	/* start SCUW */
	iRet = scuw_start(wp->new_value);
	if (ERROR_NONE != iRet) {
		sndp_log_err("scuw start error(code=%d)\n", iRet);
		goto start_err;
	}

	/* start FSI */
	iRet = fsi_start(wp->new_value);
	if (ERROR_NONE != iRet) {
		sndp_log_err("fsi start error(code=%d)\n", iRet);
		goto start_err;
	}

	/* start CLKGEN */
	iRet = clkgen_start(wp->new_value, SNDP_NORMAL_RATE);
	if (ERROR_NONE != iRet) {
		sndp_log_err("clkgen start error(code=%d)\n", iRet);
		goto start_err;
	}

	/* Set to ENABLE the speaker amp */
	if (g_sndp_codec_info.dev_playback_speaker & ulSetDevice) {
		if (NULL != g_sndp_codec_info.set_speaker_amp) {
			iRet = g_sndp_codec_info.set_speaker_amp(
					g_sndp_codec_info.speaker_enable);
			if (ERROR_NONE != iRet) {
				sndp_log_err(
					"speaker_amp ENABLE error(code=%d)\n",
					iRet);
				goto start_err;
			}
		}
	}

start_err:
	/* Wake Unlock */
	sndp_wake_lock(E_UNLOCK);

	sndp_log_debug_func("end\n");
}


/*!
   @brief Work queue function for FM Radio stop

   @param[in]	work	work queue structure
   @param[out]	none

   @retval	none
 */
static void sndp_work_fm_radio_stop(struct work_struct *work)
{
	int			iRet = ERROR_NONE;
	u_long			ulSetDevice = g_sndp_codec_info.dev_none;
	struct sndp_work_info	*wp = NULL;


	sndp_log_debug_func("start\n");

	/* To get a work queue structure */
	wp = container_of((void *)work, struct sndp_work_info, work);

	/* Set to DISABLE the speaker amp */
	if (NULL != g_sndp_codec_info.get_device) {
		iRet = g_sndp_codec_info.get_device(&ulSetDevice);
		if (ERROR_NONE != iRet)
			sndp_log_err("get_device error(code=%d)\n",
				     iRet);
	}

	if (g_sndp_codec_info.dev_playback_speaker & ulSetDevice) {
		if (NULL != g_sndp_codec_info.set_speaker_amp) {
			iRet = g_sndp_codec_info.set_speaker_amp(
					g_sndp_codec_info.speaker_disable);
			if (ERROR_NONE != iRet)
				sndp_log_err(
					"speaker_amp DISABLE error(code=%d)\n",
					iRet);
		}
	}

	/* Init to Capture side */
	ulSetDevice &= ~g_sndp_codec_info.in_dev_all;

	/* set Device */
	if (NULL != g_sndp_codec_info.set_device) {
		iRet = g_sndp_codec_info.set_device(
				ulSetDevice,
				SNDP_VALUE_INIT,
				g_sndp_codec_info.power_on);
		if (ERROR_NONE != iRet)
			sndp_log_err("set_device error (code=%d)\n",
				     iRet);
	}

	/* Disable the power domain */
	if (!g_sndp_playrec_flg) {
		/* stop SCUW */
		scuw_stop();
		/* stop FSI */
		fsi_stop();
		/* stop CLKGEN */
		clkgen_stop();

		/* FSI master for ES 2.0 over */
		if ((system_rev & 0xffff) >= 0x3E10)
			common_set_pll22(GET_OLD_VALUE(SNDP_PCM_IN), STAT_OFF);

		pm_runtime_put_sync(g_sndp_power_domain);
	}

	/* Wake Force Unlock */
	sndp_wake_lock((g_sndp_playrec_flg) ? E_UNLOCK : E_FORCE_UNLOCK);

	sndp_log_debug_func("end\n");
}


/*!
   @brief wake up start clkgen callback function

   @param[in]	none
   @param[out]	none

   @retval	none
 */
static void sndp_watch_start_clk_cb(void)
{
	sndp_log_debug_func("start\n");

	atomic_set(&g_sndp_watch_start_clk, 1);
	wake_up_interruptible(&g_watch_start_clk_queue);

	sndp_log_debug_func("end\n");
}


/*!
   @brief wake up stop clkgen callback function

   @param[in]	none
   @param[out]	none

   @retval	none
 */
static void sndp_watch_stop_clk_cb(void)
{
	sndp_log_debug_func("start\n");

	atomic_set(&g_sndp_watch_stop_clk, 1);
	wake_up_interruptible(&g_watch_stop_clk_queue);

	sndp_log_debug_func("end\n");
}


/*!
   @brief Switching path of the sound during a Call + Playback

   @param[in]	uiValue		PCM value
   @param[out]	none

   @retval	none
 */
static void sndp_path_switching(const u_int uiValue)
{
	sndp_log_debug_func("start uiValue[0x%08X]\n", uiValue);

	/* stop SCUW */
	scuw_stop();

	/* stop FSI */
	fsi_stop();

	/* stop CLKGEN */
	clkgen_stop();

	/* for Register dump debug */
	/* g_sndp_now_direction = SNDP_PCM_OUT; */

	/* Running Playback */
	g_sndp_playrec_flg |= E_PLAY;

	/* To register a work queue to start processing Playback */
	sndp_work_start(SNDP_PCM_OUT);

	sndp_log_debug_func("end\n");
}


/*!
   @brief Back-out path of the sound during a Call + Playback

   @param[in]	uiValue		PCM value
   @param[out]	none

   @retval	none
 */
static void sndp_path_backout(const u_int uiValue)
{
	int			iRet = ERROR_NONE;
	struct sndp_stop	*stop;
	struct sndp_arg		*arg;


	sndp_log_debug_func("start uiValue[0x%08X]\n", uiValue);

	arg = &g_sndp_main[SNDP_PCM_OUT].arg;
	/* FSI trigger stop process */
	fsi_set_trigger_stop(arg->fsi_substream, false);

	/* Init register dump log flag for debug */
	/* g_sndp_now_direction = SNDP_PCM_DIRECTION_MAX; */

	/* A work queue processing to register TRIGGER_STOP */
	g_sndp_stop_trigger_condition[SNDP_PCM_OUT] |=
			SNDP_STOP_TRIGGER_PLAYBACK;

	stop = &g_sndp_work_play_stop.stop;

	stop->fsi_substream = *arg->fsi_substream;
	if (NULL == &stop->fsi_substream)
		sndp_log_debug_func("#### stop->fsi_substream is NULL.\n");

	stop->fsi_dai =	*arg->fsi_dai;
	if (NULL == &stop->fsi_dai)
		sndp_log_debug_func("#### stop->fsi_dai is NULL.\n");

	/* Stop Playback runnning */
	g_sndp_playrec_flg &= ~E_PLAY;

	/* To register a work queue to stop processing Playback */
	sndp_work_stop(&g_sndp_work_play_stop.work, SNDP_PCM_OUT);

	/* Reset a Trigger stop status flag */
	g_sndp_stop_trigger_condition[SNDP_PCM_OUT] &=
					~SNDP_STOP_TRIGGER_PLAYBACK;

	/* Wake up main process */
	wake_up_interruptible(&g_sndp_stop_wait);

	/* start SCUW */
	iRet = scuw_start(uiValue);
	if (ERROR_NONE != iRet)
		sndp_log_err("scuw start error(code=%d)\n", iRet);

	/* start FSI */
	iRet = fsi_start(uiValue);
	if (ERROR_NONE != iRet)
		sndp_log_err("fsi start error(code=%d)\n", iRet);

	/* start CLKGEN */
	iRet = clkgen_start(uiValue, 0);
	if (ERROR_NONE != iRet)
		sndp_log_err("clkgen start error(code=%d)\n", iRet);

	sndp_log_debug_func("end\n");
}


/*!
   @brief SoundPath Start and HardWare Parameter settings

   @param[in]	direction	SNDP_PCM_OUT/SNDP_PCM_IN
   @param[out]	none

   @retval	none
 */
static void sndp_work_start(const int direction)
{
	int	iRet = ERROR_NONE;
	u_long	ulSetDevice = g_sndp_codec_info.dev_none;
	u_int	uiValue;


	sndp_log_debug_func("start direction[%d]\n", direction);

	uiValue = GET_OLD_VALUE(direction);

	/* set device */
	/* (In the case of IN_CALL, the device has been set) */
	if (SNDP_MODE_INCALL != SNDP_GET_MODE_VAL(uiValue)) {
		ulSetDevice = sndp_get_next_devices(uiValue);
		sndp_log_debug("set_next_dev[%08x]\n", ulSetDevice);
		if (NULL != g_sndp_codec_info.set_device) {
			if ((SNDP_PCM_IN == direction) &&
			    (E_CAP == g_sndp_playrec_flg)) {
				if (SNDP_BUILTIN_MIC & ulSetDevice)
					ulSetDevice |=
					g_sndp_codec_info.dev_playback_speaker;
				else if (SNDP_WIREDHEADSET & ulSetDevice)
					ulSetDevice |=
					g_sndp_codec_info.dev_playback_headphones;
			}
			iRet = g_sndp_codec_info.set_device(
					ulSetDevice, uiValue,
					g_sndp_codec_info.power_on);
			if (ERROR_NONE != iRet) {
				sndp_log_err("set device error (code=%d)\n",
					iRet);
				return;
			}
		}
	}

	/* PM_RUNTIME */
	if ((E_PLAY | E_CAP) != g_sndp_playrec_flg) {
		iRet = pm_runtime_get_sync(g_sndp_power_domain);
		/* 0:success 1:active */
		if (!(0 == iRet || 1 == iRet)) {
			sndp_log_err("modules power on error(ret=%d)\n", iRet);
		} else {
			/* CPG soft reset */
			fsi_soft_reset();
		}
		/* FSI master for ES 2.0 over */
		if ((system_rev & 0xffff) >= 0x3E10)
			common_set_pll22(uiValue, STAT_ON);
	}

	/* FSI slave setting ON for switch */
	if (SNDP_MODE_INCALL == SNDP_GET_MODE_VAL(uiValue))
		fsi_set_slave(true);

	/* FSI startup */
	if (NULL != g_sndp_dai_func.fsi_startup) {
		sndp_log_debug("fsi_dai_startup\n");
		if (false == (SNDP_GET_DEVICE_VAL(uiValue) & SNDP_BLUETOOTHSCO)) {
			iRet = g_sndp_dai_func.fsi_startup(
				g_sndp_main[direction].arg.fsi_substream,
				g_sndp_main[direction].arg.fsi_dai);
		} else {
			iRet = fsi_dai_startup_bt(
				g_sndp_main[direction].arg.fsi_substream,
				g_sndp_main[direction].arg.fsi_dai);
		}
		if (ERROR_NONE != iRet) {
			sndp_log_err("fsi_dai_startup error(code=%d)\n", iRet);
			return;
		}
	}

	/* Set FSI format */
	if (NULL != g_sndp_dai_func.fsi_set_fmt) {
		sndp_log_debug("fsi_dai_set_fmt\n");
		iRet = g_sndp_dai_func.fsi_set_fmt(
					g_sndp_main[direction].arg.fsi_dai,
					g_sndp_fsi_format);
		if (ERROR_NONE != iRet) {
			sndp_log_err("fsi_dai_set_fmt error(code=%d)\n", iRet);
			return;
		}
	}

	/* Set FSI hardware parameters */
	if (NULL != g_sndp_dai_func.fsi_hw_params) {
		sndp_log_debug("fsi_dai_hw_params [params = %d]\n",
			params_rate(&g_sndp_main[direction].arg.fsi_params));
		g_sndp_dai_func.fsi_hw_params(
				g_sndp_main[direction].arg.fsi_substream,
				&g_sndp_main[direction].arg.fsi_params,
				g_sndp_main[direction].arg.fsi_dai);
	}

	/* Set to ENABLE the speaker amp */
	/* (In the case of IN_CALL, the amp has been set) */
	if (SNDP_MODE_INCALL != SNDP_GET_MODE_VAL(uiValue)) {
		if ((SNDP_PCM_OUT == direction) &&
		    (SNDP_SPEAKER & SNDP_GET_DEVICE_VAL(uiValue))) {
			if (NULL != g_sndp_codec_info.set_speaker_amp) {
				iRet = g_sndp_codec_info.set_speaker_amp(
					g_sndp_codec_info.speaker_enable);
				if (ERROR_NONE != iRet) {
					sndp_log_err(
					"speaker_amp ENABLE error(code=%d)\n",
					iRet);
					return;
				}
			}
		}
	}

	/* start CLKGEN */
	if (SNDP_MODE_INCALL != SNDP_GET_MODE_VAL(uiValue)) {
		iRet = clkgen_start(uiValue, SNDP_NORMAL_RATE);
	} else {
		/* set mode INCALL -> NORMAL */
		uiValue &= 0xFFFFFFFC;
		iRet = clkgen_start(uiValue, SNDP_CALL_RATE);
	}
	if (ERROR_NONE != iRet) {
		sndp_log_err("clkgen start error(code=%d)\n", iRet);
		return;
	}

	/* FSI Trigger start */
	if (NULL != g_sndp_dai_func.fsi_trigger) {
		sndp_log_debug("fsi_dai_trigger start\n");
		iRet = g_sndp_dai_func.fsi_trigger(
				g_sndp_main[direction].arg.fsi_substream,
				SNDRV_PCM_TRIGGER_START,
				g_sndp_main[direction].arg.fsi_dai);
		if (ERROR_NONE != iRet)
			sndp_log_err("fsi_trigger error(code=%d)\n", iRet);
	}

	sndp_log_debug_func("end\n");
}


/*!
   @brief SoundPath Stop

   @param[in]	work		work queue structure
   @param[in]	direction	SNDP_PCM_OUT/SNDP_PCM_IN
   @param[out]	none

   @retval	none
 */
static void sndp_work_stop(
	struct work_struct *work,
	const int direction)
{
	int			iRet = ERROR_NONE;
	u_long			ulSetDevice = g_sndp_codec_info.dev_none;
	struct sndp_work_info	*wp = NULL;
	u_int			uiValue;


	sndp_log_debug_func("start direction[%d]\n", direction);

	uiValue = GET_OLD_VALUE(direction);

	/* To get a work queue structure */
	wp = container_of((void *)work, struct sndp_work_info, work);

	/* Set to DISABLE the speaker amp */
	if (SNDP_MODE_INCALL != SNDP_GET_MODE_VAL(uiValue)) {
		if ((SNDP_PCM_OUT == direction) &&
		    (SNDP_SPEAKER & SNDP_GET_DEVICE_VAL(uiValue))) {
			if (NULL != g_sndp_codec_info.set_speaker_amp) {
				iRet = g_sndp_codec_info.set_speaker_amp(
					g_sndp_codec_info.speaker_disable);
				if (ERROR_NONE != iRet)
					sndp_log_err(
					"speaker_amp DISABLE error(code=%d)\n",
					iRet);
			}
		}
	}

	/* FSI Trigger stop */
	if (NULL != g_sndp_dai_func.fsi_trigger) {
		sndp_log_debug("fsi_dai_trigger stop\n");
		g_sndp_dai_func.fsi_trigger(&(wp->stop.fsi_substream),
					    SNDRV_PCM_TRIGGER_STOP,
					    &(wp->stop.fsi_dai));
	}

	/* FSI DAI Shutdown */
	if (NULL != g_sndp_dai_func.fsi_shutdown) {
		sndp_log_debug("fsi_dai_shutdown\n");
		g_sndp_dai_func.fsi_shutdown(&(wp->stop.fsi_substream),
					     &(wp->stop.fsi_dai));
	}

	/* FSI slave setting OFF */
	fsi_set_slave(false);

	if (SNDP_MODE_INCALL != SNDP_GET_MODE_VAL(uiValue)) {
		if (NULL != g_sndp_codec_info.get_device) {
			iRet = g_sndp_codec_info.get_device(&ulSetDevice);
			sndp_log_debug("get_dev[%08x]\n", ulSetDevice);
			if (ERROR_NONE != iRet)
				sndp_log_err("get_device error(codec=%d)\n",
					     iRet);
		}

		if (SNDP_PCM_OUT == direction) {
			/* Init to Out devices */
			ulSetDevice &= ~g_sndp_codec_info.out_dev_all;
		} else if (SNDP_PCM_IN == direction) {
			/* Init to In devices */
			ulSetDevice &= ~g_sndp_codec_info.in_dev_all;
			if (E_PLAY != g_sndp_playrec_flg)
				ulSetDevice = 0;
		}

		if (NULL != g_sndp_codec_info.set_device) {
			if (!((SNDP_PCM_OUT == direction) &&
			      (E_CAP == g_sndp_playrec_flg))) {
				sndp_log_debug("set_next_dev[%08x]\n",
					ulSetDevice);
				iRet = g_sndp_codec_info.set_device(
					ulSetDevice, SNDP_VALUE_INIT,
						g_sndp_codec_info.power_on);
				if (ERROR_NONE != iRet)
					sndp_log_err("set_device error (code=%d)\n",
						iRet);
			}
		}
	}

	/* Disable the power domain */
	if (!g_sndp_playrec_flg) {
		/* stop CLKGEN */
		clkgen_stop();

		/* FSI master for ES 2.0 over */
		if ((system_rev & 0xffff) >= 0x3E10)
			common_set_pll22(GET_OLD_VALUE(direction), STAT_OFF);

		pm_runtime_put_sync(g_sndp_power_domain);
	}

	/* Wake Unlock or Force Unlock */
	sndp_wake_lock((g_sndp_playrec_flg) ? E_UNLOCK : E_FORCE_UNLOCK);

	sndp_log_debug_func("end\n");
}


/*!
   @brief SoundPath Start and HardWare Parameter settings

   @param[in]	direction	SNDP_PCM_OUT/SNDP_PCM_IN
   @param[out]	none

   @retval	none
 */
static void sndp_fm_work_start(const int direction)
{
	int	iRet = ERROR_NONE;
	u_int	uiValue;

	/* FSI Trigger in FM radio start */
	if (NULL != fsi_dai_trigger_in_fm) {
		sndp_log_debug("fsi_dai_trigger_in_fm start\n");
		iRet = fsi_dai_trigger_in_fm(
				g_sndp_main[direction].arg.fsi_substream,
				SNDRV_PCM_TRIGGER_START,
				g_sndp_main[direction].arg.fsi_dai);
		if (ERROR_NONE != iRet)
			sndp_log_err("fsi_trigger_in_fm error(code=%d)\n",
				     iRet);
	}
}


/*!
   @brief SoundPath Stop

   @param[in]	work		work queue structure
   @param[in]	direction	SNDP_PCM_OUT/SNDP_PCM_IN
   @param[out]	none

   @retval	none
 */
static void sndp_fm_work_stop(
	struct work_struct *work,
	const int direction)
{
	u_int	uiValue;
	struct sndp_work_info	*wp = NULL;

	/* To get a work queue structure */
	wp = container_of((void *)work, struct sndp_work_info, work);

	uiValue = GET_OLD_VALUE(direction);

	/* FSI Trigger stop */
	if (NULL != fsi_dai_trigger_in_fm) {
		sndp_log_debug("fsi_dai_trigger_in_fm stop\n");
		fsi_dai_trigger_in_fm(&(wp->stop.fsi_substream),
					    SNDRV_PCM_TRIGGER_STOP,
					    &(wp->stop.fsi_dai));
	}
}


/*!
   @brief Voice stop and Normal device change <br>
	  (Post-processing of this sndp_work_call_capture_stop())

   @param[in]	iInValue	PCM value for INPUT side
   @param[in]	iOutValue	PCM value for OUTPUT side
   @param[out]	none

   @retval	none
 */
static void sndp_after_of_work_call_capture_stop(
	const u_int iInValue,
	const u_int iOutValue)
{
	int	iRet = ERROR_NONE;
	u_long	ulSetDevice = g_sndp_codec_info.dev_none;


	sndp_log_debug_func("start iInValue[0x%08X] iOutValue[0x%08X]\n",
							iInValue, iOutValue);
	/*
	 * A process similar to sndp_work_voice_stop()
	 */

	/* stop SCUW */
	scuw_stop();

	/* stop FSI */
	fsi_stop();

	/* stop CLKGEN */
	clkgen_stop();

	/* Disable the power domain */
	iRet = pm_runtime_put_sync(g_sndp_power_domain);
	if (ERROR_NONE != iRet)
		sndp_log_debug("modules power off iRet=%d\n", iRet);

	/* Trigger stop control flag update */
	g_sndp_stop_trigger_condition[SNDP_PCM_IN] &= ~SNDP_STOP_TRIGGER_VOICE;

	/* Return from the waiting of the shutdown */
	wake_up_interruptible(&g_sndp_stop_wait);

	/*
	 * A process similar to sndp_work_normal_dev_chg()
	 */

	/* device change */
	ulSetDevice = sndp_get_next_devices(iOutValue);
	if (NULL != g_sndp_codec_info.set_device) {
		iRet = g_sndp_codec_info.set_device(
				ulSetDevice,
				iOutValue,
				g_sndp_codec_info.power_on);
		if (ERROR_NONE != iRet)
			sndp_log_err("set device error (code=%d)\n", iRet);
	}

	/* Wake Force Unlock */
	sndp_wake_lock(E_FORCE_UNLOCK);

	sndp_log_debug_func("end\n");
}


#ifdef SOUND_TEST

#define SYSC_PHY_BASE	(0xE6180000)
#define SYSC_REG_MAX	(0x0084)

/* SYSC base address */
u_long g_sysc_Base;

/* Path test pm_runtime get function */
void sndp_path_test_pm_runtime_get_sync(void)
{
	u_int reg;
	int i;


/*	fsi_set_callback(sndp_fsi_interrupt); */
/*	pm_runtime_get_sync(g_sndp_power_domain); */

	/* Get SYSC Logical Address */
	g_sysc_Base = (u_long)ioremap_nocache(SYSC_PHY_BASE, SYSC_REG_MAX);
	if (0 >= g_sysc_Base) {
		printk(KERN_WARNING "%s SYSC ioremap failed\n", __func__);
		return;
	}

	/* PSTR */
	reg = ioread32(SYSC_PSTR);
	if (reg & (1 << 8)) {
		printk(KERN_WARNING "%s supplied to A4MP\n", __func__);
		return;
	}

	/* WakeUp */
	iowrite32((1 << 8), SYSC_SWUCR);
	for (i = 0; i < 500; i++) {
		reg = ioread32(SYSC_SWUCR);
		reg &= (1 << 8);
		if (!reg)
			break;
	}
	if (500 == i)
		printk(KERN_WARNING "%s Wake up error\n", __func__);

}


/* Path test pm_runtime put function */
void sndp_path_test_pm_runtime_put_sync(void)
{
/*	pm_runtime_put_sync(g_sndp_power_domain); */

	/* Release SYSC Logical Address */
	if (0 < g_sysc_Base) {
		iounmap((void *)g_sysc_Base);
		g_sysc_Base = 0;
	}
}


/* Path test sndp_init */
void sndp_path_test_sndp_init(void)
{
	int			iRet = -EFAULT;
	struct proc_dir_entry	*entry = NULL;
	struct proc_dir_entry	*reg_dump_entry = NULL;

	printk(KERN_WARNING"start\n");
#if 0
	/* Logical address base */
	g_fsi_Base = 0;
	g_scuw_Base = 0;
	g_clkgen_Base = 0;

	/* for Proc control */
	g_sndp_parent = proc_mkdir(SNDP_DRV_NAME, NULL);
	if (NULL != g_sndp_parent) {
		/* create file for log level entry */
		entry = create_proc_entry(
				LOG_LEVEL,
				S_IRUGO | S_IWUGO,
				g_sndp_parent);
		if (NULL != entry) {
			entry->read_proc  = sndp_proc_read;
			entry->write_proc = sndp_proc_write;
		} else {
			printk(KERN_WARNING"create_proc_entry(LOG) failed\n");
		}

		/* create file for register dump */
		reg_dump_entry = create_proc_entry(
						SNDP_REG_DUMP,
						S_IRUGO | S_IWUGO,
						g_sndp_parent);
		if (NULL != reg_dump_entry) {
			reg_dump_entry->read_proc = sndp_proc_reg_dump_read;
			reg_dump_entry->write_proc = sndp_proc_reg_dump_write;
		} else {
			printk(KERN_WARNING"create_proc_entry(REG) failed\n");
			remove_proc_entry(LOG_LEVEL, g_sndp_parent);
			remove_proc_entry(CALL_STATUS_SWITCH, g_sndp_parent);
		}

	} else {
		printk(KERN_WARNING "create failed for proc parrent\n");
	}

	/* for Log output */
	g_sndp_log_level = 0x14;

	/* ioremap */
	iRet = common_ioremap();
	if (ERROR_NONE != iRet)
		printk(KERN_WARNING "ioremap error\n");
#endif
}


static void sndp_fsi_interrupt(void)
{
/*	fsi_test_interrupt_a(); */
}

#endif /* SOUND_TEST */


