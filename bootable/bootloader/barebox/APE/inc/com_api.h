/* com_api.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#define UL_CONV_ENDIAN(data)	(ulong)(((data&0xFF000000)>>24) | ((data&0x00FF0000)>>8) | ((data&0x0000FF00)<<8) | ((data&0x000000FF)<<24))
#define UI_CONV_ENDIAN(data)	(uint)(((data&0xFF00)>>8) | ((data&0x00FF)<<8))
