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
 
 
#ifndef CRYS_AESGCM_LOCAL_H
#define CRYS_AESGCM_LOCAL_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "DX_VOS_BaseTypes.h"
#include "CRYS_AESGCM_error.h"


#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object %CRYS_AESGCM_Local.h    : %
   *  State           :  %state%
   *  Creation date   :  21 March 2011
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief A brief description of this module
   *
   *  \version CRYS_AESGCM_Local.h#1:incl:1
   *  \author R.Levin
   */

/************************ Defines **************************************/

/* the AESGCM user context validity TAG */
#define AESGCM_CONTEXT_VALIDATION_TAG   0xAE50CC10


/************************ Enums ****************************************/

/************************ Typedefs  ************************************/

/************************ Structs  *************************************/

/************************ MACROS ***************************************/

/* this macro is required to remove compilers warnings if the AESGCM 
   is not supported */
#if (defined CRYS_NO_AESGCM_SUPPORT || defined CRYS_NO_AES_SUPPORT)
#define RETURN_IF_AESGCM_UNSUPPORTED( a , b , c , d , e , f , g , h , i , j , k , l , m , n , o , p , q , r , s , t , u , v , w , x, y ) \
  (a)=0;(b)=0;(c)=0;(d)=0;(e)=0;(f)=0;(g)=0;(h)=0;(i)=0;(j)=0;(k)=0;(l)=0;(m)=0;(n)=0;(o)=0;(p)=0;(q)=0;(r)=0;(s)=0; \
  (t)=0;(u)=0;(v)=0;(w)=0;(x)=0;(y)=0; \
  (a)=(a);(b)=(b);(c)=(c);(d)=(d);(e)=(e);(f)=(f);(g)=(g);(h)=(h);(i)=(i);(j)=(j);(k)=(k);(l)=(l);(m)=(m);(n)=(n); \
  (o)=o;(p)=p;(q)=q;(r)=r;(s)=s;(t)=t;(u)=u;(v)=v;(w)=w;(x)=x;(y)=y; \
   return CRYS_AESGCM_IS_NOT_SUPPORTED
#else  /* !CRYS_NO_AESGCM_SUPPORT */
#define RETURN_IF_AESGCM_UNSUPPORTED( a , b , c , d , e , f , g , h , i , j , k , l , m , n , o , p , q , r , s , t ,u , v , w , x, y) 
#endif /* !CRYS_NO_AESGCM_SUPPORT */


/************************ Public Functions *****************************/

#ifdef __cplusplus
}
#endif

#endif


