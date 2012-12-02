/*
 * d2153_extern.h -- Public interface definitions for D2153 Audio Codec
 * Based on wm1811_extern.h header file.
 *
 * Copyright (c) 2012 Dialog Semiconductor
 *
 * Author: Adam Thomson <Adam.Thomson@diasemi.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __D2153_EXTERN_H__
#define __D2153_EXTERN_H__

/*---------------------------------------------------------------------------*/
/* include files                                                             */
/*---------------------------------------------------------------------------*/
#ifdef __D2153_UT__
/* none. */
#else	/* __D2153_UT__ */
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <asm/irq.h>
#endif	/* __D2153_UT__ */

/*---------------------------------------------------------------------------*/
/* typedef declaration                                                       */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* define macro declaration                                                  */
/*---------------------------------------------------------------------------*/
#define __D2153_TODO__   /* todo */

#ifdef __D2153_TODO__
/* #define __D2153_DEBUG__ */
#endif  /* __D2153_TODO__ */

#define D2153_VOLUMEL_STEP	5

/*---------------------------------------------------------------------------*/
/* define function macro declaration                                         */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* enum declaration                                                          */
/*---------------------------------------------------------------------------*/
/*!
  @brief device ID value.
*/
enum D2153_DEV_VAL {
	D2153_DEV_NONE			= 0x00000000,
	/**< device none. */
	D2153_DEV_PLAYBACK_SPEAKER	= 0x00000001,
	/**< playback speaker. */
	D2153_DEV_PLAYBACK_EARPIECE	= 0x00000002,
	/**< playback earpiece. */
	D2153_DEV_PLAYBACK_HEADPHONES	= 0x00000004,
	/**< playback headphones or headset. */
	D2153_DEV_CAPTURE_MIC		= 0x00000008,
	/**< capture mic. */
	D2153_DEV_CAPTURE_HEADSET_MIC	= 0x00000010,
	/**< capture headset mic. */
	D2153_DEV_MAX
};

/*!
  @brief device volume value.
*/
enum D2153_VOLUMEL_VAL {
	D2153_VOLUMEL0 = 0,	/**< volume level 0. */
	D2153_VOLUMEL1 = D2153_VOLUMEL0 + D2153_VOLUMEL_STEP,
	/**< volume level 1. */
	D2153_VOLUMEL2 = D2153_VOLUMEL1 + D2153_VOLUMEL_STEP,
	/**< volume level 2. */
	D2153_VOLUMEL3 = D2153_VOLUMEL2 + D2153_VOLUMEL_STEP,
	/**< volume level 3. */
	D2153_VOLUMEL4 = D2153_VOLUMEL3 + D2153_VOLUMEL_STEP,
	/**< volume level 4. */
	D2153_VOLUMEL5 = D2153_VOLUMEL4 + D2153_VOLUMEL_STEP,
	/**< volume level 5. */
	D2153_VOLUMEL_MAX = D2153_VOLUMEL5 + D2153_VOLUMEL_STEP
};

/*!
  @brief device mute value.
*/
enum D2153_MUTE_VAL {
	D2153_MUTE_DISABLE = 0,	/**< mute disable. */
	D2153_MUTE_ENABLE		/**< mute enable. */
};

/*!
  @brief speaker amplifiers setting value.
*/
enum D2153_SPEAKER_AMP_VAL {
	D2153_SPEAKER_AMP_DISABLE = 0,	/**< speaker amp disable. */
	D2153_SPEAKER_AMP_ENABLE	/**< speaker amp enable. */
};

/*!
  @brief device power state value.
*/
enum D2153_POWER_VAL {
	D2153_POWER_OFF = 0,	/**< d2153 PowerOFF. */
	D2153_POWER_ON		/**< d2153 PowerON. */
};

/*---------------------------------------------------------------------------*/
/* prototype declaration                                                     */
/*---------------------------------------------------------------------------*/

/*------------------------------------*/
/* for sound driver                   */
/*------------------------------------*/
/*!
  @brief set device.

  @param[i] device     value of device ID.
  @param[i] pcm_value  value of pcm.
  @param[i] power      value of power state.

  @return function results.

  @see D2153_DEV_VAL.
*/
int d2153_set_device(const u_long device, const u_int pcm_value,
	u_int power);

/*!
  @brief get device setting.

  @param[o] device  value of device ID.

  @return function results.

  @see D2153_DEV_VAL.
*/
int d2153_get_device(u_long *device);

/*!
  @brief set volume.

  @param[i] device  value of device ID.
  @param[i] volume  value of volume.

  @return function results.

  @see D2153_DEV_VAL, D2153_VOLUMEL_VAL.
*/
int d2153_set_volume(const u_long device, const u_int volume);

/*!
  @brief get volume setting.

  @param[i] device  value of device ID.
  @param[o] volume  value of volume.

  @return function results.

  @see D2153_DEV_VAL, D2153_VOLUMEL_VAL.
*/
int d2153_get_volume(const u_long device, u_int *volume);

/*!
  @brief set mute setting.

  @param[i] mute  value of mute setting.

  @return function results.

  @see D2153_MUTE_VAL.
*/
int d2153_set_mute(const u_int mute);

/*!
  @brief get mute setting.

  @param[o] mute  value of mute setting.

  @return function results.

  @see D2153_MUTE_VAL.
*/
int d2153_get_mute(u_int *mute);

/*!
  @brief set speaker amplifiers setting value.

  @param[i] value  speaker amplifiers setting value.

  @return function results.

  @see D2153_SPEAKER_AMP_VAL.
*/
int d2153_set_speaker_amp(const u_int value);

/*!
  @brief get speaker amplifiers setting value.

  @param[o] value  speaker amplifiers setting value.

  @return function results.

  @see D2153_SPEAKER_AMP_VAL.
*/
int d2153_get_speaker_amp(u_int *value);

/*---------------------------------------------------------------------------*/
/* inline function implementation                                            */
/*---------------------------------------------------------------------------*/
/* none */

#endif  /* __D2153_EXTERN_H__ */
