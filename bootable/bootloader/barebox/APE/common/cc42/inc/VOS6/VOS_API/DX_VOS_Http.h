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
 
 #ifndef _DX_VOS_HTTP_H
#define _DX_VOS_HTTP_H


#ifdef __cplusplus
extern "C"
{
#endif

#include "DX_VOS_BaseTypes.h"

typedef enum {
    DX_HTTP_GET,
    DX_HTTP_POST,
} DxHttpMethod;

typedef struct _DxVosHttp* DxVosHttp;

/*! Issues a HTTP request of the specified httpMethod (GET/POST) to the specified url. 
    The function adds the first header line (HTTP...), the Host line and Content-Length.
    The function append to the header the content of additionalHeaderLines (if not NULL).
    additionalHeaderLines may contains a sequence of header lines. Every line is terminated by CRLF.
    The function sends the content of requestContent as the request body (if not NULL).
    The function set httpHandlePtr to point to the new request's handle. This handle may be passed
    later to DX_VOS_HttpReadResponseHeader(), DX_VOS_HttpReadResponseData() & DX_VOS_HttpClose().
    The function may return:
    - DX_SUCCESS - On Success.
    - DX_BAD_ARGUMENTS - if one of the arguments is invalid.
    - DX_MEM_ALLOCATION_ERROR - is memory allocation failed.
    - Any other error code if some other error occurred.
*/
DxStatus DX_VOS_HttpSendRequest(DxVosHttp* httpHandlePtr, DxHttpMethod httpMethod, const DxChar* url, const DxChar* requestContent, const DxChar* additionalHeaderLines);

/*! The function reads the response header of a request whose handle was received from previous call
    to DX_VOS_HttpSendRequest(). The data that is read into the outBuffer is always NULL terminated.
    Every line of the HTTP header is '\n' terminated including the last line of the HTTP header (which is an empty line).
    If not all the header is available within the specified timeout the function should return the available data
    when timeout expired with DX_TIMEOUT_EXPIRED as return value. if outBuffer cannot hold the entire HTTP header
    DX_BUFFER_IS_NOT_BIG_ENOUGH is returned. In this case on exit outBuffer will contain buffSizeInBytes-1 
    characters and the terminating NULL. A subsequent call to DX_VOS_HttpReadResponseHeader() will fill outBuffer 
    with the rest of the HTTP header.
    The function may return:
    - DX_SUCCESS - If the header data was read successfully and we reached end of header.
    - DX_BAD_ARGUMENTS - if one of the arguments is invalid.
    - DX_MEM_ALLOCATION_ERROR - is memory allocation failed.
    - DX_TIMEOUT_EXPIRED - If not all the header could be read within the specified timeout.
    - DX_BUFFER_IS_NOT_BIG_ENOUGH - if the header data is larger than the size of outBuffer.
    - Any other error code if some other error occurred.
*/
DxStatus DX_VOS_HttpReadResponseHeader(DxVosHttp httpHandle, DxChar* outBuffer, DxUint32 buffSizeInBytes, DxUint32 timeout);

/*! The function reads the response body of a request whose handle was received from previous call
    to DX_VOS_HttpSendRequest(). This function should be called only after a call to 
    DX_VOS_HttpReadResponseHeader() returned DX_SUCCESS. On exit bytesRead should hold the number of bytes 
    that were actually read. If not all the requested bytes is available within the specified timeout the 
    function should return the available data when timeout expired with DX_TIMEOUT_EXPIRED as return value.
    if outBuffer cannot hold the entire HTTP body DX_BUFFER_IS_NOT_BIG_ENOUGH is returned. In this case on exit 
    outBuffer will contain buffSizeInBytes characters. A subsequent call to DX_VOS_HttpReadResponseData() will 
    fill outBuffer with the rest of the HTTP response body.

    The function may return:
    - DX_SUCCESS - If data was read successfully and connection was closed gracefully by the server.
    - DX_BAD_ARGUMENTS - if one of the arguments is invalid.
    - DX_MEM_ALLOCATION_ERROR - is memory allocation failed.
    - DX_TIMEOUT_EXPIRED - If not all the response body could be read within the specified timeout.
    - DX_BUFFER_IS_NOT_BIG_ENOUGH - if the response body is larger than the size of outBuffer.
    - Any other error code if some other error occurred.
*/
DxStatus DX_VOS_HttpReadResponseData(DxVosHttp httpHandle, void* outBuffer, DxUint32 buffSizeInBytes, DxUint32* bytesRead, DxUint32 timeout);

/* Closes the httpHandle and releases all occupied resources. */
void DX_VOS_HttpClose(DxVosHttp httpHandle);

#ifdef __cplusplus
}
#endif

#endif
