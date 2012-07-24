/*
 * lcd_drv.c
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
#include "cp1_c09_gpio.h"
#include "cp1_m11_lcdc.h"

#include "mipi_dsi_drv.h"				/* MIPI-DSI driver    */
#include "lcd_panel_drv.h"				/* LCD_PANEL driver   */
#include "timer_drv.h"					/* Timer function header    */
#include "port_io_drv.h"				/* PORT I/O driver     */

#include "lcd_drv.h"					/* header file    */

/***************************************************************/
/* STATIC CONSTANT DEFINE                                      */
/***************************************************************/
#define U2S_LCD_RST_PORT			(u2)( 217 )
#define U4S_SUBCKCR_CKSEL_EXTAL2	(u4)( 0x00000080 )

#define	U4S_LCDC_OFFSET				(u4)( LCDC1_BADR - LCDC_BADR )
/* LCDC_BADR : 0xFE940000 */
#define	U4S_MLDDCKPAT1R_ADDR		(u4)( LCDC_BADR + 0x0400 )		/* Main LCD Dot clock buffer setting register1 */
#define	U4S_MLDDCKPAT2R_ADDR		(u4)( LCDC_BADR + 0x0404 )		/* Main LCD Dot clock buffer setting register2 */
#define	U4S_SLDDCKPAT1R_ADDR		(u4)( LCDC_BADR + 0x0408 )		/* LCD Dot clock buffer setting register1 */
#define	U4S_SLDDCKPAT2R_ADDR		(u4)( LCDC_BADR + 0x040C )		/* LCD Dot clock buffer setting register2 */
#define	U4S_LDDCKR_ADDR				(u4)( LCDC_BADR + 0x0410 )		/* LCDC dot clock register */
#define	U4S_LDDCKSTPR_ADDR			(u4)( LCDC_BADR + 0x0414 )		/* dot clock stop register */
#define	U4S_MLDMT1R_ADDR			(u4)( LCDC_BADR + 0x0418 )		/* Main LCD module type register1 */
#define	U4S_MLDMT2R_ADDR			(u4)( LCDC_BADR + 0x041C )		/* Main LCD module type register2 */
#define	U4S_MLDMT3R_ADDR			(u4)( LCDC_BADR + 0x0420 )		/* Main LCD module type register3 */
#define	U4S_MLDDFR_ADDR				(u4)( LCDC_BADR + 0x0424 )		/* Main LCD Data format register */
#define	U4S_MLDSM1R_ADDR			(u4)( LCDC_BADR + 0x0428 )		/* Main LCD Scanning mode register1 */
#define	U4S_MLDSM2R_ADDR			(u4)( LCDC_BADR + 0x042C )		/* Main LCD Scanning mode register2 */
#define	U4S_MLDSA1R_ADDR			(u4)( LCDC_BADR + 0x0430 )		/* Main LCD The displayed data start address register1 */
#define	U4S_MLDSA2R_ADDR			(u4)( LCDC_BADR + 0x0434 )		/* Main LCD The displayed data start address register2 */
#define	U4S_MLDMLSR_ADDR			(u4)( LCDC_BADR + 0x0438 )		/* Main LCD Display data storage memory line size register */
#define	U4S_MLDWBFR_ADDR			(u4)( LCDC_BADR + 0x043C )		/* Main LCD Writing return data format register */
#define	U4S_MLDWBCNTR_ADDR			(u4)( LCDC_BADR + 0x0440 )		/* Main LCD Writing return control register */
#define	U4S_MLDWBAR_ADDR			(u4)( LCDC_BADR + 0x0444 )		/* Main LCD Start address register of writing return destination */
#define	U4S_MLDHCNR_ADDR			(u4)( LCDC_BADR + 0x0448 )		/* Main LCD The horizontal character number register */
#define	U4S_MLDHSYNR_ADDR			(u4)( LCDC_BADR + 0x044C )		/* Main LCD Horizontal synchronizing signal register */
#define	U4S_MLDVLNR_ADDR			(u4)( LCDC_BADR + 0x0450 )		/* Main LCD Vertical line number register */
#define	U4S_MLDVSYNR_ADDR			(u4)( LCDC_BADR + 0x0454 )		/* Main LCD Vertical synchronizing signal register */
#define	U4S_MLDHPDR_ADDR			(u4)( LCDC_BADR + 0x0458 )		/* Main LCD The horizontal, partial screen register */
#define	U4S_MLDVPDR_ADDR			(u4)( LCDC_BADR + 0x045C )		/* Main LCD Vertical, partial screen register */
#define	U4S_MLDPMR_ADDR				(u4)( LCDC_BADR + 0x0460 )		/* Main LCD Power management register */
#define	U4S_LDPALCR_ADDR			(u4)( LCDC_BADR + 0x0464 )		/* LCDC Palette control register */
#define	U4S_LDINTR_ADDR				(u4)( LCDC_BADR + 0x0468 )		/* LCDC Interrupt register */
#define	U4S_LDSR_ADDR				(u4)( LCDC_BADR + 0x046C )		/* LCDC Status register */
#define	U4S_LDCNT1R_ADDR			(u4)( LCDC_BADR + 0x0470 )		/* LCDC Control register1 */
#define	U4S_LDCNT2R_ADDR			(u4)( LCDC_BADR + 0x0474 )		/* LCDC Control register2 */
#define	U4S_LDRCNTR_ADDR			(u4)( LCDC_BADR + 0x0478 )		/* LCDC Register side control register */
#define	U4S_LDDDSR_ADDR				(u4)( LCDC_BADR + 0x047C )		/* LCDC Input image data swap register */
#define	U4S_LDRCR_ADDR				(u4)( LCDC_BADR + 0x0484 )		/* LCDC Compulsion on register side specification register */
#define	U4S_LDDBSLMR_ADDR			(u4)( LCDC_BADR + 0x0490 )		/* Doubler mode register */
#define	U4S_LDSLHPNR_ADDR			(u4)( LCDC_BADR + 0x0494 )		/* Register of number of horizontal slides */
#define	U4S_LDSLVLNR_ADDR			(u4)( LCDC_BADR + 0x0498 )		/* Register of number of vertical slides */
#define	U4S_LDSLRGBR_ADDR			(u4)( LCDC_BADR + 0x049C )		/* Slide data register */
#define	U4S_MLDHAJR_ADDR			(u4)( LCDC_BADR + 0x04A0 )		/* Main LCD Horizontal synchronizing signal adjustment register */
#define	U4S_MLDIVSNR_ADDR			(u4)( LCDC_BADR + 0x04A4 )		/* Main LCD Vertical synchronizing signal adjustment register */
#define	U4S_SLDMT1R_ADDR			(u4)( LCDC_BADR + 0x0600 )		/* LCD module type register1 */
#define	U4S_SLDMT2R_ADDR			(u4)( LCDC_BADR + 0x0604 )		/* LCD module type register2 */
#define	U4S_SLDMT3R_ADDR			(u4)( LCDC_BADR + 0x0608 )		/* LCD module type register3 */
#define	U4S_SLDDFR_ADDR				(u4)( LCDC_BADR + 0x060C )		/* LCD Data format register */
#define	U4S_SLDSM1R_ADDR			(u4)( LCDC_BADR + 0x0610 )		/* LCD Scanning mode register1 */
#define	U4S_SLDSM2R_ADDR			(u4)( LCDC_BADR + 0x0614 )		/* LCD Scanning mode register2 */
#define	U4S_SLDSA1R_ADDR			(u4)( LCDC_BADR + 0x0618 )		/* LCD The displayed data start address register1 */
#define	U4S_SLDSA2R_ADDR			(u4)( LCDC_BADR + 0x061C )		/* LCD The displayed data start address register2 */
#define	U4S_SLDMLSR_ADDR			(u4)( LCDC_BADR + 0x0620 )		/* LCD Display data storage memory line size register */
#define	U4S_SLDHCNR_ADDR			(u4)( LCDC_BADR + 0x0624 )		/* LCD The horizontal character number register */
#define	U4S_SLDHSYNR_ADDR			(u4)( LCDC_BADR + 0x0628 )		/* LCD The horizontal, synchronous timing register */
#define	U4S_SLDVLNR_ADDR			(u4)( LCDC_BADR + 0x062C )		/* LCD Vertical line number register */
#define	U4S_SLDVSYNR_ADDR			(u4)( LCDC_BADR + 0x0630 )		/* LCD Vertical synchronizing signal register */
#define	U4S_SLDHPDR_ADDR			(u4)( LCDC_BADR + 0x0634 )		/* LCD The horizontal, partial screen register */
#define	U4S_SLDVPDR_ADDR			(u4)( LCDC_BADR + 0x0638 )		/* LCD Vertical, partial screen register */
#define	U4S_SLDPMR_ADDR				(u4)( LCDC_BADR + 0x063C )		/* LCD Power management register */

/***************************************************************/
/* STATIC TYPEDEF                                              */
/***************************************************************/

/***************************************************************/
/* STATIC VARIABLE                                             */
/***************************************************************/
static u4 u4s_lcd_offset;

/***************************************************************/
/* STATIC FUNCTION PROTOTYPE                                   */
/***************************************************************/
static void vos_lcd_module_start( void );
static void vos_lcd_set_gpio( u1 u1t_lcd_mode );

/***************************************************************/
/* PUBLIC VARIABLE                                             */
/***************************************************************/
/* VRAM Specification of common area */
/* LCD display */

/***************************************************************/
/* PUBLIC FUNCTION PROTOTYPE                                   */
/***************************************************************/
/* ***_lcd_***(); */
void vog_lcd_display_on( u2* ptt_vram_sta_addr, u1 u1t_lcd_mode );
void vog_lcd_display_off( u1 u1t_lcd_mode );
void vog_lcd_init_for_panel( u2* vram_sta_addr, u1 u1t_lcd_mode );

void vog_lcdc_to_dsi_start( u2* ptt_vram_sta_addr, u1 u1t_lcd_mode );
void vog_lcdc_to_stop( u1 u1t_lcd_mode );
void vog_lcdc_int_wait( void );
void vog_lcd_display_draw_cmode( u4 vram_addr );

/**
 * vog_lcd_display_draw_cmode - LCD Command Mode processing
 * @return                    - none
 */
void vog_lcd_display_draw_cmode( u4 vram_addr )
{

	/* Send Command Mode */
	vog_mipi_dsi_draw_cmode( vram_addr );

	return;

}

/*************************************************************************************************/
/* function : lcd_display_ON( UH* vram_sta_addr, int lcd_mode )                                  */
/* parameter: UH* vram_sta_addr, int lcd_mode(0:LCD_MAIN, 1:LCD_SUB)                             */
/* return   : void                                                                               */
/* outline  : LCD display ON                                                                     */
/*************************************************************************************************/
void vog_lcd_display_on( u2* ptt_vram_sta_addr, u1 u1t_lcd_mode )
{

	/* Setting CPGA */
	vos_lcd_module_start();
	
	/* Setting GPIO */
	vos_lcd_set_gpio( u1t_lcd_mode );

	/*------------------*/
	/* MIPI-DSI START   */
	/*------------------*/
	/* Initialize MIPI-DSI */
	vog_mipi_dsi_init( u1t_lcd_mode );

	/*------------------*/
	/* PANEL DISPLAY ON */
	/*------------------*/
	/* Initialize LCD panel */
	vog_lcd_panel_init( u1t_lcd_mode );

	/* draw command mode to avoid displaying noise */
	vog_lcd_display_draw_cmode( ptt_vram_sta_addr );

	vog_timer_ms_wait( 20 );

	/* LCD Panel display ON */
	vog_lcd_panel_display_on( u1t_lcd_mode );

	return;

}

/*************************************************************************************************/
/* function :lcd_display_OFF( int lcd_mode )                                                     */
/* parameter: int lcd_mode(0:LCD_MAIN, 1:LCD_SUB)                                                */
/* return   : void                                                                               */
/* outline  : LCD display OFF                                                                    */
/*************************************************************************************************/
void vog_lcd_display_off( u1 u1t_lcd_mode )
{
	U4R_SRCR1 |= 0x00060001;			/* SRCR1 ; [25]:TMU0,[18]:DSI TX Link,[17]:LCDC1,  [0]:LCDC ON */
	U4R_SRCR4 |= 0x00800000;			/* SRCR4 ; [23]:DSI TX Link */
	vog_timer_ms_wait( 5 );
	U4R_SRCR1 &= ~0x00060001;			/* SRCR1 ; [25]:TMU0,[18]:DSI TX Link,[17]:LCDC1,  [0]:LCDC ON */
	U4R_SRCR4 &= ~0x00800000;			/* SRCR4 ; [23]:DSI TX Link */

	return;
}


/*************************************************************************************************/
/* function : lcd_Init_for_Panel( UH* vram_sta_addr, int lcd_mode )                              */
/* parameter: UH* vram_sta_addr, int lcd_mode(0:LCD_MAIN, 1:LCD_SUB)                             */
/* return   : void                                                                               */
/* outline  : Initialization of LCD                                                              */
/*************************************************************************************************/
void vog_lcd_init_for_panel( u2* vram_sta_addr, u1 u1t_lcd_mode )
{
	/* Setting CPGA */
	vos_lcd_module_start();

	/* Setting GPIO */
	vos_lcd_set_gpio( u1t_lcd_mode );

	/* Initialize MIPI-DSI */
	vog_mipi_dsi_init( u1t_lcd_mode );

	/* Initialize LCD panel */
	vog_lcd_panel_init( u1t_lcd_mode );
	
	return;
}

/*************************************************************************************************/
/* function : lcd_module_start( void )                                                           */
/* parameter: void                                                                               */
/* return   : void                                                                               */
/* outline  : Start of module of LCDC_MVI3                                                       */
/*************************************************************************************************/
static void vos_lcd_module_start( void )
{
	U4R_SMSTPCR0 &= ~0x00000008;			/* MSTPCR0 ; ALL ON [7]:ICB */
	U4R_SMSTPCR1 &= ~0x02040001;			/* MSTPCR1 ; [25]:TMU0,[18]:DSI TX0 Link,[17]:LCDC1,  [0]:LCDC0 ON */
	U4R_SMSTPCR3 &= ~0x00000010;			/* MSTPCR3 ; [4]:TPU0 */

	u2 temp = 0;
	
	temp = U4R_MSTPSR0;
	while((temp & 0x00000008)==0x00000008)
	{
		temp = U4R_MSTPSR0;
	}
	
	temp = U4R_MSTPSR1;
	while((temp & 0x02040001)==0x02040001)
	{
		temp = U4R_MSTPSR1;
	}
	
	temp = U4R_MSTPSR3;
	while((temp & 0x00000010)==0x00000010)
	{
		temp = U4R_MSTPSR3;
	}

	return;
}


/*************************************************************************************************/
/* function : vos_lcd_set_gpio( u1 u1t_lcd_mode )                                                */
/* parameter: void                                                                               */
/* return   : void                                                                               */
/* outline  : The GPIO pin used with MIPI-DSI is set.                                                  */
/*************************************************************************************************/
static void vos_lcd_set_gpio( u1 u1t_lcd_mode )
{

	if( U1G_LCD_SUB != u1t_lcd_mode )
	{
		/* Enable LCDC0 port */
		U4R_MSEL3CR &= 0xFFFFFFBF;
		while( (U4R_MSEL3CR & 0x00000040) == 0x00000040 );
	}
	else
	{
		/* Enable LCDC1 port */
		U4R_MSEL3CR |= 0x00000040;
		while( (U4R_MSEL3CR & 0x00000040) != 0x00000040 );
	}

	return;
}

/*************************************************************************************************/
/* function : lcdc_to_dsi_start( UH* vram_sta_addr, int lcd_mode )                               */
/* parameter: UH* vram_sta_addr(LCD display start address), int lcd_mode(0:LCD_MAIN, 1:LCD_SUB)  */
/* return   : void                                                                               */
/* outline  : LCDC setting							                                             */
/*************************************************************************************************/
/* Horizontal Timing                                                                             */
/*                                                                                               */
/* Hsync  __         __________________________         ____________________________        __   */
/*          |_______|                          |_______|                            |______|     */
/* Enable _____________                      _____________                        ____________   */
/*                     |____________________|             |______________________|               */
/*        ------------------------------------------------------------------------------------   */
/* Data     invalid    |       valid        |   invalid   |        valid         |  invalid      */
/*        ------------------------------------------------------------------------------------   */
/*                                                                                               */
/*          ------------------------------------                                                 */
/*          |  x    |xx|        xxx         |xx|                                                 */
/*          ------------------------------------                                                 */
/*          ------------                                                                         */
/*          |    xx    |                                                                         */
/*          ------------                                                                         */
/*          ------------------------------------                                                 */
/*          |                   xxx            |                                                 */
/*          ------------------------------------                                                 */
/*************************************************************************************************/
void vog_lcdc_to_dsi_start( u2* ptt_vram_sta_addr, u1 u1t_lcd_mode )
{
	u4s_lcd_offset = U4G_ZERO;

	/* Selection of use LCDC register */
	if( U1G_LCD_SUB == u1t_lcd_mode )
	{
		u4s_lcd_offset = U4S_LCDC_OFFSET;
	}

	/* Main LCD soft-reset */
	(*(VU4 *)( U4S_LDCNT2R_ADDR + u4s_lcd_offset)) |= 0x00000200;
	
	u2 temp = (*(VU4 *)( U4S_LDCNT2R_ADDR + u4s_lcd_offset));
	while((temp & 0x00000200)==0x00000200)
	{
		temp = (*(VU4 *)( U4S_LDCNT2R_ADDR + u4s_lcd_offset));
	}
	
	/* Main LCD module-reset */
	(*(VU4 *)( U4S_LDCNT2R_ADDR + u4s_lcd_offset)) |= 0x00000100;
	
	temp = (*(VU4 *)( U4S_LDCNT2R_ADDR + u4s_lcd_offset));
	while((temp & 0x00000100)==0x00000100)
	{
		temp = (*(VU4 *)( U4S_LDCNT2R_ADDR + u4s_lcd_offset));
	}
	
	
	/* LCD/DE=0 set */
	(*(VU4 *)( U4S_LDCNT1R_ADDR + u4s_lcd_offset)) = 0x00000000;

	/* Main LCD/ME=0 set */
	(*(VU4 *)( U4S_LDCNT2R_ADDR + u4s_lcd_offset)) = 0x00000000;

	/* stop the dot clock */
	(*(VU4 *)( U4S_LDDCKSTPR_ADDR + u4s_lcd_offset)) = 0x00000001;

	/* Main LCD / Vsync,Hsync,DSIP=High(VPOL=0, HPOL=0, DIPOL=0)[MIPI-DSI—p] / HSCNT =0 / RGB Inter face(IFM=0) / 24bit(MIFTYP[3:0]=4'b1011)  */
	(*(VU4 *)( U4S_MLDMT1R_ADDR + u4s_lcd_offset)) = 0x0400000B;
	/* VSYNC output mode (LCDC outputs the VSYNC) */
	(*(VU4 *)( U4S_MLDMT2R_ADDR + u4s_lcd_offset)) = 0x04000000;

	/* DotCLK invalid as MIPI-DSI */
	(*(VU4 *)( U4S_MLDDCKPAT1R_ADDR + u4s_lcd_offset)) = 0x00000000;
	/* DotCLK invalid as MIPI-DSI */
	(*(VU4 *)( U4S_MLDDCKPAT2R_ADDR + u4s_lcd_offset)) = 0x00000000;
	/* ICKSEL=01: Selects the peripheral clock, MOSEL = DotCLK = 1/1 [MIPI-DSI] */
	(*(VU4 *)( U4S_LDDCKR_ADDR + u4s_lcd_offset)) = 0x00013C7C;

	/* HDCN=480pixel/8=60(0x3C), HTCN=(480+8+8+8+480)/8=123 */
	(*(VU4 *)( U4S_MLDHCNR_ADDR + u4s_lcd_offset)) = 0x003C0070;

	/* HSYNW=1*8dot, HSYNP=HDCN(480+8+480) */
	(*(VU4 *)( U4S_MLDHSYNR_ADDR + u4s_lcd_offset)) = 0x0001006E;

	/* HDCN_ADPIX=0, HTCN_ADPIX=7, HSYNW_ADPIX=1, HSYNP_ADPIX=2 */
	(*(VU4 *)( U4S_MLDHAJR_ADDR + u4s_lcd_offset)) = 0x00000000;

	/* VACT Active lines per frame WVGA 864H VBP Vertical back porch 4H */
	/* V Total Time 864+1+4+4=873(0x369) */
	(*(VU4 *)( U4S_MLDVLNR_ADDR + u4s_lcd_offset)) = 0x03600369;
	/* VACT Active lines per frame WVGA 864H VBP Vertical back porch 8H */


	/* VSA Vertical sync active 1H VFP Vertical front porch 4H */
	(*(VU4 *)( U4S_MLDVSYNR_ADDR + u4s_lcd_offset)) = 0x00010364;

	/* ITU-R BT.601 YCbCr format full-range YCbCr format [0, 255] 4:2:2 YCbCr */
	/* 16bit:RGB565 */
	(*(VU4 *)( U4S_MLDDFR_ADDR + u4s_lcd_offset)) = 0x00000003;

	/* Full screen size, Continuous mode */
	(*(VU4 *)( U4S_MLDSM1R_ADDR + u4s_lcd_offset)) = 0x00000000;
	/* Disable one frame of data read */
	(*(VU4 *)( U4S_MLDSM2R_ADDR + u4s_lcd_offset)) = 0x00000000;

	/* 1line Memory Size 960byte=480pixel*2 */
	(*(VU4 *)( U4S_MLDMLSR_ADDR + u4s_lcd_offset)) = 0x000003C0;

	/* LCD Display Data Read Start Address 1 Ydata*/
	(*(VU4 *)( U4S_MLDSA1R_ADDR + u4s_lcd_offset)) = (u4)ptt_vram_sta_addr;

	/* long word swap */
	if( U1G_LCD_MAIN == u1t_lcd_mode )
	{
		(*(VU4 *)( U4S_LDDDSR_ADDR + u4s_lcd_offset ) ) = 0x00000006;
	}
	else
	{
		(*(VU4 *)( U4S_LDDDSR_ADDR + u4s_lcd_offset ) ) = 0x00000007;
	}

	/* start the dot clock */
	(*(VU4 *)( U4S_LDDCKSTPR_ADDR + u4s_lcd_offset ) ) &= ~( 0x00000001 );
	while( ( (*(VU4 *)( U4S_LDDCKSTPR_ADDR + u4s_lcd_offset) ) & 0x00010000 ) != 0x00000000 );

	/* data on */
	(*(VU4 *)( U4S_LDCNT1R_ADDR + u4s_lcd_offset ) ) |= 0x00000001;
	/* disp on */
	(*(VU4 *)( U4S_LDCNT2R_ADDR + u4s_lcd_offset ) ) |= 0x00000003;
	
	return;
}

/*************************************************************************************************/
/* function : lcd_lcdc_stop( int lcd_mode )                                                      */
/* parameter: int lcd_mode(0:LCD_MAIN, 1:LCD_SUB)                                                */
/* return   : void                                                                               */
/* outline  : Stop to display LCD                                                                */
/*************************************************************************************************/
void vog_lcdc_to_stop( u1 u1t_lcd_mode )
{
	u4s_lcd_offset = U4G_ZERO;

	/* Selection of use LCDC register */
	if( U1G_LCD_SUB == u1t_lcd_mode )
	{
		u4s_lcd_offset = U4S_LCDC_OFFSET;
	}

	/* disp off */
	(*(VU4 *)( U4S_LDCNT2R_ADDR + u4s_lcd_offset ) ) &= ~0x00000003;
	/* data off */
	(*(VU4 *)( U4S_LDCNT1R_ADDR + u4s_lcd_offset ) ) &= ~0x00000001;

	/* stop the dot clock */
	(*(VU4 *)( U4S_LDDCKSTPR_ADDR + u4s_lcd_offset ) ) |= 0x00000001;
	while( ( ( *(VU4 *)( U4S_LDDCKSTPR_ADDR + u4s_lcd_offset ) ) & 0x00010000 ) == 0x00000000 );

	return;
}

/*************************************************************************************************/
/* function : vog_lcdc_int_wait( void )                                                          */
/* parameter: void                                                                               */
/* return   : void                                                                               */
/* outline  : LDINTR(bit6:Frame Start Interrupt Status)                                          */
/*************************************************************************************************/
void vog_lcdc_int_wait( void )
{

    /*The image data update timing is aimed at. (frame data transmission end interrupt)*/
	while( ( U4R_LDINTR & 0x0000004 ) != 0x0000004 );
	/* Interrupt clear */
	U4R_LDINTR &= ~0x0000004;

}
