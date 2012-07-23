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
 
 
#ifndef LLF_AES_RESTRICT_H
#define LLF_AES_RESTRICT_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "DX_VOS_BaseTypes.h"
#include "CRYS_AES.h"
#include "LLF_AES_error.h"


#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object %name    : %
   *  State           :  %state%
   *  Creation date   :  01 Apr. 2010
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief A brief description of this module
   *
   *  \version LLF_AES.h#1:incl:1
   *  \author R.Levin
   */




/************************ Defines ******************************/

/************************ Enums ********************************/

/************************ Typedefs  ****************************/

/* The AES restricted context data-base used by some algorithms on the low level */
typedef struct  
{ 
	DxUint32_t virtualHwBaseAddr;
	
	CRYS_AES_OperationMode_t aesMode;
	CRYS_AES_EncryptMode_t   encryptMode;
	
}AESResrtictContext_t; 

/************************ Structs  ******************************/

/************************ Public Variables **********************/

/************************ Public Functions **********************/


/******************************************************************************************/
/********************        Restricted LLF AES Functions     *****************************/
/******************************************************************************************/

/************************************************************************************/
/**
 * @brief The low level LLF_AES_RESTR_StartInit function gets the AES HW semaphore.
 *
 *   The function must be called once before starting AES_RESTRICTED processing.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */                              
CRYSError_t  LLF_AES_RESTR_StartInit ( void );


/************************************************************************************/
/**
 * @brief The low level LLF_RND_AES_Init function initializes the hardware to operate 
 *        AES at CBC-MAC or CTR modes
 *
 *
 * @param[in] key_ptr - The pointer to AES key.
 * @param[in] keySizeWords  -  The key size in words.
 * @param[in] iv_ptr        -  The pointer to the initial (chaining) value.
 * @param[in] aesMode       -  AES Operation mode.
 * @param[in] encrMode      -  AES encypt-decrypt mode.
 * @param[in] aesCont_ptr   -  The pointer to restricted AES context used for specific operations.
 *                             In HW implementation it contains virtual Hw Base address.  
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */                              
CRYSError_t  LLF_AES_RESTR_Init (	                       
	                       DxUint32_t      *key_ptr,
	                       DxUint16_t       keySizeWords,
	                       DxUint32_t      *iv_ptr,
	                       CRYS_AES_OperationMode_t aesMode,
	                       CRYS_AES_EncryptMode_t   encrMode,
	                       AESResrtictContext_t *aesCont_ptr );
	                       

/****************************************************************************************/
/**
 * @brief The function performs AES operations on full AES blocks of data.
 *        
 * @param[in] in_ptr - The pointer to the input buffer.
 * @param[in] out_ptr - The pointer to the output buffer.
 * @param[in] sizeInBlocks - size of data in AES blocks.
 * @param[in] aesCont_ptr   -  The pointer to restricted AES context used for specific operations.
 *                             In HW implementation it contains virtual Hw Base address.  
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
CRYSError_t LLF_AES_RESTR_BlockExec( 
                        DxUint32_t *in_ptr,
                        DxUint32_t *out_ptr,
                        DxUint32_t  sizeInBlocks,
                        AESResrtictContext_t *aesCont_ptr );
                        
                        
/*********************************************************************************************************/
/**
 * @brief The low level LLF_RND_AES_Finish reads the last block from the DATA_OUT.
 *        and closes the hardware clock.     
 *         
 * @param[in] v_ptr         -  The pointer to the initial (chaining) or CTR value.
 * @param[in] aesCont_ptr   -  The pointer to restricted AES context used for specific operations.
 *                             In HW implementation it contains virtual Hw Base address.  
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */                              
CRYSError_t  LLF_AES_RESTR_Finish( 
	                       DxUint32_t      *v_ptr,              /*in*/
                    	   AESResrtictContext_t *aesCont_ptr ); /*in*/  
                    	   
/************************************************************************************/
/**
 * @brief The low level LLF_AES_RESTR_EndFinish function releases the AES HW semaphore.
 *
 *   The function must be called after finish AES_RESTRICTED processing.
 *
 * @return - no return value
 *
 */                              
void  LLF_AES_RESTR_EndFinish ( void );

                    	   
/*********************************************************************************************************/
/**
 * @brief The LLF_AES_RESTR_ReadIV function gets the IV fron HW registers in MAC or CBC modes 
 *         
 * @param[in] iv_ptr        -  The pointer to the initial (chaining) value.
 * @param[in] aesCont_ptr   -  The pointer to restricted AES context used for specific operations.
 *                             In HW implementation it contains virtual Hw Base address.  
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */                              
void  LLF_AES_RESTR_ReadIV( 
                       DxUint32_t           *iv_ptr,        /*in*/
                	   AESResrtictContext_t *aesCont_ptr ); /*in*/
                	   
                	   
/****************************************************************************************/
/**
 * @brief This macro performs AES-MAC block an used by the RND for time optimizations.
 *        
 *      
 * @param[in] VirtualHwBaseAddr - The base address.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
#define LLF_AES_RESTR_MacBlockExec( virtualHwBaseAddr, in_ptr, sizeInBlocks ) \
 { \
    DxUint32_t i;  \
    for( i = 0 ; i < (sizeInBlocks); i++ ) \
    { \
        CRYS_PLAT_SYS_WriteRegistersBlock( (virtualHwBaseAddr) + LLF_AES_HW_DIN_DOUT_ADDR, \
                                           &(in_ptr)[4*i*CRYS_AES_BLOCK_SIZE_IN_WORDS], \
                                           CRYS_AES_BLOCK_SIZE_IN_WORDS ); \
	   	LLF_AES_HW_WAIT_ON_AES_BUSY_BIT( virtualHwBaseAddr ); \
    } \
 }

                	   
                    	                         	                       
          
#ifdef __cplusplus
}
#endif

#endif


