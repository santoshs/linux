/***********************************************************************************  
 * Copyright 2009 © Discretix Technologies Ltd. 
 * This software is protected by copyright, international treaties and 
 * various patents. If the license governing the use of this Software 
 * allows copy or redistribution of this  software then any copy or 
 * reproduction of this Software must include this Copyright Notice 
 * as well as any other notices provided under such license. 
 ***********************************************************************************/


/************* Include Files ****************/

#include "DX_VOS_Mem.h"
#include "CRYS.h"
#include "CRYS_HMAC_error.h"
#include "CRYS_HMAC_Local.h"
#include "CRYS_CCM.h"
#include "CRYS_COMMON.h"
 
/************************ Defines ***************************************/

/* canceling the PC-lint warning:
   Use of goto is deprecated */
/*lint --e{801} */

/************************ macros ****************************************/

/************************ Typedefs **************************************/


/* the MAC key IPAD and OPAD bytes */
#define MAC_KEY_IPAD_BYTE 0x36
#define MAC_KEY_OPAD_BYTE 0x5C

/************************ Global Data ***********************************/


/************* Private function prototype *******************************/


/************************ Public Functions ******************************/

/**
 * This function initializes the HMAC machine on the CRYS level.
 *
 * This function allocates and initializes the HMAC Context .
 * The function receives as input a pointer to store the context handle to HMAC Context. 
 *
 * If the key is larger then 64 bytes it performs on it a HASH operation.
 * then the function executed a HASH_init session and processes a HASH update 
 * on the Key XOR ipad and stores it on the context.
 *
 * the context. 
 * 
 * @param[in] ContextID_ptr - a pointer to the HMAC context buffer allocated by the user that
 *                       is used for the HMAC machine operation.
 *
 * @param[in] OperationMode - The operation mode : MD5 or SHA1.
 *
 * @param[in] key_ptr - The pointer to the users key buffer.
 *
 * @oaram[in] keySize - The size of the received key.
 *
 * @return CRYSError_t on success the function returns CRYS_OK else non ZERO error.
 *      
 */
CEXPORT_C CRYSError_t CRYS_HMAC_Init(CRYS_HMACUserContext_t *ContextID_ptr,
                           CRYS_HASH_OperationMode_t         OperationMode,
                           DxUint8_t                        *key_ptr,
                           DxUint16_t                        keySize )
{
   /* LOCAL DECLERATIONS */

   /* The return error identifier */
   CRYSError_t Error;
   CRYSError_t Error1;
   
   /* defining a pointer to the active context allcated by the CCM */
   HMACContext_t *ccmWorkingContext_ptr;
   
   /* the key buffer XOR with ipad as an input to the HASH - we takes the highest value for SHA2 */
   DxUint8_t keyXorIpadBuff[CRYS_HMAC_SHA2_1024BIT_KEY_SIZE_IN_BYTES];
   
   /* the KEY hash result - relevant if the key is larger then 64 bytes */
   CRYS_HASH_Result_t keyHashResult;
   
   /* the length of the key according to the HASH used */
   DxUint16_t		  keyHashLength;
   
   /* loop variable */
   DxUint8_t i;
        
   /* FUNCTION LOGIC */
   
   /* ............... local initializations .............................. */
   /* -------------------------------------------------------------------- */
   
   /* initializing the Error to O.K */
   Error = CRYS_OK;
   Error1 = CRYS_OK;

   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */   

   RETURN_IF_HMAC_UNSUPPORTED( OperationMode , key_ptr , keySize , 
                               ccmWorkingContext_ptr , keyHashResult[0] , keyHashResult[0] , 
                               keyXorIpadBuff[0] , keyHashLength , Error , i ); 
                              
   #ifndef CRYS_NO_HASH_SUPPORT                                      
      
   /* ............... checking the parameters validity ................... */
   /* -------------------------------------------------------------------- */
   
   /* if the users context ID pointer is DX_NULL return an error */
   if( ContextID_ptr == DX_NULL )
   
      return CRYS_HMAC_INVALID_USER_CONTEXT_POINTER_ERROR;
      
   /* init the context valid_tag field to avoid warnings */
   ContextID_ptr->valid_tag = 0;

   /* check if the operation mode is legal */
   if( OperationMode >= CRYS_HASH_NumOfModes )
   
      return CRYS_HMAC_ILLEGAL_OPERATION_MODE_ERROR;
   
   /* check if the key pointer is valid */
   if( key_ptr == DX_NULL )
   
      return CRYS_HMAC_INVALID_KEY_POINTER_ERROR;
   

   /* check if the key size is valid */
   if( keySize == 0 )
   
      return CRYS_HMAC_UNVALID_KEY_SIZE_ERROR;

#ifndef CRYS_NO_FIPS_SUPPORT      
   	/* check if we are in FIPS mode */
   if(DX_GLOBAL_FIPS_MODE & DX_CRYS_FIPS_MODE_STATE)
   {
   	/* check if there was a FIPS error */
		if(DX_GLOBAL_FIPS_MODE & DX_CRYS_FIPS_MODE_ERROR_STATE)
   	{
   		return CRYS_HMAC_FIPS_MODE_FAIL_ERROR;
   	}
   	/* no check if the self test was already executed */
   	if( !(DX_GLOBAL_FIPS_SF_STATUS & CRYS_SELF_TEST_HMAC_BIT) )
   	{
   		return CRYS_HMAC_FIPS_MODE_SF_NOT_PERFORMED_ERROR;
   	}
   	/* check for CBC-MAC - not allowed by FIPS */
   	if(OperationMode == CRYS_HASH_MD5_mode)
   	{
   		return CRYS_HMAC_FIPS_MODE_NOT_ALLOWED_ERROR;
   	}
   }   
#endif      
    
   /* ................. aquiring the HMAC contest ........................... */
   /* ----------------------------------------------------------------------- */
   Error = CRYS_CCM_GetContext( ContextID_ptr,       				 /* the users context space - in */
                                (void **) &ccmWorkingContext_ptr,    /* the CCM context returned - out */
                                DX_HMAC_CONTEXT,                     /* HMAC type - in */ 
                                AES_DONT_DECRYPT_CONTEXT);			 /* don't need to decrypt context in Init*/   
  
   if( Error != CRYS_OK ) 
   
     return Error;;                                    
   
   /* ................. setting the key ...................................... */
   /* ------------------------------------------------------------------------- */
   if(OperationMode == CRYS_HASH_SHA512_mode || OperationMode == CRYS_HASH_SHA384_mode)
   		keyHashLength = CRYS_HMAC_SHA2_1024BIT_KEY_SIZE_IN_BYTES;
   else
   		keyHashLength = CRYS_HMAC_KEY_SIZE_IN_BYTES;
   
   
   /*  clear the key vector */  
   DX_VOS_MemSet( keyXorIpadBuff , 0 , sizeof(keyXorIpadBuff) );
   
   /* if the key size is more then 64 byte perform a hash on the key  */
   if( keySize > keyHashLength/*CRYS_HMAC_KEY_SIZE_IN_BYTES*/ )
   {
      /* operate the hash on the key */
      Error = CRYS_HASH  ( OperationMode,                      /* the operation mode - in */
                           key_ptr,                            /* the data in */ 
                           keySize,                            /* the data in size */
                           keyHashResult );                    /* the result - out */ 
                           
      if( Error != CRYS_OK )
      
        goto End; 
      
      /* update the new key size according to the mode */
      switch(OperationMode)
      {
      	case CRYS_HASH_MD5_mode:
      		keySize = CRYS_HASH_MD5_DIGEST_SIZE_IN_BYTES;
      		break;
      		
      	case CRYS_HASH_SHA1_mode:
      		keySize = CRYS_HASH_SHA1_DIGEST_SIZE_IN_BYTES;
      		break;
      		
      	case CRYS_HASH_SHA224_mode:
      		keySize = CRYS_HASH_SHA224_DIGEST_SIZE_IN_BYTES;
      		break;
      		
      	case CRYS_HASH_SHA256_mode:
      		keySize = CRYS_HASH_SHA256_DIGEST_SIZE_IN_BYTES;
      		break;
      		
      	case CRYS_HASH_SHA384_mode:
      		keySize = CRYS_HASH_SHA384_DIGEST_SIZE_IN_BYTES;
      		break;
      		
      	case CRYS_HASH_SHA512_mode:
      		keySize = CRYS_HASH_SHA512_DIGEST_SIZE_IN_BYTES;
      		break;
      		
      	default:
      		break;
      }
      
          
      /* copy the result to the buffer */
      DX_VOS_FastMemCpy( keyXorIpadBuff , keyHashResult , keySize );                       
                                 
   }/* end of key larger then 64 bytes case */
   
   /*  if the key is no mopre then 64 bytes copy it to the buffer */
   else
   
      DX_VOS_FastMemCpy(keyXorIpadBuff , key_ptr , keySize );                                   
   
   /* ................. initializing the context ............................ */
   /* ----------------------------------------------------------------------- */ 
     
   /* load the key XOR opad to the context */
   for ( i = 0 ; i < keyHashLength/*CRYS_HMAC_KEY_SIZE_IN_BYTES*/ ; i++ )
   
      ccmWorkingContext_ptr->KeyXorOpadBuff[i] = keyXorIpadBuff[i] ^ MAC_KEY_OPAD_BYTE;
   
   /* execute the XOR ipad on the key */
   for ( i = 0 ; i < keyHashLength/*CRYS_HMAC_KEY_SIZE_IN_BYTES*/ ; i++ )
    
      keyXorIpadBuff[i] ^= MAC_KEY_IPAD_BYTE;
      
   /* initialize the HASH with the HASH subset context on the HMAC context .... */ 
   Error = CRYS_HASH_Init( &ccmWorkingContext_ptr->HashUserContext,
                           OperationMode);
                               
   if( Error != CRYS_OK )
      
     goto End; 
                            
   /* update the HASH context with the Key XOR ipad */
   Error = CRYS_HASH_Update( &ccmWorkingContext_ptr->HashUserContext,
                             keyXorIpadBuff,
                             keyHashLength/*CRYS_HMAC_KEY_SIZE_IN_BYTES*/ );
  
   if( Error != CRYS_OK )
      
     goto End; 

   /* set the operation mode */
   ccmWorkingContext_ptr->OperationMode = OperationMode;

   /* set the HMAC tag to the users context */
   ContextID_ptr->valid_tag = HMAC_CONTEXT_VALIDATION_TAG;
    
    
   /* ................. release the context ................................. */
   /* ----------------------------------------------------------------------- */
  
   End:
   
   /*  clear the key vector for security reasons */  
   DX_VOS_MemSet( keyXorIpadBuff , 0 , sizeof(keyXorIpadBuff) );


   Error1 = CRYS_CCM_ReleaseContext(ContextID_ptr,				   /* the users context space - in */
                              ccmWorkingContext_ptr,       /* the CCM context returned - in */
                              DX_HMAC_CONTEXT);            /* HASH type - in */
                                   
   if(Error == CRYS_OK)
   {
        Error = Error1;
   }
   return Error;                                 
  
   #endif /* !CRYS_NO_HASH_SUPPORT */
                              
}/* END OF CRYS_HMAC_Init */

/**
 * This function process a HMAC block of data via the HASH Hardware.
 * The function receives as input an handle to the  HMAC Context , and operated the HASH update on the data
 * below.
 * 
 * @param[in] ContextID_ptr - a pointer to the HMAC context buffer allocated by the user that
 *                       is used for the HMAC machine operation.
 *
 * @param[in] DataIn_ptr - The pointer to the buffer of the input data to the HMAC. The pointer does 
 *                         not need to be aligned. On CSI input mode the pointer must be equal to
 *                         value (0xFFFFFFFC | DataInAlignment). 
 * 
 * @param[in] DataInSize - The size of the data to be hashed in bytes. On CSI data transfer mode the size must  
 *                         multiple of HASH_BLOCK_SIZE for used HASH mode.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* CRYS_HMAC_error.h
 *
 */

CEXPORT_C CRYSError_t CRYS_HMAC_Update(CRYS_HMACUserContext_t  *ContextID_ptr,
                             DxUint8_t                 *DataIn_ptr,
                             DxUint32_t                 DataInSize )
{
   /* LOCAL DECLERATIONS */

   /* The return error identifier */
   CRYSError_t Error;
   CRYSError_t Error1;
   
   /* defining a pointer to the active context allcated by the CCM */
   HMACContext_t *ccmWorkingContext_ptr;

   /* FUNCTION LOGIC */

   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */   
   
   RETURN_IF_HMAC_UNSUPPORTED( Error , ContextID_ptr , DataIn_ptr , 
                               DataInSize , ccmWorkingContext_ptr , ccmWorkingContext_ptr , 
                               ccmWorkingContext_ptr , ccmWorkingContext_ptr , ccmWorkingContext_ptr , ccmWorkingContext_ptr ); 
                              
   #ifndef CRYS_NO_HASH_SUPPORT                                      

   /* ............... checking the parameters validity ................... */
   /* -------------------------------------------------------------------- */

   /* if the users context ID pointer is DX_NULL return an error */
   if( ContextID_ptr == DX_NULL )
   
      return CRYS_HMAC_INVALID_USER_CONTEXT_POINTER_ERROR; 
   
   /* if the users context TAG is illegal return an error - the context is invalid */
   if( ContextID_ptr->valid_tag != HMAC_CONTEXT_VALIDATION_TAG )
   
      return CRYS_HMAC_USER_CONTEXT_CORRUPTED_ERROR;   
   
   /* if the users Data In pointer is illegal and the size is not 0 return an error */
   if( DataIn_ptr == DX_NULL && DataInSize )
 
      return CRYS_HMAC_DATA_IN_POINTER_INVALID_ERROR;
           
   /* if the data size is zero no need to execute an update , return CRYS_OK */
   if( DataInSize == 0 )
  
      return CRYS_OK;

   /* ................. aquiring the HMAC contest ........................... */
   /* ----------------------------------------------------------------------- */
   Error = CRYS_CCM_GetContext( ContextID_ptr,       				 /* the users context space - in */
                                (void **) &ccmWorkingContext_ptr,    /* the CCM context returned - out */
                                DX_HMAC_CONTEXT,                     /* HMAC type - in */ 
                                AES_DECRYPT_CONTEXT);			 	 /* need to decrypt context in Update*/   
     
   if( Error != CRYS_OK ) 
   
     return Error;                                    

   /* ................. call the HASH update function ....................... */
   /* ----------------------------------------------------------------------- */
   
   Error = CRYS_HASH_Update( &ccmWorkingContext_ptr->HashUserContext,
                             DataIn_ptr,
                             DataInSize );

   if( Error != CRYS_OK ) 
   
     goto End;                                    

   /* ................. release the context ................................. */
   /* ----------------------------------------------------------------------- */
  
   End:

   Error1 = CRYS_CCM_ReleaseContext(ContextID_ptr, 				   /* the users context space - in */
                              ccmWorkingContext_ptr,       /* the CCM context returned - in */
                              DX_HMAC_CONTEXT);            /* HASH type - in */
                              
   if(Error == CRYS_OK)
   {
        Error = Error1;
   }
   
   if(Error)
   {
   	DX_VOS_MemSetZero(ContextID_ptr , sizeof(CRYS_HMACUserContext_t));
   }
   
   return Error;                              

   #endif /* !CRYS_NO_HASH_SUPPORT */
   
}/* END OF CRYS_HMAC_Update */ 

/**
 * This function finalize the HMAC process of data block.
 * The function receives as input an handle to the HMAC Context , that was initialized before
 * by an CRYS_HMAC_Init function or by CRYS_HMAC_Update function.
 * The function finishes the HASH operation on the ipad and text then 
 * executes a new hash operation with the key XOR opad and the previous HASH operation result.  
 *
 *  @param[in] ContextID_ptr - a pointer to the HMAC context buffer allocated by the user that
 *                       is used for the HMAC machine operation.
 *
 *  @retval HmacResultBuff a pointer to the target buffer where the 
 *                       HMAC result stored in the context is loaded to.
 *
 *  @return CRYSError_t on success the function returns CRYS_OK else non ZERO error.
 */

CEXPORT_C CRYSError_t CRYS_HMAC_Finish( CRYS_HMACUserContext_t  *ContextID_ptr ,
                              CRYS_HASH_Result_t       HmacResultBuff )
{
   /* LOCAL DECLERATIONS */
 
   /* The return error identifier */
   CRYSError_t Error;
   CRYSError_t Error1;
   
   /* defining a pointer to the active context allcated by the CCM */
   HMACContext_t *ccmWorkingContext_ptr;
   
   /* the result of the step1 og the HMAC operation */
   CRYS_HASH_Result_t HmacStep1ResultBuff;
   
   /* the result size */
   DxUint8_t HmacStep1ResultSize;
   
   /* the length of the key used according to SHA mode */
   DxUint16_t keyHashLength;
   

   /* FUNCTION LOGIC */
  
   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */   
   
   RETURN_IF_HMAC_UNSUPPORTED( Error , ContextID_ptr , HmacResultBuff[0] , 
                               ccmWorkingContext_ptr , HmacStep1ResultBuff[0] , HmacStep1ResultSize , 
                               keyHashLength , Error , Error , Error ); 
                              
   #ifndef CRYS_NO_HASH_SUPPORT                                      
 
   /* ............... checking the parameters validity ................... */
   /* -------------------------------------------------------------------- */

   /* if the users context ID pointer is DX_NULL return an error */
   if( ContextID_ptr == DX_NULL )
   
      return CRYS_HMAC_INVALID_USER_CONTEXT_POINTER_ERROR;

   /* if the result buffer DX_NULL return an error */
   if( HmacResultBuff == DX_NULL )
   
      return CRYS_HMAC_INVALID_RESULT_BUFFER_POINTER_ERROR;
   
      /* if the users context TAG is illegal return an error - the context is invalid */
   if( ContextID_ptr->valid_tag != HMAC_CONTEXT_VALIDATION_TAG )
   
      return CRYS_HMAC_USER_CONTEXT_CORRUPTED_ERROR;   

   /* ................. aquiring the HMAC contest ........................... */
   /* ----------------------------------------------------------------------- */
   
   Error = CRYS_CCM_GetContext( ContextID_ptr,       				       /* the users context space - in */
                                (void **) &ccmWorkingContext_ptr,  /* the CCM context returned - out */
                                DX_HMAC_CONTEXT,                   /* HMAC type - in */ 
                                AES_DECRYPT_CONTEXT);			 	       /* need to decrypt context in Finish*/   
     
   if( Error != CRYS_OK ) 
   
     return Error; 
     
   /* ......... call the HASH finish function to get step1 result ........... */
   /* ----------------------------------------------------------------------- */
   
   /* The step 1 result containes the HASH result of the Kipad and the data */
   Error = CRYS_HASH_Finish( &ccmWorkingContext_ptr->HashUserContext ,
                             HmacStep1ResultBuff );
                                      
   if( Error != CRYS_OK ) 
   
     goto End;
   
   /* ......... re initialize the HASH context .............................. */
   /* ----------------------------------------------------------------------- */
   
   /* update the key length according to the operation mode */
   if(ccmWorkingContext_ptr->OperationMode == CRYS_HASH_SHA512_mode || ccmWorkingContext_ptr->OperationMode == CRYS_HASH_SHA384_mode)

   		keyHashLength = CRYS_HMAC_SHA2_1024BIT_KEY_SIZE_IN_BYTES;
   else
   		keyHashLength = CRYS_HMAC_KEY_SIZE_IN_BYTES;
   
   
      /* initialize the HASH with the HASH subset context on the HMAC context .... */ 
   Error = CRYS_HASH_Init( &ccmWorkingContext_ptr->HashUserContext,
                           ccmWorkingContext_ptr->OperationMode);
                               
   if( Error != CRYS_OK )
      
     goto End; 
                                   

   /* ......... call the HASH update function with the key XOR opad vector .. */
   /* ----------------------------------------------------------------------- */
   
   Error = CRYS_HASH_Update( &ccmWorkingContext_ptr->HashUserContext,
                             ccmWorkingContext_ptr->KeyXorOpadBuff,
                             keyHashLength/*CRYS_HMAC_KEY_SIZE_IN_BYTES*/ );
   
   /* ......... call the HASH update function with the result of step 1 ..... */
   /* ----------------------------------------------------------------------- */
   
   
   /* set the size of the result according to the mode */
   switch(ccmWorkingContext_ptr->OperationMode)
   {
   	case CRYS_HASH_MD5_mode:
   		HmacStep1ResultSize = CRYS_HASH_MD5_DIGEST_SIZE_IN_BYTES;
   		break;
   		
   	case CRYS_HASH_SHA1_mode:
   		HmacStep1ResultSize = CRYS_HASH_SHA1_DIGEST_SIZE_IN_BYTES;
   		break;

	case CRYS_HASH_SHA224_mode:
   		HmacStep1ResultSize = CRYS_HASH_SHA224_DIGEST_SIZE_IN_BYTES;
   		break;
   		
   	case CRYS_HASH_SHA256_mode:
   		HmacStep1ResultSize = CRYS_HASH_SHA256_DIGEST_SIZE_IN_BYTES;
   		break;
   		
   	case CRYS_HASH_SHA384_mode:
   		HmacStep1ResultSize = CRYS_HASH_SHA384_DIGEST_SIZE_IN_BYTES;
   		break;
   		
   	case CRYS_HASH_SHA512_mode:
   		HmacStep1ResultSize = CRYS_HASH_SHA512_DIGEST_SIZE_IN_BYTES;
   		break;
   		
   	default:
   		/* the default size is 0 - we should never reach this code */
   		HmacStep1ResultSize = 0;
   		break;
   }
     
     
   Error = CRYS_HASH_Update( &ccmWorkingContext_ptr->HashUserContext,
                             (DxUint8_t*)HmacStep1ResultBuff,
                             HmacStep1ResultSize );
 
   if( Error != CRYS_OK ) 
   
     goto End;
     
   /* ......... call the HASH finish function ............................... */
   /* ----------------------------------------------------------------------- */
   
   Error = CRYS_HASH_Finish( &ccmWorkingContext_ptr->HashUserContext ,
                             HmacResultBuff );
                                      
   if( Error != CRYS_OK ) 
   
     goto End;

   /* ................. release the context ................................. */
   /* ----------------------------------------------------------------------- */
  
   End:

   Error1 = CRYS_CCM_ReleaseContext(ContextID_ptr, 				/* the users context space - in */
                              ccmWorkingContext_ptr,    /* the CCM context returned - in */
                              DX_HMAC_CONTEXT);         /* HMAC type - in */

   if(Error == CRYS_OK)
   {
        Error = Error1;
   }
   /* .............. clearing the users context .......................... */
   /* -------------------------------------------------------------------- */
   
   DX_VOS_MemSet( ContextID_ptr , 0 , sizeof(CRYS_HMACUserContext_t) );   
   
   return Error;   

   #endif /* !CRYS_NO_HASH_SUPPORT */

}/* END OF CRYS_HMAC_Finish */

/**
 * @brief This function is a service function that frees the context if the operation has
 *        failed. 
 *
 *        The function executes the following major steps:
 *
 *        1. Checks the validity of all of the inputs of the function. 
 *           If one of the received parameters is not valid it shall return an error.
 *
 *           The major checkers that are run over the received parameters:
 *           - verifying that the context pointer is not DX_NULL (*ContextID_ptr). 
 *        2. Clearing the users context.
 *        3. Exit the handler with the OK code.
 *      
 *
 * @param[in] ContextID_ptr - a pointer to the HMAC context buffer allocated by the user that
 *                       is used for the HMAC machine operation. this should be the same context that was
 *                       used on the previous call of this session.
 *
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* CRYS_HMAC_error.h
 */

CEXPORT_C CRYSError_t  CRYS_HMAC_Free(CRYS_HMACUserContext_t  *ContextID_ptr )
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
   
   RETURN_IF_HMAC_UNSUPPORTED( ContextID_ptr , ContextID_ptr , ContextID_ptr , 
                               ContextID_ptr , ContextID_ptr , ContextID_ptr , 
                               ContextID_ptr , ContextID_ptr , Error , Error ); 
                              
   #ifndef CRYS_NO_HASH_SUPPORT                                      
  
   /* ............... checking the parameters validity ................... */
   /* -------------------------------------------------------------------- */

   /* if the users context ID pointer is DX_NULL return an error */
   if( ContextID_ptr == DX_NULL )
   
      return CRYS_HMAC_INVALID_USER_CONTEXT_POINTER_ERROR;
   
   /* if the users context TAG is illegal return an error - the context is invalid */
   if( ContextID_ptr->valid_tag != HMAC_CONTEXT_VALIDATION_TAG )
   
      return CRYS_HMAC_USER_CONTEXT_CORRUPTED_ERROR;
   
   /* .............. clearing the users context .......................... */
   /* -------------------------------------------------------------------- */
   
   DX_VOS_MemSet( ContextID_ptr , 0 , sizeof(CRYS_HMACUserContext_t) );   

   /* ................. end of function ..................................... */
   /* ----------------------------------------------------------------------- */
       
   return Error;   
 
   #endif /* !CRYS_NO_HASH_SUPPORT */

}/* END OF CRYS_HMAC_Free */

/**
 * This function provide HASH function to process one buffer of data.
 * The function allocates an internal HASH Context , it initializes the 
 * HASH Context with the cryptographic attributes that are needed for 
 * the HASH block operation ( initialize H's value for the HASH algorithm ).
 * Then the function loads the Hardware with the initializing values and after 
 * that process the data block using the hardware to do hash .
 * At the end the function return the message digest of the data buffer .
 *
 *
 * @param[in] OperationMode - The operation mode : MD5 or SHA1.
 *
 * @param[in] key_ptr - The pointer to the users key buffer.
 *
 * @oaram[in] keySize - The size of the received key.
 * 
 * @param[in] ContextID_ptr - a pointer to the HMAC context buffer allocated by the user that
 *                       is used for the HMAC machine operation.
 *
 * @param[in] DataIn_ptr - The pointer to the buffer of the input data to the HMAC. The pointer does 
 *                         not need to be aligned. On CSI input mode the pointer must be equal to
 *                         value (0xFFFFFFFC | DataInAlignment). 
 * 
 * @param[in] DataInSize - The size of the data to be hashed in bytes. On CSI data transfer mode the size must  
 *                         multiple of HASH_BLOCK_SIZE for used HASH mode.
 *
 * param[out] HashResultBuff - a pointer to the target buffer where the 
 *                      HMAC result stored in the context is loaded to.
 *
 * @return CRYSError_t on success the function returns CRYS_OK else non ZERO error.
 *
 */
CEXPORT_C CRYSError_t CRYS_HMAC( 
                         CRYS_HASH_OperationMode_t     OperationMode,
                         DxUint8_t                    *key_ptr,
                         DxUint16_t                    keySize,
                         DxUint8_t                    *DataIn_ptr,
                         DxUint32_t                    DataSize,
                         CRYS_HASH_Result_t            HmacResultBuff )
{                         
   /* FUNCTION DECLERATIONS */

   /* The return error identifier */
   CRYSError_t Error;
   
   /* a users context used to pass to all of the CRYS functions */
   CRYS_HMACUserContext_t  ContextID;

   /* FUNCTION LOGIC */
   
   /* ............... local initializations .............................. */
   /* -------------------------------------------------------------------- */
   
   /* initializing the Error to O.K */
   Error = CRYS_OK;

   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */   
   
   RETURN_IF_HMAC_UNSUPPORTED( OperationMode , key_ptr , keySize , 
                               DataIn_ptr , DataSize , HmacResultBuff[0] , 
                               ContextID.valid_tag , Error , Error , Error ); 
                              
   #ifndef CRYS_NO_HASH_SUPPORT                                      
   
   /* ............... calling the CRYS init function ...................... */
   /* --------------------------------------------------------------------- */
 
   Error = CRYS_HMAC_Init( &ContextID,
                           OperationMode,
                           key_ptr,
                           keySize);

   if( Error != CRYS_OK ) 
   
     goto End;                                    
 
   /* ............... calling the CRYS Update function .................... */
   /* --------------------------------------------------------------------- */
 
   Error = CRYS_HMAC_Update( &ContextID,
                             DataIn_ptr,
                             DataSize);

   if( Error != CRYS_OK ) 
   
     goto End;                                    
 
   /* .............. calling the CRYS Finish function ..................... */
   /* --------------------------------------------------------------------- */
   
   Error = CRYS_HMAC_Finish( &ContextID ,
                             HmacResultBuff );

       
   /* ................. end of function ..................................... */
   /* ----------------------------------------------------------------------- */
   
   End:
   

   /* call the free function - release the users context */
   CRYS_HMAC_Free( &ContextID );
   
   return Error;

   #endif /* !CRYS_NO_HASH_SUPPORT */
   
}/* END OF CRYS_HMAC */   

