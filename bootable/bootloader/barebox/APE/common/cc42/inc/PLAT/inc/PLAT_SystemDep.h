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
 
 
#ifndef PLAT_SYSTEM_DEP_H
#define PLAT_SYSTEM_DEP_H

/*
 * Includes
 */

#include "DX_VOS_BaseTypes.h" 
#include "CRYS_test_flags.h"

#if CRYS_TST_HARDWARE_REG_PRINT_MODE
#include "stdio.h"
#endif

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
   *  \version PLAT_SystemDep.h#1:incl:1
   *  \author adams
   */

/************************ Defines ******************************/

/* .................... basic Macros for accessing hardware registers ...................... */
/* ----------------------------------------------------------------------------------------- */

#ifdef DX_SPANSION_HW_WORKAROUND
	/*working with spansion HW demands workaround for register access*/
	#include "PLAT_SpanDep.h"

#else /*DX_SPANSION_HW_WORKAROUND*/
	#if CRYS_TST_HARDWARE_REG_PRINT_MODE

		#define CRYS_PLAT_SYS_WriteRegister(addr,val) \
		do \
		{ \
		  ( (*((volatile DxUint32_t*)(addr))) = (DxUint32_t)(val) ); \
		  printf( "master_write(32'h%08lX , 32'h%08lX); \n",(addr),(val)); \
		}while(0)   

		#if CRYS_TST_HARDWARE_REG_SIM_PRINT_MODE

			#define CRYS_PLAT_SYS_ReadRegister(addr,val) \
			do \
			{ \
			  ( (val) = (*((volatile DxUint32_t*)(addr))) ); \
			  printf( "master_read_expect(32'h%08lX , 32'h%08lX); \n",(addr),(DxUint32_t)(val)); \
			}while(0)   

		#else /* CRYS_TST_HARDWARE_REG_SIM_PRINT_MODE */

			#define CRYS_PLAT_SYS_ReadRegister(addr,val) \
			do \
			{ \
			  ( (val) = (*((volatile DxUint32_t*)(addr))) ); \
			  printf( "master_read(32'h%08lX , 32'h%08lX); \n",(addr),(DxUint32_t)(val)); \
			}while(0)   

		#endif /* CRYS_TST_HARDWARE_REG_SIM_PRINT_MODE */

	#else /* CRYS_TST_HARDWARE_REG_PRINT_MODE */

		#define CRYS_PLAT_SYS_WriteRegister(addr,val)  ( (*((volatile DxUint32_t*)(addr))) = (DxUint32_t)(val) )
		#define CRYS_PLAT_SYS_ReadRegister(addr,val)  ( (val) = (*((volatile DxUint32_t*)(addr))) )


	#endif /* CRYS_TST_HARDWARE_REG_PRINT_MODE */

#endif /*DX_SPANSION_HW_WORKAROUND*/
/* .................... Macros for hardware registers block utilities ...................... */
/* ----------------------------------------------------------------------------------------- */

#define CRYS_PLAT_SYS_WriteRegistersBlock( addr , buffer , size_in_words ) \
do \
{ \
  DxUint32_t im0,km0; \
  for ( im0 = 0 , km0=(addr) ; im0 < (size_in_words) ; km0+=sizeof(DxUint32_t) , im0++ ) \
  { \
    CRYS_PLAT_SYS_WriteRegister( km0 , (buffer)[im0] ); \
  } \
}while(0)       

#define CRYS_PLAT_SYS_ClearRegistersBlock( addr , size_in_words ) \
do \
{ \
  DxUint32_t im0,km0; \
  for ( im0 = 0 , km0=(addr) ; im0 < (size_in_words) ; km0+=sizeof(DxUint32_t) , im0++ ) \
  { \
    CRYS_PLAT_SYS_WriteRegister( km0 , 0UL ); \
  } \
}while(0)       


#define CRYS_PLAT_SYS_ReadRegistersBlock( addr , buffer , size_in_words ) \
do \
{ \
  DxUint32_t im0,km0; \
  for ( im0 = 0 , km0=(addr) ; im0 < (size_in_words) ; km0+=sizeof(DxUint32_t) , im0++ ) \
  { \
    CRYS_PLAT_SYS_ReadRegister( km0 , (buffer)[im0] ); \
  } \
}while(0)       

/************************ Enums ********************************/


/************************ Typedefs  ****************************/


/************************ Structs  ******************************/


/************************ Public Variables **********************/


/************************ Public Functions **********************/

/**
 * @brief The folowing function tap the SEP watchdog . 
 *        
 *
 */
 /* ********************* Provate Functions *******************************/

void PLAT_WatchDogReset( void );
/**
 * @brief The folowing function handels the printings logging of the develop group .
 *        This function should be called by using the PLAT_LOG_DEV_PLOT macro 
 *        ( we do not want to actully call a function if the printings are not enabled ). 
 *        
 *
 * @param[in] level - the printing level if it is below the CRYS_LOG_DEV_MAX_LEVEL_ENABLED
 *                    flag setting it will not be printed.
 *
 * @param[in] format,... - the printf format. 
 *
 */

#if CRYS_LOG_DEV_MAX_LEVEL_ENABLED
void PLAT_LOG_DEV( DxUint8_t level , const char *format,... );
#endif

#if CRYS_LOG_DEV_MAX_LEVEL_ENABLED
#define PLAT_LOG_DEV_PRINT(args) (PLAT_LOG_DEV args)
#else
#define PLAT_LOG_DEV_PRINT(args)   
#endif

/**
 * @brief The folowing function handels the printings logging of the testing group . 
 *        This function should be called by using the PLAT_LOG_TST_PLOT macro  
 *
 * @param[in] level - the printing level if it is below the CRYS_LOG_TST_MAX_LEVEL_ENABLED
 *                    flag setting it will not be printed.
 *
 * @param[in] format,... - the printf format. 
 *
 */

#if CRYS_LOG_TST_MAX_LEVEL_ENABLED
void PLAT_LOG_TST( DxUint8_t level , const char *format,... );
#endif

#if CRYS_LOG_TST_MAX_LEVEL_ENABLED
#define PLAT_LOG_TST_PRINT(args) (PLAT_LOG_TST args)
#else
#define PLAT_LOG_TST_PRINT(args)   
#endif

/**
 * @brief The folowing function prints a buffer to the Maple format on Little endian. 
 *        
 *
 * @param[in] level - the printing level if it is below the CRYS_LOG_DEV_MAX_LEVEL_ENABLED
 *                    flag setting it will not be printed.
 *
 * @param[in] Label - a lable string.
 * @param[in] Buffer - the buffer to be printed.
 * @param[in] Size  - the buffer size in bytes.
 * 
 */
#if CRYS_LOG_DEV_MAX_LEVEL_ENABLED
void PLAT_LOG_DEV_DisplayDataMapleLittleEndian(DxUint8_t level ,char  *Label,DxUint8_t *Buffer,DxUint32_t Size );
#endif

#if CRYS_LOG_DEV_MAX_LEVEL_ENABLED
#define PLAT_LOG_DEV_PRINT_DisplayDataMapleLittleEndian(args) (PLAT_LOG_DEV_DisplayDataMapleLittleEndian args)
#else
#define PLAT_LOG_DEV_PRINT_DisplayDataMapleLittleEndian(args)   
#endif

/**
 * @brief The folowing function prints a buffer to the Maple format on Big endian. 
 *        
 *
 * @param[in] level - the printing level if it is below the CRYS_LOG_DEV_MAX_LEVEL_ENABLED
 *                    flag setting it will not be printed.
 *
 * @param[in] Label - a lable string.
 * @param[in] Buffer - the buffer to be printed.
 * @param[in] Size  - the buffer size in bytes.
 * 
 */
#if CRYS_LOG_DEV_MAX_LEVEL_ENABLED
void PLAT_LOG_DEV_DisplayDataMapleBigEndian(DxUint8_t level ,char  *Label,DxUint8_t *Buffer,DxUint32_t Size );
#endif

#if CRYS_LOG_DEV_MAX_LEVEL_ENABLED
#define PLAT_LOG_DEV_PRINT_DisplayDataMapleBigEndian(args) (PLAT_LOG_DEV_DisplayDataMapleBigEndian args)
#else
#define PLAT_LOG_DEV_PRINT_DisplayDataMapleBigEndian(args)   
#endif

/**
 * @brief The folowing function prints a buffer. 
 *        
 *
 * @param[in] level - the printing level if it is below the CRYS_LOG_DEV_MAX_LEVEL_ENABLED
 *                    flag setting it will not be printed.
 *
 * @param[in] Label - a lable string.
 * @param[in] Buffer - the buffer to be printed.
 * @param[in] Size  - the buffer size in bytes.
 * 
 */
#if CRYS_LOG_DEV_MAX_LEVEL_ENABLED
void PLAT_LOG_DEV_DisplayBuffer(DxUint8_t level , char  *Label,DxUint8_t *Buffer,DxUint32_t Size );
#endif

#if CRYS_LOG_DEV_MAX_LEVEL_ENABLED
#define PLAT_LOG_DEV_PRINT_DisplayBuffer(args) (PLAT_LOG_DEV_DisplayBuffer args)
#else
#define PLAT_LOG_DEV_PRINT_DisplayBuffer(args)   
#endif

/**
 * @brief The folowing function prints a buffer to the Maple format on Little endian. 
 *        
 *
 * @param[in] level - the printing level if it is below the CRYS_LOG_TST_MAX_LEVEL_ENABLED
 *                    flag setting it will not be printed.
 *
 * @param[in] Label - a lable string.
 * @param[in] Buffer - the buffer to be printed.
 * @param[in] Size  - the buffer size in bytes.
 * 
 */
#if CRYS_LOG_TST_MAX_LEVEL_ENABLED
void PLAT_LOG_TST_DisplayDataMapleLittleEndian(DxUint8_t level ,char  *Label,DxUint8_t *Buffer,DxUint32_t Size );
#endif

#if CRYS_LOG_TST_MAX_LEVEL_ENABLED
#define PLAT_LOG_TST_PRINT_DisplayDataMapleLittleEndian(args) (PLAT_LOG_TST_DisplayDataMapleLittleEndian args)
#else
#define PLAT_LOG_TST_PRINT_DisplayDataMapleLittleEndian(args)   
#endif

/**
 * @brief The folowing function prints a buffer to the Maple format on Big endian. 
 *        
 *
 * @param[in] level - the printing level if it is below the CRYS_LOG_TST_MAX_LEVEL_ENABLED
 *                    flag setting it will not be printed.
 *
 * @param[in] Label - a lable string.
 * @param[in] Buffer - the buffer to be printed.
 * @param[in] Size  - the buffer size in bytes.
 * 
 */
#if CRYS_LOG_TST_MAX_LEVEL_ENABLED
void PLAT_LOG_TST_DisplayDataMapleBigEndian(DxUint8_t level ,char  *Label,DxUint8_t *Buffer,DxUint32_t Size );
#endif

#if CRYS_LOG_TST_MAX_LEVEL_ENABLED
#define PLAT_LOG_TST_PRINT_DisplayDataMapleBigEndian(args) (PLAT_LOG_TST_DisplayDataMapleBigEndian args)
#else
#define PLAT_LOG_TST_PRINT_DisplayDataMapleBigEndian(args)   
#endif

/**
 * @brief The folowing function prints a buffer. 
 *        
 *
 * @param[in] level - the printing level if it is below the CRYS_LOG_TST_MAX_LEVEL_ENABLED
 *                    flag setting it will not be printed.
 *
 * @param[in] Label - a lable string.
 * @param[in] Buffer - the buffer to be printed.
 * @param[in] Size  - the buffer size in bytes.
 * 
 */
#if CRYS_LOG_TST_MAX_LEVEL_ENABLED
void PLAT_LOG_TST_DisplayBuffer(DxUint8_t level , char  *Label,DxUint8_t *Buffer,DxUint32_t Size );
#endif

#if CRYS_LOG_TST_MAX_LEVEL_ENABLED
#define PLAT_LOG_TST_PRINT_DisplayBuffer(args) (PLAT_LOG_TST_DisplayBuffer args)
#else
#define PLAT_LOG_TST_PRINT_DisplayBuffer(args)   
#endif



#ifdef __cplusplus
}
#endif

#endif





