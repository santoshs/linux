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
 
 #ifndef _DX_VOS_THREAD_STORAGE_H
#define _DX_VOS_THREAD_STORAGE_H

#include "DX_VOS_BaseTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! \file DX_VOS_ThreadStorage.h
This module provide utilities for saving & retrieval of data to & from thread context.
Example:
    DxVosThreadStorage ThreadName;

    void Thread1()
    {
        DxChar* threadName = DX_NULL;
        DX_VOS_ThreadStoragePut(ThreadName, "Thread1");
        threadName = (DxChar*) DX_VOS_ThreadStorageGet(ThreadName);
        DX_VOS_PrintString(threadName); // Prints "Thread1"
    }

    void Thread2()
    {
    DxChar* threadName = DX_NULL;
    DX_VOS_ThreadStoragePut(ThreadName, "Thread2");
    threadName = (DxChar*) DX_VOS_ThreadStorageGet(ThreadName);
    DX_VOS_PrintString(threadName); // Prints "Thread2"
    }

    void Main()
    {
        DX_VOS_ThreadStorageInit(ThreadName);
        // Create Thread1
        // Create Thread2
    }
*/

typedef void* DxVosThreadStorage[DX_VOS_THREAD_RESOLUTION];

/* Initializes the thread storage */
void DX_VOS_ThreadStorageInit(DxVosThreadStorage threadStorage);

/* Saves the specified data in the thread's context */
void DX_VOS_ThreadStoragePut(DxVosThreadStorage threadStorage, void* data);

/* Retrieves the data that is stored in the thread's context */
void* DX_VOS_ThreadStorageGet(DxVosThreadStorage threadStorage);
#ifdef __cplusplus
}
#endif

#endif
