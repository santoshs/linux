/*
 * /include/video/hdmi.h
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
#ifndef __HDMI__H__
#define __HDMI__H__
 
#define HDMI_IOC_MAGIC 		0xcc
#define IOC_AUTHENTICATE_HDCP		_IO(HDMI_IOC_MAGIC, 18)
#define IOC_DEAUTHENTICATE_HDCP	_IO(HDMI_IOC_MAGIC, 19)
#define IOC_SET_REVOCATION_LIST	_IOW(HDMI_IOC_MAGIC, 20, int)
#define IOC_SET_AESHDCP_KEY		_IOW(HDMI_IOC_MAGIC, 21, char[294])	

#endif 			/* __HDMI__H__ */