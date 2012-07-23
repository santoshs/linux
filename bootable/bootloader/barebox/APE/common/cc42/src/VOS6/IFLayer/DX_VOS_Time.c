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

 #include "VOS_API/DX_VOS_Time.h"
#include "VOS_API/DX_VOS_Stdio.h"

DxStatus DX_VOS_GetTimeStamp(DxChar* timeStamp, DxUint32 timeStampSize)
{
    DX_DECLARE(DxStatus, result, DX_SUCCESS);
    DxTimeStruct_t timeStampStruct;

    result = DX_VOS_GetLocalTime(DX_VOS_GetTime(), &timeStampStruct);
    if (result != DX_SUCCESS)
        RETURN_OLD_ERROR(result);

    result = DX_VOS_SPrintf(timeStamp, timeStampSize, "%04d%02d%02d_%02d%02d%02d_%08X",
        timeStampStruct.tm_year, timeStampStruct.tm_mon, timeStampStruct.tm_mday,
        timeStampStruct.tm_hour, timeStampStruct.tm_min, timeStampStruct.tm_sec, DX_VOS_GetTickCount());
    if (result != DX_SUCCESS)
        RETURN_OLD_ERROR(result);
    DX_RETURN(DX_SUCCESS);
}

int DX_VOS_TimeCompare(const DxTimeStruct_t* time1, const DxTimeStruct_t* time2)
{
    if (time1->tm_year < time2->tm_year) return -1;
    if (time1->tm_year > time2->tm_year) return 1;
    if (time1->tm_mon < time2->tm_mon) return -1;
    if (time1->tm_mon > time2->tm_mon) return 1;
    if (time1->tm_mday < time2->tm_mday) return -1;
    if (time1->tm_mday > time2->tm_mday) return 1;
    if (time1->tm_hour < time2->tm_hour) return -1;
    if (time1->tm_hour > time2->tm_hour) return 1;
    if (time1->tm_min < time2->tm_min) return -1;
    if (time1->tm_min > time2->tm_min) return 1;
    if (time1->tm_sec < time2->tm_sec) return -1;
    if (time1->tm_sec > time2->tm_sec) return 1;

    return 0;
}

DxStatus DX_VOS_SafeGlobalTimeToSecs(const DxTimeStruct_t* aTimeStruct, DxTime_t* aTime)
{
	DX_DECLARE(DxStatus, result, DX_SUCCESS);

	DX_ASSERT_PARAM(aTimeStruct != DX_NULL);
	DX_ASSERT_PARAM(aTime != DX_NULL);

	if (aTimeStruct->tm_year < 1970)
		*aTime = 0;
	else if (aTimeStruct->tm_year > 2037)
		*aTime = DX_INFINITE;
	else
	{
		result = DX_VOS_GlobalTimeToSecs(aTimeStruct, aTime);
		if (result != DX_SUCCESS)
			RETURN_VAR_STATUS(result);
	}

	DX_RETURN(DX_SUCCESS);
}
