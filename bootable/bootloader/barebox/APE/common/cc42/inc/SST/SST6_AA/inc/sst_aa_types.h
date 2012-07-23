/*! \file 
**********************************************************************************	
* Title:						Discretix SST AA Type Definitions Header File						 					
*																			
* Filename:						sst_aa_types.h 															
*																			
* Project, Target, subsystem:	SST 6.0, A&A
* 
* Created:						05.02.2007														
*
* Modified:						07.06.2007										
*
* \Author						Raviv levi														
*																			
* \Remarks						
*           Copyright (C) 2007 by Discretix Technologies Ltd. All Rights reserved.
**********************************************************************************/

#ifndef _DX_SST_AA_TYPES_H_
    #define _DX_SST_AA_TYPES_H_

	#include "DX_VOS_Sem.h"

    #if defined EXTERN
        #error EXTERN ALREADY DEFINED !!! (sst_aa_types.h) 
    #else
        #if defined _DX_SST_AA_NO_EXTERN_
            #define EXTERN 
        #else
            #define EXTERN extern
        #endif /* _DX_SST_AA_NO_EXTERN_*/
    #endif /*EXTERN*/


    #include "sst_aa_config.h"
    #include "sst_aa_def.h"
    #include "sst_aa.h"
    #include "sst_vdb.h"
    
    /*----------- Local type definitions -----------------------------------------*/
    
    /*! \brief SST AA salt definition, used for HASH calculations **/
    //typedef DxByte_t    SSTAASalt_t[SST_AA_SALT_SIZE_IN_BYTES];
    // todo - remove from doc, no need to hash if not kept as plain.


    /*! \brief SST AA Authentication type enumeration **/
    typedef enum
    {
        SST_AA_AUTH_TYPE_AES_SS		=0,	/*AES shared secret authentication*/
        SST_AA_AUTH_TYPE_PK_RSA_SV	=1,	/*RSA Public Key sign-verify authentication*/ 
        SST_AA_AUTH_TYPE_PWD		=2,	/*Password authentication*/
		SST_AA_AUTH_TYPE_PK_ECC_SV	=3,	/*ECC Public Key sign-verify authentication*/
        SST_AA_AUTH_TYPE_APP_ID		=4, /*Application ID authenticator*/
        SST_AA_AUTH_TYPE_CLOSE		=5, /*Close authenticator - can not be open*/
    }SSTAAAuthenticationType_t;
    
    /*! \brief SST AA Meta Data definition **/
    typedef struct  
    {
        /* The handle of the object referred to by this meta data object*/
        SSTHandle_t    			objHandle;

		/*! \brief SST AA bound auth' information (auth' handle, permission and reference counter)*/

        /* Auth's that are bound to the data object */

        /*!  An array of authenticator handles that can allow access ti this object **/
        SSTHandle_t    			handle[SST_AA_MAX_AUTHS_PER_OBJ];

        /*! The corresponding access permission that each authenticator can allow  **/
        DxByte_t      			permission[SST_AA_MAX_AUTHS_PER_OBJ];

		DxByte_t				padding[10];

    }SSTAAMetaData_t;
    
    /*! \brief SST AA Session definition **/
    typedef struct  
    {
        /* An array that keeps the handles of the authenticators
           that are currently open in this session          */
        SSTHandle_t    authHandles[SST_AA_MAX_AUTHS_PER_SESSION];

        /* The ID of this session */
        SSTSessionId_t        sessionId;

    }SSTAASession_t;

    /*! \brief SST AA authenticator data definition (common to all authenticators) **/
    typedef struct  
    {
        /* The authentication type */
        SSTAAAuthenticationType_t   authType;

        /* The amount of objects that rely on / use this authenticator */
        DxUint32_t                  refCount;

        /* The mode that the authenticator can be open in */
        SSTMode_t                   authMode;

    } SSTAAAuthData_t;

    /*! \brief SST AA auth' first block, used to retrieve only the auth' general info from NVS **/
    typedef struct
    {
        SSTAAAuthData_t authData;
        DxByte_t        padd[SST_VDB_MIN_BLOCK_TO_READ - sizeof(SSTAAAuthData_t)];
    }SSTAAAuthFirstBlock_t;
    
    /*! \brief SST AA password type authenticator definition **/
    typedef struct  
    {
        /* General authenticator data - MUST BE THE FIRST FIELD todo - because partial retrieve in the delete, update doc*/
        SSTAAAuthData_t authData;
        
        /* The password change/reset authenticator */
        SSTHandle_t    masterAuthHandle;

        /* The password's hash */
        SSTVCRYSHASHResult_t  pwdHash;

        /*
         * No need to add salt to the password hash since the hash is not
         * stored as plain text but encrypted.
         * (equivalent to using salt with the AES key size)
         */
        
        /* The number of consecutive erroneous attempts to open this authenticator */ 
        DxUint32_t  faultyTries;

        /* The maximal number of consecutive erroneous attempts to open this 
           authenticator. (after which the authenticator will be locked)          
           0 indicates that this feature is disabled                        */
        DxUint32_t  maxRetry;        

    }SSTAAAuthenticatorPWD_t;

    /*! \brief SST AA AES shared secret type authenticator definition **/
    typedef struct  
    {
        /* General authenticator data  - MUST BE THE FIRST FIELD*/
        SSTAAAuthData_t authData;

        SSTAESKeyType_t keyType;

        /* The AES shared secret (the AES Key) */
        DxByte_t        aesKey[SST_AA_AES_KEY_SIZE_IN_BYTES];        
    
    }SSTAAAuthenticatorAES_t;

    /*! \brief SST AA RSA public key type authenticator definition **/
    typedef struct  
    {
        /* General authenticator data  - MUST BE THE FIRST FIELD*/
        SSTAAAuthData_t authData;

        /* The value of the modulus (the exponent is constant) */
        DxByte_t     modulus[SST_AA_PK_MOD_SIZE_IN_BYTES];
        
        DxByte_t     exponent[SST_AA_PK_EXP_SIZE_IN_BYTES];

        DxUint32_t   exponentSize;        
    
    }SSTAAAuthPubKeyRSA_t;

	/*! \brief SST AA ECC public key type authenticator definition **/
	typedef struct  
	{
		/* General authenticator data  - MUST BE THE FIRST FIELD*/
		SSTAAAuthData_t authData;

		/* The value of the public key */
		DxByte_t     publicKey[SST_AA_ECC_PK_MAX_SIZE_IN_BYTES];

		DxUint32_t   PublicKeySizeInBytes;        

	}SSTAAAuthPubKeyECC_t;

    /*! \brief SST AA internal session table definition **/
    typedef SSTAASession_t SSTAASessionTable_t[SST_AA_MAX_OPEN_SESSIONS];

    /*! \brief SST AA Challenge definition **/
    typedef struct  
    {
        /* An array that keeps the pending challenge */
        DxByte_t          challenge[SST_AA_CHALLENGE_SIZE_IN_BYTES];

        /* The ID of this challenge */
        SSTChallengeId_t  challengeId;

    }SSTAAChallenge_t;
    
    /*! \brief SST AA internal pending challenge table definition **/
    typedef SSTAAChallenge_t SSTAAChallengeTable_t[SST_AA_MAX_PENDING_CHALLENGES];

    /*! \brief SST AA internal run time variables **/
	typedef struct  
	{
	    SSTAASessionTable_t     openSessions;
	    SSTAAChallengeTable_t   pendingChallenges;
	    DxBool_t                isActive;
        DxVosSem                sessionTableSem;
        DxVosSem                challengeTableSem;
            
        #if defined SST_AA_DEBUG_MODE_ENABLED
            // Debug related run time variables.
        #endif
    	
	} SSTAARuntimeVars_t; 
		
	/*----------- module global variables --------------------------------------*/
    EXTERN SSTAARuntimeVars_t g_sstAaVars;

    #undef EXTERN

#endif  /* _DX_SST_AA_TYPES_H_ */
