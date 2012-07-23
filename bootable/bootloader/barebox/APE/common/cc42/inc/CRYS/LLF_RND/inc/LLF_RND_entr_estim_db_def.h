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
 
#ifndef LLF_RND_ENTR_ESTIM_DB_DEF_H
#define LLF_RND_ENTR_ESTIM_DB_DEF_H

/*
 * This file is #included in the middle of the struct declaration for CRYS_RND_EntropyEstimatData_t.
 * It contains the platform-specific parts of the context struct. 
 *
 *  Note:
 *    - file should not use any includes it is a part of the CRYS_RND.h file !!!!
 *    - only the CRYS_RND.h file should include this file. 
 */

  /*
   *  Object %name    : %
   *  State           :  %state%
   *  Creation date   :  24 Feb 2010
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief A brief description of this module
   *
   *  \version LLF_RND_entr_estim_db_def.h#1:incl:1
   *  \author R.Levin
   */  

/* structure containing parameters and buffers for entropy estimator */
struct
{ 
   /* estimated entropy size */
   DxUint32_t EstimEntropySizeBits;     
   /* estimated error of entropy size */
   DxUint32_t EstimEntropySizeErrorInBits;  

   /* special buffers */
   DxUint32_t h[CRYS_RND_NB];         /* histogram */
   DxUint32_t ec[CRYS_RND_NB/2];      /* equality counter for prefix */
   
   /*!!! Note: The p buffer is not in use now, but saved for compatibility
      with CC52 ROM version. It may be deleted in the future */
   DxUint32_t p[CRYS_RND_NB/2];       /* previous bit for prefix */
   
}LLF;


#endif
