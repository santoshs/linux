/* C_MacUsr.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

/*------------------------------------------------------------
 * USB FIFO register access macro
 *------------------------------------------------------------*/
/* Read/Write USB FIFO register */
#define	USBRD_FF( r, v )		do { ( v ) = ( r ); } while(0)
#define	USBWR_FF( r, v )		do { ( r ) = ( v ); } while(0)

/*------------------------------------------------------------
 * USB register access macro
 *------------------------------------------------------------*/
/* Read/Write USB register */
#define	USBRD( r, v )			do { ( v ) = ( r ); } while(0)
#define	USBWR( r, v )			do { ( r ) = ( v ); } while(0)

/*------------------------------------------------------------
 * USB register bit access macro
 *------------------------------------------------------------*/
/* set bit(s) of USB register	 */
/* r : USB register				 */
/* v : value to set				 */
#define	USB_SET_PAT( r, v )		do { U16 mtmp;			\
									USBRD( r, mtmp );	\
									mtmp |= ( v );		\
									USBWR( r, mtmp ); } while(0)

/* reset bit(s) of USB register  */
/* r : USB register				 */
/* m : bit pattern to reset		 */
#define	USB_CLR_PAT( r, m )		do { U16 mtmp;			\
									USBRD( r, mtmp );	\
									mtmp &= ( ~(m) );	\
									USBWR( r, mtmp ); } while(0)

/* modify bit(s) of USB register */
/* r : USB register				 */
/* v : value to set				 */
/* m : bit pattern to modify	 */
#define	USB_MDF_PAT( r, v, m )	do { U16 mtmp;			\
									USBRD( r, mtmp );	\
									mtmp &= ( ~(m) );	\
									mtmp |= v;			\
									USBWR( r, mtmp ); } while(0)

/* reset bit(s) of USB status	 */
/* r : USB register				 */
/* m : bit pattern to reset		 */
#define	USB_CLR_STS( r, m )		USBWR( r, (unsigned short)( ~(m)) )

/* set bit(s) of USB status		 */
/* r : USB register				 */
/* m : dummy					 */
#define	USB_SET_STS( r, m )		USBWR( r, 0xffff )
