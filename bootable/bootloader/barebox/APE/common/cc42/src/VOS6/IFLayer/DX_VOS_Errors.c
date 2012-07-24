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

 #ifndef DX_NO_DEBUG_PRINT
#ifndef DX_DEBUG_PRINT
#define DX_DEBUG_PRINT
#endif
#include "DX_VOS_Errors.h"
#include "DX_VOS_ThreadStorage.h"
#include "DX_VOS_Mem.h"
#include "DX_VOS_Thread.h"

#define DX_MAX_NUM_OF_ERRORS_IN_STACK   50

typedef struct _DxErrorRecord
{
    const DxChar* m_FileName;
    const DxChar* m_FuncName;
    const DxChar* m_ErrorName;
    DxUint32 m_LineNum;
    DxUint32 m_ErrorCode;
} DxErrorRecord;

typedef struct _DxErrorsStack
{
    DxUint_t m_NumOfErrors;
    DxErrorRecord m_Errors[DX_MAX_NUM_OF_ERRORS_IN_STACK];
} DxErrorsStack;

static DxVosThreadStorage g_DxErrorStack = { 0 };
static DxVosThreadStorage g_DxErrorDisabled = { 0 };
static DxErrorsStack* _GetCurrThreadErrorStack(DxBool createIfNotExist)
{
    DxErrorsStack* errorStack = (DxErrorsStack*)DX_VOS_ThreadStorageGet(g_DxErrorStack);
    /* if (!DX_VOS_IsLogFileOpened()) */ /* WIP_NoOS_R_Loader */
        return DX_NULL;

    if (errorStack == DX_NULL && createIfNotExist)
    {
        if (DX_VOS_ThreadStorageGet(g_DxErrorDisabled))
            return DX_NULL;
        DX_VOS_ThreadStoragePut(g_DxErrorDisabled, (void*)DX_TRUE);
        errorStack = (DxErrorsStack*)DX_VOS_MemMalloc(sizeof(DxErrorsStack));
        if (errorStack == DX_NULL)
          return DX_NULL;
        errorStack->m_NumOfErrors = 0;
        DX_VOS_ThreadStoragePut(g_DxErrorStack, errorStack);
        DX_VOS_ThreadStoragePut(g_DxErrorDisabled, (void*)DX_FALSE);
    }
    return errorStack;
}

void DxErrorStack_Clear()
{
    DxErrorsStack* errorStack = _GetCurrThreadErrorStack(DX_FALSE);
    if (errorStack == DX_NULL)
        return;
    errorStack->m_NumOfErrors = 0;
}

void DxErrorStack_Add(const DxChar* fileName, DxUint32 lineNum, const DxChar* funcName, DxUint32 errorCode, const DxChar* errorName)
{
    DxErrorRecord* errorRecord = DX_NULL;
    DxErrorsStack* errorStack = DX_NULL;

    errorStack = _GetCurrThreadErrorStack(DX_TRUE);
    if (errorStack == DX_NULL)
        return;
    if (errorStack->m_NumOfErrors >= DX_MAX_NUM_OF_ERRORS_IN_STACK)
        return;
    errorRecord = errorStack->m_Errors + errorStack->m_NumOfErrors;
    errorRecord->m_FileName = fileName;
    errorRecord->m_LineNum = lineNum;
    errorRecord->m_FuncName = funcName;
    errorRecord->m_ErrorCode = errorCode;
    if (errorStack->m_NumOfErrors > 0)
    {
        DxErrorRecord* lastRecord = errorStack->m_Errors + errorStack->m_NumOfErrors - 1;
        if (lastRecord->m_ErrorCode == errorCode)
            errorName = lastRecord->m_ErrorName;
    }
    errorRecord->m_ErrorName = errorName;
    errorStack->m_NumOfErrors++;
}
static void DxErrorStack_LogStack(DxErrorsStack* errorStack, DxUint32 moduleCode, DxUint32 debugLevel)
{
    DxUint_t numOfErrors = 0;
    DxUint_t i = 0;
    numOfErrors = errorStack->m_NumOfErrors;
    errorStack->m_NumOfErrors = 0;
    /* for (i = 0; i < numOfErrors; i++)
    {
        DX_VOS_DebugPrint(moduleCode, errorStack->m_Errors[i].m_FileName, errorStack->m_Errors[i].m_LineNum,
            errorStack->m_Errors[i].m_FuncName, debugLevel, "Error: 0x%08X(%s)", errorStack->m_Errors[i].m_ErrorCode,
            errorStack->m_Errors[i].m_ErrorName);
    }*/ /* WIP_NoOS_R_Loader */
}

void DxErrorStack_Log(DxUint32 moduleCode, DxUint32 debugLevel)
{
    DxErrorsStack* errorStack = DX_NULL;
    errorStack = _GetCurrThreadErrorStack(DX_FALSE);
    if (errorStack == DX_NULL)
        return;
    if (errorStack->m_NumOfErrors == 0)
        return;
    DX_VOS_ThreadStoragePut(g_DxErrorDisabled, (void*)DX_TRUE);
    DX_VOS_ThreadStoragePut(g_DxErrorStack, DX_NULL);
    DxErrorStack_LogStack(errorStack, moduleCode, debugLevel);
    DX_VOS_ThreadStoragePut(g_DxErrorStack, errorStack);
    DX_VOS_ThreadStoragePut(g_DxErrorDisabled, (void*)DX_FALSE);

}

void DxErrorStack_Delete()
{
    DxErrorsStack* errorStack = (DxErrorsStack*)DX_VOS_ThreadStorageGet(g_DxErrorStack);
    if (errorStack != DX_NULL)
    {
        DX_VOS_MemFree(errorStack);
        DX_VOS_ThreadStoragePut(g_DxErrorStack, DX_NULL);
    }
}

#define DX_VOS_THREAD_MASK (DX_VOS_THREAD_RESOLUTION - 1)

void DxErrorStack_Terminate()
{
    DxUint32 threadId = DX_VOS_GetCurrThreadId() & DX_VOS_THREAD_MASK;
    DxUint_t i = 0;
    void* ptr = DX_NULL;
    for (i = 0; i < DX_VOS_THREAD_RESOLUTION; i++)
    {
        ptr = g_DxErrorStack[i];
        if (threadId == i)
            continue;
        if (ptr != DX_NULL)
            DxErrorStack_LogStack((DxErrorsStack*)ptr, DX_DBG_MODULE, DX_ERROR_PROPAGATION_LEVEL);
        g_DxErrorStack[i] = DX_NULL;
        DX_VOS_MemFree(ptr);
    }

    ptr = g_DxErrorStack[threadId];
    g_DxErrorStack[threadId] = DX_NULL;
    DX_VOS_MemFree(ptr);
}
#endif
