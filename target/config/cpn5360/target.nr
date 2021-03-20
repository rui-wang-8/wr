'\" t
.so wrs.an
.\" cpn5360/target.nr - CPN5360 target-specific documentation
.\"
.\" Copyright 1984-2002 Wind River Systems, Inc.
.\" Copyright 1999, 2000 Motorola, Inc. All Rights Reserved
.\"
.\" modification history
.\" --------------------
.\" 01q,30apr02,dat  Update for T2.2, use binary images for flash booting.
.\" 01p,24aug01,dgp  changed manual entry to reference entry per SPR 23698
.\" 01o,06jun00,scb  Add information regarding PIRQ interrupt routing register.
.\" 01n,09may00,djs  incorporate WRS review comments.
.\" 01m,29mar00,dmw  Removed statement for mkbootFlash5360 options.
.\" 01l,28mar00,scb  Add RTC support (from cpv5000 base).
.\" 01k,20mar00,djs  Initial code review changes.
.\" 01j,09mar00,dmw  Add non-support statement for mkbootFlash5360 options.
.\" 01i,08mar00,sjb  Add missing "/" in 4th line, above.
.\" 01h,06mar00,sjb  General updates prior to cpn5360 EA release.
.\" 01g,16feb00,dmw  Added section on ROM booting.
.\" 01f,14feb00,djs  Removed references to unsupported images.
.\" 01e,11feb00,sjb  Added ataDevCreate warning, and ata boot device notes.
.\" 01d,07feb00,scb  Cleaned up shared memory writeup.
.\" 01c,14jan00,sjb  Modified ATA/IDE driver text for IDE Simplification chg.
.\" 01b,09dec99,dmw  Added section regarding DEC2155x SROM.
.\" 01a,27oct99,djs  Copied from CPV5300 pre-release base
.\"
.\" NOTE
.\" EXOS
.\"
.TH "cpn5360" T "cpn5360" "Rev: 25 Feb 00" "TORNADO REFERENCE: VXWORKS"

.SH "NAME"
.aX "Motorola CPN5360"

.SH "INTRODUCTION"
This reference entry provides board-specific information necessary 
to run VxWorks on the Motorola CPN5360. If this is the first
time that VxWorks is being booted on the CPN5360, please use
the delivered vxWorks.st and bootrom images to verify the 
board's operation. Please review sections "Bios Setup" and 
"Boot Issues" before continuing.

.SS "Bios Setup"
Prior to running VxWorks on the CPN5360, several of the 
Phoenix BIOS settings need to be modified.
Before powering on the CPN5360, plug a video monitor, 
PC compatible keyboard, and PS/2 mouse into the CPN5360 Transition
Module. 
Please refer to the "CPN5360 CompactPCI Single Board Computer and
Transition Module Installation and Reference Guide
CPN5360A/IH1" for details.

To enter the BIOS system setup, press the <F2> key
during the power on BIOS memory test. The Setup main 
menu should be displayed.

Select the "Advanced" menu.  Make sure that the "Plug and Play Aware
OS" is set to NO and that the "Reset Configuration Data" item is set
to YES.

Also under the "Advanced" menu set "Legacy USB Support" to disabled.
To get to the "Legacy USB Support" option, use the arrow keys to
scroll down.

On the same "Advanced" menu, open the "PCI configuration" item to view
the associated submenu.  On this submenu open the "PCI/PNP ISA IRQ
Resource Exclusion" submenu.  Here you will find IRQ 9 and IRQ 11
items.  Make sure they are set to "available" which will allow the
BIOS to assign them to PCI interrupts.

The basic BIOS setup is now complete.  If you are planning to use
VxWorks shared memory networking to communicate across the cPCI
backplane however, there are other BIOS setup considerations which are
outlined in this document under "BIOS and Dec2155x SROM considerations
with shared memory".

Once all of the BIOS parameters are modified, save and exit the BIOS.

A floppy device will be required to boot the board for VxWorks for the
first time.  Once booted, the user can then use the mkbootFlash5360
program to burn a boot program into flash. (Hint: burn two banks of flash
and use one for a backup copy).

.SS "Boot Floppy"
The boot floppy is a formatted, double-sided, 3.5" high density
diskette.
The boot floppy includes the boot sector (sector 0) and a DOS file system
containing a boot ROM image named "bootrom.sys".
The boot floppy image is one of the following:

.TS E
expand;
		Image is	Image Loads
Image Name	Description	Compressed	Into
=
vxWorks.st_rom.bin	bootable vxWorks.st	Yes	High Memory
bootrom.bin	bootrom	Yes	Low Memory
bootrom_uncmp.bin	bootrom	No	Low Memory
_
.TE

For more information, refer to the "Boot Devices", "Boot ROMs",
"Boot Issues", and "Boot Methods" sections of the HARDWARE
DETAILS section.

.SS "Jumpers"
Refer to the "CPN5360 CompactPCI Single Board Computer and Transition 
Module Installation and Reference Guide CPN5360A/IH1" for information
on jumper settings..

.SH "FEATURES"
The following subsections list all supported and unsupported
features, as well as any feature interaction.

.SS "Supported Features"
The following features of the CPN5360 are supported:

.TS E
expand;
Pentium II (266/333 Mhz)
Pentium III (500 Mhz)
L1 and L2 cache support
440BX host bridge
Up to 256MB on board memory 
Non-System Slot Configuration
Dual 10/100Mb Intel 82559 Ethernet
2 IDE channels, up to 4 IDE devices
2 PMC slots
DEC21554 non-transparent PCI-PCI Bridge
2 16650 serial ports (COM1 and COM2)
Keyboard & Mouse
Parallel Port
3.5" 1.44MB Floppy disk
ATA Compact Flash Chipset
On-Card ATA Hard drive
Watchdog (failsafe) timer
Real Time Clock Timekeeping and Alarms
LM78 System Monitor
PCI Auto Configuration on Local PCI Bus
VxWorks Shared Memory
Programming of Intel i28f320 StrataFlash for BIOS-supported ROM boot
VGA 
PCI Interface: 32-bit address, 32-bit data; complies with \f2PCI Local Bus Specification\f1, Revision 2.1
.TE

.ne 2i
.SS "Unsupported Features"
The following features of the CPN5360 are unsupported or not tested:
.TS E
expand;
System Slot Configuration
Universal Serial Bus
Hot Swap
AGP Video
Power Management
ECC
Vital Product Data (VPD SROM)
PMC I/O Module (PIM) connector on the CPN5360 transition module
DMA
X-Bus Peripheral Support
.TE

.SS "Feature Interactions"
None known.

.SH "HARDWARE DETAILS"
This section documents the details of the device drivers and board
hardware elements.

.SS "Device Drivers"
Drivers included with this BSP are for on-board chips.
Refer to the vendor's documentation for information regarding 
the on-board devices.

In the table below, drivers ending in the ".c" extension are
delivered in source form; the other drivers are delivered
in object form only.
    
.TS E
expand;
lf3 lf3
lw13 lw(3.7i) .
.sp .5
Driver	Description
_
"i8250Sio.c"	T{
Intel 8250 Universal Asynchronous Receiver Transmitter (UART) tty driver.
Used for the serial ports
T}
"i8237Dma.c"	T{
Driver for the ISA DMA controller.  This is used in "nec765Fd.c", which also
serves as a good application example.
T}
"pcConsole.c"	T{
T}
"i8042Kbd.c"	T{
T}
"i8048Kbd.c"	T{
Driver for the on-board Intel 8042 and 8048 keyboard controllers.
To use these controllers the INCLUDE_PC_CONSOLE directive must be 
#define'd in "config.h". The macro PC_KBD_TYPE should be defined in 
"config.h" as PC_PS2_101_KBD to include "i8042Kbd.c", and as 
PC_XT_83_KBD to include "i8048Kbd.c".
T}
"m6845Vga.c"	T{
Driver for the Motorola M6845 VGA controller.
To use this controller, define INCLUDE_PC_CONSOLE in "config.h".
T}
"nec765Fd.c"	T{
Driver for the nec765 floppy disk controller.
To use this driver, the INCLUDE_FD directive must be defined in
"config.h".
T}
"ataDrv.c"	T{
Driver for the IDE/ATA hard disk controller.  
To use this driver, the INCLUDE_ATA directive must be defined in "config.h".
Additionally, the ATA_DEVn_STATE defines in "config.h" provide the 
ability to select which IDE/ATA devices are used by the ATA driver.
Note that the old INCLUDE_IDE directive is replaced by INCLUDE_ATA,
and that vxsys() is replaced by mkbootFd() and mkbootAta().
Note also that a new version of mkboot.obj  is included in the BSP
directory of this release, and should be used for mkbootAta
functionality.
T}
"i8253Timer.c"	T{
This library contains routines to manipulate the timer functions on the
Intel 8253 chip with a board-independent interface.  This library handles
both the system clock and the auxiliary clock functions.  It also
contains code for the TimeStamp Counter in the Pentium Processor. 
It also contains code for the periodic interrupt of the CMOS Real 
Time Clock (RTC) device.

The macros SYS_CLK_RATE_MIN, SYS_CLK_RATE_MAX, AUX_CLK_RATE_MIN, and
AUX_CLK_RATE_MAX must be defined to provide parameter checking for the
sys[Aux]ClkRateSet() routines.

The macro PIT_CLOCK must also be defined to indicate the clock frequency
of the i8253.

This driver includes a timestamp driver; to use this
feature, the macro INCLUDE_TIMESTAMP must be defined in "config.h".

The system clock is the first programmable interrupt timer in the 8253 
(PIT0).  The Auxiliary clock is generated using the Periodic Interrupt 
of the CMOS Real Time Clock device by undefining PIT1_FOR_AUX.
T}
"i8259Pic.c"	T{
Driver for the Intel 8259A Programmable Interrupt Controller (PIC).
T}
"byteNvRam.c"	T{
This is an NVRAM driver which may be used to read and write
a byte-wide non volatile memory device.  This driver is isolated
from the hardware specifics thru the definition of macros
found in cpn5360.h.  Note that the CPN5360 board has no useable 
NVRAM, but the CMOS device, also known as the Real Time Clock 
device, needs the NVRAM driver for its chip access.

The macro NV_RAM_SIZE should be defined as NONE for targets lacking
non-volatile RAM.
T}
"fei82557End.o"	T{
Generic driver for the Intel 82557 Ethernet network interface chip.
To use this driver the INCLUDE_FEI directive must be defined in "config.h".
On the CPN5360, the INCLUDE_END directive must also be defined in 
"config.h". (see Known Problems)
T}
"lm78.c"	T{
Support routines for the LM78 system monitor.  To use these support
routines, INCLUDE_LM78 must be defined in "config.h".  If you wish
to have lm78Init() called from the boot module, you must have
INCLUDE_LM78 defined in the build of the boot image.
T}
"failsafe.c"	T{
Support routines for the hardware watchdog timer.  To use these support
routines, INCLUDE_FAILSAFE must be defined in "config.h".  
T}
"dec2155xSrom.c"	T{
Support routines to read and write the DEC2155x PCI-PCI drawbridge.
To use this support, the object dec2155xSrom.o needs to be added to the
MACH_EXTRA line in the Makefile.  
T}
"mkbootFlash5360.c, copyRom.s"	T{
Support routines, modeled after mkbootAta, to allow the user to program
(flash) a VxWorks image into any of the seven available flash banks 
of the onboard ROM flash parts.  To use these routines, add 
mkbootFlash5360.o and copyRom.o to the MACH_EXTRA list in the Makefile.
Or you can download the prebuilt copyRom.obj and mkbootFlash5360.obj
modules included with the BSP.  (dowload copyRom.obj first).
T}
.TE

.ne 4i
.SS "Memory Maps"

The following table illustrates PCI memory areas and how they
relate to upper and lower memory.
.CS

Start addr               Size                       Access to
__________________________________________________________________________
LOCAL_MEM_LOCAL_ADRS     0xa0000                    lower memory
 (default = 0)

0xa0000                  0x60000                    video ram, etc.

0x100000                 LOCAL_MEM_SIZE - 0x100000  upper memory

PCI_MSTR_MEMIO_BUS       PCI_MSTR_MEMIO_SIZE        32-bit Nonprefetchable
 (default = 0x40000000)   (default = 0x10000000)     PCI memory

PCI_MSTR_MEM_BUS         PCI_MSTR_MEM_SIZE          32-bit Prefetchable
 (default = 0x41400000)   (default = 0x00800000)     PCI memory

ISA_MSTR_IO_BUS          ISA_MSTR_IO_SIZE           16-bit ISA I/O
 (default = 0x00001000)   (default = 0x00006000)

PCI_MSTR_IO_BUS          PCI_MSTR_IO_SIZE           32-bit PCI I/O
 (default = 0x00008000)   (default = 0x00008000)

.CE

Note: 
.sp 0
The PCI_MSTR_MEMIO_SIZE is large enough for the CPN5360 to
successfully operate in a single chassis shared-memory system with a
total of seven slave boards and a cPCI master board.  The PCI memory
spaces above apply only when VxWorks performs the PCI bus
initialization (INCLUDE_PCI_AUTOCONF is #define'd in "config.h").  If
BIOS performs the PCI configuration (INCLUDE_PCI_AUTOCONF is
#undef'ed) then the PCI memory spaces are positioned at different
addresses that are dynamically determined by the BIOS, and the PCI
related #define's described above have no effect.  When
INCLUDE_PCI_AUTOCONF is #define'd in "config.h", PCI memory space can
be configured in the range from 0x40000000 to a maximum value of
0xFEDFFFFFF by altering PCI_MSTR_MEMIO_SIZE and PCI_MSTR_MEM_SIZE.

On-board RAM for these boards always appears at address 0x0 locally.
 
Dynamic memory sizing is supported.  By default, LOCAL_MEM_AUTOSIZE is
defined so memory is auto-sized at hardware initialization time.
If auto-sizing is not selected, LOCAL_MEM_SIZE must be set to the actual size
of DRAM memory available on the board to ensure all memory is available.
The default fixed RAM size is set to 16MB (see LOCAL_MEM_SIZE in "config.h").

The VxWorks System Memory Layout differs slightly from the layout
given in the Wind River Programmer's Guide.  In particular RAM_LOW_ADRS
which represents the start of the system image in memory has been changed
from its old value of 0x108000 to 0x118000.  This change was made to
allow a shared memory pool to begin at 0x100000.  Below is the correct VxWorks 
System Memory Layout (x86 Upper Memory):

.ne 55
.CS
                                          Address
      +---------------------------------+  +0x0000 + LOCAL_MEM_LOCAL_ADRS
      |                                 |
      |     Interrupt Vector Table      |
      |            (2KB)                |
      +---------------------------------+  +0x800
      |                                 |
      |             GDT                 |
      |                                 |
      +---------------------------------+ + 0x1100
      |           SM Anchor             |
      +---------------------------------+ + 0x1200
      |           Boot Line             |
      +---------------------------------+ + 0x1300
      |        Exception Message        |
      |                                 |
      +---------------------------------+ + 0x2000
      |                                 |
      |          FD DMA Area            |
      |                                 |
      +---------------------------------+ + 0x5000
      |                                 |
      |         Reserved Memory         |
      |                                 |
      +---------------------------------+ + 0xA000
      |          (no memory)            |
      +---------------------------------+ + 0x100000
      |                                 |
      |         Shared Memory           |
      |             Pool                |
      |                                 |
      +---------------------------------+ + 0x110000
      |                                 |
      |         Initial Stack           |
      |                                 |
      +---------------------------------+ + 0x118000
      |                                 |
      |          System Image           |
      |                                 |
      |                                 |
      +---------------------------------+ _end
      |         WDB Memory Pool         |
      |          (available)            |
      +---------------------------------+
      |         Interrupt Stack         |
      +---------------------------------+
      |                                 |
      |       System Memory Pool        |
      |          (available)            |
      |                                 |
      +---------------------------------+ sysMemTop()
.CE

.SS "Interrupts"
All interrupts are external to the CPU and are routed
through the interrupt prioritization hardware.  This hardware is
comprised of two 82C59 PICs.  There are 16 interrupts and
interrupt priority levels numbered 0 through 15. The mapping between
interrupt numbers and priority levels is not necessarily one to one.
The hardware determines the mapping of interrupt
request lines (IRQ) to priority levels. The hardware should
adhere to the standard assignments:

.ne 18 
            IRQ         Priority
            ---         --------
             0              0
             1              1
             2              2
             3             11
             4             12
             5             13
             6             14
             7             15
             8              3
             9              4
            10              5
            11              6
            12              7
            13              8
            14              9
            15             10

IRQs 0 - 7 are handled by PIC1 and IRQs 8 - 15 by PIC2.  PIC2 interrupts are
cascaded into PIC1 at IRQ2 which is reflected in the above table.

The IRQ numbers to which the four PCI interrupts (PIRQA, PIRQB, PIRQC,
PIRQD) are routed is determined by the PIRQx Route Control register,
part of the configuration space of the PIIX4 chip.  The register
appears at configuration offset 0x60, for bus 0, device 0x7, function
0 and can be read with the VxWorks pciConfigInLong() function.  The
BIOS will program this register to default values which will be used
if the "config.h" parameter INCLUDE_PCI_AUTOCONF is #undef'ined.  By
#define'ing INCLUDE_PCI_AUTOCONF, the Route Control Register will be
overwritten at VxWorks initialization time by the values PIRQ[A-D]
#define'ed in "cpn5360.h".  The values used in "cpn5360.h" represent
either IRQs which have not been preassigned by hardware or those which
have been reserved by hardware strictly for PIRQ use.  For the
cpn5360, the PIRQx values assigned are safe in that they can be shared
by more than one device on the same IRQ.  

Even though it is possible to configure and run VxWorks with
INCLUDE_PCI_AUTOCONF #undef'ined, the default setting is for this
parameter to be #define'd and this is the recommended mode of operation.

Certain BIOS/cpn5360 combinations can produce default programming of
the PIRQx register which is incompatible with VxWorks.  For example,
the secondary ATA controller is assigned to IRQ 0xf and cannot be
shared with any other device.  If the BIOS programs one of the bytes
in the PIRQx register with 0x0f then the routing of the associated PCI
interrupt will conflict with the ATA secondary controller.  If this is
the case, rebuilding the bootrom and kernel with INCLUDE_PCI_AUTOCONF
#define'd will force the use of the "cpn5360.h" PIRQ[A-D] values for
the PIRQx routing register and thus avoid the interrupt assignment
conflict initiated by the BIOS.

It should not be necessary to change the PIRQ[A-D] values in
"cpn5360.h" for a system to properly function.  In some unusual cases,
however, it may be desirable to reassign some of the PIRQ[A-D]
#define's for performance enhancement.  Be aware of the potential
conflicts with VxWorks if this is done, however.  It is possible to
assign duplicate IRQ numeric values to two or more of the PIRQ[A-D]
#define's if the values are selected from the original set of four.

.SS "Serial Configuration"
To use a COM port as the console, undef INCLUDE_PC_CONSOLE in "config.h". 
Define CONSOLE_TTY as 0, to use COM1 as the console or define 
CONSOLE_TTY as 1, to use COM2 as the console.  The serial connection
can be made through the front panel or through the rear IO of the
transition module.

.SS "Parallel Port Configuration"
The LPT_RESOURCE structure in "sysLib.c" is set up to work with the parallel
port on the CPN5360.  The VxWorks 5.4 Programmer's Guide, Appendix D explains
the format for the resource structure.  The LPT_INT_LVL is defined in
"cpn5360.h", not "pc.h" as indicated in the programmer's guide.

.SS "Network Configuration"

The fei0 network connection can be configured to come out the 
front panel of the CPN5360, or out the rear using the appropriate
transition module.  By default the fei0 network connection comes
out the front panel network jack. In order to use the rear fei0
ethernet jack, a BIOS setting must be modified.  In the BIOS setup
Utility, under "Advanced", "PCI Configuration", change "Ethernet 1 
Connection".  A setting of "Front" or "Rear" selects the desired 
ethernet port location.  The 2nd network connection, fei1, is always
routed to the rear transition module; there is no front panel jack
for fei1.  

Both network devices, fei0 and fei1, are supported when 
INCLUDE_SECONDARY_ENET is defined.  Only fei0 is supported 
if INCLUDE_SECONDARY_ENET is undefined.  If a network device 
is specified for use by the "bootrom" image in the 
DEFAULT_BOOT_LINE definition of "config.h", it will be the only 
one enabled by the boot process.  Any non-boot network device 
will need to be enabled by the user, either at the command line 
or programmatically.  The following commands, when modified with 
actual IP addresses, etc., can be used to enable fei1 when it is 
not used as the boot device.  A similar command sequence will be 
needed to enable fei0 if it is not the device used to boot the 
board.  Note that the two fei interfaces should be assigned 
unique IP addresses. Please refer to the VxWorks reference entries for 
the exact syntax for the following commands, and to the VxWorks 
Network Programmer's Guide for general information on networking. 

.CS
ipAttach( 1, "fei")
ifAddrSet( "fei1", "xxx.xxx.xxx.xxx")  
ifMaskSet( "fei1", 0xffffff00)
routeAdd( "yyy.yyy.yyy.yyy", "xxx.xxx.xxx.xxx")
.CE

.SS "NVRAM"
The CPN5360 does not have available non-volatile RAM; thus, boot 
parameters are not preserved when the system is powered off.  
However, static boot parameters can be set in the boot disk by 
adjusting the boot line parameter macro DEFAULT_BOOT_LINE in 
"config.h". If the DEFAULT_BOOT_LINE is modified, a new
boot disk must be created.

.SS "Real Time Clock Timekeeping and Alarms"
In addition to the standard VxWorks timekeeping and alarm 
features, the CPN5360 provides additional support routines
for the on-board Real Time Clock (RTC).  The RTC chip on the
CPN5360 is compatible with other available chips, which all seem 
to implement pretty much the same Real Time Clock/Calendar 
programmatic interface and functionality.  This includes both 
a time/date feature and the ability to set up an alarm interrupt 
at chosen time(s).  

The BSP routines for the RTC are:

.IP "sysAuxRtcInt()" 
An Interrupt Service Routine (ISR) which keeps 
a global time/date structure up to date, processes alarm 
interrupts, and also calls the user's hooked-in aux clock routine.
.IP "sysRtcDateTimeSet()"
A routine which sets the RTC time/date per caller's specification
.IP "sysRtcDateTimeGet()"
A routine which reads the RTC time/date and returns it to the caller 
.IP "sysRtcAlarmSet()"
A routine which sets the chip up for the future generation of a 
single or recurring alarm interrupt(s) 
.IP "sysRtcAlarmCancel()" 
A routine which cancels the generation of future alarm interrupts
.IP "sysRtcDateTimeTest()"
A routine which validates the time/date structure, and is called 
from several routines previously noted 
.IP "sysRtcDateTimePrint()" 
A routine which simply prints the time/date 
.IP "sysRtcInit()" 
An initialization function, which configures the chip for 
BCD / 24-hour / Daylight Savings Disabled / Update Ended Intr
Enabled.   
.IP "sysRtcDateTimeHook()" 
A hook routine for the dosFsLib, gives the dosFs current date 
and time information.
.IP "sysRtcAlarmConnect()"
connect a user's routine to the alarm interrupt
.LP

Note that the RTC device on the CPN5360 will assert an alarm
interrupt approximately 2 milliseconds before it finishes 
updating the RTC date and time.  Therefore, it is possible that
sysRtcDateTimeGet() will return date/time values 1 second less
than the intended alarm time, if sysRtcDateTimeGet() is called from
within the user's connected Alarm ISR. 

The RTC features can be included in the BSP by defining
INCLUDE_RTC in config.h.

See each function's reference entry for more detail.
 
.SS "LM78"
The National Semiconductor LM78 support routines are available when
the kernel is built with INCLUDE_LM78 defined in "config.h".  The LM78
system monitor allows for monitoring of five different positive input
voltages (IN0, IN1, IN2, IN3, and IN4), two negative input voltages
(IN5 and IN6), one temperature (degrees C), three fan RPMs (FAN1,
FAN2, and FAN3), and several bi-state single-pin inputs.  The LM78 is
capable of automatic out-of-bounds checking for voltages, temperature
and fan RPM.

These LM78 functions do not directly support interrupt handling.
There is nothing in these routines, however, to prevent the user from
enabling interrupts associated with the LM78.  If this is done, it is
up to user software to handle the related interrupts.

Because of hardware restrictions, it is not possible to read the raw
status within 1.5 seconds of turning on monitoring, or if monitoring
is already on, within 1.5 seconds of a previous read of the raw
status.  The LM78 support functions provided in this BSP will enforce
these restrictions.

These support routines do not handle reentrancy contention.

Complete software support for system monitoring will involve
the writing of a system monitor task (not provided in this BSP), which
uses the LM78 functions supplied in this BSP.  Knowledge of the
information in the "National Semiconductor LM78 Microprocessor System 
Hardware Monitor" specification is essential for development of a
complete monitoring system.

One possible outline for a system monitor task would be:

Read the raw status using lm78RawStatusGet(), and obtain the current
settings in a LM78_RAW_STAT structure.

Using lm78PlusVoltToRaw(), lm78MinusVoltToRaw(), lm78TemperatureToRaw
compute "raw value RAM" numbers which represent the desired high and low
limits for voltages, and the high limit for temperature.

Insert the computed values into the LM78 by altering the LM78_RAW_STAT
structure.  The "lm78.h" file contains #defines which fix the locations
within the value RAM for these computed limits.

Use lm78RawStatusSet() to write back the raw status.  This will set
the desired limit values into the LM78 chip itself.

Turn on monitoring using lm78MonitorOn().

In a loop: 

.IP
Set up a taskDelay() of about 2 seconds (raw status cannot be read
from the lm78 more often then once per 1.5 seconds).

.IP
Read the raw status using lm78RawStatusGet() again obtaining the
LM78_RAW_STAT structure.  Check the interrupt status registers 
in this structure for out of bounds conditions.  The "lm78.h" file has
#defines for each of the bits in these registers.  Note that if some
functions are not wired up to the LM78 then out of bounds conditions for
these functions are to be expected.

.IP
If out of bounds conditions are found to exist, the exact value of the
out of bounds condition (as well as the limit values which triggered
the out of bounds condition) can be determined by calling
lm78RawStatusFormat() with the LM78_RAW_STAT structure.  A formatted
structure will be returned.  The data in the formatted structure is
suitable for display with printf(), using float formats for voltages,
integer format for temperature.  An operator could be alerted with
a message specifying the parameter that was out of bounds.
.LP

End of loop.

.SS "Failsafe Timer"

Support for the watchdog timer in the FPGA is provided.  This support
is not part of the standard VxWorks watchdog library, wdLib.  The 
primary advantage the new failsafe watchdog has over wdLib is that
the failsafe timer expiration is a non-maskable event which will 
drive the sysreset line on the board.  The failsafe is a one-shot 
timer, it will expire only once when set.  This prevents it from
repeatedly driving the board's sysreset line.

Failsafe timer support can be included in the BSP by defining 
INCLUDE_FAILSAFE in config.h.  This support by default is excluded.
There is only one failsafe timer on the board, so only one failsafe 
timer can be established at any given time.  

In order to use the failsafe timer, the user will need to 
first call failsafeStart().  Subsequently, the user will
typically call the failsafeStrobe() routine on a periodic basis,
to prevent expiration of the failsafe timer.  If the failsafe
timer is no longer needed, it should be disabled with a call to
failsafeCancel().  If desired, the user can call failsafeCausedReset()
at any time to determine if the board's previous reset was caused
by expiration of the failsafe timer.

The failsafe timer, if active, is cancelled during the processing of
a user reboot request.  This prevents the failsafe from an untimely
expiration, after the reboot has occurred.  For short failsafe timeout
delay lengths, the reboot processing may be too slow, allowing the
failsafe to expire before the failsafe is cancelled.   

.SS "DEC2155x SROM Support"

Support routines to read and write the DEC2155x PCI-PCI drawbridge
SROM are provided.  Normally, the CPN5360 board will be injected by 
Manufacturing ATE with SROM default values.  The dec2155xSrom.c 
file allows the user to redefine the default factory settings.  The
routine sysDec2155xSromWrite() expects a buffer pointer to a place in
memory containing an entire, valid SROM image.  A function is 
provided which will program the DEC2155x's SROM with "reasonable" 
values.  This function is named sysDec2155xSromDefault() and takes 
the DEVSEL Id of the DEC2155x as its argument.  The routine 
supplies a "canned" set of default values.  Those values are 
outlined below:

.CS

Offset    Size    Value       Description/comments
------    ----    -----       --------------------
00        4       80,00,00,00 ROM present and reserved bytes
04        1       00          Primary programming interface
05        1       02          (Pentium)
                  20          (PowerPC)   Primary Sub-class code
06        1       0B          (Bridge Device) Primary Base class code
07        2       57, 10      Subsystem vendor ID (Motorola)
09        2       14, 48      (CPN5360)
0B        1       00          Primary minimum grant
0C        1       00          Primary maximum latency
0D        1       00          Secondary programming interface
0E        1       80          Secondary Sub-class code
0F        1       06          Secondary Base class code
10        1       00          Secondary minimum grant
11        1       00          Secondary maximum latency
12        4       00, 00, 00, 00  Downstream memory 0 setup register
16        4       00, 00, 00, FF  Downstream memory 1 setup register
1A        4       00, 00, 00, FF  Downstream memory 2 setup register
1E        4       00, 00, 00, 00  Downstream memory 3 setup register
22        4       00, 00, 00, 00  Downstream memory 3 upper setup register
26        2       00, 00          Primary expansion ROM setup register
28        4       08, 00, F0, FF  Upstream memory 0 setup register
2C        4       08, 00, F0, FF  Upstream memory 1 setup register
30        2       00, 04      Chip control 0 register
32        2       00, 00      Chip control 1 register
34        2       00, 02      Arbiter control register
36        1       00          Primary SERR Disables
37        1       00          Secondary SERR Disables
38        8       00,01,02,03,
                  04,05,06,07 PCI Power Management Data Register Values
40        1       00, Compact PCI Hot-Swap ECP ID
41        2       01, 00      PCI Power Management and BIST
43        3D      00,00,00 ...    Unused and reserved
80        180     00,00,00 ...    VPD data (unused currently)

* - Subsystem IDs for other boards are defined in "sysMotCpci.h"

.CE

The Downstream memory 1 setup register and Downstream memory 2 setup 
register, then, will open two 16MB windows from which other cPCI participants
can read from.  The Upstream memory 0 setup register and Upstream memory 1 
setup register will open two 1MB prefetchable windows to read from.

.SS "PCI Configuration"
PCI devices can either be configured by the BIOS
or by VxWorks. The user has the choice based on the
INCLUDE_PCI_AUTOCONF define in "config.h". Defining
this symbol will cause the Wind River PCI Auto 
Configuration code to enumerate the bus. This
is the default state. Un-defining INCLUDE_PCI_AUTOCONF
will cause VxWorks to use the BIOS configured values.
Prior to the PCI Auto Configuration code running, the
secondary bus (cPCI) will be reset. The reason for 
this reset is to make sure that devices on the cPCI 
bus are using the Wind River PCI configuration
values, not the previously configured, BIOS settings. 
The Wind River PCI Auto Configuration code is only
executed when VxWorks is cold booting from a ROM-based
image (i.e. when the BIOS hands control to VxWorks).

PCI configuration, which assigns memory addresses to PCI devices, is
always performed by the BIOS before VxWorks is booted.  If
INCLUDE_PCI_AUTOCONF is define'd, then VxWorks will scrap the
BIOS assigned PCI addresses and assign its own PCI addresses to the
devices.  These addresses will differ from those that the BIOS
assigned.  Also MMU access to the assigned addresses will be defined
though static entries in the sysPhysMemDesc[] table in "sysLib.c".

When INCLUDE_PCI_AUTOCONF is undef'd, the BIOS address
assignment to PCI devices is not altered by VxWorks.  Also, VxWorks
will perform the MMU memory mapping for the PCI devices by making
individual calls to sysMmuMapAdd() for each PCI memory region which
was assigned by the BIOS.  For each sysMmuMapAdd() call made, a
DUMMY_MMU_ENTRY is used up in the "sysLib.c" table sysPhysMemDesc[].
There are enough DUMMY_MMU_ENTRIES in the table to accommodate all of
the on-board devices on the CPN5360 as well as other devices which may
be plugged into the PMC slots.  In the unlikely case that there are
insufficient DUMMY_MMU_ENTRIES, some devices may be unusable.  More
entries may be added by editing the sysPhysMemDesc[] table in
"sysLib.c".  It's OK to add more than you need, the only penalty is
a slightly larger bootrom and kernel image.

The Wind River PCI Autoconfiguration performs the following:

.TS E
expand;
l l .
1.	T{
Probes the host PCI bridge for all devices on the host
PCI bus (bus zero).  Bus zero might contain one or more
PCI-PCI bridges.  These bridges are probed
as well and recursive probing occurs until all devices
and bridges are found. 
T}
2.	T{
Checks all of the devices found during the probing process against an
optional "roll call" list (see expanded explanation below).  If some 
devices are slow to appear on the
PCI bus, they may not be detected during the initial roll call check.
If they do not appear, the bus enumeration step (1) will again be
performed and roll call checked until either all of the listed devices
are found or a configurable timeout value has been reached.  See
"config.h" for more information on this roll call feature.
T}
3.	T{
Memory is assigned to each device and sub-bridge found.
For devices, each Base Address Register (BAR) is queried.
Memory or I/O space (or both) is allocated for each BAR
which has been implemented.
T}
4.	T{
Complete initialization of the devices is performed,
including cache line size, command register, latency timer,
interrupt line and base address registers (0 through 5).
PCI-PCI bridges are initialized with the correct primary
bus, secondary bus and subordinate bus designation.  In
short, the entire bridge/device "tree" rooted at the host
pci bridge is completely configured and ready for driver
access.
T}
.TE

.SS "PCI Autoconfiguration Roll Call"

A feature of PCI autoconfiguration is "roll call".  If you expect to
find a certain number of specific devices identified by device ID and
vendor ID during PCI autoconfiguration you can enter the information
into a roll call list.  This roll call feature is only available when
INCLUDE_PCI_AUTOCONF is #defined in "config.h".  For example, assume
that you know the autoconfiguration process should find seven
different devices with device ID of 0x0046 and vendor ID of 0x1011
(this would be the Dec2155x device found on the CPN5360 board).  You
want PCI autoconfiguration to "wait" until it finds at least seven
such devices but you do not want it to wait more than 2 seconds.  If 2
seconds have elapsed and seven different Dec2155x chips have not
appeared in the bus enumeration process, you would like the PCI
autoconfiguration process to proceed anyway.

You would construct the "roll call" list in "config.h" as shown
below:

.CS
#define PCI_ROLL_CALL_LIST_ENTRIES \
    { 7, 0x0046, 0x1011 },

#ifdef PCI_ROLL_CALL_LIST_ENTRIES
#   define ROLL_CALL_MAX_DURATION 2
#endif
.CE


The parameter ROLL_CALL_MAX_DURATION specifies that no more than 2
seconds should elapse before proceeding on with the autoconfiguration,
even though less than seven Dec2155x devices have been found.  Note
that ROLL_CALL_MAX_DURATION always expresses an integer number of
seconds.  If it is given a value of zero (the minimum possible value),
then roll call is effectively bypassed.  That is, PCI
autoconfiguration will proceed regardless of the number of devices
found during bus enumeration.

You can see the entry { 4, 0x0046, 0x1011 } which says that you expect
to find at least 4 devices whose device ID is 0x0046 and vendor
ID is 0x1011.
Note that "cpn5360.h" contains defines for some device and vendor IDs,
such a define could be used here instead of a hard-coded
device and vendor ID.

Also note that this list can be extended so more than one device and
vendor ID is identified with possibly a different count.

If the list is empty then there
is no roll call waiting performed, regardless of the setting of
ROLL_CALL_MAX_DURATION (seconds).

The roll call feature can be useful for devices which need a bit of
time to appear on the cPCI bus.  For example, CPN5360 CPU boards
(which contain the Dec2155x nontransparent PCI bridge) will not be
visible to a CPV5300 master which is enumerating the bus until the
CPN5360 clears the "primary access lock-out" bit in the Dec2155x chip
control 0 register.  Just prior to enumerating the PCI bus in
preparation for autoconfiguration, the CPV5300 performs a secondary
bus reset which causes each CPN5360 to reset and begin executing
initialization code.  The CPV5300 immediately begins cPCI bus
enumeration in preparation for autconfiguration.  Since the CPN5360s
are busy initializing, they may not have proceeded to the point
where they have cleared the "primary access lock-out" bit and thus
would be invisible to the CPV5300 PCI bus enumeration process.
Without roll call, the CPV5300 may not see the CPN5360s and thus would
not configure them.  The roll call feature allows for bus enumeration
polling until the specified devices actually appear.  Note that roll
call may not always be required.  Some
system configurations and timings may work without using the roll call
feature.  A bit of experimentation may be necessary to determine
whether roll call is needed.  The only penalty for configuring roll
call if it is not required is a possible slight delay in coming up
after reset or power-on.

.SS "Shared Memory Support"

Please note that the CPN5360 requires special Dec2155x SROM programming
before it can be used in an shared memory configuration. See 
the section below titled: "BIOS and Dec2155x SROM considerations with 
shared memory" for an explanation of how to program the SROMs
as part of the preparation for use in a shared memory setup.

The CPN5360 can be used in shared memory configurations with other
Motorola system-slot boards such as the CPV5000, CPV5300 and MCP750 as
well as slave-slot boards which contain a Dec2155x nontransparent
bridge chip.  Examples of these boards are the CPV3060 and MCPN750.
Other Motorola boards which become available with shared memory
support will also be useable in conjunction with the CPN5360.
The documentation covering these other boards will be useful in
configuring a working shared memory system.  The Wind River
Programmer's Guide also provides a great deal of information regarding
shared memory concepts.

.IP "BIOS and Dec2155x SROM considerations with shared memory"
When configuring the CPN5360 to run in a shared memory configuration
perform the following:

.TS E
expand;
l l .
1.	T{
Boot a working VxWorks kernel onto the CPN5360 in preparation for
reprogramming the Dec2155x SROM.
T}
2.	T{
Change the "config.h" parameters to support the desired shared memory
configuration.  Examples of how to do this appear below.
T}
3.	T{
With the desired "config.h" parameters defined, one can build just
the "dec2155xSrom.o" utility, and then separately load it onto a 
running VxWorks kernel by using the VxWorks "ld" command.  Another
approach is to include the utility in the kernel build by addition of 
"dec2155xSrom.o" to the MACH_EXTRA list of the Makefile, and then
build and load the VxWorks kernel.  With either approach, the 
last step is to reprogram the SROM by entering "sysDec2155xSromSmProg()".

It is important to note that you must always rerun the
"sysDec2155xSromSmProg()" function when you elect to change any of the
following Dec2155x related "config.h" parameters which deal with
shared memory: DEC2155X_CSR_AND_DS_MEM0_SIZE,
DEC2155X_CSR_AND_DS_MEM0_TYPE, DEC2155X_US_IO_OR_MEM0_SIZE,
DEC2155X_US_IO_OR_MEM0_TYPE, DEC2155X_US_MEM1_SIZE, and/or
DEC2155X_US_MEM1_TYPE.  Note that the "sysDec2155xSromSmProg()"
utility will reprogram the entire SROM of the Dec2155x chip with
values specified by #define's in "config.h", "dec2155xSrom.h", and
"cpn5360.h".  The specific #define's shown above are the ones dealing
directly with shared memory.  The hardware reference manual for the
Dec2155x defines the contents of the SROM.  The source code which
programs the SROM is contained in the BSP file "dec2155xSromSmProg.c".

Note that if the Dec2155x SROM is not programmed to match the "config.h"
parameters of the bootrom and/or kernel being booted you can experience
startup problems which may include page faults.  To get a kernel up
and running, temporarily "Enable" the "BIOS Setup of the DEC21554 Bridge"
(see step 4 below), and configure the bootrom and kernel with the
"config.h" parameter INCLUDE_PCI_AUTOCONF #undef'ed.  This will allow
you to boot the kernel.  Then alter the "config.h" parameters to
the desired values, rebuild "dec2155xSrom.o", load this utility and
run "sysDec2155xSromSmProg()".  You should then be able to follow the
steps in this section to successfully build and boot a VxWorks kernel
which matches the SROM programming of the Dec21554.
T}
4.	T{
Reset the system and insure that the BIOS parameter is set to allow SROM-only
programming of the Dec2155x.  This is done by powering up the cpn5360 and
from the BIOS video menu (entered by depressing F2 as the board comes up)
performing the following:  From the "Advanced" menu, select 
"PCI configuration".  From "PCI configuration" select "Dec21554 PCI-to-PCI
Bridge Setup".  From "Dec21554 PCI-to-PCI Bridge Setup", select "Disabled"
for "BIOS Setup of DEC21554 Bridge".  
T}
5.	T{
Rebuild and install the bootrom using the "config.h" parameters set up
in step 2.  Rebuild and reload the VxWorks kernel.
T}
.TE

The above steps describe the general method for configuring the
CPN5360 to run in a shared memory environment.  Details involving the
meaning of the pertinent parameters, how and when to change them, are
outlined below.  First, a brief summary of shared memory concepts is
presented.  These concepts are fully explained in the Wind River
Programmers Guide.

.IP "Anchor"
This is an area of memory which must be accessible to all nodes
participating in shared memory backplane communication.  The anchor
points to the actual shared memory buffer pool which must be located
in the same memory space as the anchor itself.  The associated
"config.h" parameter is SM_ANCHOR_ADRS.  The anchor itself is always
located on the "master node" which is always "node zero".  The 
algorithm used to find the anchor is described below:
.CS
if "sm=0xXXXXXXXX" is specified as a boot parameter
then
   use 0xXXXXXXXX as the local address of the anchor
else ("sm" used by itself with no "=0xXXXXXXXX)
   if "config.h" parameter SM_OFF_BOARD == TRUE
   then
      Poll for anchor location using the function 
      defined by SM_ANCHOR_ADRS, in this case the 
      function is sysSmAnchorAdrs() (present in 
      "sysLib.c").  
   else
      Use the build-time calculated value coded in the 
      "config.h" parameter SM_ANCHOR_ADRS as the actual 
      address of the anchor.
   endif
endif
.CE
Note that if sm=0xXXXXXXXX is used to define the anchor address,
it is always specified as a local address even if the anchor resides
on a another board.  In the case where the anchor resides on another
board, the value 0xXXXXXXXX would be a local address which translates
into the correct PCI memory space address to access the anchor on the 
remote board.  

.IP "Master node"
This node is always designated as node zero.  It is the node which
sets up the anchor and shared memory pool.  Once the anchor and shared
memory pool is set up, the master node acts as a peer with the other
nodes.  The node number (always 0 for the master node) is one of the
boot parameters which can be set up with the "c" command from the
bootline prompt or by configuring the DEFAULT_BOOT_LINE parameter in
"config.h".  Don't confuse "master node" with "host node", explained
below.  

.IP "Host node"
The "host node" is the node which configures the compact PCI bus
during startup initialization.  In a system consisting of a CPV5300
and one or more CPN5360s, the "host node" is always the CPV5300.
Don't confuse "host node" with "master node".  "Master node" is simply
a synonym for "node zero".  Although it is typical for the CPV5300 (host
node) to also be node zero, it is not a requirement.  One of the examples
presented below shows a setup where the master node is a CPN5360.
Note also that the "host node" need not necessarily be a VxWorks node.

.IP "Sequential addressing"
This is governed by a "config.h" parameter, INCLUDE_SM_SEQ_ADDR and is
used when sequential IP addresses are assigned to the participating
nodes.  Node zero is assigned the lowest IP address, followed by nodes
1, 2 etc. which are assigned the subsequent and sequential IP addresses.
The advantage of sequential addressing is that fewer boot parameters
must be specified to configure the system.
.LP

.IP "Shared Memory Restrictions"
The following restrictions apply to shared memory configurations.

.TS E
expand;
l l .
1.	T{
Node zero (the master node) must not boot over the shared memory 
interface.  Only nonzero nodes are allowed to boot over the shared 
memory "sm" interface.
T}
2.	T{
Node zero (the master node) cannot poll for the location of the
anchor.  For node zero, the location of the anchor must be absolutely
defined at build time and specified by the SM_ANCHOR_ADRS parameter.
The nonzero nodes need not know the location of the anchor at build or
boot time but can be configured to poll for the anchor dynamically.
Another option (besides polling) for nonzero nodes is to specify the
anchor location via sm=0xXXXXXXXX where 0xXXXXXXXX is the local
address of the anchor.  Determining the value 0xXXXXXXXX for this
option requires some knowledge about the dynamic placement of PCI
devices during PCI configuration.
T}
.TE

.IP "Shared Memory Alterable Parameters"
Configuring a working shared memory system involves proper setting of
certain "config.h" parameters and proper setting of boot parameters
via the "c" command from the boot prompt or through the
DEFAULT_BOOT_LINE parameter in "config.h".  The boot parameters which
are involved in shared memory are:

.TS E
expand;
l l .
1.	T{
"boot device" which should be set to "sm" or "sm=0xXXXXXXXX
for all nonmaster (nonzero) nodes.  Note that the master node
will use some other value here (typically referring to a floppy disk
or maybe an ethernet boot source) since it cannot boot through
the shared memory interface.
T}
2.	T{
"processor number" which, when set to zero, specifies that this
node is the master node.
T}
3.	T{
"inet on backplane (b)" must be set for master nodes.  It
is optional for nonmaster nodes.  For nonmaster nodes, it is
required if sequential addressing is not specified 
(#undef INCLUDE_SM_SEQ_ADDR in "config.h")  It is not required
for nonmaster nodes if sequential addressing is defined.
T}
4.	T{
"gateway Inet (g)" must be set for nonmaster nodes to indicate
the backplane gateway IP address (i.e. the backplane address of the
master node).
T}
.TE

.IP "Specific Shared Memory Related Parameters"
Shared memory setups require careful adjustment of "config.h" parameters
depending upon the mix of boards in the shared memory system.  You may
have to make modifications to the "config.h" file for all of the boards
in the system, not just the CPN5360.  In general, the following are
the only parameters that you may have to adjust:

.IP "INCLUDE_SM_NET (#define or #undef)"
The parameter is #define'd by default.  It must be #define'd to
enable the inclusion of shared memory support code.

.IP "SM_OFF_BOARD (TRUE or FALSE)"
The parameter has a configurable value of either TRUE or FALSE
and directly determines the value of SM_ANCHOR_ADRS (the anchor
address).  
.LP

If SM_OFF_BOARD is FALSE then the following conditions hold: The node
number must be zero.  The location of the anchor is defined to be
in local memory at LOCAL_MEM_LOCAL_ADRS + SM_ANCHOR_OFFSET.  
SM_ANCHOR_OFFSET has a default value of 0x1100 in this case.  
LOCAL_MEM_LOCAL_ADRS is defined as zero.

If SM_OFF_BOARD is defined as TRUE, then the following conditions
hold: The node number is not zero.  The location of the anchor is
dynamically defined by sysSmAnchorAdrs() (explained above under 
"Anchor").

Note that if "sm=0xXXXXXXXX" is used as a boot parameter, then
SM_OFF_BOARD has no effect.  The value of "0xXXXXXXXX" will be used
as the anchor location regardless of the setting of SM_OFF_BOARD.
If simply "sm" is used as a boot parameter, then SM_OFF_BOARD is
queried at initialization time to determine if polling is required
or not.

.IP "SM_ANCHOR_OFFSET (0x1100 or 0x4100)"
This defines the anchor offset.  Its value should be 0x1100 if the anchor
resides on an Intel architecture board such as a CPV5000 or CPV5300.
Its value should be 0x4100 if the anchor resides on a PowerPC board
such as an MCP750 or MCPN750 or CPV3060.  Note that the anchor resides
on the board designated as VxWorks node zero.

.IP "CPCI_MSTR_MEM_BUS (0x00000000 or 0x80000000)"
This defines the Compact PCI address which accesses the DRAM of the
system slot board beginning at local DRAM address 0x0.  If the cPCI
system-slot board is an Intel architecture board such as a CPV5000 or
CPV5300, this this value should be 0x00000000.  If the cPCI system-slot 
board is a PowerPC board, this value should be 0x80000000.

.IP "DEC2155X_US_MEM1_TRANS (0x40000000, 0xfc000000, or 0x00000000)"
This defines the translation address which the Dec2155x will apply to
PCI memory accesses passing through upstream memory window 1.
Transactions which pass through this window access peer cPCI slave
boards on the cPCI backplane.  The address of the cPCI peer Dec2155x
devices is dependent upon how the cPCI system-slot board configured
the bus.  The address to use here can be computed once the lowest cPCI
bus address associated with a slave Dec2155x chip is known.  Round
this address down to the nearest DEC2155X_US_MEM1_SIZE boundary.  Note
that DEC2155X_US_MEM1_SIZE is 64MB (0x04000000 by default).  If the
cPCI system-slot is a Motorola board running VxWorks, the following
values for DEC2155X_US_MEM1_TRANS will work.  If the system-slot board
is a PowerPC board, use 0x00000000.  If the system-slot board is an
Intel architecture board such as a CPV5000 for CPV5300 and
INCLUDE_PCI_AUTOCONF is #define'd, use 0x40000000.  If the
system-slot Intel board has INCLUDE_PCI_AUTOCONF #undef'ed then use
0xfc000000.

The following parameters are related to shared memory but are not
normally changed unless there are unusual circumstances:

.IP "SM_INT_TYPE (SM_INT_MAILBOX_1 or SM_INT_NONE)"
Defines if running in polled mode or interrupt mode.  Normally systems
are run in interrupt mode and this parameter is #define'd to be
SM_INT_MAILBOX_1.  If running in polled mode it should be set to 
SM_INT_NONE. 

.IP "DEC2155X_US_IO_OR_MEM0_SIZE (0x00400000 - default)"
Defines the size of the window which allows the CPN5360 to access
the system-slot DRAM.  It has a value of 0x00400000 (4MB) by default 
which is large enough to allow access to the shared memory pool and 
anchor if it resides on the system-slot board.

.IP "DEC2155X_US_MEM1_SIZE (0x04000000 - default)"
Defines the size of the window opening up onto the cPCI bus to
access peer cPCI slave boards.  It has a 64MB (0x04000000) value
by default.  This is large enough to see six other 4MB peer
Dec2155x boards plus extra space has been defined to allow for 
other uses of PCI memory space on the cPCI backplane unrelated 
to shared memory.

.IP "SYS_SM_SYSTEM_MEM_POLL (#define or #undef)"
This define has an effect only if anchor polling (SM_OFF_BOARD
#define'd as TRUE).  When defined, SYS_SM_SYSTEM_MEM_POLL instructs
the board to poll for the anchor on system memory, that is on the DRAM
of the system-slot board.  The address which is examined for the
anchor will be that described by SM_ANCHOR_OFFSET on the system-slot
board's DRAM.

.IP "SYS_SM_ANCHOR_POLL_LIST (#define or #undef)"
This define has an effect only if anchor polling (SM_OFF_BOARD
#define'd as TRUE).  When defined, SYS_SM_ANCHOR_POLL_LIST allows a
list of devices, identified by device/vendor ID and subsystem
ID/subsystem vendor ID to be specified as candidates for the anchor
location.  Devices which appear directly on the compact PCI bus
(defined by bus number SYS_SM_CPCI_BUS_NUMBER in "config.h") are found
and if they also appear on the list defined by
SYS_SM_ANCHOR_POLL_LIST, they are checked to see if they house the
shared memory anchor.  The exact memory location queried is defined by
the contents of the first memory BAR of the device plus the value of
SM_ANCHOR_OFFSET, define in "config.h" If SYS_SM_ANCHOR_POLL_LIST is
not defined, ALL devices on the compact PCI bus will be considered
candidates for the anchor location and will be polled.  The board
which houses the anchor is distinguished from those that don't by the
appearance, at the anchor offset address, of a special pattern in
memory and a changing value in memory which represents a "heartbeat".
Note that the SM_ANCHOR_OFFSET value must be set in "config.h"
according to the type of board which houses the anchor.  A power PC
board will have the anchor at offset 0x4100 by default, an Intel-based
board will have the anchor at 0x1100 by default.

.IP "INCLUDE_SM_SEQ_ADDR (#define or #undef)"
If "undef'ed", sequential addressing is disabled.  This symbol
is defined by default.

.IP "PCI_MEM_NONPREFETCH_SIZE"
This "config.h" parameter is used to set the value of PCI_MSTR_MEMIO_SIZE.
It defines the size of nonprefetchable memory space which
can be configured by the CPN5360 during PCI autoconfiguration.  It is
only applicable if INCLUDE_PCI_AUTOCONF is defined.  

There must be enough memory specified to accommodate three classes of access:

.TS E
expand;
l l .
1.	T{
All CPN5360 on-board devices requesting PCI nonprefetchable memory space.
T}
2.	T{
The compact PCI host's (cpv5000 in this case) DRAM, typically the lower 4MB.
T}
3.	T{
DRAM for each peer board containing the Dec2155x, typically the lower 4MB.
T}
.TE
If this value is too small, some of the devices which require
nonprefetchable memory space on the PCI bus will not be configured
during PCI autoconfiguration.  The default value is 0x10000000
(256MB).  This value is large enough to allow a shared memory
configuration with seven slave boards and a cPCI master to operate.

.IP "CPN5360 anchor "polling"
Referring to "config.h", "cpn5360.h" and "sysMotCpci.h" will make the
following explanation a bit easier to follow:

A functioning shared memory system consists of a cPCI master such as a
CPV5300, the CPN5360 itself and optionally one or more other
nonsystem-slot boards.  If the CPN5360 is configured with SM_OFF_BOARD
#define'd as TRUE then it must poll for the shared memory anchor in
system DRAM if SYS_SM_SYSTEM_MEM_POLL is #define'd in "config.h"
and/or on cPCI nonsystem-slot boards if SYS_SM_ANCHOR_POLL_LIST is
#define'd in "config.h".  In the case where nonsystem-slot boards must
be polled, the CPN5360 startup code must first be able to find those
boards which are participating in shared memory, one of these may be
the master node where the anchor is resides.  The potential candidates
for shared memory participation are "found" by performing
configuration accesses onto the cPCI bus.  To construct a proper
configuration access onto the cPCI bus, the cPCI bus number must be
known.  This number is defined by SYS_SM_CPCI_BUS_NUMBER #define'd in
"cpn5360.h".  The "list" of identifiers of potential shared memory
candidates is contained in SYS_SM_ANCHOR_POLL_LIST which is #define'd
by default in "config.h".  The actual list contents are defined by
SYS_MOT_SM_ANCHOR_POLL_LIST defined in "sysMotCpci.h".  This list
contains subsystem and vendor IDs which, if found in the Dec2155x
header, identify the associated board as a potential candidate for 
shared memory participation.  The list defined in "sysMotCpci.h" 
contains the subsystem IDs and vendor IDs associated with various 
Motorola boards which currently support shared memory.  This list 
is continually expanded as more Motorola boards are developed which 
support shared memory.

.IP "Example Configurations"

Consider a system consisting of a CPV5300 (master node) and two
CPN5360s.  The development system is typically a Unix system or an
Microsoft Windows NT system.  The configuration parameters presented
below affect the CPN5360s and the CPV5300.  In addition to the proper
setting of these parameters, it is also necessary to configure the
Unix or Windows NT host.  Refer to the VxWorks documentation regarding
"adding gateways to a network" for information on how to configure the
host.

The example presented below applies only if CPN5360s are used as cPCI
shared memory participants.  If other types of boards are used in the
shared memory configuration, some of these parameters may need to
change.

The following two configurations are possible:

.TS E
expand;
lf3 lf3 lf3
l l l .
.ne 8
.sp .5
	Master node location	Anchor location
_
1.	CPV5300	CPV5300	
2.	CPN5360-1	CPN5360-2	
_
.TE
.LP

Below is a description of how the above systems would be
configured.  The parameter settings which commonly need
to be considered for a shared memory setup are given even
if they do not change value from the original "config.h" 
default.  Parameters not mentioned below retain the value
they had in the original "config.h".

.IP "1a."
CPV5300 master, sequential addressing:
.CS

   CPV5300 (node 0):

	#define 	INCLUDE_SM_NET
	#define 	SM_OFF_BOARD 		FALSE
	#define		SM_ANCHOR_OFFSET 	0x1100

     Boot parameters:

        boot device          : fei0
        processor number     : 0
        host name            : sunshine
        file name            : /tftpboot/cpv5300/vxWorks.st
        inet on ethernet (e) : 123.234.1.180
        inet on backplane (b): 123.200.200.1:ffffff00
        host inet (h)        : 123.234.1.162
        gateway inet (g)     : 123.234.1.253
        user (u)             : joe
        ftp password (pw) (blank = use rsh):
        flags (f)            : 0x0
        target name (tn)     : alpha
        startup script (s)   :
        other (o)            :

   CPN5360-1 (node 1):

	#define		INCLUDE_SM_NET
	#define		SM_OFF_BOARD 		TRUE
	#define		SM_ANCHOR_OFFSET 	0x1100
	#define		CPCI_MSTR_MEM_BUS 	0x00000000

     If the CPV5300 has #define INCLUDE_PCI_AUTOCONF

        #define DEC2155X_US_MEM1_TRANS	0x40000000
 
     else if the CPV5300 has #undef INCLUDE_PCI_AUTOCONF

        #define DEC2155X_US_MEM1_TRANS	0xfc000000


     Boot parameters:

        boot device          : sm
        processor number     : 1
        host name            : sunshine
        file name            : /tftpboot/cpn5360-1/vxWorks.st
        inet on ethernet (e) : 
        inet on backplane (b): 
        host inet (h)        : 123.234.1.162
        gateway inet (g)     : 123.200.200.1
        user (u)             : joe
        ftp password (pw) (blank = use rsh):
        flags (f)            : 0x0
        target name (tn)     : beta
        startup script (s)   :
        other (o)            :


   cpn5360-2 (node 2):

     If the CPV5300 has #define INCLUDE_PCI_AUTOCONF

        #define DEC2155X_US_MEM1_TRANS	0x40000000
 
     else if the CPV5300 has #undef INCLUDE_PCI_AUTOCONF

        #define DEC2155X_US_MEM1_TRANS	0xfc000000

     Boot parameters:

        boot device          : sm
        processor number     : 2
        host name            : sunshine
        file name            : /tftpboot/cpn5360-2/vxWorks.st
        inet on ethernet (e) : 
        inet on backplane (b): 
        host inet (h)        : 123.234.1.162
        gateway inet (g)     : 123.200.200.1
        user (u)             : joe
        ftp password (pw) (blank = use rsh):
        flags (f)            : 0x0
        target name (tn)     : gamma
        startup script (s)   :
        other (o)            :
.CE

.IP "1b."
CPV5300 master, no sequential addressing:

This example is identical to 1) except sequential addressing
is NOT enabled.  You will notice that more CPN5360 boot
parameters must be specified with sequential addressing
disabled.
.CS

   CPV5300 (node 0):

	#define 	INCLUDE_SM_NET
	#define 	SM_OFF_BOARD 		FALSE
	#define		SM_ANCHOR_OFFSET 	0x1100
	#undef		INCLUDE_SM_SEQ_ADDR


     Boot parameters:

        boot device          : fei0
        processor number     : 0
        host name            : sunshine
        file name            : /tftpboot/cpv5300/vxWorks.st
        inet on ethernet (e) : 123.234.1.180
        inet on backplane (b): 123.200.200.1:ffffff00
        host inet (h)        : 123.234.1.162
        gateway inet (g)     : 123.234.1.253
        user (u)             : joe
        ftp password (pw) (blank = use rsh):
        flags (f)            : 0x0
        target name (tn)     : alpha
        startup script (s)   :
        other (o)            :

   CPN5360-1 (node 1):

	#define		INCLUDE_SM_NET 
	#define		SM_OFF_BOARD 		TRUE
	#define		SM_ANCHOR_OFFSET 	0x1100
	#define		CPCI_MSTR_MEM_BUS 	0x00000000
	#undef		INCLUDE_SM_SEQ_ADDR

     If the CPV5300 has #define INCLUDE_PCI_AUTOCONF

        #define DEC2155X_US_MEM1_TRANS	0x40000000
 
     else if the CPV5300 has #undef INCLUDE_PCI_AUTOCONF

        #define DEC2155X_US_MEM1_TRANS	0xfc000000

     Boot parameters:

        boot device          : sm
        processor number     : 1
        host name            : sunshine
        file name            : /tftpboot/cpn5360-1/vxWorks.st
        inet on ethernet (e) : 123.234.1.181
        inet on backplane (b): 123.200.200.2:ffffff00
        host inet (h)        : 123.234.1.162
        gateway inet (g)     : 123.200.200.1
        user (u)             : joe
        ftp password (pw) (blank = use rsh):
        flags (f)            : 0x0
        target name (tn)     : beta
        startup script (s)   :
        other (o)            :

   CPN5360-2 (node 2):

	#define		INCLUDE_SM_NET 
	#define		SM_OFF_BOARD 		TRUE
	#define		SM_ANCHOR_OFFSET 	0x1100
	#define		CPCI_MSTR_MEM_BUS 	0x00000000
	#undef		INCLUDE_SM_SEQ_ADDR

     If the CPV5300 has #define INCLUDE_PCI_AUTOCONF

        #define DEC2155X_US_MEM1_TRANS	0x40000000
 
     else if the CPV5300 has #undef INCLUDE_PCI_AUTOCONF

        #define DEC2155X_US_MEM1_TRANS	0xfc000000

     Boot parameters:

        boot device          : sm
        processor number     : 2
        host name            : sunshine
        file name            : /tftpboot/cpn5360-2/vxWorks.st
        inet on ethernet (e) : 123.234.1.182
        inet on backplane (b): 123.200.200.3:ffffff00
        host inet (h)        : 123.234.1.162
        gateway inet (g)     : 123.200.200.1
        user (u)             : joe
        ftp password (pw) (blank = use rsh):
        flags (f)            : 0x0
        target name (tn)     : gamma
        startup script (s)   :
        other (o)            :

.CE

.IP "1c."
This example is identical to 1b) except that the cPCI master and
VxWorks node zero is an MCP750 (a PowerPC base board) instead of
the Intel-based CPV5300.  This has an effect on the parameter 
settings as is shown below:
.CS

   MCP750 (master node):
	#define		INCLUDE_SM_NET 
	#define		SM_OFF_BOARD 		FALSE
	#define		SM_ANCHOR_OFFSET 	0x4100
        #undef  	INCLUDE_SM_SEQ_ADDR

     Boot parameters:

        boot device          : dc
        processor number     : 0
        host name            : sunshine
        file name            : /tftpboot/mcp750/vxWorks.st
        inet on ethernet (e) : 123.234.1.180
        inet on backplane (b): 123.200.200.1:ffffff00
        host inet (h)        : 123.234.1.162
        gateway inet (g)     : 123.234.1.253
        user (u)             : joe
        ftp password (pw) (blank = use rsh):
        flags (f)            : 0x0
        target name (tn)     : alpha
        startup script (s)   :
        other (o)            :


   CPN5360-1 (node 1):

	#define		INCLUDE_SM_NET 
	#define		SM_OFF_BOARD 		TRUE
	#define		SM_ANCHOR_OFFSET 	0x4100
	#define		CPCI_MSTR_MEM_BUS 	0x80000000
        #define 	DEC2155X_US_MEM1_TRANS	0x00000000
        #undef  	INCLUDE_SM_SEQ_ADDR
 
     Boot parameters:

        boot device          : sm
        processor number     : 1
        host name            : sunshine
        file name            : /tftpboot/cpn5360-1/vxWorks.st
        inet on ethernet (e) : 123.234.1.181
        inet on backplane (b): 123.200.200.2:ffffff00
        host inet (h)        : 123.234.1.162
        gateway inet (g)     : 123.200.200.1
        user (u)             : joe
        ftp password (pw) (blank = use rsh):
        flags (f)            : 0x0
        target name (tn)     : beta
        startup script (s)   :
        other (o)            :

   CPN5360-2 (node 2):

	#define		INCLUDE_SM_NET 
	#define		SM_OFF_BOARD 		TRUE
	#define		SM_ANCHOR_OFFSET 	0x4100
	#define		CPCI_MSTR_MEM_BUS 	0x80000000
        #define 	DEC2155X_US_MEM1_TRANS	0x00000000
        #undef  	INCLUDE_SM_SEQ_ADDR

     Boot parameters:

        boot device          : sm
        processor number     : 2
        host name            : sunshine
        file name            : /tftpboot/cpn5360-2/vxWorks.st
        inet on ethernet (e) : 123.234.1.182
        inet on backplane (b): 123.200.200.3:ffffff00
        host inet (h)        : 123.234.1.162
        gateway inet (g)     : 123.200.200.1
        user (u)             : joe
        ftp password (pw) (blank = use rsh):
        flags (f)            : 0x0
        target name (tn)     : gamma
        startup script (s)   :
        other (o)            :


.CE

.IP "2."
The following example, unlike the previous ones, has the VxWorks
node zero residing on a non-system slot board, a CPN5360.  The
cPCI master, a CPV5300, is slave node 1.:
.CS

   CPV5300 (slave node 1):
	#define		INCLUDE_SM_NET 
	#define		SM_OFF_BOARD 		TRUE
	#define		SM_ANCHOR_OFFSET 	0x1100
        #undef  	INCLUDE_SM_SEQ_ADDR

     Boot parameters:

        boot device          : sm
        processor number     : 1
        host name            : sunshine
        file name            : /tftpboot/cpv5300/vxWorks.st
        inet on ethernet (e) : 123.234.1.180
        inet on backplane (b): 123.200.200.1:ffffff00
        host inet (h)        : 123.234.1.162
        gateway inet (g)     : 123.200.200.2
        user (u)             : joe
        ftp password (pw) (blank = use rsh):
        flags (f)            : 0x0
        target name (tn)     : alpha
        startup script (s)   :
        other (o)            :


   CPN5360-1 (master node 0):

	#define		INCLUDE_SM_NET 
	#define		SM_OFF_BOARD 		FALSE
	#define		SM_ANCHOR_OFFSET 	0x1100
	#define		CPCI_MSTR_MEM_BUS 	0x00000000
        #undef  	INCLUDE_SM_SEQ_ADDR

     If the CPV5300 has #define INCLUDE_PCI_AUTOCONF

        #define DEC2155X_US_MEM1_TRANS	0x40000000
 
     else if the CPV5300 has #undef INCLUDE_PCI_AUTOCONF

        #define DEC2155X_US_MEM1_TRANS	0xfc000000

     Boot parameters:

        boot device          : fei0
        processor number     : 0
        host name            : sunshine
        file name            : /tftpboot/cpn5360-1/vxWorks.st
        inet on ethernet (e) : 123.234.1.181
        inet on backplane (b): 123.200.200.2:ffffff00
        host inet (h)        : 123.234.1.162
        gateway inet (g)     : 123.234.1.253
        user (u)             : joe
        ftp password (pw) (blank = use rsh):
        flags (f)            : 0x0
        target name (tn)     : beta
        startup script (s)   :
        other (o)            :

   CPN5360-2 (node 2):

	#define		INCLUDE_SM_NET 
	#define		SM_OFF_BOARD 		TRUE
	#define		SM_ANCHOR_OFFSET 	0x1100
	#define		CPCI_MSTR_MEM_BUS 	0x00000000
        #undef  	INCLUDE_SM_SEQ_ADDR

     Boot parameters:

        boot device          : sm
        processor number     : 2
        host name            : sunshine
        file name            : /tftpboot/cpn5360-2/vxWorks.st
        inet on ethernet (e) : 123.234.1.183
        inet on backplane (b): 123.200.200.3:ffffff00
        host inet (h)        : 123.234.1.162
        gateway inet (g)     : 123.234.1.253
        user (u)             : joe
        ftp password (pw) (blank = use rsh):
        flags (f)            : 0x0
        target name (tn)     : gamma
        startup script (s)   :
        other (o)            :

.CE

.SS "Boot Devices"
The supported boot devices are:
	
.CS
    \f3fei\f1 - Ethernet 82559 device
.CE
.CS
    \f3fd=<drive number>,<diskette type>\f1 - Diskette
        drive number is one of:
            0 = default; the first diskette drive (drive A:)
            1 = the second diskette drive (drive B:)
       diskette type is one of:
            0 = default; 3.5" diskette
            1 = 5.25" diskette (not supported)
.CE
.CS
    \f3ata=<controller number>,<drive number>\f1 - ATA/IDE drive
        controller number is one of:
            0 = controller described in the first entry of 
                the ataResources table
            1 = controller described in the second entry of 
                the ataResources table
        drive number is one of:
            0 = first drive on the controller
            1 = second drive on the controller
.CE
.IP "Note:"
By default, with INCLUDE_ATA, vxWorks is set up for 
one ATA hard disk device on the primary ATA controller.  
The 2nd hard disk on the primary controller and the 
secondary controller are normally disabled to prevent timeouts.
If you have multiple controllers and/or hard disks
you must modify the ATA_DEVn_STATE defines in "config.h" 
to support more drives.  Very lengthy boot times will 
result if a device's ATA_DEVn_STATE entry is set to 
DEV_PRESENT when the device is not actually present.
.CS
    \f3ROM\f1 - Refer to section "Boot ROMs".
.CE

.SS "Boot ROMs"
The CPN5360 has, onboard, a 4MB i28f320 StrataFlash.  512KB "windows"
can be selected via the FPGA device.  The enabled 512KB window appears at
0xFFF80000 to 0xFFFFFFFF in the address map.

By default, the system BIOS is programmed into bank 0, or the first
512KB window.

The user can select to boot from one of the remaining 7 flash banks
by selecting a flash bank other than "disabled" in the "Boot"
menu in the BIOS.

The user must first, however, program the desired flash bank with
a bootable VxWorks image.

The supported ROMable VxWorks images are:

.TS E
expand;
		Image is	Image Loads
Image Name	Description	Compressed	Into
=
vxWorks.st_rom.bin	bootable vxWorks.st	Yes	High Memory
bootrom.bin	bootrom	Yes	Low Memory
_
.TE

The utility mkbootFlash5360 needs to be added to the VxWorks kernel
by adding mkbootFlash5360.obj and copyRom.obj to the the MACH_EXTRA line 
in the Makefile.

From the VxWorks shell, the user supplies the bank number and
the file name to mkbootFlash5360.  For example:

.CS
    -> mkbootFlash5360 1, "bootrom.bin"
.CE

where "1" is the destination flash bank number and "bootrom" is
the name of the binary file in the current working directory.  Note
that when working from an NT host, the default current working
directory is c:\.  Under Unix, the default current working directory
is the user's $HOME directory.  For Tornado 2.2, be sure to specify a
binary file type and not an ELF or a.out file type.

An absolute path name can also be used.  For example:

.CS
    -> mkbootFlash5360 2, "/home/user/vxWorks.st_rom.bin" 
.CE

After the flash bank has been programmed, the user would reboot to
BIOS and select the flash bank to boot from.

Additionally, the define SYS_WARM_TYPE controls which device is
to be the warm boot device.  By default, the define is set to
SYS_WARM_BIOS.  When sysReboot is invoked (via sysToMonitor or
reboot), the warm boot device is checked.  If the kernel was
booted from a flash bank other than the BIOS and SYS_WARM_TYPE
is defined as SYS_WARM_BIOS, the BIOS flash bank is selected and 
the board is reset to BIOS.  If the kernel was booted from a flash 
bank other than the BIOS and SYS_WARM_TYPE is defined as SYS_WARM_ROM,
the ROM bank is rebooted (i.e. the BIOS does not run).

To load VxWorks, and for more information, follow the instructions in the
.I "Tornado User's Guide: Getting Started."


.SS "Boot Methods"
Ethernet boot methods are affected by the boot parameters.  If no password is
specified, RSH (remote shell) protocol is used.  If a password is specified,
FTP protocol is used, or, if the flag is set, TFTP protocol is used.

.SH "SPECIAL CONSIDERATIONS"
This section describes miscellaneous information  concerning
this BSP and its use.

.SS "Boot Issues: Creating Floppy and ATA Boot Disks"

The boot floppy image must be copied to the floppy disk or 
the IDE disk using the DOS boot utility "mkboot.bat" or the 
VxWorks boot utilities "mkbootFd" for floppy disks or 
"mkbootAta" for hard disks.
Note that a version of mkboot.obj is included in the BSP
directory of this release, and should be loaded 
to provide VxWorks mkbootAta() functionality.

The usage of the two VxWorks boot utilities is as follows:
.ne 15
.CS
STATUS mkbootFd
    (
    int drive,    /* destination drive number: (0 - 3)  */
    int fdType,   /* type of floppy disk: (0 - 1)       */
    char *in      /* source file name                   */
    )

STATUS mkbootAta 
    (
    int ctrl,     /* dest. controller number: (0 - 1)   */
    int drive,    /* dest. drive number: (0 - 1)        */
    char *in      /* source file name                   */
    )
.CE
Both routines return OK on success and ERROR if there is an error while
copying the image from the source onto the disk.

Examples:

Example 1: Creating a boot floppy disk using mkbootFd:

The floppy disk is in drive 0 (or a:), the diskette is of type 0,
and the image file name is `bootrom':
.CS
    -> mkbootFd 0, 0, "bootrom"
.CE
Example 2: Creating a bootable hard disk using mkbootAta:

The hard disk is on ATA controller 0 and is drive number 0
(or c:). The image file name is `bootrom':
.CS
    -> mkbootAta 0, 0, "bootrom"
.CE

Please see note regarding ATA boot devices in the 
"Known Problems" section.

Example 3: Creating a boot floppy using "mkboot.bat":

"mkboot.bat" is a MS-DOS batch file that will create a boot floppy.
This batch file must be run on a PC under a MS-DOS session.

1. On the development host, change to the BSP directory and use make
to produce the boot floppy image. Copy the resulting file to a legal
MS-DOS file name.
.CS
    % make clean
    % make bootrom.bin
.CE
2. If your host development system is not a PC, you will need to 
transfer the executable image, "bootrom.dat", to a PC running MS-DOS.

3. Insert a blank, high-density diskette into the PC's floppy drive.

4. Execute the mkboot batch file:
.CS
    C:\> mkboot a: bootrom.bin
.CE
5. Take the floppy out of the PC and insert
it into the floppy drive of the target and reboot the target. 

For information on the use of the DOS utility "mkboot.bat" 
and how to boot VxWorks, please refer to the
.I "VxWorks Programmer's Guide, Appendix D: Intel i386/i486."

.SS "Delivered Objects"
.ne 20
.TS
center;
c c
l l.
Object Name	Description
_
vxWorks	T{
image with no target shell or target symbol table. Network is included
and initialized.
T}
vxWorks.st	T{
fully linked stand-alone image including
a target based shell, symbol table, and network 
interface. Note that the network interface is not initialized.
This is because the network interface is initialized only if
the bootrom is configured to load the vxWorks image from the 
network. If only the floppy, local ATA device, etc., is used 
to boot to a VxWorks prompt, then the network interface will 
not be initialized. There is no WDB agent.
T}
bootrom	T{
bootrom with compressed image which will run in lower memory.
T}
mkboot.obj	T{
vxWorks utility for creating boot disks.
T}
copyRom.obj	T{
flash loader code needed by mkbootFlash5360.
T}
mkbootFlash5360.obj	T{
utility to burn binary image into flash bank.
T}
_
.TE

.SS "Make Targets"
The BSP supports the following make targets:

.TS E
expand;
Image Name	Description	Comments
=
vxWorks.st_rom	T{
high memory compressed bootable vxWorks.st.
T}
.sp 1
bootrom	T{
low memory compressed bootrom.
T}
.sp 1
bootrom_uncmp	T{
low memory uncompressed bootrom.  Depending upon configuration
the image size may be too large for the flash bank or for a
floppy.
T}
.sp 1
vxWorks	standard "Tornado-style" vxWorks
.sp 1
vxWorks.st	T{
Fully linked stand alone vxWorks
including target based shell, symbol table, and network 
interface. The network interface is not initialized.
There is no WDB agent.
T}
_
.TE

.SS "Project Build Spec's"
The CPN5360 BSP is delivered with prebuilt vxWorks and vxWorks_romCompress
project build specifications.  The uncompressed ROM build spec, and the ROM
resident build spec, generate images that are too large for loading into a
flash bank or a floppy disk.

.SS "Special Routines"
The following routines are specific to this BSP and are available
to the user. The are written in assembly code in sysALib.s. For 
further details see the reference entries:
.CS
sysInByte - input one byte from I/O space
sysInWord - input one word from I/O space
sysInLong - input one long-word from I/O space
sysOutByte - output one byte to I/O space
sysOutWord - output one word to I/O space
sysOutLong - output one long-word to I/O space
sysInWordString - input word string from I/O space
sysInLongString - input long string from I/O space
sysOutWordString - output word string to I/O space
sysOutLongString - output long string to I/O space
.CE
.SS "Known Problems"

When INCLUDE_PCI_AUTOCONF is undef'd, references to memory
locations which are not mapped do not result in an expected page
fault.  Reads from unmapped locations return all 0xF's, writes have
no effect.  This problem does not occur when INCLUDE_PCI_AUTOCONF is
#define'd, in which case page faults occur when accesses to unmapped
memory are made.

In "config.h", INCLUDE_END must be defined.  Erratic behavior
including extraneous reflected network packets results if INCLUDE_END
is not defined.

In "config.h" if INCLUDE_SHOW_ROUTINES is #define'd and the DEFAULT_BOOT_LINE
parameter is longer than 163 characters, the boot code will hang and
never get to the boot prompt.  If INCLUDE_SHOW_ROUTINES is not
#define'd then the DEFAULT_BOOT_LINE parameter can be longer than 163 
characters with no problem.

.IP "ATA Boot Devices Note" 12
It should be noted that use of the ataDevCreate() command
(part of ataDrv) with an offset of zero (the 4th parameter) 
will overwrite the ATA device's boot sector.  This will make 
the device unuseable as a VxWorks boot device, because VxWorks 
requires a MS-DOS format on all ATA boot devices.  It will not be 
possible to use mkbootAta to install a bootrom image 
on the ATA device, until the boot sector is restored.  
It is strongly recommended that an offset of at least 1 
(sector) be passed in the ataDevCreate call for any 
device which may need to be used as a boot device later.  
Use for example:
.CS
ataDevCreate(0,0,0,1)
.CE
Although this issue applies to all ATA boot devices, it
is especially problematic with the ATA compact flash 
device soldered to the CPN5360 board, which cannot easily 
be replaced.  After using the ataDevCreate command, even when 
an offset of 1 is used, it will be necessary to restore an 
MS-DOS compatible file system on the drive prior to running 
mkbootAta.  The partition the file system resides on must 
be "active".
.LP

The keyboard is inoperable when booting from a flash bank with
video console enabled (INCLUDE_PC_CONSOLE #define'd).  Workarounds
are to boot from floppy or to specify COM1 (serial link) as the
console by #undef'ining INCLUDE_PC_CONSOLE.

.SH SEE ALSO
.tG "Getting Started,"
.pG "Configuration"
.pG "Architecture Appendix"

.SH "BIBLIOGRAPHY"
.iB Refer to Motorola's documentation for the CPN5360, "CPN5360 
CompactPCI Single Board Computer and Transition Module Installation 
and Reference Guide CPN5360A/IH1"
.iB "LM78 Microprocessor System Hardware Monitor (National Semiconductor)"
