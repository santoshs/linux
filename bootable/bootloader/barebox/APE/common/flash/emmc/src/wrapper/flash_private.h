/*	flash_private.h										*/
/*														*/
/* Copyright (C) 2012, Renesas Mobile Corp.				*/
/* All rights reserved.									*/
/*														*/

#ifndef _FLASH_PRIVATE_H_
#define _FLASH_PRIVATE_H_

#include "com_type.h"
#include "emmc.h"


/*****************************************************************************
; Data Definition
******************************************************************************/
/* temporary buffer byte size */
#define FLASH_ACCESS_MAX_TEMP_BUFF_SIZE		EMMC_BLOCK_LENGTH

#if 0	/* Don't check MAX */
/* maximum physical address */
#define PHYSICAL_MAX_ADDR_USER	(((uint64)EMMC_MAX_SECTOR_COUNT_USER << EMMC_SECTOR_SIZE_SHIFT) - 1)
#define PHYSICAL_MAX_ADDR_BOOT1	(((uint64)EMMC_MAX_SECTOR_COUNT_BOOT1 << EMMC_SECTOR_SIZE_SHIFT) - 1)
#define PHYSICAL_MAX_ADDR_BOOT2	(((uint64)EMMC_MAX_SECTOR_COUNT_BOOT2 << EMMC_SECTOR_SIZE_SHIFT) - 1)

/* maximum sector address */
#define SECTOR_MAX_ADDR_USER	EMMC_END_SECTOR_USER	/* end of sector (user data) */
#define SECTOR_MAX_ADDR_BOOT1	EMMC_END_SECTOR_BOOT1	/* end of sector (boot partition1) */
#define SECTOR_MAX_ADDR_BOOT2	EMMC_END_SECTOR_BOOT2	/* end of sector (boot partition2) */
#endif	/* Don't check MAX */

#define SECTOR_BYTE_SIZE	EMMC_BLOCK_LENGTH		/* byte size of 1 sector */
#define SECTOR_SHIFT_LENGTH	EMMC_SECTOR_SIZE_SHIFT	/* shift length of sector */

#define MIDDLE_IN_SECTOR_MASK	(0x00000000000001FF) /* address in the middle of a sector */

/* shift bits to convert block into sector */
#define ADDRESS_TO_SECTOR_SHIFT	(9)
#define BLOCK_TO_SECTOR_SHIFT	(10)

/* shift bits to convert write protect group into sector */
#define WP_GROUP_TO_SECTOR_SHIFT (12)

typedef enum {
	FLASH_ACCESS_STATE_START = 0,	/* Start State */
	FLASH_ACCESS_STATE_UNMOUNT,		/* Unmount State */
	FLASH_ACCESS_STATE_MOUNT,		/* Mount State */
	FLASH_ACCESS_STATE_MAX			/* The maximum of defined value */
} FLASH_ACCESS_STATE;


/*****************************************************************************
; Function definition
******************************************************************************/
extern RC flash_Judge_Start(void);
extern RC flash_Judge_Unmount(void);
extern RC flash_Judge_Mount(void);
extern void flash_Set_Status(FLASH_ACCESS_STATE status);
extern RC flash_Check_RW_Param(uchar *pBuff, ulong length, uint64 physical, EMMC_PARTITION part);
extern EMMC_PARTITION flash_Get_Partition_from_Blk(ulong block);
extern EMMC_PARTITION flash_Get_Partition_from_Addr(uint64 logical);
extern ulong flash_Get_Sector(ulong block, uint64 address, ulong data_size);
extern uint64 flash_Get_Physical_Addr(uint64 logical);
extern ulong flash_Get_Sector_From_WPGroup(ulong wp_group);
extern ulong flash_Get_Max_Protect_Group_Count(void);

#endif /* _FLASH_PRIVATE_H_ */
