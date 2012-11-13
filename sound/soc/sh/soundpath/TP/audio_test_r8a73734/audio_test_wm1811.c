/* audio_test_wm1811.c
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

/*!
  @file		audio_test_wm1811.c

  @brief	Audio test command source file.
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
#include <mach/r8a73734.h>
#include <linux/hwspinlock.h>

#include <sound/soundpath/TP/audio_test_extern.h>
#include "audio_test_wm1811.h"
#include "audio_test_reg_wm1811.h"

/*---------------------------------------------------------------------------*/
/* typedef declaration (private)                                             */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* define macro declaration (private)                                        */
/*---------------------------------------------------------------------------*/
/*!
  @brief	Detect bit.
*/
#define AUDIO_TEST_JACK_BIT_IN		(0x0001)/**< Insertion and Removal. */
#define AUDIO_TEST_JACK_BIT_BTN		(0x0004)/**< Accessory Button. */

/*!
  @brief	Mic volume.
*/
#define AUDIO_TEST_MAINMIC_VOL		(0x000B)/**< Main mic volume. */
#define AUDIO_TEST_SUBMIC_VOL		(0x000B)/**< Sub mic volume. */
#define AUDIO_TEST_HEADSETMIC_VOL	(0x000B)/**< Headset mic volume. */

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
/*!
  @brief	Volume level.
*/
enum audio_test_volume {
	AUDIO_TEST_VOLUME_0,
	AUDIO_TEST_VOLUME_1,			/**< Volume level 1. */
	AUDIO_TEST_VOLUME_2,			/**< Volume level 2. */
	AUDIO_TEST_VOLUME_3,			/**< Volume level 3. */
	AUDIO_TEST_VOLUME_4,			/**< Volume level 4. */
	AUDIO_TEST_VOLUME_5,			/**< Volume level 5. */
	AUDIO_TEST_VOLUME_MAX
};

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
static int audio_test_tuneup_ic(u_int in_device_type, u_int out_device_type);
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
  @brief	Device count.
*/
static int audio_test_devs = 1;
/*!
  @brief	MAJOR: dynamic allocation.
*/
static int audio_test_major;
/*!
  @brief	MINOR: static allocation.
*/
static int audio_test_minor;
/*!
  @brief	Audio Test control device.
*/
static struct cdev audio_test_cdev;
/*!
  @brief	Audio Test class.
*/
static struct class *audio_test_class;
/*!
  @brief	Audio Test device.
*/
static dev_t audio_test_dev;
/*!
  @brief	Audio Test file operations.
*/
static const struct file_operations audio_test_fops = {
	.owner = THIS_MODULE,
	.open = audio_test_open,
	.release = audio_test_close,
	.unlocked_ioctl = audio_test_ioctl,
};
/***********************************/
/* internal parameter              */
/***********************************/
/*!
  @brief	Store the AudioLSI driver config.
*/
static struct audio_test_priv *audio_test_conf;
/***********************************/
/* HW clock flag                   */
/***********************************/
/*!
  @brief	Clock status flag.
*/
static u_int g_audio_test_clock_flag;
/*!
  @brief	Loopback flag.
*/
static u_int g_audio_test_loopback = AUDIO_TEST_DRV_STATE_OFF;
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
/***********************************/
/* Table for volume                */
/***********************************/
/*!
  @brief	Speker volume (00_0000:-57dB - 11_1111:+6dB).
*/
static u_short audio_test_tbl_speaker_vol[] = {
	0x0000, 0x004D, 0x005C, 0x006A, 0x0079, 0x007F
};
/*!
  @brief	Headphone volume (00_0000:-57dB - 11_1111:+6dB).
*/
static u_short audio_test_tbl_headphone_vol[] = {
	0x0000, 0x004D, 0x005C, 0x006A, 0x0076, 0x007F
};
/*!
  @brief	Earpiece volume (00_0000:-57dB - 11_1111:+6dB).
*/
static u_short audio_test_tbl_earpiece_vol[] = {
	0x0000, 0x004D, 0x005C, 0x006A, 0x0079, 0x007F
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
	u_short reg_l = 0;
	u_short reg_r = 0;

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

	/* AIF1 disable */
	ret = audio_test_ic_read(0x0200, &reg);
	if (0 != ret) {
		audio_test_log_err("audio_test_ic_read");
		goto error;
	}
	audio_test_log_info("aif1 addr[0x0200] reg[%#010x]", reg);
	reg = reg & 0xFFFE;
	audio_test_log_info("aif1 addr[0x0200] convert reg[%#010x]", reg);
	ret = audio_test_ic_write(0x0200, reg);
	if (0 != ret) {
		audio_test_log_err("audio_test_ic_write");
		goto error;
	}

	/***********************************/
	/* Set volume                      */
	/***********************************/
	switch (in_device_type) {
	case AUDIO_TEST_DRV_IN_MIC:
		ret = audio_test_ic_write(0x0018, AUDIO_TEST_MAINMIC_VOL);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_write");
			goto error;
		}
		ret = audio_test_ic_write(0x001B, AUDIO_TEST_SUBMIC_VOL);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_write");
			goto error;
		}

		ret = audio_test_ic_write(0x0018,
					AUDIO_TEST_MAINMIC_VOL | 0x0100);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_write");
			goto error;
		}
		ret = audio_test_ic_write(0x001B,
					AUDIO_TEST_SUBMIC_VOL | 0x0100);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_write");
			goto error;
		}

		ret = audio_test_ic_read(0x0018, &reg_l);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_read");
			goto error;
		}
		audio_test_log_info("main mic addr[0x0018] reg[%#010x]", reg_l);
		ret = audio_test_ic_read(0x001B, &reg_r);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_read");
			goto error;
		}
		audio_test_log_info("sub mic addr[0x001B] reg[%#010x]", reg_r);
		break;

	case AUDIO_TEST_DRV_IN_HEADSETMIC:
		ret = audio_test_ic_write(0x001A, AUDIO_TEST_HEADSETMIC_VOL);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_write");
			goto error;
		}

		ret = audio_test_ic_write(0x001A,
					AUDIO_TEST_HEADSETMIC_VOL | 0x0100);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_write");
			goto error;
		}

		ret = audio_test_ic_read(0x001A, &reg_l);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_read");
			goto error;
		}
		audio_test_log_info("hs mic addr[0x001A] reg[%#010x]", reg_l);
		break;

	default:
		audio_test_log_info("unknown input device");
		break;
	}

	switch (out_volume) {
	case AUDIO_TEST_VOLUME_1:
	case AUDIO_TEST_VOLUME_2:
	case AUDIO_TEST_VOLUME_3:
	case AUDIO_TEST_VOLUME_4:
	case AUDIO_TEST_VOLUME_5:
		break;
	default:
		audio_test_log_info("invalid volume");
		ret = -1;
		goto error;
		break;
	}
	switch (out_device_type) {
	case AUDIO_TEST_DRV_OUT_SPEAKER:
		ret = audio_test_ic_write(0x0003, 0x0030);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_write");
			goto error;
		}
		ret = audio_test_ic_write(0x0026,
			audio_test_tbl_speaker_vol[out_volume]);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_write");
			goto error;
		}
		ret = audio_test_ic_write(0x0027,
			audio_test_tbl_speaker_vol[out_volume]);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_write");
			goto error;
		}
		ret = audio_test_ic_write(0x0003, 0x0330);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_write");
			goto error;
		}

		ret = audio_test_ic_write(0x0003, 0x0030);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_write");
			goto error;
		}
		ret = audio_test_ic_write(0x0026,
			audio_test_tbl_speaker_vol[out_volume] | 0x0100);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_write");
			goto error;
		}
		ret = audio_test_ic_write(0x0027,
			audio_test_tbl_speaker_vol[out_volume] | 0x0100);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_write");
			goto error;
		}
		ret = audio_test_ic_write(0x0003, 0x0330);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_write");
			goto error;
		}

		ret = audio_test_ic_read(0x0026, &reg_l);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_read");
			goto error;
		}
		audio_test_log_info("speaker addr[0x0026] reg[%#010x]", reg_l);
		ret = audio_test_ic_read(0x0027, &reg_r);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_read");
			goto error;
		}
		audio_test_log_info("speaker addr[0x0027] reg[%#010x]", reg_r);
		break;

	case AUDIO_TEST_DRV_OUT_HEADPHONE:
		ret = audio_test_ic_write(0x001C,
			audio_test_tbl_headphone_vol[out_volume]);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_write");
			goto error;
		}
		ret = audio_test_ic_write(0x001D,
			audio_test_tbl_headphone_vol[out_volume]);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_write");
			goto error;
		}

		ret = audio_test_ic_write(0x001C,
			audio_test_tbl_headphone_vol[out_volume] | 0x0100);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_write");
			goto error;
		}
		ret = audio_test_ic_write(0x001D,
			audio_test_tbl_headphone_vol[out_volume] | 0x0100);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_write");
			goto error;
		}

		ret = audio_test_ic_read(0x001C, &reg_l);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_read");
			goto error;
		}
		audio_test_log_info("headphone addr[0x001C] reg[%#010x]",
					reg_l);
		ret = audio_test_ic_read(0x001D, &reg_r);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_read");
			goto error;
		}
		audio_test_log_info("headphone addr[0x001D] reg[%#010x]",
					reg_r);
		break;

	case AUDIO_TEST_DRV_OUT_EARPIECE:
		ret = audio_test_ic_write(0x0003, 0x0030);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_write");
			goto error;
		}
		ret = audio_test_ic_write(0x0020,
			audio_test_tbl_earpiece_vol[out_volume]);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_write");
			goto error;
		}
		ret = audio_test_ic_write(0x0021,
			audio_test_tbl_earpiece_vol[out_volume]);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_write");
			goto error;
		}
		ret = audio_test_ic_write(0x0003, 0x00F0);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_write");
			goto error;
		}

		ret = audio_test_ic_write(0x0003, 0x0030);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_write");
			goto error;
		}
		ret = audio_test_ic_write(0x0020,
			(audio_test_tbl_earpiece_vol[out_volume]) | 0x0100);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_write");
			goto error;
		}
		ret = audio_test_ic_write(0x0021,
			(audio_test_tbl_earpiece_vol[out_volume]) | 0x0100);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_write");
			goto error;
		}
		ret = audio_test_ic_write(0x0003, 0x00F0);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_write");
			goto error;
		}

		ret = audio_test_ic_read(0x0020, &reg_l);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_read");
			goto error;
		}
		audio_test_log_info("earpiece addr[0x0020] reg[%#010x]", reg_l);
		ret = audio_test_ic_read(0x0021, &reg_r);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_read");
			goto error;
		}
		audio_test_log_info("earpiece addr[0x0021] reg[%#010x]", reg_r);
		break;

	default:
		audio_test_log_info("unknown output device");
		break;
	}

	/***********************************/
	/* Set LR                          */
	/***********************************/
	/* 0x0001 (Power Management (1)) */
	ret = audio_test_ic_read(0x0001, &oe);
	if (0 != ret) {
		audio_test_log_err("audio_test_ic_read");
		goto error;
	}

	audio_test_log_info("old output enable[%#010x]", oe);

	switch (out_device_type) {
	case AUDIO_TEST_DRV_OUT_SPEAKER:
	case AUDIO_TEST_DRV_OUT_HEADPHONE:
	/* case AUDIO_TEST_DRV_OUT_EARPIECE: */
		audio_test_cnv_oe(out_device_type, out_LR_type, &oe);

		ret = audio_test_ic_write(0x0001, oe);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_write");
			goto error;
		}
		break;
	case AUDIO_TEST_DRV_OUT_EARPIECE:
		/* NOP */
		break;
	default:
		audio_test_log_info("unknown output device");
		break;
	}

	audio_test_log_info("new output enable[%#010x]", oe);

	/* AIF1 enable */
	ret = audio_test_ic_read(0x0200, &reg);
	if (0 != ret) {
		audio_test_log_err("audio_test_ic_read");
		goto error;
	}
	audio_test_log_info("aif1 addr[0x0200] reg[%#010x]", reg);
	reg = reg | 0x0001;
	audio_test_log_info("aif1 addr[0x0200] convert reg[%#010x]", reg);
	ret = audio_test_ic_write(0x0200, reg);
	if (0 != ret) {
		audio_test_log_err("audio_test_ic_write");
		goto error;
	}

	/***********************************/
	/* Tune up Audio IC                */
	/***********************************/
	ret = audio_test_tuneup_ic(in_device_type, out_device_type);
	if (0 != ret) {
		audio_test_log_err("audio_test_tuneup_ic");
		goto error;
	}

	audio_test_log_rfunc("ret[%d]", ret);
	return ret;

error:
	audio_test_log_err("ret[%d]", ret);
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

	g_audio_test_loopback = AUDIO_TEST_DRV_STATE_ON;

error:
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

	g_audio_test_loopback = AUDIO_TEST_DRV_STATE_OFF;

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
	u_short reg = 0;
	u_short jack_reg = 0;
	u_short state_jack = 0;

	audio_test_log_efunc("");

	/* JACKDET_MODE B'01 */
	ret = audio_test_ic_read(0x0039, &reg);
	if (0 != ret) {
		audio_test_log_err("audio_test_ic_read");
		goto error;
	}
	audio_test_log_info("JACKDET_MODE addr[0x0039] reg[%#010x]", reg);
	if (AUDIO_TEST_DRV_STATE_OFF == g_audio_test_loopback) {
		jack_reg = reg;
		reg = reg | 0x0080;
		reg = reg & 0xFEFF;
		audio_test_log_info(
			"JACKDET_MODE addr[0x0039] convert reg[%#010x]", reg);
		ret = audio_test_ic_write(0x0039, reg);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_write");
			goto error;
		}
		/* MICD_ENA B'1 */
		ret = audio_test_ic_read(0x00D0, &reg);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_read");
			goto error;
		}
		audio_test_log_info("MICD_ENA addr[0x00D0] reg[%#010x]", reg);
		reg = reg | 0x0001;
		audio_test_log_info(
			"MICD_ENA mode addr[0x00D0] convert reg[%#010x]", reg);
		ret = audio_test_ic_write(0x00D0, reg);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_write");
			goto error;
		}
		/* GP1_FN B'101 */
		ret = audio_test_ic_read(0x0700, &reg);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_read");
			goto error;
		}
		audio_test_log_info("GP1_FN addr[0x0700] reg[%#010x]", reg);
		reg = reg | 0x0005;
		reg = reg & 0xFFFD;
		audio_test_log_info("GP1_FN addr[0x0700] convert reg[%#010x]",
					reg);
		ret = audio_test_ic_write(0x0700, reg);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_write");
			goto error;
		}

		mdelay(2000);
	}

	/***********************************/
	/* Get state of jack               */
	/***********************************/
	/* 0x00D2 (Mic Detect 3) */
	ret = audio_test_ic_read(0x00D2, &state_jack);
	if (0 != ret) {
		audio_test_log_err("audio_test_ic_read");
		goto error;
	}

	audio_test_log_info("state[%#010x]", state_jack);

	if (state_jack & AUDIO_TEST_JACK_BIT_IN)
		*state = AUDIO_TEST_DRV_STATE_ON;
	else
		*state = AUDIO_TEST_DRV_STATE_OFF;

	if (AUDIO_TEST_DRV_STATE_OFF == g_audio_test_loopback) {
		/* JACKDET_MODE B'10 */
		ret = audio_test_ic_read(0x0039, &reg);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_read");
			goto error;
		}
		audio_test_log_info(
			"JACKDET_MODE addr[0x0039] reg[%#010x]", reg);
		reg = reg & 0xFF7F;
		reg = reg | 0x0100;
		audio_test_log_info(
			"JACKDET_MODE addr[0x0039] convert reg[%#010x]", reg);
		ret = audio_test_ic_write(0x0039, reg);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_write");
			goto error;
		}
		/* MICD_ENA B'0 */
		ret = audio_test_ic_read(0x00D0, &reg);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_read");
			goto error;
		}
		audio_test_log_info("MICD_ENA addr[0x00D0] reg[%#010x]", reg);
		reg = reg & 0xFFFE;
		audio_test_log_info(
			"MICD_ENA mode addr[0x00D0] convert reg[%#010x]", reg);
		ret = audio_test_ic_write(0x00D0, reg);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_write");
			goto error;
		}
		/* GP1_FN B'011 */
		ret = audio_test_ic_read(0x0700, &reg);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_read");
			goto error;
		}
		audio_test_log_info("GP1_FN addr[0x0700] reg[%#010x]", reg);
		reg = reg | 0x0003;
		reg = reg & 0xFFFB;
		audio_test_log_info("GP1_FN addr[0x0700] convert reg[%#010x]",
					reg);
		ret = audio_test_ic_write(0x0700, reg);
		if (0 != ret) {
			audio_test_log_err("audio_test_ic_write");
			goto error;
		}
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
	/* 0x00D2 (Mic Detect 3) */
	ret = audio_test_ic_read(0x00D2, &state_jack);
	if (0 != ret) {
		audio_test_log_err("audio_test_ic_read");
		goto error;
	}

	audio_test_log_info("state[%#010x]", state_jack);

	if (state_jack & AUDIO_TEST_JACK_BIT_BTN)
		*state = AUDIO_TEST_DRV_STATE_ON;
	else
		*state = AUDIO_TEST_DRV_STATE_OFF;

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

	g_audio_test_loopback = AUDIO_TEST_DRV_STATE_ON;

	audio_test_log_rfunc("ret[%d]", ret);
	return ret;

error:
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

	g_audio_test_loopback = AUDIO_TEST_DRV_STATE_OFF;

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

	g_audio_test_loopback = AUDIO_TEST_DRV_STATE_ON;

	audio_test_log_rfunc("ret[%d]", ret);
	return ret;

error:
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

	g_audio_test_loopback = AUDIO_TEST_DRV_STATE_OFF;

	audio_test_log_rfunc("ret[%d]", ret);
	return ret;

error:
	audio_test_log_err("ret[%d]", ret);
	return ret;
}

/*!
  @brief	Tune up Audio IC.

  @param	in_device_type [i] Input device.
  @param	out_device_type [i] Output device.

  @return	Function results.

  @note		.
*/
static int audio_test_tuneup_ic(u_int in_device_type, u_int out_device_type)
{
	int ret = 0;

	audio_test_log_efunc("in_dev[%d] out_dev[%d]",
				in_device_type, out_device_type);

	audio_test_log_rfunc("ret[%d]", ret);
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
	dev_t dev = MKDEV(audio_test_major, 0);
	int alloc_ret = 0;
	int major = 0;
	int cdev_err = 0;
	struct device *dev_cre = NULL;
	struct audio_test_priv *dev_conf = NULL;

	audio_test_log_efunc("");

	/***********************************/
	/* Create device class             */
	/***********************************/
	alloc_ret = alloc_chrdev_region(&dev, 0, audio_test_devs, "audio_test");
	if (alloc_ret) {
		audio_test_log_err("alloc_chrdev_region ret[%d]", alloc_ret);
		ret = -1;
		goto error;
	}
	audio_test_major = major = MAJOR(dev);

	cdev_init(&audio_test_cdev, &audio_test_fops);
	audio_test_cdev.owner = THIS_MODULE;
	audio_test_cdev.ops = &audio_test_fops;
	cdev_err = cdev_add(&audio_test_cdev,
		MKDEV(audio_test_major, audio_test_minor), 1);
	if (cdev_err) {
		audio_test_log_err("cdev_add ret[%d]", cdev_err);
		ret = -1;
		goto error;
	}

	audio_test_class = class_create(THIS_MODULE, "audio_test");
	if (IS_ERR(audio_test_class)) {
		audio_test_log_err("class_create");
		ret = -1;
		goto error;
	}
	audio_test_dev = MKDEV(audio_test_major, audio_test_minor);
	dev_cre = device_create(
		audio_test_class,
		NULL,
		audio_test_dev,
		NULL,
		"audio_test%d",
		audio_test_minor);

	audio_test_log_info("audio_test_driver(major %d) installed.", major);

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
			goto error;
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
			goto error;
		}
	}

	/* Power domain setting */
	ret = power_domain_devices("snd-soc-audio-test",
				    &g_audio_test_power_domain,
				    &g_audio_test_power_domain_count);
	if (0 != ret) {
		audio_test_log_err("Power domain setting ret[%d]\n", ret);
		goto error;
	}

	/* RuntimePM */
	pm_runtime_enable(g_audio_test_power_domain);
	ret = pm_runtime_resume(g_audio_test_power_domain);
	if (ret < 0) {
		audio_test_log_err("pm_runtime_resume ret[%d]\n", ret);
		goto error;
	}

	/***********************************/
	/* Regist callback for VCD         */
	/***********************************/
	audio_test_call_regist_watch();

	/***********************************/
	/* Get logical address             */
	/***********************************/
	ret = audio_test_get_logic_addr();

	audio_test_log_rfunc("ret[%d]", ret);
	return ret;

error:
	/***********************************/
	/* Delete device class             */
	/***********************************/
	if (0 == cdev_err)
		cdev_del(&audio_test_cdev);

	if (0 == alloc_ret)
		unregister_chrdev_region(dev, audio_test_devs);

	/***********************************/
	/* Remove log proc                 */
	/***********************************/
	if (audio_test_conf->log_entry)
		remove_proc_entry(AUDIO_TEST_LOG_LEVEL,
				audio_test_conf->proc_parent);
	if (audio_test_conf->proc_parent)
		remove_proc_entry(AUDIO_TEST_DRV_NAME, NULL);

	/***********************************/
	/* Free internal parameter area    */
	/***********************************/
	kfree(dev_conf);

	if (g_audio_test_power_domain) {
		pm_runtime_disable(g_audio_test_power_domain);
		g_audio_test_power_domain = NULL;
	}

	audio_test_log_err("ret[%d]", ret);
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
	dev_t dev = MKDEV(audio_test_major, 0);

	audio_test_log_efunc("");

	/***********************************/
	/* Delete device class             */
	/***********************************/
	device_destroy(audio_test_class, audio_test_dev);
	class_destroy(audio_test_class);

	cdev_del(&audio_test_cdev);
	unregister_chrdev_region(dev, audio_test_devs);

	audio_test_log_info("audio_test_driver removed.");

	/***********************************/
	/* Remove log proc                 */
	/***********************************/
	if (audio_test_conf->log_entry)
		remove_proc_entry(AUDIO_TEST_LOG_LEVEL,
				audio_test_conf->proc_parent);
	if (audio_test_conf->proc_parent)
		remove_proc_entry(AUDIO_TEST_DRV_NAME, NULL);

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
