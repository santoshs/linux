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
 
 
#ifndef _1_CRYS_Defs_h_H
#define _1_CRYS_Defs_h_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
 
/*#include "ModuleDefines.h"*/


#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object %name    : %
   *  State           :  %state%
   *  Creation date   :  Mon Jan 03 18:37:21 2005
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief A brief description of this module
   *
   *  \version CRYS_Defs.h#1:incl:1
   *  \author ohads
   */
#ifndef CRYS_NO_PKI_SUPPORT
#include "LLF_PKI_EngineInfo.h"
#endif

#ifndef CRYS_NO_AES_SUPPORT
#include "LLF_AES_EngineInfo.h"
#endif

#ifndef CRYS_NO_DES_SUPPORT
#include "LLF_DES_EngineInfo.h"
#endif

#ifndef CRYS_NO_HASH_SUPPORT
#include "LLF_HASH_EngineInfo.h"
#endif

#ifndef CRYS_NO_RND_SUPPORT
#include "LLF_RND_EngineInfo.h"
#endif

#ifndef CRYS_NO_RC4_SUPPORT
#include "LLF_RC4_EngineInfo.h"
#endif

#ifndef CRYS_NO_ECPKI_SUPPORT
#include "LLF_ECPKI_EngineInfo.h"
#endif

#ifndef CRYS_NO_OTF_MC_SUPPORT
//#include "LLF_OTF_MultiContext_EngineInfo.h" // for cc5
#endif


/************************ Defines ******************************/
#define CRYS_DEFS_DUMMY_AES_BLOCK_SIZE 		16

/*whether needed to export CRYS APIs for firmware testing*/
#ifdef CRYS_EXPORT_APIS_FOR_DLL
#define CEXPORT_C EXPORT_C
#define CIMPORT_C IMPORT_C
#else
#define CEXPORT_C
#define CIMPORT_C
#endif

/* CRYS HW HOST memory base address for accsess from SEP */
#define CRYS_COMMON_SEP_BASE_ADDR_OF_HOST_MEM  0xA0000000

/************************ Enums ********************************/

/* Defines the enum that is used for specifying whether or not to perform 
 * a decrypt operation when performing the AES operation mode on the Context
 */
typedef enum
{
   AES_DECRYPT_CONTEXT = 0,
   AES_DONT_DECRYPT_CONTEXT = 1,
/*  AES_ENCRYPT_RELEASE_CONTEXT = 2*/
   
   CRYS_AES_CONTEXTS_flagLast = 0x7FFFFFFF,

}CRYS_AES_CONTEXTS_flag;

/* definition of the engine types */
#define CRYS_DEFS_SW_ENGINE_TYPE       0
#define CRYS_DEFS_CC_LITE_ENGINE_TYPE  1
#define CRYS_DEFS_CC_LITE_PKI256_TYPE  2
#define CRYS_DEFS_SSDMA_ENGINE_TYPE    3
#define CRYS_DEFS_SSDMA_2_ENGINE_TYPE  4
#define CRYS_DEFS_CF_3_ENGINE_TYPE     5
#define CRYS_DEFS_CC_6_ENGINE_TYPE     6
#define CRYS_DEFS_CC6_PKA_ENGINE_TYPE  7
#define CRYS_DEFS_CF52_ENGINE_TYPE     8
#define CRYS_DEFS_CF6_ENGINE_TYPE      9
#define CRYS_DEFS_OTHER_ENGINE_TYPE    0xE
#define CRYS_DEFS_NOT_SUPPORTED        0xF


#if ((LLF_PKI_ENGINE_TYPE == CRYS_DEFS_SW_ENGINE_TYPE) && (LLF_AES_ENGINE_TYPE == CRYS_DEFS_SW_ENGINE_TYPE)  && \
	 (LLF_DES_ENGINE_TYPE == CRYS_DEFS_SW_ENGINE_TYPE) && (LLF_HASH_ENGINE_TYPE == CRYS_DEFS_SW_ENGINE_TYPE) && \
	 (LLF_RND_ENGINE_TYPE == CRYS_DEFS_SW_ENGINE_TYPE) && (LLF_RC4_ENGINE_TYPE == CRYS_DEFS_SW_ENGINE_TYPE)  && \
	 (LLF_OTF_MC_ENGINE_TYPE == CRYS_DEFS_SW_ENGINE_TYPE) )
#define CRYS_DEFS_ALL_ENGINES_SW_TYPE	1
#else
#define CRYS_DEFS_ALL_ENGINES_SW_TYPE	0
#endif

/************************ Enums ********************************/

/* The CSI Input or Output flag value, which may be provided by data In or Out pointers,
Note: last two bits of the pointer will be provide the data alignment */    
#define DX_CSI  0xFFFFFFFCUL 


/************************ Typedefs  ****************************/


/************************ Structs  ******************************/


/************************ Public Variables **********************/


/************************ Public Functions **********************/

#ifdef __cplusplus
}
#endif

#endif
