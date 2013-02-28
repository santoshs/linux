/* audio_test_wrap_wm1811.c
 *
 * Copyright (C) 2012-2013 Renesas Mobile Corp.
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
  @file		audio_test_wrap_wm1811.c

  @brief	Audio test command wrapper source file.
*/



/*---------------------------------------------------------------------------*/
/* include files                                                             */
/*---------------------------------------------------------------------------*/
#include <linux/module.h>

#include <sound/soundpath/TP/audio_test_extern.h>
#include "audio_test.h"

#include <sound/soundpath/soundpath.h>
#include <sound/soundpath/wm1811_extern.h>

/*---------------------------------------------------------------------------*/
/* typedef declaration (private)                                             */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* define macro declaration (private)                                        */
/*---------------------------------------------------------------------------*/
/*!
  @brief	Volume range.
*/
#define AUDIO_TEST_VOL_SPK_MIN		(0x00)	/**< -57dB. */
#define AUDIO_TEST_VOL_SPK_MAX		(0x3F)	/**< +6dB. */
#define AUDIO_TEST_VOL_HP_MIN		(0x00)	/**< -57dB. */
#define AUDIO_TEST_VOL_HP_MAX		(0x3F)	/**< +6dB. */
#define AUDIO_TEST_VOL_RCV_MIN		(0x00)	/**< 0dB. */
#define AUDIO_TEST_VOL_RCV_MAX		(0x01)	/**< -6dB. */
#define AUDIO_TEST_VOLUME0		(0)	/**< Volume level 0. */
#define AUDIO_TEST_VOLUME1		(1)	/**< Volume level 1. */
#define AUDIO_TEST_VOLUME2		(2)	/**< Volume level 2. */
#define AUDIO_TEST_VOLUME3		(3)	/**< Volume level 3. */
#define AUDIO_TEST_VOLUME4		(4)	/**< Volume level 4. */
#define AUDIO_TEST_VOLUME5		(5)	/**< Volume level 5. */

/*!
  @brief	LR range.
*/
#define AUDIO_TEST_OE_SPEAKER_ENABLE_L		(0x1000)/**< SPKOUTL_ENA. */
#define AUDIO_TEST_OE_SPEAKER_ENABLE_R		(0x2000)/**< SPKOUTR_ENA. */
#define AUDIO_TEST_OE_SPEAKER_ENABLE_LR		(0x3000)/**< SPKOUTLR_ENA. */
#define AUDIO_TEST_OE_HEADPHONE_ENABLE_L	(0x0200)/**< HPOUT1L_ENA. */
#define AUDIO_TEST_OE_HEADPHONE_ENABLE_R	(0x0100)/**< HPOUT1R_ENA. */
#define AUDIO_TEST_OE_HEADPHONE_ENABLE_LR	(0x0300)/**< HPOUT1LR_ENA. */
#define AUDIO_TEST_OE_EARPIECE_ENABLE_L		(0x0800)/**< HPOUT2_ENA. */
#define AUDIO_TEST_OE_EARPIECE_ENABLE_R		(0x0800)/**< HPOUT2_ENA. */
#define AUDIO_TEST_OE_EARPIECE_ENABLE_LR	(0x0800)/**< HPOUT2_ENA. */
#define AUDIO_TEST_OE_OTHER			(0x0037)/**< Othrer bit. */

/*---------------------------------------------------------------------------*/
/* define function macro declaration (private)                               */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* enum declaration (private)                                                */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* structure declaration (private)                                           */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* prototype declaration (private)                                           */
/*---------------------------------------------------------------------------*/
/* none */


/*---------------------------------------------------------------------------*/
/* global variable declaration                                               */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* extern variable declaration                                               */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* extern function declaration                                               */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* inline function implementation                                            */
/*---------------------------------------------------------------------------*/
/* none */



/*---------------------------------------------------------------------------*/
/* function implementation                                                   */
/*---------------------------------------------------------------------------*/
/*------------------------------------*/
/* for public function                */
/*------------------------------------*/
/*!
  @brief	Call read() of AudioIC.

  @param	addr [i] Register address.
  @param	value [o] Register value.

  @return	Function results.

  @note		.
*/
int audio_test_ic_read(const u_short addr, u_short *value)
{
	int ret = 0;

	audio_test_log_efunc("");

	ret = wm1811_read(addr, value);
	if (0 <= ret) {
		audio_test_log_info("ret[%d]", ret);
		ret = 0;
	}

	audio_test_log_rfunc("ret[%d]", ret);
	return ret;
}

/*!
  @brief	Call write() of AudioIC.

  @param	addr [i] Register address.
  @param	value [i] Register value.

  @return	Function results.

  @note		.
*/
int audio_test_ic_write(const u_short addr, const u_short value)
{
	int ret = 0;

	audio_test_log_efunc("");

	ret = wm1811_write(addr, value);

	audio_test_log_rfunc("ret[%d]", ret);
	return ret;
}

/*!
  @brief	Call set_device() of AudioIC.

  @param	device [i] Device bit.
  @param	mode [i] SNDP_PLAYBACK_SPEAKER_INCALL
			 SNDP_PLAYBACK_SPEAKER_NORMAL

  @return	Function results.

  @note		.
*/
int audio_test_ic_set_device(const u_long device, const u_int mode)
{
	int ret = 0;

	audio_test_log_efunc("");

	ret = wm1811_set_device(device, mode, WM1811_POWER_ON);

	audio_test_log_rfunc("ret[%d]", ret);
	return ret;
}

/*!
  @brief	Call set_device() of AudioIC to clear.

  @param	.

  @return	Function results.

  @note		.
*/
int audio_test_ic_clear_device(void)
{
	int ret = 0;

	audio_test_log_efunc("");

	ret = wm1811_set_device(WM1811_DEV_NONE, 0x00000000, WM1811_POWER_ON);

	audio_test_log_rfunc("ret[%d]", ret);
	return ret;
}

/*!
  @brief	Call get_device() of AudioIC.

  @param	device [o] Device bit.

  @return	Function results.

  @note		.
*/
int audio_test_ic_get_device(u_long *device)
{
	int ret = 0;

	audio_test_log_efunc("");

	ret = wm1811_get_device(device);

	audio_test_log_rfunc("ret[%d]", ret);
	return ret;
}

/*!
  @brief	Call set_volume() of AudioIC.

  @param	device [i] Device bit.
  @param	volume [i] Volume.

  @return	Function results.

  @note		.
*/
int audio_test_ic_set_volume(const u_long device, const u_int volume)
{
	int ret = 0;

	audio_test_log_efunc("");

	ret = wm1811_set_volume(device, volume);

	audio_test_log_rfunc("ret[%d]", ret);
	return ret;
}

/*!
  @brief	Call get_volume() of AudioIC.

  @param	device [i] Device bit.
  @param	volume [o] Volume.

  @return	Function results.

  @note		.
*/
int audio_test_ic_get_volume(const u_long device, u_int *volume)
{
	int ret = 0;

	audio_test_log_efunc("");

	ret = wm1811_get_volume(device, volume);

	audio_test_log_rfunc("ret[%d]", ret);
	return ret;
}

/*!
  @brief	Convert input device type to device bit.

  @param	device_type [i] Input device type.
  @param	device [o] Device bit.

  @return	.

  @note		.
*/
void audio_test_cnv_input_device(u_int device_type, u_long *device)
{
	audio_test_log_efunc("");

	switch (device_type) {
	/***********************************/
	/* Main Mic                        */
	/***********************************/
	case AUDIO_TEST_DRV_IN_MIC:
		*device |= WM1811_DEV_CAPTURE_MIC;
		break;
	/***********************************/
	/* Headset Mic                     */
	/***********************************/
	case AUDIO_TEST_DRV_IN_HEADSETMIC:
		*device |= WM1811_DEV_CAPTURE_HEADSET_MIC;
		break;
	/***********************************/
	/* Unknown                         */
	/***********************************/
	default:
		audio_test_log_info("unknown input device");
		break;
	}

	audio_test_log_rfunc("");
}

/*!
  @brief	Convert output device type to device bit.

  @param	device_type [i] Output device type.
  @param	device [o] Device bit.

  @return	.

  @note		.
*/
void audio_test_cnv_output_device(u_int device_type, u_long *device)
{
	audio_test_log_efunc("");

	switch (device_type) {
	/***********************************/
	/* Speaker                         */
	/***********************************/
	case AUDIO_TEST_DRV_OUT_SPEAKER:
		*device |= WM1811_DEV_PLAYBACK_SPEAKER;
		break;
	/***********************************/
	/* Headphone                       */
	/***********************************/
	case AUDIO_TEST_DRV_OUT_HEADPHONE:
		*device |= WM1811_DEV_PLAYBACK_HEADPHONES;
		break;
	/***********************************/
	/* Earpiece                        */
	/***********************************/
	case AUDIO_TEST_DRV_OUT_EARPIECE:
		*device |= WM1811_DEV_PLAYBACK_EARPIECE;
		break;
	/***********************************/
	/* Unknown                         */
	/***********************************/
	default:
		audio_test_log_info("unknown output device");
		break;
	}

	audio_test_log_rfunc("");
}

/*!
  @brief	Convert specified volume type to AudioIC volume type.

  @param	volume_type [i] Specified volume type.
  @param	volume [o] AudioIC volume type.

  @return	.

  @note		.
*/
void audio_test_cnv_volume(u_int volume_type, u_int *volume)
{
	audio_test_log_efunc("");

	switch (volume_type) {
	/***********************************/
	/* Volume 0                        */
	/***********************************/
	case AUDIO_TEST_VOLUME0:
		*volume = WM1811_VOLUMEL0;
		break;
	/***********************************/
	/* Volume 1                        */
	/***********************************/
	case AUDIO_TEST_VOLUME1:
		*volume = WM1811_VOLUMEL1;
		break;
	/***********************************/
	/* Volume 2                        */
	/***********************************/
	case AUDIO_TEST_VOLUME2:
		*volume = WM1811_VOLUMEL2;
		break;
	/***********************************/
	/* Volume 3                        */
	/***********************************/
	case AUDIO_TEST_VOLUME3:
		*volume = WM1811_VOLUMEL3;
		break;
	/***********************************/
	/* Volume 4                        */
	/***********************************/
	case AUDIO_TEST_VOLUME4:
		*volume = WM1811_VOLUMEL4;
		break;
	/***********************************/
	/* Volume 5                        */
	/***********************************/
	case AUDIO_TEST_VOLUME5:
	default:
		*volume = WM1811_VOLUMEL5;
		break;
	}

	audio_test_log_rfunc("");
}

/*!
  @brief	Convert output device type and LR type to output enable bit.

  @param	device_type [i] Output device type.
  @param	LR_type [i] LR type.
  @param	oe [o] Output enable bit.

  @return	.

  @note		.
*/
void audio_test_cnv_oe(u_int device_type, u_int LR_type, u_short *oe)
{
	u_int oe_other = 0;

	audio_test_log_efunc("");

	switch (device_type) {
	/***********************************/
	/* Speaker                         */
	/***********************************/
	case AUDIO_TEST_DRV_OUT_SPEAKER:
		switch (LR_type) {
		/***********************************/
		/* Left                            */
		/***********************************/
		case AUDIO_TEST_DRV_LR_L:
			oe_other = *oe & AUDIO_TEST_OE_OTHER;
			*oe = (AUDIO_TEST_OE_SPEAKER_ENABLE_L |
				oe_other);
			break;
		/***********************************/
		/* Right                           */
		/***********************************/
		case AUDIO_TEST_DRV_LR_R:
			oe_other = *oe & AUDIO_TEST_OE_OTHER;
			*oe = (AUDIO_TEST_OE_SPEAKER_ENABLE_R |
				oe_other);
			break;
		/***********************************/
		/* Left/Right                      */
		/***********************************/
		case AUDIO_TEST_DRV_LR_LR:
			oe_other = *oe & AUDIO_TEST_OE_OTHER;
			*oe = (AUDIO_TEST_OE_SPEAKER_ENABLE_LR |
				oe_other);
			break;
		/***********************************/
		/* Unknown                         */
		/***********************************/
		default:
			audio_test_log_info("unknown LR info");
			break;
		}
		break;
	/***********************************/
	/* Heaphone                        */
	/***********************************/
	case AUDIO_TEST_DRV_OUT_HEADPHONE:
		switch (LR_type) {
		/***********************************/
		/* Left                            */
		/***********************************/
		case AUDIO_TEST_DRV_LR_L:
			oe_other = *oe & AUDIO_TEST_OE_OTHER;
			*oe = (AUDIO_TEST_OE_HEADPHONE_ENABLE_L |
				oe_other);
			break;
		/***********************************/
		/* Right                           */
		/***********************************/
		case AUDIO_TEST_DRV_LR_R:
			oe_other = *oe & AUDIO_TEST_OE_OTHER;
			*oe = (AUDIO_TEST_OE_HEADPHONE_ENABLE_R |
				oe_other);
			break;
		/***********************************/
		/* Left/Right                      */
		/***********************************/
		case AUDIO_TEST_DRV_LR_LR:
			oe_other = *oe & AUDIO_TEST_OE_OTHER;
			*oe = (AUDIO_TEST_OE_HEADPHONE_ENABLE_LR |
				oe_other);
			break;
		/***********************************/
		/* Unknown                         */
		/***********************************/
		default:
			audio_test_log_info("unknown LR info");
			break;
		}
		break;
	/***********************************/
	/* Earpiece                        */
	/***********************************/
	case AUDIO_TEST_DRV_OUT_EARPIECE:
		switch (LR_type) {
		/***********************************/
		/* Left                            */
		/***********************************/
		case AUDIO_TEST_DRV_LR_L:
			oe_other = *oe & AUDIO_TEST_OE_OTHER;
			*oe = (AUDIO_TEST_OE_EARPIECE_ENABLE_L |
				oe_other);
			break;
		/***********************************/
		/* Right                           */
		/***********************************/
		case AUDIO_TEST_DRV_LR_R:
			oe_other = *oe & AUDIO_TEST_OE_OTHER;
			*oe = (AUDIO_TEST_OE_EARPIECE_ENABLE_R |
				oe_other);
			break;
		/***********************************/
		/* Left/Right                      */
		/***********************************/
		case AUDIO_TEST_DRV_LR_LR:
			oe_other = *oe & AUDIO_TEST_OE_OTHER;
			*oe = (AUDIO_TEST_OE_EARPIECE_ENABLE_LR |
				oe_other);
			break;
		/***********************************/
		/* Unknown                         */
		/***********************************/
		default:
			audio_test_log_info("unknown LR info");
			break;
		}
		break;
	/***********************************/
	/* Unknown                         */
	/***********************************/
	default:
		audio_test_log_info("unknown output device");
		break;
	}

	audio_test_log_rfunc("");
}

MODULE_LICENSE("GPL v2");
