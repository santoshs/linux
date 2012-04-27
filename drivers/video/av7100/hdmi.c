/*
 * /drivers/video/av7100/hdmi.c
 *
 * Copyright (C) 2012 Renesas Mobile Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>
#include <linux/poll.h>
#include <linux/mutex.h>
#include <linux/ctype.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/clk.h>
#include <linux/switch.h>
#include <linux/gpio.h>
#include <linux/errno.h>
#include <linux/interrupt.h>

#include <mach/r8a73734.h>
#include <video/hdmi.h>

#include "av7100_regs.h"
#include "av7100.h"
#include "hdmi_local.h"

DEFINE_MUTEX(hdmi_events_mutex);
#define LOCK_HDMI_EVENTS mutex_lock(&hdmi_events_mutex)
#define UNLOCK_HDMI_EVENTS mutex_unlock(&hdmi_events_mutex)

struct hdmi_data *hdmi_dev;

/****************************************************************************
*	name	=	info_show
*	func	=	Read infor value of switch class
*	input	=	struct device *dev,
*			struct device_attribute *attr, char *buf
*	output	=	None
*	return	=	0
****************************************************************************/
static ssize_t info_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", hdmi_dev->info);
}
static DEVICE_ATTR(info, S_IRUGO | S_IWUSR, info_show, NULL);

/*****************************************************************************
 *	name	=	hdmi_get_current_hpd
 *	func	=	Get current HPD state from GPIO port
 *	input	=	None
 *	output	=	None
 *	return	=	0, 1
 *****************************************************************************/
int hdmi_get_current_hpd(void)
{
    return gpio_get_value(hdmi_dev->gpio);
}

/*****************************************************************************
 *	name	=	hdmi_hpd_handler
 *	func	=	Handle hot plug detection of HDMI
 *	input	=	int hpd
 *	output	=	None
 *	return	=	0
 *****************************************************************************/
int hdmi_hpd_handler(int hpd)
{
	atomic_set(&(hdmi_dev->hpd_state), 
							(hpd ? HPD_STATE_PLUGGED : HPD_STATE_UNPLUGGED));
	queue_work(hdmi_dev->my_workq, &hdmi_dev->gpio_work);
	return 0;
}

/*****************************************************************************
 *	name	=	hdcp_chkaesotp
 *	func	=	Check  AESOTP key has already programmed in AV7100
 *	input	=	u8 *progged
 *	output	=	u8 *progged
 *	return	=	0,	-EFAULT,	-EINVAL
 *****************************************************************************/
static int hdcp_chkaesotp(u8 *progged)
{
	union av7100_configuration config;
	u8 buf_len = 0;
	u8 buf[2];
	/*Set read operation to fuse_operation*/
	config.fuse_aes_key_format.fuse_operation = AV7100_FUSE_READ;
	memset(config.fuse_aes_key_format.key, 0, AV7100_FUSE_KEY_SIZE);
	
	if (av7100_conf_prep(AV7100_COMMAND_FUSE_AES_KEY, &config) != 0) {
		dev_err(hdmi_dev->hdmidev, "av7100_conf_prep FAIL\n");
		return -EFAULT;
	}
	
	/*Write new configuration to AV7100*/
	if (av7100_conf_w(AV7100_COMMAND_FUSE_AES_KEY, &buf_len, buf) != 0) {
		dev_err(hdmi_dev->hdmidev, "av7100_conf_w FAIL\n");
		return -EFAULT;
	}
	
	/*Check result of AESOTP key reading*/
	if (buf_len == 2) {
		if (progged)
			*progged = buf[1];	/*Set result of reading to progged*/
	} else {
		return -EFAULT;
	}
	return 0;
}
/*****************************************************************************
 *	name	=	hdcp_loadaes
 *	func	=	Load  HDCP keys have been encrypted by AES key
 *				from secured memory to AV7100 
 *	input	=	u8 block, u8 key_len, u8 *key
 *	output	=	u8 *result
 *	return	=	0,	-EFAULT,	-EINVALID
 *****************************************************************************/
static int hdcp_loadaes(u8 block, u8 key_len, u8 *key, u8 *result)
{
	union av7100_configuration config;
	if (!result || !key)
		return -EINVAL;
	
	/* Default result of loading is fail */
	*result = 1;
	
	/*Initialize values to hdcp_send_key_format*/
	config.hdcp_send_key_format.key_number = block;
	config.hdcp_send_key_format.data_len = key_len;
	memcpy(config.hdcp_send_key_format.data, key, key_len);
	
	/*Prepare to write write new configuration to AV710*/
	if (av7100_conf_prep(AV7100_COMMAND_HDCP_SENDKEY, &config) != 0) {
		dev_err(hdmi_dev->hdmidev, "av7100_conf_prep FAIL\n");
		return -EFAULT;
	}
	
	/*Write new configuration to AV7100*/
	if (av7100_conf_w(AV7100_COMMAND_HDCP_SENDKEY, NULL, NULL) != 0) {
		dev_err(hdmi_dev->hdmidev, "av7100_conf_w FAIL\n");
		return -EFAULT;
	}
	*result = 0; /*Result of loading is successful*/
	
	return 0;
}
/*****************************************************************************
 *	name	=	authenticate_hdcp
 *	func	=	Authenticate/De-authenticate HDCP
 *	input	=	u8 auth_type
 *	output	=	None
 *	return	=	0,	-EFAULT,	-ENODATA(-61)
 *****************************************************************************/
static int authenticate_hdcp(u8 auth_type)
{
	union av7100_configuration config;
	/*AES HDCP keys have not loaded to AV7100*/ 
	if (hdmi_dev->hdcp_loadaesall.result != 0) 
		return -ENODATA;
	
	if (auth_type == HDMI_HDCP_AUTH_ON) 
		config.hdcp_management_format.req_type = AV7100_HDCP_AUTH_REQ_ON;
	else
		config.hdcp_management_format.req_type = AV7100_HDCP_AUTH_REQ_OFF;
	
	/*Encrypted types: EESS(HDMI), OESS(DVI)*/
	config.hdcp_management_format.req_encr = AV7100_HDCP_ENCR_USE_EESS;
	
	/*Prepare to write write new configuration to AV710*/
	if (av7100_conf_prep(AV7100_COMMAND_HDCP_MANAGEMENT, &config) != 0) {
		dev_err(hdmi_dev->hdmidev, "av7100_conf_prep FAIL\n");
		return -EFAULT;
	}
	
	/*Write new configuration to AV7100*/
	if (av7100_conf_w(AV7100_COMMAND_HDCP_MANAGEMENT, NULL, NULL) != 0) {
		dev_err(hdmi_dev->hdmidev, "av7100_conf_w FAIL\n");
		return -EFAULT;
	}
	return 0;
}
/*****************************************************************************
 *	name	=	check_revoq_list
 *	func	=	Check Bksv of receiver exist in revocation list or not 
 *	input	=	u8 *revoq, u8 *bksv
 *	output	=	None
 *	return	=	0 : None exist	1 : Exist
 *****************************************************************************/
static int check_revoq_list(u8 *revoq, u8 *bksv)
{
	return 0;
}
/*****************************************************************************
 *	name	=	change_hdcp_status
 *	func	=	Control HDCP state
 *	input	=	u8 hdcp_state
 *	output	=	None
 *	return	=	0,	-EFAULT,	-ENODATA(-61)
 *****************************************************************************/
 static int change_hdcp_status(u8 hdcp_state)
 {
	union av7100_configuration config;
	int ret = 0;
	u8 buf_len = 0;
	u8 buf[6];
	char * revoq = NULL;
	
	/*Encrypted types: EESS(HDMI), OESS(DVI)*/
	config.hdcp_management_format.req_encr = AV7100_HDCP_ENCR_USE_EESS;
	switch (hdcp_state) {
	case HDMI_HDCP_NO_RECEIVER:
		printk(KERN_ALERT "No receiver state\n");
		break;
	case HDMI_HDCP_RECEIVER_CONNECTED:
		printk(KERN_ALERT "Receiver connected state\n");
		break;
	case HDMI_HDCP_NO_HDCP_RECEIVER:
		printk(KERN_ALERT "No receiver state\n");
		break;
	case HDMI_HDCP_NO_ENCRYPTION:
		printk(KERN_ALERT "No encryption\n");
		break;
	case HDMI_HDCP_AUTHENTICATION_ON_GOING:
		printk(KERN_ALERT "Authentication on going state\n");
		break;
	case HDMI_HDCP_AUTHENTICATION_FAIL:
		hdmi_dev->auth_flag = 0;
		ret = av7100_hdmi_config(AV7100_HDMI_OFF);
		if (ret != 0)
			return -EFAULT;
		printk(KERN_ALERT "Authentication fail state\n");
		break;
	case HDMI_HDCP_AUTHENTICATION_SUCCEED:
		/*Default authenticated result is fail*/
		hdmi_dev->auth_flag = 0;
		/*Get BKSV of sink*/
		config.hdcp_management_format.req_type = AV7100_HDCP_REV_LIST_REQ;
		if (av7100_conf_prep(AV7100_COMMAND_HDCP_MANAGEMENT, &config) != 0) {
			dev_err(hdmi_dev->hdmidev, "av7100_conf_prep FAIL\n");
			return -EFAULT;
		}
		if (av7100_conf_w(AV7100_COMMAND_HDCP_MANAGEMENT,
			&buf_len, &buf[0]) != 0) {
			dev_err(hdmi_dev->hdmidev, "av7100_conf_w FAIL\n");
			return -EFAULT;
		}
		/*Check revocation list. This function is not defined*/
		if (check_revoq_list(revoq,&buf[0]) == 1) {
			av7100_hdmi_config(AV7100_HDMI_OFF);
			return -EFAULT;
		}
		config.hdcp_management_format.req_type =
			AV7100_HDCP_AUTH_CONT;
		if (av7100_conf_prep(AV7100_COMMAND_HDCP_MANAGEMENT, &config) != 0) {
			dev_err(hdmi_dev->hdmidev, "av7100_conf_prep FAIL\n");
			return -EFAULT;
		}
		if (av7100_conf_w(AV7100_COMMAND_HDCP_MANAGEMENT, NULL, NULL) != 0) {
			dev_err(hdmi_dev->hdmidev, "av7100_conf_w FAIL\n");
			return -EFAULT;
		}
		hdmi_dev->auth_flag = 1;
		printk(KERN_ALERT "Authentication is successful\n");
		break;
	case HDMI_HDCP_ENCRYPTION_ON_GOING:
		printk(KERN_ALERT "Encryption on going state\n");
		break;
	default:
		printk(KERN_ALERT "Input argument of HDCP state is wrong\n");
		ret = -EFAULT;
		break;
	}
	return ret;
}

/*****************************************************************************
 *	name	=	edidread
 *	func	=	Read EDID information from HDMI panel via DDC
 *	input	=	int block_nr, u8 *data
 *	output	=	u8 *data
 *	return	=	0, -EINVAL, -EFAULT
 *****************************************************************************/
static int edidread(int block_nr, u8 *data)
{
	union av7100_configuration config;
	u8 buf_len = 0;
	int retval = 0;
	
	if (!data)
		return -EINVAL;
	
	/* Block number is validated from 0 to 255 */
	if ((block_nr < 0) || (block_nr > 255))
		return -EINVAL;
	
	/* Device address 0xA0 for DDC */
	config.edid_section_readback_format.address = 0xA0;
	config.edid_section_readback_format.block_number = block_nr;

	retval = av7100_conf_prep(AV7100_COMMAND_EDID_SECTION_READBACK, &config);
	if (retval)
		return -EFAULT;

	retval = av7100_conf_w(AV7100_COMMAND_EDID_SECTION_READBACK,
						   &buf_len, data);
	if (retval)
		return -EFAULT;
	
	/* EDID data at 128th byte is checksum. 
	 * Sum of all 128 bytes should equal 0 (mod 256) */
	if ((buf_len == 0) || (data[127] != 0))
		return -EINVAL;
	
	return 0;
}

/*****************************************************************************
 *	name	=	edid_video_type
 *	func	=	Get video supported type from EDID information
 *	input	=	None
 *	output	=	None
 *	return	=	video supported type
*****************************************************************************/
 static int edid_video_type(void)
{
	int block = 0;					/* Block number */
	int dt_descriptor = 0;			/* Start of Detailed Timing Descriptor */
	int next_blk_dc = 0;
	
	int blk_dc_type = 0;			/* Block Data Collection type */
	int blk_dc_length = 0;			/* length of each type */
	u8 current_index = 0x04;		/* Start of Block Data Collection */
	u8 prefered_index = 0x00;		/* Start of Video block type */
	
	int ret = 0;
	u8 data[128];					/* One EDID data block is 128 bytes */
	
	/* Get VESA block */
	ret = edidread(block, data);
	if (ret)
		return -EFAULT;

	if (data[126] == 0) {
		return AV7100_CEA2_3_720X480P_60HZ;
	} else {
		block = 1;
		
		/* Get CEA block */
		ret = edidread(block, data);
		if (ret)
			return -EFAULT;
		
		/* Get address of detailed timing descriptor */
		dt_descriptor = data[2];
		if (dt_descriptor == 0x00 || dt_descriptor == 0x04)
			return AV7100_CEA2_3_720X480P_60HZ;
		
		while (current_index < dt_descriptor) {
			/* Get type of each data block collection */
			blk_dc_type = EDID_GET_DBC_TYPE(data[current_index]);
			/* Get length of each data block collection */
			blk_dc_length = EDID_GET_DBC_LENGTH(data[current_index]);
			
			switch (blk_dc_type) {
			case EDID_AUDIO_TYPE:
			case EDID_SPEAKER_TYPE:
			case EDID_VENDOR_TYPE:
				/* jump to next data block collection */
				current_index = current_index + blk_dc_length + 1;
				break;
			case EDID_VIDEO_TYPE:
				{
					/* update index to next_blk_dc */
					next_blk_dc = current_index + blk_dc_length + 1;
					/* current index of video supported type */
					current_index++;
					
					/* check preferred attribute of video type */
					prefered_index = current_index;
					for (; prefered_index < next_blk_dc; prefered_index++) {
						if (EDID_GET_PREFERRED_ATT(data[prefered_index]) == 1) {
							switch (EDID_GET_VIDEO_TYPE(data[prefered_index])) {
							case 2:
							case 3:
								return AV7100_CEA2_3_720X480P_60HZ;
								break;
							case 4:
								return AV7100_CEA4_1280X720P_60HZ;
								break;
							case 16:
								return AV7100_CEA16_1920X1080P_60HZ;
								break;
							case 17:
							case 18:
								return AV7100_CEA17_18_720X576P_50HZ;
								break;
							case 19:
								return AV7100_CEA19_1280X720P_50HZ;
								break;
							case 31:
								return AV7100_CEA31_1920x1080P_50Hz;
								break;
							case 34:
								return AV7100_CEA34_1920X1080P_30HZ;
								break;
							default:
								break;
							}	/* end switch */
						}	/* end if */
					}	/* end for */
					
					for (; current_index < next_blk_dc; current_index++) {
						switch (EDID_GET_VIDEO_TYPE(data[current_index])) {
						case 2:
						case 3:
							return AV7100_CEA2_3_720X480P_60HZ;
							break;
						case 4:
							return AV7100_CEA4_1280X720P_60HZ;
							break;
						case 16:
							return AV7100_CEA16_1920X1080P_60HZ;
							break;
						case 17:
						case 18:
							return AV7100_CEA17_18_720X576P_50HZ;
							break;
						case 19:
							return AV7100_CEA19_1280X720P_50HZ;
							break;
						case 31:
							return AV7100_CEA31_1920x1080P_50Hz;
							break;
						case 34:
							return AV7100_CEA34_1920X1080P_30HZ;
							break;
						default:
							break;
						}	/* end switch */
					}	/* end for */
					return AV7100_CEA2_3_720X480P_60HZ;
				}	/* end case */
			default:
				break;
			}
		}	/* end while */
	}
	return AV7100_CEA2_3_720X480P_60HZ;
}

/*************************************************************************
 *	name	=	down_load
 *	func	=	Download firmware and initialize HDMI video settings for AV7100
 *	input	=	None
 *	output	=	None
 *	return	=	0,	-EFAULT
 *************************************************************************/
int down_load(void)
{
	struct clk *clk;
	int ret = 0;
	union av7100_configuration config;
	
	/* prepare master clock for video input format */
	clk = clk_get(NULL, "vck5_clk");
	clk_set_rate(clk, clk_round_rate(clk, 36000000));
	clk_enable(clk);
	clk_put(clk);
	
	ret = av7100_download_firmware();
	if (ret)
		return -EFAULT;

	ret = edid_video_type();
	if (ret < 0)
		return -EFAULT;
	
	/* Update info attribute in switch class */
	hdmi_dev->info = ret;

	/* Update video input format */
	config.video_output_format.video_output_cea_vesa = ret;
	
	ret = av7100_conf_prep(AV7100_COMMAND_VIDEO_OUTPUT_FORMAT, &config);
	if (ret) {
		return -EFAULT;
	}

	ret = av7100_conf_w(AV7100_COMMAND_VIDEO_OUTPUT_FORMAT, NULL, NULL);
	if (ret) {
		return -EFAULT;
	}

	ret = av7100_conf_prep(AV7100_COMMAND_VIDEO_INPUT_FORMAT, &config);
	if (ret) {
		return -EFAULT;
	}
	
	/* OUTPUT */
	ret = av7100_conf_w(AV7100_COMMAND_VIDEO_INPUT_FORMAT, NULL, NULL);
	if (ret) {
		return -EFAULT;
	}

	/* Get HDMI command values */
	ret = av7100_conf_get(AV7100_COMMAND_HDMI, &config);
	if (ret) {
		return -EFAULT;
	}

	config.hdmi_format.hdmi_mode = AV7100_HDMI_ON;

	ret = av7100_conf_prep(AV7100_COMMAND_HDMI, &config);
	if (ret) {
		return -EFAULT;
	}

	ret = av7100_conf_w(AV7100_COMMAND_HDMI, NULL, NULL);
	if (ret) {
		return -EFAULT;
	}
	
	return 0;
 }

/*************************************************************************
 *	name	=	hdmi_worker
 *	func	=	Initialize HDMI environment for AV7100 and handle HDCP 
 *	input	=	struct work_struct *work
 *	output	=	None
 *	return	=	None
 *************************************************************************/
static void hdmi_worker(struct work_struct *work)
{
	u8 oni = 0;
	u8 hdcpi = 0;
	u8 hdcps = 0;
	u8 progged = 0;
	int block_cnt = 0;
	int ret = 0;
	struct hdcp_loadaesone hdcp_loadaesone;
	/* Read standby pending interrupt register */
	if (av7100_reg_stby_pend_int_r(NULL, &oni, NULL, NULL)) 
		return;
		
	if (oni == 1) {
	
		/* Download firmware and initialize HDMI video 
		settings for AV7100 */
		if (down_load())
			return;
			
		/* Check a secret AES-128 bit in AV7100 */
		if(hdcp_chkaesotp(&progged))
			return;
		if (progged == 0)
			return;
			
		/* Load AES HDCP key */
		
		while (block_cnt < HDMI_HDCP_AES_NR_OF_BLOCKS) {
			memcpy(hdcp_loadaesone.key, hdmi_dev->hdcp_loadaesall.key +
					block_cnt * HDMI_HDCP_AES_KEYSIZE,
					HDMI_HDCP_AES_KEYSIZE);
					
			if (hdcp_loadaes(block_cnt + HDMI_HDCP_AES_BLOCK_START,
					HDMI_HDCP_AES_KEYSIZE,
					hdcp_loadaesone.key,
					&hdcp_loadaesone.result))
				return;
				
			block_cnt ++;
		}
		
		/* Load KSV key */
		memset(hdcp_loadaesone.key, 0, HDMI_HDCP_AES_KSVZEROESSIZE);
		memcpy(hdcp_loadaesone.key + HDMI_HDCP_AES_KSVZEROESSIZE,
				hdmi_dev->hdcp_loadaesall.ksv, HDMI_HDCP_AES_KSVSIZE);
		
		ret = hdcp_loadaes(HDMI_HDCP_KSV_BLOCK,
				HDMI_HDCP_AES_KSVSIZE + HDMI_HDCP_AES_KSVZEROESSIZE,
				hdcp_loadaesone.key,
				&hdcp_loadaesone.result);
		
		hdmi_dev->hdcp_loadaesall.result = hdcp_loadaesone.result;
		
		if (ret)
			return;
		
		/* Set IDLE mode to operating mode */
		av7100_set_state(AV7100_OPMODE_IDLE);
		
		/* Change DSI command mode to video mode */
		/* TODO */
		
		/* Set HPD state */
		switch_set_state(&hdmi_dev->sdev, 1);
		
		/* Set 1 to ONI */
		if (av7100_reg_stby_pend_int_w(
				AV7100_STANDBY_PENDING_INTERRUPT_HPDI_LOW, 
				AV7100_STANDBY_PENDING_INTERRUPT_ONI_HIGH, 
				AV7100_STANDBY_PENDING_INTERRUPT_CCRST_LOW, 
				AV7100_STANDBY_PENDING_INTERRUPT_CCI_LOW))
			return;
			
	} 
	
	/* Handle HDCP */
	
	/* Read general interrupt register */
	if (av7100_reg_gen_int_r(NULL, NULL, NULL, NULL, &hdcpi, NULL))
		return;
		
	if (hdcpi == 1) {
		
		/* Read general interrupt register */
		if (av7100_reg_gen_status_r(NULL, NULL, NULL, NULL, NULL, &hdcps))
			return;
		
		/* Control HDCP state change */
		change_hdcp_status(hdcps);
		
		/* Set 1 to HDCPI */
		if (av7100_reg_gen_int_w(
				AV7100_GENERAL_INTERRUPT_EOCI_LOW, 
				AV7100_GENERAL_INTERRUPT_VSII_LOW, 
				AV7100_GENERAL_INTERRUPT_VSOI_LOW, 
				AV7100_GENERAL_INTERRUPT_CECI_LOW , 
				AV7100_GENERAL_INTERRUPT_HDCPI_HIGH, 
				AV7100_GENERAL_INTERRUPT_UOVBI_LOW))
			return;

	}
	
	return;
}

 /****************************************************************************
*	name	=	gpio_switch_work
*	func	=	GPIO switch worker
*	input	=	struct work_struct *work
*	output	=	None
*	return	=	None
****************************************************************************/
static void gpio_switch_work(struct work_struct *work)
{
	int hdmi_hpd_state = atomic_read(&hdmi_dev->hpd_state);

	if (hdmi_hpd_state == HPD_STATE_UNPLUGGED) {
		av7100_poweroff();
		switch_set_state(&hdmi_dev->sdev, HPD_STATE_UNPLUGGED);
		av7100_poweron();
		/* DSI ULPS/ Standby ON */
	} else {
		av7100_set_state(AV7100_OPMODE_INIT);
		/* DSI ULPS/ Standby OFF */
		
		/* Master clock timing, running */
		av7100_reg_stby_w(AV7100_STANDBY_STBY_HIGH,
				AV7100_STANDBY_MCLKRNG_22_5_TO_27_5_MHZ);
	}
}
/****************************************************************************
*	name	=	gpio_irq_handler
*	func	=	Interrupt hanlder of GPIO switch port
*	input	=	int irq, void *dev
*	output	=	None
*	return	=	IRQ_HANDLED
****************************************************************************/
static irqreturn_t gpio_irq_handler(int irq, void *dev)
{
	hdmi_hpd_handler(hdmi_get_current_hpd());
	return IRQ_HANDLED;
}

/*************************************************************************
 *	name	=	hdmi_open
 *	func	=	Open HDMI device node
 *	input	=	struct inode *inode, struct file *filp
 *	output	=	None
 *	return	=	0,	-EBUSY(-16)
 *************************************************************************/
static int hdmi_open(struct inode *inode, struct file *filp)
{
	LOCK_HDMI_EVENTS;
	if (hdmi_dev->device_open) {
		UNLOCK_HDMI_EVENTS;
		return -EBUSY;
	}
	hdmi_dev->device_open++;
	UNLOCK_HDMI_EVENTS;
	return 0;
}
/*************************************************************************
 *	name	=	hdmi_release
 *	func	=	Release HDMI device node
 *	input	=	struct inode *inode, struct file *filp
 *	output	=	None
 *	return	=	0
 *************************************************************************/
static int hdmi_release(struct inode *inode, struct file *filp)
{
	LOCK_HDMI_EVENTS;
	if (hdmi_dev->device_open)
		hdmi_dev->device_open--;
	UNLOCK_HDMI_EVENTS;
	return 0;
}
/*************************************************************************
 *	name	=	hdmi_ioctl
 *	func	=	Enable/disable HDCP authentication
 *	input	=	struct inode *inode, struct file *filp
 *	output	=	None
 *	return	=	0,	-EFAULT
 *************************************************************************/
static long hdmi_ioctl(struct file *file,
		       unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	switch (cmd) {
	case IOC_AUTHENTICATE_HDCP:
		ret = authenticate_hdcp(HDMI_HDCP_AUTH_ON);
		if (ret < 0) 
			ret = -EFAULT;
		break;
	case IOC_DEAUTHENTICATE_HDCP:
		ret = authenticate_hdcp(HDMI_HDCP_AUTH_OFF);
		if (ret < 0)
			ret = -EFAULT;
		break;
	case IOC_SET_AESHDCP_KEY:
		/*Get AESHDCP key from user side*/
		if (copy_from_user(&hdmi_dev->hdcp_loadaesall, (void *)arg,
			sizeof(struct hdcp_loadaesall)))
			ret = -EFAULT;
		break;
	case IOC_SET_REVOCATION_LIST:
		/*To do*/
		break;
	default:
		ret = -ENOTTY; /*Ioctl command is not supported*/
		break;
	}
	return ret;
}
/*************************************************************************
 *	File operation which use to control HDCP
 *************************************************************************/
static const struct file_operations hdmi_fops = {
	.owner =    THIS_MODULE,
	.open =     hdmi_open,
	.release =  hdmi_release,
	.unlocked_ioctl = hdmi_ioctl
};
/*************************************************************************
 *	Structure of miscellaneous device
 *************************************************************************/
static struct miscdevice hdmi_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "hdmi",
	.fops = &hdmi_fops
};
/*************************************************************************
 *	name	=	hdmi_init
 *	func	=	Create work queue, register switch driver and
 *			register miscellaneous device for HDMI driver
 *	input	=	None
 *	output	=	None
 *	return	=	0,	-ENOTSUPP(-524),	-ENOMEM(-12),
 *			-EINVAL(-22),	-EBUSY(-16)
 *************************************************************************/
int hdmi_init(void)
{
	int ret = 0;
	
	/*Allocate memory for hdmi_data*/
	hdmi_dev = kzalloc(sizeof(struct hdmi_data), GFP_KERNEL);
	if (!hdmi_dev)
		return -ENOMEM;
	ret = misc_register(&hdmi_miscdev);
	if(ret != 0) {
		ret = -ENOTSUPP;
		goto err_create_misc_device;
	}
	hdmi_dev->hdmidev = hdmi_miscdev.this_device;
	/*Set initial value for switch data*/
	hdmi_dev->sdev.name = "hdmi";
	hdmi_dev->gpio = GPIO_PORT14;
	hdmi_dev->info = 0;
	hdmi_dev->auth_flag = 0;
	hdmi_dev->device_open = 0;
	
	atomic_set(&hdmi_dev->hpd_state, 0);
	
	/*Register switch driver*/
	ret = switch_dev_register(&hdmi_dev->sdev);
	if (ret < 0) {
		ret = -EFAULT;
		goto err_switch_dev_register;
	}
	
	/*Create node info*/
	ret = device_create_file(hdmi_dev->sdev.dev, &dev_attr_info);
	if (ret) {
		ret = -EFAULT;
		goto err_create_file;
	}
	
	/*Request GPIO port for hot plug detection*/
	ret = gpio_request(hdmi_dev->gpio, "switch-gpio");
	if (ret < 0) {
		ret = -EFAULT;
		goto err_request_gpio;
	}
	
	/*Set input direction to GPIO port*/
	ret = gpio_direction_input(hdmi_dev->gpio);
	if (ret < 0) {
		ret = -EFAULT;
		goto err_set_gpio_input;
	}
	
	/*Create workqueue and register processing function*/
	hdmi_dev->my_workq = create_singlethread_workqueue("hdmi_workqueue");
	if (hdmi_dev->my_workq == NULL) {
		ret = -ENOMEM;
		goto err_create_workqueue;
	}
	INIT_WORK(&hdmi_dev->hdmi_work, hdmi_worker);
	INIT_WORK(&hdmi_dev->gpio_work, gpio_switch_work);
	
	/*Map GPIO to IRQ number*/	
	hdmi_dev->irq = gpio_to_irq(hdmi_dev->gpio);
	if (hdmi_dev->irq < 0) {
		ret = -EFAULT;
		goto err_detect_irq_num_failed;
	}
	/*Request interrupt channel for GPIO port*/
	ret = request_irq(hdmi_dev->irq, gpio_irq_handler,
			  IRQF_TRIGGER_LOW | IRQF_TRIGGER_HIGH,
			  "switch-gpio", hdmi_dev);
	if (ret < 0) {
		ret = -EFAULT;
		goto err_request_irq;
	}
	
	return 0;
	
	/* Error handling*/
err_request_irq:
err_detect_irq_num_failed:
	destroy_workqueue(hdmi_dev->my_workq);
err_create_workqueue:
err_set_gpio_input:
	gpio_free(hdmi_dev->gpio);
err_request_gpio:
    device_remove_file(hdmi_dev->sdev.dev, &dev_attr_info);
err_create_file:
	switch_dev_unregister(&hdmi_dev->sdev);
err_switch_dev_register:
	misc_deregister(&hdmi_miscdev);
err_create_misc_device:
	kfree(hdmi_dev);
	hdmi_dev = NULL;
	return ret;
}
/*************************************************************************
 *	name	=	hdmi_exit
 *	func	=	Finalize configuration for GPIO 
 *			and miscellaneous device 
 *	input	=	None
 *	output	=	None
 *	return	=	None
 *************************************************************************/
void hdmi_exit(void)
{
	free_irq(hdmi_dev->irq, NULL);
	destroy_workqueue(hdmi_dev->my_workq);
	gpio_free(hdmi_dev->gpio);
	device_remove_file(hdmi_dev->sdev.dev, &dev_attr_info);
	switch_dev_unregister(&hdmi_dev->sdev);
	misc_deregister(&hdmi_miscdev);
	kfree(hdmi_dev);
	hdmi_dev = NULL;
}