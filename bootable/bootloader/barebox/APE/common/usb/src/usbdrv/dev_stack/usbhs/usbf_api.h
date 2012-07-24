/* usbf_api.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef __USBF_API_H__
#define __USBF_API_H__

#include "c_typedef.h"
#include "c_defusr.h"
#include "c_macusr.h"
#include "c_libassp.h"
#include "c_extern.h"

// define
#define USBF_NULL ((void *)0)
#define USBF_TRUE 1
#define USBF_FALSE 0

// error code
#define    USBF_E_OK        0          // success / data is valid

// Call Back Function
typedef long (*USBF_CB)(int id, void *);

typedef struct{
    unsigned char *devdesc;
    unsigned char *qualdesc;
    unsigned char *configdesc_hi;
    unsigned char *configdesc_full;
    unsigned char *strdesc[ USB_STR_DESC_IDX_MAX ];
} TBL_USBF_DESC;

typedef struct{
    TBL_USBF_DESC desctbl;
    unsigned short *epcfg;
    unsigned short epcfgsize;
    unsigned short *pipe2epnum;
    unsigned short *epnum2pipe;
} TBL_USBF_REGEP;

// external function/variable prototype
extern USBF_CB g_UsbFCallback;

// global function/variable prototype
long UsbF_Init( USBF_CB func, int useHiSpeed );
long UsbF_Term( void );
long UsbF_RegistEP( TBL_USBF_REGEP *regData );
long UsbF_GetSpeed( unsigned char *speed );
long UsbF_SetDp( int pullup );
long UsbF_OUTStart( unsigned char epnum, long size, char *buff );
long UsbF_INStart( unsigned char epnum, long size, char *buff, int nullflg );
long UsbF_ClearFIFO( unsigned char epnum );
long UsbF_SetStall( unsigned char epnum, unsigned char status );
long UsbF_SetTrnEnable( unsigned char epnum, unsigned char enable);
long UsbF_SetUSBMask( unsigned long status );
void UsbF_Inthdr( void );
long UsbF_CallBack( int id, void *param );

/*
 * ============================================================================
 *  for Call Back
 */
// id
typedef enum{
	// usb status
    CBID_VBUS,
    CBID_BUSRESET,
    CBID_SUSPEND,
    CBID_RESUME,

	// ctl tran
    CBID_SETCFG,
    CBID_SETIF,
    CBID_CHG_CTSQ_IDST,
    CBID_CHG_CTSQ_RDDS,
    CBID_CHG_CTSQ_RDSS,
    CBID_CHG_CTSQ_WRDS,
    CBID_CHG_CTSQ_WRSS,
    CBID_CHG_CTSQ_WRND,

	// data tran
    CBID_OUT_CPL,
    CBID_OUT_REQ_BUF,
    CBID_IN_CPL,

    CBID_MAX
} TBL_CBID;

// parameter type
typedef struct{
    unsigned short pipenum;      // driver reserv
    unsigned char epnum;
    void *param_data;
} TBL_USBF_PARAM;

typedef struct {
    unsigned char bmRequestType;
    unsigned char bRequest;
    unsigned short wVlue;
    unsigned short wIndex;
    unsigned short wLenght;
} TBL_SETUP_CMD;

typedef struct {
	unsigned short	dtin_cnt ;
    unsigned char **in_buf;
} TBL_IN_DATABUF;

#endif
