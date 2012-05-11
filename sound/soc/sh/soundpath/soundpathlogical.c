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
/*
 *#include <mach/pm.h>
 *#include <linux/vcd/vcd.h>
*/
#include <sound/soundpath/soundpath.h>
#include <sound/soundpath/common_extern.h>
#include <sound/soundpath/scuw_extern.h>
#include <sound/soundpath/fsi_extern.h>
#include <sound/soundpath/clkgen_extern.h>
#include <sound/soundpath/max98090_extern.h>
#include <sound/soundpath/call_extern.h>
#include <sound/sh_fsi.h>
#include "soundpathlogical.h"



/*
 *
 * DEFINE Definitions
 *
 */
/* For all Playback device types */
#define SNDP_OUT_DEV_ALL	(MAX98090_DEV_PLAYBACK_SPEAKER	|	\
				 MAX98090_DEV_PLAYBACK_EARPIECE |	\
				 MAX98090_DEV_PLAYBACK_HEADPHONES)
/* For all Capture device types */
#define SNDP_IN_DEV_ALL		(MAX98090_DEV_CAPTURE_MIC	|	\
				 MAX98090_DEV_CAPTURE_HEADSET_MIC)


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
/* COMMENT: Debaisu ha, korede zenbu? */
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
	[SNDP_MODE_INIT][SNDP_MODE_NORMAL]  =	{SNDP_PROC_MAXIM_START,
							SNDP_MODE_NORMAL},
	[SNDP_MODE_INIT][SNDP_MODE_RING]    =	{SNDP_PROC_MAXIM_START,
							SNDP_STAT_RINGTONE},
	[SNDP_MODE_INIT][SNDP_MODE_INCALL]  =	{SNDP_PROC_CALL_START |
						 SNDP_PROC_WATCH_STOP_FW,
							SNDP_STAT_IN_CALL},
	[SNDP_MODE_INIT][SNDP_MODE_INCOMM]  =	{SNDP_PROC_MAXIM_START,
							SNDP_STAT_IN_COMM},
	[SNDP_MODE_NORMAL][SNDP_MODE_NORMAL] =	{SNDP_PROC_MAXIM_START,
							SNDP_STAT_NOT_CHG},
	[SNDP_MODE_NORMAL][SNDP_MODE_RING]   =	{SNDP_PROC_MAXIM_START,
							SNDP_STAT_RINGTONE},
	[SNDP_MODE_NORMAL][SNDP_MODE_INCALL] =	{SNDP_PROC_CALL_START |
						 SNDP_PROC_WATCH_STOP_FW,
							SNDP_STAT_IN_CALL},
	[SNDP_MODE_NORMAL][SNDP_MODE_INCOMM] =	{SNDP_PROC_MAXIM_START,
							SNDP_STAT_IN_COMM},
	[SNDP_MODE_RING][SNDP_MODE_NORMAL]   =	{SNDP_PROC_MAXIM_START,
							SNDP_STAT_NORMAL},
	[SNDP_MODE_RING][SNDP_MODE_RING]     =	{SNDP_PROC_MAXIM_START,
							SNDP_STAT_NOT_CHG},
	[SNDP_MODE_RING][SNDP_MODE_INCALL]   =	{SNDP_PROC_CALL_START |
						 SNDP_PROC_WATCH_STOP_FW,
							SNDP_STAT_IN_CALL},
	[SNDP_MODE_RING][SNDP_MODE_INCOMM]   =	{SNDP_PROC_MAXIM_START,
							SNDP_STAT_IN_COMM},
	[SNDP_MODE_INCALL][SNDP_MODE_NORMAL] =	{SNDP_PROC_CALL_STOP |
						 SNDP_PROC_MAXIM_START,
							SNDP_STAT_NORMAL},
	[SNDP_MODE_INCALL][SNDP_MODE_RING]   =	{SNDP_PROC_CALL_STOP |
						 SNDP_PROC_MAXIM_START,
							SNDP_STAT_RINGTONE},
	[SNDP_MODE_INCALL][SNDP_MODE_INCALL] =	{SNDP_PROC_DEV_CHANGE,
							SNDP_STAT_NOT_CHG},
	[SNDP_MODE_INCALL][SNDP_MODE_INCOMM] =	{SNDP_PROC_CALL_STOP |
						 SNDP_PROC_MAXIM_START,
							SNDP_STAT_IN_COMM},
	[SNDP_MODE_INCOMM][SNDP_MODE_NORMAL] =	{SNDP_PROC_MAXIM_START,
							SNDP_STAT_NORMAL},
	[SNDP_MODE_INCOMM][SNDP_MODE_RING]   =	{SNDP_PROC_MAXIM_START,
							SNDP_STAT_RINGTONE},
	[SNDP_MODE_INCOMM][SNDP_MODE_INCALL] =	{SNDP_PROC_CALL_START |
						 SNDP_PROC_WATCH_STOP_FW,
							SNDP_STAT_IN_CALL},
	[SNDP_MODE_INCOMM][SNDP_MODE_INCOMM] =	{SNDP_PROC_MAXIM_START,
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
	.maxim_startup		= NULL,
	.maxim_shutdown		= NULL,
	.maxim_hw_params	= {NULL, NULL},
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
/* Device change for not voice */
static struct sndp_work_info g_sndp_work_normal_dev_chg;
/* Playback start for MAXIM */
static struct sndp_work_info g_sndp_work_maxim_play_start;
/* Capture start for MAXIM */
static struct sndp_work_info g_sndp_work_maxim_capture_start;
/* Playback stop for MAXIM */
static struct sndp_work_info g_sndp_work_maxim_play_stop;
/* Capture stop for MAXIM */
static struct sndp_work_info g_sndp_work_maxim_capture_stop;
/* Start during a call playback */
static struct sndp_work_info g_sndp_work_call_playback_start;
/* Start during a call capture */
static struct sndp_work_info g_sndp_work_call_capture_start;
/* Stop during a call playback */
static struct sndp_work_info g_sndp_work_call_playback_stop;
/* Stop during a call capture */
static struct sndp_work_info g_sndp_work_call_capture_stop;
/* VCD_COMMAND_WATCH_STOP_FW registration process */
static struct sndp_work_info g_sndp_work_regist_watch_stop_fw;
/* VCD_COMMAND_WATCH_STOP_FW process */
static struct sndp_work_info g_sndp_work_watch_stop_fw;

/* for Power control */
static int g_sndp_power_status = SNDP_POWER_INIT;
static struct device *g_sndp_power_domain;
/* static size_t g_sndp_power_domain_count;	 *TODO */

/* for Wake Lock */
DEFINE_SPINLOCK(lock);
static u_int g_wake_lock_count;

/* for Proc control */
static struct proc_dir_entry *g_sndp_parent;

/* for Register Dump Log */
static int g_sndp_now_direction = SNDP_PCM_DIRECTION_MAX;

/* Running state of the Playback or Capture */
static int g_sndp_playrec_flg = E_IDLE;

/* Routing type of the stream, in during a call */
static int g_sndp_stream_route = E_ROUTE_NORMAL;

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

/* for MAXIM DAI OPS */
#if 0	/* TODO */
static struct snd_soc_dai_ops sndp_maxim_dai_ops = {
	.startup	= sndp_maxim_startup,
	.shutdown	= sndp_maxim_shutdown,
	.hw_params	= sndp_maxim_hw_params,
};
#endif	/* TODO */

/* pointer receive log cycle counter */
u_int g_sndp_log_cycle_counter[SNDP_PCM_DIRECTION_MAX];

#if defined(DEBUG) && defined(__PRN_SNDP__)
/* Device name */
/* COMMENT: Debaisuha, korede zenbu? */
static const struct sndp_pcm_name_suffix device_suffix[] = {
	{ SNDP_OUT_EARPIECE,			"_Earpiece"		},
	{ SNDP_OUT_SPEAKER,			"_Speaker"		},
	{ SNDP_OUT_WIRED_HEADSET,		"_Headset"		},
	{ SNDP_OUT_WIRED_HEADPHONE,		"_Headphone"		},
	{ SNDP_OUT_BLUETOOTH_SCO,		"_Bluetooth"		},
	{ SNDP_OUT_BLUETOOTH_SCO_HEADSET,	"_Bluetooth"		},
	{ SNDP_OUT_BLUETOOTH_A2DP,		"_Bluetooth-A2DP"	},
	{ SNDP_OUT_UPLINK,			"_Uplink"		},
	{ SNDP_IN_BUILTIN_MIC,			"_Mic"			},
	{ SNDP_IN_BLUETOOTH_SCO_HEADSET,	"_Bluetooth"		},
	{ SNDP_IN_WIRED_HEADSET,		"_Headset"		},
	{ SNDP_IN_VOICE_CALL,			"_Voicecall"		},
};

/* Mode name */
static const struct sndp_pcm_name_suffix mode_suffix[] = {
	{ SNDP_MODE_NORMAL,			"_normal"		},
	{ SNDP_MODE_RING,			"_ringtone"		},
	{ SNDP_MODE_INCALL,			"_incall"		},
	{ SNDP_MODE_INCOMM,			"_incommunication"	},
};

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


/*!
   @brief Print PCM name for debug

   @param[in]	uiValue		PCM value
   @param[out]	none

   @retval	none
 */

#if defined(DEBUG) && defined(__PRN_SNDP__)

static inline void sndp_print_pcm_name(const u_int uiValue)
{
	char	cBuf[SNDP_PCM_NAME_MAX_LEN];
	int	iCnt;
	u_int	uiDevice;
	u_int	uiMode;


	/* Log level check */
	if (LOG_DEBUG_PRINT > LOG_BYTE_LOW(g_sndp_log_level))
		return;

	/* Get mode type */
	uiMode = SNDP_GET_MODE_VAL(uiValue);

	/* Get device type */
	uiDevice = SNDP_GET_AUDIO_DEVICE(uiValue);

	/* Direction type */
	strcpy(cBuf,
	       (SNDP_GET_DIRECTION_VAL(uiValue)) ?
		SNDP_IN_PCM_SUFFIX : SNDP_OUT_PCM_SUFFIX);

	/* Device name */
	for (iCnt = 0; ARRAY_SIZE(device_suffix) > iCnt; iCnt++) {
		if (device_suffix[iCnt].key & uiDevice)
			strcat(cBuf, device_suffix[iCnt].suffix);
	}

	/* Mode name */
	for (iCnt = 0; ARRAY_SIZE(mode_suffix) > iCnt; iCnt++) {
		if (mode_suffix[iCnt].key == uiMode) {
			strcat(cBuf, mode_suffix[iCnt].suffix);
			break;
		}
	}

	sndp_log_info("PCM: %s [0x%08X]\n", cBuf, uiValue);
}
#else
#define sndp_print_pcm_name(uiValue) do { } while (0)
#endif


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
	u_long	ulTmpNextDev = MAX98090_DEV_NONE;
	u_int	uiDev;


	sndp_log_debug_func("start uiValue[0x%08X]\n", uiValue);

	/* OUTPUT side */
	if (SNDP_PCM_OUT == SNDP_GET_DIRECTION_VAL(uiValue)) {

		/* Get now enabled devices */
		iRet = max98090_get_device(&ulTmpNextDev);
		if (ERROR_NONE != iRet)
			sndp_log_err("max98090_get_device error(code=%d)\n",
				iRet);

		/* Init to Playback side */
		ulTmpNextDev &= ~SNDP_OUT_DEV_ALL;

		/* Check OUTPUT device */
		uiDev = SNDP_GET_DEVICE_VAL(uiValue);

		/* [OUT]Speaker */
		if (SNDP_SPEAKER & uiDev)
			ulTmpNextDev |= MAX98090_DEV_PLAYBACK_SPEAKER;

		/* [OUT]Earpiece */
		if (SNDP_EARPIECE & uiDev)
			ulTmpNextDev |= MAX98090_DEV_PLAYBACK_EARPIECE;

		/* [OUT]Headphone */
		if (SNDP_WIREDHEADPHONE & uiDev)
			ulTmpNextDev |= MAX98090_DEV_PLAYBACK_HEADPHONES;

		/* [OUT]Headset */
		if (SNDP_WIREDHEADSET & uiDev)
			ulTmpNextDev |= MAX98090_DEV_PLAYBACK_HEADPHONES;

		/* IN_CALL mode check */
		if (SNDP_MODE_INCALL == SNDP_GET_MODE_VAL(uiValue)) {

			/* Init to Capture side */
			ulTmpNextDev &= ~SNDP_IN_DEV_ALL;

			/*
			 * To identify the INPUT device,
			 * to check the OUTPUT device.
			 */
			if (SNDP_WIREDHEADSET & uiDev) {
				/*
				 * [OUT]Including the Headset,
				 * [IN]Headset MIC
				 */
				ulTmpNextDev |= MAX98090_DEV_CAPTURE_HEADSET_MIC;
			} else {
				/*
				 * [OUT]Dosn't include the Headset,
				 * [IN]Built-in MIC
				 */
				ulTmpNextDev |= MAX98090_DEV_CAPTURE_MIC;
			}
		}
	/* INPUT side */
	} else {
		/* Get now enabled devices */
		iRet = max98090_get_device(&ulTmpNextDev);
		if (ERROR_NONE != iRet)
			sndp_log_err("max98090_get_device error(code=%d)\n",
				     iRet);

		/* Init to Capture side */
		ulTmpNextDev &= ~SNDP_IN_DEV_ALL;

		/* Check INPUT device */
		uiDev = SNDP_GET_DEVICE_VAL(uiValue);

		/* [IN]Built-in MIC */
		if (SNDP_BUILTIN_MIC & uiDev)
			ulTmpNextDev |= MAX98090_DEV_CAPTURE_MIC;

		/* [IN]Headset MIC */
		if (SNDP_WIREDHEADSET & uiDev)
			ulTmpNextDev |= MAX98090_DEV_CAPTURE_HEADSET_MIC;
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


extern void max98090_set_soc_controls(
	struct snd_kcontrol_new *controls,
	u_int array_size);

#define SYSC_PHY_BASE	(0xE6180000)
#define SYSC_REG_MAX	(0x0084)

/* SYSC base address */
u_long g_sysc_Base;

#define SYSC_SPDCR		(g_sysc_Base + 0x0008)
#define SYSC_SWUCR		(g_sysc_Base + 0x0014)
#define SYSC_PSTR		(g_sysc_Base + 0x0080)

/*!
   @brief Sound path driver init function (from module_init)

   @param[in]	fsi_port_dai		DAI for FSI(CPU DAI)
   @param[in]	maxim_dai		DAI for MAXIM(CODEC DAI)
   @param[in]	fsi_soc_platform	Structure for FSI SoC platform
   @param[out]	none

   @retval	0			Successful
   @retval	-EFAULT			Other error
 */
int sndp_init(struct snd_soc_dai_driver *fsi_port_dai_driver,
	      struct snd_soc_dai_driver *max98090_dai_driver,
	      struct snd_soc_platform_driver *fsi_soc_platform)
{
	int			iRet = -EFAULT;
	int			iCnt = 0;
	struct proc_dir_entry	*entry = NULL;
	struct proc_dir_entry	*reg_dump_entry = NULL;

	int reg, i;


	sndp_log_debug_func("start\n");

	/* Logical address base */
/*
	g_fsi_Base = 0;
	g_scuw_Base = 0;
	g_clkgen_Base = 0;
*/
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
	INIT_WORK(&g_sndp_work_normal_dev_chg.work,
		  sndp_work_normal_dev_chg);
	INIT_WORK(&g_sndp_work_maxim_play_start.work,
		  sndp_work_maxim_play_start);
	INIT_WORK(&g_sndp_work_maxim_capture_start.work,
		  sndp_work_maxim_capture_start);
	INIT_WORK(&g_sndp_work_maxim_play_stop.work,
		  sndp_work_maxim_play_stop);
	INIT_WORK(&g_sndp_work_maxim_capture_stop.work,
		  sndp_work_maxim_capture_stop);
	INIT_WORK(&g_sndp_work_call_playback_start.work,
		  sndp_work_call_playback_start);
	INIT_WORK(&g_sndp_work_call_capture_start.work,
		  sndp_work_call_capture_start);
	INIT_WORK(&g_sndp_work_call_playback_stop.work,
		  sndp_work_call_playback_stop);
	INIT_WORK(&g_sndp_work_call_capture_stop.work,
		  sndp_work_call_capture_stop);
	INIT_WORK(&g_sndp_work_regist_watch_stop_fw.work,
		  sndp_work_regist_watch_stop_fw);
	INIT_WORK(&g_sndp_work_watch_stop_fw.work,
		  sndp_work_watch_stop_fw);

	/* To initialize the flag waiting for the trigger stop processing. */
	for (iCnt = 0; SNDP_PCM_DIRECTION_MAX > iCnt; iCnt++)
		g_sndp_stop_trigger_condition[iCnt] = SNDP_STOP_TRIGGER_INIT;

	/* To save the volume value specified by the APL
	 * (Assumption that the AudioLSI driver already started)
	 */
	iRet = max98090_set_volume(SNDP_OUT_DEV_ALL, MAX98090_VOLUMEL5);
	if (ERROR_NONE != iRet) {
		sndp_log_always_err("set volume[%d] error\n",
				    MAX98090_VOLUMEL5);
		goto set_volume_err;
	}

	/* To save the MIC mute value specified by the APL
	 * (Assumption that the AudioLSI driver already started)
	 */
	iRet = max98090_set_mute(MAX98090_MUTE_DISABLE);
	if (ERROR_NONE != iRet) {
		sndp_log_always_err("set mute[%d] error\n",
				    MAX98090_MUTE_DISABLE);
		goto set_mute_err;
	}

	/* ioremap */
	iRet = common_ioremap();
	if (ERROR_NONE != iRet)
		goto ioremap_err;

	/* Replaced of function pointers. */
	g_sndp_dai_func.fsi_startup = fsi_port_dai_driver->ops->startup;
	g_sndp_dai_func.fsi_shutdown = fsi_port_dai_driver->ops->shutdown;
	g_sndp_dai_func.fsi_trigger = fsi_port_dai_driver->ops->trigger;
	g_sndp_dai_func.fsi_set_fmt = fsi_port_dai_driver->ops->set_fmt;
	g_sndp_dai_func.fsi_hw_params = fsi_port_dai_driver->ops->hw_params;
	g_sndp_dai_func.fsi_pointer = fsi_soc_platform->ops->pointer;
/*
	g_sndp_dai_func.maxim_hw_params[SNDP_PCM_OUT] =
		max98090_dai_driver[SNDP_PCM_OUT]->ops->hw_params;
	g_sndp_dai_func.maxim_hw_params[SNDP_PCM_IN] =
		max98090_dai_driver[SNDP_PCM_IN]->ops->hw_params;
*/
#if 0
	/* COMMENT: atode I/F wo kimeru. */
	g_sndp_dai_func.maxim_startup = g_max98090_ctrl_dai_func.dai_startup;
	g_sndp_dai_func.maxim_shutdown = g_max98090_ctrl_dai_func.dai_shutdown;
#endif

	fsi_port_dai_driver->ops = &sndp_fsi_dai_ops;
/*
	max98090_dai_driver[SNDP_PCM_OUT]->ops = &sndp_maxim_dai_ops;
	max98090_dai_driver[SNDP_PCM_IN]->ops = &sndp_maxim_dai_ops;
*/
	fsi_soc_platform->ops->pointer = sndp_fsi_pointer;
	fsi_set_run_time(sndp_fsi_suspend, sndp_fsi_resume);

	/* MAXIM SoC control */
	/* COMMENT: Kono syoriha hitsuyouka?? */
	max98090_set_soc_controls(g_sndpdrv_controls,
				  ARRAY_SIZE(g_sndpdrv_controls));
/*
	iRet = snd_soc_add_controls(maxim_dai[SNDP_PCM_OUT].codec,
				    g_sndpdrv_controls,
				    ARRAY_SIZE(g_sndpdrv_controls));
	if (ERROR_NONE != iRet) {
		sndp_log_err("Failed to create control\n");
		goto add_control_err;
	}
*/
/*
	// Power domain setting
	iRet = power_domain_devices("snd-soc-fsi",
				    &g_sndp_power_domain,
				    &g_sndp_power_domain_count);
	if (ERROR_NONE != iRet) {
		sndp_log_err("Modules not found ... [iRet=%d]\n", iRet);
		goto power_domain_err;
	}
*/

	/* create work queue for call*/
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

	/* Wake lock init */
	wake_lock_init(&g_sndp_wake_lock_suspend,
		       WAKE_LOCK_SUSPEND,
		       "snd-soc-fsi");

	/* wait queue init */
	init_waitqueue_head(&g_sndp_stop_wait);

	/* Get SYSC Logical Address */
	g_sysc_Base = (u_long)ioremap_nocache(SYSC_PHY_BASE, SYSC_REG_MAX);
	if (0 >= g_sysc_Base) {
		printk(KERN_WARNING "sndp_init() SYSC ioremap failed error\n");
		return ERROR_NONE;
	}

	/* PSTR */
	reg = ioread32(SYSC_PSTR);
	printk(KERN_WARNING "sndp_init() reg = 0x%x\n", reg);
	if (reg & (1 << 8)) {
		printk(KERN_WARNING "sndp_init() Power is supplied to A4MP area\n");
		iounmap((void *)g_sysc_Base);
		return ERROR_NONE;
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
		printk(KERN_WARNING "sndp_init() Wake up error\n");

	iounmap((void *)g_sysc_Base);



	sndp_log_debug_func("end\n");
	return ERROR_NONE;

set_volume_err:
set_mute_err:
ioremap_err:
mkproc_sub_err:
/*
add_control_err:	TODO
power_domain_err:	TODO
*/
workque_create_err:
	remove_proc_entry(SNDP_DRV_NAME, NULL);

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
#if 0

	/* for MAXIM */
	if ((REG_DUMP_ALL == ulIn) || (REG_DUMP_MAXIM & ulIn)) {
		iRet = max98090_dump_registers(MAX98090_AUDIO_IC_ALL);
		if (ERROR_NONE != iRet)
			sndp_log_reg_dump("MAXIM register is not ready.\n");
	}
#endif
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
		if ((SNDP_PCM_DIRECTION_MAX != g_sndp_now_direction) &&
		    (SNDP_MODE_INCALL == 
			SNDP_GET_MODE_VAL(GET_OLD_VALUE(SNDP_PCM_OUT))) &&
			(!(E_ROUTE_PLAY_CHANGED & g_sndp_stream_route)))
			scuw_reg_dump();
		else
			sndp_log_reg_dump("SCUW register is not ready.\n");
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
	/* int	iRet = ERROR_NONE; */
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
	sndp_print_pcm_name(uiValue);

	/* Gets the direction (Playback/Capture) */
	uiDirection = SNDP_GET_DIRECTION_VAL(uiValue);

	/* Get Old Value */
	uiOldValue = GET_OLD_VALUE(uiDirection);
	old_mode   = SNDP_GET_MODE_VAL(uiOldValue);
#if 0
	/* Uplink only/Downlink only/Both Uplink and Downlink */
	if (SNDP_PLAYBACK_UPLINK_INCALL == uiValue) {
		/* Uplink only */
		/* COMMENT: call_set_play_uplink() wo be-su ni kaizou   */
		/* COMMENT: rc5t7316_voice_dl_mute() wo be-su ni kaizou */
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
	if (SNDP_PCM_IN == uiDirection) {
		/* Saving the type of PCM */
		SET_OLD_VALUE(uiDirection, uiValue);
		return ERROR_NONE;
	}

	/* for Register dump debug */
	g_sndp_now_direction =
	(SNDP_MODE_INCALL == uiMode) ? SNDP_PCM_OUT : SNDP_PCM_DIRECTION_MAX;

	/* Processing for each process */
	/* SNDP_PROC_CALL_STOP */
	if (uiProcess & SNDP_PROC_CALL_STOP) {
		/* Call + Recording is not running */
		if (SNDP_MODE_INCALL !=
			SNDP_GET_MODE_VAL(GET_OLD_VALUE(SNDP_PCM_IN))) {

			/* Wake Lock */
			sndp_wake_lock(E_LOCK);

			/* Registered in the work queue for call stop */
			g_sndp_work_voice_stop.old_value = uiOldValue;

			queue_work(g_sndp_queue_main,
				   &g_sndp_work_voice_stop.work);
		}
	}

	/* SNDP_PROC_CALL_START */
	if (uiProcess & SNDP_PROC_CALL_START) {
		/* Enable the power domain */
#if 0
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
#endif
		/* Wake Lock */
		sndp_wake_lock(E_LOCK);

		/* Registered in the work queue for call start */
		g_sndp_work_voice_start.new_value = uiValue;
		queue_work(g_sndp_queue_main, &g_sndp_work_voice_start.work);
	}

	/* SNDP_PROC_MAXIM_START */
	if (uiProcess & SNDP_PROC_MAXIM_START) {

		/* Wake Lock */
		sndp_wake_lock(E_LOCK);

		/*
		 * Registered in the work queue for
		 * device change (not voice call)
		 */
		g_sndp_work_normal_dev_chg.new_value = uiValue;
		queue_work(g_sndp_queue_main, &g_sndp_work_normal_dev_chg.work);
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

	/* SNDP_PROC_WATCH_STOP_FW */
	if (uiProcess & SNDP_PROC_WATCH_STOP_FW) {
		/*
		 * Registered in the work queue for
		 * VCD_COMMAND_WATCH_STOP_FW registration
		 */
		queue_work(g_sndp_queue_main,
			   &g_sndp_work_regist_watch_stop_fw.work);
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


	/* Get the current settings(Always specify the MAX98090_PLACE_SW) */
	iRet = max98090_get_volume(SNDP_OUT_DEV_ALL, &uiVal);

	if (ERROR_NONE > iRet) {
		sndp_log_err("maxim get volume error(code=%d)\n", iRet);
		return iRet;
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
	iRet = max98090_set_volume(SNDP_OUT_DEV_ALL, uiVal);

	if (ERROR_NONE != iRet) {
		sndp_log_err("maxim set volume error(code=%d)\n", iRet);
		return iRet;
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
	iRet = max98090_get_mute(&uiVal);
	if (ERROR_NONE != iRet) {
		sndp_log_err("maxim get mute error(code=%d)\n", iRet);
		return iRet;
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
	iRet = max98090_set_mute(uiVal);
	if (ERROR_NONE != iRet) {
		sndp_log_err("maxim set mute error(code=%d)\n", iRet);
		return iRet;
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


	sndp_log_info("start\n");

	/* Otherwise only IN_CALL, for processing */
	if (SNDP_MODE_INCALL !=
		SNDP_GET_MODE_VAL(GET_OLD_VALUE(SNDP_PCM_OUT))) {
		if (SNDP_POWER_SUSPEND != g_sndp_power_status) {
			/*
			 * Transition to SUSPEND,
			 * status of MAXIM (Disable all devices)
			 */
			iRet = max98090_set_device(MAX98090_DEV_NONE);
			if (ERROR_NONE != iRet)
				sndp_log_err("max set device error(code=%d)\n",
					     iRet);

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
	u_long	ulSetDevice = MAX98090_DEV_NONE;


	sndp_log_info("start\n");

	/* Otherwise only IN_CALL, for processing */
	if (SNDP_MODE_INCALL !=
		SNDP_GET_MODE_VAL(GET_OLD_VALUE(SNDP_PCM_OUT))) {
		if (SNDP_POWER_RESUME != g_sndp_power_status) {
			/* Transition to RESUME, status of MAXIM */
			ulSetDevice =
			sndp_get_next_devices(GET_OLD_VALUE(SNDP_PCM_OUT));

			iRet = max98090_set_device(ulSetDevice);
			if (ERROR_NONE != iRet)
				sndp_log_err("max set device error(code=%d)\n",
					     iRet);

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
	long	iRet = 0;


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

	/* Check the waiting processing of TRIGGER STOP */
	iRet = wait_event_interruptible_timeout(
		g_sndp_stop_wait, !SNDP_STOP_TRIGGER_CHECK(substream->stream),
		msecs_to_jiffies(SNDP_WAIT_MAX));

	/* Initialize the trigger stop processing flag */
	SNDP_STOP_TRIGGER_INIT_SET(substream->stream);

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
	bool			trigger_normal = true;
	struct sndp_stop	*stop;
	struct sndp_arg		*arg;


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

	/* Checking whether a call */
	if (SNDP_MODE_INCALL ==
		SNDP_GET_MODE_VAL(GET_OLD_VALUE(substream->stream)))
		trigger_normal = false;

	/* MM Playback or MM Capture process route */
	if (trigger_normal) {
		switch (cmd) {
		case SNDRV_PCM_TRIGGER_START:	/* TRIGGER_START */
			sndp_log_info("#Trigger start[MM]\n");

			/* Display the name of PCM */
			sndp_print_pcm_name(GET_OLD_VALUE(substream->stream));

			/* Wake Lock */
			sndp_wake_lock(E_LOCK);

			/* A work queue processing to register TRIGGER_START */
			if (SNDP_PCM_OUT == substream->stream) {
				queue_work(g_sndp_queue_main,
					   &g_sndp_work_maxim_play_start.work);

				/* for Register dump debug */
				g_sndp_now_direction = SNDP_PCM_OUT;
			} else {
				queue_work(g_sndp_queue_main,
					&g_sndp_work_maxim_capture_start.work);

				/* for Register dump debug */
				g_sndp_now_direction = SNDP_PCM_IN;
			}
			break;

		case SNDRV_PCM_TRIGGER_STOP:	/* TRIGGER_STOP */
			sndp_log_info("#Trigger stop[MM]\n");

			arg = &g_sndp_main[substream->stream].arg;
			/* FSI trigger stop process */
			fsi_set_trigger_stop(arg->fsi_substream, false);

			/* Init register dump log flag for debug */
			g_sndp_now_direction = SNDP_PCM_DIRECTION_MAX;

			/* A work queue processing to register TRIGGER_STOP */
			if (SNDP_PCM_OUT == substream->stream) {
				g_sndp_stop_trigger_condition[SNDP_PCM_OUT] |=
						SNDP_STOP_TRIGGER_PLAYBACK;

				stop = &g_sndp_work_maxim_play_stop.stop;

				stop->fsi_substream = *arg->fsi_substream;

				stop->fsi_dai =	*arg->fsi_dai;

				queue_work(g_sndp_queue_main,
					   &g_sndp_work_maxim_play_stop.work);
			} else {
				g_sndp_stop_trigger_condition[SNDP_PCM_IN] |=
						SNDP_STOP_TRIGGER_CAPTURE;

				stop = &g_sndp_work_maxim_capture_stop.stop;

				stop->fsi_substream = *arg->fsi_substream;

				stop->fsi_dai = *arg->fsi_dai;

				queue_work(g_sndp_queue_main,
					&g_sndp_work_maxim_capture_stop.work);
			}
			break;
		default:
			sndp_log_debug("playback trigger none.\n");
			break;
		}
	/* Call process route */
	} else {
		sndp_call_trigger(substream,
				  cmd,
				  dai,
				  GET_OLD_VALUE(substream->stream));
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
	/* COMMENT: Hitomazu, Play to Capture no kubetsu ha shinai. */
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

	/* Not in a call */
	if (SNDP_MODE_INCALL !=
		SNDP_GET_MODE_VAL(GET_OLD_VALUE(substream->stream))) {
		iRet = g_sndp_dai_func.fsi_pointer(substream);

	/* During a call */
	} else {
		/* VCD is dead */
		if ((E_ROUTE_PLAY_CHANGED & g_sndp_stream_route) &&
		    (SNDP_PCM_OUT == substream->stream)) {
			iRet = g_sndp_dai_func.fsi_pointer(substream);

		/* VCD is alive */
		} else {
			iRet = call_pcmdata_pointer(substream);
		}
	}

	sndp_log_data_rcv_indicator(substream->stream, iRet);
	return iRet;
}


/*!
   @brief MAXIM startup function

   @param[in]	substream	PCM substream structure
   @param[in]	dai		Digital audio interface structure

   @retval	0		Successful
 */
#if 0	/* TODO */
static int sndp_maxim_startup(
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
	g_sndp_main[substream->stream].arg.maxim_substream = substream;
	g_sndp_main[substream->stream].arg.maxim_dai = dai;

	sndp_log_debug_func("end\n");
	return iRet;
}
#endif	/* TODO */


/*!
   @brief MAXIM shutdown function

   @param[in]	substream	PCM substream structure
   @param[in]	dai		Digital audio interface structure

   @retval	None
 */
#if 0	/* TODO */
static void sndp_maxim_shutdown(
	struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	sndp_log_debug_func("start\n");
	sndp_log_info("substream->stream = %d(%s)  old_value = 0x%08X\n",
		substream->stream,
		(SNDP_PCM_OUT == substream->stream) ? "PLAYBACK" : "CAPTURE",
		GET_OLD_VALUE(substream->stream));

	/* Playback or Capture, than the Not processing */
	if ((SNDP_PCM_OUT != substream->stream) &&
	    (SNDP_PCM_IN  != substream->stream)) {
		return;
	}

	/* To store information about Substream and DAI */
	g_sndp_main[substream->stream].arg.maxim_substream = substream;
	g_sndp_main[substream->stream].arg.maxim_dai = dai;

	/* During a Call + Playback, to return */
	if ((SNDP_MODE_INCALL ==
		SNDP_GET_MODE_VAL(GET_OLD_VALUE(substream->stream))) &&
	    (SNDP_PCM_OUT == substream->stream)) {
		return;
	}

	/* Initialize Old Value(Only Input side) */
	if (SNDP_PCM_IN == substream->stream)
		SET_OLD_VALUE(substream->stream, SNDP_VALUE_INIT);

	sndp_log_debug_func("end\n");
}
#endif	/* TODO */


/*!
   @brief MAXIM HW Parameters function

   @param[in]	substream	PCM substream structure
   @param[in]	params		HW Parameters structure
   @param[in]	dai		Digital audio interface structure

   @retval	0		Successful
 */
#if 0	/* TODO */
static int sndp_maxim_hw_params(
	struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params,
	struct snd_soc_dai *dai)
{
	sndp_log_debug_func("start\n");
	sndp_log_info("params = %d\n", params_rate(params));

	/* To store information about Hardware parameters */
	g_sndp_main[substream->stream].arg.maxim_params = *params;

	sndp_log_debug_func("end\n");
	return ERROR_NONE;
}
#endif	/* TODO */


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
   @brief Work queue function for Voice Start

   @param[in]	work	work queue structure
   @param[out]	none

   @retval	none
 */
static void sndp_work_voice_start(struct work_struct *work)
{
	int			iRet = ERROR_NONE;
	u_long			ulSetDevice = MAX98090_DEV_NONE;
	struct sndp_work_info	*wp = NULL;


	sndp_log_debug_func("start\n");

	/* To get a work queue structure */
	wp = container_of((void *)work, struct sndp_work_info, work);

	/* start MAXIM */
	ulSetDevice = sndp_get_next_devices(wp->new_value);
	iRet = max98090_set_device(ulSetDevice);
	if (ERROR_NONE != iRet) {
		sndp_log_err("maxim set device error (code=%d)\n", iRet);
		goto start_err;
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
	iRet = clkgen_start(wp->new_value);
	if (ERROR_NONE != iRet) {
		sndp_log_err("clkgen start error(code=%d)\n", iRet);
		goto start_err;
	}

	/* Set to ENABLE the speaker amp */
	if (SNDP_SPEAKER & SNDP_GET_DEVICE_VAL(wp->new_value)) {
		iRet = max98090_set_speaker_amp(MAX98090_SPEAKER_AMP_ENABLE);
		if (ERROR_NONE != iRet) {
			sndp_log_err("speaker_amp ENABLE error(code=%d)\n",
				     iRet);
			goto start_err;
		}
	}

start_err:
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
		iRet = max98090_set_speaker_amp(MAX98090_SPEAKER_AMP_DISABLE);
		if (ERROR_NONE != iRet)
			sndp_log_err("speaker_amp DISABLE error(code=%d)\n",
				     iRet);
	}

	/* stop SCUW */
	scuw_stop();

	/* stop FSI */
	fsi_stop();

	/* stop CLKGEN */
	clkgen_stop();

	/* stop MAXIM */
	if (NULL != g_sndp_dai_func.maxim_shutdown) {
		sndp_log_debug("maxim_dai_shutdown\n");
		g_sndp_dai_func.maxim_shutdown(
			g_sndp_main[SNDP_PCM_OUT].arg.maxim_dai,
			wp->old_value);
	}
#if 0
	/* Disable the power domain */
	iRet = pm_runtime_put_sync(g_sndp_power_domain);
	if (ERROR_NONE != iRet)
		sndp_log_debug("modules power off iRet=%d\n", iRet);
#endif
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
	u_long			ulSetDevice = MAX98090_DEV_NONE;
	struct sndp_work_info	*wp = NULL;


	sndp_log_debug_func("start\n");

	/* To get a work queue structure */
	wp = container_of((void *)work, struct sndp_work_info, work);

	/* MAXIM device change */
	if (wp->new_value != wp->old_value) {
		ulSetDevice = sndp_get_next_devices(wp->new_value);
		iRet = max98090_set_device(ulSetDevice);
		if (ERROR_NONE != iRet)
			sndp_log_err("maxim set device error (code=%d)\n",
				     iRet);
	}

	/* Set to ENABLE/DISABLE the speaker amp */
	if (SNDP_SPEAKER & SNDP_GET_DEVICE_VAL(wp->new_value))
		iRet = max98090_set_speaker_amp(MAX98090_SPEAKER_AMP_ENABLE);
	else
		iRet = max98090_set_speaker_amp(MAX98090_SPEAKER_AMP_DISABLE);

	if (ERROR_NONE != iRet) {
		sndp_log_err("max98090_set_speaker_amp %s error(code=%d)\n",
			(SNDP_SPEAKER & SNDP_GET_DEVICE_VAL(wp->new_value)) ?
							"ENABLE" : "DISABLE",
			iRet);
	}

	/* Wake Unlock */
	sndp_wake_lock(E_UNLOCK);

	sndp_log_debug_func("end\n");
}


/*!
   @brief Work queue function for Device change (not IN_CALL)

   @param[in]	work	work queue structure
   @param[out]	none

   @retval	none
 */
static void sndp_work_normal_dev_chg(struct work_struct *work)
{
	int			iRet = ERROR_NONE;
	u_long			ulSetDevice = MAX98090_DEV_NONE;
	struct sndp_work_info	*wp = NULL;


	sndp_log_debug_func("start\n");

	/* To get a work queue structure */
	wp = container_of((void *)work, struct sndp_work_info, work);

	/* MAXIM device change */
	ulSetDevice = sndp_get_next_devices(wp->new_value);
	iRet = max98090_set_device(ulSetDevice);
	if (ERROR_NONE != iRet)
		sndp_log_err("maxim set device error (code=%d)\n", iRet);

	/* Set to ENABLE/DISABLE the speaker amp */
	if (SNDP_SPEAKER & SNDP_GET_DEVICE_VAL(wp->new_value))
		iRet = max98090_set_speaker_amp(MAX98090_SPEAKER_AMP_ENABLE);
	else
		iRet = max98090_set_speaker_amp(MAX98090_SPEAKER_AMP_DISABLE);

	if (ERROR_NONE != iRet) {
		sndp_log_err("max98090_set_speaker_amp %s error(code=%d)\n",
			(SNDP_SPEAKER & SNDP_GET_DEVICE_VAL(wp->new_value)) ?
							"ENABLE" : "DISABLE",
			iRet);
	}

	/* Wake Unlock */
	sndp_wake_lock(E_UNLOCK);

	sndp_log_debug_func("end\n");
}


/*!
   @brief Work queue function for MAXIM Setting(Playback)

   @param[in]	work	work queue structure
   @param[out]	none

   @retval	none
 */
static void sndp_work_maxim_play_start(struct work_struct *work)
{
	sndp_log_debug_func("start\n");

	/* Running Playback */
	g_sndp_playrec_flg |= E_PLAY;

	/* To register a work queue to start processing Playback */
	sndp_maxim_work_start(SNDP_PCM_OUT);

	sndp_log_debug_func("end\n");
}


/*!
   @brief Work queue function for MAXIM Setting(Capture)

   @param[in]	work	work queue structure
   @param[out]	none

   @retval	none
 */
static void sndp_work_maxim_capture_start(struct work_struct *work)
{
	sndp_log_debug_func("start\n");

	/* Running Capture */
	g_sndp_playrec_flg |= E_CAP;

	/* To register a work queue to start processing Capture */
	sndp_maxim_work_start(SNDP_PCM_IN);

	sndp_log_debug_func("end\n");
}


/*!
   @brief Work queue function for MAXIM Stop(Playback)

   @param[in]	work	work queue structure
   @param[out]	none

   @retval	none
 */
static void sndp_work_maxim_play_stop(struct work_struct *work)
{
	sndp_log_debug_func("start\n");

	/* Stop Playback runnning */
	g_sndp_playrec_flg &= ~E_PLAY;

	/* To register a work queue to stop processing Playback */
	sndp_maxim_work_stop(work, SNDP_PCM_OUT);

	/* Reset a Trigger stop status flag */
	g_sndp_stop_trigger_condition[SNDP_PCM_OUT] &=
					~SNDP_STOP_TRIGGER_PLAYBACK;

	/* Wake up main process */
	wake_up_interruptible(&g_sndp_stop_wait);

	sndp_log_debug_func("end\n");
}


/*!
   @brief Work queue function for MAXIM Stop(Capture)

   @param[in]	work	work queue structure
   @param[out]	none

   @retval	none
 */
static void sndp_work_maxim_capture_stop(struct work_struct *work)
{
	sndp_log_debug_func("start\n");

	/* Stop Capture running */
	g_sndp_playrec_flg &= ~E_CAP;

	/* To register a work queue to stop processing Capture */
	sndp_maxim_work_stop(work, SNDP_PCM_IN);

	/* Reset a Trigger stop status flag */
	g_sndp_stop_trigger_condition[SNDP_PCM_IN] &=
					~SNDP_STOP_TRIGGER_CAPTURE;

	/* Wake up main process */
	wake_up_interruptible(&g_sndp_stop_wait);

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

	if (ERROR_NONE != iRet) {
		/* Switching path of the sound during a Call + Playback */
		if (!(E_ROUTE_PLAY_CHANGED & g_sndp_stream_route)) {
			sndp_path_switching(GET_OLD_VALUE(SNDP_PCM_OUT));
			g_sndp_stream_route |= E_ROUTE_PLAY_CHANGED;
		}
	}

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
		if (!(E_ROUTE_CAP_DUMMY & g_sndp_stream_route)) {
			call_change_dummy_rec();
			g_sndp_stream_route |= E_ROUTE_CAP_DUMMY;
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

	/* Back-out path of the sound during a Call + Playback */
	if (E_ROUTE_PLAY_CHANGED & g_sndp_stream_route) {
		/* COMMENT: mada kono kansuu ha nai */
		fsi_set_trigger_stop(&(wp->stop.fsi_substream), false);
		sndp_path_backout(GET_OLD_VALUE(SNDP_PCM_OUT));
		g_sndp_stream_route &= ~E_ROUTE_PLAY_CHANGED;
	}

	/* Call + Playback stop request */
	call_playback_stop();

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

	if (E_ROUTE_CAP_DUMMY & g_sndp_stream_route) {
		/* Dummy capture stop */
		g_sndp_stream_route &= ~E_ROUTE_CAP_DUMMY;
	}

	/* Call + Capture stop request */
	call_record_stop();

	/* Reset a Trigger stop status flag */
	g_sndp_stop_trigger_condition[SNDP_PCM_IN] &=
					~SNDP_STOP_TRIGGER_CAPTURE;

	/* If the state already NORMAL Playback side */
	if ((!(E_ROUTE_PLAY_CHANGED & g_sndp_stream_route)) &&
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
   @brief Work queue processing for VCD_COMMAND_WATCH_STOP_FW
	  registration process

   @param[in]	work	work queue structure
   @param[out]	none

   @retval	none
 */
static void sndp_work_regist_watch_stop_fw(struct work_struct *work)
{
	int	iRet = ERROR_NONE;


	sndp_log_debug_func("start\n");

	/* VCD command execution (VCD_COMMAND_WATCH_STOP_FW) */
	iRet = call_watch_stop_fw(sndp_watch_stop_fw_cb);
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

	/* During a Call + Playback */
	if (SNDP_STAT_IN_CALL_PLAY == GET_SNDP_STATUS(SNDP_PCM_OUT)) {
		/* Switching path of the sound during a Call + Playback */
		if (!(E_ROUTE_PLAY_CHANGED & g_sndp_stream_route)) {
			sndp_path_switching(GET_OLD_VALUE(SNDP_PCM_OUT));
			g_sndp_stream_route |= E_ROUTE_PLAY_CHANGED;
		}
	}

	/* During a Call + Capture */
	if (SNDP_STAT_IN_CALL_CAP == GET_SNDP_STATUS(SNDP_PCM_IN)) {
		/* Dummy capture start */
		if (!(E_ROUTE_CAP_DUMMY & g_sndp_stream_route)) {
			call_change_dummy_rec();
			g_sndp_stream_route |= E_ROUTE_CAP_DUMMY;
		}
	}

	sndp_log_debug_func("end\n");
}


/*!
   @brief Watch stop Firmware notification callback function

   @param[in]	uiNop	Not used
   @param[out]	none

   @retval	none
 */
static void sndp_watch_stop_fw_cb(u_int uiNop)
{
	sndp_log_debug_func("start uiNop[%d]\n", uiNop);

	/*
	 * Registered in the work queue for
	 * VCD_COMMAND_WATCH_STOP_FW process
	 */
	queue_work(g_sndp_queue_main, &g_sndp_work_watch_stop_fw.work);

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

	/* Wake Lock */
	sndp_wake_lock(E_LOCK);

	/* for Register dump debug */
	/* g_sndp_now_direction = SNDP_PCM_OUT; */

	/* Running Playback */
	g_sndp_playrec_flg |= E_PLAY;

	/* To register a work queue to start processing Playback */
	sndp_maxim_work_start(SNDP_PCM_OUT);
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

	stop = &g_sndp_work_maxim_play_stop.stop;

	stop->fsi_substream = *arg->fsi_substream;
	if (NULL == &stop->fsi_substream)
		sndp_log_debug_func("#### stop->fsi_substream is NULL.\n");

	stop->fsi_dai =	*arg->fsi_dai;
	if (NULL == &stop->fsi_dai)
		sndp_log_debug_func("#### stop->fsi_dai is NULL.\n");

	/* Stop Playback runnning */
	g_sndp_playrec_flg &= ~E_PLAY;

	/* To register a work queue to stop processing Playback */
	sndp_maxim_work_stop(&g_sndp_work_maxim_play_stop.work, SNDP_PCM_OUT);

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
	iRet = clkgen_start(uiValue);
	if (ERROR_NONE != iRet)
		sndp_log_err("clkgen start error(code=%d)\n", iRet);
}


/*!
   @brief MAXIM Start and HardWare Parameter settings

   @param[in]	direction	SNDP_PCM_OUT/SNDP_PCM_IN
   @param[out]	none

   @retval	none
 */
static void sndp_maxim_work_start(const int direction)
{
	int	iRet = ERROR_NONE;
	u_long	ulSetDevice = MAX98090_DEV_NONE;


	sndp_log_debug_func("start direction[%d]\n", direction);

	if (SNDP_PCM_IN == direction) {
		/* start MAXIM */
		ulSetDevice = sndp_get_next_devices(GET_OLD_VALUE(direction));
		iRet = max98090_set_device(ulSetDevice);
		if (ERROR_NONE != iRet) {
			sndp_log_err("maxim set device error (code=%d)\n",
				     iRet);
			return;
		}
	}

	/* Set MAXIM hardware parameters */
	if (NULL != g_sndp_dai_func.maxim_hw_params[direction]) {
		g_sndp_dai_func.maxim_hw_params[direction](
			g_sndp_main[direction].arg.maxim_substream,
			&g_sndp_main[direction].arg.maxim_params,
			g_sndp_main[direction].arg.maxim_dai);
	}

	/* PM_RUNTIME */
	if ((E_PLAY | E_CAP) != g_sndp_playrec_flg) {
		/* ret = pm_runtime_get_sync(g_sndp_power_domain);
		if (!(0 == ret || 1 == ret)) {  // 0:success 1:active
			sndp_log_err("modules power on error(ret=%d)\n", ret);
		} else { */
			/* CPG soft reset */
			fsi_soft_reset();
		/*}*/
		/* pm_runtime_put_sync(g_sndp_power_domain); */
	}

	/* FSI startup */
	if (NULL != g_sndp_dai_func.fsi_startup) {
		sndp_log_debug("fsi_dai_startup\n");
		iRet = g_sndp_dai_func.fsi_startup(
				g_sndp_main[direction].arg.fsi_substream,
				g_sndp_main[direction].arg.fsi_dai);
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
	if ((SNDP_PCM_OUT == direction) &&
	    (SNDP_SPEAKER & SNDP_GET_DEVICE_VAL(GET_OLD_VALUE(direction)))) {
		iRet = max98090_set_speaker_amp(MAX98090_SPEAKER_AMP_ENABLE);
		if (ERROR_NONE != iRet) {
			sndp_log_err("speaker_amp ENABLE error(code=%d)\n",
				     iRet);
			return;
		}
	}

	/* FSI Trigger start */
	if (NULL != g_sndp_dai_func.fsi_trigger) {
		sndp_log_debug("fsi_dai_trigger start\n");
		iRet = g_sndp_dai_func.fsi_trigger(
				g_sndp_main[direction].arg.fsi_substream,
				SNDRV_PCM_TRIGGER_START,
				g_sndp_main[direction].arg.fsi_dai);
		if (ERROR_NONE != iRet) {
			sndp_log_err("fsi_trigger error(code=%d)\n", iRet);
			return;
		}
	}

	/* start CLKGEN */
	iRet = clkgen_start(GET_OLD_VALUE(direction));
	if (ERROR_NONE != iRet)
		sndp_log_err("clkgen start error(code=%d)\n", iRet);

	sndp_log_debug_func("end\n");
}


/*!
   @brief MAXIM Stop

   @param[in]	work		work queue structure
   @param[in]	direction	SNDP_PCM_OUT/SNDP_PCM_IN
   @param[out]	none

   @retval	none
 */
static void sndp_maxim_work_stop(
	struct work_struct *work,
	const int direction)
{
	int			iRet = ERROR_NONE;
	u_long			ulSetDevice = MAX98090_DEV_NONE;
	struct sndp_work_info	*wp = NULL;


	sndp_log_debug_func("start direction[%d]\n", direction);

	/* To get a work queue structure */
	wp = container_of((void *)work, struct sndp_work_info, work);

	/* Set to DISABLE the speaker amp */
	if ((SNDP_PCM_OUT == direction) &&
	    (SNDP_SPEAKER & SNDP_GET_DEVICE_VAL(GET_OLD_VALUE(direction)))) {
		iRet = max98090_set_speaker_amp(MAX98090_SPEAKER_AMP_DISABLE);
		if (ERROR_NONE != iRet)
			sndp_log_err("speaker_amp DISABLE error(code=%d)\n",
				     iRet);
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

	/* Capture path is delete */
	if (SNDP_PCM_IN == direction) {
		iRet = max98090_get_device(&ulSetDevice);
		if (ERROR_NONE != iRet)
			sndp_log_err("max98090_get_device error(code=%d)\n",
				     iRet);

		/* Init to Capture side */
		ulSetDevice &= ~SNDP_IN_DEV_ALL;

		/* set Device */
		iRet = max98090_set_device(ulSetDevice);
		if (ERROR_NONE != iRet)
			sndp_log_err("max98090_set_device error (code=%d)\n",
				     iRet);
	}

	/* stop CLKGEN */
	clkgen_stop();

	/* Wake Unlock or Force Unlock */
	sndp_wake_lock((g_sndp_playrec_flg) ? E_UNLOCK : E_FORCE_UNLOCK);

	sndp_log_debug_func("end\n");
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
	u_long	ulSetDevice = MAX98090_DEV_NONE;


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

	/* stop MAXIM */
	if (NULL != g_sndp_dai_func.maxim_shutdown) {
		sndp_log_debug("maxim_dai_shutdown\n");
		g_sndp_dai_func.maxim_shutdown(
				g_sndp_main[SNDP_PCM_OUT].arg.maxim_dai,
				iInValue);
	}
#if 0
	/* Disable the power domain */
	iRet = pm_runtime_put_sync(g_sndp_power_domain);
	if (ERROR_NONE != iRet)
		sndp_log_debug("modules power off iRet=%d\n", iRet);
#endif
	/* Trigger stop control flag update */
	g_sndp_stop_trigger_condition[SNDP_PCM_IN] &= ~SNDP_STOP_TRIGGER_VOICE;

	/* Return from the waiting of the shutdown */
	wake_up_interruptible(&g_sndp_stop_wait);

	/*
	 * A process similar to sndp_work_normal_dev_chg()
	 */

	/* MAXIM device change */
	ulSetDevice = sndp_get_next_devices(iOutValue);
	iRet = max98090_set_device(ulSetDevice);
	if (ERROR_NONE != iRet)
		sndp_log_err("maxim set device error (code=%d)\n", iRet);

	/* Wake Force Unlock */
	sndp_wake_lock(E_FORCE_UNLOCK);

	sndp_log_debug_func("end\n");
}


#ifdef SOUND_TEST

#define SYSC_PHY_BASE	(0xE6180000)
#define SYSC_REG_MAX	(0x0084)

/* SYSC base address */
u_long g_sysc_Base;

#define SYSC_SPDCR	(g_sysc_Base + 0x0008)
#define SYSC_SWUCR	(g_sysc_Base + 0x0014)
#define SYSC_PSTR	(g_sysc_Base + 0x0080)


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


