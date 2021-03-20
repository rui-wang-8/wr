/* cpv5000/config.h - Motorola CPV5000/5300/5350 configuration header */

/* Copyright 1984-2002 Wind River Systems, Inc. */
/* Copyright 1998,1999 Motorola, Inc., All Rights Reserved */

/*
modification history
--------------------
02t,18jul02,tfr  Changed RAM_HIGH_ADRS, RAM_LOW_ADRS, LOCAL_MEM_ADRS.
02s,16apr02,dat  Update for T2.2 release, trimmed default configuration so the
		 bootroms would have more space. Updated shared memory config
		 support, removed use of CPU_VARIANT macro. Default console
		 is serial port, not VGA/keyboard.
02r,04dec01,jkf  adding macros for reboot device strings  
02q,27jun00,dat  fixup to clean up REV numbers, and merge from cpv5350
02p,03may00,rcs  added include dec2155xCpci.h
02o,18feb00,scb  Mods for shared memory support enhancement.
02n,01feb00,djs  incorporate WRS review comments.
02m,02nov99,djs  Changed BSP_REV to "/3.3" for cpv5000.
02l,18oct99,scb  Changed BSP_REV to "/0.1" for cpv5300.
02k,14oct99,sjb  Added INCLUDE_SECONDARY_ENET define.
02j,12oct99,scb  Beefed up roll call commentary to explain about slow boot.
02i,04oct99,scb  Fixes to support shared memory.
02h,01sep99,scb  Made INCLUDE_SM_NET #define'ed by default.
02g,23aug99,sjb  Removed INCLUDE_RTC define.
02f,19aug99,scb  warn VIRTUAL_WIRE_MODE or SYMMETRIC_IO_MODE are NOT to be used.
02e,18aug99,sjb  Added Failsafe Timer support.
02d,29jul99,scb  Added LM78 (system monitor) support.
02c,16jul99,sjb  Added INCLUDE_RTC define.
02b,06jun99,scb  Removed conditional compiles for DEFAULT_BOOT_LINE
02a,18jun99,scb  add #define SCSI_AUTO_CONFIG
01z,16jun99,sjb  support for CMOS block read/write on cpv5000
01y,07jun99,scb  Port from T1 to T2.
01x,25may99,scb  fixed INCLUDE_SM_NET to compile #define'ed or #undef'ed.
01w,04may99,scb  Warned that INCLUDE_END must be defined in "config.h".
01v,03may99,scb  Made #defines for PCI prefetch & nonprefetch memory space.
01u,28apr99,scb  Move RAM_LOW_ADRS to prevent for sm pool address conflicts,
                        move anchor offset back to 0x1100.
01t,13apr99,scb  Make roll call the default.
01s,12apr99,scb  Simplify console (vga or com) port selection.
01r,12apr99,scb  Comment out roll call defines.
01q,12apr99,scb  Move SM_TAS_TYPE from "config.h" to "cpv5000.h".
01p,08apr99,scb  Support for shared memory with MCPN750.
01o,07apr99,djs  changed BSP_REV to 3.1
01n,05mar99,djs  changes made porting WRS pciAutoConfigLib
01m,24feb99,djs  Moved LOCAL_MEM_AUTOSIZE to were other related defines
                         are located. Set the minimum LOCAL_MEM_SIZE to 16MB.
01l,11feb99,scb  Merged in WRS CD10 changes.
01k,15mar99,cn   corrected cross-dependencies between INCLUDE_PCI and 
		 INCLUDE_LN_97X_END, INCLUDE_EL_3C90X_END (SPR# 25680).
01j,12mar99,cn   added support for el3c90xEnd driver (SPR# 25327).
01i,08mar99,sbs  added support for SMC Elite Ultra ethernet card.(SPR #25234)
                 moved INCLUDE_LN_97X_END into INCLUDE_END loop.
                 added support for ne2000End driver (SPR #25398)
01h,26feb99,dat	 removed FEI from NETIF_USR_ENTRIES (23818)
01g,25feb99,hdn  added PentiumPro's MESI bus snoop.
		 removed GLOBAL bit from VM_STATE_FOR_MEM_OS.
01f,24feb99,pr   removed CONSOLE_TTY define for PC_CONSOLE (SPR#23075)
01e,01feb99,jkf  added END support for AMD 7997x PCI PCNet-FAST card.
                 made FEI_END the default.
01d,26jan99,jkf  INCLUDE_ADD_BOOTMEM added.  INCLUDE_END
                 is again made the default.  SPR#21338
01c,26nov98,ms_  add elt3c509 END support
01b,12nov98,dat  END drivers are not selected by default (See SPR xxxxx)
01a,06nov98,djs  Created from pc.h (02m,03aug98,cn)
*/

/*
This module contains the configuration parameters for the
CPV5000/CPV5300/CPV5350
*/

#ifndef	INCconfigh
#define	INCconfigh

#ifdef __cplusplus
extern "C" {
#endif

/* BSP version/revision identification, before configAll.h */

#define BSP_VER_1_1	1	/* 1.2 is backward compatible with 1.1 */
#define BSP_VER_1_2	1
#define BSP_VERSION	"1.2"
#define BSP_REV		"/2"	/* T2.2 release */

#include "configAll.h"
#include "sysMotCpci.h"
#include "dec2155xCpci.h"

/* BSP-specific includes */

#include "cpv5000.h"

#define DEFAULT_BOOT_LINE \
	"fd=0,0(0,0)host:/fd0/vxWorks.st h=90.0.0.3 e=90.0.0.50 u=target" 

/* Warm boot (reboot) devices and parameters */

#define SYS_WARM_BIOS 		0 	/* warm start from BIOS */
#define SYS_WARM_FD   		1 	/* warm start from FD */
#define SYS_WARM_ATA  		2	/* warm start from ATA */
#define SYS_WARM_TFFS  		3	/* warm start from DiskOnChip */

#define SYS_WARM_TYPE		SYS_WARM_FD /* warm start device */
#define SYS_WARM_FD_DRIVE       0       /* 0 = drive a:, 1 = b: */
#define SYS_WARM_FD_TYPE        0       /* 0 = 3.5" 2HD, 1 = 5.25" 2HD */
#define SYS_WARM_ATA_CTRL       0       /* controller 0 */
#define SYS_WARM_ATA_DRIVE      0       /* 0 = c:, 1 = d: */
#define SYS_WARM_TFFS_DRIVE     0       /* 0 = c: (DOC) */

/* Warm boot (reboot) device and filename strings */

/* 
 * BOOTROM_DIR is the device name for the device containing
 * the bootrom file. This string is used in sysToMonitor, sysLib.c 
 * in dosFsDevCreate().
 */

#define BOOTROM_DIR  "/vxboot/"

/* 
 * BOOTROM_BIN is the default path and file name to either a binary 
 * bootrom file or an A.OUT file with its 32 byte header stripped.
 * Note that the first part of this string must match BOOTROM_DIR
 * The "bootrom.sys" file name will work with VxLd 1.5.
 */

#define BOOTROM_BIN  "/vxboot/bootrom.sys"

/* 
 * BOOTROM_AOUT is that default path and file name of an A.OUT bootrom
 * _still containing_ its 32byte A.OUT header.   This is legacy code.
 * Note that the first part of this string must match BOOTROM_DIR
 * The "bootrom.dat" file name does not work with VxLd 1.5.
 */

#define BOOTROM_AOUT "/vxboot/bootrom.dat"

/* driver and file system options */

#define	INCLUDE_DOSFS		/* include dosFs file system */
#define	INCLUDE_FD		/* include floppy disk driver */
#undef	INCLUDE_LPT		/* include parallel port driver */
#undef	INCLUDE_ATA		/* include IDE/EIDE(ATA) hard disk driver */
#undef	INCLUDE_TIMESTAMP	/* include TIMESTAMP timer for Wind View */
#undef	INCLUDE_TFFS		/* include TrueFFS driver for Flash */
#undef	INCLUDE_PCMCIA		/* include PCMCIA driver */
#undef	INCLUDE_SCSI		/* include SCSI driver */

#ifdef	INCLUDE_PCMCIA
#define INCLUDE_ATA		/* include ATA driver */
#define INCLUDE_SRAM		/* include SRAM driver */
#undef	INCLUDE_TFFS		/* include TFFS driver */
#endif	/* INCLUDE_PCMCIA */

#ifdef	INCLUDE_TFFS
#define	INCLUDE_SHOW_ROUTINES
#endif	/* INCLUDE_TFFS */

#ifdef  INCLUDE_SCSI            /* include SCSI driver */
#define INCLUDE_SCSI2           /* select SCSI2 not SCSI1 */
#define SCSI_AUTO_CONFIG
#define INCLUDE_AIC_7880        /* include AIC 7880 SCSI driver */
#define INCLUDE_SCSI_BOOT       /* include ability to boot from SCSI */
#undef  INCLUDE_CDROMFS         /* file system to be used */
#undef  INCLUDE_TAPEFS          /* file system to be used */
#endif  /* INCLUDE_SCSI */

#undef INCLUDE_LM78		/* Support for the LM78 system monitor */

#define INCLUDE_DEC2155X_SYSTEM_SUPPORT /* Support for slaves with Dec2155x  */

/* Network driver options */

#define	INCLUDE_END		/* Use Enhanced Network Drivers */

#define	INCLUDE_FEI		/* include Intel Ether Express PRO100B PCI */
#undef INCLUDE_SLIP		/* include serial line interface */
#define SLIP_TTY	1	/* serial line IP channel COM2 */
#define	INCLUDE_SM_COMMON	/* include shared memory */
#define	INCLUDE_SM_NET		/* include backplane net interface */
 
/* verify network choices and dependencies */
 
#ifdef INCLUDE_END
#   undef  WDB_COMM_TYPE
#   define WDB_COMM_TYPE	WDB_COMM_END /* END is prefered choice */

#   ifdef INCLUDE_FEI
#	undef INCLUDE_FEI	/* don't use BSD driver */
#	define INCLUDE_FEI_END	/* Use END driver instead */
#   endif

#   ifdef INCLUDE_FEI_END		  /* Using END driver */
#       if defined(CPV5000)
#           undef INCLUDE_SECONDARY_ENET  /* only 1 ethernet chip on 5000 */
#       elif defined(CPV5300) | defined(CPV5350)
#           undef INCLUDE_SECONDARY_ENET  /* define this for dual enet */
#       endif
#   endif  

#endif

#if defined(INCLUDE_FEI) || defined(INCLUDE_FEI_END)
#   define INCLUDE_PCI          /* they all are PCI based */
#endif
 
#if defined(INCLUDE_SM_NET)
#   define STANDALONE_NET
#   define INCLUDE_NET_SHOW
#   define INCLUDE_BSD

    /*
     * The array fragment below lists Dec2155x-based boards which participate
     * in shared memory backplane networking. The boards are listed by subsystem
     * vendor ID and subsystem ID. Simply add new boards to the list.
     * NOTE: This list only applies to Dec2155x-based boards. There is no
     * equivalent mechanism for other boards since this BSP only contains
     * interrupt drivers for the Dec2155x.
     */

#   ifdef INCLUDE_DEC2155X_SYSTEM_SUPPORT
#       define SYS_SM_DEVICE_LIST \
            SYS_MOT_SM_DEVICE_LIST
#   endif

#   ifndef INCLUDE_SM_COMMON
#	define INCLUDE_SM_COMMON
#   endif

#endif

#if defined(INCLUDE_SM_COMMON)

    /* Shared memory related #define's */

#   define SM_OFF_BOARD FALSE	/* TRUE if anchor off-board, else FALSE */

    /*
     * Finding the shared memory anchor:
     *
     * There are three ways to communicate the location of the anchor to the
     * initialization code:
     *
     * 1) If "sm=xxxxxxxx" is specified as a boot parameter, then "xxxxxxxx"
     *    is used as the local address of the anchor.
     *
     * 2) If case (1) above is not satisfied, then if SM_OFF_BOARD is FALSE,
     *    the address LOCAL_MEM_LOCAL_ADRS + SM_ANCHOR_OFFSET is used as the
     *    local address of the anchor.
     *
     * 3) If neither (1) or (2) above is satisfied (that is "sm=xxxxxxxx" is
     *    NOT specified AND SM_OFF_BOARD is defined as TRUE) then the shared
     *    memory anchor is found via a polling algorithm as described below:
     *
     *    Devices on the compactPCI bus (defined by SYS_SM_CPCI_BUS_NUMBER)
     *    are queried through the first memory BAR.  Memory at offset
     *    SM_ANCHOR_OFFSET is examined to determine if the anchor is there.
     *
     *    If SYS_SM_ANCHOR_POLL_LIST is defined then only those
     *    devices whose device/vendorID and subsystem device/vendorID
     *    are defined in this list are queried.  If SYS_SM_ANCHOR_POLL_LIST
     *    is NOT defined then ALL devices found on SYS_SM_CPCI_BUS_NUMBER
     *    are queried.
     *
     *    When shared memory anchor polling is enabled, SYS_SM_CPCI_BUS_NUMBER
     *    specifies the PCI bus number on which to poll devices for the 
     *    shared memory anchor.
     */

#   undef INCLUDE_SM_SEQ_ADDR	 /* shared memory network auto address setup */

#   ifndef _ASMLANGUAGE
        IMPORT  char * sysSmAnchorAdrs();
        IMPORT  int    smIntArg1;
        IMPORT  int    smIntArg2;
#   endif

#   define SM_INT_TYPE  SM_INT_BUS 	/* cPCI bus interrupt */

#   define SM_INT_ARG1 (smIntArg1)
#   define SM_INT_ARG2 (smIntArg2)
#   define SM_INT_ARG3 NONE

    /*
     * The following defines are only used by the master.
     * The slave only uses the "Anchor" address.
     */

#   define SM_MEM_ADRS          0x00100000
#   define SM_MEM_SIZE          0x00010000
#   define SM_OBJ_MEM_ADRS      (SM_MEM_ADRS+SM_MEM_SIZE) /* SM Objects pool */
#   define SM_OBJ_MEM_SIZE      0x00010000

    /*
     * If the anchor is offboard (SM_OFF_BOARD == TRUE) then place the
     * anchor SM_ANCHOR_OFFSET at 0x4100 if the actual anchor is on an
     * MCP750.
     */

#   undef SM_ANCHOR_ADRS

#   if (SM_OFF_BOARD == TRUE)
#       undef SM_ANCHOR_OFFSET
#       define SM_ANCHOR_OFFSET 0x1100
#       define SM_ANCHOR_ADRS   (sysSmAnchorAdrs())
#   else
#       define SM_ANCHOR_ADRS 	((char *)(LOCAL_MEM_LOCAL_ADRS + \
                                          SM_ANCHOR_OFFSET))
#   endif

/*
 * SYS_SM_ANCHOR_POLL_LIST defines the devices on which to look for a
 * shared memory anchor.
 */

#   if (SM_OFF_BOARD == TRUE)
#      define SYS_SM_ANCHOR_POLL_LIST \
            SYS_MOT_SM_ANCHOR_POLL_LIST
#   endif /* (SM_OFF_BOARD == TRUE */

#endif /* defined(INCLUDE_SM_COMMON) */

/*
 * Note: CPV5x00/MCPN750 requires a modified software Test and Set algorithm.
 * SM_TAS_TYPE is set to SM_TAS_HARD despite the lack of a hardware TAS
 * mechanism to force the use of a BSP-specific software TAS algorithm. The
 * modified algorithm is required to work around a problem encountered with
 * PCI-to-PCI bridges.  Do NOT change SM_TAS_TYPE, it must be set as 
 * SM_TAS_HARD to work.
 */

#undef SM_TAS_TYPE
#define SM_TAS_TYPE     SM_TAS_HARD

/*
 * DEC2155X_SM_DOORBELL_BIT must be between 0 and 15.  It represents the
 * bit number of the primary doorbell register used to interrupt the
 * MCP750 for shared memory bus interrupts.
 */

#define DEC2155X_SM_DOORBELL_BIT 0

/*
 * DEC2155X_CSR_AND_DS_MEM0_SIZE governs the size of memory (in bytes) 
 * residing on shared memory cPCI slaves which is opened up for access 
 * from the CPV5x00, thorugh cPCI backplane accesses in a shared memory 
 * environment.  The lower 4K of this region accesses the MCPN750 CSR 
 * space.  Addresses beyond the 4K base access DRAM on the cPCI slave.
 */

#define DEC2155X_CSR_AND_DS_MEM0_SIZE 0x00400000

/*
 * Dec2155x (Drawbridge) related subsystem vendor ID is currently
 * specified as 'Motorola' (0x1057). For Motorola cPCI slave boards
 * containing the Dec21554 chip, this is the correct value.  It is
 * possible, however that this field can be changed through reprogramming
 * of the Dec21554 SROM on the slave board.
 */

#define DEC2155X_SUB_VNDR_ID_VAL MOT_SUB_VNDR_ID_VAL   /* Vendor = Motorola */

/* Misc. options */

#define INCLUDE_MMU_BASIC 	/* bundled mmu support */
#undef	VM_PAGE_SIZE
#define	VM_PAGE_SIZE		PAGE_SIZE_4KB	/* 4KB page */

#define VM_STATE_MASK_FOR_ALL \
	VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE
#define VM_STATE_FOR_IO \
	VM_STATE_VALID | VM_STATE_WRITABLE | VM_STATE_CACHEABLE_NOT
#define VM_STATE_FOR_MEM_OS \
	VM_STATE_VALID | VM_STATE_WRITABLE | VM_STATE_CACHEABLE
#define VM_STATE_FOR_MEM_APPLICATION \
	VM_STATE_VALID | VM_STATE_WRITABLE | VM_STATE_CACHEABLE
#define VM_STATE_FOR_PCI \
	VM_STATE_VALID | VM_STATE_WRITABLE | VM_STATE_CACHEABLE_NOT

/* 
 * software floating point emulation support. DO NOT undefine hardware fp 
 * support in configAll.h as it is required for software fp emulation.
 */


#define INCLUDE_SW_FP		

#if	(CPU == PENTIUM) /* Pentium specific macros */

#undef	INCLUDE_SW_FP		/* Pentium has hardware FPP */
#undef	USER_D_CACHE_MODE	/* Pentium write-back data cache support */
#define	USER_D_CACHE_MODE	CACHE_COPYBACK
#define	INCLUDE_MTRR_GET	/* get MTRR to sysMtrr[] */
#define	INCLUDE_PMC		/* include PMC */

#else	/* Pentium 2/3/4 specific macros */

#undef	INCLUDE_SW_FP		/* PentiumPro has hardware FPP */
#undef	USER_D_CACHE_MODE	/* PentiumPro write-back data cache w MESI */
#define	USER_D_CACHE_MODE	(CACHE_COPYBACK | CACHE_SNOOP_ENABLE)
#define	INCLUDE_MTRR_GET	/* get MTRR to sysMtrr[] */
#define	INCLUDE_PMC		/* include PMC */

/*
 * Leave VIRTUAL_WIRE_MODE and SYMMETRIC_IO_MODE #undef'ed for all CPV5000,
 *  CPV5300 and CPV5350 boards - these boards have no local APIC.
 */

#undef	VIRTUAL_WIRE_MODE	/* Interrupt Mode: Virtual Wire Mode */
#undef	SYMMETRIC_IO_MODE	/* Interrupt Mode: Symmetric IO Mode */

#if	defined(VIRTUAL_WIRE_MODE) || defined(SYMMETRIC_IO_MODE)
#define	INCLUDE_APIC_TIMER	/* include Local APIC timer */
#define	PIT0_FOR_AUX		/* use channel 0 as an Aux Timer */
#endif	/* defined(VIRTUAL_WIRE_MODE) || defined(SYMMETRIC_IO_MODE) */

#define	INCLUDE_TIMESTAMP_TSC	/* include TSC for timestamp */
#define	PENTIUMPRO_TSC_FREQ	0	  /* auto detect TSC freq */
#if	FALSE
#define	PENTIUMPRO_TSC_FREQ	150000000 /* use specified TSC freq */
#endif	/* FALSE */

#define	INCLUDE_MMU_PENTIUMPRO	/* include 32bit MMU for PentiumPro */
#ifdef	INCLUDE_MMU_PENTIUMPRO

#undef	VM_PAGE_SIZE		/* page size could be 4KB or 4MB */
#define	VM_PAGE_SIZE		PAGE_SIZE_4KB	/* 4KB page */
#if	FALSE
#define	VM_PAGE_SIZE		PAGE_SIZE_4MB	/* 4MB page */
#endif	/* FALSE */

#undef	VM_STATE_MASK_FOR_ALL
#undef	VM_STATE_FOR_IO
#undef	VM_STATE_FOR_MEM_OS
#undef	VM_STATE_FOR_MEM_APPLICATION
#undef	VM_STATE_FOR_PCI
#define VM_STATE_MASK_FOR_ALL \
	VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | \
	VM_STATE_MASK_CACHEABLE | VM_STATE_MASK_WBACK | VM_STATE_MASK_GLOBAL
#define VM_STATE_FOR_IO \
	VM_STATE_VALID | VM_STATE_WRITABLE | \
	VM_STATE_CACHEABLE_NOT | VM_STATE_WBACK_NOT | VM_STATE_GLOBAL_NOT
#define VM_STATE_FOR_MEM_OS \
	VM_STATE_VALID | VM_STATE_WRITABLE | \
	VM_STATE_CACHEABLE | VM_STATE_WBACK | VM_STATE_GLOBAL_NOT
#define VM_STATE_FOR_MEM_APPLICATION \
	VM_STATE_VALID | VM_STATE_WRITABLE | \
	VM_STATE_CACHEABLE | VM_STATE_WBACK | VM_STATE_GLOBAL_NOT
#define VM_STATE_FOR_PCI \
	VM_STATE_VALID | VM_STATE_WRITABLE | \
	VM_STATE_CACHEABLE_NOT | VM_STATE_WBACK_NOT | VM_STATE_GLOBAL_NOT

#endif	/* INCLUDE_MMU_PENTIUMPRO */
#endif	/* (CPU == PENTIUM) */


#define FEI_POOL_ADRS	NONE	/* allocate pool space */

#ifdef  INCLUDE_SLIP
#define SLIP_TTY	1	/* serial line IP channel COM2 */
#define SLIP_BAUDRATE	19200	/* baudrate 19200 */
#endif  /* INCLUDE_SLIP */

/* vectors for PIC(i8259a) and APIC */

#ifdef	SYMMETRIC_IO_MODE

/*
 * Vector number could be any number in between 0x20 to 0xff.
 * Interrupt priority is implied by its vector number, according to
 * the following relationship: 
 *   priority = vectorNo / 16
 * The lowest priority is 1 and 15 is the highest.  To avoid losing
 * interrupts, software should allocate no more than 2 interrupt
 * vectors per priority.  Here is an example to get the vectorNo.
 *   vectorNo(IRQn) = (0xe0 - (n * 8))
 */
#define	NUMBER_OF_IRQS		24
#undef	PIT0_INT_LVL
#define	PIT0_INT_LVL		0x2	/* interrupt level */
#define	INT_NUM_IRQ0		0xe0	/* vector number for IRQ0 */
#define INT_VEC_GET(irq)	(INT_NUM_IRQ0 - (irq * 8))
#define SPURIOUS_INT_VEC	0xff	/* local interrupt */
#define TIMER_INT_VEC		0xf0	/* local interrupt */
#define ERROR_INT_VEC		0xef	/* local interrupt */
#define PIT0_INT_VEC		(INT_VEC_GET (PIT0_INT_LVL))
#define	COM1_INT_VEC		(INT_VEC_GET (COM1_INT_LVL))
#define	COM2_INT_VEC		(INT_VEC_GET (COM2_INT_LVL))
#define RTC_INT_VEC		(INT_VEC_GET (RTC_INT_LVL))
#define FD_INT_VEC		(INT_VEC_GET (FD_INT_LVL))
#define ATA0_INT_VEC 		(INT_VEC_GET (ATA0_INT_LVL))
#define ATA1_INT_VEC		(INT_VEC_GET (ATA1_INT_LVL))
#define PCIC_INT_VEC		(INT_VEC_GET (PCIC_INT_LVL))
#define TCIC_INT_VEC		(INT_VEC_GET (TCIC_INT_LVL))
#define ELT0_INT_VEC		(INT_VEC_GET (ELT0_INT_LVL))
#define ELT1_INT_VEC		(INT_VEC_GET (ELT1_INT_LVL))
#define LPT_INT_VEC		(INT_VEC_GET (LPT_INT_LVL))
#define FEI0_INT_VEC		(INT_VEC_GET (FEI0_INT_LVL))
#define FEI1_INT_VEC		(INT_VEC_GET (FEI1_INT_LVL))
#define FEI2_INT_VEC		(INT_VEC_GET (FEI2_INT_LVL))
#define FEI3_INT_VEC		(INT_VEC_GET (FEI3_INT_LVL))
#define KBD_INT_VEC		(INT_VEC_GET (KBD_INT_LVL))

#else	/* SYMMETRIC_IO_MODE */

/* 
 * Vector number is not flexable as APIC and obtained by:
 *   vectorNo(IRQn) = vectorNo(IRQ0) + n
 */
#define	NUMBER_OF_IRQS		16
#define INT_NUM_IRQ0		0x20	/* vector number for IRQ0 */
#define INT_VEC_GET(irq)	(INT_NUM_IRQ0 + irq)
#define SPURIOUS_INT_VEC	0xff	/* local interrupt */
#define TIMER_INT_VEC		0xf0	/* local interrupt */
#define ERROR_INT_VEC		0xef	/* local interrupt */
#define PIT0_INT_VEC		(INT_VEC_GET (PIT0_INT_LVL))
#define	COM1_INT_VEC		(INT_VEC_GET (COM1_INT_LVL))
#define	COM2_INT_VEC		(INT_VEC_GET (COM2_INT_LVL))
#define RTC_INT_VEC		(INT_VEC_GET (RTC_INT_LVL))
#define FD_INT_VEC		(INT_VEC_GET (FD_INT_LVL))
#define ATA0_INT_VEC 		(INT_VEC_GET (ATA0_INT_LVL))
#define ATA1_INT_VEC		(INT_VEC_GET (ATA1_INT_LVL))
#define PCIC_INT_VEC		(INT_VEC_GET (PCIC_INT_LVL))
#define TCIC_INT_VEC		(INT_VEC_GET (TCIC_INT_LVL))
#define ELT0_INT_VEC		(INT_VEC_GET (ELT0_INT_LVL))
#define ELT1_INT_VEC		(INT_VEC_GET (ELT1_INT_LVL))
#define LPT_INT_VEC		(INT_VEC_GET (LPT_INT_LVL))
#define FEI0_INT_VEC		(INT_VEC_GET (FEI0_INT_LVL))
#define FEI1_INT_VEC		(INT_VEC_GET (FEI1_INT_LVL))
#define FEI2_INT_VEC		(INT_VEC_GET (FEI2_INT_LVL))
#define FEI3_INT_VEC		(INT_VEC_GET (FEI3_INT_LVL))
#define KBD_INT_VEC		(INT_VEC_GET (KBD_INT_LVL))

#endif	/* SYMMETRIC_IO_MODE */


/* 
 * SYS_CLK_RATE_MAX depends upon a CPU power and a work load of an application.
 * The value was chosen in order to pass the internal test suit,
 * but it could go up to PIT_CLOCK.
 */
#define SYS_CLK_RATE_MIN  19            /* minimum system clock rate */
#define SYS_CLK_RATE_MAX  (PIT_CLOCK/256) /* maximum system clock rate */
#define AUX_CLK_RATE_MIN  2             /* minimum auxiliary clock rate */
#define AUX_CLK_RATE_MAX  8192          /* maximum auxiliary clock rate */

/*
 * pc console definitions  -
 * #define INCLUDE_PC_CONSOLE to use VGA as console output,
 * #undef INCLUDE_PC_CONSOLE to use one of the COM ports as output,
 * select the desired COM port as shown below:
 *     #define CONSOLE_TTY as 0 to use COM1 as output.
 *     #define CONSOLE_TTY as 1 to use COM2 as output.
 */

#undef INCLUDE_PC_CONSOLE              /*  define to use VGA */

#ifdef INCLUDE_PC_CONSOLE

#define PC_CONSOLE              0       /* console number */
#define N_VIRTUAL_CONSOLES      2       /* shell / application */
#undef  CONSOLE_TTY
#define CONSOLE_TTY             -1

#else
#undef  CONSOLE_TTY
#define CONSOLE_TTY     0

#endif /* INCLUDE_PC_CONSOLE */

#undef	NUM_TTY
#define NUM_TTY			(N_UART_CHANNELS)

/* define a type of keyboard. The default is 101 KEY for PS/2 */

#define PC_KBD_TYPE		PC_PS2_101_KBD
#if	FALSE
#define PC_KBD_TYPE		PC_XT_83_KBD
#endif	/* FALSE */

/*
 * Defining INCLUDE_PCI_AUTOCONF, causes the VxWorks bootroms to use
 * the pciAutoConfigLib to enumerate PCI devices. This is
 * the recommended default setting. Undef'ing
 * INCLUDE_PCI_AUTOCONF, causes the BIOS PCI configuration
 * settings to be used by VxWorks.
 */

#define INCLUDE_PCI_AUTOCONF

#ifndef PCI_CFG_TYPE
#    ifdef INCLUDE_PCI_AUTOCONF
#        define PCI_CFG_TYPE PCI_CFG_AUTO
#    else
#        define PCI_CFG_TYPE PCI_CFG_NONE
#    endif
#endif

/* memory addresses */

/* User reserved memory.  See sysMemTop(). */

#define	USER_RESERVED_MEM	0

/*
 * PCI prefetch and noprefetch memory sizes are applicable only
 * when INCLUDE_PCI_AUTOCONF is #define'd.  These default sizes 
 * reflect on-board requirements plus about 16MB.  They can be 
 * adjusted upward to support more devices plugged into the compact 
 * PCI chassis.  PCI_MSTR_MEMIO_SIZE defines the maximum amount of 
 * PCI nonprefetchable memory, PCI_MSTR_MEM_SIZE defines the maximum 
 * amount of PCI prefetchable memory.  When INCLUDE_PCI_AUTOCONF
 * is #undef'ed, the BIOS (not vxWorks) configures the devices, PCI
 * memory space is dynamically allocated and these two defines have 
 * no effect.
 */

#define PCI_MSTR_MEMIO_SIZE		0x01400000
#define PCI_MSTR_MEM_SIZE		0x01300000

/*
 * Local-to-Bus memory address constants:
 * the local memory address always appears at 0 locally;
 * it is not dual ported.
 */

#define LOCAL_MEM_LOCAL_ADRS	0x00100000	/* fixed */
#define LOCAL_MEM_SIZE		0x00800000	/* 8MB w lower mem */

/*
 * Auto-sizing of memory is supported when this option is defined, in which
 * case LOCAL_MEM_SIZE is ignored.  See sysyPhysMemTop().
 */

#define	LOCAL_MEM_AUTOSIZE

/*
 * The following parameters are defined here and in the Makefile.
 * The must be kept synchronized; effectively config.h depends on Makefile.
 * Any changes made here must be made in the Makefile and vice versa.
 */

#ifdef	BOOTCODE_IN_RAM
#undef ROMSTART_BOOT_CLEAR
#define ROM_BASE_ADRS		(0x00008000)	/* base address of ROM */
#define ROM_TEXT_ADRS		(ROM_BASE_ADRS)	/* booting from A: or C: */
#define ROM_SIZE		(0x00090000)	/* size of ROM */
#else
#define ROM_BASE_ADRS		(0xfff20000)	/* base address of ROM */
#define ROM_TEXT_ADRS		(ROM_BASE_ADRS)	/* booting from EPROM */
#define ROM_SIZE		(0x0007fe00)	/* size of ROM */
#endif

#define RAM_LOW_ADRS		(0x00308000)	/* VxWorks image entry point */
#define RAM_HIGH_ADRS		(0x00108000)	/* Boot image entry point */

/*
 * PCI autoconfiguration "roll call" list:
 * Each entry in the "roll call" list defined below, consists of two
 * separate components: a "count" component followed by a PCI header
 * "vendor/ID value" component.  During the first phase of PCI
 * autoconfiguration, the "enumeration" phase, the PCI busses are
 * dynamically probed and a list of devices actually found is
 * constructed.  After the enumeration pass is complete, the
 * dynamically created list of actual devices is compared against the
 * "roll call" list.  For each defined "device/vendor ID" in the roll
 * call list, the corresponding entry in the dynamically created
 * enumeration list is queried.  If the dynamic list indicates fewer
 * devices of the desired type than the "roll call" list does, the PCI
 * enumeration pass is performed again and a new dynamic list is
 * created.  This process will continue until the roll call list is
 * actually satisfied by devices found during dynamic enumeration or
 * until a timeout iteration value (specified by ROLL_CALL_MAX_DURATION) 
 * is exceeded.  After the enumeration phase is completed (by satisfying
 * the "roll call" list or by exceeding the roll call iteration count), 
 * the second phase of PCI autoconfiguration (address assignment) is
 * performed.
 *
 * The roll call list is useful for holding off autoconfiguration while
 * devices make themselves visible on the cPCI bus.
 *
 * ROLL_CALL_MAX_DURATION defines the maximum number of PCI bus enumeration
 * iterations which will run before PCI configuration proceeds.  That is, 
 * even if the roll call repeatedly fails, the configuration  will proceed 
 * anyway after ROLL_CALL_MAX_DURATION number of iterations is completed.
 *
 * To eliminate roll call checking altogether, simply make sure that
 * PCI_ROLL_CALL_LIST_ENTRIES is not defined.
 *
 * There is a one-second delay between enumeration iterations.  In addition,
 * the enumeration process itself can take several seconds.  You can decrease
 * the time required for booting by eliminating roll call checking altogether.
 * Be aware however, that if a compact PCI device is slow to make itself
 * seen on the compact PCI bus that it may be possible for that device to
 * escape the PCI bus configuration process altogether.  If this is the 
 * case, enabling roll call checking will be required for proper bus 
 * enumeration and configuration.
 *
 * The example below, sets up a roll call list to specify seven
 * Dec21554 devices (seven MCPN750 boards) with a roll call
 * timeout value of 2 seconds.
 *
 */

#define DEC21554_COUNT 7

#define PCI_ROLL_CALL_LIST_ENTRIES \
    { DEC21554_COUNT, PCI_ID_DEV_DEC21554, PCI_ID_VEND_DEC21554 },

#ifdef PCI_ROLL_CALL_LIST_ENTRIES
#   define ROLL_CALL_MAX_DURATION 2
#endif

/*
 * The macro INCLUDE_ADD_BOOTMEM enables sysHwInit2,sysLib.c code which 
 * adds some upper memory (>1MB) to the bootrom image memory pool.  
 * It should not be defined for x86 systems with limited memory,< 4MB.  
 * The default value for ADDED_BOOTMEM_SIZE is 0x00200000 (2MB).
 * This value may be increased, but one must ensure that the pool 
 * does not overlap with the downloaded vxWorks image.  If there is 
 * an overlap, then loading the vxWorks runtime image will corrupt 
 * the added memory pool.   The calculation is:
 * (RAM_LOW_ADRS + vxWorks image size) < (memTopPhys - ADDED_BOOTMEM_SIZE)
 * Where: RAM_LOW_ADRS == 0x00108000 and memTopPhys is from sysLib.c.
 * This corrects SPR#21338.
 */
 
#define INCLUDE_ADD_BOOTMEM  /* Add upper memory to low memory bootrom */
#define ADDED_BOOTMEM_SIZE   0x00200000   /* 2MB */

/* define the following to include failsafe WatchDog Timer routines. */ 

#undef INCLUDE_FAILSAFE 

/* define the following to include RTC routines. */ 

#undef INCLUDE_RTC 

#ifdef __cplusplus
}
#endif

#endif	/* INCconfigh */

#if defined(PRJ_BUILD)
#include "prjParams.h"
#endif
