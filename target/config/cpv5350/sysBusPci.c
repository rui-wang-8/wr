/* sysBusPci.c - CPV5xxx autoconfig support */

/* Copyright 1999-2002 Wind River Systems, Inc. All Rights Reserved */
/* Copyright 1999-2000 Motorola, Inc. All Rights Reserved */

/*
modification history
--------------------
04o,30apr02,dat  Update to incorporate cpn5360 support
04n,22may00,djs   Added support for the CPV5350.
04m,22oct99,scb   Clean up sysPciRollcallRtn().
04l,13oct99,scb   IDE configured generically instead of with special routine.
01k,04oct99,scb   Fixed interrupt routing for CPV5300, bridge device 0x11.
01j,02jul99,djs   added support for PCI devices on CPV5300
01i,11jun99,scb   Fixed PCI window naming.
01h,08jun99,scb   Moved from T1 base to T2 base.
01g,10may99,djs   Typo's in debug messages fixed
01f,20apr99,scb   Syntax issues with CPV5300.
01e,13apr99,djs   Added routines to handle the special pci configuration
		  requirements of the IDE device.
01d,12mar99,scb   Fix broken roll call loop which never allowed exit.
01c,26mar99,scb   Add code for PCI autoconfig "roll call" support.
01b,26mar99,djs   Added code to setup intLine table.
01a,08mar99,djs   Initial version of this file derived from the 
		  mvme2400 equivelant.

*/

/*
DESCRIPTION

This module contains the "non-generic" or "board specific" PCI-PCI
bridge initialization code.  The module contains code to:

  1.  Determine if a particular function is to be excluded from the
          automatic configuration process.
  2.  Program the "interrupt line" field of the PCI configuration header.

*/

/* includes */

#include "vxWorks.h"
#include "logLib.h"
#include "taskLib.h"
#include "config.h"
#include "drv/pci/pciConfigLib.h"
#include "drv/pci/pciAutoConfigLib.h"

/* defines */

#ifdef CPN5360

#   define PIRQVAL ((PIRQD << 24) | (PIRQC << 16) | \
	(PIRQB << 8)  | (PIRQA << 0))

#else /* CPV5000/5300/5350 */

    /* Dummy intial table values */

#   define PIRQA	0x81
#   define PIRQB	0x82
#   define PIRQC	0x83
#   define PIRQD	0x84

#endif

#define NUM_DEVICES 21
#define NUM_PCIINTS 4

/* typedefs */

/* globals */

/* locals */

static UCHAR intLine [NUM_DEVICES][NUM_PCIINTS] =
    {
#if defined(CPV5000)
    { 0xff, 0xff, 0xff, 0xff },      /* device number 0, PCI Host bridge */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 1 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 2 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 3 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 4 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 5 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 6 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 7, PIIX 3 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 8 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 9 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 10 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 11 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 12 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 13 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 14 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 15 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 16 */
    { PIRQD, 0xff, 0xff, 0xff },     /* device number 17, SCSI */
    { PIRQA, PIRQB, PIRQC, PIRQD },  /* device number 18, DEC PCI-PCI bridge */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 19, Video */
    { PIRQB, 0xff, 0xff, 0xff }      /* device number 20, Ethernet */

#elif defined(CPV5300)

    { 0xff, 0xff, 0xff, 0xff },      /* device number 0, PCI Host bridge */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 1 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 2 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 3 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 4 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 5 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 6 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 7, PIIX 4 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 8 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 9 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 10 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 11 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 12 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 13 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 14 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 15 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 16 */
    { PIRQD, PIRQA, PIRQB, PIRQC },  /* device number 17, DEC PCI-PCI bridge */
    { PIRQA, 0xff, 0xff, 0xff },     /* device number 18, Ethernet 2 */
    { PIRQC, 0xff, 0xff, 0xff },     /* device number 19, Ethernet 1 */
    { PIRQB, 0xff, 0xff, 0xff }      /* device number 20, SCSI */

#elif defined(CPV5350)

    { 0xff, 0xff, 0xff, 0xff },      /* device number 0, PCI Host bridge */
    { PIRQA, PIRQB, PIRQC, PIRQD },  /* device number 1 P2P Bridge */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 2 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 3 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 4 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 5 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 6 */
    { PIRQA, PIRQB, PIRQC, PIRQD },  /* device number 7, PIIX 4 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 8 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 9 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 10 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 11 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 12 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 13 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 14 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 15 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 16 */
    { PIRQA, PIRQB, PIRQC, PIRQD },  /* device number 17, DEC PCI-PCI bridge */
    { PIRQA, 0xff, 0xff, 0xff },     /* device number 18, Ethernet 2 */
    { PIRQC, 0xff, 0xff, 0xff },     /* device number 19, Ethernet 1 */
    { 0xff,  0xff, 0xff, 0xff }      /* device number 20, */

#elif defined(CPN5360)

    { 0xff, 0xff, 0xff, 0xff },      /* device number 0, PCI Host bridge */
    { PIRQA, PIRQB, PIRQC, PIRQD },  /* device number 1 P2P Bridge */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 2 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 3 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 4 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 5 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 6 */
    { PIRQA, PIRQB, PIRQC, PIRQD },  /* device number 7, PIIX 4 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 8 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 9 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 10 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 11 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 12 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 13 */
    { 0xff, 0xff, 0xff, 0xff },      /* device number 14 */
    { PIRQD, PIRQA, PIRQB, PIRQC },  /* device number 15, PMC1 */
    { PIRQA, PIRQB, PIRQC, PIRQD },  /* device number 16, PMC2 */
    { PIRQA, 0xff, 0xff, 0xff },     /* device number 17, DEC21554 */
    { PIRQC, 0xff, 0xff, 0xff },     /* device number 18, Ethernet 1 */
    { PIRQA, 0xff, 0xff, 0xff },     /* device number 19, Ethernet 2 */
    { 0xff, 0xff, 0xff, 0xff }       /* device number 20 */
#endif

    };

LOCAL    PCI_SYSTEM    sysParams;

#ifdef PCI_ROLL_CALL_LIST_ENTRIES
/* PCI autoconfig roll call support */

/* Roll call list entry structure, list elements specified in "config.h" */

typedef struct _PCI_ROLL_CALL_LIST
    {
    UINT count;
    UINT Dev;
    UINT Vend;
    } PCI_ROLL_CALL_LIST;

LOCAL PCI_ROLL_CALL_LIST rollCall[] =
    {
    PCI_ROLL_CALL_LIST_ENTRIES
    { 0xffff, 0xffff, 0xffff }  /* Required entry: marks end of list */
    };
#endif

/* forward declarations */

LOCAL UCHAR sysPciAutoConfigIntAsgn ( PCI_SYSTEM * pSys, PCI_LOC * pFunc,
    			UCHAR intPin );

LOCAL STATUS sysPciAutoConfigInclude ( PCI_SYSTEM *pSys, PCI_LOC *pciLoc,
			UINT devVend );

#ifdef PCI_ROLL_CALL_LIST_ENTRIES
LOCAL STATUS sysPciRollcallRtn ( );
#endif

/* subroutines */

/******************************************************************************
*
* sysPciAutoConfigInclude - Determine if function is to be autoConfigured
*
* This function is called with PCI bus, device, function, and vendor 
* information.  It returns an indication of whether or not the particular
* function should be included in the automatic configuration process.
* This capability is useful if it is desired that a particular function
* NOT be automatically configured.  Of course, if the device is not
* included in automatic configuration, it will be unusable unless the
* user's code made provisions to configure the function outside of the
* the automatic process.
*
* RETURNS: OK if function is to be included in automatic configuration,
* ERROR otherwise.
*/
 
LOCAL STATUS sysPciAutoConfigInclude
    (
    PCI_SYSTEM *pSys,       /* input: AutoConfig system information */
    PCI_LOC *pciLoc,        /* input: PCI address of this function */
    UINT     devVend        /* input: Device/vendor ID number      */
    )
    {
    BOOL retVal = OK;
    
    /* If it's the host bridge then exclude it */

    if ((pciLoc->bus == 0) && (pciLoc->device == 0) && (pciLoc->function == 0))
        return ERROR;

    switch(devVend)
	{

	/* EXCLUDED Devices */

	case PCI_ID_HOST_BRIDGE:
		retVal = ERROR;
		PCI_AUTO_DEBUG_MSG("sysPciAutoconfigInclude: Excluding N. Bridge\n",
			0, 0, 0, 0, 0, 0);
		break;

	case PCI_ID_USB:
		retVal = ERROR;
		PCI_AUTO_DEBUG_MSG("sysPciAutoconfigInclude: Excluding USB\n",
			0, 0, 0, 0, 0, 0);
		break;

	case PCI_ID_VIDEO:
		retVal = ERROR;
		PCI_AUTO_DEBUG_MSG("sysPciAutoconfigInclude: Excluding Video\n",
			0, 0, 0, 0, 0, 0);
		break;

	 case PCI_ID_PM:
		retVal = ERROR;
		PCI_AUTO_DEBUG_MSG("sysPciAutoconfigInclude: Excluding Power Management\n",
			0, 0, 0, 0, 0, 0);
		break;

	case PCI_ID_AGP:
		retVal = ERROR;
		PCI_AUTO_DEBUG_MSG("sysPciAutoconfigInclude: Excluding AGP Bridge\n",
			0, 0, 0, 0, 0, 0);
		break;


	/* INCLUDED Devices */

    	default:
    		retVal = OK;
		PCI_AUTO_DEBUG_MSG("sysPciAutoconfigInclude: "
			"Including DeviceVend 0x%x\n",
			devVend, 0, 0, 0, 0, 0);
		break;
	}

    return retVal;
    }

/******************************************************************************
*
* sysPciAutoConfigIntAssign - Assign the "interrupt line" value
*
* RETURNS: "interrupt line" value.
*
*/

LOCAL UCHAR sysPciAutoConfigIntAsgn
    ( 
    PCI_SYSTEM * pSys,	/* input: AutoConfig system information */
    PCI_LOC * pFunc,
    UCHAR intPin	/* input: interrupt pin number */
    )
    {
    UCHAR irqValue = 0xff;    /* Calculated value                */


    if (intPin == 0) 
        return irqValue;

    irqValue = intLine [(pFunc->device)][(intPin - 1)];

    PCI_AUTO_DEBUG_MSG("intAssign called for device [%d %d %d] IRQ: %d\n",
    		pFunc->bus, pFunc->device, pFunc->function,
    		irqValue, 0, 0 );

    return (irqValue);
    }

#ifdef PCI_ROLL_CALL_LIST_ENTRIES
/*****************************************************************************
*
* sysPciRollCallRtn - Check "roll call" list against list of PCI devices found
*
* This function checks if the number of devices actually found during
* the 1st pass of PCI autoconfiguration (bus enumeration process)
* passes the "roll call" test.  That is, for each entry in the roll call
* list (consisting of a count and device/vendor ID), a check is made to
* insure that at least the specified minimum number of devices has
* actually been discovered.  If the roll call passes, the function returns
* TRUE.  If the roll call fails and the time duration in seconds represented
* by ROLL_CALL_MAX_DURATION has not elapsed, the function will wait 1
* second and return FALSE.  If the roll call fails and the time duration in
* seconds represented by ROLL_CALL_MAX_DURATION has elapsed, the function
* will return TRUE.
*
* RETURNS: TRUE if roll call test passes or timeout, FALSE otherwise.
*/

LOCAL STATUS sysPciRollcallRtn
    (
    )
    {
    STATUS  rollCallPass;       /* Flag indicating pass or fail */
    int     rollIndex;
    UINT    bus;
    UINT    dev;
    UINT    func;
    int     count;
    static  int secDelay = 0;

    rollCallPass = TRUE;        /* Default = "passed" */

    rollIndex = 0;

    while (secDelay < ROLL_CALL_MAX_DURATION) 
        {

        if (rollCall[rollIndex].Vend == 0xffff)
            break;              /* End of roll call list, we're done */

        count = 0;

        while (pciFindDevice(rollCall[rollIndex].Vend, rollCall[rollIndex].Dev,
			    count, &bus, &dev, &func) == OK)
	    count++;

        if (count < rollCall[rollIndex].count)
	    {
            rollCallPass = FALSE;        /* Roll call - someone is missing */
	    sysNanoDelay(1000000000);    /* Delay a second */
	    secDelay++;
	    break;
	    }

	rollIndex++;
        }

    if (rollCallPass == TRUE)
	secDelay = 0;

    return (rollCallPass);
    }
#endif

#ifndef CPN5360
/*****************************************************************************
*
* sysPciAutoConfigIntInit - Initialize the "interrupt line" table
*
* RETURNS: nothing.
*
*/

LOCAL void sysPciAutoConfigIntInit
    ( 
    void
    )
    {
    UCHAR	dev;
    UCHAR	pciIntLine;
    UINT	pirqVal;
    UCHAR	pirqA;
    UCHAR	pirqB;
    UCHAR	pirqC;
    UCHAR	pirqD;

    /* Read the Routing Registers */

    pciConfigInLong(PCI2ISA_BUS, PCI2ISA_DEV, PCI2ISA_FUN,
		    PCI2ISA_PIRQ_OFF, &pirqVal);

    /* Pull out the PIRQA-D settings */

    pirqA = (pirqVal & 0x000000ff);
    pirqB = ((pirqVal & 0x0000ff00) >> 8);
    pirqC = ((pirqVal & 0x00ff0000) >> 16);
    pirqD = ((pirqVal & 0xff000000) >> 24);

    /* If the PIRQ is disabled, change the PIRQ value to 0xFF */

    if (pirqA & PCI2ISA_PIRQ_DISABLED)
	pirqA = 0xff;

    if (pirqB & PCI2ISA_PIRQ_DISABLED)
	pirqB = 0xff;

    if (pirqC & PCI2ISA_PIRQ_DISABLED)
	pirqC = 0xff;

    if (pirqD & PCI2ISA_PIRQ_DISABLED)
	pirqD = 0xff;

    /* Search the table for the dummy values, replace with actual */

    for (dev = 0; dev < NUM_DEVICES; dev++)
	for (pciIntLine = 0; pciIntLine < NUM_PCIINTS; pciIntLine++)
		{
		if (intLine[dev][pciIntLine] == PIRQA)
			intLine[dev][pciIntLine] = pirqA;
		else if (intLine[dev][pciIntLine] == PIRQB)
			intLine[dev][pciIntLine] = pirqB;
		else if (intLine[dev][pciIntLine] == PIRQC)
			intLine[dev][pciIntLine] = pirqC;
		else if (intLine[dev][pciIntLine] == PIRQD)
			intLine[dev][pciIntLine] = pirqD;
		}
    return;
    }
#endif /* CPN5360 */

/*******************************************************************************
*
* sysPciAutoConfig - PCI autoConfig support routine
*
* This routine instantiates the PCI_SYSTEM structure needed to configure
* the system. This consists of assigning address ranges to each category
* of PCI system resource: Prefetchable and Non-Prefetchable 32-bit Memory, and
* 16- and 32-bit I/O. Global values for the Cache Line Size and Maximum
* Latency are also specified. Finally, the four supplemental routines for 
* device inclusion/exclusion, interrupt assignment, and pre- and
* post-enumeration bridge initialization are specified. 
*
* RETURNS: N/A
*/
void sysPciAutoConfig(void)
    {

    /* 32-bit Prefetchable Memory Space */

    sysParams.pciMem32 = PCI_MSTR_MEM_BUS;
    sysParams.pciMem32Size = PCI_MSTR_MEM_SIZE;

    /* 32-bit Non-prefetchable Memory Space */

    sysParams.pciMemIo32 = PCI_MSTR_MEMIO_BUS;
    sysParams.pciMemIo32Size = PCI_MSTR_MEMIO_SIZE;

    /* 16-bit ISA I/O Space */

    sysParams.pciIo16 = ISA_MSTR_IO_BUS;
    sysParams.pciIo16Size = ISA_MSTR_IO_SIZE;

    /* 32-bit PCI I/O Space */

    sysParams.pciIo32 = PCI_MSTR_IO_BUS;
    sysParams.pciIo32Size = PCI_MSTR_IO_SIZE;

    /* Configuration space parameters */

    sysParams.maxBus = 0;
    sysParams.cacheSize = ( _CACHE_ALIGN_SIZE / sizeof(UINT32) );
    sysParams.maxLatency = PCI_LAT_TIMER;

    sysParams.autoIntRouting = TRUE;

    /* Device inclusion and interrupt routing routines */

    sysParams.includeRtn = sysPciAutoConfigInclude;
    sysParams.intAssignRtn = sysPciAutoConfigIntAsgn;

#ifdef PCI_ROLL_CALL_LIST_ENTRIES
    /* "Roll call" routine */

    sysParams.pciRollcallRtn = sysPciRollcallRtn;
#endif

    /*
     * PCI-to-PCI Bridge Pre-
     * and Post-enumeration init
     * routines
     */

    sysParams.bridgePreConfigInit = NULL;
    sysParams.bridgePostConfigInit = NULL;

#ifdef CPN5360

    /* Program the PCI routing control register */

    pciConfigOutLong (PCI2ISA_BUS, PCI2ISA_DEV, PCI2ISA_FUN,
		      PCI2ISA_PIRQ_OFF, PIRQVAL);

#else
    /* 
     * Initialize the intLine table based on the Routing 
     * Register settings.
     */

    sysPciAutoConfigIntInit();
#endif


    /* Perform AutoConfig */

    pciAutoConfig(&sysParams);

    }

