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
 
 
#ifndef CRYS_RND_LOCAL_H
#define CRYS_RND_LOCAL_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */



#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object %name    : %
   *  State           :  %state%
   *  Creation date   :  Wed Dec 29 13:23:59 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief A brief description of this module
   *
   *  \version CRYS_RND_local.h#1:incl:1
   *  \author adams
   */




/************************ Defines ******************************/
#ifndef CRYS_RND_AES_OLD_128BIT_ONLY

#define CRYS_RND_BASIC_BLOCK_SIZE_IN_WORDS 4
#define CRYS_RND_BASIC_BLOCK_SIZE_IN_BYTES (CRYS_RND_BASIC_BLOCK_SIZE_IN_WORDS*sizeof(DxUint32_t))
#define CRYS_RND_ENTROPY_BLOCK_SIZE_IN_WORDS 4
#define CRYS_RND_ENTROPY_BLOCK_SIZE_IN_BYTES (CRYS_RND_ENTROPY_BLOCK_SIZE_IN_WORDS*sizeof(DxUint32_t))

/*  Maximal reseed counter - indicates maximal number of 
	requests allowed between reseeds; according NIST 800-90 
	it is (2^48 - 1) = 0xFFFFFFFFFFFFF (two 32-bits words).  
	In our implementation allowed the following restricted value */
#define CRYS_RND_MAX_RESEED_COUNTER 	(0xFFFFFFFF - 0xF)

#else
#define CRYS_RND_MAX_SIZE_OF_ADDITIONAL_INPUT_BLOCK_SIZE 32
#define CRYS_RND_ADDITIONAL_INPUT_BLOCK_SIZE_32_IN_BYTES 32
#define CRYS_RND_ADDITIONAL_INPUT_BLOCK_SIZE_16_IN_BYTES 16
#define CRYS_RND_BASIC_BLOCK_SIZE_IN_WORDS 4
#define CRYS_RND_ENTROPY_BLOCK_SIZE_IN_WORDS 4
#define CRYS_RND_ENTROPY_BLOCK_SIZE_IN_BYTES (CRYS_RND_ENTROPY_BLOCK_SIZE_IN_WORDS*sizeof(DxUint32_t))
#define CRYS_RND_NONCE_BLOCK_SIZE_IN_WORDS 2
#define CRYS_RND_NONCE_BLOCK_SIZE_IN_BYTES (CRYS_RND_NONCE_BLOCK_SIZE_IN_WORDS*sizeof(DxUint32_t))
#define CRYS_RND_BASIC_BLOCK_SIZE_IN_BYTES (CRYS_RND_BASIC_BLOCK_SIZE_IN_WORDS*sizeof(DxUint32_t))
#define CRYS_RND_FIRST_ENTROPY_BLOCK_SIZE_IN_BYTES (CRYS_RND_ENTROPY_BLOCK_SIZE_IN_BYTES/2)

#define CRYS_RND_MAX_SIZE_FOR_GENERATE_IN_BYTES 		65536 

/* The number of TRNG block the user want to use */
#ifdef RND_KAT_TEST_MODE_FOR_VER_21
	/* Must be  2 - for the Instantiation KAT */
	#define CRYS_RND_NUM_OF_GLOBAL_TRNG_BLOCKS_FOR_INSTANTIATION 2
	/* Must be  1 - for the Reseeding KAT*/
	#define CRYS_RND_NUM_OF_GLOBAL_TRNG_BLOCKS_FOR_RESEEDING 1
#else
	/* Must be at least 2 */
	#define CRYS_RND_NUM_OF_GLOBAL_TRNG_BLOCKS_FOR_INSTANTIATION 0x3C0
	/* Must be at least 1 */
	#define CRYS_RND_NUM_OF_GLOBAL_TRNG_BLOCKS_FOR_RESEEDING 0x3C0
#endif

#define CRYS_RND_NUM_OF_TRNG_BLOCKS_FOR_INSTANTIATION (CRYS_RND_NUM_OF_GLOBAL_TRNG_BLOCKS_FOR_INSTANTIATION-1)
#define CRYS_RND_NUM_OF_TRNG_BLOCKS_FOR_RESEEDING (CRYS_RND_NUM_OF_GLOBAL_TRNG_BLOCKS_FOR_RESEEDING-1)
		  
#define CRYS_RND_AES_KEY_SIZE_IN_BYTES 16

#define CRYS_RND_NUM_OF_GLOBAL_TRNG_BYTES_FOR_INSTANTIATION (CRYS_RND_NUM_OF_GLOBAL_TRNG_BLOCKS_FOR_INSTANTIATION * CRYS_RND_ENTROPY_BLOCK_SIZE_IN_BYTES)
#define CRYS_RND_NUM_OF_GLOBAL_TRNG_BYTES_FOR_RESEEDING (CRYS_RND_NUM_OF_GLOBAL_TRNG_BLOCKS_FOR_RESEEDING * CRYS_RND_ENTROPY_BLOCK_SIZE_IN_BYTES)

#define CRYS_RND_NUM_OF_TRNG_BYTES_FOR_INSTANTIATION (CRYS_RND_NUM_OF_TRNG_BLOCKS_FOR_INSTANTIATION * CRYS_RND_ENTROPY_BLOCK_SIZE_IN_BYTES)
#define CRYS_RND_NUM_OF_TRNG_BYTES_FOR_RESEEDING (CRYS_RND_NUM_OF_TRNG_BLOCKS_FOR_RESEEDING * CRYS_RND_ENTROPY_BLOCK_SIZE_IN_BYTES)


	#endif

#if defined CRYS_NO_AES_RESTRICT_SUPPORT || defined CRYS_NO_RND_SUPPORT
#define RETURN_IF_RND_UNSUPPORTED( a , b , c , d , e , f , g , h , i , j ) \
	(a)=0;(b)=0;(c)=0;(d)=0;(e)=0;(f)=0;(g)=0;(h)=0;(i)=0;(j)=0; \
	(a)=(a);(b)=(b);(c)=(c);(d)=(d);(e)=(e);(f)=(f);(g)=(g);(h)=(h);(i)=(i);(j)=(j); \
	return CRYS_RND_IS_NOT_SUPPORTED
#else  /* !CRYS_NO_AES_SUPPORT */
#define RETURN_IF_RND_UNSUPPORTED( a , b , c , d , e , f , g , h , i , j ) 
#endif /* CRYS_NO_AES_SUPPORT || CRYS_NO_RND_SUPPORT */
	
/************************ Enums ********************************/


/************************ Typedefs  ****************************/


/************************ Structs  ******************************/

/************************ Public Variables **********************/
/* next externals are included for compilation with old projects */
extern DxUint32_t AdditionalInput[];
extern DxUint32_t K_Buffer[];
extern DxUint32_t V_Buffer[];
extern DxUint16_t AI_FLAG;

/************************ Public Functions **********************/

CRYSError_t CRYS_RND_Init( void );

#ifdef __cplusplus
}
#endif

#endif


