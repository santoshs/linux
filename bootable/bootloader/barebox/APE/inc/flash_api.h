/* flash_api.
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef _FLASH_API_H_
#define _FLASH_API_H_

#include "com_type.h"

/*****************************************************************************
; Native Type
******************************************************************************/
#ifndef UNUSED
#define UNUSED				0
#endif


/*****************************************************************************
; Error Code
******************************************************************************/
#define FLASH_ACCESS_SUCCESS			(0)		/* Success */
#define FLASH_ACCESS_ERR_PARAM			(-1)	/* Parameter Error */
#define FLASH_ACCESS_ERR_NOT_MOUNT		(-2)	/* Not Mounted */
#define FLASH_ACCESS_ERR_ALREADY_MOUNT	(-4)	/* Alreadey Mounted */
#define FLASH_ACCESS_ERR_MOUNT			(-5)	/* Mount Failed */
#define FLASH_ACCESS_ERR_READ			(-7)	/* Read Failed */
#define FLASH_ACCESS_ERR_WRITE			(-8)	/* Write Failed */
#define FLASH_ACCESS_ERR_ERASE			(-9)	/* Erase Failed */
#define FLASH_ACCESS_ERR_FORMAT			(-10)	/* Format Failed */
#define FLASH_ACCESS_ERR_INIT			(-11)	/* Initialize Failed */
#define FLASH_ACCESS_ERR_NOT_INIT		(-12)	/* Not Initialized */
#define FLASH_ACCESS_ERR_WRITE_PROTECT	(-13)	/* Protect Failed */
#define FLASH_ACCESS_ERR_CLEAR_PROTECT	(-14)	/* Clear Protect Failed */
#define FLW_VERIFY_ERROR				(-15)	/* Verify data error */
/*****************************************************************************
; API Prototypes
******************************************************************************/
extern RC Flash_Access_Init(ulong param1);
extern RC Flash_Access_Mount(ulong param1);
extern RC Flash_Access_Unmount(ulong param1);
extern RC Flash_Access_Read(uchar* pBuff, ulong length, uint64 addr_start, ulong param1);
extern RC Flash_Access_Write(uchar* pBuff, ulong length, uint64 addr_start, ulong param1);
extern RC Flash_Access_Erase(ulong start_block, ulong end_block, ulong param1);
extern RC Flash_Access_Format(ulong param1);
extern RC Flash_Access_Change_Protect(ulong branch_mode);
extern RC Flash_Access_Write_Protect(ulong sect_start, ulong sect_end, ulong userdata);
extern RC Flash_Access_Clear_Protect(ulong sect_start, ulong sect_end, ulong userdata);
#endif /* _FLASH_API_H_ */
