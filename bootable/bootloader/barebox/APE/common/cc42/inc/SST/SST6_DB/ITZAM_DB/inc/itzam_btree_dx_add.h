/*
**********************************************************************************	
* Title:						Discretix SST DB Itzam Additions Header file				 					
*																			
* Filename:						itzam_btree_dx_add.h			
*																			
* Project, Target, subsystem:	SST 6.0 Itzam DB
* 
* Created:						05.06.2007														
*
* Modified:						07.06.2007										
*
* \Author						Einat Ron														
*																			
* \Remarks						Copyright (C) 2006 by Discretix Technologies Ltd.     			
*  								All Rights reserved											
**********************************************************************************/

#ifndef _DX_SST_ITZAM_BTREE_DX_ADD_H_
    #define _DX_SST_ITZAM_BTREE_DX_ADD_H_

	#include "DX_VOS_BaseTypes.h"
	#include "itzam.h"
	#include "sst_vcrys.h"

/*----------- Global type definitions ----------------------------------------*/

/*----------- Extern definition ----------------------------------------------*/
#define SST_ITZAM_SCRATCH_BUFFER_MIN_SIZE		(SST_VCRYS_MAC_BLOCK_SIZE_IN_BYTES + \
                                                SST_ITZAM_BTREE_INTERNAL_OPERATION_MIN_SIZE_IN_BYTES)

#define SST_ITZAM_BTREE_INTERNAL_OPERATION_MIN_SIZE_IN_BYTES	(80UL)
/*----------- Global variable declarations -----------------------------------*/

/*----------- Global constant definitions ------------------------------------*/

/*----------- Global function prototypes -------------------------------------*/
/************************************************************************/
/*              Init functions                                          */
/************************************************************************/


/*!
\brief 
Initialize the Itzam DB 
@param rootMac_ptr	[in] the MoM of the DB stored in the B-tree header
**/
void itzam_integrity_btree_init (itzam_btree *btree_ptr);



/*API function additions*/


/*!
\brief 
Modifying a record in the DB, without changing the length of the record

@param btree_ptr			[in]	reference to the B-tree
@param key_ptr				[in]	the key of the record modified
@param offset				[in]	offset in the record to write the new data
@param data_ptr				[in]	data to write in the record
@param dataLen				[in]	length of the new data 
@param ivCounter			[in]	Buffer of the iv counter of the record
@param scratchBuffer_ptr	[in]	A pointer to the start of the scratch buffer
@param scratchBufferSize	[in]	The size of the scratch buffer

@return ITZAM_OKAY				operation completed successfully
@return ITZAM_FAILED			operation failed
@return ITZAM_INTEGRITY_FAILED	integrity of the DB was compromised
@return ITZAM_NOT_FOUND			the handle was not found in the DB
@return ITZAM_DATA_WRITE_FAILED the write operation in the DB failed
**/
itzam_state itzam_btree_modify  ( itzam_btree  *btree_ptr, 
									const void   *key_ptr,
									itzam_ref     offset, 
									const void   *data_ptr, 
									DxUint32_t    dataLen,
									SSTVCRYSIvCounter_t ivCounter,
									DxByte_t	 *scratchBuffer_ptr,
									DxUint32_t	  scratchBufferSize);



/*!
\brief 
Retrieving the size of a record stored in the DB

@param btree_ptr	[in]	reference to the B-tree
@param key_ptr		[in]	the key of the record modified
@param result		[out]	the size result of the record 
							(zero if record not found)

@return ITZAM_OKAY				operation completed successfully
@return ITZAM_FAILED			operation failed
@return ITZAM_INTEGRITY_FAILED	integrity of the DB was compromised
@return ITZAM_NOT_FOUND			the handle was not found in the DB
**/
itzam_state itzam_btree_get_size_rec(itzam_btree *btree_ptr, 
									const void  *key_ptr, 
									DxUint32_t	*result);


/*!
\brief 
Read a record from a given offset, include integrity check of 
the record data

@param btree_ptr			[in]	Reference to the B-tree
@param key_ptr				[in]	Pointer to the key of the record modified
@param recordBuffer_ptr		[out]	Pointer to the buffer to write the data 
@param offset				[in]	The offset from which to read (note 1)
@param max_rec_len_ptr		[in\out]Pointer to the size of the data to read (note 2)
@param scratchBuffer_ptr	[in]	Pointer to the scratch buffer used to check integrity
@param scratchBufferSize	[in]	Size of the scratch buffer
@param prevBlock_ptr		[out]	Pointer to buffer to write to iv counter of the data
@param RecordSize			[out]	Size of the record

@return ITZAM_OKAY				operation completed successfully
@return ITZAM_FAILED			operation failed
@return ITZAM_INTEGRITY_FAILED	integrity of the DB was compromised
@return ITZAM_NOT_FOUND			the handle was not found in the DB

Notes:	1. The offset given must be aligned with encrypt block size
		2. Size of the data read must be a multiple of the encrypt block size
		3. IV counter parameter is the data in the block before the data requested.
		   if offset is zero, then the DX_NULL will be returned


**/
itzam_state itzam_integrity_read_rec(   itzam_btree        *btree_ptr,
                                        const void         *key_ptr,
                                        DxByte_t           *recordBuffer_ptr,
                                        DxUint32_t          offset,
                                        DxUint32_t         *max_rec_len_ptr,
                                        DxByte_t           *scratchBuffer_ptr,
                                        DxUint32_t          scratchBufferSize,
                                        SSTVCRYSIvCounter_t *prevBlock_ptr,
										DxUint32_t			*RecordSize );



/*!
\brief 
Inserting a record to the DB. 

@param itzam_btree	*btree_ptr	[in] reference to the B-tree
@param const void	*key_ptr	[in] the key of the record modified
@param DxUint32_t	key_len		[in] the length of the given key
@param DxByte_t		record		[in] pointer to the record 
@param DxUint32_t	rec_len		[in] the length of the record
@param SSTVCRYSIvCounter_t	ivCounter	[in] IV of the record
@param DxByte_t		*scratchBuffer_ptr	[in] Pointer to the scratch buffer used to check integrity
@param DxUint32_t	scratchBufferSize	[in] Size of the scratch buffer

@return ITZAM_OKAY				operation completed successfully
@return ITZAM_FAILED			operation failed
@return ITZAM_INTEGRITY_FAILED	integrity of the DB was compromised
@return ITZAM_DUPLICATE			the handle given already exists in the DB
@return ITZAM_DATA_WRITE_FAILED the write operation in the DB failed
**/
#define itzam_btree_insert_rec_data(btree,key,key_len,record,rec_len,					\
									ivCounter,scratchBuffer_ptr,scratchBufferSize)		\
		itzam_btree_insert_rec(btree,key,key_len,record,(itzam_int)rec_len,0,			\
								ivCounter,scratchBuffer_ptr,scratchBufferSize)

/*!
\brief 
Inserting an empty record to the DB. This function will allocate the 
size needed in the DB, and will insert zero all data. Note that if the 
zero padding will be part of the record, if it’s not modified.

@param itzam_btree	*btree_ptr	[in] reference to the B-tree
@param const void	*key_ptr	[in] the key of the record modified
@param DxUint32_t	key_len		[in] the length of the given key
@param DxUint32_t	dataLen		[in] the length of the empty record
@param SSTVCRYSIvCounter_t	ivCounter	[in] IV of the record
@param DxByte_t		*scratchBuffer_ptr	[in] Pointer to the scratch buffer used to check integrity
@param DxUint32_t	scratchBufferSize	[in] Size of the scratch buffer

@return ITZAM_OKAY				operation completed successfully
@return ITZAM_FAILED			operation failed
@return ITZAM_INTEGRITY_FAILED	integrity of the DB was compromised
@return ITZAM_DUPLICATE			the handle given already exists in the DB
@return ITZAM_DATA_WRITE_FAILED the write operation in the DB failed
**/

#define itzam_btree_insert_rec_empty(btree_ptr,key_ptr,key_len,dataLen,ivCounter,		\
									scratchBuffer_ptr,scratchBufferSize)				\
		itzam_btree_insert_rec(btree_ptr,key_ptr,key_len,&dataLen,(itzam_int)dataLen,	\
							   ITZAM_RECORD_EMPTY,ivCounter,scratchBuffer_ptr,scratchBufferSize)


/*!
\brief 
store the SST extra data in the itzam DB, 
if data already exists it will be written on

@param btree_ptr	[in]	reference to the B-tree
@param data			[in]	data to store
@param dataSize		[in]	size of data

@return ITZAM_OKAY				operation completed successfully
@return ITZAM_FAILED			operation failed
**/

itzam_state itzam_btree_insert_db_header_data(itzam_btree	*btree_ptr,
												DxByte_t	*data_ptr, 
												DxUint32_t	dataSize);


/*!
\brief 
store the SST extra data in the itzam DB

@param btree_ptr	[in]		reference to the B-tree
@param data			[in]		buffer to read data into
@param dataSize		[in\out]	size to read \ actual size read

@return ITZAM_OKAY				operation completed successfully
@return ITZAM_FAILED			operation failed
**/

itzam_state itzam_btree_read_db_header_data(itzam_btree	*btree_ptr,
											DxByte_t	*data_ptr, 
											DxUint32_t	*dataSize);



/*!
\brief 
performing the integrity check on the MoM of the DB

@param btree_ptr	[in]	reference to the B-tree

@return ITZAM_OKAY				operation completed successfully
@return ITZAM_FAILED			operation failed
@return ITZAM_INTEGRITY_FAILED	integrity of the DB was compromised

@notes: On DB Open operation, a first integrity check will be performed on MoM, 
to assure that the DB was not compromised. This check will be performed twice if 
necessary with both current VRPC value and the next value (if first check does 
not succeed).
**/
itzam_state itzam_integrity_mom_check (itzam_btree *btree_ptr);



/*!
\brief 
performing the integrity check on all the data stored in the DB 
(records and structure)

@param btree_ptr	[in]	reference to the B-tree
@param checkRecords [in]	flag indicating if on top of checking the 
							DB structure we are performing the MAC 
							recalculation on the records stored in the DB.
@param scratchBuffer_ptr	[in]	A pointer to the start of the scratch buffer
@param scratchBufferSize	[in]	The size of the scratch buffer
									MUST be at least SST_ITZAM_SCRATCH_BUFFER_MIN_SIZE

@return ITZAM_OKAY				operation completed successfully
@return ITZAM_FAILED			operation failed
@return ITZAM_INTEGRITY_FAILED	integrity of the DB was compromised
**/
itzam_state itzam_integrity_debug_check_db (itzam_btree *btree_ptr, 
											DxBool_t     checkRecords,
											DxByte_t	*scratchBuff_ptr, 
											DxUint32_t	 scratchBuffSize);




#endif  /* _DX_SST_ITZAM_BTREE_DX_ADD_H_ */
