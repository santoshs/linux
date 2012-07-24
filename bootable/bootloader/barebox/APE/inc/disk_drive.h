/*
 * disk_drive.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef __DISK_DRIVE_H__
#define __DISK_DRIVE_H__

#include "common.h"
#include "flash_api.h"
#include "divmod.h"

#define SECTOR_SIZE 512ul

/* Description of one partition table entry (D*S type) */
typedef struct {
	unsigned char boot_indicator;
	unsigned char chs_begin[3];
	unsigned char type;
	unsigned char chs_end[3];
	unsigned long partition_start;
	unsigned long partition_size;
} partition_entry ;

/* Description of GPT header */
typedef struct {
	uint64 signature;
	ulong revision;
	ulong header_size;
	ulong header_crc32;
	ulong reserved;
	uint64 my_lba;
	uint64 alternate_lba;
	uint64 first_usable_lba;
	uint64 last_usable_lba;
	uchar  disk_guid[16];
	uint64 partition_entry_lba;
	ulong number_of_partition_entries;
	ulong size_of_partition_entry;
	ulong partition_entry_array_crc32;
	uchar  reserved2[SECTOR_SIZE - 92];
} gpt_header;

/* Description of GPT partition entry array */
typedef struct {
	uchar  partition_type_guid[16];
	uchar  unique_partition_guid[16];
	uint64 starting_lba;
	uint64 ending_lba;
	uint64 attributes;
	uchar  partition_name[72];
} gpt_entry_array;

/* Description of one partition information */
typedef struct {
	unsigned long long address;
	unsigned long size;
	unsigned char order;
} partition_info ;

/* Function Prototypes */
RC disk_get_partition(unsigned char order, partition_info *part);
#endif	/* __DISK_DRIVE_H__ */
