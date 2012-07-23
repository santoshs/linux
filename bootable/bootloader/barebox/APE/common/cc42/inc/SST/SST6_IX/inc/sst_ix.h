/*! \file 
**********************************************************************************
* Title:                           Discretix SST IX header file
*
* Filename:                        sst_ix.h
*
* Project, Target, subsystem:      SST 6.0 Index Lookup Utility
* 
* Created:                         17.04.2007
*
* Modified:						   07.06.2007
*
* \Author                          Ira Boguslvasky
*
* \Remarks                         Copyright (C) 2007 by Discretix Technologies Ltd.
*                                  All Rights reserved
**********************************************************************************/

#ifndef _DX_SST_IX_H_
    #define _DX_SST_IX_H_

    #include "sst_ix_types.h"
    #include "sst_ix_hash.h"

    /*----------- Global defines -------------------------------------------------*/
    /*! \brief SST AA minimum workspace size in bytes **/
	#define SST_IX_MIN_WORKSPACE_SIZE_IN_BYTES					 /*148 Bytes*/	\
			((SST_ALIGN_TO_WORD(sizeof(SSTIXIndexMetaDataObject_t))) +	/*84*/	\
			(SST_ALIGN_TO_WORD(sizeof(SSTIXIndexObject_t))))			/*64*/
    
    /*----------- Global type definitions ----------------------------------------*/

    /*! \brief SST Index Lookup Utility Return Codes**/
    typedef enum
    {
        SST_IX_RC_OK                                        = 0,
        SST_IX_RC_FAIL                                      = 1,
        SST_IX_RC_ERROR_NULL_POINTER                        = 2,
        SST_IX_RC_ERROR_DB                                  = 3,
        SST_IX_RC_ERROR_RECORD_NOT_FOUND                    = 4,
        SST_IX_RC_ERROR_MAX_COLLISIONS                      = 5,
        SST_IX_RC_ERROR_NO_MORE_SPACE                       = 6,
        SST_IX_RC_ERROR_INVALID_PARAM                       = 7,
        SST_IX_RC_ERROR_NOT_ENOUGH_MEMORY                   = 8,
        SST_IX_RC_ERROR_IMD_WITHOUT_ANY_INDEX               = 9,
        SST_IX_RC_ERROR_IND_WITHOUT_ANY_STRING              = 10,
        SST_IX_RC_ERROR_VDB                                 = 11,
        SST_IX_RC_ERROR_HANDLE_IN_USE                       = 12,
        SST_IX_RC_ERROR_DUPLICATE_STRING                    = 13,
        SST_IX_RC_ERROR_ITERATOR_END                        = 14,
        SST_IX_RC_ERROR_LOOKUP_HANDLE_NOT_BOUND             = 15,
        SST_IX_RC_ERROR_TXN_INVALID                         = 16,
        SST_IX_RC_ERROR_FATAL                               = 17,
		SST_IX_RC_ERROR_TXN_NOT_ENDED						= 18,
		SST_IX_RC_ERROR_INTEGRITY_COMPROMISED				= 19,
        SST_IX_RC_ERROR_NVS_MAIN_MAX_SIZE_EXCEEDED          = 20,
        SST_IX_RC_ERROR_NVS_TXN_MAX_SIZE_EXCEEDED           = 21,
        SST_IX_RC_ERROR_NVS_WRITE                           = 22,
        SST_IX_RC_ERROR_NVS_READ                            = 23,
        SST_IX_RC_ERROR_NVS_ACCESS                          = 24,

        SST_IX_RETURN_CODES_FORCE_INT32                     = 0x7FFFFFFF /* force enum to 32 bit in all compilers */

    }SSTIXReturnCodes_t;
    
    /*----------- Global function prototypes -------------------------------------*/
    /*!
    \brief 
    Bind a string (array of characters) to a handle of a data object in the SST.

    @param transactionId                [in]    Current transaction id
    @param recordHandle                 [in]    Handle to the record 
    @param string_ptr                   [in]    String which need bind 
    @param stringLengthInBytes          [in]    Length of the string
    @param scratchBuffer_ptr            [in]    Pointer to the start of the scratch buffer
    @param scratchBufferSizeInBytes     [in]    Size of the scratch buffer (Cannot be less then SST_IX_MIN_WORKSPACE_SIZE_IN_BYTES)

    @return SST_IX_RC_OK                                     The bind operation was successful
    @return SST_IX_RC_FAIL                                   The bind operation failed
    @return SST_IX_RC_ERROR_VDB                              DB operation failed
    @return SST_IX_RC_ERROR_NULL_POINTER                     Null pointer parameter
    @return SST_IX_RC_ERROR_RECORD_NOT_FOUND                 The requested record was not found
    @return SST_IX_RC_ERROR_MAX_COLLISIONS                   
   **/
    SSTIXReturnCodes_t SST_IXBind                       (SSTTxnId_t            transactionId,
                                                         SSTHandle_t           recordHandle,
                                                         DxByte_t             *string_ptr,
                                                         DxUint32_t            stringLengthInBytes,
                                                         DxByte_t             *scratchBuffer_ptr,
                                                         DxUint32_t            scratchBufferSizeInBytes);
    /*!
    \brief 
    Unbind a string (array of characters) to a handle of a data object in the SST.

    @param transactionId                [in]    Current transaction id
    @param recordHandle                 [in]    Pointer to the start of the scratch buffer
    @param string_ptr                   [in]    Pointer to the string
    @param stringLengthInBytes          [in]    Length of the string
    @param scratchBuffer_ptr            [in]    Pointer to the start of the scratch buffer
    @param scratchBufferSizeInBytes     [in]    Size of the scratch buffer (Cannot be less then SST_IX_MIN_WORKSPACE_SIZE_IN_BYTES)

    @return SST_IX_RC_OK                                     The unbind operation was successful 
    @return SST_IX_RC_FAIL                                   The unbind operation failed
    @return SST_IX_RC_ERROR_VDB                              DB operation failed
    @return SST_IX_RC_ERROR_RECORD_NOT_FOUND                 The requested record was not found
    @return SST_IX_RC_ERROR_NOT_ENOUGH_MEMORY                Not enough memory
    **/
    SSTIXReturnCodes_t SST_IXUnbind                     (SSTTxnId_t            transactionId,
                                                         SSTHandle_t           recordHandle,
                                                         DxByte_t             *string_ptr,
                                                         DxUint32_t            stringLengthInBytes,
                                                         DxByte_t             *scratchBuffer_ptr,
                                                         DxUint32_t            scratchBufferSizeInBytes);

    /*!
    \brief 
    Unbind all objects from the specific data object.

    @param transactionId                  [in]    Current transaction id
    @param recordHandle                   [in]    Handle to the record 
    @param scratchBuffer_ptr              [in]    Pointer to the start of the scratch buffer
    @param scratchBufferSizeInBytes       [in]    The size of the scratch buffer (Cannot be less then SST_IX_MIN_WORKSPACE_SIZE_IN_BYTES)

    @return SST_IX_RC_OK                                     The all indexes unbind operation was successful 
    @return SST_IX_RC_FAIL                                   The all indexes unbind operation failed
    @return SST_IX_RC_ERROR_VDB                              VDB operation failed
    @return SST_IX_RC_ERROR_RECORD_NOT_FOUND                 The requested record was not found
    **/
    SSTIXReturnCodes_t SST_IXAllIndexesUnbind           (SSTTxnId_t            transactionId,
                                                         SSTHandle_t           recordHandle,
                                                         DxByte_t             *scratchBuffer_ptr,
                                                         DxUint32_t            scratchBufferSizeInBytes);

    /* The following code is added for the futures features */
#ifdef SST_IX_ALL_OBJS_UNBIND  
    /*!
    \brief 
    Delete all objects same string corresponding to the different Metadata objects.

    @param transactionId                  [in]    Current transaction id
    @param string_ptr                     [in]    Pointer to the string
    @param stringLengthInBytes            [in]    Length of the string
    @param scratchBuffer_ptr              [in]    A pointer to the start of the scratch buffer
    @param scratchBufferSizeInBytes       [in]    The size of the scratch buffer (Cannot be less then SST_IX_MIN_WORKSPACE_SIZE_IN_BYTES)
    
    @return SST_IX_RC_OK                                     The all objects unbind operation was successful 
    @return SST_IX_RC_FAIL                                   The all objects unbind operation failed
    @return SST_IX_RC_ERROR_VDB                              VDB operation failed
    **/

    SSTIXReturnCodes_t SST_IXAllObjectsUnbind           (SSTTxnId_t            transactionId,
                                                         DxByte_t             *string_ptr,
                                                         DxUint32_t            stringLengthInBytes,
                                                         DxByte_t             *scratchBuffer_ptr,
                                                         DxUint32_t            scratchBufferSizeInBytes);
#endif

   /*!
    \brief 
        Create iterator by index for a given string

        @param string_ptr                 [in]      Pointer to the string   
        @param stringLengthInBytes        [in]      Length of the string
        @param iterCookie_ptr             [out]     Pointer to iterator

        @return SST_IX_RC_OK                                The iterator create operation was successful 
        @return SST_IX_RC_FAIL                              The iterator create operation failed
        @return SST_IX_RC_ERROR_VDB                         VDB operation failed

        **/
    SSTIXReturnCodes_t SST_IXHandleIteratorCreate       (DxByte_t             *string_ptr,
                                                         DxUint32_t            stringLengthInBytes, 
                                                         SSTIXCookie_t        *iterCookie_ptr);

    /*!
    \brief 
        Create iterator by index for a given string

        @param iterCookie_ptr             [in]     Pointer to the iterator handle
        @param handlesBuff                [out]    Pointer to a cookie chunk
        @param numberOfHandlesToRead_ptr  [in/out] Number of handles to be read. It is also output parameter which contains a real number of read handles
        @param handlesLeft_ptr            [in/out] Number of left handles 

        @return SST_IX_RC_OK                                The handles get operation was successful 
        @return SST_IX_RC_FAIL                              The handles get operation failed
        @return SST_IX_RC_ERROR_VDB                         VDB operation failed
        **/

    SSTIXReturnCodes_t SST_IXHandlesGet                 (SSTIXCookie_t       *iterCookie_ptr,
                                                         SSTHandle_t         *handlesBuff,
                                                         DxUint32_t          *numberOfHandlesToRead_ptr,
                                                         DxUint32_t          *handlesLeft_ptr);

    /*!
    \brief 
    Terminate iterator by index for a given string

    @param iterCookie_ptr                 [in]     Handle the user can identify this iterator

    @return SST_IX_RC_OK                                    The iterator terminate operation was successful 
    @return SST_IX_RC_FAIL                                  The iterator terminate operation failed
    @return SST_IX_RC_ERROR_VDB                             VDB operation failed

    **/
    SSTIXReturnCodes_t SST_IXHandleIteratorTerminate    (SSTIXCookie_t *iterCookie_ptr);
#endif  /* _DX_SST_IX_H_ */
