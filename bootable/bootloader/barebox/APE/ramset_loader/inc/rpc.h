#ifndef __RPC_H__
#define __RPC_H__
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
 * @file rpc.h
 * @brief header file for rpc API.
 * @author Renesas
 *
 */

#include "types.h"
// #include "CPU_Register.h"
#include "common.h"
#include "sysc.h"
#include "cpg.h"
#include "gpio.h"


/* ***************** MACROS, CONSTANTS, COMPILATION FLAGS ****************** */
#define SHA1_HASH_SIZE      20
#define MAX_HASH_SIZE       64

/* Non-Secure -> Secure request parameter area */
#define PARAM_NSSREQ_START              ((INTER_CONNECT_RAM0_BASE)+0x0000)
/* SEC2PUB  */
#define SEC2PUB_START                   ((INTER_CONNECT_RAM0_BASE)+0x0000DC00)
/* Image copy area (MERAM) */
#define IMAGE_TOC_START                 MERAM_START_ADDRESS

/*!
 *  RPC_CALL public return code
 */
#define PUB_ROM_RET_OK                                      0x00000001
#define PUB_ROM_RET_FAIL                                    0x80000000
#define PUB_ROM_RET_LOAD_SKIP                               0x80000001

/*!
 * Public rom rpc handler indexes
 */
/** RPC_CALL parameter LOAD_IMAGE */
#define LOAD_IMAGE                                          0x00000001
/** RPC_CALL parameter RAMSet */
#define RAM_SET                                             0x00000002
/** RPC_CALL parameter SET_RAM_CODE_ENTRY_POINT */
#define RAM_CODE_ENTRY_POINT                                0x00000003

/* This service is meant for Secure mode initialization. */
#define SSAPI_PRE_INIT_SERV                                 0x00000001
/* This service is meant for initialization after MCU clock is set to higher frequency. */
#define SSAPI_POST_SPEEDUP_INIT_SERV                        0x00000002
/* This service is meant for ISSW importing. */
#define SSAPI_ISSW_IMPORT_SERV                              0x00000003
/* This service ID is meant when returning from handling interrupt
   back to secure mode. */
#define SSAPI_RET_FROM_INT_SERV                             0x00000004
/* This service ID is meant when returning from RPC call. */
#define SSAPI_RET_FROM_RPC_SERV                             0x00000005
/* This service is used to run the ISSW after it has been loaded */
#define SSAPI_ISSW_EXECUTE_SERV                             0x00000006

/*!
 * Public rom rpc handler types
 */
/** RPC_CALL parameter RAMSet */
#define TYPE_RAM_SET                                        0x00000001
/** RPC_CALL parameter 2nd Loader */
#define TYPE_2ND_LOADER                                     0x00000002

/** address for sec2pub */
#define SEC_2_PUB                  SEC2PUB_START
/** address for pub rpm issw info */
#define PUB_ROM_ISSW               (IMAGE_TOC_START+0x00000200)
/** address for pub rpm issw info */
#define PUB_ROM_ISSW_INFO          (PARAM_NSSREQ_START+0x0000D800)
/** address for rpc params */
#define PUB_ROM_RPC_PARAMS         (PUB_ROM_ISSW_INFO+0x00000020)

/* When When set, enables ICache within the secure mode */
#define SEC_ROM_ICACHE_ENABLE_MASK                        0x00000001

/* When When set, enables DCache within the secure mode */
#define SEC_ROM_DCACHE_ENABLE_MASK                        0x00000002

/* When When set, enables IRQ within the secure mode */
#define SEC_ROM_IRQ_ENABLE_MASK                           0x00000004

/* When set, enables FIQ within the secure mode */
#define SEC_ROM_FIQ_ENABLE_MASK                           0x00000008

#define FLAGS     (SEC_ROM_ICACHE_ENABLE_MASK | SEC_ROM_DCACHE_ENABLE_MASK | \
                   SEC_ROM_IRQ_ENABLE_MASK | SEC_ROM_FIQ_ENABLE_MASK)

#define FIQ_IRQ_MASK                            0xC0
#define FIQ_MASK                                0x40
#define IRQ_MASK                                0x80



/* ********************** STRUCTURES, TYPE DEFINITIONS ********************* */
/*
 * This structure is read-only for public ROM code but sec code writes to it
 * to pass parameters to the public side. Public code has a single statically
 * allocated instance of this structure the address of which is passed to
 * secure ROM in the call to SSA_PRE_INIT.
 */
typedef struct
{
    uint32_t checksum;
    uint8_t  chip_type;
    uint8_t  public_ID[SHA1_HASH_SIZE];
    uint32_t root_key_hash_type;
    uint8_t  root_key_hash_size;
    uint8_t  root_key_hash[MAX_HASH_SIZE];
} SEC2PUB_STR;


typedef struct
{
    void*   rpc_handler_address;
    void*   rpc_params;
    void*   TOC_address_physical;
    void*   TOC_address_virtual;
    void*   vendor_reserved;
} PUB_ROM_ISSW_INFO_STRUCT;

/** structure of load image parameter */
typedef struct
{
    /** Defines the interface that is used for loading the image */
    uint32_t interface;
    /** Size of the image to be loaded in bytes */
    uint32_t size_of_image;
    /** Physical address of the image in the mass memory */
    uint32_t address_to_load_from;
    /** Physical address where image should be loaded to */
    uint32_t address_to_load_to;
    /** Physical address to buffer that contains configuration data to be sent to host as configuration info */
    uint32_t configuration_info_p;
    /** Bits for defining additional behaviour/features */
    uint32_t feature_flags;
    /** Timeout value for the transfer in ms */
    uint32_t timeout;
} st_load_image_param_t;

/** structure of ram code entry point */
typedef struct
{
    /** Function type */
    uint32_t type;
    /** Physical address for executing specified function */
    uint32_t address_to_execute_from;
} st_ram_code_entry_point_t;

/** result of rpc api */
typedef enum
{
    /** success return code for RPC api */
    RPC_SUCCESS = 0,
    /** failure return code for RPC api */
    RPC_FAIL
} rom_rpc_result;

/** dummy definition because of security */
typedef struct
{
    uint32_t a;
} ISSW_CERTIFICATE;



/** callback function to register rpc api */
typedef uint32_t (*rom_rpc_cb_func)(uint32_t interface, uint32_t size_of_image, uint32_t address_to_load_from,
                                    uint32_t address_to_load_to, uint32_t configuration_info_p, uint32_t feature_flags, 
                                    uint32_t timeout);


/* ************************** FUNCTION PROTOTYPES ************************** */
void rom_call_ssa_pre_init(SEC2PUB_STR *p);
void rom_call_ssa_issw_import(const ISSW_CERTIFICATE* p, PUB_ROM_ISSW_INFO_STRUCT* iis);
void rom_call_issw_execute(void);
rom_rpc_result rom_set_rpc_callback_func(rom_rpc_cb_func callback_func);
void * rom_rpc_v_to_p( void * v_addr );
void _hw_sec_rom_pub_dispatcher(uint32_t appl_id, uint32_t flags, ...);

/* ******************************** END ************************************ */

#endif /* #ifndef __RPC_H__    */
