/* fb_common.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef __FB_COMMON_H__
#define __FB_COMMON_H__
 
#include "log_output.h"
/*******************************
 * Definitions
 *******************************/
#define MAX_LEN_CMD				64
#define UNUSED					0
#define FB_VERSION				"0.4"
#define FB_PRODUCT				"Renesas"
#define FB_CPUREV				"EOS"
#define FB_DOWNLOADSIZE			"256 MB"
#define BOOTLOADER_VERSION 		"X.XX"
#define BASEBAND_VERSION 		"Y.YY"
#define SERIAL			 		"123456789"
#define MAX_PTN					32
#define MAX_LEN_ERASE			0x100000	/* 1MB */
#define WRITE_MAX_SIZE			32*1024*1024-512 /* 32MB - 512B (1block) */
#define MAX_DMA_TRANSFER_SIZE	0x20000000		/* Max data transfer count is 512MB */
/* Buffer addr */
#define SEND_BUFF_ADDR			0x43000000
#define COMMAND_BUFF_ADDR		0x43000040
#define DATA_BUFF_ADDR			COMMAND_BUFF_ADDR + 64
#define SPARSE_TEMP_BUFF		0x53000040

/* Reset Control Register2 */
#define RESCNT2					((volatile unsigned long*)(0xE6188020))
#define RESCNT2_PRES			(0x80000000)

/* Command ID  */
#define GETVAR					0x01
#define BOOT					0x02
#define REBOOT					0x03
#define REBOOT_BOOTLOADER		0x04
#define DOWNLOAD				0x05
#define FLASH					0x06
#define ERASE					0x07

#define BOOTLOADER				0x01
// #define BOOT					0x02
#define SYSTEM					0x03
#define USERDATA				0x04
#define RECOVERY				0x05
#define VERSION					0x06
#define PRODUCT					0x07
#define CPUREV					0x08
#define DOWNLOADSIZE			0x09
#define R_LOADER				0x0A
#define FASTBOOT				0x0B
#define VERSION_BOOTLOADER		0x0C
#define VERSION_BASEBAND		0x0D
#define SERIALNO				0x0E

 /* Error type */
#define FB_OK					 0	/* Success */
#define FB_ERR_PARAM			-1	/* Parameter Error */
#define FB_ERR_NOT_MOUNT		-2	/* Not Mounted */
#define FB_ERR_ALREADY_MOUNT 	-3 	/* Alreadey Mounted */
#define FB_ERR_MOUNT			-4	/* Mount Failed */
#define FB_ERR_READ				-5	/* Read Failed */
#define FB_ERR_WRITE			-6	/* Write Failed */
#define FB_ERR_ERASE			-7	/* Erase Failed */
#define FB_ERR_FORMAT		 	-8	/* Format Failed */
#define FB_ERR_INIT				-9	/* Initialize Failed */
#define FB_ERR_NOT_INIT			-10	/* Not Initialized */
#define FB_ERR_UNSUPPORT		-11	/* The input value is not support/found */
#define FB_ERR_EXISTED			-12 /* The input value is already existed */
#define FB_ERR_DISC				-13	/* Disconnect */
#define FB_ERR_OPEN				-14 /* Error opening device */
#define FB_ERR_ALREADY_OPEN		-15 /* Error device already opened */
#define FB_ERR_UNSUPPORT_PART	-16 /* Error partition unsupported */

/*******************************
 * Prototypes
 *******************************/

int fb_main_detect_addr(unsigned int value, unsigned long long *addr_start,
												unsigned long *size);
int fb_flash_init(void);

typedef struct fb_ptentry fb_ptentry;

/*
 *
 */
struct fb_ptentry {
	unsigned int id;
	
	unsigned long addr_start1;
	unsigned long addr_start2;
	unsigned long long addr_start;
	
	unsigned long size;
	unsigned int flags;
};

fb_ptentry *fb_flash_find_ptn(unsigned int id);

#endif /* __FB_COMMON_H__ */ 
