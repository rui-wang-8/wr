/* failsafe.h - failsafe watchdog timer definitions for the PLFPGA */

/* Copyright 1999-2002 Wind River Systems, Inc. All Rights Reserved */
/* Copyright 1999-2000 Motorola, Inc.  All Rights Reserved */

/*
modification history
--------------------
01c,20mar00,djs  Initial code review changes. Brought over cpv5000 version
01b,11Feb00,sjb  Incorporate WRS Code Review Comments
01a,18Aug99,sjb  Initial Development 
*/

#ifndef INCfailsafeh
#define INCfailsafeh

#ifdef __cplusplus
extern "C" {
#endif

/* 
 * There are 8 possible setting of the WDCFG register's SEL bits 
 * Each corresponds to a different timeout length.  These are the
 * values which should be passed to failsafeStart.
 *
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
 *
 */

#define FAILSAFE_SELECT_PAT0 0   
#define FAILSAFE_SELECT_PAT1 1  
#define FAILSAFE_SELECT_PAT2 2 
#define FAILSAFE_SELECT_PAT3 3
#define FAILSAFE_SELECT_PAT4 4 
#define FAILSAFE_SELECT_PAT5 5 
#define FAILSAFE_SELECT_PAT6 6 
#define FAILSAFE_SELECT_PAT7 7
#define FAILSAFE_SELECT_NUMBER 8

/* these should be used for all chip accesses */

#define FAILSAFE_WDSTB (FAILSAFE_ISA_BASE_ADRS + 0x0b)   /* strobe reg */
#define FAILSAFE_WDINDEX (FAILSAFE_ISA_BASE_ADRS + 0x0d) /* wdt index reg */
#define FAILSAFE_WDDATA (FAILSAFE_ISA_BASE_ADRS + 0x0f)  /* wdt data reg */
#define FAILSAFE_WDCFG_OFFSET (0x03)      /* reg # of the wdt config reg */
#define FAILSAFE_DEVNUM_OFFSET (0x0f)     /* reg # of the DEVNUM reg */
#define FAILSAFE_LEGACY_DEVNUM (0x0)      /* DEVNUM of the legacy features */

/* bit fields and masks for the watchdog timer config register (WDCFG) */

#define FAILSAFE_WDCFG_CLR_STATUS (0x80)  /* 1: stat latched in reset state */ 
#define FAILSAFE_WDCFG_ALARM_EN   (0x40)  /* 1: alarm signal active */
#define FAILSAFE_WDCFG_WDX_BITS   (0x18)  /* WD1 WD0, define failsafe event */
#define FAILSAFE_WDCFG_WDX_MASK   (0xe7)  /* mask off WD1 WD0 */
#define FAILSAFE_WDCFG_SELX_BITS  (0x07)  /* These bits select failsafe len */
#define FAILSAFE_WDCFG_SELX_MASK  (0xf8)  /* mask off failsafe select bits */

/* the possible values for the failsafe's WD bits */

#define FAILSAFE_WDX_DISABLED     (0x00)  /* no failsafe event, wdt disabled */
#define FAILSAFE_WDX_IRQX         (0x08)  /* failsafe event is an irq */
#define FAILSAFE_WDX_NMI          (0x10)  /* failsafe event is an NMI */
#define FAILSAFE_WDX_NMI_RESET    (0x18)  /* failsafe event NMI then sysreset */

/* Strobe register bit definition(s) */

#define FAILSAFE_WDSTB_HISTORY    (0x04)  /* 1: failsafe caused last reset */

#ifdef __cplusplus
}
#endif

#endif /* INCfailsafeh */
