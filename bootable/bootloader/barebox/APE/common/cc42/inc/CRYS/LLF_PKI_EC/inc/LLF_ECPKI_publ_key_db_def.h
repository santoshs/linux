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
 
 #ifndef LLF_ECPKI_PUBL_KEY_DB_DEF_H
#define LLF_ECPKI_PUBL_KEY_DB_DEF_H  
/*
   *  Object %name    : %
   *  State           :  %state%
   *  Creation date   :  30.03.2008
   *  Last modified   :  % % 
   */
  /** @file
   *  \brief A brief description of this module
   *
   *  \version LLF_ECPKI_pub_key_db_def.h#1:incl:1
   *  \author R.Levin
   */
/*
 * This file is #included in the middle of the struct declaration for CRYS_ECPKI_PublKey_t
 * It contains the platform-specific parts of the context struct. As such:
 *
 *  1) file should not use any includes it is a part of the CRYS_ECPKI_Types.h file !!!!
 *  2) only the CRYS_ECPKI_Types.h file should include this file. 
 */


     struct
     {   
        DxUint32_t NP[LLF_ECPKI_PKA_BARRETT_MOD_TAG_BUFF_SIZE_IN_WORDS]; /* Barrett for modulus*/
        DxUint32_t RP[LLF_ECPKI_PKA_BARRETT_MOD_TAG_BUFF_SIZE_IN_WORDS]; /* Barrett for order */
        
     } LLF;

#endif
