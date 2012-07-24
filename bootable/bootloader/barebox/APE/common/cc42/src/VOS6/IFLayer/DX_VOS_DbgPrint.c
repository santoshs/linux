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
#include "VOS_API/DX_VOS_BaseTypes.h"
#include "VOS_API/DX_VOS_DbgPrint.h"
#include "VOS_API/DX_VOS_Stdio.h"
#include "VOS_API/DX_VOS_Time.h"
#include "VOS_API/DX_VOS_Mem.h"
#include "VOS_API/DX_VOS_ThreadStorage.h"
#include "VOS_API/DX_VOS_Thread.h"
#include "VOS_API/DX_VOS_String.h"
#include "VOS_API/DX_VOS_Socket.h"

static DxBool DX_VOS_DbgPrint_ShouldPrintFileName = DX_TRUE;
static DxBool DX_VOS_DbgPrint_ShouldPrintLineNum = DX_TRUE;
static DxBool DX_VOS_DbgPrint_ShouldPrintFuncName = DX_TRUE;
static const DxChar* DX_VOS_DbgPrint_TimeFormat = "%02d:%02d:%02d";
static const DxChar* DX_VOS_DbgPrint_DateFormat = "%02d.%02d.%02d";
static DxUint32 DX_VOS_DbgPrint_DebugLevel = 0xFFFFFFFFUL;
static DxPrintfFunc_t DX_VOS_DbgPrint_PrintFunc = DX_VOS_DebugFilePrint;

static DxUint32 DX_VOS_DbgPrint_ModulesMask = 0xFFFFFFFF;

static DxChar g_DxDebugFileName[DX_VOS_MAX_PATH];
static DxVosThreadStorage g_DxDebugFile = { 0 };
static DxVosThreadStorage g_LastDbgFlash = { 0 };
static DxVosThreadStorage g_DbgPrintDisabled = { 0 };
static DxVosSocket g_DxDebugSocket = DX_INVALID_SOCKET;
static DxChar* g_DxSocketBuffer = DX_NULL;
static DxChar* g_DxSocketFileName = DX_NULL;
static const DxUint32 g_DxSocketBufferSize = 2048;
static const DxUint32 g_DxSocketInfoSentTime = 0;
static const DxUint32 g_DxSocketInfoSentDelta = 2000;
static DxBool g_DxSocketDebugActive = DX_FALSE;

DxBool DX_VOS_DbgPrint_SetShouldPrintFileName(DxBool value)
{
	DxBool temp = DX_VOS_DbgPrint_ShouldPrintFileName;
	DX_VOS_DbgPrint_ShouldPrintFileName = value;
	DX_RETURN(temp);
}

DxBool DX_VOS_DbgPrint_SetShouldPrintLineNum(DxBool value)
{
	DxBool temp = DX_VOS_DbgPrint_ShouldPrintLineNum;
	DX_VOS_DbgPrint_ShouldPrintLineNum = value;
	DX_RETURN(temp);
}

DxBool DX_VOS_DbgPrint_SetShouldPrintFuncName(DxBool value)
{
	DxBool temp = DX_VOS_DbgPrint_ShouldPrintFuncName;
	DX_VOS_DbgPrint_ShouldPrintFuncName = value;
	DX_RETURN(temp);
}

const DxChar* DX_VOS_DbgPrint_SetTimeFormat(const DxChar* value)
{
	const DxChar* temp = DX_VOS_DbgPrint_TimeFormat;
	DX_VOS_DbgPrint_TimeFormat = value;
	DX_RETURN(temp);
}

const DxChar* DX_VOS_DbgPrint_SetDateFormat(const DxChar* value)
{
	const DxChar* temp = DX_VOS_DbgPrint_DateFormat;
	DX_VOS_DbgPrint_DateFormat = value;
	DX_RETURN(temp);
}

DxUint32 DX_VOS_DbgPrint_SetDebugLevel(DxUint32 value)
{
	DxUint32 temp = DX_VOS_DbgPrint_DebugLevel;
	DX_VOS_DbgPrint_DebugLevel = value;
	DX_RETURN(temp);
}

DxPrintfFunc_t DX_VOS_DbgPrint_SetPrintFunc(DxPrintfFunc_t value)
{
	DxPrintfFunc_t temp = DX_VOS_DbgPrint_PrintFunc;
	DX_VOS_DbgPrint_PrintFunc = value;
	DX_RETURN(temp);
}

DxUint32 DX_VOS_DbgPrint_SetModulesMask(DxUint32 value)
{
	DxUint32 temp = DX_VOS_DbgPrint_ModulesMask;
	DX_VOS_DbgPrint_ModulesMask = value;
	DX_RETURN(temp);
}

DxUint32 DX_VOS_DbgPrint_EnableModulePrinting(DxUint32 ModuleCode)
{
	return DX_VOS_DbgPrint_SetModulesMask(DX_VOS_DbgPrint_ModulesMask | ModuleCode);
}

DxUint32 DX_VOS_DbgPrint_DisableModulePrinting(DxUint32 ModuleCode)
{
	return DX_VOS_DbgPrint_SetModulesMask(DX_VOS_DbgPrint_ModulesMask & ~ModuleCode);
}

void DX_VOS_CloseLogFile(void)
{
    DxUint32 i = 0;
    DxErrorStack_Terminate();

    g_DxDebugFileName[0] = 0;

    for (i = 0; i < DX_ITEMS_IN_ARRAY(g_DxDebugFile); i++)
    {
        DxVosFile debugFile = (DxVosFile)g_DxDebugFile[i];
        g_DxDebugFile[i] = DX_NULL;
        DX_VOS_FileClose(&debugFile);
    }
}

const DxChar* DX_VOS_OpenLogFile(const DxChar* fileName)
{
    DX_DECLARE(DxStatus, result, DX_SUCCESS);
    DxChar timeStamp[DX_VOS_SIZEOF_TIMESTAMP_STR];

    if (fileName == DX_NULL)
        DX_RETURN(DX_NULL);

    if (g_DxDebugFileName[0] != 0)
        DX_VOS_CloseLogFile();

    result = DX_VOS_GetTimeStamp(timeStamp, sizeof(timeStamp));
    if (result != DX_SUCCESS)
        DX_RETURN(DX_NULL);

    result = DX_VOS_SPrintf(g_DxDebugFileName, sizeof(g_DxDebugFileName), "%s_%s", fileName, timeStamp);
    if (result != DX_SUCCESS)
        DX_RETURN(DX_NULL);

    DX_RETURN(g_DxDebugFileName);
}

static DxVosFile DX_VOS_InnerOpenLogFile()
{
    DX_DECLARE(DxStatus, result, DX_SUCCESS);
    DxChar fileName[DX_VOS_MAX_PATH];
    DxVosFile tmpFile = DX_NULL;

    result = DX_VOS_SPrintf(fileName, sizeof(fileName), "%s_%d.DxLog", g_DxDebugFileName, DX_VOS_GetCurrThreadId());
    if (result != DX_SUCCESS)
        DX_RETURN(DX_NULL);

    result = DX_VOS_FileOpen(&tmpFile, fileName, DX_FILE_CREATE_NEW, DX_SHARE_READ);
    if (result != DX_SUCCESS)
        DX_RETURN(DX_NULL);

    // Printing headers line
    DX_VOS_FPrintf(tmpFile, "Module\tLevel\tTick Count\t");
    if (DX_VOS_DbgPrint_ShouldPrintFileName)
        DX_VOS_FPrintf(tmpFile, "File\t");
    if (DX_VOS_DbgPrint_ShouldPrintLineNum)
        DX_VOS_FPrintf(tmpFile, "Line\t");
    if (DX_VOS_DbgPrint_ShouldPrintFuncName)
        DX_VOS_FPrintf(tmpFile, "Function\t");
    if (DX_VOS_DbgPrint_TimeFormat != DX_NULL)
        DX_VOS_FPrintf(tmpFile, "Date\t");
    if (DX_VOS_DbgPrint_DateFormat != DX_NULL)
        DX_VOS_FPrintf(tmpFile, "Time\t");

    DX_VOS_FPrintf(tmpFile, "Message");
    DX_VOS_ThreadStoragePut(g_DxDebugFile, tmpFile);

    DX_RETURN(tmpFile);
}

DxBool DX_VOS_IsLogFileOpened(void)
{
    return g_DxDebugFileName[0] != 0 ? DX_TRUE : DX_FALSE;
}

#define DATA_TIME_STR_MAX_SIZE 20
void DX_VOS_DebugPrint(DxUint32 ModuleCode, const DxChar *fileName, DxUint32 lineNum, const DxChar *funcName, DxUint32 debugLevel, const DxChar *format, ...)
{
	DxChar timeStr[DATA_TIME_STR_MAX_SIZE];
	DxChar dateStr[DATA_TIME_STR_MAX_SIZE];
	DxTimeStruct_t timeStruct;
	DX_VA_LIST arg_list;

    if (debugLevel > DX_VOS_DbgPrint_DebugLevel ||
		DX_VOS_DbgPrint_PrintFunc == DX_NULL ||
		(ModuleCode & DX_VOS_DbgPrint_ModulesMask) == 0 ||
        DX_VOS_ThreadStorageGet(g_DbgPrintDisabled))
		return;

    LOG_ERROR_STACK();

	timeStr[0] = DX_NULL;
	dateStr[0] = DX_NULL;

	if (DX_VOS_DbgPrint_TimeFormat != DX_NULL || DX_VOS_DbgPrint_DateFormat != DX_NULL)
	{
		if (DX_VOS_GetLocalTime(DX_VOS_GetTime(),&timeStruct) == DX_SUCCESS)
		{
			if (DX_VOS_DbgPrint_TimeFormat != DX_NULL)
				DX_VOS_SPrintf(timeStr, sizeof(timeStr), DX_VOS_DbgPrint_TimeFormat,
					timeStruct.tm_hour, timeStruct.tm_min, timeStruct.tm_sec);

			if (DX_VOS_DbgPrint_DateFormat != DX_NULL)
				DX_VOS_SPrintf(dateStr, sizeof(dateStr), DX_VOS_DbgPrint_DateFormat,
					timeStruct.tm_mday, timeStruct.tm_mon, timeStruct.tm_year);
		}
	}

    DX_VOS_ThreadStoragePut(g_DbgPrintDisabled, (void*) DX_TRUE);
	DX_VA_START(arg_list,format);
	DX_VOS_DbgPrint_PrintFunc(
        ModuleCode, debugLevel,
		DX_VOS_DbgPrint_ShouldPrintFileName	? fileName : DX_NULL,
		DX_VOS_DbgPrint_ShouldPrintLineNum	? lineNum	: 0,
		DX_VOS_DbgPrint_ShouldPrintFuncName	? funcName	: DX_NULL,
		dateStr, timeStr, format, arg_list
		);
	DX_VA_END(arg_list);
    DX_VOS_ThreadStoragePut(g_DbgPrintDisabled, (void*) DX_FALSE);
}

void DX_VOS_DebugStdoutPrint(DxUint32 moduleCode, DxUint32 debugLevel, const DxChar *fileName, DxUint32 lineNum, const DxChar *funcName, const DxChar* aDate, const DxChar* aTime, const DxChar *format, DX_VA_LIST arg_list)
{
    moduleCode = moduleCode;
    debugLevel = debugLevel;


	DX_VOS_Printf("\r\n");
	if (fileName != DX_NULL && *fileName != 0)
		DX_VOS_Printf("File: %s, ", fileName);

	if (lineNum != 0)
		DX_VOS_Printf("Line: %d, ", lineNum);

	if (funcName != DX_NULL && *funcName != 0)
		DX_VOS_Printf("Function: %s, ", funcName);

	if (aDate != DX_NULL && *aDate != 0)
		DX_VOS_Printf("%s ", aDate);

	if (aTime != DX_NULL && *aTime != 0)
		DX_VOS_Printf("%s ", aTime);

	DX_VOS_VPrintf(format, arg_list);
}

void DX_VOS_DbgFlush(void)
{
    DxVosFile debugFile = (DxVosFile)DX_VOS_ThreadStorageGet(g_DxDebugFile);
    if (debugFile != DX_NULL)
    {
        DX_VOS_FFlush(debugFile);
        DX_VOS_ThreadStoragePut(g_LastDbgFlash, (void*) DX_VOS_GetTickCount());
    }
}

#define DX_FLASH_INTERVAL  1000
void DX_VOS_DebugFilePrint(DxUint32 moduleCode, DxUint32 debugLevel, const DxChar *fileName, DxUint32 lineNum, const DxChar *funcName, const DxChar* aDate, const DxChar* aTime, const DxChar *format, DX_VA_LIST arg_list)
{
    DxVosFile debugFile = (DxVosFile)DX_VOS_ThreadStorageGet(g_DxDebugFile);
    DxUint32 lastDebugFlush = (DxUint32)DX_VOS_ThreadStorageGet(g_LastDbgFlash);
    DxUint32 currTick = DX_VOS_GetTickCount();

    if (g_DxDebugFileName[0] == 0)
		return;

    if (debugFile == DX_NULL)
    {
        debugFile = DX_VOS_InnerOpenLogFile();
        if (debugFile == DX_NULL)
            return;
    }

	DX_VOS_FPrintf(debugFile, "\r\n");

    DX_VOS_FPrintf(debugFile, "%08X\t%d\t%d\t", moduleCode, debugLevel, currTick);

	if (fileName != DX_NULL && *fileName != 0)
		DX_VOS_FPrintf(debugFile, "%s", fileName);
	if (DX_VOS_DbgPrint_ShouldPrintFileName)
		DX_VOS_FPrintf(debugFile, "\t");

	if (lineNum != 0)
		DX_VOS_FPrintf(debugFile, "%d", lineNum);
	if (DX_VOS_DbgPrint_ShouldPrintLineNum)
		DX_VOS_FPrintf(debugFile, "\t");

	if (funcName != DX_NULL && *funcName != 0)
		DX_VOS_FPrintf(debugFile, "%s", funcName);
	if (DX_VOS_DbgPrint_ShouldPrintFuncName)
		DX_VOS_FPrintf(debugFile, "\t");

	if (aDate != DX_NULL && *aDate != 0)
		DX_VOS_FPrintf(debugFile, "%s", aDate);
	if (DX_VOS_DbgPrint_TimeFormat != DX_NULL)
		DX_VOS_FPrintf(debugFile, "\t");

	if (aTime != DX_NULL && *aTime != 0)
		DX_VOS_FPrintf(debugFile, "%s", aTime);
	if (DX_VOS_DbgPrint_DateFormat != DX_NULL)
		DX_VOS_FPrintf(debugFile, "\t");

	DX_VOS_VFPrintf(debugFile, format, arg_list);

    if (currTick - lastDebugFlush > DX_FLASH_INTERVAL)
        DX_VOS_DbgFlush();
}


#define MAX_NUM_OF_PRINT_FUNCS	5
#ifndef DX_VOS_NO_SOCKET_LOG
static DxPrintfFunc_t DxPrintFuncsArray[MAX_NUM_OF_PRINT_FUNCS] = {
	DX_VOS_DebugStdoutPrint, DX_VOS_DebugFilePrint, DX_VOS_DebugSocketPrint, DX_NULL, DX_NULL
};
#else
static DxPrintfFunc_t DxPrintFuncsArray[MAX_NUM_OF_PRINT_FUNCS] = {
	DX_VOS_DebugStdoutPrint, DX_VOS_DebugFilePrint, DX_NULL, DX_NULL, DX_NULL
};
#endif
void DX_VOS_DebugMultiPrint(DxUint32 moduleCode, DxUint32 debugLevel, const DxChar *fileName, DxUint32 lineNum, const DxChar *funcName,
							const DxChar* aDate, const DxChar* aTime, const DxChar *format, DX_VA_LIST arg_list)
{
	DxUint i = 0;
	while (i < MAX_NUM_OF_PRINT_FUNCS && DxPrintFuncsArray[i] != DX_NULL)
		DxPrintFuncsArray[i++](moduleCode, debugLevel, fileName, lineNum, funcName, aDate, aTime, format, arg_list);
}

void DX_VOS_ClearDebugPrintFuncArray(void)
{
	DX_VOS_MemSetZero(DxPrintFuncsArray,sizeof(DxPrintFuncsArray));
}

DxStatus DX_VOS_RegisterDebugPrintFunc(DxPrintfFunc_t func)
{
	DxUint i = 0;
	while (i < MAX_NUM_OF_PRINT_FUNCS && DxPrintFuncsArray[i] != DX_NULL)
		i++;
	if (i < MAX_NUM_OF_PRINT_FUNCS)
	{
		DxPrintFuncsArray[i] = func;
		DX_RETURN(DX_SUCCESS);
	}
	RETURN_NEW_ERROR(DX_FAILURE);
}


#ifndef DX_VOS_NO_SOCKET_LOG

// Called on socket open and every once in a while with the filename and header info.
DxStatus DX_VOS_DebugSocketLogInfo()
{
	DX_DECLARE(DxStatus, result, DX_SUCCESS);
	DxUint32 socketBuffSize = 0;
	if (!g_DxSocketDebugActive)
		DX_RETURN(DX_SUCCESS);

	g_DxSocketBuffer[0] = DX_NULL;

	result = DX_VOS_StrNCat(g_DxSocketBuffer + socketBuffSize, g_DxSocketBufferSize - socketBuffSize, "\r\n<logInfo><fileName>");
	if (result != DX_SUCCESS)
		GOTO_END_WITH_VAR_STATUS(result);
	socketBuffSize = DX_VOS_StrLen(g_DxSocketBuffer);

	result = DX_VOS_StrNCat(g_DxSocketBuffer + socketBuffSize, g_DxSocketBufferSize - socketBuffSize, g_DxSocketFileName);
	if (result != DX_SUCCESS)
		GOTO_END_WITH_VAR_STATUS(result);
	socketBuffSize = DX_VOS_StrLen(g_DxSocketBuffer);

	result = DX_VOS_StrNCat(g_DxSocketBuffer + socketBuffSize, g_DxSocketBufferSize - socketBuffSize, "</fileName><headers>");
	if (result != DX_SUCCESS)
		GOTO_END_WITH_VAR_STATUS(result);
	socketBuffSize = DX_VOS_StrLen(g_DxSocketBuffer);

	// Printing headers line
	result = DX_VOS_StrNCat(g_DxSocketBuffer + socketBuffSize, g_DxSocketBufferSize - socketBuffSize, "Thread ID\tModule\tLevel\tTick Count\t");
	if (result != DX_SUCCESS)
		GOTO_END_WITH_VAR_STATUS(result);
	socketBuffSize = DX_VOS_StrLen(g_DxSocketBuffer);

	if (DX_VOS_DbgPrint_ShouldPrintFileName)
	{
		result = DX_VOS_StrNCat(g_DxSocketBuffer + socketBuffSize, g_DxSocketBufferSize - socketBuffSize, "File\t");
		if (result != DX_SUCCESS)
			GOTO_END_WITH_VAR_STATUS(result);
		socketBuffSize = DX_VOS_StrLen(g_DxSocketBuffer);
	}
	if (DX_VOS_DbgPrint_ShouldPrintLineNum)
	{
		result = DX_VOS_StrNCat(g_DxSocketBuffer + socketBuffSize, g_DxSocketBufferSize - socketBuffSize, "Line\t");
		if (result != DX_SUCCESS)
			GOTO_END_WITH_VAR_STATUS(result);
		socketBuffSize = DX_VOS_StrLen(g_DxSocketBuffer);
	}
	if (DX_VOS_DbgPrint_ShouldPrintFuncName)
	{
		result = DX_VOS_StrNCat(g_DxSocketBuffer + socketBuffSize, g_DxSocketBufferSize - socketBuffSize, "Function\t");
		if (result != DX_SUCCESS)
			GOTO_END_WITH_VAR_STATUS(result);
		socketBuffSize = DX_VOS_StrLen(g_DxSocketBuffer);
	}
	if (DX_VOS_DbgPrint_TimeFormat != DX_NULL)
	{
		result = DX_VOS_StrNCat(g_DxSocketBuffer + socketBuffSize, g_DxSocketBufferSize - socketBuffSize, "Date\t");
		if (result != DX_SUCCESS)
			GOTO_END_WITH_VAR_STATUS(result);
		socketBuffSize = DX_VOS_StrLen(g_DxSocketBuffer);
	}
	if (DX_VOS_DbgPrint_DateFormat != DX_NULL)
	{
		result = DX_VOS_StrNCat(g_DxSocketBuffer + socketBuffSize, g_DxSocketBufferSize - socketBuffSize, "Time\t");
		if (result != DX_SUCCESS)
			GOTO_END_WITH_VAR_STATUS(result);
		socketBuffSize = DX_VOS_StrLen(g_DxSocketBuffer);
	}

	result = DX_VOS_StrNCat(g_DxSocketBuffer + socketBuffSize, g_DxSocketBufferSize - socketBuffSize, "Message</headers></logInfo>");
	if (result != DX_SUCCESS)
		GOTO_END_WITH_VAR_STATUS(result);
	socketBuffSize = DX_VOS_StrLen(g_DxSocketBuffer);

	result = DX_VOS_SocketWrite(g_DxDebugSocket, g_DxSocketBuffer, socketBuffSize, DX_NULL, 1000);
	if (result != DX_SUCCESS)
		GOTO_END_WITH_VAR_STATUS(result);

end:
	if (result != DX_SUCCESS)
		DX_VOS_CloseLogSocket();
	DX_RETURN(result);
};

void DX_VOS_OpenLogSocket(const DxChar* fileName, DxUint8 b1, DxUint8 b2, DxUint8 b3, DxUint8 b4, DxUint16 port)
{
	DX_DECLARE(DxStatus, result, DX_SUCCESS);
	DxIpAddress ipAddr = {0};
	DxChar timeStamp[DX_VOS_SIZEOF_TIMESTAMP_STR];

	//DX_GOTO_ASSERT(aBindAddr != DX_NULL);

	DX_VOS_SetIPAddress(&ipAddr, b1, b2, b3, b4, port);

	if (g_DxDebugSocket != DX_NULL)
	{
		result = DX_VOS_SocketClose(&g_DxDebugSocket);
		if (result != DX_SUCCESS)
			GOTO_END_WITH_VAR_STATUS(result);
	}

	result = DX_VOS_SocketCreate(&g_DxDebugSocket, DX_AF_INET, DX_SOCK_DATAGRAM, DX_IPPROTO_UDP);
	if (result != DX_SUCCESS)
		GOTO_END_WITH_OLD_ERROR(result);

	result = DX_VOS_SocketConnect(g_DxDebugSocket, &ipAddr);
	if (result != DX_SUCCESS)
		GOTO_END_WITH_OLD_ERROR(result);

	// We allocate the buffer to that we can use it as a "workspace" for constructing the log message.
	// We want to create the whole message before writing to the socket.
	g_DxSocketBuffer = (DxChar*)DX_VOS_MemMalloc(g_DxSocketBufferSize);
	GOTO_END_IF_ALLOC_FAILED(g_DxSocketBuffer);

	//This is a suggested filename for the log (should be handled on the receiver side).
	g_DxSocketFileName = (DxChar*)DX_VOS_MemMalloc(DX_VOS_MAX_PATH);
	GOTO_END_IF_ALLOC_FAILED(g_DxSocketFileName);

	result = DX_VOS_GetTimeStamp(timeStamp, sizeof(timeStamp));
	if (result != DX_SUCCESS)
		GOTO_END_WITH_VAR_STATUS(result);

	result = DX_VOS_SPrintf(g_DxSocketFileName, DX_VOS_MAX_PATH, "%s_%s.DxLog", fileName, timeStamp);
	if (result != DX_SUCCESS)
		GOTO_END_WITH_VAR_STATUS(result);

	g_DxSocketDebugActive = DX_TRUE;

	result = DX_VOS_DebugSocketLogInfo();
	if (result != DX_SUCCESS)
		GOTO_END_WITH_VAR_STATUS(result);
end:
	if (result != DX_SUCCESS)
		DX_VOS_CloseLogSocket();
}

void DX_VOS_DebugSocketPrint(DxUint32 moduleCode, DxUint32 debugLevel,
							 const DxChar *fileName, DxUint32 lineNum, const DxChar *funcName,
							 const DxChar* aDate, const DxChar* aTime, const DxChar *format, DX_VA_LIST arg_list)
{
	DxUint32 currTick = DX_VOS_GetTickCount();
	DxUint32 socketBuffSize = 0;

	// Every once in a while we want to resend the headers
	if (currTick - g_DxSocketInfoSentTime > g_DxSocketInfoSentDelta)
		DX_VOS_DebugSocketLogInfo();

	if (!g_DxSocketDebugActive)
		return;

	g_DxSocketBuffer[0] = DX_NULL;

	if (DX_SUCCESS != DX_VOS_StrNCat(g_DxSocketBuffer + socketBuffSize, g_DxSocketBufferSize - socketBuffSize, "\r\n"))
		goto end;
	socketBuffSize += 2;

	if (DX_SUCCESS != DX_VOS_SPrintf(g_DxSocketBuffer + socketBuffSize, g_DxSocketBufferSize - socketBuffSize, "%d\t%08X\t%d\t%d\t", DX_VOS_GetCurrThreadId(), moduleCode, debugLevel, currTick))
		goto end;
	socketBuffSize = DX_VOS_StrLen(g_DxSocketBuffer);

	if (fileName != DX_NULL && *fileName != 0)
	{
		if (DX_SUCCESS != DX_VOS_SPrintf(g_DxSocketBuffer + socketBuffSize, g_DxSocketBufferSize - socketBuffSize, "%s", fileName))
			goto end;
		socketBuffSize = DX_VOS_StrLen(g_DxSocketBuffer);
	}
	if (DX_VOS_DbgPrint_ShouldPrintFileName)
	{
		if (DX_SUCCESS != DX_VOS_StrNCat(g_DxSocketBuffer + socketBuffSize, g_DxSocketBufferSize - socketBuffSize, "\t"))
			goto end;
		socketBuffSize += 1;
	}

	if (lineNum != 0)
	{
		if (DX_SUCCESS != DX_VOS_SPrintf(g_DxSocketBuffer + socketBuffSize, g_DxSocketBufferSize - socketBuffSize, "%d", lineNum))
			goto end;
		socketBuffSize = DX_VOS_StrLen(g_DxSocketBuffer);
	}
	if (DX_VOS_DbgPrint_ShouldPrintLineNum)
	{
		if (DX_SUCCESS != DX_VOS_StrNCat(g_DxSocketBuffer + socketBuffSize, g_DxSocketBufferSize - socketBuffSize, "\t"))
			goto end;
		socketBuffSize += 1;
	}

	if (funcName != DX_NULL && *funcName != 0)
	{
		if (DX_SUCCESS != DX_VOS_SPrintf(g_DxSocketBuffer + socketBuffSize, g_DxSocketBufferSize - socketBuffSize, "%s", funcName))
			goto end;
		socketBuffSize = DX_VOS_StrLen(g_DxSocketBuffer);
	}
	if (DX_VOS_DbgPrint_ShouldPrintFuncName)
	{
		if (DX_SUCCESS != DX_VOS_StrNCat(g_DxSocketBuffer + socketBuffSize, g_DxSocketBufferSize - socketBuffSize, "\t"))
			goto end;
		socketBuffSize += 1;
	}

	if (aDate != DX_NULL && *aDate != 0)
	{
		if (DX_SUCCESS != DX_VOS_SPrintf(g_DxSocketBuffer + socketBuffSize, g_DxSocketBufferSize - socketBuffSize, "%s", aDate))
			goto end;
		socketBuffSize = DX_VOS_StrLen(g_DxSocketBuffer);
	}
	if (DX_VOS_DbgPrint_TimeFormat != DX_NULL)
	{
		if (DX_SUCCESS != DX_VOS_StrNCat(g_DxSocketBuffer + socketBuffSize, g_DxSocketBufferSize - socketBuffSize, "\t"))
			goto end;
		socketBuffSize += 1;
	}

	if (aTime != DX_NULL && *aTime != 0)
	{
		if (DX_SUCCESS != DX_VOS_SPrintf(g_DxSocketBuffer + socketBuffSize, g_DxSocketBufferSize - socketBuffSize, "%s", aTime))
			goto end;
		socketBuffSize = DX_VOS_StrLen(g_DxSocketBuffer);
	}
	if (DX_VOS_DbgPrint_DateFormat != DX_NULL)
	{
		if (DX_SUCCESS != DX_VOS_StrNCat(g_DxSocketBuffer + socketBuffSize, g_DxSocketBufferSize - socketBuffSize, "\t"))
			goto end;
		socketBuffSize += 1;
	}

	if (DX_SUCCESS != DX_VOS_VSPrintf(g_DxSocketBuffer + socketBuffSize, g_DxSocketBufferSize - socketBuffSize, format, arg_list))
		goto end;
	socketBuffSize = DX_VOS_StrLen(g_DxSocketBuffer);
end:
	DX_VOS_SocketWrite(g_DxDebugSocket, g_DxSocketBuffer, DX_VOS_StrLen(g_DxSocketBuffer), DX_NULL, 1000);
}

void DX_VOS_CloseLogSocket()
{
	DX_VOS_MemFree(g_DxSocketBuffer);		g_DxSocketBuffer = DX_NULL;
	DX_VOS_MemFree(g_DxSocketFileName);	g_DxSocketFileName = DX_NULL;

	DX_VOS_SocketClose(&g_DxDebugSocket);
	g_DxSocketDebugActive = DX_FALSE;
}

#endif

#endif
