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
 
 
#ifndef LLF_AESCCM_version_H
#define LLF_AESCCM_version_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "CRYS_version.h"


#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object %name    : %
   *  State           :  %state%
   *  Creation date   :  02 Oct. 2008
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief This file contains the prototype of LLF_AESCCM_GetVersion function.
   *
   *  \version LLF_AESCCM_version.h#1:incl:1
   *  \author R.Levin
   */


/************************ Defines ******************************/

/************************ Enums ********************************/

/************************ Typedefs  ****************************/

/************************ Structs  ******************************/

/************************ Public Variables **********************/

/************************ Public Functions **********************/

/**
 * @brief This API returns the LLF AESCCM version.
 *
 * The version contains the following:
 *
 * - component string - a string describing the nature of the release.
 * - release type : CRYS_DEFS_HW_ENGINE_TYPE 
 * - major , minor , sub , internal - the release digits.
 *
 * Each component: CRYS , LLF machines receives this database. 
 *
 * @param[in] version_ptr - a pointer to the version structure.
 *
 */

 void  LLF_AESCCM_GetVersion(CRYS_ComponentVersion_t *version_ptr);


#ifdef __cplusplus
}
#endif

#endif

