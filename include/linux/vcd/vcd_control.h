/* vcd_control.h
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
#ifndef __VCD_CONTROL_H__
#define __VCD_CONTROL_H__

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
extern int vcd_ctrl_get_msg_buffer(void);
extern int vcd_ctrl_get_async_area(void);
extern int vcd_ctrl_free_async_area(unsigned int adr);
extern int vcd_ctrl_start_vcd(void);
extern int vcd_ctrl_stop_vcd(void);
extern int vcd_ctrl_set_hw_param(void);
extern int vcd_ctrl_start_call(int call_type, int mode);
extern int vcd_ctrl_stop_call(int call_type);
extern int vcd_ctrl_set_udata(void);
extern void vcd_ctrl_get_status(void);
extern int vcd_ctrl_get_result(void);

extern int vcd_ctrl_start_record(struct vcd_record_option *option);
extern int vcd_ctrl_stop_record(void);
extern int vcd_ctrl_start_playback(struct vcd_playback_option *option);
extern int vcd_ctrl_stop_playback(void);
extern void vcd_ctrl_get_record_buffer(struct vcd_record_buffer_info *info);
extern void vcd_ctrl_get_playback_buffer(struct vcd_playback_buffer_info *info);
extern void vcd_ctrl_get_voip_ul_buffer(struct vcd_voip_ul_buffer_info *info);
extern void vcd_ctrl_get_voip_dl_buffer(struct vcd_voip_dl_buffer_info *info);

extern void vcd_ctrl_rec_trigger(void);
extern void vcd_ctrl_play_trigger(void);
extern void vcd_ctrl_stop_fw(void);
extern void vcd_ctrl_udata_ind(void);
extern void vcd_ctrl_start_clkgen(void);
extern void vcd_ctrl_stop_clkgen(void);
extern void vcd_ctrl_wait_path(void);

extern int vcd_ctrl_suspend(void);
extern int vcd_ctrl_resume(void);
extern int vcd_ctrl_runtime_suspend(void);
extern int vcd_ctrl_runtime_resume(void);
extern int vcd_ctrl_probe(void);
extern int vcd_ctrl_remove(void);

extern void vcd_ctrl_dump_status(void);
extern void vcd_ctrl_dump_registers(void);
extern void vcd_ctrl_dump_hpb_registers(void);
extern void vcd_ctrl_dump_cpg_registers(void);
extern void vcd_ctrl_dump_crmu_registers(void);
extern void vcd_ctrl_dump_gtu_registers(void);
extern void vcd_ctrl_dump_voiceif_registers(void);
extern void vcd_ctrl_dump_intcvo_registers(void);
extern void vcd_ctrl_dump_spuv_registers(void);
extern void vcd_ctrl_dump_dsp0_registers(void);
extern void vcd_ctrl_dump_memories(void);
extern void vcd_ctrl_dump_pram0_memory(void);
extern void vcd_ctrl_dump_xram0_memory(void);
extern void vcd_ctrl_dump_yram0_memory(void);
extern void vcd_ctrl_dump_dspio_memory(void);
extern void vcd_ctrl_dump_sdram_static_area_memory(void);
extern void vcd_ctrl_dump_fw_static_buffer_memory(void);

#endif /* __VCD_CONTROL_H__ */
