/* sysNvRam.c - NVRAM routines for the CPV5350 */

/* Copyright 2000 Wind River Systems, Inc. All Rights Reserved */
/* Copyright 2000 Motorola, Inc.  All Rights Reserved */

/*
modification history
--------------------
01a,19may00,djs  Initial Development 
*/

/* 
DESCRIPTION:

This file contains support routines which allow for the reading,
writing and manipulation of NVRAM.

When enabled, NVRAM is located in the memory map, in the
BIOS Option Expansion Area, starting at 0x000D0000. Only
32K bytes of the NVRAM are visible at one time. If the
offset requested is outside the current 32K bank window,
the bank selection will be switched to the next correct
bank. The NVRAM is only active during calls to
sysNvRamGet() or sysNvRamSet() routines. Using a 
memory dump routine to look at a 32K window will result
in incorrect values read back since the NVRAM is disabled.

*/

/* includes */

#include "vxWorks.h"
#include "config.h"

#define FPGA_DEVICE_SELECT      (0x0F)  /* select command */
#define FPGA_FLASH_DEVICE       (0x14)	/* flash device's indicie */

#define FPGA_ISA_BASE_ADRS      (0x58)
#define FPGA_INDEX	        (FPGA_ISA_BASE_ADRS + 0x05) /* index */
#define FPGA_DATA               (FPGA_ISA_BASE_ADRS + 0x07) /* data */

#define FPGA_NVRAM_DEVICE       (0x12)  /* NVRAM device index */
#define FPGA_NVRAM_PORT  	(0x01)  /* NVRAM control port */
#define FPGA_NVRAM_ENABLE 	(1<<7)  /* NVRAM enable */

#define NVRAM_OFFSET_MASK	0x00007FFF

/* locals */

LOCAL	UINT8 currentBank = 0xFF;

/* forward declarations */

LOCAL void sysNvEnableBank (UINT32 offset);
LOCAL UINT8 sysNvReadByte (UINT32 offset);
LOCAL void sysNvWriteByte (UINT32 offset, UINT8  value);

/*******************************************************************************
*
* sysNvDisable - Disable reads and writes to the NVRAM
*
* This routine disables the ability to read and write to the
* currently selected 32K byte NVRAM window.
*
* RETURNS: NA
*/

void sysNvDisable (void)

    {
    UINT8 nvCtrlPort;

    /* Read the NVRAM port */

    sysOutByte (FPGA_INDEX, FPGA_DEVICE_SELECT);   /* device select */
    sysOutByte (FPGA_DATA, FPGA_NVRAM_DEVICE); /* select slot ctrl device */
    sysOutByte (FPGA_INDEX, FPGA_NVRAM_PORT);  /* select slot ctrl port */
    nvCtrlPort = sysInByte (FPGA_DATA);

    /* Disable reads and writes access to NVRAM */

    nvCtrlPort &= ~(FPGA_NVRAM_ENABLE);

    /* Write out the updated port value */

    sysOutByte (FPGA_DATA, nvCtrlPort);   
    }

/*******************************************************************************
*
* sysNvEnable - Enable reads and writes to the NVRAM
*
* This routine enables the ability to read and write to the
* currently selected 32K byte NVRAM window.
*
* RETURNS: NA
*/

void sysNvEnable (void)

    {
    UINT8 nvCtrlPort;

    /* Read the NVRAM port */

    sysOutByte (FPGA_INDEX, FPGA_DEVICE_SELECT);   /* device select */
    sysOutByte (FPGA_DATA, FPGA_NVRAM_DEVICE); /* select slot ctrl device */
    sysOutByte (FPGA_INDEX, FPGA_NVRAM_PORT);  /* select slot ctrl port */
    nvCtrlPort = sysInByte (FPGA_DATA);

    nvCtrlPort |= (FPGA_NVRAM_ENABLE);

    /* Enable reads and writes access to NVRAM */

    sysOutByte (FPGA_DATA, nvCtrlPort);   
    }

/*******************************************************************************
*
* sysNvEnableBank - Enable the correct bank for the current offset
*
* This routine sets the correct 32K byte window, given the offset
* into NVRAM.
*
* RETURNS: NA
*/

LOCAL void sysNvEnableBank
    ( 
    UINT32 offset 	/* Address in nvram to read */
    )

    {
    UINT8 nvCtrlPort;
    UINT8 bank;
    UINT32 uBound;
    UINT32 lBound;

    bank = 0;
    lBound = 0;

    /*
     * Determine, based on the NVRAM address to write to,
     * which 32K bank needs to be activated.
     */

    for (uBound = NV_RAM_BANK_SIZE; uBound < NV_RAM_SIZE; 
		uBound += NV_RAM_BANK_SIZE)
	{
	if ((offset >= lBound) && (offset < uBound))
		{
		break;
		}
	else
		{
		bank++;
		lBound = uBound;
		}
	}

    /* 
     * If the currently active bank is the same as the 
     * bank needed for the current offset, we're done.
     */

    if (currentBank == bank)
	return;

    /* Read the current bank setting */

    sysOutByte (FPGA_INDEX, FPGA_DEVICE_SELECT);   /* device select */
    sysOutByte (FPGA_DATA, FPGA_NVRAM_DEVICE); /* select slot ctrl device */
    sysOutByte (FPGA_INDEX, FPGA_NVRAM_PORT);  /* select slot ctrl port */
    nvCtrlPort = sysInByte (FPGA_DATA);

    /* Clear out the old bank setting */

    nvCtrlPort &= ~(nvCtrlPort & 0x0f);

    /* Set the new bank setting */

    nvCtrlPort |= (FPGA_NVRAM_ENABLE | bank);

    /* Save the current active bank setting */

    currentBank = bank;

    /* Write out the new bank setting */

    sysOutByte (FPGA_DATA, nvCtrlPort);   
    }

/*******************************************************************************
*
* sysNvReadByte - Read a byte of NVRAM
*
* This routine reads a byte at the requeseted offset in NVRAM.
*
* RETURNS: NA
*/

LOCAL UINT8 sysNvReadByte 
    ( 
    UINT32 offset	/* Address in nvram to read */
    )

    {
    /* Enable the correct 32K bank for the requested address */

    sysNvEnableBank (offset);

    /* Read the value and return */

    return (*(UINT8 *)(NV_RAM_ADDRS + (offset & NVRAM_OFFSET_MASK)));
    }

/*******************************************************************************
*
* sysNvWriteByte - Write a byte of NVRAM
*
* This routine writes a byte at the requeseted offset in NVRAM.
*
* RETURNS: NA
*/

LOCAL void sysNvWriteByte 
    ( 
    UINT32 offset,	/* Address in nvram to write */
    UINT8  value	/* Value to write to NVRAM */
    )

    {
    /* Enable the correct 32K bank for the requested address */

    sysNvEnableBank (offset);

    /* Write the value */

    *(UINT8 *)(NV_RAM_ADDRS + (offset & NVRAM_OFFSET_MASK)) = value;
    }

/*******************************************************************************
*
* sysNvReadReg - Read the current NVRAM control register
*
* This routine reads the register in the FPGA that controls the
* current settings. Bit 7 NVRAM ENABLE, enabled=1, disabled=0
* Bits 6&5 unused. Bit 4 BATTLO, low=0, ok=1. Bits 3-0 Bank Selects.
*
* RETURNS: NA
*/

UINT8 sysNvReadReg (void)

    {
    UINT8 nvCtrlPort;

    /* Read the NVRAM port settings */

    sysOutByte (FPGA_INDEX, FPGA_DEVICE_SELECT);   /* device select */
    sysOutByte (FPGA_DATA, FPGA_NVRAM_DEVICE); /* select slot ctrl device */
    sysOutByte (FPGA_INDEX, FPGA_NVRAM_PORT);  /* select slot ctrl port */
    nvCtrlPort = sysInByte (FPGA_DATA);

    return (nvCtrlPort);
    }
