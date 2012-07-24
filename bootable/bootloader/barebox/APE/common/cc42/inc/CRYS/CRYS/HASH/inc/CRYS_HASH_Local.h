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
 
 
#ifndef CRYS_HASH_LOCAL_H
#define CRYS_HASH_LOCAL_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include"DX_VOS_BaseTypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object %name    : %
   *  State           :  %state%
   *  Creation date   :  Mon Nov 29 14:34:34 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief this file containes definitions relevant to the HASH module
   *         that should be hidden from the to level 'H' file ( internal definitions ) 
   *
   *  \version CRYS_HASH_Local.h#1:incl:1
   *  \author adams
   */

/************************ Defines ******************************/

/* The HASH init value on SHA1 mode */
#define CRYS_HASH_SHA1_INIT_0       0x67452301UL
#define CRYS_HASH_SHA1_INIT_1       0xefcdab89UL
#define CRYS_HASH_SHA1_INIT_2       0x98badcfeUL
#define CRYS_HASH_SHA1_INIT_3       0x10325476UL
#define CRYS_HASH_SHA1_INIT_4       0xc3d2e1f0UL 

/* The HASH init value on MD5 mode */
#define CRYS_HASH_MD5_INIT_0       0x67452301UL
#define CRYS_HASH_MD5_INIT_1       0xefcdab89UL
#define CRYS_HASH_MD5_INIT_2       0x98badcfeUL
#define CRYS_HASH_MD5_INIT_3       0x10325476UL
#define CRYS_HASH_MD5_INIT_4       0x0UL 

/* The HASH init values for SHA-224 */
#define CRYS_HASH_SHA224_INIT_0      0xc1059ed8UL 
#define CRYS_HASH_SHA224_INIT_1		 0x367cd507UL
#define CRYS_HASH_SHA224_INIT_2      0x3070dd17UL
#define CRYS_HASH_SHA224_INIT_3      0xf70e5939UL
#define CRYS_HASH_SHA224_INIT_4      0xffc00b31UL
#define CRYS_HASH_SHA224_INIT_5      0x68581511UL
#define CRYS_HASH_SHA224_INIT_6      0x64f98fa7UL
#define CRYS_HASH_SHA224_INIT_7		 0xbefa4fa4UL

/* The HASH init value on SHA256 mode */
#define CRYS_HASH_SHA256_INIT_0       0x6a09e667UL
#define CRYS_HASH_SHA256_INIT_1       0xbb67ae85UL
#define CRYS_HASH_SHA256_INIT_2       0x3c6ef372UL
#define CRYS_HASH_SHA256_INIT_3       0xa54ff53aUL
#define CRYS_HASH_SHA256_INIT_4       0x510e527fUL
#define CRYS_HASH_SHA256_INIT_5       0x9b05688cUL 
#define CRYS_HASH_SHA256_INIT_6       0x1f83d9abUL 
#define CRYS_HASH_SHA256_INIT_7       0x5be0cd19UL

/* The HASH init value on SHA384 mode */
#define CRYS_HASH_SHA384_INIT_0_HI    0xcbbb9d5dUL
#define CRYS_HASH_SHA384_INIT_1_HI    0x629a292aUL
#define CRYS_HASH_SHA384_INIT_2_HI    0x9159015aUL
#define CRYS_HASH_SHA384_INIT_3_HI    0x152fecd8UL
#define CRYS_HASH_SHA384_INIT_4_HI    0x67332667UL
#define CRYS_HASH_SHA384_INIT_5_HI    0x8eb44a87UL 
#define CRYS_HASH_SHA384_INIT_6_HI    0xdb0c2e0dUL 
#define CRYS_HASH_SHA384_INIT_7_HI    0x47b5481dUL

#define CRYS_HASH_SHA384_INIT_0_LO    0xc1059ed8UL
#define CRYS_HASH_SHA384_INIT_1_LO    0x367cd507UL
#define CRYS_HASH_SHA384_INIT_2_LO    0x3070dd17UL
#define CRYS_HASH_SHA384_INIT_3_LO    0xf70e5939UL
#define CRYS_HASH_SHA384_INIT_4_LO    0xffc00b31UL
#define CRYS_HASH_SHA384_INIT_5_LO    0x68581511UL 
#define CRYS_HASH_SHA384_INIT_6_LO    0x64f98fa7UL 
#define CRYS_HASH_SHA384_INIT_7_LO    0xbefa4fa4UL

/* The HASH init value on SHA512 mode */
#define CRYS_HASH_SHA512_INIT_0_HI    0x6a09e667UL
#define CRYS_HASH_SHA512_INIT_1_HI    0xbb67ae85UL
#define CRYS_HASH_SHA512_INIT_2_HI    0x3c6ef372UL
#define CRYS_HASH_SHA512_INIT_3_HI    0xa54ff53aUL
#define CRYS_HASH_SHA512_INIT_4_HI    0x510e527fUL
#define CRYS_HASH_SHA512_INIT_5_HI    0x9b05688cUL 
#define CRYS_HASH_SHA512_INIT_6_HI    0x1f83d9abUL 
#define CRYS_HASH_SHA512_INIT_7_HI    0x5be0cd19UL

#define CRYS_HASH_SHA512_INIT_0_LO    0xf3bcc908UL
#define CRYS_HASH_SHA512_INIT_1_LO    0x84caa73bUL
#define CRYS_HASH_SHA512_INIT_2_LO    0xfe94f82bUL
#define CRYS_HASH_SHA512_INIT_3_LO    0x5f1d36f1UL
#define CRYS_HASH_SHA512_INIT_4_LO    0xade682d1UL
#define CRYS_HASH_SHA512_INIT_5_LO    0x2b3e6c1fUL 
#define CRYS_HASH_SHA512_INIT_6_LO    0xfb41bd6bUL 
#define CRYS_HASH_SHA512_INIT_7_LO    0x137e2179UL  


/* the HASH user context validity TAG */
#define HASH_CONTEXT_VALIDATION_TAG 0x12345678

/* this macro is required to remove compilers warnings if the HASH is not supported */

#ifdef CRYS_NO_HASH_SUPPORT
#define RETURN_IF_HASH_UNSUPPORTED( a , b , c , d , e , f , g , h , i , j ) \
  (a)=(0);(b)=(0);(c)=(0);(d)=(0);(e)=(0);(f)=(0);(g)=(0);(h)=(0);(i)=(0);(j)=(0); \
  (a)=(a);(b)=(b);(c)=(c);(d)=(d);(e)=(e);(f)=(f);(g)=(g);(h)=(h);(i)=(i);(j)=(j); \
  return CRYS_HASH_IS_NOT_SUPPORTED
#else  /* !CRYS_NO_HASH_SUPPORT */
#define RETURN_IF_HASH_UNSUPPORTED( a , b , c , d , e , f , g , h , i , j ) 
#endif /* !CRYS_NO_HASH_SUPPORT */
  

/************************ Enums ********************************/

/************************ Typedefs  ****************************/

/************************ Structs  ******************************/

/************************ Public Variables **********************/



#ifdef __cplusplus
}
#endif

#endif

