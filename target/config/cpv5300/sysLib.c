/* sysLib.c - CPV5xxx system-dependent library */

/* Copyright 1984-2002 Wind River Systems, Inc. */
/* Copyright 1999-2000 Motorola, Inc. All Rights Reserved */
#include "copyright_wrs.h"

/*
modification history
--------------------
02o,18jul02,tfr  Removed extra code for sysSmAnchorFind().
02n,21jun02,tfr  added fix to SPR 78395 -- fixes sysSmAnchorFind() for
                 VxMP
02m,16may02,hdn  added MMU component check in the GDT limit adjusting
02l,16apr02,dat  Update for T2.2 release, new CPU_ID struct, new shared
		 memory support macro names, removed CPU_VARIANT macro,
		 updated sysToMonitor code.
02k,03dec01,jkf  sysToMonitor: checks if device exists before creation,
                 and using new macros for device and file name strings.
02j,27jun00,dat  using generic byteNvRam 
02i,22may00,djs  Add support items for the CPV5350.
02h,22feb00,scb  Fix unreliable system reset/reboot in sysToMonitor().
02g,18feb00,scb  Mods for shared memory support enhancement.
02f,01feb00,djs  incorporate WRS review comments.
02e,22oct99,scb  Enable instruction cache in sysHwInit.
02d,13oct99,scb  IDE configured generically instead of with special routine.
02c,04oct99,scb  Fixes relatated to shared memory support.
02b,23aug99,sjb  Removed Real Time Clock Support 
02a,19aug99,scb  Reworked sysModel() to discriminate between P6 processors.
01z,18aug99,sjb  Added Failsafe Timer support.
01y,29jul99,scb  Added LM78 (system monitor) support.
01x,19jul99,sjb  Added Real Time Clock Support for CPV5000.
01w,02jul99,djs  additional changes to sysModel for CPV5000/CPV5300
01v,24jun99,scb  Minor mods to get printer (parallel port) to work.
01u,24jun99,scb  Add sysMsDelay() (missed in port from T1 to T2).
01t,16jun99,sjb  Support for NVRAM CPV5000
01s,10jun99,scb  Move pciAutoConfig() from sysHwInit() to sysHwInit2(),
		   fixed PCI window naming.
01r,07jun99,scb  Port from T1 to T2.
01q,25may99,scb  fixed INCLUDE_SM_NET to compile #define'ed or #undef'ed.
01p,07may99,djs  changed ATA_PCMCIA to IDE_LOCAL.
01o,03may99,scb  remove delay between secondary bus reset and PCI autoconfig.
01n,28apr99,djs  added back logic in sysMemTop.
01m,21apr99,hdn  added conditional tffsDrv.h inclusion (SPR# 26922)
01l,13apr99,djs  add call to sysIdePciInit()
01k,08apr99,scb  Support for shared memory CPV5000/MCPN750.
01j,31mar99,djs  modified sysModel for CPV5000/CPV5300
01i,26mar99,djs  removed the writing of the PCIRQ's.
01h,24mar99,djs  changes made to only run pciAutoConfig code from ROM image
01g,23mar99,djs  changes made to reset secondary PCI bus
01f,12mar99,cn   added support for el3c90xEnd driver (SPR# 25327).
01e,09mar99,sbs  added support for ne2000End driver
                 added support for ultraEnd driver
                 added support for elt3c509End driver
01d,05mar99,djs  changes made porting WRS pciAutoConfigLib
01c,23feb99,djs  changed sysPhysMemTop() to return the correct memTop value.
                 implemented the non-autosizing capability.
01b,22feb99,djs  give the boot floppy application more DRAM to operate in.
                 Modified sysMemTop()
01a,22feb99,djs  written based on pcPentium version.
*/

/*
DESCRIPTION
This library provides board-specific routines.  The chip drivers included are:

    i8250Sio.c - Intel 8250 UART tty driver
    i8253Timer.c - Intel 8253 timer driver
    i8259Intr.c - Intel 8259 Programmable Interrupt Controller (PIC) library
    byteNvRam.c - CMOS NVRAM library
    nullVme.c - null VMEbus library
    if_eex32.c - Intel Ether Express (EISA) Ethernet network interface driver
    pciConfigLib.c - PCI configuration space access support for PCI drivers
    pciIntLib.c - PCI shared interrupt support
    pciConfigShow.c - Show routines for PCI configuration library
    if_fei.c - Intel 82557 Ethernet network interface driver
    aic7880Lib.c - Adaptec 7880 SCSI Host Adapter Library
    sysEl3c90xEnd.c -  system configuration module for el3c90xEnd driver
    

INCLUDE FILES: sysLib.h

SEE ALSO:
.pG "Configuration"
*/

/* includes */

#include "vxWorks.h"
#include "vme.h"
#include "memLib.h"
#include "sysLib.h"
#include "string.h"
#include "intLib.h"
#include "config.h"
#include "logLib.h"
#include "taskLib.h"
#include "vxLib.h"
#include "errnoLib.h"
#include "dosFsLib.h"
#include "stdio.h"
#include "cacheLib.h"
#include "private/vmLibP.h"
#include "arch/i86/pentiumLib.h"
#include "pci.h"
#include "drv/pci/pciIntLib.h"
#include "netinet/in.h"		/* ntohl() definition */
#include "cpv5000.h"
#include "dec2155xCpci.h"    
#ifdef	INCLUDE_TFFS
#include "tffs/tffsDrv.h"
#endif	/* INCLUDE_TFFS */
#include "rebootLib.h"
#include "cpuid.h"

/* defines */

#define sysPciInByte(addr) (*(UINT8 *)(addr))
#define sysPciInWord(addr) (*(UINT16 *)(addr))
#define sysPciInLong(addr) (*(UINT32 *)(addr))
#define sysPciOutByte(addr,val) ( (*(UINT8 *)(addr)) = (UINT8)(val) )
#define sysPciOutWord(addr,val) ( (*(UINT16 *)(addr)) = (UINT16)(val) )
#define sysPciOutLong(addr,val) ( (*(UINT32 *)(addr)) = (UINT32)(val) )

#define DEFAULT_TAS_CHECKS      10              /* rechecks for soft tas */
#define TAS_CONST               0x80

/* To make man pages for network support routines */

#ifdef  DOC
#define INCLUDE_FEI
#define INCLUDE_FEI_END
#endif

/* imports */

IMPORT char end;		  /* end of system, created by ld */
IMPORT GDT sysGdt[];		  /* the global descriptor table */
IMPORT VOIDFUNCPTR intEOI;	  /* pointer to a function sysIntEOI() */
IMPORT VOIDFUNCPTR intVecSetEnt;  /* entry hook for intVecSet() */
IMPORT VOIDFUNCPTR intVecSetExit; /* exit  hook for intVecSet() */
IMPORT void elcdetach (int unit);
IMPORT void ultradetach (int unit);
IMPORT VOIDFUNCPTR intEoiGet;     /* function used in intConnect() for B/EOI */
IMPORT void intEnt (void);
IMPORT UINT sysCpuProbe(void);
IMPORT ULONG    tickGet ();
IMPORT void failsafeRebootHook (int ignored); /* reboot hook, failsafe timer */
STATUS sysMmuMapAdd(void *address, UINT len, UINT initialStateMask,
                    UINT initialState);

void    sysPciOutWordConfirm (UINT32, UINT16);
void	sysNanoDelay (UINT32);
void	sysMsDelay (UINT32);
UCHAR	sysNvRead(ULONG);
void	sysNvWrite(ULONG,UCHAR);

#ifdef INCLUDE_SM_COMMON
    LOCAL void	 sysSmParamsCompute (void);

#   if (SM_OFF_BOARD == TRUE)
#	ifdef SYS_SM_ANCHOR_POLL_LIST
	    LOCAL UINT	 sysSmAnchorCandidate ( UINT, UINT, UINT);
#	endif
	LOCAL STATUS sysSmAnchorFind ( int, char **);
	LOCAL char   *sysSmAnchorPoll (void);
	char *	sysSmAnchorAdrs ();	/* Anchor address (dynamic) */
#   endif
#endif

#ifdef INCLUDE_DEC2155X_SYSTEM_SUPPORT

#if (PCI_CFG_TYPE == PCI_CFG_NONE)
    LOCAL void   sysDec2155xMemMap (void);
#endif
    LOCAL UINT   sysSmDeviceParticipant (UINT, UINT, UINT);
    LOCAL void   sysDec2155xIntConnect (void);
    LOCAL void   sysDec2155xIntr (UINT32);
    LOCAL STATUS sysDec2155xIntEnable (int);
    LOCAL STATUS sysDec2155xIntDisable (int);

#endif /* INCLUDE_DEC2155X_SYSTEM_SUPPORT */

#ifdef INCLUDE_PCI
    
    LOCAL void sysPciDevsInit (void);

#endif /* INCLUDE_PCI */

/* globals */

PHYS_MEM_DESC sysPhysMemDesc [] =
    {
    /* adrs and length parameters must be page-aligned (multiples of 4KB/4MB) */

#if	(VM_PAGE_SIZE == PAGE_SIZE_4KB)

    /* lower memory */
    {
    (void *) LOCAL_MEM_LOCAL_ADRS,
    (void *) LOCAL_MEM_LOCAL_ADRS,
    0xa0000,
    VM_STATE_MASK_FOR_ALL,
    VM_STATE_FOR_MEM_OS
    },

    /* video ram, etc */
    {
    (void *) 0xa0000,
    (void *) 0xa0000,
    0x60000,
    VM_STATE_MASK_FOR_ALL,
    VM_STATE_FOR_IO
    },

    /* upper memory for OS */
    {
    (void *) 0x100000,
    (void *) 0x100000,
    0x080000,
    VM_STATE_MASK_FOR_ALL,
    VM_STATE_FOR_MEM_OS
    },

    /* upper memory for Application */
    {
    (void *) 0x180000,
    (void *) 0x180000,
    LOCAL_MEM_SIZE - 0x180000,	/* it is changed in sysMemTop() */
    VM_STATE_MASK_FOR_ALL,
    VM_STATE_FOR_MEM_APPLICATION
    },

#if (PCI_CFG_TYPE == PCI_CFG_AUTO)

    /* 32-bit Non-prefetchable Memory Space */
    {
    (void *) PCI_MSTR_MEMIO_LOCAL,
    (void *) PCI_MSTR_MEMIO_LOCAL,
    PCI_MSTR_MEMIO_SIZE,	
    VM_STATE_MASK_FOR_ALL,
    VM_STATE_FOR_PCI
    },

    /* 32-bit Prefetchable Memory Space */
    {
    (void *) PCI_MSTR_MEM_LOCAL,
    (void *) PCI_MSTR_MEM_LOCAL,
    PCI_MSTR_MEM_SIZE,	
    VM_STATE_MASK_FOR_ALL,
    VM_STATE_FOR_PCI
    },
#endif

    /* entries for dynamic mappings - create sufficient entries */

    DUMMY_MMU_ENTRY,
    DUMMY_MMU_ENTRY,
    DUMMY_MMU_ENTRY,
    DUMMY_MMU_ENTRY,
    DUMMY_MMU_ENTRY,
    DUMMY_MMU_ENTRY,
    DUMMY_MMU_ENTRY,
    DUMMY_MMU_ENTRY,
    DUMMY_MMU_ENTRY,
    DUMMY_MMU_ENTRY,

#else	/* (VM_PAGE_SIZE == PAGE_SIZE_4KB) */

    /* 1st 4MB: lower memory + video ram etc + upper memory for OS */
    {
    (void *) LOCAL_MEM_LOCAL_ADRS,
    (void *) LOCAL_MEM_LOCAL_ADRS,
    0x400000,
    VM_STATE_MASK_FOR_ALL,
    VM_STATE_FOR_MEM_OS
    },

    /* 2nd 4MB: upper memory for Application */
    {
    (void *) 0x400000,
    (void *) 0x400000,
    LOCAL_MEM_SIZE - 0x400000,	/* it is changed in sysMemTop() */
    VM_STATE_MASK_FOR_ALL,
    VM_STATE_FOR_MEM_APPLICATION
    },

    /* entries for dynamic mappings - create sufficient entries */

    DUMMY_MMU_ENTRY,
    DUMMY_MMU_ENTRY,
    DUMMY_MMU_ENTRY,
    DUMMY_MMU_ENTRY,
    DUMMY_MMU_ENTRY,
    DUMMY_MMU_ENTRY,
    DUMMY_MMU_ENTRY,
    DUMMY_MMU_ENTRY,
    DUMMY_MMU_ENTRY,
    DUMMY_MMU_ENTRY,

#endif	/* (VM_PAGE_SIZE == PAGE_SIZE_4KB) */
    };

int sysPhysMemDescNumEnt; 	/* number Mmu entries to be mapped */

#ifdef	INCLUDE_PC_CONSOLE

PC_CON_DEV	pcConDv [N_VIRTUAL_CONSOLES] = 
    {
    {{{{NULL}}}, FALSE, NULL, NULL}, 
    {{{{NULL}}}, FALSE, NULL, NULL}
    };

#endif	/* INCLUDE_PC_CONSOLE */

#ifdef INCLUDE_FD

IMPORT STATUS usrFdConfig (int type, int drive, char *fileName);
FD_TYPE fdTypes[] =
    {
    {2880,18,2,80,2,0x1b,0x54,0x00,0x0c,0x0f,0x02,1,1,"1.44M"},
    {2400,15,2,80,2,0x24,0x50,0x00,0x0d,0x0f,0x02,1,1,"1.2M"},
    };
UINT    sysFdBuf     = FD_DMA_BUF_ADDR;	/* floppy disk DMA buffer address */
UINT    sysFdBufSize = FD_DMA_BUF_SIZE;	/* floppy disk DMA buffer size */

#endif	/* INCLUDE_FD */

#ifdef	INCLUDE_ATA

IMPORT STATUS usrAtaConfig (int ctrl, int drive, char *fileName);
ATA_TYPE ataTypes[ATA_MAX_CTRLS][ATA_MAX_DRIVES] =
    {
    {{761, 8, 39, 512, 0xff},		/* ctrl 0 drive 0 */
     {761, 8, 39, 512, 0xff}},		/* ctrl 0 drive 1 */
    {{761, 8, 39, 512, 0xff},		/* ctrl 1 drive 0 */
     {761, 8, 39, 512, 0xff}},		/* ctrl 1 drive 1 */
    };

ATA_RESOURCE ataResources[ATA_MAX_CTRLS] =
    {
    {
     {
     5, 0,
     {ATA0_IO_START0, ATA0_IO_START1}, {ATA0_IO_STOP0, ATA0_IO_STOP1}, 0,
     0, 0, 0, 0, 0
     },
     IDE_LOCAL, 1, ATA0_INT_VEC, ATA0_INT_LVL, ATA0_CONFIG,
     ATA_SEM_TIMEOUT, ATA_WDG_TIMEOUT, 0, 0
    },	/* ctrl 0 */
    {
     {
     5, 0,
     {ATA1_IO_START0, ATA1_IO_START1}, {ATA1_IO_STOP0, ATA1_IO_STOP1}, 0,
     0, 0, 0, 0, 0
     },
     ATA_PCMCIA, 1, ATA1_INT_VEC, ATA1_INT_LVL, ATA1_CONFIG,
     ATA_SEM_TIMEOUT, ATA_WDG_TIMEOUT, 0, 0
    }	/* ctrl 1 */
    };

#endif	/* INCLUDE_ATA */

#ifdef	INCLUDE_LPT

LPT_RESOURCE lptResources[LPT_CHANNELS] = 
    {
     {LPT1_BASE_ADRS, LPT_INT_VEC, LPT_INT_LVL, FALSE, 10000, 10000, 1, 1},
    };

#endif	/* INCLUDE_LPT */

int	sysBus		= BUS;		/* system bus type (VME_BUS, etc) */
int	sysCpu		= CPU;		/* system cpu type (MC680x0) */
char	*sysBootLine	= BOOT_LINE_ADRS; /* address of boot line */
char	*sysExcMsg	= EXC_MSG_ADRS;	/* catastrophic message area */
int	sysProcNum;			/* processor number of this cpu */
int	sysFlags;			/* boot flags */
char	sysBootHost [BOOT_FIELD_LEN];	/* name of host from which we booted */
char	sysBootFile [BOOT_FIELD_LEN];	/* name of file from which we booted */
UINT	sysIntIdtType	= SYS_INT_TRAPGATE; /* IDT entry type */
UINT	sysProcessor	= NONE;		/* 0=386, 1=486, 2=P5, 3=ns486, 4=P6 */
UINT	sysCoprocessor	= 0;		/* 0=none, 1=387, 2=487 */
int 	sysWarmType	= SYS_WARM_TYPE;      /* system warm boot type */
int	sysWarmFdDrive	= SYS_WARM_FD_DRIVE;  /* 0 = drive a:, 1 = b: */
int	sysWarmFdType	= SYS_WARM_FD_TYPE;   /* 0 = 3.5" 2HD, 1 = 5.25" 2HD */
int	sysWarmAtaCtrl	= SYS_WARM_ATA_CTRL;  /* controller 0 or 1 */
int	sysWarmAtaDrive	= SYS_WARM_ATA_DRIVE; /* Hd drive 0 (c:), 1 (d:) */
int	sysWarmTffsDrive= SYS_WARM_TFFS_DRIVE; /* TFFS drive 0 (DOC) */
UINT	sysStrayIntCount = 0;		/* Stray interrupt count */
char	*memTopPhys	= NULL;		/* top of memory */
GDT	*pSysGdt	= (GDT *)(LOCAL_MEM_LOCAL_ADRS + GDT_BASE_OFFSET);
CPUID	sysCpuId	= {0,{0},0,0,0,0,0,0,0,0,{0},{0}}; /* CPUID struct. */
BOOL	sysBp		= TRUE;		/* TRUE for BP, FALSE for AP */

int    smIntArg1 = -1;                          /* Shared memory SM_INT_ARG1 */
int    smIntArg2 = -1;                          /* Shared memory SM_INT_ARG2 */

/* locals */

LOCAL short *sysRomBase[] = 
    {
    (short *)0xce000, (short *)0xce800, (short *)0xcf000, (short *)0xcf800
    };
#define ROM_SIGNATURE_SIZE	16
LOCAL char sysRomSignature[ROM_SIGNATURE_SIZE] = 
    {
    0x55,0xaa,0x01,0x90,0x90,0x90,0x90,0x90,
    0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90
    };

#if	(CPU == PENTIUM)
    /*
     * The cache control flags and MTRRs operate hierarchically for restricting
     * caching.  That is, if the CD flag is set, caching is prevented globally.
     * If the CD flag is clear, either the PCD flags and/or the MTRRs can be
     * used to restrict caching.  If there is an overlap of page-level caching
     * control and MTRR caching control, the mechanism that prevents caching
     * has precedence.  For example, if an MTRR makes a region of system memory
     * uncachable, a PCD flag cannot be used to enable caching for a page in 
     * that region.  The converse is also true; that is, if the PCD flag is 
     * set, an MTRR cannot be used to make a region of system memory cacheable.
     * If there is an overlap in the assignment of the write-back and write-
     * through caching policies to a page and a region of memory, the write-
     * through policy takes precedence.  The write-combining policy takes
     * precedence over either write-through or write-back.
     */ 
LOCAL MTRR sysMtrr =
    { 					/* MTRR table */
    {0,0},				/* MTRR_CAP register */
    {0,0},				/* MTRR_DEFTYPE register */
    					/* Fixed Range MTRRs */
    {{{MTRR_WB, MTRR_WB, MTRR_WB, MTRR_WB, MTRR_WB, MTRR_WB, MTRR_WB, MTRR_WB}},
     {{MTRR_WB, MTRR_WB, MTRR_WB, MTRR_WB, MTRR_WB, MTRR_WB, MTRR_WB, MTRR_WB}},
     {{MTRR_UC, MTRR_UC, MTRR_UC, MTRR_UC, MTRR_WC, MTRR_WC, MTRR_WC, MTRR_WC}},
     {{MTRR_WP, MTRR_WP, MTRR_WP, MTRR_WP, MTRR_WP, MTRR_WP, MTRR_WP, MTRR_WP}},
     {{MTRR_UC, MTRR_UC, MTRR_UC, MTRR_UC, MTRR_UC, MTRR_UC, MTRR_UC, MTRR_UC}},
     {{MTRR_UC, MTRR_UC, MTRR_UC, MTRR_UC, MTRR_UC, MTRR_UC, MTRR_UC, MTRR_UC}},
     {{MTRR_UC, MTRR_UC, MTRR_UC, MTRR_UC, MTRR_UC, MTRR_UC, MTRR_UC, MTRR_UC}},
     {{MTRR_UC, MTRR_UC, MTRR_UC, MTRR_UC, MTRR_UC, MTRR_UC, MTRR_UC, MTRR_UC}},
     {{MTRR_UC, MTRR_UC, MTRR_UC, MTRR_UC, MTRR_UC, MTRR_UC, MTRR_UC, MTRR_UC}},
     {{MTRR_UC, MTRR_UC, MTRR_UC, MTRR_UC, MTRR_UC, MTRR_UC, MTRR_UC, MTRR_UC}},
     {{MTRR_UC, MTRR_UC, MTRR_UC, MTRR_UC, MTRR_UC, MTRR_UC, MTRR_UC, MTRR_UC}}},
    {{0LL, 0LL},			/* Variable Range MTRRs */
     {0LL, 0LL},
     {0LL, 0LL},
     {0LL, 0LL},
     {0LL, 0LL},
     {0LL, 0LL},
     {0LL, 0LL},
     {0LL, 0LL}}
    };
#endif	/* (CPU == PENTIUM) */

LOCAL char sysModelStr[80];

/* 
 * Shared memory polling list:
 * This list defines PCI locations which may contain the
 * shared memory anchor
 */

#ifdef SYS_SM_ANCHOR_POLL_LIST

static SYS_SM_DEV_LIST sysSmAnchorPollList[] =
    {
    SYS_SM_ANCHOR_POLL_LIST
    { 0xffffffff, 0xffffffff }  /* Required entry: marks end of list */
    };
#endif

/*
 * Shared memory device list:
 * This list defines PCI locations which may be defined VxWorks
 * nodes and participate in shared memory interrupt generation.
 */

static SYS_SM_DEV_LIST sysSmDeviceList[] =
    {
#   ifdef SYS_SM_DEVICE_LIST
    SYS_SM_DEVICE_LIST
#   endif
    { 0xffffffff, 0xffffffff }  /* Required entry: marks end of list */
    };

LOCAL int       smUtilTasValue = 0;             /* special soft tas value */

/* forward declarations */

LOCAL void sysStrayInt   (void);
char *sysPhysMemTop	 (void);
STATUS sysMmuMapAdd	 (void *address, UINT len, UINT initialStateMask,
                    	  UINT initialState);
LOCAL void sysIntInitPIC (void);
LOCAL void sysIntEoiGet  (VOIDFUNCPTR * vector, 
			  VOIDFUNCPTR * routineBoi, int * parameterBoi,
			  VOIDFUNCPTR * routineEoi, int * parameterEoi);


/* includes (source file) */

LOCAL void sysStrayInt		(void);
#if (PCI_CFG_TYPE == PCI_CFG_AUTO)
    LOCAL void sysVideoPciInit	(void);
    LOCAL void sysSecondaryBusReset (void);
#endif
#if	FALSE  /* they are called at entry and exit of intVecSet() */ 
LOCAL void sysIntVecSetEnt	(FUNCPTR *vector, FUNCPTR function);
LOCAL void sysIntVecSetExit	(FUNCPTR *vector, FUNCPTR function);
#endif	/* FALSE */

#if defined(CPV5350)
#    include "sysNvRam.c"
#endif /* CPV5350 */

#include "mem/byteNvRam.c"
#include "sysSerial.c"

#if ! defined (PIT0_FOR_AUX) && ! defined (PIT1_FOR_AUX)
#   define RTC_FOR_AUX
#endif 

#if	defined (INCLUDE_RTC)
#   include "sysRtc.c"
#endif

#if     defined (INCLUDE_LM78)
#    include "lm78.c"
#endif

#if	defined (INCLUDE_FAILSAFE)
#    include "failsafe.c"
#endif

#if	defined(VIRTUAL_WIRE_MODE)
#   include "intrCtl/loApicIntr.c"
#   include "intrCtl/i8259Intr.c"
#   ifdef INCLUDE_APIC_TIMER
#      include "loApicTimer.c"		/* local reference */
#   else
#      include "i8253Timer.c"		/* local reference */
#   endif /* INCLUDE_APIC_TIMER */
#elif	defined(SYMMETRIC_IO_MODE)
#   include "intrCtl/loApicIntr.c"
#   include "intrCtl/i8259Intr.c"
#   include "intrCtl/ioApicIntr.c"
#   ifdef INCLUDE_APIC_TIMER
#      include "loApicTimer.c"		/* local reference */
#   else
#      include "i8253Timer.c"		/* local reference */
#   endif /* INCLUDE_APIC_TIMER */
#else
#   include "intrCtl/i8259Intr.c"
#   include "i8253Timer.c"		/* local reference */
#endif	/* defined(VIRTUAL_WIRE_MODE) */

#ifdef	INCLUDE_PCI
#   include "pci/pciConfigLib.c"
#   include "pci/pciIntLib.c"

#   if (PCI_CFG_TYPE == PCI_CFG_AUTO)
#       include "pci/pciAutoConfigLib.c"
#       include "./sysBusPci.c"
#   endif /* PCI_CFG_TYPE */

#   ifdef	INCLUDE_SHOW_ROUTINES
#	    include "pci/pciConfigShow.c"
#   endif	/* INCLUDE_SHOW_ROUTINES */
#endif	/* INCLUDE_PCI */

#ifdef	INCLUDE_PCMCIA
#   include "pcmcia/pccardLib.c"
#   include "pcmcia/pccardShow.c"
#endif	/* INCLUDE_PCMCIA */

#include "sysNetif.c"		/* network driver support */
#include "sysScsi.c"		/* scsi support */

/*******************************************************************************
*
* sysModel - return the model name of the CPU board
*
* This routine returns the model name of the CPU board.
*
* RETURNS: A pointer to the string.
*/

char *sysModel (void)

    {
    char * pBrdType;
    char * pCpuType;
    UINT   cpu;
    BOOL   unknown = TRUE;
    int	   i;
    UINT32 eax;
    UINT32 procSig;
    UINT32 intelCfgParams[4];

    /* Initialize the board type */

    pBrdType = "Unknown";

    /* Set the board type */

#if defined(CPV5000)
    pBrdType = "CPV5000";
#elif defined(CPV5300)
    pBrdType = "CPV5300";
#elif defined(CPV5350)
    pBrdType = "CPV5350";
#else
    pBrdType = "PENTIUM"; /* generic pentium */
#endif

    /* Assume we don't know Model type, change this when we find out */

    pCpuType = "Unknown";
    unknown = TRUE;

    /* Find out the CPU type */

    cpu = sysCpuProbe();

    switch (cpu)
    	{
	case 0:
            pCpuType = "386";
	    break;
	case 1:
            pCpuType = "486";
	    break;
	default:

	    if ( (sysCpuId.vendorId[0] == CPUID_VENDOR_INTEL_EBX) &&
	        (sysCpuId.vendorId[2] == CPUID_VENDOR_INTEL_ECX) &&
	        (sysCpuId.vendorId[1] == CPUID_VENDOR_INTEL_EDX) )
	        {

	        /* It's genuine Intel */

	        procSig = sysCpuId.signature;
                eax = procSig & CPUID_FAMILY;

		switch (eax)
		    {
		    case CPUID_PENTIUM:		/* Really "P5" Family */
		        pCpuType = "PENTIUM";
		        unknown = FALSE;
			break;
		    case CPUID_PENTIUMPRO:	/* Really "P6" Family */
		        eax = procSig & CPUID_MODEL;
		        switch (eax)
			    {
			    case CPUID_INTEL_MODEL_P6M1:
		        	pCpuType = "PENTIUMPRO";
		        	unknown = FALSE;
				break;
			    case CPUID_INTEL_MODEL_P6M3:
		        	pCpuType = "PENTIUMII";
		        	unknown = FALSE;
				break;
			    case CPUID_INTEL_MODEL_P6M5:
				intelCfgParams[0] = sysCpuId.cacheEax;
				intelCfgParams[1] = sysCpuId.cacheEbx;
				intelCfgParams[2] = sysCpuId.cacheEcx;
				intelCfgParams[3] = sysCpuId.cacheEdx;
				for (i=0; i<=15; i++)
				    {

				    /* If no cache, it's a Celeron, model 5 */

				    if ( (((intelCfgParams[i/4]) >> (8*(i%4))) &
					 0xff) == CPUID_DESCRIPTOR_NOCACHE )
					{
		        	        pCpuType = "CELERON";
		        	        unknown = FALSE;
                                        break; 
					}
				    }
                                if (unknown == TRUE)
				    {
		        	    pCpuType = "PENTIUMII";
		        	    unknown = FALSE;
				    }
				break;

			    case CPUID_INTEL_MODEL_P6M6: 
			        pCpuType = "CELERON";	/* Celeron, model 6 */
		        	unknown = FALSE;
				break;
			    case CPUID_INTEL_MODEL_P6M7:
			    case CPUID_INTEL_MODEL_P6M8:
		        	pCpuType = "PENTIUMIII";
		        	unknown = FALSE;
				break;
			    }
		    }
	        }
	    else if ( (sysCpuId.vendorId[0] == CPUID_VENDOR_AMD_EBX) &&
	              (sysCpuId.vendorId[2] == CPUID_VENDOR_AMD_ECX) &&
	              (sysCpuId.vendorId[1] == CPUID_VENDOR_AMD_EDX) )
		{

		/* It's an AMD processor, don't take it any further for now */

		pCpuType = "AMD";
		unknown = FALSE;
		break;
		}
    	}

    /* Build up the model string */

#if defined(CPV5300) || defined (CPV5350)

    /* SW correction for incorrect cpuid on Motorla CPV5300/CPV5350 */

    if (strcmp(pCpuType,"CELERON") == 0)
	 pCpuType = "PENTIUMII";
#endif

    sprintf (sysModelStr, "Motorola %s - %s", pBrdType, pCpuType);

    return(sysModelStr);
    }

/*******************************************************************************
*
* sysBspRev - return the BSP version and revision number
*
* This routine returns a pointer to a BSP version and revision number, for
* example, 1.1/0. BSP_REV is concatenated to BSP_VERSION and returned.
*
* RETURNS: A pointer to the BSP version/revision string.
*/

char * sysBspRev (void)
    {
    return (BSP_VERSION BSP_REV);
    }

/*******************************************************************************
*
* sysHwInit - initialize the system hardware
*
* This routine initializes various features of the i386/i486 board.
* It is called from usrInit() in usrConfig.c.
*
* NOTE: This routine should not be called directly by the user application.
*
* RETURNS: N/A
*/

void sysHwInit (void)
    {
    PHYS_MEM_DESC *pMmu;
    int ix = 0;

#ifdef USER_I_CACHE_ENABLE
    cacheEnable(INSTRUCTION_CACHE);
#endif

#if	(CPU == PENTIUM)

    /* enable MTRR (Memory Type Range Registers) */

    if ((sysCpuId.featuresEdx & CPUID_MTRR) == CPUID_MTRR)
	{
        pentiumMtrrDisable ();		/* disable MTRR */
#ifdef	INCLUDE_MTRR_GET
        (void) pentiumMtrrGet (&sysMtrr); /* get MTRR initialized by BIOS */
#else
        (void) pentiumMtrrSet (&sysMtrr); /* set your own MTRR */
#endif	/* INCLUDE_MTRR_GET */
        pentiumMtrrEnable ();		/* enable MTRR */
	}

#ifdef	INCLUDE_PMC
    /* enable PMC (Performance Monitoring Counters) */

    if ((sysProcessor == X86CPU_PENTIUMPRO) && 
	((sysCpuId.featuresEdx & CPUID_MSR) == CPUID_MSR))
	{
	pentiumPmcStop ();		/* stop PMC0 and PMC1 */
	pentiumPmcReset ();		/* reset PMC0 and PMC1 */

	/* 
	 * select events of your interest, such as:
	 * PMC_HW_INT_RX        - number of hardware interrupts received
	 * PMC_MISALIGN_MEM_REF - number of misaligned data memory references
	 */

	(void) pentiumPmcStart (PMC_EN | PMC_OS | PMC_UMASK_00 | PMC_HW_INT_RX,
		        PMC_EN | PMC_OS | PMC_UMASK_00 | PMC_MISALIGN_MEM_REF);
	}
#endif	/* INCLUDE_PMC */

    /* enable MCA (Machine Check Architecture) */

    if ((sysCpuId.featuresEdx & CPUID_MCE) == CPUID_MCE)
	{
#ifdef	INCLUDE_SHOW_ROUTINES
	IMPORT FUNCPTR excMcaInfoShow;
	
	/* 
	 * if excMcaInfoShow is not NULL, it is called in the default
	 * exception handler when Machine Check Exception happened
	 */

	excMcaInfoShow = (FUNCPTR) pentiumMcaShow;
#endif	/* INCLUDE_SHOW_ROUTINES */

        if ((sysCpuId.featuresEdx & CPUID_MCA) == CPUID_MCA)
	    {
	    UINT32 zero[2] = {0x00000000,0x00000000};
	    UINT32 one[2]  = {0xffffffff,0xffffffff};
	    UINT32 cap[2];
	    int mcaBanks;
	    int ix;

	    /* enable all MCA features if MCG_CTL register is present */

	    pentiumMsrGet (MSR_MCG_CAP, (long long int *)&cap);
	    if (cap[0] & MCG_CTL_P)
	        pentiumMsrSet (MSR_MCG_CTL, (long long int *)&one);

	    mcaBanks = cap[0] & MCG_COUNT;	/* get number of banks */

	    /* enable logging of all errors except for the MC0_CTL register */

	    for (ix = 1; ix < mcaBanks; ix++)
	        pentiumMsrSet (MSR_MC0_CTL+(ix * 4), (long long int *)&one);

	    /* clear all errors */

	    for (ix = 0; ix < mcaBanks; ix++)
	        pentiumMsrSet (MSR_MC0_STATUS+(ix * 4), (long long int *)&zero);
	    }

	pentiumCr4Set (pentiumCr4Get () | CR4_MCE); /* enable MC exception */
	}

#endif	/* (CPU == PENTIUM) */

    /* initialize the number of active mappings (sysPhysMemDescNumEnt) */

    pMmu = &sysPhysMemDesc[0];

    for (ix = 0; ix < NELEMENTS (sysPhysMemDesc); ix++) 
        if (pMmu->virtualAddr != (void *)DUMMY_VIRT_ADDR)
            pMmu++;
        else
            break;

    sysPhysMemDescNumEnt = ix;

    /* initialize the PIC (Programmable Interrupt Controller) */

    sysIntInitPIC ();
    intEoiGet = sysIntEoiGet;	/* function pointer used in intConnect () */

#ifdef  INCLUDE_PCI

    /* initialize PCI and related devices */

    pciConfigLibInit (PCI_MECHANISM_1, PCI_CONFIG_ADDR, PCI_CONFIG_DATA, NONE);
    pciIntLibInit ();
#if (PCI_CFG_TYPE == PCI_CFG_NONE)
    sysPciDevsInit ();
#endif

#endif /* INCLUDE_PCI */

    /* initializes the serial devices */

    sysSerialHwInit ();      /* initialize serial data structure */
    }

/*******************************************************************************
*
* sysHwInit2 - additional system configuration and initialization
*
* This routine connects system interrupts and does any additional
* configuration necessary.
*
* RETURNS: N/A
*/

void sysHwInit2 (void)

    {

#if defined (INCLUDE_ADD_BOOTMEM)

    /*
     * We memAddToPool some upper memory into any low memory
     * x86 "rom" images pool.  The x86 low memory images reside
     * from 0x8000 to 0xa0000.  By memAddToPool'ing some upper
     * memory here, we allow devices a larger pool to swim within.
     * (SPR#21338).  This is no longer performed in bootConfig.c
     */

#if (ADDED_BOOTMEM_SIZE != 0x0)
 
    /*
     * if &end (compiler symbol) is in lower memory, then we assume 
     * this is a low memory image, and add some upper memory to the pool.
     */
 
    if ((int)&end < 0x100000)
        {
        /* Only do this if there is enough memory. Default is 4MB min. */
 
        if ((int)memTopPhys >= (0x00200000 + ADDED_BOOTMEM_SIZE))
            {
            memAddToPool ((char *)memTopPhys - ADDED_BOOTMEM_SIZE,
                          ADDED_BOOTMEM_SIZE);
            }
        }
#endif  /* (ADDED_BOOTMEM_SIZE !=0) */
#endif  /* INCLUDE_ADD_BOOTMEM defined */
 

    /* connect sys clock interrupt and auxiliary clock interrupt*/

#ifdef	INCLUDE_APIC_TIMER
    (void)intConnect (INUM_TO_IVEC (TIMER_INT_VEC), sysClkInt, 0);
#ifdef PIT0_FOR_AUX
    (void)intConnect (INUM_TO_IVEC (PIT0_INT_VEC), sysAuxClkInt, 0);
#else
#ifdef INCLUDE_RTC
    /* If the RTC support code is included, use the new RTC-based aux code. */
    (void)intConnect (INUM_TO_IVEC (RTC_INT_VEC), sysAuxRtcInt, 
		(UINT32) &globalTimeBuf);
#else /* INCLUDE_RTC */
    /* use the original aux int handler */
    (void)intConnect (INUM_TO_IVEC (RTC_INT_VEC), sysAuxClkInt, 0);
#endif /* INCLUDE_RTC */
#endif /* PIT0_FOR_AUX */
#else /* not using the APIC timer code */
    (void)intConnect (INUM_TO_IVEC (PIT0_INT_VEC), sysClkInt, 0);
#ifdef INCLUDE_RTC
    /* If RTC support code is included, use new RTC-compatible int hndlr */
    (void)intConnect (INUM_TO_IVEC (RTC_INT_VEC), sysAuxRtcInt, (UINT32) &globalTimeBuf);
#else /* INCLUDE_RTC */
    /* use the original aux int handler */
    (void)intConnect (INUM_TO_IVEC (RTC_INT_VEC), sysAuxClkInt, 0);
#endif /* INCLUDE_RTC */
#endif	/* INCLUDE_APIC_TIMER */

    /* connect serial interrupt */  

    sysSerialHwInit2();

    /* connect stray(spurious/phantom) interrupt */  

#if     defined(VIRTUAL_WIRE_MODE)
    (void)intConnect (INUM_TO_IVEC (SPURIOUS_INT_VEC), sysStrayInt, 0);
    (void)intConnect (INUM_TO_IVEC (LPT_INT_VEC), sysStrayInt, 0);
#elif   defined(SYMMETRIC_IO_MODE)
    (void)intConnect (INUM_TO_IVEC (SPURIOUS_INT_VEC), sysStrayInt, 0);
#else
    (void)intConnect (INUM_TO_IVEC (LPT_INT_VEC), sysStrayInt, 0);
#endif  /* defined(VIRTUAL_WIRE_MODE) */

#ifdef	INCLUDE_PC_CONSOLE

    /* connect keyboard Controller 8042 chip interrupt */

    (void) intConnect (INUM_TO_IVEC (KBD_INT_VEC), kbdIntr, 0);

#endif	/* INCLUDE_PC_CONSOLE */

    /* initialize PCI and related devices */

#ifdef  INCLUDE_PCI

#if (PCI_CFG_TYPE == PCI_CFG_AUTO)

    	/*
    	 * Test to determine if we need to configure the PCI busses with
    	 * pciAutoConfig. If we are coming up from a cold boot, 
	 * ROM-based image then we need to reconfigure. (Cold boot
	 * meaning that the BIOS has handed control to VxWorks.)
	 * The purpose of this check is to avoid configuring the PCI busses 
	 * twice on startup.
    	 */

    	if ( ! PCI_AUTOCONFIG_DONE )
    	    {

	    /* 
	     * Reset the devices on the cPCI bus. This is done so that
	     * devices on the cPCI bus use the WRS PCI configuration values,
	     * not the BIOS programmed values.
	     */

    	    sysSecondaryBusReset();

    	    /* Auto-Configure the PCI busses/devices. */

	    sysPciAutoConfig ();

	    /* Remember that PCI is configured */

	    PCI_AUTOCONFIG_FLAG = TRUE;  

	    /*
	     * The Video device has been excluded through the mechanism
	     * in sysBusPci.c. The below routine will perform the actual
	     * setup of the device.
	     */

	    sysVideoPciInit();
	    }

    /* Initialize specific PCI devices */

    sysPciDevsInit ();

#endif /* PCI_CFG_TYPE == PCI_CFG_AUTO */

    /* 
     * PCI-to-PCI bridge initialization should be done here, if it is.
     * It is not necessary for Intel 430HX PCISET, which splits
     * the extended memory area as follows:
     *   - Flash BIOS area from 4GByte to (4GB - 512KB)
     *   - DRAM memory from 1MB to a maximum of 512MB
     *   - PCI memory space from the top of DRAM to (4GB - 512KB)
     */

#endif /* INCLUDE_PCI */

#ifdef INCLUDE_DEC2155X_SYSTEM_SUPPORT

        /* Connect Dec2155x shared memory interrupt handlers */

        sysDec2155xIntConnect ();
#endif

#ifdef INCLUDE_SM_COMMON
	sysSmParamsCompute ();
#endif

#ifdef  INCLUDE_RTC
	/* init the RTC device */
	sysRtcInit();
#       ifdef INCLUDE_DOSFS
	dosFsDateTimeInstall((FUNCPTR)sysRtcDateTimeHook);
#       endif
#endif

#ifdef  INCLUDE_LM78
        /* Initialize the LM78 */

	lm78Init();
#endif

#ifdef  INCLUDE_FAILSAFE

	/* prevent a failsafe timer from remaining active after a reboot */

        rebootHookAdd ( (FUNCPTR) failsafeRebootHook );
#endif


    }

#ifdef INCLUDE_PCI
/*******************************************************************************
*
* sysPciDevsInit - Initialize specific PCI devices
*
* This routine initializes specific PCI devices which must use the
* information placed into the configuration header via the generic PCI
* configuration process.
*
* RETURNS: N/A
*
*/

LOCAL void sysPciDevsInit (void)

    {
#if defined (INCLUDE_FEI) || defined (INCLUDE_FEI_END)
    sys557PciInit ();
#endif  /* INCLUDE_FEI || INCLUDE_FEI_END */

#ifdef INCLUDE_SCSI
#ifdef  INCLUDE_AIC_7880
    sysAic7880PciInit ();
#endif  /* INCLUDE_AIC_7880 */
#endif  /* INCLUDE_SCSI */

#if defined(INCLUDE_DEC2155X_SYSTEM_SUPPORT) && (PCI_CFG_TYPE == PCI_CFG_NONE)
    sysDec2155xMemMap ();
#endif

    }
#endif /* INCLUDE_PCI */

/*******************************************************************************
*
* sysPhysMemTop - get the address of the top of physical memory
*
* This routine returns the address of the first missing byte of memory,
* which indicates the top of physical memory.
*
* Memory probing begins at the end of BSS; at every 64K boundary an int
* test pattern is written and read back. This read/write 
* sequence continues until the test patern is no longer writen and read
* back correctly. Then the boundary is switched to 4K to fine tune the 
* the actual memory size.
*
* RETURNS: The address of the top of physical memory.
*
* INTERNAL
* This routine is used by sysHwInit() to differentiate between models.
* It is highly questionable whether vxMemProbe() can be used during the
* initial stage of booting.
*/

char *sysPhysMemTop (void)
    {
#define TEST_PATTERN		0x12345678
#define SYS_PAGE_SIZE		0x00010000
#define N_TIMES				3

    static char *memTop = NULL;            /* top of memory */
    PHYS_MEM_DESC *pMmu;
    char gdtr[6];
#ifdef LOCAL_MEM_AUTOSIZE
    char *p;
    int ix;
    int temp[N_TIMES];
    int delta = SYS_PAGE_SIZE;
    BOOL found = FALSE;
#endif

    if (memTop != NULL) 
        return (memTop);

#ifdef LOCAL_MEM_AUTOSIZE
    /* 
     * if (&end) is in upper memory, we assume it is VxWorks image.
     * if not, it is Boot image 
     */

    if ((int)&end > 0x100000)
        p = (char *)(((int)&end + (delta - 1)) & (~ (delta - 1)));
    else
        p = (char *)0x100000;

    /* find out the actual size of the memory (max 1GB) */

    for (; (int)p < 0x40000000; p += delta)
        {
        for (ix=0; ix<N_TIMES; ix++)            /* save and write */
            {
            temp[ix] = *((int *)p + ix);
            *((int *)p + ix) = TEST_PATTERN;
            }

        cacheFlush (DATA_CACHE, p, 4 * sizeof(int));    /* for 486, Pentium */

    	/* Check to see if the test patern was written to memory */

	if (*(int *)p != TEST_PATTERN)			/* compare */
            {
            memTop = p;
            found = TRUE;
            break;
            }
        else
            {
	    for (ix=0; ix<N_TIMES; ix++)		/* restore */
                *((int *)p + ix) = temp[ix];
            }
        }
    
    if (!found)        /* we are fooled by write-back external cache */
        memTop = (char *)(LOCAL_MEM_LOCAL_ADRS + LOCAL_MEM_SIZE);
#else

   memTop = (char *)(LOCAL_MEM_LOCAL_ADRS + LOCAL_MEM_SIZE);

#endif

    /* copy the global descriptor table from RAM/ROM to RAM */

    bcopy ((char *)sysGdt, (char *)pSysGdt, GDT_ENTRIES * sizeof(GDT));
    *(short *)&gdtr[0]    = GDT_ENTRIES * sizeof(GDT) - 1;
    *(int *)&gdtr[2]    = (int)pSysGdt;

/* 
 * We assume that there are no memory mapped IO addresses
 * above the "memTop" if INCLUDE_PCI is not defined.
 * Thus we set the "limit" to get the General Protection Fault
 * when the memory above the "memTop" is accessed.
 */

#if	!defined (INCLUDE_PCI) && \
	!defined (INCLUDE_MMU_BASIC) && !defined (INCLUDE_MMU_FULL)
    {
    GDT *pGdt = pSysGdt;
    int limit = (((int)memTop) / 0x1000 - 1);

    for (ix=1; ix < GDT_ENTRIES; ix++)
	{
	pGdt++;
	pGdt->limit00 = limit & 0x0ffff;
	pGdt->limit01 = ((limit & 0xf0000) >> 16) | (pGdt->limit01 & 0xf0);
	}
    }
#endif	/* INCLUDE_PCI */

    /* load the global descriptor table. set the MMU table */

    sysLoadGdt (gdtr);

#if	(VM_PAGE_SIZE == PAGE_SIZE_4KB)
    pMmu = &sysPhysMemDesc[3];		/* 4th entry: above 1.5MB upper memory */
    pMmu->len = (UINT)memTop - (UINT)pMmu->physicalAddr;
#else	/* (VM_PAGE_SIZE == PAGE_SIZE_4KB) */
    pMmu = &sysPhysMemDesc[1];		/* 2nd entry: above 4MB upper memory */
    pMmu->len = (UINT)memTop - (UINT)pMmu->physicalAddr;
#endif	/* (VM_PAGE_SIZE == PAGE_SIZE_4KB) */

    memTopPhys = memTop;        /* set the real memory size */

    return (memTop);
    }

/*******************************************************************************
*
* sysMemTop - get the address of the top of VxWorks memory
*
* This routine returns a pointer to the first byte of memory not
* controlled or used by VxWorks.
*
* The user can reserve memory space by defining the macro USER_RESERVED_MEM
* in config.h.  This routine returns the address of the reserved memory
* area.  The value of USER_RESERVED_MEM is in bytes.
*
* RETURNS: The address of the top of VxWorks memory.
*/

char * sysMemTop (void)
    {
    static char * memTop = NULL;

    if (memTop == NULL)
        {
        memTop = sysPhysMemTop () - USER_RESERVED_MEM;

        if ((int)&end < 0x100000)		/* this is for bootrom */
            memTop = (char *)0xa0000;
        }

    return (memTop);
    }

/*******************************************************************************
*
* sysToMonitor - transfer control to the ROM monitor
*
* This routine transfers control to the ROM monitor.  It is usually called
* only by reboot() -- which services ^X -- and by bus errors at interrupt
* level.  However, in some circumstances, the user may wish to introduce a
* new <startType> to enable special boot ROM facilities.
*
* RETURNS: Does not return.
*/

STATUS sysToMonitor
    (
    int startType   /* passed to ROM to tell it how to boot */
    )
    {
    FUNCPTR pEntry;
    int ix;
    int iy;
    int iz;
    char buf[ROM_SIGNATURE_SIZE];
    short *pSrc;
    short *pDst;
    u_char * pChar;                     /* used in dosFsVolDescGet() */

    VM_ENABLE (FALSE);			/* disable MMU */

#if	(CPU == PENTIUM) || (CPU == PENTIUM2) || (CPU == PENTIUM3) || \
	(CPU == PENTIUM4)

    pentiumMsrInit ();			/* initialize MSRs */

#endif	/* (CPU == PENTIUM) || (CPU == PENTIUM[234]) */

    /* decide a destination RAM address and the entry point */

    if ((int)&end > 0x100000)
	{
	pDst = (short *)RAM_HIGH_ADRS;	/* copy it in lower mem */
	pEntry = (FUNCPTR)(RAM_HIGH_ADRS + ROM_WARM_HIGH);
	}
    else
	{
	pDst = (short *)RAM_LOW_ADRS;	/* copy it in upper mem */
	pEntry = (FUNCPTR)(RAM_LOW_ADRS + ROM_WARM_LOW);
	}

    /* copy EPROM to RAM and jump, if there is a VxWorks EPROM */

    for (ix = 0; ix < NELEMENTS(sysRomBase); ix++)
	{
	bcopyBytes ((char *)sysRomBase[ix], buf, ROM_SIGNATURE_SIZE);
	if (strncmp (sysRomSignature, buf, ROM_SIGNATURE_SIZE) == 0)
	    {
	    for (iy = 0; iy < 1024; iy++)
		{
		*sysRomBase[ix] = iy;		/* map the moveable window */
		pSrc = (short *)((int)sysRomBase[ix] + 0x200);
	        for (iz = 0; iz < 256; iz++)
		    *pDst++ = *pSrc++;
		}
            sysClkDisable ();	/* disable the system clock interrupt */
	    sysIntInitPIC ();	/* reset the used interrupt controllers */
	    (*pEntry) (startType);
	    }
	}

#if     (defined(INCLUDE_FD) || defined(INCLUDE_ATA) || defined(INCLUDE_TFFS))   
    if ((sysWarmType == SYS_WARM_FD) || (sysWarmType == SYS_WARM_ATA) ||         
        (sysWarmType == SYS_WARM_TFFS))                                          
        {                                                                        
        /* check to see if device exists, if so don't create it */               
                                                                                 
        if (NULL != dosFsVolDescGet(BOOTROM_DIR, &pChar))                        
            {                                                                    
            goto bootDevExists; /* avoid attempt to recreate device */           
            }                                                                    
        }                                                                        
#endif /* defined(INCLUDE_FD) || defined(INCLUDE_ATA) || defined(INCLUDE_TFFS) */

#ifdef	INCLUDE_FD
    if (sysWarmType == SYS_WARM_FD)
	{
	IMPORT int dosFsDrvNum;

        fdDrv (FD_INT_VEC, FD_INT_LVL);		/* initialize floppy disk */
	if (dosFsDrvNum == ERROR)
    	    dosFsInit (NUM_DOSFS_FILES);	/* initialize DOS-FS */

        if (usrFdConfig (sysWarmFdDrive, sysWarmFdType, BOOTROM_DIR) == ERROR)
	    {
	    printErr ("usrFdConfig failed.\n");
	    return (ERROR);
	    }
	}
#endif	/* INCLUDE_FD */

#ifdef	INCLUDE_ATA
    if (sysWarmType == SYS_WARM_ATA)
	{
	ATA_RESOURCE *pAtaResource  = &ataResources[sysWarmAtaCtrl];
	IMPORT int dosFsDrvNum;

        if (ataDrv (sysWarmAtaCtrl, pAtaResource->drives,
	    pAtaResource->intVector, pAtaResource->intLevel,
	    pAtaResource->configType, pAtaResource->semTimeout,
	    pAtaResource->wdgTimeout) == ERROR)	/* initialize ATA/IDE disk */
	    {
	    printErr ("Could not initialize.\n");
	    return (ERROR);
	    }
	if (dosFsDrvNum == ERROR)
    	    dosFsInit (NUM_DOSFS_FILES);        /* initialize DOS-FS */

        if (ERROR == usrAtaConfig (sysWarmAtaCtrl,
                                   sysWarmAtaDrive, BOOTROM_DIR))
	    {
	    printErr ("usrAtaConfig failed.\n");
	    return (ERROR);
	    }
	}
#endif	/* INCLUDE_ATA */

#ifdef	INCLUDE_TFFS
    if (sysWarmType == SYS_WARM_TFFS)
	{
	IMPORT int dosFsDrvNum;

        tffsDrv ();				/* initialize TFFS */
	if (dosFsDrvNum == ERROR)
    	    dosFsInit (NUM_DOSFS_FILES);	/* initialize DOS-FS */

        if (usrTffsConfig (sysWarmTffsDrive, FALSE, BOOTROM_DIR) == ERROR)
	    {
	    printErr ("usrTffsConfig failed.\n");
	    return (ERROR);
	    }
	}
#endif	/* INCLUDE_TFFS */

bootDevExists:  /* reboot device exists */

#if	(defined(INCLUDE_FD) || defined(INCLUDE_ATA) || defined(INCLUDE_TFFS))
    if ((sysWarmType == SYS_WARM_FD) || (sysWarmType == SYS_WARM_ATA) || 
	(sysWarmType == SYS_WARM_TFFS))
	{
        int fd;

        if ((fd = open (BOOTROM_BIN, O_RDONLY, 0644)) == ERROR)
	    {
            if ((fd = open (BOOTROM_AOUT, O_RDONLY, 0644)) == ERROR)
		{
                printErr ("Can't open \"%s\"\n", "bootrom.{sys,dat}");
	        return (ERROR);
		}
            if (read (fd, (char *)pDst, 0x20) == ERROR) /* a.out header */
	        {
	        printErr ("Error during read file: %x\n", errnoGet ());
	        return (ERROR);
                }
	    }

        /* read text and data, write them to the memory */

        if (read (fd, (char *)pDst, 0x98000) == ERROR)
	    {
	    printErr ("Error during read file: %x\n", errnoGet ());
	    return (ERROR);
            }

#ifdef INCLUDE_FD
        /* explicitly release floppy disk, SPR#30280 */

        if (SYS_WARM_FD == sysWarmType)
            {
            sysOutByte(FD_REG_OUTPUT,(FD_DOR_CLEAR_RESET | FD_DOR_DMA_ENABLE));
            sysDelay ();
            }
#endif /* INCLUDE_FD */

        sysClkDisable ();	/* disable the system clock interrupt */
	sysIntInitPIC ();	/* reset the used interrupt controllers */

#if	defined (SYMMETRIC_IO_MODE) || defined (VIRTUAL_WIRE_MODE)

        intLock ();			/* LOCK INTERRUPTS */
        loApicEnable (FALSE);		/* disable the Local APIC */

#   if defined (SYMMETRIC_IO_MODE)

	if (sysBp)
            ioApicEnable (FALSE);	/* disable the IO APIC */

#   endif /* defined (SYMMETRIC_IO_MODE) */
#endif	/* defined (SYMMETRIC_IO_MODE) || defined (VIRTUAL_WIRE_MODE) */

        (*pEntry) (startType);
	}
#endif	/* (INCLUDE_FD) || (INCLUDE_ATA) || (INCLUDE_TFFS) */

    intLock ();

    sysClkDisable ();

    /* Induce a reboot by causing the PIIX to reset the CPU */

    sysOutByte (PIIX_RC, PIIX_RC_RCPU | PIIX_RC_SRST);	/* Hard reset */

    return (OK); /* Should never get here */
    }


/*******************************************************************************
*
* sysIntInitPIC - initialize the interrupt controller
*
* This routine initializes the interrupt controller.
*
* RETURNS: N/A
*
* ARGSUSED0
*/

LOCAL void sysIntInitPIC (void)
    {

#if	defined(VIRTUAL_WIRE_MODE)
    {
    int addrLo;		/* page aligned Local APIC Base Address */
    int lengthLo;	/* length of Local APIC registers */

    loApicInit ();
    i8259Init ();

    /* add an entry to the sysMmuPhysDesc[] for Local APIC */

    addrLo   = ((int)loApicBase / PAGE_SIZE) * PAGE_SIZE;
    lengthLo = (int)loApicBase - addrLo + LOAPIC_LENGTH;
    if ((lengthLo % PAGE_SIZE) != 0)
	lengthLo = (lengthLo / PAGE_SIZE + 1) * PAGE_SIZE;
    
    sysMmuMapAdd ((void *)addrLo, lengthLo, 
		  VM_STATE_MASK_FOR_ALL, VM_STATE_FOR_IO);
    }
#elif	defined(SYMMETRIC_IO_MODE)
    {
    int addrLo;		/* page aligned Local APIC Base Address */
    int addrIo;		/* page aligned IO APIC Base Address */
    int lengthLo;	/* length of Local APIC registers */
    int lengthIo;	/* length of IO APIC registers */

    loApicInit ();
    i8259Init ();
    ioApicInit ();

    /* add an entry to the sysMmuPhysDesc[] for Local APIC and IO APIC */

    addrLo   = ((int)loApicBase / PAGE_SIZE) * PAGE_SIZE;
    lengthLo = (int)loApicBase - addrLo + LOAPIC_LENGTH;
    if ((lengthLo % PAGE_SIZE) != 0)
	lengthLo = (lengthLo / PAGE_SIZE + 1) * PAGE_SIZE;
    
    addrIo   = ((int)ioApicBase / PAGE_SIZE) * PAGE_SIZE;
    lengthIo = (int)ioApicBase - addrIo + IOAPIC_LENGTH;
    if ((lengthIo % PAGE_SIZE) != 0)
	lengthIo = (lengthIo / PAGE_SIZE + 1) * PAGE_SIZE;
    
    if (addrLo == addrIo)
	{
        sysMmuMapAdd ((void *)addrLo, 
		      max (lengthLo, lengthIo),
		      VM_STATE_MASK_FOR_ALL, VM_STATE_FOR_IO);
	}
    else if ((addrLo < addrIo) && ((addrLo + lengthLo) >= addrIo)) 
	{
        sysMmuMapAdd ((void *)addrLo, 
		      max ((addrLo + lengthLo), (addrIo + lengthIo)) - addrLo,
		      VM_STATE_MASK_FOR_ALL, VM_STATE_FOR_IO);
	}
    else if ((addrIo < addrLo) && ((addrIo + lengthIo) >= addrLo))
	{
        sysMmuMapAdd ((void *)addrIo, 
		      max ((addrLo + lengthLo), (addrIo + lengthIo)) - addrIo,
		      VM_STATE_MASK_FOR_ALL, VM_STATE_FOR_IO);
	}
    else
	{
        sysMmuMapAdd ((void *)addrLo, lengthLo, 
		      VM_STATE_MASK_FOR_ALL, VM_STATE_FOR_IO);
        sysMmuMapAdd ((void *)addrIo, lengthIo, 
		      VM_STATE_MASK_FOR_ALL, VM_STATE_FOR_IO);
	}
    }
#else
    i8259Init ();
#endif	/* defined(VIRTUAL_WIRE_MODE) */
    }

/*******************************************************************************
*
* sysIntLock - lock out all interrupts
*
* This routine saves the mask and locks out all interrupts.
* It should be called in the interrupt disable state(IF bit is 0).
*
* SEE ALSO: sysIntUnlock()
*
* ARGSUSED0
*/

VOID sysIntLock (void)

    {

#if	defined(VIRTUAL_WIRE_MODE)
    loApicIntLock ();
    i8259IntLock ();
#elif	defined(SYMMETRIC_IO_MODE)
    loApicIntLock ();
    ioApicIntLock ();
#else
    i8259IntLock ();
#endif	/* defined(VIRTUAL_WIRE_MODE) */
    }

/*******************************************************************************
*
* sysIntUnlock - unlock the PIC interrupts
*
* This routine restores the mask and unlocks the PIC interrupts
* It should be called in the interrupt disable state(IF bit is 0).
*
* SEE ALSO: sysIntLock()
*
* ARGSUSED0
*/

VOID sysIntUnlock (void)

    {

#if	defined(VIRTUAL_WIRE_MODE)
    loApicIntUnlock ();
    i8259IntUnlock ();
#elif	defined(SYMMETRIC_IO_MODE)
    loApicIntUnlock ();
    ioApicIntUnlock ();
#else
    i8259IntUnlock ();
#endif	/* defined(VIRTUAL_WIRE_MODE) */
    }

/*******************************************************************************
*
* sysIntDisablePIC - disable a bus interrupt level
*
* This routine disables a specified bus interrupt level.
*
* RETURNS: OK, or ERROR if failed.
*
* ARGSUSED0
*/

STATUS sysIntDisablePIC
    (
    int irqNo		/* IRQ(PIC) or INTIN(APIC) number to disable */
    )
    {

#if	defined(VIRTUAL_WIRE_MODE)
    return (i8259IntDisable (irqNo));
#elif	defined(SYMMETRIC_IO_MODE)
    return (ioApicIntDisable (irqNo));
#else
    return (i8259IntDisable (irqNo));
#endif	/* defined(VIRTUAL_WIRE_MODE) */
    }

/*******************************************************************************
*
* sysIntEnablePIC - enable a bus interrupt level
*
* This routine enables a specified bus interrupt level.
*
* RETURNS: OK, or ERROR if failed.
*
* ARGSUSED0
*/

STATUS sysIntEnablePIC
    (
    int irqNo		/* IRQ(PIC) or INTIN(APIC) number to enable */
    )
    {

#if	defined(VIRTUAL_WIRE_MODE)
    return (i8259IntEnable (irqNo));
#elif	defined(SYMMETRIC_IO_MODE)
    return (ioApicIntEnable (irqNo));
#else
    return (i8259IntEnable (irqNo));
#endif	/* defined(VIRTUAL_WIRE_MODE) */
    }

/*******************************************************************************
*
* sysIntEoiGet - get EOI/BOI function and its parameter
*
* This routine gets EOI function and its parameter for the interrupt controller.
* If returned EOI/BOI function is NULL, intHandlerCreateX86() replaces 
* "call _routineBoi/Eoi" in intConnectCode[] with NOP instruction.
*
* RETURNS: N/A
*
* ARGSUSED0
*/

LOCAL void sysIntEoiGet
    (
    VOIDFUNCPTR * vector,	/* interrupt vector to attach to */
    VOIDFUNCPTR * routineBoi,	/* BOI function */
    int * parameterBoi,		/* a parameter of the BOI function */
    VOIDFUNCPTR * routineEoi,	/* EOI function */
    int * parameterEoi		/* a parameter of the EOI function */
    )
    {
    int vectorNo = IVEC_TO_INUM (vector);
    int irqNo;

    *routineBoi = NULL;		/* set the default value */
    *parameterBoi = 0;		/* set the default value */

#if	defined(VIRTUAL_WIRE_MODE)
    if (vectorNo == TIMER_INT_VEC)
	{
        *routineEoi   = loApicIntEoi;
        *parameterEoi = TIMER_INT_LVL;
	}
    else if (vectorNo == SPURIOUS_INT_VEC)
	{
        *routineEoi   = NULL;		/* no EOI is necessary */
        *parameterEoi = SPURIOUS_INT_LVL;
	}
    else if (vectorNo == ERROR_INT_VEC)
	{
        *routineEoi   = loApicIntEoi;
        *parameterEoi = ERROR_INT_LVL;
	}
    else
        {
        irqNo = vectorNo - INT_NUM_IRQ0;
	if ((irqNo == 7) || (irqNo == 15))
	    {
            *routineBoi   = i8259IntBoi;
            *parameterBoi = irqNo;
	    }
	if (irqNo < 8)
            *routineEoi   = i8259IntEoiMaster;
	else
            *routineEoi   = i8259IntEoiSlave;
        *parameterEoi = irqNo;
        }
#elif	defined(SYMMETRIC_IO_MODE)
    if (vectorNo == TIMER_INT_VEC)
	{
        *routineEoi   = loApicIntEoi;
        *parameterEoi = TIMER_INT_LVL;
	}
    else if (vectorNo == SPURIOUS_INT_VEC)
	{
        *routineEoi   = NULL;		/* no EOI is necessary */
        *parameterEoi = SPURIOUS_INT_LVL;
	}
    else if (vectorNo == ERROR_INT_VEC)
	{
        *routineEoi   = loApicIntEoi;
        *parameterEoi = ERROR_INT_LVL;
	}
    else
        {
        for (irqNo = 0; irqNo < NELEMENTS (redTable); irqNo++)
	    if (redTable [irqNo].vectorNo == vectorNo)
	        break;
        *routineEoi   = ioApicIntEoi;
        *parameterEoi = irqNo;
        }
#else
    irqNo = vectorNo - INT_NUM_IRQ0;
    if ((irqNo == 7) || (irqNo == 15))
        {
        *routineBoi   = i8259IntBoi;
        *parameterBoi = irqNo;
        }
    if (irqNo < 8)
        *routineEoi   = i8259IntEoiMaster;
    else
        *routineEoi   = i8259IntEoiSlave;
    *parameterEoi = irqNo;
#endif	/* defined(VIRTUAL_WIRE_MODE) */
    }

/*******************************************************************************
*
* sysIntLevel - get an IRQ(PIC) or INTIN(APIC) number in service
*
* This routine gets an IRQ(PIC) or INTIN(APIC) number in service.  
* We assume followings:
*   - this function is called in intEnt()
*   - IRQ number of the interrupt is at intConnectCode [29]
*
* RETURNS: 0 - (NUMBER_OF_IRQS - 1), or NUMBER_OF_IRQS if we failed to get it.
*
* ARGSUSED0
*/

#ifdef	SYS_INT_DEBUG
UINT32 sysIntCount[NUMBER_OF_IRQS + 1];
#endif	/* SYS_INT_DEBUG */

int sysIntLevel 
    (
    int arg		/* parameter to get the stack pointer */
    )
    {
    UINT32 * pStack;
    UCHAR * pInst;
    int ix;
    int irqNo = NUMBER_OF_IRQS;	/* return NUMBER_OF_IRQS if we failed */

    pStack = &arg;		/* get the stack pointer */
    pStack += 3;		/* skip pushed volitile registers */

    /* 
     * we are looking for a return address on the stack which point
     * to the next instruction of "call _intEnt" in the malloced stub.
     * Then get the irqNo at intConnectCode [29].
     */

    for (ix = 0; ix < 10; ix++, pStack++)
	{
	pInst = (UCHAR *)*pStack;		/* return address */
	if ((*pInst == 0x50) && 		/* intConnectCode [5] */
	    ((*(int *)(pInst - 4) + (int)pInst) == (int)intEnt))
	    {
    	    irqNo = *(int *)(pInst + 24);	/* intConnectCode [29] */
	    break;
	    }
	}

#ifdef	SYS_INT_DEBUG
    sysIntCount[irqNo]++;
#endif	/* SYS_INT_DEBUG */

    return (irqNo);
    }

/****************************************************************************
*
* sysProcNumGet - get the processor number
*
* This routine returns the processor number for the CPU board, which is
* set with sysProcNumSet().
*
* RETURNS: The processor number for the CPU board.
*
* SEE ALSO: sysProcNumSet()
*/

int sysProcNumGet (void)

    {
    return (sysProcNum);
    }


/****************************************************************************
*
* sysProcNumSet - set the processor number
*
* Set the processor number for the CPU board.  Processor numbers should be
* unique on a single backplane.
*
* NOTE: By convention, only Processor 0 should dual-port its memory.
*
* RETURNS: N/A
*
* SEE ALSO: sysProcNumGet()
*/

void sysProcNumSet
    (
    int procNum		/* processor number */
    )
    {
    sysProcNum = procNum;
    }

/*******************************************************************************
*
* sysDelay - allow recovery time for port accesses
*
* This routine provides a brief delay used between accesses to the same serial
* port chip.
* 
* RETURNS: N/A
*/

void sysDelay (void)
    {
    char ix;

    ix = sysInByte (UNUSED_ISA_IO_ADDRESS);	/* it takes 720ns */
    }

/*******************************************************************************
*
* sysNanoDelay - delay a specifc number of nanoSeconds
*
* This routine provides a delay for a specified number of nanoSeconds.
* 
* RETURNS: N/A
*/

void sysNanoDelay 
    (
    UINT32 nanoSeconds
    )

    {
    int count = (nanoSeconds / 720) + 1;

    while (count-- > 0)
	sysDelay();
    }

/********************************************************************************
* sysMsDelay - delay for a specfied number of milliseconds
*
* This routine will delay for the approximate number of milliseconds
* specified in the input paramter.  It compensates for tick timer rollover.
*
*/

void sysMsDelay 
    (
    UINT32	delay			/* length of time in MS to delay */
    )
    {

    register ULONG oldval;           /* decrementer value */
    register ULONG newval;           /* decrementer value */
    register ULONG totalDelta;       /* Dec. delta for entire delay period */
    register ULONG tickElapsed;      /* cumulative decrementer ticks */

    /*
     * Calculate delta of decrementer ticks for desired elapsed time.
     */

    totalDelta = ( (sysClkRateGet() * delay) / 1000 ) + 1;

    /*
     * Now keep grabbing tick counter value and incrementing "decElapsed" 
     * until we hit the desired delay value.  Compensate for rollover.
     */

    tickElapsed = 0;
    oldval = tickGet ();
    while (tickElapsed < totalDelta)
        {
        newval = tickGet ();
        if (newval >= oldval)
            tickElapsed += (newval - oldval); 	/* No rollover */
        else
	    tickElapsed += oldval;		/* Rollover */
        oldval = newval;
        }
    }

/*******************************************************************************
*
* sysStrayInt - Do nothing for stray interrupts.
*
* Do nothing for stray interrupts.
*/

LOCAL void sysStrayInt (void)
    {
    sysStrayIntCount++;
    }

/*******************************************************************************
*
* sysMmuMapAdd - insert a new mmu mapping
*
* This routine adds a new mmu mapping entry to allow dynamic mappings. 
*
* RETURNS: OK or ERROR depending on availability of free mappings.
*/  

STATUS sysMmuMapAdd 
    (
    void *address,
    UINT length,
    UINT initialStateMask,
    UINT initialState
    )
    {
    PHYS_MEM_DESC *pMmu;
    STATUS result = OK;

    if (sysPhysMemDescNumEnt >= sizeof(sysPhysMemDesc)/sizeof(PHYS_MEM_DESC))
	{

	/* No more entries in the sysPhysMemDesc [] table. */

	return (ERROR);
	}

    pMmu = &sysPhysMemDesc[sysPhysMemDescNumEnt];
  
    if(pMmu->virtualAddr != (void *)DUMMY_VIRT_ADDR)
        result = ERROR;
    else
        {
        pMmu->virtualAddr = address;
        pMmu->physicalAddr = address;
        pMmu->len = length;
        pMmu->initialStateMask = initialStateMask;
        pMmu->initialState = initialState;
        sysPhysMemDescNumEnt += 1;
        }
   
    return (result);
    }

#if (PCI_CFG_TYPE == PCI_CFG_AUTO)
/*******************************************************************************
*
* sysVideoPciInit - Perform PCI configuration for the video device
*
*/

LOCAL void sysVideoPciInit(void)
    {

    int   busNo;	/* PCI bus number              */
    int   devNo;	/* PCI device number           */
    int   funcNo;	/* PCI function number         */
    int   n = 0;	/* desired instance of device  */

    /* Find the nth instance of a video device */

    if ((pciFindClass ((PCI_CLASS_DISPLAY_CTLR << 16), 
			n, &busNo, &devNo, &funcNo)) == OK)
        {
#   	ifdef INCLUDE_PC_CONSOLE

	/* Enable io accesses to this legacy device */

	pciConfigModifyLong (busNo, devNo, devNo, PCI_CFG_COMMAND,
			 (0xffff0000 | PCI_CMD_IO_ENABLE), PCI_CMD_IO_ENABLE);

#    	else
	
	/*
	 * If the console is not a video device, Disable I/O, Mem, 
	 * and upstream transactions / reset status bits.
	 */

	pciConfigOutLong(busNo, devNo, devNo, PCI_CFG_COMMAND, 0xffff0000);

#  	endif /* INCLUDE_PC_CONSOLE */

	}
    }
/*******************************************************************************
*
* sysSecondaryBusReset - Reset the devices on the secondary bus
*
*/

LOCAL void sysSecondaryBusReset
    (
    void
    )
    {

    /* Assert RST# on the secondary bus */

    pciConfigModifyByte (PCI2PCI_BUS, PCI2PCI_DEV, PCI2PCI_FUN, 
		PCI_CFG_BRIDGE_CONTROL, PCI_CFG_SEC_BUS_RESET, 0x40);

    /* Clear the bit, deasserting the RST# on the secondary bus */

    pciConfigModifyByte (PCI2PCI_BUS, PCI2PCI_DEV, PCI2PCI_FUN, 
		PCI_CFG_BRIDGE_CONTROL, PCI_CFG_SEC_BUS_RESET, 0x00);
    }

#endif /* (PCI_CFG_TYPE == PCI_CFG_AUTO) */

/*******************************************************************************
*
* sysLocalToBusAdrs - convert a local address to a bus address
*/

STATUS sysLocalToBusAdrs
    (
    int  adrsSpace,     /* bus address space in which busAdrs resides,  */
                        /* use address modifier codes                   */
    char *localAdrs,    /* local address to convert                     */
    char **pBusAdrs     /* where to return bus address                  */
    )
    {
    *pBusAdrs = localAdrs;
    return (OK);
    }

/******************************************************************************
*
* sysBusToLocalAdrs - convert a bus address to a local address
*
*/

STATUS sysBusToLocalAdrs
    (
    int  adrsSpace,     /* bus address space in which busAdrs resides,  */
                        /* use address modifier codes                   */
    char *busAdrs,      /* bus address to convert                       */
    char **pLocalAdrs   /* where to return local address                */
    )
    {
    *pLocalAdrs = busAdrs;
    return (OK);
    }

/*******************************************************************************
*
* sysBusTas - test and set a location across the bus
*
* The cPCI bridge chips do not support PCI target locking, therefore there is
* no atomic RMW operation. This routine performs a software-based mutual
* exclusion algorithm in place of a true test and set.
*
* NOTE: This algorithm is performed through a PCI-to-PCI bridge to a shared
* location that is subject to unprotected access by multiple simultaneous
* processors. There is the possibility that the bridge will deliver a delayed
* read completion to a PCI bus master which was not the original initiator of
* the delayed read. The bridge delivers the delayed read completion to the new
* requestor because it believes that the new delayed read request is actually
* the original master performing a delayed read retry as required by the PCI
* spec. When the original master comes back with the genuine retry, the bridge
* treats it as a new request. When this "aliasing" occurs, a read performed
* after a write can appear to complete ahead of the write, which is in violation
* of PCI transaction ordering rules. Since this algorithm depends on a strict
* time-ordered sequence of operations, it can deadlock under this condition.
* To prevent the deadlock, a throw-away read must be performed after the initial
* write. Since the bridge only remembers once instance of a queued delayed
* read request, the throw-away read will "consume" the results of a
* mis-directed read completion and subsequent read requests are guaranteed to
* be queued and completed after the write.
*
* RETURNS: TRUE if lock acquired.
*	   FALSE if lock not acquired.
*
* SEE ALSO: sysBusTasClear()
*/

BOOL sysBusTas
    (
    char * adrs 	 /* address to be tested and set */
    )
    {
    FAST int	value;			/* value to place in lock variable */
    FAST int	nChecks;		/* number of times to re-check lock */
    FAST int	count;			/* running count of re-checks */
    int 	oldLvl; 		/* previous interrupt level */

    volatile int * lockAdrs = (int *)adrs;

    if (smUtilTasValue == 0)
	smUtilTasValue =  htonl ((TAS_CONST + sysProcNumGet ())<< 24);

    value   = smUtilTasValue;			/* special value to write */
    nChecks = DEFAULT_TAS_CHECKS;		/* number of checks to do */

    /* Lock out interrupts */

    oldLvl = intLock ();

    /* Test that lock variable is initially empty */

    if (*lockAdrs != 0)
	{
	intUnlock (oldLvl);
	return (FALSE); 			/* someone else has lock */
	}

    /* Set lock value */

    *lockAdrs = value;

    /* Perform a preliminary read due to PCI bridge issues */

    count = *lockAdrs;

    /* Check that no one else is trying to take lock */

    for (count = 0;  count < nChecks;  count++)
	{
	if (*lockAdrs != value)
	    {
	    intUnlock (oldLvl);
	    return (FALSE);			/* someone else stole lock */
	    }
	}

    intUnlock (oldLvl);
    return (TRUE);				/* exclusive access obtained */

    }

/*****************************************************************************
*
* sysPciOutWordConfirm - Word out to PCI memory space and flush buffers.
*
* This function outputs a word to PCI memory space and then flushes the PCI
* write posting buffers by reading from the target address. Since the PCI
* spec requires the completion of posted writes before the completion of delayed
* reads, when the read completes, the write posting buffers have been flushed.
*
* NOTE: If the write is performed through a PCI-to-PCI bridge to a shared
* location that is subject to unprotected access by multiple simultaneous
* processors, there is the possibility that the bridge will deliver a delayed
* read completion to a PCI bus master which was not the original initiator of
* the delayed read. When this occurs, it appears as if a PCI delayed read had
* passed a posted write, which would violate PCI transaction ordering rules.
* If this is a concern, an additional read must be performed outside of this
* routine to guarantee that the confirming read performed in this routine was
* not aliased.
*
* RETURNS: N/A
*/

void sysPciOutWordConfirm
    (
    UINT32 adrs,       /* PCI address */
    UINT16 data        /* data to be written */
    )
    {
    UINT16 temp;

    sysPciOutWord (adrs, data);
    temp = sysPciInWord (adrs);

    }

/*******************************************************************************
*
* sysBusIntGen - generate a bus interrupt
*
* This routine generates a specified Compact PCI bus interrupt level.
*
* NOTE: This routine is included for BSP compliance only.  Since the host
* (system board) does not generate bus interrupts, the routine does nothing.
*
* RETURNS: ERROR
*
* SEE ALSO: sysBusIntAck()
*/

STATUS sysBusIntGen
    (
    int intLevel,	/* interrupt level to acknowledge */
    int vector
    )
    {
    return (ERROR);
    }

/*******************************************************************************
*
* sysMailboxConnect - connect a routine to the mailbox interrupt
*
* This routine specifies the interrupt service routine to be called at each
* mailbox interrupt.
*
* NOTE: This routine is included for BSP compliance only.  Since the host
* (system board) not have mailboxes, this routine simply returns ERROR.
*
* RETURNS: ERROR
*
* SEE ALSO: sysIntEnable()
*/

STATUS sysMailboxConnect
    (
    FUNCPTR routine,	/* routine called at each mailbox interrupt */
    int arg		/* argument with which to call routine	    */
    )
    {
    return (ERROR);
    }

/*******************************************************************************
*
* sysMailboxEnable - enable the mailbox interrupt
*
* This routine enables the mailbox interrupt.
*
* NOTE: This routine is included for BSP compliance only.  Since the host
* (system board) not have mailboxes, this routine simply returns ERROR.
*
* RETURNS: ERROR
*
*/

STATUS sysMailboxEnable
    (
    char *mailboxAdrs		/* address of mailbox (ignored) */
    )
    {
    return (ERROR);
    }

/*******************************************************************************
*
* sysBusIntAck - acknowledge a bus interrupt
*
* This routine acknowledges a specified Compact PCI bus interrupt level.
*
* NOTE: This routine is included for BSP compliance only.  Since Compact
* PCI bus interrupts are routed directly to the interrupt controller,
* interrupts are re-enabled in the interrupt controller's handler and this
* routine is a	no-op.
*
* RETURNS: 0, always
*
* SEE ALSO: sysBusIntGen()
*/

int sysBusIntAck
    (
    int intLevel	/* interrupt level to acknowledge */
    )
    {
    return (0);
    }

#ifdef INCLUDE_DEC2155X_SYSTEM_SUPPORT 
#if (PCI_CFG_TYPE == PCI_CFG_NONE)
/*****************************************************************************
*
* sysDec2155xMemMap
*
* This routine finds each of the Motorola MCP750s (Dec 21554 bridges) and
* for each board found, maps MMU memory for the board.
*
*/

LOCAL void sysDec2155xMemMap ()
    {

    int 	index = 0;
    UINT16	subVendId;
    UINT16	subSystId;
    UINT32	busAddr;
    char	*locAddr;
    UINT	bus;
    UINT	dev;
    UINT	func;

    /*
     * For each shared memory dec2155x found, map memory to cover 
     * CSR space and low DRAM.
     */

    while (pciFindDevice(PCI_ID_VEND_DEC21554, PCI_ID_DEV_DEC21554, index, 
			 &bus, &dev, &func) == OK)
	{

	pciConfigInWord (bus, dev, func, PCI_CFG_SUB_VENDER_ID, &subVendId);
	pciConfigInWord (bus, dev, func, PCI_CFG_SUB_SYSTEM_ID, &subSystId);

	/* If it's a shared memory participant board then ... */

        if (sysSmDeviceParticipant (bus, dev, func))

	    {
	    pciConfigInLong (bus, dev, func, PCI_CFG_BASE_ADDRESS_0, &busAddr);

	    busAddr &= PCI_MEMBASE_MASK;

	    if ( sysBusToLocalAdrs (PCI_SPACE_MEM_SEC, 
				    (char *)busAddr, &locAddr) == ERROR )
		continue;

            (void) sysMmuMapAdd ( locAddr, DEC2155X_CSR_AND_DS_MEM0_SIZE,
				  VM_STATE_MASK_FOR_ALL, VM_STATE_FOR_PCI );


	    }
	index++;
	}

    }
#endif /* PCI_CFG_TYPE == PCI_CFG_NONE */

/*****************************************************************************
*
* sysSmDeviceParticipant - determine if a device participates in Shared Memory.
*
* This function will determine whether a given bus, device, and function
* number correspond to a defined device in sysSmDeviceList.
*
* RETURNS: TRUE if device is in sysSmDeviceList, FALSE if device is not
* in the sysSmDeviceList.
*/
LOCAL UINT sysSmDeviceParticipant
    (
    UINT busNo,		/* PCI bus number to check */
    UINT deviceNo,	/* PCI device number to check */
    UINT funcNo		/* PCI function number to check */
    )
    {
    UINT i;
    UINT found = FALSE;
    UINT devVend;
    UINT subIdVend;

    for (i = 0 ; (sysSmDeviceList[i].devVend != 0xffffffff) ; i++)
        {
        pciConfigInLong (busNo, deviceNo, funcNo, PCI_CFG_VENDOR_ID,
                             &devVend);
        if (devVend == sysSmDeviceList[i].devVend)
            {

            pciConfigInLong(busNo, deviceNo, funcNo, PCI_CFG_SUB_VENDER_ID,
                                &subIdVend);
            if (subIdVend == sysSmDeviceList[i].subIdVend)
                {
                found = TRUE;
                break;
                }
            }
        }
    return (found);
    }

/*******************************************************************************
*
* sysDec2155xIntConnect
*
* This routine finds each of the Motorola MCP750s (Dec 21554 bridges) and
* for each board found, connects an interrupt handler.
*
*/

LOCAL void sysDec2155xIntConnect ()
    {

    int 	index = 0;
    UINT16	subVendId;
    UINT16	subSystId;
    UINT16	command;
    UINT32	busAddr;
    UCHAR	intLine;
    char	*locAddr;
    UINT	bus;
    UINT	dev;
    UINT	func;

    /*
     * For each shared memory participant found, connect an interrupt 
     * service routine.
     */

    while (pciFindDevice(PCI_ID_VEND_DEC21554, PCI_ID_DEV_DEC21554, index++, 
			 &bus, &dev, &func) == OK)
	{

	pciConfigInWord (bus, dev, func, PCI_CFG_SUB_VENDER_ID, &subVendId);
	pciConfigInWord (bus, dev, func, PCI_CFG_SUB_SYSTEM_ID, &subSystId);

        /* If it's a shared memory participant then ... */

        if (sysSmDeviceParticipant (bus, dev, func))

	    {
            pciConfigInWord (bus, dev, func, PCI_CFG_COMMAND, &command);

	    if ((command & PCI_CMD_MASTER_ENABLE) == 0)
		continue;	/* Not configured so don't connect interrupt */

	    pciConfigInLong (bus, dev, func, PCI_CFG_BASE_ADDRESS_0, &busAddr);

	    busAddr &= PCI_MEMBASE_MASK;

	    pciConfigInByte (bus, dev, func, PCI_CFG_DEV_INT_LINE, &intLine);

	    if ( sysBusToLocalAdrs (PCI_SPACE_MEM_SEC, 
				    (char *)busAddr, &locAddr) == ERROR )
		continue;

            (void)pciIntConnect (INUM_TO_IVEC(intLine + INT_NUM_IRQ0), 
		                 sysDec2155xIntr, (UINT32)locAddr);

	    /* Enable PIC interrupt associated with this Dec2155x */

	    sysIntEnablePIC (intLine);
	    }
	}

    }

/*******************************************************************************
*
* sysDec2155xIntr - Field bus interrupts from Dec2155x Primary IRQ sets.
*
* This function is attached to one of the compact PCI bus interrupts.  A
* separate attachment of this function is made for each shared memory
* (Dec21554 chip) which the cPCI master finds by dynamically scannning
* the cPCI bus.  When this function is called due to cPCI bus interrupt
* assertion, it inspects the primary IRQ bits in the Dec21554 associated
* with the board it controls.  If no bits are active, the function just
* returns.  If the shared memory doorbell bit is set, it clears the
* source of the interrupt and then calls the interrupt handler which has
* previously attached to the shared memory doorbell interrupt pseudo
* vector.  If a primary IRQ bit is set but it is NOT the shared memory
* doorbell bit, the source of the interrupt is not cleared prior to
* calling the attached handler.
*
* RETURNS: N/A
*/

LOCAL void sysDec2155xIntr
    (
    UINT32 csrAddr
    )
    {
    UINT16		vecNum;
    UINT16		priIrqActive;
    UINT16		priIrqCurrent;
    UINT16		mask;
    UINT32		intClrAddr;

    /* Find out what primary IRQ bits are actually set */

    intClrAddr = csrAddr + DEC2155X_CSR_PRI_CLR_IRQ;
    priIrqActive = sysPciInWord (intClrAddr);

    /* Invert mask so bits which are set indicate enabled interrupts */

    mask = ~(sysPciInWord (csrAddr + DEC2155X_CSR_PRI_SET_IRQ_MSK));

    /* Isolate only those IRQ "set" bits which are enabled */

    if ( (priIrqActive = (priIrqActive &= mask)) == 0 )
	return;		/* No primary IRQ interrupts for this board */

    /* Examine from doorbell0 to doorbell15 for interrupt */

    mask = 1;

    for (vecNum = DEC2155X_DOORBELL0_INT_VEC;
	 vecNum <= DEC2155X_DOORBELL15_INT_VEC; vecNum++)
	{

	if ( (priIrqCurrent = (priIrqActive & mask)) != 0 )
	    {

	    /* If shared memory interrupt, clear the source */

	    if (vecNum == DEC2155X_SM_DOORBELL_INT_VEC)
	        sysPciOutWordConfirm (intClrAddr, priIrqCurrent);

	    /* Call the chained handlers */
		    
	    pciInt(vecNum);

	    }

        /* advance to next bit */

        mask <<= 1;

	}
    }

/*******************************************************************************
*
* sysIntEnable - enable a Compact PCI bus interrupt level (vector)
*
* This routine enables reception of a specified Compact PCI interrupt level.
*
* If the interrupt level is associated with the Dec2155x primary doorbell
* register, the interrupt is enabled via call to sysDec2155xIntEnable(), else
* an error is returned.
*
* RETURNS: OK if interrupt enabled, ERROR otherwise (invalid interrupt level).
*
* SEE ALSO: sysIntDisable()
*/

STATUS sysIntEnable
    (
    int intLevel        /* interrupt level (vector) */
    )
    {
    if ( (intLevel >= DEC2155X_DOORBELL0_INT_LVL) &&
         (intLevel <= DEC2155X_DOORBELL15_INT_LVL) )
        return (sysDec2155xIntEnable (intLevel));
    else
        return ( ERROR );

    }

/*******************************************************************************
*
* sysDec2155xIntEnable - Enable a Dec2155x internal interrupt
*
* This routine finds all of the shared memory dec2155x chips which are 
* interrupting via primary doorbell bit associated with the intLevel 
* input parameter.  For each interrupting dec2155x so found, the 
* primary clear IRQ mask is set to enable the interrupt.
*
* RETURNS: OK
*
* SEE ALSO: sysDec2155xIntDisable()
*
*/

LOCAL STATUS sysDec2155xIntEnable
    (
    int intLevel       /* interrupt level for interrupt */
    )
    {
    int bit;
    UINT16	subVendId;
    UINT16	subSystId;
    UINT16	command;
    UINT32	busAddr;
    char	*locAddr;
    UINT	bus;
    UINT	dev;
    UINT	func;
    int 	index = 0;

    /* calculate the bit in the Primary Clear IRQ Mask Register */

    bit = intLevel - DEC2155X_DOORBELL0_INT_LVL;

    while (pciFindDevice(PCI_ID_VEND_DEC21554, PCI_ID_DEV_DEC21554, index++,
			 &bus, &dev, &func) == OK)
	{

	pciConfigInWord (bus, dev, func, PCI_CFG_SUB_VENDER_ID, &subVendId);
	pciConfigInWord (bus, dev, func, PCI_CFG_SUB_SYSTEM_ID, &subSystId);

        /* If it's a shared memory participant then ... */

        if (sysSmDeviceParticipant (bus, dev, func))

	    {
            pciConfigInWord (bus, dev, func, PCI_CFG_COMMAND, &command);
	    if ((command & PCI_CMD_MASTER_ENABLE) == 0)
		continue;	/* Not configured so don't enable interrupt */

	    pciConfigInLong (bus, dev, func, PCI_CFG_BASE_ADDRESS_0, &busAddr);

	    busAddr &= PCI_MEMBASE_MASK;

	    if ( sysBusToLocalAdrs (PCI_SPACE_MEM_SEC, 
				    (char *)busAddr, &locAddr) == ERROR )
		continue;

            sysPciOutWord (locAddr +
                           DEC2155X_CSR_PRI_CLR_IRQ_MSK, (UINT16)(1 << bit) );

	    }
	}

    return (OK);
    }

/*******************************************************************************
*
* sysIntDisable - disable a bus interrupt level
*
* This routine disables reception of a specified Compact PCI interrupt level.
*
* If the interrupt level represents one of the primary doorbell interrupts
* (which pull one of the four cPCI interrupt lines) then this routne will
* call sysDec2155xIntDisable() to disable the interrupt source.  Otherwise
* ERROR is returned.
*
* RETURNS: OK if interrupt disabled, ERROR otherwise (invalid interrupt level).
*
* SEE ALSO: sysIntEnable()
*/

STATUS sysIntDisable
    (
    int intLevel        /* interrupt level (vector) */
    )
    {
    if ( (intLevel >= DEC2155X_DOORBELL0_INT_LVL) &&
         (intLevel <= DEC2155X_DOORBELL15_INT_LVL) )
        return (sysDec2155xIntDisable (intLevel));
    else
        return ( ERROR );
    }

/*******************************************************************************
*
* sysDec2155xIntDisable - disable a Dec2155x internal interrupt
*
* This routine finds all of the shared memory dec2155x chips which are 
* interrupting via primary doorbell bit associated with the intLevel 
* input parameter.  For each interrupting dec2155x so found, the primary 
* set IRQ mask is set to disable the interrupt.
*
* RETURNS: OK
*
* SEE ALSO: sysDec2155xIntEnable()
*
*/

LOCAL STATUS sysDec2155xIntDisable
    (
    int intLevel
    )
    {
    int bit;
    UINT16	subVendId;
    UINT16	subSystId;
    UINT16	command;
    UINT32	busAddr;
    char	*locAddr;
    UINT	bus;
    UINT	dev;
    UINT	func;
    int 	index = 0;

    /* calculate the bit in the Primary Set IRQ Mask Register */

    bit = intLevel - DEC2155X_DOORBELL0_INT_LVL;

    /* set the correct bit in the Secondary Set IRQ Mask Register */

    while (pciFindDevice(PCI_ID_VEND_DEC21554, PCI_ID_DEV_DEC21554, index++, 
			 &bus, &dev, &func) == OK)
	{

	pciConfigInWord (bus, dev, func, PCI_CFG_SUB_VENDER_ID, &subVendId);
	pciConfigInWord (bus, dev, func, PCI_CFG_SUB_SYSTEM_ID, &subSystId);

	/* If it's a shared memory participant ... */

        if (sysSmDeviceParticipant (bus, dev, func))

	    {
            pciConfigInWord (bus, dev, func, PCI_CFG_COMMAND, &command);
	    if ((command & PCI_CMD_MASTER_ENABLE) == 0)
		continue;	/* Not configured so don't disable interrupt */

	    pciConfigInLong (bus, dev, func, PCI_CFG_BASE_ADDRESS_0, &busAddr);

	    busAddr &= PCI_MEMBASE_MASK;

	    if ( sysBusToLocalAdrs (PCI_SPACE_MEM_SEC, 
				    (char *)busAddr, &locAddr) == ERROR )
		continue;

            sysPciOutWord (locAddr +
                           DEC2155X_CSR_PRI_SET_IRQ_MSK, (UINT16)(1 << bit) );

	    }
	}

    return (OK);
    }

#endif /* INCLUDE_DEC2155X_SYSTEM_SUPPORT */

#ifndef INCLUDE_DEC2155X_SYSTEM_SUPPORT
/*****************************************************************************
*
* sysIntDisable - disable a bus interrupt level
*
* This routine disables a specified bus interrupt level.
*
* RETURNS: ERROR, always.
*
* ARGSUSED0
*/

STATUS sysIntDisable
    (
    int intLevel	/* interrupt level to disable */
    )
    {
    return (ERROR);
    }


/*******************************************************************************
*
* sysIntEnable - enable a bus interrupt level
*
* This routine enables a specified bus interrupt level.
*
* RETURNS: ERROR, always.
*
* ARGSUSED0
*/

STATUS sysIntEnable
    (
    int intLevel	/* interrupt level to enable */
    )
    {
    return (ERROR);
    }

#endif /* !INCLUDE_DEC2155X_SYSTEM_SUPPORT */

#ifdef INCLUDE_SM_COMMON
/*****************************************************************************
*
* sysSmParamsCompute - Compute dynamic run-time shared memory parameters.
*
* This function computes the shared memory anchor address SM_ANCHOR_ADRS
* (saved in global variable sysSmAnchorAdrs), the SM_INT_ARG1 parameter
* (saved in smIntArg1), and SM_INT_ARG2 (saved in smIntArg2).
*
* RETURNS: NA
*/

LOCAL void sysSmParamsCompute ()
    {

#ifdef INCLUDE_DEC2155X_SYSTEM_SUPPORT

#if (SM_INT_TYPE == SM_INT_BUS)
    smIntArg1 = DEC2155X_DOORBELL0_INT_LVL + DEC2155X_SM_DOORBELL_BIT;
    smIntArg2 = DEC2155X_MAILBOX_INT_VEC;
#endif

#else /* !INCLUDE_DEC2155X_SYSTEM_SUPPORT Not a system board */

    UINT32 pciAddr;
    UINT32 locAddr;

    smIntArg1 = PCI_SPACE_MEM_SEC;

    pciConfigInLong (0, DEC2155X_PCI_DEV_NUMBER, 0,
		     DEC2155X_CFG_SEC_CSR_MEM_BAR, &pciAddr);
    pciAddr += DEC2155X_CSR_SEC_SET_IRQ  + (DEC2155X_SM_DOORBELL_BIT >= 8);
    sysBusToLocalAdrs (PCI_SPACE_MEM_PRI, (char *)pciAddr,
		       (char **)&locAddr);
    sysLocalToBusAdrs (PCI_SPACE_MEM_SEC, (char *)locAddr,
		       (char **)&smIntArg2);


#endif /* !INCLUDE_DEC2155X_SYSTEM_SUPPORT */
    }
#endif	/* INCLUDE_SM_COMMON */

#if (SM_OFF_BOARD == TRUE)

#ifdef SYS_SM_ANCHOR_POLL_LIST
/*****************************************************************************
*
* sysSmAnchorCandidate - Dynamically poll for the anchor address.
*
* This function will determine whether a given bus, device, and function
* number correspond to a defined device on the SYS_SM_ANCHOR_POLL_LIST
*
* RETURNS: TRUE if device is on the SYS_SM_ANCHOR_POLL_LIST
*	   FALSE if device is not on the SYS_SM_ANCHOR_POLL_LIST
*/
LOCAL UINT sysSmAnchorCandidate
    (
    UINT busNo,
    UINT deviceNo,
    UINT funcNo
    )
    {
    UINT i;
    UINT found = FALSE;
    UINT devVend;
    UINT subIdVend;

    for (i = 0 ; (sysSmAnchorPollList[i].devVend != 0xffffffff) ; i++)
	{
	pciConfigInLong (busNo, deviceNo, funcNo, PCI_CFG_VENDOR_ID,
			     &devVend);
	if (devVend == sysSmAnchorPollList[i].devVend)
	    {

	    pciConfigInLong(busNo, deviceNo, funcNo, PCI_CFG_SUB_VENDER_ID,
				&subIdVend);
	    if (subIdVend == sysSmAnchorPollList[i].subIdVend)
		{
		found = TRUE;
		break;
		}
	    }
	}
    return (found);
    }
#endif /* SYS_SM_ANCHOR_POLL_LIST */

/*****************************************************************************
*
* sysSmAnchorFind - Dynamically poll for the anchor address.
*
* This function makes one pass on a polling list of devices in an
* attempt to find the shared memory anchor address.  If
* SYS_SM_ANCHOR_POLL_LIST is defined, the devices on this are searched.
* If SYS_SM_ANCHOR_POLL_LIST is not defined, then all devices on the
* "pciBus" parameter are polled for the existence of the shared memory
* anchor.  The "window" which is searched corresponds to
* PCI_CFG_BASE_ADDRESS_0 and the offset queried is SM_ANCHOR_OFFSET.  If
* a value of SM_READY is found, the shared memory anchor is assumed to
* be at the corresponding address.
*
* RETURNS: OK if SM_READY found, 'anchor' parameter will contain local address.
*	   ERROR if SM_READY not find after one scan of the list.
*/

LOCAL STATUS sysSmAnchorFind
    (
    int pciBus,
    char **anchor
    )
    {
    UINT32 devVend;
    UINT32 memBarVal;
    char   *tryAddr;
    int    deviceNo;
    int    maxDevNo;

#ifdef SYS_SM_ANCHOR_POLL_LIST

    /* if polling list is defined but empty then we're done */

    if (sysSmAnchorPollList[0].devVend != 0xffffffff)
#endif
	{
	if (pciBus == SYS_SM_CPCI_BUS_NUMBER)
	    maxDevNo = 16;
	else
	    maxDevNo = PCI_MAX_DEV ;

	for (deviceNo = 0; deviceNo < maxDevNo; deviceNo++)
	    {
	    if (pciConfigInLong (pciBus, deviceNo, 0, 0, &devVend) != OK)
		continue;

#ifdef SYS_SM_ANCHOR_POLL_LIST
	    if (!sysSmAnchorCandidate (pciBus, deviceNo, 0))
		continue;
#endif

	    if (pciConfigInLong (pciBus, deviceNo, 0,
				 PCI_CFG_BASE_ADDRESS_0, &memBarVal) != OK)
		continue;

	    if (sysBusToLocalAdrs (PCI_SPACE_MEM_SEC,
				   (char *)(memBarVal + SM_ANCHOR_OFFSET),
				   &tryAddr) != OK)
		continue;

	    if (ntohl(*(UINT *)tryAddr) == SM_READY)
		{
		*anchor = tryAddr;
		return (OK);
		}
	    }
	}
    return (ERROR);
    }

/*****************************************************************************
*
* sysSmAnchorPoll - Dynamically poll for the anchor address.
*
* This function will loop and repeatedly call a polling routine to search
* for the shared memrory anchor on the cPCI bus number indicated by
* SYS_SM_CPCI_BUS_NUMBER.  It returns with an anchor address only when the
* anchor is actually found.
*
* RETURNS: Anchor address
*/

LOCAL char *sysSmAnchorPoll()
    {

    char *anchor;
    int  count = 0;

    while (sysSmAnchorFind (SYS_SM_CPCI_BUS_NUMBER, &anchor) != OK)
	{
	if (((++count) % 6) == 0)
	    printf ("sysSmAnchorPoll() searching for shared memory anchor\n");
	sysMsDelay (1000);
	}
    return (anchor);
    }

/*****************************************************************************
*
* sysSmAnchorAdrs - Dynamically compute anchor address
*
* This function is called if SM_OFF_BOARD == TRUE, that is the shared memory
* anchor is not on this node.  If the anchor has been found by a previous
* call to this routine, the previously found anchor value is returned.
* Otherwise  a polling routine is called which will dynamically search for
* the anchor.  This routine is aliased to SM_ANCHOR_ADRS and thus is first
* called when SM_ANCHOR_ADRS is first referenced.
*
* RETURNS: Anchor address
*/

char *sysSmAnchorAdrs()
    {
    static char *anchorAddr;
    static int anchorFound = FALSE;

    if (anchorFound)
	return (anchorAddr);
    anchorAddr = sysSmAnchorPoll ();
    anchorFound = TRUE;
    return (anchorAddr);
    }

#endif /* (SM_OFF_BOARD == TRUE) */

/******************************************************************************
*
* sysNvRead - read one byte from NVRAM
*
* This routine reads a single byte from a specified offset in CMOS block.
* The offset value of zero will result in a read from the first 
* byte in the CMOS block.
*
* RETURNS: The byte from the specified offset.
*/

UCHAR sysNvRead
    (
    ULONG	offset	/* CMOS block offset to read the byte from */
    )
    {
    CMOS_A(offset);
    return (CMOS_D_R);
    }


/******************************************************************************
*
* sysNvWrite - write one byte to NVRAM
*
* This routine writes a single byte to a specified offset in CMOS block.
* The offset value of zero will result in a write to the first 
* byte in the CMOS block.
*
* RETURNS: N/A
*/

void sysNvWrite
    (
    ULONG	offset,	/* NVRAM offset to write the byte to */
    UCHAR	data	/* datum byte */
    )
    {
    CMOS_W(offset, data);
    }
