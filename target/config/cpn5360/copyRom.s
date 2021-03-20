/* copyRom.s - ROM extraction module */

/* Copyright 2000-2002 Wind River Systems, Inc. */
/* Copyright 2000 Motorola, Inc. All Rights Reserved */

/*
modification history
--------------------
01c,09may02,dat  Update for T2.2, new FUNC labels, chg'd '/4' to '>>2'.
		 No more b.out headers in flash.
01b,30may00,scb  romFixup() bug did not copy entire jmp instruction.
01a,05jan00,dmw  written.
*/

/*
DESCRIPTION
This module contains the ROM loader routine used by the BIOS
to copy the ROM image to RAM.

*/

    .data
#define _ASMLANGUAGE
#include "vxWorks.h"
#include "sysLib.h"
#include "regs.h"
#include "asm.h"
#include "config.h"

    .globl GTEXT(romCopyStart)
    .globl GTEXT(romCopyEnd)
    .globl GTEXT(romJumpLoc)
    .globl GTEXT(copyRom)


    .text
    .balign 16

#define ROM_EXECUTION_ADDR 0xFFF80000
#define FILETYPE_OFFSET    0xFF00    /* load address programmed at 0xffffff00 */
#define TEMP_IDTR_OFFSET   0xFFBC   /* offset to _tempIdtr in this CS */
#define TEMP_GDTR_OFFSET   0xFFC2   /* offset to _tempGdtr in this CS */
#define TEMP_GDT_OFFSET    0xFFFFFFC8 /* offset to tempGdt in this CS, PE set */

/*******************************************************************************
*
* copyRom - copy ROM to DRAM.
*
* This routine copies the ROM image at ROM_EXECUTION_ADDR (offset to 
* actual code) to RAM_HIGH_ADRS or RAM_LOW_ADRS for BIOS_PROM_SIZE bytes.  
* The destination RAM address is read from FILETYPE_OFFSET from the current
* Code Section that the BIOS has placed us (F000:FF00).
* It then executes the instruction at the destination address.
* 
* This code is loaded by the BIOS at F000:0000 if the boot from flash
* option is configured.  The first instruction the BIOS will execute
* must reside at F000:FFF0.  The "jmp _copyRom" instruction is forced 
* by mkbootFlash5360() to reside at that location.
*
* RETURNS: N/A
*/

    .balign 16,0x90
FUNC_LABEL(romCopyStart)
FUNC_LABEL(copyRom)
    xor    %eax,%eax                /* clear eax */
    mov    %ax,%es                  /* clear es */

    .byte  0x67, 0x66		    /* next inst has 32bit addr/operand */
    movl   %cs:FILETYPE_OFFSET,%edi

    .byte  0x66			    /* next inst has 32bit operand */
    mov    $ROM_EXECUTION_ADDR,%esi /* source ROM */

    .byte  0x66			    /* next inst has 32bit operand */
    mov    $((BIOS_PROM_SIZE)>>2),%ecx /* length divided by 4 (copy longs) */
    cld

    .byte  0x67, 0x66		    /* next inst has 32bit addr/operand */
    rep
    es
    movsl                           /* copy src to dest for ecx longs */

    /* make sure we are in real mode */

    mov    %cr0,%eax                /* move CR0 to eax */
    .byte  0x66			    /* next inst has 32bit operand */
    and    $0xFFFFFFFE,%eax         /* clear the PE bit */
    mov    %eax,%cr0                /* move CR0 to eax */

    .byte  0x67, 0x66		    /* next inst has 32bit addr/operand */
    lidt   %cs:TEMP_IDTR_OFFSET     /* _tempIdtr */

    /* load temporary GDT */

    .byte  0x67, 0x66		    /* next inst has 32bit addr/operand */
    lgdt   %cs:TEMP_GDTR_OFFSET     /* _tempGdtr */

    /* switch to protected mode */

    mov    %cr0,%eax                /* move CR0 to EAX */
    .byte  0x66			    /* next inst has 32bit operand */
    or     $0x00000001,%eax         /* set the PE bit */
    mov    %eax,%cr0                /* move EAX to CR0 */
    jmp    flICache                 /* near jump to flush a instruction queue */

flICache:
    .byte  0x66			    /* next inst has 32bit operand */
    mov    $0x0010,%eax             /* a selector 0x10 is 3rd descriptor */
    mov    %ax,%ds                  /* load it to DS */
    mov    %ax,%es                  /* load it to ES */
    mov    %ax,%fs                  /* load it to FS */
    mov    %ax,%gs                  /* load it to GS */
    mov    %ax,%ss                  /* load it to SS */

    .byte  0x66			    /* next inst has 32bit operand */
    mov    $ROM_STACK,%esp          /* set a stack pointer */

    .byte  0x67, 0x66		    /* next inst has 32bit addr/operand */
    movl   $BOOT_COLD,4(%esp)       /* push the startType */

    .byte  0x67, 0x66		    /* next inst has 32bit addr/operand */
    movl   %cs:FILETYPE_OFFSET,%edi

    .byte  0x67, 0x66		    /* next inst has 32bit addr/operand */
    cmpl   $RAM_HIGH_ADRS,%edi

    .code32                         /* execute as 32bit code */
    jne    goLow

    .byte  0x67, 0x66		    /* next inst has 32bit addr/operand */
    ljmp   $0x08,$RAM_HIGH_ADRS + ROM_WARM_HIGH  /* far jump to warm high */

goLow:
    .byte  0x67, 0x66		    /* next inst has 32bit addr/operand */
    ljmp   $0x08,$RAM_LOW_ADRS + ROM_WARM_LOW /* far jump to warm low */


    /* data for global descriptor tables */

    .balign 16,0x90
_romIdtrLoc:
    .word    0x0000                 /* size   : 0 */
    .long    0x00000000             /* address: 0 */

_romGdtrLoc:
    .word    0x0027                 /* size   : 39(8 * 5 - 1) bytes */
    .long    TEMP_GDT_OFFSET        /* address: tempGdt */

_romGdtLoc:
    /* 0(selector=0x0000): Null descriptor */
    .word    0x0000
    .word    0x0000
    .byte    0x00
    .byte    0x00
    .byte    0x00
    .byte    0x00

    /* 1(selector=0x0008): Code descriptor */
    .word    0xffff                 /* limit: xffff */
    .word    0x0000                 /* base : xxxx0000 */
    .byte    0x00                   /* base : xx00xxxx */
    .byte    0x9a                   /* Code e/r, Present, DPL0 */
    .byte    0xcf                   /* limit: fxxxx, Page Gra, 32bit */
    .byte    0x00                   /* base : 00xxxxxx */

    /* 2(selector=0x0010): Data descriptor */
    .word    0xffff                 /* limit: xffff */
    .word    0x0000                 /* base : xxxx0000 */
    .byte    0x00                   /* base : xx00xxxx */
    .byte    0x92                   /* Data r/w, Present, DPL0 */
    .byte    0xcf                   /* limit: fxxxx, Page Gra, 32bit */
    .byte    0x00                   /* base : 00xxxxxx */

    /* 3(selector=0x0018): Code descriptor, for the nesting interrupt */
    .word    0xffff                 /* limit: xffff */
    .word    0x0000                 /* base : xxxx0000 */
    .byte    0x00                   /* base : xx00xxxx */
    .byte    0x9a                   /* Code e/r, Present, DPL0 */
    .byte    0xcf                   /* limit: fxxxx, Page Gra, 32bit */
    .byte    0x00                   /* base : 00xxxxxx */

    /* 4(selector=0x0020): Code descriptor, for the nesting interrupt */
    .word    0xffff                 /* limit: xffff */
    .word    0x0000                 /* base : xxxx0000 */
    .byte    0x00                   /* base : xx00xxxx */
    .byte    0x9a                   /* Code e/r, Present, DPL0 */
    .byte    0xcf                   /* limit: fxxxx, Page Gra, 32bit */
    .byte    0x00                   /* base : 00xxxxxx */

FUNC_LABEL(romJumpLoc)              /* Must be just ahead of "jmp copyRom" */
    .byte  0x67, 0x66		    /* next inst has 32bit addr/operand */
    jmp      FUNC(copyRom)          /* this instr gets executed by BIOS */
FUNC_LABEL(romCopyEnd)		    /* Must be just behind "jmp_ copyrom" */

    /* Shouldn't get here.  romCopyEnd label is needed for flash routine */
