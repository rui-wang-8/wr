/* intSync.c - a program to demonstrate synchronizing among tasks
 *             running on multiple CPUs by using interrupts across
 *             the VME backplane.
 */

/* Copyright 1984-1997 Wind River Systems, Inc. */

/*
modification history
--------------------
01c,06nov97,mm  added copyright.
01b,17sept97,ram added include files stdio.h, sysLib.h, logLib.h, intLib.h
		 casted semGive to be a VOIDFUNCPTR in function intInit()		 
01a,14jan92,ms   written.
*/

#include "vxWorks.h"
#include "semLib.h"
#include "iv.h"
#include "intGen.h"
#include "stdio.h"
#include "sysLib.h"
#include "logLib.h"
#include "intLib.h"

LOCAL SEM_ID semId;          /* semaphore ID */
LOCAL void intInit (void);/*initialize for synchronization with VME interrupt*/


/*****************************************************************************
 * intSync - Synchronize with VME bus interrupt
 *
 * DESCRIPTION
 *
 * This task is used for synchronization with VME bus interrupt generated by
 * CPU 0.
 *
 * The intSync task runs on CPU1, intGen task runs on CPU 0
 *
 * EXAMPLE
 *
 * The following is an example from VxWorks shell:
 *
 * -> sp (intSync)
 *
 */

void intSync (void)
    {

    /* initialize for synchronization */
    intInit ();

    FOREVER
        {
        printf ("CPU 1: Waiting to be synchronized by VME bus interrupt from CPU 0\n");
        if (semTake (semId, WAIT_FOREVER) == ERROR)
            {
            perror ("intSync: semTake failed");
            return;
            } 
        printf ("\nSynchronization done\n");

        /* Do the work that need to be synchronized */
        printf ("RUNNING\n\n\n");
        }
    }

/*****************************************************************************
 * intHdlr - interrupt handler for I960 architecture targets. I960 
 *           architecture targets need to acknowledge the bus interrupt in
 *           software.
 *           
 */

void intHdlr
    (
    SEM_ID semId
    )
    {
    /*unsigned char vec = sysBusIntAck (1);*/
    logMsg ("intHdlr:: acknowledging interrupt\n",0,0,0,0,0,0);
    semGive (semId);
    }

/*****************************************************************************
 * intInit - initialize for synchronization with VME bus interrupt
 *
 * DESCRIPTION
 *
 * This task does necessary initialization for synchronization with VME
 * bus interrupt generated by CPU 0.
 */

LOCAL void intInit (void)
    {
    /* set up semaphore for synchronization */
    if ((semId = semBCreate (SEM_Q_PRIORITY, SEM_EMPTY)) == NULL)
        {
        perror ("intInit: semBCreate failed");
        return;
        }

    /* enable interrupts                  */
    if (sysIntEnable (INTLEVEL) == ERROR)
        {
        printf ("intInit: Error in enabling interrupts\n");
        return;
        }

    /* connect semGive to the hardware interrupt */

#if (CPU_FAMILY == I960)

    /*  connect the interrupt handler (that calls semGive) to the hardware 
     *  interrupt 
     */
    if (intConnect ((VOIDFUNCPTR *) INUM_TO_IVEC (INTNUM), 
                                             intHdlr, semId) == ERROR)
        {
        perror ("intInit: Error in connecting to the ISR");
        return;
        }
#endif

#if (CPU_FAMILY == MC680X0)
    if (intConnect ((VOIDFUNCPTR *) INUM_TO_IVEC (INTNUM), 
                                            (VOIDFUNCPTR) semGive, (int)semId) == ERROR)
        {
        perror ("intInit: Error in connecting to the ISR");
        return;
        }
#endif

#if (CPU_FAMILY == SPARC)
    if (intConnect ((VOIDFUNCPTR *) INUM_TO_IVEC (INT_NUM_INTSYNC), 
                                             semGive, semId) == ERROR)
        {
        perror ("intInit: Error in connecting to the ISR");
        return;
        }
#endif
    }

