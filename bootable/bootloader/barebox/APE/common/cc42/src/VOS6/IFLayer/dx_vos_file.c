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

 #define DX_DBG_MODULE DX_DBG_GENERAL_VOS

#include "DX_VOS_BaseFile.h"
#include "VOS_API/DX_VOS_Mem.h"
#include "VOS_API/DX_VOS_String.h"
#include "VOS_API/DX_VOS_Utils.h"
#include "VOS_API/DX_VOS_Stdio.h"
#include "VOS_API/DX_VOS_Time.h"
#include <stdio.h>


DxStatus DX_VOS_CreateFileHandleFromOsHandle(DxVosFile* aFileHandle, void* osHandle)
{
    DX_DECLARE(DxStatus, result, DX_SUCCESS);

    DX_ASSERT_PARAM(aFileHandle != DX_NULL);
    DX_ASSERT_PARAM(osHandle != DX_NULL);

    result = DX_VOS_BaseCreateFileHandleFromOsHandle(aFileHandle, osHandle);
    if (result != DX_SUCCESS)
	RETURN_OLD_ERROR(result);

    DX_RETURN(DX_SUCCESS);
}

DxStatus DX_VOS_GetOsHandleFromFileHandle(DxVosFile aFileHandle, void** osHandle)
{
    DX_DECLARE(DxStatus, result, DX_SUCCESS);

    DX_ASSERT_PARAM(aFileHandle != DX_NULL);
    DX_ASSERT_PARAM(osHandle != DX_NULL);

    result = DX_VOS_BaseGetOsHandleFromFileHandle(aFileHandle, osHandle);
    if (result != DX_SUCCESS)
        RETURN_OLD_ERROR(result);

    DX_RETURN(DX_SUCCESS);
}

DxStatus DX_VOS_FileOpenWithTimeStamp(DxVosFile* aFileHandle, const DxChar *aFileName, DxUint32 openMode, DxUint32 shareMode)
{
    DX_DECLARE(DxStatus, result, DX_SUCCESS);
    DxChar timeStamp[DX_VOS_SIZEOF_TIMESTAMP_STR];
    DxChar fileName[DX_VOS_MAX_PATH];

    result = DX_VOS_GetTimeStamp(timeStamp, sizeof(timeStamp));
    if (result != DX_SUCCESS)
        RETURN_OLD_ERROR(result);

    result = DX_VOS_SPrintf(fileName, sizeof(fileName), aFileName, timeStamp);
    if (result != DX_SUCCESS)
        RETURN_NEW_ERROR(DX_BAD_ARGUMENTS);

    result = DX_VOS_FileOpen(aFileHandle, fileName, openMode, shareMode);
    if (result != DX_SUCCESS)
	RETURN_OLD_ERROR(result);

    DX_RETURN(DX_SUCCESS);
}

DxStatus DX_VOS_FOpen(DxVosFile* aFileHandle, const DxChar *aFileName, const DxChar *aMode)
{
	DX_DECLARE(DxStatus, result, DX_SUCCESS);
	DxUint terminatingChar = 0;
    DxChar fileName[DX_VOS_MAX_PATH];

    DX_ASSERT_PARAM(aFileName != DX_NULL);
    DX_ASSERT_PARAM(aMode != DX_NULL);
    DX_ASSERT_PARAM(aFileHandle != DX_NULL);

    DX_VERIFY_PARAM(*aFileName != 0);

	*aFileHandle = DX_NULL;

    result = DX_VOS_StrNCopy(fileName, sizeof(fileName), aFileName);
    if (result != DX_SUCCESS)
	RETURN_OLD_ERROR(result);

    switch( aMode[0] )
	{
	case 'r':
	case 'w':
		terminatingChar = 1;
		break;
	default:
		/* return an error */
		RETURN_NEW_ERROR(DX_BAD_ARGUMENTS);
	}
	switch( aMode[1] )
	{
	case 0:   break;
	case '+':
		terminatingChar = 2;
		if (aMode[2] == 'b' || aMode[2] == 't')
			terminatingChar = 3;
		break;
	case 'b':
	case 't': terminatingChar = 2;
		break;
	default:
		/* return an error */
		RETURN_NEW_ERROR(DX_BAD_ARGUMENTS);
	}

	if (aMode[terminatingChar] != 0)
		RETURN_NEW_ERROR(DX_BAD_ARGUMENTS);


	if (aMode[0] == 'w')
	{
		const DxChar *Pos = DX_VOS_StrRChr(fileName, *DX_VOS_PATH_DIVIDER);
		if (Pos != DX_NULL)
		{
            DxChar  oldChar = 0;
			DxUint DirLength = (DxUint)(Pos - fileName + 1);

            oldChar = fileName[DirLength];
			fileName[DirLength] = 0;

			result = DX_VOS_CreateDirectory(fileName);
			if (result != DX_SUCCESS)
				RETURN_OLD_ERROR(result);

            fileName[DirLength] = oldChar;
		}
	}
    result = DX_VOS_BaseFOpen(aFileHandle, fileName, aMode);
    if (result != DX_SUCCESS)
    {
        DX_VOS_BaseLogFileResult();
        DX_DBG_PRINT2(DX_DBGPRINT_ERROR_LEVEL, "Failed to open: %s (mode: %s)", fileName, aMode);
	RETURN_OLD_ERROR(result);
    }

    DX_DBG_LOG_OBJECT_CREATION(DxVosFile, *aFileHandle);
    DX_RETURN(DX_SUCCESS);
}

DxStatus DX_VOS_FileOpen(DxVosFile* aFileHandle, const DxChar *aFileName, DxUint32 openMode, DxUint32 shareMode)
{
    DX_DECLARE(DxStatus, result, DX_SUCCESS);
    DxChar fileName[DX_VOS_MAX_PATH];

    DX_ASSERT_PARAM(aFileHandle != DX_NULL);
    DX_ASSERT_PARAM(aFileName != DX_NULL);
    // You must use only valid flags
    DX_ASSERT_PARAM((openMode & 0xF0) == 0);
    // You must select DX_FILE_READ or DX_FILE_WRITE
    DX_ASSERT_PARAM((openMode & (DX_FILE_READ | DX_FILE_WRITE)) != 0);
    DX_ASSERT_PARAM(shareMode < DX_NUMBER_OF_FILE_SHARE_MODES );
    DX_VERIFY_PARAM(*aFileName != 0);

    *aFileHandle = DX_NULL;

    result = DX_VOS_StrNCopy(fileName, sizeof(fileName), aFileName);
    if (result != DX_SUCCESS)
        RETURN_OLD_ERROR(result);

    if (openMode & DX_FILE_CREATE)
    {
        const DxChar *Pos = DX_VOS_StrRChr(fileName, '/');
		if (Pos == DX_NULL)
			Pos = DX_VOS_StrRChr(fileName, '\\');
        if (Pos != DX_NULL)
        {
            DxChar  oldChar = 0;
            DxUint DirLength = (DxUint)(Pos - fileName + 1);

            oldChar = fileName[DirLength];
            fileName[DirLength] = 0;

            result = DX_VOS_CreateDirectory(fileName);
            if (result != DX_SUCCESS)
                RETURN_OLD_ERROR(result);

            fileName[DirLength] = oldChar;
        }
    }
    result = DX_VOS_BaseFileOpen(aFileHandle, fileName, (EDxOpenMode)openMode, (EDxShareMode)shareMode);
    if (result != DX_SUCCESS)
    {
        DX_VOS_BaseLogFileResult();
        DX_DBG_PRINT3(DX_DBGPRINT_ERROR_LEVEL, "Failed to open: %s (openMode: %d, shareMode: %d)", fileName, openMode, shareMode);
        RETURN_OLD_ERROR(result);
    }

    DX_DBG_LOG_OBJECT_CREATION(DxVosFile, *aFileHandle);
    DX_RETURN(DX_SUCCESS);
}

DxStatus DX_VOS_FClose (DxVosFile aFileHandle)
{
    DX_DECLARE(DxStatus, result, DX_SUCCESS);

    if (aFileHandle == DX_NULL)
		DX_RETURN(DX_SUCCESS);

    DX_DBG_LOG_OBJECT_DESTRUCTION(DxVosFile, aFileHandle);

    result = DX_VOS_BaseFileClose(aFileHandle);
    if (result != DX_SUCCESS)
    {
        DX_VOS_BaseLogFileResult();
	RETURN_OLD_ERROR(result);
    }

	DX_RETURN(DX_SUCCESS);
}

DxBool DX_VOS_IsFileExists(const DxChar* fileName)
{
    if (fileName == DX_NULL || *fileName == DX_NULL)
        return DX_FALSE;
    return DX_VOS_BaseIsFileExists(fileName);
}

DxStatus DX_VOS_FileClose (DxVosFile* aFileHandle)
{
    DX_DECLARE(DxStatus, result, DX_SUCCESS);
    if (aFileHandle == DX_NULL)
        DX_RETURN(DX_SUCCESS);
    if (*aFileHandle == DX_NULL)
        DX_RETURN(DX_SUCCESS);

    result = DX_VOS_BaseFileClose(*aFileHandle);
    if (result != DX_SUCCESS)
    {
        DX_VOS_BaseLogFileResult();
        RETURN_OLD_ERROR(result);
    }

	DX_DBG_LOG_OBJECT_DESTRUCTION(DxVosFile, *aFileHandle);
    *aFileHandle = DX_NULL;

    DX_RETURN(DX_SUCCESS);
}

DxStatus DX_VOS_FReadEx(DxVosFile aFileHandle, void* aBuf, DxUint32 aSize,  DxUint32* aReadBytesPtr)
{
	DX_DECLARE(DxStatus, result, DX_SUCCESS);
	DxUint32 numRead = 0;

	if (aReadBytesPtr == DX_NULL)
		aReadBytesPtr = &numRead;

	*aReadBytesPtr = 0;

	DX_ASSERT_PARAM(aFileHandle != DX_NULL);

	if (aSize == 0)
		DX_RETURN(DX_SUCCESS);

	DX_ASSERT_PARAM(aBuf != DX_NULL);

	result = DX_VOS_BaseFileRead(aFileHandle, aBuf, aSize, aReadBytesPtr);

	if (result != DX_SUCCESS)
    {
        DX_VOS_BaseLogFileResult();
		RETURN_OLD_ERROR(result);
    }

	if (aReadBytesPtr == &numRead && *aReadBytesPtr != aSize)
		RETURN_NEW_ERROR(DX_VOS_FILE_ERROR);

    if (result != DX_SUCCESS)
	RETURN_OLD_ERROR(result);

    DX_RETURN(DX_SUCCESS);
}


DxStatus DX_VOS_FWrite (DxVosFile aFileHandle, const void *aBuf, DxUint32 aSize)
{
    DX_DECLARE(DxStatus, result, DX_SUCCESS);
	DX_ASSERT_PARAM(aFileHandle != DX_NULL);

	if (aSize == 0)
		DX_RETURN(DX_SUCCESS);

    DX_ASSERT_PARAM(aBuf != DX_NULL);

    result = DX_VOS_BaseFileWrite(aFileHandle, aBuf, aSize);

    if (result != DX_SUCCESS)
    {
        DX_VOS_BaseLogFileResult();
	RETURN_OLD_ERROR(result);
    }
    DX_RETURN(DX_SUCCESS);
}

DxStatus DX_VOS_FSeekEx(DxVosFile aFileHandle, DxInt32 aOffset,DxInt aOrigin)
{
    DX_DECLARE(DxStatus, result, DX_SUCCESS);
    DX_ASSERT_PARAM(aFileHandle != DX_NULL);

	DX_ASSERT_PARAM(aOrigin == SEEK_SET || aOrigin == SEEK_END || aOrigin == SEEK_CUR);

	result = DX_VOS_BaseFileSeek(aFileHandle, aOffset, aOrigin);

    if (result != DX_SUCCESS)
    {
        DX_VOS_BaseLogFileResult();
        RETURN_OLD_ERROR(result);
    }
    DX_RETURN(DX_SUCCESS);

}

DxStatus DX_VOS_FTell(DxVosFile aFileHandle, DxUint32* aCurPosPtr)
{
    DX_DECLARE(DxStatus, result, DX_SUCCESS);
    DX_ASSERT_PARAM(aFileHandle != DX_NULL);
    DX_ASSERT_PARAM(aCurPosPtr != DX_NULL);

	*aCurPosPtr = 0;

	result = DX_VOS_BaseFileTell(aFileHandle,aCurPosPtr);

    if (result != DX_SUCCESS)
    {
        DX_VOS_BaseLogFileResult();
        RETURN_OLD_ERROR(result);
    }
    DX_RETURN(DX_SUCCESS);

}
/*
DxStatus DX_VOS_FTruncate(DxVosFile aFileHandle)
{
    DX_DECLARE(DxStatus, result, DX_SUCCESS);
    DX_ASSERT_PARAM(aFileHandle != DX_NULL);

    result = DX_VOS_BaseFileTruncate(aFileHandle);

    if (result != DX_SUCCESS)
    {
        DX_VOS_BaseLogFileResult();
        RETURN_OLD_ERROR(result);
    }
    DX_RETURN(DX_SUCCESS);
}
*/
DxStatus DX_VOS_FGetFileSize(DxVosFile aFileHandle, DxUint32* aFileSizePtr)
{
	DX_DECLARE(DxStatus, result, DX_SUCCESS);
	DxUint32 currPos = 0, fileSize = 0;

    DX_ASSERT_PARAM(aFileHandle != DX_NULL);
    DX_ASSERT_PARAM(aFileSizePtr != DX_NULL);

	*aFileSizePtr = 0;

	result = DX_VOS_FTell(aFileHandle, &currPos);
	if (result != DX_SUCCESS)
    {
        DX_VOS_BaseLogFileResult();
		RETURN_OLD_ERROR(result);
    }

	result = DX_VOS_FSeekEx(aFileHandle, 0, SEEK_END);
	if (result != DX_SUCCESS)
    {
        DX_VOS_BaseLogFileResult();
		RETURN_OLD_ERROR(result);
    }

	result = DX_VOS_FTell(aFileHandle, &fileSize);
	if (result != DX_SUCCESS)
    {
        DX_VOS_BaseLogFileResult();
		RETURN_OLD_ERROR(result);
    }

	result = DX_VOS_FSeekEx(aFileHandle, currPos, SEEK_SET);
	if (result != DX_SUCCESS)
    {
        DX_VOS_BaseLogFileResult();
		RETURN_OLD_ERROR(result);
    }

	*aFileSizePtr = fileSize;

	DX_RETURN(DX_SUCCESS);
}

DxStatus DX_VOS_FEof(DxVosFile aFileHandle)
{
    DX_DECLARE(DxStatus, result, DX_SUCCESS);
    DX_ASSERT_PARAM(aFileHandle != DX_NULL);

	result = DX_VOS_BaseFileEof(aFileHandle);
    if (result != DX_SUCCESS && result != DX_VOS_END_OF_FILE)
    {
        DX_VOS_BaseLogFileResult();
        RETURN_OLD_ERROR(result);
    }

    DX_RETURN(result);
}

DxStatus DX_VOS_FGets(DxVosFile aFileHandle, DxChar *aString, DxUint aLength)
{
	DX_DECLARE(DxStatus, result, DX_SUCCESS);

    DX_ASSERT_PARAM(aFileHandle != DX_NULL);
    DX_ASSERT_PARAM(aString != DX_NULL);
    DX_ASSERT_PARAM(aLength > 0);

	aLength--; // Saving room for terminating NULL;
	*aString = 0;

	while (aLength > 0)
	{
		DxUint32 wasRead = 0;
		result = DX_VOS_FReadEx(aFileHandle, aString, 1, &wasRead);
		if (result != DX_SUCCESS)
		{
			*aString = 0;
            DX_VOS_BaseLogFileResult();
			RETURN_OLD_ERROR(result);
		}
		if (wasRead == 0)
			DX_RETURN(DX_SUCCESS);
		if (*aString == '\n')
		{
			*(++aString) = 0;
			DX_RETURN(DX_SUCCESS);
		}
		if (*aString != '\r')
		{
			aString++;
			aLength--;
		}
		*aString = 0;
	}
	RETURN_NEW_ERROR(DX_BUFFER_IS_NOT_BIG_ENOUGH);
}

DxStatus DX_VOS_FFlush  (DxVosFile aFileHandle)
{
    DX_DECLARE(DxStatus, result, DX_SUCCESS);
    DX_ASSERT_PARAM(aFileHandle != DX_NULL);

	result = DX_VOS_BaseFileFlush(aFileHandle);
    if (result != DX_SUCCESS)
    {
        DX_VOS_BaseLogFileResult();
        RETURN_OLD_ERROR(result);
    }
    DX_RETURN(DX_SUCCESS);
}

DxStatus DX_VOS_FDelete(const DxChar *aFileName)
{
    DX_DECLARE(DxStatus, result, DX_SUCCESS);
    DX_ASSERT_PARAM(aFileName != DX_NULL);

    DX_VERIFY_PARAM(*aFileName != 0);

    if (!DX_VOS_BaseIsFileExists(aFileName))
        DX_RETURN(DX_SUCCESS);

	result = DX_VOS_BaseFileDelete(aFileName);
    if (result != DX_SUCCESS)
    {
        DX_VOS_BaseLogFileResult();
        RETURN_OLD_ERROR(result);
    }
    DX_RETURN(DX_SUCCESS);
}

DxStatus DX_VOS_FRename(const DxChar *aOrigName, const DxChar *aNewName)
{
    DX_DECLARE(DxStatus, result, DX_SUCCESS);
    DX_ASSERT_PARAM(aOrigName != DX_NULL);
    DX_ASSERT_PARAM(aNewName != DX_NULL);

    DX_VERIFY_PARAM(*aOrigName != 0);
    DX_VERIFY_PARAM(*aNewName != 0);

    result = DX_VOS_BaseFileRename(aOrigName, aNewName);
    if (result != DX_SUCCESS)
    {
        DX_VOS_BaseLogFileResult();
        RETURN_OLD_ERROR(result);
    }
    DX_RETURN(DX_SUCCESS);
}

DxStatus DX_VOS_CreateDirectory(const DxChar *aDirName)
{
	DX_DECLARE(DxStatus, result, DX_SUCCESS);
	DxChar DirName[DX_VOS_MAX_PATH];
	DxChar *Pos = DX_NULL;
	DxUint Len = 0;

    DX_ASSERT_PARAM(aDirName != DX_NULL);

    DX_VERIFY_PARAM(*aDirName != 0);

	result = DX_VOS_BaseCreateDirectory(aDirName);

	if (result == DX_SUCCESS)
		DX_RETURN(DX_SUCCESS);
	/* if creation failed we assume that parent directory doesn't exist
	so we try to create all the directory chain starting from the root */
    Len = DX_VOS_StrNLen(aDirName, DX_VOS_MAX_PATH);

	DX_VOS_MemCpy(DirName, sizeof(DirName), aDirName, Len + 1);

	Pos = DirName;
	while ((Pos = DX_VOS_StrChr(Pos,*DX_VOS_PATH_DIVIDER)) != DX_NULL)
	{
		DxChar Temp = *(++Pos);
		*Pos = 0;
		result = DX_VOS_BaseCreateDirectory(DirName);
		if (result != DX_SUCCESS)
        {
            DX_DBG_PRINT1(DX_DBGPRINT_ERROR_LEVEL, "Failed to create directory: %s", DirName);
            DX_VOS_BaseLogFileResult();
			RETURN_OLD_ERROR(result);
        }
		*Pos = Temp;
	}

	result = DX_VOS_BaseCreateDirectory(aDirName);
    if (result != DX_SUCCESS)
    {
        DX_VOS_BaseLogFileResult();
        RETURN_OLD_ERROR(result);
    }
    DX_RETURN(DX_SUCCESS);

}
#ifndef NO_OS
DxStatus DX_VOS_EnumerateDir(const DxChar *aDirName,
									  DxChar* DirsBuff, DxUint32* DirsBuffSize, DxUint32* NumOfDirs,
									  DxChar* FilesBuff, DxUint32* FilesBuffSize, DxUint32* NumOfFiles)
{
    DX_DECLARE(DxStatus, result, DX_SUCCESS);

	if (NumOfDirs != DX_NULL)
		*NumOfDirs = 0;

	if (NumOfFiles != DX_NULL)
		*NumOfFiles = 0;

    DX_ASSERT_PARAM(aDirName != DX_NULL);

    DX_VERIFY_PARAM(*aDirName != 0);

	if (DirsBuff != NULL)
	{
        DX_ASSERT_PARAM(DirsBuffSize != DX_NULL);

        if (*DirsBuffSize == 0)
			RETURN_NEW_ERROR(DX_BUFFER_IS_NOT_BIG_ENOUGH);
		*DirsBuff = 0;
	}
	if (FilesBuff != NULL)
	{
        DX_ASSERT_PARAM(FilesBuffSize != DX_NULL);

        if (*FilesBuffSize == 0)
			RETURN_NEW_ERROR(DX_BUFFER_IS_NOT_BIG_ENOUGH);
		*FilesBuff = 0;
	}

    result = DX_VOS_BaseEnumerateDir(aDirName, DirsBuff, DirsBuffSize, NumOfDirs,
		    FilesBuff, FilesBuffSize, NumOfFiles);

    if (result != DX_SUCCESS)
    {
        DX_VOS_BaseLogFileResult();
        RETURN_OLD_ERROR(result);
    }
    DX_RETURN(DX_SUCCESS);
}

DxStatus  DX_VOS_RemoveDirectory(const DxChar *aDirName)
{

	DX_DECLARE(DxStatus, result, DX_SUCCESS);

    DX_ASSERT_PARAM(aDirName != DX_NULL);
    DX_VERIFY_PARAM(*aDirName != 0);

	/* Optimistic approach */
	if (DX_VOS_BaseRemoveDirectory(aDirName) == DX_SUCCESS)
		DX_RETURN(DX_SUCCESS);

	/* So we empty the directory */
	result = DX_VOS_BaseCleanDirectory(aDirName);
	if (result != DX_SUCCESS)
    {
        DX_VOS_BaseLogFileResult();
        DX_DBG_PRINT1(DX_DBGPRINT_ERROR_LEVEL, "Failed to clean directory: %s", aDirName);
		RETURN_OLD_ERROR(result);
    }

	/* And try again... */
	if (DX_VOS_BaseRemoveDirectory(aDirName) != DX_SUCCESS)
    {
        DX_VOS_BaseLogFileResult();
        DX_DBG_PRINT1(DX_DBGPRINT_ERROR_LEVEL, "Failed to remove directory: %s", aDirName);
        RETURN_NEW_ERROR(DX_VOS_FILE_ERROR);
    }

    DX_RETURN(DX_SUCCESS);
}

DxStatus DX_VOS_CopyFile (const DxChar *ExistingFileName, const DxChar *NewFileName, DxBool bOverwrite)
{
    DX_DECLARE(DxStatus, result, DX_SUCCESS);
	if (ExistingFileName == NULL || NewFileName == NULL)
		RETURN_NEW_ERROR(DX_BAD_ARGUMENTS);

	if ((bOverwrite == DX_FALSE) && DX_VOS_BaseIsFileExists(NewFileName))
		RETURN_NEW_ERROR(DX_VOS_FILE_ERROR);

	result = DX_VOS_BaseCopyFile(ExistingFileName, NewFileName);
    if (result != DX_SUCCESS)
    {
        DX_VOS_BaseLogFileResult();
        RETURN_OLD_ERROR(result);
    }
    DX_RETURN(DX_SUCCESS);

}

DxStatus DX_VOS_DeepDirCopy(const DxChar* source, const DxChar* dest)
{
    DX_DECLARE(DxStatus, result, DX_SUCCESS);
    DxUint32 buffSize = 1024;
    DxUint32 actualSize = 0;
    DxChar *buff = DX_NULL;
    DxChar *newBuff = DX_NULL;

    DxChar *dirName = DX_NULL;
    DxChar *fileName = DX_NULL;

	DxUint32 sourceLen = 0;
	DxUint32 destLen = 0;

    DxChar tmpSource[DX_VOS_MAX_PATH];
    DxChar tmpDest[DX_VOS_MAX_PATH];

    //before create will remove all
    result = DX_VOS_RemoveDirectory(dest);
    if (result != DX_SUCCESS)
	RETURN_OLD_ERROR(result);

    for(;;)
    {
        newBuff = (DxChar*)DX_VOS_MemRealloc(buff, buffSize);
        GOTO_END_IF_ALLOC_FAILED(newBuff);

        buff = newBuff;
        actualSize = buffSize;

        result = DX_VOS_EnumerateDir(source, buff, &actualSize, DX_NULL, DX_NULL, DX_NULL, DX_NULL);
        if (result == DX_BUFFER_IS_NOT_BIG_ENOUGH)
        {
            buffSize *= 2;
            continue;
        }

        if (result != DX_SUCCESS)
		GOTO_END_WITH_VAR_STATUS(result);

        if (result == DX_SUCCESS)
            break;
    }

    result = DX_VOS_CreateDirectory(dest);
    if (result != DX_SUCCESS)
        RETURN_OLD_ERROR(result);

    dirName = buff;
    sourceLen = DX_VOS_StrLen(source);
    destLen = DX_VOS_StrLen(dest);
    while(dirName != DX_NULL && *dirName != 0)
    {
	if (source[sourceLen - 1] == *DX_VOS_PATH_DIVIDER)
		result = DX_VOS_SPrintf(tmpSource, sizeof(tmpSource), "%s%s", source, dirName);
	else
		result = DX_VOS_SPrintf(tmpSource, sizeof(tmpSource), "%s" DX_VOS_PATH_DIVIDER "%s", source, dirName);
        if (result != DX_SUCCESS)
            GOTO_END_WITH_VAR_STATUS(result);

	if (dest[destLen - 1] == *DX_VOS_PATH_DIVIDER)
		result = DX_VOS_SPrintf(tmpDest, sizeof(tmpDest), "%s%s", dest, dirName);
	else
		result = DX_VOS_SPrintf(tmpDest, sizeof(tmpDest), "%s" DX_VOS_PATH_DIVIDER "%s", dest, dirName);

        if (result != DX_SUCCESS)
            GOTO_END_WITH_VAR_STATUS(result);

        result = DX_VOS_DeepDirCopy(tmpSource,tmpDest);//recursive call
        if (result != DX_SUCCESS)
            GOTO_END_WITH_VAR_STATUS(result);

        dirName = DX_VOS_NextStringInList(dirName);
    }

    for(;;)
    {
        actualSize = buffSize;

        result = DX_VOS_EnumerateDir(source, DX_NULL, DX_NULL, DX_NULL, buff, &actualSize, DX_NULL);
        if (result == DX_BUFFER_IS_NOT_BIG_ENOUGH)
        {
            buffSize *= 2;
            newBuff = (DxChar*)DX_VOS_MemRealloc(buff, buffSize);
            GOTO_END_IF_ALLOC_FAILED(newBuff);

            buff = newBuff;
            continue;
        }

        if (result != DX_SUCCESS)
            GOTO_END_WITH_VAR_STATUS(result);

        if (result == DX_SUCCESS)
            break;
    }

    fileName = buff;
    while(fileName != DX_NULL && *fileName != 0)
    {
	if (source[sourceLen - 1] == *DX_VOS_PATH_DIVIDER)
	        result = DX_VOS_SPrintf(tmpSource, sizeof(tmpSource), "%s%s", source, fileName);
	else
	        result = DX_VOS_SPrintf(tmpSource, sizeof(tmpSource), "%s" DX_VOS_PATH_DIVIDER "%s", source, fileName);

        if (result != DX_SUCCESS)
            GOTO_END_WITH_VAR_STATUS(result);

	if (dest[destLen - 1] == *DX_VOS_PATH_DIVIDER)
		result = DX_VOS_SPrintf(tmpDest, sizeof(tmpDest), "%s%s", dest, fileName);
	else
		result = DX_VOS_SPrintf(tmpDest, sizeof(tmpDest), "%s" DX_VOS_PATH_DIVIDER "%s", dest, fileName);

        if (result != DX_SUCCESS)
            GOTO_END_WITH_VAR_STATUS(result);

        result = DX_VOS_CopyFile(tmpSource, tmpDest, DX_TRUE);
        if (result != DX_SUCCESS)
            GOTO_END_WITH_VAR_STATUS(result);

        fileName = DX_VOS_NextStringInList(fileName);
    }

end:
    DX_VOS_MemFree(buff);
    if (result != DX_SUCCESS)
	RETURN_OLD_ERROR(result);

    DX_RETURN(DX_SUCCESS);
}


#endif

const DxChar* DX_VOS_GetBaseFileName(const DxChar* fileName)
{
    const DxChar* baseFileName = 0;
    baseFileName = DX_VOS_StrRChr(fileName, '/');
    if (baseFileName == DX_NULL)
    {
        baseFileName = DX_VOS_StrRChr(fileName, '\\');
        if (baseFileName == DX_NULL)
            baseFileName = fileName;
        else
            baseFileName++;
    }
    else
        baseFileName++;
    return baseFileName;
}

DxStatus DX_VOS_DirNCat(DxChar *aDirName1, DxUint32 aBuf_size, const DxChar *aDirName2)
{
	size_t dir1Len = 0, dir2Len = 0, dividerLen = 0;
	DX_DECLARE(DxStatus, result, DX_SUCCESS);

    DX_ASSERT_PARAM(aDirName1 != DX_NULL);
    DX_ASSERT_PARAM(aDirName2 != DX_NULL);

	dir1Len = DX_VOS_StrNLen(aDirName1, DX_VOS_MAX_PATH);
	dir2Len = DX_VOS_StrNLen(aDirName2, DX_VOS_MAX_PATH);
	/* The Divider is appended only if it is not already exists */
	if (dir1Len > 0 && aDirName1[dir1Len - 1] != *DX_VOS_PATH_DIVIDER)
        dividerLen = 1;
	else
        dividerLen = 0;

	if (dir1Len + dir2Len + dividerLen + 1 > aBuf_size)
		RETURN_NEW_ERROR(DX_BUFFER_IS_NOT_BIG_ENOUGH);

	if (dividerLen > 0)
	{
		result = DX_VOS_StrNCat(aDirName1, aBuf_size, DX_VOS_PATH_DIVIDER);
		if (result != DX_SUCCESS)
			RETURN_OLD_ERROR(result);
	}

	result = DX_VOS_StrNCat(aDirName1, aBuf_size, aDirName2);
	if (result != DX_SUCCESS)
		RETURN_OLD_ERROR(result);

	DX_RETURN(DX_SUCCESS);
}

DxStatus DX_VOS_BuildPathStr(DxChar* Buff, DxUint BuffSize, const DxChar* fmt, ...)
{
	DX_VA_LIST arg_list;
	const DxChar *p;
	DxUint32 i;
	DxChar *s;
	DxChar fmtbuf[16];
	DX_DECLARE(DxStatus, result, DX_SUCCESS);
	DX_VA_START(arg_list, fmt);

    DX_ASSERT_PARAM(Buff != DX_NULL);
    DX_ASSERT_PARAM(fmt != DX_NULL);

	*Buff = 0;
	for(p = fmt; *p != '\0' && result == DX_SUCCESS; p++)
	{
		if(*p != '%')
		{
			fmtbuf[0] = *p;
			fmtbuf[1] = 0;
			result = DX_VOS_StrNCat(Buff, BuffSize, fmtbuf);
			continue;
		}
		switch(*++p)
		{
		case 'd':
			result = DX_VOS_StrNCat(Buff, BuffSize, DX_VOS_PATH_DIVIDER);
			break;
		case 's':
			s = DX_VA_ARG(arg_list,DxChar*);
			result = DX_VOS_StrNCat(Buff, BuffSize, s);
			break;
		case 'c':	/* 8-bit int */
			i = DX_VA_ARG(arg_list,DxUint8);
			DX_VOS_UnsignedToStr(i, fmtbuf, sizeof(fmtbuf), 16);
			result = DX_VOS_StrNCat(Buff, BuffSize, fmtbuf);
			break;
		case 'u':  /* 16- or 32-bit int */
			switch(*++p)
			{
			case 'l':
				i = DX_VA_ARG(arg_list,DxUint32);
				DX_VOS_UnsignedToStr(i, fmtbuf, sizeof(fmtbuf), 16);
				result = DX_VOS_StrNCat(Buff, BuffSize, fmtbuf);
				break;
			case 'h':
				p++; /* This cancels the following push back */
			default:
				p--;	/* someone forgot the h or l, push back the character and assume 16bit.  */
				i = DX_VA_ARG(arg_list,DxUint16);
				DX_VOS_UnsignedToStr(i, fmtbuf, sizeof(fmtbuf), 16);
				result = DX_VOS_StrNCat(Buff, BuffSize, fmtbuf);
				break;
			}
			break;
		case '%':
			result = DX_VOS_StrNCat(Buff, BuffSize, "%");
			break;
		}
	}
	RETURN_OLD_ERROR(result);
}


const DxChar* DX_VOS_FullPathToFileName(const DxChar *aFullPathName)
{
	const DxChar *pos = DX_NULL;

	pos = DX_VOS_StrRChr(aFullPathName, *DX_VOS_PATH_DIVIDER);
	if (pos == DX_NULL)
		DX_RETURN(aFullPathName);
	else
		return pos + 1;
}


#ifdef DX_USE_LEGACY_VOS
DxVosFile DX_VOS_Fopen(const DxChar* fileName, const DxChar* mode, DxStatus* errorCode)
{
    DxVosFile hFile = DX_NULL;
    DxStatus error = DX_VOS_FOpen(&hFile, fileName, mode);
    if (errorCode != DX_NULL)
        *errorCode = error;
    DX_RETURN(hFile);
}

DxStatus DX_VOS_FreadEx(void* aBuf, DxUint32 aSize, DxUint32 aNitems, DxUint* aReadBytesPtr, DxVosFile aFileHandle)
{
    DX_DECLARE(DxStatus, result, DX_SUCCESS);
    result = DX_VOS_FReadEx(aFileHandle, aBuf, (aSize) * (aNitems), (DxUint32*)aReadBytesPtr);
    if (result != DX_SUCCESS)
	RETURN_OLD_ERROR(result);

    if (*aReadBytesPtr != (aSize) * (aNitems))
        RETURN_NEW_ERROR(DX_VOS_FILE_ERROR);

    DX_RETURN(DX_SUCCESS);
}

#endif
