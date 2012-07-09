/* vcd.h
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
#ifndef __VCD_IF_H__
#define __VCD_IF_H__

/*
 * constant definition
 */
#define VCD_PROC_FILE_NAME_EXEC_PROC	"exec_process"
#define VCD_PROC_FILE_NAME_LOG_LEVEL	"log_level"
#define VCD_PROC_FILE_NAME_EXEC_FUNC	"exec_func"

#define VCD_PROC_BUF_SIZE	32

#define VCD_PROC_IF_GET_MSG_BUFFER_LOG		\
		"V <-- A : '0'  - GET_MSG_BUFFER.\n"
#define VCD_PROC_IF_START_VCD_LOG		\
		"V <-- A : '1'  - START_VCD.\n"
#define VCD_PROC_IF_STOP_VCD_LOG		\
		"V <-- A : '2'  - STOP_VCD.\n"
#define VCD_PROC_IF_SET_HW_PARAM_LOG		\
		"V <-- A : '3'  - SET_HW_PARAM.\n"
#define VCD_PROC_IF_START_CALL_LOG		\
		"V <-- A : '4'  - START_CALL.\n"
#define VCD_PROC_IF_STOP_CALL_LOG		\
		"V <-- A : '5'  - STOP_CALL.\n"
#define VCD_PROC_IF_START_TTY_CTM_LOG		\
		"V <-- A : '6'  - START_TTY_CTM.\n"
#define VCD_PROC_IF_STOP_TTY_CTM_LOG		\
		"V <-- A : '7'  - STOP_TTY_CTM.\n"
#define VCD_PROC_IF_CONFIG_TTY_CTM_LOG		\
		"V <-- A : '8'  - CONFIG_TTY_CTM.\n"
#define VCD_PROC_IF_SET_UDATA_LOG		\
		"V <-- A : '9'  - SET_UDATA.\n"
#define VCD_PROC_IF_GET_STATUS_LOG		\
		"V <-- A : '10' - GET_STATUS.\n"


/*
 * define macro declaration
 */


/*
 * enum declaration
 */
enum VCD_PROC_IF {
	VCD_PROC_IF_GET_MSG_BUFFER = 0,
	VCD_PROC_IF_START_VCD,
	VCD_PROC_IF_STOP_VCD,
	VCD_PROC_IF_SET_HW_PARAM,
	VCD_PROC_IF_START_CALL,
	VCD_PROC_IF_STOP_CALL,
	VCD_PROC_IF_START_TTY_CTM,
	VCD_PROC_IF_STOP_TTY_CTM,
	VCD_PROC_IF_CONFIG_TTY_CTM,
	VCD_PROC_IF_SET_UDATA,
	VCD_PROC_IF_GET_STATUS
};

enum VCD_DEBUG_COMMAND {
	VCD_DEBUG_DUMP_STATUS = VCD_COMMAND_MAX,
	VCD_DEBUG_DUMP_REGISTERS,
	VCD_DEBUG_DUMP_CPG_REGISTERS,
	VCD_DEBUG_DUMP_CRMU_REGISTERS,
	VCD_DEBUG_DUMP_GTU_REGISTERS,
	VCD_DEBUG_DUMP_VOICEIF_REGISTERS,
	VCD_DEBUG_DUMP_INTCVO_REGISTERS,
	VCD_DEBUG_DUMP_SPUV_REGISTERS,
	VCD_DEBUG_DUMP_DSP0_REGISTERS,
	VCD_DEBUG_SET_CS_CALL_MODE,
	VCD_DEBUG_SET_1KHZ_TONE_MODE,
	VCD_DEBUG_SET_PCM_LOOPBACK_MODE,
	VCD_DEBUG_SET_VIF_LOOPBACK_MODE,
	VCD_DEBUG_SET_MODE_0,
	VCD_DEBUG_SET_MODE_1,
	VCD_DEBUG_SET_MODE_2,
	VCD_DEBUG_SET_MODE_3,
};


/*
 * structure declaration
 */
struct vcd_execute_func {
	unsigned int	command;
	int		(*func)(void *arg);
};


/*
 * table declaration
 */


/*
 * prototype declaration
 */

/* Internal public notification functions */
void vcd_complete_buffer(void);
void vcd_beginning_buffer(void);
void vcd_stop_fw(void);
void vcd_start_clkgen(void);

/* Internal functions */
static void vcd_get_msg_buffer(void);
static void vcd_start_vcd(void);
static void vcd_stop_vcd(void);
static void vcd_set_hw_param(void);
static void vcd_start_call(void);
static void vcd_stop_call(void);
static void vcd_start_tty_ctm(void);
static void vcd_stop_tty_ctm(void);
static void vcd_config_tty_ctm(void);
static void vcd_set_udata(void);
static void vcd_get_status(void);

static int vcd_set_call_mode(void *arg);
static int vcd_start_record(void *arg);
static int vcd_stop_record(void *arg);
static int vcd_start_playback(void *arg);
static int vcd_stop_playback(void *arg);
static int vcd_get_record_buffer(void *arg);
static int vcd_get_playback_buffer(void *arg);
static int vcd_watch_stop_fw(void *arg);
static int vcd_watch_start_clkgen(void *arg);

/* Proc functions */
static int vcd_read_exec_proc(char *page, char **start, off_t offset,
					int count, int *eof, void *data);
static int vcd_write_exec_proc(struct file *filp, const char *buffer,
					unsigned long len, void *data);
static int vcd_read_log_level(char *page, char **start, off_t offset,
					int count, int *eof, void *data);
static int vcd_write_log_level(struct file *filp, const char *buffer,
					unsigned long len, void *data);
static int vcd_read_exec_func(char *page, char **start, off_t offset,
					int count, int *eof, void *data);
static int vcd_write_exec_func(struct file *filp, const char *buffer,
					unsigned long len, void *data);

static int vcd_debug_execute(unsigned int command);
static void vcd_debug_complete_buffer(void);
static void vcd_debug_beginning_buffer(void);
static void vcd_debug_watch_fw(void);

static int vcd_create_proc_entry(void);
static void vcd_remove_proc_entry(void);

/* Driver functions */
static int vcd_probe(void);
static void vcd_remove(void);
static int vcd_suspend(struct device *dev);
static int vcd_resume(struct device *dev);
static int vcd_runtime_suspend(struct device *dev);
static int vcd_runtime_resume(struct device *dev);
static int vcd_fops_open(struct inode *inode, struct file *filp);
static int vcd_fops_release(struct inode *inode, struct file *filp);
static int vcd_fops_mmap(struct file *fp, struct vm_area_struct *vma);

static int __init vcd_module_init(void);
static void __exit vcd_module_exit(void);

#endif /* __VCD_IF_H__ */
