/* wm1811_extern.h
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
  @file wm1811_extern.h

  @brief Public definition Audio LSI driver header file.
*/

#ifndef __WM1811_EXTERN_H__
#define __WM1811_EXTERN_H__

/*---------------------------------------------------------------------------*/
/* include files                                                             */
/*---------------------------------------------------------------------------*/
#ifdef __WM1811_UT__
/* none. */
#else	/* __WM1811_UT__ */
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <asm/irq.h>
#endif	/* __WM1811_UT__ */

/*---------------------------------------------------------------------------*/
/* typedef declaration                                                       */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* define macro declaration                                                  */
/*---------------------------------------------------------------------------*/
#define __WM1811_TODO__   /* todo */

#ifdef __WM1811_TODO__
/* #define __WM1811_DEBUG__ */
#endif  /* __WM1811_TODO__ */

#define WM1811_VOLUMEL_STEP	5

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
enum WM1811_DEV_VAL {
	WM1811_DEV_NONE			= 0x00000000,
	/**< device none. */
	WM1811_DEV_PLAYBACK_SPEAKER	= 0x00000001,
	/**< playback speaker. */
	WM1811_DEV_PLAYBACK_EARPIECE	= 0x00000002,
	/**< playback earpiece. */
	WM1811_DEV_PLAYBACK_HEADPHONES	= 0x00000004,
	/**< playback headphones or headset. */
	WM1811_DEV_CAPTURE_MIC		= 0x00000008,
	/**< capture mic. */
	WM1811_DEV_CAPTURE_HEADSET_MIC	= 0x00000010,
	/**< capture headset mic. */
	WM1811_DEV_MAX
};

/*!
  @brief device volume value.
*/
enum WM1811_VOLUMEL_VAL {
	WM1811_VOLUMEL0 = 0,	/**< volume level 0. */
	WM1811_VOLUMEL1 = WM1811_VOLUMEL0 + WM1811_VOLUMEL_STEP,
	/**< volume level 1. */
	WM1811_VOLUMEL2 = WM1811_VOLUMEL1 + WM1811_VOLUMEL_STEP,
	/**< volume level 2. */
	WM1811_VOLUMEL3 = WM1811_VOLUMEL2 + WM1811_VOLUMEL_STEP,
	/**< volume level 3. */
	WM1811_VOLUMEL4 = WM1811_VOLUMEL3 + WM1811_VOLUMEL_STEP,
	/**< volume level 4. */
	WM1811_VOLUMEL5 = WM1811_VOLUMEL4 + WM1811_VOLUMEL_STEP,
	/**< volume level 5. */
	WM1811_VOLUMEL_MAX = WM1811_VOLUMEL5 + WM1811_VOLUMEL_STEP
};

/*!
  @brief device mute value.
*/
enum WM1811_MUTE_VAL {
	WM1811_MUTE_DISABLE = 0,	/**< mute disable. */
	WM1811_MUTE_ENABLE		/**< mute enable. */
};

/*!
  @brief speaker amplifiers setting value.
*/
enum WM1811_SPEAKER_AMP_VAL {
	WM1811_SPEAKER_AMP_DISABLE = 0,	/**< speaker amp disable. */
	WM1811_SPEAKER_AMP_ENABLE	/**< speaker amp enable. */
};

/*---------------------------------------------------------------------------*/
/* structure declaration                                                     */
/*---------------------------------------------------------------------------*/
/*!
  @brief structure to register a callback function.
*/
struct wm1811_callback_func {
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
int __init wm1811_init(void);

/*!
  @brief AudioLSI module exit.

  @param none.

  @return none
*/
void __exit wm1811_exit(void);

/*------------------------------------*/
/* for sound driver                   */
/*------------------------------------*/
/*!
  @brief set device.

  @param[i] device     value of device ID.
  @param[i] pcm_value  value of pcm.

  @return function results.

  @see WM1811_DEV_VAL.
*/
int wm1811_set_device(const u_long device, const u_int pcm_value);

/*!
  @brief get device setting.

  @param[o] device  value of device ID.

  @return function results.

  @see WM1811_DEV_VAL.
*/
int wm1811_get_device(u_long *device);

/*!
  @brief set volume.

  @param[i] device  value of device ID.
  @param[i] volume  value of volume.

  @return function results.

  @see WM1811_DEV_VAL, WM1811_VOLUMEL_VAL.
*/
int wm1811_set_volume(const u_long device, const u_int volume);

/*!
  @brief get volume setting.

  @param[i] device  value of device ID.
  @param[o] volume  value of volume.

  @return function results.

  @see WM1811_DEV_VAL, WM1811_VOLUMEL_VAL.
*/
int wm1811_get_volume(const u_long device, u_int *volume);

/*!
  @brief set mute setting.

  @param[i] mute  value of mute setting.

  @return function results.

  @see WM1811_MUTE_VAL.
*/
int wm1811_set_mute(const u_int mute);

/*!
  @brief get mute setting.

  @param[o] mute  value of mute setting.

  @return function results.

  @see WM1811_MUTE_VAL.
*/
int wm1811_get_mute(u_int *mute);

/*!
  @brief set speaker amplifiers setting value.

  @param[i] value  speaker amplifiers setting value.

  @return function results.

  @see WM1811_SPEAKER_AMP_VAL.
*/
int wm1811_set_speaker_amp(const u_int value);

/*!
  @brief get speaker amplifiers setting value.

  @param[o] value  speaker amplifiers setting value.

  @return function results.

  @see WM1811_SPEAKER_AMP_VAL.
*/
int wm1811_get_speaker_amp(u_int *value);

/*!
  @brief get interrupt status.

  @param[o] irq_status  value of interrupt status.

  @return function results.

  @see WM1811_INT_FACTOR_VAL.
*/
int wm1811_get_status(u_long *irq_status);

/*!
  @brief register callback function.

  @param[i] callback  structure to register a callback function.

  @return function results.
*/
int wm1811_register_callback_func(struct wm1811_callback_func *callback_);

/*!
  @brief dump registers.

  @param none.

  @return function results.

*/
int wm1811_dump_registers(void);

/*!
  @brief read audio register.

  @param[i] addr  read register address.
  @param[o] value  read register value.

  @return function results.
*/
int wm1811_read(const u_short addr, u_short *value);

/*!
  @brief write audio register.

  @param[i] addr  write register address.
  @param[o] value  write register value.

  @return function results.
*/
int wm1811_write(const u_short addr, const u_short value);

/*---------------------------------------------------------------------------*/
/* inline function implementation                                            */
/*---------------------------------------------------------------------------*/
/* none */

#endif  /* __WM1811_EXTERN_H__ */
