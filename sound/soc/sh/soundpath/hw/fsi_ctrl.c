/* fsi_ctrl.c
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

#define __FSI_CTRL_NO_EXTERN__

#include <sound/soundpath/soundpath.h>
#include <sound/soundpath/common_extern.h>
#include <sound/soundpath/fsi_extern.h>
#include "fsi_ctrl.h"
#include <mach/common.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GLOBAL DATA Definitions
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// 1st path value
static u_int g_fsi_ui1stPathValue = SNDP_VALUE_INIT;

// FSI Control functions table
static ctrl_func_tbl_t g_fsi_func_tbl[] = {
	{ SNDP_PLAYBACK_EARPIECE_NORMAL,                    fsi_playback   },
	{ SNDP_PLAYBACK_EARPIECE_RINGTONE,                  fsi_playback   },
	{ SNDP_PLAYBACK_EARPIECE_INCALL,                    fsi_voicecall  },
	{ SNDP_PLAYBACK_EARPIECE_INCOMMUNICATION,           fsi_playback   },
	{ SNDP_PLAYBACK_SPEAKER_NORMAL,                     fsi_playback   },
	{ SNDP_PLAYBACK_SPEAKER_RINGTONE,                   fsi_playback   },
	{ SNDP_PLAYBACK_SPEAKER_INCALL,                     fsi_voicecall  },
	{ SNDP_PLAYBACK_SPEAKER_INCOMMUNICATION,            fsi_playback   },
	{ SNDP_PLAYBACK_BLUETOOTH_NORMAL,                   fsi_playback   },
	{ SNDP_PLAYBACK_BLUETOOTH_RINGTONE,                 fsi_playback   },
	{ SNDP_PLAYBACK_BLUETOOTH_INCALL,                   fsi_voicecall  },
	{ SNDP_PLAYBACK_BLUETOOTH_INCOMMUNICATION,          fsi_playback   },
	{ SNDP_PLAYBACK_HEADSET_NORMAL,                     fsi_playback   },
	{ SNDP_PLAYBACK_HEADSET_RINGTONE,                   fsi_playback   },
	{ SNDP_PLAYBACK_HEADSET_INCALL,                     fsi_voicecall  },
	{ SNDP_PLAYBACK_HEADSET_INCOMMUNICATION,            fsi_playback   },
	{ SNDP_PLAYBACK_SPEAKER_HEADSET_NORMAL,             fsi_playback   },
	{ SNDP_PLAYBACK_SPEAKER_HEADSET_RINGTONE,           fsi_playback   },
	{ SNDP_PLAYBACK_SPEAKER_HEADSET_INCALL,             fsi_voicecall  },
	{ SNDP_PLAYBACK_SPEAKER_HEADSET_INCOMMUNICATION,    fsi_playback   },
	{ SNDP_PLAYBACK_HEADPHONE_NORMAL,                   fsi_playback   },
	{ SNDP_PLAYBACK_HEADPHONE_RINGTONE,                 fsi_playback   },
	{ SNDP_PLAYBACK_HEADPHONE_INCALL,                   fsi_voicecall  },
	{ SNDP_PLAYBACK_HEADPHONE_INCOMMUNICATION,          fsi_playback   },
	{ SNDP_PLAYBACK_SPEAKER_HEADPHONE_NORMAL,           fsi_playback   },
	{ SNDP_PLAYBACK_SPEAKER_HEADPHONE_RINGTONE,         fsi_playback   },
	{ SNDP_PLAYBACK_SPEAKER_HEADPHONE_INCALL,           fsi_voicecall  },
	{ SNDP_PLAYBACK_SPEAKER_HEADPHONE_INCOMMUNICATION,  fsi_playback   },
	{ SNDP_PLAYBACK_AUXDIGITAL_NORMAL,                  fsi_playback   },
	{ SNDP_PLAYBACK_AUXDIGITAL_RINGTONE,                NULL           },
	{ SNDP_PLAYBACK_AUXDIGITAL_INCALL,                  NULL           },
	{ SNDP_PLAYBACK_AUXDIGITAL_INCOMMUNICATION,         NULL           },
	{ SNDP_CAPTURE_MIC_NORMAL,                          fsi_capture    },
	{ SNDP_CAPTURE_MIC_RINGTONE,                        fsi_capture    },
	{ SNDP_CAPTURE_MIC_INCALL,                          NULL           },
	{ SNDP_CAPTURE_MIC_INCOMMUNICATION,                 fsi_capture    },
	{ SNDP_CAPTURE_HEADSET_NORMAL,                      fsi_capture    },
	{ SNDP_CAPTURE_HEADSET_RINGTONE,                    fsi_capture    },
	{ SNDP_CAPTURE_HEADSET_INCALL,                      NULL           },
	{ SNDP_CAPTURE_HEADSET_INCOMMUNICATION,             fsi_capture    },
	{ SNDP_CAPTURE_BLUETOOTH_NORMAL,                    NULL           },
	{ SNDP_CAPTURE_BLUETOOTH_RINGTONE,                  NULL           },
	{ SNDP_CAPTURE_BLUETOOTH_INCALL,                    NULL           },
	{ SNDP_CAPTURE_BLUETOOTH_INCOMMUNICATION,           NULL           },
};


// Table for Voice call(PortA)
static common_reg_table fsi_reg_tbl_voicecallA[] = {
//    Register          Value       Delay time
	{ FSI_CLK_SEL,		0x00000001,	0 },			// Bus clock(MP clock)
	{ FSI_ACK_MD,		0x00000100,	0 },			// 512 fs, 64bit/fs, DIIS:Slave, DOIS:Slave
	{ FSI_ACK_RV,		0x00000001,	0 },			// LRS:Clock not inverted, BRS:Clock inverted
	{ FSI_DO_FMT,		0x00000030,	0 },			// 24bits, PCM format, I2S
	{ FSI_DI_FMT,		0x00000030,	0 },			// 24bits, PCM format, I2S
	{ FSI_MUTE,			0x00001111,	0 },			// MUTE OFF
};

// Table for Voice call(PortB)
static common_reg_table fsi_reg_tbl_voicecallB[] = {
//    Register                              Value       Delay time
	{ FSI_CLK_SEL,							0x00000001,	0 },		// Bus clock(MP clock)
	{ (FSI_ACK_MD + FSI_PORTB_OFFSET),		0x00000100,	0 },		// 512 fs, 64bit/fs, DIIS:Slave, DOIS:Slave
	{ (FSI_ACK_RV + FSI_PORTB_OFFSET),		0x00000001,	0 },		// LRS:Clock not inverted, BRS:Clock inverted
	{ (FSI_DO_FMT + FSI_PORTB_OFFSET),		0x00000030,	0 },		// 24bits, PCM format, I2S
	{ (FSI_DI_FMT + FSI_PORTB_OFFSET),		0x00000030,	0 },		// 24bits, PCM format, I2S
	{ FSI_MUTE,								0x00001111,	0 },		// MUTE OFF
};

// Table for Playback(PortA)
static common_reg_table fsi_reg_tbl_playA[] = {
//    Register          Value       Delay time
	{ FSI_CLK_SEL,		0x00000001,	0 },			// Bus clock(MP clock)
	{ FSI_ACK_MD,		0x00000100,	0 },			// 512 fs, 64bit/fs, DIIS:Slave, DOIS:Slave
	{ FSI_ACK_RV,		0x00000001,	0 },			// LRS:Clock not inverted, BRS:Clock inverted
	{ FSI_DO_FMT,		0x00000030,	0 },			// 24bits, PCM format, I2S
	{ FSI_DI_FMT,		0x00000030,	0 },			// 24bits, PCM format, I2S
	{ FSI_MUTE,			0x00001111,	0 },			// MUTE OFF
};

// Table for Playback(PortB)
static common_reg_table fsi_reg_tbl_playB[] = {
//    Register          Value       Delay time
	{ FSI_CLK_SEL,							0x00000001,	0 },		// Bus clock(MP clock)
	{ (FSI_ACK_MD + FSI_PORTB_OFFSET),		0x00000100,	0 },		// 512 fs, 64bit/fs, DIIS:Slave, DOIS:Slave
	{ (FSI_ACK_RV + FSI_PORTB_OFFSET),		0x00000001,	0 },		// LRS:Clock not inverted, BRS:Clock inverted
	{ (FSI_DO_FMT + FSI_PORTB_OFFSET),		0x00000030,	0 },		// 24bits, PCM format, I2S
	{ (FSI_DI_FMT + FSI_PORTB_OFFSET),		0x00000030,	0 },		// 24bits, PCM format, I2S
	{ FSI_MUTE,								0x00001111,	0 },		// MUTE OFF
};

// Table for Capture(PortA)
static common_reg_table fsi_reg_tbl_captureA[] = {
//    Register          Value       Delay time
	{ FSI_CLK_SEL,		0x00000001,	0 },			// Bus clock(MP clock)
	{ FSI_ACK_MD,		0x00000100,	0 },			// 512 fs, 64bit/fs, DIIS:Slave, DOIS:Slave
	{ FSI_ACK_RV,		0x00000001,	0 },			// LRS:Clock not inverted, BRS:Clock inverted
	{ FSI_DO_FMT,		0x00000030,	0 },			// 24bits, PCM format, I2S
	{ FSI_DI_FMT,		0x00000030,	0 },			// 24bits, PCM format, I2S
	{ FSI_MUTE,			0x00001111,	0 },			// MUTE OFF
};

// Table for Capture(PortB)
static common_reg_table fsi_reg_tbl_captureB[] = {
//    Register          Value       Delay time
	{ FSI_CLK_SEL,							0x00000001,	0 },		// Bus clock(MP clock)
	{ (FSI_ACK_MD + FSI_PORTB_OFFSET),		0x00000100,	0 },		// 512 fs, 64bit/fs, DIIS:Slave, DOIS:Slave
	{ (FSI_ACK_RV + FSI_PORTB_OFFSET),		0x00000001,	0 },		// LRS:Clock not inverted, BRS:Clock inverted
	{ (FSI_DI_FMT + FSI_PORTB_OFFSET),		0x00000030,	0 },		// 24bits, PCM format, I2S
	{ FSI_MUTE,								0x00001111,	0 },		// MUTE OFF
};


/*!
   @brief FSI start function

   @param[in]	uiValue		PCM type
   @param[out]	None

   @retval		0			Successful
   @retval		-EINVAL		Invalid argument
 */
int fsi_start(const u_int uiValue)
{
	// Local variable declaration
	int iCnt;

	sndp_log_debug_func("start [0x%08x]\n", uiValue);

	// Call of function of each PATH
	for (iCnt=0; ARRAY_SIZE(g_fsi_func_tbl) > iCnt; iCnt++) {
		// uiValue check
		if (uiValue == g_fsi_func_tbl[iCnt].uiValue) {
			// Function pointer check
			if (NULL != g_fsi_func_tbl[iCnt].func) {
				g_fsi_ui1stPathValue = uiValue;
				// Clock framework API, Status ON
				audio_ctrl_func(SNDP_HW_FSI, STAT_ON);
				// Path setting API call
				g_fsi_func_tbl[iCnt].func(uiValue);
			}
			fsi_reg_dump(g_fsi_ui1stPathValue);
			sndp_log_debug_func("end\n");
			return ERROR_NONE;
		}
	}

	sndp_log_err("UnSupported value error end\n");
	return -EINVAL;
}


/*!
   @brief FSI stop function

   @param[in]	None
   @param[out]	None

   @retval		None
 */
void fsi_stop(void)
{
	sndp_log_debug_func("start\n");

	fsi_reg_dump(g_fsi_ui1stPathValue);

	// Clock framework API, Status OFF
	audio_ctrl_func(SNDP_HW_FSI, STAT_OFF);

	// PCM clear
	g_fsi_ui1stPathValue = SNDP_VALUE_INIT;

	sndp_log_debug_func("end\n");
}


/*!
   @brief FSI registers setting (Voice call)

   @param[in]	None
   @param[out]	None

   @retval		None
 */
static void fsi_voicecall(const u_int uiValue)
{
	/*************************************************
	 * PATH : SNDP_PLAYBACK_EARPIECE_INCALL,
	 *        SNDP_PLAYBACK_SPEAKER_INCALL,
	 *        SNDP_PLAYBACK_BLUETOOTH_INCALL,
	 *        SNDP_PLAYBACK_HEADSET_INCALL,
	 *        SNDP_PLAYBACK_SPEAKER_HEADSET_INCALL,
	 *        SNDP_PLAYBACK_HEADPHONE_INCALL,
	 *        SNDP_PLAYBACK_SPEAKER_HEADPHONE_INCALL,
	 * FSI           : Slave
	 * MAXIM         : Slave
	 * CLKGEN        : Master
	 * Port          : PortA or B
	 * Ch            : 2
	 * Sampling rate : 16000
	 * Adjust mode   : OFF
	 *************************************************/

	// Local variable declaration
	u_int				devices		= 0;
	common_reg_table	*reg_tbl	= NULL;
	u_int				tbl_size	= 0;

	sndp_log_debug_func("start\n");

	// Device check
	devices = SNDP_GET_DEVICE_VAL(uiValue);
	// SPEAKER, EARPIECE, WIREDHEADSET, WIREDHEADPHONE
	if (devices != SNDP_BLUETOOTHSCO) {
		reg_tbl  = fsi_reg_tbl_voicecallA;
		tbl_size = ARRAY_SIZE(fsi_reg_tbl_voicecallA);
	// BLUETOOTHSCO
	} else {
		reg_tbl  = fsi_reg_tbl_voicecallB;
		tbl_size = ARRAY_SIZE(fsi_reg_tbl_voicecallB);
	}

	// Register setting function call
	common_set_register(SNDP_HW_FSI, reg_tbl, tbl_size);

	sndp_log_debug_func("end\n");
}


/*!
   @brief FSI registers setting (Playback)

   @param[in]	None
   @param[out]	None

   @retval		None
 */
static void fsi_playback(const u_int uiValue)
{
	/********************************************
	 * PATH : SNDP_PLAYBACK_XXXXX_NORMAL
	 *        SNDP_PLAYBACK_XXXXX_RINGTONE
	 *        SNDP_PLAYBACK_XXXXX_INCOMMUNICATION
	 * FSI           : Slave
	 * MAXIM         : Slave
	 * CLKGEN        : Master
	 * Port          : PortA or B
	 * Ch            : 2
	 * Sampling rate : 48000
	 ********************************************/

	// Local variable declaration
	u_int				devices		= 0;
	common_reg_table	*reg_tbl	= NULL;
	u_int				tbl_size	= 0;

	sndp_log_debug_func("start\n");

	// Device check
	devices = SNDP_GET_DEVICE_VAL(uiValue);
	// SPEAKER, EARPIECE, WIREDHEADSET, WIREDHEADPHONE, AUXDIGITAL(HDMI)
	if (devices != SNDP_BLUETOOTHSCO) {
		reg_tbl  = fsi_reg_tbl_playA;
		tbl_size = ARRAY_SIZE(fsi_reg_tbl_playA);
	// BLUETOOTHSCO
	} else {
		reg_tbl  = fsi_reg_tbl_playB;
		tbl_size = ARRAY_SIZE(fsi_reg_tbl_playB);
	}

	// Register setting function call
	common_set_register(SNDP_HW_FSI, reg_tbl, tbl_size);

	sndp_log_debug_func("end\n");
}


/*!
   @brief FSI registers setting (Capture)

   @param[in]	None
   @param[out]	None

   @retval		None
 */
static void fsi_capture(const u_int uiValue)
{
	/********************************************
	 * PATH : SNDP_CAPTURE_MIC_NORMAL,
	 *        SNDP_CAPTURE_MIC_RINGTONE,
	 *        SNDP_CAPTURE_MIC_INCOMMUNICATION,
	 *        SNDP_CAPTURE_HEADSET_NORMAL,
	 *        SNDP_CAPTURE_HEADSET_RINGTONE,
	 *        SNDP_CAPTURE_HEADSET_INCOMMUNICATION,
	 * FSI           : Slave
	 * MAXIM         : Slave
	 * CLKGEN        : Master
	 * Port          : PortA or B
	 * Ch            : 2
	 * Sampling rate : 48000
	 ********************************************/

	// Local variable declaration
	u_int				devices		= 0;
	common_reg_table	*reg_tbl	= NULL;
	u_int				tbl_size	= 0;

	sndp_log_debug_func("start\n");

	// Device check
	devices = SNDP_GET_DEVICE_VAL(uiValue);
	// MIC, WIREDHEADSET
//	if (devices != ) {
		reg_tbl  = fsi_reg_tbl_captureA;
		tbl_size = ARRAY_SIZE(fsi_reg_tbl_captureA);
	// FM(T.B.D.)
//	} else {
//		reg_tbl  = fsi_reg_tbl_captureB;
//		tbl_size = ARRAY_SIZE(fsi_reg_tbl_captureB);
//	}

	// Register setting function call
	common_set_register(SNDP_HW_FSI, reg_tbl, tbl_size);

	sndp_log_debug_func("end\n");
}


/*!
   @brief DMA process start function

   @param[in]	
   @param[out]	

   @retval		
 */
void fsi_dma_start(void)
{
	sndp_log_debug_func("start\n");

	// T.B.D.

	sndp_log_debug_func("end\n");
}


/*!
   @brief DMA process stop function

   @param[in]	
   @param[out]	

   @retval		
 */
void fsi_dma_stop(void)
{
	sndp_log_debug_func("start\n");

	// T.B.D.

	sndp_log_debug_func("end\n");
}


/*!
   @brief FSI registers dump

   @param[in]	None
   @param[out]	None

   @retval		None
 */
void fsi_reg_dump(const u_int uiValue)
{
	sndp_log_reg_dump("===== FSI Registers Dump Start =====\n");

	if (SNDP_BLUETOOTHSCO != SNDP_GET_DEVICE_VAL(uiValue)) {
		sndp_log_reg_dump("\n<PortA>\n");
		sndp_log_reg_dump("DO_FMT     [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_DO_FMT)   ,(g_fsi_Base + FSI_DO_FMT));
		sndp_log_reg_dump("DOFF_CTL   [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_DOFF_CTL) ,(g_fsi_Base + FSI_DOFF_CTL));
		sndp_log_reg_dump("DOFF_ST    [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_DOFF_ST)  ,(g_fsi_Base + FSI_DOFF_ST));
		sndp_log_reg_dump("DI_FMT     [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_DI_FMT)   ,(g_fsi_Base + FSI_DI_FMT));
		sndp_log_reg_dump("DIFF_CTL   [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_DIFF_CTL) ,(g_fsi_Base + FSI_DIFF_CTL));
		sndp_log_reg_dump("DIFF_ST    [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_DIFF_ST)  ,(g_fsi_Base + FSI_DIFF_ST));
		sndp_log_reg_dump("ACK_MD     [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_ACK_MD)   ,(g_fsi_Base + FSI_ACK_MD));
		sndp_log_reg_dump("ACK_RV     [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_ACK_RV)   ,(g_fsi_Base + FSI_ACK_RV));
//		sndp_log_reg_dump("DIDT       [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_DIDT)     ,(g_fsi_Base + FSI_DIDT));
//		sndp_log_reg_dump("DODT       [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_DODT)     ,(g_fsi_Base + FSI_DODT));
		sndp_log_reg_dump("MUTE_ST    [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_MUTE_ST)  ,(g_fsi_Base + FSI_MUTE_ST));
		sndp_log_reg_dump("OUT_DMAC   [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_OUT_DMAC) ,(g_fsi_Base + FSI_OUT_DMAC));
		sndp_log_reg_dump("OUT_SEL    [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_OUT_SEL)  ,(g_fsi_Base + FSI_OUT_SEL));
		sndp_log_reg_dump("OUT_SPST   [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_OUT_SPST) ,(g_fsi_Base + FSI_OUT_SPST));
		sndp_log_reg_dump("IN_DMAC    [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_IN_DMAC)  ,(g_fsi_Base + FSI_IN_DMAC));
		sndp_log_reg_dump("IN_SEL     [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_IN_SEL)   ,(g_fsi_Base + FSI_IN_SEL));
	} else {
		sndp_log_reg_dump("\n<PortB>\n");
		sndp_log_reg_dump("DO_FMT     [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_DO_FMT   + FSI_PORTB_OFFSET)  ,(g_fsi_Base + FSI_DO_FMT   + FSI_PORTB_OFFSET));
		sndp_log_reg_dump("DOFF_CTL   [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_DOFF_CTL + FSI_PORTB_OFFSET)  ,(g_fsi_Base + FSI_DOFF_CTL + FSI_PORTB_OFFSET));
		sndp_log_reg_dump("DOFF_ST    [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_DOFF_ST  + FSI_PORTB_OFFSET)  ,(g_fsi_Base + FSI_DOFF_ST  + FSI_PORTB_OFFSET));
		sndp_log_reg_dump("DI_FMT     [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_DI_FMT   + FSI_PORTB_OFFSET)  ,(g_fsi_Base + FSI_DI_FMT   + FSI_PORTB_OFFSET));
		sndp_log_reg_dump("DIFF_CTL   [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_DIFF_CTL + FSI_PORTB_OFFSET)  ,(g_fsi_Base + FSI_DIFF_CTL + FSI_PORTB_OFFSET));
		sndp_log_reg_dump("DIFF_ST    [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_DIFF_ST  + FSI_PORTB_OFFSET)  ,(g_fsi_Base + FSI_DIFF_ST  + FSI_PORTB_OFFSET));
		sndp_log_reg_dump("ACK_MD     [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_ACK_MD   + FSI_PORTB_OFFSET)  ,(g_fsi_Base + FSI_ACK_MD   + FSI_PORTB_OFFSET));
		sndp_log_reg_dump("ACK_RV     [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_ACK_RV   + FSI_PORTB_OFFSET)  ,(g_fsi_Base + FSI_ACK_RV   + FSI_PORTB_OFFSET));
//		sndp_log_reg_dump("DIDT       [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_DIDT     + FSI_PORTB_OFFSET)  ,(g_fsi_Base + FSI_DIDT     + FSI_PORTB_OFFSET));
//		sndp_log_reg_dump("DODT       [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_DODT     + FSI_PORTB_OFFSET)  ,(g_fsi_Base + FSI_DODT     + FSI_PORTB_OFFSET));
		sndp_log_reg_dump("MUTE_ST    [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_MUTE_ST  + FSI_PORTB_OFFSET)  ,(g_fsi_Base + FSI_MUTE_ST  + FSI_PORTB_OFFSET));
		sndp_log_reg_dump("OUT_DMAC   [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_OUT_DMAC + FSI_PORTB_OFFSET)  ,(g_fsi_Base + FSI_OUT_DMAC + FSI_PORTB_OFFSET));
		sndp_log_reg_dump("OUT_SEL    [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_OUT_SEL  + FSI_PORTB_OFFSET)  ,(g_fsi_Base + FSI_OUT_SEL  + FSI_PORTB_OFFSET));
		sndp_log_reg_dump("OUT_SPST   [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_OUT_SPST + FSI_PORTB_OFFSET)  ,(g_fsi_Base + FSI_OUT_SPST + FSI_PORTB_OFFSET));
		sndp_log_reg_dump("IN_DMAC    [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_IN_DMAC  + FSI_PORTB_OFFSET)  ,(g_fsi_Base + FSI_IN_DMAC  + FSI_PORTB_OFFSET));
		sndp_log_reg_dump("IN_SEL     [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_IN_SEL   + FSI_PORTB_OFFSET)  ,(g_fsi_Base + FSI_IN_SEL   + FSI_PORTB_OFFSET));
	}
	sndp_log_reg_dump("TMR_CTL    [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_TMR_CTL)    ,(g_fsi_Base + FSI_TMR_CTL));
	sndp_log_reg_dump("TMR_CLR    [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_TMR_CLR)    ,(g_fsi_Base + FSI_TMR_CLR));
	sndp_log_reg_dump("INT_SEL    [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_INT_SEL)    ,(g_fsi_Base + FSI_INT_SEL));
	sndp_log_reg_dump("INT_CLR    [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_INT_CLR)    ,(g_fsi_Base + FSI_INT_CLR));
	sndp_log_reg_dump("CPU_INT_ST [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_CPU_INT_ST) ,(g_fsi_Base + FSI_CPU_INT_ST));
	sndp_log_reg_dump("CPU_IEMSK  [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_CPU_IEMSK)  ,(g_fsi_Base + FSI_CPU_IEMSK));
	sndp_log_reg_dump("CPU_IMSK   [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_CPU_IMSK)   ,(g_fsi_Base + FSI_CPU_IMSK));
	sndp_log_reg_dump("DSP_INT_ST [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_DSP_INT_ST) ,(g_fsi_Base + FSI_DSP_INT_ST));
	sndp_log_reg_dump("DSP_IEMSK  [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_DSP_IEMSK)  ,(g_fsi_Base + FSI_DSP_IEMSK));
	sndp_log_reg_dump("DSP_IMSK   [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_DSP_IMSK)   ,(g_fsi_Base + FSI_DSP_IMSK));
	sndp_log_reg_dump("MUTE       [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_MUTE)       ,(g_fsi_Base + FSI_MUTE));
	sndp_log_reg_dump("ACK_RST    [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_ACK_RST)    ,(g_fsi_Base + FSI_ACK_RST));
	sndp_log_reg_dump("SOFT_RST   [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_SOFT_RST)   ,(g_fsi_Base + FSI_SOFT_RST));
	sndp_log_reg_dump("FIFO_SZ    [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_FIFO_SZ)    ,(g_fsi_Base + FSI_FIFO_SZ));
	sndp_log_reg_dump("CLK_SEL    [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_CLK_SEL)    ,(g_fsi_Base + FSI_CLK_SEL));
	sndp_log_reg_dump("SWAP_SEL   [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_SWAP_SEL)   ,(g_fsi_Base + FSI_SWAP_SEL));
	sndp_log_reg_dump("HPB_SRST   [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_HPB_SRST)   ,(g_fsi_Base + FSI_HPB_SRST));

	sndp_log_reg_dump("\n===== FSI Registers Dump End =====\n");
}

#ifdef SOUND_TEST

#define TEST_NONE  0
#define TEST_PLAY  1
#define TEST_REC   2
#define TEST_VOICE 4

#define RET_NONE 0
#define RET_NG -1


static int fsi_test_status_a = TEST_NONE;
static char *fsi_play_test_buf;
static u_int fsi_play_test_buf_len;
static u_int fsi_play_test_byte_offset;

// Prototype declaration
int fsi_test_init_a(void);
void fsi_test_fifo_write_a(void);
void fsi_test_fifo_read_a(void);
void fsi_test_interrupt_a(void);
void fsi_test_stop_a(void);
void fsi_test_reg_dump(void);

// fsi play test function
int fsi_play_test_start_a(char *buf, u_int size)
{
	// Local variable declaration
	int ret;
	u32 reg;

	// FSI initialization
	ret = fsi_test_init_a();
	if (RET_NONE != ret) {
		return RET_NG;
	}

	// Set play information
	fsi_play_test_buf = buf;
	fsi_play_test_buf_len = size;
	fsi_play_test_byte_offset = 0;

	// FIFO Clear
	iowrite32(0x00100001, (g_fsi_Base + FSI_DOFF_CTL));	// FSI_DOFF_CTL

	// Setting valid data, Package in the back
	iowrite32(0x00000010, (g_fsi_Base + FSI_OUT_DMAC));	// OUT_DMAC

	iowrite32(0x00000100, (g_fsi_Base + FSI_ACK_MD));	// ACK_MD
	iowrite32(0x00000001, (g_fsi_Base + FSI_ACK_RV));	// ACK_RV
	iowrite32(0x00000030, (g_fsi_Base + FSI_DO_FMT));	// DO_FMT(Ste)
	iowrite32(0x00001111, (g_fsi_Base + FSI_MUTE));		// MUTE

	iowrite32(0x00000001, (g_fsi_Base + FSI_ACK_RST));	// FSI_ACK_RST

	// IR reset
	reg = ioread32(g_fsi_Base + FSI_SOFT_RST);
	reg &= ~(1 << 4);
	iowrite32(reg, (g_fsi_Base + FSI_SOFT_RST));		// FSI_SOFT_RST

	reg = ioread32(g_fsi_Base + FSI_SOFT_RST);
	reg |= (1 << 4);
	iowrite32(reg, (g_fsi_Base + FSI_SOFT_RST));		// FSI_SOFT_RST

#ifndef NO_INTURRUPT
	// Fifo write
	fsi_test_fifo_write_a();
#endif // NO_INTURRUPT

	reg = ioread32(g_fsi_Base + FSI_CPU_INT_ST);
	reg &= ~(1 << 0);
	reg |= 0 & (1 << 0);
	iowrite32(reg, (g_fsi_Base + FSI_CPU_INT_ST));		// CPU_INT_ST

	// FSI start
	fsi_test_status_a = TEST_PLAY;

	fsi_test_reg_dump();

#ifdef NO_INTURRUPT
	udelay(500);
#endif // NO_INTURRUPT

	return RET_NONE;
}

// fsi play test stop function
void fsi_play_test_stop_a(void)
{
	fsi_test_reg_dump();

	// FSI stop
	fsi_test_stop_a();
}


static char *fsi_rec_test_buf;
static u_int fsi_rec_test_buf_len;
static u_int fsi_rec_test_byte_offset;

// fsi rec test function
int fsi_rec_test_start_a(char *buf, u_int size)
{
	// Local variable declaration
	int ret;
	u32 reg;

	// FSI initialization
	ret = fsi_test_init_a();
	if (RET_NONE != ret) {
		return RET_NG;
	}

	// Set rec information
	fsi_rec_test_buf = buf;
	fsi_rec_test_buf_len = size;
	fsi_rec_test_byte_offset = 0;

	// FIFO Clear
	iowrite32(0x00100001, (g_fsi_Base + FSI_DIFF_CTL));	// FSI_DIFF_CTL

	// Setting valid data, Package in the back
	iowrite32(0x00000010, (g_fsi_Base + FSI_IN_DMAC));	// IN_DMAC

	iowrite32(0x00000100, (g_fsi_Base + FSI_ACK_MD));	// ACK_MD
	iowrite32(0x00000001, (g_fsi_Base + FSI_ACK_RV));	// ACK_RV
	iowrite32(0x00000030, (g_fsi_Base + FSI_DI_FMT));	// DI_FMT(Ste)
	iowrite32(0x00001111, (g_fsi_Base + FSI_MUTE));		// MUTE

	iowrite32(0x00000001, (g_fsi_Base + FSI_ACK_RST));	// FSI_ACK_RST

	// IR reset
	reg = ioread32(g_fsi_Base + FSI_SOFT_RST);
	reg &= ~(1 << 4);
	iowrite32(reg, (g_fsi_Base + FSI_SOFT_RST));		// FSI_SOFT_RST

	reg = ioread32(g_fsi_Base + FSI_SOFT_RST);
	reg |= (1 << 4);
	iowrite32(reg, (g_fsi_Base + FSI_SOFT_RST));		// FSI_SOFT_RST

#ifndef NO_INTURRUPT
	// Fifo read
	fsi_test_fifo_read_a();
#endif // NO_INTURRUPT

	reg = ioread32(g_fsi_Base + FSI_CPU_INT_ST);
	reg &= ~(1 << 0);
	reg |= 0 & (1 << 0);
	iowrite32(reg, (g_fsi_Base + FSI_CPU_INT_ST));		// CPU_INT_ST

	// FSI start
	fsi_test_status_a = TEST_REC;

	fsi_test_reg_dump();

#ifdef NO_INTURRUPT
	udelay(500);
#endif // NO_INTURRUPT

	return RET_NONE;
}

// fsi rec test stop function
int fsi_rec_test_stop_a(void)
{
	int size;

	fsi_test_reg_dump();

	// FSI stop
	fsi_test_stop_a();

	size = fsi_rec_test_byte_offset;

	return size;
}

// fsi test initialization function
int fsi_test_init_a()
{
	int reg;

	// Already FSI started
	if(TEST_NONE != fsi_test_status_a) {
		// Simultaneous operation is to expand later.
		return RET_NG;
	}

	// Clock framework API, Status ON
	audio_ctrl_func(SNDP_HW_FSI, STAT_ON);

	// IRQ Disable
	reg = ioread32(g_fsi_Base + FSI_CPU_IMSK);
	reg &= ~(1 << 0);
	iowrite32(reg, (g_fsi_Base + FSI_CPU_IMSK));		// CPU_IMSK

	reg = ioread32(g_fsi_Base + FSI_CPU_IEMSK);
	reg &= ~(1 << 0);
	iowrite32(reg, (g_fsi_Base + FSI_CPU_IEMSK));		// CPU_IEMSK

	reg = ioread32(g_fsi_Base + FSI_CPU_INT_ST);
	reg &= ~(1 << 0);
	iowrite32(reg, (g_fsi_Base + FSI_CPU_INT_ST));		// CPU_INT_ST

	return RET_NONE;
}

// fsi test fifo write function
void fsi_test_fifo_write_a(void)
{
	// Local variable declaration
	int i;
	int reg;
	int send;
	int residue;
	int fifo_free;
	int fifo_max = 128;
	u8 *start;

#ifdef NO_INTURRUPT
while(1) {
#endif // NO_INTURRUPT

	// Get send num
	send = (fsi_play_test_buf_len - fsi_play_test_byte_offset) / 2;

	// Get FIFO free size
	reg = ioread32(g_fsi_Base + FSI_DOFF_ST);
	residue = 0x1ff & (reg >> 8);
	fifo_free = fifo_max - residue;

	// Size check
	if ((fifo_free * 4 / 2) < send) {
		send = (fifo_free * 4 / 2);
	}

	start = fsi_play_test_buf;
	start += fsi_play_test_byte_offset;

	for (i = 0; i < send; i++)
		iowrite32(((u32)*((u16 *)start + i) << 8), (g_fsi_Base + FSI_DODT));

	fsi_play_test_byte_offset += (send * 2);

	// All data complete
	if (0 >= (fsi_play_test_buf_len - fsi_play_test_byte_offset)) {
		fsi_play_test_stop_a();
		return;
	}

#ifdef NO_INTURRUPT
	udelay(500);
}
#endif // NO_INTURRUPT

	// irq_enable
	reg = ioread32(g_fsi_Base + FSI_CPU_IMSK);
	reg |= (1 << 0);
	iowrite32(reg, (g_fsi_Base + FSI_CPU_IMSK));	// CPU_IMSK

	reg = ioread32(g_fsi_Base + FSI_CPU_IEMSK);
	reg |= (1 << 0);
	iowrite32(reg, (g_fsi_Base + FSI_CPU_IEMSK));	// CPU_IEMSK

}

// fsi test fifo read function
void fsi_test_fifo_read_a(void)
{
	// Local variable declaration
	int i;
	int reg;
	int read;
	int fifo_fill;
	u8 *start;

#ifdef NO_INTURRUPT
	int count = 0;
while(1) {
#endif // NO_INTURRUPT

	// Get read num
	read = (fsi_rec_test_buf_len - fsi_rec_test_byte_offset) / 2;

	// Get FIFO free size
	reg = ioread32(g_fsi_Base + FSI_DIFF_ST);
	fifo_fill = 0x1ff & (reg >> 8);

	// Size check
	if ((fifo_fill * 4 / 2) < read) {
		read = (fifo_fill * 4 / 2);
	}

	start = fsi_rec_test_buf;
	start += fsi_rec_test_byte_offset;

	for (i = 0; i < read; i++) {
		*((u16 *)start + i) = (u16)(ioread32(g_fsi_Base + FSI_DIDT) >> 8);
		//printk("%x\n", *((u16 *)start + i));
	}
	fsi_rec_test_byte_offset += (read * 2);

	// All data complete
	if (0 >= (fsi_rec_test_buf_len - fsi_rec_test_byte_offset)) {
		fsi_rec_test_stop_a();
		return;
	}

#ifdef NO_INTURRUPT
	udelay(500);
	count++;
}
#endif // NO_INTURRUPT

	// irq_enable
	reg = ioread32(g_fsi_Base + FSI_CPU_IMSK);
	reg |= (1 << 0);
	iowrite32(reg, (g_fsi_Base + FSI_CPU_IMSK));	// CPU_IMSK

	reg = ioread32(g_fsi_Base + FSI_CPU_IEMSK);
	reg |= (1 << 0);
	iowrite32(reg, (g_fsi_Base + FSI_CPU_IEMSK));	// CPU_IEMSK

}

// fsi test interrupt function
void fsi_test_interrupt_a(void)
{
	// Local variable declaration
	u_int reg;

	reg = ioread32(g_fsi_Base + FSI_SOFT_RST);
	reg &= ~(1 << 4);
	iowrite32(reg, (g_fsi_Base + FSI_SOFT_RST));		// FSI_SOFT_RST

	reg = ioread32(g_fsi_Base + FSI_SOFT_RST);
	reg &= ~(1 << 4);
	reg |= (1 << 4);
	iowrite32(reg, (g_fsi_Base + FSI_SOFT_RST));		// FSI_SOFT_RST

	if (TEST_PLAY == fsi_test_status_a) {
		fsi_test_fifo_write_a();
	} else {
		fsi_test_fifo_read_a();
	}

	reg = ioread32(g_fsi_Base + FSI_CPU_INT_ST);
	reg &= ~(1 << 0);
	iowrite32(reg, (g_fsi_Base + FSI_CPU_INT_ST));		// CPU_INT_ST
}

// fsi test stop function
void fsi_test_stop_a(void)
{
	// Local variable declaration
	u_int reg;

	// Already FSI stoped
	if (TEST_NONE == fsi_test_status_a) {
		// Simultaneous operation with the Capture is to expand later.
		return;
	}

	// IRQ Disable
	reg = ioread32(g_fsi_Base + FSI_CPU_IMSK);
	reg &= ~(1 << 0);
	iowrite32(reg, (g_fsi_Base + FSI_CPU_IMSK));		// CPU_IMSK

	reg = ioread32(g_fsi_Base + FSI_CPU_IEMSK);
	reg &= ~(1 << 0);
	iowrite32(reg, (g_fsi_Base + FSI_CPU_IEMSK));		// CPU_IEMSK

	reg = ioread32(g_fsi_Base + FSI_CPU_INT_ST);
	reg &= ~(1 << 0);
	iowrite32(reg, (g_fsi_Base + FSI_CPU_INT_ST));		// CPU_INT_ST

	// 3msec wait
	mdelay(3);

	// Clock framework API, Status OFF
	audio_ctrl_func(SNDP_HW_FSI, STAT_OFF);

	// FSI stop
	fsi_test_status_a = TEST_NONE;
}

void fsi_test_reg_dump(void)
{
	printk(KERN_WARNING "===== FSI Registers Dump Start =====\n");

	printk(KERN_WARNING "\n<PortA>\n");
	printk(KERN_WARNING "DO_FMT     [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_DO_FMT)   ,(g_fsi_Base + FSI_DO_FMT));
	printk(KERN_WARNING "DOFF_CTL   [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_DOFF_CTL) ,(g_fsi_Base + FSI_DOFF_CTL));
	printk(KERN_WARNING "DOFF_ST    [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_DOFF_ST)  ,(g_fsi_Base + FSI_DOFF_ST));
	printk(KERN_WARNING "DI_FMT     [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_DI_FMT)   ,(g_fsi_Base + FSI_DI_FMT));
	printk(KERN_WARNING "DIFF_CTL   [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_DIFF_CTL) ,(g_fsi_Base + FSI_DIFF_CTL));
	printk(KERN_WARNING "DIFF_ST    [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_DIFF_ST)  ,(g_fsi_Base + FSI_DIFF_ST));
	printk(KERN_WARNING "ACK_MD     [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_ACK_MD)   ,(g_fsi_Base + FSI_ACK_MD));
	printk(KERN_WARNING "ACK_RV     [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_ACK_RV)   ,(g_fsi_Base + FSI_ACK_RV));
//	printk(KERN_WARNING "DIDT       [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_DIDT)     ,(g_fsi_Base + FSI_DIDT));
//	printk(KERN_WARNING "DODT       [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_DODT)     ,(g_fsi_Base + FSI_DODT));
	printk(KERN_WARNING "MUTE_ST    [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_MUTE_ST)  ,(g_fsi_Base + FSI_MUTE_ST));
	printk(KERN_WARNING "OUT_DMAC   [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_OUT_DMAC) ,(g_fsi_Base + FSI_OUT_DMAC));
	printk(KERN_WARNING "OUT_SEL    [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_OUT_SEL)  ,(g_fsi_Base + FSI_OUT_SEL));
	printk(KERN_WARNING "OUT_SPST   [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_OUT_SPST) ,(g_fsi_Base + FSI_OUT_SPST));
	printk(KERN_WARNING "IN_DMAC    [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_IN_DMAC)  ,(g_fsi_Base + FSI_IN_DMAC));
	printk(KERN_WARNING "IN_SEL     [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_IN_SEL)   ,(g_fsi_Base + FSI_IN_SEL));
	printk(KERN_WARNING "TMR_CTL    [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_TMR_CTL)    ,(g_fsi_Base + FSI_TMR_CTL));
	printk(KERN_WARNING "TMR_CLR    [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_TMR_CLR)    ,(g_fsi_Base + FSI_TMR_CLR));
	printk(KERN_WARNING "INT_SEL    [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_INT_SEL)    ,(g_fsi_Base + FSI_INT_SEL));
	printk(KERN_WARNING "INT_CLR    [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_INT_CLR)    ,(g_fsi_Base + FSI_INT_CLR));
	printk(KERN_WARNING "CPU_INT_ST [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_CPU_INT_ST) ,(g_fsi_Base + FSI_CPU_INT_ST));
	printk(KERN_WARNING "CPU_IEMSK  [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_CPU_IEMSK)  ,(g_fsi_Base + FSI_CPU_IEMSK));
	printk(KERN_WARNING "CPU_IMSK   [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_CPU_IMSK)   ,(g_fsi_Base + FSI_CPU_IMSK));
	printk(KERN_WARNING "DSP_INT_ST [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_DSP_INT_ST) ,(g_fsi_Base + FSI_DSP_INT_ST));
	printk(KERN_WARNING "DSP_IEMSK  [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_DSP_IEMSK)  ,(g_fsi_Base + FSI_DSP_IEMSK));
	printk(KERN_WARNING "DSP_IMSK   [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_DSP_IMSK)   ,(g_fsi_Base + FSI_DSP_IMSK));
	printk(KERN_WARNING "MUTE       [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_MUTE)       ,(g_fsi_Base + FSI_MUTE));
	printk(KERN_WARNING "ACK_RST    [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_ACK_RST)    ,(g_fsi_Base + FSI_ACK_RST));
	printk(KERN_WARNING "SOFT_RST   [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_SOFT_RST)   ,(g_fsi_Base + FSI_SOFT_RST));
	printk(KERN_WARNING "FIFO_SZ    [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_FIFO_SZ)    ,(g_fsi_Base + FSI_FIFO_SZ));
	printk(KERN_WARNING "CLK_SEL    [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_CLK_SEL)    ,(g_fsi_Base + FSI_CLK_SEL));
	printk(KERN_WARNING "SWAP_SEL   [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_SWAP_SEL)   ,(g_fsi_Base + FSI_SWAP_SEL));
	printk(KERN_WARNING "HPB_SRST   [%08X][%08lX]\n", ioread32(g_fsi_Base + FSI_HPB_SRST)   ,(g_fsi_Base + FSI_HPB_SRST));

	printk(KERN_WARNING "\n===== FSI Registers Dump End =====\n");
}

int fsi_voice_test_start_a(void)
{
	// Already FSI started
	if(TEST_NONE != fsi_test_status_a) {
		// Simultaneous operation is to expand later.
		return RET_NG;
	}

	// Clock framework API, Status ON
	audio_ctrl_func(SNDP_HW_FSI, STAT_ON);

	iowrite32(0x00000001, (g_fsi_Base + FSI_CLK_SEL));
	iowrite32(0x00212901, (g_fsi_Base + FSI_ACK_MD));
	iowrite32(0x00000001, (g_fsi_Base + FSI_ACK_RV));
	iowrite32(0x00000001, (g_fsi_Base + FSI_DO_FMT));
	iowrite32(0x00000001, (g_fsi_Base + FSI_DI_FMT));
	iowrite32(0x00000001, (g_fsi_Base + FSI_MUTE));

	// FSI start
	fsi_test_status_a = TEST_VOICE;

	fsi_test_reg_dump();

	return RET_NONE;
}

void fsi_voice_test_stop_a(void)
{
	// Clock framework API, Status OFF
	audio_ctrl_func(SNDP_HW_FSI, STAT_OFF);

	// FSI stop
	fsi_test_status_a = TEST_NONE;
}

#endif // SOUND_TEST

