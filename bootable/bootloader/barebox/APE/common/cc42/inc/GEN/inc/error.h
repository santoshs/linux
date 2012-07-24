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
 
 
#ifndef _ERROR_H
#define _ERROR_H
 
/*! \file error.h
    \brief This file containes all internal ESC error codes.          
*/

#include "DX_VOS_BaseTypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* ******************** global definitions ******************** */

/* host base error */
#define HOST_BASE_ERROR_OFFSET                    0x0B000000

//typedef DxUint32_t DxError;

/* success code */
#define DX_OK 									  0

/* return message from SEP contains wrong opcode */
#define DX_WRONG_OPCODE_FROM_SEP_ERR              (HOST_BASE_ERROR_OFFSET + 0x1)

/* failure to allocate memory for flow table  */
#define DX_FAIL_ALLOCATE_MEMORY_FOR_TABLE_ERR     (HOST_BASE_ERROR_OFFSET + 0x2)

/* wrong start token at the beginnong of the incoming message from SEP */
#define DX_FAILED_START_TOKEN_ERR                 (HOST_BASE_ERROR_OFFSET + 0x3)

/* error in CRC in the incoming message */
#define DX_FAILED_CRC_IN_MESSAGE_ERR              (HOST_BASE_ERROR_OFFSET + 0x4)

/* allocation failed for KMNG */
#define DX_KMNG_ALLOCATION_FAIL_ERR               (HOST_BASE_ERROR_OFFSET + 0x5)

/* invalid input pointer */
#define DX_KMNG_INVALID_INPUT_POINTER_ERR         (HOST_BASE_ERROR_OFFSET + 0x6)

/* invalid polling type */
#define DX_INVALID_POLLING_TYPE_ERR               (HOST_BASE_ERROR_OFFSET + 0x7)

/* not enough shared area allocated memory */
#define DX_NOT_ENOUGH_MEMORY_IN_SHARED_ARREA_ERR  (HOST_BASE_ERROR_OFFSET + 0x8)

/* message parameter is not word alligned */
#define DX_PARAM_NOT_ALLIGNED_ERR                 (HOST_BASE_ERROR_OFFSET + 0x9)

/* SEP_GPR3 ==1 */
#define DX_MNG_SEP_IS_DISABLE_ERR					  (HOST_BASE_ERROR_OFFSET + 0xA)

/* SEP_GPR3==1 from the function DX_MNG_GetLCS */
#define DX_MNG_LCS_SECURITY_DISABLE_ERR			  (HOST_BASE_ERROR_OFFSET + 0xB)

/* SEP_GPR3==0x20 (BM error) and waits for HOST handling */
#define DX_MNG_BM_ERROR_ON_SEP_ERR			  	      (HOST_BASE_ERROR_OFFSET + 0xC)

/* trying to recover from I-cache violation without the SEP in this state */
#define DX_MNG_NO_I_CACHE_VIOLATION_ERR  	      (HOST_BASE_ERROR_OFFSET + 0xD)

/* Wrong SEP status to get  */
#define DX_MNG_WRONG_SEP_STATUS_ERR  	           (HOST_BASE_ERROR_OFFSET + 0xE)
/* Wrong SEP status to get  */
#define DX_INVALID_PARAMETERS_ERR  	           	  (HOST_BASE_ERROR_OFFSET + 0xF)

/* Wrong HOST D-cache parameter  */
#define DX_INVALID_D_CACHE_PARAM_ERR  	           	  (HOST_BASE_ERROR_OFFSET + 0x10)

/* num of ranges limitted up to 4 ranges */
#define DX_NUM_OF_RANGES_OVERFLOW_ERR						(HOST_BASE_ERROR_OFFSET + 0x11) 

/* message is too long */
#define DX_MESSAGE_OVERFLOW_ERR                      (HOST_BASE_ERROR_OFFSET + 0x12)

#ifdef  __cplusplus
}
#endif


#endif /* ifndef ERROR */





