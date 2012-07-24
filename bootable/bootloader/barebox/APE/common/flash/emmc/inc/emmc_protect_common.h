/*	emmc_protect_common.h								*/
/*														*/
/* Copyright (C) 2012, Renesas Mobile Corp.		        */
/* All rights reserved.									*/
/*														*/

#ifndef _EMMC_PROTECT_COMMON_H_
#define _EMMC_PROTECT_COMMON_H_


/*****************************************************************************
; Mode Definition
******************************************************************************/
#define MMC_PROTECT_OPERATION	0
#define MMC_PROTECT_OPE_WRITE	1
#define MMC_PROTECT_OPE_CLEAR	2


RC ProtectOperation(unsigned long start_addr, unsigned long end_addr, unsigned int ope);
unsigned long flash_Get_Sector_From_WPGroup(ulong wp_group);

#endif
