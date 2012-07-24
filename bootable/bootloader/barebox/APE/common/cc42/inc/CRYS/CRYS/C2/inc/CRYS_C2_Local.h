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
 
 
#ifndef CRYS_C2_LOCAL_H
#define CRYS_C2_LOCAL_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "DX_VOS_BaseTypes.h"
#include "CRYS_C2_error.h"


#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object %CRYS_C2_Local.h    : %
   *  State           :  %state%
   *  Creation date   :  Sun Nov 21 19:32:08 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief A brief description of this module
   *
   *  \version CRYS_C2_Local.h#1:incl:1
   *  \author R. Levin (using as templet the DES files of adams).
   *  \remarks Copyright (C) 2007 by Discretix Technologies Ltd.
   *           All Rights reserved
   */




/************************ Defines ******************************/


/* the C2 Cipher user context validity TAG */
#define C2_CIPHER_CONTEXT_VALIDATION_TAG 0xAABBCCEE

/* the C2 HASH user context validity TAG */
#define C2_HASH_CONTEXT_VALIDATION_TAG 0xAABBCCFF


/************************ Enums ********************************/

/************************ MACROS ******************************/

/* this macro is required to remove compilers warnings if the C2 is not supported */

#ifdef CRYS_NO_C2_SUPPORT
#define RETURN_IF_C2_UNSUPPORTED( a , b , c , d , e , f , g , h , i , j ) \
  (a)=0;(b)=0;(c)=0;(d)=0;(e)=0;(f)=0;(g)=0;(h)=0;(i)=0;(j)=0; \
  (a)=(a);(b)=(b);(c)=(c);(d)=(d);(e)=(e);(f)=(f);(g)=(g);(h)=(h);(i)=(i);(j)=(j); \
  return CRYS_C2_IS_NOT_SUPPORTED
#else  /* !CRYS_NO_C2_SUPPORT */
#define RETURN_IF_C2_UNSUPPORTED( a , b , c , d , e , f , g , h , i , j ) 
#endif /* !CRYS_NO_C2_SUPPORT */

/************************ Typedefs  ****************************/

/************************ Structs  ******************************/

/************************ Local Functions ***********************/


#ifdef __cplusplus
}
#endif

#endif


