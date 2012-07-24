/*
 * r8a7373 processor support
 *
 * Copyright (C) 2012  Renesas Electronics Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/platform_data/rmobile_hwsem.h>
#include <linux/platform_device.h>
#include <linux/serial_sci.h>
#include <linux/i2c/i2c-sh_mobile.h>
#include <linux/clocksource.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <asm/delay.h>
#include <asm/system_info.h>
#include <asm/hardware/cache-l2x0.h>
#include <asm/mach/map.h>
#include <asm/mach/time.h>
#include <asm/sched_clock.h>
#include <mach/cmt.h>
#include <mach/common.h>
#include <mach/irqs.h>
#include <mach/r8a7373.h>
#include <mach/serial.h>

static struct map_desc r8a7373_io_desc[] __initdata = {
	{
		.virtual	= 0xe6000000,
		.pfn		= __phys_to_pfn(0xe6000000),
		.length		= SZ_16M,
		.type		= MT_DEVICE
	},
	{
		.virtual	= 0xf0000000,
		.pfn		= __phys_to_pfn(0xf0000000),
		.length		= SZ_2M,
		.type		= MT_DEVICE
	},
};

void __init r8a7373_map_io(void)
{
	iotable_init(r8a7373_io_desc, ARRAY_SIZE(r8a7373_io_desc));
}

/* SCIFA0 */
static struct plat_sci_port scif0_platform_data = {
	.mapbase	= 0xe6450000,
	.flags		= UPF_BOOT_AUTOCONF | UPF_IOREMAP,
	.scscr		= SCSCR_RE | SCSCR_TE,
	.scbrr_algo_id	= SCBRR_ALGO_4,
	.type		= PORT_SCIFA,
	.irqs		= { gic_spi(102), gic_spi(102),
			    gic_spi(102), gic_spi(102) },
	.ops		= &shmobile_sci_port_ops,
};

static struct platform_device scif0_device = {
	.name		= "sh-sci",
	.id		= 0,
	.dev		= {
		.platform_data	= &scif0_platform_data,
	},
};

/* SCIFA1 */
static struct plat_sci_port scif1_platform_data = {
	.mapbase	= 0xe6c50000,
	.flags		= UPF_BOOT_AUTOCONF | UPF_IOREMAP,
	.scscr		= SCSCR_RE | SCSCR_TE,
	.scbrr_algo_id	= SCBRR_ALGO_4,
	.type		= PORT_SCIFA,
	.irqs		= { gic_spi(103), gic_spi(103),
			    gic_spi(103), gic_spi(103) },
	.ops		= &shmobile_sci_port_ops,
};

static struct platform_device scif1_device = {
	.name		= "sh-sci",
	.id		= 1,
	.dev		= {
		.platform_data	= &scif1_platform_data,
	},
};

/* SCIFA2 */
static struct plat_sci_port scif2_platform_data = {
	.mapbase	= 0xe6c60000,
	.flags		= UPF_BOOT_AUTOCONF | UPF_IOREMAP,
	.scscr		= SCSCR_RE | SCSCR_TE,
	.scbrr_algo_id	= SCBRR_ALGO_4,
	.type		= PORT_SCIFA,
	.irqs		= { gic_spi(104), gic_spi(104),
			    gic_spi(104), gic_spi(104) },
	.ops		= &shmobile_sci_port_ops,
};

static struct platform_device scif2_device = {
	.name		= "sh-sci",
	.id		= 2,
	.dev		= {
		.platform_data	= &scif2_platform_data,
	},
};

/* SCIFA3 */
static struct plat_sci_port scif3_platform_data = {
	.mapbase	= 0xe6c70000,
	.flags		= UPF_BOOT_AUTOCONF | UPF_IOREMAP,
	.scscr		= SCSCR_RE | SCSCR_TE,
	.scbrr_algo_id	= SCBRR_ALGO_4,
	.type		= PORT_SCIFA,
	.irqs		= { gic_spi(105), gic_spi(105),
			    gic_spi(105), gic_spi(105) },
	.ops		= &shmobile_sci_port_ops,
};

static struct platform_device scif3_device = {
	.name		= "sh-sci",
	.id		= 3,
	.dev		= {
		.platform_data	= &scif3_platform_data,
	},
};

/* SCIFB0 */
static struct plat_sci_port scif4_platform_data = {
	.mapbase	= 0xe6c20000,
	.flags		= UPF_BOOT_AUTOCONF | UPF_IOREMAP,
	.scscr		= SCSCR_RE | SCSCR_TE,
	.scbrr_algo_id	= SCBRR_ALGO_4,
	.type		= PORT_SCIFB,
	.irqs		= { gic_spi(107), gic_spi(107),
			    gic_spi(107), gic_spi(107) },
	.ops		= &shmobile_sci_port_ops,
};

static struct platform_device scif4_device = {
	.name		= "sh-sci",
	.id		= 4,
	.dev		= {
		.platform_data	= &scif4_platform_data,
	},
};

/* SCIFB1 */
static struct plat_sci_port scif5_platform_data = {
	.mapbase	= 0xe6c30000,
	.flags		= UPF_BOOT_AUTOCONF | UPF_IOREMAP,
	.scscr		= SCSCR_RE | SCSCR_TE,
	.scbrr_algo_id	= SCBRR_ALGO_4,
	.type		= PORT_SCIFB,
	.irqs		= { gic_spi(108), gic_spi(108),
			    gic_spi(108), gic_spi(108) },
	.ops		= &shmobile_sci_port_ops,
};

static struct platform_device scif5_device = {
	.name		= "sh-sci",
	.id		= 5,
	.dev		= {
		.platform_data	= &scif5_platform_data,
	},
};

/* SCIFB2 */
static struct plat_sci_port scif6_platform_data = {
	.mapbase	= 0xe6ce0000,
	.flags		= UPF_BOOT_AUTOCONF | UPF_IOREMAP,
	.scscr		= SCSCR_RE | SCSCR_TE,
	.scbrr_algo_id	= SCBRR_ALGO_4,
	.type		= PORT_SCIFB,
	.irqs		= { gic_spi(116), gic_spi(116),
			    gic_spi(116), gic_spi(116) },
	.ops		= &shmobile_sci_port_ops,
};

static struct platform_device scif6_device = {
	.name		= "sh-sci",
	.id		= 6,
	.dev		= {
		.platform_data	= &scif6_platform_data,
	},
};

/* SCIFB3 */
static struct plat_sci_port scif7_platform_data = {
	.mapbase	= 0xe6470000,
	.flags		= UPF_BOOT_AUTOCONF | UPF_IOREMAP,
	.scscr		= SCSCR_RE | SCSCR_TE,
	.scbrr_algo_id	= SCBRR_ALGO_4,
	.type		= PORT_SCIFB,
	.irqs		= { gic_spi(117), gic_spi(117),
			    gic_spi(117), gic_spi(117) },
	.ops		= &shmobile_sci_port_ops,
};

static struct platform_device scif7_device = {
	.name		= "sh-sci",
	.id		= 7,
	.dev		= {
		.platform_data	= &scif7_platform_data,
	},
};

/* IIC0 */
static struct i2c_sh_mobile_platform_data i2c0_platform_data = {
	.bus_speed	= 400000,
};

static struct resource i2c0_resources[] = {
	[0] = {
		.name	= "IIC0",
		.start	= 0xe6820000,
		.end	= 0xe6820425 - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= gic_spi(184),
		.end	= gic_spi(184),
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device i2c0_device = {
	.name		= "i2c-sh_mobile",
	.id		= 0,
	.resource	= i2c0_resources,
	.num_resources	= ARRAY_SIZE(i2c0_resources),
	.dev		= {
		.platform_data	= &i2c0_platform_data,
	},
};

/* IIC1 */
static struct i2c_sh_mobile_platform_data i2c1_platform_data = {
	.bus_speed	= 400000,
};

static struct resource i2c1_resources[] = {
	[0] = {
		.name	= "IIC1",
		.start	= 0xe6822000,
		.end	= 0xe6822425 - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= gic_spi(185),
		.end	= gic_spi(185),
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device i2c1_device = {
	.name		= "i2c-sh_mobile",
	.id		= 1,
	.resource	= i2c1_resources,
	.num_resources	= ARRAY_SIZE(i2c1_resources),
	.dev		= {
		.platform_data	= &i2c1_platform_data,
	},
};

/* IIC2 */
static struct i2c_sh_mobile_platform_data i2c2_platform_data = {
	.bus_speed	= 400000,
};

static struct resource i2c2_resources[] = {
	[0] = {
		.name	= "IIC2",
		.start	= 0xe6824000,
		.end	= 0xe6824425 - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= gic_spi(186),
		.end	= gic_spi(186),
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device i2c2_device = {
	.name		= "i2c-sh_mobile",
	.id		= 2,
	.resource	= i2c2_resources,
	.num_resources	= ARRAY_SIZE(i2c2_resources),
	.dev		= {
		.platform_data	= &i2c2_platform_data,
	},
};

/* IIC3 */
static struct i2c_sh_mobile_platform_data i2c3_platform_data = {
	.bus_speed	= 400000,
};

static struct resource i2c3_resources[] = {
	[0] = {
		.name	= "IIC3",
		.start	= 0xe6826000,
		.end	= 0xe6826425 - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= gic_spi(187),
		.end	= gic_spi(187),
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device i2c3_device = {
	.name		= "i2c-sh_mobile",
	.id		= 3,
	.resource	= i2c3_resources,
	.num_resources	= ARRAY_SIZE(i2c3_resources),
	.dev		= {
		.platform_data	= &i2c3_platform_data,
	},
};

/* IIC0H */
static struct i2c_sh_mobile_platform_data i2c4_platform_data = {
	.bus_speed	= 400000,
};

static struct resource i2c4_resources[] = {
	[0] = {
		.name	= "IIC0H",
		.start	= 0xe6828000,
		.end	= 0xe6828425 - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= gic_spi(188),
		.end	= gic_spi(188),
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device i2c4_device = {
	.name		= "i2c-sh_mobile",
	.id		= 4,
	.resource	= i2c4_resources,
	.num_resources	= ARRAY_SIZE(i2c4_resources),
	.dev		= {
		.platform_data	= &i2c4_platform_data,
	},
};

/* IIC1H */
static struct i2c_sh_mobile_platform_data i2c5_platform_data = {
	.bus_speed	= 400000,
};

static struct resource i2c5_resources[] = {
	[0] = {
		.name	= "IIC1H",
		.start	= 0xe682a000,
		.end	= 0xe682a425 - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= gic_spi(189),
		.end	= gic_spi(189),
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device i2c5_device = {
	.name		= "i2c-sh_mobile",
	.id		= 5,
	.resource	= i2c5_resources,
	.num_resources	= ARRAY_SIZE(i2c5_resources),
	.dev		= {
		.platform_data	= &i2c5_platform_data,
	},
};

/* IIC2H */
static struct i2c_sh_mobile_platform_data i2c6_platform_data = {
	.bus_speed	= 400000,
};

static struct resource i2c6_resources[] = {
	[0] = {
		.name	= "IIC2H",
		.start	= 0xe682c000,
		.end	= 0xe682c425 - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= gic_spi(128),
		.end	= gic_spi(128),
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device i2c6_device = {
	.name		= "i2c-sh_mobile",
	.id		= 6,
	.resource	= i2c6_resources,
	.num_resources	= ARRAY_SIZE(i2c6_resources),
	.dev		= {
		.platform_data	= &i2c6_platform_data,
	},
};

/* IIC3H */
static struct i2c_sh_mobile_platform_data i2c7_platform_data = {
	.bus_speed	= 400000,
};

static struct resource i2c7_resources[] = {
	[0] = {
		.name	= "IIC3H",
		.start	= 0xe682e000,
		.end	= 0xe682e425 - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= gic_spi(181),
		.end	= gic_spi(181),
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device i2c7_device = {
	.name		= "i2c-sh_mobile",
	.id		= 7,
	.resource	= i2c7_resources,
	.num_resources	= ARRAY_SIZE(i2c7_resources),
	.dev		= {
		.platform_data	= &i2c7_platform_data,
	},
};

/* SYS-DMAC */

/* Transmit sizes and respective CHCR register values */
enum {
	XMIT_SZ_8BIT		= 0,
	XMIT_SZ_16BIT		= 1,
	XMIT_SZ_32BIT		= 2,
	XMIT_SZ_64BIT		= 7,
	XMIT_SZ_128BIT		= 3,
	XMIT_SZ_256BIT		= 4,
	XMIT_SZ_512BIT		= 5,
};

/* log2(size / 8) - used to calculate number of transfers */
#define TS_SHIFT {			\
	[XMIT_SZ_8BIT]		= 0,	\
	[XMIT_SZ_16BIT]		= 1,	\
	[XMIT_SZ_32BIT]		= 2,	\
	[XMIT_SZ_64BIT]		= 3,	\
	[XMIT_SZ_128BIT]	= 4,	\
	[XMIT_SZ_256BIT]	= 5,	\
	[XMIT_SZ_512BIT]	= 6,	\
}

#define TS_INDEX2VAL(i) ((((i) & 3) << 3) | (((i) & 0xc) << (20 - 2)))
#define CHCR_TX(xmit_sz) (DM_FIX | SM_INC | 0x800 | TS_INDEX2VAL((xmit_sz)))
#define CHCR_RX(xmit_sz) (DM_INC | SM_FIX | 0x800 | TS_INDEX2VAL((xmit_sz)))

static const struct sh_dmae_slave_config r8a7373_dmae_slaves[] = {
	{
		.slave_id	= SHDMA_SLAVE_SCIF0_TX,
		.addr		= 0xe6450020,
		.chcr		= CHCR_TX(XMIT_SZ_8BIT),
		.mid_rid	= 0x21,
	}, {
		.slave_id	= SHDMA_SLAVE_SCIF0_RX,
		.addr		= 0xe6450024,
		.chcr		= CHCR_RX(XMIT_SZ_8BIT),
		.mid_rid	= 0x22,
	}, {
		.slave_id	= SHDMA_SLAVE_SCIF1_TX,
		.addr		= 0xe6c50020,
		.chcr		= CHCR_TX(XMIT_SZ_8BIT),
		.mid_rid	= 0x25,
	}, {
		.slave_id	= SHDMA_SLAVE_SCIF1_RX,
		.addr		= 0xe6c50024,
		.chcr		= CHCR_RX(XMIT_SZ_8BIT),
		.mid_rid	= 0x26,
	}, {
		.slave_id	= SHDMA_SLAVE_SCIF2_TX,
		.addr		= 0xe6c60020,
		.chcr		= CHCR_TX(XMIT_SZ_8BIT),
		.mid_rid	= 0x29,
	}, {
		.slave_id	= SHDMA_SLAVE_SCIF2_RX,
		.addr		= 0xe6c60024,
		.chcr		= CHCR_RX(XMIT_SZ_8BIT),
		.mid_rid	= 0x2a,
	}, {
		.slave_id	= SHDMA_SLAVE_SCIF3_TX,
		.addr		= 0xe6c70020,
		.chcr		= CHCR_TX(XMIT_SZ_8BIT),
		.mid_rid	= 0x2d,
	}, {
		.slave_id	= SHDMA_SLAVE_SCIF3_RX,
		.addr		= 0xe6c70024,
		.chcr		= CHCR_RX(XMIT_SZ_8BIT),
		.mid_rid	= 0x2e,
	}, {
		.slave_id	= SHDMA_SLAVE_SCIF4_TX,
		.addr		= 0xe6c20020,
		.chcr		= CHCR_TX(XMIT_SZ_8BIT),
		.mid_rid	= 0x3d,
	}, {
		.slave_id	= SHDMA_SLAVE_SCIF4_RX,
		.addr		= 0xe6c20024,
		.chcr		= CHCR_RX(XMIT_SZ_8BIT),
		.mid_rid	= 0x3e,
	}, {
		.slave_id	= SHDMA_SLAVE_SCIF5_TX,
		.addr		= 0xe6c30020,
		.chcr		= CHCR_TX(XMIT_SZ_8BIT),
		.mid_rid	= 0x19,
	}, {
		.slave_id	= SHDMA_SLAVE_SCIF5_RX,
		.addr		= 0xe6c30024,
		.chcr		= CHCR_RX(XMIT_SZ_8BIT),
		.mid_rid	= 0x1a,
	}, {
		.slave_id	= SHDMA_SLAVE_SCIF6_TX,
		.addr		= 0xe6ce0020,
		.chcr		= CHCR_TX(XMIT_SZ_8BIT),
		.mid_rid	= 0x1d,
	}, {
		.slave_id	= SHDMA_SLAVE_SCIF6_RX,
		.addr		= 0xe6ce0024,
		.chcr		= CHCR_RX(XMIT_SZ_8BIT),
		.mid_rid	= 0x1e,
	}, {
		.slave_id	= SHDMA_SLAVE_SCIF7_TX,
		.addr		= 0xe6470020,
		.chcr		= CHCR_TX(XMIT_SZ_8BIT),
		.mid_rid	= 0x35,
	}, {
		.slave_id	= SHDMA_SLAVE_SCIF7_RX,
		.addr		= 0xe6470024,
		.chcr		= CHCR_RX(XMIT_SZ_8BIT),
		.mid_rid	= 0x36,
	}, {
		.slave_id	= SHDMA_SLAVE_SDHI0_TX,
		.addr		= 0xee100030,
		.chcr		= CHCR_TX(XMIT_SZ_16BIT),
		.mid_rid	= 0xc1,
		.burst_sizes	= (1 << 1) | (1 << 4) | (1 << 5),
	}, {
		.slave_id	= SHDMA_SLAVE_SDHI0_RX,
		.addr		= 0xee100030,
		.chcr		= CHCR_RX(XMIT_SZ_16BIT),
		.mid_rid	= 0xc2,
		.burst_sizes	= (1 << 1) | (1 << 4) | (1 << 5),
	}, {
		.slave_id	= SHDMA_SLAVE_SDHI1_TX,
		.addr		= 0xee120030,
		.chcr		= CHCR_TX(XMIT_SZ_16BIT),
		.mid_rid	= 0xc5,
		.burst_sizes	= (1 << 1) | (1 << 4) | (1 << 5),
	}, {
		.slave_id	= SHDMA_SLAVE_SDHI1_RX,
		.addr		= 0xee120030,
		.chcr		= CHCR_RX(XMIT_SZ_16BIT),
		.mid_rid	= 0xc6,
		.burst_sizes	= (1 << 1) | (1 << 4) | (1 << 5),
	}, {
		.slave_id	= SHDMA_SLAVE_SDHI2_TX,
		.addr		= 0xee140030,
		.chcr		= CHCR_TX(XMIT_SZ_16BIT),
		.mid_rid	= 0xc9,
		.burst_sizes	= (1 << 1) | (1 << 4) | (1 << 5),
	}, {
		.slave_id	= SHDMA_SLAVE_SDHI2_RX,
		.addr		= 0xee140030,
		.chcr		= CHCR_RX(XMIT_SZ_16BIT),
		.mid_rid	= 0xca,
		.burst_sizes	= (1 << 1) | (1 << 4) | (1 << 5),
	}, {
		.slave_id	= SHDMA_SLAVE_MMCIF0_TX,
		.addr		= 0xe6bd0034,
		.chcr		= CHCR_TX(XMIT_SZ_32BIT),
		.mid_rid	= 0xd1,
	}, {
		.slave_id	= SHDMA_SLAVE_MMCIF0_RX,
		.addr		= 0xe6bd0034,
		.chcr		= CHCR_RX(XMIT_SZ_32BIT),
		.mid_rid	= 0xd2,
	}, {
		.slave_id	= SHDMA_SLAVE_MMCIF1_TX,
		.addr		= 0xe6be0034,
		.chcr		= CHCR_TX(XMIT_SZ_32BIT),
		.mid_rid	= 0xe1,
	}, {
		.slave_id	= SHDMA_SLAVE_MMCIF1_RX,
		.addr		= 0xe6be0034,
		.chcr		= CHCR_RX(XMIT_SZ_32BIT),
		.mid_rid	= 0xe2,
	}, {
		.slave_id	= SHDMA_SLAVE_FSIA_TX,
		.addr		= 0xec230024,
		.chcr		= CHCR_TX(XMIT_SZ_32BIT),
		.mid_rid	= 0xd5,
	}, {
		.slave_id	= SHDMA_SLAVE_FSIA_RX,
		.addr		= 0xec230020,
		.chcr		= CHCR_RX(XMIT_SZ_32BIT),
		.mid_rid	= 0xd6,
	}, {
		.slave_id	= SHDMA_SLAVE_FSIB_TX,
		.addr		= 0xec230064,
		.chcr		= CHCR_TX(XMIT_SZ_32BIT),
		.mid_rid	= 0xd9,
	}, {
		.slave_id	= SHDMA_SLAVE_FSIB_RX,
		.addr		= 0xec230060,
		.chcr		= CHCR_RX(XMIT_SZ_32BIT),
		.mid_rid	= 0xda,
	}, {
		.slave_id	= SHDMA_SLAVE_SCUW_FFD_TX,
		.addr		= 0xec700708,
		.chcr		= CHCR_TX(XMIT_SZ_32BIT),
		.mid_rid	= 0x79,
	}, {
		.slave_id	= SHDMA_SLAVE_SCUW_FFU_RX,
		.addr		= 0xec700714,
		.chcr		= CHCR_RX(XMIT_SZ_32BIT),
		.mid_rid	= 0x7a,
	}, {
		.slave_id	= SHDMA_SLAVE_SCUW_CPU_FIFO0_TX,
		.addr		= 0xec700720,
		.chcr		= CHCR_RX(XMIT_SZ_32BIT),
		.mid_rid	= 0x7d,
	}, {
		.slave_id	= SHDMA_SLAVE_SCUW_CPU_FIFO2_RX,
		.addr		= 0xec700738,
		.chcr		= CHCR_RX(XMIT_SZ_32BIT),
		.mid_rid	= 0x7e,
	}, {
		.slave_id	= SHDMA_SLAVE_SCUW_CPU_FIFO1_TX,
		.addr		= 0xec70072c,
		.chcr		= CHCR_RX(XMIT_SZ_32BIT),
		.mid_rid	= 0x81,
	}, {
		.slave_id	= SHDMA_SLAVE_PCM2PWM_TX,
		.addr		= 0xec380080,
		.chcr		= CHCR_RX(XMIT_SZ_16BIT),
		.mid_rid	= 0x91,
	},
};

#define DMAE_CHANNEL(_offset)					\
	{							\
		.offset		= _offset - 0x20,		\
		.dmars		= _offset - 0x20 + 0x40,	\
	}

static const struct sh_dmae_channel r8a7373_dmae_channels[] = {
	DMAE_CHANNEL(0x8000),
	DMAE_CHANNEL(0x8080),
	DMAE_CHANNEL(0x8100),
	DMAE_CHANNEL(0x8180),
	DMAE_CHANNEL(0x8200),
	DMAE_CHANNEL(0x8280),
	DMAE_CHANNEL(0x8300),
	DMAE_CHANNEL(0x8380),
	DMAE_CHANNEL(0x8400),
	DMAE_CHANNEL(0x8480),
	DMAE_CHANNEL(0x8500),
	DMAE_CHANNEL(0x8580),
	DMAE_CHANNEL(0x8600),
	DMAE_CHANNEL(0x8680),
	DMAE_CHANNEL(0x8700),
	DMAE_CHANNEL(0x8780),
	DMAE_CHANNEL(0x8800),
	DMAE_CHANNEL(0x8880),
#if 0
	/*
	 * Tha last two channels, CH18 and CH19, are intended for debugging
	 * purposes and come with different functions (for instance, they're
	 * not capable of slave DMA feature).  Such channels should not be
	 * registered to the kernel and the shdma driver.
	 */
	DMAE_CHANNEL(0x8900),
	DMAE_CHANNEL(0x8980),
#endif
};

static const unsigned int ts_shift[] = TS_SHIFT;

static struct sh_dmae_pdata r8a7373_dmae_platform_data = {
	.slave          = r8a7373_dmae_slaves,
	.slave_num      = ARRAY_SIZE(r8a7373_dmae_slaves),
	.channel        = r8a7373_dmae_channels,
	.channel_num    = ARRAY_SIZE(r8a7373_dmae_channels),
	.ts_low_shift   = 3,
	.ts_low_mask    = 0x18,
	.ts_high_shift  = (20 - 2),     /* 2 bits for shifted low TS */
	.ts_high_mask   = 0x00300000,
	.ts_shift       = ts_shift,
	.ts_shift_num   = ARRAY_SIZE(ts_shift),
	.dmaor_init     = DMAOR_DME,
};

static struct resource r8a7373_dmae_resources[] = {
	{
		/* Registers including DMAOR and channels including DMARSx */
		.start  = 0xfe000020,
		.end    = 0xfe008a00 - 1,
		.flags  = IORESOURCE_MEM,
	},
	{
		/* DMA error IRQ */
		.start  = gic_spi(167),
		.end    = gic_spi(167),
		.flags  = IORESOURCE_IRQ,
	},
	{
		/* IRQ for channels 0-17 */
		.start  = gic_spi(147),
		.end    = gic_spi(164),
		.flags  = IORESOURCE_IRQ,
	},
};

static struct platform_device dma0_device = {
	.name		= "sh-dma-engine",
	.id		= 0,
	.resource	= r8a7373_dmae_resources,
	.num_resources	= ARRAY_SIZE(r8a7373_dmae_resources),
	.dev		= {
		.platform_data	= &r8a7373_dmae_platform_data,
	},
};

/* Bus Semaphores 0 */
static struct hwsem_desc r8a7373_hwsem0_descs[] = {
	HWSEM(SMGPIO, 0x20),
	HWSEM(SMCPG, 0x50),
	HWSEM(SMSYSC, 0x70),
};

static struct hwsem_pdata r8a7373_hwsem0_platform_data = {
	.base_id	= SMGPIO,
	.descs		= r8a7373_hwsem0_descs,
	.nr_descs	= ARRAY_SIZE(r8a7373_hwsem0_descs),
};

static struct resource r8a7373_hwsem0_resources[] = {
	{
		.start  = 0xe6001800,
		.end    = 0xe600187f,
		.flags  = IORESOURCE_MEM,
	},
};

static struct platform_device hwsem0_device = {
	.name		= "rmobile_hwsem",
	.id		= 0,
	.resource	= r8a7373_hwsem0_resources,
	.num_resources	= ARRAY_SIZE(r8a7373_hwsem0_resources),
	.dev		= {
		.platform_data	= &r8a7373_hwsem0_platform_data,
	},
};

/*
 * These three HPB semaphores will be requested at board-init timing,
 * and globally available (even for out-of-tree loadable modules).
 */
struct hwspinlock *r8a7373_hwlock_gpio;
struct hwspinlock *r8a7373_hwlock_cpg;
struct hwspinlock *r8a7373_hwlock_sysc;
EXPORT_SYMBOL(r8a7373_hwlock_gpio);
EXPORT_SYMBOL(r8a7373_hwlock_cpg);
EXPORT_SYMBOL(r8a7373_hwlock_sysc);

/* Bus Semaphores 1 - general purpose semaphores with software extension */
static struct hwsem_desc r8a7373_hwsem1_descs[] = {
	HWSEM(SMGP000, 0x30), HWSEM(SMGP001, 0x30),
	HWSEM(SMGP002, 0x30), HWSEM(SMGP003, 0x30),
	HWSEM(SMGP004, 0x30), HWSEM(SMGP005, 0x30),
	HWSEM(SMGP006, 0x30), HWSEM(SMGP007, 0x30),
	HWSEM(SMGP008, 0x30), HWSEM(SMGP009, 0x30),
	HWSEM(SMGP010, 0x30), HWSEM(SMGP011, 0x30),
	HWSEM(SMGP012, 0x30), HWSEM(SMGP013, 0x30),
	HWSEM(SMGP014, 0x30), HWSEM(SMGP015, 0x30),
	HWSEM(SMGP016, 0x30), HWSEM(SMGP017, 0x30),
	HWSEM(SMGP018, 0x30), HWSEM(SMGP019, 0x30),
	HWSEM(SMGP020, 0x30), HWSEM(SMGP021, 0x30),
	HWSEM(SMGP022, 0x30), HWSEM(SMGP023, 0x30),
	HWSEM(SMGP024, 0x30), HWSEM(SMGP025, 0x30),
	HWSEM(SMGP026, 0x30), HWSEM(SMGP027, 0x30),
	HWSEM(SMGP028, 0x30), HWSEM(SMGP029, 0x30),
	HWSEM(SMGP030, 0x30), HWSEM(SMGP031, 0x30),
};

static struct hwsem_pdata r8a7373_hwsem1_platform_data = {
	.base_id	= SMGP000,
	.descs		= r8a7373_hwsem1_descs,
	.nr_descs	= ARRAY_SIZE(r8a7373_hwsem1_descs),
};

static struct resource r8a7373_hwsem1_resources[] = {
	{
		.start  = 0xe6001800,
		.end    = 0xe600187f,
		.flags  = IORESOURCE_MEM,
	},
	{
		.start  = 0xe63c0000, /* software extension base */
		.end    = 0xe63c007f,
		.flags  = IORESOURCE_MEM,
	},
};

static struct platform_device hwsem1_device = {
	.name		= "rmobile_hwsem",
	.id		= 1,
	.resource	= r8a7373_hwsem1_resources,
	.num_resources	= ARRAY_SIZE(r8a7373_hwsem1_resources),
	.dev		= {
		.platform_data	= &r8a7373_hwsem1_platform_data,
	},
};

static struct resource sgx_resources[] = {
	{
		.start  = gic_spi(92),
		.flags  = IORESOURCE_IRQ,
	},
};

static struct platform_device sgx_device = {
	.name		= "pvrsrvkm",
	.id		= -1,
	.resource	= sgx_resources,
	.num_resources	= ARRAY_SIZE(sgx_resources),
};

static struct platform_device *r8a7373_devices[] __initdata = {
	&scif0_device,
	&scif1_device,
	&scif2_device,
	&scif3_device,
	&scif4_device,
	&scif5_device,
	&scif6_device,
	&scif7_device,
	&i2c0_device,
	&i2c1_device,
	&i2c2_device,
	&i2c3_device,
	&i2c4_device,
	&i2c5_device,
	&i2c6_device,
	&i2c7_device,
	&dma0_device,
	&hwsem0_device,
	&hwsem1_device,
	&sgx_device,
};

void __init r8a7373_add_standard_devices(void)
{
#ifdef CONFIG_CACHE_L2X0
	/*
	 * [30] Early BRESP enable
	 * [27] Non-secure interrupt access control
	 * [26] Non-secure lockdown enable
	 * [22] Shared attribute override enable
	 * [19:17] Way-size: b010 = 32KB
	 * [16] Accosiativity: 0 = 8-way
	 */
	l2x0_init(IOMEM(0xf0100000), 0x4c440000, 0x820f0fff);
#endif

	r8a7373_pm_init();

	r8a7373_init_pm_domain(&r8a7373_a3sg);
	r8a7373_init_pm_domain(&r8a7373_a3sp);
	r8a7373_init_pm_domain(&r8a7373_a3r);
	r8a7373_init_pm_domain(&r8a7373_a4rm);
	r8a7373_init_pm_domain(&r8a7373_a4mp);

	r8a7373_pm_add_subdomain(&r8a7373_a4rm, &r8a7373_a3r);

	platform_add_devices(r8a7373_devices, ARRAY_SIZE(r8a7373_devices));

	r8a7373_add_device_to_domain(&r8a7373_a3sp, &i2c0_device);
	r8a7373_add_device_to_domain(&r8a7373_a3sp, &i2c1_device);
	r8a7373_add_device_to_domain(&r8a7373_a3sp, &i2c2_device);
	r8a7373_add_device_to_domain(&r8a7373_a3sp, &i2c3_device);
	r8a7373_add_device_to_domain(&r8a7373_a3sp, &i2c4_device);
	r8a7373_add_device_to_domain(&r8a7373_a3sp, &i2c5_device);
	r8a7373_add_device_to_domain(&r8a7373_a3sp, &i2c6_device);
	r8a7373_add_device_to_domain(&r8a7373_a3sp, &i2c7_device);
	r8a7373_add_device_to_domain(&r8a7373_a3sp, &dma0_device);
	r8a7373_add_device_to_domain(&r8a7373_a3sp, &scif1_device);
	r8a7373_add_device_to_domain(&r8a7373_a3sp, &scif2_device);
	r8a7373_add_device_to_domain(&r8a7373_a3sp, &scif3_device);
	r8a7373_add_device_to_domain(&r8a7373_a3sp, &scif4_device);
	r8a7373_add_device_to_domain(&r8a7373_a3sp, &scif5_device);
	r8a7373_add_device_to_domain(&r8a7373_a3sp, &scif6_device);
	r8a7373_add_device_to_domain(&r8a7373_a3sg, &sgx_device);
}

/* do nothing for !CONFIG_SMP or !CONFIG_HAVE_TWD */
void __init __weak r8a7373_register_twd(void) { }

/* CMT10 clocksource */
#define CMCLKE	0xe6131000
#define CMSTR0	0xe6130000
#define CMCSR0	0xe6130010
#define CMCNT0	0xe6130014
#define CMCOR0	0xe6130018

/* CMT14 sched_clock */
#define CMSTR4	0xe6130400
#define CMCSR4	0xe6130410
#define CMCNT4	0xe6130414
#define CMCOR4	0xe6130418

static u32 notrace cmt_read_sched_clock(void)
{
	return __raw_readl(CMCNT4);
}

static void __init cmt_clocksource_init(void)
{
	struct clk *cp_clk, *r_clk;
	unsigned long flags, rate;

	clk_enable(clk_get_sys("sh_cmt.10", NULL));
	cp_clk = clk_get(NULL, "cp_clk");
	rate = clk_get_rate(cp_clk);
	clk_enable(cp_clk);

	spin_lock_irqsave(&cmt_lock, flags);
	__raw_writel(__raw_readl(CMCLKE) | (1 << 0), CMCLKE);
	spin_unlock_irqrestore(&cmt_lock, flags);

	/* stop */
	__raw_writel(0, CMSTR0);

	/* setup */
	__raw_writel(0, CMCNT0);
	__raw_writel(0x10b, CMCSR0); /* Free-running, debug, cp_clk/1 */
	__raw_writel(0xffffffff, CMCOR0);
	while (__raw_readl(CMCNT0) != 0)
		;

	/* start */
	__raw_writel(1, CMSTR0);

	clocksource_mmio_init(IOMEM(CMCNT0), "cmt10", rate, 125, 32,
			      clocksource_mmio_readl_up);

	clk_enable(clk_get_sys("sh_cmt.14", NULL));
	r_clk = clk_get(NULL, "r_clk");
	clk_enable(r_clk);
	rate = clk_get_rate(r_clk);

	spin_lock_irqsave(&cmt_lock, flags);
	__raw_writel(__raw_readl(CMCLKE) | (1 << 4), CMCLKE);
	spin_unlock_irqrestore(&cmt_lock, flags);

	/* stop */
	__raw_writel(0, CMSTR4);

	/* setup */
	__raw_writel(0, CMCNT4);
	__raw_writel(0x10f, CMCSR4); /* Free-running, debug, RCLK/1 */
	__raw_writel(0xffffffff, CMCOR4);
	while (__raw_readl(CMCNT4) != 0)
		;

	/* start */
	__raw_writel(1, CMSTR4);

	setup_sched_clock(cmt_read_sched_clock, 32, rate);
}

static struct cmt_timer_clock cmt1_cks_table[] = {
	[0] = CKS("cp_clk", 8),
	[1] = CKS("cp_clk", 32),
	[2] = CKS("cp_clk", 128),
	[3] = CKS("cp_clk", 1),
	[4] = CKS("r_clk", 8),
	[5] = CKS("r_clk", 32),
	[6] = CKS("r_clk", 128),
	[7] = CKS("r_clk", 1),
	/* Pseudo 32KHz/1 is omitted */
};

/* CMT11, CMT12 clockevent */
static struct cmt_timer_config cmt1_timers[2] = {
	[0] = {
		.res = {
			DEFINE_RES_MEM(0xe6130100, 0x100),
			DEFINE_RES_IRQ(gic_spi(94)),
		},
		.name		= "sh_cmt.11",
		.timer_bit	= 1,
		.cks_table	= cmt1_cks_table,
		.cks_num	= ARRAY_SIZE(cmt1_cks_table),
		.cks		= 7,
		.cmcsr_init	= 0x128, /* Free-run, request interrupt, debug */
	},
	[1] = {
		.res = {
			DEFINE_RES_MEM(0xe6130200, 0x100),
			DEFINE_RES_IRQ(gic_spi(95)),
		},
		.name		= "sh_cmt.12",
		.timer_bit	= 2,
		.cks_table	= cmt1_cks_table,
		.cks_num	= ARRAY_SIZE(cmt1_cks_table),
		.cks		= 7,
		.cmcsr_init	= 0x128, /* Free-run, request interrupt, debug */
	},
};

/* Current timer */
int cmt_read_current_timer(unsigned long *timer_val)
{
	*timer_val = __raw_readl(CMCNT0);
	return 0;
}

struct delay_timer cmt_delay_timer __read_mostly;

static int __init setup_current_timer(void)
{
	struct clk *clk;

	/* sharing CMT10 with clocksource */
	clk = clk_get_sys("sh_cmt.10", NULL);
	if (IS_ERR(clk)) {
		pr_err("Error, cannot get clock for current timer\n");
		return PTR_ERR(clk);
	}
	clk_enable(clk);

	cmt_delay_timer.read_current_timer = cmt_read_current_timer;
	cmt_delay_timer.freq = clk_get_rate(clk);
	register_current_timer_delay(&cmt_delay_timer);
	return 0;
}

static void __init r8a7373_timer_init(void)
{
	r8a7373_clock_init();
	shmobile_calibrate_delay_early();
	cmt_clocksource_init();
	cmt_clockevent_init(cmt1_timers, 2, 0, CMCLKE);

	setup_current_timer();
}

#define CCCR		0xe600101c

void __init r8a7373_init_early(void)
{
	system_rev = __raw_readl(IOMEM(CCCR));

	/* override timer setup with soc-specific code */
	shmobile_timer.init = r8a7373_timer_init;
}
