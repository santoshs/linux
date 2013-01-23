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

/* Voltage */
#define RENESAS_SDHI_POWER_OFF		0x00
#define RENESAS_SDHI_POWER_ON		0x01
#define RENESAS_SDHI_SIGNAL_V330	0x10
#define RENESAS_SDHI_SIGNAL_V180	0x11

struct renesas_sdhi_dma {
	struct sh_dmae_slave chan_tx;
	struct sh_dmae_slave chan_rx;
};

#define RENESAS_SDHI_PULL_OFF	0
#define RENESAS_SDHI_PULL_DOWN	1
#define RENESAS_SDHI_PULL_UP	2

#define RENESAS_SDHI_DIRECTION_NOT_SET	(-1)
#define RENESAS_SDHI_DIRECTION_NONE		0
#define RENESAS_SDHI_DIRECTION_OUTPUT	1
#define RENESAS_SDHI_DIRECTION_INPUT	2

#define RENESAS_SDHI_OUT_LEVEL_NOT_SET	(-1)
#define RENESAS_SDHI_OUT_LEVEL_HI		1
#define RENESAS_SDHI_OUT_LEVEL_LOW		0

struct renesas_sdhi_gpio_setting {
	u32	port_mux;	/* pin function */
	s32	pull;
	s32 direction;
	s32 out_level;	/* It become enable only when direction is output. */
};

struct renesas_sdhi_gpio_setting_info {
	u32	flag;		/* 0:nochange 1:change */
	u32	port;		/* gpio port num */
	struct renesas_sdhi_gpio_setting active;
	struct renesas_sdhi_gpio_setting deactive;
};

struct renesas_sdhi_platdata {
	unsigned long		caps;
	mmc_pm_flag_t		pm_caps;
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

	/* Gpio setting */
	u32			port_cnt;
	struct renesas_sdhi_gpio_setting_info	*gpio_setting_info;

	void (*detect_int)(struct platform_device *pdev);
	int (*get_cd)(struct platform_device *pdev);

	void (*set_pwr)(struct platform_device *pdev, int state);

	void (*set_dma)(struct platform_device *pdev, int size);
};

#endif
