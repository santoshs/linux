/*
 * d2153-core.c  --  Device access for Dialog D2153
 *   
 * Copyright(c) 2012 Dialog Semiconductor Ltd.
 *  
 * Author: Dialog Semiconductor Ltd. D. Chen
 *  
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

#include <linux/bug.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/irq.h>
#include <linux/ioport.h>
#include <linux/i2c.h>

#include <linux/smp.h>
#include <linux/proc_fs.h>
#include <linux/kthread.h>
#include <asm/uaccess.h>

#include <linux/d2153/d2153_version.h>
#include <linux/d2153/pmic.h>
#include <linux/d2153/d2153_reg.h> 
#include <linux/d2153/rtc.h>
#include <linux/d2153/hwmon.h>
#include <linux/d2153/core.h>

#include <mach/common.h>

#define D2153_REG_DEBUG
//#define D2153_SUPPORT_I2C_HIGH_SPEED


#ifdef D2153_REG_DEBUG
#define D2153_MAX_HISTORY           	100
#define D2153_HISTORY_READ_OP       	0
#define D2153D_HISTORY_WRITE_OP      	1

#define d2153_write_reg_cache(reg,data)   gD2153RegCache[reg] = data;

struct d2153_reg_history{
	u8 mode;
	u8 regnum; 
	u8 value;
	long long time;
};

static u8 gD2153RegCache[D2153_MAX_REGISTER_CNT];
static u8 gD2153CurHistory=0;
static struct d2153_reg_history gD2153RegHistory[D2153_MAX_HISTORY];
#endif /* D2153_REG_DEBUG */

/*
 *   Static global variable
 */
static struct d2153 *d2153_dev_info;

/*
 * D2153 Device IO
 */

#ifdef D2153_REG_DEBUG
void d2153_write_reg_history(u8 opmode,u8 reg,u8 data) { 
	//int cpu = smp_processor_id(); 

	if(gD2153CurHistory == D2153_MAX_HISTORY) 
		gD2153CurHistory=0;
		
	gD2153RegHistory[gD2153CurHistory].mode=opmode; 
	gD2153RegHistory[gD2153CurHistory].regnum=reg; 
	gD2153RegHistory[gD2153CurHistory].value=data;         
	//gD2153RegHistory[gD2153CurHistory].time= cpu_clock(cpu)/1000; 
	gD2153CurHistory++; 
}
#endif /* D2153_REG_DEBUG */

/*
 *
 */
static int d2153_read(struct d2153 *d2153, u8 reg, int num_regs, u8 * dest)
{
	int bytes = num_regs;

	if (d2153->read_dev == NULL)
		return -ENODEV;

	if ((reg + num_regs - 1) > D2153_MAX_REGISTER_CNT) {
		dev_err(d2153->dev, "Invalid reg %x\n", reg + num_regs - 1);
		return -EINVAL;
	}

	/* Actually read it out */
	return d2153->read_dev(d2153, reg, bytes, (char *)dest);
}

/*
 *
 */
static int d2153_write(struct d2153 *d2153, u8 reg, int num_regs, u8 * src)
{
	int bytes = num_regs;

	if (d2153->write_dev == NULL)
		return -ENODEV;

	if ((reg + num_regs - 1) > D2153_MAX_REGISTER_CNT) {
		dev_err(d2153->dev, "Invalid reg %x\n", reg + num_regs - 1);
		return -EINVAL;
	}
	/* Actually write it out */
	return d2153->write_dev(d2153, reg, bytes, (char *)src);
}

/*
 * d2153_clear_bits - 
 * @ d2153 :
 * @ reg :
 * @ mask :
 *
 */
int d2153_clear_bits(struct d2153 * const d2153, u8 const reg, u8 const mask)
{
	u8 data;
	int err;

	if(u2_get_board_rev() <= 4) {
		dlg_info("%s is called on old Board revision. error\n", __func__);
		return 0;
	}
	mutex_lock(&d2153->d2153_io_mutex);
	err = d2153_read(d2153, reg, 1, &data);
	if (err != 0) {
		dev_err(d2153->dev, "read from reg R%d failed\n", reg);
		goto out;
	}
#ifdef D2153_REG_DEBUG   
	else
		d2153_write_reg_history(D2153_HISTORY_READ_OP,reg,data);
#endif
	data &= ~mask;
	err = d2153_write(d2153, reg, 1, &data);
	if (err != 0)
		dlg_err("write to reg R%d failed\n", reg);
#ifdef D2153_REG_DEBUG    
	else  {   
		d2153_write_reg_history(D2153D_HISTORY_WRITE_OP,reg,data);
		d2153_write_reg_cache(reg,data);
	}
#endif    
out:
	mutex_unlock(&d2153->d2153_io_mutex);

	return err;
}
EXPORT_SYMBOL_GPL(d2153_clear_bits);


/*
 * d2153_set_bits - 
 * @ d2153 :
 * @ reg :
 * @ mask :
 *
 */
int d2153_set_bits(struct d2153 * const d2153, u8 const reg, u8 const mask)
{
	u8 data;
	int err;

	if(u2_get_board_rev() <= 4) {
		dlg_info("%s is called on old Board revision. error\n", __func__);
		return 0;
	}
	mutex_lock(&d2153->d2153_io_mutex);
	err = d2153_read(d2153, reg, 1, &data);
	if (err != 0) {
		dev_err(d2153->dev, "read from reg R%d failed\n", reg);
		goto out;
	}
#ifdef D2153_REG_DEBUG    
	else {
		d2153_write_reg_history(D2153_HISTORY_READ_OP,reg,data);
	}
#endif      
	data |= mask;
	err = d2153_write(d2153, reg, 1, &data);
	if (err != 0)
		dlg_err("write to reg R%d failed\n", reg);
#ifdef D2153_REG_DEBUG    
	else  {   
		d2153_write_reg_history(D2153D_HISTORY_WRITE_OP,reg,data);
		d2153_write_reg_cache(reg,data);
	}
#endif    
out:
	mutex_unlock(&d2153->d2153_io_mutex);

	return err;
}
EXPORT_SYMBOL_GPL(d2153_set_bits);


/*
 * d2153_reg_read - 
 * @ d2153 :
 * @ reg :
 * @ *dest :
 *
 */
int d2153_reg_read(struct d2153 * const d2153, u8 const reg, u8 *dest)
{
	u8 data;
	int err;

	if(u2_get_board_rev() <= 4) {
		dlg_info("%s is called on old Board revision. error\n", __func__);
		return 0;
	}
	mutex_lock(&d2153->d2153_io_mutex);
	err = d2153_read(d2153, reg, 1, &data);
	if (err != 0)
		dlg_err("read from reg R%d failed\n", reg);
#ifdef D2153_REG_DEBUG    
	else
		d2153_write_reg_history(D2153_HISTORY_READ_OP,reg,data);
#endif       
	*dest = data;
	mutex_unlock(&d2153->d2153_io_mutex);
	return err;
}
EXPORT_SYMBOL_GPL(d2153_reg_read);


/*
 * d2153_reg_write - 
 * @ d2153 :
 * @ reg :
 * @ val :
 *
 */
int d2153_reg_write(struct d2153 * const d2153, u8 const reg, u8 const val)
{
	int ret;
	u8 data = val;

	if(u2_get_board_rev() <= 4) {
		dlg_info("%s is called on old Board revision. error\n", __func__);
		return 0;
	}
	mutex_lock(&d2153->d2153_io_mutex);
	ret = d2153_write(d2153, reg, 1, &data);
	if (ret != 0)
		dlg_err("write to reg R%d failed\n", reg);
#ifdef D2153_REG_DEBUG    
	else  {   
		d2153_write_reg_history(D2153D_HISTORY_WRITE_OP,reg,data);
		d2153_write_reg_cache(reg,data);
	}
#endif    
	mutex_unlock(&d2153->d2153_io_mutex);
	return ret;
}
EXPORT_SYMBOL_GPL(d2153_reg_write);


/*
 * d2153_block_read - 
 * @ d2153 :
 * @ start_reg :
 * @ regs :
 * @ *dest :
 *
 */
int d2153_block_read(struct d2153 * const d2153, u8 const start_reg, u8 const regs,
		      u8 * const dest)
{
	int err = 0;
#ifdef D2153_REG_DEBUG   
	int i;
#endif	   

	if(u2_get_board_rev() <= 4) {
		dlg_info("%s is called on old Board revision. error\n", __func__);
		return 0;
	}
	mutex_lock(&d2153->d2153_io_mutex);
	err = d2153_read(d2153, start_reg, regs, dest);
	if (err != 0)
		dlg_err("block read starting from R%d failed\n", start_reg);
#ifdef D2153_REG_DEBUG    
	else {
		for(i=0; i<regs; i++)
			d2153_write_reg_history(D2153D_HISTORY_WRITE_OP,start_reg+i,*(dest+i));
	}
#endif       
	mutex_unlock(&d2153->d2153_io_mutex);
	return err;
}
EXPORT_SYMBOL_GPL(d2153_block_read);


/*
 * d2153_block_write - 
 * @ d2153 :
 * @ start_reg :
 * @ regs :
 * @ *src :
 *
 */
int d2153_block_write(struct d2153 * const d2153, u8 const start_reg, u8 const regs,
		       u8 * const src)
{
	int ret = 0;
#ifdef D2153_REG_DEBUG   
	int i;
#endif

	if(u2_get_board_rev() <= 4) {
		dlg_info("%s is called on old Board revision. error\n", __func__);
		return 0;
	}
	mutex_lock(&d2153->d2153_io_mutex);
	ret = d2153_write(d2153, start_reg, regs, src);
	if (ret != 0)
		dlg_err("block write starting at R%d failed\n", start_reg);
#ifdef D2153_REG_DEBUG    
	else {
		for(i=0; i<regs; i++)
			d2153_write_reg_cache(start_reg+i,*(src+i));
	}
#endif    
	mutex_unlock(&d2153->d2153_io_mutex);
	return ret;
}
EXPORT_SYMBOL_GPL(d2153_block_write);

/*
 * Register a client device.  This is non-fatal since there is no need to
 * fail the entire device init due to a single platform device failing.
 */
static void d2153_client_dev_register(struct d2153 *d2153,
				       const char *name,
				       struct platform_device **pdev)
{
	int ret;

	*pdev = platform_device_alloc(name, -1);
	if (*pdev == NULL) {
		dev_err(d2153->dev, "Failed to allocate %s\n", name);
		return;
	}

	(*pdev)->dev.parent = d2153->dev;
	platform_set_drvdata(*pdev, d2153);
	ret = platform_device_add(*pdev);
	if (ret != 0) {
		dev_err(d2153->dev, "Failed to register %s: %d\n", name, ret);
		platform_device_put(*pdev);
		*pdev = NULL;
	}
}

/*
 * d2153_aad_dev_register
 */
static void d2153_codec_dev_register(struct d2153 *d2153,
				       const char *name)
{
	struct i2c_board_info info;

	strncpy(info.type, name, sizeof(info.type));

	/* D2153 actual address */
	info.addr = D2153_CODEC_I2C_ADDR;
	info.platform_data = d2153;
	info.flags = 0;
	info.irq = 0;
	info.archdata = NULL;
	info.of_node = NULL;

	/* Add the Codec I2C device.  This calls the probe() function. */

	d2153->codec_i2c_client = i2c_new_device(d2153->pmic_i2c_client->adapter, &info);

	if (!d2153->codec_i2c_client)
		dev_err(d2153->dev, "Unable to add I2C device for 0x%x\n", info.addr);
}


static void d2153_codec_dev_unregister(struct i2c_client *codec_i2c_client)
{
	 i2c_unregister_device(codec_i2c_client);
}

/*
 * d2153_system_event_handler
 */
static irqreturn_t d2153_system_event_handler(int irq, void *data)
{
	//todo DLG export the event??
	//struct d2153 *d2153 = data;
	return IRQ_HANDLED;	
}

/*
 * d2153_system_event_init
 */
static void d2153_system_event_init(struct d2153 *d2153)
{
	d2153_register_irq(d2153, D2153_IRQ_EVDD_MON, d2153_system_event_handler, 
                            0, "VDD MON", d2153);
}

/*****************************************/
/* 	Debug using proc entry           */
/*****************************************/
#ifdef CONFIG_PROC_FS
static int d2153_ioctl_open(struct inode *inode, struct file *file)
{
	dlg_info("%s\n", __func__);
	file->private_data = PDE(inode)->data;
	return 0;
}

int d2153_ioctl_release(struct inode *inode, struct file *file)
{
	if(u2_get_board_rev() <= 4) {
		dlg_info("%s is called on old Board revision. error\n", __func__);
		return 0;
	}
	file->private_data = NULL;
	return 0;
}

/*
 * d2153_ioctl
 */
static long d2153_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
		struct d2153 *d2153 =  file->private_data;
		pmu_reg reg;
		int ret = 0;
		u8 reg_val, event_reg[4];
	
		if (!d2153)
			return -ENOTTY;			

		switch (cmd) {
			
			case D2153_IOCTL_READ_REG:
			if (copy_from_user(&reg, (pmu_reg *)arg, sizeof(pmu_reg)) != 0)
				return -EFAULT;

			ret = d2153_read(d2153, reg.reg,1, &reg_val);
			reg.val = (unsigned short)reg_val;
			if (copy_to_user((pmu_reg *)arg, &reg, sizeof(pmu_reg)) != 0)
				return -EFAULT;
			break;
	
		case D2153_IOCTL_WRITE_REG:
			if (copy_from_user(&reg, (pmu_reg *)arg, sizeof(pmu_reg)) != 0)
				return -EFAULT;
			d2153_write(d2153, reg.reg,1, (u8)reg.val);
			break;	
	
		default:
			dlg_err("%s: unsupported cmd\n", __func__);
			ret = -ENOTTY;
		}
	
		return ret;
}

#define MAX_USER_INPUT_LEN      100
#define MAX_REGS_READ_WRITE     10

enum pmu_debug_ops {
	PMUDBG_READ_REG = 0UL,
	PMUDBG_WRITE_REG,
};

struct pmu_debug {
	int read_write;
	int len;
	int addr;
	u8 val[MAX_REGS_READ_WRITE];
};

/*
 * d2153_dbg_usage
 */
static void d2153_dbg_usage(void)
{
	printk(KERN_INFO "Usage:\n");
	printk(KERN_INFO "Read a register: echo 0x0800 > /proc/pmu0\n");
	printk(KERN_INFO
		"Read multiple regs: echo 0x0800 -c 10 > /proc/pmu0\n");
	printk(KERN_INFO
		"Write multiple regs: echo 0x0800 0xFF 0xFF > /proc/pmu0\n");
	printk(KERN_INFO
		"Write single reg: echo 0x0800 0xFF > /proc/pmu0\n");
	printk(KERN_INFO "Max number of regs in single write is :%d\n",
		MAX_REGS_READ_WRITE);
	printk(KERN_INFO "Register address is encoded as follows:\n");
	printk(KERN_INFO "0xSSRR, SS: i2c slave addr, RR: register addr\n");
}


/*
 * d2153_dbg_parse_args
 */
static int d2153_dbg_parse_args(char *cmd, struct pmu_debug *dbg)
{
	char *tok;                 /* used to separate tokens             */
	const char ct[] = " \t";   /* space or tab delimits the tokens    */
	bool count_flag = false;   /* whether -c option is present or not */
	int tok_count = 0;         /* total number of tokens parsed       */
	int i = 0;

	dbg->len        = 0;

	/* parse the input string */
	while ((tok = strsep(&cmd, ct)) != NULL) {
		dlg_info("token: %s\n", tok);

		/* first token is always address */
		if (tok_count == 0) {
			sscanf(tok, "%x", &dbg->addr);
		} else if (strnicmp(tok, "-c", 2) == 0) {
			/* the next token will be number of regs to read */
			tok = strsep(&cmd, ct);
			if (tok == NULL)
				return -EINVAL;

			tok_count++;
			sscanf(tok, "%d", &dbg->len);
			count_flag = true;
			break;
		} else {
			int val;

			/* this is a value to be written to the pmu register */
			sscanf(tok, "%x", &val);
			if (i < MAX_REGS_READ_WRITE) {
				dbg->val[i] = val;
				i++;
			}
		}

		tok_count++;
	}

	/* decide whether it is a read or write operation based on the
	 * value of tok_count and count_flag.
	 * tok_count = 0: no inputs, invalid case.
	 * tok_count = 1: only reg address is given, so do a read.
	 * tok_count > 1, count_flag = false: reg address and atleast one
	 *     value is present, so do a write operation.
	 * tok_count > 1, count_flag = true: to a multiple reg read operation.
	 */
	switch (tok_count) {
		case 0:
			return -EINVAL;
		case 1:
			dbg->read_write = PMUDBG_READ_REG;
			dbg->len = 1;
			break;
		default:
			if (count_flag == true) {
				dbg->read_write = PMUDBG_READ_REG;
			} else {
				dbg->read_write = PMUDBG_WRITE_REG;
				dbg->len = i;
		}
	}

	return 0;
}

/* 
 * d2153_ioctl_write
 */
static ssize_t d2153_ioctl_write(struct file *file, const char __user *buffer,
	size_t len, loff_t *offset)
{
	struct d2153 *d2153 = file->private_data;
	struct pmu_debug dbg;
	char cmd[MAX_USER_INPUT_LEN];
	int ret, i;

	dlg_info("%s\n", __func__);

	if (!d2153) {
		dlg_err("%s: driver not initialized\n", __func__);
		return -EINVAL;
	}

	if (len > MAX_USER_INPUT_LEN)
		len = MAX_USER_INPUT_LEN;

	if (copy_from_user(cmd, buffer, len)) {
		dlg_err("%s: copy_from_user failed\n", __func__);
		return -EFAULT;
	}

	/* chop of '\n' introduced by echo at the end of the input */
	if (cmd[len - 1] == '\n')
		cmd[len - 1] = '\0';

	if (d2153_dbg_parse_args(cmd, &dbg) < 0) {
		d2153_dbg_usage();
		return -EINVAL;
	}

	dlg_info("operation: %s\n", (dbg.read_write == PMUDBG_READ_REG) ?
		"read" : "write");
	dlg_info("address  : 0x%x\n", dbg.addr);
	dlg_info("length   : %d\n", dbg.len);

	if (dbg.read_write == PMUDBG_READ_REG) {
		ret = d2153_read(d2153, dbg.addr, dbg.len, dbg.val);
		if (ret < 0) {
			dlg_err("%s: pmu reg read failed\n", __func__);
			return -EFAULT;
		}

		for (i = 0; i < dbg.len; i++, dbg.addr++)
			dlg_info("[%x] = 0x%02x\n", dbg.addr,
				dbg.val[i]);
	} else {
		ret = d2153_write(d2153, dbg.addr, dbg.len, dbg.val);
		if (ret < 0) {
			dlg_err("%s: pmu reg write failed\n", __func__);
			return -EFAULT;
		}
	}

	*offset += len;

	return len;
}

static const struct file_operations d2153_pmu_ops = {
	.open = d2153_ioctl_open,
	.unlocked_ioctl = d2153_ioctl,
	.write = d2153_ioctl_write,
	.release = d2153_ioctl_release,
	.owner = THIS_MODULE,
};

/*
 * d2153_debug_proc_init
 */
void d2153_debug_proc_init(struct d2153 *d2153)
{
	struct proc_dir_entry *entry;

	if(u2_get_board_rev() <= 4) {
		dlg_info("%s is called on old Board revision. error\n", __func__);
		return;
	}
	disable_irq(d2153->chip_irq);
	entry = proc_create_data("pmu0", S_IRWXUGO, NULL, &d2153_pmu_ops, d2153);
	enable_irq(d2153->chip_irq);
	dlg_crit("\nD2153-core.c: proc_create_data() = %p; name=\"%s\"\n", entry, (entry?entry->name:""));
}

/*
 * d2153_debug_proc_exit
 */
void d2153_debug_proc_exit(void)
{
	if(u2_get_board_rev() <= 4) {
		dlg_info("%s is called on old Board revision. error\n", __func__);
		return;
	}
	//disable_irq(client->irq);
	remove_proc_entry("pmu0", NULL);
	//enable_irq(client->irq);
}
#endif /* CONFIG_PROC_FS */

struct d2153 *d2153_regl_info = NULL;
EXPORT_SYMBOL(d2153_regl_info);


/*
 * d2153_device_init
 */
int d2153_device_init(struct d2153 *d2153, int irq,
		       struct d2153_platform_data *pdata)
{
	int ret = 0;//, tmp;
	//struct regulator *regulator;
#ifdef D2153_REG_DEBUG 
	int i;
	u8 data;
#endif

	if(u2_get_board_rev() <= 4) {
		dlg_info("%s is called on old Board revision. error\n", __func__);
		return 0;
	}
	if(d2153 != NULL)
		d2153_regl_info = d2153;
	else
		goto err;

	dlg_info("D2153 Driver : built at %s on %s\n", __TIME__, __DATE__);

	//DLG eric. d2153->pmic.max_dcdc = 25; //
	d2153->pdata = pdata;

	mutex_init(&d2153->d2153_io_mutex);

#ifdef D2153_SUPPORT_I2C_HIGH_SPEED
	d2153_set_bits(d2153, D2153_CONTROL_B_REG, D2153_I2C_SPEED_MASK);

	/* Page write for I2C we donot support repeated write and I2C speed set to 1.7MHz */
	d2153_clear_bits(d2153, D2153_CONTROL_B_REG, D2153_WRITE_MODE_MASK);
#else
	/* Page write for I2C we donot support repeated write and I2C speed set to 400KHz */
	d2153_clear_bits(d2153, D2153_CONTROL_B_REG, D2153_WRITE_MODE_MASK | D2153_I2C_SPEED_MASK);
#endif

	d2153_reg_write(d2153, D2153_PD_DIS_REG, 0x0);
	d2153_reg_write(d2153, D2153_PULLDOWN_D_REG, 0x0C);
	d2153_reg_write(d2153, D2153_BBAT_CONT_REG, 0x1F);
	d2153_set_bits(d2153,  D2153_SUPPLY_REG, D2153_BBCHG_EN_MASK);

	d2153_reg_write(d2153, D2153_AUDIO_REG_DFLT_6_REG,0x20);

	d2153_reg_write(d2153, D2153_GPADC_MCTL_REG, 0x55);

	d2153_set_bits(d2153,  D2153_OUT2_32K_ONKEY_CONT_REG, D2153_OUT2_32K_EN_MASK);
	
	d2153_dev_info = d2153;
	//pm_power_off = d2153_system_poweroff;

	ret = d2153_irq_init(d2153, irq, pdata);
	if (ret < 0)
		goto err;
    
	if (pdata && pdata->init) {
		ret = pdata->init(d2153);
		if (ret != 0) {
			dev_err(d2153->dev, "Platform init() failed: %d\n", ret);
			goto err_irq;
		}
	}
	
	// Regulator Specific Init
	ret = d2153_platform_regulator_init(d2153);
	if (ret != 0) {
		dev_err(d2153->dev, "Platform Regulator init() failed: %d\n", ret);
		goto err_irq;
	}

	d2153_client_dev_register(d2153, "d2153-battery", &(d2153->batt.pdev));
	d2153_client_dev_register(d2153, "d2153-rtc", &(d2153->rtc.pdev));
	d2153_client_dev_register(d2153, "d2153-onkey", &(d2153->onkey.pdev));
	d2153_codec_dev_register(d2153, "d2153-codec");

	d2153_system_event_init(d2153);
#ifdef CONFIG_PROC_FS
	d2153_debug_proc_init(d2153);
#endif

#ifdef D2153_REG_DEBUG    
	  for(i = 0; i < D2153_MAX_REGISTER_CNT; i++)
	  {
	  	d2153_reg_read(d2153, i, &data);
		d2153_write_reg_cache(i,data);
	  }
#endif   

	// set MCTRL_EN enabled
	d2153_set_mctl_enable();

	return 0;

err_irq:
	d2153_irq_exit(d2153);
	d2153_dev_info = NULL;
	pm_power_off = NULL;
err:
	dlg_crit("\n\nD2153-core.c: device init failed ! \n\n");
	return ret;
}
EXPORT_SYMBOL_GPL(d2153_device_init);


/*
 * d2153_device_exit
 */
void d2153_device_exit(struct d2153 *d2153)
{
	if(u2_get_board_rev() <= 4) {
		dlg_info("%s is called on old Board revision. error\n", __func__);
		return 0;
	}
#ifdef CONFIG_PROC_FS
	d2153_debug_proc_exit();
#endif
	d2153_dev_info = NULL;

	platform_device_unregister(d2153->batt.pdev);
	platform_device_unregister(d2153->rtc.pdev);
	platform_device_unregister(d2153->onkey.pdev);
	d2153_codec_dev_unregister(d2153->codec_i2c_client);

	d2153_free_irq(d2153, D2153_IRQ_EVDD_MON);
	d2153_irq_exit(d2153);
}
EXPORT_SYMBOL_GPL(d2153_device_exit);

//AFO extern int d2153_get_last_vbat_adc(void);
//AFO extern int d2153_get_last_temp_adc(void);


/*
 * d2153_system_poweroff
 */
void d2153_system_poweroff(void)
{
	u8 dst;
	struct d2153 *d2153 = d2153_dev_info;

	dlg_info("%s\n", __func__);

	if(u2_get_board_rev() <= 4) {
		dlg_info("%s is called on old Board revision. error\n", __func__);
		return;
	}	
	if(d2153 == NULL || d2153->read_dev == NULL) {
		dlg_err("%s. Platform or Device data is NULL\n", __func__);
		return;
	}
	
	// Disable OTP reloading and MCTL
	d2153_clear_bits(d2153, D2153_CONTROL_B_REG, 		D2153_OTPREAD_EN_MASK);
	d2153_clear_bits(d2153, D2153_POWER_CONT_REG, 		D2153_MCTRL_EN_MASK);

	// Disable nOnkey interrupt
	d2153_reg_write(d2153, D2153_IRQ_MASK_B_REG, 0x0);	//onkey mask clear

	// Disable LDOs(LDO2, LDO6 ~ LDO20, LDOAUD1 and LDOAUD2)
	d2153_clear_bits(d2153, D2153_LDO2_REG, 			(0x1<<6));
	d2153_clear_bits(d2153, D2153_LDO6_REG, 			(0x1<<6));
	d2153_clear_bits(d2153, D2153_LDO7_REG, 			(0x1<<6));
	d2153_clear_bits(d2153, D2153_LDO8_REG, 			(0x1<<6));
	d2153_clear_bits(d2153, D2153_LDO9_REG, 			(0x1<<6));
	d2153_clear_bits(d2153, D2153_LDO10_REG, 			(0x1<<6));
	d2153_clear_bits(d2153, D2153_LDO11_REG, 			(0x1<<6));
	d2153_clear_bits(d2153, D2153_LDO12_REG, 			(0x1<<6));
	d2153_clear_bits(d2153, D2153_LDO13_REG, 			(0x1<<6));
	d2153_clear_bits(d2153, D2153_LDO14_REG, 			(0x1<<6));
	d2153_clear_bits(d2153, D2153_LDO15_REG, 			(0x1<<6));
	d2153_clear_bits(d2153, D2153_LDO16_REG, 			(0x1<<6));
	d2153_clear_bits(d2153, D2153_LDO17_REG, 			(0x1<<6));
	d2153_clear_bits(d2153, D2153_LDO19_LDO_19_REG, 	(0x1<<6));
	d2153_clear_bits(d2153, D2153_LDO20_LDO_20_REG, 	(0x1<<6));
	d2153_clear_bits(d2153, D2153_LDO21_LDO_AUD1_REG, 	(0x1<<6));
	d2153_clear_bits(d2153, D2153_LDO22_LDO_AUD2_REG, 	(0x1<<6));

	//Do BUCK5 and LDO18 are turned off even if these are controlled by HOST's GPIO ???
	//d2153_clear_bits(d2153, D2153_LDO18_LDO_VRFANA_REG, (0x1<<6));
	//d2153_clear_bits(d2153, D2153_BUCKPERI_BUCK5_REG,   (0x1<<6));

	d2153_reg_write(d2153, D2153_POWER_CONT_REG, 0x0E);   // Have to check.
	d2153_reg_write(d2153, D2153_PD_DIS_REG, 0xCF);

	d2153_reg_read(d2153, D2153_CONTROL_B_REG, &dst);
	dst |= D2153_DEEP_SLEEP_MASK;
	d2153_reg_write(d2153, D2153_CONTROL_B_REG, dst);
	
	return;
}
EXPORT_SYMBOL(d2153_system_poweroff);


MODULE_AUTHOR("Dialog Semiconductor Ltd < william.seo@diasemi.com >");
MODULE_DESCRIPTION("D2153 PMIC Core");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:" D2153_I2C);
