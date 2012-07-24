/*
 * eos_config.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */
#ifndef __H_EOS_CONFIG_
#define __H_EOS_CONFIG_

/***************************************************************/
/* DEFINE         				                               */
/***************************************************************/
/* 500ms */
#define U2G_LED_WAIT_TIME		( 500 )
/* DDR-SDRAM */
#define U4G_SDRAM_CLEAR_DATA	( 0x00000000 )

/***************************************************************/
/* DEFINE                   				                   */
/***************************************************************/
#define	U2G_OCAM_AUTO_CNT		( 0x00C0 )
#define	U2G_ICAM_AUTO_CNT		( 0x0060 )
#define	U2G_LCD_AUTO_CNT		( 0x0100 )

/***************************************************************/
/* Compile SW USED DEFINE                                      */
/***************************************************************/
/* Compile SW invalid */
#define CSW_INVALID				( 0 )
/* Compile SW invalid */
#define CSW_VALID				( 1 )

/* NOR FLASH   0:CS0,1:CS4 */
#define CSW_NOR_USE_CS			( 1 )

#define CSW_SDRAM_END_CLEAR		( 1 )

#define CSW_SBSC1_AREA0			( 1 )

#define CSW_TP_AUTOMATIC_MODE	( 0 )

#define CSW_INTERRUPT_USE		( 0 )

#endif  /* __H_EOS_CONFIG_ */
