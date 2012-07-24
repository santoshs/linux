/*
 * i2c_drv.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */
/***************************************************************/
/* INCLUDE FILE                                                */
/***************************************************************/
#include "eos_stdio.h"					/* EOS-Standard Header */
#include "cp1_c01_cpg.h"				/* CPG REG Header      */

#include "port_io_drv.h"				/* header file  	*/
#include "timer_drv.h"					/* header file    	*/
#include "gpio_led_drv.h"				/* LED header file 	*/

#include "i2c_drv.h"					/* I2C driver header file */

/***************************************************************/
/* STATIC CONSTANT DEFINE                                      */
/***************************************************************/
#define U1S_I2C_CMD_WRITE			(u1)( 0x00 )
#define U1S_I2C_CMD_READ			(u1)( 0x01 )

#define U1S_I2C_NONE_ERR			(u1)( 0 )
#define U1S_I2C_ERR_SET_CH			(u1)( 1 )
#define U1S_I2C_ERR_TIMEOUT			(u1)( 2 )

#define U1G_I2C_REG_CLEAR			(u1)( 0x00 )

#define U1S_I2C_START_COND			(u1)( 0x94 )
#define U1S_I2C_STOP_COND			(u1)( 0x90 )
#define U1S_I2C_T2R_CHANGE			(u1)( 0x81 )
#define U1S_I2C_STOP_CONDR			(u1)( 0xC0 )

#define U1S_I2C_BUS_RESET			(u1)( 0x00 )
#define U1S_I2C_ICCR_ICE_BIT		(u1)( 0x80 )
#define U1S_I2C_ICSR_DTE_BIT		(u1)( 0x01 )
#define U1S_I2C_ICSR_WAIT_BIT		(u1)( 0x02 )
#define U1S_I2C_ICSR_BUSY_BIT		(u1)( 0x10 )
#define U1S_I2C_ICIC_DTEE_BIT		(u1)( 0x01 )
#define U1S_I2C_ICIC_WAITE_BIT		(u1)( 0x02 )
#define U1S_I2C_ICTC_SET			(u1)( 0x00 )

#define U1S_I2C_INT_DTE				(u1)( 0 )
#define U1S_I2C_INT_WAIT			(u1)( 1 )
#define U1S_I2C_INT_BUSY			(u1)( 2 )
#define U1S_I2C_DTE_BITSHIFT		(u1)( 0 )
#define U1S_I2C_WAIT_BITSHIFT		(u1)( 1 )
#define U1S_I2C_BUSY_BITSHIFT		(u1)( 4 )

#define U2S_I2C_TIMEOUT				(u2)( 100000 )		/* Unit us(100ms) */
#define U2S_I2C_WAIT_CYC			(u2)( 10 )			/* Unit us(10us) */

//#define U4S_ICDR0_ADDR				(u4)(U1R_ICDR0)

/* for AG5 DEBUG( ※DELETE Later ) */
#define	U1R_PORT236CR				(*(VU1 *)(GPIO_BADR+0x30EC))		/* PORT236 control register	Initial value: 0xA0 */
#define	U1R_PORT237CR				(*(VU1 *)(GPIO_BADR+0x30ED))		/* PORT237 control register	Initial value: 0xA0 */

#define	U1R_PORT248CR				(*(VU1 *)(GPIO_BADR+0x30F8))		/* PORT248 control register	Initial value: 0xA0 */
#define	U1R_PORT249CR				(*(VU1 *)(GPIO_BADR+0x30F9))		/* PORT249 control register	Initial value: 0xA0 */

#define	U4S_ICDR0_ADDR				((u4)(I2C0_BADR+0x0000))			/* I2C register */
#define	U4S_ICCR0_ADDR				((u4)(I2C0_BADR+0x0004))			/* I2C register */
#define	U4S_ICSR0_ADDR				((u4)(I2C0_BADR+0x0008))			/* I2C register */
#define	U4S_ICIC0_ADDR				((u4)(I2C0_BADR+0x000C))			/* I2C register */
#define	U4S_ICCL0_ADDR				((u4)(I2C0_BADR+0x0010))			/* I2C register */
#define	U4S_ICCH0_ADDR				((u4)(I2C0_BADR+0x0014))			/* I2C register */
#define	U4S_ICTC0_ADDR				((u4)(I2C0_BADR+0x0028))			/* I2C register */

#define	U4S_ICDR1_ADDR				((u4)(I2C1_BADR+0x0000))			/* I2C register */
#define	U4S_ICCR1_ADDR				((u4)(I2C1_BADR+0x0004))			/* I2C register */
#define	U4S_ICSR1_ADDR				((u4)(I2C1_BADR+0x0008))			/* I2C register */
#define	U4S_ICIC1_ADDR				((u4)(I2C1_BADR+0x000C))			/* I2C register */
#define	U4S_ICCL1_ADDR				((u4)(I2C1_BADR+0x0010))			/* I2C register */
#define	U4S_ICCH1_ADDR				((u4)(I2C1_BADR+0x0014))			/* I2C register */
#define	U4S_ICTC1_ADDR				((u4)(I2C1_BADR+0x0028))			/* I2C register */

#define	U4S_ICDR2_ADDR				((u4)(I2C2_BADR+0x0000))			/* I2C register */
#define	U4S_ICCR2_ADDR				((u4)(I2C2_BADR+0x0004))			/* I2C register */
#define	U4S_ICSR2_ADDR				((u4)(I2C2_BADR+0x0008))			/* I2C register */
#define	U4S_ICIC2_ADDR				((u4)(I2C2_BADR+0x000C))			/* I2C register */
#define	U4S_ICCL2_ADDR				((u4)(I2C2_BADR+0x0010))			/* I2C register */
#define	U4S_ICCH2_ADDR				((u4)(I2C2_BADR+0x0014))			/* I2C register */
#define	U4S_ICTC2_ADDR				((u4)(I2C2_BADR+0x0028))			/* I2C register */

#define	U4S_ICDR3_ADDR				((u4)(I2C3_BADR+0x0000))			/* I2C register */
#define	U4S_ICCR3_ADDR				((u4)(I2C3_BADR+0x0004))			/* I2C register */
#define	U4S_ICSR3_ADDR				((u4)(I2C3_BADR+0x0008))			/* I2C register */
#define	U4S_ICIC3_ADDR				((u4)(I2C3_BADR+0x000C))			/* I2C register */
#define	U4S_ICCL3_ADDR				((u4)(I2C3_BADR+0x0010))			/* I2C register */
#define	U4S_ICCH3_ADDR				((u4)(I2C3_BADR+0x0014))			/* I2C register */
#define	U4S_ICTC3_ADDR				((u4)(I2C3_BADR+0x0028))			/* I2C register */

#define	U4S_ICDR0H_ADDR				((u4)(I2C0H_BADR+0x0000))			/* I2C register */
#define	U4S_ICCR0H_ADDR				((u4)(I2C0H_BADR+0x0004))			/* I2C register */
#define	U4S_ICSR0H_ADDR				((u4)(I2C0H_BADR+0x0008))			/* I2C register */
#define	U4S_ICIC0H_ADDR				((u4)(I2C0H_BADR+0x000C))			/* I2C register */
#define	U4S_ICCL0H_ADDR				((u4)(I2C0H_BADR+0x0010))			/* I2C register */
#define	U4S_ICCH0H_ADDR				((u4)(I2C0H_BADR+0x0014))			/* I2C register */
#define	U4S_ICTC0H_ADDR				((u4)(I2C0H_BADR+0x0028))			/* I2C register */

#define	U4S_ICDR1H_ADDR				((u4)(I2C1H_BADR+0x0000))			/* I2C register */
#define	U4S_ICCR1H_ADDR				((u4)(I2C1H_BADR+0x0004))			/* I2C register */
#define	U4S_ICSR1H_ADDR				((u4)(I2C1H_BADR+0x0008))			/* I2C register */
#define	U4S_ICIC1H_ADDR				((u4)(I2C1H_BADR+0x000C))			/* I2C register */
#define	U4S_ICCL1H_ADDR				((u4)(I2C1H_BADR+0x0010))			/* I2C register */
#define	U4S_ICCH1H_ADDR				((u4)(I2C1H_BADR+0x0014))			/* I2C register */
#define	U4S_ICTC1H_ADDR				((u4)(I2C1H_BADR+0x0028))			/* I2C register */

#define	U4S_ICDR2H_ADDR				((u4)(I2C2H_BADR+0x0000))			/* I2C register */
#define	U4S_ICCR2H_ADDR				((u4)(I2C2H_BADR+0x0004))			/* I2C register */
#define	U4S_ICSR2H_ADDR				((u4)(I2C2H_BADR+0x0008))			/* I2C register */
#define	U4S_ICIC2H_ADDR				((u4)(I2C2H_BADR+0x000C))			/* I2C register */
#define	U4S_ICCL2H_ADDR				((u4)(I2C2H_BADR+0x0010))			/* I2C register */
#define	U4S_ICCH2H_ADDR				((u4)(I2C2H_BADR+0x0014))			/* I2C register */
#define	U4S_ICTC2H_ADDR				((u4)(I2C2H_BADR+0x0028))			/* I2C register */

#define	U4S_ICDR3H_ADDR				((u4)(I2C3H_BADR+0x0000))			/* I2C register */
#define	U4S_ICCR3H_ADDR				((u4)(I2C3H_BADR+0x0004))			/* I2C register */
#define	U4S_ICSR3H_ADDR				((u4)(I2C3H_BADR+0x0008))			/* I2C register */
#define	U4S_ICIC3H_ADDR				((u4)(I2C3H_BADR+0x000C))			/* I2C register */
#define	U4S_ICCL3H_ADDR				((u4)(I2C3H_BADR+0x0010))			/* I2C register */
#define	U4S_ICCH3H_ADDR				((u4)(I2C3H_BADR+0x0014))			/* I2C register */
#define	U4S_ICTC3H_ADDR				((u4)(I2C3H_BADR+0x0028))			/* I2C register */

#define	U1R_ICVCON4					(*(VU1 *)(IIC4_BADR+0x006C))		/* I2C register */

#define U2S_I2C_SCL0H_PORT			(u2)( 84 )
#define U2S_I2C_SDA0H_PORT			(u2)( 85 )
#define U2S_I2C_SCL1H_PORT			(u2)( 86 )
#define U2S_I2C_SDA1H_PORT			(u2)( 87 )
#define U2S_I2C_SCL2H_PORT			(u2)( 82 )
#define U2S_I2C_SDA2H_PORT			(u2)( 83 )
#define U2S_I2C_SCL3H_PORT			(u2)( 273 )
#define U2S_I2C_SDA3H_PORT			(u2)( 274 )
#define U1S_I2C0H_PORT_SET			(u1)( 0xD2 )
#define U1S_I2C1H_PORT_SET			(u2)( 0xD2 )
#define U1S_I2C2H_PORT_SET			(u2)( 0xD2 )
#define U1S_I2C3H_PORT_SET			(u2)( 0xD2 )

/***************************************************************/
/* STATIC TYPEDEF                                              */
/***************************************************************/
typedef struct {
	u4 u4_icdr_addr;
	u4 u4_iccr_addr;
	u4 u4_icsr_addr;
	u4 u4_icic_addr;
	u4 u4_iccl_addr;
	u4 u4_icch_addr;
	u4 u4_ictc_addr;
}ST_I2C_REG_SET;

typedef struct {
	u1 u1_iccl;
	u1 u1_icch;
}ST_I2C_CLK_SET;

/***************************************************************/
/* STATIC VARIABLE                                             */
/***************************************************************/

/***************************************************************/
/* STATIC FUNCTION PROTOTYPE                                   */
/***************************************************************/
static u1 u1s_i2c_reg_set( u1 u1t_set_ch, ST_I2C_REG_SET* ptt_i2c_reg );
//static u1 u1s_i2c_clk_set( u1 u1t_set_clk, u1* ptt_iccl, u1* ptt_icch );
static u1 u1s_i2c_clk_set( u1 u1t_set_clk, u1* ptt_iccl, u1* ptt_icch, u1* ptt_icic );
static void vos_i2c_reset( u1 u1t_i2c_ch_no );
static u1 u1s_i2c_timeout_check( u1 u1t_i2c_dev_ch, VU1* ptt_reg_data, u1 u1t_check_data, u2 u2t_timeout_set );

/***************************************************************/
/* PUBLIC VARIABLE PROTOTYPE                                   */
/***************************************************************/

/***************************************************************/
/* PUBLIC FUNCTION PROTOTYPE                                   */
/***************************************************************/
IB i2c_1byte_write( u1 dev_ch, u1 ch_no, u1 clk_sp_type, u1 slave_addr, u1 sub_addr, u1  w_data );
IB i2c_1byte_read ( u1 dev_ch, u1 ch_no, u1 clk_sp_type, u1 slave_addr, u1 sub_addr, u1* r_data );
IB i2c_nbyte_write( u1 dev_ch, u1 ch_no, u1 clk_sp_type, u1 slave_addr, u1 data_num, u1* w_data );
IB i2c_nbyte_read ( u1 dev_ch, u1 ch_no, u1 clk_sp_type, u1 slave_addr, u1 data_num, u1* w_data, u1 r_data_num, u1* r_data );

/*************************************************************************************************/
/* Function name   : i2c_1byte_write( * ) *                                                 */
/* Input    : UB dev_ch( 0:I2C_RT 1:I2C_SYS ) ,                                                  */
/*          : UB ch_no( When I2C_RT is used 0:CH0 1:CH1, When I2C_SYS is used 0:CH1 1;CH0 ),                   */
/*          : UB clk_sp_type( 0:100[KHz] 1:200[KHz] 2:300[KHz] 3:400[KHz] ) ,                    */
/*          : UB slave_addr(Slave address), UB sub_addr(Sub address), UB w_data( Writing value )    */
/* Return   : IB                                                                                 */
/* Processing : I2C 1byte Transmission processing                                                                 */
/*************************************************************************************************/
IB i2c_1byte_write(UB dev_ch , UB ch_no, UB clk_sp_type, UB slave_addr ,UB sub_addr ,UB w_data)
{

	UCHAR* reg_icdr;
	UCHAR* reg_iccr;
	UCHAR* reg_icsr;
	UCHAR* reg_icic;
	UCHAR* reg_iccl;
	UCHAR* reg_icch;
	UCHAR* reg_ictc;

	ST_I2C_REG_SET stt_i2c_reg;
	u1 iccl_select;
	u1 icch_select;
	u1 u1t_icic = 0x00;
	u1 ch_select = 0x00;
	u1 slaveaddr_set;
	u2 u2t_i;
	u1 u1t_ret = U1G_OK;

	/* I2C CH REG SETTING */
	u1t_ret = u1s_i2c_reg_set( dev_ch, &stt_i2c_reg );
	reg_icdr = (VU1 *)stt_i2c_reg.u4_icdr_addr;
	reg_iccr = (VU1 *)stt_i2c_reg.u4_iccr_addr;
	reg_icsr = (VU1 *)stt_i2c_reg.u4_icsr_addr;
	reg_icic = (VU1 *)stt_i2c_reg.u4_icic_addr;
	reg_iccl = (VU1 *)stt_i2c_reg.u4_iccl_addr;
	reg_icch = (VU1 *)stt_i2c_reg.u4_icch_addr;
	reg_ictc = (VU1 *)stt_i2c_reg.u4_ictc_addr;

	/* CLK SETTING */
	u1t_ret = u1s_i2c_clk_set( clk_sp_type, &iccl_select, &icch_select, &u1t_icic );

	if( u1t_ret )
	{
		return u1t_ret;
	}

/*********************************************************************/
/* DATA SEND SEQUENS                                                 */
/*********************************************************************/
	*reg_iccr = 0x00;										/* I2C bus reset */

	*reg_iccr |= 0x80;										/* ICCR.ICE[7] I2C is enable. */

	*reg_iccl = iccl_select;								/* Clock Speed setting */
	*reg_icch = icch_select;								/* Clock Speed setting */
	*reg_icic |= u1t_icic;									/* ICIC[7:6] Setting */

	*reg_ictc = ch_select;									/* CH is selected. */

	*reg_icic |= 0x03;										/* WAIT DTE enabling interrupt */

	*reg_iccr = 0x94;										/* Start Condition */


	/* DTE waiting */
	u1t_ret = u1s_i2c_timeout_check( dev_ch, reg_icsr, 0x01, U2S_I2C_TIMEOUT );
	if( u1t_ret != 0 )
	{
		//monprintf("I2C_START_COND_TIMEOUT ERROR!\n");
		return u1t_ret;
	}

	*reg_icic &= ~(0x01);									/* DTE interrupt inhibit */
    slaveaddr_set = (((slave_addr<<1) + U1G_I2C_CMD_WRITE) & 0xFF);
	*reg_icdr = slaveaddr_set;  							/* Slave address */

	/* WAIT interrupt waiting */
	u1t_ret = u1s_i2c_timeout_check( dev_ch, reg_icsr, 0x02, U2S_I2C_TIMEOUT );
	if( u1t_ret != 0 )
	{
		return u1t_ret;
	}

	*reg_icdr = sub_addr;  									/* Sub Address */
	/* WAIT, clear causes of interrupts */
	*reg_icsr &= ~(0x02);

	/* WAIT interrupt waiting */
	u1t_ret = u1s_i2c_timeout_check( dev_ch, reg_icsr, 0x02, U2S_I2C_TIMEOUT );
	if( u1t_ret != 0 )
	{
			return u1t_ret;
	}

	*reg_icdr = w_data;  									/* DATA Write */
	/* WAIT, clear causes of interrupts */
	*reg_icsr &= ~(0x02);

	/* WAIT interrupt waiting */
	u1t_ret = u1s_i2c_timeout_check( dev_ch, reg_icsr, 0x02, U2S_I2C_TIMEOUT );
	if( u1t_ret != 0 )
	{
			return u1t_ret;
	}

	/* WAIT, clear causes of interrupts */
	*reg_icsr &= ~(0x02);
	*reg_iccr = 0x90;										/* Stop Condition */

	/* Waiting of bus open */
	u1t_ret = u1s_i2c_timeout_check( dev_ch, reg_icsr, 0xC0, U2S_I2C_TIMEOUT );
	if( u1t_ret != 0 )
	{
			return u1t_ret;
	}

	/* Clear interrupt status register */
	*reg_icsr = 0x00;
	/* Clear interrupt control register */
	*reg_icic = 0x00;
	/* ICCR.ICE(Bit7) = 0 */
	*reg_iccr &= ~(0x80);

	/* WAIT, because the following cannot start early */
	for( u2t_i = 0; u2t_i < 0x1000; u2t_i++ );

	/* Normal terminal */
	return 0;

}

/*************************************************************************************************/
/* Function name   : i2c_1byte_read( * ) *                                                  */
/* Input     : UB dev_ch( 0:I2C_RT 1:I2C_SYS ) ,                                                  */
/*          : UB ch_no( When I2C_RT is used 0:CH0 1:CH1, When I2C_SYS is used 0:CH1 1;CH0 ),                   */
/*          : UB clk_sp_type( 0:100[KHz] 1:200[KHz] 2:300[KHz] 3:400[KHz] ) ,                    */
/*          : UB slave_addr(Slave address), UB sub_addr(Sub address), UB* r_data( Reading value ) */
/* Return   : IB                                                                                 */
/* Processing : I2C 1byte Receive processing                                                                 */
/*************************************************************************************************/
IB i2c_1byte_read(UB dev_ch ,UB ch_no, UB clk_sp_type, UB slave_addr ,UB sub_addr ,UB* r_data )
{

	UCHAR* reg_icdr;
	UCHAR* reg_iccr;
	UCHAR* reg_icsr;
	UCHAR* reg_icic;
	UCHAR* reg_iccl;
	UCHAR* reg_icch;
	UCHAR* reg_ictc;

	ST_I2C_REG_SET stt_i2c_reg;
	u1 iccl_select;
	u1 icch_select;
	u1 u1t_icic = 0x00;
	u1 ch_select = 0x00;
	u1 slaveaddr_set;
	u2 u2t_i;
	u1 u1t_ret = U1G_OK;

/*********************************************************************/
/* chanel Setting                                                    */
/*********************************************************************/

	/* I2C CH REG SETTING */
	u1t_ret = u1s_i2c_reg_set( dev_ch, &stt_i2c_reg );
	if( u1t_ret != 0 )
	{
		return u1t_ret;
	}
	reg_icdr = (VU1 *)stt_i2c_reg.u4_icdr_addr;
	reg_iccr = (VU1 *)stt_i2c_reg.u4_iccr_addr;
	reg_icsr = (VU1 *)stt_i2c_reg.u4_icsr_addr;
	reg_icic = (VU1 *)stt_i2c_reg.u4_icic_addr;
	reg_iccl = (VU1 *)stt_i2c_reg.u4_iccl_addr;
	reg_icch = (VU1 *)stt_i2c_reg.u4_icch_addr;
	reg_ictc = (VU1 *)stt_i2c_reg.u4_ictc_addr;

/******************************************************************************************/
/* chanel Setting    0:100[KHz] 1:200[KHz] 2:300[KHz] 3:400[KHz]                          */
/******************************************************************************************/
	/* CLK SETTING */
	u1t_ret = u1s_i2c_clk_set( clk_sp_type, &iccl_select, &icch_select, &u1t_icic );

	if( u1t_ret )
	{
		return u1t_ret;
	}

/*********************************************************************/
/* DATA RECIVE SEQUENS                                               */
/*********************************************************************/
	/* I2C bus reset */
	*reg_iccr = 0x00;
	/* ICCR.ICE[7] I2C is enable. */
	*reg_iccr |= 0x80;
	/* ICCL(Clock Speed) setting */
	*reg_iccl = iccl_select;
	/* ICCH(Clock Speed) setting */
	*reg_icch = icch_select;
	/* ICIC[7:6] Setting */
	*reg_icic |= u1t_icic;

	/* Output PIN of each CH is selected :0( PIN output as CH number)Fix */
	*reg_ictc = ch_select;
	/* WAIT DTE enabling interrupt */
	*reg_icic |= 0x03;
	/* Start Condition */
	*reg_iccr = 0x94;

	/* DTE waiting */
	u1t_ret = u1s_i2c_timeout_check( dev_ch, reg_icsr, 0x01, U2S_I2C_TIMEOUT );
	if( u1t_ret != 0 )
	{
		return u1t_ret;
	}

	*reg_icic &= ~(0x01);									/* DTE interrupt inhibit */
    slaveaddr_set = (((slave_addr<<1) + U1G_I2C_CMD_WRITE ) & 0xFF) ;	
	*reg_icdr = slaveaddr_set;  							/* Slave address */

	/* WAIT interrupt waiting */
	u1t_ret = u1s_i2c_timeout_check( dev_ch, reg_icsr, 0x02, U2S_I2C_TIMEOUT );
	if( u1t_ret != 0 )
	{
		return u1t_ret;
	}

	*reg_icdr = sub_addr;  									/* Sub Address */

	*reg_iccr = 0x94;										/* READ用START Condition */
	*reg_icic |= 0x01;										/* DTE enabling interrupt */
	
	/* WAIT, clear causes of interrupts */
	*reg_icsr &= ~(0x02);

	/* WAIT interrupt waiting */
	u1t_ret = u1s_i2c_timeout_check( dev_ch, reg_icsr, 0x02, U2S_I2C_TIMEOUT );
	if( u1t_ret != 0 )
	{
		return u1t_ret;
	}

	/* WAIT, clear causes of interrupts */
	*reg_icsr &= ~(0x02);

    /***********************************************************************/
    /* Data read phase                                                 */
    /***********************************************************************/
	/* DTE waiting */
	u1t_ret = u1s_i2c_timeout_check( dev_ch, reg_icsr, 0x01, U2S_I2C_TIMEOUT );
	if( u1t_ret != 0 )
	{
		return u1t_ret;
	}

	*reg_icic |= 0x02;										/* WAIT Enable interrupt */
	*reg_icic &= ~(0x01);									/* DTE interrupt inhibit */

    slaveaddr_set = (((slave_addr<<1) + U1G_I2C_CMD_READ ) & 0xFF) ;
	*reg_icdr = slaveaddr_set;  							/* Slave address */

	/* WAIT */
	u1t_ret = u1s_i2c_timeout_check( dev_ch, reg_icsr, 0x02, U2S_I2C_TIMEOUT );
	if( u1t_ret != 0 )
	{
		return u1t_ret;
	}

	*reg_iccr = 0x81;										/* Switch of sending and receiving */
	/* WAIT, clear causes of interrupts */
	*reg_icsr &= ~(0x02);

	/* WAIT */
	u1t_ret = u1s_i2c_timeout_check( dev_ch, reg_icsr, 0x02, U2S_I2C_TIMEOUT );
	if( u1t_ret != 0 )
	{
		return u1t_ret;
	}

	*reg_icic |= 0x01;										/* DTE enabling interrupt */
	*reg_iccr=0xC0;											/* Stop Condition */
	/* WAIT, clear causes of interrupts */
	*reg_icsr &= ~(0x02);

	/* DTE waiting */
	u1t_ret = u1s_i2c_timeout_check( dev_ch, reg_icsr, 0x01, U2S_I2C_TIMEOUT );
	if( u1t_ret != 0 )
	{
		return u1t_ret;
	}

	*reg_icic &= ~(0x02);									/* WAIT interrupt inhibit */
	*r_data = *reg_icdr;									/* Defult DATA Read */

	/* Waiting of bus open */
	u1t_ret = u1s_i2c_timeout_check( dev_ch, reg_icsr, 0xC0, U2S_I2C_TIMEOUT );
	if( u1t_ret != 0 )
	{
		return u1t_ret;
	}

	/* Clear interrupt status register */
	*reg_icsr = 0x00;
	/* Clear interrupt control register */
	*reg_icic = 0x00;
	/* ICCR.ICE(Bit7) = 0 */
	*reg_iccr &= ~(0x80);

	/* WAIT, because the following cannot start early */
	for( u2t_i = 0; u2t_i < 0x1000; u2t_i++ );
	/* Normal terminal */
	return 0;
}


/*************************************************************************************************/
/* Function name   : i2c_nbyte_write( * ) *                                                 					*/
/* Input     : UB dev_ch( 0:I2C_RT 1:I2C_SYS ) ,                                                  				*/
/*          : UB ch_no( When I2C_RT is used 0:CH0 1:CH1, When I2C_SYS is used 0:CH1 1;CH0 ),    	              */
/*          : UB clk_sp_type( 0:100[KHz] 1:200[KHz] 2:300[KHz] 3:400[KHz] ) ,					                  */
/*          : UB slave_addr(Slave address), UB data_num(Number of transmission data), UB* w_data(Writing value)   */
/* Return   : IB                                     					                                          */
/* Processing : I2C nbyte Transmission processing                                                                 */
/*************************************************************************************************/
IB i2c_nbyte_write(UB dev_ch, UB ch_no, UB clk_sp_type, UB slave_addr, UB data_num, UB* w_data )
{

	UCHAR* reg_icdr;
	UCHAR* reg_iccr;
	UCHAR* reg_icsr;
	UCHAR* reg_icic;
	UCHAR* reg_iccl;
	UCHAR* reg_icch;
	UCHAR* reg_ictc;
   
	ST_I2C_REG_SET stt_i2c_reg;
	u1 iccl_select;
	u1 icch_select;
	u1 u1t_icic = 0x00;
	u1 ch_select = 0x00;
	u1 slaveaddr_set;
	u2 u2t_i;
	u1 u1t_ret = U1G_OK;
	int nRemData;


/*********************************************************************/
/* chanel Setting                                                    */
/*********************************************************************/
	/* I2C CH REG SETTING */
	u1t_ret = u1s_i2c_reg_set( dev_ch, &stt_i2c_reg );
	if( u1t_ret != 0 )
	{
		return u1t_ret;
	}
	reg_icdr = (VU1 *)stt_i2c_reg.u4_icdr_addr;
	reg_iccr = (VU1 *)stt_i2c_reg.u4_iccr_addr;
	reg_icsr = (VU1 *)stt_i2c_reg.u4_icsr_addr;
	reg_icic = (VU1 *)stt_i2c_reg.u4_icic_addr;
	reg_iccl = (VU1 *)stt_i2c_reg.u4_iccl_addr;
	reg_icch = (VU1 *)stt_i2c_reg.u4_icch_addr;
	reg_ictc = (VU1 *)stt_i2c_reg.u4_ictc_addr;

/*********************************************************************/
/* chanel Setting    0:100[KHz] 1:200[KHz] 2:300[KHz] 3:400[KHz]     */
/*********************************************************************/
	/* CLK SETTING */
	u1t_ret = u1s_i2c_clk_set( clk_sp_type, &iccl_select, &icch_select, &u1t_icic );

	if( u1t_ret )
	{
		return u1t_ret;
	}

/*********************************************************************/
/* DATA SEND SEQUENS                                                 */
/*********************************************************************/
	*reg_iccr = 0x00;										/* I2C bus reset */

	*reg_iccr |= 0x80;										/* ICCR.ICE[7] I2C is enable. */

	*reg_iccl = iccl_select;								/* Clock Speed setting */
	*reg_icch = icch_select;								/* Clock Speed setting */

	*reg_icic |= u1t_icic;									/* ICIC[7:6] Setting */

	*reg_ictc = ch_select;									/* CH is selected. */

	*reg_icic |= 0x03;										/* WAIT DTE enabling interrupt */
		
	*reg_iccr = 0x94;										/* Start Condition */

	/* DTE wait */
	u1t_ret = u1s_i2c_timeout_check( dev_ch, reg_icsr, 0x01, U2S_I2C_TIMEOUT );
	if( u1t_ret != 0 )
	{
		return u1t_ret;
	}

	*reg_icic &= ~(0x01);									/* DTE interrupt inhibit */
    slaveaddr_set = (((slave_addr<<1) + U1G_I2C_CMD_WRITE) & 0xFF );
	*reg_icdr = slaveaddr_set;  							/* Slave address */

	/* WAIT interrupt waiting */
	u1t_ret = u1s_i2c_timeout_check( dev_ch, reg_icsr, 0x02, U2S_I2C_TIMEOUT );
	if( u1t_ret != 0 )
	{
		return u1t_ret;
	}
	vog_timer_ms_wait( 1 );

	/* Set data num*/
	nRemData = data_num;
/************************Begin transmit n byte data*************************///

	/* When the remainder transmission data is 1 or 0 or less,it ends (after it loops)*/
	while(nRemData != 1 || nRemData <= 0){
		*reg_icdr = (unsigned char)*w_data;  				/* Write データ */
		/* WAIT, clear causes of interrupts */
		*reg_icsr &= ~(0x02);

		/* WAIT interrupt waiting */
		u1t_ret = u1s_i2c_timeout_check( dev_ch, reg_icsr, 0x02, U2S_I2C_TIMEOUT );
		if( u1t_ret != 0 )
		{
			//monprintf("I2C_DATA[%d]_WRITE_TIMEOUT ERROR!\n", (data_num-nRemData)+1);
			return u1t_ret;
		}

		nRemData--;
		w_data++;
		vog_timer_ms_wait( 1 );
	}

	*reg_icdr = (unsigned char)*w_data;  					/* DATA Write(LastData) */
	*reg_iccr = 0x90;										/* Stop Condition */
	/* WAIT, clear causes of interrupts */
	*reg_icsr &= ~(0x02);

	/* WAIT interrupt waiting */
	u1t_ret = u1s_i2c_timeout_check( dev_ch, reg_icsr, 0x02, U2S_I2C_TIMEOUT );
	if( u1t_ret != 0 )
	{
		//monprintf("I2C_DATA_WRITE_TIMEOUT ERROR!\n");
		return u1t_ret;
	}

/************************n byte data transmission end*************************///

	/* WAIT, clear causes of interrupts */
	*reg_icsr &= ~(0x02);

	/* Waiting of bus open */
	u1t_ret = u1s_i2c_timeout_check( dev_ch, reg_icsr, 0xC0, U2S_I2C_TIMEOUT );
	if( u1t_ret != 0 )
	{
		//monprintf("I2C_DATA_WRITE_TIMEOUT ERROR!\n");
		return u1t_ret;
	}

	/* Clear interrupt status register */
	*reg_icsr = 0x00;
	/* Clear interrupt control register */
	*reg_icic = 0x00;
	/* ICCR.ICE(Bit7) = 0 */
	*reg_iccr &= ~(0x80);

	/* WAIT, because the following cannot start early */
	for( u2t_i=0; u2t_i<0x1000; u2t_i++ );

	/* Normal terminal */
	return 0;

}

/*************************************************************************************************/
/* Function name   : i2c_nbyte_read( * ) *                                                  */
/* Input     : UB dev_ch( 0:I2C_RT 1:I2C_SYS ) ,                                                  */
/*          : UB ch_no( When I2C_RT is used 0:CH0 1:CH1, When I2C_SYS is used 0:CH1 1;CH0 ),                   */
/*          : UB clk_sp_type( 0:100[KHz] 1:200[KHz] 2:300[KHz] 3:400[KHz] ) ,                    */
/*          : UB slave_addr(Slave address), UB data_num(Number of transmission data), UB* r_data(Reading value)   */
/*          : UB r_data_num(Read data byte), UB* r_data(Read data)                        */
/* Return   : IB                                                                                 */
/* Processing : I2C nbyte Receive processing                                                                 */
/*************************************************************************************************/
IB i2c_nbyte_read(UB dev_ch, UB ch_no, UB clk_sp_type, UB slave_addr, UB data_num, UB* w_data, UB r_data_num, UB* r_data )
{

	UCHAR* reg_icdr;
	UCHAR* reg_iccr;
	UCHAR* reg_icsr;
	UCHAR* reg_icic;
	UCHAR* reg_iccl;
	UCHAR* reg_icch;
	UCHAR* reg_ictc;

	ST_I2C_REG_SET stt_i2c_reg;
	u1 iccl_select;
	u1 icch_select;
	u1 u1t_icic = 0x00;
	u1 ch_select = 0x00;
	u1 slaveaddr_set;
	u2 u2t_i;
	u1 tmp_icsr;
	u1 u1t_ret = U1G_OK;
	int nRemData;
	int nWaitFlag = 0;
	int nReadCnt = 0;

/*********************************************************************/
/* chanel Setting                                                    */
/*********************************************************************/
	/* I2C CH REG SETTING */
	u1t_ret = u1s_i2c_reg_set( dev_ch, &stt_i2c_reg );
	if( u1t_ret != 0 )
	{
		return u1t_ret;
	}
	reg_icdr = (VU1 *)stt_i2c_reg.u4_icdr_addr;
	reg_iccr = (VU1 *)stt_i2c_reg.u4_iccr_addr;
	reg_icsr = (VU1 *)stt_i2c_reg.u4_icsr_addr;
	reg_icic = (VU1 *)stt_i2c_reg.u4_icic_addr;
	reg_iccl = (VU1 *)stt_i2c_reg.u4_iccl_addr;
	reg_icch = (VU1 *)stt_i2c_reg.u4_icch_addr;
	reg_ictc = (VU1 *)stt_i2c_reg.u4_ictc_addr;

/******************************************************************************************/
/* chanel Setting    0:100[KHz] 1:200[KHz] 2:300[KHz] 3:400[KHz]                          */
/******************************************************************************************/
	/* CLK SETTING */
	u1t_ret = u1s_i2c_clk_set( clk_sp_type, &iccl_select, &icch_select, &u1t_icic );

	if( u1t_ret )
	{
		return u1t_ret;
	}

/*********************************************************************/
/* DATA SEND SEQUENS                                                 */
/*********************************************************************/
	*reg_iccr = 0x00;										/* I2C bus reset */

	*reg_iccr |= 0x80;										/* ICCR.ICE[7] I2C is enable. */

	*reg_iccl = iccl_select;								/* Clock Speed setting */
	*reg_icch = icch_select;								/* Clock Speed setting */
	*reg_icic |= u1t_icic;									/* ICIC[7:6] Setting */

	*reg_ictc = ch_select;									/* CH is selected. */

	*reg_icic |= 0x03;										/* WAIT DTE enabling interrupt */
		
	*reg_iccr = 0x94;										/* Start Condition */
//	tmp_icsr = *reg_icsr;

	/* DTE waiting */
	u1t_ret = u1s_i2c_timeout_check( dev_ch, reg_icsr, 0x01, U2S_I2C_TIMEOUT );
	if( u1t_ret != 0 )
	{
		//monprintf("I2C_START_COND_TIMEOUT ERROR!\n");
		return u1t_ret;
	}

	*reg_icic &= ~(0x01);									/* DTE interrupt inhibit */
    slaveaddr_set = (((slave_addr<<1) + U1G_I2C_CMD_WRITE ) & 0xFF );
	*reg_icdr = slaveaddr_set;  							/* Slave address */

	/* WAIT interrupt waiting */
	u1t_ret = u1s_i2c_timeout_check( dev_ch, reg_icsr, 0x02, U2S_I2C_TIMEOUT );
	if( u1t_ret != 0 )
	{
		return u1t_ret;
	}
	vog_timer_ms_wait( 1 );

	/* The number of transmission data is acquired from the argument */
	nRemData = data_num;
/************************Begin transmit n byte data*************************///

	/* When the remainder transmission data is 1 or 0 or less , it ends (after it loops). */
	while(nRemData != 1 || nRemData <= 0){
		*reg_icdr = (unsigned char)*w_data;  				/* Write データ */
		/* WAIT, clear causes of interrupts */
		*reg_icsr &= ~(0x02);

		/* WAIT interrupt waiting */
		u1t_ret = u1s_i2c_timeout_check( dev_ch, reg_icsr, 0x02, U2S_I2C_TIMEOUT );
		if( u1t_ret != 0 )
		{
			return u1t_ret;
		}
		nRemData--;
		w_data++;
		vog_timer_ms_wait( 1 );
	}

	*reg_icdr = (unsigned char)*w_data;  					/* DATA Write(LastData) */
	*reg_iccr = 0x94;										/* ReadStart Condition */

	*reg_icic |= 0x01;										/* DTE enabling interrupt */
	
	/* WAIT, clear causes of interrupts */
	*reg_icsr &= ~(0x02);

	/* WAIT interrupt waiting */
	u1t_ret = u1s_i2c_timeout_check( dev_ch, reg_icsr, 0x02, U2S_I2C_TIMEOUT );
	if( u1t_ret != 0 )
	{
		//monprintf("I2C_READ_TIMEOUT ERROR!\n");
		return u1t_ret;
	}

	/* WAIT, clear causes of interrupts */
	*reg_icsr &= ~(0x02);

    /***********************************************************************/
    /* Data read phase                                                 */
    /***********************************************************************/

	/* DTE waiting */
	u1t_ret = u1s_i2c_timeout_check( dev_ch, reg_icsr, 0x01, U2S_I2C_TIMEOUT );
	if( u1t_ret != 0 )
	{
		//monprintf("I2C_READ_TIMEOUT ERROR!\n");
		return u1t_ret;
	}

	*reg_icic |= 0x02;										/* WAIT Enable interrupt */
	*reg_icic &= ~(0x01);									/* DTE interrupt inhibit */

    slaveaddr_set = (((slave_addr<<1) + U1G_I2C_CMD_READ ) & 0xFF ) ;
	*reg_icdr = slaveaddr_set;  							/* Slave address */

	/* WAIT interrupt waiting */
	u1t_ret = u1s_i2c_timeout_check( dev_ch, reg_icsr, 0x02, U2S_I2C_TIMEOUT );
	if( u1t_ret != 0 )
	{
		return u1t_ret;
	}

	if( r_data_num != 1 )
	{
		*reg_iccr = 0x81;										/* Switch of sending and receiving */
		/* WAIT, clear causes of interrupts */
		*reg_icsr &= ~(0x02);

		
		nReadCnt = 0;
		while( nWaitFlag == 0 ){									/* WAIT until one remain data is reading. */

			/* WAIT interrupt waiting */
			u1t_ret = u1s_i2c_timeout_check( dev_ch, reg_icsr, 0x02, U2S_I2C_TIMEOUT );
			if( u1t_ret != 0 )
			{
				//monprintf("I2C_READ_TIMEOUT ERROR!\n");
				return u1t_ret;
			}

			tmp_icsr = *reg_icsr;
			if( 0x01 == ( 0x01 & tmp_icsr)){						/* DTE=1? */
				*r_data = *reg_icdr;								/* 1byte DATA Read */
				nReadCnt++;
				r_data++;
				/* r_data_num(PacketLength+RequestedPacketLength ) */
				if( nReadCnt+1 == r_data_num ){
					/* receive remain data byte */
					*reg_iccr=0xC0;									/* Stop Condition */
					nWaitFlag = 1;
				}
			}

			*reg_icic |= 0x01;										/* DTE enabling interrupt */
			/* WAIT, clear causes of interrupts */
			*reg_icsr &= ~(0x02);

		}

		/* DTE waiting */
		u1t_ret = u1s_i2c_timeout_check( dev_ch, reg_icsr, 0x01, U2S_I2C_TIMEOUT );
		if( u1t_ret != 0 )
		{
			//monprintf("I2C_READ_TIMEOUT ERROR!\n");
			return u1t_ret;
		}

		*reg_icic &= ~(0x02);									/* WAIT interrupt inhibit */
		*r_data = *reg_icdr;							/* Defult DATA Read High Byte*/

		/* Waiting of bus open */
		u1t_ret = u1s_i2c_timeout_check( dev_ch, reg_icsr, 0xC0, U2S_I2C_TIMEOUT );
		if( u1t_ret != 0 )
		{
			return u1t_ret;
		}

		/* Clear interrupt status register */
		*reg_icsr = 0x00;
		/* Clear interrupt control register */
		*reg_icic = 0x00;
		/* ICCR.ICE(Bit7) = 0 */
		*reg_iccr &= ~(0x80);

		for( u2t_i=0; u2t_i<0x1000; u2t_i++ );									/* WAIT, because the following cannot start early */

	}
	else
	{
		*reg_iccr = 0x81;										/* Switch of sending and receiving */
		/* WAIT, clear causes of interrupts */
		*reg_icsr &= ~(0x02);

		/* WAIT interrupt waiting */
		u1t_ret = u1s_i2c_timeout_check( dev_ch, reg_icsr, 0x02, U2S_I2C_TIMEOUT );
		if( u1t_ret != 0 )
		{
			//monprintf("I2C_READ_TIMEOUT ERROR!\n");
			return u1t_ret;
		}

		*reg_icic |= 0x01;										/* DTE enabling interrupt */
		*reg_iccr=0xC0;											/* Stop Condition */
		/* WAIT, clear causes of interrupts */
		*reg_icsr &= ~(0x02);

		/* DTE waiting */
		u1t_ret = u1s_i2c_timeout_check( dev_ch, reg_icsr, 0x01, U2S_I2C_TIMEOUT );
		if( u1t_ret != 0 )
		{
			return u1t_ret;
		}

		*reg_icic &= ~(0x02);									/* WAIT interrupt inhibit */
		*r_data = *reg_icdr;									/* Defult DATA Read */

		/* Waiting of bus open */
		u1t_ret = u1s_i2c_timeout_check( dev_ch, reg_icsr, 0xC0, U2S_I2C_TIMEOUT );
		if( u1t_ret != 0 )
		{
			//monprintf("I2C_DATA_WRITE_TIMEOUT ERROR!\n");
			return u1t_ret;
		}

		/* Clear interrupt status register */
		*reg_icsr = 0x00;
		/* Clear interrupt control register */
		*reg_icic = 0x00;
		/* ICCR.ICE(Bit7) = 0 */
		*reg_iccr &= ~(0x80);
		/* WAIT, because the following cannot start early */
		for( u2t_i=0; u2t_i<0x1000; u2t_i++ );

	}
	/* Normal terminal */
	return 0;

}

/*************************************************************************************************/
/* Function name   : u1s_i2c_reg_set( u1 u1t_set_ch, ST_I2C_REG stt_i2c_reg )                           */
/* Input     : u1 u1t_set_ch, ST_I2C_REG stt_i2c_reg                                              */
/* Return   : u1                                                                                 */
/* Processing : set register address of I2C CH operation, GPIO, and CPGA                          */
/*************************************************************************************************/
static u1 u1s_i2c_reg_set( u1 u1t_set_ch, ST_I2C_REG_SET* ptt_i2c_reg )
{

	u1 u1t_ret = U1G_OK;

	switch( u1t_set_ch )
	{
		case U1G_I2C_CH0:
			U4R_SMSTPCR1 &= ~(0x00010000);	/* IIC0 ON*/ 
			/* reg_addr setting  IIC0  AP_IIC_BASE0  0xE6820000 */
			ptt_i2c_reg->u4_icdr_addr = U4S_ICDR0_ADDR;
			ptt_i2c_reg->u4_iccr_addr = U4S_ICCR0_ADDR;
			ptt_i2c_reg->u4_icsr_addr = U4S_ICSR0_ADDR;
			ptt_i2c_reg->u4_icic_addr = U4S_ICIC0_ADDR;
			ptt_i2c_reg->u4_iccl_addr = U4S_ICCL0_ADDR;
			ptt_i2c_reg->u4_icch_addr = U4S_ICCH0_ADDR;
			ptt_i2c_reg->u4_ictc_addr = U4S_ICTC0_ADDR;
			break;
		case U1G_I2C_CH1:
			/* CPGA Setting */
			U4R_SMSTPCR3 &= ~(0x00800000);	/* IIC1 ON*/
			/* reg_addr setting  IIC1  AP_IIC_BASE1  0xE6822000 */
			ptt_i2c_reg->u4_icdr_addr = U4S_ICDR1_ADDR;
			ptt_i2c_reg->u4_iccr_addr = U4S_ICCR1_ADDR;
			ptt_i2c_reg->u4_icsr_addr = U4S_ICSR1_ADDR;
			ptt_i2c_reg->u4_icic_addr = U4S_ICIC1_ADDR;
			ptt_i2c_reg->u4_iccl_addr = U4S_ICCL1_ADDR;
			ptt_i2c_reg->u4_icch_addr = U4S_ICCH1_ADDR;
			ptt_i2c_reg->u4_ictc_addr = U4S_ICTC1_ADDR;
			break;
		case U1G_I2C_CH2:
			/* CPGA Setting */
			U4R_SMSTPCR0 &= ~(0x00000002);	/* IIC2 ON*/
			/* reg_addr setting  IIC2  AP_IIC_BASE2  0xE6824000 */
			ptt_i2c_reg->u4_icdr_addr = U4S_ICDR2_ADDR;
			ptt_i2c_reg->u4_iccr_addr = U4S_ICCR2_ADDR;
			ptt_i2c_reg->u4_icsr_addr = U4S_ICSR2_ADDR;
			ptt_i2c_reg->u4_icic_addr = U4S_ICIC2_ADDR;
			ptt_i2c_reg->u4_iccl_addr = U4S_ICCL2_ADDR;
			ptt_i2c_reg->u4_icch_addr = U4S_ICCH2_ADDR;
			ptt_i2c_reg->u4_ictc_addr = U4S_ICTC2_ADDR;
			break;
		case U1G_I2C_CH3:
			/* CPGA Setting IIC3 Default ON */
			U4R_SMSTPCR4 &= ~(0x00000800);	/* IIC3 ON*/
			/* reg_addr setting  IIC3  AP_IIC_BASE3  0xE6826000 */
			ptt_i2c_reg->u4_icdr_addr = U4S_ICDR3_ADDR;
			ptt_i2c_reg->u4_iccr_addr = U4S_ICCR3_ADDR;
			ptt_i2c_reg->u4_icsr_addr = U4S_ICSR3_ADDR;
			ptt_i2c_reg->u4_icic_addr = U4S_ICIC3_ADDR;
			ptt_i2c_reg->u4_iccl_addr = U4S_ICCL3_ADDR;
			ptt_i2c_reg->u4_icch_addr = U4S_ICCH3_ADDR;
			ptt_i2c_reg->u4_ictc_addr = U4S_ICTC3_ADDR;
			break;
		case U1G_I2C_CH0H:
			/* CPGA Setting IIC0H Default ON */
			U4R_SMSTPCR4 &= ~(0x01000000);	/* IIC0H ON*/
			/* reg_addr setting  IIC0H  IIC0H_BADR  0xE6828000 GPIO_FUNC2 */
			vog_port_st_set( U2S_I2C_SCL0H_PORT, U1S_I2C0H_PORT_SET );
			vog_port_st_set( U2S_I2C_SDA0H_PORT, U1S_I2C0H_PORT_SET );
			ptt_i2c_reg->u4_icdr_addr = U4S_ICDR0H_ADDR;
			ptt_i2c_reg->u4_iccr_addr = U4S_ICCR0H_ADDR;
			ptt_i2c_reg->u4_icsr_addr = U4S_ICSR0H_ADDR;
			ptt_i2c_reg->u4_icic_addr = U4S_ICIC0H_ADDR;
			ptt_i2c_reg->u4_iccl_addr = U4S_ICCL0H_ADDR;
			ptt_i2c_reg->u4_icch_addr = U4S_ICCH0H_ADDR;
			ptt_i2c_reg->u4_ictc_addr = U4S_ICTC0H_ADDR;
			break;
		case U1G_I2C_CH1H:
			/* CPGA Setting IIC1H Default ON */
			U4R_SMSTPCR4 &= ~(0x02000000);	/* IIC1H ON*/
			/* reg_addr setting  IIC1H  IIC1H_BADR  0xE682A000 GPIO_FUNC2 */
			vog_port_st_set( U2S_I2C_SCL1H_PORT, U1S_I2C1H_PORT_SET );
			vog_port_st_set( U2S_I2C_SDA1H_PORT, U1S_I2C1H_PORT_SET );
			ptt_i2c_reg->u4_icdr_addr = U4S_ICDR1H_ADDR;
			ptt_i2c_reg->u4_iccr_addr = U4S_ICCR1H_ADDR;
			ptt_i2c_reg->u4_icsr_addr = U4S_ICSR1H_ADDR;
			ptt_i2c_reg->u4_icic_addr = U4S_ICIC1H_ADDR;
			ptt_i2c_reg->u4_iccl_addr = U4S_ICCL1H_ADDR;
			ptt_i2c_reg->u4_icch_addr = U4S_ICCH1H_ADDR;
			ptt_i2c_reg->u4_ictc_addr = U4S_ICTC1H_ADDR;
			break;
		case U1G_I2C_CH2H:
			/* CPGA Setting IIC2H Default ON */
			U4R_SMSTPCR4 &= ~(0x04000000);	/* IIC2H ON*/
			/* reg_addr setting  IIC2H  IIC2H_BADR  0xE682C000 GPIO_FUNC2 */
			vog_port_st_set( U2S_I2C_SCL2H_PORT, U1S_I2C2H_PORT_SET );
			vog_port_st_set( U2S_I2C_SDA2H_PORT, U1S_I2C2H_PORT_SET );
			ptt_i2c_reg->u4_icdr_addr = U4S_ICDR2H_ADDR;
			ptt_i2c_reg->u4_iccr_addr = U4S_ICCR2H_ADDR;
			ptt_i2c_reg->u4_icsr_addr = U4S_ICSR2H_ADDR;
			ptt_i2c_reg->u4_icic_addr = U4S_ICIC2H_ADDR;
			ptt_i2c_reg->u4_iccl_addr = U4S_ICCL2H_ADDR;
			ptt_i2c_reg->u4_icch_addr = U4S_ICCH2H_ADDR;
			ptt_i2c_reg->u4_ictc_addr = U4S_ICTC2H_ADDR;
			break;
		case U1G_I2C_CH3H:
			/* CPGA Setting IIC3H Default ON */
			U4R_SMSTPCR4 &= ~(0x08000000);	/* IIC3H ON*/
			/* reg_addr setting  IIC3H  IIC3H_BADR  0xE682E000 GPIO_FUNC2 */
			vog_port_st_set( U2S_I2C_SCL3H_PORT, U1S_I2C3H_PORT_SET );
			vog_port_st_set( U2S_I2C_SDA3H_PORT, U1S_I2C3H_PORT_SET );
			ptt_i2c_reg->u4_icdr_addr = U4S_ICDR3H_ADDR;
			ptt_i2c_reg->u4_iccr_addr = U4S_ICCR3H_ADDR;
			ptt_i2c_reg->u4_icsr_addr = U4S_ICSR3H_ADDR;
			ptt_i2c_reg->u4_icic_addr = U4S_ICIC3H_ADDR;
			ptt_i2c_reg->u4_iccl_addr = U4S_ICCL3H_ADDR;
			ptt_i2c_reg->u4_icch_addr = U4S_ICCH3H_ADDR;
			ptt_i2c_reg->u4_ictc_addr = U4S_ICTC3H_ADDR;
			break;
		default:
			u1t_ret = U1G_NG;
			break;
	}

	return u1t_ret;

}

/*************************************************************************************************/
/* Function name   : u1s_i2c_clk_set                                                                    */
/* Input     : u1 u1t_set_clk, u1* ptt_iccl, u1* ptt_iccl, u1* ptt_icic                           */
/* Return   : u1                                                                                 */
/* Processing : Set (ICCH,ICCL) for CLK when I2C CH operates                                       */
/*************************************************************************************************/
static u1 u1s_i2c_clk_set( u1 u1t_set_clk, u1* ptt_iccl, u1* ptt_icch, u1* ptt_icic )
{

	u1 u1t_ret = U1G_OK;
	*ptt_icic = 0x00;

	/**************************************************************************/
	/* clk Setting    0:100[KHz] 3:400[KHz]                                   */
	/**************************************************************************/
	switch( u1t_set_clk )
	{
		case U1G_ESCL100K:
			/* ICCL and ICCH values for 100 KHz(104MHZ) */
			*ptt_iccl = 0x20;
			*ptt_icch = 0xE5;
			/* ICIC[7] = 1 */
			*ptt_icic = 0x80;
			break;

		case U1G_ESCL400K:
			/* ICCL and ICCH values for 400 KHz(104MHZ) */
			*ptt_iccl = 0x48;

			*ptt_icch = 0x37;
			break;

		default:
			u1t_ret = U1G_NG;
			break;
	}

	return u1t_ret;
}

/*************************************************************************************************/
/* FUNCTION : u1s_i2c_timeout_check                                                              */
/* ARGUMENT : u1 u1t_i2c_dev_ch, VU1* ptt_reg_data, u1 u1t_check_data, u2 u2t_timeout_set        */
/* RETURN   : u1(normal:0, timeout:0xff)                                                         */
/* OUTLINE  : i2c timeout check                                                                  */
/*************************************************************************************************/
static u1 u1s_i2c_timeout_check( u1 u1t_i2c_dev_ch, VU1* ptt_reg_data, u1 u1t_check_data, u2 u2t_timeout_set )
{

	u1 u1t_ret;
	u1 u1t_tmp_reg_data;
	u2 u2t_time_out_cnt;

	u1t_ret = U1G_ZERO;
	u2t_time_out_cnt = U2G_ZERO;

	u1t_tmp_reg_data = *ptt_reg_data;

	while( u1t_check_data != ( u1t_check_data & u1t_tmp_reg_data ) )		/* DTE waiting */
	{
		u1t_tmp_reg_data = *ptt_reg_data;
		/* ACK, WAIT, DTE waiting */
		vog_timer_us_wait( 1 );
		u2t_time_out_cnt++;
		/* When WAIT ms or more, it is assumed TimeOut. */
		if( u2t_time_out_cnt >= u2t_timeout_set )
		{
			vos_i2c_reset( u1t_i2c_dev_ch );
			u1t_ret = 0xFF;
			break;
		}
	}

	return u1t_ret;

}

/*************************************************************************************************/
/* Function name   : vog_i2c_Reset_Ctrl( u1 u1t_dev_ch )                                         */
/* Input     : u1 dev_ch(0:I2C_RT, 1:I2C_SYS, 2:I2C_RT2)                                         */
/* Return   : void                                                                               */
/* Processing : Reset control of I2C 						                                     */
/*************************************************************************************************/
static void vos_i2c_reset( u1 u1t_i2c_ch_no )
{

	switch( u1t_i2c_ch_no )
	{
		case U1G_I2C_CH0:
			/* Reset IIC0  SRCR1:bit16 */
			U4R_SRCR1 |= 0x00010000;
			/* Reset issue completion waiting */
			while( ( U4R_SRCR1 & 0x00010000 ) != 0x00010000 );
			/* 50us Wait (RCLK1 clock) */
			vog_timer_us_wait( 50 );
			/* Reset IIC0 */
			U4R_SRCR1 &= ~(0x00010000);
			/* Reset release completion waiting */
			while( ( U4R_SRCR1 & 0x00010000 ) == 0x00010000 );
			break;

		case U1G_I2C_CH1:
			/* Reset IIC1 SRCR3:bit23 */
			U4R_SRCR3 |= 0x00800000;
			/* Reset issue completion waiting */
			while( ( U4R_SRCR3 & 0x00800000 ) != 0x00800000 );
			/* 50us Wait (RCLK1 clock) */
			vog_timer_us_wait( 50 );
			/* Reset IIC1 */
			U4R_SRCR3 &= ~(0x00800000);
			/* Reset release completion waiting */
			while( ( U4R_SRCR3 & 0x00800000 ) == 0x00800000 );
			break;

		case U1G_I2C_CH2:
			/* Reset IIC2 SRCR0:bit1 */
			U4R_SRCR0 |= 0x00000002;
			/* Reset issue completion waiting */
			while( ( U4R_SRCR0 & 0x00000002 ) != 0x00000002 );
			/* 50us Wait (RCLK1 clock) */
			vog_timer_us_wait( 50 );
			/* Reset IIC2*/
			U4R_SRCR0 &= ~(0x00000002);
			/* Reset release completion waiting */
			while( ( U4R_SRCR0 & 0x00000002) == 0x00000002 );
			break;

		case U1G_I2C_CH3:
			/* Reset IIC3 SRCR4:bit11 */
			U4R_SRCR4 |= 0x00000800;
			/* Reset issue completion waiting */
			while( ( U4R_SRCR4 & 0x00000800 ) != 0x00000800 );
			/* 50us Wait (RCLK1 clock) */
			vog_timer_us_wait( 50 );
			/* Reset IIC3 */
			U4R_SRCR4 &= ~(0x00000800);
			/* Reset release completion waiting */
			while( ( U4R_SRCR4 & 0x00000800) == 0x00000800 );
			break;

		default:
			break;
	}
	return;
}

