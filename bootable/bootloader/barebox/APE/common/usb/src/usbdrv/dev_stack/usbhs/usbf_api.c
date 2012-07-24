/* usbf_api.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "usbf_api.h"

// global function/variable prototype
USBF_CB g_UsbFCallback = USBF_NULL;

// local function/variable prototype
static unsigned char g_pipe2epnum[MAX_PIPE_NO+1];      // PIPE number to EP number
static unsigned short g_epnum2pipe[MAX_PIPE_NO+1];     // EP number to PIPE number

static void delay_10us( void );


/**
 * UsbF_Init
 * @return USBF_E_OK
 */
long UsbF_Init( USBF_CB func, int useHiSpeed )
{
    g_UsbFCallback = func;

    // SCKE set
    USB_SET_PAT(SYSCFG, SCKE);      // SCLK Enable

    // USBE set
    USB_CLR_PAT(SYSCFG, USBE);      // USBE DisEnable
    USB_SET_PAT(SYSCFG, USBE);      // USBE Enable

    // HSE set
    if (useHiSpeed == USBF_TRUE) {
        USB_SET_PAT(SYSCFG,  HSE);  // Hi-Speed Mode
    }
	// for DMA
	USB_SET_PAT(USBDMA_DMAOR, DME);
    return (USBF_E_OK);
}


/**
 * UsbF_Term
 * @return USBF_E_OK
 */
long UsbF_Term( void )
{
    g_UsbFCallback = USBF_NULL;

    USB_CLR_PAT(SYSCFG, USBE);

    stopClock( );               // SCKE clear

    return (USBF_E_OK);
}


/**
 * UsbF_RegistEP
 * @return USBF_E_OK
 */
long UsbF_RegistEP( TBL_USBF_REGEP *regData )
{
    int i;
    TBL_USBF_DESC *pdesc;

    // pipe configation
    for (i=0; i<regData->epcfgsize; i++) {
        EPtbl_1[i] = regData->epcfg[i];
    }

    // discriptor backup
    pdesc = &(regData->desctbl);
    for (i=0; i<pdesc->devdesc[0]; i++) {
        DeviceDescriptor[i] = pdesc->devdesc[i];
    }
    if (pdesc->qualdesc) {
        for (i=0; i<pdesc->qualdesc[0]; i++) {
            QualifierDescriptor[i] = pdesc->qualdesc[i];
        }
    }
    if (pdesc->configdesc_hi) {
        for (i=0; i< (pdesc->configdesc_hi[3]<<8 |pdesc->configdesc_hi[2]); i++) {
            Configuration_Hi_1[i] = pdesc->configdesc_hi[i];
        }
    }
    for (i=0; i< (pdesc->configdesc_full[3]<<8 |pdesc->configdesc_full[2]); i++) {
        Configuration_Full_1[i] = pdesc->configdesc_full[i];
    }
    for (i=0; i<USB_STR_DESC_IDX_MAX; i++) {
        if (pdesc->strdesc[i] == USBF_NULL) {
            continue;
        }
        StrPtr[i] = pdesc->strdesc[i];
    }

    // PIPE <-> EP table
    for (i=0; i<=MAX_PIPE_NO; i++) {
        g_pipe2epnum[i] = regData->pipe2epnum[i];
        g_epnum2pipe[i] = regData->epnum2pipe[i];
    }

    return (USBF_E_OK);
}


/**
 * UsbF_RegistEP
 * @return USBF_E_OK
 */
long UsbF_GetSpeed( unsigned char *speed )
{
    if (isHiSpeed( )) {
        *speed = USBF_TRUE;
    }
	else {
        *speed = USBF_FALSE;
    }

    return (USBF_E_OK);
}


/**
 * UsbF_SetDp
 * @return USBF_E_OK
 */
long UsbF_SetDp( int pullup )
{
    if (pullup == USBF_FALSE) {
		// (1) DPRPU = 0
        USB_CLR_PAT(SYSCFG, DPRPU);
		// (2) 1us(1000ns) above wait
		delay_10us();
		// (3) DCFM = 1
        USB_SET_PAT(SYSCFG, DCFM);
		// (4) 200ns above wait
		delay_10us();
		// (5) DCFM = 0
        USB_CLR_PAT(SYSCFG, DCFM);
    }
	else {
        USB_SET_PAT(SYSCFG, DPRPU);
    }

    return (USBF_E_OK);
}


/**
 * UsbF_OUTStart
 * @return USBF_E_OK
 */
long UsbF_OUTStart( unsigned char epnum, long size, char *buff )
{
    unsigned short pipenum;
    epnum = epnum & ~0x80;

    pipenum = g_epnum2pipe[epnum];

    if (pipenum == 0) {
        // DCP Transfer
        P_CW_Start(size, (U8*)buff);
    }
	else {
        // EP1 .. EP15 Tranfer
        Receive_Start(pipenum, size, (U8*)buff);
    }
    return (USBF_E_OK);
}


/**
 * UsbF_OUTStart
 * @return USBF_E_OK
 */
long UsbF_INStart( unsigned char epnum, long size, char *buff, int nullflg )
{
    unsigned short pipenum;
    epnum = epnum & ~0x80;

    pipenum = g_epnum2pipe[epnum];

    if (pipenum == 0) {
        // DCP Transfer
        P_CR_Start(size, (U8*)buff);
    }
	else {
        // EP1 .. EP15 Tranfer
        Send_Start(pipenum, size, (U8*)buff);
    }
    return (USBF_E_OK);
}


/**
 * UsbF_ClearFIFO
 * @return USBF_E_OK
 */
long UsbF_ClearFIFO( unsigned char epnum )
{
    unsigned short pipenum;

	if (epnum == 0xFE) {
		// FIFO All Clear
		for (pipenum = PIPE0; pipenum <= MAX_PIPE_NO; pipenum++) {
			// pipe 0~9 clear
		    C_FIFOCLR( pipenum );
		}
	}
	else {
	    epnum = epnum & ~0x80;

    	pipenum = g_epnum2pipe[epnum];

	    C_FIFOCLR( pipenum );
	}
    return (USBF_E_OK);
}


/**
 * UsbF_SetStall
 * @return USBF_E_OK
 */
long UsbF_SetStall( unsigned char epnum, unsigned char status )
{
    unsigned short pipenum;
    epnum = epnum & ~0x80;

    pipenum = g_epnum2pipe[epnum];

    if (status == USBF_FALSE) {
        ClrSTALL( pipenum );
    }
	else {
        SetSTALL( pipenum );
    }
    return (USBF_E_OK);
}


/**
 * UsbF_SetTrnEnable
 * @return USBF_E_OK
 */
long UsbF_SetTrnEnable( unsigned char epnum, unsigned char enable)
{
    unsigned short pipenum, buffer;
    epnum = epnum & ~0x80;

    pipenum = g_epnum2pipe[epnum];

    USBRD(PIPECFG, buffer);
    if ((buffer & DIR) == 0) {
        // out
        if (enable == USBF_FALSE) {
            SetNAK( pipenum );
        }
    	else {
            SetBUF( pipenum );
        }
    }
	else {
        // in
        // nouse
    }

    return (USBF_E_OK);
}


/**
 * UsbF_SetUSBMask
 * @return USBF_E_OK
 */
long UsbF_SetUSBMask( unsigned long status )
{
    unsigned short buffer = (unsigned short)status;
    USBWR(INTENB0, buffer);
    return (USBF_E_OK);
}


/**
 * UsbF_SetUSBMask
 * @return None
 */
void UsbF_Inthdr( void )
{
    usbint( );
}


/**
 * UsbF_CallBack
 * @return callback function return code(USBF_E_OK or NOT)
 */
long UsbF_CallBack( int id, void *param )
{
    // pipenum -> epnum overwrite
    TBL_USBF_PARAM *tmp = (TBL_USBF_PARAM *)param;
    tmp->epnum = g_pipe2epnum[tmp->pipenum];

    return g_UsbFCallback(id, tmp );
}


/**
 * delay_10us
 * @return None
 */
static void delay_10us( void )
{
	volatile U32	i=USB_CNT_10us;	
	
	// Wait 10us (Please change for your MCU)
	while(i--);
}
