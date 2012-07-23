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
 
 
#ifndef _1_CRYS_DH_local_h_H
#define _1_CRYS_DH_local_h_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "DX_VOS_BaseTypes.h"
#include "CRYS_error.h" 
#include "CRYS_Defs.h"
#include "CRYS_DH.h"


#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object %name    : %
   *  State           :  %state%
   *  Creation date   :  Wed Feb 02 13:39:23 2005
   *  Last modified   :  %modify_time% 
   */
  /** @file
   *  \brief This file contains the functions for generating prime domain parameters 
   *         P and Q accocrding to PKCS-3 standard
   *
   *  \version CRYS_DH_local.h#1:incl:1
   *  \author ohads
   */




/************************ Defines ******************************/

/************************ Enums ********************************/


/************************ Typedefs  ****************************/


/************************ Structs  ******************************/


/************************ Public Variables **********************/


/************************ Public Functions **********************/

/******************************************************************************************/
/**
 * @brief CRYS_DH_PKCS3_PRV_P generates the private P random buffer
 *
 * @param[in]  Prime_ptr 			- A pointer to the Prime octet string. Assumption - no leading zeros
 * @param[in]  PrimeSize 			- The size of the Prime string (in bytes)
 * @param[out] Output_ptr           - A pointer to the output Private key octet string
 * @param[out] Outputsize_ptr       - A pointer to the actual size of the output Private key octet string
 *
 * @return CRYSError_t              - On success CRYS_OK is returned, on failure a
 *			                          and on failure an ERROR as defined in CRYS_DH_error.h
 */

CRYSError_t CRYS_DH_PKCS3_PRV_P(DxUint8_t * Prime_ptr, DxUint16_t  PrimeSize ,DxUint8_t * Output_ptr ,DxUint16_t * Outputsize_ptr);


/******************************************************************************************/
/**
 * @brief CRYS_DH_PKCS3_PRV_P generates the random private key of size L bits so, that PrivKey <= P-1.
 *
 *      Assuming: The prime P has no leading zeros and P-1 >= 2^(L-1) .
 *
 * @param[in]  L        			- The size of the private key (in bits).
 * @param[in]  Prime_ptr 			- A pointer to the Prime octet string. 
 * @param[in]  PrimeSizeBytes   	- The size of the Prime string (in bytes).
 * @param[out] Output_ptr           - A pointer to the output private key octet string.
 * @param[out] Outputsize_ptr       - A pointer to the actual size of the output Private key octet string.
 *
 * @return CRYSError_t              - On success CRYS_OK is returned, on failure a
 *			                          and on failure an ERROR as defined in CRYS_DH_error.h
 */CRYSError_t CRYS_DH_PKCS3_PRV_L(DxUint16_t L, DxUint8_t *Prime_ptr, DxUint16_t PrimeSizeBytes, DxUint8_t *Output_ptr, DxUint16_t *Outputsize_ptr);



                          
                          

#ifdef __cplusplus
}
#endif

#endif



