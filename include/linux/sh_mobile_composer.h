/*
 * Function        : Composer driver for R Mobile
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
#ifndef _SH_MOBILE_COMPOSER_H
#define _SH_MOBILE_COMPOSER_H
#ifdef CONFIG_MISC_R_MOBILE_COMPOSER

/* common kernel and user */

/******************************/
/* define structure for ioctl */
/******************************/
struct cmp_lay_supportpixfmt {
	int level;
	int pixfmt;
};

struct cmp_lay_open {
	int level;
	int bufferid;
};

struct cmp_lay_close {
	int level;
};

struct cmp_lay_disp {
	int level;
	int on;
};

struct cmp_lay_alpha {
	int           level;
	unsigned char alpha;
};

struct cmp_lay_keycolor {
	int           level;
	unsigned int  keycolor;
};

struct cmp_lay_backcolor {
	int           level;
	unsigned int  backcolor;
};


struct cmp_lay_data_size {
	int           level;
	unsigned int  x;
	unsigned int  y;
};

struct cmp_lay_pixfmt {
	int           level;
	unsigned int  pixfmt;
	unsigned int  colorspace;
};

struct cmp_lay_cropsize {
	int           level;
	unsigned int  x;
	unsigned int  y;
};

struct cmp_lay_croppos {
	int           level;
	unsigned int  x;
	unsigned int  y;
};

struct cmp_lay_pos {
	int           level;
	unsigned int  x;
	unsigned int  y;
};

struct cmp_lay_compose_size {
	int           level;
	unsigned int  x;
	unsigned int  y;
};

struct cmp_viewlay {
	int           level;
	unsigned int  data_x;
	unsigned int  data_y;
	unsigned int  croppos_x;
	unsigned int  croppos_y;
	unsigned int  cropsize_x;
	unsigned int  cropsize_y;
	unsigned int  pos_x;
	unsigned int  pos_y;
	unsigned int  compose_x;
	unsigned int  compose_y;
};

struct cmp_layaddr {
	int           level;
	unsigned char *addr;
	unsigned int  datasize;
	unsigned int  app_id;
	unsigned int  offset;
	unsigned char *addr_c0;
	unsigned int  datasize_c0;
	unsigned int  app_id_c0;
	unsigned int  offset_c0;
	unsigned char *addr_c1;
	unsigned int  datasize_c1;
	unsigned int  app_id_c1;
	unsigned int  offset_c1;
};

/************************/
/* define cmd for ioctl */
/************************/

#define IOC_SH_MOBILE_COMP_MAGIC 'S'

#define CMP_IOC_ISLAYEX \
	_IO(IOC_SH_MOBILE_COMP_MAGIC, 0x00)
#define CMP_IOCG_SUPPORTPIXFMT \
	_IOW(IOC_SH_MOBILE_COMP_MAGIC, 0x01, struct cmp_lay_supportpixfmt)
#define CMP_IOCS_OPEN \
	_IOW(IOC_SH_MOBILE_COMP_MAGIC, 0x02, struct cmp_lay_open)
#define CMP_IOCS_CLOSE \
	_IOW(IOC_SH_MOBILE_COMP_MAGIC, 0x03, struct cmp_lay_close)
#define CMP_IOCS_DISP \
	_IOW(IOC_SH_MOBILE_COMP_MAGIC, 0x04, struct cmp_lay_disp)
#define CMP_IOCS_ALPHA \
	_IOW(IOC_SH_MOBILE_COMP_MAGIC, 0x05, struct cmp_lay_alpha)
#define CMP_IOCG_ALPHA \
	_IOWR(IOC_SH_MOBILE_COMP_MAGIC, 0x06, struct cmp_lay_alpha)
#define CMP_IOCS_KEYCOLOR \
	_IOW(IOC_SH_MOBILE_COMP_MAGIC, 0x07, struct cmp_lay_keycolor)
#define CMP_IOCG_KEYCOLOR \
	_IOWR(IOC_SH_MOBILE_COMP_MAGIC, 0x08, struct cmp_lay_keycolor)
#define CMP_IOCS_LAYSIZE \
	_IOW(IOC_SH_MOBILE_COMP_MAGIC, 0x09, struct cmp_lay_data_size)
#define CMP_IOCG_LAYSIZE \
	_IOWR(IOC_SH_MOBILE_COMP_MAGIC, 0x0a, struct cmp_lay_data_size)
#define CMP_IOCS_PIXFMT \
	_IOW(IOC_SH_MOBILE_COMP_MAGIC, 0x0b, struct cmp_lay_pixfmt)
#define CMP_IOCG_PIXFMT \
	_IOWR(IOC_SH_MOBILE_COMP_MAGIC, 0x0c, struct cmp_lay_pixfmt)
#define CMP_IOCS_CROPSIZE \
	_IOW(IOC_SH_MOBILE_COMP_MAGIC, 0x0d, struct cmp_lay_cropsize)
#define CMP_IOCG_CROPSIZE \
	_IOWR(IOC_SH_MOBILE_COMP_MAGIC, 0x0e, struct cmp_lay_cropsize)
#define CMP_IOCS_CROPPOS \
	_IOW(IOC_SH_MOBILE_COMP_MAGIC, 0x0f, struct cmp_lay_croppos)
#define CMP_IOCG_CROPPOS \
	_IOWR(IOC_SH_MOBILE_COMP_MAGIC, 0x10, struct cmp_lay_croppos)
#define CMP_IOCS_POS \
	_IOW(IOC_SH_MOBILE_COMP_MAGIC, 0x10, struct cmp_lay_pos)
#define CMP_IOCG_POS \
	_IOWR(IOC_SH_MOBILE_COMP_MAGIC, 0x11, struct cmp_lay_pos)
#define CMP_IOCS_SIZE \
	_IOW(IOC_SH_MOBILE_COMP_MAGIC, 0x12, struct cmp_lay_compose_size)
#define CMP_IOCG_SIZE \
	_IOWR(IOC_SH_MOBILE_COMP_MAGIC, 0x13, struct cmp_lay_compose_size)
#define CMP_IOCS_VIEWLAY \
	_IOW(IOC_SH_MOBILE_COMP_MAGIC, 0x14, struct cmp_viewlay)
#define CMP_IOCS_LAYADDR \
	_IOW(IOC_SH_MOBILE_COMP_MAGIC, 0x15, struct cmp_layaddr)
#define CMP_IOCG_LAYADDR \
	_IOWR(IOC_SH_MOBILE_COMP_MAGIC, 0x16, struct cmp_layaddr)
#define CMP_IOCS_BACKCOLOR \
	_IOW(IOC_SH_MOBILE_COMP_MAGIC, 0x17, struct cmp_lay_backcolor)
#define CMP_IOCG_BACKCOLOR \
	_IOWR(IOC_SH_MOBILE_COMP_MAGIC, 0x18, struct cmp_lay_backcolor)
#define CMP_IOC_START \
	_IO(IOC_SH_MOBILE_COMP_MAGIC, 0x19)
#define CMP_IOC_ISSUSPEND \
	_IO(IOC_SH_MOBILE_COMP_MAGIC, 0x1a)
#define CMP_IOC_WAITCOMP \
	_IO(IOC_SH_MOBILE_COMP_MAGIC, 0x1b)
#define CMP_IOC_WAITDRAW \
	_IOW(IOC_SH_MOBILE_COMP_MAGIC, 0x1c, int)


/*******************/
/* define constant */
/*******************/
#define CMP_OUT_PLANE  0
#define CMP_CH1_LAYER  1
#define CMP_CH2_LAYER  2
#define CMP_CH3_LAYER  3
#define CMP_CH4_LAYER  4

#define CMPPIXFMT_ARGB8888  0
#define CMPPIXFMT_RGB888    1
#define CMPPIXFMT_RGB565    2
#define CMPPIXFMT_YUV420SP  3
#define CMPPIXFMT_YUV422SP  4
#define CMPPIXFMT_YUV420PL  5

#define CMPYUVCOLOR_BT601_COMPRESS  0
#define CMPYUVCOLOR_BT601_FULLSCALE 1
#define CMPYUVCOLOR_BT709_COMPRESS  2
#define CMPYUVCOLOR_BT709_FULLSCALE 3

#define CMPKEYCOLOR_OFF 0xFFFFFFFF

#define CMP_OK	0
#define CMP_NG	-1
#define CMP_NG_BASE	0x1000
#define CMP_NG_OPEN_OUT	(-(CMP_NG_BASE+0x003))
#define CMP_NG_OPEN_IN	(-(CMP_NG_BASE+0x001))
#define CMP_NG_NORESOURCE	(-(CMP_NG_BASE+0x002))

#define CMP_NG_VIEWLAY1	(-(CMP_NG_BASE+0x081)) /* laysize */
#define CMP_NG_VIEWLAY2	(-(CMP_NG_BASE+0x082)) /* cropsize */
#define CMP_NG_VIEWLAY3	(-(CMP_NG_BASE+0x083)) /* croppos */
#define CMP_NG_VIEWLAY4	(-(CMP_NG_BASE+0x084)) /* size */
#define CMP_NG_VIEWLAY5	(-(CMP_NG_BASE+0x085)) /* pos */

#define CMP_NG_LAYERPOS_L0  (-(CMP_NG_BASE+0x100)) /*out */
#define CMP_NG_LAYERPOS_L1  (-(CMP_NG_BASE+0x101)) /*l1  */
#define CMP_NG_LAYERPOS_L2  (-(CMP_NG_BASE+0x102)) /*l2  */
#define CMP_NG_LAYERPOS_L3  (-(CMP_NG_BASE+0x103)) /*l3  */
#define CMP_NG_LAYERPOS_L4  (-(CMP_NG_BASE+0x104)) /*l4  */

#define CMP_NG_INSUFFICIENT_L0  (-(CMP_NG_BASE+0x110)) /*out */
#define CMP_NG_INSUFFICIENT_L1  (-(CMP_NG_BASE+0x111)) /*l1  */
#define CMP_NG_INSUFFICIENT_L2  (-(CMP_NG_BASE+0x112)) /*l2  */
#define CMP_NG_INSUFFICIENT_L3  (-(CMP_NG_BASE+0x113)) /*l3  */
#define CMP_NG_INSUFFICIENT_L4  (-(CMP_NG_BASE+0x114)) /*l4  */

#define CMP_NG_RESIZE_L0  (-(CMP_NG_BASE+0x120)) /*out */
#define CMP_NG_RESIZE_L1  (-(CMP_NG_BASE+0x121)) /*l1  */
#define CMP_NG_RESIZE_L2  (-(CMP_NG_BASE+0x122)) /*l2  */
#define CMP_NG_RESIZE_L3  (-(CMP_NG_BASE+0x123)) /*l3  */
#define CMP_NG_RESIZE_L4  (-(CMP_NG_BASE+0x124)) /*l4  */

#define CMP_NG_CROPPOS_L0  (-(CMP_NG_BASE+0x130)) /*out */
#define CMP_NG_CROPPOS_L1  (-(CMP_NG_BASE+0x131)) /*l1  */
#define CMP_NG_CROPPOS_L2  (-(CMP_NG_BASE+0x132)) /*l2  */
#define CMP_NG_CROPPOS_L3  (-(CMP_NG_BASE+0x133)) /*l3  */
#define CMP_NG_CROPPOS_L4  (-(CMP_NG_BASE+0x134)) /*l4  */

/********************/
/* define err code  */

#ifdef __KERNEL__
/* please not define __KERNEL__ when build application. */

#include <linux/spinlock.h>
#include <linux/semaphore.h>
#include <linux/list.h>
#include <rtapi/screen_graphics.h>
#include <media/sh_mobile_appmem.h>

struct composer_bufer;
struct appshare_list;
struct composer_fh;

/* for internal use for kernel */
struct bufferlink {
	struct             list_head list;
	wait_queue_head_t  *body;
	struct composer_fh *fh;
};

struct composer_buffer {
	spinlock_t          buf_lock;
	struct semaphore    buf_sem;
	struct bufferlink   buf_link;
	unsigned int        buf_id;

	/* for buffer parameters */
	unsigned int        alpha;
	unsigned int        keycolor;
	unsigned int        backcolor;
	unsigned int        data_x;
	unsigned int        data_y;
	unsigned int        data_fmt;
	unsigned int        col_space;
	unsigned int        crop_size_x;
	unsigned int        crop_size_y;
	unsigned int        crop_pos_x;
	unsigned int        crop_pos_y;
	unsigned int        pos_x;
	unsigned int        pos_y;
	unsigned int        size_x;
	unsigned int        size_y;
	int                 rotate;
	int                 mirror;

	/* for buffer address (Y/RGB) */
	unsigned char       *addr;
	unsigned int        app_id;
	unsigned int        offset;
	unsigned char       *app_addr;
	unsigned int        app_size;

	/* for buffer address (UV) */
	unsigned char       *addr_c0;
	unsigned int        app_id_c0;
	unsigned int        offset_c0;
	unsigned char       *app_addr_c0;
	unsigned int        app_size_c0;

	/* for buffer address (V) */
	unsigned char       *addr_c1;
	unsigned int        app_id_c1;
	unsigned int        offset_c1;
	unsigned char       *app_addr_c1;
	unsigned int        app_size_c1;
};


struct composer_info {
	struct bufferlink wqlink;
	unsigned int      id;
	struct composer_buffer *buffer;
	struct appmem_handle  *appinfo[3];
};

struct composer_blendcommon {
	wait_queue_head_t        wait_notify;
	int                      status;
	screen_grap_image_blend  *_blend;

	void                     (*callback)(
		int result, void *user_data);
	void                     *user_data;
};

struct composer_grapdata {
	/* rt-api arguments. */
	screen_grap_image_blend _blend;

	screen_grap_layer       _in[4];

	/* work: temporally generate information. */
	/* --- for stride --- */
	int          work_linebyte[5];
	int          work_linebyte_c0[5];
	int          work_linebyte_c1[5];
	int          work_pixelsize[5];
	int          work_pixelsize_c0[5];
	int          work_pixelsize_c1[5];
	/* --- for crop and size --- */
	int          work_rect_x[5];
	int          work_rect_y[5];
	int          work_rect_w[5];
	int          work_rect_h[5];
	/* --- for address --- */
	unsigned int work_base[5];
	void         *work_point[5];
};

struct localworkqueue {
	struct   list_head  top;
	spinlock_t          lock;
	wait_queue_head_t   wait;
	wait_queue_head_t   finish;
	struct task_struct  *task;
};

struct localwork;

struct localwork {
	struct list_head  link;
	void              (*func)(struct localwork *work);
	int               status;
};

/* file handle */
struct composer_fh {
	struct localwork             fh_wqtask;
	struct composer_blendcommon  fh_wqcommon;
	struct list_head         fh_filelist;
	struct semaphore         fh_sem;
	rwlock_t                 fh_rwlock;
	wait_queue_head_t        fh_wait;
	struct composer_info     fh_info[5];
	int                      fh_status;
	struct composer_buffer   fh_buffer_id0[5];
	struct composer_grapdata grap_data;
};

#ifdef CONFIG_MISC_R_MOBILE_COMPOSER_REQUEST_QUEUE
#ifdef CONFIG_MACH_KOTA2
/* support HDMI for KOTA2 */
#define SH_MOBILE_COMPOSER_SUPPORT_HDMI    1
#else
/* support HDMI for other */
#define SH_MOBILE_COMPOSER_SUPPORT_HDMI    3
#endif
#define SH_MOBILE_COMPOSER_WAIT_DRAWEND    0
struct cmp_request_queuedata {
	screen_grap_image_blend blend;
	screen_grap_layer       layer[4];
	int                     num_layers;
#if SH_MOBILE_COMPOSER_SUPPORT_HDMI
	int                     extlayer_index;
	struct {
		struct {
			unsigned short   format;
			unsigned short   yuv_format;
			unsigned short   yuv_range;
			unsigned short   reserved_for_alignment;
		} image;
		int                 rotate;
	} extlayer;
#endif
};

/* request queue handle */
struct composer_rh {
	struct localwork             rh_wqtask;
	struct composer_blendcommon  rh_wqcommon;
	struct localwork             rh_wqtask_hdmi;
	struct cmp_request_queuedata data;
	int                          active;
	void                        (*user_callback)\
		(void *user_data, int result);
	void                         *user_data;
	int                          refcount;

	struct list_head             list;
};

extern int sh_mobile_composer_queue(
	void *data,
	int   data_size,
	void  (*callback)(void *user_data, int result),
	void   *user_data);

extern unsigned char *sh_mobile_composer_phy_change_rtaddr(
	unsigned long p_adr);
extern int sh_mobile_composer_blendoverlay(unsigned long addr);
#if SH_MOBILE_COMPOSER_SUPPORT_HDMI
extern int sh_mobile_composer_hdmiset(int mode);
#endif
#if SH_MOBILE_COMPOSER_WAIT_DRAWEND
extern void sh_mobile_composer_notifyrelease(void);
#endif
#endif
/* end CONFIG_MISC_R_MOBILE_COMPOSER_REQUEST_QUEUE */

#endif
/* end __KERNEL__ */
#endif
/* end  CONFIG_MISC_R_MOBILE_COMPOSER */
#endif
/* end _SH_MOBILE_COMPOSER_H */

