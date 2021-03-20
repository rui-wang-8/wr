/* timerDev.h - Generic Timer header */

/* Copyright 1984-1996 Wind River Systems, Inc. */

/*
modification history
--------------------
01d,06aug96,dat  merged in timestamp support
01c,22sep92,rrr  added support for c++
01b,07jul92,ccc  change name to timerDev.h
01a,22jun92,ccc  created.
*/

/*
DESCRIPTION
This file contains header information for generic timer routines.
*/

#ifndef __INCtimerDevh
#define __INCtimerDevh

#ifdef __cplusplus
extern "C" {
#endif

/* function declarations */

#ifndef	_ASMLANGUAGE
#if defined(__STDC__) || defined(__cplusplus)

IMPORT	STATUS	sysClkConnect (FUNCPTR routine, int arg);
IMPORT	void	sysClkDisable (void);
IMPORT	void	sysClkEnable (void);
IMPORT	int	sysClkRateGet (void);
IMPORT	STATUS	sysClkRateSet (int ticksPerSecond);
IMPORT	STATUS	sysAuxClkConnect (FUNCPTR routine, int arg);
IMPORT	void	sysAuxClkDisable (void);
IMPORT	void	sysAuxClkEnable (void);
IMPORT	int	sysAuxClkRateGet (void);
IMPORT	STATUS	sysAuxClkRateSet (int ticksPerSecond);
IMPORT	STATUS	sysTimestampConnect (FUNCPTR routine, int arg);
IMPORT	STATUS	sysTimestampEnable (void);
IMPORT	STATUS	sysTimestampDisable (void);
IMPORT	UINT32	sysTimestampFreq (void);
IMPORT	UINT32	sysTimestampPeriod (void);
IMPORT	UINT32	sysTimestamp (void);
IMPORT	UINT32	sysTimestampLock (void);


#else	/* __STDC__ */

IMPORT	STATUS	sysClkConnect ();
IMPORT	void	sysClkDisable ();
IMPORT	void	sysClkEnable ();
IMPORT	int	sysClkRateGet ();
IMPORT	STATUS	sysClkRateSet ();
IMPORT	STATUS	sysAuxClkConnect ();
IMPORT	void	sysAuxClkDisable ();
IMPORT	void	sysAuxClkEnable ();
IMPORT	int	sysAuxClkRateGet ();
IMPORT	STATUS	sysAuxClkRateSet ();
IMPORT	STATUS	sysTimestampConnect ();
IMPORT	STATUS	sysTimestampEnable ();
IMPORT	STATUS	sysTimestampDisable ();
IMPORT	UINT32	sysTimestampFreq ();
IMPORT	UINT32	sysTimestampPeriod ();
IMPORT	UINT32	sysTimestamp ();
IMPORT	UINT32	sysTimestampLock ();


#endif	/* __STDC__ */

#endif	/* _ASMLANGUAGE */

#ifdef __cplusplus
}
#endif

#endif /* __INCtimerDevh */
