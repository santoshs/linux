/*
 * cp1_m11_lcdc.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */
#ifndef __H_CP1_M11_LCDC_
#define __H_CP1_M11_LCDC_

/***************************************************************/
/* INCLUDE FILE                                                */
/***************************************************************/
#include "eos_system.h"				/* EOS-EVM-TP SYSTEM header */

/***************************************************************/
/* PUBLIC CONSTANT DEFINITON                                   */
/*-------------------------------------------------------------*/
/* LCD controller (LCDC)                                      */
/*-------------------------------------------------------------*/
/* LCDC_BADR:0xFE940000                                        */
/***************************************************************/
/* INDEX_M11_LCDC */
#define	U4R_MLDDCKPAT1R			(*(VU4 *)(LCDC_BADR+0x0400))		/* Main LCD Dot clock buffer setting register1 */
#define	U4R_MLDDCKPAT2R			(*(VU4 *)(LCDC_BADR+0x0404))		/* Main LCD Dot clock buffer setting register2 */
#define	U4R_SLDDCKPAT1R			(*(VU4 *)(LCDC_BADR+0x0408))		/* LCD Dot clock buffer setting register1 */
#define	U4R_SLDDCKPAT2R			(*(VU4 *)(LCDC_BADR+0x040C))		/* LCD Dot clock buffer setting register2 */
#define	U4R_LDDCKR				(*(VU4 *)(LCDC_BADR+0x0410))		/* LCDC dot clock register */
#define	U4R_LDDCKSTPR			(*(VU4 *)(LCDC_BADR+0x0414))		/* dot clock stop register */
#define	U4R_MLDMT1R				(*(VU4 *)(LCDC_BADR+0x0418))		/* Main LCD module type register1 */
#define	U4R_MLDMT2R				(*(VU4 *)(LCDC_BADR+0x041C))		/* Main LCD module type register2 */
#define	U4R_MLDMT3R				(*(VU4 *)(LCDC_BADR+0x0420))		/* Main LCD module type register3 */
#define	U4R_MLDDFR				(*(VU4 *)(LCDC_BADR+0x0424))		/* Main LCD Data format register */
#define	U4R_MLDSM1R				(*(VU4 *)(LCDC_BADR+0x0428))		/* Main LCD Scanning mode register1 */
#define	U4R_MLDSM2R				(*(VU4 *)(LCDC_BADR+0x042C))		/* Main LCD Scanning mode register2 */
#define	U4R_MLDSA1R				(*(VU4 *)(LCDC_BADR+0x0430))		/* Main LCD The displayed data start address register1 */
#define	U4R_MLDSA2R				(*(VU4 *)(LCDC_BADR+0x0434))		/* Main LCD The displayed data start address register2 */
#define	U4R_MLDMLSR				(*(VU4 *)(LCDC_BADR+0x0438))		/* Main LCD Display data storage memory line size register */
#define	U4R_MLDWBFR				(*(VU4 *)(LCDC_BADR+0x043C))		/* Main LCD Writing return data format register */
#define	U4R_MLDWBCNTR			(*(VU4 *)(LCDC_BADR+0x0440))		/* Main LCD Writing return control register */
#define	U4R_MLDWBAR				(*(VU4 *)(LCDC_BADR+0x0444))		/* Main LCD Start address register of writing return destination */
#define	U4R_MLDHCNR				(*(VU4 *)(LCDC_BADR+0x0448))		/* Main LCD The horizontal character number register */
#define	U4R_MLDHSYNR			(*(VU4 *)(LCDC_BADR+0x044C))		/* Main LCD Horizontal synchronizing signal register */
#define	U4R_MLDVLNR				(*(VU4 *)(LCDC_BADR+0x0450))		/* Main LCD Vertical line number register */
#define	U4R_MLDVSYNR			(*(VU4 *)(LCDC_BADR+0x0454))		/* Main LCD Vertical synchronizing signal register */
#define	U4R_MLDHPDR				(*(VU4 *)(LCDC_BADR+0x0458))		/* Main LCD The horizontal, partial screen register */
#define	U4R_MLDVPDR				(*(VU4 *)(LCDC_BADR+0x045C))		/* Main LCD Vertical, partial screen register */
#define	U4R_MLDPMR				(*(VU4 *)(LCDC_BADR+0x0460))		/* Main LCD Power management register */
#define	U4R_LDPALCR				(*(VU4 *)(LCDC_BADR+0x0464))		/* LCDC Palette control register */
#define	U4R_LDINTR				(*(VU4 *)(LCDC_BADR+0x0468))		/* LCDC Interrupt register */
#define	U4R_LDSR				(*(VU4 *)(LCDC_BADR+0x046C))		/* LCDC Status register */
#define	U4R_LDCNT1R				(*(VU4 *)(LCDC_BADR+0x0470))		/* LCDC Control register1 */
#define	U4R_LDCNT2R				(*(VU4 *)(LCDC_BADR+0x0474))		/* LCDC Control register2 */
#define	U4R_LDRCNTR				(*(VU4 *)(LCDC_BADR+0x0478))		/* LCDC Register side control register */
#define	U4R_LDDDSR				(*(VU4 *)(LCDC_BADR+0x047C))		/* LCDC Input image data swap register */
#define	U4R_LDRCR				(*(VU4 *)(LCDC_BADR+0x0484))		/* LCDC Compulsion on register side specification register */
#define	U4R_LDDBSLMR			(*(VU4 *)(LCDC_BADR+0x0490))		/* Doubler mode register */
#define	U4R_LDSLHPNR			(*(VU4 *)(LCDC_BADR+0x0494))		/* Register of number of horizontal slides */
#define	U4R_LDSLVLNR			(*(VU4 *)(LCDC_BADR+0x0498))		/* Register of number of vertical slides */
#define	U4R_LDSLRGBR			(*(VU4 *)(LCDC_BADR+0x049C))		/* Slide data register */
#define	U4R_MLDHAJR				(*(VU4 *)(LCDC_BADR+0x04A0))		/* Main LCD Horizontal synchronizing signal adjustment register */
#define	U4R_MLDIVSNR			(*(VU4 *)(LCDC_BADR+0x04A4))		/* Main LCD Vertical synchronizing signal adjustment register */
#define	U4R_SLDMT1R				(*(VU4 *)(LCDC_BADR+0x0600))		/* LCD module type register1 */
#define	U4R_SLDMT2R				(*(VU4 *)(LCDC_BADR+0x0604))		/* LCD module type register2 */
#define	U4R_SLDMT3R				(*(VU4 *)(LCDC_BADR+0x0608))		/* LCD module type register3 */
#define	U4R_SLDDFR				(*(VU4 *)(LCDC_BADR+0x060C))		/* LCD Data format register */
#define	U4R_SLDSM1R				(*(VU4 *)(LCDC_BADR+0x0610))		/* LCD Scanning mode register1 */
#define	U4R_SLDSM2R				(*(VU4 *)(LCDC_BADR+0x0614))		/* LCD Scanning mode register2 */
#define	U4R_SLDSA1R				(*(VU4 *)(LCDC_BADR+0x0618))		/* LCD The displayed data start address register1 */
#define	U4R_SLDSA2R				(*(VU4 *)(LCDC_BADR+0x061C))		/* LCD The displayed data start address register2 */
#define	U4R_SLDMLSR				(*(VU4 *)(LCDC_BADR+0x0620))		/* LCD Display data storage memory line size register */
#define	U4R_SLDHCNR				(*(VU4 *)(LCDC_BADR+0x0624))		/* LCD The horizontal character number register */
#define	U4R_SLDHSYNR			(*(VU4 *)(LCDC_BADR+0x0628))		/* LCD The horizontal, synchronous timing register */
#define	U4R_SLDVLNR				(*(VU4 *)(LCDC_BADR+0x062C))		/* LCD Vertical line number register */
#define	U4R_SLDVSYNR			(*(VU4 *)(LCDC_BADR+0x0630))		/* LCD Vertical synchronizing signal register */
#define	U4R_SLDHPDR				(*(VU4 *)(LCDC_BADR+0x0634))		/* LCD The horizontal, partial screen register */
#define	U4R_SLDVPDR				(*(VU4 *)(LCDC_BADR+0x0638))		/* LCD Vertical, partial screen register */
#define	U4R_SLDPMR				(*(VU4 *)(LCDC_BADR+0x063C))		/* LCD Power management register */
#define	U4R_LDDWD0R				(*(VU4 *)(LCDC_BADR+0x0800))		/* LCDC driver write data register 0 */
#define	U4R_LDDWD1R				(*(VU4 *)(LCDC_BADR+0x0804))		/* LCDC driver write data register 1 */
#define	U4R_LDDWD2R				(*(VU4 *)(LCDC_BADR+0x0808))		/* LCDC driver write data register 2 */
#define	U4R_LDDWD3R				(*(VU4 *)(LCDC_BADR+0x080C))		/* LCDC driver write data register 3 */
#define	U4R_LDDWD4R				(*(VU4 *)(LCDC_BADR+0x0810))		/* LCDC driver write data register 4 */
#define	U4R_LDDWD5R				(*(VU4 *)(LCDC_BADR+0x0814))		/* LCDC driver write data register 5 */
#define	U4R_LDDWD6R				(*(VU4 *)(LCDC_BADR+0x0818))		/* LCDC driver write data register 6 */
#define	U4R_LDDWD7R				(*(VU4 *)(LCDC_BADR+0x081C))		/* LCDC driver write data register 7 */
#define	U4R_LDDWD8R				(*(VU4 *)(LCDC_BADR+0x0820))		/* LCDC driver write data register 8 */
#define	U4R_LDDWD9R				(*(VU4 *)(LCDC_BADR+0x0824))		/* LCDC driver write data register 9 */
#define	U4R_LDDWDAR				(*(VU4 *)(LCDC_BADR+0x0828))		/* LCDC driver write data register A */
#define	U4R_LDDWDBR				(*(VU4 *)(LCDC_BADR+0x082C))		/* LCDC driver write data register B */
#define	U4R_LDDWDCR				(*(VU4 *)(LCDC_BADR+0x0830))		/* LCDC driver write data register C */
#define	U4R_LDDWDDR				(*(VU4 *)(LCDC_BADR+0x0834))		/* LCDC driver write data register D */
#define	U4R_LDDWDER				(*(VU4 *)(LCDC_BADR+0x0838))		/* LCDC driver write data register E */
#define	U4R_LDDWDFR				(*(VU4 *)(LCDC_BADR+0x083C))		/* LCDC driver write data register F */
#define	U4R_LDDRDR				(*(VU4 *)(LCDC_BADR+0x0840))		
#define	U4R_LDDWAR				(*(VU4 *)(LCDC_BADR+0x0900))		
#define	U4R_LDDRAR				(*(VU4 *)(LCDC_BADR+0x0904))		
#define	U4R_LDCMRKRGBR			(*(VU4 *)(LCDC_BADR+0x0A00))		/* LCDC Color management matrix R Coefficient RGB register  */
#define	U4R_LDCMRKCMYR			(*(VU4 *)(LCDC_BADR+0x0A04))		/* LCDC Color management matrix R Coefficient CMY register  */
#define	U4R_LDCMRK1R			(*(VU4 *)(LCDC_BADR+0x0A08))		/* LCDC Color management matrix R Register between coefficient hue 1 */
#define	U4R_LDCMRK2R			(*(VU4 *)(LCDC_BADR+0x0A0C))		/* LCDC Color management matrix R Register between coefficient hue 2 */
#define	U4R_LDCMGKRGBR			(*(VU4 *)(LCDC_BADR+0x0A10))		/* LCDC Color management matrix G Coefficient RGB register  */
#define	U4R_LDCMGKCMYR			(*(VU4 *)(LCDC_BADR+0x0A14))		/* LCDC Color management matrix G Coefficient CMY register  */
#define	U4R_LDCMGK1R			(*(VU4 *)(LCDC_BADR+0x0A18))		/* LCDC Color management matrix G Register between coefficient hue 1 */
#define	U4R_LDCMGK2R			(*(VU4 *)(LCDC_BADR+0x0A1C))		/* LCDC Color management matrix G Register between coefficient hue 2 */
#define	U4R_LDCMBKRGBR			(*(VU4 *)(LCDC_BADR+0x0A20))		/* LCDC Color management matrix B Coefficient RGB register  */
#define	U4R_LDCMBKCMYR			(*(VU4 *)(LCDC_BADR+0x0A24))		/* LCDC Color management matrix B Coefficient CMY register  */
#define	U4R_LDCMBK1R			(*(VU4 *)(LCDC_BADR+0x0A28))		/* LCDC Color management matrix B Register between coefficient hue 1 */
#define	U4R_LDCMBK2R			(*(VU4 *)(LCDC_BADR+0x0A2C))		/* LCDC Color management matrix B Register between coefficient hue 2 */
#define	U4R_LDCMHKPR			(*(VU4 *)(LCDC_BADR+0x0A30))		
#define	U4R_LDCMHKQR			(*(VU4 *)(LCDC_BADR+0x0A34))		
#define	U4R_LDCMSELR			(*(VU4 *)(LCDC_BADR+0x0A38))		
#define	U4R_LDCMTVR				(*(VU4 *)(LCDC_BADR+0x0A3C))		
#define	U4R_LDCMTVSELR			(*(VU4 *)(LCDC_BADR+0x0A40))		
#define	U4R_LDCMDTHR			(*(VU4 *)(LCDC_BADR+0x0A44))		
#define	U4R_LDCMCNTR			(*(VU4 *)(LCDC_BADR+0x0A48))		
#define	U4R_LDPALECR			(*(VU4 *)(LCDC_BADR+0x0A50))		
#define	U4R_LDPALADD			(*(VU4 *)(LCDC_BADR+0x0A54))		
#define	U4R_LDBCR				(*(VU4 *)(LCDC_BADR+0x0B00))		
#define	U4R_LDB1BSIFR			(*(VU4 *)(LCDC_BADR+0x0B20))		/* CH1 Source image format register  */
#define	U4R_LDB1BSSZR			(*(VU4 *)(LCDC_BADR+0x0B24))		/* CH1 Source size specification egister  */
#define	U4R_LDB1BLOCR			(*(VU4 *)(LCDC_BADR+0x0B28))		/* CH1 Blend location setting register  */
#define	U4R_LDB1BSMWR			(*(VU4 *)(LCDC_BADR+0x0B2C))		/* CH1 Source memory width specificationregister  */
#define	U4R_LDB1BSARY			(*(VU4 *)(LCDC_BADR+0x0B30))		/* CH1Source address Y register  */
#define	U4R_LDB1BSARC			(*(VU4 *)(LCDC_BADR+0x0B34))		/* CH1Source address C register  */
#define	U4R_LDB1BSARA			(*(VU4 *)(LCDC_BADR+0x0B38))		/* CH1Source address Éøregister  */
#define	U4R_LDB1BPPCR			(*(VU4 *)(LCDC_BADR+0x0B3C))		/* CH1 color control register  */
#define	U4R_LDB1BBGCL			(*(VU4 *)(LCDC_BADR+0x0B10))		/* CH1 BGCOLOR register  */
#define	U4R_LDB2BSIFR			(*(VU4 *)(LCDC_BADR+0x0B40))		/* CH2Source image format register  */
#define	U4R_LDB2BSSZR			(*(VU4 *)(LCDC_BADR+0x0B44))		/* CH2 Source size specification egister  */
#define	U4R_LDB2BLOCR			(*(VU4 *)(LCDC_BADR+0x0B48))		/* CH2 Blend location setting register  */
#define	U4R_LDB2BSMWR			(*(VU4 *)(LCDC_BADR+0x0B4C))		/* CH2 Source memory width specificationregister  */
#define	U4R_LDB2BSARY			(*(VU4 *)(LCDC_BADR+0x0B50))		/* CH2 Source address Y register  */
#define	U4R_LDB2BSARC			(*(VU4 *)(LCDC_BADR+0x0B54))		/* CH2 Source address C register  */
#define	U4R_LDB2BSARA			(*(VU4 *)(LCDC_BADR+0x0B58))		/* CH2 Source address Éøregister  */
#define	U4R_LDB2BPPCR			(*(VU4 *)(LCDC_BADR+0x0B5C))		/* CH2 color control register  */
#define	U4R_LDB2BBGCL			(*(VU4 *)(LCDC_BADR+0x0B14))		/* CH2 BGCOLOR register  */
#define	U4R_LDB3BSIFR			(*(VU4 *)(LCDC_BADR+0x0B60))		/* CH3 Source image format register  */
#define	U4R_LDB3BSSZR			(*(VU4 *)(LCDC_BADR+0x0B64))		/* CH3 Source size specification egister  */
#define	U4R_LDB3BLOCR			(*(VU4 *)(LCDC_BADR+0x0B68))		/* CH3 Blend location setting register  */
#define	U4R_LDB3BSMWR			(*(VU4 *)(LCDC_BADR+0x0B6C))		/* CH3 Source memory width specificationregister  */
#define	U4R_LDB3BSARY			(*(VU4 *)(LCDC_BADR+0x0B70))		/* CH3 Source address Y register  */
#define	U4R_LDB3BSARC			(*(VU4 *)(LCDC_BADR+0x0B74))		/* CH3 Source address C register  */
#define	U4R_LDB3BSARA			(*(VU4 *)(LCDC_BADR+0x0B78))		/* CH3 Source address Éøregister  */
#define	U4R_LDB3BPPCR			(*(VU4 *)(LCDC_BADR+0x0B7C))		/* CH3 color control register  */
#define	U4R_LDB3BBGCL			(*(VU4 *)(LCDC_BADR+0x0B18))		/* CH3 BGCOLOR register  */
#define	U4R_LDB4BSIFR			(*(VU4 *)(LCDC_BADR+0x0B80))		/* CH4 Source image format register  */
#define	U4R_LDB4BSSZR			(*(VU4 *)(LCDC_BADR+0x0B84))		/* CH4 Source size specification egister  */
#define	U4R_LDB4BLOCR			(*(VU4 *)(LCDC_BADR+0x0B88))		/* CH4 Blend location setting register  */
#define	U4R_LDB4BSMWR			(*(VU4 *)(LCDC_BADR+0x0B8C))		/* CH4 Source memory width specificationregister  */
#define	U4R_LDB4BSARY			(*(VU4 *)(LCDC_BADR+0x0B90))		/* CH4 Source address Y register  */
#define	U4R_LDB4BSARC			(*(VU4 *)(LCDC_BADR+0x0B94))		/* CH4 Source address C register  */
#define	U4R_LDB4BSARA			(*(VU4 *)(LCDC_BADR+0x0B98))		/* CH4 Source address Éøregister  */
#define	U4R_LDB4BPPCR			(*(VU4 *)(LCDC_BADR+0x0B9C))		/* CH4 color control register  */
#define	U4R_LDB4BBGCL			(*(VU4 *)(LCDC_BADR+0x0B1C))		/* CH4 BGCOLOR register  */
#define	U4R_LDPR00				(*(VU4 *)(LCDC_BADR+0x0000))		/* LCDC Palette data register 000 */
#define	U4R_LDPR01				(*(VU4 *)(LCDC_BADR+0x0004))		/* LCDC Palette data register 001 */
#define	U4R_LDPR02				(*(VU4 *)(LCDC_BADR+0x0008))		/* LCDC Palette data register 002 */
#define	U4R_LDPR03				(*(VU4 *)(LCDC_BADR+0x000C))		/* LCDC Palette data register 003 */
#define	U4R_LDPR04				(*(VU4 *)(LCDC_BADR+0x0010))		/* LCDC Palette data register 004 */
#define	U4R_LDPR05				(*(VU4 *)(LCDC_BADR+0x0014))		/* LCDC Palette data register 005 */
#define	U4R_LDPR06				(*(VU4 *)(LCDC_BADR+0x0018))		/* LCDC Palette data register 006 */
#define	U4R_LDPR07				(*(VU4 *)(LCDC_BADR+0x001C))		/* LCDC Palette data register 007 */
#define	U4R_LDPR08				(*(VU4 *)(LCDC_BADR+0x0020))		/* LCDC Palette data register 008 */
#define	U4R_LDPR09				(*(VU4 *)(LCDC_BADR+0x0024))		/* LCDC Palette data register 009 */
#define	U4R_LDPR0A				(*(VU4 *)(LCDC_BADR+0x0028))		/* LCDC Palette data register 010 */
#define	U4R_LDPR0B				(*(VU4 *)(LCDC_BADR+0x002C))		/* LCDC Palette data register 011 */
#define	U4R_LDPR0C				(*(VU4 *)(LCDC_BADR+0x0030))		/* LCDC Palette data register 012 */
#define	U4R_LDPR0D				(*(VU4 *)(LCDC_BADR+0x0034))		/* LCDC Palette data register 013 */
#define	U4R_LDPR0E				(*(VU4 *)(LCDC_BADR+0x0038))		/* LCDC Palette data register 014 */
#define	U4R_LDPR0F				(*(VU4 *)(LCDC_BADR+0x003C))		/* LCDC Palette data register 015 */
#define	U4R_LDPR10				(*(VU4 *)(LCDC_BADR+0x0040))		/* LCDC Palette data register 016 */
#define	U4R_LDPR11				(*(VU4 *)(LCDC_BADR+0x0044))		/* LCDC Palette data register 017 */
#define	U4R_LDPR12				(*(VU4 *)(LCDC_BADR+0x0048))		/* LCDC Palette data register 018 */
#define	U4R_LDPR13				(*(VU4 *)(LCDC_BADR+0x004C))		/* LCDC Palette data register 019 */
#define	U4R_LDPR14				(*(VU4 *)(LCDC_BADR+0x0050))		/* LCDC Palette data register 020 */
#define	U4R_LDPR15				(*(VU4 *)(LCDC_BADR+0x0054))		/* LCDC Palette data register 021 */
#define	U4R_LDPR16				(*(VU4 *)(LCDC_BADR+0x0058))		/* LCDC Palette data register 022 */
#define	U4R_LDPR17				(*(VU4 *)(LCDC_BADR+0x005C))		/* LCDC Palette data register 023 */
#define	U4R_LDPR18				(*(VU4 *)(LCDC_BADR+0x0060))		/* LCDC Palette data register 024 */
#define	U4R_LDPR19				(*(VU4 *)(LCDC_BADR+0x0064))		/* LCDC Palette data register 025 */
#define	U4R_LDPR1A				(*(VU4 *)(LCDC_BADR+0x0068))		/* LCDC Palette data register 026 */
#define	U4R_LDPR1B				(*(VU4 *)(LCDC_BADR+0x006C))		/* LCDC Palette data register 027 */
#define	U4R_LDPR1C				(*(VU4 *)(LCDC_BADR+0x0070))		/* LCDC Palette data register 028 */
#define	U4R_LDPR1D				(*(VU4 *)(LCDC_BADR+0x0074))		/* LCDC Palette data register 029 */
#define	U4R_LDPR1E				(*(VU4 *)(LCDC_BADR+0x0078))		/* LCDC Palette data register 030 */
#define	U4R_LDPR1F				(*(VU4 *)(LCDC_BADR+0x007C))		/* LCDC Palette data register 031 */
#define	U4R_LDPR20				(*(VU4 *)(LCDC_BADR+0x0080))		/* LCDC Palette data register 032 */
#define	U4R_LDPR21				(*(VU4 *)(LCDC_BADR+0x0084))		/* LCDC Palette data register 033 */
#define	U4R_LDPR22				(*(VU4 *)(LCDC_BADR+0x0088))		/* LCDC Palette data register 034 */
#define	U4R_LDPR23				(*(VU4 *)(LCDC_BADR+0x008C))		/* LCDC Palette data register 035 */
#define	U4R_LDPR24				(*(VU4 *)(LCDC_BADR+0x0090))		/* LCDC Palette data register 036 */
#define	U4R_LDPR25				(*(VU4 *)(LCDC_BADR+0x0094))		/* LCDC Palette data register 037 */
#define	U4R_LDPR26				(*(VU4 *)(LCDC_BADR+0x0098))		/* LCDC Palette data register 038 */
#define	U4R_LDPR27				(*(VU4 *)(LCDC_BADR+0x009C))		/* LCDC Palette data register 039 */
#define	U4R_LDPR28				(*(VU4 *)(LCDC_BADR+0x00A0))		/* LCDC Palette data register 040 */
#define	U4R_LDPR29				(*(VU4 *)(LCDC_BADR+0x00A4))		/* LCDC Palette data register 041 */
#define	U4R_LDPR2A				(*(VU4 *)(LCDC_BADR+0x00A8))		/* LCDC Palette data register 042 */
#define	U4R_LDPR2B				(*(VU4 *)(LCDC_BADR+0x00AC))		/* LCDC Palette data register 043 */
#define	U4R_LDPR2C				(*(VU4 *)(LCDC_BADR+0x00B0))		/* LCDC Palette data register 044 */
#define	U4R_LDPR2D				(*(VU4 *)(LCDC_BADR+0x00B4))		/* LCDC Palette data register 045 */
#define	U4R_LDPR2E				(*(VU4 *)(LCDC_BADR+0x00B8))		/* LCDC Palette data register 046 */
#define	U4R_LDPR2F				(*(VU4 *)(LCDC_BADR+0x00BC))		/* LCDC Palette data register 047 */
#define	U4R_LDPR30				(*(VU4 *)(LCDC_BADR+0x00C0))		/* LCDC Palette data register 048 */
#define	U4R_LDPR31				(*(VU4 *)(LCDC_BADR+0x00C4))		/* LCDC Palette data register 049 */
#define	U4R_LDPR32				(*(VU4 *)(LCDC_BADR+0x00C8))		/* LCDC Palette data register 050 */
#define	U4R_LDPR33				(*(VU4 *)(LCDC_BADR+0x00CC))		/* LCDC Palette data register 051 */
#define	U4R_LDPR34				(*(VU4 *)(LCDC_BADR+0x00D0))		/* LCDC Palette data register 052 */
#define	U4R_LDPR35				(*(VU4 *)(LCDC_BADR+0x00D4))		/* LCDC Palette data register 053 */
#define	U4R_LDPR36				(*(VU4 *)(LCDC_BADR+0x00D8))		/* LCDC Palette data register 054 */
#define	U4R_LDPR37				(*(VU4 *)(LCDC_BADR+0x00DC))		/* LCDC Palette data register 055 */
#define	U4R_LDPR38				(*(VU4 *)(LCDC_BADR+0x00E0))		/* LCDC Palette data register 056 */
#define	U4R_LDPR39				(*(VU4 *)(LCDC_BADR+0x00E4))		/* LCDC Palette data register 057 */
#define	U4R_LDPR3A				(*(VU4 *)(LCDC_BADR+0x00E8))		/* LCDC Palette data register 058 */
#define	U4R_LDPR3B				(*(VU4 *)(LCDC_BADR+0x00EC))		/* LCDC Palette data register 059 */
#define	U4R_LDPR3C				(*(VU4 *)(LCDC_BADR+0x00F0))		/* LCDC Palette data register 060 */
#define	U4R_LDPR3D				(*(VU4 *)(LCDC_BADR+0x00F4))		/* LCDC Palette data register 061 */
#define	U4R_LDPR3E				(*(VU4 *)(LCDC_BADR+0x00F8))		/* LCDC Palette data register 062 */
#define	U4R_LDPR3F				(*(VU4 *)(LCDC_BADR+0x00FC))		/* LCDC Palette data register 063 */
#define	U4R_LDPR40				(*(VU4 *)(LCDC_BADR+0x0100))		/* LCDC Palette data register 064 */
#define	U4R_LDPR41				(*(VU4 *)(LCDC_BADR+0x0104))		/* LCDC Palette data register 065 */
#define	U4R_LDPR42				(*(VU4 *)(LCDC_BADR+0x0108))		/* LCDC Palette data register 066 */
#define	U4R_LDPR43				(*(VU4 *)(LCDC_BADR+0x010C))		/* LCDC Palette data register 067 */
#define	U4R_LDPR44				(*(VU4 *)(LCDC_BADR+0x0110))		/* LCDC Palette data register 068 */
#define	U4R_LDPR45				(*(VU4 *)(LCDC_BADR+0x0114))		/* LCDC Palette data register 069 */
#define	U4R_LDPR46				(*(VU4 *)(LCDC_BADR+0x0118))		/* LCDC Palette data register 070 */
#define	U4R_LDPR47				(*(VU4 *)(LCDC_BADR+0x011C))		/* LCDC Palette data register 071 */
#define	U4R_LDPR48				(*(VU4 *)(LCDC_BADR+0x0120))		/* LCDC Palette data register 072 */
#define	U4R_LDPR49				(*(VU4 *)(LCDC_BADR+0x0124))		/* LCDC Palette data register 073 */
#define	U4R_LDPR4A				(*(VU4 *)(LCDC_BADR+0x0128))		/* LCDC Palette data register 074 */
#define	U4R_LDPR4B				(*(VU4 *)(LCDC_BADR+0x012C))		/* LCDC Palette data register 075 */
#define	U4R_LDPR4C				(*(VU4 *)(LCDC_BADR+0x0130))		/* LCDC Palette data register 076 */
#define	U4R_LDPR4D				(*(VU4 *)(LCDC_BADR+0x0134))		/* LCDC Palette data register 077 */
#define	U4R_LDPR4E				(*(VU4 *)(LCDC_BADR+0x0138))		/* LCDC Palette data register 078 */
#define	U4R_LDPR4F				(*(VU4 *)(LCDC_BADR+0x013C))		/* LCDC Palette data register 079 */
#define	U4R_LDPR50				(*(VU4 *)(LCDC_BADR+0x0140))		/* LCDC Palette data register 080 */
#define	U4R_LDPR51				(*(VU4 *)(LCDC_BADR+0x0144))		/* LCDC Palette data register 081 */
#define	U4R_LDPR52				(*(VU4 *)(LCDC_BADR+0x0148))		/* LCDC Palette data register 082 */
#define	U4R_LDPR53				(*(VU4 *)(LCDC_BADR+0x014C))		/* LCDC Palette data register 083 */
#define	U4R_LDPR54				(*(VU4 *)(LCDC_BADR+0x0150))		/* LCDC Palette data register 084 */
#define	U4R_LDPR55				(*(VU4 *)(LCDC_BADR+0x0154))		/* LCDC Palette data register 085 */
#define	U4R_LDPR56				(*(VU4 *)(LCDC_BADR+0x0158))		/* LCDC Palette data register 086 */
#define	U4R_LDPR57				(*(VU4 *)(LCDC_BADR+0x015C))		/* LCDC Palette data register 087 */
#define	U4R_LDPR58				(*(VU4 *)(LCDC_BADR+0x0160))		/* LCDC Palette data register 088 */
#define	U4R_LDPR59				(*(VU4 *)(LCDC_BADR+0x0164))		/* LCDC Palette data register 089 */
#define	U4R_LDPR5A				(*(VU4 *)(LCDC_BADR+0x0168))		/* LCDC Palette data register 090 */
#define	U4R_LDPR5B				(*(VU4 *)(LCDC_BADR+0x016C))		/* LCDC Palette data register 091 */
#define	U4R_LDPR5C				(*(VU4 *)(LCDC_BADR+0x0170))		/* LCDC Palette data register 092 */
#define	U4R_LDPR5D				(*(VU4 *)(LCDC_BADR+0x0174))		/* LCDC Palette data register 093 */
#define	U4R_LDPR5E				(*(VU4 *)(LCDC_BADR+0x0178))		/* LCDC Palette data register 094 */
#define	U4R_LDPR5F				(*(VU4 *)(LCDC_BADR+0x017C))		/* LCDC Palette data register 095 */
#define	U4R_LDPR60				(*(VU4 *)(LCDC_BADR+0x0180))		/* LCDC Palette data register 096 */
#define	U4R_LDPR61				(*(VU4 *)(LCDC_BADR+0x0184))		/* LCDC Palette data register 097 */
#define	U4R_LDPR62				(*(VU4 *)(LCDC_BADR+0x0188))		/* LCDC Palette data register 098 */
#define	U4R_LDPR63				(*(VU4 *)(LCDC_BADR+0x018C))		/* LCDC Palette data register 099 */
#define	U4R_LDPR64				(*(VU4 *)(LCDC_BADR+0x0190))		/* LCDC Palette data register 100 */
#define	U4R_LDPR65				(*(VU4 *)(LCDC_BADR+0x0194))		/* LCDC Palette data register 101 */
#define	U4R_LDPR66				(*(VU4 *)(LCDC_BADR+0x0198))		/* LCDC Palette data register 102 */
#define	U4R_LDPR67				(*(VU4 *)(LCDC_BADR+0x019C))		/* LCDC Palette data register 103 */
#define	U4R_LDPR68				(*(VU4 *)(LCDC_BADR+0x01A0))		/* LCDC Palette data register 104 */
#define	U4R_LDPR69				(*(VU4 *)(LCDC_BADR+0x01A4))		/* LCDC Palette data register 105 */
#define	U4R_LDPR6A				(*(VU4 *)(LCDC_BADR+0x01A8))		/* LCDC Palette data register 106 */
#define	U4R_LDPR6B				(*(VU4 *)(LCDC_BADR+0x01AC))		/* LCDC Palette data register 107 */
#define	U4R_LDPR6C				(*(VU4 *)(LCDC_BADR+0x01B0))		/* LCDC Palette data register 108 */
#define	U4R_LDPR6D				(*(VU4 *)(LCDC_BADR+0x01B4))		/* LCDC Palette data register 109 */
#define	U4R_LDPR6E				(*(VU4 *)(LCDC_BADR+0x01B8))		/* LCDC Palette data register 110 */
#define	U4R_LDPR6F				(*(VU4 *)(LCDC_BADR+0x01BC))		/* LCDC Palette data register 111 */
#define	U4R_LDPR70				(*(VU4 *)(LCDC_BADR+0x01C0))		/* LCDC Palette data register 112 */
#define	U4R_LDPR71				(*(VU4 *)(LCDC_BADR+0x01C4))		/* LCDC Palette data register 113 */
#define	U4R_LDPR72				(*(VU4 *)(LCDC_BADR+0x01C8))		/* LCDC Palette data register 114 */
#define	U4R_LDPR73				(*(VU4 *)(LCDC_BADR+0x01CC))		/* LCDC Palette data register 115 */
#define	U4R_LDPR74				(*(VU4 *)(LCDC_BADR+0x01D0))		/* LCDC Palette data register 116 */
#define	U4R_LDPR75				(*(VU4 *)(LCDC_BADR+0x01D4))		/* LCDC Palette data register 117 */
#define	U4R_LDPR76				(*(VU4 *)(LCDC_BADR+0x01D8))		/* LCDC Palette data register 118 */
#define	U4R_LDPR77				(*(VU4 *)(LCDC_BADR+0x01DC))		/* LCDC Palette data register 119 */
#define	U4R_LDPR78				(*(VU4 *)(LCDC_BADR+0x01E0))		/* LCDC Palette data register 120 */
#define	U4R_LDPR79				(*(VU4 *)(LCDC_BADR+0x01E4))		/* LCDC Palette data register 121 */
#define	U4R_LDPR7A				(*(VU4 *)(LCDC_BADR+0x01E8))		/* LCDC Palette data register 122 */
#define	U4R_LDPR7B				(*(VU4 *)(LCDC_BADR+0x01EC))		/* LCDC Palette data register 123 */
#define	U4R_LDPR7C				(*(VU4 *)(LCDC_BADR+0x01F0))		/* LCDC Palette data register 124 */
#define	U4R_LDPR7D				(*(VU4 *)(LCDC_BADR+0x01F4))		/* LCDC Palette data register 125 */
#define	U4R_LDPR7E				(*(VU4 *)(LCDC_BADR+0x01F8))		/* LCDC Palette data register 126 */
#define	U4R_LDPR7F				(*(VU4 *)(LCDC_BADR+0x01FC))		/* LCDC Palette data register 127 */
#define	U4R_LDPR80				(*(VU4 *)(LCDC_BADR+0x0200))		/* LCDC Palette data register 128 */
#define	U4R_LDPR81				(*(VU4 *)(LCDC_BADR+0x0204))		/* LCDC Palette data register 129 */
#define	U4R_LDPR82				(*(VU4 *)(LCDC_BADR+0x0208))		/* LCDC Palette data register 130 */
#define	U4R_LDPR83				(*(VU4 *)(LCDC_BADR+0x020C))		/* LCDC Palette data register 131 */
#define	U4R_LDPR84				(*(VU4 *)(LCDC_BADR+0x0210))		/* LCDC Palette data register 132 */
#define	U4R_LDPR85				(*(VU4 *)(LCDC_BADR+0x0214))		/* LCDC Palette data register 133 */
#define	U4R_LDPR86				(*(VU4 *)(LCDC_BADR+0x0218))		/* LCDC Palette data register 134 */
#define	U4R_LDPR87				(*(VU4 *)(LCDC_BADR+0x021C))		/* LCDC Palette data register 135 */
#define	U4R_LDPR88				(*(VU4 *)(LCDC_BADR+0x0220))		/* LCDC Palette data register 136 */
#define	U4R_LDPR89				(*(VU4 *)(LCDC_BADR+0x0224))		/* LCDC Palette data register 137 */
#define	U4R_LDPR8A				(*(VU4 *)(LCDC_BADR+0x0228))		/* LCDC Palette data register 138 */
#define	U4R_LDPR8B				(*(VU4 *)(LCDC_BADR+0x022C))		/* LCDC Palette data register 139 */
#define	U4R_LDPR8C				(*(VU4 *)(LCDC_BADR+0x0230))		/* LCDC Palette data register 140 */
#define	U4R_LDPR8D				(*(VU4 *)(LCDC_BADR+0x0234))		/* LCDC Palette data register 141 */
#define	U4R_LDPR8E				(*(VU4 *)(LCDC_BADR+0x0238))		/* LCDC Palette data register 142 */
#define	U4R_LDPR8F				(*(VU4 *)(LCDC_BADR+0x023C))		/* LCDC Palette data register 143 */
#define	U4R_LDPR90				(*(VU4 *)(LCDC_BADR+0x0240))		/* LCDC Palette data register 144 */
#define	U4R_LDPR91				(*(VU4 *)(LCDC_BADR+0x0244))		/* LCDC Palette data register 145 */
#define	U4R_LDPR92				(*(VU4 *)(LCDC_BADR+0x0248))		/* LCDC Palette data register 146 */
#define	U4R_LDPR93				(*(VU4 *)(LCDC_BADR+0x024C))		/* LCDC Palette data register 147 */
#define	U4R_LDPR94				(*(VU4 *)(LCDC_BADR+0x0250))		/* LCDC Palette data register 148 */
#define	U4R_LDPR95				(*(VU4 *)(LCDC_BADR+0x0254))		/* LCDC Palette data register 149 */
#define	U4R_LDPR96				(*(VU4 *)(LCDC_BADR+0x0258))		/* LCDC Palette data register 150 */
#define	U4R_LDPR97				(*(VU4 *)(LCDC_BADR+0x025C))		/* LCDC Palette data register 151 */
#define	U4R_LDPR98				(*(VU4 *)(LCDC_BADR+0x0260))		/* LCDC Palette data register 152 */
#define	U4R_LDPR99				(*(VU4 *)(LCDC_BADR+0x0264))		/* LCDC Palette data register 153 */
#define	U4R_LDPR9A				(*(VU4 *)(LCDC_BADR+0x0268))		/* LCDC Palette data register 154 */
#define	U4R_LDPR9B				(*(VU4 *)(LCDC_BADR+0x026C))		/* LCDC Palette data register 155 */
#define	U4R_LDPR9C				(*(VU4 *)(LCDC_BADR+0x0270))		/* LCDC Palette data register 156 */
#define	U4R_LDPR9D				(*(VU4 *)(LCDC_BADR+0x0274))		/* LCDC Palette data register 157 */
#define	U4R_LDPR9E				(*(VU4 *)(LCDC_BADR+0x0278))		/* LCDC Palette data register 158 */
#define	U4R_LDPR9F				(*(VU4 *)(LCDC_BADR+0x027C))		/* LCDC Palette data register 159 */
#define	U4R_LDPRA0				(*(VU4 *)(LCDC_BADR+0x0280))		/* LCDC Palette data register 160 */
#define	U4R_LDPRA1				(*(VU4 *)(LCDC_BADR+0x0284))		/* LCDC Palette data register 161 */
#define	U4R_LDPRA2				(*(VU4 *)(LCDC_BADR+0x0288))		/* LCDC Palette data register 162 */
#define	U4R_LDPRA3				(*(VU4 *)(LCDC_BADR+0x028C))		/* LCDC Palette data register 163 */
#define	U4R_LDPRA4				(*(VU4 *)(LCDC_BADR+0x0290))		/* LCDC Palette data register 164 */
#define	U4R_LDPRA5				(*(VU4 *)(LCDC_BADR+0x0294))		/* LCDC Palette data register 165 */
#define	U4R_LDPRA6				(*(VU4 *)(LCDC_BADR+0x0298))		/* LCDC Palette data register 166 */
#define	U4R_LDPRA7				(*(VU4 *)(LCDC_BADR+0x029C))		/* LCDC Palette data register 167 */
#define	U4R_LDPRA8				(*(VU4 *)(LCDC_BADR+0x02A0))		/* LCDC Palette data register 168 */
#define	U4R_LDPRA9				(*(VU4 *)(LCDC_BADR+0x02A4))		/* LCDC Palette data register 169 */
#define	U4R_LDPRAA				(*(VU4 *)(LCDC_BADR+0x02A8))		/* LCDC Palette data register 170 */
#define	U4R_LDPRAB				(*(VU4 *)(LCDC_BADR+0x02AC))		/* LCDC Palette data register 171 */
#define	U4R_LDPRAC				(*(VU4 *)(LCDC_BADR+0x02B0))		/* LCDC Palette data register 172 */
#define	U4R_LDPRAD				(*(VU4 *)(LCDC_BADR+0x02B4))		/* LCDC Palette data register 173 */
#define	U4R_LDPRAE				(*(VU4 *)(LCDC_BADR+0x02B8))		/* LCDC Palette data register 174 */
#define	U4R_LDPRAF				(*(VU4 *)(LCDC_BADR+0x02BC))		/* LCDC Palette data register 175 */
#define	U4R_LDPRB0				(*(VU4 *)(LCDC_BADR+0x02C0))		/* LCDC Palette data register 176 */
#define	U4R_LDPRB1				(*(VU4 *)(LCDC_BADR+0x02C4))		/* LCDC Palette data register 177 */
#define	U4R_LDPRB2				(*(VU4 *)(LCDC_BADR+0x02C8))		/* LCDC Palette data register 178 */
#define	U4R_LDPRB3				(*(VU4 *)(LCDC_BADR+0x02CC))		/* LCDC Palette data register 179 */
#define	U4R_LDPRB4				(*(VU4 *)(LCDC_BADR+0x02D0))		/* LCDC Palette data register 180 */
#define	U4R_LDPRB5				(*(VU4 *)(LCDC_BADR+0x02D4))		/* LCDC Palette data register 181 */
#define	U4R_LDPRB6				(*(VU4 *)(LCDC_BADR+0x02D8))		/* LCDC Palette data register 182 */
#define	U4R_LDPRB7				(*(VU4 *)(LCDC_BADR+0x02DC))		/* LCDC Palette data register 183 */
#define	U4R_LDPRB8				(*(VU4 *)(LCDC_BADR+0x02E0))		/* LCDC Palette data register 184 */
#define	U4R_LDPRB9				(*(VU4 *)(LCDC_BADR+0x02E4))		/* LCDC Palette data register 185 */
#define	U4R_LDPRBA				(*(VU4 *)(LCDC_BADR+0x02E8))		/* LCDC Palette data register 186 */
#define	U4R_LDPRBB				(*(VU4 *)(LCDC_BADR+0x02EC))		/* LCDC Palette data register 187 */
#define	U4R_LDPRBC				(*(VU4 *)(LCDC_BADR+0x02F0))		/* LCDC Palette data register 188 */
#define	U4R_LDPRBD				(*(VU4 *)(LCDC_BADR+0x02F4))		/* LCDC Palette data register 189 */
#define	U4R_LDPRBE				(*(VU4 *)(LCDC_BADR+0x02F8))		/* LCDC Palette data register 190 */
#define	U4R_LDPRBF				(*(VU4 *)(LCDC_BADR+0x02FC))		/* LCDC Palette data register 191 */
#define	U4R_LDPRC0				(*(VU4 *)(LCDC_BADR+0x0300))		/* LCDC Palette data register 192 */
#define	U4R_LDPRC1				(*(VU4 *)(LCDC_BADR+0x0304))		/* LCDC Palette data register 193 */
#define	U4R_LDPRC2				(*(VU4 *)(LCDC_BADR+0x0308))		/* LCDC Palette data register 194 */
#define	U4R_LDPRC3				(*(VU4 *)(LCDC_BADR+0x030C))		/* LCDC Palette data register 195 */
#define	U4R_LDPRC4				(*(VU4 *)(LCDC_BADR+0x0310))		/* LCDC Palette data register 196 */
#define	U4R_LDPRC5				(*(VU4 *)(LCDC_BADR+0x0314))		/* LCDC Palette data register 197 */
#define	U4R_LDPRC6				(*(VU4 *)(LCDC_BADR+0x0318))		/* LCDC Palette data register 198 */
#define	U4R_LDPRC7				(*(VU4 *)(LCDC_BADR+0x031C))		/* LCDC Palette data register 199 */
#define	U4R_LDPRC8				(*(VU4 *)(LCDC_BADR+0x0320))		/* LCDC Palette data register 200 */
#define	U4R_LDPRC9				(*(VU4 *)(LCDC_BADR+0x0324))		/* LCDC Palette data register 201 */
#define	U4R_LDPRCA				(*(VU4 *)(LCDC_BADR+0x0328))		/* LCDC Palette data register 202 */
#define	U4R_LDPRCB				(*(VU4 *)(LCDC_BADR+0x032C))		/* LCDC Palette data register 203 */
#define	U4R_LDPRCC				(*(VU4 *)(LCDC_BADR+0x0330))		/* LCDC Palette data register 204 */
#define	U4R_LDPRCD				(*(VU4 *)(LCDC_BADR+0x0334))		/* LCDC Palette data register 205 */
#define	U4R_LDPRCE				(*(VU4 *)(LCDC_BADR+0x0338))		/* LCDC Palette data register 206 */
#define	U4R_LDPRCF				(*(VU4 *)(LCDC_BADR+0x033C))		/* LCDC Palette data register 207 */
#define	U4R_LDPRD0				(*(VU4 *)(LCDC_BADR+0x0340))		/* LCDC Palette data register 208 */
#define	U4R_LDPRD1				(*(VU4 *)(LCDC_BADR+0x0344))		/* LCDC Palette data register 209 */
#define	U4R_LDPRD2				(*(VU4 *)(LCDC_BADR+0x0348))		/* LCDC Palette data register 210 */
#define	U4R_LDPRD3				(*(VU4 *)(LCDC_BADR+0x034C))		/* LCDC Palette data register 211 */
#define	U4R_LDPRD4				(*(VU4 *)(LCDC_BADR+0x0350))		/* LCDC Palette data register 212 */
#define	U4R_LDPRD5				(*(VU4 *)(LCDC_BADR+0x0354))		/* LCDC Palette data register 213 */
#define	U4R_LDPRD6				(*(VU4 *)(LCDC_BADR+0x0358))		/* LCDC Palette data register 214 */
#define	U4R_LDPRD7				(*(VU4 *)(LCDC_BADR+0x035C))		/* LCDC Palette data register 215 */
#define	U4R_LDPRD8				(*(VU4 *)(LCDC_BADR+0x0360))		/* LCDC Palette data register 216 */
#define	U4R_LDPRD9				(*(VU4 *)(LCDC_BADR+0x0364))		/* LCDC Palette data register 217 */
#define	U4R_LDPRDA				(*(VU4 *)(LCDC_BADR+0x0368))		/* LCDC Palette data register 218 */
#define	U4R_LDPRDB				(*(VU4 *)(LCDC_BADR+0x036C))		/* LCDC Palette data register 219 */
#define	U4R_LDPRDC				(*(VU4 *)(LCDC_BADR+0x0370))		/* LCDC Palette data register 220 */
#define	U4R_LDPRDD				(*(VU4 *)(LCDC_BADR+0x0374))		/* LCDC Palette data register 221 */
#define	U4R_LDPRDE				(*(VU4 *)(LCDC_BADR+0x0378))		/* LCDC Palette data register 222 */
#define	U4R_LDPRDF				(*(VU4 *)(LCDC_BADR+0x037C))		/* LCDC Palette data register 223 */
#define	U4R_LDPRE0				(*(VU4 *)(LCDC_BADR+0x0380))		/* LCDC Palette data register 224 */
#define	U4R_LDPRE1				(*(VU4 *)(LCDC_BADR+0x0384))		/* LCDC Palette data register 225 */
#define	U4R_LDPRE2				(*(VU4 *)(LCDC_BADR+0x0388))		/* LCDC Palette data register 226 */
#define	U4R_LDPRE3				(*(VU4 *)(LCDC_BADR+0x038C))		/* LCDC Palette data register 227 */
#define	U4R_LDPRE4				(*(VU4 *)(LCDC_BADR+0x0390))		/* LCDC Palette data register 228 */
#define	U4R_LDPRE5				(*(VU4 *)(LCDC_BADR+0x0394))		/* LCDC Palette data register 229 */
#define	U4R_LDPRE6				(*(VU4 *)(LCDC_BADR+0x0398))		/* LCDC Palette data register 230 */
#define	U4R_LDPRE7				(*(VU4 *)(LCDC_BADR+0x039C))		/* LCDC Palette data register 231 */
#define	U4R_LDPRE8				(*(VU4 *)(LCDC_BADR+0x03A0))		/* LCDC Palette data register 232 */
#define	U4R_LDPRE9				(*(VU4 *)(LCDC_BADR+0x03A4))		/* LCDC Palette data register 233 */
#define	U4R_LDPREA				(*(VU4 *)(LCDC_BADR+0x03A8))		/* LCDC Palette data register 234 */
#define	U4R_LDPREB				(*(VU4 *)(LCDC_BADR+0x03AC))		/* LCDC Palette data register 235 */
#define	U4R_LDPREC				(*(VU4 *)(LCDC_BADR+0x03B0))		/* LCDC Palette data register 236 */
#define	U4R_LDPRED				(*(VU4 *)(LCDC_BADR+0x03B4))		/* LCDC Palette data register 237 */
#define	U4R_LDPREE				(*(VU4 *)(LCDC_BADR+0x03B8))		/* LCDC Palette data register 238 */
#define	U4R_LDPREF				(*(VU4 *)(LCDC_BADR+0x03BC))		/* LCDC Palette data register 239 */
#define	U4R_LDPRF0				(*(VU4 *)(LCDC_BADR+0x03C0))		/* LCDC Palette data register 240 */
#define	U4R_LDPRF1				(*(VU4 *)(LCDC_BADR+0x03C4))		/* LCDC Palette data register 241 */
#define	U4R_LDPRF2				(*(VU4 *)(LCDC_BADR+0x03C8))		/* LCDC Palette data register 242 */
#define	U4R_LDPRF3				(*(VU4 *)(LCDC_BADR+0x03CC))		/* LCDC Palette data register 243 */
#define	U4R_LDPRF4				(*(VU4 *)(LCDC_BADR+0x03D0))		/* LCDC Palette data register 244 */
#define	U4R_LDPRF5				(*(VU4 *)(LCDC_BADR+0x03D4))		/* LCDC Palette data register 245 */
#define	U4R_LDPRF6				(*(VU4 *)(LCDC_BADR+0x03D8))		/* LCDC Palette data register 246 */
#define	U4R_LDPRF7				(*(VU4 *)(LCDC_BADR+0x03DC))		/* LCDC Palette data register 247 */
#define	U4R_LDPRF8				(*(VU4 *)(LCDC_BADR+0x03E0))		/* LCDC Palette data register 248 */
#define	U4R_LDPRF9				(*(VU4 *)(LCDC_BADR+0x03E4))		/* LCDC Palette data register 249 */
#define	U4R_LDPRFA				(*(VU4 *)(LCDC_BADR+0x03E8))		/* LCDC Palette data register 250 */
#define	U4R_LDPRFB				(*(VU4 *)(LCDC_BADR+0x03EC))		/* LCDC Palette data register 251 */
#define	U4R_LDPRFC				(*(VU4 *)(LCDC_BADR+0x03F0))		/* LCDC Palette data register 252 */
#define	U4R_LDPRFD				(*(VU4 *)(LCDC_BADR+0x03F4))		/* LCDC Palette data register 253 */
#define	U4R_LDPRFE				(*(VU4 *)(LCDC_BADR+0x03F8))		/* LCDC Palette data register 254 */
#define	U4R_LDPRFF				(*(VU4 *)(LCDC_BADR+0x03FC))		/* LCDC Palette data register 255 */


/* for AG5 DEBUG( Å¶DELETE Later ) */
/* Ñ§ SUB_INDEX_LCDC1 */
#define	U4R_MLDDCKPAT1R1		(*(VU4 *)(LCDC1_BADR+0x0400))		/* Main LCD Dot clock buffer setting register1 */
#define	U4R_MLDDCKPAT2R1		(*(VU4 *)(LCDC1_BADR+0x0404))		/* Main LCD Dot clock buffer setting register2 */
#define	U4R_SLDDCKPAT1R1		(*(VU4 *)(LCDC1_BADR+0x0408))		/* LCD Dot clock buffer setting register1 */
#define	U4R_SLDDCKPAT2R1		(*(VU4 *)(LCDC1_BADR+0x040C))		/* LCD Dot clock buffer setting register2 */
#define	U4R_LDDCKR1				(*(VU4 *)(LCDC1_BADR+0x0410))		/* LCDC dot clock register */
#define	U4R_LDDCKSTPR1			(*(VU4 *)(LCDC1_BADR+0x0414))		/* dot clock stop register */
#define	U4R_MLDMT1R1			(*(VU4 *)(LCDC1_BADR+0x0418))		/* Main LCD module type register1 */
#define	U4R_MLDMT2R1			(*(VU4 *)(LCDC1_BADR+0x041C))		/* Main LCD module type register2 */
#define	U4R_MLDMT3R1			(*(VU4 *)(LCDC1_BADR+0x0420))		/* Main LCD module type register3 */
#define	U4R_MLDDFR1				(*(VU4 *)(LCDC1_BADR+0x0424))		/* Main LCD Data format register */
#define	U4R_MLDSM1R1			(*(VU4 *)(LCDC1_BADR+0x0428))		/* Main LCD Scanning mode register1 */
#define	U4R_MLDSM2R1			(*(VU4 *)(LCDC1_BADR+0x042C))		/* Main LCD Scanning mode register2 */
#define	U4R_MLDSA1R1			(*(VU4 *)(LCDC1_BADR+0x0430))		/* Main LCD The displayed data start address register1 */
#define	U4R_MLDSA2R1			(*(VU4 *)(LCDC1_BADR+0x0434))		/* Main LCD The displayed data start address register2 */
#define	U4R_MLDMLSR1			(*(VU4 *)(LCDC1_BADR+0x0438))		/* Main LCD Display data storage memory line size register */
#define	U4R_MLDWBFR1			(*(VU4 *)(LCDC1_BADR+0x043C))		/* Main LCD Writing return data format register */
#define	U4R_MLDWBCNTR1			(*(VU4 *)(LCDC1_BADR+0x0440))		/* Main LCD Writing return control register */
#define	U4R_MLDWBAR1			(*(VU4 *)(LCDC1_BADR+0x0444))		/* Main LCD Start address register of writing return destination */
#define	U4R_MLDHCNR1			(*(VU4 *)(LCDC1_BADR+0x0448))		/* Main LCD The horizontal character number register */
#define	U4R_MLDHSYNR1			(*(VU4 *)(LCDC1_BADR+0x044C))		/* Main LCD Horizontal synchronizing signal register */
#define	U4R_MLDVLNR1			(*(VU4 *)(LCDC1_BADR+0x0450))		/* Main LCD Vertical line number register */
#define	U4R_MLDVSYNR1			(*(VU4 *)(LCDC1_BADR+0x0454))		/* Main LCD Vertical synchronizing signal register */
#define	U4R_MLDHPDR1			(*(VU4 *)(LCDC1_BADR+0x0458))		/* Main LCD The horizontal, partial screen register */
#define	U4R_MLDVPDR1			(*(VU4 *)(LCDC1_BADR+0x045C))		/* Main LCD Vertical, partial screen register */
#define	U4R_MLDPMR1				(*(VU4 *)(LCDC1_BADR+0x0460))		/* Main LCD Power management register */
#define	U4R_LDPALCR1			(*(VU4 *)(LCDC1_BADR+0x0464))		/* LCDC Palette control register */
#define	U4R_LDINTR1				(*(VU4 *)(LCDC1_BADR+0x0468))		/* LCDC Interrupt register */
#define	U4R_LDSR1				(*(VU4 *)(LCDC1_BADR+0x046C))		/* LCDC Status register */
#define	U4R_LDCNT1R1			(*(VU4 *)(LCDC1_BADR+0x0470))		/* LCDC Control register1 */
#define	U4R_LDCNT2R1			(*(VU4 *)(LCDC1_BADR+0x0474))		/* LCDC Control register2 */
#define	U4R_LDRCNTR1			(*(VU4 *)(LCDC1_BADR+0x0478))		/* LCDC Register side control register */
#define	U4R_LDDDSR1				(*(VU4 *)(LCDC1_BADR+0x047C))		/* LCDC Input image data swap register */
#define	U4R_LDRCR1				(*(VU4 *)(LCDC1_BADR+0x0484))		/* LCDC Compulsion on register side specification register */
#define	U4R_LDDBSLMR1			(*(VU4 *)(LCDC1_BADR+0x0490))		/* Doubler mode register */
#define	U4R_LDSLHPNR1			(*(VU4 *)(LCDC1_BADR+0x0494))		/* Register of number of horizontal slides */
#define	U4R_LDSLVLNR1			(*(VU4 *)(LCDC1_BADR+0x0498))		/* Register of number of vertical slides */
#define	U4R_LDSLRGBR1			(*(VU4 *)(LCDC1_BADR+0x049C))		/* Slide data register */
#define	U4R_MLDHAJR1			(*(VU4 *)(LCDC1_BADR+0x04A0))		/* Main LCD Horizontal synchronizing signal adjustment register */
#define	U4R_MLDIVSNR1			(*(VU4 *)(LCDC1_BADR+0x04A4))		/* Main LCD Vertical synchronizing signal adjustment register */
#define	U4R_SLDMT1R1			(*(VU4 *)(LCDC1_BADR+0x0600))		/* LCD module type register1 */
#define	U4R_SLDMT2R1			(*(VU4 *)(LCDC1_BADR+0x0604))		/* LCD module type register2 */
#define	U4R_SLDMT3R1			(*(VU4 *)(LCDC1_BADR+0x0608))		/* LCD module type register3 */
#define	U4R_SLDDFR1				(*(VU4 *)(LCDC1_BADR+0x060C))		/* LCD Data format register */
#define	U4R_SLDSM1R1			(*(VU4 *)(LCDC1_BADR+0x0610))		/* LCD Scanning mode register1 */
#define	U4R_SLDSM2R1			(*(VU4 *)(LCDC1_BADR+0x0614))		/* LCD Scanning mode register2 */
#define	U4R_SLDSA1R1			(*(VU4 *)(LCDC1_BADR+0x0618))		/* LCD The displayed data start address register1 */
#define	U4R_SLDSA2R1			(*(VU4 *)(LCDC1_BADR+0x061C))		/* LCD The displayed data start address register2 */
#define	U4R_SLDMLSR1			(*(VU4 *)(LCDC1_BADR+0x0620))		/* LCD Display data storage memory line size register */
#define	U4R_SLDHCNR1			(*(VU4 *)(LCDC1_BADR+0x0624))		/* LCD The horizontal character number register */
#define	U4R_SLDHSYNR1			(*(VU4 *)(LCDC1_BADR+0x0628))		/* LCD The horizontal, synchronous timing register */
#define	U4R_SLDVLNR1			(*(VU4 *)(LCDC1_BADR+0x062C))		/* LCD Vertical line number register */
#define	U4R_SLDVSYNR1			(*(VU4 *)(LCDC1_BADR+0x0630))		/* LCD Vertical synchronizing signal register */
#define	U4R_SLDHPDR1			(*(VU4 *)(LCDC1_BADR+0x0634))		/* LCD The horizontal, partial screen register */
#define	U4R_SLDVPDR1			(*(VU4 *)(LCDC1_BADR+0x0638))		/* LCD Vertical, partial screen register */
#define	U4R_SLDPMR1				(*(VU4 *)(LCDC1_BADR+0x063C))		/* LCD Power management register */
#define	U4R_LDDWD0R1			(*(VU4 *)(LCDC1_BADR+0x0800))		/* LCDC driver write data register 0 */
#define	U4R_LDDWD1R1			(*(VU4 *)(LCDC1_BADR+0x0804))		/* LCDC driver write data register 1 */
#define	U4R_LDDWD2R1			(*(VU4 *)(LCDC1_BADR+0x0808))		/* LCDC driver write data register 2 */
#define	U4R_LDDWD3R1			(*(VU4 *)(LCDC1_BADR+0x080C))		/* LCDC driver write data register 3 */
#define	U4R_LDDWD4R1			(*(VU4 *)(LCDC1_BADR+0x0810))		/* LCDC driver write data register 4 */
#define	U4R_LDDWD5R1			(*(VU4 *)(LCDC1_BADR+0x0814))		/* LCDC driver write data register 5 */
#define	U4R_LDDWD6R1			(*(VU4 *)(LCDC1_BADR+0x0818))		/* LCDC driver write data register 6 */
#define	U4R_LDDWD7R1			(*(VU4 *)(LCDC1_BADR+0x081C))		/* LCDC driver write data register 7 */
#define	U4R_LDDWD8R1			(*(VU4 *)(LCDC1_BADR+0x0820))		/* LCDC driver write data register 8 */
#define	U4R_LDDWD9R1			(*(VU4 *)(LCDC1_BADR+0x0824))		/* LCDC driver write data register 9 */
#define	U4R_LDDWDAR1			(*(VU4 *)(LCDC1_BADR+0x0828))		/* LCDC driver write data register A */
#define	U4R_LDDWDBR1			(*(VU4 *)(LCDC1_BADR+0x082C))		/* LCDC driver write data register B */
#define	U4R_LDDWDCR1			(*(VU4 *)(LCDC1_BADR+0x0830))		/* LCDC driver write data register C */
#define	U4R_LDDWDDR1			(*(VU4 *)(LCDC1_BADR+0x0834))		/* LCDC driver write data register D */
#define	U4R_LDDWDER1			(*(VU4 *)(LCDC1_BADR+0x0838))		/* LCDC driver write data register E */
#define	U4R_LDDWDFR1			(*(VU4 *)(LCDC1_BADR+0x083C))		/* LCDC driver write data register F */
#define	U4R_LDDRDR1				(*(VU4 *)(LCDC1_BADR+0x0840))		
#define	U4R_LDDWAR1				(*(VU4 *)(LCDC1_BADR+0x0900))		
#define	U4R_LDDRAR1				(*(VU4 *)(LCDC1_BADR+0x0904))		
#define	U4R_LDCMRKRGBR1			(*(VU4 *)(LCDC1_BADR+0x0A00))		/* LCDC Color management matrix R Coefficient RGB register  */
#define	U4R_LDCMRKCMYR1			(*(VU4 *)(LCDC1_BADR+0x0A04))		/* LCDC Color management matrix R Coefficient CMY register  */
#define	U4R_LDCMRK1R1			(*(VU4 *)(LCDC1_BADR+0x0A08))		/* LCDC Color management matrix R Register between coefficient hue 1 */
#define	U4R_LDCMRK2R1			(*(VU4 *)(LCDC1_BADR+0x0A0C))		/* LCDC Color management matrix R Register between coefficient hue 2 */
#define	U4R_LDCMGKRGBR1			(*(VU4 *)(LCDC1_BADR+0x0A10))		/* LCDC Color management matrix G Coefficient RGB register  */
#define	U4R_LDCMGKCMYR1			(*(VU4 *)(LCDC1_BADR+0x0A14))		/* LCDC Color management matrix G Coefficient CMY register  */
#define	U4R_LDCMGK1R1			(*(VU4 *)(LCDC1_BADR+0x0A18))		/* LCDC Color management matrix G Register between coefficient hue 1 */
#define	U4R_LDCMGK2R1			(*(VU4 *)(LCDC1_BADR+0x0A1C))		/* LCDC Color management matrix G Register between coefficient hue 2 */
#define	U4R_LDCMBKRGBR1			(*(VU4 *)(LCDC1_BADR+0x0A20))		/* LCDC Color management matrix B Coefficient RGB register  */
#define	U4R_LDCMBKCMYR1			(*(VU4 *)(LCDC1_BADR+0x0A24))		/* LCDC Color management matrix B Coefficient CMY register  */
#define	U4R_LDCMBK1R1			(*(VU4 *)(LCDC1_BADR+0x0A28))		/* LCDC Color management matrix B Register between coefficient hue 1 */
#define	U4R_LDCMBK2R1			(*(VU4 *)(LCDC1_BADR+0x0A2C))		/* LCDC Color management matrix B Register between coefficient hue 2 */
#define	U4R_LDCMHKPR1			(*(VU4 *)(LCDC1_BADR+0x0A30))		
#define	U4R_LDCMHKQR1			(*(VU4 *)(LCDC1_BADR+0x0A34))		
#define	U4R_LDCMSELR1			(*(VU4 *)(LCDC1_BADR+0x0A38))		/* LCDC Color management multiplication coefficient selection register  */
#define	U4R_LDCMTVR1			(*(VU4 *)(LCDC1_BADR+0x0A3C))		
#define	U4R_LDCMTVSELR1			(*(VU4 *)(LCDC1_BADR+0x0A40))		
#define	U4R_LDCMDTHR1			(*(VU4 *)(LCDC1_BADR+0x0A44))		
#define	U4R_LDCMCNTR1			(*(VU4 *)(LCDC1_BADR+0x0A48))		
#define	U4R_LDPALECR1			(*(VU4 *)(LCDC1_BADR+0x0A50))		
#define	U4R_LDPALADD1			(*(VU4 *)(LCDC1_BADR+0x0A54))		
#define	U4R_LDBCR1				(*(VU4 *)(LCDC1_BADR+0x0B00))		
#define	U4R_LDB1BSIFR1			(*(VU4 *)(LCDC1_BADR+0x0B20))		/* CH1Source image format register  */
#define	U4R_LDB1BSSZR1			(*(VU4 *)(LCDC1_BADR+0x0B24))		/* CH1 Source size specification egister  */
#define	U4R_LDB1BLOCR1			(*(VU4 *)(LCDC1_BADR+0x0B28))		/* CH1 Blend location setting register  */
#define	U4R_LDB1BSMWR1			(*(VU4 *)(LCDC1_BADR+0x0B2C))		/* CH1 Source memory width specificationregister  */
#define	U4R_LDB1BSARY1			(*(VU4 *)(LCDC1_BADR+0x0B30))		/* CH1 Source address Y register  */
#define	U4R_LDB1BSARC1			(*(VU4 *)(LCDC1_BADR+0x0B34))		/* CH1 Source address C register  */
#define	U4R_LDB1BSARA1			(*(VU4 *)(LCDC1_BADR+0x0B38))		/* CH1 Source address Éøregister  */
#define	U4R_LDB1BPPCR1			(*(VU4 *)(LCDC1_BADR+0x0B3C))		/* CH1 color control register  */
#define	U4R_LDB1BBGCL1			(*(VU4 *)(LCDC1_BADR+0x0B10))		/* CH1 BGCOLOR register  */
#define	U4R_LDB2BSIFR1			(*(VU4 *)(LCDC1_BADR+0x0B40))		/* CH2 Source image format register  */
#define	U4R_LDB2BSSZR1			(*(VU4 *)(LCDC1_BADR+0x0B44))		/* CH2 Source size specification egister  */
#define	U4R_LDB2BLOCR1			(*(VU4 *)(LCDC1_BADR+0x0B48))		/* CH2 Blend location setting register  */
#define	U4R_LDB2BSMWR1			(*(VU4 *)(LCDC1_BADR+0x0B4C))		/* CH2 Source memory width specificationregister  */
#define	U4R_LDB2BSARY1			(*(VU4 *)(LCDC1_BADR+0x0B50))		/* CH2 Source address Y register  */
#define	U4R_LDB2BSARC1			(*(VU4 *)(LCDC1_BADR+0x0B54))		/* CH2 Source address C register  */
#define	U4R_LDB2BSARA1			(*(VU4 *)(LCDC1_BADR+0x0B58))		/* CH2 Source address Éøregister  */
#define	U4R_LDB2BPPCR1			(*(VU4 *)(LCDC1_BADR+0x0B5C))		/* CH2 color control register  */
#define	U4R_LDB2BBGCL1			(*(VU4 *)(LCDC1_BADR+0x0B14))		/* CH2 BGCOLOR register  */
#define	U4R_LDB3BSIFR1			(*(VU4 *)(LCDC1_BADR+0x0B60))		/* CH3 Source image format register  */
#define	U4R_LDB3BSSZR1			(*(VU4 *)(LCDC1_BADR+0x0B64))		/* CH3 Source size specification egister  */
#define	U4R_LDB3BLOCR1			(*(VU4 *)(LCDC1_BADR+0x0B68))		/* CH3 Blend location setting register  */
#define	U4R_LDB3BSMWR1			(*(VU4 *)(LCDC1_BADR+0x0B6C))		/* CH3 Source memory width specificationregister  */
#define	U4R_LDB3BSARY1			(*(VU4 *)(LCDC1_BADR+0x0B70))		/* CH3 Source address Y register  */
#define	U4R_LDB3BSARC1			(*(VU4 *)(LCDC1_BADR+0x0B74))		/* CH3 Source address C register  */
#define	U4R_LDB3BSARA1			(*(VU4 *)(LCDC1_BADR+0x0B78))		/* CH3 Source address Éøregister  */
#define	U4R_LDB3BPPCR1			(*(VU4 *)(LCDC1_BADR+0x0B7C))		/* CH3 color control register  */
#define	U4R_LDB3BBGCL1			(*(VU4 *)(LCDC1_BADR+0x0B18))		/* CH3 BGCOLOR register  */
#define	U4R_LDB4BSIFR1			(*(VU4 *)(LCDC1_BADR+0x0B80))		/* CH4 Source image format register  */
#define	U4R_LDB4BSSZR1			(*(VU4 *)(LCDC1_BADR+0x0B84))		/* CH4 Source size specification egister  */
#define	U4R_LDB4BLOCR1			(*(VU4 *)(LCDC1_BADR+0x0B88))		/* CH4 Blend location setting register  */
#define	U4R_LDB4BSMWR1			(*(VU4 *)(LCDC1_BADR+0x0B8C))		/* CH4 Source memory width specificationregister  */
#define	U4R_LDB4BSARY1			(*(VU4 *)(LCDC1_BADR+0x0B90))		/* CH4 Source address Y register  */
#define	U4R_LDB4BSARC1			(*(VU4 *)(LCDC1_BADR+0x0B94))		/* CH4 Source address C register  */
#define	U4R_LDB4BSARA1			(*(VU4 *)(LCDC1_BADR+0x0B98))		/* CH4 Source address Éøregister  */
#define	U4R_LDB4BPPCR1			(*(VU4 *)(LCDC1_BADR+0x0B9C))		/* CH4 color control register  */
#define	U4R_LDB4BBGCL1			(*(VU4 *)(LCDC1_BADR+0x0B1C))		/* CH4 BGCOLOR register  */

#endif  /* __H_CP1_M11_LCDC_ */

