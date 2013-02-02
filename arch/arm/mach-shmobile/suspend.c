/*
 * Suspend-to-RAM support code for SH-Mobile ARM
 *
 *  Copyright (C) 2011 Magnus Damm
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */

#include <linux/pm.h>
#include <linux/suspend.h>
#include <linux/module.h>
#include <linux/err.h>
#include <asm/io.h>
#include <asm/system_misc.h>

#ifdef CONFIG_PM_DEBUG
/*
 * Dynamic on/off for System Suspend
 *   0: System Suspend is disable
 *   1: System Suspend in enable
 */
static int enable_module = 1;
static DEFINE_SPINLOCK(systemsuspend_lock);
#endif  /* CONFIG_PM_DEBUG */

static int shmobile_suspend_default_enter(suspend_state_t suspend_state)
{
	cpu_do_idle();
	return 0;
}

static int shmobile_suspend_begin(suspend_state_t state)
{
	disable_hlt();
	return 0;
}

static void shmobile_suspend_end(void)
{
	enable_hlt();
}

struct platform_suspend_ops shmobile_suspend_ops = {
	.begin		= shmobile_suspend_begin,
	.end		= shmobile_suspend_end,
	.enter		= shmobile_suspend_default_enter,
	.valid		= suspend_valid_only_mem,
};

#ifdef CONFIG_PM_DEBUG
int control_systemsuspend(int is_enabled)
{
        unsigned long irqflags;
        int cur_state = 0;
        spin_lock_irqsave(&systemsuspend_lock, irqflags);
        cur_state = get_suspend_state();

        if (cur_state == PM_SUSPEND_MEM)
                request_suspend_state(PM_SUSPEND_ON);

        enable_module = is_enabled;
        spin_unlock_irqrestore(&systemsuspend_lock, irqflags);
        return 0;
}
EXPORT_SYMBOL(control_systemsuspend);

int is_systemsuspend_enable(void)
{
        return enable_module;
}
EXPORT_SYMBOL(is_systemsuspend_enable);
#endif /* CONFIG_PM_DEBUG */

static int __init shmobile_suspend_init(void)
{
	suspend_set_ops(&shmobile_suspend_ops);
	return 0;
}
late_initcall(shmobile_suspend_init);
