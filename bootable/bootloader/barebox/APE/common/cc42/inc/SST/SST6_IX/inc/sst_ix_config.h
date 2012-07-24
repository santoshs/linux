/*! \file 
**********************************************************************************    
* Title:                           Discretix SST IX header file                                             
*                                                                            
* Filename:                        sst_ix_config.h                                                            
*                                                                            
* Project, Target, subsystem:      SST 6.0 Index Lookup Utility
* 
* Created:                         17.04.2007                                                        
*
* Modified:                        07.06.2007                                       
*
* \Author                          Ira Boguslvasky                                                    
*                                                                            
* \Remarks                         Copyright (C) 2007 by Discretix Technologies Ltd.                 
*                                  All Rights reserved                                            
**********************************************************************************/

#ifndef _DX_SST_IX_CONFIG_H_
#define _DX_SST_IX_CONFIG_H_
 
#include "sst_config.h"

	/*Maximum number of available collisions (strings) in the index object
	collisions are when different strings are hashed to the same digest**/
	#define SST_IX_MAX_STRINGS_OBJ_COUNT		SST_MAX_STRINGS_COLLISION_OBJ_COUNT

	/* Maximum number of strings per record
		Note the actual number of strings could be larger in case of a hash 
		collision using different strings*/
	#define SST_IX_MAX_IND_IN_IMD               SST_MAX_STRINGS_PER_RECORD

    /*Index LookUp - Max number of handles to read when performing look up operation*/
    #define SST_IX_MAX_HANDLES_TO_READ						(15UL)

#endif  /* _DX_SST_IX_CONFIG_H_ */
