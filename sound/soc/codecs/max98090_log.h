/* max98090_log.h
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
  @file max98090_log.h

  @brief Audio LSI driver log header file.
*/

#ifndef __MAX98090_LOG_H__
#define __MAX98090_LOG_H__

/*---------------------------------------------------------------------------*/
/* include files                                                             */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* typedef declaration                                                       */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* define macro declaration                                                  */
/*---------------------------------------------------------------------------*/
#define AUDIO_LSI_DRV_NAME              "audio_lsi_driver"
#define MAX98090_LOG_LEVEL              "log_level"
#define MAX98090_TUNE_REG               "tune_reg"
#define MAX98090_DUMP_REG               "dump_reg"
#define MAX98090_PATH_SET               "path_set"
#define MAX98090_LOG_NO_PRINT           (0x00)
#define MAX98090_LOG_ERR_PRINT          (0x01)
#define MAX98090_LOG_PROC_PRINT         (0x02)
#define MAX98090_LOG_FUNC_PRINT         (0x03)
#define MAX98090_LOG_DEBUG_PRINT        (0x04)

#define MAX98090_LOG_BIT_REG_DUMP       (0x10)
#define MAX98090_LOG_BIT_DMESG          (0x80)

#define MAX98090_LOG_LEVEL_MAX          (0xffffffff)
#define MAX98090_LOG_BYTE_LOW(sw)       ((sw) & 0x0000000f)

#define MAX98090_GET_PROCESS_TIME(tv)   do_gettimeofday(&tv)
#define __PRN_LSI__

/*---------------------------------------------------------------------------*/
/* define function macro declaration                                         */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* enum declaration                                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* structure declaration                                                     */
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
/* prototype declaration                                                     */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* inline function implementation                                            */
/*---------------------------------------------------------------------------*/
#ifdef __PRN_LSI__

#define max98090_log_reg_dump(fmt, ...)                                      \
do {                                                                         \
	if (max98090_log_level & MAX98090_LOG_BIT_REG_DUMP) {                \
		(max98090_log_level & MAX98090_LOG_BIT_DMESG) ?              \
			pr_err(fmt, ##__VA_ARGS__) :                         \
			pr_alert(fmt, ##__VA_ARGS__);                        \
	}                                                                    \
} while (0)


#ifdef __PRN_ADD_TIME__

#define max98090_log_err(fmt, ...)                                           \
do {                                                                         \
	struct timeval tv;                                                   \
	if (MAX98090_LOG_ERR_PRINT <=                                        \
		MAX98090_LOG_BYTE_LOW(max98090_log_level)) {                 \
		MAX98090_GET_PROCESS_TIME(tv);                               \
		if (max98090_log_level & MAX98090_LOG_BIT_DMESG) {           \
			pr_err("[%5ld.%06ld] " AUDIO_LSI_DRV_NAME            \
				" : %s():[ERR]:" fmt, tv.tv_sec, tv.tv_usec, \
				__func__,  ##__VA_ARGS__);                   \
		} else {                                                     \
			pr_alert("[%5ld.%06ld] " AUDIO_LSI_DRV_NAME          \
				" : %s():[ERR]:" fmt, tv.tv_sec, tv.tv_usec, \
				__func__, ##__VA_ARGS__);                    \
		}                                                            \
	}                                                                    \
} while (0)

#define max98090_log_info(fmt, ...)                                          \
do {                                                                         \
	struct timeval tv;                                                   \
	if (MAX98090_LOG_PROC_PRINT <=                                       \
		MAX98090_LOG_BYTE_LOW(max98090_log_level)) {                 \
		MAX98090_GET_PROCESS_TIME(tv);                               \
		if (max98090_log_level & MAX98090_LOG_BIT_DMESG) {           \
			pr_err("[%5ld.%06ld] " AUDIO_LSI_DRV_NAME            \
				" : %s():[INF]:" fmt, tv.tv_sec, tv.tv_usec, \
				__func__, ##__VA_ARGS__);                    \
		} else {                                                     \
			pr_alert("[%5ld.%06ld] " AUDIO_LSI_DRV_NAME          \
				" : %s():[INF]:" fmt, tv.tv_sec, tv.tv_usec, \
				__func__, ##__VA_ARGS__);                    \
		}                                                            \
	}                                                                    \
} while (0)

#define max98090_log_debug(fmt, ...)                                         \
do {                                                                         \
	struct timeval tv;                                                   \
	if (MAX98090_LOG_DEBUG_PRINT <=                                      \
		MAX98090_LOG_BYTE_LOW(max98090_log_level)) {                 \
		MAX98090_GET_PROCESS_TIME(tv);                               \
		if (max98090_log_level & MAX98090_LOG_BIT_DMESG) {           \
			pr_err("[%5ld.%06ld] " AUDIO_LSI_DRV_NAME            \
				" : %s():[DBG]:" fmt, tv.tv_sec, tv.tv_usec, \
				__func__, ##__VA_ARGS__);                    \
		} else {                                                     \
			pr_alert("[%5ld.%06ld] " AUDIO_LSI_DRV_NAME          \
				" : %s():[DBG]:" fmt, tv.tv_sec, tv.tv_usec, \
				__func__, ##__VA_ARGS__);                    \
		}                                                            \
	}                                                                    \
} while (0)

#define max98090_log_efunc(fmt, ...)                                         \
do {                                                                         \
	struct timeval tv;                                                   \
	if (MAX98090_LOG_FUNC_PRINT <=                                       \
		MAX98090_LOG_BYTE_LOW(max98090_log_level)) {                 \
		MAX98090_GET_PROCESS_TIME(tv);                               \
		if (max98090_log_level & MAX98090_LOG_BIT_DMESG) {           \
			pr_err("[%5ld.%06ld] " AUDIO_LSI_DRV_NAME            \
				" : %s():[ENT]:" fmt, tv.tv_sec, tv.tv_usec, \
				__func__, ##__VA_ARGS__);                    \
		} else {                                                     \
			pr_alert("[%5ld.%06ld] " AUDIO_LSI_DRV_NAME          \
				" : %s():[ENT]:" fmt, tv.tv_sec, tv.tv_usec, \
				__func__, ##__VA_ARGS__);                    \
		}                                                            \
	}                                                                    \
} while (0)

#define max98090_log_rfunc(fmt, ...)                                         \
do {                                                                         \
	struct timeval tv;                                                   \
	if (MAX98090_LOG_FUNC_PRINT <=                                       \
		MAX98090_LOG_BYTE_LOW(max98090_log_level)) {                 \
		MAX98090_GET_PROCESS_TIME(tv);                               \
		if (max98090_log_level & MAX98090_LOG_BIT_DMESG) {           \
			pr_err("[%5ld.%06ld] " AUDIO_LSI_DRV_NAME            \
				" : %s():[RTN]:" fmt, tv.tv_sec, tv.tv_usec, \
				__func__, ##__VA_ARGS__);                    \
		} else {                                                     \
			pr_alert("[%5ld.%06ld] " AUDIO_LSI_DRV_NAME          \
				" : %s():[RTN]:" fmt, tv.tv_sec, tv.tv_usec, \
				__func__, ##__VA_ARGS__);                    \
		}                                                            \
	}                                                                    \
} while (0)


#else /* != __PRN_ADD_TIME__ */


#define max98090_log_err(fmt, ...)                                           \
do {                                                                         \
	if (MAX98090_LOG_ERR_PRINT <=                                        \
		MAX98090_LOG_BYTE_LOW(max98090_log_level)) {                 \
		if (max98090_log_level & MAX98090_LOG_BIT_DMESG) {           \
			pr_err(AUDIO_LSI_DRV_NAME " : %s():[ERR]:"           \
				fmt, __func__, ##__VA_ARGS__);               \
		} else {                                                     \
			pr_alert(AUDIO_LSI_DRV_NAME " : %s():[ERR]:"         \
				fmt, __func__, ##__VA_ARGS__);               \
		}                                                            \
	}                                                                    \
} while (0)

#define max98090_log_info(fmt, ...)                                          \
do {                                                                         \
	if (MAX98090_LOG_PROC_PRINT <=                                       \
		MAX98090_LOG_BYTE_LOW(max98090_log_level)) {                 \
		if (max98090_log_level & MAX98090_LOG_BIT_DMESG) {           \
			pr_err(AUDIO_LSI_DRV_NAME " : %s():[INF]:"           \
				fmt, __func__, ##__VA_ARGS__);               \
		} else {                                                     \
			pr_alert(AUDIO_LSI_DRV_NAME " : %s():[INF]:"         \
				fmt, __func__, ##__VA_ARGS__);               \
		}                                                            \
	}                                                                    \
} while (0)

#define max98090_log_debug(fmt, ...)                                         \
do {                                                                         \
	if (MAX98090_LOG_DEBUG_PRINT <=                                      \
		MAX98090_LOG_BYTE_LOW(max98090_log_level)) {                 \
		if (max98090_log_level & MAX98090_LOG_BIT_DMESG) {           \
			pr_err(AUDIO_LSI_DRV_NAME " : %s():[DBG]:"           \
				fmt, __func__, ##__VA_ARGS__);               \
		} else {                                                     \
			pr_alert(AUDIO_LSI_DRV_NAME " : %s():[DBG]:"         \
				fmt, __func__, ##__VA_ARGS__);               \
		}                                                            \
	}                                                                    \
} while (0)

#define max98090_log_efunc(fmt, ...)                                         \
do {                                                                         \
	if (MAX98090_LOG_FUNC_PRINT <=                                       \
		MAX98090_LOG_BYTE_LOW(max98090_log_level)) {                 \
		if (max98090_log_level & MAX98090_LOG_BIT_DMESG) {           \
			pr_err(AUDIO_LSI_DRV_NAME " : %s():[ENT]:"           \
				fmt, __func__, ##__VA_ARGS__);               \
		} else {                                                     \
			pr_alert(AUDIO_LSI_DRV_NAME " : %s():[ENT]:"         \
				fmt, __func__, ##__VA_ARGS__);               \
		}                                                            \
	}                                                                    \
} while (0)

#define max98090_log_rfunc(fmt, ...)                                         \
do {                                                                         \
	if (MAX98090_LOG_FUNC_PRINT <=                                       \
		MAX98090_LOG_BYTE_LOW(max98090_log_level)) {                 \
		if (max98090_log_level & MAX98090_LOG_BIT_DMESG) {           \
			pr_err(AUDIO_LSI_DRV_NAME " : %s():[RTN]:"           \
				fmt, __func__, ##__VA_ARGS__);               \
		} else {                                                     \
			pr_alert(AUDIO_LSI_DRV_NAME " : %s():[RTN]:"         \
				fmt, __func__, ##__VA_ARGS__);               \
		}                                                            \
	}                                                                    \
} while (0)

#endif  /* __PRN_ADD_TIME__ */
#else   /* != __PRN_LSI__ */

#define max98090_log_reg_dump(fmt, ...)         do { } while (0)
#define max98090_log_err(fmt, ...)              do { } while (0)
#define max98090_log_info(fmt, ...)             do { } while (0)
#define max98090_log_debug(fmt, ...)            do { } while (0)
#define max98090_log_efunc(fmt, ...)            do { } while (0)
#define max98090_log_rfunc(fmt, ...)            do { } while (0)

#endif	/* __PRN_LSI__ */

#endif  /* __MAX98090_LOG_H__ */
