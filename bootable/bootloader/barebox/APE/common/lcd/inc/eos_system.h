/*
 * eos_system.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */
#ifndef __H_EOS_SYSTEM_
#define __H_EOS_SYSTEM_

/***************************************************************/
/* INCLUDE FILE                                                */
/***************************************************************/
#include "eos_config.h"

/***************************************************************/
/* PUBLIC TYPEDEFE                                             */
/***************************************************************/
/* INDEX_TYPE_DEFINE */
typedef unsigned char              u1;
typedef unsigned short             u2;
typedef unsigned long              u4;
typedef signed char                s1;
typedef signed short               s2;
typedef signed long                s4;
typedef volatile unsigned char    VU1;
typedef volatile unsigned short   VU2;
typedef volatile unsigned long    VU4;
/* EOS USE TYPE_DEFINE */
typedef char				B;
typedef short				H;
typedef long				W;
typedef unsigned char		UB;
typedef unsigned short		UH;
typedef unsigned long		UW;
typedef signed char			IB;
typedef signed short		IH;
typedef signed long			IW;
typedef volatile unsigned char		UCHAR;
typedef volatile unsigned short		USHORT;
typedef volatile unsigned long		ULONG;

/***************************************************************/
/* PUBLIC CONSTANT DEFINITON                                   */
/***************************************************************/
#define	U1G_ZERO			(u1)( 0x00 )
#define	U2G_ZERO			(u2)( 0x0000 )
#define	U4G_ZERO			(u4)( 0x00000000 )
#define	S1G_ZERO			(s1)( 0x00 )
#define	S2G_ZERO			(s2)( 0x0000 )
#define	S4G_ZERO			(s4)( 0x00000000 )
#define	U1G_OFF				(u1)( 0 )
#define	U1G_ON				(u1)( 1 )
#define	U2G_OFF				(u2)( 0x0000 )
#define	U2G_ON				(u2)( 0x0001 )
#define	U4G_OFF				(u4)( 0x00000000 )
#define	U4G_ON				(u4)( 0x00000001 )
#define U1G_OK				(u1)( 0 )
#define U1G_NG				(u1)( 1 )

/* INDEX_BASE_ADDRESS */
#define	CPG_BADR			0xE6150000		/* (CPG) */
#define	CPGA_BADR			0xE6150000		/* (CPG) */
#define	SYSC_BADR			0xE6180000		/* System Controller (SYSC) */
#define	BSC_BADR			0xFEC10000		/* Bus State Controller(BSC) */
#define	SBSC1_BADR			0xFE400000		/* Bus State Controller for SDRAM (SBSC1) */
#define	SDMRA_BADR			0xFE500000		/* Bus State Controller for SDRAM (SBSC1) */
#define	TPU_BADR			0xE6600000		/* 16-Bit Timer Pulse Unit (TPU) Mirror:0xE7600000*/
#define	SCIFA0_BADR			0xE6C40000		/* Serial Comunication InterfaceA CH0(SCIFA0)mirror:0xE7C40000 */
#define	SCIFA1_BADR			0xE6C50000		/* Serial Comunication InterfaceA CH1(SCIFA1)mirror:0xE7C50000 */
#define	SCIFA2_BADR			0xE6C60000		/* Serial Comunication InterfaceA CH2(SCIFA2)mirror:0xE7C60000 */
#define	I2C0_BADR			0xE6820000		/* I2C0 Controller */
#define	I2C1_BADR			0xE6822000		/* I2C1 Controller */
#define	I2C2_BADR			0xE6824000		/* I2C2 Controller */
#define	I2C3_BADR			0xE6826000		/* I2C3 Controller */
#define	I2C0H_BADR			0xE6828000		/* I2C0H Controller */
#define	I2C1H_BADR			0xE682A000		/* I2C1H Controller */
#define	I2C2H_BADR			0xE682C000		/* I2C2H Controller */
#define	I2C3H_BADR			0xE682E000		/* I2C3H Controller */
#define	GPIO_BADR			0xE6050000		/* IO PORT(GPIO)*/
#define	MMCIF_BADR			0xE6BD0000		/* Multi Media Card Interface (MMCIF) */
#define	MMCIF0_BADR			0xE6BD0000		/* Multi Media Card Interface0 (MMCIF0) */
#define	MMCIF1_BADR			0xE6BE0000		/* Multi Media Card Interface1 (MMCIF1) */
#define	TMU0_BADR			0xE61E0000		/* Timer unit 0 (TMU0)Secure */
#define	TMU1_BADR			0xFFF60000		/* Timer unit 0 (TMU0) */
#define	TMU2_BADR			0xFFF70000		/* Timer unit 0 (TMU1) */
#define	CEU_BADR			0xFE910000		/* Capture engine unit (CEU)*/
#define	MERAM_BADR			0xE5500000		/* Media RAM(MERAM) */
#define	LCDC_BADR			0xFE940000		/* LCD controller (LCDC)*/
#define	LCDC1_BADR			0xFE944000		/* LCD controller (LCDC1) */
#define	DSI_BADR			0xFEAB0000		/* Display Serial Interface(DSI-Tx) */

/* INDEX_DDR-SDRAM_AREA */
#define U4G_SDRAM_SBSC1			(u4)( 0x40000000 )

#define U4G_EMMC_SECT_SIZE		(u4)( 0x01000000 )
#define U4G_EMMC_ST_SECT_ADDR	(u4)( 0x00000000 )
#define U4G_EMMC_ED_SECT_ADDR	(u4)( U4G_EMMC_ST_SECT_ADDR + U4G_EMMC_SECT_SIZE - 1 )
#define U4G_EMMC_ATEST_1SIZE	(u4)( 0x00080000 )

/***************************************************************/
/* PUBLIC VARIABLE EXTERN                                      */
/***************************************************************/
/* VRL5 format storage CONST data */
extern u1 u1g_on_mask_rom_vrl5[];
extern u1 u1g_on_mask_rom_vrl5_ag5[];
/* TP-automatic operation execution variable 0: Invalidity 1: Effective*/
//extern u1 u1g_tp_auto_mode;

/***************************************************************/
/* PUBLIC FUNCTION EXTERN                                      */
/***************************************************************/
//extern int monprintf(const char *format,...);
//extern void vog_ver_code_get( ST_EVM_VER* ptt_evm_ver );

#endif  /* __H_EOS_SYSTEM_ */
