/* audio_test_extern.h
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
  @file		audio_test_extern.h

  @brief	Public definition Audio test command header file.
*/

#ifndef __AUDIO_TEST_EXTERN_H__
#define __AUDIO_TEST_EXTERN_H__

/*---------------------------------------------------------------------------*/
/* include files                                                             */
/*---------------------------------------------------------------------------*/
#include <linux/ioctl.h>

/*---------------------------------------------------------------------------*/
/* typedef declaration                                                       */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* define macro declaration                                                  */
/*---------------------------------------------------------------------------*/
/***********************************/
/* ioctl                           */
/***********************************/
#define AUDIO_TEST_IOC_MAGIC		'a'

/*---------------------------------------------------------------------------*/
/* structure declaration                                                     */
/*---------------------------------------------------------------------------*/
/*!
  @brief	ioctl command.
*/
struct audio_test_ioctl_cmd {
	u_int in_device_type;	/**< Intput Device type */
	u_int out_device_type;	/**< Output Device type */
	u_int out_LR_type;	/**< LR type */
	u_int out_volume;	/**< Volume */
	u_int fsi_port;		/**< FSI Port */
	u_int *detect_jack;	/**< Jack detect */
	u_int *detect_key;	/**< Key detect */
	u_int vqa_val;		/**< VQA Valid */
	u_int delay_val;	/**< Delay Valid */
};

/*---------------------------------------------------------------------------*/
/* define function macro declaration                                         */
/*---------------------------------------------------------------------------*/
#define AUDIO_TEST_IOCTL_SETDEVICE	_IOW(AUDIO_TEST_IOC_MAGIC, 1, \
					struct audio_test_ioctl_cmd)
#define AUDIO_TEST_IOCTL_STARTSCUWLOOP	_IOW(AUDIO_TEST_IOC_MAGIC, 2, \
					struct audio_test_ioctl_cmd)
#define AUDIO_TEST_IOCTL_STOPSCUWLOOP	_IOW(AUDIO_TEST_IOC_MAGIC, 3, \
					struct audio_test_ioctl_cmd)
#define AUDIO_TEST_IOCTL_DETECTJACK	_IOR(AUDIO_TEST_IOC_MAGIC, 4, \
					struct audio_test_ioctl_cmd)
#define AUDIO_TEST_IOCTL_DETECTKEY	_IOR(AUDIO_TEST_IOC_MAGIC, 5, \
					struct audio_test_ioctl_cmd)
#define AUDIO_TEST_IOCTL_STARTTONE	_IOW(AUDIO_TEST_IOC_MAGIC, 6, \
					struct audio_test_ioctl_cmd)
#define AUDIO_TEST_IOCTL_STOPTONE	_IOW(AUDIO_TEST_IOC_MAGIC, 7, \
					struct audio_test_ioctl_cmd)
#define AUDIO_TEST_IOCTL_STARTSPUVLOOP	_IOW(AUDIO_TEST_IOC_MAGIC, 8, \
					struct audio_test_ioctl_cmd)
#define AUDIO_TEST_IOCTL_STOPSPUVLOOP	_IOW(AUDIO_TEST_IOC_MAGIC, 9, \
					struct audio_test_ioctl_cmd)

/*---------------------------------------------------------------------------*/
/* enum declaration                                                          */
/*---------------------------------------------------------------------------*/
/*!
  @brief	ON/OFF type.
*/
enum audio_test_state_type {
	AUDIO_TEST_DRV_STATE_OFF,	/**< 0x00: OFF. */
	AUDIO_TEST_DRV_STATE_ON,	/**< 1x01: ON. */
	AUDIO_TEST_DRV_STATE_MAX
};

/*!
  @brief	Input device type.
*/
enum audio_test_in_device_type {
	AUDIO_TEST_DRV_IN_MIC,		/**< 0x00: Mic. */
	AUDIO_TEST_DRV_IN_HEADSETMIC,	/**< 0x01: Headset mic. */
	AUDIO_TEST_DRV_IN_MAX
};

/*!
  @brief	Output device type.
*/
enum audio_test_out_device_type {
	AUDIO_TEST_DRV_OUT_SPEAKER,	/**< 0x00: Speaker. */
	AUDIO_TEST_DRV_OUT_HEADPHONE,	/**< 0x01: Headphone. */
	AUDIO_TEST_DRV_OUT_EARPIECE,	/**< 0x02: Earpiece. */
	AUDIO_TEST_DRV_OUT_MAX
};

/*!
  @brief	Output LR type.
*/
enum audio_test_out_LR_type {
	AUDIO_TEST_DRV_LR_L,		/**< 0x00: Use L. */
	AUDIO_TEST_DRV_LR_R,		/**< 0x01: Use R. */
	AUDIO_TEST_DRV_LR_LR,		/**< 0x02: Use LR. */
	AUDIO_TEST_DRV_LR_MAX
};

/*!
  @brief	FSI Port.
*/
enum audio_test_fsi_port {
	AUDIO_TEST_DRV_FSI_PORTA,	/**< 0x00: Port A. */
	AUDIO_TEST_DRV_FSI_PORTB,	/**< 0x01: Port B. */
	AUDIO_TEST_DRV_FSI_MAX
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
/* none */

/*---------------------------------------------------------------------------*/
/* inline function implementation                                            */
/*---------------------------------------------------------------------------*/
/* none */

#endif  /* __AUDIO_TEST_EXTERN_H__ */
