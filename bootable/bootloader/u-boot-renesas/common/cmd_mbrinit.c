/*
 * (C) Copyright 2011  Igel Co., Ltd
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
 */
#include <common.h>
#include <part.h>
#include <mmc.h>
#include <fastboot.h>

/**
  * Currently only supported fixed allocation of partitions
  */
static char * part_names[] = { /* fastboot names are max 16 chars */
	"(reserved)",
	"boot",
	"system",
	"(reserved)",
	"userdata",
	"cache",
	"delta",
	"fotamng",
	"fotaua",
	"fotauabk",
	"recovery",
	"(reserved)",
	"security",
	"modem1",
	"modem2",
	"sddownloader",
	"factory1",
	"factory2",
	"factory3",
	"free",
	"areaforfactory",
	"checksum",
	"renesas_log",
	"modem_log",
	"kc_log",
	"tuneupval"
};

#define MAX_PARTS 32
int load_mbrpart(struct mmc *mmc) {
	struct fastboot_ptentry e;
	disk_partition_t part;
	int i, ret, n;
	block_dev_desc_t *mmc_dev;

	mmc_dev = &mmc->block_dev;

	for (i = 0; i < MAX_PARTS; i++) {	
		ret = get_partition_info(mmc_dev, i + 1, &part);
		if (ret < 0 || part.start == 0)
			break;

		if (i < ARRAY_SIZE(part_names)) {
			strcpy(e.name, part_names[i]);
		} else {
			for (n = 0; n < (sizeof(e.name) - 1); n++)
				e.name[n] = part.name[n];
			e.name[n] = 0;
		}
		e.start = part.start;
		e.length = part.size * 512;
		e.flags = 0;
		fastboot_flash_add_ptn(&e);
		if (e.length > 0x100000)
			printf("%8d %7dM %s\n", e.start, e.length/0x100000,
				e.name);
		else
			printf("%8d %7dK %s\n", e.start, e.length/0x400,
				e.name);
	}
	return 0;
}

int do_mbrboot_init (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]) {
	struct mmc *mmc;
	int dev_num;
	if (argc < 2)
		dev_num = 0;
	else
		dev_num = simple_strtoul(argv[1], NULL, 0);

	mmc = find_mmc_device(dev_num);

	if (mmc) {
		mmc_init(mmc);
	} else {
		printf("Couldn't find mmc device 0\n");
		return 1;
	}
	return load_mbrpart(mmc);
}

U_BOOT_CMD(
	mbrboot_init, 2, 0,	do_mbrboot_init,
	"Read partition info from mmc",
	""
);
