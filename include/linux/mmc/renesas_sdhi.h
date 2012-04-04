/*
 * Renesas SDHI driver header
 *
 * Copyright (C) 2011 Renesas Electronics Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 */

#ifndef RENESAS_SDHI_H
#define RENESAS_SDHI_H

#include <linux/platform_device.h>
#include <linux/sh_dma.h>

/* flags */
#define RENESAS_SDHI_WP_DISABLE		0x01	/* Disable WriteProtect check */
#define RENESAS_SDHI_SDCLK_OFFEN	0x02	/* Enable Card clock auto off */
#define RENESAS_SDHI_SDCLK_DIV1		0x04	/* Enable SDCLK div 1 */
#define RENESAS_SDHI_DMA_SLAVE_CONFIG	0x08	/* Runtime DMA config support */

struct renesas_sdhi_dma {
	struct sh_dmae_slave chan_tx;
	struct sh_dmae_slave chan_rx;
};

struct renesas_sdhi_platdata {
	unsigned long		caps;
	unsigned long		flags;
	u32			ocr;

	/* DMA */
	struct renesas_sdhi_dma	*dma;
	u8			dma_buf_acc32;	/* 0:16bit 1:32bit */
	u16			dma_en_val;	/* default:0x0002 */
	u16			dma_alignment;	/* default:2 */
	u32			dma_min_size;	/* default:8 */

	/* Detection */
	u32			detect_irq;
	u32			detect_msec;
	void (*detect_int)(struct platform_device *pdev);
	int (*get_cd)(struct platform_device *pdev);

	void (*set_pwr)(struct platform_device *pdev, int state);

	void (*set_dma)(struct platform_device *pdev, int size);
};

#endif
