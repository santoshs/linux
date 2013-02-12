/*
 * D2153 ALSA SoC codec driver
 *
 * Copyright (c) 2012 Dialog Semiconductor
 *
 * Written by Adam Thomson <Adam.Thomson.Opensource@diasemi.com>
 * Based on DA9055 ALSA SoC codec driver.
 * 
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */


#ifndef __AUDIO_TEST_D2153_H__
#define __AUDIO_TEST_D2153_H__

/*---------------------------------------------------------------------------*/
/* include files                                                             */
/*---------------------------------------------------------------------------*/
#include <linux/d2153/d2153_codec.h>


/*---------------------------------------------------------------------------*/
/* typedef declaration                                                       */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* define macro declaration                                                  */
/*---------------------------------------------------------------------------*/
/***********************************/
/* Compile switch                  */
/***********************************/
/* #define AUDIO_TEST_DBG */

/***********************************/
/* log level                       */
/***********************************/
#define AUDIO_TEST_DRV_NAME		"audio_test"
#define AUDIO_TEST_LOG_LEVEL		"log_level"
#define AUDIO_TEST_LOG_NO_PRINT		(0x00)
#define AUDIO_TEST_LOG_ERR_PRINT	(0x01)
#define AUDIO_TEST_LOG_PROC_PRINT	(0x02)
#define AUDIO_TEST_LOG_FUNC_PRINT	(0x03)
#define AUDIO_TEST_LOG_DEBUG_PRINT	(0x04)

#define AUDIO_TEST_LOG_BIT_REG_DUMP	(0x10)
#define AUDIO_TEST_LOG_BIT_DMESG	(0x80)

#define AUDIO_TEST_LOG_LEVEL_MAX	(0xffffffff)
#define AUDIO_TEST_LOG_BYTE_LOW(sw)	((sw) & 0x0000000f)

#define AUDIO_TEST_GET_PROCESS_TIME(tv)	do_gettimeofday(&tv)
#define __PRN_AUDIO_TEST__
#define __PRN_AUDIO_TEST_ADD_TIME__

/*---------------------------------------------------------------------------*/
/* structure declaration                                                     */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* define function macro declaration                                         */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* enum declaration                                                          */
/*---------------------------------------------------------------------------*/
/*!
  @brief	HW ID value.
*/
enum audio_test_hw_val {
	AUDIO_TEST_HW_CLKGEN,		/**< CLKGEN. */
	AUDIO_TEST_HW_FSI,		/**< FSI. */
	AUDIO_TEST_HW_SCUW,		/**< SCUW. */
	AUDIO_TEST_HW_MAX
};

/*---------------------------------------------------------------------------*/
/* extern variable declaration                                               */
/*---------------------------------------------------------------------------*/
/*!
  @brief	Audio Test driver log level.
*/
extern u_int audio_test_log_level;

/*---------------------------------------------------------------------------*/
/* extern function declaration                                               */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* prototype declaration                                                     */
/*---------------------------------------------------------------------------*/
int audio_test_ic_read(const u_short addr, u_short *value);
int audio_test_ic_write(const u_short addr, const u_short value);
int audio_test_ic_set_device(const u_long device);
int audio_test_ic_clear_device(void);
int audio_test_ic_get_device(u_long *device);
int audio_test_ic_set_volume(const u_long device, const u_int volume);
int audio_test_ic_get_volume(const u_long device, u_int *volume);
void audio_test_cnv_input_device(u_int device_type, u_long *device);
void audio_test_cnv_output_device(u_int device_type, u_long *device);
void audio_test_cnv_volume(u_int volume_type, u_int *volume);
void audio_test_cnv_oe(u_int device_type, u_int LR_type, u_short *oe);
#ifdef D2153_FSI_SOUNDPATH
int audio_test_ic_aad_read(const u_short addr, u_short *value);
int audio_test_ic_aad_write(const u_short addr, const u_short value);
int audio_test_ic_pmic_read(const u_short addr, u_short *value);
#endif
/*---------------------------------------------------------------------------*/
/* inline function implementation                                            */
/*---------------------------------------------------------------------------*/
#ifdef __PRN_AUDIO_TEST__

#define audio_test_log_reg_dump(fmt, ...)                                    \
do {                                                                         \
	if (audio_test_log_level & AUDIO_TEST_LOG_BIT_REG_DUMP) {            \
		(audio_test_log_level & AUDIO_TEST_LOG_BIT_DMESG) ?          \
			pr_err(fmt, ##__VA_ARGS__) :                         \
			pr_alert(fmt, ##__VA_ARGS__);                        \
	}                                                                    \
} while (0)


#ifdef __PRN_AUDIO_TEST_ADD_TIME__

#define audio_test_log_err(fmt, ...)                                         \
do {                                                                         \
	struct timeval tv;                                                   \
	if (AUDIO_TEST_LOG_ERR_PRINT <=                                      \
		AUDIO_TEST_LOG_BYTE_LOW(audio_test_log_level)) {             \
		AUDIO_TEST_GET_PROCESS_TIME(tv);                             \
		if (audio_test_log_level & AUDIO_TEST_LOG_BIT_DMESG) {       \
			pr_err("[%5ld.%06ld] " AUDIO_TEST_DRV_NAME           \
				" : %s():[ERR]:" fmt, tv.tv_sec, tv.tv_usec, \
				__func__,  ##__VA_ARGS__);                   \
		} else {                                                     \
			pr_alert("[%5ld.%06ld] " AUDIO_TEST_DRV_NAME         \
				" : %s():[ERR]:" fmt, tv.tv_sec, tv.tv_usec, \
				__func__, ##__VA_ARGS__);                    \
		}                                                            \
	}                                                                    \
} while (0)

#define audio_test_log_info(fmt, ...)                                        \
do {                                                                         \
	struct timeval tv;                                                   \
	if (AUDIO_TEST_LOG_PROC_PRINT <=                                     \
		AUDIO_TEST_LOG_BYTE_LOW(audio_test_log_level)) {             \
		AUDIO_TEST_GET_PROCESS_TIME(tv);                             \
		if (audio_test_log_level & AUDIO_TEST_LOG_BIT_DMESG) {       \
			pr_err("[%5ld.%06ld] " AUDIO_TEST_DRV_NAME           \
				" : %s():[INF]:" fmt, tv.tv_sec, tv.tv_usec, \
				__func__, ##__VA_ARGS__);                    \
		} else {                                                     \
			pr_alert("[%5ld.%06ld] " AUDIO_TEST_DRV_NAME         \
				" : %s():[INF]:" fmt, tv.tv_sec, tv.tv_usec, \
				__func__, ##__VA_ARGS__);                    \
		}                                                            \
	}                                                                    \
} while (0)

#define audio_test_log_debug(fmt, ...)                                       \
do {                                                                         \
	struct timeval tv;                                                   \
	if (AUDIO_TEST_LOG_DEBUG_PRINT <=                                    \
		AUDIO_TEST_LOG_BYTE_LOW(audio_test_log_level)) {             \
		AUDIO_TEST_GET_PROCESS_TIME(tv);                             \
		if (audio_test_log_level & AUDIO_TEST_LOG_BIT_DMESG) {       \
			pr_err("[%5ld.%06ld] " AUDIO_TEST_DRV_NAME           \
				" : %s():[DBG]:" fmt, tv.tv_sec, tv.tv_usec, \
				__func__, ##__VA_ARGS__);                    \
		} else {                                                     \
			pr_alert("[%5ld.%06ld] " AUDIO_TEST_DRV_NAME         \
				" : %s():[DBG]:" fmt, tv.tv_sec, tv.tv_usec, \
				__func__, ##__VA_ARGS__);                    \
		}                                                            \
	}                                                                    \
} while (0)

#define audio_test_log_efunc(fmt, ...)                                       \
do {                                                                         \
	struct timeval tv;                                                   \
	if (AUDIO_TEST_LOG_FUNC_PRINT <=                                     \
		AUDIO_TEST_LOG_BYTE_LOW(audio_test_log_level)) {             \
		AUDIO_TEST_GET_PROCESS_TIME(tv);                             \
		if (audio_test_log_level & AUDIO_TEST_LOG_BIT_DMESG) {       \
			pr_err("[%5ld.%06ld] " AUDIO_TEST_DRV_NAME           \
				" : %s():[ENT]:" fmt, tv.tv_sec, tv.tv_usec, \
				__func__, ##__VA_ARGS__);                    \
		} else {                                                     \
			pr_alert("[%5ld.%06ld] " AUDIO_TEST_DRV_NAME         \
				" : %s():[ENT]:" fmt, tv.tv_sec, tv.tv_usec, \
				__func__, ##__VA_ARGS__);                    \
		}                                                            \
	}                                                                    \
} while (0)

#define audio_test_log_rfunc(fmt, ...)                                       \
do {                                                                         \
	struct timeval tv;                                                   \
	if (AUDIO_TEST_LOG_FUNC_PRINT <=                                     \
		AUDIO_TEST_LOG_BYTE_LOW(audio_test_log_level)) {             \
		AUDIO_TEST_GET_PROCESS_TIME(tv);                             \
		if (audio_test_log_level & AUDIO_TEST_LOG_BIT_DMESG) {       \
			pr_err("[%5ld.%06ld] " AUDIO_TEST_DRV_NAME           \
				" : %s():[RTN]:" fmt, tv.tv_sec, tv.tv_usec, \
				__func__, ##__VA_ARGS__);                    \
		} else {                                                     \
			pr_alert("[%5ld.%06ld] " AUDIO_TEST_DRV_NAME         \
				" : %s():[RTN]:" fmt, tv.tv_sec, tv.tv_usec, \
				__func__, ##__VA_ARGS__);                    \
		}                                                            \
	}                                                                    \
} while (0)


#else /* != __PRN_AUDIO_TEST_ADD_TIME__ */


#define audio_test_log_err(fmt, ...)                                         \
do {                                                                         \
	if (AUDIO_TEST_LOG_ERR_PRINT <=                                      \
		AUDIO_TEST_LOG_BYTE_LOW(audio_test_log_level)) {             \
		if (audio_test_log_level & AUDIO_TEST_LOG_BIT_DMESG) {       \
			pr_err(AUDIO_TEST_DRV_NAME " : %s():[ERR]:"          \
				fmt, __func__, ##__VA_ARGS__);               \
		} else {                                                     \
			pr_alert(AUDIO_TEST_DRV_NAME " : %s():[ERR]:"        \
				fmt, __func__, ##__VA_ARGS__);               \
		}                                                            \
	}                                                                    \
} while (0)

#define audio_test_log_info(fmt, ...)                                        \
do {                                                                         \
	if (AUDIO_TEST_LOG_PROC_PRINT <=                                     \
		AUDIO_TEST_LOG_BYTE_LOW(audio_test_log_level)) {             \
		if (audio_test_log_level & AUDIO_TEST_LOG_BIT_DMESG) {       \
			pr_err(AUDIO_TEST_DRV_NAME " : %s():[INF]:"          \
				fmt, __func__, ##__VA_ARGS__);               \
		} else {                                                     \
			pr_alert(AUDIO_TEST_DRV_NAME " : %s():[INF]:"        \
				fmt, __func__, ##__VA_ARGS__);               \
		}                                                            \
	}                                                                    \
} while (0)

#define audio_test_log_debug(fmt, ...)                                       \
do {                                                                         \
	if (AUDIO_TEST_LOG_DEBUG_PRINT <=                                    \
		AUDIO_TEST_LOG_BYTE_LOW(audio_test_log_level)) {             \
		if (audio_test_log_level & AUDIO_TEST_LOG_BIT_DMESG) {       \
			pr_err(AUDIO_TEST_DRV_NAME " : %s():[DBG]:"          \
				fmt, __func__, ##__VA_ARGS__);               \
		} else {                                                     \
			pr_alert(AUDIO_TEST_DRV_NAME " : %s():[DBG]:"        \
				fmt, __func__, ##__VA_ARGS__);               \
		}                                                            \
	}                                                                    \
} while (0)

#define audio_test_log_efunc(fmt, ...)                                       \
do {                                                                         \
	if (AUDIO_TEST_LOG_FUNC_PRINT <=                                     \
		AUDIO_TEST_LOG_BYTE_LOW(audio_test_log_level)) {             \
		if (audio_test_log_level & AUDIO_TEST_LOG_BIT_DMESG) {       \
			pr_err(AUDIO_TEST_DRV_NAME " : %s():[ENT]:"          \
				fmt, __func__, ##__VA_ARGS__);               \
		} else {                                                     \
			pr_alert(AUDIO_TEST_DRV_NAME " : %s():[ENT]:"        \
				fmt, __func__, ##__VA_ARGS__);               \
		}                                                            \
	}                                                                    \
} while (0)

#define audio_test_log_rfunc(fmt, ...)                                       \
do {                                                                         \
	if (AUDIO_TEST_LOG_FUNC_PRINT <=                                     \
		AUDIO_TEST_LOG_BYTE_LOW(audio_test_log_level)) {             \
		if (audio_test_log_level & AUDIO_TEST_LOG_BIT_DMESG) {       \
			pr_err(AUDIO_TEST_DRV_NAME " : %s():[RTN]:"          \
				fmt, __func__, ##__VA_ARGS__);               \
		} else {                                                     \
			pr_alert(AUDIO_TEST_DRV_NAME " : %s():[RTN]:"        \
				fmt, __func__, ##__VA_ARGS__);               \
		}                                                            \
	}                                                                    \
} while (0)

#endif  /* __PRN_AUDIO_TEST_ADD_TIME__ */
#else   /* != __PRN_AUDIO_TEST__ */

#define audio_test_log_reg_dump(fmt, ...)       do { } while (0)
#define audio_test_log_err(fmt, ...)            do { } while (0)
#define audio_test_log_info(fmt, ...)           do { } while (0)
#define audio_test_log_debug(fmt, ...)          do { } while (0)
#define audio_test_log_efunc(fmt, ...)          do { } while (0)
#define audio_test_log_rfunc(fmt, ...)          do { } while (0)

#endif	/* __PRN_AUDIO_TEST__ */

#endif  /* __AUDIO_TEST_WM1811_H__ */
