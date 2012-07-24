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
 
 
#ifndef CRYS_SST_KG_H
#define CRYS_SST_KG_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "DX_VOS_BaseTypes.h"
#include "CRYS_error.h"
#include "CRYS_Defs.h"
#include "CRYS_AES.h"
#include "CRYS_DES.h"
#include "CRYS_HMAC.h"
#include "CRYS_RSA_SCHEMES.h"
#include "CRYS_RSA_PRIM.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object % CRYS_SST_KG.h    : %
   *  State           :  %state%
   *  Creation date   :  Wed Nov 17 16:42:30 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief This file contains all of the enums and definitions that are used for the 
   *         CRYS KG APIs, as well as the APIs themselves. 
   *
   *  \version CRYS_SST_KG.h#1:incl:13
   *  \author adams
   */

/************************ Defines ******************************/

/** 
@brief - a definition describing the low level Engine type ( SW , Hardware , Etc )
*/


/************************ Enums ********************************/

/* Enum defining the user's key size argument */

/************************ Typedefs  ****************************/


/************************ context Structs  ******************************/


/************************ Public Variables **********************/


/************************ Public Functions **********************/

/********************************** AES *******************************************/

/**
 * @brief The function creates AES key saves it in the SST and 
 *        return the handle to it.
 *
 *
 * @param[in] TeansactionId – the opened atransaction for the SST changes. 
 *
 * @param[in] SessionId – the opened authenticator SessionId. 
 *
 * @param[in] MasterAuthHandle - the master authenticator that has all of the 
 *								access writes on the new inserted data including 
 *								binding writes to additional authenticators.
 *
 * @param[in] DataType – user depend for internal SST use. 
 *
 * @param[in] KeySize - type of AES key (128,192 or 256 bits).
 *
 * @param[in/out] KeyHandle_ptr - handle to SST entry.The user can enter an empty handle  
 *								  and get a new handle, or enter handle and force the SST to use it
 *								  if possible.
 *
 * @return CRYSError_t - On success the value CRYS_OK is returned, 
 *                        and on failure a value from CRYS_SST_error.h TBD
 */

CIMPORT_C CRYSError_t  CRYS_SST_KG_AES(  DxUint32_t			TeansactionId,
									     SSTSessionId_t		SessionId,
										 SSTHandle_t		MasterAuthHandle,
			                             SSTDataType_t      DataType,
										 CRYS_AES_KeySize_t KeySize,
										 SSTHandle_t		*KeyHandle_ptr,
										 DxUint8_t*			aWorkspace_ptr,
										 DxUint32_t		    aWorkspaceSizeInBytes);

/**
 * @brief The function creates DES key saves it in the SST and 
 *        return the handle to it.
 *
 *
 * @param[in] TeansactionId – the opened atransaction for the SST changes. 
 *
 * @param[in] SessionId – the opened authenticator SessionId. 
 *
 * @param[in] MasterAuthHandle - the master authenticator that has all of the 
 *								access writes on the new inserted data including 
 *								binding writes to additional authenticators.
 *
 * @param[in] DataType – user depand for internal SST use. 
 *
 * @param[in,out] NumOfKeys - type of DES key (DES or 3DES).
 *
 * @param[in/out] KeyHandle_ptr - handle to SST entry.The user can enter an empty handle  
 *								  and get a new handle, or enter handle and force the SST to use it
 *								  if possible.
 *
 * @return CRYSError_t - On success the value CRYS_OK is returned, 
 *                        and on failure a value from CRYS_SST_error.h TBD
 */

CIMPORT_C CRYSError_t  CRYS_SST_KG_DES(  DxUint32_t			TeansactionId,
									     SSTSessionId_t		SessionId,
										 SSTHandle_t		MasterAuthHandle,
			                             SSTDataType_t      DataType,
										 CRYS_DES_NumOfKeys_t NumOfKeys,
										 SSTHandle_t		*KeyHandle_ptr,
										 DxUint8_t*			aWorkspace_ptr,
										 DxUint32_t		    aWorkspaceSizeInBytes);


/**
 * @brief The function creates HMAC key saves it in the SST and 
 *        return the handle to it.
 *
 *
 * @param[in] TeansactionId – the opened transaction for the SST changes. 
 *
 * @param[in] SessionId – the opened authenticator SessionId. 
 *
 * @param[in] MasterAuthHandle - the master authenticator that has all of the 
 *								access writes on the new inserted data including 
 *								binding writes to additional authenticators.
 *
 * @param[in] DataType – user depend for internal SST use. 
 *
 * @param[in] KeySize - HMAC key len in bytes.
 *
 * @param[in/out] KeyHandle_ptr - handle to SST entry.The user can enter an empty handle  
 *								  and get a new handle, or enter handle and force the SST to use it
 *								  if possible.
 *
 * @return CRYSError_t - On success the value CRYS_OK is returned, 
 *                        and on failure a value from CRYS_SST_error.h TDB
 */

CIMPORT_C CRYSError_t  CRYS_SST_KG_HMAC( DxUint32_t			TeansactionId,
									     SSTSessionId_t		SessionId,
										 SSTHandle_t		MasterAuthHandle,
			                             SSTDataType_t      DataType,
										 DxUint32_t			KeySize,
										 SSTHandle_t		*KeyHandle_ptr,
										 DxUint8_t*			aWorkspace_ptr,
										 DxUint32_t		    aWorkspaceSizeInBytes);


/**
 * @brief The function creates RSA key pair saves it in the SST and 
 *        return the handle to it.
 *
 *
 * @param[in] TeansactionId – the opened atransaction for the SST changes. 
 *
 * @param[in] SessionId – the opened authenticator SessionId. 
 *
 * @param[in] MasterAuthHandle - the master authenticator that has all of the 
 *								access writes on the new inserted data including 
 *								binding writes to additional authenticators.
 *
 * @param[in] DataType – user depand for internal SST use. 
 *
 * @param[in/out] KeyHandle_ptr - handle to SST entry.The user can enter an empty handle  
 *								  and get a new handle, or enter handle and force the SST to use it
 *								  if possible.
 *
 * @param[in] PubExp_ptr - public exponent  
 *
 * @param[in] PubExpSizeInBytes - size of public exponent in bytes.  
 *
 * @param[in] KeySize - size of the modulus in bits
 *
 * @param[out] UserPrivKey_ptr - private key structure.This structure is used as internal buffer.  
 *
 * @param[out] UserPubKey_ptr -public key structure.This structure is used as internal buffer.  
 *
 * @param[in] KeyGenData_ptr - internal buffer for the key gen operation.  
 *
 * @return CRYSError_t - On success the value CRYS_OK is returned, 
 *                        and on failure a value from CRYS_SST_error.h TBD
 */

CIMPORT_C CRYSError_t CRYS_SST_RSA_KG_GenerateKeyPair(DxUint32_t					TeansactionId,
													  SSTSessionId_t				SessionId,
													  SSTHandle_t					MasterAuthHandle,
													  SSTDataType_t					DataType,
													  SSTHandle_t					*KeyHandle_ptr,
													  DxUint8_t						*PubExp_ptr,
													  DxUint16_t					PubExpSizeInBytes,
													  DxUint32_t					KeySize,
													  CRYS_SST_RSA_keys_and_data_container_t	*KeyGenData_ptr);


/**
 * @brief The function creates RSA key pair with CRT format ,saves it in the SST and 
 *        return the handle to it.
 *
 *
 * @param[in] TeansactionId – the opened atransaction for the SST changes. 
 *
 * @param[in] SessionId – the opened authenticator SessionId. 
 *
 * @param[in] MasterAuthHandle - the master authenticator that has all of the 
 *								access writes on the new inserted data including 
 *								binding writes to additional authenticators.
 *
 * @param[in] DataType – user depand for internal SST use. 
 *
 * @param[in/out] KeyHandle_ptr - handle to SST entry.The user can enter an empty handle  
 *								  and get a new handle, or enter handle and force the SST to use it
 *								  if possible.
 *
 * @param[in] PubExp_ptr - public exponent  
 *
 * @param[in] PubExpSizeInBytes - size of public exponent in bytes.  
 *
 * @param[in] KeySize - size of the modulus in bits  
 *
 * @param[in] KeyGenData_ptr - internal buffer for the key gen operation.  
 *
 * @return CRYSError_t - On success the value CRYS_OK is returned, 
 *                        and on failure a value from CRYS_SST_error.h TBD
 */

CIMPORT_C CRYSError_t CRYS_SST_RSA_KG_GenerateKeyPairCRT(DxUint32_t					TeansactionId,
														 SSTSessionId_t				SessionId,
														 SSTHandle_t				MasterAuthHandle,
														 SSTDataType_t				DataType,
														 SSTHandle_t				*KeyHandle_ptr,
														 DxUint8_t					*PubExp_ptr,
														 DxUint16_t					PubExpSizeInBytes,
														 DxUint32_t					KeySize,
														 CRYS_SST_RSA_keys_and_data_container_t	*KeyGenData_ptr);	

#ifdef __cplusplus
}
#endif

#endif
