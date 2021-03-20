/* i8253Timer.c - Intel 8253 timer library */

/* Copyright 1984-2002 Wind River Systems, Inc. */
/* Copyright 1999 Motorola, Inc.  All Rights Reserved */
#include "copyright_wrs.h"

/*
modification history
--------------------
01r,13may02,dat  Another NULL fix.
01q,16apr02,dat  Update for T2.2 release, fix warning about improper use of NULL
01p,19jul99,sjb  changed the RTC version of the Aux Clk code to be 
		 compatible with other Real Time Clock activity. 
01o,08may98,hdn  fixed typo.  sysTsc..() to pentiumTsc..().
01n,20apr98,hdn  added documentation for INCLUDE_TIMESTAMP_TSC.
01m,09apr98,hdn  added support for Pentium, PentiumPro.
01l,22jul96,dat  merged timestamp driver into normal driver.
01k,23may96,wlf  doc: cleanup.
01j,12feb96,hdn  added support for a channel-1 as Aux timer with PIT1_FOR_AUX.
01i,25apr94,hdn  changed sysAuxClkTicksPerSecond from 60 to 64.
01h,09nov93,vin  added SYS_CLK_RATE_MIN, SYS_CLK_RATE_MAX, AUX_CLK_RATE_MIN,
                 AUX_CLK_RATE_MAX macros, moved intConnect of auxiliary clock
		 sysLib.c; made sysAuxClkRateSet return error if any value
		 other than the one in the auxTable is set. Added sysClkDisable
		 and sysClkEnable in sysClkRateSet.
01g,27oct93,hdn  deleted memAddToPool stuff from sysClkConnect().
01f,12oct93,hdn  added sysIntEnablePIC(), sysIntDisablePIC().
01e,16aug93,hdn  added aux clock functions.
01d,16jun93,hdn  updated to 5.1.
01c,08apr93,jdi  doc cleanup.
01d,07apr93,hdn  renamed compaq to pc.
01c,26mar93,hdn  added the global descriptor table, memAddToPool.
		 moved enabling A20 to romInit.s. added cacheClear for 486.
01b,18nov92,hdn  supported nested interrupt.
01a,15may92,hdn  written based on frc386 version.
*/

/*
DESCRIPTION
This library contains routines to manipulate the timer functions on the
Intel 8253 chip with a board-independent interface.  This library handles
both the system clock and the auxiliary clock functions.  It also
contains code for the TimeStamp Counter in the Pentium Processor. 
It also contains code for the periodic interrupt of the CMOS Real 
Time Clock (RTC) device.

The macros SYS_CLK_RATE_MIN, SYS_CLK_RATE_MAX, AUX_CLK_RATE_MIN, and
AUX_CLK_RATE_MAX must be defined to provide parameter checking for the
sys[Aux]ClkRateSet() routines.

The macro PIT_CLOCK must also be defined to indicate the clock frequency
of the i8253.

The macro INCLUDE_TIMESTAMP_TSC lets the time stamp driver use PentiumPro's
onchip TSC (Time Stamp Counter).
The PentiumPro processor provides a 64-bit time-stamp counter that is
incremented every processor clock cycle.  The counter is incremented even
when the processor is halted by the HLT instruction or the external STPCLK#
pin.  The time-stamp counter is set to 0 following a hardware reset of the
processor.  The RDTSC instruction reads the time stamp counter and is
guaranteed to return a monotonically increasing unique value whenever
executed, except for 64-bit counter wraparound.  Intel guarantees,
architecturally, that the time-stamp counter frequency and configuration will
be such that it will not wraparound within 10 years after being reset to 0.
The period for counter wrap is several thousands of years in the PentiumPro
and Pentium processors.

The system clock is the first programmable interrupt timer in the 8253 (PIT0).
The Auxiliary clock can be either the second timer in the i8253 (PIT1),
or the Periodic Interrupt of the CMOS Real Time Clock device, 
The setting of PIT1_FOR_AUX controls which device is Aux Clock.

*/


#ifndef PIT1_FOR_AUX
#include "sysRtc.h"  
#endif /* PIT1_FOR_AUX */

#ifdef	INCLUDE_TIMESTAMP

/* Locals */ 

LOCAL BOOL	sysTimestampRunning = FALSE;	/* running flag */
LOCAL int	sysTimestampPeriodValue = 0;	/* Max counter value */

union uc
    {
    USHORT count16;
    UCHAR  count08[2];
    };

#ifdef	INCLUDE_TIMESTAMP_TSC
LOCAL FUNCPTR	sysTimestampRoutine   = NULL;	/* user rollover routine */
LOCAL int   	sysTimestampTickCount = 0;	/* system ticks counter */
LOCAL UINT32	sysTimestampFreqValue = PENTIUMPRO_TSC_FREQ;	/* TSC freq */
#endif	/* INCLUDE_TIMESTAMP_TSC */
#endif	/* INCLUDE_TIMESTAMP */

/* These are exported because the RTC code uses them in the ISR */

FUNCPTR sysAuxClkRoutine	= NULL;
int sysAuxClkRunning		= FALSE;
int sysAuxClkArg		= 0;

/* locals */

LOCAL FUNCPTR sysClkRoutine	= NULL; /* routine to call on clock interrupt */
LOCAL int sysClkArg		= 0; /* its argument */
LOCAL int sysClkRunning		= FALSE;
LOCAL int sysClkConnected	= FALSE;
LOCAL int sysClkTicksPerSecond	= 60;

LOCAL int sysAuxClkConnected	= FALSE;

#ifdef	PIT1_FOR_AUX
LOCAL int sysAuxClkTicksPerSecond = 60;
#else
LOCAL int sysAuxClkTicksPerSecond = 64;
LOCAL CLK_RATE auxTable[] =
    {
    {   2, 0x0f}, 
    {   4, 0x0e}, 
    {   8, 0x0d}, 
    {  16, 0x0c}, 
    {  32, 0x0b}, 
    {  64, 0x0a}, 
    { 128, 0x09}, 
    { 256, 0x08}, 
    { 512, 0x07}, 
    {1024, 0x06}, 
    {2048, 0x05}, 
    {4096, 0x04}, 
    {8192, 0x03} 
    };
#endif	/* PIT1_FOR_AUX */


/* forward declarations */

#if	defined (INCLUDE_TIMESTAMP) && defined (INCLUDE_TIMESTAMP_TSC)
LOCAL void	sysTimestampFreqGet (void);
#endif	/* defined (INCLUDE_TIMESTAMP) && defined (INCLUDE_TIMESTAMP_TSC) */


/*******************************************************************************
*
* sysClkInt - interrupt level processing for system clock
*
* This routine handles the system clock interrupt.  It is attached to the
* clock interrupt vector by the routine sysClkConnect().
*/

LOCAL void sysClkInt (void)
    {
    /* acknowledge interrupt */

    if (sysClkRoutine != NULL)
	(* sysClkRoutine) (sysClkArg);

#if	defined (INCLUDE_TIMESTAMP) && defined (INCLUDE_TIMESTAMP_TSC)
    if (sysTimestampRoutine != NULL)
	(* sysTimestampRoutine) ();
#endif	/* defined (INCLUDE_TIMESTAMP) && defined (INCLUDE_TIMESTAMP_TSC) */
    }


/*******************************************************************************
*
* sysClkConnect - connect a routine to the system clock interrupt
*
* This routine specifies the interrupt service routine to be called at each
* clock interrupt.  Normally, it is called from usrRoot() in usrConfig.c to 
* connect usrClock() to the system clock interrupt.
*
* RETURN: OK, or ERROR if the routine cannot be connected to the interrupt.
*
* SEE ALSO: intConnect(), usrClock(), sysClkEnable()
*/

STATUS sysClkConnect
    (
    FUNCPTR routine,	/* routine to be called at each clock interrupt */
    int arg		/* argument with which to call routine */
    )
    {
    sysHwInit2 ();	/* XXX for now -- needs to be in usrConfig.c */

    sysClkRoutine   = routine;
    sysClkArg	    = arg;
    sysClkConnected = TRUE;

    return (OK);
    }


/*******************************************************************************
*
* sysClkDisable - turn off system clock interrupts
*
* This routine disables system clock interrupts.
*
* RETURNS: N/A
*
* SEE ALSO: sysClkEnable()
*/

void sysClkDisable (void)
    {
    int oldLevel;

    if (sysClkRunning)
	{
        oldLevel = intLock ();				/* LOCK INTERRUPT */
	sysOutByte (PIT_CMD (PIT_BASE_ADR), 0x38);
	sysOutByte (PIT_CNT0 (PIT_BASE_ADR), LSB(0));
	sysOutByte (PIT_CNT0 (PIT_BASE_ADR), MSB(0));
        intUnlock (oldLevel);				/* UNLOCK INTERRUPT */

	sysIntDisablePIC (PIT0_INT_LVL);
	
#ifdef	INCLUDE_TIMESTAMP
        sysOutByte (0x61, sysInByte (0x61) & 0xfe);	/* disable counter 2 */
#endif	/* INCLUDE_TIMESTAMP */

	sysClkRunning = FALSE;
	}
    }


/*******************************************************************************
*
* sysClkEnable - turn on system clock interrupts
*
* This routine enables system clock interrupts.
*
* RETURNS: N/A
*
* SEE ALSO: sysClkConnect(), sysClkDisable(), sysClkRateSet()
*/

void sysClkEnable (void)
    {
    UINT tc0;
    UINT tc2;
    int oldLevel;

    if (!sysClkRunning)
	{
	
	tc0 = PIT_CLOCK / sysClkTicksPerSecond;
        tc2 = PIT_CLOCK / sysClkTicksPerSecond * 2;

        oldLevel = intLock ();				/* LOCK INTERRUPT */

#ifdef	INCLUDE_TIMESTAMP
        sysOutByte (0x61, sysInByte (0x61) | 1);	/* enable counter 2 */
#endif	/* INCLUDE_TIMESTAMP */

	sysOutByte (PIT_CMD (PIT_BASE_ADR), 0x36);
	sysOutByte (PIT_CNT0 (PIT_BASE_ADR), LSB(tc0));
	sysOutByte (PIT_CNT0 (PIT_BASE_ADR), MSB(tc0));

#ifdef	INCLUDE_TIMESTAMP
        sysOutByte (PIT_CMD (PIT_BASE_ADR), 0xb6);
        sysOutByte (PIT_CNT2 (PIT_BASE_ADR), LSB(tc2));
        sysOutByte (PIT_CNT2 (PIT_BASE_ADR), MSB(tc2));
#ifdef	INCLUDE_TIMESTAMP_TSC
	sysTimestampRoutine = (FUNCPTR)pentiumTscReset;
#endif	/* INCLUDE_TIMESTAMP_TSC */
#endif	/* INCLUDE_TIMESTAMP */

        intUnlock (oldLevel);				/* UNLOCK INTERRUPT */

	/* enable clock interrupt */

	sysIntEnablePIC (PIT0_INT_LVL);
	
	sysClkRunning = TRUE;

#if	defined (INCLUDE_TIMESTAMP) && defined (INCLUDE_TIMESTAMP_TSC)
	if (sysTimestampFreqValue == 0)			/* get TSC freq */
	    {
	    FUNCPTR oldFunc = sysTimestampRoutine;
	    sysTimestampRoutine = (FUNCPTR)sysTimestampFreqGet;
	    taskDelay (sysClkTicksPerSecond + 5);	/* wait 1 sec */
	    sysTimestampRoutine = oldFunc;
	    }
#endif	/* defined (INCLUDE_TIMESTAMP) && defined (INCLUDE_TIMESTAMP_TSC) */
	}
    }


/*******************************************************************************
*
* sysClkRateGet - get the system clock rate
*
* This routine returns the system clock rate.
*
* RETURNS: The number of ticks per second of the system clock.
*
* SEE ALSO: sysClkEnable(), sysClkRateSet()
*/

int sysClkRateGet (void)
    {
    return (sysClkTicksPerSecond);
    }


/*******************************************************************************
*
* sysClkRateSet - set the system clock rate
*
* This routine sets the interrupt rate of the system clock.
* It is called by usrRoot() in usrConfig.c.
*
* RETURNS: OK, or ERROR if the tick rate is invalid or the timer cannot be set.
*
* SEE ALSO: sysClkEnable(), sysClkRateGet()
*/

STATUS sysClkRateSet
    (
    int ticksPerSecond	    /* number of clock interrupts per second */
    )
    {
    if (ticksPerSecond < SYS_CLK_RATE_MIN || ticksPerSecond > SYS_CLK_RATE_MAX)
	return (ERROR);

    sysClkTicksPerSecond = ticksPerSecond;

    if (sysClkRunning)
	{
	sysClkDisable ();
	sysClkEnable ();
	}

    return (OK);
    }

#ifdef	PIT1_FOR_AUX

/*******************************************************************************
*
* sysAuxClkInt - handle an auxiliary clock interrupt
*
* This routine handles an auxiliary clock interrupt.  It acknowledges the
* interrupt and calls the routine installed by sysAuxClkConnect().
*
* RETURNS: N/A
*/

LOCAL void sysAuxClkInt (void)
    {

    /* call auxiliary clock service routine */

    if (sysAuxClkRoutine != NULL)
	(*sysAuxClkRoutine) (sysAuxClkArg);
    }

/*******************************************************************************
*
* sysAuxClkConnect - connect a routine to the auxiliary clock interrupt
*
* This routine specifies the interrupt service routine to be called at each
* auxiliary clock interrupt.  It does not enable auxiliary clock interrupts.
*
* RETURNS: OK, or ERROR if the routine cannot be connected to the interrupt.
*
* SEE ALSO: intConnect(), sysAuxClkEnable()
*/

STATUS sysAuxClkConnect
    (
    FUNCPTR routine,    /* routine called at each aux clock interrupt    */
    int arg             /* argument to auxiliary clock interrupt routine */
    )
    {
    sysAuxClkRoutine	= routine;
    sysAuxClkArg	= arg;
    sysAuxClkConnected	= TRUE;

    return (OK);
    }

/*******************************************************************************
*
* sysAuxClkDisable - turn off auxiliary clock interrupts
*
* This routine disables auxiliary clock interrupts.
*
* RETURNS: N/A
*
* SEE ALSO: sysAuxClkEnable()
*/

void sysAuxClkDisable (void)
    {
    if (sysAuxClkRunning)
        {
	sysOutByte (PIT_CMD (PIT_BASE_ADR), 0x78);
	sysOutByte (PIT_CNT1 (PIT_BASE_ADR), LSB(0));
	sysOutByte (PIT_CNT1 (PIT_BASE_ADR), MSB(0));

	sysIntDisablePIC (PIT1_INT_LVL);
	
	sysAuxClkRunning = FALSE;
        }
    }

/*******************************************************************************
*
* sysAuxClkEnable - turn on auxiliary clock interrupts
*
* This routine enables auxiliary clock interrupts.
*
* RETURNS: N/A
*
* SEE ALSO: sysAuxClkConnect(), sysAuxClkDisable(), sysAuxClkRateSet()
*/

void sysAuxClkEnable (void)
    {
    UINT tc;

    if (!sysAuxClkRunning)
	{
	tc = PIT_CLOCK / sysAuxClkTicksPerSecond;

	sysOutByte (PIT_CMD (PIT_BASE_ADR), 0x76);
	sysOutByte (PIT_CNT1 (PIT_BASE_ADR), LSB(tc));
	sysOutByte (PIT_CNT1 (PIT_BASE_ADR), MSB(tc));

	/* enable clock interrupt */

	sysIntEnablePIC (PIT1_INT_LVL);
	
	sysAuxClkRunning = TRUE;
	}
    }

/*******************************************************************************
*
* sysAuxClkRateGet - get the auxiliary clock rate
*
* This routine returns the interrupt rate of the auxiliary clock.
*
* RETURNS: The number of ticks per second of the auxiliary clock.
*
* SEE ALSO: sysAuxClkEnable(), sysAuxClkRateSet()
*/

int sysAuxClkRateGet (void)
    {
    return (sysAuxClkTicksPerSecond);
    }

/*******************************************************************************
*
* sysAuxClkRateSet - set the auxiliary clock rate
*
* This routine sets the interrupt rate of the auxiliary clock.  It does not
* enable auxiliary clock interrupts.
*
* RETURNS: OK, or ERROR if the tick rate is invalid or the timer cannot be set.
*
* SEE ALSO: sysAuxClkEnable(), sysAuxClkRateGet()
*/

STATUS sysAuxClkRateSet
    (
    int ticksPerSecond  /* number of clock interrupts per second */
    )
    {
    if (ticksPerSecond < AUX_CLK_RATE_MIN || ticksPerSecond > AUX_CLK_RATE_MAX)
        return (ERROR);

    sysAuxClkTicksPerSecond = ticksPerSecond;

    if (sysAuxClkRunning)
	{
	sysAuxClkDisable ();
	sysAuxClkEnable ();
	}

    return (OK);
    }

#else /*PIT1_FOR_AUX*/ /* RTC is used for Aux Clock */

#ifndef INCLUDE_RTC 	

/*******************************************************************************
*
* sysAuxClkInt - handle an auxiliary clock interrupt
*
* This routine handles an auxiliary clock interrupt.  It acknowledges the
* interrupt and calls the routine installed by sysAuxClkConnect().
*
* RETURNS: N/A
*/

LOCAL void sysAuxClkInt (void)
    {
    int oldLevel;

    /* acknowledge the interrupt */

    oldLevel = intLock ();				/* LOCK INTERRUPT */
    sysOutByte (RTC_INDEX, 0x0c);
    sysInByte (RTC_DATA); 
    intUnlock (oldLevel);				/* UNLOCK INTERRUPT */

    /* call auxiliary clock service routine */

    if (sysAuxClkRoutine != NULL)
	(*sysAuxClkRoutine) (sysAuxClkArg);
    }

#endif /* not defined INCLUDE_RTC */

/***************************************************************************
*
* sysAuxClkConnect - connect a routine to the auxiliary clock interrupt
*
* This routine specifies the interrupt service routine to be called at each
* auxiliary clock interrupt.  It does not enable auxiliary clock interrupts.
*
* RETURNS: OK, or ERROR if the routine cannot be connected to the interrupt.
*
* SEE ALSO: intConnect(), sysAuxClkEnable()
*/

STATUS sysAuxClkConnect
    (
    FUNCPTR routine,    /* routine called at each aux clock interrupt */
    int arg             /* argument to auxiliary clock interrupt routine */
    )
    {
    sysAuxClkRoutine	= routine;
    sysAuxClkArg	= arg;
    sysAuxClkConnected	= TRUE;

    return (OK);
    }

/***************************************************************************
*
* sysAuxClkDisable - turn off auxiliary clock interrupts
*
* This routine disables auxiliary clock interrupts.
*
* RETURNS: N/A
*
* SEE ALSO: sysAuxClkEnable()
*/

void sysAuxClkDisable (void)
    {
    UCHAR statusReg;
    if (sysAuxClkRunning)
        {

	/* disable generation of periodic interrupt by the RTC */

        statusReg = sysNvRead (REGB);
	sysNvWrite (REGB,statusReg & ~CMOS_RTC_B_PIE );

#ifndef INCLUDE_RTC

	/* turn off interrupts from other RTC sources, too */

	sysNvWrite (REGB, CMOS_RTC_B_24_12);

        sysIntDisablePIC (RTC_INT_LVL);
#endif
	sysAuxClkRunning = FALSE;

        }
    }

/***************************************************************************
*
* sysAuxClkEnable - turn on auxiliary clock interrupts
*
* This routine enables auxiliary clock interrupts.
*
* RETURNS: N/A
*
* SEE ALSO: sysAuxClkConnect(), sysAuxClkDisable(), sysAuxClkRateSet()
*/

void sysAuxClkEnable (void)
    {
    int ix;
    UCHAR statusReg;

    if (!sysAuxClkRunning)
        {

	/* set an interrupt rate */

	for (ix = 0; ix < NELEMENTS (auxTable); ix++)
	    {
	    if (auxTable [ix].rate == sysAuxClkTicksPerSecond)
		{

		/* modify the Rate Selection Bits only */

                statusReg = sysNvRead (REGA) & 0xf0;
                sysNvWrite (REGA,statusReg | auxTable [ix].bits);
		break;
		}
	    }

	/* enable generation of periodic interrupt by the RTC */

        statusReg = sysNvRead (REGB);
	sysNvWrite (REGB,statusReg | CMOS_RTC_B_PIE );

#ifndef INCLUDE_RTC

	/* must turn off other intr sources */

	sysNvWrite (REGB,CMOS_RTC_B_PIE | CMOS_RTC_B_24_12);

        /* start the timer */

        sysIntEnablePIC (RTC_INT_LVL);
#endif

	sysAuxClkRunning = TRUE;
	}
    }

/***************************************************************************
*
* sysAuxClkRateGet - get the auxiliary clock rate
*
* This routine returns the interrupt rate of the auxiliary clock.
*
* RETURNS: The number of ticks per second of the auxiliary clock.
*
* SEE ALSO: sysAuxClkEnable(), sysAuxClkRateSet()
*/

int sysAuxClkRateGet (void)
    {
    return (sysAuxClkTicksPerSecond);
    }

/***************************************************************************
*
* sysAuxClkRateSet - set the auxiliary clock rate
*
* This routine sets the interrupt rate of the auxiliary clock.  It does not
* enable auxiliary clock interrupts.
*
* RETURNS: OK, or ERROR if the tick rate is invalid or the timer cannot be set.
*
* SEE ALSO: sysAuxClkEnable(), sysAuxClkRateGet()
*/

STATUS sysAuxClkRateSet
    (
    int ticksPerSecond  /* number of clock interrupts per second */
    )
    {
    int		ix;	/* hold temporary variable */
    BOOL	match;	/* hold the match status */

    match = FALSE; 	/* initialize to false */

    if (ticksPerSecond < AUX_CLK_RATE_MIN || 
	ticksPerSecond > AUX_CLK_RATE_MAX)
        return (ERROR);

    for (ix = 0; ix < NELEMENTS (auxTable); ix++)
	{
	if (auxTable [ix].rate == ticksPerSecond)
	    {
	    sysAuxClkTicksPerSecond = ticksPerSecond;
	    match = TRUE;
	    break;
	    }
	}

    if (!match)		/* ticksPerSecond not matching the values in table */
       return (ERROR);

    if (sysAuxClkRunning)
	{
	sysAuxClkDisable ();
	sysAuxClkEnable ();
	}

    return (OK);
    }

#endif	/* PIT1_FOR_AUX */

#ifdef	INCLUDE_TIMESTAMP

/*******************************************************************************
*
* sysTimestampConnect - connect a user routine to the timestamp timer interrupt
*
* This routine specifies the user interrupt routine to be called at each
* timestamp timer interrupt.  It does not enable the timestamp timer itself.
*
* RETURNS: OK, or ERROR if sysTimestampInt() interrupt handler is not used.
*/
 
STATUS sysTimestampConnect
    (
    FUNCPTR routine,	/* routine called at each timestamp timer interrupt */
    int arg		/* argument with which to call routine */
    )
    {
    return (ERROR);
    }

/*******************************************************************************
*
* sysTimestampEnable - initialize and enable the timestamp timer
*
* This routine connects the timestamp timer interrupt and initializes the
* counter registers.  If the timestamp timer is already running, this routine
* merely resets the timer counter.
*
* The rate of the timestamp timer should be set explicitly within the BSP,
* in the sysHwInit() routine.  This routine does not intialize the timer
* rate.  
*
* RETURNS: OK, or ERROR if the timestamp timer cannot be enabled.
*/
 
STATUS sysTimestampEnable (void)
    {
    if (sysTimestampRunning)
	return (OK);
    
    sysTimestampRunning = TRUE;

    return (OK);
    }

/********************************************************************************
* sysTimestampDisable - disable the timestamp timer
*
* This routine disables the timestamp timer.  Interrupts are not disabled,
* although the tick counter will not increment after the timestamp timer
* is disabled, thus interrupts will no longer be generated.
*
* RETURNS: OK, or ERROR if the timestamp timer cannot be disabled.
*/
 
STATUS sysTimestampDisable (void)
    {
    if (sysTimestampRunning)
	{
        sysTimestampRunning = FALSE;
	}

    return (ERROR);
    }

/*******************************************************************************
*
* sysTimestampPeriod - get the timestamp timer period
*
* This routine returns the period of the timestamp timer in ticks.
* The period, or terminal count, is the number of ticks to which the timestamp
* timer will count before rolling over and restarting the counting process.
*
* RETURNS: The period of the timestamp timer in counter ticks.
*/
 
UINT32 sysTimestampPeriod (void)
    {
    sysTimestampPeriodValue = sysTimestampFreq () / sysClkTicksPerSecond;

    return (sysTimestampPeriodValue);
    }

/*******************************************************************************
*
* sysTimestampFreq - get the timestamp timer clock frequency
*
* This routine returns the frequency of the timer clock, in ticks per second.
* The rate of the timestamp timer should be set explicitly within the BSP,
* in the sysHwInit() routine.
*
* RETURNS: The timestamp timer clock frequency, in ticks per second.
*/
 
UINT32 sysTimestampFreq (void)
    {
#ifdef	INCLUDE_TIMESTAMP_TSC
    return (sysTimestampFreqValue);
#else
    return (PIT_CLOCK * 2);
#endif	/* INCLUDE_TIMESTAMP_TSC */
    }
 
#ifdef	INCLUDE_TIMESTAMP_TSC
LOCAL void sysTimestampFreqGet (void)
    {
    if (sysTimestampTickCount == sysClkTicksPerSecond)
	{
	sysTimestampFreqValue = pentiumTscGet32 ();
	sysTimestampTickCount = 0;
	}
    if (sysTimestampTickCount == 0)
	pentiumTscReset ();
    sysTimestampTickCount++;
    }
#endif	/* INCLUDE_TIMESTAMP_TSC */

/*******************************************************************************
*
* sysTimestamp - get the timestamp timer tick count
*
* This routine returns the current value of the timestamp timer tick counter.
* The tick count can be converted to seconds by dividing by the return of
* sysTimestampFreq().
*
* This routine should be called with interrupts locked.  If interrupts are
* not already locked, sysTimestampLock() should be used instead.
*
* RETURNS: The current timestamp timer tick count.
*
* SEE ALSO: sysTimestampLock()
*/
 
UINT32 sysTimestamp (void)
    {
#ifdef	INCLUDE_TIMESTAMP_TSC
    return (pentiumTscGet32 ());
#else
    union uc uc;

    sysOutByte (PIT_CMD (PIT_BASE_ADR), 0x80);
    uc.count08[0] = sysInByte (PIT_CNT2 (PIT_BASE_ADR));
    uc.count08[1] = sysInByte (PIT_CNT2 (PIT_BASE_ADR));

    return (sysTimestampPeriodValue - uc.count16);
#endif	/* INCLUDE_TIMESTAMP_TSC */
    }

/*******************************************************************************
*
* sysTimestampLock - get the timestamp timer tick count
*
* This routine returns the current value of the timestamp timer tick counter.
* The tick count can be converted to seconds by dividing by the return of
* sysTimestampFreq().
*
* This routine locks interrupts for cases where it is necessary to stop the
* tick counter in order to read it, or when two independent counters must
* be read.  If interrupts are already locked, sysTimestamp() should be
* used instead.
*
* RETURNS: The current timestamp timer tick count.
*
* SEE ALSO: sysTimestamp()
*/
 
UINT32 sysTimestampLock (void)
    {
#ifdef	INCLUDE_TIMESTAMP_TSC
    /* pentiumTscGet32() has just one instruction "rdtsc", so locking
     * the interrupt is not necessary.
     */
    return (pentiumTscGet32 ());
#else
    int oldLevel;
    union uc uc;

    oldLevel = intLock ();
    sysOutByte (PIT_CMD (PIT_BASE_ADR), 0x80);
    uc.count08[0] = sysInByte (PIT_CNT2 (PIT_BASE_ADR));
    uc.count08[1] = sysInByte (PIT_CNT2 (PIT_BASE_ADR));
    intUnlock (oldLevel);

    return (sysTimestampPeriodValue - uc.count16);
#endif	/* INCLUDE_TIMESTAMP_TSC */
    }

#endif	/* INCLUDE_TIMESTAMP */

