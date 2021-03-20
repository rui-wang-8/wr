/* lm78.c - LM78 system monitor support */

/* Copyright 1999-2000 Wind River Systems, Inc. */
/* Copyright 1999-2000 Motorola, Inc., All Rights Reserved */

/*
modification history
--------------------
01c,20mar00,djs  Initial code review changes. Brought over cpv5000 version
01b,14feb00,scb  Incorporate WRS code review suggestions.
01a,29jul99,scb  Initial Development.
*/

/* 
DESCRIPTION:

This file contains support routines which allow for reading and
manipulation of the Lm78 system monitor chip.  This chip allows for
monitoring of five different positive input voltages (IN0, IN1, IN2,
IN3, and IN4), two negative input voltages (IN5 and IN6), one
temperature (degrees C), three fan RPMs (FAN1, FAN2, and FAN3), and
several bi-state single-pin inputs.  The Lm78 is capable of automatic
out-of-bounds checking for voltages, temperature and fan RPM.

The Lm78 functions in this file can be used as components of a system
monitor task. One possible outline for a system monitor task would be:

Read the raw status using lm78RawStatusGet(), and obtain the current
settings in a LM78_RAW_STAT structure.

Using lm78PlusVoltToRaw(), lm78MinusVoltToRaw(), lm78TemperatureToRaw
compute "raw value RAM" numbers which represent the desired high and low
limits for voltages, and the high limit for temperature.

Insert the computed values into the LM78 by altering the LM78_RAW_STAT
structure.  The "lm78.h" file contains #defines which fix the locations
within the value RAM for these computed limits.

Use lm78RawStatusSet() to write back the raw status.  This will set
the desired limit values into the LM78 chip itself.

Turn on monitoring using lm78MonitorOn().

In a loop:

.IP
Set up a taskDelay() of about 2 seconds (raw status cannot be read
from the lm78 more often then once per 1.5 seconds).

.IP
Read the raw status using lm78RawStatusGet() again obtaining the
LM78_RAW_STAT structure.  Check the interrupt status registers
in this structure for out of bounds conditions.  The "lm78.h" file has
#defines for each of the bits in these registers.  Note that if some
functions are not wired up to the LM78 then out of bounds conditions for
these functions are to be expected.

.IP
If out of bounds conditions are found to exist, the exact value of the
out of bounds condition (as well as the limit values which triggered
the out of bounds condition) can be determined by calling
lm78RawStatusFormat() with the LM78_RAW_STAT structure.  A formatted
structure will be returned.  The data in the formatted structure is
suitable for display with printf(), using float formats for voltages,
integer format for temperature.  An operator could be alerted with
a message specifying the parameter that was out of bounds.
.LP

USAGE RESTRICTIONS

These lm78 functions do not directly support interrupt handling.
There is nothing in these routines, however, to prevent the user
from enabling interrupts associated with the Lm78.  If this is
done, it is up to user software to handle the related interrupts.

Because of hardware restrictions, it is not possible to read the
raw status within 1.5 seconds of turning on monitoring, or if
monitoring is already on, within 1.5 seconds of a previous read
of the raw status.  The functions in this file will enforce these
restrictions.

These support routines do not handle reentrancy contention.

The National Semiconductor LM78 manual contains detailed information
about the LM78 part.

INCLUDE FILES: lm78.h

*/

/* includes */

#include "vxWorks.h"
#include "lm78.h"

/* locals */

LOCAL LM78_RAW_STAT localRawStat;  /* Raw status for internal use */

LOCAL struct _lm78Settle
    {
    int       wdTickCount;      /* Tick count for Lm78 1.5 second delay */
    BOOL      OKtoFetch;	/* Flag for "Is it OK to fetch status" */
    WDOG_ID   wdId; 		/* Watchdog timer ID */
    } lm78Settle = { 0, TRUE, NULL };

/* forward declarations */

LOCAL UINT8  lm78Get (UINT8 reg);
LOCAL void   lm78Set (UINT8 reg, UINT8 value);
LOCAL void   lm78FetchOK (void);
LOCAL STATUS lm78FetchNotOK (void);
LOCAL BOOL   lm78EnoughTime (void);

/*******************************************************************************
*
* lm78Init - Initialize Lm78 to quiescent state
*
* This function initializes the Lm78 to a quiescent state with monitoring
* turned off, interrupt outputs disabled, fan divisors set to 1, and status 
* bits enabled.  The limit values are untouched and thus in an indeterminate 
* state.
*
* RETURNS: NA
*/

void lm78Init(void)

    {
    UINT8 temp;

    /* Turn off monitoring, disable SMI#, NMI interrupts */

    lm78RegSet (LM78_CONFIG, 0);

    /* Read interrupt status registers in order to clear them */

    (void) lm78RegGet (LM78_INTSTAT1, &temp);
    (void) lm78RegGet (LM78_INTSTAT2, &temp);

    /* Clear mask registers */

    lm78RegSet (LM78_SMI_MASK1, 0);
    lm78RegSet (LM78_SMI_MASK2, 0);
    lm78RegSet (LM78_NMI_MASK1, 0);
    lm78RegSet (LM78_NMI_MASK2, 0);

    /* Set fan divisor register to indicate a divisor of 1 */

    lm78RegSet (LM78_VID_FAN_DIV, 0);
    }

/*******************************************************************************
*
* lm78RawToPlusVolt - Convert raw value to positive voltage
*
* This function converts a positive "raw value RAM" voltage indicator to 
* an actual floating point voltage value.  The voltage returned is V(s) 
* (source voltage) and is computed accounting for attenuation provided 
* by the two associated resistors (R1 and R2).
*
* RETURNS: Converted voltage value in float format.
*
* SEE ALSO: lm78PlusVoltToRaw()
*/

float lm78RawToPlusVolt
    (
    UINT8 rawVal,	/* Raw value to convert to voltage */
    float r1Val,	/* Resistor #1 value in ohms */
    float r2Val		/* Resistor #2 value in ohms */
    )

    {
    float vIn;
    float vS;

    if (r2Val == 0.0)
	return (0.0);

    vIn = rawVal * LM78_VOLTS_PER_COUNT;
    vS = (vIn * ( r1Val + r2Val)) / r2Val;

    return (vS);
    }

/*******************************************************************************
*
* lm78PlusVoltToRaw - Convert positive voltage to raw value
*
* This function converts a positive floating point voltage to a "raw value RAM"
* voltage indicator.  The input voltage is V(s) (source voltage) and is 
* attenuated by the two associated resistors (R1 and R2).  A typical use of
* this function would be to convert voltage values to raw values to place
* into the LM78_RAW_STAT structure as limit values for the Lm78 voltage
* comparitor.  The modified raw status could then be written back to the
* Lm78 using lm78RawStatusSet().  Note that setting the high limit voltage 
* raw value to 0xff means that an out-of-spec voltage conditon will only be 
* signaled if the voltage falls below the low limit value.
*
* RETURNS: Converted "raw value RAM" indicator.
*
* SEE ALSO: lm78RawToPlusVolt()
*/

UINT8 lm78PlusVoltToRaw
    (
    float vS,		/* Positive signed input voltage value */
    float r1Val,	/* Resistor #1 value in ohms */
    float r2Val		/* Resistor #2 value in ohms */
    )

    {
    float raw;
    UINT8 rawVal;
    float divisor = LM78_VOLTS_PER_COUNT * (r1Val + r2Val);
    
    if (divisor == 0.0)
	return (0);
    
    raw = ((vS * r2Val) + (divisor / 2)) / divisor;

    if (raw > 255.0)
       rawVal = 0xff;
    else
       rawVal = raw;

    return (rawVal);
    }

/*******************************************************************************
*
* lm78RawToMinusVolt - Convert raw value to negative voltage
*
* This function converts a negative "raw value RAM" voltage indicator to an 
* actual  floating point voltage value.  The voltage returned is V(s) 
* (source voltage) and is computed using the input and feedback resistances 
* (R(in) and R(f)).
*
* RETURNS: Converted voltage value in float format.
*
* SEE ALSO: lm78MinusVoltToRaw()
*/

float lm78RawToMinusVolt
    (
    UINT8 rawVal,	/* Raw input value which requires conversion */
    float rInVal,	/* Input resistor value in ohms */	
    float rFVal		/* Feedback resistor value in ohms */
    )

    {
    float vF;
    float vS;

    if (rFVal == 0.0)
	return (0.0);

    vF = rawVal * LM78_VOLTS_PER_COUNT;
    vS = -( (vF * rInVal) / rFVal );
    return (vS);
    }

/*******************************************************************************
*
* lm78MinusVoltToRaw - Convert negative voltage to raw value
*
* This function converts a negative floating point source voltage to a "raw 
* value RAM" voltage indicator.  The raw value returned is computed using 
* the input and feedback resistances (R(in) and R(f)).  A typical use of
* this function would be to convert voltage values to raw values to place
* into the LM78_RAW_STAT structure as limit values for the Lm78 voltage
* comparitor.  The modified raw status could then be written back to the
* Lm78 using lm78RawStatusSet().  Note that setting the high limit voltage 
* raw value to 0xff means that an out-of-spec voltage conditon will only be 
* signaled if the voltage falls below (close to zero volts) than the low 
* limit value.
*
* RETURNS: Converted "raw value RAM" indicator.
*
* SEE ALSO: lm78RawToMinusVolt()
*/

UINT8 lm78MinusVoltToRaw
    (
    float vS,		/* Negative signed input voltage value */
    float rInVal,	/* Input voltage value in ohms */
    float rFVal		/* Feedback voltage value in ohms */
    )

    {
    float raw;
    UINT8 rawVal;
    float divisor = rInVal * LM78_VOLTS_PER_COUNT;

    if (divisor == 0.0)
	return (0);

    raw = -( ((vS * rFVal) - (divisor / 2)) / divisor );

    if (raw > 255.0)
       rawVal = 255;
    else
       rawVal = raw;

    return (rawVal);
    }

/*******************************************************************************
*
* lm78RawToTemperature - Convert raw value to temperature
*
* This function converts a "raw value RAM" temperature indicator into an 
* actual integer temperature (degrees C) value.
*
* RETURNS: Converted temperature value in integer (degrees C)  format.
*
* SEE ALSO: lm78TemperatureToRaw()
*/

int lm78RawToTemperature
    (
    UINT8 rawVal	/* Raw input value to convert to temperature */
    )

    {
    INT8 scratch;

    scratch = (INT8)rawVal;
    return ((int)scratch);
    }

/*******************************************************************************
*
* lm78TemperatureToRaw - Convert temperature to raw value
*
* This function converts temperature in degrees C into a "raw value RAM"
* indicator.  A typical use of this function would be to convert temperature 
* values to raw values to place into the LM78_RAW_STAT structure as limit 
* values for the Lm78 temperature comparitor.  The modified raw status could 
* then be written back to the Lm78 using lm78RawStatusSet().  Note that
* setting the high limit temperature raw value to 0x7f means that an 
* out-of-spec temperature conditon will only be signaled if the temperature
* falls below the low limit value.
*
* RETURNS: Converted "raw value RAM" indicator.
*
* SEE ALSO: lm78RawToTemperature()
*/

UINT8 lm78TemperatureToRaw
    (
    int temperature	/* Input temperature value in degrees C */
    )

    {
    UINT8 rawVal;

    INT8 scratch;

    /* Keep temperature within bounds of one-byte expression */

    if (temperature > 127)
	temperature = 127;
    else if (temperature < -128)
	temperature = -128;

    scratch = temperature;

    rawVal = (UINT8)scratch;

    return (rawVal);
    }

/*******************************************************************************
*
* lm78RawToRPM - Convert raw value to fan RPM
*
* This function converts a "raw value RAM" fan RPM indicator and divisor into 
* an integer RPM value.
*
* RETURNS: Converted RPM value.
*
* SEE ALSO: lm78RPMToRaw()
*/

UINT lm78RawToRPM
    (
    UINT8 rawVal,	/* Input raw value to convert to RPM */
    UINT  divisor	/* Divisor should be 1, 2, 4, or 8 */
    )

    {
    int rpm;

    /* Full-scale reading is VERY slow or "off" fan */

    if (rawVal == 0xff)
	return (0);

    /* Protect division operation */

    if ((rawVal == 0) || (divisor == 0))
	return (0x7ffffff);	/* Huge RPM for zero divisor */

    /* Round to nearest RPM */

    rpm = ( LM78_REV_COUNT_PER_MINUTE + ((rawVal * divisor) >> 1) ) /
            (rawVal * divisor);

    return (rpm);
    }

/*******************************************************************************
*
* lm78RPMToRaw - Convert fan RPM to raw value
*
* This function converts an integer fan RPM value and divisor into a "raw value 
* RAM" fan indicator.  A typical use of this function would be to convert 
* RPM into the LM78_RAW_STAT structure as a limit value for the Lm78 RPM
* comparitor.  The modified raw status could the be written back to the
* Lm78 using lm78RawStatusSet().
*
* RETURNS: Converted "value RAM" indicator.
*
* SEE ALSO: lm78RawToRPM()
*/

UINT8 lm78RPMToRaw
    (
    UINT  rpm,		/* Input RPM value to convert to raw value */
    UINT  divisor	/* Divisor should be 1, 2, 4, or 8 */
    )

    {
    UINT  raw;
    UINT8 rawVal;

    /* Protect division operation */

    if (rpm == 0)
	return (0xff);	/* Return full-scale reading for slow of "off" fan */

    /* Round to nearest raw value  */

    /*
     * Protect the division operation - if divisor is zero, which
     * it should never be, then we will return an erroneous RPM
     * value rather than generate a fault.
     */

    if (divisor == 0)
	divisor = 1;

    raw = ( LM78_REV_COUNT_PER_MINUTE + ((rpm * divisor) >> 1) ) /
           (rpm * divisor);

    if (raw > 0xff)
	rawVal = 0xff;
    else
	rawVal = raw;

    return (rawVal);
    }

/*******************************************************************************
*
* lm78MonitorOn - Turn on LM78 monitoring
*
* This function will turn on the Lm78 monitoring.  If monitoring is already
* on, the function will do nothing.  The function also starts a watchdog
* timer to allow verification of 1.5 seconds between raw status reads or
* between turning monitoring on and a raw status read.
*
* RETURNS:  OK if monitoring started and watchdog timer activated, ERROR
* if watchdog timer cannot be activated.
*
* SEE ALSO: lm78MonitorOff()
*/

STATUS lm78MonitorOn(void)

    {
    UINT8      	    configReg;	/* Scratch loc to hold config Register value */
    LM78_RAW_STAT   rawStat; 	/* Holding place for raw status */

    configReg = lm78Get (LM78_CONFIG);
    if ((configReg & LM78_CONFIG_BITS_START) == 0) /* If monitoring NOT on */
	{

	/* Force a raw status to be cached before turning on monitoring */

        (void) lm78RawStatusGet (&rawStat);

	configReg |= LM78_CONFIG_BITS_START;
        lm78Set (LM78_CONFIG, configReg);

        /* Set up watchdog timer */

	if (lm78FetchNotOK () == ERROR)
	    return (ERROR);
 
	}
    return (OK);
    }

/*******************************************************************************
*
* lm78MonitorOff - Turn off LM78 monitoring
*
* This function will turn on the Lm78 monitoring.  If monitoring is already
* off, the function will do nothing.  The function also stops the watchdog
* timer which was started when monitoring was turned on.
*
* RETURNS:  OK if monitoring stopped and watchdog timer canceled, ERROR
* if watchdog timer cannot be canceled.
*
* SEE ALSO: lm78MonitorOn()
*/

STATUS lm78MonitorOff(void)

    {
    UINT8 configReg;

    configReg = lm78Get (LM78_CONFIG);
    if (configReg & LM78_CONFIG_BITS_START)	/* If monitoring on ... */
	{

	/* Turn off the monitoring */

	configReg &= ~LM78_CONFIG_BITS_START;
        lm78Set (LM78_CONFIG, configReg);

        /* Cancel the watchdog timer */

	if (lm78Settle.wdId != NULL)
            if (wdCancel(lm78Settle.wdId) == ERROR)
	        return (ERROR);

	/* Indicate it's OK to read the raw status again */

        lm78Settle.OKtoFetch = TRUE;

	}
    return (OK);
    }

/*******************************************************************************
*
* lm78RegGet - Get a single Lm78 register
*
* This function gets contents of a single Lm78 register.  The only registers 
* supported by this routine are: configuration, SMI# mask 1, SMI# mask2,
* NMI mask 1, NMI mask 2, VID/fan divisor, serial bus address, and
* chip reset/ID.
*  
* RETURNS: OK if register successfully fetched, or ERROR if input register
* parameter representing register ID is out of range, or if return parameter
* is NULL.
*
* SEE ALSO: lm78RegSet()
*/

STATUS lm78RegGet
    (
    UINT  regId,	/* Identifier for register to fetch */
    UINT8 * regVal	/* Pointer to return value */
    )

    {

    /* Check for null return */

    if (regVal == NULL)
	return (ERROR);

    switch (regId)
	{
	case LM78_CONFIG:
	case LM78_SMI_MASK1:
	case LM78_SMI_MASK2:
	case LM78_NMI_MASK1:
	case LM78_NMI_MASK2:
	case LM78_VID_FAN_DIV:
	case LM78_SER_BUS_ADDR:
	case LM78_CHIP_RESET_ID:
            *regVal = lm78Get (regId);
            break;
        default:
            return (ERROR);
	}

    return (OK);
    }

/*******************************************************************************
*
* lm78RegSet - Set a single Lm78 register
*
* This function sets contents of a single Lm78 register.  The only registers 
* supported by this routine are: configuration, SMI# mask 1, SMI# mask2,
* NMI mask 1, NMI mask 2, VID/fan divisor, serial bus address, and
* chip reset/ID.
*  
* RETURNS: OK if register successfully set, or ERROR if input register
* parameter representing register ID is out of range.
*
* SEE ALSO: lm78RegGet()
*/

STATUS lm78RegSet
    (
    UINT  regId,	/* Identifier for register to set */
    UINT8 regVal	/* Value to place into register */
    )

    {
    switch (regId)
	{
	case LM78_CONFIG:
	case LM78_SMI_MASK1:
	case LM78_SMI_MASK2:
	case LM78_NMI_MASK1:
	case LM78_NMI_MASK2:
	case LM78_VID_FAN_DIV:
	case LM78_SER_BUS_ADDR:
	case LM78_CHIP_RESET_ID:
            lm78Set (regId, regVal);
            break;
        default:
            return (ERROR);
	}

    return (OK);
    }

/*******************************************************************************
*
* lm78RawStatusGet - Get raw status from the Lm78
*
* This function will return the contents of the Lm78 "raw status" which 
* consists of the contents of interrupt status register 1, interrupt status
* register 2, the POST (Power On Self-Test) RAM, and the Value RAM (consisting
* of monitor results and limit values).  An Lm78 hardware constraint dictates 
* that the raw status can only be read once every 1.5 seconds when monitoring
* is turned on (bit 0 of the configuration register is set).  When monitoring
* is turned off, the raw status can be read as often as desired.  If an 
* attempt to read the raw status is done within 1.5 seconds of the last
* successful read or within 1.5 seconds of turning on monitoring, an error
* status will be returned and the last raw monitor status which was  
* successfully read will be returned.
*  
* RETURNS: OK if current raw status was returned, ERROR if return pointer is
* NULL or if not enough time has elapsed to read the status, in which case 
* the previous correctly obtained status will be returned.
*
* SEE ALSO: lm78RawStatusSet()
*/

STATUS lm78RawStatusGet
    (
    LM78_RAW_STAT * rawStat /* Pointer to struct in which status is returned */
    )

    {
    STATUS stat = OK;
    UINT8  regVal;
    BOOL   monitorOn;
    int    i;
    int    j;

    /* Check for null return */

    if (rawStat == NULL)
	return (ERROR);

    /* Determine if monitoring is turned on or not */

    regVal = lm78Get (LM78_CONFIG);

    if (regVal & LM78_CONFIG_BITS_START)
	monitorOn = TRUE;
    else
	monitorOn = FALSE;

    if ( ((monitorOn == TRUE) && lm78EnoughTime()) || (monitorOn == FALSE) )
        {

        /* Fetch the status from the LM78 into a local copy */
    
	localRawStat.intStat1 = lm78Get ( LM78_INTSTAT1 );
	localRawStat.intStat2 = lm78Get ( LM78_INTSTAT2 );
        
        for (i = LM78_POST_RAM_LOW, j = 0; i <= LM78_POST_RAM_HIGH; i++, j++)
	    localRawStat.postRam[j] = lm78Get ( i );

        for (i = LM78_VALUE_RAM_LOW, j = 0; i <= LM78_VALUE_RAM_HIGH; i++, j++)
	    localRawStat.valueRam[j] = lm78Get ( i );

	if (monitorOn)
	    lm78FetchNotOK ();

        }
    else 
        stat = ERROR; 

    /* Local copy has been fetched, copy it to the user's structure */

    bcopy ((char *)&localRawStat, (char *)rawStat, sizeof(LM78_RAW_STAT));

    return (stat);
    }

/*******************************************************************************
*
* lm78RawStatusSet - Set raw status into LM78
*
* This function sets the contents of the LM78 "raw status" which consists of 
* the POST (Power On Self-Test) RAM, and the Value RAM (consisting of monitor 
* results and limit values).  Monitoring must be turned off to set the raw 
* status, else an error is returned.
*  
* RETURNS: OK if current raw status was set, ERROR if monitoring was not
* turned off before attempting this call.
*
* SEE ALSO: lm78RawStatusGet()
*/

STATUS lm78RawStatusSet
    (
    LM78_RAW_STAT * rawStat 	/* Input raw status to set into LM78 */
    )

    {
    UINT8  regVal;
    int    i;
    int    j;

    /* Check for null return */

    if (rawStat == NULL)
	return (ERROR);

    /* If monitoring on, return error */

    regVal = lm78Get (LM78_CONFIG);

    if (regVal & LM78_CONFIG_BITS_START)
	return (ERROR);
        
    for (i = LM78_POST_RAM_LOW, j = 0; i <= LM78_POST_RAM_HIGH; i++, j++)
        lm78Set (i, rawStat->postRam[j]);

    for (i = LM78_VALUE_RAM_LOW, j = 0; i <= LM78_VALUE_RAM_HIGH; i++, j++)
	lm78Set (i, rawStat->valueRam[j]);

    return (OK);
    }

/*******************************************************************************
*
* lm78RawStatusFormat - Convert raw Lm78 status to formatted form
*
* This function will format the Lm78 "raw status" into values suitable for
* printf().  Voltages will be returned as floats and fan values will be 
* returned as integer RPM.  This function does not read the status from the 
* part but instead depends upon a an input parameter which contains the raw 
* status previously fetched via call to lm78RawStatusGet().  
*  
* RETURNS: NA
*/

void lm78RawStatusFormat
    (
    LM78_RAW_STAT * rawStat,		/* Input raw status to set into LM78 */
    LM78_FORMAT_STAT * formatStat	/* Formatted status values */
    )

    {

    formatStat->exceed.in0 = ( (rawStat->intStat1 & 
				LM78_INTSTAT1_BITS_IN0) != 0);

    formatStat->exceed.in1 = ( (rawStat->intStat1 & 
				LM78_INTSTAT1_BITS_IN1) != 0);

    formatStat->exceed.in2 = ( (rawStat->intStat1 & 
				LM78_INTSTAT1_BITS_IN2) != 0);

    formatStat->exceed.in3 = ( (rawStat->intStat1 & 
				LM78_INTSTAT1_BITS_IN3) != 0);

    formatStat->exceed.in4 = ( (rawStat->intStat2 & 
				LM78_INTSTAT2_BITS_IN4) != 0);

    formatStat->exceed.in5 = ( (rawStat->intStat2 & 
				LM78_INTSTAT2_BITS_IN5) != 0);

    formatStat->exceed.in6 = ( (rawStat->intStat2 & 
				LM78_INTSTAT2_BITS_IN6) != 0);

    formatStat->exceed.temperature = ( (rawStat->intStat1 & 
					LM78_INTSTAT1_BITS_TEMPERATURE) != 0);

    formatStat->exceed.bti = ( (rawStat->intStat1 & 
				LM78_INTSTAT1_BITS_BTI) != 0);

    formatStat->exceed.fan1 = ( (rawStat->intStat1 & 
				 LM78_INTSTAT1_BITS_FAN1) != 0);

    formatStat->exceed.fan2 = ( (rawStat->intStat1 & 
				 LM78_INTSTAT1_BITS_FAN2) != 0);

    formatStat->exceed.fan3 = ( (rawStat->intStat2 & 
				 LM78_INTSTAT2_BITS_FAN3) != 0);

    formatStat->exceed.chassisIntrusion = ( (rawStat->intStat2 & 
				    LM78_INTSTAT2_BITS_CHASSIS_INTRUSION) != 0);

    formatStat->exceed.fifoOverflow = ( (rawStat->intStat2 & 
				        LM78_INTSTAT2_BITS_FIFO_OVERFLOW) != 0);

    formatStat->exceed.smiIn = ( (rawStat->intStat2 & 
				  LM78_INTSTAT2_BITS_SMI_IN) != 0);

    formatStat->values.in0 = lm78RawToPlusVolt 
			     (rawStat->valueRam [LM78_VALUE_IN0], LM78_IN0_R1, 
			      LM78_IN0_R2);

    formatStat->values.in1 = lm78RawToPlusVolt 
			     (rawStat->valueRam [LM78_VALUE_IN1], LM78_IN1_R1, 
			      LM78_IN1_R2);

    formatStat->values.in2 = lm78RawToPlusVolt 
			     (rawStat->valueRam [LM78_VALUE_IN2], LM78_IN2_R1, 
			      LM78_IN2_R2);

    formatStat->values.in3 = lm78RawToPlusVolt 
			       (rawStat->valueRam [LM78_VALUE_IN3], LM78_IN3_R1,
			        LM78_IN3_R2);

    formatStat->values.in4 = lm78RawToPlusVolt 
			       (rawStat->valueRam [LM78_VALUE_IN4], LM78_IN4_R1,
			        LM78_IN4_R2);

    formatStat->values.in5 = lm78RawToMinusVolt 
					    (rawStat->valueRam [LM78_VALUE_IN5],
			                     LM78_IN5_RIN, LM78_IN5_RF);

    formatStat->values.in6 = lm78RawToMinusVolt 
			                    (rawStat->valueRam [LM78_VALUE_IN6],
			                     LM78_IN6_RIN, LM78_IN6_RF);

    formatStat->values.temperature = lm78RawToTemperature 
			     ((INT8)rawStat->valueRam [LM78_VALUE_TEMPERATURE]);

    formatStat->values.fan1 = lm78RawToRPM (rawStat->valueRam [LM78_VALUE_FAN1],
                                            LM78_FAN1_DIVISOR);

    formatStat->values.fan2 = lm78RawToRPM (rawStat->valueRam [LM78_VALUE_FAN2],
                                            LM78_FAN2_DIVISOR);

    formatStat->values.fan3 = lm78RawToRPM (rawStat->valueRam [LM78_VALUE_FAN3],
                                            LM78_FAN3_DIVISOR);

    formatStat->limits.in0High = lm78RawToPlusVolt 
				       (rawStat->valueRam [LM78_VALUE_IN0_HIGH],
    			                LM78_IN0_R1, LM78_IN0_R2);

    formatStat->limits.in0Low = lm78RawToPlusVolt 
				        (rawStat->valueRam [LM78_VALUE_IN0_LOW],
    			                 LM78_IN0_R1, LM78_IN0_R2);

    formatStat->limits.in1High = lm78RawToPlusVolt 
				       (rawStat->valueRam [LM78_VALUE_IN1_HIGH],
    			                LM78_IN1_R1, LM78_IN1_R2);

    formatStat->limits.in1Low = lm78RawToPlusVolt 
					(rawStat->valueRam [LM78_VALUE_IN1_LOW],
    			                 LM78_IN1_R1, LM78_IN1_R2);

    formatStat->limits.in2High = lm78RawToPlusVolt 
				       (rawStat->valueRam [LM78_VALUE_IN2_HIGH],
    			                LM78_IN2_R1, LM78_IN2_R2);

    formatStat->limits.in2Low = lm78RawToPlusVolt 
				        (rawStat->valueRam [LM78_VALUE_IN2_LOW],
    			                 LM78_IN2_R1, LM78_IN2_R2);

    formatStat->limits.in3High = lm78RawToPlusVolt 
				       (rawStat->valueRam [LM78_VALUE_IN3_HIGH],
    			                LM78_IN3_R1, LM78_IN3_R2);

    formatStat->limits.in3Low = lm78RawToPlusVolt 
				        (rawStat->valueRam [LM78_VALUE_IN3_LOW],
    			                 LM78_IN3_R1, LM78_IN3_R2);

    formatStat->limits.in4High = lm78RawToPlusVolt 
				       (rawStat->valueRam [LM78_VALUE_IN4_HIGH],
    			                LM78_IN4_R1, LM78_IN4_R2);

    formatStat->limits.in4Low = lm78RawToPlusVolt 
					(rawStat->valueRam [LM78_VALUE_IN4_LOW],
    			                 LM78_IN4_R1, LM78_IN4_R2);

    formatStat->limits.in5High = lm78RawToMinusVolt 
				       (rawStat->valueRam [LM78_VALUE_IN5_HIGH],
                                        LM78_IN5_RIN, LM78_IN5_RF);

    formatStat->limits.in5Low = lm78RawToMinusVolt 
					(rawStat->valueRam [LM78_VALUE_IN5_LOW],
    			                 LM78_IN5_RIN, LM78_IN5_RF);

    formatStat->limits.in6High = lm78RawToMinusVolt 
				       (rawStat->valueRam [LM78_VALUE_IN6_HIGH],
                                        LM78_IN6_RIN, LM78_IN6_RF);

    formatStat->limits.in6Low = lm78RawToMinusVolt 
				       (rawStat->valueRam [LM78_VALUE_IN6_LOW],
    			                LM78_IN6_RIN, LM78_IN6_RF);

    formatStat->limits.temperatureHigh = lm78RawToTemperature 
			 ((INT8)rawStat->valueRam[LM78_VALUE_TEMPERATURE_HIGH]);

    formatStat->limits.temperatureLow = lm78RawToTemperature 
			 ((INT8)rawStat->valueRam [LM78_VALUE_TEMPERATURE_LOW]);

    formatStat->limits.fan1Low = lm78RawToRPM 
		   (rawStat->valueRam [LM78_VALUE_FAN1_LOW], LM78_FAN1_DIVISOR);

    formatStat->limits.fan2Low = lm78RawToRPM 
		   (rawStat->valueRam [LM78_VALUE_FAN2_LOW], LM78_FAN2_DIVISOR);

    formatStat->limits.fan3Low = lm78RawToRPM 
		   (rawStat->valueRam [LM78_VALUE_FAN3_LOW], LM78_FAN3_DIVISOR);

    }

/*******************************************************************************
*
* lm78Get - Get a byte value from the Lm78 register or RAM
*
* This function uses the address and data ports of the Lm78 to actually
* get the value of one byte of the Lm78 memory (register or RAM).
* There is no error checking for an out-of-bounds request.
*
* RETURNS: Byte value read from Lm78 register.
*
* SEE ALSO: lm78Set()
*/

LOCAL UINT8 lm78Get
    (
    UINT8 reg		/* Identifier for register to fetch */
    )

    {
    sysOutByte (LM78_INDEX, reg);
    return (sysInByte (LM78_DATA));
    }

/*******************************************************************************
*
* lm78Set - Get a byte value into an Lm78 register or RAM.
*
* This function uses the address and data ports of the Lm78 to actually
* set the value of one byte of the Lm78 memory (register or RAM).
* There is no error checking for an out-of-bounds request.
*
* RETURNS: NA
*
* SEE ALSO: lm78Get()
*/

LOCAL void lm78Set
    (
    UINT8 reg,		/* Identifier for register or memory to set */
    UINT8 value		/* Value to place into register */
    )

    {
    sysOutByte (LM78_INDEX, reg);
    sysOutByte (LM78_DATA, value);
    }

/*******************************************************************************
*
* lm78FetchOK - Indicate that it's OK to fetch Lm78 raw status.
*
* This function works in conjunction with the watchdog timer.  It sets a
* flag on each watchdog timer pop, indicating to other functions that
* enough time has elapsed to allow another raw status reading from the Lm78.
*
* RETURNS: NA
*
* SEE ALSO: lm78FetchNotOK(), lm78EnoughTime()
*/

LOCAL void lm78FetchOK(void)

    {
    lm78Settle.OKtoFetch = TRUE; /* TRUE says it's OK to fetch raw status */
    }

/*******************************************************************************
*
* lm78FetchNotOK - Indicate that it's NOT OK to fetch the Lm78 raw status.
*
* The function creates the watchdog timer if necessary and resets it so that
* it will pop again in 1.5 seconds.  It sets a flag indicating that it is
* too soon to fetch the Lm78 raw status.  This aids in controlling 
* access to the raw status of the Lm78, allowing a fresh read of the 
* status only once per 1.5 seconds.  The time delay is required to allow
* the Lm78 to "settle" after having previously had monitoring turned on or 
* having had the raw status read.
*
* RETURNS: ERROR if watchdog timer cannot be
*
* SEE ALSO: lm78FetchOK(), lm78EnoughTime()
*/

LOCAL STATUS lm78FetchNotOK(void)

    {

    /* Create watch dog timer it it is not already created */

    if (lm78Settle.wdId == NULL)
        if ((lm78Settle.wdId = wdCreate()) == NULL)
	    {
            logMsg ("lm78: wdCreate() failed\n",1,2,3,4,5,6);
	    return (ERROR);
	    }

    /* Cancel any active watch dog timer */

    if (wdCancel(lm78Settle.wdId) == ERROR)
        return (ERROR);

    /* Calculate tick count for 1.5 seconds */

    lm78Settle.wdTickCount = sysClkRateGet ();
    lm78Settle.wdTickCount += ((lm78Settle.wdTickCount + 1) >> 1);

    /* Start the watchdog timer */

    if (wdStart (lm78Settle.wdId, lm78Settle.wdTickCount, 
		(FUNCPTR) lm78FetchOK, 0) == ERROR)
        {
        logMsg ("lm78MonitorOn(): wdStart() failed\n",1,2,3,4,5,6);
        return (ERROR);
	}

    /* Indicate it's too soon to read the raw status */

    lm78Settle.OKtoFetch = FALSE;

    return (OK);
    }

/*******************************************************************************
*
* lm78EnoughTime - Determine if enough has elapsed to allo Lm78 raw status read.
*
* This function returns the value of the watch dog flag.  This flag is TRUE
* only after 1.5 seconds or more has elapsed from either turning on monitoring 
* or from the last raw status read of the Lm78.  A return value of TRUE
* indicates that it's OK to read a new raw status from the Lm78.  A return
* value of FALSE indicates that it's too soon to read a new raw status because
* the Lm78 has not had enough time to settle from a previous turning on of
* monitoring or previous raw status read.
*
* RETURNS:  TRUE if 1.5 seconds has elapsed, FALSE otherwise
*
* SEE ALSO: lm78FetchOK(), lm78FetchNotOK()
*/

LOCAL BOOL lm78EnoughTime(void)

    {
    return (lm78Settle.OKtoFetch);
    }
