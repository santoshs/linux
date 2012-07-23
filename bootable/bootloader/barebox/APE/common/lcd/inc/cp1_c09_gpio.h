/*
 * cp1_c09_gpio.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef __H_CP1_C9_GPIO_
#define __H_CP1_C9_GPIO_

/***************************************************************/
/* INCLUDE FILE                                                */
/***************************************************************/

#define	U1R_PORT281CR			(*(VU1 *)(GPIO_BADR+0x3119))		/* PORT281Control register	Initial value:0x11 */

/***************************************************************/
/* PUBLIC CONSTANT DEFINITON                                   */
/*-------------------------------------------------------------*/
/* IO PORT(GPIO)                                             */
/*-------------------------------------------------------------*/
/* GPIO_BADR:0xE6050000                                        */
/***************************************************************/
/* INDEX_C09_GPIO */

/* PORTsxxx_yyy Data Register */
#define	U4R_PORTL031_000DR		(*(VU4 *)(GPIO_BADR+0x4000))		/* PORTL031_000 Data Register ini:0x00000000 */
#define	U4R_PORTL063_032DR		(*(VU4 *)(GPIO_BADR+0x4004))		/* PORTL063_032 Data Register ini:0x00000000 */
#define	U4R_PORTL095_064DR		(*(VU4 *)(GPIO_BADR+0x4008))		/* PORTL095_064 Data Register ini:0x00000000 */
#define	U4R_PORTL127_096DR		(*(VU4 *)(GPIO_BADR+0x400C))		/* PORTD127_096 Data Register ini:0x00000000 */
#define	U4R_PORTD159_128DR		(*(VU4 *)(GPIO_BADR+0x5000))		/* PORTD159_128 Data Register ini:0x00000000 */
#define	U4R_PORTR223_192DR		(*(VU4 *)(GPIO_BADR+0x6000))		/* PORTR223_192 Data Register ini:0x00000000 */
#define	U4R_PORTR255_224DR		(*(VU4 *)(GPIO_BADR+0x6004))		/* PORTR255_224 Data Register ini:0x00000000 */
#define	U4R_PORTR287_256DR		(*(VU4 *)(GPIO_BADR+0x6008))		/* PORTR287_256 Data Register ini:0x00000000 */
#define	U4R_PORTR319_288DR		(*(VU4 *)(GPIO_BADR+0x600C))		/* PORTR319_288 Data Register ini:0x00000000 */
#define	U4R_PORTR351_320DR		(*(VU4 *)(GPIO_BADR+0x6010))		/* PORTU351_320 Data Register ini:0x00000000 */

/* PORTsxxx_yyy Data Set Register */
#define	U4R_PORTL031_000DSR		(*(VU4 *)(GPIO_BADR+0x4100))		/* PORTL031_000 Data Set Register ini:0x00000000 */
#define	U4R_PORTL063_032DSR		(*(VU4 *)(GPIO_BADR+0x4104))		/* PORTL063_032 Data Set Register ini:0x00000000 */
#define	U4R_PORTL095_064DSR		(*(VU4 *)(GPIO_BADR+0x4108))		/* PORTL095_064 Data Set Register ini:0x00000000 */
#define	U4R_PORTL127_096DSR		(*(VU4 *)(GPIO_BADR+0x410C))		/* PORTD127_096 Data Set Register ini:0x00000000 */
#define	U4R_PORTD159_128DSR		(*(VU4 *)(GPIO_BADR+0x5100))		/* PORTD159_128 Data Set Register ini:0x00000000 */
#define	U4R_PORTR223_192DSR		(*(VU4 *)(GPIO_BADR+0x6100))		/* PORTR223_192 Data Set Register ini:0x00000000 */
#define	U4R_PORTR255_224DSR		(*(VU4 *)(GPIO_BADR+0x6104))		/* PORTR255_224 Data Set Register ini:0x00000000 */
#define	U4R_PORTR287_256DSR		(*(VU4 *)(GPIO_BADR+0x6108))		/* PORTR287_256 Data Set Register ini:0x00000000 */
#define	U4R_PORTR319_288DSR		(*(VU4 *)(GPIO_BADR+0x610C))		/* PORTR319_288 Data Set Register ini:0x00000000 */
#define	U4R_PORTR351_320DSR		(*(VU4 *)(GPIO_BADR+0x6110))		/* PORTU351_320 Data Set Register ini:0x00000000 */

/* PORTsxxx_yyy Data Clear Register */
#define	U4R_PORTL031_000DCR		(*(VU4 *)(GPIO_BADR+0x4200))		/* PORTL031_000 Data Clear Register ini:0x00000000 */
#define	U4R_PORTL063_032DCR		(*(VU4 *)(GPIO_BADR+0x4204))		/* PORTL063_032 Data Clear Register ini:0x00000000 */
#define	U4R_PORTL095_064DCR		(*(VU4 *)(GPIO_BADR+0x4208))		/* PORTL095_064 Data Clear Register ini:0x00000000 */
#define	U4R_PORTL127_096DCR		(*(VU4 *)(GPIO_BADR+0x420C))		/* PORTD127_096 Data Clear Register ini:0x00000000 */
#define	U4R_PORTD159_128DCR		(*(VU4 *)(GPIO_BADR+0x5200))		/* PORTD159_128 Data Clear Register ini:0x00000000 */
#define	U4R_PORTR223_192DCR		(*(VU4 *)(GPIO_BADR+0x6200))		/* PORTR223_192 Data Clear Register ini:0x00000000 */
#define	U4R_PORTR255_224DCR		(*(VU4 *)(GPIO_BADR+0x6204))		/* PORTR255_224 Data Clear Register ini:0x00000000 */
#define	U4R_PORTR287_256DCR		(*(VU4 *)(GPIO_BADR+0x6208))		/* PORTR287_256 Data Clear Register ini:0x00000000 */
#define	U4R_PORTR319_288DCR		(*(VU4 *)(GPIO_BADR+0x620C))		/* PORTR319_288 Data Clear Register ini:0x00000000 */
#define	U4R_PORTR351_320DCR		(*(VU4 *)(GPIO_BADR+0x6210))		/* PORTU351_320 Data Clear Register ini:0x00000000 */

/* Module function select register */
#define U4R_MSEL3CR			(*(VU4 *)(GPIO_BADR+0x8020))		/* Module Function Register3 ini:0x00000000 */
#define U4R_MSEL4CR			(*(VU4 *)(GPIO_BADR+0x8024))		/* Module Function Register4 ini:0x00000000 */

/* Driver Control Register */
#define U2R_DRVR_BSC			(*(VU2 *)(GPIO_BADR+0x8110))		/* BSC Driver Control Register ini:0x0003 */
#define U2R_DRVR_VCCQ18			(*(VU2 *)(GPIO_BADR+0x8112))		/* VCCQ18 Driver Control Register ini:0x0003 */
#define U2R_DRVR_CKO			(*(VU2 *)(GPIO_BADR+0x8114))		/* CKODriver Control Register ini:0x0003 */
#define U2R_DRVR_KEY			(*(VU2 *)(GPIO_BADR+0x8116))		/* KEYDriver Control Register ini:0x0003 */
#define U2R_DRVR_MMCCK			(*(VU2 *)(GPIO_BADR+0x8118))		/* MMCCK Driver Control Register ini:0x0003 */
#define U2R_DRVR_LCQ			(*(VU2 *)(GPIO_BADR+0x811A))		/* LCQDriver Control Register ini:0x0003 */
#define U2R_DRVR_SDCLK1			(*(VU2 *)(GPIO_BADR+0x811C))		/* SDCLK1 Driver Control Register ini:0x0003 */
#define U2R_DRVR_SDCLK0			(*(VU2 *)(GPIO_BADR+0x811E))		/* SDCLK0 Driver Control Register ini:0x0003 */
#define U2R_DRVR_RSTO			(*(VU2 *)(GPIO_BADR+0x8180))		/* RSTO Driver Control Register ini:0x0003 */
#define U2R_DRVR_VCK1			(*(VU2 *)(GPIO_BADR+0x8182))		/* VCK1 Driver Control Register ini:0x0003 */
#define U2R_DRVR_VCK2			(*(VU2 *)(GPIO_BADR+0x8184))		/* VCK2 Driver Control Register ini:0x0003 */
#define U2R_DRVR_VCK3			(*(VU2 *)(GPIO_BADR+0x8186))		/* VCK3 Driver Control Register ini:0x0003 */
#define U2R_DRVR_VCK4			(*(VU2 *)(GPIO_BADR+0x8188))		/* VCK4 Driver Control Register ini:0x0003 */
#define U2R_DRVR_JTAG			(*(VU2 *)(GPIO_BADR+0x818A))		/* JTAG Driver Control Register ini:0x0003 */
#define U2R_DRVR_SD1			(*(VU2 *)(GPIO_BADR+0x818C))		/* SD1 Driver Control Register ini:0x0003 */
#define U2R_DRVR_SD0			(*(VU2 *)(GPIO_BADR+0x818E))		/* SD0 Driver Control Register ini:0x0003 */
#define U2R_DRVR_MMC			(*(VU2 *)(GPIO_BADR+0x8190))		/* MMC Driver Control Register ini:0x0003 */
#define U2R_DRVR_SIM1			(*(VU2 *)(GPIO_BADR+0x8192))		/* SIM1 Driver Control Register ini:0x0003 */
#define U2R_DRVR_SIM2			(*(VU2 *)(GPIO_BADR+0x8194))		/* SIM2 Driver Control Register ini:0x0003 */
#define U2R_DRVR_ULPI			(*(VU2 *)(GPIO_BADR+0x8196))		/* ULPI Driver Control Register ini:0x0003 */
#define U2R_DRVR_MHSI			(*(VU2 *)(GPIO_BADR+0x8198))		/* MHSI Driver Control Register ini:0x0003 */

#endif  /* __H_CP1_C9_GPIO_ */

