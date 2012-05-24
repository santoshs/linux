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

/**
 * Guess the size of the disk, based on the partition table entries
 * @param dev device to create partitions for
 * @param table partition table
 * @return size in sectors
 */
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
	struct ata_interface *intf = dev->platform_data;
	struct ata_block_device *atablk = xzalloc(sizeof(*atablk));
	sector = xmalloc(SECTOR_SIZE);

	rc = intf->read(dev, 0, 1, sector);
	if (rc != 0) {
		dev_err(dev, "Cannot read MBR of this device\n");
		rc = -ENODEV;
		goto on_error;
	}

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
		dev->size = disk_guess_size(dev,
			(struct partition_entry*)&sector[446]) * SECTOR_SIZE;
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


	rc = disk_register_partitions(dev, (struct partition_entry*)&sector[446]);

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
