/*! \file 
**********************************************************************************	
* Title:						Discretix SST VRPC header file						 					
*																			
* Filename:						sst_vrpc.h															
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

#ifndef _DX_SST_VRPC_H_
    #define _DX_SST_VRPC_H_

	#include "sst_debug.h"
	
	#include "sst_vrpc_config.h"
		
	/*----------- Global defines -------------------------------------------------*/
	
    /*----------- Global macro definitions ---------------------------------------*/

    /*----------- Global type definitions ----------------------------------------*/
	/* Definition of the VRPC value  */
	typedef		DxUint32_t		SSTVRPCValue_t;

	/* Specifies the Return Codes from this module */
	typedef enum
	{
		SST_VRPC_RC_OK							= 0,
		SST_VRPC_RC_FAIL						= 1,
		SST_VRPC_RC_ERROR_WRITE_FAIL			= 2,
		SST_VRPC_RC_ERROR_READ_FAIL				= 3,
		SST_VRPC_RC_ERROR_TIMED_OUT				= 4,
		SST_VRPC_RC_ERROR_SEMAPHORE_FAIL		= 5,
		SST_VRPC_RC_ERROR_VOS_FAIL				= 6,
		SST_VRPC_RC_ERROR_INCREMENT_FAIL		= 7,

		SST_VRPC_RETURN_CODES_FORCE_INT32       = 0x7FFFFFFF /* force enum to 32 bit in all compilers */

	}SSTVRPCReturnCodes_t;
    /*----------- Extern definition ----------------------------------------------*/

    /*----------- Global variable declarations -----------------------------------*/
	
    /*----------- Global constant definitions ------------------------------------*/
	
    /*----------- Global function prototypes -------------------------------------*/
	
	
	/* Init functions*/

	/*!
	\brief 
	Initialize the VRPC  
	@param fileId	Identification of the file, to sperate it from other VRPC file
	@return SST_VRPC_RC_OK						On success
	@return	SST_VRPC_RC_FAIL					Operation failed 
	@return SST_VRPC_RC_ERROR_SEMAPHORE_FAIL	Error when using semaphore
	@return SST_VRPC_RC_ERROR_VOS_FAIL			Error when using VOS
	@return	SST_VRPC_RC_ERROR_WRITE_FAIL		Write operation failed
	**/
	SSTVRPCReturnCodes_t      SST_VRPCInit				(DxUint32_t fileId);

	
	/*!
	\brief 
	Terminate the VRPC and free any used resources 
	@return SST_VRPC_RC_OK						On success
	@return	SST_VRPC_RC_FAIL					Operation failed 
	@return SST_VRPC_RC_ERROR_SEMAPHORE_FAIL	Error when using semaphore
	@return SST_VRPC_RC_ERROR_VOS_FAIL			Error when using VOS
	**/
	SSTVRPCReturnCodes_t	    SST_VRPCTerminate			(void);
	
	/*----------------------------------------------------------------------------*/
	/* API functions*/
	/*!
	\brief 
	Get the current value of the VRPC 
	@param  vrpcValue_ptr						The VRPC current value
	@return SST_VRPC_RC_OK						On success
	@return	SST_VRPC_RC_FAIL					Operation failed 
	@return SST_VRPC_RC_ERROR_SEMAPHORE_FAIL	Error when using semaphore
	@return SST_VRPC_RC_ERROR_VOS_FAIL			Error when using VOS
	@return	SST_VRPC_RC_ERROR_READ_FAIL			Read operation failed
**/
	SSTVRPCReturnCodes_t      SST_VRPCCurrentGet			(SSTVRPCValue_t *vrpcValue_ptr);


	/*!
	\brief 
	Get the next expected value of the VRPC
	@param  vrpcValue_ptr						The VRPC next value
	@return SST_VRPC_RC_OK						On success
	@return	SST_VRPC_RC_FAIL					Operation failed 
	@return SST_VRPC_RC_ERROR_SEMAPHORE_FAIL	Error when using semaphore
	@return SST_VRPC_RC_ERROR_VOS_FAIL			Error when using VOS
	@return SST_VRPC_RC_ERROR_TIMED_OUT			The VRPC is in use and cannot be read
	@return	SST_VRPC_RC_ERROR_READ_FAIL			Read operation failed
	**/
	SSTVRPCReturnCodes_t      SST_VRPCNextGet			(SSTVRPCValue_t *vrpcValue_ptr);


	/*!
	\brief 
	Increment the value of the VRPC  
	@return SST_VRPC_RC_OK						On success
	@return	SST_VRPC_RC_FAIL					Operation failed 
	@return SST_VRPC_RC_ERROR_SEMAPHORE_FAIL	Error when using semaphore
	@return SST_VRPC_RC_ERROR_VOS_FAIL			Error when using VOS
	@return SST_VRPC_RC_ERROR_TIMED_OUT			The VRPC is in use and cannot be read
	@return	SST_VRPC_RC_ERROR_READ_FAIL			Read operation failed
	@return	SST_VRPC_RC_ERROR_WRITE_FAIL		Write operation failed
	**/
	SSTVRPCReturnCodes_t      SST_VRPCIncrement			(void);
	
#endif  /* _DX_SST_VRPC_H_ */
