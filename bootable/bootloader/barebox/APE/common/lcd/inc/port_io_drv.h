/*
 * port_io_drv.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */
#ifndef __H_PORT_IO_DRV_
#define __H_PORT_IO_DRV_

/***************************************************************/
/* INCLUDE FILE                                                */
/***************************************************************/

/***************************************************************/
/* PUBLIC CONSTANT DEFINITON                                   */
/***************************************************************/
#define U1G_PORT_OFF	(u1)( 0 )
#define U1G_PORT_ON		(u1)( 1 )
#define U1G_PORT_LO		(u1)( 0 )
#define U1G_PORT_HI		(u1)( 1 )
#define U1G_PORT_OUT	(u1)( 0x10 )
#define U1G_PORT_IN		(u1)( 0x20 )
#define U1G_PU_MASK		(u1)( 0xC0 )
#define U1G_PD_MASK		(u1)( 0x80 )

#define U2G_PORT_END	(u2)( 0xFFFF )	/* PORT Table END No.  */
#define U1G_PORT_RW		(u1)( 0 )		/* PORT I/O            */
#define U1G_PORT_R		(u1)( 1 )		/* PORT I ONLY         */
#define U1G_PORT_W		(u1)( 2 )		/* PORT O ONLY         */

/***************************************************************/
/* PUBLIC TYPEDEF                                              */
/***************************************************************/
typedef struct port_info{
	u2 u2_port_num;						/* PORT number */
	u4 u4_portcr_addr;					/* PORT control register */
	u4 u4_portdr_addr;					/* PORT data register */
	u1 u1_portdr_bit;					/* PORT (LSB0-MSB31) */
	u1 u1_port_io;						/* PORT I/O ( U1S_PORT_RW(0)=I/O U1S_PORT_R(1)=I U1S_PORT_W(2)=O ) */
} ST_PORT_INFO;

/***************************************************************/
/* PUBLIC VARIABLE EXTERN                                      */
/***************************************************************/

/***************************************************************/
/* PUBLIC FUNCTION EXTERN                                      */
/***************************************************************/
extern void vog_port_st_get( u2 u2t_port_num, u1* u1t_get_data );
extern void vog_port_st_set( u2 u2t_port_num, u1  u1t_set_data );
extern void vog_port_get( u2 u2t_port_num, u1* u1t_get_data );
extern void vog_port_set( u2 u2t_port_num, u1  u1t_set_data );
extern void vog_port_on( u2 u2t_port_num );
extern void vog_port_off( u2 u2t_port_num );
extern u1 u1g_port_info_check( u2 u2t_port_num, ST_PORT_INFO* ptt_port_info );

#endif /* __H_PORT_IO_DRV_ */

