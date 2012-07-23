/* P_Status.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

/******* include file ********************************************************/
#include "c_typedef.h"
#include "c_defusr.h"


/**
 * P_Csrch - Configuration number
 * @return Number of possible configurations (bNumConfigurations)
 */
U16 P_Csrch(void)
{
	return (U16)DeviceDescriptor[DEV_NUM_CONFIG];
}


/**
 * P_Isrch - Interface number
 * @return Number of this interface (bNumInterfaces)
 */
U16 P_Isrch(U16 Con_Num)
{
	return (U16)(*(ConPtr[Con_Num-1]+4));
}


/**
 * P_Asrch - Alternate setting number
 * @return Value used to select this alternate setting(bAlternateSetting)
 */
U16 P_Asrch(U16 Con_Num, U16 Int_Num)
{
	U16	i;
	U16	alt_num = 0;
	U8	*ptr;
	U16	length;

	ptr = (U8 *)(ConPtr[Con_Num-1]);				// ConfigurationDescriptor[0]
	i = ptr[0];
	ptr += ptr[0];									// InterfaceDescriptor[0]
	length  = (U16)(*(ConPtr[Con_Num-1]+2));
	length |= (U16)(*(ConPtr[Con_Num-1]+3)) << 8;
	for ( ; i < length; ) {							// Search Descriptor Table size
		switch (ptr[1]) {							// Descriptor Type
		case DT_INTERFACE:							// Interface
			if (Int_Num == ptr[2]) {
				alt_num = (U16)ptr[3];				// Alternate Number count
			}
			i += ptr[0];
			ptr += ptr[0];
			break;
		case DT_DEVICE:								// Device
		case DT_CONFIGURATION:						// Configuration
		case DT_STRING:								// String
		case DT_ENDPOINT:							// Endpoint
		default:									// Class, Vendor, else
			i += ptr[0];
			ptr += ptr[0];
			break;
		}
	}
	return alt_num;
}


/**
 * P_Esrch - Endpoint table initialize
 * @return None
 */
void P_Esrch(U16 Con_Num, U16 Int_Num, U16 Alt_Num)
{
	U8		*ptr;
	U16		i, j, length;
	U16		start, numbers, ep;

	ptr = (U8 *)ConPtr[Con_Num-1];					// ConfigurationDescriptor
	i = *ptr;
	length = (U16)*(ptr + 3) << 8 | (U16)*(ptr + 2);
	ptr += *ptr;
	start = 0;
	numbers = 0;
	j = 0;

	for ( ; i < length; ) {
		switch (*(ptr + 1)) {						// Descriptor Type
		case DT_INTERFACE:							// Interface
			if ((*(ptr + 2) == Int_Num) && (*(ptr + 3) == Alt_Num)) {
				numbers = *(ptr + 4);
			}
			else {
				start += *(ptr + 4);
			}
			i += *ptr;
			ptr += *ptr;
			break;
		case DT_ENDPOINT:							// Endpoint
			if (j < numbers) {
				ep = (*(ptr + 2) & 0x0f);
				EPTableIndex[ep] = (start + j);
				++j;
			}
			i += *ptr;
			ptr += *ptr;
			break;
		case DT_DEVICE:								// Device
		case DT_CONFIGURATION:						// Configuration
		case DT_STRING:								// String
		default:									// Class, Vendor, else
			i += *ptr;
			ptr += *ptr;
			break;
		}
	}
}


/**
 * P_RWsrch - Check remote wakeup bit (bmAttributes)
 * @return ON  : Support Remote Wakeup
 *         OFF : not Support Remote Wakeup
 */
U16 P_RWsrch(void)
{
	U8	atr;

	if (ConfigNum == 0)
		return OFF;

	atr = *(ConPtr[ConfigNum - 1] + 7);
	if (atr & CF_RWUP)
		return ON;
	return OFF;
}
