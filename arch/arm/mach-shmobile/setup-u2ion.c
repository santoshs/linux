#include <asm/sizes.h>
#include <linux/ion.h>
#include <linux/platform_device.h>
#include <linux/memblock.h>
#include <mach/board-u2evm.h>
#include <mach/setup-u2ion.h>

struct ion_platform_data u2evm_ion_data = {
#ifdef CONFIG_ION_R_MOBILE_USE_VIDEO_HEAP
	.nr = 5,
#else
	.nr = 4,
#endif
	.heaps = {
		{
			.type = ION_HEAP_TYPE_SYSTEM,
			.id = ION_HEAP_SYSTEM_ID,
			.name = "system",
		},
		{
			.type = ION_HEAP_TYPE_SYSTEM_CONTIG,
			.id = ION_HEAP_SYSTEM_CONTIG_ID,
			.name = "system-contig",
		},
		{
			.type = ION_HEAP_TYPE_CARVEOUT,
			.id = ION_HEAP_CAMERA_ID,
			.name = "camera",
			.base = ION_HEAP_CAMERA_ADDR,
			.size = ION_HEAP_CAMERA_SIZE,
		},
		{
			.type = ION_HEAP_TYPE_CARVEOUT,
			.id = ION_HEAP_GPU_ID,
			.name = "gpu",
			.base = ION_HEAP_GPU_ADDR,
			.size = ION_HEAP_GPU_SIZE,
		},
#ifdef CONFIG_ION_R_MOBILE_USE_VIDEO_HEAP
		{
			.type = ION_HEAP_TYPE_CARVEOUT,
			.id = ION_HEAP_VIDEO_ID,
			.name = "video",
			.base = ION_HEAP_VIDEO_ADDR,
			.size = ION_HEAP_VIDEO_SIZE,
		},
#endif
	},
};

struct platform_device u2evm_ion_device = {
	.name = "ion-r-mobile",
	.id = -1,
	.dev = {
		.platform_data = &u2evm_ion_data,
	},
};

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
