/*! \file 
**********************************************************************************	
* Title:						Discretix SST VRPC Type Definitions Header File						 					
*																			
* Filename:						sst_vrpc_stub_types.h 															
*																			
* Project, Target, subsystem:	SST 6.0 Virtual Reflash Protection Counter
* 
* Created:						09.10.2007														
*
* Modified:						09.10.2007										
*
* \Author						Einat Ron														
*																			
* \Remarks						Copyright (C) 2006 by Discretix Technologies Ltd.     			
*  								All Rights reserved											
**********************************************************************************/

#ifndef _DX_SST_VRPC_STUB_TYPES_H_
    #define _DX_SST_VRPC_STUB_TYPES_H_

	#include "DX_VOS_BaseTypes.h"
	#include "DX_VOS_File.h"
	#include "DX_VOS_Sem.h"

    /*----------- Local type definitions -----------------------------------------*/
 	typedef struct  
	{	
        DxVosFile	vrpcFileHandle;
        DxVosSem	vrpcSem; 
	}SSTVRPCFileRuntimeVars_t;
	/*----------- Global Definitions --------------------------------------*/
    
   	#define SST_VRPC_FILE_NAME_SIZE				        (64UL)
    #define SST_VRPC_DEFAULT_FILE_NAME_SIZE_IN_BYTES	(9UL) 
    #define SST_VRPC_DEFAULT_FILE_NAME					"sst_vrpc_"
    
    #if (SST_VRPC_FILE_NAME_SIZE < SST_VRPC_DEFAULT_FILE_NAME_SIZE_IN_BYTES)
        #error "SST_VRPC_FILE_NAME_SIZE is not sufficient"
    #endif
		
#endif  /* _DX_SST_VRPC_STUB_TYPES_H_ */
