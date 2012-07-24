/*
 * r_loader.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef __R_LOADER_H__
#define __R_LOADER_H__

#include "compile_option.h"
#include "com_type.h"
#include "common.h"

/* Error Code */
#define R_LOADER_SUCCESS     			(0)		/* Successful */
#define R_LOADER_ERR_VOLTAGE 			(-1)    /* Voltage setting error */
#define R_LOADER_ERR_BATTERY 			(-2)    /* Battery check error */
#define R_LOADER_ERR_FLASH   			(-3)    /* Flash access error */
#define R_LOADER_ERR_KEY	  			(-4)    /* Key input error */
#define R_LOADER_ERR_LOAD_MLT			(-5)	/* The module load table reading error */
#define R_LOADER_ERR_LOAD_NUM			(-6)	/* The number of modules is an error */
#define R_LOADER_ERR_LOAD_VRL			(-7)	/* The VRL loading error */
#define R_LOADER_ERR_LOAD_MODULE		(-8)	/* The module loading error */
#define R_LOADER_ERR_MODULE_NOT_EXIST	(-9)	/* Module does not exist error */
#define R_LOADER_ERR_SECURE  			(-10)	/* Secure check error */
#define R_LOADER_ERR_RTCINIT			(-11)	/* RTC initialize error */
#define R_LOADER_ERR_BRANCH_MODE		(-12)	/* Branch mode error */
#define R_LOADER_ERR_BOOT_MATRIX		(-13)

/* Flash memory address */
#define SECTOR_LENGTH					(512)			/* Byte size of sector in flash memory */

/* Reset Control Register */
#define MD3_HIGH						(1 << 29)

/* temporary */
#define SHBARCR11  ((volatile ulong*)0xE6244100)
#define SHBARCR_ALL (0x00008F00)

/* Constant of battery status */
typedef enum {
	BATTERY_STATUS_OK = 0,		/* Battery status OK. */
	BATTERY_STATUS_NG			/* Battery status NG. */
} BATTERY_STATUS;

#define SDRAM_LOAD_ADDR					(0x41000000)	/* loading & executing address on SDRAM */

/* WDT Check Time  (0x01 = 125msec) */
#define WDT_125MSEC						(0x01)			/* 125msec wait count */
#define WDT_300MSEC						(0x03)			/* 300msec wait count */
#define WDT_5SEC						(0x28)			/* 5sec wait count */
#define WDT_30SEC						(0xF0)			/* 30sec wait count */

/* Key detection */
#define KEY_INPUT_TIMEOUT				(WDT_125MSEC)	/* Timeout in key input */
#define KEY_INPUT_POLLING_TIME			(4)				/* Key input polling interval(ms) */
#define KEY_INPUT_DETECT_COUNT			(5)				/* Key input detect cont */

/* Module Load Table */
#define MLT_LEN							((ulong)(0x00000400))			/* Module load table buffer = 1KByte */
/* BEGIN: CR722: Apply GPT */
#define MLT_SRC_ADDR					((*STBCHRB1 & STBCHRB1_BOOT_PARTITION2_USED) == 0 ? (0xF00000000003A400ull):(0xE00000000003A400ull))
/* END: CR722: Apply GPT */
#define MLT_MODULE_MAX					(21)			/* module maximum number */

/* Branching mode */
#define STBCHRB2_NO_SELECT				(0x00)			/* No select */
#define STBCHRB2_FACTORY				(0x02)			/* Factory */
#define STBCHRB2_UPDATER				(0x01)			/* Updater */
#define STBCHRB2_FASTBOOT				(0x03)			/* Fastboot */

/* Flash memory address */
#define TABLE1_SEC_ADDR					(0xE8DFDEUll)	/* Sector of table1 in flash memory */
#define TABLE1_SEC_SIZE					(8192)			/* Number of sectors */
#define TABLE1_SDRAM_ADDR_ES1			(0x47BE0000)	/* First address in SDRAM */
#define TABLE1_SDRAM_ADDR_ES2			(0x4C800000)	/* First address in SDRAM */

#define SPU_SEC_ADDR					(0x28E000ULL)	/* Sector of SPU bin in flash memory */
#define SPU_SEC_SIZE					(3072)			/* Number of sectors */
#define SPU_SDRAM_ADDR_ES1				(0x47800000)	/* First address in SDRAM */
#define SPU_SDRAM_ADDR_ES2				(0x4C000000)	/* First address in SDRAM */

/* Module branch mode - BELOW VALUES MUST BE CORRESPONDING WITH MDULE LOAD TABLE */
#define MODULE_BRANCH_MODE_NUMBER		(4)		/* The number of the module branch modes */
#define MODULE_BRANCH_MODE_ERROR		(-1)	/* mode error */
#define MODULE_BRANCH_MODE_BOOTLOADER	(0x00)		/* Bootloader mode */
#define MODULE_BRANCH_MODE_RECOVERY		(0x01)		/* Recovery mode */
#define MODULE_BRANCH_MODE_FACTORY1		(0x02)		/* Factory1 mode */
#define MODULE_BRANCH_MODE_FACTORY2		(0x03)		/* Factory2 mode */
#define MODULE_BRANCH_MODE_FASTBOOT		(0x04)		/* Fastboot mode */
#define MODULE_BRANCH_MODE_UPDATER		(0x05)		/* Updater mode */
#define MODULE_BRANCH_MODE_SDDL			(0x06)		/* SD Downloader mode */
#define MODULE_BRANCH_MODE_FOTA			(0x07)		/* FOTA mode */
#define MODULE_BRANCH_MODE_OFFCHARGE	(0x08)		/* Off-charge mode */
#define MODULE_BRANCH_MODE_HIGHTEMP		(0x09)		/* High temperature mode */
#define MODULE_BRANCH_MODE_NOBATT		(0x0A)		/* No battery mode */

/* Module load table entry */
typedef struct {
	uchar boot_flags[12];
	uchar key_mask[3];
	uchar stbchrb2;
	uint64 flash_addr;
	ulong load_addr;
	ulong payload;
	uint64 signature_addr1;
	uint64 signature_addr2;
} Branch_mode_info;

/* Module load table info */
typedef struct {
	Branch_mode_info info[MLT_MODULE_MAX];
} Module_load_table;

#if 1	/* For SMP issue as temporary */
#define L2CLEANWAY						((volatile ulong *)(0xF01007BC))
#define L2CTRLREG						((volatile ulong *)(0xF0100100))
#define ARM_SECURE1						((volatile ulong *)(0xE6110030))
#define ARM_SECURE2						((volatile ulong *)(0xE6110138))
#endif /* For SMP issue as temporary */

#ifdef __INTEGRITY_CHECK_ENABLE__
/* Security */
/* BEGIN: CR994: Re-allocate SDRAM to reserve more free memory */
#define CERTIFICATE_ADDRESS 						(0x5C100000)
#define CERTIFICATE_ADDRESS2	 					(0x5C101000)
#define CERTIFICATE_SIZE 							(0x800)		/* 2KBs */
#define CERTIFICATE_NONE 							(0xFFFFFFFFFFFFFFFFull)
#define SECOND_MODULE_ADDRESS 						(0x60000000)
/* END: CR994: Re-allocate SDRAM to reserve more free memory */
#endif /* __INTEGRITY_CHECK_ENABLE__ */

/* Function Prototypes */
#ifdef __INTEGRITY_CHECK_ENABLE__
extern RC Secure_Check(void *certificate, unsigned long *image);
#endif /* __INTEGRITY_CHECK_ENABLE__ */

#ifdef __RL_LCD_ENABLE__
void r_loader_load_logo(void);
#endif	/* __RL_LCD_ENABLE__ */

void r_loader_main(void);
RC r_loader_check_module(ulong branch_mode);
void r_loader_init(void);
RC r_loader_load_module(RC branch_mode);
RC r_loader_select_module(void);
void r_loader_startup_module(RC branch_mode);
RC r_loader_save_NVM_bootflag(void);

RC r_loader_bootflags_lookup(const char * flag);
RC r_loader_keymap_lookup(ulong key);

RC r_loader_copy_table_to_RAM(void);
RC r_loader_copy_SPU_to_RAM(void);
void LCD_DisplayWarning(const char* info);
RC Detect_Key_Input(ulong *key);
#endif /* __R_LOADER_H__ */
