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

 #include "VOS_API/DX_VOS_Stdio.h"
#include "VOS_API/DX_VOS_Mem.h"
#include <stdio.h>

IMPORT_C void  DX_VOS_PrintString(const DxChar *aStr)
{
	if (aStr != DX_NULL)
		printf("%s",aStr);
}

IMPORT_C DxInt_t DX_VOS_GetCh(void)
{
    return getchar();
}

//FILE originalStdOut;
DxBool_t redirected = DX_FALSE;
DxStatus DX_VOS_RedirectStdoutToFile(const DxChar* fileName)
{
    if (redirected == DX_FALSE)
    {
        //originalStdOut = *stdout;
        redirected = DX_TRUE;
        //DX_VOS_MemSet(stdout,0,sizeof(FILE));
    }
    /*
    if (freopen(fileName, "wb", stdout) == 0)
        RETURN_CONST_STATUS(DX_VOS_FILE_ERROR);
	*/
	DX_RETURN(DX_SUCCESS);
}

void DX_VOS_CancelStdOutRedirection(void)
{
    if (redirected)
    {
        //fclose(stdout);
        //*stdout = originalStdOut;
        redirected = DX_FALSE;
    }
}

DxStatus DX_VOS_UpdateStdout(void)
{
    //RETURN_VAR_STATUS(DX_VOS_FFlush((DxVosFile)stdout));
	DX_RETURN(DX_SUCCESS);
}
