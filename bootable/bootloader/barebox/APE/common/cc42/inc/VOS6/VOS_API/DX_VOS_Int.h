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
 
 #ifndef _DX_VOS_INT_H
#define _DX_VOS_INT_H

/*! \file DX_VOS_Int.h
    \brief This file enables interrupt handling - blocking and unblocking 
*/

#include "DX_VOS_BaseTypes.h"
#include "DX_VOS_Errors.h"

#ifdef __cplusplus
extern "C"
{
#endif
				  
/*************************** DEFINES ***********************/





/*************************** Typedefs *********************/


#define DX_VOS_INT_DISABLE	0xFFFFFFFF
#define DX_VOS_INT_ENABLE	0x0


  

/*!
 \brief 
 DX_VOS_InterruptControl () - control the interrups - set the mask 
 @param[in] aIntCtrlMask - the requested masking of the interrupts 
 @param[in] aPrevMask - the previous mask
 @return On success the function returns DX_SUCCESS. 
 	                        On error,
 	                        if argument equal NULL, DX_BAD_ARGUMENTS is returned
 	                        if create operation fails, DX_VOS_FAIL is returned
 **/

DxVosResult_t DX_VOS_InterruptControl (DxUint32  aIntCtrlMask, DxUint32 *aPrevMask);



#ifdef  __cplusplus
}
#endif


#endif /* ifndef _DX_VOS_INT_H */
