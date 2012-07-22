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
** Copyright (C) 2012 Renesas Electronics Corp.                              **
** All rights reserved.                                                      **
** ************************************************************************* */
#ifndef SEC_HAL_DEV_IOCTL_H
#define SEC_HAL_DEV_IOCTL_H

#include <linux/ioctl.h>


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
#define SD_IOCTL_PARAMS_SZ sizeof(sd_ioctl_params_t)
#define SD_IOCTL_PARAMS_ZI {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
#define PARAM_SZ           sizeof(unsigned int)

#define SD_MAGICNUM            's'
#define SD_STARTNUM            _IOWR(SD_MAGICNUM, 0)
#define SD_INIT                _IOWR(SD_MAGICNUM, 1, sd_ioctl_params_t*)
#define SD_KEY_INFO            _IOWR(SD_MAGICNUM, 2, sd_ioctl_params_t*)
#define SD_CERT_REGISTER       _IOWR(SD_MAGICNUM, 3, sd_ioctl_params_t*)
#define SD_DATA_CERT_REGISTER  _IOWR(SD_MAGICNUM, 4, sd_ioctl_params_t*)
#define SD_RANDOM_NUMBER_GET   _IOWR(SD_MAGICNUM, 5, sd_ioctl_params_t*)
#define SD_MAC_ADDRESS_GET     _IOWR(SD_MAGICNUM, 6, sd_ioctl_params_t*)
#define SD_IMEI_GET            _IOWR(SD_MAGICNUM, 7, sd_ioctl_params_t*)
#define SD_RAT_BAND_GET        _IOWR(SD_MAGICNUM, 8, sd_ioctl_params_t*)
#define SD_PP_FLAGS_COUNT_GET  _IOWR(SD_MAGICNUM, 9, sd_ioctl_params_t*)
#define SD_PP_FLAGS_GET        _IOWR(SD_MAGICNUM, 10, sd_ioctl_params_t*)
#define SD_SL_LEVELS_OPEN      _IOWR(SD_MAGICNUM, 11, sd_ioctl_params_t*)
#define SD_SL_LEVEL_OPEN       _IOWR(SD_MAGICNUM, 12, sd_ioctl_params_t*)
#define SD_SL_LEVEL_STATUS_GET _IOWR(SD_MAGICNUM, 13, sd_ioctl_params_t*)
#define SD_AUTH_DATA_SIZE_GET  _IOWR(SD_MAGICNUM, 14, sd_ioctl_params_t*)
#define SD_AUTH_DATA_GET       _IOWR(SD_MAGICNUM, 15, sd_ioctl_params_t*)
#define SD_PERIODIC_ICHECK     _IOWR(SD_MAGICNUM, 16, sd_ioctl_params_t*)
#define SD_SELFTEST            _IOWR(SD_MAGICNUM, 17, sd_ioctl_params_t*)
#define SD_TOC_READ            _IOWR(SD_MAGICNUM, 18, sd_ioctl_params_t*)
#define SD_EXIT                _IOWR(SD_MAGICNUM, 19, sd_ioctl_params_t*)
#define SD_ENDNUM              _IOWR(SD_MAGICNUM, 20)


#endif /* SEC_HAL_DEV_IOCTL_H */
