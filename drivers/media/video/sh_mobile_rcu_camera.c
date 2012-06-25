/*
 * V4L2 Driver for SuperH Mobile RCU interface
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 * Copyright (C) 2008 Magnus Damm
 *
 * Based on V4L2 Driver for PXA camera host - "pxa_camera.c",
 *
 * Copyright (C) 2006, Sascha Hauer, Pengutronix
 * Copyright (C) 2008, Guennadi Liakhovetski <kernel@pengutronix.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/* for debug */
#undef DEBUG
/* #define DEBUG */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/moduleparam.h>
#include <linux/time.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/videodev2.h>
#include <linux/pm_runtime.h>
#include <linux/sched.h>
#include <linux/sh_clk.h>

#include <media/v4l2-common.h>
#include <media/v4l2-dev.h>
#include <media/soc_camera.h>
#include <media/sh_mobile_rcu.h>
#include <media/videobuf2-dma-contig.h>
#include <media/v4l2-mediabus.h>
#include <media/soc_mediabus.h>

#include <media/videobuf2-memops.h>

/* alignment */
#define ALIGN32(size)		(((size) + 31) & ~31)
#define ALIGN4(size)		(((size) +  3) &  ~3)

#define ROUNDDOWN(size, X)	(0 == (X) ? (size) : ((size) / (X)) * (X))

#define PRINT_FOURCC(fourcc) ((fourcc) & 0xff, ((fourcc) >> 8) & 0xff, ((fourcc) >> 16) & 0xff, ((fourcc) >> 24) & 0xff)

/* register offsets for r8a73734 */

/* Capture start register */
/* Capture Mode control register */
/* Capture control register */
/* Capture Paralell Interface Control register */
/* Capture offset register */
/* Capture Parallel Interface Horizontal Cycle resgster */
/* RCU register control register */
/* RCU register forcible control register */
/* Capture size clip register */
/* Capture destination width register */
/* Capture data address Y register */
/* Capture data address C register */
/* Capture bundle destination size register */
/* Firewall operating control register */
/* Capture data output control register */
/* Capture event interrupt enable register */
/* Capture event flag clear register */
/* Capture status register */
/* Capture software reset register */
/* Capture data capacity register */
/* Capture data address Y register 2 */
/* Capture data address C register 2 */
/* Extra Capture Control Register */
/* Extra Capture data address register */
/* Extra Capture capacity register */
/* Extra Firewall operating control register */
#define RCAPSR				0x0000UL
#define RCAPCR				0x0004UL
#define RCAMCR				0x0008UL
#define RCPICR				0x000CUL
#define RCAMOR				0x0010UL
#define RCPIHCR				0x0014UL
#define RCRCNTR				0x0028UL
#define RCRCMPR				0x002CUL
#define RCSZR				0x0034UL
#define RCDWDR				0x0038UL
#define RCDAYR				0x003CUL
#define RCDACR				0x0040UL
#define RCBDSR				0x004CUL
#define RCFWCR				0x005CUL
#define RCDOCR				0x0064UL
#define RCEIER				0x0070UL
#define RCETCR				0x0074UL
#define RCSTSR				0x007CUL
#define RCSRTR				0x0080UL
#define RCDSSR				0x0084UL
#define RCDAYR2				0x0090UL
#define RCDACR2				0x0094UL
#define RCECNTR				0x0200UL
#define RCEDAYR				0x0204UL
#define RCEDSSR				0x0214UL
#define RCEFWCR				0x0228UL

#define RCAPSR_CPKIL		16	/* offset of RCAPSR.CPKIL */
#define RCAPSR_CSE		1	/* offset of RCAPSR.CSE */
#define RCAPSR_CE		0	/* offset of RCAPSR.CE */
#define RCAPCR_MTCM		20	/* offset of RCAPCR.MTCM */
#define RCAPCR_CTNCP		16	/* offset of RCAPCR.CTNCP */
#define RCAMCR_OCNT		28	/* offset of RCAMCR.OCNT */
#define RCAMCR_RPAC		24	/* offset of RCAMCR.RPAC */
#define RCAMCR_UCON		22	/* offset of RCAMCR.UCON */
#define RCAMCR_CMPMD		20	/* offset of RCAMCR.CMPMD */
#define RCAMCR_E810B		19	/* offset of RCAMCR.E810B */
#define RCAMCR_E810I		18	/* offset of RCAMCR.E810I */
#define RCAMCR_RAWTYP		16	/* offset of RCAMCR.RAWTYP */
#define RCAMCR_VSS2		15	/* offset of RCAMCR.VSS2 */
#define RCAMCR_HSSM		13	/* offset of RCAMCR.HSSM */
#define RCAMCR_HSS2		12	/* offset of RCAMCR.HSS2 */
#define RCAMCR_CFMT		4	/* offset of RCAMCR.CFMT */
#define RCPICR_IFS		13	/* offset of RCPICR.IFS */
#define RCPICR_DTARY		8	/* offset of RCPICR.DTARY */
#define RCPICR_CKPOL		2	/* offset of RCPICR.CKPOL */
#define RCPICR_VDPOL		1	/* offset of RCPICR.VDPOL */
#define RCPICR_HDPOL		0	/* offset of RCPICR.HDPOL */
#define RCAMOR_VOFST		16	/* offset of RCAMOR.VOFST */
#define RCAMOR_HOFST		0	/* offset of RCAMOR.HOFST */
#define RCPIHCR_STC		16	/* offset of RCPIHCR.STC */
#define RCPIHCR_ENDC		0	/* offset of RCPIHCR.ENDC */
#define RCRCNTR_RSS		17	/* offset of RCRCNTR.RSS */
#define RCRCNTR_RSC		16	/* offset of RCRCNTR.RSC */
#define RCRCNTR_RS		1	/* offset of RCRCNTR.RS */
#define RCRCNTR_RC		0	/* offset of RCRCNTR.RC */
#define RCRCMPR_RSA		16	/* offset of RCRCMPR.RSA */
#define RCRCMPR_RA		0	/* offset of RCRCMPR.RA */
#define RCSZR_VFCLP		16	/* offset of RCSZR.VFCLP */
#define RCSZR_HFCLP		0	/* offset of RCSZR.HFCLP */
#define RCDWDR_CHDW		0	/* offset of RCDWDR.CHDW */
#define RCDAYR_CAYR		0	/* offset of RCDAYR.CAYR */
#define RCDACR_CACR		0	/* offset of RCDACR.CACR */
#define RCBDSR_CBVS		0	/* offset of RCBDSR.CBVS */
#define RCFWCR_WLE		31	/* offset of RCFWCR.WLE */
#define RCFWCR_WLSE		0	/* offset of RCFWCR.WLSE */
#define RCDOCR_CBE		16	/* offset of RCDOCR.CBE */
#define RCDOCR_T420		15	/* offset of RCDOCR.T420 */
#define RCDOCR_H420		12	/* offset of RCDOCR.H420 */
#define RCDOCR_C1BS		9	/* offset of RCDOCR.C1BS */
#define RCDOCR_COLS		2	/* offset of RCDOCR.COLS */
#define RCDOCR_COWS		1	/* offset of RCDOCR.COWS */
#define RCDOCR_COBS		0	/* offset of RCDOCR.COBS */
#define RCEIER_DFOE		28	/* offset of RCEIER.DFOE */
#define RCEIER_ISPOE		24	/* offset of RCEIER.ISPOE */
#define RCEIER_EFWFE		23	/* offset of RCEIER.EFWFE */
#define RCEIER_FWFE		22	/* offset of RCEIER.FWFE */
#define RCEIER_EVBPE		21	/* offset of RCEIER.EVBPE */
#define RCEIER_VBPE		20	/* offset of RCEIER.VBPE */
#define RCEIER_ECDTOFE		17	/* offset of RCEIER.ECDTOFE */
#define RCEIER_CDTOFE		16	/* offset of RCEIER.CDTOFE */
#define RCEIER_CPBE2E		13	/* offset of RCEIER.CPBE2E */
#define RCEIER_CPBE1E		12	/* offset of RCEIER.CPBE1E */
#define RCEIER_VDE		9	/* offset of RCEIER.VDE */
#define RCEIER_HDE		8	/* offset of RCEIER.HDE */
#define RCEIER_EVDE		7	/* offset of RCEIER.EVDE */
#define RCEIER_IGRWE		4	/* offset of RCEIER.IGRWE */
#define RCEIER_CPSEE		2	/* offset of RCEIER.CPSEE */
#define RCEIER_CPEE		0	/* offset of RCEIER.CPEE */
#define RCETCR_DFO		28	/* offset of RCETCR.DFO */
#define RCETCR_ISPO		24	/* offset of RCETCR.ISPO */
#define RCETCR_EFWF		23	/* offset of RCETCR.EFWF */
#define RCETCR_FWF		22	/* offset of RCETCR.FWF */
#define RCETCR_EVBP		21	/* offset of RCETCR.EVBP */
#define RCETCR_VBP		20	/* offset of RCETCR.VBP */
#define RCETCR_ECDTOF		17	/* offset of RCETCR.ECDTOF */
#define RCETCR_CDTOF		16	/* offset of RCETCR.CDTOF */
#define RCETCR_CPBE2		13	/* offset of RCETCR.CPBE2 */
#define RCETCR_CPBE1		12	/* offset of RCETCR.CPBE1 */
#define RCETCR_VD		9	/* offset of RCETCR.VD */
#define RCETCR_HD		8	/* offset of RCETCR.HD */
#define RCETCR_EVD		7	/* offset of RCETCR.EVD */
#define RCETCR_IGRW		4	/* offset of RCETCR.IGRW */
#define RCETCR_CPSE		2	/* offset of RCETCR.CPSE */
#define RCETCR_CPE		0	/* offset of RCETCR.CPE */
#define RCSTSR_CRSST		25	/* offset of RCSTSR.CRSST */
#define RCSTSR_CRST		24	/* offset of RCSTSR.CRST */
#define RCSTSR_CPREQ		9	/* offset of RCSTSR.CPREQ */
#define RCSTSR_CPACK		8	/* offset of RCSTSR.CPACK */
#define RCSTSR_CPTSON		1	/* offset of RCSTSR.CPTSON */
#define RCSTSR_CPTON		0	/* offset of RCSTSR.CPTON */
#define RCSRTR_ALLRST		0	/* offset of RCSRTR.ALLRST */
#define RCDSSR_CDSS		0	/* offset of RCDSSR.CDSS */
#define RCDAYR2_CAYR2		0	/* offset of RCDAYR2.CAYR2 */
#define RCDACR2_CACR2		0	/* offset of RCDACR2.CACR2 */
#define RCECNTR_CTNCP		4	/* offset of RCECNTR.CTNCP */
#define RCECNTR_JPG		1	/* offset of RCECNTR.JPG */
#define RCEDAYR_CAYR		0	/* offset of RCEDAYR.CAYR */
#define RCEDSSR_CDSS		0	/* offset of RCEDSSR.CDSS */
#define RCEFWCR_WLE		31	/* offset of RCEFWCR.WLE */
#define RCEFWCR_WLSE		0	/* offset of RCEFWCR.WLSE */

#define SH_RCU_MODE_IMAGE	0x00	/* image capture mode */
#define SH_RCU_MODE_DATA	0x01	/* data fech mode */
#define SH_RCU_MODE_ENABLE	0x02	/* data enable fetch mode */

#define SH_RCU_STREAMING_OFF	0x00	/* streaming OFF */
#define SH_RCU_STREAMING_ON	0x01	/* streaming ON */

#define SH_RCU_RAWTYP_RAW8	0x00	/* RAW8 format */
#define SH_RCU_RAWTYP_RAW10	0x01	/* RAW10 format */
#define SH_RCU_RAWTYP_RAW12	0x02	/* RAW12 format */

#define SH_RCU_RPAC_ENHANCE	0x00	/* 16bit Enhancing */
#define SH_RCU_RPAC_PACKING	0x01	/* Packing(MIPI-CSI2 standerd) */

/* Transferred to the bus in 4-burst 1-transfer (4QW x 1) units */
#define SH_RCU_MTCM_SIZE_1		0x00
/* Transferred to the bus in 4-burst 2-transfer (4QW x 2) units */
#define SH_RCU_MTCM_SIZE_2		0x01
/* Transferred to the bus in 4-burst 4-transfer (4QW x 4) units */
#define SH_RCU_MTCM_SIZE_4		0x02
/* Transferred to the bus in 4-burst 8-transfer (4QW x 8) units */
#define SH_RCU_MTCM_SIZE_8		0x03
/* Image input data is fetched in the order of Cb, Y0, Cr, and Y1. */
#define SH_RCU_ORDER_UYVY		0x00
/* Image input data is fetched in the order of Cr, Y0, Cb, and Y1. */
#define SH_RCU_ORDER_VYUY		0x01
/* Image input data is fetched in the order of Y0, Cb, Y1, and Cr. */
#define SH_RCU_ORDER_YUYV		0x02
/* Image input data is fetched in the order of Y0, Cr, Y1, and Cb. */
#define SH_RCU_ORDER_YVYU		0x03

#define SH_RCU_MAX_WIDTH	8188	/* max width size  [pixels] */
#define SH_RCU_MAX_HEIGHT	8188	/* max height size [lines] */
#define SH_RCU_MIN_WIDTH	48		/* min width size  [pixels] */
#define SH_RCU_MIN_HEIGHT	48		/* min height size [lines] */

#undef DEBUG_GEOMETRY
#ifdef DEBUG_GEOMETRY
#define dev_geo	dev_info
#else
#define dev_geo	dev_dbg
#endif

/* per video frame buffer */
struct sh_mobile_rcu_buffer {
	struct vb2_buffer vb; /* v4l buffer must be first */
	struct list_head queue;
	enum v4l2_mbus_pixelcode code;
};

struct sh_mobile_rcu_dev {
	struct soc_camera_host ici;
	struct soc_camera_device *icd;

	unsigned int irq;
	void __iomem *base;
	unsigned long video_limit;

	spinlock_t lock;		/* Protects video buffer lists */
	struct list_head capture;
	struct vb2_buffer *active;
	struct vb2_alloc_ctx *alloc_ctx;

	struct sh_mobile_rcu_info *pdata;

	int sequence;

	unsigned int image_mode:2;
	unsigned int output_mode:2;
	unsigned int streaming:1;
	unsigned int output_offset;

	struct clk *iclk;
	struct clk *fclk;
	struct clk *mclk;
};

struct sh_mobile_rcu_cam {
	/* RCU offsets */
	unsigned int rcu_left;
	unsigned int rcu_top;
	/* RCU output size */
	unsigned int width;
	unsigned int height;

	const struct soc_mbus_pixelfmt *extra_fmt;
	enum v4l2_mbus_pixelcode code;
};

static struct sh_mobile_rcu_buffer *to_rcu_vb(struct vb2_buffer *vb)
{
	return container_of(vb, struct sh_mobile_rcu_buffer, vb);
}

static unsigned long make_bus_param(struct sh_mobile_rcu_dev *pcdev)
{
	unsigned long flags;

	flags = SOCAM_MASTER |
		SOCAM_PCLK_SAMPLE_RISING |
		SOCAM_HSYNC_ACTIVE_HIGH |
		SOCAM_HSYNC_ACTIVE_LOW |
		SOCAM_VSYNC_ACTIVE_HIGH |
		SOCAM_VSYNC_ACTIVE_LOW |
		SOCAM_DATA_ACTIVE_HIGH |
		SOCAM_DATAWIDTH_8;

	if (flags & SOCAM_DATAWIDTH_MASK)
		return flags;

	return 0;
}

static void rcu_write(struct sh_mobile_rcu_dev *priv,
		      unsigned long reg_offs, u32 data)
{
	iowrite32(data, priv->base + reg_offs);
}

static u32 rcu_read(struct sh_mobile_rcu_dev *priv, unsigned long reg_offs)
{
	return ioread32(priv->base + reg_offs);
}

static int sh_mobile_rcu_soft_reset(struct sh_mobile_rcu_dev *pcdev)
{
	int i, success = 0;
	struct soc_camera_device *icd = pcdev->icd;

	rcu_write(pcdev, RCAPSR, 1 << RCAPSR_CPKIL); /* reset */

	/* wait RCSTSR.CPTON bit */
	for (i = 0; i < 1000; i++) {
		if (!(rcu_read(pcdev, RCSTSR) & (1 << RCSTSR_CPTON))) {
			success++;
			break;
		}
		udelay(1);
	}

	/* wait CAPSR.CPKIL bit */
	for (i = 0; i < 1000; i++) {
		if (!(rcu_read(pcdev, RCAPSR) & (1 << RCAPSR_CPKIL))) {
			success++;
			break;
		}
		udelay(1);
	}


	if (2 != success) {
		dev_warn(&icd->dev, "soft reset time out\n");
		return -EIO;
	}

	return 0;
}

int sh_mobile_rcu_bytes_per_line(u32 width,
const struct soc_mbus_pixelfmt *mf, struct sh_mobile_rcu_dev *pcdev)
{
	int bytes_per_line = -EINVAL;

	if (SH_RCU_MODE_IMAGE == pcdev->image_mode)
		bytes_per_line = soc_mbus_bytes_per_line(ALIGN32(width), mf);
	else
		bytes_per_line = ALIGN4(soc_mbus_bytes_per_line(width, mf));

	return bytes_per_line;
}

/*
 *  Videobuf operations
 */
static int sh_mobile_rcu_videobuf_setup(struct vb2_queue *vq,
			unsigned int *count, unsigned int *num_planes,
			unsigned long sizes[], void *alloc_ctxs[])
{
	struct soc_camera_device *icd =
			container_of(vq, struct soc_camera_device, vb2_vidq);
	struct soc_camera_host *ici = to_soc_camera_host(icd->dev.parent);
	struct sh_mobile_rcu_dev *pcdev = ici->priv;
	int bytes_per_line = sh_mobile_rcu_bytes_per_line(
						icd->user_width,
						icd->current_fmt->host_fmt,
						pcdev);

	dev_geo(icd->dev.parent, "%s(): stride=%d width=%d packing=%d bps=%d\n",
		__func__, bytes_per_line, icd->user_width,
		icd->current_fmt->host_fmt->packing,
		icd->current_fmt->host_fmt->bits_per_sample);

	if (bytes_per_line < 0)
		return bytes_per_line;

	*num_planes = 1;

	pcdev->sequence = 0;
	sizes[0] = bytes_per_line * icd->user_height;
	alloc_ctxs[0] = pcdev->alloc_ctx;

	if (!*count)
		*count = 2;

	if (pcdev->video_limit) {
		if (PAGE_ALIGN(sizes[0]) * *count > pcdev->video_limit)
			*count = pcdev->video_limit / PAGE_ALIGN(sizes[0]);
	}

	dev_dbg(icd->dev.parent, "count=%d, size=%lu\n", *count, sizes[0]);

	return 0;
}

/* acknowledge magical interrupt sources */
#define RCU_RCETCR_MAGIC	0x11F33395
/* prohibited register access interrupt bit */
#define RCU_RCETCR_IGRW		(1 << RCETCR_IGRW)
/* vbp error */
#define RCU_RCETCR_VBP		(1 << RCETCR_VBP)
/* depacking-fifo was overflow */
#define RCU_RCETCR_DFO		(1 << RCETCR_DFO)

/* continuous capture mode (if set) */
#define RCU_RCAPCR_CTNCP	(1 << RCAPCR_CTNCP)
/* one-frame capture end interrupt */
#define RCU_RCEIER_CPEE		(1 << RCEIER_CPEE)
/* vbp error */
#define RCU_RCEIER_VBPE		(1 << RCEIER_VBPE)
/* depacking-fifo was overflow */
#define RCU_RCEIER_DFOE		(1 << RCEIER_DFOE)
/* output of one frame is completed to ISP module. */
#define RCU_RCEIER_ISPOE	(1 << RCEIER_ISPOE)

/* error mask */
#define RCU_RCETCR_ERR_MASK	(RCU_RCETCR_VBP | RCU_RCETCR_DFO)
/* interrupt enable mask */
#define RCU_RCEIER_MASK		(RCU_RCEIER_ISPOE | RCU_RCEIER_VBPE | \
				RCU_RCEIER_DFOE)
/* interrupt enable mask */
#define RCU_RCEIER_MASK_MEM	(RCU_RCEIER_CPEE  | RCU_RCEIER_VBPE | \
				RCU_RCEIER_DFOE)


/*
 * return value doesn't reflex the success/failure to queue the new buffer,
 * but rather the status of the previous buffer.
 */
static int sh_mobile_rcu_capture(struct sh_mobile_rcu_dev *pcdev)
{
	dma_addr_t phys_addr_top;
	u32 status, rcamcr, rceier;
	int ret = 0;
	struct soc_camera_device *icd = pcdev->icd;

	/*
	 * The hardware is _very_ picky about this sequence. Especially
	 * the RCU_RCETCR_MAGIC value. It seems like we need to acknowledge
	 * several not-so-well documented interrupt sources in CETCR.
	 */
	if (SH_RCU_OUTPUT_MEM == pcdev->output_mode)
		rceier = RCU_RCEIER_MASK_MEM;
	else
		rceier = RCU_RCEIER_MASK;

	rcu_write(pcdev, RCEIER, rcu_read(pcdev, RCEIER) & ~rceier);
	status = rcu_read(pcdev, RCETCR);
	rcu_write(pcdev, RCETCR, ~status & RCU_RCETCR_MAGIC);
	rcu_write(pcdev, RCEIER, rcu_read(pcdev, RCEIER) | rceier);
	rcu_write(pcdev, RCAPCR, rcu_read(pcdev, RCAPCR) & ~RCU_RCAPCR_CTNCP);
	rcu_write(pcdev, RCETCR, RCU_RCETCR_MAGIC ^ RCU_RCETCR_IGRW);

	/*
	 * When a VBP interrupt occurs, a capture end interrupt does not occur
	 * and the image of that frame is not captured correctly. So, soft reset
	 * is needed here.
	 * When a DFO interrupt occurs, depacking-fifo was overflow. When the
	 * data input from the camera exceeds the processing performance in RCU,
	 * this interrupt is generated.
	 */
	if (status & RCU_RCETCR_ERR_MASK) {
		dev_err(pcdev->icd->dev.parent,
			"Error interrupt occurred! RCETCR=0x%08X\n", status);
		sh_mobile_rcu_soft_reset(pcdev);
		ret = -EIO;
	}

	/* ISP output */
	/* Memory and ISP output mode (Active buffer doesn't exist.) */
	if ((SH_RCU_OUTPUT_ISP     == pcdev->output_mode) ||
		(SH_RCU_OUTPUT_MEM_ISP == pcdev->output_mode &&
		!pcdev->active)) {
		/* set output mode */
		rcamcr = rcu_read(pcdev, RCAMCR);
		rcamcr = rcamcr & ~(3 << RCAMCR_OCNT);
		rcu_write(pcdev, RCAMCR, rcamcr |
			(SH_RCU_OUTPUT_ISP << RCAMCR_OCNT));

		rcu_write(pcdev, RCDAYR, 0);
		rcu_write(pcdev, RCDACR, 0);
		rcu_write(pcdev, RCFWCR, 0);

	/* Memory and ISP output mode (Active buffer exists.) */
	/* Memory output (Active buffer exists.) */
	} else if ((SH_RCU_OUTPUT_MEM_ISP == pcdev->output_mode &&
			pcdev->active) ||
			(SH_RCU_OUTPUT_MEM == pcdev->output_mode &&
			pcdev->active)) {
		/* set output mode */
		/*
		 * When there is not an active buffer,
		 * output mode is set in SH_RCU_OUTPUT_MEM_ISP.
		 * Therefore output mode is set again.
		 */
		rcamcr = rcu_read(pcdev, RCAMCR);
		rcamcr = rcamcr & ~(3 << RCAMCR_OCNT);
		rcu_write(pcdev, RCAMCR, rcamcr |
			(pcdev->output_mode << RCAMCR_OCNT));

		phys_addr_top = pcdev->active->v4l2_planes[0].m.userptr;
		rcu_write(pcdev, RCDAYR, phys_addr_top);

		if (SH_RCU_MODE_IMAGE == pcdev->image_mode) {
			if (SH_RCU_OUTPUT_OFFSET_32B == pcdev->output_offset)
				phys_addr_top += ALIGN32(icd->user_width)
					* ALIGN32(icd->user_height);
			else
				phys_addr_top += icd->user_width
					* icd->user_height;
			rcu_write(pcdev, RCDACR, phys_addr_top);
		} else {
			rcu_write(pcdev, RCDACR, 0);
		}

		rcu_write(pcdev, RCFWCR, 0);

	} else {
		return ret;
	}

	rcu_write(pcdev, RCAPSR, 1 << RCAPSR_CE); /* start capture */

	return ret;
}

static int sh_mobile_rcu_videobuf_prepare(struct vb2_buffer *vb)
{
	struct soc_camera_device *icd =
			container_of(vb->vb2_queue, struct soc_camera_device,
				vb2_vidq);
	struct soc_camera_host *ici = to_soc_camera_host(icd->dev.parent);
	struct sh_mobile_rcu_dev *pcdev = ici->priv;
	struct sh_mobile_rcu_buffer *buf;
	int bytes_per_line = sh_mobile_rcu_bytes_per_line(
						icd->user_width,
						icd->current_fmt->host_fmt,
						pcdev);
	unsigned long size;

	dev_dbg(icd->dev.parent, "%s(vb=0x%p): 0x%lx %lu\n", __func__,
		vb, vb->v4l2_planes[0].m.userptr, vb2_get_plane_payload(vb, 0));

	if (bytes_per_line < 0)
		return bytes_per_line;

	buf = to_rcu_vb(vb);

	/* Added list head initialization on alloc */
	WARN(!list_empty(&buf->queue), "Buffer %p on queue!\n", vb);

#ifdef DEBUG
	/*
	 * This can be useful if you want to see if we actually fill
	 * the buffer with something
	 */
	if (vb2_plane_vaddr(vb, 0)) {
		memset(vb2_plane_vaddr(vb, 0), 0xaa,
			vb2_get_plane_payload(vb, 0));
	}
#endif

	BUG_ON(NULL == icd->current_fmt);

	size = icd->user_height * bytes_per_line;

	if (vb->v4l2_planes[0].length < size) {
		dev_err(icd->dev.parent, "Buffer too small (%d < %lu)\n",
			vb->v4l2_planes[0].length, size);
		return -ENOBUFS;
	}

	vb2_set_plane_payload(vb, 0, size);

	return 0;
}

static void sh_mobile_rcu_videobuf_queue(struct vb2_buffer *vb)
{
	struct soc_camera_device *icd =
			container_of(vb->vb2_queue, struct soc_camera_device,
				vb2_vidq);
	struct soc_camera_host *ici = to_soc_camera_host(icd->dev.parent);
	struct sh_mobile_rcu_dev *pcdev = ici->priv;
	struct sh_mobile_rcu_buffer *buf = to_rcu_vb(vb);
	unsigned long flags;

	dev_dbg(icd->dev.parent, "%s(vb=0x%p): 0x%p %lu\n", __func__,
		vb, vb2_plane_vaddr(vb, 0), vb2_get_plane_payload(vb, 0));

	spin_lock_irqsave(&pcdev->lock, flags);
	list_add_tail(&buf->queue, &pcdev->capture);

	if (!pcdev->active) {
		pcdev->active = vb;
		if (SH_RCU_STREAMING_ON == pcdev->streaming) {
			/* restart capture */
			sh_mobile_rcu_capture(pcdev);
		}
	}
	spin_unlock_irqrestore(&pcdev->lock, flags);
}

static void sh_mobile_rcu_videobuf_release(struct vb2_buffer *vb)
{
	struct soc_camera_device *icd =
			container_of(vb->vb2_queue, struct soc_camera_device,
				vb2_vidq);
	struct soc_camera_host *ici = to_soc_camera_host(icd->dev.parent);
	struct sh_mobile_rcu_buffer *buf = to_rcu_vb(vb);
	struct sh_mobile_rcu_dev *pcdev = ici->priv;
	unsigned long flags;

	dev_geo(icd->dev.parent, "%s(): active=0x%p vb=0x%p\n", __func__,
		pcdev->active, vb);

	spin_lock_irqsave(&pcdev->lock, flags);

	if (pcdev->active == vb) {
		/* disable capture (release DMA buffer), reset */
		rcu_write(pcdev, RCAPSR, 1 << RCAPSR_CPKIL);
		pcdev->active = NULL;
	}

	/* Doesn't hurt also if the list is empty */
	list_del_init(&buf->queue);

	spin_unlock_irqrestore(&pcdev->lock, flags);
}

static int sh_mobile_rcu_videobuf_init(struct vb2_buffer *vb)
{
	struct soc_camera_device *icd =
			container_of(vb->vb2_queue, struct soc_camera_device,
				vb2_vidq);
	dev_geo(icd->dev.parent, "%s():\n", __func__);

	/* This is for locking debugging only */
	INIT_LIST_HEAD(&to_rcu_vb(vb)->queue);
	return 0;
}

static int sh_mobile_rcu_start_streaming(struct vb2_queue *q)
{
	struct soc_camera_device *icd =
			container_of(q, struct soc_camera_device, vb2_vidq);
	struct soc_camera_host *ici = to_soc_camera_host(icd->dev.parent);
	struct sh_mobile_rcu_dev *pcdev = ici->priv;
	unsigned long flags;
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	int ret = 0;

	dev_geo(icd->dev.parent, "%s():\n", __func__);

	spin_lock_irqsave(&pcdev->lock, flags);

	if (0x00003E00 == system_rev) {
		void __iomem *intcs_base = ioremap_nocache(0xFFD50000, 0x1000);
		/* IPRVS3 */
		iowrite16(ioread16(intcs_base + 0x54) | 0x0001,
			intcs_base + 0x54);
		/* IMCR10SA3 */
		iowrite8(0x01, intcs_base + 0x1E8);
		dev_geo(icd->dev.parent, "> IPRVS3=0x04%x, IMR10SA3=0x02%x\n",
			ioread16(intcs_base + 0x54), ioread8(intcs_base + 0x1A8));
		iounmap(intcs_base);
	}

	pcdev->streaming = SH_RCU_STREAMING_ON;
	rcu_write(pcdev, RCEIER, 0);
	sh_mobile_rcu_capture(pcdev);

	spin_unlock_irqrestore(&pcdev->lock, flags);

	ret = v4l2_device_call_until_err(sd->v4l2_dev, 0,
				video, s_stream, 1);
	if (ret) {
		printk(KERN_ALERT "%s :Error v4l2_device_call_until_err(%d)\n",
				__func__, ret);
	}

	return 0;
}

static int sh_mobile_rcu_stop_streaming(struct vb2_queue *q)
{
	struct soc_camera_device *icd =
			container_of(q, struct soc_camera_device, vb2_vidq);
	struct soc_camera_host *ici = to_soc_camera_host(icd->dev.parent);
	struct sh_mobile_rcu_dev *pcdev = ici->priv;
	struct list_head *buf_head, *tmp;
	unsigned long flags;
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	int ret = 0;

	dev_geo(icd->dev.parent, "%s():\n", __func__);

	ret = v4l2_device_call_until_err(sd->v4l2_dev, 0, video, s_stream, 0);
	if (ret) {
		printk(KERN_ALERT "%s :Error v4l2_device_call_until_err(%d)\n",
				__func__, ret);
	}

	spin_lock_irqsave(&pcdev->lock, flags);

	list_for_each_safe(buf_head, tmp, &pcdev->capture)
		list_del_init(buf_head);

	pcdev->streaming = SH_RCU_STREAMING_OFF;

	spin_unlock_irqrestore(&pcdev->lock, flags);

	return sh_mobile_rcu_soft_reset(pcdev);
}

static struct vb2_ops sh_mobile_rcu_videobuf_ops = {
	.queue_setup		= sh_mobile_rcu_videobuf_setup,
	.buf_prepare		= sh_mobile_rcu_videobuf_prepare,
	.buf_queue			= sh_mobile_rcu_videobuf_queue,
	.buf_cleanup		= sh_mobile_rcu_videobuf_release,
	.buf_init			= sh_mobile_rcu_videobuf_init,
	.wait_prepare		= soc_camera_unlock,
	.wait_finish		= soc_camera_lock,
	.start_streaming	= sh_mobile_rcu_start_streaming,
	.stop_streaming		= sh_mobile_rcu_stop_streaming,
};

static irqreturn_t sh_mobile_rcu_irq(int irq, void *data)
{
	struct sh_mobile_rcu_dev *pcdev = data;
	struct vb2_buffer *vb;
	int ret;
	unsigned long flags;

	spin_lock_irqsave(&pcdev->lock, flags);

	/* stream on */
	if (SH_RCU_STREAMING_ON == pcdev->streaming) {

		vb = pcdev->active;

		/* ISP output */
		/* Memory and ISP output mode (Active buffer doesn't exist.) */
		if ((SH_RCU_OUTPUT_ISP     == pcdev->output_mode) ||
		    (SH_RCU_OUTPUT_MEM_ISP == pcdev->output_mode && !vb)) {
			sh_mobile_rcu_capture(pcdev);

		/* Memory and ISP output mode (Active buffer exists.) */
		/* Memory output (Active buffer exists.) */
		} else if ((SH_RCU_OUTPUT_MEM_ISP == pcdev->output_mode ||
				vb) ||
				(SH_RCU_OUTPUT_MEM == pcdev->output_mode ||
				vb)) {
			list_del_init(&to_rcu_vb(vb)->queue);

			if (!list_empty(&pcdev->capture))
				pcdev->active = &list_entry(pcdev->capture.next,
						struct sh_mobile_rcu_buffer,
						queue)->vb;
			else
				pcdev->active = NULL;

			ret = sh_mobile_rcu_capture(pcdev);
			do_gettimeofday(&vb->v4l2_buf.timestamp);
			if (!ret) {
				vb->v4l2_buf.field = V4L2_FIELD_NONE;
				vb->v4l2_buf.sequence = pcdev->sequence++;
			}
			vb2_buffer_done(vb,
				ret < 0 ? VB2_BUF_STATE_ERROR : \
				VB2_BUF_STATE_DONE);

		} else {
			rcu_write(pcdev, RCETCR, ~RCU_RCETCR_MAGIC);
		}
	} else {
		rcu_write(pcdev, RCETCR, ~RCU_RCETCR_MAGIC);
	}

	spin_unlock_irqrestore(&pcdev->lock, flags);

	return IRQ_HANDLED;
}

/* Called with .video_lock held */
static int sh_mobile_rcu_add_device(struct soc_camera_device *icd)
{
	struct soc_camera_host *ici = to_soc_camera_host(icd->dev.parent);
	struct sh_mobile_rcu_dev *pcdev = ici->priv;
	int ret;

	dev_geo(icd->dev.parent, "%s():\n", __func__);

	if (pcdev->icd)
		return -EBUSY;

	/* request irq */
	ret = request_irq(pcdev->irq, sh_mobile_rcu_irq, IRQF_DISABLED,
			  dev_name(pcdev->ici.v4l2_dev.dev), pcdev);
	if (ret) {
		dev_err(pcdev->ici.v4l2_dev.dev, "Unable to register RCU interrupt.\n");
		return ret;
	}

	dev_info(icd->dev.parent,
		 "SuperH Mobile RCU driver attached to camera %d\n",
		 icd->devnum);

	pm_runtime_get_sync(ici->v4l2_dev.dev);

	ret = sh_mobile_rcu_soft_reset(pcdev);
	if (!ret)
		pcdev->icd = icd;

	return ret;
}

/* Called with .video_lock held */
static void sh_mobile_rcu_remove_device(struct soc_camera_device *icd)
{
	struct soc_camera_host *ici = to_soc_camera_host(icd->dev.parent);
	struct sh_mobile_rcu_dev *pcdev = ici->priv;
	unsigned long flags;

	dev_geo(icd->dev.parent, "%s(): streaming=%d active=%p\n",
		__func__, pcdev->streaming, pcdev->active);

	BUG_ON(icd != pcdev->icd);

	/* disable capture, disable interrupts */
	rcu_write(pcdev, RCEIER, 0);
	sh_mobile_rcu_soft_reset(pcdev);
	pcdev->streaming = SH_RCU_STREAMING_OFF;
	/* make sure active buffer is canceled */
	spin_lock_irqsave(&pcdev->lock, flags);
	if (pcdev->active) {
		list_del_init(&to_rcu_vb(pcdev->active)->queue);
		vb2_buffer_done(pcdev->active, VB2_BUF_STATE_ERROR);
		pcdev->active = NULL;
	}
	spin_unlock_irqrestore(&pcdev->lock, flags);

	pm_runtime_put_sync(ici->v4l2_dev.dev);

	free_irq(pcdev->irq, pcdev);

	dev_info(icd->dev.parent,
		 "SuperH Mobile RCU driver detached from camera %d\n",
		 icd->devnum);

	pcdev->icd = NULL;
}

/* rect is guaranteed to not exceed the scaled camera rectangle */
static void sh_mobile_rcu_set_rect(struct soc_camera_device *icd)
{
	struct soc_camera_host *ici = to_soc_camera_host(icd->dev.parent);
	struct sh_mobile_rcu_cam *cam = icd->host_priv;
	struct sh_mobile_rcu_dev *pcdev = ici->priv;
	u32 rcamor = 0, rcszr = 0, rcdwdr = 0;
	int bytes_per_line;

	dev_geo(icd->dev.parent, "Crop %ux%u@%u:%u\n",
		icd->user_width, icd->user_height, cam->rcu_left, cam->rcu_top);

	if (SH_RCU_MODE_IMAGE == pcdev->image_mode) {
		rcdwdr = ALIGN32(icd->user_width);

	} else {
		bytes_per_line = sh_mobile_rcu_bytes_per_line(
				icd->user_width,
				icd->current_fmt->host_fmt, pcdev);

		if (bytes_per_line < 0)
			rcdwdr = ALIGN4(icd->user_width * 2);
		else
			rcdwdr = bytes_per_line;
	}

	/* Set RCAMOR, RCSZR, RCDWDR */
	rcamor = cam->rcu_left | (cam->rcu_top << RCAMOR_VOFST);
	rcszr  = icd->user_width | (icd->user_height << RCSZR_VFCLP);

	dev_geo(icd->dev.parent,
		"RCAMOR 0x%08x, RCSZR 0x%08x, RCDWDR 0x%08x\n",
		rcamor, rcszr, rcdwdr);

	rcu_write(pcdev, RCAMOR, rcamor);
	rcu_write(pcdev, RCSZR,  rcszr);
	rcu_write(pcdev, RCDWDR, rcdwdr);
}

static u32 capture_save_reset(struct sh_mobile_rcu_dev *pcdev)
{
	u32 rcapsr = rcu_read(pcdev, RCAPSR);
	rcu_write(pcdev, RCAPSR, 1 << RCAPSR_CPKIL); /* reset, stop capture */
	return rcapsr;
}

static void capture_restore(struct sh_mobile_rcu_dev *pcdev, u32 rcapsr)
{
	unsigned long timeout = jiffies + 10 * HZ;

	/*
	 * Wait until the end of the current frame. It can take a long time,
	 * but if it has been aborted by a RCAPSR reset, it shoule exit sooner.
	 */
	while ((rcu_read(pcdev, RCSTSR) & (1 << RCSTSR_CPTON)) &&
			time_before(jiffies, timeout))
		msleep(1);

	if (time_after(jiffies, timeout)) {
		dev_err(pcdev->ici.v4l2_dev.dev,
			"Timeout waiting for frame end! Interface problem?\n");
		return;
	}

	/* Wait until reset clears, this shall not hang... */
	while (rcu_read(pcdev, RCAPSR) & (1 << RCAPSR_CPKIL))
		udelay(10);

	/* Anything to restore? */
	if (rcapsr & ~(1 << RCAPSR_CPKIL))
		rcu_write(pcdev, RCAPSR, rcapsr);
}

/* Capture is not running, no interrupts, no locking needed */
static int sh_mobile_rcu_set_bus_param(struct soc_camera_device *icd,
				       __u32 pixfmt)
{
	struct soc_camera_host *ici = to_soc_camera_host(icd->dev.parent);
	struct sh_mobile_rcu_dev *pcdev = ici->priv;
	int ret;
	unsigned long camera_flags, common_flags;
	struct sh_mobile_rcu_cam *cam = icd->host_priv;
	u32 rcapsr = capture_save_reset(pcdev);
	u32 rcapcr = 0, rcamcr = 0, rcpicr = 0, rcdocr = 0;
	int yuv_lineskip;

	dev_geo(icd->dev.parent, "%s():\n", __func__);

	camera_flags = icd->ops->query_bus_param(icd);
	common_flags = soc_camera_bus_param_compatible(camera_flags,
						       make_bus_param(pcdev));
	if (!common_flags)
		return -EINVAL;

	/* Make choises, based on platform preferences */
	if ((common_flags & SOCAM_HSYNC_ACTIVE_HIGH) &&
	    (common_flags & SOCAM_HSYNC_ACTIVE_LOW)) {
		if (pcdev->pdata->flags & SH_RCU_FLAG_HSYNC_LOW)
			common_flags &= ~SOCAM_HSYNC_ACTIVE_HIGH;
		else
			common_flags &= ~SOCAM_HSYNC_ACTIVE_LOW;
	}

	if ((common_flags & SOCAM_VSYNC_ACTIVE_HIGH) &&
	    (common_flags & SOCAM_VSYNC_ACTIVE_LOW)) {
		if (pcdev->pdata->flags & SH_RCU_FLAG_VSYNC_LOW)
			common_flags &= ~SOCAM_VSYNC_ACTIVE_HIGH;
		else
			common_flags &= ~SOCAM_VSYNC_ACTIVE_LOW;
	}

	ret = icd->ops->set_bus_param(icd, common_flags);
	if (ret < 0)
		return ret;

	/* setting RCAMCR */
	rcamcr |= pcdev->image_mode << RCAMCR_CFMT;
	rcamcr |= pcdev->output_mode << RCAMCR_OCNT;

	yuv_lineskip = 0;
	switch (icd->current_fmt->host_fmt->fourcc) {
	case V4L2_PIX_FMT_SBGGR8:
	case V4L2_PIX_FMT_SGBRG8:
	case V4L2_PIX_FMT_SGRBG8:
	case V4L2_PIX_FMT_SRGGB8:
		rcamcr |= SH_RCU_RAWTYP_RAW8 << RCAMCR_RAWTYP;
		break;
	case V4L2_PIX_FMT_SBGGR10:
	case V4L2_PIX_FMT_SGBRG10:
	case V4L2_PIX_FMT_SGRBG10:
	case V4L2_PIX_FMT_SRGGB10:
		rcamcr |= SH_RCU_RAWTYP_RAW10 << RCAMCR_RAWTYP;
		rcamcr |= SH_RCU_RPAC_ENHANCE << RCAMCR_RPAC;
		switch (cam->code) {
		case V4L2_MBUS_FMT_SBGGR8_1X8:
		case V4L2_MBUS_FMT_SGBRG8_1X8:
		case V4L2_MBUS_FMT_SGRBG8_1X8:
		case V4L2_MBUS_FMT_SRGGB8_1X8:
			rcamcr |= 1 << RCAMCR_E810B;
			rcamcr |= 1 << RCAMCR_E810I;
			break;
		default:
			break;
		}
		break;
	case V4L2_PIX_FMT_SBGGR12:
	case V4L2_PIX_FMT_SGBRG12:
	case V4L2_PIX_FMT_SGRBG12:
	case V4L2_PIX_FMT_SRGGB12:
		rcamcr |= SH_RCU_RAWTYP_RAW12 << RCAMCR_RAWTYP;
		rcamcr |= SH_RCU_RPAC_ENHANCE << RCAMCR_RPAC;
		break;
	case V4L2_PIX_FMT_NV12:
	case V4L2_PIX_FMT_NV21:
		yuv_lineskip = 1; /* skip for NV12/21, no skip for NV16/61 */
		/* fall-through */
	case V4L2_PIX_FMT_NV16:
	case V4L2_PIX_FMT_NV61:
		switch (cam->code) {
		case V4L2_MBUS_FMT_UYVY8_2X8:
			/* Cb0, Y0, Cr0, Y1 */
			rcpicr |= SH_RCU_ORDER_UYVY << RCPICR_DTARY;
			break;
		case V4L2_MBUS_FMT_VYUY8_2X8:
			/* Cr0, Y0, Cb0, Y1 */
			rcpicr |= SH_RCU_ORDER_VYUY << RCPICR_DTARY;
			break;
		case V4L2_MBUS_FMT_YUYV8_2X8:
			/* Y0, Cb0, Y1, Cr0 */
			rcpicr |= SH_RCU_ORDER_YUYV << RCPICR_DTARY;
			break;
		case V4L2_MBUS_FMT_YVYU8_2X8:
			/* Y0, Cr0, Y1, Cb0 */
			rcpicr |= SH_RCU_ORDER_YVYU << RCPICR_DTARY;
			break;
		default:
			BUG();
		}
		break;
	default:
		break;
	}

	/* setting RCPICR */
	rcpicr |= common_flags & SOCAM_VSYNC_ACTIVE_LOW ? 1 << RCPICR_VDPOL : 0;
	rcpicr |= common_flags & SOCAM_HSYNC_ACTIVE_LOW ? 1 << RCPICR_HDPOL : 0;
	if (!pcdev->pdata->csi2_dev)
		rcpicr |= 1 << RCPICR_IFS;	/* Paralell I/F */

	/* setting RCAPCR */
	rcapcr |= SH_RCU_MTCM_SIZE_8 << RCAPCR_MTCM;

	/* setting RCDOCR */
	/*
	 * A few words about byte order (observed in Big Endian mode)
	 *
	 * In data fetch mode bytes are received in chunks of 8 bytes.
	 * D0, D1, D2, D3, D4, D5, D6, D7 (D0 received first)
	 *
	 * The data is however by default written to memory in reverse order:
	 * D7, D6, D5, D4, D3, D2, D1, D0 (D7 written to lowest byte)
	 *
	 * The lowest three bits of RCDOCR allows us to do swapping,
	 * using 7 we swap the data bytes to match the incoming order:
	 * D0, D1, D2, D3, D4, D5, D6, D7
	 */
	rcdocr |= 1 << RCDOCR_COLS;
	rcdocr |= 1 << RCDOCR_COWS;
	rcdocr |= 1 << RCDOCR_COBS;

	if (yuv_lineskip)
		rcdocr |= 1 << RCDOCR_T420;	/* output YUV420 */

	if ((icd->current_fmt->host_fmt->fourcc == V4L2_PIX_FMT_NV21) ||
	    (icd->current_fmt->host_fmt->fourcc == V4L2_PIX_FMT_NV61)) {
	    /* swap U, V to change from NV1x->NVx1 */
		rcdocr |= 1 << RCDOCR_C1BS;
	}

	sh_mobile_rcu_set_rect(icd);
	mdelay(1);

	dev_geo(icd->dev.parent,
		"RCAPCR 0x%08x, RCAMCR 0x%08x, RCPICR 0x%08x, RCDOCR 0x%08x\n",
		rcapcr, rcamcr, rcpicr, rcdocr);

	rcu_write(pcdev, RCAPCR,  rcapcr);
	rcu_write(pcdev, RCAMCR,  rcamcr);
	rcu_write(pcdev, RCPICR,  rcpicr);
	rcu_write(pcdev, RCDOCR,  rcdocr);
	rcu_write(pcdev, RCPIHCR, 0);
	rcu_write(pcdev, RCRCNTR, 0);
	rcu_write(pcdev, RCRCMPR, 0);
	rcu_write(pcdev, RCBDSR,  0);

	dev_dbg(icd->dev.parent, "S_FMT successful for %c%c%c%c %ux%u\n",
		PRINT_FOURCC(pixfmt), icd->user_width, icd->user_height);

	capture_restore(pcdev, rcapsr);

	dev_dbg(icd->dev.parent, "> RCAPSR  : 0x%08X\n",
		rcu_read(pcdev, RCAPSR));
	dev_dbg(icd->dev.parent, "> RCAPCR  : 0x%08X\n",
		rcu_read(pcdev, RCAPCR));
	dev_dbg(icd->dev.parent, "> RCAMCR  : 0x%08X\n",
		rcu_read(pcdev, RCAMCR));
	dev_dbg(icd->dev.parent, "> RCPICR  : 0x%08X\n",
		rcu_read(pcdev, RCPICR));
	dev_dbg(icd->dev.parent, "> RCAMOR  : 0x%08X\n",
		rcu_read(pcdev, RCAMOR));
	dev_dbg(icd->dev.parent, "> RCPIHCR : 0x%08X\n",
		rcu_read(pcdev, RCPIHCR));
	dev_dbg(icd->dev.parent, "> RCRCNTR : 0x%08X\n",
		rcu_read(pcdev, RCRCNTR));
	dev_dbg(icd->dev.parent, "> RCRCMPR : 0x%08X\n",
		rcu_read(pcdev, RCRCMPR));
	dev_dbg(icd->dev.parent, "> RCSZR   : 0x%08X\n",
		rcu_read(pcdev, RCSZR));
	dev_dbg(icd->dev.parent, "> RCDWDR  : 0x%08X\n",
		rcu_read(pcdev, RCDWDR));
	dev_dbg(icd->dev.parent, "> RCDAYR  : 0x%08X\n",
		rcu_read(pcdev, RCDAYR));
	dev_dbg(icd->dev.parent, "> RCDACR  : 0x%08X\n",
		rcu_read(pcdev, RCDACR));
	dev_dbg(icd->dev.parent, "> RCBDSR  : 0x%08X\n",
		rcu_read(pcdev, RCBDSR));
	dev_dbg(icd->dev.parent, "> RCFWCR  : 0x%08X\n",
		rcu_read(pcdev, RCFWCR));
	dev_dbg(icd->dev.parent, "> RCDOCR  : 0x%08X\n",
		rcu_read(pcdev, RCDOCR));
	dev_dbg(icd->dev.parent, "> RCEIER  : 0x%08X\n",
		rcu_read(pcdev, RCEIER));
	dev_dbg(icd->dev.parent, "> RCETCR  : 0x%08X\n",
		rcu_read(pcdev, RCETCR));
	dev_dbg(icd->dev.parent, "> RCSTSR  : 0x%08X\n",
		rcu_read(pcdev, RCSTSR));
	dev_dbg(icd->dev.parent, "> RCSRTR  : 0x%08X\n",
		rcu_read(pcdev, RCSRTR));
	dev_dbg(icd->dev.parent, "> RCDSSR  : 0x%08X\n",
		rcu_read(pcdev, RCDSSR));
	dev_dbg(icd->dev.parent, "> RCDAYR2 : 0x%08X\n",
		rcu_read(pcdev, RCDAYR2));
	dev_dbg(icd->dev.parent, "> RCDACR2 : 0x%08X\n",
		rcu_read(pcdev, RCDACR2));
	dev_dbg(icd->dev.parent, "> RCECNTR : 0x%08X\n",
		rcu_read(pcdev, RCECNTR));
	dev_dbg(icd->dev.parent, "> RCEDAYR : 0x%08X\n",
		rcu_read(pcdev, RCEDAYR));
	dev_dbg(icd->dev.parent, "> RCEDSSR : 0x%08X\n",
		rcu_read(pcdev, RCEDSSR));
	dev_dbg(icd->dev.parent, "> RCEFWCR : 0x%08X\n",
		rcu_read(pcdev, RCEFWCR));

	/* not in bundle mode: skip RCBDSR, RCDAYR2, RCDACR2 */
	return 0;
}

static int sh_mobile_rcu_try_bus_param(struct soc_camera_device *icd,
				       unsigned char buswidth)
{
	struct soc_camera_host *ici = to_soc_camera_host(icd->dev.parent);
	struct sh_mobile_rcu_dev *pcdev = ici->priv;
	unsigned long camera_flags, common_flags;

	camera_flags = icd->ops->query_bus_param(icd);
	common_flags = soc_camera_bus_param_compatible(camera_flags,
						       make_bus_param(pcdev));
	if (!common_flags || buswidth > 8 ||
		!(common_flags & SOCAM_DATAWIDTH_8)) {
		return -EINVAL;
	}

	return 0;
}

static const struct soc_mbus_pixelfmt sh_mobile_rcu_formats[] = {
	{
		.fourcc				= V4L2_PIX_FMT_SBGGR8,
		.name				= "Bayer 8 BGGR",
		.bits_per_sample	= 8,
		.packing			= SOC_MBUS_PACKING_NONE,
		.order				= SOC_MBUS_ORDER_LE,
	}, {
		.fourcc				= V4L2_PIX_FMT_SGBRG8,
		.name				= "Bayer 8 GBRG",
		.bits_per_sample	= 8,
		.packing			= SOC_MBUS_PACKING_NONE,
		.order				= SOC_MBUS_ORDER_LE,
	}, {
		.fourcc				= V4L2_PIX_FMT_SGRBG8,
		.name				= "Bayer 8 GRBG",
		.bits_per_sample	= 8,
		.packing			= SOC_MBUS_PACKING_NONE,
		.order				= SOC_MBUS_ORDER_LE,
	}, {
		.fourcc				= V4L2_PIX_FMT_SRGGB8,
		.name				= "Bayer 8 RGGB",
		.bits_per_sample	= 8,
		.packing			= SOC_MBUS_PACKING_NONE,
		.order				= SOC_MBUS_ORDER_LE,
	}, {
		.fourcc				= V4L2_PIX_FMT_SBGGR10,
		.name				= "Bayer 10 BGGR",
		.bits_per_sample	= 10,
		.packing			= SOC_MBUS_PACKING_EXTEND16,
		.order				= SOC_MBUS_ORDER_LE,
	}, {
		.fourcc				= V4L2_PIX_FMT_SGBRG10,
		.name				= "Bayer 10 GBRG",
		.bits_per_sample	= 10,
		.packing			= SOC_MBUS_PACKING_EXTEND16,
		.order				= SOC_MBUS_ORDER_LE,
	}, {
		.fourcc				= V4L2_PIX_FMT_SGRBG10,
		.name				= "Bayer 10 GRBG",
		.bits_per_sample	= 10,
		.packing			= SOC_MBUS_PACKING_EXTEND16,
		.order				= SOC_MBUS_ORDER_LE,
	}, {
		.fourcc				= V4L2_PIX_FMT_SRGGB10,
		.name				= "Bayer 10 RGGB",
		.bits_per_sample	= 10,
		.packing			= SOC_MBUS_PACKING_EXTEND16,
		.order				= SOC_MBUS_ORDER_LE,
	}, {
		.fourcc				= V4L2_PIX_FMT_SBGGR12,
		.name				= "Bayer 12 BGGR",
		.bits_per_sample	= 12,
		.packing			= SOC_MBUS_PACKING_EXTEND16,
		.order				= SOC_MBUS_ORDER_LE,
	}, {
		.fourcc				= V4L2_PIX_FMT_SGBRG12,
		.name				= "Bayer 12 GBRG",
		.bits_per_sample	= 12,
		.packing			= SOC_MBUS_PACKING_EXTEND16,
		.order				= SOC_MBUS_ORDER_LE,
	}, {
		.fourcc				= V4L2_PIX_FMT_SGRBG12,
		.name				= "Bayer 12 GRBG",
		.bits_per_sample	= 12,
		.packing			= SOC_MBUS_PACKING_EXTEND16,
		.order				= SOC_MBUS_ORDER_LE,
	}, {
		.fourcc				= V4L2_PIX_FMT_SRGGB12,
		.name				= "Bayer 12 RGGB",
		.bits_per_sample	= 12,
		.packing			= SOC_MBUS_PACKING_EXTEND16,
		.order				= SOC_MBUS_ORDER_LE,
	}, {
		.fourcc				= V4L2_PIX_FMT_NV12,
		.name				= "NV12",
		.bits_per_sample	= 12,
		.packing			= SOC_MBUS_PACKING_NONE,
		.order				= SOC_MBUS_ORDER_LE,
	}, {
		.fourcc				= V4L2_PIX_FMT_NV21,
		.name				= "NV21",
		.bits_per_sample	= 12,
		.packing			= SOC_MBUS_PACKING_NONE,
		.order				= SOC_MBUS_ORDER_LE,
	}, {
		.fourcc				= V4L2_PIX_FMT_NV16,
		.name				= "NV16",
		.bits_per_sample	= 16,
		.packing			= SOC_MBUS_PACKING_NONE,
		.order				= SOC_MBUS_ORDER_LE,
	}, {
		.fourcc				= V4L2_PIX_FMT_NV61,
		.name				= "NV61",
		.bits_per_sample	= 16,
		.packing			= SOC_MBUS_PACKING_NONE,
		.order				= SOC_MBUS_ORDER_LE,
	},
};

static int client_g_rect(struct v4l2_subdev *sd, struct v4l2_rect *rect);

static int sh_mobile_rcu_get_formats(struct soc_camera_device *icd,
		unsigned int idx, struct soc_camera_format_xlate *xlate)
{
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	struct device *dev = icd->dev.parent;
	struct soc_camera_host *ici = to_soc_camera_host(dev);
	struct sh_mobile_rcu_dev *pcdev = ici->priv;
	int ret, n, j, k;
	int formats = 0;
	struct sh_mobile_rcu_cam *cam;
	enum v4l2_mbus_pixelcode code;
	const struct soc_mbus_pixelfmt *fmt;

	dev_geo(dev, "%s(): idx=%d xlate=%p\n", __func__, idx, xlate);

	ret = v4l2_subdev_call(sd, video, enum_mbus_fmt, idx, &code);
	if (ret < 0)
		/* No more formats */
		return 0;

	fmt = soc_mbus_get_fmtdesc(code);
	if (!fmt) {
		dev_warn(dev, "unsupported format code #%u: 0x%x\n", idx, code);
		return 0;
	}

	if (!pcdev->pdata->csi2_dev) {
		ret = sh_mobile_rcu_try_bus_param(icd, fmt->bits_per_sample);
		if (ret < 0)
			return 0;
	}

	if (!icd->host_priv) {
		struct v4l2_mbus_framefmt mf;
		struct v4l2_rect rect;
		int shift = 0;

		/* FIXME: subwindow is lost between close / open */

		/* First time */
		ret = v4l2_subdev_call(sd, video, g_mbus_fmt, &mf);
		if (ret < 0)
			return ret;

		while ((mf.width > 3264 || mf.height > 2448) && shift < 4) {
			/* Try 2560x1920, 1280x960, 640x480, 320x240 */
			mf.width	= 2560 >> shift;
			mf.height	= 1920 >> shift;
			ret = v4l2_device_call_until_err(sd->v4l2_dev,
				(long)icd, video, s_mbus_fmt, &mf);
			if (ret < 0)
				return ret;
			shift++;
		}

		if (shift == 4) {
			dev_err(dev, "Failed to configure the client below %ux%x\n",
				mf.width, mf.height);
			return -EIO;
		}

		dev_geo(dev, "camera fmt %ux%u\n", mf.width, mf.height);

		/* Cache current client geometry */
		ret = client_g_rect(sd, &rect);
		if (ret < 0)
			return ret;

		cam = kzalloc(sizeof(*cam), GFP_KERNEL);
		if (!cam)
			return -ENOMEM;

		/* We are called with current camera crop, */
		/* initialise rect with it */
		cam->width		= mf.width;
		cam->height		= mf.height;
		cam->rcu_left	= rect.left;
		cam->rcu_top	= rect.top;

		icd->host_priv = cam;
	} else {
		cam = icd->host_priv;
	}

	/* Beginning of a pass */
	if (!idx)
		cam->extra_fmt = NULL;

	switch (code) {
	case V4L2_MBUS_FMT_SBGGR8_1X8:
		formats++;
		if (xlate) {
			xlate->host_fmt	= &sh_mobile_rcu_formats[4];
			xlate->code		= code;
			dev_dbg(dev, "Providing format %s using code 0x%x\n",
				xlate->host_fmt->name, code);
			xlate++;
		}
		break;
	case V4L2_MBUS_FMT_SGBRG8_1X8:
		formats++;
		if (xlate) {
			xlate->host_fmt	= &sh_mobile_rcu_formats[5];
			xlate->code		= code;
			dev_dbg(dev, "Providing format %s using code 0x%x\n",
				xlate->host_fmt->name, code);
			xlate++;
		}
		break;
	case V4L2_MBUS_FMT_SGRBG8_1X8:
		formats++;
		if (xlate) {
			xlate->host_fmt	= &sh_mobile_rcu_formats[6];
			xlate->code		= code;
			dev_dbg(dev, "Providing format %s using code 0x%x\n",
				xlate->host_fmt->name, code);
			xlate++;
		}
		break;
	case V4L2_MBUS_FMT_SRGGB8_1X8:
		formats++;
		if (xlate) {
			xlate->host_fmt	= &sh_mobile_rcu_formats[7];
			xlate->code		= code;
			dev_dbg(dev, "Providing format %s using code 0x%x\n",
				xlate->host_fmt->name, code);
			xlate++;
		}
		break;
	case V4L2_MBUS_FMT_SBGGR10_1X10:
		formats++;
		if (xlate) {
			xlate->host_fmt	= &sh_mobile_rcu_formats[4];
			xlate->code		= code;
			dev_dbg(dev, "Providing format %s using code 0x%x\n",
				xlate->host_fmt->name, code);
			xlate++;
		}
		break;
	case V4L2_MBUS_FMT_SGBRG10_1X10:
		formats++;
		if (xlate) {
			xlate->host_fmt	= &sh_mobile_rcu_formats[5];
			xlate->code		= code;
			dev_dbg(dev, "Providing format %s using code 0x%x\n",
				xlate->host_fmt->name, code);
			xlate++;
		}
		break;
	case V4L2_MBUS_FMT_SGRBG10_1X10:
		formats++;
		if (xlate) {
			xlate->host_fmt	= &sh_mobile_rcu_formats[6];
			xlate->code		= code;
			dev_dbg(dev, "Providing format %s using code 0x%x\n",
				xlate->host_fmt->name, code);
			xlate++;
		}
		break;
	case V4L2_MBUS_FMT_SRGGB10_1X10:
		formats++;
		if (xlate) {
			xlate->host_fmt	= &sh_mobile_rcu_formats[7];
			xlate->code		= code;
			dev_dbg(dev, "Providing format %s using code 0x%x\n",
				xlate->host_fmt->name, code);
			xlate++;
		}
		break;
	case V4L2_MBUS_FMT_SBGGR12_1X12:
		formats++;
		if (xlate) {
			xlate->host_fmt	= &sh_mobile_rcu_formats[8];
			xlate->code		= code;
			dev_dbg(dev, "Providing format %s using code 0x%x\n",
				xlate->host_fmt->name, code);
			xlate++;
		}
		break;
	case V4L2_MBUS_FMT_SGBRG12_1X12:
		formats++;
		if (xlate) {
			xlate->host_fmt	= &sh_mobile_rcu_formats[9];
			xlate->code		= code;
			dev_dbg(dev, "Providing format %s using code 0x%x\n",
				xlate->host_fmt->name, code);
			xlate++;
		}
		break;
	case V4L2_MBUS_FMT_SGRBG12_1X12:
		formats++;
		if (xlate) {
			xlate->host_fmt	= &sh_mobile_rcu_formats[10];
			xlate->code		= code;
			dev_dbg(dev, "Providing format %s using code 0x%x\n",
				xlate->host_fmt->name, code);
			xlate++;
		}
		break;
	case V4L2_MBUS_FMT_SRGGB12_1X12:
		formats++;
		if (xlate) {
			xlate->host_fmt	= &sh_mobile_rcu_formats[11];
			xlate->code		= code;
			dev_dbg(dev, "Providing format %s using code 0x%x\n",
				xlate->host_fmt->name, code);
			xlate++;
		}
		break;
	case V4L2_MBUS_FMT_UYVY8_2X8:
	case V4L2_MBUS_FMT_VYUY8_2X8:
	case V4L2_MBUS_FMT_YUYV8_2X8:
	case V4L2_MBUS_FMT_YVYU8_2X8:
		if (cam->extra_fmt)
			break;

		/*
		 * Our case is simple so far: for any of the above four camera
		 * formats we add all our four synthesized NV* formats, so,
		 * just marking the device with a single flag suffices. If
		 * the format generation rules are more complex, you would have
		 * to actually hang your already added / counted formats onto
		 * the host_priv pointer and check whether the format you're
		 * going to add now is already there.
		 */
		cam->extra_fmt = sh_mobile_rcu_formats;

		n = 4;	/* support formats */
		j = 12;	/* offset */
		formats += n;
		for (k = j; xlate && k < n + j; k++) {
			xlate->host_fmt	= &sh_mobile_rcu_formats[k];
			xlate->code	= code;
			xlate++;
			dev_dbg(dev, "Providing format %s using code 0x%x\n",
				sh_mobile_rcu_formats[k].name, code);
		}
		break;
	default:
		dev_dbg(dev, "unsupported code 0x%x\n", code);
		return 0;
	}

	/* Generic pass-through */
	formats++;
	if (xlate) {
		xlate->host_fmt	= fmt;
		xlate->code	= code;
		xlate++;
		dev_dbg(dev, "Providing format %s in pass-through mode\n",
			fmt->name);
	}

	return formats;
}

static void sh_mobile_rcu_put_formats(struct soc_camera_device *icd)
{
	dev_geo(icd->dev.parent, "%s():\n", __func__);

	kfree(icd->host_priv);
	icd->host_priv = NULL;
}

/* Get and store current client crop */
static int client_g_rect(struct v4l2_subdev *sd, struct v4l2_rect *rect)
{
	struct v4l2_crop crop;
	struct v4l2_cropcap cap;
	int ret;

	crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	ret = v4l2_subdev_call(sd, video, g_crop, &crop);
	if (!ret) {
		*rect = crop.c;
		return ret;
	}

	/* Camera driver doesn't support .g_crop(), assume default rectangle */
	cap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	ret = v4l2_subdev_call(sd, video, cropcap, &cap);
	if (!ret)
		*rect = cap.defrect;

	return ret;
}

/* Alignment is multiple of 2^(walign or haling). */
static void get_alignment(__u32 pixfmt, unsigned int output_mode,
				unsigned int *walign, unsigned int *haling)
{
	*haling = 0;
	switch (pixfmt) {
	case V4L2_PIX_FMT_NV12:
	case V4L2_PIX_FMT_NV21:
	case V4L2_PIX_FMT_NV16:
	case V4L2_PIX_FMT_NV61:
		*walign = 1;
		break;
	case V4L2_PIX_FMT_SBGGR8:
	case V4L2_PIX_FMT_SGBRG8:
	case V4L2_PIX_FMT_SGRBG8:
	case V4L2_PIX_FMT_SRGGB8:
		*walign = (SH_RCU_OUTPUT_ISP == output_mode) ? 0 : 2;
		break;
	case V4L2_PIX_FMT_SBGGR10:
	case V4L2_PIX_FMT_SGBRG10:
	case V4L2_PIX_FMT_SGRBG10:
	case V4L2_PIX_FMT_SRGGB10:
	case V4L2_PIX_FMT_SBGGR12:
	case V4L2_PIX_FMT_SGBRG12:
	case V4L2_PIX_FMT_SGRBG12:
	case V4L2_PIX_FMT_SRGGB12:
		*walign = (SH_RCU_OUTPUT_ISP == output_mode) ? 0 : 1;
		break;
	default:
		*walign = 0;
		break;
	}
}

static int sh_mobile_rcu_set_fmt(struct soc_camera_device *icd,
				 struct v4l2_format *f)
{
	struct soc_camera_host *ici = to_soc_camera_host(icd->dev.parent);
	struct sh_mobile_rcu_dev *pcdev = ici->priv;
	struct sh_mobile_rcu_cam *cam = icd->host_priv;
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	struct v4l2_pix_format *pix = &f->fmt.pix;
	struct v4l2_mbus_framefmt mf;
	struct device *dev = icd->dev.parent;
	__u32 pixfmt = pix->pixelformat;
	const struct soc_camera_format_xlate *xlate;
	int ret;
	unsigned int walign, halign;
	struct v4l2_rect rect;

	dev_geo(dev, "%s(): S_FMT(%c%c%c%c, %ux%u)\n", __func__,
		PRINT_FOURCC(pixfmt), pix->width, pix->height);

	xlate = soc_camera_xlate_by_fourcc(icd, pixfmt);
	if (!xlate) {
		dev_warn(dev, "Format %c%c%c%c not found\n",
			PRINT_FOURCC(pixfmt));
		return -EINVAL;
	}

	/* set format */
	mf.width		= pix->width;
	mf.height		= pix->height;
	mf.field		= pix->field;
	mf.colorspace	= pix->colorspace;
	mf.code			= xlate->code;

	ret = v4l2_device_call_until_err(sd->v4l2_dev,
			0, video, s_mbus_fmt, &mf);
	dev_geo(dev, "fmt %ux%u, requested %ux%u\n",
		mf.width, mf.height, pix->width, pix->height);
	if (ret < 0)
		return ret;

	if (mf.code != xlate->code)
		return -EINVAL;

	/* Cache current client geometry */
	ret = client_g_rect(sd, &rect);
	if (ret < 0)
		return ret;

	get_alignment(pixfmt, pcdev->output_mode, &walign, &halign);
	mf.width  = ROUNDDOWN(mf.width,  1<<walign);
	mf.height = ROUNDDOWN(mf.height, 1<<halign);
	if (mf.width  < pix->width)
		pix->width  = mf.width;
	if (mf.height < pix->height)
		pix->height = mf.height;

	/* Prepare RCU crop */
	cam->width			= pix->width;
	cam->height			= pix->height;
	cam->rcu_left		= rect.left;
	cam->rcu_top		= rect.top;
	cam->code			= xlate->code;
	pix->field			= V4L2_FIELD_NONE;
	pix->colorspace		= mf.colorspace;
	icd->current_fmt	= xlate;

	switch (pixfmt) {
	case V4L2_PIX_FMT_NV12:
	case V4L2_PIX_FMT_NV21:
	case V4L2_PIX_FMT_NV16:
	case V4L2_PIX_FMT_NV61:
		pcdev->image_mode = SH_RCU_MODE_IMAGE;
		break;
	default:
		pcdev->image_mode = SH_RCU_MODE_DATA;
	}

	dev_geo(dev, "rcu crop: width=%d height=%d left=%d top=%d\n",
		cam->width, cam->height, cam->rcu_left, cam->rcu_top);

	return 0;
}

static int sh_mobile_rcu_try_fmt(struct soc_camera_device *icd,
				 struct v4l2_format *f)
{
	struct soc_camera_host *ici = to_soc_camera_host(icd->dev.parent);
	struct sh_mobile_rcu_dev *pcdev = ici->priv;
	const struct soc_camera_format_xlate *xlate;
	struct v4l2_pix_format *pix = &f->fmt.pix;
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	struct v4l2_mbus_framefmt mf;
	__u32 pixfmt = pix->pixelformat;
	int ret;
	unsigned int walign, halign;

	dev_geo(icd->dev.parent, "%s(): TRY_FMT(%c%c%c%c, %ux%u)\n", __func__,
		PRINT_FOURCC(pixfmt), pix->width, pix->height);

	xlate = soc_camera_xlate_by_fourcc(icd, pixfmt);
	if (!xlate) {
		dev_warn(icd->dev.parent, "Format %c%c%c%c not found\n",
			PRINT_FOURCC(pixfmt));
		return -EINVAL;
	}

	switch (pixfmt) {
	case V4L2_PIX_FMT_NV12:
	case V4L2_PIX_FMT_NV21:
	case V4L2_PIX_FMT_NV16:
	case V4L2_PIX_FMT_NV61:
		/* ISP module does not support YUV format. */
		if (pcdev->output_mode != SH_RCU_OUTPUT_MEM)
			return -EINVAL;
		break;
	default:
		break;
	}

	/* FIXME: calculate using depth and bus width */

	get_alignment(pixfmt, pcdev->output_mode, &walign, &halign);
	v4l_bound_align_image(&pix->width,  SH_RCU_MIN_WIDTH,
				SH_RCU_MAX_WIDTH,  walign,
				&pix->height, SH_RCU_MIN_HEIGHT,
				SH_RCU_MAX_HEIGHT, halign, 0);

	/* limit to sensor capabilities */
	mf.width	= pix->width;
	mf.height	= pix->height;
	mf.field	= pix->field;
	mf.code		= xlate->code;
	mf.colorspace	= pix->colorspace;

	ret = v4l2_device_call_until_err(sd->v4l2_dev, 0, video,
			try_mbus_fmt, &mf);
	dev_geo(icd->dev.parent, "try_fmt %ux%u, requested %ux%u\n",
		mf.width, mf.height, pix->width, pix->height);
	if (ret < 0)
		return ret;

	mf.width  = ROUNDDOWN(mf.width,  1<<walign);
	mf.height = ROUNDDOWN(mf.height, 1<<halign);
	if (mf.width  < pix->width)
		pix->width  = mf.width;
	if (mf.height < pix->height)
		pix->height = mf.height;

	pix->bytesperline =
		sh_mobile_rcu_bytes_per_line(pix->width,
				xlate->host_fmt, pcdev);
	if ((int)pix->bytesperline < 0)
		return pix->bytesperline;
	pix->sizeimage = pix->height * pix->bytesperline;

	pix->field		= mf.field;
	pix->colorspace	= mf.colorspace;

	dev_geo(icd->dev.parent,
		"pix format: width=%d height=%d stride=%d sizeimage=%d\n",
		pix->width, pix->height, pix->bytesperline, pix->sizeimage);

	return ret;
}

static unsigned int sh_mobile_rcu_poll(struct file *file, poll_table *pt)
{
	struct soc_camera_device *icd = file->private_data;

	dev_geo(icd->dev.parent, "%s():\n", __func__);

	return vb2_poll(&icd->vb2_vidq, file, pt);
}

static int sh_mobile_rcu_querycap(struct soc_camera_host *ici,
				  struct v4l2_capability *cap)
{
	dev_geo(ici->v4l2_dev.dev, "%s():\n", __func__);

	strlcpy(cap->card, "SuperH_Mobile_RCU", sizeof(cap->card));
	cap->version = KERNEL_VERSION(0, 0, 5);
	cap->capabilities = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING;
	return 0;
}

static void *sh_mobile_rcu_contig_get_userptr(void *alloc_ctx, unsigned long vaddr,
					unsigned long size, int write)
{
	return NULL;
}

static void sh_mobile_rcu_contig_put_userptr(void *mem_priv)
{
	return;
}

const struct vb2_mem_ops sh_mobile_rcu_memops = {
	.get_userptr	= sh_mobile_rcu_contig_get_userptr,
	.put_userptr	= sh_mobile_rcu_contig_put_userptr,
};

static int sh_mobile_rcu_init_videobuf(struct vb2_queue *q,
				       struct soc_camera_device *icd)
{
	dev_geo(icd->dev.parent, "%s():\n", __func__);

	q->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	q->io_modes = VB2_MMAP | VB2_USERPTR;
	q->drv_priv = icd;
	q->ops = &sh_mobile_rcu_videobuf_ops;
	q->mem_ops = &sh_mobile_rcu_memops;
	q->buf_struct_size = sizeof(struct sh_mobile_rcu_buffer);

	return vb2_queue_init(q);
}

static int sh_mobile_rcu_get_ctrl(struct soc_camera_device *icd,
				  struct v4l2_control *ctrl)
{
	dev_geo(icd->dev.parent, "%s():\n", __func__);

	return -ENOIOCTLCMD;
}

static int sh_mobile_rcu_set_ctrl(struct soc_camera_device *icd,
				  struct v4l2_control *ctrl)
{
	struct soc_camera_host *ici = to_soc_camera_host(icd->dev.parent);
	struct sh_mobile_rcu_dev *pcdev = ici->priv;

	dev_geo(icd->dev.parent, "%s(): id=0x%x value=0x%x\n",
			__func__, ctrl->id, ctrl->value);

	switch (ctrl->id) {
	case V4L2_CID_SET_OUTPUT_MODE:
		if (SH_RCU_STREAMING_ON == pcdev->streaming)
			return -EBUSY;
		if ((SH_RCU_OUTPUT_ISP     != ctrl->value) &&
		    (SH_RCU_OUTPUT_MEM_ISP != ctrl->value) &&
		    (SH_RCU_OUTPUT_MEM     != ctrl->value)) {
			return -EINVAL;
		}
		pcdev->output_mode = ctrl->value;
		return 0;
	case V4L2_CID_SET_OUTPUT_OFFSET:
		pcdev->output_offset = ctrl->value;
		return 0;
	}
	return -ENOIOCTLCMD;
}

static struct soc_camera_host_ops sh_mobile_rcu_host_ops = {
	.owner		= THIS_MODULE,
	.add		= sh_mobile_rcu_add_device,
	.remove		= sh_mobile_rcu_remove_device,
	.get_formats	= sh_mobile_rcu_get_formats,
	.put_formats	= sh_mobile_rcu_put_formats,
	.set_fmt	= sh_mobile_rcu_set_fmt,
	.try_fmt	= sh_mobile_rcu_try_fmt,
	.set_ctrl	= sh_mobile_rcu_set_ctrl,
	.get_ctrl	= sh_mobile_rcu_get_ctrl,
	.poll		= sh_mobile_rcu_poll,
	.querycap	= sh_mobile_rcu_querycap,
	.set_bus_param	= sh_mobile_rcu_set_bus_param,
	.init_videobuf2	= sh_mobile_rcu_init_videobuf,
};

struct bus_wait {
	struct notifier_block	notifier;
	struct completion	completion;
	struct device		*dev;
};

static int bus_notify(struct notifier_block *nb,
		      unsigned long action, void *data)
{
	struct device *dev = data;
	struct bus_wait *wait = container_of(nb, struct bus_wait, notifier);

	dev_geo(dev, "%s():\n", __func__);

	if (wait->dev != dev)
		return NOTIFY_DONE;

	switch (action) {
	case BUS_NOTIFY_UNBOUND_DRIVER:
		/* Protect from module unloading */
		wait_for_completion(&wait->completion);
		return NOTIFY_OK;
	}
	return NOTIFY_DONE;
}

static int __devinit sh_mobile_rcu_probe(struct platform_device *pdev)
{
	struct sh_mobile_rcu_dev *pcdev;
	struct resource *res;
	void __iomem *base;
	unsigned int irq;
	int err = 0;
	struct bus_wait wait = {
		.completion = COMPLETION_INITIALIZER_ONSTACK(wait.completion),
		.notifier.notifier_call = bus_notify,
	};
	struct device *csi2;

	dev_err(&pdev->dev, "%s():\n", __func__);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	irq = platform_get_irq(pdev, 0);
	if (!res || (int)irq <= 0) {
		dev_err(&pdev->dev, "Not enough RCU platform resources.\n");
		err = -ENODEV;
		goto exit;
	}

	pcdev = kzalloc(sizeof(*pcdev), GFP_KERNEL);
	if (!pcdev) {
		dev_err(&pdev->dev, "Could not allocate pcdev\n");
		err = -ENOMEM;
		goto exit;
	}

	INIT_LIST_HEAD(&pcdev->capture);
	spin_lock_init(&pcdev->lock);

	pcdev->pdata = pdev->dev.platform_data;
	if (!pcdev->pdata) {
		err = -EINVAL;
		dev_err(&pdev->dev, "RCU platform data not set.\n");
		goto exit_kfree;
	}

	base = ioremap_nocache(res->start, resource_size(res));
	if (!base) {
		err = -ENXIO;
		dev_err(&pdev->dev, "Unable to ioremap RCU registers.\n");
		goto exit_kfree;
	}

	pcdev->irq = irq;
	pcdev->base = base;
	pcdev->video_limit = 0; /* only enabled if second resource exists */
	pcdev->image_mode = SH_RCU_MODE_DATA;
	pcdev->output_mode = SH_RCU_OUTPUT_ISP;
	pcdev->output_offset = SH_RCU_OUTPUT_OFFSET_32B;
	pcdev->streaming = SH_RCU_STREAMING_OFF;

	pcdev->iclk = clk_get(NULL, "icb");
	if (IS_ERR(pcdev->iclk)) {
		pcdev->iclk = NULL;
		dev_err(&pdev->dev, "cannot get clock \"icb\"\n");
	}
	pcdev->fclk = clk_get(NULL, pcdev->pdata->mod_name);
	if (IS_ERR(pcdev->fclk)) {
		pcdev->fclk = NULL;
		dev_err(&pdev->dev, "cannot get clock \"%s\"\n",
				pcdev->pdata->mod_name);
	}
	pcdev->mclk = clk_get(NULL, "meram");
	if (IS_ERR(pcdev->mclk)) {
		pcdev->mclk = NULL;
		dev_err(&pdev->dev, "cannot get clock \"meram\"\n");
	}
	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (res) {
		err = dma_declare_coherent_memory(&pdev->dev, res->start,
						  res->start,
						  resource_size(res),
						  DMA_MEMORY_MAP |
						  DMA_MEMORY_EXCLUSIVE);
		if (!err) {
			dev_err(&pdev->dev, "Unable to declare RCU memory.\n");
			err = -ENXIO;
			goto exit_iounmap;
		}

		pcdev->video_limit = resource_size(res);
	}

	platform_set_drvdata(pdev, pcdev);

	pm_suspend_ignore_children(&pdev->dev, true);
	pm_runtime_enable(&pdev->dev);
	pm_runtime_resume(&pdev->dev);

	pcdev->ici.priv = pcdev;
	pcdev->ici.v4l2_dev.dev = &pdev->dev;
	pcdev->ici.nr = pdev->id;
	pcdev->ici.drv_name = dev_name(&pdev->dev);
	pcdev->ici.ops = &sh_mobile_rcu_host_ops;

	/* CSI2 interfacing */
	csi2 = pcdev->pdata->csi2_dev;
	dev_geo(&pdev->dev, "CSI2 interface %p\n", csi2);
	if (csi2) {
		wait.dev = csi2;

		err = bus_register_notifier(&platform_bus_type, &wait.notifier);
		if (err < 0)
			goto exit_free_clk;

		/*
		 * From this point the driver module will not unload, until
		 * we complete the completion.
		 */

		if (!csi2->driver) {
			complete(&wait.completion);
			/* Either too late, or probing failed */
			bus_unregister_notifier(&platform_bus_type,
						&wait.notifier);
			err = -ENXIO;
			goto exit_free_clk;
		}

		/*
		 * The module is still loaded, in the worst case it is hanging
		 * in device release on our completion. So, _now_ dereferencing
		 * the "owner" is safe!
		 */

		err = try_module_get(csi2->driver->owner);

		/* Let notifier complete, if it has been locked */
		complete(&wait.completion);
		bus_unregister_notifier(&platform_bus_type, &wait.notifier);
		if (!err) {
			err = -ENODEV;
			goto exit_free_clk;
		}
	}

	pcdev->alloc_ctx = vb2_dma_contig_init_ctx(&pdev->dev);
	if (IS_ERR(pcdev->alloc_ctx)) {
		err = PTR_ERR(pcdev->alloc_ctx);
		goto exit_module_put;
	}

	err = soc_camera_host_register(&pcdev->ici);
	if (err)
		goto exit_free_ctx;

	dev_err(&pdev->dev, "%s():out\n", __func__);
	return 0;

exit_free_ctx:
	vb2_dma_contig_cleanup_ctx(pcdev->alloc_ctx);
exit_module_put:
	if (csi2 && csi2->driver)
		module_put(csi2->driver->owner);
exit_free_clk:
	pm_runtime_disable(&pdev->dev);
	if (platform_get_resource(pdev, IORESOURCE_MEM, 1))
		dma_release_declared_memory(&pdev->dev);
exit_iounmap:
	iounmap(base);
exit_kfree:
	kfree(pcdev);
exit:
	return err;
}

static int __devexit sh_mobile_rcu_remove(struct platform_device *pdev)
{
	struct soc_camera_host *soc_host = to_soc_camera_host(&pdev->dev);
	struct sh_mobile_rcu_dev *pcdev = container_of(soc_host,
					struct sh_mobile_rcu_dev, ici);
	struct device *csi2 = pcdev->pdata->csi2_dev;

	dev_geo(&pdev->dev, "%s():\n", __func__);

	soc_camera_host_unregister(soc_host);
	pm_runtime_disable(&pdev->dev);
	if (platform_get_resource(pdev, IORESOURCE_MEM, 1))
		dma_release_declared_memory(&pdev->dev);
	iounmap(pcdev->base);
	vb2_dma_contig_cleanup_ctx(pcdev->alloc_ctx);
	if (csi2 && csi2->driver)
		module_put(csi2->driver->owner);
	kfree(pcdev);

	return 0;
}

static int sh_mobile_rcu_suspend(struct device *dev)
{
	return 0;
}

static int sh_mobile_rcu_resume(struct device *dev)
{
	return 0;
}


static int sh_mobile_rcu_runtime_suspend(struct device *dev)
{
	struct sh_mobile_rcu_dev *pcdev;
	if (NULL == dev)
		return 0;

	pcdev = dev_get_drvdata(dev);

	/*
	 * This driver re-initializes all registers after
	 * pm_runtime_get_sync() anyway so there is no need
	 * to save and restore registers here.
	 */

	if (NULL == pcdev) {
		dev_err(pcdev->icd->dev.parent,
			"%s, pcdev is NULL\n", __func__);
		return 0;
	}

	if (NULL != pcdev->iclk)
		clk_disable(pcdev->iclk);
	else {
		dev_err(pcdev->icd->dev.parent,
			"%s, pcdev->iclk is NULL\n", __func__);
	}

	if (NULL != pcdev->fclk)
		clk_disable(pcdev->fclk);
	else {
		dev_err(pcdev->icd->dev.parent,
			"%s, pcdev->fclk is NULL\n", __func__);
	}

	if (NULL != pcdev->mclk)
		clk_disable(pcdev->mclk);
	else {
		dev_err(pcdev->icd->dev.parent,
			"%s, pcdev->mclk is NULL\n", __func__);
	}

	return 0;
}

static int sh_mobile_rcu_runtime_resume(struct device *dev)
{
	struct sh_mobile_rcu_dev *pcdev;
	if (NULL == dev)
		return 0;

	pcdev = dev_get_drvdata(dev);

	if (NULL == pcdev) {
		dev_err(pcdev->icd->dev.parent,
			"%s, pcdev is NULL\n", __func__);
		return 0;
	}

	if (NULL != pcdev->iclk)
		clk_enable(pcdev->iclk);
	else {
		dev_err(pcdev->icd->dev.parent,
			"%s, pcdev->iclk is NULL\n", __func__);
	}

	if (NULL != pcdev->fclk)
		clk_enable(pcdev->fclk);
	else {
		dev_err(pcdev->icd->dev.parent,
			"%s, pcdev->fclk is NULL\n", __func__);
	}

	if (NULL != pcdev->mclk)
		clk_enable(pcdev->mclk);
	else {
		dev_err(pcdev->icd->dev.parent,
			"%s, pcdev->mclk is NULL\n", __func__);
	}

	return 0;

}

static const struct dev_pm_ops sh_mobile_rcu_dev_pm_ops = {
	.suspend = sh_mobile_rcu_suspend,
	.resume = sh_mobile_rcu_resume,
	.runtime_suspend = sh_mobile_rcu_runtime_suspend,
	.runtime_resume = sh_mobile_rcu_runtime_resume,
};

static struct platform_driver sh_mobile_rcu_driver = {
	.driver		= {
		.name	= "sh_mobile_rcu",
		.pm	= &sh_mobile_rcu_dev_pm_ops,
	},
	.probe		= sh_mobile_rcu_probe,
	.remove		= __devexit_p(sh_mobile_rcu_remove),
};

static int __init sh_mobile_rcu_init(void)
{
	printk(KERN_DEBUG "%s():\n", __func__);

	/* Whatever return code */
	request_module("sh_mobile_csi2");
	return platform_driver_register(&sh_mobile_rcu_driver);
}

static void __exit sh_mobile_rcu_exit(void)
{
	printk(KERN_DEBUG "%s():\n", __func__);

	platform_driver_unregister(&sh_mobile_rcu_driver);
}

module_init(sh_mobile_rcu_init);
module_exit(sh_mobile_rcu_exit);

MODULE_DESCRIPTION("SuperH Mobile RCU driver");
MODULE_AUTHOR("Renesas Mobile Corp.");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:sh_mobile_rcu");
