/*
 * Driver for Samsung SR030PC50 VGA Camera
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __SR030PC50_H__
#define __SR030PC50_H__

#include <linux/device.h>

#if defined(CONFIG_SOC_CAMERA_SR030PC50)
int SR030PC50_power(struct device *dev, int power_on);
#else
static inline int SR030PC50_power(struct device *dev, int power_on) { return 0; }
#endif

#endif /* __SR030PC50_H__ */

