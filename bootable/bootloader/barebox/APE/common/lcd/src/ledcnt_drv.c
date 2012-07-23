/*
 * ledcnt_drv.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */
/***************************************************************/
/* INCLUDE FILE                                                */
/***************************************************************/
#include "eos_csw.h"					/* EOS CSW Header file */
#include "eos_stdio.h"					/* EOS-Standard Header */

#include "timer_drv.h"					/* Timer function header    */
#include "port_io_drv.h"				/* PORT I/O header     */
#include "i2c_drv.h"					/* I2C header   */

#include "ledcnt_drv.h"					/* header file    */

/***************************************************************/
/* STATIC CONSTANT DEFINE                                      */
/***************************************************************/
#define CSW_LCD_BACKLIGHT_CTRL		1

#define U1S_ALLLED_OFF			(u1)( 0x00 )

#define U1S_RLED_ON				(u1)( 0x01 )
#define U1S_GLED_ON				(u1)( 0x02 )
#define U1S_YLED_ON				(u1)( 0x03 )
#define U1S_BLED_ON				(u1)( 0x04 )
#define U1S_PLED_ON				(u1)( 0x05 )
#define U1S_SLED_ON				(u1)( 0x06 )
#define U1S_WLED_ON				(u1)( 0x07 )

#define U1S_CLED_ON				(u1)( 0x80 )

#define U1S_PHLED_ON			(u1)( 0x01 )
#define U1S_FLLED1_ON			(u1)( 0x02 )
#define U1S_PHFL1LED_ON			(u1)( 0x03 )
#define U1S_FLLED2_ON			(u1)( 0x04 )
#define U1S_PHFL2LED_ON			(u1)( 0x05 )
#define U1S_FLLED_ON			(u1)( 0x06 )
#define U1S_ALLLED_ON			(u1)( 0x07 )

#define U2S_LEDCNT_WAIT_TIME	(u2)( 1000 )		/* Each LED lighting examination interval of time(Unit:ms)*/
#define U1S_CLED_TEST_CNT		(u1)( 3 )

//#define U2S_LEDCNT_XRST_PORT	(u2)( 13 )			/* Reset PORT number for LED controller */
#define U2S_LEDCNT_XRST_PORT	(u2)( 235 )			/* Reset PORT number for LED controller */
#define U1S_LEDCNT_XRST_WAIT	(u2)( 24 )			/* LEDCNTreset recommendation WAIT time(Unit:ms)*/
#define U1S_LEDCNT_SOFTRESET	(u1)( 0x01 )		/* LEDCNT software reset  */
#define U1S_LEDCNT_GUARD_BIT	(u1)( 0xAA )		/* LEDCNT software reset config  */
#define U2S_LEDCNT_COT_PULSE	(u2)( 136 )			/* LEDCNT COT_PULSEópPORT */

#define U1S_RGB1_CH				(u1)( 0x00 )		/* RGB1 CH */
#define U1S_RGB2_CH				(u1)( 0x01 )		/* RGB2 CH */
#define U1S_RGB_SET_MASK		(u1)( 0x07 )		/* RGB  BGR config Mask */
#define U1S_RGB2_SHIFTBIT		(u1)( 0x04 )		/* RGB2 BGR config shift bit */
#define U1S_RGB1_10MA_SET		(u1)( 0x00 )		/* RGB1 BGR Current config  10mA */
#define U1S_RGB1_20MA_SET		(u1)( 0x15 )		/* RGB1 BGR Current config  20mA */
#define U1S_RGB2_10MA_SET		(u1)( 0x00 )		/* RGB2 BGR Current config  10mA */
#define U1S_RGB2_20MA_SET		(u1)( 0x2A )		/* RGB2 BGR Current config  20mA */
#define U1S_RGB1_ON_SET			(u1)( 0x01 )		/* REG1ONconfig  */
#define U1S_RGB2_ON_SET			(u1)( 0x04 )		/* REG2ONconfig  */
#define U1S_RGBALL_ON_SET		(u1)( 0x05 )		/* REG1,REG2ONconfig  */
#define U1S_RGBALL_OFF_SET		(u1)( 0x00 )		/* REG1,REG2OFFconfig  */
#define U1S_ALL_CTRL_ON			(u1)( 0x77 )		/* RGB1,RGB2êßå‰ON */

#define U1S_PSCONT1				(u1)( 0x00 )		/* REG1,REG2 ON/OFF control register */
#define U1S_RSV01				(u1)( 0x01 )		/* unused register */
#define U1S_REGVSET				(u1)( 0x02 )		/* REG1 Voltage config register */
#define U1S_LEDCNT1				(u1)( 0x03 )		/* LED Driver ON/OFF control register, the individual selection is done by [PORTSEL] register */
#define U1S_PORTSEL				(u1)( 0x04 )		/* MAIN Driver Selection register */
#define U1S_RGB1CNT1			(u1)( 0x05 )		/* RGB1 Driver Current(10/20mA)config register */
#define U1S_RGB1CNT2			(u1)( 0x06 )		/* RGB1 External edgesub synchronization config ON/OFF register */
#define U1S_RGB1CNT3			(u1)( 0x07 )		/* RGB1R DUTY config register */
#define U1S_RGB1CNT4			(u1)( 0x08 )		/* RGB1G DUTY config register */
#define U1S_RGB1CNT5			(u1)( 0x09 )		/* RGB1B DUTY Setting register */
#define U1S_RGB1FLM				(u1)( 0x0A )		/* RGB1 Clock setting register */
#define U1S_RGB2CNT1			(u1)( 0x0B )		/* RGB2Driver Current setting(10/20mA)register */
#define U1S_RGB2CNT2			(u1)( 0x0C )		/* RGB2 External Edge sub synchronization setting ON/OFF register */
#define U1S_RGB2CNT3			(u1)( 0x0D )		/* RGB2R DUTY Setting register */
#define U1S_RGB2CNT4			(u1)( 0x0E )		/* RGB2G DUTY Setting register */
#define U1S_RGB2CNT5			(u1)( 0x0F )		/* RGB2B DUTY Setting register */
#define U1S_RGB2FLM				(u1)( 0x10 )		/* RGB2 Clock setting register */
#define U1S_PSCONT3				(u1)( 0x11 )		/* REG3 ON/OFF control register */
#define U1S_SMMONCNT			(u1)( 0x12 )		/* control register */
#define U1S_DCDCCNT				(u1)( 0x13 )		/* backlightópDCDC Overcurrent overvoltage protection value switch */
#define U1S_IOSEL				(u1)( 0x14 )		/* GPIO I/O Selection register */
#define U1S_OUT1				(u1)( 0x15 )		/* GPIO Output level setting register */
#define U1S_OUT2				(u1)( 0x16 )		/* GPO Output level setting register OUT04,03 Default (OUT04:Hiz)*/
#define U1S_MASK1				(u1)( 0x17 )		/* GPIO Interrupt mask register */
#define U1S_MASK2				(u1)( 0x18 )		/* DCDC Protection interrupt mask register */
#define U1S_FACTOR1				(u1)( 0x19 )		/* GPIO Causes of interrupts reading register */
#define U1S_FACTOR2				(u1)( 0x1A )		/* DCDC Causes of interrupts reading register */
#define U1S_CLRFACT1			(u1)( 0x1B )		/* GPIO Flag (FACTOR1) clear register */
#define U1S_CLRFACT2			(u1)( 0x1C )		/* DCDC Protection circuit operation flag (FACTOR2) clear register */
#define U1S_STATE1				(u1)( 0x1D )		
#define U1S_LSIVER				(u1)( 0x1E )		/* LSI Version reading register */
#define U1S_GRPSEL				(u1)( 0x1F )		/* MAIN Driver Grouping selection register */
#define U1S_LEDCNT2				(u1)( 0x20 )		/* Individual ON/OFF RGB1MODE/RGB2MODE:RGB/BL mode switching and RGB Driver settings register */
#define U1S_LEDCNT3				(u1)( 0x21 )		/* EXTPWMEN: External PWM inputting enable control PWMSEL:PWM control switch */
#define U1S_MCURRENT			(u1)( 0x22 )		/* MAIN Driver Current setting(16/20mA)register */
#define U1S_MAINCNT1			(u1)( 0x23 )		/* MAIN brilliance control register/smoothing counter monitor */
#define U1S_MAINCNT2			(u1)( 0x24 )		/* SUB Side brightness adjustment register */
#define U1S_SLOPECNT			(u1)( 0x25 )		/* ON/OFF Setting register */
#define U1S_MSLOPE				(u1)( 0x26 )		/* MAIN Setting register */
#define U1S_RGBSLOPE			(u1)( 0x27 )		/* RGB1/2 Setting register */
#define U1S_RSV28				(u1)( 0x28 )		/* unused register */
#define U1S_TEST				(u1)( 0x29 )		/* Test register */
#define U1S_SFTRST				(u1)( 0x2A )		/* softreset (Auto return'0') */
#define U1S_SFTRSTGD			(u1)( 0x2B )		/* softreset Guard bit */



/***************************************************************/
/* STATIC TYPEDEF ENUM                                         */
/***************************************************************/

/***************************************************************/
/* STATIC VARIABLE                                             */
/***************************************************************/

/***************************************************************/
/* STATIC FUNCTION PROTOTYPE                                   */
/***************************************************************/
// static void vos_ledcnt_init( void );
static void vos_ledcnt_soft_reset( void );
// static void vos_rgbled1_set( u1 u1t_set_data );
// static void vos_rgbled2_set( u1 u1t_set_data );
static void vos_ledcnt_i2c_write( u1 u1t_reg_addr, u1 u1t_write_data );

/***************************************************************/
/* PUBLIC VARIABLE PROTOTYPE                                   */
/***************************************************************/

/***************************************************************/
/* PUBLIC FUNCTION PROTOTYPE                                   */
/***************************************************************/
void vog_ledcnt_xreset( void );
void vog_ledcnt_soft_reset( void );
void vog_ledcnt_backlight_on( void );
void vog_ledcnt_backlight_off( void );


/*************************************************************************************************/
/* Function name   : vos_ledcnt_soft_reset(void)                                                 */
/* Input     : void                                                                              */
/* Return   : void                                                                               */
/* Processing : LEDCNT software reset 		                                                     */
/*************************************************************************************************/
static void vos_ledcnt_soft_reset( void )
{
	/*  software reset*/
	vos_ledcnt_i2c_write( U1S_SFTRSTGD, U1S_LEDCNT_GUARD_BIT );
	/*  software reset  */
	vos_ledcnt_i2c_write( U1S_SFTRST, U1S_LEDCNT_SOFTRESET );
}

/*************************************************************************************************/
/* Function name   : vog_ledcnt_xreset( void )                                                   */
/* Input     : void                                                                              */
/* Return   : void                                                                               */
/* Processing : LEDCONT_hardware reset			                                                 */
/*************************************************************************************************/
void vog_ledcnt_xreset( void )
{

	/* LEDCNT_XRST LO LEDCNTreset  */
	vog_port_set( U2S_LEDCNT_XRST_PORT, U1G_PORT_LO );
	/* LEDCNT_XRST PORT is changed to the PORT writing control. */
	vog_port_st_set( U2S_LEDCNT_XRST_PORT, 0x10 );
	/* 24ms Wait */
	vog_timer_ms_wait( U1S_LEDCNT_XRST_WAIT );
	/* LEDCNT_XRST HI LEDCNT reset  */
	vog_port_set( U2S_LEDCNT_XRST_PORT, U1G_PORT_HI );

}

/*************************************************************************************************/
/* Function name   : vos_ledcnt_i2c_write( u1 u1t_reg_addr, u1 u1t_write_data )                         */
/* Input     : u1 u1t_reg_addr, u1 u1t_write_data                        */
/* Return   : void                                                                               */
/* Processing : LEDCONT I2C communication transmission processing                                                            */
/*************************************************************************************************/
static void vos_ledcnt_i2c_write( u1 u1t_reg_addr, u1 u1t_write_data )
{
	i2c_1byte_write( U1G_I2C_CH1 , U1G_I2C_CH1, U1G_ESCL400K, U1G_SLADR_LEDCNT, u1t_reg_addr ,u1t_write_data );
}

/*************************************************************************************************/
/* Function name   : vog_ledcnt_backlight_on( void )                                              */
/* Input     : void                                                                               */
/* Return   : void                                                                               */
/* Processing : LEDCONT LCDópbacklightON                                                        */
/*************************************************************************************************/
void vog_ledcnt_backlight_on( void )
{
	vog_ledcnt_xreset();

	vos_ledcnt_i2c_write( 0x13, 0x01 );
	/* MAIN Driver LEDM0 ON */
	vos_ledcnt_i2c_write( 0x04, 0x01 );
	/* PWM ON DUTY100% */
	vos_ledcnt_i2c_write( 0x23, 0x80 );
	
	vos_ledcnt_i2c_write( 0x22, 0x01 );

	/* MAIN Driver ON */
	vos_ledcnt_i2c_write( 0x03, 0x01 );
	/* Overvoltage protection function release */
	vos_ledcnt_i2c_write( 0x29, 0x24 );
	
	vos_ledcnt_i2c_write( 0x11, 0x01 );

}

/*************************************************************************************************/
/* Function name   : vog_ledcnt_backlight_off( void )                                                    */
/* Input     : void                                                                               */
/* Return   : void                                                                               */
/* Processing : LEDCONT LCD backlight OFF                                                        */
/*************************************************************************************************/
void vog_ledcnt_backlight_off( void )
{
	vog_ledcnt_xreset();

	/* MAIN Driver LEDM0,1,2 OFF */
	vos_ledcnt_i2c_write( 0x04, 0x00 );
	/* PWM ON DUTY0% */
	vos_ledcnt_i2c_write( 0x23, 0x00 );
	/* MAIN Driver OFF */
	vos_ledcnt_i2c_write( 0x03, 0x01 );
}

/*************************************************************************************************/
/* Function name   : vog_ledcnt_soft_reset(void)                                                 */
/* Input     : void                                                                              */
/* Return   : void                                                                               */
/* Processing : LEDCNT software reset                                                    		 */
/*************************************************************************************************/
void vog_ledcnt_soft_reset( void )
{
	vos_ledcnt_soft_reset();
}
