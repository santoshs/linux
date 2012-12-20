/*
 * rcvry_drv_main.c
 *	 Real Time Domain Recovery Driver API function file.
 *
 * Copyright (C) 2012 Renesas Electronics Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/kthread.h>
#include <linux/list.h>
#include <linux/completion.h>
#include <linux/spinlock.h>
#include <linux/semaphore.h>
#include <linux/syscalls.h>

#include <log_kernel.h>
#include <iccom_drv.h>
#include <rcvry_drv_common.h>
#include <iccom_drv_standby.h>
#include "rcvry_drv_private.h"

#ifdef ICCOM_ENABLE_STANDBYCONTROL
#include <linux/mfis.h>
#endif

static int rcvry_drv_open(
	struct inode	*inode,
	struct file		*fp
);

static long rcvry_drv_ioctl(
	struct file		*fp,
	unsigned int	cmd,
	unsigned long	arg
);

static int rcvry_drv_close(
	struct inode	*inode,
	struct file		*fp
);

/* file operation */
static const struct file_operations g_rcvry_drv_fops = {
	.owner			= THIS_MODULE,
	.open			= rcvry_drv_open,
	.unlocked_ioctl	= rcvry_drv_ioctl,
	.release		= rcvry_drv_close
};

/* miscdevice info */
static struct miscdevice g_rcvry_device;

/* Semaphore */
static struct semaphore g_rcvry_sem;

/* Recovery Driver info */
static rcvry_info rcvry_info_t[RECOVERY_DRIVER_INFO_MAX];

#define	RCVRY_DRIVER_NAME	"rtds_rcvry"

/*******************************************************************************
 * Function   : rcvry_drv_sub_index_search
 * Description: Search the index of emtpy table.
 * Parameters : NONE
 * Returns	  : The index of emtpy table.
 *******************************************************************************/
static long rcvry_drv_sub_emptyfp_table_search(void)
{
	unsigned long i;
	long rtn_index;

	MSG_HIGH("[RCVRYD_SUB]IN |[%s]\n",
			__func__);

	rtn_index = -1;
	i = 0;

	while (i < RECOVERY_DRIVER_INFO_MAX) {
		if (NULL == rcvry_info_t[i].rcvry_fp) {
			rtn_index = i;
			break;
		}
		i++;
	}

	MSG_HIGH("[RCVRYD_SUB]OUT|[%s] index = %ld\n",
			__func__, rtn_index);

	return rtn_index;
}

/*******************************************************************************
 * Function   : rcvry_drv_sub_currentfp_table_search
 * Description: Search the index of table from file discriptor.
 * Parameters : NONE
 * Returns	  : The index of table
 *******************************************************************************/
static long rcvry_drv_sub_currentfp_table_search(struct file *fp)
{
	unsigned long i;
	long rtn_index;

	MSG_HIGH("[RCVRYD_SUB]IN |[%s]\n",
			__func__);

	rtn_index = -1;
	i = 0;

	while (i < RECOVERY_DRIVER_INFO_MAX) {
		if (fp == rcvry_info_t[i].rcvry_fp) {
			MSG_HIGH("[RCVRYD]INF|[%s]Hit fp in table = 0x%08x\n",
					__func__, (u32)rcvry_info_t[i].rcvry_fp);
			rtn_index = i;
			break;
		}
		MSG_HIGH("[RCVRYD]INF|[%s]fp in table = 0x%08x\n",
				__func__, (u32)rcvry_info_t[i].rcvry_fp);
		i++;
	}

	return rtn_index;
}

/*******************************************************************************
 * Function   : rcvry_drv_sub_new
 * Description: Create a handle.
 * Parameters : NONE
 * Returns	  : The address of handle
 *******************************************************************************/
static void *rcvry_drv_sub_new(void)
{
	iccom_drv_init_param iccom_init;
	rcvry_handle *p_rcvry_handle;

	MSG_HIGH("[RCVRYD_SUB]IN |[%s]\n",
			__func__);

	p_rcvry_handle = kmalloc(sizeof(*p_rcvry_handle), GFP_KERNEL);
	if (NULL == p_rcvry_handle) {
		MSG_ERROR("[RCVRYD_SUB] ERR |[%s] Recovery Handle Alloc Fail\n",
				__func__);
		MSG_HIGH("[RCVRYD_SUB]OUT 1|[%s]\n",
				__func__);
		return NULL;
	}

	iccom_init.user_data	= p_rcvry_handle;
	iccom_init.comp_notice	= NULL;

	p_rcvry_handle->handle = iccom_drv_init(&iccom_init);
	if (NULL == p_rcvry_handle->handle) {
		kfree(p_rcvry_handle);
		MSG_ERROR("[RCVRYD_SUB] ERR |[%s] iccom_drv_init Error\n",
				__func__);
		MSG_HIGH("[RCVRYD_SUB]OUT 2|[%s]\n",
				__func__);
		return NULL;
	}

	MSG_HIGH("[RCVRYD_SUB]OUT|[%s]\n",
			__func__);
	return p_rcvry_handle;
}

/*******************************************************************************
 * Function   : rcvry_drv_sub_delete
 * Description: Delete a handle.
 * Parameters : The handle of address
 * Returns	  : NONE
 *******************************************************************************/
static void rcvry_drv_sub_delete(rcvry_delete *p_handle)
{
	iccom_drv_cleanup_param iccom_cleanup;

	MSG_HIGH("[RCVRYD_SUB]IN |[%s]\n",
			__func__);

	memset(&iccom_cleanup, 0, sizeof(iccom_cleanup));

	iccom_cleanup.handle = ((rcvry_delete *)p_handle->handle)->handle;
	iccom_drv_cleanup(&iccom_cleanup);
	kfree(p_handle->handle);

	MSG_HIGH("[RCVRYD_SUB]OUT|[%s]\n",
			__func__);
	return;
}

/*******************************************************************************
 * Function   : rcvry_drv_sub_wait_killable
 * Description: Wait until the current process is killed or the function of complete is called
 * Parameters : file discriptor
 * Returns	  : Error Code
 *******************************************************************************/
static long rcvry_drv_sub_wait_killable(struct file *fp)
{
	long ret;
	int error;
	int send_data;
	long function_ret;
	iccom_drv_send_cmd_param iccom_send_cmd;
	void *p_rcvry_iccom_handle;
	rcvry_delete t_rcvry_delete;
	iccom_drv_enable_standby_param ena_standby;
	long local_index;
	unsigned long *local_killable_flag;
	long *local_standby_counter;
	struct completion *local_completion;
	pid_t *local_pid;
	struct file *local_fp = NULL;

	local_fp = fp;

	down(&g_rcvry_sem);
	local_index = rcvry_drv_sub_currentfp_table_search(local_fp);
	MSG_HIGH("[RCVRYD]INF|[%s]empty index = %lu\n",
			__func__, local_index);
	if (0 > local_index) {
		MSG_ERROR("[RCVRYD]ERR|[%s]No pid\n",
					__func__);
		up(&g_rcvry_sem);
		return SMAP_NG;
	}
	local_completion		= &rcvry_info_t[local_index].rcvry_completion;
	local_standby_counter	= &rcvry_info_t[local_index].standby_counter;
	local_pid				= &rcvry_info_t[local_index].pid;
	local_killable_flag		= &rcvry_info_t[local_index].killable_flag;
	local_fp				=  rcvry_info_t[local_index].rcvry_fp;
	up(&g_rcvry_sem);

	error = wait_for_completion_killable(local_completion);
	MSG_MED("[RCVRYD]INF|[%s] wait_for_completion_killable error = %d\n",
			__func__, error);

	if (0 < *local_standby_counter) {

		MSG_HIGH("[RCVRYD]INF|[%s] STANDBY NG CANCEL PASS\n",
				__func__);
		MSG_HIGH("[RCVRYD]INF|[%s] Before g_standby_counter = %ld\n",
				__func__, *local_standby_counter);

		down(&g_rcvry_sem);

		p_rcvry_iccom_handle	= rcvry_drv_sub_new();

		ena_standby.handle		= p_rcvry_iccom_handle;
		t_rcvry_delete.handle	= p_rcvry_iccom_handle;

		ret = iccom_drv_enable_standby(&ena_standby);
		(*local_standby_counter)--;

		rcvry_drv_sub_delete(&t_rcvry_delete);

		up(&g_rcvry_sem);

		MSG_HIGH("[RCVRYD]INF|[%s] After  g_standby_counter = %ld\n",
				__func__, *local_standby_counter);

	} else {
		MSG_HIGH("[RCVRYD]INF|[%s] Not Done\n",
				__func__);
	}

#ifdef ICCOM_ENABLE_STANDBYCONTROL
	ret = mfis_drv_resume();
#endif

	if (error) {
		down(&g_rcvry_sem);
		p_rcvry_iccom_handle = rcvry_drv_sub_new();
		if (NULL == p_rcvry_iccom_handle) {
			MSG_ERROR("[RCVRYD]ERR |[%s] Recovery Handle Create Fail\n",
					__func__);
			ret = SMAP_NG;
			MSG_HIGH("[RCVRYD]OUT 1|[%s]\n",
					__func__);
			rcvry_info_t[local_index].pid = 0;
			up(&g_rcvry_sem);
			return ret;
		}
		t_rcvry_delete.handle = p_rcvry_iccom_handle;
		send_data = current->pid;

		MSG_MED("[RCVRYD]INF|[%s] Send date(pid) = %d\n",
				__func__, send_data);

		function_ret = 0;
		iccom_send_cmd.handle		= ((rcvry_handle *)p_rcvry_iccom_handle)->handle;
		iccom_send_cmd.task_id		= TASK_RCVRY01;
		iccom_send_cmd.function_id	= EVENT_TASK_RESOURCE_RELEASE;
		iccom_send_cmd.send_mode	= ICCOM_DRV_SYNC;
		iccom_send_cmd.send_size	= sizeof(int);
		iccom_send_cmd.send_data	= (unsigned char *)&send_data;
		iccom_send_cmd.recv_size	= sizeof(long);
		iccom_send_cmd.recv_data	= (unsigned char *)&function_ret;
		MSG_MED("[RCVRYD]INF|[%s] iccom_drv_send_command in 1\n",
				__func__);
		ret = iccom_drv_send_command(&iccom_send_cmd);
		if (SMAP_OK != ret) {
			MSG_ERROR("[RCVRYD]ERR |[%s] Send Command Fail 1 ret_code = 0x%08lx\n",
					__func__, ret);
			ret = SMAP_NG;
			rcvry_drv_sub_delete(&t_rcvry_delete);
			MSG_HIGH("[RCVRYD]OUT 1|[%s]\n",
					__func__);
			rcvry_info_t[local_index].pid = 0;
			up(&g_rcvry_sem);
			return ret;
		}
		MSG_MED("[RCVRYD]INF|[%s] iccom_drv_send_command out 1 ret = %ld function_ret = 0x%08lx\n",
				__func__, ret, function_ret);

		function_ret = 0;
		iccom_send_cmd.handle		= ((rcvry_handle *)p_rcvry_iccom_handle)->handle;
		iccom_send_cmd.task_id		= TASK_RCVRY01;
		iccom_send_cmd.function_id	= EVENT_TASK_RCVRY01_ID;
		iccom_send_cmd.send_mode	= ICCOM_DRV_SYNC;
		iccom_send_cmd.send_size	= sizeof(int);
		iccom_send_cmd.send_data	= (unsigned char *)&send_data;
		iccom_send_cmd.recv_size	= sizeof(long);
		iccom_send_cmd.recv_data	= (unsigned char *)&function_ret;
		MSG_MED("[RCVRYD]INF|[%s] iccom_drv_send_command in 2\n",
				__func__);
		ret = iccom_drv_send_command(&iccom_send_cmd);
		if (SMAP_OK != ret) {
			MSG_ERROR("[RCVRYD]ERR |[%s] Send Command Fail 2 ret_code = 0x%08lx\n",
					__func__, ret);
			ret = SMAP_NG;
			rcvry_drv_sub_delete(&t_rcvry_delete);
			MSG_HIGH("[RCVRYD]OUT 2|[%s]\n",
					__func__);
			rcvry_info_t[local_index].pid = 0;
			up(&g_rcvry_sem);
			return ret;
		}
		MSG_MED("[RCVRYD]INF|[%s] iccom_drv_send_command out 2 ret = %ld function_ret = 0x%08lx\n",
				__func__, ret, function_ret);

		function_ret = 0;
		iccom_send_cmd.handle		= ((rcvry_handle *)p_rcvry_iccom_handle)->handle;
		iccom_send_cmd.task_id		= TASK_RCVRY02;
		iccom_send_cmd.function_id	= EVENT_TASK_RCVRY02_ID;
		iccom_send_cmd.send_mode	= ICCOM_DRV_SYNC;
		iccom_send_cmd.send_size	= sizeof(int);
		iccom_send_cmd.send_data	= (unsigned char *)&send_data;
		iccom_send_cmd.recv_size	= sizeof(long);
		iccom_send_cmd.recv_data	= (unsigned char *)&function_ret;
		MSG_MED("[RCVRYD]INF|[%s] iccom_drv_send_command in 3\n",
				__func__);
		ret = iccom_drv_send_command(&iccom_send_cmd);
		if (SMAP_OK != ret) {
			MSG_ERROR("[RCVRYD]ERR |[%s] Send Command Fail 3 ret_code = 0x%08lx\n",
					__func__, ret);
			ret = SMAP_NG;
			rcvry_drv_sub_delete(&t_rcvry_delete);
			MSG_HIGH("[RCVRYD]OUT 3|[%s]\n",
					__func__);
			rcvry_info_t[local_index].pid = 0;
			up(&g_rcvry_sem);
			return ret;
		}
		MSG_MED("[RCVRYD]INF|[%s] iccom_drv_send_command out 3 ret = %ld function_ret = 0x%08lx\n",
				__func__, ret, function_ret);

		function_ret = 0;
		iccom_send_cmd.handle		= ((rcvry_handle *)p_rcvry_iccom_handle)->handle;
		iccom_send_cmd.task_id		= TASK_RCVRY03;
		iccom_send_cmd.function_id	= EVENT_TASK_RCVRY03_ID;
		iccom_send_cmd.send_mode	= ICCOM_DRV_SYNC;
		iccom_send_cmd.send_size	= sizeof(int);
		iccom_send_cmd.send_data	= (unsigned char *)&send_data;
		iccom_send_cmd.recv_size	= sizeof(long);
		iccom_send_cmd.recv_data	= (unsigned char *)&function_ret;
		MSG_MED("[RCVRYD]INF|[%s] iccom_drv_send_command in 4\n",
				__func__);
		ret = iccom_drv_send_command(&iccom_send_cmd);
		if (SMAP_OK != ret) {
			MSG_ERROR("[RCVRYD]ERR |[%s] Send Command Fail 4 ret_code = 0x%08lx\n",
					__func__, ret);
			ret = SMAP_NG;
			rcvry_drv_sub_delete(&t_rcvry_delete);
			MSG_HIGH("[RCVRYD]OUT 3|[%s]\n",
					__func__);
			*local_pid = 0;
			up(&g_rcvry_sem);
			return ret;
		}
		MSG_MED("[RCVRYD]INF|[%s] iccom_drv_send_command out 4 ret = %ld function_ret = 0x%08lx\n",
				__func__, ret, function_ret);

		rcvry_drv_sub_delete(&t_rcvry_delete);
		*local_killable_flag = 1;
		up(&g_rcvry_sem);

		MSG_HIGH("[RCVRYD]OUT|[%s]\n",
				__func__);
	} else {
		ret = SMAP_OK;
		MSG_HIGH("[RCVRYD]OUT|[%s] complete return \n",
				__func__);
	}

	return ret;
}

/*******************************************************************************
 * Function   : rcvry_drv_sub_wait_killable_cancel
 * Description: Return the function of wait_for_completion_killable
 * Parameters : file discriptor
 * Returns	  : Error Code
 *******************************************************************************/
static long rcvry_drv_sub_wait_killable_cancel(struct file *fp)
{
	long local_index;
	struct file *local_fp = NULL;
	struct completion *local_completion;

	local_fp = fp;

	down(&g_rcvry_sem);
	local_index = rcvry_drv_sub_currentfp_table_search(local_fp);
	MSG_HIGH("[RCVRYD]INF|[%s]empty index = %lu\n",
			__func__, local_index);
	if (0 > local_index) {
		MSG_ERROR("[RCVRYD]ERR|[%s]No pid\n",
					__func__);
		up(&g_rcvry_sem);
		return SMAP_NG;
	}
	local_completion = &rcvry_info_t[local_index].rcvry_completion;
	up(&g_rcvry_sem);
	complete(local_completion);

	MSG_HIGH("[RCVRYD]OUT|[%s]ret = SMAP_OK\n",
			__func__);

	return SMAP_OK;

}

/*******************************************************************************
 * Function   : rcvry_drv_open
 * Description: This function open Recovery driver.
 * Parameters : inode   -   inode information
 *				fp		-   file descriptor
 * Returns	  : SMAP_OK		-   Success
 *******************************************************************************/
static int rcvry_drv_open(
	struct inode	*inode,
	struct file		*fp
)
{
	unsigned long local_index;

	down(&g_rcvry_sem);

	MSG_HIGH("[RCVRYD]IN |[%s]\n",
			__func__);

	MSG_MED("[RCVRYD]   |inode_p[0x%08X]\n",
			(u32)inode);
	MSG_MED("[RCVRYD]   |fp[0x%08X]\n",
			(u32)fp);
	MSG_LOW("[RCVRYD]   |major[%d]\n",
			imajor(inode));
	MSG_LOW("[RCVRYD]   |minor[%d]\n",
			iminor(inode));

	local_index = rcvry_drv_sub_emptyfp_table_search();
	MSG_HIGH("[RCVRYD]INF|[%s]empty index = %lu\n",
			__func__, local_index);
	if (0 > local_index) {
		MSG_ERROR("[RCVRYD]ERR|[%s]No Empty\n",
					__func__);
		up(&g_rcvry_sem);
		return SMAP_NG;
	}

	rcvry_info_t[local_index].rcvry_fp = fp;
	init_completion(&rcvry_info_t[local_index].rcvry_completion);

	MSG_HIGH("[RCVRYD]OUT|[%s]ret = SMAP_OK\n",
			__func__);

	up(&g_rcvry_sem);

	return SMAP_OK;
}

/*******************************************************************************
 * Function   : rcvry_drv_ioctl
 * Description: This function ioctl Recovery driver.
 * Parameters : fp		-   file descriptor
 *				cmd		-   send command
 *				arg		-   argument
 * Returns	  : SMAP_OK	-   Success
 *
 *******************************************************************************/
static long rcvry_drv_ioctl(
	struct file		*fp,
	unsigned int	cmd,
	unsigned long	arg
)
{
	long ret;
	int cmd_param = 0;
	void *p_rcvry_iccom_handle;
	iccom_drv_disable_standby_param dis_standby;
	iccom_drv_enable_standby_param ena_standby;
	rcvry_delete t_rcvry_delete;
	struct task_struct *task = current;
	long *local_standby_counter;
	long local_index;
	struct file *local_fp = NULL;

	ret = SMAP_NG;
	task->flags |= PF_FREEZER_SKIP;

	MSG_HIGH("[RCVRYD]IN  |[%s]\n",
			__func__);

	if (0 != arg) {
		if (!access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd))) {
			MSG_ERROR("[RCVRYD]ERR| parameter error.\n");
			ret = SMAP_PARA_NG;
			return ret;
		}
	}

	local_fp = fp;

	switch (cmd) {
	case RCVRY_CMD_KILL_WAIT:
		ret = rcvry_drv_sub_wait_killable(local_fp);
		break;
	case RCVRY_CMD_KILL_WAIT_CANCEL:
		ret = rcvry_drv_sub_wait_killable_cancel(local_fp);
		break;
#ifdef ICCOM_ENABLE_STANDBYCONTROL
	case RCVRY_CMD_STANDBY_NG:
		MSG_HIGH("[RCVRYD]INF|[%s] STANDBY NG PASS\n",
				__func__);
		down(&g_rcvry_sem);
		local_index = rcvry_drv_sub_currentfp_table_search(local_fp);
		MSG_HIGH("[RCVRYD]INF|[%s]empty index = %lu\n",
				__func__, local_index);
		if (0 > local_index) {
			MSG_ERROR("[RCVRYD]ERR|[%s]No pid\n",
						__func__);
			up(&g_rcvry_sem);
			return SMAP_NG;
		}
		local_standby_counter = &rcvry_info_t[local_index].standby_counter;
		up(&g_rcvry_sem);
		MSG_HIGH("[RCVRYD]INF|[%s] Before g_standby_counter = %ld\n",
				__func__, *local_standby_counter);

		down(&g_rcvry_sem);
		p_rcvry_iccom_handle	= rcvry_drv_sub_new();

		dis_standby.handle		= p_rcvry_iccom_handle;
		t_rcvry_delete.handle	= p_rcvry_iccom_handle;

		ret = iccom_drv_disable_standby(&dis_standby);
		(*local_standby_counter)++;

		rcvry_drv_sub_delete(&t_rcvry_delete);
		up(&g_rcvry_sem);

		MSG_HIGH("[RCVRYD]INF|[%s] After  g_standby_counter = %ld\n",
				__func__, *local_standby_counter);

		break;

	case RCVRY_CMD_STANDBY_NG_CANCEL:
		MSG_HIGH("[RCVRYD]INF|[%s] STANDBY NG CANCEL PASS\n",
				__func__);
		down(&g_rcvry_sem);
		local_index = rcvry_drv_sub_currentfp_table_search(local_fp);
		MSG_HIGH("[RCVRYD]INF|[%s]empty index = %lu\n",
				__func__, local_index);
		if (0 > local_index) {
			MSG_ERROR("[RCVRYD]ERR|[%s]No pid\n",
						__func__);
			up(&g_rcvry_sem);
			return SMAP_NG;
		}
		local_standby_counter = &rcvry_info_t[local_index].standby_counter;
		up(&g_rcvry_sem);
		MSG_HIGH("[RCVRYD]INF|[%s] Before g_standby_counter = %ld\n",
				__func__, *local_standby_counter);

		if (0 < *local_standby_counter) {
			down(&g_rcvry_sem);
			p_rcvry_iccom_handle	= rcvry_drv_sub_new();

			ena_standby.handle		= p_rcvry_iccom_handle;
			t_rcvry_delete.handle	= p_rcvry_iccom_handle;

			ret = iccom_drv_enable_standby(&ena_standby);
			(*local_standby_counter)--;

			rcvry_drv_sub_delete(&t_rcvry_delete);
			up(&g_rcvry_sem);

		} else {
			MSG_HIGH("[RCVRYD]INF|[%s] Not Done\n",
					__func__);
		}
		MSG_HIGH("[RCVRYD]INF|[%s] After  g_standby_counter = %ld\n",
				__func__, *local_standby_counter);
		break;
#endif	/* ICCOM_ENABLE_STANDBYCONTROL */

	case RCVRY_CMD_GET_PID:
		MSG_HIGH("[RCVRYD]INF|[%s] GET PID PASS\n",
				__func__);
		cmd_param = current->pid;
		if (copy_to_user((void __user *)arg, &cmd_param, sizeof(cmd_param))) {
			MSG_ERROR("[RCVRYD]ERR|[%s] output data copy.\n",
						__func__);
			ret = SMAP_NG;
		} else {
			MSG_HIGH("[RCVRYD]INF|[%s] pid = %d\n",
						__func__, cmd_param);
			ret = SMAP_OK;
		}
		break;

	default:
		ret = SMAP_NG;
		break;
	}

	return ret;
}

/*******************************************************************************
 * Function   : rcvry_drv_close
 * Description: This function close Recovery driver.
 * Parameters : inode   -   inode information
 *				fp		-   file descriptor
 * Returns	  : SMAP_OK		-   Success
 *
 *******************************************************************************/
static int rcvry_drv_close(
	struct inode	*inode,
	struct file		*fp
)
{
	unsigned long local_index;

	MSG_HIGH("[RCVRYD]IN |[%s]\n",
			__func__);
	MSG_MED("[RCVRYD]   |fp[0x%08X]\n",
			(u32)fp);

	down(&g_rcvry_sem);

	local_index = rcvry_drv_sub_currentfp_table_search(fp);
	MSG_HIGH("[RCVRYD]INF|[%s]empty index = %lu\n",
			__func__, local_index);
	if (0 > local_index) {
		MSG_ERROR("[RCVRYD]ERR|[%s]No tgid\n",
					__func__);
		up(&g_rcvry_sem);
		return SMAP_NG;
	}

	MSG_HIGH("[RCVRYD]INF|[%s]Already wait_for_completion_killable return\n",
			__func__);
	rcvry_info_t[local_index].pid = 0;
	rcvry_info_t[local_index].standby_counter = 0;
	rcvry_info_t[local_index].killable_flag = 0;
	rcvry_info_t[local_index].rcvry_fp = NULL;

	up(&g_rcvry_sem);

	MSG_HIGH("[RCVRYD]OUT|[%s]ret = SMAP_OK\n",
			__func__);

	return SMAP_OK;
}

/*******************************************************************************
 * Function   : rcvry_init_module
 * Description: This function initialize Recovery driver.
 * Parameters : none
 * Returns	  : SMAP_OK -   Success
 *				SMAP_NG -   Fatal error
 *******************************************************************************/
int rcvry_init_module(void)
{

	int ret;

	MSG_HIGH("[RCVRYD]IN |[%s]\n",
			__func__);
	MSG_HIGH("[RCVRYD]IN |Built Test[%s]\n",
			__func__);

	/* Register device driver */
	g_rcvry_device.name		= RCVRY_DRIVER_NAME;
	g_rcvry_device.fops		= &g_rcvry_drv_fops;
	g_rcvry_device.minor	= MISC_DYNAMIC_MINOR;
	ret = misc_register(&g_rcvry_device);
	if (0 != ret) {
		MSG_ERROR("[RCVRYD]ERR| misc_register failed ret[%d]\n",
				ret);
		MSG_HIGH("[RCVRYD]OUT|[%s]ret = SMAP_NG\n",
				__func__);
		return SMAP_NG;
	}

	init_MUTEX(&g_rcvry_sem);

	memset(rcvry_info_t , 0 , sizeof(rcvry_info_t));

	MSG_HIGH("[RCVRYD]OUT|[%s]ret = SMAP_OK\n",
			__func__);

	return SMAP_OK;
}

module_init(rcvry_init_module);
