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
 
 #ifndef _DX_VOS_MOBILE_UTILS_H
#define _DX_VOS_MOBILE_UTILS_H

/*! \file DX_VOS_MobileUtils.h
This module provides Mobile devices information retrieval utilities.

All function can return one of the follwing values:
- DX_SUCCESS - if value was retrieved successfully.
- DX_BUFFER_IS_NOT_BIG_ENOUGH - if data could not fit supplied buffer.
- DX_VOS_MOBILE_UTILS_ERROR - if retrieval failed.
*/
#include "DX_VOS_BaseTypes.h"

#ifdef __cplusplus
extern "C" {
#endif
/*!	Retrieves subscriber ID. */
DxStatus DX_VOS_GetCurrentIMSI(DxChar* aBuff, DxUint aBuffSize);

/*! Retrieves Device Manufacturer ID*/
DxStatus DX_VOS_GetDevManufacturerId(DxChar* aBuff, DxUint aBuffSize);

/*! Retrieves Device Model */
DxStatus DX_VOS_GetDevModel(DxChar* aBuff, DxUint aBuffSize);

/*! Retrieves Device Version */
DxStatus DX_VOS_GetDevVersion(DxChar* aBuff, DxUint aBuffSize);

#ifdef __cplusplus
}
#endif
#endif
