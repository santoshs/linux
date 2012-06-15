/* common_ctrl.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */


#define __COMMON_CTRL_NO_EXTERN__

#include <linux/clk.h>
#include <sound/soundpath/common_extern.h>
#include "common_ctrl.h"


/*
 * GLOBAL DATA Definitions
 */

/* CPGA register base address */
static u_long g_common_ulClkRstRegBase;
/* CPGA register(soft reset) base address */
static u_long g_common_ulSrstRegBase;

/* GPIO register base address */
static u_long g_common_ulClkGpioRegBase;

/* Clock status flag */
static u_int g_clock_flag;


/*!
   @brief Register value modify

   @param[in]	uiClr	Clear bit
   @param[in]	uiSet	Set bit
   @param[in]	uiReg	Register address
   @param[out]	none

   @retval	none
 */
void iomodify32(u_int uiClr, u_int uiSet, u_int uiReg)
{
	/* Local variable declaration */
	u_int uiTmp;

	uiTmp = ioread32((void __iomem *)uiReg);
	uiTmp &= (~uiClr);
	uiTmp |= uiSet;
	iowrite32(uiTmp, (void __iomem *)uiReg);
}


/*!
   @brief Registers ioremap for Common/CLKGEN/FSI/SCUW

   @param[in]	none
   @param[out]	none

   @retval	0		Successfull
   @retval	-ENOMEM		Resource error
 */
int common_ioremap(void)
{
	/* Local variable declaration */
	int iRet = ERROR_NONE;

	iRet = common_audio_status_ioremap();
	if (ERROR_NONE != iRet)
		goto ioremap_audio_err;

	iRet = clkgen_ioremap();
	if (ERROR_NONE != iRet)
		goto ioremap_clkgen_err;

	iRet = fsi_ioremap();
	if (ERROR_NONE != iRet)
		goto ioremap_fsi_err;

	iRet = scuw_ioremap();
	if (ERROR_NONE != iRet)
		goto ioremap_scuw_err;

	/* Success of all */
	return ERROR_NONE;

ioremap_scuw_err:
	fsi_iounmap();
ioremap_fsi_err:
	clkgen_iounmap();
ioremap_clkgen_err:
	common_audio_status_iounmap();
ioremap_audio_err:
	return iRet;
}


/*!
   @brief Registers iounmap for Common/CLKGEN/FSI/SCUW

   @param[in]	none
   @param[out]	none

   @retval	none
 */
void common_iounmap(void)
{
	scuw_iounmap();
	fsi_iounmap();
	clkgen_iounmap();
	common_audio_status_iounmap();
}


/*!
   @brief FSI registers ioremap

   @param[in]	none
   @param[out]	none

   @retval	0		Successful
   @retval	-ENOMEM		Resource error
 */
static int fsi_ioremap(void)
{
	/* Get FSI Logical Address */
	g_fsi_Base = (u_long)ioremap_nocache(FSI_PHY_BASE, FSI_MAP_LEN);
	if (0 >= g_fsi_Base) {
		sndp_log_err("error fsi ioremap failed\n");
		return -ENOMEM;
	}

	/* Successfull all */
	return ERROR_NONE;
}


/*!
   @brief SCUW ioremap function

   @param[in]	none
   @param[out]	none

   @retval	ERROR_NONE	successful
   @retval	-ENOMEM		resource error
 */
static int scuw_ioremap(void)
{
	/* Get SCUW Logical Address */
	g_scuw_Base = (u_long)ioremap_nocache(SCUW_PHY_BASE, SUCW_MAP_LEN);
	if (0 >= g_scuw_Base) {
		sndp_log_err("error scuw ioremap failed\n");
		return -ENOMEM;
	}

	/* Get SCUW(FFD) Logical Address */
	g_scuw_Base_FFD =
		(u_long)ioremap_nocache(SCUW_PHY_BASE_FFD, SUCW_MAP_LEN_FFD);
	if (0 >= g_scuw_Base_FFD) {
		sndp_log_err("error scuw(FFD) ioremap failed\n");
		/* Release SCUW Logical Address */
		iounmap((void *)g_scuw_Base);
		g_scuw_Base = 0;
		return -ENOMEM;
	}

	/* Successfull all */
	return ERROR_NONE;
}


/*!
   @brief CLKGEN registers ioremap

   @param[in]	none
   @param[out]	none

   @retval	0		Successful
   @retval	-ENOMEM		Resource error
 */
int clkgen_ioremap(void)
{
	/* Get CLKGEN Logical Address */
	g_clkgen_Base =
		(u_long)ioremap_nocache(CLKGEN_PHY_BASE, CLKGEN_MAP_LEN);
	if (0 >= g_clkgen_Base) {
		sndp_log_err("error clkgen ioremap failed\n");
		return -ENOMEM;
	}

	return ERROR_NONE;
}


/*!
   @brief Registers ioremap for use audio_ctrl_func()

   @param[in]	none
   @param[out]	none

   @retval	0		Succcessfull
   @retval	-ENOMEM		Resource error
 */
static int common_audio_status_ioremap(void)
{
	/* Get CPGA Logical Address */
	g_common_ulClkRstRegBase =
		(u_long)ioremap_nocache(CPG_PHY_BASE, CPG_REG_MAX);
	if (0 >= g_common_ulClkRstRegBase) {
		sndp_log_err("error CPGA register ioremap failed\n");
		return -ENOMEM;
	}

	/* Get CPGA(soft reset) Logical Address */
	g_common_ulSrstRegBase =
		(u_long)ioremap_nocache(CPG_PHY_BASE_SRST, CPG_REG_MAX_SRST);
	if (0 >= g_common_ulSrstRegBase) {
		sndp_log_err("error Software Reset register ioremap failed\n");
		/* Release CPGA Logical Address */
		iounmap((void *)g_common_ulClkRstRegBase);
		g_common_ulClkRstRegBase = 0;
		return -ENOMEM;
	}

	/* Get GPIO Logical Address */
	g_common_ulClkGpioRegBase =
		(u_long)ioremap_nocache(GPIO_PHY_BASE, GPIO_REG_MAX);
	if (0 >= g_common_ulClkRstRegBase) {
		sndp_log_err("error GPIO register ioremap failed\n");
		return -ENOMEM;
	}

	/* Successfull all */
	return ERROR_NONE;
}


/*!
   @brief FSI registers iounmap

   @param[in]	none
   @param[out]	none

   @retval	none
 */
static void fsi_iounmap(void)
{
	/* Release FSI Logical Address */
	if (0 < g_fsi_Base) {
		iounmap((void *)g_fsi_Base);
		g_fsi_Base = 0;
	}
}


/*!
   @brief SCUW iounmap function

   @param[in]	none
   @param[out]	none
   @retval	none
 */
static void scuw_iounmap(void)
{
	/* Release SCUW Logical Address */
	if (0 < g_scuw_Base) {
		iounmap((void *)g_scuw_Base);
		g_scuw_Base = 0;
	}
	/* Release SCUW Logical Address to FFD */
	if (0 < g_scuw_Base_FFD) {
		iounmap((void *)g_scuw_Base_FFD);
		g_scuw_Base_FFD = 0;
	}
}


/*!
   @brief CLKGEN registers iounmap

   @param[in]	none
   @param[out]	none

   @retval	none
 */
void clkgen_iounmap(void)
{
	/* Release CLKGEN Logical Address */
	if (0 < g_clkgen_Base) {
		iounmap((void *)g_clkgen_Base);
		g_clkgen_Base = 0;
	}
}


/*!
   @brief Registers iounmap for use audio_ctrl_func()

   @param[in]	none
   @param[out]	none

   @retval	none
 */
static void common_audio_status_iounmap(void)
{
	/* Release CPGA Logical Address */
	if (0 < g_common_ulClkRstRegBase) {
		iounmap((void *)g_common_ulClkRstRegBase);
		g_common_ulClkRstRegBase = 0;
	}

	/* Release CPGA(soft reset) Logical Address */
	if (0 < g_common_ulSrstRegBase) {
		iounmap((void *)g_common_ulSrstRegBase);
		g_common_ulSrstRegBase = 0;
	}

	/* Release GPIO Logical Address */
	if (0 < g_common_ulClkGpioRegBase) {
		iounmap((void *)g_common_ulClkGpioRegBase);
		g_common_ulClkGpioRegBase = 0;
	}
}


/*!
   @brief Power management for CLKGEN/FSI/SCUW

   @param[in]	drv	H/W type
   @param[in]	stat	On/Off
   @param[out]	none

   @retval	none
 */
void audio_ctrl_func(enum sndp_hw_audio drv, int stat)
{
	/* Local variable declaration */
	struct clk *clk;
/*	unsigned long flags; */
#ifdef SOUND_TEST
	u_int reg;
#endif

	switch (drv) {
	case SNDP_HW_CLKGEN:
		/* Status ON */
		if (STAT_ON == stat) {
			if (!(SNDP_CLK_CLKGEN & g_clock_flag)) {
				clk = clk_get(NULL, "clkgen");
				if (IS_ERR(clk)) {
					sndp_log_err("clkget(clkgen) error\n");
				} else {
					clk_enable(clk);
					clk_put(clk);
				}

/*				ape5r_get_cpg_hpb_sem_with_lock(flags); */
				/* Soft Reset */
				iomodify32(0, 0x01000000, CPG_SRCR2);
				udelay(62);
				/* CLKGEN operates */
				iomodify32(0x01000000, 0, CPG_SRCR2);
/*				ape5r_put_cpg_hpb_sem_with_lock(flags); */

				g_clock_flag |= SNDP_CLK_CLKGEN;
			}
		/* Status OFF */
		} else {
			if ((SNDP_CLK_CLKGEN & g_clock_flag)) {
				clk = clk_get(NULL, "clkgen");
				if (IS_ERR(clk)) {
					sndp_log_err("clkget(clkgen) error\n");
				} else {
					clk_disable(clk);
					clk_put(clk);
				}
				g_clock_flag &= ~SNDP_CLK_CLKGEN;
			}
		}
		break;

	case SNDP_HW_FSI:
		/* Status ON */
		if (STAT_ON == stat) {
			if (!(SNDP_CLK_FSI & g_clock_flag)) {
				clk = clk_get(NULL, "fsi");
				if (IS_ERR(clk)) {
					sndp_log_err("clkget(fsi) error\n");
				} else {
					clk_enable(clk);
					clk_put(clk);
				}

/*				ape5r_get_cpg_hpb_sem_with_lock(flags); */
				/* Soft Reset */
				iomodify32(0, 0x10000000, CPG_SRCR3);
				udelay(62);
				/* FSI operates */
				iomodify32(0x10000000, 0, CPG_SRCR3);
/*				ape5r_put_cpg_hpb_sem_with_lock(flags); */

				g_clock_flag |= SNDP_CLK_FSI;
#ifdef SOUND_TEST
				reg = ioread32(CPG_SUBCKCR);
				if (reg & (1 << 11)) {
					iomodify32(0x00000800, 0, CPG_SUBCKCR);
				reg = ioread32(CPG_SUBCKCR);
				}
#endif /* SOUND_TEST */
			}
		/* Status OFF */
		} else {
			if ((SNDP_CLK_FSI & g_clock_flag)) {
				clk = clk_get(NULL, "fsi");
				if (IS_ERR(clk)) {
					sndp_log_err("clkget(fsi) error\n");
				} else {
					clk_disable(clk);
					clk_put(clk);
				}

				g_clock_flag &= ~SNDP_CLK_FSI;
			}
		}
		break;

	case SNDP_HW_SCUW:
		/* Status ON */
		if (STAT_ON == stat) {
			if (!(SNDP_CLK_SCUW & g_clock_flag)) {
				clk = clk_get(NULL, "scuw");
				if (IS_ERR(clk)) {
					sndp_log_err("clkget(scuw) error\n");
				} else {
					clk_enable(clk);
					clk_put(clk);
				}

/*				ape5r_get_cpg_hpb_sem_with_lock(flags); */
				/* Soft Reset */
				iomodify32(0, 0x04000000, CPG_SRCR3);
				udelay(62);
				/* SCUW operates */
				iomodify32(0x04000000, 0, CPG_SRCR3);
/*				ape5r_put_cpg_hpb_sem_with_lock(flags); */

				g_clock_flag |= SNDP_CLK_SCUW;
			}
		/* Status OFF */
		} else {
			if ((SNDP_CLK_SCUW & g_clock_flag)) {
				clk = clk_get(NULL, "scuw");
				if (IS_ERR(clk)) {
					sndp_log_err("clkget(scuw) error\n");
				} else {
					clk_disable(clk);
					clk_put(clk);
				}
				g_clock_flag &= ~SNDP_CLK_SCUW;
			}
		}
		break;

	default:
		break;
	}
}


/*!
   @brief Common registers setting function

   @param[in]	H/W type
   @param[in]	Register table
   @param[in]	Register table size
   @param[out]	none

   @retval	none
 */
void common_set_register(
	enum sndp_hw_audio drv,
	struct common_reg_table *reg_tbl,
	u_int size)
{
	/* Local variable declaration */
	int	i;

	sndp_log_debug_func("start\n");

	for (i = 0; size > i; i++) {
		/* Delay */
		if (0 != reg_tbl[i].uiDelay) {
			/* 1000 micro over */
			if (COMMON_UDELAY_MAX <= reg_tbl[i].uiDelay)
				mdelay((reg_tbl[i].uiDelay /
					COMMON_UDELAY_MAX));
			else
				udelay(reg_tbl[i].uiDelay);
		/* Modify */
		} else if (0 != reg_tbl[i].uiClrbit) {
			/* CLKGEN */
			if (SNDP_HW_CLKGEN == drv)
				iomodify32(reg_tbl[i].uiClrbit,
					   reg_tbl[i].uiValue,
					   (g_clkgen_Base + reg_tbl[i].uiReg));
			/* FSI */
			else if (SNDP_HW_FSI == drv)
				iomodify32(reg_tbl[i].uiClrbit,
					   reg_tbl[i].uiValue,
					   (g_fsi_Base + reg_tbl[i].uiReg));
			/* SCUW */
			else
				iomodify32(reg_tbl[i].uiClrbit,
					   reg_tbl[i].uiValue,
					   (g_scuw_Base + reg_tbl[i].uiReg));
		/* Register setting */
		} else {
			/* CLKGEN */
			if (SNDP_HW_CLKGEN == drv)
				iowrite32(reg_tbl[i].uiValue,
					  (g_clkgen_Base + reg_tbl[i].uiReg));
			/* FSI */
			else if (SNDP_HW_FSI == drv)
				iowrite32(reg_tbl[i].uiValue,
					  (g_fsi_Base + reg_tbl[i].uiReg));
			/* SCUW */
			else
				iowrite32(reg_tbl[i].uiValue,
					  (g_scuw_Base + reg_tbl[i].uiReg));
		}
	}

	sndp_log_debug_func("end\n");
}


/*!
   @brief PLL22 and FSIACKCR/FSIBCKCR setting function

   @param[in]	uiValue		PCM type
   @param[in]	stat		On/Off
   @param[out]	none

   @retval	none
 */
void common_set_pll22(const u_int uiValue, int stat)
{
	/* Local variable declaration */
	u_int dev, fsickcr;
	int cnt;

	sndp_log_debug_func("start\n");

	/* Device check */
	dev = SNDP_GET_DEVICE_VAL(uiValue);
	/* PortA */
	if ((false == (dev & SNDP_BLUETOOTHSCO)) &&
	    (false == (dev & SNDP_FM_RADIO_TX)) &&
	    (false == (dev & SNDP_FM_RADIO_RX))) {
		fsickcr = CPG_FSIACKCR;
	/* PortB */
	} else {
		fsickcr = CPG_FSIBCKCR;
	}

	/* Status ON */
	if (STAT_ON == stat) {
		/* mode check */
		if (SNDP_MODE_INCALL != SNDP_GET_MODE_VAL(uiValue)) {
			/* Pll22 enable 66 divide */
			iowrite32(0x41000000, CPG_PLL22CR);
			iomodify32(0, 0x00000010, CPG_PLLECR);

			for (cnt = 0; cnt < 10; cnt++) {
				if (!(0x1000 & ioread32(CPG_PLLECR)))
					udelay(100);
				else
					break;
			}
			if (10 == cnt)
				sndp_log_err("CPG_PLLECR is not available.\n");

			/* FSICKCR enable 38 divide */
			iowrite32(0x00001065, fsickcr);

			/* FM Radio */
			if (false != (dev & SNDP_FM_RADIO_RX)) {
				/* PortA */
				fsickcr = CPG_FSIACKCR;
				/* FSICKCR enable 38 divide */
				iowrite32(0x00001065, fsickcr);
			}
		} else {
			/* Pll22 enable 64 divide */
			/* FSICKCR enable 25 divide */
			/*
			 * iowrite32(0x39000000, CPG_PLL22CR);
			 * iomodify32(0, 0x00000010, CPG_PLLECR);
			 * iowrite32(0x00001058, fsickcr);
			 */
		}
	/* Status OFF */
	} else {
		/* mode check */
		if (SNDP_MODE_INCALL != SNDP_GET_MODE_VAL(uiValue)) {
			/* FSICKCR disable */
			iowrite32(0x00000100, fsickcr);

			/* FM Radio */
			if (false != (dev & SNDP_FM_RADIO_RX)) {
				/* PortA */
				fsickcr = CPG_FSIACKCR;
				/* FSICKCR disable */
				iowrite32(0x00000100, fsickcr);
			}

			/* Pll22 disable */
			iomodify32(0x00000010, 0, CPG_PLLECR);
		}
	}

	sndp_log_debug_func("end\n");
}


/*!
   @brief FSI2CR(GPIO) setting function for FSI master

   @param[in]	stat		On/Off
   @param[out]	none

   @retval	none
 */
void common_set_fsi2cr(int stat)
{
	sndp_log_debug_func("start\n");

	if (STAT_ON == stat)
		iowrite16(0x0300, GPIO_FSI2CR);
	else
		iowrite16(0x0000, GPIO_FSI2CR);

	sndp_log_debug_func("end\n");
}

