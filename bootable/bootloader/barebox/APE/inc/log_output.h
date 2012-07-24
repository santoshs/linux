/*
 * log_output.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef __LOG_OUTPUT_H__
#define __LOG_OUTPUT_H__

#include "compile_option.h"
#include "serial.h"
#include "string.h"

int uart_printf(const char *format, ...);
#define PRINTF    uart_printf

#endif /* __LOG_OUTPUT_H__ */