/* P_Descrip.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

/************************
 *	Device Descriptor	*
 ************************/
// Standard Device Descriptor
__attribute__ ((aligned(4))) U8	DeviceDescriptor[18];

// Device Qualifier Descriptor
__attribute__ ((aligned(4))) U8	QualifierDescriptor[10];

/************************************************************
 *	Configuration Or Other_Speed_Configuration Descriptor	*
 ************************************************************/
// For Full-Speed
__attribute__ ((aligned(4))) U8	Configuration_Full_1[USB_CFG_DESC_FULL_MAX];

// For Hi-Speed
__attribute__ ((aligned(4))) U8	Configuration_Hi_1[USB_CFG_DESC_HI_MAX];

U8 *ConPtr[] = {
	(U8 *)SOFTWARE_CHANGE,
	(U8 *)SOFTWARE_CHANGE,
	(U8 *)SOFTWARE_CHANGE,
	(U8 *)SOFTWARE_CHANGE,
	(U8 *)SOFTWARE_CHANGE
};

U8 *ConPtr_Other[] = {
	(U8 *)SOFTWARE_CHANGE,
	(U8 *)SOFTWARE_CHANGE,
	(U8 *)SOFTWARE_CHANGE,
	(U8 *)SOFTWARE_CHANGE,
	(U8 *)SOFTWARE_CHANGE
};

U8 *StrPtr[USB_STR_DESC_IDX_MAX];
