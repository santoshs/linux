/*	CPU_Register.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef _CPU_REGISTER_H_
#define _CPU_REGISTER_H_

#ifdef __cplusplus
extern "C" {
#endif

#define USB_BASE_REG				0xE6890000			// The address at the head of USB high speed module (USBHS)
#define USB_DMAC_BASE_REG			0xE68A0000

// #define SYSSTS   					(*((volatile ushort *)(USB_BASE_REG + 0004)))
#define SYSSTS_VBUSST_MASK 			(0xC000)
#define USB_SPADDR					((volatile ushort *)(USB_BASE_REG + 0x0138)) /*H'E689 0138*/
#define USB_SPWDAT					((volatile ushort *)(USB_BASE_REG + 0x013A)) /*H'E689 013A*/
#define USB_SPCTRL					((volatile ushort *)(USB_BASE_REG + 0x013C)) /*H'E689 013C*/
#define USB_SPRDAT					((volatile ushort *)(USB_BASE_REG + 0x013E)) /*H'E689 013E*/
#define USB_SPWR					0x0001
#define USB_SPRD				0x0002

/* ULPI I/F */
#define PORT203CR					*((volatile uchar *)0xE60520CB)	// for ULPI_DATA0
#define PORT204CR					*((volatile uchar *)0xE60520CC)	// for ULPI_DATA1
#define PORT205CR					*((volatile uchar *)0xE60520CD)	// for ULPI_DATA2
#define PORT206CR					*((volatile uchar *)0xE60520CE)	// for ULPI_DATA3
#define PORT207CR					*((volatile uchar *)0xE60520CF)	// for ULPI_DATA4
#define PORT208CR					*((volatile uchar *)0xE60520D0)	// for ULPI_DATA5
#define PORT209CR					*((volatile uchar *)0xE60520D1)	// for ULPI_DATA6
#define PORT210CR					*((volatile uchar *)0xE60520D2)	// for ULPI_DATA7

#define PORT211CR					*((volatile uchar *)0xE60520D3)	// for ULPI_CLK
#define PORT212CR					*((volatile uchar *)0xE60520D4)	// for ULPI_STP
#define PORT213CR					*((volatile uchar *)0xE60520D5)	// for ULPI_DIR
#define PORT214CR					*((volatile uchar *)0xE60520D6)	// for ULPI_NXT
#define PORT217CR					*((volatile uchar *)0xE60520D9)	// for VIO_CK03

#define PORT130CR					*((volatile uchar *)0xE6051082)	// for TUSB_CS
#define PORT131CR					*((volatile uchar *)0xE6051083)	// for nTUSB_RST

/* GPIO */
#define PORT_F1						0x01		// Pull up/down OFF, Input/Output enable, Function 1
#define PORT_PDOE_F0				0x90		// Pull down, Output enable, Function 0
#define PORT_PUOE_F0				0xD0		// Pull up, Output enable, Function 0
#define PORT_OE_F0					0x10		//Pull up/down OFF, output enable, Function 0
#define PORT_OE_F1					0x11		// Pull up/down OFF, Output enable, Function 1

#define MSTPSR3						(*(volatile ulong *)0xE6150048)
#define PHYFUNCTR_USB   			(*(volatile ushort *)0xE6890104)
#define SUSMON						0x4000
#define PRESET						0x2000
#define CKSTP						0x00000100
#define	INT_ENABLE					0x011FFFB1 // USBDMA_CHCR0 enable interrupt
#define TS							0x000000C0 // B7, b6

#define USBDMA_CHCR0_USB			(*(volatile long *)0xE68A0034)
#define	D0FIFOSEL_USB				(*(volatile ushort *)0xE6890028)
#define	CFIFOSEL_USB				(*(volatile ushort *)0xE6890020)

#define SMTP322 					0x00400000			// Module Stop 322 (Controls clock supply to USB)
#define SRT322						0x00400000			// Software Reset bit 322 (Issues the reset to USB)
#define MSTP214						0x00004000

#define CMNCR       				(volatile ulong*)0xFEC10000
#define BSC_RD_N   					(volatile uchar*)0xE60520E0  	/* GPIO PORT224CR */
#define BSC_WE0_N   				(volatile uchar* )0xE60520E1  	/* GPIO PORT225CR */
#define BSC_WE1_N   				( ( volatile uchar* )0xE60520E2 )  /* GPIO PORT226CR */
#define BSC_CS4_N   				( ( volatile uchar* )0xE60520E5 )  /* GPIO PORT229CR */
#define BSC_WAIT_N 					 ( ( volatile uchar* )0xE60520E6 )  /* GPIO PORT230CR */
#define BSC_RDWR    				( ( volatile uchar* )0xE60520E7 )  /* GPIO PORT231CR */
#define BSC_D15     				( ( volatile uchar* )0xE60520E8 )  /* GPIO PORT232CR */
#define BSC_D14     				( ( volatile uchar* )0xE60520E9 )  /* GPIO PORT233CR */
#define BSC_D13     				( ( volatile uchar* )0xE60520EA )  /* GPIO PORT234CR */
#define BSC_D12     				( ( volatile uchar* )0xE60520EB )  /* GPIO PORT235CR */
#define BSC_D11     				( ( volatile uchar* )0xE60520EC )  /* GPIO PORT236CR */
#define BSC_D10     				( ( volatile uchar* )0xE60520ED )  /* GPIO PORT237CR */
#define BSC_D9      				( ( volatile uchar* )0xE60520EE )  /* GPIO PORT238CR */
#define BSC_D8      				( ( volatile uchar* )0xE60520EF )  /* GPIO PORT239CR */
#define BSC_D7      				( ( volatile uchar* )0xE60520F0 )  /* GPIO PORT240CR */
#define BSC_D6      				( ( volatile uchar* )0xE60520F1 )  /* GPIO PORT241CR */
#define BSC_D5      				( ( volatile uchar* )0xE60520F2 )  /* GPIO PORT242CR */
#define BSC_D4      				( ( volatile uchar* )0xE60520F3 )  /* GPIO PORT243CR */
#define BSC_D3      				( ( volatile uchar* )0xE60520F4 )  /* GPIO PORT244CR */
#define BSC_D2      				( ( volatile uchar* )0xE60520F5 )  /* GPIO PORT245CR */
#define BSC_D1      				( ( volatile uchar* )0xE60520F6 )  /* GPIO PORT246CR */
#define BSC_D0      				( ( volatile uchar* )0xE60520F7 )  /* GPIO PORT247CR */
#define BSC_CKO     				( ( volatile uchar* )0xE60520F8 )  /* GPIO PORT248CR */
#define BSC_A10     				( ( volatile uchar* )0xE60520F9 )  /* GPIO PORT249CR */
#define BSC_A9      				( ( volatile uchar* )0xE60520FA )  /* GPIO PORT250CR */
#define BSC_A8      				( ( volatile uchar* )0xE60520FB )  /* GPIO PORT251CR */
#define BSC_A7      				( ( volatile uchar* )0xE60520FC )  /* GPIO PORT252CR */
#define BSC_A6      				( ( volatile uchar* )0xE60520FD )  /* GPIO PORT253CR */
#define BSC_A5      				( ( volatile uchar* )0xE60520FE )  /* GPIO PORT254CR */
#define BSC_A4      				( ( volatile uchar* )0xE60520FF )  /* GPIO PORT255CR */
#define BSC_A3      				( ( volatile uchar* )0xE6052100 )  /* GPIO PORT256CR */
#define BSC_A2      				( ( volatile uchar* )0xE6052101 )  /* GPIO PORT257CR */
#define BSC_A1      				( ( volatile uchar* )0xE6052102 )  /* GPIO PORT258CR */
#define BSC_A0      				( ( volatile uchar* )0xE6052103 )  /* GPIO PORT259CR */

/* =========================================================================== */
/* Base Address                                                                */
/* =========================================================================== */

#define PORTD159_128DSR				*((volatile ulong *)0xE6055104)	// for setting PORT130CR/PORT131CR 
#define PORTD159_128DSR_ES2			*((volatile ulong *)0xE6055100)	
#define PORTD159_128DCR				*((volatile ulong *)0xE6055204)	// for setting PORT130CR/PORT131CR 
#define PORTD159_128DCR_ES2			*((volatile ulong *)0xE6055200)	
#define PORTD159_128DR				*((volatile ulong *)(0xE6055004))
#define PORTD159_128DR_ES2			*((volatile ulong *)(0xE6055000))


#define PORT131_TUSB_RST			0x00000008							// PORT131 (nTUSB_RST)
#define PORT130_TUSB_CS				0x00000004							// PORT130 (TUSB_CS)
#define VBUS_VALID					0x0006 /* SeddValid and Vbusvalid of MPINTER */

/****************************************************************/
/*        CPGA                                                  */
/****************************************************************/
#define SMSTPCR2					(*(volatile ulong  *)0xE6150138)
#define SMSTPCR3					(*(volatile ulong  *)0xE615013C)
#define MSTPSR3						(*(volatile ulong  *)0xE6150048)
#define SRCR3						(*(volatile ulong  *)0xE61580B8)	// Software Reset Register 3

#define VCLKCR3         			(*((volatile ulong *)0xE6150014))
#define FRQCRB_KICK        			0x80000000
#define FRQCRD_DDRPCHG          	0x80000000
#define VCLKCR_EXSRC_MAIN       	0x00006000
#define VCLKCR_CKSTP            	0x00000100
#define CLK_26MHz        			0x00006100

#define PHYFUNCTR_SUSMON        	0x4000
#define PHYFUNCTR_PRESET        	0x2000
#define PHYFUNCTR_TERMSEL       	0x0400
#define PHYFUNCTR_XCVRSEL       	0x0100
#define PHYFUNCTR_RESET_DATA    	0x6500

#ifdef __cplusplus
extern }
#endif
#endif
