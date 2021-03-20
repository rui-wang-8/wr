/* cpn5360.h - CPN5360 header */

/* Copyright 1999-2002 Wind River Systems, Inc. All Rights Reserved */
/* Copyright 1999-2000 Motorola, Inc. All Rights Reserved */

/*
modification history
--------------------
01k,30apr02,dat  Update for T2.2, added sysHwInit0, chg'd ideDrv to ataDrv.
01j,19jun00,scb  Align PIRQx routing #defines to mimic default BIOS values.
01i,06jun00,scb  Add PIRQx routing #defines.
01h,09may00,djs  incorporate WRS review comments.
01g,20mar00,djs  Initial code review changes.
01f,11feb00,dmw  Added PIIX 4 reset control register.
01e,10feb00,dmw  Added PIIX 4 registers, Intel i28f320 ID and _MOT_ sig.
01d,09feb00,scb  Add slot control #defines.
01c,08feb00,dmw  moved BIOS_PROM_BASE here, added FPGA defines.
01b,16dec99,scb  support for dec21554 initialization
01a,08nov99,djs  written based on cpv5300 01s,14oct99,sjb
*/

/*
This file contains IO address and related constants for the
CPN5360
*/

#ifndef	__INCcpn5360h
#define	__INCcpn5360h

#include "drv/intrCtl/i8259.h"
#include "drv/timer/i8253.h"
#include "drv/timer/mc146818.h"
#include "drv/timer/timerDev.h"
#include "drv/timer/timestampDev.h"
#include "drv/hdisk/ataDrv.h"
#include "drv/fdisk/nec765Fd.h"
#include "drv/serial/pcConsole.h"
#include "drv/parallel/lptDrv.h"
#include "drv/hdisk/ataDrv.h"
#include "arch/i86/mmuI86Lib.h"
#include "drv/pci/pciConfigLib.h"

#undef	CAST
#define	CAST

#if defined(CPN5360)

    /*
     * PIRQA - PIRQD are the IRQ values which will be programmed into
     * the PCI interrupt routing control register to correspond to the
     * four different PCI interrupts.  The IRQ number must be in the 
     * range from 0 through 15 and must associated with an edge triggered
     * high interrupt.
     */

#   define PIRQA	0x0B
#   define PIRQB	0x05
#   define PIRQC	0x0A
#   define PIRQD	0x09

#   define SYS_SM_CPCI_BUS_NUMBER    2		/* Compact PCI bus number */
#   define PCI_ID_82371AB	0x71108086	/* Id for 82443BX PIIX4	*/
#   define PCI_ID_82443BX	0x71908086	/* CPU to PCI bridge */

#   define PCI_DEVID_HB		PCI_ID_82443BX
#   define PCI_DEVID_ISA	PCI_ID_82371AB
#   define PCI_DEVID_IDE	0x71118086
#   define PCI_DEVID_USB	0x71128086
#   define PCI_DEVID_VIDEO	0x00C0102C

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
#endif /* CPN5360 */

/* Pentium needs sysHwInit0 for T2.2 */

#define INCLUDE_SYS_HW_INIT_0
#define SYS_HW_INIT_0()         (sysHwInit0())

/* PIIX4 BIOS r/w enable defines */

#define X_BUS_CS_REG	0x4E	/* x-bus chip select register */
#define BIOSC_WP_ENABLE	0x04	/* BIOS chip select write enable */
#define LOWER_BIOS_EN	0x40	/* lower BIOS enable */
#define UPPER_BIOS_EN	0x80	/* extended BIOS enable */

/* PIIX4 Reset Control Register Defines */

#define PIIX4_RC	0xCF9   /* reset control register offset */
#define PIIX4_RC_RCPU	0x04	/* reset initiate bit */
#define PIIX4_RC_SRST	0x02	/* reset type bit 1 = hard */

/* Intel i28f320 StrataFlash defines */

#define INTEL_MANUFACTID	0x89
#define I28F320PARTID		0x14

/* DEC2155x defines */

/* Bus and device number */

#define DEC2155X_PCI_DEV_NUMBER PCI2PCI_DEV
#define DEC2155X_PCI_BUS_NUMBER PCI2PCI_BUS

/* MMU related */

#define DEC2155X_INIT_STATE_MASK  VM_STATE_MASK_FOR_ALL
#define DEC2155X_INIT_STATE       VM_STATE_FOR_PCI

/* Dec 2155x defines  - for shared memory support */

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

#ifdef INCLUDE_DEC2155X

#   define DEC2155X_BIST_VAL 		0x00
#   define DEC2155X_PRI_PRG_IF_VAL 	0x00
#   define DEC2155X_PRI_SUBCLASS_VAL 	0x02	/* Pentium */
#   define DEC2155X_PRI_CLASS_VAL 	0x0b	/* Processor */
#   define DEC2155X_SEC_PRG_IF_VAL 	0x00
#   define DEC2155X_SEC_SUBCLASS_VAL 	0x80
#   define DEC2155X_SEC_CLASS_VAL 	0x06
#   define DEC2155X_MAX_LAT_VAL 	0x00
#   define DEC2155X_MIN_GNT_VAL 	0xff

#   define DEC2155X_CHP_CTRL0_VAL 	0x0000
#   define DEC2155X_CHP_CTRL1_VAL 	0x0000

#   define DEC2155X_PRI_SERR_VAL (DEC2155X_SERR_DIS_DLYD_TRNS_MSTR_ABRT | \
                               DEC2155X_SERR_DIS_DLYD_RD_TRNS_TO | \
                               DEC2155X_SERR_DIS_DLYD_WRT_TRNS_DISC | \
                               DEC2155X_SERR_DIS_PSTD_WRT_DATA_DISC | \
                               DEC2155X_SERR_DIS_PSTD_WRT_TRGT_ABRT | \
                               DEC2155X_SERR_DIS_PSTD_WRT_MSTR_ABRT | \
                               DEC2155X_SERR_DIS_PSTD_WRT_PAR_ERROR)

#   define DEC2155X_SEC_SERR_VAL (DEC2155X_SERR_DIS_DLYD_TRNS_MSTR_ABRT | \
                               DEC2155X_SERR_DIS_DLYD_RD_TRNS_TO | \
                               DEC2155X_SERR_DIS_DLYD_WRT_TRNS_DISC | \
                               DEC2155X_SERR_DIS_PSTD_WRT_DATA_DISC | \
                               DEC2155X_SERR_DIS_PSTD_WRT_TRGT_ABRT | \
                               DEC2155X_SERR_DIS_PSTD_WRT_MSTR_ABRT | \
                               DEC2155X_SERR_DIS_PSTD_WRT_PAR_ERROR)

/* 
 * Note:  dec2155xCpciInit() allows for setup from SROM or vxWorks
 * setup (initialization) of the dec2155x drawbridge chip.  The cpn5360
 * only allows for SROM setup of the chip, thus the following define
 * is here rather than in "config.h" where the user might be tempted
 * to #undef it.
 */

#    define DEC2155X_SETUP_FROM_SROM

#endif /* INCLUDE_DEC2155X */


#define TARGET_PC386

#ifndef BUS_TYPE_PCI
#    define BUS_TYPE_PCI      3
#endif /* BUS_TYPE_PCI */

#define BUS		BUS_TYPE_PCI

#define INT_NUM_IRQ0	0x20		/* vector number for IRQ0 */

#define	BOOTCODE_IN_RAM			/* for ../all/bootInit.c */

/*
 * The following parameters are defined here and in the Makefile.
 * The must be kept synchronized; effectively config.h depends on Makefile.
 * Any changes made here must be made in the Makefile and vice versa.
 */

#define ROM_BASE_ADRS    0x00008000      /* base address of ROM */
#define ROM_TEXT_ADRS    (ROM_BASE_ADRS) /* booting from A: or C: */
#define ROM_SIZE         0x00090000      /* ROM size */

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

/* Address and size of the flash bank(s) */

#define BIOS_PROM_BASE          0xFFF80000      /* ROM base address */
#define BIOS_PROM_SIZE          0x00080000      /* ROM size */

#define ROM_SIGNATURE_SIZE	5
#define ROM_SIGNATURE		"_MOT_"

/* cpn5360's FPGA device defines */

#define FPGA_ISA_BASE_ADRS      (0x58)
#define FPGA_INDEX	        (FPGA_ISA_BASE_ADRS + 0x05) /* index */
#define FPGA_DATA               (FPGA_ISA_BASE_ADRS + 0x07) /*data */

#define FPGA_DEVICE_SELECT      (0x0F)  /* select command */
#define FPGA_FLASH_DEVICE       (0x14)	/* flash device's indicie */
#define FPGA_FLASH_CONTROL      (0x01)	/* flash control register */

#define FPGA_SLOT_CTRL_DEVICE   (0x15)  /* Slot control device index */
#define FPGA_SLOT_CTRL_PORT  	(0x01)  /* Slot control port */
#define SLOT_CTRL_PORT_INTIN 	(1<<5)  /* cPCI interrupt enable */


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

/* PCI Access to local memory space */

/* 
 * PCI_SLV_MEM_BUS is equal to the base  of local DRAM as seen from 
 * the local pci bus (secondary side of the Dec2155x).
 */

#define PCI_SLV_MEM_BUS         0x00000000


/* Latency Timer value - 255 PCI clocks */
 
#define PCI_LAT_TIMER		0xff

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

/* PCI I/O function defines */

#ifndef PCI_INSERT_LONG
#   define PCI_INSERT_LONG(a,m,d) sysPciInsertLong((a),(m),(d))
#endif  /* PCI_INSERT_LONG */

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
     * Shared memory anchor polling list
     */

    typedef struct _SYS_SM_ANCHOR_POLLING_LIST
        {
        UINT devVend;
        UINT subIdVend;
        } SYS_SM_ANCHOR_POLLING_LIST;
#endif /* _ASMLANGUAGE */

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
#    undef NV_BOOT_OFFSET
#endif /* NV_BOOT_OFFSET */
#define NV_BOOT_OFFSET  	0x80  /* Offset to 1st available byte */

/* These machinations are needed for Validation Test Suite */
#define NV_RAM_SIZE_WRITEABLE ( NV_RAM_SIZE - NV_BOOT_OFFSET )
#if (NV_RAM_SIZE == NV_BOOT_OFFSET)
#    undef NV_RAM_SIZE
#    define NV_RAM_SIZE NONE
#endif /* NV_RAM_SIZE == NV_BOOT_OFFSET */


/* 
 * The following procedural macros isolate the generic VxWorks NVRAM
 * routines from the board-specific routines. 
 */
                                                                     
#define  NV_RAM_READ(x)    sysNvRead(x)
#define  NV_RAM_WRITE(x,y) sysNvWrite (x,y)
#define  NV_RAM_WR_ENBL    /* No Op for SMsC Ultra I/O device */
#define  NV_RAM_WR_DSBL    /* No Op for SMsC Ultra I/O device */
                                                                     

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

#endif /* __INCcpn5360h */

