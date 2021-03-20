/* sysAta.c - Motorola CPN5360 ATA-2 initialization */

/* Copyright 1984-1998 Wind River Systems, Inc. */
/* Copyright 1997-2000 Motorola, Inc. All Rights Reserved */
#include "copyright_wrs.h"

/*
modification history
--------------------
01a,07jan00,sjb  ported from PowerPC base  (01e,17jul99,dmw)
*/

/* 
Description

This file contains the sysAtaInit() necessary for
initializing the ATA/EIDE subsystem. 
*/

/* includes */

#include "vxWorks.h"
#include "config.h"


#ifdef	INCLUDE_ATA

/* local defines */

/* external declarations */

/* global declarations */

/* function declarations */

/******************************************************************************
*
* sysAtaInit - initialize the EIDE/ATA interface
*
* Perform the necessary initialization required before starting up the
* ATA/EIDE driver.
*/

void sysAtaInit ()
    {

    /* 
     * REFERENCE:  The PIIX4e does not use standard PCI Base Address 
     * Registers for the IDE function.  Configuration space offsets 
     * 0x10 thru 0x1f, representing BAR 0 through BAR 4, are reserved, 
     * and read 0.  The PCI configuration sequence will only configure 
     * BAR 4, the BMIBA register for the IDE device.   
     * Although some other PCI IDE devices provide read-only 
     * BARs, containing the legacy IDE addresses, PIIX4 doesn't,
     * so we'll have to use #defines for the legacy addresses.  These
     * addresses are stored in the ataResources array for use by the
     * ATA driver.
     */

    /* save Base Address of Command Reg for Primary Controller */

    ataResources[0].resource.ioStart[0] = ATA0_IO_START0;

    /* save Base Address of Control Reg for Primary Controller */

    ataResources[0].resource.ioStart[1] = ATA0_IO_START1;

    /*
     * Initialize the remainder of the ataResources structure for the
     * Primary Controller
     */

    ataResources[0].resource.vcc = 5;	
    ataResources[0].resource.vpp = 0;	
    ataResources[0].ctrlType = IDE_LOCAL;
    ataResources[0].drives = ATA_DEV0_STATE + ATA_DEV1_STATE;    
    ataResources[0].intVector = (int)ATA0_INT_VEC;
    ataResources[0].intLevel = (int)ATA0_INT_LVL;
    ataResources[0].configType = ( ATA0_CONFIG );
    ataResources[0].semTimeout = ATA_SEM_TIMEOUT;
    ataResources[0].wdgTimeout = ATA_WDG_TIMEOUT;
    ataResources[0].sockTwin = 0;
    ataResources[0].pwrdown = 0;

    /* save Base Address of Command Reg for Secondary Controller */

    ataResources[1].resource.ioStart[0] =  ATA1_IO_START0;

    /* save Base Address of Control Reg for Secondary Controller */

    ataResources[1].resource.ioStart[1] = ATA1_IO_START1;

    /*
     * Initialize the remainder of the ataResources structure for the
     * Secondary Controller
     */

    ataResources[1].resource.vcc = 5;	
    ataResources[1].resource.vpp = 0;	
    ataResources[1].ctrlType = IDE_LOCAL;
    ataResources[1].drives = ATA_DEV2_STATE + ATA_DEV3_STATE;
    ataResources[1].intVector = (int)ATA1_INT_VEC;
    ataResources[1].intLevel = (int)ATA1_INT_LVL;
    ataResources[1].configType = ( ATA1_CONFIG );
    ataResources[1].semTimeout = ATA_SEM_TIMEOUT;
    ataResources[1].wdgTimeout = ATA_WDG_TIMEOUT;
    ataResources[1].sockTwin = 0;
    ataResources[1].pwrdown = 0;

    }

#endif /* INCLUDE_ATA */
