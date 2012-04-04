/*
 * drivers/char/rtapi/include/system_memory.h
 *     This file is memory manager function.
 *
 * Copyright (C) 2011 Renesas Electronics Corporation
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
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef __SYSTEM_MEMORY_H__
#define __SYSTEM_MEMORY_H__

/*  definition  */
/* return code */
#define SMAP_LIB_MEMORY_OK          (0)
#define SMAP_LIB_MEMORY_NG          (-1)
#define SMAP_LIB_MEMORY_PARA_NG     (-2)
#define SMAP_LIB_MEMORY_NO_MEMORY   (-3)

/* Memory area type */
#define RT_MEMORY_WORK          (1)
#define RT_MEMORY_WORKSMALL     (2)
#define RT_MEMORY_MFIWORKAPP    (3)
#define RT_MEMORY_MFIWORKRT     (4)

/* cache type */
#define RT_MEMORY_WRITEBACK         0x00000000
#define RT_MEMORY_WRITETHROUGH      0x00000001
#define RT_MEMORY_NONCACHE          0x00000002
#define RT_MEMORY_BUFFER_NONCACHE   0x00000004

/* app memory align macro */
#define RT_MEMORY_APAREA_SIZE(x) (((x + 31) & (~31)) + 32)

/* this structure is for information on the shared memory. */
typedef struct
{
    unsigned int    mem_size;
    unsigned int    free_size;
    unsigned int    max_free_size;
} memory_info;

/* RT-API Parameter */
/* system_memory_rt_alloc() */
typedef struct
{
    void*           handle;
    unsigned int    alloc_size;
    unsigned int    rtmem_apaddr;
} system_mem_rt_alloc;

/* system_memory_rt_free() */
typedef struct
{
    void*           handle;
    unsigned int    rtmem_apaddr;
} system_mem_rt_free;

/* system_memory_rt_change_rtaddr() */
typedef struct
{
    void*           handle;
    unsigned int    rtmem_apaddr;
    unsigned int    rtmem_rtaddr;
} system_mem_rt_change_rtaddr;

/* system_memory_rt_change_apaddr() */
typedef struct
{
    void*           handle;
    unsigned int    rtmem_rtaddr;
    unsigned int    rtmem_apaddr;
} system_mem_rt_change_apaddr;

/* system_memory_ap_open() */
/* system_memory_ap_open_wb() */
typedef struct
{
    void*           handle;
    unsigned int    aparea_size;
    unsigned int    cache_kind;
    void*           apmem_handle;
} system_mem_ap_open;

/* system_memory_ap_close() */
typedef struct
{
    void*           handle;
    void*           apmem_handle;
} system_mem_ap_close;

/* system_memory_ap_alloc() */
typedef struct
{
    void*           handle;
    unsigned int    alloc_size;
    void*           apmem_handle;
    unsigned int    apmem_apaddr;
} system_mem_ap_alloc;

/* system_memory_ap_free() */
typedef struct
{
    void*           handle;
    void*           apmem_handle;
    unsigned int apmem_apaddr;
} system_mem_ap_free;

/* system_memory_ap_change_rtaddr() */
typedef struct
{
    void*           handle;
    unsigned int    cache_kind;
    void*           apmem_handle;
    unsigned int    apmem_apaddr;
    unsigned int    apmem_rtaddr;
} system_mem_ap_change_rtaddr;

/* system_memory_ap_change_apaddr() */
typedef struct
{
    void*           handle;
    void*           apmem_handle;
    unsigned int    apmem_rtaddr;
    unsigned int    apmem_apaddr;
} system_mem_ap_change_apaddr;

/* system_memory_ap_cache_flush() */
typedef struct
{
    void*           handle;
    void*           apmem_handle;
    unsigned int    apmem_apaddr;
    unsigned int    flush_size;
} system_mem_ap_cache_flush;

/* system_memory_ap_cache_clear() */
typedef struct
{
    void*           handle;
    void*           apmem_handle;
    unsigned int    apmem_apaddr;
    unsigned int    clear_size;
} system_mem_ap_cache_clear;

/* system_memory_ap_buffer_flush() */
typedef struct
{
    void*           handle;
} system_mem_ap_buffer_flush;

/* system_memory_ap_get_apmem_id() */
typedef struct
{
    void*           handle;
    void*           apmem_handle;
} system_mem_ap_get_apmem_id;

/* system_memory_ap_share_area() */
typedef struct
{
    void*           handle;
    unsigned int    apmem_id;
    void*           apmem_handle;
} system_mem_ap_share_area;

/* system_memory_ap_share_mem_offset() */
typedef struct
{
    void*           handle;
    void*           apmem_handle;
    unsigned int    apmem_apaddr;
    unsigned int    apmem_offset;
} system_mem_ap_share_mem_offset;

/* system_memory_ap_share_mem() */
typedef struct
{
    void*           handle;
    void*           apmem_handle;
    unsigned int    apmem_offset;
    unsigned int    apmem_apaddr;
} system_mem_ap_share_mem;

/* system_memory_rt_map() */
typedef struct
{
    void*           handle;
    unsigned int    phys_addr;
    unsigned int    map_size;
    unsigned int    rtaddr;
} system_mem_rt_map;

/* system_memory_rt_unmap() */
typedef struct
{
    void*           handle;
    unsigned int    rtaddr;
    unsigned int    map_size;
} system_mem_rt_unmap;

/* system_memory_get_rtinfo() */
typedef struct
{
    void*           handle;
    short           memory_type;
    memory_info     mem_info;
} system_mem_get_rtinfo;

/* system_memory_get_apinfo() */
typedef struct
{
    void*           handle;
    unsigned int    apmem_id;
    memory_info     mem_info;
} system_mem_get_apinfo;

/* system_memory_info_delete() */
typedef struct
{
    void*           handle;
} system_mem_info_delete;

/* system_memory_meram_alloc() */
typedef struct
{
    void*           handle;
    unsigned int    alloc_size;
    unsigned int    meram_offset;
    unsigned int    ch_num;
} system_mem_meram_alloc;

/* system_memory_meram_free() */
typedef struct
{
    void*           handle;
    unsigned int    ch_num;
} system_mem_meram_free;

/* function kernel side */
extern int system_memory_rt_alloc
(
    system_mem_rt_alloc* rt_alloc
);

extern int system_memory_rt_free
(
    system_mem_rt_free* rt_free
);

extern int system_memory_rt_change_rtaddr
(
    system_mem_rt_change_rtaddr* rt_change_rtaddr
);

extern int system_memory_rt_change_apaddr
(
    system_mem_rt_change_apaddr* rt_change_apaddr
);

extern int system_memory_ap_open
(
    system_mem_ap_open*   ap_open
);

extern int system_memory_ap_open_wb
(
    system_mem_ap_open*   ap_open
);

extern int system_memory_ap_close
(
    system_mem_ap_close*  ap_close
);

extern int system_memory_ap_alloc
(
    system_mem_ap_alloc*  ap_alloc
);

extern int system_memory_ap_free
(
    system_mem_ap_free*   ap_free
);

extern int system_memory_ap_change_rtaddr
(
    system_mem_ap_change_rtaddr* ap_change_rtaddr
);

extern int system_memory_ap_change_apaddr
(
    system_mem_ap_change_apaddr* ap_change_apaddr
);

extern int system_memory_ap_cache_flush
(
    system_mem_ap_cache_flush* ap_cache_flush
);

extern int system_memory_ap_cache_clear
(
    system_mem_ap_cache_clear* ap_cache_clear
);

extern void system_memory_ap_buffer_flush
(
    system_mem_ap_buffer_flush* apm_buffer_flush
);

extern unsigned int system_memory_ap_get_apmem_id
(
    system_mem_ap_get_apmem_id* ap_get_apmem_id
);

extern int system_memory_ap_share_area
(
    system_mem_ap_share_area* ap_share_area
);

extern int system_memory_ap_share_mem_offset
(
    system_mem_ap_share_mem_offset* ap_share_mem_offset
);

extern int system_memory_ap_share_mem
(
    system_mem_ap_share_mem* ap_share_mem
);

extern int system_memory_rt_map
(
    system_mem_rt_map*   rt_map
);

extern int system_memory_rt_unmap
(
    system_mem_rt_unmap*  rt_unmap
);

extern void* system_memory_info_new
(
    void
);

extern int system_memory_get_rtinfo
(
    system_mem_get_rtinfo*  get_rtinfo
);

extern int system_memory_get_apinfo
(
    system_mem_get_apinfo*  get_apinfo
);

extern void system_memory_info_delete
(
    system_mem_info_delete*  info_delete
);

extern int system_memory_meram_alloc
(
    system_mem_meram_alloc*  meram_alloc
);

extern int system_memory_meram_free
(
    system_mem_meram_free*  meram_free
);

#endif  /* __SYSTEM_MEMORY_H__ */

