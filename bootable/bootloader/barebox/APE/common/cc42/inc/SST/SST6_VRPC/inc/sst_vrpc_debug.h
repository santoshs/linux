/*! \file 
**********************************************************************************	
* Title:						Discretix SST VRPC Debug Header File						 					
*																			
* Filename:						sst_vrpc_debug.h 															
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

#ifndef _DX_SST_VRPC_DEBUG_H_
    #define _DX_SST_VRPC_DEBUG_H_

	#include "sst_vrpc_def.h"

	/*----------- Global defines -------------------------------------------------*/
	
    /*----------- Global macro definitions ---------------------------------------*/

    /*----------- Global type definitions ----------------------------------------*/
	
    /*----------- Extern definition ----------------------------------------------*/

    /*----------- Global variable declarations -----------------------------------*/
	
    /*----------- Global constant definitions ------------------------------------*/
	
    /*----------- Global function prototypes -------------------------------------*/
	    
    #if (defined SST_VRPC_DEBUG_MODE_ENABLED)
		/*----------- Global defines -------------------------------------------------*/
		#define SST_VRPC_DEBUG_SET_VALUE					10
		
		/*----------- Global function prototypes -------------------------------------*/
		/* Debug functions.	*/
		/*!
		\brief 
		Set the VRPC value to inputted value 
		@param  vrpcValue_ptr						The VRPC new value
		@return SST_VRPC_RC_OK						On success
		@return	SST_VRPC_RC_FAIL					Operation failed 
		@return SST_VRPC_RC_ERROR_SEMAPHORE_FAIL	Error when using semaphore
		@return SST_VRPC_RC_ERROR_VOS_FAIL			Error when using VOS
		@return SST_VRPC_RC_ERROR_TIMED_OUT			The VRPC is in use and cannot be read
		@return	SST_VRPC_RC_ERROR_WRITE_FAIL		Write operation failed
		@return	SST_VRPC_RC_ERROR_READ_FAIL			Read operation failed
		**/
		SSTVRPCReturnCodes_t      SST_VRPC_DBGSet    (SSTVRPCValue_t *vrpcValue_ptr);
		
    #endif /*SST_VRPC_DEBUG_MODE_ENABLED*/

	
#endif  /* _DX_SST_VRPC_DEBUG_H_ */
	
	

		

