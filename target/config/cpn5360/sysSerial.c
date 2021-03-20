/* sysSerial.c - PC386/486 BSP serial device initialization */

/* Copyright 1984-1996 Wind River Systems, Inc. */
#include "copyright_wrs.h"

/*
modification history
--------------------
01g,21sep98,fle  added library description
01f,19jun96,wlf  doc: cleanup.
01e,23oct95,jdi  doc: cleaned up and removed all NOMANUALs.
01d,03aug95,myz  fixed the warning message
01c,20jun95,ms   fixed comments for mangen
01b,15jun95,ms	 updated for new serial driver
01a,15mar95,myz  written based on mv162 version.
*/

/*
DESCRIPTION

This library contains routines for PC386/486 BSP serial device initialization
*/

#include "vxWorks.h"
#include "iv.h"
#include "intLib.h"
#include "config.h"
#include "sysLib.h"
#include "drv/sio/i8250Sio.h"

/* device initialization structure */

typedef struct
    {
    USHORT vector;
    ULONG  baseAdrs;
    USHORT regSpace;
    USHORT intLevel;
    } I8250_CHAN_PARAS;

#ifdef INCLUDE_PC_CONSOLE       /* if key board and VGA console needed */

#include "serial/pcConsole.c"
#include "serial/m6845Vga.c"

#if (PC_KBD_TYPE == PC_PS2_101_KBD)     /* 101 KEY PS/2                 */
#include "serial/i8042Kbd.c"
#else
#include "serial/i8048Kbd.c"            /* 83 KEY PC/PCXT/PORTABLE      */
#endif /* (PC_KBD_TYPE == PC_XT_83_KBD) */

#endif /* INCLUDE_PC_CONSOLE */


/* Local data structures */

static I8250_CHAN  i8250Chan[N_UART_CHANNELS];

static I8250_CHAN_PARAS devParas[] = 
    {
      {COM1_INT_VEC,COM1_BASE_ADR,UART_REG_ADDR_INTERVAL,COM1_INT_LVL},
      {COM2_INT_VEC,COM2_BASE_ADR,UART_REG_ADDR_INTERVAL,COM2_INT_LVL}
    };

#define UART_REG(reg,chan) \
(devParas[chan].baseAdrs + reg*devParas[chan].regSpace)

/******************************************************************************
*
* sysSerialHwInit - initialize the BSP serial devices to a quiescent state
*
* This routine initializes the BSP serial device descriptors and puts the
* devices in a quiescent state.  It is called from sysHwInit() with
* interrupts locked.
*
* RETURNS: N/A
*
* SEE ALSO: sysHwInit()
*/


void sysSerialHwInit (void)
    {
    int i;

    for (i=0;i<N_UART_CHANNELS;i++)
        {
	i8250Chan[i].int_vec = devParas[i].vector;
	i8250Chan[i].channelMode = 0;
	i8250Chan[i].lcr =  UART_REG(UART_LCR,i);
	i8250Chan[i].data =  UART_REG(UART_RDR,i);
	i8250Chan[i].brdl = UART_REG(UART_BRDL,i);
	i8250Chan[i].brdh = UART_REG(UART_BRDH,i);
	i8250Chan[i].ier =  UART_REG(UART_IER,i);
	i8250Chan[i].iid =  UART_REG(UART_IID,i);
	i8250Chan[i].mdc =  UART_REG(UART_MDC,i);
	i8250Chan[i].lst =  UART_REG(UART_LST,i);
	i8250Chan[i].msr =  UART_REG(UART_MSR,i);

	i8250Chan[i].outByte = sysOutByte;
	i8250Chan[i].inByte  = sysInByte;

	i8250HrdInit(&i8250Chan[i]);
        }

    }
/******************************************************************************
*
* sysSerialHwInit2 - connect BSP serial device interrupts
*
* This routine connects the BSP serial device interrupts.  It is called from
* sysHwInit2().  
* 
* Serial device interrupts cannot be connected in sysSerialHwInit() because
* the kernel memory allocator is not initialized at that point, and
* intConnect() calls malloc().
*
* RETURNS: N/A
*
* SEE ALSO: sysHwInit2()
*/

void sysSerialHwInit2 (void)
    {
    int i;

    /* connect serial interrupts */

     for (i=0;i<N_UART_CHANNELS; i++)
         if (i8250Chan[i].int_vec)
	     {
             (void) intConnect (INUM_TO_IVEC (i8250Chan[i].int_vec),
                                i8250Int, (int)&i8250Chan[i] );
             sysIntEnablePIC (devParas[i].intLevel); 
             }

    }


/******************************************************************************
*
* sysSerialChanGet - get the SIO_CHAN device associated with a serial channel
*
* This routine gets the SIO_CHAN device associated with a specified serial
* channel.
*
* RETURNS: A pointer to the SIO_CHAN structure for the channel, or ERROR
* if the channel is invalid.
*/

SIO_CHAN * sysSerialChanGet
    (
    int channel		/* serial channel */
    )
    {
    return ( (SIO_CHAN *)&i8250Chan[channel] );
    }


