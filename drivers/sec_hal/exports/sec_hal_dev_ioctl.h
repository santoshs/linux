/* *********************************************************************** **
**                              Renesas                                    **
** *********************************************************************** */

/* ************************** COPYRIGHT INFORMATION ********************** **
** This program contains proprietary information that is a trade secret of **
** Renesas and also is protected as an unpublished work under              **
** applicable Copyright laws. Recipient is to retain this program in       **
** confidence and is not permitted to use or make copies thereof other than**
** as permitted in a written agreement with Renesas.                       **
**                                                                         **
** Copyright (C) 2012 Renesas Electronics Corp.                            **
** All rights reserved.                                                    **
** *********************************************************************** */
#ifndef SEC_HAL_DEV_IOCTL_H
#define SEC_HAL_DEV_IOCTL_H

#include <linux/ioctl.h>

#define SEC_STORAGE_FILENAME_MAXLEN   128
#define SEC_STORAGE_FILE_MAXLEN      3072

typedef struct
{
    unsigned int param0;
    unsigned int param1;
    unsigned int param2;
    unsigned int param3;
    unsigned int param4;
    unsigned int param5;
    unsigned int reserved1;
    unsigned int reserved2;
} sd_ioctl_params_t;

typedef struct
{
    unsigned int param0;
    unsigned int param1;
    unsigned int param2;
    unsigned int param3;
    unsigned int param4;
    unsigned int param5;
    unsigned int reserved1;
    unsigned int reserved2;
    char fname[SEC_STORAGE_FILENAME_MAXLEN];
    char data[SEC_STORAGE_FILE_MAXLEN];
} sd_rpc_params_t;

#define SD_IOCTL_PARAMS_SZ      sizeof(sd_ioctl_params_t)
#define SD_IOCTL_PARAMS_ZI      {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
#define SD_RPC_PARAMS_SZ        sizeof(sd_rpc_params_t)
#define SD_RPC_PARAMS_ZI        {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,"\0", "\0"}
#define PARAM_SZ                sizeof(unsigned int)

#define SD_MAGICNUM             's'
#define SD_STARTNUM             _IOWR(SD_MAGICNUM, 0)

#define SD_INIT                 _IOWR(SD_MAGICNUM, 1, sd_ioctl_params_t*)
#define SD_KEY_INFO             _IOWR(SD_MAGICNUM, 2, sd_ioctl_params_t*)
#define SD_CERT_REGISTER        _IOWR(SD_MAGICNUM, 3, sd_ioctl_params_t*)
#define SD_DATA_CERT_REGISTER   _IOWR(SD_MAGICNUM, 4, sd_ioctl_params_t*)
#define SD_RANDOM_NUMBER_GET    _IOWR(SD_MAGICNUM, 5, sd_ioctl_params_t*)
#define SD_MAC_ADDRESS_GET      _IOWR(SD_MAGICNUM, 6, sd_ioctl_params_t*)
#define SD_IMEI_GET             _IOWR(SD_MAGICNUM, 7, sd_ioctl_params_t*)
#define SD_RAT_BAND_GET         _IOWR(SD_MAGICNUM, 8, sd_ioctl_params_t*)
#define SD_PP_FLAGS_COUNT_GET   _IOWR(SD_MAGICNUM, 9, sd_ioctl_params_t*)
#define SD_PP_FLAGS_GET         _IOWR(SD_MAGICNUM, 10, sd_ioctl_params_t*)
#define SD_SL_LEVELS_OPEN       _IOWR(SD_MAGICNUM, 11, sd_ioctl_params_t*)
#define SD_SL_LEVEL_OPEN        _IOWR(SD_MAGICNUM, 12, sd_ioctl_params_t*)
#define SD_SL_LEVEL_STATUS_GET  _IOWR(SD_MAGICNUM, 13, sd_ioctl_params_t*)
#define SD_AUTH_DATA_SIZE_GET   _IOWR(SD_MAGICNUM, 14, sd_ioctl_params_t*)
#define SD_AUTH_DATA_GET        _IOWR(SD_MAGICNUM, 15, sd_ioctl_params_t*)
#define SD_PERIODIC_ICHECK      _IOWR(SD_MAGICNUM, 16, sd_ioctl_params_t*)
#define SD_SELFTEST             _IOWR(SD_MAGICNUM, 17, sd_ioctl_params_t*)
#define SD_TOC_READ            _IOWR(SD_MAGICNUM, 18, sd_ioctl_params_t*)
#define SD_EXIT                _IOWR(SD_MAGICNUM, 19, sd_ioctl_params_t*)


#define SD_DRM_ENTER_PLAY       _IOWR(SD_MAGICNUM, 20, sd_ioctl_params_t*)
#define SD_DRM_EXIT_PLAY        _IOWR(SD_MAGICNUM, 21, sd_ioctl_params_t*)
#define SD_DRM_SET_ENTIT_KEY    _IOWR(SD_MAGICNUM, 22, sd_ioctl_params_t*)
#define SD_DRM_DER_CTL_WORD     _IOWR(SD_MAGICNUM, 23, sd_ioctl_params_t*)
#define SD_DRM_DECRYPT_AUDIO    _IOWR(SD_MAGICNUM, 24, sd_ioctl_params_t*)
#define SD_DRM_DECRYPT_VIDEO    _IOWR(SD_MAGICNUM, 25, sd_ioctl_params_t*)
#define SD_DRM_VALID_KEYBOX     _IOWR(SD_MAGICNUM, 26, sd_ioctl_params_t*)
#define SD_DRM_DEVICE_ID_GET    _IOWR(SD_MAGICNUM, 27, sd_ioctl_params_t*)
#define SD_DRM_KEYDATA_GET      _IOWR(SD_MAGICNUM, 28, sd_ioctl_params_t*)
#define SD_DRM_WRAP_KEYBOX      _IOWR(SD_MAGICNUM, 29, sd_ioctl_params_t*)
#define SD_DRM_INSTALL_KEYBOX   _IOWR(SD_MAGICNUM, 30, sd_ioctl_params_t*)

#define SD_SIMU_DS0_TEST        _IOWR(SD_MAGICNUM, 31, sd_ioctl_params_t*)
#define SD_SIMU_DS1_TEST        _IOWR(SD_MAGICNUM, 32, sd_ioctl_params_t*)

#define SD_SEC_STORAGE_SELFTEST    _IOWR(SD_MAGICNUM, 33, sd_ioctl_params_t*)
#define SD_SECURE_STORAGE_DAEMON_PID_REGISTER     _IOWR(SD_MAGICNUM, 34, sd_ioctl_params_t*)

#define SD_TEE_INIT_CONTEXT        _IOWR(SD_MAGICNUM, 50, sd_ioctl_params_t*)
#define SD_TEE_FINALIZE_CONTEXT    _IOWR(SD_MAGICNUM, 51, sd_ioctl_params_t*)
#define SD_TEE_OPEN_SESSION        _IOWR(SD_MAGICNUM, 52, sd_ioctl_params_t*)
#define SD_TEE_CLOSE_SESSION       _IOWR(SD_MAGICNUM, 53, sd_ioctl_params_t*)
#define SD_TEE_INVOKE_COMMAND      _IOWR(SD_MAGICNUM, 54, sd_ioctl_params_t*)
#define SD_TEE_PRE_MMAP            _IOWR(SD_MAGICNUM, 55, sd_ioctl_params_t*)

#define SD_ENDNUM              _IOWR(SD_MAGICNUM, 56)



#define SD_FLAGS_RPC_ENABLED     0x00001000

#define SD_RPC_SIGNAL_ID         SIGURG

#define SD_RPC_ALLOC             0x80000001
#define SD_RPC_FREE              0x80000002
#define SD_RPC_RPMB              0x80000003
#define SD_RPC_STORAGE_INFO_GET  0x80000004
#define SD_RPC_FS_ROOT           0x80000005
#define SD_RPC_FS_LOOKUP         0x80000006
#define SD_RPC_FS_READ           0x80000007
#define SD_RPC_FS_WRITE          0x80000008
#define SD_RPC_FS_CREATE         0x80000009
#define SD_RPC_FS_REMOVE         0x8000000A
#define SD_RPC_FS_MOVE           0x8000000B
#define SD_RPC_FS_SIZE           0x8000000C
#define SD_RPC_PROT_DATA_FREE    0x8000000D
#define SD_RPC_PROT_DATA_ALLOC   0x8000000E
#define SD_RPC_TRACE             0x8000000F

#define SFS_STATUS_OK                    0
#define SFS_STATUS_FAIL                  1
#define SFS_STATUS_INVALID_CALL          2
#define SFS_STATUS_NOT_SUPPORTED         3
#define SFS_STATUS_STALE                 4
#define SFS_STATUS_INVALID_NAME          5
#define SFS_STATUS_HANDLE_NOT_DIRECTORY  6
#define SFS_STATUS_HANDLE_NOT_FILE       7
#define SFS_STATUS_NOT_FOUND             8
#define SFS_STATUS_END_OF_FILE           9
#define SFS_STATUS_NO_SPACE              10
#define SFS_STATUS_NAME_EXISTS           11
#define SFS_STATUS_NOT_EMPTY             12
#define SFS_STATUS_INVALID_MODE          13
#define SFS_STATUS_INVALID_COOKIE        14
#define SFS_STATUS_ERR_ACCESS            15


#endif /* SEC_HAL_DEV_IOCTL_H */
