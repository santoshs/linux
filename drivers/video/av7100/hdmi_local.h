/*
 * /drivers/video/av7100/hdmi_local.h
 *
 * Copyright (C) 2012 Renesas Mobile Corporation. 
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
#ifndef __HDMI_LOCAL__H__
#define __HDMI_LOCAL__H__

#define EDID_BUF_LEN				128

#define HDMI_HDCP_AES_NR_OF_BLOCKS	18
#define HDMI_HDCP_AES_KEYSIZE		16
#define HDMI_HDCP_AES_BLOCK_START	128
#define HDMI_HDCP_AES_KSVZEROESSIZE	3
#define HDMI_HDCP_AES_KSVSIZE		5
#define HDMI_HDCP_KSV_BLOCK		40

#define EDID_GET_DBC_TYPE(__byte) 			(((__byte) >> 0x05) & 0x07)
#define EDID_GET_DBC_LENGTH(__byte) 		(__byte & 0x1F)
#define EDID_GET_VIDEO_TYPE(__byte) 		(__byte & 0x7F)
#define EDID_GET_PREFERRED_ATT(__byte) 	(__byte & 0x80)

#define EDID_AUDIO_TYPE 		0x01
#define EDID_VIDEO_TYPE 		0x02
#define EDID_VENDOR_TYPE 		0x03
#define EDID_SPEAKER_TYPE 		0x04

extern int hdmi_get_current_hpd(void);
extern int hdmi_hpd_handler(int hpd);

struct edid_read {
	u8 address;
	u8 block_nr;
	u8 data_length;
	u8 data[128];
};

struct info_fr {
	u8 type;
	u8 ver;
	u8 crc;
	u8 length;
	u8 data[27];
};

struct hdcp_loadaesall {
	u8 ksv[5];
	u8 key[288];
	u8 result;
};

struct hdcp_authencr {
	u8 auth_type;
	u8 encr_type;
};

struct edid_data {
	u8 buf_len;
	u8 buf[EDID_BUF_LEN];
};

struct hdmi_register {
	unsigned char value;
	unsigned char offset;
};

struct hdcp_loadaesone {
	u8 key[16];
	u8 result;
};

struct hdmi_data {
	atomic_t hpd_state;
	unsigned int info;
	unsigned gpio;
	int irq;
	int auth_flag;
	int device_open;
	struct switch_dev sdev;
	struct workqueue_struct *my_workq;
	struct work_struct gpio_work;
	struct work_struct hdmi_work;
	struct hdcp_loadaesall hdcp_loadaesall;
	struct device *hdmidev;
};

extern int hdmi_init(void);
extern void hdmi_exit(void);

#endif /* __HDMI_LOCAL_H__ */