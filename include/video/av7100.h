/*
 * Copyright (C) ST-Ericsson AB 2010
 * Copyright (C) 2012 Renesas Mobile Corporation.
 *
 * AV7100 driver
 *
 * Author: Per Persson <per.xb.persson@stericsson.com>
 * for ST-Ericsson.
 *
 * License terms: GNU General Public License (GPL), version 2.
 */
#ifndef __AV7100__H__
#define __AV7100__H__

/* Temp: TODO: remove (or move to menuconfig) */
/*#define CONFIG_AV7100_SDTV*/

#define AV7100_CEC_MESSAGE_SIZE		16
#define AV7100_HDCP_SEND_KEY_SIZE	16
#define AV7100_INFOFRAME_SIZE		28
#define AV7100_FUSE_KEY_SIZE		16
#define AV7100_CHIPVER_1		1
#define AV7100_CHIPVER_2		2

#define HDMI_HDCP_NO_RECEIVER					0
#define HDMI_HDCP_RECEIVER_CONNECTED			1
#define HDMI_HDCP_NO_HDCP_RECEIVER				2
#define HDMI_HDCP_NO_ENCRYPTION					3
#define HDMI_HDCP_AUTHENTICATION_ON_GOING		4
#define HDMI_HDCP_AUTHENTICATION_FAIL			5
#define HDMI_HDCP_AUTHENTICATION_SUCCEED			6
#define HDMI_HDCP_ENCRYPTION_ON_GOING			7

struct av7100_platform_data {
    unsigned	gpio_base;
    int irq;
    struct platform_device *lcdc; 
    struct platform_device *mipi_hdmi_device;
    struct sh_mobile_lcdc_chan_cfg *lcdc_hdmi_chan;
};

enum av7100_error {
	AV7100_OK = 0x0,
	AV7100_INVALID_COMMAND = 0x1,
	AV7100_INVALID_INTERFACE = 0x2,
	AV7100_INVALID_IOCTL = 0x3,
	AV7100_COMMAND_FAIL = 0x4,
	AV7100_FWDOWNLOAD_FAIL = 0x5,
	AV7100_FAIL = 0xFF,
};

enum av7100_command_type {
	AV7100_COMMAND_VIDEO_INPUT_FORMAT  = 0x1,
	AV7100_COMMAND_AUDIO_INPUT_FORMAT,
	AV7100_COMMAND_VIDEO_OUTPUT_FORMAT,
	AV7100_COMMAND_VIDEO_SCALING_FORMAT,
	AV7100_COMMAND_COLORSPACECONVERSION,
	AV7100_COMMAND_CEC_MESSAGE_WRITE,
	AV7100_COMMAND_CEC_MESSAGE_READ_BACK,
	AV7100_COMMAND_DENC,
	AV7100_COMMAND_HDMI,
	AV7100_COMMAND_HDCP_SENDKEY,
	AV7100_COMMAND_HDCP_MANAGEMENT,
	AV7100_COMMAND_INFOFRAMES,
	AV7100_COMMAND_EDID_SECTION_READBACK,
	AV7100_COMMAND_PATTERNGENERATOR,
	AV7100_COMMAND_FUSE_AES_KEY,
};

enum interface_type {
	I2C_INTERFACE = 0x0,
	DSI_INTERFACE = 0x1,
};

enum av7100_dsi_mode {
	AV7100_HDMI_DSI_OFF,
	AV7100_HDMI_DSI_COMMAND_MODE,
	AV7100_HDMI_DSI_VIDEO_MODE
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
	AV7100_TE_OFF,		/* NO TE*/
	AV7100_TE_DSI_LANE,	/* TE generated on DSI lane */
	AV7100_TE_IT_LINE,	/* TE generated on IT line (GPIO) */
	AV7100_TE_DSI_IT,	/* TE generatedon both DSI lane & IT line*/
	AV7100_TE_GPIO_IT	/* TE on GPIO I2S DAT3 & or IT line*/
};

enum av7100_audio_if_format {
	AV7100_AUDIO_I2S_MODE,
	AV7100_AUDIO_I2SDELAYED_MODE, /* I2S Mode by default*/
	AV7100_AUDIO_TDM_MODE         /* 8 Channels by default*/
};

enum av7100_sample_freq {
	AV7100_AUDIO_FREQ_32KHZ,
	AV7100_AUDIO_FREQ_44_1KHZ,
	AV7100_AUDIO_FREQ_48KHZ,
	AV7100_AUDIO_FREQ_64KHZ,
	AV7100_AUDIO_FREQ_88_2KHZ,
	AV7100_AUDIO_FREQ_96KHZ,
	AV7100_AUDIO_FREQ_128KHZ,
	AV7100_AUDIO_FREQ_176_1KHZ,
	AV7100_AUDIO_FREQ_192KHZ
};

enum av7100_audio_word_length {
	AV7100_AUDIO_16BITS,
	AV7100_AUDIO_20BITS,
	AV7100_AUDIO_24BITS
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

enum av7100_output_CEA_VESA {
	AV7100_CUSTOM,
	AV7100_CEA1_640X480P_59_94HZ,
	AV7100_CEA2_3_720X480P_59_94HZ,
	AV7100_CEA4_1280X720P_60HZ,
	AV7100_CEA5_1920X1080I_60HZ,
	AV7100_CEA6_7_NTSC_60HZ,
	AV7100_CEA14_15_480p_60HZ,
	AV7100_CEA16_1920X1080P_60HZ,
	AV7100_CEA17_18_720X576P_50HZ,
	AV7100_CEA19_1280X720P_50HZ,
	AV7100_CEA20_1920X1080I_50HZ,
	AV7100_CEA21_22_576I_PAL_50HZ,
	AV7100_CEA29_30_576P_50HZ,
	AV7100_CEA31_1920x1080P_50Hz,
	AV7100_CEA32_1920X1080P_24HZ,
	AV7100_CEA33_1920X1080P_25HZ,
	AV7100_CEA34_1920X1080P_30HZ,
	AV7100_CEA60_1280X720P_24HZ,
	AV7100_CEA61_1280X720P_25HZ,
	AV7100_CEA62_1280X720P_30HZ,
	AV7100_VESA9_800X600P_60_32HZ,
	AV7100_VESA14_848X480P_60HZ,
	AV7100_VESA16_1024X768P_60HZ,
	AV7100_VESA22_1280X768P_59_99HZ,
	AV7100_VESA23_1280X768P_59_87HZ,
	AV7100_VESA27_1280X800P_59_91HZ,
	AV7100_VESA28_1280X800P_59_81HZ,
	AV7100_VESA39_1360X768P_60_02HZ,
	AV7100_VESA81_1366X768P_59_79HZ,
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

enum av7100_DVI_format {
	AV7100_DVI_CTRL_CTL0,
	AV7100_DVI_CTRL_CTL1,
	AV7100_DVI_CTRL_CTL2
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
	AV7100_PATTERN_AUDIO_ON,
	AV7100_PATTERN_AUDIO_I2S_MEM
};




struct av7100_video_input_format_cmd {
	enum av7100_dsi_mode		dsi_input_mode;
	enum av7100_pixel_format	input_pixel_format;
	unsigned short			total_horizontal_pixel;
	unsigned short			total_horizontal_active_pixel;
	unsigned short			total_vertical_lines;
	unsigned short			total_vertical_active_lines;
	enum av7100_video_mode		video_mode;
	enum av7100_dsi_nb_data_lane	nb_data_lane;
	unsigned char			nb_virtual_ch_command_mode;
	unsigned char			nb_virtual_ch_video_mode;
	unsigned short			TE_line_nb;
	enum av7100_te_config		TE_config;
	unsigned long			master_clock_freq;
	unsigned char			ui_x4;
};

struct av7100_audio_input_format_cmd {
	enum av7100_audio_if_format	audio_input_if_format;
	unsigned char			i2s_input_nb;
	enum av7100_sample_freq		sample_audio_freq;
	enum av7100_audio_word_length	audio_word_lg;
	enum av7100_audio_format	audio_format;
	enum av7100_audio_if_mode	audio_if_mode;
	enum av7100_audio_mute		audio_mute;
};

struct av7100_video_output_format_cmd {
	enum av7100_output_CEA_VESA	video_output_cea_vesa;
	enum av7100_video_sync_pol	vsync_polarity;
	enum av7100_video_sync_pol	hsync_polarity;
	unsigned short		total_horizontal_pixel;
	unsigned short		total_horizontal_active_pixel;
	unsigned short		total_vertical_in_half_lines;
	unsigned short		total_vertical_active_in_half_lines;
	unsigned short		hsync_start_in_pixel;
	unsigned short		hsync_length_in_pixel;
	unsigned short		vsync_start_in_half_line;
	unsigned short		vsync_length_in_half_line;
	unsigned short		hor_video_start_pixel;
	unsigned short		vert_video_start_pixel;
	enum av7100_video_mode	video_type;
	unsigned short		pixel_repeat;
	unsigned long		pixel_clock_freq_Hz;
};

struct av7100_video_scaling_format_cmd {
	unsigned short	h_start_in_pixel;
	unsigned short	h_stop_in_pixel;
	unsigned short	v_start_in_line;
	unsigned short	v_stop_in_line;
	unsigned short	h_start_out_pixel;
	unsigned short	h_stop_out_pixel;
	unsigned short	v_start_out_line;
	unsigned short	v_stop_out_line;
};

struct av7100_color_space_conversion_format_cmd {
	unsigned short	c0;
	unsigned short	c1;
	unsigned short	c2;
	unsigned short	c3;
	unsigned short	c4;
	unsigned short	c5;
	unsigned short	c6;
	unsigned short	c7;
	unsigned short	c8;
	unsigned short	aoffset;
	unsigned short	boffset;
	unsigned short	coffset;
	unsigned char	lmax;
	unsigned char	lmin;
	unsigned char	cmax;
	unsigned char	cmin;
};

struct av7100_cea {
        char cea_id[40];
        int cea_nb;
        int vtotale;
        int vactive;
        int vsbp;
        int vslen;
        int vsfp;
        char vpol[5];
        int htotale;
        int hactive;
        int hbp;
        int hslen;
        int hfp;
        int frequence;
        char hpol[5];
        int reg_line_duration;
        int blkoel_duration;
        int uix4;
        int pll_mult;
        int pll_div;
};

const extern struct av7100_color_space_conversion_format_cmd col_cvt_identity;
const extern struct av7100_color_space_conversion_format_cmd
						col_cvt_identity_clamp_yuv;
const extern struct av7100_color_space_conversion_format_cmd
						col_cvt_yuv422_to_rgb;
const extern struct av7100_color_space_conversion_format_cmd
						col_cvt_yuv422_to_denc;
const extern struct av7100_color_space_conversion_format_cmd
						col_cvt_rgb_to_denc;

struct av7100_cec_message_write_format_cmd {
	unsigned char buffer_length;
	unsigned char buffer[AV7100_CEC_MESSAGE_SIZE];
};

struct av7100_cec_message_read_back_format_cmd {
};

enum av7100_cvbs_video_format {
	AV7100_CVBS_625,
	AV7100_CVBS_525,
};

enum av7100_standard_selection {
	AV7100_PAL_BDGHI,
	AV7100_PAL_N,
	AV7100_NTSC_M,
	AV7100_PAL_M
};

struct av7100_denc_format_cmd {
	enum av7100_cvbs_video_format cvbs_video_format;
	enum av7100_standard_selection standard_selection;
	unsigned char on_off;
	unsigned char macrovision_on_off;
	unsigned char internal_generator;
};

struct av7100_hdmi_cmd {
	enum av7100_hdmi_mode	hdmi_mode;
	enum av7100_hdmi_format	hdmi_format;
	enum av7100_DVI_format	dvi_format; /* used only if HDMI_format = DVI*/
};

struct av7100_hdcp_send_key_format_cmd {
	unsigned char key_number;
	unsigned char data_len;
	unsigned char data[AV7100_HDCP_SEND_KEY_SIZE];
};

enum av7100_hdcp_auth_req_type {
	AV7100_HDCP_AUTH_REQ_OFF = 0,
	AV7100_HDCP_AUTH_REQ_ON = 1,
	AV7100_HDCP_REV_LIST_REQ = 2,
	AV7100_HDCP_AUTH_CONT = 3,
};

enum av7100_hdcp_encr_req_type {
	AV7100_HDCP_ENCR_REQ_OFF = 0,
	AV7100_HDCP_ENCR_REQ_ON = 1,
};

enum av7100_hdcp_encr_use {
	AV7100_HDCP_ENCR_USE_OESS = 0,
	AV7100_HDCP_ENCR_USE_EESS = 1,
};

struct av7100_hdcp_management_format_cmd {
	unsigned char req_type;
	unsigned char req_encr;
	unsigned char encr_use;
};

struct av7100_infoframes_format_cmd {
	unsigned char type;
	unsigned char version;
	unsigned char length;
	unsigned char crc;
	unsigned char data[AV7100_INFOFRAME_SIZE];
};

struct av7100_edid_section_readback_format_cmd {
	unsigned char address;
	unsigned char block_number;
};

struct av7100_pattern_generator_format_cmd {
	enum av7100_pattern_type	pattern_type;
	enum av7100_pattern_format	pattern_video_format;
	enum av7100_pattern_audio	pattern_audio_mode;
};

enum av7100_fuse_operation {
	AV7100_FUSE_READ = 0,
	AV7100_FUSE_WRITE = 1,
};

struct av7100_fuse_aes_key_format_cmd {
	unsigned char fuse_operation;
	unsigned char key[AV7100_FUSE_KEY_SIZE];
};

union av7100_configuration {
	struct av7100_video_input_format_cmd	video_input_format;
	struct av7100_audio_input_format_cmd	audio_input_format;
	struct av7100_video_output_format_cmd	video_output_format;
	struct av7100_video_scaling_format_cmd	video_scaling_format;
	struct av7100_color_space_conversion_format_cmd
		color_space_conversion_format;
	struct av7100_cec_message_write_format_cmd
		cec_message_write_format;
	struct av7100_cec_message_read_back_format_cmd
		cec_message_read_back_format;
	struct av7100_denc_format_cmd		denc_format;
	struct av7100_hdmi_cmd			hdmi_format;
	struct av7100_hdcp_send_key_format_cmd	hdcp_send_key_format;
	struct av7100_hdcp_management_format_cmd hdcp_management_format;
	struct av7100_infoframes_format_cmd	infoframes_format;
	struct av7100_edid_section_readback_format_cmd
		edid_section_readback_format;
	struct av7100_pattern_generator_format_cmd pattern_generator_format;
	struct av7100_fuse_aes_key_format_cmd	fuse_aes_key_format;
};

enum av7100_operating_mode {
	AV7100_OPMODE_UNDEFINED = 0,
	AV7100_OPMODE_SHUTDOWN,
	AV7100_OPMODE_STANDBY,
	AV7100_OPMODE_SCAN,
	AV7100_OPMODE_INIT,
	AV7100_OPMODE_IDLE,
	AV7100_OPMODE_VIDEO,
};

enum av7100_plugin_status {
	AV7100_PLUGIN_NONE = 0x0,
	AV7100_HDMI_PLUGIN = 0x1,
	AV7100_CVBS_PLUGIN = 0x2,
};

enum av7100_hdmi_event {
	AV7100_HDMI_EVENT_NONE =		0x0,
	AV7100_HDMI_EVENT_HDMI_PLUGIN =		0x1,
	AV7100_HDMI_EVENT_HDMI_PLUGOUT =	0x2,
	AV7100_HDMI_EVENT_CEC =			0x4,
	AV7100_HDMI_EVENT_HDCP =		0x8,
};

struct av7100_status {
	enum av7100_operating_mode	av7100_state;
	enum av7100_plugin_status	av7100_plugin_status;
	int				hdmi_on;
};


int av7100_init(void);
void av7100_exit(void);
int av7100_powerup(void);
int av7100_powerdown(void);
int av7100_disable_interrupt(void);
int av7100_enable_interrupt(void);
int av7100_download_firmware(char *fw_buff, int numOfBytes,
	enum interface_type if_type);
int av7100_reg_stby_w(
		unsigned char stby,
		unsigned char mclkrng);
int av7100_reg_hdmi_5_volt_time_w(
		unsigned char hdmi_off_time,
		unsigned char on_time);
int av7100_reg_stby_int_mask_w(
		unsigned char hpdm,
		unsigned char stbygpiocfg,
		unsigned char ipol);
int av7100_reg_stby_pend_int_w(
		unsigned char hpdi,
		unsigned char oni);
int av7100_reg_gen_int_mask_w(
		unsigned char eocm,
		unsigned char vsim,
		unsigned char vsom,
		unsigned char cecm,
		unsigned char hdcpm,
		unsigned char uovbm);
int av7100_reg_gen_int_w(
		unsigned char eoci,
		unsigned char vsii,
		unsigned char vsoi,
		unsigned char ceci,
		unsigned char hdcpi,
		unsigned char uovbi);
int av7100_reg_gpio_conf_w(
		unsigned char dat3dir,
		unsigned char dat3val,
		unsigned char dat2dir,
		unsigned char dat2val,
		unsigned char dat1dir,
		unsigned char dat1val,
		unsigned char ucdbg);
int av7100_reg_gen_ctrl_w(
		unsigned char fdl,
		unsigned char hld,
		unsigned char wa,
		unsigned char ra);
int av7100_reg_fw_dl_entry_w(
	unsigned char mbyte_code_entry);
int av7100_reg_w(
		unsigned char offset,
		unsigned char value);
int av7100_reg_stby_r(
		unsigned char *stby,
		unsigned char *hpds,
		unsigned char *mclkrng);
int av7100_reg_hdmi_5_volt_time_r(
		unsigned char *hdmi_off_time,
		unsigned char *on_time);
int av7100_reg_stby_int_mask_r(
		unsigned char *hpdm,
		unsigned char *stbygpiocfg,
		unsigned char *ipol);
int av7100_reg_stby_pend_int_r(
		unsigned char *hpdi,
		unsigned char *oni,
		unsigned char *sid);
int av7100_reg_gen_int_mask_r(
		unsigned char *eocm,
		unsigned char *vsim,
		unsigned char *vsom,
		unsigned char *cecm,
		unsigned char *hdcpm,
		unsigned char *uovbm);
int av7100_reg_gen_int_r(
		unsigned char *eoci,
		unsigned char *vsii,
		unsigned char *vsoi,
		unsigned char *ceci,
		unsigned char *hdcpi,
		unsigned char *uovbi);
int av7100_reg_gen_status_r(
		unsigned char *cecrec,
		unsigned char *cectrx,
		unsigned char *uc,
		unsigned char *onuvb,
		unsigned char *hdcps);
int av7100_reg_gpio_conf_r(
		unsigned char *dat3dir,
		unsigned char *dat3val,
		unsigned char *dat2dir,
		unsigned char *dat2val,
		unsigned char *dat1dir,
		unsigned char *dat1val,
		unsigned char *ucdbg);
int av7100_reg_gen_ctrl_r(
		unsigned char *fdl,
		unsigned char *hld,
		unsigned char *wa,
		unsigned char *ra);
int av7100_reg_fw_dl_entry_r(
	unsigned char *mbyte_code_entry);
int av7100_reg_r(
		unsigned char offset,
		unsigned char *value);
int av7100_conf_get(enum av7100_command_type command_type,
	union av7100_configuration *config);
int av7100_conf_prep(enum av7100_command_type command_type,
	union av7100_configuration *config);
int av7100_conf_w(enum av7100_command_type command_type,
		unsigned char *return_buffer_length,
		unsigned char *return_buffer, enum interface_type if_type);
int av7100_conf_w_raw(enum av7100_command_type command_type,
	unsigned char buffer_length,
	unsigned char *buffer,
	unsigned char *return_buffer_length,
	unsigned char *return_buffer);
struct av7100_status av7100_status_get(void);
enum av7100_output_CEA_VESA av7100_video_output_format_get(int xres,
	int yres,
	int htot,
	int vtot,
	int pixelclk,
	bool interlaced);
void av7100_hdmi_event_cb_set(void (*event_callback)(enum av7100_hdmi_event));
u8 av7100_ver_get(void);
extern u16 av7100_get_te_line_nb(enum av7100_output_CEA_VESA);
extern u16 av7100_get_ui_x4(enum av7100_output_CEA_VESA);
//unsigned long  av7100_get_clk_rate(void);

#endif /* __AV7100__H__ */
