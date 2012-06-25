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

#define WM1811_BIAS_VMID_ENABLE	0x0003

#define WM1811_SPEAKER_ENABLE	0x3000
#define WM1811_SPEAKER_VOL	0x0300

#define WM1811_EARPEACE_ENABLE	0x0800
#define WM1811_EARPEACE_VOL	0x000F

#define WM1811_HEADPHONE_ENABLE	0x0300
#define WM1811_HEADPHONE_VOL	0x0030

#define WM1811_MIC_ENABLE	0x0030

#define WM1811_GPIO_BASE	IO_ADDRESS(0xE6050000)
#define WM1811_GPIO_024		(WM1811_GPIO_BASE + 0x0018)/* EAR_SEND_END */
#define WM1811_GPIO_034		(WM1811_GPIO_BASE + 0x0022)/* CODEC_LDO_EN */

/*---------------------------------------------------------------------------*/
/* define function macro declaration (private)                               */
/*---------------------------------------------------------------------------*/
/* none */

/*---------------------------------------------------------------------------*/
/* enum declaration (private)                                                */
/*---------------------------------------------------------------------------*/
/* none */

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
	/* i2c */
	struct i2c_client *client_wm1811;   /**< i2c driver wm1811 client. */
	/* irq */
	u_int irq;                          /**< irq number. */
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
};

/*---------------------------------------------------------------------------*/
/* prototype declaration (private)                                           */
/*---------------------------------------------------------------------------*/
static int wm1811_enable_vclk4(void);

static int wm1811_setup(struct i2c_client *client,
			struct wm1811_priv *dev);
static int wm1811_setup_r8a73734(void);
static int wm1811_setup_wm1811(void);

static int wm1811_setup_proc(struct wm1811_priv *dev);
static int wm1811_switch_dev_register(struct wm1811_priv *dev);
static int wm1811_create_proc_entry(char *name,
					struct proc_dir_entry **proc_child);

static void wm1811_irq_work_func(struct work_struct *unused);
static irqreturn_t wm1811_irq_handler(int irq, void *data);

static int wm1811_conv_device_info(const u_long device,
				struct wm1811_info *device_info);

static int wm1811_check_device(const u_long device);

static int wm1811_set_device_active(const struct wm1811_info new_device);
static int wm1811_set_speaker_device(const u_int cur_dev,
				const u_int new_dev);
static int wm1811_set_earpiece_device(const u_int cur_dev,
				const u_int new_dev);
static int wm1811_set_headphone_device(const u_int cur_dev,
				const u_int new_dev);
static int wm1811_set_mic_device(const u_int cur_dev, const u_int new_dev);
static int wm1811_set_headset_mic_device(const u_int cur_dev,
				const u_int new_dev);

#ifdef __WM1811_DEBUG__
static void wm1811_dump_wm1811_priv(void);
#endif /* __WM1811_DEBUG__ */

static int wm1811_i2c_read_device(struct i2c_client *wm1811,
				unsigned short reg,
				int bytes,
				void *dest);
#if 0	/* Interim support i2c driver problem ---->>> */
static int wm1811_i2c_write_device(struct i2c_client *wm1811,
				unsigned short reg,
				int bytes,
				const void *src);
#else	/* Interim support i2c driver problem ---- */
static int wm1811_i2c_write_device(struct i2c_client *wm1811,
				unsigned short reg,
				int bytes /* not use */,
				u_short *src);
#endif	/* Interim support i2c driver problem ----<<< */

static int wm1811_proc_log_read(char *page, char **start, off_t offset,
			int count, int *eof, void *data);
static int wm1811_proc_log_write(struct file *filp, const char *buffer,
			unsigned long count, void *data);
static int wm1811_proc_dump_read(char *page, char **start, off_t offset,
			int count, int *eof, void *data);
static int wm1811_proc_dump_write(struct file *filp, const char *buffer,
			unsigned long count, void *data);

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
static u_int wm1811_log_level = WM1811_LOG_NO_PRINT;
//static u_int wm1811_log_level = (WM1811_LOG_ERR_PRINT |
//				   WM1811_LOG_PROC_PRINT |
//				   WM1811_LOG_FUNC_PRINT
//				   | WM1811_LOG_DEBUG_PRINT
//				  );
#else   /* __WM1811_DEBUG__ */
static u_int wm1811_log_level = WM1811_LOG_NO_PRINT;
#endif  /* __WM1811_DEBUG__ */

/*!
  @brief Store the AudioLSI driver config.
*/
static struct wm1811_priv *wm1811_conf;

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
	struct clk *main_clk = NULL;
	struct clk *vclk4_clk = NULL;
	wm1811_log_efunc("");
	ret = gpio_request(GPIO_FN_VIO_CKO4, NULL);

	if (0 != ret) {
		wm1811_log_err("gpio_request() ret[%d]", ret);
		goto err_gpio_request;
	}

	vclk4_clk = clk_get(NULL, "vclk4_clk");

	if (IS_ERR(vclk4_clk)) {
		ret = IS_ERR(vclk4_clk);
		wm1811_log_err("clk_get(vclk4_clk) ret[%d]", ret);
		goto err_gpio_request;
	}

	main_clk = clk_get(NULL, "main_clk");

	if (IS_ERR(main_clk)) {
		ret = IS_ERR(main_clk);
		wm1811_log_err("clk_get(main_clk) ret[%d]", ret);
		goto err_gpio_request;
	}

	/* vclk4_clk */
	ret = clk_set_parent(vclk4_clk, main_clk);

	if (0 != ret) {
		wm1811_log_err("clk_set_parent() ret[%d]", ret);
		goto err_gpio_request;
	}

	ret = clk_set_rate(vclk4_clk, 26000000);

	if (0 != ret) {
		wm1811_log_err("clk_set_rate() ret[%d]", ret);
		goto err_gpio_request;
	}

	ret = clk_enable(vclk4_clk);

	if (0 != ret) {
		wm1811_log_err("clk_enable() ret[%d]", ret);
		goto err_gpio_request;
	}

	clk_put(vclk4_clk);
	wm1811_log_rfunc("ret[%d]", ret);
	return ret;
err_gpio_request:
	wm1811_log_err("ret[%d]", ret);
	return ret;
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
			  (IRQF_DISABLED | IRQF_TRIGGER_FALLING),
			  client->name, NULL);

	if (0 != ret) {
		wm1811_log_err("irq request err. ret[%d]", ret);
		goto err_request_irq;
	}

	/***********************************/
	/* setup wm1811                  */
	/***********************************/
	ret = wm1811_setup_wm1811();

	if (0 != ret)
		goto err_setup_wm1811;

	wm1811_log_rfunc("ret[%d]", ret);
	return ret;

err_setup_wm1811:
err_request_irq:
err_create_singlethread_workqueue:
	if (dev->irq_workqueue)
		destroy_workqueue(dev->irq_workqueue);
err_setup_r8a73734:
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
	ret = wm1811_enable_vclk4();

	if (0 != ret)
		goto err_enable_vclk4;

	iowrite8(0xe0, WM1811_GPIO_024);
	iowrite8(0xe0, WM1811_GPIO_034);

	wm1811_log_info("not supported yet.");
	wm1811_log_rfunc("ret[%d]", ret);
	return ret;

err_enable_vclk4:
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
	wm1811_log_info("not supported yet.");
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
	wm1811_log_efunc("");

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

	switch (mic_detect3) {
	case 0x402:
		wm1811_log_info("jack remove.");
		state = 0x00;
		break;
	case 0x0B:
		wm1811_log_info("jack insert.");
		wm1811_log_info("mic.");
		state = 0x01;
		break;
	case 0x07:
		wm1811_log_info("jack insert.");
		wm1811_log_info("no mic.");
		state = 0x02;
		break;
	default:
		wm1811_log_info("jack detect error.");
		break;
	}

	if (wm1811_conf->switch_data.state != state) {
		wm1811_log_info("notify jack state[%d]", state);
		switch_set_state(&wm1811_conf->switch_data.sdev, state);
		wm1811_conf->switch_data.state = state;
	}

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
	wm1811_log_efunc("device[%ld]", device);

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

	wm1811_log_rfunc("");
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
  @brief active the device.

  @param[i] new_device  device information.

  @return function results.
*/
static int wm1811_set_device_active(const struct wm1811_info new_device)
{
	int ret = 0;
	u_int val = 0;
	wm1811_log_efunc("new_device.raw_device[%ld]",
			new_device.raw_device);

	if (0 != new_device.raw_device) {
		/* device enabled */
	} else {
		/* device disabled */
	}

	/* wm1811_log_info("not supported yet."); */

	wm1811_log_rfunc("ret[%d] val[0x%02X]", ret, val);
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

	wm1811_log_efunc("cur_dev[%d] new_dev[%d]", cur_dev, new_dev);

	if (cur_dev != new_dev) {
		/* update device conf */
		ret = wm1811_read(0x0001, &pm);
		ret = wm1811_read(0x0003, &oe);
		if (WM1811_ENABLE == new_dev) {
			/* speaker on */
			vol = 0x0;
			vol_p = 0x3;
			oe |= WM1811_SPEAKER_VOL;
			/***********************************/
			/* Speaker Enable                  */
			/***********************************/

			if (WM1811_BIAS_VMID_ENABLE & pm)
			{
				pm |= WM1811_SPEAKER_ENABLE;
			} else {
				pm |= (WM1811_BIAS_VMID_ENABLE |
					WM1811_SPEAKER_ENABLE);
			}
		} else {
			/* speaker off */
			vol = 0x3;
			vol_p = 0x0;
			oe &= ~WM1811_SPEAKER_VOL;
			if (~(WM1811_BIAS_VMID_ENABLE |
				WM1811_SPEAKER_ENABLE) & pm) {
				pm &= ~WM1811_SPEAKER_ENABLE;
			} else {
				pm &= ~(WM1811_BIAS_VMID_ENABLE |
					WM1811_SPEAKER_ENABLE);
			}
		}

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
	wm1811_log_efunc("cur_dev[%d] new_dev[%d]", cur_dev, new_dev);

	if (cur_dev != new_dev) {
		/* update device conf */
		ret = wm1811_read(0x0001, &pm);
		ret = wm1811_read(0x0003, &oe);
		if (WM1811_ENABLE == new_dev) {
			/* earpiece on */
			hp_out2_mute = 0x0;
			mix_out_mute = 0x1;
			mix_out_vol = 0x18;
			hp_out2_mix = 0x40;
			oe |= WM1811_EARPEACE_VOL;
			if (WM1811_BIAS_VMID_ENABLE & pm)
			{
				pm |= WM1811_EARPEACE_ENABLE;
			} else {
				pm |= (WM1811_BIAS_VMID_ENABLE |
					WM1811_EARPEACE_ENABLE);
			}
		} else {
			/* earpiece off */
			hp_out2_mute = 0x20;
			mix_out_mute = 0x0;
			mix_out_vol = 0x0;
			hp_out2_mix = 0x0;
			oe &= ~WM1811_EARPEACE_VOL;
			if (~(WM1811_BIAS_VMID_ENABLE |
				WM1811_EARPEACE_VOL) & pm) {
				pm &= ~WM1811_EARPEACE_ENABLE;
			} else {
				pm &= ~(WM1811_BIAS_VMID_ENABLE |
					WM1811_EARPEACE_ENABLE);
			}
		}

		/***********************************/
		/* Earpiece Enable                 */
		/***********************************/
		/* Enable Left Output Mixer (MIXOUTL), */
		/* Enable Right Output Mixer (MIXOUTR),*/
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
		/* Unmute Right Output Mixer to HPOUT2 (Earpiece) path */
		ret = wm1811_write(0x0033, mix_out_vol);

		/* Enable HPOUT2 Mixer and Input Stage */
		ret = wm1811_write(0x0038, hp_out2_mix);

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
	u_short oe = 0;
	u_short cp_dyn_pwr = 0;
	u_short hpout1_dly = 0;
	u_short charge_pump = 0;
	u_short dac1_to_mixout =0;
	u_short dcs_ena = 0;
	u_short analog_hp = 0;
	wm1811_log_efunc("cur_dev[%d] new_dev[%d]", cur_dev, new_dev);

	if (cur_dev != new_dev) {
		/* update device conf */
		if (WM1811_ENABLE == new_dev) {
		/* update device conf */
		ret = wm1811_read(0x0001, &pm);
		ret = wm1811_read(0x0003, &oe);
		if (WM1811_ENABLE == new_dev) {
			/* earpiece on */
			cp_dyn_pwr = 0x5;
			hpout1_dly = 0x22;
			charge_pump = 0x9F25;
			dac1_to_mixout = 0x1;
			dcs_ena = 0x33;
			analog_hp = 0xEE;
			oe |= WM1811_HEADPHONE_VOL;
			if (WM1811_BIAS_VMID_ENABLE & pm)
			{
				pm |= WM1811_HEADPHONE_ENABLE;
			} else {
				pm |= (WM1811_BIAS_VMID_ENABLE |
					WM1811_HEADPHONE_ENABLE);
			}
		} else {
			/* earpiece off */
			cp_dyn_pwr = 0x4;
			hpout1_dly = 0x0;
			charge_pump = 0x1F25;
			dac1_to_mixout = 0x0;
			dcs_ena = 0;
			analog_hp = 0;
			oe &= ~WM1811_HEADPHONE_VOL;
			if (~(WM1811_BIAS_VMID_ENABLE |
				WM1811_HEADPHONE_ENABLE) & pm) {
				pm &= ~WM1811_HEADPHONE_ENABLE;
			} else {
				pm &= ~(WM1811_BIAS_VMID_ENABLE |
					WM1811_HEADPHONE_ENABLE);
			}
		}
			/* headphone on */
			/* Enable Class W, Class W Envelope Tracking = AIF1 */
			ret = wm1811_write(0x0051, cp_dyn_pwr);

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

			/* Enable Left Output Mixer (MIXOUTL), */
			/* Enable Right Output Mixer (MIXOUTR)*/
			ret = wm1811_write(0x0003, oe);

			/* Enable DC Servo and trigger start-up mode on */
			/* left and right channels */
			ret = wm1811_write(0x0054, dcs_ena);

			/* INSERT_DELAY_MS [250] */
			mdelay(250);

			/* Enable HPOUT1 (Left) and HPOUT1 (Right) */
			/* intermediate and output stages. Remove clamps */
			ret = wm1811_write(0x0060, analog_hp);

		} else {
			/* headphone off */
		}
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

  @return function results.
*/
static int wm1811_set_mic_device(const u_int cur_dev, const u_int new_dev)
{
	int ret = 0;
	u_short pm = 0;
	wm1811_log_efunc("cur_dev[%d] new_dev[%d]", cur_dev, new_dev);

	if (cur_dev != new_dev) {
		/* update device conf */
		wm1811_read(0x0001, &pm);
		if (WM1811_ENABLE == new_dev) {
			/* mic on */
			if (WM1811_BIAS_VMID_ENABLE & pm)
			{
				pm |= WM1811_MIC_ENABLE;
			} else {
				pm |= (WM1811_BIAS_VMID_ENABLE |
					WM1811_MIC_ENABLE);
			}

		} else {
			/* mic off */
			if (~(WM1811_BIAS_VMID_ENABLE |
				WM1811_HEADPHONE_VOL) & pm) {
				pm &= ~WM1811_MIC_ENABLE;
			} else {
				pm &= ~(WM1811_BIAS_VMID_ENABLE |
					WM1811_MIC_ENABLE);
			}
		}
	} else {
		/* nothing to do. */
	}

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
	wm1811_log_efunc("cur_dev[%d] new_dev[%d]", cur_dev, new_dev);

	if (cur_dev != new_dev) {
		/* update device conf */
		if (WM1811_ENABLE == new_dev) {
			/* mic on */
		} else {
			/* mic off */
		}
	} else {
		/* nothing to do. */
	}

	wm1811_log_info("not supported yet.");

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
				unsigned short reg,
				int bytes,
				void *dest)
{
	struct i2c_msg msg[2];
	u_char wbf[2];
	u_char rbf[2];
	int ret = 0;
	wm1811_log_efunc("");

	wbf[0] = (reg>>8) & 0x00ff;
	wbf[1] = (reg>>0) & 0x00ff;
	rbf[0] = 0x0;
	rbf[1] = 0x0;

	msg[0].addr  = wm1811->addr;
	msg[0].flags = wm1811->flags & I2C_M_TEN;
	msg[0].len   = sizeof(wbf);
	msg[0].buf   = wbf;

	msg[1].addr  = wm1811->addr;
	msg[1].flags =(wm1811->flags & I2C_M_TEN) | I2C_M_RD;
	msg[1].len   = sizeof(rbf);
	msg[1].buf   = rbf;

	ret = i2c_transfer(wm1811->adapter,
			   msg,
			   ARRAY_SIZE(msg));
	*(u_short *)dest = (rbf[0] << 8) + rbf[1];

	wm1811_log_rfunc("ret[%d]", ret);
	return ret;
}

#if 0	/* Interim support i2c driver problem ---->>> */
static int wm1811_i2c_write_device(struct i2c_client *wm1811,
				unsigned short reg,
				int bytes,
				const void *src)
{
	struct i2c_msg xfer[2];
	int ret;
	wm1811_log_efunc("");

	reg = cpu_to_be16(reg);

	xfer[0].addr = wm1811->addr;
	xfer[0].flags = 0;
	xfer[0].len = 2;
	xfer[0].buf = (char *)&reg;

	xfer[1].addr = wm1811->addr;
	xfer[1].flags = I2C_M_NOSTART;
	xfer[1].len = bytes;
	xfer[1].buf = (char *)src;

	ret = i2c_transfer(wm1811->adapter, xfer, 2);

	if (ret < 0) {
		wm1811_log_rfunc("ret[%d]", ret);
		return ret;
	}

	if (ret != 2) {
		ret = -EIO;
		wm1811_log_err("ret[%d] i2c_transfer()", ret);
		goto err_i2c_transfer;
	}

	ret = 0;
	wm1811_log_rfunc("ret[%d]", ret);
	return ret;

err_i2c_transfer:
	wm1811_log_rfunc("ret[%d]", ret);
	return ret;
}
#else	/* Interim support i2c driver problem ---- */
static int wm1811_i2c_write_device(struct i2c_client *wm1811,
				unsigned short reg,
				int bytes /* not use */,
				u_short *src)
{
	int ret;
	u_char w_data2[4];
	wm1811_log_efunc("");

	w_data2[0] = (reg    & 0xFF00) >> 8;
	w_data2[1] = (reg    & 0x00FF);
	w_data2[2] = (src[0] & 0xFF00) >> 8;
	w_data2[3] = (src[0] & 0x00FF);

	ret = i2c_smbus_write_i2c_block_data(
				wm1811_conf->client_wm1811,
				w_data2[0], 3, &w_data2[1]);

	wm1811_log_rfunc("ret[%d]", ret);
	return ret;
}
#endif	/* Interim support i2c driver problem ----<<< */

/*------------------------------------*/
/* for public function		*/
/*------------------------------------*/
int wm1811_set_device(const u_long device, const u_int pcm_value)
{
	int ret = 0;
	struct wm1811_info new_device = wm1811_conf->info;
	static int audio_on = WM1811_DISABLE;
	wm1811_log_efunc("device[%ld]", device);

	ret = wm1811_check_device(device);

	if (0 != ret) {
		wm1811_log_err("parameter error. ret[%d]", ret);
		return ret;
	}

	/* check update */
	if (wm1811_conf->info.raw_device == device) {
		wm1811_log_rfunc("no changed. ret[%d]", ret);
		return ret;
	}

	ret = wm1811_conv_device_info(device, &new_device);

	if (0 != ret) {
		wm1811_log_err("device convert error. ret[%d]", ret);
		return ret;
	}

	if (WM1811_DISABLE == audio_on) {
		/***********************************/
		/* Software reset                  */
		/***********************************/
		ret = wm1811_write(0x0000, 0x0000);

#if 1
		/*****************************************************/
		/* General Purpose Input/Output (GPIO) Configuration */
		/*****************************************************/
		/* GPIO 1 - Set to GPIO (Microphone Detect IRQ output) */
		ret = wm1811_write(0x0700, 0x0005);
		/* ret = wm1811_write(0x0700, 0x0003); */

		ret = wm1811_write(0x00D0, 0x0F03);
		ret = wm1811_write(0x00D1, 0x0001);
		/*ret = wm1811_write(0x00D2, 0x0001);*/

		ret = wm1811_write(0x0738, 0x07DE);
		ret = wm1811_write(0x0739, 0xDBED);
		ret = wm1811_write(0x0740, 0x0000);
#else
		/*****************************************************/
		/* General Purpose Input/Output (GPIO) Configuration */
		/*****************************************************/
		/* GPIO 1 - Set to GPIO (LRCLK1 used instead of ADCLRCLK1) */
		ret = wm1811_write(0x0700, 0x8101);
#endif

		/***********************************/
		/* Analogue Configuration          */
		/***********************************/
		/* MICBIAS1 1.8V */
		ret = wm1811_write(0x003D, 0x0033);

		/* MICBIAS2 2.6V */
		ret = wm1811_write(0x003E, 0x003F);

#if 1
		ret = wm1811_write(0x0039, 0x01E4);
#else
		/* Enable VMID soft start (fast), */
		/* Start-up Bias Current Enabled */
		ret = wm1811_write(0x0039, 0x01E4);
#endif

		/* Enable bias generator, Enable VMID */
		ret = wm1811_write(0x0001, 0x0003);

		/* INSERT_DELAY_MS [50] */
		mdelay(50);

		/***********************************/
		/* Analogue Input Configuration    */
		/***********************************/
		/* Enable IN1L Input PGA, Enable IN2R Input PGA, */
		/* Enable Left Input Mixer (MIXINL), */
		/* Enable Right Input Mixer (MIXINR) */
		ret = wm1811_write(0x0002, 0x6360);

		/* Unmute IN1L Input PGA */
		ret = wm1811_write(0x0018, 0x011F);

		/* Unmute IN2R Input PGA */
		ret = wm1811_write(0x001B, 0x011F);

		/* Connect IN1LN to IN1L PGA, Connect IN1LP to IN1L PGA */
		/* Connect IN2RN to IN2R PGA, Connect IN2RP to IN2R PGA */
		ret = wm1811_write(0x0028, 0x003C);

		/* Unmute IN1L PGA output to Left Input Mixer (MIXINL) Path */
		ret = wm1811_write(0x0029, 0x002F);

		/* Unmute IN2R PGA output to Right Input Mixer (MIXINR) Path */
		ret = wm1811_write(0x002A, 0x010F);

		/***********************************/
		/* Path Configuration              */
		/***********************************/
		/* Enable DAC1 (Left), Enable DAC1 (Right), */
		ret = wm1811_write(0x0005, 0x0303);

		/* Enable ADC (Left), Enable ADC (Right) */
		ret = wm1811_write(0x0004, 0x0303);

		/* Enable the AIF1 (Left) to DAC 1 (Left) mixer path */
		ret = wm1811_write(0x0601, 0x0001);

		/* Enable the AIF1 (Right) to DAC 1 (Right) mixer path */
		ret = wm1811_write(0x0602, 0x0001);

	        /* Enable ADC (Left) to AIF1 Timeslot 0 ADC (Left) Path */
		ret = wm1811_write(0x0606, 0x0002);

	        /* Enable ADC (Right) to AIF1 Timeslot 0 ADC (Right) Path */
		ret = wm1811_write(0x0607, 0x0002);

		/***********************************/
		/* Clocking                        */
		/***********************************/
	        /* AIF1 Sample Rate = 44.1 kHz, */
	        /* AIF1CLK/Fs ratio = 256 (Default Register Value) */
		ret = wm1811_write(0x0210, 0x0073);

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
		/* FLL1: (LR / REF_DIV[1]) * (N[128] + THETA[0]/LAMBDA) */
		/* * FRATIO[16] / OUTDIV[8] */
		/* OUTDIV=7(/8), FRATIO=100(x16) */
		ret = wm1811_write(0x0221, 0x0704);

		/* THETA=0 */
		ret = wm1811_write(0x0222, 0x0000);

		/* N=128 */
		ret = wm1811_write(0x0223, 0x1000);

		/* BYP=0, non-free, REF_DIV=0(/1), SRC=2(LRCLK) */
		ret = wm1811_write(0x0224, 0x0002);

		/* FLL_ENA=1, FLL_OSC_ENA=0 */
		ret = wm1811_write(0x0220, 0x0001);

		/* INSERT_DELAY_MS [5] */
		mdelay(5);

		/* AIF1CLK_SRC=2(FLL1), INV=0, DIV=/1, AIF1CLK=enable */
		ret = wm1811_write(0x0200, 0x0011);

		/***********************************/
		/* FFL2                            */
		/***********************************/
		/* FLL2: (MCLK / REF_DIV[2]) * */
		/* (N[7] + THETA[391h]/LAMBDA[659h]) */
		/* * FRATIO[1] / OUTDIV[8] */
		/* OUTDIV=7(/8), FRATIO=000(x1) */
		ret = wm1811_write(0x0241, 0x0700);

		/* THETA=391h(913d) */
		ret = wm1811_write(0x0242, 0x0391);

		/* N=7 */
		ret = wm1811_write(0x0243, 0x00E0);

		/* BYP=0, non-free, REF_DIV=1(/2), SRC=0(MCLK1) */
		ret = wm1811_write(0x0244, 0x0008);

		/* LAMBDA=659h(1625d) */
		ret = wm1811_write(0x0246, 0x0659);

		/* EFS_ENA=1 */
		ret = wm1811_write(0x0247, 0x0007);

		/* FLL_ENA=1, FLL_OSC_ENA=0 */
		ret = wm1811_write(0x0240, 0x0001);

		/* INSERT_DELAY_MS [5] */
		mdelay(5);

		/* AIF2CLK_SRC=3(FLL2), INV=0, DIV=/1, AIF2CLK=enable */
		ret = wm1811_write(0x0204, 0x0019);
		/***********************************/
		/* Headphone Enable                */
		/***********************************/
		/* Enable Class W, Class W Envelope Tracking = AIF1 */
		ret = wm1811_write(0x0051, 0x0005);

		/* Enable bias generator, Enable VMID, */
		/* Enable HPOUT1 (Left) and Enable HPOUT1 (Right) */
		/* input stages */
		ret = wm1811_write(0x0001, 0x0333);

		/* Enable HPOUT1 (Left) and HPOUT1 (Right) */
		/* intermediate stages */
		ret = wm1811_write(0x0060, 0x0022);

		/* Enable Charge Pump */
		ret = wm1811_write(0x004C, 0x9F25);

		/* INSERT_DELAY_MS [15]->[25] */
		mdelay(25);

		/* Enable DAC1 (Left), Enable DAC1 (Right), */
		ret = wm1811_write(0x0005, 0x0303);

		/* Enable AIF1 DAC (Left) Path, Enable AIF1 DAC (Right) Path */
		/* Select DAC1 (Left) to Left Headphone Output PGA */
		/* (HPOUT1LVOL) path */
		ret = wm1811_write(0x002D, 0x0001);

		/* Select DAC1 (Right) to Right Headphone Output */
		/* PGA (HPOUT1RVOL) path */
		ret = wm1811_write(0x002E, 0x0001);

		/* Enable Left Output Mixer (MIXOUTL), */
		/* Enable Right Output Mixer (MIXOUTR)*/
		ret = wm1811_write(0x0003, 0x0030);

		/* Enable DC Servo and trigger start-up mode on */
		/* left and right channels */
		ret = wm1811_write(0x0054, 0x0033);

		/* INSERT_DELAY_MS [250] */
		mdelay(250);

		/* Enable HPOUT1 (Left) and HPOUT1 (Right) */
		/* intermediate and output stages. Remove clamps */
		ret = wm1811_write(0x0060, 0x00EE);
		ret = wm1811_write(0x0003, 0x00C0);
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

/*
	ret = wm1811_set_mic_device(wm1811_conf->info.mic,
					      new_device.mic);

		if (0 != ret)
			goto err_set_device;

	ret = wm1811_set_headset_mic_device(wm1811_conf->info.headset_mic,
					      new_device.headset_mic);

		if (0 != ret)
			goto err_set_device;

*/

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

	ret = wm1811_set_device_active(new_device);
		if (0 != ret)
			goto err_set_device;

	wm1811_conf->info = new_device;
	wm1811_log_rfunc("ret[%d]", ret);
	return ret;
err_set_device:
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

#ifdef __WM1811_DEBUG__
	wm1811_dump_wm1811_priv();
#endif /* __WM1811_DEBUG__ */

	for (i = 0; i < WM1811_CACHE_SIZE; i++) {
		if (0x0000 == wm1811_access_masks[i].readable) {
			wm1811_log_debug("addr[0x%02X] write-only register",
					i);
		} else {
			ret = wm1811_read(i, &val);

			if (0 > ret)
				goto err_i2c_read;

			wm1811_log_debug("ret[%d] addr[0x%02X] val[0x%02X]",
					ret, i, val);
		}
	}

	wm1811_log_debug("ret[%d]", ret);
	return ret;
err_i2c_read:
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
	wm1811_log_info("WM1811 addr[0x%02X] value[0x%02X]", addr, *value);

	wm1811_log_debug("ret[%d]", ret);
	return ret;
}
EXPORT_SYMBOL(wm1811_read);

int wm1811_write(const u_short addr, const u_short value)
{
	int ret = 0;
	wm1811_log_debug("");

	wm1811_log_info("WM1811 addr[0x%02X] value[0x%02X]", addr, value);

#if 0	/* Interim support i2c driver problem ---->>> */
	ret = wm1811_i2c_write_device(wm1811_conf->client_wm1811,
					addr, 2, &value);
#else	/* Interim support i2c driver problem ---- */
	ret = wm1811_i2c_write_device(wm1811_conf->client_wm1811,
					addr, 2, (u_short *)&value);
#endif	/* Interim support i2c driver problem ----<<< */

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
	unsigned long count, void *data)
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

#if 1
	switch (in) {
	case 0:
		wm1811_set_device(WM1811_DEV_PLAYBACK_SPEAKER, 0);
		break;
	case 1:
		wm1811_set_device(WM1811_DEV_PLAYBACK_EARPIECE, 0);
		break;
	case 2:
		wm1811_set_device(WM1811_DEV_PLAYBACK_HEADPHONES, 0);
		break;
	default:
		break;
	}
#else
	wm1811_log_level = (u_int)in;
#endif
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
	unsigned long count, void *data)
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
		.name = "wm1811-fm",
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
EXPORT_SYMBOL(wm1811_dai_driver);

static void wm1811_set_soc_controls(struct snd_kcontrol_new *controls,
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

	if (ret) {
		wm1811_log_err("Failed to register codec: %d\n", ret);
	}

	ret = wm1811_setup(client, dev);

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

	gpio_free(GPIO_PORT29);
	kfree(wm1811_conf);

	wm1811_log_rfunc("ret[%d]", ret);
	return ret;
}

int __init wm1811_init(void)
{
	int ret = 0;
	wm1811_log_efunc("");

	ret = i2c_add_driver(&wm1811_i2c_driver);

	wm1811_log_rfunc("ret[%d]", ret);
	return ret;
}

void __exit wm1811_exit(void)
{
	wm1811_log_efunc("");

	i2c_del_driver(&wm1811_i2c_driver);

	wm1811_log_rfunc("");
}


module_init(wm1811_init);
module_exit(wm1811_exit);

MODULE_LICENSE("GPL v2");
