/* i2cm.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef __I2CM_H__
#define __I2CM_H__

#include "com_type.h"


/* Error Code */
#define I2CM_SUCCESS              (0)     /* Successful */
#define I2CM_ERR_START            (-1)    /* I2CM transmission start error */
#define I2CM_ERR_TIMEOUT          (-2)    /* I2CM timeout error */
#define I2CM_ERR_NACK             (-3)    /* I2CM NACK error */
#define I2CM_ERR_PARAM            (-4)    /* I2CM parameter error */
#define I2CM_ERR_ARBITRATION_LOST (-5)    /* I2CM arbitration lost error */


/* I2CM register address */
#define I2CM_BASE	(0xE6D20000ul)								/* I2CM bus interface */
#define I2CM_ICCR1		(volatile uchar*)(I2CM_BASE + 0x0000)	/* I2C bus control register 1 */
#define I2CM_ICCR2		(volatile uchar*)(I2CM_BASE + 0x0001)	/* I2C bus control register 2 */
#define I2CM_ICMR		(volatile uchar*)(I2CM_BASE + 0x0002)	/* I2C bus mode register */
#define I2CM_ICIER		(volatile uchar*)(I2CM_BASE + 0x0003)	/* I2C bus interrupt enable register */
#define I2CM_ICSR		(volatile uchar*)(I2CM_BASE + 0x0004)	/* I2C bus status register */
#define I2CM_SAR		(volatile uchar*)(I2CM_BASE + 0x0005)	/* Slave address register */
#define I2CM_ICDRT		(volatile uchar*)(I2CM_BASE + 0x0006)	/* I2C bus transmit data register */
#define I2CM_ICDRR		(volatile uchar*)(I2CM_BASE + 0x0007)	/* I2C bus receive data register */
#define I2CM_NF2CYC		(volatile uchar*)(I2CM_BASE + 0x0008)	/* NF2CYC register */


/* Function Prototypes */
void I2CM_Init(void);
RC I2CM_Write(uchar slave_addr, uchar register_addr, uchar *w_data, RC w_num);
RC I2CM_Read(uchar slave_addr, uchar register_addr, uchar *r_data, RC r_num);

#endif /* __I2C_H__ */
