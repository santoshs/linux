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

 #include "VOS_API/DX_VOS_TimeUtils.h"
#include "VOS_API/DX_VOS_Time.h"
#include "VOS_API/DX_VOS_Mem.h"

/* Internal function no need for parameter check */
static DxStatus DX_VOS_2DigitsStrToInt(const DxUint8 *aString, DxUint32* val)
{
	if ((DxUint8)(aString[0]  - '0') > 9 || (DxUint8)(aString[1] - '0') > 9)
		RETURN_NEW_ERROR(DX_VOS_TIME_ERROR);
	*val = (aString[0]  - '0') * 10 + (aString[1] - '0');
	DX_RETURN(DX_SUCCESS);
}

static void DX_VOS_IntTo2DigitsStr(DxUint32 val, DxUint8 *aString)
{
	aString[1] = '0' + (DxUint8)(val % 10);
	aString[0] = '0' + (DxUint8)((val / 10) % 10);
}

DxStatus DX_VOS_GetUTCTime(DxUtcTime_t *aUtcTime)
{
    DX_DECLARE(DxStatus, result, DX_SUCCESS);
    result = DX_VOS_SecsToUTCTime(DX_VOS_GetTime(), aUtcTime);
    if (result != DX_SUCCESS)
	RETURN_OLD_ERROR(result);
    DX_RETURN(DX_SUCCESS);
}

DxStatus DX_VOS_GetGeneralizedTime(DxGeneralizedTime_t *aGenTime)
{
    DX_DECLARE(DxStatus, result, DX_SUCCESS);
    result = DX_VOS_SecsToGenTime(DX_VOS_GetTime(), aGenTime);
    if (result != DX_SUCCESS)
	RETURN_OLD_ERROR(result);
    DX_RETURN(DX_SUCCESS);
}

DxInt DX_VOS_UTCTimeCompare(const DxUtcTime_t *aUtc1, const DxUtcTime_t *aUtc2)
{
	if (aUtc1 == DX_NULL)
	{
		if (aUtc2 == DX_NULL)
			return 0;
		return -1;
	}
	if (aUtc2 == DX_NULL)
		return 1;
	if (aUtc1->year_m >= '5' && aUtc2->year_m < '5')
		return -1;
	if (aUtc2->year_m >= '5' && aUtc1->year_m < '5')
		return 1;

    // We don't compare Zulu & endOfStr
	return DX_VOS_MemCmp(aUtc1, aUtc2, sizeof(DxUtcTime_t) - 2);
}

DxInt DX_VOS_GenTimeCompare(const DxGeneralizedTime_t *aTm1, const DxGeneralizedTime_t *aTm2)
{
    // We don't compare Zulu & endOfStr
	return DX_VOS_MemCmp(aTm1, aTm2, sizeof(DxGeneralizedTime_t) - 2);
}

DxStatus DX_VOS_UTCTimeToSecs(const DxUtcTime_t* aUtcTime, DxTime_t* aTime)
{
	DX_DECLARE(DxStatus, result, DX_SUCCESS);
	DxTimeStruct_t timeStruct = {0};
    DX_ASSERT_PARAM(aUtcTime != DX_NULL);
    DX_ASSERT_PARAM(aTime != DX_NULL);

	result |= DX_VOS_2DigitsStrToInt(&(aUtcTime->seconds_m), &(timeStruct.tm_sec));
	result |= DX_VOS_2DigitsStrToInt(&(aUtcTime->minutes_m), &(timeStruct.tm_min));
	result |= DX_VOS_2DigitsStrToInt(&(aUtcTime->hours_m), &(timeStruct.tm_hour));
	result |= DX_VOS_2DigitsStrToInt(&(aUtcTime->day_m), &(timeStruct.tm_mday));
	result |= DX_VOS_2DigitsStrToInt(&(aUtcTime->months_m), &(timeStruct.tm_mon));
	result |= DX_VOS_2DigitsStrToInt(&(aUtcTime->year_m), &(timeStruct.tm_year));
	if (result != DX_SUCCESS)
	  RETURN_OLD_ERROR(result);

	if (timeStruct.tm_year < 50)  /* if year is 00-50 (i.e. 2000-2050) */
	    timeStruct.tm_year += 2000;
    else
        timeStruct.tm_year += 1900;

	result = DX_VOS_GlobalTimeToSecs(&timeStruct, aTime);
    if (result != DX_SUCCESS)
	RETURN_OLD_ERROR(result);

    DX_RETURN(DX_SUCCESS);
}


DxStatus DX_VOS_SecsToUTCTime(DxTime_t aTime, DxUtcTime_t* aUtcTime)
{
	DX_DECLARE(DxStatus, result, DX_SUCCESS) ;
	DxTimeStruct_t timeStruct;

    DX_ASSERT_PARAM(aUtcTime != DX_NULL);
    if (aTime > DX_MAX_INT32) // invalid time value
        RETURN_NEW_ERROR(DX_OVERFLOW);

	DX_VOS_MemSetZero(aUtcTime, sizeof(DxUtcTime_t));

	result = DX_VOS_GetGlobalTime(aTime, &timeStruct);
	if (result != DX_SUCCESS)
		RETURN_OLD_ERROR(result);

	DX_VOS_IntTo2DigitsStr(timeStruct.tm_year ,  &(aUtcTime->year_m));
	DX_VOS_IntTo2DigitsStr(timeStruct.tm_mon  ,  &(aUtcTime->months_m));
	DX_VOS_IntTo2DigitsStr(timeStruct.tm_mday ,  &(aUtcTime->day_m));      /* day of the month         - [1, 31] */
	DX_VOS_IntTo2DigitsStr(timeStruct.tm_hour ,  &(aUtcTime->hours_m));    /* hours after midnight     - [0, 23] */
	DX_VOS_IntTo2DigitsStr(timeStruct.tm_min  ,  &(aUtcTime->minutes_m));  /* minutes after the hour   - [0, 59] */
	DX_VOS_IntTo2DigitsStr(timeStruct.tm_sec  ,  &(aUtcTime->seconds_m));  /* seconds after the minute - [0, 59] */
	aUtcTime->Zulu = 'Z';
    aUtcTime->endOfStr = 0;
	DX_RETURN(DX_SUCCESS);
}

DxStatus DX_VOS_SecsToGenTime(DxTime_t aTime, DxGeneralizedTime_t* aGenTime)
{
	DX_DECLARE(DxStatus, result, DX_SUCCESS) ;
	DxTimeStruct_t timeStruct;

    DX_ASSERT_PARAM(aGenTime != DX_NULL);

    if (aTime > DX_MAX_INT32) // invalid time value
        RETURN_NEW_ERROR(DX_OVERFLOW);

	result = DX_VOS_GetGlobalTime(aTime, &timeStruct);
	if (result != DX_SUCCESS)
		RETURN_OLD_ERROR(result);


	DX_VOS_IntTo2DigitsStr(timeStruct.tm_year / 100,  &(aGenTime->year_m1));
	DX_VOS_IntTo2DigitsStr(timeStruct.tm_year ,  &(aGenTime->year_m2));
	DX_VOS_IntTo2DigitsStr(timeStruct.tm_mon  ,  &(aGenTime->months_m));
	DX_VOS_IntTo2DigitsStr(timeStruct.tm_mday ,  &(aGenTime->day_m));      /* day of the month         - [1, 31] */
	DX_VOS_IntTo2DigitsStr(timeStruct.tm_hour ,  &(aGenTime->hours_m));    /* hours after midnight     - [0, 23] */
	DX_VOS_IntTo2DigitsStr(timeStruct.tm_min  ,  &(aGenTime->minutes_m));  /* minutes after the hour   - [0, 59] */
	DX_VOS_IntTo2DigitsStr(timeStruct.tm_sec  ,  &(aGenTime->seconds_m));  /* seconds after the minute - [0, 59] */
	aGenTime->Zulu = 'Z';
    aGenTime->endOfStr = 0;
    DX_RETURN(DX_SUCCESS);
}

DxStatus DX_VOS_GenTimeToSecs(const DxGeneralizedTime_t* aGenTime, DxTime_t* aTime)
{
	DX_DECLARE(DxStatus, result, DX_SUCCESS);
	DxTimeStruct_t timeStruct;
	DxUint32 year_m = 0, year_l = 0;

    DX_ASSERT_PARAM(aGenTime != DX_NULL);
    DX_ASSERT_PARAM(aTime != DX_NULL);

	result |= DX_VOS_2DigitsStrToInt(&(aGenTime->seconds_m), &(timeStruct.tm_sec));
	result |= DX_VOS_2DigitsStrToInt(&(aGenTime->minutes_m), &(timeStruct.tm_min));
	result |= DX_VOS_2DigitsStrToInt(&(aGenTime->hours_m), &(timeStruct.tm_hour));
	result |= DX_VOS_2DigitsStrToInt(&(aGenTime->day_m), &(timeStruct.tm_mday));
	result |= DX_VOS_2DigitsStrToInt(&(aGenTime->months_m), &(timeStruct.tm_mon));
	result |= DX_VOS_2DigitsStrToInt(&(aGenTime->year_m1), &(year_m));
	result |= DX_VOS_2DigitsStrToInt(&(aGenTime->year_m2), &(year_l));
	if (result != DX_SUCCESS)
		RETURN_OLD_ERROR(result);

	timeStruct.tm_year = year_m * 100 + year_l;

    result = DX_VOS_GlobalTimeToSecs(&timeStruct, aTime);
    if (result != DX_SUCCESS)
	RETURN_OLD_ERROR(result);
    DX_RETURN(DX_SUCCESS);
}



#define GEN_TIME_STRING_LENGTH 15 // Lenght not including null termination
#define UTC_TIME_STRING_LENGTH 13 // Lenght not including null termination

DxStatus DX_VOS_InitGenTimeFromMem(const void* timeStr, DxUint32 length, DxGeneralizedTime_t* aGenTime)
{
	const DxChar* current = DX_NULL;

	DX_ASSERT_PARAM(timeStr != DX_NULL);
	DX_ASSERT_PARAM(aGenTime != DX_NULL);

	current = (const DxChar*)timeStr;

	if (length == GEN_TIME_STRING_LENGTH)
	{
		aGenTime->year_m1 = *(current++);
		aGenTime->year_l1 = *(current++);
	}
	else if (length == UTC_TIME_STRING_LENGTH)
	{
		if ((*current)>= '5')
		{ // years under 50 are considered in the 2000s and years greater than 50 are considered in the 1900s
			aGenTime->year_m1 = '1';
			aGenTime->year_l1 = '9';
		}else
		{
			aGenTime->year_m1 = '2';
			aGenTime->year_l1 = '0';
		}
	}
	else
		RETURN_CONST_STATUS(DX_BAD_ARGUMENTS);

	aGenTime->year_m2 = *(current++);
	aGenTime->year_l2 = *(current++);
	aGenTime->months_m = *(current++);
	aGenTime->months_l = *(current++);
	aGenTime->day_m = *(current++);
	aGenTime->day_l = *(current++);
	aGenTime->hours_m = *(current++);
	aGenTime->hours_l = *(current++);
	aGenTime->minutes_m = *(current++);
	aGenTime->minutes_l = *(current++);
	aGenTime->seconds_m = *(current++);
	aGenTime->seconds_l = *(current++);
	aGenTime->Zulu = *(current++);
	aGenTime->endOfStr = 0;

	DX_RETURN(DX_SUCCESS);
}


DxStatus DX_VOS_InitUtcTimeFromMem(const void* timeStr, DxUint32 length, DxUtcTime_t* aUtcTime)
{
	const DxChar* current = DX_NULL;

	DX_ASSERT_PARAM(timeStr != DX_NULL);
	DX_ASSERT_PARAM(aUtcTime != DX_NULL);
    DX_ASSERT_PARAM(length == 14);
	current = (const DxChar*)timeStr;

	aUtcTime->year_m = *(current++);
	aUtcTime->year_l = *(current++);
	aUtcTime->months_m = *(current++);
	aUtcTime->months_l = *(current++);
	aUtcTime->day_m = *(current++);
	aUtcTime->day_l = *(current++);
	aUtcTime->hours_m = *(current++);
	aUtcTime->hours_l = *(current++);
	aUtcTime->minutes_m = *(current++);
	aUtcTime->minutes_l = *(current++);
	aUtcTime->seconds_m = *(current++);
	aUtcTime->seconds_l = *(current++);
	aUtcTime->Zulu = *(current++);
	aUtcTime->endOfStr = 0;

	DX_RETURN(DX_SUCCESS);
}

#ifdef DX_USE_LEGACY_VOS

DxInt DX_VOS_DRM_UTCTimeCompare(const DxUtcTime_t *aUtc1, const DxUtcTime_t *aUtc2)
{
    DxInt compare = DX_VOS_UTCTimeCompare(aUtc1,aUtc2);
    if (compare > 0)
        DX_RETURN(TIME_1_IS_AFTER);
    else if (compare < 0)
        DX_RETURN(TIME_2_IS_AFTER);
    else
        DX_RETURN(IDENTICAL_TIME);
}

DxInt DX_VOS_DRM_generalizedTimeCompare(const DxGeneralizedTime_t *aTm1, const DxGeneralizedTime_t *aTm2)
{
    DxInt compare = DX_VOS_GenTimeCompare(aTm1,aTm2);
    if (compare > 0)
        DX_RETURN(TIME_1_IS_AFTER);
    else if (compare < 0)
        DX_RETURN(TIME_2_IS_AFTER);
    else
        DX_RETURN(IDENTICAL_TIME);
}

#endif
