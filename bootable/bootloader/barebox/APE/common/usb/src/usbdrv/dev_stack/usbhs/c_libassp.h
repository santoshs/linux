/* c_LibAssp.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */


/********** macro define (COMMON) ********************************************/
#define	ON							(1)
#define	OFF							(0)

#ifndef YES
#define	YES							(1)
#endif

#ifndef NO
#define	NO							(0)
#endif

#define	ERROR						(-1)

/* ----- Pipe Status ----- */
#define	PIPE_IDLE					0x00
#define	PIPE_WAIT					0x01
#define	PIPE_DONE					0x02
#define	PIPE_NORES					0x03
#define	PIPE_STALL					0x04

#define	FULL_SPEED					(0)
#define	HI_SPEED					(1)

#define	FIFO_USE					0x3000
#define	C_FIFO_USE					0x0000


/* FIFO port & access define */
#define	CUSE			(U16)0		// CFIFO CPU transfer
#define	D0USE			(U16)1		// D0FIFO CPU transfer
#define	D0DMA			(U16)2		// D0FIFO DMA transfer
#define	D1USE			(U16)3		// D1FIFO CPU transfer
#define	D1DMA			(U16)4		// D1FIFO DMA transfer


/********** macro define (PERIPHERAL) ****************************************/
/* Configuration Descriptor  DEFINE */
#define	CF_SELF						0x40			// Self Powered
#define	CF_RWUP						0x20			// Remote Wakeup

#define	DEV_NUM_CONFIG				17				// index of bNumConfigurations

#define	ALT_NO						255

#define	EP_ERROR					0xFF
#define	SOFTWARE_CHANGE				0


/* FIFO read / write result */
#define	FIFOERROR	(U16)ERROR		// FIFO not ready
#define	WRITEEND	(U16)0x00		// end of write ( but packet may not be outputting )
#define	WRITESHRT	(U16)0x01		// end of write ( send short packet )
#define	WRITING		(U16)0x02		// write continues
#define	READEND		(U16)0x00		// end of read
#define	READSHRT	(U16)0x01		// insufficient ( receive short packet )
#define	READING		(U16)0x02		// read continues
#define	READOVER	(U16)0x03		// buffer size over
