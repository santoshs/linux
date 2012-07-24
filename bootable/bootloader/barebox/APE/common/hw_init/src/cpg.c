/*	cpg.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "cpg.h"
#include "common.h"

/**
 * CPG_Set_Clock_ES10 - Set clock for ES1.0
 * @return None
 */
void CPG_Set_Clock_ES10(CPU_CLOCK clock)
{
	ulong work = 0;
	ulong pclkcr_data = 0;
	ulong frqcra_data = 0;
	ulong pll0cr_data = 0;
	
	/* CPU clock */
	switch(clock){
		case CPU_CLOCK_988MHZ:
			/* 988MHz setting */
			pclkcr_data = CPG_PCLKCR_988MHZ;	/* PCLKCR setting */
			frqcra_data = CPG_FRQCRA_988MHZ;	/* FRQCRA setting */
			pll0cr_data = CPG_PLL0CR_X38;		/* PLLCR0 setting */
			break;
		case CPU_CLOCK_104MHZ:
			pclkcr_data = CPG_PCLKCR_104MHZ;
			frqcra_data = CPG_FRQCRA_104MHZ;
			pll0cr_data = CPG_PLL0CR_X4;
			break;
		case CPU_CLOCK_26MHZ:
			pclkcr_data = CPG_PCLKCR_26MHZ;
			frqcra_data = CPG_FRQCRA_26MHZ;
			pll0cr_data = CPG_PLL0CR_X1;
			break;
		default:
			pclkcr_data = CPG_PCLKCR_988MHZ;	/* PCLKCR setting */
			frqcra_data = CPG_FRQCRA_988MHZ;	/* FRQCRA setting */
			pll0cr_data = CPG_PLL0CR_X38;		/* PLLCR0 setting */
			break;
	}
			
	/* CPG register setting process */

	/* Turn off all PLL circuit */
	*CPG_PLLECR  = CPG_PLLECR_CLKOFF;

	/* wait PLL status off */
	while (1)
	{
		work = *CPG_PLLECR;
		work &= CPG_PLLECR_STOFF;
		if (work == 0)
		{
			break;
		}
	}

	*CPG_FRQCRA = frqcra_data;
	*CPG_FRQCRB = CPG_FRQCRB_DATA;
	*CPG_FRQCRB |= CPG_FRQCRB_KICK;

	/* wait FRQCRB KICK bit is 0 */
	while (1)
	{
		work = *CPG_FRQCRB;
		work &= CPG_FRQCRB_KICK;
		if (work == 0)
		{
			break;
		}
	}

	*CPG_PCLKCR    = pclkcr_data;
	*CPG_VCLKCR1   = CPG_SETCLOCK_24MHZ;/* 24MHz */
	*CPG_VCLKCR2   = CPG_SETCLOCK_24MHZ;/* 24MHz */
	*CPG_VCLKCR3   = CPG_SETCLOCK_26MHZ;/* 26MHz */
	*CPG_VCLKCR4   = CPG_VCLKCR4_DATA;	/* 13MHz */

	*CPG_ZBCKCR    = CPG_ZBCKCR_BSC;
	*CPG_SD0CKCR   = CPG_SD0CKCR_DATA;
	*CPG_SD1CKCR   = CPG_SD1CKCR_DATA;
	*CPG_FSIACKCR  = CPG_SETCLOCK_9800KHZ;
	*CPG_FSIBCKCR  = CPG_FSIBCKCR_DATA;
	*CPG_MPCKCR    = CPG_MPCKCR_48MHz;

	*CPG_SPUACKCR  = CPG_SETCLOCK_104MHZ;
	*CPG_HSICKCR   = CPG_SETCLOCK_208MHZ;
	*CPG_DSITCKCR  = CPG_SETCLOCK_78MHZ;
	*CPG_DSI0PCKCR = CPG_DSI0PCKCR_DATA;

	*CPG_DSI0PHYCR = CPG_DSI0PHYCR_DATA;
	*CPG_PLL0CR    = pll0cr_data;

	*CPG_PLL1CR    = CPG_PLL1CR_DATA;
	*CPG_PLL2CR    = CPG_PLL2CR_DATA;
	

	*CPG_PLL0STPCR = CPG_PLL0STPCR_DATA;
	*CPG_PLL1STPCR = CPG_PLL1STPCR_DATA;


	if (clock == CPU_CLOCK_26MHZ)
	{
		*CPG_PLLECR    = (CPG_PLLECR_PLL3E | CPG_PLLECR_PLL1E);
	}
	else
	{
		*CPG_PLLECR    = CPG_PLLECR_CLKON;
	}

	/* wait PLL status on */
	while (1)
	{
		work = *CPG_PLLECR;

		if (clock == CPU_CLOCK_26MHZ)
		{
			work &= (CPG_PLLECR_PLL3ST | CPG_PLLECR_PLL1ST);
			if (work == (CPG_PLLECR_PLL3ST | CPG_PLLECR_PLL1ST))
			{
				break;
			}
		}
		else
		{
			work &= CPG_PLLECR_STON;
			if (work == CPG_PLLECR_STON)
			{
				break;
			}
		}
	}

	*CPG_MPMODE  = CPG_MPMODE_DATA;
	*CPG_VREFCR  = CPG_VREFCR_DATA;
}
/**
 * CPG_Set_Clock_ES20 - Set clock for ES2.0
 * @return None
 */
void CPG_Set_Clock_ES20(CPU_CLOCK clock)
{
	ulong work = 0;
	ulong pclkcr_data = 0;
	ulong frqcra_data = 0;
	ulong pll0cr_data = 0;
	
	/* CPU clock */
	switch(clock){
		case CPU_CLOCK_1456MHZ:
			/* 988MHz setting */
			pclkcr_data = CPG_PCLKCR_1456MHZ;	/* PCLKCR setting */
			frqcra_data = CPG_FRQCRA_1456MHZ;	/* FRQCRA setting */
			pll0cr_data = CPG_PLL0CR_X56;		/* PLLCR0 setting */
			break;
		case CPU_CLOCK_1196MHZ:
			/* 988MHz setting */
			pclkcr_data = CPG_PCLKCR_1196MHZ;	/* PCLKCR setting */
			frqcra_data = CPG_FRQCRA_1196MHZ;	/* FRQCRA setting */
			pll0cr_data = CPG_PLL0CR_X46;		/* PLLCR0 setting */
			break;
		case CPU_CLOCK_104MHZ:
			pclkcr_data = CPG_PCLKCR_104MHZ;	/* PCLKCR setting */
			frqcra_data = CPG_FRQCRA_104MHZ;	/* FRQCRA setting */
			pll0cr_data = CPG_PLL0CR_X4;		/* PLLCR0 setting */
			break;	
		case CPU_CLOCK_26MHZ:
			pclkcr_data = CPG_PCLKCR_26MHZ;		/* PLLCR0 setting */
			frqcra_data = CPG_FRQCRA_26MHZ;		/* PLLCR0 setting */
			pll0cr_data = CPG_PLL0CR_X1;		/* PLLCR0 setting */
			break;
		default:
			pclkcr_data = CPG_PCLKCR_1196MHZ;	/* PCLKCR setting */
			frqcra_data = CPG_FRQCRA_1196MHZ;	/* FRQCRA setting */
			pll0cr_data = CPG_PLL0CR_X46;		/* PLLCR0 setting */
			break;
	}
	
	/* CPG register setting process */

	/* Turn off all PLL circuit */
	*CPG_PLLECR  = CPG_PLLECR_CLKOFF;

	/* wait PLL status off */
	while (1)
	{
		work = *CPG_PLLECR;
		work &= CPG_PLLECR_STOFF;
		if (work == 0)
		{
			break;
		}
	}

	*CPG_FRQCRA = frqcra_data;
	*CPG_FRQCRB = CPG_FRQCRB_DATA;
	*CPG_FRQCRB |= CPG_FRQCRB_KICK;

	/* wait FRQCRB KICK bit is 0 */
	while (1)
	{
		work = *CPG_FRQCRB;
		work &= CPG_FRQCRB_KICK;
		if (work == 0)
		{
			break;
		}
	}

	*CPG_PCLKCR    = pclkcr_data;
	*CPG_VCLKCR1   = CPG_SETCLOCK_24MHZ;	/* 24MHz */
	*CPG_VCLKCR2   = CPG_SETCLOCK_24MHZ;	/* 24MHz */
	*CPG_VCLKCR3   = CPG_SETCLOCK_26MHZ;	/* 26MHz */
	*CPG_VCLKCR4   = CPG_SETCLOCK_26MHZ;	/* 26MHz */
	*CPG_VCLKCR5   = CPG_SETCLOCK_26MHZ;	/* 26MHz */

	*CPG_ZBCKCR    = CPG_ZBCKCR_BSC;
	*CPG_SD0CKCR   = CPG_SD0CKCR_DATA;
	*CPG_SD1CKCR   = CPG_SD1CKCR_DATA;
	*CPG_FSIACKCR  = CPG_SETCLOCK_9800KHZ;
	*CPG_FSIBCKCR  = CPG_FSIBCKCR_DATA;
	*CPG_MPCKCR    = CPG_MPCKCR_48MHz;

	*CPG_SPUACKCR  = CPG_SETCLOCK_104MHZ; /* 104MHz */
	*CPG_SPU2VCKCR = CPG_SETCLOCK_156MHZ; /* 156MHz */
	*CPG_SLIMBCKCR = CPG_SETCLOCK_104MHZ; /* 104MHz */
	*CPG_HSICKCR   = CPG_SETCLOCK_208MHZ;
	*CPG_DSITCKCR  = CPG_SETCLOCK_78MHZ;
	*CPG_DSI0PCKCR = CPG_SETCLOCK_26MHZ;	
	*CPG_DSI0PHYCR = CPG_DSI0PHYCR_DATA;
	
	*CPG_DSI1PCKCR = CPG_DSI1PCKCR_DATA; /* 74.3 MHz */		
	*CPG_M4CKCR    = CPG_M4CKCR_DATA; 	 /* M4 = 104 MHz, M4D2 = 52 MHz */
	*CPG_PLL0CR    = pll0cr_data;
	*CPG_PLL1CR    = CPG_PLL1CR_DATA;
	*CPG_PLL2CR    = CPG_PLL2CR_DATA;
	
	*CPG_PLL0STPCR = CPG_PLL0STPCR_DATA;
	*CPG_PLL1STPCR = CPG_PLL1STPCR_DATA;
	
	if (clock == CPU_CLOCK_26MHZ)
	{
		*CPG_PLLECR    |= (CPG_PLLECR_PLL1E);
	}
	else
	{
		*CPG_PLLECR    = CPG_PLLECR_CLKON;
	}

	/* wait PLL status on */
	while (1)
	{
		work = *CPG_PLLECR;

		if (clock == CPU_CLOCK_26MHZ)
		{
			work &= (CPG_PLLECR_PLL1ST);
			if (work == CPG_PLLECR_PLL1ST)
			{
				break;
			}
		}
		else
		{
			work &= CPG_PLLECR_STON;
			if (work == CPG_PLLECR_STON)
			{
				break;
			}
		}
	}

	*CPG_MPMODE  = CPG_MPMODE_DATA;
	*CPG_VREFCR  = CPG_VREFCR_DATA;

}

/**
 * CPG_Set_Clock - Set clock
 * @return None
 */
void CPG_Set_Clock(CPU_CLOCK clock)
{
	if (CHIP_VERSION() == CHIP_RMU2_ES10){
		CPG_Set_Clock_ES10(clock);
	}
	else{ 
		CPG_Set_Clock_ES20(clock);
	}
}

/**
 * CPG_InterconnectRAM_Enable - Enable Interconnect RAM/Secure RAM
 * @return None
 */
void CPG_InterconnectRAM_Enable(void)
{
	ulong value;
	/* Enable secure boot ROM, secure RAM and Interconnect RAM */ 
	*CPG_SMSTPCR5 &= ~((1<<30)|(1<<29)|(1<<28)|(1<<27));
	
	while(1)
	{
		value = *CPG_MSTPSR5;
		if((value & 0x78000000) == 0x00000000)
			break;
	}
}

/**
 * CPG_2nd_Setting - Second HW setting
 * @return None
 */
void CPG_2nd_Setting(void)
{
	*CPG_DSITCKCR  |= 0x00000100;
	*CPG_DSI0PCKCR |= 0x00000300;
	*CPG_DSI0PHYCR &= ~0x00008000;
	
	*CPG_RMSTPCR0 &= ~CPG_MSTP019;		 /* H-UDI */
	*CPG_RMSTPCR0 &= ~CPG_MSTP018;		 /* RT_CPU debug module1 */
	/* For connecting E10A */
	*CPG_RMSTPCR0 &= ~CPG_MSTP017;
}

/**
 * CPG_Stop_VCK3clock - Stop VCK3
 * @return None
 */
void CPG_Stop_VCK3clock(void)
{
	*CPG_VCLKCR3   = CPG_SETCLOCK_26MHZ; /* Stop VCK3 */
}

