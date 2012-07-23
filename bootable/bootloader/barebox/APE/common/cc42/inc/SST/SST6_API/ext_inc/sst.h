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
 
 /*! \file 
**********************************************************************************	
* Title:						Discretix SST API header file						 					
*																			
* Filename:					    sst.h															
*																			
* Project, Target, subsystem:	SST 6.0, API
* 
* Created:						29.03.2007														
*
* Modified:						07.06.2007										
*
* \Author						Raviv levi														
*																			
* \Remarks						
**********************************************************************************/


#ifndef _DX_SST_API_H_
    #define _DX_SST_API_H_
#ifdef __cplusplus
extern "C" {
#endif
	/*----------- Doxygen Main Page text -----------------------------------------*/
	/*! \mainpage SST6 API
			
			<h1>SST6 API Documentation</h1>
			<p>
				<img width="268" height="148" src="DxLogo.jpg">
			</p>
			<P class="ConfidentialNotice" style="TEXT-ALIGN: justify">
				This document may be used 
				in its complete form only and is solely for the use of Discretix employees and 
                authorized Discretix channels or customers. The material herein is proprietary 
                to Discretix Ltd., any unauthorized reproduction of any part thereof is 
                strictly prohibited. The contents of this document are believed to be accurate 
				at the time of distribution. Discretix reserves the right to alter this 
				information at any time without notice. The primary distribution media of this 
				document is soft copy.
			</P>
			</DIV>
	**/

	/*----------- Global Includes ------------------------------------------------*/
    #include "DX_VOS_BaseTypes.h" 
    /*----------- SST Internal Includes ------------------------------------------*/
    #include "sst_version.h"
    #include "sst_authentication.h"
    #include "sst_data_operations.h"
    #include "sst_data_iterators.h"
    #include "sst_key_management.h"
    #include "sst_errors.h"
    #include "sst_init_terminate.h"
	#include "sst_utility.h"
    #include "sst_mm.h"
    /*----------- Global defines -------------------------------------------------*/
   

#ifdef __cplusplus
}
#endif
#endif  /* _DX_SST_API_H_ */




