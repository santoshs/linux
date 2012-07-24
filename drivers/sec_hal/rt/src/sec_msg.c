
/* ************************************************************************* **
**                               Renesas                                     **
** ************************************************************************* */

/* *************************** COPYRIGHT INFORMATION *********************** **
** This program contains proprietary information that is a trade secret of   **
** Renesas and also is protected as an unpublished work under                **
** applicable Copyright laws. Recipient is to retain this program in         **
** confidence and is not permitted to use or make copies thereof other than  **
** as permitted in a written agreement with Renesas.                         **
**                                                                           **
** All rights reserved. Company confidential.                                **
* ************************************************************************** */
/*
 *
 * Implements secure message interface. Byte order handling is not yet
 * implemented, for now it is assumed little endian is used.
 *
 */

#ifndef SEC_MSG_CFILE_
#define SEC_MSG_CFILE_

#include "sec_serv_api.h"
#include "sec_hal_rt_cmn.h"

//#include <linux/stdlib.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/kernel.h>
//#include <linustdio.h>
#ifdef SECURE_ENVIRONMENT

#include <string.h>
#include "sla.h"
/*!
 * Fill in secure environment equivalent for functions below..
 * (Standard functionality expected.)
 */
#define sec_memcpy      SLA_MEMCPY
#define sec_calloc      sec_env_calloc
#define sec_free        sec_env_free

/* allocate and zero initialize interconnect memory */
void *sec_env_calloc(uint32_t blocks, uint32_t block_bytes)
{
  return (void *)SLA_RPC_CALL((uint32_t)0, SEC_SERV_RPC_ALLOC,
                              block_bytes, blocks, 0);
}

/* free interconnect memory */
void sec_env_free(void *mem)
{
  (void)SLA_RPC_CALL((uint32_t)0, SEC_SERV_RPC_FREE, (uint32_t)mem, 0, 0);
}

#elif defined(PUBLIC_ENVIRONMENT)

#ifndef NULL
#define NULL (void*)0
#endif /* NULL */

unsigned long sec_hal_mem_msg_area_memcpy(void *dst, const void *src, unsigned long sz);
void* sec_hal_mem_msg_area_calloc(unsigned int n, unsigned int sz);
void  sec_hal_mem_msg_area_free(void *virt_addr);
unsigned long sec_hal_mem_msg_area_write(void *dst, const void *src,	unsigned long sz);
unsigned long sec_hal_mem_msg_area_read(void *dst, const void *src,	unsigned long sz);

#define sec_memcpy      sec_hal_mem_msg_area_memcpy
//#define sec_memcpy  memcpy
#define sec_calloc      sec_hal_mem_msg_area_calloc
#define sec_free        sec_hal_mem_msg_area_free
#define sec_msgwrite      sec_hal_mem_msg_area_write
#define sec_msgread      sec_hal_mem_msg_area_read
#else /* WORKSTATION TESTING */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define sec_memcpy      memcpy
#define sec_calloc      calloc
#define sec_free        free

#endif

/*!
 * Secure message element header type
 */
typedef struct {
  uint8_t elem_id;      /* element identifier */
  uint8_t reserved;     /* reserved */
  uint16_t size;        /* unaligned byte size of element data */
} sec_msg_elem_t;

/*!
 * Secure message header
 */
struct sec_msg {
  uint32_t object_id;   /* object identifier (reserved) */
  uint8_t version;      /* service specific message version */
  uint8_t byte_order;   /* byte order used in message */
  uint16_t size;        /* total byte size of message */
};

/*!
 * Callback used for allocating memory via special allocator.
 */
typedef void* (*cb_calloc)(uint32_t n, uint32_t size);

/*!
 * Callback used for freeing memory via special de-allocator.
 */
typedef void  (*cb_free)(void *ptr);


static inline sec_msg_elem_t *_sec_msg_elem_get(const sec_msg_handle_t *handle);

static sec_msg_status_t _sec_msg_elem_data_read(cb_memcpy f_ptr,
                                                sec_msg_handle_t *handle,
                                                const uint16_t read_length,
                                                void *data,
                                                uint16_t *data_length);

static sec_msg_status_t _sec_msg_elem_data_write(cb_memcpy f_ptr,
                                                 sec_msg_handle_t *handle,
                                                 const void *data,
                                                 const uint16_t write_length,
                                                 const uint8_t param_id);

static inline uint32_t _sec_msg_align(uint32_t size);

/*!
 * Generic alignment.
 *
 * @param uint32_t      size            Number to align.
 *
 * @retval uint32_t     Aligned result.
 *
 */
static inline uint32_t _sec_msg_align(uint32_t size)
{
  return ((size + 3) & ~3);
}

/*!
 * Get element header from handle.
 *
 * @param sec_msg_handle_t *handle      Handle from where to get element.
 *
 * @retval sec_msg_elem_t *             Element at current position.
 *
 */
static inline sec_msg_elem_t *_sec_msg_elem_get(const sec_msg_handle_t *handle)
{
  sec_msg_elem_t *elem =
      (sec_msg_elem_t *)((uint8_t *)handle->msg + handle->offset);

  if ((uint8_t *)elem + sizeof(sec_msg_elem_t) >
      (uint8_t *)handle->msg + handle->msg->size)
    {
      return NULL;
    }

  return elem;
}

/*!
 * Get data field from an element.
 *
 * @param sec_msg_elem_t *      elem    Element
 *
 * @retval void *               Data.
 *
 */
static inline void *_sec_msg_elem_data_get(const sec_msg_elem_t *elem)
{
  return (void *)((uint8_t *)elem + sizeof(sec_msg_elem_t));
}

/*!
 * Write data to message element.
 *
 * @param handle
 *      Handle to message. Handle will be forwarded to the next element
 *      after write.
 * @param data
 *      Data to write.
 * @param data_size
 *      Size of data.
 * @param param_id
 *      Parameter id.
 *
 * @retval sec_msg_status_t     Status of write operation.
 *
 */
static sec_msg_status_t _sec_msg_elem_data_write(cb_memcpy f_ptr,
                                                 sec_msg_handle_t *handle,
                                                 const void *data,
                                                 const uint16_t write_length,
                                                 const uint8_t param_id)
{
  sec_msg_elem_t *elem = 0;
  void *elem_data = 0;
 uint8_t *ptemp;
 int i;
 
printk("sec api sec_msg_elem_data_write in \n");
  if (!handle || !handle->msg)
    {
		printk("  Bad address null pointer \n");
              return SEC_MSG_STATUS_NULL_PTR;
    }
    printk(" handle 0x%x Msg info msg hex:%x offs:%d msgsz:%d byte order 0x%x obj id 0x%x version 0x%x \n",
		handle, handle->msg,handle->offset,handle->msg->size,handle->msg->byte_order,handle->msg->object_id,handle->msg->version);
  if ((uint8_t *)handle->msg + handle->offset >=
      (uint8_t *)handle->msg + handle->msg->size)
    {
        printk("Param out of range \n");
        return SEC_MSG_STATUS_PARAM_OUT_OF_RANGE;
    }

  elem = _sec_msg_elem_get(handle);
  if (!elem)
    {
      printk("No Params \n");
      return SEC_MSG_STATUS_PARAM_NULL;
    }

  elem_data = _sec_msg_elem_data_get(elem);
  if ((uint8_t *)elem_data + write_length >
      (uint8_t *)handle->msg + handle->msg->size)
    {
      printk("param out of range \n");
      return SEC_MSG_STATUS_PARAM_OUT_OF_RANGE;
    }
    printk(" element 0x%x elem data 0x%x  len 0x%x elm id 0x%x \n",elem,elem_data, write_length, param_id );
 // elem->size = write_length;
  // elem->elem_id = param_id;
      iowrite16(write_length,&elem->size);
     iowrite8(param_id, &elem->elem_id);	  
  switch (write_length)
    {
#ifdef SECURE_ENVIRONMENT
    case 1:
      *(uint8_t *)elem_data = *(uint8_t *)data;
      break;
    case 2:
      *(uint16_t *)elem_data = *(uint16_t *)data;
      break;
    case 4:
      *(uint32_t *)elem_data = *(uint32_t *)data;
      break;
    case 8:
      *(uint64_t *)elem_data = *(uint64_t *)data;
      break;
#endif /* SECURE_ENVIRONMENT */
    default:
     printk("write data to ICRAM  \n");
      (*f_ptr)(elem_data, data, write_length);
      break;
    }
  //  printk(" using iowrite32 \n");
   // iowrite32((*(uint32_t *)data),elem_data);
  handle->offset += sizeof(sec_msg_elem_t) + _sec_msg_align(elem->size);
#if 0
  ptemp = handle->msg;
  for ( i = 0; i < handle->offset ; i++)
  {
     printk("Read ICRAM  data 0x%x addr 0x%x\n",ioread8(ptemp),ptemp);
     ptemp++;
  }
#endif
printk(" sec api sec_msg_elem_data_write exit");

  return SEC_MSG_STATUS_OK;
}

/*!
 * Read data from message element.
 *
 * @param[in,out] handle
 *      Handle to message. Handle will be forwarded to the next element
 *      after read.
 * @param[in,out] data_length
 *      Length of data to read. Data length will be given as zero when reading
 *      a pointer.
 * @param[in,out] data
 *      Data buffer.
 * @param[out] data_length
 *      Byte length of data within element, used only when reading a pointer.
 *
 * @retval sec_msg_status_t     Status of read operation.
 *
 */
static sec_msg_status_t _sec_msg_elem_data_read(cb_memcpy f_ptr,
                                                sec_msg_handle_t *handle,
                                                const uint16_t read_length,
                                                void *data,
                                                uint16_t *data_length)
{
  int32_t bytes = 0;
  sec_msg_elem_t *elem = 0;
  void *dptr = 0;

  if (!handle || !handle->msg)
   {
     return SEC_MSG_STATUS_NULL_PTR;
   }

  if ((uint8_t *)handle->msg + handle->offset >=
      (uint8_t *)handle->msg + handle->msg->size)
    {
      return SEC_MSG_STATUS_PARAM_OUT_OF_RANGE;
    }

  elem = _sec_msg_elem_get(handle);
  if (NULL == elem)
    {
      return SEC_MSG_STATUS_PARAM_NULL;
    }

  if (elem->size == 0)
    {
      return SEC_MSG_STATUS_PARAM_EMPTY;
    }

  dptr = _sec_msg_elem_data_get(elem);
  if ((uint8_t *)dptr + read_length >
      (uint8_t *)handle->msg + handle->msg->size)
    {
      return SEC_MSG_STATUS_PARAM_OUT_OF_RANGE;
    }

  if (read_length &&
      ((read_length > elem->size) || (read_length < elem->size)))
    {
      return SEC_MSG_STATUS_PARAM_SIZE_MISMATCH;
    }

  bytes = elem->size > read_length ? read_length : elem->size;

  switch (read_length)
    {
#ifdef SECURE_ENVIRONMENT
    case 0:
      if (!data_length)
        {
          return SEC_MSG_STATUS_FAIL;
        }
      *(void **)data = dptr;
      *data_length = elem->size;
      break;
    case 1:
      *(uint8_t *)data = *(uint8_t *)dptr;
      break;
    case 2:
      *(uint16_t *)data = *(uint16_t *)dptr;
      break;
    case 4:
      *(uint32_t *)data = *(uint32_t *)dptr;
      break;
    case 8:
      *(uint64_t *)data = *(uint64_t *)dptr;
      break;
#endif /* SECURE_ENVIRONMENT */
    default:
#if 0
        printk("read data to ICRAM  \n");
#endif
      (*f_ptr)(data, dptr, bytes);
      break;
   }

  handle->offset += _sec_msg_align(elem->size) + sizeof(sec_msg_elem_t);

  return SEC_MSG_STATUS_OK;
}

sec_msg_t *_sec_msg_alloc(cb_calloc f_ptr,
                         sec_msg_handle_t *handle,
                         uint16_t size,
                         uint32_t object_id,
                         uint8_t msg_version,
                         uint8_t byte_order)
{
    
    printk("_sec_msg_alloc  called \n");

    uint16_t msg_size = _sec_msg_align(size + sizeof(sec_msg_t));

    

      sec_msg_t *msg = (sec_msg_t *)(*f_ptr)(1, msg_size);

     if (!msg)
    {
         printk(" sec_msg_alloc null \n");
         return NULL;
    }
   printk("sec_msg_alloc size=%d sec_msg_t=%d msg_size=%d  msg_addr 0x%X \n",size,sizeof(sec_msg_t),msg_size,msg );	
 #if 0   
  msg->byte_order = byte_order;
  msg->version = msg_version;
  msg->object_id = object_id;
  msg->size = msg_size;
  #else
  iowrite32(object_id,&msg->object_id);
  iowrite8(msg_version,&msg->version);
  iowrite8(byte_order,&msg->byte_order);
  iowrite16(msg_size,&msg->size);  
  #endif

  handle->msg = msg;
  handle->offset = sizeof(sec_msg_t);

 

 #if 0 
      iowrite32(object_id,msg+0);
      iowrite8(msg_version,msg+4);
      iowrite8(byte_order,msg+5);
      iowrite16(msg_size,msg+6);

	__raw_writel(0x00,(msg+0));
	__raw_writel(22,(msg+0));


	printk("Nagasri called sec_msg_alloc calcrd %d \n",__raw_readl(msg+0));
	printk("Nagasri called sec_msg_alloc calcru %u \n",__raw_readl(msg+0));

	printk("Nagasri called sec_msg_alloc calcpoid %d \n",(uint32_t *)(msg+0));
	printk("Nagasri called sec_msg_alloc calcpoiu %u \n",(uint32_t *)(msg+0));



printk("Nagasri called sec_msg_alloc calc handle sz=%d off sz=%d \n",sizeof(*(handle->msg)),handle->offset);
printk("Nagasri called sec_msg_alloc out");
#endif

  return msg;
}

sec_msg_status_t _sec_msg_free(cb_free f_ptr, sec_msg_t *msg)
{
  if (msg)
    {
      (*f_ptr)(msg);
      return SEC_MSG_STATUS_OK;
    }

  return SEC_MSG_STATUS_NULL_PTR;
}

sec_msg_status_t sec_msg_open(sec_msg_handle_t *handle, const sec_msg_t *msg)
{
  if (!msg || !handle)
    {
      return SEC_MSG_STATUS_NULL_PTR;
    }

#ifdef SECURE_ENVIRONMENT
  /* check that message is completely in interconnect */
  if (!SLA_IS_NON_SEC_AREA((void *)msg, (void *)((uint8_t *)msg + msg->size)))
    {
      return SEC_MSG_STATUS_INVALID_PTR;
    }
#endif /* SECURE_ENVIRONMENT */

  handle->msg = (sec_msg_t *)msg;
  handle->offset = sizeof(sec_msg_t);

  return SEC_MSG_STATUS_OK;
}

sec_msg_status_t sec_msg_close(sec_msg_handle_t *handle)
{
  if (!handle)
    {
      return SEC_MSG_STATUS_NULL_PTR;
    }
  handle->msg = NULL;
  handle->offset = 0;

  return SEC_MSG_STATUS_OK;
}

sec_msg_status_t sec_msg_byte_order_get(const sec_msg_t *msg,
                                        uint8_t *byte_order)
{
  if (!msg || !byte_order)
    {
      return SEC_MSG_STATUS_NULL_PTR;
    }
  *byte_order = msg->byte_order;

  return SEC_MSG_STATUS_OK;
}

sec_msg_status_t sec_msg_size_get(const sec_msg_t *msg, uint16_t *size)
{
  if (!msg || !size)
    {
      return SEC_MSG_STATUS_NULL_PTR;
    }
  *size = msg->size;

  return SEC_MSG_STATUS_OK;
}

sec_msg_status_t sec_msg_version_get(const sec_msg_t *msg,
                                     uint8_t *version)
{
  if (!msg || !version)
    {
      return SEC_MSG_STATUS_NULL_PTR;
    }
  *version = msg->version;

  return SEC_MSG_STATUS_OK;
}

sec_msg_status_t sec_msg_object_id_get(const sec_msg_t *msg,
                                       uint32_t *object_id)
{
  if (!msg || !object_id)
    {
      return SEC_MSG_STATUS_NULL_PTR;
    }

  *object_id = msg->object_id;

  return SEC_MSG_STATUS_OK;
}

uint16_t sec_msg_param_size(const uint16_t size)
{

 return _sec_msg_align(sizeof(sec_msg_elem_t) + size);
}

sec_msg_status_t sec_msg_param_size_get(const sec_msg_handle_t *handle,
                                        uint16_t *param_size)
{
  sec_msg_elem_t *elem = 0;

  if (!handle || !handle->msg || !param_size)
    {
      return SEC_MSG_STATUS_NULL_PTR;
    }

  elem = _sec_msg_elem_get(handle);
  if (!elem)
    {
      return SEC_MSG_STATUS_PARAM_NULL;
    }
  *param_size = elem->size;

  return SEC_MSG_STATUS_OK;
}

sec_msg_status_t sec_msg_rewind(sec_msg_handle_t *handle)
{
  if (!handle)
    {
      return SEC_MSG_STATUS_NULL_PTR;
    }
  handle->offset = sizeof(sec_msg_t);

  return SEC_MSG_STATUS_OK;
}

sec_msg_status_t sec_msg_param_write(sec_msg_handle_t *handle,
                                     const void *data,
                                     const uint16_t data_length,
                                     const uint8_t param_id)
{
  return _sec_msg_elem_data_write(&sec_msgwrite, handle, data, data_length, param_id);
}
sec_msg_status_t _sec_msg_param_write(cb_memcpy f_ptr,
                                      sec_msg_handle_t *handle,
                                      const void *data,
                                      const uint16_t data_length,
                                      const uint8_t param_id)
{
	return _sec_msg_elem_data_write(f_ptr, handle, data, data_length, param_id);
}

sec_msg_status_t sec_msg_param_write8(sec_msg_handle_t *handle,
                                      const uint8_t data,
                                      const uint8_t param_id)
{
  return _sec_msg_elem_data_write(&sec_msgwrite, handle, &data, sizeof(uint8_t), param_id);
}

sec_msg_status_t sec_msg_param_write16(sec_msg_handle_t *handle,
                                       const uint16_t data,
                                       const uint8_t param_id)
{
  return _sec_msg_elem_data_write(&sec_msgwrite, handle, &data, sizeof(uint16_t), param_id);
}

sec_msg_status_t sec_msg_param_write32(sec_msg_handle_t *handle,
                                       const uint32_t data,
                                       const uint8_t param_id)
{
  return _sec_msg_elem_data_write(&sec_msgwrite, handle, &data, sizeof(uint32_t), param_id);
}
sec_msg_status_t _sec_msg_param_write32(cb_memcpy f_ptr,
                                        sec_msg_handle_t *handle,
                                        const uint32_t data,
                                        const uint8_t param_id)
{
  return _sec_msg_elem_data_write(f_ptr, handle, &data, sizeof(uint32_t), param_id);
}

sec_msg_status_t sec_msg_param_write64(sec_msg_handle_t *handle,
                                       const uint64_t data,
                                       const uint8_t param_id)
{
  return _sec_msg_elem_data_write(&sec_msgwrite, handle, &data, sizeof(uint64_t), param_id);
}

sec_msg_status_t sec_msg_param_ptr_read(sec_msg_handle_t *handle,
                                        void **data_ptr,
                                        uint16_t *data_length)
{
  return _sec_msg_elem_data_read(&sec_msgread, handle, 0, (void *)data_ptr, data_length);
}

sec_msg_status_t sec_msg_param_read(sec_msg_handle_t *handle,
                                    void *readbuf,
                                    const uint16_t length)
{
  return _sec_msg_elem_data_read(&sec_msgread, handle, length, readbuf, NULL);
}
sec_msg_status_t _sec_msg_param_read(cb_memcpy f_ptr,
                                     sec_msg_handle_t *handle,
                                     void *readbuf,
                                     const uint16_t length)
{
  return _sec_msg_elem_data_read(f_ptr, handle, length, readbuf, NULL);
}

sec_msg_status_t sec_msg_param_read8(sec_msg_handle_t *handle, uint8_t *param)
{
  return _sec_msg_elem_data_read(&sec_msgread, handle, sizeof(uint8_t), (void *)param, NULL);
}

sec_msg_status_t sec_msg_param_read16(sec_msg_handle_t *handle, uint16_t *param)
{
  return _sec_msg_elem_data_read(&sec_msgread, handle, sizeof(uint16_t), (void *)param, NULL);
}

sec_msg_status_t sec_msg_param_read32(sec_msg_handle_t *handle, uint32_t *param)
{
  return _sec_msg_elem_data_read(&sec_msgread, handle, sizeof(uint32_t), (void *)param, NULL);
}

sec_msg_status_t sec_msg_param_read64(sec_msg_handle_t *handle, uint64_t *param)
{
  return _sec_msg_elem_data_read(&sec_msgread, handle, sizeof(uint64_t), (void *)param, NULL);
}

sec_msg_t *sec_msg_alloc(sec_msg_handle_t *handle,
                         uint16_t size,
                         uint32_t object_id,
                         uint8_t msg_version,
                         uint8_t byte_order)
{
  return _sec_msg_alloc(&sec_calloc, handle, size, object_id, msg_version, byte_order);
}

sec_msg_status_t sec_msg_free(sec_msg_t *msg)
{
  return _sec_msg_free(&sec_free, msg);
}

#endif /* SEC_MSG_CFILE_ */

