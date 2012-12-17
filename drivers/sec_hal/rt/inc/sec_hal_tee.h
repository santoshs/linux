/* *********************************************************************** **
**                               Renesas                                   **
** *********************************************************************** */

/* *************************** COPYRIGHT INFORMATION ********************* **
** This program contains proprietary information that is a trade secret of **
** Renesas and also is protected as an unpublished work under              **
** applicable Copyright laws. Recipient is to retain this program in       **
** confidence and is not permitted to use or make copies thereof other than**
** as permitted in a written agreement with Renesas.                       **
**                                                                         **
** Copyright (C) 2010-2012 Renesas Electronics Corp.                       **
** All rights reserved.                                                    **
** *********************************************************************** */
#ifndef SEC_HAL_TEE_H
#define SEC_HAL_TEE_H


/* ************************ HEADER (INCLUDE) SECTION ********************* */
#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <inttypes.h>
#endif
#include "sec_hal_res.h"
#include "tee_defs.h"

/* ***************** MACROS, CONSTANTS, COMPILATION FLAGS **************** */

uint32_t sec_hal_tee_initialize_context( const char* name,
                                         TEEC_Context* context);

uint32_t sec_hal_tee_finalize_context( TEEC_Context* context);

uint32_t sec_hal_tee_open_session( TEEC_Context* context,
                          TEEC_Session* session,
                          const TEEC_UUID* destination,
                          uint32_t connectionMethod,
                          const void* connectionData,
                          TEEC_Operation* operation,
                          uint32_t* returnOrigin);

uint32_t  sec_hal_tee_close_session( TEEC_Session* session );

uint32_t sec_hal_tee_invoke_command( TEEC_Session* session,
                        uint32_t commandID,
                        TEEC_Operation* operation,
                        uint32_t* returnOrigin);

uint32_t sec_hal_tee_get_param_type(uint32_t index, uint32_t types);

uint32_t  sec_hal_tee_convert_memrefs( TEEC_Operation* user_operation,
                                       TEEC_Operation* operation,
                                       uint32_t direction );

uint32_t sec_hal_tee_register_shared_memory_area( TEEC_Context* kernel_context, TEEC_SharedMemory* kernel_shmem);

uint32_t sec_hal_tee_release_shared_memory_area(TEEC_SharedMemory* kernel_shmem);

uint32_t sec_hal_tee_allocate_TA_memory_workaround();

/* ******************************** END ********************************** */

#endif /* SEC_HAL_TEE_H */

