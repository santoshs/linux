
  /*
   *  Object %name    : %
   *  State           :  %state%
   *  Creation date   :  05 June 2007
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief A brief description of this module
   *
   *  \version CRYS_RC4.c#1:csrc:1
   *  \author R.Levin
   *  \remarks Copyright (C) 2007 by Discretix Technologies Ltd.
   *           All Rights reserved
   */



/************* Include Files ****************/

#include "DX_VOS_Mem.h"
#include "CRYS.h"
#include "CRYS_CCM.h"
#include "CRYS_RC4_error.h"
#include "CRYS_RC4_Local.h"

#include "LLF_RC4.h"

#include "log_output.h"


/************************ Defines ***************************************/

/* canceling the lint warning:
   Use of goto is deprecated */
/*lint --e{801} */


/************************ Enums *****************************************/


/************************ Macros ****************************************/


/************************ Public Functions ******************************/

/**
 * @brief This function is used to initialize the RC4 machine.
 *        To operate the RC4 machine, this should be the first function called.
 *
 * @param[in] ContextID_ptr - A pointer to the RC4 context buffer that is allocated by the user
 *                       and is used for the RC4 machine operation.
 * @param[in] Key_ptr -  A pointer to the user's key buffer.
 * @param[in] KeySize - The size of the KEY in bytes. Requirements:
 *             - for SW implementation    0 < KeySize < CRYS_RC4_MAX_KEY_SIZE_IN_BYTES,
 *             - for HW implementation    LLF_RC4_MIN_KEY_SIZE_IN_BYTES  < KeySize < LLF_RC4_MAX_KEY_SIZE_IN_BYTES,
 *
 * @return CRYSError_t - CRYS_OK,
 *                       CRYS_RC4_INVALID_USER_CONTEXT_POINTER_ERROR,
 *                       CRYS_RC4_ILLEGAL_KEY_SIZE_ERROR,
 *                       CRYS_RC4_INVALID_KEY_POINTER_ERROR
 */

CEXPORT_C CRYSError_t  CRYS_RC4_Init( CRYS_RC4UserContext_t    *ContextID_ptr,
                                      DxUint8_t                *Key_ptr,
                                      DxUint32_t                KeySizeInBytes)
{
   /* LOCAL VARIABLES AND INITIALIZATIONS */

   /* the Error identifiers */
   CRYSError_t Error;
   CRYSError_t Error1;

   /* defining a pointer to the active context allcated by the CCM */
   RC4Context_t *ccmWorkingContext_ptr;

   /* FUNCTION LOGIC */

   /* ............... local initializations .............................. */
   /* -------------------------------------------------------------------- */

   /* initializing the Error to O.K */
   Error = CRYS_OK;
   Error1 = CRYS_OK;

   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */
   //PRINTF("CRYS_RC4_Init function starts");
   RETURN_IF_RC4_UNSUPPORTED( ContextID_ptr , Key_ptr , KeySizeInBytes ,
                              ccmWorkingContext_ptr , Error , Error ,
                              Error, Error , Error , Error , Error , Error , Error , Error );

   #ifndef CRYS_NO_RC4_SUPPORT

   /* ............... checking the parameters validity ................... */
   /* -------------------------------------------------------------------- */

   /* if the users context ID pointer is DX_NULL return an error */
   if( ContextID_ptr == DX_NULL )

      return CRYS_RC4_INVALID_USER_CONTEXT_POINTER_ERROR;


   /*init the context valid_tag field to avoid warnings*/
   ContextID_ptr->valid_tag = 0;

   /* If the Keys size is invalid return an error */
   if( KeySizeInBytes == 0 || KeySizeInBytes > CRYS_RC4_MAX_KEY_SIZE_IN_BYTES)

      return  CRYS_RC4_ILLEGAL_KEY_SIZE_ERROR;


   /* If the the key pointer is not validity */
   if( Key_ptr == DX_NULL )

      return  CRYS_RC4_INVALID_KEY_POINTER_ERROR;

   /* ................. aquiring the AES contest ............................. */
   /* ----------------------------------------------------------------------- */

   Error = CRYS_CCM_GetContext( ContextID_ptr,                     /* the users context space - in */
                                (void **) &ccmWorkingContext_ptr,  /* the CCM context returned - out */
                                DX_RC4_CONTEXT,                    /* AES type - in */
                                AES_DONT_DECRYPT_CONTEXT);		   /*No need to decrypt context in Init*/

   if( Error != CRYS_OK )

     return Error;

   /* 5. Set IsSboxInitialized flag to 0 befor S-box initialisation */
   ccmWorkingContext_ptr->IsSboxInitialized = 0;


   /* ................. calling the low level init function ................. */
   /* ----------------------------------------------------------------------- */

   Error = LLF_RC4_Init( ccmWorkingContext_ptr , Key_ptr , KeySizeInBytes );

   if( Error != CRYS_OK )

     goto End;


   /* set the RC4 tag to the users context */
   ContextID_ptr->valid_tag = RC4_CONTEXT_VALIDATION_TAG;


 End:
   /* ................. release the context ................................. */
   /* ----------------------------------------------------------------------- */

   Error1 = CRYS_CCM_ReleaseContext(ContextID_ptr,               /* the users context space - in */
                           ccmWorkingContext_ptr,       /* the CCM context returned - in */
                           DX_RC4_CONTEXT);             /* RC4 type - in */

    if(Error == CRYS_OK)
    {
        Error = Error1;
    }
   if( Error != CRYS_OK )

       /*  clearing the users context  */
       DX_VOS_MemSet( ContextID_ptr , 0 , sizeof(CRYS_RC4UserContext_t) );
    //PRINTF("CRYS_RC4_Init completed");

   return Error;

   #endif /*CRYS_NO_RC4_SUPPORT*/

}/* END OF CRYS_RC4_Init */


/******************************************************************************************************/
/**
 * @brief This function is used to process a stream on the RC4 machine.
 *        This function should be called after the CRYS_RS4_Init.
 *
 *
 * @param[in] ContextID_ptr - A pointer to the RC4 context buffer allocated by the user
 *                       that is used for the RC4 machine operation. This should be the
 *                       same context as was used for the previous call of this session.
 *
 * @param[in] DataIn_ptr - The pointer to the buffer of the input data to the RC4.
 *                   The pointer's value does not need to be word-aligned.
 *
 * @param[in] DataInSize - The size of the input data.
 *
 * @param[in,out] DataOut_ptr - The pointer to the buffer of the output data from the RC4.
 *                        The pointer's value does not need to be word-aligned.
 *
 * @return CRYSError_t - CRYS_OK,
 *                       CRYS_RC4_INVALID_USER_CONTEXT_POINTER_ERROR,
 *                       CRYS_RC4_USER_CONTEXT_CORRUPTED_ERROR,
 *                       CRYS_RC4_DATA_IN_POINTER_INVALID_ERROR,
 *                       CRYS_RC4_DATA_OUT_POINTER_INVALID_ERROR,
 *                       CRYS_RC4_DATA_OUT_DATA_IN_OVERLAP_ERROR,
 *                       CRYS_RC4_DATA_SIZE_ILLEGAL
 */
 CEXPORT_C CRYSError_t  CRYS_RC4_Stream(
	                          CRYS_RC4UserContext_t        *ContextID_ptr,
                              DxUint8_t                    *DataIn_ptr,
                              DxUint32_t                    DataInSize,
                              DxUint8_t                    *DataOut_ptr)
{
   /* LOCAL VARIABLES AND INITIALIZATIONS */

   /* the Error identifiers */
   CRYSError_t Error;
   CRYSError_t Error1;

   /* defining a pointer to the active context allcated by the CCM */
   RC4Context_t *ccmWorkingContext_ptr;

   /* FUNCTION LOGIC */

   /* ............... local initializations .............................. */
   /* -------------------------------------------------------------------- */

   /* initializing the Error to O.K */
   Error = CRYS_OK;
   Error1 = CRYS_OK;

   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */
   //PRINTF("CRYS_RC4_Stream function starts\n\n");
   RETURN_IF_RC4_UNSUPPORTED( ContextID_ptr , DataIn_ptr , DataInSize ,
                              DataOut_ptr , ccmWorkingContext_ptr , Error ,
                              Error , Error , Error , Error , Error , Error , Error , Error );

   #ifndef CRYS_NO_RC4_SUPPORT

   /* ............... checking the parameters validity ................... */
   /* -------------------------------------------------------------------- */

   /* if the users context ID pointer is DX_NULL return an error */
   if( ContextID_ptr == DX_NULL )

      return CRYS_RC4_INVALID_USER_CONTEXT_POINTER_ERROR;


   /* if the users context TAG is illegal return an error - the context is invalid */
   if( ContextID_ptr->valid_tag != RC4_CONTEXT_VALIDATION_TAG )

      return CRYS_RC4_USER_CONTEXT_CORRUPTED_ERROR;

   /* if the users Data In pointer is illegal return an error */
   if( DataIn_ptr == DX_NULL )

      return CRYS_RC4_DATA_IN_POINTER_INVALID_ERROR;

   /* if the users Data Out pointer is illegal return an error */
   if( DataOut_ptr == DX_NULL )

      return CRYS_RC4_DATA_OUT_POINTER_INVALID_ERROR;

      /* if the data size is zero return error */
   if( DataInSize == 0 )

      return CRYS_RC4_DATA_SIZE_ILLEGAL;

   /* checking that there is no overlapping between the data input and data out put buffer
      except the inplace case that is legal */
   if( DataIn_ptr != DataOut_ptr )
   {
      /* checking the case that the input buffer is in a higher address then the output buffer */
      if ( DataIn_ptr > DataOut_ptr )
      {
         /* if after adding the size to the data out pointer it is larger then the data in pointer
            return the overlap error */
         if( DataOut_ptr + DataInSize > DataIn_ptr )

            return CRYS_RC4_DATA_OUT_DATA_IN_OVERLAP_ERROR;

      }/* end of DataIn_ptr > DataOut_ptr */

      /* checking the case that the output buffer is in a higher address then the input buffer */
      else
      {
         /* if after adding the size to the data in pointer it is larger then the data out pointer
         return the overlap error */
         if( DataIn_ptr + DataInSize > DataOut_ptr )

            return CRYS_RC4_DATA_OUT_DATA_IN_OVERLAP_ERROR;

      }/* end of DataOut_ptr > DataIn_ptr */

   }/* end of DataIn_ptr != DataOut_ptr case */


   /* ................. aquiring the AES contest ............................. */
   /* ----------------------------------------------------------------------- */

   Error = CRYS_CCM_GetContext(
                      ContextID_ptr,					 /* the users context space - in */
                      (void **) &ccmWorkingContext_ptr,  /* the CCM context returned - out */
                      DX_RC4_CONTEXT,                    /* RC4 type - in */
                      AES_DECRYPT_CONTEXT);				 /* need to decrypt context in AES_block*/

   if( Error != CRYS_OK )

     return Error;

   /* .................. calling the low level stream function ............... */
   /* ------------------------------------------------------------------------ */

   Error = LLF_RC4_Stream( ccmWorkingContext_ptr,      /* the working context - in */
                           DataIn_ptr,                 /* the input data buffer - in */
                           DataOut_ptr,                /* the output data buffer i/o */
                           DataInSize);                /* the data in size - in */



   /* ................. release the context ................................. */
   /* ----------------------------------------------------------------------- */

   Error1 = CRYS_CCM_ReleaseContext( ContextID_ptr,			   /* the users context space - in */
                            ccmWorkingContext_ptr,       /* the CCM context returned - in */
                            DX_RC4_CONTEXT);             /* RC4 type - in */
    if(Error == CRYS_OK)
    {
        Error = Error1;
    }

   if( Error != CRYS_OK )

      /* on error clean the users context */
      DX_VOS_MemSet( ContextID_ptr , 0 , sizeof(CRYS_RC4UserContext_t) );

  //PRINTF("CRYS_RC4_Stream function completed");
   return Error;

   #endif /*CRYS_NO_RC4_SUPPORT*/

}/* END OF CRYS_RC4_Stream */


/******************************************************************************************************/
/**
 * @brief This function is used to end the RC4 processing session.
 *        It is the last function called for the RC4 process.
 *
 *
 * @param[in] ContextID_ptr - A pointer to the RC4 context buffer allocated by the user that
 *                       is used for the RC4 machine operation. This should be the
 *                       same context as was used for the previous call of this session.
 *
 *
 * @return CRYSError_t -  CRYS_OK,
 *                        CRYS_RC4_INVALID_USER_CONTEXT_POINTER_ERROR,
 *                        CRYS_RC4_USER_CONTEXT_CORRUPTED_ERROR
 */
 CEXPORT_C CRYSError_t  CRYS_RC4_Free(CRYS_RC4UserContext_t  *ContextID_ptr )
 {
   /* FUNCTION DECLERATIONS */

   /* The return error identifier */
   CRYSError_t Error;

   /* FUNCTION LOGIC */

   /* ............... local initializations .............................. */
   /* -------------------------------------------------------------------- */

   /* initializing the Error to O.K */
   Error = CRYS_OK;

   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */
   //PRINTF("CRYS_RC4_Free function starts\n\n");
   RETURN_IF_RC4_UNSUPPORTED( ContextID_ptr , ContextID_ptr , ContextID_ptr ,
                              ContextID_ptr , ContextID_ptr , ContextID_ptr ,
                              Error , Error , Error , Error , Error , Error , Error , Error );

   #ifndef CRYS_NO_RC4_SUPPORT

   /* ............... checking the parameters validity ................... */
   /* -------------------------------------------------------------------- */

   /* if the users context ID pointer is DX_NULL return an error */
   if( ContextID_ptr == DX_NULL )
   {
      Error = CRYS_RC4_INVALID_USER_CONTEXT_POINTER_ERROR;
      goto End;
   }

   /* if the users context TAG is illegal return an error - the context is invalid */
   if( ContextID_ptr->valid_tag != RC4_CONTEXT_VALIDATION_TAG )
   {
      Error = CRYS_RC4_USER_CONTEXT_CORRUPTED_ERROR;
      goto End;
   }

   /* .............. clearing the users context .......................... */
   /* -------------------------------------------------------------------- */


   DX_VOS_MemSet( ContextID_ptr , 0 , sizeof(CRYS_RC4UserContext_t) );

   //PRINTF("CRYS_RC4_Free function ends\n\n");

   /* ................. end of function ..................................... */
   /* ----------------------------------------------------------------------- */

 End:

   return Error;

   #endif /*CRYS_NO_RC4_SUPPORT*/

}/* END OF CRYS_RC4_Free */


/***********************************************************************************************/
/**
 * \brief This function provides a RC4 operation for processing one continuous block of data.
 *
 *        The function performs RC4 operation  by calling CRYS_RC4_Init, CRYS_RC4_Stream and
 *        CRYS_RC4_Free functions.
 *
 * @param[in] Key_ptr -  A pointer to the user's key buffer.
 *
 * @param[in] KeySize - The size of the KEY in bytes.
 *
 * @param[in] DataIn_ptr - The pointer to the buffer of the input data to the RC4.
 *                   The pointer's value does not need to be word-aligned.
 *
 * @param[in] DataInSize - The size of the input data.
 *
 * @param[in,out] The pointer to the buffer of the output data from the RC4.
 *                The pointer's value does not need to be word-aligned. The size of this buffer
 *                must be the same as the DataIn buffer.
 *
 * @return CRYSError_t -  On success the function returns the value CRYS_OK,
 *				and on failure a value from CRYS_RC4_error.h
 *
 */
CEXPORT_C    CRYSError_t CRYS_RC4( DxUint8_t   *Key_ptr,
							      DxUint32_t    KeySizeInBytes,
								  DxUint8_t    *DataIn_ptr,
								  DxUint32_t    DataInSize,
								  DxUint8_t    *DataOut_ptr )
{
   /* FUNCTION DECLERATIONS */

   /* The return error identifier */
   CRYSError_t Error;

   /* a users context used to pass to all of the CRYS functions */
   CRYS_RC4UserContext_t  ContextID;

   /* FUNCTION LOGIC */

   /* ............... local initializations .............................. */
   /* -------------------------------------------------------------------- */

   /* initializing the Error to O.K */
   Error = CRYS_OK;

   PRINTF("CRYS_RC4 function entered\n\n");

   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */

   RETURN_IF_RC4_UNSUPPORTED( Key_ptr, KeySizeInBytes, DataIn_ptr, DataInSize,
                              DataOut_ptr , ContextID.valid_tag, Error , Error ,
                              Error , Error , Error , Error , Error , Error  );

   #ifndef CRYS_NO_RC4_SUPPORT

   /* ............... calling the CRYS init function ...................... */
   /* --------------------------------------------------------------------- */

   Error = CRYS_RC4_Init( &ContextID,
                          Key_ptr,
				  KeySizeInBytes);

   if( Error != CRYS_OK )

     return Error;

   /* ............... calling the CRYS Stream function .................... */
   /* --------------------------------------------------------------------- */

   Error =  CRYS_RC4_Stream(&ContextID,
                            DataIn_ptr,
                            DataInSize,
                            DataOut_ptr);

   if( Error != CRYS_OK )

     return Error;

   /* ................. end of function ..................................... */
   /* ----------------------------------------------------------------------- */

   /* call the free function - release the users context */

   Error = CRYS_RC4_Free( &ContextID );

   //PRINTF("CRYS_RC4 function ended\n\n");

   return Error;

   #endif /*CRYS_NO_RC4_SUPPORT*/

}/* END OF CRYS_RC4 */
