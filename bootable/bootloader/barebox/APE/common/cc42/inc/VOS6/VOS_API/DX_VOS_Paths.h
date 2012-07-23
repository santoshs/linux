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
 
 #ifndef DX_VOS_PATHS_H
#define DX_VOS_PATHS_H

#include "DX_VOS_BaseTypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

const DxChar* DX_VOS_RootPath(void);
const DxChar* DX_VOS_RomRootPath(void);
const DxChar* DX_VOS_PublicRootPath(void);
const DxChar* DX_VOS_DefaultRootDrive(void);

const DxChar* DX_VOS_PrefixRootPath(const DxChar* apPath);
const DxChar* DX_VOS_PrefixRomRootPath(const DxChar* apPath);
const DxChar* DX_VOS_PrefixPublicRootPath(const DxChar* apPath);
const DxChar* DX_VOS_PrefixDefaultRootDrive(const DxChar* apPath);

DxChar* DX_VOS_ExtendPrefixRootPath(const DxChar* apPostfix, DxChar* apOutputPath, DxUint32 aBuffSize);
DxChar* DX_VOS_ExtendPrefixRomRootPath(const DxChar* apPostfix, DxChar* apOutputPath, DxUint32 aBuffSize);
DxChar* DX_VOS_ExtendPrefixPublicRootPath(const DxChar* apPostfix, DxChar* apOutputPath, DxUint32 aBuffSize);
DxChar* DX_VOS_ExtendPrefixDefaultRootDrive(const DxChar* apPostfix, DxChar* apOutputPath, DxUint32 aBuffSize);

#ifdef __cplusplus
}
#endif

#endif

