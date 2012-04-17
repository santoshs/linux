/* max97236_defs.h
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
  @file max97236_defs.h

  @brief max97236 header file.
*/

#ifndef __MAX97236_DEFS_H__
#define __MAX97236_DEFS_H__

/*---------------------------------------------------------------------------*/
/* include files                                                             */
/*---------------------------------------------------------------------------*/
#include "max98090_common_defs.h"

/*---------------------------------------------------------------------------*/
/* typedef declaration                                                       */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* define macro declaration                                                  */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* define function macro declaration                                         */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* enum declaration                                                          */
/*---------------------------------------------------------------------------*/
/*!
  @brief MAX97236 Register ID value.
*/
enum MAX97236_REG_ID {
	MAX97236_REG_OR_STATUS1,
	/**< 0x00: Configuration and Device Status Registers1. */
	MAX97236_REG_OR_STATUS2,
	/**< 0x01: Configuration and Device Status Registers2. */
	MAX97236_REG_OR_STATUS3,
	/**< 0x02: Configuration and Device Status Registers3. */
	MAX97236_REG_RW_IRQ_MASK1,
	/**< 0x04: Interrupt Mask Registers1. */
	MAX97236_REG_RW_IRQ_MASK2,
	/**< 0x05: Interrupt Mask Registers2. */
	MAX97236_REG_RW_LEFT_VOLUME,
	/**< 0x07: Headphone Volume Registers1. */
	MAX97236_REG_RW_RIGHT_VOLUME,
	/**< 0x08: Headphone Volume Registers2. */
	MAX97236_REG_RW_MICROPHONE,
	/**< 0x09: Microphone Bias and Gain Register. */
	MAX97236_REG_OR_VENDOR_ID_REGISTER,
	/**< 0x0B: Vendor ID Register. */
	MAX97236_REG_RW_KEYSCAN_CLOCK_DIVIDER_1,
	/**< 0x12: Keyscan Clock Divider Registers1. */
	MAX97236_REG_RW_KEYSCAN_CLOCK_DIVIDER_2,
	/**< 0x13: Keyscan Clock Divider Registers2. */
	MAX97236_REG_RW_KEYSCAN_CLOCK_DIVIDER_ADC,
	/**< 0x14: Keyscan ADC Clock Divider Registers. */
	MAX97236_REG_RW_KEYSCAN_DEBOUNCE,
	/**< 0x15: Keyscan Debounce Register. */
	MAX97236_REG_RW_KEYSCAN_DELAY,
	/**< 0x16: Keyscan Delay Register. */
	MAX97236_REG_OR_PASSIVE_MBH_KEYSCAN_DATA,
	/**< 0x17: Keyscan Data Register. */
	MAX97236_REG_RW_DC_TEST_SLEW_CONTROL,
	/**< 0x18: Ramp Test Slew Control. */
	MAX97236_REG_RW_STATE_FORCING,
	/**< 0x19: Load State Forcing. */
	MAX97236_REG_RW_AC_TEST_CONTRO,
	/**< 0x1A: Jack Detect Test Hardware Settings. */
	MAX97236_REG_RW_ENABLE1,
	/**< 0x1D: Enable Registers1. */
	MAX97236_REG_RW_ENABLE2,
	/**< 0x1E: Enable Registers2. */
	MAX97236_REG_ID_MAX
};

/*!
  @brief MAX97236 Register Bit ID value.
*/
enum MAX97236_BIT_ID {
	MAX97236_BIT_STATUS1_JKIN,
	/**< 0x00[7]: Jack Detected. */
	MAX97236_BIT_STATUS1_DDONE,
	/**< 0x00[6]: Jack Configuration Detect Done. */
	MAX97236_BIT_STATUS1_VOL,
	/**< 0x00[5]: Volume Slew Complete. */
	MAX97236_BIT_STATUS1_MIC_IN,
	/**< 0x00[3]: Microphone Connected/Disconnected. */
	MAX97236_BIT_STATUS1_JACKSW,
	/**< 0x00[2]: JACKSW Status. */
	MAX97236_BIT_STATUS1_MCSW,
	/**< 0x00[1]: Microphone Switch Status. */
	MAX97236_BIT_STATUS1_MBH,
	/**< 0x00[0]: Multibutton Headset Status. */
	MAX97236_BIT_STATUS2_LINE_L,
	/**< 0x01[7]: Line-Level Load on TIP Detected. */
	MAX97236_BIT_STATUS2_LINE_R,
	/**< 0x01[6]: Line-Level Load on ROUT Detected. */
	MAX97236_BIT_STATUS2_HP_L,
	/**< 0x01[5]: Headphone Load on TIP Detected. */
	MAX97236_BIT_STATUS2_HP_R,
	/**< 0x01[4]: Headphone Load on RING1 Detected. */
	MAX97236_BIT_STATUS2_JACKSWINC,
	/**< 0x01[3]: JACKSW Incorrect. */
	MAX97236_BIT_STATUS2_KEY,
	/**< 0x01[2]: Passive Multibutton Headset KEY Status. */
	MAX97236_BIT_STATUS3_GND,
	/**< 0x02[1]: Jack Common Location Identifier. */
	MAX97236_BIT_IRQ_MASK1_IJKIN,
	/**< 0x04[7]: Jack Detect Interrupt Enable. */
	MAX97236_BIT_IRQ_MASK1_DDONE,
	/**< 0x04[6]: Jack Configuration Detect Done Interrupt Enable. */
	MAX97236_BIT_IRQ_MASK1_IVOL,
	/**< 0x04[5]: Volume Slew Interrupt Enable. */
	MAX97236_BIT_IRQ_MASK1_IMIC,
	/**< 0x04[3]: Microphone Interrupt Enable. */
	MAX97236_BIT_IRQ_MASK1_ACKSW,
	/**< 0x04[2]: JACKSW Status Interrupt Enable. */
	MAX97236_BIT_IRQ_MASK1_IMCSW,
	/**< 0x04[1]: Microphone Switch Interrupt Enable. */
	MAX97236_BIT_IRQ_MASK1_IMBH,
	/**< 0x04[0]: Multibutton Release Status Interrupt Enable. */
	MAX97236_BIT_IRQ_MASK2_ILINE_L,
	/**< 0x05[7]: Line-Level Load TIP Interrupt Enable. */
	MAX97236_BIT_IRQ_MASK2_ILINE_R,
	/**< 0x05[6]: Line-Level Load RING1 Interrupt Enable. */
	MAX97236_BIT_IRQ_MASK2_IHP_L,
	/**< 0x05[5]: Headphone Load TIP Interrupt Enable. */
	MAX97236_BIT_IRQ_MASK2_IHP_R,
	/**< 0x05[4]: Headphone Load RING1 Interrupt Enable. */
	MAX97236_BIT_IRQ_MASK2_IJACKSW,
	/**< 0x05[3]: JACKSW Incorrect Interrupt Enable. */
	MAX97236_BIT_IRQ_MASK2_IKEY,
	/**< 0x05[2]: KEY Interrupt Enable. */
	MAX97236_BIT_LEFT_VOLUME_L_EQUAL_R,
	/**< 0x07[7]: Left/Right Tracking. */
	MAX97236_BIT_LEFT_VOLUME_MUTEL,
	/**< 0x07[6]: Headphone Mute. */
	MAX97236_BIT_LEFT_VOLUME_LVOL,
	/**< 0x07[5]: Left Headphone Output Volume Level. */
	MAX97236_BIT_RIGHT_VOLUME_MUTER,
	/**< 0x08[6]: Headphone Mute. */
	MAX97236_BIT_RIGHT_VOLUME_RVOL,
	/**< 0x08[5]: Left/Right Headphone Output Volume Level. */
	MAX97236_BIT_MICROPHONE_GAIN,
	/**< 0x09[6]: Microphone Preamplifier Gain Select. */
	MAX97236_BIT_MICROPHONE_MICR,
	/**< 0x09[5]: Microphone Bias Resistor Select. */
	MAX97236_BIT_MICROPHONE_BIAS,
	/**< 0x09[2]: Microphone Bias Voltage Select. */
	MAX97236_BIT_VENDOR_ID_REGISTER_ID,
	/**< 0x0B[7]: Vendor ID. */
	MAX97236_BIT_KEYSCAN_CLOCK_DIVIDER_1_KEY_DIV_HIGH,
	/**< 0x12[7]: Keyscan Clock Divider. */
	MAX97236_BIT_KEYSCAN_CLOCK_DIVIDER_2_KEY_DIV_LOW,
	/**< 0x13[7]: Keyscan Clock Divider. */
	MAX97236_BIT_KEYSCAN_CLOCK_DIVIDER_ADC_KEY_DIV_ADC,
	/**< 0x14[7]: Keyscan ADC Clock Divider. */
	MAX97236_BIT_KEYSCAN_DEBOUNCE_KEY_DEB,
	/**< 0x15[7]: Keyscan Debounce Register. */
	MAX97236_BIT_KEYSCAN_DELAY_KEY_DEL,
	/**< 0x16[7]: Keyscan Delay Register. */
	MAX97236_BIT_PASSIVE_MBH_KEYSCAN_DATA_PRESS,
	/**< 0x17[7]: Release. */
	MAX97236_BIT_PASSIVE_MBH_KEYSCAN_DATA_RANGE,
	/**< 0x17[6]: */
	MAX97236_BIT_PASSIVE_MBH_KEYSCAN_DATA_KEYDATA,
	/**< 0x17[5]: Keyscan Data. */
	MAX97236_BIT_DC_TEST_SLEW_CONTROL_DC_SLEW,
	/**< 0x18[7]: DC Slew Control. */
	MAX97236_BIT_STATE_FORCING_FORCE,
	/**< 0x19[5]: Load State Force Enable. */
	MAX97236_BIT_STATE_FORCING_STATE,
	/**< 0x19[4]: State Value. */
	MAX97236_BIT_AC_TEST_CONTRO_AC_REPEAT1,
	/**< 0x1A[5]: AC_Repeat. */
	MAX97236_BIT_AC_TEST_CONTRO_AC_REPEAT0,
	/**< 0x1A[4]: AC_Repeat. */
	MAX97236_BIT_AC_TEST_CONTRO_PULSE_WIDTH1,
	/**< 0x1A[3]: Pulse Width. */
	MAX97236_BIT_AC_TEST_CONTRO_PULSE_WIDTH0,
	/**< 0x1A[2]: Pulse Width. */
	MAX97236_BIT_AC_TEST_CONTRO_PULSE_AMP1,
	/**< 0x1A[1]: Pulse Amplitude. */
	MAX97236_BIT_AC_TEST_CONTRO_PULSE_AMP0,
	/**< 0x1A[0]: Pulse Amplitude. */
	MAX97236_BIT_ENABLE1_SHDN,
	/**< 0x1D[7]: Full Device Shutdown Control. */
	MAX97236_BIT_ENABLE1_RESET,
	/**< 0x1D[6]: RESET Jack Detection. */
	MAX97236_BIT_ENABLE1_MIC_BIAS,
	/**< 0x1D[4]: Microphone Bias Enable/Status. */
	MAX97236_BIT_ENABLE1_MIC_AMP,
	/**< 0x1D[3]: Microphone Amplifier Enable/Status. */
	MAX97236_BIT_ENABLE1_KS,
	/**< 0x1D[2]: Keyscan Enable/Status. */
	MAX97236_BIT_ENABLE2_LFTEN,
	/**< 0x1E[7]: Left Headphone Enable/Status. */
	MAX97236_BIT_ENABLE2_RGHEN,
	/**< 0x1E[6]: Right Headphone Enable/Status. */
	MAX97236_BIT_ENABLE2_VSEN,
	/**< 0x1E[5]: Volume Adjustment Slewing. */
	MAX97236_BIT_ENABLE2_ZDEN,
	/**< 0x1E[4]: Zero-Crossing Detection. */
	MAX97236_BIT_ENABLE2_FAST,
	/**< 0x1E[3]: Jack Insertion Polling Speed. */
	MAX97236_BIT_ENABLE2_THRH,
	/**< 0x1E[2]: Class H Threshold Select. */
	MAX97236_BIT_ENABLE2_AUTO,
	/**< 0x1E[1]: Automatic Mode Select. */
	MAX97236_BIT_ID_MAX
};

/*!
  @brief MAX97236 STATUS1[JKIN] Value: Jack Detected.
*/
enum MAX97236_BIT_STATUS1_JKIN_VAL {
	MAX97236_BIT_STATUS1_JKIN_NO_LOAD = 0,
	/**< No load at TIP. */
	MAX97236_BIT_STATUS1_JKIN_LOAD = 1
	/**< Load detected at TIP. */
};

/*!
  @brief MAX97236 STATUS1[DDONE] Value: Jack Configuration Detect Done.
*/
enum MAX97236_BIT_STATUS1_DDONE_VAL {
	MAX97236_BIT_STATUS1_DDONE_NOT_COMPLETE = 0,
	/**< Jack detect algorithm is not complete. */
	MAX97236_BIT_STATUS1_DDONE_COMPLETE = 1
	/**< Jack detection algorithm is complete. */
};

/*!
  @brief MAX97236 STATUS1[VOL] Value: Volume Slew Complete.
*/
enum MAX97236_BIT_STATUS1_VOL_VAL {
	MAX97236_BIT_STATUS1_VOL_NO_VOLUME = 0,
	/**< No volume slewing sequences have completed
	     since any register was last read. */
	MAX97236_BIT_STATUS1_VOL_VOLUME = 1
	/**< Volume slewing complete. */
};

/*!
  @brief MAX97236 STATUS1[MIC_IN] Value: Microphone Connected/Disconnected.
*/
enum MAX97236_BIT_STATUS1_MIC_IN_VAL {
	MAX97236_BIT_STATUS1_MIC_IN_IN_REMOVED = 0,
	/**< Microphone is removed. */
	MAX97236_BIT_STATUS1_MIC_IN_IN_CONNECTED = 1
	/**< Microphone is connected. */
};

/*!
  @brief MAX97236 STATUS1[JACKSW] Value: JACKSW Status.
*/
enum MAX97236_BIT_STATUS1_JACKSW_VAL {
	MAX97236_BIT_STATUS1_JACKSW_NO_JACK = 0,
	/**< Mechanical jack switch reports no jack is connected. */
	MAX97236_BIT_STATUS1_JACKSW_JACK = 1
	/**< Mechanical jack switch shows that the jack is connected. */
};

/*!
  @brief MAX97236 STATUS1[MCSW] Value: Microphone Switch Status.
*/
enum MAX97236_BIT_STATUS1_MCSW_VAL {
	MAX97236_BIT_STATUS1_MCSW_NO_CHANGE = 0,
	/**< No change in microphone bias, no switch press. */
	MAX97236_BIT_STATUS1_MCSW_CHANGE = 1
	/**< Microphone bias has been pulled to ground
	     and debounced since the last status read. */
};

/*!
  @brief MAX97236 STATUS1[MBH] Value: Multibutton Headset Status.
*/
enum MAX97236_BIT_STATUS1_MBH_VAL {
	MAX97236_BIT_STATUS1_MBH_NO_ACTIVE = 0,
	/**< No active keypress detected. */
	MAX97236_BIT_STATUS1_MBH_ACTIVE = 1
	/**< Active keypress has been detected. */
};

/*!
  @brief MAX97236 STATUS2[LINE_L] Value: Line-Level Load on TIP Detected.
*/
enum MAX97236_BIT_STATUS2_LINE_L_VAL {
	MAX97236_BIT_STATUS2_LINE_L_NOT_DETECTED = 0,
	/**< Line-level load on TIP not detected. */
	MAX97236_BIT_STATUS2_LINE_L_DETECTED = 1
	/**< Line-level load on TIP detected. */
};

/*!
  @brief MAX97236 STATUS2[LINE_R] Value: Line-Level Load on ROUT Detected.
*/
enum MAX97236_BIT_STATUS2_LINE_R_VAL {
	MAX97236_BIT_STATUS2_LINE_R_NOT_DETECTED = 0,
	/**< Line-level load on RING1 not detected. */
	MAX97236_BIT_STATUS2_LINE_R_DETECTED = 1
	/**< Line-level load on RING1 detected. */
};

/*!
  @brief MAX97236 STATUS2[HP_L] Value: Headphone Load on TIP Detected.
*/
enum MAX97236_BIT_STATUS2_HP_L_VAL {
	MAX97236_BIT_STATUS2_HP_L_NOT_DETECTED  = 0,
	/**< Headphone load on TIP not detected. */
	MAX97236_BIT_STATUS2_HP_L_DETECTED = 1
	/**< Headphone load on TIP detected. */
};

/*!
  @brief MAX97236 STATUS2[HP_R] Value: Headphone Load on RING1 Detected.
*/
enum MAX97236_BIT_STATUS2_HP_R_VAL {
	MAX97236_BIT_STATUS2_HP_R_NOT_DETECTED  = 0,
	/**< Headphone load on RING1 not detected. */
	MAX97236_BIT_STATUS2_HP_R_DETECTED = 1
	/**< Headphone load on RING1 detected. */
};

/*!
  @brief MAX97236 STATUS2[JACKSWINC] Value: JACKSW Incorrect.
*/
enum MAX97236_BIT_STATUS2_JACKSWINC_VAL {
	MAX97236_BIT_STATUS2_JACKSWINC_CORRECT = 0,
	/**< JACKSW reporting is correct and correlates with JKIN. */
	MAX97236_BIT_STATUS2_JACKSWINC_NOT_CORRECT = 1
	/**< JACKSW reporting is not correct
	     and does not correlate with JKIN. */
};

/*!
  @brief MAX97236 STATUS2[KEY] Value: Passive Multibutton Headset KEY Status.
*/
enum MAX97236_BIT_STATUS2_KEY_VAL {
	MAX97236_BIT_STATUS2_KEY_NOT_PRESSED = 0,
	/**< No button pressed. */
	MAX97236_BIT_STATUS2_KEY_PRESSED = 1
	/**< Button has been pressed/released. */
};

/*!
  @brief MAX97236 STATUS3[GND] Value: Jack Common Location Identifier.
*/
enum MAX97236_BIT_STATUS3_GND_VAL {
	MAX97236_BIT_STATUS3_GND_GND_RING2 = 0x00,
	/**< The common jack connection is RING2. */
	MAX97236_BIT_STATUS3_GND_GND_SLEEVE = 0x01,
	/**< The common jack connection is SLEEVE. */
	MAX97236_BIT_STATUS3_GND_GND_RING2_SLEEVE = 0x02
	/**< Common on both RING2 and SLEEVE. */
};

/*!
  @brief MAX97236 IRQ_MASK1[IJKIN] Value: Jack Detect Interrupt Enable.
*/
enum MAX97236_BIT_IRQ_MASK1_IJKIN_VAL {
	MAX97236_BIT_IRQ_MASK1_IJKIN_DISABLED = 0,
	/**< Disabled. */
	MAX97236_BIT_IRQ_MASK1_IJKIN_ENABLED = 1
	/**< Enabled. */
};

/*!
  @brief MAX97236 IRQ_MASK1[DDONE] Value:
	Jack Configuration Detect Done Interrupt Enable.
*/
enum MAX97236_BIT_IRQ_MASK1_DDONE_VAL {
	MAX97236_BIT_IRQ_MASK1_DDONE_DISABLED = 0,
	/**< Disabled. */
	MAX97236_BIT_IRQ_MASK1_DDONE_ENABLED = 1
	/**< Enabled. */
};

/*!
  @brief MAX97236 IRQ_MASK1[IVOL] Value: Volume Slew Interrupt Enable.
*/
enum MAX97236_BIT_IRQ_MASK1_IVOL_VAL {
	MAX97236_BIT_IRQ_MASK1_IVOL_DISABLED = 0,
	/**< Disabled. */
	MAX97236_BIT_IRQ_MASK1_IVOL_ENABLED = 1
	/**< Enabled. */
};

/*!
  @brief MAX97236 IRQ_MASK1[IMIC] Value: Microphone Interrupt Enable.
*/
enum MAX97236_BIT_IRQ_MASK1_IMIC_VAL {
	MAX97236_BIT_IRQ_MASK1_IMIC_DISABLED = 0,
	/**< Disabled. */
	MAX97236_BIT_IRQ_MASK1_IMIC_ENABLED = 1
	/**< Enabled. */
};

/*!
  @brief MAX97236 IRQ_MASK1[ACKSW] Value: JACKSW Status Interrupt Enable.
*/
enum MAX97236_BIT_IRQ_MASK1_ACKSW_VAL {
	MAX97236_BIT_IRQ_MASK1_ACKSW_DISABLED = 0,
	/**< Disabled. */
	MAX97236_BIT_IRQ_MASK1_ACKSW_ENABLED = 1
	/**< Enabled */
};

/*!
  @brief MAX97236 IRQ_MASK1[IMCSW] Value: Microphone Switch Interrupt Enable.
*/
enum MAX97236_BIT_IRQ_MASK1_IMCSW_VAL {
	MAX97236_BIT_IRQ_MASK1_IMCSW_DISABLED = 0,
	/**< Disabled. */
	MAX97236_BIT_IRQ_MASK1_IMCSW_ENABLED = 1
	/**< Enabled. */
};

/*!
  @brief MAX97236 IRQ_MASK1[IMBH] Value:
	Multibutton Release Status Interrupt Enable.
*/
enum MAX97236_BIT_IRQ_MASK1_IMBH_VAL {
	MAX97236_BIT_IRQ_MASK1_IMBH_DISABLED = 0,
	/**< Disabled. */
	MAX97236_BIT_IRQ_MASK1_IMBH_ENABLED = 1
	/**< Enabled. */
};

/*!
  @brief MAX97236 IRQ_MASK2[ILINE_L] Value:
	Line-Level Load TIP Interrupt Enable.
*/
enum MAX97236_BIT_IRQ_MASK2_ILINE_L_VAL {
	MAX97236_BIT_IRQ_MASK2_ILINE_L_DISABLED = 0,
	/**< Disabled. */
	MAX97236_BIT_IRQ_MASK2_ILINE_L_ENABLED = 1
	/**< Enabled. */
};

/*!
  @brief MAX97236 IRQ_MASK2[ILINE_R] Value:
	Line-Level Load RING1 Interrupt Enable.
*/
enum MAX97236_BIT_IRQ_MASK2_ILINE_R_VAL {
	MAX97236_BIT_IRQ_MASK2_ILINE_R_DISABLED = 0,
	/**< Disabled. */
	MAX97236_BIT_IRQ_MASK2_ILINE_R_ENABLED = 1
	/**< Enabled. */
};

/*!
  @brief MAX97236 IRQ_MASK2[IHP_L] Value: Headphone Load TIP Interrupt Enable.
*/
enum MAX97236_BIT_IRQ_MASK2_IHP_L_VAL {
	MAX97236_BIT_IRQ_MASK2_IHP_L_DISABLED = 0,
	/**< Disabled. */
	MAX97236_BIT_IRQ_MASK2_IHP_L_ENABLED = 1
	/**< Enabled. */
};

/*!
  @brief MAX97236 IRQ_MASK2[IHP_R] Value: Headphone Load RING1 Interrupt Enable.
*/
enum MAX97236_BIT_IRQ_MASK2_IHP_R_VAL {
	MAX97236_BIT_IRQ_MASK2_IHP_R_DISABLED = 0,
	/**< Disabled. */
	MAX97236_BIT_IRQ_MASK2_IHP_R_ENABLED = 1
	/**< Enabled. */
};

/*!
  @brief MAX97236 IRQ_MASK2[IJACKSW] Value: JACKSW Incorrect Interrupt Enable.
*/
enum MAX97236_BIT_IRQ_MASK2_IJACKSW_VAL {
	MAX97236_BIT_IRQ_MASK2_IJACKSW_DISABLED = 0,
	/**< Disabled. */
	MAX97236_BIT_IRQ_MASK2_IJACKSW_ENABLED = 1
	/**< Enabled. */
};

/*!
  @brief MAX97236 IRQ_MASK2[IKEY] Value: KEY Interrupt Enable.
*/
enum MAX97236_BIT_IRQ_MASK2_IKEY_VAL {
	MAX97236_BIT_IRQ_MASK2_IKEY_DISABLED = 0,
	/**< Disabled. */
	MAX97236_BIT_IRQ_MASK2_IKEY_ENABLED = 1
	/**< Enabled. */
};

/*!
  @brief MAX97236 LEFT_VOLUME[L_EQUAL_R] Value: Left/Right Tracking.
*/
enum MAX97236_BIT_LEFT_VOLUME_L_EQUAL_R_VAL {
	MAX97236_BIT_LEFT_VOLUME_L_EQUAL_R_LR_LRVOL = 0,
	/**< The right-channel volume control is independent of the left. */
	MAX97236_BIT_LEFT_VOLUME_L_EQUAL_R_LR_LVOL = 1
	/**< Control both volume controls bywriting to LVOL. */
};

/*!
  @brief MAX97236 LEFT_VOLUME[MUTEL] Value: Headphone Mute.
*/
enum MAX97236_BIT_LEFT_VOLUME_MUTEL_VAL {
	MAX97236_BIT_LEFT_VOLUME_MUTEL_MUTEL_DISABLE = 0,
	/**< Disable. */
	MAX97236_BIT_LEFT_VOLUME_MUTEL_MUTEL_ENABLE = 1
	/**< Enable, output is muted. */
};

/*!
  @brief MAX97236 LEFT_VOLUME[LVOL] Value: Left Headphone Output Volume Level.
*/
enum MAX97236_BIT_LEFT_VOLUME_LVOL_VAL {
	MAX97236_BIT_LEFT_VOLUME_LVOL_M60DB = 0x00,
	/**< -60dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M58DB = 0x01,
	/**< -58dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M56DB = 0x02,
	/**< -56dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M54DB = 0x03,
	/**< -54dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M53DB = 0x04,
	/**< -53dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M52DB = 0x05,
	/**< -52dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M51DB = 0x06,
	/**< -51dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M50DB = 0x07,
	/**< -50dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M49DB = 0x08,
	/**< -49dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M48DB = 0x09,
	/**< -48dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M47DB = 0x0A,
	/**< -47dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M46DB = 0x0B,
	/**< -46dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M45DB = 0x0C,
	/**< -45dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M44DB = 0x0D,
	/**< -44dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M43DB = 0x0E,
	/**< -43dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M42DB = 0x0F,
	/**< -42dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M41DB = 0x10,
	/**< -41dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M40DB = 0x11,
	/**< -40dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M39DB = 0x12,
	/**< -39dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M38DB = 0x13,
	/**< -38dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M37DB = 0x14,
	/**< -37dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M36DB = 0x15,
	/**< -36dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M35DB = 0x16,
	/**< -35dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M34DB = 0x17,
	/**< -34dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M33DB = 0x18,
	/**< -33dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M32DB = 0x19,
	/**< -32dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M31DB = 0x1A,
	/**< -31dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M30DB = 0x1B,
	/**< -30dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M29DB = 0x1C,
	/**< -29dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M28DB = 0x1D,
	/**< -28dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M27DB = 0x1E,
	/**< -27dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M26DB = 0x1F,
	/**< -26dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M25DB = 0x20,
	/**< -25dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M24DB = 0x21,
	/**< -24dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M23DB = 0x22,
	/**< -23dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M22DB = 0x23,
	/**< -22dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M21DB = 0x24,
	/**< -21dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M20DB = 0x25,
	/**< -20dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M19DB = 0x26,
	/**< -19dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M18DB = 0x27,
	/**< -18dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M17DB = 0x28,
	/**< -17dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M16DB = 0x29,
	/**< -16dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M15DB = 0x2A,
	/**< -15dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M14DB = 0x2B,
	/**< -14dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M13DB = 0x2C,
	/**< -13dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M12DB = 0x2D,
	/**< -12dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M11DB = 0x2E,
	/**< -11dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M10DB = 0x2F,
	/**< -10dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M9DB = 0x30,
	/**< -9dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M8DB = 0x31,
	/**< -8dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M7DB = 0x32,
	/**< -7dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M6DB = 0x33,
	/**< -6dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M5DB = 0x34,
	/**< -5dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M4DB = 0x35,
	/**< -4dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M3DB = 0x36,
	/**< -3dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M2DB = 0x37,
	/**< -2dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_M1DB = 0x38,
	/**< -1dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_0DB = 0x39,
	/**< 0dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_1DB = 0x3A,
	/**< 1dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_2DB = 0x3B,
	/**< 2dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_3DB = 0x3C,
	/**< 3dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_4DB = 0x3D,
	/**< 4dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_5DB = 0x3E,
	/**< 5dB */
	MAX97236_BIT_LEFT_VOLUME_LVOL_6DB = 0x3F
	/**< 6dB */
};
/*!
  @brief MAX97236 RIGHT_VOLUME[MUTER] Value: Headphone Mute.
*/
enum MAX97236_BIT_RIGHT_VOLUME_MUTER_VAL {
	MAX97236_BIT_RIGHT_VOLUME_MUTER_DISABLE = 0,
	/**< Disable. */
	MAX97236_BIT_RIGHT_VOLUME_MUTER_ENABLE = 1
	/**< Enable, output is muted. */
};

/*!
  @brief MAX97236 RIGHT_VOLUME[RVOL] Value:
	Left/Right Headphone Output Volume Level.
*/
enum MAX97236_BIT_RIGHT_VOLUME_RVOL_VAL {
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M60DB = 0x00,
	/**< -60dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M58DB = 0x01,
	/**< -58dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M56DB = 0x02,
	/**< -56dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M54DB = 0x03,
	/**< -54dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M53DB = 0x04,
	/**< -53dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M52DB = 0x05,
	/**< -52dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M51DB = 0x06,
	/**< -51dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M50DB = 0x07,
	/**< -50dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M49DB = 0x08,
	/**< -49dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M48DB = 0x09,
	/**< -48dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M47DB = 0x0A,
	/**< -47dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M46DB = 0x0B,
	/**< -46dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M45DB = 0x0C,
	/**< -45dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M44DB = 0x0D,
	/**< -44dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M43DB = 0x0E,
	/**< -43dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M42DB = 0x0F,
	/**< -42dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M41DB = 0x10,
	/**< -41dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M40DB = 0x11,
	/**< -40dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M39DB = 0x12,
	/**< -39dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M38DB = 0x13,
	/**< -38dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M37DB = 0x14,
	/**< -37dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M36DB = 0x15,
	/**< -36dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M35DB = 0x16,
	/**< -35dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M34DB = 0x17,
	/**< -34dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M33DB = 0x18,
	/**< -33dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M32DB = 0x19,
	/**< -32dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M31DB = 0x1A,
	/**< -31dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M30DB = 0x1B,
	/**< -30dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M29DB = 0x1C,
	/**< -29dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M28DB = 0x1D,
	/**< -28dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M27DB = 0x1E,
	/**< -27dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M26DB = 0x1F,
	/**< -26dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M25DB = 0x20,
	/**< -25dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M24DB = 0x21,
	/**< -24dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M23DB = 0x22,
	/**< -23dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M22DB = 0x23,
	/**< -22dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M21DB = 0x24,
	/**< -21dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M20DB = 0x25,
	/**< -20dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M19DB = 0x26,
	/**< -19dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M18DB = 0x27,
	/**< -18dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M17DB = 0x28,
	/**< -17dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M16DB = 0x29,
	/**< -16dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M15DB = 0x2A,
	/**< -15dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M14DB = 0x2B,
	/**< -14dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M13DB = 0x2C,
	/**< -13dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M12DB = 0x2D,
	/**< -12dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M11DB = 0x2E,
	/**< -11dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M10DB = 0x2F,
	/**< -10dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M9DB = 0x30,
	/**< -9dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M8DB = 0x31,
	/**< -8dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M7DB = 0x32,
	/**< -7dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M6DB = 0x33,
	/**< -6dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M5DB = 0x34,
	/**< -5dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M4DB = 0x35,
	/**< -4dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M3DB = 0x36,
	/**< -3dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M2DB = 0x37,
	/**< -2dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_M1DB = 0x38,
	/**< -1dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_0DB = 0x39,
	/**< 0dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_1DB = 0x3A,
	/**< 1dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_2DB = 0x3B,
	/**< 2dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_3DB = 0x3C,
	/**< 3dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_4DB = 0x3D,
	/**< 4dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_5DB = 0x3E,
	/**< 5dB */
	MAX97236_BIT_RIGHT_VOLUME_RVOL_6DB = 0x3F
	/**< 6dB */
};

/*!
  @brief MAX97236 MICROPHONE[GAIN] Value: Microphone Preamplifier Gain Select.
*/
enum MAX97236_BIT_MICROPHONE_GAIN_VAL {
	MAX97236_BIT_MICROPHONE_GAIN_12DB = 0,
	/**< 12dB */
	MAX97236_BIT_MICROPHONE_GAIN_24DB = 1
	/**< 24dB */
};

/*!
  @brief MAX97236 MICROPHONE[MICR] Value: Microphone Bias Resistor Select.
*/
enum MAX97236_BIT_MICROPHONE_MICR_VAL {
	MAX97236_BIT_MICROPHONE_MICR_2_2KI = 0x00,
	/**< 2.2kI */
	MAX97236_BIT_MICROPHONE_MICR_2_6KI = 0x01,
	/**< 2.6kI */
	MAX97236_BIT_MICROPHONE_MICR_3_0KI = 0x02,
	/**< 3.0kI */
	MAX97236_BIT_MICROPHONE_MICR_BYPASSED = 0x03,
	/**< Bypassed */
	MAX97236_BIT_MICROPHONE_MICR_HIGH = 0x04
	/**< High impedance (sleep mode) */
};

/*!
  @brief MAX97236 MICROPHONE[BIAS] Value: Microphone Bias Voltage Select.
*/
enum MAX97236_BIT_MICROPHONE_BIAS_VAL {
	MAX97236_BIT_MICROPHONE_BIAS_2_0V = 0,
	/**< 2.0V */
	MAX97236_BIT_MICROPHONE_BIAS_2_6V = 1
	/**< 2.6V */
};

/*!
  @brief MAX97236 PASSIVE_MBH_KEYSCAN_DATA[PRESS] Value: Release.
*/
enum MAX97236_BIT_PASSIVE_MBH_KEYSCAN_DATA_PRESS_VAL {
	MAX97236_BIT_PASSIVE_MBH_KEYSCAN_DATA_PRESS_KEY_RELEASE = 0,
	/**< Key release. */
	MAX97236_BIT_PASSIVE_MBH_KEYSCAN_DATA_PRESS_KEYPRESS = 1
	/**< Keypress. */
};

/*!
  @brief MAX97236 PASSIVE_MBH_KEYSCAN_DATA[RANGE] Value:
*/
enum MAX97236_BIT_PASSIVE_MBH_KEYSCAN_DATA_RANGE_VAL {
	MAX97236_BIT_PASSIVE_MBH_KEYSCAN_DATA_RANGE_COARSE = 0,
	/**< Coarse range. */
	MAX97236_BIT_PASSIVE_MBH_KEYSCAN_DATA_RANGE_FINE = 1
	/**< Fine range. */
};

/*!
  @brief MAX97236 STATE_FORCING[FORCE] Value: Load State Force Enable.
*/
enum MAX97236_BIT_STATE_FORCING_FORCE_VAL {
	MAX97236_BIT_STATE_FORCING_FORCE_FORCES = 0,
	/**< Forces the IC into a configuration defined by 0b4-0b0. */
	MAX97236_BIT_STATE_FORCING_FORCE_STATE = 1
	/**< State forcing disabled. */
};

/*!
  @brief MAX97236 STATE_FORCING[STATE] Value: State Value.
*/
enum MAX97236_BIT_STATE_FORCING_STATE_VAL {
	MAX97236_BIT_STATE_FORCING_STATE_FFFF = 0x01,
	/**< FFFF. */
	MAX97236_BIT_STATE_FORCING_STATE_LRGM = 0x02,
	/**< LRGM. */
	MAX97236_BIT_STATE_FORCING_STATE_LRMG = 0x03,
	/**< LRMG. */
	MAX97236_BIT_STATE_FORCING_STATE_LRGG_AC = 0x07,
	/**< LRGG_AC. */
	MAX97236_BIT_STATE_FORCING_STATE_LRGF = 0x08,
	/**< LRGF. */
	MAX97236_BIT_STATE_FORCING_STATE_LFGF = 0x09,
	/**< LFGF. */
	MAX97236_BIT_STATE_FORCING_STATE_FRGF = 0x0A,
	/**< FRGF. */
	MAX97236_BIT_STATE_FORCING_STATE_LGGM = 0x0C,
	/**< LGGM. */
	MAX97236_BIT_STATE_FORCING_STATE_LGMG = 0x0D,
	/**< LGMG. */
	MAX97236_BIT_STATE_FORCING_STATE_LLGM = 0x0E,
	/**< LLGM. */
	MAX97236_BIT_STATE_FORCING_STATE_LLMG = 0x0F,
	/**< LLMG. */
	MAX97236_BIT_STATE_FORCING_STATE_LFGM = 0x10,
	/**< LFGM. */
	MAX97236_BIT_STATE_FORCING_STATE_LFMG = 0x11,
	/**< LFMG. */
	MAX97236_BIT_STATE_FORCING_STATE_LRGG_DC = 0x12,
	/**< LRGG_DC. */
	MAX97236_BIT_STATE_FORCING_STATE_LFGG = 0x13,
	/**< LFGG. */
	MAX97236_BIT_STATE_FORCING_STATE_FRGG = 0x14
	/**< FRGG. */
};

/*!
  @brief MAX97236 AC_TEST_CONTRO[AC_REPEAT1] Value: AC_Repeat.
*/
enum MAX97236_BIT_AC_TEST_CONTRO_AC_REPEAT1_VAL {
	MAX97236_BIT_AC_TEST_CONTRO_AC_REPEAT1_1 = 0x00,
	/**< 1 */
	MAX97236_BIT_AC_TEST_CONTRO_AC_REPEAT1_3 = 0x01,
	/**< 3 */
	MAX97236_BIT_AC_TEST_CONTRO_AC_REPEAT1_5 = 0x02,
	/**< 5 */
	MAX97236_BIT_AC_TEST_CONTRO_AC_REPEAT1_7 = 0x03
	/**< 7 */
};

/*!
  @brief MAX97236 AC_TEST_CONTRO[AC_REPEAT0] Value: AC_Repeat.
*/
enum MAX97236_BIT_AC_TEST_CONTRO_AC_REPEAT0_VAL {
	MAX97236_BIT_AC_TEST_CONTRO_AC_REPEAT0_1 = 0x00,
	/**< 1 */
	MAX97236_BIT_AC_TEST_CONTRO_AC_REPEAT0_3 = 0x01,
	/**< 3 */
	MAX97236_BIT_AC_TEST_CONTRO_AC_REPEAT0_5 = 0x02,
	/**< 5 */
	MAX97236_BIT_AC_TEST_CONTRO_AC_REPEAT0_7 = 0x03
	/**< 7 */
};

/*!
  @brief MAX97236 AC_TEST_CONTRO[PULSE_WIDTH1] Value: Pulse Width.
*/
enum MAX97236_BIT_AC_TEST_CONTRO_PULSE_WIDTH1_VAL {
	MAX97236_BIT_AC_TEST_CONTRO_PULSE_WIDTH1_50US  = 0x00,
	/**< 50us */
	MAX97236_BIT_AC_TEST_CONTRO_PULSE_WIDTH1_100US = 0x01,
	/**< 100us */
	MAX97236_BIT_AC_TEST_CONTRO_PULSE_WIDTH1_150US = 0x02,
	/**< 150us */
	MAX97236_BIT_AC_TEST_CONTRO_PULSE_WIDTH1_300US = 0x03
	/**< 300us */
};

/*!
  @brief MAX97236 AC_TEST_CONTRO[PULSE_WIDTH0] Value: Pulse Width.
*/
enum MAX97236_BIT_AC_TEST_CONTRO_PULSE_WIDTH0_VAL {
	MAX97236_BIT_AC_TEST_CONTRO_PULSE_WIDTH0_50US  = 0x00,
	/**< 50us */
	MAX97236_BIT_AC_TEST_CONTRO_PULSE_WIDTH0_100US = 0x01,
	/**< 100us */
	MAX97236_BIT_AC_TEST_CONTRO_PULSE_WIDTH0_150US = 0x02,
	/**< 150us */
	MAX97236_BIT_AC_TEST_CONTRO_PULSE_WIDTH0_300US = 0x03
	/**< 300us */
};

/*!
  @brief MAX97236 AC_TEST_CONTRO[PULSE_AMP1] Value: Pulse Amplitude.
*/
enum MAX97236_BIT_AC_TEST_CONTRO_PULSE_AMP1_VAL {
	MAX97236_BIT_AC_TEST_CONTRO_PULSE_AMP1_1 = 0x00,
	/**< For tests 1 and 4, pulse amplitude is 25mV.
	     Tests 6, 7, and 8 pulse amplitude is 37mV. */
	MAX97236_BIT_AC_TEST_CONTRO_PULSE_AMP1_2 = 0x01,
	/**< For tests 1 and 4, pulse amplitude is 50mV.
	     Tests 6, 7, and 8 pulse amplitude is 75mV. */
	MAX97236_BIT_AC_TEST_CONTRO_PULSE_AMP1_3 = 0x02,
	/**< For tests 1 and 4, pulse amplitude is 100mV.
	     Tests 6, 7, and 8 pulse amplitude is 150mV. */
	MAX97236_BIT_AC_TEST_CONTRO_PULSE_AMP1_4 = 0x03
	/**< For tests 1 and 4, pulse amplitude is 200mV.
	     Tests 6, 7, and 8 pulse amplitude is 220mV. */
};

/*!
  @brief MAX97236 AC_TEST_CONTRO[PULSE_AMP0] Value: Pulse Amplitude.
*/
enum MAX97236_BIT_AC_TEST_CONTRO_PULSE_AMP0_VAL {
	MAX97236_BIT_AC_TEST_CONTRO_PULSE_AMP0_1 = 0x00,
	/**< For tests 1 and 4, pulse amplitude is 25mV.
	     Tests 6, 7, and 8 pulse amplitude is 37mV. */
	MAX97236_BIT_AC_TEST_CONTRO_PULSE_AMP0_2 = 0x01,
	/**< For tests 1 and 4, pulse amplitude is 50mV.
	     Tests 6, 7, and 8 pulse amplitude is 75mV. */
	MAX97236_BIT_AC_TEST_CONTRO_PULSE_AMP0_3 = 0x02,
	/**< For tests 1 and 4, pulse amplitude is 100mV.
	     Tests 6, 7, and 8 pulse amplitude is 150mV. */
	MAX97236_BIT_AC_TEST_CONTRO_PULSE_AMP0_4 = 0x03
	/**< For tests 1 and 4, pulse amplitude is 200mV.
	     Tests 6, 7, and 8 pulse amplitude is 220mV. */
};

/*!
  @brief MAX97236 ENABLE1[SHDN] Value: Full Device Shutdown Control.
*/
enum MAX97236_BIT_ENABLE1_SHDN_VAL {
	MAX97236_BIT_ENABLE1_SHDN_CIRCUITRY_ACTIVE = 0,
	/**< The IC is in shutdown mode
	     with jack detection circuitry active. */
	MAX97236_BIT_ENABLE1_SHDN_ACTIVE = 1
	/**< The IC is active. */
};

/*!
  @brief MAX97236 ENABLE1[MIC_BIAS] Value: Microphone Bias Enable/Status.
*/
enum MAX97236_BIT_ENABLE1_MIC_BIAS_VAL {
	MAX97236_BIT_ENABLE1_MIC_BIAS_DISABLED = 0,
	/**< Microphone bias is disabled. */
	MAX97236_BIT_ENABLE1_MIC_BIAS_ENABLED = 1
	/**< Microphone bias is enabled. */
};

/*!
  @brief MAX97236 ENABLE1[MIC_AMP] Value: Microphone Amplifier Enable/Status.
*/
enum MAX97236_BIT_ENABLE1_MIC_AMP_VAL {
	MAX97236_BIT_ENABLE1_MIC_AMP_DISABLED = 0,
	/**< Microphone amplifiers are disabled. */
	MAX97236_BIT_ENABLE1_MIC_AMP_ENABLED = 1
	/**< Microphone amplifiers are enabled. */
};

/*!
  @brief MAX97236 ENABLE1[KS] Value: Keyscan Enable/Status.
*/
enum MAX97236_BIT_ENABLE1_KS_VAL {
	MAX97236_BIT_ENABLE1_KS_DISABLED = 0,
	/**< Keyscan ADC is disabled. */
	MAX97236_BIT_ENABLE1_KS_ENABLED = 1
	/**< Keyscan ADC is enabled. */
};

/*!
  @brief MAX97236 ENABLE2[LFTEN] Value: Left Headphone Enable/Status.
*/
enum MAX97236_BIT_ENABLE2_LFTEN_VAL {
	MAX97236_BIT_ENABLE2_LFTEN_DISABLED = 0,
	/**< Headphone amplifier left channel is disabled. */
	MAX97236_BIT_ENABLE2_LFTEN_ENABLED = 1
	/**< Headphone amplifier left channel is enabled. */
};

/*!
  @brief MAX97236 ENABLE2[RGHEN] Value: Right Headphone Enable/Status.
*/
enum MAX97236_BIT_ENABLE2_RGHEN_VAL {
	MAX97236_BIT_ENABLE2_RGHEN_DISABLED = 0,
	/**< Headphone amplifier right channel is disabled. */
	MAX97236_BIT_ENABLE2_RGHEN_ENABLED = 1
	/**< Headphone amplifier right channel is enabled. */
};

/*!
  @brief MAX97236 ENABLE2[VSEN] Value: Volume Adjustment Slewing.
*/
enum MAX97236_BIT_ENABLE2_VSEN_VAL {
	MAX97236_BIT_ENABLE2_VSEN_ENABLED = 0,
	/**< Enabled. */
	MAX97236_BIT_ENABLE2_VSEN_DISABLED = 1
	/**< Disabled. */
};

/*!
  @brief MAX97236 ENABLE2[ZDEN] Value: Zero-Crossing Detection.
*/
enum MAX97236_BIT_ENABLE2_ZDEN_VAL {
	MAX97236_BIT_ENABLE2_ZDEN_ENABLED = 0,
	/**< Enabled. */
	MAX97236_BIT_ENABLE2_ZDEN_DISABLED = 1
	/**< Disabled. */
};

/*!
  @brief MAX97236 ENABLE2[FAST] Value: Jack Insertion Polling Speed.
*/
enum MAX97236_BIT_ENABLE2_FAST_VAL {
	MAX97236_BIT_ENABLE2_FAST_SLOW = 0,
	/**< Slow polling mode, 2s delay between polls. */
	MAX97236_BIT_ENABLE2_FAST_FAST = 1
	/**< Fast polling mode, 333ms delay between polls. */
};

/*!
  @brief MAX97236 ENABLE2[THRH] Value: Class H Threshold Select.
*/
enum MAX97236_BIT_ENABLE2_THRH_VAL {
	MAX97236_BIT_ENABLE2_THRH_LOW = 0,
	/**< Low threshold. */
	MAX97236_BIT_ENABLE2_THRH_HIGH = 1
	/**< High threshold. */
};

/*!
  @brief MAX97236 ENABLE2[AUTO] Value: Automatic Mode Select.
*/
enum MAX97236_BIT_ENABLE2_AUTO_VAL {
	MAX97236_BIT_ENABLE2_AUTO_MODE_1 = 0x00,
	/**< Auto Mode1. */
	MAX97236_BIT_ENABLE2_AUTO_MODE_2 = 0x01,
	/**< Auto Mode2. */
	MAX97236_BIT_ENABLE2_AUTO_MODE_3 = 0x02
	/**< Auto Mode3. */
};

/*---------------------------------------------------------------------------*/
/* structure declaration                                                     */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* global variable declaration                                               */
/*---------------------------------------------------------------------------*/
/*!
  @brief MAX97236 register address table.
*/
const int MAX97236_reg_addr[MAX97236_REG_ID_MAX] = {
	0x00,
	/**< MAX97236_REG_OR_STATUS1 */
	0x01,
	/**< MAX97236_REG_OR_STATUS2 */
	0x02,
	/**< MAX97236_REG_OR_STATUS3 */
	0x04,
	/**< MAX97236_REG_RW_IRQ_MASK1 */
	0x05,
	/**< MAX97236_REG_RW_IRQ_MASK2 */
	0x07,
	/**< MAX97236_REG_RW_LEFT_VOLUME */
	0x08,
	/**< MAX97236_REG_RW_RIGHT_VOLUME */
	0x09,
	/**< MAX97236_REG_RW_MICROPHONE */
	0x0B,
	/**< MAX97236_REG_OR_VENDOR_ID_REGISTER */
	0x12,
	/**< MAX97236_REG_RW_KEYSCAN_CLOCK_DIVIDER_1 */
	0x13,
	/**< MAX97236_REG_RW_KEYSCAN_CLOCK_DIVIDER_2 */
	0x14,
	/**< MAX97236_REG_RW_KEYSCAN_CLOCK_DIVIDER_ADC */
	0x15,
	/**< MAX97236_REG_RW_KEYSCAN_DEBOUNCE */
	0x16,
	/**< MAX97236_REG_RW_KEYSCAN_DELAY */
	0x17,
	/**< MAX97236_REG_OR_PASSIVE_MBH_KEYSCAN_DATA */
	0x18,
	/**< MAX97236_REG_RW_DC_TEST_SLEW_CONTROL */
	0x19,
	/**< MAX97236_REG_RW_STATE_FORCING */
	0x1A,
	/**< MAX97236_REG_RW_AC_TEST_CONTRO */
	0x1D,
	/**< MAX97236_REG_RW_ENABLE1 */
	0x1E,
	/**< MAX97236_REG_RW_ENABLE2 */
};

/*!
  @brief MAX97236 bit information table.
*/
const struct max98090_bit_info MAX97236_bit_info[MAX97236_BIT_ID_MAX] = {
	{ 7, BIT7                                       },
	/**< MAX97236_BIT_STATUS1_JKIN */
	{ 6, BIT6                                       },
	/**< MAX97236_BIT_STATUS1_DDONE */
	{ 5, BIT5                                       },
	/**< MAX97236_BIT_STATUS1_VOL */
	{ 3, BIT3                                       },
	/**< MAX97236_BIT_STATUS1_MIC_IN */
	{ 2, BIT2                                       },
	/**< MAX97236_BIT_STATUS1_JACKSW */
	{ 1, BIT1                                       },
	/**< MAX97236_BIT_STATUS1_MCSW */
	{ 0, BIT0                                       },
	/**< MAX97236_BIT_STATUS1_MBH */
	{ 7, BIT7                                       },
	/**< MAX97236_BIT_STATUS2_LINE_L */
	{ 6, BIT6                                       },
	/**< MAX97236_BIT_STATUS2_LINE_R */
	{ 5, BIT5                                       },
	/**< MAX97236_BIT_STATUS2_HP_L */
	{ 4, BIT4                                       },
	/**< MAX97236_BIT_STATUS2_HP_R */
	{ 3, BIT3                                       },
	/**< MAX97236_BIT_STATUS2_JACKSWINC */
	{ 2, BIT2                                       },
	/**< MAX97236_BIT_STATUS2_KEY */
	{ 0, (BIT1|BIT0)                                },
	/**< MAX97236_BIT_STATUS3_GND */
	{ 7, BIT7                                       },
	/**< MAX97236_BIT_IRQ_MASK1_IJKIN */
	{ 6, BIT6                                       },
	/**< MAX97236_BIT_IRQ_MASK1_DDONE */
	{ 5, BIT5                                       },
	/**< MAX97236_BIT_IRQ_MASK1_IVOL */
	{ 3, BIT3                                       },
	/**< MAX97236_BIT_IRQ_MASK1_IMIC */
	{ 2, BIT2                                       },
	/**< MAX97236_BIT_IRQ_MASK1_ACKSW */
	{ 1, BIT1                                       },
	/**< MAX97236_BIT_IRQ_MASK1_IMCSW */
	{ 0, BIT0                                       },
	/**< MAX97236_BIT_IRQ_MASK1_IMBH */
	{ 7, BIT7                                       },
	/**< MAX97236_BIT_IRQ_MASK2_ILINE_L */
	{ 6, BIT6                                       },
	/**< MAX97236_BIT_IRQ_MASK2_ILINE_R */
	{ 5, BIT5                                       },
	/**< MAX97236_BIT_IRQ_MASK2_IHP_L */
	{ 4, BIT4                                       },
	/**< MAX97236_BIT_IRQ_MASK2_IHP_R */
	{ 3, BIT3                                       },
	/**< MAX97236_BIT_IRQ_MASK2_IJACKSW */
	{ 2, BIT2                                       },
	/**< MAX97236_BIT_IRQ_MASK2_IKEY */
	{ 7, BIT7                                       },
	/**< MAX97236_BIT_LEFT_VOLUME_L_EQUAL_R */
	{ 6, BIT6                                       },
	/**< MAX97236_BIT_LEFT_VOLUME_MUTEL */
	{ 0, (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)            },
	/**< MAX97236_BIT_LEFT_VOLUME_LVOL */
	{ 6, BIT6                                       },
	/**< MAX97236_BIT_RIGHT_VOLUME_MUTER */
	{ 0, (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)            },
	/**< MAX97236_BIT_RIGHT_VOLUME_RVOL */
	{ 6, BIT6                                       },
	/**< MAX97236_BIT_MICROPHONE_GAIN */
	{ 3, (BIT5|BIT4|BIT3)                           },
	/**< MAX97236_BIT_MICROPHONE_MICR */
	{ 2, BIT2                                       },
	/**< MAX97236_BIT_MICROPHONE_BIAS */
	{ 4, (BIT7|BIT6|BIT5|BIT4)                      },
	/**< MAX97236_BIT_VENDOR_ID_REGISTER_ID */
	{ 0, (BIT7|BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)  },
	/**< MAX97236_BIT_KEYSCAN_CLOCK_DIVIDER_1_KEY_DIV_HIGH */
	{ 0, (BIT7|BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)  },
	/**< MAX97236_BIT_KEYSCAN_CLOCK_DIVIDER_2_KEY_DIV_LOW */
	{ 0, (BIT7|BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)  },
	/**< MAX97236_BIT_KEYSCAN_CLOCK_DIVIDER_ADC_KEY_DIV_ADC */
	{ 0, (BIT7|BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)  },
	/**< MAX97236_BIT_KEYSCAN_DEBOUNCE_KEY_DEB */
	{ 0, (BIT7|BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)  },
	/**< MAX97236_BIT_KEYSCAN_DELAY_KEY_DEL */
	{ 7, BIT7                                       },
	/**< MAX97236_BIT_PASSIVE_MBH_KEYSCAN_DATA_PRESS */
	{ 6, BIT6                                       },
	/**< MAX97236_BIT_PASSIVE_MBH_KEYSCAN_DATA_RANGE */
	{ 0, (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)            },
	/**< MAX97236_BIT_PASSIVE_MBH_KEYSCAN_DATA_KEYDATA */
	{ 0, (BIT7|BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)  },
	/**< MAX97236_BIT_DC_TEST_SLEW_CONTROL_DC_SLEW */
	{ 5, BIT5                                       },
	/**< MAX97236_BIT_STATE_FORCING_FORCE */
	{ 0, (BIT4|BIT3|BIT2|BIT1|BIT0)                 },
	/**< MAX97236_BIT_STATE_FORCING_STATE */
	{ 5, BIT5                                       },
	/**< MAX97236_BIT_AC_TEST_CONTRO_AC_REPEAT1 */
	{ 4, BIT4                                       },
	/**< MAX97236_BIT_AC_TEST_CONTRO_AC_REPEAT0 */
	{ 3, BIT3                                       },
	/**< MAX97236_BIT_AC_TEST_CONTRO_PULSE_WIDTH1 */
	{ 2, BIT2                                       },
	/**< MAX97236_BIT_AC_TEST_CONTRO_PULSE_WIDTH0 */
	{ 1, BIT1                                       },
	/**< MAX97236_BIT_AC_TEST_CONTRO_PULSE_AMP1 */
	{ 0, BIT0                                       },
	/**< MAX97236_BIT_AC_TEST_CONTRO_PULSE_AMP0 */
	{ 7, BIT7                                       },
	/**< MAX97236_BIT_ENABLE1_SHDN */
	{ 6, BIT6                                       },
	/**< MAX97236_BIT_ENABLE1_RESET */
	{ 4, BIT4                                       },
	/**< MAX97236_BIT_ENABLE1_MIC_BIAS */
	{ 3, BIT3                                       },
	/**< MAX97236_BIT_ENABLE1_MIC_AMP */
	{ 2, BIT2                                       },
	/**< MAX97236_BIT_ENABLE1_KS */
	{ 7, BIT7                                       },
	/**< MAX97236_BIT_ENABLE2_LFTEN */
	{ 6, BIT6                                       },
	/**< MAX97236_BIT_ENABLE2_RGHEN */
	{ 5, BIT5                                       },
	/**< MAX97236_BIT_ENABLE2_VSEN */
	{ 4, BIT4                                       },
	/**< MAX97236_BIT_ENABLE2_ZDEN */
	{ 3, BIT3                                       },
	/**< MAX97236_BIT_ENABLE2_FAST */
	{ 2, BIT2                                       },
	/**< MAX97236_BIT_ENABLE2_THRH */
	{ 0, (BIT1|BIT0)                                },
	/**< MAX97236_BIT_ENABLE2_AUTO */
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
/* prototype declaration                                                     */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* inline function implementation                                            */
/*---------------------------------------------------------------------------*/
/* none */

#endif  /* __MAX97236_DEFS_H__ */
