/* C_Extern.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef __GLOBAL_C__

/********** global valiable (COMMON) *****************************************/
extern U16			ConfigNum;						// configuration Number

extern U32			dtcnt[];						// PIPEn Buffer counter
extern U32			rdcnt[];						// PIPEn receive data counter
extern U8			*dtptr[];						// PIPEn Buffer pointer
extern U16			PipeIgnore[];					// Ignore count
extern U16			PipeTbl[];
extern U16			PipeFlag[];
extern U32			PipeDataSize[];

extern U16			BitSet[];

/********** global valiable (PERIPHERAL) *************************************/
extern U16			EPtbl_1[];
extern const U16	*EndPntPtr[];

extern U8			DeviceDescriptor[];
extern U8			QualifierDescriptor[];
extern U8			Configuration_Full_1[];
extern U8			Configuration_Hi_1[];
extern U8			*ConPtr[];
extern U8			*ConPtr_Other[];

extern U8			*StrPtr[];

extern void			(*pbRequest0[])(U16 Reqtype, U16 Value, U16 Index, U16 Length);
extern void			(*pbRequest1[])(U16 Reqtype, U16 Value, U16 Index, U16 Length);
extern void			(*pbRequest2[])(U16 Reqtype, U16 Value, U16 Index, U16 Length);
extern void			(*pbRequest3[])(U16 Reqtype, U16 Value, U16 Index, U16 Length);
extern void			(*pbRequest4[])(U16 Reqtype, U16 Value, U16 Index, U16 Length);
extern void			(*pbRequest5[])(U16 Reqtype, U16 Value, U16 Index, U16 Length);

extern U16			Alternate[];
extern U16			RemoteWakeupFlag;				// Remote Wakeup Enable Flag
extern U16			TestModeFlag;
extern U16			TestModeSelectors;
extern U16			EPTableIndex[];					// Index of EP Information table
extern U16			ReqType;						// request type
extern U16			ReqTypeType;					// request type TYPE
extern U16			ReqTypeRecip;					// request type RECIPIENT
extern U16			ReqRequest;						// request
extern U16			ReqValue;						// value
extern U16			ReqIndex;						// index
extern U16			ReqLength;						// length

#endif /* __C_GLOBAL_C__ */

/********** global function (COMMON) *****************************************/
extern void			C_MemryClear(void);				// memory data clear
extern void			PipeTblClear(void);

extern void			Receive_Start(U16 Pipe, U32 Bsize, U8 *Tbl);
extern U16			Send_Start(U16 Pipe, U32 Bsize, U8 *Tbl);
extern U16			Buf_Write(U16 Pipe);
extern U16			Buf_Write_C(U16 Pipe);
extern U16			Buf_Read(U16 Pipe);
extern U16			Buf_Read_C(U16 Pipe);
extern void			DMA_Read(void);
extern U16			DMA_Read_C(void);

extern U16			GetPid(U16 Pipe);
extern void			SetBUF(U16 Pipe);
extern void			SetNAK(U16 Pipe);
extern void			SetSTALL(U16 Pipe);
extern void			ClrSTALL(U16 Pipe);
extern void			DoSQCLR(U16 Pipe);
extern void			C_FIFOCLR(U16 pipe);

extern void			usbint(void);
extern void			INTR_int_pipe(U16 Status, U16 Int_enbl);
extern void			INTN_int_pipe(U16 Status, U16 Int_enbl);
extern void			BEMP_int_pipe(U16 Status, U16 Int_enbl);

extern U16			isHiSpeed(void);
extern U16			isHiSpeedEnable(void);

extern void			EnableIntR(U16 Pipe);
extern void			DisableIntR(U16 Pipe);
extern void			EnableIntE(U16 Pipe);
extern void			DisableIntE(U16 Pipe);

extern void			VBINT_StsClear(void);
extern void			RESM_StsClear(void);

extern U16			getBufSize(U16 Pipe);
extern U16			getMaxPacketSize(U16 Pipe);

extern void			stopClock(void);

extern U16			FPortChange1(U16 Pipe, U16 fifosel, U16 isel);
extern void			FPortChange2(U16 Pipe, U16 fifosel, U16 isel);

/********** global function (PERIPHERAL) *************************************/
extern void			P_usbint(void);

extern U8			P_GetCurrentPower(void);
extern void			P_Change_Config(U16 Con_Num);
extern void			P_Change_Interface(U16 Con_Num, U16 Int_Num, U16 Alt_Num);

extern U16			P_Csrch(void);
extern U16			P_Isrch(U16 Con_Num);
extern U16			P_Asrch(U16 Con_Num, U16 Int_Num);
extern void			P_Esrch(U16 Con_Num, U16 Int_Num, U16 Alt_Num);
extern void			P_EPTableIndexClear(void);
extern U16			P_RWsrch(void);
extern void			P_resetDeviceDescriptor(U16 Mode);
extern void			P_saveRequest(void);

extern void			P_INTR_int(U16 Status, U16 Int_enbl);
extern void			P_INTN_int(U16 Status, U16 Int_enbl);
extern void			P_BEMP_int(U16 Status, U16 Int_enbl);

extern void			P_Class0 (U16 Reqtype, U16 Request, U16 Value, U16 Index, U16 Length);
extern void			P_Class1 (U16 Reqtype, U16 Request, U16 Value, U16 Index, U16 Length);
extern void			P_Class2 (U16 Reqtype, U16 Request, U16 Value, U16 Index, U16 Length);
extern void			P_Class3 (U16 Reqtype, U16 Request, U16 Value, U16 Index, U16 Length);
extern void			P_Class4 (U16 Reqtype, U16 Request, U16 Value, U16 Index, U16 Length);
extern void			P_Class5 (U16 Reqtype, U16 Request, U16 Value, U16 Index, U16 Length);
extern void			P_Vendor0(U16 Reqtype, U16 Request, U16 Value, U16 Index, U16 Length);
extern void			P_Vendor1(U16 Reqtype, U16 Request, U16 Value, U16 Index, U16 Length);
extern void			P_Vendor2(U16 Reqtype, U16 Request, U16 Value, U16 Index, U16 Length);
extern void			P_Vendor3(U16 Reqtype, U16 Request, U16 Value, U16 Index, U16 Length);
extern void			P_Vendor4(U16 Reqtype, U16 Request, U16 Value, U16 Index, U16 Length);
extern void			P_Vendor5(U16 Reqtype, U16 Request, U16 Value, U16 Index, U16 Length);

extern void			P_Get_Status	   (U16 Reqtype, U16 Value, U16 Index, U16 Length);
extern void			P_Clear_Feature	   (U16 Reqtype, U16 Value, U16 Index, U16 Length);
extern void			P_Set_Feature	   (U16 Reqtype, U16 Value, U16 Index, U16 Length);
extern void			P_Set_Address	   (U16 Reqtype, U16 Value, U16 Index, U16 Length);
extern void			P_Get_Descriptor   (U16 Reqtype, U16 Value, U16 Index, U16 Length);
extern void			P_Set_Descriptor   (U16 Reqtype, U16 Value, U16 Index, U16 Length);
extern void			P_Get_Configuration(U16 Reqtype, U16 Value, U16 Index, U16 Length);
extern void			P_Set_Configuration(U16 Reqtype, U16 Value, U16 Index, U16 Length);
extern void			P_Get_Interface	   (U16 Reqtype, U16 Value, U16 Index, U16 Length);
extern void			P_Set_Interface	   (U16 Reqtype, U16 Value, U16 Index, U16 Length);
extern void			P_Synch_Frame	   (U16 Reqtype, U16 Value, U16 Index, U16 Length);

extern void			P_Resrv_0	(U16 Reqtype, U16 Value, U16 Index, U16 Length);
extern void			P_Resrv_123	(U16 Reqtype, U16 Value, U16 Index, U16 Length);
extern void			P_Resrv_4	(U16 Reqtype, U16 Value, U16 Index, U16 Length);
extern void			P_Resrv_5	(U16 Reqtype, U16 Value, U16 Index, U16 Length);

extern void			P_usb_busreset(void);
extern void			P_usb_resume(void);
extern void			P_usb_suspend(void);
extern void			P_usb_testmode(void);

extern void			P_resetDCP(void);
extern U16			P_isConfigured(void);
extern U16			P_EpToPipe(U16 Dir_Ep);

extern void			P_resetEP(U16 Con_Num);			// set PIPEn Configuration register

extern void			P_MemConfig(U16 Value);

extern U16			P_CR_Start(U32 Bsize, U8 *Table);	// start Control Read
extern void			P_CW_Start(U32 Bsize, U8 *Table);	// start Control Write
