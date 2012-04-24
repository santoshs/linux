/* clkgen_ctrl.c
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


#define __CLKGEN_CTRL_NO_EXTERN__

#include <sound/soundpath/soundpath.h>
#include <sound/soundpath/common_extern.h>
#include <sound/soundpath/clkgen_extern.h>
#include "clkgen_ctrl.h"
#include <mach/common.h>


/*
 * GLOBAL DATA Definitions
 */

/* CLKGEN Control functions table */
static struct ctrl_func_tbl g_clkgen_ctrl_func_tbl[] = {
	{ SNDP_PLAYBACK_EARPIECE_NORMAL,                    clkgen_playback   },
	{ SNDP_PLAYBACK_EARPIECE_RINGTONE,                  clkgen_playback   },
	{ SNDP_PLAYBACK_EARPIECE_INCALL,                    clkgen_voicecall  },
	{ SNDP_PLAYBACK_EARPIECE_INCOMMUNICATION,           clkgen_playback   },
	{ SNDP_PLAYBACK_SPEAKER_NORMAL,                     clkgen_playback   },
	{ SNDP_PLAYBACK_SPEAKER_RINGTONE,                   clkgen_playback   },
	{ SNDP_PLAYBACK_SPEAKER_INCALL,                     clkgen_voicecall  },
	{ SNDP_PLAYBACK_SPEAKER_INCOMMUNICATION,            clkgen_playback   },
	{ SNDP_PLAYBACK_BLUETOOTH_NORMAL,                   clkgen_playback   },
	{ SNDP_PLAYBACK_BLUETOOTH_RINGTONE,                 clkgen_playback   },
	{ SNDP_PLAYBACK_BLUETOOTH_INCALL,                   clkgen_voicecall  },
	{ SNDP_PLAYBACK_BLUETOOTH_INCOMMUNICATION,          clkgen_playback   },
	{ SNDP_PLAYBACK_HEADSET_NORMAL,                     clkgen_playback   },
	{ SNDP_PLAYBACK_HEADSET_RINGTONE,                   clkgen_playback   },
	{ SNDP_PLAYBACK_HEADSET_INCALL,                     clkgen_voicecall  },
	{ SNDP_PLAYBACK_HEADSET_INCOMMUNICATION,            clkgen_playback   },
	{ SNDP_PLAYBACK_SPEAKER_HEADSET_NORMAL,             clkgen_playback   },
	{ SNDP_PLAYBACK_SPEAKER_HEADSET_RINGTONE,           clkgen_playback   },
	{ SNDP_PLAYBACK_SPEAKER_HEADSET_INCALL,             clkgen_voicecall  },
	{ SNDP_PLAYBACK_SPEAKER_HEADSET_INCOMMUNICATION,    clkgen_playback   },
	{ SNDP_PLAYBACK_HEADPHONE_NORMAL,                   clkgen_playback   },
	{ SNDP_PLAYBACK_HEADPHONE_RINGTONE,                 clkgen_playback   },
	{ SNDP_PLAYBACK_HEADPHONE_INCALL,                   clkgen_voicecall  },
	{ SNDP_PLAYBACK_HEADPHONE_INCOMMUNICATION,          clkgen_playback   },
	{ SNDP_PLAYBACK_SPEAKER_HEADPHONE_NORMAL,           clkgen_playback   },
	{ SNDP_PLAYBACK_SPEAKER_HEADPHONE_RINGTONE,         clkgen_playback   },
	{ SNDP_PLAYBACK_SPEAKER_HEADPHONE_INCALL,           clkgen_voicecall  },
	{ SNDP_PLAYBACK_SPEAKER_HEADPHONE_INCOMMUNICATION,  clkgen_playback   },
	{ SNDP_PLAYBACK_AUXDIGITAL_NORMAL,                  clkgen_playback   },
	{ SNDP_PLAYBACK_AUXDIGITAL_RINGTONE,                NULL              },
	{ SNDP_PLAYBACK_AUXDIGITAL_INCALL,                  NULL              },
	{ SNDP_PLAYBACK_AUXDIGITAL_INCOMMUNICATION,         NULL              },
	{ SNDP_CAPTURE_MIC_NORMAL,                          clkgen_capture    },
	{ SNDP_CAPTURE_MIC_RINGTONE,                        clkgen_capture    },
	{ SNDP_CAPTURE_MIC_INCALL,                          NULL              },
	{ SNDP_CAPTURE_MIC_INCOMMUNICATION,                 clkgen_capture    },
	{ SNDP_CAPTURE_HEADSET_NORMAL,                      clkgen_capture    },
	{ SNDP_CAPTURE_HEADSET_RINGTONE,                    clkgen_capture    },
	{ SNDP_CAPTURE_HEADSET_INCALL,                      NULL              },
	{ SNDP_CAPTURE_HEADSET_INCOMMUNICATION,             clkgen_capture    },
	{ SNDP_CAPTURE_BLUETOOTH_NORMAL,                    NULL              },
	{ SNDP_CAPTURE_BLUETOOTH_RINGTONE,                  NULL              },
	{ SNDP_CAPTURE_BLUETOOTH_INCALL,                    NULL              },
	{ SNDP_CAPTURE_BLUETOOTH_INCOMMUNICATION,           NULL              },
};


/* Table for Playback(PortA) */
static struct common_reg_table clkgen_reg_tbl_playA[] = {
/*	  Register	 Value	     Delay time */
	{ CLKG_SYSCTL,	 0x00000000, 0 }, /* EXTAL1 clock supply */
	{ CLKG_FSIACOM,	 0x00212901, 0 }, /* 2ch, 64fs, 48kHz, CLKGEN master,
					   * Non - continuos mode */
	{ CLKG_PULSECTL, 0x00000001, 0 }, /* PortA Enable */
};

/* Table for Playback(PortB) */
static struct common_reg_table clkgen_reg_tbl_playB[] = {
/*	  Register	 Value	     Delay time */
	{ CLKG_SYSCTL,	 0x00000000, 0 }, /* EXTAL1 clock supply */
	{ CLKG_FSIBCOM,	 0x00212901, 0 }, /* 2ch, 64fs, 48kHz, CLKGEN master,
					   * Non - continuos mode */
	{ CLKG_PULSECTL, 0x00000002, 0 }, /* PortB Enable */
};

/* Table for Capture(PortA) */
static struct common_reg_table clkgen_reg_tbl_captureA[] = {
/*	  Register	 Value	     Delay time */
	{ CLKG_SYSCTL,	 0x00000000, 0 }, /* EXTAL1 clock supply */
	{ CLKG_TIMSEL1,	 0x00000000, 0 }, /* REC TIM0(PortA) */
	{ CLKG_FSIACOM,	 0x00212901, 0 }, /* 2ch, 64fs, 48kHz, CLKGEN master,
					   * Non - continuos mode */
	{ CLKG_PULSECTL, 0x00000001, 0 }, /* PortA Enable */
};

/* Table for Capture(PortB) */
static struct common_reg_table clkgen_reg_tbl_captureB[] = {
/*	  Register	 Value	     Delay time */
	{ CLKG_SYSCTL,	 0x00000000, 0 }, /* EXTAL1 clock supply */
	{ CLKG_TIMSEL1,	 0x00000002, 0 }, /* REC TIM0(PortB) */
	{ CLKG_FSIBCOM,	 0x00212901, 0 }, /* 2ch, 64fs, 48kHz, CLKGEN master,
					   * Non - continuos mode */
	{ CLKG_PULSECTL, 0x00000002, 0 }, /* PortB Enable */
};

/* Table for Voicecall(PortA) */
static struct common_reg_table clkgen_reg_tbl_voicecallA[] = {
/*	  Register	 Value	     Delay time */
	{ CLKG_SYSCTL,	 0x00000000, 0 }, /* EXTAL1 clock supply */
	{ CLKG_SPUVCOM,	 0x00212401, 0 }, /* 2ch, 64fs, 16kHz, CLKGEN master,
					   * Non - continuos mode */
	{ CLKG_TIMSEL1,	 0x00000000, 0 }, /* REC TIM0(PortA) */
	{ CLKG_FSIACOM,	 0x00212401, 0 }, /* 2ch, 64fs, 16kHz, CLKGEN master,
					   * Non - continuos mode */
	{ CLKG_PULSECTL, 0x00000011, 0 }, /* SPUV / PortA Enable */
};

/* Table for Voicecall(PortB) */
static struct common_reg_table clkgen_reg_tbl_voicecallB[] = {
/*	  Register	 Value	     Delay time */
	{ CLKG_SYSCTL,	 0x00000000, 0 }, /* EXTAL1 clock supply */
	{ CLKG_SPUVCOM,	 0x00212401, 0 }, /* 2ch, 64fs, 16kHz, CLKGEN master,
					   * Non - continuos mode */
	{ CLKG_TIMSEL1,	 0x00000002, 0 }, /* REC TIM0(PortB) */
	{ CLKG_FSIBCOM,	 0x00212401, 0 }, /* 2ch, 64fs, 16kHz, CLKGEN master,
					   * Non - continuos mode */
	{ CLKG_PULSECTL, 0x00000012, 0 }, /* SPUV / PortB Enable */
};


/*!
   @brief CLKGEN start function

   @param[in]	uiValue		PCM type
   @param[out]	none

   @retval	0		Successful
   @retval	-EINVAL		Invalid argument
 */
int clkgen_start(const u_int uiValue)
{
	/* Local variable declaration */
	int iCnt;

	sndp_log_debug_func("start [0x%08x]\n", uiValue);

	/* Call of function of each PATH */
	for (iCnt = 0; ARRAY_SIZE(g_clkgen_ctrl_func_tbl) > iCnt; iCnt++) {
		/* uiValue check */
		if (uiValue == g_clkgen_ctrl_func_tbl[iCnt].uiValue) {
			/* Function pointer check */
			if (NULL != g_clkgen_ctrl_func_tbl[iCnt].func) {
				/* Clock framework API, Status ON */
				audio_ctrl_func(SNDP_HW_CLKGEN, STAT_ON);
				/* Path setting API call */
				g_clkgen_ctrl_func_tbl[iCnt].func(uiValue);
			}
			clkgen_reg_dump();
			sndp_log_debug_func("end\n");
			return ERROR_NONE;
		}
	}

	sndp_log_err("UnSupported value error end\n");
	return -EINVAL;
}


/*!
   @brief CLKGEN stop function

   @param[in]	none
   @param[out]	none

   @retval	none
 */
void clkgen_stop(void)
{
	sndp_log_debug_func("start\n");

	clkgen_reg_dump();

	/* Clock framework API, Status OFF */
	audio_ctrl_func(SNDP_HW_CLKGEN, STAT_OFF);

	sndp_log_debug_func("end\n");
}


/*!
   @brief CLKGEN registers setting (Voice call)

   @param[in]	uiValue		PCM type
   @param[out]	none

   @retval	none
 */
static void clkgen_voicecall(const u_int uiValue)
{
	/**********************************************
	 * PATH : SNDP_PLAYBACK_EARPIECE_INCALL
	 *        SNDP_PLAYBACK_SPEAKER_INCALL
	 *        SNDP_PLAYBACK_BLUETOOTH_INCALL
	 *        SNDP_PLAYBACK_HEADSET_INCALL
	 *        SNDP_PLAYBACK_SPEAKER_HEADSET_INCALL
	 *        SNDP_PLAYBACK_HEADPHONE_INCALL
	 *        SNDP_PLAYBACK_SPEAKER_HEADPHONE_INCALL
	 * FSI           : Slave
	 * MAXIM         : Slave
	 * CLKGEN        : Master
	 * Port          : PortA or B
	 * Ch            : 2
	 * Sampling rate : 16000
	 * Adjust mode   : OFF
	 **********************************************/

	/* Local variable declaration */
	u_int			devices		= 0;
	struct common_reg_table	*reg_tbl	= NULL;
	u_int			tbl_size	= 0;

	sndp_log_debug_func("start\n");

	/* Device check */
	devices = SNDP_GET_DEVICE_VAL(uiValue);
	/* SPEAKER, EARPIECE, WIREDHEADSET, WIREDHEADPHONE */
	if (devices != SNDP_BLUETOOTHSCO) {
		reg_tbl  = clkgen_reg_tbl_voicecallA;
		tbl_size = ARRAY_SIZE(clkgen_reg_tbl_voicecallA);
	/* BLUETOOTHSCO */
	} else {
		reg_tbl  = clkgen_reg_tbl_voicecallB;
		tbl_size = ARRAY_SIZE(clkgen_reg_tbl_voicecallB);
	}

	/* Register setting function call */
	common_set_register(SNDP_HW_CLKGEN, reg_tbl, tbl_size);

	sndp_log_debug_func("end\n");
}


/*!
   @brief CLKGEN registers setting (Playback)

   @param[in]	uiValue		PCM type
   @param[out]	none

   @retval	none
 */
static void clkgen_playback(const u_int uiValue)
{
	/*********************************************
	 * PATH : SNDP_PLAYBACK_XXXXX_NORMAL
	 *        SNDP_PLAYBACK_XXXXX_RINGTONE
	 *        SNDP_PLAYBACK_XXXXX_INCOMMUNICATION
	 * FSI           : Slave
	 * MAXIM         : Slave
	 * CLKGEN        : Master
	 * Port          : PortA or B
	 * Ch            : 2
	 * Sampling rate : 48000
	 *********************************************/

	/* Local variable declaration */
	u_int			devices		= 0;
	struct common_reg_table	*reg_tbl	= NULL;
	u_int			tbl_size	= 0;

	sndp_log_debug_func("start\n");

	/* Device check */
	devices = SNDP_GET_DEVICE_VAL(uiValue);
	/*
	 * SPEAKER, EARPIECE, WIREDHEADSET, WIREDHEADPHONE,
	 * AUXDIGITAL(HDMI)
	 */
	if (devices != SNDP_BLUETOOTHSCO) {
		reg_tbl  = clkgen_reg_tbl_playA;
		tbl_size = ARRAY_SIZE(clkgen_reg_tbl_playA);
	/* BLUETOOTHSCO */
	} else {
		reg_tbl  = clkgen_reg_tbl_playB;
		tbl_size = ARRAY_SIZE(clkgen_reg_tbl_playB);
	}

	/* Register setting function call */
	common_set_register(SNDP_HW_CLKGEN, reg_tbl, tbl_size);

	sndp_log_debug_func("end\n");
}


/*!
   @brief CLKGEN registers setting (Capture)

   @param[in]	uiValue		PCM type
   @param[out]	none

   @retval	none
 */
static void clkgen_capture(const u_int uiValue)
{
	/*********************************************
	 * PATH : SNDP_CAPTURE_MIC_NORMAL
	 *        SNDP_CAPTURE_MIC_RINGTONE
	 *        SNDP_CAPTURE_MIC_INCOMMUNICATION
	 *        SNDP_CAPTURE_HEADSET_NORMAL
	 *        SNDP_CAPTURE_HEADSET_RINGTONE
	 *        SNDP_CAPTURE_HEADSET_INCOMMUNICATION
	 * FSI            : Slave
	 * MAXIM          : Slave
	 * CLKGEN         : Master
	 * Port           : PortA or B
	 * Ch             : 2
	 * Sampling rate  : 48000
	 *********************************************/

	/* Local variable declaration */
	u_int			devices		= 0;
	struct common_reg_table	*reg_tbl	= NULL;
	u_int			tbl_size	= 0;

	sndp_log_debug_func("start\n");

	/* Device check */
	devices = SNDP_GET_DEVICE_VAL(uiValue);
	/* MIC, WIREDHEADSET */
/*	if (devices != ) { */
		reg_tbl  = clkgen_reg_tbl_captureA;
		tbl_size = ARRAY_SIZE(clkgen_reg_tbl_captureA);
	/* FM(T.B.D.) */
/*
 *	} else {
 *		reg_tbl = clkgen_reg_tbl_captureB;
 *		tbl_size = ARRAY_SIZE(clkgen_reg_tbl_captureB);
 *	}
 */
	/* Register setting function call */
	common_set_register(SNDP_HW_CLKGEN, reg_tbl, tbl_size);

	sndp_log_debug_func("end\n");
}


/*!
   @brief CLKGEN registers dump

   @param[in]	none
   @param[out]	none

   @retval	none
 */
void clkgen_reg_dump(void)
{
	sndp_log_reg_dump("===== CLKGEN Registers Dump Start =====\n");

	sndp_log_reg_dump("CLKG_SYSCTL   [%08X][%08lX]\n",
			ioread32((g_clkgen_Base + CLKG_SYSCTL)),
			(g_clkgen_Base + CLKG_SYSCTL));
	sndp_log_reg_dump("CLKG_PULSECTL [%08X][%08lX]\n",
			ioread32((g_clkgen_Base + CLKG_PULSECTL)),
			(g_clkgen_Base + CLKG_PULSECTL));
	sndp_log_reg_dump("CLKG_TIMSEL0  [%08X][%08lX]\n",
			ioread32((g_clkgen_Base + CLKG_TIMSEL0)),
			(g_clkgen_Base + CLKG_TIMSEL0));
	sndp_log_reg_dump("CLKG_TIMSEL1  [%08X][%08lX]\n",
			ioread32((g_clkgen_Base + CLKG_TIMSEL1)),
			(g_clkgen_Base + CLKG_TIMSEL1));
	sndp_log_reg_dump("CLKG_FSISEL   [%08X][%08lX]\n",
			ioread32((g_clkgen_Base + CLKG_FSISEL)),
			(g_clkgen_Base + CLKG_FSISEL));
	sndp_log_reg_dump("CLKG_FSIACOM  [%08X][%08lX]\n",
			ioread32((g_clkgen_Base + CLKG_FSIACOM)),
			(g_clkgen_Base + CLKG_FSIACOM));
	sndp_log_reg_dump("CLKG_FSIBCOM  [%08X][%08lX]\n",
			ioread32((g_clkgen_Base + CLKG_FSIBCOM)),
			(g_clkgen_Base + CLKG_FSIBCOM));
	sndp_log_reg_dump("CLKG_CPF0COM  [%08X][%08lX]\n",
			ioread32((g_clkgen_Base + CLKG_CPF0COM)),
			(g_clkgen_Base + CLKG_CPF0COM));
	sndp_log_reg_dump("CLKG_CPF1COM  [%08X][%08lX]\n",
			ioread32((g_clkgen_Base + CLKG_CPF1COM)),
			(g_clkgen_Base + CLKG_CPF1COM));
	sndp_log_reg_dump("CLKG_SPUVCOM  [%08X][%08lX]\n",
			ioread32((g_clkgen_Base + CLKG_SPUVCOM)),
			(g_clkgen_Base + CLKG_SPUVCOM));
	sndp_log_reg_dump("CLKG_AURCOM   [%08X][%08lX]\n",
			ioread32((g_clkgen_Base + CLKG_AURCOM)),
			(g_clkgen_Base + CLKG_AURCOM));
	sndp_log_reg_dump("CLKG_FFDCOM   [%08X][%08lX]\n",
			ioread32((g_clkgen_Base + CLKG_FFDCOM)),
			(g_clkgen_Base + CLKG_FFDCOM));
	sndp_log_reg_dump("CLKG_SLIMCOM  [%08X][%08lX]\n",
			ioread32((g_clkgen_Base + CLKG_SLIMCOM)),
			(g_clkgen_Base + CLKG_SLIMCOM));
	sndp_log_reg_dump("CLKG_FSIAAD   [%08X][%08lX]\n",
			ioread32((g_clkgen_Base + CLKG_FSIAAD)),
			(g_clkgen_Base + CLKG_FSIAAD));
	sndp_log_reg_dump("CLKG_FSIBAD   [%08X][%08lX]\n",
			ioread32((g_clkgen_Base + CLKG_FSIBAD)),
			(g_clkgen_Base + CLKG_FSIBAD));
	sndp_log_reg_dump("CLKG_CPF0AD   [%08X][%08lX]\n",
			ioread32((g_clkgen_Base + CLKG_CPF0AD)),
			(g_clkgen_Base + CLKG_CPF0AD));
	sndp_log_reg_dump("CLKG_CPF1AD   [%08X][%08lX]\n",
			ioread32((g_clkgen_Base + CLKG_CPF1AD)),
			(g_clkgen_Base + CLKG_CPF1AD));
	sndp_log_reg_dump("CLKG_SPUVAD   [%08X][%08lX]\n",
			ioread32((g_clkgen_Base + CLKG_SPUVAD)),
			(g_clkgen_Base + CLKG_SPUVAD));
	sndp_log_reg_dump("CLKG_AURAD    [%08X][%08lX]\n",
			ioread32((g_clkgen_Base + CLKG_AURAD)),
			(g_clkgen_Base + CLKG_AURAD));
	sndp_log_reg_dump("CLKG_FFDAD    [%08X][%08lX]\n",
			ioread32((g_clkgen_Base + CLKG_FFDAD)),
			(g_clkgen_Base + CLKG_FFDAD));
	sndp_log_reg_dump("CLKG_CLKADIV  [%08X][%08lX]\n",
			ioread32((g_clkgen_Base + CLKG_CLKADIV)),
			(g_clkgen_Base + CLKG_CLKADIV));
	sndp_log_reg_dump("CLKG_CLKSDIV  [%08X][%08lX]\n",
			ioread32((g_clkgen_Base + CLKG_CLKSDIV)),
			(g_clkgen_Base + CLKG_CLKSDIV));

	sndp_log_reg_dump("\n===== CLKGEN Registers Dump End =====\n");
}

#ifdef SOUND_TEST


void clkgen_play_test_start_a(void)
{
	/* Clock framework API, Status ON */
	audio_ctrl_func(SNDP_HW_CLKGEN, STAT_ON);

	iowrite32(0x00000000, (g_clkgen_Base + CLKG_SYSCTL));
	iowrite32(0x00212901, (g_clkgen_Base + CLKG_FSIACOM));
	iowrite32(0x00000001, (g_clkgen_Base + CLKG_PULSECTL));

	clkgen_reg_dump();
}

void clkgen_rec_test_start_a(void)
{
	/* Clock framework API, Status ON */
	audio_ctrl_func(SNDP_HW_CLKGEN, STAT_ON);

	iowrite32(0x00000000, (g_clkgen_Base + CLKG_SYSCTL));
	iowrite32(0x00000000, (g_clkgen_Base + CLKG_TIMSEL1));
	iowrite32(0x00212901, (g_clkgen_Base + CLKG_FSIACOM));
	iowrite32(0x00000001, (g_clkgen_Base + CLKG_PULSECTL));

	clkgen_reg_dump();
}

void clkgen_voice_test_start_a(void)
{
	/* Clock framework API, Status ON */
	audio_ctrl_func(SNDP_HW_CLKGEN, STAT_ON);

	iowrite32(0x00000000, (g_clkgen_Base + CLKG_SYSCTL));
	iowrite32(0x00212401, (g_clkgen_Base + CLKG_SPUVCOM));
	iowrite32(0x00000000, (g_clkgen_Base + CLKG_TIMSEL1));
	iowrite32(0x00212001, (g_clkgen_Base + CLKG_FSIACOM));
	iowrite32(0x00000011, (g_clkgen_Base + CLKG_PULSECTL));

	clkgen_reg_dump();
}

void clkgen_play_test_stop_a(void)
{
	clkgen_reg_dump();

	/* Clock framework API, Status OFF */
	audio_ctrl_func(SNDP_HW_CLKGEN, STAT_OFF);
}

void clkgen_rec_test_stop_a(void)
{
	clkgen_reg_dump();

	/* Clock framework API, Status OFF */
	audio_ctrl_func(SNDP_HW_CLKGEN, STAT_OFF);
}

void clkgen_voice_test_stop_a(void)
{
	clkgen_reg_dump();

	/* Clock framework API, Status OFF */
	audio_ctrl_func(SNDP_HW_CLKGEN, STAT_OFF);
}

#endif /* SOUND_TEST */

