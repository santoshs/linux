/* usbfunc_hs.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

/*===========================================================================

                     INCLUDE FILES FOR MODULE

===========================================================================*/
#include "usbd_def.h"
#include "usbd_types.h"
#include "usbd_system.h"
#include "usb.h"
#include "dev_api.h"
#include "dev_globals.h"

#include "usbf_api.h"

long UsbHs_CallBack( int id, void *param);

// FIFO_USE

typedef struct _st_EP_CFG {
	unsigned short	pipe_sel ;
	unsigned short	pipe_cfg ;
	unsigned short	pipe_buf ;
	unsigned short	pipe_maxp ;
	unsigned short	pipe_peri ;
} st_EP_CFG, *pst_EP_CFG;

#define USE_PIPE_COUNT		3

void CommAcmGetBulkOut(void);
void UsbDevSetConfigEnd(pst_DATA_CALLBACK pstDataCallbackArg);

void CPU_CONTA_OPORT_ON(void);
void CPU_CONTA_OPORT_OFF(void);
int vbus_check(void);

void TimeWaitLoop(unsigned long count_us);

#define MAX_PACKET_SIZE			512
extern const unsigned char gDeviceQualifier[];
extern const unsigned char gOtherSpeedCfgQualifier[];

UB			writeDummyBuf[MAX_PACKET_SIZE];
int		g_vbus_status ;


/**
 * DeinitDevUSB_HS
 * @return USBF_E_OK
 */
int DeinitDevUSB_HS(void)
{
	long	err ;

	err = UsbF_Term();

	return err ;
}


/**
 * InitDevUSB_HS
 * @return USBF_E_OK
 */
int InitDevUSB_HS(void)
{
	TBL_USBF_REGEP regData ;
	long	err ;
	pst_USB_DEV_GLOBALS gP;		// USB Device Info

	st_EP_CFG	epcfg[USE_PIPE_COUNT] = {{PIPE6 | FIFO_USE , INTR | DIR_P_IN  | EP3 , BUF_SIZE(16) | 0x4 ,16,0},
										 {PIPE1 | FIFO_USE , BULK | DIR_P_IN  | EP1 , BUF_SIZE(MAX_PACKET_SIZE) | 0x6 ,MAX_PACKET_SIZE,0},
										 {PIPE2 | FIFO_USE , BULK | DIR_P_OUT | EP2 , BUF_SIZE(MAX_PACKET_SIZE) | 0x12 ,MAX_PACKET_SIZE,0}};

	unsigned short	pipe2ep[MAX_PIPE_NO] = {0,1,2,0,0,0,3,0,0,0,0,0,0,0,0};
	unsigned short	ep2pipe[MAX_PIPE_NO] = {0,1,2,6,0,0,0,0,0,0,0,0,0,0,0};

	gP   = USB_DEV_GLOBAL();
	g_vbus_status = 0 ;

	UsbF_Init(UsbHs_CallBack, USBF_TRUE);

	regData.desctbl.devdesc = (unsigned char *)&gP->stDeviceDescriptor ;
	regData.desctbl.qualdesc = (unsigned char *)gDeviceQualifier ;
	regData.desctbl.configdesc_hi = gP->pbConfigDescriptor;
	regData.desctbl.configdesc_full = (unsigned char *)gOtherSpeedCfgQualifier;
	regData.desctbl.strdesc[0] = (unsigned char *)&gP->stLanguageID ;
	regData.desctbl.strdesc[1] = gP->pbMStringDescriptor ;
	regData.desctbl.strdesc[2] = gP->pbPStringDescriptor ;
	regData.desctbl.strdesc[3] = gP->pbSStringDescriptor ;
	regData.desctbl.strdesc[4] = NULL ;

	regData.epcfg = (unsigned short*)&epcfg ;
	regData.epcfgsize = USE_PIPE_COUNT * EPL ;
	regData.pipe2epnum = pipe2ep ;
	regData.epnum2pipe = ep2pipe ;

	err = UsbF_RegistEP (&regData);

	UsbF_ClearFIFO(0xFE);	// review 0xFE:FIFO ALL Clear

	UsbF_SetUSBMask(DVSE | CTRE | BEMPE | NRDYE | BRDYE );

	return err ;
}


/**
 * IntDevUSB_HS
 * @return None
 */
void IntDevUSB_HS(void)
{
	int vbus ;

	UsbF_Inthdr();

	vbus = vbus_check() ;
	if (g_vbus_status != vbus) {
		UsbHs_CallBack( CBID_VBUS, NULL );
	}
}


/**
 * ConvertEpToChannel_HS
 * @return Channel_HS
 */
unsigned short ConvertEpToChannel_HS(unsigned char epAddress)
{
	unsigned short		ch;

	ch = (unsigned short)epAddress & 0x0F ;

	return ch ;
}


/**
 * StallEp_HS
 * @return 0
 */
int StallEp_HS(unsigned char bEpAddress, BOOL bFlag)
{
	if (bFlag == TRUE) {
		UsbF_SetStall(bEpAddress,USBF_TRUE);
	}
	else{
		UsbF_SetStall(bEpAddress,USBF_FALSE);
	}

	return 0 ;
}


/**
 * WriteEp_HS
 * @return USBF_E_OK
 */
int WriteEp_HS(pst_DATA_CALLBACK pstDataCB)
{
	long	err ;
	unsigned short	ch ;
	pst_USB_DEV_GLOBALS gP;

	gP = USB_DEV_GLOBAL();
	ch = ConvertEpToChannel_HS(pstDataCB->bEpAddress);

	gP->pvEpManage[ch * 2] = (void *)pstDataCB;

	err = UsbF_INStart(	(unsigned char)pstDataCB->bEpAddress ,
						(long)pstDataCB->wBufferSize ,
						(char*)pstDataCB->pbBuffer,
						1);

	return err ;
}


/**
 * ReadEp_HS
 * @return USBF_E_OK
 */
int ReadEp_HS(pst_DATA_CALLBACK pstDataCB)
{
	long 	err ;
	unsigned short	ch ;
	pst_USB_DEV_GLOBALS gP;

	gP = USB_DEV_GLOBAL();
	ch = ConvertEpToChannel_HS(pstDataCB->bEpAddress);
	gP->pvEpManage[ch * 2 + 1] = (void *)pstDataCB;

	err = UsbF_OUTStart((unsigned char)pstDataCB->bEpAddress ,
						(long)pstDataCB->wBufferSize ,
						(char*)pstDataCB->pbBuffer);

	return err ;
}


/**
 * UsbFuncPlugIn_HS
 * @return 0
 */
ER UsbFuncPlugIn_HS(void)
{
	return 0 ;
}


/**
 * UsbFuncPlugOut_HS
 * @return 0
 */
ER UsbFuncPlugOut_HS(void)
{
	return 0 ;
}


/**
 * CheckEpStatus_HS
 * @return 0
 */
int CheckEpStatus_HS(unsigned char bEpAddress, unsigned char *pbStatus)
{
	return 0 ;
}


/**
 * SetDeviceAddress_HS
 * @return 0
 */
int SetDeviceAddress_HS(unsigned char bDeviceAddress)
{
	return 0 ;
}


/**
 * OpenOtherEndpoint_HS
 * @return USBF_E_OK
 */
int OpenOtherEndpoint_HS(void)
{
	pst_USB_DEV_GLOBALS    	gP;						// USB Device Stack Info
	pst_CONFIG_DESCRIPTOR  	pstConfigDescriptor;	// Config Descriptor
	st_ENDPOINT_DESCRIPTOR 	stEndpointDescriptor;	// EndPoint Descriptor
	unsigned short         	wChannel=0;				// Channel Number
	unsigned short         	wTotalLength;			// Configration Descriptor Total Length
	unsigned char          	*pWork;
	unsigned char          	bLength;
	long					err = 0 ;
	int   length_err ;

	length_err = 0 ;

	gP = USB_DEV_GLOBAL();

	pstConfigDescriptor = (pst_CONFIG_DESCRIPTOR)gP->pbConfigDescriptor;

	MEM_SET_DW(&stEndpointDescriptor, 0, sizeof(st_ENDPOINT_DESCRIPTOR));

	wTotalLength = pstConfigDescriptor->wTotalLength;	// Configration Descriptor Total Length
	pWork        = gP->pbConfigDescriptor;				// Configration Descriptor Head

	// EP Discripta is done in the sense EP and the configuration value
	// corresponding to the address is made
	while (wTotalLength > 0) {
		if (pWork[1] == DESC_TYPE_EP) {
			stEndpointDescriptor.bLength          = pWork[0];
			stEndpointDescriptor.bDescriptorType  = pWork[1];
			stEndpointDescriptor.bEndpointAddress = pWork[2];
			stEndpointDescriptor.bmAttributes     = pWork[3];
			stEndpointDescriptor.wMaxPacketSize   = pWork[4] | (pWork[5] << 8);
			stEndpointDescriptor.bInterval        = pWork[6];

			// The channel number is acquired in order defined in EP Discripta

			// The EP address corresponding to the acquired channel number is
			// registered in the channel number table
			gP->bChannelEpTable[wChannel] = stEndpointDescriptor.bEndpointAddress & 0x0F;

			if ((stEndpointDescriptor.bEndpointAddress & 0x80) == 0) {

				CommAcmGetBulkOut();

				// OUT
				err = UsbF_OUTStart((unsigned char)stEndpointDescriptor.bEndpointAddress & 0x0F,
									(long)stEndpointDescriptor.wMaxPacketSize ,
									(char*)writeDummyBuf);

				UsbF_SetTrnEnable ((unsigned char)stEndpointDescriptor.bEndpointAddress & 0x0F,
									USBF_TRUE);
			}
		}

		bLength       = pWork[0];					// bLengthExtraction
		if (bLength == 0) {
			length_err = 1 ;						// Length Error
			break;
		}
		if (wTotalLength >= (unsigned short)bLength) {
			wTotalLength -= (unsigned short)bLength; 	// Subtraction finished reading descriptor
			pWork        += bLength;     				// Next Descriptor
		}
		else {
			length_err = 1 ;
			break;
		}
	}

	// Set EndPoint Descriptor
	// 0 settings of EP not used

	if (length_err == 1) {
		while (1) {
			// wait WDT
		}
	}
	return err ;
}


/**
 * UsbHs_CallBack
 * @return USBF_E_OK
 */
long UsbHs_CallBack( int id, void *param)
{
	unsigned char 		status ;
	UB					*buf ;
	TBL_USBF_PARAM 		*tmpdata = (TBL_USBF_PARAM *)param ;
	TBL_SETUP_CMD 		*cmd ;
	TBL_IN_DATABUF 		*inbuf;
	unsigned short		ch ;
	pst_USB_DEV_GLOBALS gP;				// USB Device Info
	pst_DATA_CALLBACK	pstDataCB;

	gP   = USB_DEV_GLOBAL();

	switch (id) {
	case CBID_VBUS :
		TimeWaitLoop(100);
		status = vbus_check() ;
		if (status == 1) {
			// Processing of USBCR2
			CPU_CONTA_OPORT_ON();
			// D+ PullUp
			UsbF_SetDp(USBF_TRUE);
		}
		else {
			// USB packet sending stop process
			USB_CLR_PAT(PIPECFG, TYP);
			TimeWaitLoop(100000);
			
			// D+ PullUp relese
			UsbF_SetDp(USBF_FALSE);

			// Processing of USBCR2
			CPU_CONTA_OPORT_OFF();
		}
		g_vbus_status = status ;
		break;
	case CBID_BUSRESET :
		break;
	case CBID_SUSPEND :
		break;
	case CBID_RESUME :
		break;
	case CBID_SETCFG :
		UsbDevSetConfigEnd(NULL);
		break;
	case CBID_SETIF :
		if (tmpdata != NULL) {
			cmd = tmpdata->param_data ;
			gP->bAltSetting      = (unsigned char)cmd->wVlue;
			gP->bInterfaceNumber = (unsigned char)cmd->wIndex;
		}
		break;
	case CBID_CHG_CTSQ_IDST :
		break;
	case CBID_CHG_CTSQ_RDSS :
		if (tmpdata != NULL) {
			cmd = tmpdata->param_data ;
		}
		break;
	case CBID_CHG_CTSQ_RDDS :
	case CBID_CHG_CTSQ_WRDS :
	case CBID_CHG_CTSQ_WRND :
		if (tmpdata != NULL) {
			cmd = tmpdata->param_data ;
			// Processes in case of the class request or the vender request reception
			if (cmd != NULL) {
				(gP->stUsbDevCallback.ClassOrVendorReqHandler)(gP->stUsbDevCallback.pvArg, (pst_DEVICE_REQUEST)cmd);
			}
		}
		break;
	case CBID_CHG_CTSQ_WRSS :
		if (tmpdata != NULL) {
			cmd = tmpdata->param_data ;
		}
		break;
	case CBID_OUT_CPL :
		if (tmpdata != NULL) {
			if (tmpdata->epnum == 2) {
				ch = ConvertEpToChannel_HS(tmpdata->epnum);
				pstDataCB = (pst_DATA_CALLBACK)gP->pvEpManage[ch * 2 + 1];
				gP->pvEpManage[ch * 2 + 1] = NULL;

				if (pstDataCB != NULL) {
					pstDataCB->iStatus = 0 ;

					if (pstDataCB->DataCallback != NULL) {
						(pstDataCB->DataCallback)((void*)pstDataCB);
					}
				}

				UsbF_OUTStart((unsigned char)ch ,
									(long)64 ,
									(char*)writeDummyBuf);
			}
		}
		break;
	case CBID_OUT_REQ_BUF :
		if (tmpdata != NULL) {
			inbuf = (TBL_IN_DATABUF *)tmpdata->param_data ;
			inbuf->in_buf	= NULL ;

			if (tmpdata->epnum == 2) {
				ch = ConvertEpToChannel_HS(tmpdata->epnum);

				pstDataCB = (pst_DATA_CALLBACK)gP->pvEpManage[ch * 2 + 1];

				if (pstDataCB != NULL) {
					inbuf = (TBL_IN_DATABUF *)tmpdata->param_data ;
					buf = &pstDataCB->pbBuffer[pstDataCB->wXferSize];
					inbuf->in_buf	= &buf ;
					pstDataCB->wXferSize += inbuf->dtin_cnt ;
				}
			}
			else {
				inbuf = (TBL_IN_DATABUF *)tmpdata->param_data ;
				buf = writeDummyBuf ;
				inbuf->in_buf	= &buf ;
			}
		}
		break;
	case CBID_IN_CPL :
		if (tmpdata != NULL) {
			if (tmpdata->epnum != 0) {
				ch = ConvertEpToChannel_HS(tmpdata->epnum);
				pstDataCB = (pst_DATA_CALLBACK)gP->pvEpManage[ch * 2];

				if (pstDataCB != NULL) {
					gP->pvEpManage[ch * 2] = NULL;
					pstDataCB->iStatus = 0 ;

					if (pstDataCB->DataCallback != NULL) {
						(pstDataCB->DataCallback)((void*)pstDataCB);
					}
				}
			}
		}
		break;
	default:
		break;
	}

	return USBF_E_OK ;
}
