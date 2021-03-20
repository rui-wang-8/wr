/* failsafe.c - failsafe WatchDog Timer routines for the PLFPGA */

/* Copyright 1999-2000 Wind River Systems, Inc. All Rights Reserved */
/* Copyright 1999-2000 Motorola, Inc.  All Rights Reserved */

/*
modification history
--------------------
01d,27aug01,dgp  change "manual pages" to "reference entry" per SPR 23698
01c,20mar00,djs  Initial code review changes. Brought over cpv5000 version
01b,11Feb00,sjb  Changes per WRS code review.
01a,18Aug99,sjb  Initial Development 
*/

/*
DESCRIPTION

Support for the watchdog timer in the FPGA is provided.  This support
is not part of the standard VxWorks watchdog library, wdLib.  The 
primary advantage the new failsafe watchdog has over wdLib is that
the failsafe timer expiration is a non-maskable event which will 
drive the sysreset line on the board.  The failsafe is a one-shot 
timer, it will expire only once when set.  This prevents it from
repeatedly driving the board's sysreset line.

Failsafe timer support can be included in the BSP by defining 
INCLUDE_FAILSAFE in config.h.  This support by default is excluded.
There is only one failsafe timer on the board, so only one failsafe 
timer can be established at any given time.  

The following routines provide failsafe timer support.  Please see
the function reference entries for more detail on each routine.
failsafeStart(): Start or restart the failsafe WatchDog Timer. 
failsafeStrobe(): Restart the failsafe count using current settings.
failsafeCancel(): Cancel failsafe WatchDog Timer.
failsafeCausedReset(): Answers question: Did failsafe cause last reset?
failsafeRebootHook(): This is a hook routine for rebootLib.

In order to use the failsafe timer, the user will need to first call
failsafeStart.  Subsequently, the user will typically call the 
failsafeStrobe routine on a periodic basis, to prevent expiration 
of the failsafe timer.  If the failsafe timer is no longer needed, 
it should be disabled with a call to failsafeCancel.  If desired, 
the user can call failsafeCausedReset at any time to determine if 
the board's previous reset was caused by expiration of the failsafe 
timer.

The failsafe timer, if active, is cancelled during the processing of
a user reboot request.  This prevents the failsafe from an untimely
expiration, after the reboot has occurred.  For short failsafe timeout
delay lengths, the reboot processing may be too slow, allowing the
failsafe to expire before the failsafe is cancelled.   

INCLUDE FILES:  failsafe.h  
*/

/* includes */

#include "failsafe.h"

/* forward declarations */

LOCAL void failsafeWrite (uint8_t valWrite);
LOCAL uint8_t failsafeRead (void);

STATUS failsafeCancel (void);
void failsafeStrobe (void);
STATUS failsafeStart (uint8_t select);
BOOL failsafeCausedReset (void);
void failsafeRebootHook (int ignored);

/***************************************************************************
*
* failsafeWrite - write to failsafe WatchDog Timer regs
*
* This routine writes to one of the failsafe watchdog timer registers.
* The address as viewed from the CPU, is passed in along with the
* value to be written.  The address is verified to be one of the
* watchdog registers. 
*
*/
LOCAL void failsafeWrite
    (
    uint8_t valWrite	        /* value to be written */
    )

    {

    /* choose PLFPGA's device number 0, (legacy features) */

    sysOutByte (FAILSAFE_WDINDEX, FAILSAFE_DEVNUM_OFFSET);
    sysOutByte (FAILSAFE_WDDATA, FAILSAFE_LEGACY_DEVNUM);

    /* write value to the PLFPGA's WDCFG register */ 

    sysOutByte (FAILSAFE_WDINDEX, FAILSAFE_WDCFG_OFFSET);
    sysOutByte (FAILSAFE_WDDATA, valWrite);

    }


/***************************************************************************
*
* failsafeRead - Read the failsafe WatchDog Timer WDCFG register
*
* This routine reads the PLFPGA's WDCFG register.
*
* RETURNS: the value read from the PLFPGA's WDCFG register 
*
*/
LOCAL uint8_t failsafeRead (void)
    {

    /* choose PLFPGA's device number 0, (legacy features) */

    sysOutByte (FAILSAFE_WDINDEX, FAILSAFE_DEVNUM_OFFSET);
    sysOutByte (FAILSAFE_WDDATA, FAILSAFE_LEGACY_DEVNUM);

    /* read value from the PLFPGA's WDCFG register */ 

    sysOutByte (FAILSAFE_WDINDEX, FAILSAFE_WDCFG_OFFSET);
    return (sysInByte (FAILSAFE_WDDATA));

    }

/***************************************************************************
*
* failsafeCancel - Cancel failsafe WatchDog Timer  
*
* This routine cancels the failsafe watchdog timer.  
* 
* RETURNS: ERROR if the failsafe was not enabled, OK otherwise
*
*/
STATUS failsafeCancel (void)

    {
    uint8_t value;       

    value = failsafeRead ();
    if ( (value & FAILSAFE_WDCFG_WDX_BITS) == FAILSAFE_WDX_DISABLED )
	{
	return (ERROR);
	}
    else
	{

	/* Cancel the failsafe, leave other settings alone */

        value &= FAILSAFE_WDCFG_WDX_MASK; 
        failsafeWrite ( value | FAILSAFE_WDX_DISABLED );
	return (OK);
	}
    }

/***************************************************************************
*
* failsafeRebootHook - this is a hook routine for rebootLib
*
* This routine puts the failsafe timer into a quiescent state, in
* preparation for a reboot.  This routine will be run as part of the
* ctrl-x / reboot processing.  This prevents the failsafe from an untimely
* expiration, after the reboot has occurred.  
* Note: For short failsafe timeout delay lengths, the reboot processing 
* may be too slow, allowing the failsafe to expire before it is cancelled.
* 
* RETURNS: NA
*
* SEE ALSO: rebootHookAdd(), and the rebootLib documentation.
*/
void failsafeRebootHook 
    ( 
    int ignored 
    )
    {

    (void) failsafeCancel ();    /* turn off any pending failsafe */

    }

/***************************************************************************
*
* failsafeStart - Start or restart the failsafe WatchDog Timer 
*
* This routine starts the failsafe watchdog timer.  If a failsafe is 
* already enabled, this routine cancels the existing failsafe, 
* configures a new failsafe, and enables the new failsafe. 
* Note that any active failsafe timer is always cancelled upon entry 
* to this routine, even if failsafeStart returns ERROR.  This is to 
* prevent a system reset occuring during the caller's processing of 
* an ERROR return.
*
* The input parameter selects the timeout delay length, by specifying
* the appropriate bit pattern to be written to the SEL bits of the
* PLFPGA Watchdog configuration register.  For the cpv5xxx board 
* family, this equates to:
* .CS
*                    Timeout Length
*  Select        CPV5000
*  Value         CPV5300         CPN5360
*    0......... 17.8 msec       0.46 sec
*    1......... 71.1 msec       0.93 sec
*    2......... 284 msec        3.73 sec
*    3......... 1.14 sec        14.91 sec
*    4......... 4.55 sec        29.8 sec
*    5......... 18.22 sec       119 sec
*    6......... 72.8 sec        238 sec
*    7......... 291 sec         477 sec
* .CE
*
* RETURNS: ERROR if the params passed in are bad, OK otherwise
*
*/
STATUS failsafeStart
    (
    uint8_t select  /* one of the 8 possible delay lengths, 0 to 7 */
    )

    {
    uint8_t value;       

    (void) failsafeCancel ();   /* turn off any pending failsafe */

    if ((select & FAILSAFE_WDCFG_SELX_BITS) != select)
	{ 
	return (ERROR);
	}

    value = failsafeRead ();

    /* preserve bits not being changed */

    value &= ( FAILSAFE_WDCFG_SELX_MASK & FAILSAFE_WDCFG_WDX_MASK);

    /* set timer event to "NMI and sysreset", set chosen timer length */

    failsafeWrite ( value | select | FAILSAFE_WDX_NMI_RESET );

    return (OK);
    }

/***************************************************************************
*
* failsafeStrobe - Restart the failsafe count using current settings
*
* This routine resets the failsafe watchdog timer count according to 
* the timer's current settings, by writing to a strobe register in 
* the timer chip.  No error checking is done in this routine, in 
* order to process the strobe request in the shortest possible time.  
*
* RETURNS: NA
*
*/
void failsafeStrobe (void)

    {

    /* write any value to the PLFPGA's WDSTB register to strobe */ 

    sysOutByte (FAILSAFE_WDSTB, 0);

    }


/***************************************************************************
*
* failsafeCausedReset - answers question: Did failsafe cause last reset?
*
* This routine checks a bit in hardware which indicates whether the 
* last reset was caused by an expiration of the failsafe watchdog timer.
*
* RETURNS: TRUE if failsafe caused last reset, FALSE otherwise
*
*/
BOOL failsafeCausedReset (void)

    {

    uint8_t value;

    value = sysInByte (FAILSAFE_WDSTB);

    /* if bit is set, then a failsafe expiration caused last reset */

    if (value & FAILSAFE_WDSTB_HISTORY)
	return (TRUE);

    return (FALSE);
    }


