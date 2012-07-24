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
 
 #ifndef _DX_VOS_STRING_H
#define _DX_VOS_STRING_H

/*! \file DX_VOS_String.h
This module provide string manipulation utilities.
*/

#include "DX_VOS_BaseTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

	/*!	Copies string from aSrc to aTarget (aSize - 1 characters at most). 
	\return 
	- DX_SUCCESS - all string was copied including the terminating NULL.
	- DX_BAD_ARGUMENTS - if one or more of the arguments are NULL.
	- DX_BUFFER_IS_NOT_BIG_ENOUGH - if not the whole string was copied.
		In this case only the characters that fit into the target buffer are copied.
	\note The aTarget will always be NULL terminated on return.
	**/
	DxStatus DX_VOS_StrNCopy(
		DxChar *aTarget,		/*!< [out]  Destination string*/
		DxUint aTargetSize,	/*!< [in]   aTarget buffer size*/
		const DxChar *aSource	/*!< [in]   Source string*/
		); 

	/*!	Calculates the length of a string.
	\return    Returns the number of characters in a string without counting the null termination character.
	if string is longer then aMaxSize - 1, aMaxSize will be returned.
	**/
	DxUint DX_VOS_StrNLen(
		const DxChar *aStr,		/*!< [in]  A pointer to an input string. if  NULL 0 will be returned. */
		DxUint aMaxSize		/*!< [in]  Max expected size of string */
		);

    DxUint DX_VOS_WideStrNLen(
        const DxWideChar *aStr,		/*!< [in]  A pointer to an input string. if  NULL 0 will be returned. */
        DxUint aMaxSize		/*!< [in]  Max expected size of string */
        );

	#define DX_VOS_StrLen(aStr) DX_VOS_StrNLen(aStr, DX_MAX_UINT)
    #define DX_VOS_WideStrLen(aStr) DX_VOS_WideStrNLen(aStr, DX_MAX_UINT)
    /*!	Compares strings (source with target)
	\return *
	- 0 - if strings are identical.
	- -1 - if first string is smaller then second string
	- 1 - if second string is smaller then second string
	\note DX_NULL is considered to be the smallest string.
	**/
	DxInt DX_VOS_StrNCmp(
		const DxChar* Str1,	/*!< [in] First string to compare */
		const DxChar* Str2,	/*!< [in] Second string to compare */
		DxUint aMaxSize	/*!< [in] Max number of characters to compare */
		);

	#define DX_VOS_StrCmp(Str1, Str2) DX_VOS_StrNCmp(Str1, Str2, DX_MAX_UINT)


    /*!	Compares strings (source with target) ignoring case
    \return *
    - 0 - if strings are identical.
    - -1 - if first string is smaller then second string
    - 1 - if second string is smaller then second string
    \note DX_NULL is considered to be the smallest string.
    **/
    DxInt DX_VOS_StrNCmpIgnoreCase(const DxChar* Str1, const DxChar* Str2, DxUint aMaxSize );

    #define DX_VOS_StrCmpIgnoreCase(Str1, Str2) DX_VOS_StrNCmpIgnoreCase(Str1, Str2, DX_MAX_UINT)

	/*!	Appends a string (source to target)
	\return 
	- DX_SUCCESS - On success.
	- DX_BAD_ARGUMENTS - if one or more of the arguments are NULL.
	- DX_BUFFER_IS_NOT_BIG_ENOUGH - if not the whole string was appended.
	\note The aTarget will always be NULL terminated on return.
	**/
	DxStatus DX_VOS_StrNCat(
		DxChar* aTarget,		/*!< [in, out] pointer to buffer that may contain a string */
		DxUint aTargetSize,	/*!< [in] Size of the target buffer(should be big enough to contain both strings */
		const DxChar* aSource	/*!< [in] pointer to string that should be appended to aTarget. */
		);

	/*!	Finds the first appearance of Ch in Str.
	\return 
	- DX_NULL - if Ch not found or if Str is DX_NULL.
	- otherwise - pointer to the first appearance of Ch in Str.
	**/
	DxChar* DX_VOS_StrChr(
		const DxChar* Str,	/*!< [in] String that should be searched */
		DxChar Ch			/*!< [in] Character that should be found */
		);

    DxChar* DX_VOS_StrNChr(
        const DxChar* Str,	/*!< [in] String that should be searched */
        DxUint32 strLen,
        DxChar Ch			/*!< [in] Character that should be found */
        );

	/*!	Finds the last appearance of Ch in Str.
	\return 
	- DX_NULL - if Ch not found or if Str is DX_NULL.
	- otherwise - pointer to the last appearance of Ch in Str.
	*/
	DxChar* DX_VOS_StrRChr(
		const DxChar* Str,	/*!< [in] String that should be searched */
		DxChar Ch			/*!< [in] Character that should be found */
		);


	/*! Converts the received string to all lowercase (in-place).
		If string is longer that maxSize characters only the first 
		maxSize characters will be converted */
	void DX_VOS_StrToLower(DxChar* Str, DxUint32 maxSize);

	/*! Converts the received string to all uppercase (in-place).
		If string is longer that maxSize characters only the first 
		maxSize characters will be converted */
	void DX_VOS_StrToUpper(DxChar* Str, DxUint32 maxSize);

	/*! Finds the string specified in searchFor in the string specified in searchIn.
		\return
			Pointer to the first occurrence of searchFor in searchIn or DX_NULL if string
			was not found.
	*/
#define DX_VOS_FindStr(searchIn, searchFor) DX_VOS_FindStrEx(searchIn, searchFor, DX_NULL)

	/*! Finds the string (or the beginning of it) specified in searchFor in the string 
		specified in searchIn. If the beginning of searchFor string is found and the end 
		of the searchIn string has been reached, then the length of the match is placed
		in foundLen.
	\return
	Pointer to the first occurrence of searchFor in searchIn or DX_NULL if string
	was not found.
	*/
	DxChar* DX_VOS_FindStrEx(const DxChar* searchIn, const DxChar* searchFor, DxUint32* foundLen);

    DxStatus DX_VOS_ReplaceAll(DxChar* searchIn, const DxChar* searchFor, const DxChar* replaceWith);

    /*  Allocates new memory and copies srcStr to the newly allocated memory. 
        On exit *destStr points to the newly allocated memory.
        If allocation fails *destStr will be NULL and DX_MEM_ALLOCATION_ERROR will be returned.
        */
    DxStatus DX_VOS_MakeCopyOfNStr(DxChar** destStr, const DxChar* srcStr, DxUint32 maxSrcSize);

    DxStatus DX_VOS_MakeCopyOfStr(DxChar** destStr, const DxChar* srcStr);
	
	/* Splits the originalString according to the specified delimiter to at most maxNumberOfSegments.
       if splitArray is provided its size must be at least maxNumberOfSegments and it will be
	   filled with the pointers to the split segments. If splitArray is DX_NULL the originalString
	   can be treated as string list that can be iterated by DX_VOS_NextStringInList().
	   Note that the string list is not terminated by an empty string so you must use
	   the return value to know its length.
	   The function returns the number of split segments.
	   */
    DxUint32 DX_VOS_SplitStr(DxChar* originalString, DxChar delimiter, DxChar** splitArray, DxUint32 maxNumberOfSegments);

    /*! Advances the received pointer to the first char which is different than " " or tab.
    Also, removes trailing spaces and tabs. */
    DxChar* DX_VOS_Trim(DxChar* str);

#define DX_VOS_IsSpace(ch)  (DX_VOS_StrChr("\x09\x0A\x0B\x0C\x0D\x20", ch) != DX_NULL)
#define DX_VOS_IsDigit(ch)  (DX_VOS_StrChr("0123456789", ch) != DX_NULL)
#define DX_VOS_IsAlpha(ch)  (DX_VOS_StrChr("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", ch) != DX_NULL)

#ifdef DX_USE_LEGACY_VOS
#define DX_VOS_StrNCpy(target, source, len) DX_VOS_StrNCopy(target, len, source)
    #ifdef DX_USE_INSECURE_LEGACY_VOS
        #define DX_VOS_StrCopy(target, source) DX_VOS_StrNCopy(target, DX_MAX_UINT, source)
        #define DX_VOS_StrCat(target, source) DX_VOS_StrNCat(target, DX_MAX_UINT, source)
    #endif
#endif

#ifdef __cplusplus
}
#endif

#endif /*_DX_VOS_MEM_H*/
