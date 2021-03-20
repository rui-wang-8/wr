/* dec2155xCcpi.c - DEC 2155X (Drawbridge) PCI-to-PCI bridge library */

/* Copyright 2002 Wind River Systems, Inc. */
/* Copyright 1998-2000 Motorola, Inc. All Rights Reserved */

/*
modification history
--------------------
01f,07may02,dat  Update for T2.2, chg'd SM_NET to SM_COMMON
01e,21mar00,scb  Fix calls do dec2155xMmu() to correctly pass size parameter.
01d,08feb00,scb  Fix INCLUDE_SM_NET scoping.
01c,04feb00,scb  Mods to support shared memory on cpn5360.
01b,30nov99,scb  Ported to cpn5360.
01a,10jun99,rhv  Written (from version 01o of mcpn750/dec2155xCcpi.c).
*/

/*
DESCRIPTION

The routines addressed here include:

Initialization of 2155x chip

Bus interrupt functions:
    - enable/disable Compact PCI interrupts
    - enable/disable drawbridge internal interrupts
    - generate Compact PCI bus interrupts

Mailbox functions:
    - enable mailbox interrupts

Portions of this code (dec2155xInit) must run before the local PCI bus
enumeration been performed. As such, the 2155x BAR registers have not been
initialized and access must be performed though the local PCI configuration
space. This is done using the pciConfigxxxx calls (in pciConfigLib.c). Once
local PCI bus enumeration has completed, access to most 2155x registers will
be performed using the local PCI memory space and sysPcixxxx calls.

*/

/* includes */

#include "vxWorks.h"
#include "config.h"
#include "vxLib.h"
#include "intLib.h"
#include "logLib.h"
#include "dec2155xCpci.h"
#include "pci.h"

/* defines */

#define DEC2155X_ENABLE_MEM_AND_IO (PCI_CMD_IO_ENABLE | PCI_CMD_MEM_ENABLE)

/* this section validates the upstream and downstream window parameters */

#if (DEC2155X_CSR_AND_DS_MEM0_SIZE & (DEC2155X_CSR_AND_DS_MEM0_SIZE - 1))
#    error DEC2155X_CSR_AND_DS_MEM0_SIZE is not a power of 2
#else
#    if (DEC2155X_CSR_AND_DS_MEM0_TRANS & (DEC2155X_CSR_AND_DS_MEM0_SIZE - 1))
#        error DEC2155X_CSR_AND_DS_MEM0_TRANS is not a multiple of window size
#    endif
#    if (DEC2155X_CSR_AND_DS_MEM0_SIZE & DEC2155X_MEM_TB_RSV_MSK)
#        error DEC2155X_CSR_AND_DS_MEM0_SIZE is less than 4KB in size
#    endif
#endif

#if (DEC2155X_DS_IO_OR_MEM1_SIZE & (DEC2155X_DS_IO_OR_MEM1_SIZE - 1))
#    error DEC2155X_DS_IO_OR_MEM1_SIZE is not a power of 2
#else
#    if (DEC2155X_DS_IO_OR_MEM1_TRANS & (DEC2155X_DS_IO_OR_MEM1_SIZE - 1))
#        error DEC2155X_DS_IO_OR_MEM1_TRANS is not a multiple of window size
#    endif
#    if (DEC2155X_DS_IO_OR_MEM1_TYPE & PCI_BAR_SPACE_IO)
#        if (DEC2155X_DS_IO_OR_MEM1_SIZE & DEC2155X_IO_OR_MEM_TB_RSV_MSK)
#            error DEC2155X_DS_IO_OR_MEM1_SIZE is less than 64 bytes
#        endif
#    else
#        if (DEC2155X_DS_IO_OR_MEM1_SIZE & DEC2155X_MEM_TB_RSV_MSK)
#            error DEC2155X_DS_IO_OR_MEM1_SIZE is less than 4KB
#        endif
#    endif        
#endif

#if (DEC2155X_DS_MEM2_SIZE & (DEC2155X_DS_MEM2_SIZE - 1))
#    error DEC2155X_DS_MEM2_SIZE is not a power of 2
#else
#    if (DEC2155X_DS_MEM2_TRANS & (DEC2155X_DS_MEM2_SIZE - 1))
#        error DEC2155X_DS_MEM2_TRANS is not a multiple of window size
#    endif
#    if (DEC2155X_DS_MEM2_SIZE & DEC2155X_MEM_TB_RSV_MSK)
#        error DEC2155X_DS_MEM2_SIZE is less than 4KB in size
#    endif
#endif

#if (DEC2155X_DS_MEM3_SIZE & (DEC2155X_DS_MEM3_SIZE - 1))
#    error DEC2155X_DS_MEM3_SIZE is not a power of 2
#else
#    if (DEC2155X_DS_MEM3_TRANS & (DEC2155X_DS_MEM3_SIZE - 1))
#        error DEC2155X_DS_MEM3_TRANS is not a multiple of window size
#    endif
#    if (DEC2155X_DS_MEM3_SIZE & DEC2155X_MEM_TB_RSV_MSK)
#        error DEC2155X_DS_MEM3_SIZE is less than 4KB in size
#    endif
#endif

#if (DEC2155X_US_IO_OR_MEM0_SIZE & (DEC2155X_US_IO_OR_MEM0_SIZE - 1))
#    error DEC2155X_US_IO_OR_MEM0_SIZE is not a power of 2
#else
#    if (DEC2155X_US_IO_OR_MEM0_TRANS & (DEC2155X_US_IO_OR_MEM0_SIZE - 1))
#        error DEC2155X_US_IO_OR_MEM0_TRANS is not a multiple of window size
#    endif
#    if (DEC2155X_US_IO_OR_MEM0_TYPE & PCI_BAR_SPACE_IO)
#        if (DEC2155X_US_IO_OR_MEM0_SIZE & DEC2155X_IO_OR_MEM_TB_RSV_MSK)
#            error DEC2155X_US_IO_OR_MEM0_SIZE is less than 64 bytes
#        endif
#    else
#        if (DEC2155X_US_IO_OR_MEM0_SIZE & DEC2155X_MEM_TB_RSV_MSK)
#            error DEC2155X_US_IO_OR_MEM0_SIZE is less than 4KB
#        endif
#    endif        
#endif

#if (DEC2155X_US_MEM1_SIZE & (DEC2155X_US_MEM1_SIZE - 1))
#    error DEC2155X_US_MEM1_SIZE is not a power of 2
#else
#    if (DEC2155X_US_MEM1_TRANS & (DEC2155X_US_MEM1_SIZE - 1))
#        error DEC2155X_US_MEM1_TRANS is not a multiple of window size
#    endif
#    if (DEC2155X_US_MEM1_SIZE & DEC2155X_MEM_TB_RSV_MSK)
#        error DEC2155X_US_MEM1_SIZE is less than 4KB in size
#    endif
#endif

/* create window setup values */

#if (DEC2155X_CSR_AND_DS_MEM0_SIZE)
#   define DEC2155X_CSR_AND_DS_MEM0_SETUP ( \
           DEC2155X_SETUP_REG_BAR_ENABLE        | \
           ~(DEC2155X_CSR_AND_DS_MEM0_SIZE - 1) | \
           DEC2155X_CSR_AND_DS_MEM0_TYPE)
#else
#   define DEC2155X_CSR_AND_DS_MEM0_SETUP 0x00000000
#endif

#if (DEC2155X_DS_IO_OR_MEM1_SIZE)
#   define DEC2155X_DS_IO_OR_MEM1_SETUP ( \
           DEC2155X_SETUP_REG_BAR_ENABLE      | \
           ~(DEC2155X_DS_IO_OR_MEM1_SIZE - 1) | \
           DEC2155X_DS_IO_OR_MEM1_TYPE)
#else
#   define DEC2155X_DS_IO_OR_MEM1_SETUP 0x00000000
#endif

#if (DEC2155X_DS_MEM2_SIZE)
#   define DEC2155X_DS_MEM2_SETUP ( \
           DEC2155X_SETUP_REG_BAR_ENABLE | \
           ~(DEC2155X_DS_MEM2_SIZE - 1)  | \
           DEC2155X_DS_MEM2_TYPE)
#else
#   define DEC2155X_DS_MEM2_SETUP 0x00000000
#endif

#if (DEC2155X_DS_MEM3_SIZE)
#   define DEC2155X_DS_MEM3_SETUP ( \
           DEC2155X_SETUP_REG_BAR_ENABLE | \
           ~(DEC2155X_DS_MEM3_SIZE - 1)  | \
           DEC2155X_DS_MEM3_TYPE)
#else
#   define DEC2155X_DS_MEM3_SETUP 0x00000000
#endif

#if (DEC2155X_US_IO_OR_MEM0_SIZE)
#   define DEC2155X_US_IO_OR_MEM0_SETUP ( \
           DEC2155X_SETUP_REG_BAR_ENABLE      | \
           ~(DEC2155X_US_IO_OR_MEM0_SIZE - 1) | \
           DEC2155X_US_IO_OR_MEM0_TYPE)
#else
#   define DEC2155X_US_IO_OR_MEM0_SETUP 0x00000000
#endif

#if (DEC2155X_US_MEM1_SIZE)
#   define DEC2155X_US_MEM1_SETUP ( \
           DEC2155X_SETUP_REG_BAR_ENABLE | \
           ~(DEC2155X_US_MEM1_SIZE - 1)  | \
           DEC2155X_US_MEM1_TYPE)
#else
#   define DEC2155X_US_MEM1_SETUP 0x00000000
#endif

#define DEC2155X_UPR32_DS_MEM3_SETUP 0x00000000

#define DEC2155X_CFG_CHP_STS_ERR_CLR (DEC2155X_CHPSR_US_DLYD_TRNS_MSTR_TO | \
                                      DEC2155X_CHPSR_US_DLYD_RD_TRNS_TO   | \
                                      DEC2155X_CHPSR_US_DLYD_WRT_TRNS_TO  | \
                                      DEC2155X_CHPSR_US_PSTD_WRT_DATA_DISC)

#define DEC2155X_CFG_STS_ERR_CLR (DEC2155X_CFG_STS_DATA_PAR_ERR_DET | \
                                  DEC2155X_CFG_STS_SIG_TGT_ABT      | \
                                  DEC2155X_CFG_STS_RCVD_TGT_ABT     | \
                                  DEC2155X_CFG_STS_RCVD_MSTR_ABT    | \
                                  DEC2155X_CFG_STS_SIG_SYS_ERR      | \
                                  DEC2155X_CFG_STS_DET_PAR_ERR)

#define DEC2155X_MEM_SIZE(setupVal) (~(setupVal & PCI_MEMBASE_MASK) + 1) 

/* structures */

typedef struct windowParameters
    {
    UINT32 windowType; /* mem or i/o */
    UINT32 origBase;   /* as seen from originating bus */
    UINT32 trgtBase;   /* as seen from target bus */
    UINT32 trgtLimit;  /* end of window + 1 */
    } WINDOW_PARAMETERS;

typedef struct windowOffsets
    {
    UINT32 bar;        /* offset to base address register */
    UINT32 trans;      /* offset to translation or opposite base address reg */
    UINT32 setup;      /* offset to setup reg or pseudo-register contents */
    UINT32 unuseable;  /* unuseable space at start of window */
    } WINDOW_OFFSETS;

/*
 * NOTE: The Primary CSR I/O BAR, Secondary CSR Memory BAR and the Secondary
 * CSR I/O BAR do not have corresponding setup registers in the Dec2155x.
 * Also note that the first 4KB of the window mapped by the
 * Primary CSR and Downstream Memory 0 BAR actually references the
 * Dec2155x CSR set. To accommodate these hardware-defined windows,
 * a number of simulated setup registers are present in the offset tables
 * below. They can be recognized by the fact that their values are far
 * to large to be true register offsets. This fact is ensured because
 * an enabled setup register will always have bit 31 set. When the 
 * window mapping code encounters one of large offsets, it simply uses
 * the emulated setup value from the table as if it had been read from
 * the Dec2155x itself. Please bear this in mind should any modifications
 * to the table be required.
 *
 * Also note that there are two table entries for the Primary CSR and
 * Downstream Memory 0 window. This apparent double mapping is done to
 * simplify the translation of addresses which lie within this window.
 * Addresses in the CSR register set (doorbell interrupts, doorbell
 * interrupt masks, etc.) are translated by looking up the address of the
 * register when viewed by the opposite processor while the memory area
 * above the CSR register space is translated using the downstream CSR
 * and Memory 0 translated base register. Even though these two windows
 * overlap for the first 4KB (the size of the CSR register image), no
 * conflict is created because the translation routines always check the
 * CSR space first. If the address resides in the CSR space, the search
 * will never advance to the memory portion of the window and only one
 * translated value is possible. If the address resides above the CSR
 * space, the first mapping window will lie below the address of
 * interest and the corresponding memory 0 mapping will be used to
 * translate the address.
 */

static WINDOW_OFFSETS dsWindowOffsets [] =
    {

    /* first two entries must be at the front of the table and in order */

        {
        DEC2155X_CFG_PRI_CSR_AND_DS_MEM0_BAR+DEC2155X_PRI_FROM_SEC_OFFSET,
        DEC2155X_CFG_SEC_CSR_MEM_BAR,
        (~(DEC2155X_DEFAULT_MEM_WINDOW_SZ - 1) | PCI_BAR_SPACE_MEM),
        0
        },

        {
        DEC2155X_CFG_PRI_CSR_AND_DS_MEM0_BAR+DEC2155X_PRI_FROM_SEC_OFFSET,
        DEC2155X_CFG_DS_MEM0_TB,
        DEC2155X_CFG_DS_MEM0_SETUP,
        DEC2155X_DEFAULT_MEM_WINDOW_SZ
        },

        {
        DEC2155X_CFG_PRI_CSR_IO_BAR+DEC2155X_PRI_FROM_SEC_OFFSET,
        DEC2155X_CFG_SEC_CSR_IO_BAR,
        (~(DEC2155X_DEFAULT_IO_WINDOW_SZ - 1) | PCI_BAR_SPACE_IO),
        0
        },

        {
        DEC2155X_CFG_PRI_DS_IO_OR_MEM1_BAR+DEC2155X_PRI_FROM_SEC_OFFSET,
        DEC2155X_CFG_DS_IO_OR_MEM1_TB,
        DEC2155X_CFG_DS_IO_OR_MEM1_SETUP,
        0
        },

        {
        DEC2155X_CFG_PRI_DS_MEM2_BAR+DEC2155X_PRI_FROM_SEC_OFFSET,
        DEC2155X_CFG_DS_MEM2_TB,
        DEC2155X_CFG_DS_MEM2_SETUP,
        0
        },

        {
        DEC2155X_CFG_PRI_DS_MEM3_BAR+DEC2155X_PRI_FROM_SEC_OFFSET,
        DEC2155X_CFG_DS_MEM3_TB,
        DEC2155X_CFG_DS_MEM3_SETUP,
        0
        },

    };

static WINDOW_OFFSETS usWindowOffsets [] =
    {

        {
        DEC2155X_CFG_SEC_CSR_MEM_BAR,
        DEC2155X_CFG_PRI_CSR_AND_DS_MEM0_BAR+DEC2155X_PRI_FROM_SEC_OFFSET,
        (~(DEC2155X_DEFAULT_MEM_WINDOW_SZ - 1) | PCI_BAR_SPACE_MEM),
        0
        },

        {
        DEC2155X_CFG_SEC_CSR_IO_BAR,
        DEC2155X_CFG_PRI_CSR_IO_BAR+DEC2155X_PRI_FROM_SEC_OFFSET,
        (~(DEC2155X_DEFAULT_IO_WINDOW_SZ - 1) | PCI_BAR_SPACE_IO),
        0
        }, 

        {
        DEC2155X_CFG_SEC_US_IO_OR_MEM0_BAR,
        DEC2155X_CFG_US_IO_OR_MEM0_TB,
        DEC2155X_CFG_US_IO_OR_MEM0_SETUP,
        0
        },

        {
        DEC2155X_CFG_SEC_US_MEM1_BAR,
        DEC2155X_CFG_US_MEM1_TB,
        DEC2155X_CFG_US_MEM1_SETUP,
        0
        }

    };

#define DS_WINDOW_COUNT (sizeof (dsWindowOffsets) / sizeof (WINDOW_OFFSETS))
#define US_WINDOW_COUNT (sizeof (usWindowOffsets) / sizeof (WINDOW_OFFSETS))

/* forward declarations */

#ifdef INCLUDE_SM_COMMON
LOCAL FUNCPTR sysMailboxRoutine  = NULL;
LOCAL int sysMailboxArg          = 0;
LOCAL UINT8 dec2155xIntLine	 = 0xff;
LOCAL BOOL sysMailboxConnected   = FALSE;
#endif

STATUS sysDec2155xIntEnable (int);
STATUS sysDec2155xIntDisable (int);
STATUS sysDec2155xPciInit ();
#ifdef INCLUDE_SM_COMMON
LOCAL void sysMailboxInt (void);
#endif 
#if (PCI_CFG_TYPE==PCI_CFG_NONE)
LOCAL STATUS dec2155xMmu (UINT32, UINT32);
#endif

/* extern declarations */

IMPORT STATUS  sysIntEnablePIC (int);
IMPORT STATUS  sysIntDisablePIC (int);
IMPORT STATUS  pciConfigModifyLong (int, int, int, int, UINT32, UINT32);
IMPORT STATUS  pciConfigModifyWord (int, int, int, int, UINT16, UINT16);
IMPORT STATUS  pciConfigModifyByte (int, int, int, int, UINT8, UINT8);
IMPORT void    sysPciInsertLong (UINT32, UINT32, UINT32);
IMPORT void    sysPciInsertWord (UINT32, UINT16, UINT16);
IMPORT void    sysPciInsertByte (UINT32, UINT8, UINT8);
IMPORT void    sysPciOutWordConfirm (UINT32, UINT16);

/* globals */

UINT32 sysDec2155xCsrAdrs = 0;	/* Base address of dec2155x CSR space */

/*
 * If sysDec2155xInit finds the drawbridge in the wrong state this variable
 * will be set non-zero.
 */

UINT32 sysDec2155xInitFail = 0;

/* Arrays to hold the Dec2155x window translation values */

WINDOW_PARAMETERS dsWindows[DS_WINDOW_COUNT] = {
                                                   {
                                                   0
                                                   }
                                               };

WINDOW_PARAMETERS usWindows[US_WINDOW_COUNT] = {
                                                   {
                                                   0
                                                   }
                                               };

/* Variables to hold the number of Dec2155x windows detected */

UINT32 dsWinsValid = 0;
UINT32 usWinsValid = 0;

/* DEC 2155x Access macros */

#ifndef DEC2155X_CSR_ADRS
#   define DEC2155X_CSR_ADRS(offset) (sysDec2155xCsrAdrs+offset)
#endif

#ifndef DEC2155X_CFG_RD_LONG
#   define DEC2155X_CFG_RD_LONG(offset, result) \
           pciConfigInLong (0, DEC2155X_PCI_DEV_NUMBER, 0, (offset), \
                            (UINT32 *)(result))
#endif

#ifndef DEC2155X_CFG_RD_WORD
#   define DEC2155X_CFG_RD_WORD(offset, result) \
           pciConfigInWord (0, DEC2155X_PCI_DEV_NUMBER, 0, (offset), \
                            (UINT16 *)(result))
#endif

#ifndef DEC2155X_CFG_RD_BYTE
#   define DEC2155X_CFG_RD_BYTE(offset, result) \
           pciConfigInByte (0, DEC2155X_PCI_DEV_NUMBER, 0, (offset), \
                            (UINT8 *)(result))
#endif

#ifndef DEC2155X_CFG_WR_LONG
#   define DEC2155X_CFG_WR_LONG(offset, result) \
           pciConfigOutLong (0, DEC2155X_PCI_DEV_NUMBER, 0, (offset), \
                             (UINT32)(result))
#endif

#ifndef DEC2155X_CFG_WR_WORD  
#   define DEC2155X_CFG_WR_WORD(offset, result) \
           pciConfigOutWord (0, DEC2155X_PCI_DEV_NUMBER, 0, (offset), \
                             (UINT16)(result))
#endif

#ifndef DEC2155X_CFG_WR_BYTE
#   define DEC2155X_CFG_WR_BYTE(offset, result) \
           pciConfigOutByte (0, DEC2155X_PCI_DEV_NUMBER, 0, (offset), \
                             (UINT8)(result))
#endif

#ifndef DEC2155X_CFG_INSERT_LONG
#   define DEC2155X_CFG_INSERT_LONG(offset, mask, data) \
           pciConfigModifyLong (0, DEC2155X_PCI_DEV_NUMBER, 0, (offset), \
                                (mask), (data))
#endif
#ifndef DEC2155X_CFG_INSERT_WORD
#   define DEC2155X_CFG_INSERT_WORD(offset, mask, data) \
           pciConfigModifyWord (0, DEC2155X_PCI_DEV_NUMBER, 0, (offset), \
                                (mask), (data))
#endif
#ifndef DEC2155X_CFG_INSERT_BYTE
#   define DEC2155X_CFG_INSERT_BYTE(offset, mask, data) \
           pciConfigModifyByte (0, DEC2155X_PCI_DEV_NUMBER, 0, (offset), \
	                        (mask), (data))
#endif
#ifndef DEC2155X_PCI_INSERT_LONG
#   define DEC2155X_PCI_INSERT_LONG(offset, mask, data) \
           PCI_INSERT_LONG(DEC2155X_CSR_ADRS(offset), (mask), (data))
#endif

#ifndef DEC2155X_PCI_INSERT_WORD
#   define DEC2155X_PCI_INSERT_WORD(offset, mask, data) \
           PCI_INSERT_WORD(DEC2155X_CSR_ADRS(offset), (mask), (data))
#endif

#ifndef DEC2155X_PCI_INSERT_BYTE
#   define DEC2155X_PCI_INSERT_BYTE(offset, mask, data) \
           PCI_INSERT_BYTE(DEC2155X_CSR_ADRS(offset), (mask), (data))
#endif

/*******************************************************************************
*
* sysDec2155xInit - initialize registers of the Dec2155x (Drawbridge) chip
*
* This routine initializes registers of the DEC 2155x PCI-to-PCI bridge and maps
* access to the Compact PCI bus.
*
* RETURNS: OK on success, else ERROR (Dec2155x not in correct state).
*/

STATUS sysDec2155xInit (void)
    {
    UINT16 priLockOut;
    UINT16 mstrEn;

    /* configure the primary and secondary SERR disables */

    DEC2155X_CFG_INSERT_BYTE(DEC2155X_CFG_PRI_SERR_DISABLES,
                             (UINT8)~DEC2155X_SERR_RSV_MSK,
                             DEC2155X_PRI_SERR_VAL);
    DEC2155X_CFG_INSERT_BYTE(DEC2155X_CFG_SEC_SERR_DISABLES,
                             (UINT8)~DEC2155X_SERR_RSV_MSK,
                             DEC2155X_SEC_SERR_VAL);

    /* configure downstream translation base registers */

    DEC2155X_CFG_INSERT_LONG(DEC2155X_CFG_DS_MEM0_TB,
                             ~DEC2155X_MEM_TB_RSV_MSK,
                             DEC2155X_CSR_AND_DS_MEM0_TRANS);

    DEC2155X_CFG_INSERT_LONG(DEC2155X_CFG_DS_IO_OR_MEM1_TB,
                             ~DEC2155X_IO_OR_MEM_TB_RSV_MSK,
                             DEC2155X_DS_IO_OR_MEM1_TRANS);

    DEC2155X_CFG_INSERT_LONG(DEC2155X_CFG_DS_MEM2_TB,
                             ~DEC2155X_MEM_TB_RSV_MSK,
                             DEC2155X_DS_MEM2_TRANS);

    DEC2155X_CFG_INSERT_LONG(DEC2155X_CFG_DS_MEM3_TB,
                             ~DEC2155X_MEM_TB_RSV_MSK,
                             DEC2155X_DS_MEM3_TRANS);

    /* configure upstream translation base values */

    DEC2155X_CFG_INSERT_LONG(DEC2155X_CFG_US_IO_OR_MEM0_TB,
                             ~DEC2155X_IO_OR_MEM_TB_RSV_MSK,
                             DEC2155X_US_IO_OR_MEM0_TRANS);

    DEC2155X_CFG_INSERT_LONG(DEC2155X_CFG_US_MEM1_TB,
                             ~DEC2155X_MEM_TB_RSV_MSK,
                             DEC2155X_US_MEM1_TRANS);

    /* get current state of Primary Lock-out and Master Enable bits */

    DEC2155X_CFG_RD_WORD(DEC2155X_CFG_CHP_CTRL0, &priLockOut);
    priLockOut &= DEC2155X_CC0_PRI_ACCESS_LKOUT;

    DEC2155X_CFG_RD_WORD(PCI_CFG_COMMAND, &mstrEn);
    mstrEn &= PCI_CMD_MASTER_ENABLE;

    /* verify ROM boot phase */

    if (!mstrEn)
        {

        /* in ROM boot phase, OK to continue */

#ifdef DEC2155X_SETUP_FROM_SROM

       /*
        * primary lock-out bit should not be set if ROM contents are OK.
        * if primary lock-out bit is set, indicate error.
        */

       if (priLockOut)
           sysDec2155xInitFail = TRUE;

#else /* !DEC2155X_SETUP_FROM_SROM */
       
       /*
        * primary lock-out bit should be set if ROM contents are OK.
        * if primary lock-out bit is not set, indicate error.
        */

       if (!priLockOut)

           sysDec2155xInitFail = TRUE;

       else
            {

            /* configure primary class register */

            DEC2155X_CFG_WR_BYTE((PCI_CFG_PROGRAMMING_IF +
                                 DEC2155X_PRI_FROM_SEC_OFFSET),
                                 DEC2155X_PRI_PRG_IF_VAL);
            DEC2155X_CFG_WR_BYTE((PCI_CFG_SUBCLASS +
                                 DEC2155X_PRI_FROM_SEC_OFFSET),
                                 DEC2155X_PRI_SUBCLASS_VAL);
            DEC2155X_CFG_WR_BYTE((PCI_CFG_CLASS +
                                 DEC2155X_PRI_FROM_SEC_OFFSET),
                                 DEC2155X_PRI_CLASS_VAL);

            /* configure bist register (shared) */

            DEC2155X_CFG_INSERT_BYTE(PCI_CFG_BIST, ~DEC2155X_BIST_RSV,
                                     DEC2155X_BIST_VAL);
            
            /* configure the downstream windows */

            DEC2155X_CFG_INSERT_LONG(DEC2155X_CFG_DS_MEM0_SETUP,
                                     ~DEC2155X_MEM_SETUP_RSV_MSK,
                                     DEC2155X_CSR_AND_DS_MEM0_SETUP);

            DEC2155X_CFG_INSERT_LONG(DEC2155X_CFG_DS_IO_OR_MEM1_SETUP,
                                     ~DEC2155X_IO_OR_MEM_SETUP_RSV_MSK,
                                     DEC2155X_DS_IO_OR_MEM1_SETUP);

            DEC2155X_CFG_INSERT_LONG(DEC2155X_CFG_DS_MEM2_SETUP,
                                     ~DEC2155X_MEM_SETUP_RSV_MSK,
                                     DEC2155X_DS_MEM2_SETUP);

            DEC2155X_CFG_INSERT_LONG(DEC2155X_CFG_DS_MEM3_SETUP,
                                     ~DEC2155X_MEM_SETUP_RSV_MSK,
                                     DEC2155X_DS_MEM3_SETUP);

            DEC2155X_CFG_WR_LONG(DEC2155X_CFG_UPR32_BITS_DS_MEM3_SU,
			         DEC2155X_UPR32_DS_MEM3_SETUP);

            /* configure subsystem and vendor id (shared) */

            DEC2155X_CFG_WR_WORD(PCI_CFG_SUB_VENDER_ID,
                                 DEC2155X_SUB_VNDR_ID_VAL);
            DEC2155X_CFG_WR_WORD(PCI_CFG_SUB_SYSTEM_ID,
                                 DEC2155X_SUB_SYS_ID_VAL);

            /* configure the primary max latency and minimum grant registers */

            DEC2155X_CFG_WR_BYTE(PCI_CFG_MAX_LATENCY + 
                                 DEC2155X_PRI_FROM_SEC_OFFSET,
                                 DEC2155X_MAX_LAT_VAL);

            DEC2155X_CFG_WR_BYTE(PCI_CFG_MIN_GRANT +
                                 DEC2155X_PRI_FROM_SEC_OFFSET,
                                 DEC2155X_MIN_GNT_VAL);

            /* configure chip ctrl 0 without altering primary lock-out */

            DEC2155X_CFG_INSERT_WORD(DEC2155X_CFG_CHP_CTRL0,
                                     ~DEC2155X_CC0_PRI_ACCESS_LKOUT,
                                     DEC2155X_CHP_CTRL0_VAL);

            /* configure chip control 1 */

            DEC2155X_CFG_WR_WORD(DEC2155X_CFG_CHP_CTRL1,
                                 DEC2155X_CHP_CTRL1_VAL);

            /*
             * set secondary master access enable so we're ready when the host
             * sets the I/O and/or Memory access enables on the primary side
             */

            DEC2155X_CFG_INSERT_WORD(PCI_CFG_COMMAND, PCI_CMD_MASTER_ENABLE,
                                     PCI_CMD_MASTER_ENABLE);

            /* allow host access from the primary side */

            DEC2155X_CFG_INSERT_WORD(DEC2155X_CFG_CHP_CTRL0,
                                     DEC2155X_CC0_PRI_ACCESS_LKOUT,
                                     DEC2155X_CHP_CTRL0_VAL);

            /* configure secondary class register */

            DEC2155X_CFG_WR_BYTE(PCI_CFG_PROGRAMMING_IF,
                                  DEC2155X_SEC_PRG_IF_VAL);
            DEC2155X_CFG_WR_BYTE(PCI_CFG_SUBCLASS,
                                 DEC2155X_SEC_SUBCLASS_VAL);
            DEC2155X_CFG_WR_BYTE(PCI_CFG_CLASS,
                                 DEC2155X_SEC_CLASS_VAL);

            /* configure upstream windows */

            DEC2155X_CFG_INSERT_LONG(DEC2155X_CFG_US_IO_OR_MEM0_SETUP,
                                     ~DEC2155X_IO_OR_MEM_SETUP_RSV_MSK,
                                     DEC2155X_US_IO_OR_MEM0_SETUP);

            DEC2155X_CFG_INSERT_LONG(DEC2155X_CFG_US_MEM1_SETUP,
                                     ~DEC2155X_MEM_SETUP_RSV_MSK,
                                     DEC2155X_US_MEM1_SETUP);

            /* configure secondary max latency and minimum grant */

            DEC2155X_CFG_WR_BYTE(PCI_CFG_MAX_LATENCY, DEC2155X_MAX_LAT_VAL);
            DEC2155X_CFG_WR_BYTE(PCI_CFG_MIN_GRANT, DEC2155X_MIN_GNT_VAL);

	    /* configure secondary (local) bus arbiter */

	    DEC2155X_CFG_INSERT_WORD(DEC2155X_CFG_ARB_CTRL,
                                     DEC2155X_ARB_CTRL_VAL,
				     DEC2155X_ARB_CTRL_MSK);

            }

#endif /* DEC2155X_SETUP_FROM_SROM */
        }

    /* return status based on error flag */

    if (sysDec2155xInitFail)
        return (ERROR);
    else
        return (OK);

    }

/*******************************************************************************
*
* sysDec2155xInit2 - perform phase 2 Dec2155x initialization
*
* This function disables Dec2155x interrupts and connects an interrupt
* handler.  It must be executed after PCI space has been configured. 
*
* RETURNS: N/A
*/

void sysDec2155xInit2 (void)
    {

    /* read the Dec2155x CSR Memory BAR, convert to CPU view and save */

    DEC2155X_CFG_RD_LONG(DEC2155X_CFG_SEC_CSR_MEM_BAR, &sysDec2155xCsrAdrs);

    sysDec2155xCsrAdrs = (sysDec2155xCsrAdrs & PCI_MEMBASE_MASK) -
                          PCI_MSTR_MEMIO_BUS + PCI_MSTR_MEMIO_LOCAL;

    /* disable and clear all in-bound doorbell interrupts. */

    sysPciOutWord (DEC2155X_CSR_ADRS(DEC2155X_CSR_SEC_SET_IRQ_MSK),
                   (UINT16)(0xffff));
    sysPciOutWord (DEC2155X_CSR_ADRS(DEC2155X_CSR_SEC_CLR_IRQ),
                   (UINT16)(0xffff));

    /* disable and clear power state interrupt. */

    sysPciOutWord (DEC2155X_CSR_ADRS(DEC2155X_CSR_CHP_SET_IRQ_MSK),
                   DEC2155X_CHPCSR_PM_D0);
    sysPciOutWord (DEC2155X_CSR_ADRS(DEC2155X_CSR_CHP_STS_CSR),
                   DEC2155X_CHPCSR_PM_D0);

    /* disable I2O in-bound list empty interrupt. */

    DEC2155X_PCI_INSERT_LONG(DEC2155X_CSR_I2O_IBND_PST_LST_MSK,
                             DEC2155X_I2O_PST_LST_MSK,
                             DEC2155X_I2O_PST_LST_MSK);

    /* disable and clear Upstream memory 2 page crossing interrupts. */

    sysPciOutLong (DEC2155X_CSR_ADRS(DEC2155X_CSR_US_PG_BND_IRQ_MSK0),
                   (UINT32)(0xffffffff));
    sysPciOutLong (DEC2155X_CSR_ADRS(DEC2155X_CSR_US_PG_BND_IRQ_MSK1),
                   (UINT32)(0xffffffff));
    sysPciOutLong (DEC2155X_CSR_ADRS(DEC2155X_CSR_US_PG_BND_IRQ0),
                   (UINT32)(0xffffffff));
    sysPciOutLong (DEC2155X_CSR_ADRS(DEC2155X_CSR_US_PG_BND_IRQ1),
                   (UINT32)(0xffffffff));

#ifdef INCLUDE_SM_COMMON
    /* connect the mailbox interrupt handler */

    DEC2155X_CFG_RD_BYTE(PCI_CFG_DEV_INT_LINE, &dec2155xIntLine);

    if (pciIntConnect (INUM_TO_IVEC (dec2155xIntLine + INT_NUM_IRQ0),
                   sysMailboxInt, 0) == OK);
	{
        sysMailboxConnected = TRUE;
	}
#endif

    }

#ifdef INCLUDE_SM_COMMON
/*******************************************************************************
*
* sysBusIntAck - acknowledge a bus interrupt
*
* This routine acknowledges a specified Compact PCI bus interrupt level.
*
* NOTE: This routine is included for BSP compliance only.  Since Compact
* PCI bus interrupts are routed directly to the interrupt controller, interrupts
* are re-enabled in the interrupt controller's handler and this routine is a
* no-op.
*
* RETURNS: NULL.
*
* SEE ALSO: sysBusIntGen()
*/

int sysBusIntAck
    (
    int intLevel        /* interrupt level to acknowledge */
    )
    {
    return (0);
    }

/*******************************************************************************
*
* sysBusIntGen - generate a bus interrupt
*
* This routine generates a Compact PCI backpanel interrupt by setting one of
* the 16 bits in the 2155x Primary Set IRQ register. The bit is set regardless
* of the current state of the bit and whether the interrupt has been enabled by
* the host processor. Because the Compact PCI bus does not have interrupt
* levels or vectors, the caller's interrupt level parameter is ignored and the
* caller's vector number is used to select the bit to set in the Primary Set IRQ
* Register. The symbol DEC2155X_DOORBELL0_INT_VEC corresponds to bit 0 (LSB)
* and the remaining bits are mapped in sequence.
*
* RETURNS: OK, or ERROR if <vector> is out of range.
*
* SEE ALSO: sysBusIntAck()
*/

STATUS sysBusIntGen
    (
    int level,          /* interrupt level to generate (not used) */
    int vector          /* interrupt vector for interrupt */
    )
    {
    int bit;

    /* make sure the interrupt vector is valid */

    if ( (vector > DEC2155X_DOORBELL15_INT_VEC) ||
         (vector < DEC2155X_DOORBELL0_INT_VEC) )
        return (ERROR);

    /* calculate the bit in the Primary Set IRQ Register */

    bit = vector - DEC2155X_DOORBELL0_INT_VEC;

    /* set the correct bit in the Primary Set IRQ Register */

    sysPciOutWord (DEC2155X_CSR_ADRS(DEC2155X_CSR_PRI_SET_IRQ),
                   (UINT16)(1 << bit));

    return (OK);
    }

/*******************************************************************************
*
* sysMailboxConnect - connect a routine to the mailbox interrupt
*
* This routine specifies the interrupt service routine to be called at each
* mailbox interrupt.
*
* NOTE: The mailbox interrupt is DEC2155X_MAILBOX_INT_VEC.
*
* RETURNS: OK, or ERROR if the routine cannot be connected to the interrupt.
*
* SEE ALSO: intConnect(), sysMailboxEnable()
*/

STATUS sysMailboxConnect
    (
    FUNCPTR routine,    /* routine called at each mailbox interrupt */
    int arg             /* argument with which to call routine      */
    )
    {
    int bit;

    if (!sysMailboxConnected &&
        pciIntConnect (INUM_TO_IVEC (dec2155xIntLine + INT_NUM_IRQ0),
                        sysMailboxInt, 0) == ERROR)
        {
        return (ERROR);
        }

    sysMailboxConnected = TRUE;
    sysMailboxRoutine   = routine;
    sysMailboxArg       = arg;

    /*
     * clear the mailbox intr by clearing the correct bit in the Secondary
     * Clear IRQ Register
     */

    bit = DEC2155X_SM_DOORBELL_BIT;

    sysPciOutWord (DEC2155X_CSR_ADRS(DEC2155X_CSR_SEC_CLR_IRQ),
                   (UINT16)(1 << bit));

    sysIntEnablePIC (dec2155xIntLine);
    return (OK);
    }

/*******************************************************************************
*
* sysMailboxEnable - enable the mailbox interrupt
*
* This routine enables the mailbox interrupt.
*
* NOTE: The mailbox interrupt is DEC2155X_MAILBOX_INT_VEC.
*
* RETURNS: OK, always.
*
* SEE ALSO: sysMailboxConnect(), sysMailboxDisable()
*/

STATUS sysMailboxEnable
    (
    char *mailboxAdrs           /* address of mailbox (ignored) */
    )
    {

    return (sysDec2155xIntEnable (DEC2155X_MAILBOX_INT_VEC));

    }

/*******************************************************************************
*
* sysMailboxDisable - disable the mailbox interrupt
*
* This routine disables the mailbox interrupt.
*
* NOTE: The mailbox interrupt is DEC2155X_MAILBOX_INT_VEC.
*
* RETURNS: OK, always.
*
* SEE ALSO: sysMailboxConnect(), sysMailboxEnable()
*/

STATUS sysMailboxDisable
    (
    char *mailboxAdrs           /* address of mailbox (ignored) */
    )
    {
    int bit;

    /* calculate the bit in the register */

    bit = DEC2155X_SM_DOORBELL_BIT;

    /*
     * clear any pending interrupt by writing a one to the proper bit in the
     * the Secondary Clear IRQ Register
     */

    sysPciOutWord (DEC2155X_CSR_ADRS(DEC2155X_CSR_SEC_CLR_IRQ),
                   (UINT16)(1 << bit));

    /* disable the interrupt */

    return (sysDec2155xIntDisable (DEC2155X_MAILBOX_INT_VEC));
    }

/*******************************************************************************
*
* sysMailboxInt - mailbox interrupt handler
*
* This function clears the doorbell interrupt and calls the associated
* mailbox routine if it exists.
*/

LOCAL void sysMailboxInt (void)
    {

    UINT16		status;
    UINT16              mask;
    UINT32		intMskAdrs;
    UINT32		intClrAdrs;

    /* check for secondary doorbell interrupts */

    intMskAdrs = DEC2155X_CSR_ADRS(DEC2155X_CSR_SEC_SET_IRQ_MSK);
    intClrAdrs = DEC2155X_CSR_ADRS(DEC2155X_CSR_SEC_CLR_IRQ);

    mask = ~(sysPciInWord (intMskAdrs));
    status = sysPciInWord (intClrAdrs);

    /* if mailbox bit is active and enabled ... */

    if (status &= (mask & (1 << DEC2155X_SM_DOORBELL_BIT)))
        {

        /* clear interrupt source */

        sysPciOutWordConfirm (intClrAdrs, status);

	/* Call the mailbox routine, if any */

        if (sysMailboxRoutine != NULL)
            sysMailboxRoutine (sysMailboxArg);

        }

    }
#endif /* INCLUDE_SM_COMMON */

/*******************************************************************************
*
* sysDec2155xIntDisable - disable a Dec2155x internal interrupt
*
* This routine disables the specified internal Dec2155x interrupt.
*
* RETURNS: OK, or ERROR if invalid interrupt vector.
*
* SEE ALSO: sysDec2155xIntEnable()
*
*/

STATUS sysDec2155xIntDisable
    (
    int vector          /* interrupt vector for interrupt */
    )
    {
    int bit;

    /* check for doorbell interrupt */

    if ( (vector >= DEC2155X_DOORBELL0_INT_VEC) && 
         (vector <= DEC2155X_DOORBELL15_INT_VEC) )
        {
        /* calculate the bit in the Secondary Set IRQ Mask Register */

        bit = vector - DEC2155X_DOORBELL0_INT_VEC;

        /* set the correct bit in the Secondary Set IRQ Mask Register */

        sysPciOutWord (DEC2155X_CSR_ADRS(DEC2155X_CSR_SEC_SET_IRQ_MSK),
                       (UINT16)(1 << bit));
        }
    else
        {
        switch (vector)
            {
            case DEC2155X_PWR_MGMT_INT_LVL:
                {
                sysPciOutWord (DEC2155X_CSR_ADRS(DEC2155X_CSR_CHP_SET_IRQ_MSK),
                               DEC2155X_CHPCSR_PM_D0);
                break;
                }
            case DEC2155X_I2O_INT_LVL:
                {
                DEC2155X_PCI_INSERT_LONG(DEC2155X_CSR_I2O_IBND_PST_LST_MSK,
                                         DEC2155X_I2O_PST_LST_MSK,
                                         DEC2155X_I2O_PST_LST_MSK);
                break;
                }
            case DEC2155X_PG_CRSSNG_INT_LVL:
                {
                sysPciOutLong (DEC2155X_CSR_ADRS(
                               DEC2155X_CSR_US_PG_BND_IRQ_MSK0),
                               (UINT32)(0xffffffff));
                sysPciOutLong (DEC2155X_CSR_ADRS(
                               DEC2155X_CSR_US_PG_BND_IRQ_MSK1),
                               (UINT32)(0xffffffff));
                break;
                }
            default:
                {
                return (ERROR);
                break;
                }
            }
        }

    return (OK);
    }

/*******************************************************************************
*
* sysDec2155xIntEnable - Enable a Dec2155x internal interrupt
*
* This routine enables the specified internal Dec2155x interrupt.
*
* RETURNS: OK, or ERROR if invalid interrupt vector.
*
* SEE ALSO: sysDec2155xIntDisable()
*
*/

STATUS sysDec2155xIntEnable
    (
    int vector          /* interrupt vector for interrupt */
    )
    {
    int bit;

    /* check for doorbell interrupt */

    if ( (vector >= DEC2155X_DOORBELL0_INT_VEC) &&
         (vector <= DEC2155X_DOORBELL15_INT_VEC) )
        {
        /* calculate the bit in the Secondary Clear IRQ Mask Register */

        bit = vector - DEC2155X_DOORBELL0_INT_VEC;

        /* set the correct bit in the Secondary Clear IRQ Mask Register */

        sysPciOutWord (DEC2155X_CSR_ADRS(DEC2155X_CSR_SEC_CLR_IRQ_MSK),
                       (UINT16)(1 << bit));
        }
    else
        {
        switch (vector)
            {
            case DEC2155X_PWR_MGMT_INT_LVL:
                {
                sysPciOutWord (DEC2155X_CSR_ADRS(DEC2155X_CSR_CHP_CLR_IRQ_MSK),
                               DEC2155X_CHPCSR_PM_D0);
                break;
                }
            case DEC2155X_I2O_INT_LVL:
                {
                DEC2155X_PCI_INSERT_LONG(DEC2155X_CSR_I2O_IBND_PST_LST_MSK,
                                         DEC2155X_I2O_PST_LST_MSK,
                                         ~DEC2155X_I2O_PST_LST_MSK);
                break;
                }
            case DEC2155X_PG_CRSSNG_INT_LVL:
                {
                sysPciOutLong (DEC2155X_CSR_ADRS(
                               DEC2155X_CSR_US_PG_BND_IRQ_MSK0),
                               (UINT32)(0x00000000));
                sysPciOutLong (DEC2155X_CSR_ADRS(
                               DEC2155X_CSR_US_PG_BND_IRQ_MSK1),
                               (UINT32)(0x00000000));
                break;
                }
            default:
                {
                return (ERROR);
                break;
                }
            }
        }

    return (OK);
    }

/*******************************************************************************
*
* sysDec2155xErrClr - Dec2155x Error Clear routine
*
* This is the Error clear routine for the Dec21554x PCI to PCI Bridge. The
* following error bits are cleared:
*
* Secondary Status Register:
* Data Parity Detected
* Signaled Target Abort
* Received Target Abort
* Received Master Abort
* Signaled System Error
* Detected Parity Error
*
* Primary Status Register:
* Received Master Abort
*
* Chip Status Register:
* Upstream Delayed Transaction Master Timeout
* Upstream Delayed Read Transaction Discarded
* Upstream Delayed Write Transaction Discarded
* Upstream Posted Write Data Discarded
*
* RETURNS: N/A
*/

void sysDec2155xErrClr (void)
    {

    DEC2155X_CFG_WR_WORD(PCI_CFG_STATUS, DEC2155X_CFG_STS_ERR_CLR);

    DEC2155X_CFG_WR_WORD(PCI_CFG_STATUS + DEC2155X_PRI_FROM_SEC_OFFSET,
                         DEC2155X_CFG_STS_RCVD_MSTR_ABT);

    /* need to use insert routine due to reserved bits in register */

    DEC2155X_CFG_INSERT_WORD(DEC2155X_CFG_CHP_STATUS,
                             DEC2155X_CFG_CHP_STS_ERR_CLR,
                             DEC2155X_CFG_CHP_STS_ERR_CLR);

    }

/******************************************************************************
*
* sysDec2155xCnvrtAdrs - converts an address across the Dec2155x
*
* This routine examines the caller-provided array of Dec2155x windows looking
* for a window which maps the target address. If such a window is found, the
* caller's address is converted to the equivalent address as it would appear
* on the opposite bus.
*
* RETURNS: OK if conversion successful, else ERROR.
*
* SEE ALSO: sysLocalToBusAdrs(), sysBusToLocalAdrs(), sysDec2155xCnvrtAdrs()
*/

LOCAL STATUS sysDec2155xCnvrtAdrs
    (
    UINT32 adrsSpace,
    UINT32 adrs,
    UINT32 *pCnvrtAdrs,
    UINT32 winCnt,
    WINDOW_PARAMETERS *pWinParam
    )
    {
    while (winCnt--)
        {
        if ( (pWinParam->windowType == adrsSpace) &&
             (adrs >= pWinParam->trgtBase) &&
             (adrs <= pWinParam->trgtLimit) )
            {
            *pCnvrtAdrs = ( adrs - pWinParam->trgtBase + pWinParam->origBase );

            return (OK);
            };

        pWinParam++;
        }

    /*
     * no window was found to contain adrs. indicate that translation was
     * not possible.
     */

    return (ERROR);
    }

/******************************************************************************
*
* sysDec2155xCaptWindows - captures Dec2155x window configuration
*
* This routine searches through either the upstream or downstream Dec2155x
* windows and captures the information required by the sysDec2155xCnvrtAdrs
* routine. 
*
* NOTE: This routine should not be called until the responsible processor has
* enumerated the PCI bus. This is assured by checking the status of the
* enable bits in the Primary and Secondary PCI command registers.
*
* RETURNS: N/A
*
* SEE ALSO: sysLocalToBusAdrs(), sysBusToLocalAdrs(), sysDec2155xCnvrtAdrs()
*/

LOCAL void sysDec2155xCaptWindows 
    (
    UINT32 winCnt,
    WINDOW_OFFSETS *pWinOffsets,
    WINDOW_PARAMETERS *pWinParam,
    UINT32 *winsValid
    )
    {
    UINT32 bar;   /* holds contents of base address register */
    UINT32 mask;  /* holds mask used to isolate valid bits translation reg */
    UINT32 trans; /* value used to convert between address spaces */

    *winsValid = 0;

    while (winCnt--)
        {

        /* read setup register offset from the offset table. */

        mask  = pWinOffsets->setup;

        /*
         * if the setup value from the table is an offset, then read the setup
         * register. if the value from the table is a default setup value
         * (indicates no setup register for this bar), then just use it as is.
         */

        if ( !(mask & DEC2155X_SETUP_REG_BAR_ENABLE) )
            DEC2155X_CFG_RD_LONG(mask, &mask);

        /* see if the register is enabled */

        if ( mask & DEC2155X_SETUP_REG_BAR_ENABLE )
            {

            /* 
             * determine which PCI space the register is in (memory or i/o).
             * and, setup the window type. adjust the mask to eliminate
             * reserved bits from the setup register.
             */

            if ( (mask & PCI_BAR_SPACE_MASK) == PCI_BAR_SPACE_MEM )
                {
                pWinParam->windowType = PCI_BAR_SPACE_MEM;
                mask &= ~DEC2155X_MEM_TB_RSV_MSK;
                }
            else
               {
                pWinParam->windowType = PCI_BAR_SPACE_IO;
                mask &= ~DEC2155X_IO_OR_MEM_TB_RSV_MSK;
               }

            /* read the remaining register contents */

            DEC2155X_CFG_RD_LONG(pWinOffsets->bar, &bar);
            DEC2155X_CFG_RD_LONG(pWinOffsets->trans, &trans);

           /* remove unused bits */

            bar   &= mask;
            trans &= mask;

            /*
             * save the base address of the window as seen from the originating
             * and target sides. calculate limit of the window as seen from
             * the target side.
             */
 
            pWinParam->origBase = bar + pWinOffsets->unuseable; 
            pWinParam->trgtBase = trans + pWinOffsets->unuseable;
            pWinParam->trgtLimit = trans + ~mask;

            if ( pWinParam->trgtLimit > pWinParam->trgtBase )
               {
               (*winsValid)++;
               pWinParam++;
               }

            /* advance to next window */

            pWinOffsets++;
            }
        else
            {

            /*
             * pci base address registers must be contiguous, first disabled
             * register terminates the loop.
             */

            break;

            }
        }
    }

/******************************************************************************
*
* sysDec2155xChkEnables - check the originating and target bus enables
*
* This routine examines the states of the I/O or Memory enables on the
* originating bus and the Master Enable on the target bus. Both must be
* enabled before the Dec2155x can pass a transaction across to the opposite
* bus.
*
* RETURNS: OK if the bridge is active, else ERROR
*
* SEE ALSO: sysLocalToBusAdrs(), sysBusToLocalAdrs(), sysDec2155xCnvrtAdrs()
*/

STATUS sysDec2155xChkEnables
    (UINT32 origOffset,
     UINT32 trgtOffset,
     UINT32 adrsSpace
     )
     {
    UINT16 cmdReg;
    UINT16 mask;

    /* first determine if the required originating side enable is active */

    switch (adrsSpace)
        {
        case PCI_BAR_SPACE_MEM:
            mask = PCI_CMD_MEM_ENABLE;
            break;

        case PCI_BAR_SPACE_IO:
            mask = PCI_CMD_IO_ENABLE;
            break;

        default:
            mask = 0;
            break;
        }

    /* read the PCI command register as seen from the originating side */

    DEC2155X_CFG_RD_WORD(origOffset, &cmdReg);

    if ( !(cmdReg & mask) )
        return (ERROR); /* access not enabled */

    /* now determine if the target side bus master enable is set */

    DEC2155X_CFG_RD_WORD(trgtOffset, &cmdReg);

    if (cmdReg & PCI_CMD_MASTER_ENABLE)
        return (OK);
    else
        return (ERROR);
    }
    
/******************************************************************************
*
* sysPciToCpciAdrs - convert a local PCI address to a Compact PCI bus address
*
* Given a local PCI address, this routine returns the corresponding Compact PCI
* address provided that such an address exists. The translation is performed by
* examining downstream windows in sequence until a window is found which
* contains the specified local address. The translation is then performed using 
* the base addresses of the window as viewed from the local PCI and Compact PCI
* bus sides.
*
*
* RETURNS: OK, or ERROR if the address space is unknown or the mapping is not
* possible.
*
* SEE ALSO: sysCpciToPciAdrs(), sysDec2155xCaptWindows(),
*           sysDec2155xCnvrtAdrs()
*/

LOCAL STATUS sysPciToCpciAdrs
    (
    int     adrsSpace,     /* bus address space where busAdrs resides */
    char *  localAdrs,     /* bus address to convert */
    char ** pBusAdrs       /* where to return bus address */
    )
    {

    /* 
     * before a downstream window can be operational, the host processor must
     * have enabled the dec2155x memory and/or i/o access in the PCI primary
     * command register and the local processor must have enabled the bus
     * master bit in the secondary PCI command register. before proceeding,
     * verify that the required access is enabled
     */

    if ( sysDec2155xChkEnables (PCI_CFG_COMMAND + DEC2155X_PRI_FROM_SEC_OFFSET,
                                 PCI_CFG_COMMAND, adrsSpace) != OK )
        return (ERROR);

    /* if the downstream windows array is un-initialized, go initialize it */

    if (dsWindows[0].trgtLimit == 0)
        sysDec2155xCaptWindows (DS_WINDOW_COUNT, &dsWindowOffsets[0],
                                &dsWindows[0], &dsWinsValid);

    /* convert a local pci address to a compact pci address */

    return ( sysDec2155xCnvrtAdrs (adrsSpace, (UINT32)localAdrs,
                                   (UINT32 *)pBusAdrs, dsWinsValid,
                                   &dsWindows[0]) );

    }

/******************************************************************************
*
* sysCpciToPciAdrs - convert a compact pci bus address to a local pci address
*
* Given a Compact PCI address, this routine returns the corresponding local PCI
* address provided that such an address exists. The translation is performed by
* examining the upstream windows in sequence until a window is found which
* contains the specified Compact PCI bus address. The translation is then
* performed using the base addresses of the window as viewed from the bus and
* local sides. 
*
* RETURNS: OK, or ERROR if the address space is unknown or the mapping is not
* possible.
*
* SEE ALSO: sysPciToCpciAdrs(), sysDec2155xCaptWindows(),
*           sysDec2155xCnvrtAdrs()
*/

LOCAL STATUS sysCpciToPciAdrs
    (
    int     adrsSpace,      /* bus address space where busAdrs resides */
    char *  busAdrs,        /* bus address to convert */
    char ** pLocalAdrs      /* where to return local address */
    )
    {

    /* 
     * before an upstream window can be operational, the local processor must
     * have enabled the dec2155x memory and/or i/o spaces in the PCI secondary
     * command register and the host processor must have enabled the bus
     * master bit in the primary PCI command register. before proceeding
     * verify that the required access is enabled.
     */

    if ( sysDec2155xChkEnables (PCI_CFG_COMMAND,
                                PCI_CFG_COMMAND + DEC2155X_PRI_FROM_SEC_OFFSET,
                                adrsSpace) != OK )
        return (ERROR);

    /* if the upstream windows array is un-initialized, go initialize it */

    if (usWindows[0].trgtLimit == 0)
        sysDec2155xCaptWindows (US_WINDOW_COUNT, &usWindowOffsets[0],
                                &usWindows[0], &usWinsValid);

    /* convert a compact pci address to a local pci address */

    return ( sysDec2155xCnvrtAdrs (adrsSpace, (UINT32)busAdrs,
                                   (UINT32 *)pLocalAdrs, usWinsValid,
                                   &usWindows[0]) );

    }

#if (PCI_CFG_TYPE==PCI_CFG_NONE)
/******************************************************************************
*
* sysDec2155xPciInit - initialize PCI specific configuration
*
* RETURNS: OK if no configuration errors, ERROR otherwise.
*
*/

STATUS sysDec2155xPciInit ()

    { 
    UINT32 dec2155xSetup;

    /* 
     * Using config space setup values to calculate memory window sizes 
     * (CSR BAR has hardwired setup value) and actual configured BAR values, 
     * call the function dec2155xMmu() to setup for the ultimate creation 
     * of page tables which cover the PCI memory space used by this device.
     */

    if (dec2155xMmu (DEC2155X_CFG_SEC_CSR_MEM_BAR, 
                     DEC2155X_MEM_SIZE(DEC2155X_US_CSR_MEM_SETUP)) == ERROR)
        return (ERROR);

    DEC2155X_CFG_RD_LONG (DEC2155X_CFG_US_IO_OR_MEM0_SETUP, &dec2155xSetup);

    if (dec2155xMmu (DEC2155X_CFG_SEC_US_IO_OR_MEM0_BAR, 
		     DEC2155X_MEM_SIZE(dec2155xSetup)) == ERROR)
        return (ERROR);

    DEC2155X_CFG_RD_LONG (DEC2155X_CFG_US_MEM1_SETUP, &dec2155xSetup);

    if (dec2155xMmu (DEC2155X_CFG_SEC_US_MEM1_BAR, 
		     DEC2155X_MEM_SIZE(dec2155xSetup)) == ERROR)
        return (ERROR);

    /* Enable I/O, memory, bus mastering */

    pciConfigOutLong (PCI2PCI_BUS, PCI2PCI_DEV, PCI2PCI_FUN, PCI_CFG_COMMAND,
                      (UINT32)(0xffff0000 | PCI_CMD_IO_ENABLE | 
		      PCI_CMD_MEM_ENABLE | PCI_CMD_MASTER_ENABLE));

    return (OK);
    }

/******************************************************************************
*
* dec2155xMmu - Make an MMU entry to cover space required by Dec2155x
*
* RETURNS: OK if no error from sysMmuMapAdd(), ERROR otherwise.
*
*/

LOCAL STATUS dec2155xMmu
    (
    UINT32 barOffset,
    UINT32 size
    )

    {
    UINT32 dec2155xAddr;

    /* update the mmu table */

    DEC2155X_CFG_RD_LONG(barOffset, &dec2155xAddr);

    if ( (size != 0) &&                     /* If memory requested and ... */
	 ((dec2155xAddr & PCI_BAR_SPACE_IO) == 0) )  /* NOT I/O space */
	{
        dec2155xAddr = (dec2155xAddr & PCI_MEMBASE_MASK) -
        PCI_MSTR_MEMIO_BUS + PCI_MSTR_MEMIO_LOCAL;

        if (sysMmuMapAdd((void *)dec2155xAddr, size, 
                         (UINT)DEC2155X_INIT_STATE_MASK, 
                         (UINT)DEC2155X_INIT_STATE) == ERROR)
	    {
            logMsg("Unable map requested memory\n", 0, 0, 0, 0, 0, 0);  
	    return (ERROR);
	    }
        }
    return (OK);
    }

#endif /* PCI_CFG_TYPE==PCI_CFG_NONE */
