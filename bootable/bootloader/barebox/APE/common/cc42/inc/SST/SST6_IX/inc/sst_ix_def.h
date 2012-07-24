/*! \file 
**********************************************************************************
* Title:                            Discretix SST IX Header file
*
* Filename:                         sst_ix_def.h
*
* Project, Target, subsystem:       SST 6.0, SST6, Index Lookup Utility
* 
* Created:                          17.04.2007
*
* Modified:							07.06.2007 
*
* \Author                           Ira Boguslavsky
*
* \Remarks                        
*           Copyright (C) 2006 by Discretix Technologies Ltd. All Rights reserved.
**********************************************************************************/
#ifndef _DX_SST_IX_DEF_
	#define _DX_SST_IX_DEF_
	
	#include "sst_general_def.h"

	/* Defines the index metadata digest */
	#define SST_IX_IMD_HANDLE_DIGEST									(0xABABABABUL)

	#define SST_IX_DATA_OFFSET											(128UL)
	#define SST_IX_METADATA_DIGEST										(0x00000000UL)
	#define SST_IX_DIGEST_LENGTH_IN_BYTES								(32UL)
#endif
