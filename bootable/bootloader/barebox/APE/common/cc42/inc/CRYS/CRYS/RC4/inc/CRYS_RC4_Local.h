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
 
 
#ifndef CRYS_RC4_LOCAL_H
#define CRYS_RC4_LOCAL_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */

#include "DX_VOS_BaseTypes.h"
#include "CRYS_RC4_error.h"


#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object %CRYS_RC4_Local.h    : %
   *  State           :  %state%
   *  Creation date   :  14 June 2007
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief A brief description of this module
   *
   *  \version CRYS_RC4_Local.h#1:incl:1
   *  \author R.Levin
   */



/************************ Defines **************************************/

/* the RC4 user context validity TAG */
#define RC4_CONTEXT_VALIDATION_TAG 0xA5A5A5A5


/************************ Enums ****************************************/

/************************ Typedefs  ************************************/

/************************ Structs  *************************************/

/************************ MACROS ***************************************/

/* this macro is required to remove compilers warnings if the RC4 
   is not supported */
#ifdef CRYS_NO_RC4_SUPPORT
#define RETURN_IF_RC4_UNSUPPORTED( a , b , c , d , e , f , g , h , i , j , k , l , m , n ) \
  (a)=0;(b)=0;(c)=0;(d)=0;(e)=0;(f)=0;(g)=0;(h)=0;(i)=0;(j)=0;(k)=0;(l)=0;(m)=0;(n)=0; \
  (a)=(a);(b)=(b);(c)=(c);(d)=(d);(e)=(e);(f)=(f);(g)=(g);(h)=(h);(i)=(i);(j)=(j);(k)=(k);(l)=(l);(m)=(m);(n)=(n); \
  return CRYS_RC4_IS_NOT_SUPPORTED
#else  /* !CRYS_NO_RC4_SUPPORT */
#define RETURN_IF_RC4_UNSUPPORTED( a , b , c , d , e , f , g , h , i , j , k , l , m , n ) 
#endif /* !CRYS_NO_RC4_SUPPORT */


/************************ Public Functions *****************************/

#ifdef __cplusplus
}
#endif

#endif


