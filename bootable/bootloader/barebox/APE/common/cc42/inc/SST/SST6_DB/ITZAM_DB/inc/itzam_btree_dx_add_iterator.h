/*
**********************************************************************************	
* Title:						Discretix SST DB Itzam Iterator Additions Header file 				 					
*																			
* Filename:						itzam_btree_dx_add_iterator.h			
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

#ifndef _DX_SST_ITZAM_BTREE_DX_ADD_ITERATOR_H_
    #define _DX_SST_ITZAM_BTREE_DX_ADD_ITERATOR_H_

#include "DX_VOS_BaseTypes.h"
#include "itzam.h"

/*----------- Global defines -------------------------------------------------*/

/*----------- Global macro definitions ---------------------------------------*/

/*----------- Global type definitions ----------------------------------------*/
/*! \brief type definition for parameters to pass to the sequence iterator 
selector function**/
typedef struct  
{
	DxUint32_t fieldMsb;
	DxUint32_t fieldLsb;
	DxUint32_t size;
}itzam_seq_iterator_func_param;

/*! 
\brief 
selects handle to be included in a iteration subset
@param handle			[in]	The handle to compare
@param iterFuncParam	[in]	The Parameters used to conclude if handle is
part of the iterator
@return DX_TRUE    include key
@return DX_FALSE   don't include this key
**/
typedef DxBool_t itzam_seq_iterator_selector(itzam_record_handle	handle,			
											 itzam_seq_iterator_func_param iterFuncParam);


/*! \brief define the direction in which the iteration is performed**/
typedef enum
{
	SST_ITZAM_SEQ_ITERATOR_MOVE_NEXT = 0,
	SST_ITZAM_SEQ_ITERATOR_MOVE_PREV = 1,

	ITZAM_SEQ_ITERATOR_DIRECTION_FORCE_INT32= 0x7FFFFFFF /* force enum to 32 bit in all compilers */

}itzam_seq_iterator_direction;

	
/*----------- Extern definition ----------------------------------------------*/

/*----------- Global variable declarations -----------------------------------*/

/*----------- Global constant definitions ------------------------------------*/

/*----------- Global function prototypes -------------------------------------*/
/*!
\brief 
create an iterator, the iterCookie will allow to access the handles relevant 
to the iteration 
@param btree_ptr		[in]	reference to the B-tree
@param seqIterHandle_ptr[in\out]a handle to the first legal handle in the DB
will be updated to the relevant handle if required
@param iterFunc			[in]	according to this function the handle will be
as part of the iteration 
@param iterFuncParam	[in]	user information to be passed to the iterFunc
in order to determine relevant to the iteration
@return ITZAM_OKAY					operation completed successfully
@return ITZAM_FAILED				operation failed

**/

itzam_state itzam_seq_iterator_create (itzam_btree					*btree_ptr, 
									   itzam_record_handle			*seqIterHandle_ptr,
									   itzam_seq_iterator_selector	*iterFunc,
									   itzam_seq_iterator_func_param iterFuncParam);

/*!
\brief 
terminate the iterator

@param btree_ptr		[in]	reference to the B-tree
@param seqIterHandle[in\out]a handle to the first legal handle in the DB


@return ITZAM_OKAY					Operation completed successfully
@return ITZAM_FAILED				Operation failed
**/
itzam_state itzam_seq_iterator_terminate(itzam_btree		*btree_ptr, 
										 itzam_record_handle	*seqIterHandle_ptr);


/*!
\brief 
get the next number of handles, using the iterator function to select it.
Note: use MACROs for sequence iterator operation

@param btree_ptr		[in]	reference to the B-tree
@param seqIterHandle_ptr[in\out]a handle to the first legal handle in the DB
								will be updated to the relevant handle if required
@param iterFunc			[in]	according to this function the handle will be
								as part of the iteration 
@param iterFuncParam	[in]	user information to be passed to the iterFunc
								in order to determine relevant to the iteration
@param iterDirection	[in]	flag indicating in which direction should the iteration go
@param resultBuff_ptr	[out]	A buffer in which the result of the iteration will
								be stored. DX_NULL indicates to return the handle in
								seqIterHandle_ptr. 
@param nHandleRead_ptr	[in/out]the required number of handles. this field will be 
								updated with the actual number of handles entered. 
								DX_NULL indicates return only the ONE handle
@param nHandlesLeft_ptr	[out]	the number of handle in the iteration left. DX_NULL 
								indicate no need to return the number of handles left
@param firstUse			[in]	flag indicating if this is the first time the iterator is used

@return ITZAM_OKAY					operation completed successfully
@return ITZAM_FAILED				operation failed
@return ITZAM_DX_ERROR_INTEGRITY	integrity of the DB was compromised
@return ITZAM_DX_ITERATOR_AT_END	the required handle sequence iterator is at
									the end of the DB. the resultBuffSize_ptr will 
									indicate the number of handles read
@return ITZAM_DX_ITERATOR_AT_BEGIN	the required handle sequence iterator is at
									the start of the DB. the resultBuffSize_ptr will 
									indicate the number of handles read
**/
itzam_state itzam_seq_iterator_move_and_get (itzam_btree					*btree_ptr, 
												itzam_record_handle			*seqIterHandle_ptr,
												itzam_seq_iterator_selector	*iterFunc,
												itzam_seq_iterator_func_param iterFuncParam,
												itzam_seq_iterator_direction iterDirection,					
												DxByte_t					*resultBuff_ptr,
												DxUint32_t					*nHandleRead_ptr,
												DxUint32_t					*nHandlesLeft_ptr,
												DxBool_t					firstUse);
/*! \brief get the next handle **/
#define itzam_seq_iterator_get_next(btree_ptr,seqIterHandle_ptr,iterFunc,iterFuncParam,firstUse) \
		itzam_seq_iterator_move_and_get(btree_ptr,seqIterHandle_ptr,iterFunc, \
										iterFuncParam,SST_ITZAM_SEQ_ITERATOR_MOVE_NEXT, \
										DX_NULL,DX_NULL,DX_NULL,firstUse)

/*! \brief get the previous handle **/
#define itzam_seq_iterator_get_prev(btree_ptr,seqIterHandle_ptr,iterFunc,iterFuncParam) \
		itzam_seq_iterator_move_and_get(btree_ptr,seqIterHandle_ptr,iterFunc, \
										iterFuncParam,SST_ITZAM_SEQ_ITERATOR_MOVE_PREV, \
										DX_NULL,DX_NULL,DX_NULL,DX_FALSE)

/*! \brief get the next ALL handles **/
#define itzam_seq_iterator_get_next_all(btree_ptr,seqIterHandle_ptr,iterFunc,iterFuncParam,\
										resultBuff_ptr,nHandleRead_ptr,nHandlesLeft_ptr,firstUse) \
		itzam_seq_iterator_move_and_get(btree_ptr,seqIterHandle_ptr,iterFunc, \
										iterFuncParam,SST_ITZAM_SEQ_ITERATOR_MOVE_NEXT, \
										resultBuff_ptr,nHandleRead_ptr,nHandlesLeft_ptr,firstUse)

/*! \brief get the previous ALL handles **/
#define itzam_seq_iterator_get_prev_all(btree_ptr,seqIterHandle_ptr,iterFunc,iterFuncParam,\
										resultBuff_ptr,nHandleRead_ptr,nHandlesLeft_ptr) \
		itzam_seq_iterator_move_and_get(btree_ptr,seqIterHandle_ptr,iterFunc, \
										iterFuncParam,SST_ITZAM_SEQ_ITERATOR_MOVE_PREV, \
										resultBuff_ptr,nHandleRead_ptr,nHandlesLeft_ptr,DX_FALSE)




/*!
\brief 
find and return the first handle, using the iterator function to select it.

@param btree_ptr		[in]	reference to the B-tree
@param seqIterHandle_ptr[in\out]a handle to the first legal handle in the DB
								will be updated to the relevant handle if required
@param iterFunc			[in]	according to this function the handle will be
								as part of the iteration 
@param iterFuncParam	[in]	user information to be passed to the iterFunc
								in order to determine relevant to the iteration
@param searchResult_ptr	[out]	return information on the page the handle is 
								associated with

@return ITZAM_OKAY					Operation completed successfully
@return ITZAM_FAILED				Operation failed
@return ITZAM_DX_ERROR_INTEGRITY	Integrity of the DB was compromised
@return ITZAM_DX_ITERATOR_AT_END	The requried handle sequence iterator is at
									the end of the DB
**/
itzam_state itzam_seq_iterator_initial_search (	itzam_btree					*btree_ptr, 
												itzam_record_handle			*seqIterHandle_ptr,
												itzam_seq_iterator_selector	*iterFunc,
												itzam_seq_iterator_func_param iterFuncParam,
												search_result				*searchResult_ptr,
												itzam_seq_iterator_direction iterDirection);


/*!
\brief 
find and return the current handle, using the iterator function to select it.
**/
#define itzam_seq_iterator_get_current(btree_ptr,seqIterHandle_ptr,iterFunc,iterFuncParam) \
		itzam_seq_iterator_initial_search(btree_ptr,seqIterHandle_ptr,iterFunc,				\
										  iterFuncParam,DX_NULL,SST_ITZAM_SEQ_ITERATOR_MOVE_NEXT)



#endif  /* _DX_SST_ITZAM_BTREE_DX_ADD_ITERATOR_H_ */
