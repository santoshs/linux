/*
 * Fifo-attached Serial Interface (FSI) support for SH7724
 *
 * Copyright (C) 2012 Renesas Mobile Corporation
 *
 * Copyright (C) 2009 Renesas Solutions Corp.
 * Kuninori Morimoto <morimoto.kuninori@renesas.com>
 *
 * Based on ssi.c
 * Copyright (c) 2007 Manuel Lauss <mano@roarinelk.homelinux.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/list.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/initval.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>
#include <sound/sh_fsi.h>
#include <asm/atomic.h>
#include <asm/cacheflush.h>
#include <sound/soundpath/common_extern.h>

#include <linux/sh_dma.h>
#include <mach/r8a73734.h>
#include <mach/common.h>

#define USE_DMA

#define DO_FMT		0x0000
#define DOFF_CTL	0x0004
#define DOFF_ST		0x0008
#define DI_FMT		0x000C
#define DIFF_CTL	0x0010
#define DIFF_ST		0x0014
#define ACK_MD		0x0018
#define ACK_RV		0x001C
#define DIDT		0x0020
#define DODT		0x0024
#define MUTE_ST		0x0028
#define OUT_DMAC	0x002C
#define IN_DMAC		0x0038
#define REG_END		IN_DMAC

#define CPU_INT_ST	0x01F4
#define CPU_IEMSK	0x01F8
#define CPU_IMSK	0x01FC
#define INT_ST		0x0200
#define IEMSK		0x0204
#define IMSK		0x0208
#define MUTE		0x020C
#define CLK_RST		0x0210
#define SOFT_RST	0x0214
#define FIFO_SZ		0x0218
#define CLK_SEL		0x0220
#ifdef USE_DMA
#define SWAP_SEL	0x0228
#endif
#define HPB_SRST	0x022C
#define FSIDIVA		0x0400
#define FSIDIVB		0x0408

#define MREG_START	CPU_INT_ST
#ifdef USE_DMA
#define MREG_END	FSIDIVB
#else
#define MREG_END	FSIDIVB
#endif

/* DO_FMT */
/* DI_FMT */
#define CR_FMT(param) ((param) << 4)
# define CR_MONO	0x0
# define CR_MONO_D	0x1
# define CR_PCM		0x2
# define CR_I2S		0x3
# define CR_TDM		0x4
# define CR_TDM_D	0x5

/* DOFF_CTL */
/* DIFF_CTL */
#define IRQ_HALF	0x00100000
#define IRQ_1SAMPLE	0x00200000
#define FIFO_CLR	0x00000001

/* DOFF_ST */
#define ERR_OVER	0x00000010
#define ERR_UNDER	0x00000001
#define ST_ERR		(ERR_OVER | ERR_UNDER)

/* ACK_MD */
#define ACKMD_MASK	0x00007000
#define BPFMD_MASK	0x00000700

/* CLK_RST */
#define B_CLK		0x00000010
#define A_CLK		0x00000001

/* INT_ST */
#define INT_B_IN	(1 << 12)
#define INT_B_OUT	(1 << 8)
#define INT_A_IN	(1 << 4)
#define INT_A_OUT	(1 << 0)

/* SOFT_RST */
#define PBSR		(1 << 12) /* Port B Software Reset */
#define PASR		(1 <<  8) /* Port A Software Reset */
#define IR		(1 <<  4) /* Interrupt Reset */
#define FSISR		(1 <<  0) /* Software Reset */

/* FIFO_SZ */
#define OUT_SZ_MASK	0x7
#define BO_SZ_SHIFT	8
#define AO_SZ_SHIFT	0

#define FSI_RATES SNDRV_PCM_RATE_8000_96000

#define FSI_FMTS (SNDRV_PCM_FMTBIT_S24_LE | SNDRV_PCM_FMTBIT_S16_LE)

#define SYSDMA_BASE				0xFE000000
#define SYSDMA_RESOURSE_SIZE	0xA000
#define SYSDMA_SAR_BASE			0x8000
#define SYSDMA_DAR_BASE			0x8004
#define SYSDMA_ADRR_INTERVAL	0x80
/************************************************************************


		struct


************************************************************************/
struct fsi_stream {
	struct snd_pcm_substream *substream;

	int fifo_max;
	int chan;

	int byte_offset;
	int period_len;
	int buffer_len;
	int periods;
#ifdef USE_DMA
	struct scatterlist		sg;
	struct sh_dmae_slave	dma_param;
	struct dma_chan			*dma_chan;
	dma_cookie_t			dma_cookie;
	int	is_play;
#endif
};

struct fsi_priv {
	void __iomem *base;
	struct fsi_master *master;

	struct fsi_stream playback;
	struct fsi_stream capture;
#ifdef USE_DMA
	void __iomem *phys_base;
	unsigned int out_dma;
	unsigned int in_dma;
#endif
};

struct fsi_regs {
	int ver;

	u32 int_st;
	u32 iemsk;
	u32 imsk;
};

struct fsi_clks {
	struct clk *clkgen;
/*	struct clk *shdmac; */
	struct clk *fsi;
};

struct fsi_master {
	void __iomem *base;
	int irq;
	struct fsi_priv fsia;
	struct fsi_priv fsib;
	struct fsi_regs *regs;
	struct sh_fsi_platform_info *info;
	spinlock_t lock;

	struct fsi_clks clks;
};

struct fsi_work {
	struct fsi_master *master;
	struct work_struct work;
};

static bool g_fsi_trigger_start[SNDRV_PCM_STREAM_LAST];
#ifdef USE_DMA
static unsigned int g_old_addr;
#endif
void fsi_set_trigger_stop(struct snd_pcm_substream *substream, bool flag);
void fsi_dma_stop_by_hooks(void);
static struct fsi_priv *fsi_get_priv(struct snd_pcm_substream *substream);
static void fsi_irq_enable(struct fsi_priv *fsi, int is_play);
static void fsi_irq_disable(struct fsi_priv *fsi, int is_play);
#ifdef USE_DMA
static void fsi_dma_callback(void *fsi);
static void fsi_dma_callback_cap(void *fsi);
/* static int fsi_dma_stop(struct fsi_priv *fsi, int is_play); */
static int fsi_dma_exit(struct fsi_priv *fsi, int is_play);
static void __iomem *sysdma_base_addr;
#else
static struct fsi_work *fsi_push_work;
static struct fsi_work *fsi_pop_work;
static struct workqueue_struct *fsi_wq;
#endif
static int pio_is_play;

void fsi_set_trigger_stop(struct snd_pcm_substream *substream, bool flag)
{
	struct fsi_priv *fsi = fsi_get_priv(substream);
	int is_play = substream->stream == SNDRV_PCM_STREAM_PLAYBACK;

	g_fsi_trigger_start[substream->stream] = flag;

	if (flag) {
		fsi_irq_enable(fsi, is_play);
	} else {
		fsi_irq_disable(fsi, is_play);
		if (is_play) {
#ifdef USE_DMA
			/* fsi_dma_stop(fsi, is_play); */
#endif /* USE_DMA */
		}
	}
}
EXPORT_SYMBOL(fsi_set_trigger_stop);

#ifdef USE_DMA
void fsi_dma_stop_by_hooks(void)
{

}
EXPORT_SYMBOL(fsi_dma_stop_by_hooks);
#endif /* USE_DMA */

/* global variable */
callback_function g_fsi_logical;

bool g_slave;
void fsi_set_slave(const bool slave)
{
	g_slave = slave;
}
EXPORT_SYMBOL(fsi_set_slave);


/************************************************************************


		basic read write function


************************************************************************/
static void __fsi_reg_write(u32 reg, u32 data)
{
	/* valid data area is 24bit */
	data &= 0x00ffffff;

	__raw_writel(data, reg);
}

static u32 __fsi_reg_read(u32 reg)
{
	return __raw_readl(reg);
}

static void __fsi_reg_mask_set(u32 reg, u32 mask, u32 data)
{
	/* valid data area is 24bit */
	u32 clear = (mask & 0x00ffffff) | 0xff000000;
	u32 set = data & mask & 0x00ffffff;
	
	sh_modify_register32(reg, clear, set);
}

static void fsi_reg_write(struct fsi_priv *fsi, u32 reg, u32 data)
{
	if (reg > REG_END)
		return;

	__fsi_reg_write((u32)(fsi->base + reg), data);
}

static u32 fsi_reg_read(struct fsi_priv *fsi, u32 reg)
{
	if (reg > REG_END)
		return 0;

	return __fsi_reg_read((u32)(fsi->base + reg));
}

static void fsi_reg_mask_set(struct fsi_priv *fsi, u32 reg, u32 mask, u32 data)
{
	if (reg > REG_END)
		return;

	__fsi_reg_mask_set((u32)(fsi->base + reg), mask, data);
}

static void fsi_master_write(struct fsi_master *master, u32 reg, u32 data)
{
	unsigned long flags;

	if ((reg < MREG_START) ||
	    (reg > MREG_END))
		return;

	spin_lock_irqsave(&master->lock, flags);
	__fsi_reg_write((u32)(master->base + reg), data);
	spin_unlock_irqrestore(&master->lock, flags);
}

static u32 fsi_master_read(struct fsi_master *master, u32 reg)
{
	u32 ret;
	unsigned long flags;

	if ((reg < MREG_START) ||
	    (reg > MREG_END))
		return 0;

	spin_lock_irqsave(&master->lock, flags);
	ret = __fsi_reg_read((u32)(master->base + reg));
	spin_unlock_irqrestore(&master->lock, flags);

	return ret;
}

static void fsi_master_mask_set(struct fsi_master *master,
			       u32 reg, u32 mask, u32 data)
{
	unsigned long flags;

	if ((reg < MREG_START) ||
	    (reg > MREG_END))
		return;

	spin_lock_irqsave(&master->lock, flags);
	__fsi_reg_mask_set((u32)(master->base + reg), mask, data);
	spin_unlock_irqrestore(&master->lock, flags);
}

/************************************************************************


		basic function


************************************************************************/
static struct fsi_master *fsi_get_master(struct fsi_priv *fsi)
{
	return fsi->master;
}

static int fsi_is_port_a(struct fsi_priv *fsi)
{
	return fsi->master->base == fsi->base;
}

static struct snd_soc_dai *fsi_get_dai(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;

	return  rtd->cpu_dai;
}

struct fsi_priv *fsi_get_priv_frm_dai(struct snd_soc_dai *dai)
{
	struct fsi_master *master = snd_soc_dai_get_drvdata(dai);

	if (0 == master->info->always_slave)
		return &master->fsia;
	else
		return &master->fsib;

}
EXPORT_SYMBOL(fsi_get_priv_frm_dai);

static struct fsi_priv *fsi_get_priv(struct snd_pcm_substream *substream)
{
	return fsi_get_priv_frm_dai(fsi_get_dai(substream));
}

static u32 fsi_get_info_flags(struct fsi_priv *fsi)
{
	struct fsi_master *master = fsi_get_master(fsi);

	if (!master->info)
		return 0;

	return master->info->port_flags;
}

static int fsi_is_master_mode(struct fsi_priv *fsi, int is_play)
{
	u32 mode;
	u32 flags = fsi_get_info_flags(fsi);

	mode = is_play ? SH_FSI_OUT_SLAVE_MODE : SH_FSI_IN_SLAVE_MODE;

	/* return
	 * 1 : master mode
	 * 0 : slave mode
	 */

	return (mode & flags) != mode;
}

static u32 fsi_port_ab_io_bit(struct fsi_priv *fsi, int is_play)
{
	int is_porta = fsi_is_port_a(fsi);
	u32 data;

	if (is_porta)
		data = is_play ? (1 << 0) : (1 << 4);
	else
		data = is_play ? (1 << 8) : (1 << 12);

	return data;
}

static inline struct fsi_stream *fsi_get_stream(struct fsi_priv *fsi,
						int is_play)
{
	return is_play ? &fsi->playback : &fsi->capture;
}

static void fsi_stream_push(struct fsi_priv *fsi,
			    int is_play,
			    struct snd_pcm_substream *substream,
			    u32 buffer_len,
			    u32 period_len)
{
	struct fsi_stream *io = fsi_get_stream(fsi, is_play);

	io->substream	= substream;
	io->buffer_len	= buffer_len;
	io->period_len	= period_len;
	io->byte_offset	= 0;
	io->periods	= 0;
}

static void fsi_stream_pop(struct fsi_priv *fsi, int is_play)
{
	struct fsi_stream *io = fsi_get_stream(fsi, is_play);

	io->substream	= NULL;
	io->buffer_len	= 0;
	io->period_len	= 0;
	io->byte_offset	= 0;
	io->periods	= 0;
}

static int fsi_get_fifo_residue(struct fsi_priv *fsi, int is_play)
{
	u32 status;
	u32 reg = is_play ? DOFF_ST : DIFF_ST;
	struct fsi_stream *io = fsi_get_stream(fsi, is_play);
	int residue;

	status = fsi_reg_read(fsi, reg);
	residue = 0x1ff & (status >> 8);
	residue *= io->chan;

	return residue;
}

/************************************************************************


		irq function


************************************************************************/
static void fsi_irq_enable(struct fsi_priv *fsi, int is_play)
{
	u32 data = fsi_port_ab_io_bit(fsi, is_play);
	struct fsi_master *master = fsi_get_master(fsi);

	fsi_master_mask_set(master, master->regs->imsk,  data, data);
	fsi_master_mask_set(master, master->regs->iemsk, data, data);
}

static void fsi_irq_disable(struct fsi_priv *fsi, int is_play)
{
	u32 data = fsi_port_ab_io_bit(fsi, is_play);
	struct fsi_master *master = fsi_get_master(fsi);

	fsi_master_mask_set(master, master->regs->imsk,  data, 0);
	fsi_master_mask_set(master, master->regs->iemsk, data, 0);
}

static u32 fsi_irq_get_status(struct fsi_master *master)
{
	return fsi_master_read(master, master->regs->int_st);
}

static void fsi_irq_clear_status(struct fsi_priv *fsi, int is_play)
{
	u32 data = 0;
	struct fsi_master *master = fsi_get_master(fsi);

	data |= fsi_port_ab_io_bit(fsi, is_play);

	/* clear interrupt factor */
	fsi_master_mask_set(master, master->regs->int_st, data, 0);
}

/************************************************************************


		ctrl function


************************************************************************/
static void fsi_clk_enable(struct fsi_master *master)
{
	/* clk_enable(master->clks.clkgen); */
	/* clk_enable(master->clks.shdmac); */
	clk_enable(master->clks.fsi);
}

static void fsi_clk_disable(struct fsi_master *master)
{
	clk_disable(master->clks.fsi);
	/* clk_disable(master->clks.shdmac); */
	/* clk_disable(master->clks.clkgen); */
}

static void fsi_clk_ctrl(struct fsi_priv *fsi, int enable)
{
	u32 val = fsi_is_port_a(fsi) ? (1 << 0) : (1 << 4);
	struct fsi_master *master = fsi_get_master(fsi);

	if (enable)
		fsi_master_mask_set(master, CLK_RST, val, val);
	else {
		fsi_master_mask_set(master, CLK_RST, val, 0);
		udelay(200); /* > one audio clock == 125us @ 8kHz */
	}
}

static void fsi_fifo_init(struct fsi_priv *fsi,
			  int is_play,
			  struct snd_soc_dai *dai)
{
	struct fsi_master *master = fsi_get_master(fsi);
	struct fsi_stream *io = fsi_get_stream(fsi, is_play);
	u32 ctrl, shift, i;

	/* get on-chip RAM capacity */
	shift = fsi_master_read(master, FIFO_SZ);
	shift >>= fsi_is_port_a(fsi) ? AO_SZ_SHIFT : BO_SZ_SHIFT;
	shift &= OUT_SZ_MASK;
	io->fifo_max = 256 << shift;
	dev_dbg(dai->dev, "fifo = %d words\n", io->fifo_max);

	/*
	 * The maximum number of sample data varies depending
	 * on the number of channels selected for the format.
	 *
	 * FIFOs are used in 4-channel units in 3-channel mode
	 * and in 8-channel units in 5- to 7-channel mode
	 * meaning that more FIFOs than the required size of DPRAM
	 * are used.
	 *
	 * ex) if 256 words of DP-RAM is connected
	 * 1 channel:  256 (256 x 1 = 256)
	 * 2 channels: 128 (128 x 2 = 256)
	 * 3 channels:  64 ( 64 x 3 = 192)
	 * 4 channels:  64 ( 64 x 4 = 256)
	 * 5 channels:  32 ( 32 x 5 = 160)
	 * 6 channels:  32 ( 32 x 6 = 192)
	 * 7 channels:  32 ( 32 x 7 = 224)
	 * 8 channels:  32 ( 32 x 8 = 256)
	 */
	for (i = 1; i < io->chan; i <<= 1)
		io->fifo_max >>= 1;
	dev_dbg(dai->dev, "%d channel %d store\n", io->chan, io->fifo_max);

	ctrl = is_play ? DOFF_CTL : DIFF_CTL;

	/* set interrupt generation factor */
	fsi_reg_write(fsi, ctrl, IRQ_HALF);

	/* clear FIFO */
	fsi_reg_mask_set(fsi, ctrl, FIFO_CLR, FIFO_CLR);

#ifdef USE_DMA
	/* Setting DMA */
	if (is_play && io->substream) {
		/* called by fsi_dma_process() */
		fsi_reg_write(fsi, OUT_DMAC, 0x21);
		fsi_master_write(master, SWAP_SEL, 2);
	} else if (io->substream) {
		/* called by fsi_dma_process() */
		fsi_reg_write(fsi, IN_DMAC, 0x1);
		fsi_master_write(master, SWAP_SEL, 2);
	} else
		fsi_master_write(master, SWAP_SEL, 0);
#endif
}

static void fsi_soft_all_reset(struct fsi_master *master)
{
#if 0
	uint addr;

	addr = (u_long)ioremap_nocache(0xE6150000, 0x100);

	fsi_clk_enable(master);

	iounmap((void *)addr);

	/* change clock to HPB */
	fsi_master_mask_set(master, HPB_SRST, (1 << 16), 0x0);
	udelay(10);
	fsi_master_mask_set(master, HPB_SRST, (1 << 20), 0x0);
	fsi_master_mask_set(master, HPB_SRST, (1 << 20), (1 << 20));

	/* port AB reset */
	fsi_master_mask_set(master, SOFT_RST, PASR | PBSR, 0);
	mdelay(13);

	/* soft reset */
	fsi_master_mask_set(master, SOFT_RST, FSISR, 0);
	fsi_master_mask_set(master, SOFT_RST, FSISR, FSISR);
	mdelay(13);

	fsi_clk_disable(master);
#endif
	return;
}

#ifdef USE_DMA
static bool fsi_dma_filter(struct dma_chan *chan, void *arg)
{
	dev_dbg(chan->device->dev, "%s: slave data %p\n", __func__, arg);

	if (chan->dev->dev_id == 0) {
		chan->private = arg;
		return true;
	} else {
		return false;
	}
}

static int fsi_dma_init(struct fsi_priv *fsi,
	struct snd_pcm_substream *substream, int is_play)
{
	dma_cap_mask_t mask;
	struct sh_dmae_slave *param;
	struct snd_soc_dai *dai;
	struct fsi_stream *io = fsi_get_stream(fsi, is_play);

	param = &io->dma_param;
	param->dma_dev = NULL;

	dai = fsi_get_dai(substream);

	if (is_play == 1 && fsi_is_port_a(fsi) == 1)
		param->slave_id = SHDMA_SLAVE_FSI2A_TX;
	if (is_play == 0 && fsi_is_port_a(fsi) == 1)
		param->slave_id = SHDMA_SLAVE_FSI2A_RX;
	if (is_play == 1 && fsi_is_port_a(fsi) == 0)
		param->slave_id = SHDMA_SLAVE_FSI2B_TX;
	if (is_play == 0 && fsi_is_port_a(fsi) == 0)
		param->slave_id = SHDMA_SLAVE_FSI2B_RX;

	dma_cap_zero(mask);
	dma_cap_set(DMA_SLAVE, mask);

	if (!io->dma_chan)
		io->dma_chan = dma_request_channel(mask, fsi_dma_filter, param);

	if (!io->dma_chan) {
		dev_dbg(dai->dev, "dma_request_channel failure.\n");
		return -ENODEV;
	}
	return 0;
}

#if 0
static int fsi_dma_stop(struct fsi_priv *fsi, int is_play)
{
	struct fsi_stream *io = fsi_get_stream(fsi, is_play);

	if (!io->dma_chan)
		return 0;

	return 0;
}
#endif

static int fsi_dma_exit(struct fsi_priv *fsi, int is_play)
{
	struct fsi_stream *io = fsi_get_stream(fsi, is_play);
	struct snd_soc_dai *dai;
	int dma_direction;

	if (!io->dma_chan)
		return 0;

	io->dma_chan->device->device_control(io->dma_chan,
		DMA_TERMINATE_ALL, 0);

	if (is_play == 1)
		dma_direction = DMA_TO_DEVICE;
	else
		dma_direction = DMA_FROM_DEVICE;

	if (io->substream) {
		dai = fsi_get_dai(io->substream);
		dma_unmap_sg(dai->dev, &io->sg, 1, dma_direction);
	}

	dma_release_channel(io->dma_chan);
	io->dma_chan = NULL;

	if (is_play)
		fsi_reg_write(fsi, OUT_DMAC, 0);
	else
		fsi_reg_write(fsi, IN_DMAC, 0);

	return 0;
}

static int fsi_dma_get_pos(int is_play, struct fsi_priv *fsi)
{
	unsigned int  addr;
	struct fsi_stream *io = fsi_get_stream(fsi, is_play);

	if (is_play) {
		addr = __raw_readl(sysdma_base_addr + SYSDMA_SAR_BASE +
			io->dma_chan->chan_id * SYSDMA_ADRR_INTERVAL);
		return addr;
	} else {
		addr = __raw_readl(sysdma_base_addr + SYSDMA_DAR_BASE +
			io->dma_chan->chan_id * SYSDMA_ADRR_INTERVAL);
		return addr;
	}
}

static int fsi_dma_data_write(struct fsi_priv *fsi,
	struct snd_pcm_substream *substream, int is_play)
{
	struct snd_pcm_runtime *runtime;
	struct snd_soc_dai *dai = fsi_get_dai(substream);
	struct dma_async_tx_descriptor *desc = NULL;
	struct fsi_stream *io = fsi_get_stream(fsi, is_play);
	struct dma_chan *chan = io->dma_chan;
	struct scatterlist *sg;
	unsigned int period_size;
	unsigned int dma_offset;
	dma_addr_t dma_pos;
	int bufsize;
	int ret;
	int dma_direction;
	void *callback;

	if (!chan) {
		dev_dbg(dai->dev, "%s :dma channel err.\n", __func__);
		return -ENODEV;
	}

	runtime = substream->runtime;

	period_size = snd_pcm_lib_period_bytes(substream);
	dma_offset = io->periods * period_size;
	dma_pos = runtime->dma_addr + dma_offset;
	bufsize = period_size;

	if (is_play == 1) {
		dma_direction = DMA_TO_DEVICE;
		callback = fsi_dma_callback;
	} else {
		dma_direction = DMA_FROM_DEVICE;
		callback = fsi_dma_callback_cap;
	}

	if (chan) {
		sg = &io->sg;
		sg_init_table(sg, 1);
		sg_set_page(sg, pfn_to_page(PFN_DOWN(dma_pos)),
			bufsize, offset_in_page(dma_pos));
		ret = dma_map_sg(dai->dev, sg, 1, dma_direction);
		if (0 < ret) {
			desc = chan->device->device_prep_slave_sg(chan, sg, 1,
			dma_direction, (DMA_PREP_INTERRUPT | DMA_CTRL_ACK));
			if (NULL != desc) {
				desc->callback = callback;
				desc->callback_param = fsi;
				io->dma_cookie = dmaengine_submit(desc);
			} else {
				dev_dbg(dai->dev,
				"%s device_prep_slave_sg err.\n", __func__);
			}
		} else {
			dev_dbg(dai->dev,
			"%s dma_map_sg err(%d).\n", __func__, ret);
		}
	}

	io->periods++;
	if (unlikely(io->periods >= runtime->periods))
		io->periods = 0;

	return 0;
}

static int fsi_dma_start(struct fsi_priv *fsi, int is_play)
{
	struct fsi_stream *io = fsi_get_stream(fsi, is_play);

	if (!io->dma_chan)
		return -ENODEV;

	dma_async_issue_pending(io->dma_chan);

	return 0;
}

static int fsi_dma_process(struct fsi_priv *fsi, int startup, int is_play)
{
	struct snd_pcm_substream *substream = NULL;
	struct fsi_stream *io = fsi_get_stream(fsi, is_play);
	struct snd_pcm_runtime *runtime;

	if (!io				||
	    !io->substream		||
	    !io->substream->runtime)
		return -EINVAL;

	substream = io->substream;
	fsi_dma_data_write(fsi, substream, is_play);

	if (startup) {
		runtime = substream->runtime;
		g_old_addr = runtime->dma_addr;
		fsi_dma_start(fsi, is_play);
		fsi_fifo_init(fsi, is_play, NULL);
	} else {
		fsi_dma_start(fsi, is_play);
	}

	return 0;
}

#if 0
static int fsi_dma_process_cap(struct fsi_priv *fsi, int startup)
{
	struct snd_pcm_runtime *runtime;
	struct snd_pcm_substream *substream = NULL;
	struct fsi_stream *io = fsi_get_stream(fsi, 0);
	u32 status;
	int free;
	int fifo_fill;
	int width;
	u8 *start;
	int i, over_period;

	if (!io			||
	    !io->substream		||
	    !io->substream->runtime)
		return -EINVAL;

	over_period	= 0;
	substream	= io->substream;
	runtime		= substream->runtime;

	/* FSI FIFO has limit.
	 * So, this driver can not send periods data at a time
	 */
	if (io->byte_offset >=
	    io->period_len * (io->periods + 1)) {

		over_period = 1;
		io->periods = (io->periods + 1) % runtime->periods;

		if (0 == io->periods)
			io->byte_offset = 0;
	}

	/* get 1 channel data width */
	width = frames_to_bytes(runtime, 1) / io->chan;

	/* get free space for alsa */
	free = (io->buffer_len - io->byte_offset) / width;

	/* get recv size */
	fifo_fill = fsi_get_fifo_residue(fsi, 0);

	if (free < fifo_fill)
		fifo_fill = free;

	start = runtime->dma_area;
	start += io->byte_offset;

	switch (width) {
	case 2:
		for (i = 0; i < fifo_fill; i++)
			*((u16 *)start + i) =
				(u16)(fsi_reg_read(fsi, DIDT) >> 8);
		break;
	case 4:
		for (i = 0; i < fifo_fill; i++)
			*((u32 *)start + i) = fsi_reg_read(fsi, DIDT);
		break;
	default:
		return -EINVAL;
	}

	io->byte_offset += fifo_fill * width;

	status = fsi_reg_read(fsi, DIFF_ST);
	if (!startup) {
		struct snd_soc_dai *dai = fsi_get_dai(substream);

		if (status & ERR_OVER)
			dev_err(dai->dev, "over run\n");
		if (status & ERR_UNDER)
			dev_err(dai->dev, "under run\n");
	}
	fsi_reg_write(fsi, DIFF_ST, 0);

	fsi_irq_enable(fsi, 0);

	if (over_period)
		snd_pcm_period_elapsed(substream);

	return 0;
}
#endif

static int fsi_dma_fsi_int(struct fsi_priv *fsi, int startup)
{
	struct snd_pcm_substream *substream = NULL;
	struct fsi_stream *io = fsi_get_stream(fsi, 1);
	u32 status;

	if (!io			||
	    !io->substream		||
	    !io->substream->runtime)
		return -EINVAL;

	substream = io->substream;

	status = fsi_reg_read(fsi, DOFF_ST);
/*
 *	if (!startup) {
 *		struct snd_soc_dai *dai = fsi_get_dai(substream);
 *
 *		if (status & ERR_OVER)
 *			dev_err(dai->dev, "over run\n");
 *		if (status & ERR_UNDER)
 *			dev_err(dai->dev, "under run\n");
 *	}
*/
	fsi_reg_write(fsi, DOFF_ST, 0);

	return 0;
}

#endif

/* playback interrupt */
static int fsi_data_push(struct fsi_priv *fsi, int startup)
{
	struct snd_pcm_runtime *runtime;
	struct snd_pcm_substream *substream = NULL;
	struct fsi_stream *io = fsi_get_stream(fsi, 1);
	u32 status;
	int send;
	int fifo_free;
	int width;
	u8 *start;
	int i, over_period;

	if (!io			||
	    !io->substream		||
	    !io->substream->runtime) {
		return -EINVAL;
	}

	over_period	= 0;
	substream	= io->substream;
	runtime		= substream->runtime;

	/* FSI FIFO has limit.
	 * So, this driver can not send periods data at a time
	 */
	if (io->byte_offset >=
	    io->period_len * (io->periods + 1)) {

		over_period = 1;
		io->periods = (io->periods + 1) % runtime->periods;

		if (0 == io->periods)
			io->byte_offset = 0;
	}

	/* get 1 channel data width */
	width = frames_to_bytes(runtime, 1) / io->chan;

	/* get send size for alsa */
	send = (io->buffer_len - io->byte_offset) / width;

	/*  get FIFO free size */
	fifo_free = (io->fifo_max * io->chan) - fsi_get_fifo_residue(fsi, 1);

	/* size check */
	if (fifo_free < send)
		send = fifo_free;

	start = runtime->dma_area;
	start += io->byte_offset;

	switch (width) {
	case 2:
		for (i = 0; i < send; i++)
			fsi_reg_write(fsi, DODT,
				      ((u32)*((u16 *)start + i) << 8));
		break;
	case 4:
		for (i = 0; i < send; i++)
			fsi_reg_write(fsi, DODT, *((u32 *)start + i));
		break;
	default:
		return -EINVAL;
	}

	io->byte_offset += send * width;
	status = fsi_reg_read(fsi, DOFF_ST);
/*
 *	if (!startup) {
 *		struct snd_soc_dai *dai = fsi_get_dai(substream);
 *
 *		if (status & ERR_OVER)
 *			dev_err(dai->dev, "over run\n");
 *		if (status & ERR_UNDER)
 *			dev_err(dai->dev, "under run\n");
 *	}
 */
	fsi_reg_write(fsi, DOFF_ST, 0);

	fsi_irq_enable(fsi, 1);

#ifndef USE_DMA
	if (over_period)
		snd_pcm_period_elapsed(substream);

	if (!g_fsi_trigger_start[substream->stream])
		return 0;
	else
		queue_work(fsi_wq, &fsi_push_work->work);
#endif

	return 0;
}

static int fsi_data_pop(struct fsi_priv *fsi, int startup)
{
	struct snd_pcm_runtime *runtime;
	struct snd_pcm_substream *substream = NULL;
	struct fsi_stream *io = fsi_get_stream(fsi, 0);
	u32 status;
	int free;
	int fifo_fill;
	int width;
	u8 *start;
	int i, over_period;

	if (!io			||
	    !io->substream		||
	    !io->substream->runtime    ||
		!io->substream->runtime->dma_area)
		return -EINVAL;

	if (false == g_fsi_trigger_start[SNDRV_PCM_STREAM_CAPTURE]) {
		fsi_irq_disable(fsi, 0);
		return 0;
	}

	over_period	= 0;
	substream	= io->substream;
	runtime		= substream->runtime;

	/* FSI FIFO has limit.
	 * So, this driver can not send periods data at a time
	 */
	if (io->byte_offset >=
	    io->period_len * (io->periods + 1)) {

		over_period = 1;
		io->periods = (io->periods + 1) % runtime->periods;

		if (0 == io->periods)
			io->byte_offset = 0;
	}

	/* get 1 channel data width */
	width = frames_to_bytes(runtime, 1) / io->chan;

	/* get free space for alsa */
	free = (io->buffer_len - io->byte_offset) / width;

	/* get recv size */
	fifo_fill = fsi_get_fifo_residue(fsi, 0);

	if (free < fifo_fill)
		fifo_fill = free;

	start = runtime->dma_area;
	start += io->byte_offset;

	switch (width) {
	case 2:
		for (i = 0; i < fifo_fill; i++)
			*((u16 *)start + i) =
				(u16)(fsi_reg_read(fsi, DIDT) >> 8);
		break;
	case 4:
		for (i = 0; i < fifo_fill; i++)
			*((u32 *)start + i) = fsi_reg_read(fsi, DIDT);
		break;
	default:
		return -EINVAL;
	}

	io->byte_offset += fifo_fill * width;

	status = fsi_reg_read(fsi, DIFF_ST);
/*
 *	if (!startup) {
 *		struct snd_soc_dai *dai = fsi_get_dai(substream);
 *
 *		if (status & ERR_OVER)
 *			dev_err(dai->dev, "over run\n");
 *		if (status & ERR_UNDER)
 *			dev_err(dai->dev, "under run\n");
 *	}
 */
	fsi_reg_write(fsi, DIFF_ST, 0);

	fsi_irq_enable(fsi, 0);

#ifndef USE_DMA
	if (over_period)
		snd_pcm_period_elapsed(substream);

	if (!g_fsi_trigger_start[substream->stream])
		return 0;
	else
		queue_work(fsi_wq, &fsi_pop_work->work);
#endif

	return 0;
}

#ifdef USE_DMA
static void fsi_dma_callback(void *data)
{
	struct fsi_priv *fsi = data;
	struct snd_pcm_substream *substream;
	struct fsi_stream *io = fsi_get_stream(fsi, 1);
	int status;
	struct snd_soc_dai *dai;
	int is_spin;

	substream = io->substream;

	status = dma_async_is_tx_complete(io->dma_chan,
				io->dma_cookie, NULL, NULL);
	if (DMA_SUCCESS != status) {
		dai = fsi_get_dai(substream);
		dev_err(dai->dev,
		"%s:dma status error(%d).\n", __func__, status);
		return;
	}

	fsi_clk_enable(fsi->master);

	if (substream) {
		if (false != g_fsi_trigger_start[substream->stream]) {
			fsi_dma_process(fsi, 0, 1);
			/* fsi_dma_process_cap(fsi, 0); */
			is_spin = spin_is_locked(&substream->self_group.lock);
			if (is_spin == 0)
				snd_pcm_period_elapsed(substream);
		} else {
			/* fsi_dma_stop(fsi, 1); */
		}
	}

	fsi_clk_disable(fsi->master);
}

static void fsi_dma_callback_cap(void *data)
{
	struct fsi_priv *fsi = data;
	struct snd_pcm_substream *substream;
	struct fsi_stream *io = fsi_get_stream(fsi, 0);
	int status;
	struct snd_soc_dai *dai;
	int is_spin;

	substream = io->substream;

	status = dma_async_is_tx_complete(io->dma_chan,
				io->dma_cookie, NULL, NULL);
	if (DMA_SUCCESS != status) {
		dai = fsi_get_dai(substream);
		dev_err(dai->dev, "%s:dma status err(%d).\n", __func__, status);
		return;
	}

	fsi_clk_enable(fsi->master);

	if (substream) {
		if (false != g_fsi_trigger_start[substream->stream]) {
			fsi_dma_process(fsi, 0, 0);
			/* fsi_dma_process_cap(fsi, 0); */
			is_spin = spin_is_locked(&substream->self_group.lock);
			if (is_spin == 0)
				snd_pcm_period_elapsed(substream);
		} else {
			/* fsi_dma_stop(fsi, 1); */
		}
	}

	fsi_clk_disable(fsi->master);
}
#endif

static irqreturn_t fsi_interrupt(int irq, void *data)
{
	struct fsi_master *master = data;
	u32 int_st;

	fsi_clk_enable(master);
	int_st = fsi_irq_get_status(master);

	if (NULL == g_fsi_logical) {
		/* clear irq status */
		fsi_master_mask_set(master, SOFT_RST, IR, 0);
		fsi_master_mask_set(master, SOFT_RST, IR, IR);

/*		if (int_st & INT_A_OUT) { */
		if (pio_is_play == 1) {
#ifdef USE_DMA
			if (master->fsia.playback.substream->runtime->dma_addr)
				fsi_dma_fsi_int(&master->fsia, 0);
			else
#endif
			fsi_data_push(&master->fsia, 0);
		}
/*
 *		if (int_st & INT_B_OUT)
 *			fsi_data_push(&master->fsib, 0);
 *		if (int_st & INT_A_IN)
 */
		if (pio_is_play == 0)
			fsi_data_pop(&master->fsia, 0);
/*
 *		if (int_st & INT_B_IN)
 *			fsi_data_pop(&master->fsib, 0);
 */
		fsi_irq_clear_status(&master->fsia, 0);
		fsi_irq_clear_status(&master->fsia, 1);
		fsi_irq_clear_status(&master->fsib, 0);
		fsi_irq_clear_status(&master->fsib, 1);
	} else {
		g_fsi_logical();
	}

	fsi_clk_disable(master);

	return IRQ_HANDLED;
}

#ifndef USE_DMA
static void __fsi_interrupt_push(struct work_struct *work)
{
	struct fsi_master *master = fsi_push_work->master;

	pio_is_play = 1;
	fsi_interrupt(master->irq, master);
}

static void __fsi_interrupt_pop(struct work_struct *work)
{
	struct fsi_master *master = fsi_pop_work->master;

	pio_is_play = 0;
	fsi_interrupt(master->irq, master);
}
#endif
/************************************************************************


		dai ops


************************************************************************/
static int fsi_dai_startup(struct snd_pcm_substream *substream,
			   struct snd_soc_dai *dai)
{
	struct fsi_priv *fsi = fsi_get_priv(substream);
	struct fsi_master *master = fsi_get_master(fsi);
	const char *msg;
	u32 flags = fsi_get_info_flags(fsi);
	u32 fmt;
	u32 reg;
	u32 data;
	int is_play = (substream->stream == SNDRV_PCM_STREAM_PLAYBACK);
	int is_master;
	int ret = 0;
	struct fsi_stream *io = fsi_get_stream(fsi, is_play);

	fsi_clk_enable(master);

	if (is_play)
		fsi_reg_write(fsi, OUT_DMAC, (0x1 << 4));
	else
		fsi_reg_write(fsi, IN_DMAC, (0x1 << 4));

	/* chip revision check */
	/* for ES2.0 */
	if ((0x10 <= (system_rev & 0xff)) && (false == g_slave)) {
		if(fsi_is_port_a(fsi))
			fsi_master_write(master, FSIDIVA, 0x00020003);
		else
			fsi_master_write(master, FSIDIVB, 0x00020003);
		fsi_reg_write(fsi, ACK_RV, 0x00000100);
		if(0 != is_play)
			fsi_reg_mask_set(fsi, ACK_MD, 0x00001101, 0x00001101);
		else
			fsi_reg_mask_set(fsi, ACK_MD, 0x00001110, 0x00001110);
	/*for ES1.0 */
	} else {
		data = is_play ? (1 << 0) : (1 << 4);
		is_master = fsi_is_master_mode(fsi, is_play);
		fsi_reg_mask_set(fsi, ACK_MD, 7<<12, 2<<12);
		fsi_reg_mask_set(fsi, ACK_MD, 7<<8, 1<<8);
		if (is_master && !master->info->always_slave)
			fsi_reg_mask_set(fsi, ACK_MD, data, data);
		else
			fsi_reg_mask_set(fsi, ACK_MD, data, 0);
		/* clock inversion (ACK_RV) */
		data = 0;
		if (SH_FSI_LRM_INV & flags)
			data |= 1 << 12;
		if (SH_FSI_BRM_INV & flags)
			data |= 1 << 8;
		if (SH_FSI_LRS_INV & flags)
			data |= 1 << 4;
		if (SH_FSI_BRS_INV & flags)
			data |= 1 << 0;
		fsi_reg_write(fsi, ACK_RV, data);
	}

	/* do fmt, di fmt */
	data = 0;
	reg = is_play ? DO_FMT : DI_FMT;
	fmt = is_play ? SH_FSI_GET_OFMT(flags) : SH_FSI_GET_IFMT(flags);

	switch (fmt) {
	case SH_FSI_FMT_MONO:
		msg = "MONO";
		data = CR_FMT(CR_MONO);
		io->chan = 1;
		break;
	case SH_FSI_FMT_MONO_DELAY:
		msg = "MONO Delay";
		data = CR_FMT(CR_MONO_D);
		io->chan = 1;
		break;
	case SH_FSI_FMT_PCM:
		msg = "PCM";
		data = CR_FMT(CR_PCM);
		io->chan = 2;
		break;
	case SH_FSI_FMT_I2S:
		msg = "I2S";
		data = CR_FMT(CR_I2S);
		io->chan = 2;
		break;
	case SH_FSI_FMT_TDM:
		msg = "TDM";
		io->chan = is_play ?
			SH_FSI_GET_CH_O(flags) : SH_FSI_GET_CH_I(flags);
		data = CR_FMT(CR_TDM) | (io->chan - 1);
		break;
	case SH_FSI_FMT_TDM_DELAY:
		msg = "TDM Delay";
		io->chan = is_play ?
			SH_FSI_GET_CH_O(flags) : SH_FSI_GET_CH_I(flags);
		data = CR_FMT(CR_TDM_D) | (io->chan - 1);
		break;
	default:
		dev_err(dai->dev, "unknown format.\n");
		return -EINVAL;
	}
	fsi_reg_write(fsi, reg, data);

	/* irq clear */
	fsi_irq_disable(fsi, is_play);
	fsi_irq_clear_status(fsi, is_play);

	/* fifo init */
	fsi_fifo_init(fsi, is_play, dai);

	fsi_clk_disable(master);

	return ret;
}

static void fsi_dai_shutdown(struct snd_pcm_substream *substream,
			     struct snd_soc_dai *dai)
{
	struct fsi_priv *fsi = fsi_get_priv(substream);
	int is_play = substream->stream == SNDRV_PCM_STREAM_PLAYBACK;

	fsi_clk_enable(fsi->master);

	fsi_irq_disable(fsi, is_play);

	fsi_clk_disable(fsi->master);
}

static int fsi_dai_trigger(struct snd_pcm_substream *substream, int cmd,
			   struct snd_soc_dai *dai)
{
	struct fsi_priv *fsi = fsi_get_priv(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	int is_play = substream->stream == SNDRV_PCM_STREAM_PLAYBACK;
	int ret = 0;

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
		fsi_clk_enable(fsi->master);
		g_fsi_trigger_start[substream->stream] = true;

		fsi_stream_push(fsi, is_play, substream,
				frames_to_bytes(runtime, runtime->buffer_size),
				frames_to_bytes(runtime, runtime->period_size));

#ifdef USE_DMA
		if (runtime->dma_addr) {
			fsi_dma_init(fsi, substream, is_play);
			ret = fsi_dma_process(fsi, 1, is_play);
		}
#else
		ret = is_play ? fsi_data_push(fsi, 1) : fsi_data_pop(fsi, 1);
#endif
		break;
	case SNDRV_PCM_TRIGGER_STOP:
		g_fsi_trigger_start[substream->stream] = false;
		fsi_irq_disable(fsi, is_play);
#ifdef USE_DMA
		/* fsi_dma_stop(fsi, is_play); */
		fsi_dma_exit(fsi, is_play);
#endif
		fsi_stream_pop(fsi, is_play);
		fsi_clk_disable(fsi->master);
		break;
	}

	return ret;
}

static int fsi_dai_hw_params(struct snd_pcm_substream *substream,
			     struct snd_pcm_hw_params *params,
			     struct snd_soc_dai *dai)
{
	struct fsi_priv *fsi = fsi_get_priv(substream);
	struct fsi_master *master = fsi_get_master(fsi);
	int is_play = (substream->stream == SNDRV_PCM_STREAM_PLAYBACK);
	int ret;

	/* clock start */
	fsi_clk_enable(master);

	fsi_clk_ctrl(fsi, 1);

	/* if slave mode, set_rate is not needed */
	if (!fsi_is_master_mode(fsi, is_play))
		return 0;

	/*ret = set_rate(fsi_is_port_a(fsi), params_rate(params));
	if (ret > 0) {
		u32 data = 0;

		switch (ret & SH_FSI_ACKMD_MASK) {
		default:
		case SH_FSI_ACKMD_512:
			data |= (0x0 << 12);
			break;
		case SH_FSI_ACKMD_256:
			data |= (0x1 << 12);
			break;
		case SH_FSI_ACKMD_128:
			data |= (0x2 << 12);
			break;
		case SH_FSI_ACKMD_64:
			data |= (0x3 << 12);
			break;
		case SH_FSI_ACKMD_32:
			if (fsi_ver < 2)
				dev_err(dai->dev, "unsupported ACKMD\n");
			else
				data |= (0x4 << 12);
			break;
		}

		switch (ret & SH_FSI_BPFMD_MASK) {
		default:
		case SH_FSI_BPFMD_32:
			data |= (0x0 << 8);
			break;
		case SH_FSI_BPFMD_64:
			data |= (0x1 << 8);
			break;
		case SH_FSI_BPFMD_128:
			data |= (0x2 << 8);
			break;
		case SH_FSI_BPFMD_256:
			data |= (0x3 << 8);
			break;
		case SH_FSI_BPFMD_512:
			data |= (0x4 << 8);
			break;
		case SH_FSI_BPFMD_16:
			if (fsi_ver < 2)
				dev_err(dai->dev, "unsupported ACKMD\n");
			else
				data |= (0x7 << 8);
			break;
		}

		fsi_reg_mask_set(fsi, ACK_MD, (ACKMD_MASK | BPFMD_MASK) , data);
		udelay(10);
		fsi_clk_ctrl(fsi, 1);
		ret = 0;
	}*/

	fsi_clk_disable(master);

	return ret;
}

static struct snd_soc_dai_ops fsi_dai_ops = {
	.startup	= fsi_dai_startup,
	.shutdown	= fsi_dai_shutdown,
	.trigger	= fsi_dai_trigger,
	.hw_params	= fsi_dai_hw_params,
};

/************************************************************************


		pcm ops


************************************************************************/
static struct snd_pcm_hardware fsi_pcm_hardware = {
	.info =		SNDRV_PCM_INFO_INTERLEAVED	|
			SNDRV_PCM_INFO_MMAP		|
			SNDRV_PCM_INFO_MMAP_VALID	|
			SNDRV_PCM_INFO_PAUSE,
	.formats		= FSI_FMTS,
	.rates			= FSI_RATES,
	.rate_min		= 8000,
	.rate_max		= 192000,
	.channels_min		= 1,
	.channels_max		= 2,
	.buffer_bytes_max	= 64 * 1024,
	.period_bytes_min	= 384,
	.period_bytes_max	= 8192,
	.periods_min		= 2,
	.periods_max		= 8,
	.fifo_size		= 256,
};

static int fsi_pcm_open(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	int ret = 0;

	snd_soc_set_runtime_hwparams(substream, &fsi_pcm_hardware);

	ret = snd_pcm_hw_constraint_integer(runtime,
					    SNDRV_PCM_HW_PARAM_PERIODS);

	return ret;
}

static int fsi_hw_params(struct snd_pcm_substream *substream,
			 struct snd_pcm_hw_params *hw_params)
{
	return snd_pcm_lib_malloc_pages(substream,
					params_buffer_bytes(hw_params));
}

static int fsi_hw_free(struct snd_pcm_substream *substream)
{
	return snd_pcm_lib_free_pages(substream);
}

static snd_pcm_uframes_t fsi_pointer(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fsi_priv *fsi = fsi_get_priv(substream);
	int is_play = (substream->stream == SNDRV_PCM_STREAM_PLAYBACK);
	struct fsi_stream *io = fsi_get_stream(fsi, is_play);
	long location;
#ifdef USE_DMA
	unsigned int clr_size;
	unsigned int location_addr;
	unsigned int addr;
	struct snd_soc_dai *dai = fsi_get_dai(substream);
#endif
	if (!g_fsi_trigger_start[substream->stream])
		return 0;

#ifdef USE_DMA
	if (runtime->dma_addr) {
		if (!io->dma_chan) {
			dev_dbg(dai->dev, "%s :dma channel err.\n", __func__);
			return 0;
		}
		addr = fsi_dma_get_pos(is_play, fsi);
		location = bytes_to_frames(runtime, addr - runtime->dma_addr);
		location_addr = runtime->dma_addr + frames_to_bytes(runtime, location);

		if (is_play && location_addr != g_old_addr) {
			if (location_addr < g_old_addr) {
				clr_size = frames_to_bytes(runtime, runtime->buffer_size) - (g_old_addr - runtime->dma_addr);

				dmac_flush_range((void *)phys_to_virt(g_old_addr),
								 (void *)(phys_to_virt(g_old_addr) + clr_size));
				outer_flush_range((unsigned long)g_old_addr,
								  (unsigned long)(g_old_addr + clr_size));

				memset((void *)phys_to_virt(g_old_addr), '\0', clr_size);

				dmac_flush_range((void *)phys_to_virt(g_old_addr),
								 (void *)(phys_to_virt(g_old_addr) + clr_size));
				outer_flush_range((unsigned long)g_old_addr,
								  (unsigned long)(g_old_addr + clr_size));

				g_old_addr = runtime->dma_addr;
			}

			if (location_addr > g_old_addr) {
				clr_size = location_addr - g_old_addr;

				dmac_flush_range((void *)phys_to_virt(g_old_addr),
								 (void *)(phys_to_virt(g_old_addr) + clr_size));
				outer_flush_range((unsigned long)g_old_addr,
								  (unsigned long)(g_old_addr + clr_size));

				memset((void *)phys_to_virt(g_old_addr), '\0', clr_size);

				dmac_flush_range((void *)phys_to_virt(g_old_addr),
								 (void *)(phys_to_virt(g_old_addr) + clr_size));
				outer_flush_range((unsigned long)g_old_addr,
								  (unsigned long)(g_old_addr + clr_size));

				g_old_addr = location_addr;
			}
		}

		if (location >= runtime->buffer_size)
			location = 0;

		return location;
	}
#endif

	if (is_play)
		location = io->byte_offset;
	else
		location = (io->byte_offset - 1);

	if (location < 0)
		location = 0;

	return bytes_to_frames(runtime, location);
}

static struct snd_pcm_ops fsi_pcm_ops = {
	.open		= fsi_pcm_open,
	.ioctl		= snd_pcm_lib_ioctl,
	.hw_params	= fsi_hw_params,
	.hw_free	= fsi_hw_free,
	.pointer	= fsi_pointer,
};

/************************************************************************


		snd_soc_platform


************************************************************************/
#define PREALLOC_BUFFER		(32 * 1024)
#define PREALLOC_BUFFER_MAX	(32 * 1024)

static void fsi_pcm_free(struct snd_pcm *pcm)
{
	snd_pcm_lib_preallocate_free_for_all(pcm);
}

static int fsi_pcm_new(struct snd_card *card,
		       struct snd_soc_dai *dai,
		       struct snd_pcm *pcm)
{
#ifdef USE_DMA
	if (!card->dev->coherent_dma_mask)
		card->dev->coherent_dma_mask = 0xffffffff;
	return snd_pcm_lib_preallocate_pages_for_all(
		pcm,
		SNDRV_DMA_TYPE_DEV,
		card->dev,
		PREALLOC_BUFFER, PREALLOC_BUFFER_MAX);
#endif
	/*
	 * dont use SNDRV_DMA_TYPE_DEV, since it will oops the SH kernel
	 * in MMAP mode (i.e. aplay -M)
	 */
	return snd_pcm_lib_preallocate_pages_for_all(
		pcm,
		SNDRV_DMA_TYPE_CONTINUOUS,
		snd_dma_continuous_data(GFP_KERNEL),
		PREALLOC_BUFFER, PREALLOC_BUFFER_MAX);
}

/************************************************************************


		alsa struct


************************************************************************/
struct snd_soc_dai_driver fsi_soc_dai[] = {
	{
		.name			= "fsia-dai",
		.playback = {
			.rates		= FSI_RATES,
			.formats	= FSI_FMTS,
			.channels_min	= 1,
			.channels_max	= 8,
		},
		.capture = {
			.rates		= FSI_RATES,
			.formats	= FSI_FMTS,
			.channels_min	= 1,
			.channels_max	= 8,
		},
		.ops = &fsi_dai_ops,
	},
	{
		.name			= "fsib-dai",
		.playback = {
			.rates		= FSI_RATES,
			.formats	= FSI_FMTS,
			.channels_min	= 1,
			.channels_max	= 8,
		},
		.capture = {
			.rates		= FSI_RATES,
			.formats	= FSI_FMTS,
			.channels_min	= 1,
			.channels_max	= 8,
		},
		.ops = &fsi_dai_ops,
	},
};
EXPORT_SYMBOL_GPL(fsi_soc_dai);

struct snd_soc_platform_driver fsi_soc_platform = {
	.ops		= &fsi_pcm_ops,
	.pcm_new	= fsi_pcm_new,
	.pcm_free	= fsi_pcm_free,
};
EXPORT_SYMBOL_GPL(fsi_soc_platform);

/************************************************************************


		platform function


************************************************************************/
static int fsi_probe(struct platform_device *pdev)
{
	struct fsi_master *master;
	const struct platform_device_id	*id_entry;
	struct resource *res;
	unsigned int irq;
	int ret = 0;

	g_fsi_trigger_start[SNDRV_PCM_STREAM_PLAYBACK] = false;
	g_fsi_trigger_start[SNDRV_PCM_STREAM_CAPTURE] = false;

	id_entry = pdev->id_entry;
	if (!id_entry) {
		dev_err(&pdev->dev, "unknown fsi device\n");
		return -ENODEV;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	irq = platform_get_irq(pdev, 0);
	if (!res || (int)irq <= 0) {
		dev_err(&pdev->dev, "Not enough FSI platform resources.\n");
		ret = -ENODEV;
		goto exit;
	}

	master = kzalloc(sizeof(*master), GFP_KERNEL);
	if (!master) {
		dev_err(&pdev->dev, "Could not allocate master\n");
		ret = -ENOMEM;
		goto exit;
	}

	master->base = ioremap_nocache(res->start, resource_size(res));
	if (!master->base) {
		ret = -ENXIO;
		dev_err(&pdev->dev, "Unable to ioremap FSI registers.\n");
		goto exit_kfree;
	}

#ifdef USE_DMA
	sysdma_base_addr = ioremap_nocache(SYSDMA_BASE, SYSDMA_RESOURSE_SIZE);
	if (!sysdma_base_addr) {
		ret = -ENXIO;
		dev_err(&pdev->dev, "Unable to ioremap SYS-DMA registers.\n");
		goto exit_iounmap_fsi;
	}
#endif

	master->irq		= irq;
	master->info		= pdev->dev.platform_data;
	master->fsia.base	= master->base;
	master->fsia.master	= master;
	master->fsib.base	= master->base + 0x40;
	master->fsib.master	= master;
	master->regs		= (struct fsi_regs *)id_entry->driver_data;
#ifdef USE_DMA
	master->fsia.phys_base	= (void *)res->start;
	master->fsib.phys_base	= (void *)res->start + 0x40;
#endif
	spin_lock_init(&master->lock);

	master->clks.clkgen = clk_get(NULL, "clkgen");
	if (IS_ERR(master->clks.clkgen)) {
		dev_err(&pdev->dev, "cannot get CLKGEN clock to enable\n");
		ret = PTR_ERR(master->clks.clkgen);
		goto exit_iounmap;
	}
	master->clks.fsi = clk_get(&pdev->dev, NULL);
	if (IS_ERR(master->clks.fsi)) {
		dev_err(&pdev->dev, "cannot get FSI clock to enable\n");
		ret = PTR_ERR(&pdev->dev);
		goto exit_clk_clkgen;
	}

	dev_set_drvdata(&pdev->dev, master);

#ifndef USE_DMA
	fsi_wq = create_workqueue("fsi_irq_queue");
	if (NULL == fsi_wq) {
		ret = -ENOMEM;
		dev_err(&pdev->dev, "failed to create workqueue\n");
		goto exit_create_workqueue;
	}

	if (0 == pdev->id) {
		fsi_push_work = kzalloc(sizeof(*fsi_push_work), GFP_KERNEL);
		if (NULL == fsi_push_work) {
			ret = -ENOMEM;
			dev_err(&pdev->dev, "failed to kzalloc\n");
			goto exit_push_work;
		}
		INIT_WORK(&fsi_push_work->work, __fsi_interrupt_push);
		fsi_push_work->master = master;

		fsi_pop_work = kzalloc(sizeof(*fsi_pop_work), GFP_KERNEL);
		if (NULL == fsi_pop_work) {
			ret = -ENOMEM;
			dev_err(&pdev->dev, "failed to kzalloc\n");
			goto exit_pop_work;
		}
		INIT_WORK(&fsi_pop_work->work, __fsi_interrupt_pop);
		fsi_pop_work->master = master;
	}
#endif

	ret = snd_soc_register_platform(&pdev->dev, &fsi_soc_platform);
	if (ret < 0) {
		dev_err(&pdev->dev, "cannot snd soc register\n");
		goto exit_reg_plt;
	}
	return snd_soc_register_dais(&pdev->dev, &fsi_soc_dai[pdev->id], 1);

exit_reg_plt:
#ifndef USE_DMA
	if (0 == pdev->id)
		kfree(&fsi_pop_work->work);
exit_pop_work:
	if (0 == pdev->id)
		kfree(&fsi_push_work->work);
exit_push_work:
	destroy_workqueue(fsi_wq);
exit_create_workqueue:
#endif
	clk_put(master->clks.fsi);
exit_clk_clkgen:
	clk_put(master->clks.clkgen);
exit_iounmap:
#ifdef USE_DMA
	iounmap(sysdma_base_addr);
exit_iounmap_fsi:
#endif
	iounmap(master->base);
exit_kfree:
	kfree(master);
	master = NULL;
exit:
	return ret;
}

static int fsi_remove(struct platform_device *pdev)
{
	struct fsi_master *master;

	master = dev_get_drvdata(&pdev->dev);

	snd_soc_unregister_dais(&pdev->dev, 1);
	snd_soc_unregister_platform(&pdev->dev);

#ifndef USE_DMA
	kfree(&fsi_pop_work->work);
	kfree(&fsi_push_work->work);
	if (fsi_wq) {
		destroy_workqueue(fsi_wq);
		fsi_wq = NULL;
	}
#endif
#ifdef USE_DMA
	iounmap(sysdma_base_addr);
	iounmap(master->base);
	kfree(master);
#endif

	clk_put(master->clks.fsi);
	/* clk_put(master->clks.shdmac); */
	/* clk_put(master->clks.clkgen); */

	return 0;
}

/*
 * This does not exactlly suspend/resume, but initialize.
 * And, it is the basic PM strategy of this driver.
 */
static int fsi_runtime_suspend(struct device *dev)
{
	return 0;
}

static int fsi_runtime_resume(struct device *dev)
{
	struct fsi_master *master = dev_get_drvdata(dev);

	fsi_soft_all_reset(master);

	return 0;
}

static struct dev_pm_ops fsi_pm_ops = {
	.runtime_suspend	= fsi_runtime_suspend,
	.runtime_resume		= fsi_runtime_resume,
};

void fsi_set_run_time(void *sndp_fsi_suspend, void *sndp_fsi_resume);

void fsi_set_run_time(void *sndp_fsi_suspend, void *sndp_fsi_resume)
{
	fsi_pm_ops.suspend = sndp_fsi_suspend;
	fsi_pm_ops.resume = sndp_fsi_resume;
}
EXPORT_SYMBOL(fsi_set_run_time);


static struct fsi_regs fsi_regs = {
	.ver	= 1,

	.int_st	= INT_ST,
	.iemsk	= IEMSK,
	.imsk	= IMSK,
};

static struct fsi_regs fsi2_regs = {
	.ver	= 2,

	.int_st	= CPU_INT_ST,
	.iemsk	= CPU_IEMSK,
	.imsk	= CPU_IMSK,
};

static struct platform_device_id fsi_id_table[] = {
	{ "sh_fsi",		(kernel_ulong_t)&fsi_regs },
	{ "sh_fsi2",	(kernel_ulong_t)&fsi2_regs },
	{},
};
MODULE_DEVICE_TABLE(platform, fsi_id_table);

static struct platform_driver fsi_driver = {
	.driver	= {
		.name	= "fsi-pcm-audio",
		.pm	= &fsi_pm_ops,
	},
	.probe		= fsi_probe,
	.remove		= fsi_remove,
	.id_table	= fsi_id_table,
};

static int __init fsi_mobile_init(void)
{
	return platform_driver_register(&fsi_driver);
}

static void __exit fsi_mobile_exit(void)
{
	platform_driver_unregister(&fsi_driver);
}
module_init(fsi_mobile_init);
module_exit(fsi_mobile_exit);

MODULE_LICENSE("GPL v2");
	
