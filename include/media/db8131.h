/*
 * Driver for Samsung DB8131 1.3M VT Camera
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __DB8131_H__
#define __DB8131_H__

#include <linux/device.h>

int DB8131_power(struct device *dev, int power_on);

#endif /* __DB8131_H__ */

