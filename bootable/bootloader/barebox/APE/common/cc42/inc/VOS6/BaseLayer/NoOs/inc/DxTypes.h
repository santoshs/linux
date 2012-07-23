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
 
 #ifndef DX_TYPES_H
#define DX_TYPES_H

#define DX_NULL 				0

typedef unsigned int       		DxUint;
typedef unsigned char        	DxUint8;
typedef unsigned short       	DxUint16;
typedef unsigned long        	DxUint32;
// at MW request if flag is set then DxUin64 is defined as long (for platforms with no 64bit support)
#ifndef DX_LONG_LONG_NOT_SUPPORTED 
typedef unsigned long long		DxUint64;
#else 
typedef unsigned long       		DxUint64;
#endif
typedef int                   	DxInt;
typedef char		          	DxInt8;
typedef short               	DxInt16;
typedef long          	        DxInt32;
#ifndef DX_LONG_LONG_NOT_SUPPORTED
typedef long long				DxInt64;
#else 
typedef  long       				DxInt64;
#endif
typedef float			        DxFloat;

typedef char			        DxChar;
typedef DxUint16 				DxWideChar;
typedef DxUint32				DxWideChar32;

typedef DxUint8                 DxByte ;

typedef enum {
    DX_FALSE = 0,
    DX_TRUE = 1
} EDxBool;

typedef DxUint32                DxBool;
typedef DxUint32	 		    DxStatus;

#define DX_SUCCESS			    0UL 
#define DX_INFINITE			    0xFFFFFFFF

#endif
