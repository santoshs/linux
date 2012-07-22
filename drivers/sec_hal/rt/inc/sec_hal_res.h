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
** Copyright (C) 2010-2012 Renesas Electronics Corp.                         **
** All rights reserved.                                                      **
** ************************************************************************* */
#ifndef SEC_HAL_RES_H
#define SEC_HAL_RES_H

/* Operation completed successfully */
#define SEC_HAL_RES_OK                        0x00000000

/* Operation failed */
#define SEC_HAL_RES_FAIL                      0x00000010

/* Incorrect parameters given to the function */
#define SEC_HAL_RES_PARAM_ERROR               0x00000020

/* Requested functionality is not, yet, implemented. */
#define SEC_HAL_RES_NOT_IMPL                  0x00000040

/* Requested service needs more data to complete the request */
#define SEC_HAL_RES_NEED_DATA                 0x00000080

/* ******************************** END ************************************ */

#endif /* SEC_HAL_RES_H */

