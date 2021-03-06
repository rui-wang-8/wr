/* nvr4131.h - NEC NVR4131 header file */

/* Copyright 2002 Wind River Systems, Inc. */

/*
modification history
--------------------
01c,20jun02,sru	 Added constants for cache control (SPR 78924).
01b,12mar02,sru  added VR4131_PCICLKSEL_DIV_3 and VR4131_PCIDMACTRLREG.
01a,10oct01,sru  created, based upon nvr4122.h
*/

/*
DESCRIPTION
This file contains constants for the NEC Vr4131.  Register address
definitions for the various subsystems are provided, and some (but
not all) register field definitions are provided.
*/

#ifndef __INCnvr4131h
#define __INCnvr4131h

#include "vxWorks.h"

#ifdef __cplusplus
extern "C" {
#endif

/* VR4131 cache sizes */

#define VR4131_ICACHE_SIZE	16384
#define VR4131_DCACHE_SIZE	16384

/* VR4131 cache line configuration (config bits in C0_CONFIG) */

#define VR4131_ICACHE_LINE_32_MASK  	(1 << 5)
#define VR4131_DCACHE_LINE_32_MASK  	(1 << 4)
#define VR4131_CACHE_LINE_SIZE_16	16
#define VR4131_CACHE_LINE_SIZE_32	32

/* VR4131 register definitions. */

#define VR4131_REG_BASE   (0x0f000000+K1BASE)

#ifdef	_ASMLANGUAGE
#define VR4131_REG32(reg)	(VR4131_REG_BASE + (reg))
#define VR4131_REG16(reg)	(VR4131_REG_BASE + (reg))
#define VR4131_REG8(reg)	(VR4131_REG_BASE + (reg))
#else
#define VR4131_REG32(reg)	((volatile UINT32 *)(VR4131_REG_BASE + (reg)))
#define VR4131_REG16(reg)	((volatile UINT16 *)(VR4131_REG_BASE + (reg)))
#define VR4131_REG8(reg)	((volatile UINT8 *)(VR4131_REG_BASE + (reg)))
#endif	/* _ASMLANGUAGE */

/* BCU registers */

#define VR4131_BCUCNTREG1	VR4131_REG16(0x00)
#define VR4131_ROMSIZEREG	VR4131_REG16(0x04)
#define VR4131_ROMSPEEDREG	VR4131_REG16(0x06)
#define VR4131_IO0SPEEDREG	VR4131_REG16(0x08)
#define VR4131_IO1SPEEDREG	VR4131_REG16(0x0a)
#define VR4131_REVIDREG		VR4131_REG16(0x10)
#define VR4131_CLKSPEEDREG	VR4131_REG16(0x14)
#define VR4131_BCUCNTREG3	VR4131_REG16(0x16)
#define VR4131_BCUCACHECNTREG	VR4131_REG16(0x18)


/* BCUCNTREG1 bit definitions */

#define VR4131_PAGESIZE		0x3000
#define VR4131_PAGEROM2		0x0400
#define VR4131_PAGEROM0		0x0100
#define VR4131_ROMWEN2		0x0040
#define VR4131_ROMWEN0		0x0010

/* DMAAU registers */

#define VR4131_CSIIBALREG	VR4131_REG16(0x20)
#define VR4131_CSIIBAHREG	VR4131_REG16(0x22)
#define VR4131_CSIIALREG	VR4131_REG16(0x24)
#define VR4131_CSIIAHREG	VR4131_REG16(0x26)
#define VR4131_CSIOBALREG	VR4131_REG16(0x28)
#define VR4131_CSIOBAHREG	VR4131_REG16(0x2a)
#define VR4131_CSIOALREG	VR4131_REG16(0x2c)
#define VR4131_CSIOAHREG	VR4131_REG16(0x2e)
#define VR4131_FIRBALREG	VR4131_REG16(0x30)
#define VR4131_FIRBAHREG	VR4131_REG16(0x32)
#define VR4131_FIRALREG		VR4131_REG16(0x34)
#define VR4131_FIRAHREG		VR4131_REG16(0x36)
#define VR4131_RAMBALREG	VR4131_REG16(0x1e0)
#define VR4131_RAMBAHREG	VR4131_REG16(0x1e2)
#define VR4131_RAMALREG		VR4131_REG16(0x1e4)
#define VR4131_RAMAHREG		VR4131_REG16(0x1e6)
#define VR4131_IOBALREG		VR4131_REG16(0x1e8)
#define VR4131_IOBAHREG		VR4131_REG16(0x1ea)
#define VR4131_IOALREG		VR4131_REG16(0x1ec)
#define VR4131_IOAHREG		VR4131_REG16(0x1ee)

/* DCU registers */

#define VR4131_DMARSTREG	VR4131_REG16(0x40)
#define VR4131_DMAIDLEREG	VR4131_REG16(0x42)
#define VR4131_DMASENREG	VR4131_REG16(0x44)
#define VR4131_DMAMSKREG	VR4131_REG16(0x46)
#define VR4131_DMAREQREG	VR4131_REG16(0x48)
#define VR4131_TDREG		VR4131_REG16(0x4a)
#define VR4131_DMAABITREG	VR4131_REG16(0x4c)
#define VR4131_CONTROLREG	VR4131_REG16(0x4e)
#define VR4131_BASSCNTLREG	VR4131_REG16(0x50)
#define VR4131_BASSCNTHREG	VR4131_REG16(0x52)
#define VR4131_CURRENTCNTLREG	VR4131_REG16(0x54)
#define VR4131_CURRENTCNTHREG	VR4131_REG16(0x56)
#define VR4131_TCINTR		VR4131_REG16(0x58)

/* DMA mask bit definitions */

#define  VR4131_DMAMSKAIOR	0x0008
#define  VR4131_DMAMSKCOUT	0x0004
#define  VR4131_DMAMSKCIN	0x0002
#define  VR4131_DMAMSKFOUT	0x0001

/* CMU register */

#define VR4131_CMUCLKMSK	VR4131_REG16(0x60)

#define VR4131_MSKPCIU		0x2080
#define VR4131_MSKSCSI		0x1000
#define VR4131_MSKDSIU		0x0800
#define VR4131_MSKFFIR		0x0400
#define VR4131_MSKSSIU		0x0100
#define VR4131_MSKCSI		0x0040
#define VR4131_MSKFIR		0x0010
#define VR4131_MSKSIU		0x0002

/* ICU system and system mask registers */

#define VR4131_SYSINT1REG  	VR4131_REG16(0x80)
#define VR4131_GIUINTLREG  	VR4131_REG16(0x88)
#define VR4131_DSIUINTREG  	VR4131_REG16(0x8a)
#define VR4131_MSYSINT1REG	VR4131_REG16(0x8c)
#define VR4131_MGIUINTLREG  	VR4131_REG16(0x94)
#define VR4131_MDSIUINTREG  	VR4131_REG16(0x96)
#define VR4131_NMIREG		VR4131_REG16(0x98)
#define VR4131_SOFTINTREG	VR4131_REG16(0x9a)
#define VR4131_SYSINT2REG	VR4131_REG16(0xa0)
#define VR4131_GIUINTHREG	VR4131_REG16(0xa2)
#define VR4131_FIRINTREG	VR4131_REG16(0xa4)
#define VR4131_MSYSINT2REG	VR4131_REG16(0xa6)
#define VR4131_MGIUINTHREG	VR4131_REG16(0xa8)
#define VR4131_MFIRINTREG	VR4131_REG16(0xaa)
#define VR4131_PCIINTREG	VR4131_REG16(0xac)
#define VR4131_SCUINTREG	VR4131_REG16(0xae)
#define VR4131_CSIINTREG	VR4131_REG16(0xb0)
#define VR4131_MPCIINTREG	VR4131_REG16(0xb2)
#define VR4131_MSCUINTREG	VR4131_REG16(0xb4)
#define VR4131_MCSIINTREG	VR4131_REG16(0xb6)
#define VR4131_BCUINTREG	VR4131_REG16(0xb8)
#define VR4131_MBCUINTREG	VR4131_REG16(0xba)

#define VR4131_CLKRUNINTR	0x1000
#define VR4131_SOFTINTR		0x0800
#define VR4131_SIUINTR		0x0200
#define VR4131_GIUINTR		0x0100
#define VR4131_ETIMERINTR	0x0008
#define VR4131_RTCL1INTR	0x0004
#define VR4131_POWERINTR	0x0002
#define VR4131_BATINTR		0x0001

#define VR4131_BCUINTR		0x0200
#define VR4131_CSIINTR		0x0100
#define VR4131_SCUINTR		0x0080
#define VR4131_PCIINTR		0x0040
#define VR4131_DSIUINTR		0x0020
#define VR4131_FIRINTR		0x0010
#define VR4131_TCLKINTR		0x0008
#define VR4131_LEDINTR		0x0002
#define VR4131_RTCL2INTR	0x0001

/* PMU registers */

#define VR4131_PMUINTREG	VR4131_REG16(0xc0)
#define VR4131_PMUCNTREG	VR4131_REG16(0xc2)
#define VR4131_PMUINT2REG	VR4131_REG16(0xc4)
#define VR4131_PMUCNT2REG	VR4131_REG16(0xc6)
#define VR4131_PMUWAITREG	VR4131_REG16(0xc8)
#define VR4131_PMUTCLKDIVREG	VR4131_REG16(0xcc)
#define VR4131_PMUINTRCLKDIVREG	VR4131_REG16(0xce)

#define VR4131_HALTIMERRST	0x0004

/* RTC registers */

#define VR4131_ETIMELREG	VR4131_REG16(0x100)
#define VR4131_ETIMEMREG	VR4131_REG16(0x102)
#define VR4131_ETIMEHREG	VR4131_REG16(0x104)
#define VR4131_ECMPLREG		VR4131_REG16(0x108)
#define VR4131_ECMPMREG		VR4131_REG16(0x10a)
#define VR4131_ECMPHREG		VR4131_REG16(0x10c)
#define VR4131_RTCL1LREG	VR4131_REG16(0x110)
#define VR4131_RTCL1HREG	VR4131_REG16(0x112)
#define VR4131_RTCL1CNTLREG	VR4131_REG16(0x114)
#define VR4131_RTCL1CNTHREG	VR4131_REG16(0x116)
#define VR4131_RTCL2LREG	VR4131_REG16(0x118)
#define VR4131_RTCL2HREG	VR4131_REG16(0x11a)
#define VR4131_RTCL2CNTLREG	VR4131_REG16(0x11c)
#define VR4131_RTCL2CNTHREG	VR4131_REG16(0x11e)
#define VR4131_TCLKLREG		VR4131_REG16(0x120)
#define VR4131_TCLKHREG		VR4131_REG16(0x122)
#define VR4131_TCLKCNTLREG	VR4131_REG16(0x124)
#define VR4131_TCLKCNTHREG	VR4131_REG16(0x126)
#define VR4131_RTCINTREG	VR4131_REG16(0x13e)

#define VR4131_RTCINTR0		0x0001
#define VR4131_RTCINTR1		0x0002
#define VR4131_RTCINTR2		0x0004
#define VR4131_RTCINTR3		0x0008

/* 
 * The VR4131 RTC module has identical functionality to the VR4102
 * RTC module. In order to use the nvr4102RTCTimer.c driver, we must
 * define the VR4102_... constants in terms of the VR4131 values.
 */

#define VR4102_ETIMELREG	VR4131_ETIMELREG
#define VR4102_ETIMEMRE		VR4131_ETIMEMRE
#define VR4102_ETIMEHREG	VR4131_ETIMEHREG
#define VR4102_ECMPLREG 	VR4131_ECMPLREG
#define VR4102_ECMPMREG 	VR4131_ECMPMREG
#define VR4102_ECMPHREG 	VR4131_ECMPHREG
#define VR4102_RTCL1LREG	VR4131_RTCL1LREG
#define VR4102_RTCL1HREG	VR4131_RTCL1HREG
#define VR4102_RTCL1CNTLREG	VR4131_RTCL1CNTLREG
#define VR4102_RTCL1CNTHREG	VR4131_RTCL1CNTHREG
#define VR4102_RTCL2LREG	VR4131_RTCL2LREG
#define VR4102_RTCL2HREG	VR4131_RTCL2HREG
#define VR4102_RTCL2CNTLREG	VR4131_RTCL2CNTLREG
#define VR4102_RTCL2CNTHREG	VR4131_RTCL2CNTHREG
#define VR4102_TCLKLREG		VR4131_TCLKLREG
#define VR4102_TCLKHREG		VR4131_TCLKHREG
#define VR4102_TCLKCNTLREG	VR4131_TCLKCNTLREG
#define VR4102_TCLKCNTHREG	VR4131_TCLKCNTHREG
#define VR4102_RTCINTREG  	VR4131_RTCINTREG
#define VR4102_RTC_RTCINTR0	VR4131_RTCINTR0
#define VR4102_RTC_RTCINTR1	VR4131_RTCINTR1
#define VR4102_RTC_RTCINTR2	VR4131_RTCINTR2
#define VR4102_RTC_RTCINTR3	VR4131_RTCINTR3

#define VR4102_RTCL1INTR	VR4131_RTCL1INTR
#define VR4102_RTCL2INTR	VR4131_RTCL2INTR

#define VR4102_ICU_MSYSINT1REG	VR4131_MSYSINT1REG

/* GIU registers */

#define VR4131_GIUIOSELL	VR4131_REG16(0x140)
#define VR4131_GIUIOSELH	VR4131_REG16(0x142)
#define VR4131_GIUPIODL		VR4131_REG16(0x144)
#define VR4131_GIUPIODH		VR4131_REG16(0x146)
#define VR4131_GIUINTSTATL	VR4131_REG16(0x148)
#define VR4131_GIUINTSTATH	VR4131_REG16(0x14a)
#define VR4131_GIUINTENL	VR4131_REG16(0x14c)
#define VR4131_GIUINTENH	VR4131_REG16(0x14e)
#define VR4131_GIUINTTYPL	VR4131_REG16(0x150)
#define VR4131_GIUINTTYPH	VR4131_REG16(0x152)
#define VR4131_GIUINTALSELL	VR4131_REG16(0x154)
#define VR4131_GIUINTALSELH	VR4131_REG16(0x156)
#define VR4131_GIUINTHTSELL	VR4131_REG16(0x158)
#define VR4131_GIUINTHTSELH	VR4131_REG16(0x15a)
#define VR4131_GIUPODATEN	VR4131_REG16(0x15c)
#define VR4131_GIUPODATL	VR4131_REG16(0x15e)

/* 
 * The general-purpose I/O pins (GPIO) are enabled and controlled via
 * identically placed bits in the GIU registers and some of the ICU
 * registers. This set of pin masks can be used with whichever registers
 * contain GPIO pin configuration. 
 */

#define  VR4131_GPIO_PIN_31	0x8000
#define  VR4131_GPIO_PIN_30	0x4000
#define  VR4131_GPIO_PIN_29	0x2000
#define  VR4131_GPIO_PIN_28	0x1000
#define  VR4131_GPIO_PIN_27	0x0800
#define  VR4131_GPIO_PIN_26	0x0400
#define  VR4131_GPIO_PIN_25	0x0200
#define  VR4131_GPIO_PIN_24	0x0100
#define  VR4131_GPIO_PIN_23	0x0080
#define  VR4131_GPIO_PIN_22	0x0040
#define  VR4131_GPIO_PIN_21	0x0020
#define  VR4131_GPIO_PIN_20	0x0010
#define  VR4131_GPIO_PIN_19	0x0008
#define  VR4131_GPIO_PIN_18	0x0004
#define  VR4131_GPIO_PIN_17	0x0002
#define  VR4131_GPIO_PIN_16	0x0001
#define  VR4131_GPIO_PIN_15	0x8000
#define  VR4131_GPIO_PIN_14	0x4000
#define  VR4131_GPIO_PIN_13	0x2000
#define  VR4131_GPIO_PIN_12	0x1000
#define  VR4131_GPIO_PIN_11	0x0800
#define  VR4131_GPIO_PIN_10	0x0400
#define  VR4131_GPIO_PIN_9	0x0200
#define  VR4131_GPIO_PIN_8	0x0100
#define  VR4131_GPIO_PIN_7	0x0080
#define  VR4131_GPIO_PIN_6	0x0040
#define  VR4131_GPIO_PIN_5	0x0020
#define  VR4131_GPIO_PIN_4	0x0010
#define  VR4131_GPIO_PIN_3	0x0008
#define  VR4131_GPIO_PIN_2	0x0004
#define  VR4131_GPIO_PIN_1	0x0002
#define  VR4131_GPIO_PIN_0	0x0001

/* SCI registers */

#define VR4131_TIMOUTCNTREG	VR4131_REG16(0x1000)
#define VR4131_TIMOUTCOUNTREG	VR4131_REG16(0x1002)
#define VR4131_ERRLADDRESSREG	VR4131_REG16(0x1004)
#define VR4131_ERRHADDRESSREG	VR4131_REG16(0x1006)
#define VR4131_SCUINTRREG	VR4131_REG16(0x1008)

/* SDRAMU registers */

#define VR4131_SDRAMMODEREG	VR4131_REG16(0x400)
#define VR4131_SDRAMCNTREG	VR4131_REG16(0x402)
#define VR4131_BCURFCNTREG	VR4131_REG16(0x404)
#define VR4131_BCURFCOUNTREG	VR4131_REG16(0x406)
#define VR4131_RAMSIZEREG	VR4131_REG16(0x408)

/* PCIU registers */

#define VR4131_PCIMMAW1REG	VR4131_REG32(0xc00)
#define VR4131_PCIMMAW2REG	VR4131_REG32(0xc04)
#define VR4131_PCITAW1REG	VR4131_REG32(0xc08)
#define VR4131_PCITAW2REG	VR4131_REG32(0xc0c)
#define VR4131_PCIMIOAWREG	VR4131_REG32(0xc10)
#define VR4131_PCICONFDREG	VR4131_REG32(0xc14)
#define VR4131_PCICONFAREG	VR4131_REG32(0xc18)
#define VR4131_PCIMAILREG	VR4131_REG32(0xc1c)
#define VR4131_BUSERRADREG	VR4131_REG32(0xc24)
#define VR4131_INTCNTSTAREG	VR4131_REG32(0xc28)
#define VR4131_PCIEXACCREG	VR4131_REG32(0xc2c)
#define VR4131_PCIRECONTREG	VR4131_REG32(0xc30)
#define VR4131_PCIENREG		VR4131_REG32(0xc34)
#define VR4131_PCICLKSELREG	VR4131_REG32(0xc38)
#define VR4131_PCITRDYVREG	VR4131_REG32(0xc3c)
#define VR4131_PCICLKRUNREG	VR4131_REG16(0xc60)
#define VR4131_PCIDMACTRLREG	VR4131_REG32(0xc80)

#define VR4131_PCICLKSEL_DIV_1  0x2
#define VR4131_PCICLKSEL_DIV_2  0x0
#define VR4131_PCICLKSEL_DIV_3  0x3
#define VR4131_PCICLKSEL_DIV_4  0x1

#define VR4131_PCICONFIGDONE	0x00000004	/* PCIENREG */
#define VR4131_PCICLKRUN	0x0001		/* PCICLKRUNREG */
#define VR4131_PCISTOPEN	0x8000		/* PCICLKRUNREG */
#define VR4131_PCIIBA		0xFF000000	/* PCIM*AW*REG */
#define VR4131_PCIMSK		0x000FE000	/* PCIM*AW*REG, PCITAWnREG */
#define VR4131_PCIWINEN		0x00001000	/* PCIM*AW*REG, PCITAWnREG */
#define VR4131_PCIPCIA		0x000000FF	/* PCIM*AW*REG */
#define VR4131_PCIITA		0x000007FF	/* PCITAWnREG */

/* PCI Config Registers */

#define VR4131_PCICONF_IDENT	VR4131_REG32(0xd00)
#define VR4131_PCICONF_CMDSR	VR4131_REG32(0xd04)
#define VR4131_PCICONF_REVCLASS	VR4131_REG32(0xd08)
#define VR4131_PCICONF_CACHELAT	VR4131_REG32(0xd0c)
#define VR4131_PCICONF_MAILBA	VR4131_REG32(0xd10)
#define VR4131_PCICONF_PCIMBA1	VR4131_REG32(0xd14)
#define VR4131_PCICONF_PCIMBA2	VR4131_REG32(0xd18)
#define VR4131_PCICONF_PCIINT	VR4131_REG32(0xd3c)
#define VR4131_PCICONF_RETVAL	VR4131_REG32(0xd40)

/* DSIU registers */

#define VR4131_DSIURB		VR4131_REG8(0x820)	/* SUILC7 = 0, read */
#define VR4131_DSIUTH		VR4131_REG8(0x820)	/* SUILC7 = 0, write */
#define VR4131_DSIUDLL		VR4131_REG8(0x820)	/* SUILC7 = 1 */
#define VR4131_DSIUIE		VR4131_REG8(0x821)	/* SUILC7 = 0 */
#define VR4131_DSIUDLM		VR4131_REG8(0x821)	/* SUILC7 = 1 */
#define VR4131_DSIUIID		VR4131_REG8(0x822)	/* read */
#define VR4131_DSIUFC		VR4131_REG8(0x822)	/* write */
#define VR4131_DSIULC		VR4131_REG8(0x823)
#define VR4131_DSIUMC		VR4131_REG8(0x824)
#define VR4131_DSIULS		VR4131_REG8(0x825)
#define VR4131_DSIUMS		VR4131_REG8(0x826)
#define VR4131_DSIUSC		VR4131_REG8(0x827)
#define VR4131_DSIURESET	VR4131_SIURESET		/* Common with SIU */

#define VR4131_DSIU_BASE	VR4131_DSIURB
#define VR4131_DSIU_DELTA	1
#define VR4131_DSIU_XTAL	18432000	/* crystal input to 16550 */

#define VR4131_DSIURST		0x0002		/* in SIURESET register */

/* LED registers */

#define VR4131_LEDHTSREG	VR4131_REG16(0x180)
#define VR4131_LEDLTSREG	VR4131_REG16(0x182)
#define VR4131_LEDCNTREG	VR4131_REG16(0x188)
#define VR4131_LEDASTCREG	VR4131_REG16(0x18a)
#define VR4131_LEDINTREG	VR4131_REG16(0x18c)

/* SIU registers */

#define VR4131_SIURB		VR4131_REG8(0x800)	/* SUILC7 = 0, read */
#define VR4131_SIUTH		VR4131_REG8(0x800)	/* SUILC7 = 0, write */
#define VR4131_SIUDLL		VR4131_REG8(0x800)	/* SUILC7 = 1 */
#define VR4131_SIUIE		VR4131_REG8(0x801)	/* SUILC7 = 0 */
#define VR4131_SIUDLM		VR4131_REG8(0x801)	/* SUILC7 = 1 */
#define VR4131_SIUIID		VR4131_REG8(0x802)	/* read */
#define VR4131_SIUFC		VR4131_REG8(0x802)	/* write */
#define VR4131_SIULC		VR4131_REG8(0x803)
#define VR4131_SIUMC		VR4131_REG8(0x804)
#define VR4131_SIULS		VR4131_REG8(0x805)
#define VR4131_SIUMS		VR4131_REG8(0x806)
#define VR4131_SIUSC		VR4131_REG8(0x807)
#define VR4131_SIUIRSEL		VR4131_REG8(0x808)
#define VR4131_SIURESET		VR4131_REG8(0x809)	/* common with DSIU */
#define VR4131_SIUCSEL		VR4131_REG8(0x80a)

#define VR4131_SIU_BASE		VR4131_SIURB
#define VR4131_SIU_DELTA	1
#define VR4131_SIU_XTAL		18432000	/* crystal input to 16550 */

#define VR4131_SIURST		0x0001		/* in SIURESET register */

/* CSI registers */

#define VR4131_CSI_MODEREG	VR4131_REG16(0x1a0)
#define VR4131_CSI_CLKSELREG	VR4131_REG16(0x1a1)
#define VR4131_CSI_SIRBREG	VR4131_REG16(0x1a2)
#define VR4131_CSI_SOTBREG	VR4131_REG16(0x1a4)
#define VR4131_CSI_SIRBEREG	VR4131_REG16(0x1a6)
#define VR4131_CSI_SOTBFREG	VR4131_REG16(0x1a8)
#define VR4131_CSI_SIOREG	VR4131_REG16(0x1aa)
#define VR4131_CSI_CNTREG	VR4131_REG16(0x1b0)
#define VR4131_CSI_INTREG	VR4131_REG16(0x1b2)
#define VR4131_CSI_IFIFOVREG	VR4131_REG16(0x1b4)
#define VR4131_CSI_OFIFOVREG	VR4131_REG16(0x1b6)
#define VR4131_CSI_IFIFOREG	VR4131_REG16(0x1b8)
#define VR4131_CSI_OFIFOREG	VR4131_REG16(0x1ba)
#define VR4131_CSI_FIFOTRGREG	VR4131_REG16(0x1bc)

/* FIR registers - not included */

/* Clock rate values for different settings of CLKSEL[2:0] pins/jumpers. */

#define CPU_PCLOCK_RATE_111	200700000
#define CPU_PCLOCK_RATE_110	180600000
#define CPU_PCLOCK_RATE_101	164200000
#define CPU_PCLOCK_RATE_100	150500000
#define CPU_PCLOCK_RATE_011	129000000
#define CPU_PCLOCK_RATE_010	100400000
#define CPU_PCLOCK_RATE_001	 90300000
#define CPU_PCLOCK_RATE_000	 78500000

/* Miscellaneous */

#define NUM_4131_TTY		2	/* SIU + DSIU */

#ifdef __cplusplus
}
#endif

#endif /* __INCnvr4131h */
