/*	sbsc.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "sbsc.h"
#include "cpg.h"
#include "sysc.h"
#include "common.h"


/**
 * SBSC_Init_ES10 - SBSC initialize process
 * @return None
 */
void SBSC_Init_ES10(void)
{
	ulong work;

	*CPG_PLLECR &= ~CPG_PLLECR_PLL3E;
	
	/* Wait PLL3 status off */
	while (1)
	{
		work = *CPG_PLLECR;
		work &= CPG_PLLECR_PLL3ST;
		if (work == 0)
		{
			break;
		}
	}

	*CPG_PLL3CR  = CPG_PLL3CR_520MHZ;
	*CPG_PLLECR |= CPG_PLLECR_PLL3E;
	
	/* Wait PLL3 status on */
	while (1)
	{
		work = *CPG_PLLECR;
		work &= CPG_PLLECR_PLL3ST;
		if (work == CPG_PLLECR_PLL3ST)
		{
			break;
		}
	}
	
	*CPG_FRQCRD = CPG_FRQCRD_32MHz;
	*CPG_FRQCRB |= CPG_FRQCRB_KICK;
	/* Wait FRQCRB KICK bit is 0 - EOS */
	while (1)
	{
		work = *CPG_FRQCRB;
		work &= CPG_FRQCRB_KICK;
		if (work == 0)
		{
			break;
		}
	}

	*SBSC_SDCR0A    = SBSC_SDCR0A_DATA;
	*SBSC_SDCR1A    = SBSC_SDCR1A_DATA;
	*SBSC_SDWCRC0A  = SBSC_SDWCRC0A_DATA;
	*SBSC_SDWCRC1A  = SBSC_SDWCRC1A_DATA;
	*SBSC_SDWCRC2A  = SBSC_SDWCRC2A_DATA;
	*SBSC_SDWCR00A  = SBSC_SDWCR00A_DATA;
	
	*SBSC_SDWCR01A  = SBSC_SDWCR01A_DATA;
	*SBSC_SDWCR10A  = SBSC_SDWCR10A_DATA;
	*SBSC_SDWCR11A  = SBSC_SDWCR11A_DATA;
	*SBSC_SDDRVCR0A = SBSC_SDDRVCR0A_DATA;
	*SBSC_SDWCR2A   = SBSC_SDWCR2A_DATA;
	*SBSC_SDPCRA   |= SBSC_SDPCRA_DATA;
	
	/* Wait CKELV bit is 1 */
	while (1)
	{
		work = *SBSC_SDPCRA;
		work &= SBSC_SDPCRA_DATA;
		if (work == SBSC_SDPCRA_DATA)
		{
			break;
		}
	}
	
	*SBSC_SDGENCNTA = SBSC_SDGENCNTA_200US;
	
	/* Wait until SDGENCNTA counter becomes 0 */
	while (1)
	{
		work = *SBSC_SDGENCNTA;
		if (work == 0)
		{
			break;
		}
	}
	
	*SBSC_SDMRACR0A   = SBSC_SDMRACR0A_RESET;
	*SBSC_SDMRA_00000 = SBSC_SDMRA_DONE;
	*SBSC_SDGENCNTA   = SBSC_SDGENCNTA_10US;
	
	/* Wait until SDGENCNTA counter becomes 0 */
	while (1)
	{
		work = *SBSC_SDGENCNTA;
		if (work == 0)
		{
			break;
		}
	}
	
	*SBSC_SDMRACR0A   = SBSC_SDMRACR0A_ZQ_CAL;
	*SBSC_SDMRA_04000 = SBSC_SDMRA_DONE;
	*SBSC_SDGENCNTA   = SBSC_SDGENCNTA_1US;
	
	/* Wait until SDGENCNTA counter becomes 0 */
	while (1)
	{
		work = *SBSC_SDGENCNTA;
		if (work == 0)
		{
			break;
		}
	}
	
	*SBSC_SDMRACR0A   = SBSC_SDMRACR0A_MR1_SET;
	*SBSC_SDMRA_00000 = SBSC_SDMRA_DONE;
	*SBSC_SDMRACR0A   = SBSC_SDMRACR0A_MR2_SET;
	*SBSC_SDMRA_00000 = SBSC_SDMRA_DONE;
	*SBSC_SDMRACR0A   = SBSC_SDMRACR0A_MR3_SET;
	*SBSC_SDMRA_00000 = SBSC_SDMRA_DONE;
	*SBSC_SDMRA_C0000 = SBSC_SDMRA_DONE;
	*SBSC_SDMRTMPCRA  = SBSC_SDMRTMPCRA_DATA;
	*SBSC_SDMRTMPMSKA = SBSC_SDMRTMPMSKA_DATA;
	*SBSC_RTCORA      = SBSC_RTCORA_DATA;
	*SBSC_RTCORHA     = SBSC_RTCORHA_DATA;
	*SBSC_RTCSRA      = SBSC_RTCSRA_DATA;
	*SBSC_SDCR0A     |= SBSC_SDCR0A_RFSH;
	*SBSC_ZQCCRA      = SBSC_ZQCCRA_DATA;
	
	*CPG_FRQCRD = CPG_FRQCRD_260MHz;
	*CPG_FRQCRB |= CPG_FRQCRB_KICK;
	/* Wait FRQCRB.KICK bit is 0 - EOS */
	while (1)
	{
		work = *CPG_FRQCRB;
		work &= CPG_FRQCRB_KICK;
		if (work == 0)
		{
			break;
		}
	}
		
}

/**
 * SBSC_Init_390Mhz - SBSC initialize process for 390Mhz
 * @return None
 */
void SBSC_Init_390Mhz(void)
{
	ulong work;
	*CPG_PLLECR &= ~CPG_PLLECR_PLL3E;
	
	/* Wait PLL3 status off */
	while (1)
	{
		work = *CPG_PLLECR;
		work &= CPG_PLLECR_PLL3ST;
		if (work == 0)
		{
			break;
		}
	}

	/* Set PLL3 = 1040 Mhz*/
	*CPG_PLL3CR  = CPG_PLL3CR_1040MHZ;
	*CPG_PLLECR |= CPG_PLLECR_PLL3E;
	/* Wait PLL3 status on */
	while (1)
	{
		work = *CPG_PLLECR;
		work &= CPG_PLLECR_PLL3ST;
		if (work == CPG_PLLECR_PLL3ST)
		{
			break;
		}
	}

	/* ZB clock 1/32 */
	*CPG_FRQCRD = CPG_FRQCRD_DIV32;
	
	*CPG_FRQCRB |= CPG_FRQCRB_KICK;
	/* Wait FRQCRB KICK bit is 0 - EOS */
	while (1)
	{
		work = *CPG_FRQCRB;
		work &= CPG_FRQCRB_KICK;
		if (work == 0)
		{
			break;
		}
	}

	*SBSC_SDPHYTCR1 = SBSC_SDPHYTCR1_DATA;
	*SBSC_SDCR0A    = SBSC_SDCR0A_DATA;
	*SBSC_SDCR1A    = SBSC_SDCR1A_DATA;
	*SBSC_SDWCRC0A  = SBSC_SDWCRC0A_DATA_390;

	
	*SBSC_SDWCRC1A  = SBSC_SDWCRC1A_DATA_390;
	*SBSC_SDWCRC2A  = SBSC_SDWCRC2A_DATA_390;
	*SBSC_SDWCR00A  = SBSC_SDWCR00A_DATA_390;
	
	*SBSC_SDWCR01A  = SBSC_SDWCR01A_DATA_390;
	*SBSC_SDWCR10A  = SBSC_SDWCR10A_DATA_390;
	*SBSC_SDWCR11A  = SBSC_SDWCR11A_DATA_390;
	
	*SBSC_SDPCRA   |= SBSC_SDPCRA_DATA;
	
	/* Wait CKELV bit is 1 */
	while (1)
	{
		work = *SBSC_SDPCRA;
		work &= SBSC_SDPCRA_DATA_CKELV;
		if (work == SBSC_SDPCRA_DATA_CKELV)
		{
			break;
		}
	}
	
	/* Skip some SBSC setting flow when SW reset would occur due to power mode 3 setting*/
	if(!SYSC_SWReset_Flag()){
		*SBSC_SDGENCNTA = SBSC_SDGENCNTA_200US;
		
		/* Wait until SDGENCNTA counter becomes 0 */
		while (1)
		{
			work = *SBSC_SDGENCNTA;
			if (work == 0)
			{
				break;
			}
		}
		
		*SBSC_SDMRACR0A   = SBSC_SDMRACR0A_RESET;
		*SBSC_SDMRA_00000 = SBSC_SDMRA_DONE;
		*SBSC_SDGENCNTA   = SBSC_SDGENCNTA_10US;
		
		/* Wait until SDGENCNTA counter becomes 0 */
		while (1)
		{
			work = *SBSC_SDGENCNTA;
			if (work == 0)
			{
				break;
			}
		}
		
		*SBSC_SDMRACR0A   = SBSC_SDMRACR0A_ZQ_CAL;
		*SBSC_SDMRA_04000 = SBSC_SDMRA_DONE;
		*SBSC_SDGENCNTA   = SBSC_SDGENCNTA_1US;
		
		/* Wait until SDGENCNTA counter becomes 0 */
		while (1)
		{
			work = *SBSC_SDGENCNTA;
			if (work == 0)
			{
				break;
			}
		}
		
		*SBSC_SDMRACR0A   = SBSC_SDMRACR0A_MR1_SET;
		*SBSC_SDMRA_00000 = SBSC_SDMRA_DONE;
		*SBSC_SDMRACR0A   = SBSC_SDMRACR0A_DATA1;
		*SBSC_SDMRA_00000 = SBSC_SDMRA_DONE;
		
		*SBSC_SDMRACR0A   = SBSC_SDMRACR0A_DATA_390;
		
		*SBSC_SDMRA_00000 = SBSC_SDMRA_DONE;
		
	}/* Skip some SBSC setting flow when SW reset would occur due to power mode 3 setting*/
	
	*SBSC_SDMRA_C0000 = SBSC_SDMRA_DONE;
		
	*SBSC_SDMRTMPCRA  = SBSC_SDMRTMPCRA_DATA;
	*SBSC_SDMRTMPMSKA = SBSC_SDMRTMPMSKA_DATA;
	*SBSC_RTCORA      = SBSC_RTCORA_DATA;
	*SBSC_RTCORHA     = SBSC_RTCORHA_DATA;
	*SBSC_RTCSRA      = SBSC_RTCSRA_DATA;
	*SBSC_SDCR0A     |= SBSC_SDCR0A_RFSH;

	*SBSC_ZQCCRA      = SBSC_ZQCCRA_DATA;
	*SBSC_SDPDCR0A	  = SBSC_SDPDCR0A_PL3DLLCNT_OFF;	
	
	*SBSC_SDPTCR0A &= ~SBSC_SDPTCR_SCR_FP;	/* Disable Secure Protect for Debug */
	*SBSC_SDPTCR1A &= ~SBSC_SDPTCR_SCR_FP;	/* Disable Secure Protect for Debug */
	*SBSC_SDPTCR2A &= ~SBSC_SDPTCR_SCR_FP;	/* Disable Secure Protect for Debug */
	*SBSC_SDPTCR3A &= ~SBSC_SDPTCR_SCR_FP;	/* Disable Secure Protect for Debug */
	*SBSC_SDPTCR4A &= ~SBSC_SDPTCR_SCR_FP;	/* Disable Secure Protect for Debug */
	*SBSC_SDPTCR5A &= ~SBSC_SDPTCR_SCR_FP;	/* Disable Secure Protect for Debug */
	*SBSC_SDPTCR6A &= ~SBSC_SDPTCR_SCR_FP;	/* Disable Secure Protect for Debug */
	*SBSC_SDPTCR7A &= ~SBSC_SDPTCR_SCR_FP;	/* Disable Secure Protect for Debug */
	
	*CPG_PLLECR &= ~CPG_PLLECR_PLL3E;
	
	/* Wait PLL3 status off */
	while (1)
	{
		work = *CPG_PLLECR;
		work &= CPG_PLLECR_PLL3ST;
		if (work == 0)
		{
			break;
		}
	}
	
	/* Set PLL3 = 780 Mhz*/
	*CPG_PLL3CR  = CPG_PLL3CR_780MHZ;
	
	*CPG_PLLECR |= CPG_PLLECR_PLL3E;
	/* Wait PLL3 status on */
	while (1)
	{
		work = *CPG_PLLECR;
		work &= CPG_PLLECR_PLL3ST;
		if (work == CPG_PLLECR_PLL3ST)
		{
			break;
		}
	}
	
	/* ZB3 = 1/2 * PLL3 = 390 */
	*CPG_FRQCRD = CPG_FRQCRD_DIV2;
	*CPG_FRQCRB |= CPG_FRQCRB_KICK;
	/* Wait FRQCRB.KICK bit is 0 - EOS */
	while (1)
	{
		work = *CPG_FRQCRB;
		work &= CPG_FRQCRB_KICK;
		if (work == 0)
		{
			break;
		}
	}
	/* Set Power-Down Control */
	*SBSC_SDCR0A     |= SBSC_SDCR0A_PDOWN3;
	*SBSC_SDCR1A     |= SBSC_SDCR1A_PDOWN3;
	
	/* Set flag for skip some SBSC setting flow when SW reset would occur*/
	SYSC_Set_SWReset_Flag();
}

/**
 * SBSC_Init_520Mhz - SBSC initialize process for 520Mhz
 * @return None
 */
void SBSC_Init_520Mhz(void)
{
	ulong work;
	*CPG_PLLECR &= ~CPG_PLLECR_PLL3E;
	
	/* Wait PLL3 status off */
	while (1)
	{
		work = *CPG_PLLECR;
		work &= CPG_PLLECR_PLL3ST;
		if (work == 0)
		{
			break;
		}
	}

	/* Set PLL3 = 1040 Mhz*/
	*CPG_PLL3CR  = CPG_PLL3CR_1040MHZ;
	*CPG_PLLECR |= CPG_PLLECR_PLL3E;
	/* Wait PLL3 status on */
	while (1)
	{
		work = *CPG_PLLECR;
		work &= CPG_PLLECR_PLL3ST;
		if (work == CPG_PLLECR_PLL3ST)
		{
			break;
		}
	}

	/* ZB clock 1/32 */
	*CPG_FRQCRD = CPG_FRQCRD_DIV32;
	
	*CPG_FRQCRB |= CPG_FRQCRB_KICK;
	/* Wait FRQCRB KICK bit is 0 - EOS */
	while (1)
	{
		work = *CPG_FRQCRB;
		work &= CPG_FRQCRB_KICK;
		if (work == 0)
		{
			break;
		}
	}

	*SBSC_SDPHYTCR1 = SBSC_SDPHYTCR1_DATA;
	*SBSC_SDCR0A    = SBSC_SDCR0A_DATA;
	*SBSC_SDCR1A    = SBSC_SDCR1A_DATA;
	*SBSC_SDWCRC0A  = SBSC_SDWCRC0A_DATA_520;
	
	*SBSC_SDWCRC1A  = SBSC_SDWCRC1A_DATA_520;
	*SBSC_SDWCRC2A  = SBSC_SDWCRC2A_DATA_520;
	*SBSC_SDWCR00A  = SBSC_SDWCR00A_DATA_520;
	
	*SBSC_SDWCR01A  = SBSC_SDWCR01A_DATA_520;
	*SBSC_SDWCR10A  = SBSC_SDWCR10A_DATA_520;
	*SBSC_SDWCR11A  = SBSC_SDWCR11A_DATA_520;
	
	*SBSC_SDPCRA   |= SBSC_SDPCRA_DATA;
	
	/* Wait CKELV bit is 1 */
	while (1)
	{
		work = *SBSC_SDPCRA;
		work &= SBSC_SDPCRA_DATA_CKELV;
		if (work == SBSC_SDPCRA_DATA_CKELV)
		{
			break;
		}
	}
	
	/* Skip some SBSC setting flow when SW reset would occur due to power mode 3 setting*/
	if(!SYSC_SWReset_Flag()){
		*SBSC_SDGENCNTA = SBSC_SDGENCNTA_200US;
		
		/* Wait until SDGENCNTA counter becomes 0 */
		while (1)
		{
			work = *SBSC_SDGENCNTA;
			if (work == 0)
			{
				break;
			}
		}
		
		*SBSC_SDMRACR0A   = SBSC_SDMRACR0A_RESET;
		*SBSC_SDMRA_00000 = SBSC_SDMRA_DONE;
		*SBSC_SDGENCNTA   = SBSC_SDGENCNTA_10US;
		
		/* Wait until SDGENCNTA counter becomes 0 */
		while (1)
		{
			work = *SBSC_SDGENCNTA;
			if (work == 0)
			{
				break;
			}
		}
		
		*SBSC_SDMRACR0A   = SBSC_SDMRACR0A_ZQ_CAL;
		*SBSC_SDMRA_04000 = SBSC_SDMRA_DONE;
		*SBSC_SDGENCNTA   = SBSC_SDGENCNTA_1US;
		
		/* Wait until SDGENCNTA counter becomes 0 */
		while (1)
		{
			work = *SBSC_SDGENCNTA;
			if (work == 0)
			{
				break;
			}
		}
		
		*SBSC_SDMRACR0A   = SBSC_SDMRACR0A_MR1_SET;
		*SBSC_SDMRA_00000 = SBSC_SDMRA_DONE;
		*SBSC_SDMRACR0A   = SBSC_SDMRACR0A_DATA1;
		*SBSC_SDMRA_00000 = SBSC_SDMRA_DONE;
		
		*SBSC_SDMRACR0A   = SBSC_SDMRACR0A_DATA_520;
		
		*SBSC_SDMRA_00000 = SBSC_SDMRA_DONE;
		
	}/* Skip some SBSC setting flow when SW reset would occur due to power mode 3 setting*/
		
	*SBSC_SDMRTMPCRA  = SBSC_SDMRTMPCRA_DATA;
	*SBSC_SDMRTMPMSKA = SBSC_SDMRTMPMSKA_DATA;
	*SBSC_RTCORA      = SBSC_RTCORA_DATA;
	*SBSC_RTCORHA     = SBSC_RTCORHA_DATA;
	*SBSC_RTCSRA      = SBSC_RTCSRA_DATA;
	*SBSC_SDCR0A     |= SBSC_SDCR0A_RFSH;

	*SBSC_ZQCCRA      = SBSC_ZQCCRA_DATA;
	*SBSC_SDPDCR0A	  = SBSC_SDPDCR0A_PL3DLLCNT_OFF;	
	
	*SBSC_SDPTCR0A &= ~SBSC_SDPTCR_SCR_FP;	/* Disable Secure Protect for Debug */
	*SBSC_SDPTCR1A &= ~SBSC_SDPTCR_SCR_FP;	/* Disable Secure Protect for Debug */
	*SBSC_SDPTCR2A &= ~SBSC_SDPTCR_SCR_FP;	/* Disable Secure Protect for Debug */
	*SBSC_SDPTCR3A &= ~SBSC_SDPTCR_SCR_FP;	/* Disable Secure Protect for Debug */
	*SBSC_SDPTCR4A &= ~SBSC_SDPTCR_SCR_FP;	/* Disable Secure Protect for Debug */
	*SBSC_SDPTCR5A &= ~SBSC_SDPTCR_SCR_FP;	/* Disable Secure Protect for Debug */
	*SBSC_SDPTCR6A &= ~SBSC_SDPTCR_SCR_FP;	/* Disable Secure Protect for Debug */
	*SBSC_SDPTCR7A &= ~SBSC_SDPTCR_SCR_FP;	/* Disable Secure Protect for Debug */
	
	*CPG_PLLECR &= ~CPG_PLLECR_PLL3E;
	
	/* Wait PLL3 status off */
	while (1)
	{
		work = *CPG_PLLECR;
		work &= CPG_PLLECR_PLL3ST;
		if (work == 0)
		{
			break;
		}
	}
	
	/* Set PLL3 = 1040 Mhz*/
	*CPG_PLL3CR  = CPG_PLL3CR_1040MHZ;
	
	*CPG_PLLECR |= CPG_PLLECR_PLL3E;
	/* Wait PLL3 status on */
	while (1)
	{
		work = *CPG_PLLECR;
		work &= CPG_PLLECR_PLL3ST;
		if (work == CPG_PLLECR_PLL3ST)
		{
			break;
		}
	}
	
	/* ZB3 = 1/2 * PLL3 = 520 Mhz */
	*CPG_FRQCRD = CPG_FRQCRD_DIV2;
	*CPG_FRQCRB |= CPG_FRQCRB_KICK;
	/* Wait FRQCRB.KICK bit is 0 - EOS */
	while (1)
	{
		work = *CPG_FRQCRB;
		work &= CPG_FRQCRB_KICK;
		if (work == 0)
		{
			break;
		}
	}
	/* Set Power-Down Control */
	*SBSC_SDCR0A     |= SBSC_SDCR0A_PDOWN3;
	*SBSC_SDCR1A     |= SBSC_SDCR1A_PDOWN3;
	
	/* Set flag for skip some SBSC setting flow when SW reset would occur*/
	SYSC_Set_SWReset_Flag();
}

/**
 * SBSC_Init - SBSC initialize process
 * @return None
 */
void SBSC_Init(void)
{
	/* ECR0069 W/A */
	*SHBMCTR2 = 0x00030027;
	if (CHIP_VERSION() == CHIP_RMU2_ES10){
		SBSC_Init_ES10();
	}
	else if (CHIP_VERSION() >= CHIP_RMU2_ES22){ 
		SBSC_Init_520Mhz();
	}
	else{
		SBSC_Init_390Mhz();
	}
}

/**
 * SBSC_SW_Semaphore - SBSC SW Semaphore initialize process
 * @return None
 */
void SBSC_SW_Semaphore(void)
{
	/* Clear SW semaphore(SD-RAM) 128 bytes*/ 
	int i = 0;
	while (i < 32){
		*(SW_SEMAPHORE_1 + i) = 0x00;
		i++;
	}
}