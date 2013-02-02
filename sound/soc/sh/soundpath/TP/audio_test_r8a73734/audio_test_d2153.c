/*
 * D2153 ALSA SoC codec driver
 *
 * Copyright (c) 2012 Dialog Semiconductor
 *
 * Written by Adam Thomson <Adam.Thomson.Opensource@diasemi.com>
 * Based on DA9055 ALSA SoC codec driver.
 * 
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */



/*---------------------------------------------------------------------------*/
/* include files                                                             */
/*---------------------------------------------------------------------------*/
#include <linux/module.h>
#include <linux/err.h>
#include <linux/proc_fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/clk.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/vcd/vcd.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/pm_runtime.h>
#include <mach/common.h>
#include <mach/pm.h>
#include <mach/r8a7373.h>
#include <linux/hwspinlock.h>
#include <linux/wakelock.h>
#include <linux/miscdevice.h>

#include <sound/soundpath/TP/audio_test_extern.h>
#include <sound/soundpath/soundpath.h>
#include "audio_test_d2153.h"
#include "audio_test_reg_d2153.h"

#ifdef D2153_FSI_SOUNDPATH
#include <linux/d2153/d2153_reg.h>
#include <linux/d2153/d2153_aad.h>
#endif

/*---------------------------------------------------------------------------*/
/* typedef declaration (private)                                             */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* define macro declaration (private)                                        */
/*---------------------------------------------------------------------------*/
/*!
  @brief	Device file name.
*/
#define AUDIO_TEST_DEVICE_NAME		"audio_test"

/*!
  @brief	Detect bit.
*/
#define AUDIO_TEST_JACK_BIT_IN		(0x0001)/**< Insertion and Removal. */
#define AUDIO_TEST_JACK_BIT_BTN		(0x0004)/**< Accessory Button. */

/*!
  @brief	MAX wait time for wait queue for VCD.
*/
#define AUDIO_TEST_WATCH_CLK_TIME_OUT	(1000)

/*---------------------------------------------------------------------------*/
/* define function macro declaration (private)                               */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* enum declaration (private)                                                */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* structure declaration (private)                                           */
/*---------------------------------------------------------------------------*/
/*!
  @brief	Audio Test driver configuration structure.
*/
struct audio_test_priv {
	/***********************************/
	/* log                             */
	/***********************************/
	struct proc_dir_entry *log_entry;	/**< log level entry. */
	struct proc_dir_entry *proc_parent;	/**< proc parent. */
};

/*!
  @brief	Register setting table.
*/
struct audio_test_common_reg_table {
	u_int uiReg;				/**< Register address. */
	u_int uiValue;				/**< Register value. */
	u_int uiDelay;				/**< Register delay. */
	u_int uiClrbit;				/**< Register clear bit. */
};

/*---------------------------------------------------------------------------*/
/* prototype declaration (private)                                           */
/*---------------------------------------------------------------------------*/
/***********************************/
/* proc                            */
/***********************************/
static int audio_test_proc_set_device(u_int in_device_type,
					u_int out_device_type,
					u_int out_LR_type,
					u_int out_volume);
static int audio_test_proc_start_scuw_loopback(u_int fsi_port);
static int audio_test_proc_stop_scuw_loopback(void);
static int audio_test_proc_detect_jack(u_int *state);
static int audio_test_proc_detect_key(u_int *state);
static int audio_test_proc_start_tone(void);
static int audio_test_proc_stop_tone(void);
static int audio_test_proc_start_spuv_loopback(u_int fsi_port, u_int vqa_val,
						u_int delay_val);
static int audio_test_proc_stop_spuv_loopback(void);
/***********************************/
/* HW write                        */
/***********************************/
static int audio_test_get_logic_addr(void);
static void audio_test_rel_logic_addr(void);
static int audio_test_loopback_setup(void);
static void audio_test_loopback_remove(void);
static void audio_test_audio_ctrl_func(enum audio_test_hw_val drv, int stat);
static void audio_test_common_set_register(enum audio_test_hw_val drv,
				struct audio_test_common_reg_table *reg_tbl,
				u_int size);
/***********************************/
/* callback for VCD                */
/***********************************/
static int audio_test_call_regist_watch(void);
static void audio_test_watch_start_clk_cb(void);
static void audio_test_watch_stop_clk_cb(void);
/***********************************/
/* log level                       */
/***********************************/
static int audio_test_create_proc_entry(char *name,
					struct proc_dir_entry **proc_child);
static int audio_test_proc_log_read(char *page, char **start, off_t offset,
					int count, int *eof, void *data);
static int audio_test_proc_log_write(struct file *filp, const char *buffer,
					u_long count, void *data);
/***********************************/
/* callback                        */
/***********************************/
static int audio_test_open(struct inode *inode, struct file *filp);
static int audio_test_close(struct inode *inode, struct file *filp);
static long audio_test_ioctl(struct file *filp, u_int cmd, u_long arg);


/*---------------------------------------------------------------------------*/
/* global variable declaration                                               */
/*---------------------------------------------------------------------------*/
/***********************************/
/* log level                       */
/***********************************/
/*!
  @brief	Audio Test driver log level.
*/
#ifdef AUDIO_TEST_DBG
u_int audio_test_log_level = (AUDIO_TEST_LOG_ERR_PRINT |
				AUDIO_TEST_LOG_PROC_PRINT |
				AUDIO_TEST_LOG_FUNC_PRINT);
#else	/* !=AUDIO_TEST_DBG */
u_int audio_test_log_level = AUDIO_TEST_LOG_NO_PRINT;
#endif	/* AUDIO_TEST_DBG */
/***********************************/
/* ioctl                           */
/***********************************/
/*!
  @brief	Audio Test file operations.
*/
static const struct file_operations audio_test_fops = {
	.owner = THIS_MODULE,
	.open = audio_test_open,
	.release = audio_test_close,
	.unlocked_ioctl = audio_test_ioctl,
};

/*
  @brief	Audio Test misc device object
 */
static struct miscdevice audio_test_misc_dev = {
	.minor			= MISC_DYNAMIC_MINOR,
	.name			= AUDIO_TEST_DEVICE_NAME,
	.fops			= &audio_test_fops,
};
/***********************************/
/* internal parameter              */
/***********************************/
/*!
  @brief	Store the AudioLSI driver config.
*/
static struct audio_test_priv *audio_test_conf;
/*!
  @brief	Output device type.
*/
static u_int audio_test_drv_out_device_type = AUDIO_TEST_DRV_OUT_SPEAKER;
/***********************************/
/* HW clock flag                   */
/***********************************/
/*!
  @brief	Clock status flag.
*/
static u_int g_audio_test_clock_flag;
/***********************************/
/* HW logical address              */
/***********************************/
/*!
  @brief	CPGA register(soft reset) base address.
*/
static u_long g_audio_test_ulSrstRegBase;
/*!
  @brief	FSI base address (PortA).
*/
static u_long g_audio_test_fsi_Base;
/*!
  @brief	SCUW base address.
*/
static u_long g_audio_test_scuw_Base;
/*!
  @brief	CLKGEN base address.
*/
static u_long g_audio_test_clkgen_Base;
/***********************************/
/* Wait queue                      */
/***********************************/
/*!
  @brief	Wait queue for start voice process wake up.
*/
static DECLARE_WAIT_QUEUE_HEAD(g_watch_start_clk_queue);
static atomic_t g_audio_test_watch_start_clk;
/*!
  @brief	Wait queue for stop voice process wake up.
*/
static DECLARE_WAIT_QUEUE_HEAD(g_watch_stop_clk_queue);
static atomic_t g_audio_test_watch_stop_clk;
/***********************************/
/* PM setting                      */
/***********************************/
/*!
  @brief	Power domain.
*/
static struct device *g_audio_test_power_domain;
/*!
  @brief	Power domain count.
*/
static size_t g_audio_test_power_domain_count;
/***********************************/
/* Wake lock setting               */
/***********************************/
/*!
  @brief	Wake lock count.
*/
struct wake_lock g_audio_test_wake_lock;

//-----------------------------------------------------------------------
// Unkonw part.. No1
//-----------------------------------------------------------------------

/***********************************/
/* Table for loopback              */
/***********************************/
static struct audio_test_common_reg_table audio_test_tbl_fsi_loopback[] = {
	/* Register			Value		Delay	Clear */
	/* Clock select register (CLK_SEL) */
	/* DSPS (0) : 1 (Bus clock(MP clock)) */
	{AUDIO_TEST_FSI_CLK_SEL,	0x00000001,	0,	0},
	/* Port A Clock Set Register (A_ACK_MD) */
	/* ACKMD (14to12) : 000 (512 fs), BPFMD (10to8) : 001 (64 bit/fs),
	   DIMD (4) : 0 (Slave operation), DOMD (0) : 0 (Slave operation) */
	{AUDIO_TEST_FSI_ACK_MD,		0x00000100,	0,	0},
	/* Port A Clock Invert control register (A_ACK_RV) */
	/* LRM (12) : 0 (Clock not inverted), BRM (8) : 0 (Clock not inverted),
	   LRS (4) : 0 (Clock not inverted), BRS (0) : 1 (Clock inverted) */
	{AUDIO_TEST_FSI_ACK_RV,		0x00000001,	0,	0},
	/* Port A/B Output Serial Format Register (A_DO_FMT) */
	/* BWS (21to20) : 00 (24 bits), DTMD (9to8) : 00 (PCM format),
	   FMT (6to4) : 011 (I2S (2 channels)), NCH (2to0) : 000 (1 channel) */
	{AUDIO_TEST_FSI_DO_FMT,		0x00000030,	0,	0},
	/* Port A Input Serial Format Register (A_DI_FMT) */
	/* BWS (21to20) : 00 (24bits), FMT (6to4) : 011 (I2S (2 channels)),
	   NCH (2to0) : 000 (1 channel) */
	{AUDIO_TEST_FSI_DI_FMT,		0x00000030,	0,	0},
	/* Port A MUTE State (A_MUTE_ST) */
	/* O_CH8 (15) : 0, O_CH7 (14) : 0, O_CH6 (13) : 0, O_CH5 (12) : 1,
	   O_CH4 (11) : 0, O_CH3 (10) : 0, O_CH2 (9) : 0, O_CH1 (8) : 1,
	   I_CH8 (7) : 0, I_CH7 (6) : 0, I_CH6 (5) : 0, I_CH5 (4) : 1,
	   I_CH4 (3) : 0, I_CH3 (2) : 0, I_CH2 (1) : 0, I_CH1 (0) : 1 */
	{AUDIO_TEST_FSI_MUTE,		0x00001111,	0,	0},
};
static struct audio_test_common_reg_table
				audio_test_tbl_scuw_scuw_loopback[] = {
	/* Register			Value		Delay	Clear */
	/* Selector Control Register 0 (SELCR_SEL0) */
	/* SEL (3to0) : 0110 (FSI-IF read port 1 data (from FSI2)) */
	{AUDIO_TEST_SCUW_SEL_SELCR0,	0x00000006,	0,	0},
	/* Selector Control Register 5 (SELCR_SEL5) */
	/* SEL (1to0) : 00 (SEL0 output data, clock supply to MIX0 halted) */
	{AUDIO_TEST_SCUW_SEL_SELCR5,	0x00000000,	0,	0},
	/* Selector Control Register 6 (SELCR_SEL6) */
	/* SEL (0) : 0 (SEL5 output data, clock supply to IIR0 halted) */
	{AUDIO_TEST_SCUW_SEL_SELCR6,	0x00000000,	0,	0},
	/* Selector Control Register 7 (SELCR_SEL7) */
	/* SEL (0) : 0 (SEL6 output data, clock supply to DVU0 halted) */
	{AUDIO_TEST_SCUW_SEL_SELCR7,	0x00000000,	0,	0},
	/* Selector Control Register 15 (SELCR_SEL15) */
	/* SEL (0) : 0 (SEL7 output data) */
	{AUDIO_TEST_SCUW_SEL_SELCR15,	0x00000000,	0,	0},
	/* Module stop register 1 (MSTP1_N) */
	/* MSTP0_N (0) : 1 (FSI-IF operates.) */
	{AUDIO_TEST_SCUW_MSTP1,		0x00000001,	0,	0},
	/* FSI IF Software Reset Register (SWRSR_FSIF) */
	/* SWRST (0) : 0 (Reset the FSI IF.) */
	{AUDIO_TEST_SCUW_FSIIF_SWRSR,	0x00000000,	0,	0},
	/* FSI IF Software Reset Register (SWRSR_FSIF) */
	/* SWRST (0) : 1 (FSI IF enters the operating state.) */
	{AUDIO_TEST_SCUW_FSIIF_SWRSR,	0x00000001,	0,	0},
	/* FSI IF Initialization Register (FSIIR) */
	/* INIT (0) : 1 (Initialization) */
	{AUDIO_TEST_SCUW_FSIIF_FSIIR,	0x00000001,	0,	0},
	/* FSI IF Audio Information Register (ADINR_FSIF0_W0) */
	/* CHNUM (3to0) : 0010 (2 channel) */
	{AUDIO_TEST_SCUW_FSIIF_ADINRW0,	0x00000002,	0,	0},
	/* FSI IF Audio Information Register (ADINR_FSIF0_R1) */
	/* CHNUM (3to0) : 0010 (2 channel) */
	{AUDIO_TEST_SCUW_FSIIF_ADINRR1,	0x00000002,	0,	0},
	/* FSI Write Address Control Register for Port 0 (WADCR_FSIF_0) */
	/* WAD (7to0) : 0x09 (FSI2 port A) */
	{AUDIO_TEST_SCUW_FSIIF_WADCR0,	0x00000009,	0,	0},
	/* FSI Read Address Control Register for Port 1 (RADCR_FSIF_1) */
	/* RAD (7to0) : 0x08 (FSI2 port A) */
	{AUDIO_TEST_SCUW_FSIIF_RADCR1,	0x00000008,	0,	0},
	/* FSI IF Initialization Register (FSIIR) */
	/* INIT (0) : 0 (Processing State) */
	{AUDIO_TEST_SCUW_FSIIF_FSIIR,	0x00000000,	0,	0},
};
static struct audio_test_common_reg_table
				audio_test_tbl_clkgen_scuw_loopback[] = {
	/* Register			Value		Delay	Clear */
	/* CLKG System control register (CLKGSYSCTL) */
	/* CKSEL (3) : 0 (Supplies EXTAL1 for core clock.),
	   CKSTP2 (2) : 0 (Supplies EXTAL2.),
	   CKSTP1 (1) : 0 (Supplies EXTAL1.), CSR (0) : 0 (Clears the reset.) */
	{AUDIO_TEST_CLKG_SYSCTL,	0x00000000,	0,	0},
	/* CLKG TIM select register1 (CLKGTIMSEL1) */
	/* PWMTIM (31to28) : 0000 (Select Port A TIM),
	   FFDTIM (27to24) : 0000 (Select CPU-FIFO1 TIM),
	   RECTIM2 (19to16) : 0000 (Select Port A TIM),
	   RECTIM1 (11to8) : 0010 (Select Port A TIM),
	   RECTIM0 (3to0) : 0000 (Select Port A TIM) */
	{AUDIO_TEST_CLKG_TIMSEL1,	0x00000200,	0,	0},
	/* CLKG common register (CLKGFSIACOM) */
	/* TDIV (27to24) : 0000 (Setting TDM-Adopter repeat time),
	   MODE (21to20) : 10 (Non-continuos clock mode),
	   FORM (17to16) : 01 (2ch(LR) format),
	   FS (14to12) : 010 (64fs), RATE (11to8) : 1000 (44.1kHz),
	   INV (5) : 0 (non-invert BCLK),
	   CLKGM (0) : 1 (Select CLKGEN master) */
	{AUDIO_TEST_CLKG_FSIACOM,	0x00212801,	0,	0},
	/* CLKG PULSE control register (CLKGPULSECTL) */
	/* VINTREV (9) : 0 (normal), VINTSEL (8) : 0 (HW VINT (from PAD).),
	   SLIMEN (7) : 0 (Disable.), FFDEN (6) : 0 (Disable.),
	   AUEN (5) : 0 (Disable.), SVEN (4) : 0 (Disable.),
	   CF1EN (3) : 0 (Disable.), CF0EN (2) : 0 (Disable.),
	   PBEN (1) : 0 (Disable.), PAEN (0) : 1 (Enable.) */
	{AUDIO_TEST_CLKG_PULSECTL,	0x00000001,	0,	0},
};
static struct audio_test_common_reg_table
				audio_test_tbl_scuw_spuv_loopback[] = {
	/* Register			Value		Delay	Clear */
	/* Selector Control Register 21 (SELCR_SEL21) */
	/* SEL (0) : 1 (SPU2V output data) */
	{AUDIO_TEST_SCUW_SEL_SELCR21,	0x00000001,	0,	0},
	/* Selector Control Register 15 (SELCR_SEL15) */
	/* SEL (0) : 1 (Voice data (from VOIP)) */
	{AUDIO_TEST_SCUW_SEL_SELCR15,	0x00000001,	0,	0},
	/* Selector Control Register 12 (SELCR_SEL12) */
	/* SEL (2to0) : 011 (FSI-IF read port 1 data (from FSI2)) */
	{AUDIO_TEST_SCUW_SEL_SELCR12,	0x00000003,	0,	0},
	/* Module stop register 1 (MSTP1_N) */
	/* MSTP0_N (0) : 1 (FSI-IF operates.) */
	{AUDIO_TEST_SCUW_MSTP1,		0x00000001,	0,	0},
	/* FSI IF Software Reset Register (SWRSR_FSIF) */
	/* SWRST (0) : 0 (Reset the FSI IF.) */
	{AUDIO_TEST_SCUW_FSIIF_SWRSR,	0x00000000,	0,	0},
	/* FSI IF Software Reset Register (SWRSR_FSIF) */
	/* SWRST (0) : 1 (FSI IF enters the operating state.) */
	{AUDIO_TEST_SCUW_FSIIF_SWRSR,	0x00000001,	0,	0},
	/* FSI IF Initialization Register (FSIIR) */
	/* INIT (0) : 1 (Initialization) */
	{AUDIO_TEST_SCUW_FSIIF_FSIIR,	0x00000001,	0,	0},
	/* FSI IF Audio Information Register (ADINR_FSIF0_W0) */
	/* CHNUM (3to0) : 0010 (2 channel) */
	{AUDIO_TEST_SCUW_FSIIF_ADINRW0,	0x00000002,	0,	0},
	/* FSI IF Audio Information Register (ADINR_FSIF0_R1) */
	/* CHNUM (3to0) : 0010 (2 channel) */
	{AUDIO_TEST_SCUW_FSIIF_ADINRR1,	0x00000002,	0,	0},
	/* FSI Write Address Control Register for Port 0 (WADCR_FSIF_0) */
	/* WAD (7to0) : 0x09 (FSI2 port A) */
	{AUDIO_TEST_SCUW_FSIIF_WADCR0,	0x00000009,	0,	0},
	/* FSI Read Address Control Register for Port 1 (RADCR_FSIF_1) */
	/* RAD (7to0) : 0x08 (FSI2 port A) */
	{AUDIO_TEST_SCUW_FSIIF_RADCR1,	0x00000008,	0,	0},
	/* FSI IF Initialization Register (FSIIR) */
	/* INIT (0) : 0 (Processing State) */
	{AUDIO_TEST_SCUW_FSIIF_FSIIR,	0x00000000,	0,	0},
	/* Voice Data Setting Register (VDSET) */
	/* VDEXPD (1) : 1 (Channel 1 to 7 are copied Channel 0) */
	{AUDIO_TEST_SCUW_VD_VDSET,	0x00000002,	0,	0},
};
static struct audio_test_common_reg_table
				audio_test_tbl_clkgen_spuv_loopback[] = {
	/* Register			Value		Delay	Clear */
	/* CLKG System control register (CLKGSYSCTL) */
	/* CKSEL (3) : 0 (Supplies EXTAL1 for core clock.),
	   CKSTP2 (2) : 0 (Supplies EXTAL2.),
	   CKSTP1 (1) : 0 (Supplies EXTAL1.), CSR (0) : 0 (Clears the reset.) */
	{AUDIO_TEST_CLKG_SYSCTL,	0x00000000,	0,	0},
	/* CLKG common register (CLKGSPUVCOM) */
	/* TDIV (27to24) : 0000 (Setting TDM-Adopter repeat time),
	   MODE (21to20) : 10 (Non-continuos clock mode),
	   FORM (17to16) : 01 (2ch(LR) format),
	   FS (14to12) : 011 (128fs), RATE (11to8) : 0100 (16kHz),
	   INV (5) : 0 (non-invert BCLK),
	   CLKGM (0) : 1 (Select CLKGEN master) */
	{AUDIO_TEST_CLKG_SPUVCOM,	0x00213401,	0,	0},
	/* CLKG TIM select register0 (CLKGTIMSEL0) */
	/* CF1TIM (27to24) : 0000 (Select CPU-FIFO1 TIM),
	   CF0TIM (19to16) : 0000 (Select CPU-FIFO0 TIM),
	   AURTIM (11to8) : 0000 (Select AURAM TIM),
	   VOTIM (3to0) : 0010 (Select Port A TIM) */
	{AUDIO_TEST_CLKG_TIMSEL0,	0x00000002,	0,	0},
	/* CLKG TIM select register1 (CLKGTIMSEL1) */
	/* PWMTIM (31to28) : 0000 (Select Port A TIM),
	   FFDTIM (27to24) : 0000 (Select CPU-FIFO1 TIM),
	   RECTIM2 (19to16) : 0000 (Select Port A TIM),
	   RECTIM1 (11to8) : 0010 (Select Port A TIM),
	   RECTIM0 (3to0) : 0000 (Select Port A TIM) */
	{AUDIO_TEST_CLKG_TIMSEL1,	0x00000200,	0,	0},
	/* CLKG common register (CLKGFSIACOM) */
	/* TDIV (27to24) : 0000 (Setting TDM-Adopter repeat time),
	   MODE (21to20) : 10 (Non-continuos clock mode),
	   FORM (17to16) : 01 (2ch(LR) format),
	   FS (14to12) : 011 (128fs), RATE (11to8) : 0100 (16kHz),
	   INV (5) : 0 (non-invert BCLK),
	   CLKGM (0) : 1 (Select CLKGEN master) */
	{AUDIO_TEST_CLKG_FSIACOM,	0x00213401,	0,	0},
	/* CLKG PULSE control register (CLKGPULSECTL) */
	/* VINTREV (9) : 0 (normal), VINTSEL (8) : 0 (HW VINT (from PAD).),
	   SLIMEN (7) : 0 (Disable.), FFDEN (6) : 0 (Disable.),
	   AUEN (5) : 0 (Disable.), SVEN (4) : 1 (Enable.),
	   CF1EN (3) : 0 (Disable.), CF0EN (2) : 0 (Disable.),
	   PBEN (1) : 0 (Disable.), PAEN (0) : 1 (Enable.) */
	{AUDIO_TEST_CLKG_PULSECTL,	0x00000011,	0,	0},
};

/*---------------------------------------------------------------------------*/
/* extern variable declaration                                               */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* extern function declaration                                               */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* inline function implementation                                            */
/*---------------------------------------------------------------------------*/
/* none */



/*---------------------------------------------------------------------------*/
/* function implementation                                                   */
/*---------------------------------------------------------------------------*/
/*------------------------------------*/
/* for private function               */
/*------------------------------------*/
/*!
  @brief	Process of setting device.

  @param	in_device_type [i] Input device.
  @param	out_device_type [i] Output device.
  @param	out_LR_type [i] LR info.
  @param	out_volume [i] Volume info.

  @return	Function results.

  @note		.
*/
static int audio_test_proc_set_device(u_int in_device_type,
					u_int out_device_type,
					u_int out_LR_type,
					u_int out_volume)
{
	int ret = 0;
	u_long new_device = 0;
	u_long old_device = 0;
	u_short oe = 0;
	u_short reg = 0;

	audio_test_log_efunc("in_dev[%d] out_dev[%d] out_LR[%d] out_vol[%d]",
		in_device_type, out_device_type, out_LR_type, out_volume);

	/***********************************/
	/* Get device bit                  */
	/***********************************/
	ret = audio_test_ic_get_device(&old_device);
	if (0 != ret) {
		audio_test_log_err("audio_test_ic_get_device");
		goto error;
	}

	audio_test_log_info("old device[%#010lx]", old_device);

	/***********************************/
	/* Set input device bit            */
	/***********************************/
	audio_test_cnv_input_device(in_device_type, &new_device);

	/***********************************/
	/* Set output device bit           */
	/***********************************/
	audio_test_cnv_output_device(out_device_type, &new_device);

	/***********************************/
	/* Set device                      */
	/***********************************/
	ret = audio_test_ic_clear_device();
	if (0 != ret) {
		audio_test_log_err("audio_test_ic_clear_device");
		goto error;
	}
	ret = audio_test_ic_set_device(new_device);
	if (0 != ret) {
		audio_test_log_err("audio_test_ic_set_device");
		goto error;
	}

	audio_test_log_info("new device[%#010lx]", new_device);

	/* AIF disable */
	ret = audio_test_ic_read(D2153_AIF_CTRL, &reg);
	if (0 != ret) {
		audio_test_log_err("audio_test_ic_read");
		goto error;
	}	

	audio_test_log_info("AIF Read value [0x%x]", reg);
	audio_test_log_info("AIF Conv. value [0x%x]", reg & (~D2153_AIF_EN));

	ret = audio_test_ic_write(D2153_AIF_CTRL, reg & (~D2153_AIF_EN));
	if (0 != ret) {
		audio_test_log_err("audio_test_ic_write");
		goto error;
	}

	if (0 != ret) {
		audio_test_log_err("audio_test_ic_write");
		goto error;
	}

	/***********************************/
	/* Set LR                          */
	/***********************************/
	audio_test_log_info("old output enable[%#010x]", oe);
	audio_test_log_info("out_device_type[%d], ", out_device_type);
	audio_test_log_info("out_LR_type[%d]", out_LR_type);

	/* AIF enable */
	ret = audio_test_ic_read(D2153_AIF_CTRL, &reg);
	if (0 != ret) {
		audio_test_log_err("audio_test_ic_read");
		goto error;
	}

	audio_test_log_info("AIF Read value [0x%x]", reg);
	audio_test_log_info("AIF Conv. value [0x%x]", reg | (D2153_AIF_EN));

	ret = audio_test_ic_write(D2153_AIF_CTRL, reg | D2153_AIF_EN);
	if (0 != ret) {
		audio_test_log_err("audio_test_ic_write");
		goto error;
	}

	audio_test_drv_out_device_type = out_device_type;

	audio_test_log_rfunc("ret[%d]", ret);
	return ret;

error:
	audio_test_log_err("ret[%d]", ret);
	return ret;
}

/*!
  @brief	Norify loopback to sound driver.

  @param	dev_chg [i] Reason of device change.

  @return	Function results.

  @note		.
*/
static int audio_test_notify_loopback(u_int dev_chg)
{
	int ret = 0;
	u_int dev = 0;

	audio_test_log_efunc("dev_chg[%d]", dev_chg);

	switch (audio_test_drv_out_device_type) {
	case AUDIO_TEST_DRV_OUT_SPEAKER:
		dev = SNDP_OUT_SPEAKER;
		break;
	case AUDIO_TEST_DRV_OUT_HEADPHONE:
		dev = SNDP_OUT_WIRED_HEADPHONE;
		break;
	case AUDIO_TEST_DRV_OUT_EARPIECE:
		dev = SNDP_OUT_EARPIECE;
		break;
	default:
		audio_test_log_info("unknown output device");
		break;
	}

	ret = sndp_pt_loopback(SNDP_MODE_INCALL, dev, dev_chg);
	if (-ENODEV == ret)
		ret = 0;
	else if (0 != ret) {
		audio_test_log_err("sndp_pt_loopback");
		goto error;
	}

error:
	audio_test_log_rfunc("ret[%d]", ret);
	return ret;
}

/*!
  @brief	Process of starting SCUW loopback.
			[AudioIC->FSI->SCUW->FSI->AudioIC].

  @param	fsi_port [i] FSI port.

  @return	Function results.

  @note		.
*/
static int audio_test_proc_start_scuw_loopback(u_int fsi_port)
{
	int ret = 0;

	audio_test_log_efunc("fsi[%d]", fsi_port);

	/* Add not to be suspend in loopback */
	wake_lock(&g_audio_test_wake_lock);

	/* Notify to Sound driver */
	/* for KeyTone Mix and Audience */
	ret = audio_test_notify_loopback(SNDP_A2220_START);
	if (0 != ret) {
		audio_test_log_err("audio_test_notify_loopback");
		goto error;
	}

	/***********************************/
	/* Setup                           */
	/***********************************/
	ret = audio_test_loopback_setup();
	if (0 != ret) {
		audio_test_log_err("audio_test_loopback_setup");
		goto error;
	}

	/***********************************/
	/* Set SCUW register               */
	/***********************************/
	audio_test_common_set_register(AUDIO_TEST_HW_SCUW,
				audio_test_tbl_scuw_scuw_loopback,
				ARRAY_SIZE(audio_test_tbl_scuw_scuw_loopback));

	/***********************************/
	/* Set FSI register                */
	/***********************************/
	audio_test_common_set_register(AUDIO_TEST_HW_FSI,
				audio_test_tbl_fsi_loopback,
				ARRAY_SIZE(audio_test_tbl_fsi_loopback));

	/***********************************/
	/* Set CLKGEN register             */
	/***********************************/
	audio_test_common_set_register(AUDIO_TEST_HW_CLKGEN,
			audio_test_tbl_clkgen_scuw_loopback,
			ARRAY_SIZE(audio_test_tbl_clkgen_scuw_loopback));

	/***********************************/
	/* Clock reset                     */
	/***********************************/
	sh_modify_register32(
		(g_audio_test_fsi_Base + AUDIO_TEST_FSI_ACK_RST),
		0, 0x00000001);

error:
	/* Add not to be suspend in loopback */
	wake_unlock(&g_audio_test_wake_lock);

	audio_test_log_rfunc("ret[%d]", ret);
	return ret;
}

/*!
  @brief	Process of stopping SCUW loopback.
			[AudioIC->FSI->SCUW->FSI->AudioIC].

  @param	.

  @return	Function results.

  @note		.
*/
static int audio_test_proc_stop_scuw_loopback(void)
{
	int ret = 0;

	audio_test_log_efunc("");

	/***********************************/
	/* Remove                          */
	/***********************************/
	audio_test_loopback_remove();

	/***********************************/
	/* Set device                      */
	/***********************************/
	ret = audio_test_ic_clear_device();
	if (0 != ret) {
		audio_test_log_err("audio_test_ic_clear_device");
		goto error;
	}

	/* Notify to Sound driver */
	/* for KeyTone Mix and Audience */
	ret = audio_test_notify_loopback(SNDP_A2220_STOP);
	if (0 != ret) {
		audio_test_log_err("audio_test_notify_loopback");
		goto error;
	}

	/* Add not to be suspend in loopback */
	wake_unlock(&g_audio_test_wake_lock);

	audio_test_log_rfunc("ret[%d]", ret);
	return ret;

error:
	audio_test_log_err("ret[%d]", ret);
	return ret;
}

/*!
  @brief	Process of detecting jack insert.

  @param	state [o] state of jack insert.

  @return	Function results.

  @note		.
*/
static int audio_test_proc_detect_jack(u_int *state)
{
	int ret = 0;
	u_short state_jack = 0;

	audio_test_log_efunc("");

	/* No info for 0x0080 of 'register 0x0039' */
	/* MIC Detect Enable. */
	/* Microphone Detect */
	audio_test_ic_aad_write(D2153_ACCDET_UNLOCK_AO, 0x4a);

	audio_test_ic_aad_write(D2153_ACCDET_TST2, 0x10);
	audio_test_ic_aad_write(D2153_ACCDET_THRESH1, 0x0f);
	audio_test_ic_aad_write(D2153_ACCDET_THRESH2, 0x56);
	audio_test_ic_aad_write(D2153_ACCDET_THRESH3, 0x0e);
	audio_test_ic_aad_write(D2153_ACCDET_THRESH4, 0x44);

	audio_test_ic_write(D2153_REFERENCES, 0x88);

	audio_test_ic_aad_write(D2153_ACCDET_CFG1, 0x5f);
	audio_test_ic_aad_write(D2153_ACCDET_CFG2, 0x00);
	audio_test_ic_aad_write(D2153_ACCDET_CFG3, 0x03);
	audio_test_ic_aad_write(D2153_ACCDET_CFG4, 0x07);

	audio_test_ic_aad_write(D2153_ACCDET_CONFIG, 0x88);

	audio_test_ic_write(D2153_UNLOCK, 0x8b);

	audio_test_ic_write(D2153_MICBIAS1_CTRL, 0x01);

	/***********************************/
	/* Get state of jack               */
	/***********************************/
	ret = audio_test_ic_aad_read(D2153_ACCDET_CFG3, &state_jack);
	if (0 > ret) {
		audio_test_log_err("audio_test_ic_read");
		goto error;
	}

	if (state_jack & D2153_ACCDET_JACK_MODE_JACK) {
		*state = AUDIO_TEST_DRV_STATE_ON;
	} else {
		*state = AUDIO_TEST_DRV_STATE_OFF;
	}

	audio_test_log_rfunc("ret[%d] state[%d]", ret, *state);
	return ret;

error:
	audio_test_log_err("ret[%d]", ret);
	return ret;
}

/*!
  @brief	Process of detecting hookkey press.

  @param	state [o] state of hookkey press.

  @return	Function results.

  @note		.
*/
static int audio_test_proc_detect_key(u_int *state)
{
	int ret = 0;
	u_short state_jack = 0;

	audio_test_log_efunc("");

	/***********************************/
	/* Get state of key               */
	/***********************************/
	/* Read accessory detect status (press/release) from PMIC */
	ret = audio_test_ic_pmic_read(D2153_STATUS_C_REG, &state_jack);
	if (state_jack & D2153_ACC_DET_STATUS_MASK) {
		/* If last event was release, then this must be a press */
		/* if (!d2153_aad->button.status) { */
		ret |= audio_test_ic_aad_read(D2153_ACCDET_STATUS, &state_jack);
		if (state_jack == 0) {
			/* 
			* Low resolution read - need to wait for AAD block to
			* perfom high resolution measurements so we know which
			* button was pressed.
			*/
			msleep(8);
			ret = audio_test_ic_aad_read(D2153_ACCDET_STATUS,
								&state_jack);
		}

		if (0 != ret) {
			audio_test_log_err("audio_test_ic_read");
			goto error;
		}

		if ((0 <= state_jack && state_jack <= 1) ||
			(2 <= state_jack && state_jack <= 3) ||
			(4 <= state_jack && state_jack <= 5)) {
			/* known button */
			*state = AUDIO_TEST_DRV_STATE_ON;
		} else {
			/* unknown button */
			*state = AUDIO_TEST_DRV_STATE_OFF;
		}
	} else {
		*state = AUDIO_TEST_DRV_STATE_OFF;
	}

	audio_test_log_rfunc("ret[%d] state[%d]", ret, *state);
	return ret;

error:
	audio_test_log_err("ret[%d]", ret);
	return ret;
}

/*!
  @brief	Process of starting tone.

  @param	.

  @return	Function results.

  @note		.
*/
static int audio_test_proc_start_tone(void)
{
	int ret = 0;

	struct vcd_execute_command cmd;
	struct vcd_call_option option;

	audio_test_log_efunc("");

	memset(&cmd, 0, sizeof(cmd));
	memset(&option, 0, sizeof(option));

	/* Add not to be suspend in loopback */
	wake_lock(&g_audio_test_wake_lock);

	/***********************************/
	/* Start TestTone mode             */
	/***********************************/
	cmd.command = VCD_COMMAND_SET_CALL_MODE;
	option.call_kind = VCD_CALL_KIND_1KHZ;
	cmd.arg = &option;
	ret = vcd_execute_test_call(&cmd);
	if (0 != ret) {
		audio_test_log_err("vcd_execute_test_call");
		goto error;
	}

	/* Notify to Sound driver */
	/* for KeyTone Mix and Audience */
	ret = audio_test_notify_loopback(SNDP_A2220_START);
	if (0 != ret) {
		audio_test_log_err("audio_test_notify_loopback");
		goto error;
	}

	/***********************************/
	/* Setup                           */
	/***********************************/
	ret = audio_test_loopback_setup();
	if (0 != ret) {
		audio_test_log_err("audio_test_loopback_setup");
		goto error;
	}

	/***********************************/
	/* Set SCUW register               */
	/***********************************/
	audio_test_common_set_register(AUDIO_TEST_HW_SCUW,
				audio_test_tbl_scuw_spuv_loopback,
				ARRAY_SIZE(audio_test_tbl_scuw_spuv_loopback));

	/***********************************/
	/* Set FSI register                */
	/***********************************/
	audio_test_common_set_register(AUDIO_TEST_HW_FSI,
				audio_test_tbl_fsi_loopback,
				ARRAY_SIZE(audio_test_tbl_fsi_loopback));

	/***********************************/
	/* Set CLKGEN register             */
	/***********************************/
	audio_test_common_set_register(AUDIO_TEST_HW_CLKGEN,
			audio_test_tbl_clkgen_spuv_loopback,
			ARRAY_SIZE(audio_test_tbl_clkgen_spuv_loopback));

	/***********************************/
	/* Clock reset                     */
	/***********************************/
	sh_modify_register32(
		(g_audio_test_fsi_Base + AUDIO_TEST_FSI_ACK_RST),
		0, 0x00000001);

	audio_test_log_rfunc("ret[%d]", ret);
	return ret;

error:
	/* Add not to be suspend in loopback */
	wake_unlock(&g_audio_test_wake_lock);

	audio_test_log_err("ret[%d]", ret);
	return ret;
}

/*!
  @brief	Process of stopping tone.

  @param	.

  @return	Function results.

  @note		.
*/
static int audio_test_proc_stop_tone(void)
{
	int ret = 0;
	struct vcd_execute_command cmd;
	struct vcd_call_option option;

	audio_test_log_efunc("");

	memset(&cmd, 0, sizeof(cmd));
	memset(&option, 0, sizeof(option));

	/***********************************/
	/* Remove                          */
	/***********************************/
	audio_test_loopback_remove();

	/***********************************/
	/* Set device                      */
	/***********************************/
	ret = audio_test_ic_clear_device();
	if (0 != ret) {
		audio_test_log_err("audio_test_ic_clear_device");
		goto error;
	}

	/* Notify to Sound driver */
	/* for KeyTone Mix and Audience */
	ret = audio_test_notify_loopback(SNDP_A2220_STOP);
	if (0 != ret) {
		audio_test_log_err("audio_test_notify_loopback");
		goto error;
	}

	/***********************************/
	/* Stop TestTone mode              */
	/***********************************/
	cmd.command = VCD_COMMAND_SET_CALL_MODE;
	option.call_kind = VCD_CALL_KIND_CALL;
	cmd.arg = &option;
	ret = vcd_execute_test_call(&cmd);
	if (0 != ret) {
		audio_test_log_err("vcd_execute_test_call");
		goto error;
	}

	/* Add not to be suspend in loopback */
	wake_unlock(&g_audio_test_wake_lock);

	audio_test_log_rfunc("ret[%d]", ret);
	return ret;

error:
	audio_test_log_err("ret[%d]", ret);
	return ret;
}

/*!
  @brief	Process of starting SPUV loopback.
			[AudioIC->FSI->SCUW->SPUV->SCUW->FSI->AudioIC].

  @param	fsi_port [i] FSI port.
  @param	vqa_val [i] VQA valid.
  @param	delay_val [i] Delay valid.

  @return	Function results.

  @note		.
*/
static int audio_test_proc_start_spuv_loopback(u_int fsi_port, u_int vqa_val,
						u_int delay_val)
{
	int ret = 0;

	struct vcd_execute_command cmd;
	struct vcd_call_option option;

	audio_test_log_efunc("");

	memset(&cmd, 0, sizeof(cmd));
	memset(&option, 0, sizeof(option));

	/* Add not to be suspend in loopback */
	wake_lock(&g_audio_test_wake_lock);

	/***********************************/
	/* Start SPUV loopback mode        */
	/***********************************/
	cmd.command = VCD_COMMAND_SET_CALL_MODE;
	option.call_kind = VCD_CALL_KIND_PCM_LB;
	if (AUDIO_TEST_DRV_STATE_OFF == vqa_val &&
		AUDIO_TEST_DRV_STATE_OFF == delay_val) {
		option.loopback_mode = VCD_LOOPBACK_MODE_INTERFACE;
	} else if (AUDIO_TEST_DRV_STATE_ON == vqa_val &&
		AUDIO_TEST_DRV_STATE_OFF == delay_val) {
		option.loopback_mode = VCD_LOOPBACK_MODE_PCM;
	} else {
		option.loopback_mode = VCD_LOOPBACK_MODE_DELAY;
	}
	cmd.arg = &option;
	ret = vcd_execute_test_call(&cmd);
	if (0 != ret) {
		audio_test_log_err("vcd_execute_test_call");
		goto error;
	}

	/* Notify to Sound driver */
	/* for KeyTone Mix and Audience */
	ret = audio_test_notify_loopback(SNDP_A2220_START);
	if (0 != ret) {
		audio_test_log_err("audio_test_notify_loopback");
		goto error;
	}

	/***********************************/
	/* Setup                           */
	/***********************************/
	ret = audio_test_loopback_setup();
	if (0 != ret) {
		audio_test_log_err("audio_test_loopback_setup");
		goto error;
	}

	/***********************************/
	/* Set SCUW register               */
	/***********************************/
	audio_test_common_set_register(AUDIO_TEST_HW_SCUW,
				audio_test_tbl_scuw_spuv_loopback,
				ARRAY_SIZE(audio_test_tbl_scuw_spuv_loopback));

	/***********************************/
	/* Set FSI register                */
	/***********************************/
	audio_test_common_set_register(AUDIO_TEST_HW_FSI,
				audio_test_tbl_fsi_loopback,
				ARRAY_SIZE(audio_test_tbl_fsi_loopback));

	/***********************************/
	/* Set CLKGEN register             */
	/***********************************/
	audio_test_common_set_register(AUDIO_TEST_HW_CLKGEN,
			audio_test_tbl_clkgen_spuv_loopback,
			ARRAY_SIZE(audio_test_tbl_clkgen_spuv_loopback));

	/***********************************/
	/* Clock reset                     */
	/***********************************/
	sh_modify_register32(
		(g_audio_test_fsi_Base + AUDIO_TEST_FSI_ACK_RST),
		0, 0x00000001);

	audio_test_log_rfunc("ret[%d]", ret);
	return ret;

error:
	/* Add not to be suspend in loopback */
	wake_unlock(&g_audio_test_wake_lock);

	audio_test_log_err("ret[%d]", ret);
	return ret;
}

/*!
  @brief	Process of stopping SPUV loopback.
			[AudioIC->FSI->SCUW->SPUV->SCUW->FSI->AudioIC].

  @param	.

  @return	Function results.

  @note		.
*/
static int audio_test_proc_stop_spuv_loopback(void)
{
	int ret = 0;
	struct vcd_execute_command cmd;
	struct vcd_call_option option;

	audio_test_log_efunc("");

	memset(&cmd, 0, sizeof(cmd));
	memset(&option, 0, sizeof(option));

	/***********************************/
	/* Remove                          */
	/***********************************/
	audio_test_loopback_remove();

	/***********************************/
	/* Set device                      */
	/***********************************/
	ret = audio_test_ic_clear_device();
	if (0 != ret) {
		audio_test_log_err("audio_test_ic_clear_device");
		goto error;
	}

	/* Notify to Sound driver */
	/* for KeyTone Mix and Audience */
	ret = audio_test_notify_loopback(SNDP_A2220_STOP);
	if (0 != ret) {
		audio_test_log_err("audio_test_notify_loopback");
		goto error;
	}

	/***********************************/
	/* Stop SPUV loopback mode         */
	/***********************************/
	cmd.command = VCD_COMMAND_SET_CALL_MODE;
	option.call_kind = VCD_CALL_KIND_CALL;
	cmd.arg = &option;
	ret = vcd_execute_test_call(&cmd);
	if (0 != ret) {
		audio_test_log_err("vcd_execute_test_call");
		goto error;
	}

	/* Add not to be suspend in loopback */
	wake_unlock(&g_audio_test_wake_lock);

	audio_test_log_rfunc("ret[%d]", ret);
	return ret;

error:
	audio_test_log_err("ret[%d]", ret);
	return ret;
}

/*!
  @brief	Get logical address.

  @param	.

  @return	Function results.

  @note		.
*/
static int audio_test_get_logic_addr(void)
{
	int ret = 0;

	audio_test_log_efunc("");

	/****************************************/
	/* Get CPGA(soft reset) Logical Address */
	/****************************************/
	g_audio_test_ulSrstRegBase =
		(u_long)ioremap_nocache(AUDIO_TEST_CPG_PHY_BASE_SRST,
					AUDIO_TEST_CPG_REG_MAX_SRST);
	if (0 >= g_audio_test_ulSrstRegBase) {
		audio_test_log_err("Software Reset register ioremap failed");
		ret = -ENOMEM;
		goto error;
	}

	/***********************************/
	/* Get FSI Logical Address         */
	/***********************************/
	g_audio_test_fsi_Base =
		(u_long)ioremap_nocache(AUDIO_TEST_FSI_PHY_BASE,
					AUDIO_TEST_FSI_MAP_LEN);
	if (0 >= g_audio_test_fsi_Base) {
		audio_test_log_err("fsi ioremap failed");
		ret = -ENOMEM;
		goto error;
	}

	/***********************************/
	/* Get SCUW Logical Address        */
	/***********************************/
	g_audio_test_scuw_Base =
		(u_long)ioremap_nocache(AUDIO_TEST_SCUW_PHY_BASE,
					AUDIO_TEST_SUCW_MAP_LEN);
	if (0 >= g_audio_test_scuw_Base) {
		audio_test_log_err("scuw ioremap failed");
		ret = -ENOMEM;
		goto error;
	}

	/***********************************/
	/* Get CLKGEN Logical Address      */
	/***********************************/
	g_audio_test_clkgen_Base =
		(u_long)ioremap_nocache(AUDIO_TEST_CLKGEN_PHY_BASE,
					AUDIO_TEST_CLKGEN_MAP_LEN);
	if (0 >= g_audio_test_clkgen_Base) {
		audio_test_log_err("clkgen ioremap failed");
		ret = -ENOMEM;
		goto error;
	}

	audio_test_log_rfunc("ret[%d]", ret);
	return ret;
error:
	/***********************************/
	/* Release logical address         */
	/***********************************/
	audio_test_rel_logic_addr();

	audio_test_log_err("ret[%d]", ret);
	return ret;
}

/*!
  @brief	Release logical address.

  @param	.

  @return	.

  @note		.
*/
static void audio_test_rel_logic_addr(void)
{
	audio_test_log_efunc("");

	/***********************************/
	/* Release FSI Logical Address     */
	/***********************************/
	if (0 < g_audio_test_fsi_Base) {
		iounmap((void *)g_audio_test_fsi_Base);
		g_audio_test_fsi_Base = 0;
	}

	/***********************************/
	/* Release SCUW Logical Address    */
	/***********************************/
	if (0 < g_audio_test_scuw_Base) {
		iounmap((void *)g_audio_test_scuw_Base);
		g_audio_test_scuw_Base = 0;
	}

	/***********************************/
	/* Release CLKGEN Logical Address  */
	/***********************************/
	if (g_audio_test_clkgen_Base) {
		iounmap((void *)g_audio_test_clkgen_Base);
		g_audio_test_clkgen_Base = 0;
	}

	/*********************************************/
	/* Release CPGA(soft reset) Logical Address  */
	/*********************************************/
	if (0 < g_audio_test_ulSrstRegBase) {
		iounmap((void *)g_audio_test_ulSrstRegBase);
		g_audio_test_ulSrstRegBase = 0;
	}

	audio_test_log_rfunc("");
}

/*!
  @brief	Setup loopback setting.

  @param	.

  @return	Function results.

  @note		.
*/
static int audio_test_loopback_setup(void)
{
	int ret = 0;
	int res = 0;
	int reg = 0;

	audio_test_log_efunc("");

	/* Enable the power domain */
	res = pm_runtime_get_sync(g_audio_test_power_domain);
	if (!(0 == res || 1 == res)) {  /* 0:success 1:active */
		audio_test_log_err("pm_runtime_get_sync res[%d]\n", res);
		ret = -1;
		goto error;
	}

	/***********************************/
	/* Enable SCUW clock               */
	/***********************************/
	audio_test_audio_ctrl_func(AUDIO_TEST_HW_SCUW,
				AUDIO_TEST_DRV_STATE_ON);

	/***********************************/
	/* Enable FSI clock                */
	/***********************************/
	audio_test_audio_ctrl_func(AUDIO_TEST_HW_FSI,
				AUDIO_TEST_DRV_STATE_ON);

	/***********************************/
	/* Wait VCD                        */
	/***********************************/
	wait_event_interruptible_timeout(
		g_watch_start_clk_queue,
		atomic_read(&g_audio_test_watch_start_clk),
		msecs_to_jiffies(AUDIO_TEST_WATCH_CLK_TIME_OUT));
	atomic_set(&g_audio_test_watch_start_clk, 0);

	/***********************************/
	/* Enable CLKGEN clock             */
	/***********************************/
	audio_test_audio_ctrl_func(AUDIO_TEST_HW_CLKGEN,
				AUDIO_TEST_DRV_STATE_ON);

	/***********************************/
	/* Set GPIO                        */
	/***********************************/
	reg = ioread16(AUDIO_TEST_FSI2CR);
	audio_test_log_info("FSI2CR[%#06x]", reg);
	if (reg & (1 << 8)) {
		sh_modify_register16(AUDIO_TEST_FSI2CR, 0x0300, 0);
		reg = ioread16(AUDIO_TEST_FSI2CR);
		audio_test_log_info("FSI2CR[%#06x]", reg);
	}

error:
	audio_test_log_rfunc("ret[%d]", ret);
	return ret;
}

/*!
  @brief	Remove loopback setting.

  @param	.

  @return	.

  @note		.
*/
static void audio_test_loopback_remove(void)
{
	int res = 0;

	audio_test_log_efunc("");

	/***********************************/
	/* Wait VCD                        */
	/***********************************/
	wait_event_interruptible_timeout(
		g_watch_stop_clk_queue,
		atomic_read(&g_audio_test_watch_stop_clk),
		msecs_to_jiffies(AUDIO_TEST_WATCH_CLK_TIME_OUT));
	atomic_set(&g_audio_test_watch_stop_clk, 0);

	/***********************************/
	/* Stop SCUW                       */
	/***********************************/
	audio_test_audio_ctrl_func(AUDIO_TEST_HW_SCUW,
				AUDIO_TEST_DRV_STATE_OFF);

	/***********************************/
	/* Stop FSI                        */
	/***********************************/
	audio_test_audio_ctrl_func(AUDIO_TEST_HW_FSI,
				AUDIO_TEST_DRV_STATE_OFF);

	/***********************************/
	/* Stop CLKGEN                     */
	/***********************************/
	audio_test_audio_ctrl_func(AUDIO_TEST_HW_CLKGEN,
				AUDIO_TEST_DRV_STATE_OFF);

	/* Disable the power domain */
	res = pm_runtime_put_sync(g_audio_test_power_domain);
	if (0 != res)
		audio_test_log_err("pm_runtime_put_sync res[%d]\n", res);

	audio_test_log_rfunc("");
}

/*!
  @brief	Power management for CLKGEN/FSI/SCUW.

  @param	drv [i] H/W type.
  @param	stat [i] On/Off.

  @return	.

  @note		.
*/
static void audio_test_audio_ctrl_func(enum audio_test_hw_val drv, int stat)
{
	struct clk *clk;
	unsigned long flags = 0;
	int res = 0;

	audio_test_log_efunc("hw[%d] stat[%d]", drv, stat);

	audio_test_log_info("clock_flag[%#08x]", g_audio_test_clock_flag);

	switch (drv) {
	/***********************************/
	/* CLKGEN                          */
	/***********************************/
	case AUDIO_TEST_HW_CLKGEN:
		if (AUDIO_TEST_DRV_STATE_ON == stat) {
			/***********************************/
			/* Status ON                       */
			/***********************************/
			if (!(AUDIO_TEST_CLK_CLKGEN &
				g_audio_test_clock_flag)) {
				clk = clk_get(NULL, "clkgen");
				if (IS_ERR(clk)) {
					audio_test_log_err(
						"clkget(clkgen) error");
				} else {
					clk_enable(clk);
					clk_put(clk);
					audio_test_log_info(
						"clkgen clock enable");
				}

				res = hwspin_lock_timeout_irqsave(
					r8a73734_hwlock_cpg, 10, &flags);
				if (0 > res)
					audio_test_log_err("Can't lock cpg\n");

				/* Soft Reset */
				sh_modify_register32(AUDIO_TEST_CPG_SRCR2,
							0, 0x01000000);
				udelay(62);
				/* CLKGEN operates */
				sh_modify_register32(AUDIO_TEST_CPG_SRCR2,
							0x01000000, 0);

				if (0 <= res)
					hwspin_unlock_irqrestore(
						r8a73734_hwlock_cpg, &flags);

				g_audio_test_clock_flag |=
					AUDIO_TEST_CLK_CLKGEN;
			}
		} else {
			/***********************************/
			/* Status OFF                      */
			/***********************************/
			if ((AUDIO_TEST_CLK_CLKGEN &
				g_audio_test_clock_flag)) {
				clk = clk_get(NULL, "clkgen");
				if (IS_ERR(clk)) {
					audio_test_log_err(
						"clkget(clkgen) error");
				} else {
					clk_disable(clk);
					clk_put(clk);
					audio_test_log_info(
						"clkgen clock disable");
				}

				g_audio_test_clock_flag &=
					~AUDIO_TEST_CLK_CLKGEN;
			}
		}
		break;
	/***********************************/
	/* FSI                             */
	/***********************************/
	case AUDIO_TEST_HW_FSI:
		if (AUDIO_TEST_DRV_STATE_ON == stat) {
			/***********************************/
			/* Status ON                       */
			/***********************************/
			if (!(AUDIO_TEST_CLK_FSI &
				g_audio_test_clock_flag)) {
				clk = clk_get(NULL, "fsi");
				if (IS_ERR(clk)) {
					audio_test_log_err(
						"clkget(fsi) error");
				} else {
					clk_enable(clk);
					clk_put(clk);
					audio_test_log_info(
						"fsi clock enable");
				}

				res = hwspin_lock_timeout_irqsave(
					r8a73734_hwlock_cpg, 10, &flags);
				if (0 > res)
					audio_test_log_err("Can't lock cpg\n");

				/* Soft Reset */
				sh_modify_register32(AUDIO_TEST_CPG_SRCR3,
							0, 0x10000000);
				udelay(62);
				/* FSI operates */
				sh_modify_register32(AUDIO_TEST_CPG_SRCR3,
							0x10000000, 0);

				if (0 <= res)
					hwspin_unlock_irqrestore(
						r8a73734_hwlock_cpg, &flags);

				g_audio_test_clock_flag |= AUDIO_TEST_CLK_FSI;
			}
		} else {
			/***********************************/
			/* Status OFF                      */
			/***********************************/
			if ((AUDIO_TEST_CLK_FSI & g_audio_test_clock_flag)) {
				clk = clk_get(NULL, "fsi");
				if (IS_ERR(clk)) {
					audio_test_log_err(
						"clkget(fsi) error\n");
				} else {
					clk_disable(clk);
					clk_put(clk);
					audio_test_log_info(
						"fsi clock disable");
				}

				g_audio_test_clock_flag &= ~AUDIO_TEST_CLK_FSI;
			}
		}
		break;
	/***********************************/
	/* SCUW                            */
	/***********************************/
	case AUDIO_TEST_HW_SCUW:
		if (AUDIO_TEST_DRV_STATE_ON == stat) {
			/***********************************/
			/* Status ON                       */
			/***********************************/
			if (!(AUDIO_TEST_CLK_SCUW &
				g_audio_test_clock_flag)) {
				clk = clk_get(NULL, "scuw");
				if (IS_ERR(clk)) {
					audio_test_log_err(
						"clkget(scuw) error\n");
				} else {
					clk_enable(clk);
					clk_put(clk);
					audio_test_log_info(
						"scuw clock enable");
				}

				res = hwspin_lock_timeout_irqsave(
					r8a73734_hwlock_cpg, 10, &flags);
				if (0 > res)
					audio_test_log_err("Can't lock cpg\n");

				/* Soft Reset */
				sh_modify_register32(AUDIO_TEST_CPG_SRCR3,
							0, 0x04000000);
				udelay(62);
				/* SCUW operates */
				sh_modify_register32(AUDIO_TEST_CPG_SRCR3,
							0x04000000, 0);

				if (0 <= res)
					hwspin_unlock_irqrestore(
						r8a73734_hwlock_cpg, &flags);

				g_audio_test_clock_flag |= AUDIO_TEST_CLK_SCUW;
			}
		} else {
			/***********************************/
			/* Status OFF                      */
			/***********************************/
			if ((AUDIO_TEST_CLK_SCUW &
				g_audio_test_clock_flag)) {
				clk = clk_get(NULL, "scuw");
				if (IS_ERR(clk)) {
					audio_test_log_err(
						"clkget(scuw) error\n");
				} else {
					clk_disable(clk);
					clk_put(clk);
					audio_test_log_info(
						"scuw clock disable");
				}

				g_audio_test_clock_flag &=
					~AUDIO_TEST_CLK_SCUW;
			}
		}
		break;
	/***********************************/
	/* Unknown                         */
	/***********************************/
	default:
		audio_test_log_err("unknown function");
		break;
	}

	audio_test_log_rfunc("");
}

/*!
  @brief	Common registers setting function.

  @param	drv [i] H/W type.
  @param	reg_tbl [i] Register table.
  @param	size [i] Register table size.

  @return	.

  @note		.
*/
static void audio_test_common_set_register(enum audio_test_hw_val drv,
				struct audio_test_common_reg_table *reg_tbl,
				u_int size)
{
	int i;

	audio_test_log_efunc("hw[%d]", drv);

	for (i = 0; size > i; i++) {
		audio_test_log_info("reg[%#010x] val[%#010x] delay[%d]",
					reg_tbl[i].uiReg,
					reg_tbl[i].uiValue,
					reg_tbl[i].uiDelay);

		if (0 != reg_tbl[i].uiDelay) {
			/***********************************/
			/* Delay                           */
			/***********************************/
			/* 1000 micro over */
			if (AUDIO_TEST_COMMON_UDELAY_MAX <=
					reg_tbl[i].uiDelay)
				mdelay((reg_tbl[i].uiDelay /
					AUDIO_TEST_COMMON_UDELAY_MAX));
			else
				udelay(reg_tbl[i].uiDelay);
		} else if (0 != reg_tbl[i].uiClrbit) {
			/***********************************/
			/* Modify                          */
			/***********************************/
			/* CLKGEN */
			if (AUDIO_TEST_HW_FSI == drv)
				sh_modify_register32(
					(g_audio_test_clkgen_Base +
						reg_tbl[i].uiReg),
					reg_tbl[i].uiClrbit,
					reg_tbl[i].uiValue);
			/* FSI */
			else if (AUDIO_TEST_HW_FSI == drv)
				sh_modify_register32(
					(g_audio_test_fsi_Base +
						reg_tbl[i].uiReg),
					reg_tbl[i].uiClrbit,
					reg_tbl[i].uiValue);
			/* SCUW */
			else
				sh_modify_register32(
					(g_audio_test_scuw_Base +
						reg_tbl[i].uiReg),
					reg_tbl[i].uiClrbit,
					reg_tbl[i].uiValue);
		} else {
			/***********************************/
			/* Register setting                */
			/***********************************/
			/* CLKGEN */
			if (AUDIO_TEST_HW_CLKGEN == drv)
				iowrite32(reg_tbl[i].uiValue,
					(g_audio_test_clkgen_Base +
						reg_tbl[i].uiReg));
			/* FSI */
			else if (AUDIO_TEST_HW_FSI == drv)
				iowrite32(reg_tbl[i].uiValue,
					(g_audio_test_fsi_Base +
						reg_tbl[i].uiReg));
			/* SCUW */
			else
				iowrite32(reg_tbl[i].uiValue,
					(g_audio_test_scuw_Base +
						reg_tbl[i].uiReg));
		}
	}

	audio_test_log_rfunc("");
}

/*!
  @brief	Set callback function for VCD Watch

  @param	.

  @return	Function results.

  @note		.
 */
static int audio_test_call_regist_watch(void)
{
	int ret = 0;
	struct vcd_execute_command cmd;
	struct vcd_watch_clkgen_info watch_clkgen_info;

	audio_test_log_efunc("");

	memset(&cmd, 0, sizeof(cmd));
	memset(&watch_clkgen_info, 0, sizeof(watch_clkgen_info));

	/***********************************/
	/* Set callback for VCD            */
	/***********************************/
	cmd.command = VCD_COMMAND_WATCH_CLKGEN;
	watch_clkgen_info.start_clkgen = audio_test_watch_start_clk_cb;
	watch_clkgen_info.stop_clkgen = audio_test_watch_stop_clk_cb;
	cmd.arg = &watch_clkgen_info;
	ret = vcd_execute_test_call(&cmd);
	if (0 != ret) {
		audio_test_log_err("vcd_execute_test_call");
		goto error;
	}

	audio_test_log_rfunc("ret[%d]", ret);
	return ret;
error:
	audio_test_log_err("ret[%d]", ret);
	return ret;
}

/*!
  @brief	Wake up start clkgen callback function

  @param	.

  @return	.

  @note		.
 */
static void audio_test_watch_start_clk_cb(void)
{
	audio_test_log_efunc("");

	atomic_set(&g_audio_test_watch_start_clk, 1);
	wake_up_interruptible(&g_watch_start_clk_queue);

	audio_test_log_rfunc("");
}

/*!
  @brief	Wake up stop clkgen callback function

  @param	.

  @return	.

  @note		.
 */
static void audio_test_watch_stop_clk_cb(void)
{
	audio_test_log_efunc("");

	atomic_set(&g_audio_test_watch_stop_clk, 1);
	wake_up_interruptible(&g_watch_stop_clk_queue);

	audio_test_log_rfunc("");
}

/*!
  @brief	Create proc entry.

  @param	name [i] Directory in proc.
  @param	proc_child [o] Proc entry.

  @return	Function results.

  @note		.
*/
static int audio_test_create_proc_entry(char *name,
					struct proc_dir_entry **proc_child)
{
	int ret = 0;

	audio_test_log_efunc("name[%s]", name);

	*proc_child = create_proc_entry(name,
					(S_IRUGO | S_IWUGO),
					audio_test_conf->proc_parent);
	if (NULL != *proc_child) {
		if (strcmp(name, AUDIO_TEST_LOG_LEVEL) == 0) {
			(*proc_child)->read_proc = audio_test_proc_log_read;
			(*proc_child)->write_proc = audio_test_proc_log_write;
		} else {
			audio_test_log_err("parameter error.");
			ret = -EINVAL;
		}
	} else {
		audio_test_log_err("create failed for %s.", name);
		ret = -ENOMEM;
	}

	audio_test_log_rfunc("ret[%d]", ret);
	return ret;
}

/*!
  @brief	Read proc file.

  @param	page [o] Writing range.
  @param	start [o] .
  @param	offset [o] .
  @param	count [o] .
  @param	eof [o] End of file.
  @param	data [o] .

  @return	Data length.

  @note		.
*/
static int audio_test_proc_log_read(char *page, char **start, off_t offset,
					int count, int *eof, void *data)
{
	int len = 0;

	audio_test_log_efunc("");

	len = sprintf(page, "%#010x\n", (int)audio_test_log_level);
	*eof = 1;

	audio_test_log_rfunc("len[%d]", len);
	return len;
}

/*!
  @brief	Write proc file.

  @param	filp [i] File pointer.
  @param	buffer [i] Data buffer.
  @param	count [i] Data count.
  @param	data [i] Data.

  @return	Data count or Function results.

  @note		.
*/
static int audio_test_proc_log_write(struct file *filp, const char *buffer,
					u_long count, void *data)
{
	int ret = 0;
	int in = 0;
	char *temp = NULL;

	audio_test_log_efunc("filp[%p] buffer[%p] count[%ld] data[%p]",
			filp, buffer, count, data);

	temp = kmalloc(count, GFP_KERNEL);
	memset(temp, 0, count);
	strncpy(temp, buffer, count);
	temp[count-1] = '\0';
	ret = kstrtoint(temp, 0, &in);
	kfree(temp);

	if (ret)
		return ret;

	audio_test_log_level = (u_int)in & AUDIO_TEST_LOG_LEVEL_MAX;

	audio_test_log_rfunc("count[%ld]", count);
	return count;
}

/*------------------------------------*/
/* for public function                */
/*------------------------------------*/
/*!
  @brief	Callback function of "open".

  @param	inode [i] I-NODE.
  @param	filp [i] File pointer.

  @return	Function results.

  @note		.
*/
int audio_test_open(struct inode *inode, struct file *filp)
{
	int ret = 0;

	audio_test_log_efunc("filp[%p]", filp);

	filp->private_data = audio_test_conf;

	audio_test_log_rfunc("ret[%d]", ret);
	return ret;
}

/*!
  @brief	Callback function of "close".

  @param	inode [i] I-NODE.
  @param	filp [i] File pointer.

  @return	Function results.

  @note		.
*/
int audio_test_close(struct inode *inode, struct file *filp)
{
	int ret = 0;

	audio_test_log_efunc("filp[%p]", filp);

	audio_test_log_rfunc("ret[%d]", ret);
	return ret;
}

/*!
  @brief	Callback function of "ioctl".

  @param	filp [i] File pointer.
  @param	cmd [i] IOCTL command.
  @param	arg [i] Argument.

  @return	Function results.

  @note		.
*/
long audio_test_ioctl(struct file *filp, u_int cmd, u_long arg)
{
	long ret = 0;
	struct audio_test_ioctl_cmd data;

	audio_test_log_efunc("filp[%p] cmd[%d]", filp, cmd);

	memset(&data, 0, sizeof(data));

	switch (cmd) {
	case AUDIO_TEST_IOCTL_SETDEVICE:
		if (!access_ok(VERIFY_WRITE, (void __user *)arg,
						_IOC_SIZE(cmd))) {
			ret = -EFAULT;
			goto done;
		}
		if (copy_from_user(&data, (int __user *)arg,
						_IOC_SIZE(cmd))) {
			ret = -EFAULT;
			goto done;
		}
		ret = audio_test_proc_set_device(data.in_device_type,
						data.out_device_type,
						data.out_LR_type,
						data.out_volume);
		break;

	case AUDIO_TEST_IOCTL_STARTSCUWLOOP:
		if (!access_ok(VERIFY_WRITE, (void __user *)arg,
						_IOC_SIZE(cmd))) {
			ret = -EFAULT;
			goto done;
		}
		if (copy_from_user(&data, (int __user *)arg,
						_IOC_SIZE(cmd))) {
			ret = -EFAULT;
			goto done;
		}
		ret = audio_test_proc_start_scuw_loopback(data.fsi_port);
		break;

	case AUDIO_TEST_IOCTL_STOPSCUWLOOP:
		if (!access_ok(VERIFY_WRITE, (void __user *)arg,
						_IOC_SIZE(cmd))) {
			ret = -EFAULT;
			goto done;
		}
		if (copy_from_user(&data, (int __user *)arg,
						_IOC_SIZE(cmd))) {
			ret = -EFAULT;
			goto done;
		}
		ret = audio_test_proc_stop_scuw_loopback();
		break;

	case AUDIO_TEST_IOCTL_DETECTJACK:
		if (!access_ok(VERIFY_READ, (void __user *)arg,
						_IOC_SIZE(cmd))) {
			ret = -EFAULT;
			goto done;
		}
		if (copy_from_user(&data, (int __user *)arg,
						_IOC_SIZE(cmd))) {
			ret = -EFAULT;
			goto done;
		}
		ret = audio_test_proc_detect_jack(data.detect_jack);

		if (copy_to_user((int __user *)arg, &data,
						_IOC_SIZE(cmd))) {
			ret = -EFAULT;
			goto done;
		}
		break;

	case AUDIO_TEST_IOCTL_DETECTKEY:
		if (!access_ok(VERIFY_READ, (void __user *)arg,
						_IOC_SIZE(cmd))) {
			ret = -EFAULT;
			goto done;
		}
		if (copy_from_user(&data, (int __user *)arg,
						_IOC_SIZE(cmd))) {
			ret = -EFAULT;
			goto done;
		}
		ret = audio_test_proc_detect_key(data.detect_key);

		if (copy_to_user((int __user *)arg, &data,
						_IOC_SIZE(cmd))) {
			ret = -EFAULT;
			goto done;
		}
		break;

	case AUDIO_TEST_IOCTL_STARTTONE:
		if (!access_ok(VERIFY_WRITE, (void __user *)arg,
						_IOC_SIZE(cmd))) {
			ret = -EFAULT;
			goto done;
		}
		if (copy_from_user(&data, (int __user *)arg,
						_IOC_SIZE(cmd))) {
			ret = -EFAULT;
			goto done;
		}
		ret = audio_test_proc_start_tone();
		break;

	case AUDIO_TEST_IOCTL_STOPTONE:
		if (!access_ok(VERIFY_WRITE, (void __user *)arg,
						_IOC_SIZE(cmd))) {
			ret = -EFAULT;
			goto done;
		}
		if (copy_from_user(&data, (int __user *)arg,
						_IOC_SIZE(cmd))) {
			ret = -EFAULT;
			goto done;
		}
		ret = audio_test_proc_stop_tone();
		break;

	case AUDIO_TEST_IOCTL_STARTSPUVLOOP:
		if (!access_ok(VERIFY_WRITE, (void __user *)arg,
						_IOC_SIZE(cmd))) {
			ret = -EFAULT;
			goto done;
		}
		if (copy_from_user(&data, (int __user *)arg,
						_IOC_SIZE(cmd))) {
			ret = -EFAULT;
			goto done;
		}
		ret = audio_test_proc_start_spuv_loopback(data.fsi_port,
							data.vqa_val,
							data.delay_val);
		break;

	case AUDIO_TEST_IOCTL_STOPSPUVLOOP:
		if (!access_ok(VERIFY_WRITE, (void __user *)arg,
						_IOC_SIZE(cmd))) {
			ret = -EFAULT;
			goto done;
		}
		if (copy_from_user(&data, (int __user *)arg,
						_IOC_SIZE(cmd))) {
			ret = -EFAULT;
			goto done;
		}
		ret = audio_test_proc_stop_spuv_loopback();
		break;

	default:
		audio_test_log_err("unknown command");
		ret = -ENOTTY;
		break;
	}

done:
	audio_test_log_rfunc("ret[%ld]", ret);
	return ret;
}

/*!
  @brief	Initialize.

  @param	.

  @return	Function results.

  @note		.
*/
static int __init audio_test_init(void)
{
	int ret = 0;
	struct audio_test_priv *dev_conf = NULL;

	if (D2153_INTRODUCE_BOARD_REV > u2_get_board_rev())
		return -ENODEV;

	audio_test_log_efunc("");

	/* register misc */
	ret = misc_register(&audio_test_misc_dev);
	if (ret) {
		audio_test_log_err("misc_register error.\n");
		goto rtn;
	}

	/***********************************/
	/* Get internal parameter area     */
	/***********************************/
	if (NULL == audio_test_conf) {
		dev_conf = kzalloc(sizeof(struct audio_test_priv), GFP_KERNEL);

		if (NULL == dev_conf) {
			ret = -ENOMEM;
			audio_test_log_err(
				"Could not allocate master. ret[%d]",
				ret);
			goto delete_misc;
		}
		audio_test_conf = dev_conf;
	} else {
		dev_conf = audio_test_conf;
	}

	/***********************************/
	/* create file for log level entry */
	/***********************************/
	audio_test_conf->proc_parent = proc_mkdir(AUDIO_TEST_DRV_NAME, NULL);
	if (NULL != audio_test_conf->proc_parent) {
		ret = audio_test_create_proc_entry(AUDIO_TEST_LOG_LEVEL,
						&audio_test_conf->log_entry);
		if (0 != ret) {
			audio_test_log_err("Create proc. ret[%d]", ret);
			goto delete_conf;
		}
	}

	/* Power domain setting */
	ret = power_domain_devices("snd-soc-audio-test",
				    &g_audio_test_power_domain,
				    &g_audio_test_power_domain_count);
	if (0 != ret) {
		audio_test_log_err("Power domain setting ret[%d]\n", ret);
		goto delete_log;
	}

	/* RuntimePM */
	pm_runtime_enable(g_audio_test_power_domain);
	ret = pm_runtime_resume(g_audio_test_power_domain);
	if (ret < 0) {
		audio_test_log_err("pm_runtime_resume ret[%d]\n", ret);
		goto remove_pm;
	}

	/***********************************/
	/* Regist callback for VCD         */
	/***********************************/
	audio_test_call_regist_watch();

	/* Add not to be suspend in loopback */
	wake_lock_init(&g_audio_test_wake_lock,
		       WAKE_LOCK_SUSPEND,
		       "snd-soc-audio-test");

	/***********************************/
	/* Get logical address             */
	/***********************************/
	ret = audio_test_get_logic_addr();
	if (0 != ret) {
		audio_test_log_err("audio_test_get_logic_addr ret[%d]\n", ret);
		goto destroy_wakelock;
	}

	goto rtn;

destroy_wakelock:
	/* Add not to be suspend in loopback */
	wake_lock_destroy(&g_audio_test_wake_lock);
remove_pm:
	if (g_audio_test_power_domain) {
		pm_runtime_disable(g_audio_test_power_domain);
		g_audio_test_power_domain = NULL;
	}
delete_log:
	if (audio_test_conf->log_entry)
		remove_proc_entry(AUDIO_TEST_LOG_LEVEL,
				audio_test_conf->proc_parent);
	if (audio_test_conf->proc_parent)
		remove_proc_entry(AUDIO_TEST_DRV_NAME, NULL);
delete_conf:
	kfree(dev_conf);
delete_misc:
	misc_deregister(&audio_test_misc_dev);

rtn:
	audio_test_log_rfunc("ret[%d]", ret);
	return ret;
}

/*!
  @brief	Exit.

  @param	.

  @return	.

  @note		.
*/
static void __exit audio_test_exit(void)
{
	if (D2153_INTRODUCE_BOARD_REV > u2_get_board_rev())
		return;

	audio_test_log_efunc("");

	misc_deregister(&audio_test_misc_dev);

	/***********************************/
	/* Remove log proc                 */
	/***********************************/
	if (audio_test_conf->log_entry)
		remove_proc_entry(AUDIO_TEST_LOG_LEVEL,
				audio_test_conf->proc_parent);
	if (audio_test_conf->proc_parent)
		remove_proc_entry(AUDIO_TEST_DRV_NAME, NULL);

	/* Add not to be suspend in loopback */
	wake_lock_destroy(&g_audio_test_wake_lock);

	/***********************************/
	/* Release logical address         */
	/***********************************/
	audio_test_rel_logic_addr();

	/***********************************/
	/* Free internal parameter area    */
	/***********************************/
	kfree(audio_test_conf);

	if (g_audio_test_power_domain) {
		pm_runtime_disable(g_audio_test_power_domain);
		g_audio_test_power_domain = NULL;
	}

	audio_test_log_rfunc("");
}

module_init(audio_test_init);
module_exit(audio_test_exit);

MODULE_LICENSE("GPL v2");
