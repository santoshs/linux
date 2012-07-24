/*! \file 
**********************************************************************************	
* Title:						Discretix SST General Handle Definitions Header File	 					
*																			
* Filename:						sst_general_def_handle.h 															
*																			
* Project, Target, subsystem:	SST 6.0 General
* 
* Created:						29.03.2007														
*
* Modified:						07.06.2007									
*
* \Author						Einat Ron														
*																			
* \Remarks						Copyright (C) 2006 by Discretix Technologies Ltd.     			
*  								All Rights reserved											
**********************************************************************************/

#ifndef _DX_SST_GENERAL_DEF_HANDLE_H_
    #define _DX_SST_GENERAL_DEF_HANDLE_H_
	
    /*----------- Local type definitions -----------------------------------------*/
	/************************************************************************/
	/* SST Handle structure:                                                */
	/*  <Record Type field :8><I/E type :1><User defined part :23>          */
	/*  OR                                                                  */
    /*  <Record   Properties    field   :9><User defined part :23>          */
    /* Notes:                                                               */
	/*      1. The I/E (Internal/External) bit denotes whether the record   */
	/*         is an SST internal object or not                             */
	/*         (affects the type interpretation, 1 - Internal).             */
	/*      2. The user defined part can be randomly generated upon request.*/
	/*      3. Both structure are equivalent! the second one is used to     */
	/*         encapsulate the type and the internal / external fields.     */
	/************************************************************************/
	
	/* Handle related macros:*/
	
	/* Repeated definition to avoid the need for API include on one handle and exposure of this file to the user on the other.*/
    #ifndef SST_HANDLE_RANDOM_GENERATE
        #define SST_HANDLE_RANDOM_GENERATE                ((SSTObjectId_t)0xFFFFFFFF)
    #endif


    /* Creates a 32 bits mask of contiguous 1's.
      (example for size 4 and shift 2: 1<<4 = 16 -->0x10-1=0xF --> 0xF<<4=0xF0) */
    #define SST_32_BIT_CONT_MASK_CREATE(size,shift)       ((DxUint32_t)((0x1<<(size))-1)<<(shift))
    
    /* Bit maneuvering utilities.*/
    #define SST_TURN_BITS_ON(reg, mask)			          ((reg) |= (mask))
    #define SST_TURN_BITS_OFF(reg, mask)		          ((reg) &= ~(mask))
    #define SST_BITS_ARE_ON(reg, mask)                    (((reg) & (mask))==(mask))

    /* The offset of the properties field.*/
    #define SST_HANDLE_PROPERTIES_FIELD_OFFSET      SST_HANDLE_SIZE_OF_USER_FIELD_IN_BITS
    
    /* The mask of the user field.*/ 
    #define SST_HANDLE_USER_FIELD_MASK                                    \
        SST_32_BIT_CONT_MASK_CREATE(SST_HANDLE_SIZE_OF_USER_FIELD_IN_BITS,0)

    /* The mask of the type field (*DOES NOT* include the Internal/External portion)*/
    #define SST_HANDLE_TYPE_FIELD_MASK                                    \
        SST_32_BIT_CONT_MASK_CREATE(SST_HANDLE_SIZE_OF_TYPE_FIELD_IN_BITS,\
                                    SST_HANDLE_SIZE_OF_USER_FIELD_IN_BITS+\
                                    SST_HANDLE_SIZE_OF_INT_EXT_FIELD_IN_BITS)

    /* The mask of the internal/external field */
    #define SST_HANDLE_INT_EXT_FIELD_MASK                                    \
        SST_32_BIT_CONT_MASK_CREATE(SST_HANDLE_SIZE_OF_INT_EXT_FIELD_IN_BITS,\
                                    SST_HANDLE_SIZE_OF_USER_FIELD_IN_BITS)

    /* The mask of the user properties field*/
    #define SST_HANDLE_PROPERTIES_FIELD_MASK                                \
        (SST_HANDLE_INT_EXT_FIELD_MASK|SST_HANDLE_TYPE_FIELD_MASK)
   
    /* Return the record type according to the handle. 
       (this macro *DOES NOT* determine if Internal or External as we skip this bit)*/
 
    #define SST_HANDLE_TYPE_FIELD_GET(handle)       ((handle)>>             \
    (SST_HANDLE_SIZE_OF_USER_FIELD_IN_BITS+SST_HANDLE_SIZE_OF_INT_EXT_FIELD_IN_BITS))

    /* Set the record type according to the requested value. 
      (this macro *DOES NOT* set Internal or External as we skip this bit)*/
    #define SST_HANDLE_TYPE_FIELD_SET(handle,typeValue)                   \
        {SSTHandleTypeField_t shiftedType = typeValue;                    \
         SST_TURN_BITS_OFF((handle),SST_HANDLE_TYPE_FIELD_MASK);          \
         shiftedType = typeValue << (SST_HANDLE_SIZE_OF_USER_FIELD_IN_BITS\
                + SST_HANDLE_SIZE_OF_INT_EXT_FIELD_IN_BITS);              \
         SST_TURN_BITS_OFF((shiftedType),~SST_HANDLE_TYPE_FIELD_MASK);    \
         SST_TURN_BITS_ON((handle),(shiftedType));}

    
    /* Return the record user field according to the handle.*/
    #define SST_HANDLE_USER_FIELD_GET(handle)       ((handle)&SST_HANDLE_USER_FIELD_MASK)
    
    /* Set the record user field according to the requested value.*/
    #define SST_HANDLE_USER_FIELD_SET(handle,userValue)             \
        {SST_TURN_BITS_OFF((handle),SST_HANDLE_USER_FIELD_MASK);    \
         SST_TURN_BITS_OFF((userValue),~SST_HANDLE_USER_FIELD_MASK);\
         SST_TURN_BITS_ON((handle),(userValue));}
    
    /* return a boolean value that indicates whether this handle
        is of an internal record or not.*/
    #define SST_HANDLE_IS_INTERNAL(handle)          (((handle)&SST_HANDLE_INT_EXT_FIELD_MASK)!=0x0)
    
    /* Return the record internal/external field according to the handle.*/
    #define SST_HANDLE_INT_EXT_FIELD_GET(handle)       ((handle)&SST_HANDLE_INT_EXT_FIELD_MASK)
    
    /* Set the record internal/external field according to the requested value.*/
    #define SST_HANDLE_INT_EXT_FIELD_SET(objId,intExtValue)                                  \
        {SSTHandleIntExtField_t ie = ((intExtValue)<<SST_HANDLE_SIZE_OF_USER_FIELD_IN_BITS); \
         SST_TURN_BITS_OFF((objId),SST_HANDLE_INT_EXT_FIELD_MASK);                           \
         SST_TURN_BITS_OFF(ie,~SST_HANDLE_INT_EXT_FIELD_MASK);                               \
         SST_TURN_BITS_ON((objId),ie);}
         
    /* Retrieve the internal / external property from the handle properties field.*/
    #define SST_HANDLE_PROPERTIES_IE_GET(propertiesField)                \
        ((propertiesField&SST_HANDLE_INT_EXT_FIELD_MASK)>>SST_HANDLE_SIZE_OF_USER_FIELD_IN_BITS)

    /* Retrieve the record type property from the handle properties field.*/
    #define SST_HANDLE_PROPERTIES_TYPE_GET(propertiesField)              \
    ((propertiesField&SST_HANDLE_TYPE_FIELD_MASK)>>                      \
        (SST_HANDLE_SIZE_OF_USER_FIELD_IN_BITS+SST_HANDLE_SIZE_OF_INT_EXT_FIELD_IN_BITS))

    /* Construct the handle properties from the type and internal/external fields.*/
    #define SST_HANDLE_PROPERTIES_GET(typeField,ieField)                                            \
        (((((typeField) << (SST_HANDLE_SIZE_OF_USER_FIELD_IN_BITS+                                  \
                          SST_HANDLE_SIZE_OF_INT_EXT_FIELD_IN_BITS))&SST_HANDLE_TYPE_FIELD_MASK)|   \
         (((ieField) << SST_HANDLE_SIZE_OF_USER_FIELD_IN_BITS)&SST_HANDLE_INT_EXT_FIELD_MASK))		\
		 >>SST_HANDLE_PROPERTIES_FIELD_OFFSET)

    /* Extract the handle properties from an object ID.*/
    #define SST_HANDLE_PROPERTIES_EXTRACT(objIdInput,propertiesOutput)                              \
    { (propertiesOutput)=(objIdInput);                                                              \
      SST_TURN_BITS_OFF((propertiesOutput),~SST_HANDLE_PROPERTIES_FIELD_MASK);                      \
        (propertiesOutput) >>= SST_HANDLE_PROPERTIES_FIELD_OFFSET;                                  \
    }

    /* Set a handle's property.*/
    #define SST_HANDLE_PROPERTIES_SET(handle,properties)                                            \
        {SSTHandlePropertiesField_t shiftedProperties = properties;                                 \
         shiftedProperties <<= SST_HANDLE_PROPERTIES_FIELD_OFFSET;                                  \
         SST_TURN_BITS_OFF((shiftedProperties),~SST_HANDLE_PROPERTIES_FIELD_MASK);                  \
         SST_TURN_BITS_OFF((handle),SST_HANDLE_PROPERTIES_FIELD_MASK);                              \
         SST_TURN_BITS_ON((handle),shiftedProperties);}
    
	/*SST Handle Copy*/
	#define SST_HANDLE_COPY(targetHandle,sourceHandle)			\
			targetHandle.objDigest	= sourceHandle.objDigest;	\
			targetHandle.objId		= sourceHandle.objId 

    /* SST Handle Internal / External definitions*/
    #define SST_HANDLE_INTERNAL                     ((SSTHandleIntExtField_t)0x1)
    #define SST_HANDLE_EXTERNAL                     ((SSTHandleIntExtField_t)0x0)
    
    /* SST Internal Record Types */
    #define SST_RECORD_TYPE_AUTH_PWD                ((SSTHandleTypeField_t)0xA0) 
    #define SST_RECORD_TYPE_AUTH_AES_SS             ((SSTHandleTypeField_t)0xA1) 
    #define SST_RECORD_TYPE_AUTH_PK_RSA_SV          ((SSTHandleTypeField_t)0xA2) 
	#define SST_RECORD_TYPE_AUTH_PK_ECC_SV          ((SSTHandleTypeField_t)0xA3) 
    #define SST_RECORD_TYPE_AUTH_APP_ID             ((SSTHandleTypeField_t)0xA4) 
    #define SST_RECORD_TYPE_AUTH_CLOSE              ((SSTHandleTypeField_t)0xAB) 
    #define SST_RECORD_TYPE_AA_INTERNAL             ((SSTHandleTypeField_t)0xAF) /* denotes empty auths' that indicate empty entries etc...*/
    
    #define SST_RECORD_TYPE_IMD_OBJ                 ((SSTHandleTypeField_t)0xB2)       
    #define SST_RECORD_TYPE_IND_OBJ                 ((SSTHandleTypeField_t)0xB3)       
    #define SST_RECORD_TYPE_STRING_OBJ              ((SSTHandleTypeField_t)0xB4)       
    
    #define SST_RECORD_TYPE_DATA                    ((SSTHandleTypeField_t)0xDD) /*a record that is pointed to by a meta data object*/

	#define SST_HANDLE_DIGEST_DEFAULT				((SSTObjectDigest_t)0xDBDBDBDB) // rlv: todo, change to SST_DATA_HANDLE_DIGEST

#endif  /* _DX_SST_GENERAL_DEF_HANDLE_H_ */
