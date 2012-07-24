/* P_Def_ep.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

/*
 ******************************************************************************
 * Endpoint Configuration Data Format
 ******************************************************************************
 *	LINE1: Pipe Window Select Register (0x64)
 *		CPU Access PIPE					: PIPE1 to PIPE7
 *		Select FIFO port				: C_FIFO_USE / D0_FIFO_USE / D1_FIFO_USE
 *	LINE2: Pipe Configuration Register (0x66)
 *		Transfer Type					: ISO / INT / BULK
 *		Double Buffer Mode				: DBLB / OFF		(PIPE1-5 only)
 *		Continuous Transmit/Receive Mode: CNTMD / OFF		(PIPE1-5 only)
 *		Transfer Direction				: DIR_P_IN / DIR_P_OUT
 *		Endpoint Number					: EP1 to EP15
 *	LINE3: Pipe Buffer Configuration Register (0x68)
 *		Buffer Size						: BUF_SIZE(x)		(PIPE1-5: x:64 to 2048)
 *															(PIPE6-7: x:64)
 *		Buffer Top Number				: 4 to 79			(PIPE6:4, PIPE7:5)
 *	LINE4: Pipe Maxpacket Size Register (0x6A)
 *		Max Packet Size					: 0 to 1024
 *	LINE5: Pipe Cycle Configuration Register (0x6C)
 *		ISO Buffer Flush Mode			: IFIS / OFF
 *		ISO Interval Value				: 0 to 7
 ******************************************************************************
 */

/*
 *	|--- Configuration 1
 *	|		|--- Interface 1-0-0
 *	|		|		|--- Endpoint 1-0-0-0
 */
// Configuration 1
U16  EPtbl_1[MAX_EP_NO * EPL];
const U16  EPtbl_2[] = { 0 };					// Configuration 2
const U16  EPtbl_3[] = { 0 };					// Configuration 3
const U16  EPtbl_4[] = { 0 };					// Configuration 4
const U16  EPtbl_5[] = { 0 };					// Configuration 5

const U16 *EndPntPtr[] = {
	EPtbl_1,
	EPtbl_2,
	EPtbl_3,
	EPtbl_4,
	EPtbl_5
};
