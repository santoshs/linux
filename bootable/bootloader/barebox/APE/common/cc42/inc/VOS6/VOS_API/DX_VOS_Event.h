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
 
 #ifndef _DX_VOS_EVENT_H
#define _DX_VOS_EVENT_H

/*! \file DX_VOS_Event.h
    \brief This file enables event operations 
*/

#include "DX_VOS_BaseTypes.h"
#include "DX_VOS_Errors.h"

#ifdef __cplusplus
extern "C"
{
#endif
				  
/*************************** DEFINES ***********************/

/* For using by DX_VOS_EventWait */

#define DX_VOS_EVENT_WAIT_INFINITY (DxInt)(0xFFFFFFFF)   

#define DX_VOS_EVENT_NoWait          0




/*************************** Typedefs *********************/


typedef enum{
	DX_VOS_EVENT_AND,       /* all the flags should be present */
	DX_VOS_EVENT_AND_CLEAR, /* all the flags should be presented, and then they are cleared */
	DX_VOS_EVENT_OR, 		/* any of the flags should be present */
	DX_VOS_EVENT_OR_CLEAR	/* any of the flags should be presented, and then they are cleared */
}DX_VOS_EventOption_t;



typedef struct _DxVosEventID*  DxVosEventID;
  

/*!
 \brief 
 DX_VOS_EventCreate () - Create event 
 @param[in] aEventId_ptr - Pointer to where the handle to the created event will be stored
 @param[in] aStringName_ptr - null-terminated string to be assigned to event name
 @return On success the function returns DX_SUCCESS. 
 	                        On error,
 	                        if argument equal NULL, DX_BAD_ARGUMENTS is returned
 	                        if create operation fails, DX_VOS_FAIL is returned
 **/

DxVosResult_t DX_VOS_EventCreate (DxVosEventID  *aEventId_ptr, DxInt8 *aStringName_ptr);


/*!
 \brief 
 DX_VOS_EventDelete () -  terminates and deallocates resources associated with the specified event 
 @param[in] aEventId - EventID as returned from DX_VOS_EventCreate.
 @return On success the function returns DX_SUCCESS. 
 	                        On error,
 	                        if argument equal NULL, DX_BAD_ARGUMENTS is returned
 	                        if create operation fails, DX_VOS_FAIL is returned
 **/


DxVosResult_t DX_VOS_EventDelete (DxVosEventID  aEventId);


/*!
 \brief 
 DxVOS_EventWait () -  wait for event with timeout
 @param[in] aEventId - EventID for create
 @param[in] requestedFlags - requested flasg for event to wait for
 @param[in] eventOption - the option for requested flags true condition
 						 DX_VOS_EVENT_AND  - wait until all the requested bits are set
 						 DX_VOS_EVENT_AND_CLEAR - wait until all the requested bits are set and then clear the bits array in the event
 						 DX_VOS_EVENT_OR - wait until any of the requested bit are set
 						 DX_VOS_EVENT_OR_CLEAR - wait until all the requested bit and then clear the bits array in the event
 @param[in] actualFlags - the actual flags that cause the termination of wait on event
 @param[in] aTimeout - timeout in mSec, or DX_VOS_EVENT_WAIT_INFINITY
 @return DxVosResult_t  - On success the function returns DX_SUCCESS. 
 	                        On error,
 	                        if argument equal NULL, DX_BAD_ARGUMENTS is returned
 	                        if create operation fails, DX_VOS_FAIL is returned
 **/

DxVosResult_t DX_VOS_EventWait(DxVosEventID aEventId, DxUint32 requestedFlags , DX_VOS_EventOption_t eventOption , DxUint32* actualFlags , DxInt aTimeout);


/*!
 \brief 
 DxVOS_EventSet () -  sets the event bits
 @param[in] aEventId - EventID as returned from DX_VOS_EventCreate.
 @param[in] setFlags - the flags to set in the event.
 @param[in] eventOption - defines how to set the flags.
 						 DX_VOS_EVENT_AND  - peform bit-wise AND between the setFlags and the bit array of the event
 						 DX_VOS_EVENT_OR - peform bit-wise OR between the setFlags and the bit array of the event
 @return DxVosResult_t  - On success the function returns DX_SUCCESS. 
 	                        On error,
 	                        if argument equal NULL, DX_BAD_ARGUMENTS is returned
 	                        if create operation fails, DX_VOS_FAIL is returned
 **/

DxVosResult_t DX_VOS_EventSet(DxVosEventID aEventId , DxUint32 setFlags , DX_VOS_EventOption_t eventOption);





#ifdef  __cplusplus
}
#endif


#endif /* ifndef _DX_VOS_SEM_H */
