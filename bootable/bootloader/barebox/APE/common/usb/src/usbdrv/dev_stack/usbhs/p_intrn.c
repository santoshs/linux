/* P_IntRN.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

/******* include file ********************************************************/
#include "c_typedef.h"
#include "c_defusr.h"


/**
 * P_INTR_int - INTR interrupt
 * @return None
 */
void P_INTR_int(U16 Status, U16 Int_enbl)
{

	if ((Status & (BitSet[PIPE0])) && (Int_enbl & (BitSet[PIPE0]))) {
		USB_CLR_STS(BRDYSTS, (BitSet[PIPE0]));
		USB_MDF_PAT(CFIFOSEL, PIPE0, CURPIPE);
		switch (Buf_Read_C(PIPE0)) {
		case	READING:					// Continue of data read
		case	READEND:					// End of data read
			SetBUF(PIPE0);					// PID = BUF
			break;
		case	READSHRT:					// End of data read
			DisableIntR(PIPE0);
			break;
		case	READOVER:					// FIFO access error
			USBWR(CFIFOCTR, BCLR);			// Clear BVAL
			DisableIntR(PIPE0);
			SetSTALL(PIPE0);				// Req Error
			break;
		case	FIFOERROR:					// FIFO access error
		default:
			DisableIntR(PIPE0);
			SetSTALL(PIPE0);				// Req Error
			break;
		}
	}
	else {
		INTR_int_pipe(Status, Int_enbl);
	}
}


/**
 * P_INTN_int - INTN interrupt
 * @return None
 */
void P_INTN_int(U16 Status, U16 Int_enbl)
{
	if ((Status & (BitSet[PIPE0])) && (Int_enbl & (BitSet[PIPE0]))) {
		USB_CLR_STS(NRDYSTS, (BitSet[PIPE0]));
	}
	else {
		INTN_int_pipe(Status, Int_enbl);
	}
}


/**
 * P_BEMP_int - BEMP interrupt
 * @return None
 */
void P_BEMP_int(U16 Status, U16 Int_enbl)
{
	if ((Status & (BitSet[PIPE0])) && (Int_enbl & (BitSet[PIPE0]))) {
		USB_CLR_STS(BEMPSTS, (BitSet[PIPE0]));
		USB_MDF_PAT(CFIFOSEL, PIPE0, CURPIPE);
		Buf_Write_C(PIPE0);
	}
	else {
		BEMP_int_pipe(Status, Int_enbl);
	}
}
