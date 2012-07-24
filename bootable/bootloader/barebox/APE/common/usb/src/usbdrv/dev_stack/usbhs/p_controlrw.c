/* P_ControlRW.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

/******* include file ********************************************************/
#include "c_typedef.h"
#include "c_defusr.h"


/**
 * P_CR_Start - Control read start
 * @return Except FIFOERROR : Normal end
 *                FIFOERROR : Abnormal end
 */
U16 P_CR_Start(U32 Bsize, U8 *Table)
{
	U16	EndFlag_K;

	dtcnt[PIPE0] = Bsize;
	dtptr[PIPE0] = Table;

	FPortChange2(PIPE0,CUSE,ISEL);
	USBWR(CFIFOCTR, BCLR);							// Buffer Clear

	EndFlag_K	= Buf_Write_C(PIPE0);

	// Peripheral Control sequence
	switch (EndFlag_K) {
	case	WRITESHRT:								// End of data write
	case	WRITEEND:								// End of data write (not null)
	case	WRITING:								// Continue of data write
		EnableIntE(PIPE0);							// Enable Empty Interrupt
		break;
	case	FIFOERROR:								// FIFO access error
		break;
	default:
		break;
	}
	SetBUF(PIPE0);									// Set BUF
	return	(EndFlag_K);							// End or Err or Continue
}


/**
 * P_CW_Start - Control write start
 * @return None
 */
void P_CW_Start(U32 Bsize, U8 *Table)
{
	dtcnt[PIPE0] = Bsize;
	dtptr[PIPE0] = Table;

	FPortChange2(PIPE0,CUSE,NO);
	USBWR(CFIFOCTR, BCLR);							// Buffer Clear

	EnableIntR(PIPE0);
	SetBUF(PIPE0);
}
