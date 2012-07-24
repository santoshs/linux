/* P_Lib596.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

/******* include file ********************************************************/
#include "c_typedef.h"
#include "c_defusr.h"


/**
 * P_resetDCP - Reset default control pipe
 * @return None
 */
void P_resetDCP(void)
{
	USBWR(CFIFOSEL, CFIFO_MBW_INIT);				// CFIFO Port Select Register  (0x1E)
	USBWR(DCPMAXP,  DeviceDescriptor[7]);			// DCP Maxpacket Size Register (0x5E)

	USBWR(D0FIFOSEL, MBW_16);						// D0FIFO Port Select Register (0x24)
	USBWR(D1FIFOSEL, MBW_16);						// D1FIFO Port Select Register (0x2A)
	/*
	 * DCP FIFO Clear, etc ...
	 */
}


/**
 * P_resetEP - Reset endpoint
 * @return None
 */
void P_resetEP(U16 Con_Num)
{
	U16		pipe, ep, index, buf;
	U16		*tbl;

	// Pipe Setting
	tbl = (U16 *)(EndPntPtr[Con_Num-1]);

	for (ep = 1; ep <= MAX_EP_NO; ++ep) {
		if (EPTableIndex[ep] != EP_ERROR) {
			index = EPL * EPTableIndex[ep];
			pipe = (tbl[index+0] & CURPIPE);		// Pipe Number

			PipeTbl[pipe] = ((tbl[index+1] & DIR) << 3) | ep | (tbl[index+0] & FIFO_USE);

			if ((tbl[index+0]&(FIFO_USE)) != C_FIFO_USE) {
				tbl[index+1]	|= SHTNAK;
			}

			// Interrupt Disable
			USB_CLR_PAT(BRDYENB, BitSet[pipe]);		// Ready Interrupt Disable
			USB_CLR_PAT(NRDYENB, BitSet[pipe]);		// NotReady Interrupt Disable
			USB_CLR_PAT(BEMPENB, BitSet[pipe]);		// Empty/SizeErr Interrupt Disable

			SetNAK(pipe);							// PID = NAK

			// CurrentPIPE Clear
			USBRD(CFIFOSEL,  buf);
			if ((buf & CURPIPE) == pipe)
				USB_CLR_PAT(CFIFOSEL,  CURPIPE);
			USBRD(D0FIFOSEL, buf);
			if ((buf & CURPIPE) == pipe)
				USB_CLR_PAT(D0FIFOSEL, CURPIPE);
			USBRD(D1FIFOSEL, buf);
			if ((buf & CURPIPE) == pipe)
				USB_CLR_PAT(D1FIFOSEL, CURPIPE);

			// PIPE Configuration
			USBWR(PIPESEL,  pipe);
			USBWR(PIPECFG,  tbl[index+1]);
			USBWR(PIPEBUF,  tbl[index+2]);
			USBWR(PIPEMAXP, tbl[index+3]);
			USBWR(PIPEPERI, tbl[index+4]);

			// Buffer Clear
			DoSQCLR(pipe);							// SQCLR=1

			// init Global
			PipeFlag[pipe]     = PIPE_IDLE;
			PipeDataSize[pipe] = 0;
		}
	}
}
