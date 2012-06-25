/* scuw_ctrl.c
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

#define __SCUW_NO_EXTERN__

#include <sound/soundpath/soundpath.h>
#include <sound/soundpath/common_extern.h>
#include <sound/soundpath/scuw_extern.h>
#include "scuw_ctrl.h"
#include <mach/common.h>

/*
 * Register Table Definisions
 */

/* [Table summary] Reg=Register, Val=Value, D=Delay time, C=Clear bit */

/* Table for Voicecall(PortA) */
static struct common_reg_table scuw_reg_tbl_voicecallA[] = {
/*        Reg			Val		D  C */
	/*   1 : SPU2V output data */
	{ SCUW_SEL_SELCR21,	0x00000001,	0, 0 },
	/*   1 : Voice data(from VOIP) */
	{ SCUW_SEL_SELCR15,	0x00000001,	0, 0 },
	/* 011 : FSI-IF read port 1 data (from FSI2) */
	{ SCUW_SEL_SELCR12,	0x00000003,	0, 0 },
	/*   1 : FSI-IF operates. */
	{ SCUW_MSTP1,		0x00000001,	0, 0 },
	/*   0 : Reset the FSI IF. */
	{ SCUW_FSIIF_SWRSR,	0x00000000,	0, 0 },
	/*   1 : FSI IF enters the operating state. */
	{ SCUW_FSIIF_SWRSR,	0x00000001,	0, 0 },
	/*   1 : Initialization */
	{ SCUW_FSIIF_FSIIR,	0x00000001,	0, 0 },
	/* 010 : 2 channel */
	{ SCUW_FSIIF_ADINRW0,	0x00000002,	0, 0 },
	/* 010 : 2 channel */
	{ SCUW_FSIIF_ADINRR1,	0x00000002,	0, 0 },
	/* target module : FSI2(0x00), Write address : FSI2 port A(0x09) */
	{ SCUW_FSIIF_WADCR0,	0x00000009,	0, 0 },
	/* target module : FSI2(0x00), Read address : FSI2 port A(0x08) */
	{ SCUW_FSIIF_RADCR1,	0x00000008,	0, 0 },
	/*   0 : Processing State */
	{ SCUW_FSIIF_FSIIR,	0x00000000,	0, 0 },
	/* 010 : Channel 1 to 7 are copied Channel 0 */
	{ SCUW_VD_VDSET,	0x00000002,	0, 0 },
};

/* Table for Voicecall(PortB) */
static struct common_reg_table scuw_reg_tbl_voicecallB[] = {
/*        Reg			Val		D  C */
	/*   1 : SPU2V output data */
	{ SCUW_SEL_SELCR21,	0x00000001,	0, 0 },
	/*   1 : Voice data (from VOIP) */
	{ SCUW_SEL_SELCR15,	0x00000001,	0, 0 },
	/* 011 : FSI-IF read port 1 data (from FSI2) */
	{ SCUW_SEL_SELCR12,	0x00000003,	0, 0 },
	/*   1 : FSI-IF operates. */
	{ SCUW_MSTP1,		0x00000001,	0, 0 },
	/*   0 : Reset the FSI IF. */
	{ SCUW_FSIIF_SWRSR,	0x00000000,	0, 0 },
	/*   1 : FSI IF enters the operating state. */
	{ SCUW_FSIIF_SWRSR,	0x00000001,	0, 0 },
	/*   1 : Initialization */
	{ SCUW_FSIIF_FSIIR,	0x00000001,	0, 0 },
	/* 010 : 2 channel */
	{ SCUW_FSIIF_ADINRW0,	0x00000001,	0, 0 },
	/* 010 : 2 channel */
	{ SCUW_FSIIF_ADINRR1,	0x00000001,	0, 0 },
	/* target module : FSI2(0x00), Write address : FSI2 port B(0x19) */
	{ SCUW_FSIIF_WADCR0,	0x00000019,	0, 0 },
	/* target module : FSI2(0x00), Read address : FSI2 port B(0x18) */
	{ SCUW_FSIIF_RADCR1,	0x00000018,	0, 0 },
	/*   0 : Processing State */
	{ SCUW_FSIIF_FSIIR,	0x00000000,	0, 0 },
	/* 010 : Channel 1 to 7 are copied Channel 0 */
	{ SCUW_VD_VDSET,	0x00000002,	0, 0 },
};

/* Table for FM(loopback, from FSIB to FSIA) */
static struct common_reg_table scuw_reg_tbl_loopbackBA[] = {
/*        Reg			Val		D  C */
	/* 110: FSI-IF read port 1 data (from FSI2) */
	{ SCUW_SEL_SELCR0,	0x00000006,	0, 0 },
	/*  00: SEL0 output data, clock supply to MIX0 halted */
	{ SCUW_SEL_SELCR5,	0x00000000,	0, 0 },
	/*   0: SEL5 output data, clock supply to IIR0 halted */
	{ SCUW_SEL_SELCR6,	0x00000000,	0, 0 },
	/*   0: SEL6 output data, clock supply to DVU0 halted */
	{ SCUW_SEL_SELCR7,	0x00000000,	0, 0 },
	/*   0: SEL7 output data */
	{ SCUW_SEL_SELCR15,	0x00000000,	0, 0 },
	/*   1 : FSI-IF operates. */
	{ SCUW_MSTP1,		0x00000001,	0, 0 },
	/*   0 : Reset the FSI IF. */
	{ SCUW_FSIIF_SWRSR,	0x00000000,	0, 0 },
	/*   1 : FSI IF enters the operating state. */
	{ SCUW_FSIIF_SWRSR,	0x00000001,	0, 0 },
	/*   1 : Initialization */
	{ SCUW_FSIIF_FSIIR,	0x00000001,	0, 0 },
	/* 010 : 2 channel */
	{ SCUW_FSIIF_ADINRW0,	0x00000002,	0, 0 },
	/* 010 : 2 channel */
	{ SCUW_FSIIF_ADINRR1,	0x00000002,	0, 0 },
	/* target module : FSI2(0x00), Write address : FSI2 port A(0x09) */
	{ SCUW_FSIIF_WADCR0,	0x00000009,	0, 0 },
	/* target module : FSI2(0x00), Read address : FSI2 port B(0x18) */
	{ SCUW_FSIIF_RADCR1,	0x00000018,	0, 0 },
	/*   0 : Processing State */
	{ SCUW_FSIIF_FSIIR,	0x00000000,	0, 0 },
};


/*!
   @brief SCUW start function

   @param[in]	uiValue		PCM type
   @param[out]	none

   @retval	ERROR_NONE	successful
 */
int scuw_start(const u_int uiValue)
{
	/* Local variable declaration */
	u_int			dev		= 0;
	struct common_reg_table	*reg_tbl	= NULL;
	u_int			tbl_size	= 0;

	sndp_log_debug_func("start\n");

	/* Start SCUW Clock Supply */
	audio_ctrl_func(SNDP_HW_SCUW, STAT_ON);

	/* Device check */
	dev = SNDP_GET_DEVICE_VAL(uiValue);
	/* SPEAKER, EARPIECE, WIREDHEADSET, WIREDHEADPHONE, MIC */
	if ((false == (dev & SNDP_BLUETOOTHSCO)) &&
	    (false == (dev & SNDP_FM_RADIO_TX)) &&
	    (false == (dev & SNDP_FM_RADIO_RX))) {
		reg_tbl  = scuw_reg_tbl_voicecallA;
		tbl_size = ARRAY_SIZE(scuw_reg_tbl_voicecallA);
	/* BLUETOOTHSCO */
	} else if (false != (dev & SNDP_BLUETOOTHSCO)) {
		reg_tbl  = scuw_reg_tbl_voicecallB;
		tbl_size = ARRAY_SIZE(scuw_reg_tbl_voicecallB);
	/* FM_RADIO_RX */
	} else {
		reg_tbl  = scuw_reg_tbl_loopbackBA;
		tbl_size = ARRAY_SIZE(scuw_reg_tbl_loopbackBA);
	}

	/* Start SCUW Settings */
	common_set_register(SNDP_HW_SCUW, reg_tbl, tbl_size);

	/* SCUW Registers Dump */
	scuw_reg_dump();

	sndp_log_debug_func("end (ret = %d)\n", ERROR_NONE);

	return ERROR_NONE;
}


/*!
   @brief SCUW stop function(all path)

   @param[in]	none
   @param[out]	none

   @retval	ERROR_NONE	successful
 */
int scuw_stop(void)
{
	sndp_log_debug_func("start\n");

	/* SCUW Registers Dump */
	scuw_reg_dump();

	/* Stop SCUW Clock Supply */
	audio_ctrl_func(SNDP_HW_SCUW, STAT_OFF);

	sndp_log_debug_func("end (ret = %d)\n", ERROR_NONE);

	return ERROR_NONE;
}


/*!
   @brief Dump registers function

   @param[in]	none
   @param[out]	none

   @retval	none
 */
void scuw_reg_dump(void)
{
	/* Register Dump Start */
	sndp_log_reg_dump("=== SCUW REGISTER DUMP START ===\n");

	sndp_log_reg_dump("SCUW_VD_VDSET       [%08lX] = %08X\n",
			(g_scuw_Base + SCUW_VD_VDSET),
			ioread32((g_scuw_Base + SCUW_VD_VDSET)));
	sndp_log_reg_dump("SCUW_SEL_SELCR0     [%08lX] = %08X\n",
			(g_scuw_Base + SCUW_SEL_SELCR0),
			ioread32((g_scuw_Base + SCUW_SEL_SELCR0)));
	sndp_log_reg_dump("SCUW_SEL_SELCR5     [%08lX] = %08X\n",
			(g_scuw_Base + SCUW_SEL_SELCR5),
			ioread32((g_scuw_Base + SCUW_SEL_SELCR5)));
	sndp_log_reg_dump("SCUW_SEL_SELCR6     [%08lX] = %08X\n",
			(g_scuw_Base + SCUW_SEL_SELCR6),
			ioread32((g_scuw_Base + SCUW_SEL_SELCR6)));
	sndp_log_reg_dump("SCUW_SEL_SELCR7    [%08lX] = %08X\n",
			(g_scuw_Base + SCUW_SEL_SELCR7),
			ioread32((g_scuw_Base + SCUW_SEL_SELCR7)));
	sndp_log_reg_dump("SCUW_SEL_SELCR12    [%08lX] = %08X\n",
			(g_scuw_Base + SCUW_SEL_SELCR12),
			ioread32((g_scuw_Base + SCUW_SEL_SELCR12)));
	sndp_log_reg_dump("SCUW_SEL_SELCR15    [%08lX] = %08X\n",
			(g_scuw_Base + SCUW_SEL_SELCR15),
			ioread32((g_scuw_Base + SCUW_SEL_SELCR15)));
	sndp_log_reg_dump("SCUW_SEL_SELCR21    [%08lX] = %08X\n",
			(g_scuw_Base + SCUW_SEL_SELCR21),
			ioread32((g_scuw_Base + SCUW_SEL_SELCR21)));
	sndp_log_reg_dump("SCUW_FSIIF_SWRSR    [%08lX] = %08X\n",
			(g_scuw_Base + SCUW_FSIIF_SWRSR),
			ioread32((g_scuw_Base + SCUW_FSIIF_SWRSR)));
	sndp_log_reg_dump("SCUW_FSIIF_FSIIR    [%08lX] = %08X\n",
			(g_scuw_Base + SCUW_FSIIF_FSIIR),
			ioread32((g_scuw_Base + SCUW_FSIIF_FSIIR)));
	sndp_log_reg_dump("SCUW_FSIIF_ADINRW0  [%08lX] = %08X\n",
			(g_scuw_Base + SCUW_FSIIF_ADINRW0),
			ioread32((g_scuw_Base + SCUW_FSIIF_ADINRW0)));
	sndp_log_reg_dump("SCUW_FSIIF_ADINRW1  [%08lX] = %08X\n",
			(g_scuw_Base + SCUW_FSIIF_ADINRW1),
			ioread32((g_scuw_Base + SCUW_FSIIF_ADINRW1)));
	sndp_log_reg_dump("SCUW_FSIIF_ADINRR0  [%08lX] = %08X\n",
			(g_scuw_Base + SCUW_FSIIF_ADINRR0),
			ioread32((g_scuw_Base + SCUW_FSIIF_ADINRR0)));
	sndp_log_reg_dump("SCUW_FSIIF_ADINRR1  [%08lX] = %08X\n",
			(g_scuw_Base + SCUW_FSIIF_ADINRR1),
			ioread32((g_scuw_Base + SCUW_FSIIF_ADINRR1)));
	sndp_log_reg_dump("SCUW_FSIIF_WADCR0   [%08lX] = %08X\n",
			(g_scuw_Base + SCUW_FSIIF_WADCR0),
			ioread32((g_scuw_Base + SCUW_FSIIF_WADCR0)));
	sndp_log_reg_dump("SCUW_FSIIF_WADCR1   [%08lX] = %08X\n",
			(g_scuw_Base + SCUW_FSIIF_WADCR1),
			ioread32((g_scuw_Base + SCUW_FSIIF_WADCR1)));
	sndp_log_reg_dump("SCUW_FSIIF_RADCR0   [%08lX] = %08X\n",
			(g_scuw_Base + SCUW_FSIIF_RADCR0),
			ioread32((g_scuw_Base + SCUW_FSIIF_RADCR0)));
	sndp_log_reg_dump("SCUW_FSIIF_RADCR1   [%08lX] = %08X\n",
			(g_scuw_Base + SCUW_FSIIF_RADCR1),
			ioread32((g_scuw_Base + SCUW_FSIIF_RADCR1)));

	sndp_log_reg_dump("==== SCUW REGISTER DUMP END ====\n");
}

#ifdef SOUND_TEST
void scuw_voice_test_start_a(void)
{
	/* Clock framework API, Status ON */
	audio_ctrl_func(SNDP_HW_SCUW, STAT_ON);

	iowrite32(0x00000001, (g_scuw_Base + SCUW_SEL_SELCR21));
	iowrite32(0x00000001, (g_scuw_Base + SCUW_SEL_SELCR15));
	iowrite32(0x00000003, (g_scuw_Base + SCUW_SEL_SELCR12));
	iowrite32(0x00000001, (g_scuw_Base + SCUW_MSTP1));
	iowrite32(0x00000000, (g_scuw_Base + SCUW_FSIIF_SWRSR));
	iowrite32(0x00000001, (g_scuw_Base + SCUW_FSIIF_SWRSR));
	iowrite32(0x00000001, (g_scuw_Base + SCUW_FSIIF_FSIIR));
	iowrite32(0x00000002, (g_scuw_Base + SCUW_FSIIF_ADINRW0));
	iowrite32(0x00000002, (g_scuw_Base + SCUW_FSIIF_ADINRR1));
	iowrite32(0x00000009, (g_scuw_Base + SCUW_FSIIF_WADCR0));
	iowrite32(0x00000008, (g_scuw_Base + SCUW_FSIIF_RADCR1));
	iowrite32(0x00000000, (g_scuw_Base + SCUW_FSIIF_FSIIR));
	iowrite32(0x00000002, (g_scuw_Base + SCUW_VD_VDSET));

	scuw_reg_dump();
}

void scuw_voice_test_stop_a(void)
{
	scuw_reg_dump();

	/* Clock framework API, Status OFF */
	audio_ctrl_func(SNDP_HW_SCUW, STAT_OFF);
}
#endif /* SOUND_TEST */

