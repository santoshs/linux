/* C_Global.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#define	__GLOBAL_C__

/******* include file ********************************************************/
#include "c_typedef.h"
#include "c_defusr.h"

#include "p_def_ep.h"

#include "p_descrip.h"


/********** global valiable (COMMON) *****************************************/
U16	ConfigNum = 0;									// configuration Number

U32	dtcnt[MAX_PIPE_NO + 1];							// PIPEn Buffer counter
U32	rdcnt[MAX_PIPE_NO + 1];							// PIPEn receive data counter
U8	*dtptr[MAX_PIPE_NO + 1];						// PIPEn Buffer pointer(8bit)
U16	PipeIgnore[MAX_PIPE_NO + 1];					// Ignore count
U16	PipeTbl[MAX_PIPE_NO + 1];						// C/D FIFO | DIR | EPnum

U16	PipeFlag[MAX_PIPE_NO + 1];						// data flag
U32	PipeDataSize[MAX_PIPE_NO + 1];					// data size

U16 BitSet[16] = {	0x0001, 0x0002, 0x0004, 0x0008,	// bit pattern table
					0x0010, 0x0020, 0x0040, 0x0080,
					0x0100, 0x0200, 0x0400, 0x0800,
					0x1000, 0x2000, 0x4000, 0x8000 };

/********** global valiable (PERIPHERAL) *************************************/
// Idle/Setup Stage
void (*pbRequest0[13])(U16 Reqtype, U16 Value, U16 Index, U16 Length) = {
	P_Resrv_0,				P_Resrv_0,				P_Resrv_0,
	P_Resrv_0,				P_Resrv_0,				P_Resrv_0,
	P_Resrv_0,				P_Resrv_0,				P_Resrv_0,
	P_Resrv_0,				P_Resrv_0,				P_Resrv_0,
	P_Resrv_0
};

// Control Read Data Stage
void (*pbRequest1[13])(U16 Reqtype, U16 Value, U16 Index, U16 Length) = {
	P_Get_Status,			P_Resrv_123,			P_Resrv_123,
	P_Resrv_123,			P_Resrv_123,			P_Resrv_123,
	P_Get_Descriptor,		P_Resrv_123,			P_Get_Configuration,
	P_Resrv_123,			P_Get_Interface,		P_Resrv_123,
	P_Synch_Frame
};

// Control Write Data Stage
void (*pbRequest2[13])(U16 Reqtype, U16 Value, U16 Index, U16 Length) = {
	P_Resrv_123,			P_Resrv_123,			P_Resrv_123,
	P_Resrv_123,			P_Resrv_123,			P_Resrv_123,
	P_Resrv_123,			P_Set_Descriptor,		P_Resrv_123,
	P_Resrv_123,			P_Resrv_123,			P_Resrv_123,
	P_Resrv_123
};

// Control Write No Data Status Stage
void (*pbRequest3[13])(U16 Reqtype, U16 Value, U16 Index, U16 Length) = {
	P_Resrv_123,			P_Clear_Feature,		P_Resrv_123,
	P_Set_Feature,			P_Resrv_123,			P_Set_Address,
	P_Resrv_123,			P_Resrv_123,			P_Resrv_123,
	P_Set_Configuration,	P_Resrv_123,			P_Set_Interface,
	P_Resrv_123
};

// Control Read Status Stage
void (*pbRequest4[13])(U16 Reqtype, U16 Value, U16 Index, U16 Length) = {
	P_Resrv_4,				P_Resrv_4,				P_Resrv_4,
	P_Resrv_4,				P_Resrv_4,				P_Resrv_4,
	P_Resrv_4,				P_Resrv_4,				P_Resrv_4,
	P_Resrv_4,				P_Resrv_4,				P_Resrv_4,
	P_Resrv_4
};

// Control Write Status Stage
void (*pbRequest5[13])(U16 Reqtype, U16 Value, U16 Index, U16 Length) = {
	P_Resrv_5,				P_Resrv_5,				P_Resrv_5,
	P_Resrv_5,				P_Resrv_5,				P_Resrv_5,
	P_Resrv_5,				P_Resrv_5,				P_Resrv_5,
	P_Resrv_5,				P_Resrv_5,				P_Resrv_5,
	P_Resrv_5
};

U16	 Alternate[ALT_NO];								// Alternate
U16	 RemoteWakeupFlag = OFF;						// Remote Wakeup Enable Flag
U16	 TestModeFlag = OFF;							// Test Mode Flag
U16  TestModeSelectors;								// Test Mode Selectors

U16	 ReqType;										// request type
U16	 ReqTypeType;									// request type TYPE
U16	 ReqTypeRecip;									// request type RECIPIENT
U16	 ReqRequest;									// request
U16  ReqValue;										// value
U16  ReqIndex;										// index
U16  ReqLength;										// length

U16	 EPTableIndex[MAX_EP_NO + 1];					// Index of Endpoint Information table



/**
 * PipeTblClear - Pipe table clear
 * @return None
 */
void PipeTblClear(void)
{
	U16	i;

	for (i = 0; i < (MAX_PIPE_NO + 1); ++i) {
		PipeTbl[i] = 0;
	}
}


/**
 * P_ClearAlt - Alternate table clear
 * @return None
 */
void P_ClearAlt(void)
{
	U16	i;

	for (i = 0; i < ALT_NO; ++i) {
		Alternate[i] = 0;							// Alternate
	}
}


/**
 * C_MemryClear - Memory clear
 * @return None
 */
void C_MemryClear(void)
{
	U16	i;

	ConfigNum = 0;									// configuration Number
	RemoteWakeupFlag = OFF;							// Remote Wakeup Enable Flag
	P_ClearAlt();									// Alternate setting clear

	for (i = 0; i < (MAX_PIPE_NO + 1); ++i) {
		PipeFlag[i]     = PIPE_IDLE;
		PipeDataSize[i] = 0;
	}
}


/**
 * P_MemConfig - Memory configuration
 * @return None
 */
void P_MemConfig(U16 Value)
{
	ConfigNum = Value;								// Configuration Number set
	P_ClearAlt();									// Alternate setting clear
}


/**
 * P_EPTableIndexClear - Endpoint index table clear
 * @return None
 */
void P_EPTableIndexClear(void)
{
	U16	i;

	for (i = 0; i <= MAX_EP_NO; ++i) {
		EPTableIndex[i] = EP_ERROR;
	}
}


/**
 * P_resetDeviceDescriptor - Device descriptor table initialize
 * @return None
 */
void P_resetDeviceDescriptor(U16 Mode)
{
    U8 count;
	if (Mode == HI_SPEED) {
		// Hi-Speed Mode
		Configuration_Hi_1[1]	= DT_CONFIGURATION;
		Configuration_Full_1[1]	= DT_OTHER_SPEED_CONFIGURATION;
		ConPtr[0]				= Configuration_Hi_1;
		ConPtr_Other[0]			= Configuration_Full_1;
        // MaxPacketSize Setting(BULK)
        for (count = 0; count<MAX_EP_NO; count++) {
            if ((EPtbl_1[1 + count * EPL] & TYP) == BULK) {
                EPtbl_1[3 + count * EPL] = 512;
            }
        }
	}
	else {
		// Full-Speed Mode
		Configuration_Hi_1[1]	= DT_OTHER_SPEED_CONFIGURATION;
		Configuration_Full_1[1]	= DT_CONFIGURATION;
		ConPtr[0]				= Configuration_Full_1;
		ConPtr_Other[0]			= Configuration_Hi_1;
        // MaxPacketSize Setting(BULK)
        for (count = 0; count<MAX_EP_NO; count++) {
            if ((EPtbl_1[1 + count * EPL] & TYP) == BULK) {
                EPtbl_1[3 + count * EPL] = 64;
            }
        }
	}
}
