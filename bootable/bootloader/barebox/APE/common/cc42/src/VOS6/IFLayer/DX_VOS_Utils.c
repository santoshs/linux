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

#include "DX_VOS_Utils.h"
#include "DX_VOS_Mem.h"
#include "DX_VOS_String.h"

DxChar* DX_VOS_SkipWhiteSpaces(const DxChar* aString, const DxChar* whiteSpaces)
{
	if (aString == DX_NULL)
		DX_RETURN(DX_NULL);

	while (DX_VOS_StrChr(whiteSpaces, *aString) != DX_NULL)
		aString++;

	return (DxChar*)aString;
}

DxStatus DX_VOS_NStrToInt(const DxChar *aString, DxUint32 stringSize, DxInt32* value, DxUint base)
{
	DxInt sign = 1;
    const DxChar* stringStart = aString;

    DX_ASSERT_PARAM(aString != DX_NULL);
    DX_ASSERT_PARAM(value != DX_NULL);

	if (base == 1 || base > 16)
		RETURN_NEW_ERROR(DX_BAD_ARGUMENTS);

	*value = 0;
	aString = DX_VOS_SkipWhiteSpaces(aString, " \t");
	if (*aString == '-')
	{
		sign = -1;
		aString++;
	}
	else if (*aString == '+')
	{
		sign = 1;
		aString++;
	}
	if (base == 0)
	{
		if (*aString == 'b')
		{
			base = 2;
			aString++;
		}
		else if (*aString == '0')
		{
			base = 8;
			aString++;
			if (*aString == 'x')
			{
				base = 16;
				aString++;
			}
		} else
			base = 10;
	}
	while (*aString != 0 && (DxUint32)(aString  - stringStart) < stringSize)
	{
		DxUint digitVal = 100;
		DxInt32 oldValue = *value;
		if (*aString >= '0' && *aString <= '9') digitVal = *aString - '0';
		if (*aString >= 'a' && *aString <= 'f') digitVal = 10 + *aString - 'a';
		if (*aString >= 'A' && *aString <= 'F') digitVal = 10 + *aString - 'A';
		if (digitVal > base)
			RETURN_NEW_ERROR(DX_INVALID_FORMAT);
		*value = *value * base + digitVal;
		if (*value < oldValue)
			RETURN_NEW_ERROR(DX_OVERFLOW);
		++aString;
	}

	*value *= sign;
	DX_RETURN(DX_SUCCESS);
}

/*DxStatus DX_VOS_NStrToUnsigned(const DxChar *aString, DxUint32 stringSize, DxUint32* value, DxUint base)
{
    const DxChar* stringStart = aString;

    DX_ASSERT_PARAM(aString != DX_NULL);
    DX_ASSERT_PARAM(value != DX_NULL);

	if (base == 1 || base > 16)
		RETURN_NEW_ERROR(DX_BAD_ARGUMENTS);

	*value = 0;
	aString = DX_VOS_SkipWhiteSpaces(aString, " \t");
	if (base == 0)
	{
		if (*aString == 'b')
		{
			base = 2;
			++aString;
		}
		else if (*aString == '0')
		{
			base = 8;
			aString++;
			if (*aString == 'x')
			{
				base = 16;
				++aString;
			}
		} else
			base = 10;
	}
	while (*aString != 0 && (DxUint32)(aString  - stringStart) < stringSize)
	{
		DxUint digitVal = 100;
		if (*aString >= '0' && *aString <= '9') digitVal = *aString - '0';
		if (*aString >= 'a' && *aString <= 'f') digitVal = 10 + *aString - 'a';
		if (*aString >= 'A' && *aString <= 'F') digitVal = 10 + *aString - 'A';
		if (digitVal > base)
			RETURN_NEW_ERROR(DX_INVALID_FORMAT);
        if (*value > ((DxUint)(-1) - digitVal) / base)
            RETURN_NEW_ERROR(DX_OVERFLOW);
		*value = *value * base + digitVal;
		++aString;
	}
	DX_RETURN(DX_SUCCESS);
}*/

DxStatus DX_VOS_IntToStr(DxInt32 aValue, DxChar *aString, DxUint aBuffSize, DxInt aRadix)
{
	DxChar tempBuff[33];
	DxUint i = 0, j = 0;
	DxInt sign = 1;
	const DxChar* digits = "0123456789ABCDEF";

    DX_ASSERT_PARAM(aString != DX_NULL);

	if (aRadix < 2 || aRadix > 16)
		RETURN_NEW_ERROR(DX_BAD_ARGUMENTS);

	if (aValue < 0)
	{
		sign = -1;
		aValue = -aValue;
	}

	while (aValue > 0)
	{
		tempBuff[i++] = digits[aValue % aRadix];
		aValue /= aRadix;
	}

	if (i == 0)
		tempBuff[i++] = '0';

	if (sign < 0)
		tempBuff[i++] = '-';

	if (i >= aBuffSize)
		RETURN_NEW_ERROR(DX_BUFFER_IS_NOT_BIG_ENOUGH);

	while (i > 0)
		aString[j++] = tempBuff[--i];

	aString[j] = 0;

	DX_RETURN(DX_SUCCESS);
}

DxStatus DX_VOS_UnsignedToStr(DxUint32 aValue, DxChar *aString, DxUint aBuffSize, DxInt aRadix)
{
	DxChar tempBuff[33];
	DxUint i = 0, j = 0;
	const DxChar* digits = "0123456789ABCDEF";

    DX_ASSERT_PARAM(aString != DX_NULL);

	if (aRadix < 2 || aRadix > 16)
		RETURN_NEW_ERROR(DX_BAD_ARGUMENTS);

	while (aValue > 0)
	{
		tempBuff[i++] = digits[aValue % aRadix];
		aValue /= aRadix;
	}

	if (i == 0)
		tempBuff[i++] = '0';

	if (i >= aBuffSize)
		RETURN_NEW_ERROR(DX_BUFFER_IS_NOT_BIG_ENOUGH);

	while (i > 0)
		aString[j++] = tempBuff[--i];

	aString[j] = 0;

	DX_RETURN(DX_SUCCESS);
}

DxStatus DX_VOS_Utf16ToUtf8(DxChar *aUtf8, DxUint aUtf8BuffSize, const DxWideChar *aUtf16)
{
	DX_DECLARE(DxStatus, result, DX_SUCCESS);
	DxUint currPos = 0;

    if (aUtf16 == DX_NULL)
    {
        if (aUtf8 != DX_NULL && aUtf8BuffSize >= sizeof(DxChar))
            *aUtf8 = 0;
        DX_RETURN(DX_SUCCESS);
    }
    DX_ASSERT_PARAM(aUtf8 != DX_NULL);

	if (aUtf8BuffSize == 0)
		RETURN_NEW_ERROR(DX_BUFFER_IS_NOT_BIG_ENOUGH);

	aUtf8BuffSize--; /* Saving Space to terminating NULL */

	while (*aUtf16 != 0 && currPos < aUtf8BuffSize)
	{
		DxUint32 value = 0;
		if ((*aUtf16 < 0xD800) || (*aUtf16 > 0xDFFF))
			value = *aUtf16++;
		else {
			if (*aUtf16 > 0xDBFF)
			{
				result = DX_INVALID_FORMAT;
				break;
			}
			value = *aUtf16++ & 0x03FF;
			if (*aUtf16 < 0xDC00 || *aUtf16 > 0xDFFF)
			{
				result = DX_INVALID_FORMAT;
				break;
			}
			value = (value << 10) | (*aUtf16++ & 0x03FF);
			value += 0x10000;
		}

		if (value <= 0x7F)
			aUtf8[currPos++] = (DxChar) value;
		else if (value <= 0x7FF)
		{
			if (currPos + 2 > aUtf8BuffSize)
			{
				result = DX_BUFFER_IS_NOT_BIG_ENOUGH;
				break;
			}
			aUtf8[currPos++] = (DxChar) (0xC0 | ((value >> 6) & 0x1F));
			aUtf8[currPos++] = (DxChar) (0x80 | (value & 0x3F));
		}
		else if (value <= 0xFFFF)
		{
			if (currPos + 3 > aUtf8BuffSize)
			{
				result = DX_BUFFER_IS_NOT_BIG_ENOUGH;
				break;
			}
			aUtf8[currPos++] = (DxChar) (0xE0 | ((value >> 12) & 0xF));
			aUtf8[currPos++] = (DxChar) (0x80 | ((value >> 6)& 0x3F));
			aUtf8[currPos++] = (DxChar) (0x80 | (value & 0x3F));
		}
		else if (value <= 0x10FFFF)
		{
			if (currPos + 4 > aUtf8BuffSize)
			{
				result = DX_BUFFER_IS_NOT_BIG_ENOUGH;
				break;
			}
			aUtf8[currPos++] = (DxChar) (0xF0 | ((value >> 18) & 0x7));
			aUtf8[currPos++] = (DxChar) (0x80 | ((value >> 12) & 0x3F));
			aUtf8[currPos++] = (DxChar) (0x80 | ((value >> 6)& 0x3F));
			aUtf8[currPos++] = (DxChar) (0x80 | (value & 0x3F));
		} else
		{
			result = DX_INVALID_FORMAT;
			break;
		}
	}

	aUtf8[currPos] = 0;

	if (result == DX_SUCCESS && *aUtf16 != 0)
		result = DX_BUFFER_IS_NOT_BIG_ENOUGH;

    if (result != DX_SUCCESS)
	RETURN_OLD_ERROR(result);
    DX_RETURN(DX_SUCCESS);
}

#ifdef BIG__ENDIAN
#define DX_CONVERT_TO_16_BIT(value)       \
	(value << 8)
#else
	#define DX_CONVERT_TO_16_BIT(value) \
	value
#endif
DxStatus DX_VOS_Utf8ToUtf16(DxWideChar *aUtf16, DxUint aUtf16BuffSize, const DxChar *aUtf8)
{
	DX_DECLARE(DxStatus, result, DX_SUCCESS);
	DxUint currPos = 0;

    if (aUtf8 == DX_NULL)
    {
        if (aUtf16 != DX_NULL && aUtf16BuffSize >= sizeof(DxWideChar))
            *aUtf16 = 0;
        DX_RETURN(DX_SUCCESS);
    }
    DX_ASSERT_PARAM(aUtf16 != DX_NULL);

	aUtf16BuffSize /= sizeof(DxUint16);

	if (aUtf16BuffSize == 0)
		RETURN_NEW_ERROR(DX_BUFFER_IS_NOT_BIG_ENOUGH);

	aUtf16BuffSize--; /* Saving Space to terminating NULL */

	while (*aUtf8 != 0 && currPos < aUtf16BuffSize)
	{
		DxUint32 value = 0;
		if ((*aUtf8 & 0x80) == 0)
			value = *aUtf8++;
		else if ((*aUtf8 & 0xE0) == 0xC0)
		{
			value = (DxUint32)(*aUtf8++ & 0x1F) << 6;
			if ((*aUtf8 & 0xC0) != 0x80)
			{
				result = DX_INVALID_FORMAT;
				break;
			}
			value |= *aUtf8++ & 0x3F;
		}
		else if ((*aUtf8 & 0xF0) == 0xE0)
		{
			value = (DxUint32)(*aUtf8++ & 0xF) << 12;
			if ((*aUtf8 & 0xC0) != 0x80)
			{
				result = DX_INVALID_FORMAT;
				break;
			}
			value |= (*aUtf8++ & 0x3F) << 6;
			if ((*aUtf8 & 0xC0) != 0x80)
			{
				result = DX_INVALID_FORMAT;
				break;
			}
			value |= *aUtf8++ & 0x3F;
		}
		else if ((*aUtf8 & 0xF8) == 0xF0)
		{
			value = (DxUint32)(*aUtf8++ & 0x7) << 18;
			if ((*aUtf8 & 0xC0) != 0x80)
			{
				result = DX_INVALID_FORMAT;
				break;
			}
			value |= (*aUtf8++ & 0x3F) << 12;
			if ((*aUtf8 & 0xC0) != 0x80)
			{
				result = DX_INVALID_FORMAT;
				break;
			}
			value |= (*aUtf8++ & 0x3F) << 6;
			if ((*aUtf8 & 0xC0) != 0x80)
			{
				result = DX_INVALID_FORMAT;
				break;
			}
			value |= *aUtf8++ & 0x3F;
		}
		else
		{
			result = DX_INVALID_FORMAT;
			break;
		}

		if (value > 0x10FFFF)
		{
			result = DX_INVALID_FORMAT;
			break;
		}
		if (value < 0x10000)
		{
			aUtf16[currPos++] = DX_CONVERT_TO_16_BIT((DxUint16)value); //aUtf16[currPos++] = ((DxUint16)value << 8);
		}
		else {
			value -= 0x10000;
			if (currPos + 2 > aUtf16BuffSize)
			{
				result = DX_BUFFER_IS_NOT_BIG_ENOUGH;
				break;
			}
			aUtf16[currPos++] = (DxUint16)(0xD800 | ((value >> 10) & 0x3FF));
			aUtf16[currPos++] = (DxUint16)(0xDC00 | (value & 0x3FF));
		}
	}
	aUtf16[currPos] = 0;

	if (result == DX_SUCCESS && *aUtf8 != 0)
		result = DX_BUFFER_IS_NOT_BIG_ENOUGH;

    if (result != DX_SUCCESS)
	RETURN_OLD_ERROR(result);
    DX_RETURN(DX_SUCCESS);
}

DxStatus DX_VOS_Utf16ToUtf32(DxWideChar32 *aUtf32, DxUint aUtf32BuffSize, const DxWideChar *aUtf16)
{
	DX_DECLARE(DxStatus, result, DX_SUCCESS);
	DxUint currPos = 0;
	DxWideChar32 planeType;

    if (aUtf16 == DX_NULL)
    {
        if (aUtf32 != DX_NULL && aUtf32BuffSize >= sizeof(DxWideChar))
            *aUtf32= '\0';
        DX_RETURN(DX_SUCCESS);
    }

    DX_ASSERT_PARAM(aUtf32 != DX_NULL);

	aUtf32BuffSize /= sizeof(DxWideChar32);

	if (aUtf32BuffSize == 0)
		RETURN_NEW_ERROR(DX_BUFFER_IS_NOT_BIG_ENOUGH);

	aUtf32BuffSize--; /* Saving Space for terminating NULL */
#ifndef DX_SUPPORT_SUPPLEMENTARY_PLANES
	planeType = 0x000000;
#else
	#ifdef DX_UNICODE_SMP
	planeType = 0x010000;
#else
    #ifdef DX_UNICODE_SIP
	planeType = 0x020000;
#else
	#ifdef DX_UNICODE_SSP
	planeType = 0x0E0000;
#else
	#ifdef DX_UNICODE_PUA
	planeType = 0x1F0000;
#else
	planeType = 0x00000;
#endif
#endif
#endif
#endif
#endif


	while (aUtf16[currPos] != '\0' && currPos < aUtf32BuffSize)
	{
		aUtf32[currPos] = (DxWideChar32)(aUtf16[currPos]) | planeType;
		++currPos;
	}

	aUtf32[currPos] = 0;

	if (result == DX_SUCCESS && *aUtf16 != '\0')
		result = DX_BUFFER_IS_NOT_BIG_ENOUGH;

	if (result != DX_SUCCESS)
		RETURN_OLD_ERROR(result);
	DX_RETURN(DX_SUCCESS);
}


DxStatus DX_VOS_Utf32ToUtf16(DxWideChar *aUtf16, DxUint aUtf16BuffSize, const DxWideChar32 *aUtf32)
{
	DX_DECLARE(DxStatus, result, DX_SUCCESS);
	DxUint currPos = 0;

    if (aUtf32 == DX_NULL)
    {
        if (aUtf16 != DX_NULL && aUtf16BuffSize >= sizeof(DxWideChar))
            *aUtf16 = '\0';
        DX_RETURN(DX_SUCCESS);
    }


    DX_ASSERT_PARAM(aUtf32 != DX_NULL);

	aUtf16BuffSize /= sizeof(DxWideChar);

	if (aUtf16BuffSize == 0)
		RETURN_NEW_ERROR(DX_BUFFER_IS_NOT_BIG_ENOUGH);

	aUtf16BuffSize--; /* Saving Space for terminating NULL */


	while (aUtf32[currPos] != '\0' && currPos < aUtf16BuffSize)
	{
		if (aUtf32[currPos] & 0xFFFF0000 )
			return DX_NOT_SUPPORTED;
		aUtf16[currPos] = (DxWideChar)(aUtf32[currPos]);
		++currPos;
	}

	aUtf16[currPos] = '\0';

	if (*aUtf32 != '\0')
		result = DX_BUFFER_IS_NOT_BIG_ENOUGH;

	DX_RETURN(DX_SUCCESS);
}


DxStatus DX_VOS_Utf8ToUtf32(DxWideChar32 *aUtf32, DxUint aUtf32BuffSize, const DxChar *aUtf8)
{
	DxWideChar* aUtf16;
	DxUint aUtf16BuffSize;
	DxStatus result;


	aUtf16BuffSize = aUtf32BuffSize;
	aUtf16 = (DxWideChar*)DX_VOS_MemMalloc(aUtf16BuffSize+sizeof(DxWideChar));
    RETURN_IF_ALLOC_FAILED(aUtf16);

	result = DX_VOS_Utf8ToUtf16(aUtf16,aUtf16BuffSize+sizeof(DxWideChar),aUtf8);
	if( DX_SUCCESS != result)
        GOTO_END_WITH_VAR_STATUS(result);

	result = DX_VOS_Utf16ToUtf32(aUtf32,aUtf32BuffSize+sizeof(DxWideChar32),aUtf16);
	if( DX_SUCCESS != result)
        GOTO_END_WITH_VAR_STATUS(result);

end:
    if (result != DX_SUCCESS)
        DX_VOS_MemFree(aUtf16);
    DX_RETURN(result);
}

DxStatus DX_VOS_Utf32ToUtf8(DxChar *aUtf8, DxUint aUtf8BuffSize, const DxWideChar32 *aUtf32)
{
	DxWideChar* aUtf16;
	DxUint aUtf16BuffSize;
	DxStatus result;

	aUtf16BuffSize = aUtf8BuffSize*2;
	aUtf16 = (DxWideChar*)DX_VOS_MemMalloc(aUtf16BuffSize+sizeof(DxWideChar));
    RETURN_IF_ALLOC_FAILED(aUtf16);

	result = DX_VOS_Utf32ToUtf16(aUtf16,aUtf16BuffSize+sizeof(DxWideChar),aUtf32);
	if( DX_SUCCESS != result)
        GOTO_END_WITH_VAR_STATUS(result);

	result = DX_VOS_Utf16ToUtf8(aUtf8,aUtf8BuffSize,aUtf16);
	if( DX_SUCCESS != result)
        GOTO_END_WITH_VAR_STATUS(result);
end:
	DX_VOS_MemFree(aUtf16);
	return DX_SUCCESS;
}


DxChar* DX_VOS_NextStringInList(const DxChar* StringList)
{
	DxUint Len = DX_VOS_StrLen(StringList);
	if (Len == 0)
		DX_RETURN(DX_NULL);
	return (DxChar*)StringList + Len + 1;
}

DxChar* DX_VOS_FindStringInList(const DxChar* StringList, const DxChar* SearchStr)
{
	const DxChar* CurrStr = StringList;
	while (CurrStr != DX_NULL)
	{
		if (DX_VOS_StrCmp(SearchStr, CurrStr) == 0)
			break;
		CurrStr = DX_VOS_NextStringInList(CurrStr);
	}
	return (DxChar*)CurrStr;
}

DxStatus DX_VOS_AddStringToList(const DxChar* string, DxChar** stringList, DxInt32* listBuffSize, DxUint32* numOfStrings)
{
	DX_DECLARE(DxStatus, result, DX_SUCCESS);
	DxInt32 Len = 0;

    DX_ASSERT_PARAM(string != DX_NULL);
    DX_ASSERT_PARAM(stringList != DX_NULL);

	if (*stringList != DX_NULL && listBuffSize == DX_NULL)
		RETURN_NEW_ERROR(DX_BAD_ARGUMENTS);

	if (numOfStrings != DX_NULL)
		(*numOfStrings)++;

	if (*stringList != DX_NULL)
	{
		Len = (DxInt32)DX_VOS_StrLen(string) + 1;
		*listBuffSize -= Len;
		if (*listBuffSize <= 0)
			RETURN_NEW_ERROR(DX_BUFFER_IS_NOT_BIG_ENOUGH);

		result = DX_VOS_MemCpy(*stringList, *listBuffSize + Len, string, Len);
		if (result != DX_SUCCESS)
			RETURN_OLD_ERROR(result);
		*stringList += Len;
		**stringList = 0;
	}
	DX_RETURN(DX_SUCCESS);
}

static DxStatus DX_VOS_HexCharToInt(DxChar hex, DxUint32* intVal)
{
    DX_ASSERT_PARAM(intVal != DX_NULL);
    if ((hex >= '0') && (hex <= '9'))
        *intVal = hex - '0';
    else if ((hex >= 'a') && (hex <= 'f'))
        *intVal = hex - 'a' + 10;
    else if((hex >= 'A') && (hex <= 'F'))
        *intVal = hex - 'A' + 10;
    else
        RETURN_NEW_ERROR(DX_INVALID_FORMAT);

    DX_RETURN(DX_SUCCESS);
}

/*DxStatus DX_VOS_DecodeNHex(const DxChar* string, DxUint32 stringMaxSize, void* buffer, DxUint32 bufferSize, DxUint32* lengthWritten, DxUint32* charsProcessed, const DxChar* whiteSpaces)
{
    DX_DECLARE(DxStatus, result, DX_SUCCESS);
    DxUint32 localBytesWritten = 0;
    DxUint8* writePtr = (DxUint8*) buffer;
    DxUint32 currPos = 0;
    DxBool  oddPos = DX_FALSE;
    DxUint32 localCharsProcessed = 0;
    DxUint32 val1 = 0;
    DxUint32 val2 = 0;

    DX_ASSERT_PARAM(string != DX_NULL);
    DX_ASSERT_PARAM(buffer != DX_NULL || bufferSize == 0);

    while(string[currPos] != 0 && currPos < stringMaxSize && localBytesWritten < bufferSize)
    {

        while (DX_VOS_StrChr(whiteSpaces, string[currPos]) != DX_NULL && currPos < stringMaxSize)
            currPos++;

        if (string[currPos] == 0 || currPos >= stringMaxSize)
            break;

        if (!oddPos)
        {
            result = DX_VOS_HexCharToInt(string[currPos++], &val1);
            if (result != DX_SUCCESS)
                break;
        } else {
            result = DX_VOS_HexCharToInt(string[currPos++], &val2);
            if (result != DX_SUCCESS)
                break;
            writePtr[localBytesWritten++] = (DxUint8)((val1 << 4) + val2);
            localCharsProcessed = currPos;
        }
        oddPos = !oddPos;
    }

    if (charsProcessed != DX_NULL)
        *charsProcessed = localCharsProcessed;
	else if (oddPos)
        RETURN_NEW_ERROR(DX_INVALID_FORMAT);

    if (lengthWritten != DX_NULL)
        *lengthWritten = localBytesWritten;
    else if (bufferSize != localBytesWritten)
        RETURN_NEW_ERROR(DX_BUFFER_IS_NOT_BIG_ENOUGH);

    if (lengthWritten != DX_NULL)
        *lengthWritten = localBytesWritten;
    else if (bufferSize != localBytesWritten)
        RETURN_NEW_ERROR(DX_WRONG_BUFFER_SIZE);

    if (result == DX_SUCCESS && string[currPos] != 0 && currPos < stringMaxSize)
        RETURN_NEW_ERROR(DX_BUFFER_IS_NOT_BIG_ENOUGH);

    if (result != DX_SUCCESS)
        RETURN_OLD_ERROR(result);

    DX_RETURN(DX_SUCCESS);
} */

static void DX_VOS_IntToHexChars(DxUint8 intVal, DxChar* hexChar1, DxChar* hexChar2)
{
    const DxChar* hexDigits = "0123456789abcdef";
    *hexChar1 = hexDigits[intVal >> 4];
    *hexChar2 = hexDigits[intVal & 0xF];
}
/*DxStatus DX_VOS_EncodeHex(const void* buffer, DxUint32 bufferSize, DxChar* string, DxUint32 stringSize)
{
    const DxUint8* readPtr = (const DxUint8*)buffer;
    DX_ASSERT_PARAM(string != DX_NULL);
    DX_ASSERT_PARAM(buffer != DX_NULL);
    DX_ASSERT_PARAM(stringSize > 0);

    stringSize--;

    while(stringSize >= 2 && bufferSize > 0)
    {
        DX_VOS_IntToHexChars(*readPtr++, string, string + 1);
        stringSize -= 2;
        bufferSize--;
        string += 2;
    }
    *string = 0;

    if (bufferSize > 0)
        RETURN_NEW_ERROR(DX_BUFFER_IS_NOT_BIG_ENOUGH);

    DX_RETURN(DX_SUCCESS);
}*/

#define DX_VOS_BASE_64_PAD 0xFF
/*static DxStatus Base64CharToInt(DxChar base64, DxUint8* intVal)
{
    DX_ASSERT_PARAM(intVal != DX_NULL);
    if ((base64 >= 'A') && (base64 <= 'Z'))
        *intVal = base64 - 'A' ;
    else if((base64 >= 'a')&&(base64 <= 'z'))
        *intVal = base64 - 'a' + 26;
    else if ((base64 >= '0')&&(base64 <= '9'))
        *intVal = base64 - '0' + 52;
    else if (base64 == '+')
        *intVal = 62;
    else if (base64 == '/')
        *intVal = 63;
    else if (base64 == '=')
        *intVal = DX_VOS_BASE_64_PAD;
    else
        RETURN_NEW_ERROR(DX_INVALID_FORMAT);

    DX_RETURN(DX_SUCCESS);
}*/

/*DxStatus DX_VOS_DecodeNBase64(const DxChar* string, DxUint32 stringMaxSize, void* buffer, DxUint32 bufferSize, DxUint32* lengthWritten, DxUint32* charsProcessed, const DxChar* whiteSpaces)
{
    DX_DECLARE(DxStatus, result, DX_SUCCESS);
    DxUint32 bitOffset =0;
    DxUint8* writePtr = (DxUint8*)buffer;
    DxUint8* bufferEnd = (DxUint8*)buffer + bufferSize;
    DxUint8 tempVal = 0;
    DxUint32 currPos = 0;
    DxUint32 localBytesWritten = 0;
    DxUint32 localCharsProcessed = 0;
    DX_ASSERT_PARAM(string != DX_NULL);
    DX_ASSERT_PARAM(buffer != DX_NULL);
    DX_ASSERT_PARAM(bufferSize > 0);

    *writePtr = 0;
    while (string[currPos] != 0 && currPos < stringMaxSize && writePtr  <  bufferEnd)
    {
        DxUint8 val;

        while (DX_VOS_StrChr(whiteSpaces, string[currPos]) != DX_NULL)
            currPos++;

        if (string[currPos] == 0  || currPos >= stringMaxSize)
            break;

        result = Base64CharToInt(string[currPos++], &val);
        if (result != DX_SUCCESS)
            break;


        if (val != DX_VOS_BASE_64_PAD)
        {
            if(bitOffset <= 2)
            {
                tempVal |= val << (2 - bitOffset);
                if (bitOffset == 2)
                {
                    *writePtr++ = tempVal;
                    tempVal = 0;
                }
            }
            else
            {
                *writePtr++ = (DxUint8)(tempVal | (val >> (bitOffset - 2)));
                tempVal = (DxUint8)(val << (10 - bitOffset));
            }
            bitOffset = (bitOffset + 6) & 0x7;
            if (bitOffset == 0)
            {
                localBytesWritten = (DxUint32)(writePtr - (DxUint8*)buffer);
                localCharsProcessed = currPos;
            }
        }
    }
    if (charsProcessed == DX_NULL)
        localBytesWritten = (DxUint32)(writePtr - (DxUint8*)buffer);
    else
        *charsProcessed = localCharsProcessed;
    if (lengthWritten != DX_NULL)
        *lengthWritten = localBytesWritten;
    else if (bufferSize != localBytesWritten)
        RETURN_NEW_ERROR(DX_BUFFER_IS_NOT_BIG_ENOUGH);


    if (result == DX_SUCCESS && string[currPos] != 0 && string[currPos] != '=' && currPos < stringMaxSize)
        RETURN_NEW_ERROR(DX_BUFFER_IS_NOT_BIG_ENOUGH);

    if (result != DX_SUCCESS)
	RETURN_OLD_ERROR(result);

    DX_RETURN(DX_SUCCESS);
}*/

static DxChar IntToBase64Char(DxUint8 intVal)
{
    const DxChar* base64Digits = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    return base64Digits[intVal & 0x3F];
}
/*DxStatus DX_VOS_EncodeBase64(const void* buffer, DxUint32 bufferSize, DxChar* string, DxUint32 stringSize)
{
    DxUint32 bitOffset = 0;

    const DxUint8* readPtr = (const DxUint8*)buffer;
    const DxUint8* bufferEnd = (const DxUint8*)buffer + bufferSize;

    DxChar* writePtr = string;
    DxChar* stringEnd = string + stringSize - 1;

    DX_ASSERT_PARAM(string != DX_NULL);
    DX_ASSERT_PARAM(buffer != DX_NULL);
    DX_ASSERT_PARAM(stringSize > 0);

    stringSize--;
    while(readPtr < bufferEnd && writePtr < stringEnd)
    {
        DxUint8 tempVal = 0;
        switch (bitOffset)
        {
            case 0:
                *writePtr++ = IntToBase64Char(*readPtr >> 2);                 // take upper 6 bits
                break;
            case 6:
                tempVal = *readPtr++ << 4;
                if (readPtr < bufferEnd)
                    tempVal |= *readPtr >> 4;
                *writePtr++ = IntToBase64Char(tempVal);
                break;
            case 4:
                tempVal = *readPtr++ << 2;
                if (readPtr < bufferEnd)
                    tempVal |= *readPtr >> 6;
                *writePtr++ = IntToBase64Char(tempVal);
                break;
            case 2:
                *writePtr++ = IntToBase64Char(*readPtr++);
                break;
            default:
                RETURN_NEW_ERROR(DX_ERROR); // we should never reach this code.
        }
        bitOffset = (bitOffset + 6) & 0x7;
    }
    while (bitOffset > 0 && writePtr < stringEnd)
    {
        *writePtr++ = '=';
        bitOffset = (bitOffset + 6) & 0x7;
    }
    *writePtr = 0;

    if ((readPtr < bufferEnd) || (bitOffset != 0))
        RETURN_NEW_ERROR(DX_BUFFER_IS_NOT_BIG_ENOUGH);

    DX_RETURN(DX_SUCCESS);
}*/

#ifdef DX_USE_INSECURE_LEGACY_VOS
DxInt DX_VOS_AtoI( const DxChar *aString )
{
    DxInt32 value = 0;
    DX_VOS_StrToInt(aString, &value, 10);
    DX_RETURN(value);
}

/*DxUint DX_VOS_AtoUI( const DxChar *aString )
{
    DxUint32 value = 0;
    DX_VOS_NStrToUnsigned(aString, DX_MAX_UINT32, &value, 16);
    DX_RETURN(value);
}*/
#endif
