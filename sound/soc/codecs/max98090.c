/* max98090.c
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
  @file max98090.c

  @brief Audio LSI driver source file.
*/

#define __SOC_CODEC_ADD__


/*---------------------------------------------------------------------------*/
/* include files                                                             */
/*---------------------------------------------------------------------------*/
#ifdef __MAX98090_UT__
#include "max98090_ut_stb.h"
#else   /* __MAX98090_UT__ */
#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/i2c/i2c-sh_mobile.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <linux/proc_fs.h>
#include <linux/switch.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/input.h>
#ifdef __SOC_CODEC_ADD__
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#else   /* __SOC_CODEC_ADD__ */
/* none */
#endif  /* __SOC_CODEC_ADD__ */

#include <mach/r8a73734.h>
#endif  /* __MAX98090_UT__ */

#include <sound/soundpath/max98090_extern.h>
#include <sound/soundpath/soundpath.h>

#include "max98090_defs.h"
#include "max97236_defs.h"
#include "max98090_log.h"

/*---------------------------------------------------------------------------*/
/* typedef declaration (private)                                             */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* define macro declaration (private)                                        */
/*---------------------------------------------------------------------------*/
#define MAX98090_SLAVE_ADDR		0x10 /* MAX98090 i2c slave address */
#define MAX97236_SLAVE_ADDR		0x40 /* MAX97236 i2c slave address */

/* INPUT ENABLE(0x3E) */
#define MAX98090_IE_MIC_ENABLE		0x10 /* MBEN */
#define MAX98090_IE_ADC_ENABLE		0x03 /* ADREN/ADLEN */
/* OUTPUT ENABLE(0x3F) */
#define MAX98090_OE_HEADPHONE_ENABLE	0xC0 /* HPREN/HPLEN */
#define MAX98090_OE_SPEAKER_ENABLE	0x30 /* SPREN/SPLEN */
#define MAX98090_OE_EARPIECE_ENABLE	0x0C /* RCVLEN/RCVREN */
#define MAX98090_OE_DAC_ENABLE		0x03 /* DAREN/DALEN */
/* I/O Config(0x25) */
#define MAX98090_IO_HIZOFF_ON		0x04 /* HIZOFF */
#define MAX98090_IO_SDOEN_ON		0x02 /* SDOUT (input device) */
#define MAX98090_IO_SDIEN_ON		0x01 /* SDIEN (output device) */

#define MAX98090_ENABLE			1    /* enable value */
#define MAX98090_DISABLE		0    /* disable value */

#define MAX98090_EVM			0    /* max97236 deployment */
#define MAX98090_E2K			1    /* max97236 non-deployment */

#define MAX98090_VOLUMEL_ELEMENT	6    /* volume element */

#define MAX98090_GPIO_BASE	IO_ADDRESS(0xE6050000)
#define MAX98090_GPIO_034	(MAX98090_GPIO_BASE + 0x0022)	/* AUDIO_IRQ */

#define MAX98090_CONFIG_INIT            "/etc/audio_lsi_driver/"
#define MAX98090_EXTENSION              ".conf"
#define MAX98090_LEN_MAX_ONELINE        (80)
#define MAX98090_WAIT_MAX               (1000)
#define MAX98090_DUMP_REG_MAX           (0xffffffff)
#define MAX98090_DISABLE_CONFIG         (0xff000004)
#define MAX98090_DISABLE_CONFIG_SUB     (0x00000000)
#define MAX98090_MAX_PATH_LENGTH        (128)
#define MAX98090_VOLUMEL_SET(table)     (table * MAX98090_VOLUMEL_ELEMENT)
DEFINE_MUTEX(max98090_mutex);
/*---------------------------------------------------------------------------*/
/* define function macro declaration (private)                               */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* enum declaration (private)                                                */
/*---------------------------------------------------------------------------*/
/*!
  @brief volume set device .
*/
enum MAX98090_VOL_DEV_VAL {
	MAX98090_VOL_DEV_SW = 0,     /**< software. */
	MAX98090_VOL_DEV_SPEAKER,    /**< playback speaker. */
	MAX98090_VOL_DEV_EARPIECE,   /**< playback earpiece. */
	MAX98090_VOL_DEV_HEADPHONES, /**< playback headphones or headset. */
	MAX98090_VOL_DEV_MAX
};

/*!
  @brief configration parameter .
*/
enum {
	E_SLV_ADDR = 0,              /**< slave address. */
	E_REG_ADDR,                  /**< register address. */
	E_REG_VAL,                   /**< register value. */
	E_B_WAIT,                    /**< before wait time. */
	E_A_WAIT,                    /**< after wait time. */
	E_MAX
};

/*!
  @brief configration value flag.
*/
enum {
	MAX98090_VAL_FALSE = 0,      /**< non-value. */
	MAX98090_VAL_TRUE = 1        /**< value. */
};

/*---------------------------------------------------------------------------*/
/* structure declaration (private)                                           */
/*---------------------------------------------------------------------------*/
/*!
  @brief AudioLSI driver internal device info.
*/
struct max98090_info {
	u_long raw_device;  /**< device ID. */
	u_int speaker;      /**< playback speaker. */
	u_int earpiece;     /**< playback earpiece. */
	u_int headphone;    /**< playback headphones or headset.*/
	u_int mic;          /**< capture mic. */
	u_int headset_mic;  /**< capture headset mic. */
	u_int volume[MAX98090_VOL_DEV_MAX];
			    /**< volume level setting. */
	u_int mute;         /**< mute setting. */
	u_int speaker_amp;  /**< speaker amplifiers setting. */
};

/*!
  @brief jack notify.
*/
struct max98090_switch_data {
	struct switch_dev sdev; /**< switch dev. */
	int state;              /**< switch state. */
	int key_press;          /**< hook button pressed/released. */
};

/*!
  @brief AudioLSI driver configuration structure.
*/
struct max98090_priv {
	/* info */
	int board;                          /**< board info. */
	struct max98090_info info;          /**< user setting info. */
	struct input_dev *input_dev;        /**< input device. */
	/* i2c */
	struct i2c_client *client_max97236; /**< i2c driver max97236 client. */
	struct i2c_client *client_max98090; /**< i2c driver max98090 client. */
	/* irq */
	u_int irq;                          /**< irq number. */
	struct workqueue_struct *irq_workqueue;
					    /**< irq workqueue. */
	struct work_struct irq_work;	    /**< irq work. */
	struct max98090_switch_data switch_data;
					    /**< switch data. */
	/* callback */
	struct max98090_callback_func *callback;
					    /**< structure to register
						 a callback function. */
	/* log */
	struct proc_dir_entry *log_entry;   /**< log level entry. */
	struct proc_dir_entry *dump_entry;  /**< dump entry. */
	struct proc_dir_entry *config_entry;/**< config entry. */
	struct proc_dir_entry *path_entry;  /**< config path entry. */
	struct proc_dir_entry *proc_parent; /**< proc parent. */
#ifdef __MAX98090_RELEASE_CHECK__
	struct proc_dir_entry *release_entry;/**< release check entry. */
#endif	/* __MAX98090_RELEASE_CHECK__ */
#ifdef __SOC_CODEC_ADD__
	struct snd_soc_codec *codec;
#else
	/* none */
#endif  /* __SOC_CODEC_ADD__ */
};


/*!
  @brief pcm structure.
*/
struct max98090_audio_device {
	u_int number;
	char *name;
};

/*!
  @brief configuration structure.
*/
struct max98090_config_value {
	u_int slv_addr;
	u_int reg_addr;
	u_int reg_value;
	u_int before_wait;
	u_int after_wait;
};

/*---------------------------------------------------------------------------*/
/* prototype declaration (private)                                           */
/*---------------------------------------------------------------------------*/
static int max98090_enable_vclk4(void);
static int max98090_disable_vclk4(void);

static int max98090_setup(struct i2c_client *client,
			struct max98090_priv *dev);
static int max98090_setup_r8a73734(void);
static int max98090_setup_max98090(void);
static int max98090_setup_max97236(void);
static int max98090_switch_dev_register(struct max98090_priv *dev);
static int max98090_switch_hook_dev_register(struct i2c_client *client,
			struct max98090_priv *dev);
static int max98090_create_proc_entry(char *name,
					struct proc_dir_entry **proc_child);

static int max98090_enable_interrupt(void);
static int max98090_disable_interrupt(void);
static void max98090_irq_work_func(struct work_struct *unused);
static irqreturn_t max98090_irq_handler(int irq, void *data);

static int max98090_conv_device_info(const u_long device,
				struct max98090_info *device_info);

static int max98090_check_device(const u_long device);
static int max98090_check_volume_level(const u_int volume);
static int max98090_check_volume_device(const u_long device);
static int max98090_check_mute(const u_int mute);
static int max98090_check_speaker_amp(const u_int value);
static int max98090_check_dump_type(const u_int dump_type);

static int max98090_set_device_active(const struct max98090_info new_device);
static int max98090_set_speaker_device(const u_int cur_dev,
				const u_int new_dev);
static int max98090_set_earpiece_device(const u_int cur_dev,
				const u_int new_dev);
static int max98090_set_headphone_device(const u_int cur_dev,
				const u_int new_dev);
static int max98090_set_mic_device(const u_int cur_dev, const u_int new_dev);
static int max98090_set_headset_mic_device(const u_int cur_dev,
				const u_int new_dev);

static int max98090_dump_max98090_registers(void);
static int max98090_dump_max97236_registers(void);

static int max98090_set_data(u_int config_val[]);
static int max98090_set_config(char *buf);
static int max98090_open_file(char *config_path, struct file **config_filp);
static int max98090_read_line(struct file *config_filp, char *buf);
static int max98090_close_file(struct file *config_filp);
static int max98090_read_config(char *pcm_name, char *pcm_path);
static int max98090_write_config(const u_int audio_ic, const u_int addr,
		const u_int value, const u_int before_wait,
		const u_int after_wait);

#ifdef __MAX98090_DEBUG__
static void max98090_dump_max98090_priv(void);
#endif /* __MAX98090_DEBUG__ */

static int max98090_proc_log_read(char *page, char **start, off_t offset,
			int count, int *eof, void *data);
static int max98090_proc_log_write(struct file *filp, const char *buffer,
			unsigned long count, void *data);
static int max98090_proc_dump_read(char *page, char **start, off_t offset,
			int count, int *eof, void *data);
static int max98090_proc_dump_write(struct file *filp, const char *buffer,
			unsigned long count, void *data);
static int max98090_proc_config_write(struct file *filp, const char *buffer,
			unsigned long count, void *data);
static int max98090_proc_path_read(char *page, char **start, off_t offset,
			int count, int *eof, void *data);
static int max98090_proc_path_write(struct file *filp, const char *buffer,
			unsigned long count, void *data);
#ifdef __MAX98090_RELEASE_CHECK__
static int max98090_proc_release_write(struct file *filp, const char *buffer,
			unsigned long count, void *data);
#endif	/* __MAX98090_RELEASE_CHECK__ */


#ifdef __SOC_CODEC_ADD__
static int max98090_probe(struct snd_soc_codec *codec);
static int max98090_remove(struct snd_soc_codec *codec);
static int max98090_i2c_probe(struct i2c_client *i2c,
			const struct i2c_device_id *id);
static int max98090_i2c_remove(struct i2c_client *i2c);
#else
static int max98090_probe(struct i2c_client *i2c,
			const struct i2c_device_id *id);
static int max98090_remove(struct i2c_client *i2c);
#endif  /* __SOC_CODEC_ADD__ */



/*---------------------------------------------------------------------------*/
/* global variable declaration                                               */
/*---------------------------------------------------------------------------*/
/*!
  @brief AudioLSI driver log level.
*/
#ifdef __MAX98090_DEBUG__
static u_int max98090_log_level = (MAX98090_LOG_ERR_PRINT |
				   MAX98090_LOG_PROC_PRINT |
				   MAX98090_LOG_FUNC_PRINT
				   /* | MAX98090_LOG_DEBUG_PRINT */
				  );
static u_int max98090_dump_reg = MAX98090_AUDIO_IC_ALL;
#else   /* __MAX98090_DEBUG__ */
static u_int max98090_log_level = MAX98090_LOG_NO_PRINT;
static u_int max98090_dump_reg = MAX98090_AUDIO_IC_NONE;
#endif  /* __MAX98090_DEBUG__ */

/*!
  @brief Store the AudioLSI driver config.
*/
static struct max98090_priv *max98090_conf;
static char max98090_configration_path[MAX98090_MAX_PATH_LENGTH];
/*!
  @brief max98090 E2K volume table.
*/
static const u_int
MAX98090_E2K_volume[MAX98090_VOL_DEV_MAX][MAX98090_VOLUMEL_ELEMENT] = {
	{/* MAX98090_VOL_DEV_SW */
		MAX98090_VOLUMEL0,
		MAX98090_VOLUMEL1,
		MAX98090_VOLUMEL2,
		MAX98090_VOLUMEL3,
		MAX98090_VOLUMEL4,
		MAX98090_VOLUMEL5
	},
	{/* MAX98090_VOL_DEV_SPEAKER */
		0x19, /* -44dB */
		0x1C, /* -32dB */
		0x1E, /* -26dB */
		0x1F, /* -23dB */
		0x20, /* -20dB */
		0x21  /* -17db */
	},
	{/* MAX98090_VOL_DEV_EARPIECE */
		0x0C, /* -20dB */
		0x0F, /* -12dB */
		0x12, /*  -6dB */
		0x15, /*   0dB */
		0x19, /*   4dB */
		0x1F  /*   8dB */
	},
	{/* MAX98090_VOL_DEV_HEADPHONES */
		0x01, /* -63dB */
		0x03, /* -55dB */
		0x05, /* -47db */
		0x07, /* -40dB */
		0x09, /* -34dB */
		0x0D  /* -22dB */
	}
};

/*!
  @brief max98090 EVM volume table.
*/
static const u_int
MAX98090_EVM_volume[MAX98090_VOL_DEV_MAX][MAX98090_VOLUMEL_ELEMENT] = {
	{/* MAX98090_VOL_DEV_SW */
		MAX98090_VOLUMEL0,
		MAX98090_VOLUMEL1,
		MAX98090_VOLUMEL2,
		MAX98090_VOLUMEL3,
		MAX98090_VOLUMEL4,
		MAX98090_VOLUMEL5
	},
	{/* MAX98090_VOL_DEV_SPEAKER */
		0x1F, /* -23dB */
		0x23, /* -12dB */
		0x26, /*  -6dB */
		0x29, /*  -3dB */
		0x2C, /*   0dB */
		0x30  /*   4db */
	},
	{/* MAX98090_VOL_DEV_EARPIECE */
		0x07, /* -35dB */
		0x09, /* -29dB */
		0x0A, /* -26dB */
		0x0D, /* -17dB */
		0x0F, /* -12dB */
		0x13  /*  -4dB */
	},
	{/* MAX98090_VOL_DEV_HEADPHONES */
		0x02, /* -59dB */
		0x04, /* -51dB */
		0x06, /* -43db */
		0x08, /* -37dB */
		0x0A, /* -31dB */
		0x0F  /* -17dB */
	}
};

/*!
  @brief i2c driver data.
*/
static const struct i2c_device_id max98090_i2c_id[] = {
	{ "max98090", MAX98090_AUDIO_IC_MAX98090 },
	{ "max97236", MAX98090_AUDIO_IC_MAX97236 },
	{ /* end of list */ },
};
MODULE_DEVICE_TABLE(i2c, max98090_i2c_id);

/*!
  @brief represent an I2C device driver.
*/
static struct i2c_driver max98090_i2c_driver = {
	.driver = {
		.name = "max98090",
	},
#ifdef __SOC_CODEC_ADD__
	.probe  = max98090_i2c_probe,
	.remove = max98090_i2c_remove,
#else
	.probe  = max98090_probe,
	.remove = max98090_remove,
#endif  /* __SOC_CODEC_ADD__ */
	.id_table = max98090_i2c_id,
};

#ifdef __SOC_CODEC_ADD__
static struct snd_kcontrol_new *max98090_controls;
static u_int max98090_array_size;
#endif  /* __SOC_CODEC_ADD__ */

/*---------------------------------------------------------------------------*/
/* extern variable declaration                                               */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* extern function declaration                                               */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* inline function implementation                                            */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* function implementation                                                   */
/*---------------------------------------------------------------------------*/
/*------------------------------------*/
/* for private function               */
/*------------------------------------*/
/*!
  @brief enable vclk4 register.

  @param none.

  @return function results.

  @note EXSRC=main clock*1/2, CKSTP=Supplies video clock, VDIV=0

*/
static int max98090_enable_vclk4(void)
{
	int ret = 0;
	struct clk *main_clk = NULL;
	struct clk *vclk4_clk = NULL;
	max98090_log_efunc("");
	ret = gpio_request(GPIO_FN_VIO_CKO4, NULL);

	if (0 != ret)
		goto err_gpio_request;

	vclk4_clk = clk_get(NULL, "vclk4_clk");

	if (IS_ERR(vclk4_clk)) {
		ret = IS_ERR(vclk4_clk);
		goto err_gpio_request;
	}

	main_clk = clk_get(NULL, "main_clk");

	if (IS_ERR(main_clk)) {
		ret = IS_ERR(main_clk);
		goto err_gpio_request;
	}

	/* vclk4_clk */
	ret = clk_set_parent(vclk4_clk, main_clk);

	if (0 != ret)
		goto err_gpio_request;

#ifdef MAX98090_26MHZ
	ret = clk_set_rate(vclk4_clk, 26000000);
#else	/* MAX98090_26MHZ */
	ret = clk_set_rate(vclk4_clk, 13000000);
#endif	/* MAX98090_26MHZ */

	if (0 != ret)
		goto err_gpio_request;

	ret = clk_enable(vclk4_clk);

	if (0 != ret)
		goto err_gpio_request;

	clk_put(vclk4_clk);
	max98090_log_rfunc("ret[%d]", ret);
	return ret;
err_gpio_request:
	max98090_log_err("ret[%d]", ret);
	return ret;
}

/*!
  @brief disable vclk4 register.

  @param none.

  @return function results.
*/
static int max98090_disable_vclk4(void)
{
	int ret = 0;
	max98090_log_efunc("");
	gpio_free(GPIO_FN_VIO_CKO4);
	max98090_log_rfunc("ret[%d]", ret);
	return ret;
}

/*!
  @brief setup max98090 and max97236.

  @param none.

  @return function results.
*/
static int max98090_setup(struct i2c_client *client,
			struct max98090_priv *dev)
{
	int ret = 0;
	max98090_log_efunc("");
	/* create file for log level entry */
	dev->proc_parent = proc_mkdir(AUDIO_LSI_DRV_NAME, NULL);

	if (NULL != dev->proc_parent) {
		ret = max98090_create_proc_entry(MAX98090_LOG_LEVEL,
						&dev->log_entry);
		if (0 != ret)
			goto err_proc;

		ret = max98090_create_proc_entry(MAX98090_DUMP_REG,
						&dev->dump_entry);
		if (0 != ret)
			goto err_proc;

		ret = max98090_create_proc_entry(MAX98090_TUNE_REG,
						&dev->config_entry);
		if (0 != ret)
			goto err_proc;

		ret = max98090_create_proc_entry(MAX98090_PATH_SET,
						&dev->path_entry);
		if (0 != ret)
			goto err_proc;

#ifdef __MAX98090_RELEASE_CHECK__

		ret = max98090_create_proc_entry("release_check",
						&dev->release_entry);
		if (0 != ret)
			goto err_proc;

#endif	/* __MAX98090_RELEASE_CHECK__ */
	}
	/* create workqueue for irq */
	if (NULL == dev->irq_workqueue) {
		dev->irq_workqueue =
			create_singlethread_workqueue("jack_detect");

		if (NULL == dev->irq_workqueue) {
			ret = -ENOMEM;
			max98090_log_err("queue create error. ret[%d]", ret);
			goto err_create_singlethread_workqueue;
		}

		INIT_WORK(&dev->irq_work, max98090_irq_work_func);
	}

	ret = max98090_switch_dev_register(dev);

	if (0 != ret)
		goto err_max98090_switch_dev_register;

	ret = max98090_switch_hook_dev_register(client, dev);
	if (0 != ret)
		goto err_max98090_hook_switch_dev_register;

	/***********************************/
	/* setup r8a73734                  */
	/***********************************/
	ret = max98090_setup_r8a73734();

	if (0 != ret)
		goto err_setup_r8a73734;

	/* request irq */
	dev->irq = client->irq;
	max98090_log_info("client->irq[%d]", client->irq);
	ret = request_irq(client->irq, &max98090_irq_handler,
			  (IRQF_DISABLED | IRQF_TRIGGER_FALLING),
			  client->name, NULL);

	if (0 != ret) {
		max98090_log_err("irq request err. ret[%d]", ret);
		goto err_request_irq;
	}

	/***********************************/
	/* setup max98090                  */
	/***********************************/
	ret = max98090_setup_max98090();

	if (0 != ret)
		goto err_setup_max98090;

	/***********************************/
	/* setup max97236                  */
	/***********************************/
	if (MAX98090_EVM == max98090_conf->board) {
		ret = max98090_setup_max97236();

		if (0 != ret)
			goto err_setup_max97236;
	}

	ret = max98090_enable_interrupt();

	if (0 != ret) {
		max98090_log_err("enable interrupt err. ret[%d]", ret);
		goto err_enable_interrupt;
	}

	max98090_log_rfunc("ret[%d]", ret);
	return ret;
err_enable_interrupt:
err_setup_max97236:
err_setup_max98090:
err_request_irq:
err_setup_r8a73734:
err_max98090_switch_dev_register:
err_max98090_hook_switch_dev_register:
err_create_singlethread_workqueue:

	if (dev->irq_workqueue)
		destroy_workqueue(dev->irq_workqueue);

err_proc:

	if (dev->log_entry)
		remove_proc_entry(MAX98090_LOG_LEVEL, dev->proc_parent);

	if (dev->dump_entry)
		remove_proc_entry(MAX98090_DUMP_REG, dev->proc_parent);

	if (dev->config_entry)
		remove_proc_entry(MAX98090_TUNE_REG, dev->proc_parent);

	if (dev->path_entry)
		remove_proc_entry(MAX98090_PATH_SET, dev->proc_parent);
#ifdef __MAX98090_RELEASE_CHECK__
	if (dev->release_entry)
		remove_proc_entry("release_check", dev->proc_parent);
#endif	/* __MAX98090_RELEASE_CHECK__ */

	if (dev->proc_parent)
		remove_proc_entry(AUDIO_LSI_DRV_NAME, NULL);
	max98090_log_err("ret[%d]", ret);
	return ret;
}

/*!
  @brief create proc.

  @param none.

  @return function results.
*/
static int max98090_create_proc_entry(char *name,
					struct proc_dir_entry **proc_child)
{
	int ret = 0;
	*proc_child = create_proc_entry(name,
					(S_IRUGO | S_IWUGO),
					max98090_conf->proc_parent);
	if (NULL != *proc_child) {
		if (strcmp(name, MAX98090_LOG_LEVEL) == 0) {
			(*proc_child)->read_proc = max98090_proc_log_read;
			(*proc_child)->write_proc = max98090_proc_log_write;
		} else if (strcmp(name, MAX98090_TUNE_REG) == 0) {
			(*proc_child)->write_proc = max98090_proc_config_write;
		} else if (strcmp(name, MAX98090_DUMP_REG) == 0) {
			(*proc_child)->read_proc = max98090_proc_dump_read;
			(*proc_child)->write_proc = max98090_proc_dump_write;
		} else if (strcmp(name, MAX98090_PATH_SET) == 0) {
			(*proc_child)->read_proc = max98090_proc_path_read;
			(*proc_child)->write_proc = max98090_proc_path_write;
		} else if (strcmp(name, "release_check") == 0) {
#ifdef __MAX98090_RELEASE_CHECK__
			(*proc_child)->write_proc =
						max98090_proc_release_write;
#endif	/* __MAX98090_RELEASE_CHECK__ */
		} else {
			max98090_log_err("parameter error.");
			ret = -EINVAL;
		}
	} else {
		max98090_log_err("create failed for %s.", name);
		ret = -ENOMEM;
	}
	return ret;
}

/*!
  @brief setup r8a73734.

  @param none.

  @return function results.
*/
static int max98090_setup_r8a73734(void)
{
	int ret = 0;
	int status1 = 0;
	max98090_log_efunc("");
	ret = max98090_enable_vclk4();

	if (0 != ret)
		goto err_enable_vclk4;

	/* update internal board info */
	ret = max98090_read(MAX98090_AUDIO_IC_MAX97236,
			MAX97236_REG_OR_STATUS1, &status1);

	if (0 > status1)
		max98090_conf->board = MAX98090_E2K;
	else
		max98090_conf->board = MAX98090_EVM;

	max98090_log_info("read max97236(0x00) status1[0x%x] board[%d]",
			status1,
			max98090_conf->board);

	if (MAX98090_EVM == max98090_conf->board) {
		ret = gpio_request(GPIO_PORT29, NULL);

		if (0 != ret)
			goto err_gpio_request;

		gpio_set_value(GPIO_PORT29, 0);
		ret = gpio_direction_output(GPIO_PORT29, 1);

		if (0 != ret)
			goto err_gpio_request;
	} else {
		iowrite8(0xe0, MAX98090_GPIO_034);
	}

	max98090_log_rfunc("ret[%d]", ret);
	return ret;
err_gpio_request:
err_enable_vclk4:
	max98090_log_err("ret[%d]", ret);
	return ret;
}

/*!
  @brief setup max98090.

  @param none.

  @return function results.
*/
static int max98090_setup_max98090(void)
{
	int ret = 0;
	max98090_log_efunc("");
	/* quick setup */
#ifdef MAX98090_26MHZ
	/* Register 0x04 (System Clock Quick Setup) =
	   0x80(Setup Device for Operation with a 26MHz Master Clock (MCLK)) */
	ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
			MAX98090_REG_W_SYSTEM_CLOCK, 0x80);
#else	/* MAX98090_26MHZ */
	/* Register 0x04 (System Clock Quick Setup) =
	   0x20(Setup Device for Operation with a 13MHz Master Clock (MCLK)) */
	ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
			MAX98090_REG_W_SYSTEM_CLOCK, 0x20);
#endif	/* MAX98090_26MHZ */

	if (0 != ret)
		goto err_i2c_write;

	/* Register 0x05 (Sample Rate Quick Setup) =
	   0x08 (Setup Clocks and Filters for a 48kHz Sample Rate) */
	ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
			MAX98090_REG_W_SAMPLE_RATE, 0x08);

	if (0 != ret)
		goto err_i2c_write;

	/* Register 0x06 (Digital Audio Interface (DAI) Quick Setup) =
	   0x01 (Setup DAI for I2S Slave Mode Operation) */
	ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
			MAX98090_REG_W_INTERFACE, 0x01);

	if (0 != ret)
		goto err_i2c_write;

	if (MAX98090_EVM == max98090_conf->board) {
		/* Register 0x03 (INTERUPT MASKS) = 0x00 */
		ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
				MAX98090_REG_RW_INTERRUPT_MASKS, 0x00);

		if (0 != ret)
			goto err_i2c_write;
	} else {
		/* Register 0x03 (INTERUPT MASKS) = 0x04:IJDET */
		ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
				MAX98090_REG_RW_INTERRUPT_MASKS, 0x04);

		if (0 != ret)
			goto err_i2c_write;

		/* Register 0x3D (JACK DETECT) = 0x83:JDETEN JDEB=200ms*/
		ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
				MAX98090_REG_RW_JACK_DETECT, 0x83);

		if (0 != ret)
			goto err_i2c_write;

		/* Register 0x27 (PLAYBACK_LEVEL_1) = 0x30:DV1G=+18dB*/
		ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
				MAX98090_REG_RW_PLAYBACK_LEVEL_1, 0x30);

		if (0 != ret)
			goto err_i2c_write;
	}

	/* set default speaker/earpiece/headphone volume */
	ret = max98090_set_volume(MAX98090_DEV_PLAYBACK_SPEAKER,
				MAX98090_VOLUMEL3);

	if (0 != ret)
		goto err_set_volume;

	ret = max98090_set_volume(MAX98090_DEV_PLAYBACK_EARPIECE,
				MAX98090_VOLUMEL3);

	if (0 != ret)
		goto err_set_volume;

	ret = max98090_set_volume(MAX98090_DEV_PLAYBACK_HEADPHONES,
				MAX98090_VOLUMEL3);

	if (0 != ret)
		goto err_set_volume;

	/* 0x44(ADC CONTROL) 0x03:OSR128=ADCCLK=64*fS ADCDITHER=Dither enabled
	   ADCHP=ADC is optimized for best performance */
	ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
			MAX98090_REG_RW_ADC_CONTROL, 0x03);

	if (0 != ret)
		goto err_i2c_write;

	max98090_log_rfunc("ret[%d]", ret);
	return ret;
err_i2c_write:
err_set_volume:
	max98090_log_err("ret[%d]", ret);
	return ret;
}

/*!
  @brief setup max97236.

  @param none.

  @return function results.
*/
static int max98090_setup_max97236(void)
{
	int ret = 0;
	max98090_log_efunc("");
	/* 0x1E(Enable2) 0x02:AUTO=0x02 */
	ret = max98090_write(MAX98090_AUDIO_IC_MAX97236,
			MAX97236_REG_RW_ENABLE2, 0x02);

	if (0 != ret)
		goto err_i2c_write;

	/* 0x1D(Enable1) 0x80:SHDN */
	ret = max98090_write(MAX98090_AUDIO_IC_MAX97236,
			MAX97236_REG_RW_ENABLE1, 0x80);

	if (0 != ret)
		goto err_i2c_write;

	/* 0x07(Left Volume) 0x39:L/R=independent RMUTEL=Disable LVOL=0db */
	ret = max98090_write(MAX98090_AUDIO_IC_MAX97236,
			MAX97236_REG_RW_LEFT_VOLUME, 0x39);

	if (0 != ret)
		goto err_i2c_write;

	/* 0x08(Right Volume) 0x39:MUTER=Disable RVOL=0db */
	ret = max98090_write(MAX98090_AUDIO_IC_MAX97236,
			MAX97236_REG_RW_RIGHT_VOLUME, 0x39);

	if (0 != ret)
		goto err_i2c_write;

	max98090_log_rfunc("ret[%d]", ret);
	return ret;
err_i2c_write:
	max98090_log_err("ret[%d]", ret);
	return ret;
}

/*!
  @brief register jack switch device.

  @param none.

  @return function results.
*/
static int max98090_switch_dev_register(struct max98090_priv *dev)
{
	int ret = 0;
	max98090_log_efunc("");
	dev->switch_data.sdev.name = "h2w";
	dev->switch_data.sdev.state = 0;
	dev->switch_data.state = 0;
	ret = switch_dev_register(&dev->switch_data.sdev);

	if (0 != ret)
		max98090_log_err("ret[%d]", ret);

	max98090_log_rfunc("ret[%d]", ret);
	return ret;
}

/*!
  @brief register hook switch device.

  @param none.

  @return function results.
*/
static int max98090_switch_hook_dev_register(struct i2c_client *client,
						struct max98090_priv *dev)
{
	int ret = 0;
	max98090_log_efunc("");
	dev->input_dev = input_allocate_device();
	dev->input_dev->name = "max98090";
	dev->input_dev->id.bustype = BUS_I2C;
	dev->input_dev->dev.parent = &client->dev;

	__set_bit(EV_KEY, dev->input_dev->evbit);
	__set_bit(KEY_MEDIA, dev->input_dev->keybit);
	ret = input_register_device(dev->input_dev);
	if (0 != ret) {
		input_free_device(dev->input_dev);
		max98090_log_err("ret[%d]", ret);
	}
	max98090_log_rfunc("ret[%d]", ret);
	return ret;
}

/*!
  @brief enable interrupt registers.

  @param none.

  @return function results.
*/
static int max98090_enable_interrupt(void)
{
	int ret = 0;
	max98090_log_efunc("");

	if (MAX98090_EVM == max98090_conf->board) {
		/* set IRQ Mask1 Enable : DDONE */
		ret = max98090_write(MAX98090_AUDIO_IC_MAX97236,
				MAX97236_REG_RW_IRQ_MASK1, 0x40);

		if (0 != ret)
			goto err_i2c_write;

		/* set IRQ Mask2 Enable : HP_L */
		ret = max98090_write(MAX98090_AUDIO_IC_MAX97236,
				MAX97236_REG_RW_IRQ_MASK2, 0x24);

		if (0 != ret)
			goto err_i2c_write;
	} else {
		/* set IRQ Mask Enable : IJDET */
		ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
				MAX98090_REG_RW_INTERRUPT_MASKS, 0x04);

		if (0 != ret)
			goto err_i2c_write;
	}

	max98090_log_rfunc("ret[%d]", ret);
	return ret;
err_i2c_write:
	max98090_log_err("ret[%d]", ret);
	return ret;
}

/*!
  @brief disable interrupt registers.

  @param none.

  @return function results.
*/
static int max98090_disable_interrupt(void)
{
	int ret = 0;
	max98090_log_efunc("");

	if (MAX98090_EVM == max98090_conf->board) {
		/* set IRQ Mask1 Disable */
		ret = max98090_write(MAX98090_AUDIO_IC_MAX97236,
				MAX97236_REG_RW_IRQ_MASK1, 0x00);

		if (0 != ret)
			goto err_i2c_write;

		/* set IRQ Mask2 Disable */
		ret = max98090_write(MAX98090_AUDIO_IC_MAX97236,
				MAX97236_REG_RW_IRQ_MASK2, 0x00);

		if (0 != ret)
			goto err_i2c_write;
	} else {
		/* set IRQ Mask Disable */
		ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
				MAX98090_REG_RW_INTERRUPT_MASKS, 0x00);

		if (0 != ret)
			goto err_i2c_write;
	}

	max98090_log_rfunc("ret[%d]", ret);
	return ret;
err_i2c_write:
	max98090_log_err("ret[%d]", ret);
	return ret;
}

/*!
  @brief Implementation of interrupt handling.

  @param unused  unused.

  @return none.
*/
static void max98090_irq_work_func(struct work_struct *unused)
{
	int res = 0;
	int state = max98090_conf->switch_data.state;
	int key_press = max98090_conf->switch_data.key_press;
	int status1 = 0;
	int status2 = 0;
	int status3 = 0;
	int enable1 = 0;
	int enable2 = 0;
	int press = 0;
	max98090_log_efunc("");
	mutex_lock(&max98090_mutex);
	res = max98090_disable_interrupt();

	if (MAX98090_EVM == max98090_conf->board) {
		/* read status */
		res = max98090_read(MAX98090_AUDIO_IC_MAX97236,
				MAX97236_REG_OR_STATUS1, &status1);

		if (0 != res)
			max98090_log_err("res[%d]", res);

		res = max98090_read(MAX98090_AUDIO_IC_MAX97236,
				MAX97236_REG_OR_STATUS2, &status2);

		if (0 != res)
			max98090_log_err("res[%d]", res);

		res = max98090_read(MAX98090_AUDIO_IC_MAX97236,
				MAX97236_REG_OR_STATUS3, &status3);

		if (0 != res)
			max98090_log_err("res[%d]", res);

		res = max98090_read(MAX98090_AUDIO_IC_MAX97236,
				MAX97236_REG_RW_ENABLE1, &enable1);

		if (0 != res)
			max98090_log_err("res[%d]", res);

		res = max98090_read(MAX98090_AUDIO_IC_MAX97236,
				MAX97236_REG_RW_ENABLE2, &enable2);

		if (0 != res)
			max98090_log_err("res[%d]", res);

		if (0x80 & enable2) {
			max98090_log_info("jack insert.");

			if (0x60 & status1) {
				/* jack configuration detect done */
				if (0x08 & status1) {
					max98090_log_info("mic.");
					state = 0x01;
				} else {
					max98090_log_info("no mic.");
					state = 0x02;
				}
			}
		} else {
			max98090_log_info("jack remove.");
			state = 0x00;
		}

		if (0x04 & status2) {
			res = max98090_read(MAX98090_AUDIO_IC_MAX97236,
				MAX97236_REG_OR_PASSIVE_MBH_KEYSCAN_DATA,
				&press);

			if (0 != res)
				max98090_log_err("res[%d]", res);

			if (0x80 & press) {
				max98090_log_info("key press.");
				key_press = 0x01;
			} else {
				max98090_log_info("key release.");
				key_press = 0x00;
			}
		}
	} else {
		/* read status */
		res = max98090_read(MAX98090_AUDIO_IC_MAX98090,
				MAX98090_REG_R_DEVICE_STATUS, &status1);

		if (0 != res)
			max98090_log_err("res[%d]", res);

		res = max98090_read(MAX98090_AUDIO_IC_MAX98090,
				MAX98090_REG_R_JACK_DETECT, &status2);

		if (0 != res)
			max98090_log_err("res[%d]", res);

		if (0x04 & status1) {

			if (0x06 == status2) {
				max98090_log_info("jack remove.");
				state = 0x00;
			} else if (0x02 == status2) {

				if (0x01 == max98090_conf->switch_data.state) {
					max98090_log_info("key release.");
					key_press = 0x00;
				} else {
					max98090_log_info("jack insert.");
					max98090_log_info("mic.");
					state = 0x01;
				}
			} else if (0x00 == status2) {

				if (0x01 == max98090_conf->switch_data.state) {
					max98090_log_info("key press.");
					key_press = 0x01;
				} else {
					max98090_log_info("jack insert.");
					max98090_log_info("no mic.");
					state = 0x02;
				}
			} else {
				/* nothing to do. */
				max98090_log_err("status1[0x%x] status2[0x%x]",
						status1, status2);
			}
		}
	}

	if (max98090_conf->switch_data.state != state) {
		max98090_log_info("notify jack state[%d]", state);
		switch_set_state(&max98090_conf->switch_data.sdev, state);
		max98090_conf->switch_data.state = state;
	}

	if (max98090_conf->switch_data.key_press != key_press) {
		max98090_log_info("notify key event[%d]", key_press);
		input_event(max98090_conf->input_dev,
				EV_KEY, KEY_MEDIA, key_press);
		input_sync(max98090_conf->input_dev);
		max98090_conf->switch_data.key_press = key_press;
	}

	/* do callback */
	if (NULL != max98090_conf->callback)
		max98090_conf->callback->irq_func();

	res = max98090_enable_interrupt();
	mutex_unlock(&max98090_mutex);
	max98090_log_rfunc("res[%d] state[%d] event[%d]", res,
			max98090_conf->switch_data.state,
			max98090_conf->switch_data.key_press);
}

/*!
  @brief interrupt handler.

  @param[i] irq  irq number.
  @param[i] data  irq data.

  @return irq return.
*/
static irqreturn_t max98090_irq_handler(int irq, void *data)
{
	max98090_log_efunc("irq[%d] data[%p]", irq, data);
	queue_work(max98090_conf->irq_workqueue, &max98090_conf->irq_work);
	max98090_log_rfunc("");
	return IRQ_HANDLED;
}

/*!
  @brief convert device information struct.

  @param[i] device  original device information.
  @param[o] device_info  converted device information.

  @retval 0 is success.
*/
static int max98090_conv_device_info(const u_long device,
				struct max98090_info *device_info)
{
	int ret = 0;
	max98090_log_efunc("device[%ld]", device);

	/* check param */
	if (NULL == device_info) {
		ret = -EINVAL;
		max98090_log_err("parameter error. ret[%d]", ret);
		return ret;
	}

	device_info->raw_device     = device;
	device_info->speaker = (MAX98090_DEV_PLAYBACK_SPEAKER & device) ?
					MAX98090_ENABLE : MAX98090_DISABLE;
	device_info->earpiece = (MAX98090_DEV_PLAYBACK_EARPIECE & device) ?
					MAX98090_ENABLE : MAX98090_DISABLE;
	device_info->headphone = (MAX98090_DEV_PLAYBACK_HEADPHONES & device) ?
					MAX98090_ENABLE : MAX98090_DISABLE;
	device_info->mic = (MAX98090_DEV_CAPTURE_MIC & device) ?
				MAX98090_ENABLE : MAX98090_DISABLE;
	device_info->headset_mic = (MAX98090_DEV_CAPTURE_HEADSET_MIC & device)
					? MAX98090_ENABLE : MAX98090_DISABLE;
	max98090_log_rfunc("");
	return 0;
}

/*!
  @brief check the range of device information.

  @param[i] device  device information.

  @return function results.
*/
static int max98090_check_device(const u_long device)
{
	int ret = 0;
	u_long dev = (MAX98090_DEV_PLAYBACK_SPEAKER |
		      MAX98090_DEV_PLAYBACK_EARPIECE |
		      MAX98090_DEV_PLAYBACK_HEADPHONES |
		      MAX98090_DEV_CAPTURE_MIC |
		      MAX98090_DEV_CAPTURE_HEADSET_MIC);
	max98090_log_efunc("device[%ld]", device);

	/* check param */
	if (~dev & device) {
		max98090_log_err("device is out of range.");
		ret = -EINVAL;
	}

	max98090_log_rfunc("ret[%d]", ret);
	return ret;
}

/*!
  @brief check the range of volume level.

  @param[i] volume  volume level.

  @return function results.
*/
static int max98090_check_volume_level(const u_int volume)
{
	int ret = 0;
	max98090_log_efunc("volume[%d]", volume);

	/* check param */
	switch (volume) {
	case MAX98090_VOLUMEL0:
	case MAX98090_VOLUMEL1:
	case MAX98090_VOLUMEL2:
	case MAX98090_VOLUMEL3:
	case MAX98090_VOLUMEL4:
	case MAX98090_VOLUMEL5:
		ret = 0;
		break;
	default:
		max98090_log_err("volume is out of range.");
		ret = -EINVAL;
		break;
	}

	max98090_log_rfunc("ret[%d]", ret);
	return ret;
}

/*!
  @brief check the range of volume device.

  @param[i] device  volume device.

  @return function results.
*/
static int max98090_check_volume_device(const u_long device)
{
	int ret = 0;
	u_int dev = (MAX98090_DEV_PLAYBACK_SPEAKER |
		     MAX98090_DEV_PLAYBACK_EARPIECE |
		     MAX98090_DEV_PLAYBACK_HEADPHONES);
	max98090_log_efunc("device[%ld]", device);

	/* check param */
	if (~dev & device) {
		max98090_log_err("device is out of range.");
		ret = -EINVAL;
	}

	max98090_log_rfunc("ret[%d]", ret);
	return ret;
}

/*!
  @brief check the range of mute.

  @param[i] mute  mute setting value.

  @return function results.
*/
static int max98090_check_mute(const u_int mute)
{
	int ret = 0;
	max98090_log_efunc("mute[%d]", mute);

	/* check param */
	switch (mute) {
	case MAX98090_MUTE_DISABLE:
	case MAX98090_MUTE_ENABLE:
		ret = 0;
		break;
	default:
		max98090_log_err("mute is out of range.");
		ret = -EINVAL;
		break;
	}

	max98090_log_rfunc("ret[%d]", ret);
	return ret;
}

/*!
  @brief check the range of speaker amplifiers value.

  @param[i] value  speaker amplifiers setting value.

  @return function results.
*/
static int max98090_check_speaker_amp(const u_int value)
{
	int ret = 0;
	max98090_log_efunc("value[%d]", value);

	/* check param */
	switch (value) {
	case MAX98090_SPEAKER_AMP_DISABLE:
	case MAX98090_SPEAKER_AMP_ENABLE:
		ret = 0;
		break;
	default:
		max98090_log_err("value is out of range.");
		ret = -EINVAL;
		break;
	}

	max98090_log_rfunc("ret[%d]", ret);
	return ret;
}

/*!
  @brief check the range of dump type.

  @param[i] dump_type  dunp type setting value.

  @return function results.
*/
static int max98090_check_dump_type(const u_int dump_type)
{
	int ret = 0;
	max98090_log_efunc("dump_type[%d]", dump_type);

	/* check param */
	switch (dump_type) {
	case MAX98090_AUDIO_IC_ALL:
	case MAX98090_AUDIO_IC_MAX98090:
		ret = 0;
		break;
	case MAX98090_AUDIO_IC_MAX97236:

		if (MAX98090_EVM == max98090_conf->board)
			ret = 0;
		else
			ret = -EINVAL;

		break;
	default:
		max98090_log_err("dump_type is out of range.");
		ret = -EINVAL;
		break;
	}

	max98090_log_rfunc("ret[%d]", ret);
	return ret;
}

/*!
  @brief active the device.

  @param[i] new_device  device information.

  @return function results.
*/
static int max98090_set_device_active(const struct max98090_info new_device)
{
	int ret = 0;
	u_int val = 0;
	max98090_log_efunc("new_device.raw_device[%ld]",
			new_device.raw_device);

	if (0 != new_device.raw_device) {
		/* Register 0x45 (shutdown) = 0x80 (Device Enabled) */
		val = 0x80;
	} else {
		/* Register 0x45 (shutdown) = 0x00 (Device disabled) */
		val = 0x00;
	}

	ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
			     MAX98090_REG_RW_DEVICE_SHUTDOWN, val);
	max98090_log_rfunc("ret[%d] val[0x%02X]", ret, val);
	return ret;
}

/*!
  @brief change state of speaker device.

  @param[i] cur_dev  cureent device state.
  @param[i] new_dev  new device state.

  @return function results.
*/
static int max98090_set_speaker_device(const u_int cur_dev,
				const u_int new_dev)
{
	int ret = 0;
	int io = 0;
	int oe = 0;
	int mix_l = 0;
	int mix_r = 0;
	max98090_log_efunc("cur_dev[%d] new_dev[%d]", cur_dev, new_dev);

	if (cur_dev != new_dev) {
		ret = max98090_read(MAX98090_AUDIO_IC_MAX98090,
				    MAX98090_REG_RW_I_OR_O_CONFIGURATION, &io);

		if (0 != ret)
			goto err_i2c_read;

		ret = max98090_read(MAX98090_AUDIO_IC_MAX98090,
				    MAX98090_REG_RW_OUTPUT_ENABLE, &oe);

		if (0 != ret)
			goto err_i2c_read;

		/* update device conf */
		if (MAX98090_ENABLE == new_dev) {
			/* speaker on */
			/* Select Microphone Input 1 to Left Speaker Mixer */
			mix_l = 0x02;
			/* right channel speaker uses left channel speaker
			   clock if both speaker channels are enabled. */
			mix_r = 0x41;

			/* Select Microphone Input 2 to Right Speaker Mixer */
			if (MAX98090_OE_DAC_ENABLE & oe) {
				/* other features are enabled. */
				/* io is update none. */
				oe |= MAX98090_OE_SPEAKER_ENABLE;
			} else {
				io |= MAX98090_IO_SDIEN_ON;
				oe |= (MAX98090_OE_SPEAKER_ENABLE |
					MAX98090_OE_DAC_ENABLE);
			}
		} else {
			/* speaker off */
			mix_l = 0x00;
			mix_r = 0x00;

			if (~(MAX98090_OE_SPEAKER_ENABLE |
				MAX98090_OE_DAC_ENABLE) & oe) {
				/* other features are enabled. */
				/* io is update none. */
				oe &= ~MAX98090_OE_SPEAKER_ENABLE;
			} else {
				io &= ~MAX98090_IO_SDIEN_ON;
				oe &= ~(MAX98090_OE_SPEAKER_ENABLE |
					MAX98090_OE_DAC_ENABLE);
			}
		}

		ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
				     MAX98090_REG_RW_LEFT_SPK_MIXER, mix_l);

		if (0 != ret)
			goto err_i2c_write;

		ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
				     MAX98090_REG_RW_RIGHT_SPK_MIXER, mix_r);

		if (0 != ret)
			goto err_i2c_write;

		ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
				     MAX98090_REG_RW_OUTPUT_ENABLE, oe);

		if (0 != ret)
			goto err_i2c_write;

		ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
				     MAX98090_REG_RW_I_OR_O_CONFIGURATION, io);

		if (0 != ret)
			goto err_i2c_write;
	} else {
		/* nothing to do. */
	}

	max98090_log_rfunc("ret[%d]", ret);
	return ret;
err_i2c_read:
err_i2c_write:
	max98090_log_err("ret[%d]", ret);
	return ret;
}

/*!
  @brief change state of earpiece device.

  @param[i] cur_dev  cureent device state.
  @param[i] new_dev  new device state.

  @return function results.
*/
static int max98090_set_earpiece_device(const u_int cur_dev,
				const u_int new_dev)
{
	int ret = 0;
	int io = 0;
	int oe = 0;
	int mix_l = 0;
	int mix_r = 0;
	max98090_log_efunc("cur_dev[%d] new_dev[%d]", cur_dev, new_dev);

	if (cur_dev != new_dev) {
		ret = max98090_read(MAX98090_AUDIO_IC_MAX98090,
				    MAX98090_REG_RW_I_OR_O_CONFIGURATION, &io);

		if (0 != ret)
			goto err_i2c_read;

		ret = max98090_read(MAX98090_AUDIO_IC_MAX98090,
				    MAX98090_REG_RW_OUTPUT_ENABLE, &oe);

		if (0 != ret)
			goto err_i2c_read;

		/* update device conf */
		if (MAX98090_ENABLE == new_dev) {
			/* earpiece on */
			/* Selects MIC 1 as the Input to the Receiver /
			   Line Out Left Mixer */
			mix_l = 0x01;
			/* LINEOUT mode. Selects MIC 2 as the Input to
			   the Line Out Right Mixer */
			mix_r = 0x82;

			if (MAX98090_OE_DAC_ENABLE & oe) {
				/* other features are enabled. */
				/* io is update none. */
				oe |= MAX98090_OE_EARPIECE_ENABLE;
			} else {
				io |= MAX98090_IO_SDIEN_ON;
				oe |= (MAX98090_OE_EARPIECE_ENABLE |
					MAX98090_OE_DAC_ENABLE);
			}
		} else {
			/* earpiece off */
			mix_l = 0x00;
			mix_r = 0x00;

			if (~(MAX98090_OE_EARPIECE_ENABLE |
				MAX98090_OE_DAC_ENABLE) & oe) {
				/* other features are enabled. */
				/* io is update none. */
				oe &= ~MAX98090_OE_EARPIECE_ENABLE;
			} else {
				io &= ~MAX98090_IO_SDIEN_ON;
				oe &= ~(MAX98090_OE_EARPIECE_ENABLE |
					MAX98090_OE_DAC_ENABLE);
			}
		}

		ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
				     MAX98090_REG_RW_RCV_OR_LOUTL_MIXER,
				     mix_l);

		if (0 != ret)
			goto err_i2c_write;

		ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
				     MAX98090_REG_RW_LOUTR_MIXER,
				     mix_r);

		if (0 != ret)
			goto err_i2c_write;

		ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
				     MAX98090_REG_RW_OUTPUT_ENABLE, oe);

		if (0 != ret)
			goto err_i2c_write;

		ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
				     MAX98090_REG_RW_I_OR_O_CONFIGURATION, io);

		if (0 != ret)
			goto err_i2c_write;
	} else {
		/* nothing to do. */
	}

	max98090_log_rfunc("ret[%d]", ret);
	return ret;
err_i2c_read:
err_i2c_write:
	max98090_log_err("ret[%d]", ret);
	return ret;
}

/*!
  @brief change state of headphone device.

  @param[i] cur_dev  cureent device state.
  @param[i] new_dev  new device state.

  @return function results.
*/
static int max98090_set_headphone_device(const u_int cur_dev,
				const u_int new_dev)
{
	int ret = 0;
	int io = 0;
	int oe = 0;
	max98090_log_efunc("cur_dev[%d] new_dev[%d]", cur_dev, new_dev);

	if (cur_dev != new_dev) {
		ret = max98090_read(MAX98090_AUDIO_IC_MAX98090,
				    MAX98090_REG_RW_I_OR_O_CONFIGURATION, &io);

		if (0 != ret)
			goto err_i2c_read;

		ret = max98090_read(MAX98090_AUDIO_IC_MAX98090,
				    MAX98090_REG_RW_OUTPUT_ENABLE, &oe);

		if (0 != ret)
			goto err_i2c_read;

		/* update device conf */
		if (MAX98090_ENABLE == new_dev) {

			/* headphone on */
			if (MAX98090_OE_DAC_ENABLE & oe) {
				/* other features are enabled. */
				/* io is update none. */
				oe |= MAX98090_OE_HEADPHONE_ENABLE;
			} else {
				io |= MAX98090_IO_SDIEN_ON;
				oe |= (MAX98090_OE_HEADPHONE_ENABLE |
					MAX98090_OE_DAC_ENABLE);
			}
		} else {

			/* headphone off */
			if (~(MAX98090_OE_HEADPHONE_ENABLE |
				MAX98090_OE_DAC_ENABLE) & oe) {
				/* other features are enabled. */
				/* io is update none. */
				oe &= ~MAX98090_OE_HEADPHONE_ENABLE;
			} else {
				io &= ~MAX98090_IO_SDIEN_ON;
				oe &= ~(MAX98090_OE_HEADPHONE_ENABLE |
					MAX98090_OE_DAC_ENABLE);
			}
		}

		ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
				     MAX98090_REG_RW_OUTPUT_ENABLE, oe);

		if (0 != ret)
			goto err_i2c_write;

		ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
				     MAX98090_REG_RW_I_OR_O_CONFIGURATION, io);

		if (0 != ret)
			goto err_i2c_write;
	} else {
		/* nothing to do. */
	}

	max98090_log_rfunc("ret[%d]", ret);
	return ret;
err_i2c_read:
err_i2c_write:
	max98090_log_err("ret[%d]", ret);
	return ret;
}

/*!
  @brief change state of microphone device.

  @param[i] cur_dev  cureent device state.
  @param[i] new_dev  new device state.

  @return function results.
*/
static int max98090_set_mic_device(const u_int cur_dev, const u_int new_dev)
{
	int ret = 0;
	int io = 0;
	int ie = 0;
	int dmic = 0;
	max98090_log_efunc("cur_dev[%d] new_dev[%d]", cur_dev, new_dev);

	if (cur_dev != new_dev) {
		ret = max98090_read(MAX98090_AUDIO_IC_MAX98090,
				    MAX98090_REG_RW_I_OR_O_CONFIGURATION, &io);

		if (0 != ret)
			goto err_i2c_read;

		ret = max98090_read(MAX98090_AUDIO_IC_MAX98090,
				    MAX98090_REG_RW_INPUT_ENABLE, &ie);

		if (0 != ret)
			goto err_i2c_read;

		/* update device conf */
		if (MAX98090_ENABLE == new_dev) {
			/* mic on */
			io |= (MAX98090_IO_SDOEN_ON | MAX98090_IO_HIZOFF_ON);
			ie |= (MAX98090_IE_MIC_ENABLE |
				MAX98090_IE_ADC_ENABLE);
			dmic = 0x30;
		} else {
			/* mic off */
			io &= ~(MAX98090_IO_SDOEN_ON | MAX98090_IO_HIZOFF_ON);
			ie &= ~(MAX98090_IE_MIC_ENABLE |
				MAX98090_IE_ADC_ENABLE);
			dmic = 0x00;
		}

		if (MAX98090_EVM == max98090_conf->board) {
			/* handset mic */
			/* 0x15(LEFT ADC MIXER)
			   0x40:Select IN3-IN4 Differential Input Direct to
			   Left ADC Mixer */
			ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
					MAX98090_REG_RW_LEFT_ADC_MIXER, 0x40);

			if (0 != ret)
				goto err_i2c_write;

			/* 0x16(RIGHT ADC MIXER)
			   0x40:Select IN3-IN4 Differential Input Direct to
			   Right ADC Mixer */
			ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
					MAX98090_REG_RW_RIGHT_ADC_MIXER, 0x40);

			if (0 != ret)
				goto err_i2c_write;

			/* 0x11(MIC2 INPUT LEVEL) 0x2A:PA2EN=0dB PGAM2=10dB */
			ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
				MAX98090_REG_RW_MIC2_INPUT_LEVEL, 0x2A);

			if (0 != ret)
				goto err_i2c_write;

		} else {
			/* digital mic */
			ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
					MAX98090_REG_RW_DIGITAL_MIC, dmic);

			if (0 != ret)
				goto err_i2c_write;
		}


		/* 0x17(LEFT ADC LEVEL)   0x10:AVLG=6dB AVL=3dB */
		ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
				MAX98090_REG_RW_LEFT_ADC_LEVEL, 0x10);

		if (0 != ret)
			goto err_i2c_write;

		/* 0x18(RIGHT ADC LEVEL)  0x10:AVRG=6dB AVR=3dB */
		ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
				MAX98090_REG_RW_RIGHT_ADC_LEVEL, 0x10);

		if (0 != ret)
			goto err_i2c_write;

		ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
				     MAX98090_REG_RW_INPUT_ENABLE, ie);

		if (0 != ret)
			goto err_i2c_write;

		ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
				     MAX98090_REG_RW_I_OR_O_CONFIGURATION, io);

		if (0 != ret)
			goto err_i2c_write;
	} else {
		/* nothing to do. */
	}

	max98090_log_rfunc("ret[%d]", ret);
	return ret;
err_i2c_read:
err_i2c_write:
	max98090_log_err("ret[%d]", ret);
	return ret;
}

/*!
  @brief change state of headset microphone device.

  @param[i] cur_dev  cureent device state.
  @param[i] new_dev  new device state.

  @return function results.
*/
static int max98090_set_headset_mic_device(const u_int cur_dev,
					const u_int new_dev)
{
	int ret = 0;
	int io = 0;
	int ie = 0;
	max98090_log_efunc("cur_dev[%d] new_dev[%d]", cur_dev, new_dev);

	if (cur_dev != new_dev) {
		ret = max98090_read(MAX98090_AUDIO_IC_MAX98090,
				    MAX98090_REG_RW_I_OR_O_CONFIGURATION, &io);

		if (0 != ret)
			goto err_i2c_read;

		ret = max98090_read(MAX98090_AUDIO_IC_MAX98090,
				    MAX98090_REG_RW_INPUT_ENABLE, &ie);

		if (0 != ret)
			goto err_i2c_read;

		/* update device conf */
		if (MAX98090_ENABLE == new_dev) {
			/* mic on */
			io |= (MAX98090_IO_SDOEN_ON | MAX98090_IO_HIZOFF_ON);
			ie |= (MAX98090_IE_MIC_ENABLE |
				MAX98090_IE_ADC_ENABLE);
		} else {
			/* mic off */
			io &= ~(MAX98090_IO_SDOEN_ON | MAX98090_IO_HIZOFF_ON);
			ie &= ~(MAX98090_IE_MIC_ENABLE |
				MAX98090_IE_ADC_ENABLE);
		}

		/* headset mic */
		/* 0x15(LEFT ADC MIXER)
		   0x04:Select Microphone Input 1 to Left ADC Mixer */
		ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
				MAX98090_REG_RW_LEFT_ADC_MIXER, 0x04);

		if (0 != ret)
			goto err_i2c_write;

		/* 0x16(RIGHT ADC MIXER)
		   0x04:Select Microphone Input 1 to Right ADC Mixer */
		ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
				MAX98090_REG_RW_RIGHT_ADC_MIXER, 0x04);

		if (0 != ret)
			goto err_i2c_write;

		/* 0x10(MIC1 INPUT LEVEL)   0x2A:PA1EN=0dB PGAM1=10dB */
		ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
				MAX98090_REG_RW_MIC1_INPUT_LEVEL, 0x2A);

		if (0 != ret)
			goto err_i2c_write;

		if (MAX98090_EVM == max98090_conf->board) {
			/* 0x17(LEFT ADC LEVEL)   0x20:AVRG=12dB AVR=0dB */
			ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
					MAX98090_REG_RW_LEFT_ADC_LEVEL, 0x23);

			if (0 != ret)
				goto err_i2c_write;

			/* 0x18(RIGHT ADC LEVEL)  0x20:AVRG=12dB AVR=0dB */
			ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
					MAX98090_REG_RW_RIGHT_ADC_LEVEL, 0x23);

			if (0 != ret)
				goto err_i2c_write;
		} else {
			/* 0x17(LEFT ADC LEVEL)   0x73:AVRG=42dB AVR=0dB */
			ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
					MAX98090_REG_RW_LEFT_ADC_LEVEL, 0x73);

			if (0 != ret)
				goto err_i2c_write;

			/* 0x18(RIGHT ADC LEVEL)  0x73:AVRG=42dB AVR=0dB */
			ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
					MAX98090_REG_RW_RIGHT_ADC_LEVEL, 0x73);

			if (0 != ret)
				goto err_i2c_write;
		}

		ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
				     MAX98090_REG_RW_INPUT_ENABLE, ie);

		if (0 != ret)
			goto err_i2c_write;

		ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
				     MAX98090_REG_RW_I_OR_O_CONFIGURATION, io);

		if (0 != ret)
			goto err_i2c_write;
	} else {
		/* nothing to do. */
	}

	max98090_log_rfunc("ret[%d]", ret);
	return ret;
err_i2c_read:
err_i2c_write:
	max98090_log_err("ret[%d]", ret);
	return ret;
}

/*!
  @brief display registers of max98090.

  @param none.

  @return function results.
*/
static int max98090_dump_max98090_registers(void)
{
	int ret = 0;
	int i = 0;
	int val = 0;
	max98090_log_debug("");

	for (i = 0; i < MAX98090_REG_ID_MAX; i++) {
		switch (i) {
		case MAX98090_REG_W_SOFTWARE_RESET:
		case MAX98090_REG_W_SYSTEM_CLOCK:
		case MAX98090_REG_W_SAMPLE_RATE:
		case MAX98090_REG_W_INTERFACE:
		case MAX98090_REG_W_DAC_PATH:
		case MAX98090_REG_W_MIC_OR_DIRECT_TO_ADC:
		case MAX98090_REG_W_LINE_TO_ADC:
		case MAX98090_REG_W_ANALOG_MIC_LOOP:
		case MAX98090_REG_W_ANALOG_LINE_LOOP:
			max98090_log_debug("addr[0x%02X] write-only register",
					MAX98090_reg_addr[i]);
			break;
		default:
			ret = max98090_read(MAX98090_AUDIO_IC_MAX98090,
					i, &val);

			if (0 != ret)
				goto err_i2c_read;

			max98090_log_debug("ret[%d] addr[0x%02X] val[0x%02X]",
					ret, MAX98090_reg_addr[i], val);
			break;
		}
	}

	max98090_log_debug("ret[%d]", ret);
	return ret;
err_i2c_read:
	max98090_log_err("ret[%d]", ret);
	return ret;
}

/*!
  @brief display registers of max97236.

  @param none.

  @return function results.
*/
static int max98090_dump_max97236_registers(void)
{
	int ret = 0;
	int i = 0;
	int val = 0;
	max98090_log_debug("");

	for (i = 0; i < MAX97236_REG_ID_MAX; i++) {
		ret = max98090_read(MAX98090_AUDIO_IC_MAX97236, i, &val);

		if (0 != ret)
			goto err_i2c_read;

		max98090_log_debug("ret[%d] addr[0x%02X] val[0x%02X]",
				ret, MAX97236_reg_addr[i], val);
	}

	max98090_log_debug("ret[%d]", ret);
	return ret;
err_i2c_read:
	max98090_log_err("ret[%d]", ret);
	return ret;
}

#ifdef __MAX98090_DEBUG__
/*!
  @brief display private data of AudioLSI driver.

  @param none.

  @return function results.
*/
static void max98090_dump_max98090_priv(void)
{
	int i = 0;

	if (max98090_conf) {
		max98090_log_debug("-------------------------------------");
		max98090_log_debug("max98090_conf[0x%p]", max98090_conf);
		max98090_log_debug("max98090_conf->board[%d]",
				max98090_conf->board);
		max98090_log_debug("max98090_conf->info.raw_device[0x%0lx]",
				max98090_conf->info.raw_device);
		max98090_log_debug("max98090_conf->info.speaker[%d]",
				max98090_conf->info.speaker);
		max98090_log_debug("max98090_conf->info.earpiece[%d]",
				max98090_conf->info.earpiece);
		max98090_log_debug("max98090_conf->info.headphone[%d]",
				max98090_conf->info.headphone);
		max98090_log_debug("max98090_conf->info.mic[%d]",
				max98090_conf->info.mic);
		max98090_log_debug("max98090_conf->info.headset_mic[%d]",
				max98090_conf->info.headset_mic);

		for (i = 0; i < MAX98090_VOL_DEV_MAX; i++) {
			max98090_log_debug(
				"max98090_conf->info.volume[%d]=[%d]", i,
				max98090_conf->info.volume[i]);
		}

		max98090_log_debug("max98090_conf->info.mute[%d]",
				max98090_conf->info.mute);
		max98090_log_debug("max98090_conf->info.speaker_amp[%d]",
				max98090_conf->info.speaker_amp);
		max98090_log_debug("max98090_conf->client_max97236[0x%p]",
				max98090_conf->client_max97236);
		max98090_log_debug("max98090_conf->client_max98090[0x%p]",
				max98090_conf->client_max98090);
		max98090_log_debug("max98090_conf->irq[%d]",
				max98090_conf->irq);
		max98090_log_debug("max98090_conf->callback[0x%p]",
				max98090_conf->callback);
		max98090_log_debug("max98090_conf->irq_workqueue[0x%p]",
				max98090_conf->irq_workqueue);
		max98090_log_debug("max98090_conf->irq_work[0x%p]",
				&max98090_conf->irq_work);
		max98090_log_debug("max98090_conf->log_entry[0x%p]",
				max98090_conf->log_entry);
		max98090_log_debug("max98090_conf->dump_entry[0x%p]",
				max98090_conf->dump_entry);
		max98090_log_debug("max98090_conf->config_entry[0x%p]",
				max98090_conf->config_entry);
		max98090_log_debug("max98090_conf->path_entry[0x%p]",
				max98090_conf->path_entry);
		max98090_log_debug("max98090_conf->proc_parent[0x%p]",
				max98090_conf->proc_parent);
		max98090_log_debug("-------------------------------------");
	} else {
		max98090_log_debug("-------------------------------------");
		max98090_log_debug("max98090_conf is NULL");
		max98090_log_debug("-------------------------------------");
	}
}
#endif /* __MAX98090_DEBUG__ */

/*!
  @brief write audio ic regster value.

  @param[i] audio_ic    audio ic device.
  @param[i] addr        audio ic register address.
  @param[i] value       set register value.
  @param[i] before_wait before waitting time.
  @param[i] after_wait  after waitting time.

  @return function results.
*/

static int max98090_write_config(const u_int audio_ic, const u_int addr,
		const u_int value, const u_int before_wait,
		const u_int after_wait)
{
	int ret = 0;
	max98090_log_debug("");

	udelay(before_wait);
	max98090_log_info("before_wait[%d]", before_wait);
	switch (audio_ic) {
	case MAX98090_AUDIO_IC_MAX98090:
		max98090_log_info("addr[0x%02X] value[0x%02X]",
				addr, value);
		ret = i2c_smbus_write_byte_data(
					max98090_conf->client_max98090,
					addr, value);
		break;
	case MAX98090_AUDIO_IC_MAX97236:

		if (MAX98090_EVM == max98090_conf->board) {
			max98090_log_info("addr[0x%02X] value[0x%02X]",
					addr, value);
			ret = i2c_smbus_write_byte_data(
						max98090_conf->client_max97236,
						addr,
						value);
		} else {
			/* nothing to do. */
		}

		break;
	default:
		max98090_log_err("device is out of range");
		ret = -EINVAL;
		break;
	}
	udelay(after_wait);
	max98090_log_info("after_wait[%d]", after_wait);

	max98090_log_debug("ret[%d]", ret);
	return ret;
}
/*!
  @brief check configration value.

  @param[i] config_val   configration value.

  @return function results.
*/

static int max98090_set_data(u_int config_val[])
{
	int ret = 0;
	int device = -1;

	max98090_log_efunc("");

	if (0x10 == config_val[E_SLV_ADDR]) {
		device = 0;
	} else if (0x40 == config_val[E_SLV_ADDR]) {
		device = 1;
	} else {
		max98090_log_err("device is out of range");
		ret = -EINVAL;
		goto err;
	}

	if (MAX98090_WAIT_MAX < config_val[E_B_WAIT])
		config_val[E_B_WAIT] = MAX98090_WAIT_MAX;
	if (MAX98090_WAIT_MAX < config_val[E_A_WAIT])
		config_val[E_A_WAIT] = MAX98090_WAIT_MAX;

	ret = max98090_write_config(device, config_val[E_REG_ADDR],
					config_val[E_REG_VAL],
					config_val[E_B_WAIT],
					config_val[E_A_WAIT]);

	max98090_log_rfunc("ret[%d]", ret);
	return ret;

err:
	max98090_log_rfunc("ret[%d]", ret);
	return ret;
}

/*!
  @brief  config parameter setting.

  @param[i]buf   data of one line.

  @return function results.
*/


static int max98090_set_config(char *buf)
{
	int ret = 0;
	int length = 0;
	int dev_flag = 0;
	int i = 0;
	int j = 0;
	int check = 0;
	char temp[81] = {'\0'};
	u_int config_val[5] = {0};

	max98090_log_efunc("");
	max98090_log_info("buffer[%s]", buf);
	length = strlen(buf);
	if (80 < length)
		length = MAX98090_LEN_MAX_ONELINE;

	if ('\0' == buf[0])
		goto comment;

	for (i = 0; i <= length; i++) {
		if ('#' == buf[i]) {
			if (E_SLV_ADDR == dev_flag) {
				goto comment;
			} else if (E_REG_VAL <= dev_flag &&
					E_A_WAIT >= dev_flag) {
				temp[j] = '\0';
				ret = kstrtoint(temp, 0,
						&config_val[dev_flag]);
				if (0 != ret) {
					max98090_log_err("not value");
					ret = -EFAULT;
					goto err;
				}
				max98090_log_info("[%d]", config_val[dev_flag]);
				goto end;
			} else {
				goto err;
			}
		} else if (' ' == buf[i]
			|| ',' == buf[i]
			|| '\t' == buf[i]
			|| '\0' == buf[i]) {
			if (dev_flag < E_MAX) {
				if (check == MAX98090_VAL_TRUE) {
					temp[j] = '\0';
					ret = kstrtoint(temp, 0,
							&config_val[dev_flag]);
					if (0 != ret) {
						max98090_log_err("not value");
						ret = -EFAULT;
						goto err;
					}
					max98090_log_info("[%d]",
							config_val[dev_flag]);
					check = 0;
					j = 0;
				}
				if (',' == buf[i])
					dev_flag++;
			}
		} else {
			temp[j++] = buf[i];
			check = MAX98090_VAL_TRUE;
		}
	}
	if (E_REG_VAL <= dev_flag)
		goto end;

err:
	max98090_log_info("not parameter line[%d]", ret);
comment:
	max98090_log_rfunc("ret[%d]", ret);
	return ret;
end:
	max98090_set_data(config_val);
	max98090_log_rfunc("ret[%d]", ret);
	return ret;
}

/*!
  @brief open configration file.

  @param[i] config_path   configration file path.
  @param[i] config_filp   configraiton file pointer.

  @return function results.
*/

static int max98090_open_file(char *config_path, struct file **config_filp)
{
	int ret = 0;
	max98090_log_efunc("");

	max98090_log_debug("path[%s]", config_path);

	*config_filp = filp_open(config_path, O_RDONLY , 0);
	if (IS_ERR(*config_filp)) {
		max98090_log_err("can't open file:err [%ld]\n",
					PTR_ERR(*config_filp));
		ret = PTR_ERR(*config_filp);
		goto err;
	}

	if (!S_ISREG((*config_filp)->f_dentry->d_inode->i_mode)) {
		max98090_log_err("access error\n");
		ret = -EACCES;
		goto err;
	}
	max98090_log_rfunc("ret[%d]", ret);
	return ret;
err:
	max98090_log_rfunc("ret[%d]", ret);
	return ret;
}

/*!
  @brief read configration one line.

  @param[i] config_filp   configration file pointer.
  @param[o] buf           data of one line.

  @return function results.
*/

static int max98090_read_line(struct file *config_filp, char *buf)
{
	mm_segment_t fs;
	int ret = 0;
	int pos = 0;
	char dummy = '\0';
	fs = get_fs();

	max98090_log_efunc("");

	max98090_log_debug("config[0x%08X]", (unsigned int)config_filp);

	/* read one line */
	for (pos = 0; pos < MAX98090_LEN_MAX_ONELINE; pos++) {
		set_fs(KERNEL_DS);
		ret = config_filp->f_op->read(config_filp, buf + pos, 1,
						&config_filp->f_pos);
		set_fs(fs);
		if (1 != ret) {
			buf[pos] = '\0';
			goto end;
		}
		if ('\n' == buf[pos]) {
			ret = 0;
			break;
		}
	}

	/* move the line of end */
	if (MAX98090_LEN_MAX_ONELINE == pos) {
		do {
			set_fs(KERNEL_DS);
			ret = config_filp->f_op->read(config_filp,
						&dummy, 1,
						&config_filp->f_pos);
			set_fs(fs);
			if (1 != ret) {
				buf[MAX98090_LEN_MAX_ONELINE] = '\0';
				goto end;
			}
		} while ('\n' != dummy);
		pos = MAX98090_LEN_MAX_ONELINE;
		ret = 0;
	}
	buf[pos] = '\0';

	max98090_log_rfunc("");
	/* reading remainder */
	return 0;
end:
	max98090_log_rfunc("");
	/* end of read */
	return -1;
}

/*!
  @brief close configration file.

  @param[i] config_filp   configration file pointer.

  @return function results.
*/

static int max98090_close_file(struct file *config_filp)
{
	int ret = 0;
	max98090_log_efunc("");

	filp_close(config_filp, NULL);
	config_filp = NULL;

	max98090_log_rfunc("");
	return ret;
}

/*!
  @brief close configration file.

  @param[i] pcm_name   pcm name.
  @param[i] pcm_path   pcm file base path.

  @return function results.
*/

static int max98090_read_config(char *pcm_name, char *pcm_path)
{
	int ret = 0;
	char *device = NULL;
	char buf[81] = {'\0'};
	u_int count = 0;
	u_int check = 0;
	struct file *config_filp = NULL;
	max98090_log_efunc("");

	count = strlen(pcm_name);
	device = kmalloc(count + MAX98090_MAX_PATH_LENGTH, GFP_KERNEL);
	memset(device, 0, count + MAX98090_MAX_PATH_LENGTH);

	strcat(device, pcm_path);
	strcat(device, pcm_name);
	strcat(device, MAX98090_EXTENSION);

	ret = max98090_open_file(device, &config_filp);
	if (0 != ret)
		goto err;

	do {
		check = max98090_read_line(config_filp, buf);
		max98090_set_config(buf);
	} while (0 == check);

	kfree(device);

	max98090_close_file(config_filp);
	max98090_log_rfunc("ret[%d]", ret);
	return ret;
err:
	kfree(device);

	max98090_log_rfunc("ret[%d]", ret);
	return ret;
}
/*------------------------------------*/
/* for public function		*/
/*------------------------------------*/
int max98090_set_device(const u_long device, const u_int pcm_value)
{
	int ret = 0;

	char pcm_buf[SNDP_PCM_NAME_MAX_LEN] = {'\0'};
	struct max98090_info new_device = max98090_conf->info;
	max98090_log_efunc("device[%ld]", device);
#ifdef __MAX98090_DEBUG__
	max98090_dump_max98090_priv();
#endif /* __MAX98090_DEBUG__ */
	/* check param */
	ret = max98090_check_device(device);

	if (0 != ret) {
		max98090_log_err("parameter error. ret[%d]", ret);
		return ret;
	}

	/* check update */
	if (max98090_conf->info.raw_device == device) {
		max98090_log_rfunc("no changed. ret[%d]", ret);
		return ret;
	}

	ret = max98090_conv_device_info(device, &new_device);

	if (0 != ret) {
		max98090_log_err("device convert error. ret[%d]", ret);
		return ret;
	}

	mutex_lock(&max98090_mutex);

	/* set value */
	ret = max98090_set_speaker_device(max98090_conf->info.speaker,
					  new_device.speaker);

	if (0 != ret)
		goto err_set_device;

	ret = max98090_set_earpiece_device(max98090_conf->info.earpiece,
					   new_device.earpiece);

	if (0 != ret)
		goto err_set_device;

	ret = max98090_set_headphone_device(max98090_conf->info.headphone,
					    new_device.headphone);

	if (0 != ret)
		goto err_set_device;

	ret = max98090_set_mic_device(max98090_conf->info.mic,
				      new_device.mic);

	if (0 != ret)
		goto err_set_device;

	ret = max98090_set_headset_mic_device(max98090_conf->info.headset_mic,
					      new_device.headset_mic);

	if (0 != ret)
		goto err_set_device;

	ret = max98090_set_device_active(new_device);

	if (0 != ret)
		goto err_set_device;

	if (pcm_value != MAX98090_DISABLE_CONFIG &&
		pcm_value != MAX98090_DISABLE_CONFIG_SUB) {
		memset(pcm_buf, '\0', sizeof(pcm_buf));
		sndp_pcm_name_generate(pcm_value, pcm_buf);
		max98090_log_info("PCM: %s [0x%08X]\n", pcm_buf, pcm_value);
		max98090_read_config(pcm_buf, max98090_configration_path);
	}

	mutex_unlock(&max98090_mutex);

	/* update internal value */
	max98090_conf->info = new_device;
#ifdef __MAX98090_DEBUG__
	max98090_dump_max98090_priv();
#endif /* __MAX98090_DEBUG__ */
	max98090_log_rfunc("ret[%d]", ret);
	return ret;
err_set_device:
	mutex_unlock(&max98090_mutex);
	max98090_log_err("ret[%d]", ret);
	return ret;
}
EXPORT_SYMBOL(max98090_set_device);

int max98090_get_device(u_long *device)
{
	int ret = 0;
	max98090_log_efunc("");

	/* check param */
	if (NULL == device) {
		ret = -EINVAL;
		max98090_log_err("parameter error. device is null. ret[%d]",
				ret);
		return ret;
	}

	/* get value */
	*device = max98090_conf->info.raw_device;
	max98090_log_rfunc("ret[%d] device[%ld]", ret, *device);
	return ret;
}
EXPORT_SYMBOL(max98090_get_device);

int max98090_set_volume(const u_long device, const u_int volume)
{
	int ret = 0;
	int i = 0;
	int vol_index = 0;
	u_int *volume_table_addr = NULL;
	max98090_log_efunc("device[%ld] volume[%d]", device, volume);
	/* check param */
	ret = max98090_check_volume_device(device);

	if (0 != ret) {
		max98090_log_err("parameter error. ret[%d]", ret);
		return ret;
	}

	ret = max98090_check_volume_level(volume);

	if (0 != ret) {
		max98090_log_err("parameter error. ret[%d]", ret);
		return ret;
	}

	if (MAX98090_EVM == max98090_conf->board)
		volume_table_addr = (u_int *)MAX98090_EVM_volume;
	else
		volume_table_addr = (u_int *)MAX98090_E2K_volume;

	/* convert volume value to index */
	for (i = 0; i < MAX98090_VOLUMEL_ELEMENT; i++) {

		if (*(volume_table_addr + i) == volume) {
			vol_index = i;
			break;
		}
	}
	mutex_lock(&max98090_mutex);
	/* set speaker volume */
	if (MAX98090_DEV_PLAYBACK_SPEAKER & device) {
		ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
			MAX98090_REG_RW_LEFT_SPK_VOLUME,
			*(volume_table_addr +
			MAX98090_VOLUMEL_SET(MAX98090_VOL_DEV_SPEAKER) +
			vol_index));

		if (0 != ret)
			goto err_i2c_write;

		ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
			MAX98090_REG_RW_RIGHT_SPK_VOLUME,
			*(volume_table_addr +
			MAX98090_VOLUMEL_SET(MAX98090_VOL_DEV_SPEAKER) +
			vol_index));

		if (0 != ret)
			goto err_i2c_write;
		else
			max98090_conf->info.volume[MAX98090_VOL_DEV_SPEAKER] =
			volume;
	}

	/* set earpiece volume */
	if (MAX98090_DEV_PLAYBACK_EARPIECE & device) {
		ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
			MAX98090_REG_RW_RCV_OR_LOUTL_VOLUME,
			*(volume_table_addr +
			MAX98090_VOLUMEL_SET(MAX98090_VOL_DEV_EARPIECE) +
			vol_index));

		if (0 != ret)
			goto err_i2c_write;

		ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
			MAX98090_REG_RW_LOUTR_VOLUME,
			*(volume_table_addr +
			MAX98090_VOLUMEL_SET(MAX98090_VOL_DEV_EARPIECE) +
			vol_index));

		if (0 != ret)
			goto err_i2c_write;
		else
			max98090_conf->info.volume[MAX98090_VOL_DEV_EARPIECE] =
			volume;
	}

	/* set headphone volume */
	if (MAX98090_DEV_PLAYBACK_HEADPHONES & device) {
		ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
			MAX98090_REG_RW_LEFT_HP_VOLUME,
			*(volume_table_addr +
			MAX98090_VOLUMEL_SET(MAX98090_VOL_DEV_HEADPHONES) +
			vol_index));

		if (0 != ret)
			goto err_i2c_write;

		ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
			MAX98090_REG_RW_RIGHT_HP_VOLUME,
			*(volume_table_addr +
			MAX98090_VOLUMEL_SET(MAX98090_VOL_DEV_HEADPHONES) +
			vol_index));

		if (0 != ret)
			goto err_i2c_write;
		else
			max98090_conf->info.volume[MAX98090_VOL_DEV_HEADPHONES]
			= volume;
	}
	mutex_unlock(&max98090_mutex);
	max98090_log_rfunc("ret[%d]", ret);
	return ret;
err_i2c_write:
	mutex_unlock(&max98090_mutex);
	max98090_log_err("ret[%d]", ret);
	return ret;
}
EXPORT_SYMBOL(max98090_set_volume);

int max98090_get_volume(const u_long device, u_int *volume)
{
	int ret = 0;
	max98090_log_efunc("device[%ld]", device);
	/* check param */
	ret = max98090_check_volume_device(device);

	if (0 != ret) {
		max98090_log_err("parameter error. ret[%d]", ret);
		return ret;
	}

	if (NULL == volume) {
		ret = -EINVAL;
		max98090_log_err("parameter error. volume is null. ret[%d]",
				ret);
		return ret;
	}

	/* get value */
	if (MAX98090_DEV_PLAYBACK_SPEAKER & device) {
		*volume =
		max98090_conf->info.volume[MAX98090_VOL_DEV_SPEAKER];
	} else if (MAX98090_DEV_PLAYBACK_EARPIECE & device) {
		*volume =
		max98090_conf->info.volume[MAX98090_VOL_DEV_EARPIECE];
	} else if (MAX98090_DEV_PLAYBACK_HEADPHONES & device) {
		*volume =
		max98090_conf->info.volume[MAX98090_VOL_DEV_HEADPHONES];
	} else {
		/* nothing to do. */
	}

	max98090_log_rfunc("ret[%d] volume[%d]", ret, *volume);
	return ret;
}
EXPORT_SYMBOL(max98090_get_volume);

int max98090_set_mute(const u_int mute)
{
	int ret = 0;
	int io = 0;
	int ie = 0;
	max98090_log_efunc("mute[%d]", mute);

	/* check param */
	ret = max98090_check_mute(mute);

	if (0 != ret) {
		max98090_log_err("parameter error. ret[%d]", ret);
		return ret;
	}

	/* check device enable */
	if ((MAX98090_DISABLE == max98090_conf->info.mic) &&
		(MAX98090_DISABLE == max98090_conf->info.headset_mic)) {
		max98090_log_err("none mute device. ret[%d]", ret);
		return ret;
	}

	/* check update */
	if (max98090_conf->info.mute == mute) {
		max98090_log_rfunc("no changed. ret[%d]", ret);
		return ret;
	}

	/* get current value */
	ret = max98090_read(MAX98090_AUDIO_IC_MAX98090,
			    MAX98090_REG_RW_I_OR_O_CONFIGURATION, &io);

	if (0 != ret)
		goto err_i2c_read;

	ret = max98090_read(MAX98090_AUDIO_IC_MAX98090,
			    MAX98090_REG_RW_INPUT_ENABLE, &ie);

	if (0 == ret) {
		switch (mute) {
		case MAX98090_MUTE_DISABLE:
			/* mic on */
			io |= (MAX98090_IO_SDOEN_ON | MAX98090_IO_HIZOFF_ON);
			ie |= (MAX98090_IE_MIC_ENABLE |
				MAX98090_IE_ADC_ENABLE);
			break;
		case MAX98090_MUTE_ENABLE:
			/* mic off */
			io &= ~(MAX98090_IO_SDOEN_ON | MAX98090_IO_HIZOFF_ON);
			ie &= ~(MAX98090_IE_MIC_ENABLE |
				MAX98090_IE_ADC_ENABLE);
			break;
		default:
			/* nothing to do. */
			break;
		}
		mutex_lock(&max98090_mutex);
		/* set value */
		ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
				     MAX98090_REG_RW_INPUT_ENABLE, ie);

		if (0 != ret)
			goto err_i2c_write;

		ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
				     MAX98090_REG_RW_I_OR_O_CONFIGURATION, io);

		if (0 != ret)
			goto err_i2c_write;

		if (0 == ret)
			max98090_conf->info.mute = mute;

		mutex_unlock(&max98090_mutex);
	}
	max98090_log_rfunc("ret[%d]", ret);
	return ret;
err_i2c_write:
	mutex_unlock(&max98090_mutex);
err_i2c_read:
	max98090_log_err("ret[%d]", ret);
	return ret;
}
EXPORT_SYMBOL(max98090_set_mute);

int max98090_get_mute(u_int *mute)
{
	int ret = 0;
	max98090_log_efunc("");

	/* check param */
	if (NULL == mute) {
		ret = -EINVAL;
		max98090_log_err("parameter error. mute is null. ret[%d]",
				ret);
		return ret;
	}

	/* get value */
	*mute = max98090_conf->info.mute;
	max98090_log_rfunc("ret[%d] mute[%d]", ret, *mute);
	return ret;
}
EXPORT_SYMBOL(max98090_get_mute);

int max98090_set_speaker_amp(const u_int value)
{
	int ret = 0;
	max98090_log_efunc("value[%d]", value);
	/* check param */
	ret = max98090_check_speaker_amp(value);

	if (0 != ret) {
		max98090_log_err("parameter error. ret[%d]", ret);
		return ret;
	}

	/* check update */
	if (max98090_conf->info.speaker_amp == value) {
		max98090_log_rfunc("no changed. ret[%d]", ret);
		return ret;
	}

	/* set value */
	if (MAX98090_EVM == max98090_conf->board) {
		switch (value) {
		case MAX98090_SPEAKER_AMP_DISABLE:
			gpio_set_value(GPIO_PORT29, 0);
			break;
		case MAX98090_SPEAKER_AMP_ENABLE:
			gpio_set_value(GPIO_PORT29, 1);
			break;
		default:
			/* nothing to do. */
			break;
		}
	}

	max98090_conf->info.speaker_amp = value;
	max98090_log_rfunc("ret[%d]", ret);
	return ret;
}
EXPORT_SYMBOL(max98090_set_speaker_amp);

int max98090_get_speaker_amp(u_int *value)
{
	int ret = 0;
	max98090_log_efunc("");

	/* check param */
	if (NULL == value) {
		ret = -EINVAL;
		max98090_log_err("parameter error. value is null. ret[%d]",
				ret);
		return ret;
	}

	/* get value */
	*value = max98090_conf->info.speaker_amp;
	max98090_log_rfunc("ret[%d] value[%d]", ret, *value);
	return ret;
}
EXPORT_SYMBOL(max98090_get_speaker_amp);

int max98090_get_status(u_long *irq_status)
{
	int ret = 0;
	int status1 = 0;
	int status2 = 0;
	max98090_log_efunc("");

	/* check param */
	if (NULL == irq_status) {
		ret = -EINVAL;
		max98090_log_err("parameter error. irq_status is null.ret[%d]",
				 ret);
		return ret;
	}

	if (MAX98090_EVM == max98090_conf->board) {
		ret = max98090_read(MAX98090_AUDIO_IC_MAX97236,
				MAX97236_REG_OR_STATUS1, &status1);
		ret = max98090_read(MAX98090_AUDIO_IC_MAX97236,
				MAX97236_REG_OR_STATUS2, &status2);
		*irq_status = (status1 << 8);
		*irq_status |= status2;
	} else {
		ret = max98090_read(MAX98090_AUDIO_IC_MAX98090,
				MAX98090_REG_R_DEVICE_STATUS, &status1);
		ret = max98090_read(MAX98090_AUDIO_IC_MAX98090,
				MAX98090_REG_R_JACK_DETECT, &status2);
		*irq_status = (status1 << 8);
		*irq_status |= status2;
	}

	max98090_log_rfunc("ret[%d] irq_status[%ld]", ret, *irq_status);
	return ret;
}
EXPORT_SYMBOL(max98090_get_status);

int max98090_register_callback_func(struct max98090_callback_func *callback_)
{
	int ret = 0;
	max98090_log_efunc("");

	/* check param */
	if (NULL == callback_) {
		ret = -EINVAL;
		max98090_log_err("parameter error. callback_ is null.ret[%d]",
				ret);
		return ret;
	}

	if (NULL == callback_->irq_func) {
		ret = -EINVAL;
		max98090_log_err(
			"parameter error. callback_->irq_func is null.ret[%d]",
			ret);
		return ret;
	}

	/* set value */
	max98090_conf->callback = callback_;
	max98090_log_rfunc("ret[%d]", ret);
	return ret;
}
EXPORT_SYMBOL(max98090_register_callback_func);

int max98090_dump_registers(const u_int dump_type)
{
	int ret = 0;
	max98090_log_debug("dump_type[%d]", dump_type);
	/* check param */
	ret = max98090_check_dump_type(dump_type);

	if (0 != ret) {
		max98090_log_err("parameter error. ret[%d]", ret);
		return ret;
	}

	/* selected dump type */
	switch (dump_type) {
	case MAX98090_AUDIO_IC_ALL:
		ret = max98090_dump_max98090_registers();

		if (0 == ret)
			ret = max98090_dump_max97236_registers();

		break;
	case MAX98090_AUDIO_IC_MAX98090:
		ret = max98090_dump_max98090_registers();
		break;
	case MAX98090_AUDIO_IC_MAX97236:
		ret = max98090_dump_max97236_registers();
		break;
	default:
		/* nothing to do. */
		break;
	}

	max98090_log_debug("ret[%d]", ret);
	return ret;
}
EXPORT_SYMBOL(max98090_dump_registers);

int max98090_read(const u_int audio_ic, const u_int addr_id, int *value)
{
	int ret = 0;
	max98090_log_debug("");

	switch (audio_ic) {
	case MAX98090_AUDIO_IC_MAX98090:
		*value = i2c_smbus_read_byte_data(
						max98090_conf->client_max98090,
						MAX98090_reg_addr[addr_id]);
		max98090_log_info("MAX98090 addr[0x%02X] value[0x%02X]",
				MAX98090_reg_addr[addr_id], *value);
		break;
	case MAX98090_AUDIO_IC_MAX97236:

		if (MAX98090_EVM == max98090_conf->board) {
			*value = i2c_smbus_read_byte_data(
						max98090_conf->client_max97236,
						MAX97236_reg_addr[addr_id]);
			max98090_log_info(
				"MAX97236 addr[0x%02X] value[0x%02X]",
				MAX97236_reg_addr[addr_id], *value);
		} else {
			max98090_log_err("device is none.");
			ret = -EINVAL;
		}

		break;
	default:
		max98090_log_err("device is out of range.");
		ret = -EINVAL;
		break;
	}

	max98090_log_debug("ret[%d]", ret);
	return ret;
}
EXPORT_SYMBOL(max98090_read);

int max98090_write(const u_int audio_ic, const u_int addr_id,
		const u_int value)
{
	int ret = 0;
	max98090_log_debug("");

	switch (audio_ic) {
	case MAX98090_AUDIO_IC_MAX98090:
		max98090_log_info("addr[0x%02X] value[0x%02X]",
				MAX98090_reg_addr[addr_id], value);
		ret = i2c_smbus_write_byte_data(
					max98090_conf->client_max98090,
					MAX98090_reg_addr[addr_id], value);
		break;
	case MAX98090_AUDIO_IC_MAX97236:

		if (MAX98090_EVM == max98090_conf->board) {
			max98090_log_info("addr[0x%02X] value[0x%02X]",
					MAX97236_reg_addr[addr_id], value);
			ret = i2c_smbus_write_byte_data(
						max98090_conf->client_max97236,
						MAX97236_reg_addr[addr_id],
						value);
		} else {
			max98090_log_err("device is none.");
			ret = -EINVAL;
		}

		break;
	default:
		max98090_log_err("device is out of range.");
		ret = -EINVAL;
		break;
	}

	max98090_log_debug("ret[%d]", ret);
	return ret;
}
EXPORT_SYMBOL(max98090_write);

/*------------------------------------*/
/* for system                         */
/*------------------------------------*/
static int max98090_proc_log_read(char *page, char **start, off_t offset,
			int count, int *eof, void *data)
{
	int len = 0;
	max98090_log_efunc("");
	len = sprintf(page, "0x%08x\n", (int)max98090_log_level);
	*eof = 1;
	max98090_log_rfunc("len[%d]", len);
	return len;
}

static int max98090_proc_log_write(struct file *filp, const char *buffer,
	unsigned long count, void *data)
{
	int r = 0;
	int in = 0;
	char *temp = NULL;
	max98090_log_efunc("filp[%p] buffer[%p] count[%ld] data[%p]",
			filp, buffer, count, data);
	temp = kmalloc(count, GFP_KERNEL);
	memset(temp, 0, count);
	strncpy(temp, buffer, count);
	temp[count-1] = '\0';
	r = kstrtoint(temp, 0, &in);
	kfree(temp);

	if (r)
		return r;

	max98090_log_level = (u_int)in & MAX98090_LOG_LEVEL_MAX;
	max98090_log_rfunc("count[%ld]", count);
	return count;
}

static int max98090_proc_dump_read(char *page, char **start, off_t offset,
			int count, int *eof, void *data)
{
	int len = 0;
	max98090_log_efunc("");
	len = sprintf(page, "0x%08x\n", (int)max98090_dump_reg);
	*eof = 1;
	max98090_log_rfunc("len[%d]", len);
	return len;
}

static int max98090_proc_dump_write(struct file *filp, const char *buffer,
	unsigned long count, void *data)
{
	int r = 0;
	int in = 0;
	char *temp = NULL;
	max98090_log_efunc("filp[%p] buffer[%p] count[%ld] data[%p]",
			filp, buffer, count, data);
	temp = kmalloc(count, GFP_KERNEL);
	memset(temp, 0, count);
	strncpy(temp, buffer, count);
	temp[count-1] = '\0';
	r = kstrtoint(temp, 0, &in);
	kfree(temp);
	if (r)
		return r;
	max98090_dump_reg = (u_int)in & MAX98090_DUMP_REG_MAX;
	max98090_dump_registers(max98090_dump_reg);
	max98090_log_rfunc("count[%ld]", count);
	return count;
}

static int max98090_proc_config_write(struct file *filp, const char *buffer,
					unsigned long count, void *data)
{
	char *temp = NULL;
	max98090_log_efunc("filp[%p] buffer[%p] count[%ld] data[%p]",
				filp, buffer, count, data);
	temp = kmalloc(count, GFP_KERNEL);
	memset(temp, 0, count);
	strncpy(temp, buffer, count);
	temp[count-1] = '\0';
	mutex_lock(&max98090_mutex);
	max98090_read_config(temp, max98090_configration_path);
	mutex_unlock(&max98090_mutex);
	kfree(temp);

	max98090_log_rfunc("count[%ld]", count);
	return count;
}

static int max98090_proc_path_read(char *page, char **start, off_t offset,
			int count, int *eof, void *data)
{
	int len = 0;
	max98090_log_efunc("");
	len = sprintf(page, "%s\n", max98090_configration_path);
	max98090_log_info("configration path[%s]", max98090_configration_path);
	*eof = 1;
	max98090_log_rfunc("len[%d]", len);
	return len;
}

static int max98090_proc_path_write(struct file *filp, const char *buffer,
	unsigned long count, void *data)
{
	char *temp = NULL;
	max98090_log_efunc("filp[%p] buffer[%p] count[%ld] data[%p]",
				filp, buffer, count, data);
	temp = kmalloc(count, GFP_KERNEL);
	memset(temp, 0, count);
	strncpy(temp, buffer, count);
	temp[count-1] = '\0';
	strcpy(max98090_configration_path, temp);
	kfree(temp);

	max98090_log_info("configration path[%s]", max98090_configration_path);
	max98090_log_rfunc("count[%ld]", count);
	return count;
}

#ifdef __MAX98090_RELEASE_CHECK__
static int max98090_proc_release_write(struct file *filp, const char *buffer,
	unsigned long count, void *data)
{
	int r = 0;
	int in = 0;
	char *temp = NULL;
	max98090_log_efunc("filp[%p] buffer[%p] count[%ld] data[%p]",
			filp, buffer, count, data);
	temp = kmalloc(count, GFP_KERNEL);
	memset(temp, 0, count);
	strncpy(temp, buffer, count);
	temp[count-1] = '\0';
	r = kstrtoint(temp, 0, &in);
	kfree(temp);

	if (r)
		return r;

	if (in == 20)
		max98090_set_mute(0);
	else if (in == 21)
		max98090_set_mute(1);
	else
		max98090_set_device(in, MAX98090_DISABLE_CONFIG_SUB);
	max98090_log_rfunc("count[%ld]", count);
	return count;
}
#endif	/* __MAX98090_RELEASE_CHECK__ */

#ifdef __SOC_CODEC_ADD__
static int max98090_probe(struct snd_soc_codec *codec)
{
	int ret = 0;
	struct max98090_priv *max98090 = snd_soc_codec_get_drvdata(codec);
	max98090->codec = codec;
	ret = snd_soc_codec_set_cache_io(codec, 8, 8, SND_SOC_I2C);

	if (ret)
		dev_err(codec->dev, "Failed to set chache I/O: %d\n", ret);

	ret = snd_soc_add_controls(codec, max98090_controls,
				max98090_array_size);

	if (ret)
		dev_err(codec->dev, "Failed to create control: %d\n", ret);

	return ret;
}

static int max98090_remove(struct snd_soc_codec *codec)
{
	return 0;
}

struct snd_soc_codec_driver soc_codec_dev_max98090 = {
	.probe =    max98090_probe,
	.remove =    max98090_remove,
	.reg_cache_size = 0,
	.reg_word_size = sizeof(u8),
	.reg_cache_default = NULL,
};

static struct snd_soc_dai_driver max98090_dai_driver[] = {
	{
		.name = "max98090-hifi",
		.playback = {
			.stream_name = "Playback",
			.channels_min = 1,
			.channels_max = 2,
			.rates = SNDRV_PCM_RATE_8000_48000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE },
		.capture = {
			.stream_name = "Capture",
			.channels_min = 2,
			.channels_max = 2,
			.rates = SNDRV_PCM_RATE_8000_48000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE },
		.ops = NULL,
	},
	{
		.name = "max98090-fm",
		.playback = {
			.stream_name = "Playback",
			.channels_min = 1,
			.channels_max = 1,
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE },
		.capture = {
			.stream_name = "Capture",
			.channels_min = 1,
			.channels_max = 1,
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE },
		.ops = NULL,
		.symmetric_rates = 1,
	},
};
EXPORT_SYMBOL(max98090_dai_driver);

static void max98090_set_soc_controls(struct snd_kcontrol_new *controls,
				u_int array_size)
{
	max98090_controls = controls;
	max98090_array_size = array_size;
}
EXPORT_SYMBOL(max98090_set_soc_controls);

#endif  /* __SOC_CODEC_ADD__ */

#ifdef __SOC_CODEC_ADD__
static int max98090_i2c_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
#else
static int max98090_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
#endif  /* __SOC_CODEC_ADD__ */
{
	int ret = 0;
	struct max98090_priv *dev = NULL;

	max98090_log_efunc("client[0x%p] id->driver_data[%ld]",
			client, id->driver_data);

	if (NULL == max98090_conf) {
		dev = kzalloc(sizeof(struct max98090_priv), GFP_KERNEL);

		if (NULL == dev) {
			ret = -ENOMEM;
			max98090_log_err("Could not allocate master. ret[%d]",
					ret);
			return ret;
		}
		strcpy(max98090_configration_path, MAX98090_CONFIG_INIT);

		max98090_conf = dev;
	} else {
		dev = max98090_conf;
	}

	max98090_log_info("client->addr[0x%02X]", client->addr);

	switch (client->addr) {
	case MAX98090_SLAVE_ADDR:
		dev->client_max98090 = client;
		break;
	case MAX97236_SLAVE_ADDR:
		dev->client_max97236 = client;
		break;
	default:
		ret = -EINVAL;
		max98090_log_err(
			"invalid I2C address specified.ret[%d]addr[%02X]",
			ret, client->addr);
		goto err_slave_addr;
	}

	i2c_set_clientdata(client, dev);

#ifdef __SOC_CODEC_ADD__

	if (MAX98090_SLAVE_ADDR == client->addr) {
		ret = snd_soc_register_codec(&client->dev,
					     &soc_codec_dev_max98090,
					     max98090_dai_driver,
					     ARRAY_SIZE(max98090_dai_driver));

		if (ret) {
			dev_err(&client->dev,
				"Failed to register codec: %d\n", ret);
		}
	}

#endif  /* __SOC_CODEC_ADD__ */

	if (MAX97236_SLAVE_ADDR == client->addr) {
		ret = max98090_setup(client, dev);

		if (0 != ret)
			goto err_setup;
	}

	max98090_log_rfunc("ret[%d]", ret);
	return ret;

err_setup:
err_slave_addr:

	if (NULL != dev->client_max98090)
		kfree(i2c_get_clientdata(dev->client_max98090));

	if (NULL != dev->client_max97236)
		kfree(i2c_get_clientdata(dev->client_max97236));

	kfree(dev);
	max98090_log_err("ret[%d]", ret);
	return ret;
}

#ifdef __SOC_CODEC_ADD__
static int max98090_i2c_remove(struct i2c_client *client)
#else
static int max98090_remove(struct i2c_client *client)
#endif  /* __SOC_CODEC_ADD__ */
{
	int ret = 0;

	max98090_log_efunc("");

	if (MAX98090_SLAVE_ADDR == client->addr) {
		ret = max98090_disable_interrupt();

		if (0 != ret)
			max98090_log_err("ret[%d]", ret);
	}

#ifdef __SOC_CODEC_ADD__
	snd_soc_unregister_codec(&client->dev);
#endif  /* __SOC_CODEC_ADD__ */

	kfree(i2c_get_clientdata(client));

	if (max98090_conf->log_entry)
		remove_proc_entry(MAX98090_LOG_LEVEL,
				max98090_conf->proc_parent);

	if (max98090_conf->dump_entry)
		remove_proc_entry(MAX98090_DUMP_REG,
				max98090_conf->proc_parent);

	if (max98090_conf->config_entry)
		remove_proc_entry(MAX98090_TUNE_REG,
				max98090_conf->proc_parent);

	if (max98090_conf->path_entry)
		remove_proc_entry(MAX98090_PATH_SET,
				max98090_conf->proc_parent);

#ifdef __MAX98090_RELEASE_CHECK__
	if (max98090_conf->release_entry)
		remove_proc_entry("release_check",
				max98090_conf->proc_parent);
#endif	/* __MAX98090_RELEASE_CHECK__ */

	if (max98090_conf->proc_parent)
		remove_proc_entry(AUDIO_LSI_DRV_NAME, NULL);

	if (MAX97236_SLAVE_ADDR == client->addr) {
		input_unregister_device(max98090_conf->input_dev);
		switch_dev_unregister(&max98090_conf->switch_data.sdev);
		max98090_disable_vclk4();
		gpio_free(GPIO_PORT29);
		kfree(max98090_conf);
	}

	max98090_log_rfunc("");
	return 0;
}

int __init max98090_init(void)
{
	int ret = 0;
	max98090_log_efunc("");

	ret = i2c_add_driver(&max98090_i2c_driver);
	max98090_log_rfunc("ret[%d]", ret);
	return ret;
}

void __exit max98090_exit(void)
{
	max98090_log_efunc("");
	i2c_del_driver(&max98090_i2c_driver);
	max98090_log_rfunc("");
}

#ifdef __MAX98090_UT__
#include "max98090_ut_priv_func.h"
#include "max98090_ut_priv_func.cc"
#endif	/* __MAX98090_UT__ */


module_init(max98090_init);
module_exit(max98090_exit);

MODULE_LICENSE("GPL v2");
