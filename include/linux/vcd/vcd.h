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
#ifndef __VCD_H__
#define __VCD_H__

#ifndef __KERNEL__
#else	/* __KERNEL__ */

/*
 * constant definition
 */
#define VCD_RECORD_BUFFER_SIZE 640
#define VCD_PLAYBACK_BUFFER_SIZE 640


/*
 * define macro declaration
 */


/*
 * enum declaration
 */
enum VCD_COMMAND {
	VCD_COMMAND_SET_CALL_MODE = 0,
	VCD_COMMAND_START_RECORD,
	VCD_COMMAND_STOP_RECORD,
	VCD_COMMAND_START_PLAYBACK,
	VCD_COMMAND_STOP_PLAYBACK,
	VCD_COMMAND_GET_RECORD_BUFFER,
	VCD_COMMAND_GET_PLAYBACK_BUFFER,
	VCD_COMMAND_WATCH_STOP_FW,
	VCD_COMMAND_WATCH_START_CLKGEN,

	VCD_COMMAND_MAX
};

enum VCD_CALL_TYPE {
	VCD_CALL_TYPE_CS = 0,
	VCD_CALL_TYPE_1KHZ,
	VCD_CALL_TYPE_PCM_LB,
	VCD_CALL_TYPE_VIF_LB
};

enum VCD_RECORD_MODE {
	VCD_RECORD_MODE_0 = 0,
	VCD_RECORD_MODE_1,
	VCD_RECORD_MODE_2
};

enum VCD_PLAYBACK_MODE {
	VCD_PLAYBACK_MODE_0 = 0,
	VCD_PLAYBACK_MODE_1,
	VCD_PLAYBACK_MODE_2
};

enum VCD_LOOPBACK_MODE {
	VCD_LOOPBACK_MODE_INTERFACE = 0,
	VCD_LOOPBACK_MODE_PCM,
};


/*
 * structure declaration
 */
struct vcd_execute_command {
	unsigned int command;
	void *arg;
};

struct vcd_call_option {
	unsigned int call_type;
	unsigned int loopback_mode;
};

struct vcd_record_option {
	unsigned int mode;
	void *complete_buffer;
};

struct vcd_playback_option {
	unsigned int mode;
	void *beginning_buffer;
};

struct vcd_record_buffer_info {
	unsigned int *record_buffer[2];
};

struct vcd_playback_buffer_info {
	unsigned int *playback_buffer[2];
};


/*
 * table declaration
 */


/*
 * prototype declaration
 */

/* for sound */
extern int vcd_execute(const struct vcd_execute_command *args);
/* for audio ic */
extern int vcd_execute_test_call(const struct vcd_execute_command *args);



#endif	/* __KERNEL__ */
#endif	/* __VCD_H__ */
