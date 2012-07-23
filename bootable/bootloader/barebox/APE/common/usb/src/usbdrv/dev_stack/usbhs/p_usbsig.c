/* P_UsbSig.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

/******* include file ********************************************************/
#include "c_typedef.h"
#include "c_defusr.h"


/**
 * P_usb_busreset - USB bus reset
 * @return None
 */
void P_usb_busreset(void)
{
    TBL_USBF_PARAM param;

	C_MemryClear();									// memory clear

	if (isHiSpeed() == YES)
		P_resetDeviceDescriptor(HI_SPEED);			// Device Descriptor reset
	else
		P_resetDeviceDescriptor(FULL_SPEED);		// Device Descriptor reset
	P_resetDCP();									// Default Control PIPE reset

	// User program
    param.pipenum       = 0;
    param.param_data    = USBF_NULL;
    UsbF_CallBack( CBID_BUSRESET, &param );
}


/**
 * P_usb_resume - USB resume
 * @return None
 */
void P_usb_resume(void)
{
    TBL_USBF_PARAM param;

	RESM_StsClear();								// RESM Status Clear
	USB_CLR_PAT(INTENB0, RSME);						// RESM Interrupt Disable

	// User program
    param.pipenum       = 0;
    param.param_data    = USBF_NULL;
    UsbF_CallBack( CBID_RESUME, &param );
}


/**
 * P_usb_suspend - USB suspend
 * @return None
 */
void P_usb_suspend(void)
{
    TBL_USBF_PARAM param;

	// User program
    param.pipenum       = 0;
    param.param_data    = USBF_NULL;
    UsbF_CallBack( CBID_SUSPEND, &param );
}


/**
 * P_usb_testmode - USB testmode (Set_Feature)
 * @return None
 */
void P_usb_testmode(void)
{
	switch (TestModeSelectors & TEST_SELECT) {
	case TEST_J:
	case TEST_K:
	case TEST_SE0_NAK:
	case TEST_PACKET:
		USB_MDF_PAT(TESTMODE, (TestModeSelectors >> 8), UTST);
		break;
	case TEST_FORCE_ENABLE:
	default:
		break;
	}
}
