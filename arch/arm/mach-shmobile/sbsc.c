/*
 * sbsc.c
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
 /*
 * This file manages SBSC HW configuration and exchange of the
 * SBSC frequency with the modem subsystem
 *
 */
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/hwspinlock.h>
#include <linux/io.h>
#include <linux/export.h>

#include <mach/common.h>
#include <mach/hardware.h>
#include <mach/r8a7373.h>
#include <mach/sbsc.h>
#include <mach/pm.h>

static void __iomem *sbsc_reg;

static void shmobile_init_cpg_lock(void);

/* SBSC register address for ZQ calibration */
static struct shared_area *sh_area;

/* Modem/APE parameter sharing */
#define LOCK_TIME_OUT_MS 10

/* static struct hwspinlock *sw_cpg_lock; */
struct hwspinlock *sw_cpg_lock;
static int cpg_lock_init;
static void __iomem *shared_area_base;

/* Flag to check whether changing clock in suspend processing or not */
unsigned int is_suspend_setclock;

static struct resource cpg_sem_res[] = {
	{
		.start  = SHARED_AREA_SBSC_START_PHY,
		.end    = SHARED_AREA_SBSC_END_PHY,
		.flags  = IORESOURCE_MEM,
	},
};

void shmobile_sbsc_init(void)
{
	sbsc_reg = NULL;
	cpg_lock_init = 0;
	if (ES_REV_2_2 > shmobile_chip_rev())
		return;
	sbsc_reg = ioremap(SBSC_BASE, SBSC_SIZE);
	if (!sbsc_reg)
		pr_info(KERN_ERR "failed to remap SBSC registers\n");

	/* Enable the change for Modem */
	cpg_enable_sbsc_change_for_modem();
	is_suspend_setclock = 0;
	shmobile_init_cpg_lock();
}

u32 shmobile_sbsc_read_reg32(u32 offset)
{
	if (!sbsc_reg)
		return 0;

	return ioread32((void __iomem *) (sbsc_reg + offset));
}

void shmobile_sbsc_write_reg32(u32 val, u32 offset)
{
	if (!sbsc_reg)
		return;

	iowrite32(val, (void __iomem *) (sbsc_reg + offset));
}


void shmobile_sbsc_update_param(struct sbsc_param *param)
{
	/* first check values */
	if ((param->SDWCRC0A == 0) ||
	(param->SDWCRC1A == 0) ||
	(param->SDWCRC2A == 0) ||
	(param->SDWCR00A == 0) ||
	(param->SDWCR01A == 0) ||
	(param->SDWCR10A == 0) ||
	(param->SDWCR11A == 0)) {
		/* Don't apply parameters */
	} else {
		shmobile_sbsc_write_reg32(param->SDWCRC0A, SBSC_SDWCRC0A);
		shmobile_sbsc_write_reg32(param->SDWCRC1A, SBSC_SDWCRC1A);
		shmobile_sbsc_write_reg32(param->SDWCRC2A, SBSC_SDWCRC2A);
		shmobile_sbsc_write_reg32(param->SDWCR00A, SBSC_SDWCR00A);
		shmobile_sbsc_write_reg32(param->SDWCR01A, SBSC_SDWCR01A);
		shmobile_sbsc_write_reg32(param->SDWCR10A, SBSC_SDWCR10A);
		shmobile_sbsc_write_reg32(param->SDWCR11A, SBSC_SDWCR11A);
		}
}

static void shmobile_init_sharedarea(struct shared_area *sh)
{
	int i;
	u32 *area = (u32 *)sh;

	/* first, set all the area to 0xCAFEDEAD */
	for (i = 0; i < (SHARED_AREA_SBSC_SIZE / sizeof(u32)); i++)
		*(area + i) = 0xCAFEDEAD;

	/* Fill the shared area for parameters */
	for (i = 0; i < ZB3_FREQ_SIZE; i++)
		memcpy(&(sh->sbsc[i]), &(zb3_lut[i]),
				sizeof(struct sbsc_param));

	/* init APE request at 520 MHz */
	sh->ape_req_freq = 520000;

	/* init PLL reprog field */
	sh->pll_reprogram = 0;
	
	/* init revision */
	sh->revision = SHARED_AREA_REV;

	/* set the value of BBFRQCRD to minimum divider */
	cpg_init_bbfrqcrd();
}

static void shmobile_init_cpg_lock(void)
{
	sw_cpg_lock = hwspin_lock_request_specific(SMGP001);

	if (!sw_cpg_lock) {
		pr_info("%s(): Can't request semaphore SMGP001\n", __func__);
		cpg_lock_init = 0;
		return;
	}
	shared_area_base = ioremap(cpg_sem_res[0].start,
			cpg_sem_res[0].end - cpg_sem_res[0].start);

	if (!shared_area_base) {
		cpg_lock_init = 0;
	} else {
		cpg_lock_init = 1;
		sh_area = (struct shared_area *) (shared_area_base);
		shmobile_init_sharedarea(sh_area);
		sh_area->ape_req_freq = 520000;
		sh_area->bb_req_freq = 520000;
		barrier();
	}

}

/*Shall be called under cpg lock only*/
void shmobile_set_ape_req_freq(unsigned int freq)
{
	/*should add some frequency check*/
	sh_area->ape_req_freq = freq;
	barrier();
}

/*Shall be called under cpg lock only*/
unsigned int shmobile_get_ape_req_freq(void)
{
	return sh_area->ape_req_freq;
}

/*Shall be called under cpg lock only*/
unsigned int shmobile_get_modem_req_freq(void)
{
	return sh_area->bb_req_freq;
}


unsigned int shmobile_get_pll_reprogram(void)
{
	return sh_area->pll_reprogram;

}

static int shmobile_hwspin_lock_timeout(
	struct hwspinlock *hwlock, unsigned int timeout)
{
	int ret;
	unsigned long expire;
	unsigned long cnt;

	expire = timeout * 1000;
	for (cnt = 0; cnt < expire; cnt++) {
		/* Try to take the hwspinlock */
		ret = __hwspin_trylock(hwlock, 0, NULL);
		if (ret == 0)
			return 0;
		/*
		 * -EBUSY, to retry again Wait 1us.
		 */
		udelay(1);
	}
	return -ETIMEDOUT;
}

int shmobile_acquire_cpg_lock(unsigned long *flags)
{
	int ret = 0;
#ifdef ZB3_CLK_DFS_ENABLE
	if (ES_REV_2_2 > shmobile_chip_rev())
		return 0;
	if ((cpg_lock_init == 0) && (r8a7373_hwlock_cpg != NULL))
		shmobile_init_cpg_lock();
	if (cpg_lock_init == 0)
		return 0;

	if (!is_suspend_setclock)
		ret = shmobile_hwspin_lock_timeout(sw_cpg_lock,
							LOCK_TIME_OUT_MS);
	else
		ret = hwspin_trylock_nospin(sw_cpg_lock);

	if (ret < 0)
		pr_info("Can't lock hwlock_cpg\n");

#endif /*ZB3_CLK_DFS_ENABLE*/
	return ret;
}
EXPORT_SYMBOL_GPL(shmobile_acquire_cpg_lock);

int shmobile_release_cpg_lock(unsigned long *flags)
{
#ifdef ZB3_CLK_DFS_ENABLE
	if (cpg_lock_init == 0)
		return 0;

	if (!is_suspend_setclock)
		hwspin_unlock(sw_cpg_lock);
	else
		hwspin_unlock_nospin(sw_cpg_lock);
#endif /*ZB3_CLK_DFS_ENABLE*/
	return 0;
}
EXPORT_SYMBOL_GPL(shmobile_release_cpg_lock);

