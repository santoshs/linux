/*
 * r_loader_boot_matrix.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef __R_LOADER_BOOT_MATRIX_H__
#define __R_LOADER_BOOT_MATRIX_H__

#include "compile_option.h"
#include "com_type.h"

/**
 * Boot matrix definition
 */
 
/* Charger type */
#define		CHRG_USB_SDP				0
#define		CHRG_USB_CDP_DCP			1
#define		CHRG_NONE					2
#define		CHRG_ACA					3
#define		CHRG_VAC					4

/* Battery status */
#define		BATT_ENOUGH			0
#define		BATT_EMPTY			1
#define		BATT_OFF			2

/* Define matrix result */
#define		MAT_BOOT_SYS				0
#define		MAT_PWR_OFF					1
#define		MAT_OFF_CHARGE				2
#define		MAT_NO_BATT					3
#define		MAT_HIGH_TEMP				4

/* Define for POWER_CONTROL detection */
#define GPIO_PORT130DR_BOOTMATRIX_DATA 		0xD0
#define TUSB_POWER_CONTROL					0x3D
#define TUSB_POWER_CONTROL_SWCONTROL		0x01
#define TUSB_POWER_CONTROL_HWCONTROL		0x00

/* Filter Bitmask */
#define FILTER_MSK								(uint)(0x3D01E6)

/* Event number */
#define MAX_SUB_EVENT							11
#define MAX_EVENT								11
#define MAX_BATT_STS							3
#define MAX_CHRG_STS							5

/* Structure of boot matrix 16 bytes*/
typedef struct __boot_matrix__ {
	ulong	srstfr;				/* SRSTFR register */
	uchar	stbchrb2;			/* STBCHRB2 register */
	uchar	ctlr_stat1;			/* CONTROLLER_STATUS1 register of PMIC */
	uchar	phoenix_start_con;	/* PHOENIX_START_CONDITION register of PMIC */
	uchar	sts_hw_conditions;	/* STS_HW_CONDITIONS register of PMIC */	
	uchar	charger;			/* Charger */
	uchar	batt_det;			/* Battery is detected */
	ushort	batt_vol;			/* Battery voltage */
	int 	cpu_temp;			/* CPU temperature */
	uchar 	long_pw_key;		/* Long power key press */
	uchar 	power_control;		/* POWER_CONTROL of TUSB1211 */
	uchar 	reservation;		/* Reservation */
} BOOT_MATRIX;

/* Error code */
#define		BOOT_LOG_OK					0
#define		BOOT_LOG_ERR_INIT_PMIC		-1
#define		BOOT_LOG_ERR_PMIC			-2
#define		BOOT_LOG_ERR_EMMC			-3
#define		BOOT_LOG_ERR_RTC			-4



/* Function Prototypes */
RC boot_matrix_update(void);
uchar boot_matrix_check(void);
void boot_matrix_print(void);

#endif /* __R_LOADER_BOOT_MATRIX_H__ */
