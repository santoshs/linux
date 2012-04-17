/* fsi_ctrl.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __FSI_CTRL_H__
#define __FSI_CTRL_H__

#include <linux/kernel.h>

/*
 *
 * FSI Registers
 *
 */
#define FSI_DO_FMT		(0x0000)	/* Output serial format register */
#define FSI_DOFF_CTL		(0x0004)	/* Output FIFO control register */
#define FSI_DOFF_ST		(0x0008)	/* Output FIFO status register */
#define FSI_DI_FMT		(0x000C)	/* Input serial format register */
#define FSI_DIFF_CTL		(0x0010)	/* Input FIFO control register */
#define FSI_DIFF_ST		(0x0014)	/* Iinput FIFO status register */
#define FSI_ACK_MD		(0x0018)	/* Audio clock control register */
#define FSI_ACK_RV		(0x001C)	/* Clock invert control register */
#define FSI_DIDT		(0x0020)	/* Read data register */
#define FSI_DODT		(0x0024)	/* Write data register */
#define FSI_MUTE_ST		(0x0028)	/* MUTE state register */
#define FSI_OUT_DMAC		(0x002C)	/* DMA control register(Output) */
#define FSI_OUT_SEL		(0x0030)	/* Output control register */
#define FSI_OUT_SPST		(0x0034)	/* Output SPDIF state register */
#define FSI_IN_DMAC		(0x0038)	/* DMA control register(Input) */
#define FSI_IN_SEL		(0x003C)	/* Input control register */
#define FSI_TMR_CTL		(0x01D8)	/* Timer set register */
#define FSI_TMR_CLR		(0x01DC)	/* Timer clear register */
#define FSI_INT_SEL		(0x01E0)	/* DSP interrupt select register */
#define FSI_INT_CLR		(0x01F0)	/* Interrupt clear register */
#define FSI_CPU_INT_ST		(0x01F4)	/* CPU Interrupt State Register */
#define FSI_CPU_IEMSK		(0x01F8)	/* CPU Interrupt Source Mask Set Register */
#define FSI_CPU_IMSK		(0x01FC)	/* CPU Interrupt Signal Mask Set Register */
#define FSI_DSP_INT_ST		(0x0200)	/* DSP Interrupt State Register */
#define FSI_DSP_IEMSK		(0x0204)	/* DSP Interrupt Source Mask Set Register */
#define FSI_DSP_IMSK		(0x0208)	/* DSP Interrupt Signal Mask Set Register */
#define FSI_MUTE		(0x020C)	/* MUTE set register */
#define FSI_ACK_RST		(0x0210)	/* Clock reset register */
#define FSI_SOFT_RST		(0x0214)	/* Software Reset Set Register */
#define FSI_FIFO_SZ		(0x0218)	/* FIFO Size Register */
#define FSI_CLK_SEL		(0x0220)	/* Clock select register */
#define FSI_SWAP_SEL		(0x0228)	/* Swap select register */
#define FSI_HPB_SRST		(0x022C)	/* HPB software reset register */

#define FSI_PORTB_OFFSET	(0x0040)	/* PortA offset to PortB  */


/*
 *
 * STRUCTURE Definitions
 *
 */

/* Stream information from linux kernel */
struct fsi_stream {
	struct snd_pcm_substream *substream;

	int fifo_max_num;

	int buff_offset;
	int buff_len;
	int period_len;
	int period_num;

	int uerr_num;
	int oerr_num;
};

/* FSI information from linux kernel */
struct fsi_priv {
	void __iomem *base;
	struct fsi_master *master;

	struct fsi_stream playback;
	struct fsi_stream capture;

	int chan_num:16;
	int clk_master:1;

	long rate;

	/* for suspend/resume */
	u32 saved_do_fmt;
	u32 saved_di_fmt;
	u32 saved_ckg1;
	u32 saved_ckg2;
	u32 saved_out_sel;
};


/*
 *
 * PROTOTYPE Declarations
 *
 */

/* Voice call setting function */
static void fsi_voicecall(const u_int uiValue);
/* Playback setting function */
static void fsi_playback(const u_int uiValue);
/* Capture setting function */
static void fsi_capture(const u_int uiValue);


#endif /* __FSI_CTRL_H__ */

