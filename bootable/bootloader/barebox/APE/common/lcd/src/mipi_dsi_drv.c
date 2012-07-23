/*
 * mipi_dsi_drv.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */
/***************************************************************/
/* INCLUDE FILE                                                */
/***************************************************************/
#include "eos_csw.h"					/* EOS CSW Header file */
#include "eos_stdio.h"					/* EOS Standard Header */
#include "cp1_c01_cpg.h"
#include "cp1_m12_dsi.h"

#include "timer_drv.h"					/* Timer function header    */
#include "mipi_dsi_drv.h"				/* header file    */
#include "lcd.h"

/***************************************************************/
/* STATIC CONSTANT DEFINE                                      */
/***************************************************************/
#define inpS(port)		  (*((volatile u1 *) (port)))
#define inpwS(port) 	  (*((volatile u2 *) (port)))
#define inplS(port) 	  (*((volatile u4 *) (port)))
#define outpS(port, val)  (*((volatile u1 *) (port)) = ((u1) (val)))
#define outpwS(port, val) (*((volatile u2 *) (port)) = ((u2) (val)))
#define outplS(port, val) (*((volatile u4 *) (port)) = ((u4) (val)))

#define inp(port)		 (*((volatile u1 *) (port)))
#define inpw(port)		 (*((volatile u2 *) (port)))
#define inpl(port)		 (*((volatile u4 *) (port)))
#define outp(port, val)  (*((volatile u1 *) (port)) = ((u1) (val)))
#define outpw(port, val) (*((volatile u2 *) (port)) = ((u2) (val)))
#define outpl(port, val) (*((volatile u4 *) (port)) = ((u4) (val)))

#define HTC_SH_ModifyRegister32(addr,clrmask,setmask) \
	outplS(addr,( ( inplS(addr) & ~(clrmask) ) | (setmask) ) )
#define HTC_SH_ModifyRegister16(addr,clrmask,setmask) \
	outpwS(addr,( ( inpwS(addr) & ~(clrmask) ) | (setmask) ) )
#define HTC_SH_ModifyRegister8(addr,clrmask,setmask) \
	outpS(addr,( ( inpS(addr) & ~(clrmask) ) | (setmask) ) )

#define U4S_DSI_BADR			(u4)( 0xFEAB0000 )					/* LCDC0 line:MainLCD_LINE */
#define U4S_DSI2_BADR			(u4)( 0xFEAB8000 )					/* LCDC1 line:SubLCD_LINE */
#define U4S_DTST_DTE0			(u4)( 0x00800000 )					/* Transmitted [CH0] */

#define	U4S_SYSCTRL_ADDR		(u4)( U4S_DSI_BADR+0x0000 )			/* System Control Register */
#define	U4S_SYSCONF_ADDR		(u4)( U4S_DSI_BADR+0x0004 )			/* System Configuration Register */
#define	U4S_TIMSET0_ADDR		(u4)( U4S_DSI_BADR+0x0008 )			/* Transition Timing Parameter Setting Register */
#define	U4S_PTYPESET0_ADDR		(u4)( U4S_DSI_BADR+0x0010 )			/* Transmission Packet Type Setting Register 0 */
#define	U4S_PTYPESET1_ADDR		(u4)( U4S_DSI_BADR+0x0014 )			/* Transmission Packet Type Setting Register 1 */
#define	U4S_RESREQSET0_ADDR		(u4)( U4S_DSI_BADR+0x0018 )			/* Response Request Setting Register 0 */
#define	U4S_RESREQSET1_ADDR		(u4)( U4S_DSI_BADR+0x001C )			/* Response Request Setting Register 1 */
#define	U4S_HSTTOVSET_ADDR		(u4)( U4S_DSI_BADR+0x0020 )			/* HS Transmission Time-out Setting Register */
#define	U4S_LPRTOVSET_ADDR		(u4)( U4S_DSI_BADR+0x0024 )			/* LP Reception Time-out Setting Register */
#define	U4S_TATOVSET_ADDR		(u4)( U4S_DSI_BADR+0x0028 )			/* Turn Around Time-out Setting Register */
#define	U4S_PRTOVSET_ADDR		(u4)( U4S_DSI_BADR+0x002C )			/* Peripheral Reset Time-out Value Setting Register */
#define	U4S_DSICTRL_ADDR		(u4)( U4S_DSI_BADR+0x0030 )			/* DSI-LINK Control Register */
#define	U4S_RPTYPESET0_ADDR		(u4)( U4S_DSI_BADR+0x0038 )			/* Reception Packet Type Setting Register 0 */
#define	U4S_RPTYPESET1_ADDR		(u4)( U4S_DSI_BADR+0x003C )			/* Reception Packet Type Setting Register 1 */
#define	U4S_DSIS_ADDR			(u4)( U4S_DSI_BADR+0x0040 )			/* DSI-LINK Status Register */
#define	U4S_DSIINT_ADDR			(u4)( U4S_DSI_BADR+0x0050 )			/* DSI-LINK Interrupt Status Register */
#define	U4S_DSIINTE_ADDR		(u4)( U4S_DSI_BADR+0x0060 )			/* DSI-LINK Interrupt Enable Register */
#define	U4S_PHYCTRL0_ADDR		(u4)( U4S_DSI_BADR+0x0070 )			/* PHY Control Register */
#define	U4S_PHYCTRL1_ADDR		(u4)( U4S_DSI_BADR+0x0074 )			/* PHY Control Register */
#define	U4S_TIMSET1_ADDR		(u4)( U4S_DSI_BADR+0x007C )			/* PHY Control Register */
#define	U4S_DTCTR_ADDR			(u4)( U4S_DSI_BADR+0x4000 )			/* DSI-L-Bridge Control Register */
#define	U4S_DTST_ADDR			(u4)( U4S_DSI_BADR+0x4004 )			/* DSI-L-Bridge Status Register */
#define	U4S_DTSTCL_ADDR			(u4)( U4S_DSI_BADR+0x4008 )			/* DSI-L-Bridge Status Clear Register */
#define	U4S_DTIRQEN_ADDR		(u4)( U4S_DSI_BADR+0x400C )			/* DSI-L-Bridge Interrupt Enable Register */
#define	U4S_VMCTR1_ADDR			(u4)( U4S_DSI_BADR+0x4020 )			/* Video Mode Control Register 1 */
#define	U4S_VMCTR2_ADDR			(u4)( U4S_DSI_BADR+0x4024 )			/* Video Mode Control Register 2 */
#define	U4S_VMLEN1_ADDR			(u4)( U4S_DSI_BADR+0x4028 )			/* Video Mode Data Length Register 1 */
#define	U4S_VMLEN2_ADDR			(u4)( U4S_DSI_BADR+0x402C )			/* Video Mode Data Length Register 2 */
#define	U4S_VMLEN3_ADDR			(u4)( U4S_DSI_BADR+0x4030 )			/* Video Mode Data Length Register 3 */
#define	U4S_VMLEN4_ADDR			(u4)( U4S_DSI_BADR+0x4034 )			/* Video Mode Data Length Register 4 */
#define	U4S_VMSDAT1_ADDR		(u4)( U4S_DSI_BADR+0x4038 )			/* Video Mode Short Packet Data Register 1 */
#define	U4S_VMSDAT2_ADDR		(u4)( U4S_DSI_BADR+0x403C )			/* Video Mode Short Packet Data Register 2 */
#define	U4S_CMRCTR_ADDR			(u4)( U4S_DSI_BADR+0x4040 )			/* Command Mode Reception Control Register */
#define	U4S_CMRDAT_ADDR			(u4)( U4S_DSI_BADR+0x4044 )			/* Command Mode Reception Data Register */
#define	U4S_CMRHEAD_ADDR		(u4)( U4S_DSI_BADR+0x4048 )			/* Command Mode Reception Header Register */
#define	U4S_CMRTEREQ_ADDR		(u4)( U4S_DSI_BADR+0x404C )			/* Tearing Effect Request Register */
#define	U4S_CMTLNGREQ0_ADDR		(u4)( U4S_DSI_BADR+0x4060 )			/* Command Mode Transmission Long Packet Request CH0 Register */
#define	U4S_CMTLNGREQ1_ADDR		(u4)( U4S_DSI_BADR+0x4064 )			/* Command Mode Transmission Long Packet Request CH1 Register */
#define	U4S_CMTLNGREQ2_ADDR		(u4)( U4S_DSI_BADR+0x4068 )			/* Command Mode Transmission Long Packet Request CH2 Register */
#define	U4S_CMTLNGREQ3_ADDR		(u4)( U4S_DSI_BADR+0x406C )			/* Command Mode Transmission Long Packet Request CH3 Register */
#define	U4S_CMTSRTREQ_ADDR		(u4)( U4S_DSI_BADR+0x4070 )			/* Command Mode Transmission Short Packet Request */
#define	U4S_CMTIADR0_ADDR		(u4)( U4S_DSI_BADR+0x4080 )			/* Command Mode Transmission Initiator Address CH0 Register */
#define	U4S_CMTIADR1_ADDR		(u4)( U4S_DSI_BADR+0x4084 )			/* Command Mode Transmission Initiator Address CH1 Register */
#define	U4S_CMTIADR2_ADDR		(u4)( U4S_DSI_BADR+0x4088 )			/* Command Mode Transmission Initiator Address CH2 Register */
#define	U4S_CMTIADR3_ADDR		(u4)( U4S_DSI_BADR+0x408C )			/* Command Mode Transmission Initiator Address CH3 Register */
#define	U4S_CMTITTL0_ADDR		(u4)( U4S_DSI_BADR+0x4090 )			/* Command Mode Transmission Initiator Total Data CH0 Register */
#define	U4S_CMTITTL1_ADDR		(u4)( U4S_DSI_BADR+0x4094 )			/* Command Mode Transmission Initiator Total Data CH1 Register */
#define	U4S_CMTITTL2_ADDR		(u4)( U4S_DSI_BADR+0x4098 )			/* Command Mode Transmission Initiator Total Data CH2 Register */
#define	U4S_CMTITTL3_ADDR		(u4)( U4S_DSI_BADR+0x409C )			/* Command Mode Transmission Initiator Total Data CH3 Register */
#define	U4S_CMTIADRI0_ADDR		(u4)( U4S_DSI_BADR+0x40A0 )			/* Command Mode Transmission Initiator Address Increment CH0 Register */
#define	U4S_CMTIADRI1_ADDR		(u4)( U4S_DSI_BADR+0x40A4 )			/* Command Mode Transmission Initiator Address Increment CH1 Register */
#define	U4S_CMTIADRI2_ADDR		(u4)( U4S_DSI_BADR+0x40A8 )			/* Command Mode Transmission Initiator Address Increment CH2 Register */
#define	U4S_CMTIADRI3_ADDR		(u4)( U4S_DSI_BADR+0x40AC )			/* Command Mode Transmission Initiator Address Increment CH3 Register */
#define	U4S_CMTIRN0_ADDR		(u4)( U4S_DSI_BADR+0x40B0 )			/* Command Mode Transmission Initiator Repeat Number CH0 Register */
#define	U4S_CMTIRN1_ADDR		(u4)( U4S_DSI_BADR+0x40B4 )			/* Command Mode Transmission Initiator Repeat Number CH1Register */
#define	U4S_CMTIRN2_ADDR		(u4)( U4S_DSI_BADR+0x40B8 )			/* Command Mode Transmission Initiator Repeat Number CH2 Register */
#define	U4S_CMTIRN3_ADDR		(u4)( U4S_DSI_BADR+0x40BC )			/* Command Mode Transmission Initiator Repeat Number CH3 Register */
#define	U4S_CMTLNGCTR0_ADDR		(u4)( U4S_DSI_BADR+0x40C0 )			/* Command Mode Transmission Long Packet Control CH0 Register */
#define	U4S_CMTLNGCTR1_ADDR		(u4)( U4S_DSI_BADR+0x40C4 )			/* Command Mode Transmission Long Packet Control CH1 Register */
#define	U4S_CMTLNGCTR2_ADDR		(u4)( U4S_DSI_BADR+0x40C8 )			/* Command Mode Transmission Long Packet Control CH2 Register */
#define	U4S_CMTLNGCTR3_ADDR		(u4)( U4S_DSI_BADR+0x40CC )			/* Command Mode Transmission Long Packet Control CH3 Register */
#define	U4S_CMTSRTCTR_ADDR		(u4)( U4S_DSI_BADR+0x40D0 )			/* Command Mode Transmission Short Packet Control Register */
#define	U4S_CMTLNGDT0_ADDR		(u4)( U4S_DSI_BADR+0x40E0 )			/* Command Mode Transmission Long Packet Data Type CH0 Register */
#define	U4S_CMTLNGDT1_ADDR		(u4)( U4S_DSI_BADR+0x40E4 )			/* Command Mode Transmission Long Packet Data Type CH1 Register */
#define	U4S_CMTLNGDT2_ADDR		(u4)( U4S_DSI_BADR+0x40E8 )			/* Command Mode Transmission Long Packet Data Type CH2 Register */
#define	U4S_CMTLNGDT3_ADDR		(u4)( U4S_DSI_BADR+0x40EC )			/* Command Mode Transmission Long Packet Data Type CH3 Register */
#define	U4S_CMTLNGMEM0_ADDR		(u4)( U4S_DSI_BADR+0x40F0 )

/***************************************************************/
/* STATIC TYPEDEF                                              */
/***************************************************************/

/***************************************************************/
/* STATIC VARIABLE                                             */
/***************************************************************/
static u4 u4s_dsi_offset;	/* DSI-offset DSI:0x0000 DSI2:0x8000*/

/***************************************************************/
/* STATIC FUNCTION PROTOTYPE                                   */
/***************************************************************/
static void vos_mipi_dsi_cpga_set( void );
static void vos_mipi_dsi_reset_link( void );
static void vos_mipi_dsi_set_link( void );
static void vos_mipi_dsi_set_lbridge( void );

/***************************************************************/
/* PUBLIC FUNCTION PROTOTYPE                                   */
/***************************************************************/
void vog_mipi_dsi_init( u1 u1t_lcd_mode );
void vog_mipi_dsi_ena_vmode( void );	/* enable_videomode */
void vog_mipi_dsi_dis_vmode( void );	/* disable_videomode */
void vog_mipi_dsi_tr_spacket( u1 u1t_data_type, u1 u1t_data0, u1 u1t_data1 );		/* transmit_shortpacket */
void vog_mipi_dsi_reg_set( u1 u1t_lcd_mode );
void vog_mipi_dsi_reg_get( u1* ptt_lcd_mode );
void vog_mipi_dsi_tr_lpacket( u1 u1t_data_type, u2 u2t_data_cnt, u1* ptt_data, u1 u1t_tr_speed );
void vog_mipi_dsi_draw_cmode( u4 addr );

/*************************************************************************************************/
/* function : mipi_dsi_init(int lcd_mode)                                                        */
/* parameter: int lcd_mode(0:LCD_MAIN, 1:LCD_SUB)                                                */
/* return   : void                                                                               */
/* outline  : Initialize MIPI_DSI                                                                */
/*************************************************************************************************/
void vog_mipi_dsi_init( u1 u1t_lcd_mode )
{
	u4s_dsi_offset = U4G_ZERO;

	if( U1G_LCD_SUB == u1t_lcd_mode )
	{
		u4s_dsi_offset = U4S_DSI2_BADR - U4S_DSI_BADR;
	}

	vos_mipi_dsi_cpga_set();

	vos_mipi_dsi_reset_link();

	vos_mipi_dsi_set_link();

	vos_mipi_dsi_set_lbridge();

	return;
}

/*************************************************************************************************/
/* function : mipi_dsi_cpga_set(void)                                                            */
/* parameter: void                                                                               */
/* return   : void                                                                               */
/* outline  : CPG register setting for MIPI-DSI                                                  */
/*************************************************************************************************/
static void vos_mipi_dsi_cpga_set(void)
{
	/*DSI PHY PLL*/
	U4R_DSI0PHYCR |= 0x00008000;

	/* 10ms Wait */
	vog_timer_wait( 10, U1G_TIME_MS );

	/* DSI TX Link */
	U4R_DSITCKCR &= ~(0x00000100);
	/* DSI PHY */
	U4R_DSI0PCKCR &= ~(0x00000100);

	return;
}

/*************************************************************************************************/
/* function : vos_mipi_dsi_reset_link( void )                                                    */
/* parameter: aDsiOffset: IP Address off set                                                     */
/* return   : void                                                                               */
/* outline  : Reset DSI LINK block                                                               */
/*************************************************************************************************/
static void vos_mipi_dsi_reset_link( void )
{
	/* System Control Register Reset LINK block(SWRST=1) */
	(*(VU4 *)(U4S_SYSCTRL_ADDR + u4s_dsi_offset)) = 0x00000001;

	(*(VU4 *)(U4S_DTCTR_ADDR + u4s_dsi_offset)) = 0x00000010;

	/* Wait 50us */
	vog_timer_wait( 50, U1G_TIME_US );

	
	/* PHY Control Register */
	(*(VU4 *)( U4S_PHYCTRL0_ADDR + u4s_dsi_offset )) = 0x00000001;		/* Bias circuit in the DSI-Tx block is power-on. */
	/* PHY Control Register */
	(*(VU4 *)( U4S_PHYCTRL1_ADDR + u4s_dsi_offset )) = 0x00000000;		/* Bias circuit in the DSI-Tx block is power-on. */

	/* Wait 200us */
	vog_timer_wait( 200, U1G_TIME_US );

	/* PHY Control Register */
	(*(VU4 *)( U4S_PHYCTRL0_ADDR + u4s_dsi_offset )) = 0x03000001;		/* PLL multiplication ratio = 13, Clock output */
	

	/* Release Reset LINK block(SWRST=0) */
	(*(VU4 *)(U4S_SYSCTRL_ADDR + u4s_dsi_offset)) = 0x00000000;

	(*(VU4 *)(U4S_DTCTR_ADDR + u4s_dsi_offset)) = 0x00000000;

	(*(VU4 *)(U4S_DTSTCL_ADDR + u4s_dsi_offset)) = 0x10FF00F0;

	(*(VU4 *)(U4S_DSIINT_ADDR + u4s_dsi_offset)) = 0x00000000;

	(*(VU4 *)(U4S_CMRCTR_ADDR + u4s_dsi_offset)) = 0x01000000;

}

/*************************************************************************************************/
/* function : vos_mipi_dsi_set_link( void )                                                      */
/* parameter: aDsiOffset: IP Address off set                                                     */
/* return   : void                                                                               */
/* outline  : DSI LINK block register setting                                                    */
/*************************************************************************************************/
static void vos_mipi_dsi_set_link( void )
{
	/* System Configuration Register */
	(*(VU4 *)( U4S_SYSCONF_ADDR + u4s_dsi_offset)) = 0x00000703;		/* 2lane, Enable EoT paket send/CRC check/ECC check */

	/* Transition Timing Parameter Setting Register */
	(*(VU4 *)( U4S_TIMSET0_ADDR + u4s_dsi_offset )) = 0x50006454;

	/* Transition Timing Parameter Setting Register */
	(*(VU4 *)( U4S_TIMSET1_ADDR + u4s_dsi_offset )) = 0x000B0113;

	/* HS Transmission Time-out Setting Register */
	(*(VU4 *)( U4S_HSTTOVSET_ADDR + u4s_dsi_offset )) = 0xffffffff;		/* 8.05[s] */

	/* LP Reception Time-out Setting Register */
	(*(VU4 *)( U4S_LPRTOVSET_ADDR + u4s_dsi_offset )) = 0xffffffff;		/* 14.39[s] */

	/* Turn Around Time-out Setting Register */
	(*(VU4 *)( U4S_TATOVSET_ADDR + u4s_dsi_offset )) = 0xffffffff;		/* 14.39[s] */

	/* Peripheral Reset Time-out Value Setting Register */
	(*(VU4 *)( U4S_PRTOVSET_ADDR + u4s_dsi_offset )) = 0xffffffff;		/* 14.50[s] */

	/* DSI-LINK Control Register */
	(*(VU4 *)( U4S_DSICTRL_ADDR + u4s_dsi_offset )) = 0x00000001;		/* Enable timer */

	(*(VU4 *)( U4S_DTCTR_ADDR + u4s_dsi_offset )) = 0x00000002;		/* release the reset state, Transmission enabled */

	return;

}

/*************************************************************************************************/
/* function : vos_mipi_dsi_set_lbridge( void )                                                   */
/* parameter: aDsiOffset: IP Address off set                                                     */
/* return   : void                                                                               */
/* outline  : DSI L-bridge block register setting                                                */
/*************************************************************************************************/
static void vos_mipi_dsi_set_lbridge( void )
{

	/* Video Mode Control Register 1 */
	(*(VU4 *)( U4S_VMCTR1_ADDR + u4s_dsi_offset )) = 0x0011003e;		/* VSYNW=1, HS ByteClock */

	/* Video Mode Control Register 2 */
	(*(VU4 *)( U4S_VMCTR2_ADDR + u4s_dsi_offset )) = 0x00E20730;		/* Non-Burst Mode with Sync Pulses: VSEE =1, HSEE =1, HSAE =1, BL2E =0 */

	(*(VU4 *)( U4S_VMLEN1_ADDR + u4s_dsi_offset )) = 0x05A00008;		/* one line = 480pixel*3. */

	(*(VU4 *)( U4S_VMLEN2_ADDR + u4s_dsi_offset )) = 0x00080000;		/* VLEN2 */

	(*(VU4 *)( U4S_VMLEN3_ADDR + u4s_dsi_offset )) = 0x00000000;		/* VLEN3 */

	(*(VU4 *)( U4S_VMLEN4_ADDR + u4s_dsi_offset )) = 0x00000000;		/* VLEN4 */

	(*(VU4 *)( U4S_VMLEN4_ADDR + u4s_dsi_offset )) = 0x00000000;		/* VLEN4 */

	(*(VU4 *)( U4S_VMSDAT1_ADDR + u4s_dsi_offset )) = 0x00000000;		/*  */

	/* Video Mode Short Packet Data Register 2 */
	(*(VU4 *)( U4S_VMSDAT2_ADDR + u4s_dsi_offset )) = 0x00000f0f;		/* EOT packet = 0x0f0f */

	(*(VU4 *)( U4S_DTCTR_ADDR + u4s_dsi_offset )) |= 0x00000004;		/* release the reset state, Transmission enabled */

	return;

}

/*************************************************************************************************/
/* function : vog_mipi_dsi_ena_vmode( void )                                                     */
/* parameter: void                                                                               */
/* return   : void                                                                               */
/* outline  : DSI operation beginning                                                            */
/*************************************************************************************************/
void vog_mipi_dsi_ena_vmode( void )
{

	/* DSI-L-Bridge Control Register */
	(*(VU4 *)( U4S_DTCTR_ADDR + u4s_dsi_offset )) |= 0x00000001;		/* Transmission disabled, DSI operation beginning(VMEN=1) */
	return;
}

/*************************************************************************************************/
/* function : vog_mipi_dsi_dis_vmode( void )                                                     */
/* parameter: int lcd_mode(0:LCD_MAIN, 1:LCD_SUB)                                                */
/* return   : void                                                                               */
/* outline  : DSI operation end                                                                  */
/*************************************************************************************************/
void vog_mipi_dsi_dis_vmode( void )
{

	/* DSI-L-Bridge Control Register */
	(*(VU4 *)( U4S_DTCTR_ADDR + u4s_dsi_offset )) = 0x00000006;		/* release the reset state, Transmission enabled */

	return;
}


/*************************************************************************************************/
/* function : vog_mipi_dsi_tr_spacket( u1 u1t_data_type, u1 u1t_data0, u1 u1t_data1 )            */
/* parameter: u1 u1t_data_type, u1 u1t_data0, u1 u1t_data1                                       */
/* return   : void                                                                               */
/* outline  : Transmit short packet to Panel for setting Panel register.                         */
/*************************************************************************************************/
void vog_mipi_dsi_tr_spacket( u1 u1t_data_type, u1 u1t_data0, u1 u1t_data1 )
{

	/* Command Mode Transmission Short Packet Control Register */
	(*(VU4 *)( U4S_CMTSRTCTR_ADDR + u4s_dsi_offset )) = 
	( 0x00000000 | ( u1t_data_type << 24 ) | ( u1t_data0 << 16 ) | ( u1t_data1 << 8 ) );	/* HighSpeed */

	/* Command Mode Transmision Short Packet Request Register */
	(*(VU4 *)( U4S_CMTSRTREQ_ADDR + u4s_dsi_offset )) = 0x00000001;		/* transmit command-packet */

	while( ((*(VU4 *)( U4S_CMTSRTREQ_ADDR + u4s_dsi_offset)) & 0x00000001) == 0x00000001 );

	return;

}



/*************************************************************************************************/
/* function : vog_mipi_dsi_tr_lpacket( u1 u1t_data_type, u2 u1t_data_cnt, u1* u1t_data )         */
/* parameter: u1 u1t_data_type, u1 u1t_data0, u1 u1t_data1                                       */
/* return   : void                                                                               */
/* outline  : Transmit Long Packet to Panel for setting Panel register.                          */
/*************************************************************************************************/

static u4 tmp;
static u4 tmp2;
static u1 u1t_da_cnt;
static u1 u1t_data_shift;
static u2 u2t_cnt;
static u4 u4t_data_addr[4] = { U4S_CMTIADR0_ADDR, U4S_CMTITTL0_ADDR, U4S_CMTIADRI0_ADDR, U4S_CMTIRN0_ADDR };

void vog_mipi_dsi_tr_lpacket( u1 u1t_data_type, u2 u2t_data_cnt, u1* ptt_data, u1 u1t_tr_speed )
{

	u4 u4t_cmtlngctr0 = U4G_ZERO;

	if(16 >= u2t_data_cnt)
	{
		
		/* Command Mode Transmission Short Packet Control Register */
		/* CMTIADRx,CMTITTLx,CMTIADRIx,CMTIRNx Data Register used */
		/* Set CMTLNGCTR0.VCID/DT/LENG/TXSEL=0 */
		u4t_cmtlngctr0 = (u4)( ( u1t_data_type << 24 ) | ( u2t_data_cnt << 8 ) );

		u1t_da_cnt = 0;
		u1t_data_shift = 24;

		tmp = 0;

		for( u2t_cnt = 0; u2t_cnt < u2t_data_cnt; u2t_cnt++ )
		{

			tmp2=(u4)(*ptt_data);
			tmp |= (tmp2 << u1t_data_shift);
			(*(VU4 *)( u4t_data_addr[u1t_da_cnt] + u4s_dsi_offset ) ) = tmp;
			if( u1t_data_shift == 0 )
			{
				u1t_da_cnt++;
				u1t_data_shift = 24;

				tmp = 0;

			}
			else
			{
				u1t_data_shift -= 8;
			}
			ptt_data++;
			
		}
	}
	else
	{
		/* Set CMTLNGCTR0.VCID/DT/LENG=0x800/TXSEL=1 */
		u4t_cmtlngctr0 = (u4)( 0x00080002 | ( u1t_data_type << 24 ) );
		
		(*(VU4 *)( U4S_CMTIADR0_ADDR + u4s_dsi_offset ))   = (u4)ptt_data;
		(*(VU4 *)( U4S_CMTITTL0_ADDR + u4s_dsi_offset ))   = u2t_data_cnt;
		(*(VU4 *)( U4S_CMTIADRI0_ADDR + u4s_dsi_offset ))  = 0;
		(*(VU4 *)( U4S_CMTIRN0_ADDR + u4s_dsi_offset ))    = 0;
		(*(VU4 *)( U4S_CMTLNGDT0_ADDR + u4s_dsi_offset ))  = 0x00700000;
	}

	/* Command Mode Transmission Long Packet Control CH0 Register */
	/* Set CMTLNGCTR0.HSSEL */
	(*(VU4 *)( U4S_CMTLNGCTR0_ADDR + u4s_dsi_offset )) = ( u4t_cmtlngctr0 | (u4)u1t_tr_speed );

	/* Command Mode Transmision Short Packet Request Register */
	(*(VU4 *)( U4S_CMTLNGREQ0_ADDR + u4s_dsi_offset )) = 0x00000001;		/* transmit command-packet */

	while( ((*(VU4 *)( U4S_CMTLNGREQ0_ADDR + u4s_dsi_offset)) & 0x00000001) == 0x00000001 );

	/* Wait 50us */
	vog_timer_wait( 50, U1G_TIME_US );

	return;

}
#if 1
void vog_mipi_dsi_draw_cmode( u4 addr )
{
	/* DSI-L-Bridge Status Clear */
	(*(VU4 *)( U4S_DTSTCL_ADDR)) = ((*(VU4 *)( U4S_DTST_ADDR)) & 0x10FF00F0);
	
	(*(VU4 *)( U4S_CMTIADR0_ADDR ))   = addr;
	(*(VU4 *)( U4S_CMTITTL0_ADDR ))   = LCD_DISP_XMAX * LCD_RGB888_ONE_PIXEL;
	(*(VU4 *)( U4S_CMTIADRI0_ADDR ))  = LCD_DISP_STRIDE * LCD_RGB888_ONE_PIXEL;
	(*(VU4 *)( U4S_CMTIRN0_ADDR ))    = 1;
	(*(VU4 *)( U4S_CMTLNGCTR0_ADDR )) = 0x39080003;		/* DCS_LONG_WRITE,0x0800byte,TXSEL,HSSET */
	(*(VU4 *)( U4S_CMTLNGDT0_ADDR ))  = 0x00700000;		/* LW,W,B swap */
	(*(VU4 *)( U4S_CMTLNGMEM0_ADDR )) = 0x8000002C;		/* Write memory start */
	(*(VU4 *)( U4S_CMTLNGREQ0_ADDR )) = 0x00000001;		/* transmit command-packet */
	while( ((*(VU4 *)( U4S_CMTLNGREQ0_ADDR)) & 0x00000001) == 0x00000001 );
	while( !(((*(VU4 *)( U4S_DTST_ADDR)) & U4S_DTST_DTE0) == U4S_DTST_DTE0) );

	/* DSI-L-Bridge Status Clear */
	(*(VU4 *)( U4S_DTSTCL_ADDR)) = ((*(VU4 *)( U4S_DTST_ADDR)) & 0x10FF00F0);
	
	(*(VU4 *)( U4S_CMTIADR0_ADDR ))   = addr + (LCD_DISP_STRIDE * LCD_RGB888_ONE_PIXEL);
	(*(VU4 *)( U4S_CMTITTL0_ADDR ))   = LCD_DISP_XMAX * LCD_RGB888_ONE_PIXEL;
	(*(VU4 *)( U4S_CMTIADRI0_ADDR ))  = LCD_DISP_STRIDE * LCD_RGB888_ONE_PIXEL;
	(*(VU4 *)( U4S_CMTIRN0_ADDR ))    = LCD_DISP_YMAX - 1;
	(*(VU4 *)( U4S_CMTLNGCTR0_ADDR )) = 0x39080003;		/* DCS_LONG_WRITE,0x0800byte,TXSEL,HSSET */
	(*(VU4 *)( U4S_CMTLNGDT0_ADDR ))  = 0x00700000;		/* LW,W,B swap */
	(*(VU4 *)( U4S_CMTLNGMEM0_ADDR )) = 0x8000003C;		/* Write memory continue */
	(*(VU4 *)( U4S_CMTLNGREQ0_ADDR )) = 0x00000001;		/* transmit command-packet */
	while( ((*(VU4 *)( U4S_CMTLNGREQ0_ADDR)) & 0x00000001) == 0x00000001 );
	while( !(((*(VU4 *)( U4S_DTST_ADDR)) & U4S_DTST_DTE0) == U4S_DTST_DTE0) );
	
	(*(VU4 *)( U4S_CMTLNGMEM0_ADDR )) = 0;
}
#endif
/*************************************************************************************************/
/* Function name   : vog_mipi_dsi_reg_set( u1 u1t_lcd_mode )                                         */
/* Input     : u1 u1t_lcd_mode( U1G_LCD_MAIN(0):LCD_MAIN, U1G_LCD_SUB(1):LCD_SUB )                */
/* Return   : void                                                                               */
/* Processing : mipi_dsi_reg_set                                      */
/*************************************************************************************************/
void vog_mipi_dsi_reg_set( u1 u1t_lcd_mode )
{
	u4s_dsi_offset = U4G_ZERO;

	if( U1G_LCD_SUB == u1t_lcd_mode )
	{
		u4s_dsi_offset = U4S_DSI2_BADR - U4S_DSI_BADR;
	}

	return;
}

/*************************************************************************************************/
/* Function name   : vog_mipi_dsi_offset_set( u1* ptt_lcd_mode )                                */
/* Input     : u1* ptt_lcd_mode( U1G_LCD_MAIN(0):LCD_MAIN, U1G_LCD_SUB(1):LCD_SUB )             */
/* Return   : void                                                                              */
/* Processing : mipi dsi offset set																*/
/*************************************************************************************************/
void vog_mipi_dsi_reg_get( u1* ptt_lcd_mode )
{
	/* DSI */
	if( U4G_ZERO == u4s_dsi_offset )
	{
		/* MAIN_LCD */
		*ptt_lcd_mode = U1G_LCD_MAIN;
	}
	else
	{
		/* SUB_LCD */
		*ptt_lcd_mode = U1G_LCD_SUB;
	}

	return;
}

