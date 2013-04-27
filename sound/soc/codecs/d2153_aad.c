/*
 * D2153 ALSA SoC Audio Accessory Detection driver
 *
 * Copyright (c) 2012 Dialog Semiconductor
 *
 * Written by Adam Thomson <Adam.Thomson.Opensource@diasemi.com>
 * 
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */
 
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <mach/common.h>
#include <sound/soc.h>
#include <sound/jack.h>
#include <sound/soundpath/soundpath.h>
#include <linux/gpio.h>
#include <mach/r8a7373.h>

//#define D2153_READ_PMIC_BUTTON_REG

/* PMIC related includes */
#include <linux/d2153/core.h>
#include <linux/d2153/d2153_reg.h>
 
#include <linux/d2153/d2153_codec.h>
#include <linux/d2153/d2153_aad.h>
 
#define D2153_AAD_MICBIAS_SETUP_TIME 50
#ifdef D2153_JACK_DETECT
 #ifdef D2153_AAD_MICBIAS_SETUP_TIME
 #define D2153_AAD_JACK_DEBOUNCE_MS (400 - D2153_AAD_MICBIAS_SETUP_TIME)
 #else
 #define D2153_AAD_JACK_DEBOUNCE_MS 400
 #endif
#else
#define D2153_AAD_JACK_DEBOUNCE_MS 400
#endif

#define D2153_AAD_JACKOUT_DEBOUNCE_MS 100

#define D2153_AAD_BUTTON_DEBOUNCE_MS 50
#define D2153_AAD_DETECT_JACK_ADC 90
#define D2153_AAD_CONNER_CASE_ADC 239

#define D2153_AAD_WATERDROP

struct d2153_aad_priv *d2153_aad_ex;
EXPORT_SYMBOL(d2153_aad_ex);


/*
 * Samsung defined resistances for 4-pole key presses:
 * 
 * 0 - 108 Ohms:	Send Button
 * 139 - 270 Ohms:	Vol+ Button
 * 330 - 680 Ohms:	Vol- Button
 */
 
/* Button resistance lookup table */
/* DLG - ADC values need to be correctly set as they're currently not known */
static const struct button_resistance button_res_2V6_tbl[MAX_BUTTONS] = {
	[SEND_BUTTON] = {
		.min_val = 0,
		.max_val = 12,
	},
	[VOL_UP_BUTTON] = {
		.min_val = 17,
		.max_val = 35,
	},
	[VOL_DN_BUTTON] = {
		.min_val = 39,
		.max_val = 86,
	},
};

static const struct button_resistance button_res_2V5_tbl[MAX_BUTTONS] = {
	[SEND_BUTTON] = {
		.min_val = 0,
		.max_val = 11,
	},
	[VOL_UP_BUTTON] = {
		.min_val = 17,
		.max_val = 34,
	},
	[VOL_DN_BUTTON] = {
		.min_val = 39,
		.max_val = 85,
	},
};

static const struct button_resistance *button_res_tbl;

/* Following register access methods based on soc-cache code */
static int d2153_aad_read(struct i2c_client *client, u8 reg)
{
//	struct d2153_aad_priv *d2153_aad = i2c_get_clientdata(client);
	
	struct i2c_msg xfer[2];
	u8 data;
	int ret;
	
//	mutex_lock(&d2153_aad->d2153_codec->d2153_pmic->d2153_io_mutex);
	
	/* Write register */
	xfer[0].addr = client->addr;
	xfer[0].flags = 0;
	xfer[0].len = 1;
	xfer[0].buf = &reg;

	/* Read data */
	xfer[1].addr = client->addr;
	xfer[1].flags = I2C_M_RD;
	xfer[1].len = 1;
	xfer[1].buf = &data;

	ret = i2c_transfer(client->adapter, xfer, 2);

//	mutex_unlock(&d2153_aad->d2153_codec->d2153_pmic->d2153_io_mutex);
	
	if (ret == 2)
		return data;
	else if (ret < 0)
		return ret;
	else
		return -EIO;
}

int d2153_aad_write(struct i2c_client *client, u8 reg, u8 value)
{
//	struct d2153_aad_priv *d2153_aad = i2c_get_clientdata(client);
	u8 data[2];
	int ret;

//	mutex_lock(&d2153_aad->d2153_codec->d2153_pmic->d2153_io_mutex);
	
    reg &= 0xff;
    data[0] = reg;
    data[1] = value & 0xff;

	ret = i2c_master_send(client, data, 2);

//	mutex_unlock(&d2153_aad->d2153_codec->d2153_pmic->d2153_io_mutex);
	
	if (ret == 2)
		return 0;
	else if (ret < 0)
		return ret;
	else
		return -EIO;
}
EXPORT_SYMBOL(d2153_aad_write);

int d2153_aad_update_bits(struct i2c_client *client, u8 reg, u8 mask,
				 u8 value)
{
	int change;
	unsigned int old, new;
	int ret;

	ret = d2153_aad_read(client, reg);
	if (ret < 0)
		return ret;

	old = ret;
	new = (old & ~mask) | value;
	change = old != new;
	if (change) {
		ret = d2153_aad_write(client, reg, new);
		if (ret < 0)
			return ret;
	}

	return change;
}
EXPORT_SYMBOL(d2153_aad_update_bits);

/* IRQ handler for HW jack detection */
static irqreturn_t d2153_jack_handler(int irq, void *data)
{
	struct d2153_aad_priv *d2153_aad = data;

	wake_lock_timeout(&d2153_aad->wakeup, HZ * 10);
	cancel_delayed_work_sync(&d2153_aad->jack_monitor_work);

	if ( d2153_aad->switch_data.state == D2153_NO_JACK)
 	schedule_delayed_work(&d2153_aad->jack_monitor_work, msecs_to_jiffies(D2153_AAD_JACK_DEBOUNCE_MS));
 	else
	 	schedule_delayed_work(&d2153_aad->jack_monitor_work, msecs_to_jiffies(D2153_AAD_JACKOUT_DEBOUNCE_MS)); 	
 	
 	
	d2153_aad->switch_data.state = D2153_NO_JACK;
	
	return IRQ_HANDLED;
}

static irqreturn_t d2153_button_handler(int irq, void *data)
{
	struct d2153_aad_priv *d2153_aad = data;

	wake_lock_timeout(&d2153_aad->wakeup, HZ * 10);

	if (D2153_AA_Silicon == d2153_aad->chip_rev) 
	{
		if(d2153_aad->button.status == D2153_BUTTON_IGNORE)
		{
			d2153_aad->button.status = D2153_BUTTON_PRESS;
			return IRQ_HANDLED;
		}

		if(d2153_aad->button.status == D2153_BUTTON_PRESS)
		{
			d2153_aad->button.status = D2153_BUTTON_RELEASE;
			return IRQ_HANDLED;
		}
	}
	
	cancel_delayed_work_sync(&d2153_aad->button_monitor_work);
	schedule_delayed_work(&d2153_aad->button_monitor_work, msecs_to_jiffies(D2153_AAD_BUTTON_DEBOUNCE_MS));
	
	return IRQ_HANDLED;
}

static int d2153_switch_dev_register(struct d2153_aad_priv *d2153_aad)
{
	int ret = 0;

	d2153_aad->switch_data.sdev.name = "h2w";
	d2153_aad->switch_data.sdev.state = 0;
	d2153_aad->switch_data.state = 0;
	d2153_aad->button.status=D2153_BUTTON_RELEASE;
	ret = switch_dev_register(&d2153_aad->switch_data.sdev);
	
	return ret;
}

static int d2153_hooksw_dev_register(struct i2c_client *client,
				struct d2153_aad_priv *d2153_aad)
{
	int ret = 0;

	d2153_aad->input_dev = input_allocate_device();
	d2153_aad->input_dev->name = "d2153-aad";
	d2153_aad->input_dev->id.bustype = BUS_I2C;
	d2153_aad->input_dev->dev.parent = &client->dev;

	__set_bit(EV_KEY, d2153_aad->input_dev->evbit);
	__set_bit(KEY_VOLUMEUP, d2153_aad->input_dev->keybit);
	__set_bit(KEY_VOLUMEDOWN, d2153_aad->input_dev->keybit);
	__set_bit(KEY_MEDIA, d2153_aad->input_dev->keybit);
	ret = input_register_device(d2153_aad->input_dev);

	if (0 != ret) {
		input_free_device(d2153_aad->input_dev);
	}

	return ret;
}

#ifdef D2153_JACK_DETECT
static void d2153_aad_jack_monitor_timer_work(struct work_struct *work)
{
	struct d2153_aad_priv *d2153_aad = container_of(work, 
									struct d2153_aad_priv, 
									jack_monitor_work.work);

	struct i2c_client *client = d2153_aad->i2c_client;
	u8 jack_mode,btn_status;
	int state = d2153_aad->switch_data.state;
#ifdef D2153_AAD_WATERDROP	
	int state_gpio;
#endif

	dlg_info("%s \n",__func__);
	
	if(d2153_aad->d2153_codec == NULL || d2153_aad->d2153_codec->codec_init ==0)
	{
		schedule_delayed_work(&d2153_aad->jack_monitor_work, msecs_to_jiffies(300));
		return;
	}

	snd_soc_update_bits(d2153_aad->d2153_codec->codec,
			D2153_MICBIAS1_CTRL, D2153_MICBIAS_EN,D2153_MICBIAS_EN);	
	msleep(D2153_AAD_MICBIAS_SETUP_TIME);
	
	jack_mode = d2153_aad_read(client, D2153_ACCDET_CFG3);	
	dlg_info(" %s, JACK MODE = 0x%x \n",__func__,jack_mode);
	
	if (jack_mode & D2153_ACCDET_JACK_MODE_JACK) {

#ifdef D2153_AAD_WATERDROP
		state_gpio = gpio_get_value(GPIO_PORT7);
		if (state_gpio == 1) 
		{
			dlg_info(" %s, state_gpio = 0x%x \n",__func__,state_gpio);
			snd_soc_update_bits(d2153_aad->d2153_codec->codec,
				D2153_MICBIAS1_CTRL, D2153_MICBIAS_EN,0);	
			schedule_delayed_work(&d2153_aad->jack_monitor_work, msecs_to_jiffies(500));
			return;
		}
#endif	 
		if (jack_mode & D2153_ACCDET_JACK_MODE_MIC) {
			d2153_unmask_irq(d2153_aad->d2153_codec->d2153_pmic, D2153_IRQ_EACCDET);
			dlg_info("%s d2153_unmask_irq - D2153_IRQ_EACCDET \n",__func__);
			
			dlg_info("%s JACK MODE! 4 Pole Heaset set \n",__func__);
			state=D2153_HEADSET;	
			//snd_soc_update_bits(d2153_aad->d2153_codec->codec,
				//D2153_MICBIAS1_CTRL, D2153_MICBIAS_EN,D2153_MICBIAS_EN);	
			d2153_aad_write(client,D2153_ACCDET_CONFIG,0x88);
		}	
		else {
			
				d2153_aad_write(client,D2153_ACCDET_CONFIG,0x88);
	
				//snd_soc_update_bits(d2153_aad->d2153_codec->codec,
					//D2153_MICBIAS1_CTRL, D2153_MICBIAS_EN,D2153_MICBIAS_EN);	
					
				if (D2153_AA_Silicon == d2153_aad->chip_rev)
					d2153_aad_write(client,D2153_ACCDET_CFG4,0x17);

				msleep(d2153_aad->button_detect_rate);
				
				btn_status = d2153_aad_read(client, D2153_ACCDET_STATUS);	
				dlg_info("%s Heaset set ADC= %d  \n",__func__,btn_status);

				if (D2153_AA_Silicon == d2153_aad->chip_rev) 
					d2153_aad_update_bits(client, D2153_ACCDET_CFG4,
						  D2153_ACCDET_ADC_COMP_OUT_INV, 0);

				//snd_soc_update_bits(d2153_aad->d2153_codec->codec,
					//D2153_MICBIAS1_CTRL, D2153_MICBIAS_EN,0);	
				if(btn_status < D2153_AAD_DETECT_JACK_ADC)	{
					dlg_info("%s ADC CHECK!! 3 Pole Heaset set2 \n",__func__);
					d2153_aad_write(client,D2153_ACCDET_CONFIG,0x08);
					state=D2153_HEADPHONE;
					snd_soc_update_bits(d2153_aad->d2153_codec->codec,
						D2153_MICBIAS1_CTRL, D2153_MICBIAS_EN,0);	
				}
				else {
					d2153_unmask_irq(d2153_aad->d2153_codec->d2153_pmic, D2153_IRQ_EACCDET);
					dlg_info("%s d2153_unmask_irq - D2153_IRQ_EACCDET \n",__func__);
					dlg_info("%s ADC CHECK!! 4 Pole Heaset set2 \n",__func__);
					state=D2153_HEADSET;	
					//snd_soc_update_bits(d2153_aad->d2153_codec->codec,
						//D2153_MICBIAS1_CTRL, D2153_MICBIAS_EN,D2153_MICBIAS_EN);	
 					d2153_aad_write(client,D2153_ACCDET_CONFIG,0x88);	
				}
		}
	}
	else {		
		d2153_mask_irq(d2153_aad->d2153_codec->d2153_pmic, D2153_IRQ_EACCDET);
		dlg_info("%s d2153_unmask_irq - D2153_IRQ_EACCDET \n",__func__);
		dlg_info("%s Jack Pull Out ! \n",__func__);
		state=D2153_NO_JACK;
		snd_soc_update_bits(d2153_aad->d2153_codec->codec,
				D2153_MICBIAS1_CTRL, D2153_MICBIAS_EN,0);
		d2153_aad_write(client,D2153_ACCDET_CONFIG,0x88);		
	}

	d2153_aad->button.status = D2153_BUTTON_RELEASE;
	
	if (d2153_aad->switch_data.state != state || d2153_aad->switch_data.state == D2153_NO_JACK) {	
		dlg_info("%s Jack state = %d\n",__func__, state);
		d2153_aad->d2153_codec->switch_state=state;
		d2153_aad->switch_data.state = state;
		switch_set_state(&d2153_aad->switch_data.sdev, state);
	}

}
#else
static void d2153_aad_jack_monitor_timer_work(struct work_struct *work)
{
	struct d2153_aad_priv *d2153_aad = container_of(work, 
									struct d2153_aad_priv, 
									jack_monitor_work.work);

	struct i2c_client *client = d2153_aad->i2c_client;
	u8 jack_mode,btn_status;
	int state = d2153_aad->switch_data.state,state_gpio;

	dlg_info("%s disable D2153_JACK_DETECT\n",__func__);
	if(d2153_aad->d2153_codec == NULL || d2153_aad->d2153_codec->codec_init ==0)
	{
		schedule_delayed_work(&d2153_aad->jack_monitor_work, msecs_to_jiffies(300));
		return;
	}
	
	state_gpio = gpio_get_value(GPIO_PORT7);
	if(state_gpio == 0 && state != D2153_NO_JACK)
		return;                 

	if (state_gpio == 0) {
		
		jack_mode = d2153_aad_read(client, D2153_ACCDET_CFG3);	

		if (jack_mode & D2153_ACCDET_JACK_MODE_MIC) {
			dlg_info("[%s] 4 Pole Heaset set \n",__func__);
			state=D2153_HEADSET;	
			snd_soc_update_bits(d2153_aad->d2153_codec->codec,
				D2153_MICBIAS1_CTRL, D2153_MICBIAS_EN,D2153_MICBIAS_EN);	
			d2153_aad_write(client,D2153_ACCDET_CONFIG,0x80);
		}	
		else {
			d2153_aad_write(client,D2153_ACCDET_CONFIG,0x80);

			snd_soc_update_bits(d2153_aad->d2153_codec->codec,
				D2153_MICBIAS1_CTRL, D2153_MICBIAS_EN,D2153_MICBIAS_EN);
				
			if (D2153_AA_Silicon == d2153_aad->chip_rev)
				d2153_aad_write(client,D2153_ACCDET_CFG4,0x17);

			msleep(d2153_aad->button_detect_rate);

			btn_status = d2153_aad_read(client, D2153_ACCDET_STATUS);	
			dlg_info("%s Heaset set ADC= %d  \n",__func__,btn_status);

			if (D2153_AA_Silicon == d2153_aad->chip_rev) 
				d2153_aad_update_bits(client, D2153_ACCDET_CFG4,
					  D2153_ACCDET_ADC_COMP_OUT_INV, 0);

			if(btn_status < D2153_AAD_DETECT_JACK_ADC)	{
				dlg_info("[%s] First 3 Pole Heaset set2 \n",__func__);

			snd_soc_update_bits(d2153_aad->d2153_codec->codec,
				D2153_MICBIAS1_CTRL, D2153_MICBIAS_EN,0);	
				
				d2153_aad_write(client,D2153_ACCDET_CONFIG,0x00);
				state=D2153_HEADPHONE;
			}
			else {
				dlg_info("[%s] First 4 Pole Heaset set2 \n",__func__);
				state=D2153_HEADSET;	
				//snd_soc_update_bits(d2153_aad->d2153_codec->codec,
				//	D2153_MICBIAS1_CTRL, D2153_MICBIAS_EN,D2153_MICBIAS_EN);	
				d2153_aad_write(client,D2153_ACCDET_CONFIG,0x80);			
			}	
		}
	}
	else {		
		dlg_info("%s Jack Pull Out ! \n",__func__);
		state=D2153_NO_JACK;
		snd_soc_update_bits(d2153_aad->d2153_codec->codec,
				D2153_MICBIAS1_CTRL, D2153_MICBIAS_EN,0);
		d2153_aad_write(client,D2153_ACCDET_CONFIG,0x00);
	}

	d2153_aad->button.status = D2153_BUTTON_RELEASE;
	
	if (d2153_aad->switch_data.state != state || d2153_aad->switch_data.state == D2153_NO_JACK) {	
		d2153_aad->d2153_codec->switch_state=state;
		d2153_aad->switch_data.state = state;
		switch_set_state(&d2153_aad->switch_data.sdev, state);
	}

	if (state == D2153_NO_JACK)
		snd_soc_dapm_disable_pin(&d2153_aad->d2153_codec->codec->dapm,
			"Headphone Enable");
	else
		snd_soc_dapm_enable_pin(&d2153_aad->d2153_codec->codec->dapm,
			"Headphone Enable");
}

#endif

static void d2153_aad_button_monitor_timer_work(struct work_struct *work)
{
	struct d2153_aad_priv *d2153_aad = container_of(work, 
									struct d2153_aad_priv, 
									button_monitor_work.work);
	struct i2c_client *client = d2153_aad->i2c_client;
	u8 btn_status;
#ifdef D2153_JACK_DETECT
	u8 jack_mode;
#endif
	int state_gpio;

	if(d2153_aad->d2153_codec == NULL || d2153_aad->d2153_codec->codec_init ==0)
	{
		schedule_delayed_work(&d2153_aad->button_monitor_work, msecs_to_jiffies(300));
		return;
	}
	
	if(d2153_aad->switch_data.state != D2153_HEADSET)
	{	
		return;
	}
	//snd_soc_update_bits(d2153_aad->d2153_codec->codec,
	//			D2153_MICBIAS1_CTRL, D2153_MICBIAS_EN,D2153_MICBIAS_EN);		
	if (D2153_AA_Silicon == d2153_aad->chip_rev)
	{
		d2153_aad_write(client,D2153_ACCDET_CFG4,0x17);
		d2153_aad->button.status = D2153_BUTTON_IGNORE;
	}
	msleep(d2153_aad->button_detect_rate);

	btn_status = d2153_aad_read(client, D2153_ACCDET_STATUS);			
	
	dlg_info("%s btn = %d ! \n",__func__,btn_status);

	if (D2153_AA_Silicon == d2153_aad->chip_rev) 
		d2153_aad_update_bits(client, D2153_ACCDET_CFG4,
					  D2153_ACCDET_ADC_COMP_OUT_INV, 0);

	//snd_soc_update_bits(d2153_aad->d2153_codec->codec,
	//			D2153_MICBIAS1_CTRL, D2153_MICBIAS_EN,0);
	
	if ((btn_status >= button_res_tbl[SEND_BUTTON].min_val) &&
		(btn_status <= button_res_tbl[SEND_BUTTON].max_val)) {
		
		d2153_aad->button.key=KEY_MEDIA;
		d2153_aad->button.status = D2153_BUTTON_PRESS;
		input_event(d2153_aad->input_dev, EV_KEY,
				d2153_aad->button.key, 1);
		input_sync(d2153_aad->input_dev);
		dlg_info("%s event Send Press !\n", __func__);

	} else if ((btn_status >= button_res_tbl[VOL_UP_BUTTON].min_val) &&
		 (btn_status <= button_res_tbl[VOL_UP_BUTTON].max_val)) {
	
		d2153_aad->button.key=KEY_VOLUMEUP;
		d2153_aad->button.status = D2153_BUTTON_PRESS;
		input_event(d2153_aad->input_dev, EV_KEY,
				d2153_aad->button.key, 1);
		input_sync(d2153_aad->input_dev);
		dlg_info("%s event VOL UP Press !\n", __func__);
	
	} else if ((btn_status >= button_res_tbl[VOL_DN_BUTTON].min_val) &&
		 (btn_status <= button_res_tbl[VOL_DN_BUTTON].max_val)) {

		d2153_aad->button.key=KEY_VOLUMEDOWN;
		d2153_aad->button.status = D2153_BUTTON_PRESS;
		input_event(d2153_aad->input_dev, EV_KEY,
				d2153_aad->button.key, 1);
		input_sync(d2153_aad->input_dev);
		dlg_info("%s event VOL DOWN Press ! \n",__func__);
	
	}
	else {

		if(d2153_aad->button.status == D2153_BUTTON_PRESS)
		{
			input_event(d2153_aad->input_dev, EV_KEY,
					d2153_aad->button.key, 0);
			input_sync(d2153_aad->input_dev);
			//d2153_aad->button.status = D2153_BUTTON_RELEASE;
			dlg_info("%s event Rel key=%d!\n",__func__,d2153_aad->button.key);
		}

#ifdef D2153_JACK_DETECT
		jack_mode = d2153_aad_read(client, D2153_ACCDET_CFG3);

		if((jack_mode & D2153_ACCDET_JACK_MODE_JACK) ==0)
#else
		state_gpio = gpio_get_value(GPIO_PORT7);
		if (state_gpio == 1) 
#endif
		{
			d2153_aad->switch_data.state = D2153_NO_JACK;
			snd_soc_update_bits(d2153_aad->d2153_codec->codec,
				D2153_MICBIAS1_CTRL, D2153_MICBIAS_EN,0);
			switch_set_state(&d2153_aad->switch_data.sdev, d2153_aad->switch_data.state);
			dlg_info("%s Jack Pull Out ! \n",__func__);
		}
	}

}

static int __devinit d2153_aad_i2c_probe(struct i2c_client *client,
					 const struct i2c_device_id *id)
{
	struct d2153_aad_priv *d2153_aad;
#ifndef D2153_JACK_DETECT
	int ret,irq;
#endif
	u8 regval;
<<<<<<< HEAD
	struct d2153 *pmic;
=======
#ifndef D2153_DEFAULT_SET_MICBIAS
	struct d2153 *pmic;
#endif	/* D2153_DEFAULT_SET_MICBIAS */
>>>>>>> EOS2_SSG_AUDIO_ALL_JB42_K34_13w14_V01.2_SSG

	d2153_aad = devm_kzalloc(&client->dev, sizeof(struct d2153_aad_priv),
				 GFP_KERNEL);
	if (!d2153_aad)
		return -ENOMEM;
	
	d2153_aad->i2c_client = client;
	d2153_aad->d2153_codec = client->dev.platform_data;

	wake_lock_init(&d2153_aad->wakeup, WAKE_LOCK_SUSPEND, "jack_monitor");
	
	i2c_set_clientdata(client, d2153_aad);

	d2153_switch_dev_register(d2153_aad);
	
	d2153_hooksw_dev_register(client, d2153_aad);

<<<<<<< HEAD
	pmic = d2153_aad->d2153_codec->d2153_pmic;
	if (D2153_MICBIAS_LEVEL_2_6V == pmic->pdata->audio.micbias1_level)
		button_res_tbl = button_res_2V6_tbl;
	else
		button_res_tbl = button_res_2V5_tbl;
=======
#ifndef D2153_DEFAULT_SET_MICBIAS
	pmic = d2153_aad->d2153_codec->d2153_pmic;
	if (D2153_MICBIAS_LEVEL_2_5V == pmic->pdata->audio.micbias1_level)
		button_res_tbl = button_res_2V5_tbl;
	else
#endif	/* D2153_DEFAULT_SET_MICBIAS */
		button_res_tbl = button_res_2V6_tbl;
>>>>>>> EOS2_SSG_AUDIO_ALL_JB42_K34_13w14_V01.2_SSG

	d2153_aad_ex = d2153_aad;

	INIT_DELAYED_WORK(&d2153_aad->jack_monitor_work, d2153_aad_jack_monitor_timer_work);
	INIT_DELAYED_WORK(&d2153_aad->button_monitor_work, d2153_aad_button_monitor_timer_work);

	d2153_aad->button_detect_rate = 20;

	/* Ensure jack & button detection is disabled, default to auto power */
	d2153_aad_write(client, D2153_ACCDET_CONFIG,0x00); 	

#ifdef D2153_JACK_DETECT
	/* Register virtual IRQs with PMIC for jack & button detection */
	d2153_register_irq(d2153_aad->d2153_codec->d2153_pmic,
			   D2153_IRQ_EJACKDET, d2153_jack_handler, 0,
			   "Jack detect", d2153_aad);
#else
	gpio_request(GPIO_PORT7, NULL);
	gpio_direction_input(GPIO_PORT7);
	gpio_pull_up_port(GPIO_PORT7);
	gpio_set_debounce(GPIO_PORT7, 1000);	/* 1msec */
	irq=gpio_to_irq(GPIO_PORT7);
	ret=request_threaded_irq(irq, NULL, d2153_jack_handler,
						IRQF_TRIGGER_FALLING |IRQF_TRIGGER_RISING| IRQF_DISABLED, "Jack detect", d2153_aad);

	enable_irq_wake(irq);
#endif
	d2153_register_irq(d2153_aad->d2153_codec->d2153_pmic,
			   D2153_IRQ_EACCDET, d2153_button_handler, 0,
			   "Button detect", d2153_aad);

	d2153_reg_read(d2153_aad->d2153_codec->d2153_pmic, 0x96, &regval);
	d2153_aad->chip_rev = regval;
	dlg_info("%s chip_rev [0x%x] \n",__func__, d2153_aad->chip_rev);
	
	schedule_delayed_work(&d2153_aad->jack_monitor_work, msecs_to_jiffies(300));

	return 0;
}

static int __devexit d2153_aad_i2c_remove(struct i2c_client *client)
{
	struct d2153_aad_priv *d2153_aad = i2c_get_clientdata(client);
	struct d2153 *d2153_pmic = d2153_aad->d2153_codec->d2153_pmic;

	/* Disable jack & button detection, default to auto power */
	d2153_aad_write(client, D2153_ACCDET_CONFIG, 0);

	/* Free up virtual IRQs from PMIC */
	d2153_free_irq(d2153_pmic, D2153_IRQ_EJACKDET);
	d2153_free_irq(d2153_pmic, D2153_IRQ_EACCDET);

	return 0;
}

static const struct i2c_device_id d2153_aad_i2c_id[] = {
	{ "d2153-aad", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, d2153_aad_i2c_id);

/* I2C AAD control layer */
static struct i2c_driver d2153_aad_i2c_driver = {
	.driver = {
		.name = "d2153-aad",
		.owner = THIS_MODULE,
	},
	.probe		= d2153_aad_i2c_probe,
	.remove		= __devexit_p(d2153_aad_i2c_remove),
	.id_table	= d2153_aad_i2c_id,
};

static int __init d2153_aad_init(void)
{
	int ret;

	ret = i2c_add_driver(&d2153_aad_i2c_driver);
	if (ret)
		pr_err("D2153 AAD I2C registration failed %d\n", ret);

	return ret;
}
module_init(d2153_aad_init);

static void __exit d2153_aad_exit(void)
{
	i2c_del_driver(&d2153_aad_i2c_driver);
}
module_exit(d2153_aad_exit);

MODULE_DESCRIPTION("ASoC D2153 AAD Driver");
MODULE_AUTHOR("Adam Thomson <Adam.Thomson.Opensource@diasemi.com>");
MODULE_LICENSE("GPL");
