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

/* Completion */
static struct completion g_rcvry_completion;

/* Semaphore */
static struct semaphore g_rcvry_sem;

/* Recovery Init Counter */
static long g_rcvry_init_counter;

#ifdef ICCOM_ENABLE_STANDBYCONTROL
static long g_standby_counter;
#endif	/* ICCOM_ENABLE_STANDBYCONTROL */

#define	RCVRY_DRIVER_NAME	"rtds_rcvry"

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
 * Parameters : NONE
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
 * Function   : rcvry_drv_sub_wait
 * Description:
 * Parameters : NONE
 * Returns	  : The address of handle
 *******************************************************************************/
static long rcvry_drv_sub_wait_killable(void)
{
	long ret;
	int error;
	int send_data;
	long function_ret;
	iccom_drv_send_cmd_param iccom_send_cmd;
	void *p_rcvry_iccom_handle;
	rcvry_delete t_rcvry_delete;
	iccom_drv_enable_standby_param ena_standby;

	send_data = 0;

	error = wait_for_completion_killable(&g_rcvry_completion);
	MSG_MED("[RCVRYD]INF|[%s] wait_for_completion_killable error = %d\n",
			__func__, error);
	down(&g_rcvry_sem);
	g_rcvry_init_counter--;
	up(&g_rcvry_sem);

	if (0 < g_standby_counter) {

		MSG_HIGH("[RCVRYD]INF|[%s] STANDBY NG CANCEL PASS\n",
				__func__);
		MSG_HIGH("[RCVRYD]INF|[%s] Before g_standby_counter = %ld\n",
				__func__, g_standby_counter);

		down(&g_rcvry_sem);

		p_rcvry_iccom_handle	= rcvry_drv_sub_new();

		ena_standby.handle		= p_rcvry_iccom_handle;
		t_rcvry_delete.handle	= p_rcvry_iccom_handle;

		ret = iccom_drv_enable_standby(&ena_standby);
		g_standby_counter--;

		rcvry_drv_sub_delete(&t_rcvry_delete);

		up(&g_rcvry_sem);

		MSG_HIGH("[RCVRYD]INF|[%s] After  g_standby_counter = %ld\n",
				__func__, g_standby_counter);

	} else {
		MSG_HIGH("[RCVRYD]INF|[%s] Not Done\n",
				__func__);
	}

#ifdef ICCOM_ENABLE_STANDBYCONTROL
	ret = mfis_drv_resume();
#endif

	p_rcvry_iccom_handle	= rcvry_drv_sub_new();
	if (NULL == p_rcvry_iccom_handle) {
		MSG_ERROR("[RCVRYD]ERR |[%s] Recovery Handle Create Fail\n",
				__func__);
		ret = SMAP_NG;
		MSG_HIGH("[RCVRYD]OUT 1|[%s]\n",
				__func__);
		return ret;
	}
	t_rcvry_delete.handle = p_rcvry_iccom_handle;

	iccom_send_cmd.handle		= ((rcvry_handle *)p_rcvry_iccom_handle)->handle;
	iccom_send_cmd.task_id		= TASK_RCVRY01;
	iccom_send_cmd.function_id	= EVENT_TASK_RCVRY01_ID;
	iccom_send_cmd.send_mode	= ICCOM_DRV_SYNC;
	iccom_send_cmd.send_size	= 0;
	iccom_send_cmd.send_data	= 0;
	iccom_send_cmd.recv_size	= sizeof(long);
	iccom_send_cmd.recv_data	= (unsigned char *)&function_ret;
	MSG_MED("[RCVRYD]INF|[%s] iccom_drv_send_command in 1\n",
			__func__);
	ret = iccom_drv_send_command(&iccom_send_cmd);
	if (SMAP_OK != ret) {
		MSG_ERROR("[RCVRYD]ERR |[%s] Send Command Fail 1 ret_code = %ld\n",
				__func__, ret);
		ret = SMAP_NG;
		rcvry_drv_sub_delete(&t_rcvry_delete);
		MSG_HIGH("[RCVRYD]OUT 1|[%s]\n",
				__func__);
		return ret;
	}
	MSG_MED("[RCVRYD]INF|[%s] iccom_drv_send_command out 1 ret = %ld function_ret = %ld\n",
			__func__, ret, function_ret);

	iccom_send_cmd.handle		= ((rcvry_handle *)p_rcvry_iccom_handle)->handle;
	iccom_send_cmd.task_id		= TASK_RCVRY02;
	iccom_send_cmd.function_id	= EVENT_TASK_RCVRY02_ID;
	iccom_send_cmd.send_mode	= ICCOM_DRV_SYNC;
	iccom_send_cmd.send_size	= sizeof(int);
	iccom_send_cmd.send_data	= (unsigned char *)&send_data;
	iccom_send_cmd.recv_size	= sizeof(long);
	iccom_send_cmd.recv_data	= (unsigned char *)&function_ret;
	MSG_MED("[RCVRYD]INF|[%s] iccom_drv_send_command in 2\n",
			__func__);
	ret = iccom_drv_send_command(&iccom_send_cmd);
	if (SMAP_OK != ret) {
		MSG_ERROR("[RCVRYD]ERR |[%s] Send Command Fail 2 ret_code = %ld\n",
				__func__, ret);
		ret = SMAP_NG;
		rcvry_drv_sub_delete(&t_rcvry_delete);
		MSG_HIGH("[RCVRYD]OUT 2|[%s]\n",
				__func__);
		return ret;
	}
	MSG_MED("[RCVRYD]INF|[%s] iccom_drv_send_command out 2 ret = %ld function_ret = %ld\n",
			__func__, ret, function_ret);

	iccom_send_cmd.handle		= ((rcvry_handle *)p_rcvry_iccom_handle)->handle;
	iccom_send_cmd.task_id		= TASK_RCVRY03;
	iccom_send_cmd.function_id	= EVENT_TASK_RCVRY03_ID;
	iccom_send_cmd.send_mode	= ICCOM_DRV_SYNC;
	iccom_send_cmd.send_size	= sizeof(int);
	iccom_send_cmd.send_data	= (unsigned char *)&send_data;
	iccom_send_cmd.recv_size	= sizeof(long);
	iccom_send_cmd.recv_data	= (unsigned char *)&function_ret;
	MSG_MED("[RCVRYD]INF|[%s] iccom_drv_send_command in 3\n",
			__func__);
	ret = iccom_drv_send_command(&iccom_send_cmd);
	if (SMAP_OK != ret) {
		MSG_ERROR("[RCVRYD]ERR |[%s] Send Command Fail 3 ret_code = %ld\n",
				__func__, ret);
		ret = SMAP_NG;
		rcvry_drv_sub_delete(&t_rcvry_delete);
		MSG_HIGH("[RCVRYD]OUT 3|[%s]\n",
				__func__);
		return ret;
	}
	MSG_MED("[RCVRYD]INF|[%s] iccom_drv_send_command out 3 ret = %ld function_ret = %ld\n",
			__func__, ret, function_ret);

	rcvry_drv_sub_delete(&t_rcvry_delete);

	MSG_HIGH("[RCVRYD]OUT|[%s]\n",
			__func__);

	return ret;
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

	MSG_HIGH("[RCVRYD]  |[%s]Before g_rcvry_init_counter = %ld\n",
			__func__, g_rcvry_init_counter);
	g_rcvry_init_counter++;
	MSG_HIGH("[RCVRYD]  |[%s]After  g_rcvry_init_counter = %ld\n",
			__func__, g_rcvry_init_counter);

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
	void *p_rcvry_iccom_handle;
	iccom_drv_disable_standby_param dis_standby;
	iccom_drv_enable_standby_param ena_standby;
	rcvry_delete t_rcvry_delete;
	struct task_struct *task = current;

	ret = SMAP_NG;
	task->flags |= PF_FREEZER_SKIP;

	MSG_HIGH("[RCVRYD]IN  |[%s]\n",
			__func__);

	switch (cmd) {
	case RCVRY_CMD_KILL_WAIT:
		ret = rcvry_drv_sub_wait_killable();
		break;
#ifdef ICCOM_ENABLE_STANDBYCONTROL
	case RCVRY_CMD_STANDBY_NG:
		MSG_HIGH("[RCVRYD]INF|[%s] STANDBY NG PASS\n",
				__func__);
		MSG_HIGH("[RCVRYD]INF|[%s] Before g_standby_counter = %ld\n",
				__func__, g_standby_counter);

		down(&g_rcvry_sem);
		p_rcvry_iccom_handle	= rcvry_drv_sub_new();

		dis_standby.handle		= p_rcvry_iccom_handle;
		t_rcvry_delete.handle	= p_rcvry_iccom_handle;

		ret = iccom_drv_disable_standby(&dis_standby);
		g_standby_counter++;

		rcvry_drv_sub_delete(&t_rcvry_delete);
		up(&g_rcvry_sem);

		MSG_HIGH("[RCVRYD]INF|[%s] After  g_standby_counter = %ld\n",
				__func__, g_standby_counter);

		break;

	case RCVRY_CMD_STANDBY_NG_CANCEL:
		MSG_HIGH("[RCVRYD]INF|[%s] STANDBY NG CANCEL PASS\n",
				__func__);
		MSG_HIGH("[RCVRYD]INF|[%s] Before g_standby_counter = %ld\n",
				__func__, g_standby_counter);

		if (0 < g_standby_counter) {
			down(&g_rcvry_sem);
			p_rcvry_iccom_handle	= rcvry_drv_sub_new();

			ena_standby.handle		= p_rcvry_iccom_handle;
			t_rcvry_delete.handle	= p_rcvry_iccom_handle;

			ret = iccom_drv_enable_standby(&ena_standby);
			g_standby_counter--;

			rcvry_drv_sub_delete(&t_rcvry_delete);
			up(&g_rcvry_sem);

		} else {
			MSG_HIGH("[RCVRYD]INF|[%s] Not Done\n",
					__func__);
		}
		MSG_HIGH("[RCVRYD]INF|[%s] After  g_standby_counter = %ld\n",
				__func__, g_standby_counter);
		break;
#endif	/* ICCOM_ENABLE_STANDBYCONTROL */

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
	down(&g_rcvry_sem);
	MSG_HIGH("[RCVRYD]IN |[%s]\n",
			__func__);
	MSG_MED("[RCVRYD]    |inode_p[0x%08X]\n",
			(u32)inode);
	MSG_MED("[RCVRYD]    |fp[0x%08X]\n",
			(u32)fp);

	if (1 == g_rcvry_init_counter) {
		MSG_HIGH("[RCVRYD]INF|[%s] Completion Pass\n",
				__func__);
		up(&g_rcvry_sem);
		complete(&g_rcvry_completion);
	} else {
		MSG_HIGH("[RCVRYD]INF|[%s] No Completion Pass\n",
				__func__);
		up(&g_rcvry_sem);
	}

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

	init_completion(&g_rcvry_completion);

	g_rcvry_init_counter	= 0;
	g_standby_counter		= 0;

	MSG_HIGH("[RCVRYD]OUT|[%s]ret = SMAP_OK\n",
			__func__);

	return SMAP_OK;
}

module_init(rcvry_init_module);
