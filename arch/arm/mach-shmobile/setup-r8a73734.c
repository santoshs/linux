#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/platform_device.h>
#include <linux/serial_sci.h>
#include <linux/sh_timer.h>
#include <linux/i2c/i2c-sh_mobile.h>
#include <mach/hardware.h>
#include <mach/r8a73734.h>

#define CKS(_name, _divisor) { .name = _name, .divisor = _divisor }

static struct sh_timer_clock cmt1_cks_table[] = {
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

static struct sh_timer_config cmt10_platform_data = {
	.name			= "CMT10",
	.channel_offset		= 0x1000 - 0,
	.timer_bit		= 0,
	.clocksource_rating	= 125,
	.cks_table	= cmt1_cks_table,
	.cks_num	= ARRAY_SIZE(cmt1_cks_table),
	.cks		= 3,
	.cmcsr_init	= 0x108, /* Free-running, debug */
};

static struct resource cmt10_resources[] = {
	{
		.name	= "CMT10",
		.start	= 0xe6130000,
		.end	= 0xe6131003,
		.flags	= IORESOURCE_MEM,
	},
};

static struct platform_device cmt10_device = {
	.name		= "sh_cmt",
	.id		= 10,
	.dev		= {
			.platform_data	= &cmt10_platform_data,
	},
	.resource	= cmt10_resources,
	.num_resources	= ARRAY_SIZE(cmt10_resources),
};

static struct sh_timer_config cmt11_platform_data = {
	.name			= "CMT11",
	.channel_offset		= 0x1000,
	.timer_bit		= 1,
	.clockevent_rating	= 125,
	.cks_table	= cmt1_cks_table,
	.cks_num	= ARRAY_SIZE(cmt1_cks_table),
	.cks		= 3,
	.cmcsr_init	= 0x128, /* Free-run, request interrupt, debug */
};

static struct resource cmt11_resources[] = {
	{
		.name	= "CMT11",
		.start	= 0xe6130100,
		.end	= 0xe6131003,
		.flags	= IORESOURCE_MEM,
	}, {
		.start	= gic_spi(94),
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device cmt11_device = {
	.name		= "sh_cmt",
	.id		= 11,
	.dev		= {
			.platform_data	= &cmt11_platform_data,
	},
	.resource	= cmt11_resources,
	.num_resources	= ARRAY_SIZE(cmt11_resources),
};

static struct sh_timer_config cmt12_platform_data = {
	.name			= "CMT12",
	.channel_offset		= 0x1000 - 0x200,
	.timer_bit		= 2,
	.clockevent_rating	= 125,
	.cks_table	= cmt1_cks_table,
	.cks_num	= ARRAY_SIZE(cmt1_cks_table),
	.cks		= 3,
	.cmcsr_init	= 0x128, /* Free-run, request interrupt, debug */
};

static struct resource cmt12_resources[] = {
	{
		.name	= "CMT12",
		.start	= 0xe6130200,
		.end	= 0xe6131003,
		.flags	= IORESOURCE_MEM,
	}, {
		.start	= gic_spi(95),
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device cmt12_device = {
	.name		= "sh_cmt",
	.id		= 12,
	.dev		= {
			.platform_data	= &cmt12_platform_data,
	},
	.resource	= cmt12_resources,
	.num_resources	= ARRAY_SIZE(cmt12_resources),
};

static struct plat_sci_port scif0_platform_data = {
	.mapbase	= 0xe6450000,
	.flags		= UPF_BOOT_AUTOCONF | UPF_IOREMAP,
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
	.flags		= UPF_BOOT_AUTOCONF | UPF_IOREMAP,
	.scscr		= SCSCR_RE | SCSCR_TE,
	.scbrr_algo_id	= SCBRR_ALGO_4,
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
	.flags		= UPF_BOOT_AUTOCONF | UPF_IOREMAP,
	.scscr		= SCSCR_RE | SCSCR_TE,
	.scbrr_algo_id	= SCBRR_ALGO_4,
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
	.flags		= UPF_BOOT_AUTOCONF | UPF_IOREMAP,
	.scscr		= SCSCR_RE | SCSCR_TE,
	.scbrr_algo_id	= SCBRR_ALGO_4,
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

static struct plat_sci_port scif4_platform_data = {
	.mapbase	= 0xe6c20000,
	.flags		= UPF_BOOT_AUTOCONF | UPF_IOREMAP,
	.scscr		= SCSCR_RE | SCSCR_TE,
	.scbrr_algo_id	= SCBRR_ALGO_4,
	.type		= PORT_SCIFB,
	.irqs		= { gic_spi(107), gic_spi(107),
			    gic_spi(107), gic_spi(107) },
};

static struct platform_device scif4_device = {
	.name		= "sh-sci",
	.id		= 4,
	.dev		= {
		.platform_data	= &scif4_platform_data,
	},
};

static struct plat_sci_port scif5_platform_data = {
	.mapbase	= 0xe6c30000,
	.flags		= UPF_BOOT_AUTOCONF | UPF_IOREMAP,
	.scscr		= SCSCR_RE | SCSCR_TE,
	.scbrr_algo_id	= SCBRR_ALGO_4,
	.type		= PORT_SCIFB,
	.irqs		= { gic_spi(108), gic_spi(108),
			    gic_spi(108), gic_spi(108) },
};

static struct platform_device scif5_device = {
	.name		= "sh-sci",
	.id		= 5,
	.dev		= {
		.platform_data	= &scif5_platform_data,
	},
};

static struct plat_sci_port scif6_platform_data = {
	.mapbase	= 0xe6ce0000,
	.flags		= UPF_BOOT_AUTOCONF | UPF_IOREMAP,
	.scscr		= SCSCR_RE | SCSCR_TE,
	.scbrr_algo_id	= SCBRR_ALGO_4,
	.type		= PORT_SCIFB,
	.irqs		= { gic_spi(116), gic_spi(116),
			    gic_spi(116), gic_spi(116) },
};

static struct platform_device scif6_device = {
	.name		= "sh-sci",
	.id		= 6,
	.dev		= {
		.platform_data	= &scif6_platform_data,
	},
};

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

static struct i2c_sh_mobile_platform_data i2c4_platform_data = {
	.bus_speed	= 400000,
};

static struct resource i2c4_resources[] = {
	[0] = {
		.name	= "IIC4",
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

static struct i2c_sh_mobile_platform_data i2c5_platform_data = {
	.bus_speed	= 400000,
};

static struct resource i2c5_resources[] = {
	[0] = {
		.name	= "IIC5",
		.start	= 0xe682a000,
		.end	= 0xe682a425 - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
#ifdef CONFIG_U2_ES1
	// This was swapped in ES1 (189 is for I2CB interrupt!)
		.start	= gic_spi(190),
		.end	= gic_spi(190),
#else
	// In ES2, 189 is for I2C5 and 190 for I2CB.
		.start	= gic_spi(189),
		.end	= gic_spi(189),
#endif
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

static struct platform_device i2c1_device = {
	.name		= "i2c-sh_mobile",
	.id		= 1,
	.resource	= i2c1_resources,
	.num_resources	= ARRAY_SIZE(i2c1_resources),
	.dev		= {
		.platform_data	= &i2c1_platform_data,
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

static struct platform_device i2c4_device = {
	.name		= "i2c-sh_mobile",
	.id		= 4,
	.resource	= i2c4_resources,
	.num_resources	= ARRAY_SIZE(i2c4_resources),
	.dev		= {
		.platform_data	= &i2c4_platform_data,
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

static const struct sh_dmae_slave_config r8a73734_dmae_slaves[] = {
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
		.slave_id	= SHDMA_SLAVE_SDHI0_TX,
		.addr		= 0xee100030,
		.chcr		= CHCR_TX(XMIT_SZ_16BIT),
		.mid_rid	= 0xc1,
	}, {
		.slave_id	= SHDMA_SLAVE_SDHI0_RX,
		.addr		= 0xee100030,
		.chcr		= CHCR_RX(XMIT_SZ_16BIT),
		.mid_rid	= 0xc2,
	}, {
		.slave_id	= SHDMA_SLAVE_SDHI1_TX,
		.addr		= 0xee120030,
		.chcr		= CHCR_TX(XMIT_SZ_16BIT),
		.mid_rid	= 0xc5,
	}, {
		.slave_id	= SHDMA_SLAVE_SDHI1_RX,
		.addr		= 0xee120030,
		.chcr		= CHCR_RX(XMIT_SZ_16BIT),
		.mid_rid	= 0xc6,
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
	},
};

#define DMAE_CHANNEL(_offset)					\
	{							\
		.offset		= _offset - 0x20,		\
		.dmars		= _offset - 0x20 + 0x40,	\
	}

static const struct sh_dmae_channel r8a73734_dmae_channels[] = {
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

static struct sh_dmae_pdata r8a73734_dmae_platform_data = {
	.slave          = r8a73734_dmae_slaves,
	.slave_num      = ARRAY_SIZE(r8a73734_dmae_slaves),
	.channel        = r8a73734_dmae_channels,
	.channel_num    = ARRAY_SIZE(r8a73734_dmae_channels),
	.ts_low_shift   = 3,
	.ts_low_mask    = 0x18,
	.ts_high_shift  = (20 - 2),     /* 2 bits for shifted low TS */
	.ts_high_mask   = 0x00300000,
	.ts_shift       = ts_shift,
	.ts_shift_num   = ARRAY_SIZE(ts_shift),
	.dmaor_init     = DMAOR_DME,
};

static struct resource r8a73734_dmae_resources[] = {
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
	.resource	= r8a73734_dmae_resources,
	.num_resources	= ARRAY_SIZE(r8a73734_dmae_resources),
	.dev		= {
		.platform_data	= &r8a73734_dmae_platform_data,
	},
};

#ifdef CONFIG_SMECO
static struct resource smc_resources[] =
{
  [0] = {
          .start = gic_spi(193),
          .end   = gic_spi(193),
          .flags = IORESOURCE_IRQ,
        },
  [1] = {
          .start = gic_spi(194),
          .end   = gic_spi(194),
          .flags = IORESOURCE_IRQ,
        },
  [2] = {
          .start = gic_spi(195),
          .end   = gic_spi(195),
          .flags = IORESOURCE_IRQ,
        },
  [3] = {
          .start = gic_spi(196),
          .end   = gic_spi(196),
          .flags = IORESOURCE_IRQ,
        },
};

static struct platform_device smc_netdevice0 =
{
    .name          = "smc_net_device",
    .id            = 0,
    .resource      = smc_resources,
    .num_resources = ARRAY_SIZE(smc_resources),
};

static struct platform_device smc_netdevice1 =
{
    .name          = "smc_net_device",
    .id            = 1,
    .resource      = smc_resources,
    .num_resources = ARRAY_SIZE(smc_resources),
};
#endif // CONFIG_SMECO

static struct platform_device *r8a73734_early_devices[] __initdata = {
	&cmt10_device,
	&cmt11_device,
	&cmt12_device,
	&scif0_device,
	&scif1_device,
	&scif2_device,
	&scif3_device,
	&scif4_device,
	&scif5_device,
	&scif6_device,
};

static struct platform_device *r8a73734_late_devices[] __initdata = {
	&i2c0_device,
	&i2c1_device,
	&i2c2_device,
	&i2c4_device,
	&i2c5_device,
	&dma0_device,
#ifdef CONFIG_SMECO
	&smc_netdevice0,
	&smc_netdevice1,
#endif
};

void __init r8a73734_add_standard_devices(void)
{
	platform_add_devices(r8a73734_early_devices,
			ARRAY_SIZE(r8a73734_early_devices));
	platform_add_devices(r8a73734_late_devices,
			ARRAY_SIZE(r8a73734_late_devices));
}

#define CCCR	IO_ADDRESS(0xe600101c)

extern void sh_cmt_register_devices(struct platform_device **devs, int num);

void __init r8a73734_add_early_devices(void)
{
	system_rev = __raw_readl(__io(CCCR));

	early_platform_add_devices(r8a73734_early_devices,
			ARRAY_SIZE(r8a73734_early_devices));

#ifdef CONFIG_SH_TIMER_CMT_ARM
	sh_cmt_register_devices(r8a73734_early_devices, 1 + NR_CPUS);
#endif
}
