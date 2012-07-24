/***********************************************************************************
 * Copyright 2009 © Discretix Technologies Ltd.
 * This software is protected by copyright, international treaties and
 * various patents. If the license governing the use of this Software
 * allows copy or redistribution of this  software then any copy or
 * reproduction of this Software must include this Copyright Notice
 * as well as any other notices provided under such license.
 ***********************************************************************************/


/************* Include Files ****************/

#include "CRYS.h"
#include "CRYS_COMMON.h"
#include "CRYS_COMMON_math.h"
#include "CRYS_COMMON_error.h"

/************************ Defines ******************************/

/************************ Enums ********************************/

/************************ Typedefs *****************************/

/************************ Global Data **************************/

/************* Private function prototype **********************/

/************************ Public Functions *********************/

#ifndef BIG__ENDIAN
/***********************************************************************
 **
 * @brief This function executes a reverse bytes copying from one buffer to another buffer.
 *
 *        Overlapping of buffers is not allowed, excluding the case, when destination and source
 *        buffers are the same.
 *        Example of a 5 byte buffer:
 *
 *        dst_ptr[4] = src_ptr[0]
 *        dst_ptr[3] = src_ptr[1]
 *        dst_ptr[2] = src_ptr[2]
 *        dst_ptr[1] = src_ptr[3]
 *        dst_ptr[0] = src_ptr[4]
 *
 * @param[in] dst_ptr - The pointer to destination buffer.
 * @param[in] src_ptr - The pointer to source buffer.
 * @param[in] size    - The size in bytes.
 *
 */
 CRYSError_t CRYS_COMMON_ReverseMemcpy( DxUint8_t *dst_ptr , DxUint8_t *src_ptr , DxUint32_t size )
 {
	 /* FUNCTION DECLARATIONS */

	 /* loop variable */
	 DxUint32_t i;

	 /* buffers position identifiers */
	 DxUint32_t buff_dst_pos, buff_src_pos;

	 /* FUNCTION LOGIC */

     /* check overlapping */
	 if( (dst_ptr > src_ptr && dst_ptr < (src_ptr + size)) ||
	     (dst_ptr < src_ptr && (dst_ptr + size) >= src_ptr) )

			return CRYS_COMMON_DATA_OUT_DATA_IN_OVERLAP_ERROR;


	 /* execute the reverse copy in case of different buffers */
	 if( dst_ptr != src_ptr )
	 {
		 /* initialize the source and the destination position */
		 buff_dst_pos = size - 1;
		 buff_src_pos = 0;

		 for( i = 0 ; i < size ; i++ )
			 dst_ptr[buff_dst_pos--] = src_ptr[buff_src_pos++];
	 }

     /* execute the reverse copy in the same place */
	 else
	 {
		 DxUint8_t temp, size1;

		 size1 = (DxUint8_t)(size / 2);

		 /* initialize the source and the destination position */
		 buff_dst_pos = size1 - 1;
		 buff_src_pos = size - size1;

		 for( i = 0 ; i < size1 ; i++ )
		 {
			 temp = dst_ptr[buff_dst_pos];
			 dst_ptr[buff_dst_pos--] = src_ptr[buff_src_pos];
			 src_ptr[buff_src_pos++] = temp;
		 }
	 }

	 return CRYS_OK;

 }/* END OF CRYS_COMMON_ReverseMemcpy */

#endif



#ifdef BIG__ENDIAN
/***********************************************************************
 **
 * @brief This function reverses endianness of words and copyies they from
 *        one buffer to another buffer.
 *
 *        NOTES:
 *            2. Assumed, that at less one of buffers (output or input) is
 *               aligned to 4-bytes word.
 *            1. Overlapping of buffers is not allowed, besides the case both
 *               input and output buffers are aligned.
 *
 *        The function does the following:
 *
 *        Copies data so, that LSWord is the first and MSWord is the last, but order
 *        of bytes in words is not changed (according to Big endian definitions).
 *
 *
 * @param[in] dst_ptr - The pointer to destination buffer.
 * @param[in] src_ptr - The pointer to source buffer, containing bytes stream, so that
 *                      the MSByte is the first and the LSByte is the last.
 * @param[in] size    - The size in bytes.
 *
 */
 CRYSError_t CRYS_COMMON_ReverseMemcpy( DxUint8_t *dst_ptr , DxUint8_t *src_ptr , DxUint32_t size )
 {
	 /* FUNCTION DECLARATIONS */

	 /* loop variable */
	 DxUint32_t i;

	 /* buffers position identifiers */
	 DxInt32_t dst_pos, src_pos;
	 DxUint32_t endSize;

	 /* FUNCTION LOGIC */

	 endSize = size % 4;

	 /* check that destination buffer is aligned */
	 if( (DxUint32_t)src_ptr % 4 != 0   &&
	     (DxUint32_t)dst_ptr % 4 != 0 )

			return CRYS_COMMON_POINTER_NOT_ALIGNED_ERROR;

     /* check overlapping */
	 if( (dst_ptr >= src_ptr && dst_ptr < (src_ptr + size)) ||
	     (dst_ptr < src_ptr && (dst_ptr + size) >= src_ptr) )

			return CRYS_COMMON_DATA_OUT_DATA_IN_OVERLAP_ERROR;


	 /* execute the reverse copy in case of dst aligned */
	 if( (DxUint32_t)dst_ptr % 4 == 0 )
	 {
		 /* initialize the source and the destination position */
		 src_pos = size - 1;
		 dst_pos = 3;  /* LSByte of word 0 of source */


         /* copy full words */
		 for( i = 0 ; i < (size - endSize); i=i+4 )
		 {
			 dst_ptr[dst_pos--] = src_ptr[src_pos--];
			 dst_ptr[dst_pos--] = src_ptr[src_pos--];
			 dst_ptr[dst_pos--] = src_ptr[src_pos--];
			 dst_ptr[dst_pos--] = src_ptr[src_pos--];

			 dst_pos += 8;   /* update src position to LSByte of next word */
		 }

		 /* copy not aligned end of input data in big endian */
		 for( i = 0 ; i < endSize ; i++ )
		 {
			 dst_ptr[dst_pos--] = src_ptr[src_pos--];
		 }
	 }

	 else  /* execute the reverse copy in case of dst is non aligned */
	 {
		 /* initialize the source and the destination position */
		 src_pos = 3;  /* LSByte of word 0 of source */
		 dst_pos = size - 1;

         /* copy full words */
		 for( i = 0 ; i < (size - endSize); i=i+4 )
		 {
			 dst_ptr[dst_pos--] = src_ptr[src_pos--];
			 dst_ptr[dst_pos--] = src_ptr[src_pos--];
			 dst_ptr[dst_pos--] = src_ptr[src_pos--];
			 dst_ptr[dst_pos--] = src_ptr[src_pos--];

			 src_pos += 8;   /* update src position to LSByte of next word */
		 }

		 /* copy not aligned end of input data in big endian */
		 for( i = 0 ; i < endSize ; i++ )
		 {
			 dst_ptr[dst_pos--] = src_ptr[src_pos--];
		 }
	 }


	 return CRYS_OK;

 }/* END OF CRYS_COMMON_ReverseMemcpy for Big Endian */

#endif

#ifdef BIG__ENDIAN
/*******************************************************************************
 *                      CRYS_COMMON_LsMsWordsArrayToMsLsBytes                  *
 *******************************************************************************
 **
 * @brief This function converts words array to bytes array,  where the words
 *        and bytes order in arrays are the following:
 *          - in source array words order is: LSWord first, MSWord last;
 *          - in the destionation array bytes order is: the MSByte first, LSByte - last.
 *
 *        NOTES:
 *            1. Assumed, that input buffer is aligned to 4-bytes word.
 *            2. This implementation of the function is intended for Big endian machines.
 *
 *
 * @param[in] dst_ptr - The pointer to destination buffer.
 * @param[in] src_ptr - The pointer to source buffer, containing bytes stream, so that
 *                      the MSByte is the first and the LSByte is the last.
 * @param[in] size    - The size in bytes.
 *
 */
 CRYSError_t CRYS_COMMON_LsMsWordsArrayToMsLsBytes( DxUint8_t *dst_ptr , DxUint8_t *src_ptr , DxUint32_t size )
 {
	 /* FUNCTION DECLARATIONS */

	 /* loop variable */
	 DxUint32_t i;

	 /* buffers position identifiers */
	 DxInt32_t dst_pos, src_pos;
	 DxUint32_t endSize;

	 /* FUNCTION LOGIC */

	 endSize = size % 4;

	 /* check that destination buffer is aligned */
	 if( (DxUint32_t)src_ptr % 4 != 0 )

			return CRYS_COMMON_POINTER_NOT_ALIGNED_ERROR;

     /* check overlapping */
	 if( (dst_ptr >= src_ptr && dst_ptr < (src_ptr + size)) ||
	     (dst_ptr < src_ptr && (dst_ptr + size) >= src_ptr) )

			return CRYS_COMMON_DATA_OUT_DATA_IN_OVERLAP_ERROR;


	 /* initialize the source and the destination position */
	 src_pos = 3;
	 dst_pos = size - 1;  /* LSByte of word 0 of source */


     /* copy full words */
	 for( i = 0 ; i < (size - endSize); i=i+4 )
	 {
		 dst_ptr[dst_pos--] = src_ptr[src_pos--];
		 dst_ptr[dst_pos--] = src_ptr[src_pos--];
		 dst_ptr[dst_pos--] = src_ptr[src_pos--];
		 dst_ptr[dst_pos--] = src_ptr[src_pos--];

		 src_pos += 8;   /* update src position to LSByte of next word */
	 }

	 /* copy not aligned end of input data in big endian */
	 for( i = 0 ; i < endSize ; i++ )
	 {
		 dst_ptr[dst_pos--] = src_ptr[src_pos--];
	 }

	 return CRYS_OK;

 }/* END OF CRYS_COMMON_LsMsWordsArrayToMsLsBytes for Big Endian */


#endif



/*******************************************************************************
 *                      CRYS_COMMON_AlignedWordsArrayToBytes                   *
 *******************************************************************************
 **
 * @brief This function converts aligned words array to bytes array/
 *
 *            1. Assumed, that input buffer is aligned to 4-bytes word and
 *               bytes order is set according to machine endianness.
 *            2. Output buffer recieves data as bytes stream from LSB to MSB.
 *               For increasing performance on small buffers, the output data is given
 *               by rounded down pointer and alignment.
 *            3. This implementation is given for both Big and Little endian machines.
 *
 *
 * @param[in] in32_ptr - The pointer to aligned input buffer.
 * @param[in] out32_ptr - The 32-bits pointer to output buffer (rounded down to 4 bytes) .
 * @param[in] outAlignBits - The actual output data alignment;
 * @param[in] sizeWords - The size in words (sizeWords >= 1).
 *
 *  return - no return value.
 */
 void CRYS_COMMON_AlignedWordsArrayToBytes( DxUint32_t *in32_ptr , DxUint32_t *out32_ptr ,
                                            DxUint32_t outAlignBits, DxUint32_t sizeWords )
{
	 /* FUNCTION DECLARATIONS */

	 /* loop variable */
	 DxUint32_t i = 0;


       /* loading output buffer with the temp buffer */

   #ifdef BIG__ENDIAN

      /* loading the temp output buffer - copy it in the reverse order */
      if( outAlignBits == 0 )
      {
	      for (i=0;i<sizeWords;i++)
	      {
              out32_ptr[i] = CRYS_COMMON_REVERSE32(in32_ptr[i]);
	      }
      }

      else
      {
	      register DxUint32_t temp, temp1;

	      temp = CRYS_COMMON_REVERSE32(in32_ptr[0]);

	      out32_ptr[i] = out32_ptr[i] & (0xFFFFFFFF << (32-outAlignBits)) | temp >> outAlignBits;

	      for ( i = 1; i < sizeWords; i++ )
	      {
	          temp1 = CRYS_COMMON_REVERSE32(in32_ptr[i]);
              out32_ptr[i] = (temp << (32-outAlignBits)) | (temp1 >> outAlignBits);
              temp = temp1;
	      }

	      out32_ptr[i] = out32_ptr[i] & (0xFFFFFFFF >> outAlignBits) | temp << (32-outAlignBits);
      }

   #else  /* LITTLE__ENDIAN */

      /* loading the temp output buffer - copy it in the direct order */
      if( outAlignBits == 0 )
      {
	      for (i=0;i<sizeWords;i++)
	      {
              out32_ptr[i] = in32_ptr[i];
	      }
      }

      else
      {
	      register DxUint32_t temp, temp1;

	      temp = in32_ptr[0];

	      out32_ptr[i] = out32_ptr[i] & (0xFFFFFFFF >> (32-outAlignBits)) | temp << outAlignBits;

	      for ( i = 1; i < sizeWords; i++ )
	      {
	          temp1 = in32_ptr[i];
              out32_ptr[i] = (temp >> (32-outAlignBits)) | (temp1 << outAlignBits);
              temp = temp1;
	      }

	      out32_ptr[i] = out32_ptr[i] & (0xFFFFFFFF << outAlignBits) | temp >> (32-outAlignBits);
      }

   #endif


	 return;
}
