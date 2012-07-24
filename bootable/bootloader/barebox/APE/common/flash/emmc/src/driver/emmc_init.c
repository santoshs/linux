/* emmc_init.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "string.h"
#include "emmc.h"
#include "emmc_private.h"

extern EMMC_DRV_INFO gDrv_Info;
extern EMMC_CMD_INFO gCmd_Info;


/**
 * Emmc_Init - eMMC init
 * @return EMMC_SUCCESS : Successful
 */
RC Emmc_Init(void)
{
	ulong temp;
	/* clear driver data */
	memset(&gDrv_Info, 0, sizeof(EMMC_DRV_INFO));
	memset(&gCmd_Info, 0, sizeof(EMMC_CMD_INFO));
	/* GPIO setting */
	SET_REG8(GPIO_PORT309CR, PULL_OFF_REG_11);
	SET_REG8(GPIO_PORT300CR, PULL_UP_REG_E1);
	SET_REG8(GPIO_PORT301CR, PULL_UP_REG_E1);
	SET_REG8(GPIO_PORT302CR, PULL_UP_REG_E1);
	SET_REG8(GPIO_PORT303CR, PULL_UP_REG_E1);
	SET_REG8(GPIO_PORT304CR, PULL_UP_REG_E1);
	SET_REG8(GPIO_PORT305CR, PULL_UP_REG_E1);
	SET_REG8(GPIO_PORT306CR, PULL_UP_REG_E1);
	SET_REG8(GPIO_PORT307CR, PULL_UP_REG_E1);
	SET_REG8(GPIO_PORT308CR, PULL_UP_REG_E1);
	SET_REG8(GPIO_PORT310CR, PULL_UP_REG_10);
	/* set Module stop control register */
	temp = (GET_REG32(SMSTPCR3) & SMSTPCR3_MMC_ENABLE);
	SET_REG32(SMSTPCR3, temp);

	/* set Software Reset Register */
	temp = (GET_REG32(SRCR3) & SRCR3_MMC_ENABLE);
	SET_REG32(SRCR3, temp);
	
	/* MMCIF initialize */
	SET_REG32(CE_VERSION, CE_VERSION_SWRST);		/* Soft reset */
	SET_REG32(CE_VERSION, 0x00000000);
	SET_REG32(CE_INT_MASK, 0x00000000);				/* all interrupt disable */
	SET_REG32(CE_INT, 0x00000000);					/* all interrupt clear */
	SET_REG32(CE_CLK_CTRL, 0x00000000);				/* MMC clock stop */
	SET_REG32(CE_BUF_ACC, CE_BUF_ACC_SWAP_ENABLE);	/* little endian */
	
	SET_REG32(GPIO_PORTR319_288DCR, GPIO_MMC_RST_EXTERNAL);	/* PORT310 = RST_n = Lo */
	/* wait 35ms (In the case of CPU1014MHz) */
	emmc_Wait(1);
	/* MMCIF RST_n trigger */
	SET_REG32(GPIO_PORTR319_288DSR, GPIO_MMC_RST_EXTERNAL);	/* PORT310 = RST_n = High */
	emmc_Wait(3);

	return EMMC_SUCCESS;
}


