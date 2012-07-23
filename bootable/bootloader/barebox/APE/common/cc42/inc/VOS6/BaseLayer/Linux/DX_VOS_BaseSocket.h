/***********************************************************************************  
 * Copyright 2009 © Discretix Technologies Ltd. 
 * This software is protected by copyright, international treaties and 
 * various patents. If the license governing the use of this Software 
 * allows copy or redistribution of this  software then any copy or 
 * reproduction of this Software must include this Copyright Notice 
 * as well as any other notices provided under such license. 
 ***********************************************************************************/
 
 
 #ifndef _DX_VOS_BASE_SOCKET_H_
#define _DX_VOS_BASE_SOCKET_H_
#include "DX_VOS_Socket.h"

/*! \file DX_VOS_BaseSocket.h
This module contains function that implements basic socket operations.
All function may assume that pointer parameters that are passed to them
are not NULL (except if specified otherwise).
For parameter documentation see the matching function in DX_VOS_Socket.h
*/

/* Conversions tables from Discretix enum values to OS dependent values. */
extern DxUint32 DxAddressFamiliesTable[DX_NUMBER_OF_ADDRESS_FAMILIES];
extern DxUint32 DxSocketTypesTable[DX_NUMBER_OF_SOCKET_TYPES];
extern DxUint32 DxProtocolsTable[DX_NUMBER_OF_PROTOCOLS];

/*!  Creates a new socket.
	Function may assume that parameters are	in the OS values after 
	conversion using the conversion tables */
DxStatus DX_VOS_BaseSocketCreate(void** aSocket, DxUint32 addressFamily,
							     DxUint32 aSockType, DxUint32 protocol);
/*!  Listens for incoming connection */
DxStatus DX_VOS_BaseSocketListen(void* aSocket, DxInt aBackLog);

/*!  Listens for incoming connection */
DxStatus DX_VOS_BaseSocketConnect(void* aSocket, const DxIpAddress* aIPAddress);

/*!  Closes an opened socket. */
DxStatus DX_VOS_BaseSocketClose (void* aSocket);

/*! Waits until data is ready for read from the socket or until
timeout expires.
\return
- DX_SUCCESS - if data is ready
- DX_TIMEOUT_EXPIRED - if timeout expired.
- DX_VOS_SOCKET_CLOSE - if connection was closed gracefully.
- DX_VOS_SOCKET_ERROR - if connection was terminated.
**/
DxStatus DX_VOS_BaseSocketWaitForData(void* aSocket, DxUint32 aTimeout);

/*!	Receives available data from a connected socket. 
Function returns immediately with available data.
If no data is available function blocks until data become available or until
error occurs. */
DxStatus DX_VOS_BaseSocketRecv (void* aSocket, void *aBuf, DxUint32 aBufLen, DxUint32* bytesRead);

/*!	Receives available datagram from a datagram socket. 
Function returns immediately with available data.
If no data is available function blocks until data become available or until
error occurs. */
DxStatus DX_VOS_BaseSocketRecvFrom (void* aSocket, void *aBuf, DxUint32 aBufLen, DxUint32* bytesRead, DxIpAddress* aFrom);

/*!	Transmits data to a previously established connection-based socket.
	Functions returns immediately with indication of how many bytes were
	actually written */
DxStatus DX_VOS_BaseSocketSend (void* aSocket, const void *aBuf, DxUint32 aBufLen, DxUint32* bytesWritten);

/*! Sends a datagram with the specified data to the specified destination */
DxStatus DX_VOS_BaseSocketSendTo (void* aSocket, const void *aBuf, DxUint32 aBufLen, DxUint32* bytesWritten, const DxIpAddress* aTo);

/*!
Associates a network address with a specified socket.
future calls to DX_VOS_SocketSendTo will use this address as source address.
future calls to DX_VOS_RecvFrom will return packets that were sent to this address.
**/
DxStatus DX_VOS_BaseSocketBind (void* aSocket, const DxIpAddress* aBindAddr);

/*!	Accepts a connection on a socket. If no connection is pending function
	blocks until a connection is accepted or error occurs.
	To check if there is a pending connection user may use DX_VOS_SocketWaitForData(). */
DxStatus DX_VOS_BaseSocketAccept(void* aSocket, void** aAcceptedSocket, DxIpAddress* aAcceptedSockAddr);

DxStatus DX_VOS_BaseSocketSelect(void** socketsToCheck, DxUint32* numOfSockets, void** readableSockets, DxUint32 aTimeout);

void DX_VOS_BaseLogSocketResult(void);

#endif
