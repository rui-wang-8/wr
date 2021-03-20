/* cpv5000.h - CPV5xxx header */

/* Copyright 1999-2002 Wind River Systems, Inc. All Rights Reserved */
/* Copyright 1999-2000 Motorola, Inc. All Rights Reserved */

/*
modification history
--------------------
01x,16apr02,dat  Update for T2.2 release, ideDrv.h removed.
01w,22may00,djs  Added support for the CPV5350.
01v,22feb00,scb  Add PIIX #defines to allow CPU reset.
01u,18feb00,scb  Mods for shared memory support enhancement.
01t,01feb00,djs  incorporate WRS review comments.
01s,14oct99,sjb  Added comment near "FEI PCI bus resource" defines
01r,04oct99,scb  Moved SYS_SM_CPCI_BUS_NUMBER to this file (shared mem support).
01q,18aug99,sjb  Added Failsafe Timer support.
01p,29jul99,scb  Added LM78 (system monitor) support.
01o,02jul99,djs  added defines for CPV5300
01n,17jun99,sjb  changes to support NVRAM Validation Test
01m,16jun99,sjb  support for read/write of CMOS block on cpv5000
01l,11jun99,scb  Fixed PCI window naming.
01k,07jun99,scb  Port from T1 to T2.
01j,07may99,djs  ATA1_INT_LVL was changed from 0x09 to 0x0f
01i,03may99,scb  PCI prefetch/nonprefetch mem space control moved to "config.h".
01h,20apr99,scb  Remove SYS_SM_ANCHOR_POLL_LIST conditional compile.
01g,12apr99,scb  Move SM_TAS_TYPE from "config.h" to "cpv5000.h".
01f,24mar99,scb  Support for shared memory.
01e,26mar99,djs  removed the PIRQ defines
01d,24mar99,djs  changes made to only run pciAutoConfig code from ROM image.
		 added defines PCI_AUTOCONFIG_DONE & PCI_AUTOCONFIG_FLAG
01c,23mar99,djs  changes made to reset secondary PCI bus
01b,22mar99,djs  trim down the PCI memory spaces to optimize mmu table
01a,08mar99,djs  file derived from pc486.h (01z,17mar98)
*/

/*
This file contains IO address and related constants for the
CPV5000
*/

#ifndef	__INCcpv5000h
#define	__INCcpv5000h

#include "drv/intrCtl/i8259.h"
#include "drv/timer/i8253.h"
#include "drv/timer/mc146818.h"
#include "drv/timer/timerDev.h"
#include "drv/timer/timestampDev.h"
#include "drv/fdisk/nec765Fd.h"
#include "drv/serial/pcConsole.h"
#include "drv/parallel/lptDrv.h"
#include "drv/pcmcia/pcmciaLib.h"
#include "drv/hdisk/ataDrv.h"
#include "arch/i86/mmuI86Lib.h"
#include "drv/pci/pciConfigLib.h"

#undef	CAST
#define	CAST

#if defined(CPV5000)
#   define SYS_SM_CPCI_BUS_NUMBER    1		/* Compact PCI bus number */
#   define PCI_ID_82371SB	0x70008086	/* Id for 82371SB PIIX3	*/
#   define PCI_ID_82439HX	0x12508086	/* CPU to PCI bridge */

#   define PCI_DEVID_HB		PCI_ID_82439HX
#   define PCI_DEVID_ISA	PCI_ID_82371SB
#   define PCI_DEVID_IDE	0x70108086
#   define PCI_DEVID_USB	0x70208086
#   define PCI_DEVID_VIDEO	0x00b81013

#   define PCI2PCI_BUS		0x00
#   define PCI2PCI_DEV		0x12
#   define PCI2PCI_FUN		0x00

/* LM78	related:
 *
 * INO through IN4 are the positive voltage inputs to the LM78.
 * In order to correctly compute the source voltages (V(s)), the
 * attenuation resistor	values (R1 and R2) must	be made	available
 * since these resistance values are parameters	in the voltage
 * calculation formula.	 The resistance	values are presented
 * below.
 */

/* Note, when no attenuation, R1 = 0, R2 = any value not equal to 0 */

#   define LM78_IN0_R1		(0.0)
#   define LM78_IN0_R2		(10000.0)	/* No attenuation */

#   define LM78_IN1_R1		(0.0)		/* No attenuation */
#   define LM78_IN1_R2		(10000.0)

#   define LM78_IN2_R1		(6800.0)
#   define LM78_IN2_R2		(10000.0)

#   define LM78_IN3_R1		(27400.0)
#   define LM78_IN3_R2		(10000.0)

#   define LM78_IN4_R1		(0.0)		/* No attenuation */
#   define LM78_IN4_R2		(10000.0)

/*
 * In a	similar	manner to the positive input voltages to the
 * LM78, the negative inputs (IN5 and IN6) have	associated input
 * resistors which are used in the voltage computation formula.
 * They	are presented below
 */

#   define LM78_IN5_RIN		(200000.0)
#   define LM78_IN5_RF		(49900.0)

#   define LM78_IN6_RIN		(0.0)		/* Not connected */
#   define LM78_IN6_RF		(1.0)

/*
 * To properly compute the fan RPMs, fan divisors values of 1,
 * 2, 4, or 8 need to be defined.
 */

#   define LM78_FAN1_DIVISOR	(2)		/* Not connected */
#   define LM78_FAN2_DIVISOR	(2)		/* Not connected */

/* Note	that FAN 3's divisor is	hard-wired to divisor of 2 */

#   define LM78_FAN3_DIVISOR	(2)		/* Not connected */

#elif defined(CPV5300) || defined(CPV5350)
#   define SYS_SM_CPCI_BUS_NUMBER    2		/* Compact PCI bus number */
#   define PCI_ID_82371AB	0x71108086	/* Id for 82443BX PIIX4	*/
#   define PCI_ID_82443BX	0x71908086	/* CPU to PCI bridge */

#   define PCI_DEVID_HB		PCI_ID_82443BX
#   define PCI_DEVID_ISA	PCI_ID_82371AB
#   define PCI_DEVID_IDE	0x71118086
#   define PCI_DEVID_USB	0x71128086
#   define PCI_DEVID_VIDEO	0x78008086

#   define PCI2PCI_BUS		0x00
#   define PCI2PCI_DEV		0x11
#   define PCI2PCI_FUN		0x00

/* LM78	related: */

#   define LM78_IN0_R1		(6800.0)
#   define LM78_IN0_R2		(10000.0)

/* Note, when no attenuation, R1 = 0, R2 = any value not equal to 0 */

#   define LM78_IN1_R1		(0.0)		/* No attenuation */
#   define LM78_IN1_R2		(10000.0)

#   define LM78_IN2_R1		(30000.0)
#   define LM78_IN2_R2		(10000.0)

#   define LM78_IN3_R1		(0.0)		/* No attenuation */
#   define LM78_IN3_R2		(10000.0)

#   define LM78_IN4_R1		(0.0)		/* No attenuation */
#   define LM78_IN4_R2		(10000.0)

#   define LM78_IN5_RIN		(200000.0)
#   define LM78_IN5_RF		(50000.0)

#   define LM78_IN6_RIN		(0.0)		/* Not connected */
#   define LM78_IN6_RF		(1.0)

#   define LM78_FAN1_DIVISOR	(2)		/* Not connected */
#   define LM78_FAN2_DIVISOR	(2)		/* Not connected */

/* Note	that FAN 3's divisor is	hard-wired to divisor of 2 */

#   define LM78_FAN3_DIVISOR	(2)		/* Not connected */
#endif

#if defined(CPV5350)
#undef INCLUDE_SCSI
#else
#define INCLUDE_SCSI
#endif /* defined(CPV5350) */

#define TARGET_PC386

#ifndef BUS_TYPE_PCI
#define BUS_TYPE_PCI      3
#endif

#define BUS		BUS_TYPE_PCI	/* XXX */

#define INT_NUM_IRQ0	0x20		/* vector number for IRQ0 */

#define	BOOTCODE_IN_RAM			/* for ../all/bootInit.c */

/* constant values in romInit.s */

#define ROM_IDTR	0xaf		/* offset to romIdtr  */
#define ROM_GDTR	0xb5		/* offset to romGdtr  */
#define ROM_GDT		0xc0		/* offset to romGdt   */
#define ROM_INIT2	0xf0		/* offset to romInit2 */
#define ROM_STACK	0x7000		/* initial stack pointer */
#define ROM_WARM_HIGH	0x10		/* warm start entry p */
#define ROM_WARM_LOW	0x20		/* warm start entry p */

/* programmable interrupt controller (PIC) */

#define	PIC1_BASE_ADR		0x20
#define	PIC2_BASE_ADR		0xa0
#define	PIC_REG_ADDR_INTERVAL	1	/* address diff of adjacent regs. */

/* serial ports (COM1,COM2) */

#define	COM1_BASE_ADR		0x3f8
#define	COM2_BASE_ADR		0x2f8
#define	COM1_INT_LVL		0x04
#define	COM2_INT_LVL		0x03
#define	UART_REG_ADDR_INTERVAL	1	/* address diff of adjacent regs. */
#define	N_UART_CHANNELS		2

/* PIIX3 or PIIX4 Reset Control Register Defines */

#define PIIX_RC        0xCF9    /* reset control register offset */
#define PIIX_RC_RCPU   0x04     /* reset initiate bit */
#define PIIX_RC_SRST   0x02     /* reset type bit 1 = hard */

/*
 * sysPhysMemDesc[] dummy entries:
 * these create space for updating sysPhysMemDesc table at a later stage
 * mainly to provide plug and play
 */

#define DUMMY_PHYS_ADDR         -1
#define DUMMY_VIRT_ADDR         -1
#define DUMMY_LENGTH            -1
#define DUMMY_INIT_STATE_MASK   -1
#define DUMMY_INIT_STATE        -1

#define DUMMY_MMU_ENTRY		{ (void *) DUMMY_PHYS_ADDR, \
                                  (void *) DUMMY_VIRT_ADDR, \
                                  DUMMY_LENGTH, \
                                  DUMMY_INIT_STATE_MASK, \
                                  DUMMY_INIT_STATE \
                                }

/* APIC (IO APIC + Local APIC) */

#define IOAPIC_BASE		0xfec00000      /* IO APIC Base Address */
#define IOAPIC_LENGTH		0x00010000      /* IO APIC registers length */
#define LOAPIC_BASE		0xfee00000      /* Local APIC Base Address */
#define LOAPIC_LENGTH		0x00001000      /* Local APIC registers length */
#define EBDA_START		0x9fc00		/* Extended BIOS Data Area */ 
#define EBDA_END		0x9ffff
#define BIOS_ROM_START		0xf0000		/* BIOS ROM space */
#define BIOS_ROM_END		0xfffff

/* Local APIC Timer, Spurious, Error */

#define TIMER_CLOCK_HZ          60000000	/* 60Mhz */
#define TIMER_INT_LVL           0		/* any number for WindView */
#define SPURIOUS_INT_LVL        24		/* any number for WindView */
#define ERROR_INT_LVL           25		/* any number for WindView */

#define INCLUDE_PCI             /* always include pci */

/* PCI Device/Vendor IDs */
 
#define PCI_ID_HOST_BRIDGE      PCI_DEVID_HB    /* CPU to PCI bridge */
#define PCI_ID_ISA_BRIDGE	PCI_DEVID_ISA   /* Id for PCI to ISA Bridge */
#define PCI_ID_IDE          	PCI_DEVID_IDE   /* Id for IDE */
#define PCI_ID_USB          	PCI_DEVID_USB   /* Id for USB */
#define PCI_ID_VIDEO            PCI_DEVID_VIDEO /* Id for Video */
#define PCI_ID_AIC7880_MB       0x80789004      /* Id for Adaptec SCSI */
#define PCI_ID_LN_82557         0x12298086      /* Id for Intel 82557 */
#define PCI_ID_BR_DEC21150      0x00221011      /* Id DEC 21150 PCI bridge */
#define PCI_ID_AGP   		0x71918086      /* PCI to PCI bridge (AGP) */
#define PCI_ID_PM		0x71138086	/* Power Management */

#undef _CACHE_ALIGN_SIZE
#define _CACHE_ALIGN_SIZE 32

/* ISA bus window related */

#define ISA_MSTR_IO_LOCAL		0x00001000
#define ISA_MSTR_IO_BUS			0x00001000
#define ISA_MSTR_IO_SIZE		0x00006000

/* PCI bus IO window related */

#define PCI_MSTR_IO_LOCAL		0x00008000
#define PCI_MSTR_IO_BUS			0x00008000
#define PCI_MSTR_IO_SIZE		0x00008000

/* PCI bus memory related (when using INCLUDE_PCI_AUTOCONF ) */

#define PCI_MEM_START			0x40000000

/* PCI bus nonprefetchable memory related */

#define PCI_MSTR_MEMIO_LOCAL		PCI_MEM_START
#define PCI_MSTR_MEMIO_BUS		PCI_MSTR_MEMIO_LOCAL

/* PCI bus prefetchable memory related */

#define PCI_MSTR_MEM_LOCAL		(((uint32_t)PCI_MSTR_MEMIO_BUS + \
				  		(uint32_t)PCI_MSTR_MEMIO_SIZE))
#define PCI_MSTR_MEM_BUS		PCI_MSTR_MEM_LOCAL

/* Latency Timer value - 255 PCI clocks */
 
#define PCI_LAT_TIMER       0xff

#define PCI2ISA_BUS		0x00
#define PCI2ISA_DEV		0x07
#define PCI2ISA_FUN		0x00

#define PCI2ISA_PIRQ_OFF	0x60
#define PCI2ISA_PIRQ_DISABLED	0x80


/*
 * Support for determining if we have already run the PCI
 * auto config and its not needed to be done a second time.
 */

#define PCI_AUTOCONFIG_FLAG_OFFSET ( 0x1400 )
#define PCI_AUTOCONFIG_FLAG ( *(UCHAR *)(LOCAL_MEM_LOCAL_ADRS + \
				     PCI_AUTOCONFIG_FLAG_OFFSET) )
#define PCI_AUTOCONFIG_DONE ( PCI_AUTOCONFIG_FLAG != 0 )

/* AIC7880 PCI bus resources */

#define AIC7880_MB_PCI_DEVICE_ID    0x8078
#define AIC7880_MEMBASE  	0xf5200000	
#define AIC7880_MEMSIZE		0x00001000	/* memory size for CSR, 4KB */
#define AIC7880_IOBASE		0xf800	
#define AIC7880_INT_LVL		0x0a
#define AIC7880_INT_VEC		(INT_NUM_IRQ0 + AIC7880_INT_LVL)
#define AIC7880_INIT_STATE_MASK (VM_STATE_MASK_VALID | \
				 VM_STATE_MASK_WRITABLE | \
                                 VM_STATE_MASK_CACHEABLE)
#define AIC7880_INIT_STATE      (VM_STATE_VALID | VM_STATE_WRITABLE | \
                                 VM_STATE_CACHEABLE_NOT)

/* timer (PIT) */

#define	PIT_BASE_ADR		0x40
#define	PIT0_INT_LVL		0x00
#define	PIT_REG_ADDR_INTERVAL	1	/* address diff of adjacent regs. */
#define	PIT_CLOCK		1193180

/* real time clock (RTC) */

#define	RTC_INDEX		0x70
#define	RTC_DATA		0x71
#define	RTC_INT_LVL		0x08

/* LM78 system monitor */

#define LM78_ISA_BASE_ADRS       0x50	/* Base address to access LM78 */

/* failsafe watchdog timer access address in the PLFPGA */

#define FAILSAFE_ISA_BASE_ADRS  (0x50)

/* NVRAM */

#define	NVRAM_INDEX		RTC_INDEX
#define	NVRAM_DATA		RTC_DATA

/* floppy disk (FD) */

#define FD_INT_LVL              0x06
#define FD_DMA_BUF_ADDR		0x2000	/* floppy disk DMA buffer address */
#define FD_DMA_BUF_SIZE		0x3000	/* floppy disk DMA buffer size */

/* hard disk (IDE) */

#define IDE_CONFIG		0x0	/* 1: uses ideTypes table */
#define IDE_INT_LVL             0x0e
#define IDE_INT_VEC             (INT_NUM_IRQ0 + IDE_INT_LVL)

/* hard disk (ATA) */

#define ATA0_IO_START0		0x1f0	/* io for ATA0 */
#define ATA0_IO_STOP0		0x1f7
#define ATA0_IO_START1		0x3f6
#define ATA0_IO_STOP1		0x3f7
#define ATA0_INT_LVL            0x0e
#define ATA0_CONFIG		(ATA_GEO_CURRENT | ATA_PIO_AUTO | \
				 ATA_BITS_16 | ATA_PIO_MULTI)
#define ATA1_IO_START0		0x170	/* io for ATA1 */
#define ATA1_IO_STOP0		0x177
#define ATA1_IO_START1		0x376
#define ATA1_IO_STOP1		0x377
#define ATA1_INT_LVL		0x0f
#define ATA1_CONFIG		(ATA_GEO_CURRENT | ATA_PIO_AUTO | \
				 ATA_BITS_16 | ATA_PIO_MULTI)
#define ATA_SEM_TIMEOUT		5       /* timeout for ATA sync sem */
#define ATA_WDG_TIMEOUT		5       /* timeout for ATA watch dog */

/* pcmcia (PCMCIA) */

#define PCMCIA_SOCKS		0x0	/* number of sockets. 0=auto detect */
#define PCMCIA_MEMBASE		0x0	/* mapping base address */
#define PCIC_BASE_ADR           0x3e0	/* Intel 82365SL */
#define PCIC_INT_LVL            0x0a
#define TCIC_BASE_ADR           0x240	/* Databook DB86082A */
#define TCIC_INT_LVL            0x0a
#define CIS_MEM_START           0xd0000	/* mapping addr for CIS tuple */
#define CIS_MEM_STOP            0xd3fff
#define CIS_REG_START           0xd4000	/* mapping addr for config reg */
#define CIS_REG_STOP            0xd4fff
#define SRAM0_MEM_START		0xd8000	/* mem for SRAM0 */
#define SRAM0_MEM_STOP		0xd8fff
#define SRAM0_MEM_LENGTH	0x100000
#define SRAM1_MEM_START		0xd9000	/* mem for SRAM1 */
#define SRAM1_MEM_STOP		0xd9fff
#define SRAM1_MEM_LENGTH	0x100000
#define SRAM2_MEM_START		0xda000	/* mem for SRAM2 */
#define SRAM2_MEM_STOP		0xdafff
#define SRAM2_MEM_LENGTH	0x100000
#define SRAM3_MEM_START		0xdb000	/* mem for SRAM3 */
#define SRAM3_MEM_STOP		0xdbfff
#define SRAM3_MEM_LENGTH	0x100000
#define ELT0_IO_START		0x260	/* io for ELT0 */
#define ELT0_IO_STOP		0x26f
#define ELT0_INT_LVL		0x05
#define ELT0_NRF		0x00
#define ELT0_CONFIG		0	/* 0=EEPROM 1=AUI  2=BNC  3=RJ45 */
#define ELT1_IO_START		0x280	/* io for ELT1 */
#define ELT1_IO_STOP		0x28f
#define ELT1_INT_LVL		0x09
#define ELT1_NRF		0x00
#define ELT1_CONFIG		0	/* 0=EEPROM 1=AUI  2=BNC  3=RJ45 */

/* parallel port (LPT) */

#define LPT0_BASE_ADRS		0x3bc
#define LPT1_BASE_ADRS		0x378
#define LPT2_BASE_ADRS		0x278
#define LPT_INT_LVL             0x07
#define LPT_CHANNELS		1

/* IDT entry type options */

#define SYS_INT_TRAPGATE 	0x0000ef00 	/* trap gate */
#define SYS_INT_INTGATE  	0x0000ee00 	/* int gate */

/* ISA IO address for sysDelay () */

#define UNUSED_ISA_IO_ADDRESS	0x84

/* FEI PCI bus resources */
/* Note that many of the following FEI defines have no real effect
 * in the operation of the BSP.  In particular, these default values
 * for memory base, io base, and irq values are overwritten in 
 * the feiResources structure.  The final values used in the 
 * structure will be determined by whatever PCI bus initialization 
 * was done.
 */

#define FEI0_MEMBASE0		0xfd000000	/* memory base for CSR */
#define FEI0_MEMSIZE0		0x00001000	/* memory size for CSR, 4KB */
#define FEI0_MEMBASE1		0xfd100000	/* memory base for Flash */
#define FEI0_MEMSIZE1		0x00100000	/* memory size for Flash, 1MB */
#define FEI0_IOBASE0		0xf400		/* IO base for CSR, 32Bytes */
#define FEI0_INT_LVL		0x0b		/* IRQ 11 */
#define FEI0_INIT_STATE_MASK	(VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | \
                                 VM_STATE_MASK_CACHEABLE)
#define FEI0_INIT_STATE		(VM_STATE_VALID | VM_STATE_WRITABLE | \
                                 VM_STATE_CACHEABLE_NOT) 
#define FEI1_MEMBASE0		0xfd200000	/* memory base for CSR */
#define FEI1_MEMSIZE0		0x00001000	/* memory size for CSR, 4KB */
#define FEI1_MEMBASE1		0xfd300000	/* memory base for Flash */
#define FEI1_MEMSIZE1		0x00100000	/* memory size for Flash, 1MB */
#define FEI1_IOBASE0		0xf420		/* IO base for CSR, 32Bytes */
#define FEI1_INT_LVL		0x05		/* IRQ 5 */
#define FEI1_INIT_STATE_MASK    (VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | \
                                 VM_STATE_MASK_CACHEABLE) 
#define FEI1_INIT_STATE         (VM_STATE_VALID | VM_STATE_WRITABLE | \
                                 VM_STATE_CACHEABLE_NOT)
#define FEI2_MEMBASE0		0xfd400000	/* memory base for CSR */
#define FEI2_MEMSIZE0		0x00001000	/* memory size for CSR, 4KB */
#define FEI2_MEMBASE1		0xfd500000	/* memory base for Flash */
#define FEI2_MEMSIZE1		0x00100000	/* memory size for Flash, 1MB */
#define FEI2_IOBASE0		0xf440		/* IO base for CSR, 32Bytes */
#define FEI2_INT_LVL		0x0c		/* IRQ 12 */
#define FEI2_INIT_STATE_MASK    (VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | \
                                 VM_STATE_MASK_CACHEABLE) 
#define FEI2_INIT_STATE         (VM_STATE_VALID | VM_STATE_WRITABLE | \
                                 VM_STATE_CACHEABLE_NOT)
#define FEI3_MEMBASE0		0xfd600000	/* memory base for CSR */
#define FEI3_MEMSIZE0		0x00001000	/* memory size for CSR, 4KB */
#define FEI3_MEMBASE1		0xfd700000	/* memory base for Flash */
#define FEI3_MEMSIZE1		0x00100000	/* memory size for Flash, 1MB */
#define FEI3_IOBASE0		0xf460		/* IO base for CSR, 32Bytes */
#define FEI3_INT_LVL		0x09		/* IRQ 9 */
#define FEI3_INIT_STATE_MASK    (VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | \
                                 VM_STATE_MASK_CACHEABLE) 
#define FEI3_INIT_STATE         (VM_STATE_VALID | VM_STATE_WRITABLE | \
                                 VM_STATE_CACHEABLE_NOT)


/* key board (KBD) */

#define PC_XT_83_KBD		0	/* 83 KEY PC/PCXT/PORTABLE 	*/
#define PC_PS2_101_KBD		1	/* 101 KEY PS/2 		*/
#define KBD_INT_LVL		0x01

#define	COMMAND_8042		0x64
#define	DATA_8042		0x60
#define	STATUS_8042		COMMAND_8042
#define COMMAND_8048		0x61	/* out Port PC 61H in the 8255 PPI */
#define	DATA_8048		0x60	/* input port */
#define	STATUS_8048		COMMAND_8048

#define JAPANES_KBD             0
#define ENGLISH_KBD             1

/* beep generator */

#define DIAG_CTRL	0x61
#define BEEP_PITCH_L	1280 /* 932 Hz */
#define BEEP_PITCH_S	1208 /* 987 Hz */
#define BEEP_TIME_L	(sysClkRateGet () / 3) /* 0.66 sec */
#define BEEP_TIME_S	(sysClkRateGet () / 8) /* 0.15 sec */

/* Monitor definitions */

#define MONOCHROME              0
#define VGA                     1
#define MONO			0
#define COLOR			1
#define	VGA_MEM_BASE		(UCHAR *) 0xb8000
#define	VGA_SEL_REG		(UCHAR *) 0x3d4
#define VGA_VAL_REG             (UCHAR *) 0x3d5
#define MONO_MEM_BASE           (UCHAR *) 0xb0000
#define MONO_SEL_REG            (UCHAR *) 0x3b4
#define MONO_VAL_REG            (UCHAR *) 0x3b5
#define	CHR			2

/* change this to JAPANES_KBD if Japanese enhanced mode wanted */

#define KEYBRD_MODE             ENGLISH_KBD 

/* undefine this if ansi escape sequence not wanted */

#define INCLUDE_ANSI_ESC_SEQUENCE 

#define GRAPH_ADAPTER   VGA

#if (GRAPH_ADAPTER == MONOCHROME)

#define DEFAULT_FG  	ATRB_FG_WHITE
#define DEFAULT_BG 	ATRB_BG_BLACK
#define DEFAULT_ATR     DEFAULT_FG | DEFAULT_BG
#define CTRL_SEL_REG	MONO_SEL_REG		/* controller select reg */
#define CTRL_VAL_REG	MONO_VAL_REG		/* controller value reg */
#define CTRL_MEM_BASE	MONO_MEM_BASE		/* controller memory base */
#define COLOR_MODE	MONO			/* color mode */

#else /* GRAPH_ADAPTER = VGA */

#define DEFAULT_FG	ATRB_FG_BRIGHTWHITE
#define DEFAULT_BG	ATRB_BG_BLUE
#define DEFAULT_ATR	DEFAULT_FG | DEFAULT_BG
#define CTRL_SEL_REG	VGA_SEL_REG		/* controller select reg */
#define CTRL_VAL_REG	VGA_VAL_REG		/* controller value reg */
#define CTRL_MEM_BASE	VGA_MEM_BASE		/* controller memory base */
#define COLOR_MODE	COLOR			/* color mode */

#endif /* (ADAPTER == MONOCHROME) */

#ifndef _ASMLANGUAGE

    /*
     * Shared memory device list
     */

    typedef struct _SYS_SM_DEV_LIST
        {
        UINT devVend;
        UINT subIdVend;
        } SYS_SM_DEV_LIST;
#endif

/*
 * Note: CPV5000/MCPN750 requires a modified software Test and Set algorithm.
 * SM_TAS_TYPE is set to SM_TAS_HARD despite the lack of a hardware TAS
 * mechanism to force the use of a BSP-specific software TAS algorithm. The
 * modified algorithm is required to work around a problem encountered with
 * PCI-to-PCI bridges.  Do NOT change SM_TAS_TYPE, it must be set as 
 * SM_TAS_HARD to work.
 */

#undef SM_TAS_TYPE
#define SM_TAS_TYPE     SM_TAS_HARD

/* Dec 2155x defines  - for shared memory support with MCPN750 */

#define DEC2155X_INTERRUPT_BASE 0x60

#define DEC2155X_DOORBELL0_INT_LVL  ( 0x00 + DEC2155X_INTERRUPT_BASE )
#define DEC2155X_DOORBELL1_INT_LVL  ( 0x01 + DEC2155X_INTERRUPT_BASE )
#define DEC2155X_DOORBELL2_INT_LVL  ( 0x02 + DEC2155X_INTERRUPT_BASE )
#define DEC2155X_DOORBELL3_INT_LVL  ( 0x03 + DEC2155X_INTERRUPT_BASE )
#define DEC2155X_DOORBELL4_INT_LVL  ( 0x04 + DEC2155X_INTERRUPT_BASE )
#define DEC2155X_DOORBELL5_INT_LVL  ( 0x05 + DEC2155X_INTERRUPT_BASE )
#define DEC2155X_DOORBELL6_INT_LVL  ( 0x06 + DEC2155X_INTERRUPT_BASE )
#define DEC2155X_DOORBELL7_INT_LVL  ( 0x07 + DEC2155X_INTERRUPT_BASE )
#define DEC2155X_DOORBELL8_INT_LVL  ( 0x08 + DEC2155X_INTERRUPT_BASE )
#define DEC2155X_DOORBELL9_INT_LVL  ( 0x09 + DEC2155X_INTERRUPT_BASE )
#define DEC2155X_DOORBELL10_INT_LVL ( 0x0a + DEC2155X_INTERRUPT_BASE )
#define DEC2155X_DOORBELL11_INT_LVL ( 0x0b + DEC2155X_INTERRUPT_BASE )
#define DEC2155X_DOORBELL12_INT_LVL ( 0x0c + DEC2155X_INTERRUPT_BASE )
#define DEC2155X_DOORBELL13_INT_LVL ( 0x0d + DEC2155X_INTERRUPT_BASE )
#define DEC2155X_DOORBELL14_INT_LVL ( 0x0e + DEC2155X_INTERRUPT_BASE )
#define DEC2155X_DOORBELL15_INT_LVL ( 0x0f + DEC2155X_INTERRUPT_BASE )
#define DEC2155X_PWR_MGMT_INT_LVL   ( 0x10 + DEC2155X_INTERRUPT_BASE )
#define DEC2155X_I2O_INT_LVL        ( 0x11 + DEC2155X_INTERRUPT_BASE )
#define DEC2155X_PG_CRSSNG_INT_LVL  ( 0x12 + DEC2155X_INTERRUPT_BASE )

#define DEC2155X_SM_DOORBELL_INT_LVL ( DEC2155X_SM_DOORBELL_BIT + \
				       DEC2155X_INTERRUPT_BASE )

#define DEC2155X_DOORBELL0_INT_VEC  ( DEC2155X_DOORBELL0_INT_LVL )
#define DEC2155X_DOORBELL1_INT_VEC  ( DEC2155X_DOORBELL1_INT_LVL )
#define DEC2155X_DOORBELL2_INT_VEC  ( DEC2155X_DOORBELL2_INT_LVL )
#define DEC2155X_DOORBELL3_INT_VEC  ( DEC2155X_DOORBELL3_INT_LVL )
#define DEC2155X_DOORBELL4_INT_VEC  ( DEC2155X_DOORBELL4_INT_LVL )
#define DEC2155X_DOORBELL5_INT_VEC  ( DEC2155X_DOORBELL5_INT_LVL )
#define DEC2155X_DOORBELL6_INT_VEC  ( DEC2155X_DOORBELL6_INT_LVL )
#define DEC2155X_DOORBELL7_INT_VEC  ( DEC2155X_DOORBELL7_INT_LVL )
#define DEC2155X_DOORBELL8_INT_VEC  ( DEC2155X_DOORBELL8_INT_LVL )
#define DEC2155X_DOORBELL9_INT_VEC  ( DEC2155X_DOORBELL9_INT_LVL )
#define DEC2155X_DOORBELL10_INT_VEC ( DEC2155X_DOORBELL10_INT_LVL )
#define DEC2155X_DOORBELL11_INT_VEC ( DEC2155X_DOORBELL11_INT_LVL )
#define DEC2155X_DOORBELL12_INT_VEC ( DEC2155X_DOORBELL12_INT_LVL )
#define DEC2155X_DOORBELL13_INT_VEC ( DEC2155X_DOORBELL13_INT_LVL )
#define DEC2155X_DOORBELL14_INT_VEC ( DEC2155X_DOORBELL14_INT_LVL )
#define DEC2155X_DOORBELL15_INT_VEC ( DEC2155X_DOORBELL15_INT_LVL )
#define DEC2155X_PWR_MGMT_INT_VEC   ( DEC2155X_PWR_MGMT_INT_LVL )
#define DEC2155X_I2O_INT_VEC        ( DEC2155X_I2O_INT_LVL )
#define DEC2155X_PG_CRSSNG_INT_VEC  ( DEC2155X_PG_CRSSNG_INT_LVL )
#define DEC2155X_MAILBOX_INT_VEC ( ( DEC2155X_DOORBELL0_INT_VEC + \
                                     DEC2155X_SM_DOORBELL_BIT ) )

#define DEC2155X_SM_DOORBELL_INT_VEC ( DEC2155X_SM_DOORBELL_INT_LVL )


/* support for NVRAM read/write on CPV5350 */

#if defined(CPV5350)
#define NV_RAM_SIZE		0x20000 /* 128K NVRAM on CPV5350 */

#ifdef NV_BOOT_OFFSET			/* set in configAll.h */
#undef NV_BOOT_OFFSET
#endif
#define NV_BOOT_OFFSET  	0x0	/* Offset to 1st available byte */

/* These machinations are needed for Validation Test Suite */
#define NV_RAM_SIZE_WRITEABLE ( NV_RAM_SIZE - NV_BOOT_OFFSET )
#if (NV_RAM_SIZE == NV_BOOT_OFFSET)
#undef NV_RAM_SIZE
#define NV_RAM_SIZE NONE
#endif

/*
 * 32K window, inside the BIOS Option Expansion Area, where
 * the 128K of NVRAM is visible.
 */

#define NV_RAM_ADDRS		((UINT8 *)0xD0000)

/* Each NVRAM bank is 32K in size. */

#define NV_RAM_BANK_SIZE	0x8000

/* 
 * The following procedural macros isolate the generic VxWorks NVRAM
 * routines from the board-specific routines. 
 */
                                                                     
#define  NV_RAM_READ(x)    sysNvReadByte(x)
#define  NV_RAM_WRITE(x,y) sysNvWriteByte (x,y)
#define  NV_RAM_WR_ENBL    sysNvEnable() 	/* Enabble R/W access */
#define  NV_RAM_WR_DSBL    sysNvDisable() 	/* Enabble R/W access */
#define  NV_RAM_RD_ENBL    sysNvEnable() 	/* Enabble R/W access */
#define  NV_RAM_RD_DSBL    sysNvDisable() 	/* Enabble R/W access */

#else

/* support for NVRAM read/write on CPV5000 */

/*
 * The CMOS block, which consists of the RTC register set and the 
 * NVRAM, are divided as follows on the CPV5000:
 * bytes  0 - 13: RTC registers
 * bytes 13 - 127: BIOS use
 * bytes 128 - 255: Duplicated Map of the 1st 128 bytes. Do Not Use!
 */
   
/* 
 * NV_RAM_SIZE: total number of NVRAM bytes
 * The BIOS's portion of the NVRAM is not 
 * considered useable NVRAM by sysNvRamGet/sysNvRamSet.
 * If NV_BOOT_OFFSET and NV_RAM_SIZE are equal, there is NO available
 * non-volatile storage for the boot line (or anything else).
 */
#define NV_RAM_SIZE		0x80 

#ifdef NV_BOOT_OFFSET  /* set in configAll.h */
#undef NV_BOOT_OFFSET
#endif
#define NV_BOOT_OFFSET  	0x80  /* Offset to 1st available byte */

/* These machinations are needed for Validation Test Suite */
#define NV_RAM_SIZE_WRITEABLE ( NV_RAM_SIZE - NV_BOOT_OFFSET )
#if (NV_RAM_SIZE == NV_BOOT_OFFSET)
#undef NV_RAM_SIZE
#define NV_RAM_SIZE NONE
#endif


/* 
 * The following procedural macros isolate the generic VxWorks NVRAM
 * routines from the board-specific routines. 
 */
                                                                     
#define  NV_RAM_READ(x)    sysNvRead(x)
#define  NV_RAM_WRITE(x,y) sysNvWrite (x,y)
#define  NV_RAM_WR_ENBL    /* No Op for SMsC Ultra I/O device */
#define  NV_RAM_WR_DSBL    /* No Op for SMsC Ultra I/O device */
                                                                     
#endif /* CPV5350 */
                                                                     
/*
 * To access a given byte of data in the CMOS block,
 * (which includes the RTC access area) one must put the offset
 * of the CMOS data byte into the RTC_INDEX port, then 
 * read/write the data from/to the RTC_DATA port.
 */

/* write the addr select */
#define CMOS_A(desired_offset) (sysOutByte(NVRAM_INDEX,desired_offset))

/* write the data */
#define CMOS_D_W(write_data) (sysOutByte(NVRAM_DATA,write_data))

/* read the data */
#define CMOS_D_R (sysInByte(NVRAM_DATA))

/* write data to offset in the CMOS ISA block */
#define CMOS_W(offset, data) { CMOS_A(offset); CMOS_D_W(data); }


#ifdef __cplusplus
}
#endif

#endif /* __INCcpv5xxxh */

