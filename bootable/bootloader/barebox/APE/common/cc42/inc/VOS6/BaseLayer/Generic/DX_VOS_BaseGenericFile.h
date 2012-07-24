/***********************************************************************************  
 * Copyright 2009 © Discretix Technologies Ltd. 
 * This software is protected by copyright, international treaties and 
 * various patents. If the license governing the use of this Software 
 * allows copy or redistribution of this  software then any copy or 
 * reproduction of this Software must include this Copyright Notice 
 * as well as any other notices provided under such license. 
 ***********************************************************************************/
 
 
 #ifndef _DX_VOS_BASEFILE_H
#define _DX_VOS_BASEFILE_H

/*! \file DX_VOS_BaseFile.h
This module implements the basic operations of files.
Parameter check was already done so function of this modules may assume:
- Pointers are not NULL (unless specified otherwise).
- File Handle is not NULL.
- Enum parameters have valid values.
- output parameter are initialized and should not be changed if operation fails (unless specified otherwise).
- All const DxChar* parameters are UTF8 encoded.
- Buffers size is not 0.

All functions return DxStatus that should be DX_SUCCESS on success 
or DX_FILE_ERROR on failure (unless specified otherwise).
*/

#include "DX_VOS_File.h"


DxStatus DX_VOS_BaseGetOsHandleFromFileHandle(DxVosFile aFileHandle, void** osHandle);

/*! Opens the file specified by aFileName in the mode specified by Mode and returns the opened file handle in aFileHandle */
DxStatus DX_VOS_BaseFOpen(DxVosFile *aFileHandle, /*!< [out] handle to the opened file. */
							 const DxChar *aFileName,	/*!< [in] UTF8 Encoded File Name */
							 const DxChar *aMode		/*!< [in] UTF8 Encoded opening mode
																The mode syntax is:	<r|w|a>[+][b|t].

																Examples: "r", "r+", "w+t", "ab"*/							 
							 );

DxStatus DX_VOS_BaseCreateFileHandleFromOsHandle(DxVosFile* aFileHandle, void* osHandle);

/*! Opens the file specified by aFileName in the mode specified by openMode & shareMode and 
    returns the opened file handle in aFileHandle.
    Supporting the shareMode is optional.
    The function may assume that all parameters are legal.*/
DxStatus DX_VOS_BaseFileOpen(DxVosFile *aFileHandle, const DxChar *aFileName, EDxOpenMode openMode, EDxShareMode shareMode);

/*! Closes the file. If the handle points to an already closed file it is not considered as error.*/
DxStatus DX_VOS_BaseFileClose(DxVosFile aFileHandle);

/*! Reads data from a file */
DxStatus DX_VOS_BaseFileRead(DxVosFile aFileHandle, /*!< [in] Handle to the file */
							 void* aBuf,	/*!< [out] pointer to an empty buffer. Buffer may be changed even if function fails. */
							 DxUint32 aSize,  /*!< [in]  size of aBuf in bytes */
							 DxUint32* aReadBytesPtr /*!< [out] number of bytes actually read. */						 
							 );

/*! Write data to file */
DxStatus DX_VOS_BaseFileWrite(DxVosFile aFileHandle, /*!< [in] Handle to the file */
							  const void *aBuf, /*!< [in] pointer to data to be written */
							  DxUint32 aSize	/*!< [in]  size of aBuf in bytes */
							  );

/*! Changes the current position in the file */
DxStatus DX_VOS_BaseFileSeek(DxVosFile aFileHandle, /*!< [in] Handle to the file */
							 DxInt32 aOffset, /*!< [in] Offset in bytes from initial position*/
							 DxInt aOrigin /*[in] Initial position: may be one of the following:
											 - DX_SEEK_CUR  Current position of file pointer
											 - DX_SEEK_END  End of file
											 - DX_SEEK_SET  Beginning of file*/
							 );
/*! Fills aCurPosPtr with the current file position */
DxStatus DX_VOS_BaseFileTell(DxVosFile aFileHandle, DxUint32* aCurPosPtr);

/*! Tests for end-of-file.
	@return if current position is not end of file, returns DX_SUCCESS. 
			if current position is at end of file, returns DX_VOS_END_OF_FILE.
			if operation fails, returns DX_VOS_FILE_ERROR.
*/
DxStatus DX_VOS_BaseFileEof(DxVosFile aFileHandle);

/*! Saves all buffered data to disk.	*/
DxStatus DX_VOS_BaseFileFlush(DxVosFile aFileHandle);

/*! Deletes a file.	The function may assume that the file exists.*/
DxStatus DX_VOS_BaseFileDelete(const DxChar *aFileName);

/*! Renames a file.	*/
DxStatus DX_VOS_BaseFileRename(const DxChar *aOrigName, const DxChar *aNewName);

/*! Creates a directory. If parent directory doesn't exist the function fails.
	If the directory already exists the function succeeds.*/
DxStatus DX_VOS_BaseCreateDirectory(const DxChar *aDirName);

/*! Enumerates all files and directories that are in the specified directory 
\note is DirsBuff or FilesBuff is not big enough to contain all files DX_BUFFER_IS_NOT_BIG_ENOUGH
will be returned and DirBuffSize & FilesBuffSize will hold the required buffer sizes.
*/
DxStatus DX_VOS_BaseEnumerateDir(
	const DxChar *aDirName, /*!< [in] A pointer to the full path of the directory to be enumerated. */
	DxChar* DirsBuff, /*!< [out] pointer to buffer that will hold the list of sub-directories of aDirName.
					  This list will not contain "." and "..". The directories in the list are NULL separated.
					  The list is terminated by an empty string (two adjacent NULLs). You can iterate through
					  the strings in the list using DX_VOS_NextStringInList() & DX_VOS_FindStringInList().
					  Can be NULL if the list of sub-directories in not required */
	DxUint32* DirsBuffSize, /*!< [inout] on entry this parameter indicates the size in bytes of DirsBuff.
							  on return the parameter indicates the number of used bytes in DirsBuff.
							  If the value on return is larger then the value on entry, it means that the
							  supplied buffer was not big enough and the parameter's value indicates what
							  should be the buffer size. In this case DX_BUFFER_IS_NOT_BIG_ENOUGH will be returned.
							  Can be NULL only if DirsBuff is NULL */
	DxUint32* NumOfDirs, /*!< [out] number of sub-directories of aDirName.
						   Can be NULL if the number of sub-directories is not required */
	DxChar* FilesBuff, /*!< [out] pointer to buffer that will hold the list of files in aDirName.
					   behaves exactly as DirsBuff.*/
	DxUint32* FilesBuffSize, /*!< [inout] on entry this parameter indicates the size in bytes of FilesBuff.
							  on return the parameter indicates the number of used bytes in FilesBuff. 
							  see: DirsBuffSize for more details.*/
	DxUint32* NumOfFiles  /*!< [out] number of files of aDirName.
							  Can be NULL if the number of files is not required */
	);

/*! Deletes all the subdirectories and files of the specified directory.
	This is internal function. The aDirName is not const to allow
	manipulation. Function may assume that size of aDirName is
	DX_VOS_MAX_FILE_SIZE 
*/
DxStatus DX_VOS_BaseCleanDirectory(const DxChar *aDirName);

/*! Deletes an empty directory.
	Function may assume that directory has no sub directories and no files. 
    If the directory doesn't exist the function succeeds.*/
DxStatus DX_VOS_BaseRemoveDirectory(const DxChar *aDirName);

/*! returns true if file exist. */
DxBool DX_VOS_BaseIsFileExists(const DxChar* File);

/*! Copies a File. If Destination file exists it should be overwritten. */
DxStatus DX_VOS_BaseCopyFile (const DxChar *ExistingFileName, const DxChar *NewFileName);

/*! Writes to the log the error of the last operation */
void DX_VOS_BaseLogFileResult(void);

#endif
