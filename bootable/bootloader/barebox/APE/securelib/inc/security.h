/* security.h
 *
 * Copyright (C) 2012 Renesas Mobile Corporation
 * All rights reserved.
 *
 */

#ifndef _SECURITY_H_
#define _SECURITY_H_

#include "arm.h"
#include "cache_mmu_if.h"
#include "ssa.h"
#include "sec_dispatch.h"
#include "sec_msg.h"
#include "sec_serv_api.h"
#include "malloc.h"

#define SEC_OK 				0	/* Service execution successful */
#define SEC_ERR_EXEC 		1	/* Service execution failed */
#define SEC_ERR_INPUT 		2	/* Invalid input parameter received */
#define SEC_ERR_OUTPUT		3	/* Invalid output parameter received */
#define SEC_ERR_CERT_EXEC	4	/* Protected data certificate execution failed */
#define SEC_ERR_PROT_EXEC	5	/* Protected data execution failed */


typedef struct {
  uint32_t  object_id;
  uint8_t   structure_version;
  uint8_t   endianess;
  uint16_t  toc_offset;
  uint32_t  certificate_size;
  uint32_t  flags;
  uint32_t  timestamp;
  uint32_t  random;
  uint32_t  signer;
  uint32_t  rsvd_1C;
} CERTIFICATE_COMMON;

RC Secure_Check(void *certificate, unsigned long *image);
RC Secure_WDT_clear(void);

#endif /* _SECURITY_H_ */
