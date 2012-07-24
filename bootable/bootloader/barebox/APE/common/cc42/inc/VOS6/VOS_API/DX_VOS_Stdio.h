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
 
 #ifndef _DX_VOS_STDIO_H
#define _DX_VOS_STDIO_H

/*! \file DX_VOS_stdio.h
\brief This module enables standard IO operations 
*/

#include "DX_VOS_File.h"

#ifdef __cplusplus
extern "C"
{
#endif

	/* Prints a string to the standard output. */
	IMPORT_C void  DX_VOS_PrintString(const DxChar *aStr);

	/*! Print formatted output to the standard output stream. */
	IMPORT_C void  DX_VOS_Printf(const DxChar *format,...);

	/*! Print formatted output to the standard output stream. */
	IMPORT_C void  DX_VOS_VPrintf (const DxChar *format, DX_VA_LIST ap);

	/*! Writes formatted data to a string.
	\return 
	- DX_SUCCESS - on success
	- DX_BUFFER_IS_NOT_BIG_ENOUGH if not all the output could be written to the storage location.
	in this case only the data that can be fit in will be written 
	*/
	IMPORT_C DxStatus DX_VOS_SPrintf(
		DxChar *aBuf,			/*!< [out] Pointer to storage location for output */
		DxUint aBuffSize,		/*!< [in] size in bytes of storage location */
		const DxChar *format,	/*!< [in] format string. Supports: %d, %u, %l, %c, %ul, %x, %X, %s */
		...);

	/*! Writes formatted data to a string. See DX_VOS_SPrintf() for details. */
	IMPORT_C DxStatus DX_VOS_VSPrintf(
		DxChar *aBuf,		
		DxUint aBuffSize,	
		const DxChar *format,	
		DX_VA_LIST arg_list
		);

	//! Inserts string arguments according to the format string. Limited to 9 arguments.
	//! \param[out] aBuff               Empty buffer that will be filled with retrieved data.
	//! \param[in]  aBuffSize           Size in bytes of buffer.
	//! \param[in]  format				A format string. the string should contains %# for each string argument
	//!									(e.g. "Second arg: %2, first arg: %1").
	//! \param[in]  numOfArgs           Number of arguments that are passed in the list. There is a limit of 9 arguments.
	//! \param[in]  ...					List of arguments to be embedded in the string.
	//! \return
	//! - DX_SUCCESS - On Success
	//! - DX_BUFFER_IS_NOT_BIG_ENOUGH - The size of aBuff is not big enough for the formatted string.
	//! - DX_INVALID_FORMAT - The format string is not valid.
	//! - DX_BAD_ARGUMENTS - aBuff or format parameters are passed as NULL.
	IMPORT_C DxStatus DX_VOS_EmbedStringArgs(DxChar *aBuff, DxUint aBuffSize, const DxChar *format, DxUint32 numOfArgs, ...);

#ifndef DX_NO_FILE_UTILS
	/*! Writes formatted data to the file. 
	\return 
	- DX_SUCCESS - on success
	- DX_VOS_FILE_ERROR - if there were some problems writing the data to the disk.
	*/
	IMPORT_C DxStatus DX_VOS_FPrintf(
		DxVosFile aFile,  /*!< [in] Pointer to output file. */
		const DxChar *format,	/*!< [in] format string. */
		...);

	/*! Writes formatted data to a file. See DX_VOS_FPrintf() for details. */
	IMPORT_C DxStatus DX_VOS_VFPrintf(
		DxVosFile aFile,  /*!< [in] Pointer to output file. */
		const DxChar *format,	/*!< [in] format string. */
		DX_VA_LIST arg_list);

#endif

	/*! Reads a "\r" or "\n" terminated string from the standard input */
	IMPORT_C DxStatus  DX_VOS_GetString(
		DxChar *aBuf,		/*!< [out] Pointer to buffer that will hold the string that is read */
		DxUint aBuffSize  /*!< [in]  Size in bytes of output buffer */ 
		);

	/*! Retrieves a character from the console without echo. 
	\return the character that was read. */
	IMPORT_C DxInt DX_VOS_GetCh(void);

	/*! Redirects the standard output (stdout) to the specified file. */
	IMPORT_C DxStatus DX_VOS_RedirectStdoutToFile(const DxChar* fileName);

	/*! Cancels a redirection previously done by DX_VOS_RedirectStdoutToFile() */
	IMPORT_C void DX_VOS_CancelStdOutRedirection(void);

	/*! Flushes output to stdout */
	IMPORT_C DxStatus DX_VOS_UpdateStdout(void);


#ifdef DX_USE_LEGACY_VOS
#define DX_VOS_Fprintf	DX_VOS_FPrintf
#define DX_VOS_Getch DX_VOS_GetCh
	IMPORT_C void DX_VOS_ScanfInt(int* aInt_ptr); 
#endif
#ifdef DX_USE_INSECURE_LEGACY_VOS
	IMPORT_C DxUint32  DX_VOS_Sprintf(DxInt8 *aBuf_ptr,const DxInt8 *aStr_ptr,...);
	IMPORT_C DxUint32  DX_VOS_FScanfString(DxVosFileHandle_ptr aFile_ptr,DxInt8 *aStr_ptr);
#define DX_VOS_ScanfString(aString_ptr) DX_VOS_GetString(aString_ptr, DX_MAX_UINT) 

#endif

#ifdef  __cplusplus
}
#endif


#endif /* ifndef _DX_VOS_STDIO_H */

