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

#include "DX_VOS_Stdio.h"
#include "DX_VOS_Utils.h"
#include "DX_VOS_String.h"
#include "DX_VOS_Mem.h"
#include "log_output.h"
#include "DX_VOS_File.h"
#include "DX_VOS_BaseStdio.h"


void DX_VOS_Printf (const DxChar *format,...)
{
    DX_VA_LIST arg_list;
    DX_VA_START(arg_list,format);
    DX_VOS_VPrintf(format, arg_list);
    DX_VA_END(arg_list);
}
#define MAX_MESSAGE_LEN 2048

void DX_VOS_VPrintf (const DxChar *format, DX_VA_LIST arg_list)
{
    DxChar data[MAX_MESSAGE_LEN];
    if (format == DX_NULL)
        return;
    DX_VOS_VSPrintf(data, sizeof(data), format, arg_list);
 //   DX_VOS_PrintString(data);
    PRINTF(data);
}

DxStatus DX_VOS_SPrintf (DxChar *aBuff, DxUint aBuffSize, const DxChar *format,...)
{
    DX_DECLARE(DxStatus, result, DX_SUCCESS);
    DX_VA_LIST arg_list;
    DX_VA_START(arg_list, format);
    result = DX_VOS_VSPrintf(aBuff, aBuffSize, format, arg_list);
    DX_VA_END(arg_list);
    if (result != DX_SUCCESS)
	RETURN_OLD_ERROR(result);
    DX_RETURN(DX_SUCCESS);
}

DxStatus DX_VOS_VSPrintf (DxChar *aBuff, DxUint aBuffSize, const DxChar *format,DX_VA_LIST arg_list)
{
    const DxChar *currPos = format;
    DxUint buffIndex = 0;
    DxChar tempBuff[35];
    DxUint tempIndex = 0;
    DX_DECLARE(DxStatus, result, DX_SUCCESS);

    DX_ASSERT_PARAM(aBuff != DX_NULL);
    DX_ASSERT_PARAM(format != DX_NULL);

    if (aBuffSize == 0)
        RETURN_NEW_ERROR(DX_BUFFER_IS_NOT_BIG_ENOUGH);

    *aBuff = 0;
    aBuffSize--;

    while (*currPos != 0 && buffIndex <= aBuffSize && result == DX_SUCCESS)
    {
        DxChar pad = ' ';
        DxInt32 padLength = 0;

        if(*currPos != '%')
        {
            aBuff[buffIndex++] = *currPos++;
            continue;
        }
        currPos++;

        if (*currPos == '0')
            pad = *currPos++;

        tempIndex = 0;
        while (*currPos >='0' && *currPos <= '9' && tempIndex < 4)
            tempBuff[tempIndex++] = *currPos++;
        tempBuff[tempIndex] = 0;
        if (tempIndex >= 1)
        {
            result = DX_VOS_StrToInt(tempBuff, &padLength, 10);
            if (result != DX_SUCCESS)
                break;
        }
        tempIndex = 0;
        tempBuff[tempIndex] = 0;
        switch(*currPos)
        {
        case 'c':
            {
                DxChar val = DX_VA_ARG(arg_list, DxChar);
                aBuff[buffIndex++] = val;
            }
            break;

        case 'd':
        case 'i':
            {
                DxInt32 val = DX_VA_ARG(arg_list, DxInt32);
                result = DX_VOS_IntToStr(val, tempBuff, sizeof(tempBuff), 10);
            }
            break;
        case 'o':
        case 'u':
        case 'x':
        case 'X':
            {
                DxUint32 val = DX_VA_ARG(arg_list, DxUint32);
                DxUint radix = 10;
                if (*currPos == 'o') radix = 8;
                if (*currPos == 'x' || *currPos == 'X') radix = 16;
                result = DX_VOS_UnsignedToStr(val, tempBuff, sizeof(tempBuff), radix);
                if (result == DX_SUCCESS && *currPos == 'x')
                    DX_VOS_StrToLower(tempBuff, sizeof(tempBuff));
            }
            break;
        case 'B':
		case 'b':
            {
				const DxChar* digits = *currPos == 'B' ? "0123456789ABCDEF" : "0123456789abcdef";
                const DxChar* data = DX_VA_ARG(arg_list, DxChar*);
                DxUint32 dataSize = DX_VA_ARG(arg_list, DxUint32);
                if (dataSize * 2  > aBuffSize - buffIndex)
                {
                    dataSize = (aBuffSize - buffIndex) / 2;
                    result = DX_BUFFER_IS_NOT_BIG_ENOUGH;
                }

                while (dataSize-- > 0)
                {
                    aBuff[buffIndex++] = digits[((*data) >> 4) & 0xF];
                    aBuff[buffIndex++] = digits[*data++ & 0xF];
                }
            }
            break;
        case 's':
            {
                const DxChar* val = DX_VA_ARG(arg_list, DxChar*);
                if (val != DX_NULL)
                {
                    DxUint strLen = DX_VOS_StrLen(val);
                    if (strLen > aBuffSize - buffIndex)
					{
                        result = DX_BUFFER_IS_NOT_BIG_ENOUGH;
						strLen = aBuffSize - buffIndex;
					}
                    padLength -= strLen;
                    while (padLength-- > 0 && buffIndex < aBuffSize)
                        aBuff[buffIndex++] = pad;

                    DX_VOS_FastMemCpy(aBuff + buffIndex, val, strLen);
                    buffIndex += strLen;
                }
            }
            break;
        case 'S':
            {
                const DxWideChar* val = DX_VA_ARG(arg_list, DxWideChar*);
                if (val != DX_NULL)
                {
                    result = DX_VOS_Utf16ToUtf8(aBuff + buffIndex, aBuffSize - buffIndex, val);
                    buffIndex += DX_VOS_StrLen(aBuff + buffIndex);
                }
            }
            break;
		case 'n':
			{
				const DxChar* data = DX_VA_ARG(arg_list, DxChar*);
				DxUint32 dataSize = DX_VA_ARG(arg_list, DxUint32);
				if (dataSize  > aBuffSize - buffIndex)
				{
					dataSize = aBuffSize - buffIndex;
					result = DX_BUFFER_IS_NOT_BIG_ENOUGH;
				}

				while (dataSize-- > 0)
					aBuff[buffIndex++] = *data++;
			}
			break;
        case '%':
            aBuff[buffIndex++] = '%';
            break;
        default:
            result = DX_INVALID_FORMAT;
        }
        currPos++;
        if (result != DX_SUCCESS)
            break;
        tempIndex = DX_VOS_StrLen(tempBuff);
        if (*tempBuff != 0)
        {
            padLength -= tempIndex;
            while (padLength-- > 0 && buffIndex < aBuffSize)
                aBuff[buffIndex++] = pad;

            result = DX_VOS_MemCpy(aBuff + buffIndex, aBuffSize - buffIndex, tempBuff, tempIndex);
            buffIndex += DX_MIN(aBuffSize - buffIndex, tempIndex);
        }
    }
	if (buffIndex > aBuffSize)
	{
		buffIndex--;
		result = DX_BUFFER_IS_NOT_BIG_ENOUGH;
	}
    aBuff[buffIndex] = 0;
    if (result == DX_SUCCESS && *currPos != 0)
        result = DX_BUFFER_IS_NOT_BIG_ENOUGH;

    if (result != DX_SUCCESS)
	RETURN_OLD_ERROR(result);
    DX_RETURN(DX_SUCCESS);
}

#ifndef DX_NO_FILE_UTILS
DxStatus DX_VOS_FPrintf (DxVosFile aFile, const DxChar *format,...)
{
    DX_DECLARE(DxStatus, result, DX_SUCCESS);
    DX_VA_LIST arg_list;
    DX_VA_START(arg_list, format);
    result = DX_VOS_VFPrintf(aFile, format, arg_list);
    DX_VA_END(arg_list);
    if (result != DX_SUCCESS)
	RETURN_OLD_ERROR(result);
    DX_RETURN(DX_SUCCESS);
}

DxStatus  DX_VOS_VFPrintf (DxVosFile aFile, const DxChar *format, DX_VA_LIST arg_list)
{
    DX_DECLARE(DxStatus, result, DX_SUCCESS);
    DxStatus result2 = DX_SUCCESS;
    DxChar data[MAX_MESSAGE_LEN];

    DX_ASSERT_PARAM(aFile != DX_NULL);
    DX_ASSERT_PARAM(format != DX_NULL);

    result = DX_VOS_VSPrintf(data, sizeof(data), format, arg_list);
    result2 = DX_VOS_FWrite(aFile, data, DX_VOS_StrLen(data));
    result = DX_MAX(result,result2);
    if (result != DX_SUCCESS)
	RETURN_OLD_ERROR(result);
    DX_RETURN(DX_SUCCESS);
}

#endif

/*DxStatus  DX_VOS_GetString(DxChar *aStr, DxUint aBuffSize)
{
    DxUint charsRead = 0;

    DX_ASSERT_PARAM(aStr != DX_NULL);
    DX_ASSERT_PARAM(aBuffSize > 0);*/

   // aBuffSize--; /* Saving Space for terminating NULL */
    /*for(;;)
    {
        DxInt Ch = DX_VOS_GetCh();
        DX_VOS_Printf("%c", (DxChar)Ch);
        if (Ch == '\n' || Ch == '\r')
            DX_RETURN(DX_SUCCESS);
        if (charsRead == aBuffSize)
            continue;
        aStr[charsRead++] = (DxChar)Ch;
        aStr[charsRead] = 0;
    }
}*/

typedef const DxChar* DxStringList[9];

DxStatus EmbedStringArgs(DxChar *aBuff, DxUint aBuffSize, const DxChar *format, DxStringList* stringList)
{
	DX_DECLARE(DxStatus, result, DX_SUCCESS);
	const DxChar *currPos = format;
	const DxChar *val = DX_NULL;
	DxUint buffIndex = 0;
	DxChar tempBuff[2];

	DX_ASSERT_PARAM(aBuff != DX_NULL);
	DX_ASSERT_PARAM(format != DX_NULL);
	DX_ASSERT_PARAM(stringList != DX_NULL);

	if (aBuffSize == 0)
		RETURN_CONST_STATUS(DX_BUFFER_IS_NOT_BIG_ENOUGH);

	*aBuff = 0;
	aBuffSize--;

	while (*currPos != 0 && buffIndex <= aBuffSize && result == DX_SUCCESS)
	{
		DxInt32 argIndex = 0;

		if(*currPos != '%')
		{
			aBuff[buffIndex++] = *currPos++;
			continue;
		}
		currPos++;

		if(*currPos == '%')
		{
			aBuff[buffIndex++] = '%';
			currPos++;
			continue;
		}

		if (*currPos < '1' || *currPos > '9')
		{
			result = DX_INVALID_FORMAT;
			break;
		}

		tempBuff[0] = *currPos++;
		tempBuff[1] = 0;

		result = DX_VOS_StrToInt(tempBuff, &argIndex, 10);
		if (result != DX_SUCCESS)
			break;

		if (argIndex == 0 || argIndex > DX_ITEMS_IN_ARRAY(*stringList))
		{
			result = DX_INVALID_FORMAT;
			break;
		}

		val = (*stringList)[argIndex - 1];	// 0 < argIndex < 10
		if (val != DX_NULL)
		{
			DxUint strLen = DX_VOS_StrLen(val);
			if (strLen > aBuffSize - buffIndex)
			{
				result = DX_BUFFER_IS_NOT_BIG_ENOUGH;
				strLen = aBuffSize - buffIndex;
			}

			DX_VOS_FastMemCpy(aBuff + buffIndex, val, strLen);
			buffIndex += strLen;
		}
	}
	if (buffIndex > aBuffSize)
	{
		buffIndex--;
		result = DX_BUFFER_IS_NOT_BIG_ENOUGH;
	}
	aBuff[buffIndex] = 0;
	if (result == DX_SUCCESS && *currPos != 0)
		result = DX_BUFFER_IS_NOT_BIG_ENOUGH;

	if (result != DX_SUCCESS)
		RETURN_VAR_STATUS(result);
	DX_RETURN(DX_SUCCESS);
}

DxStatus DX_VOS_EmbedStringArgs( DxChar *aBuff, DxUint aBuffSize, const DxChar *format, DxUint32 numOfArgs, ...)
{
	DX_DECLARE(DxStatus, result, DX_SUCCESS);
	DxStringList stringList = {0};
	DxUint32 index = 0;
	DX_VA_LIST arg_list;
	DX_VA_START(arg_list, numOfArgs);

	if (numOfArgs > DX_ITEMS_IN_ARRAY(stringList))
		RETURN_CONST_STATUS(DX_NOT_SUPPORTED);

	for (index = 0; index < numOfArgs; index++)
		stringList[index] = DX_VA_ARG(arg_list, DxChar*);
	DX_VA_END(arg_list);

	result = EmbedStringArgs(aBuff, aBuffSize, format, &stringList);
	if (result != DX_SUCCESS)
		RETURN_VAR_STATUS(result);

	DX_RETURN(DX_SUCCESS);
}
