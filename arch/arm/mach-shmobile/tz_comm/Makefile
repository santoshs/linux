# 
#  Copyright © Renesas Mobile Corporation 2010 . All rights reserved     
# 
#  This material, including documentation and any related source code and information, is protected by 
#  copyright controlled by Renesas. All rights are reserved. Copying, including reproducing, storing, adapting,
#  translating and modifying, including decompiling or reverse engineering, any or all of this material
#  requires the prior written consent of Renesas. This material also contains confidential information, which
#  may not be disclosed to others without the prior written consent of Renesas.                                        
#
#
# Following environment specific symbols must be implemented:
#
#Error: L6218E: Undefined symbol hw_arm_dcache_maintenance (referred from sec_bridge_tz.o).
#Error: L6218E: Undefined symbol hw_arm_l2_cache_area_clean (referred from sec_bridge_tz.o).
#Error: L6218E: Undefined symbol hw_mmu_physical_address_get (referred from sec_bridge_tz.o).
#
# Assumes following directory structure
#
#./inc:
#    arm.h
#    arm.inc
#    cache_mmu_if.h
#    sec_comm.h
#    sec_dispatch.h
#    sec_msg.h
#    sec_serv_api.h
#    ssa.h
#    ssa.inc
#
#./src:
#    Makefile.sample
#    sec_bridge_tz.s
#    sec_comm.c
#    sec_dispatch.c
#    sec_msg.c
#

target=testcompile

#CC=armcc
#LD=armlink
#AS=armasm

#CC=arm-eabi-gcc
#LD=arm-eabi-ld
#AS=arm-eabi-as


#DEFINES 	=            -DPUBLIC_ROM                    -DLOADER                    -DPUBLIC_ENVIRONMENT
#ASDEFINES	= --predefine "PUBLIC_ROM SETA 1" --predefine "LOADER SETA 1" --predefine "TRUSTZONE SETA 1"

ccflags-y                 += -DPUBLIC_ENVIRONMENT -DSEC_HAL_TRACE_TO_KERNEL
#asflags-y                 += $(INCLUDES)


#CFLAGS_sec_bridge_tz.o := -c --apcs=/interwork --licretry --c99 --library_interface=aeabi_clib99 			\
#		--retain=calls --retain=calls:distinct --retain=libcalls --bss_threshold=0 		\
#		--cpu=Cortex-A9.no_neon.no_vfp --no_unaligned_access --littleend -g -O2 --thumb 	\
#		--no_inline  -DPUBLIC_ENVIRONMENT

#ASFLAGS_sec_bridge_tz.o := 	--apcs=/interwork --licretry --cpu=Cortex-A9.no_neon.no_vfp --no_unaligned_access 	\
#		--littleend -g $(ASDEFINES) 

SOURCES = 			\
		sec_bridge_tz.s	\

#obj-y		:= sec_bridge_tz.o sec_comm.o sec_dispatch.o sec_msg.o sec_hal_rt.o sec_lib.o
obj-$(CONFIG_ARM_TZ)		:= sec_comm.o
ccflags-y                 += -DPUBLIC_ENVIRONMENT -DSEC_HAL_TRACE_TO_KERNEL
