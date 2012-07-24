/**************************************************************************
 *   Copyright 2009 © Discretix Technologies Ltd. This software is         *
 *   protected by copyright, international treaties and various patents.   *
 *   Any copy or reproduction of this Software as permitted below, must    *
 *   include this Copyright Notice as well as any other notices provided   *
 *   under such license.                                                   *
 *                                                                         *
 *   This program shall be governed by, and may be used and redistributed  *
 *   under the terms and conditions of the GNU Lesser General Public       *
 *   License, version 2.1, as published by the Free Software Foundation.   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY liability and WARRANTY; without even the implied      *
 *   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.      *
 *   See the GNU General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this program; if not, please write to the          *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
 
 #ifndef CRYS_ECPKI_BUILD_H
#define CRYS_ECPKI_BUILD_H

/*
 *  Object name     :  CRYS_ECPKI_BUILD.h
 *  State           :  %state%
 *  Creation date   :  02.02.2006
 *  Last modified   :  %modify_time%
 */
/** @file CRYS_ECPKI_BUILD.h
 * \brief Defines the API that supports EC Diffie-Hellman
 *        shared secret value derivation primitives
 *
 * \version CRYS_ECPKI_BUILD.h #1:incl:1
 * \author R.Levin
 */
#include "DX_VOS_BaseTypes.h"
#include "CRYS_Defs.h"
#include "CRYS_error.h"

#include "CRYS_ECPKI_Types.h"

#ifdef __cplusplus
extern "C"
{
#endif



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
CIMPORT_C CRYSError_t CRYS_ECPKI_BuildPrivKey(
			    CRYS_ECPKI_DomainID_t      DomainID,	      /*in */   
				DxUint8_t			      *PrivKeyIn_ptr,     /*in*/
				DxUint32_t                 PrivKeySizeInBytes,/*in*/
				CRYS_ECPKI_UserPrivKey_t  *UserPrivKey_ptr    /*out*/ );


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
 CIMPORT_C CRYSError_t _DX_ECPKI_BuildPublKey(
									CRYS_ECPKI_DomainID_t        DomainID,	          /*in*/				
									DxUint8_t			        *PublKeyIn_ptr,       /*in*/									
									DxUint32_t                   PublKeySizeInBytes,  /*in*/
									EC_PublKeyCheckMode_t        CheckMode,           /*in*/
									CRYS_ECPKI_UserPublKey_t    *UserPublKey_ptr,     /*out*/
									CRYS_ECPKI_BUILD_TempData_t *TempBuff_ptr         /*in*/ ); 


/**********************************************************************************
 *	               CRYS_ECPKI_BuildPublKey macro                                  *
 **********************************************************************************/
/**
   @brief 	The CRYS_ECPKI_BuildPublKey macro definition calls the _DX_ECPKI_BuildPublKey
            function for building the public key with checking input pointers and sizes. 

 
   @param[in]  ECPKI_DomainID  - The enumerator variable defines current EC domain.
   @param[in]  PublKeyIn_ptr   - The pointer to private key data.
   @param[in]  PublKeySizeInBytes - Size of private key data in bytes (according to EC domain),
               it should be equal to 2*modulusSize (CRYS_ECPKI_DomainID_WMDRM10) or 
			   2*modulusSize + 1byte (other domains).
   @param[out] UserPublKey_ptr - A pointer to the private key structure.

   @return CRYSError_t - CRYS_OK,
                         CRYS_ECPKI_BUILD_KEY_INVALID_PUBL_KEY_IN_PTR_ERROR
						 CRYS_ECPKI_BUILD_KEY_INVALID_USER_PUBL_KEY_PTR_ERROR
						 CRYS_ECPKI_BUILD_KEY_ILLEGAL_DOMAIN_ID_ERROR
						 CRYS_ECPKI_BUILD_KEY_INVALID_PUBL_KEY_DATA_ERROR
						 CRYS_ECPKI_BUILD_KEY_INVALID_COMPRESSION_MODE_ERROR
*/
 #define  CRYS_ECPKI_BuildPublKey( DomainID, PublKeyIn_ptr, PublKeySizeInBytes, UserPublKey_ptr ) \
          _DX_ECPKI_BuildPublKey( (DomainID), (PublKeyIn_ptr), (PublKeySizeInBytes), CheckPointersAndSizesOnly , (UserPublKey_ptr),  DX_NULL )


/**********************************************************************************
 *	               CRYS_ECPKI_BuildPublKeyPartlyCheck macro                         *
 **********************************************************************************/
/**
   @brief 	The CRYS_ECPKI_BuildPublKeyPartlyCheck macro definition calls the _DX_ECPKI_BuildPublKey
            function for building the public key with partally validation of the key [SEC1. ECC standard: 3.2.3]. 

   @param[in]  ECPKI_DomainID  - The enumerator variable defines current EC domain.
   @param[in]  PublKeyIn_ptr   - The pointer to private key data.
   @param[in]  PublKeySizeInBytes - Size of private key data in bytes (according to EC domain),
               it should be equal to 2*modulusSize (CRYS_ECPKI_DomainID_WMDRM10) or 
			   2*modulusSize + 1byte (other domains).
   @param[out] UserPublKey_ptr - A pointer to the private key structure.
   @param[in]  TempBuff_ptr    - A pointer to the temp buffer structure for build function.

   @return CRYSError_t - CRYS_OK,
                         CRYS_ECPKI_BUILD_KEY_INVALID_PUBL_KEY_IN_PTR_ERROR
						 CRYS_ECPKI_BUILD_KEY_INVALID_USER_PUBL_KEY_PTR_ERROR
						 CRYS_ECPKI_BUILD_KEY_ILLEGAL_DOMAIN_ID_ERROR
						 CRYS_ECPKI_BUILD_KEY_INVALID_PUBL_KEY_DATA_ERROR
						 CRYS_ECPKI_BUILD_KEY_INVALID_COMPRESSION_MODE_ERROR
*/
#define  CRYS_ECPKI_BuildPublKeyPartlyCheck( DomainID, PublKeyIn_ptr, PublKeySizeInBytes, UserPublKey_ptr,  TempBuff_ptr ) \
	     _DX_ECPKI_BuildPublKey( DomainID,	PublKeyIn_ptr, PublKeySizeInBytes, ECpublKeyPartlyCheck, UserPublKey_ptr,  TempBuff_ptr )


/**********************************************************************************
 *	               CRYS_ECPKI_BuildPublKeyFullCheck macro                         *
 **********************************************************************************/
/**
   @brief 	The CRYS_ECPKI_BuildPublKeyFullCheck macro definition calls the _DX_ECPKI_BuildPublKey
            function for building the public key with full validation of the key [SEC1. ECC standard: 3.2.2]. 

   @param[in]  ECPKI_DomainID  - The enumerator variable defines current EC domain.
   @param[in]  PublKeyIn_ptr   - The pointer to private key data.
   @param[in]  PublKeySizeInBytes - Size of private key data in bytes (according to EC domain),
               it should be equal to 2*modulusSize (CRYS_ECPKI_DomainID_WMDRM10) or 
			   2*modulusSize + 1byte (other domains).
   @param[out] UserPublKey_ptr - A pointer to the private key structure.
   @param[in]  TempBuff_ptr    - A pointer to the temp buffer structure for build function.

   @return CRYSError_t - CRYS_OK,
                         CRYS_ECPKI_BUILD_KEY_INVALID_PUBL_KEY_IN_PTR_ERROR
						 CRYS_ECPKI_BUILD_KEY_INVALID_USER_PUBL_KEY_PTR_ERROR
						 CRYS_ECPKI_BUILD_KEY_ILLEGAL_DOMAIN_ID_ERROR
						 CRYS_ECPKI_BUILD_KEY_INVALID_PUBL_KEY_DATA_ERROR
						 CRYS_ECPKI_BUILD_KEY_INVALID_COMPRESSION_MODE_ERROR
*/
#define  CRYS_ECPKI_BuildPublKeyFullCheck( DomainID, PublKeyIn_ptr, PublKeySizeInBytes, UserPublKey_ptr,  TempBuff_ptr ) \
	     _DX_ECPKI_BuildPublKey( DomainID,	PublKeyIn_ptr, PublKeySizeInBytes, ECpublKeyFullCheck, UserPublKey_ptr,  TempBuff_ptr )

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
CIMPORT_C CRYSError_t CRYS_ECPKI_ExportPublKey(
   					   CRYS_ECPKI_UserPublKey_t      *UserPublKey_ptr,     /*in*/
					   CRYS_ECPKI_PointCompression_t  Compression,         /*in*/
					   DxUint8_t			         *ExternPublKey_ptr,   /*in*/
				       DxUint32_t                    *PublKeySizeInBytes   /*in/out*/ ); 


/**********************************************************************************
 *                    CRYS_ECPKI_ExportPrivKey function                           *
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
CIMPORT_C CRYSError_t CRYS_ECPKI_ExportPrivKey(
								CRYS_ECPKI_UserPrivKey_t  *UserPrivKey_ptr,        /*in*/
								DxUint8_t			      *PrivKeyOut_ptr,         /*out*/
								DxUint32_t                *PrivKeySizeInBytes_ptr  /*in/out*/ );
				              

#ifdef __cplusplus
}
#endif

#endif
