/*
 * Renesas SuperH DMA Engine support
 *
 * Copyright (C) 2009 Nobuhiro Iwamatsu <iwamatsu.nobuhiro@renesas.com>
 * Copyright (C) 2009 Renesas Solutions, Inc. All rights reserved.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */
#ifndef __DMA_SHDMA_H
#define __DMA_SHDMA_H

#include <linux/sh_dma.h>
#include <linux/shdma-base.h>
#include <linux/dmaengine.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/dma-direction.h>

/* DMA descriptor control */
enum shdma_desc_status {
	DESC_IDLE,
	DESC_PREPARED,
	DESC_SUBMITTED,
	DESC_COMPLETED, /* completed, have to call callback */
	DESC_WAITING,   /* callback called, waiting for ack / re-submit */
};

#define NR_DESCS_PER_CHANNEL 128

#define to_shdma_chan(c) container_of(c, struct shdma_chan, dma_chan)
#define to_shdma_dev(d) container_of(d, struct shdma_dev, dma_dev)

#define SH_DMAE_MAX_CHANNELS 20
#define SH_DMAE_TCR_MAX 0x00FFFFFF	/* 16MB */

struct device;

struct sh_dmae_chan {
	struct shdma_chan shdma_chan;
	struct sh_dmae_slave_config *config; /* Slave DMA configuration */
	int xmit_shift;			/* log_2(bytes_per_xfer) */
	u32 __iomem *base;
	char dev_id[16];		/* unique name per DMAC of channel */
	int pm_error;
	void __iomem *desc_mem;
	phys_addr_t desc_pmem;
	int desc_mode;
	int no_of_descs;
	enum dma_transfer_direction direction;
	struct scatterlist *sgl;
	unsigned int sg_len;
	dma_addr_t addr;
};

struct sh_dmae_device {
	struct shdma_dev shdma_dev;
	struct sh_dmae_chan *chan[SH_DMAE_MAX_CHANNELS];
	struct sh_dmae_pdata *pdata;
	struct list_head node;
	u32 __iomem *chan_reg;
	u16 __iomem *dmars;
	void __iomem *desc_mem;
	phys_addr_t desc_pmem;
	unsigned int chcr_offset;
	u32 chcr_ie_bit;
};


struct sh_dmae_desc {
	struct sh_dmae_regs hw;
	struct shdma_desc shdma_desc;
};

#define to_sh_chan(chan) container_of(chan, struct sh_dmae_chan, shdma_chan)
#define to_sh_desc(lh) container_of(lh, struct sh_desc, node)
#define tx_to_sh_desc(tx) container_of(tx, struct sh_desc, async_tx)
#define to_sh_dev(chan) container_of(chan->shdma_chan.dma_chan.device,\
				     struct sh_dmae_device, shdma_dev.dma_dev)
/* TODO - ### Check, is this the right place ?? */
void dmae_rpt_halt(struct shdma_chan *schan);
int dmae_rpt_init_reg(struct shdma_chan *schan);
void dmae_rpt_start(struct shdma_chan *schan);

struct dma_async_tx_descriptor *sh_dmae_rpt_prep_sg(
	struct shdma_chan *schan,
	struct scatterlist *sgl, unsigned int sg_len, dma_addr_t *addr,
	enum dma_data_direction direction, unsigned long flags);
void set_desc_mode(struct shdma_chan *schan, bool val);
int is_desc_mode(struct shdma_chan *schan);

#endif	/* __DMA_SHDMA_H */
