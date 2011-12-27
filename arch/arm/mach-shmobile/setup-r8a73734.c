#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/platform_device.h>
#include <linux/serial_sci.h>
#include <linux/sh_timer.h>
#include <mach/hardware.h>
#include <mach/r8a73734.h>

static struct sh_timer_config cmt10_platform_data = {
	.name			= "CMT10",
	.timer_bit		= 0,
	.clockevent_rating	= 125,
	.clocksource_rating	= 125,
};

static struct resource cmt10_resources[] = {
	{
		.name	= "CMT10",
		.start	= 0xe6130000,
		.end	= 0xe6130044,
		.flags	= IORESOURCE_MEM,
	}, {
		.start	= gic_spi(93),
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device cmt10_device = {
	.name		= "sh_cmt1",
	.id		= 0,
	.dev		= {
			.platform_data	= &cmt10_platform_data,
	},
	.resource	= cmt10_resources,
	.num_resources	= ARRAY_SIZE(cmt10_resources),
};

static struct sh_timer_config cmt11_platform_data = {
	.name			= "CMT11",
	.timer_bit		= 1,
	.clockevent_rating	= 125,
	.clocksource_rating	= 125,
};

static struct resource cmt11_resources[] = {
	{
		.name	= "CMT11",
		.start	= 0xe6130100,
		.end	= 0xe6130144,
		.flags	= IORESOURCE_MEM,
	}, {
		.start	= gic_spi(94),
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device cmt11_device = {
	.name		= "sh_cmt1",
	.id		= 1,
	.dev		= {
			.platform_data	= &cmt11_platform_data,
	},
	.resource	= cmt11_resources,
	.num_resources	= ARRAY_SIZE(cmt11_resources),
};

static struct plat_sci_port scif0_platform_data = {
	.mapbase	= 0xe6450000,
	.flags		= UPF_BOOT_AUTOCONF,
	.scscr		= SCSCR_RE | SCSCR_TE,
	.scbrr_algo_id	= SCBRR_ALGO_4,
	.type		= PORT_SCIFA,
	.irqs		= { gic_spi(102), gic_spi(102),
			    gic_spi(102), gic_spi(102) },
};

static struct platform_device scif0_device = {
	.name		= "sh-sci",
	.id		= 0,
	.dev		= {
		.platform_data	= &scif0_platform_data,
	},
};

static struct plat_sci_port scif1_platform_data = {
	.mapbase	= 0xe6c50000,
	.flags		= UPF_BOOT_AUTOCONF,
	.scscr		= SCSCR_RE | SCSCR_TE,
	.scbrr_algo_id	= SCBRR_ALGO_1,
	.type		= PORT_SCIFA,
	.irqs		= { gic_spi(103), gic_spi(103),
			    gic_spi(103), gic_spi(103) },
};

static struct platform_device scif1_device = {
	.name		= "sh-sci",
	.id		= 1,
	.dev		= {
		.platform_data	= &scif1_platform_data,
	},
};

static struct plat_sci_port scif2_platform_data = {
	.mapbase	= 0xe6c60000,
	.flags		= UPF_BOOT_AUTOCONF,
	.scscr		= SCSCR_RE | SCSCR_TE,
	.scbrr_algo_id	= SCBRR_ALGO_1,
	.type		= PORT_SCIFA,
	.irqs		= { gic_spi(104), gic_spi(104),
			    gic_spi(104), gic_spi(104) },
};

static struct platform_device scif2_device = {
	.name		= "sh-sci",
	.id		= 2,
	.dev		= {
		.platform_data	= &scif2_platform_data,
	},
};

static struct plat_sci_port scif3_platform_data = {
	.mapbase	= 0xe6c70000,
	.flags		= UPF_BOOT_AUTOCONF,
	.scscr		= SCSCR_RE | SCSCR_TE,
	.scbrr_algo_id	= SCBRR_ALGO_1,
	.type		= PORT_SCIFA,
	.irqs		= { gic_spi(105), gic_spi(105),
			    gic_spi(105), gic_spi(105) },
};

static struct platform_device scif3_device = {
	.name		= "sh-sci",
	.id		= 3,
	.dev		= {
		.platform_data	= &scif3_platform_data,
	},
};

static struct platform_device *r8a73734_early_devices[] __initdata = {
	&cmt10_device,
	&cmt11_device,
	&scif0_device,
	&scif1_device,
	&scif2_device,
	&scif3_device,
};

void __init r8a73734_add_standard_devices(void)
{
}

void __init r8a73734_add_early_devices(void)
{
	early_platform_add_devices(r8a73734_early_devices,
			ARRAY_SIZE(r8a73734_early_devices));
}
