/*
 * Renesas SDHI driver
 *
 * Copyright (C) 2011 Renesas Electronics Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/dmaengine.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/workqueue.h>
#include <linux/pagemap.h>
#include <linux/scatterlist.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/mmc/host.h>
#include <linux/mmc/core.h>
#include <linux/mmc/sdio.h>
#include <linux/mmc/renesas_sdhi.h>

#define SDHI_CMD		0x00
#define SDHI_ARG		0x04
#define SDHI_STOP		0x08
#define SDHI_SECCNT		0x0a
#define SDHI_RSP		0x0c
#define SDHI_INFO		0x1c
#define SDHI_INFO_MASK		0x20
#define SDHI_CLK_CTRL		0x24
#define SDHI_SIZE		0x26
#define SDHI_OPTION		0x28
#define SDHI_ERR_STS		0x2c
#define SDHI_BUF0		0x30
#define SDHI_SDIO_MODE		0x34
#define SDHI_SDIO_INFO		0x36
#define SDHI_SDIO_INFO_MASK	0x38
#define SDHI_DMA_MODE		0xd8
#define SDHI_SOFT_RST		0xe0
#define SDHI_EXT_ACC		0xe4
#define SDHI_EXT_SWAP		0xf0

/* SDHI_CMD */
#define SDHI_CMD_NO_CMD12	(1 << 14)
#define SDHI_CMD_MULTI		(1 << 13)
#define SDHI_CMD_READ		(1 << 12)
#define SDHI_CMD_DATA		(1 << 11)
#define SDHI_CMD_RSP_R1		(4 << 8)
#define SDHI_CMD_RSP_R1B	(5 << 8)
#define SDHI_CMD_RSP_R2		(6 << 8)
#define SDHI_CMD_RSP_R3		(7 << 8)
#define SDHI_CMD_ACMD		(1 << 6)

/* SDHI_STOP */
#define SDHI_STOP_MULTI		(1 << 8)
#define SDHI_STOP_STOP		(1 << 0)

/* SDHI_INFO */
#define SDHI_INFO_ILA		(1 << 31)
#define SDHI_INFO_DIVEN		(1 << 29)
#define SDHI_INFO_BWE		(1 << 25)
#define SDHI_INFO_BRE		(1 << 24)
#define SDHI_INFO_DAT0		(1 << 23)
#define SDHI_INFO_CTO		(1 << 22)
#define SDHI_INFO_ILRA		(1 << 21)
#define SDHI_INFO_ILWA		(1 << 20)
#define SDHI_INFO_DTO		(1 << 19)
#define SDHI_INFO_ENDERR	(1 << 18)
#define SDHI_INFO_CRCERR	(1 << 17)
#define SDHI_INFO_CMDERR	(1 << 16)
#define SDHI_INFO_WP		(1 << 7)
#define SDHI_INFO_CD		(1 << 5)
#define SDHI_INFO_CARD_INSERT	(1 << 4)
#define SDHI_INFO_CARD_REMOVE	(1 << 3)
#define SDHI_INFO_RW_END	(1 << 2)
#define SDHI_INFO_RSP_END	(1 << 0)

#define SDHI_INFO_ALLERR	0x807f0000
#define SDHI_INFO_DETECT	(SDHI_INFO_CARD_INSERT | SDHI_INFO_CARD_REMOVE)

/* SDHI_CLK_CTRL */
#define SDHI_CLK_OFFEN		(1 << 9)
#define SDHI_CLK_EN		(1 << 8)

/* SDHI_OPTION */
#define SDHI_OPT_WIDTH1		(1 << 15)
#define SDHI_OPT_TIMEOUT	(0x0e << 4)

/* SDHI_SDIO_MODE */
#define SDHI_MODE_IOMOD		(1 << 0)

/* SDHI_SDIO_INFO */
#define SDHI_SDIO_IOIRQ		(1 << 0)

/* SDHI_EXT_MODE */
#define SDHI_DMA_EN		(1 << 1)

/* SDHI_SOFT_RST */
#define SDHI_RST_UNRESET	(1 << 0)


#define SDHI_MIN_DMA_LEN	8
#define SDHI_TIMEOUT		5000	/* msec */

struct renesas_sdhi_host {
	struct mmc_host *mmc;
	struct mmc_request *mrq;
	struct mmc_command *cmd;
	struct mmc_data *data;
	struct platform_device *pdev;
	struct renesas_sdhi_platdata *pdata;
	void __iomem *base;
	unsigned long bus_shift;

	/* current state */
	u8 bus_width;
	u8 power_mode;
	u8 app_mode;
	u8 connect;
	u8 dynamic_clock;
	u32 clock;
	u32 info_mask;

	struct clk *clk;
	unsigned int hclk;	/* master clock */

	spinlock_t lock;

	struct workqueue_struct *work;
	struct delayed_work	detect_wq;
	struct delayed_work	timeout_wq;

	/* DMA support */
	bool			force_pio;
	struct dma_chan		*dma_tx;
	struct dma_chan		*dma_rx;
	u32			burst_size;

	/* pio related stuff */
	struct scatterlist      *sg_ptr;
	struct scatterlist      *sg_orig;
	unsigned int            sg_len;
	unsigned int            sg_off;
};

static u16 sdhi_read16(struct renesas_sdhi_host *host, u32 offset)
{
	return __raw_readw(host->base + (offset << host->bus_shift));
}

static void sdhi_read16s(struct renesas_sdhi_host *host,
		u32 offset, u16 *buf, int count)
{
	__raw_readsw(host->base + (offset << host->bus_shift), buf, count);
}

static u32 sdhi_read32(struct renesas_sdhi_host *host, u32 offset)
{
	return __raw_readw(host->base + (offset << host->bus_shift)) |
	  __raw_readw(host->base + ((offset + 2) << host->bus_shift)) << 16;
}

static void sdhi_write16(struct renesas_sdhi_host *host, u32 offset, u16 val)
{
	int timeout = 0;

	switch (offset) {
	case SDHI_CMD:
	case SDHI_STOP:
	case SDHI_SECCNT:
	case SDHI_CLK_CTRL:
	case SDHI_SIZE:
	case SDHI_OPTION:
	case SDHI_SDIO_MODE:
	case SDHI_EXT_ACC:
		while ((sdhi_read32(host, SDHI_INFO) & SDHI_INFO_DIVEN) == 0) {
			if (timeout++ > 43)
				break;
			udelay(1);
		}
		break;
	}
	__raw_writew(val, host->base + (offset << host->bus_shift));
}

static void sdhi_write16s(struct renesas_sdhi_host *host,
		u32 offset, u16 *buf, int count)
{
	__raw_writesw(host->base + (offset << host->bus_shift), buf, count);
}

static void sdhi_write32(struct renesas_sdhi_host *host, u32 offset, u32 val)
{
	__raw_writew(val & 0xffff, host->base + (offset << host->bus_shift));
	__raw_writew(val >> 16, host->base + ((offset + 2) << host->bus_shift));
}

static void sdhi_enable_irqs(struct renesas_sdhi_host *host, u32 i)
{
	host->info_mask &= ~i;
	sdhi_write32(host, SDHI_INFO_MASK, host->info_mask);
}

static void sdhi_disable_irqs(struct renesas_sdhi_host *host, u32 i)
{
	host->info_mask |= i;
	sdhi_write32(host, SDHI_INFO_MASK, host->info_mask);
}

static void sdhi_dma_enable(struct renesas_sdhi_host *host, bool enable)
{
	struct renesas_sdhi_platdata *pdata = host->pdata;
	u16 val;

	sdhi_write16(host, SDHI_DMA_MODE, enable ? pdata->dma_en_val : 0);

	if (pdata->flags & RENESAS_SDHI_DMA_SLAVE_CONFIG)
		pdata->set_dma(host->pdev, 2);

	if (pdata->dma_buf_acc32) {
		val = sdhi_read16(host, SDHI_EXT_ACC);
		sdhi_write16(host, SDHI_EXT_ACC,
				enable ? (val | 1) : (val & ~1));
	}
}

static void sdhi_reset(struct renesas_sdhi_host *host)
{
	sdhi_write16(host, SDHI_SOFT_RST, 0x0000);
	sdhi_write16(host, SDHI_SOFT_RST, 0x0001);

	host->info_mask = 0xffffffff;
	sdhi_enable_irqs(host, SDHI_INFO_ALLERR);
	if (!host->dynamic_clock)
		sdhi_enable_irqs(host, SDHI_INFO_DETECT);
	sdhi_write32(host, SDHI_INFO, 0);
	sdhi_write16(host, SDHI_OPTION, SDHI_OPT_WIDTH1 | SDHI_OPT_TIMEOUT);

	/*
	 * Ensure that the data size of DMA transfer is set to default '2'
	 * on reset, regardless of RENESAS_SDHI_DMA_SLAVE_CONFIG option.
	 */
	if (host->pdata->set_dma)
		host->pdata->set_dma(host->pdev, 2);

	host->clock = 0;
	host->bus_width = 0;
	host->app_mode = 0;
	host->force_pio = false;
	host->burst_size = 0;
}

static void renesas_sdhi_set_clock(
	struct renesas_sdhi_host *host, int new_clock)
{
	u32 clk = 0, clock;

	if (new_clock) {
		for (clock = host->mmc->f_min, clk = 0x80000080;
				new_clock >= (clock<<1); clk >>= 1)
			clock <<= 1;
		clk |= 0x100;
		if (host->pdata->flags & RENESAS_SDHI_SDCLK_OFFEN)
			clk |= 0x200;
	}
	sdhi_write16(host, SDHI_CLK_CTRL, clk & 0x3ff);
}

static void renesas_sdhi_power(struct renesas_sdhi_host *host, int power)
{
	struct renesas_sdhi_platdata *pdata = host->pdata;

	switch (power) {
	case 1:
		if (pdata->set_pwr)
			pdata->set_pwr(host->pdev, 1);
		if (host->dynamic_clock) {
			pm_runtime_get_sync(&host->pdev->dev);
			sdhi_reset(host);
		}
		break;
	default:
		if (host->dynamic_clock)
			pm_runtime_put_sync(&host->pdev->dev);
		if (pdata->set_pwr)
			pdata->set_pwr(host->pdev, 0);
		break;
	}
}

static void renesas_sdhi_init_sg(
	struct renesas_sdhi_host *host, struct mmc_data *data)
{
	host->sg_len = data->sg_len;
	host->sg_ptr = data->sg;
	host->sg_orig = data->sg;
	host->sg_off = 0;
}

static int renesas_sdhi_next_sg(struct renesas_sdhi_host *host)
{
	host->sg_ptr = sg_next(host->sg_ptr);
	host->sg_off = 0;
	return --host->sg_len;
}

static void renesas_sdhi_pio_irq(struct renesas_sdhi_host *host)
{
	struct mmc_data *data = host->data;
	void *sg_virt;
	unsigned short *buf;
	unsigned int count;

	if (!data) {
		dev_err(&host->pdev->dev, "%s: Spurious PIO IRQ\n", __func__);
		return;
	}

	sg_virt = kmap_atomic(sg_page(host->sg_ptr), KM_BIO_SRC_IRQ) +
			host->sg_ptr->offset;
	buf = (unsigned short *)(sg_virt + host->sg_off);

	count = host->sg_ptr->length - host->sg_off;
	if (count > data->blksz)
		count = data->blksz;

	/* Transfer the data */
	if (data->flags & MMC_DATA_READ)
		sdhi_read16s(host, SDHI_BUF0, buf, count >> 1);
	else
		sdhi_write16s(host, SDHI_BUF0, buf, count >> 1);

	host->sg_off += count;

	kunmap_atomic(sg_virt - host->sg_ptr->offset, KM_BIO_SRC_IRQ);

	if (host->sg_off == host->sg_ptr->length)
		renesas_sdhi_next_sg(host);
}

static void renesas_sdhi_data_done(
	struct renesas_sdhi_host *host, struct mmc_command *cmd)
{
	struct mmc_data *data = host->data;
	int dir;
	u32 val;

	if (data) {
		if (host->dma_tx || host->dma_rx) {
			dir = (host->data->flags & MMC_DATA_READ) ?
				DMA_FROM_DEVICE : DMA_TO_DEVICE;
			dma_unmap_sg(mmc_dev(host->mmc), host->sg_ptr,
						host->sg_len, dir);
		}
		if (!host->data->error)
			host->data->bytes_xfered = data->blocks * data->blksz;
		else
			host->data->bytes_xfered = 0;
	}

	host->data = NULL;
	host->cmd = NULL;
	host->mrq = NULL;
	host->force_pio = false;

	/* Clear interrupt */
	val = sdhi_read32(host, SDHI_INFO) & SDHI_INFO_DETECT;
	sdhi_write32(host, SDHI_INFO, val);

	sdhi_disable_irqs(host,
		SDHI_INFO_BWE | SDHI_INFO_BRE | SDHI_INFO_RW_END);

	clk_disable(host->clk);
	mmc_request_done(host->mmc, cmd->mrq);
}

static void renesas_sdhi_cmd_done(
	struct renesas_sdhi_host *host, struct mmc_command *cmd)
{
	int i, addr;
	u32 val;

	if (cmd->flags & MMC_RSP_PRESENT) {
		for (i = 3, addr = SDHI_RSP ; i >= 0 ; i--, addr += 4)
			cmd->resp[i] = sdhi_read32(host, addr);

		if (cmd->flags &  MMC_RSP_136) {
			cmd->resp[0] = (cmd->resp[0]<<8) | (cmd->resp[1]>>24);
			cmd->resp[1] = (cmd->resp[1]<<8) | (cmd->resp[2]>>24);
			cmd->resp[2] = (cmd->resp[2]<<8) | (cmd->resp[3]>>24);
			cmd->resp[3] <<= 8;
		} else if (cmd->flags & MMC_RSP_R3) {
			cmd->resp[0] = cmd->resp[3];
		}
	}

	if (cmd->flags & MMC_RSP_BUSY) {
		/* wait DATA0 == 1 */
		while (!(sdhi_read32(host, SDHI_INFO) & SDHI_INFO_DAT0))
			;
	}

	if (host->app_mode)
		host->app_mode = 0;
	else if (cmd->opcode == 55)
		host->app_mode = 1;

	if (host->data == NULL) {
		/* Clear interrupt */
		val = sdhi_read32(host, SDHI_INFO) & SDHI_INFO_DETECT;
		sdhi_write32(host, SDHI_INFO, val);
		clk_disable(host->clk);
		host->mrq = NULL;
		host->cmd = NULL;
		mmc_request_done(host->mmc, cmd->mrq);
	} else if (host->data->error) {
		if (!host->force_pio) {
			if (host->data->flags & MMC_DATA_READ)
				dmaengine_terminate_all(host->dma_rx);
			else
				dmaengine_terminate_all(host->dma_tx);
		}
		__cancel_delayed_work(&host->timeout_wq);
		renesas_sdhi_data_done(host, host->cmd);
	} else {
		sdhi_disable_irqs(host, SDHI_INFO_RSP_END);
	}
}

static irqreturn_t renesas_sdhi_irq(int irq, void *dev_id)
{
	struct renesas_sdhi_host *host = dev_id;
	struct mmc_host *mmc = host->mmc;
	u32 status, end_trans = 0, val;
	u16 sdio_status;

	spin_lock(&host->lock);
	clk_enable(host->clk);

	status = sdhi_read32(host, SDHI_INFO) & ~host->info_mask;

	/* sdio */
	if (!status) {
		sdio_status = sdhi_read16(host, SDHI_SDIO_INFO);
		sdhi_write16(host, SDHI_SDIO_INFO, 0);
		if (sdio_status & SDHI_SDIO_IOIRQ) {
			mmc_signal_sdio_irq(mmc);
			goto end;
		}
	}

	if (status & SDHI_INFO_DETECT) {
		sdhi_write32(host, SDHI_INFO, ~SDHI_INFO_DETECT);
		queue_delayed_work(host->work, &host->detect_wq, 0);
	}

	if (status & SDHI_INFO_ALLERR) {
		end_trans = 1;
		if (status & SDHI_INFO_CTO) {
			dev_dbg(&host->pdev->dev, "%s: cmd=%d CMD Timeout\n",
					__func__, host->cmd->opcode);
			host->cmd->error = -ETIMEDOUT;
		} else if (status & SDHI_INFO_CRCERR) {
			dev_dbg(&host->pdev->dev, "%s: cmd=%d CRC error\n",
					__func__, host->cmd->opcode);
			host->cmd->error = -EILSEQ;
		} else {
			dev_dbg(&host->pdev->dev,
					"%s: cmd=%d Other error.(0x%x)\n",
					__func__, host->cmd->opcode, status);
			host->cmd->error = -EILSEQ;
		}
		if (host->data) {
			if (status & SDHI_INFO_DTO) {
				dev_dbg(&host->pdev->dev,
					"%s: cmd=%d DATA Timeout\n",
					__func__, host->cmd->opcode);
				host->data->error = -ETIMEDOUT;
			} else {
				dev_dbg(&host->pdev->dev,
					"%s: cmd=%d Other err.(0x%x)\n",
					__func__, host->cmd->opcode, status);
				host->data->error = -EILSEQ;
			}
		}
	}

	if (status & SDHI_INFO_RSP_END || end_trans) {
		renesas_sdhi_cmd_done(host, host->cmd);
		goto end;
	}

	if (status & (SDHI_INFO_BWE | SDHI_INFO_BRE)) {
		val = sdhi_read32(host, SDHI_INFO) &
				~(SDHI_INFO_BWE | SDHI_INFO_BRE);
		sdhi_write32(host, SDHI_INFO, val);
		renesas_sdhi_pio_irq(host);
		goto end;
	}

	if (status & SDHI_INFO_RW_END) {
		__cancel_delayed_work(&host->timeout_wq);
		renesas_sdhi_data_done(host, host->cmd);
		goto end;
	}

end:
	clk_disable(host->clk);
	spin_unlock(&host->lock);
	return IRQ_HANDLED;
}

static void renesas_sdhi_detect_work(struct work_struct *work)
{
	struct renesas_sdhi_host *host =
		container_of(work, struct renesas_sdhi_host, detect_wq.work);
	struct renesas_sdhi_platdata *pdata = host->pdata;
	u32 status;
	unsigned long flags;

	spin_lock_irqsave(&host->lock, flags);
	clk_enable(host->clk);

	if (pdata->detect_irq) {
		host->connect = pdata->get_cd(host->pdev);
		if (pdata->detect_msec)
			enable_irq(pdata->detect_irq);
	} else {
		status = sdhi_read32(host, SDHI_INFO);
		host->connect = status & SDHI_INFO_CD ? 1 : 0;
	}

	sdhi_reset(host);

	if (!IS_ERR_OR_NULL(host->mrq)) {
		if (host->mrq->cmd)
			host->mrq->cmd->error = -ENOMEDIUM;
		if (host->mrq->data) {
			host->mrq->data->error = -ENOMEDIUM;
			if (host->dma_tx)
				dmaengine_terminate_all(host->dma_tx);
			if (host->dma_rx)
				dmaengine_terminate_all(host->dma_rx);
			__cancel_delayed_work(&host->timeout_wq);
		}
		renesas_sdhi_data_done(host, host->cmd);
	}

	clk_disable(host->clk);
	spin_unlock_irqrestore(&host->lock, flags);

	mmc_detect_change(host->mmc, msecs_to_jiffies(200));
}

static irqreturn_t renesas_sdhi_detect_irq(int irq, void *dev_id)
{
	struct renesas_sdhi_host *host = dev_id;
	struct renesas_sdhi_platdata *pdata = host->pdata;

	spin_lock(&host->lock);

	if (pdata->detect_msec)
		disable_irq_nosync(irq);
	if (pdata->detect_int)
		pdata->detect_int(host->pdev);
	queue_delayed_work(host->work, &host->detect_wq,
			msecs_to_jiffies(pdata->detect_msec));

	spin_unlock(&host->lock);
	return IRQ_HANDLED;
}

static void renesas_sdhi_timeout_work(struct work_struct *work)
{
	struct renesas_sdhi_host *host =
		container_of(work, struct renesas_sdhi_host, timeout_wq.work);
	unsigned long flags;
	int timeout = 0;
	u32 val;

	spin_lock_irqsave(&host->lock, flags);

	if (IS_ERR_OR_NULL(host->mrq)) {
		spin_unlock_irqrestore(&host->lock, flags);
		return;
	}

	sdhi_disable_irqs(host, 0xffffffff);
	spin_unlock_irqrestore(&host->lock, flags);

	dev_err(&host->pdev->dev,
		"timeout waiting for hardware interrupt (CMD%u)\n",
		host->cmd->opcode);

	sdhi_write16(host, SDHI_STOP,
			sdhi_read16(host, SDHI_STOP) | SDHI_STOP_STOP);
	while ((sdhi_read32(host, SDHI_INFO) & SDHI_INFO_DIVEN) == 0) {
		if (timeout++ > 1000) {
			dev_err(&host->pdev->dev,
				"timeout waiting STOP command\n");
			break;
		}
		msleep(1);
	}

	if (host->dma_tx)
		dmaengine_terminate_all(host->dma_tx);
	if (host->dma_rx)
		dmaengine_terminate_all(host->dma_rx);

	host->cmd->error = -ETIMEDOUT;
	host->data->error = -ETIMEDOUT;

	val = sdhi_read32(host, SDHI_INFO) & SDHI_INFO_DETECT;
	sdhi_write32(host, SDHI_INFO, val);

	host->info_mask = 0xffffffff;
	sdhi_enable_irqs(host, SDHI_INFO_ALLERR);
	if (!host->dynamic_clock)
		sdhi_enable_irqs(host, SDHI_INFO_DETECT);

	renesas_sdhi_data_done(host, host->cmd);
}

static bool renesas_sdhi_filter(struct dma_chan *chan, void *arg)
{
	dev_dbg(chan->device->dev, "%s: slave data %p\n", __func__, arg);
	chan->private = arg;
	return true;
}

static void renesas_sdhi_request_dma(struct renesas_sdhi_host *host)
{
	struct renesas_sdhi_platdata *pdata = host->pdata;

	if (!pdata->dma)
		return;

	if (!host->dma_tx && !host->dma_rx) {
		dma_cap_mask_t mask;

		dma_cap_zero(mask);
		dma_cap_set(DMA_SLAVE, mask);

		host->dma_tx = dma_request_channel(mask,
			renesas_sdhi_filter, &pdata->dma->chan_tx);
		dev_dbg(&host->pdev->dev, "%s: TX: got channel %p\n",
				__func__, host->dma_tx);

		if (!host->dma_tx)
			return;

		host->dma_rx = dma_request_channel(mask,
			renesas_sdhi_filter, &pdata->dma->chan_rx);
		dev_dbg(&host->pdev->dev, "%s: RX: got channel %p\n",
					__func__, host->dma_rx);

		if (!host->dma_rx) {
			dma_release_channel(host->dma_tx);
			host->dma_tx = NULL;
			return;
		}
	}
}

static void renesas_sdhi_release_dma(struct renesas_sdhi_host *host)
{
	if (host->dma_tx) {
		dma_release_channel(host->dma_tx);
		host->dma_tx = NULL;
	}
	if (host->dma_rx) {
		dma_release_channel(host->dma_rx);
		host->dma_rx = NULL;
	}
}

static void renesas_sdhi_dma_callback(void *arg)
{
	struct renesas_sdhi_host *host = arg;

	if (!host->data)
		dev_err(&host->pdev->dev, "NULL data in DMA completion!\n");
	else
		sdhi_enable_irqs(host, SDHI_INFO_RW_END);
}

static void renesas_sdhi_config_dma(struct renesas_sdhi_host *host,
				    unsigned int length)
{
	struct renesas_sdhi_platdata *pdata = host->pdata;
	struct dma_slave_config config;
	int burst_size, ret;

	config.src_addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
	config.dst_addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
	if (length % 32 == 0) {
		burst_size = 32;
		config.src_maxburst = 16;
		config.dst_maxburst = 16;
		pdata->set_dma(host->pdev, 32);
	} else if (length % 16 == 0) {
		burst_size = 16;
		config.src_maxburst = 8;
		config.dst_maxburst = 8;
		pdata->set_dma(host->pdev, 16);
	} else {
		burst_size = 2;
		config.src_maxburst = 1;
		config.dst_maxburst = 1;
	}

	if (burst_size == host->burst_size)
		return;

	config.direction = DMA_FROM_DEVICE;
	ret = dmaengine_slave_config(host->dma_rx, &config);

	config.direction = DMA_TO_DEVICE;
	ret |= dmaengine_slave_config(host->dma_tx, &config);

	if (ret) {
		dev_err(&host->pdev->dev,
			"%s(): dmaengine_slave_config error\n", __func__);
		host->burst_size = 2;
		pdata->set_dma(host->pdev, 2);
		return;
	}

	host->burst_size = burst_size;
}

static void renesas_sdhi_start_dma(
	struct renesas_sdhi_host *host, struct mmc_data *data)
{
	struct scatterlist *sg = host->sg_ptr, *sg_tmp;
	int dir, count, i;
	struct dma_async_tx_descriptor *desc = NULL;
	struct dma_chan *chan;
	dma_cookie_t cookie;
	u32 align = host->pdata->dma_alignment - 1;

	if (!host->dma_tx || !host->dma_rx)
		goto force_pio;

	if (sg->length < host->pdata->dma_min_size)
		goto force_pio;

	for_each_sg(sg, sg_tmp, host->sg_len, i) {
		if (sg_tmp->offset & align || sg_tmp->length & align)
			goto force_pio;
	}

	sdhi_dma_enable(host, true);

	if (data->flags & MMC_DATA_READ) {
		dir = DMA_FROM_DEVICE;
		chan = host->dma_rx;
	} else {
		dir = DMA_TO_DEVICE;
		chan = host->dma_tx;
	}

	if (host->pdata->flags & RENESAS_SDHI_DMA_SLAVE_CONFIG)
		renesas_sdhi_config_dma(host, sg->length);

	count = dma_map_sg(chan->device->dev, data->sg, data->sg_len, dir);
	if (count <= 0) {
		dev_err(&host->pdev->dev, "%s(): dma_map_sg error\n", __func__);
		goto force_pio;
	}

	desc = chan->device->device_prep_slave_sg(chan,
			data->sg, count, dir, DMA_CTRL_ACK);
	if (desc) {
		desc->callback = renesas_sdhi_dma_callback;
		desc->callback_param = host;
		cookie = dmaengine_submit(desc);
		if (cookie < 0) {
			dev_err(&host->pdev->dev,
				"%s(): dmaengine_submit error\n", __func__);
			dmaengine_terminate_all(chan);
			goto force_pio;
		}
	} else {
		dev_err(&host->pdev->dev,
			"%s(): device_prep_slave_sg error\n", __func__);
		dmaengine_terminate_all(chan);
		goto force_pio;
	}
	dma_async_issue_pending(chan);
	return;

force_pio:
	host->force_pio = true;
	sdhi_dma_enable(host, false);
	sdhi_enable_irqs(host,
		SDHI_INFO_BWE | SDHI_INFO_BRE | SDHI_INFO_RW_END);
}

static void renesas_sdhi_setup_data(
	struct renesas_sdhi_host *host, struct mmc_data *data)
{
	dev_dbg(&host->pdev->dev, "blksize=%d blknum=%d\n",
					data->blksz, data->blocks);

	host->data = data;

	renesas_sdhi_init_sg(host, data);

	sdhi_write16(host, SDHI_SIZE, data->blksz);
	sdhi_write16(host, SDHI_SECCNT, data->blocks);

	renesas_sdhi_start_dma(host, data);
}

static void renesas_sdhi_start_cmd(struct renesas_sdhi_host *host,
			struct mmc_command *cmd, u16 cmddat)
{
	host->cmd = cmd;

	cmddat |= cmd->opcode;

	switch (mmc_resp_type(cmd)) {
	case MMC_RSP_R1: /* short CRC, OPCODE */
		cmddat |= SDHI_CMD_RSP_R1;
		break;
	case MMC_RSP_R1B:/* short CRC, OPCODE, BUSY */
		cmddat |= SDHI_CMD_RSP_R1B;
		break;
	case MMC_RSP_R2: /* long 136 bit + CRC */
		cmddat |= SDHI_CMD_RSP_R2;
		break;
	case MMC_RSP_R3: /* short */
		cmddat |= SDHI_CMD_RSP_R3;
		break;
	}

	if (host->app_mode)
		cmddat |= SDHI_CMD_ACMD;

	if (host->data) {
		if (host->data->blocks > 1) {
			/*
			 * For SDIO devices, disable automatic CMD12 issuance
			 * function to avoid unnecessary command timeout.
			 */
			if (cmd->opcode == SD_IO_RW_EXTENDED)
				cmddat |= SDHI_CMD_NO_CMD12;
		}
	}

	sdhi_write32(host, SDHI_ARG, cmd->arg);

	dev_dbg(&host->pdev->dev, "CMD %d %x %x %x %x\n",
		cmd->opcode, cmd->arg, cmd->flags, cmd->retries, cmddat);

	/* Send command */
	sdhi_write16(host, SDHI_CMD, cmddat);
}

static void renesas_sdhi_request(struct mmc_host *mmc, struct mmc_request *mrq)
{
	struct renesas_sdhi_host *host = mmc_priv(mmc);
	u16 cmddat = 0;
	u32 val;

	if (host->connect == 0) {
		mrq->cmd->error = -ENOMEDIUM;
		mmc_request_done(mmc, mrq);
		return;
	}

	host->mrq = mrq;

	clk_enable(host->clk);

	/* Clear status */
	val = sdhi_read32(host, SDHI_INFO) & SDHI_INFO_DETECT;
	sdhi_write32(host, SDHI_INFO, val);

	sdhi_enable_irqs(host, SDHI_INFO_RSP_END);

	mrq->cmd->error = 0;

	if (mrq->data) {
		mrq->data->error = 0;
		renesas_sdhi_setup_data(host, mrq->data);
		cmddat |= SDHI_CMD_DATA;
		if (mrq->data->flags & MMC_DATA_READ)
			cmddat |= SDHI_CMD_READ;
		if (mrq->data->blocks > 1) {
			cmddat |= SDHI_CMD_MULTI;
			sdhi_write16(host, SDHI_STOP, SDHI_STOP_MULTI);
		}
		queue_delayed_work(host->work, &host->timeout_wq,
					msecs_to_jiffies(SDHI_TIMEOUT));
	}

	renesas_sdhi_start_cmd(host, mrq->cmd, cmddat);
}

static void renesas_sdhi_set_ios(struct mmc_host *mmc, struct mmc_ios *ios)
{
	struct renesas_sdhi_host *host = mmc_priv(mmc);
	u32 val;

	if (host->power_mode != ios->power_mode &&
			ios->power_mode == MMC_POWER_UP) {
		clk_enable(host->clk);
		renesas_sdhi_power(host, 1);
		clk_disable(host->clk);
		host->power_mode = ios->power_mode;
	}

	if (host->bus_width != ios->bus_width &&
			host->power_mode != MMC_POWER_OFF) {
		clk_enable(host->clk);
		val = sdhi_read16(host, SDHI_OPTION);
		if (ios->bus_width == MMC_BUS_WIDTH_4)
			val &= ~SDHI_OPT_WIDTH1;
		else
			val |= SDHI_OPT_WIDTH1;
		sdhi_write16(host, SDHI_OPTION, val);
		clk_disable(host->clk);
		host->bus_width = host->bus_width;
	}

	if (host->clock != ios->clock &&
			host->power_mode != MMC_POWER_OFF) {
		clk_enable(host->clk);
		renesas_sdhi_set_clock(host, ios->clock);
		clk_disable(host->clk);
		host->clock = ios->clock;
	}

	if (host->power_mode != ios->power_mode &&
			ios->power_mode == MMC_POWER_OFF) {
		clk_enable(host->clk);
		renesas_sdhi_power(host, 0);
		clk_disable(host->clk);
		host->power_mode = ios->power_mode;
	}
}

static int renesas_sdhi_get_ro(struct mmc_host *mmc)
{
	struct renesas_sdhi_host *host = mmc_priv(mmc);
	struct renesas_sdhi_platdata *pdata = host->pdata;
	int ret;

	clk_enable(host->clk);
	ret = !((pdata->flags & RENESAS_SDHI_WP_DISABLE) ||
			(sdhi_read32(host, SDHI_INFO) & SDHI_INFO_WP));
	clk_disable(host->clk);

	return ret;
}

static int renesas_sdhi_get_cd(struct mmc_host *mmc)
{
	struct renesas_sdhi_host *host = mmc_priv(mmc);
	struct renesas_sdhi_platdata *pdata = host->pdata;

	if (pdata->get_cd)
		return pdata->get_cd(host->pdev);
	else
		return -ENOSYS;
}

static void renesas_sdhi_enable_sdio_irq(struct mmc_host *mmc, int enable)
{
	struct renesas_sdhi_host *host = mmc_priv(mmc);
	u32 val;

	if (enable) {
		clk_enable(host->clk);
		val = sdhi_read16(host, SDHI_SDIO_INFO_MASK);
		val &= ~SDHI_SDIO_IOIRQ;
		sdhi_write16(host, SDHI_SDIO_MODE, 0x0001);
		sdhi_write16(host, SDHI_SDIO_INFO_MASK, val);
	} else {
		val = sdhi_read16(host, SDHI_SDIO_INFO_MASK);
		val |= SDHI_SDIO_IOIRQ;
		sdhi_write16(host, SDHI_SDIO_MODE, 0x0000);
		sdhi_write16(host, SDHI_SDIO_INFO_MASK, val);
		clk_disable(host->clk);
	}
}

static const struct mmc_host_ops renesas_sdhi_ops = {
	.request	= renesas_sdhi_request,
	.set_ios	= renesas_sdhi_set_ios,
	.get_ro         = renesas_sdhi_get_ro,
	.get_cd		= renesas_sdhi_get_cd,
	.enable_sdio_irq = renesas_sdhi_enable_sdio_irq,
};

static int __devinit renesas_sdhi_probe(struct platform_device *pdev)
{
	struct mmc_host *mmc;
	struct renesas_sdhi_host *host;
	struct renesas_sdhi_platdata *pdata;
	struct resource *res;
	int i, irq, ret;
	u32 val;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "platform_get_resource error.\n");
		return -EINVAL;
	}

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(&pdev->dev, "platform_get_irq error.\n");
		return -EINVAL;
	}

	if (!pdev->dev.platform_data) {
		dev_err(&pdev->dev, "platform_data error.\n");
		return -EINVAL;
	}
	pdata = pdev->dev.platform_data;

	mmc = mmc_alloc_host(sizeof(struct renesas_sdhi_host), &pdev->dev);
	if (!mmc)
		return -ENOMEM;

	host = mmc_priv(mmc);
	host->mmc = mmc;
	host->pdev = pdev;
	host->pdata = pdata;

	if (!pdata->dma_en_val)
		pdata->dma_en_val = SDHI_DMA_EN;
	if (!pdata->dma_alignment)
		pdata->dma_alignment = 2;
	if (!pdata->dma_min_size)
		pdata->dma_min_size = 8;

	host->clk = clk_get(&pdev->dev, NULL);
	if (IS_ERR(host->clk)) {
		dev_err(&pdev->dev, "cannot get clock\n");
		ret = PTR_ERR(host->clk);
		goto err1;
	}
	host->hclk = clk_get_rate(host->clk);

	/* SD control register space size is 0x100, 0x200 for bus_shift=1 */
	host->bus_shift = resource_size(res) >> 9;

	host->base = ioremap(res->start, resource_size(res));
	if (!host->base) {
		ret = -ENOMEM;
		dev_err(&pdev->dev, "ioremap error.\n");
		goto err2;
	}

	mmc->ops = &renesas_sdhi_ops;
	mmc->caps = MMC_CAP_4_BIT_DATA |
			MMC_CAP_SD_HIGHSPEED | MMC_CAP_MMC_HIGHSPEED;
	if (pdata->caps)
		mmc->caps |= pdata->caps;
	mmc->f_max = host->hclk;
	mmc->f_min = mmc->f_max / 512;
	mmc->max_segs = 128;
	mmc->max_blk_size = 512;
	mmc->max_req_size = PAGE_CACHE_SIZE * mmc->max_segs;
	mmc->max_blk_count = mmc->max_req_size / mmc->max_blk_size;
	mmc->max_seg_size = mmc->max_req_size;
	if (pdata->ocr)
		mmc->ocr_avail = pdata->ocr;
	else
		mmc->ocr_avail = MMC_VDD_32_33 | MMC_VDD_33_34;

	spin_lock_init(&host->lock);
	host->work = create_singlethread_workqueue("sdhi");
	INIT_DELAYED_WORK(&host->detect_wq, renesas_sdhi_detect_work);
	INIT_DELAYED_WORK(&host->timeout_wq, renesas_sdhi_timeout_work);

	if ((pdata->flags & RENESAS_SDHI_DMA_SLAVE_CONFIG) && !pdata->set_dma) {
		ret = -EINVAL;
		goto err3;
	}

	pm_runtime_enable(&pdev->dev);
	ret = pm_runtime_resume(&pdev->dev);
	if (ret < 0)
		goto err3;

	if (pdata->detect_irq) {
		if (!pdata->get_cd) {
			ret = -EINVAL;
			goto err4;
		}
		host->connect = pdata->get_cd(pdev);
		host->dynamic_clock = 1;
	} else if (host->pdata->caps &
			(MMC_CAP_NEEDS_POLL | MMC_CAP_NONREMOVABLE)) {
		host->connect = 1;
		host->dynamic_clock = 1;
	} else {
		pm_runtime_get_noresume(&pdev->dev);
		clk_enable(host->clk);
		sdhi_reset(host);
		val = sdhi_read32(host, SDHI_INFO);
		host->connect = val & SDHI_INFO_CD ? 1 : 0;
		host->dynamic_clock = 0;
	}

	/* irq */
	for (i = 0; i < 3; i++) {
		irq = platform_get_irq(pdev, i);
		if (irq < 0)
			break;
		ret = request_irq(irq, renesas_sdhi_irq,
				0, dev_name(&pdev->dev), host);
		if (ret) {
			dev_err(&pdev->dev, "request_irq error. (irq=%d)\n",
					irq);
			while (i--) {
				irq = platform_get_irq(pdev, i);
				if (irq >= 0)
					free_irq(irq, host);
			}
			goto err4;
		}
	}

	renesas_sdhi_request_dma(host);

	platform_set_drvdata(pdev, host);
	mmc_add_host(mmc);

	if (pdata->detect_irq) {
		/* externel detect irq */
		ret = request_irq(pdata->detect_irq, renesas_sdhi_detect_irq,
				0, dev_name(&pdev->dev), host);
		if (ret) {
			dev_err(&pdev->dev, "request_irq error. (irq=%d)\n",
					pdata->detect_irq);
			goto err5;
		}
		device_init_wakeup(&pdev->dev, 1);
	}

	dev_info(&pdev->dev, "%s base at 0x%08lx clock rate %u MHz\n",
		 mmc_hostname(host->mmc), (unsigned long)res->start,
		 host->hclk / 1000000);

	return 0;

err5:
	mmc_remove_host(host->mmc);
	renesas_sdhi_release_dma(host);
err4:
	pm_runtime_disable(&pdev->dev);
err3:
	if (host->work)
		destroy_workqueue(host->work);
	iounmap(host->base);
err2:
	clk_put(host->clk);
err1:
	mmc_free_host(mmc);
	return ret;
}

static int __devexit renesas_sdhi_remove(struct platform_device *pdev)
{
	struct renesas_sdhi_host *host = platform_get_drvdata(pdev);
	struct renesas_sdhi_platdata *pdata = host->pdata;
	int i, irq;

	mmc_remove_host(host->mmc);

	cancel_delayed_work_sync(&host->detect_wq);
	destroy_workqueue(host->work);

	renesas_sdhi_release_dma(host);

	iounmap(host->base);
	for (i = 0; i < 3; i++) {
		irq = platform_get_irq(pdev, i);
		if (irq >= 0)
			free_irq(irq, host);
	}
	if (pdata->detect_irq)
		free_irq(pdata->detect_irq, host);

	platform_set_drvdata(pdev, NULL);
	if (!host->dynamic_clock) {
		clk_disable(host->clk);
		pm_runtime_put_sync(&pdev->dev);
	}

	mmc_free_host(host->mmc);

	pm_runtime_disable(&pdev->dev);

	return 0;
}

#ifdef CONFIG_PM
int renesas_sdhi_suspend(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct renesas_sdhi_host *host = platform_get_drvdata(pdev);
	int ret = mmc_suspend_host(host->mmc);

	if (!host->dynamic_clock) {
		clk_disable(host->clk);
		pm_runtime_put_sync(dev);
	}

	if (device_may_wakeup(dev))
		enable_irq_wake(host->pdata->detect_irq);

	return ret;
}

int renesas_sdhi_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct renesas_sdhi_host *host = platform_get_drvdata(pdev);
	u32 val;

	if (device_may_wakeup(dev))
		disable_irq_wake(host->pdata->detect_irq);

	if (!host->dynamic_clock) {
		pm_runtime_get_sync(dev);
		clk_enable(host->clk);
		sdhi_reset(host);
		val = sdhi_read32(host, SDHI_INFO);
		host->connect = val & SDHI_INFO_CD ? 1 : 0;
	}

	return mmc_resume_host(host->mmc);
}
#else
#define renesas_sdhi_suspend	NULL
#define renesas_sdhi_resume	NULL
#endif	/* CONFIG_PM */

static const struct dev_pm_ops renesas_sdhi_dev_pm_ops = {
	.suspend	= renesas_sdhi_suspend,
	.resume		= renesas_sdhi_resume,
};

static struct platform_driver renesas_sdhi_driver = {
	.probe		= renesas_sdhi_probe,
	.remove		= __devexit_p(renesas_sdhi_remove),
	.driver		= {
		.name	= "renesas_sdhi",
		.owner	= THIS_MODULE,
		.pm	= &renesas_sdhi_dev_pm_ops,
	},
};

static int __init renesas_sdhi_init(void)
{
	return platform_driver_register(&renesas_sdhi_driver);
}

static void __exit renesas_sdhi_exit(void)
{
	platform_driver_unregister(&renesas_sdhi_driver);
}

module_init(renesas_sdhi_init);
module_exit(renesas_sdhi_exit);

MODULE_DESCRIPTION("Renesas SDHI driver");
MODULE_AUTHOR("Renesas");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:renesas_sdhi");
