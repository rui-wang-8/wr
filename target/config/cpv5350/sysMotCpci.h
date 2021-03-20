/* sysMotCpci.h - Motorola CompactPCI Backpanel Devices and Board IDs */

/* Copyright 2002 Wind River Systems, Inc. */
/* Copyright 1999-2000 Motorola, Inc., All Rights Reserved */

/*
modification history
--------------------
01e,09apr02,mil  Added entry for MCPN765 with Intel21555 P2P bridge, and
                 consolidated board list.
01d,15dec99,scb  Add entries for CPN5360.
01c,30sep99,srr  Add entries for PPMC600.
01b,29sep99,srr  Add entries for MCPN765.
01a,25aug99,rhv  created.
*/

/*
This file contains:
 - The cPCI subsystem vendor and board IDs for Motorola CompactPCI boards
 - Motorola cPCI boards which participate in VxWorks shared memory.
 - Motorola cPCI boards which participate in VxWorks backpanel networking.
*/

#ifndef __INCsysMotCpcih
#define __INCsysMotCpcih

#ifdef __cplusplus
extern "C" {
#endif

/* CompactPCI subsystem vendor and board IDs for Motorola CompactPCI boards */

#define MOT_SUB_VNDR_ID_VAL      0x1057   /* Vendor = Motorola */
#define CPV3060_SUB_SYS_ID_VAL   0x4804   /* Subsystem = CPV3060 */
#define CPN5360_SUB_SYS_ID_VAL   0x4814	  /* Subsystem = CPN5360 */
#define MCPN750_SUB_SYS_ID_VAL   0x4805   /* Subsystem = MCPN750 (Sitka) */
#define MCPN765_SUB_SYS_ID_VAL   0x4811   /* Subsystem = MCPN765 */
#define PPMC750_SUB_SYS_ID_VAL   0x480e   /* Subsystem = PPMC Carrier */
#define PRPMCBASE_SUB_SYS_ID_VAL PPMC750_SUB_SYS_ID_VAL  /* PrPMC Carrier */
#define PRPMC750_SUB_SYS_ID_VAL  0x480f   /* Subsystem = PrPMC750 */
#define PPMC600_SUB_SYS_ID_VAL   0x4810   /* Subsystem = PPMC600 */
#define PRPMC600_SUB_SYS_ID_VAL  PPMC600_SUB_SYS_ID_VAL  /* PrPMC600 */
#define PRPMC800_SUB_SYS_ID_VAL  0x4817   /* Subsystem = PrPMC800 */

/*
 * The poll list should be shared among BSPs in order for inter-detection.
 * However, some BSPs may not have all the macros defined.  Thus it is
 * necessary to define them here, with the risk of inconsistency if one
 * is changed only within a BSP.
 */

#ifndef PCI_ID_BR_DEC21554
#  define PCI_ID_BR_DEC21554 0x00461011     /* ID Intel 21554 PCI bridge */
#endif

#ifndef PCI_ID_BR_DEC21555
#  define PCI_ID_BR_DEC21555 0xb5558086     /* ID Intel 21555 PCI bridge */
#endif

#ifndef PCI_ID_HARRIER
#  define PCI_ID_HARRIER 0x480B1057         /* ID Harrier PHB */
#endif

/* Motorola CompactPCI boards which participate in VxWorks shared memory. */

#define SYS_MOT_SM_ANCHOR_POLL_LIST \
    { PCI_ID_BR_DEC21554, \
      (CPV3060_SUB_SYS_ID_VAL << 16) | MOT_SUB_VNDR_ID_VAL },\
    { PCI_ID_BR_DEC21554, \
      (CPN5360_SUB_SYS_ID_VAL << 16) | MOT_SUB_VNDR_ID_VAL },\
    { PCI_ID_BR_DEC21554, \
      (MCPN750_SUB_SYS_ID_VAL << 16) | MOT_SUB_VNDR_ID_VAL },\
    { PCI_ID_BR_DEC21554, \
      (MCPN765_SUB_SYS_ID_VAL << 16) | MOT_SUB_VNDR_ID_VAL },\
    { PCI_ID_BR_DEC21555, \
      (MCPN765_SUB_SYS_ID_VAL << 16) | MOT_SUB_VNDR_ID_VAL },\
    { PCI_ID_BR_DEC21554, \
      (PPMC600_SUB_SYS_ID_VAL << 16) | MOT_SUB_VNDR_ID_VAL },\
    { PCI_ID_BR_DEC21554, \
      (PPMC750_SUB_SYS_ID_VAL << 16) | MOT_SUB_VNDR_ID_VAL },\
    { PCI_ID_BR_DEC21554, \
      (PRPMC750_SUB_SYS_ID_VAL << 16) | MOT_SUB_VNDR_ID_VAL },\
    { PCI_ID_BR_DEC21554, \
      (PRPMC800_SUB_SYS_ID_VAL << 16) | MOT_SUB_VNDR_ID_VAL },\
    { PCI_ID_HARRIER, \
      (PRPMC800_SUB_SYS_ID_VAL << 16) | MOT_SUB_VNDR_ID_VAL },

/*
 * The following list is for the CompactPCI Host to use when connecting
 * interrupt handlers to support each DEC2155x device interrupting on the
 * CompactPCI backpanel when using shared memory.
 */

#define SYS_MOT_SM_DEVICE_LIST \
    { PCI_ID_BR_DEC21554, \
      (CPV3060_SUB_SYS_ID_VAL << 16) | MOT_SUB_VNDR_ID_VAL },\
    { PCI_ID_BR_DEC21554, \
      (CPN5360_SUB_SYS_ID_VAL << 16) | MOT_SUB_VNDR_ID_VAL },\
    { PCI_ID_BR_DEC21554, \
      (MCPN750_SUB_SYS_ID_VAL << 16) | MOT_SUB_VNDR_ID_VAL },\
    { PCI_ID_BR_DEC21554, \
      (MCPN765_SUB_SYS_ID_VAL << 16) | MOT_SUB_VNDR_ID_VAL },\
    { PCI_ID_BR_DEC21555, \
      (MCPN765_SUB_SYS_ID_VAL << 16) | MOT_SUB_VNDR_ID_VAL },\
    { PCI_ID_BR_DEC21554, \
      (PPMC600_SUB_SYS_ID_VAL << 16) | MOT_SUB_VNDR_ID_VAL },\
    { PCI_ID_BR_DEC21554, \
      (PPMC750_SUB_SYS_ID_VAL << 16) | MOT_SUB_VNDR_ID_VAL },\
    { PCI_ID_BR_DEC21554, \
      (PRPMC750_SUB_SYS_ID_VAL << 16) | MOT_SUB_VNDR_ID_VAL },\
    { PCI_ID_BR_DEC21554, \
      (PRPMC800_SUB_SYS_ID_VAL << 16) | MOT_SUB_VNDR_ID_VAL },\
    { PCI_ID_HARRIER, \
      (PRPMC800_SUB_SYS_ID_VAL << 16) | MOT_SUB_VNDR_ID_VAL },

#ifdef __cplusplus
}
#endif

#endif	/* __INCsysMotCpcih */
