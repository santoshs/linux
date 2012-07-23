/*
 * fb_comm.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */
#ifndef __FB_COMM_H__
#define __FB_COMM_H__
 
#define PACK_FS			64
#define PACK_HS			512
#define MAX_PACK_SIZE	PACK_HS

void fb_comm_init(void);
void fb_comm_connect(void);
int fb_comm_detect_conn(void);
void fb_comm_get_cmd(unsigned char *fb_buff);
int fb_comm_get_data(unsigned char *fb_buff, unsigned long length,
												unsigned long timeout);
int fb_comm_respond(char *msg);

#endif /* __FB_COMM_H__ */
