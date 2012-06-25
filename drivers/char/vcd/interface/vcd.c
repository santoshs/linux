/* vcd.c
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
#include <linux/platform_device.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/semaphore.h>
#include <linux/miscdevice.h>
#include <linux/mm.h>

#include "linux/vcd/vcd_common.h"
#include "linux/vcd/vcd_control.h"
#include "vcd.h"


/*
 * global variable declaration
 */
DEFINE_SEMAPHORE(g_vcd_semaphore);
void (*g_vcd_complete_buffer)(void);
void (*g_vcd_beginning_buffer)(void);
void (*g_vcd_stop_fw)(void);
void (*g_vcd_start_clkgen)(void);
static struct proc_dir_entry *g_vcd_parent;
unsigned int g_vcd_log_level;

int g_vcd_debug_call_type;
int g_vcd_debug_mode;

/*
 * table declaration
 */
static struct vcd_execute_func vcd_func_table[] = {
	{ VCD_COMMAND_START_RECORD,		vcd_start_record	},
	{ VCD_COMMAND_STOP_RECORD,		vcd_stop_record		},
	{ VCD_COMMAND_START_PLAYBACK,		vcd_start_playback	},
	{ VCD_COMMAND_STOP_PLAYBACK,		vcd_stop_playback	},
	{ VCD_COMMAND_GET_RECORD_BUFFER,	vcd_get_record_buffer	},
	{ VCD_COMMAND_GET_PLAYBACK_BUFFER,	vcd_get_playback_buffer	},
	{ VCD_COMMAND_WATCH_STOP_FW,		vcd_watch_stop_fw	},
	{ VCD_COMMAND_WATCH_START_CLKGEN,	vcd_watch_start_clkgen	}
};

/*
 * file object
 */
static const struct file_operations vcd_fops = {
	.owner		= THIS_MODULE,
	.read		= NULL,
	.write		= NULL,
	.poll		= NULL,
	.unlocked_ioctl	= NULL,
	.open		= vcd_fops_open,
	.release	= vcd_fops_release,
	.mmap		= vcd_fops_mmap,
};

/*
 * misc device object
 */
static struct miscdevice vcd_misc_dev = {
	.minor			= MISC_DYNAMIC_MINOR,
	.name			= VCD_DEVICE_NAME,
	.fops			= &vcd_fops,
};

/*
 * callback object
 */
static const struct dev_pm_ops vcd_dev_pm_ops = {
	.suspend		= vcd_suspend,
	.resume			= vcd_resume,
	.runtime_suspend	= vcd_runtime_suspend,
	.runtime_resume		= vcd_runtime_resume,
};

/*
 * driver object
 */
static struct platform_driver vcd_platform_driver = {
	.driver		= {
		.name	= VCD_DRIVER_NAME,
		.pm	= &vcd_dev_pm_ops,
		.probe = NULL,
		.remove = NULL,
	},
};


/* ========================================================================= */
/* Internal public notification functions                                    */
/* ========================================================================= */

/**
 * @brief	record buffer write complete function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_complete_buffer(void)
{
	vcd_pr_start_interface_function();

	if (NULL != g_vcd_complete_buffer)
		g_vcd_complete_buffer();

	vcd_pr_end_interface_function();
	return;
}


/**
 * @brief	playback buffer read start function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_beginning_buffer(void)
{
	vcd_pr_start_interface_function();

	if (NULL != g_vcd_beginning_buffer)
		g_vcd_beginning_buffer();

	vcd_pr_end_interface_function();
	return;
}


/**
 * @brief	fw stop notification function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_stop_fw(void)
{
	vcd_pr_start_if_user();

	if (NULL != g_vcd_stop_fw)
		g_vcd_stop_fw();

	g_vcd_complete_buffer = NULL;
	g_vcd_beginning_buffer = NULL;

	vcd_pr_end_if_user();
	return;
}


/**
 * @brief	start clkgen timing notification function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_start_clkgen(void)
{
	vcd_pr_start_if_user();

	if (NULL != g_vcd_start_clkgen)
		g_vcd_start_clkgen();

	vcd_pr_end_if_user();
	return;
}


/* ========================================================================= */
/* External public functions                                                 */
/* ========================================================================= */

/**
 * @brief	VCD process execute function.
 *
 * @param[in]	args	pointer of command information
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	VCD_ERR_PARAM	invalid argument.
 * @retval	others		result of called function.
 */
int vcd_execute(const struct vcd_execute_command *args)
{
	int ret = VCD_ERR_NONE;
	int loop_count = 0;

	/* semaphore start */
	down(&g_vcd_semaphore);

	/* check system_rev */
	if ((system_rev & 0xff) == 0x00) {
		vcd_pr_err("system_rev[%x].\n", system_rev);
		ret = VCD_ERR_NOT_SUPPORT;
		goto rtn;
	}

	/* check parameter */
	if (NULL == args) {
		vcd_pr_start_if_user("args[%p].\n", args);
		vcd_pr_err("parameter error. args[%p].\n", args);
		ret = VCD_ERR_PARAM;
		goto rtn;
	}

	vcd_pr_start_if_user("args->command[%d].\n", args->command);

	/* function switcher */
	for (loop_count = 0; loop_count < VCD_COMMAND_MAX; loop_count++) {
		if (vcd_func_table[loop_count].command == args->command) {
			/* execute private function */
			ret = vcd_func_table[loop_count].func(args->arg);
			goto rtn;
		}
	}

	vcd_pr_err("parameter error. args->command[%d].\n", args->command);
	ret = VCD_ERR_PARAM;

rtn:
	vcd_pr_end_if_user("ret[%d] args->command[%d].\n", ret, args->command);

	/* semaphore end */
	up(&g_vcd_semaphore);

	return ret;
}
EXPORT_SYMBOL_GPL(vcd_execute);


/* ========================================================================= */
/* Internal functions                                                        */
/* ========================================================================= */

/**
 * @brief	get msg buffer function.
 *
 * @param	none.
 *
 * @retval	none.
 */
static void vcd_get_msg_buffer(void)
{
	vcd_pr_start_interface_function();

	/* execute control function */
	vcd_ctrl_get_msg_buffer();

	vcd_pr_end_interface_function();
}


/**
 * @brief	start vcd function.
 *
 * @param	none.
 *
 * @retval	none.
 */
static void vcd_start_vcd(void)
{
	vcd_pr_start_interface_function();

	/* execute control function */
	vcd_ctrl_start_vcd();

	vcd_pr_end_interface_function();
}


/**
 * @brief	stop vcd function.
 *
 * @param	none.
 *
 * @retval	none.
 */
static void vcd_stop_vcd(void)
{
	vcd_pr_start_interface_function();

	/* execute control function */
	vcd_ctrl_stop_vcd();

	/* close notification */
	vcd_stop_fw();

	/* initialize variable */
	g_vcd_complete_buffer	= NULL;
	g_vcd_beginning_buffer	= NULL;

	vcd_pr_end_interface_function();
}


/**
 * @brief	set hw param function.
 *
 * @param	none.
 *
 * @retval	none.
 */
static void vcd_set_hw_param(void)
{
	vcd_pr_start_interface_function();

	/* execute control function */
	vcd_ctrl_set_hw_param();

	vcd_pr_end_interface_function();
}


/**
 * @brief	start call function.
 *
 * @param	none.
 *
 * @retval	none.
 */
static void vcd_start_call(void)
{
	vcd_pr_start_interface_function();

	/* execute control function */
	vcd_ctrl_start_call(g_vcd_debug_call_type, g_vcd_debug_mode);

	vcd_pr_end_interface_function();
}


/**
 * @brief	stop call function.
 *
 * @param	none.
 *
 * @retval	none.
 */
static void vcd_stop_call(void)
{
	vcd_pr_start_interface_function();

	/* execute control function */
	vcd_ctrl_stop_call(g_vcd_debug_call_type);

	vcd_pr_end_interface_function();
}


/**
 * @brief	start tty/ctm function.
 *
 * @param	none.
 *
 * @retval	none.
 */
static void vcd_start_tty_ctm(void)
{
	vcd_pr_start_interface_function();

	/* execute control function */
	vcd_ctrl_start_tty_ctm();

	vcd_pr_end_interface_function();
}


/**
 * @brief	stop tty/ctm function.
 *
 * @param	none.
 *
 * @retval	none.
 */
static void vcd_stop_tty_ctm(void)
{
	vcd_pr_start_interface_function();

	/* execute control function */
	vcd_ctrl_stop_tty_ctm();

	vcd_pr_end_interface_function();
}


/**
 * @brief	config tty/ctm function.
 *
 * @param	none.
 *
 * @retval	none.
 */
static void vcd_config_tty_ctm(void)
{
	vcd_pr_start_interface_function();

	/* execute control function */
	vcd_ctrl_config_tty_ctm();

	vcd_pr_end_interface_function();
}


/**
 * @brief	set udata function.
 *
 * @param	none.
 *
 * @retval	none.
 */
static void vcd_set_udata(void)
{
	vcd_pr_start_interface_function();

	/* execute control function */
	vcd_ctrl_set_udata();

	vcd_pr_end_interface_function();
}


/**
 * @brief	get status function.
 *
 * @param	none.
 *
 * @retval	none.
 */
static void vcd_get_status(void)
{
	vcd_pr_start_interface_function();

	/* execute control function */
	vcd_ctrl_get_status();

	vcd_pr_end_interface_function();
}


/**
 * @brief	start record function.
 *
 * @param[in]	arg	pointer of record option structure.
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	VCD_ERR_PARAM	parameter error.
 * @retval	others		result of called function.
 */
static int vcd_start_record(void *arg)
{
	int ret = VCD_ERR_NONE;
	struct vcd_record_option option = {0};

	vcd_pr_start_interface_function("arg[%p].\n", arg);

	vcd_pr_if_sound("V <-- S : VCD_COMMAND_START_RECORD.\n");

	/* check parameter */
	if (NULL == arg) {
		vcd_pr_err("parameter error. arg[%p].\n", arg);
		ret = VCD_ERR_PARAM;
		goto rtn;
	}

	memcpy(&option, arg, sizeof(option));
	vcd_pr_interface_info("option.mode[%d].\n", option.mode);
	vcd_pr_interface_info("option.complete_buffer[%p].\n",
		option.complete_buffer);

	if ((VCD_RECORD_MODE_0 > option.mode) ||
		(VCD_RECORD_MODE_2  < option.mode)) {
		vcd_pr_err("parameter error. option.mode[%d].\n", option.mode);
		ret = VCD_ERR_PARAM;
		goto rtn;
	}

	if (NULL == option.complete_buffer) {
		vcd_pr_err("parameter error. option.complete_buffer[%p].\n",
			option.complete_buffer);
		ret = VCD_ERR_PARAM;
		goto rtn;
	}

	/* register notify function */
	g_vcd_complete_buffer = option.complete_buffer;

	/* execute control function */
	ret = vcd_ctrl_start_record(&option);

rtn:
	vcd_pr_end_interface_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	stop record function.
 *
 * @param	arg	unused.
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	others		result of called function.
 */
static int vcd_stop_record(void *arg)
{
	int ret = VCD_ERR_NONE;

	vcd_pr_start_interface_function("arg[%p].\n", arg);

	vcd_pr_if_sound("V <-- S : VCD_COMMAND_STOP_RECORD.\n");

	/* initialize variable */
	g_vcd_complete_buffer	= NULL;

	/* execute control function */
	ret = vcd_ctrl_stop_record();

	vcd_pr_end_interface_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	start playback function.
 *
 * @param[in]	arg	pointer of playback option structure.
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	VCD_ERR_PARAM	parameter error.
 * @retval	others		result of called function.
 */
static int vcd_start_playback(void *arg)
{
	int ret = VCD_ERR_NONE;
	struct vcd_playback_option option = {0};

	vcd_pr_start_interface_function("arg[%p].\n", arg);

	vcd_pr_if_sound("V <-- S : VCD_COMMAND_START_PLAYBACK.\n");

	/* check parameter */
	if (NULL == arg) {
		vcd_pr_err("parameter error. arg[%p].\n", arg);
		ret = VCD_ERR_PARAM;
		goto rtn;
	}

	memcpy(&option, arg, sizeof(option));
	vcd_pr_interface_info("option.mode[%d].\n", option.mode);
	vcd_pr_interface_info("option.beginning_buffer[%p].\n",
					option.beginning_buffer);

	if ((VCD_PLAYBACK_MODE_0 > option.mode) ||
		(VCD_PLAYBACK_MODE_2  < option.mode)) {
		vcd_pr_err("parameter error. option.mode[%d].\n", option.mode);
		ret = VCD_ERR_PARAM;
		goto rtn;
	}

	if (NULL == option.beginning_buffer) {
		vcd_pr_err("parameter error. option.beginning_buffer[%p].\n",
			option.beginning_buffer);
		ret = VCD_ERR_PARAM;
		goto rtn;
	}

	/* register notify function */
	g_vcd_beginning_buffer = option.beginning_buffer;

	/* execute control function */
	ret = vcd_ctrl_start_playback(&option);

rtn:
	vcd_pr_end_interface_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	stop playback function.
 *
 * @param	arg	unused.
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	others		result of called function.
 */
static int vcd_stop_playback(void *arg)
{
	int ret = VCD_ERR_NONE;

	vcd_pr_start_interface_function("arg[%p].\n", arg);

	vcd_pr_if_sound("V <-- S : VCD_COMMAND_STOP_PLAYBACK.\n");

	/* initialize variable */
	g_vcd_beginning_buffer	= NULL;

	/* execute control function */
	ret = vcd_ctrl_stop_playback();

	vcd_pr_end_interface_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	get record buffer function.
 *
 * @param[out]	arg	pointer of record buffer info structure.
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	VCD_ERR_PARAM	parameter error.
 * @retval	others		result of called function.
 */
static int vcd_get_record_buffer(void *arg)
{
	int ret = VCD_ERR_NONE;

	vcd_pr_start_interface_function("arg[%p].\n", arg);

	vcd_pr_if_sound("V <-- S : VCD_COMMAND_GET_RECORD_BUFFER.\n");

	/* check parameter */
	if (NULL == arg) {
		vcd_pr_err("parameter error. arg[%p].\n", arg);
		ret = VCD_ERR_PARAM;
		goto rtn;
	}

	/* execute control function */
	vcd_ctrl_get_record_buffer(arg);

rtn:
	vcd_pr_end_interface_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	get playback buffer function.
 *
 * @param[out]	arg	pointer of playback buffer info structure.
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	VCD_ERR_PARAM	parameter error.
 * @retval	others		result of called function.
 */
static int vcd_get_playback_buffer(void *arg)
{
	int ret = VCD_ERR_NONE;

	vcd_pr_start_interface_function("arg[%p].\n", arg);

	vcd_pr_if_sound("V <-- S : VCD_COMMAND_GET_PLAYBACK_BUFFER.\n");

	/* check parameter */
	if (NULL == arg) {
		vcd_pr_err("parameter error. arg[%p].\n", arg);
		ret = VCD_ERR_PARAM;
		goto rtn;
	}

	/* execute control function */
	vcd_ctrl_get_playback_buffer(arg);

rtn:
	vcd_pr_end_interface_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	watch stop fw function.
 *
 * @param[in]	arg	pointer of notify function.
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	VCD_ERR_PARAM	parameter error.
 * @retval	others		result of called function.
 */
static int vcd_watch_stop_fw(void *arg)
{
	int ret = VCD_ERR_NONE;

	vcd_pr_start_interface_function("arg[%p].\n", arg);

	vcd_pr_if_sound("V <-- S : VCD_COMMAND_WATCH_STOP_FW.\n");

	/* check parameter */
	if (NULL == arg) {
		vcd_pr_err("parameter error. arg[%p].\n", arg);
		ret = VCD_ERR_PARAM;
		goto rtn;
	}

	/* register notify function */
	g_vcd_stop_fw = arg;

rtn:
	vcd_pr_end_interface_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	watch start clkgen function.
 *
 * @param[in]	arg	pointer of notify function.
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	VCD_ERR_PARAM	parameter error.
 * @retval	others		result of called function.
 */
static int vcd_watch_start_clkgen(void *arg)
{
	int ret = VCD_ERR_NONE;

	vcd_pr_start_interface_function("arg[%p].\n", arg);

	vcd_pr_if_sound("V <-- S : VCD_COMMAND_WATCH_START_CLKGEN.\n");

	/* check parameter */
	if (NULL == arg) {
		vcd_pr_err("parameter error. arg[%p].\n", arg);
		ret = VCD_ERR_PARAM;
		goto rtn;
	}

	/* register notify function */
	g_vcd_start_clkgen = arg;

rtn:
	vcd_pr_end_interface_function("ret[%d].\n", ret);
	return ret;
}


/* ========================================================================= */
/* Proc functions                                                            */
/* ========================================================================= */

/**
 * @brief	execute process read function.
 *
 * @param[in]	page	write position.
 * @param[in]	start	unused.
 * @param[in]	offset	unused.
 * @param[in]	count	maximum write length.
 * @param[in]	eof	unused.
 * @param[in]	data	unused.
 *
 * @retval	len	write length.
 */
static int vcd_read_exec_proc(char *page, char **start, off_t offset,
					int count, int *eof, void *data)
{
	int len = 0;
	int result = 0;

	/* semaphore start */
	down(&g_vcd_semaphore);

	vcd_pr_start_if_user();

	/* execute control function */
	result = vcd_ctrl_get_result();

	if ((system_rev & 0xff) == 0x00)
		result =  VCD_ERR_NOT_SUPPORT;

	len = snprintf(page, count, "%d\n", result);

	vcd_pr_if_amhal("V --> A : [%x].\n", result);

	vcd_pr_end_if_user("result[%d].\n", result);

	/* semaphore end */
	up(&g_vcd_semaphore);

	return len;
}


/**
 * @brief	execute process write function.
 *
 * @param[in]	filp	unused.
 * @param[in]	buffer	user data.
 * @param[in]	len	length of data.
 * @param[in]	data	unused.
 *
 * @retval	len	read length.
 */
static int vcd_write_exec_proc(struct file *filp, const char *buffer,
					unsigned long len, void *data)
{
	int ret = VCD_ERR_NONE;
	unsigned char proc_buf[VCD_PROC_BUF_SIZE] = {0};
	unsigned int exec_proc = 0;

	/* semaphore start */
	down(&g_vcd_semaphore);

	/* check system_rev */
	if ((system_rev & 0xff) == 0x00) {
		vcd_pr_err("system_rev[%x].\n", system_rev);
		goto rtn;
	}

	/* buffer size check */
	if (VCD_PROC_BUF_SIZE <= len) {
		vcd_pr_err("size over len[%ld].\n", len);
		goto rtn;
	}

	/* copy from user */
	if (copy_from_user(proc_buf, (void __user *)buffer, len)) {
		vcd_pr_err("copy_from_user failed.\n");
		goto rtn;
	}

	/* get exec proc */
	ret = kstrtouint(proc_buf, 0, &exec_proc);
	if (0 != ret) {
		vcd_pr_err("entry num error. proc_buf[%s].\n", proc_buf);
		goto rtn;
	}
	vcd_pr_start_if_user("exec_proc[%d].\n", exec_proc);

	switch (exec_proc) {
	case VCD_PROC_IF_GET_MSG_BUFFER:
		vcd_pr_if_amhal(VCD_PROC_IF_GET_MSG_BUFFER_LOG);
		vcd_get_msg_buffer();
		break;
	case VCD_PROC_IF_START_VCD:
		vcd_pr_if_amhal(VCD_PROC_IF_START_VCD_LOG);
		vcd_start_vcd();
		break;
	case VCD_PROC_IF_STOP_VCD:
		vcd_pr_if_amhal(VCD_PROC_IF_STOP_VCD_LOG);
		vcd_stop_vcd();
		break;
	case VCD_PROC_IF_SET_HW_PARAM:
		vcd_pr_if_amhal(VCD_PROC_IF_SET_HW_PARAM_LOG);
		vcd_set_hw_param();
		break;
	case VCD_PROC_IF_START_CALL:
		vcd_pr_if_amhal(VCD_PROC_IF_START_CALL_LOG);
		vcd_start_call();
		break;
	case VCD_PROC_IF_STOP_CALL:
		vcd_pr_if_amhal(VCD_PROC_IF_STOP_CALL_LOG);
		vcd_stop_call();
		break;
	case VCD_PROC_IF_START_TTY_CTM:
		vcd_pr_if_amhal(VCD_PROC_IF_START_TTY_CTM_LOG);
		vcd_start_tty_ctm();
		break;
	case VCD_PROC_IF_STOP_TTY_CTM:
		vcd_pr_if_amhal(VCD_PROC_IF_STOP_TTY_CTM_LOG);
		vcd_stop_tty_ctm();
		break;
	case VCD_PROC_IF_CONFIG_TTY_CTM:
		vcd_pr_if_amhal(VCD_PROC_IF_CONFIG_TTY_CTM_LOG);
		vcd_config_tty_ctm();
		break;
	case VCD_PROC_IF_SET_UDATA:
		vcd_pr_if_amhal(VCD_PROC_IF_SET_UDATA_LOG);
		vcd_set_udata();
		break;
	case VCD_PROC_IF_GET_STATUS:
		vcd_pr_if_amhal(VCD_PROC_IF_GET_STATUS_LOG);
		vcd_get_status();
		break;
	default:
		vcd_pr_err("write number failed.\n");
		goto rtn;
	}

rtn:
	vcd_pr_end_if_user("len[%ld].\n", len);

	/* semaphore end */
	up(&g_vcd_semaphore);

	return len;
}


/**
 * @brief	log level read function.
 *
 * @param[in]	page	write position.
 * @param[in]	start	unused.
 * @param[in]	offset	unused.
 * @param[in]	count	maximum write length.
 * @param[in]	eof	unused.
 * @param[in]	data	unused.
 *
 * @retval	len	write length.
 */
static int vcd_read_log_level(char *page, char **start, off_t offset,
					int count, int *eof, void *data)
{
	int len = 0;

	vcd_pr_start_interface_function();

#ifdef __PRINT_VCD__
	len = snprintf(page, count, "0x%x\n", g_vcd_log_level);
#endif /* __PRINT_VCD__ */

	vcd_pr_end_interface_function("len[%d].\n", len);
	return len;
}


/**
 * @brief	log level write function.
 *
 * @param[in]	filp	unused.
 * @param[in]	buffer	user data.
 * @param[in]	len	length of data.
 * @param[in]	data	unused.
 *
 * @retval	len	read length.
 */
static int vcd_write_log_level(struct file *filp, const char *buffer,
						unsigned long len, void *data)
{
	int ret = VCD_ERR_NONE;
	unsigned char proc_buf[VCD_PROC_BUF_SIZE] = {0};
	unsigned int temp_log_level = VCD_LOG_NONE;

	vcd_pr_start_interface_function();

#ifndef __PRINT_VCD__
	goto rtn;
#endif /* __PRINT_VCD__ */

	/* buffer size check */
	if (VCD_PROC_BUF_SIZE <= len) {
		vcd_pr_err("size over len[%ld].\n", len);
		goto rtn;
	}

	/* copy from user */
	if (copy_from_user(proc_buf, (void __user *)buffer, len)) {
		vcd_pr_err("copy_from_user failed.\n");
		goto rtn;
	}

	/* get log level */
	ret = kstrtouint(proc_buf, 0, &temp_log_level);
	if (0 != ret) {
		vcd_pr_err("entry num error. proc_buf[%s].\n", proc_buf);
		goto rtn;
	}

	/* check log level */
	if ((VCD_LOG_LEVEL_UPPER < temp_log_level) &&
		((!(VCD_LOG_LEVEL_LOCK & temp_log_level)) ||
		(VCD_LOG_LEVEL_CONDITION_LOCK & temp_log_level))) {
		temp_log_level = VCD_LOG_LEVEL_UPPER;
	}

	/* set log level */
	g_vcd_log_level = temp_log_level;

rtn:
	vcd_pr_interface_info("g_vcd_log_level[0x%x].\n", g_vcd_log_level);
	vcd_pr_end_interface_function("len[%ld].\n", len);
	return len;
}


/**
 * @brief	exec func read function.
 *
 * @param[in]	page	write position.
 * @param[in]	start	unused.
 * @param[in]	offset	unused.
 * @param[in]	count	maximum write length.
 * @param[in]	eof	unused.
 * @param[in]	data	unused.
 *
 * @retval	len	write length.
 */
static int vcd_read_exec_func(char *page, char **start, off_t offset,
					int count, int *eof, void *data)
{
	int len = 0;

	vcd_pr_start_interface_function();

	/* nop */

	vcd_pr_end_interface_function("len[%d].\n", len);
	return len;
}


/**
 * @brief	exec func write function.
 *
 * @param[in]	filp	unused.
 * @param[in]	buffer	user data.
 * @param[in]	len	length of data.
 * @param[in]	data	unused.
 *
 * @retval	count	read length.
 */
static int vcd_write_exec_func(struct file *filp, const char *buffer,
					unsigned long len, void *data)
{
	int ret = VCD_ERR_NONE;
	unsigned char proc_buf[VCD_PROC_BUF_SIZE] = {0};
	unsigned int write_func = 0;

	vcd_pr_start_interface_function(
		"filp[%p],buffer[%p],len[%ld],data[%p].\n",
		filp, buffer, len, data);

#ifndef __DEBUG__
	goto rtn;
#endif /* __DEBUG__ */

	/* buffer size check */
	if (VCD_PROC_BUF_SIZE <= len) {
		vcd_pr_err("size over len[%ld].\n", len);
		goto rtn;
	}

	/* copy from user */
	if (copy_from_user(proc_buf, (void __user *)buffer, len)) {
		vcd_pr_err("copy_from_user failed.\n");
		goto rtn;
	}

	/* write execute function */
	ret = kstrtouint(proc_buf, 0, &write_func);
	if (0 != ret) {
		vcd_pr_err("entry num error. proc_buf[%s].\n", proc_buf);
		goto rtn;
	}

	/* execute vcd_execute() */
	vcd_debug_execute(write_func);

rtn:
	vcd_pr_end_interface_function("len[%ld].\n", len);
	return len;
}


/**
 * @brief	debug execute function.
 *
 * @param[in]	command	execute command.
 *
 * @retval	ret	result.
 */
static int vcd_debug_execute(unsigned int command)
{
	int ret = VCD_ERR_NONE;
	struct vcd_execute_command args = {0};
	struct vcd_record_option record_option = {0};
	struct vcd_playback_option playback_option = {0};
	struct vcd_record_buffer_info record_buf_info = { {0} };
	struct vcd_playback_buffer_info playback_buf_info = { {0} };
	unsigned int temp_log_level = VCD_LOG_NONE;

	vcd_pr_start_interface_function("command[%d].\n", command);

	args.command = command;

	switch (args.command) {
	case VCD_COMMAND_START_RECORD:
		record_option.mode =  g_vcd_debug_mode;
		record_option.complete_buffer = vcd_debug_complete_buffer;
		args.arg = &record_option;
		break;
	case VCD_COMMAND_STOP_RECORD:
		args.arg = NULL;
		break;
	case VCD_COMMAND_START_PLAYBACK:
		playback_option.mode = g_vcd_debug_mode;
		playback_option.beginning_buffer = vcd_debug_beginning_buffer;
		args.arg = &playback_option;
		break;
	case VCD_COMMAND_STOP_PLAYBACK:
		args.arg = NULL;
		break;
	case VCD_COMMAND_GET_RECORD_BUFFER:
		args.arg = &record_buf_info;
		break;
	case VCD_COMMAND_GET_PLAYBACK_BUFFER:
		args.arg = &playback_buf_info;
		break;
	case VCD_COMMAND_WATCH_STOP_FW:
		args.arg = vcd_debug_watch_fw;
		break;
	default:
		/* check debug commands */
		goto debug;
	}

	/* execute */
	ret = vcd_execute(&args);
	goto rtn;

debug:
	/* enable dump log_level */
	temp_log_level = g_vcd_log_level;
	g_vcd_log_level = g_vcd_log_level | VCD_LOG_REGISTERS_DUMP;

	switch (args.command) {
	case VCD_DEBUG_DUMP_STATUS:
		/* execute control function */
		vcd_ctrl_dump_status();
		break;
	case VCD_DEBUG_DUMP_REGISTERS:
		/* execute control function */
		vcd_ctrl_dump_registers();
		break;
	case VCD_DEBUG_DUMP_CPG_REGISTERS:
		/* execute control function */
		vcd_ctrl_dump_cpg_registers();
		break;
	case VCD_DEBUG_DUMP_CRMU_REGISTERS:
		/* execute control function */
		vcd_ctrl_dump_crmu_registers();
		break;
	case VCD_DEBUG_DUMP_GTU_REGISTERS:
		/* execute control function */
		vcd_ctrl_dump_gtu_registers();
		break;
	case VCD_DEBUG_DUMP_VOICEIF_REGISTERS:
		/* execute control function */
		vcd_ctrl_dump_voiceif_registers();
		break;
	case VCD_DEBUG_DUMP_INTCVO_REGISTERS:
		/* execute control function */
		vcd_ctrl_dump_intcvo_registers();
		break;
	case VCD_DEBUG_DUMP_SPUV_REGISTERS:
		/* execute control function */
		vcd_ctrl_dump_spuv_registers();
		break;
	case VCD_DEBUG_DUMP_DSP0_REGISTERS:
		/* execute control function */
		vcd_ctrl_dump_dsp0_registers();
		break;
	case VCD_DEBUG_SET_CS_CALL_MODE:
		g_vcd_debug_call_type = 0;
		break;
	case VCD_DEBUG_SET_1KHZ_TONE_MODE:
		g_vcd_debug_call_type = 1;
		break;
	case VCD_DEBUG_SET_PCM_LOOPBACK_MODE:
		g_vcd_debug_call_type = 2;
		break;
	case VCD_DEBUG_SET_VIF_LOOPBACK_MODE:
		g_vcd_debug_call_type = 3;
		break;
	case VCD_DEBUG_SET_MODE_0:
		g_vcd_debug_mode = 0;
		break;
	case VCD_DEBUG_SET_MODE_1:
		g_vcd_debug_mode = 1;
		break;
	case VCD_DEBUG_SET_MODE_2:
		g_vcd_debug_mode = 2;
		break;
	case VCD_DEBUG_SET_MODE_3:
		g_vcd_debug_mode = 3;
		break;
	default:
		vcd_pr_err("parameter error. command[%d].\n", command);
		break;
	}

	/* disable dump log_level */
	g_vcd_log_level = temp_log_level;

rtn:
	vcd_pr_end_interface_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	record buffer update debug function.
 *
 * @param	none.
 *
 * @retval	none.
 */
static void vcd_debug_complete_buffer(void)
{
	vcd_pr_start_interface_function();
	vcd_pr_end_interface_function();
}


/**
 * @brief	playback buffer update debug function.
 *
 * @param	none.
 *
 * @retval	none.
 */
static void vcd_debug_beginning_buffer(void)
{
	vcd_pr_start_interface_function();
	vcd_pr_end_interface_function();
}


/**
 * @brief	firmware stop notify debug function.
 *
 * @param	none.
 *
 * @retval	none.
 */
static void vcd_debug_watch_fw(void)
{
	vcd_pr_start_interface_function();
	vcd_pr_end_interface_function();
}


/**
 * @brief	create proc entry function.
 *
 * @param	none.
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	VCD_ERR_SYSTEM	create failure.
 */
static int vcd_create_proc_entry(void)
{
	int ret = VCD_ERR_NONE;
	struct proc_dir_entry *exec_proc = NULL;
	struct proc_dir_entry *log_level = NULL;
	struct proc_dir_entry *exec_func = NULL;

	vcd_pr_start_interface_function();


	/* make directory /proc/--- */
	g_vcd_parent = proc_mkdir(VCD_DRIVER_NAME, NULL);
	if (NULL != g_vcd_parent) {
		/* create file for execute process */
		exec_proc = create_proc_entry(VCD_PROC_FILE_NAME_EXEC_PROC,
				(S_IFREG | S_IRUGO | S_IWUGO), g_vcd_parent);
		if (NULL != exec_proc) {
			exec_proc->read_proc  = vcd_read_exec_proc;
			exec_proc->write_proc = vcd_write_exec_proc;
		} else {
			vcd_pr_always_err("create failed for exec.\n");
			ret = VCD_ERR_SYSTEM;
			goto rm_dir;
		}
		/* create file for log level */
		log_level = create_proc_entry(VCD_PROC_FILE_NAME_LOG_LEVEL,
				(S_IFREG | S_IRUGO | S_IWUGO), g_vcd_parent);
		if (NULL != log_level) {
			log_level->read_proc  = vcd_read_log_level;
			log_level->write_proc = vcd_write_log_level;
		} else {
			vcd_pr_always_err("create failed for log level.\n");
			ret = VCD_ERR_SYSTEM;
			goto rm_exec_proc;
		}
		/* create file for execute function */
		exec_func = create_proc_entry(VCD_PROC_FILE_NAME_EXEC_FUNC,
				(S_IFREG | S_IRUGO | S_IWUGO), g_vcd_parent);
		if (NULL != exec_func) {
			exec_func->read_proc  = vcd_read_exec_func;
			exec_func->write_proc = vcd_write_exec_func;
		} else {
			vcd_pr_always_err("create failed for exec func.\n");
			ret = VCD_ERR_SYSTEM;
			goto rm_log_proc;
		}
	} else {
		vcd_pr_always_err("create failed for proc parent.\n");
		ret = VCD_ERR_SYSTEM;
		goto rtn;
	}

	goto rtn;

rm_log_proc:
	remove_proc_entry(VCD_PROC_FILE_NAME_LOG_LEVEL, g_vcd_parent);
rm_exec_proc:
	remove_proc_entry(VCD_PROC_FILE_NAME_EXEC_PROC, g_vcd_parent);
rm_dir:
	remove_proc_entry(VCD_DRIVER_NAME, NULL);
	g_vcd_parent = NULL;
rtn:
	vcd_pr_end_interface_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	remove proc entry function.
 *
 * @param	none.
 *
 * @retval	none.
 */
static void vcd_remove_proc_entry(void)
{
	vcd_pr_start_interface_function();

	if (NULL != g_vcd_parent) {

		remove_proc_entry(VCD_PROC_FILE_NAME_EXEC_FUNC, g_vcd_parent);
		remove_proc_entry(VCD_PROC_FILE_NAME_LOG_LEVEL, g_vcd_parent);
		remove_proc_entry(VCD_PROC_FILE_NAME_EXEC_PROC, g_vcd_parent);
		remove_proc_entry(VCD_DRIVER_NAME, NULL);
		g_vcd_parent = NULL;
	}

	vcd_pr_end_interface_function();
}


/* ========================================================================= */
/* Driver functions                                                          */
/* ========================================================================= */

/**
 * @brief	module init function.
 *
 * @param	none.
 *
 * @retval	ret	result.
 */
static int vcd_probe(void)
{
	int ret = VCD_ERR_NONE;

	vcd_pr_start_interface_function();

	/* register driver */
	ret = platform_driver_register(&vcd_platform_driver);
	if (VCD_ERR_NONE != ret) {
		vcd_pr_always_err("driver registration error.\n");
		goto rtn;
	}

	/* register misc */
	ret = misc_register(&vcd_misc_dev);
	if (ret) {
		vcd_pr_always_err("misc_register error.\n");
		goto unreg_driver;
	}

	/* execute control function */
	ret = vcd_ctrl_probe();
	if (VCD_ERR_NONE != ret) {
		vcd_pr_always_err("vcd_ctrl_probe error.\n");
		goto unreg_misc;
	}

	ret = vcd_create_proc_entry();
	if (VCD_ERR_NONE != ret) {
		vcd_pr_always_err("vcd_create_proc_entry error.\n");
		goto ctrl_remove;
	}

	goto rtn;

ctrl_remove:
	vcd_ctrl_remove();
unreg_misc:
	misc_deregister(&vcd_misc_dev);
unreg_driver:
	platform_driver_unregister(&vcd_platform_driver);
rtn:
	vcd_pr_end_interface_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	module exit function.
 *
 * @param	none.
 *
 * @retval	none.
 */
static void vcd_remove(void)
{
	vcd_pr_start_interface_function();

	/* remove proc entry */
	vcd_remove_proc_entry();

	/* execute control function */
	vcd_ctrl_remove();

	/* deregister misc */
	misc_deregister(&vcd_misc_dev);

	/* unregister driver */
	platform_driver_unregister(&vcd_platform_driver);

	vcd_pr_end_interface_function();
	return;
}


/**
 * @brief	suspend notification receive function.
 *
 * @param	dev	unused.
 *
 * @retval	ret	result.
 */
static int vcd_suspend(struct device *dev)
{
	int ret = VCD_ERR_NONE;

	/* semaphore start */
	down(&g_vcd_semaphore);

	vcd_pr_start_interface_function("dev[%p].\n", dev);

	/* execute control function */
	ret = vcd_ctrl_suspend();
	if (VCD_ERR_NONE != ret)
		vcd_pr_interface_debug(
			"VCD cannot enter the 'suspend' currently.\n");

	vcd_pr_end_interface_function("ret[%d].\n", ret);

	/* semaphore end */
	up(&g_vcd_semaphore);

	return ret;
}


/**
 * @brief	resume notification receive function.
 *
 * @param	dev	unused.
 *
 * @retval	ret	result.
 */
static int vcd_resume(struct device *dev)
{
	int ret = VCD_ERR_NONE;

	/* semaphore start */
	down(&g_vcd_semaphore);

	vcd_pr_start_interface_function("dev[%p].\n", dev);

	/* execute control function */
	vcd_ctrl_resume();

	vcd_pr_end_interface_function("ret[%d].\n", ret);

	/* semaphore end */
	up(&g_vcd_semaphore);

	return ret;
}


/**
 * @brief	runtime suspend notification receive function.
 *
 * @param	dev	unused.
 *
 * @retval	ret	result.
 */
static int vcd_runtime_suspend(struct device *dev)
{
	int ret = VCD_ERR_NONE;

	/* semaphore start */
	down(&g_vcd_semaphore);

	vcd_pr_start_interface_function("dev[%p].\n", dev);

	/* execute control function */
	ret = vcd_ctrl_runtime_suspend();
	if (VCD_ERR_NONE != ret)
		vcd_pr_interface_debug(
			"VCD cannot enter the 'runtime suspend' currently.\n");

	vcd_pr_end_interface_function("ret[%d].\n", ret);

	/* semaphore end */
	up(&g_vcd_semaphore);

	return ret;
}


/**
 * @brief	runtime resume notification receive function.
 *
 * @param	dev	unused.
 *
 * @retval	ret	result.
 */
static int vcd_runtime_resume(struct device *dev)
{
	int ret = VCD_ERR_NONE;

	/* semaphore start */
	down(&g_vcd_semaphore);

	vcd_pr_start_interface_function("dev[%p].\n", dev);

	/* execute control function */
	vcd_ctrl_runtime_resume();

	vcd_pr_end_interface_function("ret[%d].\n", ret);

	/* semaphore end */
	up(&g_vcd_semaphore);

	return ret;
}


/**
 * @brief	file operations open function.
 *
 * @param	inode	unused.
 * @param	filp	unused.
 *
 * @retval	VCD_ERR_NONE	result.
 */
static int vcd_fops_open(struct inode *inode, struct file *filp)
{
	return VCD_ERR_NONE;
}


/**
 * @brief	file operations release function.
 *
 * @param	inode	unused.
 * @param	filp	unused.
 *
 * @retval	VCD_ERR_NONE	result.
 */
static int vcd_fops_release(struct inode *inode, struct file *filp)
{
	return VCD_ERR_NONE;
}


/**
 * @brief	file operations mmap function.
 *
 * @param	fp	file struct.
 * @param	vma	vm area struct.
 *
 * @retval	ret	result.
 */
static int vcd_fops_mmap(struct file *fp, struct vm_area_struct *vma)
{
	int ret = VCD_ERR_NONE;

	ret = remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff,
			vma->vm_end - vma->vm_start, vma->vm_page_prot);

	return ret;
}


/**
 * @brief	module init function.
 *
 * @param	none.
 *
 * @retval	ret	result.
 */
static int __init vcd_module_init(void)
{
	int ret = VCD_ERR_NONE;

	vcd_pr_start_interface_function();

	ret = vcd_probe();

	vcd_pr_end_interface_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	module exit function.
 *
 * @param	none.
 *
 * @retval	none.
 */
static void __exit vcd_module_exit(void)
{
	vcd_pr_start_interface_function();

	vcd_remove();

	vcd_pr_end_interface_function();
}


module_init(vcd_module_init);
module_exit(vcd_module_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Renesas Mobile Corp.");
MODULE_DESCRIPTION("Voice Codec Driver");
MODULE_VERSION("0.5.0");
