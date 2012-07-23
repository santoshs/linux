/* string.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef __STRING_H__
#define __STRING_H__
 
#include "com_type.h"
#include "libc.h"

extern int strcmp(const char *cs, const char *ct);

extern int strncmp(const char *cs, const char *ct, size_t count);

extern size_t strlen(const char *s);

extern char* strcpy(char *dest,const char *src);

extern int memcmp(const void *cs,const void *ct,size_t count);

extern void* memcpy(void *, const void *, long);

extern void* memmove(void *, const void *, long);

extern void* memchr(const void *, int, long);

extern void* memset(void *, int, long);

extern void uppercase(char *in);

#endif	/* __STRING_H__ */
