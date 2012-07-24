/* P_LibAssp.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

/******* include file ********************************************************/
#include "c_typedef.h"
#include "c_defusr.h"


/**
 * P_isConfigured - Check state ( Configured )
 * @return YES : Configured
 *         NO  : not Configured
 */
U16 P_isConfigured(void)
{
	U16	buf;

	USBRD(INTSTS0, buf);
	if ((buf & DVSQ) == DS_CNFG)
		return YES;
	return NO;
}


/**
 * P_EpToPipe - Endpoint number to pipe number
 * @return DONE  : Pipe Number
 *         ERROR : EP_ERROR
 */
U16 P_EpToPipe(U16 Dir_Ep)
{
	U16	i;

	for (i = 1; i <= MAX_PIPE_NO; ++i) {
		if ((PipeTbl[i] & 0x00ff) == Dir_Ep)
			return i;
	}
	return EP_ERROR;
}
