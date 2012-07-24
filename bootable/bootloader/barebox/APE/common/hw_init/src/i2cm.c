/*	i2cm.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */


#include "i2cm.h"
#include "gpio.h"
#include "cpg.h"
#include "tmu_api.h"
#include "log_output.h"



/* I2CM setting value */
#define ICCR1_DATA_ICE			(0x80)	/* ICCR1 - ICE  (7 Bit) */
#define ICCR1_DATA_RCVD			(0x40)	/* ICCR1 - RCVD (6 Bit) */
#define ICCR1_DATA_MST			(0x20)	/* ICCR1 - MST  (5 Bit) */
#define ICCR1_DATA_TRS			(0x10)	/* ICCR1 - TRS  (4 Bit) */
#define ICCR2_DATA_BBSY			(0x80)	/* ICCR2 - BBSY (7 Bit) */
#define ICCR2_DATA_SCP			(0x40)	/* ICCR2 - BBSY (6 Bit) */
#define ICIER_DATA_TIE			(0x80)	/* ICIER - TIE  (7 Bit) */
#define ICIER_DATA_TEIE			(0x40)	/* ICIER - TEIE (6 Bit) */
#define ICIER_DATA_RIE			(0x20)	/* ICIER - RIE  (5 Bit) */
#define ICIER_DATA_STIE			(0x08)	/* ICIER - STIE (3 Bit) */
#define ICIER_DATA_ACKBR		(0x02)	/* ICIER - ACKBR(1 Bit) */
#define ICIER_DATA_ACKBT		(0x01)	/* ICIER - ACKBT(0 Bit) */
#define ICSR_DATA_TDRE			(0x80)	/* ICSR  - TDRE (7 Bit) */
#define ICSR_DATA_TEND			(0x40)	/* ICSR  - TEND (6 Bit) */
#define ICSR_DATA_RDRF			(0x20)	/* ICSR  - RDRF (5 Bit) */
#define ICSR_DATA_STOP			(0x08)	/* ICSR  - STOP (3 Bit) */
#define ICSR_DATA_ALOVE			(0x04)	/* ICSR  - STOP (2 Bit)  */

#define ICCR1_DATA_CKS_400KHZ	(0x04)	/* ICCR1 - CKS  (3-0 Bit) 400kHz */


/* Data transfer direction setting value */
#define I2CM_CMD_READ			(1)		/* Data demand (reading) */
#define I2CM_CMD_WRITE			(0)		/* Transmission (writing) */


/* define for ICSR reference */
#define ICSR_WAIT_LOOP_10US		(13)		/* 10us wait */
#define ICSR_TIMEOUT_COUNT		(10000)		/* maximum reference number of times */


/* Register setting value */
typedef struct {
	volatile uchar* ICCR1;
	volatile uchar* ICCR2;
	volatile uchar* ICMR;
	volatile uchar* ICIER;
	volatile uchar* ICSR;
	volatile uchar* SAR;
	volatile uchar* ICDRT;
	volatile uchar* ICDRR;
	volatile uchar* NF2CYC;
} I2CM_REG;


/* Internal function Prototypes */
static RC I2CM_PrepareTransfer(I2CM_REG *reg);
static RC I2CM_StartTransfer(I2CM_REG *reg, uchar slave_addr, uchar rw);
static RC I2CM_SendData(I2CM_REG *reg, uchar w_data);
static RC I2CM_ReceiveData(I2CM_REG *reg, RC r_num, uchar * r_data);
static void I2CM_StopTransfer(I2CM_REG *reg);
static RC I2CM_Wait(I2CM_REG *reg, uchar check_bit);


/**
 * I2CM_Init - Initialize I2CM module
 * @return None
 */
void I2CM_Init(void)
{
	uchar val;

    /* I2CM module initialize */
	*CPG_SMSTPCR4 &= ~CPG_SMSTPCR4_412;
	*CPG_SRCR4    &= ~CPG_SRCR4_SRT412;

	/* I2CM register initialize */
	*(I2CM_ICCR1)  = 0x00;
	*(I2CM_ICCR2)  = 0x7D;
	*(I2CM_ICMR)   = 0x38;
	*(I2CM_ICIER)  = 0x00;
	*(I2CM_ICSR)   = 0x00;
	*(I2CM_SAR)    = 0x00;
	*(I2CM_ICDRT)  = 0xFF;
	*(I2CM_ICDRR)  = 0xFF;
	*(I2CM_NF2CYC) = 0x00;

	val = *(I2CM_ICCR1);
	val |= (ICCR1_DATA_ICE | 		/* I2CM bus interface enable */
		    ICCR1_DATA_CKS_400KHZ);
	*(I2CM_ICCR1) = val;

	return ;
}

/**
 * I2CM_Write - I2CM plural byte write
 * @return I2CM_SUCCESS     : Successful
 *         I2CM_ERR_PARAM   : paramter error
 *         Others
 */
RC I2CM_Write(uchar slave_addr, uchar register_addr, uchar *w_data, RC w_num)
{
	RC ret;
	RC i;
	I2CM_REG reg;
	
	if ((w_data == NULL) ||
	    (w_num  == 0)      )
	{
		PRINTF("FAIL I2CM Write - parameter error\n");
		return I2CM_ERR_PARAM;
	}

	/* Prepare process */
	ret = I2CM_PrepareTransfer(&reg);
	if (ret != I2CM_SUCCESS)
	{
		PRINTF("FAIL I2CM Write - I2CM_PrepareTransfer()=%d\n", ret);
		return ret;
	}

	/* Register access start */
	ret = I2CM_StartTransfer(&reg, slave_addr, I2CM_CMD_WRITE);
	if (ret != I2CM_SUCCESS)
	{
		PRINTF("FAIL I2CM Write - I2CM_StartTransfer()=%d\n", ret);
		return ret;
	}

	/* transmission of the register address */
	ret = I2CM_SendData(&reg, register_addr);
	if (ret != I2CM_SUCCESS)
	{
		PRINTF("FAIL I2CM Write - I2CM_SendData()=%d register addr send\n", ret);
		return ret;
	}

	/* transmission of the register data */
	for (i = 0;i < w_num;i++)
	{
		ret = I2CM_SendData(&reg, w_data[i]);
		if (ret != I2CM_SUCCESS)
		{
			PRINTF("FAIL I2CM Write - I2CM_SendData()=%d data send\n", ret);
			return ret;
		}
	}

	/* Register access stop */
	I2CM_StopTransfer(&reg);

	return I2CM_SUCCESS;
}


/**
 * I2CM_Read - I2CM plural byte read
 * @return I2CM_SUCCESS     : Successful
 *         I2CM_ERR_PARAM  : paramter error
 *         Others
 */
RC I2CM_Read(uchar slave_addr, uchar register_addr, uchar *r_data, RC r_num)
{
	RC ret;
	I2CM_REG reg;

	if ((r_data == NULL) ||
	    (r_num  == 0)      )
	{
		PRINTF("FAIL I2CM Read - parameter error\n");
		return I2CM_ERR_PARAM;
	}

	/* Prepare process */
	ret = I2CM_PrepareTransfer(&reg);
	if (ret != I2CM_SUCCESS)
	{
		PRINTF("FAIL I2CM Read - I2CM_PrepareTransfer()=%d\n", ret);
		return ret;
	}

	/* register access start */
	ret = I2CM_StartTransfer(&reg, slave_addr, I2CM_CMD_WRITE);
	if (ret != I2CM_SUCCESS)
	{
		PRINTF("FAIL I2CM Read - I2CM_StartTransfer()=%d\n", ret);
		return ret;
	}

	/* transmission of the register address */
	ret = I2CM_SendData(&reg, register_addr);
	if (ret != I2CM_SUCCESS)
	{
		PRINTF("FAIL I2CM Read - I2CM_SendData()=%d\n", ret);
		return ret;
	}

	/* register access restart */
	ret = I2CM_StartTransfer(&reg, slave_addr, I2CM_CMD_READ);
	if (ret != I2CM_SUCCESS)
	{
		PRINTF("FAIL I2CM Read - I2CM_StartTransfer()=%d\n", ret);
		return ret;
	}

	/* receive of the register data */
	ret = I2CM_ReceiveData(&reg, r_num, r_data);
	if (ret != I2CM_SUCCESS)
	{
		PRINTF("FAIL I2CM Read - I2CM_ReceiveData()=%d\n", ret);
		return ret;
	}

	return I2CM_SUCCESS;
}


/**
 * I2CM_PrepareTransfer - Prepare process(internal)
 * @return I2CM_SUCCESS      : Successful
 *         I2CM_ERR_START    : Initialize error
 */
RC I2CM_PrepareTransfer(I2CM_REG *reg)
{
	uchar val;

	/* check SMSTPCR4 MSTP412 (Controls clock supply to IICM) */
	if ((*CPG_SMSTPCR4 & CPG_SMSTPCR4_412) != 0)
	{
		PRINTF("FAIL I2CM PrepareTransfer - SMSTPCR4(0x%08x) check\n", *CPG_SMSTPCR4);
		return I2CM_ERR_START;
	}
	/* check SRCR4 SRT412 (Issues the reset to IICM) */
	if ((*CPG_SRCR4 & CPG_SRCR4_SRT412) != 0)
	{
		PRINTF("FAIL I2CM PrepareTransfer - SRCR4(0x%08x) check\n", *CPG_SRCR4);
		return I2CM_ERR_START;
	}

	/* I2CM register setting for IICM */
	reg->ICCR1  = I2CM_ICCR1;
	reg->ICCR2  = I2CM_ICCR2;
	reg->ICMR   = I2CM_ICMR;
	reg->ICIER  = I2CM_ICIER;
	reg->ICSR   = I2CM_ICSR;
	reg->SAR    = I2CM_SAR;
	reg->ICDRT  = I2CM_ICDRT;
	reg->ICDRR  = I2CM_ICDRR;
	reg->NF2CYC = I2CM_NF2CYC;

	/* check Bus Busy */
	val = *(reg->ICCR2);
	if ((val & ICCR2_DATA_BBSY) != 0)
	{
		PRINTF("FAIL I2CM PrepareTransfer - Bus Busy(0x%02x) check\n", val);
		return I2CM_ERR_START;
	}

	/* set master transmit mode */
	val = *(reg->ICCR1);
	val |= (ICCR1_DATA_MST | ICCR1_DATA_TRS);
	*(reg->ICCR1) = val;

	return I2CM_SUCCESS;
}


/**
 * I2CM_StartTransfer - Register access start(internal)
 * @return I2CM_SUCCESS     : Successful
 *         I2CM_ERR_NACK   : NACK error
 *         Others
 */
RC I2CM_StartTransfer(I2CM_REG *reg, uchar slave_addr, uchar rw)
{
	RC ret;
	uchar val;

	/* issue the start condition */
	val = *(reg->ICCR2);
	val |= ICCR2_DATA_BBSY;
	val &= ~(ICCR2_DATA_SCP);
	*(reg->ICCR2) = val;

	/* wait for setting TDRE */
	ret = I2CM_Wait(reg, ICSR_DATA_TDRE);
	if (ret != I2CM_SUCCESS)
	{
		PRINTF("FAIL I2CM StartTransfer - wait TDRE error=%d\n", ret);
		return ret;
	}
	
	/* set slave address */
	*(reg->ICDRT) = (((slave_addr << 1) | rw) & 0xFF);

	/* wait for setting TEND */
	ret = I2CM_Wait(reg, ICSR_DATA_TEND);
	if (ret != I2CM_SUCCESS)
	{
		PRINTF("FAIL I2CM StartTransfer - wait TEND error=%d\n", ret);
		return ret;
	}
	
	/* check Receive Acknowledge */
	val = *(reg->ICIER);
	if ((val & ICIER_DATA_ACKBR) != 0)
	{
		PRINTF("FAIL I2CM StartTransfer - Receive Acknowledge(0x%02x) check\n", val);
		I2CM_StopTransfer(reg);
		return I2CM_ERR_NACK;
	}

	return I2CM_SUCCESS;
}


/**
 * I2CM_SendData - Register write to I2CM connection device(internal)
 * @return I2CM_SUCCESS     : Successful
 *         I2CM_ERR_NACK    : NACK error
 *         Others
 */
RC I2CM_SendData(I2CM_REG *reg, uchar w_data)
{
	RC ret;
	uchar val;

	/* set data */
	*(reg->ICDRT) = (w_data & 0xFF);

	/* wait for setting TEND */
	ret = I2CM_Wait(reg, ICSR_DATA_TEND);
	if (ret != I2CM_SUCCESS)
	{
		PRINTF("FAIL I2CM SendData - wait TEND error=%d\n", ret);
		I2CM_StopTransfer(reg);
		return ret;
	}
	
	/* check Receive Acknowledge */
	val = *(reg->ICIER);
	if ((val & ICIER_DATA_ACKBR) != 0)
	{
		PRINTF("FAIL I2CM SendData - Receive Acknowledge(0x%02x) check\n", val);
		I2CM_StopTransfer(reg);
		return I2CM_ERR_NACK;
	}

	return I2CM_SUCCESS;
}


/**
 * I2CM_SendData - Register read from I2CM connection device(internal)
 * @return I2CM_SUCCESS     : Successful
 *         Others
 */
RC I2CM_ReceiveData(I2CM_REG *reg, RC r_num, uchar * r_data)
{
	RC ret;
	uchar dummy;
	uchar val;

	/* set master receive mode */
	*(reg->ICSR) &= ~(ICSR_DATA_TEND);

	val = *(reg->ICCR1);
	val |= ICCR1_DATA_MST;
	val &= ~(ICCR1_DATA_TRS);
	*(reg->ICCR1) = val;

	*(reg->ICSR) &= ~(ICSR_DATA_TDRE);


	if (r_num > 1)
	{
		RC i;
		/* Clear ACKBT */
		*(reg->ICIER) &= ~(ICIER_DATA_ACKBT);
		
		/* Dummy-read */
		dummy = *(reg->ICDRR);
		
		/* wait for setting RDRF */
		ret = I2CM_Wait(reg, ICSR_DATA_RDRF);
		if (ret != I2CM_SUCCESS)
		{
			PRINTF("FAIL I2CM ReceiveData - wait RDRF for dummy data error=%d\n", ret);
			I2CM_StopTransfer(reg);
			return ret;
		}

		for (i = 1;i < r_num-1;i++)
		{
			/* read the receive data */
			*r_data++ = *(reg->ICDRR);
			
			/* wait for setting RDRF */
			ret = I2CM_Wait(reg, ICSR_DATA_RDRF);
			if (ret != I2CM_SUCCESS)
			{
				PRINTF("FAIL I2CM ReceiveData - wait RDRF for read data[%d] error=%d\n", i, ret);
				I2CM_StopTransfer(reg);
				return ret;
			}
		}
		
		/* case the (Last receive-1) data read      */
		/* Set acknowledge of the final byte.       */
		/* Disable continuous reception (RCVD = 1). */
		*(reg->ICIER) |= ICIER_DATA_ACKBT;
		*(reg->ICCR1) |= ICCR1_DATA_RCVD;
		
		/* read the (Last-1) receive data */
		*r_data++ = *(reg->ICDRR);

		/* wait for setting RDRF */
		ret = I2CM_Wait(reg, ICSR_DATA_RDRF);
		if (ret != I2CM_SUCCESS)
		{
			PRINTF("FAIL I2CM ReceiveData - wait RDRF for read data[last-1] error=%d\n", ret);
			I2CM_StopTransfer(reg);
			return ret;
		}
	}
	else
	{
		/* when the size of receive data is only one byte */
		/* Set acknowledge of the final byte.       */
		/* Disable continuous reception (RCVD = 1). */
		*(reg->ICIER) |= ICIER_DATA_ACKBT;
		*(reg->ICCR1) |= ICCR1_DATA_RCVD;
		
		/* Dummy-read */
		dummy = *(reg->ICDRR);

		/* wait for setting RDRF */
		ret = I2CM_Wait(reg, ICSR_DATA_RDRF);
		if (ret != I2CM_SUCCESS)
		{
			PRINTF("FAIL I2CM ReceiveData - wait RDRF for dummy data error=%d\n", ret);
			I2CM_StopTransfer(reg);
			return ret;
		}
	}

	/* Clear STOP */
	*(reg->ICSR) &= ~(ICSR_DATA_STOP);
	
	/* issue stop condition */
	val = *(reg->ICCR2);
	val &= ~(ICCR2_DATA_BBSY | ICCR2_DATA_SCP);
	*(reg->ICCR2) = val;

	/* wait for setting RDRF */
	ret = I2CM_Wait(reg, ICSR_DATA_RDRF);
	if (ret != I2CM_SUCCESS)
	{
		PRINTF("FAIL I2CM ReceiveData - wait RDRF for read data[last] error=%d\n", ret);
		I2CM_StopTransfer(reg);
		return ret;
	}

	/* wait for setting STOP */
	ret = I2CM_Wait(reg, ICSR_DATA_STOP);
	if (ret != I2CM_SUCCESS)
	{
		PRINTF("FAIL I2CM ReceiveData - wait STOP error=%d\n", ret);
		I2CM_StopTransfer(reg);
		return ret;
	}
	
	/* read the Last receive data */
	*r_data = *(reg->ICDRR);
	
	/* Clear RCVD */
	*(reg->ICCR1) &= ~(ICCR1_DATA_RCVD);
	
	/* Clear MST */
	*(reg->ICCR1) &= ~(ICCR1_DATA_MST);

	return I2CM_SUCCESS;
}


/**
 * I2CM_StopTransfer - Register access stop(internal)
 * @return None
 */
void I2CM_StopTransfer(I2CM_REG *reg)
{
	RC ret;
	uchar val;

	/* clear TEND bit */
	*(reg->ICSR) &= ~(ICSR_DATA_TEND);

	/* clear STOP bit */
	*(reg->ICSR) &= ~(ICSR_DATA_STOP);

	/* issue stop condition */
	val = *(reg->ICCR2);
	val &= ~(ICCR2_DATA_BBSY | ICCR2_DATA_SCP);
	*(reg->ICCR2) = val;

	/* wait for setting STOP */
	ret = I2CM_Wait(reg, ICSR_DATA_STOP);
	if (ret != I2CM_SUCCESS)
	{
		PRINTF("FAIL I2CM StopTransfer - wait STOP error=%d\n", ret);
	}

	/* set slave receive mode */
	val = *(reg->ICCR1);
	val &= ~(ICCR1_DATA_MST | ICCR1_DATA_TRS);
	*(reg->ICCR1) = val;

	*(reg->ICSR) &= ~(ICSR_DATA_TDRE);

	return;
}


/**
 * I2CM_Wait - Wait process
 * @return I2CM_SUCCESS     : Successful
 *         I2CM_ERR_TIMEOUT : I2C timeout error
 */
RC I2CM_Wait(I2CM_REG *reg, uchar check_bit)
{
	volatile ulong count = 0;
	volatile ulong i;
	volatile uchar temp_icsr;
	
	temp_icsr = *(reg->ICSR);
	
	while (check_bit != (check_bit & temp_icsr))
	{
		temp_icsr = *(reg->ICSR);

		/* chesk Arbitration Lost */
		if ((temp_icsr & ICSR_DATA_ALOVE) != 0)
		{
			PRINTF("FAIL I2CM Wait - issue Arbitration Lost(0x%02x)\n", temp_icsr);
			return I2CM_ERR_ARBITRATION_LOST;
		}

		count++;
		if(count >= ICSR_TIMEOUT_COUNT)
		{
			PRINTF("FAIL I2CM Wait - Time out\n");
			return I2CM_ERR_TIMEOUT;
		}
	}
	
	return I2CM_SUCCESS;
}
