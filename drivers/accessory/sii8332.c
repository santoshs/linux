/***************************************************************************
*
*   Silicon Image SiI8332 MHL Transmitter Driver
*
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation version 2.
*
* This program is distributed "as is" WITHOUT ANY WARRANTY of any
* kind, whether express or implied; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*****************************************************************************/

/* ========================================================

daniel.lee@siliconimage.com

========================================================*/
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/i2c.h>
#include <linux/gpio.h>

#include <asm/irq.h>
#include <linux/delay.h>

#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/fcntl.h>
#include <asm/uaccess.h>

#include "sii8332.h"
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/input.h>


//#define SFEATURE_HDCP_SUPPORT

static int __devinit simg72_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int __devinit simg7A_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int __devinit simg92_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int __devinit simg9A_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int __devinit simgC8_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int __devinit simgA0_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int __devinit simg60_probe(struct i2c_client *client, const struct i2c_device_id *id);

static int __devexit simg72_remove(struct i2c_client *client);
static int __devexit simg7A_remove(struct i2c_client *client);
static int __devexit simg92_remove(struct i2c_client *client);
static int __devexit simg9A_remove(struct i2c_client *client);
static int __devexit simgC8_remove(struct i2c_client *client);
static int __devexit simgA0_remove(struct i2c_client *client);
static int __devexit simg60_remove(struct i2c_client *client);


#define DEBUG_MHL_FUNC_START() printk(KERN_INFO"sii8332 %s, %d \n",__func__, __LINE__)

#define	MHL_MAX_RCP_KEY_CODE	(0x7F + 1)	// inclusive
u8 rcpSupportTable [MHL_MAX_RCP_KEY_CODE] = {
	(MHL_DEV_LD_GUI),		// 0x00 = Select
	(MHL_DEV_LD_GUI),		// 0x01 = Up
	(MHL_DEV_LD_GUI),		// 0x02 = Down
	(MHL_DEV_LD_GUI),		// 0x03 = Left
	(MHL_DEV_LD_GUI),		// 0x04 = Right
	0, 0, 0, 0,				// 05-08 Reserved
	(MHL_DEV_LD_GUI),		// 0x09 = Root Menu
	0, 0, 0,				// 0A-0C Reserved
	(MHL_DEV_LD_GUI),		// 0x0D = Select
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	// 0E-1F Reserved
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO | MHL_DEV_LD_MEDIA | MHL_DEV_LD_TUNER),	// Numeric keys 0x20-0x29
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO | MHL_DEV_LD_MEDIA | MHL_DEV_LD_TUNER),
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO | MHL_DEV_LD_MEDIA | MHL_DEV_LD_TUNER),
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO | MHL_DEV_LD_MEDIA | MHL_DEV_LD_TUNER),
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO | MHL_DEV_LD_MEDIA | MHL_DEV_LD_TUNER),
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO | MHL_DEV_LD_MEDIA | MHL_DEV_LD_TUNER),
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO | MHL_DEV_LD_MEDIA | MHL_DEV_LD_TUNER),
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO | MHL_DEV_LD_MEDIA | MHL_DEV_LD_TUNER),
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO | MHL_DEV_LD_MEDIA | MHL_DEV_LD_TUNER),
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO | MHL_DEV_LD_MEDIA | MHL_DEV_LD_TUNER),
	0,						// 0x2A = Dot
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO | MHL_DEV_LD_MEDIA | MHL_DEV_LD_TUNER),	// Enter key = 0x2B
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO | MHL_DEV_LD_MEDIA | MHL_DEV_LD_TUNER),	// Clear key = 0x2C
	0, 0, 0,				// 2D-2F Reserved
	(MHL_DEV_LD_TUNER),		// 0x30 = Channel Up
	(MHL_DEV_LD_TUNER),		// 0x31 = Channel Dn
	(MHL_DEV_LD_TUNER),		// 0x32 = Previous Channel
	(MHL_DEV_LD_AUDIO),		// 0x33 = Sound Select
	0,						// 0x34 = Input Select
	0,						// 0x35 = Show Information
	0,						// 0x36 = Help
	0,						// 0x37 = Page Up
	0,						// 0x38 = Page Down
	0, 0, 0, 0, 0, 0, 0,	// 0x39-0x3F Reserved
	0,						// 0x40 = Undefined

	(MHL_DEV_LD_SPEAKER),	// 0x41 = Volume Up
	(MHL_DEV_LD_SPEAKER),	// 0x42 = Volume Down
	(MHL_DEV_LD_SPEAKER),	// 0x43 = Mute
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO),	// 0x44 = Play
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO | MHL_DEV_LD_RECORD),	// 0x45 = Stop
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO | MHL_DEV_LD_RECORD),	// 0x46 = Pause
	(MHL_DEV_LD_RECORD),	// 0x47 = Record
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO),	// 0x48 = Rewind
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO),	// 0x49 = Fast Forward
	(MHL_DEV_LD_MEDIA),		// 0x4A = Eject
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO | MHL_DEV_LD_MEDIA),	// 0x4B = Forward
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO | MHL_DEV_LD_MEDIA),	// 0x4C = Backward
	0, 0, 0,				// 4D-4F Reserved
	0,						// 0x50 = Angle
	0,						// 0x51 = Subpicture
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 52-5F Reserved
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO),	// 0x60 = Play Function
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO),	// 0x61 = Pause the Play Function
	(MHL_DEV_LD_RECORD),	// 0x62 = Record Function
	(MHL_DEV_LD_RECORD),	// 0x63 = Pause the Record Function
	(MHL_DEV_LD_VIDEO | MHL_DEV_LD_AUDIO | MHL_DEV_LD_RECORD),	// 0x64 = Stop Function

	(MHL_DEV_LD_SPEAKER),	// 0x65 = Mute Function
	(MHL_DEV_LD_SPEAKER),	// 0x66 = Restore Mute Function
	0, 0, 0, 0, 0, 0, 0, 0, 0,	                        // 0x67-0x6F Undefined or reserved
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0		// 0x70-0x7F Undefined or reserved
};

video_mode_reg regList1920x1080[20]=
{
     {PAGE_0, 0x00, 0x00} //
    ,{PAGE_0, 0x01, 0x1D} //
    ,{PAGE_3, 0x8F, 0xA1} // HS_POL, VS_POL active high
    ,{PAGE_3, 0xD6, 0x7E} // HS Front Porch Low
    ,{PAGE_3, 0xD7, 0x02} // HS Front Porch High
    ,{PAGE_3, 0xD8, 0x2C} // HS Pulse Width Low
    ,{PAGE_3, 0xD9, 0x00} // HS Pulse Width High
    ,{PAGE_3, 0xDA, 0xBE} // H Total Pixels Low
    ,{PAGE_3, 0xDB, 0x0A} // H Total Pixels High
    ,{PAGE_3, 0xDC, 0x80} // H Active Pixels Low
    ,{PAGE_3, 0xDD, 0x07} // H Active Pixels High
    ,{PAGE_3, 0xDE, 0x38} // V Active Lines Low
    ,{PAGE_3, 0xDF, 0x04} // V Active Lines High
    ,{PAGE_3, 0xE0, 0x04} // VS Front Porch Low
    ,{PAGE_3, 0xE1, 0x00} // VS Front Porch High
    ,{PAGE_3, 0xE2, 0x05} // VS Pulse Width Low
    ,{PAGE_3, 0xE3, 0x00} // VS Pulse Width High
    ,{PAGE_3, 0xE4, 0x65} // V Total Lines Low
    ,{PAGE_3, 0xE5, 0x04} // V Total Lines High
    ,{0xFF, 0xFF, 0x00} // end of row
};

video_mode_reg regList1024x768[20]=
{
     {PAGE_0, 0x00, 0x00} //
    ,{PAGE_0, 0x01, 0x1D} //
    ,{PAGE_3, 0x8F, 0xA1} // HS_POL, VS_POL active high
    ,{PAGE_3, 0xD6, 0xA0} // HS Front Porch Low
    ,{PAGE_3, 0xD7, 0x00} // HS Front Porch High
    ,{PAGE_3, 0xD8, 0x88} // HS Pulse Width Low
    ,{PAGE_3, 0xD9, 0x00} // HS Pulse Width High
    ,{PAGE_3, 0xDA, 0x80} // H Total Pixels Low
    ,{PAGE_3, 0xDB, 0x05} // H Total Pixels High
    ,{PAGE_3, 0xDC, 0x00} // H Active Pixels Low
    ,{PAGE_3, 0xDD, 0x04} // H Active Pixels High
    ,{PAGE_3, 0xDE, 0x00} // V Active Lines Low
    ,{PAGE_3, 0xDF, 0x03} // V Active Lines High
    ,{PAGE_3, 0xE0, 0x03} // VS Front Porch Low
    ,{PAGE_3, 0xE1, 0x00} // VS Front Porch High
    ,{PAGE_3, 0xE2, 0x06} // VS Pulse Width Low
    ,{PAGE_3, 0xE3, 0x00} // VS Pulse Width High
    ,{PAGE_3, 0xE4, 0x26} // V Total Lines Low
    ,{PAGE_3, 0xE5, 0x03} // V Total Lines High
    ,{0xFF, 0xFF, 0x00} // end of row
};

video_mode_reg regList1280x720[20]=
{
     {PAGE_0, 0x00, 0x00} //
    ,{PAGE_0, 0x01, 0x1D} //
    ,{PAGE_3, 0x8F, 0xA1} // HS_POL, VS_POL active high
    ,{PAGE_3, 0xD6, 0x6E} // HS Front Porch Low
    ,{PAGE_3, 0xD7, 0x00} // HS Front Porch High
    ,{PAGE_3, 0xD8, 0x28} // HS Pulse Width Low
    ,{PAGE_3, 0xD9, 0x00} // HS Pulse Width High
    ,{PAGE_3, 0xDA, 0x72} // H Total Pixels Low
    ,{PAGE_3, 0xDB, 0x06} // H Total Pixels High
    ,{PAGE_3, 0xDC, 0x00} // H Active Pixels Low
    ,{PAGE_3, 0xDD, 0x05} // H Active Pixels High
    ,{PAGE_3, 0xDE, 0xD0} // V Active Lines Low
    ,{PAGE_3, 0xDF, 0x02} // V Active Lines High
    ,{PAGE_3, 0xE0, 0x05} // VS Front Porch Low
    ,{PAGE_3, 0xE1, 0x00} // VS Front Porch High
    ,{PAGE_3, 0xE2, 0x05} // VS Pulse Width Low
    ,{PAGE_3, 0xE3, 0x00} // VS Pulse Width High
    ,{PAGE_3, 0xE4, 0xEE} // V Total Lines Low
    ,{PAGE_3, 0xE5, 0x02} // V Total Lines High
    ,{0xFF, 0xFF, 0x00} // end of row
};


video_mode_reg regList800x600[20]=
{
     {PAGE_0, 0x00, 0x00} //
    ,{PAGE_0, 0x01, 0x1D} //
    ,{PAGE_3, 0x8F, 0xA1} // HS_POL, VS_POL active high
    ,{PAGE_3, 0xD6, 0x28} // HS Front Porch Low
    ,{PAGE_3, 0xD7, 0x00} // HS Front Porch High
    ,{PAGE_3, 0xD8, 0x80} // HS Pulse Width Low
    ,{PAGE_3, 0xD9, 0x00} // HS Pulse Width High
    ,{PAGE_3, 0xDA, 0x20} // H Total Pixels Low
    ,{PAGE_3, 0xDB, 0x04} // H Total Pixels High
    ,{PAGE_3, 0xDC, 0x20} // H Active Pixels Low
    ,{PAGE_3, 0xDD, 0x03} // H Active Pixels High
    ,{PAGE_3, 0xDE, 0x58} // V Active Lines Low
    ,{PAGE_3, 0xDF, 0x02} // V Active Lines High
    ,{PAGE_3, 0xE0, 0x01} // VS Front Porch Low
    ,{PAGE_3, 0xE1, 0x00} // VS Front Porch High
    ,{PAGE_3, 0xE2, 0x04} // VS Pulse Width Low
    ,{PAGE_3, 0xE3, 0x00} // VS Pulse Width High
    ,{PAGE_3, 0xE4, 0x74} // V Total Lines Low
    ,{PAGE_3, 0xE5, 0x02} // V Total Lines High
    ,{0xFF, 0xFF, 0x00} // end of row
};

video_mode_reg regList720x576[20]=
{
     {PAGE_0, 0x00, 0x8C} //
    ,{PAGE_0, 0x01, 0x0A} //
    ,{PAGE_3, 0x8F, 0xAD} // HS_POL, VS_POL active high
    ,{PAGE_3, 0xD6, 0x0C} // HS Front Porch Low
    ,{PAGE_3, 0xD7, 0x00} // HS Front Porch High
    ,{PAGE_3, 0xD8, 0x40} // HS Pulse Width Low
    ,{PAGE_3, 0xD9, 0x00} // HS Pulse Width High
    ,{PAGE_3, 0xDA, 0x60} // H Total Pixels Low
    ,{PAGE_3, 0xDB, 0x03} // H Total Pixels High
    ,{PAGE_3, 0xDC, 0xD0} // H Active Pixels Low
    ,{PAGE_3, 0xDD, 0x02} // H Active Pixels High
    ,{PAGE_3, 0xDE, 0x40} // V Active Lines Low
    ,{PAGE_3, 0xDF, 0x02} // V Active Lines High
    ,{PAGE_3, 0xE0, 0x05} // VS Front Porch Low
    ,{PAGE_3, 0xE1, 0x00} // VS Front Porch High
    ,{PAGE_3, 0xE2, 0x05} // VS Pulse Width Low
    ,{PAGE_3, 0xE3, 0x00} // VS Pulse Width High
    ,{PAGE_3, 0xE4, 0x71} // V Total Lines Low
    ,{PAGE_3, 0xE5, 0x02} // V Total Lines High
    ,{0xFF, 0xFF, 0x00} // end of row
};

video_mode_reg regList1920x1080i[20]=
{
     {PAGE_0, 0x00, 0x00} //
    ,{PAGE_0, 0x01, 0x1D} //
    ,{PAGE_3, 0x8F, 0xA1} // HS_POL, VS_POL active high
    ,{PAGE_3, 0xD6, 0x58} // HS Front Porch Low
    ,{PAGE_3, 0xD7, 0x00} // HS Front Porch High
    ,{PAGE_3, 0xD8, 0x2C} // HS Pulse Width Low
    ,{PAGE_3, 0xD9, 0x00} // HS Pulse Width High
    ,{PAGE_3, 0xDA, 0x98} // H Total Pixels Low
    ,{PAGE_3, 0xDB, 0x08} // H Total Pixels High
    ,{PAGE_3, 0xDC, 0x80} // H Active Pixels Low
    ,{PAGE_3, 0xDD, 0x07} // H Active Pixels High
    ,{PAGE_3, 0xDE, 0x1C} // V Active Lines Low
    ,{PAGE_3, 0xDF, 0x02} // V Active Lines High
    ,{PAGE_3, 0xE0, 0x02} // VS Front Porch Low
    ,{PAGE_3, 0xE1, 0x00} // VS Front Porch High
    ,{PAGE_3, 0xE2, 0x05} // VS Pulse Width Low
    ,{PAGE_3, 0xE3, 0x00} // VS Pulse Width High
    ,{PAGE_3, 0xE4, 0x32} // V Total Lines Low
    ,{PAGE_3, 0xE5, 0x02} // V Total Lines High
    ,{0xFF, 0xFF, 0x00} // end of row
};

video_mode_reg regList640x480[20]=
{
     {PAGE_0, 0x00, 0xC4} //
    ,{PAGE_0, 0x01, 0x09} //
    ,{PAGE_3, 0x8F, 0xAD} // HS_POL, VS_POL active high
    ,{PAGE_3, 0xD6, 0x10} // HS Front Porch Low
    ,{PAGE_3, 0xD7, 0x00} // HS Front Porch High
    ,{PAGE_3, 0xD8, 0x60} // HS Pulse Width Low
    ,{PAGE_3, 0xD9, 0x00} // HS Pulse Width High
    ,{PAGE_3, 0xDA, 0x20} // H Total Pixels Low
    ,{PAGE_3, 0xDB, 0x03} // H Total Pixels High
    ,{PAGE_3, 0xDC, 0x80} // H Active Pixels Low
    ,{PAGE_3, 0xDD, 0x02} // H Active Pixels High
    ,{PAGE_3, 0xDE, 0xE0} // V Active Lines Low
    ,{PAGE_3, 0xDF, 0x01} // V Active Lines High
    ,{PAGE_3, 0xE0, 0x0A} // VS Front Porch Low
    ,{PAGE_3, 0xE1, 0x00} // VS Front Porch High
    ,{PAGE_3, 0xE2, 0x02} // VS Pulse Width Low
    ,{PAGE_3, 0xE3, 0x00} // VS Pulse Width High
    ,{PAGE_3, 0xE4, 0x0D} // V Total Lines Low
    ,{PAGE_3, 0xE5, 0x02} // V Total Lines High
    ,{0xFF, 0xFF, 0x00} // end of row
};

video_mode_reg regList720x480[20]=
{
     {PAGE_0, 0x00, 0x8C} //
    ,{PAGE_0, 0x01, 0x0A} //
    ,{PAGE_3, 0x8F, 0xAD} // HS_POL, VS_POL active high
    ,{PAGE_3, 0xD6, 0x10} // HS Front Porch Low
    ,{PAGE_3, 0xD7, 0x00} // HS Front Porch High
    ,{PAGE_3, 0xD8, 0x3E} // HS Pulse Width Low
    ,{PAGE_3, 0xD9, 0x00} // HS Pulse Width High
    ,{PAGE_3, 0xDA, 0x5A} // H Total Pixels Low
    ,{PAGE_3, 0xDB, 0x03} // H Total Pixels High
    ,{PAGE_3, 0xDC, 0xD0} // H Active Pixels Low
    ,{PAGE_3, 0xDD, 0x02} // H Active Pixels High
    ,{PAGE_3, 0xDE, 0xE0} // V Active Lines Low
    ,{PAGE_3, 0xDF, 0x01} // V Active Lines High
    ,{PAGE_3, 0xE0, 0x09} // VS Front Porch Low
    ,{PAGE_3, 0xE1, 0x00} // VS Front Porch High
    ,{PAGE_3, 0xE2, 0x06} // VS Pulse Width Low
    ,{PAGE_3, 0xE3, 0x00} // VS Pulse Width High
    ,{PAGE_3, 0xE4, 0x0D} // V Total Lines Low
    ,{PAGE_3, 0xE5, 0x02} // V Total Lines High
    ,{0xFF, 0xFF, 0x00} // end of row
};


video_mode_info VideoModeInfo[8]=
{
     {{1920,1080,0x28,  32,   0, tsfDDROver2_2Lanes|tsfDDROver2_3Lanes } ,
     {{PAGE_0, 0x00, 0x00} //
      ,{PAGE_0, 0x01, 0x1D} //
      ,{PAGE_3, 0x8F, 0xA1} // HS_POL, VS_POL active high
      ,{PAGE_3, 0xD6, 0x7E} // HS Front Porch Low
      ,{PAGE_3, 0xD7, 0x02} // HS Front Porch High
      ,{PAGE_3, 0xD8, 0x2C} // HS Pulse Width Low
      ,{PAGE_3, 0xD9, 0x00} // HS Pulse Width High
      ,{PAGE_3, 0xDA, 0xBE} // H Total Pixels Low
      ,{PAGE_3, 0xDB, 0x0A} // H Total Pixels High
      ,{PAGE_3, 0xDC, 0x80} // H Active Pixels Low
      ,{PAGE_3, 0xDD, 0x07} // H Active Pixels High
      ,{PAGE_3, 0xDE, 0x38} // V Active Lines Low
      ,{PAGE_3, 0xDF, 0x04} // V Active Lines High
      ,{PAGE_3, 0xE0, 0x04} // VS Front Porch Low
      ,{PAGE_3, 0xE1, 0x00} // VS Front Porch High
      ,{PAGE_3, 0xE2, 0x05} // VS Pulse Width Low
      ,{PAGE_3, 0xE3, 0x00} // VS Pulse Width High
      ,{PAGE_3, 0xE4, 0x65} // V Total Lines Low
      ,{PAGE_3, 0xE5, 0x04} // V Total Lines High
      ,{0xFF, 0xFF, 0x00} // end of row
    }}

    ,{{1024, 768,0x18,0x00,0x00, 0},
      {{PAGE_0, 0x00, 0x00} //
      ,{PAGE_0, 0x01, 0x1D} //
      ,{PAGE_3, 0x8F, 0xA1} // HS_POL, VS_POL active high
      ,{PAGE_3, 0xD6, 0xA0} // HS Front Porch Low
      ,{PAGE_3, 0xD7, 0x00} // HS Front Porch High
      ,{PAGE_3, 0xD8, 0x88} // HS Pulse Width Low
      ,{PAGE_3, 0xD9, 0x00} // HS Pulse Width High
      ,{PAGE_3, 0xDA, 0x80} // H Total Pixels Low
      ,{PAGE_3, 0xDB, 0x05} // H Total Pixels High
      ,{PAGE_3, 0xDC, 0x00} // H Active Pixels Low
      ,{PAGE_3, 0xDD, 0x04} // H Active Pixels High
      ,{PAGE_3, 0xDE, 0x00} // V Active Lines Low
      ,{PAGE_3, 0xDF, 0x03} // V Active Lines High
      ,{PAGE_3, 0xE0, 0x03} // VS Front Porch Low
      ,{PAGE_3, 0xE1, 0x00} // VS Front Porch High
      ,{PAGE_3, 0xE2, 0x06} // VS Pulse Width Low
      ,{PAGE_3, 0xE3, 0x00} // VS Pulse Width High
      ,{PAGE_3, 0xE4, 0x26} // V Total Lines Low
      ,{PAGE_3, 0xE5, 0x03} // V Total Lines High
      ,{0xFF, 0xFF, 0x00} // end of row
    }}


    ,{{1280, 720,0x28,   4,0x00, tsfDDROver2_2Lanes|tsfDDROver2_3Lanes  },
      {{PAGE_0, 0x00, 0x00} //
      ,{PAGE_0, 0x01, 0x1D} //
      ,{PAGE_3, 0x8F, 0xA1} // HS_POL, VS_POL active high
      ,{PAGE_3, 0xD6, 0x6E} // HS Front Porch Low
      ,{PAGE_3, 0xD7, 0x00} // HS Front Porch High
      ,{PAGE_3, 0xD8, 0x28} // HS Pulse Width Low
      ,{PAGE_3, 0xD9, 0x00} // HS Pulse Width High
      ,{PAGE_3, 0xDA, 0x72} // H Total Pixels Low
      ,{PAGE_3, 0xDB, 0x06} // H Total Pixels High
      ,{PAGE_3, 0xDC, 0x00} // H Active Pixels Low
      ,{PAGE_3, 0xDD, 0x05} // H Active Pixels High
      ,{PAGE_3, 0xDE, 0xD0} // V Active Lines Low
      ,{PAGE_3, 0xDF, 0x02} // V Active Lines High
      ,{PAGE_3, 0xE0, 0x05} // VS Front Porch Low
      ,{PAGE_3, 0xE1, 0x00} // VS Front Porch High
      ,{PAGE_3, 0xE2, 0x05} // VS Pulse Width Low
      ,{PAGE_3, 0xE3, 0x00} // VS Pulse Width High
      ,{PAGE_3, 0xE4, 0xEE} // V Total Lines Low
      ,{PAGE_3, 0xE5, 0x02} // V Total Lines High
      ,{0xFF, 0xFF, 0x00} // end of row
    }}

    ,{{ 800, 600,0x18,0x00,0x00, 0},
     {{PAGE_0, 0x00, 0x00} //
      ,{PAGE_0, 0x01, 0x1D} //
      ,{PAGE_3, 0x8F, 0xA1} // HS_POL, VS_POL active high
      ,{PAGE_3, 0xD6, 0x28} // HS Front Porch Low
      ,{PAGE_3, 0xD7, 0x00} // HS Front Porch High
      ,{PAGE_3, 0xD8, 0x80} // HS Pulse Width Low
      ,{PAGE_3, 0xD9, 0x00} // HS Pulse Width High
      ,{PAGE_3, 0xDA, 0x20} // H Total Pixels Low
      ,{PAGE_3, 0xDB, 0x04} // H Total Pixels High
      ,{PAGE_3, 0xDC, 0x20} // H Active Pixels Low
      ,{PAGE_3, 0xDD, 0x03} // H Active Pixels High
      ,{PAGE_3, 0xDE, 0x58} // V Active Lines Low
      ,{PAGE_3, 0xDF, 0x02} // V Active Lines High
      ,{PAGE_3, 0xE0, 0x01} // VS Front Porch Low
      ,{PAGE_3, 0xE1, 0x00} // VS Front Porch High
      ,{PAGE_3, 0xE2, 0x04} // VS Pulse Width Low
      ,{PAGE_3, 0xE3, 0x00} // VS Pulse Width High
      ,{PAGE_3, 0xE4, 0x74} // V Total Lines Low
      ,{PAGE_3, 0xE5, 0x02} // V Total Lines High
      ,{0xFF, 0xFF, 0x00} // end of row
    }}

    ,{{ 720, 576,0x18,  17,0x00, tsfDDROver2_1Lane},
      {{PAGE_0, 0x00, 0x8C} //
      ,{PAGE_0, 0x01, 0x0A} //
      ,{PAGE_3, 0x8F, 0xAD} // HS_POL, VS_POL active high
      ,{PAGE_3, 0xD6, 0x0C} // HS Front Porch Low
      ,{PAGE_3, 0xD7, 0x00} // HS Front Porch High
      ,{PAGE_3, 0xD8, 0x40} // HS Pulse Width Low
      ,{PAGE_3, 0xD9, 0x00} // HS Pulse Width High
      ,{PAGE_3, 0xDA, 0x60} // H Total Pixels Low
      ,{PAGE_3, 0xDB, 0x03} // H Total Pixels High
      ,{PAGE_3, 0xDC, 0xD0} // H Active Pixels Low
      ,{PAGE_3, 0xDD, 0x02} // H Active Pixels High
      ,{PAGE_3, 0xDE, 0x40} // V Active Lines Low
      ,{PAGE_3, 0xDF, 0x02} // V Active Lines High
      ,{PAGE_3, 0xE0, 0x05} // VS Front Porch Low
      ,{PAGE_3, 0xE1, 0x00} // VS Front Porch High
      ,{PAGE_3, 0xE2, 0x05} // VS Pulse Width Low
      ,{PAGE_3, 0xE3, 0x00} // VS Pulse Width High
      ,{PAGE_3, 0xE4, 0x71} // V Total Lines Low
      ,{PAGE_3, 0xE5, 0x02} // V Total Lines High
      ,{0xFF, 0xFF, 0x00} // end of row
    }}


    ,{{1920, 540,0x28,   5,0x40, tsfDDROver2_2Lanes|tsfDDROver2_3Lanes  },
      {{PAGE_0, 0x00, 0x00} //
      ,{PAGE_0, 0x01, 0x1D} //
      ,{PAGE_3, 0x8F, 0xA1} // HS_POL, VS_POL active high
      ,{PAGE_3, 0xD6, 0x58} // HS Front Porch Low
      ,{PAGE_3, 0xD7, 0x00} // HS Front Porch High
      ,{PAGE_3, 0xD8, 0x2C} // HS Pulse Width Low
      ,{PAGE_3, 0xD9, 0x00} // HS Pulse Width High
      ,{PAGE_3, 0xDA, 0x98} // H Total Pixels Low
      ,{PAGE_3, 0xDB, 0x08} // H Total Pixels High
      ,{PAGE_3, 0xDC, 0x80} // H Active Pixels Low
      ,{PAGE_3, 0xDD, 0x07} // H Active Pixels High
      ,{PAGE_3, 0xDE, 0x1C} // V Active Lines Low
      ,{PAGE_3, 0xDF, 0x02} // V Active Lines High
      ,{PAGE_3, 0xE0, 0x02} // VS Front Porch Low
      ,{PAGE_3, 0xE1, 0x00} // VS Front Porch High
      ,{PAGE_3, 0xE2, 0x05} // VS Pulse Width Low
      ,{PAGE_3, 0xE3, 0x00} // VS Pulse Width High
      ,{PAGE_3, 0xE4, 0x32} // V Total Lines Low
      ,{PAGE_3, 0xE5, 0x02} // V Total Lines High
      ,{0xFF, 0xFF, 0x00} // end of row
    }}

    ,{{ 640, 480,0x18,   1,0x00, tsfDDROver2_1Lane},
      {{PAGE_0, 0x00, 0xC4} //
      ,{PAGE_0, 0x01, 0x09} //
      ,{PAGE_3, 0x8F, 0xAD} // HS_POL, VS_POL active high
      ,{PAGE_3, 0xD6, 0x10} // HS Front Porch Low
      ,{PAGE_3, 0xD7, 0x00} // HS Front Porch High
      ,{PAGE_3, 0xD8, 0x60} // HS Pulse Width Low
      ,{PAGE_3, 0xD9, 0x00} // HS Pulse Width High
      ,{PAGE_3, 0xDA, 0x20} // H Total Pixels Low
      ,{PAGE_3, 0xDB, 0x03} // H Total Pixels High
      ,{PAGE_3, 0xDC, 0x80} // H Active Pixels Low
      ,{PAGE_3, 0xDD, 0x02} // H Active Pixels High
      ,{PAGE_3, 0xDE, 0xE0} // V Active Lines Low
      ,{PAGE_3, 0xDF, 0x01} // V Active Lines High
      ,{PAGE_3, 0xE0, 0x0A} // VS Front Porch Low
      ,{PAGE_3, 0xE1, 0x00} // VS Front Porch High
      ,{PAGE_3, 0xE2, 0x02} // VS Pulse Width Low
      ,{PAGE_3, 0xE3, 0x00} // VS Pulse Width High
      ,{PAGE_3, 0xE4, 0x0D} // V Total Lines Low
      ,{PAGE_3, 0xE5, 0x02} // V Total Lines High
      ,{0xFF, 0xFF, 0x00} // end of row
    }}

    ,{{ 720, 480,0x18,   2,0x00, tsfDDROver2_1Lane},
      {{PAGE_0, 0x00, 0x8C} //
      ,{PAGE_0, 0x01, 0x0A} //
      ,{PAGE_3, 0x8F, 0xAD} // HS_POL, VS_POL active high
      ,{PAGE_3, 0xD6, 0x10} // HS Front Porch Low
      ,{PAGE_3, 0xD7, 0x00} // HS Front Porch High
      ,{PAGE_3, 0xD8, 0x3E} // HS Pulse Width Low
      ,{PAGE_3, 0xD9, 0x00} // HS Pulse Width High
      ,{PAGE_3, 0xDA, 0x5A} // H Total Pixels Low
      ,{PAGE_3, 0xDB, 0x03} // H Total Pixels High
      ,{PAGE_3, 0xDC, 0xD0} // H Active Pixels Low
      ,{PAGE_3, 0xDD, 0x02} // H Active Pixels High
      ,{PAGE_3, 0xDE, 0xE0} // V Active Lines Low
      ,{PAGE_3, 0xDF, 0x01} // V Active Lines High
      ,{PAGE_3, 0xE0, 0x09} // VS Front Porch Low
      ,{PAGE_3, 0xE1, 0x00} // VS Front Porch High
      ,{PAGE_3, 0xE2, 0x06} // VS Pulse Width Low
      ,{PAGE_3, 0xE3, 0x00} // VS Pulse Width High
      ,{PAGE_3, 0xE4, 0x0D} // V Total Lines Low
      ,{PAGE_3, 0xE5, 0x02} // V Total Lines High
      ,{0xFF, 0xFF, 0x00} // end of row
    }}
};

#define NUM_OF_VIDEO_MODE (sizeof(VideoModeInfo)/sizeof(VideoModeInfo[0]))

static struct i2c_device_id SIMG72_id[] = {
	{"SIMG72", 0},
	{}
};

static struct i2c_device_id SIMG7A_id[] = {
  {"SIMG7A", 0},
  {}
};

static struct i2c_device_id SIMG92_id[] = {
  {"SIMG92", 0},
  {}
};

static struct i2c_device_id SIMG9A_id[] = {
  {"SIMG9A", 0},
  {}
};

static struct i2c_device_id SIMGC8_id[] = {
  {"SIMGC8", 0},
  {}
};

static struct i2c_device_id SIMGA0_id[] = {
  {"SIMGA0", 0},
  {}
};

static struct i2c_device_id SIMG60_id[] = {
  {"SIMG60", 0},
  {}
};

static struct i2c_driver simg72_driver =
{
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "SIMG72",
	},
	.id_table	= SIMG72_id,
	.probe	= simg72_probe,
	.remove	= __devexit_p(simg72_remove),
	.command = NULL,
};

static struct i2c_driver simg7A_driver =
{
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "SIMG7A",
	},
	.id_table	= SIMG7A_id,
	.probe	= simg7A_probe,
	.remove	= __devexit_p(simg7A_remove),
	.command = NULL,
};

static struct i2c_driver simg92_driver =
{
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "SIMG92",
	},
	.id_table	= SIMG92_id,
	.probe	= simg92_probe,
	.remove	= __devexit_p(simg92_remove),
	.command = NULL,
};

static struct i2c_driver simg9A_driver =
{
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "SIMG9A",
	},
	.id_table	= SIMG9A_id,
	.probe	= simg9A_probe,
	.remove	= __devexit_p(simg9A_remove),
	.command = NULL,
};

static struct i2c_driver simgC8_driver =
{
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "SIMGC8",
	},
	.id_table	= SIMGC8_id,
	.probe	= simgC8_probe,
	.remove	= __devexit_p(simgC8_remove),
	.command = NULL,
};

static struct i2c_driver simgA0_driver =
{
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "SIMGA0",
	},
	.id_table	= SIMGA0_id,
	.probe	= simgA0_probe,
	.remove	= __devexit_p(simgA0_remove),
	.command = NULL,
};

static struct i2c_driver simg60_driver =
{
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "SIMG60",
	},
	.id_table	= SIMG60_id,
	.probe	= simg60_probe,
	.remove	= __devexit_p(simg60_remove),
	.command = NULL,
};

static void hw_resst(struct mhl_platform_data *pdata)
{

	DEBUG_MHL_FUNC_START();

	gpio_direction_output(pdata->mhl_rst, false);
	msleep(100);
  gpio_direction_output(pdata->mhl_rst, true);
}

struct i2c_client* get_simgI2C_client(struct mhl_platform_data *pdata, u8 device_id)
{

	struct i2c_client* client_ptr;

	if(device_id == 0x72)
		client_ptr = pdata->simg72_tx_client;
	else if(device_id == 0x7A)
		client_ptr = pdata->simg7A_tx_client;
	else if(device_id == 0x92)
		client_ptr = pdata->simg92_tx_client;
	else if(device_id == 0x9A)
		client_ptr = pdata->simg9A_tx_client;
	else if(device_id == 0xC8)
		client_ptr = pdata->simgC8_tx_client;
	else if(device_id == 0x60)
		client_ptr = pdata->simg60_tx_client;
	else if(device_id == 0xA0)
		client_ptr = pdata->simgA0_tx_client;
	else
		client_ptr = NULL;

	return client_ptr;
}


static int I2C_ReadByte(struct mhl_platform_data *pdata, u8 deviceID, u8 offset, u8 *data)
{
	int ret=0;

	struct i2c_client* client_ptr = get_simgI2C_client(pdata, deviceID);

	if(!data){
		printk(KERN_INFO"[MHL]I2C_ReadByte error1 %x\n",deviceID);
		return -EINVAL;
	}

	if(!client_ptr)
	{
		printk(KERN_INFO"[MHL]I2C_ReadByte error2 %x\n",deviceID);
		return -EINVAL;
	}


	ret = i2c_smbus_read_byte_data(client_ptr,offset);

	if(ret<0){
		printk(KERN_INFO"[MHL]I2C_ReadByte error3 %x\n",deviceID);
		return ret;
	}

	*data = (ret &0x000000FF);

	return ret;

}

static int I2C_ReadByte_block(struct mhl_platform_data *pdata, u8 deviceID, u8 offset, u8 length, u8 *data)
{
  int ret=0;

	struct i2c_client* client_ptr = get_simgI2C_client(pdata, deviceID);

  if(!client_ptr){
		printk(KERN_INFO"[MHL]I2C_ReadByte error2 %x\n",deviceID);
		return -EINVAL;
	}

  ret = i2c_smbus_read_i2c_block_data(client_ptr,offset,length,data);
  return ret;
}

static int I2C_WriteByte_block(struct mhl_platform_data *pdata, u8 deviceID, u8 offset, u8 length, u8 *data)
{
  int ret=0;

	struct i2c_client* client_ptr = get_simgI2C_client(pdata, deviceID);

  if(!client_ptr){
		printk(KERN_INFO"[MHL]I2C_ReadByte error2 %x\n",deviceID);
		return -EINVAL;
	}

  ret = i2c_smbus_write_i2c_block_data(client_ptr,offset,length,data);
  return ret;
}


static int I2C_WriteByte(struct mhl_platform_data *pdata, u8 deviceID, u8 offset, u8 value)
{
	int ret;

	struct i2c_client* client_ptr = get_simgI2C_client(pdata, deviceID);

	if(client_ptr == NULL){
		printk("I2C_WriteByte (client_ptr == NULL) \n");
		return 0;
	}

	ret = i2c_smbus_write_byte_data(client_ptr,offset,value);
	return ret;

}

static int I2CReadModify(struct mhl_platform_data *pdata, u8 deviceID, u8 Offset, u8 Mask, u8 Data)
{
	u8 rd;
	int ret;

	ret = I2C_ReadByte(pdata,deviceID,Offset,&rd);

	if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		return ret;
	}

	rd &= ~Mask;
	rd |= (Data & Mask);

	ret= I2C_WriteByte(pdata,deviceID,Offset,rd);

	if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		return ret;
	}

	return ret;
}

#define	SET_BIT(pdata, deviceID, offset, bitnumber)		I2CReadModify(pdata, deviceID, offset,(1<<bitnumber),(1<<bitnumber))
#define	CLR_BIT(pdata, deviceID, offset, bitnumber)		I2CReadModify(pdata, deviceID, offset,(1<<bitnumber),0x00)


static int init_cbus_regs(struct mhl_tx *mhl)
{
  int ret = 0;
  u8 tmp = 0;

	DEBUG_MHL_FUNC_START();
  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_CBUS, 0x07, 0xF2);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_CBUS, 0x36, 0x0B);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_CBUS, 0x39, 0x30);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_CBUS, 0x40, 0x03);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_CBUS, 0x80, DEVCAP_VAL_DEV_STATE);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_CBUS, 0x81, DEVCAP_VAL_MHL_VERSION);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_CBUS, 0x82, DEVCAP_VAL_DEV_CAT);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_CBUS, 0x83, DEVCAP_VAL_ADOPTER_ID_H);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_CBUS, 0x84, DEVCAP_VAL_ADOPTER_ID_L);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_CBUS, 0x85, DEVCAP_VAL_VID_LINK_MODE);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_CBUS, 0x86, DEVCAP_VAL_AUD_LINK_MODE);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_CBUS, 0x87, DEVCAP_VAL_VIDEO_TYPE);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_CBUS, 0x88, DEVCAP_VAL_LOG_DEV_MAP);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_CBUS, 0x89, DEVCAP_VAL_BANDWIDTH);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_CBUS, 0x8A, DEVCAP_VAL_FEATURE_FLAG);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_CBUS, 0x8B, DEVCAP_VAL_DEVICE_ID_H);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_CBUS, 0x8C, DEVCAP_VAL_DEVICE_ID_L);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_CBUS, 0x8D, DEVCAP_VAL_SCRATCHPAD_SIZE);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_CBUS, 0x8E, DEVCAP_VAL_INT_STAT_SIZE);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_CBUS, 0x8F, DEVCAP_VAL_RESERVED);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_ReadByte(mhl->pdata,PAGE_CBUS, 0x31, &tmp);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  tmp = (tmp|0x0C);
  ret = I2C_WriteByte(mhl->pdata,PAGE_CBUS, 0x31, tmp);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_CBUS, 0x22, 0x0F);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_CBUS, 0x30, 0x01);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2CReadModify(mhl->pdata,PAGE_CBUS,0x2E,BIT4, BIT4);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  exit_func:
  if(ret<0){
    mutex_unlock(&mhl->i2c_lock);
  }

  return ret;
}

static int cbus_reset(struct mhl_tx *mhl)
{
  int ret;

	DEBUG_MHL_FUNC_START();

  mutex_lock(&mhl->i2c_lock);
  ret = SET_BIT(mhl->pdata, PAGE_3, 0x00, 3);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  msleep(2);

  mutex_lock(&mhl->i2c_lock);
  ret = CLR_BIT(mhl->pdata, PAGE_3, 0x00, 3);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);


// interrupt #4
  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_3, 0x22, (BIT_INTR4_RGND_RDY ));
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_CBUS, 0xE0, 0xFF);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

    mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_CBUS, 0xE1, 0xFF);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_CBUS, 0xE2, 0xFF);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

   mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_CBUS, 0xE3, 0xFF);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_CBUS, 0xF0, 0xFF);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_CBUS, 0xF1, 0xFF);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_CBUS, 0xF2, 0xFF);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_CBUS, 0xF3, 0xFF);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
//set TPI mode...
  ret = I2C_WriteByte(mhl->pdata,PAGE_0,0xC7, 0x00);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  msleep(100);

    mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_0,0x3C, ( BIT_TPI_INTR_ST0_HDCP_AUTH_STATUS_CHANGE_EVENT
                                        | BIT_TPI_INTR_ST0_HDCP_VPRIME_VALUE_READY_EVENT
                                        | BIT_TPI_INTR_ST0_HDCP_SECURITY_CHANGE_EVENT
                                        | BIT_TPI_INTR_ST0_HOT_PLUG_EVENT
                                        ));
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  exit_func:
  if(ret<0){
    mutex_unlock(&mhl->i2c_lock);
  }

  return ret;
}


static int TX_GO2D3(struct mhl_tx *mhl)
{
	int ret=0;

	DEBUG_MHL_FUNC_START();

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_3,0x30, 0xD0);
	if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
    goto exit_func;
	}
  mutex_unlock(&mhl->i2c_lock);

	msleep(50);

	  mutex_lock(&mhl->i2c_lock);
  ret = CLR_BIT(mhl->pdata,PAGE_1,0x3D,0x00);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
    goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  exit_func:
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
    mutex_unlock(&mhl->i2c_lock);
  }

	return ret;

}

static int forceUSBIDswitchOpen(struct mhl_tx *mhl)
{
  int ret = 0;

  mutex_lock(&mhl->i2c_lock);
  ret = I2CReadModify(mhl->pdata, PAGE_3, 0x10, BIT0, 0x00);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
    goto exit_func;

  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2CReadModify(mhl->pdata, PAGE_3, 0x15, BIT6, BIT6);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
    goto exit_func;

  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_3,0x12, 0x86);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
    goto exit_func;

  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2CReadModify(mhl->pdata, PAGE_3, 0x20, BIT5 | BIT4, BIT4);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
    goto exit_func;

  }
  mutex_unlock(&mhl->i2c_lock);

  exit_func:
  if(ret<0){
    mutex_unlock(&mhl->i2c_lock);
  }

  return ret;
}

static int releaseUSBIDswitchOpen(struct mhl_tx *mhl)
{
  int ret =0;

  msleep(50);

  mutex_lock(&mhl->i2c_lock);
  ret = I2CReadModify(mhl->pdata, PAGE_3, 0x15, BIT6, 0x00);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
    goto exit_func;

  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2CReadModify(mhl->pdata, PAGE_3, 0x10, BIT0, BIT0);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
    goto exit_func;

  }
  mutex_unlock(&mhl->i2c_lock);

  exit_func:
  if(ret<0){
    mutex_unlock(&mhl->i2c_lock);
  }

  return ret;
}

static int mhl_init_func(struct mhl_tx *mhl)
{
  int ret;

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_3,0x8E, 0x03);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_1,0x3D, 0x3F);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_3,0x33, 0xC8);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_3,0x35, 0xBC);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_3,0x36, 0x03);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_3,0x37, 0x0A);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_0,0x7A, 0x00);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_0,0x0D, 0x02);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_0,0x80, 0x08);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_0,0xF8, 0x4C);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2CReadModify(mhl->pdata,PAGE_2,0x05,BIT5, 0x00);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_3,0x10, 0x27);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_3,0x11, 0xAD);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_3,0x12, 0x86);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_3,0x13, 0x8C);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_3,0x14, 0x57);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_3,0x15, 0x11);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_3,0x16, 0x20);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_3,0x17, 0x82);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_3,0x18, 0x25);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_3,0xCA, 0x10);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
//////////////////////////////////////////////////////
//for mipi 3 lanes
  ret = I2C_WriteByte(mhl->pdata,PAGE_3,0xA5, 0x1c);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_3,0xA6, 0x26);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_3,0xA7, 0x32);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_3,0x8E, 0x03);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
//////////////////////////////////////////////////////
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2CReadModify(mhl->pdata,PAGE_0,0x85,BIT0, 0x00);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_3,0x00, 0x84);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  ret = cbus_reset(mhl);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		return ret;
  }

  msleep(100);

  ret = init_cbus_regs(mhl);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		return ret;
  }

  exit_func:
  if(ret<0){
    mutex_unlock(&mhl->i2c_lock);
  }

  return ret;
}


static int switchToD0(struct mhl_tx *mhl)
{
  int ret =0;

  ret = mhl_init_func(mhl);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
    return ret;
  }

  mutex_lock(&mhl->i2c_lock);
  ret = I2CReadModify(mhl->pdata, PAGE_3, 0x10, BIT1, 0x00);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
    goto exit_func;

  }
  mutex_unlock(&mhl->i2c_lock);


  mutex_lock(&mhl->i2c_lock);
  ret = I2CReadModify(mhl->pdata, PAGE_0, 0x1E, (BIT1 | BIT0), 0x00);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
    goto exit_func;

  }
  mutex_unlock(&mhl->i2c_lock);

  exit_func:
  if(ret<0){
    mutex_unlock(&mhl->i2c_lock);
  }

  return ret;

}


static void SiiMhlTxRapkSend(struct mhl_tx *mhl)
{
  mutex_lock(&mhl->cbus_cmd_lock);
  mhl->msc_cmd_q[mhl->cmd_rx_cnt].command = MHL_MSC_MSG;
  mhl->msc_cmd_q[mhl->cmd_rx_cnt].offset= 0x00;
  mhl->msc_cmd_q[mhl->cmd_rx_cnt].lenght= 0x02;
  mhl->msc_cmd_q[mhl->cmd_rx_cnt].buff[0]= MHL_MSC_MSG_RAPK;
  mhl->msc_cmd_q[mhl->cmd_rx_cnt].buff[1]= 0x00;
  mhl->cmd_rx_cnt += 1;

  if(mhl->cmd_rx_cnt == 1){
    queue_work(mhl->cbus_cmd_wqs, &mhl->cbus_cmd_work);
  }
  mutex_unlock(&mhl->cbus_cmd_lock);
}

static void SiiMhlTxRcpkSend(struct mhl_tx *mhl,u8 keycode)
{
  mutex_lock(&mhl->cbus_cmd_lock);
  mhl->msc_cmd_q[mhl->cmd_rx_cnt].command = MHL_MSC_MSG;
  mhl->msc_cmd_q[mhl->cmd_rx_cnt].offset= 0x00;
  mhl->msc_cmd_q[mhl->cmd_rx_cnt].lenght= 0x02;
  mhl->msc_cmd_q[mhl->cmd_rx_cnt].buff[0]= MHL_MSC_MSG_RCPK;
  mhl->msc_cmd_q[mhl->cmd_rx_cnt].buff[1]= keycode;
  mhl->cmd_rx_cnt += 1;

  if(mhl->cmd_rx_cnt == 1){
    queue_work(mhl->cbus_cmd_wqs, &mhl->cbus_cmd_work);
  }
  mutex_unlock(&mhl->cbus_cmd_lock);
}

static void SiiMhlTxRcpeSend(struct mhl_tx *mhl, u8 erro_code, u8 key_code)
{
  mutex_lock(&mhl->cbus_cmd_lock);
  mhl->msc_cmd_q[mhl->cmd_rx_cnt].command = MHL_MSC_MSG;
  mhl->msc_cmd_q[mhl->cmd_rx_cnt].offset= 0x00;
  mhl->msc_cmd_q[mhl->cmd_rx_cnt].lenght= 0x03;
  mhl->msc_cmd_q[mhl->cmd_rx_cnt].buff[0]= MHL_MSC_MSG_RCPE;
  mhl->msc_cmd_q[mhl->cmd_rx_cnt].buff[1]= erro_code;
  mhl->msc_cmd_q[mhl->cmd_rx_cnt].buff[2]= key_code;
  mhl->cmd_rx_cnt += 1;

  if(mhl->cmd_rx_cnt == 1){
    queue_work(mhl->cbus_cmd_wqs, &mhl->cbus_cmd_work);
  }
  mutex_unlock(&mhl->cbus_cmd_lock);
}

static int cbus_cmd_send(struct mhl_tx *mhl, u8 id)
{
  int ret = 0;
  u8 startbit = 0;

  ret = I2C_WriteByte(mhl->pdata,PAGE_CBUS,0x13, mhl->msc_cmd_q[id].offset);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }

  ret = I2C_WriteByte(mhl->pdata,PAGE_CBUS,0x14, mhl->msc_cmd_q[id].buff[0]);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  //printk("sii8332 CMD:%x, Offset:%x, D[0]:%x, D[1]:%x \n",mhl->msc_cmd_q[id].command,  mhl->msc_cmd_q[id].offset, mhl->msc_cmd_q[id].buff[0], mhl->msc_cmd_q[id].buff[1] );

  switch(mhl->msc_cmd_q[id].command)
  {
    case MHL_SET_INT:
      startbit = BIT_CBUS_MSC_WRITE_STAT_OR_SET_INT;
      break;

    case MHL_WRITE_STAT:
      startbit = BIT_CBUS_MSC_WRITE_STAT_OR_SET_INT;
      break;

    case MHL_READ_DEVCAP:
      startbit = BIT_CBUS_MSC_READ_DEVCAP;
      break;

    case MHL_GET_STATE:
    case MHL_GET_VENDOR_ID:
    case MHL_SET_HPD:
    case MHL_CLR_HPD:
    case MHL_GET_SC1_ERRORCODE:
    case MHL_GET_DDC_ERRORCODE:
    case MHL_GET_MSC_ERRORCODE:
    case MHL_GET_SC3_ERRORCODE:
      ret = I2C_WriteByte(mhl->pdata,PAGE_CBUS,0x13, mhl->msc_cmd_q[id].command);
      if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
      }
      startbit = 0x01;
      break;

    case MHL_MSC_MSG:
      ret = I2C_WriteByte(mhl->pdata,PAGE_CBUS,0x15,  mhl->msc_cmd_q[id].buff[1]);
      if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
      }

      ret = I2C_WriteByte(mhl->pdata,PAGE_CBUS,0x13,  mhl->msc_cmd_q[id].command);
      if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
      }
      startbit = BIT_CBUS_MSC_MSG;

      break;

    case MHL_WRITE_BURST:
      //will add code later...
      startbit = BIT_CBUS_MSC_WRITE_BURST;
      break;

    default:
      goto exit_func;
      break;
    }

    ret = I2C_WriteByte(mhl->pdata,PAGE_CBUS,0x12,  startbit);
    if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
    }

  exit_func:

    return ret;

}


static int cbus_cmd_done_reaction(struct mhl_tx *mhl, u8 tx_cnt, u8 i)
{
  int ret =0;
  u8 cap_data;

  switch( mhl->msc_cmd_q[(tx_cnt+i)].command)
  {
    case MHL_WRITE_STAT:
     break;

    case MHL_READ_DEVCAP:
      switch(mhl->msc_cmd_q[(tx_cnt+i)].offset)
      {
        case MHL_CAP_MHL_VERSION:

            ret = I2C_ReadByte(mhl->pdata, PAGE_CBUS, 0x16, &cap_data);
            if(ret<0){
              printk(KERN_INFO "[ERROR]sii8240: %s():%d failed !\n", __func__, __LINE__);
              goto exit_func;
            }
            mhl->pdata->rx_cap.mhl_ver = cap_data;

          break;

        case MHL_DEV_CATEGORY_OFFSET:

            ret = I2C_ReadByte(mhl->pdata, PAGE_CBUS, 0x16, &cap_data);
            if(ret<0){
              printk(KERN_INFO "[ERROR]sii8240: %s():%d failed !\n", __func__, __LINE__);
              goto exit_func;
            }

          cap_data = (0x1F & cap_data);

          switch(cap_data)
            {
              case MHL_DEV_CAT_SOURCE:
                 mhl->pdata->rx_cap.dev_type = MHL_DEV_CAT_SOURCE;
                break;

              case MHL_SINK_W_POW:
                 mhl->pdata->rx_cap.dev_type = MHL_SINK_W_POW;
                 //mhl tx doesn't need power out
                break;

              case MHL_SINK_WO_POW:
                 mhl->pdata->rx_cap.dev_type = MHL_SINK_WO_POW;
                break;

              case MHL_DONGLE_W_POW:
                 mhl->pdata->rx_cap.dev_type = MHL_DONGLE_W_POW;
                break;

              case MHL_DONGLE_WO_POW:
                 mhl->pdata->rx_cap.dev_type = MHL_DONGLE_WO_POW;
                break;

              default:
                 mhl->pdata->rx_cap.dev_type = cap_data;
                break;
            }
          break;

        case MHL_CAP_ADOPTER_ID_H:
          ret = I2C_ReadByte(mhl->pdata, PAGE_CBUS, 0x16, &cap_data);
          if(ret<0){
            printk(KERN_INFO "[ERROR]sii8240: %s():%d failed !\n", __func__, __LINE__);
            goto exit_func;
          }

          mhl->pdata->rx_cap.adopter_id = cap_data;
          mhl->pdata->rx_cap.adopter_id <<= 8;

          break;

        case MHL_CAP_ADOPTER_ID_L:
          ret = I2C_ReadByte(mhl->pdata, PAGE_CBUS, 0x16, &cap_data);
          if(ret<0){
            printk(KERN_INFO "[ERROR]sii8240: %s():%d failed !\n", __func__, __LINE__);
            goto exit_func;
          }

          mhl->pdata->rx_cap.adopter_id |= cap_data;

          break;

        case MHL_CAP_VID_LINK_MODE:
          ret = I2C_ReadByte(mhl->pdata, PAGE_CBUS, 0x16, &cap_data);
          if(ret<0){
            printk(KERN_INFO "[ERROR]sii8240: %s():%d failed !\n", __func__, __LINE__);
            goto exit_func;
          }

          mhl->pdata->rx_cap.vid_link_mode = (0x3F & cap_data);

          if(mhl->pdata->rx_cap.vid_link_mode & MHL_DEV_VID_LINK_SUPPRGB444){
              //printk(KERN_INFO "sii8240: %s():%d  MHL_DEV_VID_LINK_SUPPRGB444  \n", __func__, __LINE__);
          }

          if(mhl->pdata->rx_cap.vid_link_mode & MHL_DEV_VID_LINK_SUPPYCBCR444){
              //printk(KERN_INFO "sii8240: %s():%d  MHL_DEV_VID_LINK_SUPPYCBCR444  \n", __func__, __LINE__);
          }

          if(mhl->pdata->rx_cap.vid_link_mode & MHL_DEV_VID_LINK_SUPPYCBCR422){
              //printk(KERN_INFO "sii8240: %s():%d  MHL_DEV_VID_LINK_SUPPYCBCR422  \n", __func__, __LINE__);
          }

          if(mhl->pdata->rx_cap.vid_link_mode & MHL_DEV_VID_LINK_SUPP_PPIXEL){
              //printk(KERN_INFO "sii8240: %s():%d  MHL_DEV_VID_LINK_SUPP_PPIXEL  \n", __func__, __LINE__);
          }

          break;

        case MHL_CAP_AUD_LINK_MODE:
          ret = I2C_ReadByte(mhl->pdata, PAGE_CBUS, 0x16, &cap_data);
          if(ret<0){
            printk(KERN_INFO "[ERROR]sii8240: %s():%d failed !\n", __func__, __LINE__);
            goto exit_func;
          }

          mhl->pdata->rx_cap.aud_link_mode = (0x03 & cap_data);
          break;

        case MHL_CAP_VIDEO_TYPE:
          ret = I2C_ReadByte(mhl->pdata, PAGE_CBUS, 0x16, &cap_data);
          if(ret<0){
            printk(KERN_INFO "[ERROR]sii8240: %s():%d failed !\n", __func__, __LINE__);
            goto exit_func;
          }

          mhl->pdata->rx_cap.video_type = (0x8F & cap_data);
          break;

        case MHL_CAP_LOG_DEV_MAP:
          ret = I2C_ReadByte(mhl->pdata, PAGE_CBUS, 0x16, &cap_data);
          if(ret<0){
            printk(KERN_INFO "[ERROR]sii8240: %s():%d failed !\n", __func__, __LINE__);
            goto exit_func;
          }

          mhl->pdata->rx_cap.log_dev_map = cap_data;

          break;

        case MHL_CAP_BANDWIDTH:
          ret = I2C_ReadByte(mhl->pdata, PAGE_CBUS, 0x16, &cap_data);
          if(ret<0){
            printk(KERN_INFO "[ERROR]sii8240: %s():%d failed !\n", __func__, __LINE__);
            goto exit_func;
          }

          mhl->pdata->rx_cap.bandwidth = cap_data;
          break;

        case MHL_CAP_FEATURE_FLAG:
          ret = I2C_ReadByte(mhl->pdata, PAGE_CBUS, 0x16, &cap_data);
          if(ret<0){
            printk(KERN_INFO "[ERROR]sii8240: %s():%d failed !\n", __func__, __LINE__);
            goto exit_func;
          }

          mhl->pdata->rx_cap.feature_flag = (0x07 & cap_data);
          mhl->pdata->rx_cap.rcp_support = (mhl->pdata->rx_cap.feature_flag & MHL_FEATURE_RCP_SUPPORT)?true:false;
          mhl->pdata->rx_cap.rap_support = (mhl->pdata->rx_cap.feature_flag & MHL_FEATURE_RAP_SUPPORT)?true:false;
          mhl->pdata->rx_cap.sp_support = (mhl->pdata->rx_cap.feature_flag & MHL_FEATURE_SP_SUPPORT)?true:false;
          break;

        case MHL_CAP_DEVICE_ID_H:
          ret = I2C_ReadByte(mhl->pdata, PAGE_CBUS, 0x16, &cap_data);
          if(ret<0){
            printk(KERN_INFO "[ERROR]sii8240: %s():%d failed !\n", __func__, __LINE__);
            goto exit_func;
          }

          mhl->pdata->rx_cap.device_id = cap_data;
          mhl->pdata->rx_cap.device_id <<= 8;
          break;

        case MHL_CAP_DEVICE_ID_L:
          ret = I2C_ReadByte(mhl->pdata, PAGE_CBUS, 0x16, &cap_data);
          if(ret<0){
            printk(KERN_INFO "[ERROR]sii8240: %s():%d failed !\n", __func__, __LINE__);
            goto exit_func;
          }

          mhl->pdata->rx_cap.device_id |= cap_data;
          break;

        case MHL_CAP_SCRATCHPAD_SIZE:
          ret = I2C_ReadByte(mhl->pdata, PAGE_CBUS, 0x16, &cap_data);
          if(ret<0){
            printk(KERN_INFO "[ERROR]sii8240: %s():%d failed !\n", __func__, __LINE__);
            goto exit_func;
          }

          mhl->pdata->rx_cap.scratchpad_size = cap_data;
          break;

        case MHL_CAP_INT_STAT_SIZE:
          ret = I2C_ReadByte(mhl->pdata, PAGE_CBUS, 0x16, &cap_data);
          if(ret<0){
            printk(KERN_INFO "[ERROR]sii8240: %s():%d failed !\n", __func__, __LINE__);
            goto exit_func;
          }

          mhl->pdata->rx_cap.int_stat_size =cap_data;
          break;

        case MHL_CAP_DEV_STATE:
        case MHL_CAP_RESERVED:
        default:
          break;
      }

      break;

    case MHL_MSC_MSG:
      if(mhl->msc_cmd_q[(tx_cnt+i)].buff[0] == MHL_MSC_MSG_RCPE){
        mutex_lock(&mhl->cbus_cmd_lock);
        mhl->msc_cmd_q[mhl->cmd_rx_cnt].command = MHL_MSC_MSG;
        mhl->msc_cmd_q[mhl->cmd_rx_cnt].offset= 0x00;
        mhl->msc_cmd_q[mhl->cmd_rx_cnt].lenght= 0x02;
        mhl->msc_cmd_q[mhl->cmd_rx_cnt].buff[0]= MHL_MSC_MSG_RCPK;
        mhl->msc_cmd_q[mhl->cmd_rx_cnt].buff[1]= mhl->msc_cmd_q[(tx_cnt+i)].buff[2];
        mhl->cmd_rx_cnt += 1;
        mutex_unlock(&mhl->cbus_cmd_lock);
      }

      break;

    case MHL_WRITE_BURST:
      break;

    case MHL_SET_INT:
        if((mhl->msc_cmd_q[(tx_cnt+i)].offset ==MHL_RCHANGE_INT )&&
                (mhl->msc_cmd_q[(tx_cnt+i)].buff[0]==MHL_INT_DSCR_CHG )){
		  //printk(KERN_INFO "sii8240: %s():%d  !\n", __func__, __LINE__);
        }
        else if((mhl->msc_cmd_q[(tx_cnt+i)].offset ==MHL_RCHANGE_INT )&&
                (mhl->msc_cmd_q[(tx_cnt+i)].buff[0]==MHL_INT_DCAP_CHG )){
		  //printk(KERN_INFO "sii8240: %s():%d  MHL_STATUS_DCAP_RDY !\n", __func__, __LINE__);
          mhl->pdata->status.connected_ready|= MHL_STATUS_DCAP_RDY;
          mutex_lock(&mhl->cbus_cmd_lock);
          mhl->msc_cmd_q[mhl->cmd_rx_cnt].command = MHL_WRITE_STAT;
          mhl->msc_cmd_q[mhl->cmd_rx_cnt].offset= MHL_STATUS_REG_CONNECTED_RDY;
          mhl->msc_cmd_q[mhl->cmd_rx_cnt].lenght= 0x01;
          mhl->msc_cmd_q[mhl->cmd_rx_cnt].buff[0]= mhl->pdata->status.connected_ready;
          mhl->cmd_rx_cnt += 1;
          mutex_unlock(&mhl->cbus_cmd_lock);
        }
      break;

    default:
      break;
  }

  exit_func:

  return ret;

}


static void cbus_cmd_thread(struct work_struct *work)
{
  struct mhl_tx *mhl =
                          container_of(work,struct mhl_tx,cbus_cmd_work);
  u8 tx_cnt = 0;
  u8 rx_cnt = 0;
  u8 i = 0;
  int ret = 0;

  mutex_lock(&mhl->cbus_cmd_lock);
  rx_cnt = mhl->cmd_rx_cnt;
  mutex_unlock(&mhl->cbus_cmd_lock);
  tx_cnt = mhl->cmd_tx_cnt;

  if(tx_cnt == rx_cnt){
    mutex_lock(&mhl->cbus_cmd_lock);
    mhl->cmd_rx_cnt = 0;
    mhl->cmd_tx_cnt = 0;
    mutex_unlock(&mhl->cbus_cmd_lock);
    goto exit_func;
  }

  do{
     for(i = 0; i<(rx_cnt - tx_cnt ); i++)
     {
      mhl->msc_cmd_done_intr = MSC_SEND;
      ret = cbus_cmd_send(mhl,(tx_cnt+i));
      if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
      }

      wait_event(mhl->cbus_hanler_wq,
        ((mhl->msc_cmd_done_intr != MSC_SEND)||(mhl->msc_cmd_abord != false)
        ||(mhl->pdata->status.op_status!= MHL_DISCOVERY_SUCCESS)));

      if(mhl->pdata->status.op_status!= MHL_DISCOVERY_SUCCESS){
        mutex_lock(&mhl->cbus_cmd_lock);
        mhl->cmd_rx_cnt = 0;
        mhl->cmd_tx_cnt = 0;
        mutex_unlock(&mhl->cbus_cmd_lock);
        goto exit_func;
      }


      if(mhl->msc_cmd_done_intr == MSC_DONE_ACK){
         mhl->cmd_tx_cnt += 1;

        ret = cbus_cmd_done_reaction(mhl, tx_cnt, i);
        if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
        }

      }
      else if(mhl->msc_cmd_abord == true){
        mhl->msc_cmd_abord = false;
        printk("msleep 2S\n");
        msleep(2000);
      }
      else{
       mutex_lock(&mhl->cbus_cmd_lock);
       mhl->cmd_rx_cnt = 0;
       mhl->cmd_tx_cnt = 0;
       mutex_unlock(&mhl->cbus_cmd_lock);
       goto exit_func;
      }
    }

    mutex_lock(&mhl->cbus_cmd_lock);
    rx_cnt = mhl->cmd_rx_cnt;
    mutex_unlock(&mhl->cbus_cmd_lock);
    tx_cnt = mhl->cmd_tx_cnt;
    if(tx_cnt == rx_cnt){
      mutex_lock(&mhl->cbus_cmd_lock);
      mhl->cmd_rx_cnt = 0;
      mhl->cmd_tx_cnt = 0;
      mutex_unlock(&mhl->cbus_cmd_lock);
      goto exit_func;
    }

  }while(tx_cnt != rx_cnt);

  exit_func:
    if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
    }

}


static int mhl_rgnd_check_func(struct mhl_tx *mhl)
{
  u8 int4status, rgnd_value;
  int ret = 0;

  DEBUG_MHL_FUNC_START();

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_ReadByte(mhl->pdata, PAGE_3, 0x21, &int4status);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
    goto exit_func;

  }
  mutex_unlock(&mhl->i2c_lock);

  if(int4status == 0x00){
    return ret;
  }

   mutex_lock(&mhl->i2c_lock);
  //clear interrupt status...
  ret = I2C_WriteByte(mhl->pdata,PAGE_3, 0x21, int4status);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
    goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  if(int4status & BIT_INTR4_RGND_RDY){

    ret = switchToD0(mhl);
    if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
      goto exit_func;
    }

    mutex_lock(&mhl->i2c_lock);
    ret = I2C_ReadByte(mhl->pdata, PAGE_3, 0x1C, &rgnd_value);
    if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
      goto exit_func;

    }
    mutex_unlock(&mhl->i2c_lock);

    rgnd_value = (rgnd_value & 0x03);
    if(rgnd_value == 0x02){
		printk(KERN_INFO "sii8332: %s():%d MHL device \n", __func__, __LINE__);
      //mhl device...
      mutex_lock(&mhl->pdata->mhl_status_lock);
      mhl->pdata->status.op_status = MHL_RX_CONNECTED;
      mutex_unlock(&mhl->pdata->mhl_status_lock);

      mutex_lock(&mhl->i2c_lock);
      ret = I2C_WriteByte(mhl->pdata,PAGE_3, 0x22, ( BIT_INTR4_MHL_EST | BIT_INTR4_USB_EST | BIT_INTR4_CBUS_LKOUT  ));
      if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
	  goto	exit_func;
      }
      mutex_unlock(&mhl->i2c_lock);
    }
    else{
		printk(KERN_INFO "sii8332: %s():%d non MHL device: %02x \n", __func__, __LINE__, rgnd_value);

      mutex_lock(&mhl->pdata->mhl_status_lock);
      mhl->pdata->status.op_status = MHL_USB_CONNECTED;
      mutex_unlock(&mhl->pdata->mhl_status_lock);

      // usb device...
      mutex_lock(&mhl->i2c_lock);
      ret = I2CReadModify(mhl->pdata, PAGE_0, 0x18, BIT3, BIT3);
      if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
        goto exit_func;

      }
      mutex_unlock(&mhl->i2c_lock);
    }
  }

  exit_func:
  if(ret<0){
    mutex_unlock(&mhl->i2c_lock);
  }

  return ret;
}



static int mhl_rx_connected_func(struct mhl_tx *mhl)
{
  int ret=0;
  u8 int4status;

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_ReadByte(mhl->pdata, PAGE_3, 0x21, &int4status);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
    goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  if(int4status == 0x00){
    return ret;
  }

   mutex_lock(&mhl->i2c_lock);
  //clear interrupt status...
  ret = I2C_WriteByte(mhl->pdata,PAGE_3, 0x21, int4status);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
    goto exit_func;

  }
  mutex_unlock(&mhl->i2c_lock);

  if(int4status & BIT_INTR4_MHL_EST){
    mutex_lock(&mhl->pdata->mhl_status_lock);
    mhl->pdata->status.op_status = MHL_DISCOVERY_SUCCESS;
    mutex_unlock(&mhl->pdata->mhl_status_lock);

    mutex_lock(&mhl->i2c_lock);
    ret = I2C_WriteByte(mhl->pdata,PAGE_3, 0x30, 0x10);
    if(ret<0){
      printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
      goto exit_func;
    }
    mutex_unlock(&mhl->i2c_lock);

    mutex_lock(&mhl->i2c_lock);
    ret = I2C_WriteByte(mhl->pdata,PAGE_CBUS, 0x07, 0xF2);
    if(ret<0){
      printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
      goto exit_func;
    }
    mutex_unlock(&mhl->i2c_lock);

    mutex_lock(&mhl->i2c_lock);
    ret = I2CReadModify(mhl->pdata, PAGE_3, 0x10, BIT0, BIT0);
    if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
      goto exit_func;

    }
    mutex_unlock(&mhl->i2c_lock);

    mutex_lock(&mhl->i2c_lock);
    ret = I2C_WriteByte(mhl->pdata,PAGE_3, 0x22, ( BIT_INTR4_CBUS_LKOUT | BIT_INTR4_CBUS_DISCONNECT  ));
    if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
    }
    mutex_unlock(&mhl->i2c_lock);

    mutex_lock(&mhl->i2c_lock);
  // CBUS interrupt #1
    ret = I2C_WriteByte(mhl->pdata,PAGE_CBUS, 0x09, (BIT2 | BIT3 | BIT4 | BIT5 | BIT6));
    if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
    }
    mutex_unlock(&mhl->i2c_lock);

    mutex_lock(&mhl->i2c_lock);
  // CBUS interrupt #2
    ret = I2C_WriteByte(mhl->pdata,PAGE_CBUS, 0x1F, (BIT2 | BIT3));
    if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
    }
    mutex_unlock(&mhl->i2c_lock);

#if 0
  ret = I2C_WriteByte(mhl->pdata,PAGE_0,0x3C, ( BIT_TPI_INTR_ST0_HDCP_AUTH_STATUS_CHANGE_EVENT
                                        | BIT_TPI_INTR_ST0_HDCP_VPRIME_VALUE_READY_EVENT
                                        | BIT_TPI_INTR_ST0_HDCP_SECURITY_CHANGE_EVENT
                                        | BIT_TPI_INTR_ST0_HOT_PLUG_EVENT
                                        ));
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		return ret;
  }
#endif

    mutex_lock(&mhl->cbus_cmd_lock);
    mhl->msc_cmd_q[mhl->cmd_rx_cnt].command = MHL_SET_INT;
    mhl->msc_cmd_q[mhl->cmd_rx_cnt].offset= MHL_RCHANGE_INT;
    mhl->msc_cmd_q[mhl->cmd_rx_cnt].lenght= 0x01;
    mhl->msc_cmd_q[mhl->cmd_rx_cnt].buff[0]= MHL_INT_DCAP_CHG;
    mhl->cmd_rx_cnt += 1;
    if(mhl->cmd_rx_cnt == 1){
      queue_work(mhl->cbus_cmd_wqs, &mhl->cbus_cmd_work);
    }
    mutex_unlock(&mhl->cbus_cmd_lock);

  }

  if(int4status & BIT_INTR4_USB_EST){
    mutex_lock(&mhl->pdata->mhl_status_lock);
    mhl->pdata->status.op_status = MHL_DISCOVERY_FAIL;
    mutex_unlock(&mhl->pdata->mhl_status_lock);
    return ret;
  }

  if(int4status & BIT4){
    ret = forceUSBIDswitchOpen(mhl);
    if(ret<0){
      printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
      return ret;
    }

    ret = releaseUSBIDswitchOpen(mhl);
    if(ret<0){
      printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
      return ret;
    }
  }

  exit_func:

  if(ret<0){
    mutex_unlock(&mhl->i2c_lock);
  }
  return ret;

}



static int cbusprocsess_error(struct mhl_platform_data *pdata, u8 status, bool *abort)
{
  int ret = 0;
  u8 ddcabortreason =0;
  u8 msc_abort_intstatus = 0;

  status &= (BIT_MSC_ABORT | BIT_MSC_XFR_ABORT | BIT_DDC_ABORT);

  if(status){
    *abort = true;

    if(status & BIT_DDC_ABORT){
      printk(KERN_INFO "Sii8332: %s():%d BIT_CBUS_DDC_ABRT \n", __func__, __LINE__);
      ret = I2C_ReadByte(pdata, PAGE_CBUS, 0x0B, &ddcabortreason);
      if(ret<0){
        printk(KERN_INFO "[ERROR]sii8240: %s():%d failed !\n", __func__, __LINE__);
        goto exit_func;
      }
    }

    if(status & BIT_MSC_XFR_ABORT){
      printk(KERN_INFO "Sii8332: %s():%d BIT_MSC_XFR_ABORT \n", __func__, __LINE__);
      ret = I2C_ReadByte(pdata, PAGE_CBUS, 0x0D, &msc_abort_intstatus);
      if(ret<0){
        printk(KERN_INFO "[ERROR]Sii8332: %s():%d failed !\n", __func__, __LINE__);
        goto exit_func;
      }

      ret = I2C_WriteByte(pdata,PAGE_CBUS, 0x0D, msc_abort_intstatus);
      if(ret<0){
	printk(KERN_INFO "[ERROR]Sii8332: %s():%d failed !\n", __func__, __LINE__);
        goto exit_func;
      }

      if (msc_abort_intstatus){
        printk(KERN_INFO "Sii8332: %s():%d CBUS:: MSC Transfer ABORTED. Clearing 0x0D \n", __func__, __LINE__);

        if (CBUSABORT_BIT_REQ_MAXFAIL & msc_abort_intstatus){
          printk(KERN_INFO "Sii8332: %s():%d CBUSABORT_BIT_REQ_MAXFAIL \n", __func__, __LINE__);
        }
        if (CBUSABORT_BIT_PROTOCOL_ERROR & msc_abort_intstatus){
          printk(KERN_INFO "Sii8332: %s():%d CBUSABORT_BIT_PROTOCOL_ERROR \n", __func__, __LINE__);
        }
        if (CBUSABORT_BIT_REQ_TIMEOUT & msc_abort_intstatus){
          printk(KERN_INFO "Sii8332: %s():%d CBUSABORT_BIT_REQ_TIMEOUT \n", __func__, __LINE__);
        }
        if (CBUSABORT_BIT_PEER_ABORTED & msc_abort_intstatus){
          printk(KERN_INFO "Sii8332: %s():%d CBUSABORT_BIT_PEER_ABORTED \n", __func__, __LINE__);
        }
        if (CBUSABORT_BIT_UNDEFINED_OPCODE & msc_abort_intstatus){
          printk(KERN_INFO "Sii8332: %s():%d CBUSABORT_BIT_UNDEFINED_OPCODE \n", __func__, __LINE__);
        }
      }

    }

  }

  exit_func:

    return ret;

}


static int mhl_cbus_intr(struct mhl_tx *mhl)
{
  int ret=0;
  u8 intr_status=0;
  u8 subcmd, msg_data;

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_ReadByte(mhl->pdata, PAGE_CBUS, 0x08, &intr_status);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
    goto exit_func;

  }
  mutex_unlock(&mhl->i2c_lock);

  if(intr_status){

    mutex_lock(&mhl->i2c_lock);
    ret = I2C_WriteByte(mhl->pdata, PAGE_CBUS, 0x08, intr_status);
    if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
      goto exit_func;

    }
    mutex_unlock(&mhl->i2c_lock);

    if(intr_status & BIT3){

      mutex_lock(&mhl->i2c_lock);
      ret = I2C_ReadByte(mhl->pdata, PAGE_CBUS, 0x18, &subcmd);
      if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
        goto exit_func;

      }
      mutex_unlock(&mhl->i2c_lock);

      mutex_lock(&mhl->i2c_lock);
      ret = I2C_ReadByte(mhl->pdata, PAGE_CBUS, 0x19, &msg_data);
      if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
        goto exit_func;

      }
      mutex_unlock(&mhl->i2c_lock);

      switch(subcmd)
      {
		case	MHL_MSC_MSG_RAP:

			if( MHL_RAP_CONTENT_ON == msg_data){
			  mutex_lock(&mhl->i2c_lock);
            ret = I2CReadModify(mhl->pdata,PAGE_0,0x1A,TMDS_OUTPUT_CONTROL_MASK | AV_MUTE_MASK | OUTPUT_MODE_MASK,
              TMDS_OUTPUT_CONTROL_ACTIVE | AV_MUTE_NORMAL|OUTPUT_MODE_HDMI);
            if(ret<0){
			printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
              goto exit_func;
            }
            mutex_unlock(&mhl->i2c_lock);

            mutex_lock(&mhl->i2c_lock);
            ret = I2C_WriteByte(mhl->pdata,PAGE_3, 0x31, 0xFC);
            if(ret<0){
			printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
              goto exit_func;
            }
            mutex_unlock(&mhl->i2c_lock);
			}
			else if( MHL_RAP_CONTENT_OFF == msg_data){
            mutex_lock(&mhl->i2c_lock);
            ret = I2CReadModify(mhl->pdata,PAGE_0,0x1A,  TMDS_OUTPUT_CONTROL_MASK|OUTPUT_MODE_MASK,
              TMDS_OUTPUT_CONTROL_POWER_DOWN    | OUTPUT_MODE_DVI);
            if(ret<0){
			printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
              goto exit_func;
            }
            mutex_unlock(&mhl->i2c_lock);

            mutex_lock(&mhl->i2c_lock);
            ret = I2C_WriteByte(mhl->pdata,PAGE_3, 0x31, 0xC0);
            if(ret<0){
			printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
              goto exit_func;
            }
            mutex_unlock(&mhl->i2c_lock);
			}

			SiiMhlTxRapkSend(mhl);
		 break;

		case	MHL_MSC_MSG_RCP:

			if(MHL_LOGICAL_DEVICE_MAP & rcpSupportTable [msg_data & 0x7F] ){
            SiiMhlTxRcpkSend(mhl,msg_data);
			}
			else{
				SiiMhlTxRcpeSend(mhl, RCPE_INEEFECTIVE_KEY_CODE, msg_data );
			}
		 break;

		case	MHL_MSC_MSG_RCPK:
			break;

		case	MHL_MSC_MSG_RCPE:
			break;

		case	MHL_MSC_MSG_RAPK:
			break;

		default:
			// Any freak value here would continue with no event to app
			break;
		}

    }

    if(intr_status & (BIT_MSC_ABORT | BIT_MSC_XFR_ABORT | BIT_DDC_ABORT)){
      mutex_lock(&mhl->i2c_lock);
      ret = cbusprocsess_error(mhl->pdata,intr_status, &mhl->msc_cmd_abord);
      if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
        goto exit_func;
      }
      mutex_unlock(&mhl->i2c_lock);

      if(mhl->msc_cmd_abord == true){
        printk(KERN_INFO "sii8332: %s():%d msc_cmd_abord !\n", __func__, __LINE__);
	if (waitqueue_active(&mhl->cbus_hanler_wq)){
          wake_up(&mhl->cbus_hanler_wq);
        }
      }

    }

     if(intr_status & BIT4){
      mhl->msc_cmd_done_intr = MSC_DONE_ACK;
	if (waitqueue_active(&mhl->cbus_hanler_wq)){
        wake_up(&mhl->cbus_hanler_wq);
      }
    }

    if(intr_status & BIT7){
      mutex_lock(&mhl->i2c_lock);
      ret = I2CReadModify(mhl->pdata,PAGE_CBUS, 0x38, 0x0F, 0x00);
      if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
        goto exit_func;
      }
      mutex_unlock(&mhl->i2c_lock);
    }
  }

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_ReadByte(mhl->pdata, PAGE_CBUS, 0x1E, &intr_status);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
    goto exit_func;

  }
  mutex_unlock(&mhl->i2c_lock);

  if(intr_status){

    mutex_lock(&mhl->i2c_lock);
    ret = I2C_WriteByte(mhl->pdata, PAGE_CBUS, 0x1E, intr_status);
    if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
      goto exit_func;

    }
    mutex_unlock(&mhl->i2c_lock);

    if(intr_status & BIT0){
		printk(KERN_INFO "sii8332: %s():%d write burst done !\n", __func__, __LINE__);
    }

    if(intr_status & BIT2){
      u8 intr[4];
      memset(intr, 0x00, 4);

      mutex_lock(&mhl->i2c_lock);
      ret = I2C_ReadByte_block(mhl->pdata, PAGE_CBUS, 0xA0, 4, intr );
      if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
        goto exit_func;

      }
      mutex_unlock(&mhl->i2c_lock);

      mutex_lock(&mhl->i2c_lock);
      ret = I2C_WriteByte_block(mhl->pdata, PAGE_CBUS, 0xA0, 4, intr );
      if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
        goto exit_func;

      }
      mutex_unlock(&mhl->i2c_lock);

      if(intr[0]){

	if(MHL_INT_DCAP_CHG & intr[0]){
          mutex_lock(&mhl->cbus_cmd_lock);
          mhl->msc_cmd_q[mhl->cmd_rx_cnt].command = MHL_READ_DEVCAP;
          mhl->msc_cmd_q[mhl->cmd_rx_cnt].offset= MHL_DEV_CATEGORY_OFFSET;
          mhl->msc_cmd_q[mhl->cmd_rx_cnt].lenght= 0;
          mhl->cmd_rx_cnt += 1;

          if(mhl->cmd_rx_cnt == 1){
            queue_work(mhl->cbus_cmd_wqs, &mhl->cbus_cmd_work);
          }
          mutex_unlock(&mhl->cbus_cmd_lock);
	}

	if(MHL_INT_DSCR_CHG & intr[0]){
          //printk(KERN_INFO "sii8332: %s():%d MHL_INT_DSCR_CHG !\n", __func__, __LINE__);
	}

	if(MHL_INT_REQ_WRT & intr[0]){
          mutex_lock(&mhl->cbus_cmd_lock);
          mhl->msc_cmd_q[mhl->cmd_rx_cnt].command = MHL_SET_INT;
          mhl->msc_cmd_q[mhl->cmd_rx_cnt].offset= MHL_RCHANGE_INT;
          mhl->msc_cmd_q[mhl->cmd_rx_cnt].lenght= 0x01;
          mhl->msc_cmd_q[mhl->cmd_rx_cnt].buff[0]= MHL_INT_GRT_WRT;
          mhl->cmd_rx_cnt += 1;
          if(mhl->cmd_rx_cnt == 1){
            queue_work(mhl->cbus_cmd_wqs, &mhl->cbus_cmd_work);
          }
          mutex_unlock(&mhl->cbus_cmd_lock);
	}

	if(MHL_INT_GRT_WRT & intr[0]){
          //printk(KERN_INFO "sii8332: %s():%d MHL_INT_GRT_WRT !\n", __func__, __LINE__);
	}
      }

      if(intr[1]& MHL_INT_EDID_CHG){
        printk(KERN_INFO "sii8332: %s():%d MHL_INT_EDID_CHG !\n", __func__, __LINE__);
        mhl->avi_work = true;
        mhl->avi_cmd = HPD_HIGH_EVENT;
	if (waitqueue_active(&mhl->avi_control_wq)){
          wake_up(&mhl->avi_control_wq);
        }
        else{
          queue_work(mhl->avi_control_wqs, &mhl->avi_control_work);
        }
      }


    }

    if(intr_status & BIT3){
      u8 tmp1[4];
      u8 tmp2[4] = {0xFF, 0xFF, 0xFF, 0xFF};

      memset(tmp1, 0x00, 4);

      mutex_lock(&mhl->i2c_lock);
      ret = I2C_ReadByte_block(mhl->pdata, PAGE_CBUS, 0xB0, 4, tmp1 );
      if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
        goto exit_func;

      }
      mutex_unlock(&mhl->i2c_lock);


      mutex_lock(&mhl->i2c_lock);
      ret = I2C_WriteByte_block(mhl->pdata, PAGE_CBUS, 0xB0, 4, tmp2 );
      if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
        goto exit_func;

      }
      mutex_unlock(&mhl->i2c_lock);

      if(MHL_STATUS_DCAP_RDY & tmp1[0]){
          mutex_lock(&mhl->cbus_cmd_lock);
          mhl->msc_cmd_q[mhl->cmd_rx_cnt].command = MHL_READ_DEVCAP;
          mhl->msc_cmd_q[mhl->cmd_rx_cnt].offset= MHL_DEV_CATEGORY_OFFSET;
          mhl->msc_cmd_q[mhl->cmd_rx_cnt].lenght= 0;
          mhl->cmd_rx_cnt += 1;

          if(mhl->cmd_rx_cnt == 1){
            queue_work(mhl->cbus_cmd_wqs, &mhl->cbus_cmd_work);
          }
          mutex_unlock(&mhl->cbus_cmd_lock);
      }

      if((MHL_STATUS_PATH_ENABLED & tmp1[1])&&
          !(mhl->pdata->status.linkmode & MHL_STATUS_PATH_ENABLED)){
        mhl->pdata->status.linkmode |= MHL_STATUS_PATH_ENABLED;
        mutex_lock(&mhl->cbus_cmd_lock);
        mhl->msc_cmd_q[mhl->cmd_rx_cnt].command = MHL_WRITE_STAT;
        mhl->msc_cmd_q[mhl->cmd_rx_cnt].offset= MHL_STATUS_REG_LINK_MODE;
        mhl->msc_cmd_q[mhl->cmd_rx_cnt].lenght= 0x01;
        mhl->msc_cmd_q[mhl->cmd_rx_cnt].buff[0]= mhl->pdata->status.linkmode;
        mhl->cmd_rx_cnt += 1;

        if(mhl->cmd_rx_cnt == 1){
          queue_work(mhl->cbus_cmd_wqs, &mhl->cbus_cmd_work);
        }
        mutex_unlock(&mhl->cbus_cmd_lock);

      }
      else if(!(MHL_STATUS_PATH_ENABLED & tmp1[1])&&
          (mhl->pdata->status.linkmode & MHL_STATUS_PATH_ENABLED)){
        mhl->pdata->status.linkmode &= ~MHL_STATUS_PATH_ENABLED;
        mutex_lock(&mhl->cbus_cmd_lock);
        mhl->msc_cmd_q[mhl->cmd_rx_cnt].command = MHL_WRITE_STAT;
        mhl->msc_cmd_q[mhl->cmd_rx_cnt].offset= MHL_STATUS_REG_LINK_MODE;
        mhl->msc_cmd_q[mhl->cmd_rx_cnt].lenght= 0x01;
        mhl->msc_cmd_q[mhl->cmd_rx_cnt].buff[0]= mhl->pdata->status.linkmode;
        mhl->cmd_rx_cnt += 1;

        if(mhl->cmd_rx_cnt == 1){
          queue_work(mhl->cbus_cmd_wqs, &mhl->cbus_cmd_work);
        }
        mutex_unlock(&mhl->cbus_cmd_lock);

      }
    }


  }

  exit_func:
    if(ret<0){
      mutex_unlock(&mhl->i2c_lock);
    }

  return ret;
}

static int get_ddc_access(struct mhl_tx *mhl, u8 *sys_ctrl, bool *result)
{
  int ret = 0;
  u8 timeoout = 50, tmp = 0;
  u8 tpi_controlImage = 0;

  *result = false;

  ret = I2C_ReadByte(mhl->pdata, PAGE_0, 0x1A, &tmp);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
    goto exit_func;
  }

  *sys_ctrl = tmp;
  tmp |= DDC_BUS_REQUEST_REQUESTED;

  ret = I2C_WriteByte(mhl->pdata, PAGE_0, 0x1A, tmp);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
    goto exit_func;
  }

  while(timeoout--)
  {
    ret = I2C_ReadByte(mhl->pdata, PAGE_0, 0x1A, &tpi_controlImage);
    if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
      goto exit_func;
    }

    if(tpi_controlImage & DDC_BUS_GRANT_MASK){
      tmp |= DDC_BUS_GRANT_GRANTED;
      ret = I2C_WriteByte(mhl->pdata, PAGE_0, 0x1A, tmp);
      if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
        goto exit_func;
      }
      *result = true;
      return ret;
    }

    ret = I2C_WriteByte(mhl->pdata, PAGE_0, 0x1A, tmp);
    if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
      goto exit_func;
    }

    msleep(200);

  }

  ret = I2C_WriteByte(mhl->pdata, PAGE_0, 0x1A, tmp);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
    goto exit_func;
  }

  exit_func:

  return ret;
}


static int release_ddc_access(struct mhl_tx *mhl, u8 sys_ctrl, bool *result)
{
  int ret = 0;
  u8 timeoout = 50;
  u8 tpi_controlImage = 0;

  *result = false;

  sys_ctrl &= ~(DDC_BUS_REQUEST_MASK| DDC_BUS_GRANT_MASK);

  while(timeoout --)
  {
    ret = I2C_WriteByte(mhl->pdata, PAGE_0, 0x1A, sys_ctrl);
    if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
      return ret;
    }

    ret = I2C_ReadByte(mhl->pdata, PAGE_0, 0x1A, &tpi_controlImage);
    if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
      return ret;
    }

		if (!(tpi_controlImage & (DDC_BUS_REQUEST_MASK| DDC_BUS_GRANT_MASK)))		// When 0x1A[2:1] read "0"
			*result = true;
  }

  return ret;
}



static bool edid_header_check(struct mhl_tx *mhl)
{
  u8 i=0;

  if(mhl->edid_block0[0] != 0x00){
    return false;
  }

  for(i=1; i<= 6; i++)
  {
    if(mhl->edid_block0[i] != 0xFF){
      return false;
    }
  }

  if(mhl->edid_block0[7] != 0x00){
    return false;
  }

  return true;
}

static bool edid_checksum(struct mhl_tx *mhl, u8 block_num)
{
  u8 i=0, checksum = 0;

  for(i=0; i<EDID_SIZE; i++)
  {
    if(block_num == 0){
      checksum += mhl->edid_block0[i];
    }
    else if(block_num == 1){
      checksum += mhl->edid_block1[i];
    }
    else if(block_num == 2){
      checksum += mhl->edid_block2[i];
    }
    else if(block_num == 3){
      checksum += mhl->edid_block3[i];
    }
  }

  if(checksum){
    return false;
  }
  else{
    return true;
  }

}

static bool ParseDetailedTiming(struct mhl_tx *mhl, u8 DetailedTimingOffset, u8 Block)
{
	u8 TmpByte;
	u8 i;
	u16 TmpWord;
  u8 edidBlockData [EDID_BLOCK_SIZE];

  memset(edidBlockData, 0x00, EDID_BLOCK_SIZE);
  if(Block == EDID_BLOCK_0){
    memcpy(edidBlockData, mhl->edid_block0, EDID_BLOCK_SIZE);
  }
	TmpWord = edidBlockData[DetailedTimingOffset + PIX_CLK_OFFSET] +
		256 * edidBlockData[DetailedTimingOffset + PIX_CLK_OFFSET + 1];

	if (TmpWord == 0x00)            // 18 byte partition is used as either for Monitor Name or for Monitor Range Limits or it is unused
	{
		if (Block == EDID_BLOCK_0)      // if called from Block #0 and first 2 bytes are 0 => either Monitor Name or for Monitor Range Limits
		{
			if (edidBlockData[DetailedTimingOffset + 3] == 0xFC) // these 13 bytes are ASCII coded monitor name
			{
				printk("Monitor Name: ");

				for (i = 0; i < 13; i++)
				{
					printk("%c", edidBlockData[DetailedTimingOffset + 5 + i]); // Display monitor name
				}
				printk("\n");
			}

			else if (edidBlockData[DetailedTimingOffset + 3] == 0xFD) // these 13 bytes contain Monitor Range limits, binary coded
			{
				printk("Monitor Range Limits:\n\n");

				i = 0;
				printk("Min Vertical Rate in Hz: %d\n", (int) edidBlockData[DetailedTimingOffset + 5 + i++]); //
				printk("Max Vertical Rate in Hz: %d\n", (int) edidBlockData[DetailedTimingOffset + 5 + i++]); //
				printk("Min Horizontal Rate in Hz: %d\n", (int) edidBlockData[DetailedTimingOffset + 5 + i++]); //
				printk("Max Horizontal Rate in Hz: %d\n", (int) edidBlockData[DetailedTimingOffset + 5 + i++]); //
				printk("Max Supported pixel clock rate in MHz/10: %d\n", (int) edidBlockData[DetailedTimingOffset + 5 + i++]); //
				printk("Tag for secondary timing formula (00h=not used): %d\n", (int) edidBlockData[DetailedTimingOffset + 5 + i++]); //
				printk("Min Vertical Rate in Hz %d\n", (int) edidBlockData[DetailedTimingOffset + 5 + i]); //
				printk("\n");
			}
		}
		else if (Block == EDID_BLOCK_2_3)                          // if called from block #2 or #3 and first 2 bytes are 0x00 (padding) then this
		{                                                                                          // descriptor partition is not used and parsing should be stopped
			printk("No More Detailed descriptors in this block\n");
			printk("\n");
			return false;
		}
	}

	else                                            // first 2 bytes are not 0 => this is a detailed timing descriptor from either block
	{
		if((Block == EDID_BLOCK_0) && (DetailedTimingOffset == 0x36))
		{
			printk("\n\n\nParse Results, EDID Block #0, Detailed Descriptor Number 1:\n");
			printk("===========================================================\n\n");
		}
		else if((Block == EDID_BLOCK_0) && (DetailedTimingOffset == 0x48))
		{
			printk("\n\n\nParse Results, EDID Block #0, Detailed Descriptor Number 2:\n");
			printk("===========================================================\n\n");
		}

		printk("Pixel Clock (MHz * 100): %d\n", (int)TmpWord);

		TmpWord = edidBlockData[DetailedTimingOffset + H_ACTIVE_OFFSET] +
			256 * ((edidBlockData[DetailedTimingOffset + H_ACTIVE_OFFSET + 2] >> 4) & FOUR_LSBITS);
		printk("Horizontal Active Pixels: %d\n", (int)TmpWord);

		TmpWord = edidBlockData[DetailedTimingOffset + H_BLANKING_OFFSET] +
			256 * (edidBlockData[DetailedTimingOffset + H_BLANKING_OFFSET + 1] & FOUR_LSBITS);
		printk("Horizontal Blanking (Pixels): %d\n", (int)TmpWord);

		TmpWord = (edidBlockData[DetailedTimingOffset + V_ACTIVE_OFFSET] )+
			256 * ((edidBlockData[DetailedTimingOffset + (V_ACTIVE_OFFSET) + 2] >> 4) & FOUR_LSBITS);
		printk("Vertical Active (Lines): %d\n", (int)TmpWord);

		TmpWord = edidBlockData[DetailedTimingOffset + V_BLANKING_OFFSET] +
			256 * (edidBlockData[DetailedTimingOffset + V_BLANKING_OFFSET + 1] & FOUR_LSBITS);
		printk("Vertical Blanking (Lines): %d\n", (int)TmpWord);

		TmpWord = edidBlockData[DetailedTimingOffset + H_SYNC_OFFSET] +
			256 * ((edidBlockData[DetailedTimingOffset + (H_SYNC_OFFSET + 3)] >> 6) & TWO_LSBITS);
		printk("Horizontal Sync Offset (Pixels): %d\n", (int)TmpWord);

		TmpWord = edidBlockData[DetailedTimingOffset + H_SYNC_PW_OFFSET] +
			256 * ((edidBlockData[DetailedTimingOffset + (H_SYNC_PW_OFFSET + 2)] >> 4) & TWO_LSBITS);
		printk("Horizontal Sync Pulse Width (Pixels): %d\n", (int)TmpWord);

		TmpWord = ((edidBlockData[DetailedTimingOffset + V_SYNC_OFFSET] >> 4) & FOUR_LSBITS) +
			256 * ((edidBlockData[DetailedTimingOffset + (V_SYNC_OFFSET + 1)] >> 2) & TWO_LSBITS);
		printk("Vertical Sync Offset (Lines): %d\n", (int)TmpWord);

		TmpWord = ((edidBlockData[DetailedTimingOffset + V_SYNC_PW_OFFSET]) & FOUR_LSBITS) +
			256 * (edidBlockData[DetailedTimingOffset + (V_SYNC_PW_OFFSET + 1)] & TWO_LSBITS);
		printk("Vertical Sync Pulse Width (Lines): %d\n", (int)TmpWord);

		TmpWord = edidBlockData[DetailedTimingOffset + H_IMAGE_SIZE_OFFSET] +
			256 * (((edidBlockData[DetailedTimingOffset + (H_IMAGE_SIZE_OFFSET + 2)]) >> 4) & FOUR_LSBITS);
		printk("Horizontal Image Size (mm): %d\n", (int)TmpWord);

		TmpWord = edidBlockData[DetailedTimingOffset + V_IMAGE_SIZE_OFFSET] +
			256 * (edidBlockData[DetailedTimingOffset + (V_IMAGE_SIZE_OFFSET + 1)] & FOUR_LSBITS);
		printk("Vertical Image Size (mm): %d\n", (int)TmpWord);

		TmpByte = edidBlockData[DetailedTimingOffset + H_BORDER_OFFSET];
		printk("Horizontal Border (Pixels): %d\n", (int)TmpByte);

		TmpByte = edidBlockData[DetailedTimingOffset + V_BORDER_OFFSET];
		printk("Vertical Border (Lines): %d\n", (int)TmpByte);

		TmpByte = edidBlockData[DetailedTimingOffset + FLAGS_OFFSET];
		if (TmpByte & BIT7)
		{
			printk("Interlaced\n");
		}
		else
		{
			printk("Non-Interlaced\n");
		}

		if (!(TmpByte & BIT5) && !(TmpByte & BIT6))
		{
			printk("Normal Display, No Stereo\n");
		}
		else
		{
		  printk("Refer to VESA E-EDID Release A, Revision 1, table 3.17\n");
		}

		if (!(TmpByte & BIT3) && !(TmpByte & BIT4))
		{
			printk("Analog Composite\n");
		}
		else if ((TmpByte & BIT3) && !(TmpByte & BIT4))
		{
			printk("Bipolar Analog Composite\n");
		}
		else if (!(TmpByte & BIT3) && (TmpByte & BIT4))
		{
			printk("Digital Composite\n");
		}
		else if ((TmpByte & BIT3) && (TmpByte & BIT4))
		{
			printk("Digital Separate\n");
		}

		printk("\n");
	}
	return true;
}


static bool parse861longdescriptors(struct mhl_tx *mhl)
{
    u8 LongDescriptorsOffset;
    u8 DescriptorNum = 1;

    LongDescriptorsOffset = mhl->edid_block1[LONG_DESCR_PTR_IDX];   // EDID block offset 2 holds the offset

    if (!LongDescriptorsOffset)                         // per CEA-861-D, table 27
    {
        printk("EDID -> No Detailed Descriptors\n");
        return false;
    }

    // of the 1st 18-byte descriptor
    while (LongDescriptorsOffset + LONG_DESCR_LEN < EDID_BLOCK_SIZE)
    {
        printk("Parse Results - CEA-861 Long Descriptor #%d:\n", (int) DescriptorNum);
        printk("===============================================================\n");


        if (!ParseDetailedTiming(mhl,LongDescriptorsOffset, EDID_BLOCK_2_3))
		{
			return false;
		}

        LongDescriptorsOffset +=  LONG_DESCR_LEN;
        DescriptorNum++;
    }

    return true;
}

static bool parse861shortdescriptors (struct mhl_tx *mhl)
{
  u8 LongDescriptorOffset;
  u8 DataBlockLength;
  u8 DataIndex;
  u8 ExtendedTagCode;
  u8 VSDB_BaseOffset = 0;

  u8 V_DescriptorIndex = 0;  // static to support more than one extension
  u8 A_DescriptorIndex = 0;  // static to support more than one extension

  u8 TagCode;

  u8 i;
  u8 j;

  if (mhl->edid_block1[EDID_TAG_ADDR] != EDID_EXTENSION_TAG){
      printk("EDID -> Extension Tag Error\n");
      return false;
  }

  if (mhl->edid_block1[EDID_REV_ADDR] != EDID_REV_THREE){
      printk("EDID -> Revision Error\n");
      return false;
  }

  LongDescriptorOffset = mhl->edid_block1[LONG_DESCR_PTR_IDX];    // block offset where long descriptors start

  mhl->EDID_Data.UnderScan = ((mhl->edid_block1[MISC_SUPPORT_IDX]) >> 7) & BIT0;  // byte #3 of CEA extension version 3
  mhl->EDID_Data.BasicAudio = ((mhl->edid_block1[MISC_SUPPORT_IDX]) >> 6) & BIT0;
  mhl->EDID_Data.YCbCr_4_4_4 = ((mhl->edid_block1[MISC_SUPPORT_IDX]) >> 5) & BIT0;
  mhl->EDID_Data.YCbCr_4_2_2 = ((mhl->edid_block1[MISC_SUPPORT_IDX]) >> 4) & BIT0;

  if (mhl->EDID_Data.YCbCr_4_4_4 == 1)
	{
		//outputColorSpace = BIT_TPI_OUTPUT_FORMAT_YCbCr444;
	}
	else if (mhl->EDID_Data.YCbCr_4_2_2 == 1)
	{
		//outputColorSpace = BIT_TPI_OUTPUT_FORMAT_YCbCr422;
	}
	else
	{
		//outputColorSpace = BIT_TPI_OUTPUT_FORMAT_HDMI_TO_RGB;
	}

  DataIndex = EDID_DATA_START;            // 4

  while (DataIndex < LongDescriptorOffset)
  {
      TagCode = (mhl->edid_block1[DataIndex] >> 5) & THREE_LSBITS;
      DataBlockLength = mhl->edid_block1[DataIndex++] & FIVE_LSBITS;
      if ((DataIndex + DataBlockLength) > LongDescriptorOffset)
      {
          printk("EDID -> V Descriptor Overflow\n");
          return false;
      }

      i = 0;                                  // num of short video descriptors in current data block

      switch (TagCode)
      {
          case VIDEO_D_BLOCK:
            while ((i < DataBlockLength) && (i < MAX_V_DESCRIPTORS))        // each SVD is 1 byte long
            {
                mhl->EDID_Data.VideoDescriptor[V_DescriptorIndex++] = mhl->edid_block1[DataIndex++];
                i++;
            }
            DataIndex += DataBlockLength - i;   // if there are more STDs than MAX_V_DESCRIPTORS, skip the last ones. Update DataIndex

            printk("EDID -> Short Descriptor Video Block\n");
            break;

          case AUDIO_D_BLOCK:
            while (i < DataBlockLength/3)       // each SAD is 3 bytes long
            {
                j = 0;
                while (j < AUDIO_DESCR_SIZE)    // 3
                {
                    mhl->EDID_Data.AudioDescriptor[A_DescriptorIndex][j++] = mhl->edid_block1[DataIndex++];
                }
                A_DescriptorIndex++;
                i++;
            }
            printk("EDID -> Short Descriptor Audio Block\n");
            break;

          case SPKR_ALLOC_D_BLOCK:
              mhl->EDID_Data.SpkrAlloc[i++] = mhl->edid_block1[DataIndex++];       // although 3 bytes are assigned to Speaker Allocation, only
              DataIndex += 2;                                     // the first one carries information, so the next two are ignored by this code.
              printk("EDID -> Short Descriptor Speaker Allocation Block\n");
            break;

          case USE_EXTENDED_TAG:
              ExtendedTagCode = mhl->edid_block1[DataIndex++];

              switch (ExtendedTagCode)
              {
                  case VIDEO_CAPABILITY_D_BLOCK:

					        printk("EDID -> Short Descriptor Video Capability Block\n");

						// TO BE ADDED HERE: Save "video capability" parameters in EDID_Data data structure
						// Need to modify that structure definition
						// In the meantime: just increment DataIndex by 1

					        DataIndex += 1;    // replace with reading and saving the proper data per CEA-861 sec. 7.5.6 while incrementing DataIndex

					        break;

                  case COLORIMETRY_D_BLOCK:
                        mhl->EDID_Data.ColorimetrySupportFlags = mhl->edid_block1[DataIndex++] & TWO_LSBITS;
                        mhl->EDID_Data.MetadataProfile = mhl->edid_block1[DataIndex++] & THREE_LSBITS;

					        printk("EDID -> Short Descriptor Colorimetry Block\n");
					        break;
              }
			    break;

          case VENDOR_SPEC_D_BLOCK:
              VSDB_BaseOffset = DataIndex - 1;

              if ((mhl->edid_block1[DataIndex++] == 0x03) &&    // check if sink is HDMI compatible
                  (mhl->edid_block1[DataIndex++] == 0x0C) &&
                  (mhl->edid_block1[DataIndex++] == 0x00)){
                  mhl->EDID_Data.HDMI_Sink = true;
                  mhl->hdmi_sink = true;
              }
              else{
                  mhl->EDID_Data.HDMI_Sink = false;
                  mhl->hdmi_sink = false;
              }

              mhl->EDID_Data.CEC_A_B = mhl->edid_block1[DataIndex++];  // CEC Physical address
              mhl->EDID_Data.CEC_C_D = mhl->edid_block1[DataIndex++];

              if ((DataIndex + 7) > VSDB_BaseOffset + DataBlockLength){        // Offset of 3D_Present bit in VSDB
                      mhl->EDID_Data._3D_Supported = false;
              }
              else if (mhl->edid_block1[DataIndex + 7] >> 7){
                      mhl->EDID_Data._3D_Supported = true;
              }
              else{
                      mhl->EDID_Data._3D_Supported = false;
              }
              DataIndex += DataBlockLength - HDMI_SIGNATURE_LEN - CEC_PHYS_ADDR_LEN; // Point to start of next block
              printk("EDID -> Short Descriptor Vendor Block\n");
              printk("\n");
            break;

          default:
              printk("EDID -> Unknown Tag Code\n");
              return false;

      }                   // End, Switch statement
  }                       // End, while (DataIndex < LongDescriptorOffset) statement

    return true;
}

static void parsebock0_timingdescriptors(struct mhl_tx *mhl)
{
  u8 i=0;
  u8 offset = 0, ar_code = 0;

  printk("Parsing Established Timing:\n");
  printk("===========================\n");

  if(mhl->edid_block0[ESTABLISHED_TIMING_INDEX] & BIT7)
      printk("720 x 400 @ 70Hz\n");
  if(mhl->edid_block0[ESTABLISHED_TIMING_INDEX] & BIT6)
      printk("720 x 400 @ 88Hz\n");
  if(mhl->edid_block0[ESTABLISHED_TIMING_INDEX] & BIT5)
      printk("640 x 480 @ 60Hz\n");
  if(mhl->edid_block0[ESTABLISHED_TIMING_INDEX] & BIT4)
      printk("640 x 480 @ 67Hz\n");
  if(mhl->edid_block0[ESTABLISHED_TIMING_INDEX] & BIT3)
      printk("640 x 480 @ 72Hz\n");
  if(mhl->edid_block0[ESTABLISHED_TIMING_INDEX] & BIT2)
      printk("640 x 480 @ 75Hz\n");
  if(mhl->edid_block0[ESTABLISHED_TIMING_INDEX] & BIT1)
      printk("800 x 600 @ 56Hz\n");
  if(mhl->edid_block0[ESTABLISHED_TIMING_INDEX] & BIT0)
      printk("800 x 400 @ 60Hz\n");

  // Parse Established Timing Byte #1:

  if(mhl->edid_block0[ESTABLISHED_TIMING_INDEX + 1]  & BIT7)
      printk("800 x 600 @ 72Hz\n");
  if(mhl->edid_block0[ESTABLISHED_TIMING_INDEX + 1]  & BIT6)
      printk("800 x 600 @ 75Hz\n");
  if(mhl->edid_block0[ESTABLISHED_TIMING_INDEX + 1]  & BIT5)
      printk("832 x 624 @ 75Hz\n");
  if(mhl->edid_block0[ESTABLISHED_TIMING_INDEX + 1]  & BIT4)
      printk("1024 x 768 @ 87Hz\n");
  if(mhl->edid_block0[ESTABLISHED_TIMING_INDEX + 1]  & BIT3)
      printk("1024 x 768 @ 60Hz\n");
  if(mhl->edid_block0[ESTABLISHED_TIMING_INDEX + 1]  & BIT2)
      printk("1024 x 768 @ 70Hz\n");
  if(mhl->edid_block0[ESTABLISHED_TIMING_INDEX + 1]  & BIT1)
      printk("1024 x 768 @ 75Hz\n");
  if(mhl->edid_block0[ESTABLISHED_TIMING_INDEX + 1]  & BIT0)
      printk("1280 x 1024 @ 75Hz\n");

  // Parse Established Timing Byte #2:

  if(mhl->edid_block0[ESTABLISHED_TIMING_INDEX + 2] & 0x80)
      printk("1152 x 870 @ 75Hz\n");

  if((!mhl->edid_block0[0])&&(!mhl->edid_block0[ESTABLISHED_TIMING_INDEX + 1]  )&&(!mhl->edid_block0[2]))
      printk("No established video modes\n");

  printk("Parsing Standard Timing:\n");
  printk("========================\n");


  for (i = 0; i < NUM_OF_STANDARD_TIMINGS; i += 2)
  {
    if ((mhl->edid_block0[STANDARD_TIMING_OFFSET + i] == 0x01) && ((mhl->edid_block0[STANDARD_TIMING_OFFSET + i +1]) == 1))
    {
      printk("Standard Timing Undefined\n");
    }
    else
    {
      printk("Horizontal Active pixels: %i\n", (int)((mhl->edid_block0[STANDARD_TIMING_OFFSET + i] + 31)*8));

    ar_code = (mhl->edid_block0[STANDARD_TIMING_OFFSET + i +1] & TWO_MSBITS) >> 6;
    printk("Aspect Ratio: ");

    switch(ar_code)
    {
      case AR16_10:
      printk("16:10\n");
      break;

      case AR4_3:
      printk("4:3\n");
      break;

      case AR5_4:
      printk("5:4\n");
      break;

      case AR16_9:
      printk("16:9\n");
      break;
    }
    }
  }

  for (i = 0; i < NUM_OF_DETAILED_DESCRIPTORS; i++)
  {
      offset = DETAILED_TIMING_OFFSET + (LONG_DESCR_LEN * i);
      ParseDetailedTiming(mhl, offset, EDID_BLOCK_0);
  }

}

static int  read_edid_block0(struct mhl_tx *mhl)
{
	int ret;
	unsigned char data[1];

  struct i2c_client* client_ptr = get_simgI2C_client(mhl->pdata, DDC_0xA0);

  data[0] = 0x00;

	struct i2c_msg msg1[] = {
		{ .addr = 0xA0 >> 1,
		  .flags = 0,
		  .len = 1,
		  .buf = data },
		{ .addr = 0xA0>> 1,
		  .flags = I2C_M_RD,
		  .len = 0x80,
		  .buf = &mhl->edid_block0[0] } };

	ret = i2c_transfer(client_ptr->adapter, msg1, 2);

	return ret;
}

static int  read_edid_block1(struct mhl_tx *mhl)
{
	int ret;
	unsigned char data[1];

  struct i2c_client* client_ptr = get_simgI2C_client(mhl->pdata, DDC_0xA0);

  data[0] = 0x80;

	struct i2c_msg msg1[] = {
		{ .addr = 0xA0 >> 1,
		  .flags = 0,
		  .len = 1,
		  .buf = data },
		{ .addr = 0xA0>> 1,
		  .flags = I2C_M_RD,
		  .len = 0x80,
		  .buf = &mhl->edid_block1[0] } };

	ret = i2c_transfer(client_ptr->adapter, msg1, 2);

	return ret;
}

static int  read_edid_block2(struct mhl_tx *mhl)
{
	int ret;
	unsigned char data[1], data1[1] ;

  struct i2c_client* client_ptr1 = get_simgI2C_client(mhl->pdata, DDC_0x60);
  struct i2c_client* client_ptr2 = get_simgI2C_client(mhl->pdata, DDC_0xA0);

  data[0] = 0x01;
  data1[0] = 0x00;

	struct i2c_msg msg1[] = {
    { .addr = 0x60 >> 1,
		  .flags = 0,
		  .len = 1,
		  .buf = data },
		{ .addr = 0xA0 >> 1,
		  .flags = 0,
		  .len = 1,
		  .buf = data1 },
		{ .addr = 0xA0>> 1,
		  .flags = I2C_M_RD,
		  .len = 0x80,
		  .buf = &mhl->edid_block2[0] } };

	ret = i2c_transfer(client_ptr1->adapter, msg1, 3);

	return ret;
}

static int  read_edid_block3(struct mhl_tx *mhl)
{
	int ret;
	unsigned char data[1], data1[1] ;;

  struct i2c_client* client_ptr1 = get_simgI2C_client(mhl->pdata, DDC_0x60);
  struct i2c_client* client_ptr2 = get_simgI2C_client(mhl->pdata, DDC_0xA0);

  data[0] = 0x01;
  data1[0] = 0x80;

	struct i2c_msg msg1[] = {
    { .addr = 0x60 >> 1,
		  .flags = 0,
		  .len = 1,
		  .buf = data },
		{ .addr = 0xA0 >> 1,
		  .flags = 0,
		  .len = 1,
		  .buf = data1 },
		{ .addr = 0xA0>> 1,
		  .flags = I2C_M_RD,
		  .len = 0x80,
		  .buf = &mhl->edid_block3[0] } };

	ret = i2c_transfer(client_ptr1->adapter, msg1, 3);

	return ret;
}

static int edid_read(struct mhl_tx *mhl)
{
  u8 sys_ctrl = 0;
  int ret = 0;
  bool result = 0;
  u8 block_cnt = 0;

  DEBUG_MHL_FUNC_START();

  mutex_lock(&mhl->i2c_lock);
  ret = get_ddc_access(mhl,&sys_ctrl, &result);
  if(ret<0){
	  printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
    mutex_unlock(&mhl->i2c_lock);
   return ret;
  }

  memset(&mhl->edid_block0[0], 0x00, EDID_SIZE);
  memset(&mhl->edid_block1[0], 0x00, EDID_SIZE);
  memset(&mhl->edid_block2[0], 0x00, EDID_SIZE);
  memset(&mhl->edid_block3[0], 0x00, EDID_SIZE);

  if(result){
	  printk(KERN_INFO "sii8332: %s():%d EDID READ!!!\n", __func__, __LINE__);
    ret = read_edid_block0(mhl);

    if(ret<0){
	  printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
      mutex_unlock(&mhl->i2c_lock);
     return ret;
    }

    if(!edid_header_check(mhl)){
        mhl->hdmi_sink = false;
      goto ddc_release;
    }

    if(!edid_checksum(mhl, 0)){
        mhl->hdmi_sink = false;
        goto ddc_release;
    }

    block_cnt = mhl->edid_block0[0x7E];
    printk(KERN_INFO "%s():%d  Block CNT:%d \n", __func__, __LINE__, block_cnt);

    if(!block_cnt){
      mhl->hdmi_sink = false;
      //mutex_unlock(&mhl->i2c_lock);
      goto ddc_release;
    }

    switch(block_cnt)
    {
      case 1:
          ret = read_edid_block1(mhl);
          if(ret<0){
		  printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
            goto ddc_release;
          }
        break;

      case 2:
          ret = read_edid_block1(mhl);
          if(ret<0){
		  printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
            goto ddc_release;
          }

          ret = read_edid_block2(mhl);
          if(ret<0){
		  printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
            goto ddc_release;
            }
        break;

      case 3:
          ret = read_edid_block1(mhl);
          if(ret<0){
		  printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
            goto ddc_release;
          }

          ret = read_edid_block2(mhl);
          if(ret<0){
		  printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
            goto ddc_release;
          }
          ret = read_edid_block3(mhl);

          if(ret<0){
		  printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
            goto ddc_release;
          }

        break;

      default:

        break;
    }

    ddc_release:
    ret = release_ddc_access(mhl,sys_ctrl, &result);
          if(ret<0){
		  printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
      mutex_unlock(&mhl->i2c_lock);
      return ret;
    }

    if(!result){
      mhl->hdmi_sink = false;
    }
    else{
      mhl->hdmi_sink = true;
    }

  }
  else{
    mhl->hdmi_sink = false;
  }
  mutex_unlock(&mhl->i2c_lock);

  return ret;

          }

static int color_format_set(struct mhl_tx *mhl)
          {
  int ret = 0;


  if(mhl->mipi_input_colorspace == INPUT_COLOR_SPACE_RGB){
    if(mhl->hdmi_sink){
		printk(KERN_INFO "sii8332: %s():%d  BIT_TPI_OUTPUT_FORMAT_HDMI_TO_RGB \n", __func__, __LINE__);
    mhl->mhl_output_colorspace = BIT_TPI_OUTPUT_FORMAT_HDMI_TO_RGB;
    }
    else{
		printk(KERN_INFO "sii8332: %s():%d  BIT_TPI_OUTPUT_FORMAT_HDMI_TO_RGB \n", __func__, __LINE__);
    mhl->mhl_output_colorspace = BIT_TPI_OUTPUT_FORMAT_DVI_TO_RGB;
    }
  }
  else if(mhl->mipi_input_colorspace == INPUT_COLOR_SPACE_YCBCR444){
		printk(KERN_INFO "sii8332: %s():%d  INPUT_COLOR_SPACE_YCBCR444 \n", __func__, __LINE__);
    mhl->mhl_output_colorspace = BIT_TPI_OUTPUT_FORMAT_YCbCr444;
  }
  else if(mhl->mipi_input_colorspace == INPUT_COLOR_SPACE_YCBCR422){
		printk(KERN_INFO "sii8332: %s():%d  INPUT_COLOR_SPACE_YCBCR422 \n", __func__, __LINE__);
    mhl->mhl_output_colorspace = BIT_TPI_OUTPUT_FORMAT_YCbCr422;
  }

  mutex_lock(&mhl->i2c_lock);
  ret = I2CReadModify(mhl->pdata,PAGE_0, 0x0A, BIT_TPI_OUTPUT_FORMAT_MASK, mhl->mhl_output_colorspace);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
    mutex_unlock(&mhl->i2c_lock);
		return ret;
  }
  mutex_unlock(&mhl->i2c_lock);

	return ret;
}


static int set_ouptput_timings(struct mhl_tx *mhl, bool *input_check)
            {
  int ret = 0;
  u8 rd_data = 0, i = 0, cnt = 0, tmp=0;;
  u16 h_res = 0, v_res = 0;
  video_mode_reg video_mode_set[20];
  bool outflag = 0;

  *input_check = false;

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_3, 0xD5, 0x36);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
            }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2CReadModify(mhl->pdata, PAGE_3, 0x8E, BIT_MIP_DDR_OVER_2, 0);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
    goto exit_func;

          }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_0, 0xBC, 0x01);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
          }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_0, 0xBD, 0x09);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
          }
  mutex_unlock(&mhl->i2c_lock);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_ReadByte(mhl->pdata,PAGE_0, 0xBE, &rd_data);
          if(ret<0){
		  printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }
  mutex_unlock(&mhl->i2c_lock);

  if(rd_data&0x01){
   printk(KERN_INFO "sii8332: %s():%d  Clock stable \n", __func__, __LINE__);
  }
  else{
   printk(KERN_INFO "sii8332: %s():%d  Clock unstable \n", __func__, __LINE__);
            //return ret;
          }

  mutex_lock(&mhl->i2c_lock);
  do{

    ret = I2CReadModify(mhl->pdata,PAGE_3, 0x96, BIT1, BIT1);
    if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
            }

    ret = I2CReadModify(mhl->pdata,PAGE_3, 0x93, MIPI_DSI_FORMAT_RESOLUTION_CLEAR, MIPI_DSI_FORMAT_RESOLUTION_CLEAR);
    if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
          }

    msleep(60);


    ret = I2CReadModify(mhl->pdata,PAGE_3, 0x96, BIT1, 0x00);
          if(ret<0){
		  printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
          }

    ret = I2CReadModify(mhl->pdata,PAGE_3, 0x93, MIPI_DSI_FORMAT_RESOLUTION_CLEAR, 0);
    if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
            }

    msleep(100);

    ret = I2C_ReadByte(mhl->pdata,PAGE_3, 0xC3, &tmp);
    if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
          }
    h_res = tmp;
		printk(KERN_INFO "sii8332: %s():%d  h_res1: %02x\n", __func__, __LINE__,h_res );
    h_res <<= 8;

    ret = I2C_ReadByte(mhl->pdata,PAGE_3, 0xC2, &tmp);
    if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
          }
    h_res |= tmp;
		printk(KERN_INFO "sii8332: %s():%d  h_res2: %02x\n", __func__, __LINE__,h_res );

    ret = I2C_ReadByte(mhl->pdata,PAGE_3, 0xC5, &tmp);
    if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
          }
    v_res = tmp;
		printk(KERN_INFO "sii8332: %s():%d  v_res1: %02x\n", __func__, __LINE__,v_res );
    v_res <<= 8;

    ret = I2C_ReadByte(mhl->pdata,PAGE_3, 0xC4, &tmp);
          if(ret<0){
		  printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
          }
    v_res |= tmp;
		printk(KERN_INFO "sii8332: %s():%d  v_res2: %02x\n", __func__, __LINE__,v_res );

		printk(KERN_INFO "sii8332: %s():%d  MIPI Input mode: %d * %d \n", __func__, __LINE__,h_res, v_res );
		printk(KERN_INFO "sii8332: %s():%d  NUM_OF_VIDEO_MODE: %d \n", __func__, __LINE__,NUM_OF_VIDEO_MODE );

    for(i = 0; i<NUM_OF_VIDEO_MODE; ++ i)
          {
      if(h_res == VideoModeInfo[i].header.hRes){
        if(v_res ==  VideoModeInfo[i].header.vRes){
          mhl->colorimetryAspectRatio = VideoModeInfo[i].header.colorimetryAspectRatio;
          mhl->inputVideoCode = VideoModeInfo[i].header.inputVideoCode;
          mhl->interlaced = VideoModeInfo[i].header.interlaced;

          memset(video_mode_set, 0x00, sizeof(video_mode_reg)*20);
          memcpy(video_mode_set, VideoModeInfo[i].regVals, sizeof(video_mode_reg)*20);
            {
            u8 j;
            for(j=0; j<19; j++){
			printk(KERN_INFO "sii8332: %s():%d  video mode set [j]:%d \n", __func__, __LINE__, j );
			printk(KERN_INFO "sii8332: %s():%d  ID:%02x, offset:%02x, value:%02x \n", __func__, __LINE__, video_mode_set[j].device_id, video_mode_set[j].offset, video_mode_set[j].value );
              ret = I2C_WriteByte(mhl->pdata,video_mode_set[j].device_id, video_mode_set[j].offset, video_mode_set[j].value);
              if(ret<0){
			printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
			goto exit_func;
            }
          }
          }

          ret = I2C_WriteByte(mhl->pdata,PAGE_3, 0xA5, 0x1C);
          if(ret<0){
			printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
			goto exit_func;
          }

          ret = I2C_WriteByte(mhl->pdata,PAGE_3, 0xA5, 0x26);
          if(ret<0){
			printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
			goto exit_func;
          }

          ret = I2C_WriteByte(mhl->pdata,PAGE_3, 0xA5, 0x32);
          if(ret<0){
			printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
			goto exit_func;
          }

          ret = I2CReadModify(mhl->pdata,PAGE_3, 0x8E, 0x87, 0x03);
          if(ret<0){
		  printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
			goto exit_func;
          }

        outflag = true;
            }


          }

    }

    cnt++;

    if((!outflag)&&(cnt== 10)){
      printk(KERN_INFO "[ERROR]sii8332: %s():%d cannot find resolution !!!\n", __func__, __LINE__);
      cnt = 0;
      mutex_unlock(&mhl->i2c_lock);
      goto exit_func;
    }

  }while(!outflag);

  ret = I2C_WriteByte(mhl->pdata,PAGE_3, 0xD5, 0x37|mhl->interlaced);
    if(ret<0){
    printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;
  }

  mutex_unlock(&mhl->i2c_lock);

  *input_check = true;

  exit_func:
  if(ret<0){
      mutex_unlock(&mhl->i2c_lock);
  }

      return ret;

    }

static void check_edid_data(struct mhl_tx *mhl)
{
  bool result;

  if(!mhl->hdmi_sink){
    return;
  }

  result = parse861shortdescriptors(mhl);
  if(!result){
	printk(KERN_INFO "sii8332: %s():%d NOT HDMI SINK \n", __func__, __LINE__);
    mhl->hdmi_sink = false;
    return;
  }

  #if 0
  result = parse861longdescriptors(mhl);
    if(!result){
	printk(KERN_INFO "sii8332: %s():%d NOT HDMI SINK \n", __func__, __LINE__);
      mhl->hdmi_sink = false;
    return;
  }
  #endif

  mhl->hdmi_sink = true;
	printk(KERN_INFO "sii8332: %s():%d HDMI SINK \n", __func__, __LINE__);

}

static u8 calculate_avi_info_checksum(u8 *data)
{
  u8 checksum = 0;
  u8 i;
  u8 infoframedata[14];

  checksum = 0x82 + 0x02 + 0x0D;  // these are set by the hardware

  memset(infoframedata, 0x00, 14);
  memcpy(infoframedata, data, 14);

  for (i = 1; i < 14; i++)
  {
    checksum += data[i];
  }

  checksum = 0x100 - checksum;
}


static void check_AVI_info_checksum(u8 *data)
{
  u8 checksum =0;
  u8 i =0;
  u8 infoframedata[14];

  memset(infoframedata, 0x00, 14);
  memcpy(infoframedata, data, 14);

  checksum = 0x82 + 0x02 + 0x0D;

  for(i=0; i< 14; i++)
  {
    printk(KERN_INFO "[ERROR]sii8332: %s():%d Data[%02x]\n", __func__, __LINE__, infoframedata[i]);
    checksum += infoframedata[i];
  }
  printk(KERN_INFO "[ERROR]sii8332: %s():%d checksum1[%02x]\n", __func__, __LINE__, checksum);
  checksum = 0x100 - checksum;
  printk(KERN_INFO "[ERROR]sii8332: %s():%d checksum2[%02x]\n", __func__, __LINE__, checksum);
  if(checksum){
   printk(KERN_INFO "[ERROR]sii8332: %s():%d checksum failed !\n", __func__, __LINE__);
    }

  }

static int set_avi_infoframe(struct mhl_tx *mhl)
{
  int ret =0, checksum=0;
  u8 data[14], i=0;

  memset(data, 0x00, 14);

  if(mhl->mhl_output_colorspace == BIT_TPI_OUTPUT_FORMAT_YCbCr444){
		printk(KERN_INFO "sii8332: %s():%d  BIT_TPI_OUTPUT_FORMAT_YCbCr444 \n", __func__, __LINE__);
   data[1] = 0x40;
  }
  else if(mhl->mhl_output_colorspace == BIT_TPI_OUTPUT_FORMAT_YCbCr422){
		printk(KERN_INFO "sii8332: %s():%d  BIT_TPI_OUTPUT_FORMAT_YCbCr422 \n", __func__, __LINE__);
   data[1] = 0x20;
  }
  else{
		printk(KERN_INFO "sii8332: %s():%d  BIT_TPI_OUTPUT_FORMAT_RGB\n", __func__, __LINE__);
   data[1] = 0x00;
  }

  data[2] = mhl->colorimetryAspectRatio;
  data[3] = 0x00;
  data[4] = mhl->inputVideoCode;
  data[5] = 0x00;
  data[6] = 0x00;
  data[7] = 0x00;
  data[8] = 0x00;
  data[9] = 0x00;
  data[10] = 0x00;
  data[11] = 0x00;
  data[12] = 0x00;
  data[13] = 0x00;

  checksum = 0x82 + 0x02 + 0x0D;  // these are set by the hardware

  for (i = 1; i < 14; i++)
  {
    checksum += data[i];
  }

  checksum = 0x100 - checksum;
  data[0] = checksum;

  //data[0] = calculate_avi_info_checksum(data);

  check_AVI_info_checksum(data);

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte(mhl->pdata,PAGE_0, 0xBF, BIT_TPI_INFO_EN|BIT_TPI_INFO_RPT|BIT_TPI_INFO_READ_FLAG_NO_READ|BIT_TPI_INFO_SEL_AVI);
  if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
	  mutex_unlock(&mhl->i2c_lock);
    return ret;
  }
  mutex_unlock(&mhl->i2c_lock);

  msleep(1); //workaround for the late response of AVI info setting.

  mutex_lock(&mhl->i2c_lock);
  ret = I2C_WriteByte_block(mhl->pdata,PAGE_0,0x0C,14, &data[0]);
  if(ret<0){
    printk(KERN_INFO "[ERROR]sii8240: %s():%d failed !\n", __func__, __LINE__);
	  mutex_unlock(&mhl->i2c_lock);
  return ret;
}
  mutex_unlock(&mhl->i2c_lock);

  return ret;

}

static void avi_control_thread(struct work_struct *work)
{
  struct mhl_tx *mhl = container_of(work,struct mhl_tx,avi_control_work);
	int ret = 0;
  u8 rd_data = 0, tmp = 0;
  bool input_check = 0;

	if(mhl->pdata->status.op_status == NO_MHL_STATUS){
		mhl->pdata->status.linkmode = 0x03;
    mhl->hdmi_sink = false;

    ret = mhl_init_func(mhl);
		if(ret<0){
			printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
      return;
		}

		mhl->pdata->status.op_status = MHL_READY_RGND_DETECT;

    ret = TX_GO2D3(mhl);
		if(ret<0){
			printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
      return ;
		}
  }
  else{
		if(mhl->avi_work == true){
			switch(mhl->avi_cmd)
			{
				case HPD_HIGH_EVENT:

          printk(KERN_INFO "sii8332: %s():%d  TMDS_OUTPUT_CONTROL_ACTIVE, MUTE, DVI mode  \n", __func__, __LINE__ );

          mutex_lock(&mhl->i2c_lock);
          ret = I2C_WriteByte(mhl->pdata,PAGE_3, 0x31, 0xFC);
          if(ret<0){
			printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
			goto exit_func;
          }
          mutex_unlock(&mhl->i2c_lock);

					ret = edid_read(mhl);
					if(ret<0){
						goto exit_func;
					}
					mhl->edid_access_done = true;

          check_edid_data(mhl);

          if(mhl->hdmi_sink){
			  mutex_lock(&mhl->i2c_lock);
            ret = I2CReadModify(mhl->pdata,PAGE_0,0x1A,TMDS_OUTPUT_CONTROL_MASK | AV_MUTE_MASK | OUTPUT_MODE_MASK,
              TMDS_OUTPUT_CONTROL_ACTIVE | AV_MUTE_MUTED|OUTPUT_MODE_HDMI);
            if(ret<0){
			printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
              goto exit_func;
            }
            mutex_unlock(&mhl->i2c_lock);
          }
          else{
			  mutex_lock(&mhl->i2c_lock);
            ret = I2CReadModify(mhl->pdata,PAGE_0,0x1A,TMDS_OUTPUT_CONTROL_MASK | AV_MUTE_MASK | OUTPUT_MODE_MASK,
              TMDS_OUTPUT_CONTROL_ACTIVE | AV_MUTE_MUTED|OUTPUT_MODE_DVI);
            if(ret<0){
			printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
              goto exit_func;
            }
            mutex_unlock(&mhl->i2c_lock);
          }

          mutex_lock(&mhl->i2c_lock);
          mhl->intr4_mask =( BIT_INTR4_CBUS_LKOUT | BIT_INTR4_CBUS_DISCONNECT|BIT_INTR4_SCDT_CHANGE);
          ret = I2C_WriteByte(mhl->pdata,PAGE_3, 0x22, mhl->intr4_mask);
          if(ret<0){
			printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
			goto exit_func;
          }
          mutex_unlock(&mhl->i2c_lock);

          #if 0
          mutex_lock(&mhl->i2c_lock);
          mhl->intr5_mask = (BIT_INTR5_PXL_FORMAT_CHG|BIT_INTR5_MHL_FIFO_OVERFLOW);
          ret = I2C_WriteByte(mhl->pdata,PAGE_3, 0x24, mhl->intr5_mask);
          if(ret<0){
			printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
			goto exit_func;
          }
          mutex_unlock(&mhl->i2c_lock);
          #endif

					break;

        case HPD_LOW_EVENT:

          mutex_lock(&mhl->i2c_lock);
          ret = I2CReadModify(mhl->pdata,PAGE_0,0x1A,  TMDS_OUTPUT_CONTROL_MASK|OUTPUT_MODE_MASK,
            TMDS_OUTPUT_CONTROL_POWER_DOWN    | OUTPUT_MODE_DVI);
          if(ret<0){
			printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
            goto exit_func;
			}
          mutex_unlock(&mhl->i2c_lock);

          mutex_lock(&mhl->i2c_lock);
          ret = I2C_WriteByte(mhl->pdata,PAGE_3, 0x31, 0xC0);
          if(ret<0){
			printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
			goto exit_func;
		}
          mutex_unlock(&mhl->i2c_lock);

          mutex_lock(&mhl->i2c_lock);
          mhl->intr4_mask =( BIT_INTR4_CBUS_LKOUT | BIT_INTR4_CBUS_DISCONNECT);
          ret = I2C_WriteByte(mhl->pdata,PAGE_3, 0x22, ( BIT_INTR4_CBUS_LKOUT | BIT_INTR4_CBUS_DISCONNECT));
          if(ret<0){
			printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
			goto exit_func;
	  }
          mutex_unlock(&mhl->i2c_lock);

          mutex_lock(&mhl->i2c_lock);
          mhl->intr5_mask = 0x00;
          ret = I2C_WriteByte(mhl->pdata,PAGE_3, 0x24, mhl->intr5_mask);
	if(ret<0){
			printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
			goto exit_func;
	}
          mutex_unlock(&mhl->i2c_lock);

          break;

        case MIPI_INPUT_EVENT:

          mutex_lock(&mhl->i2c_lock);
          ret = I2C_ReadByte(mhl->pdata,PAGE_3, 0x92, &rd_data);
          if(ret<0){
			printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
			goto exit_func;
}
          mutex_unlock(&mhl->i2c_lock);

         if(rd_data & BIT_MIPI_R_DSI_PXL_FORMAT_CHANGED){
			printk(KERN_INFO "sii8332: %s():%d  MIPI -> DSI pixel format changed \n", __func__, __LINE__);

            mutex_lock(&mhl->i2c_lock);
            ret = I2C_WriteByte(mhl->pdata,PAGE_3, 0x92, rd_data|BIT_MIPI_W_DSI_PXL_FORMAT_CHANGED_IN);
            if(ret<0){
			printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
			goto exit_func;
            }
            mutex_unlock(&mhl->i2c_lock);

            mutex_lock(&mhl->i2c_lock);
            ret = I2C_WriteByte(mhl->pdata,PAGE_3, 0x92, rd_data);
            if(ret<0){
			printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
			goto exit_func;
            }
            mutex_unlock(&mhl->i2c_lock);

            mutex_lock(&mhl->i2c_lock);
            ret = I2C_ReadByte(mhl->pdata,PAGE_3, 0x94, &tmp);
            if(ret<0){
			printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
			goto exit_func;
            }
            mutex_unlock(&mhl->i2c_lock);

            tmp &= 0x3F;

            printk(KERN_INFO "sii8332: %s():%d  MIPI -> Format %02x  \n", __func__, __LINE__, tmp);

            switch(tmp)
            {
              case 0x0C:
                printk(KERN_INFO "sii8332: %s():%d  LPPS 20-bit YCbCr 4:2:2 \n", __func__, __LINE__);
                mhl->mipi_input_colorspace = INPUT_COLOR_SPACE_YCBCR422;
                break;

              case 0x1C:
                printk(KERN_INFO "sii8332: %s():%d  PPS 24-bit YCbCr 4:2:2 \n", __func__, __LINE__);
                mhl->mipi_input_colorspace = INPUT_COLOR_SPACE_YCBCR422;
                break;

              case 0x2C:
                printk(KERN_INFO "sii8332: %s():%d  PPS 16-bit YCbCr 4:2:2 \n", __func__, __LINE__);
                mhl->mipi_input_colorspace = INPUT_COLOR_SPACE_YCBCR422;
                break;

              case 0x0E:
                printk(KERN_INFO "sii8332: %s():%d  PPS 16-bit RGB 565 \n", __func__, __LINE__);
                mhl->mipi_input_colorspace = INPUT_COLOR_SPACE_RGB;
                break;

              case 0x1E:
                printk(KERN_INFO "sii8332: %s():%d PPS 18-bit RGB 666 \n", __func__, __LINE__);
                mhl->mipi_input_colorspace = INPUT_COLOR_SPACE_RGB;
                break;

              case 0x2E:
                printk(KERN_INFO "sii8332: %s():%d LPPS 18-bit RGB 666\n", __func__, __LINE__);
                mhl->mipi_input_colorspace = INPUT_COLOR_SPACE_RGB;
                break;

              case 0x3E:
                printk(KERN_INFO "sii8332: %s():%d PPS 24-bit RGB 888\n", __func__, __LINE__);
                mhl->mipi_input_colorspace = INPUT_COLOR_SPACE_RGB;
                break;

              default:
                printk(KERN_INFO "sii8332: %s():%d Unknown...  \n", __func__, __LINE__);
                mhl->mipi_input_colorspace = INPUT_COLOR_SPACE_RGB;
                break;
            }

            mutex_lock(&mhl->i2c_lock);
            ret = I2C_WriteByte(mhl->pdata,PAGE_0, 0x09, mhl->mipi_input_colorspace);
            if(ret<0){
			printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
			goto exit_func;
            }
            mutex_unlock(&mhl->i2c_lock);

            ret = set_ouptput_timings(mhl, &input_check);
            if(ret<0){
			printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
			return ret;
            }
			printk(KERN_INFO "sii8332: %s():%d !\n", __func__, __LINE__);
            ret = color_format_set(mhl);
            if(ret<0){
			printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
			return ret;
            }
			printk(KERN_INFO "sii8332: %s():%d !\n", __func__, __LINE__);

           if(mhl->hdmi_sink){
			printk(KERN_INFO "sii8332: %s():%d !\n", __func__, __LINE__);
            #if 0
            ret = set_audio_infoframe(mhl);
            if(ret<0){
            printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
            return ret;
            }
            #endif

            ret = set_avi_infoframe(mhl);
            if(ret<0){
            printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
            return ret;
            }

            printk(KERN_INFO "sii8332: %s():%d  TMDS_OUTPUT_CONTROL_ACTIVE, AV_MUTE_NORMAL, OUTPUT_MODE_HDMI  \n", __func__, __LINE__ );
			  mutex_lock(&mhl->i2c_lock);
            ret = I2CReadModify(mhl->pdata,PAGE_0,0x1A,TMDS_OUTPUT_CONTROL_MASK | AV_MUTE_MASK | OUTPUT_MODE_MASK,
              TMDS_OUTPUT_CONTROL_ACTIVE | AV_MUTE_NORMAL|OUTPUT_MODE_HDMI);
            if(ret<0){
			printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
              goto exit_func;
            }
            mutex_unlock(&mhl->i2c_lock);
           }
           else{
			  mutex_lock(&mhl->i2c_lock);
            printk(KERN_INFO "sii8332: %s():%d  TMDS_OUTPUT_CONTROL_ACTIVE, AV_MUTE_NORMAL, OUTPUT_MODE_DVI  \n", __func__, __LINE__ );
            ret = I2CReadModify(mhl->pdata,PAGE_0,0x1A,TMDS_OUTPUT_CONTROL_MASK | AV_MUTE_MASK | OUTPUT_MODE_MASK,
              TMDS_OUTPUT_CONTROL_ACTIVE | AV_MUTE_NORMAL|OUTPUT_MODE_DVI);
            if(ret<0){
			printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
              goto exit_func;
            }
            mutex_unlock(&mhl->i2c_lock);
           }

          }
          else{
			printk(KERN_INFO "sii8332: %s():%d  MIPI -> DSI pixel format NOT changed \n", __func__, __LINE__);

            mutex_lock(&mhl->i2c_lock);
            ret = I2C_WriteByte(mhl->pdata,PAGE_3, 0x92, rd_data|BIT_MIPI_W_DSI_PXL_FORMAT_CHANGED_IN);
            if(ret<0){
			printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
			goto exit_func;
            }
            mutex_unlock(&mhl->i2c_lock);

            mutex_lock(&mhl->i2c_lock);
            ret = I2C_WriteByte(mhl->pdata,PAGE_3, 0x92, rd_data);
            if(ret<0){
			printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
			goto exit_func;
            }
            mutex_unlock(&mhl->i2c_lock);

            mutex_lock(&mhl->i2c_lock);
            ret = I2C_ReadByte(mhl->pdata,PAGE_3, 0x94, &tmp);
            if(ret<0){
			printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
			goto exit_func;
            }
            mutex_unlock(&mhl->i2c_lock);

            tmp &= 0x3F;

            printk(KERN_INFO "sii8332: %s():%d  MIPI -> Format %02x  \n", __func__, __LINE__, tmp);

            switch(tmp)
            {
              case 0x0C:
                printk(KERN_INFO "sii8332: %s():%d  LPPS 20-bit YCbCr 4:2:2 \n", __func__, __LINE__);
                mhl->mipi_input_colorspace = INPUT_COLOR_SPACE_YCBCR422;
                break;

              case 0x1C:
                printk(KERN_INFO "sii8332: %s():%d  PPS 24-bit YCbCr 4:2:2 \n", __func__, __LINE__);
                mhl->mipi_input_colorspace = INPUT_COLOR_SPACE_YCBCR422;
                break;

              case 0x2C:
                printk(KERN_INFO "sii8332: %s():%d  PPS 16-bit YCbCr 4:2:2 \n", __func__, __LINE__);
                mhl->mipi_input_colorspace = INPUT_COLOR_SPACE_YCBCR422;
                break;

              case 0x0E:
                printk(KERN_INFO "sii8332: %s():%d  PPS 16-bit RGB 565 \n", __func__, __LINE__);
                mhl->mipi_input_colorspace = INPUT_COLOR_SPACE_RGB;
                break;

              case 0x1E:
                printk(KERN_INFO "sii8332: %s():%d PPS 18-bit RGB 666 \n", __func__, __LINE__);
                mhl->mipi_input_colorspace = INPUT_COLOR_SPACE_RGB;
                break;

              case 0x2E:
                printk(KERN_INFO "sii8332: %s():%d LPPS 18-bit RGB 666\n", __func__, __LINE__);
                mhl->mipi_input_colorspace = INPUT_COLOR_SPACE_RGB;
                break;

              case 0x3E:
                printk(KERN_INFO "sii8332: %s():%d PPS 24-bit RGB 888\n", __func__, __LINE__);
                mhl->mipi_input_colorspace = INPUT_COLOR_SPACE_RGB;
                break;

              default:
                printk(KERN_INFO "sii8332: %s():%d Unknown...  \n", __func__, __LINE__);
                mhl->mipi_input_colorspace = INPUT_COLOR_SPACE_RGB;
                break;
            }

            mutex_lock(&mhl->i2c_lock);
            ret = I2C_WriteByte(mhl->pdata,PAGE_0, 0x09, mhl->mipi_input_colorspace);
            if(ret<0){
			printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
			goto exit_func;
            }
            mutex_unlock(&mhl->i2c_lock);

            ret = set_ouptput_timings(mhl, &input_check);
            if(ret<0){
			printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
			return ret;
            }
			printk(KERN_INFO "sii8332: %s():%d !\n", __func__, __LINE__);
            ret = color_format_set(mhl);
            if(ret<0){
			printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
			return ret;
            }
			printk(KERN_INFO "sii8332: %s():%d !\n", __func__, __LINE__);

           if(mhl->hdmi_sink){
			printk(KERN_INFO "sii8332: %s():%d !\n", __func__, __LINE__);
            #if 0
            ret = set_audio_infoframe(mhl);
            if(ret<0){
            printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
            return ret;
            }
            #endif

            ret = set_avi_infoframe(mhl);
            if(ret<0){
            printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
            return ret;
            }

            printk(KERN_INFO "sii8332: %s():%d  TMDS_OUTPUT_CONTROL_ACTIVE, AV_MUTE_NORMAL, OUTPUT_MODE_HDMI  \n", __func__, __LINE__ );
			  mutex_lock(&mhl->i2c_lock);
            ret = I2CReadModify(mhl->pdata,PAGE_0,0x1A,TMDS_OUTPUT_CONTROL_MASK | AV_MUTE_MASK | OUTPUT_MODE_MASK,
              TMDS_OUTPUT_CONTROL_ACTIVE | AV_MUTE_NORMAL|OUTPUT_MODE_HDMI);
            if(ret<0){
			printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
              goto exit_func;
            }
            mutex_unlock(&mhl->i2c_lock);
           }
           else{
			  mutex_lock(&mhl->i2c_lock);
            printk(KERN_INFO "sii8332: %s():%d  TMDS_OUTPUT_CONTROL_ACTIVE, AV_MUTE_NORMAL, OUTPUT_MODE_DVI  \n", __func__, __LINE__ );
            ret = I2CReadModify(mhl->pdata,PAGE_0,0x1A,TMDS_OUTPUT_CONTROL_MASK | AV_MUTE_MASK | OUTPUT_MODE_MASK,
              TMDS_OUTPUT_CONTROL_ACTIVE | AV_MUTE_NORMAL|OUTPUT_MODE_DVI);
            if(ret<0){
			printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
              goto exit_func;
            }
            mutex_unlock(&mhl->i2c_lock);
           }

          }

          break;

        default:

          break;
      }

      mhl->avi_work = false;

    }
    else{

    }
  }

  exit_func:
    if(ret<0){
		  printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
      mutex_unlock(&mhl->i2c_lock);
    }
    return ;

}


static void MipiIsr(struct mhl_tx *mhl)
{
	u8 th_intr=0;
	u8 temp;
	u16 hRes;
	u16 vRes;
	u16 read_reg;


	//TMDS output enable
	I2CReadModify(mhl->pdata,PAGE_0,0x1A,TMDS_OUTPUT_CONTROL_MASK | AV_MUTE_MASK | OUTPUT_MODE_MASK,
		TMDS_OUTPUT_CONTROL_ACTIVE | AV_MUTE_NORMAL|OUTPUT_MODE_HDMI);

	//read MIPI/DSI contol register
	I2C_ReadByte(mhl->pdata, PAGE_3, 0x92, &th_intr);

	//pixel format changed
	I2C_WriteByte(mhl->pdata, PAGE_3, 0x92, th_intr | 0x40);
	I2C_WriteByte(mhl->pdata, PAGE_3, 0x92, th_intr);

	//read pixel format
	I2C_ReadByte(mhl->pdata, PAGE_3, 0x94, &temp);
	temp &= 0x3f;

	printk("MIPI -> Format 0x%02X - \n", (int)temp);

	switch (temp)
	{
		case 0x0C:
			printk("LPPS 20-bit YCbCr 4:2:2\n");
			break;
		case 0x1C:
			printk("PPS 24-bit YCbCr 4:2:2\n");
			break;
		case 0x2C:
			printk("PPS 16-bit YCbCr 4:2:2\n");
			break;
		case 0x0E:
			printk("PPS 16-bit RGB 565\n");
			break;
		case 0x1E:
			printk("PPS 18-bit RGB 666\n");
			break;
		case 0x2E:
			printk("LPPS 18-bit RGB 666\n");
			break;
		case 0x3E:
			printk("PPS 24-bit RGB 888\n");
			break;
		default:
			printk("Unknown\n");
			break;
	}

	//override output timings

	I2C_WriteByte(mhl->pdata, PAGE_3, 0xd5, 0x36);

	// clear DDR/2 mode for measurement purposes (we'll set it later if necessary)
	I2CReadModify(mhl->pdata, PAGE_3, 0x8e, 0x80, 0);

	// write 1 to clear display dimension info, then set back to zero
	I2CReadModify(mhl->pdata, PAGE_3, 0x96, 0x2, 0x2);
	I2CReadModify(mhl->pdata, PAGE_3, 0x93, 0x40, 0x40);
	udelay(50);
	I2CReadModify(mhl->pdata, PAGE_3, 0x96, 0x2, 0);
	I2CReadModify(mhl->pdata, PAGE_3, 0x93, 0x40, 0);
	udelay(100);

	I2C_ReadByte(mhl->pdata, PAGE_3, 0xc3, &temp);
	hRes = temp << 8;
	I2C_ReadByte(mhl->pdata, PAGE_3, 0xc2, &temp);
	hRes |= temp;

	I2C_ReadByte(mhl->pdata, PAGE_3, 0xc5, &temp);
	vRes = temp << 8;
	I2C_ReadByte(mhl->pdata, PAGE_3, 0xc4, &temp);
	vRes |= temp;

	printk("MIPI Input Mode: %dx%d\n", (int) hRes, (int) vRes);
}


static int discovery_success(struct mhl_tx *mhl)
{
	int ret;
	u8 intr_status=0;

	DEBUG_MHL_FUNC_START();

	MipiIsr(mhl);


	mutex_lock(&mhl->i2c_lock);
	ret = I2C_ReadByte(mhl->pdata, PAGE_3, 0x21, &intr_status);
	if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		goto exit_func;

	}
	mutex_unlock(&mhl->i2c_lock);

	if(intr_status){
		mutex_lock(&mhl->i2c_lock);
		ret = I2C_WriteByte(mhl->pdata, PAGE_3, 0x21, intr_status);
		if(ret<0){
			printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
			goto exit_func;
		}
		mutex_unlock(&mhl->i2c_lock);

		if((mhl->intr4_mask&BIT_INTR4_SCDT_CHANGE)&&(intr_status & BIT_INTR4_SCDT_CHANGE)){
			printk(KERN_INFO "sii8332: %s():%d  BIT_INTR4_SCDT_CHANGE \n", __func__, __LINE__);

			mhl->avi_work = true;
			mhl->avi_cmd = MIPI_INPUT_EVENT;
			if (waitqueue_active(&mhl->avi_control_wq)){
			printk(KERN_INFO "sii8332: %s():%d  wake_up \n", __func__, __LINE__);
			wake_up(&mhl->avi_control_wq);
			}else{
			printk(KERN_INFO "sii8332: %s():%d  queue_work \n", __func__, __LINE__);
			queue_work(mhl->avi_control_wqs, &mhl->avi_control_work);
			}
		}

		if(intr_status & BIT_INTR4_CBUS_DISCONNECT){
			printk(KERN_INFO "sii8332: %s():%d  BIT_INTR4_CBUS_DISCONNECT \n", __func__, __LINE__);
			mhl->pdata->status.op_status = MHL_RX_DISCONNECTED;
		}

		if(intr_status & BIT4){
			ret = forceUSBIDswitchOpen(mhl);
			if(ret<0){
				printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
				return ret;
			}

			ret = releaseUSBIDswitchOpen(mhl);
			if(ret<0){
			printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
			return ret;
			}
		}
	}

	mutex_lock(&mhl->i2c_lock);
  ret = I2C_ReadByte(mhl->pdata, PAGE_3, 0x23, &intr_status);
	if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
	goto exit_func;

	}
	mutex_unlock(&mhl->i2c_lock);

	printk(KERN_INFO "sii8332: %s():%d  intr_4status: %02x \n", __func__, __LINE__, intr_status);
  if(intr_status){

		mutex_lock(&mhl->i2c_lock);
    ret = I2C_WriteByte(mhl->pdata, PAGE_3, 0x23, intr_status);
		if(ret<0){
				printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		  goto exit_func;

		}
		mutex_unlock(&mhl->i2c_lock);

    if((mhl->intr4_mask&BIT_INTR5_CKDT)&&(intr_status & BIT_INTR5_CKDT)){
		printk(KERN_INFO "sii8332: %s():%d  BIT_INTR5_CKDT \n", __func__, __LINE__);

      mhl->avi_work = true;
      mhl->avi_cmd = MIPI_INPUT_EVENT;
	if (waitqueue_active(&mhl->avi_control_wq)){
		printk(KERN_INFO "sii8332: %s():%d  wake_up \n", __func__, __LINE__);
        wake_up(&mhl->avi_control_wq);
      }
      else{
        printk(KERN_INFO "sii8332: %s():%d  queue_work \n", __func__, __LINE__);
        queue_work(mhl->avi_control_wqs, &mhl->avi_control_work);
      }
		}

    if(intr_status & BIT_INTR5_VBUS_CHG){
		printk(KERN_INFO "sii8332: %s():%d  BIT_INTR5_VBUS_CHG \n", __func__, __LINE__);
		}

    if((mhl->intr4_mask&BIT_INTR5_PXL_FORMAT_CHG)&&(intr_status & BIT_INTR5_PXL_FORMAT_CHG)){
		printk(KERN_INFO "sii8332: %s():%d  BIT_INTR5_PXL_FORMAT_CHG \n", __func__, __LINE__);
      mhl->avi_work = true;
      mhl->avi_cmd = MIPI_INPUT_EVENT;
	if (waitqueue_active(&mhl->avi_control_wq)){
		printk(KERN_INFO "sii8332: %s():%d  wake_up \n", __func__, __LINE__);
        wake_up(&mhl->avi_control_wq);
      }
      else{
		printk(KERN_INFO "sii8332: %s():%d  queue_work \n", __func__, __LINE__);
        queue_work(mhl->avi_control_wqs, &mhl->avi_control_work);
      }
    }

    if((mhl->intr4_mask&BIT_INTR5_MHL_FIFO_OVERFLOW)&&(intr_status & BIT_INTR5_MHL_FIFO_OVERFLOW)){
		printk(KERN_INFO "sii8332: %s():%d  BIT_INTR5_MHL_FIFO_OVERFLOW \n", __func__, __LINE__);
      #if 0
      mhl->avi_work = true;
      mhl->avi_cmd = MIPI_INPUT_EVENT;
	if (waitqueue_active(&mhl->avi_control_wq)){
		printk(KERN_INFO "sii8332: %s():%d  wake_up \n", __func__, __LINE__);
        wake_up(&mhl->avi_control_wq);
      }
      else{
		printk(KERN_INFO "sii8332: %s():%d  queue_work \n", __func__, __LINE__);
        queue_work(mhl->avi_control_wqs, &mhl->avi_control_work);
      }
      #endif
			  }

    if(intr_status & BIT_INTR5_MHL_FIFO_UNDERFLOW){
		printk(KERN_INFO "sii8332: %s():%d  BIT_INTR5_MHL_FIFO_UNDERFLOW \n", __func__, __LINE__);
		}

	}

	ret = mhl_cbus_intr(mhl);
	if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
    return ret;
	}

	mutex_lock(&mhl->i2c_lock);
  ret = I2C_ReadByte(mhl->pdata, PAGE_0, 0x3D, &intr_status);
	if(ret<0){
		printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
	goto exit_func;

	}
	mutex_unlock(&mhl->i2c_lock);

  if(intr_status){

		mutex_lock(&mhl->i2c_lock);
    ret = I2C_WriteByte(mhl->pdata, PAGE_0, 0x3D, intr_status);
		if(ret<0){
			printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
			goto exit_func;

		}
		mutex_unlock(&mhl->i2c_lock);

    if (BIT_TPI_INTR_ST0_HOT_PLUG_EVENT & intr_status){
		//printk(KERN_INFO "sii8332: %s():%d  BIT_TPI_INTR_ST0_HOT_PLUG_EVENT \n", __func__, __LINE__);

		  mutex_lock(&mhl->i2c_lock);
      ret = I2C_ReadByte(mhl->pdata, PAGE_CBUS, 0x0D, &intr_status);
		  if(ret<0){
		    printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
		    goto exit_func;
		  }
		  mutex_unlock(&mhl->i2c_lock);

      if(intr_status & BIT6){
				printk(KERN_INFO "sii8332: %s():%d  HPD HIGH \n", __func__, __LINE__);
		    mhl->avi_work = true;
		    mhl->avi_cmd = HPD_HIGH_EVENT;
			if (waitqueue_active(&mhl->avi_control_wq)){
		      wake_up(&mhl->avi_control_wq);
		    }
		    else{
		      queue_work(mhl->avi_control_wqs, &mhl->avi_control_work);
		    }
		  }
		  else{
				printk(KERN_INFO "sii8332: %s():%d  HPD LOW \n", __func__, __LINE__);
		    mhl->avi_work = true;
		    mhl->avi_cmd = HPD_LOW_EVENT;
			if (waitqueue_active(&mhl->avi_control_wq)){
		      wake_up(&mhl->avi_control_wq);
		    }
		    else{
		      queue_work(mhl->avi_control_wqs, &mhl->avi_control_work);
		    }
		  }

		}
	}

  exit_func:
    if(ret<0){
      mutex_unlock(&mhl->i2c_lock);
    }
    return ret;
}


static irqreturn_t mhl_irq_thread(int irq, void *data)
{
	struct mhl_tx *mhl = data;
	int ret;
	u8 local_op_status =0;

	intr_check:

	DEBUG_MHL_FUNC_START();

	local_op_status = mhl->pdata->status.op_status;

	if(local_op_status == NO_MHL_STATUS){
		printk(KERN_INFO "sii8332: %s():%d NO_MHL_STATUS !\n", __func__, __LINE__);
		return IRQ_HANDLED;
	}

	switch (local_op_status)
	{
	case MHL_READY_RGND_DETECT:
		printk(KERN_INFO "sii8332: %s():%d MHL_READY_RGND_DETECT \n", __func__, __LINE__);

		ret = mhl_rgnd_check_func(mhl);
		if(ret<0){
			printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
			return IRQ_HANDLED;
		}

		if(mhl->pdata->status.op_status == MHL_USB_CONNECTED){
			mhl->pdata->status.linkmode = 0x03;
			mhl->hdmi_sink = false;

			mutex_lock(&mhl->cbus_cmd_lock);
			mhl->cmd_rx_cnt = 0;
			mhl->cmd_tx_cnt = 0;
			mutex_unlock(&mhl->cbus_cmd_lock);

			mhl->pdata->status.op_status = NO_MHL_STATUS;
			ret = mhl_init_func(mhl);
			if(ret<0){
				printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
				return IRQ_HANDLED;
			}

			ret = TX_GO2D3(mhl);
			if(ret<0){
				printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
				return IRQ_HANDLED;
			}

			mhl->pdata->status.op_status = MHL_READY_RGND_DETECT;
		}

		break;

	case MHL_RX_CONNECTED:

		printk(KERN_INFO "sii8332: %s():%d MHL_RX_CONNECTED \n", __func__, __LINE__);

		ret = mhl_rx_connected_func(mhl);

		if(ret<0){
			printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
			return IRQ_HANDLED;
		}

		if(mhl->pdata->status.op_status == MHL_DISCOVERY_FAIL){
			printk(KERN_INFO "sii8332: %s():%d MHL_DISCOVERY_FAIL \n", __func__, __LINE__);

			mhl->pdata->status.linkmode = 0x03;
			mhl->hdmi_sink = false;

			mutex_lock(&mhl->cbus_cmd_lock);
			mhl->cmd_rx_cnt = 0;
			mhl->cmd_tx_cnt = 0;
			mutex_unlock(&mhl->cbus_cmd_lock);


			if (waitqueue_active(&mhl->cbus_hanler_wq)){
			printk(KERN_INFO "sii8332: %s():%d wake_up \n", __func__, __LINE__);
			wake_up(&mhl->cbus_hanler_wq);
			}


			mhl->pdata->status.op_status = NO_MHL_STATUS;

			ret = mhl_init_func(mhl);
			if(ret<0){
				printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
			return IRQ_HANDLED;
			}

			ret = TX_GO2D3(mhl);
			if(ret<0){
				printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
			return IRQ_HANDLED;
			}

			mhl->pdata->status.op_status = MHL_READY_RGND_DETECT;

			return IRQ_HANDLED;
		}else if(mhl->pdata->status.op_status == MHL_DISCOVERY_ON){
			printk(KERN_INFO "sii8332: %s():%d MHL_DISCOVERY_ON \n", __func__, __LINE__);
			if (waitqueue_active(&mhl->cbus_hanler_wq)){
				printk(KERN_INFO "sii8332: %s():%d wake_up \n", __func__, __LINE__);
				wake_up(&mhl->cbus_hanler_wq);
			}
		}
		else if(mhl->pdata->status.op_status == MHL_RX_CONNECTED){
		printk(KERN_INFO "sii8332: %s():%d MHL_RX_CONNECTED \n", __func__, __LINE__);
		}

		break;

	case MHL_DISCOVERY_SUCCESS:
		//printk(KERN_INFO " sii8332: %s():\n", __func__);
		printk(KERN_INFO "sii8332: %s():%d MHL_DISCOVERY_SUCCESS \n", __func__, __LINE__);

		ret = discovery_success(mhl);
		if(ret<0){
			printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
			return IRQ_HANDLED;
		}

		if(mhl->pdata->status.op_status == MHL_RX_DISCONNECTED){
			printk(KERN_INFO "sii8332: %s():%d MHL_DISCOVERY_FAIL \n", __func__, __LINE__);

			mhl->pdata->status.linkmode = 0x03;
			mutex_lock(&mhl->cbus_cmd_lock);
			mhl->cmd_rx_cnt = 0;
			mhl->cmd_tx_cnt = 0;
			mutex_unlock(&mhl->cbus_cmd_lock);

			mhl->pdata->status.op_status = NO_MHL_STATUS;

			if (waitqueue_active(&mhl->cbus_hanler_wq)){
				printk(KERN_INFO "sii8332: %s():%d wake_up \n", __func__, __LINE__);
				wake_up(&mhl->cbus_hanler_wq);
			}

			ret = mhl_init_func(mhl);
			if(ret<0){
				printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
				return IRQ_HANDLED;
			}

			mhl->hdmi_sink = false;

			ret = TX_GO2D3(mhl);
			if(ret<0){
				printk(KERN_INFO "[ERROR]sii8332: %s():%d failed !\n", __func__, __LINE__);
				return IRQ_HANDLED;
			}

			mhl->pdata->status.op_status = MHL_READY_RGND_DETECT;

			return IRQ_HANDLED;
		}

		break;

	default:
		break;
	}

	if( !gpio_get_value(mhl->pdata->mhl_int)){
		printk(KERN_INFO " interrupt not clear???!\n", __func__, __LINE__);
		goto intr_check;
	}

	return IRQ_HANDLED;
}


static int __devinit simg72_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct tx_page0 *page0;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
		return -EIO;

	page0 = kzalloc(sizeof(struct tx_page0), GFP_KERNEL);
	if (!page0) {
		dev_err(&client->dev, " page0 failed to allocate driver data\n");
		return -ENOMEM;
	}

	page0->pdata = client->dev.platform_data;
	page0->pdata->simg72_tx_client = client;
	page0->pdata->irq = gpio_to_irq(page0->pdata->mhl_int);
	printk(KERN_INFO"irq=%d, mhl_int=%d \n", page0->pdata->irq, page0->pdata->mhl_int);

	if(!page0->pdata){
		printk(KERN_INFO"\n SIMG72 no platform data \n");
		kfree(page0);
		return -EINVAL;
	}

	i2c_set_clientdata(client, page0);

	return 0;
}

static int __devinit simg7A_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct tx_page1 *page1;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
		return -EIO;

	page1 = kzalloc(sizeof(struct tx_page1), GFP_KERNEL);
	if (!page1) {
		dev_err(&client->dev, " page1 failed to allocate driver data\n");
		return -ENOMEM;
	}

	page1->pdata = client->dev.platform_data;
	page1->pdata->simg7A_tx_client = client;

	if(!page1->pdata){
		printk(KERN_INFO"\n SIMG7A no platform data \n");
		kfree(page1);
		return -EINVAL;
	}

	i2c_set_clientdata(client, page1);


	return 0;
}

static int __devinit simg92_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
  struct tx_page2 *page2;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
    return -EIO;

	page2 = kzalloc(sizeof(struct tx_page2), GFP_KERNEL);
	if (!page2) {
		dev_err(&client->dev, " page2 failed to allocate driver data\n");
		return -ENOMEM;
	}

  page2->pdata = client->dev.platform_data;
  page2->pdata->simg92_tx_client = client;

  if(!page2->pdata){
    printk(KERN_INFO"\n SIMG92 no platform data \n");
	kfree(page2);
    return -EINVAL;
  }

  i2c_set_clientdata(client, page2);


  return 0;
}

static int __devinit simg9A_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
  struct tx_page3 *page3;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
    return -EIO;

	page3 = kzalloc(sizeof(struct tx_page3), GFP_KERNEL);
	if (!page3) {
		dev_err(&client->dev, " page3 failed to allocate driver data\n");
		return -ENOMEM;
	}

  page3->pdata = client->dev.platform_data;
  page3->pdata->simg9A_tx_client = client;

  if(!page3->pdata){
    printk(KERN_INFO"\n SIMG9A no platform data \n");
	kfree(page3);
    return -EINVAL;
  }

  i2c_set_clientdata(client, page3);


  return 0;
}

static int __devinit simgA0_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
  struct tx_0xA0 *DDC_A0;


	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
    return -EIO;

	DDC_A0 = kzalloc(sizeof(struct tx_0xA0), GFP_KERNEL);
	if (!DDC_A0) {
		dev_err(&client->dev, " simgA0_probe failed to allocate driver data\n");
		return -ENOMEM;
	}

  DDC_A0->pdata = client->dev.platform_data;
  DDC_A0->pdata->simgA0_tx_client = client;

  if(!DDC_A0->pdata){
    printk(KERN_INFO"\n SIMGA0 no platform data \n");
	kfree(DDC_A0);
    return -EINVAL;
  }

  i2c_set_clientdata(client, DDC_A0);


  return 0;
}

static int __devinit simg60_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct tx_0x60 *DDC_60;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
		return -EIO;

	DDC_60 = kzalloc(sizeof(struct tx_0x60), GFP_KERNEL);
	if (!DDC_60) {
		dev_err(&client->dev, " simg60_probe failed to allocate driver data\n");
		return -ENOMEM;
	}

	DDC_60->pdata = client->dev.platform_data;
	DDC_60->pdata->simg60_tx_client = client;

	if(!DDC_60->pdata){
		printk(KERN_INFO"\n SIM60 no platform data \n");
		kfree(DDC_60);
	return -EINVAL;
	}

	i2c_set_clientdata(client, DDC_60);


	return 0;
}

static int __devinit simgC8_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct mhl_tx *mhl;
	int ret =0;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
		return -EIO;

	mhl = kzalloc(sizeof(struct mhl_tx), GFP_KERNEL);
	if (!mhl) {
		dev_err(&client->dev, " cbus failed to allocate driver data\n");
		return -ENOMEM;
	}

	mhl->pdata = client->dev.platform_data;
	mhl->pdata->simgC8_tx_client = client;
	mhl->pdata->status.op_status = NO_MHL_STATUS;
	mhl->edid_access_done = false;

	if(!mhl->pdata){
		printk(KERN_INFO"\n SIMGC8 no platform data \n");
		kfree(mhl);
		return -EINVAL;
	}

	mutex_init(&mhl->i2c_lock);
	mutex_init(&mhl->cbus_cmd_lock);
	mutex_init(&mhl->pdata->mhl_status_lock);

	i2c_set_clientdata(client, mhl);
	init_waitqueue_head(&mhl->cbus_hanler_wq);
	init_waitqueue_head(&mhl->avi_control_wq);

	INIT_WORK(&mhl->cbus_cmd_work, cbus_cmd_thread);
	INIT_WORK(&mhl->avi_control_work, avi_control_thread);


	mhl->cbus_cmd_wqs = create_singlethread_workqueue("cbus_cmd_wqs");
	if (!mhl->cbus_cmd_wqs)
		return -ENOMEM;

	mhl->avi_control_wqs = create_singlethread_workqueue("avi_control_wqs");
	if (!mhl->avi_control_wqs)
		return -ENOMEM;


	ret = request_threaded_irq(mhl->pdata->irq, NULL, mhl_irq_thread,
	IRQF_TRIGGER_FALLING	, "cbus_intr", mhl);

	if(ret<0){
		printk(KERN_INFO "cbus irq regist fail \n");
		kfree(mhl);
		return ret;
	}

	hw_resst(mhl->pdata);
  enable_irq(mhl->pdata->irq);

	queue_work(mhl->avi_control_wqs, &mhl->avi_control_work);

	return 0;
}


static int __devexit simg72_remove(struct i2c_client *client)
{
	i2c_del_driver(&simg72_driver);
	return 0;
}

static int __devexit simg7A_remove(struct i2c_client *client)
{
	i2c_del_driver(&simg7A_driver);
	return 0;
}

static int __devexit simg92_remove(struct i2c_client *client)
{
	i2c_del_driver(&simg92_driver);
	return 0;
}

static int __devexit simg9A_remove(struct i2c_client *client)
{
	i2c_del_driver(&simg9A_driver);
	return 0;
}

static int __devexit simgC8_remove(struct i2c_client *client)
{
	i2c_del_driver(&simgC8_driver);
	return 0;
}

static int __devexit simgA0_remove(struct i2c_client *client)
{
	i2c_del_driver(&simgA0_driver);
	return 0;
}

static int __devexit simg60_remove(struct i2c_client *client)
{
	i2c_del_driver(&simg60_driver);
	return 0;
}


static int __init simg_init(void)
{
	int ret=0;


	ret = i2c_add_driver(&simg72_driver);
	if (ret !=0)
		goto err_exit1;

	ret = i2c_add_driver(&simg7A_driver);
	if (ret !=0)
		goto err_exit2;

	ret = i2c_add_driver(&simg92_driver);
	if (ret !=0)
		goto err_exit3;

	ret = i2c_add_driver(&simg9A_driver);
	if (ret !=0)
		goto err_exit4;

	ret = i2c_add_driver(&simgA0_driver);
	if (ret !=0)
		goto err_exit5;

	ret = i2c_add_driver(&simg60_driver);
	if (ret !=0)
		goto err_exit6;

	ret = i2c_add_driver(&simgC8_driver);
	if (ret !=0)
		goto err_exit7;

	return 0;

err_exit7:
	printk(KERN_INFO"simgC8_driver fail\n");
	i2c_del_driver(&simgC8_driver);

err_exit6:
	printk(KERN_INFO"simg60_driver fail\n");
	i2c_del_driver(&simg60_driver);

err_exit5:
	printk(KERN_INFO"simgA0_driver fail\n");
	i2c_del_driver(&simgA0_driver);

err_exit4:
	printk(KERN_INFO"simg9A_driver fail\n");
	i2c_del_driver(&simg9A_driver);

err_exit3:
	printk(KERN_INFO"simg92_driver fail\n");
	i2c_del_driver(&simg92_driver);

err_exit2:
	printk(KERN_INFO"simg7A_driver fail\n");
	i2c_del_driver(&simg7A_driver);

err_exit1:
	printk(KERN_INFO"simg72_driver fail\n");
	i2c_del_driver(&simg72_driver);

	return ret;

}

static void __exit simg_exit(void)
{
	i2c_del_driver(&simg72_driver);
	i2c_del_driver(&simg7A_driver);
	i2c_del_driver(&simg92_driver);
	i2c_del_driver(&simg9A_driver);
	i2c_del_driver(&simgC8_driver);
	i2c_del_driver(&simgA0_driver);
	i2c_del_driver(&simg60_driver);
}


module_init(simg_init);
module_exit(simg_exit);

MODULE_DESCRIPTION("Silicon Image MHL Transmitter driver");
MODULE_AUTHOR("Daniel(Philju) Lee <daniel.lee@siliconimage.com>");
MODULE_LICENSE("GPL");
