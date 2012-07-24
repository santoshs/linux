/*	ramset.s
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */
#-------------------------------------------
# Entry point
#-------------------------------------------
.arm
.global _start
.extern  ramset_main
_start:
    LDR     pc, = ramset_main       


