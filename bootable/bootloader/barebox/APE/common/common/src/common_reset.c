/* common_reset.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "com_type.h"
#include "common.h"

/********************************************************************************/
/*	Definitions																	*/
/********************************************************************************/
/* Reset Control Register2 */
#define RESCNT2						((volatile ulong*)(0xE6188020))
#define RESCNT2_PRES				((ulong)(0x80000000))


/**
 * ChangeFlashingMode - The processing to change into Flashing Mode
 * @return            - none
 */
void ChangeFlashingMode(void)
{
	SetUsbDownload();
	
	Reset();
	
	/* This code below can't be reached*/
	while(1){};
}


/**
 * Reset   - Soft power on reset processing
 * @return - none
 */
void Reset(void)
{
	/* Set Reset Control Register2 */
	*RESCNT2 |= RESCNT2_PRES;
}

