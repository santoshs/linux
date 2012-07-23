/*! \file 
**********************************************************************************	
* Title:						Discretix SST VCRYS Configurations Header File						 	
*																			
* Filename:						sst_vcrys_config.h 															
*																			
* Project, Target, subsystem:	SST 6.0 Virtual Cryptography Services
* 
* Created:						05.03.2007														
*
* Modified:						07.06.2007										
*
* \Author						Einat Ron														
*																			
* \Remarks						Copyright (C) 2006 by Discretix Technologies Ltd.     			
*  								All Rights reserved											
**********************************************************************************/


#ifndef _DX_SST_VCRYS_CONFIG_H_
    #define _DX_SST_VCRYS_CONFIG_H_

	#define SST_VCRYS_NUM_DESCRIPTOR_ENTRIES			(2UL)
	
	#define SST_VCRYS_RSA_MODULU_SIZE					(256UL)			/*size in bits 2048*/
	#define SST_VCRYS_RSA_PUBLIC_EXPONENT				{0x01,0x00,0x01} /* == 2^16+1 */
	
	#define SST_VCRYS_MAC_IV_COUNTER_INIT				{0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,\
														0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0}

	#define SST_VCRYS_SINGLE_DECRYPT_IV_COUNTER_INIT	{0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,\
														 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0}

		
#endif  /* _DX_SST_VCRYS_CONFIG_H_ */
