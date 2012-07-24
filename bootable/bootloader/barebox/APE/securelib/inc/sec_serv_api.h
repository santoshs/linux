/* ************************************************************************* **
**                               Renesas                                     **
** ************************************************************************* */

/* *************************** COPYRIGHT INFORMATION *********************** **
** This program contains proprietary information that is a trade secret of   **
** Renesas and also is protected as an unpublished work under                **
** applicable Copyright laws. Recipient is to retain this program in         **
** confidence and is not permitted to use or make copies thereof other than  **
** as permitted in a written agreement with Renesas.                         **
**                                                                           **
** All rights reserved. Company confidential.                                **
* ************************************************************************** */

#ifndef _SEC_SERV_API_H_
#define _SEC_SERV_API_H_

#include "sec_type.h"

/*!
 * This is the type of the status expected from a secure service.
 */
typedef uint32_t sec_serv_status_t;

/*!
 * RPC messages from Secure Environment
 */
#define SEC_SERV_RPC_ALLOC                      0x80000001
#define SEC_SERV_RPC_FREE                       0x80000002
#define SEC_SERV_RPC_RPMB                       0x80000003
#define SEC_SERV_RPC_STORAGE_INFO_GET           0x80000004
#define SEC_SERV_RPC_FS_ROOT                    0x80000005
#define SEC_SERV_RPC_FS_LOOKUP                  0x80000006
#define SEC_SERV_RPC_FS_READ                    0x80000007
#define SEC_SERV_RPC_FS_WRITE                   0x80000008
#define SEC_SERV_RPC_FS_CREATE                  0x80000009
#define SEC_SERV_RPC_FS_REMOVE                  0x8000000A
#define SEC_SERV_RPC_FS_MOVE                    0x8000000B
#define SEC_SERV_RPC_FS_SIZE                    0x8000000C
#define SEC_SERV_RPC_PROT_DATA_FREE             0x8000000D
#define SEC_SERV_RPC_PROT_DATA_ALLOC            0x8000000E
#define SEC_SERV_RPC_TRACE                      0x8000000F

/*!
 * Defined secure services. See [1].
 */
#define SEC_SERV_FORCE_RESET                    0x10000001
#define SEC_SERV_CERTIFICATE_REGISTER           0x10000002
#define SEC_SERV_ISSW_REGISTER                  0x10000003
#define SEC_SERV_IMEI_REQUEST                   0x10000004
#define SEC_SERV_MAC_ADDRESS_REQUEST            0x10000005
#define SEC_SERV_DEVICE_AUTH_DATA_REQUEST       0x10000006
#define SEC_SERV_DEVICE_AUTH_DATA_SIZE_REQUEST  0x10000007
#define SEC_SERV_KEY_INFO_REQUEST               0x10000008
#define SEC_SERV_RANDOM_DATA_REQUEST            0x10000009
#define SEC_SERV_RAT_BAND_INFO_REQUEST          0x1000000A
#define SEC_SERV_PP_FLAGS_REQUEST               0x1000000B
#define SEC_SERV_PP_FLAGS_SIZE_REQUEST          0x1000000C
#define SEC_SERV_INTEGRITY_CHECK                0x1000000D
#define SEC_SERV_PROT_DATA_REGISTER             0x1000000E
#define SEC_SERV_SIMLOCK_CHECK_LOCKS            0x1000000F
#define SEC_SERV_SIMLOCK_CHECK_ONE_LOCK         0x10000010
#define SEC_SERV_SIMLOCK_GET_STATE              0x10000011
#define SEC_SERV_RUNTIME_INIT                   0x10000012
#define SEC_SERV_SELFTEST                       0x10000013
#define SEC_SERV_RPC_ADDRESS                    0x10000014
#define SEC_SERV_L2_CACHE_CONTROL               0x10000015

/*!
 * Service status values
 */
#define SEC_SERV_STATUS_OK                      0x00000000
#define SEC_SERV_STATUS_FAIL                    0x00000001
#define SEC_SERV_STATUS_INVALID_INPUT           0x00000002
#define SEC_SERV_STATUS_INVALID_OUTPUT          0x00000003
#define SEC_SERV_STATUS_PROT_DATA_CERT_FAIL     0x00000004
#define SEC_SERV_STATUS_PROT_DATA_FAIL          0x00000005
/*
 * Protected data types
 */
#define SEC_PROT_TYPE_LIST                      0x00000001
#define SEC_PROT_TYPE_DATA                      0x00000002

#ifndef SECURE_ENVIRONMENT
/* These are return values from the dispatcher. */
#define SEC_ROM_RET_OK                          0x0001
#define SEC_ROM_RET_NON_SUPPORTED_SERV          0x0002
#define SEC_ROM_RET_FAIL                        0x0009
#endif /* SECURE_ENVIRONMENT */

#endif /* _SEC_SERV_API_H_ */
