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
 
 
#ifndef LLF_AESGCM_HWDEFS_H
#define LLF_AESGCM_HWDEFS_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "cc_hw_defs.h"
#include "LLF_AES_HwDefs.h"


#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object % LLF_AESGCM_HwDefs.h    : %
   *  State           :  %state%
   *  Creation date   :  Apr. 04  2011
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief This file contains the hardware addresses of the registers relevant to the AES module.
   *
   *  \version LLF_AESGCM_HwDefs.h#1:incl:1
   *  \author R.Levin
   */




/************************ Defines ******************************/

/* define minimal size of multi-LLI entry in bytes */
#define LLF_AESGCM_MINIMAL_MLLI_ENTRY_SIZE_BYTES  8

/* the definitions of the specific AES hardware registers used in AESGCM. Addresses are 
   given as offset above the base address, defined by the user and the relevant bits */
#define LLF_AESGCM_HW_CRYPTO_CTL_ADDR             HW_CRYPTO_CTL_REG_ADDR     /*0x0900UL*/
   
/* AES control register fields for tunnel mode works in AESGCM */
#define LLF_AESGCM_HW_AES_CTL_TUNNEL_IS_ON_VAL             (0x0001UL << 10 )
#define LLF_AESGCM_HW_AES_CTL_TUN_B1_USES_PADDED_DATA_IN   (0x0001UL << 23 )  /* dataIn (padded) -> B1 */ 
#define LLF_AESGCM_HW_AES_CTL_TUN_B0_ENCRYPT_VAL           (0x0001UL << 24 )  /* B0 do encrypt */
#define LLF_AESGCM_HW_AES_CTL_OUT_MID_TUNNEL_DATA_VAL      (0x0001UL << 25 )  /* enable output data from B0 into DOUT */ 
#define LLF_AESGCM_HW_AES_CTL_TUNNEL_B1_PAD_EN_VAL         (0x0001UL << 26 )  /* enable padding of B0 output (for input to B1 only) */
#define LLF_AESGCM_HW_AES_CTL_PAD_DATA_IN_VAL              (0x0001UL << 27 )  /* enable padding of input data */
					    

/* Crypto flow control values */
#define LLF_AESGCM_HW_CRYPTO_FLOW_AES                     0x0001UL
#define LLF_AESGCM_HW_CRYPTO_FLOW_HASH                    0x0007UL
#define LLF_AESGCM_HW_CRYPTO_FLOW_AES_AND_HASH            0x0003UL
#define LLF_AESGCM_HW_CRYPTO_FLOW_AES_TO_HASH_AND_DOUT    0x000AUL

/* HASH and GHASH registers */
#define LLF_AESGCM_HW_HASH_CONTROL_ADDR           HW_HASH_CONTROL_REG_ADDR       /*0x07C0UL*/
#define LLF_AESGCM_HW_HASH_CLK_ENABLE_ADDR        HW_HASH_CLK_ENABLE_REG_ADDR    /*0x0818UL*/

#define   LLF_AESGCM_HW_GHASH_SUBKEY0_REG_ADDR    HW_GHASH_SUBKEY0_REG_ADDR      /*0x700UL*/
#define   LLF_AESGCM_HW_GHASH_SUBKEY1_REG_ADDR    HW_GHASH_SUBKEY1_REG_ADDR      /*0x704UL*/
#define   LLF_AESGCM_HW_GHASH_SUBKEY2_REG_ADDR    HW_GHASH_SUBKEY2_REG_ADDR      /*0x708UL*/
#define   LLF_AESGCM_HW_GHASH_SUBKEY3_REG_ADDR    HW_GHASH_SUBKEY3_REG_ADDR      /*0x70CUL*/
#define   LLF_AESGCM_HW_GHASH_DIGEST0_REG_ADDR    HW_GHASH_DIGEST0_REG_ADDR      /*0x710UL*/
#define   LLF_AESGCM_HW_GHASH_DIGEST1_REG_ADDR    HW_GHASH_DIGEST1_REG_ADDR      /*0x714UL*/
#define   LLF_AESGCM_HW_GHASH_DIGEST2_REG_ADDR    HW_GHASH_DIGEST2_REG_ADDR      /*0x718UL*/
#define   LLF_AESGCM_HW_GHASH_DIGEST3_REG_ADDR    HW_GHASH_DIGEST3_REG_ADDR      /*0x71CUL*/
																			
#define   LLF_AESGCM_HW_GHASH_START_GCM_REG_ADDR   HW_GHASH_START_GCM_REG_ADDR   /*0x720UL*/

#define LLF_AESGCM_HW_HASH_CTL_GHASH_VAL      0x0010UL

/* LLI word1 bits */ 								     
#define   FIRST_ENTRY   (1UL << 30)								     
#define   LAST_ENTRY    (1UL << 31)

/*  Addresses realted to Busy bits used in GHASH */
#define   LLF_AESGCM_HW_DIN_MEM_DMA_BUSY_REG_ADDR  HW_DIN_MEM_DMA_BUSY_REG_ADDR							     
								     
								     
								     
/* ********************** Macros **  ***************************** */

#define LLF_AESGCM_HW_WAIT_ON_REGISTER( VirtualHwBaseAddr, RegisterAddr )  LLF_AES_HW_WAIT_ON_REGISTER( (VirtualHwBaseAddr), (RegisterAddr) )

#define LLF_AESGCM_HW_WAIT_ON_REGISTER_VALUE( VirtualHwBaseAddr, RegisterAddr, RegValue ) \
do \
{ \
   volatile DxUint32_t output_reg_val;\
   for(output_reg_val = 0; (DxInt32_t)output_reg_val < LLF_AES_BUSY_DELAYS_COUNT ; output_reg_val++);\
   do \
   { \
      CRYS_PLAT_SYS_ReadRegister( (VirtualHwBaseAddr) + (RegisterAddr) , output_reg_val ); \
   }while( (output_reg_val & 0x1) != (RegValue) ); \
}while(0)    
								     
/************************ Enums ********************************/
								      
#ifdef __cplusplus					  
}									  
#endif								  
									  
#endif								  
									  
									  

									  
