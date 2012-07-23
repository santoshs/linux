/*	usb_comm.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef __USB_COMM_H__
#define __USB_COMM_H__


// Interface Number
#define COMM_INTERFACE_NUM					2

// comm class request bRequest
#define COMM_REQ_SEND_ENCAPSULATED_COM		0x00	// Abstract, Ethernet Networking, ATM Networking
#define COMM_REQ_SET_COMM_FEATURE			0x02	// Abstract, Telephone
#define COMM_REQ_CLEAR_COMM_FEATURE			0x04	// Abstract, Telephone
#define COMM_REQ_SET_LINE_CODING			0x20	// Abstract
#define COMM_REQ_GET_LINE_CODING			0x21	// Abstract
#define COMM_REQ_SET_CONTROL_LINE_STATE		0x22	// Abstract
#define COMM_REQ_SEND_BREAK					0x23	// Abstract

// Line Coding Structure for SET_LINE_CODING, GET_LINE_CODING
typedef struct _st_LINE_CODE {
	ULONG	dwDTERate;		// Number Data terminal rate, in bits per second.
	UCHAR	bCharFormat;	// Number top bits
							//	0 - 1 Stop bit
							//	1 - 1.5 Stop bits
							//	2 - 2 Stop bits
	UCHAR	bParityType;	// Number Parity
							//	0 - None
							//	1 - Odd
							//	2 - Even
							//	3 - Mark
							//	4 - Space
	UCHAR	bDataBits;		// Number Data bits (5, 6, 7, 8 or 16).
} st_LINE_CODE, *pst_LINE_CODE;

#endif /* __USB_COMM_H__ */
