/*
 * rmu2_rwdt.c
 *
 * Copyright (C) 2012 Renesas Mobile Corporation
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

#include <linux/rmu2_rwdt.h>
#include <asm/io.h>

#define CONFIG_GIC_NS
#define CONFIG_GIC_NS_CMT

//#define CONFIG_RWDT_DEBUG
#ifdef CONFIG_RWDT_DEBUG
#include <linux/proc_fs.h>

static struct proc_dir_entry *proc_watch_entry;
static int start_stop_cmt_watchdog = 0;
#endif

#define ICD_ISR0 0xF0001080
#define ICD_IPTR0 0xf0001800

static struct delayed_work *dwork = NULL;
static struct workqueue_struct *wq = NULL;
static struct clk *rmu2_rwdt_clk = NULL;
static int unsigned cpu_num = 0;
static unsigned int max_cpus = 0;
static unsigned long cntclear_time = 0;
static int stop_func_flg = 0;
void rmu2_rwdt_software_reset(void);

/*
 * Modify register
 */
static DEFINE_SPINLOCK(io_lock);
#ifdef CONFIG_GIC_NS_CMT
static DEFINE_SPINLOCK(cmt_lock);
#endif	/* CONFIG_GIC_NS_CMT */

void rmu2_modify_register32(unsigned int addr, u32 clear, u32 set)
{
	unsigned long flags;
	u32 val;
	spin_lock_irqsave(&io_lock, flags);
	val = *(volatile u32 *)addr;
	val &= ~clear;
	val |= set;
	*(volatile u32 *)addr = val;
	spin_unlock_irqrestore(&io_lock, flags);
}

static struct resource rmu2_rwdt_resources[] = {
	[0] = {
		.start = RWDT_BASE,
		.end = RWDT_BASE + REG_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= gic_spi(RWDT_SPI),	/* RWDT */
		.flags	= IORESOURCE_IRQ,
	}
};

static struct platform_device rmu2_rwdt_dev = {
	.name = "rwdt",
	.id = - 1,
	.num_resources = ARRAY_SIZE(rmu2_rwdt_resources),
	.resource = rmu2_rwdt_resources
};

/*
 * open API
 */
static int rmu2_rwdt_open(struct inode *inode, struct file *filep)
{
	return 0;
}

/*
 * release API
 */
static int rmu2_rwdt_release(struct inode *inode, struct file *filep)
{
	return 0;
}

/*
 * rmu2_ioctl_rwdt: ioctl API
 * input:
 * 		@*filep: struct file
 * 		@cmd: command
 * 		@arg: other argument
 * output:none
 * return:
 * 		0: sucessful
 * 		-EINVAL: Invalid argument
 */
static long rmu2_ioctl_rwdt(struct file *filep, unsigned int cmd, unsigned long arg)
{
	long ret = 0;

	RWDT_DEBUG( "ioctl_called!!");
	if (cmd == IOCTL_RWDT_SOFT_RESET) {
		rmu2_rwdt_software_reset();
	} else {
		ret = -EINVAL;
	}
	
	return ret;
}

/*
 * Kernel Interfaces
 */
static struct file_operations rwdt_fops = {
	.owner = THIS_MODULE,
	.open = rmu2_rwdt_open,
	.release = rmu2_rwdt_release,
	.unlocked_ioctl = rmu2_ioctl_rwdt,
};

static struct miscdevice rwdt_mdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "rmu2_rwdt",
	.fops = &rwdt_fops,
};

/*
 * cpg_check_init: CPG Check initialization
 * input: none
 * output: none
 * return: none
 */
static void cpg_check_init(void)
{
	__raw_writel(0x3fff3fffU,CPG_CHECK_REG);
	__raw_writel(0x3fff3fffU,CPG_CHECK_REG + 4);
}

/*
 * cpg_check_check: CPG Check
 * input: none
 * output: none
 * return: none
 */
static void cpg_check_check(void)
{
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
			printk(KERN_EMERG " %08x=%08x\n",addr,__raw_readl(addr));
		}
		
		panic("Bus timeout occurred!!");
	} else if ((0x3fff3fffU != (val0 & 0x3fff3fffU)) || 
							(0x3fff3fffU != (val1 & 0x3fff3fffU))) {
		RWDT_DEBUG( "CPG CHECK register was modified and should be reset\n");
		__raw_writel(0x3fff3fffU,CPG_CHECK_REG);
		__raw_writel(0x3fff3fffU,CPG_CHECK_REG + 4);
	} else {
		/* Do nothing */
	}
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
	unsigned long flags;
	
	RWDT_DEBUG( "START < %s >\n", __func__);
	RWDT_DEBUG( "< %s >CMCLKE=%08x\n", __func__, __raw_readl(CMCLKE));
	RWDT_DEBUG( "< %s >CMSTR15=%08x\n", __func__, __raw_readl(CMSTR15));
	RWDT_DEBUG( "< %s >CMCSR15=%08x\n", __func__, __raw_readl(CMCSR15));
	RWDT_DEBUG( "< %s >CMCNT15=%08x\n", __func__, __raw_readl(CMCNT15));
	RWDT_DEBUG( "< %s >CMCOR15=%08x\n", __func__, __raw_readl(CMCOR15));

	spin_lock_irqsave(&cmt_lock, flags);
	__raw_writel(__raw_readl(CMCLKE) | (1<<5), CMCLKE);
	spin_unlock_irqrestore(&cmt_lock, flags);
	
	__raw_writel(0, CMSTR15);
	__raw_writel(0x000001a6U, CMCSR15);	/* Int enable, 32bits operation */
	__raw_writel(0U, CMCNT15);
	__raw_writel(dec2hex(CMT_OVF), CMCOR15);
	__raw_writel(1, CMSTR15);

	RWDT_DEBUG( "< %s >CMCLKE=%08x\n", __func__, __raw_readl(CMCLKE));
	RWDT_DEBUG( "< %s >CMSTR15=%08x\n", __func__, __raw_readl(CMSTR15));
	RWDT_DEBUG( "< %s >CMCSR15=%08x\n", __func__, __raw_readl(CMCSR15));
	RWDT_DEBUG( "< %s >CMCNT15=%08x\n", __func__, __raw_readl(CMCNT15));
	RWDT_DEBUG( "< %s >CMCOR15=%08x\n", __func__, __raw_readl(CMCOR15));
}

/*
 * rmu2_cmt_stop: stop CMT
 * input: none
 * output: none
 * return: none
 */
void rmu2_cmt_stop(void)
{
	__raw_readl(CMCSR15);
	__raw_writel(0x00000186U, CMCSR15);	/* Int disable */
	__raw_writel(0U, CMCNT15);
}

/*
 * rmu2_cmt_clear: CMT counter clear
 * input: none
 * output: none
 * return: none
 */
static void rmu2_cmt_clear(void)
{
#ifdef CONFIG_RWDT_DEBUG
	printk(KERN_DEBUG "clear the CMT counter!!\n");
#endif
	__raw_writel(0U, CMCNT15);
}

/*
 * rmu2_cmt_irq: IRQ handler for CMT
 * input:
 * 		@irq: interrupt number
 * 		@dev_id: device ID
 * output: none
 * return: 
 *		IRQ_HANDLED: irq handled
 */
static irqreturn_t rmu2_cmt_irq(int irq, void *dev_id)
{
	unsigned int reg_val = __raw_readl(CMCSR15);
	
	printk(KERN_ERR "CMT15 Counter Overflow Occur!\n");
	reg_val &= ~0x0000c000U;
	__raw_writel(reg_val, CMCSR15);
	
	return IRQ_HANDLED;
}

/*
 * rmu2_cmt_init_irq: IRQ initialization handler for CMT
 * input: none
 * output: none
 * return: none
 */
static void rmu2_cmt_init_irq(void)
{
	int ret;
	unsigned int irq;

	RWDT_DEBUG( "START < %s >\n", __func__);
	
	irq = gic_spi(CMT15_SPI);
	set_irq_flags(irq, IRQF_VALID | IRQF_NOAUTOEN);
	ret = request_irq(irq, rmu2_cmt_irq, IRQF_DISABLED, "CMT15_RWDT0", (void *)irq);
	if (0 > ret) {
		printk(KERN_ERR "%s:%d request_irq failed err=%d\n",
										__func__, __LINE__, ret);
		free_irq(irq, (void *)irq);
		return;
	}
	
#ifdef CONFIG_GIC_NS
	{
#ifdef CONFIG_RMU2_RWDT_30S
		int i;
		unsigned int val;
		i=CMT15_SPI+32;
		__raw_writel(__raw_readl(ICD_ISR0+4*(int)(i/32)) & ~(1<<(i%32)), ICD_ISR0+4*(int)(i/32));
		printk(KERN_DEBUG "< %s > ICD_ISR%d = %08x\n",__func__,i,__raw_readl(ICD_ISR0+4*(int)(i/32)));

		//DIST to CPU0 & CPU1
		val = __raw_readl(ICD_IPTR0+4*(int)(i/4));
		val = (val & ~(0xff << (8*(int)(i%4)))) | (0x03 << (8*(int)(i%4)));
		__raw_writel(val, ICD_IPTR0+4*(int)(i/4));
		printk(KERN_ERR "< %s > ICD_IPTR%d = %08x\n",__func__,i,__raw_readl(ICD_IPTR0+4*(int)(i/4)));
#endif  /* CONFIG_RMU2_RWDT_30S */
	}
#endif  /* CONFIG_GIC_NS */


	enable_irq(irq);
}
#endif	/* CONFIG_GIC_NS_CMT */

/*
 * rmu2_rwdt_cntclear: RWDT counter clear
 * input: none
 * output: none
 * return:
 * 			0: sucessful
 *			-EAGAIN: try again
 */
int rmu2_rwdt_cntclear(void)
{
	int ret = 0;
	unsigned int base;
	struct resource *r;
	u8 reg8;
	u32 wrflg;

	RWDT_DEBUG( "START < %s >\n", __func__);
	r = platform_get_resource(&rmu2_rwdt_dev, IORESOURCE_MEM, 0);
	if (NULL == r) {
		ret = -ENOMEM;
		printk(KERN_ERR "%s:%d platform_get_resource failed err=%d\n", 
													__func__, __LINE__, ret);
		return ret;
	}
	base = r->start;

	/* check RWTCSRA wrflg */
	reg8 = __raw_readb(base + RWTCSRA);
	wrflg = ((u32)reg8 >> 5) & 0x01U;
	if (0U == wrflg) {
#ifdef CONFIG_RWDT_DEBUG
	printk(KERN_DEBUG "Clear the watchdog counter!!\n");
#endif
		__raw_writel(RESCNT_CLEAR_DATA, base + RWTCNT);
		return 0;
	} else {
		return -EAGAIN; /* try again */
	}
}

/*
 * rmu2_rwdt_stop: stop counter
 * input: none
 * output: none
 * return: none
 */
static int rmu2_rwdt_stop(void)
{
	int ret = 0;
	unsigned int base;
	struct resource *r;
	u8 reg8;
	u32 reg32;

	RWDT_DEBUG( "START < %s >\n", __func__);

	stop_func_flg = 1;
	
	r = platform_get_resource(&rmu2_rwdt_dev, IORESOURCE_MEM, 0);
	if (NULL == r) {
		ret = -ENOMEM;
		printk(KERN_ERR "%s:%d platform_get_resource failed err=%d\n", 
													__func__, __LINE__, ret);
		return ret;
	}
	base = r->start;

	cancel_delayed_work_sync(dwork);
	flush_workqueue(wq);

	/* set 0 to RWTCSRA TME for stopping timer */
	reg8 = __raw_readb(base + RWTCSRA);
	reg32 = (u32)reg8;
	reg32 &= ~(RWTCSRA_TME_MASK);
	reg32 |= RESCSR_HEADER;
	__raw_writel(reg32, base + RWTCSRA);

	ndelay(WRFLG_WAITTIME); /* wait 7RCLK for module stop */

	/* module stop */
	clk_disable(rmu2_rwdt_clk);

	stop_func_flg = 0;
	
	return ret;
}

/*
 * rmu2_rwdt_workfn: work function of RWDT
 * input:
 * 		@work: work struct
 * output: none
 * return: none
 */
static void rmu2_rwdt_workfn(struct work_struct *work)
{
	int ret;

#ifdef CONFIG_RWDT_DEBUG
	if ( start_stop_cmt_watchdog == 1) {
		printk(KERN_DEBUG "Skip to clear RWDT for debug!!\n");
		return;
	}
#endif
	RWDT_DEBUG( "START < %s >\n", __func__);

	cpg_check_check();

#ifdef CONFIG_GIC_NS_CMT
	rmu2_cmt_clear();
#endif	/* CONFIG_GIC_NS_CMT */

	ret = rmu2_rwdt_cntclear();
	if (0 > ret) {
		ret = -EBUSY;			/* cannot write RWTCNT  */
		printk(KERN_ERR "< %s > rmu2_rwdt_cntclear() = %d ->HARDWARE ERROR\n",
																__func__, ret);
	}

	if (0 == stop_func_flg) {	/* do not execute while stop() */
#ifdef CONFIG_SMP
		cpu_num++;				/* change cpu */
		if (cpu_num == max_cpus) {
			cpu_num = 0U;
		}
		queue_delayed_work_on(cpu_num, wq, dwork, cntclear_time);
#else	/* CONFIG_SMP */
		queue_delayed_work(wq, dwork, cntclear_time);
#endif	/* CONFIG_SMP */
	}
}

#ifndef CONFIG_RMU2_RWDT_REBOOT_ENABLE
/*
 * rmu2_rwdt_irq: implement for RWDT's IRQ
 * input:
 * 		@irq: interrupt number
 * 		@dev_id: device ID
 * output: none
 * return: 
 * 			IRQ_HANDLED: irq handled
 */
static irqreturn_t rmu2_rwdt_irq(int irq, void *dev_id)
{
	printk(KERN_ERR "RWDT Counter Overflow Occur!! Start Crush Dump\n");
	/* __raw_readl(IO_ADDRESS(0x00000000)); */
	
	return IRQ_HANDLED;
}

/*
 * rmu2_rwdt_init_irq: initialization handler for RWDT's IRQ
 * input: none
 * output: none
 * return: 
 * 			0: sucessful
 * 			-EINVAL: Invalid argument
 * 			-ENOMEM: Out of memory
 * 			-ENOSYS: Function not implemented
 */
static int rmu2_rwdt_init_irq(void)
{
	int ret = 0;
	unsigned int irq;
	struct resource *r;

	RWDT_DEBUG( "START < %s >\n", __func__);

	r = platform_get_resource(&rmu2_rwdt_dev, IORESOURCE_IRQ, 0);
	if (NULL == r) {
		ret = -ENOMEM;
		printk(KERN_ERR "%s:%d platform_get_resource failed err=%d\n", 
													__func__, __LINE__, ret);
		return ret;
	}
	irq = r->start;
	
	set_irq_flags(irq, IRQF_VALID | IRQF_NOAUTOEN);
	
	ret = request_irq(irq, rmu2_rwdt_irq, IRQF_DISABLED, "RWDT0", (void *)irq);
	if (0 > ret) {
		printk(KERN_ERR "%s:%d request_irq failed err=%d\n", 
													__func__, __LINE__, ret);
		free_irq(irq, (void *)irq);
		
		return ret;
	}
	
	enable_irq(irq);
	
	return ret;
}
#endif /* CONFIG_RMU2_RWDT_REBOOT_ENABLE */

/*
 * rmu2_rwdt_start: start RWDT
 * input: none
 * output: none
 * return: 
 * 			0: sucessful
 * 			-ENOMEM: Out of memory
 * 			-EBUSY: Device or resource busy
 */
static int rmu2_rwdt_start(void)
{
	unsigned int base;
	struct resource *r;
	int ret = 0;
	u8 reg8;
	u16 clockSelect;
	u32 reg32;

	RWDT_DEBUG( "START < %s >\n", __func__);

	r = platform_get_resource(&rmu2_rwdt_dev, IORESOURCE_MEM, 0);
	if (NULL == r) {
		ret = -ENOMEM;
		printk(KERN_ERR "%s:%d platform_get_resource failed err=%d\n", 
													__func__, __LINE__, ret);
		return ret;
	}
	base = r->start;
	cpu_num = DEFAULT_CPU_NUMBER;

	/* set 11 to SYSC RESCNT2 RWD0A for selecting soft power on reset */
#ifdef CONFIG_RMU2_RWDT_REBOOT_ENABLE
	rmu2_modify_register32(SYSC_RESCNT2, 0x00000000, RESCNT2_RWD0A_MASK);
#else	/* CONFIG_RMU2_RWDT_REBOOT_ENABLE */
	rmu2_modify_register32(SYSC_RESCNT2, RESCNT2_RWD0A_MASK, 0x00000000);
#endif	/* CONFIG_RMU2_RWDT_REBOOT_ENABLE */
	/* module stop release */
	clk_enable(rmu2_rwdt_clk);
	
	reg8 = __raw_readb(base + RWTCSRA);
	reg8 &= ~(RWTCSRA_TME_MASK);
	reg32 = RESCSR_HEADER + (u32)reg8;
	__raw_writel(reg32, base + RWTCSRA);
	
	reg8 = __raw_readb(base + RWTCSRA);
	reg8 &= ~(RWTCSRA_WOVF_MASK);
	reg32 = RESCSR_HEADER + (u32)reg8;
	__raw_writel(reg32, base + RWTCSRA);
	
#ifndef CONFIG_RMU2_RWDT_REBOOT_ENABLE
	/* set 1 to WTCSRA WOVFE for overflow interrupt enable */
	reg8 = __raw_readb(base + RWTCSRA);
	reg8 |= RWTCSRA_WOVFE_MASK;
	reg32 = RESCSR_HEADER + (u32)reg8;
	__raw_writel(reg32, base + RWTCSRA);
#endif	/* CONFIG_RMU2_RWDT_REBOOT_ENABLE */

	clockSelect = CONFIG_RMU2_RWDT_OVF;

	reg8 = __raw_readb(base + RWTCSRA);
	reg8 &= ~RWTCSRA_CSK0_MASK;
	reg8 |= ((clockSelect >> 8) & 0x00FFU);
	reg32 = RESCSR_HEADER + (u32)reg8;
	__raw_writel(reg32, base + RWTCSRA);

	reg8 = (clockSelect & 0x00FFU);
	reg32 = RESCSR_HEADER + (u32)reg8;
	__raw_writel(reg32, base + RWTCSRB);

	/* clear RWDT counter */
	ret = rmu2_rwdt_cntclear();

	if (0 > ret) {
		/* retry */
		printk(KERN_ERR "< %s > rmu2_rwdt_cntclear() = %d ->Try again\n", 
															__func__, ret);
		ndelay(WRFLG_WAITTIME); /* 7RCLK wait for counter clear */
		ret = rmu2_rwdt_cntclear();

		if (0 > ret) {
			ret = -EBUSY; /* cannot write RWTCNT  */
			printk(KERN_ERR "< %s > rmu2_rwdt_cntclear() = %d ->HARDWARE ERROR\n", 
															__func__, ret);

			/* module stop */
			clk_disable(rmu2_rwdt_clk);

			return ret;
		}
	}

	/* start soft timer */
#ifdef CONFIG_SMP
	queue_delayed_work_on(cpu_num, wq, dwork, cntclear_time);
#else	/* CONFIG_SMP */
	queue_delayed_work(wq, dwork, cntclear_time);
#endif	/* CONFIG_SMP */
	
	/* set 1 to RWTCSRA TME for starting timer */
	reg8 = __raw_readb(base + RWTCSRA);
	reg8 |= RWTCSRA_TME_MASK;
	reg32 = RESCSR_HEADER + (u32)reg8;
	__raw_writel(reg32, base + RWTCSRA);
	
	return ret;
}

/*
 * rmu2_rwdt_probe:
 * input:
 * 		@pdev: platform device
 * output:
 * return:
 */
static int __devinit rmu2_rwdt_probe(struct platform_device *pdev)
{
	int ret = 0;
	unsigned int base;
	u8 reg8;
	u32 reg32;
	struct resource *r;

	RWDT_DEBUG( "START < %s >\n", __func__);

	r = platform_get_resource(&rmu2_rwdt_dev, IORESOURCE_MEM, 0);
	if (NULL == r) {
		ret = -ENOMEM;
		printk(KERN_ERR "%s:%d platform_get_resource failed err=%d\n", 
													__func__, __LINE__, ret);
		goto clk_get_err;
	}
	base = r->start;

	rmu2_rwdt_clk = clk_get(NULL,"rwdt0");
	if (true == IS_ERR(rmu2_rwdt_clk)) {
		ret = PTR_ERR(rmu2_rwdt_clk);
		printk(KERN_ERR "< %s > ret = %d clk_get err\n", __func__, ret);
		goto clk_get_err;
	}

	/* module stop release */
	clk_enable(rmu2_rwdt_clk);

	/* set 0 to WTCSRA WOVF for clearing WOVF */
	reg8 = __raw_readb(base + RWTCSRA);
	reg32 = (u32)reg8;
	reg32 &= ~(RWTCSRA_WOVF_MASK);
	reg32 |= RESCSR_HEADER;
	__raw_writel(reg32, base + RWTCSRA);

	/* module stop */
	clk_disable(rmu2_rwdt_clk);

	stop_func_flg = 0;

	max_cpus = num_online_cpus();
	if (1U > max_cpus) {
		ret = -ENXIO;
		printk(KERN_ERR "< %s > ret = %d num_online_cpus err\n", __func__, ret);
		goto num_online_cpus_err;
	}
	/* set counter clear time */
	cntclear_time = msecs_to_jiffies(CONFIG_RMU2_RWDT_CLEARTIME);

    wq = create_workqueue("rmu2_rwdt_queue");
	if (NULL == wq) {
		ret = -ENOMEM;
		printk(KERN_ERR "< %s > ret = %d create_workqueue err\n", __func__, ret);
		goto create_workqueue_err;
	}

	dwork = kzalloc(sizeof(*dwork), GFP_KERNEL);
	if (NULL == dwork) {
		ret = -ENOMEM;
		printk(KERN_ERR "< %s > ret = %d dwork kzalloc err\n", __func__, ret);
		goto dwork_err; 
	}

	INIT_DELAYED_WORK(dwork, rmu2_rwdt_workfn);
	
	/* Device Registration */
	ret = misc_register(&rwdt_mdev);
	if (0 > ret) {
		return ret;
	}

	cpg_check_init();

#ifdef CONFIG_GIC_NS_CMT
	rmu2_cmt_init_irq();
	rmu2_cmt_start();
#endif	/* CONFIG_GIC_NS_CMT */

#ifndef CONFIG_RMU2_RWDT_REBOOT_ENABLE
	ret = rmu2_rwdt_init_irq();
#endif /* CONFIG_RMU2_RWDT_REBOOT_ENABLE */
	
	if (0 <= ret) {
		ret = rmu2_rwdt_start();
    	if (0 <= ret) {
		return ret; 
		}
	}

	kfree(dwork);
dwork_err:
	flush_workqueue(wq);
	destroy_workqueue(wq);
create_workqueue_err:
num_online_cpus_err:
	clk_put(rmu2_rwdt_clk);
clk_get_err:

	return ret;
}

/*
 * rmu2_rwdt_remove:
 * input:
 * 		@pdev: platform device
 * output:none
 * return:
 * 		0:sucessful
 */
static int __devexit rmu2_rwdt_remove(struct platform_device *pdev)
{
	RWDT_DEBUG( "START < %s >\n", __func__);
	
	rmu2_rwdt_stop();
#ifdef CONFIG_GIC_NS_CMT
	rmu2_cmt_stop();
#endif	/* CONFIG_GIC_NS_CMT */
	kfree(dwork);
	destroy_workqueue(wq);
	
	return 0;
}

/*
 * rmu2_rwdt_suspend:
 * input:
 * 		@pdev: platform device
 * 		@state: power management
 * output:none
 * return:
 * 		0:sucessful
 *		-EAGAIN: try again
 */
static int rmu2_rwdt_suspend(struct platform_device *pdev, pm_message_t state)
{
	int ret;
	RWDT_DEBUG( "START < %s >\n", __func__);
	
#ifdef CONFIG_GIC_NS_CMT
	rmu2_cmt_clear();
#endif	/* CONFIG_GIC_NS_CMT */

	/* clear RWDT counter */
	ret = rmu2_rwdt_cntclear();

	if (0 > ret) {
		/* retry */
		printk(KERN_ERR "< %s > rmu2_rwdt_cntclear() =  %d ->Try again\n", 
																__func__, ret);
		ndelay(WRFLG_WAITTIME); /* 7RCLK wait for counter clear */
		ret = rmu2_rwdt_cntclear();

		if (0 > ret) {
			ret = -EBUSY; /* cannot write RWTCNT */
			printk(KERN_ERR "< %s > rmu2_rwdt_cntclear() = %d ->HARDWARE ERROR\n", 
																__func__, ret);
			return ret;
		}
	}

	cancel_delayed_work_sync(dwork);
	flush_workqueue(wq);

	return 0;
}

/*
 * rmu2_rwdt_resume:
 * input:
 * 		@pdev: platform device
 * output:none
 * return:
 * 		0:sucessful
 *		-EAGAIN: try again
 */
static int rmu2_rwdt_resume(struct platform_device *pdev)
{
	int ret;
	RWDT_DEBUG( "START < %s >\n", __func__);
	
#ifdef CONFIG_GIC_NS_CMT
	rmu2_cmt_clear();
#endif	/* CONFIG_GIC_NS_CMT */

	/* clear RWDT counter */
	ret = rmu2_rwdt_cntclear();

	if (0 > ret) {
		/* retry */
		printk(KERN_ERR "< %s > rmu2_rwdt_cntclear() =  %d ->Try again\n", 
																__func__, ret);	
		ndelay(WRFLG_WAITTIME); /* 7RCLK wait for counter clear */
		ret = rmu2_rwdt_cntclear();

		if (0 > ret) {
			ret = -EBUSY; /* cannot write RWTCNT */
			printk(KERN_ERR "< %s > rmu2_rwdt_cntclear() = %d ->HARDWARE ERROR\n", 
																__func__, ret);
			return ret;
		}
	}

	/* start soft timer */
#ifdef CONFIG_SMP
	queue_delayed_work_on(cpu_num, wq, dwork, cntclear_time);
#else	/* CONFIG_SMP */
	queue_delayed_work(wq, dwork, cntclear_time);
#endif	/* CONFIG_SMP */

	return 0;
}

static struct platform_driver rmu2_rwdt_driver = {
	.probe		= rmu2_rwdt_probe,
	.remove		= rmu2_rwdt_remove,
	.suspend	= rmu2_rwdt_suspend,
	.resume		= rmu2_rwdt_resume,

	.driver		= {
		.name	= "rwdt",
	}

};

#ifdef CONFIG_RWDT_DEBUG

int read_proc(char *buf,char **start,off_t offset,int count,int *eof,void *data )
{
	int len=0;
	len = sprintf(buf,"%x",start_stop_cmt_watchdog);

	return len;
}

int write_proc(struct file *file,const char __user *buf,unsigned int count,void *data )
{
	char buffer[4];

	if(count > sizeof(start_stop_cmt_watchdog))
		return -EFAULT;

   
	if(copy_from_user(buffer, buf, count))
		return -EFAULT;

	sscanf(buffer,"%x",&start_stop_cmt_watchdog);

	return start_stop_cmt_watchdog;
}

void create_new_proc_entry(void)
{
	proc_watch_entry = create_proc_entry("proc_watch_entry",0666,NULL);
	if(!proc_watch_entry) {
	printk(KERN_INFO "Error creating proc entry");
	return;
	}
	proc_watch_entry->read_proc = (read_proc_t *)read_proc ;
	proc_watch_entry->write_proc =(write_proc_t *)write_proc;
}

#endif
/*
 * init routines
 */
static int __init rmu2_rwdt_init(void)
{
	int ret;

	RWDT_DEBUG( "START < %s >\n", __func__);

	ret = platform_device_register(&rmu2_rwdt_dev);
	if (0 > ret) {
		printk(KERN_ERR "< %s > platform_device_register() = %d\n", __func__, ret);
		return ret;
	}
	
	ret = platform_driver_register(&rmu2_rwdt_driver);
	if (0 > ret) {
		printk(KERN_ERR "< %s > platform_driver_register() = %d\n", __func__, ret);
		return ret;
	}

#ifdef CONFIG_RWDT_DEBUG
	create_new_proc_entry();
#endif

	return ret;
}

/*
 * exit routines
 */
static void __exit rmu2_rwdt_exit(void)
{
	RWDT_DEBUG( "START < %s >\n",__func__);
	
	platform_driver_unregister(&rmu2_rwdt_driver);
	platform_device_unregister(&rmu2_rwdt_dev);
#ifdef CONFIG_RWDT_DEBUG
	remove_proc_entry("proc_watch_entry",NULL);
#endif
}

/*
 * rmu2_rwdt_software_reset: software reset
 * input:none
 * output:none
 * return:none
 */
void rmu2_rwdt_software_reset(void)
{
	u8 reg = 0;
	/* set 0x22 to STBCHRB1(0xE6180041) */
	/* __raw_writeb(0x22, (unsigned long)0xE6180041); */

	reg = __raw_readb(STBCHR2); /* read STBCHR2 for debug */
	__raw_writeb((reg | APE_RESETLOG_RWDT_SOFTWARE_RESET), STBCHR2); /* write STBCHR2 for debug */
	/* execute software reset by setting 0x80000000 to RESCNT2 */
	rmu2_modify_register32(SYSC_RESCNT2, RESCNT2_PRES_MASK, RESCNT2_PRES_MASK);
}

module_init(rmu2_rwdt_init);
module_exit(rmu2_rwdt_exit);

MODULE_AUTHOR("Renesas Mobile Corp.");
MODULE_DESCRIPTION("RWDT driver for R-Mobile-U2");
MODULE_LICENSE("GPL");
