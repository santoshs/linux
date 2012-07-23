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
 
 
#ifndef CRYS_SST_LOCAL_H
#define CRYS_SST_LOCAL_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "DX_VOS_BaseTypes.h"
#include "CRYS_error.h" 
#include "CRYS_Defs.h"
#include "CRYS_RSA_Types.h"



#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object %name    : %
   *  State           :  %state%
   *  Creation date   :  Wed Dec 22 17:36:19 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief this file containes the prototype of the service functions for
   *         the CRYS RSA module that are for internaly usage.  
   *
   *  \version CRYS_RSA_Local.h#1:incl:1
   *  \author adams
   */




/************************ Defines ******************************/

#define CRYS_SST_MAX_HMAC_KEY_LEN_IN_BYTES			256
/************************ macros ********************************/

/* this macro is required to remove compilers warnings if the HASH or PKI is not supported */

#ifdef CRYS_NO_SST_SUPPORT
#define RETURN_IF_SST_UNSUPPORTED( a , b , c , d , e , f , g , h , i , j , k , l ) \
  (a)=0;(b)=0;(c)=0;(d)=0;(e)=0;(f)=0;(g)=0;(h)=0;(i)=0;(j)=0;(k)=0;(l)=0; \
  (a)=(a);(b)=(b);(c)=(c);(d)=(d);(e)=(e);(f)=(f);(g)=(g);(h)=(h);(i)=(i);(j)=(j);(k)=(k);(l)=(l); \
  return CRYS_SST_IS_NOT_SUPPORTED
#else  
#define RETURN_IF_SST_UNSUPPORTED( a , b , c , d , e , f , g , h , i , j , k , l ) 
#endif 
               
/************************ Typedefs  ****************************/


/************************ Structs  ******************************/


/************************ Public Variables **********************/


/************************ Public Functions **********************/


#ifdef __cplusplus
}
#endif

#endif

