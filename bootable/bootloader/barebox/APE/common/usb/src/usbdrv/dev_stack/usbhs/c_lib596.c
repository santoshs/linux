/* C_Lib596.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

/******* include file ********************************************************/
#include "c_typedef.h"
#include "c_defusr.h"


/**
 * getBufSize - Return Buf size
 * @return Buf size
 */
U16 getBufSize(U16 Pipe)
{
	U16	size, buffer;

	if (Pipe == PIPE0) {
		USBRD(DCPMAXP, buffer);
		size = (U16)(buffer & MAXP);				// Max Packet Size
	}
	else {
		USBWR(PIPESEL, Pipe);						// pipe select
		USBRD(PIPECFG, buffer);						// read CNTMD
		if( (buffer&CNTMD)==CNTMD ) {
			USBRD(PIPEBUF, buffer);
			size = (U16)(((buffer>>10) + 1) * PIPExBUF);	// Buffer Size
		}
		else {
			USBRD(PIPEMAXP, buffer);
			size = (U16)(buffer & MXPS);			// Max Packet Size
		}
	}
	return size;
}


/**
 * getMaxPacketSize - Get max packet size
 * @return Max packet size
 */
U16 getMaxPacketSize(U16 Pipe)
{
	U16	size, buffer;

	if (Pipe == PIPE0) {
		USBRD(DCPMAXP, buffer);
		size = (U16)(buffer & MAXP);				// Max Packet Size
	}
	else {
		USBWR(PIPESEL, Pipe);						// pipe select
		USBRD(PIPEMAXP, buffer);
		size = (U16)(buffer & MXPS);				// Max Packet Size
	}
	return size;
}


/**
 * VBINT_StsClear - VBINT Interrupt Status Clear
 * @return None
 */
void VBINT_StsClear(void)
{
	U16	buf;

	USB_CLR_STS(INTSTS0, VBINT);					// Status Clear
	USBRD(SYSCFG, buf);
	if ((buf & SCKE) != SCKE) {
		USB_SET_STS(INTSTS0, VBINT);				// Status Clear (Clock stop)
	}
}


/**
 * RESM_StsClear - RESM Interrupt Status Clear
 * @return None
 */
void RESM_StsClear(void)
{
	U16	buf;

	USB_CLR_STS(INTSTS0, RESM);						// Status Clear
	USBRD(SYSCFG, buf);
	if ((buf & SCKE) != SCKE) {
		USB_SET_STS(INTSTS0, RESM);					// Status Clear (Clock stop)
	}
}
