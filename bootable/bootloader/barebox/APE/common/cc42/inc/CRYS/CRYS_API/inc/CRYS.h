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
 
 
#ifndef CRYS_H
#define CRYS_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#ifdef _INTERNAL_ROM_COMP
#include "CRYS_duplicate.h"
#endif
#include "DX_VOS_BaseTypes.h"
#include "PLAT_SystemDep.h"
#include "CRYS_Defs.h"
#include "CRYS_error.h"

#ifndef CRYS_NO_AES_SUPPORT
#include "CRYS_AES.h"
#endif

#ifndef CRYS_NO_DES_SUPPORT
#include "CRYS_DES.h"
#endif

#ifndef CRYS_NO_HASH_SUPPORT
#include "CRYS_HASH.h"
#ifndef CRYS_NO_HMAC_SUPPORT
#include "CRYS_HMAC.h"
#endif
#endif

#ifndef CRYS_NO_RND_SUPPORT
#include "CRYS_RND.h"
#endif

#ifndef CRYS_NO_KDF_SUPPORT
#include "CRYS_KDF.h"
#endif

#ifndef CRYS_NO_PKI_SUPPORT

#include "CRYS_RSA_BUILD.h"
#include "CRYS_RSA_Types.h"
#include "CRYS_RSA_PRIM.h"
#include "CRYS_RSA_SCHEMES.h"
#include "CRYS_RSA_KG.h"

#ifndef CRYS_NO_DH_SUPPORT
#ifndef CRYS_NO_KDF_SUPPORT
#include "CRYS_DH.h"
#include "CRYS_DH_KG.h"
#endif
#endif

#endif

#include "CRYS_init.h"

#ifndef CRYS_NO_OTF_SUPPORT
#include "CRYS_OTF.h"
#endif

#include "CRYS_version.h"
#include "CRYS_SELF_TEST.h"

#ifndef CRYS_NO_RC4_SUPPORT
#include "CRYS_RC4.h"
#endif

#ifndef CRYS_NO_CMLA_SUPPORT
#include "CRYS_CMLA.h"
#endif

/* CRYS_ECPKI includes */
#ifndef CRYS_NO_ECPKI_SUPPORT
	#include "CRYS_ECPKI_Types.h"
	#include "CRYS_ECPKI_error.h"
	#include "CRYS_ECPKI_KG.h"
	#include "CRYS_ECPKI_BUILD.h"
	#include "CRYS_ECPKI_ECDSA.h"
	#include "CRYS_ECPKI_ELGAMAL.h"
	#include "CRYS_ECPKI_DH.h"
#endif

#ifndef CRYS_NO_SST_SUPPORT
	#include "CRYS_SST.h"
	#include "CRYS_SST_KG.h"
#endif

#ifdef CRYS_SEP_SIDE_WORK_MODE

	#ifndef CRYS_NO_AES_SUPPORT
	#include "CRYS_AES_SEP.h"
	#endif

	#ifndef CRYS_NO_HASH_SUPPORT
	#include "CRYS_HASH_SEP.h"
	#endif

	#ifndef CRYS_NO_DES_SUPPORT
	#include "CRYS_DES_SEP.h"
	#endif

	#ifndef CRYS_NO_HMAC_SUPPORT
	#include "CRYS_HMAC_SEP.h"
	#endif

	#ifndef CRYS_NO_PKI_SUPPORT
	#include "CRYS_RSA_SEP_SCHEMES.h"
	#endif

	#ifndef CRYS_NO_ECPKI_SUPPORT
	#include "CRYS_ECPKI_ECDSA_SEP.h"
	#endif
	
	#ifndef CRYS_NO_RC4_SUPPORT
	#include "CRYS_RC4_SEP.h"
	#endif

#endif

#ifndef CRYS_NO_C2_SUPPORT
#include "CRYS_C2.h"
#endif

#ifndef CRYS_NO_RC4_SUPPORT
#include "CRYS_RC4.h"
#endif

#ifndef CRYS_NO_KMNG_SUPPORT
#include "CRYS_KMNG.h"
#endif

#ifndef CRYS_NO_SST_SUPPORT
#include "CRYS_SST.h"
#include "CRYS_SST_KG.h"
#endif

#ifndef CRYS_NO_AESCCM_SUPPORT
#include "CRYS_AESCCM.h"
#endif

#ifndef CRYS_NO_AESGCM_SUPPORT
#include "CRYS_AESGCM.h"
#endif

#ifndef CRYS_NO_OTF_MC_SUPPORT
#include "CRYS_OTF_MultiContext.h"
#endif

#ifndef CRYS_NO_FIPS_SUPPORT
#include "CRYS_FIPS.h"
#endif 

#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object %name    : %
   *  State           :  %state%
   *  Creation date   :  Wed Nov 17 16:41:49 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief A brief description of this module
   *
   *  \version CRYS.h#1:incl:1
   *  \author adams
   */


/************************ Defines ******************************/

/************************ Enums ********************************/

/************************ Typedefs  ****************************/

/************************ Structs  ******************************/

/************************ Public Variables **********************/

/************************ Public Functions **********************/

#ifdef __cplusplus
}
#endif

#endif
