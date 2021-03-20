/* sysRtc.h - Real Time Clock Support header */
/* Copyright 1999-2002 Wind River Systems, Inc. All Rights Reserved */
/* Copyright 1999 Motorola, Inc. All Rights Reserved */

/*
modification history
--------------------
01c,14jun02,rhe  Add C++ Protection
01b,01feb00,djs  incorporate WRS review comments.
01a,23jun99,sjb  written for CPV5000.
*/

/* Real Time Clock Date and Time Structure */

#ifndef INCsysRtch
#define INCsysRtch

#ifdef __cplusplus
extern "C" {
#endif

extern FUNCPTR sysAuxClkRoutine;
extern int sysAuxClkArg;
extern int sysAuxClkRunning;


/* The RTC code expects binary values in this structure, not BCD. */

typedef struct rtcdt         /* RTC_DATE_TIME */
    {
	int         year;             /* year */
	int         month;            /* month */
	int         day;              /* day */
	int         hour;             /* hour */
	int         minute;           /* minute */
	int         second;           /* second */
    } RTC_DATE_TIME;


/* define this if running the MCG RTC test utility */

#define VERBOSE_TEST

/* CMOS RTC register A bit definitions */

#define CMOS_RTC_A_UIP 0x80     /* RO 1:indicates an update is in progress*/
#define CMOS_RTC_A_DV2 0x40     /* the DVn bits turn on/off osc. */
#define CMOS_RTC_A_DV1 0x20
#define CMOS_RTC_A_DV0 0x10
#define CMOS_RTC_A_RS3 0x08     /* rate selection bits for sq wave    */
#define CMOS_RTC_A_RS2 0x04     /*    or for periodic interrupt       */
#define CMOS_RTC_A_RS1 0x02
#define CMOS_RTC_A_RS0 0x01

/* CMOS RTC register B bit definitions */

#define CMOS_RTC_B_SET 0x80     /* if set, time/date can be init'd */
#define CMOS_RTC_B_PIE 0x40     /* if set, enables periodic interrupts */
#define CMOS_RTC_B_AIE 0x20     /* if set, enables alarm interrupts */
#define CMOS_RTC_B_UIE 0x10     /* if set, enable update ended interrupt */
#define CMOS_RTC_B_SQW 0x08     /* if set, enable square wave generation */
#define CMOS_RTC_B_DM  0x04     /* 1:binary data mode, 0:BCD data mode */
#define CMOS_RTC_B_24_12 0x02   /* 1: 24-hour mode, 0: 12-hour mode */
#define CMOS_RTC_B_DSE 0x01     /* 1: enable daylight savings updates */


/* CMOS RTC register C bit definitions */
/* Reg C is read only */

#define CMOS_RTC_C_IRQ 0x80     /* if set, an intr has occurred. clear by read*/
#define CMOS_RTC_C_PF  0x40     /* Periodic Intr Flag (FLAG, not intr) */
#define CMOS_RTC_C_AF  0x20     /* Alarm Intr Flag (FLAG, not intr)    */
#define CMOS_RTC_C_UF  0x10     /* Update Ended Intr flag (not an intr) */

/* Current choices for the most subjective options shown above */

#define RTC_DATA_MODE 0				/* BCD mode */
#define RTC_CLOCK_MODE CMOS_RTC_B_24_12		/* 24 hr mode */
#define RTC_DSE_MODE 0				/* no daylight savings */

/* These define the offsets of these regs in the CMOS Data block */

#define SEC             0x00
#define SEC_ALARM       0x01
#define MIN             0x02
#define MIN_ALARM       0x03
#define HOUR            0x04
#define HOUR_ALARM      0x05
#define DOW             0x06
#define DOM             0x07
#define MONTH           0x08
#define YEAR            0x09
#define REGA            0x0a
#define REGB            0x0b
#define REGC            0x0c
#define REGD            0x0d

/* control values for the sysRtcAlarmSet routine */

#define ALARM_AT 1      /* Need one alarm intr at a specific T.O.D. */
#define ALARM_EVERY 2   /* Need an alarm to recur on sec,min,hr interval */

#define DONT_CARE_BITS 0xc0  /* when set indicate alarm is "don't care" */

/*
 * the following macros convert from BCD to binary and back.
 * Be careful that the arguments are chars, only char width returned.
 * This is required if we need to run the chip in BCD mode.  
 */

#define BCD_TO_BIN(bcd) (( ((((bcd)&0xf0)>>4)*10) + ((bcd)&0xf) ) & 0xff) 
#define BIN_TO_BCD(bin) (( (((bin)/10)<<4) + ((bin)%10) ) & 0xff) 

#ifdef __cplusplus
}
#endif
#endif  /* INCsysRtch */
