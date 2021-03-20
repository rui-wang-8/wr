# Makefile - makefile for target/config/cpn5360
#
# Copyright 1999-2002 Wind River Systems, Inc.
# Copyright 1999-2001 Motorola, Inc. All Rights Reserved
#
# modification history
# --------------------
# 01l,18jul02,tfr  Changed RAM_HIGH_ADRS, RAM_LOW_ADRS.
# 01k,22may02,rhe  Changed RELEASE bootrom_uncmp bootrom_uncmp.bin
# 01j,21may02,rhe  Add bootrom_uncmp.bin to RELEASE
# 01i,30apr02,dat  Update for T2.2, removed CPUVARIANT, removed .obj files
#                  removed bootrom_uncmp from RELEASE
# 01h,30oct01,tpw  Standardize kernel links and bootrom conversions.
# 01g,25sep01,dat  Update for T2.2
# 01f,18sep01,hdn  set CPU to PENTIUM3
# 01e,09may00,djs  incorporate WRS review comments.
# 01d,28mar00,scb  added sysRtc to DOC_FILES list
# 01c,22mar00,djs  moved DOC_FILES definition below includes
# 01b,11feb00,djs  removed unsupported images
# 01a,08nov99,djs  written based on cpv5300/Makefile 01c,14oct99,scb
#
# DESCRIPTION
# This file contains rules for building VxWorks for the
# CPN5360.
#
# NOTE
# The constants ROM_TEXT_ADRS and RAM_HIGH_ADRS
# must be identical to those in config.h.
# 
#*/

CPU		= PENTIUM3
TOOL		= gnu

TGT_DIR = $(WIND_BASE)/target

include $(TGT_DIR)/h/make/defs.bsp
#include $(TGT_DIR)/h/make/make.$(CPU)$(TOOL)
#include $(TGT_DIR)/h/make/defs.$(WIND_HOST_TYPE)

# 
# DOC_FILES is defined in h/make/defs.bsp. It is being
# redefined here so that the appropriate html documentation
# is created when a make man command is executed.
#

DOC_FILES  = sysLib tyCoDrv mkboot lm78 failsafe sysRtc \
mkbootFlash5360 dec2155xSrom

## Only redefine make definitions below this point, or your definitions will
## be overwritten by the makefile stubs above.

TARGET_DIR = cpn5360
VENDOR     = Motorola
BOARD      = CPN5360

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

EXTRA_DEFINE       = -DCPN5360

MACH_EXTRA	   = 

RELEASE += bootrom_uncmp.bin bootrom.bin mk.objs


## Only redefine make definitions above this point, or the expansion of 
## makefile target dependencies may be incorrect.

include $(TGT_DIR)/h/make/rules.bsp
#include $(TGT_DIR)/h/make/rules.$(WIND_HOST_TYPE)

# Make .obj files for the released product

mk.objs:
	$(RM) mkboot.obj copyRom.obj mkbootFlash5360.obj
	$(MAKE) mkboot.o copyRom.o mkbootFlash5360.o
	$(MV) mkboot.o mkboot.obj
	$(MV) copyRom.o copyRom.obj
	$(MV) mkbootFlash5360.o mkbootFlash5360.obj

# One of the following bootable vxWorks images should be placed 
# on bootable media by the supplied utilities. See Wind River 
# documentation for further information.
#   vxWorks.st_rom.bin	- bootable vxWorks.st(compressed): upper mem
#   bootrom.bin		- bootrom(compressed):		   lower mem
#   bootrom_uncmp.bin	- bootrom:			   lower mem
