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
 
 #ifndef _DX_VOS_NOTIFICATION_H
#define _DX_VOS_NOTIFICATION_H

/*! \file DX_VOS_Notification.h
This module provide notification utilities. 
Notification is a signal that is sent from process to process.
There are two kinds of notificators: 
- Sender notificator which is used to send notification to other notificator.
- Reciever notificator which is used to retrieve notification.

A Single sender notificator may send many notification to many receiver notificators.
Every receiver notificator is identified by a notificator id and every
notification is identified by notification id. The sender notificator must specify
the destination notificator id and the notification id in order to send a notification.
*/

#ifdef __cplusplus
extern "C" {
#endif

#include "DX_VOS_BaseTypes.h"
typedef void* DxVosSenderNotificator;
typedef void* DxVosReceiverNotificator;
typedef DxUint32 DxVosNotificationId;

/*! Initialize a Sender notificator */
DxStatus DX_VOS_CreateSenderNotificator(DxVosSenderNotificator* notificator);

/*! Deletes an existing Sender notificator */
DxStatus DX_VOS_DeleteSenderNotificator(DxVosSenderNotificator notificator);

/*! Sends a notification identified by 'notificationId' to the receiver notificator
	identified by 'destNotificatorId' using the sender notificator specified in 'notificator' */
DxStatus DX_VOS_SendNotification(DxVosSenderNotificator notificator, DxUint32 destNotificatorId, DxVosNotificationId notificationId);

/*! Initialize a Receiver notificator that will receive all the notification which
	re sent to the specified notificatorId.	*/
DxStatus DX_VOS_CreateReceiverNotificator(DxVosReceiverNotificator* notificator, DxUint32 notificatorId);

/*! Deletes an existing Receiver notificator */
DxStatus DX_VOS_DeleteReceiverNotificator(DxVosReceiverNotificator notificator);

/*! Retrieves a notification from the specified notificator. 
	On exist notificationId will hold the id of the received notification.
	If there is no pending notification the function return immediately.
	\return
	- DX_SUCCESS - if notification arrived
	- DX_VOS_NO_NOTIFICATION - if there is no pending notification.
	- Any other error is operation fails.
*/
DxStatus DX_VOS_GetNotification(DxVosReceiverNotificator notificator, DxVosNotificationId* notificationId);

/*! Waits until a notification arrives. The notification can be retrieved using DX_VOS_GetNotification().
	\return
	-DX_SUCCESS - if notification is pending.
	- Any other error is operation fails.
	*/
DxStatus DX_VOS_WaitForNotification(DxVosReceiverNotificator notificator);

#ifdef __cplusplus
}
#endif

#endif
