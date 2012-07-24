/*! \file 
**********************************************************************************	
* Title:						Discretix SST VDB header file						 					
*																			
* Filename:						sst_vdb.h															
*																			
* Project, Target, subsystem:	SST 6.0 Virtual DataBase
* 
* Created:						05.03.2007														
*
* Modified:						07.06.2007										
*
* \Author						Einat Ron														
*																			
* \Remarks						Copyright (C) 2007 by Discretix Technologies Ltd.     			
*  								All Rights reserved											
**********************************************************************************/

#ifndef _DX_SST_VDB_H_
    #define _DX_SST_VDB_H_

	#include "sst_vdb_config.h"
	#include "sst_vdb_def.h"
	#include "itzam_btree_dx_add.h"
	
	#include "sst_debug.h"
	#include "sst_general_types.h"
	/*----------- Global defines -------------------------------------------------*/
	/*! \brief SST VDB minimum scratch buffer size**/
	#define SST_VDB_MIN_SCRATCH_BUFFER_SIZE_IN_BYTES  /*128Byte*/	\
    		(SST_VDB_INTERNAL_MIN_SCRATCH_BUFFER_SIZE_IN_BYTES		\
			+ SST_ITZAM_SCRATCH_BUFFER_MIN_SIZE)
    /*----------- Global macro definitions ---------------------------------------*/
	#define SST_VDB_MIN_BLOCK_TO_READ						SST_VCRYS_ENCRYPT_DECRYPT_BLOCK_SIZE_IN_BYTES

	#define SST_VDB_PREFIX_MAX_SIZE_IN_BITS					(63UL)
    /*----------- Global type definitions ----------------------------------------*/

	/*! \brief SST VDB operation types **/
	typedef enum
	{
        SST_VDB_OP_INSERT,
        SST_VDB_OP_MODIFY,
        SST_VDB_OP_MODIFY_NO_LENGTH_CHANGE,
        SST_VDB_OP_DELETE,

        SST_VDB_OP_TYPE_FORCE_INT32= 0x7FFFFFFF /* force enum to 32 bit in all compilers */

    } SSTVDBOpType_t;

/*! \brief SST VDB Return Codes**/
	typedef enum
	{
		SST_VDB_RC_OK									= 0,
		SST_VDB_RC_FAIL									= 1,

		SST_VDB_RC_ERROR_NULL_POINTER					= 2,
		SST_VDB_RC_ERROR_DB								= 3,
		SST_VDB_RC_ERROR_VOS							= 4,
		SST_VDB_RC_ERROR_VCRYS							= 5,
		SST_VDB_RC_ERROR_WRITE_FAIL		 				= 6,
		SST_VDB_RC_ERROR_SIZE_MISMATCH					= 7,
		SST_VDB_RC_ERROR_INSUFFICIENT_SCRATCH_BUFFER	= 8,
		SST_VDB_RC_ERROR_BAD_PARAMTERS					= 9,

		SST_VDB_RC_ERROR_RECORD_HANDLE_DUPLICATED		= 10,
		SST_VDB_RC_ERROR_RECORD_NOT_FOUND				= 11,
		
		SST_VDB_RC_ERROR_TRANSACTION_ID_INCORRECT		= 12,	
		SST_VDB_RC_ERROR_TRANSACTION_STARTED			= 13,
		SST_VDB_RC_ERROR_TRANSACTION_NOT_STARTED		= 14,
		SST_VDB_RC_ERROR_TRANSACTION_TIMED_OUT			= 15,
		
		SST_VDB_RC_ERROR_INTEGRITY_COMPROMISED			= 16,
	
		SST_VDB_RC_ERROR_ITERATOR_BEGIN					= 17,
		SST_VDB_RC_ERROR_ITERATOR_END					= 18,
		SST_VDB_RC_ERROR_ITERATOR_INVALID				= 19,

		SST_VDB_RC_ERROR_DB_IS_OPEN						= 20,
		SST_VDB_RC_ERROR_DB_IS_NOT_OPEN					= 21,
		SST_VDB_RC_ERROR_DB_MAC_STACK					= 22,
		SST_VDB_RC_ERROR_FEATURE_NOT_SUPPORTED			= 23,
        SST_VDB_RC_ERROR_WRONG_MEMORY_ID                = 24,

        SST_VDB_RC_ERROR_NVS_TXN_MAX_SIZE_EXCEEDED      = 25,
        SST_VDB_RC_ERROR_NVS_MAIN_MAX_SIZE_EXCEEDED     = 26,
        SST_VDB_RC_ERROR_NVS_WRITE                      = 27,
        SST_VDB_RC_ERROR_NVS_READ                       = 28,
        SST_VDB_RC_ERROR_NVS_MAX_SIZE_TOO_BIG           = 29,
        SST_VDB_RC_ERROR_NVS_MAX_SIZE_TOO_SMALL         = 30,
        SST_VDB_RC_ERROR_NVS_ACCESS                     = 31,
        SST_VDB_RC_ERROR_READ_PASSED_USED_SIZE          = 32,

        SST_VDB_RETURN_CODES_FORCE_INT32                = 0x7FFFFFFF /* force enum to 32 bit in all compilers */

	}SSTVDBReturnCodes_t;

	/*! \brief defines SST VDB Iterator Prefix **/
	typedef struct 
	{
		DxUint32_t prefixFieldMsb;
		DxUint32_t prefixFieldLsb;
		DxUint32_t prefixSize;		/*In bits legal values: 0 to 63 */
	}SSTVDBIterPrefix_t;

	/*! \brief defines SST VDB Init information **/
	typedef struct 
	{
		DxBool_t powerDownRecover;
		DxBool_t newDatabase;
	}SSTVDBInitInfo_t;

    /*----------- Extern definition ----------------------------------------------*/

    /*----------- Global variable declarations -----------------------------------*/
	
    /*----------- Global constant definitions ------------------------------------*/
	
    /*----------- Global function prototypes -------------------------------------*/
	/* Init functions*/

	/*!
	\brief 
	Initialize the VDB first phase. if a DB exists open it without integrity check
	
	@param mainMemoryId			[in]	The main memory Id that will be used to store the DB
	@param secondaryMemoryID	[in]	The secondary memory Id that will be used to store 
										transaction data
	@param initInfo_ptr			[out]	Information regarding the VDB after initialization 
										have succeeded, including indication if the system 
										recovered from an unexpected power down. 
										may accept DX_NULL, for no indication.
	@param scratchBuffer_ptr	[in]	A pointer to the start of the scratch buffer
	@param scratchBufferSize	[in]	The size of the scratch buffer

	@return SST_VDB_RC_OK					On success
	@return	SST_VDB_RC_FAIL					Operation failed 
	@return	SST_VDB_RC_ERROR_DB_IS_OPEN		if DB is already open
	@return SST_VDB_RC_ERROR_NULL_POINTER	At least one of the arguments is null
	@return SST_VDB_RC_ERROR_VOS			VOS operation failed
	@return SST_VDB_RC_ERROR_DB				DB operation failed
	**/
	SSTVDBReturnCodes_t SST_VDBInitPhase1 (DxNvsMemoryId_t  mainMemoryId, 
										   DxNvsMemoryId_t  secondaryMemoryID,
										   SSTVDBInitInfo_t *initInfo_ptr,
										   DxByte_t			*scratchBuffer_ptr,
										   DxUint32_t		scratchBufferSize);
	

	
	/*!
	\brief 
	Initialize the VDB second phase. create a new DB and store SK, if DB is open
	perform integrity check
	
	@param mainMemoryId			[in] The main memory Id that will be used to store the DB
	@param secondaryMemoryID	[in] The secondary memory Id that will be used to store transaction data

	@return SST_VDB_RC_OK					On success
	@return	SST_VDB_RC_FAIL					Operation failed 
	@return SST_VDB_RC_ERROR_NULL_POINTER	At least one of the arguments is null
	@return SST_VDB_RC_ERROR_VOS			VOS operation failed
	@return SST_VDB_RC_ERROR_DB				DB operation failed
	**/
	SSTVDBReturnCodes_t SST_VDBInitPhase2  (DxNvsMemoryId_t  mainMemoryId, 
											DxNvsMemoryId_t  secondaryMemoryID);
	

	/*!
	\brief 
	Terminate VDB, including closing the DB, and releasing any used resources.
	Integrity is not checked to allow termination of the DB
	
	@return SST_VDB_RC_OK							On success
	@return	SST_VDB_RC_FAIL							Operation failed 
	@return SST_VDB_RC_ERROR_TRANSACTION_STARTED	VDB is in transaction and can't be closed
	@return SST_VDB_RC_ERROR_VOS					VOS operation failed
	@return SST_VDB_RC_ERROR_DB						DB operation failed
	**/
	SSTVDBReturnCodes_t SST_VDBTerminate (void);


    /*!
    \brief 
    Terminate VDB, including closing the DB,releasing any used resources and initialize globals

    @return SST_VDB_RC_OK							On success
    @return	SST_VDB_RC_FAIL							Operation failed     
    @return SST_VDB_RC_ERROR_VOS					VOS operation failed    
    **/
    SSTVDBReturnCodes_t SST_VDBTerminate_dbg (void);
	

	/*----------------------------------------------------------------------------*/
	/* Data functions*/
	/*!
	\brief 
	Insert a new record to the DB. 

	@param transactionID	[in]	The current transactionID
	@param recordProperties	[in]	The type of the record, used to create the record handle
	@param data_ptr			[in]	The record data 
	@param dataLen			[in]	The length of the record data
	@param isEncrypt		[in]	states whether or not the data should be encrypted.
	@param recordHandle_ptr [in\out]User input handle OR 
									Randomly generate handle if indicated SST_VDB_GEN_HANDLE
	@param scratchBuffer_ptr	[in]	A pointer to the start of the scratch buffer
	@param scratchBufferSize	[in]	The size of the scratch buffer
	
	@return SST_VDB_RC_OK								 On success
	@return	SST_VDB_RC_FAIL								 Operation failed
	@return	SST_VDB_RC_ERROR_DB_IS_NOT_OPEN				 if DB is not open
	@return SST_VDB_RC_ERROR_NULL_POINTER				 At least one of the arguments is null
	@return SST_VDB_RC_ERROR_DB							 The DB operation failed
	@return SST_VDB_RC_ERROR_VOS						 The VOS operation failed
	@return SST_VDB_RC_ERROR_VCRYS						 The VCRYS operation failed
	@return SST_VDB_RC_ERROR_WRITE_FAIL					 The write operation failed
	@return SST_VDB_RC_ERROR_INSUFFICIENT_SCRATCH_BUFFER Insufficient workspace given by user 
	@return SST_VDB_RC_ERROR_RECORD_HANDLE_DUPLICATED	 The Handle is duplicated
	@return SST_VDB_RC_ERROR_TRANSACTION_ID_INCORRECT	 The transaction id was invalid
	@return SST_VDB_RC_ERROR_TRANSACTION_NOT_STARTED	 No transaction started
	@return SST_VDB_RC_ERROR_INTEGRITY_COMPROMISED		 The DB integrity has been compromised
	
	@notes: 1.	The record type is 9 bit long.
			2.	Before performing the operation, the transaction status will be checked. 
			3.	recordHandle_ptr can either be inputted, if not it will be randomly generated. 
				If the recordHandle_ptr is SST_VDB_GEN_HANDLE, than the handle will be randomly 
				generated. If not the value in recordHandle_ptr will be used in the DB.
			4.	If this user input handle exists an error return code will indicate so.
			5.	The data will be encrypted if required. 
			6.  Data length have to be larger then 0.
	**/

	SSTVDBReturnCodes_t SST_VDBRecordInsert (	SSTTxnId_t			transactionId,
												SSTHandlePropertiesField_t	recordProperties, 
												const DxByte_t		*data_ptr, 
												DxUint32_t			dataLen,
												DxBool_t			isEncrypt, 
												SSTHandle_t			*recordHandle_ptr,
												DxByte_t			*scratchBuffer_ptr,
												DxUint32_t			scratchBufferSize);



	/*!
	\brief 
	remove a record from the DB

	@param transactionID		[in]	The current transactionID
	@param recordHandle			[in]	Handle to the record
	@param scratchBuffer_ptr	[in]	A pointer to the start of the scratch buffer
	@param scratchBufferSize	[in]	The size of the scratch buffer

	@return SST_VDB_RC_OK								On success
	@return	SST_VDB_RC_FAIL								Operation failed
	@return	SST_VDB_RC_ERROR_DB_IS_NOT_OPEN				if DB is not open
	@return SST_VDB_RC_ERROR_DB							The DB operation failed
	@return SST_VDB_RC_ERROR_WRITE_FAIL					The write operation failed
	@return SST_VDB_RC_ERROR_RECORD_NOT_FOUND			The requested record was not found
	@return SST_VDB_RC_ERROR_TRANSACTION_NOT_STARTED	No transaction started
	@return SST_VDB_RC_ERROR_TRANSACTION_ID_INCORRECT	The transaction id was invalid
	@return SST_VDB_RC_ERROR_INTEGRITY_COMPROMISED		The DB integrity has been compromised

	@notes: Before performing the operation, the transaction status will be checked.
	**/

	SSTVDBReturnCodes_t SST_VDBRecordRemove (SSTTxnId_t	  transactionId,
											 SSTHandle_t  recordHandle,
											 DxByte_t		*scratchBuffer_ptr,
											 DxUint32_t		scratchBufferSize);



	/*!
	\brief 
	Modify a record in the DB WITHOUT changing its length

	@param transactionID	[in]		The current transactionID
	@param scratchBuffer_ptr[in]		A pointer to the start of the scratch buffer
	@param scratchBufferSize[in]		The size of the scratch buffer
	@param recordHandle		[in\out]	Handle of the record
	@param isEncrypt		[in]		states whether or not the data should be encrypted.
	@param offset			[in]		Offset from the beginning of the record to start writing 
										the given data
	@param data_ptr			[in]		The data to be written in the record
	@param dataLen			[in]		The length of the data

	@return SST_VDB_RC_OK								 On success
	@return	SST_VDB_RC_FAIL								 Operation failed
	@return	SST_VDB_RC_ERROR_DB_IS_NOT_OPEN				 if DB is not open
	@return SST_VDB_RC_ERROR_NULL_POINTER				 At least one of the arguments is null
	@return SST_VDB_RC_ERROR_DB							 The DB operation failed
	@return SST_VDB_RC_ERROR_VOS						 The VOS operation failed
	@return SST_VDB_RC_ERROR_VCRYS						 The VCRYS operation failed
	@return SST_VDB_RC_ERROR_WRITE_FAIL					 The write operation failed
	@return SST_VDB_RC_ERROR_INSUFFICIENT_SCRATCH_BUFFER Insufficient workspace given by user 
	@return SST_VDB_RC_ERROR_RECORD_NOT_FOUND			 The requested record was not found
	@return SST_VDB_RC_ERROR_TRANSACTION_ID_INCORRECT	 The transaction id was invalid
	@return SST_VDB_RC_ERROR_TRANSACTION_NOT_STARTED	 No transaction started
	@return SST_VDB_RC_ERROR_INTEGRITY_COMPROMISED		 The DB integrity has been compromised

	@notes: 1.	Using this function assures that the record handle does NOT change
			2.	Before performing the operation, the transaction status will be checked.
			3.	This operation is time consuming.
			4.  Data length have to be larger then 0
	**/

	SSTVDBReturnCodes_t SST_VDBRecordNoLenChangeModify(	SSTTxnId_t			transactionId,
														DxByte_t			*scratchBuffer_ptr,
														DxUint32_t			scratchBufferSize,
														SSTHandle_t			recordHandle, 
														DxBool_t			isEncrypt, 
														DxUint32_t			offset, 
														const DxByte_t		*data_ptr, 
														DxUint32_t			dataLen);


/*!
	\brief 
	Modify a record in the DB WITH length change

	@param transactionID	[in]		The current transactionID
	@param scratchBuffer_ptr[in]		A pointer to the start of the scratch buffer
	@param scratchBufferSize[in]		The size of the scratch buffer
	@param recordHandle_ptr	[in\out]	Handle of the record
	@param isEncrypt		[in]		states whether or not the data should be encrypted.
	@param offset			[in]		Offset from the beginning of the record to start writing 
										the given data
	@param data_ptr			[in]		The data to be written in the record
	@param dataLen			[in]		The length of the data

	@return SST_VDB_RC_OK								 On success
	@return	SST_VDB_RC_FAIL								 Operation failed
	@return	SST_VDB_RC_ERROR_DB_IS_NOT_OPEN				 if DB is not open
	@return SST_VDB_RC_ERROR_NULL_POINTER				 At least one of the arguments is null
	@return SST_VDB_RC_ERROR_DB							 The DB operation failed
	@return SST_VDB_RC_ERROR_VOS						 The VOS operation failed
	@return SST_VDB_RC_ERROR_VCRYS						 The VCRYS operation failed
	@return SST_VDB_RC_ERROR_WRITE_FAIL					 The write operation failed
	@return SST_VDB_RC_ERROR_INSUFFICIENT_SCRATCH_BUFFER Insufficient workspace given by user 
	@return SST_VDB_RC_ERROR_RECORD_NOT_FOUND			 The requested record was not found
	@return SST_VDB_RC_ERROR_TRANSACTION_ID_INCORRECT	 The transaction id was invalid
	@return SST_VDB_RC_ERROR_TRANSACTION_NOT_STARTED	 No transaction started
	@return SST_VDB_RC_ERROR_INTEGRITY_COMPROMISED		 The DB integrity has been compromised

	@notes: 1.	This operation WILL change the record handle
			2.	Before performing the operation, the transaction status will be checked.
			3.	Since modification causes a length change the offset parameter can 
				get any value between 0 to record length (including). 
			4.	This operation is time consuming.
			5.  Data length have to be larger then 0
			
	**/

	SSTVDBReturnCodes_t SST_VDBRecordWithLenChangeModify(	SSTTxnId_t			transactionId,
															DxByte_t			*scratchBuffer_ptr,
															DxUint32_t			scratchBufferSize,
															SSTHandle_t			*recordHandle_ptr, 
															DxBool_t			isEncrypt, 
															DxUint32_t			offset, 
															const DxByte_t		*data_ptr, 
															DxUint32_t			dataLen);




	/*!
	\brief 
	Read an existing record from the DB to the buffer

	@param scratchBuffer_ptr[in]	A pointer to the start of the scratch buffer
	@param scratchBufferSize[in]	The size of the scratch buffer
	@param recordHandle		[in]	Handle to the record
	@param isEncrypt		[in]	states whether or not the data should be encrypted.
	@param buffer_ptr		[out]	The record data
	@param offset			[in]	Offset to read from 
	@param maxDataLen		[in\out]The maximum length of the record data to read. 
									When the given offset and the record length is smaller 
									than this value, only the record length will be written.
									This field will be updated with the actual size of record 
									read in bytes.
	
	@return SST_VDB_RC_OK								 On success
	@return	SST_VDB_RC_FAIL								 Operation failed
	@return	SST_VDB_RC_ERROR_DB_IS_NOT_OPEN				 if DB is not open
	@return SST_VDB_RC_ERROR_NULL_POINTER				 At least one of the arguments is null
	@return SST_VDB_RC_ERROR_DB							 The DB operation failed
	@return SST_VDB_RC_ERROR_VCRYS						 The VCRYS operation failed
	@return SST_VDB_RC_ERROR_INSUFFICIENT_SCRATCH_BUFFER Insufficient workspace given by user 
	@return SST_VDB_RC_ERROR_RECORD_NOT_FOUND			 The requested record was not found
	@return SST_VDB_RC_ERROR_INTEGRITY_COMPROMISED		 The DB integrity has been compromised
	
	@notes: 1.	The user must allocate at least maxDataLen buffer size
			2.	Any legal offset and size are allowed in this function. Using an aligned offset and 
				a size which is a multiple of Block size (SST_VCRYS_ENCRYPT_DECRYPT_BLOCK_SIZE_IN_BYTES) 
				will improve performence of this function.
	**/
	
	SSTVDBReturnCodes_t SST_VDBRecordRead (	DxByte_t			*scratchBuffer_ptr,
											DxUint32_t			scratchBufferSize,
											SSTHandle_t			recordHandle, 
											DxBool_t			isEncrypt, 
											DxByte_t			*buffer_ptr,
											DxUint32_t			offset,
											DxUint32_t			*maxDataLen);

	
	
	/*!
	\brief 
	Get the length of an existing record from the DB 

	@param recordHandle [in]	Handle to the record
	@param recordSize	[out]	The size of the record requested.

	@return SST_VDB_RC_OK							On success
	@return	SST_VDB_RC_FAIL							Operation failed
	@return	SST_VDB_RC_ERROR_DB_IS_NOT_OPEN			if DB is not open
	@return SST_VDB_RC_ERROR_NULL_POINTER			At least one of the arguments is null
	@return SST_VDB_RC_ERROR_DB						The DB operation failed
	@return SST_VDB_RC_ERROR_RECORD_NOT_FOUND		The requested record was not found
	@return SST_VDB_RC_ERROR_INTEGRITY_COMPROMISED	The DB integrity has been compromised
	**/

	SSTVDBReturnCodes_t SST_VDBRecordSizeGet  (	SSTHandle_t  recordHandle, 
												DxUint32_t	*recordSize_ptr);


	/*!
	\brief 
	Insert the SK to the DB 

	@param data		[in]		the data to insert
	@param dataSize	[in\out]	Pointer to the size of the buffer

	@return SST_VDB_RC_OK							On success
	@return	SST_VDB_RC_FAIL							Operation failed
	**/

	SSTVDBReturnCodes_t SST_VDBInternalSKInsert(DxByte_t *data_ptr,
												DxUint32_t dataSize);

	
	/*!
	\brief 
	Get the SK stored in the DB 

	@param data		[in]	the data buffer to read to 
	@param dataSize	[in]	The size of the buffer

	@return SST_VDB_RC_OK							On success
	@return	SST_VDB_RC_FAIL							Operation failed
	@return SST_VDB_RC_ERROR_RECORD_NOT_FOUND		The requested record was not found

	**/

	SSTVDBReturnCodes_t SST_VDBInternalSKRead(	DxByte_t *data,
												DxUint32_t *dataSize);
			
	
	/*----------------------------------------------------------------------------*/
	/*transaction functions*/
	
	/*!
	\brief 
	Start a new transaction in the DB. 

	@param transactionID_ptr	[out]	The current transactionID to start

	@return SST_VDB_RC_OK							On success
	@return	SST_VDB_RC_FAIL							Operation failed
	@return	SST_VDB_RC_ERROR_DB_IS_NOT_OPEN			if DB is not open
	@return SST_VDB_RC_ERROR_DB						The DB operation failed
	@return SST_VDB_RC_ERROR_VOS					The VOS operation failed
	@return SST_VDB_RC_ERROR_WRITE_FAIL				The write operation failed
	@return SST_VDB_RC_ERROR_TRNASACTION_STARTED	There is a transaction open that 
													was not committed
	@return SST_VDB_RC_ERROR_TRANSACTION_TIMED_OUT	The transaction semaphore was timed out
	
	@notes: If a transaction has already started the operation fails.
	**/
	
	SSTVDBReturnCodes_t SST_VDBTransactionStart  (SSTTxnId_t *transactionId_ptr);
	

	/*!
	\brief 
	Commit the transaction that has been performed.

	@param transactionID		[in]	The current transactionID to commit
	@param isReflashSensitive	[in]	Flag indicating if the transaction should be reflash sensitive
	@param scratchBuffer_ptr	[in]	A pointer to the start of the scratch buffer
	@param scratchBufferSize	[in]	The size of the scratch buffer

	@return SST_VDB_RC_OK								On success
	@return	SST_VDB_RC_FAIL								Operation failed
	@return	SST_VDB_RC_ERROR_DB_IS_NOT_OPEN				if DB is not open
	@return SST_VDB_RC_ERROR_DB							The DB operation failed
	@return SST_VDB_RC_ERROR_VOS						The VOS operation failed
	@return SST_VDB_RC_ERROR_WRITE_FAIL					The write operation failed
	@return SST_VDB_RC_ERROR_TRANSACTION_NOT_STARTED	There is no existing transaction 
														to commit
	@return SST_VDB_RC_ERROR_TRANSACTION_ID_INCORRECT	The transaction id was invalid

	@notes: If a transaction has not started the operation fails
	**/
	SSTVDBReturnCodes_t SST_VDBTransactionCommit (	SSTTxnId_t	transactionId,
													DxBool_t	isReflashSensitive,
													DxByte_t	*scratchBuffer_ptr,
													DxUint32_t	scratchBufferSize);

	/*!
	\brief 
	Rollback the transaction that has been performed.

	@param transactionID		[in]	The current transactionID to commit
	@param scratchBuffer_ptr	[in]	A pointer to the start of the scratch buffer
	@param scratchBufferSize	[in]	The size of the scratch buffer

	@return SST_VDB_RC_OK								On success
	@return	SST_VDB_RC_FAIL								Operation failed
	@return	SST_VDB_RC_ERROR_DB_IS_NOT_OPEN				if DB is not open
	@return SST_VDB_RC_ERROR_DB							The DB operation failed
	@return SST_VDB_RC_ERROR_VOS						The VOS operation failed
	@return SST_VDB_RC_ERROR_WRITE_FAIL					The write operation failed
	@return SST_VDB_RC_ERROR_TRANSACTION_NOT_STARTED	There is no existing transaction 
														to rollback
	@return SST_VDB_RC_ERROR_TRANSACTION_ID_INCORRECT	The transaction id was invalid

	@notes: If a transaction has not started the operation fails
	**/
	SSTVDBReturnCodes_t SST_VDBTransactionRollBack (SSTTxnId_t	transactionId,
													DxByte_t	*scratchBuffer_ptr,
													DxUint32_t	scratchBufferSize);

	/*!
	\brief 
	Returns if the DB is in transaction (DX_TRUE) or not (DX_FALSE).
	**/
	DxBool_t			SST_VDBTransactionStatus(void);

	/*----------------------------------------------------------------------------*/
	/*iterator functions*/
	/*!
	\brief 
	create an iterator that can be used to retrieve all handles from a given type

	@param iterPrefix		[in]	prefix of the record and its size
	@param iterCookie_ptr	[out]	Handle the user can identify this iterator.

	@return SST_VDB_RC_OK							On success
	@return	SST_VDB_RC_FAIL							Operation failed
	@return	SST_VDB_RC_ERROR_NULL_POINTER			At least one of the arguments is null
	@return SST_VDB_RC_ERROR_BAD_PARAMETERS			The parameters given are not valid
	@return	SST_VDB_RC_ERROR_DB_IS_NOT_OPEN			if DB is not open
	@return SST_VDB_RC_ERROR_INTEGRITY_COMPROMISED	The DB integrity has been compromised
	**/
                                                    
	SSTVDBReturnCodes_t SST_VDBIteratorCreate  (SSTVDBIterPrefix_t iterPrefix, 
												SSTIteratorCookie_t	*iterCookie_ptr);



	/*!
	\brief 
	terminate this iteration session

	@param iterCookie_ptr  [in] Handle the user can identify this iterator.

	@return SST_VDB_RC_OK							On success
	@return	SST_VDB_RC_FAIL							Operation failed
	@return	SST_VDB_RC_ERROR_NULL_POINTER			At least one of the arguments is null
	@return	SST_VDB_RC_ERROR_DB_IS_NOT_OPEN			if DB is not open
	@return SST_VDB_RC_ERROR_INTEGRITY_COMPROMISED	The DB integrity has been compromised
	**/
	SSTVDBReturnCodes_t SST_VDBIteratorTerminate(SSTIteratorCookie_t *iterCookie_ptr);
	

	/*!
	\brief 
	Retrieve all relevant handles (up to a user defined maximal amount).

	@param iterCookie_ptr		[in]		Handle the user can identify this iterator.
	@param nHandlesRead_ptr		[in\out]	The maximal numbers of handles to read, 
											and The number of handles that actually read.
											DX_NULL indicates only ONE handle should retrieved
											Note that in this case the number of handles left 
											will not be updated
	@param recordHandleBuff_ptr [out]		A pointer to the buffer where the returned 
											handle will be stored.
	@param nHandlesLeft_ptr		[out]		A pointer to a location where the number of 
											the handles that are left will be stored.
											DX_NULL indicates that this value will not be 
											calculated and returned
											

	@return SST_VDB_RC_OK							On success
	@return	SST_VDB_RC_FAIL							Operation failed
	@return	SST_VDB_RC_ERROR_NULL_POINTER			At least one of the arguments is null
	@return	SST_VDB_RC_ERROR_DB_IS_NOT_OPEN			if DB is not open
	@return SST_VDB_RC_ITERATOR_END					The iterator has reached the last item
	@return SST_VDB_RC_ERROR_INTEGRITY_COMPROMISED	The DB integrity has been compromised
	**/
	SSTVDBReturnCodes_t SST_VDBIteratorGetAllHandles(SSTIteratorCookie_t	*iterCookie_ptr,
													DxUint32_t				*nHandlesRead_ptr,
													SSTHandle_t		 	*recordHandleBuff_ptr,
													DxUint32_t				*nHandlesLeft_ptr);

	/*!
	\brief 
	Increment the iterator to point to the next item and retrieve the next handle.
	iteration will update the iterCookie for future use
	
	@param iterCookie_ptr	[in\out]	Handle the user can identify this iterator.
	@param recordHandle_ptr [out]		The record handle of the next record 

	@return SST_VDB_RC_OK							On success
	@return	SST_VDB_RC_FAIL							Operation failed
	@return SST_VDB_RC_ITERATOR_END					The iterator has reached the last item
	@return SST_VDB_RC_ERROR_INTEGRITY_COMPROMISED	The DB integrity has been compromised
	**/
	#define SST_VDBIteratorIncAndGet(iterCookie_ptr,recordHandle_ptr)	\
			SST_VDBIteratorGetAllHandles(iterCookie_ptr,DX_NULL,recordHandle_ptr,DX_NULL)
		


	/*!
	\brief 
	decrement the iterator to point to the previous item and retrieve the next handle.
	iteration will update the iterCookie for future use

	@param iterCookie_ptr	[in\out]	Handle the user can identify this iterator.
	@param recordHandle_ptr [out]		The record handle of the previous record 

	@return SST_VDB_RC_OK							On success
	@return	SST_VDB_RC_FAIL							Operation failed
	@return	SST_VDB_RC_ERROR_NULL_POINTER			At least one of the arguments is null
	@return	SST_VDB_RC_ERROR_DB_IS_NOT_OPEN			if DB is not open
	@return SST_VDB_RC_ITERATOR_BEGIN				The iterator has reached the last item
	@return SST_VDB_RC_ERROR_INTEGRITY_COMPROMISED	The DB integrity has been compromised
	**/

	SSTVDBReturnCodes_t SST_VDBIteratorDecAndGet(SSTIteratorCookie_t	*iterCookie_ptr,
												 SSTHandle_t			*recordHandle_ptr);



	/*!
	\brief 
	Retrieve the handle of the item on which the iterator currently points

	@param iterCookie	[in]	Handle the user can identify this iterator.
	@param recordHandle_ptr [out]	The record handle of the record currently pointed 
								by the iterator

	@return SST_VDB_RC_OK							On success
	@return	SST_VDB_RC_FAIL							Operation failed
	@return	SST_VDB_RC_ERROR_NULL_POINTER			At least one of the arguments is null
	@return	SST_VDB_RC_ERROR_DB_IS_NOT_OPEN			if DB is not open
	@return SST_VDB_RC_ERROR_INTEGRITY_COMPROMISED	The DB integrity has been compromised
	**/
	SSTVDBReturnCodes_t SST_VDBIteratorGetCurrent(	SSTIteratorCookie_t *iterCookie_ptr,
													SSTHandle_t			*recordHandle_ptr);
	


	
	/*----------------------------------------------------------------------------*/	
	
	/*!
	    \brief 
	    Perform a full check of the DB to insure its integrity
	    @param checkRecords		[in]	flag indicating if the integrity of the records 
									    should be checked too.
	    @param scratchBuffer_ptr	[in]	A pointer to the start of the scratch buffer
	    @param scratchBufferSize	[in]	The size of the scratch buffer
    	
	    @return SST_VDB_RC_OK							On success
	    @return	SST_VDB_RC_FAIL							Operation failed
	    @return	SST_VDB_RC_ERROR_DB_IS_NOT_OPEN			if DB is not open
	    @return SST_VDB_RC_ERROR_VCRYS					The VCRYS operation failed
	    @return SST_VDB_RC_ERROR_INTEGRITY_COMPROMISED	The DB integrity has been compromised
    		
	    @notes: 1.	The DB must be open in order to perform this check.
			    2.	This is a time consuming method, especially if the checkRecords flag 
				    indicates that the records should be checked

	**/
	SSTVDBReturnCodes_t SST_VDBIntegrityCheck (DxBool_t checkRecords,
											   DxByte_t	*scratchBuffer_ptr,
											   DxUint32_t scratchBufferSize);




    /* SST_VDBUtilNVSMaxSizeSet */
    /*!
        \brief 
        check for sufficient space in the NVS for certain operation.

        @param recordSizeInbytes     [in]  the size of the record at hand.
        @param dataOpType	         [in]  the type of the data operation.
        @param isSpaceSufficient_ptr [out] a pointer to hold the result.
    **/
    SSTVDBReturnCodes_t SST_VDBUtilNVSSufficientSpaceCheck(DxUint32_t     recordSizeInbytes,
                                                           SSTVDBOpType_t dataOpType,
                                                           DxBool_t      *isMainSpaceSufficient_ptr,
                                                           DxBool_t      *isTxnSpaceSufficient_ptr);

#endif  /* _DX_SST_VDB_H_ */
