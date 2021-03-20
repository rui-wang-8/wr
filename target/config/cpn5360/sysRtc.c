/* sysRtc.c - Support routines for the Real Time Clock Device */

/* Copyright 1999 Motorola, Inc.  All Rights Reserved */

/*
modification history
--------------------
01b,01feb00,djs  incorporate WRS review comments.
01a,19Jul99,sjb  Initial Development 
*/

/*
DESCRIPTION

This code provides support routines for the Real Time
Clock.  The RTC is part of the the Standard Microsystems Corporation 
Ultra I/O Real Time Clock, which is also known as the FDC37C93x.   
This chip is compatible with other available chips, which all seem 
to implement pretty much the same Real Time Clock/Calendar 
programmatic interface and functionality.

GENERAL DESCRIPTION OF RTC

TIMEKEEPING

The RTC device updates its time/date registers once per second.
During the update, the time/date registers are invalid.  A
status register bit, Update Finished (UF), and a corresponding
UF Interrupt, is set immediately following the update cycle.  
A window of 999msec follows, during which the time/date 
registers are accurate.  The UF bit is cleared (only) when read.
Given this chip design, the validity of the time/date registers 
can be assumed only if we can respond quickly to UF.  Therefore, 
the actual read of the time/date info from the chip is done in 
an intr service routine.  ( The only way to ensure validity of 
the time being read in non-intr code would be to throw up the 
mask, clear the UF flag, and spin until the UF flag becomes 
newly true.  This could take up to 999msec, an undesireable 
approach.) 

A global time/date structure is declared in sysRtc.c.  
THIS STRUCTURE IS WRITTEN ONLY BY THE ISR.   It is 
copied by the sysRtcDateTimeGet routine into the user's
time/date structure when the user requests the time.

A local date/time structure, (struct RTC_DATE_TIME), should be 
declared by the user.  This is the structure whose addr is 
expected as an argument to the sysRtcDateTimeGet and 
sysRtcDateTimeSet routines.   

The globalTimeBuf struct pointer must not be passed to either 
sysRtcDateTimeGet or sysRtcDateTimeSet.

ALARMS

The RTC device provides a Time of Day alarm, which can be set
to generate a flag, or flag and interrupt, on an interval 
range of once per second to once per day.  The alarm condition
is determined by the chip to be true whenever the contents
of the time registers "match" the alarm time registers.
This matching includes "don't care" ability -- the user 
can configure the chip such that ANY value of current
time hour, minute, or second is considered a "match".

The "don't care" feature is lightly described in the chip
documentation.  Testing has shown the following responses
based on these sample alarm time values:

.CS
alarm_time.hour		=  255
alarm_time.minute	=  255
alarm_time.second	=  10
Effect:		   an alarm on the 10th second of every minute

alarm_time.hour		=  255
alarm_time.minute	=  255
alarm_time.second	=  255
Effect:		   an alarm every second

alarm_time.hour		=  2
alarm_time.minute	=  255
alarm_time.second	=  17
Effect:		   An alarm on the 17th second of every minute
		   in the 2nd hour of each day.  No alarm intr or alarm
		   flag at any other time.

alarm_time.hour		=  9
alarm_time.minute	=  8
alarm_time.second	=  7
Effect:		   An alarm intr on the 7th second of the 8th minute
		   of the 9th hour of each day.
.CE

RTC INITIALIZATION

sysRtcInit initializes the RTC device.  It configures the chip, 
for these modes: BCD / 24-hour / Daylight Savings Disabled / 
Update Ended Intr Enabled.   sysRtcInit is called from 
sysHwInit2, before the intConnect call for the Aux Clock.  

ERROR REPORTING

The source of RTC reported errors is the check for validity 
of the date/time.  This validity check function is called by 
sysRtcInit, sysRtcDateTimeGet, and sysRtcDateTimeSet.  
Throughout the design, an ERROR result indicates an error.  
If sysRtcDateTimeTest returns ERROR, the error continues to
be passed up to the application which called the service.  
sysRtcDateTimeTest checks all fields in the date/time structure 
for validity, including a check for the proper range of days 
for each particular month, ie maximum of 30 days in April, 
31 in August, etc.  It also handles the 29-day February in 
leap years.  

INTERRUPT SERVICE ROUTINE

The ISR, sysAuxRtcInt, has 3 functions.

1) The ISR keeps the global time/date structure up to date. 
2) The ISR monitors for alarm intrs.  The alarm intr portion 
   of the isr will call the routine specified by the user's 
   sysRtcAlarmConnect call. 
3) The ISR responds to the Periodic Interrupt of the
   RTC, which can be used as the VxWorks Auxilliary Clock.  
   The Auxilliary Clock portion of the ISR will call the routine
   specified by the user's sysAuxClkConnect call.

INCLUDE FILES:  sysRtc.h

*/

/* includes */

#include "sysRtc.h"

/* This struct should only be written to by the ISR!  */

static RTC_DATE_TIME globalTimeBuf;

LOCAL FUNCPTR sysRtcAlarmRoutine  = NULL;
LOCAL int sysRtcAlarmArg          = NULL;
LOCAL int sysRtcAlarmConnected    = NULL;

/* forward declarations */

STATUS sysRtcDateTimeTest (RTC_DATE_TIME *timecal);
STATUS sysRtcDateTimeSet (RTC_DATE_TIME *timecal);
void  sysAuxRtcInt (RTC_DATE_TIME *timecal);
STATUS sysRtcInit (void);
void sysRtcDateTimePrint (RTC_DATE_TIME *timecal);
STATUS sysRtcDateTimeGet (RTC_DATE_TIME *timecal);
STATUS sysRtcAlarmSet (UCHAR method, RTC_DATE_TIME *a_time);
STATUS sysRtcAlarmCancel (void);
STATUS sysRtcAlarmConnect (FUNCPTR routine, int arg);

/* ranges of acceptable values for RTC registers */

static struct rtcdt leastClockVal =  
    { 
    (int) 1987, 
    (int) 1, 
    (int) 1,
    (int) 0,
    (int) 0,
    (int) 0
    };

static struct rtcdt mostClockVal =
    {
    (int) 2086,
    (int) 12,
    (int) 31,
    (int) 23,
    (int) 59,
    (int) 59
    };

/* months are 1-12, ignore element 0 */

static const char mostDayVal[] = { 0,31,28,31,30,31,30,31,31,30,31,30,31 };

/***************************************************************************
*
* sysRtcDateTimeHook - this is a hook routine for the dosFsLib.  
*
* This routine fills in a DOS_DATE_TIME structure with the current Date 
* and Time.  This is used by the standard VxWorks dosFsLib as a way to
* update the date/time values used for file timestamps.  This is
* "hooked" into the dosFsLib during board initialization if the both
* the dosFsLib and the RTC support is included in the BSP.
* 
* RETURNS: void
*
* SEE ALSO: dosFsDateTimeInstall(), and the dosFsLib documentation.
*/
void sysRtcDateTimeHook
    (
    DOS_DATE_TIME *pDateTime	/* pointer to time keeping structure */
    )
    {
    (void) sysRtcDateTimeGet ((RTC_DATE_TIME *) pDateTime);
    }

/***************************************************************************
*
* sysRtcDateTimeTest - Tests whether date/time values are valid 
*
* The source of RTC reported errors is the check for validity 
* of the date/time.  This validity check function is called by 
* sysRtcInit(), sysRtcDateTimeGet(), and sysRtcDateTimeSet().  
* sysRtcDateTimeTest() checks all fields in the date/time structure 
* for validity, including a check for the proper range of days 
* for each particular month, ie maximum of 30 days in April, 
* 31 in August, etc.  It also handles the 29-day February in 
* leap years.  
*
* RETURNS:  OK, or ERROR if the date/time structure values are invalid
*/

STATUS sysRtcDateTimeTest
    (
    RTC_DATE_TIME *timecal	/* pointer to time keeping structure */
    )
    {

    if (timecal->second < leastClockVal.second || 
        timecal->second > mostClockVal.second)
        {
        return (ERROR);
        }
    if (timecal->minute < leastClockVal.minute || 
        timecal->minute > mostClockVal.minute)
        {
        return (ERROR);
        }
    if (timecal->hour < leastClockVal.hour || 
        timecal->hour > mostClockVal.hour)
        {
        return (ERROR);
        }
    if (timecal->day < leastClockVal.day || 
        timecal->day > mostClockVal.day)
        {
        return (ERROR);
        }
    if (timecal->month < leastClockVal.month || 
        timecal->month > mostClockVal.month)
        {
        return (ERROR);
        }
    if (timecal->year < leastClockVal.year || 
        timecal->year > mostClockVal.year)
        {
        return (ERROR);
        }

    /* now check that the day is within range for chosen month */

    if (timecal->month != 2)
        { 
        /* all months but Feb have a fixed number of days */

        if (timecal->day > mostDayVal [timecal->month])
            {
            return (ERROR);
            }
        } 
    else 
	{
        /* Month is Feb... check if this is a leap year. */
        if (timecal->year % 4)
            {
            /* not a leap year */

            if (timecal->day > mostDayVal [timecal->month])
                {
                return (ERROR);
                }
            } 
        else 
	    {
            /* leap year */

            if (timecal->day > mostDayVal [timecal->month] + 1)
                {
                return (ERROR);
                }
            }
        }
    return (OK);
    }

/***************************************************************************
*
* sysRtcDateTimeSet - sets the RTC's date/time per caller's values
*
* sysRtcDateTimeSet allows the caller to set the RTC time and date.  
* The caller must allocate space for an RTC_DATE_TIME structure, fill 
* the structure with the desired time/date, and call this routine.   
* The passed in time and date are verified to be within proper range.
*
* NOTE: DO NOT use globalTimeBuf pointer as the parameter 
* passed to this routine.
*
* RETURNS:  OK, or ERROR if the date/time structure values are invalid
*/
STATUS sysRtcDateTimeSet
    (
    RTC_DATE_TIME *timecal	/* pointer to time keeping structure */
    ) 
    {
    UCHAR regb_val;
    int ospl;                 /* Saved interrupt level */

    /* Validate the incoming time selection.                    */

    if (sysRtcDateTimeTest (timecal) == ERROR)  
        {
        return (ERROR);
        } 

    /* mask has to be up because intr handler uses ports 70 & 71, too */

    ospl = intLock ();

    /* read REGB */

    regb_val = sysNvRead (REGB);

    /* set the SET bit in reg B */

    sysNvWrite (REGB, regb_val | CMOS_RTC_B_SET);

    /* now the time/date can be written into chip regs */

    /*  isolate the seconds from RTC_DATE_TIME struct, write to SEC offset */

    sysNvWrite (SEC, BIN_TO_BCD (timecal->second));   

    /*  isolate the minutes from RTC_DATE_TIME struct, write to MIN offset */

    sysNvWrite (MIN, BIN_TO_BCD (timecal->minute));   

    /*  isolate the hours from RTC_DATE_TIME struct, write to HOUR offset */

    sysNvWrite (HOUR, BIN_TO_BCD (timecal->hour));   

    /*  isolate day of month from RTC_DATE_TIME struct, write to DOM offset */

    sysNvWrite (DOM, BIN_TO_BCD (timecal->day));   

    /*  isolate the month from RTC_DATE_TIME struct, write to MONTH offset */

    sysNvWrite (MONTH, BIN_TO_BCD (timecal->month));   

    /*  isolate the year from RTC_DATE_TIME struct, write to YEAR offset */

    sysNvWrite (YEAR, BIN_TO_BCD ((timecal->year % 100)));   

    /*
     *  -- register b gets set to its original value 
     * Note: this will restore the UIE bit, which is cleared when the 
     *       SET  bit goes high.   
     */

    sysNvWrite (REGB, regb_val);

    intUnlock (ospl);

    return (OK);
    }

/***************************************************************************
*
* sysAuxRtcInt  - the RTC interrupt service routine
*
* This is the interrupt service routine for the RTC.  It processes the 
* RTC's Update Finished Interrupt once per second.  It writes the current 
* time/date information into the global RTC_DATE_TIME struct, 
* globalTimeBuf.  A 2-digit century is added to the 2-digit year value 
* returned by the chip.  The ISR also executes the user's Alarm interrupt 
* service code, if it has been hooked in with a call to sysRtcAlarmConnect.   
* The ISR also executes the user's Aux Clock interrupt service code, if 
* it has been hooked in with a call to sysAuxClkConnect.  
* 
* RETURNS: void
*/

void sysAuxRtcInt
    (
    RTC_DATE_TIME *timecal	/* pointer to time keeping structure */
    ) 
    {
    UCHAR regc_val;

    regc_val = sysNvRead (REGC);
  
    /* Update Ended Intr flag check */

    if (regc_val & CMOS_RTC_C_UF)
        {

	/*
         * The time/cal update has just finished.  Quick, grab a copy of 
         * the time/cal info while it is still valid.  (It is invalid during
         * the update cycle of the RTC which happens once per second.) 
	 */

        /* read seconds, convert to binary, store in struct */

        timecal->second = BCD_TO_BIN (sysNvRead (SEC));

        /* read minutes */

        timecal->minute = BCD_TO_BIN (sysNvRead (MIN));

        /* read hours */

        timecal->hour = BCD_TO_BIN ( sysNvRead (HOUR));

        /* read day of month */

        timecal->day = BCD_TO_BIN (sysNvRead (DOM));

        /* read month */

        timecal->month = BCD_TO_BIN (sysNvRead (MONTH));

        /* read year */

        timecal->year = BCD_TO_BIN (sysNvRead (YEAR));

        /* Provide a century value because chip doesn't */

        if (timecal->year <= 86)
            {
            timecal->year += 2000;
            } 
	else 
	    {
            timecal->year += 1900;
            }
        }

#ifdef RTC_FOR_AUX

    if (regc_val & CMOS_RTC_C_PF )
	{

        /* Perform whatever auxilliary clock code the user has connected */

        if ((sysAuxClkRunning == TRUE) && (sysAuxClkRoutine != NULL))
	    {
            (*sysAuxClkRoutine) (sysAuxClkArg);
	    }
	}

#endif

    /*  check for the alarm flag */

    if (regc_val & CMOS_RTC_C_AF )
        {
        /*
         * Merely turning off the Alarm Intr Enable bit will not prevent an 
         * Alarm Flag in reg C from being set when the current time and
         * the alarm time match.   So, at LEAST once a day an alarm match
         * will occur, even if the user is not interested in it.  Therefore,
         * we need to check the CMOS_RTC_B_AIE bit to determine whether
         * the alarm is important to the user. 
	 */

        if(sysNvRead(REGB) & CMOS_RTC_B_AIE)
            {

            /* call Alarm service routine */

	    if (sysRtcAlarmRoutine != NULL)
	        {
                sysRtcAlarmRoutine (sysRtcAlarmArg); 
	        }
            } 
        }
    }

/***************************************************************************
*
* sysRtcInit - start the RTC, and enable the RTC's Interrupt 
*
* sysRtcInit(void) initializes the RTC device.  It configures the chip
* for these modes: BCD / 24-hour / Daylight Savings Disabled / 
* Update Ended Intr Enabled.   
*
* RETURNS:  OK, or ERROR if the chip can't be initialized. 
*/

STATUS sysRtcInit(void)
    {
    UCHAR regb_val, regc_val;
    RTC_DATE_TIME timecal;
    int ospl;                  /* Saved interrupt level */
    UINT32 count;

    /* this turns the chip's oscillator on, enables countdown chain */

    sysNvWrite (REGA, CMOS_RTC_A_DV1 );

    /* set the rtc modes */

    sysNvWrite ((REGB), RTC_CLOCK_MODE | RTC_DATA_MODE | RTC_DSE_MODE);

    /*  the mask must be up because ISR uses ports 70/71 too */

    ospl = intLock (); 

    /* enable RTC intr thru the PIC to the processor */

    sysIntEnablePIC (RTC_INT_LVL);

    /* This section enables the Update Finished Intr in RTC */

    /* first, clear pending intrs in regc by reading regc */

    regc_val = sysNvRead (REGC);

    /* read REGB */

    regb_val = sysNvRead (REGB);
    sysNvWrite (REGB, regb_val | CMOS_RTC_B_UIE); 
    intUnlock (ospl); 
    count = 120;   /* wait up to approx .48 sec for chip to get ready */
    while (count > 0)
        {
        if (sysRtcDateTimeGet (&timecal) != ERROR)
            {
            break;
            }
        count--;
	sysNanoDelay (4000000);  /* this reads the isa bus... */
        }

    if (count == 0)
        return (ERROR);

    return (OK);
    }

/***************************************************************************
*
* sysRtcDateTimePrint - show the date/time on the user's display
*
* This routine formats the timecal structure in a user-readable fashion, 
* and sends it to the user's display.  It calls printf to accomplish
* this.  
*
* RETURNS: void
*/

void sysRtcDateTimePrint
    (
    RTC_DATE_TIME *timecal	/* pointer to time keeping structure */
    ) 
    {
    printf ("\n\r            Time: %d:%d:%d  Date: %d/%d/%d\n\r",
	    timecal->hour, timecal->minute, timecal->second, 
	    timecal->month, timecal->day, timecal->year);
    }

/***************************************************************************
* 
* sysRtcDateTimeGet - get current date/time for caller 
*
* This routine allows the caller to obtain the current RTC time and date.  
* The caller must allocate space for a RTC_DATE_TIME structure, then call 
* this routine.  The time and date are verified to be within proper range 
* prior to being written into the caller's structure. 
*
* Warning:   DO  NOT use globalTimeBuf pointer as the parameter passed to 
* this routine!
* 
* RETURNS: OK, or ERROR if current time/date is invalid.          
*/

STATUS sysRtcDateTimeGet
    (
    RTC_DATE_TIME *timecal	/* pointer to time keeping structure */
    ) 
    {
    int ospl;                 /* Saved interrupt level */

    /*  the mask must be up because ISR writes to globalTimeBuf */

    ospl = intLock (); 
  
    /* Validate the time values.  */

    if (sysRtcDateTimeTest (&globalTimeBuf) == ERROR)  
        {

        /* restore intr mask level */

        intUnlock (ospl);
        return (ERROR);
        } 

    /* copy from the ISR's time/date buffer to the user's */

    timecal->second = globalTimeBuf.second;   
    timecal->minute = globalTimeBuf.minute;   
    timecal->hour = globalTimeBuf.hour;   
    timecal->day = globalTimeBuf.day;   
    timecal->month = globalTimeBuf.month;   
    timecal->year = globalTimeBuf.year;   

    /* restore intr mask level */

    intUnlock (ospl);

    return (OK); 
    }

/***************************************************************************
* 
* sysRtcAlarmSet -  Set an Alarm per the caller's settings
*
* This function will enable the (future) generation of repeating alarm 
* interrupts from the Real Time Clock. The alarm interrupt can be 
* programmed in two ways.  The first method provides an alarm at a specific 
* time of day (ALARM_AT). The second method allows for generation of an 
* alarm interrupt every second, minute, or hour (ALARM_EVERY).
* 
* The alarm condition is determined by the chip to be true whenever 
* the contents of the chip's time registers "match" the chip's alarm 
* time registers.  This matching includes "don't care" ability -- the 
* user can configure the chip such that ANY value of current time hour, 
* minute, or second is considered a "match".
* 
* The "don't care" feature is lightly described in the chip
* documentation.  Testing has shown the following responses
* based on these sample alarm time values:
* 
* .CS
* To set an alarm on the 10th second of every minute:
* alarm_time.hour	  = 255
* alarm_time.minute = 255
* alarm_time.second =  10
* 
* To set an alarm every second:
* alarm_time.hour   = 255
* alarm_time.minute = 255
* alarm_time.second = 255
* 
* To set an alarm on the 17th second of every minute in the 2nd hour 
* of each day, no alarm at any other time:
* alarm_time.hour   =    2
* alarm_time.minute =  255
* alarm_time.second =   17
* 
* To set an alarm on the 7th second of the 8th minute of the 9th 
* hour of each day:
* alarm_time.hour   =  9
* alarm_time.minute =  8
* alarm_time.second =  7
* .CE
* 
*
* RETURNS: OK, or ERROR if there is an error in the passed-in values.
*/

STATUS sysRtcAlarmSet
    (
    UCHAR method,		/* set to either ALARM_AT or ALARM_EVERY */
    RTC_DATE_TIME *a_time	/* pointer to time keeping structure */
    )
    { 
    int ospl;                 /* Saved interrupt level */
    UCHAR regb_val;

    if(method != ALARM_EVERY && method != ALARM_AT)
        {
        return (ERROR);
        }

    /* check here for valid time values in a_time for ALARM_AT type */
    /* can't check for ALARM_EVERY... all values (0-255) are valid */
    if(method == ALARM_AT)
        {
        if (a_time->second < leastClockVal.second || 
            a_time->second > mostClockVal.second)
            {
            return (ERROR);
            }
        if (a_time->minute < leastClockVal.minute || 
            a_time->minute > mostClockVal.minute)
            {
            return (ERROR);
            }
        if (a_time->hour < leastClockVal.hour || 
	    a_time->hour > mostClockVal.hour)
            {
            return (ERROR);
            }
    }

    /*  the mask must be up because ISR uses ports 70/71 too */

    ospl = intLock (); 

    /* 
     * read and save the current regb settings, then set the regb SET 
     * bit, and disable the alarm interrupt while writing the new 
     * alarm settings 
     */

    regb_val = sysNvRead (REGB);
    sysNvWrite (REGB, (regb_val & ~CMOS_RTC_B_AIE) | CMOS_RTC_B_SET);

    /* ----- set up for an alarm every so often ----- */

    /* write seconds value from RTC_DATE_TIME struct to alarm reg */

    if ((a_time->second & DONT_CARE_BITS) == DONT_CARE_BITS)
        {

        /* don't try to turn don't care value into BCD */

        sysNvWrite (SEC_ALARM, (a_time->second));   
        } 
    else 
	{
        sysNvWrite (SEC_ALARM, BIN_TO_BCD (a_time->second));   
        }

    /*  write minutes value from RTC_DATE_TIME struct to alarm reg */

    if ((a_time->minute & DONT_CARE_BITS) == DONT_CARE_BITS )
        {

        /* don't try to turn don't care value into BCD */

        sysNvWrite (MIN_ALARM, (a_time->minute));   
        }
    else 
	{
        sysNvWrite (MIN_ALARM, BIN_TO_BCD (a_time->minute));   
        }

    /*  write hours from RTC_DATE_TIME struct to alarm reg */

    if ((a_time->hour & DONT_CARE_BITS ) == DONT_CARE_BITS)
        {

        /* don't try to turn don't care value into BCD */

        sysNvWrite (HOUR_ALARM, (a_time->hour));   
        } 
    else 
	{
        sysNvWrite (HOUR_ALARM, BIN_TO_BCD (a_time->hour));   
        }

    /* enable the alarm interrupt, restore original regb settings */

    sysNvWrite (REGB, regb_val | CMOS_RTC_B_AIE);

    /* restore intr mask level */
    intUnlock (ospl);
    return (OK);
}

/***************************************************************************
* sysRtcAlarmCancel - disable RTC alarms.
* 
* Disable the (future) generation of alarm interrupts from the Real Time 
* Clock.  
*
* RETURNS: OK, or ERROR if alarm interrupt wasn't enabled upon entry.
*/

STATUS sysRtcAlarmCancel(void)
    {
    UCHAR regb_val;
    int ospl;                 /* Saved interrupt level */

    /*  the mask must be up because ISR uses ports 70/71 too */

    ospl = intLock (); 

    /* read and save the current regb settings, disable alarm interrupt */

    regb_val = sysNvRead (REGB);
    if (!(regb_val & CMOS_RTC_B_AIE))
        {
        intUnlock (ospl);
        return (ERROR);
        }

    /*
     * Note that merely turning off the AIE bit will not prevent the
     * Alarm Flag in reg C from being set when the current time and
     * the alarm time match.
     */
    sysNvWrite (REGB, regb_val & ~CMOS_RTC_B_AIE);

    intUnlock (ospl);
    return (OK);
    }

/***************************************************************************
*
* sysRtcAlarmConnect - connect a routine to the alarm interrupt
*
* This routine specifies the user's interrupt service routine (isr) 
* to be called 
* at each alarm interrupt.  It does not enable alarm interrupts.  The 
* caller's isr will be called by the BSP's alarm isr, therefore the
* caller's isr should not attempt to acknowledge the interrupt.  It
* should only perform application-specific processing.  The caller's
* isr runs within the context of the interrupt, therefore common 
* precautions (such as avoiding printf) must be taken.
*
* RETURNS: OK, or ERROR if the passed in function ptr is NULL.
*
* SEE ALSO: sysRtcAlarmSet()
*/

STATUS sysRtcAlarmConnect
    (
    FUNCPTR routine,    /* routine called at each aux clock interrupt */
    int arg             /* argument to auxiliary clock interrupt routine */
    )
    {
    if (routine == NULL)
        return(ERROR);
    sysRtcAlarmRoutine = routine;
    sysRtcAlarmArg = arg;
    sysRtcAlarmConnected = TRUE;

    return (OK);
    }
