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

#include "DX_VOS_Sem.h"
DxStatus DX_VOS_SemCreate (DxVosSem  *aSemId, DxUint_t  aInitialValue, const DxChar* semName)
{
	/* semName is deliberately ignored because in Windows OS calling CreateSemaphore
	   with the name of existing semaphore returns a handle to the existing semaphore
	   instead of creating a new one */
	semName = semName;
	DX_ASSERT_PARAM(aSemId != DX_NULL);

	*aSemId = (DxVosSem)1;
	DX_RETURN(DX_SUCCESS);
}

DxStatus DX_VOS_SemDelete (DxVosSem  aSemId)
{
    return DX_VOS_SemDestroy(&aSemId);
}

DxStatus DX_VOS_SemDestroy (DxVosSem*  aSemId)
{
    if (aSemId == DX_NULL || *aSemId == DX_NULL)
		DX_RETURN(DX_SUCCESS);

    *aSemId = DX_NULL;
	DX_RETURN(DX_SUCCESS);
}

DxStatus DX_VOS_SemWait(DxVosSem aSemId, DxUint32_t aTimeout)
{
	DX_RETURN(DX_SUCCESS);
}

DxStatus DX_VOS_SemGive(DxVosSem aSemId)
{
	DX_RETURN(DX_SUCCESS);
}
