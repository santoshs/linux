/* arch/arm/mach-shmobile/pm_secure_support/sec_hal.c
 *
 * Copyright (C) 2012 Renesas Mobile Corporation
 * 
 */


#include <linux/module.h> 
#include <linux/types.h>

#include <asm/system.h>
#include <asm/io.h>
#include <linux/err.h>
#include "pm_ram0.h"

#define SEC_HAL_RES_OK						0x00000000
/* #define pm_writel(v, a)			__raw_writel(v, (void *__iomem)a)*/

uint32_t sec_hal_coma_entry(uint32_t mode, uint32_t freq, uint32_t wakeup_address, uint32_t context_save_address)
{
	/* pm_writel(0, ram0SecHalvalue);*/
	return SEC_HAL_RES_OK;

}
EXPORT_SYMBOL(sec_hal_coma_entry);

uint32_t  sec_hal_power_off()
{
	return SEC_HAL_RES_OK;
}
EXPORT_SYMBOL(sec_hal_power_off);

