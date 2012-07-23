/*
 * malloc.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef _MALLOC_H_
#define _MALLOC_H_

#include "com_type.h"
#include "string.h"

#define MALLOC_START	0xE63A0000	/* ICRAM0 start */
#define MALLOC_END		0xE63B1FFF  /* ICRAM0 end */

typedef unsigned int 				ptrdiff_t;

typedef struct _mem_dictionary
{
  void *addr;
  size_t size;
} mem_dictionary;

unsigned long mem_malloc_start(void);
unsigned long mem_malloc_end(void);
void mem_malloc_init(void);
void *sbrk(ptrdiff_t increment);
void *malloc(size_t size);
void *calloc(size_t num, size_t size);
void free(void *ptr);

#endif /* __MALLOC_H__ */
