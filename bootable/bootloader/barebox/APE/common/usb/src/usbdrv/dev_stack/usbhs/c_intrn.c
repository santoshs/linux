/* C_IntRN.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

/******* include file ********************************************************/
#include "c_typedef.h"
#include "c_defusr.h"
#include "usb_private.h"

extern unsigned long gRecvDataEndPos;
extern unsigned long gbBulkOutBuffSize;

/**
 * INTR_int_pipe - INTR interrupt
 * @return None
 */
void INTR_int_pipe(U16 Status, U16 Int_enbl)
{
	U16	buffer;
	U8 counter;

    for (counter=PIPE1; counter<=MAX_PIPE_NO; counter++) {
        if ((Status & (BitSet[counter])) && (Int_enbl & (BitSet[counter]))) {
		if (counter==PIPE2) {
        		if (gRecvDataEndPos > (gbBulkOutBuffSize - USB_MAX_DMA_TRANSFER_SIZE)) {
					DisableIntR(PIPE2);	// Disable Ready Interrupt
					return;
				}
        	}
            USB_CLR_STS(BRDYSTS, (BitSet[counter]));
            USB_CLR_STS(BEMPSTS, (BitSet[counter]));		// BEMP Status Clear
            USBWR(PIPESEL, counter);
            USBRD(PIPECFG, buffer);
            if ((buffer & DIR) == 0) {
                if (counter==PIPE2) {
                	DMA_Read();
            	} else {
            		Buf_Read(counter);
            	}
            } else {
                Buf_Write(counter);
            }
        }
    }
}


/**
 * INTN_int_pipe - INTN interrupt
 * @return None
 */
void INTN_int_pipe(U16 Status, U16 Int_enbl)
{
	U16		buffer, bitcheck, i;

	bitcheck	= (U16)(Status & Int_enbl);

	USB_CLR_STS(NRDYSTS, Status);
	for (i = PIPE1; i <= MAX_PIPE_NO; i++) {
		if ((bitcheck&BitSet[i])==BitSet[i]) {				// interrupt check
			buffer	= GetPid(i);
			if ((buffer&PID_STALL)==PID_STALL) {			// STALL
				PipeFlag[i] = PIPE_STALL;
			}
			else {
				PipeIgnore[i]++;							// Ignore count
				if (PipeIgnore[i] == PIPEERR) {
					PipeFlag[i] = PIPE_NORES;
				}
				else {
					SetBUF(i);								// PIPE0 Send IN or OUT token
				}
			}
		}
	}
}


/**
 * BEMP_int_pipe - BEMP interrupt
 * @return None
 */
void BEMP_int_pipe(U16 Status, U16 Int_enbl)
{
	U16	buffer;
    U8 counter;
    TBL_USBF_PARAM param;

    for (counter=PIPE1; counter<=MAX_PIPE_NO; counter++) {
        if ((Status & (BitSet[counter])) && (Int_enbl & (BitSet[counter]))) {
            USB_CLR_STS(BEMPSTS, (BitSet[counter]));
            buffer = GetPid(counter);
            if (buffer & PID_STALL) {
                PipeFlag[counter] = PIPE_STALL;
            }
        	else {
                if ((counter>=PIPE6) && (counter<=PIPE15)) {
                    DisableIntE(counter);					// Disable BEMP Interrupt
                    PipeFlag[counter] = PIPE_DONE;			// End
                    // User program
                    param.pipenum       = counter;
                    param.param_data    = USBF_NULL;
                    UsbF_CallBack( CBID_IN_CPL, &param );
                }
        		else {
                    USBRD((PIPEnCTR(counter)), buffer);
                    if ((buffer & INBUFM) == 0) {
                        DisableIntE(counter);				// Disable BEMP Interrupt
                        PipeFlag[counter] = PIPE_DONE;		// End
                        // User program
                        param.pipenum       = counter;
                        param.param_data    = USBF_NULL;
                        UsbF_CallBack( CBID_IN_CPL, &param );
                    }
                }
            }
        }
    }
}
