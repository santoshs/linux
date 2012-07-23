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

 //#include <time.h>
#include "VOS_API/DX_VOS_Time.h"
#include "VOS_API/DX_VOS_Mem.h"

#define TIME_ZONE_DELTA		7200
DxTime_t DX_VOS_GetTime()
{
	return (DxTime_t) time(DX_NULL);
//	return 0;
}

DxStatus DX_VOS_GetLocalTime(DxTime_t aTime, DxTimeStruct_t* aTimeStruct)
{
    DxUint32_t numOfDays, numOfLeepYears, deltaYear, roundMonthDays, leepYear, numOfDaysSinseStartOfYear;

    /* Compute seconds */
    aTimeStruct->tm_sec  = aTime%60;

    /* Compute minutes */
    aTimeStruct->tm_min  = ((aTime - aTime%60)/60)%60;

    /* Compute hours */
    aTimeStruct->tm_hour = ((aTime - (aTimeStruct->tm_sec + aTimeStruct->tm_min*60))/3600)%24;

    /* Compute number of day passed since 01/01/1970 */
    numOfDays       = (aTime - (aTimeStruct->tm_sec + aTimeStruct->tm_min*60 + aTimeStruct->tm_hour*60*60))/86400 + 1;

    /* Compute number of day passed since 01/01/1972 */
    numOfDays -= (365*2);

    /* Compute number of leap years since 1972 include 1972 */
    numOfLeepYears  = (numOfDays / (365*3 + 366))+1;

    /* Check margins  */
    if (((numOfDays-numOfLeepYears)%365)!=0)
    {
        /* Compute number of years passed since 01/01/1970 */
        deltaYear       = (numOfDays-numOfLeepYears)/365 + 2;

        /* Compute number of years passed start of the year */
        numOfDays      = numOfDays - ((deltaYear-2)*365 + numOfLeepYears);

        /* Compute number of year */
        aTimeStruct->tm_year = 1970 + deltaYear;

        /* Update the number of days */
        if (!(aTimeStruct->tm_year%4))
            numOfDays++;
    }
    else
    {
        /* Compute number of years passed since 01/01/1970 */
        deltaYear       = (numOfDays-numOfLeepYears)/365 + 1;

        /* Compute number of years passed start of the year */
        numOfDays       = numOfDays - ((deltaYear-2)*365 + numOfLeepYears) + 1;

        /* Compute number of year */
        aTimeStruct->tm_year = 1970 + deltaYear;

        /* Update the number of days */
        if ((aTimeStruct->tm_year%4))
            numOfDays--;
    }

    roundMonthDays = 0;

    numOfDaysSinseStartOfYear = numOfDays;

    /* January */
    if((numOfDays >= 1) && (numOfDays <= 31))
    {
        aTimeStruct->tm_mon  = 1;
        aTimeStruct->tm_mday = numOfDays;
        return DX_SUCCESS;
    }
    roundMonthDays += 31;
    numOfDays = numOfDaysSinseStartOfYear - roundMonthDays;

    /* February */
    if(!(aTimeStruct->tm_year%4))
    {
        leepYear = 1;
    }
    else
    {
        leepYear = 0;
    }
    if((numOfDays >= 1) && (numOfDays <= 28 + leepYear))
    {
        aTimeStruct->tm_mon  = 2;
        aTimeStruct->tm_mday = numOfDays;
        return DX_SUCCESS;
    }
    roundMonthDays += 28 + leepYear;
    numOfDays = numOfDaysSinseStartOfYear - roundMonthDays;

    /* March */
    if((numOfDays >= 1) && (numOfDays <= 31))
    {
        aTimeStruct->tm_mon  = 3;
        aTimeStruct->tm_mday = numOfDays;
        return DX_SUCCESS;
    }
    roundMonthDays += 31;
    numOfDays = numOfDaysSinseStartOfYear - roundMonthDays;

    /* April */
    if((numOfDays >= 1) && (numOfDays <= 30))
    {
        aTimeStruct->tm_mon  = 4;
        aTimeStruct->tm_mday = numOfDays;
        return DX_SUCCESS;
    }
    roundMonthDays += 30;
    numOfDays = numOfDaysSinseStartOfYear - roundMonthDays;

    /* May */

    if((numOfDays >= 1) && (numOfDays <= 31))
    {
        aTimeStruct->tm_mon  = 5;
        aTimeStruct->tm_mday = numOfDays;
        return DX_SUCCESS;
    }
    roundMonthDays += 31;
    numOfDays = numOfDaysSinseStartOfYear - roundMonthDays;

    /* June */
    if((numOfDays >= 1) && (numOfDays <= 30))
    {
        aTimeStruct->tm_mon  = 6;
        aTimeStruct->tm_mday = numOfDays;
        return DX_SUCCESS;
    }
    roundMonthDays += 30;
    numOfDays = numOfDaysSinseStartOfYear - roundMonthDays;

    /* July */
    if((numOfDays >= 1) && (numOfDays <= 31))
    {
        aTimeStruct->tm_mon  = 7;
        aTimeStruct->tm_mday = numOfDays;
        return DX_SUCCESS;
    }
    roundMonthDays += 31;
    numOfDays = numOfDaysSinseStartOfYear - roundMonthDays;

    /* August */
    if((numOfDays >= 1) && (numOfDays <= 31))
    {
        aTimeStruct->tm_mon  = 8;
        aTimeStruct->tm_mday = numOfDays;
        return DX_SUCCESS;
    }
    roundMonthDays += 31;
    numOfDays = numOfDaysSinseStartOfYear - roundMonthDays;

    /* September */
    if((numOfDays >= 1) && (numOfDays <= 30))
    {
        aTimeStruct->tm_mon  = 9;
        aTimeStruct->tm_mday = numOfDays;
        return DX_SUCCESS;
    }
    roundMonthDays += 30;
    numOfDays = numOfDaysSinseStartOfYear - roundMonthDays;

    /* October */
    if((numOfDays >= 1) && (numOfDays <= 31))
    {
        aTimeStruct->tm_mon  = 10;
        aTimeStruct->tm_mday = numOfDays;
        return DX_SUCCESS;
    }
    roundMonthDays += 31;
    numOfDays = numOfDaysSinseStartOfYear - roundMonthDays;

    /* November */
    if((numOfDays >= 1) && (numOfDays <= 30))
    {
        aTimeStruct->tm_mon  = 11;
        aTimeStruct->tm_mday = numOfDays;
        return DX_SUCCESS;
    }
    roundMonthDays += 30;
    numOfDays = numOfDaysSinseStartOfYear - roundMonthDays;

    /* December */
    if((numOfDays >= 1) && (numOfDays <= 31))
    {
        aTimeStruct->tm_mon  = 12;
        aTimeStruct->tm_mday = numOfDays;
        return DX_SUCCESS;
    }

    /* If we got here -> failure */
    return DX_FAILURE;
}




DxStatus DX_VOS_GetGlobalTime(DxTime_t aTime, DxTimeStruct_t* aTimeStruct)

{
    DxUint32_t numOfDays, numOfLeepYears, deltaYear, roundMonthDays, leepYear, numOfDaysSinseStartOfYear;

    /* Compute seconds */
    aTimeStruct->tm_sec  = aTime%60;

    /* Compute minutes */
    aTimeStruct->tm_min  = ((aTime - aTime%60)/60)%60;

    /* Compute hours */
    aTimeStruct->tm_hour = ((aTime - (aTimeStruct->tm_sec + aTimeStruct->tm_min*60))/3600)%24;

    /* Compute number of day passed since 01/01/1970 */
    numOfDays       = (aTime - (aTimeStruct->tm_sec + aTimeStruct->tm_min*60 + aTimeStruct->tm_hour*60*60))/86400 + 1;

    /* Compute number of day passed since 01/01/1972 */
    numOfDays -= (365*2);

    /* Compute number of leap years since 1972 include 1972 */
    numOfLeepYears  = (numOfDays / (365*3 + 366))+1;

    /* Check margins  */
    if (((numOfDays-numOfLeepYears)%365)!=0)
    {
        /* Compute number of years passed since 01/01/1970 */
        deltaYear       = (numOfDays-numOfLeepYears)/365 + 2;

        /* Compute number of years passed start of the year */
        numOfDays      = numOfDays - ((deltaYear-2)*365 + numOfLeepYears);

        /* Compute number of year */
        aTimeStruct->tm_year = 1970 + deltaYear;

        /* Update the number of days */
        if (!(aTimeStruct->tm_year%4))
            numOfDays++;
    }
    else
    {
        /* Compute number of years passed since 01/01/1970 */
        deltaYear       = (numOfDays-numOfLeepYears)/365 + 1;

        /* Compute number of years passed start of the year */
        numOfDays       = numOfDays - ((deltaYear-2)*365 + numOfLeepYears) + 1;

        /* Compute number of year */
        aTimeStruct->tm_year = 1970 + deltaYear;

        /* Update the number of days */
        if ((aTimeStruct->tm_year%4))
            numOfDays--;
    }

    roundMonthDays = 0;

    numOfDaysSinseStartOfYear = numOfDays;

    /* January */
    if((numOfDays >= 1) && (numOfDays <= 31))
    {
        aTimeStruct->tm_mon  = 1;
        aTimeStruct->tm_mday = numOfDays;
        return DX_SUCCESS;
    }
    roundMonthDays += 31;
    numOfDays = numOfDaysSinseStartOfYear - roundMonthDays;

    /* February */
    if(!(aTimeStruct->tm_year%4))
    {
        leepYear = 1;
    }
    else
    {
        leepYear = 0;
    }
    if((numOfDays >= 1) && (numOfDays <= 28 + leepYear))
    {
        aTimeStruct->tm_mon  = 2;
        aTimeStruct->tm_mday = numOfDays;
        return DX_SUCCESS;
    }
    roundMonthDays += 28 + leepYear;
    numOfDays = numOfDaysSinseStartOfYear - roundMonthDays;

    /* March */
    if((numOfDays >= 1) && (numOfDays <= 31))
    {
        aTimeStruct->tm_mon  = 3;
        aTimeStruct->tm_mday = numOfDays;
        return DX_SUCCESS;
    }
    roundMonthDays += 31;
    numOfDays = numOfDaysSinseStartOfYear - roundMonthDays;

    /* April */
    if((numOfDays >= 1) && (numOfDays <= 30))
    {
        aTimeStruct->tm_mon  = 4;
        aTimeStruct->tm_mday = numOfDays;
        return DX_SUCCESS;
    }
    roundMonthDays += 30;
    numOfDays = numOfDaysSinseStartOfYear - roundMonthDays;

    /* May */

    if((numOfDays >= 1) && (numOfDays <= 31))
    {
        aTimeStruct->tm_mon  = 5;
        aTimeStruct->tm_mday = numOfDays;
        return DX_SUCCESS;
    }
    roundMonthDays += 31;
    numOfDays = numOfDaysSinseStartOfYear - roundMonthDays;

    /* June */
    if((numOfDays >= 1) && (numOfDays <= 30))
    {
        aTimeStruct->tm_mon  = 6;
        aTimeStruct->tm_mday = numOfDays;
        return DX_SUCCESS;
    }
    roundMonthDays += 30;
    numOfDays = numOfDaysSinseStartOfYear - roundMonthDays;

    /* July */
    if((numOfDays >= 1) && (numOfDays <= 31))
    {
        aTimeStruct->tm_mon  = 7;
        aTimeStruct->tm_mday = numOfDays;
        return DX_SUCCESS;
    }
    roundMonthDays += 31;
    numOfDays = numOfDaysSinseStartOfYear - roundMonthDays;

    /* August */
    if((numOfDays >= 1) && (numOfDays <= 31))
    {
        aTimeStruct->tm_mon  = 8;
        aTimeStruct->tm_mday = numOfDays;
        return DX_SUCCESS;
    }
    roundMonthDays += 31;
    numOfDays = numOfDaysSinseStartOfYear - roundMonthDays;

    /* September */
    if((numOfDays >= 1) && (numOfDays <= 30))
    {
        aTimeStruct->tm_mon  = 9;
        aTimeStruct->tm_mday = numOfDays;
        return DX_SUCCESS;
    }
    roundMonthDays += 30;
    numOfDays = numOfDaysSinseStartOfYear - roundMonthDays;

    /* October */
    if((numOfDays >= 1) && (numOfDays <= 31))
    {
        aTimeStruct->tm_mon  = 10;
        aTimeStruct->tm_mday = numOfDays;
        return DX_SUCCESS;
    }
    roundMonthDays += 31;
    numOfDays = numOfDaysSinseStartOfYear - roundMonthDays;

    /* November */
    if((numOfDays >= 1) && (numOfDays <= 30))
    {
        aTimeStruct->tm_mon  = 11;
        aTimeStruct->tm_mday = numOfDays;
        return DX_SUCCESS;
    }
    roundMonthDays += 30;
    numOfDays = numOfDaysSinseStartOfYear - roundMonthDays;

    /* December */
    if((numOfDays >= 1) && (numOfDays <= 31))
    {
        aTimeStruct->tm_mon  = 12;
        aTimeStruct->tm_mday = numOfDays;
        return DX_SUCCESS;
    }

    /* If we got here -> failure */
    return DX_FAILURE;
}


/* warning:  calculations as implemented by Ira, not tested fully*/
DxStatus DX_VOS_GlobalTimeToSecs(const DxTimeStruct_t* aTimeStruct, DxTime_t* aTime)
{
    DxUint32_t leapYearFlag;
    DxUint32_t timeInSeconds = 0;
    DxUint32_t numOfDays = 0;
    DxUint32_t numOfLeapYears;

	// struct tm globalTime = {0};
	if (aTimeStruct == DX_NULL)
		RETURN_CONST_STATUS(DX_BAD_ARGUMENTS);

	if (aTime == DX_NULL)
		RETURN_CONST_STATUS(DX_BAD_ARGUMENTS);

    if (aTimeStruct->tm_hour > 23 ||
        aTimeStruct->tm_min > 59 ||
        aTimeStruct->tm_sec > 59 ||
        aTimeStruct->tm_year < 1900 ||
        aTimeStruct->tm_mon < 1 ||
        aTimeStruct->tm_mon > 12 ||
        aTimeStruct->tm_mday < 1 ||
        aTimeStruct->tm_mday > 31
        )
        RETURN_CONST_STATUS(DX_BAD_ARGUMENTS);


     /************************************************************************/
     /* Time1  calculations                                                  */
     /************************************************************************/
     /* seconds passed in the day*/
     timeInSeconds  = aTimeStruct->tm_sec +
                      aTimeStruct->tm_min * 60 +
                      aTimeStruct->tm_hour * 60 * 60;

     if (aTimeStruct->tm_year%4)
         leapYearFlag = 0;
     else
         leapYearFlag = 1;

     /* Count days from start of the year */
     switch(aTimeStruct->tm_mon)
     {
     case 1:
         numOfDays = aTimeStruct->tm_mday;
     break;
     case 2:
         numOfDays = 31 + /* January */
                     aTimeStruct->tm_mday;
     break;
     case 3:
         numOfDays = 31 +                 /* January */
                     28 + leapYearFlag +  /* February */
                     aTimeStruct->tm_mday;
         break;
     case 4:
         numOfDays = 31 +                  /* January */
                     28 + leapYearFlag +   /* February */
                     31 +                  /* March */
                     aTimeStruct->tm_mday;
         break;
     case 5:
         numOfDays = 31 +                  /* January. */
                     28 + leapYearFlag +   /* February */
                     31 +                  /* March */
                     30 +                  /* April */
                     aTimeStruct->tm_mday;
      break;
     case 6:
         numOfDays = 31 +                  /* January. */
                     28 + leapYearFlag +   /* February */
                     31 +                  /* March */
                     30 +                  /* April */
                     31 +                  /* May */
                     aTimeStruct->tm_mday;
     break;
     case 7:
         numOfDays = 31 +                  /* January. */
                     28 + leapYearFlag +   /* February */
                     31 +                  /* March */
                     30 +                  /* April */
                     31 +                  /* May */
                     30 +                  /* Jun */
                     aTimeStruct->tm_mday;
         break;
     case 8:
         numOfDays = 31 +                  /* January. */
                     28 + leapYearFlag +   /* February */
                     31 +                  /* March */
                     30 +                  /* April */
                     31 +                  /* May */
                     30 +                  /* June */
                     31 +                  /* July */
                     aTimeStruct->tm_mday;
         break;
     case 9:
         numOfDays = 31 +                  /* January. */
                     28 + leapYearFlag +   /* February */
                     31 +                  /* March */
                     30 +                  /* April */
                     31 +                  /* May */
                     30 +                  /* June */
                     31 +                  /* July */
                     31 +                  /* August */
                     aTimeStruct->tm_mday;
         break;
     case 10:
         numOfDays = 31 +                  /* January. */
                     28 + leapYearFlag +   /* February */
                     31 +                  /* March */
                     30 +                  /* April */
                     31 +                  /* May */
                     30 +                  /* June */
                     31 +                  /* July */
                     31 +                  /* August */
                     30 +                  /* September */
                     aTimeStruct->tm_mday;
         break;
     case 11:
         numOfDays = 31 +                  /* January. */
                     28 + leapYearFlag +   /* February */
                     31 +                  /* March */
                     30 +                  /* April */
                     31 +                  /* May */
                     30 +                  /* June */
                     31 +                  /* July */
                     31 +                  /* August */
                     30 +                  /* September */
                     31 +                  /* October */
                     aTimeStruct->tm_mday;
         break;
     case 12:
         numOfDays = 31 +                  /* January. */
                     28 + leapYearFlag +   /* February */
                     31 +                  /* March */
                     30 +                  /* April */
                     31 +                  /* May */
                     30 +                  /* June */
                     31 +                  /* July */
                     31 +                  /* August */
                     30 +                  /* September */
                     31 +                  /* October */
                     30 +                  /* November */
                     aTimeStruct->tm_mday;
         break;
     default:
         break;
     }
     /* Count a number of leap years from 1970 */
     numOfLeapYears = (aTimeStruct->tm_year - 1970) / 4;

     timeInSeconds =  timeInSeconds +                   /* Seconds in the current day */
                      numOfDays * 60 * 60 * 24 +        /* Number of seconds since beginning of the year */
                     (aTimeStruct->tm_year - 1970) *    /* Number of seconds in all years since 1970 */
                      60 * 60 * 24 * 365 +
                      numOfLeapYears * 60 * 60 * 24;

     *aTime = timeInSeconds;


	DX_RETURN(DX_SUCCESS);
}


DxUint32_t DX_VOS_GetTickCount()
{
	return DX_VOS_GetTime() * 1000;
}

DxUint32_t DX_VOS_GetHighResTickCount()
{
	return DX_VOS_GetTickCount();
}

DxUint32_t DX_VOS_GetHighResTicksPerSecond()
{
	return 1000;
}
