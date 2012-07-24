/*! \file 
**********************************************************************************	
* Title:                        Discretix SST IX header file
*
* Filename:                     sst_ix_types.h
*
* Project, Target, subsystem:   SST 6.0 Index Lookup Utility
* 
* Created:                      19.04.2007
*
* Modified:						07.06.2007 
*
* \Author                       Ira Boguslvasky
*
* \Remarks                      Copyright (C) 2007 by Discretix Technologies Ltd.
*                               All Rights reserved
**********************************************************************************/
#ifndef _DX_SST_IX_TYPES_H_
#define _DX_SST_IX_TYPES_H_

#include "sst_ix_config.h"
#include "sst_vdb.h"
#include "sst_types.h"


/*! \brief Index Meta-data object structure   **/
typedef struct
{
    /*!  Counter of Items which are existed in arrayOfIndexObjHandles (Index handles)  **/
    DxUint32_t          itemsCounter;                          
    /*!  Array of IND object handles **/
    SSTHandle_t         arrayOfIndexObjHandles[SST_IX_MAX_IND_IN_IMD];    
}SSTIXIndexMetaDataObject_t;

/*! \brief String field structure    **/
typedef struct  
{
    /*!  handle to the string  (array of characters)     **/
    SSTHandle_t         stringObjHandle;  
    /*!  the length of the string which corresponds to the stringObjHandle     **/
    DxUint32_t          stringLength;                                      
}SSTIXStringMD_t;

/*! \brief Index object structure     **/
typedef struct
{
    /*!   Number of strings in Index object, which are kept in the  arrayOfStrings  **/
    DxUint32_t          numberOfStrings;
    /*!   Consists of Array of strings and memory which the strings occupy **/
    SSTIXStringMD_t     arrayOfStrings[SST_IX_MAX_STRINGS_OBJ_COUNT ];
}SSTIXIndexObject_t;

typedef struct
{
    SSTIXIndexObject_t              indObj;
    SSTIXIndexMetaDataObject_t      imdObj;
}SSTIXGeneralObject_t;
#endif
