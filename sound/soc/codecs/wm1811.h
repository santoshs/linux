/*
 * wm1811.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __WM1811_H__
#define __WM1811_H__

#define WM1811_CACHE_SIZE 1570

struct wm1811_access_mask {
	unsigned short readable;   /* Mask of readable bits */
	unsigned short writable;   /* Mask of writable bits */
};

extern const struct wm1811_access_mask wm1811_access_masks[WM1811_CACHE_SIZE];

#endif	/* __WM1811_H__ */
