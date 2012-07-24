/*	flash_clearprot.c									*/
/*														*/
/* Copyright (C) 2012, Renesas Mobile Corp.				*/
/* All rights reserved.									*/
/*														*/

#include "protect_option.h"
#include "flash_api.h"
#include "flash_private.h"


extern RC Emmc_Write_Protect_Clear(ulong , ulong );
ulong flash_Get_Sector_From_WPGroup(ulong);
ulong getshift(ulong num);

ulong g_shift_digit = 0;

/**
 * Flash_Access_Clear_Protect - eMMC write protection clear wrapper. 
 * @return FLASH_ACCESS_SUCCESS           : Success
 *         FLASH_ACCESS_ERR_PARAM         : Parameter error
 *         FLASH_ACCESS_ERR_MOUNT         : Mount error
 *         FLASH_ACCESS_ERR_NOT_INIT      : Initialize error
 *         FLASH_ACCESS_ERR_CLEAR_PROTECT : Error
 */
RC Flash_Access_Clear_Protect(ulong start_group, ulong end_group, ulong user_data)
{
	RC ret = FLASH_ACCESS_SUCCESS;

	ulong max_clear_protect_count = 0;
	ulong start_sector = 0;
	ulong end_sector = 0;

	/* check status */
	ret = flash_Judge_Mount();
	if (ret != FLASH_ACCESS_SUCCESS)
	{
		return ret;
	}

	if(user_data != ALL_USER_DATA_PROTECT)
	{
		max_clear_protect_count = flash_Get_Max_Protect_Group_Count();
		if(max_clear_protect_count == 0)
		{
			ret = FLASH_ACCESS_ERR_PARAM;
		}

		if(start_group > end_group)
		{
			ret = FLASH_ACCESS_ERR_PARAM;
		}

		if(end_group >= max_clear_protect_count)
		{
			ret = FLASH_ACCESS_ERR_PARAM;
		}
	}

	if(ret != FLASH_ACCESS_SUCCESS)
	{
		return ret;
	}

	start_sector = flash_Get_Sector_From_WPGroup(start_group);
	end_sector   = (flash_Get_Sector_From_WPGroup(end_group + 1) - 1);

	ret = Emmc_Clear_Protect( start_sector , end_sector, user_data );

	if(ret == EMMC_SUCCESS)
	{
		ret = FLASH_ACCESS_SUCCESS;		
	}
	else if(ret == EMMC_ERR_PARAM)
	{
		ret = FLASH_ACCESS_ERR_PARAM;
	}
	else
	{
		ret = FLASH_ACCESS_ERR_CLEAR_PROTECT;
	}

	return ret;
}



/**
 * flash_Get_Sector_From_WPGroup - Get sector from protect group. 
 * @return ulong : Sector address
 */
ulong flash_Get_Sector_From_WPGroup(ulong wp_group)
{
	ulong sector = 0L;

	/* get convert protect group into sector */
	sector = (ulong)(wp_group << g_shift_digit);
	
	return sector;
}



/**
 * flash_Get_Max_Protect_Group_Count - get max protect group count from eMMC.
 * @return unsigned long : max protect group count.
 */
ulong flash_Get_Max_Protect_Group_Count(void)
{
	unsigned long mask_a,mask_b;
	unsigned long group_sector_count = 0L;
	unsigned long max_sector_count = 0L;
	unsigned long max_group_count = 0L;
	
	max_sector_count = Emmc_Get_Max_Sector_Count();
	group_sector_count = Emmc_Get_Write_Protect_Group_Size();
#if 0
	if(group_sector_count == 0x1000)
	{
		/* In case of write protect group size is 2MByte */
		shift_digit = 12;
		mask_a = 0x00000FFF;
		mask_b = 0x000FFFFF;
	}
	else
	{
		return 0;	/* not supported */
	}

	if( (max_sector_count & mask_a) != 0)
	{
		return 0;	/* not supported */
	}
#else
	g_shift_digit = getshift(group_sector_count);
	mask_a = 0x00000FFF;
	mask_b = 0x000FFFFF;
	if( (max_sector_count & mask_a) != 0) {
		return 0;	/* not supported */
	}	
	max_group_count = ((max_sector_count >> g_shift_digit) & mask_b);
#endif
	return max_group_count;
}
ulong getshift(ulong num){
	ulong ret = -1;
	ulong temp = num;
	do {
		temp = (ulong)(temp >> 1);
		ret++;
	}
	while(temp != 0);
	
	return ret;
}
