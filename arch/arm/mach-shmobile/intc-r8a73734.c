#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <mach/io.h>
#include <asm/hardware/gic.h>

static int r8a73734_irq_set_wake(struct irq_data *data, unsigned int on)
{
	return 0;
}

void __init r8a73734_init_irq(void)
{
	void __iomem *gic_dist_base = __io(0xf0001000);
	void __iomem *gic_cpu_base = __io(0xf0000100);

	gic_init(0, 9, gic_dist_base, gic_cpu_base);
	gic_arch_extn.irq_set_wake = r8a73734_irq_set_wake;
}
