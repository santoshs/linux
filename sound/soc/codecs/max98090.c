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
#ifdef __SOC_CODEC_ADD__
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#else   /* __SOC_CODEC_ADD__ */
/* none */
#endif  /* __SOC_CODEC_ADD__ */

#include <mach/r8a73734.h>
#endif  /* __MAX98090_UT__ */

#include <sound/soundpath/max98090_extern.h>

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

#ifdef __MAX98090_TODO_POWER__
u_long max98090_sysc_Base;  /* SYSC base address */

#define SYSC_PHY_BASE   (0xE6180000)
#define SYSC_REG_MAX    (0x0084)

#define SYSC_SPDCR      (max98090_sysc_Base + 0x0008)
#define SYSC_SWUCR      (max98090_sysc_Base + 0x0014)
#define SYSC_PSTR       (max98090_sysc_Base + 0x0080)
#endif  /* __MAX98090_TODO_POWER__ */

#define MAX98090_GPIO_BASE	IO_ADDRESS(0xE6050000)
#define MAX98090_GPIO_034	(MAX98090_GPIO_BASE + 0x0022)	/* AUDIO_IRQ */

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
#ifdef __MAX98090_RELEASE_CHECK__
	struct proc_dir_entry *release_entry;/**< release check entry. */
#endif	/* __MAX98090_RELEASE_CHECK__ */
	struct proc_dir_entry *log_parent;  /**< log level parent. */
#ifdef __SOC_CODEC_ADD__
	struct snd_soc_codec *codec;
#else
	/* none */
#endif  /* __SOC_CODEC_ADD__ */
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
#ifdef __MAX98090_DEBUG__
static void max98090_dump_max98090_priv(void);
#endif /* __MAX98090_DEBUG__ */

static int max98090_proc_read(char *page, char **start, off_t offset,
			int count, int *eof, void *data);
static int max98090_proc_write(struct file *filp, const char *buffer,
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
#else   /* __MAX98090_DEBUG__ */
static u_int max98090_log_level = MAX98090_LOG_NO_PRINT;
#endif  /* __MAX98090_DEBUG__ */

/*!
  @brief Store the AudioLSI driver config.
*/
static struct max98090_priv *max98090_conf;

/*!
  @brief max98090 volume table.
*/
static const u_int
MAX98090_volume[MAX98090_VOL_DEV_MAX][MAX98090_VOLUMEL_MAX] = {
	{/* MAX98090_VOL_DEV_SW */
		MAX98090_VOLUMEL0,
		MAX98090_VOLUMEL1,
		MAX98090_VOLUMEL2,
		MAX98090_VOLUMEL3,
		MAX98090_VOLUMEL4,
		MAX98090_VOLUMEL5
	},
	{/* MAX98090_VOL_DEV_SPEAKER */
		0x18, /* -48dB */
		0x1B, /* -36dB */
		0x1F, /* -23dB */
		0x24, /* -10dB */
		0x2E, /*   2dB */
		0x3F  /*  14db */
	},
	{/* MAX98090_VOL_DEV_EARPIECE */
		0x00, /* -62dB */
		0x03, /* -50dB */
		0x07, /* -35dB */
		0x0C, /* -20dB */
		0x12, /* - 6dB */
		0x1F  /*   8dB */
	},
	{/* MAX98090_VOL_DEV_HEADPHONES */
		0x00, /* -67dB */
		0x03, /* -55dB */
		0x07, /* -40db */
		0x0C, /* -25dB */
		0x12, /* -11dB */
		0x1F  /*   3dB */
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

	ret = clk_set_rate(vclk4_clk, 26000000);

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
	dev->log_parent = proc_mkdir(AUDIO_LSI_DRV_NAME, NULL);

	if (NULL != dev->log_parent) {
		dev->log_entry = create_proc_entry(MAX98090_LOG_LEVEL,
						(S_IRUGO | S_IWUGO),
						dev->log_parent);

		if (NULL != dev->log_entry) {
			dev->log_entry->read_proc  = max98090_proc_read;
			dev->log_entry->write_proc = max98090_proc_write;
		} else {
			max98090_log_err("create failed for proc log_entry.");
			ret = -ENOMEM;
			goto err_proc;
		}
	}

#ifdef __MAX98090_RELEASE_CHECK__
	if (NULL != dev->log_parent) {
		dev->release_entry = create_proc_entry("release_check",
						(S_IRUGO | S_IWUGO),
						dev->log_parent);

		if (NULL != dev->release_entry) {
			dev->release_entry->write_proc =
						max98090_proc_release_write;
		} else {
			max98090_log_err("create failed for proc entry.");
			ret = -ENOMEM;
			goto err_proc;
		}
	}
#endif	/* __MAX98090_RELEASE_CHECK__ */

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
err_create_singlethread_workqueue:

	if (dev->irq_workqueue)
		destroy_workqueue(dev->irq_workqueue);

err_proc:

	if (dev->log_entry)
		remove_proc_entry(MAX98090_LOG_LEVEL, dev->log_parent);

#ifdef __MAX98090_RELEASE_CHECK__
	if (dev->release_entry)
		remove_proc_entry("release_check", dev->log_parent);
#endif	/* __MAX98090_RELEASE_CHECK__ */

	if (dev->log_parent)
		remove_proc_entry(AUDIO_LSI_DRV_NAME, NULL);

	max98090_log_err("ret[%d]", ret);
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
	/* Register 0x04 (System Clock Quick Setup) =
	   0x80(Setup Device for Operation with a 26MHz Master Clock (MCLK)) */
	ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
			MAX98090_REG_W_SYSTEM_CLOCK, 0x80);

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

		/* Register 0x3D (JACK DETECT) = 0x80:JDETEN */
		ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
				MAX98090_REG_RW_JACK_DETECT, 0x82);

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
		/* todo notify key event imple */
		max98090_conf->switch_data.key_press = key_press;
	}

	/* do callback */
	if (NULL != max98090_conf->callback)
		max98090_conf->callback->irq_func();

	res = max98090_enable_interrupt();
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
		max98090_log_debug("max98090_conf->log_parent[0x%p]",
				max98090_conf->log_parent);
		max98090_log_debug("-------------------------------------");
	} else {
		max98090_log_debug("-------------------------------------");
		max98090_log_debug("max98090_conf is NULL");
		max98090_log_debug("-------------------------------------");
	}
}
#endif /* __MAX98090_DEBUG__ */

/*------------------------------------*/
/* for public function		*/
/*------------------------------------*/
int max98090_set_device(const u_long device)
{
	int ret = 0;
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

	/* update internal value */
	max98090_conf->info = new_device;
#ifdef __MAX98090_DEBUG__
	max98090_dump_max98090_priv();
#endif /* __MAX98090_DEBUG__ */
	max98090_log_rfunc("ret[%d]", ret);
	return ret;
err_set_device:
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

	/* convert volume value to index */
	for (i = 0; i < MAX98090_VOLUMEL_MAX; i++) {

		if (MAX98090_volume[MAX98090_VOL_DEV_SW][i] == volume) {
			vol_index = i;
			break;
		}
	}

	/* set speaker volume */
	if (MAX98090_DEV_PLAYBACK_SPEAKER & device) {
		ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
			MAX98090_REG_RW_LEFT_SPK_VOLUME,
			MAX98090_volume[MAX98090_VOL_DEV_SPEAKER][vol_index]);

		if (0 != ret)
			goto err_i2c_write;

		ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
			MAX98090_REG_RW_RIGHT_SPK_VOLUME,
			MAX98090_volume[MAX98090_VOL_DEV_SPEAKER][vol_index]);

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
			MAX98090_volume[MAX98090_VOL_DEV_EARPIECE][vol_index]);

		if (0 != ret)
			goto err_i2c_write;

		ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
			MAX98090_REG_RW_LOUTR_VOLUME,
			MAX98090_volume[MAX98090_VOL_DEV_EARPIECE][vol_index]);

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
			MAX98090_volume[MAX98090_VOL_DEV_HEADPHONES][vol_index]
			);

		if (0 != ret)
			goto err_i2c_write;

		ret = max98090_write(MAX98090_AUDIO_IC_MAX98090,
			MAX98090_REG_RW_RIGHT_HP_VOLUME,
			MAX98090_volume[MAX98090_VOL_DEV_HEADPHONES][vol_index]
			);

		if (0 != ret)
			goto err_i2c_write;
		else
			max98090_conf->info.volume[MAX98090_VOL_DEV_HEADPHONES]
			= volume;
	}

	max98090_log_rfunc("ret[%d]", ret);
	return ret;
err_i2c_write:
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
	}

	max98090_log_rfunc("ret[%d]", ret);
	return ret;
err_i2c_read:
err_i2c_write:
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
#ifdef __MAX98090_TODO_POWER__
	int reg = 0;
	int i = 0;
#endif  /* __MAX98090_TODO_POWER__ */
	max98090_log_debug("");
#ifdef __MAX98090_TODO_POWER__
	/* PSTR */
	reg = ioread32(SYSC_PSTR);

	if (!(reg & (1 << 17))) {
		/* WakeUp */
		iowrite32((1 << 17), SYSC_SWUCR);

		for (i = 0; i < 500; i++) {
			reg = ioread32(SYSC_SWUCR);
			reg &= (1 << 17);

			if (!reg)
				break;
		}

		if (500 == i)
			printk(KERN_WARNING
				"iowrite32((1<<17),SYSC_SWUCR)Wake up error\n"
				);
	}

#endif  /* __MAX98090_TODO_POWER__ */

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
#ifdef __MAX98090_TODO_POWER__
	int reg = 0;
	int i = 0;
#endif  /* __MAX98090_TODO_POWER__ */
	max98090_log_debug("");
#ifdef __MAX98090_TODO_POWER__
	/* PSTR */
	reg = ioread32(SYSC_PSTR);

	if (!(reg & (1 << 17))) {
		/* WakeUp */
		iowrite32((1 << 17), SYSC_SWUCR);

		for (i = 0; i < 500; i++) {
			reg = ioread32(SYSC_SWUCR);
			reg &= (1 << 17);

			if (!reg)
				break;
		}

		if (500 == i)
			printk(KERN_WARNING
				"iowrite32((1<<17),SYSC_SWUCR)Wake up error\n"
				);
	}
#endif  /* __MAX98090_TODO_POWER__ */

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
static int max98090_proc_read(char *page, char **start, off_t offset,
			int count, int *eof, void *data)
{
	int len = 0;
	max98090_log_efunc("");
	len = sprintf(page, "0x%08x\n", (int)max98090_log_level);
	*eof = 1;
	max98090_log_rfunc("len[%d]", len);
	return len;
}

static int max98090_proc_write(struct file *filp, const char *buffer,
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
		max98090_set_device(in);

	max98090_log_rfunc("count[%ld]", count);
	return count;
}
#endif	/* __MAX98090_RELEASE_CHECK__ */

#ifdef __SOC_CODEC_ADD__
static int max98090_probe(struct snd_soc_codec *codec)
{
	int ret;
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
			.channels_min = 2,
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

		max98090_conf = dev;
	} else {
		dev = max98090_conf;
	}

#ifdef __MAX98090_TODO_POWER__
	/* Get SYSC Logical Address */
	max98090_sysc_Base = (u_long)ioremap_nocache(SYSC_PHY_BASE,
						SYSC_REG_MAX);

	if (0 >= max98090_sysc_Base) {
		printk(KERN_WARNING
			"max98090_probe() SYSC ioremap failed error\n");
		return 0;
	}
#endif /* __MAX98090_TODO_POWER__ */

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
				max98090_conf->log_parent);
#ifdef __MAX98090_RELEASE_CHECK__
	if (max98090_conf->release_entry)
		remove_proc_entry("release_check",
				max98090_conf->log_parent);
#endif	/* __MAX98090_RELEASE_CHECK__ */

	if (max98090_conf->log_parent)
		remove_proc_entry(AUDIO_LSI_DRV_NAME, NULL);

	if (MAX97236_SLAVE_ADDR == client->addr) {
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
