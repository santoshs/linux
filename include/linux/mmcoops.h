/*	mmcoops.h															*/
/*																		*/
/* Copyright (C) 2011 Renesas Mobile Corp.								*/
/* All rights reserved.													*/
/*																		*/

#ifndef __MMCOOPS_H
#define __MMCOOPS_H

#include <linux/platform_device.h>
#include <mach/crashlog.h>

#define MMCOOPS_RECORD_CNT			3
#define MMCOOPS_HEADER_SIZE			1
#define MMCOOPS_FOOTER_SIZE			1
#define MMCOOPS_KMSG_SIZE			256
#define MMCOOPS_LOGCAT_MAIN_SIZE		512
#define MMCOOPS_LOGCAT_SYSTEM_SIZE		512
#define MMCOOPS_LOGCAT_RADIO_SIZE		512
#define MMCOOPS_LOGCAT_EVENTS_SIZE		512
#define MMCOOPS_RECORD_SIZE			(MMCOOPS_HEADER_SIZE        \
						+MMCOOPS_KMSG_SIZE          \
						+MMCOOPS_LOGCAT_MAIN_SIZE   \
						+MMCOOPS_LOGCAT_SYSTEM_SIZE \
						+MMCOOPS_LOGCAT_RADIO_SIZE  \
						+MMCOOPS_LOGCAT_EVENTS_SIZE \
						+MMCOOPS_FOOTER_SIZE)

#define MMCOOPS_KMSG_SIZE_DDR			256
#define MMCOOPS_LOGCAT_MAIN_SIZE_DDR	256
#define MMCOOPS_LOGCAT_SYSTEM_SIZE_DDR	256
#define MMCOOPS_LOGCAT_RADIO_SIZE_DDR	256
#define MMCOOPS_LOGCAT_EVENTS_SIZE_DDR	256

#define MMCOOPS_LOCAL_VERSION	0x01150612		// 0x01ddmmyy (release time)
#define MMCOOPS_START_OFFSET	2654208	/* mmcblk0p10 */
#define MMCOOPS_START_OFFSET_DDR	0x44841200	/* DDR Address */
#define MMCOOPS_LOG_SIZE		(MMCOOPS_RECORD_SIZE * MMCOOPS_RECORD_CNT)

#define MAX_LOG_SIZE_ON_DDR	(0xBFE00)

struct mmcoops_platform_data {
	struct platform_device	*pdev;
	unsigned long		start;
	unsigned long		start_ddr;
	unsigned long		size;
	unsigned long		record_size;
	unsigned long		kmsg_size;
	unsigned long		logcat_main_size;
	unsigned long		logcat_system_size;
	unsigned long		logcat_radio_size;
	unsigned long		logcat_events_size;
	unsigned long		kmsg_size_ddr;
	unsigned long		logcat_main_size_ddr;
	unsigned long		logcat_system_size_ddr;
	unsigned long		logcat_radio_size_ddr;
	unsigned long		logcat_events_size_ddr;
	u32			local_version;
	char			soft_version[CRASHLOG_R_LOCAL_VER_LENGTH];
};

#endif
