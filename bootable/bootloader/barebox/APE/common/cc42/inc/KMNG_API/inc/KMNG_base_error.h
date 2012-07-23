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
 
 
#ifndef _KMNG_BASE_ERROR_H
#define _KMNG_BASE_ERROR_H
 
/*! \file error.h
    \brief This file containes all KMNG error codes.          
*/

#include "DX_VOS_BaseTypes.h"

#ifdef __cplusplus
extern "C"
{
#endif



/* success code */
#define KMNG_OK 										0

#define DX_SEP_KMNG_MODULE_ERROR_BASE			  0x0D000000

/* The error range number assigned to each module on its specified layer */
#define DX_KMNG_ERROR_MODULE_RANGE  0x00001000UL

#define KMNG_LAYER_ERROR_IDX  0UL
#define KMNG_LLF_LAYER_ERROR_IDX   1UL
/* KMNG module on the KMNG layer base address - 0x0D000000 */
#define DX_SEP_KMNG_KMNG_MODULE_ERROR_BASE  (DX_SEP_KMNG_MODULE_ERROR_BASE + \
										(DX_KMNG_ERROR_MODULE_RANGE * KMNG_LAYER_ERROR_IDX ) )  

/* LLF module on the KMNG layer base address - 0x0D001000 */
#define DX_SEP_KMNG_LLF_MODULE_ERROR_BASE  (DX_SEP_KMNG_MODULE_ERROR_BASE + \
										(DX_KMNG_ERROR_MODULE_RANGE * KMNG_LLF_LAYER_ERROR_IDX ) )  


#ifdef  __cplusplus
}
#endif


#endif /* ifndef ERROR */





