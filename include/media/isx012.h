/*
 * Driver for SONY ISX012 CMOS Image Sensor
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __ISX012_H__
#define __ISX012_H__

#include <linux/device.h>

int ISX012_power0(struct device *dev, int power_on);

#endif /* __ISX012_H__ */
