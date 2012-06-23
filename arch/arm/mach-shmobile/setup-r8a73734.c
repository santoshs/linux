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
#include <linux/i2c-gpio.h>
#include <linux/gpio.h>

#define CKS(_name, _divisor) { .name = _name, .divisor = _divisor }

static bool is_es20(void);

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

//ES2.0 change start
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
//ES2.0 change stop


static struct i2c_sh_mobile_platform_data i2c4_platform_data = {
	.bus_speed	= 400000,
};
//IIC0H
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
//IIC1H
static struct resource i2c5_resources_es10[] = {
	[0] = {
		.name	= "IIC5",
		.start	= 0xe682a000,
		.end	= 0xe682a425 - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {

	// This was swapped in ES1 (189 is for I2CB interrupt!)
		.start	= gic_spi(190),
		.end	= gic_spi(190),

		.flags	= IORESOURCE_IRQ,
	},
};

static struct resource i2c5_resources_es20[] = {
	[0] = {
		.name	= "IIC5",
		.start	= 0xe682a000,
		.end	= 0xe682a425 - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
	// In ES2, 189 is for I2C5 and 190 for I2CB.
		.start	= gic_spi(189),
		.end	= gic_spi(189),

		.flags	= IORESOURCE_IRQ,
	},
};

static struct i2c_sh_mobile_platform_data i2c6_platform_data = {
	.bus_speed	= 400000,
};

static struct resource i2c6_resources[] = {
	[0] = {
		.name	= "IICM",
		.start	= 0xe6d20000,
		.end	= 0xe6d20009 - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= gic_spi(191),
		.end	= gic_spi(191),
		.flags	= IORESOURCE_IRQ,
	},
};

//ES2.0 change start
static struct i2c_sh_mobile_platform_data i2c7_platform_data = {
	.bus_speed	= 400000,
};
//IIC2H
static struct resource i2c7_resources[] = {
	[0] = {
		.name	= "IIC7",
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

static struct i2c_sh_mobile_platform_data i2c8_platform_data = {
	.bus_speed	= 400000,
};
//IIC3H
static struct resource i2c8_resources[] = {
	[0] = {
		.name	= "IIC8",
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
//ES2.0 change end

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

//ES2.0 change start
static struct platform_device i2c3_device= {
	.name		= "i2c-sh_mobile",
	.id		= 3,
	.resource	= i2c3_resources,
	.num_resources	= ARRAY_SIZE(i2c3_resources),
	.dev		= {
		.platform_data	= &i2c3_platform_data,
	},
};
//ES2.0 change end

static struct platform_device i2c4_device = {
	.name		= "i2c-sh_mobile",
	.id		= 4,
	.resource	= i2c4_resources,
	.num_resources	= ARRAY_SIZE(i2c4_resources),
	.dev		= {
		.platform_data	= &i2c4_platform_data,
	},
};

static struct platform_device i2c5_device_es10 = {
	.name		= "i2c-sh_mobile",
	.id		= 5,
	.resource	= i2c5_resources_es10,
	.num_resources	= ARRAY_SIZE(i2c5_resources_es10),
	.dev		= {
		.platform_data	= &i2c5_platform_data,
	},
};

static struct platform_device i2c5_device_es20 = {
	.name		= "i2c-sh_mobile",
	.id		= 5,
	.resource	= i2c5_resources_es20,
	.num_resources	= ARRAY_SIZE(i2c5_resources_es20),
	.dev		= {
		.platform_data	= &i2c5_platform_data,
	},
};

static struct platform_device i2c6_device = {
	.name		= "i2c-sh7730",
	.id		= 6,
	.resource	= i2c6_resources,
	.num_resources	= ARRAY_SIZE(i2c6_resources),
	.dev		= {
		.platform_data	= &i2c6_platform_data,
	},
};

// ES2.0 change start
static struct platform_device i2c7_device = {
	.name		= "i2c-sh_mobile",
	.id		= 7,
	.resource	= i2c7_resources,
	.num_resources	= ARRAY_SIZE(i2c7_resources),
	.dev		= {
		.platform_data	= &i2c7_platform_data,
	},
};

static struct platform_device i2c8_device = {
	.name		= "i2c-sh_mobile",
	.id		= 8,
	.resource	= i2c8_resources,
	.num_resources	= ARRAY_SIZE(i2c8_resources),
	.dev		= {
		.platform_data	= &i2c8_platform_data,
	},
};
//ES2.0 change end

//GPIO Port number needs to be modified by the respective driver module
//Udealy=5 will set I2C bus speed to 100k HZ

static struct i2c_gpio_platform_data  i2c0gpio_platform_data = {
      .sda_pin        = GPIO_PORT5,
      .scl_pin        = GPIO_PORT4,
      .udelay         = 5,
};

static struct platform_device i2c0gpio_device = {
  .name          = "i2c-gpio",
  .id    = 9,
  .dev           = {
         .platform_data  = &i2c0gpio_platform_data,
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
		.slave_id	= SHDMA_SLAVE_SDHI2_TX,
		.addr		= 0xee140030,
		.chcr		= CHCR_TX(XMIT_SZ_16BIT),
		.mid_rid	= 0xc9,
	}, {
		.slave_id	= SHDMA_SLAVE_SDHI2_RX,
		.addr		= 0xee140030,
		.chcr		= CHCR_RX(XMIT_SZ_16BIT),
		.mid_rid	= 0xca,
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
		.slave_id	= SHDMA_SLAVE_FSI2A_RX,
		.addr		= 0xec230020,
		.chcr		= CHCR_RX(XMIT_SZ_16BIT),
		.mid_rid	= 0xd6,
	}, {
		.slave_id	= SHDMA_SLAVE_FSI2A_TX,
		.addr           = 0xec230024,
		.chcr           = CHCR_TX(XMIT_SZ_32BIT),
		.mid_rid        = 0xd5,
	}, {
		.slave_id       = SHDMA_SLAVE_FSI2B_TX,
		.addr           = 0xec230064,
		.chcr           = CHCR_TX(XMIT_SZ_32BIT),
		.mid_rid        = 0xd9,
	}, {
		.slave_id       = SHDMA_SLAVE_FSI2B_RX,
		.addr           = 0xec230060,
		.chcr           = CHCR_RX(XMIT_SZ_16BIT),
		.mid_rid        = 0xda,
	}, {
		.slave_id       = SHDMA_SLAVE_SCUW_FFD_TX,
		.addr           = 0xEC700708,
		.chcr           = CHCR_TX(XMIT_SZ_32BIT),
		.mid_rid        = 0x79,
	}, {
		.slave_id       = SHDMA_SLAVE_SCUW_FFU_RX,
		.addr           = 0xEC700714,
		.chcr           = CHCR_RX(XMIT_SZ_32BIT),
		.mid_rid        = 0x7A,
	}, {
		.slave_id       = SHDMA_SLAVE_SCUW_CPUFIFO_0_TX,
		.addr           = 0xEC700720,
		.chcr           = CHCR_TX(XMIT_SZ_32BIT),
		.mid_rid        = 0x7D,
	}, {
		.slave_id       = SHDMA_SLAVE_SCUW_CPUFIFO_2_RX,
		.addr           = 0xEC700738,
		.chcr           = CHCR_RX(XMIT_SZ_32BIT),
		.mid_rid        = 0x7E,
	}, {
		.slave_id       = SHDMA_SLAVE_SCUW_CPUFIFO_1_TX,
		.addr           = 0xEC70072C,
		.chcr           = CHCR_TX(XMIT_SZ_32BIT),
		.mid_rid        = 0x81,
	},{
		.slave_id       = SHDMA_SLAVE_PCM2PWM_TX,
		.addr           = 0xec380080,
		.chcr           = CHCR_TX(XMIT_SZ_16BIT),
		.mid_rid        = 0x91,
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
		/* DescriptorMEM */
		.start  = 0xFE00A000,
		.end    = 0xFE00A7FC,
		.flags  = IORESOURCE_MEM,
	},
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

#if 0
static struct platform_device *r8a73734_late_devices[] __initdata = {
	&i2c0_device,
	&i2c1_device,
	&i2c2_device,
	&i2c4_device,
	/*&i2c5_device,*/
	&i2c6_device,
	&i2c0gpio_device,
	&dma0_device,
#ifdef CONFIG_SMECO
	&smc_netdevice0,
	&smc_netdevice1,
#endif
};
#endif
// HS-- ES10 Specific late devices
static struct platform_device *r8a73734_late_devices_es10[] __initdata = {
    &i2c0_device,
    &i2c1_device,
    &i2c2_device,
    &i2c4_device,
    &i2c5_device_es10,
    &i2c6_device,
    &dma0_device,
#ifdef CONFIG_SMECO
    &smc_netdevice0,
    &smc_netdevice1,
#endif
};

// HS-- ES20 Specific late devices
static struct platform_device *r8a73734_late_devices_es20[] __initdata = {
    &i2c0_device,
    &i2c1_device,
    &i2c2_device,
    &i2c3_device,
    &i2c4_device,
    &i2c5_device_es20,
    &i2c6_device,
    &i2c7_device,
    &i2c8_device,
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
//ES2.0 change start

	if ((system_rev & 0xFF) == 0x00)
	{
		platform_add_devices(r8a73734_late_devices_es10,
				ARRAY_SIZE(r8a73734_late_devices_es10));
	
	}
	else
	{
		platform_add_devices(r8a73734_late_devices_es20,
				ARRAY_SIZE(r8a73734_late_devices_es20)); 	  
	
	}
//ES2.0 change end

/*	platform_add_devices(r8a73734_early_devices,
			ARRAY_SIZE(r8a73734_early_devices));
	platform_add_devices(r8a73734_late_devices,
			ARRAY_SIZE(r8a73734_late_devices));

	if( is_es20() )
	{
		printk("Loading ES20 late devices...\n");
		platform_add_devices(r8a73734_late_devices_es20,
			ARRAY_SIZE(r8a73734_late_devices_es20));
	}
	else
	{
		printk("Loading ES10 late devices...\n");
		platform_add_devices(r8a73734_late_devices_es10,
			ARRAY_SIZE(r8a73734_late_devices_es10));
	} */
}

#define CCCR	IO_ADDRESS(0xe600101c)

static bool is_es20(void)
{
    unsigned int cccr = 0x00;
    unsigned int major = 0x00;
    unsigned int minor = 0x00;

    cccr = __raw_readl(__io(CCCR));

    major = ((cccr & 0xF0) >> 4) + 1;
    minor = cccr & 0x0F;

    return (((major&0xFF)<<4) + (minor&0xFF) == 0x20 );
}

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
