/* soundpath.h
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


#ifndef __SOUNDPATH_H__
#define __SOUNDPATH_H__

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/wakelock.h>
#include <linux/wait.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>

/* PCM direction kind */
typedef enum {
	SNDP_PCM_OUT,			/* out(playback) */
	SNDP_PCM_IN,			/* in(capture)   */
	SNDP_PCM_DIRECTION_MAX,
} sndp_direction;			/* direction */

/* Wake lock kind */
typedef enum {
	E_LOCK = 0,			/* to Wake Lock   */
	E_UNLOCK,			/* to Wake Unlock */
	E_FORCE_UNLOCK,			/* to Wake Unlock Forced */
} sndp_wake_lock_kind;

#ifndef __RC5T7316_CTRL_NO_EXTERN__

#ifdef __SOUNDPATHLOGICAL_NO_EXTERN__
#define SOUNDPATHLOGICAL_NO_EXTERN
#else
#define SOUNDPATHLOGICAL_NO_EXTERN			extern
#endif

SOUNDPATHLOGICAL_NO_EXTERN	int sndp_init(struct snd_soc_dai_driver *fsi_port_dai_driver,
										struct snd_soc_dai_driver *max98090_dai_driver,
										struct snd_soc_platform_driver *fsi_soc_platform);

SOUNDPATHLOGICAL_NO_EXTERN	void sndp_exit(void);
SOUNDPATHLOGICAL_NO_EXTERN	void sndp_work_portb_start(struct work_struct *work);

SOUNDPATHLOGICAL_NO_EXTERN	void sndp_set_portb_start(bool flag);

SOUNDPATHLOGICAL_NO_EXTERN	void sndp_wake_lock(const sndp_wake_lock_kind kind);

SOUNDPATHLOGICAL_NO_EXTERN	struct workqueue_struct		*g_sndp_queue_main;
SOUNDPATHLOGICAL_NO_EXTERN	wait_queue_head_t			g_sndp_stop_wait;

SOUNDPATHLOGICAL_NO_EXTERN	struct wake_lock g_sndp_wake_lock_idle;
SOUNDPATHLOGICAL_NO_EXTERN	struct wake_lock g_sndp_wake_lock_suspend;

SOUNDPATHLOGICAL_NO_EXTERN	u_int g_sndp_log_level;
SOUNDPATHLOGICAL_NO_EXTERN	u_int g_sndp_mode;

#endif	/* != __RC5T7316_CTRL_NO_EXTERN__ */

/* start : define for debug */
#ifndef DEBUG
#define DEBUG
#endif

#define DEBUG_FUNC
#define __PRN_SNDP__
#define __PRN_ADD_TIME__
/* #define __SNDP_ROUTE_DEBUG__ */

/* end : define for debug */

#define SNDP_DRV_NAME		"sndp"
#define LOG_LEVEL			"log_level"
#define CALL_STATUS_SWITCH	"call_status"
#define SNDP_REG_DUMP       "reg_dump"

#define LOG_NO_PRINT						(0x00)
#define LOG_ERR_PRINT						(0x01)
#define LOG_PROC_PRINT						(0x02)
#define LOG_DEBUG_PRINT						(0x03)
#define LOG_FUNC_PRINT						(0x04)

#define LOG_BIT_REG_DUMP					(0x10)
#define LOG_BIT_DMESG						(0x80)

#define LOG_LEVEL_MAX						(0xffffffff)
#define LOG_BYTE_LOW(sw)					((sw) & 0x0000000f)

#define GET_PROCESS_TIME(tv)				do_gettimeofday(&tv)

#define REG_DUMP_ALL						(0x00)
#define REG_DUMP_MAXIM						(0x01)
#define REG_DUMP_FSI						(0x02)
#define REG_DUMP_CLKGEN						(0x04)
#define REG_DUMP_SCUW						(0x08)

/* #define SOUND_TEST */
#define NO_INTURRUPT
#ifdef SOUND_TEST
SOUNDPATHLOGICAL_NO_EXTERN	void sndp_path_test_pm_runtime_get_sync(void);
SOUNDPATHLOGICAL_NO_EXTERN	void sndp_path_test_pm_runtime_put_sync(void);
SOUNDPATHLOGICAL_NO_EXTERN	void sndp_path_test_sndp_init(void);
#endif /* SOUND_TEST */

#ifdef __PRN_SNDP__

#define sndp_log_reg_dump(fmt, ...)																					\
do {																												\
	if (g_sndp_log_level & LOG_BIT_REG_DUMP) {																		\
		(g_sndp_log_level & LOG_BIT_DMESG) ? pr_err(fmt, ##__VA_ARGS__) : pr_alert(fmt, ##__VA_ARGS__);				\
	}																												\
} while (0)

#ifdef __PRN_ADD_TIME__

#define sndp_log_ver(fmt, ...)																						\
do {																												\
	struct timeval tv;																								\
	if (LOG_ERR_PRINT <= LOG_BYTE_LOW(g_sndp_log_level)) {															\
		GET_PROCESS_TIME(tv);																						\
		if (g_sndp_log_level & LOG_BIT_DMESG) {																		\
			pr_err("[%5ld.%06ld] " SNDP_DRV_NAME " : Version " fmt, tv.tv_sec, tv.tv_usec, ##__VA_ARGS__);			\
		} else {																									\
			pr_alert("[%5ld.%06ld] " SNDP_DRV_NAME " : Version " fmt, tv.tv_sec, tv.tv_usec, ##__VA_ARGS__);		\
		}																											\
	}																												\
} while (0)

#define sndp_log_err(fmt, ...)																						\
do {																												\
	struct timeval tv;																								\
	if (LOG_ERR_PRINT <= LOG_BYTE_LOW(g_sndp_log_level)) {															\
		GET_PROCESS_TIME(tv);																						\
		if (g_sndp_log_level & LOG_BIT_DMESG) {																		\
			pr_err("[%5ld.%06ld] " SNDP_DRV_NAME " : %s():" fmt, tv.tv_sec, tv.tv_usec, __func__, ##__VA_ARGS__);	\
		} else {																									\
			pr_alert("[%5ld.%06ld] " SNDP_DRV_NAME " : %s(): " fmt, tv.tv_sec, tv.tv_usec, __func__, ##__VA_ARGS__);\
		}																											\
	}																												\
} while (0)

#define sndp_log_info(fmt, ...)																						\
do {																												\
	struct timeval tv;																								\
	if (LOG_PROC_PRINT <= LOG_BYTE_LOW(g_sndp_log_level)) {															\
		GET_PROCESS_TIME(tv);																						\
		if (g_sndp_log_level & LOG_BIT_DMESG) {																		\
			pr_err("[%5ld.%06ld] " SNDP_DRV_NAME " : %s(): " fmt, tv.tv_sec, tv.tv_usec, __func__, ##__VA_ARGS__);	\
		} else {																									\
			pr_alert("[%5ld.%06ld] " SNDP_DRV_NAME " : %s(): " fmt, tv.tv_sec, tv.tv_usec, __func__, ##__VA_ARGS__);\
		}																											\
	}																												\
} while (0)

#define sndp_log_debug(fmt, ...)																					\
do {																												\
	struct timeval tv;																								\
	if (LOG_DEBUG_PRINT <= LOG_BYTE_LOW(g_sndp_log_level)) {														\
		GET_PROCESS_TIME(tv);																						\
		if (g_sndp_log_level & LOG_BIT_DMESG) {																		\
			pr_err("[%5ld.%06ld] " SNDP_DRV_NAME " : %s(): " fmt, tv.tv_sec, tv.tv_usec, __func__, ##__VA_ARGS__);	\
		} else {																									\
			pr_alert("[%5ld.%06ld] " SNDP_DRV_NAME " : %s(): " fmt, tv.tv_sec, tv.tv_usec, __func__, ##__VA_ARGS__);\
		}																											\
	}																												\
} while (0)

#ifdef DEBUG_FUNC

#define sndp_log_debug_func(fmt, ...)																				\
do {																												\
	struct timeval tv;																								\
	if (LOG_FUNC_PRINT <= LOG_BYTE_LOW(g_sndp_log_level)) {															\
		GET_PROCESS_TIME(tv);																						\
		if (g_sndp_log_level & LOG_BIT_DMESG) {																		\
			pr_err("[%5ld.%06ld] " SNDP_DRV_NAME " : %s(): " fmt, tv.tv_sec, tv.tv_usec, __func__, ##__VA_ARGS__);	\
		} else {																									\
			pr_alert("[%5ld.%06ld] " SNDP_DRV_NAME " : %s(): " fmt, tv.tv_sec, tv.tv_usec, __func__, ##__VA_ARGS__);\
		}																											\
	}																												\
} while (0)

#else	/* != DEBUG_FUNC */

#define sndp_log_debug_func(fmt, ...)		do { } while (0)

#endif	/* DEBUG_FUNC */

#else	/* != __PRN_ADD_TIME__ */

#define sndp_log_ver(fmt, ...)																						\
do {																												\
	if (LOG_ERR_PRINT <= LOG_BYTE_LOW(g_sndp_log_level)) {															\
		if (g_sndp_log_level & LOG_BIT_DMESG) {																		\
			pr_err(SNDP_DRV_NAME " : Version " fmt, ##__VA_ARGS__);													\
		} else {																									\
			pr_alert(SNDP_DRV_NAME " : Version " fmt, ##__VA_ARGS__);												\
		}																											\
	}																												\
} while (0)

#define sndp_log_err(fmt, ...)																						\
do {																												\
	if (LOG_ERR_PRINT <= LOG_BYTE_LOW(g_sndp_log_level)) {															\
		if (g_sndp_log_level & LOG_BIT_DMESG) {																		\
			pr_err(SNDP_DRV_NAME " : %s():" fmt, __func__, ##__VA_ARGS__);											\
		} else {																									\
			pr_alert(SNDP_DRV_NAME " : %s(): " fmt, __func__, ##__VA_ARGS__);										\
		}																											\
	}																												\
} while (0)

#define sndp_log_info(fmt, ...)																						\
do {																												\
	if (LOG_PROC_PRINT <= LOG_BYTE_LOW(g_sndp_log_level)) {															\
		if (g_sndp_log_level & LOG_BIT_DMESG) {																		\
			pr_err(SNDP_DRV_NAME " : %s(): " fmt, __func__, ##__VA_ARGS__);											\
		} else {																									\
			pr_alert(SNDP_DRV_NAME " : %s(): " fmt, __func__, ##__VA_ARGS__);										\
		}																											\
	}																												\
} while (0)

#define sndp_log_debug(fmt, ...)																					\
do {																												\
	if (LOG_DEBUG_PRINT <= LOG_BYTE_LOW(g_sndp_log_level)) {														\
		if (g_sndp_log_level & LOG_BIT_DMESG) {																		\
			pr_err(SNDP_DRV_NAME " : %s(): " fmt, __func__, ##__VA_ARGS__);											\
		} else {																									\
			pr_alert(SNDP_DRV_NAME " : %s(): " fmt, __func__, ##__VA_ARGS__);										\
		}																											\
	}																												\
} while (0)

#define sndp_log_dump(fmt, ...)																						\
	(g_sndp_log_level & LOG_BIT_DMESG) ? pr_err(fmt, ##__VA_ARGS__) : pr_alert(fmt, ##__VA_ARGS__)

#ifdef DEBUG_FUNC

#define sndp_log_debug_func(fmt, ...)																				\
do {																												\
	if (LOG_FUNC_PRINT <= LOG_BYTE_LOW(g_sndp_log_level)) {															\
		if (g_sndp_log_level & LOG_BIT_DMESG) {																		\
			pr_err(SNDP_DRV_NAME " : %s(): " fmt, __func__, ##__VA_ARGS__);											\
		} else {																									\
			pr_alert(SNDP_DRV_NAME " : %s(): " fmt, __func__, ##__VA_ARGS__);										\
		}																											\
	}																												\
} while (0)

#else	/* != DEBUG_FUNC */

#define sndp_log_debug_func(fmt, ...)		do { } while (0)

#endif	/* DEBUG_FUNC */

#endif	/* __PRN_ADD_TIME__ */

#else /* != __PRN_SNDP__ */

#define sndp_log_ver(fmt, ...)				do { } while (0)
#define sndp_log_err(fmt, ...)				do { } while (0)
#define sndp_log_info(fmt, ...)				do { } while (0)
#define sndp_log_debug(fmt, ...)			do { } while (0)
#define sndp_log_debug_func(fmt, ...)		do { } while (0)
#define sndp_log_reg_dump(fmt, ...)			do { } while (0)
#define sndp_log_dump(fmt, ...)				do { } while (0)

#endif	/* __PRN_SNDP__ */

#ifdef __PRN_ADD_TIME__

#define sndp_log_always_err(fmt, ...)																				\
do {																												\
	struct timeval tv;																								\
	GET_PROCESS_TIME(tv);																							\
	pr_alert("[%5ld.%06ld] " SNDP_DRV_NAME " : %s(): " fmt, tv.tv_sec, tv.tv_usec, __func__, ##__VA_ARGS__);		\
} while (0)

#else	/* != __PRN_ADD_TIME__ */

#define sndp_log_always_err(fmt, ...)																				\
	pr_alert(SNDP_DRV_NAME " : %s(): " fmt, __func__, ##__VA_ARGS__)

#endif	/* __PRN_ADD_TIME__ */

/*
 * Constant definitions
 */
/* On/Off */
#define ERROR_NONE							(0)

#define SNDP_ON								(1)
#define SNDP_OFF							(0)

/* Start/Stop */
#define SNDP_START							(1)
#define SNDP_STOP							(0)

#define STAT_OFF							(0)
#define STAT_ON								(1)

typedef enum {
	SNDP_HW_CLKGEN,
	SNDP_HW_FSI,
	SNDP_HW_SCUW,
} sndp_hw_audio;


typedef enum {
	SNDP_RC5T7316_INCALL_3G,
	SNDP_RC5T7316_INCALL_2G,
	SNDP_RC5T7316_INCALL_MAX,
} sndp_incall_kind;


/**
 ** ************************************************** *
 ***  Struct declaration                              **
 ** ************************************************** *
 */
/* Function table */
typedef struct {
	u_int		uiValue;
	void		(*func)(const u_int);
} ctrl_func_tbl_t;

typedef enum {
	SNDP_NO_DEVICE = 0,
	SNDP_SPEAKER = 0x1,			/* SPEAKER */
	SNDP_WIREDHEADSET = 0x2,		/* WIRED_HEADSET */
	SNDP_EARPIECE = 0x4,			/* EARPIECE */
	SNDP_BLUETOOTHSCO = 0x8,		/* BLUETOOTH_SCO */
	SNDP_AUXDIGITAL = 0x10,			/* AUX_DIGITAL(HDMI:Port C) */
	SNDP_BUILTIN_MIC = 0x20,		/* MIC */
	SNDP_WIREDHEADPHONE = 0x40,		/* WIRED_HEADPHONE */
} sndp_device_type;				/* device type */

/* Mode types */
typedef enum {
	SNDP_MODE_NORMAL,			/* MODE_NORMAL */
	SNDP_MODE_RING,				/* MODE_RINGTONE */
	SNDP_MODE_INCALL,			/* MODE_IN_CALL */
	SNDP_MODE_INCOMM,			/* MODE_IN_COMMUNICATION */
	SNDP_MODE_INIT,				/* init */
	SNDP_MODE_MAX,
} sndp_mode_type;				/* mode type */

#define SNDP_DIRECTION_BIT			(20)
#define SNDP_DEVICE_BIT				(4)
#define SNDP_MODE_BIT				(0)
#define SNDP_VALUE_INIT				(0xff000000 + SNDP_MODE_INIT)

typedef enum {
	/* output devices */
	SNDP_OUT_EARPIECE = 0x1,
	SNDP_OUT_SPEAKER = 0x2,
	SNDP_OUT_WIRED_HEADSET = 0x4,
	SNDP_OUT_WIRED_HEADPHONE = 0x8,
	SNDP_OUT_BLUETOOTH_SCO = 0x10,
	SNDP_OUT_BLUETOOTH_SCO_HEADSET = 0x20,
	SNDP_OUT_BLUETOOTH_SCO_CARKIT = 0x40,
	SNDP_OUT_BLUETOOTH_A2DP = 0x80,
	SNDP_OUT_BLUETOOTH_A2DP_HEADPHONES = 0x100,
	SNDP_OUT_BLUETOOTH_A2DP_SPEAKER = 0x200,
	SNDP_OUT_AUX_DIGITAL = 0x400,
	SNDP_OUT_UPLINK = 0x800,
	/* input devices */
	SNDP_IN_COMMUNICATION = 0x10000,
	SNDP_IN_AMBIENT = 0x20000,
	SNDP_IN_BUILTIN_MIC = 0x40000,
	SNDP_IN_BLUETOOTH_SCO_HEADSET = 0x80000,
	SNDP_IN_WIRED_HEADSET = 0x100000,
	SNDP_IN_AUX_DIGITAL = 0x200000,
	SNDP_IN_VOICE_CALL = 0x400000,
	SNDP_IN_BACK_MIC = 0x800000,
} sndp_audio_devices;

#define SNDP_GET_DIRECTION_VAL(val)				\
	((u_int)(val) >= ((u_int)1 << SNDP_DIRECTION_BIT))

#define SNDP_GET_AUDIO_DEVICE(val)				\
	((u_int)(val) >> SNDP_DEVICE_BIT)

static inline u_int SNDP_GET_DEVICE_VAL(u_int val)
{
	u_int		audio_val = SNDP_GET_AUDIO_DEVICE(val);
	u_int		ret = SNDP_NO_DEVICE;

	if (audio_val & SNDP_OUT_SPEAKER)
		ret |= SNDP_SPEAKER;

	if (audio_val & SNDP_OUT_EARPIECE)
		ret |= SNDP_EARPIECE;

	if ((audio_val & SNDP_OUT_WIRED_HEADSET) || (audio_val & SNDP_IN_WIRED_HEADSET))
		ret |= SNDP_WIREDHEADSET;

	if ((audio_val & SNDP_OUT_BLUETOOTH_SCO) || (audio_val & SNDP_OUT_BLUETOOTH_SCO_HEADSET) ||
		(audio_val & SNDP_OUT_BLUETOOTH_SCO_CARKIT) || (audio_val & SNDP_IN_BLUETOOTH_SCO_HEADSET))
		ret |= SNDP_BLUETOOTHSCO;

	if ((audio_val & SNDP_OUT_AUX_DIGITAL) || (audio_val & SNDP_IN_AUX_DIGITAL))
		ret |= SNDP_AUXDIGITAL;

	if (audio_val & SNDP_IN_BUILTIN_MIC)
		ret |= SNDP_BUILTIN_MIC;

	if (audio_val & SNDP_OUT_WIRED_HEADPHONE)
		ret |= SNDP_WIREDHEADPHONE;
	return ret;
}

#define SNDP_GET_MODE_VAL(val)					\
	(((u_int)(val) & ~((0xffffffff >> SNDP_DEVICE_BIT) << SNDP_DEVICE_BIT)) >> SNDP_MODE_BIT)

#define SNDP_GET_VALUE(dev, mod)				\
	(((u_int)(dev) << SNDP_DEVICE_BIT) | ((u_int)(mod) << SNDP_MODE_BIT))

/* 0x00000020 */
#define SNDP_PLAYBACK_SPEAKER_NORMAL				\
	SNDP_GET_VALUE(SNDP_OUT_SPEAKER, SNDP_MODE_NORMAL)
/* 0x00000021 */
#define SNDP_PLAYBACK_SPEAKER_RINGTONE				\
	SNDP_GET_VALUE(SNDP_OUT_SPEAKER, SNDP_MODE_RING)
/* 0x00000022 */
#define SNDP_PLAYBACK_SPEAKER_INCALL				\
	SNDP_GET_VALUE(SNDP_OUT_SPEAKER, SNDP_MODE_INCALL)
/* 0x00000023 */
#define SNDP_PLAYBACK_SPEAKER_INCOMMUNICATION			\
	SNDP_GET_VALUE(SNDP_OUT_SPEAKER, SNDP_MODE_INCOMM)
/* 0x00000040 */
#define SNDP_PLAYBACK_HEADSET_NORMAL				\
	SNDP_GET_VALUE(SNDP_OUT_WIRED_HEADSET, SNDP_MODE_NORMAL)
/* 0x00000041 */
#define SNDP_PLAYBACK_HEADSET_RINGTONE				\
	SNDP_GET_VALUE(SNDP_OUT_WIRED_HEADSET, SNDP_MODE_RING)
/* 0x00000042 */
#define SNDP_PLAYBACK_HEADSET_INCALL				\
	SNDP_GET_VALUE(SNDP_OUT_WIRED_HEADSET, SNDP_MODE_INCALL)
/* 0x00000043 */
#define SNDP_PLAYBACK_HEADSET_INCOMMUNICATION			\
	SNDP_GET_VALUE(SNDP_OUT_WIRED_HEADSET, SNDP_MODE_INCOMM)
/* 0x00000080 */
#define SNDP_PLAYBACK_HEADPHONE_NORMAL				\
	SNDP_GET_VALUE(SNDP_OUT_WIRED_HEADPHONE, SNDP_MODE_NORMAL)
/* 0x00000081 */
#define SNDP_PLAYBACK_HEADPHONE_RINGTONE			\
	SNDP_GET_VALUE(SNDP_OUT_WIRED_HEADPHONE, SNDP_MODE_RING)
/* 0x00000082 */
#define SNDP_PLAYBACK_HEADPHONE_INCALL				\
	SNDP_GET_VALUE(SNDP_OUT_WIRED_HEADPHONE, SNDP_MODE_INCALL)
/* 0x00000083 */
#define SNDP_PLAYBACK_HEADPHONE_INCOMMUNICATION			\
	SNDP_GET_VALUE(SNDP_OUT_WIRED_HEADPHONE, SNDP_MODE_INCOMM)
/* 0x00000010 */
#define SNDP_PLAYBACK_EARPIECE_NORMAL				\
	SNDP_GET_VALUE(SNDP_OUT_EARPIECE, SNDP_MODE_NORMAL)
/* 0x00000011 */
#define SNDP_PLAYBACK_EARPIECE_RINGTONE				\
	SNDP_GET_VALUE(SNDP_OUT_EARPIECE, SNDP_MODE_RING)
/* 0x00000012 */
#define SNDP_PLAYBACK_EARPIECE_INCALL				\
	SNDP_GET_VALUE(SNDP_OUT_EARPIECE, SNDP_MODE_INCALL)
/* 0x00000013 */
#define SNDP_PLAYBACK_EARPIECE_INCOMMUNICATION			\
	SNDP_GET_VALUE(SNDP_OUT_EARPIECE, SNDP_MODE_INCOMM)
/* 0x00000100 */
#define SNDP_PLAYBACK_BLUETOOTH_NORMAL				\
	SNDP_GET_VALUE(SNDP_OUT_BLUETOOTH_SCO, SNDP_MODE_NORMAL)
/* 0x00000101 */
#define SNDP_PLAYBACK_BLUETOOTH_RINGTONE			\
	SNDP_GET_VALUE(SNDP_OUT_BLUETOOTH_SCO, SNDP_MODE_RING)
/* 0x00000102 */
#define SNDP_PLAYBACK_BLUETOOTH_INCALL				\
	SNDP_GET_VALUE(SNDP_OUT_BLUETOOTH_SCO, SNDP_MODE_INCALL)
/* 0x00000103 */
#define SNDP_PLAYBACK_BLUETOOTH_INCOMMUNICATION			\
	SNDP_GET_VALUE(SNDP_OUT_BLUETOOTH_SCO, SNDP_MODE_INCOMM)
/* 0x00004000 */
#define SNDP_PLAYBACK_AUXDIGITAL_NORMAL				\
	SNDP_GET_VALUE(SNDP_OUT_AUX_DIGITAL, SNDP_MODE_NORMAL)
/* 0x00004001 */
#define SNDP_PLAYBACK_AUXDIGITAL_RINGTONE			\
	SNDP_GET_VALUE(SNDP_OUT_AUX_DIGITAL, SNDP_MODE_RING)
/* 0x00004002 */
#define SNDP_PLAYBACK_AUXDIGITAL_INCALL				\
	SNDP_GET_VALUE(SNDP_OUT_AUX_DIGITAL, SNDP_MODE_INCALL)
/* 0x00004003 */
#define SNDP_PLAYBACK_AUXDIGITAL_INCOMMUNICATION		\
	SNDP_GET_VALUE(SNDP_OUT_AUX_DIGITAL, SNDP_MODE_INCOMM)
/* 0x01000000 */
#define SNDP_CAPTURE_HEADSET_NORMAL				\
	SNDP_GET_VALUE(SNDP_IN_WIRED_HEADSET, SNDP_MODE_NORMAL)
/* 0x01000001 */
#define SNDP_CAPTURE_HEADSET_RINGTONE				\
	SNDP_GET_VALUE(SNDP_IN_WIRED_HEADSET, SNDP_MODE_RING)
/* 0x01000002 */
#define SNDP_CAPTURE_HEADSET_INCALL				\
	SNDP_GET_VALUE(SNDP_IN_WIRED_HEADSET, SNDP_MODE_INCALL)
/* 0x01000003 */
#define SNDP_CAPTURE_HEADSET_INCOMMUNICATION			\
	SNDP_GET_VALUE(SNDP_IN_WIRED_HEADSET, SNDP_MODE_INCOMM)
/* 0x00800000 */
#define SNDP_CAPTURE_BLUETOOTH_NORMAL					\
	SNDP_GET_VALUE(SNDP_IN_BLUETOOTH_SCO_HEADSET, SNDP_MODE_NORMAL)
/* 0x00800001 */
#define SNDP_CAPTURE_BLUETOOTH_RINGTONE					\
	SNDP_GET_VALUE(SNDP_IN_BLUETOOTH_SCO_HEADSET, SNDP_MODE_RING)
/* 0x00800002 */
#define SNDP_CAPTURE_BLUETOOTH_INCALL					\
	SNDP_GET_VALUE(SNDP_IN_BLUETOOTH_SCO_HEADSET, SNDP_MODE_INCALL)
/* 0x00800003 */
#define SNDP_CAPTURE_BLUETOOTH_INCOMMUNICATION			\
	SNDP_GET_VALUE(SNDP_IN_BLUETOOTH_SCO_HEADSET, SNDP_MODE_INCOMM)
/* 0x00400000 */
#define SNDP_CAPTURE_MIC_NORMAL					\
	SNDP_GET_VALUE(SNDP_IN_BUILTIN_MIC, SNDP_MODE_NORMAL)
/* 0x00400001 */
#define SNDP_CAPTURE_MIC_RINGTONE				\
	SNDP_GET_VALUE(SNDP_IN_BUILTIN_MIC, SNDP_MODE_RING)
/* 0x00400002 */
#define SNDP_CAPTURE_MIC_INCALL					\
	SNDP_GET_VALUE(SNDP_IN_BUILTIN_MIC, SNDP_MODE_INCALL)
/* 0x00400003 */
#define SNDP_CAPTURE_MIC_INCOMMUNICATION			\
	SNDP_GET_VALUE(SNDP_IN_BUILTIN_MIC, SNDP_MODE_INCOMM)

/* 0x00000060 */
#define SNDP_PLAYBACK_SPEAKER_HEADSET_NORMAL			\
	SNDP_GET_VALUE((SNDP_OUT_SPEAKER | SNDP_OUT_WIRED_HEADSET), SNDP_MODE_NORMAL)
/* 0x00000061 */
#define SNDP_PLAYBACK_SPEAKER_HEADSET_RINGTONE			\
	SNDP_GET_VALUE((SNDP_OUT_SPEAKER | SNDP_OUT_WIRED_HEADSET), SNDP_MODE_RING)
/* 0x00000062 */
#define SNDP_PLAYBACK_SPEAKER_HEADSET_INCALL			\
	SNDP_GET_VALUE((SNDP_OUT_SPEAKER | SNDP_OUT_WIRED_HEADSET), SNDP_MODE_INCALL)
/* 0x00000063 */
#define SNDP_PLAYBACK_SPEAKER_HEADSET_INCOMMUNICATION	\
	SNDP_GET_VALUE((SNDP_OUT_SPEAKER | SNDP_OUT_WIRED_HEADSET), SNDP_MODE_INCOMM)

/* 0x000000a0 */
#define SNDP_PLAYBACK_SPEAKER_HEADPHONE_NORMAL			\
	SNDP_GET_VALUE((SNDP_OUT_SPEAKER | SNDP_OUT_WIRED_HEADPHONE), SNDP_MODE_NORMAL)
/* 0x000000a1 */
#define SNDP_PLAYBACK_SPEAKER_HEADPHONE_RINGTONE		\
	SNDP_GET_VALUE((SNDP_OUT_SPEAKER | SNDP_OUT_WIRED_HEADPHONE), SNDP_MODE_RING)
/* 0x000000a2 */
#define SNDP_PLAYBACK_SPEAKER_HEADPHONE_INCALL			\
	SNDP_GET_VALUE((SNDP_OUT_SPEAKER | SNDP_OUT_WIRED_HEADPHONE), SNDP_MODE_INCALL)
/* 0x000000a3 */
#define SNDP_PLAYBACK_SPEAKER_HEADPHONE_INCOMMUNICATION		\
	SNDP_GET_VALUE((SNDP_OUT_SPEAKER | SNDP_OUT_WIRED_HEADPHONE), SNDP_MODE_INCOMM)

/* 0x00008002 */
#define SNDP_PLAYBACK_UPLINK_INCALL				\
	SNDP_GET_VALUE(SNDP_OUT_UPLINK, SNDP_MODE_INCALL)

/* 0x04000002 */
#define SNDP_CAPTURE_VOICE_INCALL				\
	SNDP_GET_VALUE(SNDP_IN_VOICE_CALL, SNDP_MODE_INCALL)

#endif /* __SOUNDPATH_H__ */
