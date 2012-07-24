/* P_UsbInt.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

/******* include file ********************************************************/
#include "c_typedef.h"
#include "c_defusr.h"


/**
 * P_usbint - USB interrupt
 * @return None
 */
void P_usbint(void)
{
	U16	int_sts0, int_sts1, int_sts2, int_sts3;		// interrupt status
	U16	int_enb0, int_enb2, int_enb3, int_enb4;		// interrupt enable
    TBL_USBF_PARAM param;

	USBRD(INTSTS0, int_sts0);
	if (!(int_sts0 & (VBINT|RESM|SOFR|DVST|CTRT|BEMP|NRDY|BRDY))) {
		return;
	}

	USBRD(BRDYSTS, int_sts1);
	USBRD(NRDYSTS, int_sts2);
	USBRD(BEMPSTS, int_sts3);
	USBRD(INTENB0, int_enb0);
	USBRD(BRDYENB, int_enb2);
	USBRD(NRDYENB, int_enb3);
	USBRD(BEMPENB, int_enb4);

	if ((int_sts0 & RESM) && (int_enb0 & RSME)) {
		P_usb_resume();
	}
	else if ((int_sts0 & VBINT) && (int_enb0 & VBSE)) {
		VBINT_StsClear();							// Status Clear
		// User program
        param.pipenum       = 0;
        param.param_data    = USBF_NULL;
        UsbF_CallBack( CBID_VBUS, &param );
	}
	else if ((int_sts0 & SOFR) && (int_enb0 & SOFE)) {
		USB_CLR_STS(INTSTS0, SOFR);					// SOFR Clear
		// User program
	}
	else if ((int_sts0 & DVST) && (int_enb0 & DVSE)) {
		USB_CLR_STS(INTSTS0, DVST);					// DVST Clear
		switch (int_sts0 & DVSQ) {
		case DS_POWR:								// Power state
			break;
		case DS_DFLT:								// Default state
			P_usb_busreset();
			break;
		case DS_ADDS:								// Address state
			break;
		case DS_CNFG:								// Configured state
			break;
		case DS_SPD_POWR:							// Power	  Suspend state
		case DS_SPD_DFLT:							// Default	  Suspend state
		case DS_SPD_ADDR:							// Address	  Suspend state
		case DS_SPD_CNFG:							// Configured Suspend state
			P_usb_suspend();
			break;
		default:									// error
			break;
		}
	}

	// Processing PIPE0 data
	else if ((int_sts0 & BEMP) && (int_enb0 & BEMPE) && ((int_sts3 & int_enb4) & (BitSet[PIPE0]))) {
		P_BEMP_int(int_sts3, int_enb4);
	}
	else if ((int_sts0 & NRDY) && (int_enb0 & NRDYE) && ((int_sts2 & int_enb3) & (BitSet[PIPE0]))) {
		P_INTN_int(int_sts2, int_enb3);
	}
	else if ((int_sts0 & BRDY) && (int_enb0 & BRDYE) && ((int_sts1 & int_enb2) & (BitSet[PIPE0]))) {
		P_INTR_int(int_sts1, int_enb2);
	}

	else if ((int_sts0 & CTRT) && (int_enb0 & CTRE)) {
		USBRD(INTSTS0, int_sts0);					// CTSQ read
		USB_CLR_STS(INTSTS0, CTRT);					// CTRT clear
		switch (int_sts0 & CTSQ) {
		case CS_IDST:								// idle or setup stage
			if (TestModeFlag == YES) {
				// Test Mode
				P_usb_testmode();
			}
			switch (ReqTypeType) {
			case STANDARD:							// standard request
				(*pbRequest0[ReqRequest])(ReqTypeRecip, ReqValue, ReqIndex, ReqLength);
				break;
			case CLASS:								// class request
				P_Class0(ReqType, ReqRequest, ReqValue, ReqIndex, ReqLength);
				break;
			case VENDOR:							// user request
				P_Vendor0(ReqType, ReqRequest, ReqValue, ReqIndex, ReqLength);
				break;
			default:								// request error
				break;
			}
			break;
		case CS_RDDS:								// control read data stage
			P_saveRequest();						// save request register
			switch (ReqTypeType) {
			case STANDARD:							// standard request
				(*pbRequest1[ReqRequest])(ReqTypeRecip, ReqValue, ReqIndex, ReqLength);
				break;
			case CLASS:								// class request
				P_Class1(ReqType, ReqRequest, ReqValue, ReqIndex, ReqLength);
				break;
			case VENDOR:							// user request
				P_Vendor1(ReqType, ReqRequest, ReqValue, ReqIndex, ReqLength);
				break;
			default:								// request error
				break;
			}
			break;
		case CS_WRDS:								// control write data stage
			P_saveRequest();						// save request register
			switch (ReqTypeType) {
			case STANDARD:							// standard request
				(*pbRequest2[ReqRequest])(ReqTypeRecip, ReqValue, ReqIndex, ReqLength);
				break;
			case CLASS:								// calss request
				P_Class2(ReqType, ReqRequest, ReqValue, ReqIndex, ReqLength);
				break;
			case VENDOR:							// user request
				P_Vendor2(ReqType, ReqRequest, ReqValue, ReqIndex, ReqLength);
				break;
			default:								// request error
				break;
			}
			break;
		case CS_WRND:								// control write nodata status stage
			P_saveRequest();						// save request register
			switch (ReqTypeType) {
			case STANDARD:							// standard request
				(*pbRequest3[ReqRequest])(ReqTypeRecip, ReqValue, ReqIndex, ReqLength);
				USB_SET_PAT(DCPCTR, CCPL);			// set CCPL bit
				break;
			case CLASS:								// class request
				P_Class3(ReqType, ReqRequest, ReqValue, ReqIndex, ReqLength);
				break;
			case VENDOR:							// user request
				P_Vendor3(ReqType, ReqRequest, ReqValue, ReqIndex, ReqLength);
				break;
			default:								// request error
				break;
			}
			break;
		case CS_RDSS:								// control read status stage
			switch (ReqTypeType) {
			case STANDARD:							// standard request
				(*pbRequest4[ReqRequest])(ReqTypeRecip, ReqValue, ReqIndex, ReqLength);
				break;
			case CLASS:								// class request
				P_Class4(ReqType, ReqRequest, ReqValue, ReqIndex, ReqLength);
				break;
			case VENDOR:							// user request
				P_Vendor4(ReqType, ReqRequest, ReqValue, ReqIndex, ReqLength);
				break;
			default:								// request error
				break;
			}
			USB_SET_PAT(DCPCTR, CCPL);				// set CCPL bit
			break;
		case CS_WRSS:								// control write status stage
			switch (ReqTypeType) {
			case STANDARD:							// standard request
				(*pbRequest5[ReqRequest])(ReqTypeRecip, ReqValue, ReqIndex, ReqLength);
				break;
			case CLASS:								// class request
				P_Class5(ReqType, ReqRequest, ReqValue, ReqIndex, ReqLength);
				break;
			case VENDOR:							// user request
				P_Vendor5(ReqType, ReqRequest, ReqValue, ReqIndex, ReqLength);
				break;
			default:								// request error
				break;
			}
			USB_SET_PAT(DCPCTR, CCPL);				// set CCPL bit
			break;
		case CS_SQER:								// control sequence error
			SetSTALL(PIPE0);
			break;
		default:									// illegal
			SetSTALL(PIPE0);
			break;
		}
	}

	// Processing whithout PIPE0 data
	else if ((int_sts0 & BEMP) && (int_enb0 & BEMPE) && (int_sts3 & int_enb4)) {
		P_BEMP_int(int_sts3, int_enb4);
	}
	else if ((int_sts0 & NRDY) && (int_enb0 & NRDYE) && (int_sts2 & int_enb3)) {
		P_INTN_int(int_sts2, int_enb3);
	}
	else if ((int_sts0 & BRDY) && (int_enb0 & BRDYE) && (int_sts1 & int_enb2)) {
		P_INTR_int(int_sts1, int_enb2);
	}
}


/**
 * P_saveRequest - save request register
 * @return None
 */
void P_saveRequest(void)
{
	U16	buf;

	USB_CLR_STS(INTSTS0, VALID);					// VALID clear
	USBRD(USBREQ, buf);
	ReqType      = (buf & USBREQ_bmRequestType);
	ReqTypeType  = (buf & USBREQ_bmRequestTypeType);
	ReqTypeRecip = (buf & USBREQ_bmRequestTypeRecip);
	ReqRequest   = ((buf & USBREQ_bRequest) >> 8);
	USBRD(USBVAL,  ReqValue);
	USBRD(USBINDX, ReqIndex);
	USBRD(USBLENG, ReqLength);
}
