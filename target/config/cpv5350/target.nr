'\" t
 .so wrs.an
.\" cpv5350/target.nr - CPV5350 target-specific documentation
.\"
.\" Copyright 1984-2002 Wind River Systems, Inc.
.\" Copyright 1999, 2000 Motorola, Inc. All Rights Reserved
.\"
.\" modification history
.\" --------------------
.\" 01c,17apr02,dat  Update for T2.2 release, new shared memory macro names
.\" 01b,24aug01,dgp  changed manual entry to reference entry per SPR 23698
.\" 01a,22may00,djs  created based on cpv5300, 01z,18feb00
.\"
.\" NOTE
.\" EXOS
.\"
.TH "cpv5350" T "cpv5350" "Rev: 22 May 00" "TORNADO REFERENCE: VXWORKS"

.SH "NAME"
.aX "Motorola CPV5350"

.SH "INTRODUCTION"
This reference entry provides board-specific information 
necessary to run VxWorks on the CPV5350. If this is the first
time that VxWorks is being booted on the CPV5350, please use
the delivered vxWorks.st and bootrom images to verify the 
board's operation. Please review sections "Bios Setup" and 
"Floppy Considerations" before continuing.

.SS "Bios Setup"
Prior to running VxWorks on the CPV5350, some of the 
Phoenix BIOS settings need to be modified.
Before powering on the CPV5350, plug in a video monitor, 
PC compatible keyboard and PS/2 mouse to the CPV5350. 
Please refer to the "CPV5350 CompactPCI Single Board Computer 
and Transition Module Installation Guide (Motorola)" for details.

To enter the BIOS system setup, press and hold the <F2> key
during the power on BIOS memory test. The Setup main 
menu should be displayed.

First make sure that the version of the BIOS running is
CPV5350 BIOS V2.2RM02 or greater. This information is 
found on the "Main" BIOS screen.

Next select the "Advanced" menu.
Make sure that the "Plug and Play Aware OS" is set to NO.
Set the "Legacy USB Support" to disabled. To get to the 
"Legacy USB Support" option, use the arrow keys to scroll down.
Also on the "Advanced" menu, select "PCI Configuration" and
then "PCI/PNP IRQ Configuration". Set the values on the
screen as follows:

.CS
IRQ 9:          [Available]
IRQ 11:         [Available]


PCI IRQ line 1: [5]
PCI IRQ line 2: [9]
PCI IRQ line 3: [10]
PCI IRQ line 4: [11]
.CE

The BIOS assigns the four PCI interrupts from a "pool" of 
what it deems are available IRQs. In this pool are 
interrupts 14 and 15. VxWorks dedicates these IRQs
to the support of IDE and does not allow them to be
shared. By explicitly stating, the PCI interrupt assignments
from within the BIOS resolves this conflict.

At this point BIOS setup is complete, save and exit the BIOS.

.SS "Boot Floppy"
The boot floppy is a formatted double sided high density 3.5"
diskette.
The boot floppy includes the boot sector (sector 0) and a DOS file system
containing a boot ROM image named `bootrom.sys'.
The boot floppy image could be one of following:

.TS E
expand;
		Image is	Image Loads
Image Name	Description	Compressed	Into
=
vxWorks.st_rom	bootable vxWorks.st	Yes	High Memory
bootrom	bootrom	Yes	Low Memory
bootrom_uncmp	bootrom	No	Low Memory
_
.TE

"vxWorks.st" is a fully linked stand-alone image which includes a
target based shell, symbol table, and network interface.  Note that
the network interface is initialized only if the bootrom is configured
to load the vxWorks.st image from the network.  If only the floppy, local
ATA device, etc., is used by the bootrom object to boot vxWorks.st, then
the network interface will not be initialized.  For a floppy-only boot
(which does not access the network) to come up with networking enabled, set 
DEFAULT_BOOT_LINE as if a vxWorks.st image is being booted from the 
network, then build "vxWorks.st_rom", and use the mkboot or mkbootFd 
utility (described in the "SPECIAL CONSIDERATIONS" section) to put 
vxWorks.st_rom onto the floppy disk.

.SH "FEATURES"
The following subsections list all supported and unsupported
features, as well as any feature interaction.

.SS "Supported Features"

The following features of the CPV5350 are supported:

.TS E
expand;
Pentium II (266/333 Mhz)
Pentium III (500 Mhz)
L1 and L2 cache support
440BX host bridge
Up to 256MB on board memory 
System Slot Configuration
Dual 10/100Mb Intel 82559 Ethernet
2 IDE channels, up to 4 IDE devices
DEC21154 transparent PCI-PCI Bridge
2 16650 serial ports (COM1 and COM2)
Keyboard & Mouse
Parallel Port
3.5" 1.44MB Floppy disk
ATA Compact Flash Chipset
Watchdog (failsafe) timer
Real Time Clock Timekeeping and Alarms
LM78 System Monitor
PCI Auto Configuration
VxWorks Shared Memory
VGA 
PCI Interface: 32-bit address, 32-bit data; complies with \f2PCI Local Bus Specification\f1, Revision 2.1
.TE

.ne 2i
.SS "Unsupported Features"
The following features of the CPV5350 are unsupported or not tested:
.TS E
expand;
Universal Serial Bus
Hot Swap
AGP Video
Power Management
ECC
Vital Product Data (VPD SROM)
DMA
X-Bus Peripheral Support
.TE

.SS "Feature Interactions"
None known.

.SH "HARDWARE DETAILS"
This section documents the details of the device drivers and board
hardware elements.

.SS "Devices"
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
#define'ed in "config.h". The macro PC_KBD_TYPE should be defined in 
"config.h" as PC_PS2_101_KBD to include "i8042Kbd.c", and as 
PC_XT_83_KBD to include "i8048Kbd.c".
T}
"m6845Vga.c"	T{
Driver for the Motorola M6845 VGA controller.
To use this controller, define INCLUDE_PC_CONSOLE in "config.h".
T}
"nec765Fd.c"	T{
Driver for the nec765 floppy disk controller.
To use this driver, the INCLUDE_FD directive must be enabled in
"config.h".
T}
"ataDrv.c"	T{
Driver for the IDE/ATA hard disk controller.  
To use this driver, the INCLUDE_ATA directive must be defined in "config.h".
Also changes my be required in "sysLib.c" with regards to the 
ataResources structure.
Note that the old INCLUDE_IDE directive is replaced by INCLUDE_ATA,
and that vxsys() is replaced by mkbootFd() and mkbootAta().
Note also that a new version of mkboot.c is included in the BSP
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
found in cpv5000.h.  
T}
\f3"fei82557End.obj"\f1	T{
Generic driver for the Intel 82557 Ethernet network interface chip.
To use this driver the INCLUDE_FEI directive must be enabled in "config.h".
On the CPV5350, the INCLUDE_END directive must also be enabled in 
"config.h". (see Known Problems)  "fei82557End.obj" is used for both Intel 
82559 devices on the CPV5350.
T}
"lm78.c"	T{
Support routines for the LM78 system monitor.  To use these support
routines, INCLUDE_LM78 must be enabled in "config.h".  If you wish
to have lm78Init() called from the boot module, you must have
INCLUDE_LM78 defined in the build of the boot image.
T}
"failsafe.c"	T{
Support routines for the hardware watchdog timer.  To use these support
routines, INCLUDE_FAILSAFE must be defined in "config.h".  
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
 (default = 0x40000000)   (default = 0x01400000)     PCI memory

PCI_MSTR_MEM_BUS         PCI_MSTR_MEM_SIZE          32-bit Prefetchable
 (default = 0x41400000)   (default = 0x01300000)     PCI memory

ISA_MSTR_IO_BUS          ISA_MSTR_IO_SIZE           16-bit ISA I/O
 (default = 0x00001000)   (default = 0x00006000)

PCI_MSTR_IO_BUS          PCI_MSTR_IO_SIZE           32-bit PCI I/O
 (default = 0x00008000)   (default = 0x00008000)

.CE

Note: 
.sp 0
If the CPV5350 is running alone in the chassis or with a small
number of peripheral boards, the default values for
PCI_MSTR_MEMIO_SIZE, and PCI_MSTR_MEM_SIZE are sufficiently large and
need not be modified. However, modification of these values may be needed
for larger systems. As an example: If a shared memory system 
with seven MCPN750s is configured, the value for PCI_MSTR_MEMIO_SIZE 
must be increased to 0x03000000 to ensure enough space for all of 
the MCPN750s to be "visible" to the CPV5350.

The PCI memory spaces above apply only when VxWorks performs the PCI bus 
initialization (INCLUDE_PCI_AUTONCONF is #define'd in "config.h").  If 
BIOS performs the PCI configuration (INCLUDE_PCI_AUTOCONF is #undef'ed) 
then the PCI memory spaces are positioned at different addresses that are
dynamically determined by the BIOS.  The amount of PCI memory space that 
VxWorks defines is the amount required for the planar PCI devices, plus 16MB.  
By altering the size parameters mentioned above, the user can define 
PCI space from 0x40000000 to 0xFEDFFFFF.

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

.SS "Serial Configuration"
To use a COM port as the console, undef the INCLUDE_PC_CONSOLE define
in "config.h". Define CONSOLE_TTY as 0, to use COM1 as the console or
define CONSOLE_TTY as 1, to use COM2 as the console.  The serial connection
can be made through the front panel or through the rear IO of the
transition module.

.SS "Parallel Port Configuration"
The LPT_RESOURCE structure in "sysLib.c" is set up to work with the parallel
port on the CPV5350.  The VxWorks 5.4 Programmer's Guide, Appendix D explains
the format for the resource structure.  The LPT_INT_LVL is defined in
"cpv5000.h", not "pc.h" as indicated in the programmer's guide.

.SS "Network Configuration"
The network connection can be configured to come out the front
panel of the board or out the rear using the appropriate
transition module. By default the network connection comes
out the front panel network jack. In order to use the rear 
ethernet port, a BIOS setting must be modified.  In the BIOS under
"Advanced", "PCI Configuration" are two items labeled "Ethernet 1
Connection" and "Ethernet 2 Connection".  These items can be set
to "Front" or "Rear" to select the desired ethernet port location.

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
unique IP addresses. Please refer to the VxWorks reference entry for 
the exact syntax for the following commands, and to the VxWorks 
Network Programmer's Guide for general information on networking. 

.CS
ipAttach( 1, "fei")
ifAddrSet( "fei1", "xxx.xxx.xxx.xxx")  
ifMaskSet( "fei1", 0xffffff00)
routeAdd( "yyy.yyy.yyy.yyy", "xxx.xxx.xxx.xxx")
.CE

.I Note on some early boards ethernet 1, fei0, is labeled "2" on the front 
.I panel, and ethernet 2, fei1 is labeled "1".

.SS "NVRAM"
The CPV5350 has 128KB available of non-volatile RAM; boot 
parameters are preserved when the system is powered off.  
The value of the DEFAULT_BOOT_LINE definition in "config.h" will
only be used if VxWorks determines that the NVRAM is 
corrupted. VxWorks reads the first byte of NVRAM and
checks to see if it's NULL. If NULL, the DEFAULT_BOOT_LINE
will be used.

.SS "Real Time Clock Timekeeping and Alarms"
In addition to the standard VxWorks timekeeping and alarm 
features, the CPV5350 provides additional support routines
for the on-board Real Time Clock (RTC).  The RTC chip on the
CPV5350 is compatible with other available chips, which all seem 
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

Note that the RTC device on the CPV5350 will assert an alarm
interrupt approximately 2 milliseconds before it finishes 
updating the RTC date and time.  Therefore, it is possible that
sysRtcDateTimeGet() will return date/time values 1 second less
than the intended alarm time, if sysRtcDateTimeGet() is called from
within the user's connected Alarm ISR. 

The RTC features can be eliminated from the BSP by undefining
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
repeatedly driving the board's sysreset line.  The timeout lengths 
range from 17.8 msec to 291 seconds.  

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
Note that "cpv5000.h" contains defines for some device and vendor IDs,
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
visible to a CPV5350 master which is enumerating the bus until the
CPN5360 clears the "primary access lock-out" bit in the Dec2155x chip
control 0 register.  Just prior to enumerating the PCI bus in
preparation for autoconfiguration, the CPV5350 performs a secondary
bus reset which causes each CPN5360 to reset and begin executing
initialization code.  The CPV5350 immediately begins PCI bus
enumeration in preparation for autoconfiguration".  Since the CPN5360s
are busy initializing, they may not have proceeded to the point
where they have cleared the "primary access lock-out" bit and thus
would be invisible to the CPV5350 PCI bus enumeration process.
Without roll call, the CPV5350 may not see the CPN5360s and thus would
not configure them.  The roll call feature allows for bus enumeration
polling until the specified devices actually appear.  Note that roll
call may not always be required.  Some
system configurations and timings may work without using the roll call
feature.  A bit of experimentation may be necessary to determine
whether roll call is needed.  The only penalty for configuring roll
call if it is not required is a possible slight delay in coming up
after reset or power-on.

.SS "Shared Memory Support"

VxWorks shared memory support is implemented for the CPV5350.  The
example which follows shows how shared memory configurations can be
set up using Motorola CPN5360s in conjunction with the CPV5350.
Wind River documentation provides a great deal of information
regarding shared memory concepts.  For additional information
regarding the CPN5360 and shared memory support, refer to the
documentation for the CPN5360 board support package.  The following
paragraphs outline the terminology use in shared memory systems:

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
during startup initialization.  In a system consisting of a CPV5350
and one or more CPN5360s, the "host node" is always the CPV5350.
Don't confuse "host node" with "master node".  "Master node" is simply
a synonym for "node zero".  Although it is typical for the CPV5350 (host
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

.LP
The "config.h" parameters which are involved in shared memory are:

.IP "INCLUDE_SM_NET (#define or #undef)"
The parameter is #define'd by default.  It must be #undef'ed to
disable the inclusion of shared memory support code.

.IP "INCLUDE_SM_COMMON (#define or #undef)"
The parameter is #define'ed by default.  It must be #undef'ed to
disable the inclusion of shared memory object support for the VxMP product.
It is also required to support shared memory networking.

.IP "SM_OFF_BOARD (TRUE or FALSE)"
The parameter has a configurable value of either TRUE or FALSE
and directly determines the value of SM_ANCHOR_ADRS (the anchor
address).  
.LP

If SM_OFF_BOARD is FALSE then the following conditions hold: The node
number must be zero.  The location of the anchor is defined to be
LOCAL_MEM_LOCAL_ADRS + SM_ANCHOR_OFFSET.  SM_ANCHOR_OFFSET has a
default value of 0x1100 in this case.  LOCAL_MEM_LOCAL_ADRS is
defined as zero.

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
If this value is
too small, some of the devices which require nonprefetchable memory
space on the PCI bus will not be configured during PCI
autoconfiguration.  If the CPN5360s are configured to open up 4MB of
DRAM space to be accessed from the PCI bus, then a value of 0x01400000
is large enough to configure three CPN5360s in the same chassis with
the CPV5350 along with the other devices on the CPV5350 which require
PCI memory space.  To configure the maximum of seven CPN5360s would
require a PCI_MSTR_MEMIO_SIZE of 0x02400000.  See the BSP documentation
file for the CPN5360 for more information about CPN5360 configuration
in a shared memory environment with the CPV5350.

.IP "CPV5350 access of shared memory targets"
A functioning shared memory system consisting of a CPV5350 and one or
more CPN5360s 
requires that the CPV5350 be able to access the CSR
memory space of each of the CPN5360s.  
(Note that other slave boards besides the CPN5360 are possible in 
such a system, 
an example would be the MCPN750 PowerPC board.  The slave boards must
contain the Dec2155x nontransparent PCI bridge.) 
Accessing the CPN5360 CSR memory 
space from the CPV5350 is necessary to allow the CPV5350 to turn off
bus interrupts generated by the CPN5360.  During initialization, the
bootrom on the CPV5350 will probe for the CPN5360 Dec21554 nontransparent
bridge chips.  The bus probed is the compact PCI bus which is actually
bus number 2.  This bus number is specified by the #define of 
SYS_SM_CPCI_BUS_NUMBER in "cpv5000.h"  

When INCLUDE_PCI_AUTOCONFIG is #define'd to FALSE in "config.h", the BIOS
is responsible for the memory configuration of each device.  In this
mode, the CPN5360 memory which is visible to the CPV5350 is mapped via
sysMmuMapAdd() calls as the bootrom initializes.  For each sysMmuMapAdd()
call made, a DUMMY_MMU_ENTRY is used up in the "sysLib.c" table 
sysPhysMemDesc[].  There are enough DUMMY_MMU_ENTRIES in the table to
accommodate all of the on-board devices on the CPV5350 as well as seven
CPN5360s (the maximum number than can be plugged into a CPV2000 chassis).
If necessary, more DUMMY_MMU_ENTRYs can be added to this table.

.IP "Specific Shared Memory Examples"
Consider a system consisting of a CPV5350 (master node) 
and two CPN5360s.  The host node is typically a Unix node
or an Microsoft Windows NT node.  The configuration parameters
presented below affect the CPN5360s and the CPV5350.  In addition
to the proper setting of these parameters, it is also necessary to
configure the Unix or Windows NT host.  Refer to the VxWorks
documentation regarding "adding gateways to a network" for 
information on how to configure the host.

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
	Master node location    Anchor location
_
1.	CPV5350	CPV5350	
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

A brief explanation of the meaning of some of the parameters associated
with the CPN5360 when used in a shared memory configuration with the
CPV5350 memory may help in understanding shared memory addressing 
relationships.

CPCI_MSTR_MEM_BUS is an CPN5360 "config.h" #define which specifies the
actual compact PCI bus address which must be generated to access
location zero of the compact PCI host node's DRAM.  This must be set
to zero if the CPV5350 is the compact PCI host.  When the CPN5360
generates a compact PCI address of zero out on the compact PCI bus, it
will cause address zero of the CPV5350's DRAM to be accessed.

PCI_MSTR_MEMIO_SIZE is an CPN5360 "config.h" #define which specifies how much
nonprefetchable memory is available for PCI configuration.  There must
be enough memory specified to accommodate three classes of access:

.TS E
expand;
l l .
1.	T{
All CPN5360 on-board devices requesting PCI memory space.
T}
2.	T{
The compact PCI host's (cpv5350 in this case) DRAM, typically the lower 4MB.
T}
3.	T{
DRAM for each peer CPN5360, typically the lower 4MB.
T}
.TE

DEC2155X_US_MEM1_TRANS is an CPN5360 "config.h" #define which
specifies the translation address which will be added to accesses
which pass through the CPN5360 Dec21554 upstream window 1 (PCI
nonprefetchable memory).  This is the window through which accesses to
peer CPN5360 DRAM is made.  When the CPV5350 is set up for PCI
autoconfiguration, the address base for PCI nonprefetchable memory
access is 0x40000000 (governed by the CPV5350 "config.h" parameter
PCI_MEM_START).  When the CPV5350 is set up for BIOS configuration
only, then the address base depends upon the BIOS version but 0xfc000000 
is a typical value.  

DEC2155X_US_MEM1_SIZE is an CPN5360 "config.h" parameter which
specifies the amount of memory visible to the CPN5360 Dec21554 through
upstream window 1 described above.  This size must be large enough to
see all of the PCI nonprefetchable memory which the CPV5350 has
configured on the compact PCI bus in order to guarantee that each peer
CPN5360's DRAM is accessible.

The two parameters DEC2155X_US_MEM1_TRANS and DEC2155X_US_MEM1_SIZE
together define a "window" which allows the CPN5360 to look out onto 
the cPCI bus and "see" the DRAM windows of all of its peer CPN5360s.
This outbound window must be large enough to allow access to all of
the peer CPN5360's DRAM windows. The size of these individual DRAM 
windows is governed by the CPN5360 "config.h" parameter 
DEC2155X_CSR_AND_DS_MEM0_SIZE.  The default size in "config.h" is 4MB 
(0x400000).  The location on the cPCI bus where these windows are 
configured is dynamically determined and programmed by the CPV5350 during
PCI bus configuration.  As mentioned earlier, if INCLUDE_PCI_AUTOCONF
is #define'd this address is 0x40000000.  If INCLUDE_PCI_AUTOCONF is NOT 
#define'd, then the configuration address for these windows is determined
by the BIOS and can vary depending on the version of BIOS being used.
Typical values may be 0xfc000000 and 0x04000000 for
DEC2155X_US_MEM1_TRANS and DEC2155X_US_MEM1_SIZE.  This maps
the region from 0xfc000000 through 0xffffffff for nonprefetchable PCI
memory space.

A good way to find out what values to use when BIOS is doing the configuring
is to run the following experiment:

.TS E
expand;
l l .
1.	T{
Plug an CPN5360 into the chassis along with the CPV5350 and boot up 
vxWorks.st which has been configured for shared memory support.
T}
2.	T{
At the prompt on the CPV5350, perform "smNetShow()".  and take note
of the "arg2" value displayed for the slave CPU (MCPN750).  Mask this
value with 0xff000000 and it will give you the translation value to
set for DEC2155X_US_MEM1_TRANS on the CPN5360.  The BIOS will configure
additional CPN5360 boards at higher addresses so this masked value
will give you the low address base which must be used in the translation
register.
T}
.TE

.IP "1a."
CPV5350 master, sequential addressing:
.CS

   CPV5350 (node 0):

	#define 	INCLUDE_SM_NET
	#define 	SM_OFF_BOARD 		FALSE
	#define		SM_ANCHOR_OFFSET 	0x1100

     Boot parameters:

        boot device          : fei0
        processor number     : 0
        host name            : sunshine
        file name            : /tftpboot/cpv5350/vxWorks.st
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

     If the CPV5350 has #define INCLUDE_PCI_AUTOCONF

        #define DEC2155X_US_MEM1_TRANS	0x40000000
 
     else if the CPV5350 has #undef INCLUDE_PCI_AUTOCONF

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

     If the CPV5350 has #define INCLUDE_PCI_AUTOCONF

        #define DEC2155X_US_MEM1_TRANS	0x40000000
 
     else if the CPV5350 has #undef INCLUDE_PCI_AUTOCONF

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
CPV5350 master, no sequential addressing:

This example is identical to 1) except sequential addressing
is NOT enabled.  You will notice that more CPN5360 boot
parameters must be specified with sequential addressing
disabled.
.CS

   CPV5350 (node 0):

	#define 	INCLUDE_SM_NET
	#define 	SM_OFF_BOARD 		FALSE
	#define		SM_ANCHOR_OFFSET 	0x1100
	#undef		INCLUDE_SM_SEQ_ADDR


     Boot parameters:

        boot device          : fei0
        processor number     : 0
        host name            : sunshine
        file name            : /tftpboot/cpv5350/vxWorks.st
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

     If the CPV5350 has #define INCLUDE_PCI_AUTOCONF

        #define DEC2155X_US_MEM1_TRANS	0x40000000
 
     else if the CPV5350 has #undef INCLUDE_PCI_AUTOCONF

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

     If the CPV5350 has #define INCLUDE_PCI_AUTOCONF

        #define DEC2155X_US_MEM1_TRANS	0x40000000
 
     else if the CPV5350 has #undef INCLUDE_PCI_AUTOCONF

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

.IP "2."
The following example, unlike the previous ones, has the VxWorks
node zero residing on a non-system slot board, a CPN5360.  The
cPCI master, a CPV5350, is slave node 1.:
.CS

   CPV5350 (slave node 1):
	#define		INCLUDE_SM_NET 
	#define		SM_OFF_BOARD 		TRUE
	#define		SM_ANCHOR_OFFSET 	0x1100
        #undef  	INCLUDE_SM_SEQ_ADDR

     Boot parameters:

        boot device          : sm
        processor number     : 1
        host name            : sunshine
        file name            : /tftpboot/cpv5350/vxWorks.st
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

     If the CPV5350 has #define INCLUDE_PCI_AUTOCONF

        #define DEC2155X_US_MEM1_TRANS	0x40000000
 
     else if the CPV5350 has #undef INCLUDE_PCI_AUTOCONF

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
    \f3fei(0,0)\f1 - Primary on-board Ethernet 82559 device
    \f3fei(1,0)\f1 - Secondary on-board Ethernet 82559 device
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
.CS
    Note: 
    By default, with INCLUDE_ATA, vxWorks is set up for 
    one ATA hard disk device on the primary ATA controller.  
    The 2nd hard disk on the primary controller and the 
    secondary controller are normally disabled to prevent timeouts.
    If you have multiple controllers and/or hard disks
    you must modify the ataResources structure array in "sysLib.c" 
    to support more drives.
.CE

.SH "SPECIAL CONSIDERATIONS"
This section describes miscellaneous information  concerning
this BSP and its use.

.SS "Boot Issues
The boot floppy image must be copied to the floppy disk or 
the IDE disk using the DOS boot utility "mkboot.bat" or the 
VxWorks boot utilities "mkbootFd" for floppy disks or 
"mkbootAta" for hard disks.

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

Example 3: Creating a boot floppy using "mkboot.bat":

"mkboot.bat" is a MS-DOS batch file that will create a boot floppy.
This batch file must be run on a PC under a MS-DOS session.

1. On the development host, change to the BSP directory and use make
to produce the boot floppy image. Copy the resulting file to a legal
MS-DOS file name.
.CS
    % make clean
    % make bootrom
    % cp bootrom bootrom.dat
.CE
2. If your host development system is not a PC, you will need to 
transfer the executable image, "bootrom.dat", to a PC running MS-DOS.

3. Insert a blank, high-density diskette into the PC's floppy drive.

4. Execute the mkboot batch file:
.CS
    C:\> mkboot a: bootrom.dat
.CE

5. Copy the VxWorks image to the location defined by the setting of
DEFAULT_BOOT_LINE when the bootrom was made.  This could be the
floppy, network, ATA drive, etc.

6. Take the floppy out of the PC and insert
it into the floppy drive of the target and reboot the target. 

For information on the use of the DOS utility "mkboot.bat" 
and how to boot VxWorks, please refer to the
.I "VxWorks Programmer's Guide, Appendix D: Intel i386/i486."

To load VxWorks, and for more information, follow the instructions in the
.I "Tornado User's Guide: Getting Started."

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
interface.   Note that
the network interface is initialized only if the bootrom is configured
to load the vxWorks image from the network.  If only the floppy, local
ATA device, etc., is used to boot to a VxWorks prompt, then the
network interface will not be initialized.
There is no WDB agent.
T}
bootrom	T{
bootrom with compressed image which will run in lower memory.
T}
mkboot.o	T{
vxWorks utility for creating boot disks.
T}
_
.TE

.SS "Make Targets"
The BSP supports the following make targets:

.TS E
expand;
Image Name	Description	Comments
=
.sp 1
vxWorks.st_rom	T{
high memory compressed bootable vxWorks.st.
T}
.sp 1
bootrom	T{
low memory compressed bootrom.
T}
.sp 1
bootrom_uncmp	T{
low memory uncompressed bootrom.
T}
.sp 1
bootrom_high	T{
high memory compressed bootrom.
T}
.sp 1
vxWorks	standard "Tornado-style" vxWorks
.sp 1
vxWorks.st	T{
Fully linked stand-alone vxWorks
including target based shell, symbol table, and network 
interface. The network interface is not initialized.
There is no WDB agent.
T}
_
.TE

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
In "config.h", INCLUDE_END must be defined.  Erratic behavior
including extraneous reflected network packets results if INCLUDE_END
is not defined.

In "config.h" if INCLUDE_SHOW_ROUTINES is #define'ed and the DEFAULT_BOOT_LINE
parameter is longer than 163 characters, the boot code will hang and
never get to the boot prompt.  If INCLUDE_SHOW_ROUTINES is not
#define'ed then the DEFAULT_BOOT_LINE parameter can be longer than 163 
characters with no problem.

.SH SEE ALSO
.tG "Getting Started,"
.pG "Configuration"
.pG "Architecture Appendix"

.SH "BIBLIOGRAPHY"
.iB "CPV5350 CompactPCI Single Board Computer and Transition Module Installation Guide (Motorola)"
.iB "LM78 Microprocessor System Hardware Monitor (National Semiconductor)"
