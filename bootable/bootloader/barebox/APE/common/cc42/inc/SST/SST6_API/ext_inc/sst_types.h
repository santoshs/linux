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
 
 /*! \file 
**********************************************************************************	
* Title:						Discretix SST API Types header file						 					
*																			
* Filename:					    sst_types.h															
*																			
* Project, Target, subsystem:	SST 6.0, API, Types
* 
* Created:						29.03.2007														
*
* Modified:						07.06.2007										
*
* \Author						Raviv levi														
*																			
* \Remarks						
**********************************************************************************/

#ifndef _DX_SST_API_TYPES_H_
    #define _DX_SST_API_TYPES_H_
#ifdef __cplusplus
extern "C" {
#endif
	/*----------- Global Includes ------------------------------------------------*/
    #include "DX_VOS_BaseTypes.h" 	
	
	/*----------- Global defines -------------------------------------------------*/
	/* \brief the version component name size */
#ifndef DX_FW_COMPONENT_NAME_SIZE_IN_CHARS
    /*If the below structure was not defined by other modules defined it here else do nothing*/
		#define DX_FW_COMPONENT_NAME_SIZE_IN_CHARS (8UL)
	 /* @brief the version structure definition */
	 typedef struct
	 {
		 DxChar_t   componentName[DX_FW_COMPONENT_NAME_SIZE_IN_CHARS];
		 DxChar_t   typeVer;
		 DxUint32_t majorVer;
		 DxUint32_t minorVer;
		 DxUint32_t subVer;
		 DxUint32_t internalVer;   
	 }DXFWComponentVersion_t;

#endif //DX_FW_COMPONENT_NAME_SIZE_IN_CHARS

    /*----------- Global type definitions ----------------------------------------*/
	/*! \brief SST initialization information definition **/
	typedef struct 
	{
		/* [OUT] indication if the system recovered from an unexpected power down.*/
		DxBool_t powerDownRecover;

		/* [IN] indication if the SST should created a new DB or use an existing one.*/
		DxBool_t newDatabase;

		/* [IN] indication if SST should support multi-process behaviour. This has an 
		   affect on rollback from a process crash */
		DxUint32_t multiProcessSupport;

		/* [IN] A 32 bit process identifier. Must be unique per process and non-zero. */
		DxUint32_t sstProcessId;

        /* [IN] NVS cache size in items (TODO: should be in bytes) */
        DxUint32_t nvsCacheSize;
        
        /* [IN] indicate if cache is enabled or disabled */
        DxBool_t    useNvsCache;
    }SSTInitInfo_t;

     /*! \brief SST Session ID definition **/
     typedef DxUint32_t SSTSessionId_t;

     /*! \brief SST Transaction ID definition **/
     typedef DxUint32_t SSTTxnId_t;

	 /*! \brief SST Object Digest definition **/
	 typedef DxUint32_t SSTObjectDigest_t;

	 /*! \brief SST Object ID definition **/
	 typedef DxUint32_t SSTObjectId_t;

     /*! \brief SST Handle definition **/
     typedef struct  
     {
		 SSTObjectDigest_t	objDigest;
		 SSTObjectId_t		objId;
     }SSTHandle_t;

     /*! \brief SST Challenge ID **/
     typedef DxUint32_t SSTChallengeId_t;

     /*! \brief SST Iterator Cookie definition **/
	 typedef struct  
	 {
		DxUint32_t	prefixMsb;
		DxUint32_t	prefixLsb;
		DxUint32_t	prefixSize;
		DxUint32_t	lastPositionMsb;
		DxUint32_t	lastPositionLsb;
		DxBool_t	initialUse;
	 }SSTIteratorCookie_t;

     /*! \brief Cookie handle structure     **/
     typedef struct
     {
         SSTIteratorCookie_t cookieHandle;
     }SSTIXCookie_t;


	 typedef struct
	 {
		 DXFWComponentVersion_t sst;
	 }SSTVersion_t;

     /*! \brief SST data type definition **/
     typedef DxUint32_t SSTDataType_t;

     /*! \brief SST Access Permissions 
          (IMPORTANT - Permission with certain value automatically provides 
		   any permission of lower values) **/
     typedef enum
     {
         SST_PERMISSION_NONE      = 0, /*!< No access what so ever (may be used as default)**/
         SST_PERMISSION_READ_ONLY = 1, /*!< Read only                                      **/
         SST_PERMISSION_MODIFY    = 2, /*!< Modify - read/write, no delete                 **/
         SST_PERMISSION_CONTROL   = 3,  /*!< Control - read/write/delete/bind               **/

         SST_PERMISSIONS_FORCE_INT32= 0x7FFFFFFF /* force enum to 32 bit in all compilers */

     }SSTPermissions_t;

     /*!
     \brief This enumerator describes the different DES key types available.
     **/
     typedef enum 
     {
         SST_KEY_TYPE_1DES = 1,
         SST_KEY_TYPE_2DES = 2,
         SST_KEY_TYPE_3DES = 3,
         SST_DES_KEY_TYPE_FORCE_INT32= 0x7FFFFFFF /* force enum to 32 bit in all compilers */

     }SSTDESKeyType_t  ;

     /*!
     \brief This enumerator describes the different AES key types available.
     **/
     typedef enum 
     {
         SST_KEY_TYPE_AES_128_BIT      = 0,
         SST_KEY_TYPE_AES_192_BIT      = 1,
         SST_KEY_TYPE_AES_256_BIT      = 2,
         SST_KEY_TYPE_AES_512_BIT	   = 3,

         SST_AES_KEY_TYPE_FORCE_INT32  = 0x7FFFFFFF /* force enum to 32 bit in all compilers */

     }SSTAESKeyType_t;

	 /*!
     \brief This enumerator describes the different RSA key record types available.
     **/
     typedef enum 
     {
         SST_KEY_TYPE_RSA_ONLY_PUBLIC_KEY				= 0,
         SST_KEY_TYPE_RSA_ONLY_PRIVATE_KEY				= 1,
         SST_KEY_TYPE_RSA_BOTH_PUBLIC_AND_PRIVATE_KEY	= 2,

         SST_RSA_KEY_TYPE_FORCE_INT32                   = 0x7FFFFFFF /* force enum to 32 bit in all compilers */

     }SSTRSAKeyType_t;
				      

	 /*!
	 \brief This enumerator describes the different SST operations that require workspace.
	 **/
	 typedef enum 
	 {

		/*Init & Terminate operations*/
		SST_WORKSPACE_INIT_OP_GENERAL					= 0,/*All Init terminate APIs*/
		
		/*Authentication operations*/
		SST_WORKSPACE_AUTH_OP_OPEN						= 1,/*SST_AuthenticationOpen */
		SST_WORKSPACE_AUTH_OP_GENERAL					= 2,/*All other Authentication APIs*/

		/*Data Operations*/
		SST_WORKSPACE_DATA_OP_DELETE					= 3,/*SST_DataDelete*/
		SST_WORKSPACE_DATA_OP_GENERAL					= 4,/*All other Data Operation APIs*/

		/*Iteration and Indexing*/
		SST_WORKSPACE_TYPE_ITERATOR_OP_GENERAL			= 5,/*All Type iterator Operation APIs*/
		SST_WORKSPACE_INDEX_LOOKUP_OP_GENERAL			= 6,/*All Index lookup Operation APIs*/

		/*Key management Operations*/
		SST_WORKSPACE_KEY_MANAGEMENT_OP_RSA_INSERT_READ	= 7,/*SST_RSAPublicKeyInsert,SST_RSAPrivateKeyInsert,
															SST_RSAPairKeyInsert,SST_RSAPublicKeyRead,
															SST_RSAPrivateKeyRead,SST_CRTRSAPublicKeyInsert,
															SST_CRTRSAPrivateKeyInsert,SST_CRTRSAPairKeyInsert,
															SST_CRTRSAPublicKeyRead,SST_CRTRSAPrivateKeyRead*/
		SST_WORKSPACE_KEY_MANAGEMENT_OP_RSA_GENERATE	= 8,/*SST_RSAKeyGenerate,SST_CRTRSAKeyGenerate*/
		SST_WORKSPACE_KEY_MANAGEMENT_OP_GENERAL			= 9,/*All other key management Operation APIs*/

		/*General*/
		SST_WORKSPACE_GENERAL							= 10,/*For any SST API (max size among all other min)*/

		SST_WORKSAPCE_OP_FORCE_INT32                    = 0x7FFFFFFF /* force enum to 32 bit in all compilers */

	 }SSTWorkspaceOp_t;

     /*!
     \brief This enumerator describes the modes of SST.
     **/
     typedef enum 
     {
         SST_MODE_PRIVILEGED     = 0,
         SST_MODE_USER           = 1,
         SST_NUM_OF_MODES        = 2,

         SST_MODE_LAST           = 0x7FFFFFFF

     }SSTMode_t;

#ifdef __cplusplus
}
#endif

#endif  /* _DX_SST_API_TYPES_H_ */
