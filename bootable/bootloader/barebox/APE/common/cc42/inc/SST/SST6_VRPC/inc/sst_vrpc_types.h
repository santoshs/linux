/*! \file 
**********************************************************************************	
* Title:						Discretix SST VRPC Type Definitions Header File						 					
*																			
* Filename:						sst_vrpc_types.h 															
*																			
* Project, Target, subsystem:	SST 6.0 Virtual Reflash Protection Counter
* 
* Created:						05.03.2007														
*
* Modified:						07.06.2007										
*
* \Author						Einat Ron														
*																			
* \Remarks						Copyright (C) 2006 by Discretix Technologies Ltd.     			
*  								All Rights reserved											
**********************************************************************************/

#ifndef _DX_SST_VRPC_TYPES_H_
    #define _DX_SST_VRPC_TYPES_H_

	#include "DX_VOS_BaseTypes.h"

    #if defined EXTERN
        #error EXTERN ALREADY DEFINED !!! (vrpc_types.h) 
    #else
        #if defined _DX_SST_VRPC_NO_EXTERN_
            #define EXTERN 
        #else
            #define EXTERN extern
        #endif /* _DX_SST_VRPC_NO_EXTERN_*/
    #endif /*EXTERN*/

    /*----------- Local type definitions -----------------------------------------*/
 	typedef struct  
	{	
		DxBool_t	isActive;
	}SSTVRPCRuntimeVars_t;
	/*----------- Package global variables --------------------------------------*/
		
	#undef EXTERN

#endif  /* _DX_SST_VRPC_TYPES_H_ */
