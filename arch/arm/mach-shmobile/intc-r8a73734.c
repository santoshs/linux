#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/sh_intc.h>
#include <asm/hardware/gic.h>
#include <mach/r8a7373.h>

#define IRQC0_INTREQ_STS0	(IRQC0_BASE + 0x000)	/* R */
#define IRQC0_INTEN_STS0	(IRQC0_BASE + 0x004)	/* R/WC1 */
#define IRQC0_INTEN_SET0	(IRQC0_BASE + 0x008)	/* W */
#define IRQC0_WAKREQ_STS0	(IRQC0_BASE + 0x080)	/* R */
#define IRQC0_WAKEN_STS0	(IRQC0_BASE + 0x084)	/* R/WC1 */
#define IRQC0_WAKEN_SET0	(IRQC0_BASE + 0x088)	/* W */
#define IRQC0_DETECT_STATUS	(IRQC0_BASE + 0x100)	/* R/WC1 */
#define IRQC0_CONFIG_00		(IRQC0_BASE + 0x180)	/* R/W */

#define IRQC1_INTREQ_STS0	(IRQC1_BASE + 0x000)	/* R */
#define IRQC1_INTEN_STS0	(IRQC1_BASE + 0x004)	/* R/WC1 */
#define IRQC1_INTEN_SET0	(IRQC1_BASE + 0x008)	/* W */
#define IRQC1_WAKREQ_STS0	(IRQC1_BASE + 0x080)	/* R */
#define IRQC1_WAKEN_STS0	(IRQC1_BASE + 0x084)	/* R/WC1 */
#define IRQC1_WAKEN_SET0	(IRQC1_BASE + 0x088)	/* W */
#define IRQC1_DETECT_STATUS	(IRQC1_BASE + 0x100)	/* R/WC1 */
#define IRQC1_CONFIG_00		(IRQC1_BASE + 0x180)	/* R/W */

static void irqc_irq_mask(struct irq_data *d)
{
	u32 irqpin = d->irq - IRQPIN_IRQ_BASE;
	u32 reg;

	reg = (irqpin >= 32) ? IRQC1_INTEN_STS0 : IRQC0_INTEN_STS0;
	__raw_writel(1 << (irqpin & 0x1f), reg);
}

static void irqc_irq_unmask(struct irq_data *d)
{
	u32 irqpin = d->irq - IRQPIN_IRQ_BASE;
	u32 reg;

	reg = (irqpin >= 32) ? IRQC1_INTEN_SET0 : IRQC0_INTEN_SET0;
	__raw_writel(1 << (irqpin & 0x1f), reg);
}

static void irqc_irq_ack(struct irq_data *d)
{
	u32 irqpin = d->irq - IRQPIN_IRQ_BASE;
	u32 reg;

	reg = (irqpin >= 32) ? IRQC1_DETECT_STATUS : IRQC0_DETECT_STATUS;
	__raw_writel(1 << (irqpin & 0x1f), reg);
}

static int irqc_set_irq_type(struct irq_data *d, unsigned int type)
{
	u32 irqpin = d->irq - IRQPIN_IRQ_BASE;
	u32 *reg;

	switch (type) {
	case IRQ_TYPE_LEVEL_LOW:
		type = 0x01;
		__irq_set_handler_locked(d->irq, handle_level_irq);
		break;
	case IRQ_TYPE_LEVEL_HIGH:
		type = 0x02;
		__irq_set_handler_locked(d->irq, handle_level_irq);
		break;
	case IRQ_TYPE_EDGE_FALLING:
		type = 0x04;
		__irq_set_handler_locked(d->irq, handle_edge_irq);
		break;
	case IRQ_TYPE_EDGE_RISING:
		type = 0x08;
		__irq_set_handler_locked(d->irq, handle_edge_irq);
		break;
	case IRQ_TYPE_EDGE_BOTH:
		type = 0x0c;
		__irq_set_handler_locked(d->irq, handle_edge_irq);
		break;
	default:
		return -EINVAL;
	}

	reg = (irqpin >= 32) ? (u32 *)IRQC1_CONFIG_00 : (u32 *)IRQC0_CONFIG_00;

	reg += (irqpin & 0x1f);
	__raw_writel((__raw_readl(reg) & ~0x3f) | type, reg);

	return 0;
}

static int irqc_set_irq_wake(struct irq_data *d, unsigned int on)
{
	u32 irqpin = d->irq - IRQPIN_IRQ_BASE;
	u32 reg;

	if (on)
		reg = (irqpin >= 32) ? IRQC1_WAKEN_SET0 : IRQC0_WAKEN_SET0;
	else
		reg = (irqpin >= 32) ? IRQC1_WAKEN_STS0 : IRQC0_WAKEN_STS0;
	__raw_writel(1 << (irqpin & 0x1f), reg);

	return 0;
}

static struct irq_chip irqc_chip = {
	.name		= "IRQC",
	.irq_ack	= irqc_irq_ack,
	.irq_mask	= irqc_irq_mask,
	.irq_unmask	= irqc_irq_unmask,
	.irq_set_type	= irqc_set_irq_type,
	.irq_set_wake	= irqc_set_irq_wake,
};

static void irqc_demux(unsigned int irq, struct irq_desc *desc)
{
	u32 irqpin_irq = IRQPIN_IRQ_BASE + irq - gic_spi(0);

	generic_handle_irq(irqpin_irq);
	desc->irq_data.chip->irq_eoi(&desc->irq_data);
}

static void setup_irqc_irq(void)
{
	u32 *irqc0 = (u32 *)IRQC0_CONFIG_00;
	u32 *irqc1 = (u32 *)IRQC1_CONFIG_00;
	int i;

	/* mask interrupts */
	__raw_writel(0xffffffff, IRQC0_INTEN_STS0);
	__raw_writel(0xffffffff, IRQC1_INTEN_STS0);

	/* clear interrupts */
	__raw_writel(0xffffffff, IRQC0_DETECT_STATUS);
	__raw_writel(0xffffffff, IRQC1_DETECT_STATUS);

	/* all interrupts high-level */
	for (i = 0; i < 32; i++) {
		__raw_writel((__raw_readl(irqc0) & ~0x3f) | 0x2, irqc0);
		irqc0++;
		__raw_writel((__raw_readl(irqc1) & ~0x3f) | 0x2, irqc1);
		irqc1++;
	}

	for (i = IRQPIN_IRQ_BASE; i < IRQPIN_IRQ_BASE + 64; i++) {
		irq_set_chip_and_handler(i, &irqc_chip, handle_level_irq);
		set_irq_flags(i, IRQF_VALID);
	}
}

enum {
	IRQC_INTERVAL_1MS = 0,
	IRQC_INTERVAL_2MS,
	IRQC_INTERVAL_4MS,
	IRQC_INTERVAL_8MS
};

int r8a73734_irqc_set_debounce(int irq, unsigned debounce)
{
	u32 val, interval, count;
	u32 *reg;

	irq -= IRQPIN_IRQ_BASE;
	if (irq > 63)
		return -ENOSYS;

	debounce = (debounce + 999) / 1000;
	if (debounce <= 0x3ff) {
		interval = IRQC_INTERVAL_1MS;
		count = debounce;
	} else if (debounce <= 0x3ff * 2) {
		interval = IRQC_INTERVAL_2MS;
		count = (debounce + 1) / 2;
	} else if (debounce <= 0x3ff * 4) {
		interval = IRQC_INTERVAL_4MS;
		count = (debounce + 3) / 4;
	} else if (debounce <= 0x3ff * 8) {
		interval = IRQC_INTERVAL_8MS;
		count = (debounce + 7) / 8;
	} else {
		interval = IRQC_INTERVAL_8MS;
		count = 0x3ff;
	}

	reg = (irq >= 32) ? (u32 *)IRQC1_CONFIG_00 : (u32 *)IRQC0_CONFIG_00;
	reg += (irq & 0x1f);

	val = __raw_readl(reg) & ~0x80ff0000;
	__raw_writel(val | (1 << 31) | (interval << 22) | (count << 16), reg);
	return 0;
}

enum {
	UNUSED = 0,

	/* interrupt sources INTCS */
	VPU, _2DDM0, TSIF, LMB, JPU_JPEG, LCDC, CSIRX,
	DSITX0_DSITX0, DSITX0_DSITX1, FSI, VSP1, ISP_PRE, ISP_POS, RCU0,
	RCU1, CSIRX1,
	/* interrupt groups INTCS */
	DSITX0,
};

static struct intc_vect intcs_vectors[] = {
	INTCS_VECT(VPU, 0x0980), INTCS_VECT(_2DDM0, 0x0a00),
	INTCS_VECT(TSIF, 0x0f20), INTCS_VECT(LMB, 0x0f60),
	INTCS_VECT(JPU_JPEG, 0x0560), INTCS_VECT(LCDC, 0x0580),
	INTCS_VECT(CSIRX, 0x17a0), INTCS_VECT(DSITX0_DSITX0, 0x17c0),
	INTCS_VECT(DSITX0_DSITX1, 0x17e0), INTCS_VECT(FSI, 0x1840),
	INTCS_VECT(VSP1, 0x1b60), INTCS_VECT(ISP_PRE, 0x1d40),
	INTCS_VECT(ISP_POS, 0x1d60), INTCS_VECT(RCU0, 0x1de0),
	INTCS_VECT(RCU1, 0x1dc0), INTCS_VECT(CSIRX1, 0x1be0),
};

static struct intc_mask_reg intcs_mask_registers[] = {
	{ 0xffd20188, 0xffd201c8, 8, /* IMR2SA / IMCR2SA */
	  { 0, 0, 0, VPU,
	    0, 0, 0, 0 } },
	{ 0xffd2018c, 0xffd201cc, 8, /* IMR3SA / IMCR3SA */
	  { 0, 0, 0, _2DDM0,
	    0, 0, 0, 0 } },
	{ 0xffd20190, 0xffd201d0, 8, /* IMR4SA / IMCR4SA */
	  { 0, 0, 0, 0,
	    JPU_JPEG, 0, 0, LCDC } },
	{ 0xffd201ac, 0xffd201ec, 8, /* IMR11SA / IMCR11SA */
	  { 0, 0, 0, 0,
	    0, 0, LMB, TSIF } },
	{ 0xffd50190, 0xffd501d0, 8, /* IMR4SA3 / IMCR4SA3 */
	  { 0, 0, 0, 0,
	    0, CSIRX, DSITX0_DSITX0, DSITX0_DSITX1 } },
	{ 0xffd50194, 0xffd501d4, 8, /* IMR5SA3 / IMCR5SA3 */
	  { 0, 0, FSI, 0,
	    0, 0, 0, 0 } },
	{ 0xffd501a0, 0xffd501e0, 8, /* IMR8SA3 / IMCR8SA3 */
	  { 0, 0, 0, VSP1,
	    0, 0, 0, CSIRX1 } },
	{ 0xffd501a8, 0xffd501e8, 8, /* IMR10SA3 / IMCR10SA3 */
	  { 0, 0, ISP_PRE, ISP_POS,
	    0, 0, RCU1, RCU0 } },
};

static struct intc_group intcs_groups[] __initdata = {
	INTC_GROUP(DSITX0, DSITX0_DSITX0, DSITX0_DSITX1),
};

static struct intc_prio_reg intcs_prio_registers[] = {
	{ 0xffd20000, 0, 16, 4, /* IPRAS */ { 0, 0, _2DDM0, 0 } },
	{ 0xffd20004, 0, 16, 4, /* IPRBS */ { JPU_JPEG, LCDC, 0, 0 } },
	{ 0xffd20010, 0, 16, 4, /* IPRES */ { 0, 0, 0, VPU } },
	{ 0xffd20020, 0, 16, 4, /* IPRIS */ { 0, 0, TSIF, 0 } },
	{ 0xffd20028, 0, 16, 4, /* IPRKS */ { 0, 0, LMB, 0 } },
	{ 0xffd50024, 0, 16, 4, /* IPRJS3 */ { 0, CSIRX, DSITX0, 0 } },
	{ 0xffd50028, 0, 16, 4, /* IPRKS3 */ { 0, 0, FSI, 0 } },
	{ 0xffd50040, 0, 16, 4, /* IPRQS3 */ { 0, 0, 0, VSP1 } },
	{ 0xffd50050, 0, 16, 4, /* IPRUS3 */ { 0, 0, ISP_PRE, ISP_POS } },
	{ 0xffd50054, 0, 16, 4, /* IPRVS3 */ { 0, 0, RCU1, RCU0 } },
	{ 0xffd50044, 0, 16, 4, /* IPRRS3 */ { 0, 0, 0, CSIRX1 } },
};

static struct resource intcs_resources[] __initdata = {
	[0] = {
		.start	= 0xffd20000,
		.end	= 0xffd201ff,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= 0xffd50000,
		.end	= 0xffd501ff,
		.flags	= IORESOURCE_MEM,
	},
};

static struct intc_desc intcs_desc __initdata = {
	.name = "intcs",
	.skip_syscore_suspend = true,
	.resource = intcs_resources,
	.num_resources = ARRAY_SIZE(intcs_resources),
	.hw = INTC_HW_DESC(intcs_vectors, intcs_groups, intcs_mask_registers,
			   intcs_prio_registers, NULL, NULL),
};

static void intcs_demux(unsigned int irq, struct irq_desc *desc)
{
	void __iomem *reg = (void *)irq_get_handler_data(irq);
	unsigned int evtcodeas = ioread32(reg);

	generic_handle_irq(intcs_evt2irq(evtcodeas));

	/* primary controller unmasking */
	desc->irq_data.chip->irq_eoi(&desc->irq_data);
}

static int r8a73734_irq_set_wake(struct irq_data *d, unsigned int on)
{
	return 0;
}

void __init r8a73734_init_irq(void)
{
	void __iomem *gic_dist_base = __io(IO_ADDRESS(0xf0001000));
	void __iomem *gic_cpu_base = __io(IO_ADDRESS(0xf0000100));
	void __iomem *intevtsa = ioremap_nocache(0xffd20100, PAGE_SIZE);
	int i;
	BUG_ON(!intevtsa);

	gic_init(0, 29, gic_dist_base, gic_cpu_base);
	gic_arch_extn.irq_set_wake = r8a73734_irq_set_wake;

	/* Setup IRQC cascade_irq */
	setup_irqc_irq();
	for (i = gic_spi(0); i < gic_spi(64); i++)
		irq_set_chained_handler(i, irqc_demux);

	/* Setup INTCS cascade_irq */
	register_intc_controller(&intcs_desc);
	irq_set_handler_data(gic_spi(223), (void *)intevtsa);
	irq_set_chained_handler(gic_spi(223), intcs_demux);
}
