/*! \file 
**********************************************************************************	
* Title:						Discretix SST VCRYS Debug Header File				 					
*																			
* Filename:						sst_vcrys_debug.h 															
*																			
* Project, Target, subsystem:	SST 6.0 Virtual Cryptography Services
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

#ifndef _DX_SST_VCRYS_DEBUG_H_
    #define _DX_SST_VCRYS_DEBUG_H_
	
	#include "sst_vcrys_def.h"

	/*----------- Global defines -------------------------------------------------*/
	
    /*----------- Global macro definitions ---------------------------------------*/

    /*----------- Global type definitions ----------------------------------------*/
	
    /*----------- Extern definition ----------------------------------------------*/

    /*----------- Global variable declarations -----------------------------------*/
	
    /*----------- Global constant definitions ------------------------------------*/
	
    /*----------- Global function prototypes -------------------------------------*/
	    
    #if (defined SST_VCRYS_DEBUG_MODE_ENABLED)
		/*----------- Global defines -------------------------------------------------*/
		
		
		/*----------- Global function prototypes -------------------------------------*/
		/* Debug functions.	*/

		
		/* security functions.	*/
		#ifdef SST_SECURITY_TESTING_ENABLED
			
			/*SST_VCRYSDebugSecretKeysRetrieve*/
			/*!
			\brief Retrieves the system secret keys used by the VCRYS, include the data
			encryption key and the integrity key, both of type AES 256 bit.
		    
			@param encrpytKey	[out] The data encryption AES key (32 bytes)
			@param integrityKey	[out] The Integrity AES key used to calculate MAC (16 byte )

			@return SST_VCRYS_RC_OK	                The operation completed successfully.
			@return SST_VCRYS_RC_FAIL	            The operation failed.
			@return SST_VCRYS_RC_ERROR_NULL_POINTER	At least one of the arguments is null
			**/
			SSTVCRYSReturnCodes_t SST_VCRYSDebugSecretKeysRetrieve(DxByte_t *encrpytKey,
																   DxByte_t *integrityKey);
		
		#endif		/*SST_SECURITY_TESTING_ENABLED*/
		
		
    #endif /*SST_VCRYS_DEBUG_MODE_ENABLED*/

	
#endif  /* _DX_SST_VCRYS_DEBUG_H_ */
	
	

		

