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
 
 #ifndef SOCKET_MSG_INTERFACE_H
#define SOCKET_MSG_INTERFACE_H

#include "DX_VOS_MsgInterface.h"
#include "DX_VOS_Socket.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! \file DX_VOS_SocketMsgInterface.h
This module implements the generic interface definde in DX_VOS_MsgInterface.h over sockets.
*/
typedef struct 
{
	DxUint32 m_ClientId;
	DxVosSocket m_Socket;
} DxOutgoingConnection;

/*! This is the "derived" DxVosMsgIFDriver structure. */
typedef struct {
	DxVosMsgIFDriver m_Driver;		/*!< Base Class. "Virtual Table" container. */
	DxVosSocket m_ListenerSocket;	/*!< Handle to socket */
	DxVosSocket m_OutgoingSocket;	/*!< Handle to socket */
	DxIpAddress m_LocalIPAddress;		/*!< If m_IsClient is true this is the remote address. 
										 If m_IsClient is false this is the local address. */
	DxIpAddress m_RemoteIPAddress;		/*!< If m_IsClient is true this is the remote address. 
									If m_IsClient is false this is the local address. */
	DxUint m_MaxIncoming;
	DxVosSocket* m_AcceptedSockets;
} DxVosSocketMsgIFDriver;


/*! initializes the virtual table and the data members. */
DxStatus DX_VOS_SOCKET_MSGIF_Init(
	DxVosSocketMsgIFDriver* driver, /*!< [in] Pointer to uninitialized MsgInterface object*/
	const DxIpAddress* localIpAddress,	/*!< [in] if the interface is intended to be server this parameter
											  indicates the local address on which the server should wait
											  for incoming client connections. 
											  If the interface is intended to be client this parameter should be DX_NULL.
											  */
	const DxIpAddress* remoteIpAddress,	/*!< [in] if the interface is intended to be server this parameter
											  should be DX_NULL.
											  If the interface is intended to be client this parameter
											  indicates the remote address to which it should connect.
											  If NULL the localhost address (127.0.0.1) will be used. 
											  The port field of the ipAddress is ignored.
											  */
    DxUint maxIncoming
);

#ifdef __cplusplus
}
#endif

#endif

