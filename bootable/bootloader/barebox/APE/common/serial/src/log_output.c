/* log_output.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "log_output.h"

extern void uart_putc( char c );

static char uart_buffer[512];
#define _INTSIZEOF(n)    ((sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1))

/**
*	uart_printf	-	This function prints log over UART.
* 	@param	:	char* format  - Strings to be sent.
* 	@return :	None
*/
#ifdef __TRACELOG__
RC uart_printf(const char *format, ...)
{
	RC len;
	RC i;
	va_list ap;

	memset(uart_buffer, 0x0, sizeof(uart_buffer));
	
	if (NULL == format)	{
		len = 0;
	} else {
		va_start(ap, format);
		len = vsnprintf(&uart_buffer[0], sizeof(uart_buffer), format, ap);
		va_end(ap);
		
		if (-1 < len) {
			uart_putc('\r');
			
			len=(len<sizeof(uart_buffer))?len:sizeof(uart_buffer);
			
			for (i = 0; i < len; i++)
			{
				uart_putc(uart_buffer[i]);
			}
		}
	}
	return len;
}
#else
RC uart_printf(const char *format, ...)
{
	/* do nothing */
	return 0;
}
#endif