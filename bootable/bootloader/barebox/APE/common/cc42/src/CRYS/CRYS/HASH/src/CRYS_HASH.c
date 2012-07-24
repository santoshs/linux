/***********************************************************************************  
 * Copyright 2009 © Discretix Technologies Ltd. 
 * This software is protected by copyright, international treaties and 
 * various patents. If the license governing the use of this Software 
 * allows copy or redistribution of this  software then any copy or 
 * reproduction of this Software must include this Copyright Notice 
 * as well as any other notices provided under such license. 
 ***********************************************************************************/

/************* Include Files ****************/

/* .............. CRYS level includes ................. */

#include "DX_VOS_Mem.h"

#include "CRYS_HASH_error.h"
#include "CRYS.h"
#include "CRYS_COMMON.h"
#include "CRYS_HASH_Local.h"
#include "CRYS_CCM.h"
#include "CRYS_COMMON_math.h"
#include "log_output.h"


/* .............. LLF level includes ................. */

#ifndef CRYS_NO_HASH_SUPPORT

#include "LLF_HASH.h"

#endif /* CRYS_NO_HASH_SUPPORT */
 
/************************ Defines ***************************************/

/* canceling the PC-lint warning:
   Use of goto is deprecated */
/*lint --e{801} */

/************************ Enums *****************************************/

/************************ macros ****************************************/

/************************ Global Data ***********************************/



/************* Private function prototype *******************************/

/************************ Public Functions ******************************/

/**
 * This function initializes the HASH machine on the CRYS level.
 *
 * This function allocates and initializes the HASH Context .
 * The function receives as input a pointer to store the context handle to HASH Context , 
 * it initializes the 
 * HASH Context with the cryptographic attributes that are needed for 
 * the HASH block operation ( initialize H's value for the HASH algorithm ).
 *
 * The function flow:
 *
 * 1) checking the validity of the arguments - returnes an error on an illegal argument case.
 * 2) Aquiring the working context from the CCM manager.
 * 3) Initializing the context with the parameters passed by the user and with the init values
 *    of the HASH.
 * 4) loading the user tag to the context.
 * 5) release the CCM context.
 * 
 * @param[in] ContextID_ptr - a pointer to the HASH context buffer allocated by the user that
 *                       is used for the HASH machine operation.
 *
 * @param[in] OperationMode - The operation mode : MD5 or SHA1.
 *
 * @return CRYSError_t on success the function returns CRYS_OK else non ZERO error.
 *      
 */
CEXPORT_C CRYSError_t CRYS_HASH_Init( CRYS_HASHUserContext_t     *ContextID_ptr,
                                      CRYS_HASH_OperationMode_t  OperationMode)
{
   /* FUNCTION DECLERATIONS */

   /* The return error identifier */
   CRYSError_t Error, ErrorCcm;
   
   /* defining a pointer to the active context allcated by the CCM */
   HASHContext_t *ccmWorkingContext_ptr;
  
   /* FUNCTION LOGIC */
   
   /* ............... local initializations .............................. */
   /* -------------------------------------------------------------------- */
   
   /* initializing the Error to O.K */
   Error = CRYS_OK;

   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */   

   RETURN_IF_HASH_UNSUPPORTED( ContextID_ptr , OperationMode , ccmWorkingContext_ptr , 
                               OperationMode , OperationMode , OperationMode , 
                               OperationMode , OperationMode , OperationMode , Error );
  //PRINTF("CRYS_HASH_Init function starts"); 
                              
   #ifndef CRYS_NO_HASH_SUPPORT                                      
      
   /* ............... checking the parameters validity ................... */
   /* -------------------------------------------------------------------- */
   
   /* if the users context ID pointer is DX_NULL return an error */
   if( ContextID_ptr == DX_NULL )
   
      return CRYS_HASH_INVALID_USER_CONTEXT_POINTER_ERROR;
      
   /* init the context valid_tag field to avoid warnings */
   ContextID_ptr->valid_tag = 0;

   /* check if the operation mode is legal */
   if( OperationMode >= CRYS_HASH_NumOfModes )
   
      return CRYS_HASH_ILLEGAL_OPERATION_MODE_ERROR;
      
 #ifdef LLF_HASH_SHA384_SHA512_NOT_SUPPORTED
   if( OperationMode == CRYS_HASH_SHA384_mode ||
       OperationMode == CRYS_HASH_SHA512_mode )
   
      return CRYS_HASH_ILLEGAL_OPERATION_MODE_ERROR; 
 #endif
 
#ifndef CRYS_NO_FIPS_SUPPORT
		/* check if we are in FIPS mode */
   if(DX_GLOBAL_FIPS_MODE & DX_CRYS_FIPS_MODE_STATE)
   {
   	/* check if there was a FIPS error */
		if(DX_GLOBAL_FIPS_MODE & DX_CRYS_FIPS_MODE_ERROR_STATE)
   	{
   		return CRYS_HASH_FIPS_MODE_FAIL_ERROR;
   	}
   	/* no check if the self test was already executed */
   	if( !(DX_GLOBAL_FIPS_SF_STATUS & CRYS_SELF_TEST_HASH_BIT) )
   	{
   		return CRYS_HASH_FIPS_MODE_SF_NOT_PERFORMED_ERROR;
   	}
   	/* check for CBC-MAC - not allowed by FIPS */
   	if(OperationMode == CRYS_HASH_MD5_mode)
   	{
   		return CRYS_HASH_FIPS_MODE_NOT_ALLOWED_ERROR;
   	}
   }
#endif     
                       
   /* ................. acquiring the HASH contest .......................... */
   /* ----------------------------------------------------------------------- */
   Error = CRYS_CCM_GetContext( ContextID_ptr,       				 /* the users context space - in */
                                (void **) &ccmWorkingContext_ptr,    /* the CCM context returned - out */
                                DX_HASH_SHA1_CONTEXT,                /* type of the context - one type for all SHA types */ 
                                AES_DONT_DECRYPT_CONTEXT);			 /* do not need to decrypt context in AES_block*/   
    
    if( Error != CRYS_OK ) 
   
     return Error;                                    
                             
   /* ................. loading the context .................................. */
   /* ------------------------------------------------------------------------ */
   
   /* loading the operation mode */     
   ccmWorkingContext_ptr->OperationMode      = OperationMode;
   
   /* initialize the number of bytes on the previous buffer data stored */
   ccmWorkingContext_ptr->NumOfBytesOnThePreviousUpdateBuff = 0; 
    
   
   /* loading the H init values according to the operation mode */
   switch(OperationMode)
   {
   		case CRYS_HASH_SHA1_mode:
   			
   			/* update the block size */
   			ccmWorkingContext_ptr->OperationModeBlockSizeInBytes = CRYS_HASH_BLOCK_SIZE_IN_BYTES;
   
   			/* the values for the low registers should be set to 0 */
   			ccmWorkingContext_ptr->HASH_Result[0] = CRYS_HASH_SHA1_INIT_0;
   			ccmWorkingContext_ptr->HASH_Result[1] = CRYS_HASH_SHA1_INIT_1; 
		    ccmWorkingContext_ptr->HASH_Result[2] = CRYS_HASH_SHA1_INIT_2;
		    ccmWorkingContext_ptr->HASH_Result[3] = CRYS_HASH_SHA1_INIT_3; 
		    ccmWorkingContext_ptr->HASH_Result[4] = CRYS_HASH_SHA1_INIT_4;
		    ccmWorkingContext_ptr->HASH_Result[5] = 0; 
		    ccmWorkingContext_ptr->HASH_Result[6] = 0;
		    ccmWorkingContext_ptr->HASH_Result[7] = 0; 
		    ccmWorkingContext_ptr->HASH_Result[8] = 0;
		    ccmWorkingContext_ptr->HASH_Result[9] = 0;
		    
		    /* setting the rest of the unused values to 0 */
			ccmWorkingContext_ptr->HASH_Result[10] = 0;
			ccmWorkingContext_ptr->HASH_Result[11] = 0;
			ccmWorkingContext_ptr->HASH_Result[12] = 0;
			ccmWorkingContext_ptr->HASH_Result[13] = 0;
			ccmWorkingContext_ptr->HASH_Result[14] = 0;
			ccmWorkingContext_ptr->HASH_Result[15] = 0;
		    
		    break;
		
		case CRYS_HASH_SHA224_mode:
   		
   			/* update the block size */
   			ccmWorkingContext_ptr->OperationModeBlockSizeInBytes = CRYS_HASH_BLOCK_SIZE_IN_BYTES;
   			
   			/* the values for the low registers should be set to 0 */
   			ccmWorkingContext_ptr->HASH_Result[0] = CRYS_HASH_SHA224_INIT_0;
   			ccmWorkingContext_ptr->HASH_Result[1] = CRYS_HASH_SHA224_INIT_1; 
		    ccmWorkingContext_ptr->HASH_Result[2] = CRYS_HASH_SHA224_INIT_2;
		    ccmWorkingContext_ptr->HASH_Result[3] = CRYS_HASH_SHA224_INIT_3; 
		    ccmWorkingContext_ptr->HASH_Result[4] = CRYS_HASH_SHA224_INIT_4;
		    ccmWorkingContext_ptr->HASH_Result[5] = CRYS_HASH_SHA224_INIT_5; 
		    ccmWorkingContext_ptr->HASH_Result[6] = CRYS_HASH_SHA224_INIT_6;
		    ccmWorkingContext_ptr->HASH_Result[7] = CRYS_HASH_SHA224_INIT_7; 
		    ccmWorkingContext_ptr->HASH_Result[8] = 0;
		    ccmWorkingContext_ptr->HASH_Result[9] = 0;
		    ccmWorkingContext_ptr->HASH_Result[10] = 0;
		    ccmWorkingContext_ptr->HASH_Result[11] = 0;
		    ccmWorkingContext_ptr->HASH_Result[12] = 0;
		    ccmWorkingContext_ptr->HASH_Result[13] = 0;
		    ccmWorkingContext_ptr->HASH_Result[14] = 0;
		    ccmWorkingContext_ptr->HASH_Result[15] = 0;
		    
		    break;
		    
   		case CRYS_HASH_SHA256_mode:
                       //PRINTF("Setting the HInit values");
   		
   			/* update the block size */
   			ccmWorkingContext_ptr->OperationModeBlockSizeInBytes = CRYS_HASH_BLOCK_SIZE_IN_BYTES;
   			
   			/* the values for the low registers should be set to 0 */
   			ccmWorkingContext_ptr->HASH_Result[0] = CRYS_HASH_SHA256_INIT_0;
   			ccmWorkingContext_ptr->HASH_Result[1] = CRYS_HASH_SHA256_INIT_1; 
		    ccmWorkingContext_ptr->HASH_Result[2] = CRYS_HASH_SHA256_INIT_2;
		    ccmWorkingContext_ptr->HASH_Result[3] = CRYS_HASH_SHA256_INIT_3; 
		    ccmWorkingContext_ptr->HASH_Result[4] = CRYS_HASH_SHA256_INIT_4;
		    ccmWorkingContext_ptr->HASH_Result[5] = CRYS_HASH_SHA256_INIT_5; 
		    ccmWorkingContext_ptr->HASH_Result[6] = CRYS_HASH_SHA256_INIT_6;
		    ccmWorkingContext_ptr->HASH_Result[7] = CRYS_HASH_SHA256_INIT_7; 
		    ccmWorkingContext_ptr->HASH_Result[8] = 0;
		    ccmWorkingContext_ptr->HASH_Result[9] = 0;
		    ccmWorkingContext_ptr->HASH_Result[10] = 0;
		    ccmWorkingContext_ptr->HASH_Result[11] = 0;
		    ccmWorkingContext_ptr->HASH_Result[12] = 0;
		    ccmWorkingContext_ptr->HASH_Result[13] = 0;
		    ccmWorkingContext_ptr->HASH_Result[14] = 0;
		    ccmWorkingContext_ptr->HASH_Result[15] = 0;
		    
		    break;
   		
   		case CRYS_HASH_SHA384_mode:
   		
   			/* update the block size */
   			ccmWorkingContext_ptr->OperationModeBlockSizeInBytes = CRYS_HASH_SHA2_BLOCK_SIZE_IN_BYTES;
   		#ifdef BIG__ENDIAN
	   		ccmWorkingContext_ptr->HASH_Result[0] = CRYS_HASH_SHA384_INIT_0_HI; 
			    ccmWorkingContext_ptr->HASH_Result[1] = CRYS_HASH_SHA384_INIT_0_LO;
			    ccmWorkingContext_ptr->HASH_Result[2] = CRYS_HASH_SHA384_INIT_1_HI; 
			    ccmWorkingContext_ptr->HASH_Result[3] = CRYS_HASH_SHA384_INIT_1_LO; 
			    ccmWorkingContext_ptr->HASH_Result[4] = CRYS_HASH_SHA384_INIT_2_HI; 
			    ccmWorkingContext_ptr->HASH_Result[5] = CRYS_HASH_SHA384_INIT_2_LO; 
			    ccmWorkingContext_ptr->HASH_Result[6] = CRYS_HASH_SHA384_INIT_3_HI; 
			    ccmWorkingContext_ptr->HASH_Result[7] = CRYS_HASH_SHA384_INIT_3_LO; 
			    ccmWorkingContext_ptr->HASH_Result[8] = CRYS_HASH_SHA384_INIT_4_HI; 
			    ccmWorkingContext_ptr->HASH_Result[9] = CRYS_HASH_SHA384_INIT_4_LO; 
			    ccmWorkingContext_ptr->HASH_Result[10] = CRYS_HASH_SHA384_INIT_5_HI; 
			    ccmWorkingContext_ptr->HASH_Result[11] = CRYS_HASH_SHA384_INIT_5_LO; 
			    ccmWorkingContext_ptr->HASH_Result[12] = CRYS_HASH_SHA384_INIT_6_HI; 
			    ccmWorkingContext_ptr->HASH_Result[13] = CRYS_HASH_SHA384_INIT_6_LO;
			    ccmWorkingContext_ptr->HASH_Result[14] = CRYS_HASH_SHA384_INIT_7_HI; 
			    ccmWorkingContext_ptr->HASH_Result[15] = CRYS_HASH_SHA384_INIT_7_LO;
		    #else
	   		 ccmWorkingContext_ptr->HASH_Result[0] = CRYS_HASH_SHA384_INIT_0_LO; 
			    ccmWorkingContext_ptr->HASH_Result[1] = CRYS_HASH_SHA384_INIT_0_HI;
			    ccmWorkingContext_ptr->HASH_Result[2] = CRYS_HASH_SHA384_INIT_1_LO; 
			    ccmWorkingContext_ptr->HASH_Result[3] = CRYS_HASH_SHA384_INIT_1_HI; 
			    ccmWorkingContext_ptr->HASH_Result[4] = CRYS_HASH_SHA384_INIT_2_LO; 
			    ccmWorkingContext_ptr->HASH_Result[5] = CRYS_HASH_SHA384_INIT_2_HI; 
			    ccmWorkingContext_ptr->HASH_Result[6] = CRYS_HASH_SHA384_INIT_3_LO; 
			    ccmWorkingContext_ptr->HASH_Result[7] = CRYS_HASH_SHA384_INIT_3_HI; 
			    ccmWorkingContext_ptr->HASH_Result[8] = CRYS_HASH_SHA384_INIT_4_LO; 
			    ccmWorkingContext_ptr->HASH_Result[9] = CRYS_HASH_SHA384_INIT_4_HI; 
			    ccmWorkingContext_ptr->HASH_Result[10] = CRYS_HASH_SHA384_INIT_5_LO; 
			    ccmWorkingContext_ptr->HASH_Result[11] = CRYS_HASH_SHA384_INIT_5_HI; 
			    ccmWorkingContext_ptr->HASH_Result[12] = CRYS_HASH_SHA384_INIT_6_LO; 
			    ccmWorkingContext_ptr->HASH_Result[13] = CRYS_HASH_SHA384_INIT_6_HI;
			    ccmWorkingContext_ptr->HASH_Result[14] = CRYS_HASH_SHA384_INIT_7_LO; 
			    ccmWorkingContext_ptr->HASH_Result[15] = CRYS_HASH_SHA384_INIT_7_HI;
		    #endif
		    break;  
		    
   		case CRYS_HASH_SHA512_mode:
   		
   			/* update the block size */
   			ccmWorkingContext_ptr->OperationModeBlockSizeInBytes = CRYS_HASH_SHA2_BLOCK_SIZE_IN_BYTES;
   		#ifdef BIG__ENDIAN
	   		 ccmWorkingContext_ptr->HASH_Result[0] = CRYS_HASH_SHA512_INIT_0_HI; 
			    ccmWorkingContext_ptr->HASH_Result[1] = CRYS_HASH_SHA512_INIT_0_LO;
			    ccmWorkingContext_ptr->HASH_Result[2] = CRYS_HASH_SHA512_INIT_1_HI; 
			    ccmWorkingContext_ptr->HASH_Result[3] = CRYS_HASH_SHA512_INIT_1_LO; 
			    ccmWorkingContext_ptr->HASH_Result[4] = CRYS_HASH_SHA512_INIT_2_HI; 
			    ccmWorkingContext_ptr->HASH_Result[5] = CRYS_HASH_SHA512_INIT_2_LO; 
			    ccmWorkingContext_ptr->HASH_Result[6] = CRYS_HASH_SHA512_INIT_3_HI; 
			    ccmWorkingContext_ptr->HASH_Result[7] = CRYS_HASH_SHA512_INIT_3_LO; 
			    ccmWorkingContext_ptr->HASH_Result[8] = CRYS_HASH_SHA512_INIT_4_HI; 
			    ccmWorkingContext_ptr->HASH_Result[9] = CRYS_HASH_SHA512_INIT_4_LO; 
			    ccmWorkingContext_ptr->HASH_Result[10] = CRYS_HASH_SHA512_INIT_5_HI; 
			    ccmWorkingContext_ptr->HASH_Result[11] = CRYS_HASH_SHA512_INIT_5_LO; 
			    ccmWorkingContext_ptr->HASH_Result[12] = CRYS_HASH_SHA512_INIT_6_HI; 
			    ccmWorkingContext_ptr->HASH_Result[13] = CRYS_HASH_SHA512_INIT_6_LO;
			    ccmWorkingContext_ptr->HASH_Result[14] = CRYS_HASH_SHA512_INIT_7_HI; 
			    ccmWorkingContext_ptr->HASH_Result[15] = CRYS_HASH_SHA512_INIT_7_LO;
		    
		    #else
			    ccmWorkingContext_ptr->HASH_Result[0] = CRYS_HASH_SHA512_INIT_0_LO; 
			    ccmWorkingContext_ptr->HASH_Result[1] = CRYS_HASH_SHA512_INIT_0_HI;
			    ccmWorkingContext_ptr->HASH_Result[2] = CRYS_HASH_SHA512_INIT_1_LO; 
			    ccmWorkingContext_ptr->HASH_Result[3] = CRYS_HASH_SHA512_INIT_1_HI; 
			    ccmWorkingContext_ptr->HASH_Result[4] = CRYS_HASH_SHA512_INIT_2_LO; 
			    ccmWorkingContext_ptr->HASH_Result[5] = CRYS_HASH_SHA512_INIT_2_HI; 
			    ccmWorkingContext_ptr->HASH_Result[6] = CRYS_HASH_SHA512_INIT_3_LO; 
			    ccmWorkingContext_ptr->HASH_Result[7] = CRYS_HASH_SHA512_INIT_3_HI; 
			    ccmWorkingContext_ptr->HASH_Result[8] = CRYS_HASH_SHA512_INIT_4_LO; 
			    ccmWorkingContext_ptr->HASH_Result[9] = CRYS_HASH_SHA512_INIT_4_HI; 
			    ccmWorkingContext_ptr->HASH_Result[10] = CRYS_HASH_SHA512_INIT_5_LO; 
			    ccmWorkingContext_ptr->HASH_Result[11] = CRYS_HASH_SHA512_INIT_5_HI; 
			    ccmWorkingContext_ptr->HASH_Result[12] = CRYS_HASH_SHA512_INIT_6_LO; 
			    ccmWorkingContext_ptr->HASH_Result[13] = CRYS_HASH_SHA512_INIT_6_HI;
			    ccmWorkingContext_ptr->HASH_Result[14] = CRYS_HASH_SHA512_INIT_7_LO; 
			    ccmWorkingContext_ptr->HASH_Result[15] = CRYS_HASH_SHA512_INIT_7_HI;
		    #endif
		    
		    break;
		    
   		case CRYS_HASH_MD5_mode:
   			
		    /* update the block size */
   			ccmWorkingContext_ptr->OperationModeBlockSizeInBytes = CRYS_HASH_BLOCK_SIZE_IN_BYTES;
   		
   			ccmWorkingContext_ptr->HASH_Result[0] = CRYS_HASH_MD5_INIT_0; 
		    ccmWorkingContext_ptr->HASH_Result[1] = CRYS_HASH_MD5_INIT_1;
		    ccmWorkingContext_ptr->HASH_Result[2] = CRYS_HASH_MD5_INIT_2; 
		    ccmWorkingContext_ptr->HASH_Result[3] = CRYS_HASH_MD5_INIT_3;
		    ccmWorkingContext_ptr->HASH_Result[4] = CRYS_HASH_MD5_INIT_4; 
		    ccmWorkingContext_ptr->HASH_Result[5] = 0; 
		    ccmWorkingContext_ptr->HASH_Result[6] = 0; 
		    ccmWorkingContext_ptr->HASH_Result[7] = 0; 
		    ccmWorkingContext_ptr->HASH_Result[8] = 0; 
		    ccmWorkingContext_ptr->HASH_Result[9] = 0; 
		    ccmWorkingContext_ptr->HASH_Result[10] = 0; 
		    ccmWorkingContext_ptr->HASH_Result[11] = 0; 
		    ccmWorkingContext_ptr->HASH_Result[12] = 0; 
		    ccmWorkingContext_ptr->HASH_Result[13] = 0;
		    ccmWorkingContext_ptr->HASH_Result[14] = 0; 
		    ccmWorkingContext_ptr->HASH_Result[15] = 0;
		    
		    break;
		    
		default:
		    break;
   }
   
   /* set the HASH tag to the users context */
   ContextID_ptr->valid_tag = HASH_CONTEXT_VALIDATION_TAG;
   
   
   /* perfrom the initialization of the low level context */
   Error = LLF_HASH_InitContext( ccmWorkingContext_ptr );      /* the working context - in */   
                            	
   
   /* ................. release the context ................................. */
   /* ----------------------------------------------------------------------- */
   
   ErrorCcm = CRYS_CCM_ReleaseContext(ContextID_ptr,				/* the users context space - in */
                           ccmWorkingContext_ptr,       /* the CCM context returned - in */
                           DX_HASH_SHA1_CONTEXT);        /* HASH type - in */ 
                                   
    if((ErrorCcm != CRYS_OK) && (Error == CRYS_OK))   
   		Error = ErrorCcm;
  /* ................. end of function ..................................... */
   /* ----------------------------------------------------------------------- */

   return Error;

   #endif /*!CRYS_NO_HASH_SUPPORT*/

}/* END OF CRYS_HASH_Init */                           

/**
 * This function process a block of data via the HASH Hardware.
 * The function receives as input an handle to the  HASH Context, that was initialized before
 * by an CRYS_HASH_Init function or by other CRYS_HASH_Update function. \n
 * The function Sets the hardware with the last H's value that where stored in
 * the CRYS HASH context and then process the data block using the hardware 
 * and in the end of the process stores in the HASH context the H's value 
 * HASH Context with the cryptographic attributes that are needed for 
 * the HASH block operation ( initialize H's value for the HASH algorithm ).
 * This function is used in cases not all the data is arrange in one 
 * continues buffer . \n
 *
 * The function flow:
 *
 * 1) checking the parameters validty if there is an error the function shall exit with an error code. 
 * 2) Aquiring the working context from the CCM manager.
 * 3) If there isnt enouth data in the previous update data buff in the context plus the received data
 *    load it to the context buffer and exit the function.
 * 4) fill the previous update data buffer to contain an entire block. 
 * 5) Calling the hardware low level function to execute the update.
 * 6) fill the previous update data buffer with the data not processed at the end of the received data.
 * 7) release the CCM context.
 * 
 * @param[in] ContextID_ptr - a pointer to the HASH context buffer allocated by the user that
 *                       is used for the HASH machine operation.
 *
 * @param[in] DataIn_ptr - The pointer to the buffer of the input data to the HASH. The pointer does 
 *                         not need to be aligned. On CSI input mode the pointer must be equal to
 *                         value (0xFFFFFFFC | DataInAlignment). 
 * 
 * @param[in] DataInSize - The size of the data to be hashed in bytes. On CSI data transfer mode the size must  
 *                         multiple of HASH_BLOCK_SIZE for used HASH mode.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* CRYS_HASH_error.h
 *
 */

CEXPORT_C CRYSError_t CRYS_HASH_Update(CRYS_HASHUserContext_t  *ContextID_ptr,
                                       DxUint8_t               *DataIn_ptr,
                                       DxUint32_t               DataInSize )
{                                              
   /* FUNCTION DECLARATIONS */

   /* The return error identifier */
   CRYSError_t Error,ErrorCcm;
   
   /* defining a pointer to the active context allcated by the CCM */
   HASHContext_t *ccmWorkingContext_ptr;
   
   /* a byte pointer to point on the previous update data stored in the context */
   DxUint8_t *DataFromThePreviousUpdateBuff_8bit_ptr;
                     
   /* FUNCTION LOGIC */
  
   /* ............... local initializations .............................. */
   /* -------------------------------------------------------------------- */
   
   /* initializing the Error to O.K */
   Error = CRYS_OK;
 
   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */   

   RETURN_IF_HASH_UNSUPPORTED( ContextID_ptr , DataIn_ptr , DataInSize , 
                               ccmWorkingContext_ptr , DataFromThePreviousUpdateBuff_8bit_ptr ,  
                               ContextID_ptr , Error , Error , Error , Error );
   
  //PRINTF("CRYS_HASH_Update function starts"); 
                              
   #ifndef CRYS_NO_HASH_SUPPORT                                      
      
   /* ............... checking the parameters validity ................... */
   /* -------------------------------------------------------------------- */

   /* if the users context ID pointer is DX_NULL return an error */
   if( ContextID_ptr == DX_NULL )
   
      return CRYS_HASH_INVALID_USER_CONTEXT_POINTER_ERROR; 
   
   /* if the users context TAG is illegal return an error - the context is invalid */
   if( ContextID_ptr->valid_tag != HASH_CONTEXT_VALIDATION_TAG )
   
      return CRYS_HASH_USER_CONTEXT_CORRUPTED_ERROR;   

   /* if the data size is zero return CRYS_OK - we just do not need to process any data */
   if( DataInSize == 0 )    
   
      return CRYS_OK;
   
   /* if the users Data In pointer is illegal and the data size is larger then 0 return an error */
   if( DataIn_ptr == DX_NULL )
 
      return CRYS_HASH_DATA_IN_POINTER_INVALID_ERROR;
           
   /* larger then 2^29 (to prevant an overflow on the transition to bits ) 
      return error */
   if( DataInSize >= (1 << 29) )
  
      return CRYS_HASH_DATA_SIZE_ILLEGAL;
      
       
                   
   /* ................. acquiring the HASH contest .......................... */
   /* ----------------------------------------------------------------------- */
   
   Error = CRYS_CCM_GetContext( ContextID_ptr,       				 /* the users context space - in */
                                (void **) &ccmWorkingContext_ptr,    /* the CCM context returned - out */
                                DX_HASH_SHA1_CONTEXT,                       /* AES type - in */ 
                                AES_DECRYPT_CONTEXT);				 /* need to decrypt context in AES_block*/   
   
   
   if( Error != CRYS_OK )         
   
     return Error; 
     
    
   /* If data input is not CSI, the save the HASH-block of data in the context */
   if( (DxUint32_t)DataIn_ptr < (DxUint32_t)DX_CSI )
   {
	   /* .... checking if there is enough data to start the operation ........... */
	   /* ------------------------------------------------------------------------ */
	   
	   /* init the pointer on the previous update buffer to the current position on the buffer */
	   DataFromThePreviousUpdateBuff_8bit_ptr = 
	      (DxUint8_t*)ccmWorkingContext_ptr->DataFromThePreviousUpdateBuff +
	      ccmWorkingContext_ptr->NumOfBytesOnThePreviousUpdateBuff;
	   
	   /* if the size of the data stored in the previous update data plus the 
	      new data is not at a packet size - load it to the buffer , release the context and exit the function */
	   if( ccmWorkingContext_ptr->NumOfBytesOnThePreviousUpdateBuff + DataInSize < ccmWorkingContext_ptr->OperationModeBlockSizeInBytes )
	   {
	      /* loading the buffer */
	      DX_VOS_FastMemCpy( 
	        DataFromThePreviousUpdateBuff_8bit_ptr,
	        DataIn_ptr,
	        DataInSize );
	        
	      /* update the number of bytes stored in the previous update buffer */
	      ccmWorkingContext_ptr->NumOfBytesOnThePreviousUpdateBuff += (DxUint16_t)DataInSize;
	       
	      goto End;
	       
	   }/* end of the case we do not have a packet */
	   
	   /* ...... fill the previous update buffer. this will be the first block we will loaded 
	             from this buffer ................................................................ */
	   /* ---------------------------------------------------------------------------------------- */    
	   
	   DX_VOS_FastMemCpy( 
	      DataFromThePreviousUpdateBuff_8bit_ptr,
	      DataIn_ptr,
	      ccmWorkingContext_ptr->OperationModeBlockSizeInBytes - ccmWorkingContext_ptr->NumOfBytesOnThePreviousUpdateBuff );
	      
	   /* update the input data pointer to the new position */
	   DataIn_ptr += (ccmWorkingContext_ptr->OperationModeBlockSizeInBytes - ccmWorkingContext_ptr->NumOfBytesOnThePreviousUpdateBuff);        
	   
	   /* update the data size to the new value ( the data that is not stored in the block ) */
	   DataInSize -= (ccmWorkingContext_ptr->OperationModeBlockSizeInBytes - ccmWorkingContext_ptr->NumOfBytesOnThePreviousUpdateBuff);
	    
	   /* set the number of bytes in the previous update buffers to the block size */
	   ccmWorkingContext_ptr->NumOfBytesOnThePreviousUpdateBuff = ccmWorkingContext_ptr->OperationModeBlockSizeInBytes;       
    }
    
    
    else  /* if data input mode is CSI */
    {
        /* check that data size is a multiple of HASH_BLOCK_SIZE */
        if( (DataInSize & (ccmWorkingContext_ptr->OperationModeBlockSizeInBytes - 1)) != 0 )
        {
	     	Error = CRYS_HASH_DATA_SIZE_IS_ILLEGAL_FOR_CSI_ERROR;
	    	
	    	goto End;        	
        }
        
        /* set the DataSize on prev buffer = 0 */
        ccmWorkingContext_ptr->NumOfBytesOnThePreviousUpdateBuff = 0;
    }
    
                                                                                         
   /* .................. calling the hardware low level block function ....... */
   /* ------------------------------------------------------------------------ */
   
   Error = LLF_HASH_Update( ccmWorkingContext_ptr,      /* the working context - in */   
                            DataIn_ptr,                 /* the input data buffer - in */                  
                            DataInSize );               /* the data in size - in */      
   //PRINTF("LLF_HASH_Update ends\n\n");

   if( Error != CRYS_OK ) 
   
     goto End; 
      
                                       
   /* If data input is not CSI, then saves the remaining data in the context */
   if( (DxUint32_t)DataIn_ptr < (DxUint32_t)DX_CSI )
   {
	   /* ..... fill the previous update buffer with the end of the data that cant be loaded to a block ... */
	   /* ------------------------------------------------------------------------------------------------- */
	   
	   /* set the data size to be loaded as the size of the data not processed */
	   ccmWorkingContext_ptr->NumOfBytesOnThePreviousUpdateBuff = (DxUint16_t)(DataInSize & (ccmWorkingContext_ptr->OperationModeBlockSizeInBytes -1));

	   /* set the data input pointer to the new position */
	   DataIn_ptr = DataIn_ptr + DataInSize - ccmWorkingContext_ptr->NumOfBytesOnThePreviousUpdateBuff;
	   
	   /* copy the data to the context */
	   DX_VOS_FastMemCpy( ccmWorkingContext_ptr->DataFromThePreviousUpdateBuff,
	                      DataIn_ptr,
	                      ccmWorkingContext_ptr->NumOfBytesOnThePreviousUpdateBuff);
   }

 
   /* ................. release the context and exit the function ........... */
   /* ----------------------------------------------------------------------- */
   
   End:
      
   ErrorCcm = CRYS_CCM_ReleaseContext(ContextID_ptr,               /* the users context space - in */
                           ccmWorkingContext_ptr,       /* the CCM context returned - in */
                           DX_HASH_SHA1_CONTEXT);        /* HASH type - in */ 
                                      
    if((ErrorCcm != CRYS_OK) && (Error == CRYS_OK))   
   		Error = ErrorCcm;

    return Error;
   
   #endif /* !CRYS_NO_HASH_SUPPORT */   
 
}/* END OF CRYS_HASH_Update */ 

/**
 * This function finalize the hashing process of data block.
 * The function receives as input an handle to the HASH Context , that was initialized before
 * by an CRYS_HASH_Init function or by CRYS_HASH_Update function.
 * The function "adds" an header to the data block as the specific hash standard 
 * specifics , then it loads the hardware and reads the final message digest.
 *
 *  the function flow:
 *
 * 1) checking the parameters validty if there is an error the function shall exit with an error code. 
 * 2) Aquiring the working context from the CCM manager.
 * 3) Setting the padding buffer to load.
 * 4) Calling the hardware low level function to execute the finish.
 * 5) fill the previous update data buffer with the data not processed at the end of the received data.
 * 6) release the CCM context.
 *
 *  @param[in] ContextID_ptr - a pointer to the HASH context buffer allocated by the user that
 *                       is used for the HASH machine operation.
 *
 *  @retval HashResultBuff a pointer to the target buffer where the 
 *                       HASE result stored in the context is loaded to.
 *
 *  @return CRYSError_t on success the function returns CRYS_OK else non ZERO error.
 */

CEXPORT_C CRYSError_t CRYS_HASH_Finish( CRYS_HASHUserContext_t  *ContextID_ptr ,
                                        CRYS_HASH_Result_t       HashResultBuff )
{  
   /* FUNCTION DECLERATIONS */

   /* The return error identifier */
   CRYSError_t Error,ErrorCcm;
   
   /* defining a pointer to the active context allcated by the CCM */
   HASHContext_t *ccmWorkingContext_ptr;


   /* FUNCTION LOGIC */
                            
   /* ............... local initializations .............................. */
   /* -------------------------------------------------------------------- */
   
   /* initializing the Error to O.K */
   Error = CRYS_OK;

  //PRINTF("CRYS_HASH_Finish function starts\n\n");

   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */   

   RETURN_IF_HASH_UNSUPPORTED( ContextID_ptr , HashResultBuff[0] , ccmWorkingContext_ptr , 
                               Error , Error , Error , Error , Error , Error , Error ); 
                              
   #ifndef CRYS_NO_HASH_SUPPORT                                      
  
   /* ............... checking the parameters validity ................... */
   /* -------------------------------------------------------------------- */

   /* if the users context ID pointer is DX_NULL return an error */
   if( ContextID_ptr == DX_NULL )
   
      return CRYS_HASH_INVALID_USER_CONTEXT_POINTER_ERROR;
      
   /* if the result buffer DX_NULL return an error */
   if( HashResultBuff == DX_NULL )
   
      return CRYS_HASH_INVALID_RESULT_BUFFER_POINTER_ERROR;
           
   /* if the users context TAG is illegal return an error - the context is invalid */
   if( ContextID_ptr->valid_tag != HASH_CONTEXT_VALIDATION_TAG )
   
      return CRYS_HASH_USER_CONTEXT_CORRUPTED_ERROR;
  
   /* ................. aquiring the HASH contest ........................... */
   /* ----------------------------------------------------------------------- */
   Error = CRYS_CCM_GetContext( ContextID_ptr,       				 /* the users context space - in */
                                (void **) &ccmWorkingContext_ptr,    /* the CCM context returned - out */
                                DX_HASH_SHA1_CONTEXT,                       /* AES type - in */ 
                                AES_DECRYPT_CONTEXT);				 /* need to decrypt context in AES_block*/   
      
   if( Error != CRYS_OK )         
   
     return Error;
   
   
   
   /* ................. calling the low level finish function - to load the data that is left ..... */
   /* --------------------------------------------------------------------------------------------- */
   
   Error = LLF_HASH_Finish( ccmWorkingContext_ptr );      
   //PRINTF("LLF_HASH_Finish function completed\n");                                     
   
   if( Error != CRYS_OK ) 
   
     goto End;                                    
 
   /* ................. loading the result stored in the context to the 
                        output buffer .................................... */
   /* -------------------------------------------------------------------- */
   
   DX_VOS_FastMemCpy( HashResultBuff , ccmWorkingContext_ptr->HASH_Result , sizeof(CRYS_HASH_Result_t) );
   
   /* ................. release the context ................................. */
   /* ----------------------------------------------------------------------- */
   
   End:
   
   ErrorCcm = CRYS_CCM_ReleaseContext(ContextID_ptr,				/* the users context space - in */
                           ccmWorkingContext_ptr,       /* the CCM context returned - in */
                           DX_HASH_SHA1_CONTEXT);        /* HASH type - in */ 
                                    
   /* .............. clearing the users context .......................... */
   /* -------------------------------------------------------------------- */
    if((ErrorCcm != CRYS_OK) && (Error == CRYS_OK))   
   		Error = ErrorCcm;
   
   DX_VOS_MemSet( ContextID_ptr , 0 , sizeof(CRYS_HASHUserContext_t) );   
   
   return Error;   
 
   #endif /* !CRYS_NO_HASH_SUPPORT*/

}/* END OF CRYS_HASH_Finish */

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
 * @param[in] ContextID_ptr - a pointer to the HASH context buffer allocated by the user that
 *                       is used for the HASH machine operation. this should be the same context that was
 *                       used on the previous call of this session.
 *
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* CRYS_HASH_error.h
 */
CEXPORT_C CRYSError_t  CRYS_HASH_Free(CRYS_HASHUserContext_t  *ContextID_ptr )
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

   RETURN_IF_HASH_UNSUPPORTED( ContextID_ptr , ContextID_ptr , ContextID_ptr , ContextID_ptr,
                               ContextID_ptr , ContextID_ptr , ContextID_ptr , ContextID_ptr ,
                               ContextID_ptr , Error ); 
                              
   #ifndef CRYS_NO_HASH_SUPPORT                                      
  
   /* ............... checking the parameters validity ................... */
   /* -------------------------------------------------------------------- */

   /* if the users context ID pointer is DX_NULL return an error */
   if( ContextID_ptr == DX_NULL )
   
      return CRYS_HASH_INVALID_USER_CONTEXT_POINTER_ERROR;
   
   /* if the users context TAG is illegal return an error - the context is invalid */
   if( ContextID_ptr->valid_tag != HASH_CONTEXT_VALIDATION_TAG )
   
      return CRYS_HASH_USER_CONTEXT_CORRUPTED_ERROR;
   
   /* .............. clearing the users context .......................... */
   /* -------------------------------------------------------------------- */
   
   DX_VOS_MemSet( ContextID_ptr , 0 , sizeof(CRYS_HASHUserContext_t) );   

   /* ................. end of function ..................................... */
   /* ----------------------------------------------------------------------- */
       
   return Error;
   
   #endif /* !CRYS_NO_HASH_SUPPORT */   
 
}/* END OF CRYS_HASH_Free */
                     
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
 * @param[in] DataIn_ptr - The pointer to the buffer of the input data to the HASH. The pointer does 
 *                         not need to be aligned. On CSI input mode the pointer must be equal to
 *                         value (0xFFFFFFFC | DataInAlignment). 
 * 
 * @param[in] DataInSize - The size of the data to be hashed in bytes. On CSI data transfer mode the size must  
 *                         multiple of HASH_BLOCK_SIZE for used HASH mode.
 *
 * @retval HashResultBuff a pointer to the target buffer where the 
 *                      HASE result stored in the context is loaded to.
 *
 * @return CRYSError_t on success the function returns CRYS_OK else non ZERO error.
 *
 */
CEXPORT_C CRYSError_t CRYS_HASH( CRYS_HASH_OperationMode_t  OperationMode,
                                 DxUint8_t                 *DataIn_ptr,
                                 DxUint32_t                 DataSize,
                                 CRYS_HASH_Result_t         HashResultBuff )
{    
   /* FUNCTION DECLERATIONS */

   /* The return error identifier */
   CRYSError_t Error;
   
   /* a users context used to pass to all of the CRYS functions */
   CRYS_HASHUserContext_t  ContextID;

   /* FUNCTION LOGIC */
   
   /* ............... local initializations .............................. */
   /* -------------------------------------------------------------------- */
   
   /* initializing the Error to O.K */
   Error = CRYS_OK;

   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */   

   RETURN_IF_HASH_UNSUPPORTED( OperationMode , DataIn_ptr , DataSize , HashResultBuff[0],
                               ContextID.valid_tag , Error , Error , Error , Error , Error ); 
                              
   #ifndef CRYS_NO_HASH_SUPPORT                                      
   
   /* ............... calling the CRYS init function ...................... */
   /* --------------------------------------------------------------------- */
 
   Error = CRYS_HASH_Init( &ContextID,
                           OperationMode);
  //PRINTF("CRYS_HASH_Init function ends");

   if( Error != CRYS_OK ) 
   
     goto End;                                    
 
   /* ............... calling the CRYS Update function .................... */
   /* --------------------------------------------------------------------- */
 
   Error = CRYS_HASH_Update( &ContextID,
                             DataIn_ptr,
                             DataSize);
   //PRINTF("CRYS_HASH_Update function ends");


   if( Error != CRYS_OK ) 
   
     goto End;                                    
 
   /* .............. calling the CRYS Finish function ..................... */
   /* --------------------------------------------------------------------- */
   
   Error = CRYS_HASH_Finish( &ContextID ,
                             HashResultBuff );

   //PRINTF("CRYS_HASH_Finish function ends");

   /* ................. end of function ..................................... */
   /* ----------------------------------------------------------------------- */
   
   End:
   
   /* call the free function - release the users context */
   CRYS_HASH_Free( &ContextID );
   
   return Error;
   
   #endif /* !CRYS_NO_HASH_SUPPORT */
   
}/* END OF CRYS_HASH */   
