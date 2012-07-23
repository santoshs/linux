/* P_StdReqGet.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

/******* include file ********************************************************/
#include "c_typedef.h"
#include "c_defusr.h"


/**
 * P_Get_Status - Get_Status
 * @return None
 */
void P_Get_Status(U16 Reqtype, U16 Value, U16 Index, U16 Length)
{
	static U8	tbl[2];
	U16			ep;
	U16			buffer;
	U16			pipe;

	if ((Value == 0) && (Length == 2)) {
		tbl[0] = 0;
		tbl[1] = 0;
		switch (Reqtype) {
		case DEVICE:
			if (Index == 0) {
				tbl[0] = P_GetCurrentPower();		// SelfPowered / BusPowered
				if (RemoteWakeupFlag == ON)			// Support Remote Wakeup
					tbl[0] |= GS_REMOTEWAKEUP;
				P_CR_Start(2, tbl);
			}
			else {
				SetSTALL(PIPE0);					// Req Error
			}
			break;
		case INTERFACE:
			if (P_isConfigured() == YES) {
				if (Index < P_Isrch(ConfigNum)) {
					P_CR_Start(2, tbl);				// Return 0
				}
				else {
					SetSTALL(PIPE0);				// Req Error (not exist interface)
				}
			}
			else {
				SetSTALL(PIPE0);					// Req Error
			}
			break;
		case ENDPOINT:
			ep = (Index & EPNUM);					// Endpoint Number
			if (ep == 0) {							// Endpoint 0
				USBRD(DCPCTR, buffer);
				if ((buffer & PID_STALL) == PID_STALL) {
				   tbl[0] = GS_HALT;				// Halt set
				}
				P_CR_Start(2, tbl);
			}
			else if (ep <= MAX_EP_NO) {				// EP1 to max
				if (P_isConfigured() == YES) {
					pipe = P_EpToPipe(Index);
					if (pipe == EP_ERROR) {
						SetSTALL(PIPE0);
					}
					else {
						buffer = GetPid(pipe);
						if ((buffer & PID_STALL) == PID_STALL) {
							tbl[0] = GS_HALT;		// Halt set
						}
						P_CR_Start(2, tbl);
					}
				}
				else {
					SetSTALL(PIPE0);
				}
			}
			else {
				SetSTALL(PIPE0);
			}
			break;
		default:
			SetSTALL(PIPE0);
			break;
		}
	}
	else {
		SetSTALL(PIPE0);
	}
}


/**
 * P_Get_Descriptor - Get_Descriptor
 * @return None
 */
void P_Get_Descriptor(U16 Reqtype, U16 Value, U16 Index, U16 Length)
{
	U16		len, idx;

	if (Reqtype == DEVICE) {
		idx = (Value & DT_INDEX);
		switch (GET_DT_TYPE(Value)) {
		case DT_DEVICE:								//---- device Descriptor ----
			if ((Index == 0) && (idx == 0)) {
				if (Length < DeviceDescriptor[0]) {
					P_CR_Start(Length, DeviceDescriptor);
				}
				else {
					P_CR_Start(DeviceDescriptor[0], DeviceDescriptor);
				}
			}
			else {
				SetSTALL(PIPE0);					// Req Error
			}
			break;
		case DT_CONFIGURATION:						//---- Configuration Descriptor ----
			if (Index == 0) {
				len  = (U16)(*(ConPtr[idx]+2));
				len |= (U16)(*(ConPtr[idx]+3)) << 8;
				if (Length < len) {					// Descriptor > wLength
					P_CR_Start(Length, ConPtr[idx]);
				}
				else {
					P_CR_Start(len, ConPtr[idx]);
				}
			}
			else {
				SetSTALL(PIPE0);					// Req Error
			}
			break;
		case DT_STRING:								//---- String Descriptor ----
			len = (U16)(*(StrPtr[idx]+0));
			if (Length < len) {
				P_CR_Start(Length, StrPtr[idx]);
			}
			else {
				P_CR_Start(len, StrPtr[idx]);
			}
			break;
		case DT_INTERFACE:							//---- Interface Descriptor ----
			SetSTALL(PIPE0);
			break;
		case DT_ENDPOINT:							//---- Endpoint Descriptor ----
			SetSTALL(PIPE0);
			break;
		case DT_DEVICE_QUALIFIER:
			if ((isHiSpeedEnable() == YES) && (Index == 0) && (idx == 0)) {
				if (Length < QualifierDescriptor[0]) {
					P_CR_Start(Length, QualifierDescriptor);
				}
				else {
					P_CR_Start(QualifierDescriptor[0], QualifierDescriptor);
				}
			}
			else {
				SetSTALL(PIPE0);					// Req Error
			}
			break;
		case DT_OTHER_SPEED_CONFIGURATION:
			if ((isHiSpeedEnable() == YES) && Index == 0) {
				len  = (U16)(*(ConPtr_Other[idx]+2));
				len |= (U16)(*(ConPtr_Other[idx]+3)) << 8;
				if (Length < len) {					// Descriptor > wLength
					P_CR_Start(Length, ConPtr_Other[idx]);
				}
				else {
					P_CR_Start(len, ConPtr_Other[idx]);
				}
			}
			else {
				SetSTALL(PIPE0);					// Req Error
			}
			break;
		case DT_INTERFACE_POWER:
			SetSTALL(PIPE0);						// Not Support
			break;
		default:
			SetSTALL(PIPE0);
			break;
		}
	}
	else {
		SetSTALL(PIPE0);
	}
}


/**
 * P_Get_Configuration - Get_Configuration
 * @return None
 */
void P_Get_Configuration(U16 Reqtype, U16 Value, U16 Index, U16 Length)
{
	static U8	tbl[1];

	if ((Reqtype == DEVICE) && (Value == 0) && (Index == 0) && (Length == 1)) {
		tbl[0] = (U8)ConfigNum;
		P_CR_Start(1, tbl);
	}
	else {
		SetSTALL(PIPE0);
	}
}


/**
 * P_Get_Interface - Get_Interface
 * @return None
 */
void P_Get_Interface(U16 Reqtype, U16 Value, U16 Index, U16 Length)
{
	static U8	tbl[1];

	if ((Reqtype == INTERFACE) && (Value == 0) && (Length == 1)) {
		if (Index < ALT_NO) {
			tbl[0] = (U8)Alternate[Index];
			P_CR_Start(1, tbl);						// Start control read
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
 * P_Resrv_0 - Reserved (Idle/Setup Stage)
 * @return None
 */
void P_Resrv_0(U16 Reqtype, U16 Value, U16 Index, U16 Length)
{
	/*==== Reserved ====*/
}


/**
 * P_Resrv_123 - Reserved (Control Read Data Stage, Control Write Data Stage, Control Write No Data Status Stage)
 * @return None
 */
void P_Resrv_123(U16 Reqtype, U16 Value, U16 Index, U16 Length)
{
	/*==== Reserved ====*/
	SetSTALL(PIPE0);
}


/**
 * P_Resrv_4 - Reserved (Control Read Status Stage)
 * @return None
 */
void P_Resrv_4(U16 Reqtype, U16 Value, U16 Index, U16 Length)
{
	/*==== Reserved ====*/
}


/**
 * P_Resrv_5 - Reserved (Control Write Status Stage)
 * @return None
 */
void P_Resrv_5(U16 Reqtype, U16 Value, U16 Index, U16 Length)
{
	/*==== Reserved ====*/
}


/**
 * P_GetCurrentPower - Get currently power
 * @return GS_SELFPOWERD : Device is currently self-powered
 *         GS_BUSPOWERD  : Device is currently bus-powered
 */
U8 P_GetCurrentPower(void)
{
	/*
	 * Please answer the currently power of your system.
	 */

    U8	tmp, current;
    U16	conf;

	conf = ConfigNum;
	if (conf < 1)
		conf = 1;									// Address state

	// Standard Configuration Descriptor
	tmp = *(ConPtr[(conf-1)]+7);
	if (tmp & CF_SELF)
		current = GS_SELFPOWERD;
	else
		current = GS_BUSPOWERD;

	/* check Currently Powered */

	return current;
}
