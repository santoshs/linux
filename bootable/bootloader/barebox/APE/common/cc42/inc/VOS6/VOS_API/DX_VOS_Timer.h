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
 
 

#ifndef _DX_VOS_TIMER_H
#define _DX_VOS_TIMER_H

/*! \file DX_VOS_TIMER.h
    \brief This file enables standard timer operations 
*/

#include "DX_VOS_BaseTypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /*************************** Typedefs *********************/
typedef struct _DxTimerValues* DxVosTimer;
typedef void (*DxTimerCallback_t)(void*);

/*! Creates a timer that will call the specified callback every specified period.
	In case of a one time timer the if the handle parameter passed is null 
    (the function user doesn't request the timers handle) the timer will be automatically destroyed
    after one call to the callback function, otherwise the user must use the DX_VOS_DeleteTimer function
    on the handle returned to free the timer's resources when through with the timer. */
IMPORT_C DxStatus DX_VOS_CreateTimer(
	DxVosTimer* handle,	/*!< [out] handle of the new timer. May be NULL if handle is not required. Note: this affects the behavior of one time timers, see above */
	DxTimerCallback_t callback, /*!< [in] function that will be called every interval */
	void* callbackArgs,			/*!< [in] parameter that will be passed to the callback.
										  May be NULL */
	DxUint32 period,			/*!< [in] interval length in milliseconds.			*/
	DxBool isOneTime			/*!< [in] if true it indicates that the callback should 
									be called only once in a "period" milliseconds from now */
	); 

/*! Deletes a timer. 
	if handle is NULL the function does nothing and succeeds. */
IMPORT_C DxStatus DX_VOS_DeleteTimer(DxVosTimer handle);

#ifdef DX_USE_LEGACY_VOS
IMPORT_C void DX_VOS_GetTimer(DxUint32 *aTickCount_ptr); 
#endif
#ifdef  __cplusplus
}
#endif


#endif /* ifndef _DX_VOS_TIMER_H */









