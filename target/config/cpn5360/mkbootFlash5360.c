/* mkbootFlash5360.c - Program Flash Memory Command Source-Module */

/* Copyright 2000-2002 Wind River Systems, Inc. */
/* Copyright 2000 Motorola, Inc. All Rights Reserved */

/*
modification history
--------------------
01h,09may02,dat  Update for T2.2, fixed warnings, added binary check,
		 fixed block erase in romFixup, chg'd to support binary
		 formats only.
01g,30may00,scb  romFixup() bug did not copy entire jmp instruction.
01f,20mar00,djs  Initial code review changes.
01e,29mar00,scb  Removed -h (host), -g (gateway), -n (filename) options
01d,09mar00,dmw  Fixed host option.
01c,03mar00,scb  Fixed problem loading image from NT workstation.
01b,14feb00,dmw  Added "standard" open/read to be more like mkboot.
01a,08feb00,dmw  Created from flashProgram.c from mv2100 BSP.
*/

/*
DESCRIPTION

This module provides the functions needed to download a binary image
and to program the flash bank.  The binary image name and flash bank number
are user defined.

The flash routines do not make use of the TFFS libraries.

The flash routines are to be used only on the Intel i28f320.
*/

/* includes */

#include "vxWorks.h"
#include "taskLib.h"
#include "stdio.h"
#include "ctype.h"
#include "string.h"
#include "sysLib.h"
#include "config.h"

/* network includes */

#include "fioLib.h"

/* defines */

#define SHOW_PROGRESS	0x1000     /* when to update progress counter */
#define ROM_BANK_SIZE	0x00020000 /* ROM Bank size 128K */
#define MIN_FLASH_BANK	0          /* minimum flash bank, BIOS */
#define MAX_FLASH_BANK	7          /* maximum flash bank */
#define IN_RANGE(x,y,z) (((x) >= y) && ((x) <= z))

#define LOADER_SIZE	0xff           /* ROM loader code at end of ROM */

/* Flash commands/status */

#define PROGRAM_SETUP	0x40
#define PROGRAM_RESUME	0xD0
#define CLEAR_STATUS	0x50
#define CLEAR_LOCK_BIT	0x60
#define READ_ID 	0x90
#define ERASE_SETUP	0x20
#define ERASE_CONFIRM	0xd0
#define READ_ARRAY	0xff

#define WSMRDY	0x80
#define WSMPROTECTERR	0x02	

#define FST_INSTR_OFFSET    (0xf)   /* 1st instr is at 0xfffffff0 */
#define FILETYPE_OFFSET     (0x100) /* start address programmed at 0xffffff00 */
#define MAX_FILE_SIZE   0x100000    /* 1 meg maximum for download */

/* 
 * The following #define encodes the address of the last byte of the 
 * BIOS PROM 
 */

#define BIOS_PROM_LAST_BYTE (BIOS_PROM_BASE + BIOS_PROM_SIZE - 1)

/*
 * The following #define is the compile time calculated length of the "jmp"
 * instruction which appears at the very end of "copyRom.s".
 */

#define JMP_INSTR_SIZE ((UINT32)romCopyEnd - (UINT32)romJumpLoc)

#define WAIT_RDY        while (!(*flashPointer & WSMRDY) && (timeout--))

/* globals */

/* 
 * User download area, created the first time the download routine
 * is used.
 */

char *userDownload = NULL;
int downloadSize = 0;

/* imports */

IMPORT int romCopyStart(void);
IMPORT int romCopyEnd(void);
IMPORT int romJumpLoc(void);

IMPORT int intLock (void);
IMPORT void intUnlock (int);

IMPORT int sysWarmType;

IMPORT int printf ();		/* formatted print */
IMPORT STATUS tftpCopy
    (
    char *		pHost,			/* host name or address	*/
    int			port,			/* optional port number	*/
    char *		pFilename,		/* remote filename 	*/
    char *		pCommand,		/* TFTP command 	*/
    char *		pMode,			/* TFTP transfer mode 	*/
    int	 		fd 			/* fd to put/get data   */
    );
IMPORT STATUS tftpXfer 
    (
    char *		pHost,			/* host name or address */
    int			port,			/* port number		*/
    char *		pFilename,		/* remote filename 	*/
    char *		pCommand,		/* TFTP command 	*/
    char *		pMode,			/* TFTP transfer mode 	*/
    int	*		pDataDesc,		/* return data desc.	*/
    int	*		pErrorDesc 		/* return error desc.	*/
    );
IMPORT int rcmd (char *host, int remotePort, char *localUser,
                 char *remoteUser, char *cmd, int *fd2p);
IMPORT void inet_netof_string (char *inetString, char *netString);
IMPORT STATUS routeAdd (char *destination, char *gateway);
IMPORT STATUS memDrv(void);
IMPORT STATUS memDevCreate (char *name, char *base, int length);

IMPORT int sysFlags;

/* forward declarations */

LOCAL  int    gevYNPrompt 	(char *prompt, unsigned char *buffer, 
				 char defResp);
LOCAL STATUS enableFlashWrite	(int bank);
LOCAL STATUS disableFlashWrite	(int bank);
LOCAL STATUS eraseFlashBlock	(int offset);
LOCAL STATUS unlockFlashBank	(char *flashPointer, char *source);
LOCAL STATUS programFlash (UCHAR *source, UCHAR *flashPointer, int numBytes);
LOCAL STATUS programFlashReverse(UCHAR *source, UCHAR *flashPointer, 
				  int numBytes);
LOCAL STATUS romFixup 		(char *romCopyCode, int romCopyBytes, 
				 char *loadAddress);
LOCAL STATUS readIdentFlash	(int bank);
LOCAL void   flashCtrlWrite	(UINT8 valWrite);
LOCAL STATUS download 		(char *fileName);

LOCAL char mkbootFlash5360FullExplanation[] = { "\
bank# bank number to program\r\n\
filename to download\r\n\
syntax: mkbootFlash5360 bank#, \"<filename>\"\r\n\
" };

/***************************************************************************
*
* gevYNPrompt - printf question and wait for input.
*
* This function's purpose is to prompt the operator
* for input.
*
* RETURNS: always zero, buffer filled in with y/n.
*/

LOCAL int gevYNPrompt 
    (
    char *prompt,          /* pointer prompt to be displayed */
    unsigned char *buffer, /* pointer to response buffer */
    char defResp           /* default response to be displayed */
    ) 
    {
    printf(prompt);
    printf(" (y/n) %c: ",defResp);

    if (gets(buffer) == NULL)
       buffer[0] = 'n';

    buffer[0] = tolower(buffer[0]);
    return (0);
    }

/***************************************************************************
*
* enableFlashWrite - enable flash bank for writing.
*
* This function's purpose is to select and enable the flash bank
* in the FPGA device and set the enable bits in the PIIX4.
*
* RETURNS: OK, or ERROR if bank out of range.
*/

LOCAL STATUS enableFlashWrite 
    (
    int bank    /* bank number to enable writes to */
    )
    {
    USHORT enable;

    if (!IN_RANGE (bank, (MIN_FLASH_BANK + 1), MAX_FLASH_BANK))
        return (ERROR);

    flashCtrlWrite (0x80 | bank);	/* select bank + write enable */

    /* enable Extended BIOS, lower BIOS and BIOSCS write protect at the PIIX4 */

    pciConfigInWord (0, 7, 0, X_BUS_CS_REG, &enable);
    enable |= (BIOSC_WP_ENABLE | LOWER_BIOS_EN | UPPER_BIOS_EN);
    pciConfigOutWord(0, 7, 0, X_BUS_CS_REG, enable); 

    return (OK);
    }


/***************************************************************************
*
* disableFlashWrite - disable flash bank for writing.
*
* This function's purpose is to select and disable the flash bank
* in the FPGA device and clear the flash BIOS write enable bit in the PIIX4.
*
* RETURNS: OK, or ERROR if bank out of range.
*/

LOCAL STATUS disableFlashWrite 
    (
    int bank    /* bank number to disable writes to */
    )
    {
    USHORT enable;

    if (!IN_RANGE (bank, MIN_FLASH_BANK, MAX_FLASH_BANK))
        return (ERROR);

    flashCtrlWrite (0x00 | bank);	/* select bank + no write protect */

    pciConfigInWord (0, 7, 0, X_BUS_CS_REG, &enable);
    enable &= ~BIOSC_WP_ENABLE;
    pciConfigOutWord(0, 7, 0, X_BUS_CS_REG, enable); 

    return (OK);
    }


/***************************************************************************
*
* eraseFlashBlock - erase flash bank offset.
*
* This function's purpose is to erase a flash bank block at
* the offset specified.
*
* RETURNS: OK if erase succeeded, ERROR otherwise.
*/

LOCAL STATUS eraseFlashBlock
    (
    int offset    /* offset into current flash bank */
    )
    {
    UCHAR *flashPointer = (UCHAR *)(BIOS_PROM_BASE + offset);
    int timeout = -1;

    *flashPointer = CLEAR_STATUS;	/* send clear status command 0x50 */
    sysDelay();

    *flashPointer = ERASE_SETUP;	/* send erase setup command 0x20 */
    sysDelay();

    *flashPointer = ERASE_CONFIRM;	/* send erase confirm command 0xd0 */

    if (*flashPointer == 0xFF)
        {
        printf("Error erasing Flash Block\r\n");
        return (ERROR);
        }

    WAIT_RDY;

    if ((*flashPointer & WSMRDY) != WSMRDY)
        {
        printf("\r\nERROR %x\r\n", *flashPointer);
        *flashPointer = READ_ARRAY;
        return (ERROR);
        }

    *flashPointer = READ_ARRAY;

    return (OK);

    }


/***************************************************************************
*
* unlockFlashBank - unlock flash bank.
*
* This function's purpose is to unlock a flash bank at address 
* "flashPointer" and program the address with the data found at "source".
*
* RETURNS: OK if unlocking succeeded, ERROR otherwise.
*/

LOCAL STATUS unlockFlashBank 
    (
    char *flashPointer,    /* flash bank address */
    char *source           /* pointer to memory to be programmed */ 
    )
    {
    int timeout = -1;

    *flashPointer = CLEAR_STATUS;	/* send clear status command 0x50 */
    sysDelay();

    *flashPointer = CLEAR_LOCK_BIT;	/* 0x60 */
    *flashPointer = PROGRAM_RESUME;	/* 0xD0 */
    sysDelay();

    *flashPointer = PROGRAM_SETUP;	/* 0x40 */
    sysDelay();
    *flashPointer = *source;

    WAIT_RDY;

    *flashPointer = READ_ARRAY;	/* restore read array mode 0xFF */
    sysDelay();

    if (*flashPointer == *source)
        return (OK);

    printf("Unlock failed %x\r\n", *flashPointer);
    return (ERROR);
    }


/***************************************************************************
*
* programFlash - program flash bank.
*
* This function's purpose is to program a flash bank at address 
* "flashPointer" with the data found at "source" for the "numBytes"
* specified.
*
* RETURNS: OK if flash programming succeeded, ERROR otherwise.
*/

LOCAL STATUS programFlash 
    (
    UCHAR *source,       /* pointer to memory source */
    UCHAR *flashPointer, /* pointer to flash device */ 
    int numBytes         /* number of bytes to program */
    )
    {
    int timeout = -1; 

    for (; numBytes--; flashPointer++, source++)
        {

        if (!((UINT)flashPointer % SHOW_PROGRESS))
            printf("Programming Src: 0x%08X, Dest: 0x%08X\r",
                   (UINT32)source, (UINT32)flashPointer);

        /* send program setup command 0x40 */

        *flashPointer = PROGRAM_SETUP;	

        sysDelay();

        *flashPointer = *source;	/* send the data */
    
	timeout = -1;
        WAIT_RDY;
    
        *flashPointer = READ_ARRAY;	/* restore read array mode */

        /* check that data matches flash contents */

        if (*flashPointer != *source)
            {
            printf("\r\nError: Data miscompare at %p found %x expected %x\r\n",
                   flashPointer, *flashPointer, *source);

            *flashPointer = PROGRAM_SETUP;
            *flashPointer = *source;
            printf("Status %x\r\n", *flashPointer);

            /* bank lock bit set, attempt recovery */

            if (*flashPointer & WSMPROTECTERR)
                {
                return (unlockFlashBank (flashPointer, source));
                }	
            return (ERROR);
            }
        }

    return (OK);
    }


/***************************************************************************
*
* programFlashReverse - program flash bank last byte first.
*
* This function's purpose is to program a flash bank at address 
* "flashPointer - numBytes" with the data found at "source - numBytes" 
* for the "numBytes" specified.
*
* RETURNS: OK, or ERROR if data miscompare.
*/

LOCAL STATUS programFlashReverse 
    (
    UCHAR *source,       /* pointer to memory source */
    UCHAR *flashPointer, /* pointer to flash destination */
    int numBytes         /* number of bytes to program */
    )
    {
    int timeout = -1;

    for (; numBytes--; flashPointer--, source--)
        {
        if (!((UINT)flashPointer % SHOW_PROGRESS))
            printf("Programming Src: 0x%08X, Dest: 0x%08X\r",
                   (UINT32)source, (UINT32)flashPointer);

        /* send program setup command 0x40 */

        *flashPointer = PROGRAM_SETUP;	

        sysDelay();

        *flashPointer = *source;	/* send the data */
    
        WAIT_RDY;
    
        *flashPointer = READ_ARRAY;	/* restore read array mode */

        /* check that data matches flash contents */

        if (*flashPointer != *source)
            {
            printf("\r\nError: Data miscompare at %p found %x expected %x\r\n",
                   flashPointer, *flashPointer, *source);

            *flashPointer = PROGRAM_SETUP;
            *flashPointer = *source;
            printf("Status %x\r\n", *flashPointer);

            /* bank lock bit set, attempt recovery */

            if (*flashPointer & WSMPROTECTERR)
                {
                return (unlockFlashBank (flashPointer, source));
                }	
            return (ERROR);
            }
        }

    return (OK);
    }


/***************************************************************************
*
* romFixup - program cpn5360/Phoenix specifics.
*
* The cpn5360's Phoenix BIOS is capable of booting from any of the 6
* other flash banks on the board.
*
* If the BIOS is set up to boot from a flash bank, the BIOS loads segment 
* F000h (F000:0000h - F000:FFFFh or flat 32-bit address 000F0000h - 
* 000FFFFFh) with the top 64K of the flash bank to be booted.
*
* The BIOS checks for a eye-catcher "_MOT_" at FFFbh and if this is true,
* jumps to the instruction found at FFF0h.
*
* This function's purpose is to program the last five bytes of the flash
* bank with the eye-catcher "_MOT_".  The program also writes a hunk of
* code found in copyRom.s to copy the ROM into RAM.  The first instruction
* executed at FFF0h is a jump backwards to the copy routines.
*
* This routine also programs the load address into the ROM to be used by
* the loader to place the code to the correct high or low address.
*
* RETURNS: OK, or ERROR if flashing fails.
*/

LOCAL STATUS romFixup 
    (
    char *romCopyCode, /* pointer to copyRom() */
    int romCopyBytes,  /* the number of bytes making up copyRom() */
    char *loadAddress  /* the address RAM_HIGH_ADRS or RAM_LOW_ADRS. */
    )
    {
    printf("\r\nAdding BIOS bootloader.                                     ");

    if (programFlash (loadAddress, 
                      (UCHAR *)(BIOS_PROM_BASE + 
                      (BIOS_PROM_SIZE - FILETYPE_OFFSET)), 4) 
        == ERROR)
        {
        printf("Program flash file type failed\n");
        return (ERROR);
        }

    /* 
     * Program the "copyRom.s" image into flash, making sure that
     * the last instruction of the module ("jmp _copyRom") begins
     * FST_INSTR_OFFSET = 0xf bytes behind the last byte in the
     * BIOS PROM.
     */

    if (programFlashReverse ((char *)((UINT32)romJumpLoc + JMP_INSTR_SIZE - 1),
                             (char *)(BIOS_PROM_LAST_BYTE - 
				      FST_INSTR_OFFSET +
				      JMP_INSTR_SIZE - 1),
                             (romCopyBytes))
        == ERROR)
        {
        printf("Program Flash failed\r\n");
        return (ERROR);
        }
    

    /* Add "_MOT_" eyecatcher */

    return (programFlash (ROM_SIGNATURE, 
                          (UCHAR *)(BIOS_PROM_BASE + 
                          (BIOS_PROM_SIZE - ROM_SIGNATURE_SIZE)), 
                          ROM_SIGNATURE_SIZE));
    }


/***************************************************************************
*
* readIdentFlash - verify flash manufacturer.
*
* This function's purpose is to verify the flash manufacturer and device.
*
* RETURNS: OK if flash indentifier successfully read and it indicates
* INTEL_MANUFACTID and I28F320PARTID, ERROR otherwise.
*/

LOCAL STATUS readIdentFlash
    (
    int bank /* bank number to read identifier from */
    )
    {
    UCHAR *flashPointer = (UCHAR *)BIOS_PROM_BASE;

    if (!IN_RANGE (bank, (MIN_FLASH_BANK + 1), MAX_FLASH_BANK))
        return (ERROR);

    enableFlashWrite (bank);

    *flashPointer = READ_ID;	/* send read identifiers command */

    if (*flashPointer != INTEL_MANUFACTID)
        {
        printf("Invalid flash manufacturer 0x%x\n", *flashPointer);
        *flashPointer = READ_ARRAY;	/* restore read array mode */
        disableFlashWrite (bank);
        return (ERROR);
        } 

    printf("Flash Manufacturer: 0x%X, ID: 0x%X\n", (USHORT)*flashPointer,
           (USHORT)*(flashPointer + 2));

    if (*(flashPointer + 2) != I28F320PARTID)
        {
        printf("Invalid flash Id %x\n", *(flashPointer + 2));
        *flashPointer = READ_ARRAY;	/* restore read array mode */
        disableFlashWrite (bank);
        return (ERROR);
        } 

    *flashPointer = READ_ARRAY;	/* restore read array mode */

    disableFlashWrite (bank);

    return (OK);

    }

/***************************************************************************
*
* flashCtrlWrite - write to Flash Control regs
*
* This routine writes to the FPGA's Flash Control register.
* The address as viewed from the CPU, is passed in along with the
* value to be written.  
*
* RETURNS: NA
*/

LOCAL void flashCtrlWrite
    (
    UINT8 valWrite	        /* value to be written */
    )
    {

    /* command "device select" */

    sysOutByte (FPGA_INDEX, FPGA_DEVICE_SELECT);

    /* command system device to do a device select */

    sysOutByte (FPGA_DATA, FPGA_FLASH_DEVICE);

    /* select the PLFPGA's flash control register */ 

    sysOutByte (FPGA_INDEX, FPGA_FLASH_CONTROL);

    /* write value to the PLFPGA's flash control register */ 

    sysOutByte (FPGA_DATA, valWrite);

    }


/***************************************************************************
*
* mkbootFlash5360 - flash memory program command
*
* This function's purpose is to provide the capability to "flash" a VxWorks 
* image into bank 1-7 of the cpn5360's onboard ROM flash.
*
* The utility is modeled after mkbootFd or mkbootAta.  This utility is
* passed a bootrom image name and the flash bank number (1-7) to write
* to.  The bootable image is copied across the network to the target and 
* the flash bank programmed.  The utility runs from the VxWorks kernel shell.  
* 
* Syntax:
* 
* mkbootFlash5360 bank_no, "bootimage name"
* 
* The image is copied to the "opposite" memory space from where it 
* is going to eventually execute.  The following table illustrates 
* this:
*  
* .CS
*   Name            Description         Compressed  Loads Into
*   ----            -----------         ----------  ----------
*   vxWorks.st_rom.bin  bootable vxWorks.st Yes         High Memory
*   bootrom.bin         bootrom             Yes         Low Memory
* .CE
* 
* Non-compressed images are not supported at this time.
*
* RETURNS: OK, or ERROR if flash programming error occurs.
*/

STATUS mkbootFlash5360
    (
    int  bank,      /* flash bank number to be programmed */
    char *fileName  /* filename of ROM image */
    )
    {
    int ospl;                 /* Saved interrupt level */
    unsigned char ynBuffer[32];
    unsigned char bankBuffer[32];
    int offset;
    UINT32 loadAddress = 0;

    if (!fileName)
        {
        printf(mkbootFlash5360FullExplanation);
        return (ERROR);
        }

    if (!IN_RANGE (bank, (MIN_FLASH_BANK + 1), MAX_FLASH_BANK))
        {
        printf("Programming bank %d is not supported\r\n", bank);
        return (ERROR);
        }

    if (download (fileName) == ERROR)
        return (ERROR);

    if (!downloadSize) 
        return (ERROR);

    if (downloadSize > (BIOS_PROM_SIZE - (LOADER_SIZE + 1)))
        {
        printf("file size %x exceeds ROM size %x\r\n", 
               downloadSize, (BIOS_PROM_SIZE - (LOADER_SIZE + 1)));
        return (ERROR);
        }

    /* 
     * Only ROM binary images are supported.  All ROM images are
     * loaded at ROM_TEXT_ADRS.
     */

    loadAddress = ROM_TEXT_ADRS;

    if (strncmp (userDownload, "\177ELF", 4) == 0)
	{
	printf ("ERROR: ELF file, need binary file\r\n");
	return ERROR;
	}

    if (*(ULONG *)userDownload == 0x00000107)
	{
	printf ("ERROR: b_out file, need binary file\r\n");
	return ERROR;
	}

    /* display the address/count data */

    printf("\r\nSrc Start/End Address = 0x%08X/0x%08X\r\n",
	   (UINT32)userDownload, (UINT32)(userDownload + (downloadSize - 1)));
    printf("Dst Start/End Address = 0x%08X/0x%08X, Bank 0x%02X\r\n",
	   BIOS_PROM_BASE, BIOS_PROM_BASE + (downloadSize - 1), bank);
    printf("Num Bytes             = 0x%08X (&%d)\r\n",
	   downloadSize, downloadSize);

    sprintf(bankBuffer, "\r\nProgram Flash Bank 0x%02X", bank);

    if (!gevYNPrompt(bankBuffer, (unsigned char *)&ynBuffer[0], 'n')) 
        if (ynBuffer[0] != 'y') 
	        return (OK);

    offset = 0;

    if (readIdentFlash(bank) == ERROR)
        {
        printf("Read Identifier Failed\r\n");
        return (ERROR);
        }

    enableFlashWrite (bank);

    ospl = intLock ();

    for (;offset < downloadSize; offset += ROM_BANK_SIZE)
        {
        if (!((UINT)offset % SHOW_PROGRESS))
            printf ("Erasing bank %d offset 0x%08X\r", bank, offset);

        if (eraseFlashBlock(offset) == ERROR)
            {
            printf("Error: Erase failed at offset 0x%08X\r\n", offset);
            flashCtrlWrite (0); /* disable writes and select bank 0 */
            intUnlock (ospl);
            return (ERROR);
            }
        }

    /* erase the last 256 bytes */

    if (eraseFlashBlock(BIOS_PROM_SIZE - FILETYPE_OFFSET) 
        == ERROR)
        {
        printf("Erase Last Block command failed\n");
	flashCtrlWrite (0); /* disable writes and select bank 0 */
	intUnlock (ospl);
	return (ERROR);
        }

    printf("\r");

    if (programFlash(userDownload, (UCHAR *)BIOS_PROM_BASE, downloadSize) 
        == ERROR)
        {
        flashCtrlWrite (0); /* disable writes and select bank 0 */
        intUnlock (ospl);
        return (ERROR);
        }
    printf("\r");


    if (romFixup ((char *)romCopyStart, 
                  (int)romCopyEnd - (int)romCopyStart, 
                  (char *)&loadAddress) 
                  == ERROR)
        {
        flashCtrlWrite (0); /* disable writes and select bank 0 */
        intUnlock (ospl);
        return (ERROR);
        }

    intUnlock (ospl);

    flashCtrlWrite (0); /* disable writes and select bank 0 */

    taskDelay (sysClkRateGet ());

    flashCtrlWrite (bank); /* select and map bank */
    if (memcmp (userDownload, (UCHAR *)BIOS_PROM_BASE, downloadSize) != 0)
	{
	printf ("\r\nERROR: Binary comparison error\r\n");
	return ERROR;
	}
    flashCtrlWrite (0); /* disable writes and select bank 0 */

    /* don't rom boot after reloading, let BIOS reset things */

    if (sysWarmType == SYS_WARM_ROM)
	sysWarmType = SYS_WARM_BIOS;

    printf("\r\nDone                                            \r\n");

    return (OK);

    }


/***************************************************************************
*
* download - Download a file into local memory
*
* This routine downloads a file to the target and prints the returned status.
*
* RETURNS: OK if file downloaded, ERROR if file cannot be accessed.
*/

LOCAL STATUS download
    (
    char *fileName  /* filename to download */
    )
    {
    int fd;

    if (fileName == NULL)
        {
        printf("no file name"); 
        return (ERROR);
        }

    printf("Downloading %s ", fileName); 

    if (userDownload == NULL)
        userDownload = malloc (MAX_FILE_SIZE);

    if ((fd = open (fileName, O_RDONLY, 0644)) == ERROR)
        {
        printErr ("Can't open \"%s\"\n", fileName);
        return (ERROR);
        }

    printf(" ...\r\n");

    /* Read it into memory */
    
    downloadSize = fioRead (fd, (char *) userDownload, MAX_FILE_SIZE);

    close (fd);

    printf("0x%08X bytes transferred to 0x%08X.\r\n",
           downloadSize, (unsigned int)userDownload);

    return (OK);
    }
