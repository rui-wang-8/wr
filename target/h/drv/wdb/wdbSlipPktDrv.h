/* wdbSlipPktLib.h - header file for WDB agents SLIP packet driver */

/* Copyright 1984-2002 Wind River Systems, Inc. */

/*
modification history
--------------------
01c,26apr02,dat  Adding cplusplus protection, SPR 75017
01b,15jun95,ms	changed SIO_DEV to SIO_CHAN
01a,20dec94,ms  written.
*/

#ifndef __INCwdbSlipPktLibh
#define __INCwdbSlipPktLibh

/* includes */

#include "sioLib.h"
#include "wdb/wdb.h"
#include "wdb/wdbCommIfLib.h"
#include "wdb/wdbMbufLib.h"

#ifdef __cplusplus
extern "C" {
#endif

/* defines */

#define SLMTU           1006    /* be compatible with BSD 4.3 SLIP */

/* data types */

typedef struct		/* hidden */
    {
    WDB_DRV_IF	wdbDrvIf;		/* all packet device have this */
    SIO_CHAN *	pSioChan;		/* underlying serial channel */

    u_int	mode;

    /* only one input buffer at a time for RPC's */

    u_int	inputState;
    caddr_t	inBufPtr;
    char	inBufBase [SLMTU];
    caddr_t	inBufEnd;

    /* output is a chain of mbufs */

    struct mbuf * pMbuf;
    u_int	outputState;
    int		outputIx;
    } WDB_SLIP_PKT_DEV;

/* function prototypes */

#if defined(__STDC__)

extern void    wdbSlipPktDevInit (WDB_SLIP_PKT_DEV *pPktDev,
				  SIO_CHAN *pSioChan,
				  void (*inputRtn)());

#else   /* __STDC__ */

extern void    wdbSlipPktDevInit ();

#endif  /* __STDC__ */

#ifdef __cplusplus
}
#endif

#endif  /* __INCwdbSlipPktLibh */

