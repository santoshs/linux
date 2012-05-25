/*
 * Copyright (C) 2011 Renesas Electronics Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/hardware.h>

u32 get_device_type(void)
{
	return 0;
}

int cpu_mmc_init(bd_t *bis)
{
	int ret = 0;

	/*
	 * In accordance with the Linux kernel, probe / allocate MMCIF
	 * controller first, then SDHI next so that we could allocate
	 * the on-board eMMC device to mmc0.
	 */
#ifdef CONFIG_SH_MMCIF
	ret = mmcif_mmc_init(MMCIF_BASE);
	if (ret)
		return ret;
#endif

#ifdef CONFIG_SH_SDHI
	ret =  sdhi_mmc_init(SDHI0_BASE);
	if (ret)
		return ret;
#endif
	return ret;
}

void reset_cpu(ulong addr)
{
}

#ifdef CONFIG_DISPLAY_CPUINFO
#define CCCR		(HPB_BASE + 0x101C)
int print_cpuinfo(void)
{
	unsigned int cccr, major, minor;

	cccr = readl(CCCR);
	major = ((cccr & 0xf0) >> 4) + 1;
	minor = cccr & 0x0f;

	printf("CPU:   Renesas R-Mobile U2 ES%d.%d\n", major, minor);

	return 0;
}
#endif
