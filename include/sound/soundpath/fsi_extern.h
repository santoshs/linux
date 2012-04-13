/* fsi_extern.h
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


#ifndef __FSI_EXTERN_H__
#define __FSI_EXTERN_H__

#include <linux/kernel.h>

#ifdef __FSI_CTRL_NO_EXTERN__
#define FSI_CTRL_NO_EXTERN
#else
#define FSI_CTRL_NO_EXTERN	extern
#endif

/* FSI start function */
FSI_CTRL_NO_EXTERN int fsi_start(const u_int uiValue);
/* FSI stop function */
FSI_CTRL_NO_EXTERN void fsi_stop(void);
/* DMA transfer function */
FSI_CTRL_NO_EXTERN void fsi_dma_start(void);
/* DMA transfer function */
FSI_CTRL_NO_EXTERN void fsi_dma_stop(void);
/* registers dump function */
FSI_CTRL_NO_EXTERN void fsi_reg_dump(const u_int uiValue);
/* FSI soft reset function */
FSI_CTRL_NO_EXTERN void fsi_soft_reset(void);

#ifdef SOUND_TEST
FSI_CTRL_NO_EXTERN int fsi_play_test_start_a(char *buf, u_int size);
FSI_CTRL_NO_EXTERN void fsi_play_test_stop_a(void);
FSI_CTRL_NO_EXTERN int fsi_rec_test_start_a(char *buf, u_int size);
FSI_CTRL_NO_EXTERN int fsi_rec_test_stop_a(void);
FSI_CTRL_NO_EXTERN int fsi_voice_test_start_a(void);
FSI_CTRL_NO_EXTERN void fsi_voice_test_stop_a(void);
FSI_CTRL_NO_EXTERN void fsi_test_interrupt_a(void);
#ifdef NO_INTURRUPT
FSI_CTRL_NO_EXTERN void fsi_test_fifo_write_a(void);
FSI_CTRL_NO_EXTERN void fsi_test_fifo_read_a(void);
#endif /* NO_INTURRUPT */
#endif /* SOUNT_TEST */

#endif /* __FSI_EXTERN_H__ */


