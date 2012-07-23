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

 #include "DX_VOS_Memmap.h"


DxStatus DX_VOS_MemMap(DxUint32 aPhysicalAddress, DxUint32 aMapSize, DxUint32 *aVirtualAddr)
{
	aMapSize = aMapSize;

	DX_ASSERT_PARAM(aVirtualAddr != DX_NULL);
       FVOS_Mmap(aVirtualAddr);
//	*aVirtualAddr=aPhysicalAddress;
	DX_RETURN(DX_SUCCESS);
}

DxStatus DX_VOS_MemUnMap(void *aVirtualAddr, DxUint32 aMapSize)
{
    DX_ASSERT_PARAM(DX_VALID_PTR_DATA(aVirtualAddr,aMapSize));
    DX_UNREFERENCED(aVirtualAddr);
    DX_UNREFERENCED(aMapSize);

	DX_RETURN(DX_SUCCESS);
}

DxStatus DX_VOS_MemVirtualToPhysical(void *aVirtualAddr, DxUint32 aMapSize, DX_PhysicalAddr_t *aPhysicalAdrr)
{
    DX_ASSERT_PARAM(aVirtualAddr != DX_NULL);
    DX_ASSERT_PARAM(aMapSize != 0);
    DX_ASSERT_PARAM(aPhysicalAdrr != DX_NULL);

    aPhysicalAdrr->nEntries = 1;
    aPhysicalAdrr->entry_addr[0] = (DxUint32*) aVirtualAddr;
    aPhysicalAdrr->entrySize[0] = aMapSize;

    DX_RETURN(DX_SUCCESS);
}
