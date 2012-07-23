/* C_Def596.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef __C_DEF596__
#define __C_DEF596__
 
/****************************************/
/*		M66596 Register definition		*/
/****************************************/
extern unsigned long	gUSB_BaseAddr ;

// System Configuration Status Register
#define SYSSTS              (*((REGP *)(gUSB_BaseAddr+0x04)))
#define SYSSTS_VBUSST_MASK      (0xC000)
#define SYSSTS_VBUSST_SESSEND   (0x0000)
#define SYSSTS_VBUSST_SESSVALID (0x8000)
#define SYSSTS_VBUSST_VBUSVALID (0xC000)

// System Configuration Control Register
#define	SYSCFG				(*((REGP *)(gUSB_BaseAddr+0x00)))
#define	SCKE				(0x0400)	// b10: USB clock enable
#define	HSE					(0x0080)	// b7: Hi-speed enable
#define	DCFM				(0x0040)	// b6: Controller Function Select(1:host controller)
#define	DPRPU				(0x0010)	// b4: D+ pull up control
#define	USBE				(0x0001)	// b0: USB module operation enable

// Device State Control Register
#define	DVSTCTR				(*((REGP *)(gUSB_BaseAddr+0x08)))
#define	RHST				(0x0003)	// b1-0: Reset handshake status
#define	HSMODE				 0x0003		// Hi-Speed mode

// Test Mode Register
#define	TESTMODE			(*((REGP *)(gUSB_BaseAddr+0x0C)))
#define	UTST				(0x000F)	// b4-0: Test select

// CFIFO/DxFIFO Port Register
#define	CFIFO_32			(*((REGP32 *)(gUSB_BaseAddr+0x14)))
#define	CFIFO				(*((REGP   *)(gUSB_BaseAddr+0x14+2)))
#define	CFIFO_8				(*((REGP8  *)(gUSB_BaseAddr+0x14+1)))
#define	DREQE				(0x1000u)	/* b12: DREQ output enable */

// CFIFO/DxFIFO Port Select Register
#define	CFIFOSEL			(*((REGP *)(gUSB_BaseAddr+0x20)))
#define	D0FIFOSEL			(*(volatile unsigned short *)(gUSB_BaseAddr+0x28))
#define	D1FIFOSEL			(*((REGP *)(gUSB_BaseAddr+0x2C)))
#define	MBW_8				 0x0000		//  8bit
#define	MBW_16				 0x0400		// 16bit
#define	MBW_32				 0x0800		// 32bit
#define	CURPIPE				(0x000F)	// b3-0: PIPE select

// CFIFO/DxFIFO Port Control Register
#define	CFIFOCTR			(*((REGP *)(gUSB_BaseAddr+0x22)))
#define	D0FIFOCTR			(*((REGP *)(gUSB_BaseAddr+0x2A)))
#define	D1FIFOCTR			(*((REGP *)(gUSB_BaseAddr+0x2E)))
#define	BVAL				(0x8000)	// b15: Buffer valid flag
#define	BCLR				(0x4000)	// b14: Buffer clear
#define	FRDY				(0x2000)	// b13: FIFO ready
#define	DTLN				(0x0FFF)	// b11-0: FIFO received data length

// Interrupt Enable Register 0
#define	INTENB0				(*((REGP *)(gUSB_BaseAddr+0x30)))
#define	VBSE				(0x8000)	// b15: VBUS interrupt
#define	RSME				(0x4000)	// b14: Resume interrupt
#define	SOFE				(0x2000)	// b13: Frame update interrupt
#define	DVSE				(0x1000)	// b12: Device state transition interrupt
#define	CTRE				(0x0800)	// b11: Control transfer stage transition interrupt
#define	BEMPE				(0x0400)	// b10: Buffer empty interrupt
#define	NRDYE				(0x0200)	// b9: Buffer not ready interrupt
#define	BRDYE				(0x0100)	// b8: Buffer ready interrupt

// BRDY Interrupt Enable/Status Register
#define	BRDYENB				(*((REGP *)(gUSB_BaseAddr+0x36)))	// Enable
#define	BRDYSTS				(*((REGP *)(gUSB_BaseAddr+0x46)))	// Status

// NRDY Interrupt Enable/Status Register
#define	NRDYENB				(*((REGP *)(gUSB_BaseAddr+0x38)))	// Enable
#define	NRDYSTS				(*((REGP *)(gUSB_BaseAddr+0x48)))	// Status

// BEMP Interrupt Enable/Status Register
#define	BEMPENB				(*((REGP *)(gUSB_BaseAddr+0x3A)))	// Enable
#define	BEMPSTS				(*((REGP *)(gUSB_BaseAddr+0x4A)))	// Status

// Interrupt Status Register 0
#define	INTSTS0				(*((REGP *)(gUSB_BaseAddr+0x40)))
#define	VBINT				(0x8000)	// b15: VBUS interrupt
#define	RESM				(0x4000)	// b14: Resume interrupt
#define	SOFR				(0x2000)	// b13: SOF frame update interrupt
#define	DVST				(0x1000)	// b12: Device state transition interrupt
#define	CTRT				(0x0800)	// b11: Control transfer stage transition interrupt
#define	BEMP				(0x0400)	// b10: Buffer empty interrupt
#define	NRDY				(0x0200)	// b9: Buffer not ready interrupt
#define	BRDY				(0x0100)	// b8: Buffer ready interrupt
#define	DVSQ				(0x0070)	// b6-4: Device state
#define	DS_SPD_CNFG			 0x0070		// Suspend Configured
#define	DS_SPD_ADDR		 	 0x0060		// Suspend Address
#define	DS_SPD_DFLT		 	 0x0050		// Suspend Default
#define	DS_SPD_POWR			 0x0040		// Suspend Powered
#define	DS_CNFG			 	 0x0030		// Configured
#define	DS_ADDS			 	 0x0020		// Address
#define	DS_DFLT			 	 0x0010		// Default
#define	DS_POWR				 0x0000		// Powered
#define	VALID				(0x0008)	// b3: Setup packet detected flag
#define	CTSQ				(0x0007)	// b2-0: Control transfer stage
#define	CS_SQER				 0x0006		// Sequence error
#define	CS_WRND				 0x0005		// Control write nodata status stage
#define	CS_WRSS				 0x0004		// Control write status stage
#define	CS_WRDS				 0x0003		// Control write data stage
#define	CS_RDSS				 0x0002		// Control read status stage
#define	CS_RDDS				 0x0001		// Control read data stage
#define	CS_IDST				 0x0000		// Idle or setup stage

// USB Request Type Register
#define	USBREQ				(*((REGP *)(gUSB_BaseAddr+0x54)))
#define	USBREQ_bRequest				(0xFF00)	// b15-8: bRequest
#define	USBREQ_bmRequestType		(0x00FF)	// b7-0: bmRequestType
#define	USBREQ_bmRequestTypeType	(0x0060)	// b6-5: Type
#define	STANDARD					 0x0000
#define	CLASS						 0x0020
#define	VENDOR						 0x0040
#define	USBREQ_bmRequestTypeRecip	(0x001F)	// b4-0: Recipient
#define	DEVICE						 0x0000
#define	INTERFACE					 0x0001
#define	ENDPOINT			 		 0x0002

// USB Request Value Register
#define	USBVAL				(*((REGP *)(gUSB_BaseAddr+0x56)))
// Standard Feature Selector
#define	ENDPOINT_HALT				 0x0000
#define	DEVICE_REMOTE_WAKEUP		 0x0001
#define	TEST_MODE					 0x0002
// Descriptor Types
#define	DT_TYPE						(0xFF00)
#define	GET_DT_TYPE(v)				(((v) & DT_TYPE) >> 8)
#define	DT_DEVICE					 0x01
#define	DT_CONFIGURATION			 0x02
#define	DT_STRING					 0x03
#define	DT_INTERFACE				 0x04
#define	DT_ENDPOINT					 0x05
#define	DT_DEVICE_QUALIFIER			 0x06
#define	DT_OTHER_SPEED_CONFIGURATION 0x07
#define	DT_INTERFACE_POWER			 0x08
#define	DT_INDEX					(0x00FF)
#define	ALT_SET						(0x00FF)

// USB Request Index Register
#define	USBINDX				(*((REGP *)(gUSB_BaseAddr+0x58)))
#define	TEST_SELECT			(0xFF00)	// b15-b8: Test Mode Selectors
#define	TEST_J				 0x0100		// Test_J
#define	TEST_K				 0x0200		// Test_K
#define	TEST_SE0_NAK		 0x0300		// Test_SE0_NAK
#define	TEST_PACKET		 	 0x0400		// Test_Packet
#define	TEST_FORCE_ENABLE	 0x0500		// Test_Force_Enable
#define	TEST_Reserved		 0x4000		// Reserved
#define	TEST_VSTModes		 0xC000		// Vendor-specific test modes
 
// USB Request Length Register
#define	USBLENG				(*((REGP *)(gUSB_BaseAddr+0x5A)))

// Default Control Pipe Maxpacket Size Register
#define	DCPMAXP				(*((REGP *)(gUSB_BaseAddr+0x5E)))
#define	MAXP				(0x007F)	// b6-0: Maxpacket size of default control pipe

// Default Control Pipe Control Register
#define	DCPCTR_ADR			(gUSB_BaseAddr+0x60)
#define	DCPCTR				(*((REGP *)(DCPCTR_ADR)))
#define	CCPL				(0x0004)	// b2: Enable control transfer complete
#define	PID					(0x0003)	// b1-0: Response PID
#define	PID_STALL			 0x0002		// STALL
#define	PID_BUF			 	 0x0001		// BUF

// Pipe Window Select Register
#define	PIPESEL				(*((REGP *)(gUSB_BaseAddr+0x64)))
#define	PIPE0				 0x0000		// PIPE 0
#define	PIPE1				 0x0001		// PIPE 1
#define	PIPE2				 0x0002		// PIPE 2
#define	PIPE6				 0x0006		// PIPE 6
#define	PIPE15				 0x000F		// PIPE 15
#define	USEPIPE				 (U16)0x00FE

// Pipe Configuration Register
#define	PIPECFG				(*((REGP *)(gUSB_BaseAddr+0x68)))
#define	TYP					(0xC000)	// b15-14: Transfer type
#define	INTR				 0x8000		// Interrupt
#define	BULK				 0x4000		// Bulk
#define	CNTMD				(0x0100)	// b8: Continuous transfer mode select
#define	SHTNAK				(0x0080)	// b7: Transfer end NAK
#define	DIR					(0x0010)	// b4: Transfer direction select
#define	DIR_P_IN			 0x0010		// PERI IN
#define	DIR_P_OUT			 0x0000		// PERI OUT
#define	EPNUM				(0x000F)	// b3-0: Eendpoint number select
#define	EP1					 0x0001
#define	EP2					 0x0002
#define	EP3					 0x0003

// Pipe Buffer Configuration Register
#define	PIPEBUF				(*((REGP *)(gUSB_BaseAddr+0x6A)))
#define	BUF_SIZE(x)			(U16)((((x) / 64) - 1) << 10)
#define	PIPExBUF			(64)

// Pipe Maxpacket Size Register
#define	PIPEMAXP			(*((REGP *)(gUSB_BaseAddr+0x6C)))
#define	MXPS				(0x07FF)	// b10-0: Maxpacket size

// Pipe Cycle Configuration Register
#define	PIPEPERI			(*((REGP *)(gUSB_BaseAddr+0x6E)))

// Pipex Control Register
#define	PIPECTR_ADR			(gUSB_BaseAddr+0x70)
#define PIPEnCTR(pipe)		(*((REGP *)( PIPECTR_ADR+(2*(pipe-1)) )))
#define	INBUFM				(0x4000)	// b14: IN buffer monitor (Only for PIPE1 to 5)
#define	ACLRM				(0x0200)	// b9: Out buffer auto clear mode
#define	SQCLR				(0x0100)	// b8: Sequence toggle bit clear
#define	PBUSY				(0x0020)	// b5: Pipe busy monitor

// GET_STATUS request to a device returns the information
#define	GS_HALT				0x01
#define	GS_BUSPOWERD		0x00
#define	GS_SELFPOWERD		0x01
#define	GS_REMOTEWAKEUP		0x02

// USB ASSP DEFINE
#define	MAX_EP_NO			15			// EP0 EP1 ... EP15
#define	MAX_PIPE_NO			15			// PIPE0 ... PIPE15
#define	EPL					5			// Endpoint Table Length

// PHY register
#define	LPCTRL				(*((REGP *)(gUSB_BaseAddr+0x100)))
#define	LPSTS				(*((REGP *)(gUSB_BaseAddr+0x102)))

#define	PHYFUNCTR			(*((REGP *)(gUSB_BaseAddr+0x104)))
#define	PHYIFCTR			(*((REGP *)(gUSB_BaseAddr+0x106)))
#define	PHYOTGCTR			(*((REGP *)(gUSB_BaseAddr+0x10A)))
#define	PHYINTR				(*((REGP *)(gUSB_BaseAddr+0x10C)))
#define	PHYINTF				(*((REGP *)(gUSB_BaseAddr+0x110)))

#define	MFUNCCNTL			(*((REGP *)(gUSB_BaseAddr+0x124)))
#define	MIFCTRL				(*((REGP *)(gUSB_BaseAddr+0x126)))
#define	MOTGCTRL			(*((REGP *)(gUSB_BaseAddr+0x12A)))
#define	MPINTER				(*((REGP *)(gUSB_BaseAddr+0x12C)))
#define	MPINTEF				(*((REGP *)(gUSB_BaseAddr+0x130)))
#define	MPINTSTS			(*((REGP *)(gUSB_BaseAddr+0x132)))

// USBDMA
#define USBDMA_BASE			0xE68A0000
#define USBDMA_SAR0			(*(volatile long *)(USBDMA_BASE+0x20))
#define USBDMA_DAR0			(*(volatile long *)(USBDMA_BASE+0x24))
#define USBDMA_DMATCR0		(*(volatile long *)(USBDMA_BASE+0x28))
#define USBDMA_CHCR0		(*(volatile long *)(USBDMA_BASE+0x34))
#define FTE					(0x01000000) // b24
#define NULLE				(0x00010000) // b16
#define NULL_BIT			(0x00001000) // b12
#define TS_LONG				(0x00000080) // b7
#define IE					(0x00000020) // b5
#define	SP					(0x00000004) // b2
#define	TE					(0x00000002) // b1
#define	DE					(0x00000001) // b0
#define USBDMA_DMAOR		(*((REGP32 *)(USBDMA_BASE+0x60)))
#define DME					(0x0001)	// b0: DMA Master Enable
#define	ISEL				(0x0020)	// b5: DCP FIFO port direction select
#define	MBW					 0x0C00		// b11-10: Maximum bit width for FIFO access
#define	MBW_32				 0x0800		// 32bit


// GET_STATUS request to a device returns the information
#define	GS_HALT				0x01
#define	GS_BUSPOWERD		0x00
#define	GS_SELFPOWERD		0x01
#define	GS_REMOTEWAKEUP		0x02

// USB ASSP DEFINE
#define	MAX_EP_NO			15			// EP0 EP1 ... EP15
#define	MAX_PIPE_NO			15			// PIPE0 ... PIPE15
#define	EPL					5			// Endpoint Table Length

#endif	/* __C_DEF596__ */
