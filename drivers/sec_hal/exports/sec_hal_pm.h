/* ********************************************************************* **
**                               Renesas                                 **
** ********************************************************************* */

/* *************************** COPYRIGHT INFORMATION ******************* **
** This program contains proprietary information that is a trade secret of **
** Renesas and also is protected as an unpublished work under              **
** applicable Copyright laws. Recipient is to retain this program in       **
** confidence and is not permitted to use or make copies thereof other than**
** as permitted in a written agreement with Renesas.                       **
**                                                                         **
** Copyright (C) 2012 Renesas Electronics Corp.                            **
** All rights reserved.                                                    **
** *********************************************************************** */
#ifndef SEC_HAL_PM_H
#define SEC_HAL_PM_H


/* **************************************************************************
 * return values given by following functions.
 * *************************************************************************/
#define SEC_HAL_PM_RES_OK           0x00000000UL
#define SEC_HAL_PM_RES_FAIL         0x00000010UL
#define SEC_HAL_PM_RES_PARAM_ERROR  0x00000020UL

/* PM start */
uint32_t
sec_hal_pm_coma_entry_init(void);
/* PM end */

/* **************************************************************************
 * sec_hal_coma_entry : sent indication of 'coma' to secure side.
 * *************************************************************************/
uint32_t
sec_hal_pm_coma_entry(uint32_t mode,
                      uint32_t wakeup_address,
                      uint32_t pll0,
                      uint32_t zclk);

/* **************************************************************************
 * sec_hal_pm_power_off : sent indication of 'power off' to secure side.
 * *************************************************************************/
uint32_t
sec_hal_pm_power_off(void);

/* **************************************************************************
 * sec_hal_pm_coma_cpu_off : sent indication of 'coma cpu off' to secure side.
 * *************************************************************************/
uint32_t
sec_hal_pm_coma_cpu_off(void);

/* **************************************************************************
 * sec_hal_pm_power_off : sent indication of 'power off' to secure side.
 * *************************************************************************/
uint32_t
sec_hal_pm_power_off(void);

/* **************************************************************************
 * sec_hal_pm_public_cc42_key_init : sent indication of 'cc42 key init'
 * to secure side.
 * *************************************************************************/
uint32_t
sec_hal_pm_public_cc42_key_init(void);

/* **************************************************************************
 * sec_hal_pm_a3sp_state_request : sent indication of 'a3sp' state
 * to secure side.
 * *************************************************************************/
uint32_t
sec_hal_pm_a3sp_state_request(uint32_t state);

#endif /* SEC_HAL_PM_H */

