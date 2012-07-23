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
 
 #ifndef _DX_VOS_SOCKET_H 
#define _DX_VOS_SOCKET_H 

/*! \file DX_VOS_Socket.h
This module enables socket operations.

The user of these functions may assume:
- Pointers should not be NULL (unless specified otherwise).
  A Buffer pointer parameter may be NULL only if its size is 0.
- Socket Handles should not be NULL (unless specified otherwise).
- Enum parameters should have valid values.
- The value of output parameter has no effect.
- All const DxChar* parameters should be UTF8 encoded.

All functions that returns DxStatus type returns:
- DX_SUCCESS - on success
- DX_BAD_ARGUMENTS - if one of the parameters is not valid (see rules mentioned earlier)
- DX_VOS_SOCKET_ERROR - if operation fails.
- DX_VOS_SOCKET_CLOSE - if socket closed gracefully during the operation.

if operation fails a string that describes the error reason can be obtained
by calling DX_VOS_SocketGetErrorCode().
*/

#include "DX_VOS_BaseTypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

	/*************************** Typedefs *********************/
    #define DX_VOS_MAX_URL_SIZE 256

	typedef struct _DxVosSocket*  DxVosSocket;

	typedef struct {
		DxUint8 m_Address[4];
		DxUint16 m_Port;
	} DxIpAddress;

	typedef enum {
		DX_AF_INET,
		DX_AF_UNIX,
		DX_NUMBER_OF_ADDRESS_FAMILIES
	} EDxAddressFamily;

	typedef enum {
		DX_SOCK_STREAM,
		DX_SOCK_DATAGRAM,
		DX_SOCK_RAW,
		DX_NUMBER_OF_SOCKET_TYPES
	} EDxSocketType;

	typedef enum {
		DX_IPPROTO_IP, 
		DX_IPPROTO_IPV6, 
		DX_IPPROTO_TCP,
		DX_IPPROTO_UDP,
		DX_SOL_SOCKET,
		DX_NUMBER_OF_PROTOCOLS
	} EDxProtocol;

	#define	DX_INVALID_SOCKET 0

	/*!	Creates a socket for communication.	*/
	DxStatus DX_VOS_SocketCreate(
		DxVosSocket *aSocket,		
		EDxAddressFamily addressFamily, 
		EDxSocketType aSockType,		
		EDxProtocol protocol
		);

	/*!	Listens for incoming connections on a local socket. */
	DxStatus DX_VOS_SocketListen(
		DxVosSocket aSocket,	/*!< [in] A socket descriptor */
		DxInt aBackLog		/*!< [in] Number of connections to queue */
		);


	/*!	Initiates a connection to the specified destination IP Address & port. 
		If DX_SUCCESS is returned the connection was established successfully.*/
	DxStatus DX_VOS_SocketConnect(
		DxVosSocket aSocket,			/*!< [in] A socket descriptor */
		const DxIpAddress* aIPAddress	/*!< [in] Destination IP address. */
		);

	/*!	Closes a socket. 
		if aSocket is DX_INVALID_SOCKET that function succeeds. */
	DxStatus DX_VOS_SocketClose (DxVosSocket* aSocket);


	/*! Waits until data is ready for read from the socket or until
		timeout expires.
		\return
		- DX_SUCCESS - if data is ready
		- DX_TIMEOUT_EXPIRED - if timeout expired.
		- DX_BAD_ARGUMENTS - if aSocket is DX_INVAlID_SOCKET.
		- DX_VOS_SOCKET_CLOSE - if connection was closed gracefully.
		- DX_VOS_SOCKET_ERROR - if connection was terminated.
	**/
	DxStatus DX_VOS_SocketWaitForData(
		DxVosSocket aSocket,	/*!< [in] socket handle */
		DxUint32 aTimeout		/*!< [in] timeout in milliseconds */
		);


	/*!	Receives available data from a connected socket. 
		Function returns immediately with available data.
		If no data is available 0 bytes will be returned. 
		If function fails aBufLen will indicate the number 
		of bytes read until the failure.*/
	DxStatus DX_VOS_SocketRecv (
		DxVosSocket aSocket, /*!< [in] socket handle */
		void *aBuf,			 /*!< [out] pointer to output buffer */
		DxUint32 aBufLen,	 /*!< [in] size in bytes of output buffer */
		DxUint32* bytesRead,/*!< [out] actual	number of bytes read. If this parameter is DX_NULL and not all
                                bytes were read DX_TIMEOUT_EXPIRED will be returned.*/
        DxUint32 timeout
		);

	/*!	Reads the specified amount of bytes. Waits until all the requested bytes
		are read or until timeout expired or error occurs. */
	DxStatus DX_VOS_SocketRead (
		DxVosSocket aSocket, /*!< [in] socket handle */
		void *aBuf,			 /*!< [out] pointer to output buffer */
		DxUint32 aBufLen,	 /*!< [in] size in bytes of output buffer */
		DxUint32* bytesRead, /*!< [out] number of bytes that were actually read. May be NULL */
		DxUint32 timeout	 /*!< [in] timeout in milliseconds */
		);


	/*!	reads messages from the indicated datagram socket into the buffer.
		if no message is available the function returns with 0 bytes.
	**/

	DxStatus DX_VOS_SocketRecvFrom (
		DxVosSocket aSocket, /*!< [in] socket handle */
		void *aBuf,			 /*!< [out] pointer to output buffer */
		DxUint32 aBufLen, /*!< [in] on entry size in bytes of output buffer */
        DxUint32* bytesRead,/*!< [out] actual	number of bytes read. If this parameter is DX_NULL and not all
                                 bytes were read DX_TIMEOUT_EXPIRED will be returned.*/
		DxIpAddress* aFrom,	 /*!< [out] IP address of sender. May be NULL */
        DxUint32 timeout
		);
	
	/*!	Transmits data to a previously established connection-based socket.
		Functions returns immediately with indication of how many bytes were
		actually written */
	DxStatus DX_VOS_SocketSend (
		DxVosSocket aSocket, /*!< [in] socket handle */
		const void *aBuf,	 /*!< [in] pointer to input buffer */
		DxUint32 aBufLen,	 /*!< [in] on entry number of bytes to write. */
        DxUint32* bytesWritten /*!< [out] actual	number of bytes written. If this parameter is DX_NULL and not all
                              bytes were read DX_VOS_SOCKET_ERROR will be returned.*/

		);

	/*!	Transmits data to a previously established connection-based socket.
		Functions returns only after all bytes are written or if timeout
		expired or error occurred. */
	DxStatus DX_VOS_SocketWrite (
		DxVosSocket aSocket, /*!< [in] socket handle */
		const void *aBuf,	 /*!< [in] pointer to input buffer */
		DxUint32 aBufLen,	 /*!< [in] number of bytes to write. */
		DxUint32* bytesWritten, /*!< [out] number of bytes that were actually written. May be NULL */
		DxUint32 timeout	 /*!< [in] timeout in milliseconds */
		);

	/*! Sends a datagram with the specified data to the specified destination */
	DxStatus DX_VOS_SocketSendTo (
		DxVosSocket aSocket, /*!< [in] socket handle */
		const void *aBuf,	 /*!< [in] pointer to input buffer */
		DxUint32 aBufLen,	 /*!< [in] on entry number of bytes to write. */
        DxUint32* bytesWritten ,/*!< [out] actual	number of bytes written. If this parameter is DX_NULL and not all
                                 bytes were read DX_VOS_SOCKET_ERROR will be returned.*/
		const DxIpAddress* aTo /*!< [in] Destination IP Address */
		);

	/*!
	Associates a network address with a specified socket.
	future calls to DX_VOS_SocketSendTo will use this address as source address.
	future calls to DX_VOS_RecvFrom will return packets that were sent to this address.
	**/
	DxStatus DX_VOS_SocketBind (
		DxVosSocket aSocket,			/*!< [in] socket handle */
		const DxIpAddress* aBindAddr	/*!< [in] local IP Address */ 
		);

	/*!	Accepts a connection on a socket. If no connection is pending function
		blocks until a connection is accepted or error occurs.
		To check if there is a pending connection user may use DX_VOS_SocketWaitForData(). */
	DxStatus DX_VOS_SocketAccept(
		DxVosSocket aSocket,			/*!< [in] socket handle */
		DxVosSocket *aAcceptedSocket,	/*!< [in] socket handle of the new socket */
		DxIpAddress *aAcceptedSockAddr	/*!< [in] IP Address of incoming connection. This parameter may be NULL.*/
		);

    DxStatus DX_VOS_SocketSelect(
        DxVosSocket* socketsToCheck,
        DxUint32* numOfSockets,
        DxVosSocket* readableSockets,
        DxUint32 aTimeout
        );

	/*! Sets the fields of an IP address. */
	void DX_VOS_SetIPAddress(DxIpAddress* ipAddress, DxUint8 b1, DxUint8 b2, DxUint8 b3, DxUint8 b4, DxUint16 port);

    /*! Initializes ipAddress from serverName & port (using DNS if necessary) */
    DxStatus DX_VOS_BuildIpAddress(DxIpAddress* ipAddress, const DxChar* serverName, DxUint16 port);

	/*! Transform the URL to ip address */
	DxStatus DX_VOS_GetHostByName(const DxChar* serverName, DxIpAddress* ipAddress);

    /*! Enables or disables the Socket Traffic logging mechanism.
        \return The old value of the mechanism activity status.
    */
    DxBool DX_VOS_EnableSocketLog(DxBool newValue);

    DxStatus DX_VOS_GetOsHandleFromSocketHandle(DxVosSocket aSocket, void** osHandle);

#ifdef  __cplusplus
}
#endif


#endif /* ifndef _DX_VOS_SOCKET_H */







