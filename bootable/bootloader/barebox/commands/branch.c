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

static int do_get_disk(struct command *cmdtp, int argc, char *argv[])
{
	unsigned char ret;
	ret = readb(STBCHRB2);
	printf("STBCHRB2 value: 0x%x\n", ret);
	ret = ret >> 6;
	printf("drive name: disk%d\n", ret);
		
	return ret;
}

BAREBOX_CMD_START(get_disk)
	.cmd		= do_get_disk,
	.usage		= "Get disk drive to branch",
BAREBOX_CMD_END

static int do_get_partition(struct command *cmdtp, int argc, char *argv[])
{
	unsigned char ret;
	ret = readb(STBCHRB2);
	printf("STBCHRB2 value: 0x%x\n", ret);
	ret = ret & ~(0xC0);
	ret = ret - 1;	/* Modify for STBCHRB2 one-based partition */
	printf("Partition name: %d\n", ret);
	
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
	printf("Input value: %d\n", temp);
	ret = readb(STBCHRB2);
	printf("Current value: 0x%x\n", ret);
	writeb(temp, STBCHRB2);
	ret = readb(STBCHRB2);
	printf("New value: 0x%x\n", ret);
	
	
	return 0;
}

BAREBOX_CMD_START(set_stbchrb2)
	.cmd		= do_set_stbchrb2,
	.usage		= "Set stbchrb2 register",
BAREBOX_CMD_END
