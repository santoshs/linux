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
#include "CRYS.h"
#include "CRYS_COMMON.h"
#include "CRYS_COMMON_math.h"
#include "CRYS_ECPKI_error.h"
#include "CRYS_ECPKI_Local.h"

/* .............. LLF level includes and definitions.............. */
#include "LLF_ECPKI_Export.h"

/* canceling the lint warning:
   Info 717: do ... while(0) */
/*lint --e{717} */

/* canceling the lint warning:
   Use of goto is deprecated */
/*lint --e{801} */

/* canceling the lint warning:
Info 716: while(1) ... */ 
/*lint --e{716} */

/************************ Defines ***************************************/

/* canceling the lint warning:
   Unusual pointer cast (incompatible indirect types) */
/*lint --e{740} */

/************************ Enums *****************************************/

/************************ Typedefs **************************************/

/************************ Global Data ***********************************/

/************* Private function prototype *******************************/

/************************ Public Functions ******************************/


/**********************************************************************************
 *                    CRYS_ECPKI_BuildPrivKey function                            *
 **********************************************************************************/
/*!\brief 	Builds (imports) the user private key structure from an existing private key
            so that this structure can be used by other EC primitives.

            When operating the EC cryptographic operations with existing and saved 
            EC private keys, this function should be called first.

		    The function performs the following operations:
			  - Checks validity of incoming variables and pointers.
			  - Checks, that 0 < PrivKey < r (r - EC generator order).
			  - Converts incoming key data from big endian into little endian form.
			  - Initializes variables and structures. 
   
   @param[in]  DomainID           The enumerator variable defines current EC domain.
   @param[in]  PrivKeyIn_ptr      Pointer to private key data. 
   @param[in]  PrivKeySizeInBytes Size of private key data in bytes. Must be great than null and
                                  less or equall to EC OrderSizeInBytes.
   @param[out] UserPrivKey_ptr    Pointer to the private key structure. 
                                  This structure is used as input to the ECPKI 
                                  cryptographic primitives.
   @return   CRYSError_t: 
			 CRYS_OK
             CRYS_ECPKI_BUILD_KEY_INVALID_PRIV_KEY_IN_PTR_ERROR
			 CRYS_ECPKI_BUILD_KEY_INVALID_USER_PRIV_KEY_PTR_ERROR
			 CRYS_ECPKI_BUILD_KEY_ILLEGAL_DOMAIN_ID_ERROR			 
*/	
CEXPORT_C CRYSError_t CRYS_ECPKI_BuildPrivKey(
			    CRYS_ECPKI_DomainID_t      DomainID,	      /*in */   
				DxUint8_t			      *PrivKeyIn_ptr,     /*in*/
				DxUint32_t                 PrivKeySizeInBytes,/*in*/
				CRYS_ECPKI_UserPrivKey_t  *UserPrivKey_ptr    /*out*/ ) 
 {
 	   /* FUNCTION DECLARATIONS */
   
   /* the private key structure pointer */
   CRYS_ECPKI_PrivKey_t *PrivKey_ptr;

   /*  EC domain info structure and parameters */
   CRYS_ECPKI_DomainInfo_t  DomainInfo;  
   DxUint32_t  OrderSizeInBytes;

   /* the Error return code identifier */
   CRYSError_t Error; 
 

   /* FUNCTION LOGIC */
 
   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */   

   RETURN_IF_ECPKI_UNSUPPORTED( DomainID , PrivKeyIn_ptr , PrivKeySizeInBytes , 
                              UserPrivKey_ptr , PrivKey_ptr , DomainInfo.EC_ModulusSizeInBits ,                              
                              OrderSizeInBytes ,Error ,  Error , Error , Error , Error , Error , Error ,
                              Error , Error , Error , Error , Error , Error , Error ,Error ); 
                                   

#ifndef CRYS_NO_ECPKI_SUPPORT  
    
   /* Error initialization */
   Error = CRYS_OK;
  
   /* ................. checking the validity of the pointer arguments ....... */
   /* ------------------------------------------------------------------------ */
  
   /* ...... checking the key database handle pointer ....................     */
   if( PrivKeyIn_ptr == DX_NULL )   
	    return  CRYS_ECPKI_BUILD_KEY_INVALID_PRIV_KEY_IN_PTR_ERROR;   	   
         
   /* ...... checking the validity of the User Private Key pointer ........... */
   if( UserPrivKey_ptr == DX_NULL ) 
	   return  CRYS_ECPKI_BUILD_KEY_INVALID_USER_PRIV_KEY_PTR_ERROR;   		
 
   /* ...... checking the EC domain ID.................... */
   if( DomainID >= CRYS_ECPKI_DomainID_OffMode )
   		return  CRYS_ECPKI_BUILD_KEY_ILLEGAL_DOMAIN_ID_ERROR;  	

   /* Get EC Domain information from LLF level */
   Error = LLF_ECPKI_GetDomainInfo(DomainID, &DomainInfo);
   
   if( Error != CRYS_OK ) 
		 goto End;  

   /* EC order size in bytes */ 
   OrderSizeInBytes = (DomainInfo.EC_OrderSizeInBits + 7) / 8;

   if( PrivKeySizeInBytes == 0 || PrivKeySizeInBytes > OrderSizeInBytes)
   	    return  CRYS_ECPKI_BUILD_KEY_INVALID_PRIV_KEY_SIZE_ERROR; 
   
   /****************  FUNCTION LOGIC  **************************************/

   /* ...... copy the buffers to the key handle structure ................ */
   /* -------------------------------------------------------------------- */
   
   /* setting the pointer to the key database */
   PrivKey_ptr = (CRYS_ECPKI_PrivKey_t *)UserPrivKey_ptr->PrivKeyDbBuff;
  
   /* clear the private key db */
   DX_VOS_MemSet( PrivKey_ptr , 0 , sizeof(CRYS_ECPKI_PrivKey_t) );

   /* loading the private key db to little endian and domain ID */
   CRYS_COMMON_ReverseMemcpy( (DxUint8_t*)(PrivKey_ptr->PrivKey), 
                              PrivKeyIn_ptr, PrivKeySizeInBytes  );
                             
   PrivKey_ptr->DomainID = DomainID;                         
 
   /* partly check the validity of the private key */
   Error = LLF_ECPKI_CheckPrivKeySize(DomainID, (DxUint8_t*)(PrivKey_ptr->PrivKey), PrivKeySizeInBytes );
   
   if( Error != CRYS_OK )      
   		goto End;
   
   /* initialize LLF private key database */
   Error = LLF_ECPKI_InitPrivKeyDb( PrivKey_ptr);

   if( Error != CRYS_OK )      
   		goto End;
   
   /* ................. end of the function ................................. */
   /* ----------------------------------------------------------------------- */
   
 End:      
   /* if the created structure is not valid - clear it */
   if( Error != CRYS_OK )
   {
      DX_VOS_MemSet( UserPrivKey_ptr , 0 , sizeof(CRYS_ECPKI_UserPrivKey_t) ); 

         return Error;
   }

   /* ................ set the private key validation tag ................... */  
   UserPrivKey_ptr->valid_tag = CRYS_ECPKI_PRIV_KEY_VALIDATION_TAG;

   return Error;

#endif /* !CRYS_NO_ECPKI_SUPPORT */

 } /* End of CRYS_ECPKI_BuildPrivKey() */
 
 


/**********************************************************************************
 *	              _DX_ECPKI_BuildPublKey function                                *
 **********************************************************************************/
/**
   @brief 	The _DX_ECPKI_BuildPublKey function checks the validity and builds the user public 
            key structure from imported public key data for using  it in other ECC primitives. 

            When operating the EC cryptographic algorithms with imported EC public
            key, this function should be called before using of the public key.

			The user must to call this function by appropriate macros, according to necessary
			validation level [SEC1. ECC standard: 3.2]:
			     - full checking of public key - CRYS_ECPKI_BuildPublKeyFullCheck,
 			     - partly checking of public key - CRYS_ECPKI_BuildPublKeyPartCheck,
                 - checking the input pointers and sizes only - CRYS_ECPKI_BuildPublKeyo.

			The function performs the following operations:
			-	Checks validity of incoming variables and pointers;
			-   Converts incoming key data from big endian into little endian form
			    as follows:
			      - For WMDRM  the function reverts endianness of public key 
				    (EC point coordinates X,Y) and copies they into output aligned buffer;
                  - For other domains:  
				    * If public key is given in uncompressed form the function reverts 
					  endianness of key point coordinates X and Y separately and copies 
					  they in output buffer.
			        * If public key is given in compressed form, the function reverts 
					  endianness and converts the point to uncompressed form and outputs 
					  the key. 
		    -   according to CheckMode parameter performs full or partly checking of public 
			    key validaty by calling the LLF function.
			-   Initializes variables and structures. 

			Incoming public key data PublKeyIn is given in big endianness as follows :
			    - for WMDRM:   X||Y ,
				- for other domains (according to IEEE 1363-2000): 
				    * PC||X||Y - for uncompressed and for hybrid points,
					* PC||X    - for compressed point,  
				  where: X,Y - EC point coordinates of public key,  size of X and Y 
				               equal to size of EC modulus,  
						 PC - single byte point control, defines type of point,
						 Size of buffers for X and also Y must be equal ModSizeInBytes.

   			NOTE: At this stage the said compressed or hybrid forms are not implemented 
			      and the function returns an error in this case.			    

   @param[in]  ECPKI_DomainID  - The enumerator variable defines current EC domain.
   @param[in]  PublKeyIn_ptr   - The pointer to private key data.
   @param[in]  PublKeySizeInBytes - Size of private key data in bytes (according to EC domain),
               it should be equal to 2*modulusSize (CRYS_ECPKI_DomainID_WMDRM10) or 
			   2*modulusSize + 1byte (other domains).
   @param[in]  CheckMode       - The parameter defining what checking of public key is necessary:
                                 preliminary check - 0, partly check - 1, full check - 2 .
   @param[out] UserPublKey_ptr - A pointer to the private key structure.
   @param[in]  TempBuff_ptr    - A pointer to the temp buffer structure for build function.

   @return CRYSError_t - CRYS_OK,
                         CRYS_ECPKI_BUILD_KEY_INVALID_PUBL_KEY_IN_PTR_ERROR
						 CRYS_ECPKI_BUILD_KEY_INVALID_USER_PUBL_KEY_PTR_ERROR
						 CRYS_ECPKI_BUILD_KEY_ILLEGAL_DOMAIN_ID_ERROR
						 CRYS_ECPKI_BUILD_KEY_INVALID_PUBL_KEY_DATA_ERROR
						 CRYS_ECPKI_BUILD_KEY_INVALID_COMPRESSION_MODE_ERROR
*/
 CEXPORT_C CRYSError_t _DX_ECPKI_BuildPublKey(
									CRYS_ECPKI_DomainID_t        DomainID,	          /*in*/				
									DxUint8_t			        *PublKeyIn_ptr,       /*in*/									
									DxUint32_t                   PublKeySizeInBytes,  /*in*/
									EC_PublKeyCheckMode_t        CheckMode,           /*in*/
									CRYS_ECPKI_UserPublKey_t    *UserPublKey_ptr,     /*out*/
									CRYS_ECPKI_BUILD_TempData_t *TempBuff_ptr         /*in*/ )
 {
 	   /* FUNCTION DECLARATIONS */
     
   /* the private key structure pointer */
   CRYS_ECPKI_PublKey_t *PublKey_ptr;

   /* structure containing part of domain information */
   CRYS_ECPKI_DomainInfo_t   DomainInfo;
   
   /* EC modulus size in bytes*/
   DxUint32_t  ModSizeInBytes; 

   /* Point control PC ( 1 byte) PC and PC1 = PC&6*/
   DxUint8_t     PC, PC1;
   
   /* the Error return code identifier */
   CRYSError_t Error; 

   /* FUNCTION LOGIC */
 
   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */   

    RETURN_IF_ECPKI_UNSUPPORTED( DomainID, PublKeyIn_ptr, PublKeySizeInBytes, CheckMode, 
		                         UserPublKey_ptr, TempBuff_ptr, PublKey_ptr,                                 
                                 DomainInfo.EC_ModulusSizeInBits ,ModSizeInBytes , PC, PC1, 
                                 Error, Error, Error, Error, Error, Error, Error, 
                                 Error, Error, Error, Error );                               
                                  
#ifndef CRYS_NO_ECPKI_SUPPORT  

   /* ...... checking the validity of the User Private Key pointer ......... */
   if( UserPublKey_ptr == DX_NULL ) 
	   return  CRYS_ECPKI_BUILD_KEY_INVALID_USER_PUBL_KEY_PTR_ERROR;  

	/* ...... checking the key database handle pointer ....................  */
   if( PublKeyIn_ptr == DX_NULL )   
	    return  CRYS_ECPKI_BUILD_KEY_INVALID_PUBL_KEY_IN_PTR_ERROR;   	   
 
   /* ...... checking the EC domain ID.................... */
   if( DomainID >= CRYS_ECPKI_DomainID_OffMode )
   		return  CRYS_ECPKI_BUILD_KEY_ILLEGAL_DOMAIN_ID_ERROR;  

   if( CheckMode >= PublKeyChecingOffMode )
	    return  CRYS_ECPKI_BUILD_KEY_INVALID_CHECK_MODE_ERROR;
  
   if( CheckMode != CheckPointersAndSizesOnly && TempBuff_ptr == DX_NULL )
        return  CRYS_ECPKI_BUILD_KEY_INVALID_TEMP_BUFF_PTR_ERROR;


   /* ...... Initializations  ............... */ 
   
   Error = CRYS_OK;
   
   /* get domain info and modulus size */
   Error = LLF_ECPKI_GetDomainInfo(DomainID, &DomainInfo);

   if( Error != CRYS_OK ) 

	   return Error;

   ModSizeInBytes = (DomainInfo.EC_ModulusSizeInBits + 7)/8;
   
   /* point control */
   PC = PublKeyIn_ptr[0];
   PC1 = PC & 0x6;

   /* continue the parameters checking */
   if(DomainID != CRYS_ECPKI_DomainID_WMDRM10)
   {
		/* check that point control (compression mode) is legal */
		if( PC < CRYS_EC_PointCompressed || PC == CRYS_EC_PointContWrong || PC >= CRYS_EC_PointCompresOffMode )      
                return  CRYS_ECPKI_BUILD_KEY_INVALID_COMPRESSION_MODE_ERROR;
        
		/* preliminary check key size */
		if( (PC1 == CRYS_EC_PointUncompressed || PC1 == CRYS_EC_PointHybrid) &&
			 PublKeySizeInBytes != (2*ModSizeInBytes + 1) ) 
                return  CRYS_ECPKI_BUILD_KEY_INVALID_PUBL_KEY_DATA_ERROR;
			
		if( PC1 == CRYS_EC_PointCompressed && PublKeySizeInBytes != (ModSizeInBytes + 1) ) 
			    return  CRYS_ECPKI_BUILD_KEY_INVALID_PUBL_KEY_DATA_ERROR; 
   }
   else  /* for DomainID = CRYS_ECPKI_DomainID_WMDRM10 */
   {
		/* preliminary check key size */
        if( PublKeySizeInBytes != 2*ModSizeInBytes )
   			return  CRYS_ECPKI_BUILD_KEY_INVALID_PUBL_KEY_SIZE_ERROR; 
   }   

   /* ...... copy the buffers to the key handle structure ................ */
   /* -------------------------------------------------------------------- */
   
   /* setting the pointer to the key database */
   PublKey_ptr = (CRYS_ECPKI_PublKey_t *)UserPublKey_ptr->PublKeyDbBuff;
   
   /* clear the public key db */
   DX_VOS_MemSet( (DxUint8_t*)UserPublKey_ptr , 0 , sizeof(CRYS_ECPKI_UserPublKey_t) );
 
   /* Set DomainID into Builded public key */
   PublKey_ptr->DomainID = DomainID;

   /* loading the public key db to little endian  */
   if( DomainID == CRYS_ECPKI_DomainID_WMDRM10)
   {
	    /* for WMDRM reverse  copy Xin to X, Yin  toY */
     	CRYS_COMMON_ReverseMemcpy( (DxUint8_t*)PublKey_ptr->PublKeyX, 
									PublKeyIn_ptr, ModSizeInBytes );        
		CRYS_COMMON_ReverseMemcpy( (DxUint8_t*)PublKey_ptr->PublKeyY, 
									PublKeyIn_ptr + ModSizeInBytes, ModSizeInBytes );
   }
   else if ( PC1 == CRYS_EC_PointUncompressed || PC1 == CRYS_EC_PointHybrid )  /*  PC1 = 2 or PC1 = 6 */
   {
	   /* Reverse mem copy public key Xin to X, Yin to Y */
	   CRYS_COMMON_ReverseMemcpy( (DxUint8_t*)PublKey_ptr->PublKeyX, 
								   PublKeyIn_ptr + 1, ModSizeInBytes  );
	   CRYS_COMMON_ReverseMemcpy( (DxUint8_t*)PublKey_ptr->PublKeyY, 
								   PublKeyIn_ptr + 1 + ModSizeInBytes, ModSizeInBytes );                              
   }
   else
	   return CRYS_ECPKI_BUILD_KEY_INVALID_COMPRESSION_MODE_ERROR;


   /* initialize LLF public key database   */
   /*--------------------------------------*/
   LLF_ECPKI_InitPubKeyDb( PublKey_ptr);
   
   
   /*................ checking the public key ............................*/
   /*---------------------------------------------------------------------*/
   if( CheckMode >= ECpublKeyPartlyCheck )
  		/* full checking of the public key validity */
		Error = LLF_ECPKI_CheckPublKeyCall( PublKey_ptr, CheckMode,(DxUint32_t*)TempBuff_ptr );
 
   else
        /* partly checking of the public key validity */
        Error = LLF_ECPKI_CheckPublKeySize( DomainID, (DxUint8_t*)(PublKey_ptr->PublKeyX), 2*ModSizeInBytes );

   if( Error != CRYS_OK )
   {
        /* if created structure is not valid - clear it and exit */
   	    DX_VOS_MemSet( UserPublKey_ptr , 0 , sizeof(CRYS_ECPKI_UserPublKey_t) );    	    
   	    return Error;
   }

   /* ................ set the private key validation tag ................... */  
   UserPublKey_ptr->valid_tag = CRYS_ECPKI_PUBL_KEY_VALIDATION_TAG;
   

   /* ................. end of the function ................................. */
   /* ----------------------------------------------------------------------- */
      
   return Error;


#endif /* !CRYS_NO_ECPKI_SUPPORT */
 
 } /* End of _DX_ECPKI_BuildPublKey() */



 /***********************************************************************************
  *                     CRYS_ECPKI_ExportPublKey function                           *
  ***********************************************************************************/
 /**
   @brief The function converts an existed public key into the big endian and outputs it.

		  The function performs the following steps:
		  - checks input parameters,
		  - Converts the X,Y coordinates of public key EC point to big endianness.
		  - Sets the public key as follows:
		    - In case WMDRM: PubKey = X||Y;
			- For other EC domains (according to IEEE 1363-2000):
			  * In case "Uncompressed"  PubKey = PC||X||Y, PC = 0x4 - single byte,
			  * In case "Compressed" PubKey = PC||X, where PC = 0x2|(LSBit of Y),
			  * In case "Hybrid"  PubKey = PC||X||Y, PC = 0x6|(LSBit of Y).
		  - Exits.

		  NOTE: - At this stage the said compressed form is not implemented  
			      and the function returns an error in this case,
				- Size of buffers for X and also Y is equal ModSizeInBytes.
     
   @param[in]  UserPublKey_ptr -   A pointer to the public key structure (in little 
                                   endian form). 
   @param[in]  Compression     -   An enumerator parameter, defines point compression.
   @param[out] ExternPublKey_ptr - A pointer to the exported public key structure in big 
                                   endian and point compression as defined by input parameter
								   Size of buffer must be not less than:
								     2*ModSiseInBytes - for WM DRM,
									 2*ModSiseInBytes+1 - for other domains.   
   @param[out] PublKeySizeInBytes - A pointer to variable for input size of user passed
                                    buffer for public key and output the size of converted 
                                    public key in bytes.

   @return CRYSError_t - CRYS_OK,
                         CRYS_ECPKI_EXPORT_PUBL_KEY_INVALID_USER_PUBL_KEY_PTR_ERROR      
                         CRYS_ECPKI_EXPORT_PUBL_KEY_ILLEGAL_COMPRESSION_MODE_ERROR       
                         CRYS_ECPKI_EXPORT_PUBL_KEY_INVALID_EXTERN_PUBL_KEY_PTR_ERROR    
                         CRYS_ECPKI_EXPORT_PUBL_KEY_INVALID_PUBL_KEY_SIZE_PTR_ERROR      
                         CRYS_ECPKI_EXPORT_PUBL_KEY_INVALID_PUBL_KEY_SIZE_ERROR          
                         CRYS_ECPKI_EXPORT_PUBL_KEY_ILLEGAL_DOMAIN_ID_ERROR  
*/
CEXPORT_C CRYSError_t CRYS_ECPKI_ExportPublKey(
   					   CRYS_ECPKI_UserPublKey_t      *UserPublKey_ptr,       /*in*/
					   CRYS_ECPKI_PointCompression_t  Compression,           /*in*/
					   DxUint8_t			         *ExternPublKey_ptr,     /*in*/
				       DxUint32_t                    *PublKeySizeInBytes_ptr /*in/out*/ )
 {
   /*-------------------- FUNCTION DECLARATIONS ------------------------*/
     
   /* the private key structure pointer */
   CRYS_ECPKI_PublKey_t *PublKey_ptr;

   /* Domain ID and DomainInfo - structure containing part of domain information */
   CRYS_ECPKI_DomainID_t       DomainID;
   CRYS_ECPKI_DomainInfo_t   DomainInfo; 
   
   /* EC modulus size in words and in bytes*/
   DxUint32_t   ModSizeInBytes; 

   /* the Error return code identifier */
   CRYSError_t Error; 

   /* FUNCTION LOGIC */
 
   /* .................. if not supported exit .............................. */
   RETURN_IF_ECPKI_UNSUPPORTED( UserPublKey_ptr, Compression, ExternPublKey_ptr,  
                              PublKeySizeInBytes_ptr, PublKey_ptr, DomainInfo.EC_ModulusSizeInBits,                                 
                              DomainID, ModSizeInBytes, Error, Error, Error, Error, Error, Error,
                              Error, Error, Error, Error, Error, Error, Error, Error ); 
                              

#ifndef CRYS_NO_ECPKI_SUPPORT  
 
    /* .................. INITIALIZATIONS  ................................. */ 
	   
   Error = CRYS_OK; /* Error initialization */


   /*............. Checking input parameters   ..............................*/

	/* ...... checking the key database handle pointer ....................  */
   if( UserPublKey_ptr == DX_NULL )   
	    return  CRYS_ECPKI_EXPORT_PUBL_KEY_INVALID_USER_PUBL_KEY_PTR_ERROR;   	   
         
   /* ...... checking the validity of the extern Public Key pointer ........ */
   if( ExternPublKey_ptr == DX_NULL ) 
	   return  CRYS_ECPKI_EXPORT_PUBL_KEY_INVALID_EXTERN_PUBL_KEY_PTR_ERROR;   	

   /* ... checking the validity of the extern Public Key size pointer ...... */
   if( PublKeySizeInBytes_ptr == DX_NULL ) 
	   return  CRYS_ECPKI_EXPORT_PUBL_KEY_INVALID_PUBL_KEY_SIZE_PTR_ERROR;   		

  
   PublKey_ptr = (CRYS_ECPKI_PublKey_t *)UserPublKey_ptr->PublKeyDbBuff; 
   DomainID = PublKey_ptr->DomainID;

   /* ...... checking the EC domain ID...................................... */
   if( DomainID >= CRYS_ECPKI_DomainID_OffMode )
   		return  CRYS_ECPKI_EXPORT_PUBL_KEY_ILLEGAL_DOMAIN_ID_ERROR;    

   
   /*------------------------ FUNCTION LOGIC --------------------------------*/

     /* get domain info and modulus size */
   Error = LLF_ECPKI_GetDomainInfo(DomainID, &DomainInfo); 

   if(Error != CRYS_OK)
	   return Error;

   ModSizeInBytes = (DomainInfo.EC_ModulusSizeInBits + 7)/8; 

   /* Convert public key to big endianness export form */

   if( DomainID == CRYS_ECPKI_DomainID_WMDRM10)
   {
   		/* check  size of uzer passed buffer for public key*/
        if( *PublKeySizeInBytes_ptr < 2*ModSizeInBytes )
			return  CRYS_ECPKI_EXPORT_PUBL_KEY_INVALID_PUBL_KEY_SIZE_ERROR;
	   
	   /* for WMDRM revers copy X, Y */
     	CRYS_COMMON_ReverseMemcpy( ExternPublKey_ptr, 
								   (DxUint8_t*)PublKey_ptr->PublKeyX , ModSizeInBytes );        
		CRYS_COMMON_ReverseMemcpy( ExternPublKey_ptr + ModSizeInBytes, 
								   (DxUint8_t*)PublKey_ptr->PublKeyY, ModSizeInBytes );
 		
		/* Set PublKeySizeInBytes */
        *PublKeySizeInBytes_ptr = 2*ModSizeInBytes;
   }
   else    /*  All other EC domains  */
   {
        switch( Compression )
        {
     		case CRYS_EC_PointUncompressed:
				{
                    /* check uzer passed size of buffer for public key*/
					if( *PublKeySizeInBytes_ptr < 2*ModSizeInBytes + 1 )
						return  CRYS_ECPKI_EXPORT_PUBL_KEY_INVALID_PUBL_KEY_SIZE_ERROR;

					/* Set ExternPublKey[0] = PC = 4 */
                    ExternPublKey_ptr[0] = 4;

					CRYS_COMMON_ReverseMemcpy( (ExternPublKey_ptr + 1), 
									(DxUint8_t*)PublKey_ptr->PublKeyX, ModSizeInBytes );
					CRYS_COMMON_ReverseMemcpy( (ExternPublKey_ptr + 1 + ModSizeInBytes), 
									(DxUint8_t*)PublKey_ptr->PublKeyY, ModSizeInBytes ); 
 
					/* Set PublKeySizeInBytes */
		            *PublKeySizeInBytes_ptr = 2*ModSizeInBytes + 1;
					break;
				}

			case CRYS_EC_PointHybrid:
            case CRYS_EC_PointCompressed:
            case CRYS_EC_PointContWrong:
            case CRYS_EC_PointCompresOffMode:
			default:

				return CRYS_ECPKI_EXPORT_PUBL_KEY_ILLEGAL_COMPRESSION_MODE_ERROR;
		}
   }   
 
   return Error;

#endif /* !CRYS_NO_ECPKI_SUPPORT */
 
 } /* End of CRYS_ECPKI_ExportPublKey */
 
 
 
/**********************************************************************************
 *                    CRYS_ECPKI_ExportPrivKey function                            *
 **********************************************************************************/
/*!\brief 	The function performs export (converting to big endian) the user private 
            key  from the internal private key structure.

		    The function performs the following operations:
			  - Checks validity of incoming variables and pointers.
			  - Converts key data from little endian into big endian form.
			  - Outputs the key and its size in bits.. 
   
   @param[in]      UserPrivKey_ptr    - Pointer to the private key structure.
   @param[in]      PrivKeyOut_ptr     - Pointer to private key output buffer. 
   @param[in/out]  PrivKeySizeInBytes_ptr - Pointer to size of private key output buffer   
                                        in bytes.Must be not less than EC OrderSizeInBytes.
                                        The function returns the exact value of priv.key 
                                        through this pointer.        
   @return   CRYSError_t: on success - CRYS_OK, on failure - error code:
             CRYS_ECPKI_BUILD_KEY_INVALID_PRIV_KEY_IN_PTR_ERROR
			 CRYS_ECPKI_BUILD_KEY_INVALID_USER_PRIV_KEY_PTR_ERROR
*/	
CEXPORT_C CRYSError_t CRYS_ECPKI_ExportPrivKey(
								CRYS_ECPKI_UserPrivKey_t  *UserPrivKey_ptr,        /*in*/
								DxUint8_t			      *PrivKeyOut_ptr,         /*out*/
								DxUint32_t                *PrivKeySizeInBytes_ptr  /*in/out*/ ) 
 {
  
   /* FUNCTION DECLARATIONS */
   
   /* the private key structure pointer */
   CRYS_ECPKI_PrivKey_t *PrivKey_ptr;

   /*  EC domain ID, info structure and parameters */
   CRYS_ECPKI_DomainID_t     DomainID;
   CRYS_ECPKI_DomainInfo_t   DomainInfo;  
   DxUint32_t  OrderSizeInBytes, PrivKeySizeBits;

   /* Error return code identifier */
   CRYSError_t Error; 
 

   /* FUNCTION LOGIC */
 
   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */   

   RETURN_IF_ECPKI_UNSUPPORTED( UserPrivKey_ptr , PrivKeyOut_ptr , PrivKeySizeInBytes_ptr , 
                              DomainID , PrivKey_ptr , DomainInfo.EC_ModulusSizeInBits ,                              
                              OrderSizeInBytes , PrivKeySizeBits , Error , Error , Error, 
                              Error , Error , Error , Error , Error , Error , Error , 
                              Error , Error , Error , Error ); 
                                   

#ifndef CRYS_NO_ECPKI_SUPPORT  
    
   /* Error initialization */
   Error = CRYS_OK;
  
   /* ................. checking the validity of the pointer arguments ....... */
   /* ------------------------------------------------------------------------ */
  
   /* ...... checking the validity of the User Private Key pointer ........... */
   if( UserPrivKey_ptr == DX_NULL ) 
	   return  CRYS_ECPKI_BUILD_KEY_INVALID_USER_PRIV_KEY_PTR_ERROR;   		
 
   /* ...... checking the key database handle pointer ....................     */
   if( PrivKeyOut_ptr == DX_NULL )   
	    return  CRYS_ECPKI_BUILD_KEY_INVALID_PRIV_KEY_IN_PTR_ERROR;   	   
         
   /* ...... checking the priv key size pointer ..........................     */
   if( PrivKeySizeInBytes_ptr == DX_NULL )   
	   return  CRYS_ECPKI_BUILD_KEY_INVALID_PRIV_KEY_SIZE_ERROR;   	   
 
   /* setting the pointer to the key database */
   PrivKey_ptr = (CRYS_ECPKI_PrivKey_t *)UserPrivKey_ptr->PrivKeyDbBuff;
  
   /* Get EC Domain information from LLF level */
   DomainID = PrivKey_ptr->DomainID;
   
   /* retrive domain info structure from LLF */
   Error = LLF_ECPKI_GetDomainInfo(DomainID, &DomainInfo);
   
   if( Error != CRYS_OK ) 
		 goto End;  

   
   
   /****************  FUNCTION LOGIC  **************************************/
   
   /* EC order size in bytes */ 
   OrderSizeInBytes = (DomainInfo.EC_OrderSizeInBits + 7) / 8;
   
   /* calculate exact private key size in bytes */
   PrivKeySizeBits = CRYS_COMMON_GetCounterEffectiveSizeInBits( (DxUint8_t*)PrivKey_ptr->PrivKey,
                                                     (DxUint16_t)OrderSizeInBytes );
   /* output of exact priv key size in bytes */
   *PrivKeySizeInBytes_ptr = ( PrivKeySizeBits + 7 ) / 8;
   
   /* check, that PrivKeyOut_ptr buffer is enough large for output the key */ 
   if( *PrivKeySizeInBytes_ptr < *PrivKeySizeInBytes_ptr)
   	    return  CRYS_ECPKI_BUILD_KEY_INVALID_PRIV_KEY_SIZE_ERROR;
                                                     

   /* ...... reverse copy the the key from key structure  ................ */
   /* -------------------------------------------------------------------- */
   
   CRYS_COMMON_ReverseMemcpy( PrivKeyOut_ptr, (DxUint8_t*)PrivKey_ptr->PrivKey, 
                              *PrivKeySizeInBytes_ptr  );
                             
 

   /* ................. end of the function ................................. */
   /* ----------------------------------------------------------------------- */
   
 End:      

   return Error;

#endif /* !CRYS_NO_ECPKI_SUPPORT */

 } /* End of CRYS_ECPKI_BuildPrivKey() */
 
 
