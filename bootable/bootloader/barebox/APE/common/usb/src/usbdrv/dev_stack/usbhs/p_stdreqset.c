/* P_StdReqSet.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

/******* include file ********************************************************/
#include "c_typedef.h"
#include "c_defusr.h"


/**
 * P_Clear_Feature - Clear_Feature
 * @return None
 */
void P_Clear_Feature(U16 Reqtype, U16 Value, U16 Index, U16 Length)
{
	U16		pipe, ep;

	if (Length == 0) {
		switch (Reqtype) {
		case DEVICE:
			if (Value == DEVICE_REMOTE_WAKEUP && Index == 0) {
				if (P_RWsrch() == ON) {
					RemoteWakeupFlag = OFF;
					SetBUF(PIPE0);					// OK
				}
				else {
					// Not Support RemoteWakeup
					SetSTALL(PIPE0);				// Req Error
				}
			}
			else {
				SetSTALL(PIPE0);					// Not Spec
			}
			break;
		case INTERFACE:
			SetSTALL(PIPE0);						// Req Error
			break;
		case ENDPOINT:
			ep = (Index & EPNUM);					// Endpoint Number
			if (Value == ENDPOINT_HALT) {
				if (ep == 0) {						// EP0
					SetBUF(PIPE0);					// Clear STALL & Set BUF
				}
				else if (ep <= MAX_EP_NO) {			// EP1 to max
					pipe = P_EpToPipe(Index);
					if (pipe == EP_ERROR) {
						SetSTALL(PIPE0);			// Req Error
					}
					else {
						SetNAK(pipe);				// Clear STALL & Set NAK
						DoSQCLR(pipe);				// SQCLR=1
						ClrSTALL(pipe);
						SetBUF(pipe);				// Set BUF
						SetBUF(PIPE0);
					}
				}
				else {
					SetSTALL(PIPE0);				// Req Error
				}
			}
			else {
				SetSTALL(PIPE0);					// Req Error
			}
			break;
		default:
			SetSTALL(PIPE0);
			break;
		}
	}
	else {
		SetSTALL(PIPE0);							// Not Spec
	}
}


/**
 * P_Set_Feature - Set_Feature
 * @return None
 */
void P_Set_Feature(U16 Reqtype, U16 Value, U16 Index, U16 Length)
{
	U16		pipe, ep;

	if (Length == 0) {
		switch (Reqtype) {
		case DEVICE:
			switch (Value) {
			case DEVICE_REMOTE_WAKEUP:
				if (Index == 0) {
					if (P_RWsrch() == ON) {
						RemoteWakeupFlag = ON;
						SetBUF(PIPE0);				// OK
					}
					else {
						// Not Support RemoteWakeup
						SetSTALL(PIPE0);			// Req Error
					}
				}
				else {
					SetSTALL(PIPE0);				// Not Spec
				}
				break;
			case TEST_MODE:
				if (isHiSpeed() == YES) {
					if (Index < TEST_Reserved || TEST_VSTModes <= Index) {
						TestModeFlag = YES;
						TestModeSelectors = Index;
						SetBUF(PIPE0);				// OK
					}
					else {
						SetSTALL(PIPE0);			// Not Spec
					}
				}
				else {
					SetSTALL(PIPE0);				// Not Spec
				}
				break;
			default:
				SetSTALL(PIPE0);					// Not Spec
				break;
			}
			break;
		case INTERFACE:
			SetSTALL(PIPE0);
			break;
		case ENDPOINT:
			ep = (Index & EPNUM);					// Endpoint Number
			if (Value == ENDPOINT_HALT) {
				if (ep == 0) {						// EP0
					SetBUF(PIPE0);
				}
				else if (ep <= MAX_EP_NO) {			// EP1 to max
					pipe = P_EpToPipe(Index);
					if (pipe == EP_ERROR) {
						SetSTALL(PIPE0);			// Req Error
					}
					else {
						SetSTALL(pipe);				// Set STALL
						SetBUF(PIPE0);				// OK
					}
				}
				else {
					SetSTALL(PIPE0);				// Req Error
				}
			}
			else {
				SetSTALL(PIPE0);					// Not Spec
			}
			break;

		default:
			SetSTALL(PIPE0);						// Req Error
			break;
		}
	}
	else {
		SetSTALL(PIPE0);							// Req Error
	}
}


/**
 * P_Set_Address - Set_Address
 * @return None
 */
void P_Set_Address(U16 Reqtype, U16 Value, U16 Index, U16 Length)
{
	if (Reqtype == DEVICE) {
		if ((Index == 0) && (Length == 0)) {
			if (Value <= 127) {
				SetBUF(PIPE0);						// OK
			}
			else {
				SetSTALL(PIPE0);					// Not Spec
			}
		}
		else {
			SetSTALL(PIPE0);						// Not Spec
		}
	}
	else {
		SetSTALL(PIPE0);							// Req Error
	}
}


/**
 * P_Set_Descriptor - Set_Descriptor
 * @return None
 */
void P_Set_Descriptor(U16 Reqtype, U16 Value, U16 Index, U16 Length)
{
	SetSTALL(PIPE0);								// Not Spec
}


/**
 * P_Set_Configuration - Set_Configuration
 * @return None
 */
void P_Set_Configuration(U16 Reqtype, U16 Value, U16 Index, U16 Length)
{
	U16	i, ifc;

	if (Reqtype == DEVICE) {
		if ((Value <= P_Csrch()) && (Index == 0) && (Length == 0)) {
			P_MemConfig(Value);						// Configuration Number set
			SetBUF(PIPE0);
			if (ConfigNum > 0) {
				P_EPTableIndexClear();
				ifc = P_Isrch(ConfigNum);
				for (i = 0; i < ifc; ++i) {
					P_Esrch(ConfigNum, i, 0);
				}
				PipeTblClear();
				P_resetEP(ConfigNum);
			}
			P_Change_Config(ConfigNum);
		}
		else {
			SetSTALL(PIPE0);						// Req Error
		}
	}
	else {
		SetSTALL(PIPE0);							// Req Error
	}
}


/**
 * P_Set_Interface - Set_Interface
 * @return None
 */
void P_Set_Interface(U16 Reqtype, U16 Value, U16 Index, U16 Length)
{
	if ((P_isConfigured() == YES) && (Reqtype == INTERFACE)) {	// Configured
		if ((Index <= P_Isrch(ConfigNum)) && (Length == 0)) {
			if (Value <= P_Asrch(ConfigNum, Index)) {
				Alternate[Index] = (Value & ALT_SET);
				SetBUF(PIPE0);
				P_EPTableIndexClear();
				P_Esrch(ConfigNum, Index, Alternate[Index]);	// Search endpoint setting
				P_resetEP(ConfigNum);
				P_Change_Interface(ConfigNum, Index, Alternate[Index]);
			}
			else {
				SetSTALL(PIPE0);					// Req Error
			}
		}
		else {
			SetSTALL(PIPE0);						// Req Error
		}
	}
	else {
		SetSTALL(PIPE0);							// Req Error
	}
}


/**
 * P_Synch_Frame - Synch_Frame
 * @return None
 */
void P_Synch_Frame(U16 Reqtype, U16 Value, U16 Index, U16 Length)
{
	SetSTALL(PIPE0);
}
