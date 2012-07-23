/*! \file 
**********************************************************************************	
* Title:						Discretix SST VDB Debug Header File				 					
*																			
* Filename:						sst_vdb_debug.h 															
*																			
* Project, Target, subsystem:	SST 6.0 Virtual DataBase
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

#ifndef _DX_SST_VDB_DEBUG_H_
    #define _DX_SST_VDB_DEBUG_H_

	#include "sst_vdb_def.h"
	/*----------- Global defines -------------------------------------------------*/
	
    /*----------- Global macro definitions ---------------------------------------*/

    /*----------- Global type definitions ----------------------------------------*/
	
    /*----------- Extern definition ----------------------------------------------*/

    /*----------- Global variable declarations -----------------------------------*/
	
    /*----------- Global constant definitions ------------------------------------*/
	
    /*----------- Global function prototypes -------------------------------------*/
	    
    #if (defined SST_VDB_DEBUG_MODE_ENABLED)
		/*----------- Global defines -------------------------------------------------*/
		
		
		/*----------- Global function prototypes -------------------------------------*/
		/* Debug functions.	*/
		
	
		/*!
		\brief 
		Delete an existing DB.
		@param mainMemoryId	[in] The main memory Id that will be used to store the DB
		@return SST_VDB_RC_OK				On success
		@return	SST_VDB_RC_FAIL				Operation failed 
		@return	SST_VDB_RC_ERROR_DB_IS_OPEN	if DB is already open
		@return SST_VDB_RC_ERROR_DB			DB operation failed
		
		@notes: 1. In order to perform this operation the DB must be closed
				2. Using this function will erase the entire DB and it cannot be recovered after
				3. The VDB does not support working with more than one file in the directory

		**/
		SSTVDBReturnCodes_t SST_VDB_DBGDelete (DxNvsMemoryId_t  mainMemoryId);
		
		/* security functions.	*/
		#ifdef SST_SECURITY_TESTING_ENABLED
			
			/*SST_VDBDebugMoMOffsetRetrieve*/
			/*!
			\brief Retrieves the MoM offset in the Memory (flash or file)
			Note SST must be initialized before this function is called
		    
			@param momOffset_ptr	[out] The offset of the MoM in the memory

			@return SST_VDB_RC_OK					On success
			@return	SST_VDB_RC_FAIL					Operation failed 
			@return SST_VDB_RC_ERROR_NULL_POINTER	At least one of the arguments is null
			@return SST_VDB_RC_ERROR_DB				DB operation failed
			**/
			SSTVDBReturnCodes_t SST_VDBDebugMoMOffsetRetrieve(DxUint32_t *momOffset_ptr);
		

			/*SST_VDBDebugRecordInfoRetrieve*/
			/*!
			\brief Retrieves the page offset and the record index in the page of a given record handle
			Note SST must be initialized before this function is called

			@param recordHandle			[in] The record handle
			@param pageOffset_ptr		[out] The page offset of the page that points to the record
			@param pageRecordIndex_ptr	[out] The record index in the page

			@return SST_VDB_RC_OK						On success
			@return	SST_VDB_RC_FAIL						Operation failed 
			@return SST_VDB_RC_ERROR_NULL_POINTER		At least one of the arguments is null
			@return SST_VDB_RC_ERROR_DB					DB operation failed
			@return SST_VDB_RC_ERROR_RECORD_NOT_FOUND	The requested record was not found
			**/
			SSTVDBReturnCodes_t SST_VDBDebugRecordInfoRetrieve(SSTHandle_t  recordHandle,
															   DxUint32_t	*pageOffset_ptr, 
															   DxUint32_t	*pageRecordIndex_ptr);
		#endif		/*SST_SECURITY_TESTING_ENABLED*/
		
    #endif /*SST_VDB_DEBUG_MODE_ENABLED*/

	
#endif  /* _DX_SST_VDB_DEBUG_H_ */
	
	

		

