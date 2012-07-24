/*************************************************************************
 **     Copyright (C) 2010 Nokia Corporation. All rights reserved.      **
 **                                                                     **
 ** Permission is hereby granted, free of charge, to any person         **
 ** obtaining a copy of this software and associated documentation      **
 ** files (the "Software"), to deal in the Software without             **
 ** restriction, including without limitation the rights to use, copy,  **
 ** modify, merge, publish, distribute, sublicense, and/or sell copies  **
 ** of the Software, and to permit persons to whom the Software is      **
 ** furnished to do so, subject to the following conditions:            **
 **                                                                     **
 ** The above copyright notice and this permission notice shall be      **
 ** included in all copies or substantial portions of the Software.     **
 ** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,     **
 ** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF  **
 ** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND               **
 ** NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS **
 ** BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN  **
 ** ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN   **
 ** CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE    **
 ** SOFTWARE.                                                           **
 **                                                                     **
 *************************************************************************
 **                                                                     **
 ** File:  ssa.h                                                        **
 **                                                                     **
 ** Desc:  Secure ROM internal Interface for Secure Service API         **
 **                                                                     **
 *************************************************************************/

#ifndef SSA_H
#define SSA_H

/*!
 * Secure Mode FLAGs
 */

/* When When set, enables ICache within the secure mode */
#define SEC_ROM_ICACHE_ENABLE_MASK                        0x00000001

/* When When set, enables DCache within the secure mode */
#define SEC_ROM_DCACHE_ENABLE_MASK                        0x00000002

/* When When set, enables IRQ within the secure mode */
#define SEC_ROM_IRQ_ENABLE_MASK                           0x00000004

/* When set, enables FIQ within the secure mode */
#define SEC_ROM_FIQ_ENABLE_MASK                           0x00000008

/* When When set, enables Unified L2 cache within the secure mode */
#define SEC_ROM_UL2_CACHE_ENABLE_MASK                     0x00000010

/*!
 * Secure mode exit reasons
 */
#define SEC_EXIT_NORMAL                      1 /* Used in ASM, do not change */
#define SEC_EXIT_START_EXT_CODE              2 /* Used in ASM, do not change */
#define SEC_EXIT_INT                         3 /* Used in ASM, do not change */
#define SEC_EXIT_RPC_CALL                    4 /* Used in ASM, do not change */

/*!
 * Secure service IDs
 */

/* This service is meant for Secure mode initialization. */
#define SSAPI_PRE_INIT_SERV                               0x00000001

/* This service is meant for initialization after MCU clock is set to higher frequency. */
#define SSAPI_POST_SPEEDUP_INIT_SERV                      0x00000002

/* This service is meant for ISSW importing. */
#define SSAPI_ISSW_IMPORT_SERV                            0x00000003

/* This service ID is meant when returning from handling interrupt
   back to secure mode. */
#define SSAPI_RET_FROM_INT_SERV                           0x00000004

/* This service ID is meant when returning from RPC call. */
#define SSAPI_RET_FROM_RPC_SERV                           0x00000005

/* This service is used to run the ISSW after it has been loaded */
#define SSAPI_ISSW_EXECUTE_SERV                           0x00000006

/*!
 * Secure ROM return codes to public side
 */

/* Application called successfully */
#define SEC_ROM_RET_OK                                    0x0001

/* Secure ROM code or ISSW does not support called application. */
#define SEC_ROM_RET_NON_SUPPORTED_SERV                    0x0002

/* Secure mode entry disabled due to non-initialized secure mode. */
#define SEC_ROM_RET_ENTRY_DISABLED                        0x0003

/* Secure mode is already in use and requested application call is not allowed. */
#define SEC_ROM_RET_RESOURCE_RESERVED                     0x0004

/* RNG block is busy (probably doing initial entropy collection) and not able to generate proper random number. */
#define SEC_ROM_RET_RNG_RESOURCE_BUSY                     0x0005

/* Non-valid address has been passed to secure mode application. Valid addresses are those that have entry in secure mode MMU table and that do not point to secure mode memories or to secure mode I/O ports. */
#define SEC_ROM_RET_NON_VALID_ADDRESS                     0x0006

/* Failure noticed while doing hash checking. */
#define SEC_ROM_RET_HASH_FAILURE                          0x0007

/* Secure mode init called although init has been called already earlier. No actions done in secure mode. */
#define SEC_ROM_RET_INIT_ALREADY_DONE                     0x0008

/* General failure. */
#define SEC_ROM_RET_FAIL                                  0x0009

/* Certificate failure. */
#define SEC_ROM_RET_CERTIFICATE_FAILURE                   0x000A

/* Extended hash check area defined in certificate although not supported. */
#define SEC_ROM_RET_DIVIDED_HASH_AREA_NOT_ALLOWED         0x000B

/* ISSW is not meant for this chip. This checking is done in ISSW init and usually it checks that secure ROM version matches with the value defined in ISSW. */
#define SEC_ROM_RET_ISSW_VERSION_WRONG                    0x000D

/* Requested application call is not allowed due to secure mode being already active (running another application). */
#define SEC_ROM_RET_ENTRY_FAILURE                         0x000E

/* Importing ISSW to secure RAM failed. This may be due to following reasons 
. ISSW is already loaded in secure mode. */
#define SEC_ROM_RET_ISSW_LOAD_FAILURE                     0x000F

/* Hash calculated over ISSW code and RW data does not match with the hash in ISSW certificate. */
#define SEC_ROM_RET_ISSW_HASH_FAILURE                     0x0010

/* ISSW certificate checking failed. This may be due to following reasons 
. Magic in ISSW certificate is wrong
. ISSW certificate signature is wrong
. Signing key type or version information in ISSW certificate is wrong */
#define SEC_ROM_RET_ISSW_CERTIFICATE_FAILURE              0x0011

/* There is not enough memory for loading ISSW into secure RAM. */
#define SEC_ROM_RET_ISSW_NOT_ENOUGH_MEMORY                0x0012

/* ISSW hash check does not support requested extended hash area checking. */
#define SEC_ROM_RET_ISSW_DIVIDED_HASH_AREA_NOT_ALLOWED    0x0013

/* COMA SERVICES ARE TBD */
/* Importing COMA to secure RAM failed. This may be due to following reasons 
. COMA is already loaded in secure mode. XTI trace: .COMA image already loaded, new loading attempt discarded..
. COMA loading is attempted in wrong phase. XTI trace: .COMA loading attempted during a wrong phase.. */
#define SEC_ROM_RET_COMA_LOAD_FAILURE                     0x001E

/* Hash calculated over COMA code and RW data does not match with the hash in COMA certificate. XTI trace: .COMA appl hash failure.. */
#define SEC_ROM_RET_COMA_HASH_FAILURE                     0x001F

/* COMA certificate checking failed. This may be due to following reasons:
. Magic in COMA certificate is wrong
. Key index in COMA certificate is wrong
. COMA certificate signature is wrong
. Signing key type or version information in COMA certificate is wrong
XTI trace: .COMA certificate failure.. */
#define SEC_ROM_RET_COMA_CERTIFICATE_FAILURE              0x0021

/* There is not enough memory for loading COMA into secure RAM. XTI trace: .Not enough memory for loading COMA appl.. */
#define SEC_ROM_RET_COMA_NOT_ENOUGH_MEMORY                0x0022

/* COMA hash check does not support requested extended hash area checking. XTI trace: .Divided hash area not allowed with COMA appl.. */
#define SEC_ROM_RET_COMA_DIVIDED_HASH_AREA_NOT_ALLOWED    0x0023

/* COMA image is not meant for this chip. This checking is done in COMA init and usually it checks that secure ROM version matches with the value defined in COMA initialization code. */
#define SEC_ROM_RET_COMA_VERSION_FAILURE                  0x0024

/*!
 * Public rom rpc handler indexes
 */
#define RPC_LOAD_IMAGE      1
/** RPC_CALL parameter RAMSet */
#define RPC_RAM_SET                                             0x00000002
/** RPC_CALL parameter SET_RAM_CODE_ENTRY_POINT */
#define RPC_RAM_CODE_ENTRY_POINT                                0x00000003


#endif /* SSA_H */

/* vim: set autoindent shiftwidth=4 smarttab expandtab : */
/* -*- mode: C; c-basic-indent: 4; indent-tabs-mode: nil -*- */
