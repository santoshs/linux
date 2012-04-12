/* max98090_defs.h
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
  @file max98090_defs.h
  
  @brief max98090 header file.
  
  
  @author author Xxxx Xxxx
  
  @attention first version is a draft.
  
  @date 2012/03/15 first version.
*/

#ifndef __MAX98090_DEFS_H__
#define __MAX98090_DEFS_H__

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
  @brief MAX98090 Register ID value.
*/
enum MAX98090_REG_ID {
    MAX98090_REG_W_SOFTWARE_RESET,          //!< 0x00: Software Reset Register.
    MAX98090_REG_R_DEVICE_STATUS,           //!< 0x01: Device Status Interrupt Register.
    MAX98090_REG_R_JACK_DETECT,             //!< 0x02: Jack Status Interrupt Register.
    MAX98090_REG_RW_INTERRUPT_MASKS,        //!< 0x03: Device Status Interrupt Mask Register.
    MAX98090_REG_W_SYSTEM_CLOCK,            //!< 0x04: System Clock Quick Setup Register.
    MAX98090_REG_W_SAMPLE_RATE,             //!< 0x05: Sample Rate Quick Setup Register.
    MAX98090_REG_W_INTERFACE,               //!< 0x06: Digital Audio Interface (DAI) Quick Setup Register.
    MAX98090_REG_W_DAC_PATH,                //!< 0x07: Digital to Analog Converter (DAC) Path Quick Setup Register.
    MAX98090_REG_W_MIC_OR_DIRECT_TO_ADC,    //!< 0x08: Microphone / Direct to Analog to Digital Converter (ADC) Path Quick Setup Register.
    MAX98090_REG_W_LINE_TO_ADC,             //!< 0x09: Line Input to Analog to Digital Converter (ADC) Path Quick Setup Register.
    MAX98090_REG_W_ANALOG_MIC_LOOP,         //!< 0x0A: Analog Microphone Input to Analog Output Loop Quick Setup Register.
    MAX98090_REG_W_ANALOG_LINE_LOOP,        //!< 0x0B: Analog Line Input to Analog Output Loop Quick Setup Register.
    MAX98090_REG_RW_DIGITAL_MIC,            //!< 0x0C: Digital Microphone Configuration Register.
    MAX98090_REG_RW_INPUT_CONFIG,           //!< 0x0D: Line Input Mixer Configuration Register.
    MAX98090_REG_RW_LINE_INPUT_LEVEL,       //!< 0x0E: Line Input Level Configuration Register.
    MAX98090_REG_RW_LINE_CONFIG,            //!< 0x0F: Line Input Mode and Source Configuration Register.
    MAX98090_REG_RW_MIC1_INPUT_LEVEL,       //!< 0x10: Microphone 1 Enable and Level Configuration Register.
    MAX98090_REG_RW_MIC2_INPUT_LEVEL,       //!< 0x11: Microphone 2 Enable and Level Configuration Register.
    MAX98090_REG_RW_MIC_BIAS_VOLTAGE,       //!< 0x12: Microphone BIAS Level Configuration Register.
    MAX98090_REG_RW_LEFT_ADC_MIXER,         //!< 0x15: Left ADC Mixer Input Configuration Register.
    MAX98090_REG_RW_RIGHT_ADC_MIXER,        //!< 0x16: Right ADC Mixer Input Configuration Register.
    MAX98090_REG_RW_LEFT_ADC_LEVEL,         //!< 0x17: Left ADC Digital Level Configuration Register.
    MAX98090_REG_RW_RIGHT_ADC_LEVEL,        //!< 0x18: Right ADC Digital Level Configuration Register.
    MAX98090_REG_RW_ADC_BIQUAD_LEVEL,       //!< 0x19: ADC Biquad Digital Preamplifier Level Configuration Register.
    MAX98090_REG_RW_ADC_SIDETONE,           //!< 0x1A: ADC Sidetone Configuration Register.
    MAX98090_REG_RW_SYSTEM_CLOCK,           //!< 0x1B: System Master Clock (MCLK) Prescaler Configuration Register.
    MAX98090_REG_RW_CLOCK_MODE,             //!< 0x1C: Clock Mode Configuration Register.
    MAX98090_REG_RW_ANY_CLOCK_1,            //!< 0x1D: Any Clock Configuration Register 1 (NI1 MSBs).
    MAX98090_REG_RW_ANY_CLOCK_2,            //!< 0x1E: Any Clock Configuration Register 2 (NI1 LSBs).
    MAX98090_REG_RW_ANY_CLOCK_3,            //!< 0x1F: Any Clock Configuration Register 3 (MI1 MSBs).
    MAX98090_REG_RW_ANY_CLOCK_4,            //!< 0x20: Any Clock Configuration Register 4 (MI1 LSBs).
    MAX98090_REG_RW_MASTER_MODE,            //!< 0x21: Master Mode Clock Configuration Register.
    MAX98090_REG_RW_INTERFACE_FORMAT,       //!< 0x22: Digital Audio Interface (DAI) Format Configuration Register.
    MAX98090_REG_RW_TDM_FORMAT_1,           //!< 0x23: Digital Audio Interface (DAI) TDM Format Register1.
    MAX98090_REG_RW_TDM_FORMAT_2,           //!< 0x24: Digital Audio Interface (DAI) TDM Format Register2.
    MAX98090_REG_RW_I_OR_O_CONFIGURATION,   //!< 0x25: Digital Audio Interface (DAI) Input / Output Configuration Register.
    MAX98090_REG_RW_FILTER_CONFIG,          //!< 0x26: Digital Audio Interface (DAI) Filter Configuration Register.
    MAX98090_REG_RW_PLAYBACK_LEVEL_1,       //!< 0x27: Digital Audio Interface (DAI) Playback Level Configuration Register1.
    MAX98090_REG_RW_PLAYBACK_LEVEL_2,       //!< 0x28: Digital Audio Interface (DAI) Playback Level Configuration Register2.
    MAX98090_REG_RW_LEFT_HP_MIXER,          //!< 0x29: Left Headphone Mixer Configuration Register.
    MAX98090_REG_RW_RIGHT_HP_MIXER,         //!< 0x2A: Right Headphone Mixer Configuration Register.
    MAX98090_REG_RW_HP_CONTROL,             //!< 0x2B: Headphone Mixer Control and Gain Register.
    MAX98090_REG_RW_LEFT_HP_VOLUME,         //!< 0x2C: Left Headphone Amplifier Volume Control Register.
    MAX98090_REG_RW_RIGHT_HP_VOLUME,        //!< 0x2D: Right Headphone Amplifier Volume Control Register.
    MAX98090_REG_RW_LEFT_SPK_MIXER,         //!< 0x2E: Left Speaker Mixer Configuration Register.
    MAX98090_REG_RW_RIGHT_SPK_MIXER,        //!< 0x2F: Right Speaker Mixer Configuration Register.
    MAX98090_REG_RW_SPK_CONTROL,            //!< 0x30: Speaker Mixer Gain Register.
    MAX98090_REG_RW_LEFT_SPK_VOLUME,        //!< 0x31: Left Speaker Amplifier Volume Control Register.
    MAX98090_REG_RW_RIGHT_SPK_VOLUME,       //!< 0x32: Right Speaker Amplifier Volume Control Register.
    MAX98090_REG_RW_ALC_TIMING,             //!< 0x33: Automatic Level Control (ALC) Timing Register.
    MAX98090_REG_RW_ALC_COMPRESSOR,         //!< 0x34: Automatic Level Control (ALC) Compressor Register.
    MAX98090_REG_RW_ALC_EXPANDER,           //!< 0x35: Automatic Level Control (ALC) Expander Register.
    MAX98090_REG_RW_ALC_GAIN,               //!< 0x36: Automatic Level Control (ALC) Gain Configuration Register.
    MAX98090_REG_RW_RCV_OR_LOUTL_MIXER,     //!< 0x37: Receiver and Left Line Output Mixer Source Configuration Register.
    MAX98090_REG_RW_RCV_OR_LOUTL_CONTROL,   //!< 0x38: Receiver and Left Line Output Mixer Level Control Register.
    MAX98090_REG_RW_RCV_OR_LOUTL_VOLUME,    //!< 0x39: Receiver and Left Line Output Volume Control Register.
    MAX98090_REG_RW_LOUTR_MIXER,            //!< 0x3A: Right Line Output Mixer Source Configuration Register.
    MAX98090_REG_RW_LOUTR_CONTROL,          //!< 0x3B: Right Line Output Mixer Level Control Register.
    MAX98090_REG_RW_LOUTR_VOLUME,           //!< 0x3C: Right Line Output Volume Control Register.
    MAX98090_REG_RW_JACK_DETECT,            //!< 0x3D: Jack Detect Configuration Register.
    MAX98090_REG_RW_INPUT_ENABLE,           //!< 0x3E: Input Enable Register.
    MAX98090_REG_RW_OUTPUT_ENABLE,          //!< 0x3F: Output Enable Register.
    MAX98090_REG_RW_LEVEL_CONTROL,          //!< 0x40: ZeroACrossing Detection and Volume Smoothing Configuration Register.
    MAX98090_REG_RW_DSP_FILTER_ENABLE,      //!< 0x41: DSP Biquad Filter Enable Register.
    MAX98090_REG_RW_BIAS_CONTROL,           //!< 0x42: Common Mode Bias Voltage Mode Register.
    MAX98090_REG_RW_DAC_CONTROL,            //!< 0x43: Output Power and Performance Mode Register.
    MAX98090_REG_RW_ADC_CONTROL,            //!< 0x44: Input Power and Performance Mode Register.
    MAX98090_REG_RW_DEVICE_SHUTDOWN,        //!< 0x45: Global Shutdown register.
    MAX98090_REG_RW_EQ1B0_1,                //!< 0x46: EQUALIZER BAND 1 COEFFICIENT B0
    MAX98090_REG_RW_EQ1B0_2,                //!< 0x47: EQUALIZER BAND 1 COEFFICIENT B0
    MAX98090_REG_RW_EQ1B0_3,                //!< 0x48: EQUALIZER BAND 1 COEFFICIENT B0
    MAX98090_REG_RW_EQ1B1_1,                //!< 0x49: EQUALIZER BAND 1 COEFFICIENT B1
    MAX98090_REG_RW_EQ1B1_2,                //!< 0x4A: EQUALIZER BAND 1 COEFFICIENT B1
    MAX98090_REG_RW_EQ1B1_3,                //!< 0x4B: EQUALIZER BAND 1 COEFFICIENT B1
    MAX98090_REG_RW_EQ1B2_1,                //!< 0x4C: EQUALIZER BAND 1 COEFFICIENT B2
    MAX98090_REG_RW_EQ1B2_2,                //!< 0x4D: EQUALIZER BAND 1 COEFFICIENT B2
    MAX98090_REG_RW_EQ1B2_3,                //!< 0x4E: EQUALIZER BAND 1 COEFFICIENT B2
    MAX98090_REG_RW_EQ1A1_1,                //!< 0x4F: EQUALIZER BAND 1 COEFFICIENT B0
    MAX98090_REG_RW_EQ1A1_2,                //!< 0x50: EQUALIZER BAND 1 COEFFICIENT A1
    MAX98090_REG_RW_EQ1A1_3,                //!< 0x51: EQUALIZER BAND 1 COEFFICIENT A1
    MAX98090_REG_RW_EQ1A2_1,                //!< 0x52: EQUALIZER BAND 1 COEFFICIENT A2
    MAX98090_REG_RW_EQ1A2_2,                //!< 0x53: EQUALIZER BAND 1 COEFFICIENT A2
    MAX98090_REG_RW_EQ1A2_3,                //!< 0x54: EQUALIZER BAND 1 COEFFICIENT A2
    MAX98090_REG_RW_EQ2B0_1,                //!< 0x55: EQUALIZER BAND 2 COEFFICIENT B0
    MAX98090_REG_RW_EQ2B0_2,                //!< 0x56: EQUALIZER BAND 2 COEFFICIENT B0
    MAX98090_REG_RW_EQ2B0_3,                //!< 0x57: EQUALIZER BAND 2 COEFFICIENT B0
    MAX98090_REG_RW_EQ2B1_1,                //!< 0x58: EQUALIZER BAND 2 COEFFICIENT B1
    MAX98090_REG_RW_EQ2B1_2,                //!< 0x59: EQUALIZER BAND 2 COEFFICIENT B1
    MAX98090_REG_RW_EQ2B1_3,                //!< 0x5A: EQUALIZER BAND 2 COEFFICIENT B1
    MAX98090_REG_RW_EQ2B2_1,                //!< 0x5B: EQUALIZER BAND 2 COEFFICIENT B2
    MAX98090_REG_RW_EQ2B2_2,                //!< 0x5C: EQUALIZER BAND 2 COEFFICIENT B2
    MAX98090_REG_RW_EQ2B2_3,                //!< 0x5D: EQUALIZER BAND 2 COEFFICIENT B2
    MAX98090_REG_RW_EQ2A1_1,                //!< 0x5E: EQUALIZER BAND 2 COEFFICIENT A1
    MAX98090_REG_RW_EQ2A1_2,                //!< 0x5F: EQUALIZER BAND 2 COEFFICIENT A1
    MAX98090_REG_RW_EQ2A1_3,                //!< 0x60: EQUALIZER BAND 2 COEFFICIENT A1
    MAX98090_REG_RW_EQ2A2_1,                //!< 0x61: EQUALIZER BAND 2 COEFFICIENT A2
    MAX98090_REG_RW_EQ2A2_2,                //!< 0x62: EQUALIZER BAND 2 COEFFICIENT A2
    MAX98090_REG_RW_EQ2A2_3,                //!< 0x63: EQUALIZER BAND 2 COEFFICIENT A2
    MAX98090_REG_RW_EQ3B0_1,                //!< 0x64: EQUALIZER BAND 3 COEFFICIENT B0
    MAX98090_REG_RW_EQ3B0_2,                //!< 0x65: EQUALIZER BAND 3 COEFFICIENT B0
    MAX98090_REG_RW_EQ3B0_3,                //!< 0x66: EQUALIZER BAND 3 COEFFICIENT B0
    MAX98090_REG_RW_EQ3B1_1,                //!< 0x67: EQUALIZER BAND 3 COEFFICIENT B1
    MAX98090_REG_RW_EQ3B1_2,                //!< 0x68: EQUALIZER BAND 3 COEFFICIENT B1
    MAX98090_REG_RW_EQ3B1_3,                //!< 0x69: EQUALIZER BAND 3 COEFFICIENT B1
    MAX98090_REG_RW_EQ3B2_1,                //!< 0x6A: EQUALIZER BAND 3 COEFFICIENT B2
    MAX98090_REG_RW_EQ3B2_2,                //!< 0x6B: EQUALIZER BAND 3 COEFFICIENT B2
    MAX98090_REG_RW_EQ3B2_3,                //!< 0x6C: EQUALIZER BAND 3 COEFFICIENT B2
    MAX98090_REG_RW_EQ3A1_1,                //!< 0x6D: EQUALIZER BAND 3 COEFFICIENT A1
    MAX98090_REG_RW_EQ3A1_2,                //!< 0x6E: EQUALIZER BAND 3 COEFFICIENT A1
    MAX98090_REG_RW_EQ3A1_3,                //!< 0x6F: EQUALIZER BAND 3 COEFFICIENT A1
    MAX98090_REG_RW_EQ3A2_1,                //!< 0x70: EQUALIZER BAND 3 COEFFICIENT A2
    MAX98090_REG_RW_EQ3A2_2,                //!< 0x71: EQUALIZER BAND 3 COEFFICIENT A2
    MAX98090_REG_RW_EQ3A2_3,                //!< 0x72: EQUALIZER BAND 3 COEFFICIENT A2
    MAX98090_REG_RW_EQ4B0_1,                //!< 0x73: EQUALIZER BAND 4 COEFFICIENT B0
    MAX98090_REG_RW_EQ4B0_2,                //!< 0x74: EQUALIZER BAND 4 COEFFICIENT B0
    MAX98090_REG_RW_EQ4B0_3,                //!< 0x75: EQUALIZER BAND 4 COEFFICIENT B0
    MAX98090_REG_RW_EQ4B1_1,                //!< 0x76: EQUALIZER BAND 4 COEFFICIENT B1
    MAX98090_REG_RW_EQ4B1_2,                //!< 0x77: EQUALIZER BAND 4 COEFFICIENT B1
    MAX98090_REG_RW_EQ4B1_3,                //!< 0x78: EQUALIZER BAND 4 COEFFICIENT B1
    MAX98090_REG_RW_EQ4B2_1,                //!< 0x79: EQUALIZER BAND 4 COEFFICIENT B2
    MAX98090_REG_RW_EQ4B2_2,                //!< 0x7A: EQUALIZER BAND 4 COEFFICIENT B2
    MAX98090_REG_RW_EQ4B2_3,                //!< 0x7B: EQUALIZER BAND 4 COEFFICIENT B2
    MAX98090_REG_RW_EQ4A1_1,                //!< 0x7C: EQUALIZER BAND 4 COEFFICIENT A1
    MAX98090_REG_RW_EQ4A1_2,                //!< 0x7D: EQUALIZER BAND 4 COEFFICIENT A1
    MAX98090_REG_RW_EQ4A1_3,                //!< 0x7E: EQUALIZER BAND 4 COEFFICIENT A1
    MAX98090_REG_RW_EQ4A2_1,                //!< 0x7F: EQUALIZER BAND 4 COEFFICIENT A2
    MAX98090_REG_RW_EQ4A2_2,                //!< 0x80: EQUALIZER BAND 4 COEFFICIENT A2
    MAX98090_REG_RW_EQ4A2_3,                //!< 0x81: EQUALIZER BAND 4 COEFFICIENT A2
    MAX98090_REG_RW_EQ5B0_1,                //!< 0x82: EQUALIZER BAND 5 COEFFICIENT B0
    MAX98090_REG_RW_EQ5B0_2,                //!< 0x83: EQUALIZER BAND 5 COEFFICIENT B0
    MAX98090_REG_RW_EQ5B0_3,                //!< 0x84: EQUALIZER BAND 5 COEFFICIENT B0
    MAX98090_REG_RW_EQ5B1_1,                //!< 0x85: EQUALIZER BAND 5 COEFFICIENT B1
    MAX98090_REG_RW_EQ5B1_2,                //!< 0x86: EQUALIZER BAND 5 COEFFICIENT B1
    MAX98090_REG_RW_EQ5B1_3,                //!< 0x87: EQUALIZER BAND 5 COEFFICIENT B1
    MAX98090_REG_RW_EQ5B2_1,                //!< 0x88: EQUALIZER BAND 5 COEFFICIENT B2
    MAX98090_REG_RW_EQ5B2_2,                //!< 0x89: EQUALIZER BAND 5 COEFFICIENT B2
    MAX98090_REG_RW_EQ5B2_3,                //!< 0x8A: EQUALIZER BAND 5 COEFFICIENT B2
    MAX98090_REG_RW_EQ5A1_1,                //!< 0x8B: EQUALIZER BAND 5 COEFFICIENT A1
    MAX98090_REG_RW_EQ5A1_2,                //!< 0x8C: EQUALIZER BAND 5 COEFFICIENT A1
    MAX98090_REG_RW_EQ5A1_3,                //!< 0x8D: EQUALIZER BAND 5 COEFFICIENT A1
    MAX98090_REG_RW_EQ5A2_1,                //!< 0x8E: EQUALIZER BAND 5 COEFFICIENT A2
    MAX98090_REG_RW_EQ5A2_2,                //!< 0x8F: EQUALIZER BAND 5 COEFFICIENT A2
    MAX98090_REG_RW_EQ5A2_3,                //!< 0x90: EQUALIZER BAND 5 COEFFICIENT A2
    MAX98090_REG_RW_EQ6B0_1,                //!< 0x91: EQUALIZER BAND 6 COEFFICIENT B0
    MAX98090_REG_RW_EQ6B0_2,                //!< 0x92: EQUALIZER BAND 6 COEFFICIENT B0
    MAX98090_REG_RW_EQ6B0_3,                //!< 0x93: EQUALIZER BAND 6 COEFFICIENT B0
    MAX98090_REG_RW_EQ6B1_1,                //!< 0x94: EQUALIZER BAND 6 COEFFICIENT B1
    MAX98090_REG_RW_EQ6B1_2,                //!< 0x95: EQUALIZER BAND 6 COEFFICIENT B1
    MAX98090_REG_RW_EQ6B1_3,                //!< 0x96: EQUALIZER BAND 6 COEFFICIENT B1
    MAX98090_REG_RW_EQ6B2_1,                //!< 0x97: EQUALIZER BAND 6 COEFFICIENT B2
    MAX98090_REG_RW_EQ6B2_2,                //!< 0x98: EQUALIZER BAND 6 COEFFICIENT B2
    MAX98090_REG_RW_EQ6B2_3,                //!< 0x99: EQUALIZER BAND 6 COEFFICIENT B2
    MAX98090_REG_RW_EQ6A1_1,                //!< 0x9A: EQUALIZER BAND 6 COEFFICIENT A1
    MAX98090_REG_RW_EQ6A1_2,                //!< 0x9B: EQUALIZER BAND 6 COEFFICIENT A1
    MAX98090_REG_RW_EQ6A1_3,                //!< 0x9C: EQUALIZER BAND 6 COEFFICIENT A1
    MAX98090_REG_RW_EQ6A2_1,                //!< 0x9D: EQUALIZER BAND 6 COEFFICIENT A2
    MAX98090_REG_RW_EQ6A2_2,                //!< 0x9E: EQUALIZER BAND 6 COEFFICIENT A2
    MAX98090_REG_RW_EQ6A2_3,                //!< 0x9F: EQUALIZER BAND 6 COEFFICIENT A2
    MAX98090_REG_RW_EQ7B0_1,                //!< 0xA0: EQUALIZER BAND 7 COEFFICIENT B0
    MAX98090_REG_RW_EQ7B0_2,                //!< 0xA1: EQUALIZER BAND 7 COEFFICIENT B0
    MAX98090_REG_RW_EQ7B0_3,                //!< 0xA2: EQUALIZER BAND 7 COEFFICIENT B0
    MAX98090_REG_RW_EQ7B1_1,                //!< 0xA3: EQUALIZER BAND 7 COEFFICIENT B1
    MAX98090_REG_RW_EQ7B1_2,                //!< 0xA4: EQUALIZER BAND 7 COEFFICIENT B1
    MAX98090_REG_RW_EQ7B1_3,                //!< 0xA5: EQUALIZER BAND 7 COEFFICIENT B1
    MAX98090_REG_RW_EQ7B2_1,                //!< 0xA6: EQUALIZER BAND 7 COEFFICIENT B2
    MAX98090_REG_RW_EQ7B2_2,                //!< 0xA7: EQUALIZER BAND 7 COEFFICIENT B2
    MAX98090_REG_RW_EQ7B2_3,                //!< 0xA8: EQUALIZER BAND 7 COEFFICIENT B2
    MAX98090_REG_RW_EQ7A1_1,                //!< 0xA9: EQUALIZER BAND 7 COEFFICIENT A1
    MAX98090_REG_RW_EQ7A1_2,                //!< 0xAA: EQUALIZER BAND 7 COEFFICIENT A1
    MAX98090_REG_RW_EQ7A1_3,                //!< 0xAB: EQUALIZER BAND 7 COEFFICIENT A1
    MAX98090_REG_RW_EQ7A2_1,                //!< 0xAC: EQUALIZER BAND 7 COEFFICIENT A2
    MAX98090_REG_RW_EQ7A2_2,                //!< 0xAD: EQUALIZER BAND 7 COEFFICIENT A2
    MAX98090_REG_RW_EQ7A2_3,                //!< 0xAE: EQUALIZER BAND 7 COEFFICIENT A2
    MAX98090_REG_RW_ADCB0_1,                //!< 0xAF: ADC BIQUAD COEFFICIENT B0
    MAX98090_REG_RW_ADCB0_2,                //!< 0xB0: ADC BIQUAD COEFFICIENT B0
    MAX98090_REG_RW_ADCB0_3,                //!< 0xB1: ADC BIQUAD COEFFICIENT B0
    MAX98090_REG_RW_ADCB1_1,                //!< 0xB2: ADC BIQUAD COEFFICIENT B1
    MAX98090_REG_RW_ADCB1_2,                //!< 0xB3: ADC BIQUAD COEFFICIENT B1
    MAX98090_REG_RW_ADCB1_3,                //!< 0xB4: ADC BIQUAD COEFFICIENT B1
    MAX98090_REG_RW_ADCB2_1,                //!< 0xB5: ADC BIQUAD COEFFICIENT B2
    MAX98090_REG_RW_ADCB2_2,                //!< 0xB6: ADC BIQUAD COEFFICIENT B2
    MAX98090_REG_RW_ADCB2_3,                //!< 0xB7: ADC BIQUAD COEFFICIENT B2
    MAX98090_REG_RW_ADCA1_1,                //!< 0xB8: ADC BIQUAD COEFFICIENT A1
    MAX98090_REG_RW_ADCA1_2,                //!< 0xB9: ADC BIQUAD COEFFICIENT A1
    MAX98090_REG_RW_ADCA1_3,                //!< 0xBA: ADC BIQUAD COEFFICIENT A1
    MAX98090_REG_RW_ADCA2_1,                //!< 0xBB: ADC BIQUAD COEFFICIENT A2
    MAX98090_REG_RW_ADCA2_2,                //!< 0xBC: ADC BIQUAD COEFFICIENT A2
    MAX98090_REG_RW_ADCA2_3,                //!< 0xBD: ADC BIQUAD COEFFICIENT A2
    MAX98090_REG_R_REVISION_ID,             //!< 0xFF: Revision ID Number Register
    MAX98090_REG_ID_MAX
};

/*!
  @brief MAX98090 Register Bit ID value.
*/
enum MAX98090_BIT_ID {
    MAX98090_BIT_SOFTWARE_RESET_SWRESET,        //!< 0x00[7]: Pushbutton Software Device Reset.
    MAX98090_BIT_DEVICE_STATUS_CLD,             //!< 0x01[7]: Clipping Detect Flag.
    MAX98090_BIT_DEVICE_STATUS_SLD,             //!< 0x01[6]: Slew Level Detect Flag.
    MAX98090_BIT_DEVICE_STATUS_ULK,             //!< 0x01[5]: Digital PLL Unlock Flag.
    MAX98090_BIT_DEVICE_STATUS_JDET,            //!< 0x01[2]: Jack Configuration Change Flag.
    MAX98090_BIT_DEVICE_STATUS_ALCACT,          //!< 0x01[1]: ALC Compression Flag.
    MAX98090_BIT_DEVICE_STATUS_ALCCLP,          //!< 0x01[0]: ALC Clipping Flag.
    MAX98090_BIT_JACK_DETECT_LSNS,              //!< 0x02[2]: LOUTP Sense State.
    MAX98090_BIT_JACK_DETECT_JKSNS,             //!< 0x02[1]: JACKSNS Sense State.
    MAX98090_BIT_INTERRUPT_MASKS_ICLD,          //!< 0x03[7]: Clipping Detect Interrupt Enable.
    MAX98090_BIT_INTERRUPT_MASKS_ISLD,          //!< 0x03[6]: Slew Level Detect Interrupt Enable.
    MAX98090_BIT_INTERRUPT_MASKS_IULK,          //!< 0x03[5]: Digital PLL Unlock Interrupt Enable.
    MAX98090_BIT_INTERRUPT_MASKS_IJDET,         //!< 0x03[2]: Jack Configuration Change Interrupt Enable.
    MAX98090_BIT_INTERRUPT_MASKS_IALCACT,       //!< 0x03[1]: ALC Compression Interrupt Enable.
    MAX98090_BIT_INTERRUPT_MASKS_IALCCLP,       //!< 0x03[0]: ALC Clipping Interrupt Enable.
    MAX98090_BIT_SYSTEM_CLOCK_MCLK_26M,         //!< 0x04[7]: Setup Device for Operation with a 26MHz Master Clock (MCLK).
    MAX98090_BIT_SYSTEM_CLOCK_MCLK_19P2M,       //!< 0x04[6]: Setup Device for Operation with a 19.2MHz Master Clock (MCLK).
    MAX98090_BIT_SYSTEM_CLOCK_MCLK_13M,         //!< 0x04[5]: Setup Device for Operation with a 13MHz Master Clock (MCLK).
    MAX98090_BIT_SYSTEM_CLOCK_MCLK_12P288M,     //!< 0x04[4]: Setup Device for Operation with a 12.288MHz Master Clock (MCLK).
    MAX98090_BIT_SYSTEM_CLOCK_MCLK_12M,         //!< 0x04[3]: Setup Device for Operation with a 12MHz Master Clock (MCLK).
    MAX98090_BIT_SYSTEM_CLOCK_MCLK_11P2896M,    //!< 0x04[2]: Setup Device for Operation with a 11.2896MHz Master Clock (MCLK).
    MAX98090_BIT_SYSTEM_CLOCK_MCLK_256Fs,       //!< 0x04[0]: Setup Device for Operation with a 256 x fS MHz Master Clock (MCLK).
    MAX98090_BIT_SAMPLE_RATE_SR_96K,            //!< 0x05[5]: Setup Clocks and Filters for a 96kHz Sample Rate.
    MAX98090_BIT_SAMPLE_RATE_SR_32K,            //!< 0x05[4]: Setup Clocks and Filters for a 32kHz Sample Rate.
    MAX98090_BIT_SAMPLE_RATE_SR_48K,            //!< 0x05[3]: Setup Clocks and Filters for a 48kHz Sample Rate.
    MAX98090_BIT_SAMPLE_RATE_SR_44K1,           //!< 0x05[2]: Setup Clocks and Filters for a 44.1kHz Sample Rate.
    MAX98090_BIT_SAMPLE_RATE_SR_16K,            //!< 0x05[1]: Setup Clocks and Filters for a 16kHz Sample Rate.
    MAX98090_BIT_SAMPLE_RATE_SR_8K,             //!< 0x05[0]: Setup Clocks and Filters for an 8kHz Sample Rate.
    MAX98090_BIT_INTERFACE_RJ_M,                //!< 0x06[5]: Setup DAI for Right Justified Master Mode Operation.
    MAX98090_BIT_INTERFACE_RJ_S,                //!< 0x06[4]: Setup DAI for Right Justified Slave Mode Operation.
    MAX98090_BIT_INTERFACE_LJ_M,                //!< 0x06[3]: Setup DAI for Left Justified Master Mode Operation.
    MAX98090_BIT_INTERFACE_LJ_S,                //!< 0x06[2]: Setup DAI for Left Justified Slave Mode Operation.
    MAX98090_BIT_INTERFACE_I2S_M,               //!< 0x06[1]: Setup DAI for I2S Master Mode Operation.
    MAX98090_BIT_INTERFACE_I2S_S,               //!< 0x06[0]: Setup DAI for I2S Slave Mode Operation.
    MAX98090_BIT_DAC_PATH_DIG2_HP,              //!< 0x07[7]: Setup the DAC to Headphone Path.
    MAX98090_BIT_DAC_PATH_DIG2_EAR,             //!< 0x07[6]: Setup the DAC to Receiver Path.
    MAX98090_BIT_DAC_PATH_DIG2_SPK,             //!< 0x07[5]: Setup the DAC to Speaker Path.
    MAX98090_BIT_DAC_PATH_DIG2_LOUT,            //!< 0x07[4]: Setup the DAC to Line Out Path.
    MAX98090_BIT_MIC_OR_DIRECT_TO_ADC_IN12_MIC1,//!< 0x08[7]: Setup the IN1-IN2 to Microphone 1 to ADCL Path.
    MAX98090_BIT_MIC_OR_DIRECT_TO_ADC_IN34_MIC2,//!< 0x08[6]: Setup the IN3-IN4 to Microphone 2 to ADCR Path.
    MAX98090_BIT_MIC_OR_DIRECT_TO_ADC_IN56_MIC1,//!< 0x08[5]: Setup the IN6-IN5 to Microphone 1 to ADCL Path (WLP Only).
    MAX98090_BIT_MIC_OR_DIRECT_TO_ADC_IN56_MIC2,//!< 0x08[4]: Setup the IN6-IN5 to Microphone 2 to ADCR Path (WLP Only).
    MAX98090_BIT_MIC_OR_DIRECT_TO_ADC_IN12_DADC,//!< 0x08[3]: Setup the IN1-IN2 Direct to ADCL Path.
    MAX98090_BIT_MIC_OR_DIRECT_TO_ADC_IN34_DADC,//!< 0x08[2]: Setup the IN3-IN4 Direct to ADCR Path.
    MAX98090_BIT_MIC_OR_DIRECT_TO_ADC_IN56_DADC,//!< 0x08[1]: Setup the IN6-IN5 Direct to ADCL Path (WLP Only).
    MAX98090_BIT_LINE_TO_ADC_IN12S_AB,          //!< 0x09[7]: Setup Stereo Single Ended Record: IN1/IN2 to Line In A/B to ADCL/R.
    MAX98090_BIT_LINE_TO_ADC_IN34S_AB,          //!< 0x09[6]: Setup Stereo Single Ended Record: IN3/IN4 to Line In A/B to ADCL/R.
    MAX98090_BIT_LINE_TO_ADC_IN56S_AB,          //!< 0x09[5]: Setup Stereo Single Ended Record: IN5/IN6 to Line In A/B to ADCL/R (WLP Only).
    MAX98090_BIT_LINE_TO_ADC_IN34D_A,           //!< 0x09[4]: Setup Mono Differential Record: IN3-IN4 to Line In A to ADCL.
    MAX98090_BIT_LINE_TO_ADC_IN56D_B,           //!< 0x09[3]: Setup Mono Differential Record: IN6-IN5 to Line In B to ADCR (WLP Only).
    MAX98090_BIT_ANALOG_MIC_LOOP_IN12_M1HPL,    //!< 0x0A[7]: Setup the IN1-IN2 Differential to Microphone 1 to Headphone Left Path.
    MAX98090_BIT_ANALOG_MIC_LOOP_IN12_M1SPKL,   //!< 0x0A[6]: Setup the IN1-IN2 Differential to Microphone 1 to Speaker Left Path.
    MAX98090_BIT_ANALOG_MIC_LOOP_IN12_M1EAR,    //!< 0x0A[5]: Setup the IN1-IN2 Differential to Microphone 1 to Receiver Path.
    MAX98090_BIT_ANALOG_MIC_LOOP_IN12_M1LOUTL,  //!< 0x0A[4]: Setup the IN1-IN2 Differential to Microphone 1 to Lineout Left Path.
    MAX98090_BIT_ANALOG_MIC_LOOP_IN34_M2HPR,    //!< 0x0A[3]: Setup the IN3-IN4 Differential to Microphone 2 to Headphone Left Path.
    MAX98090_BIT_ANALOG_MIC_LOOP_IN34_M2SPKR,   //!< 0x0A[2]: Setup the IN3-IN4 Differential to Microphone 2 to Speaker Left Path.
    MAX98090_BIT_ANALOG_MIC_LOOP_IN34_M2EAR,    //!< 0x0A[1]: Setup the IN3-IN4 Differential to Microphone 2 to Receiver Path.
    MAX98090_BIT_ANALOG_MIC_LOOP_N34_M2LOUTR,   //!< 0x0A[0]: Setup the IN3-IN4 Differential to Microphone 2 to Lineout Left Path.
    MAX98090_BIT_ANALOG_LINE_LOOP_IN12S_ABHP,   //!< 0x0B[7]: Setup the IN1/IN2 Single Ended to Line In A/B to Headphone L/R Path
    MAX98090_BIT_ANALOG_LINE_LOOP_IN34D_ASPKL,  //!< 0x0B[6]: Setup the IN3-IN4 Differential to Line In A to Speaker Left Path.
    MAX98090_BIT_ANALOG_LINE_LOOP_IN34D_AEAR,   //!< 0x0B[5]: Setup the IN3-IN4 Differential to Line In A to Receiver Path.
    MAX98090_BIT_ANALOG_LINE_LOOP_IN12S_ABLOUT, //!< 0x0B[4]: Setup the IN1/IN2 Single Ended to Line In A/B to Lineout L/R Path.
    MAX98090_BIT_ANALOG_LINE_LOOP_IN34S_ABHP,   //!< 0x0B[3]: Setup the IN3/IN4 Single Ended to Line In A/B to Headphone L/R Path.
    MAX98090_BIT_ANALOG_LINE_LOOP_IN56D_BSPKR,  //!< 0x0B[2]: Setup the IN6-IN5 Differential to Line In B to Speaker Right Path (WLP Only).
    MAX98090_BIT_ANALOG_LINE_LOOP_IN56D_BEAR,   //!< 0x0B[1]: Setup the IN6-IN5 Differential to Line In B to Receiver Path (WLP Only).
    MAX98090_BIT_ANALOG_LINE_LOOP_IN34S_ABLOUT, //!< 0x0B[0]: Setup the IN3/IN4 Single Ended to Line In A/B to Lineout L/R Path.
    MAX98090_BIT_DIGITAL_MIC_MICCLK,            //!< 0x0C[7]: Digital Microphone Clock Rate Configuration.
    MAX98090_BIT_DIGITAL_MIC_DIGMICR,           //!< 0x0C[5]: Digital Microphone Right Channel Enable.
    MAX98090_BIT_DIGITAL_MIC_DIGMICL,           //!< 0x0C[4]: Digital Microphone Left Channel Enable.
    MAX98090_BIT_INPUT_CONFIG_IN34DIFF,         //!< 0x0D[7]: Selects IN3-IN4 Differentially as an Input to the Line A Mixer.
    MAX98090_BIT_INPUT_CONFIG_IN56DIFF,         //!< 0x0D[6]: Selects IN6-IN5 Differentially as an Input to the Line B Mixer (WLP Only).
    MAX98090_BIT_INPUT_CONFIG_IN1SEEN,          //!< 0x0D[5]: Selects IN1 Single Ended as an Input to the Line A Mixer.
    MAX98090_BIT_INPUT_CONFIG_IN2SEEN,          //!< 0x0D[4]: Selects IN2 Single Ended as an Input to the Line B Mixer.
    MAX98090_BIT_INPUT_CONFIG_IN3SEEN,          //!< 0x0D[3]: Selects IN3 Single Ended as an Input to the Line A Mixer.
    MAX98090_BIT_INPUT_CONFIG_IN4SEEN,          //!< 0x0D[2]: Selects IN4 Single Ended as an Input to the Line B Mixer.
    MAX98090_BIT_INPUT_CONFIG_IN5SEEN,          //!< 0x0D[1]: Selects IN5 Single Ended as an Input to the Line A Mixer (WLP Only).
    MAX98090_BIT_INPUT_CONFIG_IN6SEEN,          //!< 0x0D[0]: Selects IN6 Single Ended as an Input to the Line B Mixer (WLP Only).
    MAX98090_BIT_LINE_INPUT_LEVEL_MIXG135,      //!< 0x0E[7]: Enable for a -6dB Reduction for Multiple Single Ended Line A Mixer Inputs.
    MAX98090_BIT_LINE_INPUT_LEVEL_MIXG246,      //!< 0x0E[6]: Enable for a -6dB Reduction for Multiple Single Ended Line B Mixer Inputs
    MAX98090_BIT_LINE_INPUT_LEVEL_LINAPGA,      //!< 0x0E[5]: Line Input A Programmable Internal Preamp Gain Configuration.
    MAX98090_BIT_LINE_INPUT_LEVEL_LINBPGA,      //!< 0x0E[2]: Line Input B Programmable Internal Preamp Gain Configuration.
    MAX98090_BIT_LINE_CONFIG_EXTBUFA,           //!< 0x0F[7]: Selects External Resistor Gain Mode for Line Input A.
    MAX98090_BIT_LINE_CONFIG_EXTBUFB,           //!< 0x0F[6]: Selects External Resistor Gain Mode for Line Input B.
    MAX98090_BIT_LINE_CONFIG_EXTMIC,            //!< 0x0F[1]: External Microphone (IN6-IN5) Input Control Configuration (WLP Only).
    MAX98090_BIT_MIC1_INPUT_LEVEL_PA1EN,        //!< 0x10[6]: Enables Microphone 1 Input Amplifier and Selects the Coarse Gain Setting.
    MAX98090_BIT_MIC1_INPUT_LEVEL_PGAM1,        //!< 0x10[4]: Microphone 1 Programmable Gain Amplifier Fine Adjust Configuration.
    MAX98090_BIT_MIC2_INPUT_LEVEL_PA2EN,        //!< 0x11[6]: Enables Microphone 2 Input Amplifier and Selects the Coarse Gain Setting.
    MAX98090_BIT_MIC2_INPUT_LEVEL_PGAM2,        //!< 0x11[4]: Microphone 2 Programmable Gain Amplifier Fine Adjust Configuration.
    MAX98090_BIT_MIC_BIAS_VOLTAGE_MBVSEL,       //!< 0x12[1]: Microphone Bias Level Configuration.
    MAX98090_BIT_LEFT_ADC_MIXER_MIXADL7,        //!< 0x15[7]: Select IN1-IN2 Differential Input Direct to Left ADC Mixer.
    MAX98090_BIT_LEFT_ADC_MIXER_MIXADL6,        //!< 0x15[6]: Select IN3-IN4 Differential Input Direct to Left ADC Mixer.
    MAX98090_BIT_LEFT_ADC_MIXER_MIXADL5,        //!< 0x15[5]: Select IN6-IN5 Differential Input Direct to Left ADC Mixer (WLP Only).
    MAX98090_BIT_LEFT_ADC_MIXER_MIXADL4,        //!< 0x15[4]: Select Line Input A to Left ADC Mixer.
    MAX98090_BIT_LEFT_ADC_MIXER_MIXADL3,        //!< 0x15[3]: Select Line Input B to Left ADC Mixer.
    MAX98090_BIT_LEFT_ADC_MIXER_MIXADL2,        //!< 0x15[2]: Select Microphone Input 1 to Left ADC Mixer.
    MAX98090_BIT_LEFT_ADC_MIXER_MIXADL1,        //!< 0x15[1]: Select Microphone Input 2 to Left ADC Mixer.
    MAX98090_BIT_LEFT_ADC_MIXER_MIXADL0,        //!< 0x15[0]: Disables Left ADC Mixer and Selects IN1-IN2 Differential Direct to Left ADC.
    MAX98090_BIT_RIGHT_ADC_MIXER_MIXADR7,       //!< 0x16[7]: Select IN1-IN2 Differential Input Direct to Right ADC Mixer.
    MAX98090_BIT_RIGHT_ADC_MIXER_MIXADR6,       //!< 0x16[6]: Select IN3-IN4 Differential Input Direct to Right ADC Mixer.
    MAX98090_BIT_RIGHT_ADC_MIXER_MIXADR5,       //!< 0x16[5]: Select IN6-IN5 Differential Input Direct to Right ADC Mixer (WLP Only).
    MAX98090_BIT_RIGHT_ADC_MIXER_MIXADR4,       //!< 0x16[4]: Select Line Input A to Right ADC Mixer.
    MAX98090_BIT_RIGHT_ADC_MIXER_MIXADR3,       //!< 0x16[3]: Select Line Input B to Right ADC Mixer.
    MAX98090_BIT_RIGHT_ADC_MIXER_MIXADR2,       //!< 0x16[2]: Select Microphone Input 1 to Right ADC Mixer.
    MAX98090_BIT_RIGHT_ADC_MIXER_MIXADR1,       //!< 0x16[1]: Select Microphone Input 2 to Right ADC Mixer.
    MAX98090_BIT_RIGHT_ADC_MIXER_MIXADR0,       //!< 0x16[0]: Disables Right ADC Mixer and Selects IN1-IN2 Differential Direct to Right ADC.
    MAX98090_BIT_LEFT_ADC_LEVEL_AVLG,           //!< 0x17[5]: Left ADC Digital Coarse Gain Configuration.
    MAX98090_BIT_LEFT_ADC_LEVEL_AVL,            //!< 0x17[3]: Left ADC Digital Fine Adjust Gain Configuration.
    MAX98090_BIT_RIGHT_ADC_LEVEL_AVRG,          //!< 0x18[5]: ADC Gain Control.
    MAX98090_BIT_RIGHT_ADC_LEVEL_AVR,           //!< 0x18[3]: ADC Level Control.
    MAX98090_BIT_ADC_BIQUAD_LEVEL_AVBQ,         //!< 0x19[3]: ADC Biquad Digital Preamplifier Gain Configuration.
    MAX98090_BIT_ADC_SIDETONE_DSTS,             //!< 0x1A[7]: ADC Sidetone Enable and Digital Source Configuration.
    MAX98090_BIT_ADC_SIDETONE_DVST,             //!< 0x1A[4]: ADC Sidetone Digital Gain Configuration.
    MAX98090_BIT_SYSTEM_CLOCK_PSCLK,            //!< 0x1B[5]: Master Clock (MCLK) Prescaler Configuration.
    MAX98090_BIT_CLOCK_MODE_FREQ1,              //!< 0x1C[7]: Exact Integer Sampling Frequency (LRCLK) Configuration.
    MAX98090_BIT_CLOCK_MODE_USE_MI1,            //!< 0x1C[0]: Set the PLL to use MI1[15:0] to set a More Accurate Frequency Ratio.
    MAX98090_BIT_ANY_CLOCK_1_NI1,               //!< 0x1D[6]: Upper Half of the PLL N Value used in Master Mode Clock Generation to Calculate the Frequency Ratio (Integer or NI Master Mode).
    MAX98090_BIT_ANY_CLOCK_2_NI1,               //!< 0x1E[7]: Lower Half of the PLL N Value used in Master Mode Clock Generation to Calculate the Frequency Ratio (Integer or NI Master Mode).
    MAX98090_BIT_ANY_CLOCK_3_MI1,               //!< 0x1F[7]: Upper Half of the PLL M Value used in Master Mode Clock Generation to Calculate an Accurate Non-Integer Frequency Ratio (PLL Master Mode).
    MAX98090_BIT_ANY_CLOCK_4_MI1,               //!< 0x20[7]: Lower Half of the PLL M Value used in Master Mode Clock Generation to Calculate an Accurate Non-Integer Frequency Ratio (PLL Master Mode).
    MAX98090_BIT_MASTER_MODE_MAS,               //!< 0x21[7]: Master Mode Enable.
    MAX98090_BIT_MASTER_MODE_BSEL,              //!< 0x21[2]: Bit Clock (BCLK) Rate Configuration (Master Mode Only).
    MAX98090_BIT_INTERFACE_FORMAT_RJ,           //!< 0x22[5]: Configures the DAI for Right Justified Mode (No Data Delay).
    MAX98090_BIT_INTERFACE_FORMAT_WCI1,         //!< 0x22[4]: Configures the DAI for Frame Clock (LRCLK) Inversion.
    MAX98090_BIT_INTERFACE_FORMAT_WCI2,         //!< 0x22[4]: Configures the DAI for Frame Clock (LRCLK) Inversion.
    MAX98090_BIT_INTERFACE_FORMAT_BCI1,         //!< 0x22[3]: Configures the DAI for Bit Clock (BCLK) Inversion.
    MAX98090_BIT_INTERFACE_FORMAT_BCI2,         //!< 0x22[3]: Configures the DAI for Bit Clock (BCLK) Inversion.
    MAX98090_BIT_INTERFACE_FORMAT_DLY,          //!< 0x22[2]: Configures the DAI for Data Delay (I2S Standard).
    MAX98090_BIT_INTERFACE_FORMAT_WS2,          //!< 0x22[1]: DAI Input Data Word Size.
    MAX98090_BIT_TDM_FORMAT_1_FSW,              //!< 0x23[1]: Configures the DAI Frame Sync Pulse Width.
    MAX98090_BIT_TDM_FORMAT_1_TDM,              //!< 0x23[0]: Enables the DAI for Time Division Multiplex (TDM) Mode.
    MAX98090_BIT_TDM_FORMAT_2_SLOTL,            //!< 0x24[7]: Selects the Time Slot to use for Left Channel Data in TDM Mode.
    MAX98090_BIT_TDM_FORMAT_2_SLOTR,            //!< 0x24[5]: Selects the Time Slot to use for Right Channel Data in TDM Mode.
    MAX98090_BIT_TDM_FORMAT_2_SLOTDLY4,         //!< 0x24[3]: Enables Data Delay for Slot 4 in TDM Mode.
    MAX98090_BIT_TDM_FORMAT_2_SLOTDLY3,         //!< 0x24[2]: Enables Data Delay for Slot 3 in TDM Mode.
    MAX98090_BIT_TDM_FORMAT_2_SLOTDLY2,         //!< 0x24[1]: Enables Data Delay for Slot 2 in TDM Mode.
    MAX98090_BIT_TDM_FORMAT_2_SLOTDLY1,         //!< 0x24[0]: Enables Data Delay for Slot 1 in TDM Mode.
    MAX98090_BIT_I_OR_O_CONFIGURATION_LTEN,     //!< 0x25[5]: Enables Data Loop Through from the ADC Output to the DAC Input.
    MAX98090_BIT_I_OR_O_CONFIGURATION_LBEN,     //!< 0x25[4]: Enables Data Loop Back from the SDIEN Input to the SDOEN Output.
    MAX98090_BIT_I_OR_O_CONFIGURATION_DMONO,    //!< 0x25[3]: Enables DAC Mono Mode where SDIN L/R are Mixed and Input to DAC L/R.
    MAX98090_BIT_I_OR_O_CONFIGURATION_HIZOFF,   //!< 0x25[2]: Disables Hi-Z Mode for SDOUT.
    MAX98090_BIT_I_OR_O_CONFIGURATION_SDOEN,    //!< 0x25[1]: Enables the Serial Data Output.
    MAX98090_BIT_I_OR_O_CONFIGURATION_SDIEN,    //!< 0x25[0]: Enables the Serial Data Input.
    MAX98090_BIT_FILTER_CONFIG_MODE,            //!< 0x26[7]: Enables the CODEC DSP FIR Music filters (Default IIR Voice Filters).
    MAX98090_BIT_FILTER_CONFIG_AHPF,            //!< 0x26[6]: Enables the ADC DC Blocking Filter (FIR Only).
    MAX98090_BIT_FILTER_CONFIG_DHPF,            //!< 0x26[5]: Enables the DAC DC Blocking Filter (FIR Only).
    MAX98090_BIT_FILTER_CONFIG_DHF,             //!< 0x26[4]: Enables the DAC High Sample Rate Mode (LRCLK > 50kHz, FIR Only).
    MAX98090_BIT_PLAYBACK_LEVEL_1_DV1M,         //!< 0x27[7]: Enables the DAI DAC Data Input Mute.
    MAX98090_BIT_PLAYBACK_LEVEL_1_DV1G,         //!< 0x27[5]: DAI Digital Input Coarse Adjust Gain Configuration.
    MAX98090_BIT_PLAYBACK_LEVEL_1_DV1,          //!< 0x27[3]: DAI Digital Input Fine Adjust Gain Configuration.
    MAX98090_BIT_PLAYBACK_LEVEL_2_EQCLP,        //!< 0x28[4]: Enables DAI Digital Input Equalizer Clipping Detection.
    MAX98090_BIT_PLAYBACK_LEVEL_2_DVEQ,         //!< 0x28[3]: DAI Digital Input Equalizer Attenuation Level Configuration.
    MAX98090_BIT_LEFT_HP_MIXER_MIXHPL5,         //!< 0x29[5]: Select Left DAC Output to Left Headphone Mixer.
    MAX98090_BIT_LEFT_HP_MIXER_MIXHPL4,         //!< 0x29[4]: Select Right DAC Output to Left Headphone Mixer.
    MAX98090_BIT_LEFT_HP_MIXER_MIXHPL3,         //!< 0x29[3]: Select Line Input A to Left Headphone Mixer.
    MAX98090_BIT_LEFT_HP_MIXER_MIXHPL2,         //!< 0x29[2]: Select Line Input B to Left Headphone Mixer.
    MAX98090_BIT_LEFT_HP_MIXER_MIXHPL1,         //!< 0x29[1]: Select Microphone Input 1 to Left Headphone Mixer.
    MAX98090_BIT_LEFT_HP_MIXER_MIXHPL0,         //!< 0x29[0]: Select Microphone Input 2 to Left Headphone Mixer.
    MAX98090_BIT_RIGHT_HP_MIXER_MIXHPR5,        //!< 0x2A[5]: Select Left DAC Output to Right Headphone Mixer.
    MAX98090_BIT_RIGHT_HP_MIXER_MIXHPR4,        //!< 0x2A[4]: Select Right DAC Output to Right Headphone Mixer.
    MAX98090_BIT_RIGHT_HP_MIXER_MIXHPR3,        //!< 0x2A[3]: Select Line Input A to Right Headphone Mixer.
    MAX98090_BIT_RIGHT_HP_MIXER_MIXHPR2,        //!< 0x2A[2]: Select Line Input B to Right Headphone Mixer.
    MAX98090_BIT_RIGHT_HP_MIXER_MIXHPR1,        //!< 0x2A[1]: Select Microphone Input 1 to Right Headphone Mixer.
    MAX98090_BIT_RIGHT_HP_MIXER_MIXHPR0,        //!< 0x2A[0]: Select Microphone Input 2 to Right Headphone Mixer.
    MAX98090_BIT_HP_CONTROL_MIXHPRSEL,          //!< 0x2B[5]: Select Headphone Mixer as Right Input Source (Default DAC Right Direct).
    MAX98090_BIT_HP_CONTROL_MIXHPLSEL,          //!< 0x2B[4]: Select Headphone Mixer as Left Input Source (Default DAC Left Direct).
    MAX98090_BIT_HP_CONTROL_MIXHPRG,            //!< 0x2B[3]: Right Headphone Mixer Gain Configuration.
    MAX98090_BIT_HP_CONTROL_MIXHPLG,            //!< 0x2B[1]: Left Headphone Mixer Gain Configuration.
    MAX98090_BIT_LEFT_HP_VOLUME_HPLM,           //!< 0x2C[7]: Left Headphone Output Mute Enable.
    MAX98090_BIT_LEFT_HP_VOLUME_HPVOLL,         //!< 0x2C[4]: Left Headphone Output Amplifier Volume Control Configuration.
    MAX98090_BIT_RIGHT_HP_VOLUME_HPRM,          //!< 0x2D[7]: Right Headphone Output Mute Enable.
    MAX98090_BIT_RIGHT_HP_VOLUME_HPVOLR,        //!< 0x2D[4]: Right Headphone Output Amplifier Volume Control Configuration.
    MAX98090_BIT_LEFT_SPK_MIXER_MIXSPL5,        //!< 0x2E[5]: Select Left DAC Output to Left Speaker Mixer.
    MAX98090_BIT_LEFT_SPK_MIXER_MIXSPL4,        //!< 0x2E[4]: Select Right DAC Output to Left Speaker Mixer.
    MAX98090_BIT_LEFT_SPK_MIXER_MIXSPL3,        //!< 0x2E[3]: Select Line Input A to Left Speaker Mixer.
    MAX98090_BIT_LEFT_SPK_MIXER_MIXSPL2,        //!< 0x2E[2]: Select Line Input B to Left Speaker Mixer.
    MAX98090_BIT_LEFT_SPK_MIXER_MIXSPL1,        //!< 0x2E[1]: Select Microphone Input 1 to Left Speaker Mixer.
    MAX98090_BIT_LEFT_SPK_MIXER_MIXSPL0,        //!< 0x2E[0]: Select Microphone Input 2 to Left Speaker Mixer.
    MAX98090_BIT_RIGHT_SPK_MIXER_SPK_SLAVE,     //!< 0x2F[6]: Speaker Slave Mode Enable.
    MAX98090_BIT_RIGHT_SPK_MIXER_MIXSPR5,       //!< 0x2F[5]: Select Left DAC Output to Right Speaker Mixer.
    MAX98090_BIT_RIGHT_SPK_MIXER_MIXSPR4,       //!< 0x2F[4]: Select Right DAC Output to Right Speaker Mixer.
    MAX98090_BIT_RIGHT_SPK_MIXER_MIXSPR3,       //!< 0x2F[3]: Select Line Input A to Right Speaker Mixer.
    MAX98090_BIT_RIGHT_SPK_MIXER_MIXSPR2,       //!< 0x2F[2]: Select Line Input B to Right Speaker Mixer.
    MAX98090_BIT_RIGHT_SPK_MIXER_MIXSPR1,       //!< 0x2F[1]: Select Microphone Input 1 to Right Speaker Mixer.
    MAX98090_BIT_RIGHT_SPK_MIXER_MIXSPR0,       //!< 0x2F[0]: Select Microphone Input 2 to Right Speaker Mixer.
    MAX98090_BIT_SPK_CONTROL_MIXSPRG,           //!< 0x30[3]: Right Speaker Mixer Gain Configuration.
    MAX98090_BIT_SPK_CONTROL_MIXSPLG,           //!< 0x30[1]: Left Speaker Mixer Gain Configuration.
    MAX98090_BIT_LEFT_SPK_VOLUME_SPLM,          //!< 0x31[7]: Left Speaker Output Mute Enable.
    MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL,        //!< 0x31[5]: Left Speaker Output Amplifier Volume Control Configuration.
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPRM,         //!< 0x32[7]: Right Speaker Output Mute Enable.
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR,       //!< 0x32[5]: Right Speaker Output Amplifier Volume Control Configuration.
    MAX98090_BIT_ALC_TIMING_ALCEN,              //!< 0x33[7]: DAC ALC Enable.
    MAX98090_BIT_ALC_TIMING_ALCRLS,             //!< 0x33[6]: DAC ALC Release Time Configuration.
    MAX98090_BIT_ALC_TIMING_ALCATK,             //!< 0x33[2]: DAC ALC Attack Time Configuration.
    MAX98090_BIT_ALC_COMPRESSOR_ALCCMP,         //!< 0x34[7]: DAC ALC Compression Ratio Configuration.
    MAX98090_BIT_ALC_COMPRESSOR_ALCTHC,         //!< 0x34[4]: DAC ALC Compression Threshold Configuration.
    MAX98090_BIT_ALC_EXPANDER_ALCEXP,           //!< 0x35[7]: DAC ALC Expansion Ratio Configuration.
    MAX98090_BIT_ALC_EXPANDER_ALCTHE,           //!< 0x35[4]: DAC ALC Expansion Threshold Configuration.
    MAX98090_BIT_ALC_GAIN_ALCG,                 //!< 0x36[4]: DAC ALC Make-Up Gain Configuration.
    MAX98090_BIT_RCV_OR_LOUTL_MIXER_MIXRCVL5,   //!< 0x37[5]: Selects DAC Left as the Input to the Receiver / Line Out Left Mixer.
    MAX98090_BIT_RCV_OR_LOUTL_MIXER_MIXRCVL4,   //!< 0x37[4]: Selects DAC Right as the Input to the Receiver / Line Out Left Mixer.
    MAX98090_BIT_RCV_OR_LOUTL_MIXER_MIXRCVL3,   //!< 0x37[3]: Selects Line A as the Input to the Receiver / Line Out Left Mixer.
    MAX98090_BIT_RCV_OR_LOUTL_MIXER_MIXRCVL2,   //!< 0x37[2]: Selects Line B as the Input to the Receiver / Line Out Left Mixer.
    MAX98090_BIT_RCV_OR_LOUTL_MIXER_MIXRCVL1,   //!< 0x37[1]: Selects MIC 1 as the Input to the Receiver / Line Out Left Mixer.
    MAX98090_BIT_RCV_OR_LOUTL_MIXER_MIXRCVL0,   //!< 0x37[0]: Selects MIC 2 as the Input to the Receiver / Line Out Left Mixer.
    MAX98090_BIT_RCV_OR_LOUTL_CONTROL_MIXRCVLG, //!< 0x38[1]: Receiver / Line Output Left Mixer Gain Configuration.
    MAX98090_BIT_RCV_OR_LOUTL_VOLUME_RCVLM,     //!< 0x39[7]: Receiver / Line Output Left Mute.
    MAX98090_BIT_RCV_OR_LOUTL_VOLUME_RCVLVOL,   //!< 0x39[4]: Receiver / Line Output Left PGA Volume Configuration.
    MAX98090_BIT_LOUTR_MIXER_LINMOD,            //!< 0x3A[7]: Selects Between Receiver BTL mode and Line Output mode.
    MAX98090_BIT_LOUTR_MIXER_MIXRCVR5,          //!< 0x3A[5]: Selects DAC Left as the Input to the Line Out Right Mixer.
    MAX98090_BIT_LOUTR_MIXER_MIXRCVR4,          //!< 0x3A[4]: Selects DAC Right as the Input to the Line Out Right Mixer.
    MAX98090_BIT_LOUTR_MIXER_MIXRCVR3,          //!< 0x3A[3]: Selects Line A as the Input to the Line Out Right Mixer.
    MAX98090_BIT_LOUTR_MIXER_MIXRCVR2,          //!< 0x3A[2]: Selects Line B as the Input to the Line Out Right Mixer.
    MAX98090_BIT_LOUTR_MIXER_MIXRCVR1,          //!< 0x3A[1]: Selects MIC 1 as the Input to the Line Out Right Mixer.
    MAX98090_BIT_LOUTR_MIXER_MIXRCVR0,          //!< 0x3A[0]: Selects MIC 2 as the Input to the Line Out Right Mixer.
    MAX98090_BIT_LOUTR_CONTROL_MIXRCVRG,        //!< 0x3B[1]: Line Output Right Mixer Gain Configuration.
    MAX98090_BIT_LOUTR_VOLUME_RCVRM,            //!< 0x3C[7]: Line Output Right Mute.
    MAX98090_BIT_LOUTR_VOLUME_RCVRVOL,          //!< 0x3C[4]: Line Output Right PGA Volume Configuration.
    MAX98090_BIT_JACK_DETECT_JDETEN,            //!< 0x3D[7]: Jack Detect Enable (JDETEN) : Enables the Jack Detect Circuitry.
    MAX98090_BIT_JACK_DETECT_JKWK,              //!< 0x3D[6]: JACKSNS Pull-up.
    MAX98090_BIT_JACK_DETECT_JDEB,              //!< 0x3D[1]: Jack Detect Debounce (JDEB).
    MAX98090_BIT_INPUT_ENABLE_MBEN,             //!< 0x3E[4]: Microphone Bias Enable.
    MAX98090_BIT_INPUT_ENABLE_LINEAEN,          //!< 0x3E[3]: Enables Line A Analog Input Block.
    MAX98090_BIT_INPUT_ENABLE_LINEBEN,          //!< 0x3E[2]: Enables Line A Analog Input Block.
    MAX98090_BIT_INPUT_ENABLE_ADREN,            //!< 0x3E[1]: Right ADC Enable.
    MAX98090_BIT_INPUT_ENABLE_ADLEN,            //!< 0x3E[0]: Left ADC Enable.
    MAX98090_BIT_OUTPUT_ENABLE_HPREN,           //!< 0x3F[7]: Right Headphone Output Enable.
    MAX98090_BIT_OUTPUT_ENABLE_HPLEN,           //!< 0x3F[6]: Left Headphone Output Enable.
    MAX98090_BIT_OUTPUT_ENABLE_SPREN,           //!< 0x3F[5]: Right Class-D Speaker Output Enable.
    MAX98090_BIT_OUTPUT_ENABLE_SPLEN,           //!< 0x3F[4]: Left Class-D Speaker Output Enable.
    MAX98090_BIT_OUTPUT_ENABLE_RCVLEN,          //!< 0x3F[3]: Receiver (Earpiece) / Line Out Left Output Enable.
    MAX98090_BIT_OUTPUT_ENABLE_RCVREN,          //!< 0x3F[2]: Right Line Output Enable.
    MAX98090_BIT_OUTPUT_ENABLE_DAREN,           //!< 0x3F[1]: Right DAC Digital Input Enable.
    MAX98090_BIT_OUTPUT_ENABLE_DALEN,           //!< 0x3F[0]: Left DAC Digital Input Enable.
    MAX98090_BIT_LEVEL_CONTROL_ZDEN,            //!< 0x40[2]: Zero-Crossing Detection (/ZDEN).
    MAX98090_BIT_LEVEL_CONTROL_VS2EN,           //!< 0x40[1]: Enhanced Volume Smoothing (/VS2EN).
    MAX98090_BIT_LEVEL_CONTROL_VSEN,            //!< 0x40[0]: Volume Adjustment Smoothing (/VSEN).
    MAX98090_BIT_DSP_FILTER_ENABLE_ADCBQEN,     //!< 0x41[3]: Enable Biquad filter in ADC path.
    MAX98090_BIT_DSP_FILTER_ENABLE_EQ3BANDEN,   //!< 0x41[2]: Enable 3 Band EQ in DAC path.
    MAX98090_BIT_DSP_FILTER_ENABLE_EQ5BANDEN,   //!< 0x41[1]: Enable 5 Band EQ in DAC path.
    MAX98090_BIT_DSP_FILTER_ENABLE_EQ7BANDEN,   //!< 0x41[0]: Enable 7 Band EQ in DAC path.
    MAX98090_BIT_BIAS_CONTROL_VCM_MODE,         //!< 0x42[0]: Select source for VCM.
    MAX98090_BIT_DAC_CONTROL_PERFMODE,          //!< 0x43[1]: Performance Mode (PERFMODE).
    MAX98090_BIT_DAC_CONTROL_DACHP,             //!< 0x43[0]: DAC High Performance Mode.
    MAX98090_BIT_ADC_CONTROL_OSR128,            //!< 0x44[2]: Generate ADC clock at 128*fS or 64*fS.
    MAX98090_BIT_ADC_CONTROL_ADCDITHER,         //!< 0x44[1]: Enable Dither to the ADC quantizer.
    MAX98090_BIT_ADC_CONTROL_ADCHP,             //!< 0x44[0]: ADC High Performance Mode.
    MAX98090_BIT_DEVICE_SHUTDOWN_SHDN,          //!< 0x45[7]: Device Shutdown.
    MAX98090_BIT_REVISION_ID_REV_ID,            //!< 0xFF[7]: Read back the revision ID of the device 
    MAX98090_BIT_ID_MAX
};

/*!
  @brief MAX98090 DIGITAL_MIC[MICCLK] Value: Digital Microphone Clock Rate Configuration.
*/
enum MAX98090_BIT_DIGITAL_MIC_MICCLK_VAL {
    MAX98090_BIT_DIGITAL_MIC_MICCLK_PCLK8   = 0x00, //!< DIGMICCLK = PCLK / 8.
    MAX98090_BIT_DIGITAL_MIC_MICCLK_PCLK6   = 0x01, //!< DIGMICCLK = PCLK / 6.
    MAX98090_BIT_DIGITAL_MIC_MICCLK_64FS    = 0x02, //!< DIGMICCLK = 64 x fS.
    MAX98090_BIT_DIGITAL_MIC_MICCLK_128FS   = 0x03  //!< DIGMICCLK = 128 x fS.
};

/*!
  @brief MAX98090 DIGITAL_MIC[DIGMICR] Value: Digital Microphone Right Channel Enable.
*/
enum MAX98090_BIT_DIGITAL_MIC_DIGMICR_VAL {
    MAX98090_BIT_DIGITAL_MIC_DIGMICR_ON_CHIP = 0,   //!< Right record channel uses on-chip ADC.
    MAX98090_BIT_DIGITAL_MIC_DIGMICR_DIGITAL = 1    //!< Right record channel uses digital microphone input.
};

/*!
  @brief MAX98090 DIGITAL_MIC[DIGMICL] Value: Digital Microphone Left Channel Enable.
*/
enum MAX98090_BIT_DIGITAL_MIC_DIGMICL_VAL {
    MAX98090_BIT_DIGITAL_MIC_DIGMICL_ON_CHIP = 0,   //!< Left record channel uses on-chip ADC.
    MAX98090_BIT_DIGITAL_MIC_DIGMICL_DIGITAL = 1    //!< Left record channel uses digital microphone input.
};

/*!
  @brief MAX98090 LINE_INPUT_LEVEL[MIXG135] Value: Enable for a -6dB Reduction for Multiple Single Ended Line A Mixer Inputs.
*/
enum MAX98090_BIT_LINE_INPUT_LEVEL_MIXG135_VAL {
    MAX98090_BIT_LINE_INPUT_LEVEL_MIXG135_NOMAL = 0,    //!< Normal Line A Mixer Operation.
    MAX98090_BIT_LINE_INPUT_LEVEL_MIXG135_GAIN  = 1     //!< Gain is Reduced by -6dB when Multiple Single Ended Inputs are selected.
};

/*!
  @brief MAX98090 LINE_INPUT_LEVEL[MIXG246] Value: Enable for a -6dB Reduction for Multiple Single Ended Line B Mixer Inputs.
*/
enum MAX98090_BIT_LINE_INPUT_LEVEL_MIXG246_VAL {
    MAX98090_BIT_LINE_INPUT_LEVEL_MIXG246_NOMAL = 0,    //!< Normal Line B Mixer Operation.
    MAX98090_BIT_LINE_INPUT_LEVEL_MIXG246_GAIN  = 1     //!< Gain is Reduced by -6dB when Multiple Single Ended Inputs are selected.
};

/*!
  @brief MAX98090 LINE_INPUT_LEVEL[LINAPGA] Value: Line Input A Programmable Internal Preamp Gain Configuration.
*/
enum MAX98090_BIT_LINE_INPUT_LEVEL_LINAPGA_VAL {
    MAX98090_BIT_LINE_INPUT_LEVEL_LINAPGA_20DB      = 0x00, //!< 20dB
    MAX98090_BIT_LINE_INPUT_LEVEL_LINAPGA_14DB      = 0x01, //!< 14dB
    MAX98090_BIT_LINE_INPUT_LEVEL_LINAPGA_3DB       = 0x02, //!< 3dB
    MAX98090_BIT_LINE_INPUT_LEVEL_LINAPGA_0DB       = 0x03, //!< 0dB
    MAX98090_BIT_LINE_INPUT_LEVEL_LINAPGA_M3DB      = 0x04, //!< -3dB
    MAX98090_BIT_LINE_INPUT_LEVEL_LINAPGA_M6DB_1    = 0x05, //!< -6dB
    MAX98090_BIT_LINE_INPUT_LEVEL_LINAPGA_M6DB_2    = 0x06, //!< -6dB
    MAX98090_BIT_LINE_INPUT_LEVEL_LINAPGA_M6DB_3    = 0x07  //!< -6dB
};

/*!
  @brief MAX98090 LINE_INPUT_LEVEL[LINBPGA] Value: Line Input B Programmable Internal Preamp Gain Configuration.
*/
enum MAX98090_BIT_LINE_INPUT_LEVEL_LINBPGA_VAL {
    MAX98090_BIT_LINE_INPUT_LEVEL_LINBPGA_20DB      = 0x00, //!< 20dB
    MAX98090_BIT_LINE_INPUT_LEVEL_LINBPGA_14DB      = 0x01, //!< 14dB
    MAX98090_BIT_LINE_INPUT_LEVEL_LINBPGA_3DB       = 0x02, //!< 3dB
    MAX98090_BIT_LINE_INPUT_LEVEL_LINBPGA_0DB       = 0x03, //!< 0dB
    MAX98090_BIT_LINE_INPUT_LEVEL_LINBPGA_M3DB      = 0x04, //!< -3dB
    MAX98090_BIT_LINE_INPUT_LEVEL_LINBPGA_M6DB_1    = 0x05, //!< -6dB
    MAX98090_BIT_LINE_INPUT_LEVEL_LINBPGA_M6DB_2    = 0x06, //!< -6dB
    MAX98090_BIT_LINE_INPUT_LEVEL_LINBPGA_M6DB_3    = 0x07  //!< -6dB
};

/*!
  @brief MAX98090 LINE_CONFIG[EXTMIC] Value: External Microphone (IN6-IN5) Input Control Configuration (WLP Only).
*/
enum MAX98090_BIT_LINE_CONFIG_EXTMIC_VAL {
    MAX98090_BIT_LINE_CONFIG_EXTMIC_MODE1 = 0x00,   //!< EXT_MIC not selected.
    MAX98090_BIT_LINE_CONFIG_EXTMIC_MODE2 = 0x01,   //!< EXT_MIC selected on MIC 1.
    MAX98090_BIT_LINE_CONFIG_EXTMIC_MODE3 = 0x02,   //!< EXT_MIC selected on MIC 2.
    MAX98090_BIT_LINE_CONFIG_EXTMIC_MODE4 = 0x03    //!< EXT_MIC not selected.
};
/*!
  @brief MAX98090 MIC1_INPUT_LEVEL[PA1EN] Value: Enables Microphone 1 Input Amplifier and Selects the Coarse Gain Setting.
*/
enum MAX98090_BIT_MIC1_INPUT_LEVEL_PA1EN_VAL {
    MAX98090_BIT_MIC1_INPUT_LEVEL_PA1EN_DISABLED    = 0x00, //!< Disabled
    MAX98090_BIT_MIC1_INPUT_LEVEL_PA1EN_0DB         = 0x01, //!< 0dB
    MAX98090_BIT_MIC1_INPUT_LEVEL_PA1EN_20DB        = 0x02, //!< 20dB
    MAX98090_BIT_MIC1_INPUT_LEVEL_PA1EN_30DB        = 0x03  //!< 30dB
};

/*!
  @brief MAX98090 MIC1_INPUT_LEVEL[PGAM1] Value: Microphone 1 Programmable Gain Amplifier Fine Adjust Configuration.
*/
enum MAX98090_BIT_MIC1_INPUT_LEVEL_PGAM1_VAL {
    MAX98090_BIT_MIC1_INPUT_LEVEL_PGAM1_0DBF    = 0x1F, //!< 0dB
    MAX98090_BIT_MIC1_INPUT_LEVEL_PGAM1_0DBE    = 0x1E, //!< 0dB
    MAX98090_BIT_MIC1_INPUT_LEVEL_PGAM1_0DBD    = 0x1D, //!< 0dB
    MAX98090_BIT_MIC1_INPUT_LEVEL_PGAM1_0DBC    = 0x1C, //!< 0dB
    MAX98090_BIT_MIC1_INPUT_LEVEL_PGAM1_0DBB    = 0x1B, //!< 0dB
    MAX98090_BIT_MIC1_INPUT_LEVEL_PGAM1_0DBA    = 0x1A, //!< 0dB
    MAX98090_BIT_MIC1_INPUT_LEVEL_PGAM1_0DB9    = 0x19, //!< 0dB
    MAX98090_BIT_MIC1_INPUT_LEVEL_PGAM1_0DB8    = 0x18, //!< 0dB
    MAX98090_BIT_MIC1_INPUT_LEVEL_PGAM1_0DB7    = 0x17, //!< 0dB
    MAX98090_BIT_MIC1_INPUT_LEVEL_PGAM1_0DB6    = 0x16, //!< 0dB
    MAX98090_BIT_MIC1_INPUT_LEVEL_PGAM1_0DB5    = 0x15, //!< 0dB
    MAX98090_BIT_MIC1_INPUT_LEVEL_PGAM1_0DB4    = 0x14, //!< 0dB
    MAX98090_BIT_MIC1_INPUT_LEVEL_PGAM1_1DB     = 0x13, //!< 1dB
    MAX98090_BIT_MIC1_INPUT_LEVEL_PGAM1_2DB     = 0x12, //!< 2dB
    MAX98090_BIT_MIC1_INPUT_LEVEL_PGAM1_3DB     = 0x11, //!< 3dB
    MAX98090_BIT_MIC1_INPUT_LEVEL_PGAM1_4DB     = 0x10, //!< 4dB
    MAX98090_BIT_MIC1_INPUT_LEVEL_PGAM1_5DB     = 0x0F, //!< 5dB
    MAX98090_BIT_MIC1_INPUT_LEVEL_PGAM1_6DB     = 0x0E, //!< 6dB
    MAX98090_BIT_MIC1_INPUT_LEVEL_PGAM1_7DB     = 0x0D, //!< 7dB
    MAX98090_BIT_MIC1_INPUT_LEVEL_PGAM1_8DB     = 0x0C, //!< 8dB
    MAX98090_BIT_MIC1_INPUT_LEVEL_PGAM1_9DB     = 0x0B, //!< 9dB
    MAX98090_BIT_MIC1_INPUT_LEVEL_PGAM1_10DB    = 0x0A, //!< 10dB
    MAX98090_BIT_MIC1_INPUT_LEVEL_PGAM1_11DB    = 0x09, //!< 11dB
    MAX98090_BIT_MIC1_INPUT_LEVEL_PGAM1_12DB    = 0x08, //!< 12dB
    MAX98090_BIT_MIC1_INPUT_LEVEL_PGAM1_13DB    = 0x07, //!< 13dB
    MAX98090_BIT_MIC1_INPUT_LEVEL_PGAM1_14DB    = 0x06, //!< 14dB
    MAX98090_BIT_MIC1_INPUT_LEVEL_PGAM1_15DB    = 0x05, //!< 15dB
    MAX98090_BIT_MIC1_INPUT_LEVEL_PGAM1_16DB    = 0x04, //!< 16dB
    MAX98090_BIT_MIC1_INPUT_LEVEL_PGAM1_17DB    = 0x03, //!< 17dB
    MAX98090_BIT_MIC1_INPUT_LEVEL_PGAM1_18DB    = 0x02, //!< 18dB
    MAX98090_BIT_MIC1_INPUT_LEVEL_PGAM1_19DB    = 0x01, //!< 19dB
    MAX98090_BIT_MIC1_INPUT_LEVEL_PGAM1_20DB    = 0x00  //!< 20dB
};

/*!
  @brief MAX98090 MIC2_INPUT_LEVEL[PA2EN] Value: Enables Microphone 2 Input Amplifier and Selects the Coarse Gain Setting.
*/
enum MAX98090_BIT_MIC2_INPUT_LEVEL_PA2EN_VAL {
    MAX98090_BIT_MIC2_INPUT_LEVEL_PA2EN_DISABLED    = 0x00, //!< Disabled
    MAX98090_BIT_MIC2_INPUT_LEVEL_PA2EN_0DB         = 0x01, //!< 0dB
    MAX98090_BIT_MIC2_INPUT_LEVEL_PA2EN_20DB        = 0x02, //!< 20dB
    MAX98090_BIT_MIC2_INPUT_LEVEL_PA2EN_30DB        = 0x03  //!< 30dB
};

/*!
  @brief MAX98090 MIC2_INPUT_LEVEL[PGAM2] Value: Microphone 2 Programmable Gain Amplifier Fine Adjust Configuration.
*/
enum MAX98090_BIT_MIC2_INPUT_LEVEL_PGAM2_VAL {
    MAX98090_BIT_MIC2_INPUT_LEVEL_PGAM2_0DBF    = 0x1F, //!< 0dB
    MAX98090_BIT_MIC2_INPUT_LEVEL_PGAM2_0DBE    = 0x1E, //!< 0dB
    MAX98090_BIT_MIC2_INPUT_LEVEL_PGAM2_0DBD    = 0x1D, //!< 0dB
    MAX98090_BIT_MIC2_INPUT_LEVEL_PGAM2_0DBC    = 0x1C, //!< 0dB
    MAX98090_BIT_MIC2_INPUT_LEVEL_PGAM2_0DBB    = 0x1B, //!< 0dB
    MAX98090_BIT_MIC2_INPUT_LEVEL_PGAM2_0DBA    = 0x1A, //!< 0dB
    MAX98090_BIT_MIC2_INPUT_LEVEL_PGAM2_0DB9    = 0x19, //!< 0dB
    MAX98090_BIT_MIC2_INPUT_LEVEL_PGAM2_0DB8    = 0x18, //!< 0dB
    MAX98090_BIT_MIC2_INPUT_LEVEL_PGAM2_0DB7    = 0x17, //!< 0dB
    MAX98090_BIT_MIC2_INPUT_LEVEL_PGAM2_0DB6    = 0x16, //!< 0dB
    MAX98090_BIT_MIC2_INPUT_LEVEL_PGAM2_0DB5    = 0x15, //!< 0dB
    MAX98090_BIT_MIC2_INPUT_LEVEL_PGAM2_0DB4    = 0x14, //!< 0dB
    MAX98090_BIT_MIC2_INPUT_LEVEL_PGAM2_1DB     = 0x13, //!< 1dB
    MAX98090_BIT_MIC2_INPUT_LEVEL_PGAM2_2DB     = 0x12, //!< 2dB
    MAX98090_BIT_MIC2_INPUT_LEVEL_PGAM2_3DB     = 0x11, //!< 3dB
    MAX98090_BIT_MIC2_INPUT_LEVEL_PGAM2_4DB     = 0x10, //!< 4dB
    MAX98090_BIT_MIC2_INPUT_LEVEL_PGAM2_5DB     = 0x0F, //!< 5dB
    MAX98090_BIT_MIC2_INPUT_LEVEL_PGAM2_6DB     = 0x0E, //!< 6dB
    MAX98090_BIT_MIC2_INPUT_LEVEL_PGAM2_7DB     = 0x0D, //!< 7dB
    MAX98090_BIT_MIC2_INPUT_LEVEL_PGAM2_8DB     = 0x0C, //!< 8dB
    MAX98090_BIT_MIC2_INPUT_LEVEL_PGAM2_9DB     = 0x0B, //!< 9dB
    MAX98090_BIT_MIC2_INPUT_LEVEL_PGAM2_10DB    = 0x0A, //!< 10dB
    MAX98090_BIT_MIC2_INPUT_LEVEL_PGAM2_11DB    = 0x09, //!< 11dB
    MAX98090_BIT_MIC2_INPUT_LEVEL_PGAM2_12DB    = 0x08, //!< 12dB
    MAX98090_BIT_MIC2_INPUT_LEVEL_PGAM2_13DB    = 0x07, //!< 13dB
    MAX98090_BIT_MIC2_INPUT_LEVEL_PGAM2_14DB    = 0x06, //!< 14dB
    MAX98090_BIT_MIC2_INPUT_LEVEL_PGAM2_15DB    = 0x05, //!< 15dB
    MAX98090_BIT_MIC2_INPUT_LEVEL_PGAM2_16DB    = 0x04, //!< 16dB
    MAX98090_BIT_MIC2_INPUT_LEVEL_PGAM2_17DB    = 0x03, //!< 17dB
    MAX98090_BIT_MIC2_INPUT_LEVEL_PGAM2_18DB    = 0x02, //!< 18dB
    MAX98090_BIT_MIC2_INPUT_LEVEL_PGAM2_19DB    = 0x01, //!< 19dB
    MAX98090_BIT_MIC2_INPUT_LEVEL_PGAM2_20DB    = 0x00  //!< 20dB
};

/*!
  @brief MAX98090 MIC_BIAS_VOLTAGE[MBVSEL] Value: Microphone Bias Level Configuration.
*/
enum MAX98090_BIT_MIC_BIAS_VOLTAGE_MBVSEL_VAL {
    MAX98090_BIT_MIC_BIAS_VOLTAGE_MBVSEL_2_2V = 0x00, //!< 2.2V
    MAX98090_BIT_MIC_BIAS_VOLTAGE_MBVSEL_2_4V = 0x01, //!< 2.4V
    MAX98090_BIT_MIC_BIAS_VOLTAGE_MBVSEL_2_6V = 0x02, //!< 2.6V
    MAX98090_BIT_MIC_BIAS_VOLTAGE_MBVSEL_2_8V = 0x03  //!< 2.8V
};

/*!
  @brief MAX98090 LEFT_ADC_LEVEL[AVLG] Value: Left ADC Digital Coarse Gain Configuration.
*/
enum MAX98090_BIT_LEFT_ADC_LEVEL_AVLG_VAL {
    MAX98090_BIT_LEFT_ADC_LEVEL_AVLG_0DB    = 0x00, //!< 0dB
    MAX98090_BIT_LEFT_ADC_LEVEL_AVLG_6DB    = 0x01, //!< +6dB
    MAX98090_BIT_LEFT_ADC_LEVEL_AVLG_12DB   = 0x02, //!< +12dB
    MAX98090_BIT_LEFT_ADC_LEVEL_AVLG_18DB   = 0x03  //!< +18dB
};

/*!
  @brief MAX98090 LEFT_ADC_LEVEL[AVL] Value: Left ADC Digital Fine Adjust Gain Configuration.
*/
enum MAX98090_BIT_LEFT_ADC_LEVEL_AVL_VAL {
    MAX98090_BIT_LEFT_ADC_LEVEL_AVL_3DB     = 0x00, //!< +3dB
    MAX98090_BIT_LEFT_ADC_LEVEL_AVL_2DB     = 0x01, //!< +2dB
    MAX98090_BIT_LEFT_ADC_LEVEL_AVL_1DB     = 0x02, //!< +1dB
    MAX98090_BIT_LEFT_ADC_LEVEL_AVL_0DB     = 0x03, //!< +0dB
    MAX98090_BIT_LEFT_ADC_LEVEL_AVL_M1DB    = 0x04, //!< -1dB
    MAX98090_BIT_LEFT_ADC_LEVEL_AVL_M2DB    = 0x05, //!< -2dB
    MAX98090_BIT_LEFT_ADC_LEVEL_AVL_M3DB    = 0x06, //!< -3dB
    MAX98090_BIT_LEFT_ADC_LEVEL_AVL_M4DB    = 0x07, //!< -4dB
    MAX98090_BIT_LEFT_ADC_LEVEL_AVL_M5DB    = 0x08, //!< -5dB
    MAX98090_BIT_LEFT_ADC_LEVEL_AVL_M6DB    = 0x09, //!< -6dB
    MAX98090_BIT_LEFT_ADC_LEVEL_AVL_M7DB    = 0x0A, //!< -7dB
    MAX98090_BIT_LEFT_ADC_LEVEL_AVL_M8DB    = 0x0B, //!< -8dB
    MAX98090_BIT_LEFT_ADC_LEVEL_AVL_M9DB    = 0x0C, //!< -9dB
    MAX98090_BIT_LEFT_ADC_LEVEL_AVL_M10DB   = 0x0D, //!< -10dB
    MAX98090_BIT_LEFT_ADC_LEVEL_AVL_M11DB   = 0x0E, //!< -11dB
    MAX98090_BIT_LEFT_ADC_LEVEL_AVL_M12DB   = 0x0F  //!< -12dB
};

/*!
  @brief MAX98090 RIGHT_ADC_LEVEL[AVRG] Value: ADC Gain Control.
*/
enum MAX98090_BIT_RIGHT_ADC_LEVEL_AVRG_VAL {
    MAX98090_BIT_RIGHT_ADC_LEVEL_AVRG_0DB   = 0x00, //!< 0dB
    MAX98090_BIT_RIGHT_ADC_LEVEL_AVRG_6DB   = 0x01, //!< +6dB
    MAX98090_BIT_RIGHT_ADC_LEVEL_AVRG_12DB  = 0x02, //!< +12dB
    MAX98090_BIT_RIGHT_ADC_LEVEL_AVRG_18DB  = 0x03  //!< +18dB
};

/*!
  @brief MAX98090 RIGHT_ADC_LEVEL[AVR] Value: ADC Level Control.
*/
enum MAX98090_BIT_RIGHT_ADC_LEVEL_AVR_VAL {
    MAX98090_BIT_RIGHT_ADC_LEVEL_AVR_3DB    = 0x00, //!< +3dB
    MAX98090_BIT_RIGHT_ADC_LEVEL_AVR_2DB    = 0x01, //!< +2dB
    MAX98090_BIT_RIGHT_ADC_LEVEL_AVR_1DB    = 0x02, //!< +1dB
    MAX98090_BIT_RIGHT_ADC_LEVEL_AVR_0DB    = 0x03, //!< +0dB
    MAX98090_BIT_RIGHT_ADC_LEVEL_AVR_M1DB   = 0x04, //!< -1dB
    MAX98090_BIT_RIGHT_ADC_LEVEL_AVR_M2DB   = 0x05, //!< -2dB
    MAX98090_BIT_RIGHT_ADC_LEVEL_AVR_M3DB   = 0x06, //!< -3dB
    MAX98090_BIT_RIGHT_ADC_LEVEL_AVR_M4DB   = 0x07, //!< -4dB
    MAX98090_BIT_RIGHT_ADC_LEVEL_AVR_M5DB   = 0x08, //!< -5dB
    MAX98090_BIT_RIGHT_ADC_LEVEL_AVR_M6DB   = 0x09, //!< -6dB
    MAX98090_BIT_RIGHT_ADC_LEVEL_AVR_M7DB   = 0x0A, //!< -7dB
    MAX98090_BIT_RIGHT_ADC_LEVEL_AVR_M8DB   = 0x0B, //!< -8dB
    MAX98090_BIT_RIGHT_ADC_LEVEL_AVR_M9DB   = 0x0C, //!< -9dB
    MAX98090_BIT_RIGHT_ADC_LEVEL_AVR_M10DB  = 0x0D, //!< -10dB
    MAX98090_BIT_RIGHT_ADC_LEVEL_AVR_M11DB  = 0x0E, //!< -11dB
    MAX98090_BIT_RIGHT_ADC_LEVEL_AVR_M12DB  = 0x0F  //!< -12dB
};

/*!
  @brief MAX98090 ADC_BIQUAD_LEVEL[AVBQ] Value: ADC Biquad Digital Preamplifier Gain Configuration.
*/
enum MAX98090_BIT_ADC_BIQUAD_LEVEL_AVBQ_VAL {
    MAX98090_BIT_ADC_BIQUAD_LEVEL_AVBQ_0DB      = 0x00, //!< +0dB
    MAX98090_BIT_ADC_BIQUAD_LEVEL_AVBQ_M1DB     = 0x01, //!< -1dB
    MAX98090_BIT_ADC_BIQUAD_LEVEL_AVBQ_M2DB     = 0x02, //!< -2dB
    MAX98090_BIT_ADC_BIQUAD_LEVEL_AVBQ_M3DB     = 0x03, //!< -3dB
    MAX98090_BIT_ADC_BIQUAD_LEVEL_AVBQ_M4DB     = 0x04, //!< -4dB
    MAX98090_BIT_ADC_BIQUAD_LEVEL_AVBQ_M5DB     = 0x05, //!< -5dB
    MAX98090_BIT_ADC_BIQUAD_LEVEL_AVBQ_M6DB     = 0x06, //!< -6dB
    MAX98090_BIT_ADC_BIQUAD_LEVEL_AVBQ_M7DB     = 0x07, //!< -7dB
    MAX98090_BIT_ADC_BIQUAD_LEVEL_AVBQ_M8DB     = 0x08, //!< -8dB
    MAX98090_BIT_ADC_BIQUAD_LEVEL_AVBQ_M9DB     = 0x09, //!< -9dB
    MAX98090_BIT_ADC_BIQUAD_LEVEL_AVBQ_M10DB    = 0x0A, //!< -10dB
    MAX98090_BIT_ADC_BIQUAD_LEVEL_AVBQ_M11DB    = 0x0B, //!< -11dB
    MAX98090_BIT_ADC_BIQUAD_LEVEL_AVBQ_M12DB    = 0x0C, //!< -12dB
    MAX98090_BIT_ADC_BIQUAD_LEVEL_AVBQ_M13DB    = 0x0D, //!< -13dB
    MAX98090_BIT_ADC_BIQUAD_LEVEL_AVBQ_M14DB    = 0x0E, //!< -14dB
    MAX98090_BIT_ADC_BIQUAD_LEVEL_AVBQ_M15DB    = 0x0F  //!< -15dB
};

/*!
  @brief MAX98090 ADC_SIDETONE[DSTS] Value: ADC Sidetone Enable and Digital Source Configuration.
*/
enum MAX98090_BIT_ADC_SIDETONE_DSTS_VAL {
    MAX98090_BIT_ADC_SIDETONE_DSTS_NO_SIDERONE      = 0x00, //!< No sidetone selected.
    MAX98090_BIT_ADC_SIDETONE_DSTS_LEFT_ADC         = 0x01, //!< Left ADC,
    MAX98090_BIT_ADC_SIDETONE_DSTS_RIGHT_ADC        = 0x02, //!< Right ADC,
    MAX98090_BIT_ADC_SIDETONE_DSTS_LEFT_RIGHT_ADC   = 0x03  //!< Left + Right ADC.
};

/*!
  @brief MAX98090 ADC_SIDETONE[DVST] Value: ADC Sidetone Digital Gain Configuration.
*/
enum MAX98090_BIT_ADC_SIDETONE_DVST_VAL {
    MAX98090_BIT_ADC_SIDETONE_DVST_0FF      = 0x00, //!< OFF
    MAX98090_BIT_ADC_SIDETONE_DVST_M0_5DB   = 0x01, //!< -0.5dB
    MAX98090_BIT_ADC_SIDETONE_DVST_M2_5DB   = 0x02, //!< -2.5dB
    MAX98090_BIT_ADC_SIDETONE_DVST_M4_5DB   = 0x03, //!< -4.5dB
    MAX98090_BIT_ADC_SIDETONE_DVST_M6_5DB   = 0x04, //!< -6.5dB
    MAX98090_BIT_ADC_SIDETONE_DVST_M8_5DB   = 0x05, //!< -8.5dB
    MAX98090_BIT_ADC_SIDETONE_DVST_M10_5DB  = 0x06, //!< -10.5dB
    MAX98090_BIT_ADC_SIDETONE_DVST_M12_5DB  = 0x07, //!< -12.5dB
    MAX98090_BIT_ADC_SIDETONE_DVST_M14_5DB  = 0x08, //!< -14.5dB
    MAX98090_BIT_ADC_SIDETONE_DVST_M16_5DB  = 0x09, //!< -16.5dB
    MAX98090_BIT_ADC_SIDETONE_DVST_M18_5DB  = 0x0A, //!< -18.5dB
    MAX98090_BIT_ADC_SIDETONE_DVST_M20_5DB  = 0x0B, //!< -20.5dB
    MAX98090_BIT_ADC_SIDETONE_DVST_M22_5DB  = 0x0C, //!< -22.5dB
    MAX98090_BIT_ADC_SIDETONE_DVST_M24_5DB  = 0x0D, //!< -24.5dB
    MAX98090_BIT_ADC_SIDETONE_DVST_M26_5DB  = 0x0E, //!< -26.5dB
    MAX98090_BIT_ADC_SIDETONE_DVST_M28_5DB  = 0x0F, //!< -28.5dB
    MAX98090_BIT_ADC_SIDETONE_DVST_M30_5DB  = 0x10, //!< -30.5dB
    MAX98090_BIT_ADC_SIDETONE_DVST_M32_5DB  = 0x11, //!< -32.5dB
    MAX98090_BIT_ADC_SIDETONE_DVST_M34_5DB  = 0x12, //!< -34.5dB
    MAX98090_BIT_ADC_SIDETONE_DVST_M36_5DB  = 0x13, //!< -36.5dB
    MAX98090_BIT_ADC_SIDETONE_DVST_M38_5DB  = 0x14, //!< -38.5dB
    MAX98090_BIT_ADC_SIDETONE_DVST_M40_5DB  = 0x15, //!< -40.5dB
    MAX98090_BIT_ADC_SIDETONE_DVST_M42_5DB  = 0x16, //!< -42.5dB
    MAX98090_BIT_ADC_SIDETONE_DVST_M44_5DB  = 0x17, //!< -44.5dB
    MAX98090_BIT_ADC_SIDETONE_DVST_M46_5DB  = 0x18, //!< -46.5dB
    MAX98090_BIT_ADC_SIDETONE_DVST_M48_5DB  = 0x19, //!< -48.5dB
    MAX98090_BIT_ADC_SIDETONE_DVST_M50_5DB  = 0x1A, //!< -50.5dB
    MAX98090_BIT_ADC_SIDETONE_DVST_M52_5DB  = 0x1B, //!< -52.5dB
    MAX98090_BIT_ADC_SIDETONE_DVST_M54_5DB  = 0x1C, //!< -54.5dB
    MAX98090_BIT_ADC_SIDETONE_DVST_M56_5DB  = 0x1D, //!< -56.5dB
    MAX98090_BIT_ADC_SIDETONE_DVST_M58_5DB  = 0x1E, //!< -58.5dB
    MAX98090_BIT_ADC_SIDETONE_DVST_M60_5DB  = 0x1F  //!< -60.5dB
};

/*!
  @brief MAX98090 SYSTEM_CLOCK[PSCLK] Value: Master Clock (MCLK) Prescaler Configuration.
*/
enum MAX98090_BIT_SYSTEM_CLOCK_PSCLK_VAL {
    MAX98090_BIT_SYSTEM_CLOCK_PSCLK_DISABLED    = 0x00, //!< Disabled.
    MAX98090_BIT_SYSTEM_CLOCK_PSCLK_MCLK_1      = 0x01, //!< MCLK/1, 10MHz <= MCLK <= 20MHz.
    MAX98090_BIT_SYSTEM_CLOCK_PSCLK_MCLK_2      = 0x02, //!< MCLK/2, 20MHz < MCLK <= 40MHz.
    MAX98090_BIT_SYSTEM_CLOCK_PSCLK_MCLK_4      = 0x03  //!< MCLK/4, 40MHz < MCLK <= 60MHz.
};

/*!
  @brief MAX98090 CLOCK_MODE[USE_MI1] Value: Set the PLL to use MI1[15:0] to set a More Accurate Frequency Ratio.
*/
enum MAX98090_BIT_CLOCK_MODE_USE_MI1_VAL {
    MAX98090_BIT_CLOCK_MODE_USE_MI1_MODE1 = 0, //!< M = 65536.
    MAX98090_BIT_CLOCK_MODE_USE_MI1_MODE2 = 1   //!< M is set by PLLM register.
};

/*!
  @brief MAX98090 MASTER_MODE[MAS] Value: Master Mode Enable.
*/
enum MAX98090_BIT_MASTER_MODE_MAS_VAL {
    MAX98090_BIT_MASTER_MODE_MAS_MASTER_MODE    = 1,//!< Master mode.
    MAX98090_BIT_MASTER_MODE_MAS_SLAVE_MODE     = 0 //!< Slave mode.
};

/*!
  @brief MAX98090 MASTER_MODE[BSEL] Value: Bit Clock (BCLK) Rate Configuration (Master Mode Only).
*/
enum MAX98090_BIT_MASTER_MODE_BSEL_VAL {
    MAX98090_BIT_MASTER_MODE_BSEL_OFF       = 0x00, //!< off
    MAX98090_BIT_MASTER_MODE_BSEL_32FS      = 0x01, //!< 32*fs
    MAX98090_BIT_MASTER_MODE_BSEL_48FS      = 0x02, //!< 48*fs
    MAX98090_BIT_MASTER_MODE_BSEL_64FS      = 0x03, //!< 64*fs
    MAX98090_BIT_MASTER_MODE_BSEL_PCLK2     = 0x04, //!< PCLK / 2
    MAX98090_BIT_MASTER_MODE_BSEL_PCLK4     = 0x05, //!< PCLK / 4
    MAX98090_BIT_MASTER_MODE_BSEL_PCLK8     = 0x06, //!< PCLK / 8
    MAX98090_BIT_MASTER_MODE_BSEL_PCLK16    = 0x07  //!< PCLK / 16
};

/*!
  @brief MAX98090 INTERFACE_FORMAT[RJ] Value: Configures the DAI for Right Justified Mode (No Data Delay).
*/
enum MAX98090_BIT_INTERFACE_FORMAT_RJ_VAL {
    MAX98090_BIT_INTERFACE_FORMAT_RJ_LEFT   = 0,//!< left justified mode with optional data delay.
    MAX98090_BIT_INTERFACE_FORMAT_RJ_RIGHT  = 1 //!< right justified mode.
};

/*!
  @brief MAX98090 INTERFACE_FORMAT[WCI1] Value: Configures the DAI for Frame Clock (LRCLK) Inversion.
*/
enum MAX98090_BIT_INTERFACE_FORMAT_WCI1_VAL {
    MAX98090_BIT_INTERFACE_FORMAT_WCI1_RIGHT = 0,   //!< Right-channel data is transmitted while LRCLK is low.
    MAX98090_BIT_INTERFACE_FORMAT_WCI1_LEFT  = 1    //!< Left-channel data is transmitted while LRCLK is low.
};

/*!
  @brief MAX98090 INTERFACE_FORMAT[WCI2] Value: Configures the DAI for Frame Clock (LRCLK) Inversion.
*/
enum MAX98090_BIT_INTERFACE_FORMAT_WCI2_VAL {
    MAX98090_BIT_INTERFACE_FORMAT_WCI2_RISE = 0, //!< Start of a new frame is signified by the rising edge of the LRCLK pulse.
    MAX98090_BIT_INTERFACE_FORMAT_WCI2_FALL = 1  //!< Start of a new frame is signified by the falling edge of the LRCLK pulse.
};

/*!
  @brief MAX98090 INTERFACE_FORMAT[BCI1] Value: Configures the DAI for Bit Clock (BCLK) Inversion.
*/
enum MAX98090_BIT_INTERFACE_FORMAT_BCI1_VAL {
    MAX98090_BIT_INTERFACE_FORMAT_BCI1_FALL = 0, //!< SDIN is accepted on the falling edge of BCLK.
    MAX98090_BIT_INTERFACE_FORMAT_BCI1_RISE = 1  //!< SDIN is accepted on the rising edge of BCLK.
};

/*!
  @brief MAX98090 INTERFACE_FORMAT[BCI2] Value: Configures the DAI for Bit Clock (BCLK) Inversion.
*/
enum MAX98090_BIT_INTERFACE_FORMAT_BCI2_VAL {
    MAX98090_BIT_INTERFACE_FORMAT_BCI2_RISE = 0, //!< Master Mode:LRCLK transitions occur on the rising edge of BCLK.
    MAX98090_BIT_INTERFACE_FORMAT_BCI2_FALL = 1  //!< Master Mode:LRCLK transitions occur on the falling edge of BCLK.
};

/*!
  @brief MAX98090 INTERFACE_FORMAT[DLY] Value: Configures the DAI for Data Delay (I2S Standard).
*/
enum MAX98090_BIT_INTERFACE_FORMAT_DLY_VAL {
    MAX98090_BIT_INTERFACE_FORMAT_DLY_MODE1 = 1, //!< The most significant bit of an audio word is latched at the second BCLK edge after the LRCLK transition.
    MAX98090_BIT_INTERFACE_FORMAT_DLY_MODE2 = 0  //!< The most significant bit of an audio word is latched at the first BCLK edge after the LRCLK transition.
};

/*!
  @brief MAX98090 INTERFACE_FORMAT[WS] Value: DAI Input Data Word Size.
*/
enum MAX98090_BIT_INTERFACE_FORMAT_WS1_VAL {
    MAX98090_BIT_INTERFACE_FORMAT_WS1_MODE1 = 0x00, //!< 16-bits
    MAX98090_BIT_INTERFACE_FORMAT_WS1_MODE2 = 0x01, //!< 20-bits
    MAX98090_BIT_INTERFACE_FORMAT_WS1_MODE3 = 0x02, //!< 24-bits
    MAX98090_BIT_INTERFACE_FORMAT_WS1_MODE4 = 0x03, //!< Reserved
};

/*!
  @brief MAX98090 INTERFACE_FORMAT[WS] Value: DAI Input Data Word Size.
*/
enum MAX98090_BIT_INTERFACE_FORMAT_WS2_VAL {
    MAX98090_BIT_INTERFACE_FORMAT_WS2_MODE1 = 0x00, //!< 16-bits
    MAX98090_BIT_INTERFACE_FORMAT_WS2_MODE2 = 0x01, //!< 20-bits
    MAX98090_BIT_INTERFACE_FORMAT_WS2_MODE3 = 0x02, //!< 20-bits
    MAX98090_BIT_INTERFACE_FORMAT_WS2_MODE4 = 0x03  //!< 20-bits
};

/*!
  @brief MAX98090 TDM_FORMAT_1[FSW] Value: Configures the DAI Frame Sync Pulse Width.
*/
enum MAX98090_BIT_TDM_FORMAT_1_FSW_VAL {
    MAX98090_BIT_TDM_FORMAT_1_FSW_MODE1 = 1, //!< Frame sync pulse extended to the width of the entire data word.
    MAX98090_BIT_TDM_FORMAT_1_FSW_MODE2 = 0  //!< Frame sync pulse is one bit wide.
};

/*!
  @brief MAX98090 TDM_FORMAT_1[TDM] Value: Enables the DAI for Time Division Multiplex (TDM) Mode.
*/
enum MAX98090_BIT_TDM_FORMAT_1_TDM_VAL {
    MAX98090_BIT_TDM_FORMAT_1_TDM_ENABLES  = 1, //!< Enables time-division multiplex mode and configures the audio interface to accept PCM data.
    MAX98090_BIT_TDM_FORMAT_1_TDM_DISABLES = 0  //!< Disables time-division multiplex mode.
};

/*!
  @brief MAX98090 TDM_FORMAT_2[SLOTL] Value: Selects the Time Slot to use for Left Channel Data in TDM Mode.
*/
enum MAX98090_BIT_TDM_FORMAT_2_SLOTL_VAL {
    MAX98090_BIT_TDM_FORMAT_2_SLOTL_1 = 0x00, //!< Time Slot 1
    MAX98090_BIT_TDM_FORMAT_2_SLOTL_2 = 0x01, //!< Time Slot 2
    MAX98090_BIT_TDM_FORMAT_2_SLOTL_3 = 0x02, //!< Time Slot 3
    MAX98090_BIT_TDM_FORMAT_2_SLOTL_4 = 0x03  //!< Time Slot 4
};

/*!
  @brief MAX98090 TDM_FORMAT_2[SLOTR] Value: Selects the Time Slot to use for Right Channel Data in TDM Mode.
*/
enum MAX98090_BIT_TDM_FORMAT_2_SLOTR_VAL {
    MAX98090_BIT_TDM_FORMAT_2_SLOTR_1 = 0x00, //!< Time Slot 1
    MAX98090_BIT_TDM_FORMAT_2_SLOTR_2 = 0x01, //!< Time Slot 2
    MAX98090_BIT_TDM_FORMAT_2_SLOTR_3 = 0x02, //!< Time Slot 3
    MAX98090_BIT_TDM_FORMAT_2_SLOTR_4 = 0x03  //!< Time Slot 4
};

/*!
  @brief MAX98090 I_OR_O_CONFIGURATION[LTEN] Value: Enables Data Loop Through from the ADC Output to the DAC Input.
*/
enum MAX98090_BIT_I_OR_O_CONFIGURATION_LTEN_VAL {
    MAX98090_BIT_I_OR_O_CONFIGURATION_LTEN_ENABLED  = 1, //!< ADC to DAC loop-through enabled.
    MAX98090_BIT_I_OR_O_CONFIGURATION_LTEN_DISABLED = 0  //!< ADC to DAC loop-through disabled.
};

/*!
  @brief MAX98090 I_OR_O_CONFIGURATION[LBEN] Value: Enables Data Loop Back from the SDIEN Input to the SDOEN Output.
*/
enum MAX98090_BIT_I_OR_O_CONFIGURATION_LBEN_VAL {
    MAX98090_BIT_I_OR_O_CONFIGURATION_LBEN_DAI_SDIN = 1, //!< DAI SDIN used as SDOUT data source.
    MAX98090_BIT_I_OR_O_CONFIGURATION_LBEN_ADC      = 0  //!< ADC used as SDOUT data source.
};

/*!
  @brief MAX98090 I_OR_O_CONFIGURATION[DMONO] Value: Enables DAC Mono Mode where SDIN L/R are Mixed and Input to DAC L/R.
*/
enum MAX98090_BIT_I_OR_O_CONFIGURATION_DMONO_VAL {
    MAX98090_BIT_I_OR_O_CONFIGURATION_DMONO_MODE1 = 1, //!< The left and right channel SDIN input data are mixed together and input to both the left and right DAC channel signal paths.
    MAX98090_BIT_I_OR_O_CONFIGURATION_DMONO_MODE2 = 0  //!< The SDIN DAC input data is treated as left/right stereo signal data processed separately.
};

/*!
  @brief MAX98090 I_OR_O_CONFIGURATION[HIZOFF] Value: Disables Hi-Z Mode for SDOUT.
*/
enum MAX98090_BIT_I_OR_O_CONFIGURATION_HIZOFF_VAL {
    MAX98090_BIT_I_OR_O_CONFIGURATION_HIZOFF_MODE1 = 1, //!< The SDOUT pin drives a valid logic level after all data bits have been transferred out of the part.
    MAX98090_BIT_I_OR_O_CONFIGURATION_HIZOFF_MODE2 = 0  //!< The SDOUT pin goes to a high impedance state after all 16 ADC data bits have been transferred out of the part, allowing the SDOUT line to be shared to the destination by other devices.
};

/*!
  @brief MAX98090 I_OR_O_CONFIGURATION[SDOEN] Value: Enables the Serial Data Output.
*/
enum MAX98090_BIT_I_OR_O_CONFIGURATION_SDOEN_VAL {
    MAX98090_BIT_I_OR_O_CONFIGURATION_SDOEN_ENABLED  = 1, //!< Serial data output enabled.
    MAX98090_BIT_I_OR_O_CONFIGURATION_SDOEN_DISABLED = 0  //!< Serial data output disabled.
};

/*!
  @brief MAX98090 I_OR_O_CONFIGURATION[SDIEN] Value: Enables the Serial Data Input.
*/
enum MAX98090_BIT_I_OR_O_CONFIGURATION_SDIEN_VAL {
    MAX98090_BIT_I_OR_O_CONFIGURATION_SDIEN_ENABLED  = 1, //!< Serial data input enabled.
    MAX98090_BIT_I_OR_O_CONFIGURATION_SDIEN_DISABLED = 0  //!< Serial data input disabled.
};

/*!
  @brief MAX98090 FILTER_CONFIG[MODE] Value: Enables the CODEC DSP FIR Music filters (Default IIR Voice Filters).
*/
enum MAX98090_BIT_FILTER_CONFIG_MODE_VAL {
    MAX98090_BIT_FILTER_CONFIG_MODE_MODE1 = 0, //!< The CODEC DSP filters operate in IIR Voice mode with stopband frequencies below the fs/2 Nyquist rate.
    MAX98090_BIT_FILTER_CONFIG_MODE_MODE2 = 1  //!< The CODEC DSP filters operate in a linear phase FIR Audio mode with optional DC blocking that may be enabled using the AHPF and DHPF I2C bits.
};

/*!
  @brief MAX98090 FILTER_CONFIG[AHPF] Value: Enables the ADC DC Blocking Filter (FIR Only).
*/
enum MAX98090_BIT_FILTER_CONFIG_AHPF_VAL {
    MAX98090_BIT_FILTER_CONFIG_AHPF_DISABLED = 0, //!< DC blocking filter disabled.
    MAX98090_BIT_FILTER_CONFIG_AHPF_ENABLED  = 1  //!< DC blocking filter enabled.
};

/*!
  @brief MAX98090 FILTER_CONFIG[DHPF] Value: Enables the DAC DC Blocking Filter (FIR Only).
*/
enum MAX98090_BIT_FILTER_CONFIG_DHPF_VAL {
    MAX98090_BIT_FILTER_CONFIG_DHPF_DISABLED = 0, //!< DC blocking filter disabled.
    MAX98090_BIT_FILTER_CONFIG_DHPF_ENABLED  = 1  //!< DC blocking filter enabled.
};

/*!
  @brief MAX98090 FILTER_CONFIG[DHF] Value: Enables the DAC High Sample Rate Mode (LRCLK > 50kHz, FIR Only).
*/
enum MAX98090_BIT_FILTER_CONFIG_DHF_VAL {
    MAX98090_BIT_FILTER_CONFIG_DHF_GREATER = 0, //!< LRCLK is greater than 50kHz.
    MAX98090_BIT_FILTER_CONFIG_DHF_LESS    = 1  //!< LRCLK is less than 50kHz.
};

/*!
  @brief MAX98090 PLAYBACK_LEVEL_1[DV1G] Value: DAI Digital Input Coarse Adjust Gain Configuration.
*/
enum MAX98090_BIT_PLAYBACK_LEVEL_1_DV1G_VAL {
    MAX98090_BIT_PLAYBACK_LEVEL_1_DV1G_0DB  = 0x00, //!< 0dB
    MAX98090_BIT_PLAYBACK_LEVEL_1_DV1G_6DB  = 0x01, //!< +6dB
    MAX98090_BIT_PLAYBACK_LEVEL_1_DV1G_12DB = 0x02, //!< +12dB
    MAX98090_BIT_PLAYBACK_LEVEL_1_DV1G_18DB = 0x03  //!< +18dB
};

/*!
  @brief MAX98090 PLAYBACK_LEVEL_1[DV1] Value: DAI Digital Input Fine Adjust Gain Configuration.
*/
enum MAX98090_BIT_PLAYBACK_LEVEL_1_DV1_VAL {
    MAX98090_BIT_PLAYBACK_LEVEL_1_DV1_0DB   = 0x00, //!< 0dB
    MAX98090_BIT_PLAYBACK_LEVEL_1_DV1_M1DB  = 0x01, //!< -1dB
    MAX98090_BIT_PLAYBACK_LEVEL_1_DV1_M2DB  = 0x02, //!< -2dB
    MAX98090_BIT_PLAYBACK_LEVEL_1_DV1_M3DB  = 0x03, //!< -3dB
    MAX98090_BIT_PLAYBACK_LEVEL_1_DV1_M4DB  = 0x04, //!< -4dB
    MAX98090_BIT_PLAYBACK_LEVEL_1_DV1_M5DB  = 0x05, //!< -5dB
    MAX98090_BIT_PLAYBACK_LEVEL_1_DV1_M6DB  = 0x06, //!< -6dB
    MAX98090_BIT_PLAYBACK_LEVEL_1_DV1_M7DB  = 0x07, //!< -7dB
    MAX98090_BIT_PLAYBACK_LEVEL_1_DV1_M8DB  = 0x08, //!< -8dB
    MAX98090_BIT_PLAYBACK_LEVEL_1_DV1_M9DB  = 0x09, //!< -9dB
    MAX98090_BIT_PLAYBACK_LEVEL_1_DV1_M10DB = 0x0A, //!< -10dB
    MAX98090_BIT_PLAYBACK_LEVEL_1_DV1_M11DB = 0x0B, //!< -11dB
    MAX98090_BIT_PLAYBACK_LEVEL_1_DV1_M12DB = 0x0C, //!< -12dB
    MAX98090_BIT_PLAYBACK_LEVEL_1_DV1_M13DB = 0x0D, //!< -13dB
    MAX98090_BIT_PLAYBACK_LEVEL_1_DV1_M14DB = 0x0E, //!< -14dB
    MAX98090_BIT_PLAYBACK_LEVEL_1_DV1_M15DB = 0x0F  //!< -15dB
};

/*!
  @brief MAX98090 PLAYBACK_LEVEL_2[EQCLP] Value: Enables DAI Digital Input Equalizer Clipping Detection.
*/
enum MAX98090_BIT_PLAYBACK_LEVEL_2_EQCLP_VAL {
    MAX98090_BIT_PLAYBACK_LEVEL_2_EQCLP_DISABLED = 1, //!< Equalizer Clip detect disabled.
    MAX98090_BIT_PLAYBACK_LEVEL_2_EQCLP_ENABLED  = 0  //!< Equalizer Clip detect enabled.
};

/*!
  @brief MAX98090 PLAYBACK_LEVEL_2[DVEQ] Value: DAI Digital Input Equalizer Attenuation Level Configuration.
*/
enum MAX98090_BIT_PLAYBACK_LEVEL_2_DVEQ_VAL {
    MAX98090_BIT_PLAYBACK_LEVEL_2_DVEQ_0DB      = 0x00, //!< +0dB
    MAX98090_BIT_PLAYBACK_LEVEL_2_DVEQ_M1DB     = 0x01, //!< -1dB
    MAX98090_BIT_PLAYBACK_LEVEL_2_DVEQ_M2DB     = 0x02, //!< -2dB
    MAX98090_BIT_PLAYBACK_LEVEL_2_DVEQ_M3DB     = 0x03, //!< -3dB
    MAX98090_BIT_PLAYBACK_LEVEL_2_DVEQ_M4DB     = 0x04, //!< -4dB
    MAX98090_BIT_PLAYBACK_LEVEL_2_DVEQ_M5DB     = 0x05, //!< -5dB
    MAX98090_BIT_PLAYBACK_LEVEL_2_DVEQ_M6DB     = 0x06, //!< -6dB
    MAX98090_BIT_PLAYBACK_LEVEL_2_DVEQ_M7DB     = 0x07, //!< -7dB
    MAX98090_BIT_PLAYBACK_LEVEL_2_DVEQ_M8DB     = 0x08, //!< -8dB
    MAX98090_BIT_PLAYBACK_LEVEL_2_DVEQ_M9DB     = 0x09, //!< -9dB
    MAX98090_BIT_PLAYBACK_LEVEL_2_DVEQ_M10DB    = 0x0A, //!< -10dB
    MAX98090_BIT_PLAYBACK_LEVEL_2_DVEQ_M11DB    = 0x0B, //!< -11dB
    MAX98090_BIT_PLAYBACK_LEVEL_2_DVEQ_M12DB    = 0x0C, //!< -12dB
    MAX98090_BIT_PLAYBACK_LEVEL_2_DVEQ_M13DB    = 0x0D, //!< -13dB
    MAX98090_BIT_PLAYBACK_LEVEL_2_DVEQ_M14DB    = 0x0E, //!< -14dB
    MAX98090_BIT_PLAYBACK_LEVEL_2_DVEQ_M15DB    = 0x0F  //!< -15dB
};

/*!
  @brief MAX98090 HP_CONTROL[MIXHPRSEL] Value: Select Headphone Mixer as Right Input Source (Default DAC Right Direct).
*/
enum MAX98090_BIT_HP_CONTROL_MIXHPRSEL_VAL {
    MAX98090_BIT_HP_CONTROL_MIXHPRSEL_DAC_ONLY          = 0, //!< DAC only source (best dynamic range and power consumption).
    MAX98090_BIT_HP_CONTROL_MIXHPRSEL_HEADPHONE_MIXER   = 1  //!< Headphone mixer source.
};

/*!
  @brief MAX98090 HP_CONTROL[MIXHPLSEL] Value: Select Headphone Mixer as Left Input Source (Default DAC Left Direct).
*/
enum MAX98090_BIT_HP_CONTROL_MIXHPLSEL_VAL {
    MAX98090_BIT_HP_CONTROL_MIXHPLSEL_DAC_ONLY          = 0, //!< DAC only source (best dynamic range and power consumption).
    MAX98090_BIT_HP_CONTROL_MIXHPLSEL_EADPHONE_MIXER    = 1  //!< Headphone mixer source.
};

/*!
  @brief MAX98090 HP_CONTROL[MIXHPRG] Value: Right Headphone Mixer Gain Configuration.
*/
enum MAX98090_BIT_HP_CONTROL_MIXHPRG_VAL {
    MAX98090_BIT_HP_CONTROL_MIXHPRG_0DB     = 0x0, //!< +0dB
    MAX98090_BIT_HP_CONTROL_MIXHPRG_M6DB    = 0x1, //!< -6dB
    MAX98090_BIT_HP_CONTROL_MIXHPRG_M9_5dB  = 0x2, //!< -9.5dB
    MAX98090_BIT_HP_CONTROL_MIXHPRG_M12dB   = 0x3   //!< -12dB
};

/*!
  @brief MAX98090 HP_CONTROL[MIXHPLG] Value: Left Headphone Mixer Gain Configuration.
*/
enum MAX98090_BIT_HP_CONTROL_MIXHPLG_VAL {
    MAX98090_BIT_HP_CONTROL_MIXHPLG_0DB     = 0x0, //!< +0dB
    MAX98090_BIT_HP_CONTROL_MIXHPLG_M6DB    = 0x1, //!< -6dB
    MAX98090_BIT_HP_CONTROL_MIXHPLG_M9_5dB  = 0x2, //!< -9.5dB
    MAX98090_BIT_HP_CONTROL_MIXHPLG_M12dB   = 0x3  //!< -12dB
};

/*!
  @brief MAX98090 LEFT_HP_VOLUME[HPLM] Value: Left Headphone Output Mute Enable.
*/
enum MAX98090_BIT_LEFT_HP_VOLUME_HPLM_VAL {
    MAX98090_BIT_LEFT_HP_VOLUME_HPLM_ENABLE  = 1, //!< Headphone output muted.
    MAX98090_BIT_LEFT_HP_VOLUME_HPLM_DISABLE = 0  //!< Headphone output volume set by the volume control bits.
};

/*!
  @brief MAX98090 LEFT_HP_VOLUME[HPVOLL] Value: Left Headphone Output Amplifier Volume Control Configuration.
*/
enum MAX98090_BIT_LEFT_HP_VOLUME_HPVOLL_VAL {
    MAX98090_BIT_LEFT_HP_VOLUME_HPVOLL_3DB      = 0x1F, //!< +3dB
    MAX98090_BIT_LEFT_HP_VOLUME_HPVOLL_2_5DB    = 0x1E, //!< +2.5dB
    MAX98090_BIT_LEFT_HP_VOLUME_HPVOLL_2DB      = 0x1D, //!< +2dB
    MAX98090_BIT_LEFT_HP_VOLUME_HPVOLL_1_5DB    = 0x1C, //!< +1.5dB
    MAX98090_BIT_LEFT_HP_VOLUME_HPVOLL_1DB      = 0x1B, //!< +1dB
    MAX98090_BIT_LEFT_HP_VOLUME_HPVOLL_0DB      = 0x1A, //!< +0dB
    MAX98090_BIT_LEFT_HP_VOLUME_HPVOLL_M1DB     = 0x19, //!< -1dB
    MAX98090_BIT_LEFT_HP_VOLUME_HPVOLL_M2DB     = 0x18, //!< -2dB
    MAX98090_BIT_LEFT_HP_VOLUME_HPVOLL_M3DB     = 0x17, //!< -3dB
    MAX98090_BIT_LEFT_HP_VOLUME_HPVOLL_M4DB     = 0x16, //!< -4dB
    MAX98090_BIT_LEFT_HP_VOLUME_HPVOLL_M5DB     = 0x15, //!< -5dB
    MAX98090_BIT_LEFT_HP_VOLUME_HPVOLL_M7DB     = 0x14, //!< -7dB
    MAX98090_BIT_LEFT_HP_VOLUME_HPVOLL_M9DB     = 0x13, //!< -9dB
    MAX98090_BIT_LEFT_HP_VOLUME_HPVOLL_M11DB    = 0x12, //!< -11dB
    MAX98090_BIT_LEFT_HP_VOLUME_HPVOLL_M13DB    = 0x11, //!< -13dB
    MAX98090_BIT_LEFT_HP_VOLUME_HPVOLL_M15DB    = 0x10, //!< -15dB
    MAX98090_BIT_LEFT_HP_VOLUME_HPVOLL_M17DB    = 0x0F, //!< -17dB
    MAX98090_BIT_LEFT_HP_VOLUME_HPVOLL_M19DB    = 0x0E, //!< -19dB
    MAX98090_BIT_LEFT_HP_VOLUME_HPVOLL_M22DB    = 0x0D, //!< -22dB
    MAX98090_BIT_LEFT_HP_VOLUME_HPVOLL_M25DB    = 0x0C, //!< -25dB
    MAX98090_BIT_LEFT_HP_VOLUME_HPVOLL_M28DB    = 0x0B, //!< -28dB
    MAX98090_BIT_LEFT_HP_VOLUME_HPVOLL_M31DB    = 0x0A, //!< -31dB
    MAX98090_BIT_LEFT_HP_VOLUME_HPVOLL_M34DB    = 0x09, //!< -34dB
    MAX98090_BIT_LEFT_HP_VOLUME_HPVOLL_M37DB    = 0x08, //!< -37dB
    MAX98090_BIT_LEFT_HP_VOLUME_HPVOLL_M40DB    = 0x07, //!< -40dB
    MAX98090_BIT_LEFT_HP_VOLUME_HPVOLL_M43DB    = 0x06, //!< -43dB
    MAX98090_BIT_LEFT_HP_VOLUME_HPVOLL_M51DB    = 0x04, //!< -51dB
    MAX98090_BIT_LEFT_HP_VOLUME_HPVOLL_M55DB    = 0x03, //!< -55dB
    MAX98090_BIT_LEFT_HP_VOLUME_HPVOLL_M59DB    = 0x02, //!< -59dB
    MAX98090_BIT_LEFT_HP_VOLUME_HPVOLL_M63DB    = 0x01, //!< -63dB
    MAX98090_BIT_LEFT_HP_VOLUME_HPVOLL_M67DB    = 0x00  //!< -67dB
};

/*!
  @brief MAX98090 RIGHT_HP_VOLUME[HPRM] Value: Right Headphone Output Mute Enable.
*/
enum MAX98090_BIT_RIGHT_HP_VOLUME_HPRM_VAL {
    MAX98090_BIT_RIGHT_HP_VOLUME_HPRM_ENABLE  = 1, //!< Headphone output muted.
    MAX98090_BIT_RIGHT_HP_VOLUME_HPRM_DISABLE = 0  //!< Headphone output volume set by the volume control bits.
};

/*!
  @brief MAX98090 RIGHT_HP_VOLUME[HPVOLR] Value: Right Headphone Output Amplifier Volume Control Configuration.
*/
enum MAX98090_BIT_RIGHT_HP_VOLUME_HPVOLR_VAL {
    MAX98090_BIT_RIGHT_HP_VOLUME_HPVOLR_3DB     = 0x1F, //!< +3dB
    MAX98090_BIT_RIGHT_HP_VOLUME_HPVOLR_2_5DB   = 0x1E, //!< +2.5dB
    MAX98090_BIT_RIGHT_HP_VOLUME_HPVOLR_2DB     = 0x1D, //!< +2dB
    MAX98090_BIT_RIGHT_HP_VOLUME_HPVOLR_1_5DB   = 0x1C, //!< +1.5dB
    MAX98090_BIT_RIGHT_HP_VOLUME_HPVOLR_1DB     = 0x1B, //!< +1dB
    MAX98090_BIT_RIGHT_HP_VOLUME_HPVOLR_0DB     = 0x1A, //!< +0dB
    MAX98090_BIT_RIGHT_HP_VOLUME_HPVOLR_M1DB    = 0x19, //!< -1dB
    MAX98090_BIT_RIGHT_HP_VOLUME_HPVOLR_M2DB    = 0x18, //!< -2dB
    MAX98090_BIT_RIGHT_HP_VOLUME_HPVOLR_M3DB    = 0x17, //!< -3dB
    MAX98090_BIT_RIGHT_HP_VOLUME_HPVOLR_M4DB    = 0x16, //!< -4dB
    MAX98090_BIT_RIGHT_HP_VOLUME_HPVOLR_M5DB    = 0x15, //!< -5dB
    MAX98090_BIT_RIGHT_HP_VOLUME_HPVOLR_M7DB    = 0x14, //!< -7dB
    MAX98090_BIT_RIGHT_HP_VOLUME_HPVOLR_M9DB    = 0x13, //!< -9dB
    MAX98090_BIT_RIGHT_HP_VOLUME_HPVOLR_M11DB   = 0x12, //!< -11dB
    MAX98090_BIT_RIGHT_HP_VOLUME_HPVOLR_M13DB   = 0x11, //!< -13dB
    MAX98090_BIT_RIGHT_HP_VOLUME_HPVOLR_M15DB   = 0x10, //!< -15dB
    MAX98090_BIT_RIGHT_HP_VOLUME_HPVOLR_M17DB   = 0x0F, //!< -17dB
    MAX98090_BIT_RIGHT_HP_VOLUME_HPVOLR_M19DB   = 0x0E, //!< -19dB
    MAX98090_BIT_RIGHT_HP_VOLUME_HPVOLR_M22DB   = 0x0D, //!< -22dB
    MAX98090_BIT_RIGHT_HP_VOLUME_HPVOLR_M25DB   = 0x0C, //!< -25dB
    MAX98090_BIT_RIGHT_HP_VOLUME_HPVOLR_M28DB   = 0x0B, //!< -28dB
    MAX98090_BIT_RIGHT_HP_VOLUME_HPVOLR_M31DB   = 0x0A, //!< -31dB
    MAX98090_BIT_RIGHT_HP_VOLUME_HPVOLR_M34DB   = 0x09, //!< -34dB
    MAX98090_BIT_RIGHT_HP_VOLUME_HPVOLR_M37DB   = 0x08, //!< -37dB
    MAX98090_BIT_RIGHT_HP_VOLUME_HPVOLR_M40DB   = 0x07, //!< -40dB
    MAX98090_BIT_RIGHT_HP_VOLUME_HPVOLR_M43DB   = 0x06, //!< -43dB
    MAX98090_BIT_RIGHT_HP_VOLUME_HPVOLR_M51DB   = 0x04, //!< -51dB
    MAX98090_BIT_RIGHT_HP_VOLUME_HPVOLR_M55DB   = 0x03, //!< -55dB
    MAX98090_BIT_RIGHT_HP_VOLUME_HPVOLR_M59DB   = 0x02, //!< -59dB
    MAX98090_BIT_RIGHT_HP_VOLUME_HPVOLR_M63DB   = 0x01, //!< -63dB
    MAX98090_BIT_RIGHT_HP_VOLUME_HPVOLR_M67DB   = 0x00  //!< -67dB
};

/*!
  @brief MAX98090 RIGHT_SPK_MIXER[SPK_SLAVE] Value: Speaker Slave Mode Enable.
*/
enum MAX98090_BIT_RIGHT_SPK_MIXER_SPK_SLAVE_VAL {
    MAX98090_BIT_RIGHT_SPK_MIXER_SPK_SLAVE_CLOCK   = 0, //!< right channel clock always generated independently.
    MAX98090_BIT_RIGHT_SPK_MIXER_SPK_SLAVE_SPEAKER = 1  //!< right channel speaker uses left channel speaker clock if both speaker channels are enabled 5
};

/*!
  @brief MAX98090 SPK_CONTROL[MIXSPRG] Value: Right Speaker Mixer Gain Configuration.
*/
enum MAX98090_BIT_SPK_CONTROL_MIXSPRG_VAL {
    MAX98090_BIT_SPK_CONTROL_MIXSPRG_0DB    = 0x00, //!< +0dB
    MAX98090_BIT_SPK_CONTROL_MIXSPRG_M6DB   = 0x01, //!< -6dB
    MAX98090_BIT_SPK_CONTROL_MIXSPRG_M9_5DB = 0x02, //!< -9.5dB
    MAX98090_BIT_SPK_CONTROL_MIXSPRG_M21DB  = 0x03  //!< -12dB
};

/*!
  @brief MAX98090 SPK_CONTROL[MIXSPLG] Value: Left Speaker Mixer Gain Configuration.
*/
enum MAX98090_BIT_SPK_CONTROL_MIXSPLG_VAL {
    MAX98090_BIT_SPK_CONTROL_MIXSPLG_0DB    = 0x00, //!< +0dB
    MAX98090_BIT_SPK_CONTROL_MIXSPLG_M6DB   = 0x01, //!< -6dB
    MAX98090_BIT_SPK_CONTROL_MIXSPLG_M9_5dB = 0x02, //!< -9.5dB
    MAX98090_BIT_SPK_CONTROL_MIXSPLG_M12dB  = 0x03  //!< -12dB
};

/*!
  @brief MAX98090 LEFT_SPK_VOLUME[SPLM] Value: Left Speaker Output Mute Enable.
*/
enum MAX98090_BIT_LEFT_SPK_VOLUME_SPLM_VAL {
    MAX98090_BIT_LEFT_SPK_VOLUME_SPLM_MUTED     = 1, //!< Left Speaker output muted.
    MAX98090_BIT_LEFT_SPK_VOLUME_SPLM_VOLUME    = 0  //!< Speaker output volume set by the volume control bits.
};

/*!
  @brief MAX98090 LEFT_SPK_VOLUME[SPVOLL] Value: Left Speaker Output Amplifier Volume Control Configuration.
*/
enum MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL_VAL {
    MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL_14DB    = 0x3F, //!< +14dB
    MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL_13_5DB  = 0x3E, //!< +13.5dB
    MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL_13DB    = 0x3D, //!< +13dB
    MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL_12_5DB  = 0x3C, //!< +12.5dB
    MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL_12DB    = 0x3B, //!< +12dB
    MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL_11_5DB  = 0x3A, //!< +11.5dB
    MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL_11DB    = 0x39, //!< +11dB
    MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL_10_5DB  = 0x38, //!< +10.5dB
    MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL_10DB    = 0x37, //!< +10dB
    MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL_9_5DB   = 0x36, //!< +9.5dB
    MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL_9DB     = 0x35, //!< +9dB
    MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL_8DB     = 0x34, //!< +8dB
    MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL_7DB     = 0x33, //!< +7dB
    MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL_6DB     = 0x32, //!< +6dB
    MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL_5DB     = 0x31, //!< +5dB
    MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL_4DB     = 0x30, //!< +4dB
    MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL_3DB     = 0x2F, //!< +3dB
    MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL_2DB     = 0x2E, //!< +2dB
    MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL_1DB     = 0x2D, //!< +1dB
    MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL_0DB     = 0x2C, //!< +0dB
    MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL_M1DB    = 0x2B, //!< -1dB
    MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL_M2DB    = 0x2A, //!< -2dB
    MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL_M3DB    = 0x29, //!< -3dB
    MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL_M4DB    = 0x28, //!< -4dB
    MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL_M5DB    = 0x27, //!< -5dB
    MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL_M6DB    = 0x26, //!< -6dB
    MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL_M8DB    = 0x25, //!< -8dB
    MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL_M10DB   = 0x24, //!< -10dB
    MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL_M12DB   = 0x23, //!< -12dB
    MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL_M14DB   = 0x22, //!< -14dB
    MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL_M17DB   = 0x21, //!< -17dB
    MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL_M20DB   = 0x20, //!< -20dB
    MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL_M23DB   = 0x1F, //!< -23dB
    MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL_M26DB   = 0x1E, //!< -26dB
    MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL_M29DB   = 0x1D, //!< -29dB
    MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL_M32DB   = 0x1C, //!< -32dB
    MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL_M36DB   = 0x1B, //!< -36dB
    MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL_M40DB   = 0x1A, //!< -40dB
    MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL_M44DB   = 0x19, //!< -44dB
    MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL_M48DB   = 0x18  //!< -48dB
};

/*!
  @brief MAX98090 RIGHT_SPK_VOLUME[SPRM] Value: Right Speaker Output Mute Enable.
*/
enum MAX98090_BIT_RIGHT_SPK_VOLUME_SPRM_VAL {
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPRM_ENABLE   = 1, //!< Right Speaker output muted.
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPRM_DISABLED = 0  //!< Speaker output volume set by the volume control bits.
};

/*!
  @brief MAX98090 RIGHT_SPK_VOLUME[SPVOLR] Value: Right Speaker Output Amplifier Volume Control Configuration.
*/
enum MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR_VAL {
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR_14DB   = 0x3F, //!< +14dB
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR_13_5DB = 0x3E, //!< +13.5dB
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR_13DB   = 0x3D, //!< +13dB
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR_12_5DB = 0x3C, //!< +12.5dB
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR_12DB   = 0x3B, //!< +12dB
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR_11_5DB = 0x3A, //!< +11.5dB
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR_11_DB  = 0x39, //!< +11dB
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR_10_5DB = 0x38, //!< +10.5dB
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR_10DB   = 0x37, //!< +10dB
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR_9_5DB  = 0x36, //!< +9.5dB
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR_9DB    = 0x35, //!< +9dB
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR_8DB    = 0x34, //!< +8dB
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR_7DB    = 0x33, //!< +7dB
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR_6DB    = 0x32, //!< +6dB
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR_5DB    = 0x31, //!< +5dB
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR_4DB    = 0x30, //!< +4dB
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR_3DB    = 0x2F, //!< +3dB
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR_2DB    = 0x2E, //!< +2dB
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR_1DB    = 0x2D, //!< +1dB
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR_0DB    = 0x2C, //!< +0dB
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR_M1DB   = 0x2B, //!< -1dB
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR_M2DB   = 0x2A, //!< -2dB
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR_M3DB   = 0x29, //!< -3dB
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR_M4DB   = 0x28, //!< -4dB
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR_M5DB   = 0x27, //!< -5dB
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR_M6DB   = 0x26, //!< -6dB
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR_M8DB   = 0x25, //!< -8dB
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR_M10DB  = 0x24, //!< -10dB
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR_M12DB  = 0x23, //!< -12dB
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR_M14DB  = 0x22, //!< -14dB
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR_M17DB  = 0x21, //!< -17dB
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR_M20DB  = 0x20, //!< -20dB
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR_M23DB  = 0x1F, //!< -23dB
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR_M26DB  = 0x1E, //!< -26dB
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR_M29DB  = 0x1D, //!< -29dB
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR_M32DB  = 0x1C, //!< -32dB
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR_M36DB  = 0x1B, //!< -36dB
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR_M40DB  = 0x1A, //!< -40dB
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR_M44DB  = 0x19, //!< -44dB
    MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR_M48DB  = 0x18  //!< -48dB
};

/*!
  @brief MAX98090 ALC_TIMING[ALCEN] Value: DAC ALC Enable.
*/
enum MAX98090_BIT_ALC_TIMING_ALCEN_VAL {
    MAX98090_BIT_ALC_TIMING_ALCEN_DISABLED = 0, //!< ALC disabled.
    MAX98090_BIT_ALC_TIMING_ALCEN_ENABLED  = 1  //!< ALC enabled.
};

/*!
  @brief MAX98090 ALC_TIMING[ALCRLS] Value: DAC ALC Release Time Configuration.
*/
enum MAX98090_BIT_ALC_TIMING_ALCRLS_VAL {
    MAX98090_BIT_ALC_TIMING_ALCRLS_8S       = 0x00, //!< 8s
    MAX98090_BIT_ALC_TIMING_ALCRLS_4S       = 0x01, //!< 4s
    MAX98090_BIT_ALC_TIMING_ALCRLS_2S       = 0x02, //!< 2s
    MAX98090_BIT_ALC_TIMING_ALCRLS_1S       = 0x03, //!< 1s
    MAX98090_BIT_ALC_TIMING_ALCRLS_0_5S     = 0x04, //!< 0.5s
    MAX98090_BIT_ALC_TIMING_ALCRLS_0_25S    = 0x05, //!< 0.25s
    MAX98090_BIT_ALC_TIMING_ALCRLS_0_125S   = 0x06, //!< 0.125s
    MAX98090_BIT_ALC_TIMING_ALCRLS_0_0625S  = 0x07  //!< 0.0625s
};

/*!
  @brief MAX98090 ALC_TIMING[ALCATK] Value: DAC ALC Attack Time Configuration.
*/
enum MAX98090_BIT_ALC_TIMING_ALCATK_VAL {
    MAX98090_BIT_ALC_TIMING_ALCATK_0_5MS    = 0x00, //!< 0.5ms
    MAX98090_BIT_ALC_TIMING_ALCATK_1MS      = 0x01, //!< 1ms
    MAX98090_BIT_ALC_TIMING_ALCATK_5MS      = 0x02, //!< 5ms
    MAX98090_BIT_ALC_TIMING_ALCATK_10MS     = 0x03, //!< 10ms
    MAX98090_BIT_ALC_TIMING_ALCATK_25MS     = 0x04, //!< 25ms
    MAX98090_BIT_ALC_TIMING_ALCATK_50MS     = 0x05, //!< 50ms
    MAX98090_BIT_ALC_TIMING_ALCATK_100MS    = 0x06, //!< 100ms
    MAX98090_BIT_ALC_TIMING_ALCATK_200MS    = 0x07  //!< 200ms
};

/*!
  @brief MAX98090 ALC_COMPRESSOR[ALCCMP] Value: DAC ALC Compression Ratio Configuration.
*/
enum MAX98090_BIT_ALC_COMPRESSOR_ALCCMP_VAL {
    MAX98090_BIT_ALC_COMPRESSOR_ALCCMP_0 = 0x00, //!< 1:1
    MAX98090_BIT_ALC_COMPRESSOR_ALCCMP_1 = 0x01, //!< 1:1.5
    MAX98090_BIT_ALC_COMPRESSOR_ALCCMP_2 = 0x02, //!< 1:2
    MAX98090_BIT_ALC_COMPRESSOR_ALCCMP_3 = 0x03, //!< 1:4
    MAX98090_BIT_ALC_COMPRESSOR_ALCCMP_4 = 0x04  //!< 1:inf
};

/*!
  @brief MAX98090 ALC_COMPRESSOR[ALCTHC] Value: DAC ALC Compression Threshold Configuration.
*/
enum MAX98090_BIT_ALC_COMPRESSOR_ALCTHC_VAL {
    MAX98090_BIT_ALC_COMPRESSOR_ALCTHC_0        = 0x00, //!< 0dB
    MAX98090_BIT_ALC_COMPRESSOR_ALCTHC_M1DB     = 0x01, //!< -1dB
    MAX98090_BIT_ALC_COMPRESSOR_ALCTHC_M2DB     = 0x02, //!< -2dB
    MAX98090_BIT_ALC_COMPRESSOR_ALCTHC_M3DB     = 0x03, //!< -3dB
    MAX98090_BIT_ALC_COMPRESSOR_ALCTHC_M4DB     = 0x04, //!< -4dB
    MAX98090_BIT_ALC_COMPRESSOR_ALCTHC_M5DB     = 0x05, //!< -5dB
    MAX98090_BIT_ALC_COMPRESSOR_ALCTHC_M6DB     = 0x06, //!< -6dB
    MAX98090_BIT_ALC_COMPRESSOR_ALCTHC_M7DB     = 0x07, //!< -7dB
    MAX98090_BIT_ALC_COMPRESSOR_ALCTHC_M8DB     = 0x08, //!< -8dB
    MAX98090_BIT_ALC_COMPRESSOR_ALCTHC_M9DB     = 0x09, //!< -9dB
    MAX98090_BIT_ALC_COMPRESSOR_ALCTHC_M10DB    = 0x0A, //!< -10dB
    MAX98090_BIT_ALC_COMPRESSOR_ALCTHC_M11DB    = 0x0B, //!< -11dB
    MAX98090_BIT_ALC_COMPRESSOR_ALCTHC_M12DB    = 0x0C, //!< -12dB
    MAX98090_BIT_ALC_COMPRESSOR_ALCTHC_M13DB    = 0x0D, //!< -13dB
    MAX98090_BIT_ALC_COMPRESSOR_ALCTHC_M14DB    = 0x0E, //!< -14dB
    MAX98090_BIT_ALC_COMPRESSOR_ALCTHC_M15DB    = 0x0F, //!< -15dB
    MAX98090_BIT_ALC_COMPRESSOR_ALCTHC_M16DB    = 0x10, //!< -16dB
    MAX98090_BIT_ALC_COMPRESSOR_ALCTHC_M17DB    = 0x11, //!< -17dB
    MAX98090_BIT_ALC_COMPRESSOR_ALCTHC_M18DB    = 0x12, //!< -18dB
    MAX98090_BIT_ALC_COMPRESSOR_ALCTHC_M19DB    = 0x13, //!< -19dB
    MAX98090_BIT_ALC_COMPRESSOR_ALCTHC_M20DB    = 0x14, //!< -20dB
    MAX98090_BIT_ALC_COMPRESSOR_ALCTHC_M21DB    = 0x15, //!< -21dB
    MAX98090_BIT_ALC_COMPRESSOR_ALCTHC_M22DB    = 0x16, //!< -22dB
    MAX98090_BIT_ALC_COMPRESSOR_ALCTHC_M23DB    = 0x17, //!< -23dB
    MAX98090_BIT_ALC_COMPRESSOR_ALCTHC_M24DB    = 0x18, //!< -24dB
    MAX98090_BIT_ALC_COMPRESSOR_ALCTHC_M25DB    = 0x19, //!< -25dB
    MAX98090_BIT_ALC_COMPRESSOR_ALCTHC_M26DB    = 0x1A, //!< -26dB
    MAX98090_BIT_ALC_COMPRESSOR_ALCTHC_M27DB    = 0x1B, //!< -27dB
    MAX98090_BIT_ALC_COMPRESSOR_ALCTHC_M28DB    = 0x1C, //!< -28dB
    MAX98090_BIT_ALC_COMPRESSOR_ALCTHC_M29DB    = 0x1D, //!< -29dB
    MAX98090_BIT_ALC_COMPRESSOR_ALCTHC_M30DB    = 0x1E, //!< -30dB
    MAX98090_BIT_ALC_COMPRESSOR_ALCTHC_M31DB    = 0x1F  //!< -31dB
};

/*!
  @brief MAX98090 ALC_EXPANDER[ALCEXP] Value: DAC ALC Expansion Ratio Configuration.
*/
enum MAX98090_BIT_ALC_EXPANDER_ALCEXP_VAL {
    MAX98090_BIT_ALC_EXPANDER_ALCEXP_0 = 0x00, //!< 1:1
    MAX98090_BIT_ALC_EXPANDER_ALCEXP_1 = 0x01, //!< 2:1
    MAX98090_BIT_ALC_EXPANDER_ALCEXP_2 = 0x02  //!< 3:1
};

/*!
  @brief MAX98090 ALC_EXPANDER[ALCTHE] Value: DAC ALC Expansion Threshold Configuration.
*/
enum MAX98090_BIT_ALC_EXPANDER_ALCTHE_VAL {
    MAX98090_BIT_ALC_EXPANDER_ALCTHE_M35DB = 0x00, //!< -35dB
    MAX98090_BIT_ALC_EXPANDER_ALCTHE_M36DB = 0x01, //!< -36dB
    MAX98090_BIT_ALC_EXPANDER_ALCTHE_M37DB = 0x02, //!< -37dB
    MAX98090_BIT_ALC_EXPANDER_ALCTHE_M38DB = 0x03, //!< -38dB
    MAX98090_BIT_ALC_EXPANDER_ALCTHE_M39DB = 0x04, //!< -39dB
    MAX98090_BIT_ALC_EXPANDER_ALCTHE_M40DB = 0x05, //!< -40dB
    MAX98090_BIT_ALC_EXPANDER_ALCTHE_M41DB = 0x06, //!< -41dB
    MAX98090_BIT_ALC_EXPANDER_ALCTHE_M42DB = 0x07, //!< -42dB
    MAX98090_BIT_ALC_EXPANDER_ALCTHE_M43DB = 0x08, //!< -43dB
    MAX98090_BIT_ALC_EXPANDER_ALCTHE_M44DB = 0x09, //!< -44dB
    MAX98090_BIT_ALC_EXPANDER_ALCTHE_M45DB = 0x0A, //!< -45dB
    MAX98090_BIT_ALC_EXPANDER_ALCTHE_M46DB = 0x0B, //!< -46dB
    MAX98090_BIT_ALC_EXPANDER_ALCTHE_M47DB = 0x0C, //!< -47dB
    MAX98090_BIT_ALC_EXPANDER_ALCTHE_M48DB = 0x0D, //!< -48dB
    MAX98090_BIT_ALC_EXPANDER_ALCTHE_M49DB = 0x0E, //!< -49dB
    MAX98090_BIT_ALC_EXPANDER_ALCTHE_M50DB = 0x0F, //!< -50dB
    MAX98090_BIT_ALC_EXPANDER_ALCTHE_M51DB = 0x10, //!< -51dB
    MAX98090_BIT_ALC_EXPANDER_ALCTHE_M52DB = 0x11, //!< -52dB
    MAX98090_BIT_ALC_EXPANDER_ALCTHE_M53DB = 0x12, //!< -53dB
    MAX98090_BIT_ALC_EXPANDER_ALCTHE_M54DB = 0x13, //!< -54dB
    MAX98090_BIT_ALC_EXPANDER_ALCTHE_M55DB = 0x14, //!< -55dB
    MAX98090_BIT_ALC_EXPANDER_ALCTHE_M56DB = 0x15, //!< -56dB
    MAX98090_BIT_ALC_EXPANDER_ALCTHE_M57DB = 0x16, //!< -57dB
    MAX98090_BIT_ALC_EXPANDER_ALCTHE_M58DB = 0x17, //!< -58dB
    MAX98090_BIT_ALC_EXPANDER_ALCTHE_M59DB = 0x18, //!< -59dB
    MAX98090_BIT_ALC_EXPANDER_ALCTHE_M60DB = 0x19, //!< -60dB
    MAX98090_BIT_ALC_EXPANDER_ALCTHE_M61DB = 0x1A, //!< -61dB
    MAX98090_BIT_ALC_EXPANDER_ALCTHE_M62DB = 0x1B, //!< -62dB
    MAX98090_BIT_ALC_EXPANDER_ALCTHE_M63DB = 0x1C, //!< -63dB
    MAX98090_BIT_ALC_EXPANDER_ALCTHE_M64DB = 0x1D, //!< -64dB
    MAX98090_BIT_ALC_EXPANDER_ALCTHE_M65DB = 0x1E, //!< -65dB
    MAX98090_BIT_ALC_EXPANDER_ALCTHE_M66DB = 0x1F  //!< -66dB
};

/*!
  @brief MAX98090 ALC_GAIN[ALCG] Value: DAC ALC Make-Up Gain Configuration.
*/
enum MAX98090_BIT_ALC_GAIN_ALCG_VAL {
    MAX98090_BIT_ALC_GAIN_ALCG_0DB          = 0x00, //!< +0dB
    MAX98090_BIT_ALC_GAIN_ALCG_1DB          = 0x01, //!< +1dB
    MAX98090_BIT_ALC_GAIN_ALCG_2DB          = 0x02, //!< +2dB
    MAX98090_BIT_ALC_GAIN_ALCG_3DB          = 0x03, //!< +3dB
    MAX98090_BIT_ALC_GAIN_ALCG_4DB          = 0x04, //!< +4dB
    MAX98090_BIT_ALC_GAIN_ALCG_5DB          = 0x05, //!< +5dB
    MAX98090_BIT_ALC_GAIN_ALCG_6DB          = 0x06, //!< +6dB
    MAX98090_BIT_ALC_GAIN_ALCG_7DB          = 0x07, //!< +7dB
    MAX98090_BIT_ALC_GAIN_ALCG_8DB          = 0x08, //!< +8dB
    MAX98090_BIT_ALC_GAIN_ALCG_9DB          = 0x09, //!< +9dB
    MAX98090_BIT_ALC_GAIN_ALCG_10DB         = 0x0A, //!< +10dB
    MAX98090_BIT_ALC_GAIN_ALCG_11DB         = 0x0B, //!< +11dB
    MAX98090_BIT_ALC_GAIN_ALCG_12DB         = 0x0C, //!< +12dB
    MAX98090_BIT_ALC_GAIN_ALCG_RESERVED1    = 0x0D, //!< reserved
    MAX98090_BIT_ALC_GAIN_ALCG_RESERVED2    = 0x0E, //!< reserved
    MAX98090_BIT_ALC_GAIN_ALCG_RESERVED3    = 0x0F  //!< reserved
};

/*!
  @brief MAX98090 RCV_OR_LOUTL_CONTROL[MIXRCVLG] Value: Receiver / Line Output Left Mixer Gain Configuration.
*/
enum MAX98090_BIT_RCV_OR_LOUTL_CONTROL_MIXRCVLG_VAL {
    MAX98090_BIT_RCV_OR_LOUTL_CONTROL_MIXRCVLG_0DB      = 0x00, //!< 0dB
    MAX98090_BIT_RCV_OR_LOUTL_CONTROL_MIXRCVLG_M6DB     = 0x01, //!< -6dB
    MAX98090_BIT_RCV_OR_LOUTL_CONTROL_MIXRCVLG_M9_5DB   = 0x02, //!< -9.5dB
    MAX98090_BIT_RCV_OR_LOUTL_CONTROL_MIXRCVLG_M12DB    = 0x03  //!< -12dB
};

/*!
  @brief MAX98090 RCV_OR_LOUTL_VOLUME[RCVLM] Value: Receiver / Line Output Left Mute.
*/
enum MAX98090_BIT_RCV_OR_LOUTL_VOLUME_RCVLM_VAL {
    MAX98090_BIT_RCV_OR_LOUTL_VOLUME_RCVLM_NOT_MUTED    = 0x00, //!< not muted.
    MAX98090_BIT_RCV_OR_LOUTL_VOLUME_RCVLM_MUTED        = 0x01  //!< muted.
};

/*!
  @brief MAX98090 RCV_OR_LOUTL_VOLUME[RCVLVOL] Value: Receiver / Line Output Left PGA Volume Configuration.
*/
enum MAX98090_BIT_RCV_OR_LOUTL_VOLUME_RCVLVOL_VAL {
    MAX98090_BIT_RCV_OR_LOUTL_VOLUME_RCVLVOL_8DB    = 0x1F, //!< +8dB
    MAX98090_BIT_RCV_OR_LOUTL_VOLUME_RCVLVOL_7_5DB  = 0x1E, //!< +7.5dB
    MAX98090_BIT_RCV_OR_LOUTL_VOLUME_RCVLVOL_7DB    = 0x1D, //!< +7dB
    MAX98090_BIT_RCV_OR_LOUTL_VOLUME_RCVLVOL_6_5DB  = 0x1C, //!< +6.5dB
    MAX98090_BIT_RCV_OR_LOUTL_VOLUME_RCVLVOL_6DB    = 0x1B, //!< +6dB
    MAX98090_BIT_RCV_OR_LOUTL_VOLUME_RCVLVOL_5DB    = 0x1A, //!< +5dB
    MAX98090_BIT_RCV_OR_LOUTL_VOLUME_RCVLVOL_4DB    = 0x19, //!< +4dB
    MAX98090_BIT_RCV_OR_LOUTL_VOLUME_RCVLVOL_3DB    = 0x18, //!< +3dB
    MAX98090_BIT_RCV_OR_LOUTL_VOLUME_RCVLVOL_2DB    = 0x17, //!< +2dB
    MAX98090_BIT_RCV_OR_LOUTL_VOLUME_RCVLVOL_1DB    = 0x16, //!< +1dB
    MAX98090_BIT_RCV_OR_LOUTL_VOLUME_RCVLVOL_0DB    = 0x15, //!< +0dB
    MAX98090_BIT_RCV_OR_LOUTL_VOLUME_RCVLVOL_M2DB   = 0x14, //!< -2dB
    MAX98090_BIT_RCV_OR_LOUTL_VOLUME_RCVLVOL_M4DB   = 0x13, //!< -4dB
    MAX98090_BIT_RCV_OR_LOUTL_VOLUME_RCVLVOL_M6DB   = 0x12, //!< -6dB
    MAX98090_BIT_RCV_OR_LOUTL_VOLUME_RCVLVOL_M8DB   = 0x11, //!< -8dB
    MAX98090_BIT_RCV_OR_LOUTL_VOLUME_RCVLVOL_M10DB  = 0x10, //!< -10dB
    MAX98090_BIT_RCV_OR_LOUTL_VOLUME_RCVLVOL_M12DB  = 0x0F, //!< -12dB
    MAX98090_BIT_RCV_OR_LOUTL_VOLUME_RCVLVOL_M14DdB = 0x0E, //!< -14dB
    MAX98090_BIT_RCV_OR_LOUTL_VOLUME_RCVLVOL_M17DdB = 0x0D, //!< -17dB
    MAX98090_BIT_RCV_OR_LOUTL_VOLUME_RCVLVOL_M20DdB = 0x0C, //!< -20dB
    MAX98090_BIT_RCV_OR_LOUTL_VOLUME_RCVLVOL_M23DdB = 0x0B, //!< -23dB
    MAX98090_BIT_RCV_OR_LOUTL_VOLUME_RCVLVOL_M26DdB = 0x0A, //!< -26dB
    MAX98090_BIT_RCV_OR_LOUTL_VOLUME_RCVLVOL_M29DdB = 0x09, //!< -29dB
    MAX98090_BIT_RCV_OR_LOUTL_VOLUME_RCVLVOL_M32DdB = 0x08, //!< -32dB
    MAX98090_BIT_RCV_OR_LOUTL_VOLUME_RCVLVOL_M35DdB = 0x07, //!< -35dB
    MAX98090_BIT_RCV_OR_LOUTL_VOLUME_RCVLVOL_M38DdB = 0x06, //!< -38dB
    MAX98090_BIT_RCV_OR_LOUTL_VOLUME_RCVLVOL_M42DdB = 0x05, //!< -42dB
    MAX98090_BIT_RCV_OR_LOUTL_VOLUME_RCVLVOL_M46DdB = 0x04, //!< -46dB
    MAX98090_BIT_RCV_OR_LOUTL_VOLUME_RCVLVOL_M50DdB = 0x03, //!< -50dB
    MAX98090_BIT_RCV_OR_LOUTL_VOLUME_RCVLVOL_M54DdB = 0x02, //!< -54dB
    MAX98090_BIT_RCV_OR_LOUTL_VOLUME_RCVLVOL_M58DdB = 0x01, //!< -58dB
    MAX98090_BIT_RCV_OR_LOUTL_VOLUME_RCVLVOL_M62DdB = 0x00  //!< -62dB
};

/*!
  @brief MAX98090 LOUTR_MIXER[LINMOD] Value: Selects Between Receiver BTL mode and Line Output mode.
*/
enum MAX98090_BIT_LOUTR_MIXER_LINMOD_VAL {
    MAX98090_BIT_LOUTR_MIXER_LINMOD_LINEOUT = 1, //!< LINEOUT mode.
    MAX98090_BIT_LOUTR_MIXER_LINMOD_BTL     = 0  //!< BTL mode. 
};

/*!
  @brief MAX98090 LOUTR_CONTROL[MIXRCVRG] Value: Line Output Right Mixer Gain Configuration.
*/
enum MAX98090_BIT_LOUTR_CONTROL_MIXRCVRG_VAL {
    MAX98090_BIT_LOUTR_CONTROL_MIXRCVRG_0DB     = 0x00, //!< 0dB
    MAX98090_BIT_LOUTR_CONTROL_MIXRCVRG_M6DB    = 0x01, //!< -6dB
    MAX98090_BIT_LOUTR_CONTROL_MIXRCVRG_M9_5DB  = 0x02, //!< -9.5dB
    MAX98090_BIT_LOUTR_CONTROL_MIXRCVRG_M12DB   = 0x03  //!< -12dB
};

/*!
  @brief MAX98090 LOUTR_VOLUME[RCVRM] Value: Line Output Right Mute.
*/
enum MAX98090_BIT_LOUTR_VOLUME_RCVRM_VAL {
    MAX98090_BIT_LOUTR_VOLUME_RCVRM_NOT_MUTED   = 0, //!< not muted
    MAX98090_BIT_LOUTR_VOLUME_RCVRM_MUTED       = 1  //!< muted
};

/*!
  @brief MAX98090 LOUTR_VOLUME[RCVRVOL] Value: Line Output Right PGA Volume Configuration.
*/
enum MAX98090_BIT_LOUTR_VOLUME_RCVRVOL_VAL {
    MAX98090_BIT_LOUTR_VOLUME_RCVRVOL_8DB   = 0x1F, //!< +8dB
    MAX98090_BIT_LOUTR_VOLUME_RCVRVOL_7_5DB = 0x1E, //!< +7.5dB
    MAX98090_BIT_LOUTR_VOLUME_RCVRVOL_7DB   = 0x1D, //!< +7dB
    MAX98090_BIT_LOUTR_VOLUME_RCVRVOL_6_5DB = 0x1C, //!< +6.5dB
    MAX98090_BIT_LOUTR_VOLUME_RCVRVOL_6DB   = 0x1B, //!< +6dB
    MAX98090_BIT_LOUTR_VOLUME_RCVRVOL_5DB   = 0x1A, //!< +5dB
    MAX98090_BIT_LOUTR_VOLUME_RCVRVOL_4DB   = 0x19, //!< +4dB
    MAX98090_BIT_LOUTR_VOLUME_RCVRVOL_3DB   = 0x18, //!< +3dB
    MAX98090_BIT_LOUTR_VOLUME_RCVRVOL_2DB   = 0x17, //!< +2dB
    MAX98090_BIT_LOUTR_VOLUME_RCVRVOL_1DB   = 0x16, //!< +1dB
    MAX98090_BIT_LOUTR_VOLUME_RCVRVOL_0DB   = 0x15, //!< +0dB
    MAX98090_BIT_LOUTR_VOLUME_RCVRVOL_M2DB  = 0x14, //!< -2dB
    MAX98090_BIT_LOUTR_VOLUME_RCVRVOL_M4DB  = 0x13, //!< -4dB
    MAX98090_BIT_LOUTR_VOLUME_RCVRVOL_M6DB  = 0x12, //!< -6dB
    MAX98090_BIT_LOUTR_VOLUME_RCVRVOL_M8DB  = 0x11, //!< -8dB
    MAX98090_BIT_LOUTR_VOLUME_RCVRVOL_M10DB = 0x10, //!< -10dB
    MAX98090_BIT_LOUTR_VOLUME_RCVRVOL_M12DB = 0x0F, //!< -12dB
    MAX98090_BIT_LOUTR_VOLUME_RCVRVOL_M14DB = 0x0E, //!< -14dB
    MAX98090_BIT_LOUTR_VOLUME_RCVRVOL_M17DB = 0x0D, //!< -17dB
    MAX98090_BIT_LOUTR_VOLUME_RCVRVOL_M20DB = 0x0C, //!< -20dB
    MAX98090_BIT_LOUTR_VOLUME_RCVRVOL_M23DB = 0x0B, //!< -23dB
    MAX98090_BIT_LOUTR_VOLUME_RCVRVOL_M26DB = 0x0A, //!< -26dB
    MAX98090_BIT_LOUTR_VOLUME_RCVRVOL_M29DB = 0x09, //!< -29dB
    MAX98090_BIT_LOUTR_VOLUME_RCVRVOL_M32DB = 0x08, //!< -32dB
    MAX98090_BIT_LOUTR_VOLUME_RCVRVOL_M35DB = 0x07, //!< -35dB
    MAX98090_BIT_LOUTR_VOLUME_RCVRVOL_M38DB = 0x06, //!< -38dB
    MAX98090_BIT_LOUTR_VOLUME_RCVRVOL_M42DB = 0x05, //!< -42dB
    MAX98090_BIT_LOUTR_VOLUME_RCVRVOL_M46DB = 0x04, //!< -46dB
    MAX98090_BIT_LOUTR_VOLUME_RCVRVOL_M50DB = 0x03, //!< -50dB
    MAX98090_BIT_LOUTR_VOLUME_RCVRVOL_M54DB = 0x02, //!< -54dB
    MAX98090_BIT_LOUTR_VOLUME_RCVRVOL_M58DB = 0x01  //!< -58dB
};

/*!
  @brief MAX98090 JACK_DETECT[JKWK] Value: JACKSNS Pull-up.
*/
enum MAX98090_BIT_JACK_DETECT_JKWK_VAL {
    MAX98090_BIT_JACK_DETECT_JKWK_2_4K  = 0, //!< 2.4k resistor to SPKLVDD (allows microphone detection).
    MAX98090_BIT_JACK_DETECT_JKWK_5UA   = 1  //!< 5uA to SPKLVDD (minimizes supply current).
};

/*!
  @brief MAX98090 JACK_DETECT[JDEB] Value: Jack Detect Debounce (JDEB).
*/
enum MAX98090_BIT_JACK_DETECT_JDEB_VAL {
    MAX98090_BIT_JACK_DETECT_JDEB_25MS  = 0x00, //!< 25ms
    MAX98090_BIT_JACK_DETECT_JDEB_50MS  = 0x01, //!< 50ms
    MAX98090_BIT_JACK_DETECT_JDEB_100MS = 0x02, //!< 100ms
    MAX98090_BIT_JACK_DETECT_JDEB_200MS = 0x03  //!< 200ms
};

/*!
  @brief MAX98090 INPUT_ENABLE[MBEN] Value: Microphone Bias Enable.
*/
enum MAX98090_BIT_INPUT_ENABLE_MBEN_VAL {
    MAX98090_BIT_INPUT_ENABLE_MBEN_DISABLE  = 0, //!< Disable Microphone Bias.
    MAX98090_BIT_INPUT_ENABLE_MBEN_ENABLE   = 1  //!< Enable Microphone Bias.
};

/*!
  @brief MAX98090 INPUT_ENABLE[LINEAEN] Value: Enables Line A Analog Input Block.
*/
enum MAX98090_BIT_INPUT_ENABLE_LINEAEN_VAL {
    MAX98090_BIT_INPUT_ENABLE_LINEAEN_DISABLE = 0, //!< Line A Input amplifier disabled.
    MAX98090_BIT_INPUT_ENABLE_LINEAEN_ENABLE  = 1  //!< Line A Input amplifier enabled.
};

/*!
  @brief MAX98090 INPUT_ENABLE[LINEBEN] Value: Enables Line A Analog Input Block.
*/
enum MAX98090_BIT_INPUT_ENABLE_LINEBEN_VAL {
    MAX98090_BIT_INPUT_ENABLE_LINEBEN_DISABLE = 0, //!< Line B Input amplifier disabled.
    MAX98090_BIT_INPUT_ENABLE_LINEBEN_ENABLE  = 1  //!< Line B Input amplifier enabled.
};

/*!
  @brief MAX98090 INPUT_ENABLE[ADREN] Value: Right ADC Enable.
*/
enum MAX98090_BIT_INPUT_ENABLE_ADREN_VAL {
    MAX98090_BIT_INPUT_ENABLE_ADREN_DISABLE = 0, //!< Right ADC disabled.
    MAX98090_BIT_INPUT_ENABLE_ADREN_ENABLE  = 1  //!< Right ADC enabled.
};

/*!
  @brief MAX98090 INPUT_ENABLE[ADLEN] Value: Left ADC Enable.
*/
enum MAX98090_BIT_INPUT_ENABLE_ADLEN_VAL {
    MAX98090_BIT_INPUT_ENABLE_ADLEN_DISABLE = 0, //!< Left ADC disabled.
    MAX98090_BIT_INPUT_ENABLE_ADLEN_ENABLE  = 1  //!< Left ADC enabled.
};

/*!
  @brief MAX98090 OUTPUT_ENABLE[HPREN] Value: Right Headphone Output Enable.
*/
enum MAX98090_BIT_OUTPUT_ENABLE_HPREN_VAL {
    MAX98090_BIT_OUTPUT_ENABLE_HPREN_DISABLE = 0, //!< Disable Right Headphone Output.
    MAX98090_BIT_OUTPUT_ENABLE_HPREN_ENABLE  = 1  //!< Enable Right Headphone Output.
};

/*!
  @brief MAX98090 OUTPUT_ENABLE[HPLEN] Value: Left Headphone Output Enable.
*/
enum MAX98090_BIT_OUTPUT_ENABLE_HPLEN_VAL {
    MAX98090_BIT_OUTPUT_ENABLE_HPLEN_DISABLE = 0, //!< Disable Left Headphone Output.
    MAX98090_BIT_OUTPUT_ENABLE_HPLEN_ENABLE  = 1  //!< Enable Left Headphone Output.
};

/*!
  @brief MAX98090 OUTPUT_ENABLE[SPREN] Value: Right Class-D Speaker Output Enable.
*/
enum MAX98090_BIT_OUTPUT_ENABLE_SPREN_VAL {
    MAX98090_BIT_OUTPUT_ENABLE_SPREN_DISABLE = 0, //!< Disable Right Speaker Output.
    MAX98090_BIT_OUTPUT_ENABLE_SPREN_ENABLE  = 1  //!< Enable Right Speaker Output.
};

/*!
  @brief MAX98090 OUTPUT_ENABLE[SPLEN] Value: Left Class-D Speaker Output Enable.
*/
enum MAX98090_BIT_OUTPUT_ENABLE_SPLEN_VAL {
    MAX98090_BIT_OUTPUT_ENABLE_SPLEN_DISABLE = 0, //!< Disable Left Speaker Output.
    MAX98090_BIT_OUTPUT_ENABLE_SPLEN_ENABLE  = 1  //!< Enable Left Speaker Output.
};

/*!
  @brief MAX98090 OUTPUT_ENABLE[RCVLEN] Value: Receiver (Earpiece) / Line Out Left Output Enable.
*/
enum MAX98090_BIT_OUTPUT_ENABLE_RCVLEN_VAL {
    MAX98090_BIT_OUTPUT_ENABLE_RCVLEN_DISABLE = 0, //!< Disable Receiver / Left Line Output
    MAX98090_BIT_OUTPUT_ENABLE_RCVLEN_ENABLE  = 1  //!< Enable Receiver / Left Line Output.
};

/*!
  @brief MAX98090 OUTPUT_ENABLE[RCVREN] Value: Right Line Output Enable.
*/
enum MAX98090_BIT_OUTPUT_ENABLE_RCVREN_VAL {
    MAX98090_BIT_OUTPUT_ENABLE_RCVREN_DISABLE = 0, //!< Disable Right Line Output
    MAX98090_BIT_OUTPUT_ENABLE_RCVREN_ENABLE  = 1  //!< Enable Right Line Output.
};

/*!
  @brief MAX98090 OUTPUT_ENABLE[DAREN] Value: Right DAC Digital Input Enable.
*/
enum MAX98090_BIT_OUTPUT_ENABLE_DAREN_VAL {
    MAX98090_BIT_OUTPUT_ENABLE_DAREN_DISABLE = 0, //!< Disable Right DAC Input
    MAX98090_BIT_OUTPUT_ENABLE_DAREN_ENABLE  = 1  //!< Enable Right DAC Input.
};

/*!
  @brief MAX98090 OUTPUT_ENABLE[DALEN] Value: Left DAC Digital Input Enable.
*/
enum MAX98090_BIT_OUTPUT_ENABLE_DALEN_VAL {
    MAX98090_BIT_OUTPUT_ENABLE_DALEN_DISABLE = 0, //!< Disable Left DAC Input.
    MAX98090_BIT_OUTPUT_ENABLE_DALEN_ENABLE  = 1  //!< Enable Left DAC Input.
};

/*!
  @brief MAX98090 LEVEL_CONTROL[ZDEN] Value: Zero-Crossing Detection (/ZDEN).
*/
enum MAX98090_BIT_LEVEL_CONTROL_ZDEN_VAL {
    MAX98090_BIT_LEVEL_CONTROL_ZDEN_1 = 1, //!< Volume changes made immediately upon request.
    MAX98090_BIT_LEVEL_CONTROL_ZDEN_0 = 0  //!< Volume changes made only at zero crossings in the audio waveform or after approximately 100ms.
};

/*!
  @brief MAX98090 LEVEL_CONTROL[VS2EN] Value: Enhanced Volume Smoothing (/VS2EN).
*/
enum MAX98090_BIT_LEVEL_CONTROL_VS2EN_VAL {
    MAX98090_BIT_LEVEL_CONTROL_VS2EN_1 = 1, //!< Enhancement disabled.
    MAX98090_BIT_LEVEL_CONTROL_VS2EN_0 = 0  //!< Slewed volume changes wait until the previous volume step has been applied to the output before changing to the next step.
};

/*!
  @brief MAX98090 LEVEL_CONTROL[VSEN] Value: Volume Adjustment Smoothing (/VSEN).
*/
enum MAX98090_BIT_LEVEL_CONTROL_VSEN_VAL {
    MAX98090_BIT_LEVEL_CONTROL_VSEN_1 = 1, //!< Volume changes made by bypassing intermediate settings.
    MAX98090_BIT_LEVEL_CONTROL_VSEN_0 = 0  //!< Volume changes smoothed by stepping through intermediate values at a rate of one setting every 1ms.
};

/*!
  @brief MAX98090 DSP_FILTER_ENABLE[ADCBQEN] Value: Enable Biquad filter in ADC path.
*/
enum MAX98090_BIT_DSP_FILTER_ENABLE_ADCBQEN_VAL {
    MAX98090_BIT_DSP_FILTER_ENABLE_ADCBQEN_NOT_USED = 0, //!< biquad filter not used.
    MAX98090_BIT_DSP_FILTER_ENABLE_ADCBQEN_USED     = 1  //!< biquad filter used in ADC path,
};

/*!
  @brief MAX98090 DSP_FILTER_ENABLE[EQ3BANDEN] Value: Enable 3 Band EQ in DAC path.
*/
enum MAX98090_BIT_DSP_FILTER_ENABLE_EQ3BANDEN_VAL {
    MAX98090_BIT_DSP_FILTER_ENABLE_EQ3BANDEN_DISABLED = 0, //!< 3 band EQ disabled.
    MAX98090_BIT_DSP_FILTER_ENABLE_EQ3BANDEN_ENABLED  = 1  //!< 3 band EQ enabled.
};

/*!
  @brief MAX98090 DSP_FILTER_ENABLE[EQ5BANDEN] Value: Enable 5 Band EQ in DAC path.
*/
enum MAX98090_BIT_DSP_FILTER_ENABLE_EQ5BANDEN_VAL {
    MAX98090_BIT_DSP_FILTER_ENABLE_EQ5BANDEN_DISABLED = 0, //!< 5 band EQ disabled.
    MAX98090_BIT_DSP_FILTER_ENABLE_EQ5BANDEN_ENABLED  = 1  //!< 5 band EQ enabled.
};

/*!
  @brief MAX98090 DSP_FILTER_ENABLE[EQ7BANDEN] Value: Enable 7 Band EQ in DAC path.
*/
enum MAX98090_BIT_DSP_FILTER_ENABLE_EQ7BANDEN_VAL {
    MAX98090_BIT_DSP_FILTER_ENABLE_EQ7BANDEN_DISABLED = 0, //!< 7band EQ disabled.
    MAX98090_BIT_DSP_FILTER_ENABLE_EQ7BANDEN_ENABLED  = 1  //!< 7 band EQ enabled.
};

/*!
  @brief MAX98090 BIAS_CONTROL[VCM_MODE] Value: Select source for VCM.
*/
enum MAX98090_BIT_BIAS_CONTROL_VCM_MODE_VAL {
    MAX98090_BIT_BIAS_CONTROL_VCM_MODE_DERIVED = 0, //!< VCM derived from resistive division selected (default).
    MAX98090_BIT_BIAS_CONTROL_VCM_MODE_CREATED = 1  //!< VCM created by bandgap reference selected.
};

/*!
  @brief MAX98090 DAC_CONTROL[PERFMODE] Value: Performance Mode (PERFMODE).
*/
enum MAX98090_BIT_DAC_CONTROL_PERFMODE_VAL {
    MAX98090_BIT_DAC_CONTROL_PERFMODE_LOW  = 1, //!< Low power playback mode.
    MAX98090_BIT_DAC_CONTROL_PERFMODE_HIGH = 0  //!< High performance playback mode.
};

/*!
  @brief MAX98090 DAC_CONTROL[DACHP] Value: DAC High Performance Mode.
*/
enum MAX98090_BIT_DAC_CONTROL_DACHP_VAL {
    MAX98090_BIT_DAC_CONTROL_DACHP_LOWEST = 0, //!< DAC settings optimised for lowest power consumption.
    MAX98090_BIT_DAC_CONTROL_DACHP_BEST   = 1  //!< DAC settings optimised for best performance.
};

/*!
  @brief MAX98090 ADC_CONTROL[OSR128] Value: Generate ADC clock at 128*fS or 64*fS.
*/
enum MAX98090_BIT_ADC_CONTROL_OSR128_VAL {
    MAX98090_BIT_ADC_CONTROL_OSR128_64FS  = 0, //!< ADCCLK = 64*fs
    MAX98090_BIT_ADC_CONTROL_OSR128_128FS = 1  //!< ADCCLK = 128*fs (default)
};

/*!
  @brief MAX98090 ADC_CONTROL[ADCDITHER] Value: Enable Dither to the ADC quantizer.
*/
enum MAX98090_BIT_ADC_CONTROL_ADCDITHER_VAL {
    MAX98090_BIT_ADC_CONTROL_ADCDITHER_DISABLED = 0, //!< Dither disabled.
    MAX98090_BIT_ADC_CONTROL_ADCDITHER_ENABLED  = 1  //!< Dither enabled.
};

/*!
  @brief MAX98090 ADC_CONTROL[ADCHP] Value: ADC High Performance Mode.
*/
enum MAX98090_BIT_ADC_CONTROL_ADCHP_VAL {
    MAX98090_BIT_ADC_CONTROL_ADCHP_LOW  = 0, //!< ADC is optimised for low power operation.
    MAX98090_BIT_ADC_CONTROL_ADCHP_BEST = 1  //!< ADC is optimised for best performance.
};

/*!
  @brief MAX98090 DEVICE_SHUTDOWN[SHDN] Value: Device Shutdown.
*/
enum MAX98090_BIT_DEVICE_SHUTDOWN_SHDN_VAL {
    MAX98090_BIT_DEVICE_SHUTDOWN_SHDN_SHUTDOWN  = 0, //!< Device is in shutdown.  All I2C registers can be written in this mode.
    MAX98090_BIT_DEVICE_SHUTDOWN_SHDN_ACTIVE    = 1  //!< Device is active.
};

/*---------------------------------------------------------------------------*/
/* structure declaration                                                     */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* global variable declaration                                               */
/*---------------------------------------------------------------------------*/
/*!
  @brief MAX98090 register address table.
*/
const int MAX98090_reg_addr[MAX98090_REG_ID_MAX] = {
    0x00,   //!< MAX98090_REG_W_SOFTWARE_RESET
    0x01,   //!< MAX98090_REG_R_DEVICE_STATUS
    0x02,   //!< MAX98090_REG_R_JACK DETECT
    0x03,   //!< MAX98090_REG_RW_INTERRUPT_MASKS
    0x04,   //!< MAX98090_REG_W_SYSTEM_CLOCK
    0x05,   //!< MAX98090_REG_W_SAMPLE_RATE
    0x06,   //!< MAX98090_REG_W_INTERFACE
    0x07,   //!< MAX98090_REG_W_DAC_PATH
    0x08,   //!< MAX98090_REG_W_MIC_OR_DIRECT_TO_ADC
    0x09,   //!< MAX98090_REG_W_LINE_TO_ADC
    0x0A,   //!< MAX98090_REG_W_ANALOG_MIC_LOOP
    0x0B,   //!< MAX98090_REG_W_ANALOG_LINE_LOOP
    0x0C,   //!< MAX98090_REG_RW_DIGITAL_MIC
    0x0D,   //!< MAX98090_REG_RW_INPUT_CONFIG
    0x0E,   //!< MAX98090_REG_RW_LINE_INPUT_LEVEL
    0x0F,   //!< MAX98090_REG_RW_LINE_CONFIG
    0x10,   //!< MAX98090_REG_RW_MIC1_INPUT_LEVEL
    0x11,   //!< MAX98090_REG_RW_MIC2_INPUT_LEVEL
    0x12,   //!< MAX98090_REG_RW_MIC_BIAS_VOLTAGE
    0x15,   //!< MAX98090_REG_RW_LEFT_ADC_MIXER
    0x16,   //!< MAX98090_REG_RW_RIGHT_ADC_MIXER
    0x17,   //!< MAX98090_REG_RW_LEFT_ADC_LEVEL
    0x18,   //!< MAX98090_REG_RW_RIGHT_ADC_LEVEL
    0x19,   //!< MAX98090_REG_RW_ADC_BIQUAD_LEVEL
    0x1A,   //!< MAX98090_REG_RW_ADC_SIDETONE
    0x1B,   //!< MAX98090_REG_RW_SYSTEM_CLOCK
    0x1C,   //!< MAX98090_REG_RW_CLOCK_MODE
    0x1D,   //!< MAX98090_REG_RW_ANY_CLOCK_1
    0x1E,   //!< MAX98090_REG_RW_ANY_CLOCK_2
    0x1F,   //!< MAX98090_REG_RW_ANY_CLOCK_3
    0x20,   //!< MAX98090_REG_RW_ANY_CLOCK_4
    0x21,   //!< MAX98090_REG_RW_MASTER_MODE
    0x22,   //!< MAX98090_REG_RW_INTERFACE_FORMAT
    0x23,   //!< MAX98090_REG_RW_TDM_FORMAT_1
    0x24,   //!< MAX98090_REG_RW_TDM_FORMAT_2
    0x25,   //!< MAX98090_REG_RW_I_OR_O_CONFIGURATION
    0x26,   //!< MAX98090_REG_RW_FILTER_CONFIG
    0x27,   //!< MAX98090_REG_RW_PLAYBACK_LEVEL_1
    0x28,   //!< MAX98090_REG_RW_PLAYBACK_LEVEL_2
    0x29,   //!< MAX98090_REG_RW_LEFT_HP_MIXER
    0x2A,   //!< MAX98090_REG_RW_RIGHT_HP_MIXER
    0x2B,   //!< MAX98090_REG_RW_HP_CONTROL
    0x2C,   //!< MAX98090_REG_RW_LEFT_HP_VOLUME
    0x2D,   //!< MAX98090_REG_RW_RIGHT_HP_VOLUME
    0x2E,   //!< MAX98090_REG_RW_LEFT_SPK_MIXER
    0x2F,   //!< MAX98090_REG_RW_RIGHT_SPK_MIXER
    0x30,   //!< MAX98090_REG_RW_SPK_CONTROL
    0x31,   //!< MAX98090_REG_RW_LEFT_SPK_VOLUME
    0x32,   //!< MAX98090_REG_RW_RIGHT_SPK_VOLUME
    0x33,   //!< MAX98090_REG_RW_ALC_TIMING
    0x34,   //!< MAX98090_REG_RW_ALC_COMPRESSOR
    0x35,   //!< MAX98090_REG_RW_ALC_EXPANDER
    0x36,   //!< MAX98090_REG_RW_ALC_GAIN
    0x37,   //!< MAX98090_REG_RW_RCV_OR_LOUTL_MIXER
    0x38,   //!< MAX98090_REG_RW_RCV_OR_LOUTL_CONTROL
    0x39,   //!< MAX98090_REG_RW_RCV_OR_LOUTL_VOLUME
    0x3A,   //!< MAX98090_REG_RW_LOUTR_MIXER
    0x3B,   //!< MAX98090_REG_RW_LOUTR_CONTROL
    0x3C,   //!< MAX98090_REG_RW_LOUTR_VOLUME
    0x3D,   //!< MAX98090_REG_RW_JACK_DETECT
    0x3E,   //!< MAX98090_REG_RW_INPUT_ENABLE
    0x3F,   //!< MAX98090_REG_RW_OUTPUT_ENABLE
    0x40,   //!< MAX98090_REG_RW_LEVEL_CONTROL
    0x41,   //!< MAX98090_REG_RW_DSP_FILTER_ENABLE
    0x42,   //!< MAX98090_REG_RW_BIAS_CONTROL
    0x43,   //!< MAX98090_REG_RW_DAC_CONTROL
    0x44,   //!< MAX98090_REG_RW_ADC_CONTROL
    0x45,   //!< MAX98090_REG_RW_DEVICE_SHUTDOWN
    0x46,   //!< MAX98090_REG_RW_EQ1B0_1
    0x47,   //!< MAX98090_REG_RW_EQ1B0_2
    0x48,   //!< MAX98090_REG_RW_EQ1B0_3
    0x49,   //!< MAX98090_REG_RW_EQ1B1_1
    0x4A,   //!< MAX98090_REG_RW_EQ1B1_2
    0x4B,   //!< MAX98090_REG_RW_EQ1B1_3
    0x4C,   //!< MAX98090_REG_RW_EQ1B2_1
    0x4D,   //!< MAX98090_REG_RW_EQ1B2_2
    0x4E,   //!< MAX98090_REG_RW_EQ1B2_3
    0x4F,   //!< MAX98090_REG_RW_EQ1A1_1
    0x50,   //!< MAX98090_REG_RW_EQ1A1_2
    0x51,   //!< MAX98090_REG_RW_EQ1A1_3
    0x52,   //!< MAX98090_REG_RW_EQ1A2_1
    0x53,   //!< MAX98090_REG_RW_EQ1A2_2
    0x54,   //!< MAX98090_REG_RW_EQ1A2_3
    0x55,   //!< MAX98090_REG_RW_EQ2B0_1
    0x56,   //!< MAX98090_REG_RW_EQ2B0_2
    0x57,   //!< MAX98090_REG_RW_EQ2B0_3
    0x58,   //!< MAX98090_REG_RW_EQ2B1_1
    0x59,   //!< MAX98090_REG_RW_EQ2B1_2
    0x5A,   //!< MAX98090_REG_RW_EQ2B1_3
    0x5B,   //!< MAX98090_REG_RW_EQ2B2_1
    0x5C,   //!< MAX98090_REG_RW_EQ2B2_2
    0x5D,   //!< MAX98090_REG_RW_EQ2B2_3
    0x5E,   //!< MAX98090_REG_RW_EQ2A1_1
    0x5F,   //!< MAX98090_REG_RW_EQ2A1_2
    0x60,   //!< MAX98090_REG_RW_EQ2A1_3
    0x61,   //!< MAX98090_REG_RW_EQ2A2_1
    0x62,   //!< MAX98090_REG_RW_EQ2A2_2
    0x63,   //!< MAX98090_REG_RW_EQ2A2_3
    0x64,   //!< MAX98090_REG_RW_EQ3B0_1
    0x65,   //!< MAX98090_REG_RW_EQ3B0_2
    0x66,   //!< MAX98090_REG_RW_EQ3B0_3
    0x67,   //!< MAX98090_REG_RW_EQ3B1_1
    0x68,   //!< MAX98090_REG_RW_EQ3B1_2
    0x69,   //!< MAX98090_REG_RW_EQ3B1_3
    0x6A,   //!< MAX98090_REG_RW_EQ3B2_1
    0x6B,   //!< MAX98090_REG_RW_EQ3B2_2
    0x6C,   //!< MAX98090_REG_RW_EQ3B2_3
    0x6D,   //!< MAX98090_REG_RW_EQ3A1_1
    0x6E,   //!< MAX98090_REG_RW_EQ3A1_2
    0x6F,   //!< MAX98090_REG_RW_EQ3A1_3
    0x70,   //!< MAX98090_REG_RW_EQ3A2_1
    0x71,   //!< MAX98090_REG_RW_EQ3A2_2
    0x72,   //!< MAX98090_REG_RW_EQ3A2_3
    0x73,   //!< MAX98090_REG_RW_EQ4B0_1
    0x74,   //!< MAX98090_REG_RW_EQ4B0_2
    0x75,   //!< MAX98090_REG_RW_EQ4B0_3
    0x76,   //!< MAX98090_REG_RW_EQ4B1_1
    0x77,   //!< MAX98090_REG_RW_EQ4B1_2
    0x78,   //!< MAX98090_REG_RW_EQ4B1_3
    0x79,   //!< MAX98090_REG_RW_EQ4B2_1
    0x7A,   //!< MAX98090_REG_RW_EQ4B2_2
    0x7B,   //!< MAX98090_REG_RW_EQ4B2_3
    0x7C,   //!< MAX98090_REG_RW_EQ4A1_1
    0x7D,   //!< MAX98090_REG_RW_EQ4A1_2
    0x7E,   //!< MAX98090_REG_RW_EQ4A1_3
    0x7F,   //!< MAX98090_REG_RW_EQ4A2_1
    0x80,   //!< MAX98090_REG_RW_EQ4A2_2
    0x81,   //!< MAX98090_REG_RW_EQ4A2_3
    0x82,   //!< MAX98090_REG_RW_EQ5B0_1
    0x83,   //!< MAX98090_REG_RW_EQ5B0_2
    0x84,   //!< MAX98090_REG_RW_EQ5B0_3
    0x85,   //!< MAX98090_REG_RW_EQ5B1_1
    0x86,   //!< MAX98090_REG_RW_EQ5B1_2
    0x87,   //!< MAX98090_REG_RW_EQ5B1_3
    0x88,   //!< MAX98090_REG_RW_EQ5B2_1
    0x89,   //!< MAX98090_REG_RW_EQ5B2_2
    0x8A,   //!< MAX98090_REG_RW_EQ5B2_3
    0x8B,   //!< MAX98090_REG_RW_EQ5A1_1
    0x8C,   //!< MAX98090_REG_RW_EQ5A1_2
    0x8D,   //!< MAX98090_REG_RW_EQ5A1_3
    0x8E,   //!< MAX98090_REG_RW_EQ5A2_1
    0x8F,   //!< MAX98090_REG_RW_EQ5A2_2
    0x90,   //!< MAX98090_REG_RW_EQ5A2_3
    0x91,   //!< MAX98090_REG_RW_EQ6B0_1
    0x92,   //!< MAX98090_REG_RW_EQ6B0_2
    0x93,   //!< MAX98090_REG_RW_EQ6B0_3
    0x94,   //!< MAX98090_REG_RW_EQ6B1_1
    0x95,   //!< MAX98090_REG_RW_EQ6B1_2
    0x96,   //!< MAX98090_REG_RW_EQ6B1_3
    0x97,   //!< MAX98090_REG_RW_EQ6B2_1
    0x98,   //!< MAX98090_REG_RW_EQ6B2_2
    0x99,   //!< MAX98090_REG_RW_EQ6B2_3
    0x9A,   //!< MAX98090_REG_RW_EQ6A1_1
    0x9B,   //!< MAX98090_REG_RW_EQ6A1_2
    0x9C,   //!< MAX98090_REG_RW_EQ6A1_3
    0x9D,   //!< MAX98090_REG_RW_EQ6A2_1
    0x9E,   //!< MAX98090_REG_RW_EQ6A2_2
    0x9F,   //!< MAX98090_REG_RW_EQ6A2_3
    0xA0,   //!< MAX98090_REG_RW_EQ7B0_1
    0xA1,   //!< MAX98090_REG_RW_EQ7B0_2
    0xA2,   //!< MAX98090_REG_RW_EQ7B0_3
    0xA3,   //!< MAX98090_REG_RW_EQ7B1_1
    0xA4,   //!< MAX98090_REG_RW_EQ7B1_2
    0xA5,   //!< MAX98090_REG_RW_EQ7B1_3
    0xA6,   //!< MAX98090_REG_RW_EQ7B2_1
    0xA7,   //!< MAX98090_REG_RW_EQ7B2_2
    0xA8,   //!< MAX98090_REG_RW_EQ7B2_3
    0xA9,   //!< MAX98090_REG_RW_EQ7A1_1
    0xAA,   //!< MAX98090_REG_RW_EQ7A1_2
    0xAB,   //!< MAX98090_REG_RW_EQ7A1_3
    0xAC,   //!< MAX98090_REG_RW_EQ7A2_1
    0xAD,   //!< MAX98090_REG_RW_EQ7A2_2
    0xAE,   //!< MAX98090_REG_RW_EQ7A2_3
    0xAF,   //!< MAX98090_REG_RW_ADCB0_1
    0xB0,   //!< MAX98090_REG_RW_ADCB0_2
    0xB1,   //!< MAX98090_REG_RW_ADCB0_3
    0xB2,   //!< MAX98090_REG_RW_ADCB1_1
    0xB3,   //!< MAX98090_REG_RW_ADCB1_2
    0xB4,   //!< MAX98090_REG_RW_ADCB1_3
    0xB5,   //!< MAX98090_REG_RW_ADCB2_1
    0xB6,   //!< MAX98090_REG_RW_ADCB2_2
    0xB7,   //!< MAX98090_REG_RW_ADCB2_3
    0xB8,   //!< MAX98090_REG_RW_ADCA1_1
    0xB9,   //!< MAX98090_REG_RW_ADCA1_2
    0xBA,   //!< MAX98090_REG_RW_ADCA1_3
    0xBB,   //!< MAX98090_REG_RW_ADCA2_1
    0xBC,   //!< MAX98090_REG_RW_ADCA2_2
    0xBD,   //!< MAX98090_REG_RW_ADCA2_3
    0xFF,   //!< MAX98090_REG_R_REVISION_ID
};

/*!
  @brief MAX98090 bit information table.
*/
const sMax98090BitInfo MAX98090_bit_info[MAX98090_BIT_ID_MAX] = {
    { 7, BIT7                                      }, //!< MAX98090_BIT_SOFTWARE_RESET_SWRESET
    { 7, BIT7                                      }, //!< MAX98090_BIT_DEVICE_STATUS_CLD
    { 6, BIT6                                      }, //!< MAX98090_BIT_DEVICE_STATUS_SLD
    { 5, BIT5                                      }, //!< MAX98090_BIT_DEVICE_STATUS_ULK
    { 2, BIT2                                      }, //!< MAX98090_BIT_DEVICE_STATUS_JDET
    { 1, BIT1                                      }, //!< MAX98090_BIT_DEVICE_STATUS_ALCACT
    { 0, BIT0                                      }, //!< MAX98090_BIT_DEVICE_STATUS_ALCCLP
    { 2, BIT2                                      }, //!< MAX98090_BIT_JACK_DETECT_LSNS
    { 1, BIT1                                      }, //!< MAX98090_BIT_JACK_DETECT_JKSNS
    { 7, BIT7                                      }, //!< MAX98090_BIT_INTERRUPT_MASKS_ICLD
    { 6, BIT6                                      }, //!< MAX98090_BIT_INTERRUPT_MASKS_ISLD
    { 5, BIT5                                      }, //!< MAX98090_BIT_INTERRUPT_MASKS_IULK
    { 2, BIT2                                      }, //!< MAX98090_BIT_INTERRUPT_MASKS_IJDET
    { 1, BIT1                                      }, //!< MAX98090_BIT_INTERRUPT_MASKS_IALCACT
    { 0, BIT0                                      }, //!< MAX98090_BIT_INTERRUPT_MASKS_IALCCLP
    { 7, BIT7                                      }, //!< MAX98090_BIT_SYSTEM_CLOCK_MCLK_26M
    { 6, BIT6                                      }, //!< MAX98090_BIT_SYSTEM_CLOCK_MCLK_19P2M
    { 5, BIT5                                      }, //!< MAX98090_BIT_SYSTEM_CLOCK_MCLK_13M
    { 4, BIT4                                      }, //!< MAX98090_BIT_SYSTEM_CLOCK_MCLK_12P288M
    { 3, BIT3                                      }, //!< MAX98090_BIT_SYSTEM_CLOCK_MCLK_12M
    { 2, BIT2                                      }, //!< MAX98090_BIT_SYSTEM_CLOCK_MCLK_11P2896M
    { 0, BIT0                                      }, //!< MAX98090_BIT_SYSTEM_CLOCK_MCLK_256Fs
    { 5, BIT5                                      }, //!< MAX98090_BIT_SAMPLE_RATE_SR_96K
    { 4, BIT4                                      }, //!< MAX98090_BIT_SAMPLE_RATE_SR_32K
    { 3, BIT3                                      }, //!< MAX98090_BIT_SAMPLE_RATE_SR_48K
    { 2, BIT2                                      }, //!< MAX98090_BIT_SAMPLE_RATE_SR_44K1
    { 1, BIT1                                      }, //!< MAX98090_BIT_SAMPLE_RATE_SR_16K
    { 0, BIT0                                      }, //!< MAX98090_BIT_SAMPLE_RATE_SR_8K
    { 5, BIT5                                      }, //!< MAX98090_BIT_INTERFACE_RJ_M
    { 4, BIT4                                      }, //!< MAX98090_BIT_INTERFACE_RJ_S
    { 3, BIT3                                      }, //!< MAX98090_BIT_INTERFACE_LJ_M
    { 2, BIT2                                      }, //!< MAX98090_BIT_INTERFACE_LJ_S
    { 1, BIT1                                      }, //!< MAX98090_BIT_INTERFACE_I2S_M
    { 0, BIT0                                      }, //!< MAX98090_BIT_INTERFACE_I2S_S
    { 7, BIT7                                      }, //!< MAX98090_BIT_DAC_PATH_DIG2_HP
    { 6, BIT6                                      }, //!< MAX98090_BIT_DAC_PATH_DIG2_EAR
    { 5, BIT5                                      }, //!< MAX98090_BIT_DAC_PATH_DIG2_SPK
    { 4, BIT4                                      }, //!< MAX98090_BIT_DAC_PATH_DIG2_LOUT
    { 7, BIT7                                      }, //!< MAX98090_BIT_MIC_OR_DIRECT_TO_ADC_IN12_MIC1
    { 6, BIT6                                      }, //!< MAX98090_BIT_MIC_OR_DIRECT_TO_ADC_IN34_MIC2
    { 5, BIT5                                      }, //!< MAX98090_BIT_MIC_OR_DIRECT_TO_ADC_IN56_MIC1
    { 4, BIT4                                      }, //!< MAX98090_BIT_MIC_OR_DIRECT_TO_ADC_IN56_MIC2
    { 3, BIT3                                      }, //!< MAX98090_BIT_MIC_OR_DIRECT_TO_ADC_IN12_DADC
    { 2, BIT2                                      }, //!< MAX98090_BIT_MIC_OR_DIRECT_TO_ADC_IN34_DADC
    { 1, BIT1                                      }, //!< MAX98090_BIT_MIC_OR_DIRECT_TO_ADC_IN56_DADC
    { 7, BIT7                                      }, //!< MAX98090_BIT_LINE_TO_ADC_IN12S_AB
    { 6, BIT6                                      }, //!< MAX98090_BIT_LINE_TO_ADC_IN34S_AB
    { 5, BIT5                                      }, //!< MAX98090_BIT_LINE_TO_ADC_IN56S_AB
    { 4, BIT4                                      }, //!< MAX98090_BIT_LINE_TO_ADC_IN34D_A
    { 3, BIT3                                      }, //!< MAX98090_BIT_LINE_TO_ADC_IN56D_B
    { 7, BIT7                                      }, //!< MAX98090_BIT_ANALOG_MIC_LOOP_IN12_M1HPL
    { 6, BIT6                                      }, //!< MAX98090_BIT_ANALOG_MIC_LOOP_IN12_M1SPKL
    { 5, BIT5                                      }, //!< MAX98090_BIT_ANALOG_MIC_LOOP_IN12_M1EAR
    { 4, BIT4                                      }, //!< MAX98090_BIT_ANALOG_MIC_LOOP_IN12_M1LOUTL
    { 3, BIT3                                      }, //!< MAX98090_BIT_ANALOG_MIC_LOOP_IN34_M2HPR
    { 2, BIT2                                      }, //!< MAX98090_BIT_ANALOG_MIC_LOOP_IN34_M2SPKR
    { 1, BIT1                                      }, //!< MAX98090_BIT_ANALOG_MIC_LOOP_IN34_M2EAR
    { 0, BIT0                                      }, //!< MAX98090_BIT_ANALOG_MIC_LOOP_N34_M2LOUTR
    { 7, BIT7                                      }, //!< MAX98090_BIT_ANALOG_LINE_LOOP_IN12S_ABHP
    { 6, BIT6                                      }, //!< MAX98090_BIT_ANALOG_LINE_LOOP_IN34D_ASPKL
    { 5, BIT5                                      }, //!< MAX98090_BIT_ANALOG_LINE_LOOP_IN34D_AEAR
    { 4, BIT4                                      }, //!< MAX98090_BIT_ANALOG_LINE_LOOP_IN12S_ABLOUT
    { 3, BIT3                                      }, //!< MAX98090_BIT_ANALOG_LINE_LOOP_IN34S_ABHP
    { 2, BIT2                                      }, //!< MAX98090_BIT_ANALOG_LINE_LOOP_IN56D_BSPKR
    { 1, BIT1                                      }, //!< MAX98090_BIT_ANALOG_LINE_LOOP_IN56D_BEAR
    { 0, BIT0                                      }, //!< MAX98090_BIT_ANALOG_LINE_LOOP_IN34S_ABLOUT
    { 6, (BIT7|BIT6)                               }, //!< MAX98090_BIT_DIGITAL_MIC_MICCLK
    { 5, BIT5                                      }, //!< MAX98090_BIT_DIGITAL_MIC_DIGMICR
    { 4, BIT4                                      }, //!< MAX98090_BIT_DIGITAL_MIC_DIGMICL
    { 7, BIT7                                      }, //!< MAX98090_BIT_INPUT_CONFIG_IN34DIFF
    { 6, BIT6                                      }, //!< MAX98090_BIT_INPUT_CONFIG_IN56DIFF
    { 5, BIT5                                      }, //!< MAX98090_BIT_INPUT_CONFIG_IN1SEEN
    { 4, BIT4                                      }, //!< MAX98090_BIT_INPUT_CONFIG_IN2SEEN
    { 3, BIT3                                      }, //!< MAX98090_BIT_INPUT_CONFIG_IN3SEEN
    { 2, BIT2                                      }, //!< MAX98090_BIT_INPUT_CONFIG_IN4SEEN
    { 1, BIT1                                      }, //!< MAX98090_BIT_INPUT_CONFIG_IN5SEEN
    { 0, BIT0                                      }, //!< MAX98090_BIT_INPUT_CONFIG_IN6SEEN
    { 7, BIT7                                      }, //!< MAX98090_BIT_LINE_INPUT_LEVEL_MIXG135
    { 6, BIT6                                      }, //!< MAX98090_BIT_LINE_INPUT_LEVEL_MIXG246
    { 3, (BIT5|BIT4|BIT3)                          }, //!< MAX98090_BIT_LINE_INPUT_LEVEL_LINAPGA
    { 0, (BIT2|BIT1|BIT0)                          }, //!< MAX98090_BIT_LINE_INPUT_LEVEL_LINBPGA
    { 7, BIT7                                      }, //!< MAX98090_BIT_LINE_CONFIG_EXTBUFA
    { 6, BIT6                                      }, //!< MAX98090_BIT_LINE_CONFIG_EXTBUFB
    { 0, (BIT1|BIT0)                               }, //!< MAX98090_BIT_LINE_CONFIG_EXTMIC
    { 5, (BIT6|BIT5)                               }, //!< MAX98090_BIT_MIC1_INPUT_LEVEL_PA1EN
    { 0, (BIT4|BIT3|BIT2|BIT1|BIT0)                }, //!< MAX98090_BIT_MIC1_INPUT_LEVEL_PGAM1
    { 5, (BIT6|BIT5)                               }, //!< MAX98090_BIT_MIC2_INPUT_LEVEL_PA2EN
    { 0, (BIT4|BIT3|BIT2|BIT1|BIT0)                }, //!< MAX98090_BIT_MIC2_INPUT_LEVEL_PGAM2
    { 0, (BIT1|BIT0)                               }, //!< MAX98090_BIT_MIC_BIAS_VOLTAGE_MBVSEL
    { 0, BIT7                                      }, //!< MAX98090_BIT_LEFT_ADC_MIXER_MIXADL7
    { 1, BIT6                                      }, //!< MAX98090_BIT_LEFT_ADC_MIXER_MIXADL6
    { 2, BIT5                                      }, //!< MAX98090_BIT_LEFT_ADC_MIXER_MIXADL5
    { 3, BIT4                                      }, //!< MAX98090_BIT_LEFT_ADC_MIXER_MIXADL4
    { 4, BIT3                                      }, //!< MAX98090_BIT_LEFT_ADC_MIXER_MIXADL3
    { 5, BIT2                                      }, //!< MAX98090_BIT_LEFT_ADC_MIXER_MIXADL2
    { 6, BIT1                                      }, //!< MAX98090_BIT_LEFT_ADC_MIXER_MIXADL1
    { 7, BIT0                                      }, //!< MAX98090_BIT_LEFT_ADC_MIXER_MIXADL0
    { 0, BIT7                                      }, //!< MAX98090_BIT_RIGHT_ADC_MIXER_MIXADR7
    { 1, BIT6                                      }, //!< MAX98090_BIT_RIGHT_ADC_MIXER_MIXADR6
    { 2, BIT5                                      }, //!< MAX98090_BIT_RIGHT_ADC_MIXER_MIXADR5
    { 3, BIT4                                      }, //!< MAX98090_BIT_RIGHT_ADC_MIXER_MIXADR4
    { 4, BIT3                                      }, //!< MAX98090_BIT_RIGHT_ADC_MIXER_MIXADR3
    { 5, BIT2                                      }, //!< MAX98090_BIT_RIGHT_ADC_MIXER_MIXADR2
    { 6, BIT1                                      }, //!< MAX98090_BIT_RIGHT_ADC_MIXER_MIXADR1
    { 7, BIT0                                      }, //!< MAX98090_BIT_RIGHT_ADC_MIXER_MIXADR0
    { 4, (BIT5|BIT4)                               }, //!< MAX98090_BIT_LEFT_ADC_LEVEL_AVLG
    { 0, (BIT3|BIT2|BIT1|BIT0)                     }, //!< MAX98090_BIT_LEFT_ADC_LEVEL_AVL
    { 5, (BIT5|BIT4)                               }, //!< MAX98090_BIT_RIGHT_ADC_LEVEL_AVRG
    { 0, (BIT3|BIT2|BIT1|BIT0)                     }, //!< MAX98090_BIT_RIGHT_ADC_LEVEL_AVR
    { 0, (BIT3|BIT2|BIT1|BIT0)                     }, //!< MAX98090_BIT_ADC_BIQUAD_LEVEL_AVBQ
    { 6, (BIT7|BIT6)                               }, //!< MAX98090_BIT_ADC_SIDETONE_DSTS
    { 0, (BIT4|BIT3|BIT2|BIT1|BIT0)                }, //!< MAX98090_BIT_ADC_SIDETONE_DVST
    { 4, (BIT5|BIT4)                               }, //!< MAX98090_BIT_SYSTEM_CLOCK_PSCLK
    { 4, (BIT7|BIT6|BIT5|BIT4)                     }, //!< MAX98090_BIT_CLOCK_MODE_FREQ1
    { 0, BIT0                                      }, //!< MAX98090_BIT_CLOCK_MODE_USE_MI1
    { 0, (BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)      }, //!< MAX98090_BIT_ANY_CLOCK_1_NI1
    { 0, (BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)      }, //!< MAX98090_BIT_ANY_CLOCK_2_NI1
    { 0, (BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)      }, //!< MAX98090_BIT_ANY_CLOCK_3_MI1
    { 0, (BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)      }, //!< MAX98090_BIT_ANY_CLOCK_4_MI1
    { 7, BIT7                                      }, //!< MAX98090_BIT_MASTER_MODE_MAS
    { 0, (BIT2|BIT1|BIT0)                          }, //!< MAX98090_BIT_MASTER_MODE_BSEL
    { 5, BIT5                                      }, //!< MAX98090_BIT_INTERFACE_FORMAT_RJ
    { 4, BIT4                                      }, //!< MAX98090_BIT_INTERFACE_FORMAT_WCI1
    { 4, BIT4                                      }, //!< MAX98090_BIT_INTERFACE_FORMAT_WCI2
    { 3, BIT3                                      }, //!< MAX98090_BIT_INTERFACE_FORMAT_BCI1
    { 3, BIT3                                      }, //!< MAX98090_BIT_INTERFACE_FORMAT_BCI2
    { 2, BIT2                                      }, //!< MAX98090_BIT_INTERFACE_FORMAT_DLY
    { 0, (BIT1|BIT0)                               }, //!< MAX98090_BIT_INTERFACE_FORMAT_WS2
    { 1, BIT1                                      }, //!< MAX98090_BIT_TDM_FORMAT_1_FSW
    { 0, BIT0                                      }, //!< MAX98090_BIT_TDM_FORMAT_1_TDM
    { 6, (BIT7|BIT6)                               }, //!< MAX98090_BIT_TDM_FORMAT_2_SLOTL
    { 4, (BIT5|BIT4)                               }, //!< MAX98090_BIT_TDM_FORMAT_2_SLOTR
    { 0, BIT3                                      }, //!< MAX98090_BIT_TDM_FORMAT_2_SLOTDLY4
    { 0, BIT2                                      }, //!< MAX98090_BIT_TDM_FORMAT_2_SLOTDLY3
    { 0, BIT1                                      }, //!< MAX98090_BIT_TDM_FORMAT_2_SLOTDLY2
    { 0, BIT0                                      }, //!< MAX98090_BIT_TDM_FORMAT_2_SLOTDLY1
    { 5, BIT5                                      }, //!< MAX98090_BIT_I_OR_O_CONFIGURATION_LTEN
    { 4, BIT4                                      }, //!< MAX98090_BIT_I_OR_O_CONFIGURATION_LBEN
    { 3, BIT3                                      }, //!< MAX98090_BIT_I_OR_O_CONFIGURATION_DMONO
    { 2, BIT2                                      }, //!< MAX98090_BIT_I_OR_O_CONFIGURATION_HIZOFF
    { 1, BIT1                                      }, //!< MAX98090_BIT_I_OR_O_CONFIGURATION_SDOEN
    { 0, BIT0                                      }, //!< MAX98090_BIT_I_OR_O_CONFIGURATION_SDIEN
    { 7, BIT7                                      }, //!< MAX98090_BIT_FILTER_CONFIG_MODE
    { 6, BIT6                                      }, //!< MAX98090_BIT_FILTER_CONFIG_AHPF
    { 5, BIT5                                      }, //!< MAX98090_BIT_FILTER_CONFIG_DHPF
    { 4, BIT4                                      }, //!< MAX98090_BIT_FILTER_CONFIG_DHF
    { 7, BIT7                                      }, //!< MAX98090_BIT_PLAYBACK_LEVEL_1_DV1M
    { 4, (BIT5|BIT4)                               }, //!< MAX98090_BIT_PLAYBACK_LEVEL_1_DV1G
    { 0, (BIT3|BIT2|BIT1|BIT0)                     }, //!< MAX98090_BIT_PLAYBACK_LEVEL_1_DV1
    { 4, BIT4                                      }, //!< MAX98090_BIT_PLAYBACK_LEVEL_2_EQCLP
    { 0, (BIT3|BIT2|BIT1|BIT0)                     }, //!< MAX98090_BIT_PLAYBACK_LEVEL_2_DVEQ
    { 5, BIT5                                      }, //!< MAX98090_BIT_LEFT_HP_MIXER_MIXHPL5
    { 4, BIT4                                      }, //!< MAX98090_BIT_LEFT_HP_MIXER_MIXHPL4
    { 3, BIT3                                      }, //!< MAX98090_BIT_LEFT_HP_MIXER_MIXHPL3
    { 2, BIT2                                      }, //!< MAX98090_BIT_LEFT_HP_MIXER_MIXHPL2
    { 1, BIT1                                      }, //!< MAX98090_BIT_LEFT_HP_MIXER_MIXHPL1
    { 0, BIT0                                      }, //!< MAX98090_BIT_LEFT_HP_MIXER_MIXHPL0
    { 5, BIT5                                      }, //!< MAX98090_BIT_RIGHT_HP_MIXER_MIXHPR5
    { 4, BIT4                                      }, //!< MAX98090_BIT_RIGHT_HP_MIXER_MIXHPR4
    { 3, BIT3                                      }, //!< MAX98090_BIT_RIGHT_HP_MIXER_MIXHPR3
    { 2, BIT2                                      }, //!< MAX98090_BIT_RIGHT_HP_MIXER_MIXHPR2
    { 1, BIT1                                      }, //!< MAX98090_BIT_RIGHT_HP_MIXER_MIXHPR1
    { 0, BIT0                                      }, //!< MAX98090_BIT_RIGHT_HP_MIXER_MIXHPR0
    { 5, BIT5                                      }, //!< MAX98090_BIT_HP_CONTROL_MIXHPRSEL
    { 4, BIT4                                      }, //!< MAX98090_BIT_HP_CONTROL_MIXHPLSEL
    { 2, (BIT3|BIT2)                               }, //!< MAX98090_BIT_HP_CONTROL_MIXHPRG
    { 0, (BIT1|BIT0)                               }, //!< MAX98090_BIT_HP_CONTROL_MIXHPLG
    { 7, BIT7                                      }, //!< MAX98090_BIT_LEFT_HP_VOLUME_HPLM
    { 0, (BIT4|BIT3|BIT2|BIT1|BIT0)                }, //!< MAX98090_BIT_LEFT_HP_VOLUME_HPVOLL
    { 7, BIT7                                      }, //!< MAX98090_BIT_RIGHT_HP_VOLUME_HPRM
    { 0, (BIT4|BIT3|BIT2|BIT1|BIT0)                }, //!< MAX98090_BIT_RIGHT_HP_VOLUME_HPVOLR
    { 5, BIT5                                      }, //!< MAX98090_BIT_LEFT_SPK_MIXER_MIXSPL5
    { 4, BIT4                                      }, //!< MAX98090_BIT_LEFT_SPK_MIXER_MIXSPL4
    { 3, BIT3                                      }, //!< MAX98090_BIT_LEFT_SPK_MIXER_MIXSPL3
    { 2, BIT2                                      }, //!< MAX98090_BIT_LEFT_SPK_MIXER_MIXSPL2
    { 1, BIT1                                      }, //!< MAX98090_BIT_LEFT_SPK_MIXER_MIXSPL1
    { 0, BIT0                                      }, //!< MAX98090_BIT_LEFT_SPK_MIXER_MIXSPL0
    { 6, BIT6                                      }, //!< MAX98090_BIT_RIGHT_SPK_MIXER_SPK_SLAVE
    { 5, BIT5                                      }, //!< MAX98090_BIT_RIGHT_SPK_MIXER_MIXSPR5
    { 4, BIT4                                      }, //!< MAX98090_BIT_RIGHT_SPK_MIXER_MIXSPR4
    { 3, BIT3                                      }, //!< MAX98090_BIT_RIGHT_SPK_MIXER_MIXSPR3
    { 2, BIT2                                      }, //!< MAX98090_BIT_RIGHT_SPK_MIXER_MIXSPR2
    { 1, BIT1                                      }, //!< MAX98090_BIT_RIGHT_SPK_MIXER_MIXSPR1
    { 0, BIT0                                      }, //!< MAX98090_BIT_RIGHT_SPK_MIXER_MIXSPR0
    { 2, (BIT3|BIT2)                               }, //!< MAX98090_BIT_SPK_CONTROL_MIXSPRG
    { 0, (BIT1|BIT0)                               }, //!< MAX98090_BIT_SPK_CONTROL_MIXSPLG
    { 7, BIT7                                      }, //!< MAX98090_BIT_LEFT_SPK_VOLUME_SPLM
    { 0, (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)           }, //!< MAX98090_BIT_LEFT_SPK_VOLUME_SPVOLL
    { 7, BIT7                                      }, //!< MAX98090_BIT_RIGHT_SPK_VOLUME_SPRM
    { 0, (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)           }, //!< MAX98090_BIT_RIGHT_SPK_VOLUME_SPVOLR
    { 7, BIT7                                      }, //!< MAX98090_BIT_ALC_TIMING_ALCEN
    { 4, (BIT6|BIT5|BIT4)                          }, //!< MAX98090_BIT_ALC_TIMING_ALCRLS
    { 0, (BIT2|BIT1|BIT0)                          }, //!< MAX98090_BIT_ALC_TIMING_ALCATK
    { 5, (BIT7|BIT6|BIT5)                          }, //!< MAX98090_BIT_ALC_COMPRESSOR_ALCCMP
    { 0, (BIT4|BIT3|BIT2|BIT1|BIT0)                }, //!< MAX98090_BIT_ALC_COMPRESSOR_ALCTHC
    { 5, (BIT7|BIT6|BIT5)                          }, //!< MAX98090_BIT_ALC_EXPANDER_ALCEXP
    { 0, (BIT4|BIT3|BIT2|BIT1|BIT0)                }, //!< MAX98090_BIT_ALC_EXPANDER_ALCTHE
    { 0, (BIT4|BIT3|BIT2|BIT1|BIT0)                }, //!< MAX98090_BIT_ALC_GAIN_ALCG
    { 5, BIT5                                      }, //!< MAX98090_BIT_RCV_OR_LOUTL_MIXER_MIXRCVL5
    { 4, BIT4                                      }, //!< MAX98090_BIT_RCV_OR_LOUTL_MIXER_MIXRCVL4
    { 3, BIT3                                      }, //!< MAX98090_BIT_RCV_OR_LOUTL_MIXER_MIXRCVL3
    { 2, BIT2                                      }, //!< MAX98090_BIT_RCV_OR_LOUTL_MIXER_MIXRCVL2
    { 1, BIT1                                      }, //!< MAX98090_BIT_RCV_OR_LOUTL_MIXER_MIXRCVL1
    { 0, BIT0                                      }, //!< MAX98090_BIT_RCV_OR_LOUTL_MIXER_MIXRCVL0
    { 0, (BIT1|BIT0)                               }, //!< MAX98090_BIT_RCV_OR_LOUTL_CONTROL_MIXRCVLG
    { 7, BIT7                                      }, //!< MAX98090_BIT_RCV_OR_LOUTL_VOLUME_RCVLM
    { 0, (BIT4|BIT3|BIT2|BIT1|BIT0)                }, //!< MAX98090_BIT_RCV_OR_LOUTL_VOLUME_RCVLVOL
    { 7, BIT7                                      }, //!< MAX98090_BIT_LOUTR_MIXER_LINMOD
    { 5, BIT5                                      }, //!< MAX98090_BIT_LOUTR_MIXER_MIXRCVR5
    { 4, BIT4                                      }, //!< MAX98090_BIT_LOUTR_MIXER_MIXRCVR4
    { 3, BIT3                                      }, //!< MAX98090_BIT_LOUTR_MIXER_MIXRCVR3
    { 2, BIT2                                      }, //!< MAX98090_BIT_LOUTR_MIXER_MIXRCVR2
    { 1, BIT1                                      }, //!< MAX98090_BIT_LOUTR_MIXER_MIXRCVR1
    { 0, BIT0                                      }, //!< MAX98090_BIT_LOUTR_MIXER_MIXRCVR0
    { 0, (BIT1|BIT0)                               }, //!< MAX98090_BIT_LOUTR_CONTROL_MIXRCVRG
    { 7, BIT7                                      }, //!< MAX98090_BIT_LOUTR_VOLUME_RCVRM
    { 0, (BIT4|BIT3|BIT2|BIT1|BIT0)                }, //!< MAX98090_BIT_LOUTR_VOLUME_RCVRVOL
    { 7, BIT7                                      }, //!< MAX98090_BIT_JACK_DETECT_JDETEN
    { 6, BIT6                                      }, //!< MAX98090_BIT_JACK_DETECT_JKWK
    { 0, (BIT1|BIT0)                               }, //!< MAX98090_BIT_JACK_DETECT_JDEB
    { 4, BIT4                                      }, //!< MAX98090_BIT_INPUT_ENABLE_MBEN
    { 3, BIT3                                      }, //!< MAX98090_BIT_INPUT_ENABLE_LINEAEN
    { 2, BIT2                                      }, //!< MAX98090_BIT_INPUT_ENABLE_LINEBEN
    { 1, BIT1                                      }, //!< MAX98090_BIT_INPUT_ENABLE_ADREN
    { 0, BIT0                                      }, //!< MAX98090_BIT_INPUT_ENABLE_ADLEN
    { 7, BIT7                                      }, //!< MAX98090_BIT_OUTPUT_ENABLE_HPREN
    { 6, BIT6                                      }, //!< MAX98090_BIT_OUTPUT_ENABLE_HPLEN
    { 5, BIT5                                      }, //!< MAX98090_BIT_OUTPUT_ENABLE_SPREN
    { 4, BIT4                                      }, //!< MAX98090_BIT_OUTPUT_ENABLE_SPLEN
    { 3, BIT3                                      }, //!< MAX98090_BIT_OUTPUT_ENABLE_RCVLEN
    { 2, BIT2                                      }, //!< MAX98090_BIT_OUTPUT_ENABLE_RCVREN
    { 1, BIT1                                      }, //!< MAX98090_BIT_OUTPUT_ENABLE_DAREN
    { 0, BIT0                                      }, //!< MAX98090_BIT_OUTPUT_ENABLE_DALEN
    { 2, BIT2                                      }, //!< MAX98090_BIT_LEVEL_CONTROL_ZDEN
    { 1, BIT1                                      }, //!< MAX98090_BIT_LEVEL_CONTROL_VS2EN
    { 0, BIT0                                      }, //!< MAX98090_BIT_LEVEL_CONTROL_VSEN
    { 3, BIT3                                      }, //!< MAX98090_BIT_DSP_FILTER_ENABLE_ADCBQEN
    { 2, BIT2                                      }, //!< MAX98090_BIT_DSP_FILTER_ENABLE_EQ3BANDEN
    { 1, BIT1                                      }, //!< MAX98090_BIT_DSP_FILTER_ENABLE_EQ5BANDEN
    { 0, BIT0                                      }, //!< MAX98090_BIT_DSP_FILTER_ENABLE_EQ7BANDEN
    { 0, BIT0                                      }, //!< MAX98090_BIT_BIAS_CONTROL_VCM_MODE
    { 1, BIT1                                      }, //!< MAX98090_BIT_DAC_CONTROL_PERFMODE
    { 0, BIT0                                      }, //!< MAX98090_BIT_DAC_CONTROL_DACHP
    { 2, BIT2                                      }, //!< MAX98090_BIT_ADC_CONTROL_OSR128
    { 1, BIT1                                      }, //!< MAX98090_BIT_ADC_CONTROL_ADCDITHER
    { 0, BIT0                                      }, //!< MAX98090_BIT_ADC_CONTROL_ADCHP
    { 7, BIT7                                      }, //!< MAX98090_BIT_DEVICE_SHUTDOWN_SHDN
    { 0, (BIT7|BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0) }, //!< MAX98090_BIT_REVISION_ID_REV_ID
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

#endif  /* __MAX98090_DEFS_H__ */
