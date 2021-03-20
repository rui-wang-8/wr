/* loApicTimer.c - Intel PentiumPro Local APIC timer library */

/* Copyright 1984-1998 Wind River Systems, Inc. */
/* Copyright 1999 Motorola, Inc.  All Rights Reserved */
#include "copyright_wrs.h"

/*
modification history
--------------------
01c,19jul99,sjb  changed the RTC version of the Aux Clk code to be 
		 compatible with other Real Time Clock activity. 
01b,21may98,hdn  re-written
01a,13jun97,sub  written
*/

/*
DESCRIPTION
This library contains routines for the  timer on the Intel PentiumPro's
Local APIC (Advanced  Programmable Interrupt Controller).  This library 
handles both the system clock and the auxiliary clock functions.  It 
also contains code for the TimeStamp Counter in the Pentium Processor. 
It also contains code for the periodic interrupt of the CMOS Real 
Time Clock (RTC) device.

Local APIC contains a 32-bit programmable timer for use by the local
processor.  This timer is configured through the timer register in the local
vector table.  The time base is derived from the processor's bus clock, 
divided by a value specified in the divide configuration register. 
After reset, the timer is initialized to zero.  The timer supports one-shot
and periodic modes.  The timer can be configured to interrupt the local
processor with an arbitrary vector.
This library get the system clock from the Local APIC timer and auxClock
from either RTC or PIT channel 0 (define PIT0_FOR_AUX in the BSP).
The macro TIMER_CLOCK_HZ must also be defined to indicate the clock 
frequency of the Local APIC Timer.
The macros SYS_CLK_RATE_MIN, SYS_CLK_RATE_MAX, AUX_CLK_RATE_MIN, and
AUX_CLK_RATE_MAX must be defined to provide parameter checking for the
sys[Aux]ClkRateSet() routines.

This driver uses PentiumPro's onchip TSC (Time Stamp Counter) for the
time stamp driver.
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

The system clock is in the APIC.  The Auxiliary clock can be either 
the first timer in the i8253 (PIT0), or the Periodic Interrupt 
of the CMOS Real Time Clock device. The setting of PIT0_FOR_AUX 
controls which device is Aux Clock.

*/

/* includes */

#include "drv/intrCtl/loApic.h"

#ifndef PIT0_FOR_AUX
#include "sysRtc.h"  
#endif /* PIT0_FOR_AUX */

/* These are exported because the RTC code uses them in the ISR */

FUNCPTR sysAuxClkRoutine	= NULL;
int sysAuxClkRunning		= FALSE;
int sysAuxClkArg		= NULL;

/* locals */

LOCAL FUNCPTR sysClkRoutine	= NULL; /* routine to call on clock interrupt */
LOCAL int sysClkArg		= NULL; /* its argument */
LOCAL int sysClkRunning		= FALSE;
LOCAL int sysClkConnected	= FALSE;
LOCAL int sysClkTicksPerSecond	= 60;

LOCAL int sysAuxClkConnected	= FALSE;

#ifdef	PIT0_FOR_AUX
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
#endif	/* PIT0_FOR_AUX */

#ifdef	INCLUDE_TIMESTAMP 
LOCAL BOOL	sysTimestampRunning	= FALSE; /* running flag */
LOCAL int	sysTimestampPeriodValue	= NULL;	 /* Max counter value */
LOCAL FUNCPTR	sysTimestampRoutine	= NULL;	 /* user rollover routine */
LOCAL int   	sysTimestampTickCount	= 0;	 /* system ticks counter */
LOCAL UINT32	sysTimestampFreqValue	= PENTIUMPRO_TSC_FREQ; /* TSC clock */
LOCAL void	sysTimestampFreqGet (void);
#endif	/* INCLUDE_TIMESTAMP */

/*******************************************************************************
*
* sysClkInt - interrupt level processing for system clock
*
* This routine handles the system clock interrupt.  It is attached to the
* clock interrupt vector by the routine sysClkConnect().
*/

LOCAL void sysClkInt (void)
    {
    if (sysClkRoutine != NULL)
	(* sysClkRoutine) (sysClkArg);

#ifdef	INCLUDE_TIMESTAMP 
    if (sysTimestampRoutine != NULL)
	(* sysTimestampRoutine) ();
#endif	/* INCLUDE_TIMESTAMP */
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
        oldLevel = intLock ();	/* LOCK INTERRUPT */

	/* mask interrupt and reset the Initial Counter */

	*(int *)(loApicBase + LOAPIC_TIMER) |= LOAPIC_MASK;
	*(int *)(loApicBase + LOAPIC_TIMER_ICR) = 0;

        intUnlock (oldLevel);	/* UNLOCK INTERRUPT */
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
    int oldLevel;
    int count;

    if (!sysClkRunning)
	{

	count = ((TIMER_CLOCK_HZ) / sysClkTicksPerSecond) - 1;

        oldLevel = intLock ();	/* LOCK INTERRUPT */

	/* set a divide value in the Configuration Register */

	*(int *)(loApicBase + LOAPIC_TIMER_CONFIG) = 
	(*(int *)(loApicBase + LOAPIC_TIMER_CONFIG) & ~0xf) | 
				          LOAPIC_TIMER_DIVBY_1;

	/* set a initial count in the Initial Count Register */

	*(int *)(loApicBase + LOAPIC_TIMER_ICR) = count ;

	/* set the vector in the local vector table (Timer) */

	*(int *)(loApicBase + LOAPIC_TIMER) = 
	(*(int *)(loApicBase + LOAPIC_TIMER) & 0xfffcff00) |
			 TIMER_INT_VEC | LOAPIC_TIMER_PERIODIC;

#ifdef	INCLUDE_TIMESTAMP 
	sysTimestampRoutine = (FUNCPTR)pentiumTscReset;
#endif	/* INCLUDE_TIMESTAMP */

        intUnlock (oldLevel);	/* UNLOCK INTERRUPT */	

	sysClkRunning = TRUE;

#ifdef	INCLUDE_TIMESTAMP 
	if (sysTimestampFreqValue == 0)			/* get TSC freq */
	    {
	    FUNCPTR oldFunc = sysTimestampRoutine;
	    sysTimestampRoutine = (FUNCPTR)sysTimestampFreqGet;
	    taskDelay (sysClkTicksPerSecond + 5);	/* wait 1 sec */
	    sysTimestampRoutine = oldFunc;
	    }
#endif	/* INCLUDE_TIMESTAMP */
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


#ifdef	PIT0_FOR_AUX

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
	sysOutByte (PIT_CMD (PIT_BASE_ADR), 0x38);
	sysOutByte (PIT_CNT0 (PIT_BASE_ADR), LSB(0));
	sysOutByte (PIT_CNT0 (PIT_BASE_ADR), MSB(0));

	sysIntDisablePIC (PIT0_INT_LVL);
	
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

	sysOutByte (PIT_CMD (PIT_BASE_ADR), 0x36);
	sysOutByte (PIT_CNT0 (PIT_BASE_ADR), LSB(tc));
	sysOutByte (PIT_CNT0 (PIT_BASE_ADR), MSB(tc));

	/* enable clock interrupt */

	sysIntEnablePIC (PIT0_INT_LVL);
	
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

#else	/* PIT0_FOR_AUX */

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

#endif /* INCLUDE_RTC */

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
	sysNvWrite (REGB, statusReg & ~CMOS_RTC_B_PIE);

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
#endif	/* PIT0_FOR_AUX */


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
    return (sysTimestampFreqValue);
    }
 
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
    return (pentiumTscGet32 ());
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
    /* 
     * pentiumTscGet32() has just one instruction "rdtsc", so locking
     * the interrupt is not necessary. 
     */

    return (pentiumTscGet32 ());
    }

#endif	/* INCLUDE_TIMESTAMP */

