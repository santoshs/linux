/* P_ClassVendor.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

/******* include file ********************************************************/
#include "c_typedef.h"
#include "c_defusr.h"


/**
 * P_Class0 - Class request (idle or setup stage)
 * @return None
 */
void P_Class0(U16 Reqtype, U16 Request, U16 Value, U16 Index, U16 Length)
{
	// User program
    TBL_USBF_PARAM param;
    TBL_SETUP_CMD cmd;
    cmd.bmRequestType   = (U8)Reqtype;
    cmd.bRequest        = (U8)Request;
    cmd.wVlue           = Value;
    cmd.wIndex          = Index;
    cmd.wLenght         = Length;
    param.pipenum       = 0;
    param.param_data    = (void *)&cmd;
    UsbF_CallBack( CBID_CHG_CTSQ_IDST, &param );
}


/**
 * P_Class1 - Class request (control read data stage)
 * @return None
 */
void P_Class1(U16 Reqtype, U16 Request, U16 Value, U16 Index, U16 Length)
{
	// User program
    TBL_USBF_PARAM param;
    TBL_SETUP_CMD cmd;
    cmd.bmRequestType   = (U8)Reqtype;
    cmd.bRequest        = (U8)Request;
    cmd.wVlue           = Value;
    cmd.wIndex          = Index;
    cmd.wLenght         = Length;
    param.pipenum       = 0;
    param.param_data    = (void *)&cmd;
    UsbF_CallBack( CBID_CHG_CTSQ_RDDS, &param );
}


/**
 * P_Class2 - Class request (control write data stage)
 * @return None
 */
void P_Class2(U16 Reqtype, U16 Request, U16 Value, U16 Index, U16 Length)
{
	// User program
    TBL_USBF_PARAM param;
    TBL_SETUP_CMD cmd;
    cmd.bmRequestType   = (U8)Reqtype;
    cmd.bRequest        = (U8)Request;
    cmd.wVlue           = Value;
    cmd.wIndex          = Index;
    cmd.wLenght         = Length;
    param.pipenum       = 0;
    param.param_data    = (void *)&cmd;
    UsbF_CallBack( CBID_CHG_CTSQ_WRDS, &param );
}


/**
 * P_Class3 - Class request (control write nodata status stage)
 * @return None
 */
void P_Class3(U16 Reqtype, U16 Request, U16 Value, U16 Index, U16 Length)
{
    long retval;
    // User program
    TBL_USBF_PARAM param;
    TBL_SETUP_CMD cmd;
    cmd.bmRequestType   = (U8)Reqtype;
    cmd.bRequest        = (U8)Request;
    cmd.wVlue           = Value;
    cmd.wIndex          = Index;
    cmd.wLenght         = Length;
    param.pipenum       = 0;
    param.param_data    = (void *)&cmd;
    retval = UsbF_CallBack( CBID_CHG_CTSQ_WRND, &param );

    if( retval == USBF_E_OK ){
        USB_SET_PAT(DCPCTR, CCPL);				/* set CCPL bit */
    }
}


/**
 * P_Class4 - Class request (control read status stage)
 * @return None
 */
void P_Class4(U16 Reqtype, U16 Request, U16 Value, U16 Index, U16 Length)
{
	// User program
    TBL_USBF_PARAM param;
    TBL_SETUP_CMD cmd;
    cmd.bmRequestType   = (U8)Reqtype;
    cmd.bRequest        = (U8)Request;
    cmd.wVlue           = Value;
    cmd.wIndex          = Index;
    cmd.wLenght         = Length;
    param.pipenum       = 0;
    param.param_data    = (void *)&cmd;
    UsbF_CallBack( CBID_CHG_CTSQ_RDSS, &param );
}


/**
 * P_Class5 - Class request (control write status stage)
 * @return None
 */
void P_Class5(U16 Reqtype, U16 Request, U16 Value, U16 Index, U16 Length)
{
	// User program
    TBL_USBF_PARAM param;
    TBL_SETUP_CMD cmd;
    cmd.bmRequestType   = (U8)Reqtype;
    cmd.bRequest        = (U8)Request;
    cmd.wVlue           = Value;
    cmd.wIndex          = Index;
    cmd.wLenght         = Length;
    param.pipenum       = 0;
    param.param_data    = (void *)&cmd;
    UsbF_CallBack( CBID_CHG_CTSQ_WRSS, &param );
}


/**
 * P_Vendor0 - Vendor request (idle or setup stage)
 * @return None
 */
void P_Vendor0(U16 Reqtype, U16 Request, U16 Value, U16 Index, U16 Length)
{
	// User program
    TBL_USBF_PARAM param;
    TBL_SETUP_CMD cmd;
    cmd.bmRequestType   = (U8)Reqtype;
    cmd.bRequest        = (U8)Request;
    cmd.wVlue           = Value;
    cmd.wIndex          = Index;
    cmd.wLenght         = Length;
    param.pipenum       = 0;
    param.param_data    = (void *)&cmd;
    UsbF_CallBack( CBID_CHG_CTSQ_IDST, &param );
}


/**
 * P_Vendor1 - Vendor request (control read data stage)
 * @return None
 */
void P_Vendor1(U16 Reqtype, U16 Request, U16 Value, U16 Index, U16 Length)
{
	// User program
    TBL_USBF_PARAM param;
    TBL_SETUP_CMD cmd;
    cmd.bmRequestType   = (U8)Reqtype;
    cmd.bRequest        = (U8)Request;
    cmd.wVlue           = Value;
    cmd.wIndex          = Index;
    cmd.wLenght         = Length;
    param.pipenum       = 0;
    param.param_data    = (void *)&cmd;
    UsbF_CallBack( CBID_CHG_CTSQ_RDDS, &param );
}


/**
 * P_Vendor2 - Vendor request (control write data stage)
 * @return None
 */
void P_Vendor2(U16 Reqtype, U16 Request, U16 Value, U16 Index, U16 Length)
{
	// User program
    TBL_USBF_PARAM param;
    TBL_SETUP_CMD cmd;
    cmd.bmRequestType   = (U8)Reqtype;
    cmd.bRequest        = (U8)Request;
    cmd.wVlue           = Value;
    cmd.wIndex          = Index;
    cmd.wLenght         = Length;
    param.pipenum       = 0;
    param.param_data    = (void *)&cmd;
    UsbF_CallBack( CBID_CHG_CTSQ_WRDS, &param );
}


/**
 * P_Vendor3 - Vendor request (control write nodata status stage)
 * @return None
 */
void P_Vendor3(U16 Reqtype, U16 Request, U16 Value, U16 Index, U16 Length)
{
    long retval;

	// User program
    TBL_USBF_PARAM param;
    TBL_SETUP_CMD cmd;
    cmd.bmRequestType   = (U8)Reqtype;
    cmd.bRequest        = (U8)Request;
    cmd.wVlue           = Value;
    cmd.wIndex          = Index;
    cmd.wLenght         = Length;
    param.pipenum       = 0;
    param.param_data    = (void *)&cmd;
    retval = UsbF_CallBack( CBID_CHG_CTSQ_WRND, &param );

    if(retval == USBF_E_OK){
        USB_SET_PAT(DCPCTR, CCPL);				/* set CCPL bit */
    }
}


/**
 * P_Vendor4 - Vendor request (control read status stage)
 * @return None
 */
void P_Vendor4(U16 Reqtype, U16 Request, U16 Value, U16 Index, U16 Length)
{
	// User program
    TBL_USBF_PARAM param;
    TBL_SETUP_CMD cmd;
    cmd.bmRequestType   = (U8)Reqtype;
    cmd.bRequest        = (U8)Request;
    cmd.wVlue           = Value;
    cmd.wIndex          = Index;
    cmd.wLenght         = Length;
    param.pipenum       = 0;
    param.param_data    = (void *)&cmd;
    UsbF_CallBack( CBID_CHG_CTSQ_RDSS, &param );
}


/**
 * P_Vendor5 - Vendor request (control write status stage)
 * @return None
 */
void P_Vendor5(U16 Reqtype, U16 Request, U16 Value, U16 Index, U16 Length)
{
	// User program
    TBL_USBF_PARAM param;
    TBL_SETUP_CMD cmd;
    cmd.bmRequestType   = (U8)Reqtype;
    cmd.bRequest        = (U8)Request;
    cmd.wVlue           = Value;
    cmd.wIndex          = Index;
    cmd.wLenght         = Length;
    param.pipenum       = 0;
    param.param_data    = (void *)&cmd;
    UsbF_CallBack( CBID_CHG_CTSQ_WRSS, &param );
}
