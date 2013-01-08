/**
 * @note To define ARCH_HAS_READ_CURRENT_TIMER,
 *       it will include arch/arm/mach-shmobile/include/mach/timex.h
 */
#include <linux/timex.h>
#ifdef ARCH_HAS_READ_CURRENT_TIMER
#include <mach/board-u2evm.h>
#include <mach/sh_cmt.h>
#include <mach/r8a73734.h>
#include <linux/spinlock_types.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <asm/io.h>

int read_current_timer(unsigned long *timer_val)
{
	*timer_val = __raw_readl(CMCNT3);
	return 0;
}

int __init setup_current_timer(void)
{
	struct clk *clk = NULL;
	unsigned long lpj = 0, flags = 0;
	int ret = 0;

	clk = clk_get(NULL, "currtimer");
	if (IS_ERR(clk))
		return PTR_ERR(clk);
	ret = clk_enable(clk);
	if (ret < 0)

	{
		pr_err("%s:couldn't enable current timer clock\n", __func__);
		clk_put(clk);
		return ret;
	}

	lpj = clk_get_rate(clk) + HZ/2;
	do_div(lpj, HZ);
	lpj_fine = lpj;

	spin_lock_irqsave(&sh_cmt_lock, flags);
	__raw_writel(__raw_readl(CMCLKE) | (1 << 3), CMCLKE);
	spin_unlock_irqrestore(&sh_cmt_lock, flags);

	__raw_writel(0, CMSTR3);
	__raw_writel(0x10b, CMCSR3); /* Free-running, DBGIVD, CKS=3 */
	__raw_writel(0xffffffff, CMCOR3);
	__raw_writel(0, CMCNT3);
	while (__raw_readl(CMCNT3) != 0)
		cpu_relax();
	__raw_writel(1, CMSTR3);

	pr_info("Current timer started (lpj=%lu)\n", lpj);

	/*
	 * TODO: Current timer (CMT1) MSTP bit vs. Suspend-to-RAM
	 *
	 * We don't have proper suspend/resume operations implemented yet
	 * for the current timer (CMT1), so there is no guarantee that CMT1
	 * module is functional when timer-based udelay() is used.  Thus
	 * we need to enable CMT1 MSTP clock here, and if possible, would
	 * like to leave it enabled forever.
	 *
	 * On the other hand, CMT1 should be halted during Suspend-to-RAM
	 * state to minimize power consumption.
	 *
	 * To solve the problem, we make the following assumptions:
	 *
	 * 1) udelay() is not used from now (time_init()) until
	 *    late_time_init() or calibrate_delay() completes
	 *
	 * 2) timer-based udelay() is functional as long as clocksource is
	 *    available
	 *
	 * and disable CMT1 MSTP clock here not to increment CMT1 usecount.
	 */
	clk_disable(clk);
	clk_put(clk);
	return 0;
}

#endif /* ARCH_HAS_READ_CURRENT_TIMER */
