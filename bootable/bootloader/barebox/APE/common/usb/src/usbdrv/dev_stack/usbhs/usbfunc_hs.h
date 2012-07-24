/* usbfunc_hs.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef __USBFUNC_HS_H__
#define __USBFUNC_HS_H__


int DeinitDevUSB_HS(void);
int InitDevUSB_HS(void);
void IntDevUSB_HS(void);
unsigned short ConvertEpToChannel_HS(unsigned char);
int StallEp_HS(unsigned char, BOOL);
int WriteEp_HS(pst_DATA_CALLBACK);
int ReadEp_HS(pst_DATA_CALLBACK);
ER UsbFuncPlugIn_HS(void);
ER UsbFuncPlugOut_HS(void);
int CheckEpStatus_HS(unsigned char, unsigned char *);
int SetDeviceAddress_HS(unsigned char);
int OpenOtherEndpoint_HS(void);

#endif /* __USBFUNC_HS_H__ */
