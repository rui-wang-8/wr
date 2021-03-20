/* nvr4121.h - NEC NVR4121 header file */

/* Copyright 1984-1997 Wind River Systems, Inc. */
/*
modification history
--------------------
01c,14oct99,jmw  fix RTC reg VR4121_ECMPHREG address
01b,07oct99,jmw  add vr4102 RTC and new BCU registers
01a,23aug99,jmw  created from nvr4102.h for vr4121 bsp

*/

/*
This file contains constants for the NEC V4R4101.  Register address
definitions for the various subsystems are provided, and some (but
not all) register field definitions are provided.
*/

#ifndef __INCnvr4121h
#define __INCnvr4121h

#include "vxWorks.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VR4121_ICACHE_SIZE	16384
#define VR4121_DCACHE_SIZE	8192


#define VR4121_ISA_IO_BASE_ADRS PHYS_TO_K1(0x15000000)   /* vr4121 SDB    */

/* interrupt bits in the status register */

#define VR4121_SR_ICU_INTERVAL_TIMER  (1 << 11)
#define VR4121_SR_ICU_OTHER	      (1 << 10)

/* VR4121 register definitions. */

#define VR4121_REG_BASE   (0x0b000000 | K1BASE)

#ifdef	_ASMLANGUAGE
#define VR4121_ADRS(reg)   (VR4121_REG_BASE + (reg))
#else
#define VR4121_ADRS(reg)   ((volatile UINT16 *)(VR4121_REG_BASE + (reg)))
#endif	/* _ASMLANGUAGE */


/* BCU registers */

#define VR4121_BCUCNTREG1	VR4121_ADRS(0x00)
#define VR4121_BCUCNTREG2	VR4121_ADRS(0x02)
#define VR4121_ROMSIZEREG	VR4121_ADRS(0x04)  /* new */
#define VR4121_RAMSIZEREG	VR4121_ADRS(0x06)  /* new */
#define VR4121_BCUSPEEDREG	VR4121_ADRS(0x0a)
#define VR4121_BCUERRSTREG	VR4121_ADRS(0x0c)
#define VR4121_BCURFCNTREG	VR4121_ADRS(0x0e)
#define VR4121_REVIDREG		VR4121_ADRS(0x10)
#define VR4121_BCURCOUNTRE	VR4121_ADRS(0x12)
#define VR4121_CLKSPEEDREG	VR4121_ADRS(0x14)
#define VR4121_BCUCNTREG3	VR4121_ADRS(0x16)  /* new */
#define VR4121_SDRAMMODEREG	VR4121_ADRS(0x1a)  /* new */
#define VR4121_SROMMODEREG	VR4121_ADRS(0x1c)  /* new */
#define VR4121_SDRAMCNTREG	VR4121_ADRS(0x1e)  /* new */


/* BCUCNTREG1 bit definitions */

#define VR4121_ROM64		(1 << 15)
#define VR4121_DRAM64		(1 << 14)
#define VR4121_ISAM_LCD		(1 << 13)	/* XXX set this? */
#define VR4121_PAGE128		(1 << 12)
#define VR4121_PAGEROM2		(1 << 10)
#define VR4121_PAGEROM0		(1 << 8)
#define VR4121_ROMWEN2		(1 << 6)
#define VR4121_ROMWEN0		(1 << 4)
#define VR4121_BUSHERREN	(1 << 2)
#define VR4121_RSTOUT		(1 << 0)

/* BCUCNTREG2 bit definitions */

#define VR4121_GMODE		(1 << 0)

/* DMAAU registers */

#define VR4121_AIUBALREG	VR4121_ADRS(0x20)
#define VR4121_AIUBAHREG	VR4121_ADRS(0x22)
#define VR4121_AIUALREG		VR4121_ADRS(0x24)
#define VR4121_AIUAHREG		VR4121_ADRS(0x26)
#define VR4121_AIUOBALREG	VR4121_ADRS(0x28)
#define VR4121_AIUOBAHREG	VR4121_ADRS(0x2a)
#define VR4121_AIUOALREG	VR4121_ADRS(0x2c)
#define VR4121_AIUOAHREG	VR4121_ADRS(0x2e)
#define VR4121_FIRBALREG	VR4121_ADRS(0x30)
#define VR4121_FIRBAHREG	VR4121_ADRS(0x32)
#define VR4121_FIRALREG		VR4121_ADRS(0x34)
#define VR4121_FIRAHREG		VR4121_ADRS(0x36)

/* DCU registers */

#define VR4121_DMARSTREG	VR4121_ADRS(0x40)
#define VR4121_DMAIDLEREG	VR4121_ADRS(0x42)
#define VR4121_DMASENREG	VR4121_ADRS(0x44)
#define VR4121_DMAMSKREG	VR4121_ADRS(0x46)
#define VR4121_DMAREQREG	VR4121_ADRS(0x48)
#define VR4121_TDREG		VR4121_ADRS(0x4a)


/* DMA mask bit definitions */

#define  VR4121_DMAMSKAIN	(1 << 3)
#define  VR4121_DMAMSKAOUT	(1 << 2)
#define  VR4121_DMAMSKFOUT	(1 << 0)

/* CMU register */

#define VR4121_CMUCLKMSK	VR4121_ADRS(0x60)
#define  VR4121_MSKFFIR		(1 << 10)
#define  VR4121_MSKSHSP		(1 << 9)
#define  VR4121_MSKSSIU		(1 << 8)  /* XXX set this */
#define  VR4121_MSKDSIU		(1 << 5)  /* XXX set this */
#define  VR4121_MSKFIR		(1 << 4)
#define  VR4121_MSKKIU		(1 << 3)
#define  VR4121_MSKADU		(1 << 2)
#define  VR4121_MSKSIU		(1 << 1)  /* XXX set this */
#define  VR4121_MSKPIU		(1 << 0)

/* ICU system and system mask registers */

#define VR4121_ICU_SYSINT1REG  	VR4121_ADRS(0x80)
#define VR4121_ICU_MSYSINT1REG	VR4121_ADRS(0x8c)
#define  VR4121_DOZEPIUINTR	(1 << 13)
#define  VR4121_SOFTINTR	(1 << 11)
#define  VR4121_WRBERRINTR	(1 << 10)
#define  VR4121_SIUINTR		(1 << 9)
#define  VR4121_GIUINTR		(1 << 8)
#define  VR4121_KIUINTR		(1 << 7)
#define  VR4121_AIUINTR		(1 << 6)
#define  VR4121_PIUINTR		(1 << 5)
#define  VR4121_ETIMERINTR	(1 << 3)
#define  VR4121_RTCL1INTR	(1 << 2)
#define  VR4121_POWERINTR	(1 << 1)
#define  VR4121_BATINTR		(1 << 0)
#define  VR4121_ZEROINTR	 0

#define VR4121_ICU_SYSINT2REG  	VR4121_ADRS(0x200)
#define VR4121_ICU_MSYSINT2REG	VR4121_ADRS(0x206)
#define  VR4121_DSIUINTR	(1 << 5)
#define  VR4121_FIRINTR		(1 << 4)
#define  VR4121_TCLKINTR	(1 << 3)
#define  VR4121_HSPINTR		(1 << 2)
#define  VR4121_LEDINTR		(1 << 1)
#define  VR4121_RTCL2INTR	(1 << 0)

/* ICU subsystem status and mask registers */

#define VR4121_ICU_PIUINTREG	VR4121_ADRS(0x82)
#define VR4121_ICU_ADUINTREG	VR4121_ADRS(0x84)
#define VR4121_ICU_KIUINTREG	VR4121_ADRS(0x86)
#define VR4121_ICU_GIUINTLREG	VR4121_ADRS(0x88)
#define VR4121_ICU_DSIUINTREG	VR4121_ADRS(0x8a)
#define VR4121_ICU_MPIUINTREG	VR4121_ADRS(0x8e)
#define VR4121_ICU_MAIUINTREG	VR4121_ADRS(0x90)
#define VR4121_ICU_MKIUINTREG	VR4121_ADRS(0x92)
#define VR4121_ICU_MGIUINTLREG	VR4121_ADRS(0x94)
#define VR4121_ICU_MDSIUINTREG	VR4121_ADRS(0x96)
#define VR4121_ICU_NMIREG	VR4121_ADRS(0x98)
#define VR4121_ICU_SOFTINTREG	VR4121_ADRS(0x9a)
#define VR4121_ICU_GIUINTHREG   VR4121_ADRS(0x202)
#define VR4121_ICU_FIRINTHREG   VR4121_ADRS(0x204)
#define VR4121_ICU_MGIUINTHREG  VR4121_ADRS(0x208)
#define VR4121_ICU_MFIRINTHREG  VR4121_ADRS(0x20a)

/* ICU MDSIUINTREG bit definitions */

#define  VR4121_ICU_DSIU_INTSR0	   (1 << 9)
#define  VR4121_ICU_DSIU_INTST0	   (1 << 8)

/* PMU registers */

#define VR4121_PMUINTREG	VR4121_ADRS(0xa0)  /* verify this adr XXX */
#define VR4121_PMUCNTREG	VR4121_ADRS(0xa2)  /* verify this adr XXX */
#define VR4121_HALTIMERRST	(1 << 2)

/* RTC registers */

#define VR4121_ETIMELREG	VR4121_ADRS(0xc0)
#define VR4121_ETIMEMREG	VR4121_ADRS(0xc2)
#define VR4121_ETIMEHREG	VR4121_ADRS(0xc4)
#define VR4121_ECMPLREG		VR4121_ADRS(0xc8)
#define VR4121_ECMPMREG		VR4121_ADRS(0xca)
#define VR4121_ECMPHREG		VR4121_ADRS(0xcc)  /* modified XXX was 0xce */
#define VR4121_RTCL1LREG	VR4121_ADRS(0xd0)
#define VR4121_RTCL1HREG	VR4121_ADRS(0xd2)
#define VR4121_RTCL1CNTLREG	VR4121_ADRS(0xd4)
#define VR4121_RTCL1CNTHREG	VR4121_ADRS(0xd6)
#define VR4121_RTCL2LREG	VR4121_ADRS(0xd8)
#define VR4121_RTCL2HREG	VR4121_ADRS(0xda)
#define VR4121_RTCL2CNTLREG	VR4121_ADRS(0xdc)
#define VR4121_RTCL2CNTHREG	VR4121_ADRS(0xde)
#define VR4121_TCLKLREG		VR4121_ADRS(0x1c0)
#define VR4121_TCLKHREG		VR4121_ADRS(0x1c2)
#define VR4121_TCLKCNTLREG	VR4121_ADRS(0x1c4)
#define VR4121_TCLKCNTHREG	VR4121_ADRS(0x1c6)
#define VR4121_RTCINTREG	VR4121_ADRS(0x1de)
#define  VR4121_RTC_RTCINTR0	(1 << 0)
#define  VR4121_RTC_RTCINTR1	(1 << 1)
#define  VR4121_RTC_RTCINTR2	(1 << 2)
#define  VR4121_RTC_RTCINTR3	(1 << 3)

/* define VR4102 RTC registers to use nvr4102RTCTimer.c */

#define VR4102_ETIMELREG	VR4121_ETIMELREG
#define VR4102_ETIMEMRE		VR4121_ETIMEMRE
#define VR4102_ETIMEHREG	VR4121_ETIMEHREG
#define VR4102_ECMPLREG 	VR4121_ECMPLREG
#define VR4102_ECMPMREG 	VR4121_ECMPMREG
#define VR4102_ECMPHREG 	VR4121_ECMPHREG
#define VR4102_RTCL1LREG	VR4121_RTCL1LREG
#define VR4102_RTCL1HREG	VR4121_RTCL1HREG
#define VR4102_RTCL1CNTLREG	VR4121_RTCL1CNTLREG
#define VR4102_RTCL1CNTHREG	VR4121_RTCL1CNTHREG
#define VR4102_RTCL2LREG	VR4121_RTCL2LREG
#define VR4102_RTCL2HREG	VR4121_RTCL2HREG
#define VR4102_RTCL2CNTLREG	VR4121_RTCL2CNTLREG
#define VR4102_RTCL2CNTHREG	VR4121_RTCL2CNTHREG
#define VR4102_TCLKLREG		VR4121_TCLKLREG
#define VR4102_TCLKHREG		VR4121_TCLKHREG
#define VR4102_TCLKCNTLREG	VR4121_TCLKCNTLREG
#define VR4102_TCLKCNTHREG	VR4121_TCLKCNTHREG
#define VR4102_RTCINTREG  	VR4121_RTCINTREG
#define  VR4102_RTC_RTCINTR0	VR4121_RTC_RTCINTR0
#define  VR4102_RTC_RTCINTR1	VR4121_RTC_RTCINTR1
#define  VR4102_RTC_RTCINTR2	VR4121_RTC_RTCINTR2
#define  VR4102_RTC_RTCINTR3	VR4121_RTC_RTCINTR3

#define  VR4102_RTCL1INTR	VR4121_RTCL1INTR
#define  VR4102_RTCL2INTR	VR4121_RTCL2INTR

#define VR4102_ICU_MSYSINT1REG	VR4121_ICU_MSYSINT1REG

/* DSU registers */

#define VR4121_DSUCNTREG	VR4121_ADRS(0xe0)
#define VR4121_DSUSETREG	VR4121_ADRS(0xe2)
#define VR4121_DSUCLRREG	VR4121_ADRS(0xe4)
#define VR4121_DSUTIMREG	VR4121_ADRS(0xe6)
#define VR4121_DSULOADREG	VR4121_ADRS(0xe8)

/* GIU registers */

#define VR4121_GIUIOSELL	VR4121_ADRS(0x100)
#define VR4121_GIUIOSELH	VR4121_ADRS(0x102)
#define VR4121_GIUPIODL		VR4121_ADRS(0x104)
#define VR4121_GIUPIODH		VR4121_ADRS(0x106)
#define VR4121_GIUINTSTATL	VR4121_ADRS(0x108)
#define VR4121_GIUINTSTATH	VR4121_ADRS(0x10a)
#define VR4121_GIUINTENL	VR4121_ADRS(0x10c)
#define VR4121_GIUINTENH	VR4121_ADRS(0x10e)
#define VR4121_GIUINTTYPL	VR4121_ADRS(0x110)
#define VR4121_GIUINTTYPH	VR4121_ADRS(0x112)
#define VR4121_GIUINTALSELL	VR4121_ADRS(0x114)
#define VR4121_GIUINTALSELH	VR4121_ADRS(0x116)
#define VR4121_GIUINTHTSELL	VR4121_ADRS(0x118)
#define VR4121_GIUINTHTSELH	VR4121_ADRS(0x11a)
#define VR4121_GIUPODATL	VR4121_ADRS(0x11c)
#define VR4121_GIUPODATH	VR4121_ADRS(0x11e)

/* The general-purpose I/O pins (GPIO) are enabled and controlled
   via identically placed bits in the GIU registers and some of the
   ICU registers. This set of pin masks can be used with whichever
   registers contain GPIO pin configuration. */

#define  VR4121_GPIO_PIN_31	(1 << 15)
#define  VR4121_GPIO_PIN_30	(1 << 14)
#define  VR4121_GPIO_PIN_29	(1 << 13)
#define  VR4121_GPIO_PIN_28	(1 << 12)
#define  VR4121_GPIO_PIN_27	(1 << 11)
#define  VR4121_GPIO_PIN_26	(1 << 10)
#define  VR4121_GPIO_PIN_25	(1 << 9)
#define  VR4121_GPIO_PIN_24	(1 << 8)
#define  VR4121_GPIO_PIN_23	(1 << 7)
#define  VR4121_GPIO_PIN_22	(1 << 6)
#define  VR4121_GPIO_PIN_21	(1 << 5)
#define  VR4121_GPIO_PIN_20	(1 << 4)
#define  VR4121_GPIO_PIN_19	(1 << 3)
#define  VR4121_GPIO_PIN_18	(1 << 2)
#define  VR4121_GPIO_PIN_17	(1 << 1)
#define  VR4121_GPIO_PIN_16	(1 << 0)
#define  VR4121_GPIO_PIN_15	(1 << 15)
#define  VR4121_GPIO_PIN_14	(1 << 14)
#define  VR4121_GPIO_PIN_13	(1 << 13)
#define  VR4121_GPIO_PIN_12	(1 << 12)
#define  VR4121_GPIO_PIN_11	(1 << 11)
#define  VR4121_GPIO_PIN_10	(1 << 10)
#define  VR4121_GPIO_PIN_9	(1 << 9)
#define  VR4121_GPIO_PIN_8	(1 << 8)
#define  VR4121_GPIO_PIN_7	(1 << 7)
#define  VR4121_GPIO_PIN_6	(1 << 6)
#define  VR4121_GPIO_PIN_5	(1 << 5)
#define  VR4121_GPIO_PIN_4	(1 << 4)
#define  VR4121_GPIO_PIN_3	(1 << 3)
#define  VR4121_GPIO_PIN_2	(1 << 2)
#define  VR4121_GPIO_PIN_1	(1 << 1)
#define  VR4121_GPIO_PIN_0	(1 << 0)

/* PIU registers */

#define VR4121_PIUCNTREG	VR4121_ADRS(0x122)
#define VR4121_PIUINTREG	VR4121_ADRS(0x124)
#define VR4121_PIUSIVLREG	VR4121_ADRS(0x126)
#define VR4121_PIUSTBLREG	VR4121_ADRS(0x128)
#define VR4121_PIUCMDREG	VR4121_ADRS(0x12a)
#define VR4121_PIUASCNREG	VR4121_ADRS(0x130)
#define VR4121_PIUAMSKREG	VR4121_ADRS(0x132)
#define VR4121_PIUCIVLREG	VR4121_ADRS(0x13e)
#define VR4121_PIUPB00REG	VR4121_ADRS(0x2a0)
#define VR4121_PIUPB01REG	VR4121_ADRS(0x2a2)
#define VR4121_PIUPB02REG	VR4121_ADRS(0x2a4)
#define VR4121_PIUPB03REG	VR4121_ADRS(0x2a6)
#define VR4121_PIUPB10REG	VR4121_ADRS(0x2a8)
#define VR4121_PIUPB11REG	VR4121_ADRS(0x2aa)
#define VR4121_PIUPB12REG	VR4121_ADRS(0x2ac)
#define VR4121_PIUPB13REG	VR4121_ADRS(0x2ae)
#define VR4121_PIUAB0REG	VR4121_ADRS(0x2b0)
#define VR4121_PIUAB1REG	VR4121_ADRS(0x2b2)
#define VR4121_PIUAB2REG	VR4121_ADRS(0x2b4)
#define VR4121_PIUAB3REG	VR4121_ADRS(0x2b6)
#define VR4121_PIUPB04REG	VR4121_ADRS(0x2bc)
#define VR4121_PIUPB14REG	VR4121_ADRS(0x2be)

/* AIU registers */

#define VR4121_MDMADATREG	VR4121_ADRS(0x160)
#define VR4121_SDMADATREG	VR4121_ADRS(0x162)
#define VR4121_SODATREG		VR4121_ADRS(0x166)
#define VR4121_SCNTREG		VR4121_ADRS(0x168)
#define VR4121_SCNVRREG		VR4121_ADRS(0x16a)
#define VR4121_SCNVCUNTREG	VR4121_ADRS(0x16c)
#define VR4121_MIDATREG		VR4121_ADRS(0x170)
#define VR4121_MCNTREG		VR4121_ADRS(0x172)
#define VR4121_MCNVRREG		VR4121_ADRS(0x174)
#define VR4121_MCNVCUNTREG	VR4121_ADRS(0x176)
#define VR4121_DVALIDREG	VR4121_ADRS(0x178)
#define VR4121_SEQREG		VR4121_ADRS(0x17a)
#define VR4121_INTREG		VR4121_ADRS(0x17c)

/* KIU registers */

#define VR4121_KIUDAT0REG	VR4121_ADRS(0x180)
#define VR4121_KIUDAT1REG	VR4121_ADRS(0x182)
#define VR4121_KIUDAT2REG	VR4121_ADRS(0x184)
#define VR4121_KIUDAT3REG	VR4121_ADRS(0x186)
#define VR4121_KIUDAT4REG	VR4121_ADRS(0x188)
#define VR4121_KIUDAT5REG	VR4121_ADRS(0x18a)

#define VR4121_KIUSCANREP	VR4121_ADRS(0x190)
#define VR4121_DIUSCANS		VR4121_ADRS(0x192)
#define VR4121_KIUWKS		VR4121_ADRS(0x194)
#define VR4121_KIUWKI		VR4121_ADRS(0x196)
#define VR4121_KIUINT		VR4121_ADRS(0x198)
#define VR4121_KIURST		VR4121_ADRS(0x19a)
#define VR4121_KIUGPEN		VR4121_ADRS(0x19c)
#define VR4121_SCANLINE		VR4121_ADRS(0x19e)

/* Debug SIU registers */

#define VR4121_PORTREG		VR4121_ADRS(0x1a0)  /* new XXX */
#define VR4121_MODEMREG		VR4121_ADRS(0x1a2)  /* new XXX */
#define VR4121_ASIM00REG	VR4121_ADRS(0x1a4)
#define VR4121_ASIM01REG	VR4121_ADRS(0x1a6)
#define VR4121_RXB0RREG		VR4121_ADRS(0x1a8)
#define VR4121_RXB0LREG		VR4121_ADRS(0x1aa)
#define VR4121_RXS0RREG		VR4121_ADRS(0x1ac)
#define VR4121_TXS0LREG		VR4121_ADRS(0x1ae)
#define VR4121_ASIS0REG		VR4121_ADRS(0x1b0)
#define VR4121_INTR0REG		VR4121_ADRS(0x1b2)
#define VR4121_BRG0REG		VR4121_ADRS(0x1b4)  /* new XXX */
#define VR4121_BPRM0REG		VR4121_ADRS(0x1b6)
#define VR4121_DSIURESETREG	VR4121_ADRS(0x1b8)

/* DSIU subsystem DSIURESETREG bit definitions */

#define VR4121_DSIURST		(1 << 0)

/* DSIU subsystem ASIM00REG register bit definitions */

#define VR4121_ASIM00REG_RESERVED  (1 << 7)
#define VR4121_RXE0		   (1 << 6)
#define VR4121_DSIU_PAR_SHIFT	   4
#define VR4121_DSIU_PAR_EVEN	   (3 << VR4121_DSIU_PAR_SHIFT)
#define VR4121_DSIU_PAR_ODD	   (2 << VR4121_DSIU_PAR_SHIFT)
#define VR4121_DSIU_PAR_ZERO	   (1 << VR4121_DSIU_PAR_SHIFT)
#define VR4121_DSIU_PAR_EXTEND	   (0 << VR4121_DSIU_PAR_SHIFT)
#define VR4121_DSIU_CHARLEN_8	   (1 << 3)
#define VR4121_DSIU_CHARLEN_7	   (0 << 3)
#define VR4121_DSIU_STOPBITS_2	   (1 << 2)
#define VR4121_DSIU_STOPBITS_1	   (1 << 2)
#define VR4121_DSIU_SCLS0	   (1 << 0)  /* new XXX */

/* DSIU subsystem BPRM0REG register bit definitions */

#define VR4121_BRCE0		   (1 << 7)

/* DSIU subsystem INTR0REG register bit definitions */

#define VR4121_INTDCD		   (1 << 3)
#define VR4121_INTSER0		   (1 << 2)
#define VR4121_INTSR0		   (1 << 1)
#define VR4121_INTST0		   (1 << 0)

/* DSIU subsystem ASIS0REG register bit definitions */

#define VR4121_SOT0		   (1 << 7)
#define VR4121_PE0		   (1 << 2)
#define VR4121_FE0		   (1 << 1)
#define VR4121_OVE0		   (1 << 0)

/* LED registers */

#define VR4121_LEDHTSREG	   VR4121_ADRS(0x240)
#define VR4121_LEDTLSREG	   VR4121_ADRS(0x242)
#define VR4121_LEDHLTCLREG	   VR4121_ADRS(0x244)
#define VR4121_LEDHLTCHREG	   VR4121_ADRS(0x246)
#define VR4121_LEDCNTREG	   VR4121_ADRS(0x248)
#define VR4121_LEDASTCREG	   VR4121_ADRS(0x24a)
#define VR4121_LEDINTREG	   VR4121_ADRS(0x24c)

/* SIU registers -- (16550-compatible registers not included) */

#define VR4121_SIURSEL		   VR4121_ADRS(0x01000008)

/* HSP registers */

#define VR4121_HSPINIT		   VR4121_ADRS(0x01000020)
#define VR4121_HSPDATA		   VR4121_ADRS(0x01000022)
#define VR4121_HSPINDEX		   VR4121_ADRS(0x01000024)
#define VR4121_HSPID		   VR4121_ADRS(0x01000028)
#define VR4121_HSPPCS		   VR4121_ADRS(0x01000029)
#define VR4121_HSPPCTEL		   VR4121_ADRS(0x01000029)

#ifdef __cplusplus
}
#endif

#endif /* __INCnvr4121h */
