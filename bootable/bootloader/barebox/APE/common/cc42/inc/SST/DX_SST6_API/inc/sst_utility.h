/***********************************************************************************  
 * Copyright 2009 © Discretix Technologies Ltd. 
 * This software is protected by copyright, international treaties and 
 * various patents. If the license governing the use of this Software 
 * allows copy or redistribution of this  software then any copy or 
 * reproduction of this Software must include this Copyright Notice 
 * as well as any other notices provided under such license. 
 ***********************************************************************************/
 
 
 /*! \file 
**********************************************************************************	
* Title:						Discretix SST utility API header file						 					
*																			
* Filename:					    sst_utility.h															
*																			
* Project, Target, subsystem:	SST 6.0, Utility, API
* 
* Created:						21.08.2007														
*
* Modified:						21.08.2007										
*
* \Author						Einat Ron														
*																			
* \Remarks						
**********************************************************************************/


#ifndef _DX_SST_UTILITY_H_
    #define _DX_SST_UTILITY_H_
#ifdef __cplusplus
extern "C" {
#endif

    /*----------- Global Includes ------------------------------------------------*/
     #include "sst_types.h" 
     #include "NVS.h"	
	 /*----------- Global defines -------------------------------------------------*/  
	/* The size of the memory buffer required to store the pages needed by the SST */
    /* This definition can be used to allocate sufficient memory for run time page */
    /* storage .                                                                   */    
    #define SST_UTIL_DB_PAGES_MEMORY_SIZE_IN_BYTES		(2048UL)

    
    /*----------- Global function prototypes -------------------------------------*/
     /*Utility services*/
        
        /*SST_DataHandleBuild*/
        /*!
        \brief Build a handle of a data object stored in the SST according to its type and bits set by the user.

        @param aDataType        [in]     The type of the relevant object. 
        @param aDataObjID       [in]     The ID provided by the user (23 LSBs will be taken to account). 
        @param aDataHandle_ptr  [out]    The returned handle.

        @return SST_RC_OK The operation completed successfully.
        @return SST_RC_FAIL The operation failed.
        @return SST_RC_ERROR_NULL_POINTER At least one of the past arguments is null.

        **/        
        DxError_t SST_DataHandleBuild(SSTDataType_t aDataType,SSTObjectId_t aDataObjID,SSTHandle_t *aDataHandle_ptr);
    
        /*SST_DataTypeGet*/
        /*!
        \brief Retrieves the type of an object by its handle.

        @param aObjHandle   [in]     The handle of the relevant object. 
        @param aObjType_ptr [out]    The returned type.

        @return SST_RC_OK The operation completed successfully.
        @return SST_RC_FAIL The operation failed.
        @return SST_RC_ERROR_NULL_POINTER At least one of the past arguments is null.

        **/
        DxError_t SST_DataTypeGet(SSTHandle_t aDataHandle,SSTDataType_t *aDataType_ptr);

        /*SST_DBPagesPointerSet*/
        /*!
        \brief Set the DB pages pointer to an outside given address
        
        @param aDBPagesAddress		[in] Address of the DB pages
		@param aDBPagesSizeInBytes	[in] size of the buffer allocated

        @return SST_RC_OK	                    The operation completed successfully.
        @return SST_RC_FAIL	                    The operation failed.
		@return SST_RC_ERROR_NULL_POINTER		At least one of the past arguments is null.
        **/
        DxError_t SST_UtilDBPagesPointerSet(DxByte_t *aDBPagesAddress,DxUint32_t aDBPagesSizeInBytes);
        
		/*SST_WorkspaceSizeGet*/
		/*!
		\brief Get the minimum workspace size needed per a SST operation

		@param aWorkspaceOp				[in]	describe the type of SST operation 
		@param aWorkspaceMinSize_ptr	[out]	minimum workspace size needed
 
		@return SST_RC_OK	                    The operation completed successfully.
		@return SST_RC_FAIL	                    The operation failed.
		@return SST_RC_ERROR_NULL_POINTER		At least one of the past arguments is null.

		Note:	Default size given is the maximum size among all other min, 
				can be retrieved by using SST_WORKSPACE_GENERAL as well
		**/
		DxError_t SST_WorkspaceSizeGet(SSTWorkspaceOp_t aWorkspaceOp,DxUint32_t *aWorkspaceMinSize_ptr);



        /* SST_UtilNVSSizesGet */
        /*!
        \brief 
        Retrieve the maximal size of the NVS and the currently used size
        according to the memory ID.

        @param memoryID	            [in]  identifier to the relevant memory.
        @param maxSizeInBytes_ptr	[out] a pointer to store the max size.
        @param usedSizeInBytes_ptr	[out] a pointer to store the used size.

        @return SST_RC_OK The operation completed successfully.
        @return SST_RC_FAIL The operation failed (Relevant error from inside modules)   
        @return SST_RC_ERROR_CLOSED The SST is closed (SST hasn't gone through a proper init)
        @return SST_RC_ERROR_WRONG_MEMORY_ID The received memory ID does not correspond to any of the memory IDs currently used

        @notes: it is allowed to pass only one pointer and set the other to NULL.  
        **/
        DxError_t SST_UtilNVSSizesGet(DxNvsMemoryId_t aMemoryID, 
                                      DxUint32_t      *aMaxSizeInBytes_ptr,
                                      DxUint32_t      *aUsedSizeInBytes_ptr);

 

        /* SST_UtilNVSMaxSizeSet */
        /*!
        \brief 
        Set the maximal NVS size to be used by the memory identified.

        @param memoryID	            [in]  identifier to the relevant memory.
        @param newMaxSizeInBytes	[in]  the new maximal NVS size

        @return SST_RC_OK The operation completed successfully.
        @return SST_RC_FAIL The operation failed (Relevant error from inside modules)   
        @return SST_RC_ERROR_CLOSED The SST is closed (SST hasn't gone through a proper init)
        @return SST_RC_ERROR_WRONG_MEMORY_ID The received memory ID does not correspond to any of the memory IDs currently used
        @return SST_RC_ERROR_NVS_MAX_SIZE_TOO_SMALL The requested size is too small 
        @return SST_RC_ERROR_NVS_MAX_SIZE_TOO_BIG The requested size is too big 
        **/
        DxError_t SST_UtilNVSMaxSizeSet(DxNvsMemoryId_t aMemoryID, 
                                        DxUint32_t      aNewMaxSizeInBytes);


       
    #ifdef __cplusplus
	}
	#endif       
#endif  /* _DX_SST_UTILITY_H_ */
