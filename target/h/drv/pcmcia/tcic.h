/* tcic.h - Databook TCIC/2 PCMCIA Host Bus Adaptor Chip header */

/* Copyright 1984-1996 Wind River Systems, Inc. */
/* Copyright (c) 1994 David A. Hinds -- All Rights Reserved */

/*
modification history
--------------------
01b,22feb96,hdn  cleaned up.
01a,03oct95,hdn  written based on David Hinds's version 2.2.3.
*/

#ifndef __INCtcich
#define __INCtcich

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _ASMLANGUAGE


#define TCIC_MAX_SOCKS		2	/* number of sockets */
#define TCIC_MEM_WINDOWS	4	/* number of memory windows */
#define TCIC_IO_WINDOWS		2	/* number of io windows */

/* offsets of registers from TCIC_BASE */

#define TCIC_DATA		0x00
#define TCIC_ADDR		0x02
#define TCIC_SCTRL		0x06
#define TCIC_SSTAT		0x07
#define TCIC_MODE		0x08
#define TCIC_PWR		0x09
#define TCIC_EDC		0x0A
#define TCIC_ICSR		0x0C
#define TCIC_IENA		0x0D
#define TCIC_AUX		0x0E

#define TCIC_SS_SHFT		12
#define TCIC_SS_MASK		0x7000

/* Flags for TCIC_ADDR */

#define TCIC_ADR2_REG		0x8000
#define TCIC_ADR2_INDREG	0x0800

#define TCIC_ADDR_REG		0x80000000
#define TCIC_ADDR_SS_SHFT	(TCIC_SS_SHFT+16)
#define TCIC_ADDR_SS_MASK	(TCIC_SS_MASK<<16)
#define TCIC_ADDR_INDREG	0x08000000
#define TCIC_ADDR_IO		0x04000000
#define TCIC_ADDR_MASK		0x03ffffff

/* Flags for TCIC_SCTRL */

#define TCIC_SCTRL_ENA		0x01
#define TCIC_SCTRL_INCMODE	0x18
#define TCIC_SCTRL_INCMODE_HOLD	0x00
#define TCIC_SCTRL_INCMODE_WORD	0x08
#define TCIC_SCTRL_INCMODE_REG	0x10
#define TCIC_SCTRL_INCMODE_AUTO	0x18
#define TCIC_SCTRL_EDCSUM	0x20
#define TCIC_SCTRL_RESET	0x80

/* Flags for TCIC_SSTAT */

#define TCIC_SSTAT_6US		0x01
#define TCIC_SSTAT_10US		0x02
#define TCIC_SSTAT_PROGTIME	0x04
#define TCIC_SSTAT_LBAT1	0x08
#define TCIC_SSTAT_LBAT2	0x10
#define TCIC_SSTAT_RDY		0x20	/* Inverted */
#define TCIC_SSTAT_WP		0x40
#define TCIC_SSTAT_CD		0x80	/* Card detect */

/* Flags for TCIC_MODE */

#define TCIC_MODE_PGMMASK	0x1f
#define TCIC_MODE_NORMAL	0x00
#define TCIC_MODE_PGMWR		0x01
#define TCIC_MODE_PGMRD		0x02
#define TCIC_MODE_PGMCE		0x04
#define TCIC_MODE_PGMDBW	0x08
#define TCIC_MODE_PGMWORD	0x10
#define TCIC_MODE_AUXSEL_MASK	0xe0

/* Registers accessed through TCIC_AUX, by setting TCIC_MODE */

#define TCIC_AUX_TCTL		(0<<5)
#define TCIC_AUX_PCTL		(1<<5)
#define TCIC_AUX_WCTL		(2<<5)
#define TCIC_AUX_EXTERN		(3<<5)
#define TCIC_AUX_PDATA		(4<<5)
#define TCIC_AUX_SYSCFG		(5<<5)
#define TCIC_AUX_ILOCK		(6<<5)
#define TCIC_AUX_TEST		(7<<5)

/* Flags for TCIC_PWR */

#define TCIC_PWR_VCC(sock)	(0x01<<(sock))
#define TCIC_PWR_VCC_MASK	0x03
#define TCIC_PWR_VPP(sock)	(0x08<<(sock))
#define TCIC_PWR_VPP_MASK	0x18
#define TCIC_PWR_CLIMENA	0x40
#define TCIC_PWR_CLIMSTAT	0x80

/* Flags for TCIC_ICSR */

#define TCIC_ICSR_CLEAR		0x01
#define TCIC_ICSR_SET		0x02
#define TCIC_ICSR_STOPCPU	0x04
#define TCIC_ICSR_ILOCK		0x08
#define TCIC_ICSR_PROGTIME	0x10
#define TCIC_ICSR_ERR		0x20
#define TCIC_ICSR_CDCHG		0x40
#define TCIC_ICSR_IOCHK		0x80

/* Flags for TCIC_IENA */

#define TCIC_IENA_CFG_MASK	0x03
#define TCIC_IENA_CFG_OFF	0x00	/* disabled */
#define TCIC_IENA_CFG_OD	0x01	/* active low, open drain */
#define TCIC_IENA_CFG_LOW	0x02	/* active low, totem pole */
#define TCIC_IENA_CFG_HIGH	0x03	/* active high, totem pole */
#define TCIC_IENA_ILOCK		0x08
#define TCIC_IENA_PROGTIME	0x10
#define TCIC_IENA_ERR		0x20	/* overcurrent or iochk */
#define TCIC_IENA_CDCHG		0x40

/* Flags for TCIC_AUX_WCTL */

#define TCIC_WAIT_COUNT_MASK	0x001f
#define TCIC_WAIT_ASYNC		0x0020
#define TCIC_WAIT_SENSE		0x0040
#define TCIC_WAIT_SRC		0x0080
#define TCIC_WCTL_WR		0x0100
#define TCIC_WCTL_RD		0x0200
#define TCIC_WCTL_CE		0x0400
#define TCIC_WCTL_LLBAT1	0x0800
#define TCIC_WCTL_LLBAT2	0x1000
#define TCIC_WCTL_LRDY		0x2000
#define TCIC_WCTL_LWP		0x4000
#define TCIC_WCTL_LCD		0x8000

/* Flags for TCIC_AUX_SYSCFG */

#define TCIC_SYSCFG_IRQ_MASK	0x000f
#define TCIC_SYSCFG_MCSFULL	0x0010
#define TCIC_SYSCFG_IO1723	0x0020
#define TCIC_SYSCFG_MCSXB	0x0040
#define TCIC_SYSCFG_ICSXB	0x0080
#define TCIC_SYSCFG_NOPDN	0x0100
#define TCIC_SYSCFG_MPSEL_SHFT	9
#define TCIC_SYSCFG_MPSEL_MASK	0x0e00
#define TCIC_SYSCFG_MPSENSE	0x2000
#define TCIC_SYSCFG_AUTOBUSY	0x4000
#define TCIC_SYSCFG_ACC		0x8000

#define TCIC_ILOCK_OUT		0x01
#define TCIC_ILOCK_SENSE	0x02
#define TCIC_ILOCK_CRESET	0x04
#define TCIC_ILOCK_CRESENA	0x08
#define TCIC_ILOCK_CWAIT	0x10
#define TCIC_ILOCK_CWAITSNS	0x20
#define TCIC_ILOCK_HOLD_MASK	0xc0
#define TCIC_ILOCK_HOLD_CCLK	0xc0

#define TCIC_ILOCKTEST_ID_SH	8
#define TCIC_ILOCKTEST_ID_MASK	0x7f00
#define TCIC_ILOCKTEST_MCIC_1	0x8000

#define TCIC_TEST_DIAG		0x8000

/* Indirectly addressed registers */

#define TCIC_SCF1(sock)	((sock)<<3)
#define TCIC_SCF2(sock) (((sock)<<3)+2)

/* Flags for SCF1 */

#define TCIC_SCF1_IRQ_MASK	0x000f
#define TCIC_SCF1_IRQ_OFF	0x0000
#define TCIC_SCF1_IRQOC		0x0010
#define TCIC_SCF1_PCVT		0x0020
#define TCIC_SCF1_IRDY		0x0040
#define TCIC_SCF1_ATA		0x0080
#define TCIC_SCF1_DMA_SHIFT	8
#define TCIC_SCF1_DMA_OFF	0
#define TCIC_SCF1_DREQ2		2
#define TCIC_SCF1_IOSTS		0x0800
#define TCIC_SCF1_SPKR		0x1000
#define TCIC_SCF1_FINPACK	0x2000
#define TCIC_SCF1_DELWR		0x4000
#define TCIC_SCF1_HD7IDE	0x8000

/* Flags for SCF2 */

#define TCIC_SCF2_RI		0x0001
#define TCIC_SCF2_IDBR		0x0002
#define TCIC_SCF2_MDBR		0x0004
#define TCIC_SCF2_MLBAT1	0x0008
#define TCIC_SCF2_MLBAT2	0x0010
#define TCIC_SCF2_MRDY		0x0020
#define TCIC_SCF2_MWP		0x0040
#define TCIC_SCF2_MCD		0x0080

/* Indirect addresses for memory window registers */

#define TCIC_MWIN(sock,map)	(0x100+(((map)+((sock)<<2))<<3))
#define TCIC_MBASE_X		2
#define TCIC_MMAP_X		4
#define TCIC_MCTL_X		6

#define TCIC_MBASE_4K_BIT	0x4000
#define TCIC_MBASE_HA_SHFT	12
#define TCIC_MBASE_HA_MASK	0x0fff

#define TCIC_MMAP_REG		0x8000
#define TCIC_MMAP_CA_SHFT	12
#define TCIC_MMAP_CA_MASK	0x3fff

#define TCIC_MCTL_WSCNT_MASK	0x001f
#define TCIC_MCTL_WCLK		0x0020
#define TCIC_MCTL_WCLK_CCLK	0x0000
#define TCIC_MCTL_WCLK_BCLK	0x0020
#define TCIC_MCTL_QUIET		0x0040
#define TCIC_MCTL_WP		0x0080
#define TCIC_MCTL_ACC		0x0100
#define TCIC_MCTL_KE		0x0200
#define TCIC_MCTL_EDC		0x0400
#define TCIC_MCTL_B8		0x0800
#define TCIC_MCTL_SS_SHFT	TCIC_SS_SHFT
#define TCIC_MCTL_SS_MASK	TCIC_SS_MASK
#define TCIC_MCTL_ENA		0x8000

/* Indirect addresses for I/O window registers */

#define TCIC_IWIN(sock,map)	(0x200+(((map)+((sock)<<1))<<2))
#define TCIC_IBASE_X		0
#define TCIC_ICTL_X		2

#define TCIC_ICTL_WSCNT_MASK	TCIC_MCTL_WSCNT_MASK
#define TCIC_ICTL_QUIET		TCIC_MCTL_QUIET
#define TCIC_ICTL_1K		0x0080
#define TCIC_ICTL_PASS16	0x0100
#define TCIC_ICTL_ACC		TCIC_MCTL_ACC
#define TCIC_ICTL_TINY		0x0200
#define TCIC_ICTL_B16		0x0400
#define TCIC_ICTL_B8		TCIC_MCTL_B8
#define TCIC_ICTL_BW_MASK	(TCIC_ICTL_B16|TCIC_ICTL_B8)
#define TCIC_ICTL_BW_DYN	0
#define TCIC_ICTL_BW_8		TCIC_ICTL_B8
#define TCIC_ICTL_BW_16		TCIC_ICTL_B16
#define TCIC_ICTL_BW_ATA	(TCIC_ICTL_B16|TCIC_ICTL_B8)
#define TCIC_ICTL_SS_SHFT	TCIC_SS_SHFT
#define TCIC_ICTL_SS_MASK	TCIC_SS_MASK
#define TCIC_ICTL_ENA		TCIC_MCTL_ENA


/* function declarations */

#if defined(__STDC__) || defined(__cplusplus)

extern int  tcicInit	(int ioBase, int intVec, int intLevel,
			 FUNCPTR showRtn);
extern void tcicShow	(int sock);


#else

extern int  tcicInit	();
extern void tcicShow	();

#endif  /* __STDC__ */

#endif	/* _ASMLANGUAGE */

#ifdef __cplusplus
}
#endif

#endif	/* __INCtcich */
