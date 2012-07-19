/* vcd_spuv.h
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
#ifndef __VCD_SPUV_H__
#define __VCD_SPUV_H__

/*
 * constant definition
 */


/*
 * define macro declaration
 */


/*
 * enum declaration
 */


/*
 * structure declaration
 */


/*
 * table declaration
 */


/*
 * prototype declaration
 */
extern int vcd_spuv_ioremap(void);
extern void vcd_spuv_iounmap(void);
extern int vcd_spuv_get_fw_buffer(void);
extern void vcd_spuv_free_fw_buffer(void);
extern int vcd_spuv_get_msg_buffer(void);
extern int vcd_spuv_start_vcd(void);
extern int vcd_spuv_stop_vcd(void);
extern int vcd_spuv_set_hw_param(void);
extern int vcd_spuv_start_call(void);
extern int vcd_spuv_stop_call(void);
extern int vcd_spuv_set_udata(void);
extern int vcd_spuv_start_record(struct vcd_record_option *option);
extern int vcd_spuv_stop_record(void);
extern int vcd_spuv_start_playback(struct vcd_playback_option *option);
extern int vcd_spuv_stop_playback(void);
extern void vcd_spuv_get_record_buffer(struct vcd_record_buffer_info *info);
extern void vcd_spuv_get_playback_buffer(struct vcd_playback_buffer_info *info);

extern int vcd_spuv_start_1khz_tone(void);
extern int vcd_spuv_stop_1khz_tone(void);
extern int vcd_spuv_start_pcm_loopback(int mode);
extern int vcd_spuv_stop_pcm_loopback(void);
extern int vcd_spuv_start_bbif_loopback(int mode);
extern int vcd_spuv_stop_bbif_loopback(void);
extern int vcd_spuv_set_trace_select(void);

extern int vcd_spuv_create_queue(void);
extern void vcd_spuv_destroy_queue(void);

extern void vcd_spuv_dump_status(void);
extern void vcd_spuv_dump_registers(void);
extern void vcd_spuv_dump_cpg_registers(void);
extern void vcd_spuv_dump_crmu_registers(void);
extern void vcd_spuv_dump_gtu_registers(void);
extern void vcd_spuv_dump_voiceif_registers(void);
extern void vcd_spuv_dump_intcvo_registers(void);
extern void vcd_spuv_dump_spuv_registers(void);
extern void vcd_spuv_dump_dsp0_registers(void);


#endif /* __VCD_SPUV_H__ */
