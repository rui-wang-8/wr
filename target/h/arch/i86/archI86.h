/* arch68k.h - Intel 80X86 specific header */

/* Copyright 1984-2001 Wind River Systems, Inc. */
/*
modification history
--------------------
01f,13mar01,sn   SPR 73723 - define supported toolchains
01e,04oct01,hdn  enclosed ARCH_REGS_INIT macro with _ASMLANGUAGE							\
01d,21aug01,hdn  added PENTIUM4's _CACHE_ALIGN_SIZE, ARCH_REGS_INIT
01c,10aug01,hdn  added PENTIUM2/3/4 support
01b,25feb99,hdn  added _CACHE_ALIGN_SIZE for Pentium and PentiumPro
01a,07jun93,hdn  written based on arch68k.h
*/

#ifndef __INCarchI86h
#define __INCarchI86h

#ifdef __cplusplus
extern "C" {
#endif

#define _ARCH_SUPPORTS_GCC

#define	_BYTE_ORDER		_LITTLE_ENDIAN
#define	_DYNAMIC_BUS_SIZING	FALSE		/* require alignment for swap */

#if	(CPU==PENTIUM4)
#define _CACHE_ALIGN_SIZE	64
#elif	(CPU==PENTIUM || CPU==PENTIUM2 || CPU==PENTIUM3)
#define _CACHE_ALIGN_SIZE	32
#else
#define _CACHE_ALIGN_SIZE	16
#endif	/* (CPU==PENTIUM || CPU==PENTIUM[234]) */

#ifdef	_ASMLANGUAGE

/* 
 * system startup macros used by sysInit(sysALib.s) and romInit(romInnit.s).
 * - no function calls to outside of BSP is allowed.
 * - no references to the data segment is allowed.
 */

#define ARCH_REGS_INIT							\
	xorl	%eax, %eax;		/* zero EAX */			\
	movl	%eax, %dr7;		/* initialize DR7 */		\
	movl	%eax, %dr6;		/* initialize DR6 */		\
	movl	%eax, %dr3;		/* initialize DR3 */		\
	movl	%eax, %dr2;		/* initialize DR2 */		\
	movl	%eax, %dr1;		/* initialize DR1 */		\
	movl	%eax, %dr0;		/* initialize DR0 */		\
	movl	%eax, %cr4;		/* initialize CR4 */		\
	movl    %cr0, %edx;		/* get CR0 */			\
	andl    $0x7ffafff1, %edx;	/* clear PG, AM, WP, TS, EM, MP */ \
	movl    %edx, %cr0;		/* set CR0 */			\
									\
	pushl	%eax;			/* initialize EFLAGS */		\
	popfl;

#endif	/* _ASMLANGUAGE */


/* no function declarations for makeSymTlb/symTbl.c */


#ifdef __cplusplus
}
#endif

#endif /* __INCarchI86h */
