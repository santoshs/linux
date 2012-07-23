/* i2c.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef __I2C_H__
#define __I2C_H__

#include "com_type.h"

/* I2C setting value */
#define REG_8BIT_CLEAR			(0x00)	/* Bit Clear */
#define ICSR_DATA_DTE			(0x01)	/* ICSR - DTE(0 Bit) */
#define ICSR_DATA_WAIT			(0x02)	/* ICSR - WAIT(1 Bit) */
#define ICSR_DATA_BUSY			(0x10)	/* ICSR - BUSY(4 Bit) */
#define ICSR_DATA_SCLM_SDAM		(0xC0)	/* ICSR - SCLM,SDAM(7-6 Bit) */
#define ICIC_DATA_DTEE			(0x01)	/* ICIC - DTEE(0 Bit) */
#define ICIC_DATA_WAITE			(0x02)	/* ICIC - WAITE(1 Bit) */
#define ICIC_DATA_WAITE_DTEE	(0x03)	/* ICIC - WAITE,DTEE(1,0 Bit) */
#define ICCR_DATA_ICE			(0x80)	/* ICCR - ICE(7 Bit) */
#define ICCR_DATA_ICE_SCP		(0x81)	/* ICCR - ICE,SCP(7,0 Bit) */
#define ICCR_DATA_ICE_TRS		(0x90)	/* ICCR - ICE,TRS(7,4 Bit) */
#define ICCR_DATA_ICE_TRS_BBSY	(0x94)	/* ICCR - ICE,TRS,BBSY(7,4,2 Bit) */
#define ICCR_DATA_ICE_RACK		(0xC0)	/* ICCR - ICE,RACK(7,6 Bit) */
#define ICCL_DATA_52MHZ			(0x24)	/* ICCL value when HP clock=52MHz */
#define ICCH_DATA_52MHZ			(0x1D)	/* ICCH value when HP clock=52MHz */
#define ICCL_DATA_104MHZ		(0x48)	/* ICCL value when HP clock=104MHz */
#define ICCH_DATA_104MHZ		(0x3A)	/* ICCH value when HP clock=104MHz */
#define ICCL_DATA_104MHZ_HS		(0x09)	/* ICCL value when HP clock=104MHz */
#define ICCH_DATA_104MHZ_HS		(0x04)	/* ICCH value when HP clock=104MHz */

#define GPIO_SELECT_FUNCTION2	(0x12)	/* PORTnCR - OE,PTMD(4,2 Bit) */
#define GPIO_SELECT_FUNCTION5	(0x15)	/* PORTnCR - OE,PTMD(4,2,0 Bit) */

/* Data transfer direction setting value */
#define I2C_CMD_READ			(1)		/* Data demand (reading) */
#define I2C_CMD_WRITE			(0)		/* Transmission (writing) */

/* define for ICSR reference */
#define ICSR_TIMEOUT_COUNT		(10000)		/* maximum reference number of times */
#define I2C_RW_END_WAIT_LOOP	(0x1)		/* wait in i2c_1byte_write/i2c_1byte_read end */

/* Register setting value */
typedef struct {
	volatile uchar * ICDR;
	volatile uchar * ICCR;
	volatile uchar * ICSR;
	volatile uchar * ICIC;
	volatile uchar * ICCL;
	volatile uchar * ICCH;
	volatile uchar * ICASTART;
} I2C_REG;

/* HP clock */
typedef enum {
    I2C_HPCLK_52MHZ = 0,        /* HP clock 52MHz */
    I2C_HPCLK_104MHZ,           /* HP clock 104MHz */
    I2C_HPCLK_26MHZ				/* HP clock 26MHz */
} I2C_HPCLK;

/* CH */
typedef enum {
    I2C_CH_IIC0 = 0,            /* IIC0 ch */
    I2C_CH_IIC1,                /* IIC1 ch */
    I2C_CH_IIC2,                /* IIC2 ch */
    I2C_CH_IIC3,                /* IIC3 ch */
    I2C_CH_IIC4,                 /* IIC4 ch */
	I2C_CH_IIC5,                 /* IIC5 ch */
	I2C_CH_IICB                 /* IICB ch */
} I2C_CH;


/* Error Code */
#define I2C_SUCCESS     (0)     /* Successful */
#define I2C_ERR_START   (-1)    /* I2C transmission start error */
#define I2C_ERR_TIMEOUT (-2)    /* I2C timeout error */


/* I2C(IIC0) register address */
#define IIC0_BASE	(0xE6820000ul)							/* I2C bus interface(IIC0) */
#define ICDR0	(volatile uchar *)(IIC0_BASE + 0x0000)		/* I2C bus dara register 0 */
#define ICCR0	(volatile uchar *)(IIC0_BASE + 0x0004)		/* I2C bus control register 0 */
#define ICSR0	(volatile uchar *)(IIC0_BASE + 0x0008)		/* I2C bus status register 0 */
#define ICIC0	(volatile uchar *)(IIC0_BASE + 0x000C)		/* I2C interruption control register 0 */
#define ICCL0	(volatile uchar *)(IIC0_BASE + 0x0010)		/* I2C clock control register low 0 */
#define ICCH0	(volatile uchar *)(IIC0_BASE + 0x0014)		/* I2C clock control register high 0 */
#define ICASTART0	(volatile uchar *)(IIC0_BASE + 0x0070)		/* I2C automatic transmission transformer start register 0 */



/* Function Prototypes */
void I2C_Init(void);
void I2C_Set_Hp(I2C_HPCLK hpclk);
void I2C_Set_Ch(I2C_CH ch);
RC I2C_Write(uchar slave_addr, uchar register_addr, uchar w_data);
RC I2C_Read(uchar slave_addr, uchar register_addr, uchar* r_data);

#endif /* __I2C_H__ */
