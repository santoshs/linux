/*************************************************************************
 **     Copyright (C) 2010 Nokia Corporation. All rights reserved.      **
 **                                                                     **
 ** Permission is hereby granted, free of charge, to any person         **
 ** obtaining a copy of this software and associated documentation      **
 ** files (the "Software"), to deal in the Software without             **
 ** restriction, including without limitation the rights to use, copy,  **
 ** modify, merge, publish, distribute, sublicense, and/or sell copies  **
 ** of the Software, and to permit persons to whom the Software is      **
 ** furnished to do so, subject to the following conditions:            **
 **                                                                     **
 ** The above copyright notice and this permission notice shall be      **
 ** included in all copies or substantial portions of the Software.     **
 ** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,     **
 ** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF  **
 ** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND               **
 ** NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS **
 ** BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN  **
 ** ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN   **
 ** CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE    **
 ** SOFTWARE.                                                           **
 **                                                                     **
 *************************************************************************
 **                                                                     **
 ** File:  arm.h                                                        **
 **                                                                     **
 ** Desc:  ARM specific definitions                                     **
 **                                                                     **
 *************************************************************************/

#ifndef ARM_H
#define ARM_H

#define MODE_USER                               0x10
#define MODE_FIQ                                0x11
#define MODE_IRQ                                0x12
#define MODE_SVC                                0x13
#define MODE_ABORT                              0x17
#define MODE_UNDEF                              0x1B
#define MODE_SYSTEM                             0x1F
#define MODE_MONITOR                            0x16
#define MODE_CLR_MASK                           0x1F

#define ARM_THUMB_MODE_MASK                     0x20

#define FIQ_IRQ_MASK                            0xC0
#define FIQ_MASK                                0x40
#define IRQ_MASK                                0x80

#endif /* ARM_H */

/* vim: set autoindent shiftwidth=4 smarttab expandtab : */
/* -*- mode: C; c-basic-indent: 4; indent-tabs-mode: nil -*- */
