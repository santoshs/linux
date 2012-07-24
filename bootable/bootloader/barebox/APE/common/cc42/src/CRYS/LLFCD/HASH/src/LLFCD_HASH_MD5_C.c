
  /*
   *  Object %LLFCD_HASH_MD5_enc.c    : %
   *  State           :  %state%
   *  Creation date   :  Mon Nov 29 14:10:40 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief A brief description of this module
   *
   *  \version LLFCD_HASH_MD5_enc.c#1:csrc:4
   *  \author adams
   *  \remarks Copyright (C) 2004 by Discretix Technologies Ltd.
   *           All Rights reserved
   */

/************* Include Files ****************/

#include "DX_VOS_Mem.h"
#include "DX_VOS_BaseTypes.h"
#include "PLAT_SystemDep.h"
#include "CRYS_COMMON_math.h"
#include "CRYS.h"
#include "LLF_HASH.h"
#include "LLF_HASH_error.h"
#include "LLFCD_HASH.h"

/************************ Defines ******************************/
/* F, G, H and I are basic MD5 functions.*/
#define MD5_F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define MD5_G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define MD5_H(x, y, z) ((x) ^ (y) ^ (z))
#define MD5_I(x, y, z) ((y) ^ ((x) | (~z)))

#define ROTATE_RIGHT_32BIT_VAR(y, x, n) ( (y) = ((x) >> (n)) | ((x) << ( 32 - (n) )) )

/************************ Enums ******************************************/


/************************ Typedefs ***************************************/


/************************ Global Data ************************************/

/* the T table used for MD5 */
const DxUint32_t T[64]=
{
  0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee
, 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501
, 0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be
, 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821

, 0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa
, 0xd62f105d,  0x2441453, 0xd8a1e681, 0xe7d3fbc8
, 0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed
, 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a

, 0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c
, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70
, 0x289b7ec6, 0xeaa127fa, 0xd4ef3085,  0x4881d05
, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665

, 0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039
, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1
, 0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1
, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

/************* Private function prototype *******************************/


/************************ Public Functions ******************************/



/*
 * ========================================================================
 * Function name: LLFCD_HASH_MD5_SingleBlockUpdate
 *
 * Description: This module performs MD5 hashing of 512-bit block.
 *
 * Author: Victor Elkonin
 *
 * Last Rivision: 1.00.00
 *
 * Update History:
 * Rev 1.00.00, Date 23 September 2004, By Victor Elkonin: Initial version.
 *                   14 December  2004, By Adam Schapira - ported to CRYS.
 * ========================================================================
 */
void LLFCD_HASH_MD5_SingleBlockUpdate( HASHContext_t *WorkingContextID_ptr, DxUint32_t *DataIn_ptr )
{
  /* LOCAL DECLERATIONS */

  DxUint32_t a,b,c,d;
  DxInt32_t k;
  DxInt32_t m;

  /* FUNCTION LOGIC */

  /* **************** Process: ******************** */

  a = WorkingContextID_ptr->HASH_Result[0];
  b = WorkingContextID_ptr->HASH_Result[1];
  c = WorkingContextID_ptr->HASH_Result[2];
  d = WorkingContextID_ptr->HASH_Result[3];

  /* Round 1 */
  for (k=0; k<16; )
  {
    a += MD5_F (b, c, d);
    a += DataIn_ptr[k];
    a += T[k++];
    ROTATE_RIGHT_32BIT_VAR (a, a, 25);
	a += b;

    d += MD5_F (a, b, c);
    d += DataIn_ptr[k];
    d += T[k++];
    ROTATE_RIGHT_32BIT_VAR (d, d, 20);
	d += a;

    c += MD5_F (d, a, b);
    c += DataIn_ptr[k];
    c += T[k++];
    ROTATE_RIGHT_32BIT_VAR (c, c, 15);
	c += d;

    b += MD5_F (c, d, a);
    b += DataIn_ptr[k];
    b += T[k++];
    ROTATE_RIGHT_32BIT_VAR (b, b, 10);
	b += c;
  }


  m=1;
  /*  Round 2 */
  for (; k<32; )
  {
    a += MD5_G (b, c, d);
    a += DataIn_ptr[m];
    m +=5;
    m &=0xf;
    a += T[k++];
    ROTATE_RIGHT_32BIT_VAR (a, a, 27);
    a += b;

    d += MD5_G (a, b, c);
    d += DataIn_ptr[m];
    m +=5;
    m &=0xf;
    d += T[k++];
    ROTATE_RIGHT_32BIT_VAR (d, d, 23);
    d += a;

    c += MD5_G (d, a, b);
    c += DataIn_ptr[m];
    m +=5;
    m &=0xf;
    c += T[k++];
    ROTATE_RIGHT_32BIT_VAR (c, c, 18);
    c += d;

    b += MD5_G (c, d, a);
    b += DataIn_ptr[m];
    m +=5;
    m &=0xf;
    b += T[k++];
    ROTATE_RIGHT_32BIT_VAR (b, b, 12);
    b += c;
  }

  m=5;
  /* Round 3 */
  for (; k<48; )
  {
    a += MD5_H (b, c, d);
    a += DataIn_ptr[m];
    m +=3;
    m &=0xf;
    a += T[k++];
    ROTATE_RIGHT_32BIT_VAR (a, a, 28);
    a += b;

    d += MD5_H (a, b, c);
    d += DataIn_ptr[m];
    m +=3;
    m &=0xf;
    d += T[k++];
    ROTATE_RIGHT_32BIT_VAR (d, d, 21);
    d += a;

    c += MD5_H (d, a, b);
    c += DataIn_ptr[m];
    m +=3;
    m &=0xf;
    c += T[k++];
    ROTATE_RIGHT_32BIT_VAR (c, c, 16);
    c += d;

    b += MD5_H (c, d, a);
    b += DataIn_ptr[m];
    m +=3;
    m &=0xf;
    b += T[k++];
    ROTATE_RIGHT_32BIT_VAR (b, b, 9);
    b += c;
  }

  m=0;
  /*  Round 4 */
  for (; k<=60; )
  {
    a += MD5_I (b, c, d);
    a += DataIn_ptr[m];
    m +=7;
    m &=0xf;
    a += T[k++];
    ROTATE_RIGHT_32BIT_VAR (a, a, 26);
    a += b;

    d += MD5_I (a, b, c);
    d += DataIn_ptr[m];
    m +=7;
    m &=0xf;
    d += T[k++];
    ROTATE_RIGHT_32BIT_VAR (d, d, 22);
    d += a;

    c += MD5_I (d, a, b);
    c += DataIn_ptr[m];
    m +=7;
    m &=0xf;
    c += T[k++];
    ROTATE_RIGHT_32BIT_VAR (c, c, 17);
    c += d;

    b += MD5_I (c, d, a);
    b += DataIn_ptr[m];
    m +=7;
    m &=0xf;
    b += T[k++];
    ROTATE_RIGHT_32BIT_VAR (b, b, 11);
    b += c;
  }

  WorkingContextID_ptr->HASH_Result[0] += a;
  WorkingContextID_ptr->HASH_Result[1] += b;
  WorkingContextID_ptr->HASH_Result[2] += c;
  WorkingContextID_ptr->HASH_Result[3] += d;

  return;

}/* END OF LLF_HASH_MD5_SingleBlockUpdate */
