/*
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
 *
 * Copyright (C) 2011-2012 Renesas Mobile Corporation.
 * All rights reserved.
 *
 */

#include "r_mobile_sdhi.h"

#define to_sdhi(mci)	container_of(mci, struct r_mobile_sdhi, mci)
#define DRIVER_NAME	"r_mobile_sdhi"

static int detect_waiting;

/*
 * sdhi_detect - Card detect
 * @return 0     : Successful
 *         Other : Fail
 */
static void sdhi_detect(struct r_mobile_sdhi *sdcard)
{
	sdhi_writew(sdcard, SDHI_OPTION,
			OPT_BUS_WIDTH_1 | sdhi_readw(sdcard, SDHI_OPTION));

	detect_waiting = 0;
}

/*
 * sdhi_intr - Interrupt process
 * @return 0     : Successful
 *         Other : Fail
 */
static int sdhi_intr(void *dev_id)
{
	struct r_mobile_sdhi *sdcard = dev_id;
	int state1 = 0, state2 = 0;

	state1 = sdhi_readw(sdcard, SDHI_INFO1);
	state2 = sdhi_readw(sdcard, SDHI_INFO2);

	pr_debug("%s: state1 = %x, state2 = %x\n", __func__, state1, state2);

	/* CARD Insert */
	if (state1 & INFO1_CARD_IN) {
		sdhi_writew(sdcard, SDHI_INFO1, ~INFO1_CARD_IN);
		if (!detect_waiting) {
			detect_waiting = 1;
			sdhi_detect(sdcard);
		}
		sdhi_writew(sdcard, SDHI_INFO1_MASK, INFO1M_RESP_END	|
				INFO1M_ACCESS_END		|
				INFO1M_CARD_IN		|
				INFO1M_DATA3_CARD_RE	|
				INFO1M_DATA3_CARD_IN);
		return -EAGAIN;
	}
	/* CARD Removal */
	if (state1 & INFO1_CARD_RE) {
		sdhi_writew(sdcard, SDHI_INFO1, ~INFO1_CARD_RE);
		if (!detect_waiting) {
			detect_waiting = 1;
			sdhi_detect(sdcard);
		}
		sdhi_writew(sdcard, SDHI_INFO1_MASK, INFO1M_RESP_END |
				INFO1M_ACCESS_END		|
				INFO1M_CARD_RE		|
				INFO1M_DATA3_CARD_RE	|
				INFO1M_DATA3_CARD_IN);
		sdhi_writew(sdcard, SDHI_SDIO_INFO1_MASK, SDIO_INFO1M_ON);
		sdhi_writew(sdcard, SDHI_SDIO_MODE, SDIO_MODE_OFF);
		return -EAGAIN;
	}

	if (state2 & INFO2_ALL_ERR) {
		sdhi_writew(sdcard, SDHI_INFO2, (unsigned short)~(INFO2_ALL_ERR));
		sdhi_writew(sdcard, SDHI_INFO2_MASK,
			INFO2M_ALL_ERR | sdhi_readw(sdcard, SDHI_INFO2_MASK));
		g_sd_error = 1;
		g_wait_int = 1;
		return 0;
	}
	/* Respons End */
	if (state1 & INFO1_RESP_END) {
		sdhi_writew(sdcard, SDHI_INFO1, ~INFO1_RESP_END);
		sdhi_writew(sdcard, SDHI_INFO1_MASK,
			INFO1M_RESP_END | sdhi_readw(sdcard, SDHI_INFO1_MASK));
		g_wait_int = 1;
		return 0;
	}
	/* SD_BUF Read Enable */
	if (state2 & INFO2_BRE_ENABLE) {
		sdhi_writew(sdcard, SDHI_INFO2, ~INFO2_BRE_ENABLE);
		sdhi_writew(sdcard, SDHI_INFO2_MASK,
				INFO2M_BRE_ENABLE | INFO2M_BUF_ILL_READ |
				sdhi_readw(sdcard, SDHI_INFO2_MASK));
		g_wait_int = 1;
		return 0;
	}
	/* SD_BUF Write Enable */
	if (state2 & INFO2_BWE_ENABLE) {
		sdhi_writew(sdcard, SDHI_INFO2, ~INFO2_BWE_ENABLE);
		sdhi_writew(sdcard, SDHI_INFO2_MASK,
				INFO2_BWE_ENABLE | INFO2M_BUF_ILL_WRITE |
				sdhi_readw(sdcard, SDHI_INFO2_MASK));
		g_wait_int = 1;
		return 0;
	}
	/* Access End */
	if (state1 & INFO1_ACCESS_END) {
		sdhi_writew(sdcard, SDHI_INFO1, ~INFO1_ACCESS_END);
		sdhi_writew(sdcard, SDHI_INFO1_MASK,
			INFO1_ACCESS_END | sdhi_readw(sdcard, SDHI_INFO1_MASK));
		g_wait_int = 1;
		return 0;
	}
	return -EAGAIN;
}

/*
 * sdhi_wait_interrupt_flag - Interrupt flag
 * @return 0     : Successful
 *         Other : Fail
 */
static int sdhi_wait_interrupt_flag(struct r_mobile_sdhi *sdcard)
{
	int timeout = 10000000;

	while (1) {
		timeout--;
		if (timeout < 0) {
			printf("timeout\n");
			return 0;
		}

		if (!sdhi_intr(sdcard)){
			break;
		}

		udelay(1);	/* 1 usec */
	}

	return 1;	/* Return value: NOT 0 = complete waiting */
}

/*
 * sdhi_clock_control - clock control
 * @return 0     : Successful
 *         Other : Fail
 */
static void sdhi_clock_control(struct r_mobile_sdhi *sdcard, unsigned long clk)
{
	if (sdhi_readw(sdcard, SDHI_INFO2) & (1 << 14)) {
		printf(DRIVER_NAME": Busy state ! Cannot change the clock\n");
		return;
	}

	sdhi_writew(sdcard, SDHI_CLK_CTRL,
			~CLK_ENABLE & sdhi_readw(sdcard, SDHI_CLK_CTRL));
	switch (clk) {
	case 0:
		return;
	case CLKDEV_INIT:
		sdhi_writew(sdcard, SDHI_CLK_CTRL, CLK_INIT);
		break;
	case CLKDEV_SD_DATA:
		sdhi_writew(sdcard, SDHI_CLK_CTRL, CLK_SD_TRANS);
		break;
	case CLKDEV_HS_DATA:
		sdhi_writew(sdcard, SDHI_CLK_CTRL, CLK_HS_TRANS);
		break;
	case CLKDEV_MMC_DATA:
		sdhi_writew(sdcard, SDHI_CLK_CTRL, CLK_MMC_TRANS);
		break;
	default:
		return;
	}

	/* Waiting for SD Bus busy to be cleared */
	while ((sdhi_readw(sdcard, SDHI_INFO2) & 0x2000) == 0);

	sdhi_writew(sdcard, SDHI_CLK_CTRL,
			CLK_ENABLE | sdhi_readw(sdcard, SDHI_CLK_CTRL));
}

/*
 * sdhi_sync_reset
 * @return 0     : Successful
 *         Other : Fail
 */
static void sdhi_sync_reset(struct r_mobile_sdhi *sdcard)
{
	sdhi_writew(sdcard, SDHI_SOFT_RST, SOFT_RST_ON);
	sdhi_writew(sdcard, SDHI_SOFT_RST, SOFT_RST_OFF);
	sdhi_writew(sdcard, SDHI_CLK_CTRL,
			CLK_ENABLE | sdhi_readw(sdcard, SDHI_CLK_CTRL));
}

/*
 * sdhi_error_manage
 * @return 0     : Successful
 *         Other : Fail
 */
static int sdhi_error_manage(struct r_mobile_sdhi *sdcard)
{
	unsigned short e_state1, e_state2;
	int ret;

	g_sd_error = 0;
	g_wait_int = 0;

	if (!(sdhi_readw(sdcard, SDHI_INFO1) & INFO1_ISD0CD)) {
		pr_debug("%s: card remove(INFO1 = %04x)\n", \
			DRIVER_NAME, sdhi_readw(sdcard, SDHI_INFO1));
		ret = NO_CARD_ERR;
	}

	e_state1 = sdhi_readw(sdcard, SDHI_ERR_STS1);
	e_state2 = sdhi_readw(sdcard, SDHI_ERR_STS2);
	if (e_state2 & ERR_STS2_SYS_ERROR) {
		if (e_state2 & ERR_STS2_RES_STOP_TIMEOUT){
			ret = TIMEOUT;
		}else{
			ret = -EILSEQ;
		}
		pr_debug("%s: ERR_STS2 = %04x\n", \
				DRIVER_NAME, sdhi_readw(sdcard, SDHI_ERR_STS2));
		sdhi_sync_reset(sdcard);
		sdhi_writew(sdcard, SDHI_INFO1_MASK,
				INFO1M_DATA3_CARD_RE | INFO1M_DATA3_CARD_IN);
		return ret;
	}
	if (e_state1 & ERR_STS1_CRC_ERROR || e_state1 & ERR_STS1_CMD_ERROR){
		ret = -EILSEQ;
	}else{
		ret = TIMEOUT;
	}

	pr_debug("%s: ERR_STS1 = %04x \n", DRIVER_NAME,
		sdhi_readw(sdcard, SDHI_ERR_STS1));
	sdhi_sync_reset(sdcard);
	sdhi_writew(sdcard, SDHI_INFO1_MASK,
			INFO1M_DATA3_CARD_RE | INFO1M_DATA3_CARD_IN);
	return ret;
}

/*
 * sdhi_single_read
 * @return 0     : Successful
 *         Other : Fail
 */
static int sdhi_single_read(struct r_mobile_sdhi *sdcard, struct mci_data *data)
{
	long time;
/*	long timeout; */
	unsigned short blocksize, i;
	unsigned short *p = (unsigned short *)data->dest;

	if ((unsigned long)p & 0x00000001) {
		printf("%s: The data pointer is unaligned.", __func__);
		return -EIO;
	}

	g_wait_int = 0;
	sdhi_writew(sdcard, SDHI_INFO2_MASK,
			~(INFO2M_BRE_ENABLE | INFO2M_BUF_ILL_READ) &
			sdhi_readw(sdcard, SDHI_INFO2_MASK));
	sdhi_writew(sdcard, SDHI_INFO1_MASK,
			~INFO1M_ACCESS_END & sdhi_readw(sdcard, SDHI_INFO1_MASK));
	time = sdhi_wait_interrupt_flag(sdcard);
	if (time == 0 || g_sd_error != 0){
		return sdhi_error_manage(sdcard);
	}

	g_wait_int = 0;
	blocksize = sdhi_readw(sdcard, SDHI_SIZE);
	for (i = 0; i < blocksize / 2; i++)
		*p++ = sdhi_readw(sdcard, SDHI_BUF0);

/* 	timeout = g_timeout; */
	time = sdhi_wait_interrupt_flag(sdcard);
	if (time == 0 || g_sd_error != 0){
		return sdhi_error_manage(sdcard);
	}

	g_wait_int = 0;
	return 0;
}

/*
 * sdhi_multi_read
 * @return 0     : Successful
 *         Other : Fail
 */
static int sdhi_multi_read(struct r_mobile_sdhi *sdcard, struct mci_data *data)
{
	long time;
	unsigned short blocksize, i, sec;
	unsigned short *p = (unsigned short *)data->dest;

	if ((unsigned long)p & 0x00000001) {
		printf("%s: The data pointer is unaligned.", __func__);
		return -EIO;
	}

	pr_debug("%s: blocks = %d, blocksize = %d\n",
			__func__, data->blocks, data->blocksize);

	g_wait_int = 0;
	for (sec = 0; sec < data->blocks; sec++) {
		sdhi_writew(sdcard, SDHI_INFO2_MASK,
				~(INFO2M_BRE_ENABLE | INFO2M_BUF_ILL_READ) &
				sdhi_readw(sdcard, SDHI_INFO2_MASK));

		time = sdhi_wait_interrupt_flag(sdcard);
		if (time == 0 || g_sd_error != 0){
			return sdhi_error_manage(sdcard);
		}

		g_wait_int = 0;
		blocksize = sdhi_readw(sdcard, SDHI_SIZE);
		for (i = 0; i < blocksize / 2; i++)
			*p++ = sdhi_readw(sdcard, SDHI_BUF0);
	}

	return 0;
}

/*
 * sdhi_single_write
 * @return 0     : Successful
 *         Other : Fail
 */
static int sdhi_single_write(struct r_mobile_sdhi *sdcard, struct mci_data *data)
{
	long time;
	unsigned short blocksize, i;
	const unsigned short *p = (const unsigned short *)data->src;

	if ((unsigned long)p & 0x00000001) {
		printf("%s: The data pointer is unaligned.", __func__);
		return -EIO;
	}

	pr_debug("%s: blocks = %d, blocksize = %d\n",
			__func__, data->blocks, data->blocksize);

	g_wait_int = 0;
	sdhi_writew(sdcard, SDHI_INFO2_MASK,
			~(INFO2M_BWE_ENABLE | INFO2M_BUF_ILL_WRITE) &
			sdhi_readw(sdcard, SDHI_INFO2_MASK));
	sdhi_writew(sdcard, SDHI_INFO1_MASK,
			~INFO1M_ACCESS_END & sdhi_readw(sdcard, SDHI_INFO1_MASK));

	time = sdhi_wait_interrupt_flag(sdcard);
	if (time == 0 || g_sd_error != 0){
		return sdhi_error_manage(sdcard);
	}

	g_wait_int = 0;
	blocksize = sdhi_readw(sdcard, SDHI_SIZE);
	for (i = 0; i < blocksize / 2; i++){
		sdhi_writew(sdcard, SDHI_BUF0, *p++);
	}

	time = sdhi_wait_interrupt_flag(sdcard);
	if (time == 0 || g_sd_error != 0){
		return sdhi_error_manage(sdcard);
	}
	g_wait_int = 0;
	return 0;
}

/*
 * sdhi_multi_write
 * @return 0     : Successful
 *         Other : Fail
 */
static int sdhi_multi_write(struct r_mobile_sdhi *sdcard, struct mci_data *data)
{
	long time;
	unsigned short i, sec, blocksize;
	const unsigned short *p = (const unsigned short *)data->src;

	pr_debug("%s: blocks = %d, blocksize = %d\n",
			__func__, data->blocks, data->blocksize);

	g_wait_int = 0;
	for (sec = 0; sec < data->blocks; sec++) {
		sdhi_writew(sdcard, SDHI_INFO2_MASK,
				~(INFO2M_BWE_ENABLE | INFO2M_BUF_ILL_WRITE) &
				sdhi_readw(sdcard, SDHI_INFO2_MASK));

		time = sdhi_wait_interrupt_flag(sdcard);
		if (time == 0 || g_sd_error != 0){
			return sdhi_error_manage(sdcard);
		}

		g_wait_int = 0;
		blocksize = sdhi_readw(sdcard, SDHI_SIZE);
		for (i = 0; i < blocksize / 2; i++){
			sdhi_writew(sdcard, SDHI_BUF0, *p++);
		}
	}

	return 0;
}

/*
 * sdhi_get_response
 * @return: None
 */
static void sdhi_get_response(struct r_mobile_sdhi *sdcard, struct mci_cmd *cmd)
{
	unsigned short i, j;
	volatile unsigned short resp[8];
	volatile unsigned long *p1, *p2;

	if (cmd->resp_type & MMC_RSP_136) {
		resp[0] = sdhi_readw(sdcard, SDHI_RSP00);
		resp[1] = sdhi_readw(sdcard, SDHI_RSP01);
		resp[2] = sdhi_readw(sdcard, SDHI_RSP02);
		resp[3] = sdhi_readw(sdcard, SDHI_RSP03);
		resp[4] = sdhi_readw(sdcard, SDHI_RSP04);
		resp[5] = sdhi_readw(sdcard, SDHI_RSP05);
		resp[6] = sdhi_readw(sdcard, SDHI_RSP06);
		resp[7] = sdhi_readw(sdcard, SDHI_RSP07);

		/* SDHI REGISTER SPECIFICATION */
		for (i = 7, j = 6; i > 0; i--) {
			resp[i] = (resp[i] << 8) & 0xff00;
			resp[i] |= (resp[j--] >> 8) & 0x00ff;
		}
		resp[0] = (resp[0] << 8) & 0xff00;
		/* SDHI REGISTER SPECIFICATION */

		p1 = ((unsigned long *)resp) + 3;
		p2 = (unsigned long *)cmd->response;
/* #if defined(__BIG_ENDIAN_BITFIELD)
		for (i = 0; i < 4; i++) {
			*p2++ = ((*p1 >> 16) & 0x0000ffff) |
					((*p1 << 16) & 0xffff0000);
			p1--;
		}
#else */
		for (i = 0; i < 4; i++)
			*p2++ = *p1--;
// #endif /* __BIG_ENDIAN_BITFIELD */ 

	} else {
		resp[0] = sdhi_readw(sdcard, SDHI_RSP00);
		resp[1] = sdhi_readw(sdcard, SDHI_RSP01);

		p1 = ((unsigned long *)resp);
		p2 = (unsigned long *)cmd->response;
/* #if defined(__BIG_ENDIAN_BITFIELD)
		*p2 = ((*p1 >> 16) & 0x0000ffff) | ((*p1 << 16) & 0xffff0000);
#else */
		*p2 = *p1;
// #endif /* __BIG_ENDIAN_BITFIELD */ 
	}
}

/*
 * sdhi_set_cmd
 * @return 0     : Successful
 *         Other : Fail
 */
static unsigned short sdhi_set_cmd(struct r_mobile_sdhi *sdcard,
			struct mci_data *data, unsigned short opc)
{
	switch (opc) {
	case SD_CMD_APP_SEND_OP_COND:
	case SD_CMD_APP_SEND_SCR:
		opc |= SDHI_APP;
		break;
	case SD_CMD_APP_SET_BUS_WIDTH:
		 /* SD_APP_SET_BUS_WIDTH*/
		if (sdcard->data == 0)
			opc |= SDHI_APP;
		else /* SD_SWITCH */
			opc = SDHI_SD_SWITCH;
		break;
	default:
		break;
	}
	return opc;
}

/*
 * sdhi_data_trans
 * @return 0     : Successful
 *         Other : Fail
 */
static unsigned short sdhi_data_trans(struct r_mobile_sdhi *sdcard,
				struct mci_data *data, unsigned short opc)
{
	unsigned short ret;

	switch (opc) {
	case MMC_CMD_READ_MULTIPLE_BLOCK:
		ret = sdhi_multi_read(sdcard, data);
		break;
	case MMC_CMD_WRITE_MULTIPLE_BLOCK:
		ret = sdhi_multi_write(sdcard, data);
		break;
	case MMC_CMD_WRITE_SINGLE_BLOCK:
		ret = sdhi_single_write(sdcard, data);
		break;
	case MMC_CMD_READ_SINGLE_BLOCK:
	case SDHI_SD_APP_SEND_SCR:
	case SDHI_SD_SWITCH: /* SD_SWITCH */
		ret = sdhi_single_read(sdcard, data);
		break;
	default:
		printf(DRIVER_NAME": SD: NOT SUPPORT CMD = d'%04d\n", opc);
		ret = -EINVAL;
		break;
	}
	return ret;

}

/*
 * sdhi_start_cmd
 * @return 0     : Successful
 *         Other : Fail
 */
static int sdhi_start_cmd(struct r_mobile_sdhi *sdcard,
			struct mci_data *data, struct mci_cmd *cmd)
{
	long time;
	unsigned short opc = cmd->cmdidx;
	int ret = 0;

	pr_debug("opc = %d, arg = %x, resp_type = %x\n",
		 opc, cmd->cmdarg, cmd->resp_type);

	if (opc == MMC_CMD_STOP_TRANSMISSION) {
		/* SDHI sends the STOP command automatically by STOP reg */
		sdhi_writew(sdcard, SDHI_INFO1_MASK, ~INFO1M_ACCESS_END &
				sdhi_readw(sdcard, SDHI_INFO1_MASK));

		time = sdhi_wait_interrupt_flag(sdcard);
		if (time == 0 || g_sd_error != 0){
			return sdhi_error_manage(sdcard);
		}

		sdhi_get_response(sdcard, cmd);
		return 0;
	}

	if (sdcard->data) {
		if ((opc == MMC_CMD_READ_MULTIPLE_BLOCK) ||
		     opc == MMC_CMD_WRITE_MULTIPLE_BLOCK) {
			sdhi_writew(sdcard, SDHI_STOP, STOP_SEC_ENABLE);
			sdhi_writew(sdcard, SDHI_SECCNT, data->blocks);
		}
		sdhi_writew(sdcard, SDHI_SIZE, data->blocksize);
	}
	opc = sdhi_set_cmd(sdcard, data, opc);

	/*
	 *  U-boot cannot use interrupt.
	 *  So this flag may not be clear by timing
	 */
	sdhi_writew(sdcard, SDHI_INFO1, ~INFO1_RESP_END);

	sdhi_writew(sdcard, SDHI_INFO1_MASK,
			INFO1M_RESP_END | sdhi_readw(sdcard, SDHI_INFO1_MASK));
	sdhi_writew(sdcard, SDHI_ARG0, (unsigned short)(cmd->cmdarg & ARG0_MASK));
	sdhi_writew(sdcard, SDHI_ARG1,
			(unsigned short)((cmd->cmdarg >> 16) & ARG1_MASK));

	/* Waiting for SD Bus busy to be cleared */
	while ((sdhi_readw(sdcard, SDHI_INFO2) & 0x2000) == 0);

	sdhi_writew(sdcard, SDHI_CMD, (unsigned short)(opc & CMD_MASK));

	g_wait_int = 0;
	sdhi_writew(sdcard, SDHI_INFO1_MASK,
			~INFO1M_RESP_END & sdhi_readw(sdcard, SDHI_INFO1_MASK));
	sdhi_writew(sdcard, SDHI_INFO2_MASK,
			~(INFO2M_CMD_ERROR | INFO2M_CRC_ERROR |
			  INFO2M_END_ERROR | INFO2M_TIMEOUT   |
			  INFO2M_RESP_TIMEOUT | INFO2M_ILA)   &
			  sdhi_readw(sdcard, SDHI_INFO2_MASK));

	time = sdhi_wait_interrupt_flag(sdcard);
	if (time == 0){
		return sdhi_error_manage(sdcard);
	}

	if (g_sd_error) {
		switch (cmd->cmdidx) {
		case MMC_CMD_ALL_SEND_CID:
		case MMC_CMD_SELECT_CARD:
		case SD_CMD_SEND_IF_COND:
		case MMC_CMD_APP_CMD:
			ret = TIMEOUT;
			break;
		default:
			printf(DRIVER_NAME": Cmd(d'%d) err\n", opc);
			printf(DRIVER_NAME": cmdidx = %d\n", cmd->cmdidx);
			ret = sdhi_error_manage(sdcard);
			break;
		}
		g_sd_error = 0;
		g_wait_int = 0;
		return ret;
	}
	if (sdhi_readw(sdcard, SDHI_INFO1) & INFO1_RESP_END){
		return -EINVAL;
	}

	if (g_wait_int == 1) {
		sdhi_get_response(sdcard, cmd);
		g_wait_int = 0;
	}
	if (sdcard->data){
		ret = sdhi_data_trans(sdcard, data, opc);
	}

	pr_debug("ret = %d, resp = %08x, %08x, %08x, %08x\n",
		 ret, cmd->response[0], cmd->response[1],
		 cmd->response[2], cmd->response[3]);
	return ret;
}

/*
 * sdhi_request
 * @return 0     : Successful
 *         Other : Fail
 */
static int sdhi_request(struct mci_host *mci, struct mci_cmd *cmd,
			struct mci_data *data)
{
	struct r_mobile_sdhi *sdcard = to_sdhi(mci);
	int ret;
#if 0
	if (!(sdhi_readw(sdcard, SDHI_INFO1) & INFO1_ISD0CD)){
		return NO_CARD_ERR;
	}
#endif

	g_sd_error = 0;

	sdcard->data = data;
	ret = sdhi_start_cmd(sdcard, data, cmd);
	sdcard->data = NULL;

	return ret;
}

/*
 * sdhi_set_ios
 * @return:		None 
 */
static void sdhi_set_ios(struct mci_host *mci, struct device_d *dev, 
						unsigned bus_width, unsigned clock)
{
	struct r_mobile_sdhi *sdcard = to_sdhi(mci);

	switch (clock) {
	case CLKDEV_INIT:
	case CLKDEV_SD_DATA:
	case CLKDEV_HS_DATA:
	case CLKDEV_MMC_DATA:
		sdhi_clock_control(sdcard, clock);
		break;
	case 0:
	default:
		sdhi_clock_control(sdcard, mci->clock);
		break;
	}

	if (bus_width == 4){
		sdhi_writew(sdcard, SDHI_OPTION, ~OPT_BUS_WIDTH_1 &
					sdhi_readw(sdcard, SDHI_OPTION));
	}else{
		sdhi_writew(sdcard, SDHI_OPTION, OPT_BUS_WIDTH_1 |
					sdhi_readw(sdcard, SDHI_OPTION));
	}

	pr_debug("clock = %d, buswidth = %d\n", mci->clock, mci->bus_width);
}

/**
 * sdhi_init
 * @return 0     : Successful
 *         Other : Fail
 */
static int sdhi_init(struct mci_host *mci, struct device_d *dev)
{
	return 0;
}

/*
 * sdhi_mmc_probe : probe process
 * @return 0     : Successful
 *         Other : Fail
 */
int sdhi_mmc_probe(struct device_d *dev)
{
	struct r_mobile_sdhi *sdcard;

	sdcard = xzalloc(sizeof(*sdcard));
	if (!sdcard)
		return -ENOMEM;

	sdcard->dev = dev;
	
	sdcard->mci.f_min = 400000;
	sdcard->mci.f_max = 96000000;
	sdcard->mci.voltages = MMC_VDD_32_33 | MMC_VDD_33_34;
	sdcard->mci.host_caps = MMC_MODE_4BIT | MMC_MODE_HS;

	sdcard->mci.send_cmd = sdhi_request;
	sdcard->mci.set_ios = sdhi_set_ios;
	sdcard->mci.init = sdhi_init;
	
	sdcard->addr = SDHI_BASE;

	sdhi_sync_reset(sdcard);
	
	sdhi_writew(sdcard, SDHI_PORTSEL, USE_1PORT);

/* #if defined(__BIG_ENDIAN_BITFIELD)
	sdhi_writew(host, SDHI_EXT_SWAP, SET_SWAP);
#endif */

	mci_register(&sdcard->mci);

	sdhi_writew(sdcard, SDHI_INFO1_MASK, INFO1M_RESP_END | INFO1M_ACCESS_END
			| INFO1M_CARD_RE | INFO1M_DATA3_CARD_RE
			| INFO1M_DATA3_CARD_IN);
	return 0;
}

static struct driver_d r_mobile_sdhi_drv = {
        .name  = "r_mobile_sdhi",
        .probe = sdhi_mmc_probe,
};

/*
 * r_mobile_sdhi_init_drv 
 * @return 0     : Successful
 */
static int r_mobile_sdhi_init_drv(void)
{
        register_driver(&r_mobile_sdhi_drv);
        return 0;
}

device_initcall(r_mobile_sdhi_init_drv);
