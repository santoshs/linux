/* max98090_extern.h
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

/*!
  @file max98090_extern.h

  @brief Public definition Audio LSI driver header file.
*/

#ifndef __MAX98090_EXTERN_H__
#define __MAX98090_EXTERN_H__

/*---------------------------------------------------------------------------*/
/* include files                                                             */
/*---------------------------------------------------------------------------*/
#ifdef __MAX98090_UT__
/* none. */
#else	/* __MAX98090_UT__ */
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <asm/irq.h>
#endif	/* __MAX98090_UT__ */

/*---------------------------------------------------------------------------*/
/* typedef declaration                                                       */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* define macro declaration                                                  */
/*---------------------------------------------------------------------------*/
#define __MAX98090_TODO__   /* todo */

#ifdef __MAX98090_TODO__
#define __MAX98090_TODO_POWER__
/* #define __MAX98090_DEBUG__ */
/* #define __MAX98090_RELEASE_CHECK__ */
#endif  /* __MAX98090_TODO__ */

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
enum MAX98090_DEV_VAL {
	MAX98090_DEV_NONE			= 0x00000000,
	/**< device none. */
	MAX98090_DEV_PLAYBACK_SPEAKER		= 0x00000001,
	/**< playback speaker. */
	MAX98090_DEV_PLAYBACK_EARPIECE		= 0x00000002,
	/**< playback earpiece. */
	MAX98090_DEV_PLAYBACK_HEADPHONES	= 0x00000004,
	/**< playback headphones or headset. */
	MAX98090_DEV_CAPTURE_MIC		= 0x00000008,
	/**< capture mic. */
	MAX98090_DEV_CAPTURE_HEADSET_MIC	= 0x00000010,
	/**< capture headset mic. */
	MAX98090_DEV_MAX
};

/*!
  @brief device volume value.
*/
enum MAX98090_VOLUMEL_VAL {
	MAX98090_VOLUMEL0 = 0,	/**< volume level 0. */
	MAX98090_VOLUMEL1 = 5,	/**< volume level 1. */
	MAX98090_VOLUMEL2 = 10,	/**< volume level 2. */
	MAX98090_VOLUMEL3 = 15,	/**< volume level 3. */
	MAX98090_VOLUMEL4 = 20,	/**< volume level 4. */
	MAX98090_VOLUMEL5 = 25,	/**< volume level 5. */
	MAX98090_VOLUMEL_MAX
};

/*!
  @brief device mute value.
*/
enum MAX98090_MUTE_VAL {
	MAX98090_MUTE_DISABLE = 0,	/**< mute disable. */
	MAX98090_MUTE_ENABLE		/**< mute enable. */
};

/*!
  @brief Audio IC ID value.
*/
enum MAX98090_AUDIO_IC_VAL {
	MAX98090_AUDIO_IC_MAX98090 = 0,	/**< max98090. */
	MAX98090_AUDIO_IC_MAX97236,	/**< max97236. */
	MAX98090_AUDIO_IC_ALL		/**< max98090 and max97236. */
};

/*!
  @brief interrupt factor value.
*/
enum MAX98090_INT_FACTOR_VAL {
	MAX98090_INT_FACTOR_NONE = 0,		/**< none. */
	MAX98090_INT_FACTOR_JACK_CONNECTED,	/**< jack connected. */
	MAX98090_INT_FACTOR_JACK_REMOVEED,	/**< jack removed. */
	MAX98090_INT_FACTOR_KEY_PRESSED,	/**< key pressed. */
	MAX98090_INT_FACTOR_KEY_RELEASED	/**< key released. */
};

/*!
  @brief speaker amplifiers setting value.
*/
enum MAX98090_SPEAKER_AMP_VAL {
	MAX98090_SPEAKER_AMP_DISABLE = 0,	/**< speaker amp disable. */
	MAX98090_SPEAKER_AMP_ENABLE		/**< speaker amp enable. */
};

/*---------------------------------------------------------------------------*/
/* structure declaration                                                     */
/*---------------------------------------------------------------------------*/
/*!
  @brief structure to register a callback function.
*/
struct max98090_callback_func {
	void (*irq_func)(void);	/**< IRQ callback function. */
};

/*---------------------------------------------------------------------------*/
/* extern variable declaration                                               */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* extern function declaration                                               */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* prototype declaration                                                     */
/*---------------------------------------------------------------------------*/
/*------------------------------------*/
/* for system                         */
/*------------------------------------*/
/*!
  @brief AudioLSI module init.

  @param none.

  @return function results.
*/
int __init max98090_init(void);

/*!
  @brief AudioLSI module exit.

  @param none.

  @return none
*/
void __exit max98090_exit(void);

/*------------------------------------*/
/* for sound driver                   */
/*------------------------------------*/
/*!
  @brief set device.

  @param[i] device  value of device ID.

  @return function results.

  @see MAX98090_DEV_VAL.
*/
int max98090_set_device(const u_long device);

/*!
  @brief get device setting.

  @param[o] device  value of device ID.

  @return function results.

  @see MAX98090_DEV_VAL.
*/
int max98090_get_device(u_long *device);

/*!
  @brief set volume.

  @param[i] device  value of device ID.
  @param[i] volume  value of volume.

  @return function results.

  @see MAX98090_DEV_VAL, MAX98090_VOLUMEL_VAL.
*/
int max98090_set_volume(const u_long device, const u_int volume);

/*!
  @brief get volume setting.

  @param[i] device  value of device ID.
  @param[o] volume  value of volume.

  @return function results.

  @see MAX98090_DEV_VAL, MAX98090_VOLUMEL_VAL.
*/
int max98090_get_volume(const u_long device, u_int *volume);

/*!
  @brief set mute setting.

  @param[i] mute  value of mute setting.

  @return function results.

  @see MAX98090_MUTE_VAL.
*/
int max98090_set_mute(const u_int mute);

/*!
  @brief get mute setting.

  @param[o] mute  value of mute setting.

  @return function results.

  @see MAX98090_MUTE_VAL.
*/
int max98090_get_mute(u_int *mute);

/*!
  @brief set speaker amplifiers setting value.

  @param[i] value  speaker amplifiers setting value.

  @return function results.

  @see MAX98090_SPEAKER_AMP_VAL.
*/
int max98090_set_speaker_amp(const u_int value);

/*!
  @brief get speaker amplifiers setting value.

  @param[o] value  speaker amplifiers setting value.

  @return function results.

  @see MAX98090_SPEAKER_AMP_VAL.
*/
int max98090_get_speaker_amp(u_int *value);

/*!
  @brief get interrupt status.

  @param[o] irq_status  value of interrupt status.

  @return function results.

  @see MAX98090_INT_FACTOR_VAL.
*/
int max98090_get_status(u_long *irq_status);

/*!
  @brief register callback function.

  @param[i] callback  structure to register a callback function.

  @return function results.
*/
int max98090_register_callback_func(struct max98090_callback_func *callback_);

/*!
  @brief dump registers.

  @param[i] dump_type  Audio IC ID value.

  @return function results.

  @see MAX98090_AUDIO_IC_VAL.
*/
int max98090_dump_registers(const u_int dump_type);

/*!
  @brief read audio register.

  @param[i] audio_ic  read register device.
  @param[i] addr_id  read register address.
  @param[o] value  read register value.

  @return function results.
*/
int max98090_read(const u_int audio_ic, const u_int addr_id, int *value);

/*!
  @brief write audio register.

  @param[i] audio_ic  write register device.
  @param[i] addr_id  write register address.
  @param[o] value  write register value.

  @return function results.
*/
int max98090_write(const u_int audio_ic, const u_int addr_id,
		const u_int value);

/*---------------------------------------------------------------------------*/
/* inline function implementation                                            */
/*---------------------------------------------------------------------------*/
/* none */

#endif  /* __MAX98090_EXTERN_H__ */
