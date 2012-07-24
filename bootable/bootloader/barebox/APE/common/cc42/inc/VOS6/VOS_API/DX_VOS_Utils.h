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
 
 

#ifndef _DX_VOS_UTILS_H
#define _DX_VOS_UTILS_H

/*! \file DX_VOS_Utils.h
    \brief This file defines the DX_VOS utils
    seed function.
*/

#include "DX_VOS_BaseTypes.h"


#ifdef __cplusplus
extern "C"
{
#endif
#define DX_VOS_NO_WHITESPACE    ""
#define DX_VOS_SINGLE_LINE_WHITESPACE " \t"
#define DX_VOS_MULTI_LINE_WHITESPACE " \t\r\n"
#define DX_HEX_BUFF_SIZE(binSize)       ((binSize) * 2 + 1)
#define DX_BASE64_BUFF_SIZE(binSize)    ((DX_DIVIDE_ROUND_UP(binSize, 3) * 4) + 1)

#define DX_VOS_HEX_BUFFER_SIZE(BinBufferSize) (2 * BinBufferSize + 1)
    /*! skip the preceding white spaces and return a pointer to the first non-whitespace character.
        The whiteSpaces parameter specified the characters that should be skipped (For Example: " \t")
    */
	DxChar* DX_VOS_SkipWhiteSpaces(const DxChar* aString, const DxChar* whiteSpaces);

/*! Convert strings to signed integer value.
	aString may contain '-' or '+' sign.
    The base parameter can be 0 or 2-16.
	If base is not 0 the value of base parameter will be used for conversion.
	If base is 0 the function will try to infer the base from the string using the
	following rules:
	- if it starts with 'b', binary (2) base will be used.
	- if it start with '0', octal (8) base will be used 
	- if it start with '0x', haxadecimal (16) base will be used 
	- in all other case decimal (10) base will be used 
	
	\return
	- DX_SUCCESS on success
	- DX_BAD_ARGUMENTS - is aString or value is NULL.
	- DX_INVALID_FORMAT - if string is not a number. In this case value holds the value that
		was parsed until the error was encountered.
	- DX_OVERFLOW - if string contains number that can not be represented in 32 bits.
*/
#define DX_VOS_StrToInt(aString, value, base) DX_VOS_NStrToInt(aString, DX_MAX_UINT32, value, base)

/*  Works the same way as DX_VOS_StrToInt() but if aString is longer then stringSize only the first
    stringSize characters will be taken into account. */
IMPORT_C DxStatus DX_VOS_NStrToInt(const DxChar *aString, DxUint32 stringSize, DxInt32* value, DxUint base);

/*! Convert strings to unsigned integer value 
    The base parameter can be 0 or 2-16.
    If base is not 0 the value of base parameter will be used for conversion.
	If base is 0 the function will try to infer the base from the string using the
	following rules:
	- if it starts with 'b', binary (2) base will be used.
	- if it start with '0', octal (8) base will be used 
	- if it start with '0x', haxadecimal (16) base will be used 
	- in all other case decimal (10) base will be used 
	\return
	- DX_SUCCESS on success
	- DX_BAD_ARGUMENTS - is aString or value is NULL.
	- DX_INVALID_FORMAT - if string is not a number. In this case value holds the value that
		was parsed until the error was encountered.
	- DX_OVERFLOW - if string contains number that can not be represented in 32 bits.
*/
#define DX_VOS_StrToUnsigned(aString,value,base) DX_VOS_NStrToUnsigned(aString, DX_MAX_UINT32, value, base)

/*  Works the same way as DX_VOS_StrToUnsigned() but if aString is longer then stringSize only the first
stringSize characters will be taken into account. */
IMPORT_C DxStatus DX_VOS_NStrToUnsigned(const DxChar *aString, DxUint32 stringSize, DxUint32* value, DxUint base);

/*! Converts the a signed integer to a decimal string representation.
	In case of negative value - sign will be prefixed.
	\return
	- DX_SUCCESS - On Success
	- DX_BAD_ARGUMENTS - if aBuff is NULL or Radix is out of range.
	- DX_BUFFER_IS_NOT_BIG_ENOUGH - if number cannot be fit into the supplied string.
 **/
IMPORT_C DxStatus DX_VOS_IntToStr(
	DxInt32 aValue,		/*!< [in]  Number to be converted. */
	DxChar *aBuff,			/*!< [out] Pointer to output buffer */ 
	DxUint aBuffSize,		/*!< [in] Buffer size in bytes. */
	DxInt aRadix			/*!< [in] Base for conversion. can be any number between 2 and 16 */
	);

/*! Converts unsigned integer to a decimal string representation.
	\return
	- DX_SUCCESS - On Success
	- DX_BAD_ARGUMENTS - if aBuff is NULL or Radix is out of range.
	- DX_BUFFER_IS_NOT_BIG_ENOUGH - if number cannot be fit into the supplied string.
**/

IMPORT_C DxStatus DX_VOS_UnsignedToStr(
	DxUint32 aValue,		/*!< [in]  Number to be converted. */
	DxChar *aBuff,			/*!< [out] Pointer to output buffer */ 
	DxUint aBuffSize,		/*!< [in] Buffer size in bytes. */
	DxInt aRadix			/*!< [in] Base for conversion. can be any number between 2 and 16 */
	);


/*! Converts UTF-16 encoding text into UTF-8 encoding.
	\return
	- DX_SUCCESS - On success
	- DX_BAD_ARGUMENTS - if one of the pointers is NULL.
	- DX_BUFFER_IS_NOT_BIG_ENOUGH - if not all data can be fit into the output buffer.
		In this case only the data that can be fit in will be converted.
	- DX_INVALID_FORMAT - if aUtf16 is not a valid UTF-16 string.
	\note In all cases (except when DX_BAD_ARGUMENTS is returned and when aUtf8BuffSize is 0) 
		aUtf8 will contains a valid UTF-8 string on exit.
*/
IMPORT_C DxStatus DX_VOS_Utf16ToUtf8(
	DxChar *aUtf8,			  /*!< [out] Pointer to UTF-8 encoded output buffer */
	DxUint aUtf8BuffSize,	  /*!< [in] Size of output buffer in bytes */
	const DxWideChar *aUtf16  /*!< [in] NULL Terminated UTF-16 string */
	);

/*! Converts UTF-8 encoding text into UTF-16 encoding.
	\return
	- DX_SUCCESS - On success
	- DX_BAD_ARGUMENTS - if one of the pointers is NULL.
	- DX_BUFFER_IS_NOT_BIG_ENOUGH - if not all data can be fit into the output buffer.
	In this case only the data that can be fit in will be converted.
	- DX_INVALID_FORMAT - if aUtf8 is not a valid UTF-8 string.
	\note In all cases (except when DX_BAD_ARGUMENTS is returned and when aUtf8BuffSize is 0) 
	aUtf8 will contains a valid UTF-16 string on exit.
*/
IMPORT_C DxStatus DX_VOS_Utf8ToUtf16(
	DxWideChar *aUtf16,		 /*!< [out] Pointer to UTF-16 encoded output buffer */
	DxUint aUtf16BuffSize, /*!< [in] Size of output buffer in bytes */
	const DxChar *aUtf8		 /*!< [in] NULL Terminated UTF-8 string */
	);

IMPORT_C DxStatus DX_VOS_Utf32ToUtf16(DxWideChar *aUtf16, DxUint aUtf16BuffSize, const DxWideChar32 *aUtf32);
IMPORT_C DxStatus DX_VOS_Utf16ToUtf32(DxWideChar32 *aUtf32, DxUint aUtf32BuffSize, const DxWideChar *aUtf16);
IMPORT_C DxStatus DX_VOS_Utf8ToUtf32(DxWideChar32 *aUtf32, DxUint aUtf32BuffSize, const DxChar *aUtf8);
IMPORT_C DxStatus DX_VOS_Utf32ToUtf8(DxChar *aUtf8, DxUint aUtf8BuffSize, const DxWideChar32 *aUtf32);

/*! Appends a string to a string list pointed by stringList, advances stringList pointer to the end of the list,
	decrements listBuffSize by the length of string and increments the value pointed by numOfStrings 
	if it is not NULL. If there is not enough space in the list the string is not added to the stringList
	buffer but numOfStrings is still incremented and listBuffSize is still decremented. This way
	the user know how much space is missing in the supplied buffer.
	\return
		- DX_SUCCESS - on success.
		- DX_BAD_ARGUMENTS - if one of the parameters is invalid.
		- DX_BUFFER_IS_NOT_BIG_ENOUGH - if stringList is not big enough to hold the string.
			In this case listBuffSize will become negative.
*/
IMPORT_C DxStatus DX_VOS_AddStringToList(
	const DxChar* string,	 /*! [in] string to be added to the list. Souldn't be NULL. */
	DxChar** stringList,	 /*! [in] pointer to the pointer to the end of existing list. Pointed value may be NULL. */
	DxInt32* listBuffSize, /*! [in] Size in bytes of the space left in the list's buffer.
									May be NULL only if stringList pointed value is also NULL. */
	DxUint32* numOfStrings /*! [in] Pointer to the number of string in the list. May be NULL. */
	);

/*! returns a pointer to the next string in the list. Empty String indicates list end.
	If StringList is NULL or if trying to go beyond end of string NULL is returned.
*/
IMPORT_C DxChar* DX_VOS_NextStringInList(const DxChar* StringList);

/*! Searches a specified string in a string list.
	returns a pointer to the string found or NULL if string was not found. */
IMPORT_C DxChar* DX_VOS_FindStringInList(const DxChar* StringList, const DxChar* SearchStr);

/*! Converts the given hex string into a binary representation in the given buffer.
    \return
    - DX_SUCCESS - on success.
    - DX_BAD_ARGUMENTS - if one of the parameters is invalid.
    - DX_BUFFER_IS_NOT_BIG_ENOUGH - if buffer is not of correct size to hold decoded data. 
	- NOTE: in case length written parameter is null DX_BUFFER_IS_NOT_BIG_ENOUGH will be returned if the output buffer size doesn't match the decoded input size (even in the case where the output buffer is too big). 
*/
IMPORT_C DxStatus DX_VOS_DecodeNHex(const DxChar* string, DxUint32 stringMaxSize, void* buffer, DxUint32 bufferSize, DxUint32* lengthWritten, DxUint32* charsProcessed, const DxChar* whiteSpaces);
#define DX_VOS_DecodeHex(string, buffer, bufferSize, lengthWritten, whiteSpaces) DX_VOS_DecodeNHex(string, DX_MAX_UINT32, buffer, bufferSize, lengthWritten, DX_NULL, whiteSpaces)
/*! Converts the given binary buffer into the a hex string. The string will always be NULL terminated.
    \return
    - DX_SUCCESS - on success.
    - DX_BAD_ARGUMENTS - if one of the parameters is invalid.
    - DX_BUFFER_IS_NOT_BIG_ENOUGH - if string is not big enough to hold the decoded data.
*/
IMPORT_C DxStatus DX_VOS_EncodeHex(const void* buffer, DxUint32 bufferSize, DxChar* string, DxUint32 stringSize);

/*! Converts the given base64 string into a binary representation in the given buffer.
\return
- DX_SUCCESS - on success.
- DX_BAD_ARGUMENTS - if one of the parameters is invalid.
- DX_BUFFER_IS_NOT_BIG_ENOUGH - if buffer is not big enough to hold the decoded data.
 */
IMPORT_C DxStatus DX_VOS_DecodeNBase64(const DxChar* string, DxUint32 stringMaxSize, void* buffer, DxUint32 bufferSize, DxUint32* lengthWritten, DxUint32* charsProcessed, const DxChar* whiteSpaces);
#define DX_VOS_DecodeBase64(string, buffer, bufferSize, lengthWritten, whiteSpaces) DX_VOS_DecodeNBase64(string, DX_MAX_UINT32, buffer, bufferSize, lengthWritten, DX_NULL, whiteSpaces)

/*! Converts the given binary buffer into the a base64 string.
\return
- DX_SUCCESS - on success.
- DX_BAD_ARGUMENTS - if one of the parameters is invalid.
- DX_BUFFER_IS_NOT_BIG_ENOUGH - if string is not big enough to hold the decoded data.
*/
IMPORT_C DxStatus DX_VOS_EncodeBase64(const void* buffer, DxUint32 bufferSize, DxChar* string, DxUint32 stringSize);

#ifdef DX_USE_INSECURE_LEGACY_VOS
DxInt DX_VOS_AtoI( const DxChar *aString);
DxUint DX_VOS_AtoUI( const DxChar *aString);

#define DX_VOS_ItoA(aValue, aString, aRadix) DX_VOS_IntToStr(aValue, aString, DX_MAX_UINT, aRadix)
#define DX_VOS_UItoA(aValue, aString, aRadix) DX_VOS_UnsignedToStr(aValue, aString, DX_MAX_UINT, aRadix)

#define DX_VOS_UnicodeToUtf8(aUnicode,aUtf8) DX_VOS_Utf16ToUtf8((DxChar*)aUtf8, DX_MAX_UINT, aUnicode)
#define DX_VOS_Utf8ToUnicode(aUtf8, aUnicode) DX_VOS_Utf8ToUtf16(aUnicode, DX_MAX_UINT, (DxChar*)aUtf8)
#endif

#ifdef  __cplusplus
}
#endif


#endif /* ifndef _DX_VOS_UTILS_H */









