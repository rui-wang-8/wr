# Makefile - makefile for target/config/cpv5300
#
# Copyright 1984-2002 Wind River Systems, Inc.
# Copyright 1999 Motorola, Inc. All Rights Reserved
#
# modification history
# --------------------
# 01l,18jul02,tfr  Changed RAM_HIGH_ADRS, RAM_LOW_ADRS.
# 01k,05may02,rhe  Add bootrom.bin to RELEASE
# 01j,16apr02,dat  Update for T2.2 release, removed lptDrv.o smUtilLib.obj,
#    		   and fei82557End.obj from MACH_EXTRA, removed CPU_VARIANT
# 01i,30oct01,tpw  Standardize kernel links and bootrom conversions.
# 01h,25sep01,dat  Update for T2.2
# 01g,18sep01,hdn  set CPU to PENTIUM2
# 01f,01feb00,djs  incorporate WRS review comments
# 01e,28oct99,sjb  changed fei82557End.o to fei82557End.obj on MACH_EXTRA.
# 01d,26oct99,sjb  added fei82557End.o to MACH_EXTRA line.
# 01c,14oct99,scb  added DOC_FILES definition.
# 01b,22sep99,sjb  added lptDrv.o to MACH_EXTRA line
# 01a,02jul99,djs  written based on cpv5000/Makefile.
#
# DESCRIPTION
# This file contains rules for building VxWorks for the
# PC CPV5300.
#
# NOTE
# The constants ROM_TEXT_ADRS and RAM_HIGH_ADRS
# must be identical to those in config.h.
# 
# INCLUDES
#     makeTarget
#*/

CPU		= PENTIUM2
TOOL		= gnu

TGT_DIR = $(WIND_BASE)/target

include $(TGT_DIR)/h/make/defs.bsp
#include $(TGT_DIR)/h/make/make.$(CPU)$(TOOL)
#include $(TGT_DIR)/h/make/defs.$(WIND_HOST_TYPE)

## Only redefine make definitions below this point, or your definitions will
## be overwritten by the makefile stubs above.

DOC_FILES  = sysLib tyCoDrv sysTffs mkboot lm78 failsafe
TARGET_DIR = cpv5300
VENDOR     = Motorola
BOARD      = CPV5300

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

EXTRA_DEFINE       = -DCPV5300

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
