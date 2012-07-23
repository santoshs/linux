/*! \file 
**********************************************************************************	
* Title:						Discretix SST IX header file						 					
*																			
* Filename:						sst_ix_hash.h															
*																			
* Project, Target, subsystem:	SST 6.0 Index Lookup Utility
* 
* Created:						26.04.2007														
*
* Modified:						07.06.2007 										
*
* \Author						Ira Boguslvasky													
*																			
* \Remarks						Copyright (C) 2007 by Discretix Technologies Ltd.     			
*  								All Rights reserved											
**********************************************************************************/
#ifndef _DX_SST_IX_HASH_H_
#define _DX_SST_IX_HASH_H_

#include "DX_VOS_BaseTypes.h"

SSTObjectDigest_t SST_IXHashFunc(DxByte_t* str_ptr, DxUint32_t lenInBytes);
#endif
