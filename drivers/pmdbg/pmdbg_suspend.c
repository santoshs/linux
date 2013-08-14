/*
 * pmdbg_suspend.c
 *
 * Copyright (C) 2011-2012 Renesas Mobile Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/pm.h>
#include <linux/regulator/driver.h>

#define PMDBG_ENABLE_PMIC_D2153

#ifdef PMDBG_ENABLE_PMIC_D2153
#include <linux/d2153/pmic.h>
#include <linux/d2153/d2153_reg.h>
#include <linux/d2153/hwmon.h>
#include <linux/d2153/core.h>
#endif /* PMDBG_ENABLE_PMIC_D2153 */

#include "pmdbg_suspend.h"
#include "pmdbg_hw.h"

static char pmdbg_enable_dump_suspend;
static char suspend_buf[1024];

LOCAL_DECLARE_MOD(suspend, suspend_init, suspend_exit);

DECLARE_CMD(suspend, suspend_cmd);
DECLARE_CMD(wakelock, wakelock_cmd);
DECLARE_CMD(enable_dump_suspend, enable_dump_suspend_cmd);
DECLARE_CMD(disable_dump_suspend, disable_dump_suspend_cmd);

#ifdef PMDBG_ENABLE_PMIC_D2153
struct pmic_table {
	const char	*name;
	unsigned char	reg;
	unsigned char	mctl_reg;
};

static struct pmic_table pmic_reg_tbl[D2153_NUMBER_OF_REGULATORS] = {
	{
		.name		= "D2153_BUCK_1",
		.reg		= D2153_BUCK2PH_BUCK1_REG,
		.mctl_reg	= D2153_BUCK1_MCTL_REG,
	},
	{
		.name		= "D2153_BUCK_2",
		.reg		= D2153_BUCKCORE_BUCK2_REG,
		.mctl_reg	= D2153_BUCK2_MCTL_REG,
	},
	{
		.name		= "D2153_BUCK_3",
		.reg		= D2153_BUCKPRO_BUCK3_REG,
		.mctl_reg	= D2153_BUCK3_MCTL_REG,
	},
	{
		.name		= "D2153_BUCK_4",
		.reg		= D2153_BUCKMEM_BUCK4_REG,
		.mctl_reg	= D2153_BUCK4_MCTL_REG,
	},
	{
		.name		= "D2153_BUCK_5",
		.reg		= D2153_BUCKPERI_BUCK5_REG,
		.mctl_reg	= D2153_BUCK5_MCTL_REG,
	},
	{
		.name		= "D2153_BUCK_6",
		.reg		= D2153_BUCKRF_CONF_REG,
		.mctl_reg	= D2153_BUCK_RF_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_1",
		.reg		= D2153_LDO1_REG,
		.mctl_reg	= D2153_LDO1_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_2",
		.reg		= D2153_LDO2_REG,
		.mctl_reg	= D2153_LDO2_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_3",
		.reg		= D2153_LDO3_REG,
		.mctl_reg	= D2153_LDO3_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_4",
		.reg		= D2153_LDO4_REG,
		.mctl_reg	= D2153_LDO4_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_5",
		.reg		= D2153_LDO5_REG,
		.mctl_reg	= D2153_LDO5_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_6",
		.reg		= D2153_LDO6_REG,
		.mctl_reg	= D2153_LDO6_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_7",
		.reg		= D2153_LDO7_REG,
		.mctl_reg	= D2153_LDO7_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_8",
		.reg		= D2153_LDO8_REG,
		.mctl_reg	= D2153_LDO8_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_9",
		.reg		= D2153_LDO9_REG,
		.mctl_reg	= D2153_LDO9_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_10",
		.reg		= D2153_LDO10_REG,
		.mctl_reg	= D2153_LDO10_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_11",
		.reg		= D2153_LDO11_REG,
		.mctl_reg	= D2153_LDO11_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_12",
		.reg		= D2153_LDO12_REG,
		.mctl_reg	= D2153_LDO12_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_13",
		.reg		= D2153_LDO13_REG,
		.mctl_reg	= D2153_LDO13_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_14",
		.reg		= D2153_LDO14_REG,
		.mctl_reg	= D2153_LDO14_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_15",
		.reg		= D2153_LDO15_REG,
		.mctl_reg	= D2153_LDO15_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_16",
		.reg		= D2153_LDO16_REG,
		.mctl_reg	= D2153_LDO16_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_17",
		.reg		= D2153_LDO17_REG,
		.mctl_reg	= D2153_LDO17_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_18",
		.reg		= D2153_LDO18_LDO_VRFANA_REG,
		.mctl_reg	= D2153_LDO18_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_19",
		.reg		= D2153_LDO19_LDO_19_REG,
		.mctl_reg	= D2153_LDO19_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_20",
		.reg		= D2153_LDO20_LDO_20_REG,
		.mctl_reg	= D2153_LDO20_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_AUD1(LDO_21)",
		.reg		= D2153_LDO21_LDO_AUD1_REG,
		.mctl_reg	= D2153_LDO21_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_AUD2(LDO_22)",
		.reg		= D2153_LDO22_LDO_AUD2_REG,
		.mctl_reg	= D2153_LDO22_MCTL_REG,
	},
};
#endif /* PMDBG_ENABLE_PMIC_D2153 */

/* suspend [type]
 * type:
 * - force: force suspend (skip early suspend)
 * */
static int suspend_cmd(char *para, int size)
{
	int ret = 0;
	char item[PAR_SIZE];
	int para_sz = 0;
	int pos = 0;
	struct timeval beforeTime;
	struct timeval afterTime;
	int suspend_time;
	FUNC_MSG_IN;
	para = strim(para);
	ret = get_word(para, size, 0, item, &para_sz);
	pos = ret;
	if (para_sz > 0) {
		ret = strncmp(item, "force", sizeof("force"));
		if (0 == ret) {
			do_gettimeofday(&beforeTime);
			ret = suspend_devices_and_enter(PM_SUSPEND_MEM);
		} else {
			ret = -ENOTSUPP;
			MSG_INFO("No supported");
			goto fail;
		}
	} else {
		do_gettimeofday(&beforeTime);
		ret = pm_suspend(PM_SUSPEND_MEM);
	}
	do_gettimeofday(&afterTime);
	suspend_time = (afterTime.tv_sec -
			beforeTime.tv_sec) * 1000000
			+ (afterTime.tv_usec - beforeTime.tv_usec);

	MSG_INFO("Suspended in %12uus", suspend_time);
fail:
	FUNC_MSG_RET(ret);
}


/* wakelock
 * - no parameter: Display list of active wakelock to console
 * */
static int wakelock_cmd(char *para, int size)
{
	int ret = 0;
	FUNC_MSG_IN;

#ifdef CONFIG_ARCH_R8A7373
	ret = has_wake_lock_no_expire(WAKE_LOCK_SUSPEND);
	if (ret == 0)
		MSG_INFO("No active suspend wakelock");
#else /*!CONFIG_ARCH_R8A7373*/
	ret = has_wake_lock(WAKE_LOCK_SUSPEND);
	if (ret == 0)
		MSG_INFO("No active suspend wakelock");
#endif /*CONFIG_ARCH_R8A7373*/

	FUNC_MSG_RET(0);
}

/* enable_dump_suspend
 * - no parameter:
 * */
static int enable_dump_suspend_cmd(char *para, int size)
{
	char *s = suspend_buf;

	FUNC_MSG_IN;

	pmdbg_enable_dump_suspend = 1;
	s += sprintf(s, "Suspended: dump suspend log output Enabled");
	MSG_INFO("%s", suspend_buf);

	FUNC_MSG_RET(0);
}

/* disable_dump_suspend
 * - no parameter:
 * */
static int disable_dump_suspend_cmd(char *para, int size)
{
	char *s = suspend_buf;

	FUNC_MSG_IN;

	pmdbg_enable_dump_suspend = 0;
	s += sprintf(s, "Suspended: dump suspend log output Disabled");
	MSG_INFO("%s", suspend_buf);

	FUNC_MSG_RET(0);
}

static int suspend_init(void)
{
	FUNC_MSG_IN;
	ADD_CMD(suspend, suspend);
	ADD_CMD(suspend, wakelock);
	ADD_CMD(suspend, enable_dump_suspend);
	ADD_CMD(suspend, disable_dump_suspend);
	pmdbg_enable_dump_suspend = 0;

	FUNC_MSG_RET(0);
}

static void suspend_exit(void)
{
	FUNC_MSG_IN;
	DEL_CMD(suspend, suspend);
	DEL_CMD(suspend, wakelock);
	DEL_CMD(suspend, enable_dump_suspend);
	DEL_CMD(suspend, disable_dump_suspend);

	FUNC_MSG_OUT;
}

void pmdbg_dump_suspend(void)
{
	CPG_dump_suspend();
	SYSC_dump_suspend();
	GPIO_dump_suspend();
	Other_dump_suspend();

	return;
}

/* Why not print the name, rather than the phys addr, like this? */
/*define sprint_regm(s,p,v,w) (s += sprintf(s, #v ": 0x%x\n", rreg##w(v)))*/
#define sprint_regm(s,p,v,w) (s += sprintf(s, "0x%x: 0x%x\n", p, rreg##w(v)))
#define sprint_reg(s,v) sprint_regm(s,IO_TO_PHYS(v),v,32)
#define sprint_reg8(s,v) sprint_regm(s,IO_TO_PHYS(v),v,8)

static void CPG_dump_suspend(void)
{
	void __iomem *vir_addr = NULL;
	char *s = suspend_buf;

	sprintf(suspend_buf, ">> %s", __func__);
	MSG_INFO("%s", suspend_buf);

	vir_addr = ioremap(SBSC_Freq_APE, 0x8);
	if (!vir_addr) {
		sprintf(suspend_buf, "FAILED");
		goto fail;
	}

	sprint_reg(s, FRQCRA);
	sprint_reg(s, FRQCRB);
	sprint_reg(s, FRQCRD);
	sprint_regm(s, SBSC_Freq_APE, vir_addr, 32);
	sprint_regm(s, SBSC_Freq_BB, vir_addr + 0x4, 32);
	sprint_reg(s, BBFRQCRD);
	sprint_reg(s, VCLKCR1);
	sprint_reg(s, VCLKCR2);
	sprint_reg(s, VCLKCR3);
	sprint_reg(s, VCLKCR4);
	sprint_reg(s, VCLKCR5);
	sprint_reg(s, ZBCKCR);
	sprint_reg(s, SD0CKCR);
	sprint_reg(s, SD1CKCR);
	sprint_reg(s, SD2CKCR);
	sprint_reg(s, FSIACKCR);
	sprint_reg(s, FSIBCKCR);
	sprint_reg(s, MPCKCR);
	sprint_reg(s, SPUACKCR);
	MSG_INFO("%s, s", suspend_buf);

	/* Done until here */
	s = suspend_buf;
	sprint_reg(s, SPUVCKCR);
	sprint_reg(s, SLIMBCKCR);
	sprint_reg(s, HSICKCR);
	sprint_reg(s, M4CKCR);
	sprint_reg(s, CPG_DSITCKCR);
	sprint_reg(s, CPG_DSI0PCKCR);
	sprint_reg(s, CPG_DSI1PCKCR);
	sprint_reg(s, CPG_DSI0PHYCR);
	sprint_reg(s, CPG_DSI1PHYCR);
	sprint_reg(s, MPMODE);
	sprint_reg(s, PLLECR);
	sprint_reg(s, PLL0CR);
	sprint_reg(s, PLL1CR);
	sprint_reg(s, CPG_PLL2CR);
	sprint_reg(s, PLL22CR);
	sprint_reg(s, PLL3CR);
	sprint_reg(s, PLL0STPCR);
	sprint_reg(s, PLL1STPCR);
	sprint_reg(s, PLL2STPCR);
	sprint_reg(s, PLL22STPCR);
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;
	sprint_reg(s, PLL3STPCR);
	sprint_reg(s, MSTPSR0);
	sprint_reg(s, MSTPSR1);
	sprint_reg(s, MSTPSR2);
	sprint_reg(s, MSTPSR3);
	sprint_reg(s, MSTPSR4);
	sprint_reg(s, MSTPSR5);
	sprint_reg(s, MSTPSR6);
	sprint_reg(s, RMSTPCR0);
	sprint_reg(s, RMSTPCR1);
	sprint_reg(s, RMSTPCR2);
	sprint_reg(s, RMSTPCR3);
	sprint_reg(s, RMSTPCR4);
	sprint_reg(s, RMSTPCR5);
	sprint_reg(s, RMSTPCR6);

	sprint_reg(s, SMSTPCR0);
	sprint_reg(s, SMSTPCR1);
	sprint_reg(s, SMSTPCR2);
	sprint_reg(s, SMSTPCR3);
	sprint_reg(s, SMSTPCR4);
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;
	sprint_reg(s, SMSTPCR5);
	sprint_reg(s, SMSTPCR6);
	sprint_reg(s, MMSTPCR0);
	sprint_reg(s, MMSTPCR1);
	sprint_reg(s, MMSTPCR2);
	sprint_reg(s, MMSTPCR3);
	sprint_reg(s, MMSTPCR4);
	sprint_reg(s, MMSTPCR5);
	sprint_reg(s, MMSTPCR6);
	sprint_reg(s, CKSCR);
	sprint_reg(s, PCLKCR);
	sprint_reg(s, ZDIVCR5); /* You may have to change this */
	MSG_INFO("%s", suspend_buf);

	iounmap(vir_addr);

	return;

fail:
	MSG_INFO("%s", suspend_buf);
	return;
}

static void SYSC_dump_suspend(void)
{
	char *s = suspend_buf;

	sprintf(suspend_buf, ">> %s", __func__);
	MSG_INFO("%s", suspend_buf);

	sprint_reg(s, WUPRMSK);
	sprint_reg(s, WUPSMSK);
	sprint_reg(s, WUPMMSK);
	sprint_reg(s, C4POWCR);
	sprint_reg(s, SYSC_PSTR);
	sprint_reg(s, SYSC_LPMWUMON);
	sprint_reg(s, EXSTMON2);
	sprint_reg(s, EXSTMON1);
	sprint_reg(s, SYSC_LPMWUMSKMON);
	sprint_reg(s, SYSC_WUPRFAC);
	sprint_reg(s, SYSC_WUPSFAC);
	sprint_reg(s, SYSC_WUPMFAC);
	sprint_reg(s, SYSC_RWBCR);
	sprint_reg(s, SYSC_SWBCR);
	sprint_reg(s, LPMWUCNT);
	sprint_reg(s, LPMWUMSKCNT);
	sprint_reg(s, APSCSTP);
	sprint_reg(s, SYCKENMSK);
	sprint_reg(s, PDNSEL);
	sprint_reg(s, SYSC_WUPRCR);
	sprint_reg(s, SYSC_WUPSCR);
	MSG_INFO("%s", suspend_buf);

	return;
}

static void GPIO_dump_suspend(void)
{
	char *s = suspend_buf;

	sprintf(suspend_buf, ">> %s", __func__);
	MSG_INFO("%s", suspend_buf);

	sprint_reg8(s, GPIO_PORT0_CR);
	sprint_reg8(s, GPIO_PORT1_CR);
	sprint_reg8(s, GPIO_PORT2_CR);
	sprint_reg8(s, GPIO_PORT3_CR);
	sprint_reg8(s, GPIO_PORT4_CR);
	sprint_reg8(s, GPIO_PORT5_CR);
	sprint_reg8(s, GPIO_PORT6_CR);
	sprint_reg8(s, GPIO_PORT7_CR);
	sprint_reg8(s, GPIO_PORT8_CR);
	sprint_reg8(s, GPIO_PORT9_CR);
	sprint_reg8(s, GPIO_PORT10_CR);
	sprint_reg8(s, GPIO_PORT11_CR);
	sprint_reg8(s, GPIO_PORT12_CR);
	sprint_reg8(s, GPIO_PORT13_CR);
	sprint_reg8(s, GPIO_PORT14_CR);
	sprint_reg8(s, GPIO_PORT15_CR);
	sprint_reg8(s, GPIO_PORT16_CR);
	sprint_reg8(s, GPIO_PORT17_CR);
	sprint_reg8(s, GPIO_PORT18_CR);
	sprint_reg8(s, GPIO_PORT19_CR);
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;
	sprint_reg8(s, GPIO_PORT20_CR);
	sprint_reg8(s, GPIO_PORT21_CR);
	sprint_reg8(s, GPIO_PORT22_CR);
	sprint_reg8(s, GPIO_PORT23_CR);
	sprint_reg8(s, GPIO_PORT24_CR);
	sprint_reg8(s, GPIO_PORT25_CR);
	sprint_reg8(s, GPIO_PORT26_CR);
	sprint_reg8(s, GPIO_PORT27_CR);
	sprint_reg8(s, GPIO_PORT28_CR);
	sprint_reg8(s, GPIO_PORT29_CR);
	sprint_reg8(s, GPIO_PORT30_CR);
	sprint_reg8(s, GPIO_PORT31_CR);
	sprint_reg8(s, GPIO_PORT32_CR);
	sprint_reg8(s, GPIO_PORT33_CR);
	sprint_reg8(s, GPIO_PORT34_CR);
	sprint_reg8(s, GPIO_PORT35_CR);
	sprint_reg8(s, GPIO_PORT36_CR);
	sprint_reg8(s, GPIO_PORT37_CR);
	sprint_reg8(s, GPIO_PORT38_CR);
	sprint_reg8(s, GPIO_PORT39_CR);
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;

	sprint_reg8(s, GPIO_PORT80_CR);
	sprint_reg8(s, GPIO_PORT81_CR);
	sprint_reg8(s, GPIO_PORT82_CR);
	sprint_reg8(s, GPIO_PORT83_CR);
	sprint_reg8(s, GPIO_PORT84_CR);
	sprint_reg8(s, GPIO_PORT85_CR);
	sprint_reg8(s, GPIO_PORT86_CR);
	sprint_reg8(s, GPIO_PORT87_CR);
	sprint_reg8(s, GPIO_PORT88_CR);
	sprint_reg8(s, GPIO_PORT89_CR);
	sprint_reg8(s, GPIO_PORT219_CR);
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;

	sprint_reg8(s, GPIO_PORT90_CR);
	sprint_reg8(s, GPIO_PORT91_CR);
	sprint_reg8(s, GPIO_PORT215_CR);
	sprint_reg8(s, GPIO_PORT216_CR);
	sprint_reg8(s, GPIO_PORT273_CR);
	sprint_reg8(s, GPIO_PORT274_CR);
	sprint_reg8(s, GPIO_PORT275_CR);
	sprint_reg8(s, GPIO_PORT276_CR);
	sprint_reg8(s, GPIO_PORT277_CR);
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;

	sprint_reg8(s, GPIO_PORT261_CR);
	sprint_reg8(s, GPIO_PORT263_CR);
	sprint_reg8(s, GPIO_PORT264_CR);
	sprint_reg8(s, GPIO_PORT265_CR);
	sprint_reg8(s, GPIO_PORT311_CR);
	sprint_reg8(s, GPIO_PORT312_CR);
	sprint_reg8(s, GPIO_PORT218_CR);
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;

	sprint_reg8(s, GPIO_PORT137_CR);
	sprint_reg8(s, GPIO_PORT138_CR);
	sprint_reg8(s, GPIO_PORT140_CR);
	sprint_reg8(s, GPIO_PORT198_CR);
	sprint_reg8(s, GPIO_PORT199_CR);
	sprint_reg8(s, GPIO_PORT200_CR);
	sprint_reg8(s, GPIO_PORT201_CR);
	sprint_reg8(s, GPIO_PORT260_CR);
	sprint_reg8(s, GPIO_PORT262_CR);
	sprint_reg8(s, GPIO_PORT266_CR);
	sprint_reg8(s, GPIO_PORT267_CR);
	sprint_reg8(s, GPIO_PORT268_CR);
	sprint_reg8(s, GPIO_PORT269_CR);
	sprint_reg8(s, GPIO_PORT270_CR);
	sprint_reg8(s, GPIO_PORT271_CR);
	sprint_reg8(s, GPIO_PORT272_CR);
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;
	sprint_reg8(s, GPIO_PORT288_CR);
	sprint_reg8(s, GPIO_PORT289_CR);
	sprint_reg8(s, GPIO_PORT290_CR);
	sprint_reg8(s, GPIO_PORT291_CR);
	sprint_reg8(s, GPIO_PORT292_CR);
	sprint_reg8(s, GPIO_PORT293_CR);
	sprint_reg8(s, GPIO_PORT294_CR);
	sprint_reg8(s, GPIO_PORT295_CR);
	sprint_reg8(s, GPIO_PORT296_CR);
	sprint_reg8(s, GPIO_PORT297_CR);
	sprint_reg8(s, GPIO_PORT298_CR);
	sprint_reg8(s, GPIO_PORT299_CR);
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;

	sprint_reg8(s, GPIO_PORT300_CR);
	sprint_reg8(s, GPIO_PORT301_CR);
	sprint_reg8(s, GPIO_PORT302_CR);
	sprint_reg8(s, GPIO_PORT303_CR);
	sprint_reg8(s, GPIO_PORT304_CR);
	sprint_reg8(s, GPIO_PORT305_CR);
	sprint_reg8(s, GPIO_PORT306_CR);
	sprint_reg8(s, GPIO_PORT307_CR);
	sprint_reg8(s, GPIO_PORT308_CR);
	sprint_reg8(s, GPIO_PORT309_CR);
	sprint_reg8(s, GPIO_PORT310_CR);

	sprint_reg8(s, GPIO_PORT44_CR);
	sprint_reg8(s, GPIO_PORT45_CR);
	sprint_reg8(s, GPIO_PORT46_CR);
	sprint_reg8(s, GPIO_PORT47_CR);
	sprint_reg8(s, GPIO_PORT48_CR);
	sprint_reg8(s, GPIO_PORT96_CR);
	sprint_reg8(s, GPIO_PORT97_CR);
	sprint_reg8(s, GPIO_PORT98_CR);
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;
	sprint_reg8(s, GPIO_PORT99_CR);
	sprint_reg8(s, GPIO_PORT100_CR);
	sprint_reg8(s, GPIO_PORT101_CR);
	sprint_reg8(s, GPIO_PORT102_CR);
	sprint_reg8(s, GPIO_PORT103_CR);
	sprint_reg8(s, GPIO_PORT104_CR);
	sprint_reg8(s, GPIO_PORT105_CR);
	sprint_reg8(s, GPIO_PORT106_CR);
	sprint_reg8(s, GPIO_PORT107_CR);
	sprint_reg8(s, GPIO_PORT108_CR);
	sprint_reg8(s, GPIO_PORT109_CR);
	sprint_reg8(s, GPIO_PORT110_CR);

	sprint_reg8(s, GPIO_PORT139_CR);
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;

	sprint_reg8(s, GPIO_PORT320_CR);
	sprint_reg8(s, GPIO_PORT321_CR);
	sprint_reg8(s, GPIO_PORT322_CR);
	sprint_reg8(s, GPIO_PORT323_CR);
	sprint_reg8(s, GPIO_PORT324_CR);
	sprint_reg8(s, GPIO_PORT325_CR);
	sprint_reg8(s, GPIO_PORT326_CR);
	sprint_reg8(s, GPIO_PORT327_CR);
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;

	sprint_reg8(s, GPIO_PORT64_CR);
	sprint_reg8(s, GPIO_PORT65_CR);
	sprint_reg8(s, GPIO_PORT66_CR);
	sprint_reg8(s, GPIO_PORT67_CR);
	sprint_reg8(s, GPIO_PORT68_CR);
	sprint_reg8(s, GPIO_PORT69_CR);
	sprint_reg8(s, GPIO_PORT70_CR);
	sprint_reg8(s, GPIO_PORT71_CR);
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;

	sprint_reg8(s, GPIO_PORT130_CR);
	sprint_reg8(s, GPIO_PORT131_CR);
	sprint_reg8(s, GPIO_PORT203_CR);
	sprint_reg8(s, GPIO_PORT204_CR);
	sprint_reg8(s, GPIO_PORT205_CR);
	sprint_reg8(s, GPIO_PORT206_CR);
	sprint_reg8(s, GPIO_PORT207_CR);
	sprint_reg8(s, GPIO_PORT208_CR);
	sprint_reg8(s, GPIO_PORT209_CR);
	sprint_reg8(s, GPIO_PORT210_CR);
	sprint_reg8(s, GPIO_PORT211_CR);
	sprint_reg8(s, GPIO_PORT212_CR);
	sprint_reg8(s, GPIO_PORT213_CR);
	sprint_reg8(s, GPIO_PORT214_CR);
	sprint_reg8(s, GPIO_PORT217_CR);
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;

	sprint_reg8(s, GPIO_PORT72_CR);
	sprint_reg8(s, GPIO_PORT73_CR);
	sprint_reg8(s, GPIO_PORT74_CR);
	sprint_reg8(s, GPIO_PORT75_CR);
	sprint_reg8(s, GPIO_PORT76_CR);
	sprint_reg8(s, GPIO_PORT77_CR);
	sprint_reg8(s, GPIO_PORT78_CR);
	sprint_reg8(s, GPIO_PORT79_CR);
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;

	sprint_reg8(s, GPIO_PORT40_CR);
	sprint_reg8(s, GPIO_PORT41_CR);
	sprint_reg8(s, GPIO_PORT42_CR);
	sprint_reg8(s, GPIO_PORT43_CR);

	sprint_reg8(s, GPIO_PORT202_CR);
	sprint_reg8(s, GPIO_PORT141_CR);
	sprint_reg8(s, GPIO_PORT142_CR);

	sprint_reg8(s, GPIO_PORT133_CR);
	sprint_reg8(s, GPIO_PORT134_CR);
	sprint_reg8(s, GPIO_PORT135_CR);
	sprint_reg8(s, GPIO_PORT136_CR);
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;

	sprint_reg8(s, GPIO_PORT128_CR);
	sprint_reg8(s, GPIO_PORT129_CR);
	sprint_reg8(s, GPIO_PORT224_CR);
	sprint_reg8(s, GPIO_PORT225_CR);
	sprint_reg8(s, GPIO_PORT226_CR);
	sprint_reg8(s, GPIO_PORT227_CR);
	sprint_reg8(s, GPIO_PORT228_CR);
	sprint_reg8(s, GPIO_PORT229_CR);
	sprint_reg8(s, GPIO_PORT230_CR);
	sprint_reg8(s, GPIO_PORT231_CR);
	sprint_reg8(s, GPIO_PORT232_CR);
	sprint_reg8(s, GPIO_PORT233_CR);
	sprint_reg8(s, GPIO_PORT234_CR);
	sprint_reg8(s, GPIO_PORT235_CR);
	sprint_reg8(s, GPIO_PORT236_CR);
	sprint_reg8(s, GPIO_PORT237_CR);
	sprint_reg8(s, GPIO_PORT238_CR);
	sprint_reg8(s, GPIO_PORT239_CR);
	sprint_reg8(s, GPIO_PORT240_CR);
	sprint_reg8(s, GPIO_PORT241_CR);
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;
	sprint_reg8(s, GPIO_PORT242_CR);
	sprint_reg8(s, GPIO_PORT243_CR);
	sprint_reg8(s, GPIO_PORT244_CR);
	sprint_reg8(s, GPIO_PORT245_CR);
	sprint_reg8(s, GPIO_PORT246_CR);
	sprint_reg8(s, GPIO_PORT247_CR);
	sprint_reg8(s, GPIO_PORT248_CR);
	sprint_reg8(s, GPIO_PORT249_CR);
	sprint_reg8(s, GPIO_PORT250_CR);
	sprint_reg8(s, GPIO_PORT251_CR);
	sprint_reg8(s, GPIO_PORT252_CR);
	sprint_reg8(s, GPIO_PORT253_CR);
	sprint_reg8(s, GPIO_PORT254_CR);
	sprint_reg8(s, GPIO_PORT255_CR);
	sprint_reg8(s, GPIO_PORT256_CR);
	sprint_reg8(s, GPIO_PORT257_CR);
	sprint_reg8(s, GPIO_PORT258_CR);
	sprint_reg8(s, GPIO_PORT259_CR);
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;

	sprint_reg(s, GPIO_PORTL031_000DR);
	sprint_reg(s, GPIO_PORTL063_032DR);
	sprint_reg(s, GPIO_PORTL095_064DR);
	sprint_reg(s, GPIO_PORTL127_096DR);
	sprint_reg(s, GPIO_PORTL159_128DR);
	sprint_reg(s, GPIO_PORTL223_192DR);
	sprint_reg(s, GPIO_PORTL255_224DR);
	sprint_reg(s, GPIO_PORTL287_256DR);
	sprint_reg(s, GPIO_PORTL319_288DR);
	sprint_reg(s, GPIO_PORTL351_320DR);
	MSG_INFO("%s", suspend_buf);

	return;
}

static void Other_dump_suspend(void)
{
	char *s = suspend_buf;
	void __iomem *vir_addr = NULL;

	sprintf(suspend_buf, ">> %s", __func__);
	MSG_INFO("%s", suspend_buf);

	sprint_reg(s, MODEM_PLL5_CR);
	sprint_reg(s, MODEM_RFCLK_CR);
	sprint_reg(s, MODEM_HPSSCLK_CR);
	sprint_reg(s, MODEM_WPSSCLK_CR);
	sprint_reg(s, MODEM_FDICLK_CR);
	sprint_reg(s, MODEM_RETMEMPDRS_CR);
	sprint_reg(s, MODEM_PSTR);
	sprint_reg(s, MODEM_MONREG);
	sprint_reg(s, MODEM_MONREG1);
	MSG_INFO("%s", suspend_buf);

	s = suspend_buf;
	sprint_reg(s, THERMAL_THSCR0);
	sprint_reg(s, THERMAL_THSCR1);

	vir_addr = ioremap_nocache(SBSC_SDPDCR0APhys, 0x4);
	if (vir_addr) {
		sprint_regm(s, SBSC_SDPDCR0APhys, vir_addr, 32);
		iounmap(vir_addr);
	} else {
		sprintf(s, "Read 0x%x FAILED\n", SBSC_SDPDCR0APhys);
	}

	MSG_INFO("%s", suspend_buf);

	return;
}

void pmdbg_pmic_dump_suspend(void *pmic_data)
{
#ifdef PMDBG_ENABLE_PMIC_D2153
	int i, ret;
	unsigned char val;
	char *s = suspend_buf;
	struct d2153 *d2153 = (struct d2153 *)pmic_data;

	sprintf(suspend_buf, ">> %s", __func__);
	MSG_INFO("%s", suspend_buf);

	if (NULL == d2153) {
		sprintf(suspend_buf, "pmic_data is NULL");
		goto fail;
	}

	/* Read BUCK, LDO  */
	for (i = D2153_BUCK_1; i < D2153_NUMBER_OF_REGULATORS; i++) {
		ret = d2153_reg_read(d2153, pmic_reg_tbl[i].reg, &val);
		if (0 > ret) {
			sprintf(suspend_buf, "Read 0x%04x FAILED",
						pmic_reg_tbl[i].reg);
			goto fail;
		}

		s += sprintf(s, "0x%04x: 0x%02x\n",
						pmic_reg_tbl[i].reg, val);
	}
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;

	/* Read MCTL settings */
	for (i = D2153_BUCK_1; i < D2153_NUMBER_OF_REGULATORS; i++) {
		ret = d2153_reg_read(d2153, pmic_reg_tbl[i].mctl_reg, &val);
		if (0 > ret) {
			sprintf(suspend_buf, "Read 0x%04x FAILED",
						pmic_reg_tbl[i].mctl_reg);
			goto fail;
		}

		s += sprintf(s, "0x%04x: 0x%02x\n",
						pmic_reg_tbl[i].mctl_reg, val);
	}
	MSG_INFO("%s", suspend_buf);

	return;

fail:
	MSG_INFO("%s", suspend_buf);
	return;

#else

	/* D2153 Excepting Not Support  */

	return;

#endif /* PMDBG_ENABLE_PMIC_D2153 */
}

char pmdbg_get_enable_dump_suspend(void)
{
	/* enable:1, disable:0 */
	return pmdbg_enable_dump_suspend;
}
