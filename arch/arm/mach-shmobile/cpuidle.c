/*
 * CPUIdle support code for SH-Mobile ARM
 *
 *  Copyright (C) 2011 Magnus Damm
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */

#include <linux/pm.h>
#include <linux/cpuidle.h>
#include <linux/suspend.h>
#include <linux/module.h>
#include <linux/err.h>
#include <asm/cpuidle.h>
#include <asm/io.h>

static void shmobile_enter_wfi(void)
{
	cpu_do_idle();
}

void (*shmobile_cpuidle_modes[CPUIDLE_STATE_MAX])(void) = {
	shmobile_enter_wfi, /* regular sleep mode */
};

static int shmobile_cpuidle_enter(struct cpuidle_device *dev,
				  struct cpuidle_driver *drv,
				  int index)
{
	shmobile_cpuidle_modes[index]();

	return index;
}

static DEFINE_PER_CPU(struct cpuidle_device, shmobile_cpuidle_dev);
static struct cpuidle_driver shmobile_cpuidle_driver = {
	.name			= "shmobile_cpuidle",
	.owner			= THIS_MODULE,
	.en_core_tk_irqen	= 1,
	.states[0]		= ARM_CPUIDLE_WFI_STATE,
	.safe_state_index	= 0, /* C1 */
	.state_count		= 1,
};

void (*shmobile_cpuidle_setup)(struct cpuidle_driver *drv);

static int shmobile_cpuidle_init(void)
{
	struct cpuidle_device *dev;
	struct cpuidle_driver *drv = &shmobile_cpuidle_driver;
	unsigned int cpu;
	int i, ret;

	for (i = 0; i < CPUIDLE_STATE_MAX; i++)
		drv->states[i].enter = shmobile_cpuidle_enter;

	if (shmobile_cpuidle_setup)
		shmobile_cpuidle_setup(drv);

	ret = cpuidle_register_driver(drv);
	if (ret) {
		pr_err("CPUidle driver registration failed\n");
		return ret;
	}

	for_each_possible_cpu(cpu) {
		dev = &per_cpu(shmobile_cpuidle_dev, cpu);
		dev->cpu = cpu;

		dev->state_count = drv->state_count;
		ret = cpuidle_register_device(dev);
		if (ret) {
			pr_err("CPU%u: CPUidle device registration failed\n", cpu);
			return ret;
		}
	}

	return 0;
}
late_initcall(shmobile_cpuidle_init);
