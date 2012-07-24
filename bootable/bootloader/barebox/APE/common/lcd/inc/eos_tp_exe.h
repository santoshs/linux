/*
 * eos_tp_exe.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */
#ifndef __H_EOS_TP_EXE_
#define __H_EOS_TP_EXE_

/***************************************************************/
/* INCLUDE FILE                                                */
/***************************************************************/
#include "power_test.h"
#include "mem_nor_drv.h"
#include "ledcnt_drv.h"

/***************************************************************/
/* PUBLIC CONSTANT DEFINITON                                   */
/***************************************************************/
#define U1G_SDRAM_TEST1			(u1)(0)
#define U1G_SDRAM_TEST2			(u1)(1)

#define U1G_SDRAM_TEST1			(u1)(0)
#define U1G_SDRAM_TEST2			(u1)(1)

/* Normal terminal */
#define U2G_DDR_ERR_NONE		(u2)(0x0000)	/* DDR-SDRAM Normal terminal of examination */
#define U2G_DDR_ERR_08P1		(u2)(0x0001)	/* DDR-SDRAM8 Bit pattern examination 0xFF NG */
#define U2G_DDR_ERR_08P2		(u2)(0x0002)	/* DDR-SDRAM8 Bit pattern examination 0x00NG */
#define U2G_DDR_ERR_08P3		(u2)(0x0004)	/* DDR-SDRAM8 Bit pattern examination 0x55NG */
#define U2G_DDR_ERR_08P4		(u2)(0x0008)	/* DDR-SDRAM8 Bit pattern examination 0xAANG */
#define U2G_DDR_ERR_16P1		(u2)(0x0010)	/* DDR-SDRAM16 Bit pattern examination 0xFF NG */
#define U2G_DDR_ERR_16P2		(u2)(0x0020)	/* DDR-SDRAM16 Bit pattern examination 0x00NG */
#define U2G_DDR_ERR_16P3		(u2)(0x0040)	/* DDR-SDRAM16 Bit pattern examination 0x55NG */
#define U2G_DDR_ERR_16P4		(u2)(0x0080)	/* DDR-SDRAM16 Bit pattern examination 0xAANG */
#define U2G_DDR_ERR_32P1		(u2)(0x0100)	/* DDR-SDRAM32 Bit pattern examination 0xFF NG */
#define U2G_DDR_ERR_32P2		(u2)(0x0200)	/* DDR-SDRAM32 Bit pattern examination 0x00NG */
#define U2G_DDR_ERR_32P3		(u2)(0x0400)	/* DDR-SDRAM32 Bit pattern examination 0x55NG */
#define U2G_DDR_ERR_32P4		(u2)(0x0800)	/* DDR-SDRAM32 Bit pattern examination 0xAANG */
#define U2G_DDR_ERR_ADRP		(u2)(0x1000)	/* DDR-SDRAM Address pattern examinationNG */

#define U1G_EMMC_NORMAL_MODE	(u1)( 0 )		/* 26MHzMMCLK( Data bus 8bit) */
#define U1G_EMMC_HIGH_MODE		(u1)( 1 )		/* 52MHzMMCLK( Data bus 1bit) */

#define U1G_LCD_MAIN			(u1)( 0 )		/* MAIN LCD  */
#define U1G_LCD_SUB				(u1)( 1 )		/* SUB LCD   */


#define U1G_POWER_OFF			(u1)( 0 )
#define U1G_POWER_ON			(u1)( 1 )

/***************************************************************/
/* PUBLIC TYPEDEF STRUCT UNION ENUM                            */
/***************************************************************/

/***************************************************************/
/* PUBLIC VARIABLE EXTERN                                      */
/***************************************************************/
extern u1 u1g_emmc_debug_read_mode;
extern u1 u1g_emmc_debug_write_mode;

/***************************************************************/
/* PUBLIC FUNCTION EXTERN                                      */
/***************************************************************/
extern void vog_mem_ddr_test( u1 u1t_flag, u2* ptt_err_info );		/* [st 0 ] DDR-SDRAM TEST */
extern void vog_lcd_disp_test( u1 u1t_lcd_mode );

extern void vog_mem_emmc_read( u4 no );
extern void vog_mem_emmc_test( u1 u1t_emmc_mode );

extern void vog_gpio_led_test( void );								/* [st 11] GPIO_LED TEST */

extern void vog_mem_emmc_boot_data_erase( void );
extern void vog_mem_emmc_rcv( void );
extern void vog_power_test( u1 u1t_mode );


#endif  /* __H_EOS_TP_EXE_ */
