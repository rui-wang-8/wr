/* elfSparc.h - Sparc specific Extended Load Format header */

/* Copyright 1984-1996 Wind River Systems, Inc. */

/*
modification history
--------------------
01b,03apr96,ism  added EM_ARCH_MACH_ALT
01a,24Jul95,ism  created
*/

/*	Copyright (c) 1984, 1986, 1987, 1988, 1989 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef __INCelfSparcsh
#define __INCelfSparcsh

#ifdef	__cplusplus
extern "C" {
#endif

#define EM_ARCH_MACHINE EM_SPARC

#ifndef EM_ARCH_MACH_ALT
#define EM_ARCH_MACH_ALT EM_ARCH_MACHINE
#endif

/*
 *	Note entry header
 */

typedef struct {
	Elf32_Word	n_namesz;	/* length of note's name */
	Elf32_Word	n_descsz;	/* length of note's "desc" */
	Elf32_Word	n_type;		/* type of note */
} Elf32_Nhdr;

/*
 *	Known values for note entry types (e_type == ET_CORE)
 */

#define	NT_PRSTATUS	1
#define	NT_PRFPREG	2
#define	NT_PRPSINFO	3

#define	R_SPARC_NONE		0	/* relocation type */
#define	R_SPARC_8		1
#define	R_SPARC_16		2
#define	R_SPARC_32		3
#define	R_SPARC_DISP8		4
#define	R_SPARC_DISP16		5
#define	R_SPARC_DISP32		6
#define	R_SPARC_WDISP30		7
#define	R_SPARC_WDISP22		8
#define	R_SPARC_HI22		9
#define	R_SPARC_22		10
#define	R_SPARC_13		11
#define	R_SPARC_LO10		12
#define	R_SPARC_GOT10		13
#define	R_SPARC_GOT13		14
#define	R_SPARC_GOT22		15
#define	R_SPARC_PC10		16
#define	R_SPARC_PC22		17
#define	R_SPARC_WPLT30		18
#define	R_SPARC_COPY		19
#define	R_SPARC_GLOB_DAT	20
#define	R_SPARC_JMP_SLOT	21
#define	R_SPARC_RELATIVE	22
#define	R_SPARC_UA32		23
#define	R_SPARC_NUM		24	/* must be >last */

#define	ELF_SPARC_MAXPGSZ	0x10000	/* maximum page size */

#ifdef	__cplusplus
}
#endif

#endif	/* __INCelfMips */
