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
 
 #ifndef MSG_INTERFACE_H
#define MSG_INTERFACE_H

/*! \file DX_VOS_MsgInterface.h
This module provide generic interface to a Message Exchange mechanism.
The Message Exchange mechanism is a mechanism that can transfer buffers between
processes (used in interprocess communication) or between processors (used to
call procedures on remote machines). A process can act as client, server or both.
A client is characterized by sending requests to the server and waiting for responses.
A Server is characterized by waiting for client requests, processing of the request
and then responding.
A Server may get requests from several clients and a client may issue requests to several servers.

The design of the module is object oriented (like in C++) and allow the implementation
of different mechanisms for different purposes in the same application (for example:
socket based mechanism for RPC to other machines and Message Queues mechanism for IPC
in the same processor).

The DxVosMsgIFDriver structure holds pointers to the objects methods and functions
like a virtual table. To implement a MsgInterface the user should "inherit" from
the DxVosMsgIFDriver structure by writing a new module that declares a new structure 
in which the first member is an instance of DxVosMsgIFDriver and it may be followed 
by other data members. 

This new "derived" module should implement the required operations: Start, Stop,
ReadRequest, SendRequest, WaitForRequest, ReadResponse, SendResponse, WaitForResponse.
The "derived" module should supply also an initialization function that sets the pointers 
in the "virtual table" to the correct functions and initialize the other data memebers.

The unit which is transferred from side to side using this mechanism is a message which
is described by DxVosMsg. Every message starts with a 32 bits opcode followed by
a 32 bits value which indicates the message buffer size in bytes.

Because the generic module does not want to limit the maximum buffer size, the DxVosMsg
is just a placeholder for a structure that begins with OpCode & BuffSize. The "derived" module
should define a new structure which is identical DxVosMsg but the size of m_Buffer array
matches the maximum size of the message buffer.

The interface supports the definition of different transfer mechanisms for requests & responses.
If the transfer mechanisms for requests & responses is identical the corresponding function pointers
can point to the same function.

All functions that return DxStatus will return:
- DX_SUCCESS - on success.
- DX_BAD_ARGUMENTS - if there are invalid parameters
- DX_TIMEOUT_EXPIRED - if timeout expired in wait & read functions.
- Any other error if function fails (depends on the mechanism implementation).

See example of implementation in: DX_VOS_SocketMsgInterface.h
*/

#include "DX_VOS_BaseTypes.h"

/*! The header of every message. This should be the type of the first member in 
very "derived" message structure */ 
typedef struct {
	DxUint32 m_OpCode;	/*!< Type of message */
	DxUint32 m_BuffSize;	/*!< Size in bytes of message buffer (not including header) */ 
} DxVosMsgHeader;

/*! This is a placeholder for a message structure. A "derived" module should define a new structure
with the same field but with different size of the m_Buffer array */
typedef struct {
	DxVosMsgHeader m_Header;
	DxUint8 m_Buffer[1];
} DxVosMsg;

typedef struct _DxVosMsgIFDriver DxVosMsgIFDriver;


/*! Starts the server mechanism. After a call to this function the server may start waiting for requests using
    DX_VOS_MSGIF_WaitForRequest(). */
typedef DxStatus (*DX_VOS_MSGIF_StartServer_t)(
	DxVosMsgIFDriver* driver,	/*!< [in] Pointer to the MsgInterface object*/
	DxUint32 serverId		    /*!< [in] The id that represent the server. All requests that are sent to this
                                          id must reach this server. */
	);

/*! Stops the mechanism (server or client). After a call to this function a call to any other function should fail.
	After calling this function the application can call the start function to reactivate the
	mechanism (there should be no need to call the "derive" module initialization function).*/
typedef DxStatus (*DX_VOS_MSGIF_Stop_t)(DxVosMsgIFDriver* driver);

/*! Reads a message from the other side of the interface. */
typedef DxStatus (*DX_VOS_MSGIF_ReadMessage_t)(
	DxVosMsgIFDriver* driver,	/*!< [in]  Pointer to the MsgInterface object (after calling to the start function) */
	DxVosMsg* msg,				/*!< [out] Empty Msg structure */
	DxUint32 maxBuffSize,		/*!< [in]  The capacity in bytes of the msg->m_Buffer array */
	DxUint32* sourceId,		/*!< [out] The id of the source that sent the message */
	DxUint32 timeout			/*!< [in]  timeout in milliseconds */
	);

/*! Reads a message from the other side of the interface. */
typedef DxStatus (*DX_VOS_MSGIF_ReadMessageFrom_t)(
	DxVosMsgIFDriver* driver,	/*!< [in]  Pointer to the MsgInterface object (after calling to the start function) */
	DxUint32 sourceId,		/*!< [out] The id of the source that sent the message */
	DxVosMsg* msg,				/*!< [out] Empty Msg structure */
	DxUint32 maxBuffSize,		/*!< [in]  The capacity in bytes of the msg->m_Buffer array */
	DxUint32 timeout			/*!< [in]  timeout in milliseconds */
	);

typedef DxStatus (*DX_VOS_MSGIF_SendMessage_t)(
	DxVosMsgIFDriver* driver,	/*!< [in] Pointer to the MsgInterface object (after calling to the start function) */
	DxUint32 destId,			/*!< [in] The id of the destination of the message */
	const DxVosMsg* msg,				/*!< [in] Message to send. All fields must be initialized 
								(especially msg->m_Header.m_BuffSize) */
	DxUint32 timeout			/*!< [in] timeout in milliseconds */
	);

/*! Waits until a message arrives (DX_RETURN(DX_SUCCESS)) or until timeout expires (return DX_TIMEOUT_EXPIRED).*/
typedef DxStatus (*DX_VOS_MSGIF_WaitForMessage_t)(
	DxVosMsgIFDriver* driver, /*!< [in] Pointer to the MsgInterface object (after calling to the start function) */ 
	DxUint32* sourceId,		/*!< [out] The id of the source that sent the message */
	DxUint32 timeout	   /*!< [in] timeout in milliseconds */
	);

/*! Waits until a message arrives (DX_RETURN(DX_SUCCESS)) or until timeout expires (return DX_TIMEOUT_EXPIRED).*/
typedef DxStatus (*DX_VOS_MSGIF_WaitForMessageFrom_t)(
	DxVosMsgIFDriver* driver, /*!< [in] Pointer to the MsgInterface object (after calling to the start function) */ 
	DxUint32 sourceId,		/*!< [out] The id of the source that sent the message */
	DxUint32 timeout	   /*!< [in] timeout in milliseconds */
	);

/*! This structure is the "base class" of "derived" MsgInterface classes.
	It holds a "virtual table" of pointer to functions that implements the specific derived mechanism.
	A "derived" module should define new structure whose first field will be of this type.
*/
struct _DxVosMsgIFDriver {

	DX_VOS_MSGIF_StartServer_t StartServer;
	DX_VOS_MSGIF_Stop_t Stop;

	DX_VOS_MSGIF_ReadMessage_t ReadRequest;
	DX_VOS_MSGIF_SendMessage_t SendRequest;
	DX_VOS_MSGIF_WaitForMessage_t WaitForRequest;

	DX_VOS_MSGIF_ReadMessageFrom_t ReadResponse;
	DX_VOS_MSGIF_SendMessage_t SendResponse;
	DX_VOS_MSGIF_WaitForMessageFrom_t WaitForResponse;
};

#define DX_VOS_MSGIF_StartServer(driver, serverId) \
	((DxVosMsgIFDriver*)driver)->StartServer((DxVosMsgIFDriver*)driver, serverId)

#define DX_VOS_MSGIF_Stop(driver) \
	((DxVosMsgIFDriver*)driver)->Stop((DxVosMsgIFDriver*)driver)

#define DX_VOS_MSGIF_ReadRequest(driver, msg, maxBuffSize, sourceId, timeout) \
	((DxVosMsgIFDriver*)driver)->ReadRequest((DxVosMsgIFDriver*)driver, msg, maxBuffSize, sourceId, timeout)
#define DX_VOS_MSGIF_SendRequest(driver, destId, msg, timeout) \
	((DxVosMsgIFDriver*)driver)->SendRequest((DxVosMsgIFDriver*)driver, destId, msg, timeout)
#define DX_VOS_MSGIF_WaitForRequest(driver, sourceId, timeout) \
	((DxVosMsgIFDriver*)driver)->WaitForRequest((DxVosMsgIFDriver*)driver, sourceId, timeout)

#define DX_VOS_MSGIF_ReadResponse(driver, sourceId, msg, maxBuffSize, timeout) \
	((DxVosMsgIFDriver*)driver)->ReadResponse((DxVosMsgIFDriver*)driver, sourceId, msg, maxBuffSize, timeout)
#define DX_VOS_MSGIF_SendResponse(driver, destId, msg, timeout) \
	((DxVosMsgIFDriver*)driver)->SendResponse((DxVosMsgIFDriver*)driver, destId, msg, timeout)
#define DX_VOS_MSGIF_WaitForResponse(driver, sourceId, timeout) \
	((DxVosMsgIFDriver*)driver)->WaitForResponse((DxVosMsgIFDriver*)driver, sourceId, timeout)

#endif
