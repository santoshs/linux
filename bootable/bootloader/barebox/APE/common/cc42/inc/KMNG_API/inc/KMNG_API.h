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
  
  
 #ifndef __KMNG_API_H__
#define __KMNG_API_H__

#include "KMNG_Defs.h"

/*
   *  Object %name    : SEPDriver.h
   *  State           :  %state%
   *  Creation date   :  Wed Nov 17 17:39:24 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief  Key Management h file (API and structures )
   *
   *  \version 
   *  \author yevgenys
   */


/*------------------------------
    DEFINES
--------------------------------*/


/*-------------------------------
  STRUCTURES
---------------------------------*/




/*------------------------------------------------
    FUNCTIONS
--------------------------------------------------*/

/**
 * @brief     This function initializes the KMNG component
 * 
 * @return     DxError_t:  
 *                        
 */
DxError_t KMNG_Init(void);

/**
 * @brief     This function returns the size of buffer in bytes,
 *			  that needs to be allocated for the KeyRing with specific paramaters.
 *
 * @param[in] numSymKeys - number of symmetric keys to be kept in the key ring
 * @param[in] numRSAKey - number of RSA keys to be kept in the key ring
 * @param[in] numDHKeys - number of DH keys to be kept in the key ring
 * @param[out] bufferSize_ptr - buffer size needed for key ring
 * 
 * @return     DxError_t_t:  
 *                        
 */
DxError_t KMNG_GetKeyRingBufferSize(DxUint32_t   numSymKeys , 
                                 DxUint32_t   numRSAKey,
                                 DxUint32_t   numDHKeys,
                                 DxUint32_t*  bufferSize_ptr);
                                           

/**
 * @brief     This function initializes the key ring. It creates key ring key,
 *			  initializes it with given password and initializes the key ring data with given keys information.
 *
 * @param[in] passwordLength - length of the password
 * @param[in] password_ptr - password pointer
 * @param[in] numSymKeys - number of symmetric keys to be kept in the key ring
 * @param[in] numRSAKeys - number of RSA keys to be kept in the key ring
 * @param[in] numDHKeys - number of DH keys to be kept in the key ring
 * @param[out] keyRing_ptr - key ring structure to be initialized
 * 
 * @return     DxError_t_t:  
 *                        
 */
DxError_t KMNG_KeyRingInit(DxUint32_t    passwordLength,
                           DxUint8_t*    password_ptr,
                           DxUint32_t    numSymKeys,
                           DxUint32_t    numRSAKeys,
                           DxUint32_t    numDHKeys, 
                           DxUint8_t*    keyRing_ptr);



/**
 * @brief     This function returns the number of symmetric, RSA and DH keys in the key ring.
 *
 *
 * @param[out] numSymKeys_ptr - number of symmetric keys  in the key ring
 * @param[out] numRSAKey_ptr - number of RSA keys  in the key ring
 * @param[out] numDHKeys_ptr - number of DH keys  in the key ring
 * @param[in] keyRing_ptr - key ring 
 * 
 * @return     DxError_t_t:  
 *                        
 */
DxError_t KMNG_GetKeyRingSize(DxUint32_t*  numSymKeys_ptr , 
                            DxUint32_t*  numRSAKey_ptr,
                            DxUint32_t*  numDHKeys_ptr,
                            DxUint8_t*   keyRing_ptr);
                                     

/**
 * @brief     This function returns the number of available entires symmetric, RSA and DH keys in the key ring
 *
 *
 * @param[out] numSymKeys_ptr - number of symmetric keys available entries in the key ring
 * @param[out] numRSAKey_ptr - number of RSA keys available entries in the key ring
 * @param[out] numDHKeys_ptr - number of DH keys available entries in the key ring
 * @param[in] keyRing_ptr - key ring 
 * 
 * @return     DxError_t_t:  
 *                        
 */
DxError_t KMNG_GetKeyRingCapacity(DxUint32_t*  numSymKeys_ptr , 
                                DxUint32_t*  numRSAKey_ptr,
                                DxUint32_t*  numDHKeys_ptr,
                                DxUint8_t*   keyRing_ptr);
                                         
                                         
/**
 * @brief     This function returns the ids of the  symmetric, RSA and DH keys in the key ring
 *
 *
 * @param[out] symKeyIds_ptr - The symmetric ids keys available entries in the key ring
 * @param[out] rsaKeyIds_ptr - The RSA ids keys available entries in the key ring
 * @param[out] dhKeyIds_ptr - The DH ids keys available entries in the key ring
 * @param[in] keyRing_ptr - key ring 
 * 
 * @return     DxError_t_t:  
 *                        
 */
DxError_t KMNG_ListKeyIDs(DxUint32_t*  symKeyIds_ptr , 
                        DxUint32_t*  rsaKeyIds_ptr,
                        DxUint32_t*  dhKeyIds_ptr,
                        DxUint8_t*   keyRing_ptr);
                                
                                
/**
 * @brief     This function creates a user key and inserts it into the keyRing
 *
 *
 * @param[in] keyRingPasswordAddr - address of the key ring password string
 * @param[in] keyRingPasswordLen - length of the key ring password
 * @param[in] userPasswordAddr - address of the new key password string
 * @param[in] userPasswordLen - length of the new key password
 * @param[in] keyType - type of the key
 * @param[in] keySize - size of the key in bytes
 * @param[in] keyUsage - usage of the key
 * @param[in] keyRestriction - restriction of the key
 * @param[in] UserSpecificKeyData - user specific data for key
 * @param[out] keyId - the returned id of the new key in the key ring
 * @param[in] keyRing_ptr - key ring 
 * 
 * @return     DxError_t:  
 *                        
 */
DxError_t KMNG_CreateUserKey(
                           DxUint8_t*                   keyRingPassword_ptr , 
                           DxUint32_t                   keyRingPasswordLen,
                           DxUint8_t*                   newKeyPassword_ptr , 
                           DxUint32_t                   newKeyPasswordLen,
                           KMNG_KeyType_t               keyType,
                           DxUint32_t                   keySize,
                           DxUint32_t                   keyUsage,
                           DxUint32_t                   keyRestriction,
                           KMNG_UserSpecificKeyData_t   UserSpecificKeyData,
                           DxUint32_t*                  keyId,
                           DxUint8_t*                   keyRing_ptr);
                                    
                                    
/**
 * @brief     This function imports a symmetric key into key ring
 *
 *
 * @param[in] keyRingPasswordAddr - address of the key ring password string
 * @param[in] keyRingPasswordLen - length of the key ring password
 * @param[in] userPasswordAddr - address of the new key password string
 * @param[in] userPasswordLen - length of the new key password
 * @param[in] keyType - type of the key
 * @param[in] keySize - size of the key in bytes
 * @param[in] keyUsage - usage of the key
 * @param[in] keyRestriction - restriction of the key
 * @param[in] UserSpecificKeyData - user specific data for key 
 * @param[in] key - symmetric key itself
 * @param[out] keyId - the returned id of the new key in the key ring
 * @param[in] keyRing_ptr - key ring 
 * 
 * @return     DxError_t:  
 *                        
 */
DxError_t KMNG_ImportSymUserKey(DxUint8_t*                keyRingPassword_ptr , 
                              DxUint32_t                  keyRingPasswordLen,
                              DxUint8_t*                  newKeyPassword_ptr , 
                              DxUint32_t                  newKeyPasswordLen,
                              KMNG_KeyType_t              keyType,
                              DxUint32_t                  keySize,
                              DxUint32_t                  keyUsage,
                              DxUint32_t                  keyRestriction,
                              KMNG_UserSpecificKeyData_t  UserSpecificKeyData,
                              DxUint8_t*                  key_ptr,
                              DxUint32_t*                 keyId_ptr,
                              DxUint8_t*                  keyRing_ptr);
                                       
                                       
/**
 * @brief     This function imports a RSA key into key ring
 *
 *
 * @param[in] keyRingPassword_ptr - address of the key ring password string
 * @param[in] keyRingPasswordLen - length of the key ring password
 * @param[in] userPassword_ptr - address of the new key password string
 * @param[in] userPasswordLen - length of the new key password
 * @param[in] keyType - type of the key
 * @param[in] keySize - size of the key in bytes
 * @param[in] keyUsage - usage of the key
 * @param[in] keyRestriction - restriction of the key
 * @param[in] UserSpecificKeyData - user specific data for key  
 * @param[in] modulus_ptr - modulus pointer
 * @param[in] modulusSize - modulus size
 * @param[in] E_ptr - E pointer
 * @param[in] E_Size - E size
 * @param[in] D_ptr - D pointer
 * @param[in] D_Size - D size
 * @param[out] keyId - the returned id of the new key in the key ring
 * @param[in] keyRing_ptr - key ring 
 * 
 * @return     DxError_t:  
 *                        
 */
DxError_t KMNG_ImportRsaUserKey(DxUint8_t*               keyRingPassword_ptr , 
                             DxUint32_t                  keyRingPasswordLen,
                             DxUint8_t*                  newKeyPassword_ptr ,
                             DxUint32_t                  newKeyPasswordLen,
                             KMNG_KeyType_t              keyType,
                             DxUint32_t                  keySize,
                             DxUint32_t                  keyUsage,
                             DxUint32_t                  keyRestriction,
                             KMNG_UserSpecificKeyData_t  UserSpecificKeyData,
                             DxUint8_t*                  modulus_ptr,
                             DxUint32_t                  modulusSize,
                             DxUint8_t*                  E_ptr,
                             DxUint32_t                  E_Size,
                             DxUint8_t*                  D_ptr,
                             DxUint32_t                  D_Size,
                             DxUint32_t*                 keyId,
                             DxUint8_t*                  keyRing_ptr);
                                       

/**
 * @brief     This function imports a DH key into key ring
 *
 *
 * @param[in] keyRingPassword_ptr - address of the key ring password string
 * @param[in] keyRingPasswordLen - length of the key ring password
 * @param[in] userPassword_ptr - address of the new key password string
 * @param[in] userPasswordLen - length of the new key password
 * @param[in] keyType - type of the key
 * @param[in] keySize - size of the key in bytes
 * @param[in] keyUsage - usage of the key
 * @param[in] keyRestriction - restriction of the key
 * @param[in] UserSpecificKeyData - user specific data for key  
 * @param[in] modulus_ptr - modulus pointer
 * @param[in] modulusSize - modulus size
 * @param[in] generator_ptr - generator pointer
 * @param[in] generatorSize - generator size
 * @param[in] N_ptr - N pointer
 * @param[in] N_Size - N size
 * @param[in] D_ptr - D pointer
 * @param[in] D_Size - D size
 * @param[out] keyId - the returned id of the new key in the key ring
 * @param[in] keyRing_ptr - key ring 
 * 
 * @return     DxError_t:  
 *                        
 */                                       
DxError_t KMNG_ImportDHUserKey(DxUint8_t*                keyRingPassword_ptr , 
                            DxUint32_t                   keyRingPasswordLen,
                            DxUint8_t*                   newKeyPassword_ptr ,
                            DxUint32_t                   newKeyPasswordLen,
                            KMNG_KeyType_t               keyType,
                            DxUint32_t                   keySize,
                            DxUint32_t                   keyUsage,
                            DxUint32_t                   keyRestriction,
                            KMNG_UserSpecificKeyData_t   UserSpecificKeyData,
                            DxUint8_t*                   prime_ptr,
                            DxUint32_t                   primeSize,
                            DxUint8_t*                   generator_ptr,
                            DxUint32_t                   generatorSize,
                            DxUint8_t*                   public_ptr,
                            DxUint32_t                   publicSize,
                            DxUint8_t*                   private_ptr,
                            DxUint32_t                   privateSize,
                            DxUint32_t*                  keyId,
                            DxUint8_t*                   keyRing_ptr);


/**
 * @brief     This function exports a symmetric key from key ring
 *
 *
 * @param[in] keyRingPassword_ptr - address of the key ring password string
 * @param[in] keyRingPasswordLen - length of the key ring password
 * @param[in] userPassword_ptr - address of the new key password string
 * @param[in] userPasswordLen - length of the new key password
 * @param[in] keyId - the id of the exported key in the key ring
 * @param[out] key_ptr - symmetric key itself
 * @param[out] keyType_ptr - type of the key
 * @param[in]  keySize_ptr - size of the key in bytes
 * @param[out] keyUsage_ptr - usage of the key
 * @param[out] keyRestriction_ptr - restriction of the key
 * @param[in]  keyRing_ptr - key ring 
 * 
 * @return     DxError_t:  
 *                        
 */                                       
DxError_t KMNG_ExportSymUserKey(DxUint8_t*                keyRingPassword_ptr , 
                              DxUint32_t                  keyRingPasswordLen,
                              DxUint8_t*                  keyPassword_ptr , 
                              DxUint32_t                  keyPasswordLen,
                              DxUint32_t                  keyId,
                              DxUint8_t*                  key_ptr,
                              KMNG_KeyType_t*             keyType_ptr,
                              DxUint32_t*                 keySize_ptr,
                              DxUint32_t*                 keyUsage,
                              DxUint32_t*                 keyRestriction_ptr,
                              DxUint8_t*                  keyRing_ptr);

/**
 * @brief     This function exports an RSA key from key ring
 *
 *
 * @param[in] keyRingPassword_ptr - address of the key ring password string
 * @param[in] keyRingPasswordLen - length of the key ring password
 * @param[in] userPassword_ptr - address of the new key password string
 * @param[in] userPasswordLen - length of the new key password
 * @param[in] keyId - the id of the exported key in the key ring
 * @param[out] modulus_ptr - modulus pointer
 * @param[out] modulusSize_ptr - modulus size
 * @param[out] E_ptr - E pointer
 * @param[out] E_Size_ptr - E size
 * @param[out] D_ptr - D pointer
 * @param[out] D_Size_ptr - D size
 * @param[in] keyRing_ptr - key ring 
 * 
 * @return     DxError_t:  
 *                        
 */                                           
DxError_t KMNG_ExportRSAUserKey(DxUint8_t*                   keyRingPassword_ptr , 
                              DxUint32_t                     keyRingPasswordLen,
                              DxUint8_t*                     keyPassword_ptr , 
                              DxUint32_t                     keyPasswordLen,
                              DxUint32_t                     keyId,
                              DxUint8_t*                     modulus_ptr,
                              DxUint32_t*                    modulusSize_ptr,
                              DxUint8_t*                     E_ptr,
                              DxUint32_t*                    E_Size_ptr,
                              DxUint8_t*                     D_ptr,
                              DxUint32_t*                    D_Size_ptr,
                              DxUint8_t*                     keyRing_ptr);

/**
 * @brief     This function exports an DH key from key ring
 *
 *
 * @param[in] keyRingPassword_ptr - address of the key ring password string
 * @param[in] keyRingPasswordLen - length of the key ring password
 * @param[in] userPassword_ptr - address of the new key password string
 * @param[in] userPasswordLen - length of the new key password
 * @param[in] keyId - the id of the exported key in the key ring
 * @param[out] modulus_ptr - modulus pointer
 * @param[out] modulusSize_ptr - modulus size
 * @param[out] generator_ptr - generator pointer
 * @param[out] generatorSize_ptr - generator size
 * @param[in] N_ptr - N pointer to public exponent
 * @param[in] N_Size_ptr - pointer to exponent size
 * @param[out] D_ptr - D pointer
 * @param[out] D_Size_ptr - D size
 * @param[in] keyRing_ptr - key ring 
 * 
 * @return     DxError_t:  
 *                        
 */                                           
DxError_t KMNG_ExportDHUserKey(
                             DxUint8_t*                     keyRingPassword_ptr , 
                             DxUint32_t                     keyRingPasswordLen,
                             DxUint8_t*                     keyPassword_ptr , 
                             DxUint32_t                     keyPasswordLen,
                             DxUint32_t                     keyId,
                             DxUint8_t*                     prime_ptr,
                             DxUint32_t*                    primeSize_ptr,
                             DxUint8_t*                     generator_ptr,
                             DxUint32_t*                    generatorSize_ptr,
                             DxUint8_t*                     public_ptr,
                             DxUint32_t*                    publicSize_ptr,
                             DxUint8_t*                     private_ptr,
                             DxUint32_t*                    privateSize_ptr,
                             DxUint8_t*                     keyRing_ptr);
/**
 * @brief     This function get the public DH key data from key ring
 *
 *
 * @param[in] keyRingPassword_ptr - address of the key ring password string
 * @param[in] keyRingPasswordLen - length of the key ring password
 * @param[in] userPassword_ptr - address of the new key password string
 * @param[in] userPasswordLen - length of the new key password
 * @param[in] keyId - the id of the exported key in the key ring
 * @param[in] modulus_ptr - modulus pointer
 * @param[in] modulusSize_ptr - modulus size
 * @param[in] generator_ptr - modulus pointer
 * @param[in] generatorSize_ptr - modulus size
 * @param[in] E_ptr - E pointer to public exponent
 * @param[in] E_Size_ptr - pointer to exponent size
 * @param[in] keyRing_ptr - key ring 
 * 
 * @return     DxError_t:  
 *                        
 */                                           
DxError_t KMNG_GetDHPublicKey(DxUint8_t*                     keyRingPassword_ptr , 
							  DxUint32_t                     keyRingPasswordLen,
                              DxUint8_t*                     keyPassword_ptr , 
                              DxUint32_t                     keyPasswordLen,
                              DxUint32_t                     keyId,
                              DxUint8_t*                     prime_ptr,
                              DxUint32_t*                    primeSize_ptr,
                              DxUint8_t*                     generator_ptr,
                              DxUint32_t*                    generatorSize_ptr,
                              DxUint8_t*                     public_ptr,
                              DxUint32_t*                    publicSize_ptr,
                              DxUint8_t*                     keyRing_ptr);
                             
                             
/**
 * @brief     This function retrieves a key information (besides key themself) from key ring.
 *
 *
 * @param[in] keyId - the id of the key in the key ring
 * @param[out] keyType_ptr - type of the key
 * @param[in]  keySize_ptr - size of the key in bytes
 * @param[out] keyUsage_ptr - usage of the key
 * @param[out] keyRestriction_ptr - restriction of the key
 * @param[in]  UserSpecificKeyData - user specific data for key   
 * @param[in]  keyRing_ptr - key ring 
 * 
 * @return     DxError_t:  
 *                        
 */                                       
DxError_t KMNG_RetrieveUserKeyInform(DxUint32_t                  keyId,
                                     KMNG_KeyType_t*             keyType_ptr,
                                     DxUint32_t*                 keySize_ptr,
                                     DxUint32_t*                 keyUsage,
                                     DxUint32_t*                 keyRestriction_ptr,
                                     KMNG_UserSpecificKeyData_t  UserSpecificKeyData,
                                     DxUint8_t*                  keyRing_ptr);
                             

/**
 * @brief     This function updates the user specific key data in key ring.
 *
 *
 * @param[in] keyRingPassword_ptr - address of the key ring password string
 * @param[in] keyRingPasswordLen - length of the key ring password
 * @param[in] userPassword_ptr - address of the new key password string
 * @param[in] userPasswordLen - length of the new key password
 * @param[in] keyId - the id of the key in the key ring
 * @param[in]  UserSpecificKeyData - user specific data for key   
 * @param[in]  keyRing_ptr - key ring 
 * 
 * @return     DxError_t:  
 *                        
 */                                       
DxError_t KMNG_UpdateUserSpecificKeyData(
                              DxUint8_t*                  keyRingPassword_ptr , 
                              DxUint32_t                  keyRingPasswordLen,
                              DxUint8_t*                  keyPassword_ptr , 
                              DxUint32_t                  keyPasswordLen,
                              DxUint32_t                  keyId,
                              KMNG_UserSpecificKeyData_t  UserSpecificKeyData,
                              DxUint8_t*                  keyRing_ptr);

                                       
/**
 * @brief     This function deletes a key from key ring
 *
 *
 * @param[in] keyRingPassword_ptr - address of the key ring password string
 * @param[in] keyRingPasswordLen - length of the key ring password
 * @param[in] userPassword_ptr - address of the key password string
 * @param[in] userPasswordLen - length of the key password
 * @param[in] keyId - the id of the key in the key ring
 * @param[in] keyRing_ptr - key ring 
 * 
 * @return     DxError_t:  
 *                        
 */                                          
DxError_t KMNG_DeleteKey(DxUint8_t*      keyRingPassword_ptr , 
                       DxUint32_t      keyRingPasswordLen,
                       DxUint8_t*      keyPassword_ptr , 
                       DxUint32_t      keyPasswordLen,
                       DxUint32_t      keyId,
                       DxUint8_t*      keyRing_ptr);
                                       
                                       
/**
 * @brief     This function transfers key from one key ring into another (actually decrypts it and then enctypts it with destination key ring key).
 *
 * @param[in] srcKeyRingPassword_ptr - password of the source key ring
 * @param[in] srcKeyRingPasswordLen - length of the password of the source key ring
 * @param[in] srcKeyPassword_ptr - password of the source key 
 * @param[in] srcKeyPasswordLen - length of the password of the source key
 * @param[in] dstKeyRingPassword_ptr - password of the destination key ring
 * @param[in] dstKeyRingPasswordLen - length of the password of the destination key ring
 * @param[in] dstKeyPassword_ptr - password of the destination key 
 * @param[in] dstKeyPasswordLen - length of the password of the destination key 
 * @param[in] srcKeyId - source key id
 * @param[out] dstKeyId_ptr - key id in the new key ring
 * @param[in] srcKeyRing_ptr -source key ring
 * @param[in] dstKeyRing_ptr - destination key ring
 *
 * @return     DxError_t:  
 *                        
 */                                
DxError_t KMNG_TransferKey(DxUint8_t*      srcKeyRingPassword_ptr , 
                         DxUint32_t      srcKeyRingPasswordLen,
                         DxUint8_t*      srcKeyPassword_ptr , 
                         DxUint32_t      srcKeyPasswordLen,
                         DxUint8_t*      dstKeyRingPassword_ptr , 
                         DxUint32_t      dstKeyRingPasswordLen,
                         DxUint8_t*      dstKeyPassword_ptr , 
                         DxUint32_t      dstKeyPasswordLen,
                         DxUint32_t      srcKeyId,
                         DxUint32_t*     dstKeyId_ptr,
                         DxUint8_t*      srcKeyRing_ptr,
                         DxUint8_t*      dstKeyRing_ptr);                                       
                                

/**
 * @brief     This function calculates the size of the buffer needed for storing the key ring on flash
 *
 * @param[in] keyRing_ptr - key ring
 * @param[out] bufferSize_ptr - the size of the buffer
 *
 * @return     DxError_t:  
 *                        
 */                                
DxError_t KMNG_GetFlashBufferSize(DxUint8_t*   keyRing_ptr,
                                DxUint32_t*  bufferSize_ptr);


/**
 * @brief    This function calculates the size of the buffer needed for storing the key ring
 *			 on flash.
 *			 The calculation is done according to the number of SymKeys, DHKeys and RSAKeys recived.
 *
 * @param[in] numSymKeys - number of symmetric keys in the key ring
 * @param[in] numSymKeys - number of RSA keys in the key ring
 * @param[in] numSymKeys - number of DH keys in the key ring
 * @param[out] bufferSize_ptr - the size of the buffer
 *
 * @return     DxError_t:  
 *                        
 */                                
DxError_t KMNG_CalcFlashBufferSize(DxUint32_t numSymKeys,
								   DxUint32_t numRSAKeys,
								   DxUint32_t numDHKeys,
								   DxUint32_t *bufferSize_ptr);

/**
 * @brief     This function stores the key ring into memory buffer
 *
 * @param[in] keyRing_ptr - key ring
 * @param[out] buffer_ptr - memory buffer
 * @param[in] bufferSize - the size of the memory buffer
 *
 * @return     DxError_t:  
 *                        
 */                                
DxError_t KMNG_SaveKeyRingToBuffer(DxUint8_t*    keyRing_ptr,
									DxUint8_t*    buffer_ptr,
									DxUint32_t	  bufferSize);                                      


/**
 * @brief     This function returns the parameters of the stores key ring:
 *			  number of SYM, RSA and DH key that was originally intended
 *            to be stored in the key ring
 *
 * @param[in] imageBuffer_ptr - image of the key ring 
 * @param[in] imageBufferSize - size of the image of the key ring
 * @param[out] numSymKeys_ptr - number of symmetric keys in the key ring
 * @param[out] numRSAKeys_ptr - number of RSA keys in the key ring
 * @param[out] numDHKeys_ptr - number of DH keys in the key ring
 *
 * @return     DxError_t:  
 *                        
 */                                
DxError_t KMNG_GetKeyRingDataFromImage(DxUint8_t*      imageBuffer_ptr,
                                       DxUint32_t      imageBufferSize,
                                       DxUint32_t*     numSymKeys_ptr,
                                       DxUint32_t*     numRSAKeys_ptr,
                                       DxUint32_t*     numDHKeys_ptr);
                                    
/**
 * @brief     This function restores the KeyRing from the flash image buffer into RAM buffer
 *
 * @param[in] imageBuffer_ptr - image of the key ring 
 * @param[in] imageBufferSize - size of the image of the key ring
 * @param[out] keyRing_ptr - key ring 
 * @param[in]  keyRingBufferLen - length of the key ring buffer
 *
 * @return     DxError_t:  
 *                        
 */                                
DxError_t KMNG_RestoreKeyRingFromImage(DxUint8_t*      imageBuffer_ptr,
                                     DxUint32_t      imageBufferSize,
                                     DxUint8_t*      keyRing_ptr,
                                     DxUint32_t      keyRingBufferLen);
                                    
                                    
/**
 * @brief     This function prepares the AES key for CRYS function usage based on the key ring AES key
 *
 * @param[in] keyRingPassword_ptr - password for access to key ring
 * @param[in] keyRingPasswordLen - key ring password length
 * @param[in] keyPassword_ptr - password for access to key data
 * @param[in] keyPasswordLen - key password length
 * @param[in] keyData_ptr - key data
 * @param[out]AES_WrappedKey_ptr - memory to stores created AES WrappedKey  
 * 
 * @return     DxError_t:  
 *                        
 */
DxError_t KMNG_ActivateAESKey(DxUint8_t*  keyRingPassword_ptr,
                            DxUint32_t  keyRingPasswordLen,
                            DxUint8_t*  keyPassword_ptr,
                            DxUint32_t  keyPasswordLen,
                            DxUint32_t  keyId,
                            DxUint8_t*  keyRing_ptr,
                            KMNG_AES_WrappedKey_t	AES_WrappedKey_ptr );

/**
 * @brief     This function prepares the DES key for CRYS function usage based on the key ring AES key
 *
 * @param[in] keyRingPassword_ptr - password for access to key ring
 * @param[in] keyRingPasswordLen - key ring password length
 * @param[in] keyPassword_ptr - password for access to key data
 * @param[in] keyPasswordLen - key password length
 * @param[in] keyData_ptr - key data
 * @param[out]DES_WrappedKey_ptr - memory to stores created DES WrappedKey  
 * 
 * @return     DxError_t:  
 *                        
 */
DxError_t KMNG_ActivateDESKey(DxUint8_t*  keyRingPassword_ptr,
                            DxUint32_t  keyRingPasswordLen,
                            DxUint8_t*  keyPassword_ptr,
                            DxUint32_t  keyPasswordLen,
                            DxUint32_t  keyId,
                            DxUint8_t*  keyRing_ptr,
                            KMNG_DES_WrappedKey_t  DES_WrappedKey_ptr );
/**
 * @brief     This function prepares the HMAC key for CRYS function usage based on the key ring AES key
 *
 * @param[in] keyRingPassword_ptr - password for access to key ring
 * @param[in] keyRingPasswordLen - key ring password length
 * @param[in] keyPassword_ptr - password for access to key data
 * @param[in] keyPasswordLen - key password length
 * @param[in] keyData_ptr - key data
 * @param[out]HMAC_WrappedKey_ptr - memory to stores created HMAC WrappedKey  
 * 
 * @return     DxError_t:  
 *                        
 */
DxError_t KMNG_ActivateHMACKey(DxUint8_t*  keyRingPassword_ptr,
                             DxUint32_t  keyRingPasswordLen,
                             DxUint8_t*  keyPassword_ptr,
                             DxUint32_t  keyPasswordLen,
                             DxUint32_t  keyId,
                             DxUint8_t*  keyRing_ptr,
                             KMNG_HMAC_WrappedKey_t	HMAC_WrappedKey_ptr);
                                     
/**
 * @brief     This function prepares the RSA key for CRYS function usage based on the key ring RSA key (It can be used both for encryption and decryption)
 *
 * @param[in] keyRingPassword_ptr - password for access to key ring
 * @param[in] keyRingPasswordLen - key ring password length
 * @param[in] keyPassword_ptr - password for access to key data
 * @param[in] keyPasswordLen - key password length
 * @param[in] keyData_ptr - key data
 * @param[out]RSA_WrappedKey_ptr - memory to stores created RSA WrappedKey  
 * 
 * @return     DxError_t:  
 *                        
 */
DxError_t KMNG_ActivateRSAKey(DxUint8_t*                keyRingPassword_ptr,
                            DxUint32_t                  keyRingPasswordLen,
                            DxUint8_t*                  keyPassword_ptr,
                            DxUint32_t                  keyPasswordLen,
                            DxUint32_t                  keyId,
                            KMNG_RsaKeyOperationType_t  rsaOperationType,
                            DxUint8_t*                  keyRing_ptr,
                            KMNG_RSA_WrappedKey_t	    RSA_WrappedKey_ptr);
                                    
                                    
/**
 * @brief     This function prepares the DH contest for CRYS function usage based on the key ring DH key
 *
 * @param[in] keyRingPassword_ptr - password for access to key ring
 * @param[in] keyRingPasswordLen - key ring password length
 * @param[in] keyPassword_ptr - password for access to key data
 * @param[in] keyPasswordLen - key password length
 * @param[in] keyData_ptr - key data
 * @param[out]DH_WrappedKey_ptr - memory to stores created DH WrappedKey  
 * 
 * @return     DxError_t:  
 *                        
 */
DxError_t KMNG_ActivateDHKey(DxUint8_t*  keyRingPassword_ptr,
                          DxUint32_t     keyRingPasswordLen,
                          DxUint8_t*     keyPassword_ptr,
                          DxUint32_t     keyPasswordLen,
                          DxUint32_t     keyId,
                          DxUint8_t*     keyRing_ptr,
                          KMNG_DH_WrappedKey_t	DH_WrappedKey_ptr);                                         

/**
 * @brief This function returns the KMNG version.
 *
 * The version contains the following fields:
 *
 * component string - a string describing the nature of the release.
 * release type: 'D' - development, 'A' - alpha (passed to QA) , 
 *                'R' - release, after QA testing.
 *
 * major, minor, sub, internal - the release digits.
 * 
 * @param[in] version_ptr - a pointer to the version structure.
 *
 */

 void  KMNG_GetVersion(KMNG_Version_t *version_ptr);

 /**
 * @brief     This function retrieve a wrapped key from the key ring given a key id.
 *
 * @param[in] keyRing_ptr - pointer to the key ring.
 * @param[in] keyId - the key id in the key ring.
 * @param[out] wrappedKey_ptr - pointer to the buffer to store the wrappedKey.
 * @param[in/out]wrappedKeySizeInBytes_ptr - get the length of the wrappedKey_ptr buffer.
 *											 return the actual size of the wrapped key.
 * @return     DxError_t:  
 *                        
 */
 DxError_t KMNG_GetKeyFromKeyRing (DxUint8_t*                 keyRing_ptr,             
                                   DxUint32_t                 keyId,                    
                                   DxUint8_t*                 wrappedKey_ptr,           
                                   DxUint32_t*                wrappedKeySizeInBytes_ptr);


 /**
 * @brief     This function retrieve a wrapped protection key from the key ring.
 *
 * @param[in] keyRing_ptr - pointer to the key ring.
 * @param[out] wrappedKey_ptr - pointer to the buffer to store the wrapped protection key.
 * @param[in/out]wrappedKeySizeInBytes_ptr - get the length of the wrappedKey_ptr buffer.
 *											 return the actual size of the wrapped key.
 * @return     DxError_t:  
 *                        
 */
 DxError_t KMNG_GetProtectionKeyFromKeyRing (DxUint8_t*                  keyRing_ptr,
                                             DxUint8_t*                  wrappedKey_ptr,
                                             DxUint32_t*                 wrappedKeySizeInBytes_ptr);

/**
 * @brief     This function store a wrapped key to the key ring, and return his id.
 *
 * @param[in] keyRing_ptr - pointer to the key ring.
 * @param[in] keyType - type of key.
 * @param[in] wrappedKey_ptr - pointer to the buffer of the wrappedKey.
 * @param[in] wrappedKeySizeInBytes_ptr - the length of the wrappedKey_ptr buffer.
 * @param[out]keyId_ptr - the returned id of this wrapped key in the key ring.
 *		
 * @return     DxError_t:  
 *                        
 */
DxError_t KMNG_StoreKeyInKeyRing (DxUint8_t*                  keyRing_ptr,
                                  KMNG_KeyType_t              keyType,
                                  DxUint8_t*                  wrappedKey_ptr,
                                  DxUint32_t                  wrappedKeySizeInBytes,
                                  DxUint32_t*                 keyId_ptr);
#endif
