// 
//  Copyright © Renesas Mobile Corporation 2010 . All rights reserved     
// 
//  This material, including documentation and any related source code and information, is protected by 
//  copyright controlled by Renesas. All rights are reserved. Copying, including reproducing, storing, adapting,
//  translating and modifying, including decompiling or reverse engineering, any or all of this material
//  requires the prior written consent of Renesas. This material also contains confidential information, which
//  may not be disclosed to others without the prior written consent of Renesas.                                                              
//

#if defined _SEC_COMM_H_
#error "Multiply included"
#endif

//#define _SEC_COMM_H_

//uint32_t set_rpc_handler(void);
//uint32_t sec_l2_cache_enable(void);

extern uint32_t sec_l2_cache_enable(void);
extern int sec_hal_mem_msg_area_init(void);
uint32_t  sec_service_integrity_check(void);
extern void sec_service_rt_multicore_enable(void);
/// EOF
