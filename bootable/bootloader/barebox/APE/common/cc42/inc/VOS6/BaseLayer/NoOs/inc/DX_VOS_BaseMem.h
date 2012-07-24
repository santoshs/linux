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
 
 #ifndef _DX_VOS_BASE_MEM_H_
#define _DX_VOS_BASE_MEM_H_

/* WIP_NoOS_R_Loader */
/*#include <stdlib.h>*/
#include "string.h"

/*! Allocates memory buffer with the specified size.
	Function signature is the same as DX_VOS_MemMalloc.
	Function may assume that size is not zero. */
/* WIP_NoOS_R_Loader */
/* #define DX_VOS_BaseMemMalloc malloc */
/*! Frees the specified memory buffer.
Function signature is the same as DX_VOS_MemFree.
Function may assume that the pointer is not NULL. */
/* WIP_NoOS_R_Loader */
/* #define DX_VOS_BaseMemFree free */

/*! Reallocates buffer with specified new size. The content of the old buffer
is moved to the new location.
\return Allocated buffer or NULL if allocation failed.
if allocation failed the original buffer can be used.
Function signature is the same as DX_VOS_MemRealloc.
Function may assume that size is not zero. */
/* WIP_NoOS_R_Loader */
/* #define DX_VOS_BaseMemRealloc realloc */

/*! Copies source buffer to target buffer.
   Function receives 3 parameters: target, source and size(number of bytes to copy).
   Function may assume that size is bigger than zero and that buffer pointers are not NULL. 
   \note Overlapping ranges MUST be supported.
   */
#define DX_VOS_BaseMemCopy	memmove

/*!	Fills buffer with specific value. 
	Function signature is the same as DX_VOS_MemSet.
	Function may assume that size is bigger that zero and that the pointer to buffer is 
	not NULL.
*/
#define DX_VOS_BaseMemSet memset

/*!	Fills buffer with Zeros. 
Function signature is the same as DX_VOS_MemSetZero.
Function may assume that size is bigger that zero and that the pointer to buffer is 
not NULL.
*/
#define DX_VOS_BaseMemSetZero(aTarget,aSize)	DX_VOS_BaseMemSet(aTarget, 0, aSize)

/*! Compares buffers (source buffer with target buffer).
	Function signature is the same as DX_VOS_MemCmp.
	\note Function may assume that size is bigger that zero and that the pointers to buffers
	are not NULL.
*/
#define DX_VOS_BaseMemCmp memcmp
#endif
