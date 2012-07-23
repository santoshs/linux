/*	sysc.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "sysc.h"
#include "cpg.h"
#include "common.h"


/**
 * SYSC_Power_Supply - Turn on power area
 * @return None
 */
void SYSC_Power_Supply(void)
{
	ulong value = 0;
	*SYS_SWUCR = 0x01FFF140;
	while(1)
	{
		value = *SYS_SWUCR;
		if(value == 0x00000000)
			break;
	}			

}
/**
 * SYSC_SW_Reset_flag - Detect SW reset flag
 * @return FALSE: No reset flag detect
 * 		   TRUE: SW reset flag detect
 */
uchar SYSC_SWReset_Flag(void)
{
	uchar ret;
	ret = *SYS_STBCHRB3;
	if((ret & SYS_SWRESET_FLAG)==SYS_SWRESET_FLAG)
	{
		return TRUE;
	}
	return FALSE;
}

/**
 * SYSC_Set_SWReset_Flag - Set SW reset flag
 * @return None
 */
void SYSC_Set_SWReset_Flag(void)
{
	*SYS_STBCHRB3 |= SYS_SWRESET_FLAG;
}

/**
 * SYSC_Soft_Power_On_Reset - Set RESCNT2.SRMD, Keep SDRAM data when SW/HW reset occurrs
 * @return None
 */
void SYSC_Soft_Power_On_Reset(void)
{
	*SYS_RESCNT2 |= SYS_RESCNT2_SRMD;
}

/**
 * SYSC_Reduce_Power - Turn off power areas to reduce power
 * @return None
 */
void SYSC_Reduce_Power(ulong power_mask)
{
	*SYS_SPDCR |= power_mask;
	while(*SYS_SPDCR != 0);
}

/**
 * SYSC_PM_Fix - Turn off C4 and C4CL
 * @return None
 */
void SYSC_PM_Fix(void)
{
	char temp;
	/* W/A: back up STBCHRB0 value*/
	temp = *STBCHRB0;
	
	*SYS_EXMSKCNT1 	= 0x000000A4;
	*SYS_APSCSTP   	= 0x00000100;
	*SYS_C4POWCR   	= 0x00000030;
	*SYS_WUPSCR 	= 0x00000001;
	*SYS_PDNSEL		= 0x0000000C;
	*SYS_SYCKENMSK	= 0x002F0000;
	
	*CPG_PLL1STPCR	 = 0x00012000;
	*CPG_PLL3STPCR	 = 0x00010000;
	
	*STBCHRB0 = temp;
}