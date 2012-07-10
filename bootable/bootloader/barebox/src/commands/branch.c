/*
 * reginfo.c - print information about SoC specifc registers
 *
 * Copyright (c) 2007 Sascha Hauer <s.hauer@pengutronix.de>, Pengutronix
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <common.h>
#include <command.h>
#include <asm/io.h>

#define STBCHRB2	(0xE6180042)

static int do_auto_boot(struct command *cmdtp, int argc, char *argv[])
{
	char drive[64];
	
#ifndef CONFIG_U2EVM_SECURITY
	unsigned char disk, part, tmp;
	char boot_cmd[32];
	/* Read STBCHRB2 */
	tmp = readb(STBCHRB2);
	
	/* Get disk info */
	disk = tmp >> 6;
	
	/* Get part info */
	part = tmp & ~(0xC0);
	part = part - 1;	/* Modify for STBCHRB2 one-based partition */
	
	printf("Clear STBCHRB2\n");
	run_command("set_stbchrb2 0x00",0);
	
	/* Run boot command */
	sprintf(drive, "disk%d.%d", disk, part);
	sprintf(boot_cmd, "booti dev/%s", drive);
	printf("Boot from dev/%s\n", drive);
	
	run_command(boot_cmd, 0);
#else
	/* Add a partition on SDRAM for booting */
	sprintf(drive, "addpart /dev/mem 20M@0x%x(boot)", CONFIG_U2EVM_SECURITY_IMG_ADDRS);
	run_command(drive, 0);
	
	/* Run boot command */
	printf("Boot from address 0x%x ...\n", CONFIG_U2EVM_SECURITY_IMG_ADDRS);
	run_command("booti /dev/mem.boot", 0);
#endif /* CONFIG_U2EVM_SECURITY */

	return 0;
}

BAREBOX_CMD_START(auto_boot)
	.cmd		= do_auto_boot,
	.usage		= "Do auto booting ...",
BAREBOX_CMD_END

static int do_get_partition(struct command *cmdtp, int argc, char *argv[])
{
	unsigned char ret;
	ret = readb(STBCHRB2);
	ret = ret & ~(0xC0);
	ret = ret - 1;	/* Modify for STBCHRB2 one-based partition */

	return ret;
}

BAREBOX_CMD_START(get_partition)
	.cmd		= do_get_partition,
	.usage		= "Get partition to branch",
BAREBOX_CMD_END

static int do_set_stbchrb2(struct command *cmdtp, int argc, char *argv[])
{
	unsigned char ret;
	unsigned char temp;
	
	temp = (char) simple_strtol(argv[1], NULL, 16);
	writeb(temp, STBCHRB2);
	
	return 0;
}

BAREBOX_CMD_START(set_stbchrb2)
	.cmd		= do_set_stbchrb2,
	.usage		= "Set stbchrb2 register",
BAREBOX_CMD_END
