/*
 * /drivers/video/av7100/av7100.h
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

#ifndef __AV7100__H__
#define __AV7100__H__

#define AV7100_PF_NAME					"av7100_pf"

#define AV7100_HDCP_SEND_KEY_SIZE			16
#define AV7100_INFOFRAME_SIZE				28
#define AV7100_FUSE_KEY_SIZE				16

#define AV7100_HDMI_ON_TIME					0
#define AV7100_HDMI_OFF_TIME				0
#define AV7100_COMMAND_OFFSET				0x10
#define AV7100_COMMAND_MAX_LENGTH			0x81

#define AV7100_TE_LINE_NB_14				14
#define AV7100_TE_LINE_NB_17				17
#define AV7100_TE_LINE_NB_18				18
#define AV7100_TE_LINE_NB_21				21
#define AV7100_TE_LINE_NB_22				22
#define AV7100_TE_LINE_NB_30				30
#define AV7100_TE_LINE_NB_38				38
#define AV7100_TE_LINE_NB_40				40
#define AV7100_UI_X4_DEFAULT				6

#define HDMI_REQUEST_FOR_REVOCATION_LIST_INPUT		2
#define HDMI_HDCP_SEND_KEY_SIZE				7
#define HDMI_INFOFRAME_DATA_SIZE			28

#define REG_16_8_LSB(p)					((u8)(p & 0xFF))
#define REG_16_8_MSB(p)					((u8)((p & 0xFF00)>>8))
#define REG_32_8_MSB(p)					((u8)((p & 0xFF000000)>>24))
#define REG_32_8_MMSB(p)				((u8)((p & 0x00FF0000)>>16))
#define REG_32_8_MLSB(p)				((u8)((p & 0x0000FF00)>>8))
#define REG_32_8_LSB(p)					((u8)(p & 0x000000FF))
#define REG_10_8_MSB(p)					((u8)((p & 0x300)>>8))
#define REG_12_8_MSB(p)					((u8)((p & 0xf00)>>8))

enum av7100_dsi_mode {
	AV7100_HDMI_DSI_OFF,
	AV7100_HDMI_DSI_COMMAND_MODE,
	AV7100_HDMI_DSI_VIDEO_MODE
};

struct av7100_color_space_conversion_format_cmd {
	unsigned short		c0;
	unsigned short		c1;
	unsigned short		c2;
	unsigned short		c3;
	unsigned short		c4;
	unsigned short		c5;
	unsigned short		c6;
	unsigned short		c7;
	unsigned short		c8;
	unsigned short		aoffset;
	unsigned short		boffset;
	unsigned short		coffset;
	unsigned char		lmax;
	unsigned char		lmin;
	unsigned char		cmax;
	unsigned char		cmin;
};

struct av7100_hdcp_send_key_format_cmd {
	unsigned char		key_number;
	unsigned char		data_len;
	unsigned char		data[AV7100_HDCP_SEND_KEY_SIZE];
};

struct av7100_hdcp_management_format_cmd {
	unsigned char		req_type;
	unsigned char		req_encr;
};

/**
 * struct av7100_cea - CEA(consumer electronic access) standard structure
 * @cea_id:
 * @cea_nb:
 * @vtotale:
 **/

struct av7100_cea {
	char	cea_id[40];
	int	cea_nb;
	int	vtotale;
	int	vactive;
	int	vsbp;
	int	vslen;
	int	vsfp;
	char	vpol[5];
	int	htotale;
	int	hactive;
	int	hbp;
	int	hslen;
	int	hfp;
	int	frequence;
	char	hpol[5];
	int	reg_line_duration;
	int	blkoel_duration;
	int	uix4;
	int	pll_mult;
	int	pll_div;
};

struct av7100_infoframes_format_cmd {
	unsigned char	type;
	unsigned char	version;
	unsigned char	length;
	unsigned char	crc;
	unsigned char	data[AV7100_INFOFRAME_SIZE];
};

struct av7100_edid_section_readback_format_cmd {
	unsigned char	address;
	unsigned char	block_number;
};

struct av7100_fuse_aes_key_format_cmd {
	unsigned char	fuse_operation;
	unsigned char	key[AV7100_FUSE_KEY_SIZE];
};

enum av7100_error {
	AV7100_OK = 0x0,
	AV7100_INVALID_COMMAND,
	AV7100_INVALID_IOCTL,
	AV7100_COMMAND_FAIL,
	AV7100_FWDOWNLOAD_FAIL,
	AV7100_FAIL = 0xFF
};

enum av7100_command_type {
	AV7100_COMMAND_VIDEO_INPUT_FORMAT  = 0x1,
	AV7100_COMMAND_AUDIO_INPUT_FORMAT,
	AV7100_COMMAND_VIDEO_OUTPUT_FORMAT,
	AV7100_COMMAND_COLORSPACECONVERSION = 0x05,
	AV7100_COMMAND_HDMI = 0x09,
	AV7100_COMMAND_HDCP_SENDKEY,
	AV7100_COMMAND_HDCP_MANAGEMENT,
	AV7100_COMMAND_INFOFRAMES,
	AV7100_COMMAND_EDID_SECTION_READBACK,
	AV7100_COMMAND_PATTERNGENERATOR,
	AV7100_COMMAND_FUSE_AES_KEY
};

enum av7100_pixel_format {
	AV7100_INPUT_PIX_RGB565,
	AV7100_INPUT_PIX_RGB666,
	AV7100_INPUT_PIX_RGB666P,
	AV7100_INPUT_PIX_RGB888,
	AV7100_INPUT_PIX_YCBCR422
};

enum av7100_video_mode {
	AV7100_VIDEO_INTERLACE,
	AV7100_VIDEO_PROGRESSIVE
};

enum av7100_dsi_nb_data_lane {
	AV7100_DATA_LANES_USED_0,
	AV7100_DATA_LANES_USED_1,
	AV7100_DATA_LANES_USED_2,
	AV7100_DATA_LANES_USED_3,
	AV7100_DATA_LANES_USED_4
};

enum av7100_te_config {
	AV7100_TE_IT_LINE = 0x02,
	AV7100_TE_GPIO_IT = 0x04
};

enum av7100_audio_if_format {
	AV7100_AUDIO_I2S_MODE,
	AV7100_AUDIO_I2SDELAYED_MODE,
	AV7100_AUDIO_TDM_MODE
};

enum av7100_sample_freq {
	AV7100_AUDIO_FREQ_32KHZ,
	AV7100_AUDIO_FREQ_44_1KHZ,
	AV7100_AUDIO_FREQ_48KHZ,
	AV7100_AUDIO_FREQ_88_2KHZ,
	AV7100_AUDIO_FREQ_96KHZ,
	AV7100_AUDIO_FREQ_176_4KHZ,
	AV7100_AUDIO_FREQ_192KHZ
};

enum av7100_audio_word_length {
	AV7100_AUDIO_16BITS,
	AV7100_AUDIO_20BITS,
	AV7100_AUDIO_24BITS,
	AV7100_AUDIO_OVER_24BITS
};

enum av7100_audio_format {
	AV7100_AUDIO_LPCM_MODE,
	AV7100_AUDIO_COMPRESS_MODE
};

enum av7100_audio_if_mode {
	AV7100_AUDIO_SLAVE,
	AV7100_AUDIO_MASTER
};

enum av7100_audio_mute {
	AV7100_AUDIO_MUTE_DISABLE,
	AV7100_AUDIO_MUTE_ENABLE
};

enum av7100_output_cea_vesa {
	AV7100_CUSTOM,
	AV7100_VESA4_640X480P_60HZ,
	AV7100_CEA2_3_720X480P_60HZ,
	AV7100_CEA4_1280X720P_60HZ,
	AV7100_CEA5_1920X1080I_60HZ,
	AV7100_CEA6_7_720X480I_60HZ,
	AV7100_CEA14_15_440X480P_60HZ,
	AV7100_CEA16_1920X1080P_60HZ,
	AV7100_CEA17_18_720X576P_50HZ,
	AV7100_CEA19_1280X720P_50HZ,
	AV7100_CEA20_1920X1080I_50HZ,
	AV7100_CEA21_22_720X576I_50HZ,
	AV7100_CEA29_30_1440X576P_50HZ,
	AV7100_CEA31_1920x1080P_50Hz,
	AV7100_CEA32_1920X1080P_24HZ,
	AV7100_CEA33_1920X1080P_25HZ,
	AV7100_CEA34_1920X1080P_30HZ,
	AV7100_CEA60_1280X720P_24HZ,
	AV7100_CEA61_1280X720P_25HZ,
	AV7100_CEA62_1280X720P_30HZ,
	AV7100_VESA9_800X600P_60HZ,
	AV7100_VESA14_848X480P_60HZ,
	AV7100_VESA16_1024X768P_60HZ,
	AV7100_VESA22_1280X768P_60HZ,
	AV7100_VESA23_1280X768P_60HZ,
	AV7100_VESA27_1280X800P_60HZ,
	AV7100_VESA28_1280X800P_60HZ,
	AV7100_VESA39_1360X768P_602HZ,
	AV7100_VESA81_1366X768P_60HZ,
	AV7100_VIDEO_OUTPUT_CEA_VESA_MAX
};

enum av7100_video_sync_pol {
	AV7100_SYNC_POSITIVE,
	AV7100_SYNC_NEGATIVE
};

enum av7100_hdmi_mode {
	AV7100_HDMI_OFF,
	AV7100_HDMI_ON,
	AV7100_HDMI_AVMUTE
};

enum av7100_hdmi_format {
	AV7100_HDMI,
	AV7100_DVI
};

enum av7100_dvi_format {
	AV7100_DVI_CTRL_CTL0,
	AV7100_DVI_CTRL_CTL1,
	AV7100_DVI_CTRL_CTL2
};

enum av7100_hdcp_auth_req_type {
	AV7100_HDCP_AUTH_REQ_OFF = 0,
	AV7100_HDCP_AUTH_REQ_ON = 1,
	AV7100_HDCP_REV_LIST_REQ = 2,
	AV7100_HDCP_AUTH_CONT = 3
};

enum av7100_hdcp_encr_use {
	AV7100_HDCP_ENCR_USE_OESS = 0,
	AV7100_HDCP_ENCR_USE_EESS = 1,
};

enum av7100_pattern_type {
	AV7100_PATTERN_OFF,
	AV7100_PATTERN_GENERATOR,
	AV7100_PRODUCTION_TESTING
};

enum av7100_pattern_format {
	AV7100_NO_PATTERN,
	AV7100_PATTERN_VGA,
	AV7100_PATTERN_720P,
	AV7100_PATTERN_1080P
};

enum av7100_pattern_audio {
	AV7100_PATTERN_AUDIO_OFF,
	AV7100_PATTERN_AUDIO_ON
};

enum av7100_fuse_operation {
	AV7100_FUSE_READ = 0,
	AV7100_FUSE_WRITE = 1
};

enum av7100_operating_mode {
	AV7100_OPMODE_UNDEFINED = 0,
	AV7100_OPMODE_SHUTDOWN,
	AV7100_OPMODE_STANDBY,
	AV7100_OPMODE_SCAN,
	AV7100_OPMODE_INIT,
	AV7100_OPMODE_IDLE,
	AV7100_OPMODE_VIDEO
};

enum hdmi_hdcp_auth_type {
	HDMI_HDCP_AUTH_OFF = 0,
	HDMI_HDCP_AUTH_ON = 1,
	HDMI_HDCP_AUTH_CONT = 2
};

enum av7100_command_size {
	AV7100_COMMAND_VIDEO_INPUT_FORMAT_SIZE  = 0x17,
	AV7100_COMMAND_AUDIO_INPUT_FORMAT_SIZE  = 0x8,
	AV7100_COMMAND_VIDEO_OUTPUT_FORMAT_SIZE = 0x1E, 
	AV7100_COMMAND_COLORSPACECONVERSION_SIZE = 0x1D,
	AV7100_COMMAND_HDMI_SIZE = 0x4,
	AV7100_COMMAND_HDCP_MANAGEMENT_SIZE = 0x3,
	AV7100_COMMAND_INFOFRAMES_SIZE = 0x21,
	AV7100_COMMAND_EDID_SECTION_READBACK_SIZE  = 0x3,
	AV7100_COMMAND_PATTERNGENERATOR_SIZE  = 0x4,
	AV7100_COMMAND_FUSE_AES_KEY_SIZE = 0x12,
};

enum hpd_state {
	HPD_STATE_UNPLUGGED = 0,
	HPD_STATE_PLUGGED = 1
};

struct av7100_pattern_generator_format_cmd {
	enum av7100_pattern_type	pattern_type;
	enum av7100_pattern_format	pattern_video_format;
	enum av7100_pattern_audio	pattern_audio_mode;
};

struct av7100_audio_input_format_cmd {
	enum av7100_audio_if_format		audio_input_if_format;
	unsigned char					i2s_input_nb;
	enum av7100_sample_freq			sample_audio_freq;
	enum av7100_audio_word_length	audio_word_lg;
	enum av7100_audio_format		audio_format;
	enum av7100_audio_if_mode		audio_if_mode;
	enum av7100_audio_mute			audio_mute;
};

struct av7100_video_output_format_cmd {
	enum av7100_output_cea_vesa	video_output_cea_vesa;
	enum av7100_video_sync_pol	vsync_polarity;
	enum av7100_video_sync_pol	hsync_polarity;
	unsigned short				total_horizontal_pixel;
	unsigned short				total_horizontal_active_pixel;
	unsigned short				total_vertical_in_half_lines;
	unsigned short				total_vertical_active_in_half_lines;
	unsigned short				hsync_start_in_pixel;
	unsigned short				hsync_length_in_pixel;
	unsigned short				vsync_start_in_half_line;
	unsigned short				vsync_length_in_half_line;
	unsigned short				hor_video_start_pixel;
	unsigned short				vert_video_start_pixel;
	enum av7100_video_mode		video_type;
	unsigned short				pixel_repeat;
	unsigned long				pixel_clock_freq_Hz;
};

struct av7100_hdmi_cmd {
	enum av7100_hdmi_mode		hdmi_mode;
	enum av7100_hdmi_format		hdmi_format;
	enum av7100_dvi_format		dvi_format; /* used only if HDMI_format = DVI*/
};

struct av7100_video_input_format_cmd {
	enum av7100_dsi_mode		dsi_input_mode;
	enum av7100_pixel_format	input_pixel_format;
	unsigned short				total_horizontal_pixel;
	unsigned short				total_horizontal_active_pixel;
	unsigned short				total_vertical_lines;
	unsigned short				total_vertical_active_lines;
	enum av7100_video_mode		video_mode;
	enum av7100_dsi_nb_data_lane	nb_data_lane;
	unsigned char				nb_virtual_ch_command_mode;
	unsigned char				nb_virtual_ch_video_mode;
	unsigned short				TE_line_nb;
	enum av7100_te_config		TE_config;
	unsigned long				master_clock_freq;
	unsigned char				ui_x4;
};

struct av7100_globals_t {
	enum av7100_operating_mode av7100_state;
	int hdmi_off_time;
	int hdmi_on_time;
	u8 hpdm;
};

struct av7100_config_t {
	struct av7100_video_input_format_cmd hdmi_video_input_cmd;
	struct av7100_audio_input_format_cmd hdmi_audio_input_cmd;
	struct av7100_video_output_format_cmd hdmi_video_output_cmd;
	struct av7100_color_space_conversion_format_cmd
					hdmi_color_space_conversion_cmd;
	struct av7100_hdmi_cmd hdmi_cmd;
	struct av7100_hdcp_send_key_format_cmd hdmi_hdcp_send_key_cmd;
	struct av7100_hdcp_management_format_cmd
		hdmi_hdcp_management_format_cmd;
	struct av7100_infoframes_format_cmd	hdmi_infoframes_cmd;
	struct av7100_edid_section_readback_format_cmd
		hdmi_edid_section_readback_cmd;
	struct av7100_pattern_generator_format_cmd hdmi_pattern_generator_cmd;
	struct av7100_fuse_aes_key_format_cmd hdmi_fuse_aes_key_cmd;
};

union av7100_configuration {
	struct av7100_video_input_format_cmd video_input_format;
	struct av7100_audio_input_format_cmd audio_input_format;
	struct av7100_video_output_format_cmd video_output_format;
	struct av7100_color_space_conversion_format_cmd 
		color_space_conversion_format;
	struct av7100_hdmi_cmd hdmi_format;
	struct av7100_hdcp_send_key_format_cmd hdcp_send_key_format;
	struct av7100_hdcp_management_format_cmd hdcp_management_format;
	struct av7100_infoframes_format_cmd infoframes_format;
	struct av7100_edid_section_readback_format_cmd edid_section_readback_format;
	struct av7100_pattern_generator_format_cmd pattern_generator_format;
	struct av7100_fuse_aes_key_format_cmd fuse_aes_key_format;
};

extern int av7100_conf_get(enum av7100_command_type command_type, 
	union av7100_configuration *config);
extern int av7100_hdmi_config(u8 hdmi_mode);
extern int av7100_conf_w(enum av7100_command_type command_type,
			u8 *return_buffer_length, u8 *return_buffer);
extern int av7100_conf_prep(enum av7100_command_type command_type, 
			union av7100_configuration *config);
extern int av7100_poweron(void);
extern int av7100_poweroff(void);
extern void av7100_set_state(enum av7100_operating_mode state);
extern int av7100_reg_stby_w(u8 stby, u8 mclkrng);
extern int av7100_reg_stby_pend_int_r(u8 *hpdi, u8 *oni, u8 *ccrst, u8 *cci);
extern int av7100_reg_stby_pend_int_w(u8 hpdi, u8 oni, u8 ccrst, u8 cci);
extern int av7100_reg_gen_int_r(u8 *eoci, u8 *vsii, u8 *vsoi,
						 u8 *ceci, u8 *hdcpi, u8 *uovbi);
extern int av7100_reg_gen_status_r(u8 *cectxe, u8 *cecrx, u8 *cectx, u8 *uc,
							u8 *onuvb, u8 *hdcps);
extern int av7100_reg_gen_int_w(u8 eoci, u8 vsii, u8 vsoi, 
						 u8 ceci, u8 hdcpi, u8 uovbi);
extern int av7100_download_firmware(void);

extern struct hdmi_data *hdmi_dev;
#endif /* __AV7100__H__ */
