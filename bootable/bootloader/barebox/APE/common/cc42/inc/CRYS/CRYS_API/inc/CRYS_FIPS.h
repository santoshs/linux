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
 
#ifndef CRYS_FIPS_H
#define CRYS_FIPS_H

#ifndef CRYS_NO_FIPS_SUPPORT

#include "DX_VOS_BaseTypes.h"
#include "CRYS_error.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /** @file
   *  \brief Return FIPS mode state
   *
   *  \author nogad
   */

/************************ Defines ******************************/
#define DX_CRYS_FIPS_MODE_STATE_OFF   0x0
#define DX_CRYS_FIPS_MODE_STATE       0x1
#define DX_CRYS_FIPS_MODE_ERROR_STATE 0x2

/************************ Structs  ******************************/


/************************ Public Functions **********************/

/**
 * @brief This function retrieves the FIPS mode (ON or OFF)
 *
 * @param[out] FipsMode - holds the returned FIPS mode
 *                            
 * @return CRYSError_t - On success the function returns the value CRYS_OK, and on failure a non-ZERO error.
 *
 */
 CIMPORT_C CRYSError_t  CRYS_FIPS_GetFipsMode (DxUint8_t *FipsMode);
 
 /**
 * @brief This function sets the FIPS mode (always to ON). If the FIPS mode is set to Error 
 *        the function will not change it.
 *
 * @param[in] void
 *                            
 * @return void
 *
 */
 CIMPORT_C CRYSError_t CRYS_FIPS_SetFipsMode ( void );

#ifdef __cplusplus
}
#endif

#endif 

#endif


