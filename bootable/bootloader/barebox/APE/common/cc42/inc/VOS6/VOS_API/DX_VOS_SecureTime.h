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
 
 #ifndef _DX_VOS_SECURE_TIME_
#define _DX_VOS_SECURE_TIME_

#include "DX_VOS_TimeUtils.h"
#include "DX_VOS_Sem.h"
/*! \file DX_VOS_SecureClock.h
    The secured clock is a clock that cannot be set or influenced by the user.
    The secured clock can be set only using DX_VOS_SetSecureTimeFromGenTime() 
    or DX_VOS_SetSecureTime() which will be called only by secured applications
    that get the current real time clock from reliable source.
    
    The secure clock should have backup battery so it will keep up running even
    if the main battery is not conncted. If the secure clock power is lost the
    secure clock enters a reset state. In this state every call to DX_VOS_GetSecureTime()
    or DX_VOS_GetSecureTimeAsGenTime() will return DX_VOS_SECURE_TIME_IN_RESET_STATE.
    The secure clock will leave the reset state only after a call to
    DX_VOS_SetSecureTimeFromGenTime() or DX_VOS_SetSecureTime().
*/
#ifdef __cplusplus
extern "C" {
#endif

    enum _DxSecureClockId {
        DX_GENERIC_CLOCK,
        DX_OMA_CLOCK,
        DX_WM_CLOCK,
		DX_OMA_V1_CLOCK,
        DX_NUM_OF_CLOCKS
    };
/*!\brief Sets the secure time to the specified time.
    
    If the secure clock was in reset state a call to this function will
    activate it again so subsequent call to DX_VOS_GetSecureTime() will
    return the secure time based on the time that was set in this call.

    \return 
    DX_SUCCESS - On success.
    DX_BAD_ARGUMENTS - If aTime is DX_NULL
    DX_VOS_TIME_ERROR - on error.
*/
    DxStatus DX_VOS_SetSecureTimeFromSecs(DxUint32 clockId, DxTime_t secs);
    DxStatus DX_VOS_SetSecureTimeFromGenTime(DxUint32 clockId, const DxGeneralizedTime_t *aTime);
    DxStatus DX_VOS_SetSecureTimeStruct(DxUint32 clockId, const DxTimeStruct_t *aTime);

 /*!\brief Retrieves the secure time into the specified structure.
    
    \return 
    DX_SUCCESS - On success.
    DX_BAD_ARGUMENTS - If aTime is DX_NULL
    DX_VOS_SECURE_TIME_IN_RESET_STATE - if the secure clock is in reset state.
    DX_VOS_TIME_ERROR - on any other error.
 */
    DxStatus DX_VOS_GetSecureTimeAsSecs(DxUint32 clockId, DxTime_t* secs);
    DxStatus DX_VOS_GetSecureTimeAsGenTime(DxUint32 clockId, DxGeneralizedTime_t *aTime);
    DxStatus DX_VOS_GetSecureTimeStruct(DxUint32 clockId, DxTimeStruct_t *aTime);

    /*! For debugging purpose only */
    DxStatus DX_VOS_UnsetSecureTime();

#ifdef __cplusplus
}
#endif

#endif /* ifndef _DX_VOS_SECURE_TIME_ */


