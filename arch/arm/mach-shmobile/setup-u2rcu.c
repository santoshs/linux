#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>

#include <mach/setup-u2camera.h>
#include <mach/setup-u2rcu.h>
#include <mach/irqs.h>

#include <mach/setup-u2csi2.h>
#include <media/sh_mobile_rcu.h>

struct sh_mobile_rcu_info sh_mobile_rcu0_info = {
	.flags		= 0,
	.csi2_dev	= &csi20_device.dev,
	.mod_name	= "rcu0",
	.led		= main_cam_led,
};

struct resource rcu0_resources[] = {
	[0] = {
		.name	= "RCU0",
		.start	= 0xfe910000,
		.end	= 0xfe91022b,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= gic_spi(83),
		.flags	= IORESOURCE_IRQ,
	},
	[2] = {
		/* place holder for contiguous memory */
	},
};

struct resource rcu0_resources_es1[] = {
	[0] = {
		.name	= "RCU0",
		.start	= 0xfe910000,
		.end	= 0xfe91022b,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= intcs_evt2irq(0x1de0),
		.flags	= IORESOURCE_IRQ,
	},
	[2] = {
		/* place holder for contiguous memory */
	},
};

struct platform_device rcu0_device = {
	.name			= "sh_mobile_rcu",
	.id				= 0, /* "rcu0" clock */
	.num_resources	= ARRAY_SIZE(rcu0_resources),
	.resource		= rcu0_resources, /* ES1 is RCU0 connect */
	.dev = {
		.platform_data	= &sh_mobile_rcu0_info,
	},
};

struct sh_mobile_rcu_info sh_mobile_rcu1_info = {
	.flags		= 0,
	.csi2_dev	= &csi21_device.dev,
	.mod_name	= "rcu1",
};

struct resource rcu1_resources[] = {
	[0] = {
		.name	= "RCU1",
		.start	= 0xfe914000,
		.end	= 0xfe91422b,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= gic_spi(84),
		.flags	= IORESOURCE_IRQ,
	},
	[2] = {
		/* place holder for contiguous memory */
	},
};

struct resource rcu1_resources_es1[] = {
	[0] = {
		.name	= "RCU1",
		.start	= 0xfe910000,
		.end	= 0xfe91022b,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= intcs_evt2irq(0x1de0),
		.flags	= IORESOURCE_IRQ,
	},
	[2] = {
		/* place holder for contiguous memory */
	},
};

struct platform_device rcu1_device = {
	.name			= "sh_mobile_rcu",
	.id				= 1, /* "rcu1" clock */
	.num_resources	= ARRAY_SIZE(rcu1_resources),
	.resource		= rcu1_resources, /* ES1 is RCU0 connect */
	.dev	= {
		.platform_data	= &sh_mobile_rcu1_info,
	},
};

int rcu1_resources_es1_size(void)
{
	return ARRAY_SIZE(rcu1_resources_es1);
}
