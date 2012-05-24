/*
 * Copyright (C) 2012 Renesas Mobile Corporation
 *
 * linux/drivers/mmc/host/sh-sdhi.c
 *
 * SD/MMC driver.
 *
 * Copyright (C) 2008-2009 Renesas Solutions Corp.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */

#include <config.h>
#include <common.h>
#include <init.h>
#include <driver.h>
#include <mci.h>
#include <clock.h>
#include <errno.h>
#include <asm/io.h>
#include <mach/hardware-base.h>

/* Address base */
#define SDHI_BASE SDHI0_BASE

#define SDHI_CMD			0x0000
#define SDHI_PORTSEL			0x0002
#define SDHI_ARG0			0x0004
#define SDHI_ARG1			0x0006
#define SDHI_STOP			0x0008
#define SDHI_SECCNT			0x000A
#define SDHI_RSP00			0x000C
#define SDHI_RSP01			0x000E
#define SDHI_RSP02			0x0010
#define SDHI_RSP03			0x0012
#define SDHI_RSP04			0x0014
#define SDHI_RSP05			0x0016
#define SDHI_RSP06			0x0018
#define SDHI_RSP07			0x001A
#define SDHI_INFO1			0x001C
#define SDHI_INFO2			0x001E
#define SDHI_INFO1_MASK			0x0020
#define SDHI_INFO2_MASK			0x0022
#define SDHI_CLK_CTRL			0x0024
#define SDHI_SIZE			0x0026
#define SDHI_OPTION			0x0028
#define SDHI_ERR_STS1			0x002C
#define SDHI_ERR_STS2			0x002E
#define SDHI_BUF0			0x0030
#define SDHI_SDIO_MODE			0x0034
#define SDHI_SDIO_INFO1			0x0036
#define SDHI_SDIO_INFO1_MASK		0x0038
#define SDHI_CC_EXT_MODE		0x00D8
#define SDHI_SOFT_RST			0x00E0
#define SDHI_EXT_SWAP			0x00F0
#define SDHI_SD_PWR			0x00F2
#define SDHI_EXT_SDIO			0x00F4
#define SDHI_EXT_WP			0x00F6
#define SDHI_EXT_CD			0x00F8
#define SDHI_EXT_CD_DAT3		0x00FA
#define SDHI_EXT_CD_MASK		0x00FC
#define SDHI_EXT_CD_DAT3_MASK		0x00FE

/* SDHI CMD VALUE */
#define CMD_MASK			0x0000ffff
#define SDHI_APP			0x0040
#define SDHI_SD_APP_SEND_SCR		0x0073
#define SDHI_SD_SWITCH			0x1C06

/* SDHI_PORTSEL */
#define USE_1PORT			(1 << 8)	/* 1 port */

/* SDHI_ARG */
#define ARG0_MASK			0x0000ffff
#define ARG1_MASK			0x0000ffff

/* SDHI_STOP */
#define STOP_SEC_ENABLE			(1 << 8)

/* SDHI_INFO1 */
#define INFO1_RESP_END			(1 << 0)
#define INFO1_ACCESS_END		(1 << 2)
#define INFO1_CARD_RE			(1 << 3)
#define INFO1_CARD_IN			(1 << 4)
#define INFO1_ISD0CD			(1 << 5)
#define INFO1_WRITE_PRO			(1 << 7)
#define INFO1_DATA3_CARD_RE		(1 << 8)
#define INFO1_DATA3_CARD_IN		(1 << 9)
#define INFO1_DATA3			(1 << 10)

/* SDHI_INFO2 */
#define INFO2_CMD_ERROR			(1 << 0)
#define INFO2_CRC_ERROR			(1 << 1)
#define INFO2_END_ERROR			(1 << 2)
#define INFO2_TIMEOUT			(1 << 3)
#define INFO2_BUF_ILL_WRITE		(1 << 4)
#define INFO2_BUF_ILL_READ		(1 << 5)
#define INFO2_RESP_TIMEOUT		(1 << 6)
#define INFO2_SDDAT0			(1 << 7)
#define INFO2_BRE_ENABLE		(1 << 8)
#define INFO2_BWE_ENABLE		(1 << 9)
#define INFO2_CBUSY			(1 << 14)
#define INFO2_ILA			(1 << 15)
#define INFO2_ALL_ERR			(0x807f)

/* SDHI_INFO1_MASK */
#define INFO1M_RESP_END			(1 << 0)
#define INFO1M_ACCESS_END		(1 << 2)
#define INFO1M_CARD_RE			(1 << 3)
#define INFO1M_CARD_IN			(1 << 4)
#define INFO1M_DATA3_CARD_RE		(1 << 8)
#define INFO1M_DATA3_CARD_IN		(1 << 9)
#define INFO1M_ALL			(0xffff)
#define INFO1M_SET			(INFO1M_RESP_END |	\
		INFO1M_ACCESS_END |	\
		INFO1M_DATA3_CARD_RE |	\
		INFO1M_DATA3_CARD_IN)

/* SDHI_INFO2_MASK */
#define INFO2M_CMD_ERROR		(1 << 0)
#define INFO2M_CRC_ERROR		(1 << 1)
#define INFO2M_END_ERROR		(1 << 2)
#define INFO2M_TIMEOUT			(1 << 3)
#define INFO2M_BUF_ILL_WRITE		(1 << 4)
#define INFO2M_BUF_ILL_READ		(1 << 5)
#define INFO2M_RESP_TIMEOUT		(1 << 6)
#define INFO2M_BRE_ENABLE		(1 << 8)
#define INFO2M_BWE_ENABLE		(1 << 9)
#define INFO2M_ILA			(1 << 15)
#define INFO2M_ALL			(0xffff)
#define INFO2M_ALL_ERR			(0x807f)

/* SDHI_CLK_CTRL */
#define CLK_ENABLE			(1 << 8)
#define CLK_INIT			(1 << 6)	/* clk / 256 */
#if defined(CONFIG_SH_MIGOR)	|| \
	defined(CONFIG_SH_AP325RXA) || \
defined(CONFIG_SH_ECOVEC)
#define CLK_MMC_TRANS			0x0001		/* for MMC */
#define CLK_SD_TRANS			0x0001		/* for SD */
#define CLK_HS_TRANS			0x0000
/* #define SDHI_CLK_HS_TRANS		0x00ff */
#elif defined(CONFIG_SH7757LCR)
#define CLK_MMC_TRANS			0x0001		/* for MMC */
#define CLK_SD_TRANS			0x0000		/* for SD */
#define CLK_HS_TRANS			0x00ff
#else
#define CLK_MMC_TRANS			0x0002		/* for MMC */
#define CLK_SD_TRANS			0x0002		/* for SD */
#define CLK_HS_TRANS			0x0001
#endif /* CONFIG_SH_MIGOR */

/* SDHI_OPTION */
#define OPT_BUS_WIDTH_1			(1 << 15)	/* bus width = 1 bit */

/* SDHI_ERR_STS1 */
#define ERR_STS1_CRC_ERROR		((1 << 11) | (1 << 10) | (1 << 9) | \
		(1 << 8) | (1 << 5))
#define ERR_STS1_CMD_ERROR		((1 << 4) | (1 << 3) | (1 << 2) | \
		(1 << 1) | (1 << 0))

/* SDHI_ERR_STS2 */
#define ERR_STS2_RES_TIMEOUT		(1 << 0)
#define ERR_STS2_RES_STOP_TIMEOUT	((1 << 0) | (1 << 1))
#define ERR_STS2_SYS_ERROR		((1 << 6) | (1 << 5) | (1 << 4) | \
		(1 << 3) | (1 << 2) | (1 << 1) | \
		(1 << 0))

/* SDHI_SDIO_MODE */
#define SDIO_MODE_ON			(1 << 0)
#define SDIO_MODE_OFF			(0 << 0)

/* SDHI_SDIO_INFO1 */
#define SDIO_INFO1_IOIRQ		(1 << 0)
#define SDIO_INFO1_EXPUB52		(1 << 14)
#define SDIO_INFO1_EXWT			(1 << 15)

/* SDHI_SDIO_INFO1_MASK */
#define SDIO_INFO1M_CLEAR		((1 << 1) | (1 << 2))
#define SDIO_INFO1M_ON			((1 << 15) | (1 << 14) | (1 << 2) | \
					 (1 << 1) | (1 << 0))

/* SDHI_EXT_SWAP */
#define SET_SWAP			((1 << 6) | (1 << 7))	/* SWAP */

/* SDHI_SOFT_RST */
#define SOFT_RST_ON			(0 << 0)
#define SOFT_RST_OFF			(1 << 0)

#define	CLKDEV_SD_DATA			25000000	/* 25 MHz */
#define CLKDEV_HS_DATA			50000000	/* 50 MHz */
#define CLKDEV_MMC_DATA			20000000	/* 20MHz */
#define	CLKDEV_INIT			400000		/* 100 - 400 KHz */

#define NO_CARD_ERR		-16 /* No SD/MMC card inserted */
#define UNUSABLE_ERR		-17 /* Unusable Card */
#define COMM_ERR		-18 /* Communications Error */
#define TIMEOUT			-19

struct r_mobile_sdhi {
	struct mci_host		mci;
	struct mci_cmd		*cmd;
	struct mci_data		*data;
	struct device_d		*dev;
	unsigned long addr;
};

static unsigned short g_wait_int;
static unsigned short g_sd_error;

static inline void sdhi_writew(struct r_mobile_sdhi *sdcard, int reg, unsigned val)
{
	writew(val, sdcard->addr + reg);
}

static inline unsigned sdhi_readw(struct r_mobile_sdhi *sdcard, int reg)
{
	return readw(sdcard->addr + reg);
}