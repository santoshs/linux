/*
 * arch/arm/mach-shmobile/setup-u2ion.c
 *
 * Copyright (C) 2013 Renesas Mobile Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#include <asm/sizes.h>
#include <linux/ion.h>
#include <linux/platform_device.h>
#include <linux/memblock.h>
#include <mach/setup-u2ion.h>



struct ion_platform_heap u2_ion_heap[] = {
	[0] = {
			.type = ION_HEAP_TYPE_SYSTEM,
			.id = ION_HEAP_SYSTEM_ID,
			.name = "system",
		},
	[1] = {
			.type = ION_HEAP_TYPE_SYSTEM_CONTIG,
			.id = ION_HEAP_SYSTEM_CONTIG_ID,
			.name = "system-contig",
		},
#ifdef CONFIG_ION_R_MOBILE_USE_VIDEO_HEAP
	[2] = {
			.type = ION_HEAP_TYPE_CARVEOUT,
			.id = ION_HEAP_VIDEO_ID,
			.name = "video",
			.base = 0x4AE00000,
			.size = (SZ_16M + SZ_2M),
		},
#endif
};

struct ion_platform_data u2evm_ion_data = {
	.nr = ARRAY_SIZE(u2_ion_heap),
	.heaps = u2_ion_heap,
};

struct platform_device u2evm_ion_device = {
	.name = "ion-r-mobile",
	.id = -1,
	.dev = {
		.platform_data = &u2evm_ion_data,
	},
};

void __init u2_add_ion_device(void)
{
	int ret = platform_device_register(&u2evm_ion_device);
	if (ret)
		pr_err("%s: failed to register ion device %d\n", __func__, ret);
}

int u2evm_ion_adjust(void)
{
	int i;
	int ret;

	for (i = 0; i < u2evm_ion_data.nr; i++) {
		if (u2evm_ion_data.heaps[i].type == ION_HEAP_TYPE_CARVEOUT) {
			ret = memblock_remove(u2evm_ion_data.heaps[i].base,
					      u2evm_ion_data.heaps[i].size);
			if (ret)
				pr_err("memblock remove of %x@%lx failed\n",
				       u2evm_ion_data.heaps[i].size,
				       u2evm_ion_data.heaps[i].base);
		}
	}

	return 0;
}
