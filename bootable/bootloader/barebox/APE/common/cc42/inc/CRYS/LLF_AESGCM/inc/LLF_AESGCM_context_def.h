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
 
#ifndef LLF_AESGCM_CONTEXT_DEF_H
#define LLF_AESGCM_CONTEXT_DEF_H

/*
 * This file is included in the middle of the struct declaration for AESGCM_Context_t
 * It contains the platform-specific parts of the context struct. As such:
 *
 *  1) file should not use any includes it is a part of the CRYS_AESGCM.h file !!!!
 *  2) only the CRYS_AESGCM.h file should include this file. 
 */


  /*
   *  Object %name    : %
   *  State           :  %state%
   *  Creation date   :  21 March 2011
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief A brief description of this module
   *
   *  \version LLF_AESGCM_context_def.h#1:incl:1
   *  \author R.Levin
   */

/* ******************** the LLF structure definition ********************* */

struct {

   /* Current (Ctr) and first (J0) counter values  */ 
   DxUint32_t Ctr[CRYS_AES_IV_COUNTER_SIZE_IN_WORDS];

   /* AESGCM Key */
   DxUint32_t   Key[CRYS_AESGCM_KEY_MAX_SIZE_WORDS ];
   DxUint32_t   KeySizeBytes;

   /* Flags */
   DxUint32_t   IsFinish;  
    	
} LLF;


#endif


