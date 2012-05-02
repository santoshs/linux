/*
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __ASM_SH_MOBILE_RCU_H__
#define __ASM_SH_MOBILE_RCU_H__

#define SH_RCU_FLAG_HSYNC_LOW		(1 << 0) /* Default High if possible */
#define SH_RCU_FLAG_VSYNC_LOW		(1 << 1) /* Default High if possible */

#define SH_RCU_OUTPUT_MEM			0x00	/* Memory output */
#define SH_RCU_OUTPUT_ISP			0x01	/* ISP output (default) */
#define SH_RCU_OUTPUT_MEM_ISP		0x03	/* Memory and ISP output */

struct device;

struct sh_mobile_rcu_info {
	unsigned long flags;
	struct device *csi2_dev;
	char *mod_name;
	char *cmod_name;
};

#endif /* __ASM_SH_MOBILE_RCU_H__ */
