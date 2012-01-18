#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <mach/io.h>
#include <asm/hardware/gic.h>

#define IRQC0_BASE	0xe61c0000
#define IRQC0_INTREQ_STS0	(IRQC0_BASE + 0x000)	/* R */
#define IRQC0_INTEN_STS0	(IRQC0_BASE + 0x004)	/* R/WC1 */
#define IRQC0_INTEN_SET0	(IRQC0_BASE + 0x008)	/* W */
#define IRQC0_WAKREQ_STS0	(IRQC0_BASE + 0x080)	/* R */
#define IRQC0_WAKEN_STS0	(IRQC0_BASE + 0x084)	/* R/WC1 */
#define IRQC0_WAKEN_SET0	(IRQC0_BASE + 0x088)	/* W */
#define IRQC0_DETECT_STATUS	(IRQC0_BASE + 0x100)	/* R/WC1 */
#define IRQC0_CONFIG_00		(IRQC0_BASE + 0x180)	/* R/W */

#define IRQC1_BASE	0xe61c0200
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

	reg = (irqpin >= 32) ? IRQC1_INTEN_STS0 : IRQC0_INTREQ_STS0;
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

static int r8a73734_irq_set_wake(struct irq_data *d, unsigned int on)
{
	return 0;
}

void __init r8a73734_init_irq(void)
{
	void __iomem *gic_dist_base = __io(0xf0001000);
	void __iomem *gic_cpu_base = __io(0xf0000100);
	int i;

	gic_init(0, 29, gic_dist_base, gic_cpu_base);
	gic_arch_extn.irq_set_wake = r8a73734_irq_set_wake;

	/* Setup IRQC cascade_irq */
	setup_irqc_irq();
	for (i = gic_spi(0); i < gic_spi(64); i++)
		irq_set_chained_handler(i, irqc_demux);
}
