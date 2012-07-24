/*
 * i2c_drv.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */
#ifndef _H_I2C_DRV
#define _H_I2C_DRV

/***************************************************************/
/* INCLUDE FILE                                                */
/***************************************************************/

/***************************************************************/
/* PUBLIC CONSTANT DEFINITON                                   */
/***************************************************************/
/*-------------------------------------------------------------*/
/* INDEX_IIC_SLAVE_ADDR(7bit Address LSB is R/W)               */
/*-------------------------------------------------------------*/
#define U1G_SLADR_PALSI4_PWR	(u1)0x29	/* B'0010 1001(R/W) */
#define U1G_SLADR_PALSI4_RTC	(u1)0x32	/* B'0011 0010(R/W) */
#define U1G_SLADR_PALSI4_AUD	(u1)0x63	/* B'0110 0011(R/W) */
#define U1G_SLADR_PALSI4_DVFS	(u1)0x12	/* B'0001 0010(R/W) */
#define U1G_SLADR_FUEL_GAUGE	(u1)0x55	/* B'0101 0101(R/W) */
/*  LEDCNT  */
#define U1G_SLADR_LEDCNT		(u1)0x6D	/* B'0110 1101(R/W) */
#define U1G_SLADR_LV5216CS		(u1)0x74	/* B'0111 0100(R/W) */
/* OUTCAMERA */
#define U1G_SLADR_OUTCAMERA		(u1)0x36	/* B'0110 110(R/W)(EOS-EVM) */
/* INCAMERA */
#define U1G_SLADR_INCAMERA		(u1)0x20	/* B'0010 0000(R/W) DEVID0=0 */
#define U1G_SLADR_KODOH4_PWR	(u1)0x29	/* B'0101 001(R/W) */
#define U1G_SLADR_KODOH4_RTC	(u1)0x32	/* B'0110 010(R/W) */
#define U1G_SLADR_KODOH4_AUD	(u1)0x63	/* B'1100 011(R/W) */
#define U1G_SLADR_KODOH4_DVFS	(u1)0x12	/* B'0010 010(R/W) */

#define U1G_SLADR_ACCELEROMETER	(u1)0x19	/* B'0001 100(R/W) : ADDR=H */
#define U1G_SLADR_AMBIENT_LIGHT	(u1)0x64	/* B'1100 100(R/W) */
#define U1G_SLADR_EL_COMPASS	(u1)0x0C	/* B'0011 001(R/W) : ADR1/ADR0=0/0 */

#define U1G_SLADR_PMIC_ID0		(u1)0x12	/* B'0010 010(R/W) DVS-I2C */
#define U1G_SLADR_PMIC_ID1		(u1)0x48	/* B'1001 000(R/W) */
#define U1G_SLADR_PMIC_ID2		(u1)0x49	/* B'1001 001(R/W) */
#define U1G_SLADR_PMIC_ID3		(u1)0x4A	/* B'1001 010(R/W) */

/*-------------------------------------------------------------*/

#define U1G_I2C_CMD_WRITE	0x00
#define U1G_I2C_CMD_READ	0x01

#define U1G_I2C_CH0			0x00
#define U1G_I2C_CH1			0x01
#define U1G_I2C_CH2			0x02
#define U1G_I2C_CH3			0x03
#define U1G_I2C_CH0H		0x08
#define U1G_I2C_CH1H		0x09
#define U1G_I2C_CH2H		0x0A
#define U1G_I2C_CH3H		0x0B
#define U1G_I2CM			0x10

#define U1G_I2C_MAX_ADDR	8
#define U1G_I2C_MAX_DATA	32
#define U1G_I2C_DEV_CH_NUM	4

/* 1 byte simple access */
#define U1G_I2C_RT			0x0
#define U1G_I2C_SYS			0x1

/***************************************************************/
/* PUBLIC TYPEDEFE                                             */
/***************************************************************/
enum{
	U1G_ESCL100K = 0x00,
	U1G_ESCL200K,
	U1G_ESCL300K,
	U1G_ESCL400K,
	U1G_ESCL3_4M,

	U1G_CLK_SET_NUM,
};

typedef struct {
	u1  u1_ch_no;					/* 0:CH0 1:CH1 2:CH2 3:CH3 4:CH4 */
	u1  u1_clk_type;				/* 0:100[KHz] 1:200[KHz] 2:300[KHz] 3:400[KHz] */
	u1  u1_slave_addr;				/* I2C slave address */
	u1* pt_snd_data;				/* Send data */
	u1* pt_rcv_data;				/* Receive data*/
	u1  u1_snd_num;					/* Send number */
	u1  u1_rcv_num;					/* Receive number */
	u1* pt_rcv_snd_data;			/* Writing data of the previous state of reading */
	u1  u1_rcv_snd_num;				/* Number of writing data at the previous state of reading */
	u1* pt_ret;						/* I/O result:  0: Normal terminal 1: Abnormal terminal */
} ST_I2C_COM;

/***************************************************************/
/* PUBLIC VARIABLE EXTERN                                      */
/***************************************************************/

/***************************************************************/
/* PUBLIC FUNCTION EXTERN                                      */
/***************************************************************/
extern void vog_i2c_com_init( ST_I2C_COM* ptt_i2c_com );
extern void vog_i2c_snd( ST_I2C_COM stt_i2c_com );
extern void vog_i2c_rcv( ST_I2C_COM stt_i2c_com );
extern IB i2c_1byte_write( u1 dev_ch, u1 ch_no, u1 clk_sp_type, u1 slave_addr, u1 sub_addr, u1  w_data );
extern IB i2c_1byte_read ( u1 dev_ch, u1 ch_no, u1 clk_sp_type, u1 slave_addr, u1 sub_addr, u1* r_data );
extern IB i2c_nbyte_write( u1 dev_ch, u1 ch_no, u1 clk_sp_type, u1 slave_addr, u1 data_num, u1* w_data );
extern IB i2c_nbyte_read ( u1 dev_ch, u1 ch_no, u1 clk_sp_type, u1 slave_addr, u1 data_num, u1* w_data, u1 r_data_num, u1* r_data );

#endif /* _H_I2C_DRV */
