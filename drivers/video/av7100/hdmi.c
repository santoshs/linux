/*
 * Copyright (C) ST-Ericsson AB 2010
 * Copyright (C) 2012 Renesas Mobile Corporation.
 *
 * ST-Ericsson HDMI driver
 *
 * Author: Per Persson <per.xb.persson@stericsson.com>
 * for ST-Ericsson.
 *
 * License terms: GNU General Public License (GPL), version 2.
 */

/* This file contains a misc driver which serves as rest of the world contact
for the av7100 driver.
   This file also has the switch driver which is is used to send uevents to android 
framework and attributes for other modules to know the status of hdmi */

#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>
#include <video/av7100.h>
#include <video/hdmi.h>
#include <linux/poll.h>
#include <linux/mutex.h>
#include <linux/ctype.h>
#include "hdmi_local.h"
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/clk.h>
#include <linux/switch.h>
#include <video/av7100_interface.h>
#include <linux/types.h>
#include "av7100_regs.h"
#define SYSFS_EVENT_FILENAME "evread"
#define AV7100_COMMAND_OFFSET 0x10

DEFINE_MUTEX(hdmi_events_mutex);
#define LOCK_HDMI_EVENTS mutex_lock(&hdmi_events_mutex)
#define UNLOCK_HDMI_EVENTS mutex_unlock(&hdmi_events_mutex)
#define EVENTS_MASK 0xFF

static int device_open;
static int events;
static int events_mask;
static bool events_received;
static wait_queue_head_t hdmi_event_wq;
static struct device *hdmidev;
static struct device *switchdev;
static struct dtd timingDescriptors[7];
struct dtd supported_formats[4];
int plugedin_state = 0;
static struct hdcp_loadaesall hdcp_ldaes;
static int auth_flag;
static u8 key_programmed_flag = 0;

struct dtd supported_dtds[] = {
{2700,720,138,480,45,0,2},/* CEA 2,3 */
{7425,1280,370,720,30,0,3},/* CEA 4 */
{7425,1920,280,1080,22,0,4},/* CEA 5 */
{5400,1440,276,480,45,0,6},/* 14 15 */
{14850,1920,280,1080,45,0,7},/* CEA 16 */
{2700,720,144,576,49,0,8},/* CEA 17,18 */
{7425,1280,700,720,30,0,9},/* CEA 19 */
{7425,1920,720,1080,22,1,10},/* CEA 20 */
{2700,1440,288,576,24,1,11},/* CEA 21,22 */
{14850,1920,720,1080,45,0,13},/* CEA 31 */
};
unsigned char otp_key[] = {
	0x10,0xEB,0x3D,0x60,0xBE,0x71,0xCA,0x15,0xF0,0xAE,0x73,
	0x2B,0x81,0x77,0x7D,0x85 };

static struct switch_dev s_dev = {
	.name = "hdmi",
	.state = 0
};

static ssize_t show_state_hdmi(struct device *dev,
		struct device_attribute *attr, char *buf);
static int show_info(struct device *dev,struct device_attribute *attr,char *buf); 
static ssize_t store_info(struct device *dev,struct device_attribute *attr,char *buf,size_t count);
ssize_t down_load(struct device *dev, struct device_attribute *attr,
			 char *buf);
extern int register_read_internal(u8 offset, u8 *value);


static DEVICE_ATTR(state, S_IRUGO, show_state_hdmi, NULL);
static DEVICE_ATTR(info, S_IRUGO | S_IWUGO, show_info, store_info);


/*****************************************************************************
 *	name	=	hdcp_chkaesotp
 *	func	=	Check  AESOTP key has already programmed in AV7100
 *	input	=	u8 *progged
 *	output	=	u8 *progged
 *	return	=	0, -EFAULT, -EINVAL
 *****************************************************************************/
static int hdcp_chkaesotp(u8 *progged)
{
	union av7100_configuration config;
	u8 buf_len = 0;
	u8 buf[2];
	u8 crc8_otp = 0x1A;
	/*Set read operation to fuse_operation*/
	config.fuse_aes_key_format.fuse_operation = AV7100_FUSE_READ;
	memset(config.fuse_aes_key_format.key, 0, AV7100_FUSE_KEY_SIZE);
	
	if (av7100_conf_prep(AV7100_COMMAND_FUSE_AES_KEY, &config) != 0) {
		dev_err(hdmidev, "av7100_conf_prep FAIL\n");
		return -EFAULT;
	}
	/*Write new configuration to AV7100*/
	if (av7100_conf_w(AV7100_COMMAND_FUSE_AES_KEY, &buf_len, buf,I2C_INTERFACE) != 0) {
		dev_err(hdmidev, "av7100_conf_w FAIL\n");
		return -EFAULT;
	}
	/*Check result of AESOTP key reading*/
	if (buf_len == 2) {
		if (progged) {
		     if(buf[0] == crc8_otp ) {
			    *progged = buf[1];	/*Set result of reading to progged*/
			 } else {
			     /* set progged to 1 only if crc matches */
			    *progged = 0;
		     }
		}
			 
	} else {
			return -EINVAL;
	}
	return 0;
}

/*****************************************************************************
 *	name	=	hdcp_loadaes
 *	func	=	Load  HDCP keys have been encrypted by AES key
 *			from secured memory to AV7100 
 *	input	=	u8 block, u8 key_len, u8 *key
 *	output	=	u8 *result
 *	return	=	0, -EFAULT, -EINVALID
 *****************************************************************************/
static int hdcp_loadaes(u8 block, u8 key_len, u8 *key, u8 *result)
{
	union av7100_configuration config;
	if (!result || !key)
		return -EINVAL;
	
	 /* Default result of loading is fail  */
	*result = 1;
	
	/* Initialize values to hdcp_send_key_format */
	config.hdcp_send_key_format.key_number = block;
	config.hdcp_send_key_format.data_len = key_len;
	memcpy(config.hdcp_send_key_format.data, key, key_len);
	
	/* Prepare to write write new configuration to AV7100 */
	if (av7100_conf_prep(AV7100_COMMAND_HDCP_SENDKEY, &config) != 0) {
		dev_err(hdmidev, "av7100_conf_prep FAIL\n");
		return -EFAULT;
	}
	
	/* Write new configuration to AV7100 */
	if (av7100_conf_w(AV7100_COMMAND_HDCP_SENDKEY, NULL, NULL,I2C_INTERFACE) != 0) {
		dev_err(hdmidev, "av7100_conf_w FAIL\n");
		return -EFAULT;
	}
	*result = 0; /* Result of loading is successful */
	
	return 0;
}

/*****************************************************************************
 *	name	=	authenticateHDCP
 *	func	=	Authenticate/De-authenticate HDCP
 *	input	=	u8 auth_type
 *	output	=	None
 *	return	=	0, -EFAULT, -ENODATA(-61)
 *****************************************************************************/
static int authenticateHDCP(u8 auth_type)
{
	union av7100_configuration config;
	/* AES HDCP keys have not loaded to AV7100 */
	if (hdcp_ldaes.result != 0) 
		return -ENODATA;
	
	if (auth_type == HDMI_HDCP_AUTH_ON) 
		config.hdcp_management_format.req_type = AV7100_HDCP_AUTH_REQ_ON;
	else
		config.hdcp_management_format.req_type = AV7100_HDCP_AUTH_REQ_OFF;
	
	/* Encrypted types: EESS(HDMI), OESS(DVI) */
	config.hdcp_management_format.req_encr = AV7100_HDCP_ENCR_USE_EESS;
	
	/* Prepare to write write new configuration to AV7100 */
	
	if (av7100_conf_prep(AV7100_COMMAND_HDCP_MANAGEMENT, &config) != 0) {
		dev_err(hdmidev, "av7100_conf_prep FAIL\n");
		return -EFAULT;
	}
	
	/* Write new configuration to AV7100 */
	if (av7100_conf_w(AV7100_COMMAND_HDCP_MANAGEMENT, NULL, NULL,I2C_INTERFACE) != 0) {
		dev_err(hdmidev, "av7100_conf_w FAIL\n");
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
        /* to be implemented by OEM */
	return 0;
}

/*****************************************************************************
 *	name	=	changeHDCPstatus
 *	func	=	Control HDCP state
 *	input	=	u8 hdcp_state
 *	output	=	None
 *	return	=	0, -EFAULT, -ENODATA(-61)
 *****************************************************************************/
int changeHDCPstatus(u8 hdcp_state)
 {
	union av7100_configuration config;
	int ret = 0;
	u8 buf_len = 0;
	u8 buf[6];
	char * revoq = NULL;
	
	/* Encrypted types: EESS(HDMI), OESS(DVI) */
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
		auth_flag = 0;
		setHdmiState(AV7100_HDMI_OFF);
		printk(KERN_ALERT "Authentication fail state\n");
		break;
	case HDMI_HDCP_AUTHENTICATION_SUCCEED:
		/* Default authenticated result is fail */
		auth_flag = 0;
		/* Get BKSV of sink */
		config.hdcp_management_format.req_type = AV7100_HDCP_REV_LIST_REQ;
		if (av7100_conf_prep(AV7100_COMMAND_HDCP_MANAGEMENT, &config) != 0) {
			dev_err(hdmidev, "av7100_conf_prep FAIL\n");
			return -EFAULT;
		}
		if (av7100_conf_w(AV7100_COMMAND_HDCP_MANAGEMENT,
			&buf_len, &buf[0],I2C_INTERFACE) != 0) {
			dev_err(hdmidev, "av7100_conf_w FAIL\n");
			return -EFAULT;
		}
		/* Check revocation list. This function is not defined as it is not possible to check revocation
           List with sample keys */
		if (check_revoq_list(revoq,&buf[0]) == 1) {
			setHdmiState(AV7100_HDMI_OFF);
			return -EFAULT;
		}
		config.hdcp_management_format.req_type =
			AV7100_HDCP_AUTH_CONT;
		if (av7100_conf_prep(AV7100_COMMAND_HDCP_MANAGEMENT, &config) != 0) {
			dev_err(hdmidev, "av7100_conf_prep FAIL\n");
			return -EFAULT;
		}
		if (av7100_conf_w(AV7100_COMMAND_HDCP_MANAGEMENT, NULL, NULL,I2C_INTERFACE) != 0) {
			dev_err(hdmidev, "av7100_conf_w FAIL\n");
			return -EFAULT;
		}
		auth_flag = 1;
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
/****************************************************************************
*	name	=	supported_output_formats
*	func	=	Gets supported formats of the sink
*	input	=	int numDescriptors 
*	output	=	None
*	return	=	CEA ID of the format supported
****************************************************************************/
u8 supported_output_formats(int numDescriptors){
   u8 i,j;
   u8 len_supported_dtds = sizeof(supported_dtds)/sizeof(struct dtd);

   for(j=0;j<numDescriptors;j++){
      for(i=0;i<len_supported_dtds;i++){

         if( (supported_dtds[i].pixelFrequency == 
                timingDescriptors[j].pixelFrequency) && 
             (supported_dtds[i].hActive == timingDescriptors[j].hActive) && 
             (supported_dtds[i].hBlank == timingDescriptors[j].hBlank) &&
             (supported_dtds[i].vActive == timingDescriptors[j].vActive) && 
             (supported_dtds[i].vBlank == timingDescriptors[j].vBlank) &&
             (supported_dtds[i].videoMode == timingDescriptors[j].videoMode)){ 
				return supported_dtds[i].ceaId;				
         }

      }
   }
   return AV7100_CEA4_1280X720P_60HZ;

}

/****************************************************************************
*	name	=	readEdid
*	func	=	Reads edid of the connected sink
*	input	=	u8 *buf,u8 *buf1
*	output	=	None
*	return	=	CEA format supported
****************************************************************************/
u8 readEdid(u8 *buf,u8 *buf1){
	
	u8 b1timingDescriptorStart = 0;
	u8 i;
	int numTimingDescriptor = 0;
	timingDescriptors[0].pixelFrequency = 
	buf[FIRST_TIMING_DESCRIPTOR_START+PIXEL_FREQUENCY_LOWBYTE] | 
	(buf[FIRST_TIMING_DESCRIPTOR_START+PIXEL_FREQUENCY_HIGHBYTE] << 8);
	
	timingDescriptors[0].hActive = 
	buf[FIRST_TIMING_DESCRIPTOR_START+HACTIVE_LOWBYTE] | 
	((buf[FIRST_TIMING_DESCRIPTOR_START+HACTIVE_HIGHNIBBLE] & 0xf0) << 4);
	
	timingDescriptors[0].hBlank = 
	buf[FIRST_TIMING_DESCRIPTOR_START+HBLANK_LOWBYTE] |
	((buf[FIRST_TIMING_DESCRIPTOR_START+HBLANK_HIGHNIBBLE] & 0x0f) << 8);
	
	timingDescriptors[0].vActive =
	buf[FIRST_TIMING_DESCRIPTOR_START+VACTIVE_LOWBYTE] |  
	((buf[FIRST_TIMING_DESCRIPTOR_START+VACTIVE_HIGHNIBBLE] & 0xf0) << 4);
	
	timingDescriptors[0].vBlank = 
	(unsigned short)buf[FIRST_TIMING_DESCRIPTOR_START+VBLANK_LOWBYTE] | 
	((buf[FIRST_TIMING_DESCRIPTOR_START+VBLANK_HIGHNIBBLE] & 0x0f) << 8);
	
	timingDescriptors[0].videoMode =
	(buf[FIRST_TIMING_DESCRIPTOR_START+VIDEOMODE] & 0x80) >> 7;
	
	timingDescriptors[1].pixelFrequency = 
	buf[FIRST_TIMING_DESCRIPTOR_START+PIXEL_FREQUENCY_LOWBYTE] | 
	(buf[SECOND_TIMING_DESCRIPTOR_START+PIXEL_FREQUENCY_HIGHBYTE] << 8);
	
	timingDescriptors[1].hActive = 
	buf[SECOND_TIMING_DESCRIPTOR_START+HACTIVE_LOWBYTE] | 
	((buf[SECOND_TIMING_DESCRIPTOR_START+HACTIVE_HIGHNIBBLE] & 0xf0) << 4);
	
	timingDescriptors[1].hBlank = 
	buf[SECOND_TIMING_DESCRIPTOR_START+HBLANK_LOWBYTE] | 
	((buf[SECOND_TIMING_DESCRIPTOR_START+HBLANK_HIGHNIBBLE] & 0x0f) << 8);
	
	timingDescriptors[1].vActive = 
	buf[SECOND_TIMING_DESCRIPTOR_START+VACTIVE_LOWBYTE] | 
	((buf[SECOND_TIMING_DESCRIPTOR_START+VACTIVE_HIGHNIBBLE] & 0xf0) << 4);
	
	timingDescriptors[1].vBlank = 
	buf[SECOND_TIMING_DESCRIPTOR_START+VBLANK_LOWBYTE] | 
	((buf[SECOND_TIMING_DESCRIPTOR_START+VBLANK_HIGHNIBBLE] & 0x0f) << 8);
	
	timingDescriptors[1].videoMode = 
	(buf[SECOND_TIMING_DESCRIPTOR_START+VIDEOMODE] & 0x80) >> 7;
	
	
	numTimingDescriptor = (buf1[3] & 0x0f)+2;
	b1timingDescriptorStart = buf1[2];
	
	if(buf1[2] != 0)  {
		
		for(i =2;i<numTimingDescriptor;i++) {
			int index = b1timingDescriptorStart + 18*(i-2);
			timingDescriptors[i].pixelFrequency = 
            (unsigned long)buf1[index+PIXEL_FREQUENCY_LOWBYTE] | 
            ((unsigned long)buf1[index+PIXEL_FREQUENCY_HIGHBYTE] << 8);
			timingDescriptors[i].hActive = 
            (unsigned short)buf1[index+HACTIVE_LOWBYTE] | 
            ((buf1[index+HACTIVE_HIGHNIBBLE] & 0xf0) << 4);
			timingDescriptors[i].hBlank = 
            (unsigned short)buf1[index+HBLANK_LOWBYTE] | 
            ((buf1[index+HBLANK_HIGHNIBBLE] & 0x0f) << 8);
			timingDescriptors[i].vActive = 
            (unsigned short)buf1[index+VACTIVE_LOWBYTE] | 
            ((buf1[index+VACTIVE_HIGHNIBBLE] & 0xf0) << 4);
			timingDescriptors[i].vBlank = 
            buf1[index+VBLANK_LOWBYTE] | 
            ((buf1[index+VBLANK_HIGHNIBBLE] & 0x0f) << 8);
			timingDescriptors[i].videoMode = (buf[index+VIDEOMODE] & 0x80) >> 7;
			
		}	
		
	}
	
	/* Return the supported formats that matches both WWODIN requirements and 
	sync */
	
	return supported_output_formats(numTimingDescriptor);
	
}

/****************************************************************************
*	name	=	selectFormatEdidBased
*	func	=	Function to select output format based 
* 			on edid of the sink
*	input	=	void
*	output	=	None
*	return	=	0, AV7100_FAIL, -EINVAL
****************************************************************************/
static int selectFormatEdidBased(void) {
   union av7100_configuration config;
   int count = 0;
   int ret = 0;
   u8 len=128;
   u8 data[128];	
   u8 data1[128];

   memset(&config, 0, sizeof(union av7100_configuration));

   /* Obtain Block 0 of CEA 865E EDID extension */
   config.edid_section_readback_format.address = 0xA0;
   config.edid_section_readback_format.block_number = 0;
   ret = av7100_conf_prep(
             AV7100_COMMAND_EDID_SECTION_READBACK, &config);
   if(av7100_conf_w(AV7100_COMMAND_EDID_SECTION_READBACK,&len,(unsigned char *)&data,I2C_INTERFACE) != 0){
      dev_err(hdmidev, "av7100_conf_w1 FAIL\n");
      return -EINVAL;
   }

   count =0;
   len = 128;
   memset(&config, 0, sizeof(union av7100_configuration));
	
   /* Obtain Block 1 of CEA 865E EDID extension */

   config.edid_section_readback_format.address = 0xA0;
   config.edid_section_readback_format.block_number = 1;
   ret = av7100_conf_prep(
            AV7100_COMMAND_EDID_SECTION_READBACK, &config);
   if (ret)
      return AV7100_FAIL;

   if(av7100_conf_w(AV7100_COMMAND_EDID_SECTION_READBACK,
              &len,(unsigned char *)&data1,I2C_INTERFACE) != 0){
      dev_err(hdmidev, "av7100_conf_w2 FAIL\n");
      return -EINVAL;
   } 

   memset(&config, 0, sizeof(union av7100_configuration));

   /* Get the timing descriptors and check if that matches with 
	the supported table supported_dtds defined above
	*/
   config.video_output_format.video_output_cea_vesa =  readEdid(&data,&data1);
   
   ret = av7100_conf_prep(AV7100_COMMAND_VIDEO_OUTPUT_FORMAT, &config);

   if (ret)
      return AV7100_FAIL; 


   memset(&config, 0, sizeof(union av7100_configuration));
   
   if(av7100_conf_get(AV7100_COMMAND_VIDEO_INPUT_FORMAT, &config) != 0) {
      dev_err(hdmidev, "av7100_conf_get video input FAIL\n");
      return AV7100_FAIL;
   }
   /* Configure additional parameters that are not set by output dep */
   config.video_input_format.input_pixel_format = AV7100_INPUT_PIX_RGB565;
   config.video_input_format.dsi_input_mode = AV7100_HDMI_DSI_VIDEO_MODE;
   config.video_input_format.nb_data_lane = 4;
   config.video_input_format.master_clock_freq = clk_get_rate(clk_get(NULL, "vclk5_clk"));

   if (av7100_conf_prep(AV7100_COMMAND_VIDEO_INPUT_FORMAT, &config)) {
      dev_err(hdmidev, "av7100_conf_get video input FAIL\n");
      return AV7100_FAIL;
   }

   if (av7100_conf_w(AV7100_COMMAND_VIDEO_INPUT_FORMAT, NULL, NULL, I2C_INTERFACE)) {
       dev_err(hdmidev, "av7100_conf_w video input FAIL\n");
       return AV7100_FAIL ;
   }
   
   if(av7100_conf_w(AV7100_COMMAND_VIDEO_OUTPUT_FORMAT,NULL,NULL,I2C_INTERFACE) != 0){
      dev_err(hdmidev, "av7100_conf_w output FAIL\n");
      return AV7100_FAIL;	
   }

   return 0;
}

/****************************************************************************
*	name	=	htoi
*	func	=	Hexadecimal to integer conversion
*	input	=	const char *ptr
*	output	=	None
*	return	=	value
****************************************************************************/
static unsigned int htoi(const char *ptr)
{
	unsigned int value = 0;
	char ch = *ptr;

	if (!ptr)
		return 0;

	if (isdigit(ch))
		value = ch - '0';
	else
		value = toupper(ch) - 'A' + 10;

	value <<= 4;
	ch = *(++ptr);

	if (isdigit(ch))
		value += ch - '0';
	else
		value += toupper(ch) - 'A' + 10;

	return value;
}

/****************************************************************************
*	name	=	atoi
*	func	=	String to integer conversion.
*	input	=	const char *string
*	output	=	None
*	return	=	converted integer
****************************************************************************/
int atoi(const char *string)
{
	int i = 0;

	while(*string)
	{
		i=(i<<3) + (i<<1) + (*string - '0');
		string++;
	}
	return(i);
}

/****************************************************************************
*	name	=	event_enable
*	func	=	Enables the hdmi events by setting proper 
* 			value of events_mask
*	input	=	bool enable, enum hdmi_event ev
*	output	=	None
*	return	=	0
****************************************************************************/
static int event_enable(bool enable, enum hdmi_event ev)
{
	dev_dbg(hdmidev, "enable_event %d %02x\n", enable, ev);
	if (enable)
		events_mask |= ev;
	else
		events_mask &= ~ev;

	return 0;
}

/****************************************************************************
*	name	=	plugdeten
*	func	=	Handle plug detection of HDMI
*	input	=	struct plug_detect *pldet
*	output	=	None
*	return	=	-EINVAL, -EFAULT, retval
****************************************************************************/
static int plugdeten(struct plug_detect *pldet)
{
	struct av7100_status status;
	u8 denc_off_time = 0;
	int retval;

	status = av7100_status_get();
	if (status.av7100_state < AV7100_OPMODE_STANDBY) {
		if (av7100_powerup() != 0) {
			dev_err(hdmidev, "av7100_powerup failed\n");
			return -EINVAL;
		}
	}


	retval = av7100_reg_hdmi_5_volt_time_w(
			pldet->hdmi_off_time,
			pldet->on_time);

	if (retval) {
		dev_err(hdmidev, "Failed to write the value to av7100 "
			"register\n");
		return -EFAULT;
	}

	status = av7100_status_get();
	if (status.av7100_state < AV7100_OPMODE_IDLE) {
		av7100_disable_interrupt();
		av7100_enable_interrupt();
	}

	event_enable(pldet->hdmi_detect_enable != 0,
		HDMI_EVENT_HDMI_PLUGIN);
	event_enable(pldet->hdmi_detect_enable != 0,
		HDMI_EVENT_HDMI_PLUGOUT);

	return retval;
}

/****************************************************************************
*	name	=	show_info
*	func	=	Used by other modules to check HDMI plug status
*	input	=	struct device *dev,struct device_attribute *attr,
* 			char *buf
*	output	=	None
*	return	=	0, ret
****************************************************************************/
static int show_info(struct device *dev,struct device_attribute *attr,char *buf){

	int video_output_format;
	int ret = 0;

	char output_format[] = "[video_output_format]\n";
	char formatNumber[10];
	struct av7100_status status;


	union av7100_configuration config;

	status = av7100_status_get();

	//When HDMI is in Stand by mode, the video_output_format is set to 0
	if(status.av7100_state <= AV7100_OPMODE_STANDBY){

		video_output_format = 0;
		sprintf(formatNumber,"%d",video_output_format);

		strcat(output_format,formatNumber);
		ret = sprintf(buf,"%s",output_format);
		return ret;

	}
	
	//When HDMI is not plugged in, the video_output_format is set to 0
	if(plugedin_state == 0){

		video_output_format = 0;
		sprintf(formatNumber,"%d",video_output_format);

		strcat(output_format,formatNumber);
                ret = sprintf(buf,"%s",output_format);

	}else{

		memset(&config, 0, sizeof(union av7100_configuration));

        	if(av7100_conf_get(AV7100_COMMAND_VIDEO_OUTPUT_FORMAT, &config) != 0) {
	      	    dev_err(hdmidev, "av7100_conf_get video output FAIL\n");
        	    return AV7100_FAIL;
  		}

		video_output_format = config.video_output_format.video_output_cea_vesa;
		sprintf(formatNumber,"%d",video_output_format);
	
		strcat(output_format,formatNumber);
		ret = sprintf(buf,"%s",output_format);

	}

	return ret;

}

/*****************************************************************************
 *	name	=	store_info
 *	func	=	Function that stores video_output_format info passed 
* 			from other modules
 *	input	=	struct device *dev,struct device_attribute *attr,
 *			char *buf,size_t count
 *	output	=	None
 *	return	=	-EINVAL, AV7100_FAIL, count
 *****************************************************************************/
static ssize_t store_info(struct device *dev,struct device_attribute *attr,char *buf,size_t count){
	
	u8 i,j,split;
	char buf1[50];
	char buf2[10];
	u8 hdmiOn = 0;
	
	enum av7100_output_CEA_VESA outputFormat;
	u8 lFormat = 0;	
	
	union av7100_configuration config;
	
	memset(&config, 0, sizeof(union av7100_configuration));
	memset(&buf1, 0, sizeof(buf1));
	memset(&buf2, 0, sizeof(buf2));
	
	if (av7100_conf_get(AV7100_COMMAND_HDMI, &config) != 0) {
		dev_err(hdmidev, "av7100_conf_get FAIL\n");
		return -EINVAL;
	}
	/* Switch off hdmi out put it it was already on */
	if(config.hdmi_format.hdmi_mode == AV7100_HDMI_ON){
		
		config.hdmi_format.hdmi_mode = AV7100_HDMI_OFF;
		/* Store to av7100_config */
		if (av7100_conf_prep(AV7100_COMMAND_HDMI, &config) != 0) 
		{
			dev_err(hdmidev, "av7100_conf_prep FAIL\n");
			return -EINVAL;
		}
		if (av7100_conf_w(AV7100_COMMAND_HDMI, NULL, NULL,
						  I2C_INTERFACE) != 0) 
		{
			dev_err(hdmidev, "av7100_conf_w FAIL\n");
			return -EINVAL;
		}
		
	}
	
	for(i=0,j=0,split=0;i<strlen(buf);i++)
	{
		
		if(buf[i] != '\n')
		{
			if(!split) 
			{
				buf1[i] = buf[i];
			} else 
			{
			    buf2[j++] = buf[i];
			}
			
		}else{
			split = 1; 
		}
   }
	
	
	if(((strcmp(buf1,"[video_output_format]")) == 0)){
		outputFormat = atoi(buf2);
	}
	
	/* if passed in output format is not one of the supported formats*/
	
	if((outputFormat != 2) && (outputFormat != 3) &&
	   (outputFormat != 4) && (outputFormat != 6) &&
	   (outputFormat != 7) && (outputFormat != 8) &&
	   (outputFormat != 9) && (outputFormat != 10) &&
	   (outputFormat != 11) && (outputFormat != 13)){
		outputFormat = 3; 
	}
	
	
	memset(&config, 0, sizeof(union av7100_configuration));
	
	/* Prepare output format */
	
	config.video_output_format.video_output_cea_vesa = outputFormat;
	
	if(av7100_conf_prep(AV7100_COMMAND_VIDEO_OUTPUT_FORMAT, &config) != 0){
		dev_err(hdmidev, "av7100_conf_prep video output FAIL\n");
		return AV7100_FAIL ;
	}
	
	memset(&config, 0, sizeof(union av7100_configuration));
	
	if(av7100_conf_get(AV7100_COMMAND_VIDEO_INPUT_FORMAT, &config) != 0) {
		dev_err(hdmidev, "av7100_conf_get video input FAIL\n");
		return 0;
	}
	
	config.video_input_format.input_pixel_format = AV7100_INPUT_PIX_RGB565;
	config.video_input_format.dsi_input_mode = AV7100_HDMI_DSI_VIDEO_MODE;
	config.video_input_format.nb_data_lane = 4;
	config.video_input_format.master_clock_freq = clk_get_rate(clk_get(NULL, "vclk5_clk")); 
	
	/* Prepare input format */
	
	if(av7100_conf_prep(AV7100_COMMAND_VIDEO_INPUT_FORMAT, &config) != 0){
		dev_err(hdmidev, "av7100_conf_prep video input FAIL\n");
		return AV7100_FAIL ;
	}
	
	if (av7100_conf_w(AV7100_COMMAND_VIDEO_INPUT_FORMAT, NULL, NULL, I2C_INTERFACE) != 0) {
		dev_err(hdmidev, "av7100_conf_w video input FAIL\n");
		return AV7100_FAIL ;
	}
	
	if(av7100_conf_w(AV7100_COMMAND_VIDEO_OUTPUT_FORMAT,NULL,NULL,I2C_INTERFACE) != 0){
		dev_err(hdmidev, "av7100_conf_w output FAIL\n");
		return AV7100_FAIL;       
   	}
	/* Turn on HDMI */
	memset(&config, 0, sizeof(union av7100_configuration));
	if (av7100_conf_get(AV7100_COMMAND_HDMI, &config) != 0) {
		dev_err(hdmidev, "av7100_conf_get FAIL\n");
		return 0;
	}
	config.hdmi_format.hdmi_mode = AV7100_HDMI_ON;
	if (av7100_conf_prep(AV7100_COMMAND_HDMI, &config) != 0) {
		dev_err(hdmidev, "av7100_conf_prep FAIL\n");
		return -EINVAL;
	}
	if (av7100_conf_w(AV7100_COMMAND_HDMI, NULL, NULL,
					  I2C_INTERFACE) != 0) {
		dev_err(hdmidev, "av7100_conf_w FAIL\n");
		return -EINVAL;
	}
	return count;
}
//#if 0 
/* for testing only */
/*****************************************************************************
 *	name	=	down_load
 *	func	=	Handle hot plug detection of HDMI
 *	input	=	struct device *dev, struct device_attribute *attr,
 *			char *buf
 *	output	=	None
 *	return	=	-EIO, 0
 *****************************************************************************/

ssize_t down_load(struct device *dev, struct device_attribute *attr,
			 char *buf)
{
	union av7100_configuration config;
	struct av7100_status status;

        u8 sid = 0;
        u8 oni = 0;
        u8 hpdi = 0;
        u8 stby = 0;
        u8 mclkrng = 0;
        u8 hpds = 0;
        u8 cpds = 0;

	
	/*
	 * HDMI video setting are based on
	 * ${LINARO}/drivers/video/mcde/display-av7100.c
	 *
	 * we don't have userland program with will call
	 * hdmi_ioctl. I guess it is needed for av7100.
	 * And it will download the firmware.
	 *
	 * This function is quick-hack for downloading firmware
	 * and settings
	 *
	 *  cat /sys/devices/virtual/misc/hdmi/dwload
	 */
	if (av7100_powerup() != 0) {
		dev_err(hdmidev, "av7100_powerup FAIL\n");
		return -EIO;
	}

	status = av7100_status_get();

	/* IOC_HDMI_DOWNLOAD_FW */
	if (status.av7100_state < AV7100_OPMODE_INIT) {
		struct clk *clk;
		u8 stby;

		clk = clk_get(NULL, "vclk5_clk");
		clk_set_rate(clk, clk_round_rate(clk, 36000000));

		clk_enable(clk);
		clk_put(clk);

		if(av7100_reg_stby_pend_int_r(&hpdi,&oni,&sid)){
			dev_err(hdmidev, "stby pend read FAIL\n");
                        return 0;
                }

		if (av7100_reg_stby_r(&stby, &hpds, &mclkrng)) {
			dev_err(hdmidev, "stby read FAIL\n");
			return 0;
		}
		
		if (!stby) {
			dev_err(hdmidev, "av7100 not running\n");
			return 0;
		}

		if (av7100_download_firmware(NULL, 0, I2C_INTERFACE) != 0) {
			dev_err(hdmidev, "av7100 dl fw FAIL\n");
			return 0;
		}

	}

		/* PATTERN CHECK */
	if(av7100_conf_w(AV7100_COMMAND_PATTERNGENERATOR, NULL, NULL, I2C_INTERFACE) != 0) {
		dev_err(hdmidev, "av7100_conf_w pattern FAIL\n");
		return 0;
	}

	if (av7100_conf_w(AV7100_COMMAND_VIDEO_OUTPUT_FORMAT, NULL, NULL, I2C_INTERFACE)) {
		dev_err(hdmidev, "av7100_conf_w output FAIL\n");
		return 0;
	}
	
	if (av7100_conf_get(AV7100_COMMAND_HDMI, &config) != 0) {
		dev_err(hdmidev, "av7100_conf_get FAIL\n");
		return 0;
	}

	config.hdmi_format.hdmi_mode = AV7100_HDMI_ON;

	if (av7100_conf_prep(AV7100_COMMAND_HDMI, &config) != 0) {
		dev_err(hdmidev, "av7100_conf_prep FAIL\n");
		return 0;
	}

	if (av7100_conf_w(AV7100_COMMAND_HDMI, NULL, NULL,
			  I2C_INTERFACE) != 0) {
		dev_err(hdmidev, "av7100_conf_w FAIL\n");
		return 0;
	}
	
	return 0;
}

//#endif
/*****************************************************************************
 *	name	=	events_read
 *	func	=	Read HDMI event that occurs
 *	input	=	void 
 *	output	=	None
 *	return	=	ret
 *****************************************************************************/

static u8 events_read(void)
{
	int ret;

	LOCK_HDMI_EVENTS;
	ret = events;
	dev_dbg(hdmidev, "%s %02x\n", __func__, events);
	UNLOCK_HDMI_EVENTS;

	return ret;
}

/*****************************************************************************
 *	name	=	events_clear
 *	func	=	Clear HDMI event that is stored
 *	input	=	u8 ev
 *	output	=	None
 *	return	=	0
 *****************************************************************************/
static int events_clear(u8 ev)
{
	dev_dbg(hdmidev, "%s %02x\n", __func__, ev);

	LOCK_HDMI_EVENTS;
	events &= ~ev & EVENTS_MASK;
	UNLOCK_HDMI_EVENTS;

	return 0;
}

/*****************************************************************************
 *	name	=	audiocfg
 *	func	=	Function to store audio configuration
 *	input	=	struct audio_cfg *cfg
 *	output	=	None
 *	return	=	-EINVAL, 0
 *****************************************************************************/
static int audiocfg(struct audio_cfg *cfg)
{
	union av7100_configuration config;
	struct av7100_status status;

	status = av7100_status_get();
	if (status.av7100_state < AV7100_OPMODE_STANDBY) {
		if (av7100_powerup() != 0) {
			dev_err(hdmidev, "av7100_powerup failed\n");
			return -EINVAL;
		}
	}

	if (status.av7100_state < AV7100_OPMODE_INIT) {
		if (av7100_download_firmware(NULL, 0, I2C_INTERFACE) != 0) {
			dev_err(hdmidev, "av7100 dl fw FAIL\n");
			return -EINVAL;
		}
	}

	config.audio_input_format.audio_input_if_format	= cfg->if_format;
	config.audio_input_format.i2s_input_nb		= cfg->i2s_entries;
	config.audio_input_format.sample_audio_freq	= cfg->freq;
	config.audio_input_format.audio_word_lg		= cfg->word_length;
	config.audio_input_format.audio_format		= cfg->format;
	config.audio_input_format.audio_if_mode		= cfg->if_mode;
	config.audio_input_format.audio_mute		= cfg->mute;

	if (av7100_conf_prep(AV7100_COMMAND_AUDIO_INPUT_FORMAT,
		&config) != 0) {
		dev_err(hdmidev, "av7100_conf_prep FAIL\n");
		return -EINVAL;
	}

	if (av7100_conf_w(AV7100_COMMAND_AUDIO_INPUT_FORMAT,
		NULL, NULL, I2C_INTERFACE) != 0) {
		dev_err(hdmidev, "av7100_conf_w FAIL\n");
		return -EINVAL;
	}

	return 0;
}

/*****************************************************************************
 *	name	=	show_state_hdmi
 *	func	=	Returns plun state of HDMI
 *	input	=	struct device *dev,struct device_attribute *attr, 
 *			char *buf
 *	output	=	None
 *	return	=	ret
 *****************************************************************************/
static ssize_t show_state_hdmi(struct device *dev,
                struct device_attribute *attr, char *buf){

	int ret  = 0;
	ret = sprintf(buf,"%d",plugedin_state);
    return ret;

}

/*****************************************************************************
 *	name	=	hdmi_open
 *	func	=	Function to open HDMI device
 *	input	=	struct inode *inode, struct file *filp
 *	output	=	None
 *	return	=	-EBUSY, 0
 *****************************************************************************/
static int hdmi_open(struct inode *inode, struct file *filp)
{
	if (device_open)
		return -EBUSY;

	device_open++;

	return 0;
}

/*****************************************************************************
 *	name	=	hdmi_release
 *	func	=	Function to close HDMI device 
 *	input	=	struct inode *inode, struct file *filp
 *	output	=	None
 *	return	=	0
 *****************************************************************************/
static int hdmi_release(struct inode *inode, struct file *filp)
{
	if (device_open)
		device_open--;

	return 0;
}

/*****************************************************************************
 *	name	=	hdmi_ioctl
 *	func	=	Function to send commands from user space
 *	input	=	struct inode *inode, struct file *file,
		        unsigned int cmd, unsigned long arg
 *	output	=	None
 *	return	=	-EFAULT, -ENOTTY, ret
 *****************************************************************************/
static long hdmi_ioctl(struct file *file,
		       unsigned int cmd, unsigned long arg)
{
	int ret = -EFAULT;
	int i;
	switch (cmd) {
	case IOC_AUTHENTICATE_HDCP:
		if(key_programmed_flag){
			ret = authenticateHDCP(HDMI_HDCP_AUTH_ON);
			if (ret < 0) 
				ret = -EFAULT;
		}
		break;
	case IOC_DEAUTHENTICATE_HDCP:
		if(key_programmed_flag){
			ret = authenticateHDCP(HDMI_HDCP_AUTH_OFF);
			if (ret < 0)
				ret = -EFAULT;
		}
		break;
	case IOC_SET_AESHDCP_KEY:

		/*Get AESHDCP key from user side*/
		if (copy_from_user(&hdcp_ldaes, (void *)arg,
			sizeof(struct hdcp_loadaesall)))
			ret = -EFAULT;

		break;
	case IOC_SET_REVOCATION_LIST:
		/*to be implemented by OEM*/
		break;
	default:
		ret = -ENOTTY; /*Ioctl command is not supported*/
		break;
	}
	return ret;

}

/*****************************************************************************
 *	name	=	hdmi_poll
 *	func	=	Function to poll for hdmi event 
 *	input	=	struct file *filp, poll_table *wait
 *	output	=	None
 *	return	=	mask
 *****************************************************************************/
static unsigned int
hdmi_poll(struct file *filp, poll_table *wait)
{
	unsigned int mask = 0;

	dev_dbg(hdmidev, "%s\n", __func__);

	poll_wait(filp, &hdmi_event_wq , wait);

	LOCK_HDMI_EVENTS;
	if (events_received == true) {
		events_received = false;
		mask = POLLIN | POLLRDNORM;
	}
	UNLOCK_HDMI_EVENTS;

	return mask;
}


static const struct file_operations hdmi_fops = {
	.owner =    THIS_MODULE,
	.open =     hdmi_open,
	.release =  hdmi_release,
	.poll = hdmi_poll,
        .unlocked_ioctl = hdmi_ioctl
};

static struct miscdevice hdmi_miscdev = {
	MISC_DYNAMIC_MINOR,
	"hdmi",
	&hdmi_fops
};

/*****************************************************************************
 *	name	=	hdmi_event
 *	func	=	Event call back function called by hardware driver
 *	input	=	enum av7100_hdmi_event ev
 *	output	=	None
 *	return	=	void
 *****************************************************************************/
void hdmi_event(enum av7100_hdmi_event ev)
{
	int events_old = 0;
	int events_new =0;
	struct av7100_status status;
	struct kobject *kobj = &hdmidev->kobj;

	u8 i;
	u8 hdmi_plugin_state;
	enum av7100_hdmi_mode hdmi_mode;
	enum av7100_output_CEA_VESA av7100_output_format;
	int set_format_result = 0;	
	u8 oni = 0;
#if 0
	u8 hdcpi = 0;
	u8 hdcps = 0;
	u8 progged = 0;
	int block_cnt = 0;
	u32 crcReceived = 0;
	/* Calculated from unencrypted keys using CRC-32 command */
	u32 crcSent = 0x02BEE20D;
	u8 crc1,crc2,crc3,crc4;

	struct hdcp_loadaesone hdcp_loadaesone;
#endif

	LOCK_HDMI_EVENTS;

	events_old = events;

	/* Set event */
	switch (ev) {
	case AV7100_HDMI_EVENT_HDMI_PLUGIN:	
				#if 1
				status = av7100_status_get();
				/* Just download only when plugging in for first time 
				   after power up */
				
				if (status.av7100_state < AV7100_OPMODE_INIT) {
					struct clk *clk;
					u8 stby;
					/* Enable clock input to AV7100 and set rate*/
					clk = clk_get(NULL, "vclk5_clk");
					clk_set_rate(clk, clk_round_rate(clk, 36000000));
					
					clk_enable(clk);
					clk_put(clk);
					/* Just to make sure av7100 is in correct state */
					if (av7100_reg_stby_r(&stby, NULL, NULL)) {
						dev_err(hdmidev, "stby read FAIL\n");
						return ;
					}
					if (!stby) {
						dev_err(hdmidev, "av7100 not running\n");
						return ;
					}
					
					if(av7100_reg_stby_pend_int_r(NULL,&oni,NULL)) {
					     dev_err(hdmidev, "av7100 error reading Firmware ready\n");
						 return ;
				        }

					if (av7100_download_firmware(NULL, 0, I2C_INTERFACE) != 0) {
					  dev_err(hdmidev, "av7100 dl fw FAIL\n");
					  return ;
				        }

#if 0
					/* Check a secret AES-128 bit in AV8100 */
					if(hdcp_chkaesotp(&progged)) {
						key_programmed_flag = 0;
						return;
					}
					if (progged == 0) {
						key_programmed_flag = 0;
						return;
		                        }						
								
								
					/* Load KSV key */
					memset(hdcp_loadaesone.key, 0, HDMI_HDCP_AES_KEYSIZE);
					memcpy(hdcp_loadaesone.key + HDMI_HDCP_AES_KSVZEROESSIZE,
									hdcp_ldaes.ksv, HDMI_HDCP_AES_KSVSIZE);
							
					if (hdcp_loadaes(HDMI_HDCP_KSV_BLOCK,
						HDMI_HDCP_AES_KSVSIZE + HDMI_HDCP_AES_KSVZEROESSIZE,
						hdcp_loadaesone.key,
							&hdcp_loadaesone.result))
								return;
							
					hdcp_ldaes.result = hdcp_loadaesone.result;
								
					/* Load AES HDCP key */

					while (block_cnt < HDMI_HDCP_AES_NR_OF_BLOCKS) {
						memcpy(hdcp_loadaesone.key, hdcp_ldaes.key +
						block_cnt * HDMI_HDCP_AES_KEYSIZE,
									HDMI_HDCP_AES_KEYSIZE);
										
						if (hdcp_loadaes(block_cnt + HDMI_HDCP_AES_BLOCK_START,
										HDMI_HDCP_AES_KEYSIZE,
										hdcp_loadaesone.key,
										&hdcp_loadaesone.result))
							return;
									
						if(hdcp_loadaesone.result)
							return;
						
							block_cnt ++;
					}

					register_read_internal( AV7100_COMMAND_OFFSET + 2,&crc1 );
					register_read_internal( AV7100_COMMAND_OFFSET + 3,&crc2 );
					register_read_internal( AV7100_COMMAND_OFFSET + 4,&crc3 );
					register_read_internal( AV7100_COMMAND_OFFSET + 5,&crc4 );

					crcReceived = ( crc1 << 24 ) | ( crc2 << 16 ) | ( crc3 << 8 ) | ( crc4 );
					
					if(crcReceived == crcSent) {
		                 		key_programmed_flag = 1;
					}
					else {
						key_programmed_flag = 0;
					}
						 
						
					if(hdcp_loadaesone.result)
						return;

					if (av7100_reg_gen_int_r(NULL, NULL, NULL, NULL, &hdcpi, NULL))
						return;
						
					if (hdcpi == 1) {
						
						/* Read general interrupt register */
						if (av7100_reg_gen_status_r(NULL, NULL, NULL, NULL, &hdcps))
							return;
						
						/* Control HDCP state change */
						changeHDCPstatus(hdcps);
						
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
#endif
					
				}
	
#if 0
				if(selectFormatEdidBased() != 0 ) {
				   dev_err(hdmidev, "Edid based format selection failed\n");
				   return ;
				} 
#endif

				
				setHdmiState(AV7100_HDMI_ON);  
				#endif
				plugedin_state = 1; 
				
				switch_set_state(&s_dev,1);
				events |= events_mask & HDMI_EVENT_HDMI_PLUGIN;
				printk("HDMI cable Plugged In \n");
      
		break;

	case AV7100_HDMI_EVENT_HDMI_PLUGOUT:
                plugedin_state = 0;
              
                setHdmiState(AV7100_HDMI_OFF); 
		        switch_set_state(&s_dev,0);
		        events |= events_mask & HDMI_EVENT_HDMI_PLUGOUT;
                        printk("HDMI cable Plugged Out\n");
		break;

	default:
		break;
	}

	events_new = events;

	UNLOCK_HDMI_EVENTS;



	if (events_new != events_old) {
		/* Wake up application waiting for event via call to poll() */
		sysfs_notify(kobj, NULL, SYSFS_EVENT_FILENAME);

		LOCK_HDMI_EVENTS;
		events_received = true;
		UNLOCK_HDMI_EVENTS;

		wake_up_interruptible(&hdmi_event_wq);
	}
}

/*****************************************************************************
 *	name	=	hdmi_init
 *	func	=	Function to register hdmi driver as miscellaneous 
 *			and switch class driver
 *	input	=	void
 *	output	=	None
 *	return	=	-ENOMEM, ret
 *****************************************************************************/
int __init hdmi_init(void)
{
	int ret;
	struct hdmi_driver_data *hdmi_driver_data;

	ret = misc_register(&hdmi_miscdev);
	if (ret)
		goto hdmi_init_out;

	ret = switch_dev_register(&s_dev);
	if(ret)
		goto hdmi_init_out;	

	hdmidev = hdmi_miscdev.this_device;
	switchdev = s_dev.dev;

	hdmi_driver_data =
		kzalloc(sizeof(struct hdmi_driver_data), GFP_KERNEL);

	if (!hdmi_driver_data)
		return -ENOMEM;

	dev_set_drvdata(hdmidev, hdmi_driver_data);

	/* Default sysfs file format is hextext */
	hdmi_driver_data->store_as_hextext = true;

	init_waitqueue_head(&hdmi_event_wq);

    if (device_create_file(hdmidev, &dev_attr_state))
		dev_info(hdmidev, "Unable to create state attribute\n");
	if (device_create_file(switchdev,&dev_attr_info))
		dev_info(switchdev,"Unable to create switch info attribute\n");

	/* Register event callback */
/*	av7100_hdmi_event_cb_set(hdmi_event);
        av7100_enable_interrupt(); */

hdmi_init_out:
	return ret;
}

/*****************************************************************************
 *	name	=	hdmi_exit
 *	func	=	Function to deregister callback function and remove 
 *			hdmi driver as miscellaneous and switch class driver
 *	input	=	void
 *	output	=	None
 *	return	=	void
 *****************************************************************************/
void hdmi_exit(void)
{
	struct hdmi_driver_data *hdmi_driver_data;

	/* Deregister event callback */
	av7100_hdmi_event_cb_set(NULL);


	device_remove_file(hdmidev, &dev_attr_state);
	device_remove_file(switchdev, &dev_attr_info);

	hdmi_driver_data = dev_get_drvdata(hdmidev);
	kfree(hdmi_driver_data);

	misc_deregister(&hdmi_miscdev);
	switch_dev_unregister(&s_dev);
}

/*****************************************************************************
 *	name	=	setHdmiState
 *	func	=	Interface function to set HDMI plug state
 *	input	=	enum av7100_hdmi_mode AV7100_HDMI_STATE
 *	output	=	None
 *	return	=	-EINVAL, 0
 *****************************************************************************/
int setHdmiState(enum av7100_hdmi_mode AV7100_HDMI_STATE)
{

        union av7100_configuration config;
        memset(&config,0,sizeof(union av7100_configuration));

        if (av7100_conf_get(AV7100_COMMAND_HDMI, &config) != 0) {
          	dev_err(hdmidev, "av7100_conf_get FAIL\n");
		return -EINVAL;
        }

	if((AV7100_HDMI_STATE == AV7100_HDMI_OFF) || (AV7100_HDMI_STATE == AV7100_HDMI_ON)){
                
	        config.hdmi_format.hdmi_mode    = AV7100_HDMI_STATE;
                
       		if (av7100_conf_prep(AV7100_COMMAND_HDMI, &config) != 0) {
               		dev_err(hdmidev, "av7100_conf_prep FAIL\n");
			return -EINVAL;
        	}
			if(plugedin_state == 1)
			{
				if (av7100_conf_w(AV7100_COMMAND_HDMI, NULL, NULL,I2C_INTERFACE) != 0) 
				{
        	      	dev_err(hdmidev, "av7100_conf_w FAIL\n");
					return -EINVAL;
				}	
			}

		return 0;
	}else{
		return -EINVAL;

	}
}

/*****************************************************************************
 *	name	=	hdmi_get_hpd_state
 *	func	=	Interface function to get current HDMI hot plug state
 *	input	=	None
 *	output	=	None
 *	return	=	s_dev.state
 *****************************************************************************/
int hdmi_get_hpd_state(){

	return s_dev.state;	

}	

/*****************************************************************************
 *	name	=	get_hdmi_state
 *	func	=	Interface function to get current HDMI state
 *	input	=	None
 *	output	=	None
 *	return	=	-EINVAL, config.hdmi_format.hdmi_mode
 *****************************************************************************/
enum av7100_hdmi_mode get_hdmi_state(){

	union av7100_configuration config;

	memset(&config,0,sizeof(union av7100_configuration));

	if (av7100_conf_get(AV7100_COMMAND_HDMI, &config) != 0) {
        	dev_err(hdmidev, "av7100_conf_get FAIL\n");
		return -EINVAL;						
        }

	return config.hdmi_format.hdmi_mode;

}

/*****************************************************************************
 *	name	=	hdmi_get_video_output_format
 *	func	=	Interface function to get the current output format
 *	input	=	None
 *	output	=	None
 *	return	=	-EINVAL,config.video_output_format.video_output_cea_vesa
 *****************************************************************************/
enum av7100_output_CEA_VESA hdmi_get_video_output_format(){

	union av7100_configuration config;

	memset(&config, 0, sizeof(union av7100_configuration));

        if(av7100_conf_get(AV7100_COMMAND_VIDEO_OUTPUT_FORMAT, &config) != 0) {
     		dev_err(hdmidev, "av7100_conf_get video input FAIL\n");
		return -EINVAL;
        }

	return config.video_output_format.video_output_cea_vesa; 

}

/*****************************************************************************
 *	name	=	hdmi_set_video_output_format
 *	func	=	Interface function to set the current output format
 *	input	=	enum av7100_output_CEA_VESA output_format
 *	output	=	None
 *	return	=	-EINVAL, 0
 *****************************************************************************/
int hdmi_set_video_output_format(enum av7100_output_CEA_VESA output_format){

	union av7100_configuration config;

	memset(&config,0,sizeof(union av7100_configuration));

	if((output_format == AV7100_CEA4_1280X720P_60HZ) || (output_format == AV7100_CEA32_1920X1080P_24HZ)){

		config.video_output_format.video_output_cea_vesa = output_format;

		if(av7100_conf_prep(AV7100_COMMAND_VIDEO_OUTPUT_FORMAT, &config)){
			dev_err(hdmidev, "av7100_conf_get video input FAIL\n");
         	       return -EINVAL;
        	}

		if(av7100_conf_w(AV7100_COMMAND_VIDEO_OUTPUT_FORMAT,NULL,NULL,I2C_INTERFACE) != 0){
		      dev_err(hdmidev, "av7100_conf_w output FAIL\n");
		      return -EINVAL;       
   		}


	}else{
		return -EINVAL;

	}

	return 0;

}

/*****************************************************************************
 *	name	=	get_supported_formats
 *	func	=	Function that returns the supported output formats 
 * 			by HDMI display
 *	input	=	struct dtd *supported_formats
 *	output	=	None
 *	return	=	void
 *****************************************************************************/
void get_supported_formats(struct dtd *supported_formats){


	union av7100_configuration config;
	int count = 0;
	int ret = 0;
	u8 len=128;
	u8 data[128];
	u8 data1[128];
	u8 i;
	u8 timingDescriptorStart=0;

	memset(&config,0,sizeof(union av7100_configuration));
	
	config.edid_section_readback_format.address = 0xA0;
	config.edid_section_readback_format.block_number = 0;

	ret = av7100_conf_prep(AV7100_COMMAND_EDID_SECTION_READBACK, &config);

	if(av7100_conf_w(AV7100_COMMAND_EDID_SECTION_READBACK,&len,(unsigned char *)&data,I2C_INTERFACE) != 0){
		dev_err(hdmidev,"av7100_conf_write1 FAIL\n");
	}

	len = 128;
	memset(&config,0,sizeof(union av7100_configuration));

	config.edid_section_readback_format.address = 0xA0;
        config.edid_section_readback_format.block_number = 1;

        ret = av7100_conf_prep(AV7100_COMMAND_EDID_SECTION_READBACK, &config);

        if(av7100_conf_w(AV7100_COMMAND_EDID_SECTION_READBACK,&len,(unsigned char *)&data1,I2C_INTERFACE) != 0){
                dev_err(hdmidev,"av7100_conf_write1 FAIL\n");
        }       


 	supported_formats[0].pixelFrequency = 
		data[FIRST_TIMING_DESCRIPTOR_START+PIXEL_FREQUENCY_LOWBYTE]|
		(data[FIRST_TIMING_DESCRIPTOR_START+PIXEL_FREQUENCY_HIGHBYTE] << 8);
   	supported_formats[0].hActive = 
	        data[FIRST_TIMING_DESCRIPTOR_START+HACTIVE_LOWBYTE] | 
        	((data[FIRST_TIMING_DESCRIPTOR_START+HACTIVE_HIGHNIBBLE] & 0xf0) << 4);
	supported_formats[0].hBlank = 
        	data[FIRST_TIMING_DESCRIPTOR_START+HBLANK_LOWBYTE] |
        	((data[FIRST_TIMING_DESCRIPTOR_START+HBLANK_HIGHNIBBLE] & 0x0f) << 8);
	supported_formats[0].vActive =
	        data[FIRST_TIMING_DESCRIPTOR_START+VACTIVE_LOWBYTE] |  
 	        ((data[FIRST_TIMING_DESCRIPTOR_START+VACTIVE_HIGHNIBBLE] & 0xf0) << 4);
	supported_formats[0].vBlank = 
	        (unsigned short)data[FIRST_TIMING_DESCRIPTOR_START+VBLANK_LOWBYTE] | 
        	((data[FIRST_TIMING_DESCRIPTOR_START+VBLANK_HIGHNIBBLE] & 0x0f) << 8);
	supported_formats[0].videoMode =
	        (data[FIRST_TIMING_DESCRIPTOR_START+VIDEOMODE] & 0x80) >> 7;

	supported_formats[1].pixelFrequency = 
        	data[FIRST_TIMING_DESCRIPTOR_START+PIXEL_FREQUENCY_LOWBYTE] | 
	        (data[SECOND_TIMING_DESCRIPTOR_START+PIXEL_FREQUENCY_HIGHBYTE] << 8);
	supported_formats[1].hActive = 
	        data[SECOND_TIMING_DESCRIPTOR_START+HACTIVE_LOWBYTE] | 
	        ((data[SECOND_TIMING_DESCRIPTOR_START+HACTIVE_HIGHNIBBLE] & 0xf0) << 4);
	supported_formats[1].hBlank = 
	        data[SECOND_TIMING_DESCRIPTOR_START+HBLANK_LOWBYTE] | 
	        ((data[SECOND_TIMING_DESCRIPTOR_START+HBLANK_HIGHNIBBLE] & 0x0f) << 8);
	supported_formats[1].vActive = 
	        data[SECOND_TIMING_DESCRIPTOR_START+VACTIVE_LOWBYTE] | 
	        ((data[SECOND_TIMING_DESCRIPTOR_START+VACTIVE_HIGHNIBBLE] & 0xf0) << 4);
	supported_formats[1].vBlank = 
	        data[SECOND_TIMING_DESCRIPTOR_START+VBLANK_LOWBYTE] | 
	        ((data[SECOND_TIMING_DESCRIPTOR_START+VBLANK_HIGHNIBBLE] & 0x0f) << 8);
	supported_formats[1].videoMode = 
       		(data[SECOND_TIMING_DESCRIPTOR_START+VIDEOMODE] & 0x80) >> 7;

	timingDescriptorStart = data1[2];

	if(data1[2] != 0){

		for(i=2;i<4;i++){
			int index = timingDescriptorStart + 18 * (i-2);
			
			supported_formats[i].pixelFrequency = 
				(unsigned long)data1[index+PIXEL_FREQUENCY_LOWBYTE] |
				((unsigned long)data1[index+PIXEL_FREQUENCY_HIGHBYTE] << 8);
			supported_formats[i].hActive = 
			        (unsigned short)data1[index+HACTIVE_LOWBYTE] | 
			        ((data1[index+HACTIVE_HIGHNIBBLE] & 0xf0) << 4);
			supported_formats[i].hBlank = 
			        (unsigned short)data1[index+HBLANK_LOWBYTE] | 
			        ((data1[index+HBLANK_HIGHNIBBLE] & 0x0f) << 8);
		        supported_formats[i].vActive = 
			        (unsigned short)data1[index+VACTIVE_LOWBYTE] | 
			        ((data1[index+VACTIVE_HIGHNIBBLE] & 0xf0) << 4);
		        supported_formats[i].vBlank = 
		                data1[index+VBLANK_LOWBYTE] | 
				((data1[index+VBLANK_HIGHNIBBLE] & 0x0f) << 8);
		        supported_formats[i].videoMode = (data1[index+VIDEOMODE] & 0x80) >> 7;	
		}

	}


}

/*****************************************************************************
 *	name	=	hdmi_set_audio_input_format
 *	func	=	Interface used to set audio input format
 *	input	=	u8 format
 *	output	=	None
 *	return	=	0
 *****************************************************************************/
u8 hdmi_set_audio_input_format(u8 format){
	return 0;
}

/*****************************************************************************
 *	name	=	hdmi_get_audio_input_format
 *	func	=	Interface used to get audio input format
 *	input	=	void
 *	output	=	None
 *	return	=	0
 *****************************************************************************/
u8 hdmi_get_audio_input_format(){
	return 0;
}
