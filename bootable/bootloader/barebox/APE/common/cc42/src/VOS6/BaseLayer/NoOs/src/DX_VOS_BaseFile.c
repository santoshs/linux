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
#include "DX_VOS_Mem.h"
#include "DX_VOS_String.h"
#include "DX_VOS_Utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
// Parameter validation already done.
void DX_VOS_BaseLogFileResult(void)
{

}
DxStatus DX_VOS_BaseFOpen(DxVosFile *aFileHandle, const DxChar *aFileName, const DxChar *aMode)
{
	*aFileHandle = (DxVosFile)fopen(aFileName, aMode);

	if (*aFileHandle ==  DX_NULL)
		RETURN_CONST_STATUS(DX_VOS_FILE_ERROR);

	DX_RETURN(DX_SUCCESS);
}

DxStatus DX_VOS_BaseGetOsHandleFromFileHandle(DxVosFile aFileHandle, void** osHandle)
{
    *osHandle = aFileHandle;
    DX_RETURN(DX_SUCCESS);
}

DxStatus DX_VOS_BaseCreateFileHandleFromOsHandle(DxVosFile* aFileHandle, void* osHandle)
{
    *aFileHandle = (DxVosFile)osHandle;
    DX_RETURN(DX_SUCCESS);
}

DxStatus DX_VOS_BaseFileOpen(DxVosFile *aFileHandle, const DxChar *aFileName,  EDxOpenMode openMode, EDxShareMode shareMode)
{

    char * openModeString;
    switch(openMode) /* switch on enum EDxOpenMode values */
    {
    case(DX_FILE_CREATE):
    case(DX_FILE_WRITE_ONLY):
        openModeString = "wb";
        break;
    case(DX_FILE_READ):
//    case(DX_FILE_READ_ONLY): same value
        openModeString = "rb";
        break;
    case(DX_FILE_OPEN_EXISITING):
        openModeString = "r+b";
        break;
    case(DX_FILE_CREATE_NEW):
    case(DX_FILE_OPEN_ALWAYS):
        openModeString = "w+b";
        break;
    case(DX_FILE_TRUNCATE):
        /* can't just truncate on NoOs */
    default:
        return DX_FAILURE;
    }

    shareMode = shareMode; // This parameter is ignored

    *aFileHandle = (DxVosFile)fopen(aFileName, openModeString);

	if (*aFileHandle ==  DX_NULL)
		RETURN_CONST_STATUS(DX_VOS_FILE_ERROR);

    DX_RETURN(DX_SUCCESS);
}

DxStatus DX_VOS_BaseFileClose (DxVosFile aFileHandle)
{
	if (fclose((FILE *)aFileHandle) != 0)
		RETURN_CONST_STATUS(DX_VOS_FILE_ERROR);
	DX_RETURN(DX_SUCCESS);
}

DxStatus DX_VOS_BaseFileRead(DxVosFile aFileHandle, void* aBuf, DxUint32_t aSize,  DxUint32_t* aReadBytesPtr)
{
	*aReadBytesPtr = fread(aBuf, 1, aSize, (FILE*)aFileHandle);
	DX_RETURN(DX_SUCCESS);
}

DxStatus DX_VOS_BaseFileWrite(DxVosFile aFileHandle, const void *aBuf, DxUint32_t aSize)
{
	DxUint_t numWritten = fwrite(aBuf, 1, aSize, (FILE*)aFileHandle);
	if (numWritten != aSize)
		RETURN_CONST_STATUS(DX_VOS_FILE_ERROR);
	DX_RETURN(DX_SUCCESS);
}

DxStatus DX_VOS_BaseFileSeek(DxVosFile aFileHandle, DxInt32_t aOffset,DxInt_t aOrigin)
{
	if (fseek((FILE *)aFileHandle, aOffset, aOrigin) != 0)
		RETURN_CONST_STATUS(DX_VOS_FILE_ERROR);  /* fseek returns 0 if successful */
	DX_RETURN(DX_SUCCESS);
}

DxStatus DX_VOS_BaseFileTell(DxVosFile aFileHandle, DxUint32_t* aCurPosPtr)
{
	DxUint32_t CurrPos = ftell((FILE*)aFileHandle);
	if (CurrPos == 0xFFFFFFFF)
		RETURN_CONST_STATUS(DX_VOS_FILE_ERROR);

	*aCurPosPtr = CurrPos;

	DX_RETURN(DX_SUCCESS);
}

DxStatus DX_VOS_BaseFileEof(DxVosFile aFileHandle)
{
	if (feof((FILE *)aFileHandle ) != 0)
		RETURN_CONST_WARNING(DX_VOS_END_OF_FILE);

	DX_RETURN(DX_SUCCESS);
}

DxStatus DX_VOS_BaseFileFlush  (DxVosFile aFileHandle)
{
	if (fflush((FILE *)aFileHandle) != 0)
		RETURN_CONST_STATUS(DX_VOS_FILE_ERROR);

	DX_RETURN(DX_SUCCESS);
}

DxStatus DX_VOS_BaseFileDelete(const DxChar *aFileName)
{
	if (remove(aFileName) != 0)
		RETURN_CONST_STATUS(DX_VOS_FILE_ERROR);

	DX_RETURN(DX_SUCCESS);
}

DxStatus DX_VOS_BaseFileRename(const DxChar *aOrigName, const DxChar *aNewName)
{
	if (rename(aOrigName, aNewName) != 0)
		RETURN_CONST_STATUS(DX_VOS_FILE_ERROR);

	DX_RETURN(DX_SUCCESS);
}

DxStatus DX_VOS_BaseCreateDirectory(const DxChar *aDirName)
{
	// If the path is of the root directory the function should succeed.
/*
	if (DX_VOS_StrCmp(aDirName, DX_VOS_DEFAULT_ROOT) == 0)
		DX_RETURN(DX_SUCCESS);
	if (_mkdir(aDirName) != 0)
	{
		if (errno == EEXIST)
			DX_RETURN(DX_SUCCESS);
		RETURN_CONST_STATUS(DX_VOS_FILE_ERROR);
	}
*/
	DX_RETURN(DX_SUCCESS);
}
DxBool_t DX_VOS_BaseIsFileExists(const DxChar* fileName)
{
    DxVosFile fileHandle;
    if (DX_VOS_BaseFileOpen(&fileHandle, fileName, DX_FILE_READ, DX_SHARE_READ_WRITE) != DX_SUCCESS)
		DX_RETURN(DX_FALSE);
    DX_VOS_BaseFileClose(fileHandle);
    DX_RETURN(DX_TRUE);
}
