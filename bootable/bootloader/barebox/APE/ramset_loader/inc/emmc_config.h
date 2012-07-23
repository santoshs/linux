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
** Copyright (C) 2011-2012 Renesas Mobile Corp.                              **
** All rights reserved.                                                      **
** ************************************************************************* */

/**
 * @file  emmc_config.h
 * @brief Configuration file
 *
 */

#ifndef __EMMC_CONFIG_H__
#define __EMMC_CONFIG_H__

/* ************************ HEADER (INCLUDE) SECTION *********************** */

/* ***************** MACROS, CONSTANTS, COMPILATION FLAGS ****************** */

/** @brief Register address virtual address mapping.
 */
#define MMCIF_TOP_ADDR          0xE6BD0000              /**< HS-MMC module (virtual) */
#define ROM_EMMC_ERROR_ADDRESS  0xC0001B40
#define EMMC_ERRINFO_BASE       ROM_EMMC_ERROR_ADDRESS  /**< Error Log Information(virtual) */

/** @brief MMC clock setting
 */
#define EMMC_HOST_CLOCK         0520000000UL            /* HP clock : 52MHz */
#define EMMC_CLK_FREQ_200K      203125UL                /* 203.125KHz */
#define EMMC_CLK_FREQ_20M       13000000UL              /* 13MHz */
#define EMMC_CLK_FREQ_26M       26000000UL              /* 26MHz */
#define EMMC_CLK_FREQ_52M       52000000UL              /* 52MHz */

#define EMMC_CLK_REG_200K       0x00072410UL            /* 203.125kHz, RspBusy=1290ms, Data=161ms  */
#define EMMC_CLK_REG_20M        0x00012A70UL            /* 13.0MHz, RspBusy=1290ms, Data=161ms */
#define EMMC_CLK_REG_26M        0x00002B80UL            /* 26.0MHz, RspBusy=1290ms, Data=161ms */
#define EMMC_CLK_REG_52M        EMMC_CLK_REG_26M        /* not used 52MHz */

/** @brief MMC driver config
 */
#define EMMC_RCA                0x0001                  /* RCA = 1 */
#define EMMC_RW_DATA_TIMEOUT    400                     /* 400ms */
#define EMMC_RETRY_COUNT        0                       /* how many times to try after fail. Don't change. */
#define TIMER_1MS               1                       /* use ms timer. Don't change */
#define EMMC_CMD_MAX            60                      /* Don't change. */

/** @brief for DMAC 
 */
#define EMMC_DMAC_CH            0                       /**< DMAC channel number */
#define EMMC_DMAC_PRIORITY      ROM_DMAC_PRIORITY_15    /**< priority */

/** @brief etc
 */
#define LOADIMAGE_FLAGS_DMA_ENABLE              0x00000001UL

/* ********************** STRUCTURES, TYPE DEFINITIONS ********************* */

/* ********************** DECLARATION OF EXTERNAL DATA ********************* */

/* ************************** FUNCTION PROTOTYPES ************************** */

/* ********************************* CODE ********************************** */

#endif  /* #ifndef __EMMC_CONFIG_H__ */
/* ******************************** END ************************************ */

