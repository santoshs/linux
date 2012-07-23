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

#include "DX_VOS_String.h"
#include "DX_VOS_Mem.h"


/*DxStatus DX_VOS_StrNCopy(DxChar *aDest, DxUint aTargetSize, const DxChar *aSrc)
{
	DxUint i;
	if ((aDest == (DxChar*) DX_NULL) || (aSrc == (DxChar*) DX_NULL))
		RETURN_NEW_ERROR(DX_BAD_ARGUMENTS);

    if (aTargetSize == 0)
        RETURN_NEW_ERROR(DX_BUFFER_IS_NOT_BIG_ENOUGH);
*/
  //  --aTargetSize; /* Saving room for the terminating NULL */

    // If aSrc comes before aDest there is a risk that the copying will overwrite the source
    // before it is fully copied. Because we don't want to waste time on checking what is the
    // real length of aSrc we take the worst assumption that is size equals to aTargetSize.
    // Example:
    // ABCDEFGHIJ
    // ^    ^
    // src  dest    aTargetSize = 20
/*	if ((DxUint)(aDest - aSrc) < aTargetSize)
    {
        // In this case we do some more expensive checking to find out what is the real length of aSrc
        DxUint srcLen = DX_VOS_StrNLen(aSrc, aTargetSize);
        // If the problem is real we do a copy from the end.
        // Example for no real problem:
        // ABC
        // ^    ^
        // src  dest    aTargetSize = 20
        if ((DxUint)(aDest - aSrc) < srcLen)
        {
            DxUint toCopy = srcLen;
            for (i = toCopy; i > 0 ; )
            {
                --i;
                aDest[i] = aSrc[i];
            }
            aDest[toCopy] = 0;
            DX_RETURN(DX_SUCCESS);
        }
    }

	for (i = 0; i < aTargetSize && *aSrc != 0 ; ++i)
		*aDest++ = *aSrc++;

	*aDest = 0;

	if (*aSrc!=0)
		RETURN_NEW_ERROR(DX_BUFFER_IS_NOT_BIG_ENOUGH);

	DX_RETURN(DX_SUCCESS);
}*/

DxUint DX_VOS_StrNLen(const DxChar *aStr, DxUint aMaxSize)
{
	DxUint i = 0;
	if ( aStr == DX_NULL)
		return 0;

	while (*aStr++ != 0 && i < aMaxSize)
		i++;
	DX_RETURN(i);
}

/*DxUint DX_VOS_WideStrNLen(const DxWideChar *aStr, DxUint aMaxSize)
{
    DxUint i = 0;
    if ( aStr == DX_NULL)
        return 0;

    while (*aStr++ != 0 && i < aMaxSize)
        i++;
    DX_RETURN(i);
}*/


DxInt DX_VOS_StrNCmp(const DxChar* Str1, const DxChar* Str2, DxUint aMaxSize )
{
	if (Str1 == DX_NULL)
	{
		if (Str2 == DX_NULL)
			return 0;
		return -1;
	}
	if (Str2 == DX_NULL)
		return 1;

	while (*Str1 != 0 && *Str2 != 0 && *Str1 == *Str2 && aMaxSize > 0)
	{
		Str1++;
		Str2++;
        aMaxSize--;
	}

	if (aMaxSize == 0 || *Str1 == *Str2)
		return 0;

	if (*Str1 < *Str2)
		return -1;

	return 1;
}

/*static DxChar LowerCase(DxChar input)
{
    if (input >= 'A' && input<='Z')
        input = input - 'A' + 'a';
    return input;
}*/

/*DxInt DX_VOS_StrNCmpIgnoreCase(const DxChar* Str1, const DxChar* Str2, DxUint aMaxSize )
{
    if (Str1 == DX_NULL)
    {
        if (Str2 == DX_NULL)
            return 0;
        return -1;
    }
    if (Str2 == DX_NULL)
        return 1;

    while (*Str1 != 0 && *Str2 != 0 && (LowerCase(*Str1) == LowerCase(*Str2)) && aMaxSize > 0)
    {
        Str1++;
        Str2++;
        aMaxSize--;
    }

    if (aMaxSize == 0 || LowerCase(*Str1) == LowerCase(*Str2))
        return 0;

    if (LowerCase(*Str1) < LowerCase(*Str2))
        return -1;

    return 1;
}*/

/*DxStatus DX_VOS_StrNCat(DxChar* aTarget, DxUint aMaxSize, const DxChar* aSource )
{
	DxUint Len = 0;
	if ((aTarget == (DxChar*) DX_NULL) || (aSource == (DxChar*) DX_NULL))
		RETURN_NEW_ERROR(DX_BAD_ARGUMENTS);

	if (aMaxSize == 0)
		RETURN_NEW_ERROR(DX_BUFFER_IS_NOT_BIG_ENOUGH);
*/ //	aMaxSize--; /* Saving room for terminating NULL */
/*	Len = DX_VOS_StrNLen(aTarget,aMaxSize);
	aTarget += Len;

	for (;Len < aMaxSize && *aSource != 0; Len++)
		*aTarget++ = *aSource++;

	*aTarget = 0;
	if (*aSource != 0)
		RETURN_NEW_ERROR(DX_BUFFER_IS_NOT_BIG_ENOUGH);

	DX_RETURN(DX_SUCCESS);
}*/

DxChar* DX_VOS_StrChr(const DxChar* Str, DxChar Ch)
{
    return DX_VOS_StrNChr(Str, DX_MAX_UINT32, Ch);
}

DxChar* DX_VOS_StrNChr(const DxChar* Str, DxUint32 strLen, DxChar Ch)
{
    DxUint32 i = 0;
    if (Str == DX_NULL)
        DX_RETURN(DX_NULL);

    for (i = 0; i < strLen && Str[i] != 0; i++)
    {
        if (Str[i] == Ch)
            return (DxChar*)(Str + i);
    }

    DX_RETURN(DX_NULL);
}

/*DxChar* DX_VOS_StrRChr(const DxChar* Str, DxChar Ch)
{
	const DxChar* LastPos = DX_NULL;
	if (Str == DX_NULL)
		DX_RETURN(DX_NULL);
	while (*Str != 0)
	{
		if (*Str == Ch)
			LastPos = Str;
		Str++;
	}

	return (DxChar*)LastPos;
}*/

void DX_VOS_StrToLower(DxChar* Str, DxUint32 maxSize)
{
	if (Str == DX_NULL)
		return;
	while (*Str != 0 && maxSize-- > 0)
	{
		if (*Str >= 'A' && *Str<='Z')
			*Str = *Str - 'A' + 'a';
		Str++;
	}
}

/*void DX_VOS_StrToUpper(DxChar* Str, DxUint32 maxSize)
{
	if (Str == DX_NULL)
		return;
	while (*Str != 0 && maxSize-- > 0)
	{
		if (*Str >= 'a' && *Str<='z')
			*Str = *Str - 'a' + 'A';
		Str++;
	}
}*/

/*
DxChar* DX_VOS_FindStr(const DxChar* searchIn, const DxChar* searchFor)
{
	if (searchFor == DX_NULL || searchIn == DX_NULL)
		DX_RETURN(DX_NULL);
	while (*searchIn != 0)
	{
		const DxChar* str1 = searchIn;
		const DxChar* str2 = searchFor;
		while (*str1 == *str2 && *str1 != 0)
		{
			str1++;
			str2++;
		}
		if (*str2 == 0)
			return (DxChar*)searchIn;
		if (*str1 == 0)
			break;
		searchIn++;
	}
	DX_RETURN(DX_NULL);
}*/


/*DxChar* DX_VOS_FindStrEx(const DxChar* searchIn, const DxChar* searchFor, DxUint32* foundLen)
{
	if (searchFor == DX_NULL || searchIn == DX_NULL)
		DX_RETURN(DX_NULL);
	while (*searchIn != 0)
	{
		const DxChar* str1 = searchIn;
		const DxChar* str2 = searchFor;
		DxUint32 matchLen = 0;
		while (*str1 == *str2 && *str1 != 0)
		{
			str1++;
			str2++;
			matchLen++;
		}
		if ((*str2 == 0) || ((*str1 == 0) && (foundLen != DX_NULL)))
		{
			if (foundLen != DX_NULL)
				*foundLen = matchLen;
			return (DxChar*)searchIn;
		}
		if (*str1 == 0)
			break;
		searchIn++;
	}
	DX_RETURN(DX_NULL);
}*/

/*DxStatus DX_VOS_MakeCopyOfStr(DxChar** destStr, const DxChar* srcStr)
{
    return DX_VOS_MakeCopyOfNStr(destStr, srcStr, DX_MAX_UINT32);
}*/

/*DxStatus DX_VOS_MakeCopyOfNStr(DxChar** destStr, const DxChar* srcStr, DxUint32 maxSrcSize)
{
    DxUint32 strSize = 0;

	DX_ASSERT_PARAM(destStr != DX_NULL);

	if (*destStr != DX_NULL)
		DX_DBG_PRINT0(DX_DBGPRINT_WARNING_LEVEL, "Possible memory leak. destStr points to an address which is not null.");

    if (srcStr == DX_NULL)
    {
        *destStr = DX_NULL;
        DX_RETURN(DX_SUCCESS);
    }

    strSize = DX_VOS_StrNLen(srcStr, maxSrcSize);

    *destStr = (DxChar*) DX_VOS_MemMalloc(strSize + 1);
    RETURN_IF_ALLOC_FAILED(*destStr);

    DX_VOS_FastMemCpy(*destStr, srcStr, strSize);
    (*destStr)[strSize] = 0;

    DX_RETURN(DX_SUCCESS);
}*/

/*DxStatus DX_VOS_ReplaceAll(DxChar* searchIn, const DxChar* searchFor, const DxChar* replaceWith)
{
    DxChar* currPos = DX_VOS_FindStr(searchIn, searchFor);
    DxUint32 newSize = DX_VOS_StrLen(replaceWith);
    DxChar* endPos = searchIn + DX_VOS_StrLen(searchIn) + 1;
    DxInt32 delta = DX_VOS_StrLen(searchFor) - newSize;
    //DX_VERIFY_PARAM(delta >= 0);
    if (delta < 0)
        RETURN_NEW_ERROR(DX_BUFFER_IS_NOT_BIG_ENOUGH);

    while (currPos != DX_NULL)
    {
        DX_VOS_FastMemCpy(currPos, replaceWith, newSize);
        currPos += newSize;
        if (delta > 0)
            DX_VOS_FastMemCpy(currPos, currPos + delta, endPos - currPos - delta);

        currPos = DX_VOS_FindStr(currPos, searchFor);
    }

    DX_RETURN(DX_SUCCESS);
}

DxUint32 DX_VOS_SplitStr(DxChar* originalString, DxChar delimiter, DxChar** splitArray, DxUint32 maxNumberOfSegments)
{
    DxUint32 count = 0;
    DxChar* currentPtr = originalString;
    DxChar* endOfSection = DX_NULL;

    if (originalString == DX_NULL || maxNumberOfSegments == 0)
        return 0;

    if (splitArray != DX_NULL)
        splitArray[count] = currentPtr;
    count++;
    while(count < maxNumberOfSegments)
    {
        endOfSection = DX_VOS_StrChr(currentPtr,delimiter);

        if (endOfSection != DX_NULL)
        {
            *endOfSection = DX_NULL;
            currentPtr = endOfSection + 1;
            if (splitArray != DX_NULL)
                splitArray[count] = currentPtr;
            count++;
        }
        else
            break;
    }

    DX_RETURN(count);
}

DxChar* DX_VOS_Trim(DxChar* str)
{
    DxUint32 length = 0;

    if (str == DX_NULL)
	return DX_NULL;

    while (*str == ' ' || *str == '\t' || *str == '\r' || *str == '\n')
        str++;
    length = DX_VOS_StrLen(str);
    if (length == 0)
        return str;

    while (length-- > 0)
    {
        if (str[length] != ' ' && str[length] != '\t' && str[length] != '\r' && str[length] != '\n')
            break;
    }
    str[length + 1] = 0;
    return str;
}*/
