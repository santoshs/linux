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
 
 #ifndef _DX_VOS_DRM_UTILS_H
#define _DX_VOS_DRM_UTILS_H

/*! \file DX_VOS_DrmUtils.h
This module provides DRM specific utilities.
*/
#include "DX_VOS_BaseTypes.h"

#ifdef __cplusplus
extern "C" {
#endif
	typedef enum
	{
		DX_UI_YES,
		DX_UI_NO,
		DX_UI_ERROR,
	} EDxUserResponse;

	typedef enum
	{
		EUI_MESSAGE_LEVEL_ERROR,
		EUI_MESSAGE_LEVEL_WARNING,
		EUI_MESSAGE_LEVEL_INFO,
	} EDxUiMessageLevel;

	typedef enum
	{
        EUI_CONSENT_MESSAGE_INDEX_UNDEFINED         = 0x0000,
		EUI_CONSENT_MESSAGE_INDEX_ACQUIRE_RIGHTS    = 0x0001, /* Would you like to acquire Preview rights?  parameters RI-URL, ContentId */
        EUI_CONSENT_MESSAGE_INDEX_REQUEST_RIGHTS    = 0x0002, /* Would you like to request rights from server?  parameters RI-URL */
        EUI_CONSENT_MESSAGE_INDEX_UPDATE_TRANSACTION_TRACKING = 0x0004, /* Do you wish to update the transaction tracking of the content?  parameters ContentId */
		EUI_CONSENT_MESSAGE_INDEX_REGISTER_RI       = 0x0008, /* Would you like to register to the RI?  parameters RI-URL */
		EUI_CONSENT_MESSAGE_INDEX_JOIN_DOMAIN       = 0x0010, /* Would you like to join the domain?  parameters RI-URL and domain-ID */
		EUI_CONSENT_MESSAGE_INDEX_LEAVE_DOMAIN      = 0x0020, /* Would you like to leave the domain?  parameters RI-URL and domain-ID */
		EUI_CONSENT_MESSAGE_INDEX_UPDATE_DOMAIN     = 0x0040, /* Would you like to update your domain?  parameters RI-URL and domain-ID */
		EUI_CONSENT_MESSAGE_INDEX_CONSUME_RIGHTS    = 0x0080, /* Do you agree to use content and consume rights?  parameters contentId . called when executeIntent on stateful rights */
        EUI_CONSENT_MESSAGE_INDEX_RO_DOWNLOAD_ERROR = 0x0100,
        EUI_CONSENT_MESSAGE_INDEX_REDIRECT_URL      = 0x0200,
        EUI_CONSENT_MESSAGE_INDEX_EXPIRED_RO        = 0x0400
	} EDxUiConsent;

	typedef enum
	{
		/* Error notifications */
		EDX_NOTIFY_EVENT_ROAP_REGISTRATION, /* unable to register RI: parameter to RiUrl(string) */
		EDX_NOTIFY_EVENT_ROAP_RIGHTS_ACQUISITION, /* unable to acquire RO, ROAP transaction failed: parameter to contentId(string) */
		EDX_NOTIFY_EVENT_ROAP_JOIN_DOMAIN, /* unable to join domain, ROAP transaction failed: parameter RI-URL(string) and domainID */
		EDX_NOTIFY_EVENT_ROAP_LEAVE_DOMAIN,/* unable to leave domain, ROAP transaction failed: parameter RI-URL(string) and domainID */

		EDX_NOTIFY_EVENT_CONTENT_CONSUMPTION, // rights error: SDS integrity failed; no right; rightsExpired ; SDS error when updating rights ; 
		// content: integrity fail
		EDX_NOTIFY_EVENT_CONTENT_IMPORT,     // embedded rights : verification (struct) failed, (mac, riId, signature,insufficient memory for storage; rights replay
		// content: integrity fail; contentId mismatch (not supported)
		EDX_NOTIFY_EVENT_CONTENT_RESTORE,     // embedded rights : verification (struct) failed, insufficient memory for storage; rights replay
		// content: integrity fail; contentId mismatch (not supported)
        EDX_NOTIFY_EVENT_WM_ERROR,
		/*

		EDX_NOTIFY_EVENT_
		EDX_NOTIFY_EVENT_
		EDX_NOTIFY_EVENT_
		EDX_NOTIFY_EVENT_

		EDX_DRM_NOTIFICATION_ERROR_INDEX_CORRUPT_RIGHTS,      // received corrupt rights: parameter to ContentId(string) 
		EDX_DRM_NOTIFICATION_ERROR_INDEX_RIGHTS_REPLAY,      // received already existing rights: parameter to ContentId(string) 
		EDX_DRM_NOTIFICATION_ERROR_INDEX_CONTENT_ID_MISMATCH, // contentId deos not match between content and embedded rights, 
		// parameter 1 content Cid(string), parameter2 rights Cid(string) 
		EDX_DRM_NOTIFICATION_ERROR_INDEX_INTEGRITY_FAILED_ON_RIGHTS,  // integrity of rights failed, rights are deleted: parameter to ContentId(string) 
		EDX_DRM_NOTIFICATION_ERROR_INDEX_INTEGRITY_FAILED_ON_CONTENT,  // integrity of content failed, rights are deleted: parameter to filename(string)
		EDX_DRM_NOTIFICATION_ERROR_INDEX_RIGHTS_EXPIRED,             // the rights have expired: parameter to contentId(string) . will be called upon expiry in the middle of play 
		EDX_DRM_NOTIFICATION_ERROR_INTEGRITY_FAILED_ON_CONTENT, //integrity fails after download completes: parameter to Contnet FileName(string) 
		// General notifications 
		EDX_DRM_NOTIFICATION_EVENT_RIGHTS_ARRIVED = 0x20, // received rights : parameter to ContentId 
		EDX_DRM_NOTIFICATION_EVENT_REGISTRATION_SUCCEEDED, // registration completed successfully: parameter RiUrl 
		EDX_DRM_NOTIFICATION_EVENT_JOIN_DOMAIN_SUCCEEDED, // domain was added successfully: parameter RiUrl, domainId 
		EDX_DRM_NOTIFICATION_EVENT_LEAVE_DOMAIN_SUCCEEDED // leave domain was successfull: domainId 
		*/  
	} EDxDrmNotification;

	typedef void* DxMsgContext_t;
	typedef void (*DxUICallbackFunc_t)(void*, EDxUserResponse);

	/*! The function displays the message to the user and return immediately.
		The function should let the user choose between two options: Yes or No.
		When the user choose one of options the system should call the callback function with the
		user selection and the specified parameter. 
		\return 
		- DX_SUCCESS - On Success.
		- DX_BAD_ARGUMENTS - if URL is NULL.
		- DX_VOS_DRM_UTILS_ERROR - if operation failed.
		*/
	DxStatus DxAsyncShowUserConsentMessage(const DxChar* Msg, DxMsgContext_t msgContext, DxUICallbackFunc_t callback, void* callbackParam);  

	DxStatus DxSyncShowUserConsentMessage(const DxChar* Msg, DxMsgContext_t msgContext, EDxUserResponse* response);  

	/*! The function should displays the message to the user and return immediately. 
		\return 
		- DX_SUCCESS - On Success.
		- DX_BAD_ARGUMENTS - if URL is NULL.
		- DX_VOS_DRM_UTILS_ERROR - if operation failed.
	*/
	DxStatus DxAsyncShowMessage(const DxChar* Msg, DxMsgContext_t msgContext);  

	DxStatus DxSyncShowMessage(const DxChar* Msg, DxMsgContext_t msgContext);  

	/*! Asks the browser to open a browser session with the specified URL (HTTP GET). 
		If ROAP trigger is received, the the browser should invoke DxClient->DxHandleDrmMessage.
		\return 
		- DX_SUCCESS - On Success.
		- DX_BAD_ARGUMENTS - if URL is NULL.
		- DX_VOS_DRM_UTILS_ERROR - if operation failed.
	*/
	DxStatus DxOpenBrowserSession(const DxChar* aUrl);  

	/*! Returns a message format string that match msgType.
		The message may contain %1 & %2 in the places where message parameters should be inserted.
		\return 
		- DX_SUCCESS - On Success.
		- DX_BAD_ARGUMENTS - if URL is NULL.
		- DX_BUFFER_IS_NOT_BIG_ENOUGH - if supplied buffer cannot contain the entire format string.
	*/
	DxStatus DxGetUserConsentMsgFormat(DxChar* aBuff, DxUint aBuffSize, EDxUiConsent msgType, DxMsgContext_t* msgContext);  

	/*! Returns a message format string that match msgType.
		The message may contain %1 & %2 in the places where message parameters should be inserted.
		\return 
		- DX_SUCCESS - On Success.
		- DX_BAD_ARGUMENTS - if URL is NULL.
		- DX_BUFFER_IS_NOT_BIG_ENOUGH - if supplied buffer cannot contain the entire format string.
	*/
	DxStatus DxGetNotificationMsgFormat(DxChar* aBuff, DxUint aBuffSize, EDxDrmNotification msgType, DxStatus errorCode, DxMsgContext_t* msgContext);

#ifdef __cplusplus
}
#endif
#endif
