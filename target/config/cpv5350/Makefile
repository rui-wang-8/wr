# Makefile - makefile for target/config/cpv5350
#
# Copyright 1984-2002 Wind River Systems, Inc.
# Copyright 2000 Motorola, Inc. All Rights Reserved
#
# modification history
# --------------------
# 01g,18jul02,tfr  Changed RAM_HIGH_ADRS, RAM_LOW_ADRS.
# 01f,20may02,rhe  Add bootrom.bin to RELEASE
# 01e,17apr02,dat  Update for T2.2 release, removed CPU_VARIANT, removed
#		   special fei and smUtil obj's.
# 01d,30oct01,tpw  Standardize kernel links and bootrom conversions.
# 01c,25sep01,dat  Update for T2.2
# 01b,18sep01,hdn  set CPU to PENTIUM3
# 01a,22may00,djs  written based on cpv5300 Makefile 01f,01feb00.
#
# DESCRIPTION
# This file contains rules for building VxWorks for the
# CPV5350.
#
# NOTE
# The constants ROM_TEXT_ADRS and RAM_HIGH_ADRS
# must be identical to those in config.h.
# 
# INCLUDES
#     makeTarget
#*/

CPU		= PENTIUM3
TOOL		= gnu

TGT_DIR = $(WIND_BASE)/target

include $(TGT_DIR)/h/make/defs.bsp
#include $(TGT_DIR)/h/make/make.$(CPU)$(TOOL)
#include $(TGT_DIR)/h/make/defs.$(WIND_HOST_TYPE)

## Only redefine make definitions below this point, or your definitions will
## be overwritten by the makefile stubs above.

DOC_FILES  = sysLib tyCoDrv sysTffs mkboot lm78 failsafe sysRtc
TARGET_DIR = cpv5350
VENDOR     = Motorola
BOARD      = CPV5350

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

EXTRA_DEFINE       =  -DCPV5350

MACH_EXTRA	   = 

RELEASE += bootrom_uncmp.bin bootrom.bin mkboot.o


## Only redefine make definitions above this point, or the expansion of 
## makefile target dependencies may be incorrect.

include $(TGT_DIR)/h/make/rules.bsp
#include $(TGT_DIR)/h/make/rules.$(WIND_HOST_TYPE)

# One of the following bootable vxWorks images should be placed
# on bootable media by the supplied utilities. See Wind River
# documentation for further information.
#   vxWorks.st_rom      - bootable vxWorks.st(compressed): upper mem
#   bootrom             - bootrom(compressed):             lower mem
#   bootrom_uncmp       - bootrom:                         lower mem
