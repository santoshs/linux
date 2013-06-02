 /*
  * rmu2_cmt15.c
  *
  * Copyright (C) 2013 Renesas Mobile Corporation
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the GNU General Public License
  * version 2 as published by the Free Software Foundation.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  * MA  02110-1301, USA.
  */

#include <linux/rmu2_cmt15.h>
#include <linux/io.h>
#include <linux/hwspinlock.h>
#include <linux/cpumask.h>
#include <linux/delay.h>

/* Android provides the fiq_glue API, which hasn't yet made it into core
 * Linux. This works better if the glue is available, but it can still work
 * with the basic fiq API, if we need to run on non-Android Linux.
 */
#ifdef CONFIG_FIQ_GLUE
#include <asm/fiq_glue.h>
#else
#include <asm/fiq.h>
#endif

/* Note this option only controls behaviour on non-secure images. In secure
 * images, it's up to the secure code whether the CMT is a FIQ or not. */
#define CONFIG_RMU2_CMT_FIQ

#ifdef CONFIG_RWDT_CMT15_TEST
#include <linux/proc_fs.h>
#include <asm/cacheflush.h>
#include <asm/traps.h>

static struct proc_dir_entry *proc_watch_entry;

/* Various nasty things we can do to the system to test the watchdog and
 * CMT timer. Example: "echo 8 > /proc/proc_watch_entry"
 */
int test_mode;

void loop(void *info)
{
	uint32_t psr = 0;

	if ((unsigned)info & 1)
		local_irq_disable();
	if ((unsigned)info & 2)
		local_fiq_disable();

	asm volatile(
		"       mrs     %0, cpsr"
		: "=r" (psr) : : "memory");

	printk(KERN_ALERT "RWDT test loop (CPU %d, preemption %s, IRQs %s, FIQs %s)\n",
			raw_smp_processor_id(),
			preempt_count() ? "off" : "on",
			psr & PSR_I_BIT ? "off" : "on",
			psr & PSR_F_BIT ? "off" : "on");

	/* Try to avoid sucking power. Note that cpu_do_idle() etc all use
	 * WFI, which won't do - it proceeds if there's a pending masked
	 * interrupt. WFE only proceeds for unmasked interrupts, which is
	 * what we want. */
	while (1)
		wfe();
}

static void loop_processor_vector(const char *name, unsigned long vector)
{
	printk(KERN_EMERG "Overwriting processor %s vector with loop\n", name);

	/* Logic copied from arch/arm/kernel/fiq.c - not sure if
	 * it's correct for all CONFIG variants, but works for ours. */
#ifdef CONFIG_CPU_USE_DOMAINS
	/* Write directly to execution address (why is this okay?)  */
	vector += CONFIG_VECTORS_BASE;
#else
	/* Use address of page in kernel linear mapping (actual
	 * vector page is read-only) */
	vector += (unsigned long) vectors_page;
#endif
#ifdef CONFIG_THUMB2_KERNEL
	*(uint16_t *)vector = 0xE7FE; /* Thumb branch to self */
#else
	*(uint32_t *)vector = 0xEAFFFFFE; /* ARM branch to self */
#endif
#if 0
/* Don't bother cleaning - crashes inside the clean are ugly due to
 *  lack of unwinding, and it will get through eventually anyway.
 */

	/* If we did it through vectors_page, it doesn't need to
	 * actually clean any D-cache, as it's write-through, so the
	 * wrong address for the data doesn't matter. Note that
	 * flush_icache_range() is supposed to work work on all cores -
	 * it does if they're all ARMv7 Inner Shareable, at least.
	 */
	flush_icache_range(CONFIG_VECTORS_BASE,
				CONFIG_VECTORS_BASE+0x20);
#endif
}
#endif

#ifdef CONFIG_GIC_NS_CMT
static DEFINE_SPINLOCK(cmt_lock);
#endif  /* CONFIG_GIC_NS_CMT */

#ifndef CONFIG_FIQ_GLUE
static DEFINE_PER_CPU(struct pt_regs, rmu2_cmt_fiq_regs);
#endif

static struct platform_device rmu2_cmt_dev = {
	.name = "cmt15",
	.id = -1,
};

/*
 * cpg_check_init: CPG Check initialization
 * input: none
 * output: none
 * return: none
 */
static void cpg_check_init(void)
{
#ifdef SHBUS_TIMEOUT_CHECK_ENABLE
	__raw_writel(0x3fff3fffU, CPG_CHECK_REG);
	__raw_writel(0x3fff3fffU, CPG_CHECK_REG + 4);
#endif /* SHBUS_TIMEOUT_CHECK_ENABLE */
}

/*
 * cpg_check_check: CPG Check
 * input: none
 * output: none
 * return: none
 */
void cpg_check_check(void)
{
#ifdef SHBUS_TIMEOUT_CHECK_ENABLE
	unsigned int val0;
	unsigned int val1;
	unsigned int addr;

	/* get values */
	val0 = __raw_readl(CPG_CHECK_REG);
	val1 = __raw_readl(CPG_CHECK_REG + 4);

	/* check */
	if ((0U != (val0 & 0x80008000U)) || (0U != (val1 & 0x80008000U))) {
		printk(KERN_EMERG "CPG STSTUS\n");
		printk(KERN_EMERG " %08x=%08x\n", CPG_CHECK_STATUS,
					__raw_readl(CPG_CHECK_STATUS));
		printk(KERN_EMERG " %08x=%08x\n", CPG_CHECK_REG, val0);
		printk(KERN_EMERG " %08x=%08x\n", CPG_CHECK_REG + 4, val1);
		for (addr = 0xE6150440U; addr <= 0xE615047CU; addr += 4U) {
			printk(KERN_EMERG " %08x=%08x\n",
					addr, __raw_readl(addr));
		}
		for (addr = 0xE6150490U; addr <= 0xE615049CU; addr += 4U) {
			printk(KERN_EMERG " %08x=%08x\n",
					addr, __raw_readl(addr));
		}

		printk(KERN_EMERG "Bus timeout occurred!!");
		} else if ((0x3fff3fffU != (val0 & 0x3fff3fffU)) ||
					(0x3fff3fffU != (val1 & 0x3fff3fffU))) {
		printk(KERN_ALERT "CPG CHECK register was modified\n");
		printk(KERN_ALERT "and should be reset\n");
		__raw_writel(0x3fff3fffU, CPG_CHECK_REG);
		__raw_writel(0x3fff3fffU, CPG_CHECK_REG + 4);
	} else {
		/* Do nothing */
	}
#endif /* SHBUS_TIMEOUT_CHECK_ENABLE */
}

#ifdef CONFIG_GIC_NS_CMT
/*
 * rmu2_cmt_start: start CMT
 * input: none
 * output: none
 * return: none
 */
static void rmu2_cmt_start(void)
{
	unsigned long flags = 0;
	unsigned long wrflg = 0;
	unsigned long i = 0;
	printk(KERN_INFO "START < %s >\n", __func__);

	spin_lock_irqsave(&cmt_lock, flags);
	__raw_writel(__raw_readl(CMCLKE) | (1<<5), CMCLKE);
	spin_unlock_irqrestore(&cmt_lock, flags);

	mdelay(8);

	__raw_writel(0, CMSTR15);
	__raw_writel(0U, CMCNT15);
	__raw_writel(0x000001a6U, CMCSR15);     /* Int enable */
	__raw_writel(CMT_OVF, CMCOR15);

	do {
		wrflg = ((__raw_readl(CMCSR15) >> 13) & 0x1);
		i++;
	} while (wrflg != 0x00 && i < 0xffffffff);

	__raw_writel(1, CMSTR15);

	printk(KERN_INFO "< %s >CMCLKE=%08x\n", __func__,
					__raw_readl(CMCLKE));
	printk(KERN_INFO "< %s >CMSTR15=%08x\n", __func__,
					__raw_readl(CMSTR15));
	printk(KERN_INFO "< %s >CMCSR15=%08x\n", __func__,
					__raw_readl(CMCSR15));
	printk(KERN_INFO "< %s >CMCNT15=%08x\n", __func__,
					__raw_readl(CMCNT15));
	printk(KERN_INFO "< %s >CMCOR15=%08x\n", __func__,
					__raw_readl(CMCOR15));
}

/*
 * rmu2_cmt_stop: stop CMT
 * input: none
 * output: none
 * return: none
 */
void rmu2_cmt_stop(void)
{
	unsigned long flags = 0;
	unsigned long wrflg = 0;
	unsigned long i = 0;

	__raw_readl(CMCSR15);
	__raw_writel(0x00000186U, CMCSR15);     /* Int disable */
	__raw_writel(0U, CMCNT15);
	__raw_writel(0, CMSTR15);

	do {
		wrflg = ((__raw_readl(CMCSR15) >> 13) & 0x1);
		i++;
	} while (wrflg != 0x00 && i < 0xffffffff);

	mdelay(12);
	spin_lock_irqsave(&cmt_lock, flags);
	__raw_writel(__raw_readl(CMCLKE) & ~(1<<5), CMCLKE);
	spin_unlock_irqrestore(&cmt_lock, flags);
	printk(KERN_INFO "START < %s >\n", __func__);
}

/*
 * rmu2_cmt_clear: CMT counter clear
 * input: none
 * output: none
 * return: none
 */
void rmu2_cmt_clear(void)
{
	int wrflg = 0;
	int i = 0;
	unsigned long flags;

	spin_lock_irqsave(&cmt_lock, flags);
	__raw_writel(0, CMSTR15);       /* Stop counting */
	__raw_writel(0U, CMCNT15);      /* Clear the count value */

	do {
		wrflg = ((__raw_readl(CMCSR15) >> 13) & 0x1);
		i++;
	} while (wrflg != 0x00 && i < 0xffffffff);

	__raw_writel(1, CMSTR15);       /* Enable counting again */
	spin_unlock_irqrestore(&cmt_lock, flags);
	/*printk(KERN_INFO "START < %s >\n", __func__);*/
}

/*
 * rmu2_cmt_irq: IRQ handler for CMT
 * input:
 * @irq: interrupt number
 * @dev_id: device ID
 * output: none
 * return:
 * IRQ_HANDLED: irq handled
 */
static irqreturn_t rmu2_cmt_irq(int irq, void *dev_id)
{
	unsigned int reg_val = __raw_readl(CMCSR15);
#ifdef CONFIG_ARM_TZ
	unsigned char *killer = NULL;
	printk(KERN_ERR "TRUST ZONE ENABLED : CMT15 counter overflow occur!\n");
#else

	printk(KERN_ERR "TRUST ZONE DISABLED : CMT15 counter overflow occur!\n");
#endif/* CONFIG_ARM_TZ */

	reg_val &= ~0x0000c000U;
	__raw_writel(reg_val, CMCSR15);

#ifdef CONFIG_ARM_TZ
	printk(KERN_ERR "Watchdog will trigger in 5 sec... Generating panic to collect logs...");
	*killer = 1;
#endif /* CONFIG_ARM_TZ */

	return IRQ_HANDLED;
}

/*
 * rmu2_cmt_fiq: FIQ handler for CMT. Called on all CPUs simultaneously.
 * return: must not return
 */
#ifdef CONFIG_FIQ_GLUE
static void rmu2_cmt_fiq(struct fiq_glue_handler *h, void *r, void *svc_sp)
{
	struct pt_regs *regs = r; /* Layout matches + orig_SPSR in orig_r0 */

	/* If we come in via the FIQ glue, then we are in SVC mode, but running
	 * on a FIQ stack. svc_sp points to the original SVC stack pointer.
	 * Good for reliability of getting here, bad for stack unwinding.
	 * The trick to transplant current_thread_info() copied from
	 * the FIQ debugger.
	 */
#define THREAD_INFO(sp) ((struct thread_info *) \
		((unsigned long)(sp) & ~(THREAD_SIZE - 1)))
	struct thread_info *real_thread_info = THREAD_INFO(svc_sp);
	int err = (int) svc_sp;

	/* This is SMP-safe-ish, at least for the FIQ handler being entered
	 * simultaneously - the bit will end up set, maybe more than once.
	 */
	u8 stbchr2 = __raw_readb(STBCHR2);
	__raw_writeb(stbchr2 | APE_RESETLOG_RWDT_CMT_FIQ, STBCHR2);

	*current_thread_info() = *real_thread_info;
#else
asmlinkage void rmu2_cmt_fiq(struct pt_regs *regs)
{
	/* If we come in through our assembler, we're on the original SVC
	 * mode stack, with nothing extra pushed. Good for unwinding, bad if
	 * the stack was broken. Our assembler does the STBCHR2 write.
	 */
	int err = 0;
#endif

	console_verbose();

	printk(KERN_CRIT "Watchdog FIQ received\n");

	die("Watchdog FIQ", regs, err);
	panic("Watchdog FIQ");
}

#ifdef CONFIG_FIQ_GLUE
static struct fiq_glue_handler fh = {
	.fiq = rmu2_cmt_fiq
};

#else
static struct fiq_handler fh = {
	.name = "rmu2_rwdt_cmt"
};

/*
 * rmu2_cmt_fiq_reg_setup: Called on each CPU to set up FIQ handler regs
 * input: unused
 * output: none
 * return: none
 */
static void rmu2_cmt_fiq_reg_setup(void *info)
{
	/* NB - secure code may mean handler doesn't see these registers */
	struct pt_regs r;
	get_fiq_regs(&r);
	r.ARM_sp = (unsigned long) this_cpu_ptr(rmu2_cmt_fiq_regs);
	set_fiq_regs(&r);
}
#endif

/*
 * rmu2_cmt_init_irq: IRQ initialization handler for CMT
 * input: none
 * output: none
 * return: none
 */
static void rmu2_cmt_init_irq(void)
{
	int ret = 0;
	unsigned int irq;
	printk(KERN_INFO "START < %s >\n", __func__);

	irq = gic_spi(CMT15_SPI);
	set_irq_flags(irq, IRQF_VALID | IRQF_NOAUTOEN);
	ret = request_irq(irq, rmu2_cmt_irq, IRQF_DISABLED,
				"CMT15_RWDT0", (void *)irq);
	if (0 > ret) {
		printk(KERN_ERR "%s:%d request_irq failed err=%d\n",
				__func__, __LINE__, ret);
		free_irq(irq, (void *)irq);
	}
#ifdef CONFIG_RMU2_CMT_FIQ
	/* In a secure system, CMT15 will already be set as a FIQ or not
	 * in the GIC by the secure code. Our non-secure writes will be
	 * ignored.
	 */
	{
		/* Poke the GIC to make CMT15 interrupt*/
		/* a FIQ (GICD_IGROUPn / ICDISR0) */
		unsigned int val;
		__raw_writel(__raw_readl(ICD_ISR0+4*(int)(irq/32)) &
			~(1<<(irq%32)), ICD_ISR0+4*(int)(irq/32));
		printk(KERN_DEBUG "< %s > ICD_ISR%d = %08x\n",
		__func__, irq, __raw_readl(ICD_ISR0+4*(int)(irq/32)));

		/* Poke the GIC to send it to*/
		/* both CPUs (GICD_ITARGETSRn / ICDIPTRn) */
		/* (Linux irq_set_affinity API will only send it to one) */
		val = __raw_readl(ICD_IPTR0+4*(int)(irq/4));
		val = (val & ~(0xff << (8*(int)(irq%4)))) |
					(0x03 << (8*(int)(irq%4)));
		__raw_writel(val, ICD_IPTR0+4*(int)(irq/4));
		printk(KERN_ERR
		"< %s > ICD_IPTR%d = %08x\n",
		__func__, irq, __raw_readl(ICD_IPTR0+4*(int)(irq/4)));

		/* Make sure the interrupt is highest*/
		/* (0) priority (GICD_IPRIORITYn / ICDIPRn) */
		__raw_writeb(0, ICD_IPR0+irq);
	}
#endif

	/* Always claim the FIQ vector - the secure code may
	 * or may not pass through the interrupt as a FIQ...
	 *
	 * Note that we rely on the platform preserving FIQ registers across
	 * power management events.
	 */
#ifdef CONFIG_FIQ_GLUE
	ret = fiq_glue_register_handler(&fh);
	if (0 > ret) {
		printk(KERN_ERR "%s:%d fiq_glue_register_handler failed err=%d\n",
				__func__, __LINE__, ret);
		free_irq(irq, (void *)irq);
		return;
	}

#else
	ret = claim_fiq(&fh);
	if (0 > ret) {
		printk(KERN_ERR "%s:%d claim_fiq failed err=%d\n",
				__func__, __LINE__, ret);
		free_irq(irq, (void *)irq);
		return;
	}

	on_each_cpu(rmu2_cmt_fiq_reg_setup, NULL, true);
	set_fiq_handler(&rmu2_cmt_fiq_handler,
			&rmu2_cmt_fiq_handler_end - &rmu2_cmt_fiq_handler);
#endif

	/* And always set up in case it's an IRQ */
	enable_irq(irq);
}
#endif	/* CONFIG_GIC_NS_CMT */

/*
 * rmu2_cmt15_probe:
 * input:
 * @pdev: platform device
 * output:
 * return:
 */
static int __devinit rmu2_cmt_probe(struct platform_device *pdev)
{
	printk(KERN_ALERT "START < %s >\n", __func__);

	cpg_check_init();
#ifdef CONFIG_GIC_NS_CMT
	rmu2_cmt_init_irq();
	rmu2_cmt_start();
#endif  /* CONFIG_GIC_NS_CMT */
	return 0;
}

/*
 * rmu2_cmt15_remove:
 * input:
 * @pdev: platform device
 * output:none
 * return:
 * 0:sucessful
 */
static int __devexit rmu2_cmt_remove(struct platform_device *pdev)
{
	printk(KERN_ALERT "START < %s >\n", __func__);

#ifdef CONFIG_GIC_NS_CMT
	rmu2_cmt_stop();
#endif  /* CONFIG_GIC_NS_CMT */

	return 0;
}

/*
 * rmu2_cmt15_suspend:
 * input:
 * @pdev: platform device
 * @state: power management
 * output:none
 * return:
 * 0:sucessful
 * -EAGAIN: try again
 */
static int rmu2_cmt_suspend_noirq(struct device *dev)
{
	printk(KERN_ALERT "START < %s >\n", __func__);

#ifdef CONFIG_GIC_NS_CMT
	rmu2_cmt_clear();
	rmu2_cmt_stop();
#endif  /* CONFIG_GIC_NS_CMT */

	return 0;
}

/*
 * rmu2_cmt15_resume:
 * input:
 * @pdev: platform device
 * output:none
 * return:
 * 0:sucessful
 * -EAGAIN: try again
 */
static int rmu2_cmt_resume_noirq(struct device *dev)
{
	printk(KERN_ALERT "START < %s >\n", __func__);

#ifdef CONFIG_GIC_NS_CMT
	rmu2_cmt_start();
	rmu2_cmt_clear();
#endif  /* CONFIG_GIC_NS_CMT */

	return 0;
}

static struct dev_pm_ops rmu2_cmt_driver_pm_ops = {
	.suspend_noirq = rmu2_cmt_suspend_noirq,
	.resume_noirq = rmu2_cmt_resume_noirq,
};

static struct platform_driver rmu2_cmt_driver = {
	.probe		= rmu2_cmt_probe,
	.remove		= rmu2_cmt_remove,
	.driver		= {
		.name	= "cmt15",
		.pm	= &rmu2_cmt_driver_pm_ops,
	}
};

#ifdef CONFIG_RWDT_CMT15_TEST
int read_proc(char *buf, char **start, off_t offset,
						int count, int *eof, void *data)
{
	int len = 0;
	len = sprintf(buf, "%u", test_mode);

	return len;
}

int write_proc(struct file *file, const char __user *buf,
						unsigned int count, void *data)
{
	char buffer[4];

	if (count > sizeof buffer)
		return -EFAULT;

	if (copy_from_user(buffer, buf, count))
		return -EFAULT;

	sscanf(buffer, "%u", &test_mode);

	switch (test_mode) {
	case TEST_LOOP:
		loop(0);
		break;
	case TEST_PREEMPT_LOOP:
		preempt_disable();
		loop(0);
		break;
	case TEST_IRQOFF_LOOP:
		loop((void *)1);
		break;
	case TEST_IRQOFF_LOOP_ALL:
		on_each_cpu(loop, (void *)1, false);
		break;
	case TEST_IRQHANDLER_LOOP:
		loop_processor_vector("IRQ", 0x18);
		break;
	case TEST_FIQOFF_LOOP:
		loop((void *)3);
		break;
	case TEST_FIQOFF_1_LOOP_ALL:
		local_fiq_disable();
		on_each_cpu(loop, (void *)1, false);
		break;
	case TEST_FIQOFF_LOOP_ALL:
		on_each_cpu(loop, (void *)3, false);
		break;
	}

	return test_mode;
}

void create_new_proc_entry(void)
{
	proc_watch_entry = create_proc_entry("proc_watch_entry", 0666, NULL);
	if (!proc_watch_entry) {
		printk(KERN_INFO "Error creating proc entry");
		return;
	}
	proc_watch_entry->read_proc = (read_proc_t *)read_proc;
	proc_watch_entry->write_proc = (write_proc_t *)write_proc;
}

#endif

/*
 * init routines
 */
static int __init rmu2_cmt_init(void)
{
	int ret = 0;
	printk(KERN_ALERT "START < %s >\n", __func__);
	ret = platform_device_register(&rmu2_cmt_dev);
	if (0 != ret) {
		printk(KERN_ERR "< %s > platform_device_register() = %d\n",
					__func__, ret);
		return ret;
	}
	ret = platform_driver_register(&rmu2_cmt_driver);
	if (0 > ret) {
		printk(KERN_ERR "< %s > platform_driver_register() = %d\n",
					__func__, ret);
		return ret;
	}

#ifdef CONFIG_RWDT_CMT15_TEST
	create_new_proc_entry();
#endif

	return ret;
}

/*
 * exit routines
 */
static void __exit rmu2_cmt_exit(void)
{
	printk(KERN_ALERT "START < %s >\n", __func__);
	platform_driver_unregister(&rmu2_cmt_driver);
	platform_device_unregister(&rmu2_cmt_dev);
#ifdef CONFIG_RWDT_CMT15_TEST
	remove_proc_entry("proc_watch_entry", NULL);
#endif
}

subsys_initcall(rmu2_cmt_init);
module_exit(rmu2_cmt_exit);

MODULE_AUTHOR("Renesas Mobile Corp.");
MODULE_DESCRIPTION("CMT15 driver for R-Mobile-U2");
MODULE_LICENSE("GPL");
