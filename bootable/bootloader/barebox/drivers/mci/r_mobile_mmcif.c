/*
 * Copyright (C) 2012 Renesas Mobile Corporation
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

#include "r_mobile_mmcif.h"

#define to_mmcif(mci)	container_of(mci, struct r_mobile_mmcif, mci)

static unsigned int g_wait_int;
static unsigned int g_sd_error;

/*
 * r_mobile_mmcif_intr: interrupt processing
 * @return 0     : Successful
 *         Other : Fail
 */
static int r_mobile_mmcif_intr(struct r_mobile_mmcif *mmc)
{
	unsigned state = 0;

	state = r_mobile_mmcif_readl(mmc, MMCIF_CE_INT)&
		r_mobile_mmcif_readl(mmc, MMCIF_CE_INT_MASK);

	if (state & INT_ERR_STS) {
		r_mobile_mmcif_writel(mmc, MMCIF_CE_INT, ~state);
		r_mobile_mmcif_bitclr(mmc, MMCIF_CE_INT_MASK, state);
		pr_debug("%s: int err state = %08x\n", DRIVER_NAME, state);
		g_sd_error = 1;
		g_wait_int = 1;
		return 0;
	}
	/* Respons End */
	if (state & INT_CRSPE) {
		r_mobile_mmcif_writel(mmc, MMCIF_CE_INT, ~(INT_CRSPE));
		r_mobile_mmcif_bitclr(mmc, MMCIF_CE_INT_MASK, MASK_MCRSPE);
		if ((r_mobile_mmcif_readl(mmc, MMCIF_CE_CMD_SET) &
				CMD_SET_RBSY) == 0) {
			g_wait_int = 1;
			return 0;
		}
	}
	if (state & INT_RBSYE) {
		r_mobile_mmcif_writel(mmc, MMCIF_CE_INT, ~(INT_RBSYE));
		r_mobile_mmcif_bitclr(mmc, MMCIF_CE_INT_MASK, MASK_MRBSYE);
		g_wait_int = 1;
		return 0;
	}

	if (state & (INT_CMD12DRE | INT_CMD12RBE)) {
		r_mobile_mmcif_writel(mmc, MMCIF_CE_INT,
			~(INT_CMD12DRE | INT_CMD12RBE |
			  INT_CMD12CRE | INT_BUFRE));
		r_mobile_mmcif_bitclr(mmc, MMCIF_CE_INT_MASK,
			MASK_MCMD12DRE | MASK_MCMD12RBE);
		g_wait_int = 1;
		return 0;
	}

	/* SD_BUF Read/Write Enable */
	if (state & (INT_BUFREN | INT_BUFWEN)) {
		r_mobile_mmcif_writel(mmc, MMCIF_CE_INT,
				~(INT_BUFREN | INT_BUFWEN));
		r_mobile_mmcif_bitclr(mmc, MMCIF_CE_INT_MASK, MASK_MBUFREN | MASK_MBUFWEN);
		g_wait_int = 1;
		return 0;
	}
	/* Access End */
	if (state & (INT_BUFRE | INT_DTRANE)) {
		r_mobile_mmcif_writel(mmc, MMCIF_CE_INT,
			~(INT_BUFRE | INT_DTRANE));
		r_mobile_mmcif_bitclr(mmc, MMCIF_CE_INT_MASK, MASK_MBUFRE | MASK_MDTRANE);
		g_wait_int = 1;
		return 0;
	}
	return -EAGAIN;
}

/*
 * r_mobile_mmcif_wait_interrupt_flag
 * @return 0     : Successful
 *         Other : Fail
 */
static int r_mobile_mmcif_wait_interrupt_flag(struct r_mobile_mmcif *mmc)
{
	int timeout = 10000000;

	while (1) {
		timeout--;
		if (timeout < 0) {
			printf("timeout\n");
			return 0;
		}

		if (!r_mobile_mmcif_intr(mmc)){
			break;
		}
		udelay(1);	/* 1 usec */
	}

	return 1;	/* Return value: NOT 0 = complete waiting */
}

/*
 * r_mobile_mmcif_clock_control
 * @return 0     : Successful
 *         Other : Fail
 */
static void r_mobile_mmcif_clock_control(struct r_mobile_mmcif *mmc, unsigned int clk)
{
	u32 clkdiv, i;
	r_mobile_mmcif_bitclr(mmc, MMCIF_CE_CLK_CTRL, CLK_ENABLE);
	r_mobile_mmcif_bitclr(mmc, MMCIF_CE_CLK_CTRL, CLK_CLEAR);

	if (clk == 0)
		return;

	clkdiv = 0x9;
	i = CONFIG_SH_MMCIF_FREQ >> (0x9 + 1);
	for ( ; clkdiv && clk >= (i << 1); clkdiv--)
		i <<= 1;

	r_mobile_mmcif_bitset(mmc, MMCIF_CE_CLK_CTRL, clkdiv << 16);
	r_mobile_mmcif_bitset(mmc, MMCIF_CE_CLK_CTRL, CLK_ENABLE);
	
}

/*
 * r_mobile_mmcif_sync_reset
 * @return 0     : Successful
 *         Other : Fail
 */
static void r_mobile_mmcif_sync_reset(struct r_mobile_mmcif *mmc)
{
	unsigned tmp;

	tmp = 0x010f0000 & r_mobile_mmcif_readl(mmc, MMCIF_CE_CLK_CTRL);

	r_mobile_mmcif_writel(mmc, MMCIF_CE_VERSION, SOFT_RST_ON);
	r_mobile_mmcif_writel(mmc, MMCIF_CE_VERSION, SOFT_RST_OFF);
	r_mobile_mmcif_bitset(mmc, MMCIF_CE_CLK_CTRL, tmp |
		SRSPTO_256 | SRBSYTO_29 | SRWDTO_29 | SCCSTO_29);
	/* byte swap on */
	r_mobile_mmcif_bitset(mmc, MMCIF_CE_BUF_ACC, BUF_ACC_ATYP);
}

/*
 * r_mobile_mmcif_error_manage
 * @return 0     : Successful
 *         Other : Fail
 */
static int r_mobile_mmcif_error_manage(struct r_mobile_mmcif *mmc)
{
	unsigned state1, state2;
	int ret;

	g_sd_error = 0;
	g_wait_int = 0;

	state1 = r_mobile_mmcif_readl(mmc, MMCIF_CE_HOST_STS1);
	state2 = r_mobile_mmcif_readl(mmc, MMCIF_CE_HOST_STS2);

	pr_debug("%s: state1=%x state2=%x\n", __func__, state1, state2);

	if (state1 & STS1_CMDSEQ) {
		r_mobile_mmcif_bitset(mmc, MMCIF_CE_CMD_CTRL, CMD_CTRL_BREAK);
		r_mobile_mmcif_bitset(mmc, MMCIF_CE_CMD_CTRL, ~CMD_CTRL_BREAK);
		while (1) {
			if (!(r_mobile_mmcif_readl(mmc, MMCIF_CE_HOST_STS1)
							& STS1_CMDSEQ)){
				break;
			}
			udelay(1000);
		}
		r_mobile_mmcif_sync_reset(mmc);
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

/*
 * r_mobile_mmcif_single_read
 * @return 0     : Successful
 *         Other : Fail
 */
static int
r_mobile_mmcif_single_read(struct r_mobile_mmcif *mmc, struct mci_data *data)
{
	long time;
	unsigned long blocksize, i;
	unsigned long *p = (unsigned long *)data->dest;

	if ((unsigned long)p & 0x00000001) {
		printf("%s: The data pointer is unaligned.", __func__);
		return -EIO;
	}

	g_wait_int = 0;
	r_mobile_mmcif_bitset(mmc, MMCIF_CE_INT_MASK, MASK_MBUFREN);
	time = r_mobile_mmcif_wait_interrupt_flag(mmc);
	if (time == 0 || g_sd_error != 0){
		return r_mobile_mmcif_error_manage(mmc);
	}

	g_wait_int = 0;
	blocksize = (BLOCK_SIZE_MASK &
			r_mobile_mmcif_readl(mmc, MMCIF_CE_BLOCK_SET)) + 3;

	for (i = 0; i < blocksize / 4; i++){
		*p++ = r_mobile_mmcif_readl(mmc, MMCIF_CE_DATA);
	}

	/* buffer read end */
	r_mobile_mmcif_bitset(mmc, MMCIF_CE_INT_MASK, MASK_MBUFRE);
	time = r_mobile_mmcif_wait_interrupt_flag(mmc);
	if (time == 0 || g_sd_error != 0){
		return r_mobile_mmcif_error_manage(mmc);
	}

	g_wait_int = 0;
	return 0;
}

/*
 * r_mobile_mmcif_multi_read
 * @return 0     : Successful
 *         Other : Fail
 */
static int
r_mobile_mmcif_multi_read(struct r_mobile_mmcif *mmc, struct mci_data *data)
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

	blocksize = BLOCK_SIZE_MASK & r_mobile_mmcif_readl(mmc,
					MMCIF_CE_BLOCK_SET);

	g_wait_int = 0;
	for (sec = 0; sec < data->blocks; sec++) {
		r_mobile_mmcif_bitset(mmc, MMCIF_CE_INT_MASK, MASK_MBUFREN);

		time = r_mobile_mmcif_wait_interrupt_flag(mmc);
		if (time == 0 || g_sd_error != 0){
			return r_mobile_mmcif_error_manage(mmc);
		}

		g_wait_int = 0;
		for (i = 0; i < blocksize / 4; i++){
			*p++ = r_mobile_mmcif_readl(mmc, MMCIF_CE_DATA);
		}
	}
	/* buffer read end */
	r_mobile_mmcif_bitset(mmc, MMCIF_CE_INT_MASK, MASK_MCMD12DRE);
	time = r_mobile_mmcif_wait_interrupt_flag(mmc);
	if (time == 0 || g_sd_error != 0){
		return r_mobile_mmcif_error_manage(mmc);
	}
	g_wait_int = 0;

	return 0;
}

/*
 * r_mobile_mmcif_single_write
 * @return 0     : Successful
 *         Other : Fail
 */
static int
r_mobile_mmcif_single_write(struct r_mobile_mmcif *mmc, struct mci_data *data)
{
	long time;
	unsigned blocksize, i;
	const unsigned long *p = (const unsigned long *)data->src;

	if ((unsigned long)p & 0x00000001) {
		printf("%s: The data pointer is unaligned.", __func__);
		return -EIO;
	}

	pr_debug("%s: blocks = %d, blocksize = %d\n",
			__func__, data->blocks, data->blocksize);

	g_wait_int = 0;
	r_mobile_mmcif_bitset(mmc, MMCIF_CE_INT_MASK, MASK_MBUFWEN);

	time = r_mobile_mmcif_wait_interrupt_flag(mmc);
	if (time == 0 || g_sd_error != 0){
		return r_mobile_mmcif_error_manage(mmc);
	}

	g_wait_int = 0;
	blocksize = (BLOCK_SIZE_MASK &
			r_mobile_mmcif_readl(mmc, MMCIF_CE_BLOCK_SET)) + 3;
	for (i = 0; i < blocksize / 4; i++){
		r_mobile_mmcif_writel(mmc, MMCIF_CE_DATA, *p++);
	}

	/* buffer write end */
	r_mobile_mmcif_bitset(mmc, MMCIF_CE_INT_MASK, MASK_MDTRANE);
	time = r_mobile_mmcif_wait_interrupt_flag(mmc);
	if (time == 0 || g_sd_error != 0){
		return r_mobile_mmcif_error_manage(mmc);
	}

	g_wait_int = 0;
	return 0;
}

/*
 * r_mobile_mmcif_multi_write
 * @return 0     : Successful
 *         Other : Fail
 */
static int
r_mobile_mmcif_multi_write(struct r_mobile_mmcif *mmc, struct mci_data *data)
{
	long time;
	unsigned i, sec, blocksize;
	const unsigned long *p = (const unsigned long *)data->src;

	pr_debug("%s: blocks = %d, blocksize = %d\n",
			__func__, data->blocks, data->blocksize);

	blocksize = BLOCK_SIZE_MASK & r_mobile_mmcif_readl(mmc,
						     MMCIF_CE_BLOCK_SET);

	g_wait_int = 0;
	for (sec = 0; sec < data->blocks; sec++) {
		r_mobile_mmcif_bitset(mmc, MMCIF_CE_INT_MASK, MASK_MBUFWEN);

		time = r_mobile_mmcif_wait_interrupt_flag(mmc);
		if (time == 0 || g_sd_error != 0){
			return r_mobile_mmcif_error_manage(mmc);
		}

		g_wait_int = 0;
		for (i = 0; i < blocksize / 4; i++){
			r_mobile_mmcif_writel(mmc, MMCIF_CE_DATA, *p++);
		}
	}
	/* buffer write end */
	r_mobile_mmcif_bitset(mmc, MMCIF_CE_INT_MASK, MASK_MCMD12RBE);
	time = r_mobile_mmcif_wait_interrupt_flag(mmc);
	if (time == 0 || g_sd_error != 0)
		return r_mobile_mmcif_error_manage(mmc);
	g_wait_int = 0;

	return 0;
}

/**
 * r_mobile_mmcif_get_response
 * @return : None
 */
static void r_mobile_mmcif_get_response(struct r_mobile_mmcif *mmc,
						struct mci_cmd *cmd)
{
	if (cmd->resp_type & MMC_RSP_136) {
		cmd->response[0] = r_mobile_mmcif_readl(mmc, MMCIF_CE_RESP3);
		cmd->response[1] = r_mobile_mmcif_readl(mmc, MMCIF_CE_RESP2);
		cmd->response[2] = r_mobile_mmcif_readl(mmc, MMCIF_CE_RESP1);
		cmd->response[3] = r_mobile_mmcif_readl(mmc, MMCIF_CE_RESP0);
	} else{
		cmd->response[0] = r_mobile_mmcif_readl(mmc, MMCIF_CE_RESP0);
	}
}

/*
 * r_mobile_mmcif_set_cmd
 * @return 0     : Successful
 *         Other : Fail
 */
static u32 r_mobile_mmcif_set_cmd(struct r_mobile_mmcif *mmc,
		struct mci_data *data, struct mci_cmd *cmd, unsigned opc)
{
	unsigned tmp = 0;

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
		switch (mmc->mci.bus_width) {
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
			pr_err("Not support bus width.\n");
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

/*
 * r_mobile_mmcif_data_trans
 * @return 0     : Successful
 *         Other : Fail
 */
static u32 r_mobile_mmcif_data_trans(struct r_mobile_mmcif *mmc,
				struct mci_data *data, unsigned opc)
{
	unsigned ret;

	switch (opc) {
	case MMC_CMD_READ_MULTIPLE_BLOCK:
		ret = r_mobile_mmcif_multi_read(mmc, data);
		break;
	case MMC_CMD_WRITE_MULTIPLE_BLOCK:
		ret = r_mobile_mmcif_multi_write(mmc, data);
		break;
	case MMC_CMD_WRITE_SINGLE_BLOCK:
		ret = r_mobile_mmcif_single_write(mmc, data);
		break;
	case MMC_CMD_READ_SINGLE_BLOCK:
	case MMC_CMD_SEND_EXT_CSD:
		ret = r_mobile_mmcif_single_read(mmc, data);
		break;
	default:
		pr_err("NOT SUPPORT CMD = d'%08d\n", opc);
		ret = -EINVAL;
		break;
	}
	return ret;
}

/*
 * r_mobile_mmcif_start_cmd
 * @return 0     : Successful
 *         Other : Fail
 */
static int r_mobile_mmcif_start_cmd(struct r_mobile_mmcif *mmc,
			struct mci_data *data, struct mci_cmd *cmd)
{
	long time;
	int ret = 0, mask = 0;
	unsigned opc = cmd->cmdidx;

	pr_debug("opc = %d, arg = %x, resp_type = %x\n",
		 opc, cmd->cmdarg, cmd->resp_type);

	if (opc == MMC_CMD_STOP_TRANSMISSION){
		return 0;
	}

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

	if (mmc->data) {
		r_mobile_mmcif_writel(mmc, MMCIF_CE_BLOCK_SET,
			data->blocksize | (data->blocks << 16));
	}

	opc = r_mobile_mmcif_set_cmd(mmc, data, cmd, opc);

	/*
	 *  Bootloader cannot use interrupt.
	 *  So this flag may not be clear by timing
	 */
	r_mobile_mmcif_writel(mmc, MMCIF_CE_INT, 0xD80430C0);
	r_mobile_mmcif_writel(mmc, MMCIF_CE_INT_MASK, mask);
	/* set arg */
	r_mobile_mmcif_writel(mmc, MMCIF_CE_ARG, cmd->cmdarg);
	/* set cmd */
	r_mobile_mmcif_writel(mmc, MMCIF_CE_CMD_SET, opc);

	g_wait_int = 0;

	time = r_mobile_mmcif_wait_interrupt_flag(mmc);
	if (time == 0){
		return r_mobile_mmcif_error_manage(mmc);
	}

	if (g_sd_error) {
		switch (cmd->cmdidx) {
		case MMC_CMD_ALL_SEND_CID:
		case MMC_CMD_SELECT_CARD:
		case SD_CMD_SEND_IF_COND:
		case MMC_CMD_APP_CMD:
			r_mobile_mmcif_error_manage(mmc);
			ret = -ETIMEDOUT;
			break;
		default:
			pr_debug(DRIVER_NAME": Cmd(h'%x) err\n", opc);
			pr_debug(DRIVER_NAME": cmdidx = %d\n", cmd->cmdidx);
			ret = r_mobile_mmcif_error_manage(mmc);
			break;
		}
		g_sd_error = 0;
		g_wait_int = 0;
		return ret;
	}

	if (g_wait_int == 1) {
		r_mobile_mmcif_get_response(mmc, cmd);
		g_wait_int = 0;
	}
	if (mmc->data){
		ret = r_mobile_mmcif_data_trans(mmc, data, cmd->cmdidx);
	}
	pr_debug("ret = %d, resp = %08x, %08x, %08x, %08x\n",
		 ret, cmd->response[0], cmd->response[1],
		 cmd->response[2], cmd->response[3]);

	return ret;
}

/*
 * r_mobile_mmcif_request
 * @return 0     : Successful
 *         Other : Fail
 */
static int r_mobile_mmcif_request(struct mci_host *mci, struct mci_cmd *cmd,
			struct mci_data *data)
{
	struct r_mobile_mmcif *mmc = to_mmcif(mci);
	int ret;

	g_sd_error = 0;

	mmc->cmd = cmd;	
	mmc->data = data;
	ret = r_mobile_mmcif_start_cmd(mmc, data, cmd);
	mmc->cmd = NULL;
	mmc->data = NULL;

	return ret;
}

/*
 * r_mobile_mmcif_set_ios
 * @return 0     : Successful
 *         Other : Fail
 */
static void r_mobile_mmcif_set_ios(struct mci_host* mci, struct device_d* dev, unsigned bus_width, unsigned clock)
{
	struct r_mobile_mmcif *mmc = to_mmcif(mci);

	switch (clock) {
	case CLKDEV_INIT:
	case CLKDEV_SD_DATA:
	case CLKDEV_HS_DATA:
	case CLKDEV_HS52_DATA:
	case CLKDEV_MMC_DATA:
	case CLKDEV_MMC26_DATA:
		r_mobile_mmcif_clock_control(mmc, clock);
		break;
	case 0:
	default:
		r_mobile_mmcif_clock_control(mmc, 0);
		break;
	}
	
	
	pr_debug("clock = %d, buswidth = %d\n", mmc->clock, mmc->bus_width);
}

/*
 * r_mobile_mmcif_init
 * @return 0   
 */
static int r_mobile_mmcif_init(struct mci_host *mci, struct device_d *dev)
{
	return 0;
}

/*
 * r_mobile_mmcif_probe
 * @return 0     : Successful
 *         Other : Fail
 */
int r_mobile_mmcif_probe(struct device_d *dev)
{
	int ret = 0;
	struct r_mobile_mmcif * mmc;	
	
	mmc = xzalloc(sizeof(*mmc));
	
	mmc->dev = dev;
	mmc->mci.f_min = 400000;
	mmc->mci.f_max = 52000000;
	mmc->mci.voltages = MMC_VDD_165_195 | MMC_VDD_32_33 | MMC_VDD_33_34;
	mmc->mci.host_caps = MMC_MODE_4BIT | MMC_MODE_8BIT |
				MMC_MODE_HS | MMC_MODE_HS_52MHz;
		
	mmc->mci.send_cmd = r_mobile_mmcif_request;
	mmc->mci.set_ios = r_mobile_mmcif_set_ios;
	mmc->mci.init = r_mobile_mmcif_init;

	mmc->addr = MMCIF_BASE;
	
	r_mobile_mmcif_sync_reset(mmc);
	r_mobile_mmcif_writel(mmc, MMCIF_CE_INT_MASK, MASK_ALL);
	
	mci_register(&mmc->mci);

	return ret;
}

static struct driver_d r_mobile_mmcif_driver = {
        .name  = "r_mobile_mmcif",
        .probe = r_mobile_mmcif_probe,
};

/*
 * r_mobile_mmcif_init_driver
 * @return 0    
 */
static int r_mobile_mmcif_init_driver(void)
{
        register_driver(&r_mobile_mmcif_driver);
        return 0;
}

device_initcall(r_mobile_mmcif_init_driver);
