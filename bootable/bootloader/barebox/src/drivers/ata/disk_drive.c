/*
 * Copyright (C) 2009 Juergen Beisert, Pengutronix
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

/**
 * @file
 * @brief Generic disk drive support
 *
 * @todo Support for disks larger than 4 GiB
 * @todo Reliable size detection for BIOS based disks (on x86 only)
 */

#include <stdio.h>
#include <init.h>
#include <driver.h>
#include <types.h>
#include <ata.h>
#include <xfuncs.h>
#include <errno.h>
#include <string.h>
#include <linux/kernel.h>
#include <malloc.h>
#include <common.h>
#include <block.h>

/**
 * Description of one partition table entry (D*S type)
 */
struct partition_entry {
	uint8_t boot_indicator;
	uint8_t chs_begin[3];
	uint8_t type;
	uint8_t chs_end[3];
	uint32_t partition_start;
	uint32_t partition_size;
} __attribute__ ((packed));

/** one for all */
#define SECTOR_SIZE 512

/* max lba of drive */
#define DISK_LBA_MAX 0xFFFFFFFF

/* BEGIN: CR722: Apply GPT */
/**
 * Description of GPT header
 */
struct gpt_header {
	uint64_t signature;
	uint32_t revision;
	uint32_t header_size;
	uint32_t header_crc32;
	uint32_t reserved;
	uint64_t my_lba;
	uint64_t alternate_lba;
	uint64_t first_usable_lba;
	uint64_t last_usable_lba;
	uint8_t  disk_guid[16];
	uint64_t partition_entry_lba;
	uint32_t number_of_partition_entries;
	uint32_t size_of_partition_entry;
	uint32_t partition_entry_array_crc32;
	uint8_t  reserved2[SECTOR_SIZE - 92];
} __attribute__ ((packed));


/**
 * Description of GPT partition entry array
 */
struct gpt_entry_array {
	uint8_t  partition_type_guid[16];
	uint8_t  unique_partition_guid[16];
	uint64_t starting_lba;
	uint64_t ending_lba;
	uint64_t attributes;
	uint8_t  partition_name[72];
} __attribute__ ((packed));

/**
 * Guess the size of the disk, based on the partition table entries
 * @param dev device to create partitions for
 * @param table partition table
 * @return size in sectors
 */
 /* END: CR722: Apply GPT */
#ifdef CONFIG_ATA_BIOS
static unsigned long disk_guess_size(struct device_d *dev, struct partition_entry *table)
{
	int part_order[4] = {0, 1, 2, 3};
	unsigned long size = 0;
	int i;

	/* TODO order the partitions */

	for (i = 0; i < 4; i++) {
		if (table[part_order[i]].partition_start != 0) {
			size += table[part_order[i]].partition_start - size; /* the gap */
			size += table[part_order[i]].partition_size;
		}
	}
#if 1
/* limit disk sizes we can't handle due to 32 bit limits */
	if (size > 0x7fffff) {
		dev_warn(dev, "Warning: Size limited due to 32 bit contraints\n");
		size = 0x7fffff;
	}
#endif
	return size;
}
#endif

/* BEGIN: CR722: Apply GPT */
/**
 * Check GPT entry information
 * @param g_entry GPT entry information
 * @return partition size
 */
static uint64_t disk_check_gpt_entry(struct device_d *dev, struct gpt_entry_array* g_entry)
{
	int i;
	uint64_t partition_size;

	/* check partition type GUID */
	for (i = 0; i < 16; i++) {
		if (g_entry->partition_type_guid[i] != 0x00) {
			break;
		}
	}

	if (16 <= i) {
		dev_dbg(dev, "partition type guid is 0x00\n");
		return 0;
	}

	/* check starting LBA */
	if (DISK_LBA_MAX < g_entry->starting_lba) {
		dev_warn(dev, "Warning: starting LBA is over\n");
		return 0;
	}

	/* check partition size */
    partition_size = (g_entry->ending_lba + 1) - g_entry->starting_lba;
	if ((DISK_LBA_MAX * SECTOR_SIZE) < partition_size) {
		dev_warn(dev, "Warning: partition size is over\n");
		return 0;
	}

	return partition_size;
}

/**
 * Guess the size of the disk, based on the partition table entries (partition type:GPT)
 * @param dev device to create partitions for
 * @return size in sectors
 */
#ifdef CONFIG_ATA_BIOS
static uint64_t disk_guess_size_gpt(struct device_d *dev)
{
	uint64_t total_size = 0;
	int i;
	int rc;
	int entry_num;
	int sub_index;
	uint8_t *sector = NULL;
	uint8_t *sector2 = NULL;
	uint64_t partition_size;
	struct ata_interface *intf = dev->platform_data;
	struct gpt_header *g_header;
	struct gpt_entry_array *g_entry;

	/* read GPT header */
	sector = xmalloc(SECTOR_SIZE);
	rc = intf->read(dev, 1, 1, sector);
	if (rc != 0) {
		dev_err(dev, "Failed to read GPT Header (%d)\n", rc);
		total_size = DISK_LBA_MAX;
		goto on_error;
	}
	g_header = (struct gpt_header*)&(sector[0]);
    
	/* get number of the entry information in 1LBA */
	entry_num = SECTOR_SIZE / g_header->size_of_partition_entry;

	/* get size of each partition */
	for (i = 0; i < g_header->number_of_partition_entries; i++) {
		/* read entry information */
		sub_index = i % entry_num;
		if (0 == sub_index) {
			free(sector2);
			sector2 = xmalloc(SECTOR_SIZE);
			rc = intf->read(dev, (i / entry_num) + 2, 1, sector2);
			if (rc != 0) {
				dev_err(dev, "Failed to read GPT entry information (%d)\n", rc);
				total_size = DISK_LBA_MAX;
				goto on_error;
			}
			g_entry = (struct gpt_entry_array*)&(sector2[0]);
		}

		/* check entry information */
		partition_size = disk_check_gpt_entry(dev, &g_entry[sub_index]);
		if (0 == partition_size) {
			continue;
		}

		/* update total size */
		total_size += partition_size;
	}

	if (DISK_LBA_MAX < total_size) {
		dev_warn(dev, "Warning: Size limited\n");
		total_size = DISK_LBA_MAX;
	}

	on_error:
	free(sector);
	free(sector2);

	return total_size;
}
#endif
/* END: CR722: Apply GPT */

/**
 * Register extended partitions found on the drive
 * @param dev device to create partitions for
 * @param table partition table
 * @param order order of extended partition
 * @param base base address of extended partition
 * @return 0 on success
 */
static int disk_register_extended_partitions(struct device_d *dev, struct partition_entry *table, int order, uint32_t base)
{
	int i, rc;
	char drive_name[16], partition_name[19];

	i = order;
	/* TODO order the partitions */

	sprintf(drive_name, "%s%d", dev->name, dev->id);
	sprintf(partition_name, "%s%d.%d", dev->name, dev->id, i);
	
	if (table[0].partition_start != 0) {
#if 1
/* ignore partitions we can't handle due to 32 bit limits */
		if (table[0].partition_start > 0x7fffff)
			printf("ignore start address of partition we can't handle due to 32 bit limits\n");
		if (table[0].partition_size > 0x7fffff)
			printf("ignore size of partition we can't handle due to 32 bit limits\n");
#endif
		dev_dbg(dev, "Registering partition %s to drive %s\n",
			partition_name, drive_name);
		rc = devfs_add_partition(drive_name,
			(table[0].partition_start + base + i - 4) * SECTOR_SIZE,
			table[0].partition_size * SECTOR_SIZE,
			DEVFS_PARTITION_FIXED, partition_name);
		if (rc != 0)
			dev_err(dev, "Failed to register partition %s (%d)\n", partition_name, rc);

#if 0
		printf("Registering partition %s to drive %s: Start address: %x, Size: %x\n", 
				partition_name, 
				drive_name, 
				(table[0].partition_start + base + i - 4) * SECTOR_SIZE, 
				table[0].partition_size * SECTOR_SIZE);
#endif
		}


	return 0;
}



/**
 * Register partitions found on the drive
 * @param dev device to create partitions for
 * @param table partition table
 * @return 0 on success
 */
static int disk_register_partitions(struct device_d *dev, struct partition_entry *table)
{
	int part_order[4] = {0, 1, 2, 3};
	int i, rc;
	char drive_name[16], partition_name[19];
	
	struct ata_interface *intf = dev->platform_data;
	uint32_t EBR_base_address;
	uint8_t *sector;
	sector = xmalloc(SECTOR_SIZE);
	
	
	/* TODO order the partitions */
	for (i = 0; i < 4; i++) {
		sprintf(drive_name, "%s%d", dev->name, dev->id);
		sprintf(partition_name, "%s%d.%d", dev->name, dev->id, i);
		if (table[part_order[i]].partition_start != 0) {
#if 1
/* ignore partitions we can't handle due to 32 bit limits */
			if (table[part_order[i]].partition_start > 0x7fffff)
				continue;
			if (table[part_order[i]].partition_size > 0x7fffff)
				continue;
#endif
			dev_dbg(dev, "Registering partition %s to drive %s\n",
				partition_name, drive_name);
			rc = devfs_add_partition(drive_name,
				table[part_order[i]].partition_start * SECTOR_SIZE,
				table[part_order[i]].partition_size * SECTOR_SIZE,
				DEVFS_PARTITION_FIXED, partition_name);
			if (rc != 0)
				dev_err(dev, "Failed to register partition %s (%d)\n", partition_name, rc);
#if 0
			printf("Registering partition %s to drive %s: Start address: %x, Size: %x\n", 
					partition_name, 
					drive_name, 
					table[part_order[i]].partition_start * SECTOR_SIZE, 
					table[part_order[i]].partition_size * SECTOR_SIZE);
#endif
		}
	}

	/* Read EBR base address */
	if(table[part_order[3]].partition_start == 0)
		goto on_error;
		
	rc = intf->read(dev, table[part_order[3]].partition_start, 1, sector);
	EBR_base_address = table[part_order[3]].partition_start;
	
	/* Loop to read all extended partition */
	while(1) {
		if (rc != 0) {
			dev_err(dev, "Cannot read EBR of this device\n");
			rc = -ENODEV;
			goto on_error;
		}
		
		if ((sector[510] != 0x55) || (sector[511] != 0xAA)) {
			dev_info(dev, "No EBR partition table found\n");
			rc = 0;
			goto on_error;
		}
		rc = disk_register_extended_partitions(dev, (struct partition_entry*)&sector[446], i++, EBR_base_address);
		
		table = (struct partition_entry*)&sector[446];
		if(table[part_order[1]].partition_start == 0)
			break;
		rc = intf->read(dev, table[part_order[1]].partition_start + EBR_base_address, 1, sector);
	
	}
	
	on_error:
	free(sector);

	return 0;
}

/* BEGIN: CR722: Apply GPT */

/**
 * Register partitions found on the drive (partition type:GPT)
 * @param dev device to create partitions for
 * @return 0 on success
 */
static int disk_register_partitions_gpt(struct device_d *dev)
{
	int i;
	int rc;
	int entry_num;
	int sub_index;
	uint8_t *sector = NULL;
	uint8_t *sector2 = NULL;
	uint64_t partition_size;
	struct ata_interface *intf = dev->platform_data;
	struct gpt_header *g_header;
	struct gpt_entry_array *g_entry;
	char drive_name[16];
	char partition_name[19];

	/* read GPT header */
	sector = xmalloc(SECTOR_SIZE);
	rc = intf->read(dev, 1, 1, sector);
	if (rc != 0) {
		dev_err(dev, "Failed to read GPT Header (%d)\n", rc);
		goto on_error;
	}
	g_header = (struct gpt_header*)&(sector[0]);

	/* get number of the entry information in 1LBA */
	entry_num = SECTOR_SIZE / g_header->size_of_partition_entry;

	/* create partition */
	for (i = 0; i < g_header->number_of_partition_entries; i++)	{
		sprintf(drive_name, "%s%d", dev->name, dev->id);
		sprintf(partition_name, "%s%d.%d", dev->name, dev->id, i);

		/* read entry information */
		sub_index = i % entry_num;
		if (sub_index == 0) {
			free(sector2);
			sector2 = xmalloc(SECTOR_SIZE);
			rc = intf->read(dev, (i / entry_num) + 2, 1, sector2);
			if (rc != 0) {
				dev_err(dev, "Failed to read GPT entry information %s (%d)\n", partition_name, rc);
				goto on_error;
			}
			g_entry = (struct gpt_entry_array*)&(sector2[0]);
		}

		/* check entry information */
		partition_size = disk_check_gpt_entry(dev, &g_entry[sub_index]);
		if (0 == partition_size) {
			continue;
		}

		/* create partition */
		dev_dbg(dev, "Registering partition %s to drive %s\n",
			partition_name, drive_name);
		dev_dbg(dev, "    starting_lba %lld\n", g_entry[sub_index].starting_lba);
		dev_dbg(dev, "    ending_lba   %lld\n", g_entry[sub_index].ending_lba);

		rc = devfs_add_partition(drive_name,
			g_entry[sub_index].starting_lba * SECTOR_SIZE,
			partition_size * SECTOR_SIZE,
			DEVFS_PARTITION_FIXED, partition_name);
		if (rc != 0) {
			dev_err(dev, "Failed to register partition %s (%d)\n", partition_name, rc);
		}
	}

	on_error:
	free(sector);
	free(sector2);

	return 0;
}
/* END: CR722: Apply GPT */

struct ata_block_device {
	struct block_device blk;
	struct device_d *dev;
	struct ata_interface *intf;
};

static int atablk_read(struct block_device *blk, void *buf, int block,
		int num_blocks)
{
	struct ata_block_device *atablk = container_of(blk, struct ata_block_device, blk);

	return atablk->intf->read(atablk->dev, block, num_blocks, buf);
}

#ifdef CONFIG_ATA_WRITE
static int atablk_write(struct block_device *blk, const void *buf, int block,
		int num_blocks)
{
	struct ata_block_device *atablk = container_of(blk, struct ata_block_device, blk);

	return atablk->intf->write(atablk->dev, block, num_blocks, buf);
}
#endif

static struct block_device_ops ataops = {
	.read = atablk_read,
#ifdef CONFIG_ATA_WRITE
	.write = atablk_write,
#endif
};

/**
 * Probe the connected disk drive
 */
static int disk_probe(struct device_d *dev)
{
	uint8_t *sector;
	int rc;
	/* BEGIN: CR722: Apply GPT */
	int gpt_enable = 0;
	/* END: CR722: Apply GPT */
	struct ata_interface *intf = dev->platform_data;
	struct ata_block_device *atablk = xzalloc(sizeof(*atablk));
	sector = xmalloc(SECTOR_SIZE);

	rc = intf->read(dev, 0, 1, sector);
	if (rc != 0) {
		dev_err(dev, "Cannot read MBR of this device\n");
		rc = -ENODEV;
		goto on_error;
	}
	/* BEGIN: CR722: Apply GPT */
	if(sector[450] == 0xee)
	{
		gpt_enable = 1;
		printf("\nGPT dectect! \n ");
	}
	else{
		gpt_enable = 0;
		printf("\nMBR dectect! \n ");
	}
	/* END: CR722: Apply GPT */
		
	/*
	 * BIOS based disks needs special handling. Not the driver can
	 * enumerate the hardware, the BIOS did it already. To show the user
	 * the drive ordering must not correspond to the Linux drive order,
	 * use the 'biosdisk' name instead.
	 */
#ifdef CONFIG_ATA_BIOS
	if (strcmp(dev->driver->name, "biosdisk") == 0)
		atablk->blk.cdev.name = asprintf("biosdisk%d", dev->id);
	else
#endif
		atablk->blk.cdev.name = asprintf("disk%d", dev->id);

#ifdef CONFIG_ATA_BIOS
	/* On x86, BIOS based disks are coming without a valid .size field */
	if (dev->size == 0) {
		/* guess the size of this drive if not otherwise given */
		if(gpt_enable){
			dev->size = disk_guess_size_gpt(dev) * SECTOR_SIZE;
		}
		else{
			dev->size = disk_guess_size(dev,
				(struct partition_entry*)&sector[446]) * SECTOR_SIZE;
		}
		dev_info(dev, "Drive size guessed to %u kiB\n", dev->size / 1024);
	}
#endif
	atablk->blk.num_blocks = dev->size / SECTOR_SIZE;
	atablk->blk.ops = &ataops;
	atablk->blk.blockbits = 9;
	atablk->dev = dev;
	atablk->intf = intf;
	blockdevice_register(&atablk->blk);

	if ((sector[510] != 0x55) || (sector[511] != 0xAA)) {
		dev_info(dev, "No partition table found\n");
		rc = 0;
		goto on_error;
	}

	if(gpt_enable){
		rc = disk_register_partitions_gpt(dev);
	}
	else{
		rc = disk_register_partitions(dev, (struct partition_entry*)&sector[446]);
	}

on_error:
	free(sector);
	return rc;
}

#ifdef CONFIG_ATA_BIOS
static struct driver_d biosdisk_driver = {
        .name   = "biosdisk",
        .probe  = disk_probe,
};
#endif

static struct driver_d disk_driver = {
        .name   = "disk",
        .probe  = disk_probe,
};

static int disk_init(void)
{
#ifdef CONFIG_ATA_BIOS
	register_driver(&biosdisk_driver);
#endif
	register_driver(&disk_driver);
	return 0;
}

device_initcall(disk_init);
