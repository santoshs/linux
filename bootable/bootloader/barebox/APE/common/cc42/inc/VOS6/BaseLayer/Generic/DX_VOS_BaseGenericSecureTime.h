/***********************************************************************************  
 * Copyright 2009 © Discretix Technologies Ltd. 
 * This software is protected by copyright, international treaties and 
 * various patents. If the license governing the use of this Software 
 * allows copy or redistribution of this  software then any copy or 
 * reproduction of this Software must include this Copyright Notice 
 * as well as any other notices provided under such license. 
 ***********************************************************************************/
 
 
 #ifndef DX_VOS_BASE_SECURE_TIME_H
#define DX_VOS_BASE_SECURE_TIME_H

#include "DX_VOS_SecureTime.h"

#define DX_SECURE_TIME_ITEM_ID  0x00807060

#ifdef __cplusplus
extern "C"
{
#endif
    
DxStatus DX_VOS_BaseUnsetSecureTime(void);
DxStatus DX_VOS_BaseSetSecureTime(DxTime_t secsTime);
DxStatus DX_VOS_BaseGetSecureTime(DxTime_t* secsTime);

#ifdef  __cplusplus
}
#endif

#endif
