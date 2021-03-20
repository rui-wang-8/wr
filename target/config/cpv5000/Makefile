# Makefile - makefile for target/config/cpv5000
#
# Copyright 1984-2002 Wind River Systems, Inc.
# Copyright 1999 Motorola, Inc. All Rights Reserved
#
# modification history
# --------------------
# 01m,18jul02,tfr  Changed RAM_HIGH_ADRS, RAM_LOW_ADRS.
# 01l,20may02,rhe  Add bootrom.bin to RELEASE
# 01k,17apr02,dat  Update for T2.2 release, removed CPU_VARIANT
#		   and special lpt driver, and special .obj's
# 01j,30oct01,tpw  Standardize kernel links and bootrom conversions.
# 01i,25sep01,dat  Update for T2.2
# 01h,01feb00,djs  incorporate WRS review comments
# 01g,22sep99,sjb  added lptDrv.o to MACH_EXTRA line
# 01f,31aug99,scb  added DOC_FILES definition.
# 01e,07jun99,scb  port from T1 to T2.
# 01d,29apr99,scb  changed RAM_LOW_ADRS to 0x118000 for shared memory to work.
# 01c,14apr99,djs  changed TARGET_DIR to cpv5000
# 01b,01apr99,scb  Add support for shared memory
# 01a,08mar99,djs  written based on pcPentium, add define for CPV5000
#
# DESCRIPTION
# This file contains rules for building VxWorks for the
# PC CPV5000.
#
# NOTE
# The constants ROM_TEXT_ADRS and RAM_HIGH_ADRS
# must be identical to those in config.h.
# 
# INCLUDES
#     makeTarget
#*/

CPU		= PENTIUM
TOOL		= gnu

TGT_DIR = $(WIND_BASE)/target

include $(TGT_DIR)/h/make/defs.bsp
#include $(TGT_DIR)/h/make/make.$(CPU)$(TOOL)
#include $(TGT_DIR)/h/make/defs.$(WIND_HOST_TYPE)

## Only redefine make definitions below this point, or your definitions will
## be overwritten by the makefile stubs above.

DOC_FILES  = sysLib tyCoDrv sysTffs mkboot lm78 failsafe
TARGET_DIR = cpv5000
VENDOR     = Motorola
BOARD      = CPV5000

#
# The constants ROM_TEXT_ADRS, ROM_SIZE, and RAM_HIGH_ADRS are defined
# in config.h, MakeSkel, Makefile, and Makefile.*
# All definitions for these constants must be identical.
#

# ifdef BOOTCODE_IN_RAM
ROM_TEXT_ADRS      = 00008000	# ROM entry address - A: or C:
ROM_SIZE           = 00090000	# number of bytes of ROM space
# else
# ROM_TEXT_ADRS      = fff20000	# ROM entry address - EPROM
# ROM_SIZE           = 0007fe00	# number of bytes of ROM space
# endif

RAM_LOW_ADRS       = 00308000	# RAM text/data address
RAM_HIGH_ADRS      = 00108000	# RAM text/data address

EXTRA_DEFINE       = -DCPV5000

MACH_EXTRA	   = 

RELEASE += bootrom_uncmp.bin bootrom.bin mkboot.o


## Only redefine make definitions above this point, or the expansion of 
## makefile target dependencies may be incorrect.

include $(TGT_DIR)/h/make/rules.bsp
#include $(TGT_DIR)/h/make/rules.$(WIND_HOST_TYPE)


# Following bootable vxWorks should be placed in bootable diskette
# by "mkboot" or "mkbootFd, mkbootAta" utility.
# vxWorks_low should be downloaded by bootrom_high boot image.
#   vxWorks.st_rom	- bootable vxWorks.st(compressed): upper mem
#   bootrom		- bootrom(compressed):		   lower mem
#   bootrom_uncmp	- bootrom:			   lower mem
#   bootrom_high	- bootrom(compressed):		   upper mem

bootrom_high :
	$(CC) -c $(CFLAGS) -DRAM_DST_ADRS=0x$(RAM_LOW_ADRS) $(BOOTINIT)
	- @ $(RM) bootrom.Z.s $@
	$(MAKE) LD_HIGH_FLAGS="-Ttext $(RAM_LOW_ADRS)" bootrom.Z.s
	$(MAKE) bootrom LD_LOW_FLAGS="$(LD_HIGH_FLAGS)"
	$(CP) bootrom $@
	- @ $(RM) bootrom.Z.* bootrom bootInit.o
