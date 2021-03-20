/* dec2155xSrom.c - DEC 2155X PCI-to-PCI bridge SROM library */

/* Copyright 2000 Motorola, Inc. All Rights Reserved */

/*
modification history
--------------------
01e,24mar00,scb  Beefed up commentary, WRS coding standards work.
01d,16dec99,scb  support for vxWorks shared memory dec21554 initialization
01c,09dec99,dmw  Fixed default formating comments.
01b,07dec99,dmw  Updated default SROM values to be almost the same between 
                 BSP's.
01a,16nov99,dmw  Written based on dec2155x.c from PPC6Bug base. 
*/

/*
DESCRIPTION

This library contains routines which allow for reading and writing of
the Dec2155x (nontransparent bridge) SROM.

*/

/* includes */

#include "vxWorks.h"
#include "stdio.h"
#include "config.h"
#include "vxLib.h"
#include "pci.h"		/* PCI header definitions */
#include "dec2155xCpci.h" /* DEC 2155X PCI-to-PCI bridge header */
#include "dec2155xSrom.h"		/* DEC2155x defines */

/* defines */

#if (_BYTE_ORDER != _BIG_ENDIAN)	

#	ifndef sysPciInByte
#		define sysPciInByte(addr) (*(UINT8 *)(addr))
#		define sysPciOutByte(addr,val) ( (*(UINT8 *)(addr)) = (UINT8)(val) )
#		define sysPciOutWord(addr,val) ( (*(UINT16 *)(addr)) = (UINT16)(val) )
#	endif /* defined sysPciInByte */

#   ifndef TRANSLATE
#   	define TRANSLATE(x,y,z)\
           ((UINT)(x) - (UINT)(y) + (UINT)(z))
#   endif /* TRANSLATE */

    IMPORT void	sysMsDelay (UINT32);	/* x86 ms Delay function */
#   define DELAY(x) sysMsDelay(x)

#define BRDG_CLASS_CODE 0x02
#else  /* _BYTE_ORDER == _BIG_ENDIAN */

    IMPORT void	sysDelay (UINT32);		/* PPC ms Delay function */
#   define DELAY(x) sysDelay(x)

#define BRDG_CLASS_CODE 0x20
#endif /* _BYTE_ORDER == _BIG_ENDIAN */

#ifndef DEC2155X_SEC_CSR
#   define DEC2155X_SEC_CSR(offset) (localDec2155xCsrAdrs+offset)
#endif

#define DEC2155X_SROM_USED 0x42	/* number of bytes actually used currently */

/* globals */

UINT32 localDec2155xCsrAdrs = 0;	/* Current DEC2155X's CSR BAR */
UINT8  currentDevSel = 0;		/* Current DEC2155X's DEVSEL */
UINT8  interactDec2155x = 0;		/* printf messages during writing  */

/* forward declarations */

LOCAL  STATUS sysDec2155xSromDeviceVerify (UINT devSel);
LOCAL  UINT   sysDec2155xCmd (UINT romaddrop, UINT romdata);
LOCAL  STATUS sysDec2155xReady (void);

STATUS sysDec2155xSromWrite   (UINT devSel, UINT8 *buffer);
STATUS sysDec2155xSromRead    (UINT devSel, UINT8 *buffer);
STATUS sysDec2155xOffsetRead  (UINT devSel, UINT offset, UINT8 *value);
STATUS sysDec2155xOffsetWrite (UINT devSel, UINT offset, UINT8 value);
STATUS sysDec2155xShow        (UINT devSel);

/****************************************************************************
*
* sysDec2155xOffsetRead - Read a byte from the DEC2155x SROM.
*
* This routine reads a single byte from the Dec2155x SROM from a 
* specified offset.
*
* RETURNS: OK if no problem, ERROR if bad 'devSel' parameter or 'offset' 
* parameter is presented.
*/

STATUS sysDec2155xOffsetRead
    (
    UINT devSel, 	/* PCI device number of the Dec2155x */
    UINT offset, 	/* Offset within SROM at which byte is written */
    UINT8 *value	/* Value read from SROM is returned here */
    )
    {

    if (sysDec2155xSromDeviceVerify (devSel) != OK)
        return (ERROR);

    if (offset >= DEC2155X_SROM_SIZE)
        return (ERROR);

    offset = offset & (DEC2155X_SROM_SIZE-1);

    *value = sysDec2155xCmd(DEC2155X_SROM_READ + offset, 0);

    return(OK);
    }


/****************************************************************************
*
* sysDec2155xOffsetWrite - Write a byte to the DEC2155x SROM.
*
* This routine writes a single byte to the Dec2155x SROM at a specified 
* offset.
*
* RETURNS: OK if no problem, ERROR if bad devSel parameter or offset 
* parameter is presented.
*/

STATUS sysDec2155xOffsetWrite
    (
    UINT devSel, 	/* PCI device number of the Dec2155x */
    UINT offset, 	/* Offset of byte-to-write in the SROM */
    UINT8 value         /* Byte value to write to SROM */
    )
    {

    if (sysDec2155xSromDeviceVerify (devSel) != OK)
        return (ERROR);

    offset = offset & (DEC2155X_SROM_SIZE-1);

    /* Enable writes and erase cmds */

    if (sysDec2155xCmd(DEC2155X_SROM_EWEN, 0)) 
        {
        return(ERROR);
        }

    /* Erase the byte at offset */

    if (sysDec2155xCmd(DEC2155X_SROM_ERASE + offset, 0))  
        {
        return(ERROR);
        }

    /* Write the new byte value at offset */

    sysDec2155xCmd(DEC2155X_SROM_WRITE + offset, value);

    /* Disable writes and erase cmds */

    if (sysDec2155xCmd(DEC2155X_SROM_EWDS, 0)) 
        {
        return(ERROR);
        }

    return(OK);
    }

/****************************************************************************
*
* sysDec2155xCmd - SROM Command interface.
*
* This routine performs a specified DEC21554 SROM primitive command.
* Primitive commands are: write enable, write disable, write all, erase all,
* write, read, erase.
*
* RETURNS:  CSR ROM data byte if command execution completed, (UINT)-1 if 
* SROM command fails due to not ready condition.
*/

LOCAL UINT sysDec2155xCmd
    (
    UINT romaddrop, 	/* SROM address/opcode bits */
    UINT romdata	/* SROM data */
    )
    {

    /* Wait for SROM interface to be ready */

    if (sysDec2155xReady() < 0) 
        {
        return(-1);
        }

    /* 
     * Setup the address and data registers and start the
     * command
     */

    sysPciOutWord (DEC2155X_SEC_CSR(DEC21554_CSR_ROM_ADRS), romaddrop);
    sysPciOutByte (DEC2155X_SEC_CSR(DEC2155X_CSR_ROM_DATA), romdata); 

    sysPciOutByte (DEC2155X_SEC_CSR(DEC2155X_CSR_ROM_CTRL), 
                   sysPciInByte (DEC2155X_CSR_ROM_CTRL) | 
		   DEC2155X_SROM_START);

#if (_BYTE_ORDER == _BIG_ENDIAN) 
    /* extra sufficient delay for PPC processors */

    DELAY(100);
#endif

    /* 
     * DEC Errata workaround: poll bit may not reliably work so
     * simply hardcode a sufficient delay for the command
     * to complete.
     */

    switch(romaddrop & 0x700) 
        {
        case DEC2155X_SROM_WRITE:
        case DEC2155X_SROM_ERASE: 
            DELAY(10);
            break;
        default:
            DELAY(1);
            break;
        }

    /* Wait for SROM interface to be ready */

    if (sysDec2155xReady() < 0) 
        {
        return(-1);
        }

    /* 
     * Clear the POLL bit if it is set (note the 
     * explicit delay above ensures the command has 
     * already completed...).
     */

    if (sysPciInByte (DEC2155X_SEC_CSR(DEC2155X_CSR_ROM_CTRL)) & 
        DEC2155X_SROM_POLL)
        {
        sysPciOutByte (DEC2155X_SEC_CSR(DEC2155X_CSR_ROM_CTRL), 
                       sysPciInByte (DEC2155X_CSR_ROM_CTRL) |
		               DEC2155X_SROM_START);
        DELAY(1);
	}

    return (sysPciInByte (DEC2155X_SEC_CSR(DEC2155X_CSR_ROM_DATA))); 
    }

/****************************************************************************
*
* sysDec2155xReady - Wait for SROM to indicate it is ready.
*
* This routine polls the Dec2155x SROM for a ready condition until a
* timeout occurs or until a ready condition is presented.
*
* RETURNS: OK if Dec2155x SROM is ready, ERROR if timeout expires without
* ready condition being presented.
*/

LOCAL STATUS sysDec2155xReady (void)
    {
    int	timeout = 0;

    /* Wait for previous command to be sent */

    while (sysPciInByte (DEC2155X_SEC_CSR(DEC2155X_CSR_ROM_CTRL)) & 
           DEC2155X_SROM_START) 
        {
        if (timeout++ > DEC2155X_SROM_CMDTIMEOUT) 
            {
            return(ERROR);
            }
        DELAY(1);
        }

    /*
     * Errata workaround: POLL bits are not reliable so
     * simply return without checking them.  The caller
     * must explicitly delay to allow command to complete.
     */

    return(OK);
    }


/****************************************************************************
*
* sysDec2155xSromDeviceVerify - Verify that specified device is a Dec21554.
*
* If a nonzero device selection parameter is supplied to this routine, PCI 
* bus zero is examined for a DEC21554 device present at the specified 
* device number.  If the device is a DEC21554 device, it's CSR address 
* read from the PCI configuration header and is saved in 
* 'localDec2155xCsrAdrs'.  Future calls to the functions 
* sysDec2155xSromRead() and sysDec2155xSromWrite() will use this dynamically 
* determined CSR address.
*
* RETURNS: OK if specified device is a DEC21554, ERROR otherwise.
*/

LOCAL STATUS sysDec2155xSromDeviceVerify 
    (
    UINT devSel		/* Device number to verify as a DEC21554 */
    )

    {
    UINT devVend = 0;

    /* 
     * Calculate the DEC2155x's CSR Memory BAR if unknown or the current 
     * DEVSEL is not the same as the supplied devSel.
     */

    if ((localDec2155xCsrAdrs == 0) || (currentDevSel != devSel))
        {
        if (devSel)
            {
            pciConfigInLong (0, devSel, 0, PCI_CFG_VENDOR_ID, &devVend);
            if (devVend != PCI_ID_BR_DEC21554)
                {
                printf("ERROR: Device at DEVSEL 0x%x is not a DEC21554\r\n",
                        devSel);
                return (ERROR);
                }
            currentDevSel = devSel;
            }
        else
            {
            printf("ERROR: DEVSEL not supplied\r\n");
            return (ERROR);
            }

        /* read the Dec2155x CSR Memory BAR, convert to CPU view and save */

        pciConfigInLong (0, devSel, 0, DEC2155X_CFG_SEC_CSR_MEM_BAR, 
                         (UINT32 *)(&localDec2155xCsrAdrs));

        localDec2155xCsrAdrs = 
            TRANSLATE((localDec2155xCsrAdrs & PCI_MEMBASE_MASK),
                      PCI_MSTR_MEMIO_BUS, PCI_MSTR_MEMIO_LOCAL);
        }

    return (OK);
    }


/****************************************************************************
*
* sysDec2155xSromRead - Read the entire DEC21554 SROM.
*
* This routine reads the DEC21554 SROM contents into a specified buffer.  
* The number of bytes read is specified by DEC2155X_SROM_SIZE.
*
* RETURNS: OK if read was successful, ERROR if not.
*/

STATUS sysDec2155xSromRead
    (
    UINT devSel, 	/* PCI device number of the Dec2155x */
    UINT8 *buffer 	/* Buffer in which to place the SROM contents */
    )
    {
    UINT offset;

    for (offset = 0; offset < DEC2155X_SROM_SIZE; offset++)
        if (sysDec2155xOffsetRead(devSel, offset, buffer++) == ERROR)
            return (ERROR);

    return (OK);
    }


/****************************************************************************
*
* sysDec2155xSromWrite - Write the entire DEC2155x SROM.
*
* This routine writes to the DEC21554 from the specified buffer.  No check 
* is made for validity of data contents.  The number of bytes written from 
* the buffer is specified by DEC2155X_SROM_SIZE.
*
* RETURNS: OK if write was successful, ERROR if not (buffer may have
* been partially written in this case).
*/

STATUS sysDec2155xSromWrite
    (
    UINT devSel, 	/* PCI device number of the Dec2155x */
    UINT8 *buffer 	/* Buffer from which SROM contents are written */
    )
    {
    UINT  offset;
    UINT8 *orgData = buffer;
    UINT8 verifyBuf[DEC2155X_SROM_SIZE];

    if (sysDec2155xSromDeviceVerify (devSel) != OK)
        return (ERROR);

    for (offset = 0; offset < DEC2155X_SROM_SIZE; offset++)
        {

        if (interactDec2155x) printf("Writing 0x%3.3x\r", offset);

        if (sysDec2155xOffsetWrite(devSel, offset, *buffer++) == ERROR)
            {
            printf("\nWrite SROM failed.\r\n");
            return (ERROR);
            }
        }

    if (interactDec2155x) printf("\r");

    for (offset = 0; offset < DEC2155X_SROM_SIZE; offset++)
        {

        if (interactDec2155x) printf("Verifying 0x%3.3x    \r", offset);

	    if (sysDec2155xOffsetRead(devSel, offset, &verifyBuf[offset]) == ERROR)
            {
            printf("\nWrite SROM Read-Verify failed.\r\n");
            return (ERROR);
            }

        if (*orgData++ != verifyBuf[offset])
            {
            printf("\nWrite SROM Verify data failed.\r\n");
            return (ERROR);
            }
        }

    if (interactDec2155x) printf("\rDone.               \r");

    return (OK);
    }

/****************************************************************************
*
* sysDec2155xShow - Print the DEC2155X SROM contents.
*
* This routine produces a formatted print of the DEC21554 device's SROM 
* contents.  The actual number of bytes displayed is specified by 
* DEC2155X_SROM_USED. 
*
* RETURNS: OK if complete printout produced, ERROR otherwise.
*/

STATUS sysDec2155xShow
    (
    UINT devSel 	/* PCI device number of the Dec2155x */
    )

    {
    UINT offset;
    UINT8 value;

    printf("\r\n");
    for (offset = 0; offset <= DEC2155X_SROM_USED; offset++)
        {
        if (!(offset % 16)) printf("\r\n %3.3x:", offset);

        if (!(offset % 8)) printf(" ");

        if (sysDec2155xOffsetRead(devSel, offset, &value) == ERROR)
            {
            printf("Read failed at %x\r\n", offset);
            return (ERROR);
            }
        printf("%2.2x ", value);
        }
    printf("\r\n");

    return (OK);
    }

/****************************************************************************
*
* sysDec2155xSromDefault - Program DEC2155x SROM contents with a defaults.
*
* This routine programs the DEC21554 SROM with the following "default" 
* contents:
*
* .CS
* Offset    Size    Value          Description/comments
* 00        4       80,00,00,00    ROM present and reserved bytes
* 04        1       00             Primary programming interface
* 05        1       02             (Pentium) or ...
*                   20             (PowerPC)       Primary Sub-class code
* 06        1       0B             (Bridge Device) Primary Base class code
* 07        2       57, 10         Subsystem vendor ID (Motorola)
* 09        2       14, 48         (CPN5360) Subsystem Device ID
* 0B        1       00             Primary minimum grant
* 0C        1       00             Primary maximum latency
* 0D        1       00             Secondary programming interface
* 0E        1       80             Secondary Sub-class code
* 0F        1       06             Secondary Base class code
* 10        1       00             Secondary minimum grant
* 11        1       00             Secondary maximum latency
* 12        4       00, 00, 00, 00 Downstream memory 0 setup register
* 16        4       08, 00, 00, FF Downstream memory 1 setup register (32MB)
* 1A        4       08, 00, 00, FF Downstream memory 2 setup register (32MB)
* 1E        4       00, 00, 00, 00 Downstream memory 3 setup register
* 22        4       00, 00, 00, 00 Downstream memory 3 upper setup register
* 26        2       00, 00         Primary expansion ROM setup register
* 28        4       08, 00, F0, FF Upstream memory 0 setup register (1MB)
* 2C        4       08, 00, F0, FF Upstream memory 1 setup register (1MB)
* 30        2       00, 04         Chip control 0 register
* 32        2       00, 00         Chip control 1 register
* 34        2       00, 02         Arbiter control register
* 36        1       00             Primary SERR Disables
* 37        1       00             Secondary SERR Disables
* 38        8       00,01,02,03,
*                   04,05,06,07    PCI Power Management Data Register Values
* 40        1       00             Compact PCI Hot-Swap ECP ID
* 41        2       01, 00         PCI Power Management and BIST
* 43        3D      00,00,00 ...   Unused and reserved
* 80        180     00,00,00 ...   VPD data (unused currently)
* .CE
*
* RETURNS: OK if programming succeeded, ERROR otherwise.
*/

STATUS sysDec2155xSromDefault
    (
    UINT devSel 	/* PCI device number of the Dec2155x */
    )
    {

    UINT8 mBuf[DEC2155X_SROM_SIZE] = 
	{
        0x80, 0x00, 0x00, 0x00, 0x00, BRDG_CLASS_CODE, 0x0b, 0x57, 0x10, 0x14, 
        0x48, 0x00, 0x00, 0x00, 0x80, 0x06, 0x00, 0x00, 0x00, 0x00,

        0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x08, 0x00, 0xf0, 0xff, 0x08, 0x00, 0xf0, 0xff, 0x00, 0x04,
        0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03,

        0x04, 0x05, 0x06, 0x07, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff
        };

    interactDec2155x = 1;

    if (sysDec2155xSromWrite(devSel, mBuf) == ERROR)
        {
        interactDec2155x = 0;
        return (ERROR);
        }

    interactDec2155x = 0;

    return (OK);
    }


/****************************************************************************
*
* sysDec2155xSromClear - Clear the DEC21554 SROM.
*
* This routine clears the DEC21554 SROM contents by writing 0xff to each
* byte.  The actual number of bytes written is specified by
* DEC2155X_SROM_SIZE. 
*
* RETURNS: OK if clearing operation succeeded, ERROR otherwise.
*/

STATUS sysDec2155xSromClear
    (
    UINT devSel 	/* PCI device number of the Dec2155x */
    )
    {
    int offset = 0;
    for (offset = 0; offset < DEC2155X_SROM_SIZE; offset++)
        {
        if (sysDec2155xOffsetWrite(devSel, offset, 0xff) == ERROR)
            return (ERROR);
        }

    return (OK);
    }

#ifdef INCLUDE_DEC2155X
#include "dec2155xSromSmProg.c"
#endif
