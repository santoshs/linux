/*
 * Configuation settings for R-Mobile U2 EVM board
 *
 * Copyright (C) 2012 Renesas Electronics Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 *
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#undef DEBUG

#define __io

#include <asm/arch/hardware.h>

#define CONFIG_SYS_HZ		1000

#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{ 115200 }

#define CONFIG_DISPLAY_CPUINFO

/* Architecture specific */
#define CONFIG_USE_ARCH_MEMSET
#define CONFIG_USE_ARCH_MEMCPY
#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

/* Command */
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_ENV
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_CMD_FLASH
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_MMC
#define CONFIG_CMD_MMC_BOOTAREA
#define CONFIG_CMD_NET
#define CONFIG_CMD_PING
#define CONFIG_CMD_RUN
#define CONFIG_CMD_SAVEENV

#define CONFIG_DOS_PARTITION
#define CONFIG_MAC_PARTITION

#define CONFIG_BOOTDELAY	1
#define CONFIG_BOOTFILE		uImage
#define CONFIG_BOOTARGS		"console=ttySC0,115200 root=/dev/ram0c"

#define CONFIG_BOOTCOMMAND	"run autoboot_emmc_ext4"
#define CONFIG_EXTRA_ENV_SETTINGS					\
	"setargs=setenv bootargs\0"					\
	"con=console=ttySC0,115200\0"					\
	"mem=vmalloc=448M\0"						\
	"a_init=init=/init\0"						\
	"l_init=init=/linuxrc\0"					\
	"root_cramfs=root=/dev/null rootflags=physaddr=0x10400000\0"	\
	"sd_root_dev=root=/dev/mmcblk1p1 rootfstype=ext4 rootwait\0"	\
	"emmc_root_dev=root=/dev/mmcblk0p1 rootfstype=ext4 rootwait\0"	\
	"emmc_root=root=/dev/mmcblk0p2 rootdelay=1\0"			\
	"read_sd_image=ext2load mmc 1 50007fc0 /boot/uImage\0"		\
	"read_emmc_image=ext2load mmc 0 50007fc0 /boot/uImage\0"	\
	"nl_set=${setargs} ${con} ${mem} ${l_init} ${root_cramfs}\0"	\
	"sl_set=${setargs} ${con} ${mem} ${l_init} ${sd_root_dev}\0"	\
	"el_set=${setargs} ${con} ${mem} ${l_init} ${emmc_root_dev}\0"	\
	"ea_set=${setargs} ${con} ${mem} ${a_init} ${emmc_root_dev}\0"	\
	"initemmc=${setargs} ${con} ${mem} ${emmc_root} ${a_init}\0"	\
	"nor_lboot=run nl_set; bootm 10100000\0"			\
	"sd_lboot=run sl_set; mmcinfo 1; ${read_sd_image}; bootm\0"	\
	"emmc_lboot=run el_set; mmcinfo 0; ${read_emmc_image}; bootm\0"	\
	"emmc_aboot=run ea_set; mmcinfo 0; ${read_emmc_image}; bootm\0"	\
	"autoboot_emmc_ext4= mmcinfo 0; run initemmc; mbrboot_init 0; booti 0 boot\0"

/* Memory */
#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM_1			0x40000000
#define PHYS_SDRAM_1_SIZE		(1024 * 1024 * 1024)
#define MODEM_MISC_SIZE			0x10000000

#define CONFIG_SYS_SDRAM_BASE	(PHYS_SDRAM_1 + MODEM_MISC_SIZE)
#define CONFIG_SYS_SDRAM_SIZE	(PHYS_SDRAM_1_SIZE - MODEM_MISC_SIZE)
#define CONFIG_SYS_LOAD_ADDR	(CONFIG_SYS_SDRAM_BASE + 0x7fc0)

/* Memtest */
#define CONFIG_SYS_ALT_MEMTEST
#define CONFIG_SYS_MEMTEST_START	(CONFIG_SYS_SDRAM_BASE)
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + (16 * 1024 * 1024))
#undef  CONFIG_SYS_MEMTEST_SCRATCH

/* Miscellaneous configurable options */
#define CONFIG_SYS_LONGHELP			/* undef to save memory	*/
#define CONFIG_SYS_PROMPT	"U2 # "		/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE	384		/* Buffer size for input from the Console */
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS	24		/* max args accepted for monitor commands */
#define CONFIG_SYS_BARGSIZE	512		/* Buffer size for Boot Arguments passed to kernel */

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(((CONFIG_ENV_SIZE+1023)/1024 + 128) * 1024)
#define CONFIG_SYS_GBL_DATA_SIZE	(256)

/* SCIF */
#define CONFIG_SCIF_CONSOLE
#define CONFIG_CONS_SCIF0
#define CONFIG_CPU_SH7720	/* use drivers/serial/serial_sh.c */

#define CONFIG_SYS_CLK_FREQ		48000000

/* MMCIF */
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_SH_MMCIF
#define CONFIG_SH_MMCIF_FREQ		104000000

/* SDHI0 */
#define CONFIG_SH_SDHI
#define CONFIG_MMC_SH_SDHI_NR_CHANNEL	1
#define CONFIG_SH_SDHI_FREQ		96000000

/* Ethernet */
#define CONFIG_NET_MULTI
#define CONFIG_SMC911X
#define CONFIG_SMC911X_16_BIT
#define CONFIG_SMC911X_BASE		0x00080000

/* NOR */
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_SYS_FLASH_PROTECTION
#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_16BIT
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_FLASH_SHOW_PROGRESS	45 /* count down from 45/5: 9..1 */

#define CONFIG_SYS_FLASH_BASE		0x10000000
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_MONITOR_LEN		(128 * 1024)
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks */
#define CONFIG_SYS_MAX_FLASH_SECT	512	/* max number of sectors on one chip */

/* Environment information */
/* env emmc */
#define CONFIG_ENV_IS_IN_MMC		1
#define CONFIG_ENV_SECT_SIZE		512
#define CONFIG_ENV_OFFSET		(1 * CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE			(15 * CONFIG_ENV_SECT_SIZE)
#define CONFIG_SYS_MMC_ENV_DEV		0

#define CONFIG_ENV_OVERWRITE

#define CONFIG_IMGBOOT		1
#define CONFIG_CMD_GPTINIT	1

#endif	/* __CONFIG_H */
