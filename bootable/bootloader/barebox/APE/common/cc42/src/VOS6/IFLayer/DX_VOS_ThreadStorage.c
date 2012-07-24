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

#include "DX_VOS_ThreadStorage.h"
#include "DX_VOS_Mem.h"
#include "DX_VOS_Thread.h"

#define DX_VOS_THREAD_MASK (DX_VOS_THREAD_RESOLUTION - 1)
void DX_VOS_ThreadStorageInit(DxVosThreadStorage threadStorage)
{
    DX_VOS_MemSetZero(threadStorage, sizeof(DxVosThreadStorage));
}

void DX_VOS_ThreadStoragePut(DxVosThreadStorage threadStorage, void* data)
{
    DxUint32 threadId = DX_VOS_GetCurrThreadId() & DX_VOS_THREAD_MASK;
    threadStorage[threadId] = data;
}

void* DX_VOS_ThreadStorageGet(DxVosThreadStorage threadStorage)
{
    DxUint32 threadId = DX_VOS_GetCurrThreadId() & DX_VOS_THREAD_MASK;
    return threadStorage[threadId];

}
