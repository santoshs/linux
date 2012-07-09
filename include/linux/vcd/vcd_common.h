/* vcd_common.h
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
#ifndef __VCD_COMMON_H__
#define __VCD_COMMON_H__

#include "linux/vcd/vcd.h"
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/time.h>

/*
 * global variable declaration
 */
extern unsigned int g_vcd_log_level;


/*
 * constant definition
 */
/* driver name */
#define VCD_DRIVER_NAME			"vcd"
#define VCD_DEVICE_NAME			"vcd_mem"

/* return values */
#define VCD_ERR_NONE			0
#define VCD_ERR_FILE_NOT_FOUND		(-ENOENT)
#define VCD_ERR_NOMEMORY		(-ENOMEM)
#define VCD_ERR_BUSY			(-EBUSY)
#define VCD_ERR_PARAM			(-EINVAL)
#define VCD_ERR_FILE_TOO_BIG		(-EFBIG)
#define VCD_ERR_NOT_SUPPORT		(-ENOTSUPP)
#define VCD_ERR_NOT_ACTIVE		(-EWOULDBLOCK)
#define VCD_ERR_FW_TIME_OUT		(-ETIMEDOUT)
#define VCD_ERR_ALREADY_EXECUTION	(-EALREADY)
#define VCD_ERR_NOW_EXECUTION		(-EINPROGRESS)
#define VCD_ERR_SYSTEM			(-EFAULT)


/* log level */
#define VCD_LOG_NONE			0x00000000
#define VCD_LOG_ERROR			0x00000001
#define VCD_LOG_IF			0x00000002
#define VCD_LOG_STATUS_CHANGE		0x00000004
#define VCD_LOG_LEVEL_UPPER		0x00000007
#define VCD_LOG_USER_IF			0x00000008
#define VCD_LOG_SPUV_IF			0x00000008
#define VCD_LOG_INTERFACE_INFO		0x00000010
#define VCD_LOG_CONTROL_INFO		0x00000020
#define VCD_LOG_SPUV_INFO		0x00000040
#define VCD_LOG_INTERFACE_DEBUG		0x00000100
#define VCD_LOG_CONTROL_DEBUG		0x00000200
#define VCD_LOG_SPUV_DEBUG		0x00000400
#define VCD_LOG_INTERFACE_FUNCTION	0x00001000
#define VCD_LOG_CONTROL_FUNCTION	0x00002000
#define VCD_LOG_SPUV_FUNCTION		0x00004000
#define VCD_LOG_REGISTERS_DUMP		0x00008000
#define VCD_LOG_ON_SYSTEM_INFO_IND	0x00100000
#define VCD_LOG_ON_UDATA_IND		0x00200000
#define VCD_LOG_ON_TRIGGER_REC_IND	0x00400000
#define VCD_LOG_ON_TRIGGER_PLAY_IND	0x00800000
#define VCD_LOG_LEVEL_CONDITION_LOCK	0x40000000
#define VCD_LOG_LEVEL_LOCK		0x80000000


/*
 * define macro declaration
 */

/* panic macro */
#define vcd_kern_fault(fmt, ...)	panic(fmt "(%d)", ##__VA_ARGS__)

/* dynamic log level change */
#ifdef __PRINT_VCD__
	#define vcd_pr_always(fmt, ...) \
		struct timeval tv; \
		do_gettimeofday(&tv); \
		pr_alert( \
			"[%5ld.%06ld] " \
			VCD_DRIVER_NAME \
			" : %s: " \
			fmt, \
			tv.tv_sec, \
			tv.tv_usec, \
			__func__, \
			##__VA_ARGS__)

	#define vcd_pr_always_err(fmt, ...) \
		struct timeval tv; \
		do_gettimeofday(&tv); \
		pr_alert( \
			"[%5ld.%06ld] " \
			VCD_DRIVER_NAME \
			" : %s: " \
			fmt, \
			tv.tv_sec, \
			tv.tv_usec, \
			__func__, \
			##__VA_ARGS__)

	#define vcd_pr_err(fmt, ...) \
		if (g_vcd_log_level & VCD_LOG_ERROR) { \
			struct timeval tv; \
			do_gettimeofday(&tv); \
			pr_alert( \
				"[%5ld.%06ld] " \
				VCD_DRIVER_NAME \
				" : [error] %s: " \
				fmt, \
				tv.tv_sec, \
				tv.tv_usec, \
				__func__, \
				##__VA_ARGS__); \
		}

	#define vcd_pr_if_audio(fmt, ...) \
		if (g_vcd_log_level & VCD_LOG_IF) { \
			struct timeval tv; \
			do_gettimeofday(&tv); \
			pr_alert( \
				"[%5ld.%06ld] " \
				VCD_DRIVER_NAME \
				" : [VCD IF - AUDIO] : " \
				fmt, \
				tv.tv_sec, \
				tv.tv_usec, \
				##__VA_ARGS__); \
		}

	#define vcd_pr_if_sound(fmt, ...) \
		if (g_vcd_log_level & VCD_LOG_IF) { \
			struct timeval tv; \
			do_gettimeofday(&tv); \
			pr_alert( \
				"[%5ld.%06ld] " \
				VCD_DRIVER_NAME \
				" : [VCD IF - SOUND] : " \
				fmt, \
				tv.tv_sec, \
				tv.tv_usec, \
				##__VA_ARGS__); \
		}

	#define vcd_pr_if_amhal(fmt, ...) \
		if (g_vcd_log_level & VCD_LOG_IF) { \
			struct timeval tv; \
			do_gettimeofday(&tv); \
			pr_alert( \
				"[%5ld.%06ld] " \
				VCD_DRIVER_NAME \
				" : [VCD IF - AMHAL] : " \
				fmt, \
				tv.tv_sec, \
				tv.tv_usec, \
				##__VA_ARGS__); \
		}

	#define vcd_pr_if_spuv(fmt, ...) \
		if (g_vcd_log_level & VCD_LOG_IF) { \
			struct timeval tv; \
			do_gettimeofday(&tv); \
			pr_alert( \
				"[%5ld.%06ld] " \
				VCD_DRIVER_NAME \
				" : [VCD IF - SPUV ] : " \
				fmt, \
				tv.tv_sec, \
				tv.tv_usec, \
				##__VA_ARGS__); \
		}

	#define vcd_pr_start_if_user(fmt, ...) \
		if (g_vcd_log_level & VCD_LOG_USER_IF) { \
			struct timeval tv; \
			do_gettimeofday(&tv); \
			pr_alert( \
				"[%5ld.%06ld] " \
				VCD_DRIVER_NAME \
				" : [start] %s: " \
				fmt, \
				tv.tv_sec, \
				tv.tv_usec, \
				__func__, \
				##__VA_ARGS__); \
		}

	#define vcd_pr_end_if_user(fmt, ...) \
		if (g_vcd_log_level & VCD_LOG_USER_IF) { \
			struct timeval tv; \
			do_gettimeofday(&tv); \
			pr_alert( \
				"[%5ld.%06ld] " \
				VCD_DRIVER_NAME \
				" : [ end ] %s: " \
				fmt, \
				tv.tv_sec, \
				tv.tv_usec, \
				__func__, \
				##__VA_ARGS__); \
		}

	#define vcd_pr_start_if_spuv(fmt, ...) \
		if (g_vcd_log_level & VCD_LOG_SPUV_IF) { \
			struct timeval tv; \
			do_gettimeofday(&tv); \
			pr_alert( \
				"[%5ld.%06ld] " \
				VCD_DRIVER_NAME \
				" : [start] %s: " \
				fmt, \
				tv.tv_sec, \
				tv.tv_usec, \
				__func__, \
				##__VA_ARGS__); \
		}

	#define vcd_pr_end_if_spuv(fmt, ...) \
		if (g_vcd_log_level & VCD_LOG_SPUV_IF) { \
			struct timeval tv; \
			do_gettimeofday(&tv); \
			pr_alert( \
				"[%5ld.%06ld] " \
				VCD_DRIVER_NAME \
				" : [ end ] %s: " \
				fmt, \
				tv.tv_sec, \
				tv.tv_usec, \
				__func__, \
				##__VA_ARGS__); \
		}

	#define vcd_pr_status_change(fmt, ...) \
		if (g_vcd_log_level & VCD_LOG_STATUS_CHANGE) { \
			struct timeval tv; \
			do_gettimeofday(&tv); \
			pr_alert( \
				"[%5ld.%06ld] " \
				VCD_DRIVER_NAME \
				" : [state] %s: " \
				fmt, \
				tv.tv_sec, \
				tv.tv_usec, \
				__func__, \
				##__VA_ARGS__); \
		}

	#define vcd_pr_interface_info(fmt, ...) \
		if (g_vcd_log_level & VCD_LOG_INTERFACE_INFO) { \
			struct timeval tv; \
			do_gettimeofday(&tv); \
			pr_alert( \
				"[%5ld.%06ld] " \
				VCD_DRIVER_NAME \
				" : [info ] %s: " \
				fmt, \
				tv.tv_sec, \
				tv.tv_usec, \
				__func__, \
				##__VA_ARGS__); \
		}

	#define vcd_pr_control_info(fmt, ...) \
		if (g_vcd_log_level & VCD_LOG_CONTROL_INFO) { \
			struct timeval tv; \
			do_gettimeofday(&tv); \
			pr_alert( \
				"[%5ld.%06ld] " \
				VCD_DRIVER_NAME \
				" : [info ] %s: " \
				fmt, \
				tv.tv_sec, \
				tv.tv_usec, \
				__func__, \
				##__VA_ARGS__); \
		}

	#define vcd_pr_spuv_info(fmt, ...) \
		if (g_vcd_log_level & VCD_LOG_SPUV_INFO) { \
			struct timeval tv; \
			do_gettimeofday(&tv); \
			pr_alert( \
				"[%5ld.%06ld] " \
				VCD_DRIVER_NAME \
				" : [info ] %s: " \
				fmt, \
				tv.tv_sec, \
				tv.tv_usec, \
				__func__, \
				##__VA_ARGS__); \
		}

	#define vcd_pr_if_spuv_system_info_ind(fmt, ...) \
		if ((g_vcd_log_level & VCD_LOG_SPUV_INFO) && \
			(g_vcd_log_level & VCD_LOG_ON_SYSTEM_INFO_IND)) { \
			struct timeval tv; \
			do_gettimeofday(&tv); \
			pr_alert( \
				"[%5ld.%06ld] " \
				VCD_DRIVER_NAME \
				" : [info ] %s: " \
				fmt, \
				tv.tv_sec, \
				tv.tv_usec, \
				__func__, \
				##__VA_ARGS__); \
		}

	#define vcd_pr_if_spuv_udata_ind(fmt, ...) \
		if ((g_vcd_log_level & VCD_LOG_SPUV_INFO) && \
			(g_vcd_log_level & VCD_LOG_ON_UDATA_IND)) { \
			struct timeval tv; \
			do_gettimeofday(&tv); \
			pr_alert( \
				"[%5ld.%06ld] " \
				VCD_DRIVER_NAME \
				" : [info ] %s: " \
				fmt, \
				tv.tv_sec, \
				tv.tv_usec, \
				__func__, \
				##__VA_ARGS__); \
		}

	#define vcd_pr_if_spuv_trigger_rec_ind(fmt, ...) \
		if ((g_vcd_log_level & VCD_LOG_SPUV_INFO) && \
			(g_vcd_log_level & VCD_LOG_ON_TRIGGER_REC_IND)) { \
			struct timeval tv; \
			do_gettimeofday(&tv); \
			pr_alert( \
				"[%5ld.%06ld] " \
				VCD_DRIVER_NAME \
				" : [info ] %s: " \
				fmt, \
				tv.tv_sec, \
				tv.tv_usec, \
				__func__, \
				##__VA_ARGS__); \
		}

	#define vcd_pr_if_spuv_trigger_play_ind(fmt, ...) \
		if ((g_vcd_log_level & VCD_LOG_SPUV_INFO) && \
			(g_vcd_log_level & VCD_LOG_ON_TRIGGER_PLAY_IND)) { \
			struct timeval tv; \
			do_gettimeofday(&tv); \
			pr_alert( \
				"[%5ld.%06ld] " \
				VCD_DRIVER_NAME \
				" : [info ] %s: " \
				fmt, \
				tv.tv_sec, \
				tv.tv_usec, \
				__func__, \
				##__VA_ARGS__); \
		}

	#define vcd_pr_interface_debug(fmt, ...) \
		if (g_vcd_log_level & VCD_LOG_INTERFACE_DEBUG) { \
			struct timeval tv; \
			do_gettimeofday(&tv); \
			pr_alert( \
				"[%5ld.%06ld] " \
				VCD_DRIVER_NAME \
				" : [debug] %s: " \
				fmt, \
				tv.tv_sec, \
				tv.tv_usec, \
				__func__, \
				##__VA_ARGS__); \
		}

	#define vcd_pr_control_debug(fmt, ...) \
		if (g_vcd_log_level & VCD_LOG_CONTROL_DEBUG) { \
			struct timeval tv; \
			do_gettimeofday(&tv); \
			pr_alert( \
				"[%5ld.%06ld] " \
				VCD_DRIVER_NAME \
				" : [debug] %s: " \
				fmt, \
				tv.tv_sec, \
				tv.tv_usec, \
				__func__, \
				##__VA_ARGS__); \
		}

	#define vcd_pr_spuv_debug(fmt, ...) \
		if (g_vcd_log_level & VCD_LOG_SPUV_DEBUG) { \
			struct timeval tv; \
			do_gettimeofday(&tv); \
			pr_alert( \
				"[%5ld.%06ld] " \
				VCD_DRIVER_NAME \
				" : [debug] %s: " \
				fmt, \
				tv.tv_sec, \
				tv.tv_usec, \
				__func__, \
				##__VA_ARGS__); \
		}

	#define vcd_pr_start_interface_function(fmt, ...) \
		if (g_vcd_log_level & VCD_LOG_INTERFACE_FUNCTION) { \
			struct timeval tv; \
			do_gettimeofday(&tv); \
			pr_alert( \
				"[%5ld.%06ld] " \
				VCD_DRIVER_NAME \
				" : [start] %s: " \
				fmt, \
				tv.tv_sec, \
				tv.tv_usec, \
				__func__, \
				##__VA_ARGS__); \
		}

	#define vcd_pr_end_interface_function(fmt, ...) \
		if (g_vcd_log_level & VCD_LOG_INTERFACE_FUNCTION) { \
			struct timeval tv; \
			do_gettimeofday(&tv); \
			pr_alert( \
				"[%5ld.%06ld] " \
				VCD_DRIVER_NAME \
				" : [ end ] %s: " \
				fmt, \
				tv.tv_sec, \
				tv.tv_usec, \
				__func__, \
				##__VA_ARGS__); \
		}

	#define vcd_pr_start_control_function(fmt, ...) \
		if (g_vcd_log_level & VCD_LOG_CONTROL_FUNCTION) { \
			struct timeval tv; \
			do_gettimeofday(&tv); \
			pr_alert( \
				"[%5ld.%06ld] " \
				VCD_DRIVER_NAME \
				" : [start] %s: " \
				fmt, \
				tv.tv_sec, \
				tv.tv_usec, \
				__func__, \
				##__VA_ARGS__); \
		}

	#define vcd_pr_end_control_function(fmt, ...) \
		if (g_vcd_log_level & VCD_LOG_CONTROL_FUNCTION) { \
			struct timeval tv; \
			do_gettimeofday(&tv); \
			pr_alert( \
				"[%5ld.%06ld] " \
				VCD_DRIVER_NAME \
				" : [ end ] %s: " \
				fmt, \
				tv.tv_sec, \
				tv.tv_usec, \
				__func__, \
				##__VA_ARGS__); \
		}

	#define vcd_pr_start_spuv_function(fmt, ...) \
		if (g_vcd_log_level & VCD_LOG_SPUV_FUNCTION) { \
			struct timeval tv; \
			do_gettimeofday(&tv); \
			pr_alert( \
				"[%5ld.%06ld] " \
				VCD_DRIVER_NAME \
				" : [start] %s: " \
				fmt, \
				tv.tv_sec, \
				tv.tv_usec, \
				__func__, \
				##__VA_ARGS__); \
		}

	#define vcd_pr_end_spuv_function(fmt, ...) \
		if (g_vcd_log_level & VCD_LOG_SPUV_FUNCTION) { \
			struct timeval tv; \
			do_gettimeofday(&tv); \
			pr_alert( \
				"[%5ld.%06ld] " \
				VCD_DRIVER_NAME \
				" : [ end ] %s: " \
				fmt, \
				tv.tv_sec, \
				tv.tv_usec, \
				__func__, \
				##__VA_ARGS__); \
		}

	#define vcd_pr_registers_dump(fmt, ...) \
		if (g_vcd_log_level & VCD_LOG_REGISTERS_DUMP) { \
			struct timeval tv; \
			do_gettimeofday(&tv); \
			pr_alert( \
				"[%5ld.%06ld] " \
				VCD_DRIVER_NAME \
				" : [ reg ] %s: " \
				fmt, \
				tv.tv_sec, \
				tv.tv_usec, \
				__func__, \
				##__VA_ARGS__); \
		}
#else
	#define vcd_pr_always(fmt, ...)				{}
	#define vcd_pr_always_err(fmt, ...)			{}
	#define vcd_pr_err(fmt, ...)				{}
	#define vcd_pr_if_audio(fmt, ...)			{}
	#define vcd_pr_if_sound(fmt, ...)			{}
	#define vcd_pr_if_amhal(fmt, ...)			{}
	#define vcd_pr_if_spuv(fmt, ...)			{}
	#define vcd_pr_if_spuv_system_info_ind(fmt, ...)	{}
	#define vcd_pr_if_spuv_udata_ind(fmt, ...)		{}
	#define vcd_pr_if_spuv_trigger_rec_ind(fmt, ...)	{}
	#define vcd_pr_if_spuv_trigger_play_ind(fmt, ...)	{}
	#define vcd_pr_start_if_user(fmt, ...)			{}
	#define vcd_pr_end_if_user(fmt, ...)			{}
	#define vcd_pr_start_if_spuv(fmt, ...)			{}
	#define vcd_pr_end_if_spuv(fmt, ...)			{}
	#define vcd_pr_status_change(fmt, ...)			{}
	#define vcd_pr_interface_info(fmt, ...)			{}
	#define vcd_pr_control_info(fmt, ...)			{}
	#define vcd_pr_spuv_info(fmt, ...)			{}
	#define vcd_pr_interface_debug(fmt, ...)		{}
	#define vcd_pr_control_debug(fmt, ...)			{}
	#define vcd_pr_spuv_debug(fmt, ...)			{}
	#define vcd_pr_start_interface_function(fmt, ...)	{}
	#define vcd_pr_end_interface_function(fmt, ...)		{}
	#define vcd_pr_start_control_function(fmt, ...)		{}
	#define vcd_pr_end_control_function(fmt, ...)		{}
	#define vcd_pr_start_spuv_function(fmt, ...)		{}
	#define vcd_pr_end_spuv_function(fmt, ...)		{}
	#define vcd_pr_registers_dump(fmt, ...)			{}
#endif  /* __PRINT_VCD__ */


/*
 * enum declaration
 */


/*
 * structure declaration
 */


/*
 * table declaration
 */


/*
 * prototype declaration
 */


#endif /* __VCD_COMMON_H__ */
