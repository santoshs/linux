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
 
 #ifndef _DX_VOS_SECURE_STORAGE_H
#define _DX_VOS_SECURE_STORAGE_H


#ifdef __cplusplus
extern "C"
{
#endif


#include "DX_VOS_BaseTypes.h"

typedef DxStatus (*DxSaveSecureItemFunc)(DxUint32 itemId, const void* data, DxUint32 dataSize);
typedef DxStatus (*DxGetSecureItemSizeFunc)(DxUint32 itemId, DxUint32* dataSize);
typedef DxStatus (*DxLoadSecureItemFunc)(DxUint32 itemId, void* data, DxUint32 dataSize, DxUint32* dataActuallyRead);
typedef DxStatus (*DxDeleteSecureItemFunc)(DxUint32 itemId);

DxStatus DX_VOS_SaveSecureItem(DxUint32 itemId, const void* data, DxUint32 dataSize);

DxStatus DX_VOS_GetSecureItemSize(DxUint32 itemId, DxUint32* dataSize);

DxStatus DX_VOS_LoadSecureItem(DxUint32 itemId, void* data, DxUint32 dataSize, DxUint32* dataActuallyRead);

DxStatus DX_VOS_DeleteSecureItem(DxUint32 itemId);

void DX_VOS_SetSaveSecureItemFunc(DxSaveSecureItemFunc saveSecureItemFunc);

void DX_VOS_SetGetSecureItemSizeFunc(DxGetSecureItemSizeFunc getSecureItemSizeFunc);

void DX_VOS_SetLoadSecureItemFunc(DxLoadSecureItemFunc loadSecureItemFunc);

void DX_VOS_SetDeleteSecureItemFunc(DxDeleteSecureItemFunc deleteSecureItemFunc);

#ifdef __cplusplus
}
#endif

#endif
