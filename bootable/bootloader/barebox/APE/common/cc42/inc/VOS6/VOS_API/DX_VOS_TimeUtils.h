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
 
 

#ifndef _DX_VOS_TIMEUTILS_H
#define _DX_VOS_TIMEUTILS_H

/*! \file DX_VOS_TIME.h
    This file imlements time format conversions.
	All functions that return DxStatus may return:
	- DX_SUCCESS - on success
	- DX_BAD_ARGUMENTS - if pointer parameters that are passed are equal to NULL.
	- DX_VOS_TIME_ERROR - if conversion failed.
*/

#include "DX_VOS_Time.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*! the structure for storage the generalized time format as described in 
    the X.509 specification (see the validity field description)
 */
typedef struct DXgeneralized_Time_t
{
  DxUint8 year_m1;
  DxUint8 year_l1;
  DxUint8 year_m2;
  DxUint8 year_l2;
  DxUint8 months_m;
  DxUint8 months_l;
  DxUint8 day_m;
  DxUint8 day_l;
  DxUint8 hours_m;
  DxUint8 hours_l;
  DxUint8 minutes_m;
  DxUint8 minutes_l;
  DxUint8 seconds_m;
  DxUint8 seconds_l;
  DxUint8 Zulu;
  DxUint8 endOfStr;
}DxGeneralizedTime_t;

/*! the structure for storage the UTC time format as described in 
    the X.509 specification (see the validity field description)
 */
 typedef struct DXutc_Time_t
 {
  DxUint8 year_m;
  DxUint8 year_l;
  DxUint8 months_m;
  DxUint8 months_l;
  DxUint8 day_m;
  DxUint8 day_l;
  DxUint8 hours_m;
  DxUint8 hours_l;
  DxUint8 minutes_m;
  DxUint8 minutes_l;
  DxUint8 seconds_m;
  DxUint8 seconds_l;
  DxUint8 Zulu;
  DxUint8 endOfStr;
 }DxUtcTime_t;


/*! Retrieves the current UTC time. */
DxStatus DX_VOS_GetUTCTime(DxUtcTime_t *aUtcTime); 

/*! Retrieves the current Generalized time. */
DxStatus DX_VOS_GetGeneralizedTime(DxGeneralizedTime_t *aGenTime);

/*! Compares 2 UTC times. a NULL pointer is considered to be the earliest time.
	\return
	- 0 if two times are identical.
	- -1 if aUtc1 < aUtc2
	- 1 if aUtc1 > aUtc2
*/
DxInt DX_VOS_UTCTimeCompare(const DxUtcTime_t *aUtc1, const DxUtcTime_t *aUtc2);

/*! Compares 2 Generalized times. a NULL pointer is considered to be the earliest time.
	\return
	- 0 if two times are identical.
	- -1 if aUtc1 < aUtc2
	- 1 if aUtc1 > aUtc2
*/
DxInt DX_VOS_GenTimeCompare(const DxGeneralizedTime_t *aTm1, const DxGeneralizedTime_t *aTm2);

/*! Converts UTC time to number of seconds since January 1, 1970. */
DxStatus DX_VOS_UTCTimeToSecs(const DxUtcTime_t* aUtcTime, DxTime_t* aTime);

/*! Converts Generalized time to number of seconds since January 1, 1970. */
DxStatus DX_VOS_GenTimeToSecs(const DxGeneralizedTime_t* aGenTime, DxTime_t* aTime);

/*! Converts time that is represented as the number of seconds since 
January 1, 1970 to UTC time */
DxStatus DX_VOS_SecsToUTCTime(DxTime_t aTime, DxUtcTime_t* aUtcTime);

/*! Converts time that is represented as the number of seconds since 
January 1, 1970 to Generalized time */
DxStatus DX_VOS_SecsToGenTime(DxTime_t aTime, DxGeneralizedTime_t* aGenTime);

/*! Converts a character representation (a string which may or may not be null terminated) of Generalized time to the DxGeneralizedTime_t struct*/
DxStatus DX_VOS_InitGenTimeFromMem(const void* timeString, DxUint32 length, DxGeneralizedTime_t* aGenTime);

/*! Converts a character representation (a string which may or may not be null terminated) of UTC time to the DxUtcTime_t struct*/
DxStatus DX_VOS_InitUtcTimeFromMem(const void* timeString, DxUint32 length, DxUtcTime_t* aUtcTime);


#ifdef DX_USE_LEGACY_VOS
DxInt DX_VOS_DRM_UTCTimeCompare(const DxUtcTime_t *aUtc1, const DxUtcTime_t *aUtc2);
DxInt DX_VOS_DRM_generalizedTimeCompare(const DxGeneralizedTime_t *aTm1, const DxGeneralizedTime_t *aTm2);

#define DX_VOS_Convert_UTC_Time DX_VOS_UTCTimeToSecs
#define DX_VOS_Convert_GEN_Time DX_VOS_GenTimeToSecs
#define DX_VOS_Convert_DxTime_t(aTime_ptr, aUtcTime_ptr) \
	DX_VOS_SecsToUTCTime(*aTime_ptr, aUtcTime_ptr)

#define IDENTICAL_TIME      0
#define TIME_1_IS_AFTER     1
#define TIME_2_IS_AFTER     2

#endif
#ifdef  __cplusplus
}
#endif


#endif /* ifndef _DX_VOS_TIME_H */









