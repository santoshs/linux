/* C_LibAssp.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

/******* include file ********************************************************/
#include "c_typedef.h"
#include "c_defusr.h"


/**
 * EnableIntR - Enable INTR
 * @return None
 */
void EnableIntR(U16 Pipe)
{
	U16  tmp;

	USBRD(INTENB0, tmp);							// Value Save
	USB_CLR_PAT(INTENB0, (BEMPE | NRDYE | BRDYE));	// Interrupt Disable
	USB_SET_PAT(BRDYENB, BitSet[Pipe]);
	USBWR(INTENB0, tmp);							// Interrupt Enable
}


/**
 * DisableIntR - Disable INTR
 * @return None
 */
void DisableIntR(U16 Pipe)
{
	U16  tmp;

	USBRD(INTENB0, tmp);							// Value Save
	USB_CLR_PAT(INTENB0, (BEMPE | NRDYE | BRDYE));	// Interrupt Disable
	USB_CLR_PAT(BRDYENB, BitSet[Pipe]);
	USBWR(INTENB0, tmp);							// Interrupt Enable
}


/**
 * EnableIntE - Enable BEMP
 * @return None
 */
void EnableIntE(U16 Pipe)
{
	U16  tmp;

	USBRD(INTENB0, tmp);							// Value Save
	USB_CLR_PAT(INTENB0, (BEMPE | NRDYE | BRDYE));	// Interrupt Disable
	USB_SET_PAT(BEMPENB, BitSet[Pipe]);
	USBWR(INTENB0, tmp);							// Interrupt Enable
}


/**
 * DisableIntE - Disable BEMP
 * @return None
 */
void DisableIntE(U16 Pipe)
{
	U16  tmp;

	USBRD(INTENB0, tmp);							// Value Save
	USB_CLR_PAT(INTENB0, (BEMPE | NRDYE | BRDYE));	// Interrupt Disable
	USB_CLR_PAT(BEMPENB, BitSet[Pipe]);
	USBWR(INTENB0, tmp);							// Interrupt Enable
}


/**
 * isHiSpeed - Check current speed
 * @return YES : Hi-Speed
 *         NO  : Full-Speed
 */
U16 isHiSpeed(void)
{
	U16	buf;

	USBRD(DVSTCTR, buf);
	if ((buf & RHST) == HSMODE) {
		return YES;									// Hi-Speed mode
	}
	return NO;										// Full-Speed mode
}


/**
 * isHiSpeedEnable - Check Hi-Speed enable bit
 * @return YES : Hi-Speed Enable
 *         NO  : Hi-Speed Disable
 */
U16 isHiSpeedEnable(void)
{
	U16	buf;

	USBRD(SYSCFG, buf);
	if ((buf & HSE) == HSE) {
		return YES;									// Hi-Speed Enable
	}
	return NO;										// Hi-Speed Disable
}


/**
 * SetBUF - Set pipe PID_BUF
 * @return None
 */
void SetBUF(U16 Pipe)
{
    if (Pipe==PIPE0) {
        USB_SET_PAT(DCPCTR,   PID_BUF);
    }
	else if (Pipe <= MAX_PIPE_NO) {
        USB_SET_PAT((PIPEnCTR(Pipe)),   PID_BUF);
    }
}


/**
 * SetNAK - Set pipe PID_NAK
 * @return None
 */
void SetNAK(U16 Pipe)
{
    if (Pipe==PIPE0) {
        USB_CLR_PAT(DCPCTR,   PID_BUF);
    }
	else if(Pipe <= MAX_PIPE_NO) {
        USB_CLR_PAT((PIPEnCTR(Pipe)),   PID_BUF);
    }
}


/**
 * SetSTALL - Set pipe PID_STALL
 * @return None
 */
void SetSTALL(U16 Pipe)
{
    if (Pipe==PIPE0) {
        USB_SET_PAT(DCPCTR,   PID_STALL);
    }
	else if (Pipe <= MAX_PIPE_NO) {
        USB_SET_PAT((PIPEnCTR(Pipe)),   PID_STALL);
    }
}


/**
 * ClrSTALL - Clear pipe PID_STALL
 * @return None
 */
void ClrSTALL(U16 Pipe)
{
    if (Pipe==PIPE0) {
        USB_CLR_PAT(DCPCTR,   PID_STALL);
    }
	else if (Pipe <= MAX_PIPE_NO) {
        USB_CLR_PAT((PIPEnCTR(Pipe)),   PID_STALL);
    }
}


/**
 * GetPid - Get Pipe PID
 * @return PID
 */
U16 GetPid(U16 Pipe)
{
	U16	buf;

    if (Pipe==PIPE0) {
        USBRD(DCPCTR,   buf);
    }
	else if (Pipe <= MAX_PIPE_NO) {
        USBRD((PIPEnCTR(Pipe)), buf);
    }
	else {
        buf = 0;
    }
	return	(U16)(buf & PID);
}


/**
 * DoSQCLR - Do SQCLR
 * @return None
 */
void DoSQCLR(U16 Pipe)
{
    if (Pipe==PIPE0) {
        USB_SET_PAT(DCPCTR,   SQCLR);
    }
	else if (Pipe <= MAX_PIPE_NO) {
        USB_SET_PAT((PIPEnCTR(Pipe)), SQCLR);
    }
}


/**
 * C_FIFOCLR - FIFO buffer clear
 * @return None
 */
void C_FIFOCLR(U16 pipe)
{
	U16		pid, buf, i;

	if (pipe == USEPIPE) {
		FPortChange2(PIPE0, D0USE, NO);
		FPortChange2(PIPE0, D1USE, NO);
		for (i = PIPE1; i <= MAX_PIPE_NO; i++) {
			FPortChange2(i, CUSE, NO);

			pid = GetPid(i);
			SetNAK(i);
			ClrSTALL(i);

			do {
				USBRD(PIPEnCTR(i), buf);
			} while (buf&PBUSY);
			buf |= ACLRM;
			USBWR(PIPEnCTR(i), buf);
			buf &= ~ACLRM;
			USBWR(PIPEnCTR(i), buf);

			if (pid&PID_STALL) {
				SetSTALL(i);
			}
			if (pid&PID_BUF) {
				SetBUF(i);
			}
		}
	}
	else {
		USBRD(D0FIFOSEL, buf);
		if ((buf & CURPIPE) == pipe) {
			FPortChange2(PIPE0, D0USE, NO);
		}
		USBRD(D1FIFOSEL, buf);
		if ((buf & CURPIPE) == pipe) {
			FPortChange2(PIPE0, D1USE, NO);
		}
		FPortChange2(pipe, CUSE, NO);

		pid = GetPid(pipe);
		SetNAK(pipe);
		ClrSTALL(pipe);

		if (pipe == PIPE0) {
			USBWR(CFIFOCTR, BCLR);					// Buffer Clear
			USBWR(CFIFOCTR, BCLR);					// Buffer Clear
		}
		else {
			do {
				USBRD(PIPEnCTR(pipe), buf);
			} while (buf&PBUSY);
			buf |= ACLRM;
			USBWR(PIPEnCTR(pipe), buf);
			buf &= ~ACLRM;
			USBWR(PIPEnCTR(pipe), buf);
		}

		if (pid&PID_STALL) {
			SetSTALL(pipe);
		}
		if (pid&PID_BUF) {
			SetBUF(pipe);
		}
	}
}
