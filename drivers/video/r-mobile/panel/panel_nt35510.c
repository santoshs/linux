/*
 * drivers/video/r-mobile/panel/panel_nt35510.c
 *
 * Copyright (C) 2013 Renesas Electronics Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <linux/kernel.h>
#include <linux/delay.h>

#include <linux/gpio.h>

#include <video/sh_mobile_lcdc.h>

#include <rtapi/screen_display.h>


#include <linux/platform_device.h>
#include <linux/fb.h>

#include <linux/regulator/consumer.h>
#include <linux/lcd.h>

#include <mach/memory-r8a7373.h>

#include "panel_common.h"

/* #define NT35510_DRAW_BLACK_KERNEL */

#define NT35510_POWAREA_MNG_ENABLE
/* #define NT35510_GED_ORG */

/* #define NT35510_ENABLE_VIDEO_MODE */

#ifdef NT35510_POWAREA_MNG_ENABLE
#include <rtapi/system_pwmng.h>
#endif

#define NT35510_ESD_RECOVERY_ENABLE

/* framebuffer address and size */
#define R_MOBILE_M_BUFF_ADDR		SDRAM_FRAME_BUFFER_START_ADDR
#define R_MOBILE_M_BUFF_SIZE		(SDRAM_FRAME_BUFFER_END_ADDR - \
					 SDRAM_FRAME_BUFFER_START_ADDR + 1)

/* panel pixel */
#define R_MOBILE_M_PANEL_PIXEL_WIDTH	 480
#define R_MOBILE_M_PANEL_PIXEL_HEIGHT	 800

/* panel size (mm) */
#define R_MOBILE_M_PANEL_SIZE_WIDTH	52
#define R_MOBILE_M_PANEL_SIZE_HEIGHT	86

/* pixclock = 1000000 / DCF */
#define R_MOBILE_M_PANEL_PIXCLOCK	 23199

/* timing */
#define R_MOBILE_M_PANEL_LEFT_MARGIN	 24
#define R_MOBILE_M_PANEL_RIGHT_MARGIN	 376
#define R_MOBILE_M_PANEL_HSYNC_LEN	 8
#define R_MOBILE_M_PANEL_UPPER_MARGIN	 4
#define R_MOBILE_M_PANEL_LOWER_MARGIN	 4
#define R_MOBILE_M_PANEL_VSYNC_LEN	 1
#define R_MOBILE_M_PANEL_H_TOTAL	 888
#define R_MOBILE_M_PANEL_V_TOTAL	 809

#define LCD_DSITCKCR		0x00000007
#define LCD_DSI0PCKCR		0x00000025
#define LCD_DSI0PHYCR		0x2A800014
#define LCD_SYSCONF		0x00000703
#define LCD_TIMSET0		0x4C2C4332
#define LCD_TIMSET1		0x00080132
#define LCD_DSICTRL		0x00000001
#define LCD_VMCTR1		0x0001003E
#define LCD_VMCTR2		0x00000710
#define LCD_VMLEN1		0x05A00000
#define LCD_VMLEN2		0x00360000
#define LCD_VMLEN3		0x00000000
#define LCD_VMLEN4		0x00000000
#define LCD_DTCTR		0x00000006
#define LCD_MLDHCNR		0x003C006F
#define LCD_MLDHSYNR		0x0001006B
#define LCD_MLDHAJR		0x00000000
#define LCD_MLDVLNR		0x03200329
#define LCD_MLDVSYNR		0x00010325
#define LCD_MLDMT1R		0x0400000B
#define LCD_LDDCKR		0x00010040
#define LCD_MLDDCKPAT1R		0x00000000
#define LCD_MLDDCKPAT2R		0x00000000
#define LCD_PHYTEST		0x0000038C

#define LCD_MASK_DSITCKCR	0x000000BF
#define LCD_MASK_DSI0PCKCR	0x0000703F
#define LCD_MASK_DSI0PHYCR	0x000000FF
#define LCD_MASK_SYSCONF	0x00000F0F
#define LCD_MASK_TIMSET0	0x7FFFF7F7
#define LCD_MASK_TIMSET1	0x003F03FF
#define LCD_MASK_DSICTRL	0x00000601
#define LCD_MASK_VMCTR1		0x00F3F03F
#define LCD_MASK_VMCTR2		0x07E2073B
#define LCD_MASK_VMLEN1		0xFFFFFFFF
#define LCD_MASK_VMLEN2		0xFFFFFFFF
#define LCD_MASK_VMLEN3		0xFFFFFFFF
#define LCD_MASK_VMLEN4		0xFFFF0000
#define LCD_MASK_DTCTR		0x00000002
#define LCD_MASK_MLDHCNR	0x07FF07FF
#define LCD_MASK_MLDHSYNR	0x001F07FF
#define LCD_MASK_MLDHAJR	0x07070707
#define LCD_MASK_MLDVLNR	0x1FFF1FFF
#define LCD_MASK_MLDVSYNR	0x001F1FFF
#define LCD_MASK_MLDMT1R	0x1F03FCCF
#define LCD_MASK_LDDCKR		0x0007007F
#define LCD_MASK_MLDDCKPAT1R	0x0FFFFFFF
#define LCD_MASK_MLDDCKPAT2R	0xFFFFFFFF
#define LCD_MASK_PHYTEST	0x000003CC

#define LCD_DSI0PCKCR_40HZ	0x00000023
#define LCD_DSI0PHYCR_40HZ	0x2A80000B

#define NT35510_INIT_RETRY_COUNT 3

#define POWER_IS_ON(pwr)	((pwr) <= FB_BLANK_NORMAL)
static int nt35510_panel_suspend(void);
static int nt35510_panel_resume(void);
static void mipi_display_reset(void);

static struct fb_panel_info r_mobile_info = {
	.pixel_width	= R_MOBILE_M_PANEL_PIXEL_WIDTH,
	.pixel_height	= R_MOBILE_M_PANEL_PIXEL_HEIGHT,
	.size_width	= R_MOBILE_M_PANEL_SIZE_WIDTH,
	.size_height	= R_MOBILE_M_PANEL_SIZE_HEIGHT,
	.buff_address	= R_MOBILE_M_BUFF_ADDR,
	.pixclock      = R_MOBILE_M_PANEL_PIXCLOCK,
	.left_margin   = R_MOBILE_M_PANEL_LEFT_MARGIN,
	.right_margin  = R_MOBILE_M_PANEL_RIGHT_MARGIN,
	.upper_margin  = R_MOBILE_M_PANEL_UPPER_MARGIN,
	.lower_margin  = R_MOBILE_M_PANEL_LOWER_MARGIN,
	.hsync_len     = R_MOBILE_M_PANEL_HSYNC_LEN,
	.vsync_len     = R_MOBILE_M_PANEL_VSYNC_LEN,
};

static screen_disp_lcd_if r_mobile_lcd_if_param = {
	.DSITCKCR    = LCD_DSITCKCR,
	.DSI0PCKCR   = LCD_DSI0PCKCR,
	.DSI0PHYCR   = LCD_DSI0PHYCR,
	.SYSCONF     = LCD_SYSCONF,
	.TIMSET0     = LCD_TIMSET0,
	.TIMSET1     = LCD_TIMSET1,
	.DSICTRL     = LCD_DSICTRL,
	.VMCTR1      = LCD_VMCTR1,
	.VMCTR2      = LCD_VMCTR2,
	.VMLEN1      = LCD_VMLEN1,
	.VMLEN2      = LCD_VMLEN2,
	.VMLEN3      = LCD_VMLEN3,
	.VMLEN4      = LCD_VMLEN4,
	.DTCTR       = LCD_DTCTR,
	.MLDHCNR     = LCD_MLDHCNR,
	.MLDHSYNR    = LCD_MLDHSYNR,
	.MLDHAJR     = LCD_MLDHAJR,
	.MLDVLNR     = LCD_MLDVLNR,
	.MLDVSYNR    = LCD_MLDVSYNR,
	.MLDMT1R     = LCD_MLDMT1R,
	.LDDCKR      = LCD_LDDCKR,
	.MLDDCKPAT1R = LCD_MLDDCKPAT1R,
	.MLDDCKPAT2R = LCD_MLDDCKPAT2R,
	.PHYTEST     = LCD_PHYTEST,
};

static screen_disp_lcd_if r_mobile_lcd_if_param_mask = {
	LCD_MASK_DSITCKCR,
	LCD_MASK_DSI0PCKCR,
	LCD_MASK_DSI0PHYCR,
	LCD_MASK_SYSCONF,
	LCD_MASK_TIMSET0,
	LCD_MASK_TIMSET1,
	LCD_MASK_DSICTRL,
	LCD_MASK_VMCTR1,
	LCD_MASK_VMCTR2,
	LCD_MASK_VMLEN1,
	LCD_MASK_VMLEN2,
	LCD_MASK_VMLEN3,
	LCD_MASK_VMLEN4,
	LCD_MASK_DTCTR,
	LCD_MASK_MLDHCNR,
	LCD_MASK_MLDHSYNR,
	LCD_MASK_MLDHAJR,
	LCD_MASK_MLDVLNR,
	LCD_MASK_MLDVSYNR,
	LCD_MASK_MLDMT1R,
	LCD_MASK_LDDCKR,
	LCD_MASK_MLDDCKPAT1R,
	LCD_MASK_MLDDCKPAT2R,
	LCD_MASK_PHYTEST,
};

static unsigned int reset_gpio;
static unsigned int irq_portno;
static struct regulator *power_ldo_3v;
static struct regulator *power_ldo_1v8;

struct specific_cmdset {
	unsigned char cmd;
	unsigned char *data;
	int size;
};
#define MIPI_DSI_DCS_LONG_WRITE		(0x39)
#define MIPI_DSI_GEN_LONG_WRITE		(0x29)
#define MIPI_DSI_DCS_SHORT_WRITE_PARAM	(0x15)
#define MIPI_DSI_GEN_SHORT_WRITE_PARAM	(0x23)
#define MIPI_DSI_DCS_SHORT_WRITE	(0x05)
#define MIPI_DSI_GEN_SHORT_WRITE	(0x03)
#define MIPI_DSI_SET_MAX_RETURN_PACKET	(0x37)
#define MIPI_DSI_DCS_READ		(0x06)
#define MIPI_DSI_DELAY			(0x00)
#define MIPI_DSI_BLACK			(0x01)
#define MIPI_DSI_END			(0xFF)

/* Power Setting Sequence */
static unsigned char maucctr1[] = { 0xF0,
		0x55, 0xAA, 0x52, 0x08, 0x01 };

static unsigned char setavdd[] = { 0xB0,
		0x09, 0x09, 0x09 };

static unsigned char bt1ctr[] = { 0xB6,
		0x34, 0x34, 0x34 };

static unsigned char setavee[] = { 0xB1,
		0x09, 0x09, 0x09 };

static unsigned char bt2ctr[] = { 0xB7,
		0x24, 0x24, 0x24 };

static unsigned char setvgh[] = { 0xB3,
		0x05, 0x05, 0x05 };

static unsigned char bt4ctr[] = { 0xB9,
		0x24, 0x24, 0x24 };

static unsigned char vghctr[] = { 0xBF, 0x01 };

static unsigned char setvglreg[] = { 0xB5,
		0x0B, 0x0B, 0x0B };

static unsigned char bt5ctr[] = { 0xBA,
		0x24, 0x24, 0x24 };

/* static unsigned char btenctr[] = { 0xC2, 0x01 }; */

static unsigned char setvgp[] = { 0xBC,
		0x00, 0xA3, 0x00 };

static unsigned char setvgn[] = { 0xBD,
		0x00, 0xA3, 0x00 };

/* Initializing Sequence */
static unsigned char maucctr0[] = { 0xF0,
		0x55, 0xAA, 0x52, 0x08, 0x00 };

#ifndef NT35510_ENABLE_VIDEO_MODE
static unsigned char dopctr[] = { 0xB1,
		0x4C, 0x04 };

static unsigned char invctr[] = { 0x36, 0x02 };
#else
static unsigned char dopctr[] = { 0xB1,
		0x1C, 0x06 };

static unsigned char invctr[] = { 0x36, 0x00 };
#endif

static unsigned char sdhdtctr[] = { 0xB6, 0x0A };

static unsigned char gseqctr[] = { 0xB7,
		0x00, 0x00 };

static unsigned char sdeqctr[] = { 0xB8,
		0x01, 0x05, 0x05, 0x05 };

static unsigned char sdvpctr[] = { 0xBA, 0x01 };

static unsigned char dpfrctr1[] = { 0xBD,
		0x01, 0x84, 0x07, 0x32, 0x00 };

static unsigned char dpfrctr2[] = { 0xBE,
		0x01, 0x84, 0x07, 0x31, 0x00 };

static unsigned char dpfrctr3[] = { 0xBF,
		0x01, 0x84, 0x07, 0x31, 0x00 };

static unsigned char teon[] = { 0x35, 0x00 };

static unsigned char dptmctr12[] = { 0xCC,
		0x03, 0x00, 0x00 };

/* Gamma Setting Sequence */
/* gamma R+ setting */
static unsigned char gmprctr1[] = { 0xD1,
		0x00, 0x01, 0x00, 0x43, 0x00, 0x6B, 0x00, 0x87,
		0x00, 0xA3, 0x00, 0xCE, 0x00, 0xF1, 0x01, 0x27,
		0x01, 0x53, 0x01, 0x98, 0x01, 0xCE, 0x02, 0x22,
		0x02, 0x83, 0x02, 0x78, 0x02, 0x9E, 0x02, 0xDD,
		0x03, 0x00, 0x03, 0x2E, 0x03, 0x54, 0x03, 0x7F,
		0x03, 0x95, 0x03, 0xB3, 0x03, 0xC2, 0x03, 0xE1,
		0x03, 0xF1, 0x03, 0xFE };

/* gamma G+ setting */
static unsigned char gmprctr2[] = { 0xD2,
		0x00, 0x01, 0x00, 0x43, 0x00, 0x6B, 0x00, 0x87,
		0x00, 0xA3, 0x00, 0xCE, 0x00, 0xF1, 0x01, 0x27,
		0x01, 0x53, 0x01, 0x98, 0x01, 0xCE, 0x02, 0x22,
		0x02, 0x83, 0x02, 0x78, 0x02, 0x9E, 0x02, 0xDD,
		0x03, 0x00, 0x03, 0x2E, 0x03, 0x54, 0x03, 0x7F,
		0x03, 0x95, 0x03, 0xB3, 0x03, 0xC2, 0x03, 0xE1,
		0x03, 0xF1, 0x03, 0xFE };

/* gamma B+ setting */
static unsigned char gmprctr3[] = { 0xD3,
		0x00, 0x01, 0x00, 0x43, 0x00, 0x6B, 0x00, 0x87,
		0x00, 0xA3, 0x00, 0xCE, 0x00, 0xF1, 0x01, 0x27,
		0x01, 0x53, 0x01, 0x98, 0x01, 0xCE, 0x02, 0x22,
		0x02, 0x83, 0x02, 0x78, 0x02, 0x9E, 0x02, 0xDD,
		0x03, 0x00, 0x03, 0x2E, 0x03, 0x54, 0x03, 0x7F,
		0x03, 0x95, 0x03, 0xB3, 0x03, 0xC2, 0x03, 0xE1,
		0x03, 0xF1, 0x03, 0xFE };

/* gamma R- setting */
static unsigned char gmprctr4[] = { 0xD4,
		0x00, 0x01, 0x00, 0x43, 0x00, 0x6B, 0x00, 0x87,
		0x00, 0xA3, 0x00, 0xCE, 0x00, 0xF1, 0x01, 0x27,
		0x01, 0x53, 0x01, 0x98, 0x01, 0xCE, 0x02, 0x22,
		0x02, 0x43, 0x02, 0x50, 0x02, 0x9E, 0x02, 0xDD,
		0x03, 0x00, 0x03, 0x2E, 0x03, 0x54, 0x03, 0x7F,
		0x03, 0x95, 0x03, 0xB3, 0x03, 0xC2, 0x03, 0xE1,
		0x03, 0xF1, 0x03, 0xFE };

/* gamma G- setting */
static unsigned char gmpgctr1[] = { 0xD5,
		0x00, 0x01, 0x00, 0x43, 0x00, 0x6B, 0x00, 0x87,
		0x00, 0xA3, 0x00, 0xCE, 0x00, 0xF1, 0x01, 0x27,
		0x01, 0x53, 0x01, 0x98, 0x01, 0xCE, 0x02, 0x22,
		0x02, 0x43, 0x02, 0x50, 0x02, 0x9E, 0x02, 0xDD,
		0x03, 0x00, 0x03, 0x2E, 0x03, 0x54, 0x03, 0x7F,
		0x03, 0x95, 0x03, 0xB3, 0x03, 0xC2, 0x03, 0xE1,
		0x03, 0xF1, 0x03, 0xFE };

/* gamma B- setting */
static unsigned char gmpgctr2[] = { 0xD6,
		0x00, 0x01, 0x00, 0x43, 0x00, 0x6B, 0x00, 0x87,
		0x00, 0xA3, 0x00, 0xCE, 0x00, 0xF1, 0x01, 0x27,
		0x01, 0x53, 0x01, 0x98, 0x01, 0xCE, 0x02, 0x22,
		0x02, 0x43, 0x02, 0x50, 0x02, 0x9E, 0x02, 0xDD,
		0x03, 0x00, 0x03, 0x2E, 0x03, 0x54, 0x03, 0x7F,
		0x03, 0x95, 0x03, 0xB3, 0x03, 0xC2, 0x03, 0xE1,
		0x03, 0xF1, 0x03, 0xFE };

/* Sleep Out */
static unsigned char slpout[] = { 0x11 };

/* Display On */
static unsigned char dispon[] = { 0x29 };

/* Display off */
static unsigned char dispoff[] = { 0x28 };

/* Sleep in */
static unsigned char slpin[] = { 0x10 };

/* Column Address Set */
static unsigned char caset[] = { 0x2A,
		0x00, 0x00, 0x01, 0xDF };

/* Row Address Set */
static unsigned char raset[] = { 0x2B,
		0x00, 0x00, 0x03, 0x1F };

/* Normal Display Mode On */
static unsigned char noron[] = { 0x13 };

static unsigned char dpfrctr1_40hz[] = { 0xBD,
		0x02, 0x45, 0x07, 0x32, 0x00 };

static struct specific_cmdset lcdfreq_cmd[] = {
	{ MIPI_DSI_DCS_LONG_WRITE,  dpfrctr1,  sizeof(dpfrctr1) },
	{ MIPI_DSI_END,             NULL,      0                }
};

static const struct specific_cmdset initialize_cmdset[] = {
	{ MIPI_DSI_DELAY,           NULL,      120              },

	{ MIPI_DSI_DCS_LONG_WRITE,  maucctr1,  sizeof(maucctr1) },
	{ MIPI_DSI_DCS_LONG_WRITE,  setavdd,   sizeof(setavdd)  },
	{ MIPI_DSI_DCS_LONG_WRITE,  bt1ctr,    sizeof(bt1ctr)   },
	{ MIPI_DSI_DCS_LONG_WRITE,  setavee,   sizeof(setavee)  },
	{ MIPI_DSI_DCS_LONG_WRITE,  bt2ctr,    sizeof(bt2ctr)   },
	{ MIPI_DSI_DCS_LONG_WRITE,  setvgh,    sizeof(setvgh)   },
	{ MIPI_DSI_DCS_LONG_WRITE,  bt4ctr,    sizeof(bt4ctr)   },
	{ MIPI_DSI_DCS_LONG_WRITE,  vghctr,    sizeof(vghctr)   },
	{ MIPI_DSI_DCS_LONG_WRITE,  setvglreg, sizeof(setvglreg)},
	{ MIPI_DSI_DCS_LONG_WRITE,  bt5ctr,    sizeof(bt5ctr)   },
/*	{ MIPI_DSI_DCS_LONG_WRITE,  btenctr,   sizeof(btenctr)  }, */
	{ MIPI_DSI_DCS_LONG_WRITE,  setvgp,    sizeof(setvgp)   },
	{ MIPI_DSI_DCS_LONG_WRITE,  setvgn,    sizeof(setvgn)   },

	{ MIPI_DSI_DCS_LONG_WRITE,  gmprctr1,  sizeof(gmprctr1) },
	{ MIPI_DSI_DCS_LONG_WRITE,  gmprctr2,  sizeof(gmprctr2) },
	{ MIPI_DSI_DCS_LONG_WRITE,  gmprctr3,  sizeof(gmprctr3) },
	{ MIPI_DSI_DCS_LONG_WRITE,  gmprctr4,  sizeof(gmprctr4) },
	{ MIPI_DSI_DCS_LONG_WRITE,  gmpgctr1,  sizeof(gmpgctr1) },
	{ MIPI_DSI_DCS_LONG_WRITE,  gmpgctr2,  sizeof(gmpgctr2) },

	{ MIPI_DSI_DCS_LONG_WRITE,  maucctr0,  sizeof(maucctr0) },
	{ MIPI_DSI_DCS_LONG_WRITE,  sdhdtctr,  sizeof(sdhdtctr) },
	{ MIPI_DSI_DCS_LONG_WRITE,  gseqctr,   sizeof(gseqctr)  },
	{ MIPI_DSI_DCS_LONG_WRITE,  sdeqctr,   sizeof(sdeqctr)  },

	{ MIPI_DSI_DCS_LONG_WRITE,  sdvpctr,   sizeof(sdvpctr)  },
	{ MIPI_DSI_DCS_LONG_WRITE,  invctr,    sizeof(invctr)   },
	{ MIPI_DSI_DCS_LONG_WRITE,  dpfrctr1,  sizeof(dpfrctr1) },
	{ MIPI_DSI_DCS_LONG_WRITE,  dpfrctr2,  sizeof(dpfrctr2) },
	{ MIPI_DSI_DCS_LONG_WRITE,  dpfrctr3,  sizeof(dpfrctr3) },
	{ MIPI_DSI_DCS_LONG_WRITE,  dptmctr12,  sizeof(dptmctr12) },
	{ MIPI_DSI_DCS_LONG_WRITE,  dopctr,    sizeof(dopctr)   },
#ifndef NT35510_ENABLE_VIDEO_MODE
	{ MIPI_DSI_DCS_SHORT_WRITE_PARAM, teon, sizeof(teon)    },
#endif

	{ MIPI_DSI_DCS_SHORT_WRITE, noron,    sizeof(noron)   },
	{ MIPI_DSI_DCS_LONG_WRITE,  caset,  sizeof(caset) },
	{ MIPI_DSI_DCS_LONG_WRITE,  raset,  sizeof(raset) },
	{ MIPI_DSI_DCS_LONG_WRITE, slpout,    sizeof(slpout)   },
	{ MIPI_DSI_DELAY,           NULL,      150              },
	{ MIPI_DSI_BLACK,           NULL,      0                },

#ifndef NT35510_ENABLE_VIDEO_MODE
	{ MIPI_DSI_DCS_SHORT_WRITE, dispon,    sizeof(dispon)   },
#endif /* NT35510_ENABLE_VIDEO_MODE */

	{ MIPI_DSI_END,             NULL,      0                }
};

#ifdef NT35510_ENABLE_VIDEO_MODE
static const struct specific_cmdset initialize_cmdset_phase2[] = {
	{ MIPI_DSI_DCS_SHORT_WRITE, dispon,    sizeof(dispon)   },
	{ MIPI_DSI_DELAY,           NULL,      150              },
	{ MIPI_DSI_BLACK,           NULL,      0                },
	{ MIPI_DSI_END,             NULL,      0                }
};
#endif /* NT35510_ENABLE_VIDEO_MODE */

static const struct specific_cmdset demise_cmdset[] = {
	{ MIPI_DSI_DCS_SHORT_WRITE, dispoff,   sizeof(dispoff)  },
	{ MIPI_DSI_DELAY,           NULL,      150              },
	{ MIPI_DSI_DCS_SHORT_WRITE, slpin,     sizeof(slpin)    },
	{ MIPI_DSI_DELAY,           NULL,      150              },
	{ MIPI_DSI_END,             NULL,      0                }
};

static struct specific_cmdset mauc0_cmd[] = {
	{ MIPI_DSI_DCS_LONG_WRITE,  maucctr0,  sizeof(maucctr0) },
	{ MIPI_DSI_END,             NULL,      0                }
};

static struct specific_cmdset mauc1_cmd[] = {
	{ MIPI_DSI_DCS_LONG_WRITE,  maucctr1,  sizeof(maucctr1) },
	{ MIPI_DSI_END,             NULL,      0                }
};

static int is_dsi_read_enabled;

static struct fb_info *common_fb_info;
static int panel_specific_cmdset(void *lcd_handle,
				   const struct specific_cmdset *cmdset);
enum lcdfreq_level_idx {
	LEVEL_NORMAL,		/* 60Hz */
	LEVEL_LOW,		/* 40Hz */
	LCDFREQ_LEVEL_END
};

struct lcd_info {
	enum lcdfreq_level_idx	level;	/* Current level */
	struct mutex		lock;	/* Lock for change frequency */
	struct device		*dev;	/* Hold device of LCD */
	struct device_attribute	*attr;	/* Hold attribute info */
	struct lcd_device	*ld;	/* LCD device info */

	unsigned int			ldi_enable;
	unsigned int			power;
};

static struct lcd_info lcd_info_data;

#if defined(NT35510_ESD_RECOVERY_ENABLE)
#define DURATION_TIME 3000 /* 3000ms */
#define ESD_CHECK_DISABLE 0
#define ESD_CHECK_ENABLE 1

static struct delayed_work esd_check_work;
static struct mutex esd_check_mutex;
static int esd_check_flag;

#endif /* NT35510_ESD_RECOVERY_ENABLE */

static int lcdfreq_lock_free(struct device *dev)
{
	void *screen_handle;
	screen_disp_set_lcd_if_param set_lcd_if_param;
	screen_disp_delete disp_delete;
	int ret;

	printk(KERN_DEBUG "%s\n", __func__);

	screen_handle =  screen_display_new();

	/* Setting peculiar to panel */
	set_lcd_if_param.handle			= screen_handle;
	set_lcd_if_param.port_no		= irq_portno;
	set_lcd_if_param.lcd_if_param		= &r_mobile_lcd_if_param;
	set_lcd_if_param.lcd_if_param_mask	= &r_mobile_lcd_if_param_mask;
	ret = screen_display_set_lcd_if_parameters(&set_lcd_if_param);
	if (ret != SMAP_LIB_DISPLAY_OK) {
		printk(KERN_ALERT "disp_set_lcd_if_parameters err!\n");
		goto out;
	}

#ifndef NT35510_ENABLE_VIDEO_MODE
	/* Transmit DSI command peculiar to a panel */
	ret = panel_specific_cmdset(screen_handle, lcdfreq_cmd);
	if (ret != 0) {
		printk(KERN_ALERT "panel_specific_cmdset err!\n");
		goto out;
	}
#endif /* NT35510_ENABLE_VIDEO_MODE */

out:
	disp_delete.handle = screen_handle;
	screen_display_delete(&disp_delete);

	return ret;
}

#if defined(NT35510_ESD_RECOVERY_ENABLE)
static int nt35510_panel_simple_reset(void)
{
	void *screen_handle;
	screen_disp_stop_lcd disp_stop_lcd;
	screen_disp_start_lcd start_lcd;
	screen_disp_draw disp_draw;
	screen_disp_delete disp_delete;
	int ret;

#ifdef NT35510_POWAREA_MNG_ENABLE
	void *system_handle;
	system_pmg_param powarea_end_notify;
	system_pmg_param powarea_start_notify;
	system_pmg_delete pmg_delete;

	system_handle = system_pwmng_new();
#endif

	printk(KERN_DEBUG "%s\n", __func__);

	is_dsi_read_enabled = 0;
	screen_handle =  screen_display_new();

	/* Start suspend sequence */
	/* GPIO control */
	gpio_direction_output(reset_gpio, 0);
	msleep(20);
	gpio_direction_output(reset_gpio, 1);
	regulator_disable(power_ldo_3v);
	regulator_disable(power_ldo_1v8);
	msleep(25);
	gpio_direction_output(reset_gpio, 0);

	disp_stop_lcd.handle		= screen_handle;
	disp_stop_lcd.output_mode	= RT_DISPLAY_LCD1;
	ret = screen_display_stop_lcd(&disp_stop_lcd);
	if (ret != SMAP_LIB_DISPLAY_OK)
		printk(KERN_ALERT "display_stop_lcd err!\n");
	/* End suspend sequence */

#ifdef NT35510_POWAREA_MNG_ENABLE
	/* Notifying the Beginning of Using Power Area */
	printk(KERN_INFO "End A4LC power area\n");
	powarea_end_notify.handle		= system_handle;
	powarea_end_notify.powerarea_name	= RT_PWMNG_POWERAREA_A4LC;
	ret = system_pwmng_powerarea_end_notify(&powarea_end_notify);
	if (ret != SMAP_LIB_PWMNG_OK) {
		printk(KERN_ALERT "system_pwmng_powerarea_end_notify err!\n");
		goto out;
	}

	msleep(20);

	/* Notifying the Beginning of Using Power Area */
	printk(KERN_INFO "Start A4LC power area\n");
	powarea_start_notify.handle		= system_handle;
	powarea_start_notify.powerarea_name	= RT_PWMNG_POWERAREA_A4LC;
	ret = system_pwmng_powerarea_start_notify(&powarea_start_notify);
	if (ret != SMAP_LIB_PWMNG_OK) {
		printk(KERN_ALERT "system_pwmng_powerarea_start_notify err!\n");
		goto out;
	}
#endif /* NT35510_POWAREA_MNG_ENABLE */

	msleep(20);

	/* Start resume sequence */
	/* LCD panel reset */
	mipi_display_reset();

	/* Start a display to LCD */
	start_lcd.handle	= screen_handle;
	start_lcd.output_mode	= RT_DISPLAY_LCD1;
	ret = screen_display_start_lcd(&start_lcd);
	if (ret != SMAP_LIB_DISPLAY_OK) {
		printk(KERN_ALERT "disp_start_lcd err!\n");
		goto out;
	}

	/* Transmit DSI command peculiar to a panel */
	ret = panel_specific_cmdset(screen_handle, initialize_cmdset);
	if (ret != 0) {
		printk(KERN_ALERT "panel_specific_cmdset err!\n");
		goto out;
	}
	/* End resume sequence */

	is_dsi_read_enabled = 1;

	disp_draw.handle = screen_handle;
	disp_draw.output_mode = RT_DISPLAY_LCD1;
	disp_draw.draw_rect.x = 0;
	disp_draw.draw_rect.y = 0;
	disp_draw.draw_rect.width = R_MOBILE_M_PANEL_PIXEL_WIDTH;
	disp_draw.draw_rect.height = R_MOBILE_M_PANEL_PIXEL_HEIGHT;
#ifdef CONFIG_FB_SH_MOBILE_RGB888
	disp_draw.format = RT_DISPLAY_FORMAT_RGB888;
#else
	disp_draw.format = RT_DISPLAY_FORMAT_ARGB8888;
#endif
	disp_draw.buffer_id = RT_DISPLAY_BUFFER_A;
	disp_draw.buffer_offset = 0;
	disp_draw.rotate = RT_DISPLAY_ROTATE_270;
	ret = screen_display_draw(&disp_draw);
	if (ret != SMAP_LIB_DISPLAY_OK) {
		r_mobile_fb_err_msg(ret, "screen_display_draw err!");
		goto out;
	}

	printk(KERN_ALERT "nt35510_panel simple initialized\n");

out:
	disp_delete.handle = screen_handle;
	screen_display_delete(&disp_delete);

#ifdef NT35510_POWAREA_MNG_ENABLE
	pmg_delete.handle = system_handle;
	system_pwmng_delete(&pmg_delete);
#endif /* NT35510_POWAREA_MNG_ENABLE */

	return ret;
}


static int nt35510_panel_check(void)
{
	unsigned char rdnumed;
	unsigned char wonder[3];
	unsigned char rdidic[3];
	unsigned char exp_rdidic[3] = {0x55, 0x10, 0x05};
	void *screen_handle;
	screen_disp_delete disp_delete;
	int ret;

	screen_handle =  screen_display_new();

	/*Read Number of Errors on DSI*/
	ret = panel_dsi_read(MIPI_DSI_DCS_READ, 0x05, 1, &rdnumed);
	printk(KERN_DEBUG "read_data(0x05) = %02X : ret(%d)\n", rdnumed, ret);
	if (rdnumed != 0x00)
		ret = -1;
	if (ret != 0)
		goto out;

	/*Read ID for IC Vender Code*/
	ret = panel_specific_cmdset(screen_handle, mauc1_cmd);
	if (ret != 0)
		goto out;
	ret = panel_dsi_read(MIPI_DSI_DCS_READ, 0xC5, 1, rdidic);
	printk(KERN_DEBUG "read_data(0xC5) = %02X : ret(%d) page 1\n",
							rdidic[0], ret);
	if (exp_rdidic[0] != rdidic[0])
		ret = -1;
	if (ret != 0)
		goto out;

	/*for check switching page*/
	ret = panel_specific_cmdset(screen_handle, mauc0_cmd);
	if (ret != 0)
		goto out;
	ret = panel_dsi_read(MIPI_DSI_DCS_READ, 0xC5, 1, wonder);
	printk(KERN_DEBUG "read_data(0xC5) = %02X : ret(%d) page 0\n",
							wonder[0], ret);
	if (wonder[0] == rdidic[0])
		ret = -1;
	if (ret != 0)
		goto out;

out:
	disp_delete.handle = screen_handle;
	screen_display_delete(&disp_delete);

	return ret;
}

static void nt35510_panel_esd_check_work(struct work_struct *work)
{
	struct delayed_work *dwork = to_delayed_work(work);

	/* For the disable entering suspend */
	mutex_lock(&esd_check_mutex);

	while (nt35510_panel_check())
		while (nt35510_panel_simple_reset())
			msleep(20);

	if (esd_check_flag == ESD_CHECK_ENABLE)
		schedule_delayed_work(dwork, msecs_to_jiffies(DURATION_TIME));

	/* Enable suspend */
	mutex_unlock(&esd_check_mutex);
}
#endif

static ssize_t level_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	printk(KERN_DEBUG "%s\n", __func__);
	return sprintf(buf, "%d\n", lcd_info_data.level);
}

static ssize_t level_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned int value;
	int ret;

	printk(KERN_DEBUG "%s\n", __func__);

	mutex_lock(&lcd_info_data.lock);

	ret = kstrtoul(buf, 0, (unsigned long *)&value);

	printk(KERN_DEBUG "\t%s :: value=%d\n", __func__, value);

	if (value >= LCDFREQ_LEVEL_END) {
		count = -EINVAL;
		goto out;
	}

	if (value) {
		/* set freq 40Hz */
		printk(KERN_ALERT "set low freq(40Hz)\n");

		r_mobile_lcd_if_param.DSI0PCKCR = LCD_DSI0PCKCR_40HZ;
		r_mobile_lcd_if_param.DSI0PHYCR = LCD_DSI0PHYCR_40HZ;

		lcdfreq_cmd[0].data = dpfrctr1_40hz;
	} else {
		/* set freq 60Hz */
		printk(KERN_ALERT "set normal freq(60Hz)\n");

		r_mobile_lcd_if_param.DSI0PCKCR = LCD_DSI0PCKCR;
		r_mobile_lcd_if_param.DSI0PHYCR = LCD_DSI0PHYCR;

		lcdfreq_cmd[0].data = dpfrctr1;
	}

	ret = lcdfreq_lock_free(dev);
	if (ret) {
		printk(KERN_ALERT "%s fail\n", __func__);
		count = -EINVAL;
		goto out;
	}

	lcd_info_data.level	= value;
	lcd_info_data.dev	= dev;
	lcd_info_data.attr	= attr;
out:
	mutex_unlock(&lcd_info_data.lock);

	return count;
}

static int lcdfreq_resume(void)
{
#ifndef NT35510_ENABLE_VIDEO_MODE
	char buf[10];

	/* Resume when a frame rate is not 60Hz */
	if (lcd_info_data.level != 0) {
		sprintf(buf, "%d", lcd_info_data.level);
		level_store(lcd_info_data.dev,
			    lcd_info_data.attr, buf, 0);
	}
#endif /* NT35510_ENABLE_VIDEO_MODE */

	return 0;
}

static DEVICE_ATTR(level, S_IRUGO|S_IWUSR, level_show, level_store);

static struct attribute *lcdfreq_attributes[] = {
	&dev_attr_level.attr,
	NULL,
};

static struct attribute_group lcdfreq_attr_group = {
	.name = "lcdfreq",
	.attrs = lcdfreq_attributes,
};

static int nt35510_lcd_frequency_register(struct device *dev)
{
	struct lcd_info *lcdfreq = NULL;
	int ret;

	printk(KERN_DEBUG "%s\n", __func__);

	lcdfreq = &lcd_info_data;
	lcdfreq->dev = dev;
	lcdfreq->level = LEVEL_NORMAL;

	mutex_init(&lcdfreq->lock);

	ret = sysfs_create_group(&lcdfreq->dev->kobj, &lcdfreq_attr_group);
	if (ret < 0) {
		printk(KERN_ALERT "fail to add sysfs entries, %d\n", __LINE__);

		return ret;
	}

	printk(KERN_DEBUG "%s is done\n", __func__);

	return 0;
}

static void nt35510_lcd_frequency_unregister(void)
{
	printk(KERN_DEBUG "%s\n", __func__);

	sysfs_remove_group(&lcd_info_data.dev->kobj,
					&lcdfreq_attr_group);
	mutex_destroy(&lcd_info_data.lock);

	printk(KERN_DEBUG "%s is done\n", __func__);

}


#ifdef NT35510_GED_ORG
static int lcd_get_power(struct lcd_device *ld)
{
#if 0
	struct lcd_info *lcd = lcd_get_data(ld);

	return lcd->power;
#endif
	return true;
}


static struct lcd_ops nt35510_lcd_ops = {
	.set_power = lcd_set_power,
	.get_power = lcd_get_power,
};

#else /* NT35510_GED_ORG */
static int nt35510_power_on(struct lcd_info *lcd)
{
	int ret = 0;

	dev_info(&lcd->ld->dev, "%s\n", __func__);

	ret = nt35510_panel_resume();

	return ret;
}

static int nt35510_power_off(struct lcd_info *lcd)
{
	int ret = 0;

	dev_info(&lcd->ld->dev, "%s\n", __func__);

	ret = nt35510_panel_suspend();

	msleep(135);

	return ret;
}

static int nt35510_power(struct lcd_info *lcd, int power)
{
	int ret = 0;

	if (POWER_IS_ON(power) && !POWER_IS_ON(lcd->power))
		ret = nt35510_power_on(lcd);
	else if (!POWER_IS_ON(power) && POWER_IS_ON(lcd->power))
		ret = nt35510_power_off(lcd);

	if (!ret)
		lcd->power = power;

	return ret;
}

static int nt35510_set_power(struct lcd_device *ld, int power)
{
	struct lcd_info *lcd = lcd_get_data(ld);

	if (power != FB_BLANK_UNBLANK && power != FB_BLANK_POWERDOWN &&
		power != FB_BLANK_NORMAL) {
		dev_err(&lcd->ld->dev, "power value should be 0, 1 or 4.\n");
		return -EINVAL;
	}

	return nt35510_power(lcd, power);
}

static int nt35510_get_power(struct lcd_device *ld)
{
	struct lcd_info *lcd = lcd_get_data(ld);

	return lcd->power;
}

static struct lcd_ops nt35510_lcd_ops = {
	.set_power = nt35510_set_power,
	.get_power = nt35510_get_power,
};

#endif /* NT35510_GED_ORG */


int panel_dsi_read(int id, int reg, int len, char *buf)
{
	void *screen_handle;
	screen_disp_write_dsi_short write_dsi_s;
	screen_disp_read_dsi_short read_dsi_s;
	screen_disp_delete disp_delete;
	int ret = 0;

	printk(KERN_DEBUG "%s\n", __func__);

	if (!is_dsi_read_enabled) {
		printk(KERN_ALERT "sequence error!!\n");
		return -EINVAL;
	}

	if ((len <= 0) || (len > 60) || (buf == NULL)) {
		printk(KERN_ALERT "argument error!!\n");
		return -EINVAL;
	}

	screen_handle =  screen_display_new();

	/* Set maximum return packet size  */
	write_dsi_s.handle		= screen_handle;
	write_dsi_s.output_mode	= RT_DISPLAY_LCD1;
	write_dsi_s.data_id		= MIPI_DSI_SET_MAX_RETURN_PACKET;
	write_dsi_s.reg_address	= len;
	write_dsi_s.write_data		= 0x00;
	write_dsi_s.reception_mode	= RT_DISPLAY_RECEPTION_ON;
	ret = screen_display_write_dsi_short_packet(&write_dsi_s);
	if (ret != SMAP_LIB_DISPLAY_OK) {
		printk(KERN_ALERT "disp_write_dsi_short err!\n");
		goto out;
	}

	/* DSI read */
	read_dsi_s.handle		= screen_handle;
	read_dsi_s.output_mode		= RT_DISPLAY_LCD1;
	read_dsi_s.data_id		= id;
	read_dsi_s.reg_address		= reg;
	read_dsi_s.write_data		= 0;
	read_dsi_s.data_count		= len;
	read_dsi_s.read_data		= &buf[0];
	ret = screen_display_read_dsi_short_packet(&read_dsi_s);
	if (ret != SMAP_LIB_DISPLAY_OK)
		printk(KERN_ALERT "disp_dsi_read err! ret = %d\n", ret);

out:
	disp_delete.handle = screen_handle;
	screen_display_delete(&disp_delete);

	return ret;
}

static int nt35510_panel_draw_black(void *screen_handle)
{
	u32 panel_width  = R_MOBILE_M_PANEL_PIXEL_WIDTH;
	u32 panel_height = R_MOBILE_M_PANEL_PIXEL_HEIGHT;
	screen_disp_draw disp_draw;
	int ret;

	printk(KERN_DEBUG "%s\n", __func__);

#ifdef NT35510_DRAW_BLACK_KERNEL
	printk(KERN_DEBUG
		"num_registered_fb = %d\n", num_registered_fb);

	if (!num_registered_fb) {
		printk(KERN_ALERT
			"num_registered_fb err!\n");
		return -1;
	}
	if (!registered_fb[0]->fix.smem_start) {
		printk(KERN_ALERT
			"registered_fb[0]->fix.smem_start"
			" is NULL err!\n");
		return -1;
	}
	printk(KERN_DEBUG
	       "registerd_fb[0]-> fix.smem_start: %08x\n"
	       "screen_base :%08x\n"
	       "fix.smem_len :%08x\n",
	       (unsigned)(registered_fb[0]->fix.smem_start),
	       (unsigned)(registered_fb[0]->screen_base),
	       (unsigned)(registered_fb[0]->fix.smem_len));
	memset(registered_fb[0]->screen_base, 0x0,
			registered_fb[0]->fix.smem_len);
#endif

	/* Memory clean */
	disp_draw.handle = screen_handle;
#ifdef NT35510_DRAW_BLACK_KERNEL
	disp_draw.output_mode = RT_DISPLAY_LCD1;
	disp_draw.buffer_id   = RT_DISPLAY_BUFFER_A;
#else
	disp_draw.output_mode = RT_DISPLAY_LCD1_ASYNC;
	disp_draw.buffer_id   = RT_DISPLAY_DRAW_BLACK;
#endif
	disp_draw.draw_rect.x = 0;
	disp_draw.draw_rect.y = 0;
	disp_draw.draw_rect.width  = panel_width;
	disp_draw.draw_rect.height = panel_height;
#ifdef CONFIG_FB_SH_MOBILE_RGB888
	disp_draw.format = RT_DISPLAY_FORMAT_RGB888;
#else
	disp_draw.format = RT_DISPLAY_FORMAT_ARGB8888;
#endif
	disp_draw.buffer_offset = 0;
	disp_draw.rotate = RT_DISPLAY_ROTATE_270;
	ret = screen_display_draw(&disp_draw);
	if (ret != SMAP_LIB_DISPLAY_OK) {
		printk(KERN_ALERT "screen_display_draw err!\n");
		return -1;
	}

	return 0;
}

static int panel_specific_cmdset(void *lcd_handle,
				   const struct specific_cmdset *cmdset)
{
	int ret;
	int loop = 0;
	screen_disp_write_dsi_short write_dsi_s;
	screen_disp_write_dsi_long  write_dsi_l;

	printk(KERN_DEBUG "%s\n", __func__);

	while (0 <= loop) {
		switch (cmdset[loop].cmd) {
		case MIPI_DSI_DCS_LONG_WRITE:
		case MIPI_DSI_GEN_LONG_WRITE:
			printk(KERN_DEBUG "panel_cmdset LONG Write\n");
			write_dsi_l.handle         = lcd_handle;
			write_dsi_l.output_mode    = RT_DISPLAY_LCD1;
			write_dsi_l.data_id        = cmdset[loop].cmd;
			write_dsi_l.data_count     = cmdset[loop].size;
			write_dsi_l.write_data     = cmdset[loop].data;
			write_dsi_l.reception_mode = RT_DISPLAY_RECEPTION_ON;
			write_dsi_l.send_mode      = RT_DISPLAY_SEND_MODE_HS;
			ret = screen_display_write_dsi_long_packet(
					&write_dsi_l);
			if (ret != SMAP_LIB_DISPLAY_OK) {
				printk(KERN_ALERT
				       "display_write_dsi_long err %d!\n", ret);
				return -1;
			}
			break;
		case MIPI_DSI_DCS_SHORT_WRITE_PARAM:
		case MIPI_DSI_GEN_SHORT_WRITE_PARAM:
			printk(KERN_DEBUG
			       "panel_cmdset SHORT Write with param\n");
			write_dsi_s.handle         = lcd_handle;
			write_dsi_s.output_mode    = RT_DISPLAY_LCD1;
			write_dsi_s.data_id        = cmdset[loop].cmd;
			write_dsi_s.reg_address    = cmdset[loop].data[0];
			write_dsi_s.write_data     = cmdset[loop].data[1];
			write_dsi_s.reception_mode = RT_DISPLAY_RECEPTION_ON;
			ret = screen_display_write_dsi_short_packet(
					&write_dsi_s);
			if (ret != SMAP_LIB_DISPLAY_OK) {
				printk(KERN_ALERT
				       "disp_write_dsi_short err %d!\n", ret);
				return -1;
			}
			break;
		case MIPI_DSI_DCS_SHORT_WRITE:
		case MIPI_DSI_GEN_SHORT_WRITE:
			printk(KERN_DEBUG "panel_cmdset SHORT Write\n");
			write_dsi_s.handle         = lcd_handle;
			write_dsi_s.output_mode    = RT_DISPLAY_LCD1;
			write_dsi_s.data_id        = cmdset[loop].cmd;
			write_dsi_s.reg_address    = cmdset[loop].data[0];
			write_dsi_s.write_data     = 0x00;
			write_dsi_s.reception_mode = RT_DISPLAY_RECEPTION_ON;
			ret = screen_display_write_dsi_short_packet(
					&write_dsi_s);
			if (ret != SMAP_LIB_DISPLAY_OK) {
				printk(KERN_ALERT
				       "disp_write_dsi_short err %d!\n", ret);
				return -1;
			}
			break;
		case MIPI_DSI_BLACK:
		{
			ret = nt35510_panel_draw_black(lcd_handle);
			if (ret != 0)
				return -1;

			break;
		}
		case MIPI_DSI_DELAY:
			msleep(cmdset[loop].size);
			break;

		case MIPI_DSI_END:
			loop = -2;
			break;
		default:
			printk(KERN_ALERT "Undefine command err!\n");
			return -1;
		}
		loop++;
	}

	return 0;
}

static void mipi_display_reset(void)
{
	printk(KERN_INFO "%s\n", __func__);

	/* GPIO control */
	gpio_request(reset_gpio, NULL);

	regulator_enable(power_ldo_3v);
	regulator_enable(power_ldo_1v8);

	gpio_direction_output(reset_gpio, 0);

	msleep(20);

	gpio_direction_output(reset_gpio, 1);

	msleep(50);
}


static int nt35510_panel_init(unsigned int mem_size)
{
	void *screen_handle;
	screen_disp_start_lcd start_lcd;
	screen_disp_stop_lcd disp_stop_lcd;
	screen_disp_set_lcd_if_param set_lcd_if_param;
	screen_disp_set_address set_address;
	screen_disp_delete disp_delete;
	unsigned char read_data[60];
	int ret = 0;
	int retry_count = NT35510_INIT_RETRY_COUNT;

#ifdef NT35510_POWAREA_MNG_ENABLE
	void *system_handle;
	system_pmg_param powarea_start_notify;
	system_pmg_delete pmg_delete;
#endif

	printk(KERN_INFO "%s\n", __func__);

#ifdef NT35510_POWAREA_MNG_ENABLE
	printk(KERN_INFO "Start A4LC power area\n");
	system_handle = system_pwmng_new();

	/* Notifying the Beginning of Using Power Area */
	powarea_start_notify.handle		= system_handle;
	powarea_start_notify.powerarea_name	= RT_PWMNG_POWERAREA_A4LC;
	ret = system_pwmng_powerarea_start_notify(&powarea_start_notify);
	if (ret != SMAP_LIB_PWMNG_OK)
		printk(KERN_ALERT "system_pwmng_powerarea_start_notify err!\n");
	pmg_delete.handle = system_handle;
	system_pwmng_delete(&pmg_delete);
#endif

retry:
	screen_handle =  screen_display_new();

	/* LCD panel reset */
	mipi_display_reset();

	/* Setting peculiar to panel */
	set_lcd_if_param.handle			= screen_handle;
	set_lcd_if_param.port_no		= irq_portno;
	set_lcd_if_param.lcd_if_param		= &r_mobile_lcd_if_param;
	set_lcd_if_param.lcd_if_param_mask	= &r_mobile_lcd_if_param_mask;
	ret = screen_display_set_lcd_if_parameters(&set_lcd_if_param);
	if (ret != SMAP_LIB_DISPLAY_OK) {
		printk(KERN_ALERT "disp_set_lcd_if_parameters err!\n");
		goto out;
	}

	/* Setting FB address */
	set_address.handle	= screen_handle;
	set_address.output_mode	= RT_DISPLAY_LCD1;
	set_address.buffer_id	= RT_DISPLAY_BUFFER_A;
	set_address.address	= R_MOBILE_M_BUFF_ADDR;
	set_address.size	= R_MOBILE_M_BUFF_SIZE;
	ret = screen_display_set_address(&set_address);
	if (ret != SMAP_LIB_DISPLAY_OK) {
		printk(KERN_ALERT "disp_set_address err!\n");
		goto out;
	}

	/* Start a display to LCD */
	start_lcd.handle	= screen_handle;
	start_lcd.output_mode	= RT_DISPLAY_LCD1;
	ret = screen_display_start_lcd(&start_lcd);
	if (ret != SMAP_LIB_DISPLAY_OK) {
		printk(KERN_ALERT "disp_start_lcd err!\n");
		goto out;
	}
	is_dsi_read_enabled = 1;

	/* Read display identification information */
	ret = panel_dsi_read(MIPI_DSI_DCS_READ, 0x04, 4, &read_data[0]);
	if (ret == 0) {
		printk(KERN_DEBUG "read_data(RDID1) = %02X\n", read_data[1]);
		printk(KERN_DEBUG "read_data(RDID2) = %02X\n", read_data[2]);
		printk(KERN_DEBUG "read_data(RDID3) = %02X\n", read_data[3]);
	}

	/* Transmit DSI command peculiar to a panel */
	ret = panel_specific_cmdset(screen_handle, initialize_cmdset);
	if (ret != 0) {
		printk(KERN_ALERT "panel_specific_cmdset err!\n");
		is_dsi_read_enabled = 0;

		if (retry_count == 0) {
			printk(KERN_ALERT "retry count 0!!!!\n");
			nt35510_panel_draw_black(screen_handle);
			ret = -ENODEV;
			goto out;
		} else {
			retry_count--;

			/* Stop a display to LCD */
			disp_stop_lcd.handle		= screen_handle;
			disp_stop_lcd.output_mode	= RT_DISPLAY_LCD1;
			ret = screen_display_stop_lcd(&disp_stop_lcd);
			if (ret != SMAP_LIB_DISPLAY_OK) {
				printk(KERN_ALERT "display_stop_lcd err!\n");
				goto out;
			}

			disp_delete.handle = screen_handle;
			screen_display_delete(&disp_delete);
			goto retry;
		}
	}

#ifdef NT35510_ENABLE_VIDEO_MODE
	is_dsi_read_enabled = 0;

	/* Stop a display to LCD */
	disp_stop_lcd.handle		= screen_handle;
	disp_stop_lcd.output_mode	= RT_DISPLAY_LCD1;
	ret = screen_display_stop_lcd(&disp_stop_lcd);
	if (ret != SMAP_LIB_DISPLAY_OK) {
		printk(KERN_ALERT "display_stop_lcd err!\n");
		goto out;
	}
	/* Start a display to LCD */
	start_lcd.handle	= screen_handle;
	start_lcd.output_mode	= RT_DISPLAY_LCD1;
	ret = screen_display_start_lcd(&start_lcd);
	if (ret != SMAP_LIB_DISPLAY_OK) {
		printk(KERN_ALERT "disp_start_lcd err!\n");
		goto out;
	}

	is_dsi_read_enabled = 1;

	/* Display ON */
	ret = panel_specific_cmdset(screen_handle, initialize_cmdset_phase2);
	if (ret != 0) {
		printk(KERN_ALERT "panel_specific_cmdset err!\n");
		is_dsi_read_enabled = 0;
		if (retry_count == 0) {
			printk(KERN_ALERT "retry count 0!!!!\n");
			nt35510_panel_draw_black(screen_handle);
			ret = -ENODEV;
			goto out;
		} else {
			retry_count--;

			/* Stop a display to LCD */
			disp_stop_lcd.handle		= screen_handle;
			disp_stop_lcd.output_mode	= RT_DISPLAY_LCD1;
			ret = screen_display_stop_lcd(&disp_stop_lcd);
			if (ret != SMAP_LIB_DISPLAY_OK) {
				printk(KERN_ALERT "display_stop_lcd err!\n");
				goto out;
			}

			disp_delete.handle = screen_handle;
			screen_display_delete(&disp_delete);
			goto retry;
		}
	}
	printk(KERN_DEBUG "Panel initialized with Video mode\n");
#else /* NT35510_ENABLE_VIDEO_MODE */
	printk(KERN_DEBUG "Panel initialized with Command mode\n");
#endif /* NT35510_ENABLE_VIDEO_MODE */

#if defined(NT35510_ESD_RECOVERY_ENABLE)
	esd_check_flag = ESD_CHECK_ENABLE;
	schedule_delayed_work(&esd_check_work, msecs_to_jiffies(DURATION_TIME));
#endif /* NT35510_ESD_RECOVERY_ENABLE */

out:
	disp_delete.handle = screen_handle;
	screen_display_delete(&disp_delete);

	return ret;
}

static int nt35510_panel_suspend(void)
{
	void *screen_handle;
	screen_disp_stop_lcd disp_stop_lcd;
	screen_disp_start_lcd start_lcd;
	screen_disp_delete disp_delete;
	int ret;

#ifdef NT35510_POWAREA_MNG_ENABLE
	void *system_handle;
	system_pmg_param powarea_end_notify;
	system_pmg_delete pmg_delete;
#endif

#if defined(NT35510_ESD_RECOVERY_ENABLE)
	esd_check_flag = ESD_CHECK_DISABLE;
	mutex_lock(&esd_check_mutex);
	/* Cancel scheduled work queue for check ESD. */
	cancel_delayed_work_sync(&esd_check_work);
#endif /* NT35510_ESD_RECOVERY_ENABLE */

	printk(KERN_INFO "%s\n", __func__);

	screen_handle =  screen_display_new();

	is_dsi_read_enabled = 0;

	/* Stop a display to LCD */
	disp_stop_lcd.handle		= screen_handle;
	disp_stop_lcd.output_mode	= RT_DISPLAY_LCD1;
	ret = screen_display_stop_lcd(&disp_stop_lcd);
	if (ret != SMAP_LIB_DISPLAY_OK)
		printk(KERN_ALERT "display_stop_lcd err!\n");

	start_lcd.handle	= screen_handle;
	start_lcd.output_mode	= RT_DISPLAY_LCD1;
	ret = screen_display_start_lcd(&start_lcd);
	if (ret != SMAP_LIB_DISPLAY_OK)
		printk(KERN_ALERT "disp_start_lcd err!\n");

	/* Transmit DSI command peculiar to a panel */
	ret = panel_specific_cmdset(screen_handle, demise_cmdset);
	if (ret != 0) {
		printk(KERN_ALERT "panel_specific_cmdset err!\n");
		/* continue */
	}

	/* Stop a display to LCD */
	disp_stop_lcd.handle		= screen_handle;
	disp_stop_lcd.output_mode	= RT_DISPLAY_LCD1;
	ret = screen_display_stop_lcd(&disp_stop_lcd);
	if (ret != SMAP_LIB_DISPLAY_OK)
		printk(KERN_ALERT "display_stop_lcd err!\n");

	/* GPIO control */
	gpio_direction_output(reset_gpio, 0);
	msleep(20);
	regulator_disable(power_ldo_3v);
	regulator_disable(power_ldo_1v8);
	msleep(25);

	disp_delete.handle = screen_handle;
	screen_display_delete(&disp_delete);

#ifdef NT35510_POWAREA_MNG_ENABLE
	printk(KERN_INFO "End A4LC power area\n");
	system_handle = system_pwmng_new();

	/* Notifying the Beginning of Using Power Area */
	powarea_end_notify.handle		= system_handle;
	powarea_end_notify.powerarea_name	= RT_PWMNG_POWERAREA_A4LC;
	ret = system_pwmng_powerarea_end_notify(&powarea_end_notify);
	if (ret != SMAP_LIB_PWMNG_OK) {
		printk(KERN_ALERT "system_pwmng_powerarea_end_notify err!\n");
		pmg_delete.handle = system_handle;
		system_pwmng_delete(&pmg_delete);
		return -1;
	}

	pmg_delete.handle = system_handle;
	system_pwmng_delete(&pmg_delete);
#endif

#if defined(NT35510_ESD_RECOVERY_ENABLE)
	mutex_unlock(&esd_check_mutex);
#endif /* NT35510_ESD_RECOVERY_ENABLE */


	return 0;
}

static int nt35510_panel_resume(void)
{
	void *screen_handle;
	screen_disp_start_lcd start_lcd;
	screen_disp_stop_lcd disp_stop_lcd;
	screen_disp_delete disp_delete;
	int ret = 0;
	int retry_count = NT35510_INIT_RETRY_COUNT;

#ifdef NT35510_POWAREA_MNG_ENABLE
	void *system_handle;
	system_pmg_param powarea_start_notify;
	system_pmg_delete pmg_delete;
#endif

#if defined(NT35510_ESD_RECOVERY_ENABLE)
	/* Wait for end of check ESD */
	mutex_lock(&esd_check_mutex);
#endif /* NT35510_ESD_RECOVERY_ENABLE */

	printk(KERN_INFO "%s\n", __func__);

#ifdef NT35510_POWAREA_MNG_ENABLE
	printk(KERN_INFO "Start A4LC power area\n");
	system_handle = system_pwmng_new();

	/* Notifying the Beginning of Using Power Area */
	powarea_start_notify.handle		= system_handle;
	powarea_start_notify.powerarea_name	= RT_PWMNG_POWERAREA_A4LC;
	ret = system_pwmng_powerarea_start_notify(&powarea_start_notify);
	if (ret != SMAP_LIB_PWMNG_OK)
		printk(KERN_ALERT "system_pwmng_powerarea_start_notify err!\n");
	pmg_delete.handle = system_handle;
	system_pwmng_delete(&pmg_delete);
#endif

retry:
	screen_handle =  screen_display_new();

	/* LCD panel reset */
	mipi_display_reset();

	/* Start a display to LCD */
	start_lcd.handle	= screen_handle;
	start_lcd.output_mode	= RT_DISPLAY_LCD1;
	ret = screen_display_start_lcd(&start_lcd);
	if (ret != SMAP_LIB_DISPLAY_OK) {
		printk(KERN_ALERT "disp_start_lcd err!\n");
		goto out;
	}
	is_dsi_read_enabled = 1;

	/* Transmit DSI command peculiar to a panel */
	ret = panel_specific_cmdset(screen_handle, initialize_cmdset);
	if (ret != 0) {
		printk(KERN_ALERT "panel_specific_cmdset err!\n");
		is_dsi_read_enabled = 0;
		if (retry_count == 0) {
			printk(KERN_ALERT "retry count 0!!!!\n");
			nt35510_panel_draw_black(screen_handle);
			ret = -ENODEV;
			goto out;
		} else {
			retry_count--;

			/* Stop a display to LCD */
			disp_stop_lcd.handle		= screen_handle;
			disp_stop_lcd.output_mode	= RT_DISPLAY_LCD1;
			ret = screen_display_stop_lcd(&disp_stop_lcd);
			if (ret != SMAP_LIB_DISPLAY_OK)
				printk(KERN_ALERT "display_stop_lcd err!\n");

			disp_delete.handle = screen_handle;
			screen_display_delete(&disp_delete);
			goto retry;
		}
	}

	/* Resume frame rate */
	lcdfreq_resume();

#ifdef NT35510_ENABLE_VIDEO_MODE
	is_dsi_read_enabled = 0;

	/* Stop a display to LCD */
	disp_stop_lcd.handle		= screen_handle;
	disp_stop_lcd.output_mode	= RT_DISPLAY_LCD1;
	ret = screen_display_stop_lcd(&disp_stop_lcd);
	if (ret != SMAP_LIB_DISPLAY_OK)
		printk(KERN_ALERT "display_stop_lcd err!\n");

	/* Start a display to LCD */
	start_lcd.handle	= screen_handle;
	start_lcd.output_mode	= RT_DISPLAY_LCD1;
	ret = screen_display_start_lcd(&start_lcd);
	if (ret != SMAP_LIB_DISPLAY_OK)
		printk(KERN_ALERT "disp_start_lcd err!\n");

	is_dsi_read_enabled = 1;

	/* Display ON */
	ret = panel_specific_cmdset(screen_handle, initialize_cmdset_phase2);
	if (ret != 0) {
		printk(KERN_ALERT "panel_specific_cmdset err!\n");
		is_dsi_read_enabled = 0;
		if (retry_count == 0) {
			printk(KERN_ALERT "retry count 0!!!!\n");
			nt35510_panel_draw_black(screen_handle);
			goto out;
		} else {
			retry_count--;

			/* Stop a display to LCD */
			disp_stop_lcd.handle		= screen_handle;
			disp_stop_lcd.output_mode	= RT_DISPLAY_LCD1;
			ret = screen_display_stop_lcd(&disp_stop_lcd);
			if (ret != SMAP_LIB_DISPLAY_OK)
				printk(KERN_ALERT "display_stop_lcd err!\n");

			disp_delete.handle = screen_handle;
			screen_display_delete(&disp_delete);
			goto retry;
		}
	}
#endif /* NT35510_ENABLE_VIDEO_MODE */

#if defined(NT35510_ESD_RECOVERY_ENABLE)
	/* Schedule check ESD */
	esd_check_flag = ESD_CHECK_ENABLE;
	mutex_unlock(&esd_check_mutex);
	schedule_delayed_work(&esd_check_work, msecs_to_jiffies(DURATION_TIME));
#endif /* NT35510_ESD_RECOVERY_ENABLE */

out:
	disp_delete.handle = screen_handle;
	screen_display_delete(&disp_delete);

	return ret;
}

static int nt35510_panel_probe(struct fb_info *info,
			    struct fb_panel_hw_info hw_info)
{
	struct platform_device *pdev;
	struct resource *res_irq_port;
	int ret;

	printk(KERN_INFO "%s\n", __func__);

	reset_gpio = hw_info.gpio_reg;

	/* fb parent device info to platform_device */
	pdev = to_platform_device(info->device);

	/* get resource info from platform_device */
	res_irq_port	= platform_get_resource_byname(pdev,
							IORESOURCE_MEM,
							"panel_irq_port");
	if (!res_irq_port) {
		printk(KERN_ALERT "panel_irq_port is NULL!!\n");
		return -ENODEV;
	}
	irq_portno = res_irq_port->start;
	power_ldo_3v = regulator_get(NULL, "vlcd_3v");
	power_ldo_1v8 = regulator_get(NULL, "vlcd_1v8");

	if (power_ldo_3v == NULL || power_ldo_1v8 == NULL) {
		printk(KERN_ERR "regulator_get failed\n");
		return -ENODEV;
	}
	printk(KERN_INFO "PMIC        : for panel power\n");
	printk(KERN_INFO "GPIO_PORT%d : for panel reset\n", reset_gpio);
	printk(KERN_INFO "IRQ%d       : for panel te\n", irq_portno);

	common_fb_info = info;
	is_dsi_read_enabled = 0;

	/* clear internal info */
	memset(&lcd_info_data, 0, sizeof(lcd_info_data));

	/* register sysfs for LCD */
	lcd_info_data.ld = lcd_device_register("panel",
						&pdev->dev,
						&lcd_info_data,
						&nt35510_lcd_ops);
	if (IS_ERR(lcd_info_data.ld))
		return PTR_ERR(lcd_info_data.ld);

	/* register sysfs for LCD frequency control */
	ret = nt35510_lcd_frequency_register(info->device);
	if (ret < 0)
		goto out;

	lcd_info_data.power = FB_BLANK_UNBLANK;

#if defined(NT35510_ESD_RECOVERY_ENABLE)
	esd_check_flag = ESD_CHECK_DISABLE;
	mutex_init(&esd_check_mutex);
	INIT_DELAYED_WORK(&esd_check_work,
		nt35510_panel_esd_check_work);
#endif

	return 0;

out:
	/* unregister sysfs for LCD */
	lcd_device_unregister(lcd_info_data.ld);

	return ret;
}

static int nt35510_panel_remove(struct fb_info *info)
{
	printk(KERN_INFO "%s\n", __func__);

#if defined(NT35510_ESD_RECOVERY_ENABLE)
	/* Wait for end of check to power mode state */
	mutex_lock(&esd_check_mutex);

	/* Cancel  scheduled work queue for check to power mode state. */
	cancel_delayed_work_sync(&esd_check_work);

	mutex_unlock(&esd_check_mutex);
#endif

	/* unregister sysfs for LCD frequency control */
	nt35510_lcd_frequency_unregister();

	/* unregister sysfs for LCD */
	lcd_device_unregister(lcd_info_data.ld);

	return 0;
}

static struct fb_panel_info nt35510_panel_info(void)
{
	printk(KERN_INFO "%s\n", __func__);

	return r_mobile_info;
}

struct fb_panel_func r_mobile_panel_func(int panel)
{

	struct fb_panel_func panel_func;

	printk(KERN_INFO "%s\n", __func__);

	memset(&panel_func, 0, sizeof(struct fb_panel_func));

/* e.g. support (panel=RT_DISPLAY_LCD1) */

	if (panel == RT_DISPLAY_LCD1) {
		panel_func.panel_init    = nt35510_panel_init;
		panel_func.panel_suspend = nt35510_panel_suspend;
		panel_func.panel_resume  = nt35510_panel_resume;
		panel_func.panel_probe   = nt35510_panel_probe;
		panel_func.panel_remove  = nt35510_panel_remove;
		panel_func.panel_info    = nt35510_panel_info;
	}

	return panel_func;
}
