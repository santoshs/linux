/* P_ChangeEP.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

/******* include file ********************************************************/
#include "c_typedef.h"
#include "c_defusr.h"


/**
 * P_Change_Config - Change configuration
 * @return None
 */
void P_Change_Config(U16 Con_Num)
{
    TBL_USBF_PARAM param;
	// User program
    param.pipenum       = 0;
    param.param_data    = USBF_NULL;
    UsbF_CallBack( CBID_SETCFG, &param );
}


/**
 * P_Change_Interface - Change interface
 * @return None
 */
void P_Change_Interface(U16 Con_Num, U16 Int_Num, U16 Alt_Num)
{
    TBL_USBF_PARAM param;
	// User program
    param.pipenum       = 0;
    param.param_data    = USBF_NULL;
    UsbF_CallBack( CBID_SETIF, &param );
}
