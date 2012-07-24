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
 
 #ifndef DX_VOS_version_H
#define DX_VOS_version_H

#include "DX_VOS_BaseTypes.h"

#ifdef __cplusplus
extern "C"
{
#endif


/* the DX_VOS API Version release definitions */
#define DX_VOS_API_MAJOR_VERSION_NUM     6
#define DX_VOS_API_MINOR_VERSION_NUM     2
#define DX_VOS_API_SUB_VERSION_NUM       0
#define DX_VOS_API_INTERNAL_VERSION_NUM  0

#define DX_VOS_API_NAME  "VOS API"


typedef struct
{
   DxInt8 m_compName[10];
   DxUint32 m_major;
   DxUint32 m_minor;
   DxUint32 m_sub;
   DxUint32 m_internal;   
}DX_VOS_ComponentVersion_t;

typedef struct
{
   DX_VOS_ComponentVersion_t DX_VOS_API_Version;
   DX_VOS_ComponentVersion_t DX_VOS_Platform_Version;
}DX_VOS_Version_t;


/* Returns the DX_VOS version.
 The version contains the following fields:
 - compName - A string describing the nature of the release.
 - major, minor, sub, internal - the release digits.
 The version include two components: API and OS dependent implementation.
 each component have its own version. 
 */

IMPORT_C void DX_VOS_GetVersion(DX_VOS_Version_t *version);

#ifdef DX_USE_LEGACY_VOS
#define compName	m_compName
#define major		m_major
#define minor		m_minor
#define sub			m_sub
#define internal	m_internal
#endif

#ifdef __cplusplus
}
#endif

#endif


