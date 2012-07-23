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

 #include "VOS_API/DX_VOS_MobileUtils.h"
#include "VOS_API/DX_VOS_String.h"
#define DX_IMSI                      "Dummy1234567890"
#define DX_DEVICE_MANUFACTURER_ID    "Discretix Inc"
#define DX_DEVICE_MODEL              "OMA DRM AGENT"
#define DX_DEVICE_VERSION            "V-2.0"

DxStatus DX_VOS_GetCurrentIMSI(DxChar* aBuff, DxUint_t aBuffSize)
{
	RETURN_VAR_STATUS(DX_VOS_StrNCopy(aBuff, aBuffSize, DX_IMSI));
}

DxStatus DX_VOS_GetDevManufacturerId(DxChar* aBuff, DxUint_t aBuffSize)
{
	RETURN_VAR_STATUS(DX_VOS_StrNCopy(aBuff, aBuffSize, DX_DEVICE_MANUFACTURER_ID));
}

DxStatus DX_VOS_GetDevModel(DxChar* aBuff, DxUint_t aBuffSize)
{
	RETURN_VAR_STATUS(DX_VOS_StrNCopy(aBuff, aBuffSize, DX_DEVICE_MODEL));
}

DxStatus DX_VOS_GetDevVersion(DxChar* aBuff, DxUint_t aBuffSize)
{
	RETURN_VAR_STATUS(DX_VOS_StrNCopy(aBuff, aBuffSize, DX_DEVICE_VERSION));
}
