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
 
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <mach/common.h>
#include <sound/soc.h>
#include <sound/jack.h>
#include <sound/soundpath/soundpath.h>

//#define D2153_READ_PMIC_BUTTON_REG

/* PMIC related includes */
#include <linux/d2153/core.h>
#include <linux/d2153/d2153_reg.h>
 
#include <linux/d2153/d2153_codec.h>
#include <linux/d2153/d2153_aad.h>
 
#define D2153_AAD_JACK_DEBOUNCE_MS 100
#define D2153_AAD_BUTTON_DEBOUNCE_MS 50

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
static const struct button_resistance button_res_tbl[MAX_BUTTONS] = {
#if 0
	[SEND_BUTTON] = {
		.min_val = 2,
		.max_val = 10,
	},
	[VOL_UP_BUTTON] = {
		.min_val = 11,
		.max_val = 27,
	},
	[VOL_DN_BUTTON] = {
		.min_val = 28,
		.max_val = 60,
	},
#else
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
#endif
};


/* Following register access methods based on soc-cache code */
static int d2153_aad_read(struct i2c_client *client, u8 reg)
{
	struct d2153_aad_priv *d2153_aad = i2c_get_clientdata(client);
	
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
	struct d2153_aad_priv *d2153_aad = i2c_get_clientdata(client);
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

#ifdef D2153_FSI_SOUNDPATH	
int d2153_aad_read_ex(u8 reg)
{
	struct i2c_msg xfer[2];
	u8 data;
	int ret;

	/* Write register */
	xfer[0].addr = d2153_aad_ex->i2c_client->addr;
	xfer[0].flags = 0;
	xfer[0].len = 1;
	xfer[0].buf = &reg;

	/* Read data */
	xfer[1].addr = d2153_aad_ex->i2c_client->addr;
	xfer[1].flags = I2C_M_RD;
	xfer[1].len = 1;
	xfer[1].buf = &data;

	ret = i2c_transfer(d2153_aad_ex->i2c_client->adapter, xfer, 2);
	if (ret == 2)
		return data;
	else if (ret < 0)
		return ret;
	else
		return -EIO;
}
EXPORT_SYMBOL(d2153_aad_read_ex);

int d2153_aad_write_ex(u8 reg, u8 value)
{
	u8 data[2];
	int ret;

        reg &= 0xff;
        data[0] = reg;
        data[1] = value & 0xff;

	ret = i2c_master_send(d2153_aad_ex->i2c_client, data, 2);
	
	if (ret == 2)
		return 0;
	else if (ret < 0)
		return ret;
	else
		return -EIO;
}
EXPORT_SYMBOL(d2153_aad_write_ex);

int d2153_pmic_read_ex(u8 reg, u8 regval)
{
	struct d2153 *d2153_pmic = d2153_aad_ex->d2153_codec->d2153_pmic;

	return d2153_reg_read(d2153_pmic, reg, &regval);
}
EXPORT_SYMBOL(d2153_pmic_read_ex);
#endif

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
	schedule_delayed_work(&d2153_aad->jack_monitor_work, msecs_to_jiffies(D2153_AAD_JACK_DEBOUNCE_MS));
	
	return IRQ_HANDLED;
}

static irqreturn_t d2153_button_handler(int irq, void *data)
{
	struct d2153_aad_priv *d2153_aad = data;

	wake_lock_timeout(&d2153_aad->wakeup, HZ * 10);
#if 1
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
#endif	
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

static void d2153_aad_jack_monitor_timer_work(struct work_struct *work)
{
	struct d2153_aad_priv *d2153_aad = container_of(work, 
									struct d2153_aad_priv, 
									jack_monitor_work.work);

	struct i2c_client *client = d2153_aad->i2c_client;
	u8 jack_mode,btn_status;
	int state = d2153_aad->switch_data.state;

	if(d2153_aad->d2153_codec == NULL || d2153_aad->d2153_codec->codec_init ==0)
	{
		schedule_delayed_work(&d2153_aad->jack_monitor_work, msecs_to_jiffies(300));
		return;
	}
	
	jack_mode = d2153_aad_read(client, D2153_ACCDET_CFG3);	
	
	if (jack_mode & D2153_ACCDET_JACK_MODE_JACK) {

		if (jack_mode & D2153_ACCDET_JACK_MODE_MIC) {
			dlg_info("%s 4 Pole Heaset set \n",__func__);
			state=D2153_HEADSET;	
			snd_soc_update_bits(d2153_aad->d2153_codec->codec,
				D2153_MICBIAS1_CTRL, D2153_MICBIAS_EN,D2153_MICBIAS_EN);	
			d2153_aad_write(client,D2153_ACCDET_CONFIG,0x88);
		}	
		else {
			if(d2153_aad->first_check_done == 0) {
				d2153_aad_write(client,D2153_ACCDET_CONFIG,0x88);

				snd_soc_update_bits(d2153_aad->d2153_codec->codec,
					D2153_MICBIAS1_CTRL, D2153_MICBIAS_EN,D2153_MICBIAS_EN);	
				
				d2153_aad_write(client,D2153_ACCDET_CFG4,0x17);

				msleep(d2153_aad->button_detect_rate);
				
				btn_status = d2153_aad_read(client, D2153_ACCDET_STATUS);	
				dlg_info("%s Heaset set ADC= %d  \n",__func__,btn_status);

				d2153_aad_update_bits(client, D2153_ACCDET_CFG4,
						  D2153_ACCDET_ADC_COMP_OUT_INV, 0);

				snd_soc_update_bits(d2153_aad->d2153_codec->codec,
					D2153_MICBIAS1_CTRL, D2153_MICBIAS_EN,0);	

				if(btn_status == 0)	{
					dlg_info("%s First 3 Pole Heaset set2 \n",__func__);
					d2153_aad_write(client,D2153_ACCDET_CONFIG,0x08);
					state=D2153_HEADPHONE;
				}
				else {
					dlg_info("%s First 4 Pole Heaset set2 \n",__func__);
					state=D2153_HEADSET;	
					snd_soc_update_bits(d2153_aad->d2153_codec->codec,
						D2153_MICBIAS1_CTRL, D2153_MICBIAS_EN,D2153_MICBIAS_EN);	
					d2153_aad_write(client,D2153_ACCDET_CONFIG,0x88);			
				}
			}
			else
			{
				dlg_info("%s 3 Pole Heaset set \n",__func__);
				d2153_aad_write(client,D2153_ACCDET_CONFIG,0x08);
				state=D2153_HEADPHONE;		
			}
		}
	}
	else {		
		dlg_info("%s Jack Pull Out ! \n",__func__);
		d2153_aad->first_check_done=1;
		state=D2153_NO_JACK;
		snd_soc_update_bits(d2153_aad->d2153_codec->codec,
				D2153_MICBIAS1_CTRL, D2153_MICBIAS_EN,0);
		d2153_aad_write(client,D2153_ACCDET_CONFIG,0x88);
	}

	d2153_aad->button.status = D2153_BUTTON_RELEASE;
	
	if (d2153_aad->switch_data.state != state) {	
		d2153_aad->d2153_codec->switch_state=state;
		d2153_aad->switch_data.state = state;
		switch_set_state(&d2153_aad->switch_data.sdev, state);
	}

}

static void d2153_aad_button_monitor_timer_work(struct work_struct *work)
{
	struct d2153_aad_priv *d2153_aad = container_of(work, 
									struct d2153_aad_priv, 
									button_monitor_work.work);
	struct i2c_client *client = d2153_aad->i2c_client;
	u8 btn_status;
	u8 jack_mode;
	int state;

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
	d2153_aad_write(client,D2153_ACCDET_CFG4,0x17);

	d2153_aad->button.status = D2153_BUTTON_IGNORE;

	msleep(d2153_aad->button_detect_rate);
	
	btn_status = d2153_aad_read(client, D2153_ACCDET_STATUS);			
	
	//dlg_info("%s btn_status = %d !!!!!!!!!!!!!!!!!!!!!! \n",__func__,btn_status);

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
		//dlg_info("%s event Send Press ! \n",__func__);

	}
	else if ((btn_status >= button_res_tbl[VOL_UP_BUTTON].min_val) &&
		 (btn_status <= button_res_tbl[VOL_UP_BUTTON].max_val)) {
	
		d2153_aad->button.key=KEY_VOLUMEUP;
		d2153_aad->button.status = D2153_BUTTON_PRESS;
		input_event(d2153_aad->input_dev, EV_KEY,
				d2153_aad->button.key, 1);
		input_sync(d2153_aad->input_dev);
		//dlg_info("%s event VOL UP Press ! \n",__func__);
	
	}
	else if ((btn_status >= button_res_tbl[VOL_DN_BUTTON].min_val) &&
		 (btn_status <= button_res_tbl[VOL_DN_BUTTON].max_val)) {

		d2153_aad->button.key=KEY_VOLUMEDOWN;
		d2153_aad->button.status = D2153_BUTTON_PRESS;
		input_event(d2153_aad->input_dev, EV_KEY,
				d2153_aad->button.key, 1);
		input_sync(d2153_aad->input_dev);
		//dlg_info("%s event VOL DOWN Press ! \n",__func__);
	
	}
	else {

		if(d2153_aad->button.status == D2153_BUTTON_PRESS)
		{
			input_event(d2153_aad->input_dev, EV_KEY,
					d2153_aad->button.key, 0);
			input_sync(d2153_aad->input_dev);
			//d2153_aad->button.status = D2153_BUTTON_RELEASE;
			//dlg_info("%s event Release key=%d ! \n",__func__,d2153_aad->button.key);
		}
		
		jack_mode = d2153_aad_read(client, D2153_ACCDET_CFG3);

		if((jack_mode & D2153_ACCDET_JACK_MODE_JACK) ==0)
		{
			d2153_aad->switch_data.state = D2153_NO_JACK;
			snd_soc_update_bits(d2153_aad->d2153_codec->codec,
				D2153_MICBIAS1_CTRL, D2153_MICBIAS_EN,0);
			switch_set_state(&d2153_aad->switch_data.sdev, state);
			dlg_info("%s Jack Pull Out ! \n",__func__);
		}
	}

}

#if 0
/* Enable/Disable HW jack detection in AAD block */
int d2153_jack_detect(struct snd_soc_jack *jack, bool enable)
{
	struct d2153_codec_priv *d2153_codec =
		snd_soc_codec_get_drvdata(jack->codec);
	struct i2c_client *client = d2153_codec->aad_i2c_client;
	struct d2153_aad_priv *d2153_aad = i2c_get_clientdata(client);

	/* Store jack for later use */
	d2153_aad->jack.jack = jack;
	d2153_aad->jack.report_mask = D2153_JACK_REPORT_TYPE;
	d2153_aad->jack.status = false;

	/* Enable/Disable jack detection in hardware */
	if (enable)
		d2153_aad_update_bits(client, D2153_ACCDET_CONFIG,
				      D2153_ACCDET_JACK_EN,
				      D2153_ACCDET_JACK_EN);
	else
		d2153_aad_update_bits(client, D2153_ACCDET_CONFIG,
				      D2153_ACCDET_JACK_EN, 0);

	return 0;
}
EXPORT_SYMBOL_GPL(d2153_jack_detect);

/* IRQ handler for HW jack detection */
static irqreturn_t d2153_jack_handler(int irq, void *data)
{
	struct d2153_aad_priv *d2153_aad = data;
	struct i2c_client *client = d2153_aad->i2c_client;
	u8 jack_mode;
	int report = 0;

	jack_mode = d2153_aad_read(client, D2153_ACCDET_CFG3);

	if (jack_mode & D2153_ACCDET_JACK_MODE_JACK) {
		report |= SND_JACK_HEADPHONE;

		if (jack_mode & D2153_ACCDET_JACK_MODE_MIC)
			report |= SND_JACK_MICROPHONE;
			
		/* DLG - Don't really need this but leaving it for now */
		d2153_aad->jack.status = true;
	}
	else {
		/* DLG - Don't really need this but leaving it for now */
		d2153_aad->jack.status = false;
	}
	
	snd_soc_jack_report(d2153_aad->jack.jack, report,
			    d2153_aad->jack.report_mask);

	return IRQ_HANDLED;
}
 
/* Enable/Disable HW button detection in AAD block */
int d2153_button_detect(struct snd_soc_jack *button, bool enable)
{
	struct d2153_codec_priv *d2153_codec =
		snd_soc_codec_get_drvdata(button->codec);
	struct i2c_client *client = d2153_codec->aad_i2c_client;
	struct d2153_aad_priv *d2153_aad = i2c_get_clientdata(client);

	/* Store jack for later use */
	d2153_aad->button.jack = button;
	d2153_aad->button.report_mask = D2153_BUTTON_REPORT_TYPE;
	d2153_aad->button.status = false;

	/* Enable/Disable button detection in hardware */
	if (enable)
		d2153_aad_update_bits(client, D2153_ACCDET_CONFIG,
				      D2153_ACCDET_BTN_EN, D2153_ACCDET_BTN_EN);
	else
		d2153_aad_update_bits(client, D2153_ACCDET_CONFIG,
				      D2153_ACCDET_BTN_EN, 0);

	return 0;
}
EXPORT_SYMBOL_GPL(d2153_button_detect);

/*
 * IRQ handler for HW button detection.
 * 
 * Jack reports are as follows:
 * 
 * SND_JACK_BTN_0 = Unknown button,
 * SND_JACK_BTN_1 = Send button,
 * SND_JACK_BTN_2 = Vol+ button,
 * SND_JACK_BTN_3 = Vol- button,
 */
static irqreturn_t d2153_button_handler(int irq, void *data)
{
	struct d2153_aad_priv *d2153_aad = data;
#ifdef D2153_READ_PMIC_BUTTON_REG
	struct d2153 *d2153_pmic = d2153_aad->d2153_codec->d2153_pmic;
	u8 status_c;
#endif
	struct i2c_client *client = d2153_aad->i2c_client;
	u8 btn_status;
	int report = 0;

	/*
	 * DLG - Am not convinced we need to read from the PMIC register for
	 * button press/release. AAD documentation suggests it can't miss an
	 * interrupt and if we keep status locally, then we can just toggle
	 * between states each time an interrupt occurs. Without hardware
	 * I can't prove this though so have implemented the PMIC register
	 * read functionality as well, just in case I'm wrong.
	 */
#ifdef D2153_READ_PMIC_BUTTON_REG
	/* Read accessory detect status (press/release) from PMIC */
	d2153_reg_read(d2153_pmic, D2153_STATUS_C_REG, &status_c);
	if (status_c & D2153_ACC_DET_STATUS_MASK) {
#else
	/* If last event was release, then this must be a press */
	if (!d2153_aad->button.status) {
#endif
		btn_status = d2153_aad_read(client, D2153_ACCDET_STATUS);
		
		if (btn_status == 0) {
			/* 
			 * Low resolution read - need to wait for AAD block to
			 * perfom high resolution measurements so we know which
			 * button was pressed.
			 */
			msleep(d2153_aad->button_detect_rate);
			btn_status = d2153_aad_read(client, D2153_ACCDET_STATUS);
		}
		
		if ((btn_status >= button_res_tbl[SEND_BUTTON].min_val) ||
		    (btn_status <= button_res_tbl[SEND_BUTTON].max_val)) {
			report = SND_JACK_BTN_1;
		}
		else if ((btn_status >= button_res_tbl[VOL_UP_BUTTON].min_val) ||
			 (btn_status <= button_res_tbl[VOL_UP_BUTTON].max_val)) {
			report = SND_JACK_BTN_2;
		}
		else if ((btn_status >= button_res_tbl[VOL_DN_BUTTON].min_val) ||
			 (btn_status <= button_res_tbl[VOL_DN_BUTTON].max_val)) {
			report = SND_JACK_BTN_3;
		}
		else
		{
			/* 
			 * Report unknown button in case user wants to do
			 * something in this scenario.
			 */
			report = SND_JACK_BTN_0;
		}
	}
	
	d2153_aad->button.status = !d2153_aad->button.status;
	snd_soc_jack_report(d2153_aad->button.jack, report,
			    d2153_aad->button.report_mask);

	return IRQ_HANDLED;
}

#endif

static int __devinit d2153_aad_i2c_probe(struct i2c_client *client,
					 const struct i2c_device_id *id)
{
	struct d2153_aad_priv *d2153_aad;

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

	d2153_aad_ex = d2153_aad;

	INIT_DELAYED_WORK(&d2153_aad->jack_monitor_work, d2153_aad_jack_monitor_timer_work);
	INIT_DELAYED_WORK(&d2153_aad->button_monitor_work, d2153_aad_button_monitor_timer_work);

	d2153_aad->button_detect_rate = 10;

	/* Ensure jack & button detection is disabled, default to auto power */
	d2153_aad_write(client, D2153_ACCDET_CONFIG,0x00); 	

	/* Register virtual IRQs with PMIC for jack & button detection */
	d2153_register_irq(d2153_aad->d2153_codec->d2153_pmic,
			   D2153_IRQ_EJACKDET, d2153_jack_handler, 0,
			   "Jack detect", d2153_aad);
	d2153_register_irq(d2153_aad->d2153_codec->d2153_pmic,
			   D2153_IRQ_EACCDET, d2153_button_handler, 0,
			   "Button detect", d2153_aad);
	
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

	if (D2153_INTRODUCE_BOARD_REV > u2_get_board_rev())
		return 0;

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

/* 
 * DLG - example code for machine driver which should reside in
 * sound/soc/<MACH> directory in relevant file.
 */
#if 0 //ndef D2153_FSI_SOUNDPATH	
static const struct snd_soc_dapm_widget d2153_aad_widgets[] = {
	SND_SOC_DAPM_MIC("Mic Jack", NULL),
	SND_SOC_DAPM_HP("Headphone Jack", NULL),
};

static const struct snd_soc_dapm_route d2153_aad_map[] = {
	/* Dest  |  Connecting Widget  |  Source */
	
	/* Mic Jack */
	{ "MICL", NULL, "Mic Jack" },

	/* Headphone Jack */
	{ "Headphone Jack", NULL, "HPL" },
	{ "Headphone Jack", NULL, "HPR" },
};

static struct snd_soc_jack jack;

static struct snd_soc_jack_pin jack_pins[] = {
	{ .pin = "Headphone Jack", .mask = SND_JACK_HEADPHONE },
	{ .pin = "Mic Jack", .mask = SND_JACK_MICROPHONE },
};

static struct snd_soc_jack button;

static int d2153_fsi_aad_init(struct snd_soc_codec *codec)
{
	struct snd_soc_dapm_context *dapm = &codec->dapm;
	int ret;

	/* Add AAD specific DAPM items */
	ret = snd_soc_dapm_new_controls(dapm, d2153_aad_widgets,
					ARRAY_SIZE(d2153_aad_widgets));
	if (ret < 0)
		return ret;
		
	ret = snd_soc_dapm_add_routes(dapm, d2153_aad_map,
				      ARRAY_SIZE(d2153_aad_map));
	if (ret < 0)
		return ret;

	/* Jack detection */
	snd_soc_jack_new(codec, "Jack", D2153_JACK_REPORT_TYPE, &jack);
	snd_soc_jack_add_pins(&jack, ARRAY_SIZE(jack_pins), jack_pins);
	d2153_jack_detect(&jack, 1);

	/* Button detection */
	snd_soc_jack_new(codec, "Button", D2153_BUTTON_REPORT_TYPE, &button);
	d2153_button_detect(&button, 1);

	return 0;
}

static int fsi_d2153_init(struct snd_soc_pcm_runtime *rtd)
{
#ifdef CONFIG_SND_SOC_D2153_AAD
	struct snd_soc_codec *codec = rtd->codec;
	static bool initialised = false;
	int ret;

	/* Only want to initialise the once */
	if (initialised)
		return 0;

	ret = d2153_fsi_aad_init(codec);
	if (ret < 0)
		return ret;
	initialised = true;
#endif

	return 0;
}
#endif
