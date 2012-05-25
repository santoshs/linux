/*
 * Copyright (C) 2010 Renesas Electronics Corporation
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
#include <command.h>
#include <mmc.h>
#include <malloc.h>
#include <mmc.h>
#include <asm/errno.h>
#include <asm/io.h>

#include "sh_mmcif.h"

#ifdef DEBUG
#define pr_debug	printf
#else
#define pr_debug(...)
#endif

#define pr_err	printf

#define DRIVER_NAME "sh-mmcif"

static unsigned int g_wait_int;
static unsigned int g_sd_error;


static void *mmc_priv(struct mmc *mmc)
{
	return (void *)mmc->priv;
}

static int sh_mmcif_intr(struct sh_mmcif_host *host)
{
	u32 state = 0;

	state = sh_mmcif_readl(host, CE_INT) &
		sh_mmcif_readl(host, CE_INT_MASK);

	if (state & INT_ERR_STS) {
		sh_mmcif_writel(host, CE_INT, ~state);
		sh_mmcif_bitclr(host, CE_INT_MASK, state);
		pr_debug("%s: int err state = %08x\n", DRIVER_NAME, state);
		g_sd_error = 1;
		g_wait_int = 1;
		return 0;
	}
	/* Respons End */
	if (state & INT_CRSPE) {
		sh_mmcif_writel(host, CE_INT, ~INT_CRSPE);
		sh_mmcif_bitclr(host, CE_INT_MASK, MASK_MCRSPE);
		if ((sh_mmcif_readl(host, CE_CMD_SET) & CMD_SET_RBSY) == 0) {
			g_wait_int = 1;
			return 0;
		}
	}
	if (state & INT_RBSYE) {
		sh_mmcif_writel(host, CE_INT, ~INT_RBSYE);
		sh_mmcif_bitclr(host, CE_INT_MASK, MASK_MRBSYE);
		g_wait_int = 1;
		return 0;
	}

	if (state & (INT_CMD12DRE | INT_CMD12RBE)) {
		sh_mmcif_writel(host, CE_INT,
			~(INT_CMD12DRE | INT_CMD12RBE |
			  INT_CMD12CRE | INT_BUFRE));
		sh_mmcif_bitclr(host, CE_INT_MASK,
			MASK_MCMD12DRE | MASK_MCMD12RBE);
		g_wait_int = 1;
		return 0;
	}

	/* SD_BUF Read/Write Enable */
	if (state & (INT_BUFREN | INT_BUFWEN)) {
		sh_mmcif_writel(host, CE_INT, ~(INT_BUFREN | INT_BUFWEN));
		sh_mmcif_bitclr(host, CE_INT_MASK, MASK_MBUFREN | MASK_MBUFWEN);
		g_wait_int = 1;
		return 0;
	}
	/* Access End */
	if (state & (INT_BUFRE | INT_DTRANE)) {
		sh_mmcif_writel(host, CE_INT, ~(INT_BUFRE | INT_DTRANE));
		sh_mmcif_bitclr(host, CE_INT_MASK, MASK_MBUFRE | MASK_MDTRANE);
		g_wait_int = 1;
		return 0;
	}
	return -EAGAIN;
}

static int sh_mmcif_wait_interrupt_flag(struct sh_mmcif_host *host)
{
	int timeout = 10000000;

	while (1) {
		timeout--;
		if (timeout < 0) {
			printf("timeout\n");
			return 0;
		}

		if (!sh_mmcif_intr(host))
			break;

		udelay(1);	/* 1 usec */
	}

	return 1;	/* Return value: NOT 0 = complete waiting */
}

static void sh_mmcif_clock_control(struct sh_mmcif_host *host, unsigned int clk)
{
	u32 clkdiv, i;

	sh_mmcif_bitclr(host, CE_CLK_CTRL, CLK_ENABLE);
	sh_mmcif_bitclr(host, CE_CLK_CTRL, CLK_CLEAR);

	if (clk == 0)
		return;

	clkdiv = 0x9;
	i = CONFIG_SH_MMCIF_FREQ >> (0x9 + 1);
	for ( ; clkdiv && clk >= (i << 1); clkdiv--)
		i <<= 1;

	sh_mmcif_bitset(host, CE_CLK_CTRL, clkdiv << 16);
	sh_mmcif_bitset(host, CE_CLK_CTRL, CLK_ENABLE);
}

static void sh_mmcif_sync_reset(struct sh_mmcif_host *host)
{
	u32 tmp;

	tmp = 0x010f0000 & sh_mmcif_readl(host, CE_CLK_CTRL);

	sh_mmcif_writel(host, CE_VERSION, SOFT_RST_ON);
	sh_mmcif_writel(host, CE_VERSION, SOFT_RST_OFF);
	sh_mmcif_bitset(host, CE_CLK_CTRL, tmp |
		SRSPTO_256 | SRBSYTO_29 | SRWDTO_29 | SCCSTO_29);
	/* byte swap on */
	sh_mmcif_bitset(host, CE_BUF_ACC, BUF_ACC_ATYP);
}

static int sh_mmcif_error_manage(struct sh_mmcif_host *host)
{
	u32 state1, state2;
	int ret;

	g_sd_error = 0;
	g_wait_int = 0;

	state1 = sh_mmcif_readl(host, CE_HOST_STS1);
	state2 = sh_mmcif_readl(host, CE_HOST_STS2);

	pr_debug("%s: state1=%x state2=%x\n", __func__, state1, state2);

	if (state1 & STS1_CMDSEQ) {
		sh_mmcif_bitset(host, CE_CMD_CTRL, CMD_CTRL_BREAK);
		sh_mmcif_bitset(host, CE_CMD_CTRL, ~CMD_CTRL_BREAK);
		while (1) {
			if (!(sh_mmcif_readl(host, CE_HOST_STS1) & STS1_CMDSEQ))
				break;
			udelay(1000);
		}
		sh_mmcif_sync_reset(host);
		pr_debug(DRIVER_NAME": Forced end of command sequence\n");
		return -EIO;
	}

	if (state2 & STS2_CRC_ERR) {
		pr_debug(DRIVER_NAME": Happened CRC error\n");
		ret = -EIO;
	} else if (state2 & STS2_TIMEOUT_ERR) {
		pr_debug(DRIVER_NAME": Happened Timeout error\n");
		ret = TIMEOUT;
	} else {
		pr_debug(DRIVER_NAME": Happened End/Index error\n");
		ret = -EIO;
	}
	return ret;
}

static int
sh_mmcif_single_read(struct sh_mmcif_host *host, struct mmc_data *data)
{
	long time;
	unsigned long blocksize, i;
	unsigned long *p = (unsigned long *)data->dest;

	if ((unsigned long)p & 0x00000001) {
		printf("%s: The data pointer is unaligned.", __func__);
		return -EIO;
	}

	g_wait_int = 0;
	sh_mmcif_bitset(host, CE_INT_MASK, MASK_MBUFREN);
	time = sh_mmcif_wait_interrupt_flag(host);
	if (time == 0 || g_sd_error != 0)
		return sh_mmcif_error_manage(host);

	g_wait_int = 0;
	blocksize = (BLOCK_SIZE_MASK & sh_mmcif_readl(host, CE_BLOCK_SET)) + 3;

	for (i = 0; i < blocksize / 4; i++)
		*p++ = sh_mmcif_readl(host, CE_DATA);

	/* buffer read end */
	sh_mmcif_bitset(host, CE_INT_MASK, MASK_MBUFRE);
	time = sh_mmcif_wait_interrupt_flag(host);
	if (time == 0 || g_sd_error != 0)
		return sh_mmcif_error_manage(host);

	g_wait_int = 0;
	return 0;
}

static int
sh_mmcif_multi_read(struct sh_mmcif_host *host, struct mmc_data *data)
{
	long time;
	unsigned long blocksize, i, sec;
	unsigned long *p = (unsigned long *)data->dest;

	if ((unsigned long)p & 0x00000001) {
		printf("%s: The data pointer is unaligned.", __func__);
		return -EIO;
	}

	pr_debug("%s: blocks = %d, blocksize = %d\n",
			__func__, data->blocks, data->blocksize);

	blocksize = BLOCK_SIZE_MASK & sh_mmcif_readl(host, CE_BLOCK_SET);

	g_wait_int = 0;
	for (sec = 0; sec < data->blocks; sec++) {
		sh_mmcif_bitset(host, CE_INT_MASK, MASK_MBUFREN);

		time = sh_mmcif_wait_interrupt_flag(host);
		if (time == 0 || g_sd_error != 0)
			return sh_mmcif_error_manage(host);

		g_wait_int = 0;
		for (i = 0; i < blocksize / 4; i++)
			*p++ = sh_mmcif_readl(host, CE_DATA);
	}

	/* buffer read end */
	sh_mmcif_bitset(host, CE_INT_MASK, MASK_MCMD12DRE);
	time = sh_mmcif_wait_interrupt_flag(host);
	if (time == 0 || g_sd_error != 0)
		return sh_mmcif_error_manage(host);
	g_wait_int = 0;

	return 0;
}

static int
sh_mmcif_single_write(struct sh_mmcif_host *host, struct mmc_data *data)
{
	long time;
	u32 blocksize, i;
	const unsigned long *p = (const unsigned long *)data->src;

	if ((unsigned long)p & 0x00000001) {
		printf("%s: The data pointer is unaligned.", __func__);
		return -EIO;
	}

	pr_debug("%s: blocks = %d, blocksize = %d\n",
			__func__, data->blocks, data->blocksize);

	g_wait_int = 0;
	sh_mmcif_bitset(host, CE_INT_MASK, MASK_MBUFWEN);

	time = sh_mmcif_wait_interrupt_flag(host);
	if (time == 0 || g_sd_error != 0)
		return sh_mmcif_error_manage(host);

	g_wait_int = 0;
	blocksize = (BLOCK_SIZE_MASK & sh_mmcif_readl(host, CE_BLOCK_SET)) + 3;
	for (i = 0; i < blocksize / 4; i++)
		sh_mmcif_writel(host, CE_DATA, *p++);

	/* buffer write end */
	sh_mmcif_bitset(host, CE_INT_MASK, MASK_MDTRANE);
	time = sh_mmcif_wait_interrupt_flag(host);
	if (time == 0 || g_sd_error != 0)
		return sh_mmcif_error_manage(host);

	g_wait_int = 0;
	return 0;
}

static int
sh_mmcif_multi_write(struct sh_mmcif_host *host, struct mmc_data *data)
{
	long time;
	u32 i, sec, blocksize;
	const unsigned long *p = (const unsigned long *)data->src;

	pr_debug("%s: blocks = %d, blocksize = %d\n",
			__func__, data->blocks, data->blocksize);

	blocksize = BLOCK_SIZE_MASK & sh_mmcif_readl(host, CE_BLOCK_SET);

	g_wait_int = 0;
	for (sec = 0; sec < data->blocks; sec++) {
		sh_mmcif_bitset(host, CE_INT_MASK, MASK_MBUFWEN);

		time = sh_mmcif_wait_interrupt_flag(host);
		if (time == 0 || g_sd_error != 0)
			return sh_mmcif_error_manage(host);

		g_wait_int = 0;
		for (i = 0; i < blocksize / 4; i++)
			sh_mmcif_writel(host, CE_DATA, *p++);
	}

	/* buffer write end */
	sh_mmcif_bitset(host, CE_INT_MASK, MASK_MCMD12RBE);
	time = sh_mmcif_wait_interrupt_flag(host);
	if (time == 0 || g_sd_error != 0)
		return sh_mmcif_error_manage(host);
	g_wait_int = 0;

	return 0;
}

static void sh_mmcif_get_response(struct sh_mmcif_host *host,
						struct mmc_cmd *cmd)
{
	if (cmd->resp_type & MMC_RSP_136) {
		cmd->response[0] = sh_mmcif_readl(host, CE_RESP3);
		cmd->response[1] = sh_mmcif_readl(host, CE_RESP2);
		cmd->response[2] = sh_mmcif_readl(host, CE_RESP1);
		cmd->response[3] = sh_mmcif_readl(host, CE_RESP0);
	} else
		cmd->response[0] = sh_mmcif_readl(host, CE_RESP0);
}

static u32 sh_mmcif_set_cmd(struct sh_mmcif_host *host,
		struct mmc_data *data, struct mmc_cmd *cmd, u32 opc)
{
	u32 tmp = 0;

	/* Response Type check */
	switch (cmd->resp_type) {
	case MMC_RSP_NONE:
		tmp |= CMD_SET_RTYP_NO;
		break;
	case MMC_RSP_R1:
	case MMC_RSP_R1b:
	case MMC_RSP_R3:
		tmp |= CMD_SET_RTYP_6B;
		break;
	case MMC_RSP_R2:
		tmp |= CMD_SET_RTYP_17B;
		break;
	default:
		pr_debug(DRIVER_NAME": Not support type response.\n");
		break;
	}

	switch (opc) {
	/* RBSY */
	case MMC_CMD_SWITCH:
	case MMC_CMD_STOP_TRANSMISSION:
		tmp |= CMD_SET_RBSY;
		break;
	}

	/* WDAT / DATW */
	if (data) {
		tmp |= CMD_SET_WDAT;
		switch (host->bus_width) {
		case 1:
			tmp |= CMD_SET_DATW_1;
			break;
		case 4:
			tmp |= CMD_SET_DATW_4;
			break;
		case 8:
			tmp |= CMD_SET_DATW_8;
			break;
		default:
			pr_err(DRIVER_NAME": Not support bus width.\n");
			break;
		}
	}

	/* DWEN */
	if (opc == MMC_CMD_WRITE_SINGLE_BLOCK ||
			opc == MMC_CMD_WRITE_MULTIPLE_BLOCK)
		tmp |= CMD_SET_DWEN;

	/* CMLTE/CMD12EN */
	if (opc == MMC_CMD_READ_MULTIPLE_BLOCK ||
			opc == MMC_CMD_WRITE_MULTIPLE_BLOCK)
		tmp |= CMD_SET_CMLTE | CMD_SET_CMD12EN;

	/* RIDXC[1:0] check bits */
	if (opc == MMC_CMD_SEND_OP_COND || opc == MMC_CMD_ALL_SEND_CID ||
	    opc == MMC_CMD_SEND_CSD || opc == MMC_CMD_SEND_CID)
		tmp |= CMD_SET_RIDXC_BITS;
	/* RCRC7C[1:0] check bits */
	if (opc == MMC_CMD_SEND_OP_COND)
		tmp |= CMD_SET_CRC7C_BITS;
	/* RCRC7C[1:0] internal CRC7 */
	if (opc == MMC_CMD_ALL_SEND_CID ||
		opc == MMC_CMD_SEND_CSD || opc == MMC_CMD_SEND_CID)
		tmp |= CMD_SET_CRC7C_INTERNAL;

	return opc = ((opc << 24) | tmp);
}

static u32 sh_mmcif_data_trans(struct sh_mmcif_host *host,
				struct mmc_data *data, u32 opc)
{
	u32 ret;

	switch (opc) {
	case MMC_CMD_READ_MULTIPLE_BLOCK:
		ret = sh_mmcif_multi_read(host, data);
		break;
	case MMC_CMD_WRITE_MULTIPLE_BLOCK:
		ret = sh_mmcif_multi_write(host, data);
		break;
	case MMC_CMD_WRITE_SINGLE_BLOCK:
		ret = sh_mmcif_single_write(host, data);
		break;
	case MMC_CMD_READ_SINGLE_BLOCK:
	case MMC_CMD_SEND_EXT_CSD:
		ret = sh_mmcif_single_read(host, data);
		break;
	default:
		pr_err(DRIVER_NAME": NOT SUPPORT CMD = d'%08d\n", opc);
		ret = -EINVAL;
		break;
	}
	return ret;
}

static int sh_mmcif_start_cmd(struct sh_mmcif_host *host,
			struct mmc_data *data, struct mmc_cmd *cmd)
{
	long time;
	int ret = 0, mask = 0;
	u32 opc = cmd->cmdidx;

	pr_debug("opc = %d, arg = %x, resp_type = %x\n",
		 opc, cmd->cmdarg, cmd->resp_type);

	if (opc == MMC_CMD_STOP_TRANSMISSION)
		return 0;

	switch (opc) {
	/* respons busy check */
	case MMC_CMD_SWITCH:
	case MMC_CMD_STOP_TRANSMISSION:
		mask = MASK_MRBSYE;
		break;
	default:
		mask = MASK_MCRSPE;
		break;
	}
	mask |=	MASK_MCMDVIO | MASK_MBUFVIO | MASK_MWDATERR |
		MASK_MRDATERR | MASK_MRIDXERR | MASK_MRSPERR |
		MASK_MCCSTO | MASK_MCRCSTO | MASK_MWDATTO |
		MASK_MRDATTO | MASK_MRBSYTO | MASK_MRSPTO;

	if (host->data) {
		sh_mmcif_writel(host, CE_BLOCK_SET,
			data->blocksize | (data->blocks << 16));
	}

	opc = sh_mmcif_set_cmd(host, data, cmd, opc);

	/*
	 *  U-boot cannot use interrupt.
	 *  So this flag may not be clear by timing
	 */
	sh_mmcif_writel(host, CE_INT, 0xD80430C0);
	sh_mmcif_writel(host, CE_INT_MASK, mask);
	/* set arg */
	sh_mmcif_writel(host, CE_ARG, cmd->cmdarg);
	/* set cmd */
	sh_mmcif_writel(host, CE_CMD_SET, opc);

	g_wait_int = 0;

	time = sh_mmcif_wait_interrupt_flag(host);
	if (time == 0)
		return sh_mmcif_error_manage(host);

	if (g_sd_error) {
		switch (cmd->cmdidx) {
		case MMC_CMD_ALL_SEND_CID:
		case MMC_CMD_SELECT_CARD:
		case SD_CMD_SEND_IF_COND:
		case MMC_CMD_APP_CMD:
			sh_mmcif_error_manage(host);
			ret = TIMEOUT;
			break;
		default:
			pr_debug(DRIVER_NAME": Cmd(h'%x) err\n", opc);
			pr_debug(DRIVER_NAME": cmdidx = %d\n", cmd->cmdidx);
			ret = sh_mmcif_error_manage(host);
			break;
		}
		g_sd_error = 0;
		g_wait_int = 0;
		return ret;
	}

	if (g_wait_int == 1) {
		sh_mmcif_get_response(host, cmd);
		g_wait_int = 0;
	}
	if (host->data)
		ret = sh_mmcif_data_trans(host, data, cmd->cmdidx);

	pr_debug("ret = %d, resp = %08x, %08x, %08x, %08x\n",
		 ret, cmd->response[0], cmd->response[1],
		 cmd->response[2], cmd->response[3]);

	return ret;
}

static int mmcif_request(struct mmc *mmc, struct mmc_cmd *cmd,
			struct mmc_data *data)
{
	struct sh_mmcif_host *host = mmc_priv(mmc);
	int ret;

	g_sd_error = 0;

	host->cmd = cmd;
	host->data = data;
	ret = sh_mmcif_start_cmd(host, data, cmd);
	host->cmd = NULL;
	host->data = NULL;

	return ret;
}

static void mmcif_set_ios(struct mmc *mmc)
{
	struct sh_mmcif_host *host = mmc_priv(mmc);

	switch (mmc->clock) {
	case CLKDEV_INIT:
	case CLKDEV_SD_DATA:
	case CLKDEV_HS_DATA:
	case CLKDEV_HS52_DATA:
	case CLKDEV_MMC_DATA:
	case CLKDEV_MMC26_DATA:
		sh_mmcif_clock_control(host, mmc->clock);
		break;
	case 0:
	default:
		sh_mmcif_clock_control(host, 0);
		break;
	}

	host->bus_width = mmc->bus_width;

	pr_debug("clock = %d, buswidth = %d\n", mmc->clock, mmc->bus_width);
}

static int mmcif_init(struct mmc *mmc)
{
	return 0;
}

int mmcif_mmc_init(unsigned long addr)
{
	int ret = 0;
	struct mmc *mmc;
	struct sh_mmcif_host *host = NULL;

	mmc = malloc(sizeof(struct mmc));
	if (!mmc)
		return -ENOMEM;
	host = malloc(sizeof(struct sh_mmcif_host));
	if (!host)
		return -ENOMEM;

	mmc->f_min = 400000;
	mmc->f_max = 52000000;
	mmc->voltages = MMC_VDD_165_195 | MMC_VDD_32_33 | MMC_VDD_33_34;
	mmc->host_caps = MMC_MODE_4BIT | MMC_MODE_8BIT |
				MMC_MODE_HS | MMC_MODE_HS_52MHz;
	memcpy(mmc->name, DRIVER_NAME, sizeof(DRIVER_NAME));
	mmc->send_cmd = mmcif_request;
	mmc->set_ios = mmcif_set_ios;
	mmc->init = mmcif_init;
	host->addr = addr;
	mmc->priv = host;

	sh_mmcif_sync_reset(host);
	sh_mmcif_writel(host, CE_INT_MASK, MASK_ALL);

	mmc_register(mmc);

	return ret;
}
