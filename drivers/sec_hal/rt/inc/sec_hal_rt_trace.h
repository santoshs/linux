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
** Copyright (C) 2011-2012 Renesas Electronics Corp.                         **
** All rights reserved.                                                      **
** ************************************************************************* */
#ifndef SEC_HAL_TRACE_H_
#define SEC_HAL_TRACE_H_

#ifndef MODNAME
#define MODNAME "SEC_HAL"
#endif /* MODNAME */


#if (defined(SEC_HAL_TRACE_TO_KERNEL) && !defined(SEC_HAL_TRACE_LOCAL_DISABLE))
/* Tracing macros for kernel space tracing.*/
#include <linux/kernel.h>
#include <linux/sched.h>

extern const char* k_module_name;

/* use at entry point, outside of main function */
#define SEC_HAL_TRACE_BUFFER_SIZE 128
#define SEC_HAL_TRACE_DEF_GLOBALS const char* k_module_name = MODNAME;
#define SEC_HAL_TRACE_INIT
#define SEC_HAL_TRACE_ALLOC(val, lit) const char* val = lit;
#define SEC_HAL_TRACE_LEVEL1 KERN_INFO
#define SEC_HAL_TRACE_LEVEL2 KERN_INFO
#define SEC_HAL_TRACE_LEVEL3 KERN_INFO
#define SEC_HAL_TRACE_FUNCTION printk
#define SEC_HAL_TRACE_ENTRY \
{SEC_HAL_TRACE_FUNCTION(SEC_HAL_TRACE_LEVEL1 "%s[%i,%i]:%s>\n", k_module_name, current->tgid, current->pid, __FUNCTION__);}
#define SEC_HAL_TRACE_EXIT \
{SEC_HAL_TRACE_FUNCTION(SEC_HAL_TRACE_LEVEL1 "%s[%i,%i]:%s<\n", k_module_name, current->tgid, current->pid, __FUNCTION__);}
#define SEC_HAL_TRACE_EXIT_INFO(fmt, ...) \
{SEC_HAL_TRACE_ALLOC(k_val, fmt)\
 char newfmt[SEC_HAL_TRACE_BUFFER_SIZE] = {0};\
 sprintf(newfmt, SEC_HAL_TRACE_LEVEL2 "%s[%i,%i]:%s< %s\n", k_module_name, current->tgid, current->pid, __FUNCTION__, k_val);\
 SEC_HAL_TRACE_FUNCTION(newfmt, ##__VA_ARGS__);}
#define SEC_HAL_TRACE(fmt, ...) \
{SEC_HAL_TRACE_ALLOC(k_val, fmt)\
 char newfmt[SEC_HAL_TRACE_BUFFER_SIZE] = {0};\
 sprintf(newfmt, SEC_HAL_TRACE_LEVEL2 "%s[%i,%i]:%s: %s\n", k_module_name, current->tgid, current->pid, __FUNCTION__, k_val);\
 SEC_HAL_TRACE_FUNCTION(newfmt, ##__VA_ARGS__);}
#define SEC_HAL_TRACE_INT(lit, integer) \
{SEC_HAL_TRACE_ALLOC(k_val, lit)\
 SEC_HAL_TRACE_FUNCTION(SEC_HAL_TRACE_LEVEL2 "%s[%i,%i]:%s: %s == %d\n", k_module_name, current->tgid, current->pid, __FUNCTION__, k_val, integer);}
#define SEC_HAL_TRACE_HEX(lit, hex) \
{SEC_HAL_TRACE_ALLOC(k_val, lit)\
 SEC_HAL_TRACE_FUNCTION(SEC_HAL_TRACE_LEVEL2 "%s[%i,%i]:%s: %s == 0x%X\n", k_module_name, current->tgid, current->pid, __FUNCTION__, k_val, hex);}
#define SEC_HAL_TRACE_STR(lit, str) \
{SEC_HAL_TRACE_ALLOC(k_val, lit)\
 SEC_HAL_TRACE_FUNCTION(SEC_HAL_TRACE_LEVEL2 "%s[%i,%i]:%s: %s == %s\n", k_module_name, current->tgid, current->pid, __FUNCTION__, k_val, str);}
#define SEC_HAL_TRACE_SECMSG(msg_ptr, res_buff) /* NOT IMPL, YET. */

#elif(defined(SEC_HAL_TRACE_TO_STDOUT) && !defined(SEC_HAL_TRACE_LOCAL_DISABLE))
/* Tracing macros for workstation env. */
#include <stdio.h>
#include <string.h>

/* use at entry point, outside of main function */
#define SEC_HAL_TRACE_BUFFER_SIZE 128
#define SEC_HAL_TRACE_DEF_GLOBALS
#define SEC_HAL_TRACE_INIT
#define SEC_HAL_TRACE_ALLOC(val, lit) const char* val = lit;
#define SEC_HAL_TRACE_LEVEL1
#define SEC_HAL_TRACE_LEVEL2
#define SEC_HAL_TRACE_LEVEL3
#define SEC_HAL_TRACE_FUNCTION printf
#define SEC_HAL_TRACE_ENTRY \
{char sec_hal_trace_buffer[SEC_HAL_TRACE_BUFFER_SIZE];\
 sprintf(sec_hal_trace_buffer, "%s>\n", __FUNCTION__);\
 SEC_HAL_TRACE_FUNCTION("%s", sec_hal_trace_buffer);}
#define SEC_HAL_TRACE_EXIT \
{char sec_hal_trace_buffer[SEC_HAL_TRACE_BUFFER_SIZE];\
 sprintf(sec_hal_trace_buffer, "%s<\n", __FUNCTION__);\
 SEC_HAL_TRACE_FUNCTION("%s", sec_hal_trace_buffer);}
#define SEC_HAL_TRACE_EXIT_INFO(fmt) \
{SEC_HAL_TRACE_ALLOC(k_val, fmt)\
 char sec_hal_trace_buffer[SEC_HAL_TRACE_BUFFER_SIZE];\
 sprintf(sec_hal_trace_buffer, "%s< %s\n", __FUNCTION__, k_val);\
 SEC_HAL_TRACE_FUNCTION("%s", sec_hal_trace_buffer);}
#define SEC_HAL_TRACE(fmt) \
{SEC_HAL_TRACE_ALLOC(k_val, fmt)\
 char sec_hal_trace_buffer[SEC_HAL_TRACE_BUFFER_SIZE];\
 sprintf(sec_hal_trace_buffer, "%s: %s\n", __FUNCTION__, k_val);\
 SEC_HAL_TRACE_FUNCTION("%s", sec_hal_trace_buffer);}
#define SEC_HAL_TRACE_INT(lit, integer) \
{SEC_HAL_TRACE_ALLOC(k_val, lit)\
 char sec_hal_trace_buffer[SEC_HAL_TRACE_BUFFER_SIZE];\
 sprintf(sec_hal_trace_buffer, "%s: %s == %d\n", __FUNCTION__, k_val, integer);\
 SEC_HAL_TRACE_FUNCTION("%s", sec_hal_trace_buffer);}
#define SEC_HAL_TRACE_HEX(lit, hex) \
{SEC_HAL_TRACE_ALLOC(k_val, lit)\
 char sec_hal_trace_buffer[SEC_HAL_TRACE_BUFFER_SIZE];\
 sprintf(g_sec_hal_trace_buffer, "%s: %s == 0x%X\n", __FUNCTION__, k_val, hex);\
 SEC_HAL_TRACE_FUNCTION("%s", sec_hal_trace_buffer);}
#define SEC_HAL_TRACE_STR(lit, str) \
{SEC_HAL_TRACE_ALLOC(k_val, lit)\
 char sec_hal_trace_buffer[SEC_HAL_TRACE_BUFFER_SIZE];\
 sprintf(sec_hal_trace_buffer, "%s: %s == %s\n", __FUNCTION__, k_val, str);\
 SEC_HAL_TRACE_FUNCTION("%s", sec_hal_trace_buffer);}
#define SEC_HAL_TRACE_SECMSG(msg_ptr, res_buff)

#else /* (defined SEC_HAL_TRACE_TO_KERNEL) */

#define SEC_HAL_TRACE_BUFFER_SIZE 0
#define SEC_HAL_TRACE_DEF_GLOBALS
#define SEC_HAL_TRACE_INIT
#define SEC_HAL_TRACE_ALLOC(val, lit)
#define SEC_HAL_TRACE_LEVEL1
#define SEC_HAL_TRACE_LEVEL2
#define SEC_HAL_TRACE_LEVEL3
#define SEC_HAL_TRACE_FUNCTION
#define SEC_HAL_TRACE_ENTRY
#define SEC_HAL_TRACE_EXIT
#define SEC_HAL_TRACE_EXIT_INFO(fmt, ...)
#define SEC_HAL_TRACE(fmt, ...)
#define SEC_HAL_TRACE_INT(lit, integer)
#define SEC_HAL_TRACE_HEX(lit, hex)
#define SEC_HAL_TRACE_STR(lit, str)
#define SEC_HAL_TRACE_SECMSG(msg_ptr, res_buff)

#endif /* (defined SEC_HAL_TRACE_TO_KERNEL) */


#endif /* SEC_HAL_TRACE_H_ */
