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
 
 #ifndef _DX_VOS_MAILBOX_H
#define _DX_VOS_MAILBOX_H

/*! \file DX_VOS_Mailbox.h
    \brief This file enables event operations 
*/

#include "DX_VOS_BaseTypes.h"
#include "DX_VOS_Errors.h"

#ifdef __cplusplus
extern "C"
{
#endif
				  
/*************************** DEFINES ***********************/

/* For using by DX_VOS_Wait */

#define DX_VOS_MAILBOX_WAIT_INFINITY (DxInt)(0xFFFFFFFF)   

#define DX_VOS_MAILBOX_NoWait          0




/*************************** Typedefs *********************/


typedef struct _DxVosMailboxID*  DxVosMailboxID;
  

/*!
 \brief 
 DX_VOS_MailboxCreate () - Create event 
 @param[in] aMailboxId_ptr - Pointer to where the handle to the created mailbox will be stored
 @param[in] aStringName_ptr - null-terminated string to be assigned to event name
 @param[in] aMessageSize - size of the message in the mailbox
 @param[in] aQueueStack_ptr - the stack that will be used by the mailbox
 @param[in] aQueueMsgNum - a number of messages in the queue. The size of the queue stack can be deducted from the aQueueMsgNum and aMessageSize
 @return On success the function returns DX_SUCCESS. 
 	                        On error,
 	                        if argument equal NULL, DX_BAD_ARGUMENTS is returned
 	                        if create operation fails, DX_VOS_FAIL is returned
 **/

DxVosResult_t DX_VOS_MailboxCreate (DxVosMailboxID  *aMailboxId_ptr, DxInt8 *aStringName_ptr , DxUint32 aMessageSize , DxInt8 *aQueueStack_ptr , DxUint32 aQueueMsgNum);


/*!
 \brief 
 DX_VOS_MailboxDelete () -  terminates and deallocates resources associated with the specified mailbox 
 @param[in] aMailboxId - MailboxID as returned from DX_VOS_MailboxCreate.
 @return On success the function returns DX_SUCCESS. 
 	                        On error,
 	                        if argument equal NULL, DX_BAD_ARGUMENTS is returned
 	                        if create operation fails, DX_VOS_FAIL is returned
 **/


DxVosResult_t DX_VOS_MailboxDelete (DxVosMailboxID  aMailboxId);


/*!
 \brief 
 DX_VOS_MailboxSend () -  send message to mailbox
 @param[in] aMailboxId - MailboxID for create
 @param[in] aMessage_ptr - the pointer to the requested message
 @param[in] aTimeout - timeout in mSec, or DX_VOS_MAILBOX_WAIT_INFINITY
 @return DxVosResult_t  - On success the function returns DX_SUCCESS. 
 	                        On error,
 	                        if argument equal NULL, DX_BAD_ARGUMENTS is returned
 	                        if create operation fails, DX_VOS_FAIL is returned
 **/

DxVosResult_t DX_VOS_MailboxSend(DxVosMailboxID aMailboxId, DxUint8* aMessage_ptr , DxInt aTimeout);


/*!
 \brief 
 DX_VOS_MailboxReceive () -  received message from mailbox
 @param[in] aMailboxId - MailboxID as returned from DX_VOS_MailboxCreate.
 @param[in] aMessage_ptr - pointer to where a message should be stored
 @param[in] aTimeout - timeout in mSec, or DX_VOS_MAILBOX_WAIT_INFINITY
 @return DxVosResult_t  - On success the function returns DX_SUCCESS. 
 	                        On error,
 	                        if argument equal NULL, DX_BAD_ARGUMENTS is returned
 	                        if create operation fails, DX_VOS_FAIL is returned
 **/

DxVosResult_t DX_VOS_MailboxReceive(DxVosMailboxID aMailboxId , DxUint8* aMessage_ptr , DxInt aTimeout);





#ifdef  __cplusplus
}
#endif


#endif /* ifndef _DX_VOS_SEM_H */
