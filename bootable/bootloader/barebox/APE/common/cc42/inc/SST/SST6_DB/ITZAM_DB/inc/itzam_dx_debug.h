/*
**********************************************************************************	
* Title:						Discretix SST DB Itzam Debug Header file 				 					
*																			
* Filename:						itzam_dx_debug.h			
*																			
* Project, Target, subsystem:	SST 6.0 Itzam DB
* 
* Created:						12.03.2007														
*
* Modified:						07.06.2007										
*
* \Author						Einat Ron														
*																			
* \Remarks						Copyright (C) 2006 by Discretix Technologies Ltd.     			
*  								All Rights reserved											
**********************************************************************************/

#ifndef _DX_SST_ITZAM_DX_DEBUG_H_
    #define _DX_SST_ITZAM_DX_DEBUG_H_


#ifdef SST_DB_ITZAM_DEBUG_MODE_ENABLED
	#include "itzam.h"
	#include "itzam_btree_dx_add_iterator.h"
	/*----------- Global defines -------------------------------------------------*/

	/*----------- Global macro definitions ---------------------------------------*/
	#define SST_MASK_OF_MEMROY_BLOCK			(0x1ff)
	#define SST_NUM_SHIFT_OF_MEMROY_BLOCK		(9UL)

	#define SST_ITZAM_MEMORY_BLOCK_SIZE_ROUNDED_UP(size) \
							((size & SST_MASK_OF_MEMROY_BLOCK) ? \
							(((size>>SST_NUM_SHIFT_OF_MEMROY_BLOCK)+1)\
							<<SST_NUM_SHIFT_OF_MEMROY_BLOCK) : (size)) 

	/*----------- Global type definitions ----------------------------------------*/
	/*define the type of modification done in the page*/	
	typedef enum
	{
		SST_ITZAM_LINK_MAC=0,
		SST_ITZAM_LINK_REF=1,
		SST_ITZAM_RECORD_MAC=2,
		SST_ITZAM_RECORD_LEN=3,
		SST_ITZAM_RECORD_HANDLE=4,
		SST_ITZAM_RECORD_REF=5,
		SST_ITZAM_PAGE_MAC=6,
		SST_ITZAM_PAGE_HANDLE_COUNT=7,
		SST_ITZAM_PAGE_PARENT_REF=8,

		SST_ITZAM_CORRUPT_PAGE_SECTION_FORCE_INT32= 0x7FFFFFFF /* force enum to 32 bit in all compilers */

	}SSTITZAMCorruptPageSection_t;


	/*// Define a structure for analysis of the read and write actions in the DB*/
	typedef struct
	{
		int32_t SST_read_big_counter;      //counter for # of read action for larger than  byte data
		int32_t SST_read_small_counter;    //counter for # of read action for smaller than  byte data
		int32_t SST_read_general_counter;  //counter for total # of read action 
		int32_t SST_write_big_counter;     //counter for # of write action for larger than  byte data
		int32_t SST_write_small_counter;   //counter for # of write action for smaller than  byte data
		int32_t SST_write_general_counter; //counter for total # of write action 
		uint32_t sizeOfSmallestDataBlock;                
		int32_t total_length_write;
		int32_t total_length_read;

	}DXDB_itzamDebugData_t;
		
	// Define a structure for analysis of the read and write actions in the DB per each operation 
	typedef struct
	{
		int32_t insert_total_length_read;
		int32_t insert_total_length_write;
		int32_t SST_insert_read_counter;     //for insert operation
		int32_t SST_insert_write_counter;
		
		int32_t find_total_length_read;
		int32_t find_total_length_write;
		int32_t SST_find_read_counter;      //for find operation
		int32_t SST_find_write_counter;

		int32_t read_rec_total_length_read;
		int32_t read_rec_total_length_write;
		int32_t SST_read_rec_read_counter;    //for read operation
		int32_t SST_read_rec_write_counter;

		int32_t remove_total_length_read;
		int32_t remove_total_length_write;
		int32_t SST_remove_read_counter;   //for remove operation
		int32_t SST_remove_write_counter;

		int32_t modify_total_length_read;
		int32_t modify_total_length_write;
		int32_t SST_modify_read_counter;   //for modify operation
		int32_t SST_modify_write_counter;

		//int32_t trans_start_total_length_read;
		//int32_t trans_start_total_length_write;
		//int32_t SST_trans_start_read_counter;    //for other operation
		//int32_t SST_trans_start_write_counter;

		//int32_t trans_commit_total_length_read;
		//int32_t trans_commit_total_length_write;
		//int32_t SST_trans_commit_read_counter;    //for other operation
		//int32_t SST_trans_commit_write_counter;

		int32_t other_total_length_read;
		int32_t other_total_length_write;
		int32_t SST_other_read_counter;    //for other operation
		int32_t SST_other_write_counter;

	}DXDB_rwPerOp_t;	

			// Define a strcture for analasis of the number operation in the DB
		typedef struct
		{
			int32_t nInsertOperation;
			int32_t nGetSizeOperation;
			int32_t nRemoveOperation;
			int32_t nReadRecOperation;
			int32_t nModifyOperation;
			/*int32_t nTransStartOperation;
			int32_t nTransCommitOperation;	*/
		}DXDB_operationCounter_t;

		//enumerator for the operation in the DB
		typedef enum
		{
			DXDB_SST_DEBUG_INSERT_OPERATION,
			DXDB_SST_DEBUG_GET_SIZE_OPERATION,
			DXDB_SST_DEBUG_REMOVE_OPERATION,
			DXDB_SST_DEBUG_READ_REC_OPERATION,
			DXDB_SST_DEBUG_MODIFY_OPERATION,
			DXDB_SST_DEBUG_TRANS_START_OPERATION,
			DXDB_SST_DEBUG_TRANS_COMMIT_OPERATION,
			DXDB_SST_DEBUG_OTHER_OPERATION,

			DXDB_SST_OPERATION_FORCE_INT32       = 0x7FFFFFFF /* force enum to 32 bit in all compilers */

		}DXDB_OPERATION;
	/*----------- Extern definition ----------------------------------------------*/
	 #if defined EXTERN
        #error "EXTERN ALREADY DEFINED !!! (dxdb_types.h)"
    #else
        #if defined _DXDB_NO_EXTERN_
            #define EXTERN 
        #else
            #define EXTERN extern
        #endif // _DXDB_NO_EXTERN_
    #endif //EXTERN
		
	EXTERN DXDB_itzamDebugData_t     _dxdbDebugData;         // extern data structure for debug 
	EXTERN DXDB_rwPerOp_t            _dxdbrwPerOp;
	EXTERN DXDB_operationCounter_t   _dxdbOperationCounter;
	EXTERN DXDB_OPERATION            _dxdbOperationState;
	
	#undef EXTERN
	/*----------- Global variable declarations -----------------------------------*/


	/*----------- Global constant definitions ------------------------------------*/

	/*----------- Global function prototypes -------------------------------------*/

	/*init functions*/

	/*!
	\brief 
	Delete an existing DB, so it cannot be used.
	@param mainMemoryId [in] The main memory Id that will be used to store the DB
	@return ITZAM_OKAY				operation completed successfully
	@return ITZAM_FAILED			operation failed
	**/
	itzam_state itzam_btree_debug_delete(DxNvsMemoryId_t  mainMemoryId);


	/*!
	\brief 
	Perform a search without checking integrity of B-tree (expect for pages integrity)
	@param btree_ptr	[in] Pointer to the B-tree 
	@param key_ptr		[in] Pointer to the key 
	@param result_ptr	[in] Pointer to the result
    @param retState_ptr [in/out] Pointer to state, indicate return code
	**/
	void itzam_integrity_debug_search (	itzam_btree		*btree_ptr, 
										const itzam_record_handle	*key_ptr, 
										search_result	*result_ptr,
                                        itzam_state*   retState_ptr);


	/*!
	\brief 
	Free the data received from the debug search
	@param result_ptr	[in] Pointer to the result
	**/
	void itzam_integrity_debug_search_free (search_result	*result_ptr);


	/*!
	\brief 
	Get the offset of the MoM In the file

	@param btree			[in] Pointer to the B-tree 
	@param momOffset_ptr	[out] Pointer to the MoM offset 
	
	@return ITZAM_OKAY				operation completed successfully
	@return ITZAM_FAILED			operation failed
	**/
	itzam_state itzam_debug_integrity_mom_offset_get(itzam_btree btree,
													 DxUint32_t *momOffset_ptr);

	/*!
	\brief 
	Perform modification on a record without updating integrity

	@param btree_ptr	[in] Pointer to the B-tree 
	@param key_ptr		[in] Pointer to the key 

	@return ITZAM_OKAY				operation completed successfully
	@return ITZAM_FAILED			operation failed
	**/

	itzam_state itzam_debug_integrity_record_modify(itzam_btree		*btree_ptr, 
													const void		*key_ptr);

	/*!
	\brief 
	Perform modification on a page without updating integrity

	@param btree_ptr	[in] Pointer to the B-tree 
	@param key_ptr		[in] Pointer to the key 
	@param corruptType	[in] indicates which section in the page to corrupt
	@param corruptIndex [in] indicates the index of record\link to corrupt if needed, 
							 default will be the first (index=0)

	@return ITZAM_OKAY				operation completed successfully
	@return ITZAM_FAILED			operation failed
	**/

	itzam_state itzam_debug_integrity_page_modify(itzam_btree	*btree_ptr, 
												  const void	*key_ptr,
												  SSTITZAMCorruptPageSection_t corruptType,
												  DxUint32_t	corruptIndex);


	/*!
	\brief 
	Perform modification on the btree MoM field by changing the fields 
	on which the MoM is calculated without updating integrity
	
	@param btree_ptr	[in] Pointer to B-tree 
	
	@return	ITZAM_OKAY 	 operation completed successfully
	@return	ITZAM_FAILED operation failed
	
	@notes	The MoM is currently calculated on the root page MAC, the order field 
			in the btree header and the VRPC. This function will allow to change 
			only the order field, since the VRPC cam be changed using VRPC debug 
			function, and the root page MAC can be changed using the 
			itzam_debug_integrity_page_modify().
	**/
	itzam_state itzam_debug_integrity_btree_modify(itzam_btree	*btree_ptr);

	/*!
	\brief 
	Perform statistics updates on read and write operations
	**/

	void          incrementReadDebugData   (unsigned int len);
	void          incrementReadDebugByOp   (unsigned int len);
	void          incrementWriteDebugData  (unsigned int len);
	void          incrementWriteDebugByOp  (unsigned int len);

	#endif 
#endif  /* _DX_SST_ITZAM_DX_ADD_H_ */
