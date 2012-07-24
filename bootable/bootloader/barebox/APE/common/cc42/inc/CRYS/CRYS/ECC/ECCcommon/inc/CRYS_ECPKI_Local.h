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
 
 #ifndef CRYS_ECPKI_LOCAL_H
#define CRYS_ECPKI_LOCAL_H
  /*
   *  Object %name    : %
   *  State           :  %state%
   *  Creation date   :  05/12/2005
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief this file containes the prototype of the service functions for
   *         the CRYS ECPKI module that are intendet for internaly usage.  
   *
   *  \version CRYS_ECPKI_Local.h#1:incl:1
   *  \author R.Levin
   */

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "DX_VOS_BaseTypes.h"
#include "CRYS_error.h" 
#include "CRYS_Defs.h"
#include "CRYS_ECPKI_Types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/************************ Defines ******************************/

/*-------------------------------------------------*/
/*  User passed structures validation tags         */
/*-------------------------------------------------*/

/* the ECPKI user public key user validity TAG */
#define CRYS_ECPKI_PUBL_KEY_VALIDATION_TAG  0xEC000001
/* the ECPKI user private key user validity TAG */
#define CRYS_ECPKI_PRIV_KEY_VALIDATION_TAG 0xEC000002

/* the ECDSA signing user context validity TAG */
#define CRYS_ECDSA_SIGN_CONTEXT_VALIDATION_TAG   0xEC000003
/* the ECDSA verifying user context validity TAG */
#define CRYS_ECDSA_VERIFY_CONTEXT_VALIDATION_TAG 0xEC000004


/************************ macros ********************************/

/* this macro is required to remove compilers warnings if the HASH or ECPKI is not supported */

#if defined(CRYS_NO_ECPKI_SUPPORT) || defined(CRYS_NO_HASH_SUPPORT)

  #define RETURN_IF_ECPKI_UNSUPPORTED( a , b , c , d , e , f , g , h , i , j , k , l , a1 , b1 , c1 , d1 , e1 , f1 , g1 , h1 , i1 , j1) \
  (a)=(0);(b)=(0);(c)=(0);(d)=(0);(e)=(0);(f)=(0);(g)=(0);(h)=(0);(i)=(0);(j)=(0);(k)=(0);(l)=(0); \
  (a1)=(0);(b1)=(0);(c1)=(0);(d1)=(0);(e1)=(0);(f1)=(0);(g1)=(0);(h1)=(0);(i1)=(0);(j1)=(0); \
  (a)=(a);(b)=(b);(c)=(c);(d)=(d);(e)=(e);(f)=(f);(g)=(g);(h)=(h);(i)=(i);(j)=(j);(k)=(k);(l)=(l); \
  (a1)=(a1);(b1)=(b1);(c1)=(c1);(d1)=(d1);(e1)=(e1);(f1)=(f1);(g1)=(g1);(h1)=(h1);(i1)=(i1);(j1)=(j1); \
  return CRYS_ECPKI_IS_NOT_SUPPORTED

#else  /* !CRYS_NO_HASH_SUPPORT && ! CRYS_NO_ECPKI_SUPPORT */

  #define RETURN_IF_ECPKI_UNSUPPORTED( a , b , c , d , e , f , g , h , i , j , k , l , a1 , b1 , c1 , d1 , e1 , f1 , g1 , h1 , i1 , j1)  

#endif /* !CRYS_NO_HASH_SUPPORT && ! CRYS_NO_ECPKI_SUPPORT */
               
/************************ Typedefs  *****************************/

/************************ Structs  ******************************/

/************************ Public Variables **********************/

/************************ Public Functions **********************/


#ifdef __cplusplus
}
#endif

#endif /* #ifndef CRYS_ECPKI_LOCAL_H */

