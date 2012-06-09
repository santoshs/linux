/*	mmcoops.h															*/
/*																		*/
/* Copyright (C) 2011 Renesas Mobile Corp.								*/
/* All rights reserved.													*/
/*																		*/

#ifndef __MMCOOPS_H
#define __MMCOOPS_H

#include <linux/platform_device.h>

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

#define MMCOOPS_LOCAL_VERSION	0x01080612		// 0x01ddmmyy (release time)
#define MMCOOPS_START_OFFSET	2654208	/* mmcblk0p10 */
#define MMCOOPS_LOG_SIZE		(MMCOOPS_RECORD_SIZE * MMCOOPS_RECORD_CNT)

struct mmcoops_platform_data {
	struct platform_device	*pdev;
	unsigned long		start;
	unsigned long		size;
	unsigned long		record_size;
	unsigned long		kmsg_size;
	unsigned long		logcat_main_size;
	unsigned long		logcat_system_size;
	unsigned long		logcat_radio_size;
	unsigned long		logcat_events_size;
	u32			local_version;
	char			soft_version[32];
};

#endif
