/*	usb_phy_get_info.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */
 
#include "usb_api.h"
#include "usb_private.h"
#include "cpu_register.h"

/**
 * usb_phy_read - Read data of TUSB1211 register via PHY
 * @return None
 */
void usb_phy_get_info(USB_INFO *info)
{
	ulong ret;
	ulong val;
	
	val = SYSSTS;
	/* reset value */
	info->usb_type = 0;
	info->aca = 0;
	info->usb_connect = 0;
	
	if ((val & SYSSTS_VBUSST_MASK)==SYSSTS_VBUSST_MASK)  {
		info->usb_connect = 1;
	}
	
	ret = otg_phy_read(VENDOR_SPECIFIC2_STS);
	if(ret & ID_RARBRC_STS){
		info->aca = 1;
	}
	
	ret = otg_phy_read(POWER_CONTROL);
	if((ret & DET_COMP)!=DET_COMP){
		info->usb_type = USB_SDP;
	}
	else{
		ret = otg_phy_read(VENDOR_SPECIFIC4);
		if(ret & CHGD_SERX_DM){
			info->usb_type = USB_DCP;
		}
		else{
			info->usb_type = USB_CDP;
		}
	}
	return;
}

