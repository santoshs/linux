/*	emmc.h												*/
/*														*/
/* Copyright (C) 2012, Renesas Mobile Corp.		        */
/* All rights reserved.									*/
/*														*/

#ifndef _EMMC_H_
#define _EMMC_H_

#include "com_type.h"

/*****************************************************************************
; Error Code
******************************************************************************/
#define EMMC_SUCCESS			(0)		/* Success */
#define EMMC_ERR_PARAM			(-1)	/* Parameter Error */
#define EMMC_ERR_MOUNT			(-2)	/* Mount Error */
#define EMMC_ERR_HIGH_SPEED		(-3)	/* High Speed Mode Error */
#define EMMC_ERR_BUSWIDTH		(-4)	/* Bus Width Setting Error */
#define EMMC_ERR_RESPONSE		(-5)	/* CMD Response Error */
#define EMMC_ERR_RESPONSE_BUSY	(-6)	/* CMD Response Busy */
#define EMMC_ERR_TRANSFER		(-7)	/* Data Transfer Error */
#define EMMC_ERR_STATE			(-8)	/* State Error */
#define EMMC_ERR_TIMEOUT		(-9)	/* Timeout Error */
#define EMMC_ERR_ILLEGAL_CARD	(-10)	/* Illegal card */
#define EMMC_ERR_CARD_BUSY		(-11)	/* Card Error */
#define EMMC_ERR_CRC			(-12)	/* CRC Error */
#define EMMC_ERR_CMD_ISSUE		(-13)	/* CMD Issue Error */
#define EMMC_ERR_BUFFER_ACCESS	(-14)	/* Buffer Access Error */


/*****************************************************************************
; Data Definition
******************************************************************************/
/* Sector size */
#define EMMC_BLOCK_LENGTH		(512)			/* 1sector = 512byte */
#define EMMC_SECTOR_SIZE_SHIFT	(9)				/* 512 = 2^9 */
#define EMMC_BLOCK_LENGTH_DW	(128)			/* 512 = 128x4 */
	
#if 0	/* Don't check MAX */
#define EMMC_MAX_SECTOR_COUNT_USER	(0x00EC0000L)	/* number of sector (user data = EXT_CSD[215-212](SEC_COUNT)) */
													/* (Approximately 8GB) */
#endif	/* Don't check MAX */

#define EMMC_MAX_SECTOR_COUNT_BOOT1	(0x00000100L)	/* number of sector (boot partition 1 = 128kB) */
#define EMMC_MAX_SECTOR_COUNT_BOOT2	(0x00000100L)	/* number of sector (boot partition 2 = 128kB) */

#if 0	/* Don't check MAX */
#define EMMC_END_SECTOR_USER	(EMMC_MAX_SECTOR_COUNT_USER - 1)	/* end sector(user data)) */
#define EMMC_END_SECTOR_BOOT1	(EMMC_MAX_SECTOR_COUNT_BOOT1 - 1)	/* end sector(boot partition 1) */
#define EMMC_END_SECTOR_BOOT2	(EMMC_MAX_SECTOR_COUNT_BOOT2 - 1)	/* end sector(boot partition 2) */
#endif	/* Don't check MAX */


/* byte size */

typedef enum {
	EMMC_PARTITION_USER = 0,	/* user data area(default) */
	EMMC_PARTITION_BOOT1,		/* boot partition 1 */
	EMMC_PARTITION_BOOT2,		/* boot partition 2 */
	EMMC_PARTITION_MAX			/* The maximum of defined value */
} EMMC_PARTITION;


/*****************************************************************************
; API definition
******************************************************************************/
extern RC Emmc_Init(void);
extern RC Emmc_Mount(void);
extern void Emmc_Unmount(void);
extern RC Emmc_Read_Multi(uchar* pBuff, ulong start_sector, ulong sector_count);
extern RC Emmc_Read_Single(uchar* pBuff, ulong start_sector);
extern RC Emmc_Write_Multi(uchar* pBuff, ulong start_sector, ulong sector_count);
extern RC Emmc_Write_Single(uchar* pBuff, ulong start_sector);
extern RC Emmc_Erase(ulong start_sector, ulong sector_count);
extern RC Emmc_Format(void);
extern RC Emmc_Set_Partition(EMMC_PARTITION id);
extern RC Emmc_Write_Protect(ulong start_sector, ulong end_sector, ulong user_data);
extern RC Emmc_Clear_Protect(ulong start_sector, ulong end_sector, ulong user_data);
extern RC Emmc_Clear_Protect_For_Format(void);
extern unsigned long Emmc_Get_Max_Sector_Count(void);
extern unsigned long Emmc_Get_Write_Protect_Group_Size(void);
#endif /* _EMMC_H_ */
