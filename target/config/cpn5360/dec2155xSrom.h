/* dec2155xSrom.h - DEC 2155X PCI-to-PCI bridge SROM library */

/* Copyright 1998-1999 Motorola, Inc. All Rights Reserved */

/*
modification history
--------------------
01c,24mar00,scb  WRS coding standards fixes.
01b,14nov99,scb  Add SROM offsets and values.
01a,16nov99,dmw  Written based on dec2155x.c from PPC6Bug base.
*/

/*
DESCRIPTION

This file contains macros for Reading and Writing the dec2155x's SROM.

*/

#ifndef __INCdec2155xSromh
#define __INCdec2155xSromh

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ROM address register definitions
 */

#define	DEC2155X_SROM_EWDS	0x000		/* Disable writes */
#define	DEC2155X_SROM_WRAL	0x080		/* Write all */
#define	DEC2155X_SROM_ERAL	0x100		/* Erase all */
#define	DEC2155X_SROM_EWEN	0x180		/* Enable writes */
#define	DEC2155X_SROM_READ	0x400		/* Read 1 byte */
#define	DEC2155X_SROM_WRITE	0x200		/* Write 1 byte */
#define	DEC2155X_SROM_ERASE	0x600		/* Erase 1 byte */

/*
 * ROM control register definitions
 */

#define	DEC2155X_SROM_START	0x01		/* Start/Busy bit */
#define	DEC2155X_SROM_POLL	0x08		/* Completion flag */

/*
 * General SROM definitions
 */

#define	DEC2155X_SROM_SIZE	512	      /* Size of DEC attached SROM */
#define	DEC2155X_SROM_CMDTIMEOUT 10	      /* Command initiate timeout */
#define	DEC2155X_SROM_POLLTIMEOUT 60	      /* Command completion timeout */

/* Dec21554 SROM offsets */

#define DEC2155X_SROM_SER_PRELOAD		0x00

#define DEC2155X_SROM_PRI_PROGRAMMING_IF	0x04
#define DEC2155X_SROM_PRI_SUBCLASS		0x05
#define DEC2155X_SROM_PRI_CLASS			0x06

#define DEC2155X_SROM_SUB_VENDER_ID		0x07
#define DEC2155X_SROM_SUB_SYSTEM_ID		0x09

#define DEC2155X_SROM_PRI_MIN_GRANT		0x0b
#define DEC2155X_SROM_PRI_MAX_LATENCY		0x0c

#define DEC2155X_SROM_SEC_PROGRAMMING_IF	0x0d
#define DEC2155X_SROM_SEC_SUBCLASS		0x0e
#define DEC2155X_SROM_SEC_CLASS			0x0f

#define DEC2155X_SROM_SEC_MIN_GRANT		0x10
#define DEC2155X_SROM_SEC_MAX_LATENCY		0x11

#define DEC2155X_SROM_DS_MEM0_SETUP		0x12
#define DEC2155X_SROM_DS_IO_OR_MEM1_SETUP	0x16
#define DEC2155X_SROM_DS_MEM2_SETUP		0x1a
#define DEC2155X_SROM_DS_MEM3_SETUP		0x1e
#define DEC2155X_SROM_UPR32_BITS_DS_MEM3_SU	0x22

#define DEC2155X_SROM_PRI_EXP_ROM_SETUP		0x26

#define DEC2155X_SROM_US_IO_OR_MEM0_SETUP	0x28
#define DEC2155X_SROM_US_MEM1_SETUP		0x2c

#define DEC2155X_SROM_CHP_CTRL0			0x30
#define DEC2155X_SROM_CHP_CTRL1			0x32

#define DEC2155X_SROM_ARB_CTRL			0x34

#define DEC2155X_SROM_PRI_SERR_DISABLES 	0x36
#define DEC2155X_SROM_SEC_SERR_DISABLES 	0x37

#define DEC2155X_SROM_PWR_MGMT_0		0x38
#define DEC2155X_SROM_PWR_MGMT_1		0x39
#define DEC2155X_SROM_PWR_MGMT_2		0x3a
#define DEC2155X_SROM_PWR_MGMT_3		0x3b
#define DEC2155X_SROM_PWR_MGMT_4		0x3c
#define DEC2155X_SROM_PWR_MGMT_5		0x3d
#define DEC2155X_SROM_PWR_MGMT_6		0x3e
#define DEC2155X_SROM_PWR_MGMT_7		0x3f

#define DEC2155X_SROM_PCI_HS_ECPID		0x40

#define DEC2155X_SROM_BIST			0x41

#define DEC2155X_SROM_PWR_MGMT_CR		0x42

/* Dec21554 SROM values to load, not defined in "config.h" */

#define DEC2155X_SROM_SER_PRELOAD_ENABLE_VAL	0x80
#define DEC2155X_SROM_PRI_EXP_ROM_SETUP_VAL	0x0000

#define DEC2155X_SROM_PWR_MGMT_0_VAL		0x00
#define DEC2155X_SROM_PWR_MGMT_1_VAL		0x01
#define DEC2155X_SROM_PWR_MGMT_2_VAL		0x02
#define DEC2155X_SROM_PWR_MGMT_3_VAL		0x03
#define DEC2155X_SROM_PWR_MGMT_4_VAL		0x04
#define DEC2155X_SROM_PWR_MGMT_5_VAL		0x05
#define DEC2155X_SROM_PWR_MGMT_6_VAL		0x06
#define DEC2155X_SROM_PWR_MGMT_7_VAL		0x07

#define DEC2155X_SROM_PWR_MGMT_CR_VAL		0x42


#ifdef __cplusplus
}
#endif

#endif  /* __INCdec2155xSromh */
