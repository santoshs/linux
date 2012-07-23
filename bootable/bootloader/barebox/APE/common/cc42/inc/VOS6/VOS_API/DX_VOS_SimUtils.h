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
 
 #ifndef DX_VOS_SIM_UTILS_H
#define DX_VOS_SIM_UTILS_H

#include "VOS_API/DX_VOS_BaseTypes.h"
#ifdef __cplusplus
extern "C"
{
#endif

#pragma pack(push, 1)
typedef struct  
{
    DxUint8 m_Class;
    DxUint8 m_Instruction;
    DxUint8 m_P1;
    DxUint8 m_P2;
    DxUint8 m_DataSize;
    DxUint8 m_Data[256];
} DxApdu;
#pragma pack(pop)

DxStatus DX_VOS_SimInit(const DxChar* cardReaderName);

void DX_VOS_SimTerminate();

DxStatus DX_VOS_ExecuteApdu(const DxApdu* cmd, DxUint32* status, void* responseData, DxUint32 dataMaxSize, DxUint32* dataActuallyRead);

#ifdef __cplusplus
}
#endif

#endif
