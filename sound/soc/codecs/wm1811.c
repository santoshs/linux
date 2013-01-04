/* wm1811.c
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
  @file wm1811.c

  @brief Audio LSI driver source file.
*/


/*---------------------------------------------------------------------------*/
/* include files                                                             */
/*---------------------------------------------------------------------------*/
#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
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
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/tlv.h>
#include <mach/common.h>
#include <mach/r8a73734.h>
#include <sound/soundpath/wm1811_extern.h>
#include <sound/soundpath/soundpath.h>

#include "wm1811.h"
#include "wm1811_log.h"

/*---------------------------------------------------------------------------*/
/* typedef declaration (private)                                             */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* define macro declaration (private)                                        */
/*---------------------------------------------------------------------------*/
#define WM1811_SLAVE_ADDR	0x1a /* WM1811 i2c slave address */

#define WM1811_ENABLE		1    /* enable value */
#define WM1811_DISABLE		0    /* disable value */

#define WM1811_BIAS_VMID_ENABLE		0x0003
#define WM1811_ADCL_ENABLE		0x0202
#define WM1811_ADCR_ENABLE		0x0101
#define WM1811_ADCLR_ENABLE		(WM1811_ADCL_ENABLE | \
					 WM1811_ADCR_ENABLE)

#define WM1811_SPEAKER_ENABLE		0x3000
#define WM1811_SPEAKER_VOL		0x0300

#define WM1811_EARPEACE_ENABLE		0x0800
#define WM1811_EARPEACE_VOL		0x00C0

#define WM1811_MIXINL_ENA		0x0200
#define WM1811_MIXINR_ENA		0x0100
#define WM1811_MIXINLR_ENA		(WM1811_MIXINL_ENA | \
					 WM1811_MIXINR_ENA)

#define WM1811_IN1L_ENA			0x0040
#define WM1811_IN2R_ENA			0x0020

#define WM1811_MIXOUTR_ENA		0x0010
#define WM1811_MIXOUTRVOL_ENA		0x0040
#define WM1811_MIXOUTRVOL_TO_HPOUT2	0x0010
#define WM1811_AIF1DAC1_MONO		0x0080

#define WM1811_HEADPHONE_ENABLE		0x0300
#define WM1811_HEADPHONE_VOL		0x0030

#define WM1811_MIC_ENABLE		0x0010
#define WM1811_MIXINL_MIC_ENA		0x0030
#define WM1811_MIXINR_MIC_ENA		0x0180

#define WM1811_HEADSET_MIC_ENABLE	0x0020
#define WM1811_MIXINR_HEADSET_MIC_ENA	0x0030

#define WM1811_TSHUT_ENA		0x4000

#define WM1811_DISABLE_CONFIG		(0xff000004)
#define WM1811_DISABLE_CONFIG_SUB	(0x00000000)

#define WM1811_MICD_INIT	0x0
#define	WM1811_MICD_STS		0x1
#define	WM1811_MICD_VALID	0x2
#define	WM1811_MICD_LVL0	0x4
#define	WM1811_MICD_LVL1	0x8
#define	WM1811_MICD_LVL2	0x10
#define	WM1811_MICD_LVL3	0x20
#define	WM1811_MICD_LVL4	0x40
#define	WM1811_MICD_LVL5	0x80
#define	WM1811_MICD_LVL6	0x100
#define	WM1811_MICD_LVL7	0x200
#define	WM1811_MICD_LVL8	0x400
#define WM1811_JACK_IN		0x402

#define WM1811_PLAYBACK_VOL_RANGE	0x003F
#define WM1811_CAPTURE_VOL_RANGE	0x001F

#define WM1811_NO_JACK		0x0
#define WM1811_HEADSET		0x1
#define WM1811_HEADPHONE	0x2

#define WM1811_IRQ_WAKE_OFF	0x0
#define WM1811_IRQ_WAKE_ON	0x1


/*---------------------------------------------------------------------------*/
/* define function macro declaration (private)                               */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* enum declaration (private)                                                */
/*---------------------------------------------------------------------------*/
/*!
  @brief volume register id.
*/
enum {
	/* Playback */
	E_VOL_SPEAKER_L = 0,	/**< Speaker Left. */
	E_VOL_SPEAKER_R,	/**< Speaker Right. */
	E_VOL_EARPIECE_L,	/**< Earpiece Left. */
	E_VOL_EARPIECE_R,	/**< Earpiece Right. */
	E_VOL_HEADPHONE_L,	/**< Headphone Left. */
	E_VOL_HEADPHONE_R,	/**< Headphone Right. */
	/* Capture */
	E_VOL_MAIN_MIC,		/**< Main Mic. */
	E_VOL_SUB_MIC,		/**< Sub Mic. */
	E_VOL_HEADSET_MIC,	/**< Sub Mic. */
	E_VOL_MAX
};


/*---------------------------------------------------------------------------*/
/* structure declaration (private)                                           */
/*---------------------------------------------------------------------------*/
/*!
  @brief AudioLSI driver internal device info.
*/
struct wm1811_info {
	u_long raw_device;  /**< device ID. */
	u_int speaker;      /**< playback speaker. */
	u_int earpiece;     /**< playback earpiece. */
	u_int headphone;    /**< playback headphones or headset.*/
	u_int mic;          /**< capture mic. */
	u_int headset_mic;  /**< capture headset mic. */
};

/*!
  @brief jack notify.
*/
struct wm1811_switch_data {
	struct switch_dev sdev; /**< switch dev. */
	int state;              /**< switch state. */
	int key_press;          /**< hook button pressed/released. */
};

/*!
  @brief AudioLSI driver configuration structure.
*/
struct wm1811_priv {
	/* info */
	struct wm1811_info info;            /**< user setting info. */
	struct input_dev *input_dev;        /**< input device. */
	u_short volume[E_VOL_MAX];          /**< volume. */
	int volume_saved[E_VOL_MAX];        /**< volume save flag. */
	/* i2c */
	struct i2c_client *client_wm1811;   /**< i2c driver wm1811 client. */
	/* irq */
	u_int irq;                          /**< irq number. */
	u_int irq_wake_state;                   /**< irq_set_irq_wake OnOff flg. */
	struct workqueue_struct *irq_workqueue;
					    /**< irq workqueue. */
	struct work_struct irq_work;	    /**< irq work. */
	struct wm1811_switch_data switch_data;
					    /**< switch data. */
	/* proc */
	struct proc_dir_entry *log_entry;   /**< log level entry. */
	struct proc_dir_entry *dump_entry;  /**< dump entry. */
	struct proc_dir_entry *proc_parent; /**< proc parent. */
	struct snd_soc_codec *codec;
	/* clock */
	struct clk *main_clk;
	struct clk *vclk4_clk;
};

/*---------------------------------------------------------------------------*/
/* prototype declaration (private)                                           */
/*---------------------------------------------------------------------------*/
static int wm1811_enable_vclk4(void);
static int wm1811_disable_vclk4(void);
static int wm1811_setup(struct i2c_client *client,
			struct wm1811_priv *dev);
static int wm1811_setup_r8a73734(void);
static int wm1811_setup_wm1811(void);

static int wm1811_setup_proc(struct wm1811_priv *dev);
static int wm1811_switch_dev_register(struct wm1811_priv *dev);
static int wm1811_hooksw_dev_register(struct i2c_client *client,
		struct wm1811_priv *dev);
static int wm1811_create_proc_entry(char *name,
					struct proc_dir_entry **proc_child);

static void wm1811_irq_work_func(struct work_struct *unused);
static irqreturn_t wm1811_irq_handler(int irq, void *data);

static int wm1811_conv_device_info(const u_long device,
				struct wm1811_info *device_info);

static int wm1811_check_device(const u_long device);

static int wm1811_set_mic_adc_pm(const u_int cur_dev_mic,
				const u_int new_dev_mic,
				const u_int cur_dev_headsetmic,
				const u_int new_dev_headsetmic,
				const u_int pcm_mode);

static int wm1811_set_mic_mixer_pm(const u_int cur_dev_mic,
				const u_int new_dev_mic,
				const u_int cur_dev_headsetmic,
				const u_int new_dev_headsetmic,
				const u_int pcm_mode);

static int wm1811_set_speaker_device(const u_int cur_dev,
				const u_int new_dev);
static int wm1811_set_earpiece_device(const u_int cur_dev,
				const u_int new_dev);
static int wm1811_set_headphone_device(const u_int cur_dev,
				const u_int new_dev);
static int wm1811_set_main_mic_device(const u_int cur_dev,
				const u_int new_dev);
static int wm1811_set_sub_mic_device(const u_int cur_dev,
				const u_int new_dev);
static int wm1811_set_mic_device(const u_int cur_dev, const u_int new_dev,
				const u_int pcm_mode);
static int wm1811_set_headset_mic_device(const u_int cur_dev,
				const u_int new_dev);

#ifdef __WM1811_DEBUG__
static void wm1811_dump_wm1811_priv(void);
#endif /* __WM1811_DEBUG__ */

static int wm1811_i2c_read_device(struct i2c_client *wm1811,
				u_short reg,
				int bytes,
				void *dest);
static int wm1811_i2c_write_device(struct i2c_client *wm1811,
				u_short reg,
				int bytes /* not use */,
				u_short *src);

static int wm1811_playback_volume_set(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol);
static int wm1811_playback_volume_get(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol);
static int wm1811_capture_volume_set(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol);
static int wm1811_capture_volume_get(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol);

static int wm1811_store_volume(const u_short addr, const u_short value);
static int wm1811_restore_volume(const u_long device);

static int wm1811_resume(const u_long device, u_int pcm_mode);
static int wm1811_suspend(u_int pcm_mode);
static int wm1811_set_irq_wake(const u_int set_wake_state, u_int pcm_mode);

static int wm1811_proc_log_read(char *page, char **start, off_t offset,
			int count, int *eof, void *data);
static int wm1811_proc_log_write(struct file *filp, const char *buffer,
			u_long count, void *data);
static int wm1811_proc_dump_read(char *page, char **start, off_t offset,
			int count, int *eof, void *data);
static int wm1811_proc_dump_write(struct file *filp, const char *buffer,
			u_long count, void *data);

static int wm1811_probe(struct snd_soc_codec *codec);
static int wm1811_remove(struct snd_soc_codec *codec);
static int wm1811_i2c_probe(struct i2c_client *i2c,
			const struct i2c_device_id *id);
static int wm1811_i2c_remove(struct i2c_client *i2c);

/*---------------------------------------------------------------------------*/
/* global variable declaration                                               */
/*---------------------------------------------------------------------------*/
/*!
  @brief AudioLSI driver log level.
*/
#ifdef __WM1811_DEBUG__
static u_int wm1811_log_level = (WM1811_LOG_ERR_PRINT |
				   WM1811_LOG_PROC_PRINT |
				   WM1811_LOG_FUNC_PRINT
				   | WM1811_LOG_DEBUG_PRINT
				  );

#else   /* __WM1811_DEBUG__ */
static u_int wm1811_log_level = WM1811_LOG_NO_PRINT;
#endif  /* __WM1811_DEBUG__ */

/*!
  @brief Store the AudioLSI driver config.
*/
static struct wm1811_priv *wm1811_conf;
static struct wake_lock wm1811_hp_irq_wake_lock;
DEFINE_MUTEX(wm1811_mutex);

/*!
  @brief i2c driver data.
*/
static const struct i2c_device_id wm1811_i2c_id[] = {
	{ "wm1811", 0       },
	{ /* end of list */ },
};
MODULE_DEVICE_TABLE(i2c, wm1811_i2c_id);

/*!
  @brief represent an I2C device driver.
*/
static struct i2c_driver wm1811_i2c_driver = {
	.driver = {
		.name = "wm1811",
	},
	.probe  = wm1811_i2c_probe,
	.remove = wm1811_i2c_remove,
	.id_table = wm1811_i2c_id,
};

static struct snd_kcontrol_new *wm1811_controls;
static u_int wm1811_array_size;

static const DECLARE_TLV_DB_SCALE(playback_tlv, -5700, 100, 0);
static const DECLARE_TLV_DB_SCALE(capture_tlv, -1650, 150, 0);

static const struct snd_kcontrol_new wm1811_volume_controls[] = {
	/* Playback volume controls */
	SOC_DOUBLE_R_EXT_TLV("Playback Speaker Volume",
		0x0026, 0x0027, 0, WM1811_PLAYBACK_VOL_RANGE, 0,
		wm1811_playback_volume_get, wm1811_playback_volume_set,
		playback_tlv),
	SOC_DOUBLE_R_EXT_TLV("Playback Earpiece Volume",
		0x0020, 0x0021, 0, WM1811_PLAYBACK_VOL_RANGE, 0,
		wm1811_playback_volume_get, wm1811_playback_volume_set,
		playback_tlv),
	SOC_DOUBLE_R_EXT_TLV("Playback Headphone Volume",
		0x001C, 0x001D, 0, WM1811_PLAYBACK_VOL_RANGE, 0,
		wm1811_playback_volume_get, wm1811_playback_volume_set,
		playback_tlv),
	/* Capture volume controls */
	SOC_SINGLE_EXT_TLV("Capture Headset Mic Volume",
		0x001A, 0, WM1811_CAPTURE_VOL_RANGE, 0,
		wm1811_capture_volume_get, wm1811_capture_volume_set,
		capture_tlv),
	SOC_SINGLE_EXT_TLV("Capture Main Mic Volume",
		0x0018, 0, WM1811_CAPTURE_VOL_RANGE, 0,
		wm1811_capture_volume_get, wm1811_capture_volume_set,
		capture_tlv),
	SOC_SINGLE_EXT_TLV("Capture Sub Mic Volume",
		0x001B, 0, WM1811_CAPTURE_VOL_RANGE, 0,
		wm1811_capture_volume_get, wm1811_capture_volume_set,
		capture_tlv),
};

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
static int wm1811_enable_vclk4(void)
{
	int ret = 0;
	wm1811_log_efunc("");

	ret = gpio_request(GPIO_FN_VIO_CKO4, NULL);

	if (0 != ret) {
		wm1811_log_err("gpio_request() ret[%d]", ret);
		goto err_gpio_request;
	}

	wm1811_conf->vclk4_clk = clk_get(NULL, "vclk4_clk");

	if (IS_ERR(wm1811_conf->vclk4_clk)) {
		ret = IS_ERR(wm1811_conf->vclk4_clk);
		wm1811_log_err("clk_get(vclk4_clk) ret[%d]", ret);
		goto err_gpio_request;
	}

	wm1811_conf->main_clk = clk_get(NULL, "main_clk");

	if (IS_ERR(wm1811_conf->main_clk)) {
		ret = IS_ERR(wm1811_conf->main_clk);
		wm1811_log_err("clk_get(main_clk) ret[%d]", ret);
		goto err_gpio_request;
	}

	/* vclk4_clk */
	ret = clk_set_parent(wm1811_conf->vclk4_clk, wm1811_conf->main_clk);

	if (0 != ret) {
		wm1811_log_err("clk_set_parent() ret[%d]", ret);
		goto err_gpio_request;
	}

	ret = clk_set_rate(wm1811_conf->vclk4_clk, 26000000);

	if (0 != ret) {
		wm1811_log_err("clk_set_rate() ret[%d]", ret);
		goto err_gpio_request;
	}

	ret = clk_enable(wm1811_conf->vclk4_clk);

	if (0 != ret) {
		wm1811_log_err("clk_enable() ret[%d]", ret);
		goto err_gpio_request;
	}

	wm1811_log_rfunc("ret[%d]", ret);
	return ret;

err_gpio_request:
	wm1811_log_err("ret[%d]", ret);
	return ret;
}

static int wm1811_disable_vclk4(void)
{
	clk_disable(wm1811_conf->vclk4_clk);
	clk_put(wm1811_conf->vclk4_clk);

	return 0;
}

/*!
  @brief setup wm1811.

  @param[i] client  i2c client.
  @param[i] dev  AudioLSI driver config.

  @return function results.
*/
static int wm1811_setup(struct i2c_client *client,
			struct wm1811_priv *dev)
{
	int ret = 0;
	wm1811_log_efunc("");

	/***********************************/
	/* setup proc                      */
	/***********************************/
	ret = wm1811_setup_proc(dev);

	if (0 != ret)
		goto err_setup_proc;

	ret = wm1811_switch_dev_register(dev);

	if (0 != ret)
		goto err_wm1811_switch_dev_register;

	ret = wm1811_hooksw_dev_register(client, dev);

	if (0 != ret)
		goto err_wm1811_hooksw_dev_register;

	/***********************************/
	/* setup r8a73734                  */
	/***********************************/
	ret = wm1811_setup_r8a73734();

	if (0 != ret)
		goto err_setup_r8a73734;

	/* create workqueue for irq */
	if (NULL == dev->irq_workqueue) {
		dev->irq_workqueue =
			create_singlethread_workqueue("jack_detect");

		if (NULL == dev->irq_workqueue) {
			ret = -ENOMEM;
			wm1811_log_err("queue create error. ret[%d]", ret);
			goto err_create_singlethread_workqueue;
		}

		INIT_WORK(&dev->irq_work, wm1811_irq_work_func);
	}

	/* request irq */
	dev->irq = client->irq;
	wm1811_log_info("client->irq[%d]", client->irq);
	ret = request_irq(client->irq, &wm1811_irq_handler,
			  (IRQF_DISABLED | IRQF_TRIGGER_RISING),
			  client->name, NULL);

	if (0 != ret) {
		wm1811_log_err("irq request err. ret[%d]", ret);
		goto err_request_irq;
	}

	ret = irq_set_irq_type(client->irq, IRQ_TYPE_EDGE_RISING);

	if (0 != ret) {
		wm1811_log_err("set irq type err. ret[%d]", ret);
		goto err_irq_set_irq_type;
	}

	/***********************************/
	/* setup wm1811                    */
	/***********************************/
	ret = wm1811_setup_wm1811();

	if (0 != ret)
		goto err_setup_wm1811;

	wm1811_log_rfunc("ret[%d]", ret);
	return ret;

err_setup_wm1811:
err_irq_set_irq_type:
err_request_irq:
err_create_singlethread_workqueue:
	if (dev->irq_workqueue)
		destroy_workqueue(dev->irq_workqueue);
err_setup_r8a73734:
	input_free_device(dev->input_dev);
err_wm1811_hooksw_dev_register:
	switch_dev_unregister(&dev->switch_data.sdev);
err_wm1811_switch_dev_register:
err_setup_proc:
	wm1811_log_err("ret[%d]", ret);
	return ret;
}

/*!
  @brief setup r8a73734.

  @param none.

  @return function results.
*/
static int wm1811_setup_r8a73734(void)
{
	int ret = 0;
	wm1811_log_efunc("");

	/* CODEC_LDO_EN */
	ret = gpio_request(GPIO_PORT34, NULL);

	if (0 != ret) {
		wm1811_log_err("gpio_request(34) ret[%d]", ret);
		goto err_gpio_request;
	}

	ret = gpio_direction_output(GPIO_PORT34, 1);

	if (0 != ret) {
		wm1811_log_err("gpio_direction_output(34) ret[%d]", ret);
		goto gpio_direction_output;
	}

	if (4 == u2_get_board_rev()) {
		/* SUB_MIC_LDO_EN */
		ret = gpio_request(GPIO_PORT46, NULL);

		if (0 != ret) {
			wm1811_log_err("gpio_request(46) ret[%d]", ret);
			goto err_gpio_request;
		}

		ret = gpio_direction_output(GPIO_PORT46, 0);

		if (0 != ret) {
			wm1811_log_err("gpio_direction_output(46) ret[%d]",
					ret);
			goto gpio_direction_output;
		}
	}

	wm1811_log_rfunc("ret[%d]", ret);
	return ret;

gpio_direction_output:
err_gpio_request:
	wm1811_log_err("ret[%d]", ret);
	return ret;
}

/*!
  @brief setup wm1811.

  @param none.

  @return function results.
*/
static int wm1811_setup_wm1811(void)
{
	int ret = 0;
	wm1811_log_efunc("");

	wm1811_conf->info.raw_device = 0;

	wm1811_log_rfunc("ret[%d]", ret);
	return ret;
}

/*!
  @brief register jack switch device.

  @param none.

  @return function results.
*/
static int wm1811_switch_dev_register(struct wm1811_priv *dev)
{
	int ret = 0;
	wm1811_log_efunc("");

	dev->switch_data.sdev.name = "h2w";
	dev->switch_data.sdev.state = 0;
	dev->switch_data.state = 0;
	ret = switch_dev_register(&dev->switch_data.sdev);

	if (0 != ret)
		wm1811_log_err("ret[%d]", ret);

	wm1811_log_rfunc("ret[%d]", ret);
	return ret;
}

/*!
	@brief register hook switch device.

	@param none.

	@return function results.
*/
static int wm1811_hooksw_dev_register(struct i2c_client *client,
				struct wm1811_priv *dev)
{
	int ret = 0;
	wm1811_log_efunc("");

	dev->input_dev = input_allocate_device();
	dev->input_dev->name = "wm1811";
	dev->input_dev->id.bustype = BUS_I2C;
	dev->input_dev->dev.parent = &client->dev;

	__set_bit(EV_KEY, dev->input_dev->evbit);
	__set_bit(KEY_VOLUMEUP, dev->input_dev->keybit);
	__set_bit(KEY_VOLUMEDOWN, dev->input_dev->keybit);
	__set_bit(KEY_MEDIA, dev->input_dev->keybit);
	ret = input_register_device(dev->input_dev);

	if (0 != ret) {
		input_free_device(dev->input_dev);
		wm1811_log_err("ret[%d]", ret);
	}

	wm1811_log_rfunc("ret[%d]", ret);
	return ret;
}

/*!
  @brief setup proc.

  @param[i] dev  AudioLSI driver config.

  @return function results.
*/
static int wm1811_setup_proc(struct wm1811_priv *dev)
{
	int ret = 0;
	wm1811_log_efunc("");

	/* create file for log level entry */
	dev->proc_parent = proc_mkdir(AUDIO_LSI_DRV_NAME, NULL);

	if (NULL != dev->proc_parent) {
		ret = wm1811_create_proc_entry(WM1811_LOG_LEVEL,
						&dev->log_entry);

		if (0 != ret)
			goto err_proc;

		ret = wm1811_create_proc_entry(WM1811_DUMP_REG,
						&dev->dump_entry);

		if (0 != ret)
			goto err_proc;
	}

	wm1811_log_rfunc("ret[%d]", ret);
	return ret;

err_proc:
	if (dev->log_entry)
		remove_proc_entry(WM1811_LOG_LEVEL, dev->proc_parent);

	if (dev->dump_entry)
		remove_proc_entry(WM1811_DUMP_REG, dev->proc_parent);

	if (dev->proc_parent)
		remove_proc_entry(AUDIO_LSI_DRV_NAME, NULL);

	wm1811_log_err("ret[%d]", ret);
	return ret;
}

/*!
  @brief create proc.

  @param[i] name  proc name.
  @param[o] proc_child  proc child entry.

  @return function results.
*/
static int wm1811_create_proc_entry(char *name,
					struct proc_dir_entry **proc_child)
{
	int ret = 0;
	wm1811_log_efunc("");

	*proc_child = create_proc_entry(name,
					(S_IRUGO | S_IWUGO),
					wm1811_conf->proc_parent);

	if (NULL != *proc_child) {

		if (0 == strcmp(name, WM1811_LOG_LEVEL)) {
			(*proc_child)->read_proc = wm1811_proc_log_read;
			(*proc_child)->write_proc = wm1811_proc_log_write;

		} else if (0 == strcmp(name, WM1811_DUMP_REG)) {
			(*proc_child)->read_proc = wm1811_proc_dump_read;
			(*proc_child)->write_proc = wm1811_proc_dump_write;

		} else {
			wm1811_log_err("parameter error.");
			ret = -EINVAL;
		}
	} else {
		wm1811_log_err("create failed for %s.", name);
		ret = -ENOMEM;
	}

	wm1811_log_rfunc("ret[%d]", ret);
	return ret;
}

/*!
  @brief Implementation of interrupt handling.

  @param unused  unused.

  @return none.
*/
static void wm1811_irq_work_func(struct work_struct *unused)
{
	int state = wm1811_conf->switch_data.state;
	u_short status1 = 0;
	u_short status2 = 0;
	u_short raw_status2 = 0;
	u_short mic_detect3 = 0;
	static u_short before_button_state = 0x0;
	u_short current_button_state = 0;
	wm1811_log_efunc("");

	mutex_lock(&wm1811_mutex);

	wm1811_read(0x0730, &status1);
	wm1811_read(0x0731, &status2);
	wm1811_read(0x0732, &raw_status2);
	wm1811_read(0x00D2, &mic_detect3);

	wm1811_write(0x0730, status1);
	wm1811_write(0x0731, status2);

	wm1811_read(0x0730, &status1);
	wm1811_read(0x0731, &status2);
	wm1811_read(0x0732, &raw_status2);
	wm1811_read(0x00D2, &mic_detect3);

	wm1811_log_info("mic_detect3:[0x%x], state:[%d]", mic_detect3, state);

	if (mic_detect3 & WM1811_MICD_STS) {

		wm1811_log_info("jack insert.");

		current_button_state = mic_detect3 & 0x3FC;

		/* check mic state */
		if (state == WM1811_NO_JACK || state == WM1811_HEADPHONE) {
			switch (current_button_state) {
			case WM1811_MICD_LVL0:
				wm1811_log_info("without mic");
				state = WM1811_HEADPHONE;
				break;
			case WM1811_MICD_LVL7:
				wm1811_log_info("with mic");
				state = WM1811_HEADSET;
				break;
			}
		}

		/* check release event */
		if ((state == WM1811_HEADSET) &&
			((before_button_state & current_button_state) == 0)) {
			switch (before_button_state) {
			case WM1811_MICD_LVL0:
				wm1811_log_info("media button released");
				input_event(wm1811_conf->input_dev, EV_KEY,
					KEY_MEDIA, 0);
				break;
			case WM1811_MICD_LVL1:
				wm1811_log_info("volup button released");
				input_event(wm1811_conf->input_dev, EV_KEY,
					KEY_VOLUMEUP, 0);
				break;
			case WM1811_MICD_LVL2:
				wm1811_log_info("voldown button released");
				input_event(wm1811_conf->input_dev, EV_KEY,
					KEY_VOLUMEDOWN, 0);
				break;
			}
			input_sync(wm1811_conf->input_dev);
		}

		/* check push event */
		if (state == WM1811_HEADSET) {
			switch (current_button_state) {
			case WM1811_MICD_LVL0:
				wm1811_log_info("media  button pushed");
				input_event(wm1811_conf->input_dev, EV_KEY,
					KEY_MEDIA, 1);
				break;
			case WM1811_MICD_LVL1:
				wm1811_log_info("voldup  button pushed");
				input_event(wm1811_conf->input_dev, EV_KEY,
					KEY_VOLUMEUP, 1);
				break;
			case WM1811_MICD_LVL2:
				wm1811_log_info("voldown  button pushed");
				input_event(wm1811_conf->input_dev, EV_KEY,
					KEY_VOLUMEDOWN, 1);
				break;
			}
		}
		input_sync(wm1811_conf->input_dev);

	} else {
		wm1811_log_info("jack remove.");
		state = WM1811_NO_JACK;
	}

	if (wm1811_conf->switch_data.state != state) {
		wm1811_log_info("notify jack state[%d]", state);
		switch_set_state(&wm1811_conf->switch_data.sdev, state);
		wm1811_conf->switch_data.state = state;
	}

	before_button_state = current_button_state;
	mutex_unlock(&wm1811_mutex);

	wm1811_log_rfunc("");
}

/*!
  @brief interrupt handler.

  @param[i] irq  irq number.
  @param[i] data  irq data.

  @return irq return.
*/
static irqreturn_t wm1811_irq_handler(int irq, void *data)
{
	wm1811_log_efunc("irq[%d] data[%p]", irq, data);
	/* Release the wakelock after a delay. */
	wake_lock_timeout(&wm1811_hp_irq_wake_lock, HZ * 10);
	queue_work(wm1811_conf->irq_workqueue, &wm1811_conf->irq_work);
	wm1811_log_rfunc("");
	return IRQ_HANDLED;
}

/*!
  @brief convert device information struct.

  @param[i] device  original device information.
  @param[o] device_info  converted device information.

  @retval 0 is success.
*/
static int wm1811_conv_device_info(const u_long device,
				struct wm1811_info *device_info)
{
	int ret = 0;
	wm1811_log_efunc("device_info->raw_device[%ld] device[%ld]",
			device_info->raw_device, device);

	/* check param */
	if (NULL == device_info) {
		ret = -EINVAL;
		wm1811_log_err("parameter error. ret[%d]", ret);
		return ret;
	}

	device_info->raw_device = device;
	device_info->speaker = (WM1811_DEV_PLAYBACK_SPEAKER & device) ?
					WM1811_ENABLE : WM1811_DISABLE;
	device_info->earpiece = (WM1811_DEV_PLAYBACK_EARPIECE & device) ?
					WM1811_ENABLE : WM1811_DISABLE;
	device_info->headphone = (WM1811_DEV_PLAYBACK_HEADPHONES & device) ?
					WM1811_ENABLE : WM1811_DISABLE;
	device_info->mic = (WM1811_DEV_CAPTURE_MIC & device) ?
				WM1811_ENABLE : WM1811_DISABLE;
	device_info->headset_mic = (WM1811_DEV_CAPTURE_HEADSET_MIC & device)
					? WM1811_ENABLE : WM1811_DISABLE;
	wm1811_log_info("speaker[%d] earpiece[%d] headphone[%d]",
			device_info->speaker,
			device_info->earpiece,
			device_info->headphone);
	wm1811_log_info("mic[%d] headset_mic[%d]",
			device_info->mic,
			device_info->headset_mic);

	wm1811_log_rfunc("device_info->raw_device[%ld]",
			device_info->raw_device);
	return 0;
}

/*!
  @brief check the range of device information.

  @param[i] device  device information.

  @return function results.
*/
static int wm1811_check_device(const u_long device)
{
	int ret = 0;
	u_long dev = (WM1811_DEV_PLAYBACK_SPEAKER |
		      WM1811_DEV_PLAYBACK_EARPIECE |
		      WM1811_DEV_PLAYBACK_HEADPHONES |
		      WM1811_DEV_CAPTURE_MIC |
		      WM1811_DEV_CAPTURE_HEADSET_MIC);
	wm1811_log_efunc("device[%ld]", device);

	/* check param */
	if (~dev & device) {
		wm1811_log_err("device is out of range.");
		ret = -EINVAL;
	}

	wm1811_log_rfunc("ret[%d]", ret);
	return ret;
}

/*!
  @brief set mic adc power (management).

  @param[i] cur_dev_mic  cureent mic device state.
  @param[i] new_dev_mic  new mic device state.
  @param[i] cur_dev_headsetmic  cureent headsetmic device state.
  @param[i] new_dev_headsetmic  new headsetmic device state.
  @param[i] pcm_mode  pcm_mode.

  @return function results.
*/
static int wm1811_set_mic_adc_pm(const u_int cur_dev_mic,
				const u_int new_dev_mic,
				const u_int cur_dev_headsetmic,
				const u_int new_dev_headsetmic,
				const u_int pcm_mode)
{
	int ret = 0;
	u_short adc = 0;
	wm1811_log_efunc("cMic[%d] nMic[%d] chMic[%d] nhMic[%d] pcm[%d]",
			cur_dev_mic, new_dev_mic,
			cur_dev_headsetmic, new_dev_headsetmic,
			pcm_mode);

	if ((cur_dev_mic != new_dev_mic) ||
		(cur_dev_headsetmic != new_dev_headsetmic)) {
		ret = wm1811_read(0x0004, &adc);

		if (WM1811_ENABLE == new_dev_mic) {
			/* mic on */
			if (SNDP_MODE_INCALL == pcm_mode) {
				wm1811_log_info("ADC LR ON");
				adc |= WM1811_ADCLR_ENABLE;
			} else {
				wm1811_log_info("ADC L ON");
				adc |= WM1811_ADCL_ENABLE;
			}
		} else if (WM1811_ENABLE == new_dev_headsetmic) {
			/* mic on */
			wm1811_log_info("ADC LR ON");
			adc |= WM1811_ADCLR_ENABLE;
		} else {
			/* mic off */
			wm1811_log_info("ADC LR OFF");
			adc &= ~WM1811_ADCLR_ENABLE;
		}

		/* Enable Left ADC,Enable Right ADC */
		ret = wm1811_write(0x0004, adc);
	} else {
		/* nothing to do. */
	}

	wm1811_log_rfunc("ret[%d]", ret);
	return ret;
}

/*!
  @brief set mic mixer power (management).

  @param[i] cur_dev_mic  cureent mic device state.
  @param[i] new_dev_mic  new mic device state.
  @param[i] cur_dev_headsetmic  cureent headsetmic device state.
  @param[i] new_dev_headsetmic  new headsetmic device state.
  @param[i] pcm_mode  pcm_mode.

  @return function results.
*/
static int wm1811_set_mic_mixer_pm(const u_int cur_dev_mic,
					const u_int new_dev_mic,
					const u_int cur_dev_headsetmic,
					const u_int new_dev_headsetmic,
					const u_int pcm_mode)
{
	int ret = 0;
	u_short pm = 0;
	wm1811_log_efunc("cMic[%d] nMic[%d] chMic[%d] nhMic[%d] pcm[%d]",
			cur_dev_mic, new_dev_mic,
			cur_dev_headsetmic, new_dev_headsetmic,
			pcm_mode);

	if ((cur_dev_mic != new_dev_mic) ||
		(cur_dev_headsetmic != new_dev_headsetmic)) {
		ret = wm1811_read(0x0002, &pm);

		if (WM1811_ENABLE == new_dev_mic) {
			/* mic on */
			if (SNDP_MODE_INCALL == pcm_mode) {
				wm1811_log_info("MIXIN LR ON");
				pm |= WM1811_MIXINLR_ENA;
			} else {
				wm1811_log_info("MIXIN L ON");
				pm |= WM1811_MIXINL_ENA;
			}
		} else if (WM1811_ENABLE == new_dev_headsetmic) {
			/* mic on */
			wm1811_log_info("MIXIN LR ON");
			pm |= WM1811_MIXINLR_ENA;
		} else {
			/* mic off */
			wm1811_log_info("MIXIN LR OFF");
			pm &= ~WM1811_MIXINLR_ENA;
		}

		/* Disable Left Input Mixer, Disable Right Input Mixer */
		ret = wm1811_write(0x0002, pm);
	} else {
		/* nothing to do. */
	}

	wm1811_log_rfunc("ret[%d]", ret);
	return ret;
}

/*!
  @brief change state of speaker device.

  @param[i] cur_dev  cureent device state.
  @param[i] new_dev  new device state.

  @return function results.
*/
static int wm1811_set_speaker_device(const u_int cur_dev,
				const u_int new_dev)
{
	int ret = 0;
	u_short oe = 0;
	u_short vol = 0;
	u_short vol_p = 0;
	u_short pm = 0;
	u_short pm2 = 0;
	u_short mute_l = 0;
	u_short mute_r = 0;
	u_short speaker_mixer = 0;
	wm1811_log_efunc("cur_dev[%d] new_dev[%d]", cur_dev, new_dev);

	if (cur_dev != new_dev) {
		/* update device conf */
		ret = wm1811_read(0x0001, &pm);
		ret = wm1811_read(0x0002, &pm2);
		ret = wm1811_read(0x0003, &oe);
		mute_l = wm1811_conf->volume[E_VOL_SPEAKER_L];
		mute_r = wm1811_conf->volume[E_VOL_SPEAKER_R];
		if (WM1811_ENABLE == new_dev) {
			/* speaker on */
			vol = 0x0;
			vol_p = 0x3;
			speaker_mixer = 0x11;
			mute_l |= 0x100;
			mute_r |= 0x100;
			oe |= WM1811_SPEAKER_VOL;

			if (WM1811_BIAS_VMID_ENABLE & pm)
				pm |= WM1811_SPEAKER_ENABLE;
			else {
				pm |= (WM1811_BIAS_VMID_ENABLE |
					WM1811_SPEAKER_ENABLE);
			}

			pm2 |= WM1811_TSHUT_ENA;
		} else {
			/* speaker off */
			vol = 0x3;
			vol_p = 0x0;
			speaker_mixer = 0x0;
			ret = wm1811_write(0x0026, 0x0000);
			ret = wm1811_write(0x0027, 0x0000);
			mute_l = 0x100;
			mute_r = 0x100;
			oe &= ~WM1811_SPEAKER_VOL;

			if (~(WM1811_BIAS_VMID_ENABLE |
				WM1811_SPEAKER_ENABLE) & pm) {
				pm &= ~WM1811_SPEAKER_ENABLE;
			} else {
				pm &= ~(WM1811_BIAS_VMID_ENABLE |
					WM1811_SPEAKER_ENABLE);
			}

			pm2 &= ~WM1811_TSHUT_ENA;
		}
		/* Disable Speaker mute */
		ret = wm1811_write(0x0024, speaker_mixer);

		/* SPKOUTL setting */
		ret = wm1811_write(0x0026, mute_l);

		/* SPKOUTR setting */
		ret = wm1811_write(0x0027, mute_r);

		/* Enable SPKRVOL PGA, Enable SPKMIXR, Enable SPKLVOL PGA, */
		/* Enable SPKMIXL */
		ret = wm1811_write(0x0003, oe);

		/* Left Speaker Mixer Volume = 0dB */
		ret = wm1811_write(0x0022, vol);

		/* Speaker output mode = Class D, */
		/* Right Speaker Mixer Volume = 0dB */
		ret = wm1811_write(0x0023, vol);

		/* Unmute DAC1 (Left) to Left Speaker Mixer (SPKMIXL) path, */
		/* Unmute DAC1 (Right) to Right Speaker Mixer (SPKMIXR) path */
		ret = wm1811_write(0x0036, vol_p);

		/* Enable Themal sensor */
		ret = wm1811_write(0x0002, pm2);

		/* Enable bias generator, Enable VMID, Enable SPKOUTL, */
		/* Enable SPKOUTR */
		ret = wm1811_write(0x0001, pm);
	} else {
		/* nothing to do. */
	}

	wm1811_log_rfunc("ret[%d]", ret);
	return ret;
}

/*!
  @brief change state of earpiece device.

  @param[i] cur_dev  cureent device state.
  @param[i] new_dev  new device state.

  @return function results.
*/
static int wm1811_set_earpiece_device(const u_int cur_dev,
				const u_int new_dev)
{
	int ret = 0;
	u_short oe = 0;
	u_short pm = 0;
	u_short hp_out2_mute = 0;
	u_short mix_out_mute = 0;
	u_short mix_out_vol = 0;
	u_short hp_out2_mix = 0;
	u_short earpiece_l = 0;
	u_short earpiece_r = 0;
	u_short stereo2mono = 0;
	u_short pm5 = 0;
	wm1811_log_efunc("cur_dev[%d] new_dev[%d]", cur_dev, new_dev);

	if (cur_dev != new_dev) {
		/* update device conf */
		ret = wm1811_read(0x0001, &pm);
		ret = wm1811_read(0x0003, &oe);
		earpiece_l = wm1811_conf->volume[E_VOL_EARPIECE_L];
		earpiece_r = wm1811_conf->volume[E_VOL_EARPIECE_R];
		if (WM1811_ENABLE == new_dev) {
			/* earpiece on */
			hp_out2_mute = 0x0;
			mix_out_mute = 0x1;
			mix_out_vol = 0x10;
			hp_out2_mix = 0x40;
			earpiece_l |= 0x100;
			earpiece_r |= 0x100;
			oe |= WM1811_EARPEACE_VOL;
			oe &= ~WM1811_MIXOUTRVOL_ENA;
			stereo2mono = 0x80;

			if (WM1811_BIAS_VMID_ENABLE & pm)
				pm |= WM1811_EARPEACE_ENABLE;
			else {
				pm |= (WM1811_BIAS_VMID_ENABLE |
					WM1811_EARPEACE_ENABLE);
			}
			pm5 = 0x0203;	/* Disable AIF1DAC1R */
		} else {
			/* earpiece off */
			hp_out2_mute = 0x20;
			mix_out_mute = 0x0;
			mix_out_vol = 0x0;
			hp_out2_mix = 0x0;
			earpiece_l &= ~0x13F;
			earpiece_r &= ~0x13F;
			stereo2mono = 0x00;
			ret = wm1811_write(0x0020, earpiece_l);
			ret = wm1811_write(0x0021, earpiece_r);
			earpiece_l |= 0x100;
			earpiece_r |= 0x100;
			oe &= ~WM1811_EARPEACE_VOL;
			oe |= WM1811_MIXOUTRVOL_ENA;
			if (~(WM1811_BIAS_VMID_ENABLE |
				WM1811_EARPEACE_ENABLE) & pm) {
				pm &= ~WM1811_EARPEACE_ENABLE;
			} else {
				pm &= ~(WM1811_BIAS_VMID_ENABLE |
					WM1811_EARPEACE_ENABLE);
			}
			pm5 = 0x0303;	/* Enable AIF1DAC1R */
		}

		/***********************************/
		/* Earpiece Enable                 */
		/***********************************/
		ret = wm1811_write(0x0005, pm5);

		/* Left Earpiece vol setting */
		ret = wm1811_write(0x0020, earpiece_l);

		/* Right Earpiece vol setting */
		ret = wm1811_write(0x0021, earpiece_r);

		/* Enable Left Output Mixer Volume Control, */
		/* Enable Right Output Mixer Volume Control */
		ret = wm1811_write(0x0003, oe);

		/* Unmute HPOUT2 (Earpiece) */
		ret = wm1811_write(0x001F, hp_out2_mute);

		/* Unmute DAC1 (Left) to Left Output Mixer (MIXOUTL) path */
		ret = wm1811_write(0x002D, mix_out_mute);

		/* Unmute DAC1 (Right) to Right Output Mixer (MIXOUTR) path */
		ret = wm1811_write(0x002E, mix_out_mute);

		/* Unmute Left Output Mixer to HPOUT2 (Earpiece) path, */
		ret = wm1811_write(0x0033, mix_out_vol);

		/* Enable HPOUT2 Mixer and Input Stage */
		ret = wm1811_write(0x0038, hp_out2_mix);

		/* Activate deactivate AIF1 mono mixing */
		ret = wm1811_write(0x0420, stereo2mono);

		/* Enable bias generator, Enable VMID, */
		/* Enable HPOUT2 (Earpiece) */
		ret = wm1811_write(0x0001, pm);
	} else {
		/* nothing to do. */
	}

	wm1811_log_rfunc("ret[%d]", ret);
	return ret;
}

/*!
  @brief change state of headphone device.

  @param[i] cur_dev  cureent device state.
  @param[i] new_dev  new device state.

  @return function results.
*/
static int wm1811_set_headphone_device(const u_int cur_dev,
				const u_int new_dev)
{
	int ret = 0;
	u_short pm = 0;
	u_short cp_dyn_pwr = 0;
	u_short hpout1_dly = 0;
	u_short charge_pump = 0;
	u_short dac1_to_mixout = 0;
	u_short dcs_ena = 0;
	u_short analog_hp = 0;
	u_short mix_out_vol = 0;
	u_short hp_l_vol = 0;
	u_short hp_r_vol = 0;
	wm1811_log_efunc("cur_dev[%d] new_dev[%d]", cur_dev, new_dev);

	if (cur_dev != new_dev) {
		/* update device conf */
		ret = wm1811_read(0x0001, &pm);
		ret = wm1811_read(0x0033, &mix_out_vol);
		hp_l_vol = wm1811_conf->volume[E_VOL_HEADPHONE_L];
		hp_r_vol = wm1811_conf->volume[E_VOL_HEADPHONE_R];
		if (WM1811_ENABLE == new_dev) {
			/* headphone on */
			cp_dyn_pwr = 0x5;
			hpout1_dly = 0x22;
			charge_pump = 0x9F25;
			dac1_to_mixout = 0x1;
			dcs_ena = 0x33;
			analog_hp = 0xEE;
			hp_l_vol |= 0x100;
			hp_r_vol |= 0x100;
			if (WM1811_BIAS_VMID_ENABLE & pm)
				pm |= WM1811_HEADPHONE_ENABLE;
			else {
				pm |= (WM1811_BIAS_VMID_ENABLE |
					WM1811_HEADPHONE_ENABLE);
			}
		} else {
			/* headphone off */
			cp_dyn_pwr = 0x4;
			hpout1_dly = 0x0;
			charge_pump = 0x1F25;
			dcs_ena = 0;
			analog_hp = 0;
			hp_l_vol &= ~0x140;
			hp_r_vol &= ~0x140;
			ret = wm1811_write(0x001C, hp_l_vol);
			ret = wm1811_write(0x001D, hp_r_vol);
			hp_l_vol |= 0x100;
			hp_r_vol |= 0x100;
			if (0x0000 == mix_out_vol) {
				/* earpiece disable */
				dac1_to_mixout = 0x0;
			} else {
				/* earpiece enable */
				dac1_to_mixout = 0x1;
			}
			if (~(WM1811_BIAS_VMID_ENABLE |
				WM1811_HEADPHONE_ENABLE) & pm) {
				pm &= ~WM1811_HEADPHONE_ENABLE;
			} else {
				pm &= ~(WM1811_BIAS_VMID_ENABLE |
					WM1811_HEADPHONE_ENABLE);
			}
		}
		/* Enable Class W, Class W Envelope Tracking = AIF1 */
		ret = wm1811_write(0x0051, cp_dyn_pwr);

		/* Disable HPOUT1LVOL mute */
		ret = wm1811_write(0x001C, hp_l_vol);

		/* Disable HPOUT1RVOL mute */
		ret = wm1811_write(0x001D, hp_r_vol);

		/* Enable bias generator, Enable VMID, */
		/* Enable HPOUT1 (Left) and Enable HPOUT1 (Right) */
		/* input stages */
		ret = wm1811_write(0x0001, pm);

		/* Enable HPOUT1 (Left) and HPOUT1 (Right) */
		/* intermediate stages */
		ret = wm1811_write(0x0060, hpout1_dly);

		/* Enable Charge Pump */
		ret = wm1811_write(0x004C, charge_pump);

		/* INSERT_DELAY_MS [15]->[25] */
		mdelay(25);

		/* Enable AIF1 DAC (Left) Path, */
		/* Enable AIF1 DAC (Right) Path */
		/* Select DAC1 (Left) to Left Headphone Output PGA */
		/* (HPOUT1LVOL) path */
		ret = wm1811_write(0x002D, dac1_to_mixout);

		/* Select DAC1 (Right) to Right Headphone Output */
		/* PGA (HPOUT1RVOL) path */
		ret = wm1811_write(0x002E, dac1_to_mixout);

		/* Enable DC Servo and trigger start-up mode on */
		/* left and right channels */
		ret = wm1811_write(0x0054, dcs_ena);

		/* INSERT_DELAY_MS [250] */
		mdelay(250);

		/* Enable HPOUT1 (Left) and HPOUT1 (Right) */
		/* intermediate and output stages. Remove clamps */
		ret = wm1811_write(0x0060, analog_hp);
	} else {
		/* nothing to do. */
	}

	wm1811_log_rfunc("ret[%d]", ret);
	return ret;
}

/*!
  @brief change state of main microphone device.

  @param[i] cur_dev  cureent device state.
  @param[i] new_dev  new device state.

  @return function results.
*/
static int wm1811_set_main_mic_device(const u_int cur_dev,
				const u_int new_dev)
{
	int ret = 0;
	u_short pm = 0;
	u_short pm2 = 0;
	u_short mixin_l = 0;
	u_short mute_main = 0;
	wm1811_log_efunc("cur_dev[%d] new_dev[%d]", cur_dev, new_dev);

	if (cur_dev != new_dev) {
		ret = wm1811_read(0x0001, &pm);
		ret = wm1811_read(0x0002, &pm2);
		ret = wm1811_read(0x0029, &mixin_l);
		mute_main = wm1811_conf->volume[E_VOL_MAIN_MIC];

		if (WM1811_ENABLE == new_dev) {
			/* main mic on */
			if (WM1811_BIAS_VMID_ENABLE & pm)
				pm |= WM1811_MIC_ENABLE;
			else {
				pm |= (WM1811_BIAS_VMID_ENABLE |
					WM1811_MIC_ENABLE);
			}

			pm2 |= WM1811_IN1L_ENA;
			mute_main &= ~0x80;
			mute_main |= 0x100;
			mixin_l |= WM1811_MIXINL_MIC_ENA;
		} else {
			/* main mic off */
			if (~(WM1811_BIAS_VMID_ENABLE |
				WM1811_HEADPHONE_ENABLE) & pm) {
				pm &= ~WM1811_MIC_ENABLE;
			} else {
				pm &= ~(WM1811_BIAS_VMID_ENABLE |
					WM1811_MIC_ENABLE);
			}

			pm2 &= ~WM1811_IN1L_ENA;
			ret = wm1811_write(0x0018, 0x80);
			mute_main = 0x180;
			mixin_l &= ~WM1811_MIXINL_MIC_ENA;
		}

		/* IN1L MUTE & VOL */
		ret = wm1811_write(0x0018, mute_main);
		/* Enable MICB1_ENA */
		ret = wm1811_write(0x0001, pm);

		/* Enable IN1L Input */
		ret = wm1811_write(0x0002, pm2);
		/* Unmute IN1L PGA output to Left Input Mixer (MIXINL) Path */
		ret = wm1811_write(0x0029, mixin_l);
	} else {
		/* nothing to do. */
	}

	wm1811_log_rfunc("ret[%d]", ret);
	return ret;
}

/*!
  @brief change state of sub microphone device.

  @param[i] cur_dev  cureent device state.
  @param[i] new_dev  new device state.

  @return function results.
*/
static int wm1811_set_sub_mic_device(const u_int cur_dev,
				const u_int new_dev)
{
	int ret = 0;
	u_short pm2 = 0;
	u_short mixin_r = 0;
	u_short mute_sub = 0;
	int sub_mic_ldo = 0;
	wm1811_log_efunc("cur_dev[%d] new_dev[%d]", cur_dev, new_dev);

	if (cur_dev != new_dev) {
		ret = wm1811_read(0x0002, &pm2);
		ret = wm1811_read(0x002A, &mixin_r);
		mute_sub = wm1811_conf->volume[E_VOL_SUB_MIC];

		if (WM1811_ENABLE == new_dev) {
			/* sub mic on */
			pm2 |= WM1811_IN2R_ENA;
			mute_sub &= ~0x80;
			mute_sub |= 0x100;
			mixin_r |= WM1811_MIXINR_MIC_ENA;
			sub_mic_ldo = WM1811_ENABLE;
		} else {
			/* sub mic off */
			pm2 &= ~WM1811_IN2R_ENA;
			ret = wm1811_write(0x001B, 0x80);
			mute_sub = 0x180;
			mixin_r &= ~WM1811_MIXINR_MIC_ENA;
			sub_mic_ldo = WM1811_DISABLE;
		}

		if (4 == u2_get_board_rev()) {
			wm1811_log_info("SUB_MIC_LDO_EN[%d]", sub_mic_ldo);
			gpio_set_value(GPIO_PORT46, sub_mic_ldo);
		}

		/* IN2R MUTE & VOL */
		ret = wm1811_write(0x001B, mute_sub);
		/* Enable IN2R Input */
		ret = wm1811_write(0x0002, pm2);
		/* Unmute IN2R PGA output to Right Input Mixer (MIXINR) Path */
		ret = wm1811_write(0x002A, mixin_r);
	} else {
		/* nothing to do. */
	}

	wm1811_log_rfunc("ret[%d]", ret);
	return ret;
}

/*!
  @brief change state of microphone device.

  @param[i] cur_dev  cureent device state.
  @param[i] new_dev  new device state.
  @param[i] pcm_mode  pcm_mode.

  @return function results.
*/
static int wm1811_set_mic_device(const u_int cur_dev, const u_int new_dev,
				const u_int pcm_mode)
{
	int ret = 0;
	wm1811_log_efunc("cur_dev[%d] new_dev[%d] pcm_mode[%d]",
			cur_dev, new_dev, pcm_mode);

	ret = wm1811_set_main_mic_device(cur_dev, new_dev);

	if ((SNDP_MODE_INCALL == pcm_mode) || (WM1811_DISABLE == new_dev))
		ret = wm1811_set_sub_mic_device(cur_dev, new_dev);

	wm1811_log_rfunc("ret[%d]", ret);
	return ret;
}

/*!
  @brief change state of headset microphone device.

  @param[i] cur_dev  cureent device state.
  @param[i] new_dev  new device state.

  @return function results.
*/
static int wm1811_set_headset_mic_device(const u_int cur_dev,
					const u_int new_dev)
{
	int ret = 0;
	u_short pm = 0;
	u_short pm2 = 0;
	u_short mixin_l = 0;
	u_short mixin_r = 0;
	u_short aif1_control = 0;
	u_short connect = 0;
	u_short mute = 0;
	wm1811_log_efunc("cur_dev[%d] new_dev[%d]", cur_dev, new_dev);

	if (cur_dev != new_dev) {
		ret = wm1811_read(0x0001, &pm);
		ret = wm1811_read(0x0002, &pm2);
		ret = wm1811_read(0x0029, &mixin_l);
		ret = wm1811_read(0x002A, &mixin_r);
		ret = wm1811_read(0x0028, &connect);
		mute = wm1811_conf->volume[E_VOL_HEADSET_MIC];
		if (WM1811_ENABLE == new_dev) {
			/* headset mic on */
			if (WM1811_BIAS_VMID_ENABLE & pm)
				pm |= WM1811_HEADSET_MIC_ENABLE;
			else {
				pm |= (WM1811_BIAS_VMID_ENABLE |
					WM1811_HEADSET_MIC_ENABLE);
			}
			pm2 |= 0x10;
			mixin_l = 0x0000;
			mixin_r |= WM1811_MIXINR_HEADSET_MIC_ENA;
			aif1_control = 0xC010;
			connect |= 0x1;
			mute &= ~0x80;
			mute |= 0x100;
		} else {
			/* headset mic off */
			if (~(WM1811_BIAS_VMID_ENABLE |
				WM1811_HEADSET_MIC_ENABLE) & pm) {
				pm &= ~WM1811_HEADSET_MIC_ENABLE;
			} else {
				pm &= ~(WM1811_BIAS_VMID_ENABLE |
					WM1811_HEADSET_MIC_ENABLE);
			}
			ret = wm1811_write(0x001A, 0x80);
			mute = 0x180;
			/* mixin_l update none. */
			mixin_r &= ~WM1811_MIXINR_HEADSET_MIC_ENA;
			pm2 &= ~0x10;
			aif1_control = 0x4010;
			connect &= ~0x1;
		}
		ret = wm1811_write(0x0300, aif1_control);

		ret = wm1811_write(0x001A, mute);
		/* Connected to IN1RN */
		ret = wm1811_write(0x0028, connect);
		/* Enable IN1R input */
		ret = wm1811_write(0x0002, pm2);
		/* Unmute IN1L PGA output to Left Input Mixer (MIXINL) Path */
		ret = wm1811_write(0x0029, mixin_l);
		/* Unmute IN2R PGA output to Right Input Mixer (MIXINR) Path */
		ret = wm1811_write(0x002A, mixin_r);
	} else {
		/* nothing to do. */
	}

	wm1811_log_rfunc("ret[%d]", ret);
	return ret;
}

#ifdef __WM1811_DEBUG__
/*!
  @brief display private data of AudioLSI driver.

  @param none.

  @return function results.
*/
static void wm1811_dump_wm1811_priv(void)
{
	if (wm1811_conf) {
		wm1811_log_debug("-------------------------------------");
		wm1811_log_debug("wm1811_conf[0x%p]", wm1811_conf);
		wm1811_log_debug("wm1811_conf->info.raw_device[0x%0lx]",
				wm1811_conf->info.raw_device);
		wm1811_log_debug("wm1811_conf->info.speaker[%d]",
				wm1811_conf->info.speaker);
		wm1811_log_debug("wm1811_conf->info.earpiece[%d]",
				wm1811_conf->info.earpiece);
		wm1811_log_debug("wm1811_conf->info.headphone[%d]",
				wm1811_conf->info.headphone);
		wm1811_log_debug("wm1811_conf->info.mic[%d]",
				wm1811_conf->info.mic);
		wm1811_log_debug("wm1811_conf->info.headset_mic[%d]",
				wm1811_conf->info.headset_mic);
		wm1811_log_debug("wm1811_conf->client_wm1811[0x%p]",
				wm1811_conf->client_wm1811);
		wm1811_log_debug("wm1811_conf->log_entry[0x%p]",
				wm1811_conf->log_entry);
		wm1811_log_debug("wm1811_conf->dump_entry[0x%p]",
				wm1811_conf->dump_entry);
		wm1811_log_debug("wm1811_conf->proc_parent[0x%p]",
				wm1811_conf->proc_parent);
		wm1811_log_debug("-------------------------------------");
	} else {
		wm1811_log_debug("-------------------------------------");
		wm1811_log_debug("wm1811_conf is NULL");
		wm1811_log_debug("-------------------------------------");
	}
}
#endif /* __WM1811_DEBUG__ */

static int wm1811_i2c_read_device(struct i2c_client *wm1811,
				u_short reg,
				int bytes,
				void *dest)
{
	struct i2c_msg msg[2];
	u_char wbf[2];
	u_char rbf[2];
	int ret = 0;
	wm1811_log_debug("");

	wbf[0] = (reg>>8) & 0x00ff;
	wbf[1] = (reg>>0) & 0x00ff;
	rbf[0] = 0x0;
	rbf[1] = 0x0;

	msg[0].addr  = wm1811->addr;
	msg[0].flags = wm1811->flags & I2C_M_TEN;
	msg[0].len   = sizeof(wbf);
	msg[0].buf   = wbf;

	msg[1].addr  = wm1811->addr;
	msg[1].flags = (wm1811->flags & I2C_M_TEN) | I2C_M_RD;
	msg[1].len   = sizeof(rbf);
	msg[1].buf   = rbf;

	ret = i2c_transfer(wm1811->adapter,
			   msg,
			   ARRAY_SIZE(msg));
	*(u_short *)dest = (rbf[0] << 8) + rbf[1];

	wm1811_log_debug("ret[%d]", ret);
	return ret;
}

static int wm1811_i2c_write_device(struct i2c_client *wm1811,
				u_short reg,
				int bytes /* not use */,
				u_short *src)
{
	int ret;
	u_char w_data2[4];
	wm1811_log_debug("");

	w_data2[0] = (reg    & 0xFF00) >> 8;
	w_data2[1] = (reg    & 0x00FF);
	w_data2[2] = (src[0] & 0xFF00) >> 8;
	w_data2[3] = (src[0] & 0x00FF);

	ret = i2c_smbus_write_i2c_block_data(
				wm1811_conf->client_wm1811,
				w_data2[0], 3, &w_data2[1]);

	wm1811_log_debug("ret[%d]", ret);
	return ret;
}

static int wm1811_playback_volume_set(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct soc_mixer_control *mc =
		(struct soc_mixer_control *)kcontrol->private_value;
	int ret = 0;
	u_int val_l = ucontrol->value.integer.value[0];
	u_int val_r = ucontrol->value.integer.value[1];
	u_short reg_val_l = 0x0040;
	u_short reg_val_r = 0x0040;
	wm1811_log_efunc("");

	mutex_lock(&wm1811_mutex);

	val_l = min(val_l, (u_int)WM1811_PLAYBACK_VOL_RANGE);
	val_r = min(val_r, (u_int)WM1811_PLAYBACK_VOL_RANGE);

	reg_val_l |= (u_short)val_l;
	reg_val_r |= (u_short)val_l;

	ret = wm1811_write((u_short)mc->reg, reg_val_l);

	if (0 != ret) {
		wm1811_log_err("wm1811 register left write failed.");
		goto err_reg_write;
	}

	ret = wm1811_write((u_short)mc->rreg, reg_val_r);

	if (0 != ret) {
		wm1811_log_err("wm1811 register right write failed.");
		goto err_reg_write;
	}

	ret = wm1811_store_volume((u_short)mc->reg, reg_val_l);

	if (0 != ret) {
		wm1811_log_err("wm1811 vol_l store failed.");
		goto err_reg_store;
	}

	ret = wm1811_store_volume((u_short)mc->rreg, reg_val_r);

	if (0 != ret) {
		wm1811_log_err("wm1811 vol_r store failed.");
		goto err_reg_store;
	}

	mutex_unlock(&wm1811_mutex);

	wm1811_log_rfunc("ret[%d]", ret);
	return ret;

err_reg_store:
err_reg_write:
	mutex_unlock(&wm1811_mutex);
	wm1811_log_err("ret[%d]", ret);
	return ret;
}

static int wm1811_playback_volume_get(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct soc_mixer_control *mc =
		(struct soc_mixer_control *)kcontrol->private_value;
	int ret = 0;
	u_int val_l = 0;
	u_int val_r = 0;
	u_short reg_val_l = 0;
	u_short reg_val_r = 0;
	wm1811_log_efunc("");

	mutex_lock(&wm1811_mutex);

	ret = wm1811_read((u_short)mc->reg, &reg_val_l);

	if (0 > ret) {
		wm1811_log_err("wm1811 register read failed.");
		goto err_reg_read;
	}

	ret = wm1811_read((u_short)mc->rreg, &reg_val_r);

	if (0 > ret) {
		wm1811_log_err("wm1811 register read failed.");
		goto err_reg_read;
	}

	val_l = (u_int)(reg_val_l & WM1811_PLAYBACK_VOL_RANGE);
	val_r = (u_int)(reg_val_r & WM1811_PLAYBACK_VOL_RANGE);

	ucontrol->value.integer.value[0] = val_l;
	ucontrol->value.integer.value[1] = val_r;

	mutex_unlock(&wm1811_mutex);

	wm1811_log_rfunc("ret[%d]", ret);
	return ret;

err_reg_read:
	mutex_unlock(&wm1811_mutex);
	wm1811_log_err("ret[%d]", ret);
	return ret;
}

static int wm1811_capture_volume_set(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct soc_mixer_control *mc =
		(struct soc_mixer_control *)kcontrol->private_value;
	int ret = 0;
	u_int val = ucontrol->value.integer.value[0];
	u_short reg_val = 0;
	wm1811_log_efunc("");

	mutex_lock(&wm1811_mutex);

	val = min(val, (u_int)31);
	reg_val |= (u_short)val;

	ret = wm1811_write((u_short)mc->reg, reg_val);

	if (0 != ret) {
		wm1811_log_err("wm1811 register write failed.");
		goto err_reg_write;
	}

	ret = wm1811_store_volume((u_short)mc->reg, reg_val);

	if (0 != ret) {
		wm1811_log_err("wm1811 vol_l store failed.");
		goto err_reg_store;
	}

	mutex_unlock(&wm1811_mutex);

	wm1811_log_rfunc("ret[%d]", ret);
	return ret;

err_reg_store:
err_reg_write:
	mutex_unlock(&wm1811_mutex);
	wm1811_log_err("ret[%d]", ret);
	return ret;
}

static int wm1811_capture_volume_get(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct soc_mixer_control *mc =
		(struct soc_mixer_control *)kcontrol->private_value;
	int ret = 0;
	u_int val = 0;
	u_short reg_val = 0;
	wm1811_log_efunc("");

	mutex_lock(&wm1811_mutex);

	ret = wm1811_read((u_short)mc->reg, &reg_val);

	if (0 > ret) {
		wm1811_log_err("wm1811 register read failed.");
		goto err_reg_read;
	}

	val = (u_int)(reg_val & WM1811_CAPTURE_VOL_RANGE);
	ucontrol->value.integer.value[0] = val;

	mutex_unlock(&wm1811_mutex);

	wm1811_log_rfunc("ret[%d]", ret);
	return ret;

err_reg_read:
	mutex_unlock(&wm1811_mutex);
	wm1811_log_err("ret[%d]", ret);
	return ret;
}

static int wm1811_store_volume(const u_short addr, const u_short value)
{
	int ret = 0;
	u_short reg_id = 0;
	wm1811_log_efunc("addr[0x%x],value[0x%x]", addr, value);

	switch (addr) {
	case 0x0026:
		reg_id = E_VOL_SPEAKER_L;
		break;
	case 0x0027:
		reg_id = E_VOL_SPEAKER_R;
		break;
	case 0x0020:
		reg_id = E_VOL_EARPIECE_L;
		break;
	case 0x0021:
		reg_id = E_VOL_EARPIECE_R;
		break;
	case 0x001C:
		reg_id = E_VOL_HEADPHONE_L;
		break;
	case 0x001D:
		reg_id = E_VOL_HEADPHONE_R;
		break;
	case 0x0018:
		reg_id = E_VOL_MAIN_MIC;
		break;
	case 0x001B:
		reg_id = E_VOL_SUB_MIC;
		break;
	case 0x001A:
		reg_id = E_VOL_HEADSET_MIC;
		break;
	default:
		ret = -EINVAL;
		break;
	}

	if (0 == ret) {
		wm1811_conf->volume[reg_id] = value;
		wm1811_conf->volume_saved[reg_id] = WM1811_ENABLE;
	}

	wm1811_log_rfunc("ret[%d]", ret);
	return ret;
}

static int wm1811_restore_volume(const u_long device)
{
	int ret = 0;
	wm1811_log_efunc("");

	if ((WM1811_DEV_PLAYBACK_SPEAKER & device) &&
		(WM1811_ENABLE ==
			wm1811_conf->volume_saved[E_VOL_SPEAKER_L]) &&
		(WM1811_ENABLE ==
			wm1811_conf->volume_saved[E_VOL_SPEAKER_R])) {
		ret = wm1811_write(0x0026,
			wm1811_conf->volume[E_VOL_SPEAKER_L]);
		ret = wm1811_write(0x0027,
			wm1811_conf->volume[E_VOL_SPEAKER_R]);
	}

	if ((WM1811_DEV_PLAYBACK_EARPIECE & device) &&
		(WM1811_ENABLE ==
			wm1811_conf->volume_saved[E_VOL_EARPIECE_L]) &&
		(WM1811_ENABLE ==
			wm1811_conf->volume_saved[E_VOL_EARPIECE_R])) {
		ret = wm1811_write(0x0020,
			wm1811_conf->volume[E_VOL_EARPIECE_L]);
		ret = wm1811_write(0x0021,
			wm1811_conf->volume[E_VOL_EARPIECE_R]);
	}

	if ((WM1811_DEV_PLAYBACK_HEADPHONES & device) &&
		(WM1811_ENABLE ==
			wm1811_conf->volume_saved[E_VOL_HEADPHONE_L]) &&
		(WM1811_ENABLE ==
			wm1811_conf->volume_saved[E_VOL_HEADPHONE_R])) {
		ret = wm1811_write(0x001C,
			wm1811_conf->volume[E_VOL_HEADPHONE_L]);
		ret = wm1811_write(0x001D,
			wm1811_conf->volume[E_VOL_HEADPHONE_R]);
	}

	if ((WM1811_DEV_CAPTURE_MIC & device) &&
		(WM1811_ENABLE ==
			wm1811_conf->volume_saved[E_VOL_MAIN_MIC]) &&
		(WM1811_ENABLE ==
			wm1811_conf->volume_saved[E_VOL_SUB_MIC])) {
		ret = wm1811_write(0x0018,
			wm1811_conf->volume[E_VOL_MAIN_MIC]);
		ret = wm1811_write(0x001B,
			wm1811_conf->volume[E_VOL_SUB_MIC]);
	}

	if ((WM1811_DEV_CAPTURE_HEADSET_MIC & device) &&
		(WM1811_ENABLE ==
			wm1811_conf->volume_saved[E_VOL_HEADSET_MIC])) {
		ret = wm1811_write(0x001A,
			wm1811_conf->volume[E_VOL_HEADSET_MIC]);
	}

	wm1811_log_rfunc("ret[%d]", ret);
	return ret;
}

static int wm1811_resume(const u_long device, u_int pcm_mode)
{
	int ret = 0;
	wm1811_log_efunc("");

	ret = wm1811_restore_volume(device);

	wm1811_log_rfunc("ret[%d]", ret);
	return ret;
}

static int wm1811_suspend(u_int pcm_mode)
{
	int ret = 0;
	u_short status1 = 0;
	u_short status2 = 0;
	u_short ldo_1 = 0;
	u_short pull_down = 0;
	u_short de_bouce = 0;
	wm1811_log_efunc("");

	ret = wm1811_write(0x0204, 0x0000);
	ret = wm1811_write(0x0211, 0x0001);
	ret = wm1811_write(0x0204, 0x0009);
	ret = wm1811_write(0x0208, 0x000F);/* use mclk2 */

	if (0 == wm1811_conf->switch_data.state) {
		/* jack detect. */
		ret = wm1811_write(0x00D0, 0x0300);
		ret = wm1811_write(0x0700, 0x2003);
		ret = wm1811_write(0x0039, 0x0164);
	} else {
		/* jack and accessory detect. */
		ret = wm1811_write(0x00D0, 0x0301);
		ret = wm1811_write(0x0700, 0x2005);
		ret = wm1811_write(0x0039, 0x00E4);
	}

	/* interrupt status clear */
	wm1811_read(0x0730, &status1);
	wm1811_read(0x0731, &status2);
	wm1811_write(0x0730, status1);
	wm1811_write(0x0731, status2);

	/* low power setting */
	wm1811_read(0x003B, &ldo_1);
	ldo_1 &= ~0x1;
	wm1811_write(0x003B, ldo_1);

	wm1811_read(0x0721, &pull_down);
	pull_down &= ~0x50;
	wm1811_write(0x0721, pull_down);

	wm1811_read(0x0705, &de_bouce);
	de_bouce &= ~0x100;
	wm1811_write(0x0705, de_bouce);

	/* Hi-Z setting */
	ret = wm1811_write(0x0302, 0x8000);
	ret = wm1811_write(0x0420, 0x0200);

	wm1811_log_rfunc("ret[%d]", ret);
	return ret;
}

static int wm1811_set_irq_wake(const u_int set_wake_state, u_int pcm_mode)
{
	int ret = 0;

	wm1811_log_efunc("conf-wake_state[%d] pcm_mode[%d] set_wake_state[%d]",
			wm1811_conf->irq_wake_state, pcm_mode, set_wake_state);

	/* pcm mode & set state check */
	if ((SNDP_MODE_INCALL == pcm_mode) &&
		(WM1811_IRQ_WAKE_OFF == set_wake_state)) {
		wm1811_log_rfunc("INCALL : No Change wake_state");
		return ret;
	}

	/* state check */
	if ((wm1811_conf->irq_wake_state) != set_wake_state) {
		wm1811_log_info("conf-wake_state[%d] set_wake_state[%d]",
				wm1811_conf->irq_wake_state, set_wake_state);
		ret = irq_set_irq_wake(wm1811_conf->irq, set_wake_state);
		if (0 == ret)
			wm1811_conf->irq_wake_state = set_wake_state;
	}

	wm1811_log_rfunc("conf-wake_state[%d] ret[%d]",
				wm1811_conf->irq_wake_state, ret);
	return ret;
}

/*------------------------------------*/
/* for public function                */
/*------------------------------------*/
int wm1811_set_device(const u_long device, const u_int pcm_value,
	u_int power)
{
	int ret = 0;
	struct wm1811_info new_device = wm1811_conf->info;
	static int audio_on = WM1811_DISABLE;
	u_int pcm_mode = 0;
	u_short aif1_rate = 0;
	u_short fll1_control2 = 0;
	u_short fll1_control4 = 0;
	u_short fll1_control5 = 0;
	wm1811_log_efunc("device[%ld] pcm_value[0x%08x] power[%d]",
			device, pcm_value, power);

	mutex_lock(&wm1811_mutex);

	if ((WM1811_POWER_ON == power) &&
		(0 == gpio_get_value(GPIO_PORT34))) {
		wm1811_log_info("CODEC_LDO_EN : wm1811 Power ON");
		gpio_set_value(GPIO_PORT34, 1);
	}

	ret = wm1811_check_device(device);

	if (0 != ret) {
		mutex_unlock(&wm1811_mutex);
		wm1811_log_err("parameter error. ret[%d]", ret);
		return ret;
	}

	/* check update */
	if ((WM1811_POWER_ON == power) &&
		(wm1811_conf->info.raw_device == device)) {
		mutex_unlock(&wm1811_mutex);
		wm1811_log_rfunc("no changed. ret[%d]", ret);
		return ret;
	}

	ret = wm1811_conv_device_info(device, &new_device);

	if (0 != ret) {
		mutex_unlock(&wm1811_mutex);
		wm1811_log_err("device convert error. ret[%d]", ret);
		return ret;
	}

	if (WM1811_DISABLE == audio_on) {
		/***********************************/
		/* Software reset                  */
		/***********************************/
		ret = wm1811_write(0x0000, 0x0000);

		/* LDO1 2.8V, discharged when disable */
		ret = wm1811_write(0x003B, 0x0009);

		/*****************************************************/
		/* General Purpose Input/Output (GPIO) Configuration */
		/*****************************************************/
		/* GPIO 1 - Set to GPIO (output, pull-down, active high */
		/* Microphone Detect IRQ output) */
		ret = wm1811_write(0x0700, 0x2005);

		ret = wm1811_write(0x00D0, 0x0801);
		ret = wm1811_write(0x00D1, 0x007F);

		ret = wm1811_write(0x0102, 0x0001);
		ret = wm1811_write(0x00D3, 0x3F3F);
		ret = wm1811_write(0x00D4, 0x3F3F);
		ret = wm1811_write(0x00D5, 0x3F3F);
		ret = wm1811_write(0x00D6, 0x3226);
		ret = wm1811_write(0x0102, 0x0000);

		ret = wm1811_write(0x0738, 0x07DE);
		ret = wm1811_write(0x0739, 0xDBED);
		ret = wm1811_write(0x0740, 0x0000);
	}

	/* get pcm mode type */
	pcm_mode = SNDP_GET_MODE_VAL(pcm_value);
	ret = wm1811_read(0x0221, &fll1_control2);

	if ((WM1811_DISABLE == audio_on) ||
	    ((SNDP_MODE_INCALL == pcm_mode) && (0x1700 != fll1_control2)) ||
	    ((SNDP_MODE_INCALL != pcm_mode) && (0x0704 != fll1_control2))) {
		/***********************************/
		/* Disable Clocking/FLL1/AIF1      */
		/***********************************/
		/* AIF1CLK Disable */
		ret = wm1811_write(0x0200, 0x0010);

		/* FLL1 Disable */
		ret = wm1811_write(0x0220, 0x0000);

		/* AIF1 Disable */
		ret = wm1811_write(0x0208, 0x0008);

		if (SNDP_MODE_INCALL == pcm_mode) {
			/* SNDP_MODE_INCALL */
			/* AIF1 Sample Rate = 16.0 kHz, */
			/* AIF1CLK/Fs ratio = 256 (Default Register Value) */
			aif1_rate = 0x0033;

			/* FLL1: (LR / REF_DIV[1]) * (N[48] + */
			/* THETA[0]/LAMBDA) */
			/* * FRATIO[1] / OUTDIV[24] */
			/* OUTDIV=23(/24), FRATIO=0(x1) */
			fll1_control2 = 0x1700;

			/* N=48 */
			fll1_control4 = 0x0600;

			/* BYP=0, non-free, REF_DIV=0(/1), SRC=3(BCLK1) */
			fll1_control5 = 0x0003;

			/* Enable irq wake */
			ret = wm1811_set_irq_wake(WM1811_IRQ_WAKE_ON,
						pcm_mode);
		} else {
			/* SNDP_MODE_NORMAL/SNDP_MODE_RING */
			/* AIF1 Sample Rate = 48 kHz, */
			/* AIF1CLK/Fs ratio = 256 (Default Register Value) */
			aif1_rate = 0x0083;

			/* FLL1: (LR / REF_DIV[1]) * (N[128] + */
			/* THETA[0]/LAMBDA) */
			/* * FRATIO[16] / OUTDIV[8] */
			/* OUTDIV=7(/8), FRATIO=100(x16) */
			fll1_control2 = 0x0704;

			/* N=128 */
			fll1_control4 = 0x1000;

			/* BYP=0, non-free, REF_DIV=0(/1), SRC=2(LRCLK) */
			fll1_control5 = 0x0002;

			/* Disable irq wake */
			ret = wm1811_set_irq_wake(WM1811_IRQ_WAKE_OFF,
								 pcm_mode);
		}

		/***********************************/
		/* Clocking                        */
		/***********************************/
		/* AIF1 Sample Rate, AIF1CLK/Fs */
		ret = wm1811_write(0x0210, aif1_rate);

		/* AIF2 Sample Rate = 48.0 kHz, */
		/* AIF2CLK/Fs ratio = 256 (Default Register Value) */
		ret = wm1811_write(0x0211, 0x0083);

		/* AIF1 Word Length = 16-bits, AIF1 Format = I2S */
		ret = wm1811_write(0x0300, 0x4010);

		/* AIF1 Slave Mode (Default Register Value) */
		ret = wm1811_write(0x0302, 0x0000);

		/* Enable the DSP processing clock for AIF1 & AIF2, */
		/* select AIF2 for SYS_CLK */
		ret = wm1811_write(0x0208, 0x000F);

		/***********************************/
		/* FFL1                            */
		/***********************************/
		/* FLL1: (LR / REF_DIV[1]) * (N + THETA[0]/LAMBDA) */
		ret = wm1811_write(0x0221, fll1_control2);

		/* THETA=0 */
		ret = wm1811_write(0x0222, 0x0000);

		/* N */
		ret = wm1811_write(0x0223, fll1_control4);

		/* BYP=0, non-free, REF_DIV=0(/1), SRC */
		ret = wm1811_write(0x0224, fll1_control5);

		/* FLL_ENA=1, FLL_OSC_ENA=0 */
		ret = wm1811_write(0x0220, 0x0001);

		/* INSERT_DELAY_MS [5] */
		mdelay(5);

		/* AIF1CLK_SRC=2(FLL1), INV=0, DIV=/1, AIF1CLK=enable */
		ret = wm1811_write(0x0200, 0x0011);

		/***********************************/
		/* FFL2                            */
		/***********************************/
		/*FLL2 is not used*/

		/* AIF2CLK_SRC=3(MCLK2), INV=0, DIV=/1, AIF2CLK=disable */
		ret = wm1811_write(0x0204, 0x0008);
	}

	if (WM1811_DISABLE == audio_on) {
		/***********************************/
		/* Analogue Configuration          */
		/***********************************/
		/* MICBIAS1 1.8V */
		ret = wm1811_write(0x003D, 0x0033);

		/* MICBIAS2 2.6V */
		ret = wm1811_write(0x003E, 0x003F);

		/* Enable VMID soft start (fast), */
		/* Start-up Bias Current Enabled */
		ret = wm1811_write(0x0039, 0x01E4);

		/* Enable bias generator, Enable VMID */
		ret = wm1811_write(0x0001, 0x0003);

		/* INSERT_DELAY_MS [50] */
		mdelay(50);

		/***********************************/
		/* Analogue Input Configuration    */
		/***********************************/
		/* Enable Themal shutdown control */
		ret = wm1811_write(0x0002, 0x2000);

		/* Connect IN2RN to IN2R PGA, Connect IN2RP to IN2R PGA */
		ret = wm1811_write(0x0028, 0x003C);

		/***********************************/
		/* Path Configuration              */
		/***********************************/
		/* Enable DAC1 (Left), Enable DAC1 (Right), */
		ret = wm1811_write(0x0005, 0x0303);

		/* Enable the AIF1 (Left) to DAC 1 (Left) mixer path */
		ret = wm1811_write(0x0601, 0x0001);

		/* Enable the AIF1 (Right) to DAC 1 (Right) mixer path */
		ret = wm1811_write(0x0602, 0x0001);

		/* Enable ADC (Left) to AIF1 Timeslot 0 ADC (Left) Path */
		ret = wm1811_write(0x0606, 0x0002);

		/* Enable ADC (Right) to AIF1 Timeslot 0 ADC (Right) Path */
		ret = wm1811_write(0x0607, 0x0002);

		/***********************************/
		/* Path Set Enable                 */
		/***********************************/
		/* Enable bias generator, Enable VMID, */
		/* Enable HPOUT1 (Left) and Enable HPOUT1 (Right) */
		/* input stages */
		ret = wm1811_write(0x0001, 0x0023);

		ret = wm1811_write(0x0003, 0x00C0);

		/***********************************/
		/* Performance                     */
		/***********************************/
		/* ADC / Digital Microphone Oversample Low Power */

		ret = wm1811_write(0x0620, 0x0000);

		ret = wm1811_write(0x0003, 0x0030);

		ret = wm1811_write(0x001C, 0x002D);

		ret = wm1811_write(0x001D, 0x002D);

		ret = wm1811_write(0x0020, 0x0040);

		ret = wm1811_write(0x0026, 0x0000);

		ret = wm1811_write(0x0027, 0x0000);

		ret = wm1811_write(0x0021, 0x0040);

		ret = wm1811_write(0x0024, 0x0000);

		ret = wm1811_resume(device, pcm_mode);
	}

	/* set value */
	ret = wm1811_set_speaker_device(wm1811_conf->info.speaker,
					  new_device.speaker);
	if (0 != ret)
		goto err_set_device;

	ret = wm1811_set_earpiece_device(wm1811_conf->info.earpiece,
					   new_device.earpiece);
	if (0 != ret)
		goto err_set_device;

	ret = wm1811_set_headphone_device(wm1811_conf->info.headphone,
					    new_device.headphone);
	if (0 != ret)
		goto err_set_device;

	ret = wm1811_set_mic_adc_pm(wm1811_conf->info.mic, new_device.mic,
					wm1811_conf->info.headset_mic,
					new_device.headset_mic,
					pcm_mode);

	if (0 != ret)
		goto err_set_device;

	ret = wm1811_set_mic_device(wm1811_conf->info.mic,
					new_device.mic,
					pcm_mode);

	if (0 != ret)
		goto err_set_device;

	ret = wm1811_set_headset_mic_device(wm1811_conf->info.headset_mic,
					      new_device.headset_mic);

	if (0 != ret)
		goto err_set_device;

	ret = wm1811_set_mic_mixer_pm(wm1811_conf->info.mic, new_device.mic,
					wm1811_conf->info.headset_mic,
					new_device.headset_mic,
					pcm_mode);

	if (0 != ret)
		goto err_set_device;

	if (WM1811_DISABLE == audio_on) {
		/***********************************/
		/* Unmutes                         */
		/***********************************/
		/* Unmute DAC 1 (Left) */
		ret = wm1811_write(0x0610, 0x00C0);

		/* Unmute DAC 1 (Right) */
		ret = wm1811_write(0x0611, 0x00C0);

		/* Unmute the AIF1 DAC path */
		ret = wm1811_write(0x0420, 0x0000);

		audio_on = WM1811_ENABLE;
	}

	if (WM1811_DEV_NONE == device) {
		ret = wm1811_suspend(pcm_mode);

		if ((WM1811_POWER_OFF == power) &&
			(1 == gpio_get_value(GPIO_PORT34))) {
			wm1811_log_info("CODEC_LDO_EN : wm1811 Power OFF");
			gpio_set_value(GPIO_PORT34, 0);
		}

		audio_on = WM1811_DISABLE;
	} else {
		ret = wm1811_write(0x0208, 0x000e);
	}

	wm1811_conf->info = new_device;
	mutex_unlock(&wm1811_mutex);

	wm1811_log_rfunc("ret[%d]", ret);
	return ret;

err_set_device:
	wm1811_conf->info = new_device;
	mutex_unlock(&wm1811_mutex);
	wm1811_log_err("ret[%d]", ret);
	return ret;
}
EXPORT_SYMBOL(wm1811_set_device);

int wm1811_get_device(u_long *device)
{
	int ret = 0;
	wm1811_log_efunc("");

	/* check param */
	if (NULL == device) {
		ret = -EINVAL;
		wm1811_log_err("parameter error. device is null. ret[%d]",
				ret);
		return ret;
	}

	/* get value */
	*device = wm1811_conf->info.raw_device;

	wm1811_log_rfunc("ret[%d] device[%ld]", ret, *device);
	return ret;
}
EXPORT_SYMBOL(wm1811_get_device);

int wm1811_set_volume(const u_long device, const u_int volume)
{
	int ret = 0;
	wm1811_log_efunc("device[%ld] volume[%d]", device, volume);
	wm1811_log_info("not supported yet.");
	wm1811_log_rfunc("ret[%d]", ret);
	return ret;
}
EXPORT_SYMBOL(wm1811_set_volume);

int wm1811_get_volume(const u_long device, u_int *volume)
{
	int ret = 0;
	wm1811_log_efunc("device[%ld]", device);
	wm1811_log_info("not supported yet.");
	wm1811_log_rfunc("ret[%d] volume[%d]", ret, *volume);
	return ret;
}
EXPORT_SYMBOL(wm1811_get_volume);

int wm1811_set_mute(const u_int mute)
{
	int ret = 0;
	wm1811_log_efunc("mute[%d]", mute);
	wm1811_log_info("not supported yet.");
	wm1811_log_rfunc("ret[%d]", ret);
	return ret;
}
EXPORT_SYMBOL(wm1811_set_mute);

int wm1811_get_mute(u_int *mute)
{
	int ret = 0;
	wm1811_log_efunc("");

	/* check param */
	if (NULL == mute) {
		ret = -EINVAL;
		wm1811_log_err("parameter error. mute is null. ret[%d]",
				ret);
		return ret;
	}

	/* get value */
	wm1811_log_info("not supported yet.");

	wm1811_log_rfunc("ret[%d] mute[%d]", ret, *mute);
	return ret;
}
EXPORT_SYMBOL(wm1811_get_mute);

int wm1811_set_speaker_amp(const u_int value)
{
	int ret = 0;
#if 0
	wm1811_log_efunc("value[%d]", value);
	wm1811_log_info("not supported yet.");
	wm1811_log_rfunc("ret[%d]", ret);
#endif
	return ret;
}
EXPORT_SYMBOL(wm1811_set_speaker_amp);

int wm1811_get_speaker_amp(u_int *value)
{
	int ret = 0;
	wm1811_log_efunc("");

	/* check param */
	if (NULL == value) {
		ret = -EINVAL;
		wm1811_log_err("parameter error. value is null. ret[%d]",
				ret);
		return ret;
	}

	/* get value */
	wm1811_log_info("not supported yet.");

	wm1811_log_rfunc("ret[%d] value[%d]", ret, *value);
	return ret;
}
EXPORT_SYMBOL(wm1811_get_speaker_amp);

int wm1811_get_status(u_long *irq_status)
{
	int ret = 0;
	wm1811_log_efunc("");

	/* check param */
	if (NULL == irq_status) {
		ret = -EINVAL;
		wm1811_log_err("parameter error. value is null. ret[%d]",
				ret);
		return ret;
	}

	/* get value */
	wm1811_log_info("not supported yet.");

	wm1811_log_rfunc("ret[%d] irq_status[%ld]", ret, *irq_status);
	return ret;
}
EXPORT_SYMBOL(wm1811_get_status);

int wm1811_register_callback_func(struct wm1811_callback_func *callback_)
{
	int ret = 0;
	wm1811_log_efunc("");

	/* check param */
	if (NULL == callback_) {
		ret = -EINVAL;
		wm1811_log_err("parameter error. callback_ is null.ret[%d]",
				ret);
		return ret;
	}

	if (NULL == callback_->irq_func) {
		ret = -EINVAL;
		wm1811_log_err(
			"parameter error. callback_->irq_func is null.ret[%d]",
			ret);
		return ret;
	}

	/* set value */
	wm1811_log_info("not supported yet.");

	wm1811_log_rfunc("ret[%d]", ret);
	return ret;
}
EXPORT_SYMBOL(wm1811_register_callback_func);

int wm1811_dump_registers(void)
{
	int ret = 0;
	u_short i = 0;
	u_short val = 0;
	wm1811_log_debug("");

	mutex_lock(&wm1811_mutex);

#ifdef __WM1811_DEBUG__
	wm1811_dump_wm1811_priv();
#endif /* __WM1811_DEBUG__ */

	for (i = 0; i < WM1811_CACHE_SIZE; i++) {
		if (0x0000 == wm1811_access_masks[i].readable) {
			wm1811_log_debug("addr[0x%04X] write-only register",
					i);
		} else {
			ret = wm1811_read(i, &val);

			if (0 > ret)
				goto err_i2c_read;

			wm1811_log_info("R%04d addr[0x%04X] val[0x%04X]",
					i, i, val);
		}
	}

	mutex_unlock(&wm1811_mutex);

	wm1811_log_debug("ret[%d]", ret);
	return ret;
err_i2c_read:
	mutex_unlock(&wm1811_mutex);
	wm1811_log_err("ret[%d]", ret);
	return ret;
}
EXPORT_SYMBOL(wm1811_dump_registers);

int wm1811_read(const u_short addr, u_short *value)
{
	int ret = 0;
	wm1811_log_debug("");

	ret = wm1811_i2c_read_device(wm1811_conf->client_wm1811,
					addr, 2, value);

	if (2 != ret)
		wm1811_log_err("ret[%d]", ret);

	wm1811_log_debug("ret[%d] addr[0x%04X] value[0x%04X]",
			ret, addr, *value);
	return ret;
}
EXPORT_SYMBOL(wm1811_read);

int wm1811_write(const u_short addr, const u_short value)
{
	int ret = 0;
	int ret2 = 0;
	int retry = 10;
	u_short val = 0;
	wm1811_log_debug("");

	wm1811_log_info("WM1811 addr[0x%04X] value[0x%04X]", addr, value);

	ret = wm1811_i2c_write_device(wm1811_conf->client_wm1811,
					addr, 2, (u_short *)&value);

	if (0 != ret)
		wm1811_log_err("ret[%d]", ret);

	if ((0x0001 == addr) || (0x0002 == addr) ||
	    (0x0003 == addr) || (0x0004 == addr) ||
	    (0x0005 == addr) || (0x0006 == addr)) {
		while (0 < retry--) {
			ret2 = wm1811_read(addr, &val);
			wm1811_log_info("addr[0x%04X]value[0x%04X]val[0x%04X]",
					addr, value, val);

			if (value == val)
				break;

			mdelay(10);
			ret2 = wm1811_i2c_write_device(
					wm1811_conf->client_wm1811,
					addr, 2, (u_short *)&value);

			if (0 != ret2)
				wm1811_log_err("ret2[%d]", ret2);

			wm1811_log_info("retry[%d]", retry);
		}
	}

	wm1811_log_debug("ret[%d]", ret);
	return ret;
}
EXPORT_SYMBOL(wm1811_write);

/*------------------------------------*/
/* for system                         */
/*------------------------------------*/
static int wm1811_proc_log_read(char *page, char **start, off_t offset,
			int count, int *eof, void *data)
{
	int len = 0;
	wm1811_log_efunc("");

	len = sprintf(page, "0x%08x\n", (int)wm1811_log_level);
	*eof = 1;

	wm1811_log_rfunc("len[%d]", len);
	return len;
}

static int wm1811_proc_log_write(struct file *filp, const char *buffer,
	u_long count, void *data)
{
	int r = 0;
	int in = 0;
	char *temp = NULL;
	wm1811_log_efunc("filp[%p] buffer[%p] count[%ld] data[%p]",
			filp, buffer, count, data);

	temp = kmalloc(count, GFP_KERNEL);
	memset(temp, 0, count);
	strncpy(temp, buffer, count);
	temp[count-1] = '\0';
	r = kstrtoint(temp, 0, &in);
	kfree(temp);

	if (r)
		return r;

	wm1811_log_level = (u_int)in;

	wm1811_log_rfunc("count[%ld]", count);
	return count;
}

static int wm1811_proc_dump_read(char *page, char **start, off_t offset,
			int count, int *eof, void *data)
{
	wm1811_log_efunc("");
	/* nothing to do. */
	wm1811_log_rfunc("");
	return 0;
}

static int wm1811_proc_dump_write(struct file *filp, const char *buffer,
	u_long count, void *data)
{
	int r = 0;
	int in = 0;
	char *temp = NULL;
	wm1811_log_efunc("filp[%p] buffer[%p] count[%ld] data[%p]",
			filp, buffer, count, data);

	temp = kmalloc(count, GFP_KERNEL);
	memset(temp, 0, count);
	strncpy(temp, buffer, count);
	temp[count-1] = '\0';
	r = kstrtoint(temp, 0, &in);
	kfree(temp);

	if (r)
		return r;

	wm1811_dump_registers();

	wm1811_log_rfunc("");
	return count;
}

static int wm1811_probe(struct snd_soc_codec *codec)
{
	int ret = 0;
	struct wm1811_priv *wm1811 = snd_soc_codec_get_drvdata(codec);
	wm1811_log_efunc("codec[0x%p]", codec);

	wm1811->codec = codec;
	ret = snd_soc_codec_set_cache_io(codec, 8, 8, SND_SOC_I2C);

	if (ret)
		wm1811_log_err("Failed to set chache I/O: %d\n", ret);

	ret = snd_soc_add_controls(codec, wm1811_controls,
				wm1811_array_size);

	if (ret)
		wm1811_log_err("Failed to create control: %d\n", ret);

	ret = snd_soc_add_controls(codec, wm1811_volume_controls,
				ARRAY_SIZE(wm1811_volume_controls));

	if (ret)
		wm1811_log_err("Failed to create volume control: %d\n", ret);

	wm1811_log_rfunc("ret[%d]", ret);
	return ret;
}

static int wm1811_remove(struct snd_soc_codec *codec)
{
	wm1811_log_efunc("codec[0x%p]", codec);
	/* nothing to do. */
	wm1811_log_rfunc("");
	return 0;
}

struct snd_soc_codec_driver soc_codec_dev_wm1811 = {
	.probe = wm1811_probe,
	.remove = wm1811_remove,
	.reg_cache_size = 0,
	.reg_word_size = sizeof(u8),
	.reg_cache_default = NULL,
};

static struct snd_soc_dai_driver wm1811_dai_driver[] = {
	{
		.name = "wm1811-hifi",
		.playback = {
			.stream_name = "Playback",
			.channels_min = 1,
			.channels_max = 2,
			.rates = SNDRV_PCM_RATE_8000_48000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
					SNDRV_PCM_FMTBIT_S24_LE },
		.capture = {
			.stream_name = "Capture",
			.channels_min = 1,
			.channels_max = 2,
			.rates = SNDRV_PCM_RATE_8000_48000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
					SNDRV_PCM_FMTBIT_S24_LE },
		.ops = NULL,
	},
	{
		.name = "wm1811-fm",
		.playback = {
			.stream_name = "Playback",
			.channels_min = 1,
			.channels_max = 1,
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
					SNDRV_PCM_FMTBIT_S24_LE },
		.capture = {
			.stream_name = "Capture",
			.channels_min = 1,
			.channels_max = 1,
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
					SNDRV_PCM_FMTBIT_S24_LE },
		.ops = NULL,
		.symmetric_rates = 1,
	},
};

void wm1811_set_soc_controls(struct snd_kcontrol_new *controls,
				u_int array_size)
{
	wm1811_log_efunc("controls[0x%p] array_size[%d]",
			controls, array_size);

	wm1811_controls = controls;
	wm1811_array_size = array_size;

	wm1811_log_rfunc("");
}
EXPORT_SYMBOL(wm1811_set_soc_controls);

static int wm1811_i2c_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	int ret = 0;
	struct wm1811_priv *dev = NULL;
	wm1811_log_efunc("client[0x%p] id->driver_data[%ld]",
			client, id->driver_data);

	if (NULL == wm1811_conf) {
		dev = kzalloc(sizeof(struct wm1811_priv), GFP_KERNEL);

		if (NULL == dev) {
			ret = -ENOMEM;
			wm1811_log_err("Could not allocate master. ret[%d]",
					ret);
			return ret;
		}
		wm1811_conf = dev;
	} else {
		dev = wm1811_conf;
	}

	wm1811_log_info("client->addr[0x%02X]", client->addr);

	if  (WM1811_SLAVE_ADDR == client->addr) {
		dev->client_wm1811 = client;
	} else {
		ret = -EINVAL;
		wm1811_log_err(
			"invalid I2C address specified.ret[%d]addr[%02X]",
			ret, client->addr);
		goto err_slave_addr;
	}

	i2c_set_clientdata(client, dev);

	ret = snd_soc_register_codec(&client->dev,
				     &soc_codec_dev_wm1811,
				     wm1811_dai_driver,
				     ARRAY_SIZE(wm1811_dai_driver));

	if (ret)
		wm1811_log_err("Failed to register codec: %d\n", ret);

	wake_lock_init(&wm1811_hp_irq_wake_lock, WAKE_LOCK_SUSPEND,
			"wm1811_hp_irq_wake_lock");

	ret = wm1811_setup(client, dev);

	if (0 != ret)
		goto err_setup;

	ret = wm1811_enable_vclk4();
	ret = wm1811_write(0x0200, 0x0011);
	ret = wm1811_write(0x0208, 0x000F); /* FIXME use MCLK2 */
	ret = wm1811_write(0x0204, 0x0009);
	mdelay(1);
	ret = wm1811_write(0x0200, 0x0011);
	ret = wm1811_write(0x0204, 0x0009);
	ret = wm1811_write(0x0208, 0x000F); /* FIXME use MCLK2 */
	ret = wm1811_disable_vclk4();
	if (0 != ret)
		goto err_setup;

	wm1811_log_rfunc("ret[%d]", ret);
	return ret;

err_setup:
err_slave_addr:

	if (NULL != dev->client_wm1811)
		kfree(i2c_get_clientdata(dev->client_wm1811));

	kfree(dev);
	wm1811_log_err("ret[%d]", ret);
	return ret;
}

static int wm1811_i2c_remove(struct i2c_client *client)
{
	int ret = 0;
	wm1811_log_efunc("");

	snd_soc_unregister_codec(&client->dev);
	kfree(i2c_get_clientdata(client));

	if (wm1811_conf->log_entry)
		remove_proc_entry(WM1811_LOG_LEVEL,
				wm1811_conf->proc_parent);

	if (wm1811_conf->dump_entry)
		remove_proc_entry(WM1811_DUMP_REG,
				wm1811_conf->proc_parent);

	if (wm1811_conf->proc_parent)
		remove_proc_entry(AUDIO_LSI_DRV_NAME, NULL);

	wake_lock_destroy(&wm1811_hp_irq_wake_lock);

	gpio_free(GPIO_PORT29);
	kfree(wm1811_conf);

	wm1811_log_rfunc("ret[%d]", ret);
	return ret;
}

int __init wm1811_init(void)
{
	int ret = 0;
	wm1811_log_efunc("");

	if (D2153_INTRODUCE_BOARD_REV <= u2_get_board_rev())
		return -ENODEV;

	ret = i2c_add_driver(&wm1811_i2c_driver);

	wm1811_log_rfunc("ret[%d]", ret);
	return ret;
}

void __exit wm1811_exit(void)
{
	wm1811_log_efunc("");

	if (D2153_INTRODUCE_BOARD_REV <= u2_get_board_rev())
		return;

	i2c_del_driver(&wm1811_i2c_driver);

	wm1811_log_rfunc("");
}


module_init(wm1811_init);
module_exit(wm1811_exit);

MODULE_LICENSE("GPL v2");
