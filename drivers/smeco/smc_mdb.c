/*
*   Copyright � Renesas Mobile Corporation 2011. All rights reserved
*
*   This material, including documentation and any related source code
*   and information, is protected by copyright controlled by Renesas.
*   All rights are reserved. Copying, including reproducing, storing,
*   adapting, translating and modifying, including decompiling or
*   reverse engineering, any or all of this material requires the prior
*   written consent of Renesas. This material also contains
*   confidential information, which may not be disclosed to others
*   without the prior written consent of Renesas.
*/
#if 0
/*
Change history:

Version:       6    03-Jan-2012     Janne Mahosenaho
Status:        draft
Description :  Added TLSF statistics.
-------------------------------------------------------------------------------
Version:       5    28-Dec-2011     Janne Mahosenaho
Status:        draft
Description :  Fixes for Linux build.
-------------------------------------------------------------------------------
Version:       4    27-Dec-2011     Janne Mahosenaho
Status:        draft
Description :  Alternative TLSF implementation added.
-------------------------------------------------------------------------------
Version:       1    01-Dec-2011     Jussi Pellinen
Status:        draft
Description :  File created
-------------------------------------------------------------------------------
*/
#endif

#include "smc_common_includes.h"

#include "smc_conf.h"
#include "smc.h"
#include "smc_trace.h"
#include "smc_mdb.h"


    //
    // Global variables
    //
static smc_mdb_channel_info_set_t* mdb_channel_info_set = NULL;

static void mdb_info_store( smc_mdb_channel_info_t* channel_info );
static smc_mdb_channel_info_t* mdb_info_fetch( uint8_t channel_id );
static void mdb_channel_info_delete( void );

/* The  debug functions  only can  be used  when _DEBUG_TLSF_  is set. */
#ifndef _DEBUG_TLSF_
#define _DEBUG_TLSF_  (0)
#endif

#if !defined(__GNUC__)
#ifndef __inline__
#define __inline__ inline
#endif
#endif

/* This is set as FALSE by default. */
#if TLSF_USE_REF 

#ifndef USE_PRINTF
#define USE_PRINTF      (1)
#endif

#ifndef TLSF_USE_LOCKS
#define TLSF_USE_LOCKS  (0)
#endif

#ifndef TLSF_STATISTIC
  /*#define TLSF_STATISTIC  (0)*/
  #define TLSF_STATISTIC  (1)
#endif

#ifndef USE_MMAP
#define USE_MMAP    (0)
#endif

#ifndef USE_SBRK
#define USE_SBRK    (0)
#endif

#if TLSF_USE_LOCKS
  #include "target.h"
#else
  #define TLSF_CREATE_LOCK(_unused_)   do{}while(0)
  #define TLSF_DESTROY_LOCK(_unused_)  do{}while(0) 
  #define TLSF_ACQUIRE_LOCK(_unused_)  do{}while(0)
  #define TLSF_RELEASE_LOCK(_unused_)  do{}while(0)
#endif

/*************************************************************************/
/* Definition of the structures used by TLSF */


/* Some IMPORTANT TLSF parameters */
/* Unlike the preview TLSF versions, now they are statics */
#define BLOCK_ALIGN (sizeof(void *) * 2)

#define MAX_FLI         (30)
#define MAX_LOG2_SLI    (5)
#define MAX_SLI         (1 << MAX_LOG2_SLI)     /* MAX_SLI = 2^MAX_LOG2_SLI */

#define FLI_OFFSET      (6)     /* tlsf structure just will manage blocks bigger */
/* than 128 bytes */
#define SMALL_BLOCK     (128)
#define REAL_FLI        (MAX_FLI - FLI_OFFSET)
#define MIN_BLOCK_SIZE  (sizeof(free_ptr_t))
#define BHDR_OVERHEAD   (sizeof(bhdr_t) - MIN_BLOCK_SIZE)
#define TLSF_SIGNATURE  (0x2A59FA59)

#define PTR_MASK    (sizeof(void *) - 1)
#define BLOCK_SIZE  (0xFFFFFFFF - PTR_MASK)

#define GET_NEXT_BLOCK(_addr, _r) ((bhdr_t *) ((char *) (_addr) + (_r)))
#define MEM_ALIGN                 ((BLOCK_ALIGN) - 1)
#define ROUNDUP_SIZE(_r)          (((_r) + MEM_ALIGN) & ~MEM_ALIGN)
#define ROUNDDOWN_SIZE(_r)        ((_r) & ~MEM_ALIGN)
#define ROUNDUP(_x, _v)           ((((~(_x)) + 1) & ((_v)-1)) + (_x))

#define BLOCK_STATE (0x1)
#define PREV_STATE  (0x2)

/* bit 0 of the block size */
#define FREE_BLOCK  (0x1)
#define USED_BLOCK  (0x0)

/* bit 1 of the block size */
#define PREV_FREE   (0x2)
#define PREV_USED   (0x0)


#define DEFAULT_AREA_SIZE (1024*10)

#if TLSF_STATISTIC
#define TLSF_ADD_SIZE(tlsf, b)                                     \
    do                                                             \
    {                                                              \
        tlsf->used_size += (b->size & BLOCK_SIZE) + BHDR_OVERHEAD; \
        if (tlsf->used_size > tlsf->max_size)                      \
        {                                                          \
            tlsf->max_size = tlsf->used_size;                      \
        }                                                          \
    } while (0)

#define TLSF_REMOVE_SIZE(tlsf, b)                                  \
    do                                                             \
    {                                                              \
        tlsf->used_size -= (b->size & BLOCK_SIZE) + BHDR_OVERHEAD; \
    } while (0)
#else
#define TLSF_ADD_SIZE(tlsf, b)     \
    do                             \
    {                              \
    } while (0)   
#define TLSF_REMOVE_SIZE(tlsf, b)  \
    do                             \
    {                              \
    } while (0)
#endif

#ifdef USE_MMAP
  #define PAGE_SIZE (getpagesize())
#endif

#ifdef USE_PRINTF
  #define PRINT_MSG(...) SMC_TRACE_PRINTF_INFO( __VA_ARGS__ )
  #define ERROR_MSG(...) SMC_TRACE_PRINTF_ERROR( __VA_ARGS__ )
#else
  #if !defined(PRINT_MSG)
  #define PRINT_MSG(fmt, args...)
  #endif
  #if !defined(ERROR_MSG)
    #define ERROR_MSG(fmt, args...)
  #endif
#endif

typedef unsigned int uint32_t;     /* NOTE: Make sure that this type is 4 bytes long on your computer */
typedef unsigned char uint8_t;     /* NOTE: Make sure that this type is 1 byte on your computer */

typedef struct free_ptr_struct
{
    struct bhdr_struct *prev;
    struct bhdr_struct *next;
} free_ptr_t;

typedef struct bhdr_struct
{
    /* This pointer is just valid if the first bit of size is set */
    struct bhdr_struct *prev_hdr;
    /* The size is stored in bytes */
    size_t size;                /* bit 0 indicates whether the block is used and */
    /* bit 1 allows to know whether the previous block is free */
    union
    {
        struct free_ptr_struct free_ptr;
        uint8_t buffer[1];         /*sizeof(struct free_ptr_struct)]; */
    } ptr;
} bhdr_t;

/* This structure is embedded at the beginning of each area, giving us
 * enough information to cope with a set of areas */

typedef struct area_info_struct
{
    bhdr_t *end;
    struct area_info_struct *next;
} area_info_t;

typedef struct TLSF_struct
{
    /* the TLSF's structure signature */
    uint32_t tlsf_signature;

#if TLSF_USE_LOCKS
    TLSF_MLOCK_T lock;
#endif

#if TLSF_STATISTIC
    /* These can not be calculated outside tlsf because we
     * do not know the sizes when freeing/reallocing memory. */
    size_t used_size;
    size_t max_size;
#endif

    /* A linked list holding all the existing areas */
    area_info_t *area_head;

    /* the first-level bitmap */
    /* This array should have a size of REAL_FLI bits */
    uint32_t fl_bitmap;

    /* the second-level bitmap */
    uint32_t sl_bitmap[REAL_FLI];

    bhdr_t *matrix[REAL_FLI][MAX_SLI];
} tlsf_t;



static const int table[] = {
    -1, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4,
    4, 4,
    4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5,
    5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6,
    6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6,
    6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7,
    7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7,
    7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7,
    7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7,
    7, 7, 7, 7, 7, 7, 7
};

extern void free_ex(void *, void *);


static __inline__ int ls_bit(int i)
{
    unsigned int a;
    unsigned int x = i & -i;

    a = x <= 0xffff ? (x <= 0xff ? 0 : 8) : (x <= 0xffffff ? 16 : 24);
    return table[x >> a] + a;
}

static __inline__ int ms_bit(int i)
{
    unsigned int a;
    unsigned int x = (unsigned int) i;

    a = x <= 0xffff ? (x <= 0xff ? 0 : 8) : (x <= 0xffffff ? 16 : 24);
    return table[x >> a] + a;
}

#undef set_bit
static __inline__ void set_bit(int nr, uint32_t * addr)
{
    addr[nr >> 5] |= 1 << (nr & 0x1f);
}

#undef clear_bit
static __inline__ void clear_bit(int nr, uint32_t * addr)
{
    addr[nr >> 5] &= ~(1 << (nr & 0x1f));
}

static __inline__ void mapping_search(size_t * _r, int *_fl, int *_sl)
{
    int _t;

    if (*_r < SMALL_BLOCK)
    {
        *_fl = 0;
        *_sl = *_r / (SMALL_BLOCK / MAX_SLI);
    } 
    else 
    {
        _t = (1 << (ms_bit(*_r) - MAX_LOG2_SLI)) - 1;
        *_r = *_r + _t;
        *_fl = ms_bit(*_r);
        *_sl = (*_r >> (*_fl - MAX_LOG2_SLI)) - MAX_SLI;
        *_fl -= FLI_OFFSET;
        /*if ((*_fl -= FLI_OFFSET) < 0) // FL wil be always >0!
         *_fl = *_sl = 0;
         */
        *_r &= ~_t;
    }
}

static __inline__ void mapping_insert(size_t _r, int *_fl, int *_sl)
{
    if (_r < SMALL_BLOCK)
    {
        *_fl = 0;
        *_sl = _r / (SMALL_BLOCK / MAX_SLI);
    } 
    else
    {
        *_fl = ms_bit(_r);
        *_sl = (_r >> (*_fl - MAX_LOG2_SLI)) - MAX_SLI;
        *_fl -= FLI_OFFSET;
    }
}

static __inline__ bhdr_t* find_suitable_block(tlsf_t * _tlsf, int *_fl, int *_sl)
{
    uint32_t _tmp = _tlsf->sl_bitmap[*_fl] & (~0 << *_sl);
    bhdr_t *_b = NULL;

    if (_tmp)
    {
        *_sl = ls_bit(_tmp);
        _b = _tlsf->matrix[*_fl][*_sl];
    }
    else
    {
        *_fl = ls_bit(_tlsf->fl_bitmap & (~0 << (*_fl + 1)));
        if (*_fl > 0)
        {         /* likely */
            *_sl = ls_bit(_tlsf->sl_bitmap[*_fl]);
            _b = _tlsf->matrix[*_fl][*_sl];
        }
    }
    return _b;
}

#define EXTRACT_BLOCK_HDR(_b, _tlsf, _fl, _sl)                                  \
    do                                                                          \
    {                                                                           \
        _tlsf -> matrix[_fl][_sl] = _b->ptr.free_ptr.next;                      \
        if (_tlsf->matrix[_fl][_sl])                                            \
        {                                                                       \
            _tlsf->matrix[_fl][_sl]->ptr.free_ptr.prev = NULL;                  \
        }                                                                       \
        else                                                                    \
        {                                                                       \
            clear_bit(_sl, &_tlsf->sl_bitmap[_fl]);                             \
            if (!_tlsf->sl_bitmap[_fl])                                         \
            {                                                                   \
                clear_bit(_fl, &_tlsf->fl_bitmap);                              \
            }                                                                   \
        }                                                                       \
        _b -> ptr.free_ptr.prev = NULL;                                         \
        _b -> ptr.free_ptr.next = NULL;                                         \
    } while (0)


#define EXTRACT_BLOCK(_b, _tlsf, _fl, _sl)                                      \
    do                                                                          \
    {                                                                           \
        if (_b->ptr.free_ptr.next)                                              \
        {                                                                       \
            _b->ptr.free_ptr.next->ptr.free_ptr.prev = _b->ptr.free_ptr.prev;   \
        }                                                                       \
        if (_b->ptr.free_ptr.prev)                                              \
        {                                                                       \
            _b->ptr.free_ptr.prev->ptr.free_ptr.next = _b->ptr.free_ptr.next;   \
        }                                                                       \
        if (_tlsf->matrix[_fl][_sl] == _b)                                      \
        {                                                                       \
            _tlsf->matrix[_fl][_sl] = _b->ptr.free_ptr.next;                    \
            if (!_tlsf->matrix[_fl][_sl])                                       \
            {                                                                   \
                 clear_bit(_sl, &_tlsf->sl_bitmap[_fl]);                        \
                 if (!_tlsf->sl_bitmap[_fl])                                    \
                 {                                                              \
                     clear_bit(_fl, &_tlsf->fl_bitmap);                         \
                 }                                                              \
            }                                                                   \
        }                                                                       \
        _b -> ptr.free_ptr.prev = NULL;                                         \
        _b -> ptr.free_ptr.next = NULL;                                         \
    } while (0)

#define INSERT_BLOCK(_b, _tlsf, _fl, _sl)                                       \
    do                                                                          \
    {                                                                           \
        _b->ptr.free_ptr.prev = NULL;                                           \
        _b->ptr.free_ptr.next = _tlsf->matrix[_fl][_sl];                        \
        if (_tlsf->matrix[_fl][_sl])                                            \
        {                                                                       \
            _tlsf->matrix[_fl][_sl]->ptr.free_ptr.prev = _b;                    \
        }                                                                       \
        _tlsf -> matrix[_fl][_sl] = _b;                                         \
        set_bit(_sl, &_tlsf->sl_bitmap[_fl]);                                   \
        set_bit(_fl, &_tlsf->fl_bitmap);                                        \
    } while (0)

static __inline__ bhdr_t *process_area(void *area, size_t size)
{
    bhdr_t *b, *lb, *ib;
    area_info_t *ai;

    ib = (bhdr_t *) area;
    ib->size =
        (sizeof(area_info_t) <
         MIN_BLOCK_SIZE) ? MIN_BLOCK_SIZE : ROUNDUP_SIZE(sizeof(area_info_t)) | USED_BLOCK | PREV_USED;
    b = (bhdr_t *) GET_NEXT_BLOCK(ib->ptr.buffer, ib->size & BLOCK_SIZE);
    b->size = ROUNDDOWN_SIZE(size - 3 * BHDR_OVERHEAD - (ib->size & BLOCK_SIZE)) | USED_BLOCK | PREV_USED;
    b->ptr.free_ptr.prev = b->ptr.free_ptr.next = 0;
    lb = GET_NEXT_BLOCK(b->ptr.buffer, b->size & BLOCK_SIZE);
    lb->prev_hdr = b;
    lb->size = 0 | USED_BLOCK | PREV_FREE;
    ai = (area_info_t *) ib->ptr.buffer;
    ai->next = 0;
    ai->end = lb;
    return ib;
}

/******************************************************************/
/******************** Begin of the allocator code *****************/
/******************************************************************/

static char *mp = NULL;         /* Default memory pool. */

/******************************************************************/
size_t init_memory_pool(size_t mem_pool_size, void *mem_pool)
{
/******************************************************************/
    tlsf_t *tlsf;
    bhdr_t *b, *ib;

    if (!mem_pool || !mem_pool_size || mem_pool_size < sizeof(tlsf_t) + BHDR_OVERHEAD * 8)
    {
        ERROR_MSG("init_memory_pool (): memory_pool invalid\n");
        return (size_t)-1;
    }

    if (((unsigned long) mem_pool & PTR_MASK))
    {
        ERROR_MSG("init_memory_pool (): mem_pool must be aligned to a word\n");
        return (size_t)-1;
    }
    tlsf = (tlsf_t *) mem_pool;
    /* Check if already initialised */
    if (tlsf->tlsf_signature == TLSF_SIGNATURE)
    {
        mp = mem_pool;
        b = GET_NEXT_BLOCK(mp, ROUNDUP_SIZE(sizeof(tlsf_t)));
        return b->size & BLOCK_SIZE;
    }

    mp = mem_pool;

    /* Zeroing the memory pool */
    memset(mem_pool, 0, sizeof(tlsf_t));

    tlsf->tlsf_signature = TLSF_SIGNATURE;

    TLSF_CREATE_LOCK(&tlsf->lock);

    ib = process_area(GET_NEXT_BLOCK
                      (mem_pool, ROUNDUP_SIZE(sizeof(tlsf_t))), ROUNDDOWN_SIZE(mem_pool_size - sizeof(tlsf_t)));
    b = GET_NEXT_BLOCK(ib->ptr.buffer, ib->size & BLOCK_SIZE);
    free_ex(b->ptr.buffer, tlsf);
    tlsf->area_head = (area_info_t *) ib->ptr.buffer;

#if TLSF_STATISTIC
    tlsf->used_size = mem_pool_size - (b->size & BLOCK_SIZE);
    tlsf->max_size = tlsf->used_size;
#endif

    return (b->size & BLOCK_SIZE);
}


/******************************************************************/
size_t get_used_size(void *mem_pool)
{
/******************************************************************/
#if TLSF_STATISTIC
    return ((tlsf_t *) mem_pool)->used_size;
#else
    return 0;
#endif
}


/******************************************************************/
size_t get_max_size(void *mem_pool)
{
/******************************************************************/
#if TLSF_STATISTIC
    return ((tlsf_t *) mem_pool)->max_size;
#else
    return 0;
#endif
}


/******************************************************************/
void destroy_memory_pool(void *mem_pool)
{
/******************************************************************/
    tlsf_t *tlsf = (tlsf_t *) mem_pool;

    tlsf->tlsf_signature = 0;

    TLSF_DESTROY_LOCK(&tlsf->lock);

}


/******************************************************************/
void *malloc_ex(size_t size, void *mem_pool)
{
/******************************************************************/
    tlsf_t *tlsf = (tlsf_t *) mem_pool;
    bhdr_t *b;
    bhdr_t *b2;
    bhdr_t *next_b;
    int fl;
    int sl;
    size_t tmp_size;

    size = (size < MIN_BLOCK_SIZE) ? MIN_BLOCK_SIZE : ROUNDUP_SIZE(size);

    /* Rounding up the requested size and calculating fl and sl */
    mapping_search(&size, &fl, &sl);

    /* Searching a free block, recall that this function changes the values of fl and sl,
       so they are not longer valid when the function fails */
    b = find_suitable_block(tlsf, &fl, &sl);
#if USE_MMAP || USE_SBRK
    if (!b)
    {
        size_t area_size;
        void *area;
        /* Growing the pool size when needed */
        area_size = size + BHDR_OVERHEAD * 8;   /* size plus enough room for the requered headers. */
        area_size = (area_size > DEFAULT_AREA_SIZE) ? area_size : DEFAULT_AREA_SIZE;
        area = get_new_area(&area_size);        /* Call sbrk or mmap */
        if (area == ((void *) ~0))
        {
            return NULL;        /* Not enough system memory */
        }
        add_new_area(area, area_size, mem_pool);
        /* Rounding up the requested size and calculating fl and sl */
        MAPPING_SEARCH(&size, &fl, &sl);
        /* Searching a free block */
        b = find_suitable_block(tlsf, &fl, &sl);
    }
#endif
    if (!b)
    {
        return NULL;            /* Not found */
    }

    EXTRACT_BLOCK_HDR(b, tlsf, fl, sl);

    /*-- found: */
    next_b = GET_NEXT_BLOCK(b->ptr.buffer, b->size & BLOCK_SIZE);
    /* Should the block be split? */
    tmp_size = (b->size & BLOCK_SIZE) - size;
    if (tmp_size >= sizeof(bhdr_t))
    {
        tmp_size -= BHDR_OVERHEAD;
        b2 = GET_NEXT_BLOCK(b->ptr.buffer, size);
        b2->size = tmp_size | FREE_BLOCK | PREV_USED;
        next_b->prev_hdr = b2;
        mapping_insert(tmp_size, &fl, &sl);
        INSERT_BLOCK(b2, tlsf, fl, sl);

        b->size = size | (b->size & PREV_STATE);
    } 
    else
    {
        next_b->size &= (~PREV_FREE);
        b->size &= (~FREE_BLOCK);       /* Now it's used */
    }

    TLSF_ADD_SIZE(tlsf, b);

    return (void *) b->ptr.buffer;
}

/******************************************************************/
void free_ex(void *ptr, void *mem_pool)
{
/******************************************************************/
    tlsf_t* tlsf = (tlsf_t*) mem_pool;
    bhdr_t* b;
    bhdr_t* tmp_b;
    int fl = 0, sl = 0;

    if (!ptr)
    {
        return;
    }
    b = (bhdr_t*) ((char*) ptr - BHDR_OVERHEAD);
    b->size |= FREE_BLOCK;

    TLSF_REMOVE_SIZE(tlsf, b);

    b->ptr.free_ptr.prev = NULL;
    b->ptr.free_ptr.next = NULL;
    tmp_b = GET_NEXT_BLOCK(b->ptr.buffer, b->size & BLOCK_SIZE);
    if (tmp_b->size & FREE_BLOCK)
    {
        mapping_insert(tmp_b->size & BLOCK_SIZE, &fl, &sl);
        EXTRACT_BLOCK(tmp_b, tlsf, fl, sl);
        b->size += (tmp_b->size & BLOCK_SIZE) + BHDR_OVERHEAD;
    }
    if (b->size & PREV_FREE)
    {
        tmp_b = b->prev_hdr;
        mapping_insert(tmp_b->size & BLOCK_SIZE, &fl, &sl);
        EXTRACT_BLOCK(tmp_b, tlsf, fl, sl);
        tmp_b->size += (b->size & BLOCK_SIZE) + BHDR_OVERHEAD;
        b = tmp_b;
    }
    mapping_insert(b->size & BLOCK_SIZE, &fl, &sl);
    INSERT_BLOCK(b, tlsf, fl, sl);

    tmp_b = GET_NEXT_BLOCK(b->ptr.buffer, b->size & BLOCK_SIZE);
    tmp_b->size |= PREV_FREE;
    tmp_b->prev_hdr = b;
}



#if _DEBUG_TLSF_

/***************  DEBUG FUNCTIONS   **************/

/* The following functions have been designed to ease the debugging of */
/* the TLSF  structure.  For non-developing  purposes, it may  be they */
/* haven't too much worth.  To enable them, _DEBUG_TLSF_ must be set.  */

extern void dump_memory_region(unsigned char *mem_ptr, unsigned int size);
extern void print_block(bhdr_t * b);
extern void print_tlsf(tlsf_t * tlsf);
void print_all_blocks(tlsf_t * tlsf);

void dump_memory_region(unsigned char *mem_ptr, unsigned int size)
{

    unsigned long begin = (unsigned long) mem_ptr;
    unsigned long end = (unsigned long) mem_ptr + size;
    int column = 0;

    begin >>= 2;
    begin <<= 2;

    end >>= 2;
    end++;
    end <<= 2;

    PRINT_MSG("\nMemory region dumped: 0x%lx - 0x%lx\n\n", begin, end);

    column = 0;
    PRINT_MSG("0x%lx ", begin);

    while (begin < end)
    {
        if (((unsigned char *) begin)[0] == 0)
        {
            PRINT_MSG("00");
        }
        else
        {
            PRINT_MSG("%02x", ((unsigned char *) begin)[0]);
        }
        if (((unsigned char *) begin)[1] == 0)
        {
            PRINT_MSG("00 ");
        }
        else
        {
            PRINT_MSG("%02x ", ((unsigned char *) begin)[1]);
        }
        begin += 2;
        column++;
        if (column == 8)
        {
            PRINT_MSG("\n0x%lx ", begin);
            column = 0;
        }

    }
    PRINT_MSG("\n\n");
}

void print_block(bhdr_t * b)
{
    if (!b)
    {
        return;
    }
    PRINT_MSG(">> [%p] (", b);
    if ((b->size & BLOCK_SIZE))
    {
        PRINT_MSG("%lu bytes, ", (unsigned long) (b->size & BLOCK_SIZE));
    }
    else
    {
        PRINT_MSG("sentinel, ");
    }
    if ((b->size & BLOCK_STATE) == FREE_BLOCK)
    {
        PRINT_MSG("free [%p, %p], ", b->ptr.free_ptr.prev, b->ptr.free_ptr.next);
    }
    else
    {
        PRINT_MSG("used, ");
    }
    if ((b->size & PREV_STATE) == PREV_FREE)
    {
        PRINT_MSG("prev. free [%p])\n", b->prev_hdr);
    }
    else
    {
        PRINT_MSG("prev used)\n");
    }
}

void print_tlsf(tlsf_t * tlsf)
{
    bhdr_t *next;
    int i;
    int j;

    PRINT_MSG("\nTLSF at %p\n", tlsf);

    PRINT_MSG("FL bitmap: 0x%x\n\n", (unsigned) tlsf->fl_bitmap);

    for (i = 0; i < REAL_FLI; i++)
    {
        if (tlsf->sl_bitmap[i])
        {
            PRINT_MSG("SL bitmap 0x%x\n", (unsigned) tlsf->sl_bitmap[i]);
        }
        for (j = 0; j < MAX_SLI; j++)
        {
            next = tlsf->matrix[i][j];
            if (next)
            {
                PRINT_MSG("-> [%d][%d]\n", i, j);
            }
            while (next)
            {
                print_block(next);
                next = next->ptr.free_ptr.next;
            }
        }
    }
}

void print_all_blocks(tlsf_t * tlsf)
{
    area_info_t *ai;
    bhdr_t *next;
    PRINT_MSG("\nTLSF at %p\nALL BLOCKS\n\n", tlsf);
    ai = tlsf->area_head;
    while (ai)
    {
        next = (bhdr_t *) ((char *) ai - BHDR_OVERHEAD);
        while (next)
        {
            print_block(next);
            if ((next->size & BLOCK_SIZE))
            {
                next = GET_NEXT_BLOCK(next->ptr.buffer, next->size & BLOCK_SIZE);
            }
            else
            {
                next = NULL;
            }
        }
        ai = ai->next;
    }
}

#endif

/* This is the TLSF implementation used by default. */
#else // !TLSF_USE_REF

/* Create/destroy a memory pool. */
size_t tlsf_create(size_t bytes, void *mem);
void tlsf_destroy(void *pool);

/* malloc/memalign/realloc/free replacements. */
void* tlsf_malloc(void *pool, size_t bytes);
void* tlsf_memalign(tlsf_pool pool, size_t align, size_t bytes);
void* tlsf_realloc(tlsf_pool pool, void* ptr, size_t size);
void  tlsf_free(void *pool, void *ptr);

#define FORCE_FLS_GENERIC 0
#define TLSF_STATISTICS 0

#if defined (__GNUC__) && (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)) \
	&& defined (__GNUC_PATCHLEVEL__) && !FORCE_FLS_GENERIC
static __inline__ int tlsf_ffs(unsigned int word)
{
	return __builtin_ffs(word) - 1;
}

static __inline__ int tlsf_fls(unsigned int word)
{
	const int bit = word ? 32 - __builtin_clz(word) : 0;
	return bit - 1;
}

#elif defined (__ARMCC_VERSION) && !FORCE_FLS_GENERIC
static __inline__ int tlsf_ffs(unsigned int word)
{
	const unsigned int reverse = word & (~word + 1);
	const int bit = 32 - __clz(reverse);
	return bit - 1;
}

static __inline__ int tlsf_fls(unsigned int word)
{
	const int bit = word ? 32 - __clz(word) : 0;
	return bit - 1;
}

#else
/* Fall back to generic implementation. */
static __inline__ int tlsf_fls_generic(unsigned int word)
{
	int bit = 32;

	if (!word) bit -= 1;
	if (!(word & 0xffff0000)) { word <<= 16; bit -= 16; }
	if (!(word & 0xff000000)) { word <<= 8; bit -= 8; }
	if (!(word & 0xf0000000)) { word <<= 4; bit -= 4; }
	if (!(word & 0xc0000000)) { word <<= 2; bit -= 2; }
	if (!(word & 0x80000000)) { word <<= 1; bit -= 1; }

	return bit;
}/* Implement ffs in terms of fls. */

#define tlsf_ffs(word) \
	(tlsf_fls_generic(word & (~word + 1)) - 1)

#define tlsf_fls(word) \
	(tlsf_fls_generic(word) - 1)

#endif

/*
** Constants.
*/

/* Public constants: may be modified. */
enum tlsf_public
{
	/* log2 of number of linear subdivisions of block sizes. */
	SL_INDEX_COUNT_LOG2 = 5,
};

/* Private constants: do not modify. */
enum tlsf_private
{
	/* All allocation sizes and addresses are aligned to 32 bytes. */
	ALIGN_SIZE_LOG2 = 5,

	ALIGN_SIZE = (1 << ALIGN_SIZE_LOG2),

	/*
	** We support allocations of sizes up to (1 << FL_INDEX_MAX) bits.
	** However, because we linearly subdivide the second-level lists, and
	** our minimum size granularity is 32 bytes, it doesn't make sense to
	** create first-level lists for sizes smaller than SL_INDEX_COUNT * 32,
	** or (1 << (SL_INDEX_COUNT_LOG2 + 2)) bytes, as there we will be
	** trying to split size ranges into more slots than we have available.
	** Instead, we calculate the minimum threshold size, and place all
	** blocks below that size into the 0th first-level list.
	*/

	FL_INDEX_MAX = 30,
	SL_INDEX_COUNT = (1 << SL_INDEX_COUNT_LOG2),
	FL_INDEX_SHIFT = (SL_INDEX_COUNT_LOG2 + ALIGN_SIZE_LOG2),
	FL_INDEX_COUNT = (FL_INDEX_MAX - FL_INDEX_SHIFT + 1),

	SMALL_BLOCK_SIZE = (1 << FL_INDEX_SHIFT),
};

#define TLSF_SIGNATURE  (0x1234AAAA)

/*
** Cast and min/max macros.
*/

#define tlsf_cast(t, exp)	((t) (exp))
#define tlsf_min(a, b)		((a) < (b) ? (a) : (b))
#define tlsf_max(a, b)		((a) > (b) ? (a) : (b))

/*
** Set assert macro, if it has not been provided by the user.
*/
#if !defined (tlsf_assert)
#define tlsf_assert assert
#endif

/*
** Static assertion mechanism.
*/

#define _tlsf_glue2(x, y) x ## y
#define _tlsf_glue(x, y) _tlsf_glue2(x, y)
#define tlsf_static_assert(exp) \
	typedef char _tlsf_glue(static_assert, __LINE__) [(exp) ? 1 : -1]

/* This code has been tested on 32- and 64-bit (LP/LLP) architectures. */
tlsf_static_assert(sizeof(int) * CHAR_BIT == 32);
tlsf_static_assert(sizeof(size_t) * CHAR_BIT >= 32);
tlsf_static_assert(sizeof(size_t) * CHAR_BIT <= 64);

/* SL_INDEX_COUNT must be <= number of bits in sl_bitmap's storage type. */
tlsf_static_assert(sizeof(unsigned int) * CHAR_BIT >= SL_INDEX_COUNT);

/* Ensure we've properly tuned our sizes. */
tlsf_static_assert(ALIGN_SIZE == SMALL_BLOCK_SIZE / SL_INDEX_COUNT);

/*
** Data structures and associated constants.
*/

/*
** Block header structure.
**
** There are several implementation subtleties involved:
** - The prev_phys_block field is only valid if the previous block is free.
** - The next_free / prev_free fields are only valid if the block is free.
** - User data (external pointer) starts after padding
*/
typedef struct block_header_t
{
	/* Points to the previous physical block. */
	struct block_header_t* prev_phys_block;

    uint8_t is_free; 
    uint8_t is_prev_free; 

    /* Internal block size including header. */
	size_t size;

    /* Padding to alignment */
    uint8_t padding[22];
	
	/* Next and previous free blocks. User data starts here if the block is in use.*/
	struct block_header_t* next_free;
	struct block_header_t* prev_free;
    
} block_header_t;

static const size_t block_header_overhead = ALIGN_SIZE;

static const size_t block_size_min = 
	(sizeof(block_header_t) + (ALIGN_SIZE - 1)) & ~(ALIGN_SIZE - 1);

static const size_t block_size_max = tlsf_cast(size_t, 1) << FL_INDEX_MAX;


/* The TLSF pool structure. */
typedef struct pool_t
{
	/* Empty lists point at this block to indicate they are free. */
	block_header_t block_null;
    uint32_t tlsf_free_marker;
    
	/* Bitmaps for free lists. */
	unsigned int fl_bitmap;
	unsigned int sl_bitmap[FL_INDEX_COUNT];

	/* Head of free lists. */
	block_header_t* blocks[FL_INDEX_COUNT][SL_INDEX_COUNT];

    /* 28 bytes to next 32 byte border. Use to store debug data. */
    uint32_t free_blocks;
    uint32_t used_blocks;
    uint32_t free_space;
    uint32_t free_space_min;
} pool_t;

/* A type used for casting when doing pointer arithmetic. */
typedef uint32_t tlsfptr_t;

/*
** block_header_t member macros.
*/

#define block_size(block) \
	(block->size)

#define block_set_size(block, new_size) \
	(block->size = new_size)

#define block_is_last(block) \
	(0 == block->size ? 1 : 0)

#define block_is_free(block) \
	(block->is_free)

#define block_set_free(block) \
	(block->is_free = 1)

#define block_set_used(block) \
	(block->is_free = 0)

#define block_is_prev_free(block) \
	(block->is_prev_free)

#define block_set_prev_free(block) \
	(block->is_prev_free = 1)

#define block_set_prev_used(block) \
	(block->is_prev_free = 0)

#define block_from_ptr(ptr) \
	(tlsf_cast(block_header_t*, \
		tlsf_cast(unsigned char*, ptr) - block_header_overhead))

#define block_to_ptr(block) \
	(tlsf_cast(void*, \
		tlsf_cast(unsigned char*, block) + block_header_overhead))

/* Return location of next block after block of given size. */
#define offset_to_block(block, size) \
	(tlsf_cast(block_header_t*, tlsf_cast(size_t, block) + size))

/* Return location of previous block. */
#define block_prev(block) \
	(block->prev_phys_block)

/* Return location of next existing block. */
#define block_next(block) \
	(offset_to_block(block, block_size(block)))

/* Link a new block with its physical neighbor, return the neighbor. */
static __inline__ block_header_t* block_link_next(block_header_t* block)
{
	block_header_t* next = block_next(block);
	next->prev_phys_block = block;
	return next;
}

static __inline__ void block_mark_as_free(block_header_t* block)
{
	/* Link the block to the next block, first. */
	block_header_t* next = block_link_next(block);
	block_set_prev_free(next);
	block_set_free(block);
}

static __inline__ void block_mark_as_used(block_header_t* block)
{
	block_header_t* next = block_next(block);
	block_set_prev_used(next);
	block_set_used(block);
}

#define align_up(x, align) \
	((x + (align - 1)) & ~(align - 1))

#define align_down(x, align) \
	(x - (x & (align - 1)))

#define align_ptr(ptr, align) \
    (tlsf_cast(void*, (tlsf_cast(tlsfptr_t, ptr) + (align - 1)) & ~(align - 1)))

/*
** Adjust an allocation size to be aligned to word size, and no smaller
** than internal minimum.
*/
#define adjust_request_size(size, align) \
    (tlsf_max(align_up(size + block_header_overhead, align), block_size_min))

/*
** TLSF utility functions. In most cases, these are direct translations of
** the documentation found in the white paper.
*/

static __inline__ void mapping_insert(size_t size, int* fli, int* sli)
{
	if (size < SMALL_BLOCK_SIZE)
	{
		/* Store small blocks in first list. */
		*fli = 0;
		*sli = size / (SMALL_BLOCK_SIZE / SL_INDEX_COUNT);
	}
	else
	{
		*fli = tlsf_fls(size);
		*sli = size >> (*fli - SL_INDEX_COUNT_LOG2) ^ (1 << SL_INDEX_COUNT_LOG2);
		*fli -= (FL_INDEX_SHIFT - 1);
	}
}

/* This version rounds up to the next block size (for allocations) */
static __inline__ void mapping_search(size_t size, int* fli, int* sli)
{
	if (size >= (1 << SL_INDEX_COUNT_LOG2))
	{
		size += (1 << (tlsf_fls(size) - SL_INDEX_COUNT_LOG2)) - 1;
	}
	mapping_insert(size, fli, sli);
}

static __inline__ block_header_t* search_suitable_block(pool_t* pool, int* fli, int* sli)
{
	/*
	** First, search for a block in the list associated with the given
	** fl/sl index.
	*/
	unsigned int sl_map = pool->sl_bitmap[*fli] & (~0 << *sli);
	if (!sl_map)
	{
		/* No block exists. Search in the next largest first-level list. */
		const unsigned int fl_map = pool->fl_bitmap & (~0 << (*fli + 1));
		if (!fl_map)
		{
			/* No free blocks available, memory has been exhausted. */
			return NULL;
		}

		*fli = tlsf_ffs(fl_map);
		sl_map = pool->sl_bitmap[*fli];
	}
	tlsf_assert(sl_map); //internal error - second level bitmap is null
	*sli = tlsf_ffs(sl_map);

	/* Return the first block in the free list. */
	return pool->blocks[*fli][*sli];
}

/* Remove a free block from the free list.*/
static __inline__ void remove_free_block(pool_t* pool, block_header_t* block, int fl, int sl)
{
	block_header_t* prev = block->prev_free;
	block_header_t* next = block->next_free;
	tlsf_assert(prev); //prev_free field can not be null
	tlsf_assert(next); //next_free field can not be null
	next->prev_free = prev;
	prev->next_free = next;

	/* If this block is the head of the free list, set new head. */
	if (pool->blocks[fl][sl] == block)
	{
		pool->blocks[fl][sl] = next;

		/* If the new head is null, clear the bitmap. */
		if (next == &pool->block_null)
		{
			pool->sl_bitmap[fl] &= ~(1 << sl);

			/* If the second bitmap is now empty, clear the fl bitmap. */
			if (!pool->sl_bitmap[fl])
			{
				pool->fl_bitmap &= ~(1 << fl);
			}
		}
	}
#if TLSF_STATISTICS
    pool->free_blocks--;
#endif
}

/* Insert a free block into the free block list. */
static __inline__ void insert_free_block(pool_t* pool, block_header_t* block, int fl, int sl)
{
	block_header_t* current_block = pool->blocks[fl][sl];
	tlsf_assert(current_block); //free list cannot have a null entry
	tlsf_assert(block); //cannot insert a null entry into the free list
	block->next_free = current_block;
	block->prev_free = &pool->block_null;
	current_block->prev_free = block;

	tlsf_assert(block_to_ptr(block) == align_ptr(block_to_ptr(block), ALIGN_SIZE)); //block not aligned properly
	/*
	** Insert the new block at the head of the list, and mark the first-
	** and second-level bitmaps appropriately.
	*/
	pool->blocks[fl][sl] = block;
	pool->fl_bitmap |= (1 << fl);
	pool->sl_bitmap[fl] |= (1 << sl);
#if TLSF_STATISTICS
    pool->free_blocks++;
#endif
}

/* Remove a given block from the free list. */
static __inline__ void block_remove(pool_t* pool, block_header_t* block)
{
	int fl, sl;
	mapping_insert(block_size(block), &fl, &sl);
	remove_free_block(pool, block, fl, sl);
}

/* Insert a given block into the free list. */
static __inline__ void block_insert(pool_t* pool, block_header_t* block)
{
	int fl, sl;
	mapping_insert(block_size(block), &fl, &sl);
	insert_free_block(pool, block, fl, sl);
}

#define block_can_split(block, size) \
	(block_size(block) >= block_size_min + size)

/* Split a block into two, the second of which is free. */
static __inline__ block_header_t* block_split(block_header_t* block, size_t size)
{
	/* Calculate the amount of space left in the remaining block. */
	block_header_t* remaining = 
		offset_to_block(block, size);

	const size_t remain_size = block_size(block) - size;

	tlsf_assert(block_to_ptr(remaining) == align_ptr(block_to_ptr(remaining), ALIGN_SIZE)); //remaining block not aligned properly

	tlsf_assert(block_size(block) == remain_size + size);
	block_set_size(remaining, remain_size);
	tlsf_assert(block_size(remaining) >= block_size_min); //block split with invalid size

	block_set_size(block, size);
	block_mark_as_free(remaining);

	return remaining;
}

/* Absorb a free block's storage into an adjacent previous free block. */
static __inline__ block_header_t* block_absorb(block_header_t* prev, block_header_t* block)
{
	tlsf_assert(!block_is_last(prev)); //previous block can't be last
	prev->size += block_size(block);
	block_link_next(prev);
	return prev;
}

/* Merge a just-freed block with an adjacent previous free block. */
static __inline__ block_header_t* block_merge_prev(pool_t* pool, block_header_t* block)
{
	if (block_is_prev_free(block))
	{
		block_header_t* prev = block_prev(block);
		tlsf_assert(prev); //prev physical block can't be null
		tlsf_assert(block_is_free(prev)); //prev block is not free though marked as such
		block_remove(pool, prev);
		block = block_absorb(prev, block);
	}

	return block;
}

/* Merge a just-freed block with an adjacent free block. */
static __inline__ block_header_t* block_merge_next(pool_t* pool, block_header_t* block)
{
	block_header_t* next = block_next(block);
	tlsf_assert(next); //next physical block can't be null

	if (block_is_free(next))
	{
		tlsf_assert(!block_is_last(block)); //previous block can't be last
		block_remove(pool, next);
		block = block_absorb(block, next);
	}

	return block;
}

/* Trim any trailing block space off the end of a block, return to pool. */
static __inline__ void block_trim_free(pool_t* pool, block_header_t* block, size_t size)
{
	tlsf_assert(block_is_free(block)); //block must be free
	if (block_can_split(block, size))
	{
		block_header_t* remaining_block = block_split(block, size);
		block_link_next(block);
		block_set_prev_free(remaining_block);
		block_insert(pool, remaining_block);
	}
}

/* Trim any trailing block space off the end of a used block, return to pool. */
static __inline__ void block_trim_used(pool_t* pool, block_header_t* block, size_t size)
{
	tlsf_assert(!block_is_free(block)); //block must be used
	if (block_can_split(block, size))
	{
		/* If the next block is free, we must coalesce. */
		block_header_t* remaining_block = block_split(block, size);
		block_set_prev_used(remaining_block);

		remaining_block = block_merge_next(pool, remaining_block);
		block_insert(pool, remaining_block);
	}
}

static __inline__ block_header_t* block_trim_free_leading(pool_t* pool, block_header_t* block, size_t size)
{
	block_header_t* remaining_block = block;
	if (block_can_split(block, size))
	{
		/* We want the 2nd block. */
		remaining_block = block_split(block, size);
		block_set_prev_free(remaining_block);

		block_link_next(block);
		block_insert(pool, block);
	}

	return remaining_block;
}

static __inline__ block_header_t* block_locate_free(pool_t* pool, size_t size)
{
	int fl = 0, sl = 0;
	block_header_t* block = 0;

	if (size)
	{
		mapping_search(size, &fl, &sl);
		block = search_suitable_block(pool, &fl, &sl);
	}

	if (block)
	{
		tlsf_assert(block_size(block) >= size);
		remove_free_block(pool, block, fl, sl);
	}

	return block;
}

static __inline__ void* block_prepare_used(pool_t* pool, block_header_t* block, size_t size)
{
	void* p = 0;
	if (block)
	{
		block_trim_free(pool, block, size);
		block_mark_as_used(block);
#if TLSF_STATISTICS
        pool->free_space -= block_size(block);
        if (pool->free_space < pool->free_space_min)
            pool->free_space_min = pool->free_space;
        SMC_TRACE_PRINTF_INFO("ALLOCATED %d bytes, free_space %d, free_space_min %d", block_size(block), pool->free_space, pool->free_space_min);
#endif
		p = block_to_ptr(block);
	}
	return p;
}

/* Clear structure and point all empty lists at the null block. */
static __inline__ void pool_construct(pool_t* pool)
{
	int i, j;

	pool->block_null.next_free = &pool->block_null;
	pool->block_null.prev_free = &pool->block_null;

    pool->tlsf_free_marker = TLSF_SIGNATURE;

	pool->fl_bitmap = 0;
	for (i = 0; i < FL_INDEX_COUNT; ++i)
	{
		pool->sl_bitmap[i] = 0;
		for (j = 0; j < SL_INDEX_COUNT; ++j)
		{
			pool->blocks[i][j] = &pool->block_null;
		}
	}
    pool->free_blocks = 0;
    pool->used_blocks = 0;
}

/*
** Debugging utilities.
*/

typedef struct integrity_t
{
	int prev_status;
	int status;
} integrity_t;

#define tlsf_insist(x) { tlsf_assert(x); if (!(x)) { status--; } }

static void integrity_walker(void* ptr, size_t size, int used, void* user)
{
	block_header_t* block = block_from_ptr(ptr);
	integrity_t* integ = tlsf_cast(integrity_t*, user);
	const int this_prev_status = block_is_prev_free(block) ? 1 : 0;
	const int this_status = block_is_free(block) ? 1 : 0;
	const size_t this_block_size = block_size(block);

	int status = 0;
	tlsf_insist(integ->prev_status == this_prev_status); //prev status incorrect
	tlsf_insist(size == this_block_size); //block size incorrect

	integ->prev_status = this_status;
	integ->status += status;
}

int tlsf_check_heap(tlsf_pool tlsf)
{
	int i, j;
	pool_t* pool = tlsf_cast(pool_t*, tlsf);
	int status = 0;

	/* Check that the blocks are physically correct. */
	integrity_t integ = { 0, 0 };
	tlsf_walk_heap(tlsf, integrity_walker, &integ);
	status = integ.status;

	/* Check that the free lists and bitmaps are accurate. */
	for (i = 0; i < FL_INDEX_COUNT; ++i)
	{
		for (j = 0; j < SL_INDEX_COUNT; ++j)
		{
			const int fl_map = pool->fl_bitmap & (1 << i);
			const int sl_list = pool->sl_bitmap[i];
			const int sl_map = sl_list & (1 << j);
			const block_header_t* block = pool->blocks[i][j];

			/* Check that first- and second-level lists agree. */
			if (!fl_map)
			{
				tlsf_insist(!sl_map); //second-level map must be null
			}

			if (!sl_map)
			{
				tlsf_insist(block == &pool->block_null); //block list must be null
				continue;
			}

			/* Check that there is at least one free block. */
			tlsf_insist(sl_list); //no free blocks in second-level map
			tlsf_insist(block != &pool->block_null); //block should not be null

			while (block != &pool->block_null)
			{
				int fli, sli;
				tlsf_insist(block_is_free(block)); //block should be free
				tlsf_insist(!block_is_prev_free(block)); //blocks should have coalesced
				tlsf_insist(!block_is_free(block_next(block))); //blocks should have coalesced
				tlsf_insist(block_is_prev_free(block_next(block))); //block should be free
				tlsf_insist(block_size(block) >= block_size_min); //block not minimum size

				mapping_insert(block_size(block), &fli, &sli);
				tlsf_insist(fli == i && sli == j); //block size indexed in wrong list
				block = block->next_free;
			}
		}
	}

	return status;
}

#undef tlsf_insist

static void default_walker(void* ptr, size_t size, int used, void* user)
{
	(void)user;
	SMC_TRACE_PRINTF_INFO("\t%p %s size: %x (%p)\n", ptr, used ? "used" : "free", (unsigned int)size, block_from_ptr(ptr));
}

void tlsf_walk_heap(tlsf_pool pool, tlsf_walker walker, void* user)
{
	tlsf_walker heap_walker = walker ? walker : default_walker;
	block_header_t* block =
		offset_to_block(pool, sizeof(pool_t));

	while (block && !block_is_last(block))
	{
		heap_walker(
			block_to_ptr(block),
			block_size(block),
			!block_is_free(block),
			user);
		block = block_next(block);
	}
}

size_t tlsf_block_size(void* ptr)
{
	size_t size = 0;
	if (ptr)
	{
		const block_header_t* block = block_from_ptr(ptr);
		size = block_size(block);
	}
	return size;
}

size_t tlsf_ptr_size(void * ptr)
{
    size_t size = 0;
    size = tlsf_block_size(ptr);
    if (size)
    {
        size -= block_header_overhead;
    }
    return size;
}

/*
** Overhead of the TLSF structures in a given memory block passed to
** tlsf_create, equal to the size of a pool_t aligned up to next 32 byte boundary.
*/
size_t tlsf_overhead()
{
	const size_t pool_overhead = align_up(sizeof(pool_t), ALIGN_SIZE);
	return pool_overhead;
}

/*
** TLSF main interface. Right out of the white paper.
*/

size_t tlsf_create(size_t bytes, void *mem)
{
	block_header_t* block;
	block_header_t* next;

	const size_t pool_overhead = tlsf_overhead();
    /* Subtract the size of pool header overhead and size of sentinel to get
    ** the actual usable pool size */
	const size_t pool_bytes = align_down(bytes - pool_overhead - block_size_min, ALIGN_SIZE);
    SMC_TRACE_PRINTF_INFO("bytes %d, pool_bytes %d, pool_overhead %d, block_size_min %d", bytes, pool_bytes, pool_overhead, block_size_min);
	pool_t* pool = tlsf_cast(pool_t*, mem);

#if _DEBUG_TLSF_
	/* Verify ffs/fls work properly. */
	int rv = 0;
	rv += (tlsf_ffs(0) == -1) ? 0 : 0x1;
	rv += (tlsf_fls(0) == -1) ? 0 : 0x2;
	rv += (tlsf_ffs(1) == 0) ? 0 : 0x4;
	rv += (tlsf_fls(1) == 0) ? 0 : 0x8;
	rv += (tlsf_ffs(0x80000000) == 31) ? 0 : 0x10;
	rv += (tlsf_ffs(0x80008000) == 15) ? 0 : 0x20;
	rv += (tlsf_fls(0x80000008) == 31) ? 0 : 0x40;
	rv += (tlsf_fls(0x7FFFFFFF) == 30) ? 0 : 0x80;
#endif

	if (pool_bytes < block_size_min || pool_bytes > block_size_max)
	{
		SMC_TRACE_PRINTF_INFO("tlsf_create: Pool size must be between %u and %u bytes.\n", 
			(unsigned int)(block_size_min),
			(unsigned int)(block_size_max));
		return 0;
	}

	/* Construct a valid pool object. */
	pool_construct(pool);

	/*
	** Create the main free block. */
    block = offset_to_block(tlsf_cast(void*, pool), pool_overhead);
	block_set_size(block, pool_bytes);
	block_set_free(block);
	block_set_prev_used(block);
	block_insert(pool, block);

#if TLSF_STATISTICS
    pool->free_space = block_size(block) - block_header_overhead;
    pool->free_space_min = pool->free_space;
#else
    pool->free_space = 0;
#endif

	/* Split the block to create a zero-size pool sentinel block. */
	next = block_link_next(block);
	block_set_size(next, 0);
	block_set_used(next);
	block_set_prev_free(next);

	return pool_bytes;
}

void tlsf_destroy(void *pool)
{
    pool_t *tlsf = (pool_t*)pool;
	tlsf->tlsf_free_marker = 0;
}

void* tlsf_malloc(void *tlsf, size_t size)
{
	pool_t* pool = tlsf_cast(pool_t*, tlsf);
	const size_t adjust = adjust_request_size(size, ALIGN_SIZE);
	block_header_t* block = block_locate_free(pool, adjust);
	return block_prepare_used(pool, block, adjust);
}

void* tlsf_memalign(tlsf_pool tlsf, size_t align, size_t size)
{
	pool_t* pool = tlsf_cast(pool_t*, tlsf);
	const size_t adjust = adjust_request_size(size, ALIGN_SIZE);

	/*
	** We must allocate an additional minimum block size bytes so that if
	** our free block will leave an alignment gap which is smaller, we can
	** trim a leading free block and release it back to the heap. We must
	** do this because the previous physical block is in use, therefore
	** the prev_phys_block field is not valid, and we can't simply adjust
	** the size of that block.
	*/
	const size_t gap_minimum = block_size_min;
	const size_t size_with_gap = adjust_request_size(adjust + align + gap_minimum, align);

	/* If alignment is less than or equals base alignment, we're done. */
	const size_t aligned_size = (align <= ALIGN_SIZE) ? adjust : size_with_gap;

	block_header_t* block = block_locate_free(pool, aligned_size);

	if (block)
	{
		void* ptr = block_to_ptr(block);
		void* aligned = align_ptr(ptr, align);
		size_t gap = tlsf_cast(size_t,
			tlsf_cast(tlsfptr_t, aligned) - tlsf_cast(tlsfptr_t, ptr));

		/* If gap size is too small, offset to next aligned boundary. */
		if (gap && gap < gap_minimum)
		{
			const size_t gap_remain = gap_minimum - gap;
			const size_t offset = tlsf_max(gap_remain, align);
			const void* next_aligned = tlsf_cast(void*,
				tlsf_cast(tlsfptr_t, aligned) + offset);

			aligned = align_ptr(next_aligned, align);
			gap = tlsf_cast(size_t,
				tlsf_cast(tlsfptr_t, aligned) - tlsf_cast(tlsfptr_t, ptr));
		}

		if (gap)
		{
			tlsf_assert(gap >= gap_minimum); //gap size too small
			block = block_trim_free_leading(pool, block, gap);
		}
	}

	return block_prepare_used(pool, block, adjust);
}

void tlsf_free(void *tlsf, void *ptr)
{
	/* Don't attempt to free a NULL pointer. */
	if (ptr)
	{
		pool_t* pool = tlsf_cast(pool_t*, tlsf);
		block_header_t* block = block_from_ptr(ptr);
#if TLSF_STATISTICS
        pool->free_space += block_size(block);
#endif
		block_mark_as_free(block);
		block = block_merge_prev(pool, block);
		block = block_merge_next(pool, block);
		block_insert(pool, block);
	}
}

/*
** The TLSF block information provides us with enough information to
** provide a reasonably intelligent implementation of realloc, growing or
** shrinking the currently allocated block as required.
**
** This routine handles the somewhat esoteric edge cases of realloc:
** - a non-zero size with a null pointer will behave like malloc
** - a zero size with a non-null pointer will behave like free
** - a request that cannot be satisfied will leave the original buffer
**   untouched
** - an extended buffer size will leave the newly-allocated area with
**   contents undefined
*/
void* tlsf_realloc(tlsf_pool tlsf, void* ptr, size_t size)
{
	pool_t* pool = tlsf_cast(pool_t*, tlsf);
	void* p = 0;

	/* Zero-size requests are treated as free. */
	if (ptr && size == 0)
	{
		tlsf_free(tlsf, ptr);
	}
	/* Requests with NULL pointers are treated as malloc. */
	else if (!ptr)
	{
		p = tlsf_malloc(pool, size);
	}
	else
	{
		block_header_t* block = block_from_ptr(ptr);
		block_header_t* next = block_next(block);

		const size_t cursize = block_size(block);
		const size_t combined = cursize + block_size(next);
		const size_t adjust = adjust_request_size(size, ALIGN_SIZE);

		/*
		** If the next block is used, or when combined with the current
		** block, does not offer enough space, we must reallocate and copy.
		*/
		if (adjust > cursize && (!block_is_free(next) || adjust > combined))
		{
			p = tlsf_malloc(pool, size);
			if (p)
			{
				const size_t minsize = tlsf_min(cursize, size);
				memcpy(p, ptr, minsize - block_header_overhead);
				tlsf_free(tlsf, ptr);
			}
		}
		else
		{
			/* Do we need to expand to the next block? */
			if (adjust > cursize)
			{
				block_merge_next(pool, block);
				block_mark_as_used(block);
			}

			/* Trim the resulting block and return the original pointer. */
			block_trim_used(pool, block, adjust);
			p = ptr;
		}
	}

	return p;
}

#endif // TLSF_USE_REF

/******************
** MDB functions **
******************/

static void mdb_info_store( smc_mdb_channel_info_t* channel_info_ptr )
{
    const smc_mdb_channel_info_t channel_info_initial = { -1, NULL, NULL, 0, 0 };
    
    SMC_TRACE_PRINTF_INFO("mdb_info_store(): channel_id: %d, pool_in: 0x%08x, pool_out: 0x%08x, size_in: %d, size_out: %d",
        channel_info_ptr->id, channel_info_ptr->pool_in, channel_info_ptr->pool_out,
        channel_info_ptr->total_size_in, channel_info_ptr->total_size_out);

    if (mdb_channel_info_set == NULL)
    {
        mdb_channel_info_set = SMC_MALLOC(sizeof(smc_mdb_channel_info_set_t) + (channel_info_ptr->id * sizeof(smc_mdb_channel_info_t)));
        for (uint8_t i = 0; i < channel_info_ptr->id + 1; i++)
        {
            mdb_channel_info_set->channel_info[i] = channel_info_initial;
            mdb_channel_info_set->channel_info[i].id = i;
        }
        mdb_channel_info_set->channel_amount = channel_info_ptr->id + 1;
    }
   
    if (channel_info_ptr->id >= mdb_channel_info_set->channel_amount)
    {
        smc_mdb_channel_info_set_t* old_channel_info_set = mdb_channel_info_set;
        mdb_channel_info_set = SMC_MALLOC(sizeof(smc_mdb_channel_info_set_t) + (channel_info_ptr->id * sizeof(smc_mdb_channel_info_t)));
        memcpy(mdb_channel_info_set, old_channel_info_set, 
            sizeof(smc_mdb_channel_info_set_t) + ((channel_info_ptr->id - 1) * sizeof(smc_mdb_channel_info_t)));
        SMC_FREE(old_channel_info_set);
        for (uint8_t i = mdb_channel_info_set->channel_amount; i < channel_info_ptr->id + 1; i++)
        {
            mdb_channel_info_set->channel_info[i] = channel_info_initial;
            mdb_channel_info_set->channel_info[i].id = i;
        }
        mdb_channel_info_set->channel_amount = channel_info_ptr->id + 1;
    }
    mdb_channel_info_set->channel_info[channel_info_ptr->id] = *channel_info_ptr;
}


static smc_mdb_channel_info_t* mdb_info_fetch( uint8_t channel_id )
{
    smc_mdb_channel_info_t* channel_info_ptr = NULL;
    const smc_mdb_channel_info_t channel_info_initial = { -1, NULL, NULL, 0, 0 };

    SMC_TRACE_PRINTF_INFO("mdb_info_fetch(): channel_id: %d", channel_id);

    if ((mdb_channel_info_set != NULL) && (channel_id < mdb_channel_info_set->channel_amount))
    {
        channel_info_ptr = &(mdb_channel_info_set->channel_info[channel_id]);
        
        SMC_TRACE_PRINTF_INFO("mdb_info_fetch(): channel_id: %d, pool_in: 0x%08x, pool_out: 0x%08x, size_in: %d, size_out: %d",
            channel_info_ptr->id, channel_info_ptr->pool_in, channel_info_ptr->pool_out,
            channel_info_ptr->total_size_in, channel_info_ptr->total_size_out);
    }
    else
    {
        SMC_TRACE_PRINTF_INFO("mdb_info_fetch(): channel_id: %d, pool_in: 0x%08x, pool_out: 0x%08x, size_in: %d, size_out: %d",
            channel_info_initial.id, channel_info_initial.pool_in, channel_info_initial.pool_out,
            channel_info_initial.total_size_in, channel_info_initial.total_size_out);
    }
    
    return channel_info_ptr;
}


static void mdb_channel_info_delete( void )
{
    if (mdb_channel_info_set != NULL)
    {
        SMC_FREE(mdb_channel_info_set);
        mdb_channel_info_set = NULL;
    }
}


/*
 * Calculates the size of required share memory MDB with given MDB data area size.
 */
uint32_t smc_mdb_calculate_required_shared_mem( uint32_t mdb_data_area_size )
{
    uint32_t required_mem = 0;

        /* No additional shared memory needed except the data area */
    required_mem = mdb_data_area_size;

    SMC_TRACE_PRINTF_INFO("smc_mdb_calculate_required_shared_mem: MDB requires %d bytes of shared memory", required_mem);

    return required_mem;
}


int32_t smc_mdb_create( uint8_t channel_id, void* pool_address, uint32_t pool_size )
{
    smc_mdb_channel_info_t channel_info = { -1, NULL, NULL, 0, 0 };
    int32_t result = 0;
    int32_t init_size = 0;
    int32_t i = 0;

    assert(sizeof(uint32_t) == 4); /* NOTE: Make sure that this type is 4 bytes long on your computer */
    assert(sizeof(uint8_t)  == 1); /* NOTE: Make sure that this type is 1 byte on your computer */

    assert(pool_address != NULL);
    assert(pool_size != NULL);
    
    smc_mdb_channel_info_t* channel_info_ptr = mdb_info_fetch(channel_id);
    
    if (channel_info_ptr != NULL)
    {
        channel_info = *channel_info_ptr;
    }
    
    channel_info.id = channel_id;
    channel_info.pool_out = pool_address;
    channel_info.total_size_out = pool_size;
    
    SMC_TRACE_PRINTF_INFO("mdb_info_create(): channel_id: %d, pool_address: 0x%08x, pool_size: %d",
        channel_info.id, channel_info.pool_out, channel_info.total_size_out);

    mdb_info_store(&channel_info);
 
#if TLSF_USE_REF
    init_size = init_memory_pool(channel_info.total_size_out, channel_info.pool_out);
#else
    init_size = tlsf_create(channel_info.total_size_out, channel_info.pool_out);
#endif
    if (init_size <= 0)
    {
        SMC_TRACE_PRINTF_ERROR("smc_mdb_create(): MDB OUT not created");
        assert(init_size <= 0);
    }
    else
    {
        SMC_TRACE_PRINTF_INFO("smc_mdb_create(): MDB OUT successfully created");
    
        for (i = channel_info.total_size_out; i > 0; i--)
        {
#if TLSF_USE_REF
            void* ptr = malloc_ex(i, channel_info.pool_out);
#else
            void* ptr = tlsf_malloc(channel_info.pool_out, i);
#endif
            if (ptr != NULL)
            {
#if TLSF_USE_REF
                free_ex(ptr, channel_info.pool_out);
#else
                tlsf_free(channel_info.pool_out, ptr);
#endif
                break;
            }
        }
    }   
#if TLSF_STATISTICS
    ((pool_t*)channel_info.pool_out)->free_space_min = ((pool_t*)channel_info.pool_out)->free_space;
#endif

    return result;
}

int32_t smc_mdb_init( uint8_t channel_id, void* pool_address, uint32_t pool_size )
{
    smc_mdb_channel_info_t channel_info = { -1, NULL, NULL, 0, 0 };
    int32_t result = 0;
    
    assert(pool_address != NULL);
    assert(pool_size != NULL);
    
    smc_mdb_channel_info_t* channel_info_ptr = mdb_info_fetch(channel_id);
    
    if (channel_info_ptr != NULL)
    {
        channel_info = *channel_info_ptr;
    }
    
    channel_info.id = channel_id;
    channel_info.pool_in = pool_address;
    channel_info.total_size_in = pool_size;
    
    SMC_TRACE_PRINTF_INFO("smc_mdb_init(): channel_id: %d, pool_address: 0x%08x, pool_size: %d",
        channel_info.id, channel_info.pool_in, channel_info.total_size_in);

    mdb_info_store(&channel_info);
#if TLSF_USE_REF 
    if (((tlsf_t *)pool_address)->tlsf_signature == TLSF_SIGNATURE)
#else
    if (((pool_t*)pool_address)->tlsf_free_marker == TLSF_SIGNATURE)
#endif
    {
        SMC_TRACE_PRINTF_INFO("smc_mdb_init(): MDB IN already initialised");
    }
    else
    {
        SMC_TRACE_PRINTF_INFO("smc_mdb_init(): MDB IN not yet initialised");
    }
    
    return result;
}


void smc_mdb_destroy( uint8_t channel_id )
{
    smc_mdb_channel_info_t* channel_info_ptr = mdb_info_fetch(channel_id);
    
    assert(channel_info_ptr != NULL);

    SMC_TRACE_PRINTF_INFO("smc_mdb_destroy(): channel_id: %d", channel_info_ptr->id);

    assert(channel_info_ptr->pool_out != NULL);
    assert(channel_info_ptr->total_size_out > 0);

#if TLSF_USE_REF
    destroy_memory_pool(channel_info_ptr->pool_out);
#else
    tlsf_destroy(channel_info_ptr->pool_out);
#endif


    channel_info_ptr->id = channel_id;    
    channel_info_ptr->pool_in = NULL;
    channel_info_ptr->pool_out = NULL;
    channel_info_ptr->total_size_in = 0;
    channel_info_ptr->total_size_out = 0;
                                     
    mdb_info_store(channel_info_ptr);
}


void smc_mdb_all_destroy( void )
{
    SMC_TRACE_PRINTF_INFO("smc_mdb_all_destroy()");

    mdb_channel_info_delete();
}


void* smc_mdb_alloc( uint8_t channel_id, uint32_t length )
{
    void* ptr;
 
    smc_mdb_channel_info_t* channel_info_ptr = mdb_info_fetch(channel_id);

    assert(channel_info_ptr != NULL);
    
    SMC_TRACE_PRINTF_INFO("smc_mdb_alloc(): channel_id: %d, length: %d", channel_info_ptr->id, length);
    
    assert(length > 0);
    assert(channel_info_ptr->pool_out != NULL);
    
#if TLSF_USE_REF
    ptr = malloc_ex(length, channel_info_ptr->pool_out);
#else
    ptr = tlsf_malloc(channel_info_ptr->pool_out, length);
#endif
    
    return ptr;
}


void smc_mdb_free( uint8_t channel_id, void* ptr )
{
    assert(ptr != NULL);
    
    smc_mdb_channel_info_t* channel_info_ptr = mdb_info_fetch(channel_id);
    
    assert(channel_info_ptr != NULL);
    
    SMC_TRACE_PRINTF_INFO("smc_mdb_free(): channel_id: %d, ptr: 0x%08x", channel_info_ptr->id, ptr);
    
    assert(channel_info_ptr->pool_out != NULL);
    assert(channel_info_ptr->total_size_out > 0);
    
#if TLSF_USE_REF
    free_ex(ptr, channel_info_ptr->pool_out);
#else
    tlsf_free(channel_info_ptr->pool_out, ptr);
#endif
}


void smc_mdb_copy( void* target, void* source, uint32_t length )
{
    SMC_TRACE_PRINTF_INFO("smc_mdb_copy(): target: 0x%08x, source: 0x%08x, length: %d", target, source, length);

    assert(target != NULL);
    assert(source != NULL);
    assert(length != 0);

    memcpy(target, source, length);
}


uint8_t smc_mdb_address_check( uint8_t channel_id, void* ptr, uint8_t direction )
{
    uint8_t result = FALSE;
    void* begin_ptr = NULL;
    void* end_ptr = NULL;
    
    smc_mdb_channel_info_t* channel_info_ptr = mdb_info_fetch(channel_id);
    
    assert(channel_info_ptr != NULL);

    SMC_TRACE_PRINTF_INFO("smc_mdb_address_check(): channel_id: %d, ptr: 0x%08x, direction: %d", channel_info_ptr->id, ptr, direction);

    switch (direction)
    {
    case SMC_MDB_IN:
        assert(channel_info_ptr->pool_in != NULL);
        begin_ptr = channel_info_ptr->pool_in;
        end_ptr = (uint8_t*)channel_info_ptr->pool_in + channel_info_ptr->total_size_in;
        break;
    case SMC_MDB_OUT:
        assert(channel_info_ptr->pool_out != NULL);
        begin_ptr = channel_info_ptr->pool_out;
        end_ptr = (uint8_t*)channel_info_ptr->pool_out + channel_info_ptr->total_size_out;
        break;
    default:
        assert((direction != SMC_MDB_IN) && (direction != SMC_MDB_OUT));
        break;
    }
    
    if ((ptr >= begin_ptr) && (ptr < end_ptr))
    {
        result = TRUE;
    }

    SMC_TRACE_PRINTF_INFO("smc_mdb_address_check() return: %s", result != FALSE ? "TRUE" : "FALSE");
    
    return result;
}


void smc_mdb_channel_info_dump( void )
{
    if (mdb_channel_info_set == NULL)
    {
        SMC_TRACE_PRINTF_INFO("smc_mdb_channel_info_dump(): channel_amount: 0");
    }
    else
    {
        SMC_TRACE_PRINTF_INFO("smc_mdb_channel_info_dump(): channel_amount %d", mdb_channel_info_set->channel_amount);
        for (uint8_t i = 0; i < mdb_channel_info_set->channel_amount; i++)
        {
            SMC_TRACE_PRINTF_INFO("channel_id: %d", mdb_channel_info_set->channel_info[i].id);
            SMC_TRACE_PRINTF_INFO("    pool_in:  0x%08x", mdb_channel_info_set->channel_info[i].pool_in);
            SMC_TRACE_PRINTF_INFO("    pool_out: 0x%08x", mdb_channel_info_set->channel_info[i].pool_out);
            SMC_TRACE_PRINTF_INFO("    total_size_in:  %d", mdb_channel_info_set->channel_info[i].total_size_in);
            SMC_TRACE_PRINTF_INFO("    total_size_out: %d", mdb_channel_info_set->channel_info[i].total_size_out);
        }
    }
}

uint32_t smc_mdb_channel_frag_get( uint8_t ch_id )
{ 
#if !TLSF_USE_REF
    pool_t *pool = mdb_channel_info_set->channel_info[ch_id].pool_out;
    return pool->free_blocks;
#else
    SMC_TRACE_PRINTF_INFO("smc_mdb_channel_frag_get(): no fragmentation information available in used TLSF implementation");
    return 0;
#endif
}

uint32_t smc_mdb_channel_free_space_get( uint8_t ch_id )
{
#if !TLSF_USE_REF
    pool_t *pool = mdb_channel_info_set->channel_info[ch_id].pool_out;
    return  pool->free_space;
#else
    SMC_TRACE_PRINTF_INFO("smc_mdb_channel_free_space_get(): no free space information available in used TLSF implementation");
    return 0;
#endif
}

uint32_t smc_mdb_channel_free_space_min_get( uint8_t ch_id )
{
#if !TLSF_USE_REF
    pool_t *pool = mdb_channel_info_set->channel_info[ch_id].pool_out;
    return  pool->free_space_min;
#else
    SMC_TRACE_PRINTF_INFO("smc_mdb_channel_free_space_min_get(): no free space information available in used TLSF implementation");
    return 0;
#endif
}
uint32_t smc_mdb_channel_largest_free_block_get( uint8_t ch_id )
{
#if !TLSF_USE_REF
    pool_t *pool = mdb_channel_info_set->channel_info[ch_id].pool_out;
    uint32_t fli = tlsf_fls(pool->fl_bitmap);
    uint32_t sli = tlsf_fls(pool->sl_bitmap[fli]);
    uint32_t size = 1U << (fli + FL_INDEX_SHIFT - 1);
    size += sli << (fli + FL_INDEX_SHIFT - 1 - SL_INDEX_COUNT_LOG2);
    return size;
#else
    SMC_TRACE_PRINTF_INFO("smc_mdb_channel_largest_free_block_get(): no largest free block information available in used TLSF implementation");
    return 0;
#endif
}

