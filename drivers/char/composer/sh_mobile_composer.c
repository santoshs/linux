/*
 * Function        : Composer driver for SH Mobile
 *
 * Copyright (C) 2011-2012 Renesas Electronics Corporation
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
 * Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
 */

#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/semaphore.h>
#include <linux/rwlock.h>
#include <linux/list.h>
#include <linux/kthread.h>
#include <generated/autoconf.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif /* CONFIG_HAS_EARLYSUSPEND */

#include <rtapi/screen_graphics.h>
#include <rtapi/screen_common.h>
#include <rtapi/system_memory.h>

#include <linux/sh_mobile_composer.h>
#include <video/sh_mobile_lcdc.h>

static int debug;    /* default debug level */

#define   INTERNAL_DEBUG   0	/* enable debug interface        */
				/*   0: none                     */
				/*   1: enable trace.            */

#define   _TIME_DBG  0		/* generate report processing time. */
#define   _LOG_DBG   1		/* generate debug log.              */
#define   _ERR_DBG   2		/* generate error log.              */

/*#define   DEBUG_NO_USE_TIMER      */  /* not use add_timer */
/*#define   DEBUG_DUMP_IMAGE_ADDRESS*/  /* dump image data */

/******************************************************/
/* define protptype                                   */
/******************************************************/
static ssize_t core_read(struct file *filp, char __user *buf, \
		size_t sz, loff_t *off);
static unsigned int core_poll(struct file *filp, \
		struct poll_table_struct *poll);
static long core_ioctl(struct file *filep, \
		unsigned int cmd, unsigned long arg);
static int core_open(struct inode *inode, struct file *filep);
static int core_release(struct inode *inode, struct file *filep);


static void work_runblend(struct localwork *work);
static int  release_buffer(struct composer_fh *fh, int level);
static int  lock_buffer(struct composer_fh *fh, int level);
static int  unlock_buffer(struct composer_fh *fh, int level);
static void wakeup_allfile(struct composer_buffer *buffer);

static int  setbusy(struct composer_fh *fh);
static int  setidle(struct composer_fh *fh);

static int    get_index_from_level(int level, int *idx);
static struct composer_info *get_composer_info(
	struct composer_fh *fh, int level);


/* ioctl */
static int  iocs_close(struct composer_fh *fh, \
	struct cmp_lay_close *arg);

/* parameter check */
static int  chk_ioc_supportpixfmt(struct composer_fh *fh, \
	struct cmp_lay_supportpixfmt *arg);
static int  chk_ioc_open(struct composer_fh *fh, \
	struct cmp_lay_open *arg);
static int  chk_ioc_close(struct composer_fh *fh, \
	struct cmp_lay_close *arg);
static int  chk_ioc_disp(struct composer_fh *fh, \
	struct cmp_lay_disp *arg);
static int  chk_ioc_alpha(struct composer_fh *fh, \
	struct cmp_lay_alpha *arg, int set_flag);
static int  chk_ioc_keycolor(struct composer_fh *fh, \
	struct cmp_lay_keycolor *arg, int set_flag);
static int  chk_ioc_laysize(struct composer_fh *fh, \
	struct cmp_lay_data_size *arg, int set_flag);
static int  chk_ioc_pixfmt(struct composer_fh *fh, \
	struct cmp_lay_pixfmt *arg, int set_flag);
static int  chk_ioc_cropsize(struct composer_fh *fh, \
	struct cmp_lay_cropsize *arg, int set_flag, \
	struct cmp_lay_data_size *laysize);
static int  chk_ioc_croppos(struct composer_fh *fh, \
	struct cmp_lay_croppos *arg, int set_flag, \
	struct cmp_lay_data_size *laysize);
static int  chk_ioc_pos(struct composer_fh *fh, \
	struct cmp_lay_pos *arg, int set_flag);
static int  chk_ioc_size(struct composer_fh *fh, \
	struct cmp_lay_compose_size *arg, int set_flag);
static int  chk_ioc_viewlay(struct composer_fh *fh, \
	struct cmp_viewlay *arg, int idx);
static int  chk_ioc_backcolor(struct composer_fh *fh, \
	struct cmp_lay_backcolor *arg, int set_flag);
static int  chk_ioc_layaddr(struct composer_fh *fh, \
	struct cmp_layaddr *arg, int set_flag);
static int  chk_ioc_start(struct composer_fh *fh);

static void notify_graphics_image_conv(int result, unsigned long user_data);
static void notify_graphics_image_blend(int result, unsigned long user_data);
#ifdef RT_GRAPHICS_MODE_IMAGE_OUTPUT
static void notify_graphics_image_output_dummy(
	int result, unsigned long user_data);
static void notify_graphics_image_blend_dummy(
	int result, unsigned long user_data);
static void notify_graphics_image_output(int result, unsigned long user_data);
#endif
static void callback_iocs_start(int result, void *user_data);
#ifdef CONFIG_MISC_R_MOBILE_COMPOSER_REQUEST_QUEUE
static void composer_blendoverlay_errorcallback(
	struct composer_rh *rh);
static void timeout_queue_process(unsigned long data);
static void process_composer_queue_callback(struct composer_rh *rh);
static int  queue_fb_address_mapping(void);
static void callback_composer_queue(int result, void *user_data);
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
static void pm_early_suspend(struct early_suspend *h);
static void pm_late_resume(struct early_suspend *h);
#endif

#if INTERNAL_DEBUG >= 1
static void tracelog_record(int logclass, int line, int ID, int val);
#endif

/******************************************************/
/* define local define                                */
/******************************************************/
#if INTERNAL_DEBUG >= 2
#error not supported.
#endif
#define TRACE_ENTER(ID)
#define TRACE_LEAVE(ID)
#define TRACE_LOG(ID)
#define TRACE_LOG1(ID, VAL1)

#if INTERNAL_DEBUG >= 1
#define INTERNAL_LOG_MSG_SIZE 8192
#define TRACELOG_SIZE         128
#endif

#define MAX_OPEN    32
#define MAX_BUFFER  64
#define MAX_WIDTH   1920
#define MAX_HEIGHT  1088
#define MAX_KERNELREQ 4

#define THREAD_PRIORITY  (MAX_RT_PRIO-1)

#define DEV_NAME      "composer"

/* define for id of struct composer_buffer */
#define BUF_ID_FLG_PIXFMT    0x001
#define BUF_ID_FLG_LAYSIZE   (BUF_ID_FLG_LAYSIZE1|BUF_ID_FLG_LAYSIZE2)
#define BUF_ID_FLG_LAYSIZE1  0x002
#define BUF_ID_FLG_LAYSIZE2  0x004
#define BUF_ID_FLG_CROPSIZE  (BUF_ID_FLG_CROPSIZE1|BUF_ID_FLG_CROPSIZE2)
#define BUF_ID_FLG_CROPSIZE1 0x008
#define BUF_ID_FLG_CROPSIZE2 0x010
#define BUF_ID_FLG_CROPPOS   (BUF_ID_FLG_CROPPOS1|BUF_ID_FLG_CROPPOS2)
#define BUF_ID_FLG_CROPPOS1  0x020
#define BUF_ID_FLG_CROPPOS2  0x040
#define BUF_ID_FLG_POS       (BUF_ID_FLG_POS1|BUF_ID_FLG_POS2)
#define BUF_ID_FLG_POS1      0x080
#define BUF_ID_FLG_POS2      0x100
#define BUF_ID_FLG_SIZE      (BUF_ID_FLG_SIZE1|BUF_ID_FLG_SIZE2)
#define BUF_ID_FLG_SIZE1     0x200
#define BUF_ID_FLG_SIZE2     0x400
#define BUF_ID_SET_BUFFER    0x800

#define BUF_ID_FLG_OUTPLANE   (0x01<<16)


/* define for id of struct composer_info */
#define INFO_ID_OUTPLANE    (0x00<<24)
#define INFO_ID_CH1LAYER    (0x01<<24)
#define INFO_ID_CH2LAYER    (0x02<<24)
#define INFO_ID_CH3LAYER    (0x03<<24)
#define INFO_ID_CH4LAYER    (0x04<<24)
#define INFO_ID_SET_APP0     0x02
#define INFO_ID_SET_APP1     0x04
#define INFO_ID_SET_APP2     0x08
#define INFO_ID_FLG_BLENDING    0x20
#define INFO_ID_FLG_NEEDAPPSET  0x40
#define INFO_ID_FLG_NOTDISP     0x80         /* control by CMP_IOCS_DISP */

/* define for fh_status of struct composer_fh */
#define FH_STATUS_OUTPLANE  (0x01<<16)
#define FH_STATUS_CH1LAYER  (0x02<<16)
#define FH_STATUS_CH2LAYER  (0x04<<16)
#define FH_STATUS_CH3LAYER  (0x08<<16)
#define FH_STATUS_CH4LAYER  (0x10<<16)
#define FH_STATUS_BLENDING  (0x01)
#define FH_STATUS_BLENDERR  (0x02)
#define FH_STATUS_POLLBLEND (0x01<<24)
#define FH_STATUS_POLLADDR  (0x02<<24)

/* define for index number of struct composer_fh */
#define IDX_OUTPLANE    0x00
#define IDX_CH1LAYER    0x01
#define IDX_CH2LAYER    0x02
#define IDX_CH3LAYER    0x03
#define IDX_CH4LAYER    0x04

/* define for trace log */
#if INTERNAL_DEBUG >= 1
#define SEQID_NOMORE_LOG     65535
#define ID_TRACE_ENTER       1
#define ID_TRACE_LEAVE       2
#define ID_TRACE_LOG         3
#define ID_TRACE_LOG1        4
#define FUNC_NONE            0x000
#define FUNC_OPEN            0x010
#define FUNC_CLOSE           0x011
#define FUNC_QUEUE           0x012
#define FUNC_BLEND           0x013
#define FUNC_CALLBACK        0x014
#define FUNC_HDMISET         0x015
#define FUNC_WQ_CREATE       0x020
#define FUNC_WQ_DELETE       0x021
#define FUNC_WQ_BLEND        0x022
#define FUNC_WQ_OVERLAY      0x023
#define FUNC_WQ_EXPIRE       0x024
#define FUNC_WQ_CREATE_HDMI  0x025
#define FUNC_WQ_DELETE_HDMI  0x026

#undef  TRACE_ENTER
#undef  TRACE_LEAVE
#undef  TRACE_LOG
#undef  TRACE_LOG1

#define TRACE_ENTER(ID)	tracelog_record(ID_TRACE_ENTER, __LINE__, ID, 0);
#define TRACE_LEAVE(ID)	tracelog_record(ID_TRACE_LEAVE, __LINE__, ID, 0);
#define TRACE_LOG(ID)	tracelog_record(ID_TRACE_LOG,   __LINE__, ID, 0);
#define TRACE_LOG1(ID, VAL1) \
	tracelog_record(ID_TRACE_LOG1, __LINE__, ID, VAL1);
#endif

/* macros for general error message */
#if _ERR_DBG >= 2
#define printk_err2(fmt, arg...) \
	do { \
		TRACE_LOG(FUNC_NONE); \
		if (debug > 1) \
			printk(KERN_ERR DEV_NAME ":E %s: " \
				fmt, __func__, ## arg); \
	} while (0)
#else
#define printk_err2(fmt, arg...)
#endif

/* macros for RT-API related error message */
#if _ERR_DBG >= 1
#define printk_err1(fmt, arg...) \
	do { \
		TRACE_LOG(FUNC_NONE); \
		if (debug > 0) \
			printk(KERN_ERR DEV_NAME ":E %s: " \
				fmt, __func__, ## arg); \
	} while (0)
#else
#define printk_err1(fmt, arg...)
#endif

#define printk_err(fmt, arg...) \
	do { \
		TRACE_LOG(FUNC_NONE); \
		printk(KERN_ERR DEV_NAME ":E %s: " fmt, __func__, ## arg); \
	} while (0)

/* macros for general log message */
#if _LOG_DBG >= 2
#define printk_dbg2(level, fmt, arg...) \
	do { \
		if ((level)+2 <= debug) \
			printk(KERN_INFO DEV_NAME ": %s: " fmt, \
				__func__, ## arg); \
	} while (0)
#else
#define printk_dbg2(level, fmt, arg...)
#endif

/* macros for RT-API log message */
#if _LOG_DBG >= 1
#define printk_dbg1(level, fmt, arg...) \
	do { \
		if ((level)+2 <= debug) \
			printk(KERN_INFO DEV_NAME ": %s: " fmt, \
				__func__, ## arg); \
	} while (0)
#else
#define printk_dbg1(level, fmt, arg...)
#endif

#define printk_dbg(level, fmt, arg...) \
	do { \
		if ((level)+2 <= debug) \
			printk(KERN_INFO DEV_NAME ": %s: " fmt, \
				__func__, ## arg); \
	} while (0)

#define DBGENTER(fmt, arg...) printk_dbg2(2, "in  "  fmt, ## arg)
#define DBGLEAVE(fmt, arg...) printk_dbg2(2, "out "  fmt, ## arg)

/******************************************************/
/* define local variables                             */
/******************************************************/
static const struct file_operations composer_fops = {
	.owner		= THIS_MODULE,
	.read		= core_read,
	.write		= NULL,
	.poll		= core_poll,
	.unlocked_ioctl	= core_ioctl,
	.open		= core_open,
	.release	= core_release,
};

static struct miscdevice composer_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "composer",
	.fops = &composer_fops,
};

#ifdef CONFIG_HAS_EARLYSUSPEND
static  struct early_suspend    early_suspend = {
	.level		= EARLY_SUSPEND_LEVEL_STOP_DRAWING,
	.suspend	= pm_early_suspend,
	.resume		= pm_late_resume,
};
#endif /* CONFIG_HAS_EARLYSUSPEND */

static struct composer_buffer local_buffer[MAX_BUFFER];
static spinlock_t             irqlock;
static DEFINE_SEMAPHORE(sem);
static int                    num_open;
static LIST_HEAD(file_top);
static struct localworkqueue  *workqueue;
static void                   *graphic_handle;
static int                    rtapi_hungup;
#ifdef CONFIG_MISC_R_MOBILE_COMPOSER_REQUEST_QUEUE
static unsigned long          queue_fb_map_address;
static unsigned long          queue_fb_map_endaddress;
static struct rtmem_phys_handle *queue_fb_map_handle;
static DECLARE_WAIT_QUEUE_HEAD(kernel_waitqueue_comp);
static DEFINE_SEMAPHORE(kernel_queue_sem);
static LIST_HEAD(kernel_queue_top);
#if SH_MOBILE_COMPOSER_WAIT_DRAWEND
static struct composer_rh     *current_overlayrequest;
static int                    overlay_draw_complete;
#endif
static struct composer_rh     kernel_request[MAX_KERNELREQ];
static spinlock_t             irqlock_timer;
static DEFINE_TIMER(kernel_queue_timer, \
	timeout_queue_process, 0, 0);
static struct localwork       expire_kernel_request;

#if SH_MOBILE_COMPOSER_SUPPORT_HDMI
static void                   *graphic_handle_hdmi;
static struct localwork       del_graphic_handle_hdmi;
static struct localwork       init_graphic_handle_hdmi;
#endif /* SH_MOBILE_COMPOSER_SUPPORT_HDMI */
#endif
#ifdef CONFIG_HAS_EARLYSUSPEND
static int                    in_early_suspend;
#endif /* CONFIG_HAS_EARLYSUSPEND */

static struct localwork       del_graphic_handle;
static struct localwork       init_graphic_handle;

#if INTERNAL_DEBUG >= 1
static int         internal_log_seqid = -1;
static char        *internal_log_msg;
static int         internal_log_msgsize;
static int         internal_log_length;
static int         internal_log_remain;
static int         log_tracebuf[TRACELOG_SIZE][3];
static spinlock_t  log_irqlock;
static int         log_tracebuf_wp;
#endif

/******************************************************/
/* local functions                                    */
/******************************************************/
static void localwork_init(
	struct localwork *work, void (*func)(struct localwork *))
{
	INIT_LIST_HEAD(&work->link);
	work->func = func;
	work->status = 0;
}


static void localworkquue_destroy(struct localworkqueue *wq)
{
	unsigned long flags;

	if (wq == NULL) {
		/* report error */
		printk_err("invalid argument.\n");
	} else {
		/* request task stop */
		if (wq->task)
			kthread_stop(wq->task);

		/* wakeup pending thread */
		printk_dbg2(3, "spinlock\n");
		spin_lock_irqsave(&wq->lock, flags);

		while (!list_empty(&wq->top)) {
			struct list_head *list;
			struct localwork *work = NULL;

			printk_dbg2(3, "localwork not empty\n");

			list_for_each(list, &wq->top)
			{
				work = list_entry(list,
					struct localwork, link);
				break;
			}
			if (work) {
				printk_dbg2(3, "localwork pending: %p\n", work);
				work->status = 1;
				list_del_init(&work->link);
			}
		}
		spin_unlock_irqrestore(&wq->lock, flags);

		wake_up_interruptible_all(&wq->wait);

		kfree(wq);
	}
}


static inline int localworkqueue_thread(void *arg)
{
	struct localworkqueue *wq = (struct localworkqueue *)arg;
	unsigned long flags;

	struct sched_param param = {.sched_priority = THREAD_PRIORITY};
	sched_setscheduler(current, SCHED_FIFO, &param);

	DBGENTER("\n");

	/* dev->th_events already initialized 0. */
	while (!kthread_should_stop()) {
		struct localwork *work = NULL;
		void   (*func)(struct localwork *);

		wait_event_interruptible(wq->wait, !list_empty(&wq->top));

		if (kthread_should_stop())
			break;

		printk_dbg2(3, "spinlock\n");
		spin_lock_irqsave(&wq->lock, flags);
		while (!list_empty(&wq->top)) {
			work = list_first_entry(&wq->top,
				struct localwork, link);

			printk_dbg2(3, "work:%p\n", work);

			func = work->func;
			spin_unlock_irqrestore(&wq->lock, flags);

			(*func)(work);

			spin_lock_irqsave(&wq->lock, flags);
			work->status = 1;
			list_del_init(&work->link);
			wake_up_all(&wq->finish);
		}
		spin_unlock_irqrestore(&wq->lock, flags);
	}

	DBGLEAVE("\n");
	return 0;
}


static struct localworkqueue *localworkqueue_create(char *taskname)
{
	struct localworkqueue *wq;

	wq = kmalloc(sizeof(*wq), GFP_KERNEL);
	if (wq == NULL) {
		/* report error */
		printk_err("can not create localwork.\n");
	} else {
		memset(wq, 0, sizeof(*wq));

		INIT_LIST_HEAD(&wq->top);
		spin_lock_init(&wq->lock);
		init_waitqueue_head(&wq->wait);
		init_waitqueue_head(&wq->finish);

		wq->task = kthread_run(localworkqueue_thread,
				     wq,
				     taskname);
		if (IS_ERR(wq->task)) {
			printk_err("could not create kernel thread\n");
			kfree(wq);
			wq = NULL;
		}
	}
	return wq;
}


static int localworkqueue_queue(
	struct localworkqueue *wq, struct localwork *work)
{
	unsigned long flags;
	int rc;
	DBGENTER("wq:%p work:%p\n", wq, work);
	if (wq && work) {
		rc = 1;

		spin_lock_irqsave(&wq->lock, flags);
		if (list_empty(&work->link)) {
			list_add_tail(&work->link, &wq->top);
			work->status = 0;
		} else {
			printk_err2("work %p alredy queued.\n", work);
			rc = 0;
		}
		spin_unlock_irqrestore(&wq->lock, flags);

		if (rc)
			wake_up_interruptible(&wq->wait);
	} else {
		/* set error code */
		printk_err("invalid argument.\n");
		rc = 0;
	}
	DBGLEAVE("%d\n", rc);
	return rc;
}

static void localworkqueue_flush(
	struct localworkqueue *wq, struct localwork *work)
{
	unsigned long flags;
	int rc = 0;
	DBGENTER("wq:%p work:%p\n", wq, work);
	if (wq && work) {
		int wait = 0;
		spin_lock_irqsave(&wq->lock, flags);
		if (work->status) {
			/* wait is not necessary. */
			printk_dbg2(3, "work %p finished.\n", work);
		} else if (list_empty(&work->link)) {
			/* report error */
			printk_dbg2(3, "work %p not queued\n", work);
			rc = -EINVAL;
		} else
			wait = 1;
		spin_unlock_irqrestore(&wq->lock, flags);

		if (wait) {
			printk_dbg2(3, "wait complete of work %p\n", work);
			wait_event( \
				wq->finish, work->status != 0);
		}
	} else {
		/* set error code */
		printk_err("invalid argument.\n");
		rc = -EINVAL;
	}
	DBGLEAVE("%d\n", rc);
	return;
}


static void initialize_blendcommon_obj(struct composer_blendcommon *obj,
	screen_grap_image_blend *_blend,
	void                    (*callback)(int result, void *user_data),
	void                     *user_data)
{
	init_waitqueue_head(&obj->wait_notify);
	obj->_blend    = _blend;
	obj->callback  = callback;
	obj->user_data = user_data;
}

static void initialize_bufferinformation(struct composer_buffer *buf)
{
	spin_lock_init(&buf->buf_lock);
	sema_init(&buf->buf_sem, 1);

	INIT_LIST_HEAD(&buf->buf_link.list);
	buf->buf_link.body = NULL; /* no buffer binding */
	buf->buf_link.fh   = NULL; /* no buffer binding */

	buf->buf_id          = 0;
	buf->keycolor        = CMPKEYCOLOR_OFF;
	buf->backcolor       = 0;
	buf->alpha           = 255;
	buf->data_fmt        = CMPPIXFMT_ARGB8888;
	buf->col_space       = CMPYUVCOLOR_BT601_COMPRESS;
	buf->crop_pos_x      = 0;
	buf->crop_pos_y      = 0;
	buf->crop_size_x     = 0;
	buf->crop_size_y     = 0;
};

static struct composer_fh *allocate_device(void)
{
	struct composer_fh *fh;
	int    i;

	DBGENTER("\n");

	fh = kmalloc(sizeof(*fh), GFP_KERNEL);
	if (NULL == fh)
		goto err_exit;

	/* initialize handle */
	memset(fh, 0, sizeof(*fh));

	/* initialize localwork */
	localwork_init(&fh->fh_wqtask, work_runblend);

	INIT_LIST_HEAD(&fh->fh_filelist);

	printk_dbg2(3, "spinlock\n");
	spin_lock(&irqlock);
	list_add_tail(&fh->fh_filelist, &file_top);
	spin_unlock(&irqlock);

	sema_init(&fh->fh_sem, 1);

	rwlock_init(&fh->fh_rwlock);

	init_waitqueue_head(&fh->fh_wait);
	initialize_blendcommon_obj(&fh->fh_wqcommon,
		&fh->grap_data._blend,
		callback_iocs_start,
		fh);

	for (i = 0; i < 5; i++) {
		struct composer_info     *fh_info   = &fh->fh_info[i];
		struct composer_buffer   *fh_buffer = &fh->fh_buffer_id0[i];

		/* initialize open buffer information */

		INIT_LIST_HEAD(&fh_info->wqlink.list);
		fh_info->wqlink.body = &fh->fh_wait;
		fh_info->wqlink.fh   = fh;

		fh_info->id         = 0;
		fh_info->buffer     = NULL;
		fh_info->appinfo[0] = NULL;
		fh_info->appinfo[1] = NULL;
		fh_info->appinfo[2] = NULL;

		/* initialize local buffer information */
		initialize_bufferinformation(fh_buffer);
	}

	fh->fh_status = 0;
err_exit:
	DBGLEAVE("%p\n", fh);

	return fh;
}

static void  free_device(struct composer_fh *fh)
{
	int    i;
	static const int fh_status_translate[] = {
		[IDX_OUTPLANE]  = FH_STATUS_OUTPLANE,
		[IDX_CH1LAYER]  = FH_STATUS_CH1LAYER,
		[IDX_CH2LAYER]  = FH_STATUS_CH2LAYER,
		[IDX_CH3LAYER]  = FH_STATUS_CH3LAYER,
		[IDX_CH4LAYER]  = FH_STATUS_CH4LAYER
	};
	static const  int level[5] = {
		[IDX_OUTPLANE]  = CMP_OUT_PLANE,
		[IDX_CH1LAYER]  = CMP_CH1_LAYER,
		[IDX_CH2LAYER]  = CMP_CH2_LAYER,
		[IDX_CH3LAYER]  = CMP_CH3_LAYER,
		[IDX_CH4LAYER]  = CMP_CH4_LAYER };

	DBGENTER("fh:%p\n", fh);

	printk_dbg2(3, "down\n");
	down(&fh->fh_sem);

	/* release open buffer */
	for (i = 0; i < ARRAY_SIZE(level); i++) {
		struct cmp_lay_close close;

		if (fh->fh_status & fh_status_translate[i]) {
			printk_dbg2(3, "index %d closing.", level[i]);
			close.level = level[i];
			iocs_close(fh, &close);
		} else {
			printk_dbg2(3, "index %d not open.", level[i]);
		}
	}
	list_del_init(&fh->fh_filelist);

	/* unlock semaphore */
	up(&fh->fh_sem);
	wake_up_interruptible_all(&fh->fh_wait);
	kfree(fh);

	DBGLEAVE("\n");
}

static int    get_index_from_level(int level, int *idx)
{
	static const int fh_level_translate[] = {
		[CMP_OUT_PLANE]  = IDX_OUTPLANE,
		[CMP_CH1_LAYER]  = IDX_CH1LAYER,
		[CMP_CH2_LAYER]  = IDX_CH2LAYER,
		[CMP_CH3_LAYER]  = IDX_CH3LAYER,
		[CMP_CH4_LAYER]  = IDX_CH4LAYER
	};
	if (level < 0 || level >= ARRAY_SIZE(fh_level_translate)) {
		printk_err2("argument level %d invalid.\n", level);
		return CMP_NG;
	}
	*idx = fh_level_translate[level];
	return CMP_OK;
}

static struct composer_info *get_composer_info(
	struct composer_fh *fh, int level)
{
	int                      idx;
	static const int fh_status_translate[] = {
		[CMP_OUT_PLANE]  = FH_STATUS_OUTPLANE,
		[CMP_CH1_LAYER]  = FH_STATUS_CH1LAYER,
		[CMP_CH2_LAYER]  = FH_STATUS_CH2LAYER,
		[CMP_CH3_LAYER]  = FH_STATUS_CH3LAYER,
		[CMP_CH4_LAYER]  = FH_STATUS_CH4LAYER,
	};

	if (get_index_from_level(level, &idx) != CMP_OK) {
		printk_err2("argument level %d invalid.\n", level);
		return NULL;
	}

	if ((fh->fh_status &  fh_status_translate[level]) == 0) {
		printk_err2("level %d not opend.\n", level);
		return NULL;
	}

	return &fh->fh_info[idx];
}

static int  bind_buffer(struct composer_fh *fh, int idx, int id)
{
	struct composer_buffer *buf;
	struct composer_info   *info;

	DBGENTER("fh:%p idx:%d id:%d\n", fh, idx, id);

	if (id == 0) {
		/* use private buffer */
		buf = &fh->fh_buffer_id0[idx];
	} else {
		/* use global buffer */
		buf = &local_buffer[id];
	}

	if (idx == IDX_OUTPLANE) {
		if (buf->buf_id & BUF_ID_FLG_OUTPLANE) {
			printk_err2("buffer already opened for output.\n");
			DBGLEAVE("%d\n", -EAGAIN);
			return -EAGAIN;
		}
	}

	info   = &fh->fh_info[idx];

	printk_dbg2(3, "info->buffer: %p\n", info->buffer);

	/* set bind to buffer */
	info->id       = (idx<<24) | (id<<16) | 0x0;
	info->buffer   = buf;

	/* lock buffer. */
	if (lock_buffer(fh, idx) < 0) {
		printk_err2("lock_buffer interrupted.\n");

		info->id       = 0;
		info->buffer   = NULL;

		DBGLEAVE("%d\n", -EAGAIN);
		return -EAGAIN;
	}

	printk_dbg2(3, "spinlock\n");
	spin_lock(&irqlock);

	/* buffer bind to info */
	list_add_tail(&info->wqlink.list, &buf->buf_link.list);

	spin_unlock(&irqlock);

	if (idx == IDX_OUTPLANE) {
		/* mark this buffer opend for output */

		printk_dbg2(3, "spinlock\n");
		spin_lock(&buf->buf_lock);

		buf->buf_id |= BUF_ID_FLG_OUTPLANE;

		spin_unlock(&buf->buf_lock);
	}

	unlock_buffer(fh, idx);

	DBGLEAVE("%d\n", 0);
	return 0;
}

static int  release_buffer(struct composer_fh *fh, int level)
{
	struct composer_info   *info;
	struct composer_buffer *buffer;
	struct appmem_handle   *app[3];
	int                  info_id;
	int           i;
	int           idx;

	DBGENTER("fh:%p level:%d\n", fh, level);

	if (get_index_from_level(level, &idx) != CMP_OK) {
		printk_err2("argument level %d invalid.\n", level);
		DBGLEAVE("%d\n", -EINVAL);
		return -EINVAL;
	}

	info   = &fh->fh_info[idx];
	buffer = info->buffer;

	printk_dbg2(3, "buffer: %p\n", buffer);

	if (0 != buffer->buf_sem.count) {
		/* report bug */
		printk_err("it's necessary to acquire buf_sem semaphore\n");
	}

	/* reset buffer info */
	info_id = info->id;
	app[0]  = info->appinfo[0];
	app[1]  = info->appinfo[1];
	app[2]  = info->appinfo[2];

	info->id = 0;
	info->buffer = NULL;
	info->appinfo[0] = NULL;
	info->appinfo[1] = NULL;
	info->appinfo[2] = NULL;

	/* acqure lock of buffer */
	printk_dbg2(3, "spinlock");
	spin_lock(&irqlock);

	/* remove link to current buffer info */
	list_del_init(&info->wqlink.list);

	if (list_empty(&buffer->buf_link.list)) {
		printk_dbg2(3, "clear CMP_IOCS_LAYADDR setting.\n");
		/* spinlock is not necessary. */
		buffer->buf_id &= ~BUF_ID_SET_BUFFER;
	} else {
		/* this buffer used by othere fd. */
		printk_dbg2(3, "not clear CMP_IOCS_LAYADDR setting.\n");
	}

	spin_unlock(&irqlock);

	if (idx == IDX_OUTPLANE) {
		/* reset mark of buffer opend for output */

		printk_dbg2(3, "spinlock\n");
		spin_lock(&buffer->buf_lock);

		buffer->buf_id &= ~BUF_ID_FLG_OUTPLANE;

		spin_unlock(&buffer->buf_lock);
	}

	/*****************************************/
	/* unlock_buffer(fh, idx) can not used.  */
	/* threafore, implement unlock directly. */
	/*****************************************/
	up(&buffer->buf_sem);

	/* notify waiting thread */
	wakeup_allfile(buffer);

	/* wakeup current handle */
	wake_up_interruptible(&fh->fh_wait);

	/* release appShare memory */
	for (i = 0; i < ARRAY_SIZE(app); i++) {
		int rc = 0;
		if (app[i]) {
			rc = sh_mobile_appmem_free(app[i]);
			/* ignore error, because recovery from error \
			   is only available by reboot-kernel.       */
		}
		if (rc) {
			/* report error */
			printk_err("sh_mobile_appmem_free return error\n");
		}
	}
	DBGLEAVE("%d\n", 0);
	return 0;
}

static int  lock_buffer(struct composer_fh *fh, int level)
{
	struct composer_info   *info   = &fh->fh_info[level];
	struct composer_buffer *buffer = info->buffer;
	unsigned long          start_jiffies;
	int      retval = CMP_NG;

	DBGENTER("fh:%p idx:%d\n", fh, level);
	printk_dbg2(3, "buffer: %p\n", buffer);

	start_jiffies = jiffies;
	do {
		if (!down_trylock(&buffer->buf_sem)) {
			/* lock successed. */
			retval = CMP_OK;
			break;
		} else {
			int rc;

			printk_dbg2(3, "wait unlock-buffer\n");
			rc = wait_event_interruptible_timeout(
				fh->fh_wait,
				buffer->buf_sem.count > 0,
				2 * HZ);

			if (rc < 0) {
				printk_err2("buffer lock interrupted.\n");
				retval = rc;
				goto err_exit;
			}
		}
	} while ((int)(jiffies - start_jiffies) < 2 * HZ);

	if (retval) {
		printk_dbg2(3, "can not lock_buffer.\n");
		retval = -EAGAIN;
		goto err_exit;
	}

	/* should acquire semaphore. */
	if (0 != buffer->buf_sem.count) {
		/* report buf */
		printk_err("it's necessary to acquire buf_sem semaphore\n");
	}
err_exit:
	DBGLEAVE("%d\n", retval);
	return retval;
}
static int  unlock_buffer(struct composer_fh *fh, int level)
{
	struct composer_info   *info   = &fh->fh_info[level];
	struct composer_buffer *buffer = info->buffer;

	DBGENTER("fh:%p idx:%d\n", fh, level);
	printk_dbg2(3, "buffer: %p\n", buffer);

	if (NULL == buffer) {
		/* report error */
		printk_err2("buffer level %d not binding\n", level);
	} else {
		if (0 != buffer->buf_sem.count) {
			/* report bug */
			printk_err("currently not acquire buf_sem semaphore\n");
		}

		up(&buffer->buf_sem);

		/* notify waiting thread */
		wakeup_allfile(buffer);
	}

	DBGLEAVE("%d\n", 0);
	return 0;
}
static void  wakeup_allfile(struct composer_buffer *buffer)
{
	struct list_head *list;

	DBGENTER("buffer:%p\n", buffer);

	if (NULL == buffer) {
		/* report error */
		printk_err2("buffer not opened.\n");
	} else {
		printk_dbg2(3, "spinlock\n");
		spin_lock(&irqlock);

		/* wakeup waiting thread */
		list_for_each(list, &buffer->buf_link.list)
		{
			struct bufferlink *buflink;

			buflink = list_entry((void *)list, \
				struct bufferlink, list);

			printk_dbg2(3, "wakeup queue ptr:%p\n", buflink->body);
			if (buflink->body) {
				/* wake up waiting thread. */
				wake_up_interruptible_all(buflink->body);
			}
		}
		spin_unlock(&irqlock);
	}

	DBGLEAVE("\n");
	return;
}


static void allfile_status_set(struct composer_buffer *buffer, int status)
{
	struct list_head *list;

	DBGENTER("buffer:%p status:0x%x\n", buffer, status);

	if (NULL == buffer) {
		/* report error */
		printk_err2("buffer not opened.\n");
	} else {
		printk_dbg2(3, "spinlock\n");
		spin_lock(&irqlock);

		printk_dbg2(3, "status 0x%x append to file\n", status);
		/* wakeup waiting thread */
		list_for_each(list, &buffer->buf_link.list)
		{
			struct bufferlink  *buflink;
			struct composer_fh *fh;

			buflink = list_entry((void *)list, \
				struct bufferlink, list);

			fh = buflink->fh;
			if (fh) {
				fh->fh_status |= status;
				printk_dbg2(3, "set status %x to fh:%p\n", \
					status, fh);
			}
		}

		spin_unlock(&irqlock);
	}

	DBGLEAVE("\n");
	return;
}



static int update_work_linebytes(
	struct composer_grapdata *grap, int idx, struct composer_buffer *buffer)
{
	int c[3], v[3];
	int w;
	int rc = CMP_NG;

	DBGENTER("grap:%p idx:%d buffer:%p\n", grap, idx, buffer);

	if ((buffer->buf_id & BUF_ID_FLG_PIXFMT) != BUF_ID_FLG_PIXFMT) {
		printk_dbg2(2, "pixelfmt not initialized.");
		goto err_exit;
	}
	if ((buffer->buf_id & BUF_ID_FLG_LAYSIZE) != BUF_ID_FLG_LAYSIZE) {
		printk_dbg2(2, "laysize not initialized.");
		goto err_exit;
	}
	if (idx >= ARRAY_SIZE(grap->work_linebyte)) {
		printk_err("idx %d invalid.", idx);
		goto err_exit;
	}

	/* set pixel data size. fixpoint format 31.1 */
	switch (buffer->data_fmt) {
	case CMPPIXFMT_ARGB8888:
	default:
		c[0] = 4*2; c[1] = 0; c[2] = 0; break;
	case CMPPIXFMT_RGB888:
		c[0] = 3*2; c[1] = 0; c[2] = 0; break;
	case CMPPIXFMT_RGB565:
		c[0] = 2*2; c[1] = 0; c[2] = 0; break;
	case CMPPIXFMT_YUV420SP:
	case CMPPIXFMT_YUV422SP:
		c[0] = 1*2; c[1] = 1*2; c[2] = 0; break;
	case CMPPIXFMT_YUV420PL:
		c[0] = 1*2; c[1] = 1; c[2] = 1; break;
	}
	printk_dbg2(3, "data_fmt:%d  " \
		"its pixel size is [31.1] %d, %d, %d\n", \
		buffer->data_fmt, c[0], c[1], c[2]);

	/* calculate linebyte */
	w = buffer->data_x;
	v[0] = w * c[0]/2;
	v[1] = w * c[1]/2;
	v[2] = w * c[2]/2;

	printk_dbg2(3, "linebyte: %d, %d, %d\n", \
		v[0], v[1], v[2]);

	grap->work_linebyte[idx]    = v[0];
	grap->work_linebyte_c0[idx] = v[1];
	grap->work_linebyte_c1[idx] = v[2];
	grap->work_pixelsize[idx]    = c[0];
	grap->work_pixelsize_c0[idx] = c[1];
	grap->work_pixelsize_c1[idx] = c[2];
	rc = CMP_OK;
err_exit:
	DBGLEAVE("%d\n", rc);
	return rc;
}

static int update_work_imagesize(
	struct composer_grapdata *grap, int idx, struct composer_buffer *buffer)
{
	int w, h;
	int c_x, c_y, c_w, c_h;
	int rc = CMP_OK;

	DBGENTER("grap:%p idx:%d buffer:%p\n", grap, idx, buffer);

	if ((buffer->buf_id & BUF_ID_FLG_LAYSIZE) != BUF_ID_FLG_LAYSIZE) {
		printk_dbg2(2, "laysize not initialized.");
		rc = CMP_NG;
		goto err_exit;
	}
	if (idx >= ARRAY_SIZE(grap->work_linebyte)) {
		printk_err("idx %d invalid.", idx);
		rc = CMP_NG;
		goto err_exit;
	}

	w = buffer->data_x;
	h = buffer->data_y;

	/* set default crop */
	c_x = 0;
	c_y = 0;
	c_w = w;
	c_h = h;

	/* update crop */
	if ((buffer->buf_id & BUF_ID_FLG_CROPPOS) == BUF_ID_FLG_CROPPOS) {
		c_x = buffer->crop_pos_x;
		c_y = buffer->crop_pos_y;
	}
	if ((buffer->buf_id & BUF_ID_FLG_CROPSIZE) == BUF_ID_FLG_CROPSIZE) {
		c_w = buffer->crop_size_x;
		c_h = buffer->crop_size_y;
	}

	printk_dbg2(3, "image size:%d x %d, crop (%d, %d)-(%d, %d)\n", \
		w, h, c_x, c_y, c_w, c_h);

	grap->work_rect_x[idx] = c_x;
	grap->work_rect_y[idx] = c_y;
	grap->work_rect_w[idx] = c_w;
	grap->work_rect_h[idx] = c_h;

err_exit:
	DBGLEAVE("%d\n", rc);
	return rc;
}

int update_work_appshare(
	struct composer_info   *info, struct composer_buffer *buffer)
{
	static const int pixfmt_numplane_translate[] = {
		[CMPPIXFMT_ARGB8888] = 1,
		[CMPPIXFMT_RGB888] = 1,
		[CMPPIXFMT_RGB565] = 1,
		[CMPPIXFMT_YUV420SP] = 2,
		[CMPPIXFMT_YUV422SP] = 2,
		[CMPPIXFMT_YUV420PL] = 3
	};
	static const int fh_status_translate_app[] = {
		[0] = INFO_ID_SET_APP0,
		[1] = INFO_ID_SET_APP1,
		[2] = INFO_ID_SET_APP2
	};
	int  i;
	int  num_plane;
	int  rc = CMP_OK;

	DBGENTER("info:%p buffer:%p\n", info, buffer);

	if ((buffer->buf_id & BUF_ID_SET_BUFFER) == 0) {
		printk_dbg2(2, "CMP_IOCS_LAYADDR not specified.\n");
		rc = CMP_NG;
		goto err_exit;
	}

	if ((buffer->buf_id & BUF_ID_FLG_PIXFMT) == 0) {
		printk_dbg2(2, "CMP_IOCS_PIXFMT not specified.\n");
		rc = 1;
		goto err_exit;
	}

	/* should acquire semaphore. */
	if (0 != buffer->buf_sem.count) {
		/* report buf */
		printk_err("it's necessary to acquire buf_sem semaphore\n");
	}

	num_plane = pixfmt_numplane_translate[buffer->data_fmt];
	if (num_plane < 1 || num_plane > 3) {
		/* report error */
		printk_err("num_plane %d invalid.\n", num_plane);
	}

	{
		void *app[3] = {NULL, NULL, NULL};
		int  appid[3], appoffset[3], appopen_flag;

/***************************/
/* pickup-informatin       */
/***************************/
		/* copy previous created APP-Share */
		for (i = 0; i < 3; i++) {
			if (info->id & fh_status_translate_app[i]) {
				info->id  &= ~fh_status_translate_app[i];
				app[i] = info->appinfo[i];
				info->appinfo[i] = NULL;
			}
		}

		appopen_flag = 0;
		if (buffer->addr == NULL && num_plane > 0) {
			appid[0]     = buffer->app_id;
			appoffset[0] = buffer->offset;
			appopen_flag |= 0x01;
		}
		if (buffer->addr_c0 == NULL && num_plane > 1) {
			appid[1]     = buffer->app_id_c0;
			appoffset[1] = buffer->offset_c0;
			appopen_flag |= 0x02;
		}
		if (buffer->addr_c1 == NULL && num_plane > 2) {
			appid[2]     = buffer->app_id_c1;
			appoffset[2] = buffer->offset_c1;
			appopen_flag |= 0x04;
		}

/***************************/
/* app-share close.        */
/***************************/
		for (i = 0; i < 3; i++) {
			int app_rc = 0;
			if (app[i]) {
				app_rc = sh_mobile_appmem_free(app[i]);
				/* ignore error, because recovery from error \
				   is only available by reboot-kernel.       */
			}
			if (app_rc) {
				printk_err("sh_mobile_appmem_free"
					" return by error\n");
			}
			app[i] = NULL;
		}

		info->id &= ~(INFO_ID_SET_APP0 | \
			INFO_ID_SET_APP1 | INFO_ID_SET_APP2);
/***************************/
/* app-share open.        */
/***************************/
		for (i = 0; i < 3; i++) {
			if ((appopen_flag & (1<<i)) == 0) {
				/* to next buffer */
				continue;
			}
			app[i] = sh_mobile_appmem_share(appid[i], DEV_NAME);
			if (app[i] == NULL) {
				printk_err("sh_mobile_appmem_share"
					" return by error\n");
				rc = CMP_NG;
			}
		}

/***************************/
/* last process            */
/***************************/
		if (rc != CMP_OK) {
			/* app share close by error */
			for (i = 0; i < 3; i++) {
				int app_rc = 0;
				if (app[i]) {
					app_rc = sh_mobile_appmem_free(app[i]);
					/* recovery from error is only \
					   available by reboot-kernel. */
				}
				if (app_rc) {
					printk_err("sh_mobile_appmem_free"
						" return by error\n");
				}
				app[i] = NULL;
			}
		} else {
			/* record open result */
			for (i = 0; i < 3; i++) {
				if (app[i]) {
					/* record app share flag */
					info->id  |= fh_status_translate_app[i];
				}
				info->appinfo[i] = app[i];
			}
		}
	}

err_exit:
	DBGLEAVE("%d\n", rc);
	return rc;
}

static int update_grap_image_param(
	screen_grap_image_param * param,
	struct composer_info    *info,
	struct composer_buffer  *buf,
	struct composer_grapdata *grap,
	int    idx)
{
	static const int pixfmt_numplane_translate[] = {
		[CMPPIXFMT_ARGB8888] = 1,
		[CMPPIXFMT_RGB888] = 1,
		[CMPPIXFMT_RGB565] = 1,
		[CMPPIXFMT_YUV420SP] = 2,
		[CMPPIXFMT_YUV422SP] = 2,
		[CMPPIXFMT_YUV420PL] = 3
	};
	static const int rt_graphics_color_transleta[] = {
		[CMPPIXFMT_ARGB8888] = RT_GRAPHICS_COLOR_ARGB8888,
		[CMPPIXFMT_RGB888]   = RT_GRAPHICS_COLOR_RGB888,
		[CMPPIXFMT_RGB565]   = RT_GRAPHICS_COLOR_RGB565,
		[CMPPIXFMT_YUV420SP] = RT_GRAPHICS_COLOR_YUV420SP,
		[CMPPIXFMT_YUV422SP] = RT_GRAPHICS_COLOR_YUV422SP,
#ifdef RT_GRAPHICS_COLOR_YUV420PL
		[CMPPIXFMT_YUV420PL] = RT_GRAPHICS_COLOR_YUV420PL,
#endif
	};
	static const int rt_yuvformat_translate[][2] = {
		[CMPYUVCOLOR_BT601_COMPRESS][0] = RT_GRAPHICS_COLOR_BT601,
		[CMPYUVCOLOR_BT601_COMPRESS][1] = RT_GRAPHICS_COLOR_COMPRESSED,
		[CMPYUVCOLOR_BT601_FULLSCALE][0] = RT_GRAPHICS_COLOR_BT601,
		[CMPYUVCOLOR_BT601_FULLSCALE][1] = RT_GRAPHICS_COLOR_FULLSCALE,
		[CMPYUVCOLOR_BT709_COMPRESS][0] = RT_GRAPHICS_COLOR_BT709,
		[CMPYUVCOLOR_BT709_COMPRESS][1] = RT_GRAPHICS_COLOR_COMPRESSED,
		[CMPYUVCOLOR_BT709_FULLSCALE][0] = RT_GRAPHICS_COLOR_BT709,
		[CMPYUVCOLOR_BT709_FULLSCALE][1] = RT_GRAPHICS_COLOR_FULLSCALE,
	};
	int  rc, num_plane;
	int  offset, offset_c;

	DBGENTER("param:%p info:%p, buf:%p grap:%p\n", param, info, buf, grap);

	num_plane = pixfmt_numplane_translate[buf->data_fmt];
	rc = CMP_OK;

	param->width     = grap->work_rect_w[idx];
	param->height    = grap->work_rect_h[idx];
	param->stride    = grap->work_linebyte[idx];
	param->stride_c  = grap->work_linebyte_c0[idx];
	switch (buf->data_fmt) {
	case CMPPIXFMT_YUV420PL:
	case CMPPIXFMT_YUV420SP:
		offset    = param->stride    * grap->work_rect_y[idx];
		offset_c  = param->stride_c  * grap->work_rect_y[idx]/2;
		break;
	default:
		offset    = param->stride    * grap->work_rect_y[idx];
		offset_c  = param->stride_c  * grap->work_rect_y[idx];
		break;
	}
	offset   += grap->work_pixelsize[idx]    * grap->work_rect_x[idx] / 2;
	offset_c += grap->work_pixelsize_c0[idx] * grap->work_rect_x[idx] / 2;

	printk_dbg2(3, "offset = 0x%x (stride:%d, y:%d, x:%d)\n", offset, \
		param->stride, grap->work_rect_y[idx], grap->work_rect_x[idx]);
	printk_dbg2(3, "offset_c = 0x%x (stride:%d, y:%d, x:%d)\n", offset_c, \
		param->stride_c,
		grap->work_rect_y[idx], grap->work_rect_x[idx]);

	param->format    = rt_graphics_color_transleta[buf->data_fmt];
	switch (buf->data_fmt) {
	case CMPPIXFMT_YUV420SP:
	case CMPPIXFMT_YUV422SP:
	case CMPPIXFMT_YUV420PL:
		param->yuv_format = rt_yuvformat_translate[buf->col_space][0];
		param->yuv_range  = rt_yuvformat_translate[buf->col_space][1];
		break;
	default:
		param->yuv_format = RT_GRAPHICS_COLOR_BT601;
		param->yuv_range  = RT_GRAPHICS_COLOR_COMPRESSED;
		break;
	}

	/* set plane 1 */
	if (buf->addr) {
		param->address      = buf->addr;
		param->apmem_handle = NULL;
	} else if (info->id & INFO_ID_SET_APP0) {
		if (info->appinfo[0] == NULL) {
			printk_err2("app share 0 is not opoen\n");
			rc = CMP_NG;
			goto err_exit;
		}
		param->address      = sh_mobile_appmem_getaddress(
			info->appinfo[0], buf->offset);
		param->apmem_handle = sh_mobile_appmem_getmemoryhandle(
			info->appinfo[0]);
		if (param->address == NULL) {
			printk_err2("can not get param->address"
				" of appinfo[0]\n");
			rc = CMP_NG;
			goto err_exit;
		}
		if (param->apmem_handle == NULL) {
			printk_err2("can not get param->apmem_handle"
				" of appinfo[0]\n");
			rc = CMP_NG;
			goto err_exit;
		}
	} else {
		param->address      = NULL;
		param->apmem_handle = NULL;
		rc = CMP_NG;
		printk_err2("can not set Y/RGB plane address\n");
	}
	printk_dbg2(3, "address adjust to %p to %p\n", param->address, \
		param->address + offset);
	/* adjsut crop position */
	param->address += offset;

	param->address_c0      = NULL;
	param->apmem_handle_c0 = NULL;
	param->address_c1      = NULL;
	param->apmem_handle_c1 = NULL;
	if (num_plane <= 1) {
		/* return */
		goto finish;
	}

	/* set plane 2 */
	if (buf->addr_c0) {
		param->address_c0      = buf->addr_c0;
		param->apmem_handle_c0 = NULL;
	} else if (info->id & INFO_ID_SET_APP1) {
		if (info->appinfo[1] == NULL) {
			printk_err2("app share 1 is not opoen\n");
			rc = CMP_NG;
			goto err_exit;
		}
		param->address_c0      = sh_mobile_appmem_getaddress(
			info->appinfo[1], buf->offset_c0);
		param->apmem_handle_c0 = sh_mobile_appmem_getmemoryhandle(
			info->appinfo[1]);
		if (param->address_c0 == NULL) {
			printk_err2("can not get param->address"
				" of appinfo[1]\n");
			rc = CMP_NG;
			goto err_exit;
		}
		if (param->apmem_handle_c0 == NULL) {
			printk_err2("can not get param->apmem_handle"
				" of appinfo[1]\n");
			rc = CMP_NG;
			goto err_exit;
		}
	} else {
		/* set error. already initialize parameteres. */
		rc = CMP_NG;
		printk_err2("can not set UV/U plane address\n");
	}
	printk_dbg2(3, "address_c0 adjust to %p to %p\n", param->address_c0, \
		param->address_c0 + offset_c);
	/* adjsut crop position */
	param->address_c0 += offset_c;

	if (num_plane <= 2) {
		/* return */
		goto finish;
	}

	/* set plane 3 */
	if (buf->addr_c1) {
		param->address_c1      = buf->addr_c1;
		param->apmem_handle_c1 = NULL;
	} else if (info->id & INFO_ID_SET_APP2) {
		if (info->appinfo[2] == NULL) {
			printk_err2("app share 2 is not opoen\n");
			rc = CMP_NG;
			goto err_exit;
		}
		param->address_c1      = sh_mobile_appmem_getaddress(
			info->appinfo[2], buf->offset_c1);
		param->apmem_handle_c1 = sh_mobile_appmem_getmemoryhandle(
			info->appinfo[2]);
		if (param->address_c1 == NULL) {
			printk_err2("can not get param->address"
				" of appinfo[2]\n");
			rc = CMP_NG;
			goto err_exit;
		}
		if (param->apmem_handle_c1 == NULL) {
			printk_err2("can not get param->apmem_handle"
				" of appinfo[2]\n");
			rc = CMP_NG;
			goto err_exit;
		}
	} else {
		/* set error. already initialize parameteres. */
		rc = CMP_NG;
		printk_err2("can not set V plane address\n");
	}
	printk_dbg2(3, "address_c1 adjust to %p to %p\n", param->address_c1, \
		param->address_c1 + offset_c);
	/* adjsut crop position */
	param->address_c1 += offset_c;

finish:
	/* pass */
	rc = CMP_OK;
err_exit:
	DBGLEAVE("%d\n", rc);
	return rc;
};


static int update_grap_arguments(struct composer_fh *fh)
{
	struct composer_grapdata *grap   = &fh->grap_data;
	int                      nBuffer = 0;
	struct composer_info     *info[5];
	struct composer_buffer   *buf[5];
	int                      bufidx[5];

	int                      i;
	int                      rc;

	static const int fh_status_translate[] = {
		[IDX_OUTPLANE]  = FH_STATUS_OUTPLANE,
		[IDX_CH1LAYER]  = FH_STATUS_CH1LAYER,
		[IDX_CH2LAYER]  = FH_STATUS_CH2LAYER,
		[IDX_CH3LAYER]  = FH_STATUS_CH3LAYER,
		[IDX_CH4LAYER]  = FH_STATUS_CH4LAYER
	};

	DBGENTER("fh:%p\n", fh);

	/* pick layer information. */
	for (i = 0; i < ARRAY_SIZE(fh_status_translate); i++) {
		if ((fh->fh_status & fh_status_translate[i]) == 0) {
			/* not open */
			continue;
		}

		if (nBuffer >= 5) {
			/* report error */
			printk_err("nBuffer %d out of valid range\n", nBuffer);
		}

		info[nBuffer] = &fh->fh_info[i];
		buf[nBuffer]  = fh->fh_info[i].buffer;
		bufidx[nBuffer] = i;

		/* check buffer configuration */
		if (fh->fh_info[i].id & INFO_ID_FLG_NOTDISP) {
			/* not blending plane */
			continue;
		}
		nBuffer++;
	}

	/* this error should detect in chk_ioc_start() */
	if (nBuffer < 1) {
		/* report error */
		printk_err("nBuffer %d out of valid range\n", nBuffer);
	}

/* update for screen_grap_image_blend */
	grap->_blend.user_data = (unsigned long) &fh->fh_wqcommon;

	/* create destination parameter */
	if (bufidx[0] != IDX_OUTPLANE) {
		/* report error */
		printk_err("bufidx[0] is not output plane\n");
	}
	i = 0;
	rc = update_grap_image_param(&grap->_blend.output_image,
		info[i], buf[i], grap, bufidx[i]);
	if (rc != CMP_OK) {
		printk_err2("parameter convert error in CMP_OUT_PLANE\n");
		goto err_exit;
	}

	grap->_blend.background_color = buf[i]->backcolor;

	/* create source parameter */
	grap->_blend.input_layer[0] = NULL;
	grap->_blend.input_layer[1] = NULL;
	grap->_blend.input_layer[2] = NULL;
	grap->_blend.input_layer[3] = NULL;
	for (i = 1; i < nBuffer; i++) {
		int layer_no = i-1;
		screen_grap_layer *_layer;

		_layer = &grap->_in[layer_no];
		rc = update_grap_image_param(&_layer->image,
			info[i], buf[i], grap, bufidx[i]);
		if (rc != CMP_OK) {
			printk_err2("parameter convert error"
				" in SRC image [%d]\n", bufidx[i]);
			goto err_exit;
		}

		grap->_blend.input_layer[layer_no] = _layer;
		_layer->rect.x      = buf[i]->pos_x;
		_layer->rect.y      = buf[i]->pos_y;
		_layer->rect.width  = buf[i]->size_x;
		_layer->rect.height = buf[i]->size_y;
		if (_layer->image.width == _layer->rect.width &&
		   _layer->image.height == _layer->rect.height) {
			/* no need resize */
			_layer->rect.width  = 0;
			_layer->rect.height = 0;
		}
		_layer->alpha       = buf[i]->alpha;

		_layer->rotate      = RT_GRAPHICS_ROTATE_0;
		_layer->mirror      = RT_GRAPHICS_MIRROR_N;
		if (buf[i]->keycolor == CMPKEYCOLOR_OFF) {
			/* no need key color */
			_layer->key_color = \
				RT_GRAPHICS_KEY_COLOR_OFF;
		} else {
			/* set key color */
			_layer->key_color = buf[i]->keycolor;
		}
#ifdef RT_GRAPHICS_PREMULTI_OFF
		_layer->premultiplied = RT_GRAPHICS_PREMULTI_OFF;
		_layer->alpha_coef    = RT_GRAPHICS_COEFFICIENT_ALPHA1;
#endif
	}

	/* pass */
	rc = CMP_OK;

err_exit:
	DBGLEAVE("%d\n", rc);
	return rc;
}

/******************************************
* ioctl (CMP_IOC_ISLAYEX)
******************************************/
static int  ioc_islayex(struct composer_fh *fh)
{
	/* always return 0 */
	DBGENTER("fh:%p\n", fh);
	DBGLEAVE("%d\n", 0);
	return 0;
}

/******************************************
* ioctl (CMP_IOCG_SUPPORTPIXFMT)
******************************************/
static int  iocg_suppotpixfmt(struct composer_fh *fh, \
	struct cmp_lay_supportpixfmt *arg)
{
	int rc = -EINVAL;

	DBGENTER("fh:%p arg:%p\n", fh, arg);
	printk_dbg2(3, "arg level:%d fmt:%d\n", arg->level, arg->pixfmt);

	/* check argument */
	rc = chk_ioc_supportpixfmt(fh, arg);
	if (rc != 0) {
		/* return by error */
		goto err_exit;
	}

err_exit:
	DBGLEAVE("%d\n", rc);
	return rc;
}

/******************************************
* ioctl (CMP_IOCS_OPEN)
******************************************/
static int  iocs_open(struct composer_fh *fh, \
	struct cmp_lay_open *arg)
{
	int rc;
	int idx;
	struct composer_info   *info;

	DBGENTER("fh:%p arg:%p\n", fh, arg);
	printk_dbg2(3, "arg level:%d id:%d\n", arg->level, arg->bufferid);

	/* check argument */
	rc = chk_ioc_open(fh, arg);
	if (rc != 0) {
		/* return by error */
		goto err_exit;
	}

	/* check already opened.      */
	if (get_index_from_level(arg->level, &idx) != CMP_OK) {
		printk_err2("argument level %d invalid.\n", arg->level);
		rc = -EINVAL;
		goto err_exit;
	}

	info   = &fh->fh_info[idx];
	if (info->buffer != NULL) {
		printk_err2("level 0x%x already opened.\n", arg->level);
		rc = -EBUSY;
		goto err_exit;
	}

	/* bind buffer */
	rc = bind_buffer(fh, idx, arg->bufferid);
	if (0 == rc) {
		static const int fh_status_translate[] = {
			[CMP_OUT_PLANE]  = FH_STATUS_OUTPLANE,
			[CMP_CH1_LAYER]  = FH_STATUS_CH1LAYER,
			[CMP_CH2_LAYER]  = FH_STATUS_CH2LAYER,
			[CMP_CH3_LAYER]  = FH_STATUS_CH3LAYER,
			[CMP_CH4_LAYER]  = FH_STATUS_CH4LAYER };

		printk_dbg2(3, "spinlock\n");
		spin_lock(&irqlock);

		fh->fh_status |= fh_status_translate[arg->level];

		spin_unlock(&irqlock);
	}
err_exit:
	DBGLEAVE("%d\n", rc);
	return rc;
}
/******************************************
* ioctl (CMP_IOCS_CLOSE)
******************************************/
static int  iocs_close(struct composer_fh *fh, \
	struct cmp_lay_close *arg)
{
	int rc;
	int idx;
	struct composer_info   *info;

	DBGENTER("fh:%p arg:%p\n", fh, arg);
	printk_dbg2(3, "arg level:%d\n", arg->level);

	/* check argument */
	rc = chk_ioc_close(fh, arg);
	if (rc != 0) {
		/* return by error */
		goto err_exit;
	}

	/* convert level to internal-index */
	if (get_index_from_level(arg->level, &idx) != CMP_OK) {
		printk_err2("argument level %d invalid.\n", arg->level);
		rc = -EINVAL;
		goto err_exit;
	}

	/* check already closeed.      */
	info   = &fh->fh_info[idx];
	if (info->buffer == NULL) {
		printk_err2("level 0x%x already closed.\n", arg->level);
		rc = 0;  /* no error */
		goto err_exit;
	}

	/* release buffer */
	rc = lock_buffer(fh, idx);
	if (rc < 0) {
		printk_err2("lock_buffer interrupted.\n");
		rc = -EAGAIN;
		goto err_exit;
	}

	rc = release_buffer(fh, arg->level);
	/* unlock_buffer executed in release_buffer */

	if (0 == rc) {
		static const int fh_status_translate[] = {
			[CMP_OUT_PLANE]  = FH_STATUS_OUTPLANE,
			[CMP_CH1_LAYER]  = FH_STATUS_CH1LAYER,
			[CMP_CH2_LAYER]  = FH_STATUS_CH2LAYER,
			[CMP_CH3_LAYER]  = FH_STATUS_CH3LAYER,
			[CMP_CH4_LAYER]  = FH_STATUS_CH4LAYER };

		printk_dbg2(3, "spinlock\n");
		spin_lock(&irqlock);

		fh->fh_status &= ~fh_status_translate[idx];

		spin_unlock(&irqlock);
	}
err_exit:
	DBGLEAVE("%d\n", rc);
	return rc;
}
/******************************************
* ioctl (CMP_IOCS_DISP)
******************************************/
static int  iocs_disp(struct composer_fh *fh, \
	struct cmp_lay_disp *arg)
{
	int rc;
	int idx;
	struct composer_info   *info;

	DBGENTER("fh:%p arg:%p\n", fh, arg);
	printk_dbg2(3, "arg level:%d on:%d\n", arg->level, arg->on);

	/* check argument */
	rc = chk_ioc_disp(fh, arg);
	if (rc != 0) {
		/* return by error */
		goto err_exit;
	}

	/* convert level to internal-index */
	if (get_index_from_level(arg->level, &idx) != CMP_OK) {
		printk_err2("argument level %d invalid.\n", arg->level);
		rc = -EINVAL;
		goto err_exit;
	}

	/* check already opened.  */
	info   = &fh->fh_info[idx];
	printk_dbg2(3, "buffer: %p\n", info->buffer);

	/* lock buffer */
	rc = lock_buffer(fh, idx);
	if (rc < 0) {
		printk_err2("lock_buffer interrupted.\n");
		rc = -EAGAIN;
		goto err_exit;
	}
	if (0 == rc) {
		if (arg->on) {
			/* clera not blend flag. */
			info->id &= ~INFO_ID_FLG_NOTDISP;
		} else {
			/* set not blend flag. */
			info->id |=  INFO_ID_FLG_NOTDISP;
		}
		rc = unlock_buffer(fh, idx);
	}
err_exit:
	DBGLEAVE("%d\n", rc);
	return rc;
}
/******************************************
* ioctl (CMP_IOCS_ALPHA)
******************************************/
static int  iocs_alpha(struct composer_fh *fh, \
	struct cmp_lay_alpha *arg)
{
	int rc;
	int idx;
	struct composer_info   *info;
	struct composer_buffer *buffer;

	DBGENTER("fh:%p arg:%p\n", fh, arg);
	printk_dbg2(3, "arg level:%d alpha:%d\n", arg->level, arg->alpha);

	/* check argument */
	rc = chk_ioc_alpha(fh, arg, 1);
	if (rc != 0) {
		/* return by error */
		goto err_exit;
	}

	/* convert level to internal-index */
	if (get_index_from_level(arg->level, &idx) != CMP_OK) {
		printk_err2("argument level %d invalid.\n", arg->level);
		rc = -EINVAL;
		goto err_exit;
	}

	/* check already opened.  */
	info   = &fh->fh_info[idx];
	printk_dbg2(3, "buffer: %p\n", info->buffer);
	buffer = info->buffer;

	/* lock buffer */
	rc = lock_buffer(fh, idx);
	if (rc < 0) {
		printk_err2("lock_buffer interrupted.\n");
		rc = -EAGAIN;
		goto err_exit;
	}
	if (0 == rc) {
		/* copy parameter */

		/* spinlock is not necessary.    */
		/* printk_dbg2(3, "spinlock");   */
		/* spin_lock(&buffer->buf_lock); */

		buffer->alpha = arg->alpha;

		/* spin_unlock(&buffer->buf_lock); */

		rc = unlock_buffer(fh, idx);
	}
err_exit:
	DBGLEAVE("%d\n", rc);
	return rc;
}
/******************************************
* ioctl (CMP_IOCG_ALPHA)
******************************************/
static int  iocg_alpha(struct composer_fh *fh, \
	struct cmp_lay_alpha *arg)
{
	int rc;
	int idx;
	struct composer_info   *info;
	struct composer_buffer *buffer;

	DBGENTER("fh:%p arg:%p\n", fh, arg);
	printk_dbg2(3, "arg level:%d alpha:-\n", arg->level);

	/* check argument */
	rc = chk_ioc_alpha(fh, arg, 0);
	if (rc != 0) {
		/* return by error */
		goto err_exit;
	}

	/* convert level to internal-index */
	if (get_index_from_level(arg->level, &idx) != CMP_OK) {
		printk_err2("argument level %d invalid.\n", arg->level);
		rc = -EINVAL;
		goto err_exit;
	}

	/* check already opened.  */
	info   = &fh->fh_info[idx];
	printk_dbg2(3, "buffer: %p\n", info->buffer);
	buffer = info->buffer;

	/* not lock buffer, because does not modify it. */
	{
		/* copy parameter */

		/* spinlock is not necessary.    */
		/* printk_dbg2(3, "spinlock");   */
		/* spin_lock(&buffer->buf_lock); */

		arg->alpha = buffer->alpha;

		/* spin_unlock(&buffer->buf_lock); */
	}
err_exit:
	DBGLEAVE("%d\n", rc);
	return rc;
}
/******************************************
* ioctl (CMP_IOCS_KEYCOLOR)
******************************************/
static int  iocs_keycolor(struct composer_fh *fh, \
	struct cmp_lay_keycolor *arg)
{
	int rc;
	int idx;
	struct composer_info   *info;
	struct composer_buffer *buffer;

	DBGENTER("fh:%p arg:%p\n", fh, arg);
	printk_dbg2(3, "arg level:%d keycolor:0x%x\n",
		arg->level, arg->keycolor);

	/* check argument */
	rc = chk_ioc_keycolor(fh, arg, 1);
	if (rc != 0) {
		/* return by error */
		goto err_exit;
	}

	/* convert level to internal-index */
	if (get_index_from_level(arg->level, &idx) != CMP_OK) {
		printk_err2("argument level %d invalid.\n", arg->level);
		rc = -EINVAL;
		goto err_exit;
	}

	/* check already opened.  */
	info   = &fh->fh_info[idx];
	printk_dbg2(3, "buffer: %p\n", info->buffer);
	buffer = info->buffer;

	/* lock buffer */
	rc = lock_buffer(fh, idx);
	if (rc < 0) {
		printk_err2("lock_buffer interrupted.\n");
		rc = -EAGAIN;
		goto err_exit;
	}
	if (0 == rc) {
		/* copy parameter */

		/* spinlock is not necessary.    */
		/* printk_dbg2(3, "spinlock");   */
		/* spin_lock(&buffer->buf_lock); */

		buffer->keycolor = arg->keycolor;

		/* spin_unlock(&buffer->buf_lock); */

		rc = unlock_buffer(fh, idx);
	}
err_exit:
	DBGLEAVE("%d\n", rc);
	return rc;
}
/******************************************
* ioctl (CMP_IOCG_KEYCOLOR)
******************************************/
static int  iocg_keycolor(struct composer_fh *fh, \
	struct cmp_lay_keycolor *arg)
{
	int rc;
	int idx;
	struct composer_info   *info;
	struct composer_buffer *buffer;

	DBGENTER("fh:%p arg:%p\n", fh, arg);
	printk_dbg2(3, "arg level:%d keycolor:-\n", arg->level);

	/* check argument */
	rc = chk_ioc_keycolor(fh, arg, 0);
	if (rc != 0) {
		/* return by error */
		goto err_exit;
	}

	/* convert level to internal-index */
	if (get_index_from_level(arg->level, &idx) != CMP_OK) {
		printk_err2("argument level %d invalid.\n", arg->level);
		rc = -EINVAL;
		goto err_exit;
	}

	/* check already opened.  */
	info   = &fh->fh_info[idx];
	printk_dbg2(3, "buffer: %p\n", info->buffer);
	buffer = info->buffer;

	/* not lock buffer, because does not modify it. */
	{
		/* copy parameter */

		/* spinlock is not necessary.    */
		/* printk_dbg2(3, "spinlock");   */
		/* spin_lock(&buffer->buf_lock); */

		arg->keycolor = buffer->keycolor;

		/* spin_unlock(&buffer->buf_lock); */
	}
err_exit:
	DBGLEAVE("%d\n", rc);
	return rc;
}
/******************************************
* ioctl (CMP_IOCS_LAYSIZE)
******************************************/
static int  iocs_laysize(struct composer_fh *fh, \
	struct cmp_lay_data_size *arg)
{
	int rc;
	int idx;
	struct composer_info   *info;
	struct composer_buffer *buffer;

	DBGENTER("fh:%p arg:%p\n", fh, arg);
	printk_dbg2(3, "arg level:%d x:%d y:%d\n", arg->level, arg->x, arg->y);

	/* check argument */
	rc = chk_ioc_laysize(fh, arg, 1);
	if (rc != 0) {
		/* return by error */
		goto err_exit;
	}

	/* convert level to internal-index */
	if (get_index_from_level(arg->level, &idx) != CMP_OK) {
		printk_err2("argument level %d invalid.\n", arg->level);
		rc = -EINVAL;
		goto err_exit;
	}

	/* check already opened.  */
	info   = &fh->fh_info[idx];
	printk_dbg2(3, "buffer: %p\n", info->buffer);
	buffer = info->buffer;

	/* lock buffer */
	rc = lock_buffer(fh, idx);
	if (rc < 0) {
		printk_err2("lock_buffer interrupted.\n");
		rc = -EAGAIN;
		goto err_exit;
	}
	if (0 == rc) {
		/* copy parameter */

		printk_dbg2(3, "spinlock\n");
		spin_lock(&buffer->buf_lock);

		buffer->data_x = arg->x;
		buffer->data_y = arg->y;
		buffer->buf_id |= BUF_ID_FLG_LAYSIZE;

		spin_unlock(&buffer->buf_lock);

		/* determine stride */
		update_work_linebytes(&fh->grap_data, idx, buffer);
		/* determine crop   */
		update_work_imagesize(&fh->grap_data, idx, buffer);

		rc = unlock_buffer(fh, idx);

		wakeup_allfile(buffer);
	}
err_exit:
	DBGLEAVE("%d\n", rc);
	return rc;
}
/******************************************
* ioctl (CMP_IOCG_LAYSIZE)
******************************************/
static int  iocg_laysize(struct composer_fh *fh, \
	struct cmp_lay_data_size *arg)
{
	int rc;
	int idx;
	struct composer_info   *info;
	struct composer_buffer *buffer;

	DBGENTER("fh:%p arg:%p\n", fh, arg);
	printk_dbg2(3, "arg level:%d x:- y:-\n", arg->level);

	/* check argument */
	rc = chk_ioc_laysize(fh, arg, 0);
	if (rc != 0) {
		/* return by error */
		goto err_exit;
	}

	/* convert level to internal-index */
	if (get_index_from_level(arg->level, &idx) != CMP_OK) {
		printk_err2("argument level %d invalid.\n", arg->level);
		rc = -EINVAL;
		goto err_exit;
	}

	/* check already opened.  */
	info   = &fh->fh_info[idx];
	printk_dbg2(3, "buffer: %p\n", info->buffer);
	buffer = info->buffer;

	/* not lock buffer, because does not modify it. */
	{
		/* copy parameter */

		printk_dbg2(3, "spinlock\n");
		spin_lock(&buffer->buf_lock);

		arg->x = buffer->data_x;
		arg->y = buffer->data_y;

		spin_unlock(&buffer->buf_lock);
	}
err_exit:
	DBGLEAVE("%d\n", rc);
	return rc;
}
/******************************************
* ioctl (CMP_IOCS_PIXFMT)
******************************************/
static int  iocs_pixfmt(struct composer_fh *fh, \
	struct cmp_lay_pixfmt *arg)
{
	int rc;
	int idx;
	struct composer_info   *info;
	struct composer_buffer *buffer;

	DBGENTER("fh:%p arg:%p\n", fh, arg);
	printk_dbg2(3, "arg level:%d fmt:%d color:%d\n",
		arg->level, arg->pixfmt, arg->colorspace);

	/* check argument */
	rc = chk_ioc_pixfmt(fh, arg, 1);
	if (rc != 0) {
		/* return by error */
		goto err_exit;
	}

	/* convert level to internal-index */
	if (get_index_from_level(arg->level, &idx) != CMP_OK) {
		printk_err2("argument level %d invalid.\n", arg->level);
		rc = -EINVAL;
		goto err_exit;
	}

	/* check already opened.  */
	info   = &fh->fh_info[idx];
	printk_dbg2(3, "buffer: %p\n", info->buffer);
	buffer = info->buffer;

	/* lock buffer */
	rc = lock_buffer(fh, idx);
	if (rc < 0) {
		printk_err2("lock_buffer interrupted.\n");
		rc = -EAGAIN;
		goto err_exit;
	}
	if (0 == rc) {
		/* copy parameter */

		printk_dbg2(3, "spinlock\n");
		spin_lock(&buffer->buf_lock);

		buffer->data_fmt  = arg->pixfmt;
		buffer->col_space = arg->colorspace;
		buffer->buf_id |= BUF_ID_FLG_PIXFMT;

		spin_unlock(&buffer->buf_lock);

		/* determine stride */
		update_work_linebytes(&fh->grap_data, idx, buffer);

		rc = unlock_buffer(fh, idx);
	}
err_exit:
	DBGLEAVE("%d\n", rc);
	return rc;
}
/******************************************
* ioctl (CMP_IOCG_PIXFMT)
******************************************/
static int  iocg_pixfmt(struct composer_fh *fh, \
	struct cmp_lay_pixfmt *arg)
{
	int rc;
	int idx;
	struct composer_info   *info;
	struct composer_buffer *buffer;

	DBGENTER("fh:%p arg:%p\n", fh, arg);
	printk_dbg2(3, "arg level:%d fmt:- col:-\n", arg->level);

	/* check argument */
	rc = chk_ioc_pixfmt(fh, arg, 0);
	if (rc != 0) {
		/* return by error */
		goto err_exit;
	}

	/* convert level to internal-index */
	if (get_index_from_level(arg->level, &idx) != CMP_OK) {
		printk_err2("argument level %d invalid.\n", arg->level);
		rc = -EINVAL;
		goto err_exit;
	}

	/* check already opened.  */
	info   = &fh->fh_info[idx];
	printk_dbg2(3, "buffer: %p\n", info->buffer);
	buffer = info->buffer;

	/* not lock buffer, because does not modify it. */
	{
		/* copy parameter */

		printk_dbg2(3, "spinlock\n");
		spin_lock(&buffer->buf_lock);

		arg->pixfmt     = buffer->data_fmt ;
		arg->colorspace = buffer->col_space;

		spin_unlock(&buffer->buf_lock);
	}
err_exit:
	DBGLEAVE("%d\n", rc);
	return rc;
}
/******************************************
* ioctl (CMP_IOCS_CROPSIZE)
******************************************/
static int  iocs_cropsize(struct composer_fh *fh, \
	struct cmp_lay_cropsize *arg)
{
	int rc;
	int idx;
	struct composer_info   *info;
	struct composer_buffer *buffer;

	DBGENTER("fh:%p arg:%p\n", fh, arg);
	printk_dbg2(3, "arg level:%d x:%d y:%d\n", arg->level, arg->x, arg->y);

	/* check argument */
	rc = chk_ioc_cropsize(fh, arg, 1, NULL);
	if (rc != 0) {
		/* return by error */
		goto err_exit;
	}

	/* convert level to internal-index */
	if (get_index_from_level(arg->level, &idx) != CMP_OK) {
		printk_err2("argument level %d invalid.\n", arg->level);
		rc = -EINVAL;
		goto err_exit;
	}

	/* check already opened.  */
	info   = &fh->fh_info[idx];
	printk_dbg2(3, "buffer: %p\n", info->buffer);
	buffer = info->buffer;

	/* lock buffer */
	rc = lock_buffer(fh, idx);
	if (rc < 0) {
		printk_err2("lock_buffer interrupted.\n");
		rc = -EAGAIN;
		goto err_exit;
	}
	if (0 == rc) {
		/* copy parameter */

		printk_dbg2(3, "spinlock\n");
		spin_lock(&buffer->buf_lock);

		buffer->crop_size_x = arg->x;
		buffer->crop_size_y = arg->y;
		buffer->buf_id |= BUF_ID_FLG_CROPSIZE;

		spin_unlock(&buffer->buf_lock);

		/* determine crop   */
		update_work_imagesize(&fh->grap_data, idx, buffer);

		rc = unlock_buffer(fh, idx);
	}
err_exit:
	DBGLEAVE("%d\n", rc);
	return rc;
}
/******************************************
* ioctl (CMP_IOCG_CROPSIZE)
******************************************/
static int  iocg_cropsize(struct composer_fh *fh, \
	struct cmp_lay_cropsize *arg)
{
	int rc;
	int idx;
	struct composer_info   *info;
	struct composer_buffer *buffer;

	DBGENTER("fh:%p arg:%p\n", fh, arg);
	printk_dbg2(3, "arg level:%d x:- y:-\n", arg->level);

	/* check argument */
	rc = chk_ioc_cropsize(fh, arg, 0, NULL);
	if (rc != 0) {
		/* return by error */
		goto err_exit;
	}

	/* convert level to internal-index */
	if (get_index_from_level(arg->level, &idx) != CMP_OK) {
		printk_err2("argument level %d invalid.\n", arg->level);
		rc = -EINVAL;
		goto err_exit;
	}

	/* check already opened.  */
	info   = &fh->fh_info[idx];
	printk_dbg2(3, "buffer: %p\n", info->buffer);
	buffer = info->buffer;

	/* not lock buffer, because does not modify it. */
	{
		/* copy parameter */

		printk_dbg2(3, "spinlock\n");
		spin_lock(&buffer->buf_lock);

		arg->x = buffer->crop_size_x;
		arg->y = buffer->crop_size_y;

		spin_unlock(&buffer->buf_lock);
	}
err_exit:
	DBGLEAVE("%d\n", rc);
	return rc;
}
/******************************************
* ioctl (CMP_IOCG_CROPPOS)
******************************************/
static int  iocs_croppos(struct composer_fh *fh, \
	struct cmp_lay_croppos *arg)
{
	int rc;
	int idx;
	struct composer_info   *info;
	struct composer_buffer *buffer;

	DBGENTER("fh:%p arg:%p\n", fh, arg);
	printk_dbg2(3, "arg level:%d x:%d y:%d\n", arg->level, arg->x, arg->y);

	/* check argument */
	rc = chk_ioc_croppos(fh, arg, 1, NULL);
	if (rc != 0) {
		/* return by error */
		goto err_exit;
	}

	/* convert level to internal-index */
	if (get_index_from_level(arg->level, &idx) != CMP_OK) {
		printk_err2("argument level %d invalid.\n", arg->level);
		rc = -EINVAL;
		goto err_exit;
	}

	/* check already opened.  */
	info   = &fh->fh_info[idx];
	printk_dbg2(3, "buffer: %p\n", info->buffer);
	buffer = info->buffer;

	/* lock buffer */
	rc = lock_buffer(fh, idx);
	if (rc < 0) {
		printk_err2("lock_buffer interrupted.\n");
		rc = -EAGAIN;
		goto err_exit;
	}
	if (0 == rc) {
		/* copy parameter */

		printk_dbg2(3, "spinlock\n");
		spin_lock(&buffer->buf_lock);

		buffer->crop_pos_x = arg->x;
		buffer->crop_pos_y = arg->y;
		buffer->buf_id |= BUF_ID_FLG_CROPPOS;

		spin_unlock(&buffer->buf_lock);

		/* determine crop   */
		update_work_imagesize(&fh->grap_data, idx, buffer);

		rc = unlock_buffer(fh, idx);
	}
err_exit:
	DBGLEAVE("%d\n", rc);
	return rc;
}
/******************************************
* ioctl (CMP_IOCG_CROPPOS)
******************************************/
static int  iocg_croppos(struct composer_fh *fh, \
	struct cmp_lay_croppos *arg)
{
	int rc;
	int idx;
	struct composer_info   *info;
	struct composer_buffer *buffer;

	DBGENTER("fh:%p arg:%p\n", fh, arg);
	printk_dbg2(3, "arg level:%d x:- y:-\n", arg->level);

	/* check argument */
	rc = chk_ioc_croppos(fh, arg, 0, NULL);
	if (rc != 0) {
		/* return by error */
		goto err_exit;
	}

	/* convert level to internal-index */
	if (get_index_from_level(arg->level, &idx) != CMP_OK) {
		printk_err2("argument level %d invalid.\n", arg->level);
		rc = -EINVAL;
		goto err_exit;
	}

	/* check already opened.  */
	info   = &fh->fh_info[idx];
	printk_dbg2(3, "buffer: %p\n", info->buffer);
	buffer = info->buffer;

	/* not lock buffer, because does not modify it. */
	{
		/* copy parameter */

		printk_dbg2(3, "spinlock\n");
		spin_lock(&buffer->buf_lock);

		arg->x = buffer->crop_pos_x;
		arg->y = buffer->crop_pos_y;

		spin_unlock(&buffer->buf_lock);
	}
err_exit:
	DBGLEAVE("%d\n", rc);
	return rc;
}
/******************************************
* ioctl (CMP_IOCS_POS)
******************************************/
static int  iocs_pos(struct composer_fh *fh, \
	struct cmp_lay_pos *arg)
{
	int rc;
	int idx;
	struct composer_info   *info;
	struct composer_buffer *buffer;

	DBGENTER("fh:%p arg:%p\n", fh, arg);
	printk_dbg2(3, "arg level:%d x:%d y:%d\n", arg->level, arg->x, arg->y);

	/* check argument */
	rc = chk_ioc_pos(fh, arg, 1);
	if (rc != 0) {
		/* return by error */
		goto err_exit;
	}

	/* convert level to internal-index */
	if (get_index_from_level(arg->level, &idx) != CMP_OK) {
		printk_err2("argument level %d invalid.\n", arg->level);
		rc = -EINVAL;
		goto err_exit;
	}

	/* check already opened.  */
	info   = &fh->fh_info[idx];
	printk_dbg2(3, "buffer: %p\n", info->buffer);
	buffer = info->buffer;

	/* lock buffer */
	rc = lock_buffer(fh, idx);
	if (rc < 0) {
		printk_err2("lock_buffer interrupted.\n");
		rc = -EAGAIN;
		goto err_exit;
	}
	if (0 == rc) {
		/* copy parameter */

		printk_dbg2(3, "spinlock\n");
		spin_lock(&buffer->buf_lock);

		buffer->pos_x = arg->x;
		buffer->pos_y = arg->y;
		buffer->buf_id |= BUF_ID_FLG_POS;

		spin_unlock(&buffer->buf_lock);

		rc = unlock_buffer(fh, idx);
	}
err_exit:
	DBGLEAVE("%d\n", rc);
	return rc;
}
/******************************************
* ioctl (CMP_IOCG_POS)
******************************************/
static int  iocg_pos(struct composer_fh *fh, \
	struct cmp_lay_pos *arg)
{
	int rc;
	int idx;
	struct composer_info   *info;
	struct composer_buffer *buffer;

	DBGENTER("fh:%p arg:%p\n", fh, arg);
	printk_dbg2(3, "arg level:%d x:- y:-\n", arg->level);

	/* check argument */
	rc = chk_ioc_pos(fh, arg, 0);
	if (rc != 0) {
		/* return by error */
		goto err_exit;
	}

	/* convert level to internal-index */
	if (get_index_from_level(arg->level, &idx) != CMP_OK) {
		printk_err2("argument level %d invalid.\n", arg->level);
		rc = -EINVAL;
		goto err_exit;
	}

	/* check already opened.  */
	info   = &fh->fh_info[idx];
	printk_dbg2(3, "buffer: %p\n", info->buffer);
	buffer = info->buffer;

	/* not lock buffer, because does not modify it. */
	{
		/* copy parameter */

		printk_dbg2(3, "spinlock\n");
		spin_lock(&buffer->buf_lock);

		arg->x = buffer->pos_x;
		arg->y = buffer->pos_y;

		spin_unlock(&buffer->buf_lock);
	}
err_exit:
	DBGLEAVE("%d\n", rc);
	return rc;
}
/******************************************
* ioctl (CMP_IOCS_SIZE)
******************************************/
static int  iocs_size(struct composer_fh *fh, \
	struct cmp_lay_compose_size *arg)
{
	int rc;
	int idx;
	struct composer_info   *info;
	struct composer_buffer *buffer;

	DBGENTER("fh:%p arg:%p\n", fh, arg);
	printk_dbg2(3, "arg level:%d x:%d y:%d\n", arg->level, arg->x, arg->y);

	/* check argument */
	rc = chk_ioc_size(fh, arg, 1);
	if (rc != 0) {
		/* return by error */
		goto err_exit;
	}

	/* convert level to internal-index */
	if (get_index_from_level(arg->level, &idx) != CMP_OK) {
		printk_err2("argument level %d invalid.\n", arg->level);
		rc = -EINVAL;
		goto err_exit;
	}

	/* check already opened.  */
	info   = &fh->fh_info[idx];
	printk_dbg2(3, "buffer: %p\n", info->buffer);
	buffer = info->buffer;

	/* lock buffer */
	rc = lock_buffer(fh, idx);
	if (rc < 0) {
		printk_err2("lock_buffer interrupted.\n");
		rc = -EAGAIN;
		goto err_exit;
	}
	if (0 == rc) {
		/* copy parameter */

		printk_dbg2(3, "spinlock\n");
		spin_lock(&buffer->buf_lock);

		buffer->size_x = arg->x;
		buffer->size_y = arg->y;
		buffer->buf_id |= BUF_ID_FLG_SIZE;

		spin_unlock(&buffer->buf_lock);

		rc = unlock_buffer(fh, idx);
	}
err_exit:
	DBGLEAVE("%d\n", rc);
	return rc;
}
/******************************************
* ioctl (CMP_IOCG_SIZE)
******************************************/
static int  iocg_size(struct composer_fh *fh, \
	struct cmp_lay_compose_size *arg)
{
	int rc;
	int idx;
	struct composer_info   *info;
	struct composer_buffer *buffer;

	DBGENTER("fh:%p arg:%p\n", fh, arg);
	printk_dbg2(3, "arg level:%d x:- y:-\n", arg->level);

	/* check argument */
	rc = chk_ioc_size(fh, arg, 0);
	if (rc != 0) {
		/* return by error */
		goto err_exit;
	}

	/* convert level to internal-index */
	if (get_index_from_level(arg->level, &idx) != CMP_OK) {
		printk_err2("argument level %d invalid.\n", arg->level);
		rc = -EINVAL;
		goto err_exit;
	}

	/* check already opened.  */
	info   = &fh->fh_info[idx];
	printk_dbg2(3, "buffer: %p\n", info->buffer);
	buffer = info->buffer;

	/* not lock buffer, because does not modify it. */
	{
		/* copy parameter */

		printk_dbg2(3, "spinlock\n");
		spin_lock(&buffer->buf_lock);

		arg->x = buffer->size_x;
		arg->y = buffer->size_y;

		spin_unlock(&buffer->buf_lock);
	}
err_exit:
	DBGLEAVE("%d\n", rc);
	return rc;
}
/******************************************
* ioctl (CMP_IOCS_VIEWLAY)
******************************************/
static int  iocs_viewlay(struct composer_fh *fh, \
	struct cmp_viewlay *arg)
{
	int rc, idx;
	struct composer_info   *info;
	struct composer_buffer *buffer;

	DBGENTER("fh:%p arg:%p\n", fh, arg);
	printk_dbg2(3, "arg level:%d data_x:%d data_y:%d " \
		"croppos_x:%d croppos_y:%d " \
		"cropsize_x:%d cropsize_y:%d " \
		"pos_x:%d pos_y:%d " \
		"compose_x:%d compose_y:%d\n" \
		, arg->level, arg->data_x, arg->data_y
		, arg->croppos_x, arg->croppos_y
		, arg->cropsize_x, arg->cropsize_y
		, arg->pos_x, arg->pos_y, arg->compose_x, arg->compose_y);

	/* check argument */
	rc = chk_ioc_viewlay(fh, arg, -1);
	if (rc != 0) {
		/* return by error */
		goto err_exit;
	}

	/* convert level to internal-index */
	if (get_index_from_level(arg->level, &idx) != CMP_OK) {
		printk_err2("argument level %d invalid.\n", arg->level);
		rc = -EINVAL;
		goto err_exit;
	}

	/* check already opened.  */
	info   = &fh->fh_info[idx];
	printk_dbg2(3, "buffer: %p\n", info->buffer);
	buffer = info->buffer;

	/* check argument and update it. if necessary. */
	rc = chk_ioc_viewlay(fh, arg, idx);
	if (rc != 0) {
		/* return by error */
		goto err_exit;
	}

	/* lock buffer */
	rc = lock_buffer(fh, idx);
	if (rc < 0) {
		printk_err2("lock_buffer interrupted.\n");
		rc = -EAGAIN;
		goto err_exit;
	}
	if (0 == rc) {
		/* copy parameter if necessary */
		unsigned long mask = 0x80000000;
		int           dst_not_support = 0;

		if (arg->level == CMP_OUT_PLANE) {
			/* destination parameter not available */
			dst_not_support = 1;
		}

		printk_dbg2(3, "spinlock\n");
		spin_lock(&buffer->buf_lock);

		if ((arg->data_x & mask) == 0) {
			buffer->data_x = arg->data_x;
			buffer->buf_id |= BUF_ID_FLG_LAYSIZE1;
		}
		if ((arg->data_y & mask) == 0) {
			buffer->data_y = arg->data_y;
			buffer->buf_id |= BUF_ID_FLG_LAYSIZE2;
		}
		if ((arg->cropsize_x & mask) == 0) {
			buffer->crop_size_x = arg->cropsize_x;
			buffer->buf_id |= BUF_ID_FLG_CROPSIZE1;
		}
		if ((arg->cropsize_y & mask) == 0) {
			buffer->crop_size_y = arg->cropsize_y;
			buffer->buf_id |= BUF_ID_FLG_CROPSIZE2;
		}
		if ((arg->croppos_x & mask) == 0) {
			buffer->crop_pos_x = arg->croppos_x;
			buffer->buf_id |= BUF_ID_FLG_CROPPOS1;
		}
		if ((arg->croppos_y & mask) == 0) {
			buffer->crop_pos_y = arg->croppos_y;
			buffer->buf_id |= BUF_ID_FLG_CROPPOS2;
		}
		if (dst_not_support == 0) {
			if ((arg->pos_x & mask) == 0) {
				buffer->pos_x = arg->pos_x;
				buffer->buf_id |= BUF_ID_FLG_POS1;
			}
			if ((arg->pos_y & mask) == 0) {
				buffer->pos_y = arg->pos_y;
				buffer->buf_id |= BUF_ID_FLG_POS2;
			}
			if ((arg->compose_x & mask) == 0) {
				buffer->size_x = arg->compose_x;
				buffer->buf_id |= BUF_ID_FLG_SIZE1;
			}
			if ((arg->compose_y & mask) == 0) {
				buffer->size_y = arg->compose_y;
				buffer->buf_id |= BUF_ID_FLG_SIZE2;
			}
		}

		spin_unlock(&buffer->buf_lock);

		/* determine stride */
		update_work_linebytes(&fh->grap_data, idx, buffer);
		/* determine crop   */
		update_work_imagesize(&fh->grap_data, idx, buffer);

		rc = unlock_buffer(fh, idx);
	}
err_exit:
	DBGLEAVE("%d\n", rc);
	return rc;
}
/******************************************
* ioctl (CMP_IOCS_BACKCOLOR)
******************************************/
static int  iocs_backcolor(struct composer_fh *fh, \
	struct cmp_lay_backcolor *arg)
{
	int rc;
	int idx;
	struct composer_info   *info;
	struct composer_buffer *buffer;

	DBGENTER("fh:%p arg:%p\n", fh, arg);
	printk_dbg2(3, "arg level:%d col:0x%x\n", arg->level, arg->backcolor);

	/* check argument */
	rc = chk_ioc_backcolor(fh, arg, 1);
	if (rc != 0) {
		/* return by error */
		goto err_exit;
	}

	/* convert level to internal-index */
	if (get_index_from_level(arg->level, &idx) != CMP_OK) {
		printk_err2("argument level %d invalid.\n", arg->level);
		rc = -EINVAL;
		goto err_exit;
	}

	/* check already opened.  */
	info   = &fh->fh_info[idx];
	printk_dbg2(3, "buffer: %p\n", info->buffer);
	buffer = info->buffer;

	/* lock buffer */
	rc = lock_buffer(fh, idx);
	if (rc < 0) {
		printk_err2("lock_buffer interrupted.\n");
		rc = -EAGAIN;
		goto err_exit;
	}
	if (0 == rc) {
		/* copy parameter */

		/* spinlock is not necessary.    */
		/* printk_dbg2(3, "spinlock");   */
		/* spin_lock(&buffer->buf_lock); */

		buffer->backcolor = arg->backcolor;

		/* spin_unlock(&buffer->buf_lock); */

		rc = unlock_buffer(fh, idx);
	}
err_exit:
	DBGLEAVE("%d\n", rc);
	return rc;
}
/******************************************
* ioctl (CMP_IOCG_BACKCOLOR)
******************************************/
static int  iocg_backcolor(struct composer_fh *fh, \
	struct cmp_lay_backcolor *arg)
{
	int rc;
	int idx;
	struct composer_info   *info;
	struct composer_buffer *buffer;

	DBGENTER("fh:%p arg:%p\n", fh, arg);
	printk_dbg2(3, "arg level:%d col:-\n", arg->level);

	/* check argument */
	rc = chk_ioc_backcolor(fh, arg, 0);
	if (rc != 0) {
		/* return by error */
		goto err_exit;
	}

	/* convert level to internal-index */
	if (get_index_from_level(arg->level, &idx) != CMP_OK) {
		printk_err2("argument level %d invalid.\n", arg->level);
		rc = -EINVAL;
		goto err_exit;
	}

	/* check already opened.  */
	info   = &fh->fh_info[idx];
	printk_dbg2(3, "buffer: %p\n", info->buffer);
	buffer = info->buffer;

	/* not lock buffer, because does not modify it. */
	{
		/* copy parameter */

		/* spinlock is not necessary.    */
		/* printk_dbg2(3, "spinlock");   */
		/* spin_lock(&buffer->buf_lock); */

		arg->backcolor = buffer->backcolor;

		/* spin_unlock(&buffer->buf_lock); */
	}
err_exit:
	DBGLEAVE("%d\n", rc);
	return rc;
}
/******************************************
* ioctl (CMP_IOCS_BACKCOLOR)
******************************************/
static int  iocs_layaddr(struct composer_fh *fh, \
	struct cmp_layaddr *arg)
{
	int rc;
	int idx;
	struct composer_info   *info;
	struct composer_buffer *buffer;

	DBGENTER("fh:%p arg:%p\n", fh, arg);
	printk_dbg2(3, "arg level:%d " \
			"addr:%p size:0x%x id:%d offset:%d " \
			"addr2:%p size2:0x%x id2:%d offset2:%d " \
			"addr3:%p size3:0x%x id3:%d offset3:%d\n", \
			arg->level, arg->addr, arg->datasize, \
			arg->app_id, arg->offset, \
			arg->addr_c0, arg->datasize_c0, \
			arg->app_id_c0, arg->offset_c0, \
			arg->addr_c1, arg->datasize_c1, \
			arg->app_id_c1, arg->offset_c1);

	/* check argument */
	rc = chk_ioc_layaddr(fh, arg, 1);
	if (rc != 0) {
		/* return by error */
		goto err_exit;
	}

	/* convert level to internal-index */
	if (get_index_from_level(arg->level, &idx) != CMP_OK) {
		printk_err2("argument level %d invalid.\n", arg->level);
		rc = -EINVAL;
		goto err_exit;
	}

	/* check already opened.  */
	info   = &fh->fh_info[idx];
	printk_dbg2(3, "buffer: %p\n", info->buffer);
	buffer = info->buffer;

	/* lock buffer */
	rc = lock_buffer(fh, idx);
	if (rc < 0) {
		printk_err2("lock_buffer interrupted.\n");
		rc = -EAGAIN;
		goto err_exit;
	}
	if (0 == rc) {
		/* copy parameter */

		printk_dbg2(3, "spinlock\n");
		spin_lock(&buffer->buf_lock);

		buffer->addr     = arg->addr;
		buffer->app_id   = arg->app_id;
		buffer->offset   = arg->offset;
		buffer->app_addr = NULL;
		buffer->app_size = arg->datasize;

		buffer->addr_c0     = arg->addr_c0;
		buffer->app_id_c0   = arg->app_id_c0;
		buffer->offset_c0   = arg->offset_c0;
		buffer->app_addr_c0 = NULL;
		buffer->app_size_c0 = arg->datasize_c0;

		buffer->addr_c1     = arg->addr_c1;
		buffer->app_id_c1   = arg->app_id_c1;
		buffer->offset_c1   = arg->offset_c1;
		buffer->app_addr_c1 = NULL;
		buffer->app_size_c1 = arg->datasize_c1;

		buffer->buf_id |= BUF_ID_SET_BUFFER;

		spin_unlock(&buffer->buf_lock);

		rc = update_work_appshare(info, buffer);

		if (rc == 1) {
			printk_dbg2(3, "app share not mapped.");
			/* app-share will be mapped CMP_IOCS_START. */

			info->id |= INFO_ID_FLG_NEEDAPPSET;
			rc = 0;
		} else if (rc != CMP_OK) {
			rc = -EINVAL;
			printk_err2("update_work_appshare return error.\n");

			printk_dbg2(3, "spinlock\n");
			spin_lock(&buffer->buf_lock);

			buffer->buf_id &= ~BUF_ID_SET_BUFFER;

			spin_unlock(&buffer->buf_lock);
		} else {
			/* update status to change address */
			allfile_status_set(buffer, FH_STATUS_POLLADDR);
		}

		unlock_buffer(fh, idx);
	}
err_exit:
	DBGLEAVE("%d\n", rc);
	return rc;
}
/******************************************
* ioctl (CMP_IOCG_BACKCOLOR)
******************************************/
static int  iocg_layaddr(struct composer_fh *fh, \
	struct cmp_layaddr *arg)
{
	int rc;
	int idx;
	struct composer_info   *info;
	struct composer_buffer *buffer;

	DBGENTER("fh:%p arg:%p\n", fh, arg);
	printk_dbg2(3, "arg level:%d\n", arg->level);

	/* check argument */
	rc = chk_ioc_layaddr(fh, arg, 0);
	if (rc != 0) {
		/* return by error */
		goto err_exit;
	}

	/* convert level to internal-index */
	if (get_index_from_level(arg->level, &idx) != CMP_OK) {
		printk_err2("argument level %d invalid.\n", arg->level);
		rc = -EINVAL;
		goto err_exit;
	}

	/* check already opened.  */
	info   = &fh->fh_info[idx];
	printk_dbg2(3, "buffer: %p\n", info->buffer);
	buffer = info->buffer;

	/* not lock buffer, because does not modify it. */
	{
		/* copy parameter */

		printk_dbg2(3, "spinlock\n");
		spin_lock(&buffer->buf_lock);

		arg->addr     = buffer->addr;
		arg->app_id   = buffer->app_id;
		arg->offset   = buffer->offset;
		arg->datasize = buffer->app_size;

		arg->addr_c0     = buffer->addr_c0;
		arg->app_id_c0   = buffer->app_id_c0;
		arg->offset_c0   = buffer->offset_c0;
		arg->datasize_c0 = buffer->app_size_c0;

		arg->addr_c1     = buffer->addr_c1;
		arg->app_id_c1   = buffer->app_id_c1;
		arg->offset_c1   = buffer->offset_c1;
		arg->datasize_c1 = buffer->app_size_c1;

		spin_unlock(&buffer->buf_lock);
	}
err_exit:
	DBGLEAVE("%d\n", rc);
	return rc;
}
static int  ioc_start(struct composer_fh *fh)
{
	int i, rc;

	DBGENTER("fh:%p\n", fh);

	/* check argument */
	rc = chk_ioc_start(fh);
	if (rc != 0) {
		/* return by error */
		goto err_exit;
	}

	if (fh->fh_status & FH_STATUS_BLENDING) {
		printk_err2("already start image blending.\n");
		rc = -EBUSY;
		goto err_exit;
	}

	/* to finish previous work. */
	localworkqueue_flush(workqueue, &fh->fh_wqtask);

	/* check app-shrea need to create. */
	for (i = 0; i < 5; i++) {
		struct composer_info   *info   = &fh->fh_info[i];
		struct composer_buffer *buffer = info->buffer;

		if ((info->id & INFO_ID_FLG_NEEDAPPSET) == 0) {
			/* nothing to do */
			continue;
		}

		/* acquire buffer lock */
		rc = lock_buffer(fh, i);
		if (rc < 0) {
			printk_err2("lock_buffer interrupted.\n");
			rc = -EAGAIN;
			goto err_exit;
		}

		if (update_work_appshare(info, buffer)) {
			printk_err2("app share can not open.");

			/* release buffer lock */
			unlock_buffer(fh, i);
			rc = -EINVAL;
			goto err_exit;
		} else {
			info->id &= ~INFO_ID_FLG_NEEDAPPSET;

			/* release buffer lock */
			unlock_buffer(fh, i);
		}
	}

	rc = update_grap_arguments(fh);
	if (rc != 0) {
		printk_err2("can not convert grap argument.\n");
		rc = -EINVAL;
		goto err_exit;
	}
	/* all buffer reserved to this fh */
	rc = setbusy(fh);
	if (rc != 0) {
		printk_err2("can not get buffer lock.\n");
		rc = -EAGAIN;
		goto err_exit;
	}


	printk_dbg2(3, "spinlock\n");
	spin_lock(&irqlock);

	fh->fh_status &= ~FH_STATUS_BLENDERR;
	fh->fh_status |= FH_STATUS_BLENDING;

	spin_unlock(&irqlock);

	rc = localworkqueue_queue(workqueue, &fh->fh_wqtask);
	if (rc) {
		/* to finish previous work. */

		localworkqueue_flush(workqueue, &fh->fh_wqtask);

		if (fh->fh_status & FH_STATUS_BLENDING) {
			/* report bug */
			printk_err("fh_status of blending should not set.\n");
		}

		setidle(fh);

		rc = 0;
		if (fh->fh_status & FH_STATUS_BLENDERR) {
			printk_err2("compose error found.\n");
			rc = -EINVAL;
		}
	} else {
		fh->fh_status &= ~FH_STATUS_BLENDING;
		printk_err2("unexpectly queue_work not successed..\n");
		rc = -EAGAIN;
		setidle(fh);
	}
err_exit:
	DBGLEAVE("%d\n", rc);
	return rc;
}

static int  ioc_issuspend(struct composer_fh *fh)
{
	int rc = -EBUSY;
	DBGENTER("fh:%p\n", fh);

	if (graphic_handle == NULL) {
		printk_dbg2(3, "not create graphic handle\n");
		/* report log */
	} else {
		printk_dbg2(3, "create graphic handle\n");
		/* report log */
	}

#ifdef CONFIG_HAS_EARLYSUSPEND
	if (in_early_suspend) {
		printk_dbg2(3, "now suspend state\n");
		rc = 0;
	}
#endif

	DBGLEAVE("%d\n", rc);
	return rc;
}

#ifdef CONFIG_MISC_R_MOBILE_COMPOSER_REQUEST_QUEUE
static int  wait_condition_for_ioc_waitcomp(void)
{
	int i;
	int rc = 0;
	for (i = 0; i < MAX_KERNELREQ; i++) {
		struct composer_rh *rh = &kernel_request[i];
		if (rh->active) {
			rc = 1;
			break;
		}
	}
	return (rc == 0);
}
#endif
static int  ioc_waitcomp(struct composer_fh *fh)
{
	int rc = 0;
#ifdef CONFIG_MISC_R_MOBILE_COMPOSER_REQUEST_QUEUE
#if _LOG_DBG > 1
	unsigned long jiffies_s = jiffies;
	int prev_state[MAX_KERNELREQ];
#endif /* _LOG_DBG > 1 */
#endif
	DBGENTER("fh:%p\n", fh);

#ifdef CONFIG_MISC_R_MOBILE_COMPOSER_REQUEST_QUEUE
#if _LOG_DBG > 1
	prev_state[0] = kernel_request[0].active;
	prev_state[1] = kernel_request[1].active;
	prev_state[2] = kernel_request[2].active;
	prev_state[3] = kernel_request[3].active;
#endif
	rc = wait_event_timeout(kernel_waitqueue_comp,
		wait_condition_for_ioc_waitcomp(), msecs_to_jiffies(100));

#if _LOG_DBG > 1
	printk_dbg2(3, "%d %d %d %d\n",
		prev_state[0],
		prev_state[1],
		prev_state[2],
		prev_state[3]);
	printk_dbg2(3, "%d %d %d %d\n",
		kernel_request[0].active,
		kernel_request[1].active,
		kernel_request[2].active,
		kernel_request[3].active);
#endif
	if (rc == 0) {
		printk_err("fail to wait task complete.\n");
		rc = -EBUSY;
	} else {
		/* wait success before timeout */
		rc = 0;
	}
#if _LOG_DBG > 1
	printk_dbg2(3, "actual waiting %d msec",
		jiffies_to_msecs(jiffies - jiffies_s));
#endif /* _LOG_DBG > 1 */
#endif

	DBGLEAVE("%d\n", rc);
	return rc;
}
static int  ioc_waitdraw(struct composer_fh *fh, int *mode)
{
	int rc = 0;
	DBGENTER("fh:%p\n", fh);

#ifdef CONFIG_MISC_R_MOBILE_COMPOSER_REQUEST_QUEUE
#if SH_MOBILE_COMPOSER_WAIT_DRAWEND
	if (*mode == 0) {
		/* clear flag */
		overlay_draw_complete = 0;
	} else if (*mode == 1) {
		/* wait set flag */
		rc = wait_event_timeout(kernel_waitqueue_comp,
			overlay_draw_complete != 0, msecs_to_jiffies(200));

		if (rc == 0) {
			printk_err("fail to wait draw complete.\n");
			rc = -EBUSY;
		} else {
			/* wait success before timeout */
			overlay_draw_complete = 0;
			rc = 0;
		}
	} else {
		printk_err("unknown mode %d.\n", *mode);
		rc = -EINVAL;
	}
#endif /* SH_MOBILE_COMPOSER_WAIT_DRAWEND */
#endif

	DBGLEAVE("%d\n", rc);
	return rc;
}

static int  chk_ioc_supportpixfmt(struct composer_fh *fh, \
	struct cmp_lay_supportpixfmt *arg)
{
	int rc = -EINVAL;
	int flag;

	switch (arg->level) {
	case CMP_OUT_PLANE:
		flag = 0;
		break;
	case CMP_CH1_LAYER:
	case CMP_CH2_LAYER:
	case CMP_CH3_LAYER:
	case CMP_CH4_LAYER:
		flag = 1;
		break;
	default:
		printk_err2("argument level 0x%x invalid.\n", arg->level);
		goto err_exit;
		break;
	}
	/* When the output format and an input format changed,*/
	/* maintain flag parameters.                          */

	switch (arg->pixfmt) {
	case CMPPIXFMT_ARGB8888:
	case CMPPIXFMT_YUV420SP:
	case CMPPIXFMT_YUV422SP:
		break;
	case CMPPIXFMT_RGB888:
	case CMPPIXFMT_RGB565:
	case CMPPIXFMT_YUV420PL:
		/* do not need check flag */
		break;
	default:
		printk_err2("argument pixfmt 0x%x invalid.\n", arg->pixfmt);
		goto err_exit;
		break;
	}

	/* pass */
	rc = 0;
err_exit:
	return rc;
}
static int  chk_ioc_open(struct composer_fh *fh, \
	struct cmp_lay_open *arg)
{
	int rc = -EINVAL;
	switch (arg->level) {
	case CMP_OUT_PLANE:
	case CMP_CH1_LAYER:
	case CMP_CH2_LAYER:
	case CMP_CH3_LAYER:
	case CMP_CH4_LAYER:
		break;
	default:
		printk_err2("argument level 0x%x invalid.\n", arg->level);
		goto err_exit;
		break;
	}

	if (arg->bufferid < 0 || arg->bufferid >= MAX_BUFFER) {
		printk_err2("argument level 0x%x invalid.\n", arg->level);
		goto err_exit;
	}

	/* pass */
	rc = 0;
err_exit:
	return rc;
}
static int  chk_ioc_close(struct composer_fh *fh, \
	struct cmp_lay_close *arg)
{
	int rc = -EINVAL;
	switch (arg->level) {
	case CMP_OUT_PLANE:
	case CMP_CH1_LAYER:
	case CMP_CH2_LAYER:
	case CMP_CH3_LAYER:
	case CMP_CH4_LAYER:
		break;
	default:
		printk_err2("argument level 0x%x invalid.\n", arg->level);
		goto err_exit;
		break;
	}

	/* pass */
	rc = 0;
err_exit:
	return rc;
}
static int  chk_ioc_disp(struct composer_fh *fh, \
	struct cmp_lay_disp *arg)
{
	int rc = -EINVAL;
	switch (arg->level) {
	case CMP_CH1_LAYER:
	case CMP_CH2_LAYER:
	case CMP_CH3_LAYER:
	case CMP_CH4_LAYER:
		break;
	default:
		printk_err2("argument level 0x%x invalid.\n", arg->level);
		goto err_exit;
		break;
	}

	if (get_composer_info(fh, arg->level) == NULL) {
		printk_err2("argument level 0x%x not open.\n", arg->level);
		goto err_exit;
	}

	/* arg->on is always valid. */

	/* pass */
	rc = 0;
err_exit:
	return rc;
}
static int  chk_ioc_alpha(struct composer_fh *fh, \
	struct cmp_lay_alpha *arg, int set_flag)
{
	int rc = -EINVAL;
	switch (arg->level) {
	case CMP_CH1_LAYER:
	case CMP_CH2_LAYER:
	case CMP_CH3_LAYER:
	case CMP_CH4_LAYER:
		break;
	default:
		printk_err2("argument level 0x%x invalid.\n", arg->level);
		goto err_exit;
		break;
	}

	if (get_composer_info(fh, arg->level) == NULL) {
		printk_err2("argument level 0x%x not open.\n", arg->level);
		goto err_exit;
	}

	/* arg->alpha is always valid. */

	/* pass */
	rc = 0;
err_exit:
	return rc;
}
static int  chk_ioc_keycolor(struct composer_fh *fh, \
	struct cmp_lay_keycolor *arg, int set_flag)
{
	int rc = -EINVAL;
	struct composer_buffer *buf;
	struct composer_info   *info;

	switch (arg->level) {
	case CMP_CH1_LAYER:
	case CMP_CH2_LAYER:
	case CMP_CH3_LAYER:
	case CMP_CH4_LAYER:
		break;
	default:
		printk_err2("argument level 0x%x invalid.\n", arg->level);
		goto err_exit;
		break;
	}

	info = get_composer_info(fh, arg->level);
	if (info == NULL) {
		printk_err2("argument level 0x%x not open.\n", arg->level);
		goto err_exit;
	}

	if (set_flag == 0) {
		/* skip parameter check of keycolor */
		goto pass_exit;
	}
	buf = info->buffer;
	if ((buf->buf_id & BUF_ID_FLG_PIXFMT) == 0) {
		/* pixel format not specified */
		printk_dbg2(2, "pixel format not specified."
			" ignore parameter check of keycolor.\n");
	} else if (arg->keycolor != CMPKEYCOLOR_OFF) {
		if (buf->data_fmt == CMPPIXFMT_RGB565) {
			if (arg->keycolor > 0xffff) {
				printk_err2("argument keycolor 0x%x invalid"
					" for RGB565.\n", arg->keycolor);
				goto err_exit;
			}
		} else if (buf->data_fmt == CMPPIXFMT_RGB888) {
			if (arg->keycolor > 0xffffff) {
				printk_err2("argument keycolor 0x%x invalid"
					" for RGB888.\n", arg->keycolor);
				goto err_exit;
			}
		} else if (buf->data_fmt == CMPPIXFMT_YUV420SP ||
			buf->data_fmt == CMPPIXFMT_YUV422SP ||
			buf->data_fmt == CMPPIXFMT_YUV420PL) {
			if (arg->keycolor > 0xff) {
				printk_err2("argument keycolor 0x%x invalid"
					" for YUV.\n", arg->keycolor);
				goto err_exit;
			}
		} else {
			/* ignore parameter */
			/* nothing to do    */
		}
	}
pass_exit:
	/* pass */
	rc = 0;
err_exit:
	return rc;
}
static int  chk_ioc_laysize(struct composer_fh *fh, \
	struct cmp_lay_data_size *arg, int set_flag)
{
	int rc = -EINVAL;
	struct composer_buffer *buf;
	struct composer_info   *info;

	switch (arg->level) {
	case CMP_OUT_PLANE:
	case CMP_CH1_LAYER:
	case CMP_CH2_LAYER:
	case CMP_CH3_LAYER:
	case CMP_CH4_LAYER:
		break;
	default:
		printk_err2("argument level 0x%x invalid.\n", arg->level);
		goto err_exit;
		break;
	}

	info = get_composer_info(fh, arg->level);
	if (info == NULL) {
		printk_err2("argument level 0x%x not open.\n", arg->level);
		goto err_exit;
	}

	if (set_flag == 0) {
		/* skip parameter check of laysize */
		goto pass_exit;
	}
	if (arg->x < 1 || arg->x > MAX_WIDTH) {
		printk_err2("argument x invalid. 1-%d\n", MAX_WIDTH);
		goto err_exit;
	}
	if (arg->y < 1 || arg->y > MAX_HEIGHT) {
		printk_err2("argument y invalid. 1-%d\n", MAX_HEIGHT);
		goto err_exit;
	}
	buf = info->buffer;
	if ((buf->buf_id & BUF_ID_FLG_PIXFMT) == 0) {
		/* pixel format not specified */
		printk_dbg2(2, "pixel format not specified."
			" ignore parameter check of data size.\n");
	} else {
		/* width: odd size check */
		if (buf->data_fmt == CMPPIXFMT_YUV420PL ||
		   buf->data_fmt == CMPPIXFMT_YUV420SP ||
		   buf->data_fmt == CMPPIXFMT_YUV422SP) {
			if (arg->x & 1) {
				printk_err2("argument x invalid. odd\n");
				goto err_exit;
			}
		}
		/* height: odd size check */
		if (buf->data_fmt == CMPPIXFMT_YUV420PL ||
		   buf->data_fmt == CMPPIXFMT_YUV420SP) {
			if (arg->y & 1) {
				printk_err2("argument y invalid. odd\n");
				goto err_exit;
			}
		}
	}
pass_exit:
	/* pass */
	rc = 0;
err_exit:
	return rc;
}
static int  chk_ioc_pixfmt(struct composer_fh *fh, \
	struct cmp_lay_pixfmt *arg, int set_flag)
{
	int rc = -EINVAL;

	switch (arg->level) {
	case CMP_OUT_PLANE:
	case CMP_CH1_LAYER:
	case CMP_CH2_LAYER:
	case CMP_CH3_LAYER:
	case CMP_CH4_LAYER:
		break;
	default:
		printk_err2("argument level 0x%x invalid.\n", arg->level);
		goto err_exit;
		break;
	}

	if (get_composer_info(fh, arg->level) == NULL) {
		printk_err2("argument level 0x%x not open.\n", arg->level);
		goto err_exit;
	}

	if (set_flag == 0) {
		/* skip parameter check of laysize */
		goto pass_exit;
	}
	if (arg->level == CMP_OUT_PLANE) {
		/* When the output format and an input format changed,*/
		/* Judgment processing is coded separately.           */
		switch (arg->pixfmt) {
		case CMPPIXFMT_ARGB8888:
		case CMPPIXFMT_YUV420SP:
		case CMPPIXFMT_YUV422SP:
		case CMPPIXFMT_RGB888:
		case CMPPIXFMT_RGB565:
		case CMPPIXFMT_YUV420PL:
			break;
		default:
			printk_err2("argument pixfmt 0x%x invalid "
				"for level %d\n", arg->pixfmt, arg->level);
			goto err_exit;
			break;
		}
	} else {
		switch (arg->pixfmt) {
		case CMPPIXFMT_ARGB8888:
		case CMPPIXFMT_RGB888:
		case CMPPIXFMT_RGB565:
		case CMPPIXFMT_YUV420SP:
		case CMPPIXFMT_YUV422SP:
		case CMPPIXFMT_YUV420PL:
			break;
		default:
			printk_err2("argument pixfmt 0x%x invalid "
				"for level %d.\n", arg->pixfmt, arg->level);
			goto err_exit;
			break;
		}
	}
	switch (arg->colorspace) {
	case CMPYUVCOLOR_BT601_COMPRESS:
	case CMPYUVCOLOR_BT601_FULLSCALE:
	case CMPYUVCOLOR_BT709_COMPRESS:
	case CMPYUVCOLOR_BT709_FULLSCALE:
		break;
	default:
		printk_err2("argument colorspace 0x%x invalid.\n",
			arg->colorspace);
		goto err_exit;
		break;
	}
pass_exit:
	/* pass */
	rc = 0;
err_exit:
	return rc;
}
static int  chk_ioc_cropsize(struct composer_fh *fh, \
	struct cmp_lay_cropsize *arg, int set_flag, \
	struct cmp_lay_data_size *laysize)
{
	int rc = -EINVAL;
	struct composer_buffer *buf;
	struct composer_info   *info;

	switch (arg->level) {
	case CMP_OUT_PLANE:
	case CMP_CH1_LAYER:
	case CMP_CH2_LAYER:
	case CMP_CH3_LAYER:
	case CMP_CH4_LAYER:
		break;
	default:
		printk_err2("argument level 0x%x invalid.\n", arg->level);
		goto err_exit;
		break;
	}

	info = get_composer_info(fh, arg->level);
	if (info == NULL) {
		printk_err2("argument level 0x%x not open.\n", arg->level);
		goto err_exit;
	}

	if (set_flag == 0) {
		/* skip parameter check of laysize */
		goto pass_exit;
	}
	if (arg->x < 1 || arg->x > MAX_WIDTH) {
		printk_err2("argument x invalid. 1-%d\n", MAX_WIDTH);
		goto err_exit;
	}
	if (arg->y < 1 || arg->y > MAX_HEIGHT) {
		printk_err2("argument x invalid. 1-%d\n", MAX_HEIGHT);
		goto err_exit;
	}
	buf = info->buffer;
	if (laysize == NULL) {
		if ((buf->buf_id & BUF_ID_FLG_LAYSIZE) != BUF_ID_FLG_LAYSIZE) {
			/* layer size not specified */
			printk_dbg2(2, "layer size not specified."
				" ignore parameter check of crop size.\n");
		} else {
			if (arg->x >  buf->data_x) {
				printk_err2("argument x invalid."
					" over layer size\n");
				goto err_exit;
			}
			if (arg->y > buf->data_y) {
				printk_err2("argument y invalid."
					" over layer size\n");
				goto err_exit;
			}
		}
	} else {
		if (arg->x >  laysize->x) {
			printk_err2("argument x invalid. over layer size\n");
			goto err_exit;
		}
		if (arg->y > laysize->y) {
			printk_err2("argument y invalid. over layer size\n");
			goto err_exit;
		}
	}

	if ((buf->buf_id & BUF_ID_FLG_PIXFMT) == 0) {
		/* pixel format not specified */
		printk_dbg2(2, "pixel format not specified."
			" ignore parameter check of data size.\n");
	} else {
		/* width: odd size check */
		if (buf->data_fmt == CMPPIXFMT_YUV420PL ||
		   buf->data_fmt == CMPPIXFMT_YUV420SP ||
		   buf->data_fmt == CMPPIXFMT_YUV422SP) {
			if (arg->x & 1) {
				printk_err2("argument x invalid. odd\n");
				goto err_exit;
			}
		}
		/* height: odd size check */
		if (buf->data_fmt == CMPPIXFMT_YUV420PL ||
		   buf->data_fmt == CMPPIXFMT_YUV420SP) {
			if (arg->y & 1) {
				printk_err2("argument y invalid. odd\n");
				goto err_exit;
			}
		}
	}
pass_exit:
	/* pass */
	rc = 0;
err_exit:
	return rc;
}
static int  chk_ioc_croppos(struct composer_fh *fh, \
	struct cmp_lay_croppos *arg, int set_flag, \
	struct cmp_lay_data_size *laysize)
{
	int rc = -EINVAL;
	struct composer_buffer *buf;
	struct composer_info   *info;

	switch (arg->level) {
	case CMP_OUT_PLANE:
	case CMP_CH1_LAYER:
	case CMP_CH2_LAYER:
	case CMP_CH3_LAYER:
	case CMP_CH4_LAYER:
		break;
	default:
		printk_err2("argument level 0x%x invalid.\n", arg->level);
		goto err_exit;
		break;
	}

	info = get_composer_info(fh, arg->level);
	if (info == NULL) {
		printk_err2("argument level 0x%x not open.\n", arg->level);
		goto err_exit;
	}

	if (set_flag == 0) {
		/* skip parameter check of laysize */
		goto pass_exit;
	}
	if (arg->x < 0 || arg->x > MAX_WIDTH - 1) {
		printk_err2("argument x invalid. 0-%d\n", MAX_WIDTH - 1);
		goto err_exit;
	}
	if (arg->y < 0 || arg->y > MAX_HEIGHT - 1) {
		printk_err2("argument y invalid. 0-%d\n", MAX_HEIGHT - 1);
		goto err_exit;
	}
	buf = info->buffer;

	if (laysize == NULL) {
		if ((buf->buf_id & BUF_ID_FLG_LAYSIZE) != BUF_ID_FLG_LAYSIZE) {
			/* layer size not specified */
			printk_dbg2(2, "layer size not specified."
				" ignore parameter check of crop size.\n");
		} else {
			if (arg->x >=  buf->data_x) {
				printk_err2("argument x invalid."
					" over layer size\n");
				goto err_exit;
			}
			if (arg->y >= buf->data_y) {
				printk_err2("argument y invalid."
					" over layer size\n");
				goto err_exit;
			}
		}
	} else {
		if (arg->x >=  laysize->x) {
			printk_err2("argument x invalid. over layer size\n");
			goto err_exit;
		}
		if (arg->y >= laysize->y) {
			printk_err2("argument y invalid. over layer size\n");
			goto err_exit;
		}
	}

	if ((buf->buf_id & BUF_ID_FLG_PIXFMT) == 0) {
		/* pixel format not specified */
		printk_dbg2(2, "pixel format not specified."
			" ignore parameter check of data size.\n");
	} else {
		/* width: odd size check */
		if (buf->data_fmt == CMPPIXFMT_YUV420PL ||
		   buf->data_fmt == CMPPIXFMT_YUV420SP ||
		   buf->data_fmt == CMPPIXFMT_YUV422SP) {
			if (arg->x & 1) {
				printk_err2("argument x invalid. odd\n");
				goto err_exit;
			}
		}
		/* height: odd size check */
		if (buf->data_fmt == CMPPIXFMT_YUV420PL ||
		   buf->data_fmt == CMPPIXFMT_YUV420SP) {
			if (arg->y & 1) {
				printk_err2("argument y invalid. odd\n");
				goto err_exit;
			}
		}
	}
pass_exit:
	/* pass */
	rc = 0;
err_exit:
	return rc;
}
static int  chk_ioc_pos(struct composer_fh *fh, \
	struct cmp_lay_pos *arg, int set_flag)
{
	int rc = -EINVAL;
	struct composer_buffer *buf, *argbuf;
	struct composer_info   *info;
	int   target_x, target_y;

	switch (arg->level) {
	case CMP_CH1_LAYER:
	case CMP_CH2_LAYER:
	case CMP_CH3_LAYER:
	case CMP_CH4_LAYER:
		break;
	default:
		printk_err2("argument level 0x%x invalid.\n", arg->level);
		goto err_exit;
		break;
	}

	info = get_composer_info(fh, arg->level);
	if (info == NULL) {
		printk_err2("argument level 0x%x not open.\n", arg->level);
		goto err_exit;
	}

	if (set_flag == 0) {
		/* skip parameter check of laysize */
		goto pass_exit;
	}
	if (arg->x < 0 || arg->x > MAX_WIDTH - 1) {
		printk_err2("argument x invalid. 0-%d\n", MAX_WIDTH - 1);
		goto err_exit;
	}
	if (arg->y < 0 || arg->y > MAX_HEIGHT - 1) {
		printk_err2("argument y invalid. 0-%d\n", MAX_HEIGHT - 1);
		goto err_exit;
	}
	argbuf = info->buffer;
	buf = fh->fh_info[IDX_OUTPLANE].buffer;

	if (buf == NULL) {
		printk_dbg2(2, "lavel CMP_OUT_PLANE not opened."
			" ignore parameter check of blend pos.\n");
		goto pass_exit;
	}

	if ((buf->buf_id & BUF_ID_FLG_CROPSIZE) == BUF_ID_FLG_CROPSIZE) {
		/* crop size specified */
		target_x = buf->crop_size_x;
		target_y = buf->crop_size_y;
	} else if ((buf->buf_id & BUF_ID_FLG_LAYSIZE) == BUF_ID_FLG_LAYSIZE) {
		/* crop size not specified */
		target_x = buf->data_x;
		target_y = buf->data_y;
	} else {
		printk_dbg2(2, "out plane layer size not specified."
			" ignore parameter check of blend pos.\n");
		target_x = 0;
		target_y = 0;
	}

	if (target_x != 0 && target_y != 0) {
		if (arg->x >=  target_x) {
			printk_err2("argument x invalid."
				" over out plpne size\n");
			goto err_exit;
		}
		if (arg->y >= target_y) {
			printk_err2("argument y invalid."
				" over out plane size\n");
			goto err_exit;
		}
	}

#if 0 /* may be not necessary */
	if ((buf->buf_id & BUF_ID_FLG_PIXFMT) == 0) {
		/* pixel format not specified */
		printk_dbg2(2, "out plane pixel format not specified."
			" ignore parameter check of disp pos.\n");
	} else {
		/* width: odd size check */
		if (buf->data_fmt == CMPPIXFMT_YUV420PL ||
		   buf->data_fmt == CMPPIXFMT_YUV420SP ||
		   buf->data_fmt == CMPPIXFMT_YUV422SP) {
			if (arg->x & 1) {
				printk_err2("argument x invalid. odd\n");
				goto err_exit;
			}
		}
		/* height: odd size check */
		if (buf->data_fmt == CMPPIXFMT_YUV420PL ||
		   buf->data_fmt == CMPPIXFMT_YUV420SP) {
			if (arg->y & 1) {
				printk_err2("argument y invalid. odd\n");
				goto err_exit;
			}
		}
	}
#endif

	if ((argbuf->buf_id & BUF_ID_FLG_PIXFMT) == 0) {
		/* pixel format not specified */
		printk_dbg2(2, "pixel format not specified."
			" ignore parameter check of disp pos.\n");
	} else {
		/* width: odd size check */
		if (argbuf->data_fmt == CMPPIXFMT_YUV420PL ||
		   argbuf->data_fmt == CMPPIXFMT_YUV420SP ||
		   argbuf->data_fmt == CMPPIXFMT_YUV422SP) {
			if (arg->x & 1) {
				printk_err2("argument x invalid. odd\n");
				goto err_exit;
			}
		}
		/* height: odd size check */
		if (argbuf->data_fmt == CMPPIXFMT_YUV420PL ||
		   argbuf->data_fmt == CMPPIXFMT_YUV420SP) {
			if (arg->y & 1) {
				printk_err2("argument y invalid. odd\n");
				goto err_exit;
			}
		}
	}

pass_exit:
	/* pass */
	rc = 0;
err_exit:
	return rc;
}
static int  chk_ioc_size(struct composer_fh *fh, \
	struct cmp_lay_compose_size *arg, int set_flag)
{
	int rc = -EINVAL;
	struct composer_buffer *buf, *argbuf;
	struct composer_info   *info;
	int   target_x, target_y;

	switch (arg->level) {
	case CMP_CH1_LAYER:
	case CMP_CH2_LAYER:
	case CMP_CH3_LAYER:
	case CMP_CH4_LAYER:
		break;
	default:
		printk_err2("argument level 0x%x invalid.\n", arg->level);
		goto err_exit;
		break;
	}

	info = get_composer_info(fh, arg->level);
	if (info == NULL) {
		printk_err2("argument level 0x%x not open.\n", arg->level);
		goto err_exit;
	}

	if (set_flag == 0) {
		/* skip parameter check of laysize */
		goto pass_exit;
	}
	if (arg->x < 1 || arg->x > MAX_WIDTH) {
		printk_err2("argument x invalid. 1-%d\n", MAX_WIDTH);
		goto err_exit;
	}
	if (arg->y < 1 || arg->y > MAX_HEIGHT) {
		printk_err2("argument x invalid. 1-%d\n", MAX_HEIGHT);
		goto err_exit;
	}
	argbuf = info->buffer;
	buf = fh->fh_info[IDX_OUTPLANE].buffer;

	if (buf == NULL) {
		printk_dbg2(2, "lavel CMP_OUT_PLANE not opened."
			" ignore parameter check of blend pos.\n");
		goto pass_exit;
	}

	if ((buf->buf_id & BUF_ID_FLG_CROPSIZE) == BUF_ID_FLG_CROPSIZE) {
		/* crop size specified */
		target_x = buf->crop_size_x;
		target_y = buf->crop_size_y;
	} else if ((buf->buf_id & BUF_ID_FLG_LAYSIZE) == BUF_ID_FLG_LAYSIZE) {
		/* layer size not specified */
		target_x = buf->data_x;
		target_y = buf->data_y;
	} else {
		printk_dbg2(2, "out plane layer size not specified."
			" ignore parameter check of blend pos.\n");
		target_x = 0;
		target_y = 0;
	}

	if (target_x != 0 && target_y != 0) {
		if (arg->x >  target_x) {
			printk_err2("argument x invalid."
				" over out plpne size\n");
			goto err_exit;
		}
		if (arg->y > target_y) {
			printk_err2("argument y invalid."
				" over out plane size\n");
			goto err_exit;
		}
	}

#if 0 /* may be not necessary */
	if ((buf->buf_id & BUF_ID_FLG_PIXFMT) == 0) {
		/* pixel format not specified */
		printk_dbg2(2, "out plane pixel format not specified."
			" ignore parameter check of data size.\n");
	} else {
		/* width: odd size check */
		if (buf->data_fmt == CMPPIXFMT_YUV420PL ||
		   buf->data_fmt == CMPPIXFMT_YUV420SP ||
		   buf->data_fmt == CMPPIXFMT_YUV422SP) {
			if (arg->x & 1) {
				printk_err2("argument x invalid. odd\n");
				goto err_exit;
			}
		}
		/* height: odd size check */
		if (buf->data_fmt == CMPPIXFMT_YUV420PL ||
		   buf->data_fmt == CMPPIXFMT_YUV420SP) {
			if (arg->y & 1) {
				printk_err2("argument y invalid. odd\n");
				goto err_exit;
			}
		}
	}
#endif

	if ((argbuf->buf_id & BUF_ID_FLG_PIXFMT) == 0) {
		/* pixel format not specified */
		printk_dbg2(2, "out plane pixel format not specified."
			" ignore parameter check of data size.\n");
	} else {
		/* width: odd size check */
		if (argbuf->data_fmt == CMPPIXFMT_YUV420PL ||
		   argbuf->data_fmt == CMPPIXFMT_YUV420SP ||
		   argbuf->data_fmt == CMPPIXFMT_YUV422SP) {
			if (arg->x & 1) {
				printk_err2("argument x invalid. odd\n");
				goto err_exit;
			}
		}
		/* height: odd size check */
		if (argbuf->data_fmt == CMPPIXFMT_YUV420PL ||
		   argbuf->data_fmt == CMPPIXFMT_YUV420SP) {
			if (arg->y & 1) {
				printk_err2("argument y invalid. odd\n");
				goto err_exit;
			}
		}
	}
pass_exit:
	/* pass */
	rc = 0;
err_exit:
	return rc;
}

#define VIEW_MASK(X) ((X) & 0x80000000)

static int  chk_ioc_viewlay(struct composer_fh *fh, \
	struct cmp_viewlay *arg, int idx)
{
	int               rc = -EINVAL;
	int               flag;
	struct cmp_lay_data_size laysize;
	struct cmp_lay_cropsize  cropsize;
	struct cmp_lay_croppos   croppos;
	struct cmp_lay_pos       pos;
	struct cmp_lay_compose_size size;
	const struct composer_buffer *buffer;

	switch (arg->level) {
	case CMP_OUT_PLANE:
		flag = 0x0;
		break;
	case CMP_CH1_LAYER:
	case CMP_CH2_LAYER:
	case CMP_CH3_LAYER:
	case CMP_CH4_LAYER:
		flag = 0x1;
		break;
	default:
		printk_err2("argument level 0x%x invalid.\n", arg->level);
		goto err_exit;
		break;
	}

	if (get_composer_info(fh, arg->level) == NULL) {
		printk_err2("argument level 0x%x not open.\n", arg->level);
		goto err_exit;
	}

	if (idx < 0) {
		/* skip parameter check of viewlay */
		goto pass_exit;
	}
	buffer = fh->fh_info[idx].buffer;
	if (buffer == NULL) {
		rc = 0;
		goto err_exit;
	}

	/* check configure paramter */
	{
		int    chk1, chk2, need_init_flag;

		/* set laysize needed. */

		chk1 = (VIEW_MASK(arg->data_x) == 0) ? 1 : 0;
		chk2 = (VIEW_MASK(arg->data_y) == 0) ? 1 : 0;
		need_init_flag = ((buffer->buf_id & BUF_ID_FLG_LAYSIZE) \
				!= BUF_ID_FLG_LAYSIZE) ? 1 : 0;

		if (chk1 || chk2) {
			flag |= (0x10);
			if (chk1 && chk2) {
				/* no need initialize */
			} else if (need_init_flag) {
				rc = CMP_NG_VIEWLAY1;
				printk_err2("CMP_IOCS_LAYSIZE not performed.");
				goto err_exit;
			}
		}

		/* set cropsize needed. */
		chk1 = (VIEW_MASK(arg->cropsize_x) == 0) ? 1 : 0;
		chk2 = (VIEW_MASK(arg->cropsize_y) == 0) ? 1 : 0;
		need_init_flag = ((buffer->buf_id & BUF_ID_FLG_CROPSIZE) \
				!= BUF_ID_FLG_CROPSIZE) ? 1 : 0;

		if (chk1 || chk2) {
			flag |= (0x20);
			if (chk1 && chk2) {
				/* no need initialize */
			} else if (need_init_flag) {
				rc = CMP_NG_VIEWLAY2;
				printk_err2("CMP_IOCS_CROPSIZE not performed.");
				goto err_exit;
			}

			if ((flag & 0x10) == 0 && \
				(buffer->buf_id & BUF_ID_FLG_LAYSIZE) \
				!= BUF_ID_FLG_LAYSIZE) {
				rc = CMP_NG_VIEWLAY1;
				printk_err2("CMP_IOCS_LAYSIZE not performed.");
				goto err_exit;
			}
		}

		/* set croppos needed. */
		chk1 = (VIEW_MASK(arg->croppos_x) == 0) ? 1 : 0;
		chk2 = (VIEW_MASK(arg->croppos_y) == 0) ? 1 : 0;
		need_init_flag = ((buffer->buf_id & BUF_ID_FLG_CROPPOS) \
				!= BUF_ID_FLG_CROPPOS) ? 1 : 0;
		if (chk1 || chk2) {
			flag |= (0x40);
			if (chk1 && chk2) {
				/* no need initialize */
			} else if (need_init_flag) {
				rc = CMP_NG_VIEWLAY3;
				printk_err2("CMP_IOCS_CROPPOS not performed.");
				goto err_exit;
			}
			if ((flag & 0x10) == 0 && \
				(buffer->buf_id & BUF_ID_FLG_LAYSIZE) \
				!= BUF_ID_FLG_LAYSIZE) {
				rc = CMP_NG_VIEWLAY1;
				printk_err2("CMP_IOCS_LAYSIZE not performed.");
				goto err_exit;
			}
		}

		/* set size needed. */
		chk1 = (VIEW_MASK(arg->compose_x) == 0) ? 1 : 0;
		chk2 = (VIEW_MASK(arg->compose_y) == 0) ? 1 : 0;
		need_init_flag = ((buffer->buf_id & BUF_ID_FLG_SIZE) \
				!= BUF_ID_FLG_SIZE) ? 1 : 0;

		if (chk1 || chk2) {
			flag |= (0x80);
			if (chk1 && chk2) {
				/* no need initialize */
			} else if (need_init_flag) {
				rc = CMP_NG_VIEWLAY4;
				printk_err2("CMP_IOCS_SIZE not performed.");
				goto err_exit;
			}
		}

		/* set pos needed. */
		chk1 = (VIEW_MASK(arg->pos_x) == 0) ? 1 : 0;
		chk2 = (VIEW_MASK(arg->pos_y) == 0) ? 1 : 0;
		need_init_flag = ((buffer->buf_id & BUF_ID_FLG_POS) \
				!= BUF_ID_FLG_POS) ? 1 : 0;

		if (chk1 || chk2) {
			flag |= (0x100);
			if (chk1 && chk2) {
				/* no need initialize */
			} else if (need_init_flag) {
				rc = CMP_NG_VIEWLAY5;
				printk_err2("CMP_IOCS_POS not performed.");
				goto err_exit;
			}
		}
	}

	/* create laysize parameter */
	if (flag & 0x10) {
		if (VIEW_MASK(arg->data_x)) {
			/* set argument from previous configuration. */
			arg->data_x = buffer->data_x;
		}
		if (VIEW_MASK(arg->data_y)) {
			/* set argument from previous configuration. */
			arg->data_y = buffer->data_y;
		}
		laysize.level = arg->level;
		laysize.x     = arg->data_x;
		laysize.y     = arg->data_y;
	} else {
		/* set default laysize */
		laysize.x     = buffer->data_x;
		laysize.y     = buffer->data_y;
	}

	/* create croppos parameter */
	if (flag & 0x40) {
		if (VIEW_MASK(arg->croppos_x)) {
			/* set argument from previous configuration. */
			arg->croppos_x = buffer->crop_pos_x;
		}
		if (VIEW_MASK(arg->croppos_y)) {
			/* set argument from previous configuration. */
			arg->croppos_y = buffer->crop_pos_y;
		}
		croppos.level = arg->level;
		croppos.x = arg->croppos_x;
		croppos.y = arg->croppos_y;
	}

	/* create cropsize parameter */
	if (flag & 0x20) {
		if (VIEW_MASK(arg->cropsize_x)) {
			/* set argument from previous configuration. */
			arg->cropsize_x = buffer->crop_size_x;
		}
		if (VIEW_MASK(arg->cropsize_y)) {
			/* set argument from previous configuration. */
			arg->cropsize_y = buffer->crop_size_y;
		}
		cropsize.level = arg->level;
		cropsize.x = arg->cropsize_x;
		cropsize.y = arg->cropsize_y;
	}

	/* create size parameter */
	if (flag & 0x80) {
		if (VIEW_MASK(arg->compose_x)) {
			/* set argument from previous configuration. */
			arg->compose_x = buffer->size_x;
		}
		if (VIEW_MASK(arg->compose_y)) {
			/* set argument from previous configuration. */
			arg->compose_y = buffer->size_y;
		}
		size.level = arg->level;
		size.x = arg->compose_x;
		size.y = arg->compose_y;
	}

	/* create size parameter */
	if (flag & 0x100) {
		if (VIEW_MASK(arg->pos_x)) {
			/* set argument from previous configuration. */
			arg->pos_x = buffer->pos_x;
		}
		if (VIEW_MASK(arg->pos_y)) {
			/* set argument from previous configuration. */
			arg->pos_y = buffer->pos_y;
		}
		pos.level = arg->level;
		pos.x = arg->pos_x;
		pos.y = arg->pos_y;
	}

	if ((flag & 0x010) && chk_ioc_laysize(fh, &laysize, 1) != 0) {
		rc = CMP_NG_VIEWLAY1;
		goto err_exit;
	}

	if ((flag & 0x020) && chk_ioc_cropsize(fh, &cropsize, 1, &laysize)
		!= 0) {
		rc = CMP_NG_VIEWLAY2;
		goto err_exit;
	}

	if ((flag & 0x040) && chk_ioc_croppos(fh, &croppos, 1, &laysize) != 0) {
		rc = CMP_NG_VIEWLAY3;
		goto err_exit;
	}

	if ((flag & 0x081) == 0x081 && chk_ioc_size(fh, &size, 1) != 0) {
		rc = CMP_NG_VIEWLAY4;
		goto err_exit;
	}

	if ((flag & 0x101) == 0x101 && chk_ioc_pos(fh, &pos, 1) != 0) {
		rc = CMP_NG_VIEWLAY5;
		goto err_exit;
	}
pass_exit:
	/* pass */
	rc = 0;
err_exit:
	return rc;
}

#undef  VIEW_MASK

static int  chk_ioc_backcolor(struct composer_fh *fh, \
	struct cmp_lay_backcolor *arg, int set_flag)
{
	int rc = -EINVAL;

	switch (arg->level) {
	case CMP_OUT_PLANE:
		break;
	default:
		printk_err2("argument level 0x%x invalid.\n", arg->level);
		goto err_exit;
		break;
	}

	if (get_composer_info(fh, arg->level) == NULL) {
		printk_err2("argument level 0x%x not open.\n", arg->level);
		goto err_exit;
	}

	if (set_flag == 0) {
		/* skip parameter check of keycolor */
		goto pass_exit;
	}
	if (arg->backcolor > 0xffffff) {
		printk_err2("argument backcolor 0x%x invalid.\n",
			arg->backcolor);
		goto err_exit;
	}
pass_exit:
	/* pass */
	rc = 0;
err_exit:
	return rc;
}
static int  chk_ioc_layaddr(struct composer_fh *fh, \
	struct cmp_layaddr *arg, int set_flag)
{
	int rc = -EINVAL;
	struct composer_info   *info;
	struct composer_buffer *buf;
	int   target_x, target_y, target_pixfmt;

	switch (arg->level) {
	case CMP_OUT_PLANE:
	case CMP_CH1_LAYER:
	case CMP_CH2_LAYER:
	case CMP_CH3_LAYER:
	case CMP_CH4_LAYER:
		break;
	default:
		printk_err2("argument level 0x%x invalid.\n", arg->level);
		goto err_exit;
		break;
	}

	info = get_composer_info(fh, arg->level);
	if (info == NULL) {
		printk_err2("argument level 0x%x not open.\n", arg->level);
		goto err_exit;
	}

	if (set_flag == 0) {
		/* skip parameter check of laysize */
		goto pass_exit;
	}
	buf = info->buffer;

	if ((buf->buf_id & BUF_ID_FLG_LAYSIZE) == BUF_ID_FLG_LAYSIZE) {
		/* layer size not specified */
		target_x = buf->data_x;
		target_y = buf->data_y;
	} else {
		printk_dbg2(2, "layer size not specified."
			" ignore parameter check of set adr.\n");
		target_x = 0;
		target_y = 0;
	}

	if ((buf->buf_id & BUF_ID_FLG_PIXFMT)) {
		/* layer size not specified */
		target_pixfmt = buf->data_fmt;
	} else {
		printk_dbg2(2, "pixel format not specified."
			" ignore parameter check of set adr.\n");
		target_pixfmt = -1;
	}

	if (target_x != 0 && target_y != 0 && target_pixfmt >= 0) {
		int min_y_size = 0, min_c_size = 0, min_c1_size = 0;
		switch (target_pixfmt) {
		case CMPPIXFMT_ARGB8888:
			min_y_size = target_x * target_y * 4;
			min_c_size = 0;
			break;
		case CMPPIXFMT_RGB888:
			min_y_size = target_x * target_y * 3;
			min_c_size = 0;
			break;
		case CMPPIXFMT_RGB565:
			min_y_size = target_x * target_y * 2;
			min_c_size = 0;
			break;
		case CMPPIXFMT_YUV420SP:
			min_y_size = target_x * target_y;
			min_c_size = target_x * target_y / 2;
			break;
		case CMPPIXFMT_YUV422SP:
			min_y_size = target_x * target_y;
			min_c_size = target_x * target_y;
			break;
		case CMPPIXFMT_YUV420PL:
			min_y_size = target_x * target_y;
			min_c_size = target_x * target_y / 4;
			min_c1_size = min_c_size;
			break;
		}

		if (arg->datasize <  min_y_size) {
			printk_err2("argument datasize is too small"
				" for %dx%d fmt:%d\n",
				target_x, target_y, target_pixfmt);
			goto err_exit;
		}
		if (min_c_size > 0 &&  arg->datasize_c0 <  min_c_size) {
			printk_err2("argument datasize_c0 is too small"
				" for %dx%d fmt:%d\n",
				target_x, target_y, target_pixfmt);
			goto err_exit;
		}
		if (min_c1_size > 0 && arg->datasize_c1 <  min_c1_size) {
			printk_err2("argument datasize_c1 is too small"
				" for %dx%d fmt:%d\n",
				target_x, target_y, target_pixfmt);
			goto err_exit;
		}
	}
pass_exit:
	/* pass */
	rc = 0;
err_exit:
	return rc;
}
static int  chk_ioc_start(struct composer_fh *fh)
{
	int i;
	int rc = 0;

	static const int fh_status_translate[] = {
		[CMP_OUT_PLANE]  = FH_STATUS_OUTPLANE,
		[CMP_CH1_LAYER]  = FH_STATUS_CH1LAYER,
		[CMP_CH2_LAYER]  = FH_STATUS_CH2LAYER,
		[CMP_CH3_LAYER]  = FH_STATUS_CH3LAYER,
		[CMP_CH4_LAYER]  = FH_STATUS_CH4LAYER
	};

	static const int buffer_mask[] = {
		[CMP_OUT_PLANE]  = BUF_ID_SET_BUFFER | \
			BUF_ID_FLG_PIXFMT | BUF_ID_FLG_LAYSIZE,
		[CMP_CH1_LAYER] = BUF_ID_SET_BUFFER | BUF_ID_FLG_PIXFMT \
			| BUF_ID_FLG_LAYSIZE | BUF_ID_FLG_POS | BUF_ID_FLG_SIZE,
		[CMP_CH2_LAYER] = BUF_ID_SET_BUFFER | BUF_ID_FLG_PIXFMT \
			| BUF_ID_FLG_LAYSIZE | BUF_ID_FLG_POS | BUF_ID_FLG_SIZE,
		[CMP_CH3_LAYER] = BUF_ID_SET_BUFFER | BUF_ID_FLG_PIXFMT \
			| BUF_ID_FLG_LAYSIZE | BUF_ID_FLG_POS | BUF_ID_FLG_SIZE,
		[CMP_CH4_LAYER] = BUF_ID_SET_BUFFER | BUF_ID_FLG_PIXFMT \
			| BUF_ID_FLG_LAYSIZE | BUF_ID_FLG_POS | BUF_ID_FLG_SIZE
	};

	static unsigned int src_pos_x[5],  src_pos_y[5];
	static unsigned int src_size_w[5], src_size_h[5];
	int    resize_idx;

	/* check open */
	if ((fh->fh_status & FH_STATUS_OUTPLANE) == 0) {
		printk_dbg2(2, "does not open CMP_OUT_PLANE\n");
		rc = CMP_NG_OPEN_OUT;
		goto err_exit;
	}

	for (i = 1; i < 5; i++) {
		if ((fh->fh_status & fh_status_translate[i]) == 0) {
			/* input layer not open */
			continue;
		}
		if (fh->fh_info[i].id & INFO_ID_FLG_NOTDISP) {
			/* not blending plane */
			continue;
		}
		/* find out usrable input layer */
		break;
	}
	if (i == 5) {
		printk_dbg2(2, "does not open CMP_CH*_LAYER\n");
		rc = CMP_NG_OPEN_IN;
		goto err_exit;
	}

	/* check parameter */
	for (i = 0; i < 5; i++) {
		unsigned int crop_x, crop_y, crop_w, crop_h;
		struct composer_info   *info;
		struct composer_buffer *buf;
		if ((fh->fh_status & fh_status_translate[i]) == 0) {
			/* not open */
			continue;
		}
		info = &fh->fh_info[i];
		buf = fh->fh_info[i].buffer;

		/* check buffer configuration */
		if (info->id & INFO_ID_FLG_NOTDISP) {
			/* not blending plane */
			continue;
		}

		/* check necessary configuration */
		if ((buf->buf_id & buffer_mask[i]) != buffer_mask[i]) {
			rc = CMP_NG_INSUFFICIENT_L0 - i;
			printk_err2("configuration not enough idx:%d 0x%x"
				" require:0x%x\n", i,
				buf->buf_id, buffer_mask[i]);
			goto err_exit;
		}

		/* set default image size */
		crop_x = 0;           crop_y = 0;
		crop_w = buf->data_x; crop_h = buf->data_y;

		if ((buf->buf_id & BUF_ID_FLG_CROPPOS)
			== BUF_ID_FLG_CROPPOS) {
			crop_x = buf->crop_pos_x;
			crop_y = buf->crop_pos_y;
		}
		if ((buf->buf_id & BUF_ID_FLG_CROPSIZE)
			== BUF_ID_FLG_CROPSIZE) {
			crop_w = buf->crop_size_x;
			crop_h = buf->crop_size_y;
		}

		/* check crop parameter */
		if (crop_x          >= buf->data_x || \
		     crop_w          >  buf->data_x || \
		     crop_x + crop_w >  buf->data_x || \
		     crop_y          >= buf->data_y || \
		     crop_h          >  buf->data_y || \
		     crop_y + crop_h >  buf->data_y) {
			rc = CMP_NG_CROPPOS_L0 - i;
			printk_err2("src area invalid. " \
				"(%d, %d)-(%d, %d), valid:(0, 0)-(%d, %d)\n",
					crop_x, crop_y, crop_x + crop_w,
					crop_y + crop_h,
					buf->data_x, buf->data_y);
			goto err_exit;
		}

		/* record src image size */
		src_pos_x[i] = crop_x;
		src_pos_y[i] = crop_y;
		src_size_w[i] = crop_w;
		src_size_h[i] = crop_h;
		printk_dbg2(2, "%d: src_pos(%d, %d), src_size(%d, %d)\n",
			i, src_pos_x[i], src_pos_y[i],
			src_size_w[i], src_size_h[i]);
	}

	/* check destination size */
	resize_idx = -1;
	for (i = 0; i < 5; i++) {
		unsigned int dst_x, dst_y, dst_w, dst_h;

		struct composer_info   *info;
		struct composer_buffer *buf;

		if ((fh->fh_status & fh_status_translate[i]) == 0) {
			/* not open */
			continue;
		}
		info = &fh->fh_info[i];
		buf = fh->fh_info[i].buffer;

		/* check buffer configuration */
		if (info->id & INFO_ID_FLG_NOTDISP) {
			/* not blending plane */
			continue;
		}
		/* set default image size */
		if ((buf->buf_id & (BUF_ID_FLG_POS | BUF_ID_FLG_SIZE))
			!= (BUF_ID_FLG_POS | BUF_ID_FLG_SIZE)) {
			/* destination parameter not supported. */
			continue;
		}

		dst_x = buf->pos_x;  dst_y = buf->pos_y;
		dst_w = buf->size_x; dst_h = buf->size_y;

		printk_dbg2(3, "%d: dst_pos(%d, %d), dst_size(%d, %d)\n",
			i, dst_x, dst_y, dst_w, dst_h);

		if (dst_x         >= src_size_w[IDX_OUTPLANE] || \
		     dst_w         >  src_size_w[IDX_OUTPLANE] || \
		     dst_x + dst_w >  src_size_w[IDX_OUTPLANE] || \
		     dst_y         >= src_size_h[IDX_OUTPLANE] || \
		     dst_h         >  src_size_h[IDX_OUTPLANE] || \
		     dst_y + dst_h >  src_size_h[IDX_OUTPLANE]) {
			rc = CMP_NG_LAYERPOS_L0 - i;
			printk_err2("dst area invalid. " \
				"(%d, %d)-(%d, %d), valid:(0, 0)-(%d, %d)\n",
					dst_x, dst_y, dst_x + dst_w,
					dst_y + dst_h,
					src_size_w[IDX_OUTPLANE],
					src_size_h[IDX_OUTPLANE]);
			goto err_exit;
		}

		if (src_size_w[i] != dst_w || src_size_h[i] != dst_h) {
			if (resize_idx < 0) {
				/* found resize channel */
				resize_idx = i;
			} else {
				printk_err2("already request resize at %d\n",
					resize_idx);
				rc = CMP_NG_NORESOURCE;
				goto err_exit;
			}

			if (dst_w < 16 || dst_h < 16 ||
			   src_size_w[i] < 4 || src_size_h[i] < 4 ||
			   dst_w           > src_size_w[i]  * 16 ||
			   src_size_w[i]    > dst_w         * 16 ||
			   dst_h            > src_size_h[i] * 16 ||
			   src_size_h[i]    > dst_h         * 16) {
				printk_err2("not supported scaler"
					" (%d, %d)->(%d, %d)\n",
					src_size_w[i], src_size_h[i],
					dst_w, dst_h);
				rc = CMP_NG_RESIZE_L0-i;
				goto err_exit;
			}
		}
	}

	/* pass */
	rc = 0;
err_exit:
	return rc;
}


#ifdef RT_GRAPHICS_MODE_IMAGE_OUTPUT
static void notify_graphics_image_output(int result, unsigned long user_data)
{
#ifdef CONFIG_MISC_R_MOBILE_COMPOSER_REQUEST_QUEUE
	struct composer_rh *rh;
#endif
	DBGENTER("result:%d user_data:0x%lx\n", result, user_data);

#ifdef CONFIG_MISC_R_MOBILE_COMPOSER_REQUEST_QUEUE
	/* confirm result code. */
	if (result < -256) {
#if SH_MOBILE_COMPOSER_SUPPORT_HDMI
		int i;
		for (i = 0; i < MAX_KERNELREQ; i++) {
			rh = &kernel_request[i];

			if (rh->active && rh->data.extlayer_index >= 0) {
				/* there is pending request. */
				rh->rh_wqcommon.status = 3;
				wake_up_interruptible_all(
					&rh->rh_wqcommon.wait_notify);
			}
		}
#endif /* SH_MOBILE_COMPOSER_SUPPORT_HDMI*/
	} else {
		rh = (struct composer_rh *) user_data;

		if (result != SMAP_LIB_GRAPHICS_OK) {
			/* report error */
			printk_err1("notify_graphics_image_output result:%d\n",
				result);
			rh->rh_wqcommon.status = 3;
		} else {
			rh->rh_wqcommon.status = 1;
		}

		/* wakeup waiting task */
		wake_up_interruptible_all(&rh->rh_wqcommon.wait_notify);
	}
#else
	printk_err1("callback unexpected.");
#endif

	DBGLEAVE("\n");
}
static void notify_graphics_image_output_dummy(
	int result, unsigned long user_data)
{
	/* currently not implemented. */
	printk_err1("callback unexpected.");
}

static void notify_graphics_image_blend_dummy(
	int result, unsigned long user_data)
{
	/* currently not implemented. */
	printk_err1("callback unexpected.");
}
#endif
static void notify_graphics_image_conv(int result, unsigned long user_data)
{
	/* currently not implemented. */
	printk_err1("callback unexpected.");
}
static void notify_graphics_image_blend(int result, unsigned long user_data)
{
	struct composer_blendcommon *common = \
		(struct composer_blendcommon *)user_data;

	DBGENTER("result:%d user_data:0x%lx\n", result, user_data);

	/* confirm result code. */
	if (result < -256) {
		struct list_head *list;
		int    match = 0;
#ifdef CONFIG_MISC_R_MOBILE_COMPOSER_REQUEST_QUEUE
		int    i;
#endif

		/* record RT-API hung-up */
		rtapi_hungup = 1;

		list_for_each(list, &file_top)
		{
			struct composer_fh *fh;

			fh = list_entry(list, struct composer_fh, fh_filelist);
			printk_dbg2(3, "list of valid user_data: %p\n", \
				&fh->fh_wqcommon);

			if (user_data == (unsigned long)&fh->fh_wqcommon) {
				match = 1;
				break;
			}
		}
#ifdef CONFIG_MISC_R_MOBILE_COMPOSER_REQUEST_QUEUE
		for (i = 0; i < MAX_KERNELREQ; i++) {
			struct composer_rh *rh = &kernel_request[i];

			printk_dbg2(3, "list of valid user_data: %p\n", \
				&rh->rh_wqcommon);

			if (user_data == (unsigned long)&rh->rh_wqcommon) {
				match = 1;
				break;
			}
		}
#endif
		if (!match) {
			printk_err("user_data 0x%lx unexpected."
				" ignore callback\n", user_data);
		}

		/* all graph status set to error */
		list_for_each(list, &file_top)
		{
			struct composer_fh *fh;

			fh = list_entry(list, struct composer_fh, \
				fh_filelist);

			fh->fh_wqcommon.status = 3;
			wake_up_interruptible_all(&fh->fh_wqcommon.wait_notify);
		}

#ifdef CONFIG_MISC_R_MOBILE_COMPOSER_REQUEST_QUEUE
		/* request queue status set to error */
		for (i = 0; i < MAX_KERNELREQ; i++) {
			struct composer_rh *rh = &kernel_request[i];

			rh->rh_wqcommon.status = 3;
			wake_up_interruptible_all(&rh->rh_wqcommon.wait_notify);
		}
#endif

		DBGLEAVE("\n");
		return;
	}

	if (result != SMAP_LIB_GRAPHICS_OK) {
		printk_err1("notify_graphics_image_blend result:%d\n", result);
		common->status = 3;
	} else {
		common->status = 1;
	}
	/* wakeup waiting task */
	wake_up_interruptible_all(&common->wait_notify);
	DBGLEAVE("\n");
}

#if _LOG_DBG >= 1
static void dump_screen_grap_initialize(screen_grap_initialize *arg)
{
	printk_dbg1(1, "screen_grap_initialize\n");
	printk_dbg1(1, "  handle:%p mode:%ld\n", arg->handle, arg->mode);
}

#if defined(CONFIG_MISC_R_MOBILE_COMPOSER_REQUEST_QUEUE) || \
	defined(DEBUG_DUMP_IMAGE_ADDRESS)
static void dump_address_data_to_string(
	unsigned char *mes, const unsigned char *ptr)
{
	*mes = 0;
	sprintf(mes, "%02x %02x %02x %02x %02x %02x %02x %02x :"
		"%02x %02x %02x %02x %02x %02x %02x %02x",
		ptr[0], ptr[1], ptr[2],  ptr[3],
		ptr[4], ptr[5], ptr[6],  ptr[7],
		ptr[8], ptr[9], ptr[10], ptr[11],
		ptr[12], ptr[13], ptr[14], ptr[15]);
}
#endif

static void dump_screen_grap_image_param(
	screen_grap_image_param * arg, char *name)
{
	printk_dbg1(1, " screen_grap_image_param[%s]\n", name);
	printk_dbg1(1, "  width:%d height:%d stride:%d stride_c:%d "
		"format:%d yuv_format:%d yuv_range:%d "
		"address:(%p, %p) address_c0(%p, %p) address_c1(%p, %p)\n",
		arg->width, arg->height, arg->stride, arg->stride_c,
		arg->format, arg->yuv_format, arg->yuv_range,
		arg->address, arg->apmem_handle,
		arg->address_c0, arg->apmem_handle_c0,
		arg->address_c1, arg->apmem_handle_c1);
	if (debug <= 2) {
		/* only report RT-API argument */
		return;
	}
#if defined(DEBUG_DUMP_IMAGE_ADDRESS)
/* functional specification of RT-API was changed, */
/* threfore no available dump image data.          */
	if (arg->apmem_handle && arg->address) {
		int i;
		char msg[256];
		const unsigned char *ptr = arg->address;
		printk_dbg1(2, "data in address");
		for (i = 0; i < 256; i += 16) {
			dump_address_data_to_string(&msg[0], ptr+i);
			printk_dbg1(2, "  %s\n", msg);
		}
	}
	if (arg->apmem_handle_c0 && arg->address_c0) {
		int i;
		char msg[256];
		const unsigned char *ptr = arg->address_c0;
		printk_dbg1(2, "data in address_c0\n");
		for (i = 0; i < 256; i += 16) {
			dump_address_data_to_string(&msg[0], ptr+i);
			printk_dbg1(2, "  %s\n", msg);
		}
	}
	if (arg->apmem_handle_c1 && arg->address_c1) {
		int i;
		char msg[256];
		const unsigned char *ptr = arg->address_c1;
		printk_dbg1(2, "data in address_c1\n");
		for (i = 0; i < 256; i += 16) {
			dump_address_data_to_string(&msg[0], ptr+i);
			printk_dbg1(2, "  %s\n", msg);
		}
	}
#endif
}

static void dump_screen_grap_layer(screen_grap_layer *arg, char *_name)
{
	static char name[256];

	if (arg == NULL) {
		/* layer not opened */
		return;
	}
	printk_dbg1(1, " screen_grap_layer[%s]\n", _name);
	sprintf(name, "%s.image", _name);
	dump_screen_grap_image_param(&arg->image, name);
	printk_dbg1(1, " %s.rect(x:%d y:%d width:%d height:%d) "
		"alpha:%d rotate:%d mirror:%d key_color:0x%lx\n",
		name,
		arg->rect.x, arg->rect.y, arg->rect.width, arg->rect.height,
		arg->alpha, arg->rotate, arg->mirror, arg->key_color);
}

static void dump_screen_grap_image_blend(screen_grap_image_blend *arg)
{
	printk_dbg1(1, "screen_grap_image_blend\n");

	printk_dbg1(1,
		"  handle:%p input_layer:(%p %p %p %p) "
		"background_color:0x%lx user_data:0x%lx\n",
		arg->handle, arg->input_layer[0], arg->input_layer[1],
		arg->input_layer[2], arg->input_layer[3],
		arg->background_color, arg->user_data);
	dump_screen_grap_image_param(&arg->output_image, "output_image");
	dump_screen_grap_layer(arg->input_layer[0], "input_layer0");
	dump_screen_grap_layer(arg->input_layer[1], "input_layer1");
	dump_screen_grap_layer(arg->input_layer[2], "input_layer2");
	dump_screen_grap_layer(arg->input_layer[3], "input_layer3");
}

static void dump_screen_grap_quit(screen_grap_quit *arg)
{
	printk_dbg1(1, "screen_grap_quit\n");
	printk_dbg1(1, "  handle:%p mode:%ld\n", arg->handle, arg->mode);
}

static void dump_screen_grap_delete(screen_grap_delete *arg)
{
	printk_dbg1(1, "screen_grap_delete\n");
	printk_dbg1(1, "  handle:%p\n", arg->handle);
}

#ifdef RT_GRAPHICS_MODE_IMAGE_OUTPUT
#if SH_MOBILE_COMPOSER_SUPPORT_HDMI > 1
static void dump_screen_grap_image_output(screen_grap_image_output *arg)
{
	printk_dbg1(1, "screen_grap_image_output\n");
	printk_dbg1(1, "  handle:%p\n", arg->handle);
	dump_screen_grap_image_param(&arg->output_image, "output_image");
	printk_dbg1(1, "  rotate:%d user_data:0x%lx\n",
		arg->rotate, arg->user_data);
}
#endif /*  SH_MOBILE_COMPOSER_SUPPORT_HDMI > 1 */
#endif

#endif

static void work_deletehandle(struct localwork *work)
{
	screen_grap_delete _del;
	screen_grap_quit   _quit;
	int                rc;

	TRACE_ENTER(FUNC_WQ_DELETE);
	DBGENTER("work:%p\n", work);

	if (sem.count != 0) {
		/* error report */
		printk_err("acquire semaphore needs to exclusive-control\n");
	}

	printk_dbg1(1, "delete_handle %p in PID:%d TGID:%d\n",
		graphic_handle, current->pid, current->tgid);

	if (graphic_handle) {
		_del.handle   = graphic_handle;
		_quit.handle  = graphic_handle;
		_quit.mode = RT_GRAPHICS_MODE_IMAGE_BLEND;
#if _LOG_DBG >= 1
		dump_screen_grap_quit(&_quit);
#endif
		rc = screen_graphics_quit(&_quit);
		if (rc != SMAP_LIB_GRAPHICS_OK) {
			/* error report */
			printk_err("screen_graphics_image_quit "
				"return by %d.\n", rc);
		}

#if _LOG_DBG >= 1
		dump_screen_grap_delete(&_del);
#endif
		screen_graphics_delete(&_del);
		graphic_handle = NULL;
	}
	TRACE_LEAVE(FUNC_WQ_DELETE);
	DBGLEAVE("\n");
}

static void work_createhandle(struct localwork *work)
{
	screen_grap_new _new;
	screen_grap_initialize  _ini;
	int  rc;

	TRACE_ENTER(FUNC_WQ_CREATE);
	DBGENTER("work:%p\n", work);

	if (sem.count != 0) {
		/* error report */
		printk_err("acquire semaphore needs to exclusive-control\n");
	}

	if (graphic_handle) {
		/* error report and free handle to re-create graphic handle */
		printk_err("graphic_handle is not NULL\n");
		work_deletehandle(work);
	}

	if (rtapi_hungup) {
		printk_err1("graphics system hungup.\n");
		goto finish;
	}

	/* update for screen_grap_new */
	_new.notify_graphics_image_conv  = notify_graphics_image_conv;
	_new.notify_graphics_image_blend = notify_graphics_image_blend;
#ifdef RT_GRAPHICS_MODE_IMAGE_OUTPUT
	_new.notify_graphics_image_output = notify_graphics_image_output_dummy;
#endif

	graphic_handle = screen_graphics_new(&_new);

	printk_dbg1(1, "screen_graphics_new result:%p in PID:%d TGID:%d\n",
		graphic_handle, current->pid, current->tgid);

	if (graphic_handle) {
		_ini.handle   = graphic_handle;
		_ini.mode = RT_GRAPHICS_MODE_IMAGE_BLEND;
#if _LOG_DBG >= 1
		dump_screen_grap_initialize(&_ini);
#endif
		rc = screen_graphics_initialize(&_ini);
		if (rc != SMAP_LIB_GRAPHICS_OK) {
			printk_err1("screen_graphics_initialize "
				"return by %d.\n", rc);

			work_deletehandle(work);
		}
#ifdef CONFIG_MISC_R_MOBILE_COMPOSER_REQUEST_QUEUE
		if (rc == 0) {
			/* map physical address */
			queue_fb_address_mapping();
		}
#endif
	} else {
		/* eror report */
		printk_dbg1(1, "graphic_handle is NULL\n");
	}
finish:
	TRACE_LEAVE(FUNC_WQ_CREATE);
	DBGLEAVE("\n");
}

#ifdef CONFIG_MISC_R_MOBILE_COMPOSER_REQUEST_QUEUE
static void work_expirequeue(struct localwork *work)
{
	int i;
	TRACE_ENTER(FUNC_WQ_EXPIRE);
	DBGENTER("work:%p\n", work);

	printk_dbg2(3, "down\n");
	down(&kernel_queue_sem);

	while (!list_empty(&kernel_queue_top)) {
		struct composer_rh *rh;

		rh = list_first_entry(&kernel_queue_top,
			struct composer_rh, list);

		printk_err("drop blend request.\n");

		/* remove list */
		list_del_init(&rh->list);

		/* process callback */
		up(&kernel_queue_sem);

		composer_blendoverlay_errorcallback(rh);

		printk_dbg2(3, "down\n");
		down(&kernel_queue_sem);
	}
	up(&kernel_queue_sem);

	for (i = 0; i < MAX_KERNELREQ; i++) {
		struct composer_rh *rh = &kernel_request[i];

		if (rh->active) {
			printk_err("force return request.\n");

			composer_blendoverlay_errorcallback(rh);
		}
	}

	TRACE_LEAVE(FUNC_WQ_EXPIRE);
	DBGLEAVE("\n");
}
#if SH_MOBILE_COMPOSER_SUPPORT_HDMI
static void work_deletehandle_hdmi(struct localwork *work)
{
	TRACE_ENTER(FUNC_WQ_DELETE_HDMI);
	DBGENTER("work:%p\n", work);

	/* currently not implemented. */
	if (graphic_handle_hdmi) {
#ifdef CONFIG_MACH_KOTA2
		/* Kota2 not support graphic output. */
		graphic_handle_hdmi = NULL;
#elif SH_MOBILE_COMPOSER_SUPPORT_HDMI <= 1 || \
	!defined(RT_GRAPHICS_MODE_IMAGE_OUTPUT)
		/* HDMI type is 1. or RT-API not available. */
		graphic_handle_hdmi = NULL;
#else
		screen_grap_delete _del;
		screen_grap_quit   _quit;
		int                rc;

		printk_dbg1(1, "delete_handle %p in PID:%d TGID:%d\n",
			graphic_handle_hdmi, current->pid, current->tgid);

		if (graphic_handle_hdmi) {
			_del.handle   = graphic_handle_hdmi;
			_quit.handle  = graphic_handle_hdmi;
			_quit.mode = RT_GRAPHICS_MODE_IMAGE_OUTPUT;
#if _LOG_DBG >= 1
			dump_screen_grap_quit(&_quit);
#endif
			rc = screen_graphics_quit(&_quit);
			if (rc != SMAP_LIB_GRAPHICS_OK) {
				/* error report */
				printk_err("screen_graphics_image_quit "
					"return by %d.\n", rc);
			}

#if _LOG_DBG >= 1
			dump_screen_grap_delete(&_del);
#endif
			screen_graphics_delete(&_del);
			graphic_handle_hdmi = NULL;
		}
#endif
	}

	TRACE_LEAVE(FUNC_WQ_DELETE_HDMI);
	DBGLEAVE("\n");
}

static void work_createhandle_hdmi(struct localwork *work)
{
	TRACE_ENTER(FUNC_WQ_CREATE_HDMI);
	DBGENTER("work:%p\n", work);

	/* currently not implemented. */
	if (rtapi_hungup) {
		/* report error */
		printk_err1("graphics system hungup.\n");
	} else if (graphic_handle_hdmi == NULL) {
#ifdef CONFIG_MACH_KOTA2
		/* Kota2 not support graphic output. */
		graphic_handle_hdmi = NULL;
#elif SH_MOBILE_COMPOSER_SUPPORT_HDMI <= 1 || \
	!defined(RT_GRAPHICS_MODE_IMAGE_OUTPUT)
		/* HDMI type is 1. or RT-API not available. */
		graphic_handle_hdmi = NULL;
#else
		/* screen_graphics_image_output available */
		screen_grap_new _new;
		screen_grap_initialize  _ini;
		int  rc;

		/* update for screen_grap_new */
		_new.notify_graphics_image_conv  = \
			notify_graphics_image_conv;
		_new.notify_graphics_image_blend = \
			notify_graphics_image_blend_dummy;
		_new.notify_graphics_image_output = \
			notify_graphics_image_output;
		graphic_handle_hdmi = screen_graphics_new(&_new);

		printk_dbg1(1, "screen_graphics_new result:%p "
			"in PID:%d TGID:%d\n",
			graphic_handle_hdmi, current->pid, current->tgid);

		if (graphic_handle_hdmi) {
			_ini.handle   = graphic_handle_hdmi;
			_ini.mode = RT_GRAPHICS_MODE_IMAGE_OUTPUT;
#if _LOG_DBG >= 1
			dump_screen_grap_initialize(&_ini);
#endif
			rc = screen_graphics_initialize(&_ini);
			if (rc != SMAP_LIB_GRAPHICS_OK) {
				printk_err1("screen_graphics_initialize "
					"return by %d.\n", rc);

				work_deletehandle_hdmi(work);
			}
		} else {
			/* eror report */
			printk_err("graphic_handle_hdmi is NULL\n");
		}
#endif
	}

	TRACE_LEAVE(FUNC_WQ_CREATE_HDMI);
	DBGLEAVE("\n");
}
#endif

static void work_overlay(struct localwork *work)
{
	struct composer_rh *rh;
	TRACE_ENTER(FUNC_WQ_OVERLAY);
	DBGENTER("work:%p\n", work);

	rh = container_of(work, struct composer_rh, rh_wqtask_hdmi);

#ifdef CONFIG_MACH_KOTA2
	/* Kota2 not support graphic output. */
	process_composer_queue_callback(rh);
#elif SH_MOBILE_COMPOSER_SUPPORT_HDMI <= 1 || \
	!defined(RT_GRAPHICS_MODE_IMAGE_OUTPUT)
	/* HDMI type is 1. or RT-API not available. */
	process_composer_queue_callback(rh);
#else
	{
		int rc;
		int index;

		index = rh->data.extlayer_index;
		if (in_early_suspend) {
			printk_dbg2(1, "suspend state.\n");
			rc = CMP_NG;
			goto finish;
		} else if (graphic_handle_hdmi == NULL) {
			printk_err1("handle for HDMI not created.\n");
			rc = CMP_NG;
			goto finish;
		} else if (rtapi_hungup) {
			printk_err1("graphics system hungup.\n");
			rc = CMP_NG;
			goto finish;
		} else if (index < 0 || index > 3) {
			printk_err1("extlayer_index out of range.\n");
			rc = CMP_NG;
			goto finish;
		} else {
			screen_grap_image_output _out;
			struct composer_blendcommon *common;

			/* share common object */
			common = &rh->rh_wqcommon;

			_out.handle = graphic_handle_hdmi;
			_out.output_image            =
				rh->data.layer[index].image;
			_out.output_image.format     =
				rh->data.extlayer.image.format;
			_out.output_image.yuv_format =
				rh->data.extlayer.image.yuv_format;
			_out.output_image.yuv_range  =
				rh->data.extlayer.image.yuv_range;
			_out.rotate                  =
				rh->data.extlayer.rotate;
			_out.user_data = (unsigned long)rh;

#if _LOG_DBG >= 1
			dump_screen_grap_image_output(&_out);
#endif

			common->status  = 0;
			rc = screen_graphics_image_output(&_out);
			if (rc != SMAP_LIB_GRAPHICS_OK) {
				printk_err("screen_graphics_image_output "
					"return by %d.\n", rc);

				rc = CMP_NG;
				goto finish;
			}

			/* wait complete */
			rc = wait_event_interruptible_timeout(
				common->wait_notify, \
				common->status != 0, msecs_to_jiffies(500));
			if (rc < 0) {
				/* report error */
				printk_err("unexpectly wait_event "
					"interrupted by %d .\n", rc);
			} else if (rc == 0) {
				/* report error */
				printk_err1("not detect notify of output.\n");
			}
			rc = (common->status == 0) ? CMP_NG : CMP_OK;
		}
finish:
		printk_dbg1(2, "results rc:%d\n", rc);
		if (rc != CMP_OK) {
			/* report error */
			printk_err1("output result is error.\n");
		}

		/* process callback */
		process_composer_queue_callback(rh);
	}
#endif

	TRACE_LEAVE(FUNC_WQ_OVERLAY);
	DBGLEAVE("\n");
	return;
}
#endif

static void work_runblend(struct localwork *work)
{
	struct composer_blendcommon *common;
	int  rc = CMP_OK;
	{
		/*****************************************************
		It's necessary to make the task registered with work,
		should be a definition of the following structure.

		struct <work_task structur> {
			struct localwork             fh_wqtask;
			struct composer_blendcommon  fh_wqcommon;
		*****************************************************/
		struct composer_fh *fh;
		fh = container_of(work, struct composer_fh, fh_wqtask);
		common = &fh->fh_wqcommon;
	}
	TRACE_ENTER(FUNC_WQ_BLEND);
	DBGENTER("work:%p\n", work);
	printk_dbg1(1, "blending handle:%p in PID:%d TGID:%d\n",
		graphic_handle, current->pid, current->tgid);

	common->status  = 0;
#ifdef CONFIG_HAS_EARLYSUSPEND
	if (in_early_suspend) {
		printk_dbg2(1, "suspend state.\n");
		rc = CMP_NG;
		goto finish;
	}
#endif
	if (graphic_handle == NULL) {
		printk_err1("handle not created.\n");
		rc = CMP_NG;
		goto finish;
	}
	if (rtapi_hungup) {
		printk_err1("graphics system hungup.\n");
		rc = CMP_NG;
		goto finish;
	}
	(*common->_blend).handle = graphic_handle;

#if _LOG_DBG >= 1
	dump_screen_grap_image_blend(common->_blend);
#endif

	rc = screen_graphics_image_blend(common->_blend);
	if (rc != SMAP_LIB_GRAPHICS_OK) {
		printk_err("screen_graphics_image_blend return by %d.\n", rc);
		rc = CMP_NG;
		goto finish3;
	}

	rc = wait_event_interruptible_timeout(
		common->wait_notify, \
		common->status != 0, 1 * HZ);
	if (rc < 0) {
		/* report error */
		printk_err("unexpectly wait_event interrupted by %d .\n", rc);
	} else if (rc == 0) {
		/* report error */
		printk_err1("not detect notify of blending.\n");
	}
	rc = (common->status == 0) ? CMP_NG : CMP_OK;

finish3:
finish:
	(*common->_blend).handle = NULL;

	if (common->status != 1) {
		printk_err1("callback result is error.\n");
		rc = CMP_NG;
	}

	printk_dbg1(2, "results rc:%d\n", rc);

	common->callback(rc, common->user_data);
	TRACE_LEAVE(FUNC_WQ_BLEND);
	DBGLEAVE("\n");
	return;
}

/* blend finish requested by iocs_start */
static void callback_iocs_start(int result, void *user_data)
{
	struct composer_fh *fh = (struct composer_fh *) user_data;

	static const int fh_status_translate[] = {
		[IDX_OUTPLANE]  = FH_STATUS_OUTPLANE,
		[IDX_CH1LAYER]  = FH_STATUS_CH1LAYER,
		[IDX_CH2LAYER]  = FH_STATUS_CH2LAYER,
		[IDX_CH3LAYER]  = FH_STATUS_CH3LAYER,
		[IDX_CH4LAYER]  = FH_STATUS_CH4LAYER
	};

	DBGENTER("result:%d user_data:%p\n", result, user_data);

#if _LOG_DBG >= 2
	dump_screen_grap_image_param(&fh->grap_data._blend.output_image,
		"output_image");
#endif

	/* image blending finished. */
	printk_dbg2(3, "spinlock\n");
	spin_lock(&irqlock);

	fh->fh_status &= ~FH_STATUS_BLENDING;
	if (result != CMP_OK)
		fh->fh_status |= FH_STATUS_BLENDERR;

	spin_unlock(&irqlock);

	if (result == CMP_OK) {
		/* no error. set flag to complete blending. */
		int i;
		for (i = 0; i < ARRAY_SIZE(fh_status_translate); i++) {
			struct composer_info   *info;
			struct composer_buffer *buf;

			if ((fh->fh_status & fh_status_translate[i]) == 0) {
				/* not open */
				continue;
			}
			info = &fh->fh_info[i];
			buf = fh->fh_info[i].buffer;

			/* check buffer configuration */
			if (info->id & INFO_ID_FLG_NOTDISP) {
				/* not blending plane */
				continue;
			}

			/* update status, to set poll blend */
			allfile_status_set(buf, FH_STATUS_POLLBLEND);
		}
	}
	DBGLEAVE("\n");
}

static int  setbusy(struct composer_fh *fh)
{
	int rc, i;
	int lock_flag;
	unsigned long          start_jiffies;
	struct composer_buffer *buffer[5];
	struct composer_info   *info[5];
	int                    nBuffer;

	static const int fh_status_translate[] = {
		[IDX_OUTPLANE]  = FH_STATUS_OUTPLANE,
		[IDX_CH1LAYER]  = FH_STATUS_CH1LAYER,
		[IDX_CH2LAYER]  = FH_STATUS_CH2LAYER,
		[IDX_CH3LAYER]  = FH_STATUS_CH3LAYER,
		[IDX_CH4LAYER]  = FH_STATUS_CH4LAYER
	};

	DBGENTER("fh:%p\n", fh);
	printk_dbg2(3, "fh->fh_status: 0x%x\n", fh->fh_status);

	memset(buffer, 0, sizeof(buffer));
	memset(info,   0, sizeof(info));

	/* pickup-buffer */
	nBuffer = 0;
	for (i = 0; i < ARRAY_SIZE(fh_status_translate); i++) {
		if ((fh->fh_status & fh_status_translate[i]) == 0) {
			/* not open */
			continue;
		}
		/* check buffer configuration */
		if (fh->fh_info[i].id & INFO_ID_FLG_NOTDISP) {
			/* not blending plane */
			continue;
		}

		info[nBuffer]   = &fh->fh_info[i];
		buffer[nBuffer] = fh->fh_info[i].buffer;

		printk_dbg2(3, "buffer: %p\n", buffer[nBuffer]);
		nBuffer++;
	}
	printk_dbg2(3, "num of buffer to lock: %d\n", nBuffer);

	start_jiffies = jiffies;
	do {
		lock_flag = 0;
		rc = CMP_OK;
		/* try lock */
		for (i = 0; i < nBuffer; i++) {
			if (down_trylock(&buffer[i]->buf_sem)) {
				/* set return code */
				rc = CMP_NG;
			} else {
				/* set flag, buffer locked. */
				lock_flag |= (1<<i);
			}
		}
		printk_dbg2(3, "lock result: %d\n", rc);

		if (rc != 0) {
			/* unlock buffer */
			for (i = 0; i < nBuffer; i++) {
				if (lock_flag & (1<<i)) {
					/* try lock success.*/
					up(&buffer[i]->buf_sem);
				}
			}
			/* wait event to enable unlock */
			for (i = 0; i < nBuffer; i++) {
				int event_rc;
				if (lock_flag & (1<<i)) {
					/* try lock success.*/
					continue;
				}

				event_rc = wait_event_interruptible_timeout(
					fh->fh_wait,
					buffer[i]->buf_sem.count > 0,
					2 * HZ);

				if (event_rc < 0) {
					printk_err2("buffer lock interrupted "
						"%d\n", event_rc);
					goto err_exit;
				} else if (event_rc == 0) {
					/* report debug message */
					printk_dbg2(3, "buf_sem may "
						"not unlocked.\n");
				}
			}
			/* may be previous busy buffer free */
		}
	} while (rc != 0 && (int)(jiffies - start_jiffies) < 2 * HZ);

	printk_dbg2(3, "lock result: %d (0x%x)\n", rc, lock_flag);

	if (rc != 0) {
		printk_dbg2(3, "time-out. give up setbusy");
		goto err_exit;
	}

	for (i = 0; i < nBuffer; i++) {
		/* set flag to this buffer use to blending. */
		info[i]->id |= INFO_ID_FLG_BLENDING;
	}

#if _LOG_DBG >= 2
	if (2 < debug) {
		int semaphore_count[5];
		int buffer_id[5];

		memset(semaphore_count, 0, sizeof(semaphore_count));
		memset(buffer_id,      0, sizeof(buffer_id));

		for (i = 0; i < nBuffer; i++) {
			semaphore_count[i] = buffer[i]->buf_sem.count;
			buffer_id[i]       = info[i]->id;
		}
		printk_dbg(2, "lock_info: %d(0x%x) %d(0x%x) " \
			"%d(0x%x) %d(0x%x) %d(0x%x)\n",       \
			semaphore_count[0], buffer_id[0],     \
			semaphore_count[1], buffer_id[1],     \
			semaphore_count[2], buffer_id[2],     \
			semaphore_count[3], buffer_id[3],     \
			semaphore_count[4], buffer_id[4]);
	}
#endif
err_exit:
	DBGLEAVE("%d\n", rc);
	return rc;
}
static int setidle(struct composer_fh *fh)
{
	int rc, i;

	static const int fh_status_translate[] = {
		[IDX_OUTPLANE]  = FH_STATUS_OUTPLANE,
		[IDX_CH1LAYER]  = FH_STATUS_CH1LAYER,
		[IDX_CH2LAYER]  = FH_STATUS_CH2LAYER,
		[IDX_CH3LAYER]  = FH_STATUS_CH3LAYER,
		[IDX_CH4LAYER]  = FH_STATUS_CH4LAYER
	};

	DBGENTER("fh:%p\n", fh);
	printk_dbg2(3, "fh->fh_status: 0x%x\n", fh->fh_status);

	/* pickup-buffer */
	for (i = 0; i < ARRAY_SIZE(fh_status_translate); i++) {
		if ((fh->fh_status & fh_status_translate[i]) == 0) {
			/* not open */
			continue;
		}
		/* check buffer configuration */
		if (fh->fh_info[i].id & INFO_ID_FLG_NOTDISP) {
			/* not blending plane */
			continue;
		}

		if (fh->fh_info[i].id & INFO_ID_FLG_BLENDING) {
			fh->fh_info[i].id &= ~INFO_ID_FLG_BLENDING;
			unlock_buffer(fh, i);
		}
	}
	/* pass */
	rc = 0;
	DBGLEAVE("%d\n", rc);
	return rc;
}

#ifdef CONFIG_MISC_R_MOBILE_COMPOSER_REQUEST_QUEUE
static void timeout_queue_process_timerstart(void)
{
	unsigned long flags;
	DBGENTER("\n");

	printk_dbg2(3, "spinlock\n");
	spin_lock_irqsave(&irqlock_timer, flags);

	if (kernel_queue_timer.data == 0) {
		kernel_queue_timer.data    = 1;
		kernel_queue_timer.expires = jiffies + \
			msecs_to_jiffies(200);

		spin_unlock_irqrestore(&irqlock_timer, flags);

#if _LOG_DBG > 0
		if (debug) {
			/* time out extend to 1 minute. */
			kernel_queue_timer.expires += \
				msecs_to_jiffies(5000);
		}
#endif
		printk_dbg2(3, "add timer expires:%u current:%u\n",
			(int)kernel_queue_timer.expires,
			(int)jiffies);
#ifdef DEBUG_NO_USE_TIMER
		printk_dbg2(3, "ignore add_timer.\n");
#else
		add_timer(&kernel_queue_timer);
#endif
	} else {
		/* nothing to do */
		spin_unlock_irqrestore(&irqlock_timer, flags);
		printk_dbg2(3, "already start timer\n");
	}

	DBGLEAVE("\n");
}

static void timeout_queue_process_timercancel(void)
{
	unsigned long flags;
	DBGENTER("\n");

	if (kernel_queue_timer.data) {
		printk_dbg2(3, "spinlock\n");
		spin_lock_irqsave(&irqlock_timer, flags);

		printk_dbg2(3, "cancel timer\n");
		del_timer(&kernel_queue_timer);
		kernel_queue_timer.data = 0;

		spin_unlock_irqrestore(&irqlock_timer, flags);
	}

	if (!list_empty(&kernel_queue_top)) {
		printk_dbg2(3, "restart timer, there is pending request\n");

		timeout_queue_process_timerstart();
	}

	DBGLEAVE("\n");
}

static int  queue_fb_address_mapping(void)
{
	int rc = CMP_NG;

	DBGENTER("\n");

	if (queue_fb_map_handle) {
		/* already mapping */
		printk_dbg2(3, "already mapping finished\n");
		rc = CMP_OK;
	} else {
		queue_fb_map_handle = sh_mobile_rtmem_physarea_register(
			queue_fb_map_endaddress - queue_fb_map_address,
			queue_fb_map_address);

		if (queue_fb_map_handle) {
			printk_dbg1(2, "framebuffer map success.\n");
			rc = 0;
		} else {
			printk_err("can not map framebuffer.\n");
			rc = CMP_NG;
		}
	}

	DBGLEAVE("%d\n", rc);
	return  rc;
}

static unsigned char *composer_get_RT_address(unsigned char *address)
{
	unsigned long p_addr = (unsigned long)address;
	unsigned char *rt_addr = NULL;

	/* translate physical to RT address */
	rt_addr = (char *)sh_mobile_rtmem_conv_phys2rtmem(p_addr);

	if (rt_addr == NULL) {
		/* resolve conversioin by RT-API */
		if (graphic_handle == NULL) {
			/* currently not open handle */
			printk_dbg2(3, "not open rt-api handle\n");
		} else {
			system_mem_phy_change_rtaddr adr;
			int                          rc;

			adr.handle    = graphic_handle;
			adr.phys_addr = p_addr;
			adr.rtaddr    = 0;

			printk_dbg1(1, "system_memory_phy_change_rtaddr"
				"handle:%p phys_addr:0x%x\n",
					adr.handle, adr.phys_addr);

			rc = system_memory_phy_change_rtaddr(&adr);
			if (rc != SMAP_LIB_MEMORY_OK) {
				/* report error */
				printk_err("system_memory_phy_change_rtaddr"
					" return by %d.\n", rc);
			} else {
				rt_addr = (unsigned char *)adr.rtaddr;
			}
		}
	}

	printk_dbg2(2, "convert result 0x%lx to %p\n", p_addr, rt_addr);
	return rt_addr;
}

unsigned char *sh_mobile_composer_phy_change_rtaddr(unsigned long p_adr)
{
	unsigned char *rt_adr;

	rt_adr = composer_get_RT_address((unsigned char *) p_adr);

	return rt_adr;
}
EXPORT_SYMBOL(sh_mobile_composer_phy_change_rtaddr);

static int  composer_covert_queueaddress(screen_grap_image_blend *blend)
{
	unsigned char *rt_addr, *phys_addr;
	int rc = CMP_NG;
	int i;

	int                     num_layer;
	screen_grap_image_param * layer[5];

	DBGENTER("blend:%p\n", blend);

	num_layer = 1;
	layer[0] = &blend->output_image;

	for (i = 0; i < 4; i++) {
		if (blend->input_layer[i] == NULL)
			break;

		layer[num_layer] = &blend->input_layer[i]->image;
		num_layer++;
	}

	for (i = 0; i < num_layer; i++) {
		phys_addr = layer[i]->address;
		if (layer[i]->apmem_handle == NULL && phys_addr != NULL) {
			rt_addr   = composer_get_RT_address(phys_addr);
			if (rt_addr == NULL) {
				printk_err2("address %p convert error\n",
					phys_addr);
				goto err_exit;
			}
			layer[i]->address = rt_addr;
		}

		phys_addr = layer[i]->address_c0;
		if (layer[i]->apmem_handle_c0 == NULL && phys_addr != NULL) {
			rt_addr   = composer_get_RT_address(phys_addr);
			if (rt_addr == NULL) {
				printk_err2("address_c0 %p convert error\n",
					phys_addr);
				goto err_exit;
			}
			layer[i]->address_c0 = rt_addr;
		}

		phys_addr = layer[i]->address_c1;
		if (layer[i]->apmem_handle_c1 == NULL && phys_addr != NULL) {
			rt_addr   = composer_get_RT_address(phys_addr);
			if (rt_addr == NULL) {
				printk_err2("address_c1 %p convert error\n",
					phys_addr);
				goto err_exit;
			}
			layer[i]->address_c1 = rt_addr;
		}
	}
	/* all of address translation successed. */
	rc = 0;
err_exit:
	DBGLEAVE("\n");
	return rc;
}



static void composer_blendoverlay_errorcallback(
	struct composer_rh *rh)
{
	rh->refcount = 0;
	process_composer_queue_callback(rh);
}


int sh_mobile_composer_blendoverlay(unsigned long fb_physical)
{
	struct composer_rh     *blend_req = NULL;

	TRACE_ENTER(FUNC_BLEND);
	DBGENTER("fb_physical:0x%lx\n", fb_physical);

	printk_dbg2(3, "down\n");
	down(&kernel_queue_sem);

	if (!list_empty(&kernel_queue_top)) {
		struct composer_rh     *rh;
		struct list_head       *list;
		int                    count;
		int                    fb_size;

		fb_size = (queue_fb_map_endaddress-queue_fb_map_address)/2;

		/* search last list entry */
		count = 0;
		list_for_each(list, &kernel_queue_top) {
			struct composer_rh *rh;
			unsigned long phys_addr;

			rh = list_entry(list, struct composer_rh, list);
			count++;

			if (count > MAX_KERNELREQ) {
				/* avooid for ever loop. */
				break;
			}
			phys_addr = sh_mobile_rtmem_conv_rt2physmem(
			(unsigned long)rh->data.blend.output_image.address);

			printk_dbg2(3, "list%d: phys addr:0x%lx.\n",
				count, phys_addr);

			if (phys_addr >= fb_physical &&
				phys_addr < fb_physical + fb_size)
				blend_req = rh;
		}

		printk_dbg2(3, "found overlay request: %p.\n", blend_req);

		/* remove not recently request. */
		if (blend_req) {
			while (!list_empty(&kernel_queue_top)) {

				/* get first entry */
				rh = list_first_entry(&kernel_queue_top,
					struct composer_rh, list);

				if (rh == blend_req)
					break;

				/* remove old request. */
				printk_err("remove request "
					"that is not processing.\n");
				printk_dbg2(3, "list %p is removed.\n", rh);

				list_del_init(&rh->list);

				/* process callback */
				up(&kernel_queue_sem);

				composer_blendoverlay_errorcallback(rh);

				printk_dbg2(3, "down\n");
				down(&kernel_queue_sem);
			}
			/* remove list */
			printk_dbg2(3, "list %p is removed.\n", blend_req);

			list_del_init(&blend_req->list);
		} else if (count >= 2) {
			printk_err("address 0x%lx is not found in list\n",
				fb_physical);

			while (!list_empty(&kernel_queue_top)) {

				rh = list_first_entry(&kernel_queue_top,
					struct composer_rh, list);

				/* remove list */
				printk_dbg2(3, "list %p is removed.\n", rh);

				list_del_init(&rh->list);

				/* process callback */
				up(&kernel_queue_sem);

				composer_blendoverlay_errorcallback(rh);

				printk_dbg2(3, "down\n");
				down(&kernel_queue_sem);
			}
		}
	}
	up(&kernel_queue_sem);

#if SH_MOBILE_COMPOSER_SUPPORT_HDMI
	/* confirm that the graphics handle for HDMI need release. */
	if (graphic_handle_hdmi) {
		if (blend_req == NULL ||
			blend_req->data.extlayer_index < 0) {
			/* queue task. */
			if (localworkqueue_queue(workqueue,
				&del_graphic_handle_hdmi)) {

				/* wait work compete. */
				localworkqueue_flush(workqueue,
					&del_graphic_handle_hdmi);
			} else {
				printk_err("failed to release graphic "
					"handle for hdmi\n");
			}
		}
	} else {
		if (blend_req &&
			blend_req->data.extlayer_index >= 0) {
			/* queue task. */
			if (localworkqueue_queue(workqueue,
				&init_graphic_handle_hdmi)) {
				/* not wait complete */
				printk_dbg1(2, "request create graphics "
					"handle for hdmi\n");
			} else {
				printk_err("failed to create graphic "
					"handle for hdmi\n");
			}
		}
	}
#endif

	/* blend image */
	if (blend_req) {
		/* cancel timer */
		timeout_queue_process_timercancel();

		if (localworkqueue_queue(workqueue, &blend_req->rh_wqtask)) {
			printk_dbg2(3, "success to queue requests.\n");

			/* wait task complete */
			localworkqueue_flush(workqueue, &blend_req->rh_wqtask);
		} else {
			printk_err("can not queue requests.");

			/* process callback */
			composer_blendoverlay_errorcallback(blend_req);
			blend_req = NULL;
		}
	}
#if SH_MOBILE_COMPOSER_SUPPORT_HDMI
	/* overlay HDMI image */
	if (blend_req && blend_req->data.extlayer_index >= 0) {
		if (localworkqueue_queue(workqueue,
			&blend_req->rh_wqtask_hdmi)) {
			printk_dbg2(3, "success to queue hdmi requests.\n");

			/* overlay no need wait complete. */
			/* temporally wait comple.        */
			localworkqueue_flush(workqueue,
				&blend_req->rh_wqtask_hdmi);
		} else {
			printk_err("can not queue requests.");

			/* process callback */
			composer_blendoverlay_errorcallback(blend_req);
			blend_req = NULL;
		}
	}
#endif
#if SH_MOBILE_COMPOSER_WAIT_DRAWEND
	current_overlayrequest = blend_req;
#endif

	TRACE_LEAVE(FUNC_BLEND);
	DBGLEAVE("\n");
	return 0;
}
EXPORT_SYMBOL(sh_mobile_composer_blendoverlay);

#if SH_MOBILE_COMPOSER_SUPPORT_HDMI
int sh_mobile_composer_hdmiset(int mode)
{
	int rc = CMP_OK;
	TRACE_ENTER(FUNC_HDMISET);
	DBGENTER("mode:%d\n", mode);

	printk_dbg2(3, "down\n");
	down(&sem);

	if (mode == 0) {
		/* confirm that the graphics handle for HDMI need release. */
		if (graphic_handle_hdmi) {
			/* queue task. */
			if (localworkqueue_queue(workqueue,
				&del_graphic_handle_hdmi)) {

				/* wait work compete. */
				localworkqueue_flush(workqueue,
					&del_graphic_handle_hdmi);
			} else {
				printk_err("failed to release graphic "
					"handle for hdmi\n");
			}
		}

		if (graphic_handle_hdmi) {
			printk_dbg2(3, "release graphic handle failed.\n");
			rc = CMP_NG;
		}
		TRACE_LOG1(FUNC_HDMISET, rc);
	}

	up(&sem);

	DBGLEAVE("rc:%d\n", rc);
	TRACE_LEAVE(FUNC_HDMISET);
	return rc;
}
EXPORT_SYMBOL(sh_mobile_composer_hdmiset);
#endif

#if SH_MOBILE_COMPOSER_WAIT_DRAWEND
void sh_mobile_composer_notifyrelease(void)
{
	struct composer_rh *rh = current_overlayrequest;
	DBGENTER("\n");

	overlay_draw_complete = 1;
	if (rh) {
#if _LOG_DBG >= 1
		if (rh->refcount != 1) {
			/* error report */
			printk_err1("buffer refcount not 1. current:%d",
				rh->refcount);
		}
#endif
		current_overlayrequest = NULL;

		/* process callback */
		process_composer_queue_callback(rh);
	} else {
		/* wake-up waiting thread */
		wake_up(&kernel_waitqueue_comp);
	}
	DBGLEAVE("\n");
}
EXPORT_SYMBOL(sh_mobile_composer_notifyrelease);
#endif

int sh_mobile_composer_queue(
	void *data,
	int   data_size,
	void  (*callback)(void *user_data, int result),
	void   *user_data)
{
	int i;
	int rc = -1;
	struct composer_rh *rh;
	TRACE_ENTER(FUNC_QUEUE);
	DBGENTER("data:%p data_size:%d callback:%p user_data:%p\n",
		data, data_size, callback, user_data);

#if _LOG_DBG >= 1
	if (debug > 2 && data && data_size < 512) {
		int i;
		char msg[256];
		const unsigned char *ptr = (unsigned char *)data;
		printk_dbg1(2, "data in address");
		for (i = 0; i < data_size; i += 16) {
			dump_address_data_to_string(&msg[0], ptr+i);
			printk_dbg1(2, "  %s\n", msg);
		}
	}
#endif

	if (data_size != sizeof(struct cmp_request_queuedata)) {
		printk_err("size of struct cmp_request_queuedata not match.\n");
		goto err_exit;
	}

	if (rtapi_hungup) {
		printk_err1("graphic system hung-up.\n");
		goto err_exit;
	}
#ifdef CONFIG_HAS_EARLYSUSPEND
	if (in_early_suspend) {
		printk_dbg2(2, "now early-suspending.\n");
		/* nothing to do */
		goto err_exit;
	}
#endif

	/* cancel timer */
	timeout_queue_process_timercancel();

	rh = NULL;
	printk_dbg2(3, "down\n");
	down(&kernel_queue_sem);
	for (i = 0; i < MAX_KERNELREQ; i++) {
		if (kernel_request[i].active == 0 &&
			list_empty(&kernel_request[i].list)) {
			printk_dbg2(3, "use request buffer index:%d\n", i);
			rh = &kernel_request[i];
			rh->active = 1;
			break;
		}
	}
	up(&kernel_queue_sem);

	if (rh == NULL) {
		printk_err2("no space left to request blending.");
		goto err_exit;
	}

	/* copy arguments */
	memcpy(&rh->data, data, sizeof(struct cmp_request_queuedata));
	rh->user_callback  = callback;
	rh->user_data      = user_data;
#if SH_MOBILE_COMPOSER_SUPPORT_HDMI
	if (rh->data.extlayer_index < 0) {
		/* not use external layer */
#endif
		rh->refcount = 1 + SH_MOBILE_COMPOSER_WAIT_DRAWEND;
#if SH_MOBILE_COMPOSER_SUPPORT_HDMI
	} else {
		/* use external layer */
		rh->refcount = 2 + SH_MOBILE_COMPOSER_WAIT_DRAWEND;
	}
#endif

	/* initialize pointer */
	{
		screen_grap_image_blend *blend = &rh->data.blend;
		blend->handle = NULL;
		for (i = 0; i < 4; i++) {
			if (blend->input_layer[i])
				blend->input_layer[i] = &rh->data.layer[i];
		}
		blend->user_data = (unsigned long)&rh->rh_wqcommon;
	}

	/* address translation */
	{
		screen_grap_image_blend *blend = &rh->data.blend;

		if (composer_covert_queueaddress(blend) != CMP_OK) {
			printk_err2("address translation failed.");
			rh->active = 0;
			goto err_exit;
		}
	}

	/* simple data check to confirm valid data. */
	{
		screen_grap_image_blend *blend = &rh->data.blend;
		int  chk_flag = (1<<RT_GRAPHICS_COLOR_YUV422SP) | \
				(1<<RT_GRAPHICS_COLOR_YUV420SP) | \
				(1<<RT_GRAPHICS_COLOR_RGB565)   | \
				(1<<RT_GRAPHICS_COLOR_RGB888)   | \
				(1<<RT_GRAPHICS_COLOR_ARGB8888) | \
				(1<<RT_GRAPHICS_COLOR_YUV420PL) | \
				(1<<RT_GRAPHICS_COLOR_XRGB8888);
		int           format;
		unsigned char *address;

#ifdef RT_GRAPHICS_COLOR_ABGR8888
		chk_flag |= (1 << RT_GRAPHICS_COLOR_ABGR8888);
#endif
#ifdef RT_GRAPHICS_COLOR_XBGR8888
		chk_flag |= (1 << RT_GRAPHICS_COLOR_XBGR8888);
#endif
		format  = blend->output_image.format;
		address = blend->output_image.address;
		if (format > 30 ||
		    (chk_flag & (1<<format)) == 0) {
			printk_err("format %d un-expected.\n", format);
			rh->active = 0;
			goto err_exit;
		}
		if (address == NULL) {
			printk_err2("address NULL un-expected.\n");
			rh->active = 0;
			goto err_exit;
		}

		for (i = 0; i < 4; i++) {
			if (blend->input_layer[i] == NULL)
				break;

			format  = rh->data.layer[i].image.format;
			address = rh->data.layer[i].image.address;
			if (format > 30 ||
			    (chk_flag & (1<<format)) == 0) {
				printk_err("layer %d format %d un-expected.\n",
					i, format);
				rh->active = 0;
				goto err_exit;
			}
			if (address == NULL) {
				printk_err2("layer %d address NULL "
					"un-expected.\n", i);
				rh->active = 0;
				goto err_exit;
			}
		}
	}

	printk_dbg2(3, "down\n");
	down(&kernel_queue_sem);

	list_add_tail(&rh->list, &kernel_queue_top);
	rc = 0;

	up(&kernel_queue_sem);

	/* start timer */
	timeout_queue_process_timerstart();

err_exit:
	if (rc) {
		/* error detected. */
		TRACE_ENTER(FUNC_CALLBACK);
		callback(user_data, 1);
		TRACE_LEAVE(FUNC_CALLBACK);
	}

	TRACE_LEAVE(FUNC_QUEUE);
	DBGLEAVE("%d\n", rc);
	return rc;
}
EXPORT_SYMBOL(sh_mobile_composer_queue);

static void process_composer_queue_callback(struct composer_rh *rh)
{
	void   (*user_callback)(void*, int);
	void   *user_data;

	TRACE_ENTER(FUNC_CALLBACK);
	DBGENTER("rh:%p\n", rh);

	user_callback = NULL;
	user_data     = NULL;

	printk_dbg2(3, "down\n");
	down(&kernel_queue_sem);

	rh->refcount--;
	printk_dbg2(3, "refcount:%d\n", rh->refcount);
	if (rh->refcount <= 0) {
		user_callback = rh->user_callback;
		user_data     = rh->user_data;
		rh->user_callback = NULL;
		rh->active = 0;

		/* wake-up waiting thread */
		wake_up(&kernel_waitqueue_comp);
	}
	up(&kernel_queue_sem);

	if (user_callback) {
		TRACE_LOG(FUNC_CALLBACK);
		user_callback(user_data, 1);
		/* process callback */
	}
	TRACE_LEAVE(FUNC_CALLBACK);
	DBGLEAVE("\n");
}


static void callback_composer_queue(int result, void *user_data)
{
	struct composer_rh *rh = (struct composer_rh *) user_data;
	DBGENTER("result:%d user_data:%p\n", result, user_data);

	TRACE_LOG1(FUNC_CALLBACK, result);
	if (result != CMP_OK) {
		/* error report */
		printk_err("composer error %d, %d, %d\n",
			in_early_suspend,
			(graphic_handle == NULL),
			rtapi_hungup);
	}
	process_composer_queue_callback(rh);

	DBGLEAVE("\n");
}


static void timeout_queue_process(unsigned long data)
{
	int rc;
	DBGENTER("data:%ld\n", data);

	printk_err("detect timeout.\n");

	rc = localworkqueue_queue(workqueue,
		&expire_kernel_request);
	if (rc == 0) {
		/* report error */
		printk_err("can not queue work of expire_kernel_request.\n");
	}

	DBGLEAVE("\n");
}
#endif

#if INTERNAL_DEBUG >= 1
static void tracelog_record(int logclass, int line, int ID, int val)
{
	unsigned long flags;

	spin_lock_irqsave(&log_irqlock, flags);
	log_tracebuf[log_tracebuf_wp][0] = (logclass<<24) | (line);
	log_tracebuf[log_tracebuf_wp][1] = ID;
	log_tracebuf[log_tracebuf_wp][2] = val;
	log_tracebuf_wp = (log_tracebuf_wp+1) & (TRACELOG_SIZE-1);
	spin_unlock_irqrestore(&log_irqlock, flags);
}

static int tracelog_create_logmessage(char *p, int n)
{
	int i, rp;
	int c;
	char *p_org = p;
	unsigned long flags;

	spin_lock_irqsave(&log_irqlock, flags);
	rp = (log_tracebuf_wp) & (TRACELOG_SIZE-1);
	for (i = 0; i < TRACELOG_SIZE; i++) {
		int logclass = log_tracebuf[rp][0]>>24;
		int logline  = log_tracebuf[rp][0] & 0xffffff;
		switch (logclass) {
		case ID_TRACE_ENTER:
			c = snprintf(p, n, "[0x%03x:ent:%d]",
				log_tracebuf[rp][1], logline);
			break;
		case ID_TRACE_LEAVE:
			c = snprintf(p, n, "[0x%03x:lev:%d]",
				log_tracebuf[rp][1], logline);
			break;
		case ID_TRACE_LOG:
			c = snprintf(p, n, "[0x%03x:%d]",
				log_tracebuf[rp][1], logline);
			break;
		case ID_TRACE_LOG1:
			c = snprintf(p, n, "[0x%03x:%d:%d]",
				log_tracebuf[rp][1], logline,
				log_tracebuf[rp][2]);
			break;
		default:
			/* no log message */
			c = 0;
			break;
		}
		if (c < n) {
			p += c;
			n -= c;
		}
		rp = (rp+1) & (TRACELOG_SIZE-1);
	}
	spin_unlock_irqrestore(&log_irqlock, flags);

	return p - p_org;
}

static void internal_debug_create_message(struct composer_fh *fh)
{
	char *p = internal_log_msg;
	int  n  = internal_log_msgsize;
	int  c;
#ifdef CONFIG_MISC_R_MOBILE_COMPOSER_REQUEST_QUEUE
	int  i;
#endif
	int  log_type  = (internal_log_seqid >> 8) & 0xff;
	int  log_index =  internal_log_seqid       & 0xff;

	if (p == NULL) {
		internal_log_seqid = SEQID_NOMORE_LOG;
		goto err_exit;
	}
	/* record sequence */
	if (internal_log_seqid == -1) {
		/* set next logtype */
		internal_log_seqid = 0 << 8;
	} else if (log_type == 0) {
		/* log of static variable */
		if (log_index == 0) {
			c = snprintf(p, n, "[static]\n");
			if (c < n) {
				p += c;
				n -= c;
			}
		}

		c = snprintf(p, n, "  semaphore sem:%d\n",
			sem.count);
		if (c < n) {
			p += c;
			n -= c;
		}
#ifdef CONFIG_MISC_R_MOBILE_COMPOSER_REQUEST_QUEUE
		c = snprintf(p, n, "  semaphore kernel_queue_sem:%d\n",
			kernel_queue_sem.count);
		if (c < n) {
			p += c;
			n -= c;
		}
#endif
		c = snprintf(p, n, "  num_open:%d\n",
			num_open);
		if (c < n) {
			p += c;
			n -= c;
		}
		c = snprintf(p, n, "  debug:%d\n",
			debug);
		if (c < n) {
			p += c;
			n -= c;
		}
		c = snprintf(p, n, "  rtapi_hungup:%d\n",
			rtapi_hungup);
		if (c < n) {
			p += c;
			n -= c;
		}
#ifdef CONFIG_HAS_EARLYSUSPEND
		c = snprintf(p, n, "  in_early_suspend:%d\n",
			in_early_suspend);
		if (c < n) {
			p += c;
			n -= c;
		}
#endif
		c = snprintf(p, n, "  graphic_handle:%p\n",
			graphic_handle);
		if (c < n) {
			p += c;
			n -= c;
		}
		c = snprintf(p, n, "  workqueue:%p\n", workqueue);
		if (c < n) {
			p += c;
			n -= c;
		}
		if (workqueue) {
			c = snprintf(p, n, "  ->top:%s\n",
				list_empty(&workqueue->top) ? "idle" : "busy");
			if (c < n) {
				p += c;
				n -= c;
			}
		}
#ifdef CONFIG_MISC_R_MOBILE_COMPOSER_REQUEST_QUEUE
		c = snprintf(p, n, "  fb_map_address\n");
		if (c < n) {
			p += c;
			n -= c;
		}
		c = snprintf(p, n, "  ->physical_address 0x%lx-0x%lx\n",
			queue_fb_map_address, queue_fb_map_endaddress);
		if (c < n) {
			p += c;
			n -= c;
		}
		c = snprintf(p, n, "  ->queue_fb_map_handle %p\n",
			queue_fb_map_handle);
		if (c < n) {
			p += c;
			n -= c;
		}
#if SH_MOBILE_COMPOSER_WAIT_DRAWEND
		c = snprintf(p, n, "  current_overlayrequest:%p\n",
			current_overlayrequest);
		if (c < n) {
			p += c;
			n -= c;
		}
		c = snprintf(p, n, "  overlay_draw_complete:%d\n",
			overlay_draw_complete);
		if (c < n) {
			p += c;
			n -= c;
		}
#endif /* SH_MOBILE_COMPOSER_WAIT_DRAWEND */
#endif
		/* set next logtype */
		internal_log_seqid = 1 << 8;
	} else if (log_type == 1) {
		/* log of queue */
#ifdef CONFIG_MISC_R_MOBILE_COMPOSER_REQUEST_QUEUE
		struct composer_rh *rh;

		if (log_index == 0) {
			c = snprintf(p, n, "[queue]\n");
			if (c < n) {
				p += c;
				n -= c;
			}
		}

		if (log_index < MAX_KERNELREQ) {
			/* set pointer */
			rh = &kernel_request[log_index];
		} else {
			/* clear pointer */
			rh = NULL;
		}

		if (rh) {
			c = snprintf(p, n, "  kernel_request[%d]:%p\n",
				log_index, rh);
			if (c < n) {
				p += c;
				n -= c;
			}

			c = snprintf(p, n, "  ->rh_wqcommon.status:%d\n",
				rh->rh_wqcommon.status);
			if (c < n) {
				p += c;
				n -= c;
			}

			c = snprintf(p, n, "  ->active:%d\n", rh->active);
			if (c < n) {
				p += c;
				n -= c;
			}

			c = snprintf(p, n, "  ->user_data:%p\n", rh->user_data);
			if (c < n) {
				p += c;
				n -= c;
			}

			c = snprintf(p, n, "  ->user_callback:%p\n",
				rh->user_callback);
			if (c < n) {
				p += c;
				n -= c;
			}

			c = snprintf(p, n, "  ->refcount:%d\n", rh->refcount);
			if (c < n) {
				p += c;
				n -= c;
			}

			c = snprintf(p, n, "  ->data\n");
			if (c < n) {
				p += c;
				n -= c;
			}
			for (i = 0; i < sizeof(rh->data); i++) {
				unsigned char *_data;

				_data = (unsigned char *)&rh->data;
				if ((i & 15) == 0) {
					c = snprintf(p, n, "    ");
					if (c < n) {
						p += c;
						n -= c;
					}
				}
				c = snprintf(p, n, "%02x", _data[i]);
				if (c < n) {
					p += c;
					n -= c;
				}

				if ((i & 15) == 15)
					c = snprintf(p, n, "\n");
				else
					c = snprintf(p, n, " ");

				if (c < n) {
					p += c;
					n -= c;
				}
			}
			if (i & 15) {
				c = snprintf(p, n, "\n");
				if (c < n) {
					p += c;
					n -= c;
				}
			}
		}

		/* set next logtype */
		if (rh)
			internal_log_seqid++;
		else
			internal_log_seqid = 2 << 8;
#else
		/* set next logtype */
		internal_log_seqid = 2 << 8;
#endif
	} else if (log_type == 2) {
		/* state of task for local workqueue */
		if (log_index == 0) {
			c = snprintf(p, n, "[task]\n");
			if (c < n) {
				p += c;
				n -= c;
			}
		}

		c = snprintf(p, n, "  del_graphic_handle\n");
		if (c < n) {
			p += c;
			n -= c;
		}
		c = snprintf(p, n, "  ->link:%s\n"
			"  ->status:%d\n",
			(list_empty(&del_graphic_handle.link) ? \
				"idle" : "busy"),
			del_graphic_handle.status);
		if (c < n) {
			p += c;
			n -= c;
		}
		c = snprintf(p, n, "  init_graphic_handle\n");
		if (c < n) {
			p += c;
			n -= c;
		}
		c = snprintf(p, n, "  ->link:%s\n"
			"  ->status:%d\n",
			(list_empty(&init_graphic_handle.link) ? \
				"idle" : "busy"),
			init_graphic_handle.status);
		if (c < n) {
			p += c;
			n -= c;
		}
#ifdef CONFIG_MISC_R_MOBILE_COMPOSER_REQUEST_QUEUE
#if SH_MOBILE_COMPOSER_SUPPORT_HDMI
		c = snprintf(p, n, "  del_graphic_handle_hdmi\n");
		if (c < n) {
			p += c;
			n -= c;
		}
		c = snprintf(p, n, "  ->link:%s\n"
			"  ->status:%d\n",
			(list_empty(&del_graphic_handle_hdmi.link) ? \
				"idle" : "busy"),
			del_graphic_handle_hdmi.status);
		if (c < n) {
			p += c;
			n -= c;
		}
		c = snprintf(p, n, "  init_graphic_handle_hdmi\n");
		if (c < n) {
			p += c;
			n -= c;
		}
		c = snprintf(p, n, "  ->link:%s\n"
			"  ->status:%d\n",
			(list_empty(&init_graphic_handle_hdmi.link) ? \
				"idle" : "busy"),
			init_graphic_handle_hdmi.status);
		if (c < n) {
			p += c;
			n -= c;
		}
#endif
		for (i = 0; i < MAX_KERNELREQ; i++) {
			struct composer_rh *rh = &kernel_request[i];

			c = snprintf(p, n, "  kernel_request[%d]\n", i);
			if (c < n) {
				p += c;
				n -= c;
			}
			c = snprintf(p, n, "    rh_wqtask\n");
			if (c < n) {
				p += c;
				n -= c;
			}
			c = snprintf(p, n, "    ->link:%s\n"
				"    ->status:%d\n",
				(list_empty(&rh->rh_wqtask.link) ? \
					"idle" : "busy"),
				rh->rh_wqtask.status);
			if (c < n) {
				p += c;
				n -= c;
			}
			c = snprintf(p, n, "    rh_wqtask_hdmi\n");
			if (c < n) {
				p += c;
				n -= c;
			}
			c = snprintf(p, n, "    ->link:%s\n"
				"    ->status:%d\n",
				(list_empty(&rh->rh_wqtask_hdmi.link) ? \
					"idle" : "busy"),
				rh->rh_wqtask_hdmi.status);
			if (c < n) {
				p += c;
				n -= c;
			}
		}
#endif

		/* set next logtype */
		internal_log_seqid = 3 << 8;
	} else if (log_type == 3) {
		/* trace log */
		c = snprintf(p, n, "[TRACELOG]\n");
		if (c < n) {
			p += c;
			n -= c;
		}
		c = tracelog_create_logmessage(p, n);
		if (c < n) {
			p += c;
			n -= c;
		}
		c = snprintf(p, n, "\n");
		if (c < n) {
			p += c;
			n -= c;
		}
		/* set next logtype */
		internal_log_seqid = 4<<8;
	} else {
		/* end of debug log */
		internal_log_seqid = SEQID_NOMORE_LOG;
		p = internal_log_msg;
	}

err_exit:
	internal_log_length = p - internal_log_msg;
	internal_log_remain = internal_log_length;
}
#endif

/******************************************************/
/* file operation entry function                      */
/******************************************************/
static ssize_t core_read(struct file *filp, char __user *buf, \
		size_t sz, loff_t *off)
{
	int rc = -EIO;
#if INTERNAL_DEBUG >= 1
	struct composer_fh     *fh;
	char   *msg, *p;
	int    n,    c;
#endif

	DBGENTER("filp:%p buf:%p sz:%d off:%p\n", filp, buf, sz, off);

#if INTERNAL_DEBUG >= 1
	fh = (struct composer_fh *)filp->private_data;

	/* allocate temporary memory. */
	msg = kmalloc(sz, GFP_KERNEL);
	if (msg == NULL || internal_log_msg == NULL) {
		printk_err2("memory allocatioin failed.\n");
		rc = -ENOMEM;
		goto err_exit;
	}

	/* initialize */
	p = &msg[0];
	n = sz;

	if (off != NULL && *off == 0) {
		/* reset sequence number */
		internal_log_seqid = -1;
	}

	while (n > 0) {
		if (internal_log_remain) {
			/* append message */
			char *src = internal_log_msg;
			c = min(n, internal_log_remain);
			src += internal_log_length - internal_log_remain;

			if (c) {
				memcpy(p, src, c);
				internal_log_remain -= c;
				p                   += c;
				n                   -= c;
				continue;
			}
		} else if  (internal_log_seqid == SEQID_NOMORE_LOG) {
			/* no more log */
			break;
		} else {
			/* create message */
			internal_debug_create_message(fh);
		}
	}

	rc = p-msg;
	if (rc) {
		if (copy_to_user(buf, msg, rc)) {
			printk_err2("fail in copy_to_user\n");
			rc = -EINVAL;
		} else if (off) {
			/* increase offset */
			*off += rc;
		}
	}
err_exit:
	if (msg) {
		/* free temporary memory. */
		kfree(msg);
	}
#endif
	DBGLEAVE("%d\n", rc);
	return rc;
}

static unsigned int core_poll(struct file *filep, \
		struct poll_table_struct *poll)
{
	int rc;
	struct composer_fh     *fh;

	DBGENTER("filep:%p poll:%p\n", filep, poll);

	fh = (struct composer_fh *)filep->private_data;

	printk_dbg2(3, "current file:%p fh_status:0x%x\n", fh, fh->fh_status);

	if (!(fh->fh_status & (FH_STATUS_POLLBLEND | FH_STATUS_POLLADDR))) {
		/* wait event */
		poll_wait(filep, &fh->fh_wait, poll);
	}

	rc = 0;

	printk_dbg2(3, "spinlock\n");
	spin_lock(&irqlock);

	if (fh->fh_status & FH_STATUS_POLLBLEND) {
		rc |= POLLOUT | POLLWRNORM;
		fh->fh_status &= ~FH_STATUS_POLLBLEND;
	}
	if (fh->fh_status & FH_STATUS_POLLADDR) {
		rc |= POLLIN | POLLRDNORM;
		fh->fh_status &= ~FH_STATUS_POLLADDR;
	}

	spin_unlock(&irqlock);

	printk_dbg2(3, "poll result file:%p fh_status:0x%x rc:0x%x\n",
		fh, fh->fh_status, rc);

	DBGLEAVE("%d\n", rc);
	return rc;
}

static long core_ioctl(struct file *filep, \
		unsigned int cmd, unsigned long arg)
{
	int rc  = -EINVAL;
	int dir = _IOC_DIR(cmd);
	int sz  = _IOC_SIZE(cmd);
	struct composer_fh *fh;
	static unsigned long _arg_area[64];
	void   *parg;

	DBGENTER("filep:%p cmd:0x%x arg:0x%lx\n", filep, cmd, arg);

/********************/
/* ProLoge of IOCTL */
/********************/
	if (sz != 0 && ((dir & (_IOC_WRITE|_IOC_READ)) != 0)) {
		if (sz >= sizeof(_arg_area)) {
			printk_err2("ioctl argument size too large\n");
			goto err_exit;
		}
	}

	if (sz != 0 && (dir & _IOC_WRITE) != 0) {
		parg = &_arg_area[0];
		printk_dbg2(3, "copy_from_user\n");
		if (copy_from_user(parg, (void __user *)arg, sz)) {
			printk_err2("fail in copy_from_user\n");
			goto err_exit;
		}
	}

	fh = (struct composer_fh *)filep->private_data;
	parg = &_arg_area[0];

	printk_dbg2(3, "down\n");
	down(&fh->fh_sem);

	switch (cmd) {
	case CMP_IOC_ISLAYEX:
		rc = ioc_islayex(fh);
		break;
	case CMP_IOCG_SUPPORTPIXFMT:
		rc = iocg_suppotpixfmt(fh, parg);
		break;
	case CMP_IOCS_OPEN:
		rc = iocs_open(fh, parg);
		break;
	case CMP_IOCS_CLOSE:
		rc = iocs_close(fh, parg);
		break;
	case CMP_IOCS_DISP:
		rc = iocs_disp(fh, parg);
		break;
	case CMP_IOCS_ALPHA:
		rc = iocs_alpha(fh, parg);
		break;
	case CMP_IOCG_ALPHA:
		rc = iocg_alpha(fh, parg);
		break;
	case CMP_IOCS_KEYCOLOR:
		rc = iocs_keycolor(fh, parg);
		break;
	case CMP_IOCG_KEYCOLOR:
		rc = iocg_keycolor(fh, parg);
		break;
	case CMP_IOCS_LAYSIZE:
		rc = iocs_laysize(fh, parg);
		break;
	case CMP_IOCG_LAYSIZE:
		rc = iocg_laysize(fh, parg);
		break;
	case CMP_IOCS_PIXFMT:
		rc = iocs_pixfmt(fh, parg);
		break;
	case CMP_IOCG_PIXFMT:
		rc = iocg_pixfmt(fh, parg);
		break;
	case CMP_IOCS_CROPSIZE:
		rc = iocs_cropsize(fh, parg);
		break;
	case CMP_IOCG_CROPSIZE:
		rc = iocg_cropsize(fh, parg);
		break;
	case CMP_IOCS_CROPPOS:
		rc = iocs_croppos(fh, parg);
		break;
	case CMP_IOCG_CROPPOS:
		rc = iocg_croppos(fh, parg);
		break;
	case CMP_IOCS_POS:
		rc = iocs_pos(fh, parg);
		break;
	case CMP_IOCG_POS:
		rc = iocg_pos(fh, parg);
		break;
	case CMP_IOCS_SIZE:
		rc = iocs_size(fh, parg);
		break;
	case CMP_IOCG_SIZE:
		rc = iocg_size(fh, parg);
		break;
	case CMP_IOCS_VIEWLAY:
		rc = iocs_viewlay(fh, parg);
		break;
	case CMP_IOCS_BACKCOLOR:
		rc = iocs_backcolor(fh, parg);
		break;
	case CMP_IOCG_BACKCOLOR:
		rc = iocg_backcolor(fh, parg);
		break;
	case CMP_IOCS_LAYADDR:
		rc = iocs_layaddr(fh, parg);
		break;
	case CMP_IOCG_LAYADDR:
		rc = iocg_layaddr(fh, parg);
		break;
	case CMP_IOC_START:
		rc = ioc_start(fh);
		break;
	case CMP_IOC_ISSUSPEND:
		rc = ioc_issuspend(fh);
		break;
	case CMP_IOC_WAITCOMP:
		rc = ioc_waitcomp(fh);
		break;
	case CMP_IOC_WAITDRAW:
		rc = ioc_waitdraw(fh, parg);
		break;
	default:
		printk_err2("invalid cmd 0x%x\n", cmd);
	}

	up(&fh->fh_sem);

/********************/
/* EpiLoge of IOCTL */
/********************/
	if (rc == 0 && sz != 0 && (dir & _IOC_READ) != 0) {
		parg = &_arg_area[0];
		printk_dbg2(3, "copy_to_user\n");
		if (copy_to_user((void __user *)arg, parg, sz)) {
			printk_err2("fail in copy_to_user\n");
			rc = -EINVAL;
		}
	}
err_exit:

	DBGLEAVE("%d\n", rc);
	return rc;
}

static int core_open(struct inode *inode, struct file *filep)
{
	int rc = 0;
	struct composer_fh *private_fh;

	TRACE_ENTER(FUNC_OPEN);
	DBGENTER("inode:%p filep:%p\n", inode, filep);
	printk_dbg2(3, "down\n");
	down(&sem);
	if (num_open >= MAX_OPEN) {
		/* set return code */
		printk_err("reach the upper limit of open\n");
		rc = -ENODEV;
	} else {
		if (num_open == 0) {
			rc = localworkqueue_queue(workqueue,
				&init_graphic_handle);
			if (rc) {
				/* add work to queue successed. */
				localworkqueue_flush(workqueue,
					&init_graphic_handle);

				if (graphic_handle) {
					/* set no error */
					rc = 0;
				} else {
					/* set error */
					rc = -ENOMEM;
					printk_err2("can not create handle\n");
				}
			} else {
				/* set error */
				rc = -ENOMEM;
				printk_err2("can not create handle\n");
			}
		}

		if (rc == 0) {
			/* increment open count */
			num_open++;
		}
	}
	up(&sem);

	if (rc) {
		/* return by error */
		goto err_exit;
	}
	printk_dbg2(3, "current num of opens:%d\n", num_open);

	/* allocate per-filehandle data */
	private_fh = allocate_device();
	if (private_fh == NULL) {

		printk_dbg2(3, "down\n");
		down(&sem);

		num_open--;
		if (num_open == 0) {
			rc = localworkqueue_queue(workqueue,
				&del_graphic_handle);
			if (rc) {
				/* add work to queue successed. */
				localworkqueue_flush(workqueue,
					&del_graphic_handle);

				if (graphic_handle) {
					/* report error */
					printk_err2("can not delete handle.\n");
				}
			}
		}
		up(&sem);

		rc = -ENOMEM;
	} else {
		filep->private_data = private_fh;
	}
err_exit:
	TRACE_LEAVE(FUNC_OPEN);
	DBGLEAVE("%d\n", rc);
	return rc;
}

static int core_release(struct inode *inode, struct file *filep)
{
	struct composer_fh *fh;

	TRACE_ENTER(FUNC_CLOSE);
	DBGENTER("inode:%p filep:%p\n", inode, filep);

	fh = (struct composer_fh *)filep->private_data;

	free_device(fh);

	printk_dbg2(3, "down\n");
	down(&sem);

	num_open--;
	if (num_open == 0) {
		int    rc;

		rc = localworkqueue_queue(workqueue,
			&del_graphic_handle);
		if (rc) {
			/* add work to queue successed. */
			localworkqueue_flush(workqueue,
				&del_graphic_handle);

			if (graphic_handle) {
				/* report error */
				printk_err2("can not delete handle.\n");
			}
		}
	}
	up(&sem);
	printk_dbg2(3, "current num of opens:%d\n", num_open);

	TRACE_LEAVE(FUNC_CLOSE);
	DBGLEAVE("%d\n", 0);
	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void pm_early_suspend(struct early_suspend *h)
{
	DBGENTER("h:%p\n", h);
	in_early_suspend = 1;

	if (rtapi_hungup && graphic_handle) {
		printk_err("not release graphic handle, "
			"due to RTAPI hung-up\n");
	} else if (graphic_handle) {
		int rc;

#ifdef CONFIG_MISC_R_MOBILE_COMPOSER_REQUEST_QUEUE
		if (kernel_queue_timer.data) {
			/* delete timer */
			del_timer_sync(&kernel_queue_timer);
			kernel_queue_timer.data = 0;
		}
#endif
		printk_dbg2(3, "down\n");
		down(&sem);

		/* flush work. */
		localworkqueue_flush(workqueue,
			&del_graphic_handle);

		/* queue task. */
		rc = localworkqueue_queue(workqueue,
			&del_graphic_handle);
		if (rc) {
			/* wait work compete. */
			localworkqueue_flush(workqueue,
				&del_graphic_handle);
		} else {
			printk_err("failed to release graphic handle\n");
		}

		up(&sem);

#ifdef CONFIG_MISC_R_MOBILE_COMPOSER_REQUEST_QUEUE
		/* remove pending queue. */
		down(&kernel_queue_sem);
		while (!list_empty(&kernel_queue_top)) {
			struct composer_rh *rh;

			rh = list_first_entry(&kernel_queue_top,
				struct composer_rh, list);

			/* remove list */
			printk_dbg2(3, "list %p is removed.\n", rh);

			list_del_init(&rh->list);

			/* process callback */
			up(&kernel_queue_sem);

			composer_blendoverlay_errorcallback(rh);

			printk_dbg2(3, "down\n");
			down(&kernel_queue_sem);
		}
		up(&kernel_queue_sem);

		/* confirm complete of request queue. */
		ioc_waitcomp(NULL);
#endif
	} else {
		printk_dbg2(3, "already release graphic handle\n");
		/* nothing to do */
	}
#ifdef CONFIG_MISC_R_MOBILE_COMPOSER_REQUEST_QUEUE
#if SH_MOBILE_COMPOSER_SUPPORT_HDMI
	printk_dbg2(3, "down\n");
	down(&sem);

	if (rtapi_hungup && graphic_handle_hdmi) {
		printk_err("not release graphic handle, "
			"due to RTAPI hung-up\n");
	} else if (graphic_handle_hdmi) {
		int rc;

		/* queue task. */
		rc = localworkqueue_queue(workqueue,
			&del_graphic_handle_hdmi);
		if (rc) {
			/* wait work compete. */
			localworkqueue_flush(workqueue,
				&del_graphic_handle_hdmi);
		} else {
			printk_err("failed to release graphic "
				"handle for hdmi\n");
		}
	}

	up(&sem);
#endif /* SH_MOBILE_COMPOSER_SUPPORT_HDMI*/
#endif

	printk_dbg2(3, "suspend state:%d graphic_handle:%p"
#ifdef CONFIG_MISC_R_MOBILE_COMPOSER_REQUEST_QUEUE
#if SH_MOBILE_COMPOSER_SUPPORT_HDMI
		"graphic_handle hdmi:%p"
#endif /* SH_MOBILE_COMPOSER_SUPPORT_HDMI*/
#endif
		"\n",
		in_early_suspend, graphic_handle
#ifdef CONFIG_MISC_R_MOBILE_COMPOSER_REQUEST_QUEUE
#if SH_MOBILE_COMPOSER_SUPPORT_HDMI
		, graphic_handle_hdmi
#endif /* SH_MOBILE_COMPOSER_SUPPORT_HDMI*/
#endif
		);
	/* nothing to do */
	DBGLEAVE("\n");
	return;
}

static void pm_late_resume(struct early_suspend *h)
{
	int need_graphic_handle;
	DBGENTER("h:%p\n", h);
	down(&sem);

	need_graphic_handle = 0;
	if (num_open > 0 && graphic_handle == NULL) {
		/* need_graphic_handle */
		need_graphic_handle = 1;
	}

	if (need_graphic_handle == 0) {
		/* not need create handle */
		/* nothing to do */
	} else if (rtapi_hungup) {
		printk_err("not create graphic handle, "
			"due to RTAPI hung-up\n");
	} else if (graphic_handle == NULL) {
		int rc;

		/* flush work. */
		localworkqueue_flush(workqueue,
			&init_graphic_handle);

		/* queue task. */
		rc = localworkqueue_queue(workqueue,
			&init_graphic_handle);
		if (rc) {
			/* wait work compete. */
			localworkqueue_flush(workqueue,
				&init_graphic_handle);
		} else {
			printk_err("failed to create graphic handle\n");
		}
	} else {
		printk_dbg2(3, "already create graphic handle\n");
		/* nothing to do */
	}
	in_early_suspend = 0;
	up(&sem);
	printk_dbg2(3, "suspend state:%d graphic_handle:%p\n",
		in_early_suspend, graphic_handle);
	DBGLEAVE("\n");
	return;
}
#endif /* CONFIG_HAS_EARLYSUSPEND */

/******************************************************/
/* module initialize function                         */
/******************************************************/
static int __init sh_mobile_composer_init(void)
{
	int ret = 0;
	int i;

	DBGENTER("\n");

	/* initialize globals */
	if (debug >= 3) {
		/* set app share library. to debug mode */
		sh_mobile_appmem_debugmode(1);
	}

	memset(&local_buffer, 0, sizeof(local_buffer));
	spin_lock_init(&irqlock);
	sema_init(&sem, 1);
	num_open = 0;
	graphic_handle = NULL;
	INIT_LIST_HEAD(&file_top);

	localwork_init(&del_graphic_handle,  work_deletehandle);
	localwork_init(&init_graphic_handle, work_createhandle);

	for (i = 0; i < MAX_BUFFER; i++) {
		struct composer_buffer   *fh_buffer = &local_buffer[i];
		initialize_bufferinformation(fh_buffer);
	}

	/* initialize request queue. */
#ifdef CONFIG_MISC_R_MOBILE_COMPOSER_REQUEST_QUEUE
	memset(&kernel_request, 0, sizeof(kernel_request));
	INIT_LIST_HEAD(&kernel_queue_top);
	for (i = 0; i < MAX_KERNELREQ; i++) {
		struct composer_rh *rh =  &kernel_request[i];
		localwork_init(&rh->rh_wqtask, work_runblend);
		localwork_init(&rh->rh_wqtask_hdmi, work_overlay);
		INIT_LIST_HEAD(&rh->list);
		initialize_blendcommon_obj(&rh->rh_wqcommon,
			&rh->data.blend,
			callback_composer_queue,
			rh);
	}
	sema_init(&kernel_queue_sem, 1);
	init_waitqueue_head(&kernel_waitqueue_comp);

	spin_lock_init(&irqlock_timer);
	kernel_queue_timer.data = 0;
	localwork_init(&expire_kernel_request, work_expirequeue);

	/* calcurate PHYSICAL ADDRESS SIZE */
	{
		unsigned long size;
		unsigned long ulLCM;
		unsigned long gcd;

		/* one line size. */
		size = SH_MLCD_WIDTH * 4;

		/* calculate Greatest common divisor */
		{
			unsigned long x, y;
			x = size;
			y = 0x1000;
			while (y != 0) {
				unsigned long r = x % y;
				x = y;
				y = r;
			}
			gcd = x;
		}

		/* calculate Least common multiple */
		if (gcd == 0)
			ulLCM = 0;
		else
			ulLCM = (size / gcd) * 0x1000;

		/* round up */
		size = (size * SH_MLCD_HEIGHT + (ulLCM - 1)) / ulLCM;
		size *= ulLCM * 2;

		queue_fb_map_address    = SCREEN_DISPLAY_BUFF_ADDR;
		queue_fb_map_endaddress = SCREEN_DISPLAY_BUFF_ADDR + size;
		queue_fb_map_handle     = NULL; /* not mapped. */
	}

#if SH_MOBILE_COMPOSER_SUPPORT_HDMI
	graphic_handle_hdmi = NULL;
	localwork_init(&del_graphic_handle_hdmi,  work_deletehandle_hdmi);
	localwork_init(&init_graphic_handle_hdmi, work_createhandle_hdmi);
#endif /* SH_MOBILE_COMPOSER_SUPPORT_HDMI*/
#endif

#if INTERNAL_DEBUG >= 1
	spin_lock_init(&log_irqlock);

	internal_log_msg = kmalloc(INTERNAL_LOG_MSG_SIZE, GFP_KERNEL);
	if (internal_log_msg) {
		/* record available memory size */
		internal_log_msgsize = INTERNAL_LOG_MSG_SIZE;
	}
#endif

	/* create workqueue */
	workqueue = localworkqueue_create("sh_mobile_cmp");
	if (workqueue == NULL) {
		printk_err("fail to create_singlethread_workqueue");
		ret = -ENOMEM;
		goto err_exit;
	}

	/* regist device */
	ret = misc_register(&composer_device);
	if (ret) {
		printk_err("fail to misc_register (MISC_DYNAMIC_MINOR)\n");
		goto err_exit;
	}
#ifdef CONFIG_HAS_EARLYSUSPEND
	in_early_suspend = 0;
	register_early_suspend(&early_suspend);
#endif /* CONFIG_HAS_EARLYSUSPEND */

	DBGLEAVE("%d\n", 0);
	return 0;

err_exit:
	if (workqueue) {
		localworkquue_destroy(workqueue);
		workqueue = NULL;
	}
	DBGLEAVE("%d\n", ret);
	return ret;
}

static void __exit sh_mobile_composer_release(void)
{
	DBGENTER("\n");

#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&early_suspend);
#endif /* CONFIG_HAS_EARLYSUSPEND */

#ifdef CONFIG_MISC_R_MOBILE_COMPOSER_REQUEST_QUEUE
	if (kernel_queue_timer.data) {
		/* delete timer */
		del_timer_sync(&kernel_queue_timer);
		kernel_queue_timer.data = 0;
	}
#endif

	if (num_open > 0)
		printk_err("there is 'not close device'.\n");

	/* unregist device */
	misc_deregister(&composer_device);

	/* release all resources */
	/* destroy workqueue */
	if (workqueue) {
		localworkquue_destroy(workqueue);
		workqueue = NULL;
	}

#if INTERNAL_DEBUG >= 1
	if (internal_log_msg) {
		/* free memory */
		kfree(internal_log_msg);
		internal_log_msgsize = 0;
	}
#endif

#ifdef CONFIG_MISC_R_MOBILE_COMPOSER_REQUEST_QUEUE
	if (queue_fb_map_handle) {
		/* unmap handle */
		sh_mobile_rtmem_physarea_unregister(queue_fb_map_handle);
		queue_fb_map_handle = NULL;
	}
#endif

	DBGLEAVE("\n");
	return;
}

module_param(debug, int, 0644);
MODULE_PARM_DESC(debug, "SHMobile Composer debug level");
MODULE_LICENSE("GPL");
module_init(sh_mobile_composer_init);
module_exit(sh_mobile_composer_release);
