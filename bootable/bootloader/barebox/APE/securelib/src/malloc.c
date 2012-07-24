/*
 * malloc.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "malloc.h"

#define PTR_MALLOC_START					((volatile ulong *)(MALLOC_START))
#define PTR_MALLOC_END						((volatile ulong *)(MALLOC_START + sizeof(ulong)))
#define PTR_MALLOC_BRK						((volatile ulong *)(MALLOC_START + (2 * sizeof(ulong))))
#define PTR_DICTIONARY_CT					((volatile ulong *)(MALLOC_START + (3 * sizeof(ulong))))
#define PTR_DICTIONARY						((volatile mem_dictionary *)(MALLOC_START + (4 * sizeof(ulong))))

unsigned long mem_malloc_start(void)
{
	return *PTR_MALLOC_START;
}

unsigned long mem_malloc_end(void)
{
	return *PTR_MALLOC_END;
}

void mem_malloc_init(void)
{
	*PTR_MALLOC_START = (unsigned long)MALLOC_START;
	*PTR_MALLOC_END = (unsigned long)MALLOC_END;
	*PTR_MALLOC_BRK = (unsigned long)PTR_DICTIONARY + (1024 * sizeof(mem_dictionary));
	*PTR_DICTIONARY_CT = 0;
}

static void *sbrk_no_zero(ptrdiff_t increment)
{
	unsigned long old = *PTR_MALLOC_BRK;
	unsigned long new = old + increment;

	if ((new < *PTR_MALLOC_START) || (new > *PTR_MALLOC_END))
		return NULL;

	*PTR_MALLOC_BRK = new;

	return (void *)old;
}

void *sbrk(ptrdiff_t increment)
{
	void *old = sbrk_no_zero(increment);

	/* Only clear increment, if valid address was returned */
	if (old != NULL)
		memset(old, 0, increment);

	return old;
}

void *malloc(size_t size)
{
  void *return_ptr = sbrk(size);
 
  PTR_DICTIONARY[*PTR_DICTIONARY_CT].addr = return_ptr;
  PTR_DICTIONARY[*PTR_DICTIONARY_CT].size = size;
  (*PTR_DICTIONARY_CT)++;
  
  return return_ptr;
}

void *calloc(size_t num, size_t size)
{
	size_t tmp = num * size;
	void * ptr = malloc(tmp);
	
	return ptr;
}

void free(void *ptr)
{
	/* dummy */
}
