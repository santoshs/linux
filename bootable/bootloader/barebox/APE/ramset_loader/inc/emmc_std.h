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
 * @file  emmc_std.h
 * @brief eMMC boot is expecting this header file
 *
 */

#ifndef __EMMC_STD_H__
#define __EMMC_STD_H__

/* ************************ HEADER (INCLUDE) SECTION *********************** */
#include "emmc_hal.h"

/* ***************** MACROS, CONSTANTS, COMPILATION FLAGS ****************** */

#define EMMC_WAIT(x)                    (emmc_wait_ms((x)))

/** @brief 32bit registers
 **/
#define SETR_32(r, v)                   (*(volatile unsigned long *)(r) = (v))
#define GETR_32(r)                      (*(volatile unsigned long *)(r))

/** @brief 16bit registers
 */
#define SETR_16(r, v)                   (*(volatile unsigned short *)(r) = (v))
#define GETR_16(r)                      (*(volatile unsigned short *)(r))

/** @brief 8bit registers
 */
#define SETR_8(r, v)                    (*(volatile unsigned char *)(r) = (v))
#define GETR_8(r)                       (*(volatile unsigned char *)(r))

/** @brief CSD register Macros
 */
#define EMMC_GET_CSD(x,y)               (emmc_bit_field(mmc_drv_obj.csd_data, (x), (y)))

#define EMMC_CSD_CSD_STRUCTURE()        (EMMC_GET_CSD(127,126))
#define EMMC_CSD_SPEC_VARS()            (EMMC_GET_CSD(125,122))
#define EMMC_CSD_TAAC()                 (EMMC_GET_CSD(119,112))
#define EMMC_CSD_NSAC()                 (EMMC_GET_CSD(111,104))
#define EMMC_CSD_TRAN_SPEED()           (EMMC_GET_CSD(103,96))
#define EMMC_CSD_CCC()                  (EMMC_GET_CSD(95,84))
#define EMMC_CSD_READ_BL_LEN()          (EMMC_GET_CSD(83,80))
#define EMMC_CSD_READ_BL_PARTIAL()      (EMMC_GET_CSD(79,79))
#define EMMC_CSD_WRITE_BLK_MISALIGN()   (EMMC_GET_CSD(78,78))
#define EMMC_CSD_READ_BLK_MISALIGN()    (EMMC_GET_CSD(77,77))
#define EMMC_CSD_DSR_IMP()              (EMMC_GET_CSD(76,76))
#define EMMC_CSD_C_SIZE()               (EMMC_GET_CSD(73,62))
#define EMMC_CSD_VDD_R_CURR_MIN()       (EMMC_GET_CSD(61,59))
#define EMMC_CSD_VDD_R_CURR_MAX()       (EMMC_GET_CSD(58,56))
#define EMMC_CSD_VDD_W_CURR_MIN()       (EMMC_GET_CSD(55,53))
#define EMMC_CSD_VDD_W_CURR_MAX()       (EMMC_GET_CSD(52,50))
#define EMMC_CSD_C_SIZE_MULT()          (EMMC_GET_CSD(49,47))
#define EMMC_CSD_ERASE_GRP_SIZE()       (EMMC_GET_CSD(46,42))
#define EMMC_CSD_ERASE_GRP_MULT()       (EMMC_GET_CSD(41,37))
#define EMMC_CSD_WP_GRP_SIZE()          (EMMC_GET_CSD(36,32))
#define EMMC_CSD_WP_GRP_ENABLE()        (EMMC_GET_CSD(31,31))
#define EMMC_CSD_DEFALT_ECC()           (EMMC_GET_CSD(30,29))
#define EMMC_CSD_R2W_FACTOR()           (EMMC_GET_CSD(28,26))
#define EMMC_CSD_WRITE_BL_LEN()         (EMMC_GET_CSD(25,22))
#define EMMC_CSD_WRITE_BL_PARTIAL()     (EMMC_GET_CSD(21,21))
#define EMMC_CSD_CONTENT_PROT_APP()     (EMMC_GET_CSD(16,16))
#define EMMC_CSD_FILE_FORMAT_GRP()      (EMMC_GET_CSD(15,15))
#define EMMC_CSD_COPY()                 (EMMC_GET_CSD(14,14))
#define EMMC_CSD_PERM_WRITE_PROTECT()   (EMMC_GET_CSD(13,13))
#define EMMC_CSD_TMP_WRITE_PROTECT()    (EMMC_GET_CSD(12,12))
#define EMMC_CSD_FILE_FORMAT()          (EMMC_GET_CSD(11,10))
#define EMMC_CSD_ECC()                  (EMMC_GET_CSD(9,8))
#define EMMC_CSD_CRC()                  (EMMC_GET_CSD(7,1))

/** @brief for sector access 
 */
#define EMMC_4B_BOUNDARY_CHECK_MASK         0x00000003  /* 4Bytes boundary check mask */
#define EMMC_SECTOR_SIZE_SHIFT              9           /* 512 = 2^9 */
#define EMMC_SECTOR_SIZE                    512
#define EMMC_BLOCK_LENGTH                   512
#define EMMC_BLOCK_LENGTH_DW                128

/** @brief eMMC specification clock
 */
#define EMMC_CLOCK_SPEC_400K                400000UL    /**< initialize clock 400KHz */
#define EMMC_CLOCK_SPEC_20M                 20000000UL  /**< normal speed 20MHz */
#define EMMC_CLOCK_SPEC_26M                 26000000UL  /**< high speed 26MHz */
#define EMMC_CLOCK_SPEC_52M                 52000000UL  /**< high speed 52MHz */

/** @brief EMMC driver error code. (extended HAL_MEMCARD_RETURN)
 */
typedef enum
{
    EMMC_ERR                    = 0,            /**< unknown error */
    EMMC_SUCCESS                = 1,            /**< OK */
    EMMC_ERR_FROM_DMAC          = 2,            /**< DMAC allocation error */
    EMMC_ERR_FROM_DMAC_TRANSFER = 3,            /**< DMAC transfer error */
    EMMC_ERR_CARD_STATUS_BIT    = 4,            /**< card status error. Non-masked error bit was set in the card status */
    EMMC_ERR_CMD_TIMEOUT        = 5,            /**< command timeout error */
    EMMC_ERR_DATA_TIMEOUT       = 6,            /**< data timeout error */
    EMMC_ERR_CMD_CRC            = 7,            /**< command CRC error */
    EMMC_ERR_DATA_CRC           = 8,            /**< data CRC error */
    EMMC_ERR_PARAM              = 9,            /**< parameter error */
    EMMC_ERR_RESPONSE           = 10,           /**< response error */
    EMMC_ERR_RESPONSE_BUSY      = 11,           /**< response busy error */
    EMMC_ERR_TRANSFER           = 12,           /**< data transfer error */
    EMMC_ERR_READ_SECTOR        = 13,           /**< read sector error */
    EMMC_ERR_WRITE_SECTOR       = 14,           /**< write sector error */
    EMMC_ERR_STATE              = 15,           /**< state error */
    EMMC_ERR_TIMEOUT            = 16,           /**< timeout error */
    EMMC_ERR_ILLEGAL_CARD       = 17,           /**< illegal card */
    EMMC_ERR_CARD_BUSY          = 18,           /**< Busy state */
    EMMC_ERR_CARD_STATE         = 19,           /**< card state error */
    EMMC_ERR_SET_TRACE          = 20,           /**< trace information error */
    EMMC_ERR_FROM_TIMER         = 21,           /**< Timer error */
    EMMC_ERR_FORCE_TERMINATE    = 22,           /**< Force terminate */
    EMMC_ERR_CARD_POWER         = 23            /**< card power fail */
} EMMC_ERROR_CODE;

/** @brief Function number */
#define EMMC_FUNCNO_NONE                        0
#define EMMC_FUNCNO_DRIVER_INIT                 1
#define EMMC_FUNCNO_CARD_POWER_ON               2
#define EMMC_FUNCNO_MOUNT                       3
#define EMMC_FUNCNO_BUS_INIT                    4
#define EMMC_FUNCNO_HIGH_SPEED                  5
#define EMMC_FUNCNO_BUS_WIDTH                   6
#define EMMC_FUNCNO_MOUNT_BOOT_PARTITION        7
#define EMMC_FUNCNO_TRACE_INFO_INIT_OK          8
#define EMMC_FUNCNO_TOC_READ                    9
#define EMMC_FUNCNO_TRACE_INFO_IMAGE_NUMBER     10
#define EMMC_FUNCNO_TRACE_INFO_IMAGE_FOUND      11
#define EMMC_FUNCNO_LOAD_ISSW                   12
#define EMMC_FUNCNO_TRACE_INFO_IMAGE_LOADED     13
#define EMMC_FUNCNO_REGIST_RPC_FUNC             14
#define EMMC_FUNCNO_ISSW_IMPORT                 15
#define EMMC_FUNCNO_EXECUTE_ISSW                16
#define EMMC_FUNCNO_LOADIMAGE_RPC               17
#define EMMC_FUNCNO_LOADIMAGE_API               18
#define EMMC_FUNCNO_READ_SECTOR                 19
#define EMMC_FUNCNO_WRITE_SECTOR                20
#define EMMC_FUNCNO_READ_RPMB                   21
#define EMMC_FUNCNO_WRITE_RPMB                  22
#define EMMC_FUNCNO_FINALIZE                    23

/** @brief Response
 */
/** R1 */
#define EMMC_R1_ERROR_MASK                      0xfd3fe080UL        /* Type 'E' bit and bit14(must be 0). ignore bit22 and bit23  */ 
#define EMMC_R1_STATE_MASK                      0x00001E00UL        /* [12:9] */
#define EMMC_R1_READY                           0x00000100UL        /* bit8 */
#define EMMC_R1_STATE_SHIFT                     9

/** R4 */
#define EMMC_R4_RCA_MASK                        0xFFFF0000UL
#define EMMC_R4_STATUS                          0x00008000UL

/** CSD */
#define EMMC_TRANSPEED_FREQ_UNIT_MASK           0x07                /* bit[2:0] */
#define EMMC_TRANSPEED_FREQ_UNIT_SHIFT          0
#define EMMC_TRANSPEED_MULT_MASK                0x78                /* bit[6:3] */
#define EMMC_TRANSPEED_MULT_SHIFT               3

/** OCR */
#define EMMC_HOST_OCR_VALUE                     0xC0000080L
#define EMMC_OCR_STATUS_BIT                     0x80000000L         /* Card power up status bit */
#define EMMC_OCR_ACCESS_MODE_MASK               0x60000000L
#define EMMC_OCR_ACCESS_MODE_SECT               0x40000000L
#define EMMC_OCR_ACCESS_MODE_BYTE               0x00000000L

/** EXT_CSD */
#define EMMC_EXT_CSD_S_CMD_SET                      504
#define EMMC_EXT_CSD_INI_TIMEOUT_AP                 241
#define EMMC_EXT_CSD_PWR_CL_DDR_52_360              239
#define EMMC_EXT_CSD_PWR_CL_DDR_52_195              238
#define EMMC_EXT_CSD_MIN_PERF_DDR_W_8_52            235
#define EMMC_EXT_CSD_MIN_PERF_DDR_R_8_52            234
#define EMMC_EXT_CSD_TRIM_MULT                      232
#define EMMC_EXT_CSD_SEC_FEATURE_SUPPORT            231
#define EMMC_EXT_CSD_SEC_ERASE_MULT                 229
#define EMMC_EXT_CSD_BOOT_INFO                      228
#define EMMC_EXT_CSD_BOOT_SIZE_MULTI                226
#define EMMC_EXT_CSD_ACC_SIZE                       225
#define EMMC_EXT_CSD_HC_ERASE_GRP_SIZE              224
#define EMMC_EXT_CSD_ERASE_TIMEOUT_MULT             223
#define EMMC_EXT_CSD_PEL_WR_SEC_C                   222
#define EMMC_EXT_CSD_HC_WP_GRP_SIZE                 221
#define EMMC_EXT_CSD_S_C_VCC                        220
#define EMMC_EXT_CSD_S_C_VCCQ                       219
#define EMMC_EXT_CSD_S_A_TIMEOUT                    217
#define EMMC_EXT_CSD_SEC_COUNT                      215
#define EMMC_EXT_CSD_MIN_PERF_W_8_52                210
#define EMMC_EXT_CSD_MIN_PERF_R_8_52                209
#define EMMC_EXT_CSD_MIN_PERF_W_8_26_4_52           208
#define EMMC_EXT_CSD_MIN_PERF_R_8_26_4_52           207
#define EMMC_EXT_CSD_MIN_PERF_W_4_26                206
#define EMMC_EXT_CSD_MIN_PERF_R_4_26                205
#define EMMC_EXT_CSD_PWR_CL_26_360                  203
#define EMMC_EXT_CSD_PWR_CL_52_360                  202
#define EMMC_EXT_CSD_PWR_CL_26_195                  201
#define EMMC_EXT_CSD_PWR_CL_52_195                  200
#define EMMC_EXT_CSD_CARD_TYPE                      196
#define EMMC_EXT_CSD_CSD_STRUCTURE                  194
#define EMMC_EXT_CSD_EXT_CSD_REV                    192
#define EMMC_EXT_CSD_CMD_SET                        191
#define EMMC_EXT_CSD_CMD_SET_REV                    189
#define EMMC_EXT_CSD_POWER_CLASS                    187
#define EMMC_EXT_CSD_HS_TIMING                      185
#define EMMC_EXT_CSD_BUS_WIDTH                      183
#define EMMC_EXT_CSD_ERASED_MEM_CONT                181
#define EMMC_EXT_CSD_PARTITION_CONFIG               179
#define EMMC_EXT_CSD_BOOT_CONFIG_PROT               178
#define EMMC_EXT_CSD_BOOT_BUS_WIDTH                 177
#define EMMC_EXT_CSD_ERASE_GROUP_DEF                175
#define EMMC_EXT_CSD_BOOT_WP                        173
#define EMMC_EXT_CSD_USER_WP                        171
#define EMMC_EXT_CSD_FW_CONFIG                      169
#define EMMC_EXT_CSD_RPMB_SIZE_MULT                 168
#define EMMC_EXT_CSD_RST_n_FUNCTION                 162
#define EMMC_EXT_CSD_PARTITIONING_SUPPORT           160
#define EMMC_EXT_CSD_MAX_ENH_SIZE_MULT              159
#define EMMC_EXT_CSD_PARTITIONS_ATTRIBUTE           156
#define EMMC_EXT_CSD_PARTITION_SETTING_COMPLETED    155
#define EMMC_EXT_CSD_GP_SIZE_MULT                   154
#define EMMC_EXT_CSD_ENH_SIZE_MULT                  142
#define EMMC_EXT_CSD_ENH_START_ADDR                 139
#define EMMC_EXT_CSD_SEC_BAD_BLK_MGMNT              134

#define EMMC_EXT_CSD_CARD_TYPE_26MHZ                0x01
#define EMMC_EXT_CSD_CARD_TYPE_52MHZ                0x02
#define EMMC_EXT_CSD_CARD_TYPE_DDR_52MHZ_12V        0x04
#define EMMC_EXT_CSD_CARD_TYPE_DDR_52MHZ_18V        0x08
#define EMMC_EXT_CSD_CARD_TYPE_52MHZ_MASK           0x0e

/** SWITCH (CMD6) argument */
#define EMMC_SWITCH_HS_TIMING           0x03b90100UL    /**< HS_TIMING = 0x01 */
#define EMMC_SWITCH_BUS_WIDTH_8         0x03B70200UL    /**< BUS_WIDTH = 0x02 */
#define EMMC_SWITCH_BUS_WIDTH_4         0x03B70100UL    /**< BUS_WIDTH = 0x01 */
#define EMMC_SWITCH_BUS_WIDTH_1         0x03B70000UL    /**< BUS_WIDTH = 0x00 */
#define EMMC_SWITCH_PARTITION_CONFIG    0x03B30000UL    /**< Partition config = 0x00 */

/** Bus width */
#define EMMC_BUSWIDTH_1BIT              CE_CMD_SET_DATW_1BIT
#define EMMC_BUSWIDTH_4BIT              CE_CMD_SET_DATW_4BIT
#define EMMC_BUSWIDTH_8BIT              CE_CMD_SET_DATW_8BIT


/** for st_mmc_base */
#define EMMC_MAX_RESPONSE_LENGTH        17
#define EMMC_MAX_CID_LENGTH             16
#define EMMC_MAX_CSD_LENGTH             16
#define EMMC_MAX_EXT_CSD_LENGTH         512

/** @brief for TAAC mask
 */
#define TAAC_TIME_UNIT_MASK         (0x07)
#define TAAC_MULTIPLIER_FACTOR_MASK (0x0F)


/* ********************** STRUCTURES, TYPE DEFINITIONS ********************* */

/** @brief Partition id
 */
typedef enum
{
    PARTITION_ID_USER   = 0x0,      /**< User Area */
    PARTITION_ID_BOOT_1 = 0x1,      /**< boot partition 1 */
    PARTITION_ID_BOOT_2 = 0x2,      /**< boot partition 2 */
    PARTITION_ID_RPMB   = 0x3,      /**< Replay Protected Memory Block */
    PARTITION_ID_GP_1   = 0x4,      /**< General Purpose partition 1 */
    PARTITION_ID_GP_2   = 0x5,      /**< General Purpose partition 2 */
    PARTITION_ID_GP_3   = 0x6,      /**< General Purpose partition 3 */
    PARTITION_ID_GP_4   = 0x7,      /**< General Purpose partition 4 */
    PARTITION_ID_MASK   = 0x7       /**< [2:0] */
} EMMC_PARTITION_ID;

/** @brief card state in R1 response [12:9]
 */
typedef enum
{
    EMMC_R1_STATE_IDLE  = 0,
    EMMC_R1_STATE_READY = 1,
    EMMC_R1_STATE_IDENT = 2,
    EMMC_R1_STATE_STBY  = 3,
    EMMC_R1_STATE_TRAN  = 4,
    EMMC_R1_STATE_DATA  = 5,
    EMMC_R1_STATE_RCV   = 6,
    EMMC_R1_STATE_PRG   = 7,
    EMMC_R1_STATE_DIS   = 8,
    EMMC_R1_STATE_BTST  = 9
} EMMC_R1_STATE;

/** @brief eMMC boot driver error information
 */
typedef struct {
    uint16  num;                /**< error no */
    uint16  code;               /**< error code */
    volatile uint32 int_flag;   /**< CE_INT register value. (hardware dependence) */
    volatile uint32 status1;    /**< CE_HOST_STS1 register value. (hardware dependence) */
    volatile uint32 status2;    /**< CE_HOST_STS2 register value. (hardware dependence) */
} st_error_info;


/** @brief Command information
 */
typedef struct {
    HAL_MEMCARD_COMMAND         cmd;    /**< Command information */
    uint32                      arg;    /**< argument */
    HAL_MEMCARD_OPERATION       dir;    /**< direction */
    uint32                      hw;     /**< H/W dependence. CE_CMD_SET register value. */
} st_command_info;


/** @brief MMC driver base
 */
typedef struct {
    st_error_info   error_info;         /**< error information */
    st_command_info cmd_info;           /**< command information */

    /* for data transfer */
    uint8   *buff_address_virtual;      /**< Dest or Src buff */
    uint8   *buff_address_physical;     /**< Dest or Src buff */
    HAL_MEMCARD_DATA_WIDTH  bus_width;  /**< bus width */
    uint32  trans_size;                 /**< transfer size for this command */
    uint32  remain_size;                /**< remain size for this command */
    uint32  response_length;            /**< response length for this command */
    uint32  image_size;                 /**< image_size (sector) */

    /* clock */
    uint32  base_clock;                 /**< MMC host controller clock */
    uint32  max_freq;                   /**< Max frequency (Card Spec)[Hz]. It changes dynamically by CSD and EXT_CSD. */
    uint32  request_freq;               /**< request freq [Hz] (400K, 26MHz, 52MHz, etc) */
    uint32  current_freq;               /**< current MMC clock[Hz] (the closest frequency supported by HW) */

    /* state flag */
    HAL_MEMCARD_PRESENCE_STATUS card_present;   /**< presence status of the memory card */
    uint32  card_power_enable;                  /**< True : Power ON */
    uint32  clock_enable;                       /**< True : Clock ON */
    uint32  initialize;                         /**< True : initialize complete. */
    uint32  access_mode;                        /**< True : sector access, FALSE : byte access */
    uint32  mount;                              /**< True : mount complete. */
    uint32  selected;                           /**< True : selected card. */
    HAL_MEMCARD_DATA_TRANSFER_MODE transfer_mode;   /**< 0: DMA, 1:PIO */
    uint32  image_num;                          /**< loaded ISSW image No. ISSW have copy image. */
    EMMC_R1_STATE current_state;                /**< card state */
    volatile uint32 during_cmd_processing;      /**< True : during command processing */
    volatile uint32 during_transfer;            /**< True : during transfer */
    volatile uint32 during_dma_transfer;        /**< True : during transfer (DMA)*/
    volatile uint32 dma_error_flag;             /**< True : occurred DMAC error */
    volatile uint32 force_terminate;            /**< force terminate flag */
    volatile uint32 state_machine_blocking;     /**< state machine blocking flag : True or False */

    /* timeout */
    uint32  data_timeout;                       /**< read and write data timeout.[ms] */

    /* retry */
    uint32  retries_after_fail;     /**< how many times to try after fail, for instance sending command */

    /* interrupt */
    volatile uint32 int_event;      /**< interrupt Event */ 

    /* response */
    uint32  *response;              /**< pointer to buffer for executing command. */
    uint32  r1_card_status;         /**< R1 response data */
    uint32  r3_ocr;                 /**< R3 response data */
    uint32  r4_resp;                /**< R4 response data */
    uint32  r5_resp;                /**< R5 response data */

    uint32  low_clock_mode_enable;  /**< True : clock mode is low. (MMC clock = Max26MHz) */
    uint32  reserved2;
    uint32  reserved3;
    uint32  reserved4;

    /* Card registers (4byte align) */
    uint8   csd_data[EMMC_MAX_CSD_LENGTH];              /**< CSD */
    uint8   cid_data[EMMC_MAX_CID_LENGTH];              /**< CID */
    uint8   ext_csd_data[EMMC_MAX_EXT_CSD_LENGTH];      /**< EXT_CSD */
    uint8   response_data[EMMC_MAX_RESPONSE_LENGTH];    /**< other response */
} st_mmc_base;

typedef int (*func)(void);

/* ********************** DECLARATION OF EXTERNAL DATA ********************* */

/* ************************** FUNCTION PROTOTYPES ************************** */
uint32 emmc_get_csd_time(void);

/* ********************************* CODE ********************************** */

/* ******************************** END ************************************ */
#endif  /* __EMMC_STD_H__ */
