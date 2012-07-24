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
 
 
#ifndef __GEN__H
#define __GEN__H

/*
 * Includes
 */


#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object %name    : %
   *  State           :  %state%
   *  Creation date   :  Wed Nov 17 17:09:43 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief A brief description of this module
   *
   *  \version gen.h#1:incl:1
   *  \author adams
   */

/************************ Defines ******************************/
/* --------------------------------------------
	defines shared by a couple of modules
--------------------------------------------------*/

  


/* .................... basic Macros for accessing hardware registers ...................... */
/* ----------------------------------------------------------------------------------------- */
#define DX_GEN_ROM_WriteRegister(addr,val)  ( (*((volatile DxUint32_t*)(addr))) = (DxUint32_t)(val) )
#define DX_GEN_ROM_ReadRegister(addr,val)  ( (val) = (*((volatile DxUint32_t*)(addr))) )

#ifndef DX_ARM_INTEGRATOR_DEBUG
#define DX_GEN_WriteRegister(addr,val)  ( (*((volatile DxUint32_t*)(addr))) = (DxUint32_t)(val) )
#define DX_GEN_ReadRegister(addr,val)  ( (val) = (*((volatile DxUint32_t*)(addr))) )
#endif 

#define DX_GEN_WriteRegister16Bit(addr,val)  ( (*((volatile DxUint16_t*)(addr))) = (DxUint16_t)(val) )
#define DX_GEN_ReadRegister16Bit(addr,val)  ( (val) = (*((volatile DxUint16_t*)(addr))) )
#define DX_GEN_WriteRegister8Bit(addr,val)  ( (*((volatile DxUint8_t*)(addr))) = (DxUint8_t)(val) )
#define DX_GEN_ReadRegister8Bit(addr,val)  ( (val) = (*((volatile DxUint8_t*)(addr))) )

#ifdef DX_ARM_INTEGRATOR_DEBUG
#define DX_GEN_WriteRegister(addr,val)  ( (*((volatile DxUint32_t*)(addr))) = (DxUint32_t)(val) )
#define DX_GEN_ReadRegister(addr,val)  ( (val) = (*((volatile DxUint32_t*)(addr))) )
#endif

#define DX_GEN_INVERSE_UINT32_BYTES( val ) \
   ( ((val) >> 24) | (((val) & 0x00FF0000) >> 8) | (((val) & 0x0000FF00) << 8) | (((val) & 0x000000FF) << 24) )


/* wait for the completion of the write signal into SRAM */
#define DX_GEN_WAIT_SRAM_DATA_READY_COMPLETE() \
do \
{ \
  DxUint32_t  regVal; \
  do \
  { \
    DX_GEN_ReadRegister( GEN_HW_SRAM_DATA_READY_REG_ADDR, (regVal) ); \
  }while(!(regVal & 0x1)); \
}while(0)

#define DX_GEN_WAIT_SRAM_WRITE_COMPLETE() \
do \
{ \
  DxUint32_t  WAIT_WRITE_regVal; \
  do \
  { \
    DX_GEN_ReadRegister( GEN_HW_SRAM_DATA_READY_REG_ADDR, (WAIT_WRITE_regVal) ); \
  }while(!(WAIT_WRITE_regVal & 0x1)); \
}while(0)

#ifndef LITTLE__ENDIAN

/* set the flags responsible for converting the BIG ENDIAN data written into SRAM into LITTLE_ENDIAN */
#define DX_GEN_SET_BIG_ENDIAN_FLAG() \
do \
{ \
  DX_GEN_WriteRegister( GEN_HW_SRAM_BIG_ENDIAN_REG_ADDR, 0x1 ); \
}while(0)

/* set the flags responsible for converting the data written into SRAM */
#define DX_GEN_UNSET_BIG_ENDIAN_FLAG() \
do \
{ \
  DX_GEN_WriteRegister( GEN_HW_SRAM_BIG_ENDIAN_REG_ADDR, 0x0 ); \
}while(0)

#define DX_GEN_CHANGE_WORD_ENDIANNESS(val) \
   ( DX_GEN_INVERSE_UINT32_BYTES(val) )

#define DX_GEN_CHANGE_ENDIANNESS(startAddr, numOfWords) \
do \
{ \
  DxUint32_t  count; \
  for(count = 0; count < numOfWords; count ++) \
  { \
    *((DxUint32_t *)(startAddr + count*sizeof(DxUint32_t))) = \
    				DX_GEN_INVERSE_UINT32_BYTES((*((DxUint32_t *)(startAddr + count*sizeof(DxUint32_t))))); \
  } \
}while(0)


#else /*LITTLE__ENDIAN*/

#define DX_GEN_SET_BIG_ENDIAN_FLAG()
#define DX_GEN_UNSET_BIG_ENDIAN_FLAG()
#define DX_GEN_CHANGE_WORD_ENDIANNESS(val) (val)
#define DX_GEN_CHANGE_ENDIANNESS(startAddr, numOfWords)

#endif


/* .................... Macros for hardware registers block utilities ...................... */
/* ----------------------------------------------------------------------------------------- */





/************************ Typedefs  ****************************/


/************************ Structs  ******************************/


/************************ Public Variables **********************/


/************************ Public Functions **********************/




#ifdef __cplusplus
}
#endif

#endif





