/*
 * disk_drive.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "disk_drive.h"

RC disk_get_partition(unsigned char order, partition_info *part)
{
	partition_entry entry;
	unsigned long long ebr_base_address;
	RC ret;
	uchar part_type;	/* 0: MBR/EBR - 1: GPT */
	gpt_header gpt_hdr;
	gpt_entry_array gpt_entry;
	ulong entry_num;	/* number of entry in a LBA */
	ulong sub_index;
	ulong index;
	
	/* Read type of first partition */
	ret = Flash_Access_Read((uchar *)&part_type, sizeof(uchar), 0x01C2, UNUSED);
	if(ret != FLASH_ACCESS_SUCCESS)
	{
		return ret;
	}
	/* BEGIN: CR722: Apply GPT */	
	if(part_type != 0xEE)	/* MBR/EBR */
	{
		if(order < 4)
		{
			/* Read partition address */
			ret = Flash_Access_Read((uchar *)&entry, sizeof(partition_entry), 0x01be + (sizeof(partition_entry) * (order - 1)), UNUSED);
			if(ret != FLASH_ACCESS_SUCCESS)
			{
				return ret;
			}
			
			(*part).address = entry.partition_start * SECTOR_SIZE;
			(*part).size = entry.partition_size * SECTOR_SIZE;
			(*part).order = order;
		}
		else	/* order >= 4 */
		{
			/* Read EBR based address */
			ret = Flash_Access_Read((uchar *)&entry, sizeof(partition_entry), 0x01be + (sizeof(partition_entry) * 3), UNUSED);
			if(ret != FLASH_ACCESS_SUCCESS)
			{
				return ret;
			}
			
			/* Get value of EBR based address */
			ebr_base_address = entry.partition_start * SECTOR_SIZE;
					
			/* Read partition address */
			ret = Flash_Access_Read((uchar *)&entry, sizeof(partition_entry), (ebr_base_address + (SECTOR_SIZE * (order - 5)) + 0x01be), UNUSED);
			if(ret != FLASH_ACCESS_SUCCESS)
			{
				return ret;
			}
			
			(*part).address = (entry.partition_start + order - 5) * SECTOR_SIZE + ebr_base_address;
			(*part).size = entry.partition_size * SECTOR_SIZE;
			(*part).order = order;
		}
	}
	else	/* GPT */
	{
		/* Read Primary GPT header */
		ret = Flash_Access_Read((uchar *)&gpt_hdr, sizeof(gpt_header), 0x200, UNUSED);
		if(ret != FLASH_ACCESS_SUCCESS)
		{
			return ret;
		}
		
		/* Check input parameter */
		if(order < 1 || order > gpt_hdr.number_of_partition_entries)
			return -1;
		
		/* Get number of the entry information in 1LBA */
		entry_num = bi_div(SECTOR_SIZE, gpt_hdr.size_of_partition_entry);
		
		/* Read GPT partition entry */
		if(order <= entry_num)
		{
			index = order - 1;
			ret = Flash_Access_Read((uchar *)&gpt_entry, sizeof(gpt_entry_array), (0x400 + ( index * sizeof(gpt_entry_array))), UNUSED);
			if(ret != FLASH_ACCESS_SUCCESS)
			{
				return ret;
			}
		}
		else
		{
			index = bi_div((order - 1) , entry_num);
			sub_index = div_mode((order - 1), entry_num);
			ret = Flash_Access_Read((uchar *)&gpt_entry, sizeof(gpt_entry_array), (0x400 +  (index * SECTOR_SIZE) + (sub_index * sizeof(gpt_entry_array))), UNUSED);
			if(ret != FLASH_ACCESS_SUCCESS)
			{
				return ret;
			}
		}
		
		(*part).address = gpt_entry.starting_lba * SECTOR_SIZE;
		(*part).size = ((gpt_entry.ending_lba + 1) -  gpt_entry.starting_lba )* SECTOR_SIZE;
		(*part).order = order;
		
	}
	/* END: CR722: Apply GPT */
	return ret;	
}
