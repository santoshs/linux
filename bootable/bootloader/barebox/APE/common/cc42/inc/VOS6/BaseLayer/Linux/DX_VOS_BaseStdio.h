/***********************************************************************************  
 * Copyright 2009 © Discretix Technologies Ltd. 
 * This software is protected by copyright, international treaties and 
 * various patents. If the license governing the use of this Software 
 * allows copy or redistribution of this  software then any copy or 
 * reproduction of this Software must include this Copyright Notice 
 * as well as any other notices provided under such license. 
 ***********************************************************************************/
 
 
 #ifndef _DX_VOS_BASE_STDIO_H_
#define _DX_VOS_BASE_STDIO_H_
#include "VOS_API/DX_VOS_Stdio.h"
#include "VOS_API/DX_VOS_BaseTypes.h"
#include <stdio.h>
//#include <conio.h>

/*! Print formatted output to the standard output stream.
	Function signature is the same as DX_VOS_VPrintf().
	*/
#define DX_VOS_BaseVPrintf vprintf

DxUint32_t DX_VOS_BaseVFPrintf (DxVosFile aFile, const DxChar *format, DX_VA_LIST ap);
// TODO: nir - replaced for compilation, find non blocking version
#define DX_VOS_BaseGetCh getchar 

#endif
