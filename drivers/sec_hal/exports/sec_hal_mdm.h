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
#ifndef SEC_HAL_MDM_H
#define SEC_HAL_MDM_H


/* ****************************************************************************
** return values given by following functions.
** ***************************************************************************/
#define SEC_HAL_MDM_RES_OK           0x00000000UL
#define SEC_HAL_MDM_RES_FAIL         0x00000010UL
#define SEC_HAL_MDM_RES_PARAM_ERROR  0x00000020UL


/* ****************************************************************************
** Function name      : sec_hal_mdm_memcpy
** Description        : copy data to a protected memory.
** Parameters         : IN/--- void* phys_dst, physical address where to copy.
**                      IN/--- void* phys_src, physical address of to-be-copied data.
**                                             continuous mem block expected.
**                      IN/--- uint32_t size, size of the to-be-copied data.
** Return value       : uint32
**                      == SEC_HAL_MDM_RES_OK if operation successful
**                      == SEC_HAL_MDM_RES_FAIL general failure indication.
**                      == SEC_HAL_MDM_RES_PARAM_ERROR given inputs were faulty.
** ***************************************************************************/
uint32_t sec_hal_mdm_memcpy(void* phys_dst, void* phys_src, uint32_t size);

/* ****************************************************************************
** Function name      : sec_hal_mdm_authenticate
** Description        : authenticate memory content with given SW CERT.
** Parameters         : IN/--- void* phys_addr, physical address of SW CERT.
**                      IN/--- uint32_t size, size of the SW CERT.
** Return value       : uint32
**                      == SEC_HAL_MDM_RES_OK if operation successful
**                      == SEC_HAL_MDM_RES_FAIL general failure indication.
**                      == SEC_HAL_MDM_RES_PARAM_ERROR given inputs were faulty.
** ***************************************************************************/
uint32_t sec_hal_mdm_authenticate(void* phys_addr, uint32_t size);


#endif /* SEC_HAL_MDM_H */
