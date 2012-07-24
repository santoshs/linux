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
 
 #ifndef _DX_VOS_THREAD_H
#define _DX_VOS_THREAD_H

/*! \file DX_VOS_thread.h
    \brief This module enables thread operations 
*/

#include "DX_VOS_BaseTypes.h"
#include "DX_VOS_config.h"

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct _ThreadParams*  DxVosThread;
typedef DxUint (*DxThreadFunc)(void*);

typedef enum {
	DX_VOS_PRIORITY_IDLE, 
	DX_VOS_PRIORITY_VERY_LOW, 
	DX_VOS_PRIORITY_LOW, 
	DX_VOS_PRIORITY_NORMAL, 
	DX_VOS_PRIORITY_HIGH, 
	DX_VOS_PRIORITY_VERY_HIGH, 
	DX_VOS_NUMBER_OF_PRIORITIES
} DX_VOS_ThreadPriority_t;

/*! Creates a new thread. 
 \return
 - DX_SUCCESS on success
 - DX_BAD_ARGUMENTS - if aThreadEntry is NULL.
 - DX_VOS_THREAD_ERROR - if operation fails.
 */

IMPORT_C DxStatus DX_VOS_ThreadCreate(
    DxVosThread *aThreadHandle,			/*!< [out] Handle for the created thread. This parameter may be NULL. */
    DxThreadFunc aThreadEntry,			/*!< [in] pointer to thread function. */
    void* aThreadParam,					/*!< [in] pointer that will be passed to the thread function.
                                        This parameter can be NULL.*/
	const DxChar *aThreadName,			/*!< [in]  New thread name. for debugging purpose only. 
											This parameter may be NULL. */
	DX_VOS_ThreadPriority_t aPriority,	/*!< [in]  Priority of new thread */
	void *aThreadStack,					/*!< [in]  Stack pointer of the thread. The stuck pointer
											is only a recommendation for the OS. The OS may use
											other memory for the thread stack. If NULL OS should
											allocate stack memory by itself. */
	DxUint32 aStackSize				/*!< [in]  Size of stack. If 0 the OS will use its default stack size. */
	);

/*! Brutally terminates the specified thread */
//IMPORT_C DxStatus DX_VOS_ThreadTerminate(DxVosThread aThreadHandle);

/*! Brutally terminates the specified thread.
    On exit *aThreadHandle will be DX_NULL.
*/
//IMPORT_C DxStatus DX_VOS_ThreadDestroy(DxVosThread* aThreadHandle);

/*! Suspends the current thread for sleepMs milliseconds and then resumes it. */
IMPORT_C void DX_VOS_Sleep ( 
	DxUint32 sleepMs	/*!< [in] The number of milliseconds to suspend the thread */
	);

/*! Returns the ID of the current Thread. */
IMPORT_C DxUint32 DX_VOS_GetCurrThreadId (void);

/*! Returns the ID of the current Process. */
IMPORT_C DxUint32 DX_VOS_GetCurrProcessId(void);

#ifdef DX_USE_LEGACY_VOS
typedef DxVosThread  DxVosTask_ptr;
typedef DxThreadFunc Dx_VosTaskEntryPoint_ptr;
#define DX_VOS_TaskCreate(aTaskName, aPriority, aTaskStack, aStackSize, aTaskEntry, aTaskParam, aStartFlag, aTaskHandle) \
    DX_VOS_ThreadCreate(aTaskHandle, aTaskEntry, aTaskParam,aTaskName, aPriority, aTaskStack, aStackSize)
#define DX_VOS_TaskTerminate    DX_VOS_ThreadTerminate
#define DX_VOS_TaskSleep        DX_VOS_Sleep
#define DX_VOS_GetCurrTaskId    DX_VOS_GetCurrThreadId
#endif

#ifdef  __cplusplus
}
#endif


#endif /* ifndef _DX_VOS_THREAD_H */
