/* lm78.h - LM78 support header */
/* Copyright 1999-2002 Wind River Systems, Inc. */
/* Copyright 1999-2000 Motorola, Inc. All Rights Reserved */

/*
modification history
--------------------
01c,20mar00,djs  Initial code review changes. Brought over cpv5000 version
01b,11feb00,scb  Incorporate WRS code review changes.
01a,29jul99,scb  Initial Development.
*/

/*
DESCRIPTION
This file contains definitions for the LM78 register offsets,
bit fields, structure typedefs and global functions.
*/

#ifndef INClm78h
#define INClm78h

#ifdef __cplusplus
extern "C" {
#endif

/* defines */

/* 
 * The following two constants were obtained from the LM78 specification
 * The first: LM78_REV_COUNT_PER_MINUTE refers the to numerator of the
 * function relating count to RPM and divisor:
 *
 * Count = (1.35 x 10E6)/(RPM x divisor)
 *
 * The second LM78_VOLTS_PER_COUNT is the amount the voltage change 
 * associated with a change of one in the "raw count value" 
 * representing voltage.
 */

#define LM78_REV_COUNT_PER_MINUTE 	(1350000)	
#define LM78_VOLTS_PER_COUNT		(0.016)	

/* 
 * The following two #define's require LM78_ISA_BASE_ADRS
 * which is defined in the <BSP>.h file e.g. "cpn5360.h"
 */

#define LM78_INDEX               ( LM78_ISA_BASE_ADRS + 5 )
#define LM78_DATA                ( LM78_ISA_BASE_ADRS + 6 )

/* Register locations and boundaries */

#define LM78_REG_LOW		0x0
#define LM78_REG_HIGH		0x7f
#define LM78_POST_RAM_LOW	0x0
#define LM78_POST_RAM_HIGH	0x1f
#define LM78_VALUE_RAM_LOW	0x20
#define LM78_VALUE_RAM_HIGH	0x3f
#define LM78_VALUE_MIRROR_LOW   0x60
#define LM78_VALUE_MIRROR_HIGH  0x7f

#define LM78_CONFIG		0x40
#define LM78_INTSTAT1		0x41
#define LM78_INTSTAT2		0x42
#define LM78_SMI_MASK1		0x43
#define LM78_SMI_MASK2		0x44
#define LM78_NMI_MASK1		0x45
#define LM78_NMI_MASK2		0x46
#define LM78_VID_FAN_DIV	0x47
#define LM78_SER_BUS_ADDR	0x48
#define LM78_CHIP_RESET_ID 	0x49

/* Configuration status register bits */

#define LM78_CONFIG_BITS_START      0x01
#define LM78_CONFIG_BITS_SMI_EN     0x02
#define LM78_CONFIG_BITS_NMIIRQ_EN  0x04
#define LM78_CONFIG_BITS_INT_CLEAR  0x08
#define LM78_CONFIG_BITS_RESET      0x10
#define LM78_CONFIG_BITS_NMIIRQ_SEL 0x20
#define LM78_CONFIG_BITS_PWR_BYPASS 0x40
#define LM78_CONFIG_BITS_INITIALIZE 0x40

/* Interrupt status 1 register bits */

#define LM78_INTSTAT1_BITS_IN0		0x01
#define LM78_INTSTAT1_BITS_IN1		0x02
#define LM78_INTSTAT1_BITS_IN2		0x04
#define LM78_INTSTAT1_BITS_IN3		0x08
#define LM78_INTSTAT1_BITS_TEMPERATURE	0x10
#define LM78_INTSTAT1_BITS_BTI		0x20
#define LM78_INTSTAT1_BITS_FAN1		0x40
#define LM78_INTSTAT1_BITS_FAN2		0x80

/* Interrupt status 2 register bits */

#define LM78_INTSTAT2_BITS_IN4			0x01
#define LM78_INTSTAT2_BITS_IN5			0x02
#define LM78_INTSTAT2_BITS_IN6			0x04
#define LM78_INTSTAT2_BITS_FAN3			0x08
#define LM78_INTSTAT2_BITS_CHASSIS_INTRUSION	0x10	/* Chassis intrusion */
#define LM78_INTSTAT2_BITS_FIFO_OVERFLOW     	0x20	/* FIFO overflow */
#define LM78_INTSTAT2_BITS_SMI_IN       	0x40
#define LM78_INTSTAT2_BITS_RESERVED     	0x80

/* SMI# Mask Register 1 bits */

#define LM78_SMI_MASK1_BITS_IN0		0x01
#define LM78_SMI_MASK1_BITS_IN1		0x02
#define LM78_SMI_MASK1_BITS_IN2		0x04
#define LM78_SMI_MASK1_BITS_IN3		0x08
#define LM78_SMI_MASK1_BITS_TEMPERATURE	0x10
#define LM78_SMI_MASK1_BITS_BTI		0x20
#define LM78_SMI_MASK1_BITS_FAN1	0x40
#define LM78_SMI_MASK1_BITS_FAN2	0x80

/* SMI# Mask Register 2 bits */

#define LM78_SMI_MASK2_BITS_IN4			0x01
#define LM78_SMI_MASK2_BITS_IN5			0x02
#define LM78_SMI_MASK2_BITS_IN6			0x04
#define LM78_SMI_MASK2_BITS_FAN3		0x08
#define LM78_SMI_MASK2_BITS_CHASSIS_INTRUSION	0x10	/* Chassis intrusion */
#define LM78_SMI_MASK2_BITS_FIFO_OVERFLOW     	0x20	/* FIFO overflow */
#define LM78_SMI_MASK2_BITS_SMI_IN       	0x40
#define LM78_SMI_MASK2_BITS_RESERVED     	0x80

/* NMI# Mask Register 1 bits */

#define LM78_NMI_MASK1_BITS_IN0		0x01
#define LM78_NMI_MASK1_BITS_IN1		0x02
#define LM78_NMI_MASK1_BITS_IN2		0x04
#define LM78_NMI_MASK1_BITS_IN3		0x08
#define LM78_NMI_MASK1_BITS_TEMPERATURE	0x10
#define LM78_NMI_MASK1_BITS_BTI		0x20
#define LM78_NMI_MASK1_BITS_FAN1	0x40
#define LM78_NMI_MASK1_BITS_FAN2	0x80

/* NMI# Mask Register 2 bits */

#define LM78_NMI_MASK2_BITS_IN4			0x01
#define LM78_NMI_MASK2_BITS_IN5			0x02
#define LM78_NMI_MASK2_BITS_IN6			0x04
#define LM78_NMI_MASK2_BITS_FAN3		0x08
#define LM78_NMI_MASK2_BITS_CHASSIS_INTRUSION	0x10	/* Chassis intrusion */
#define LM78_NMI_MASK2_BITS_FIFO_OVERFLOW     	0x20	/* FIFO overflow */
#define LM78_NMI_MASK2_BITS_SMI_IN       	0x40
#define LM78_NMI_MASK2_BITS_RESERVED     	0x80

/* VID/Fan Divisor Register masks and bits */

#define LM78_VID_FAN_MASK_VID           0x07
#define LM78_VID_FAN_BITS_VID1		0x01
#define LM78_VID_FAN_BITS_VID2		0x02
#define LM78_VID_FAN_BITS_VID3		0x04
#define LM78_VID_FAN_MASK_FAN1_DIV      0x30
#define LM78_VID_FAN_MASK_FAN2_DIV      0xc0

/* Serial Bus Address Register mask */

#define LM78_SER_BUS_ADDR_MASK		0x7F		

/* Chip Reset/ID Register bits */

#define LM78_CHIP_RESET_ID_RESET 	0x10
#define LM78_CHIP_RESET_ID_DEVID 	0x20

/* Power-On Self-Test (POST) related */

#define LM78_POST_RAM_BASE	0x0
#define LM78_POST_RAM_LIMIT	0x1f

/* Offsets within raw value RAM  - current readings */

#define LM78_VALUE_IN0          0x0
#define LM78_VALUE_IN1          0x1
#define LM78_VALUE_IN2          0x2
#define LM78_VALUE_IN3          0x3
#define LM78_VALUE_IN4          0x4
#define LM78_VALUE_IN5          0x5
#define LM78_VALUE_IN6          0x6
#define LM78_VALUE_TEMPERATURE  0x7
#define LM78_VALUE_FAN1		0x8
#define LM78_VALUE_FAN2		0x9
#define LM78_VALUE_FAN3		0xa

/* Offsets within raw value RAM  - limit values */

#define LM78_VALUE_IN0_HIGH		0xb
#define LM78_VALUE_IN0_LOW		0xc
#define LM78_VALUE_IN1_HIGH		0xd
#define LM78_VALUE_IN1_LOW		0xe
#define LM78_VALUE_IN2_HIGH		0xf
#define LM78_VALUE_IN2_LOW		0x10
#define LM78_VALUE_IN3_HIGH		0x11
#define LM78_VALUE_IN3_LOW      	0x12
#define LM78_VALUE_IN4_HIGH     	0x13
#define LM78_VALUE_IN4_LOW      	0x14
#define LM78_VALUE_IN5_HIGH		0x15
#define LM78_VALUE_IN5_LOW		0x16
#define LM78_VALUE_IN6_HIGH		0x17
#define LM78_VALUE_IN6_LOW		0x18
#define LM78_VALUE_TEMPERATURE_HIGH	0x19
#define LM78_VALUE_TEMPERATURE_LOW	0x1a
#define LM78_VALUE_FAN1_LOW		0x1b
#define LM78_VALUE_FAN2_LOW		0x1c
#define LM78_VALUE_FAN3_LOW		0x1d

/* typedefs */

/* Raw Status */

typedef struct
    {
    UINT8 intStat1;
    UINT8 intStat2;
    UINT8 postRam  [ LM78_POST_RAM_HIGH - LM78_POST_RAM_LOW + 1 ];
    UINT8 valueRam [ LM78_VALUE_RAM_HIGH - LM78_VALUE_RAM_LOW + 1 ];
    } LM78_RAW_STAT;

/* Formatted exceeded values (from interrupt stat 1 & 2 registers) */

typedef struct
    {
    BOOL in0;
    BOOL in1;
    BOOL in2;
    BOOL in3;
    BOOL in4;
    BOOL in5;
    BOOL in6;
    BOOL temperature;
    BOOL bti;
    BOOL fan1;
    BOOL fan2;
    BOOL fan3;
    BOOL chassisIntrusion;
    BOOL fifoOverflow;
    BOOL smiIn;
    } LM78_FORMAT_EXCEEDED;

/* Formatted current values (readings) */

typedef struct
    {
    float in0;		/* volts */
    float in1;
    float in2;
    float in3;
    float in4;
    float in5;
    float in6;
    int   temperature;	/* degrees C */
    int   fan1;		/* RPM */
    int   fan2;
    int   fan3;
    } LM78_FORMAT_VALUES;

/* Formatted limit values */

typedef struct
    {
    float in0High;
    float in0Low;
    float in1High;
    float in1Low;
    float in2High;
    float in2Low;
    float in3High;
    float in3Low;
    float in4High;
    float in4Low;
    float in5High;
    float in5Low;
    float in6High;
    float in6Low;
    int   temperatureHigh;
    int   temperatureLow;
    int   fan1Low;
    int   fan2Low;
    int   fan3Low;
    } LM78_FORMAT_LIMITS;

/* Formatted status */

typedef struct
    {
    LM78_FORMAT_EXCEEDED exceed;
    LM78_FORMAT_VALUES   values; 
    LM78_FORMAT_LIMITS   limits;
    } LM78_FORMAT_STAT;

/* globals */

extern void   lm78Init (void);
extern float  lm78RawToPlusVolt (UINT8 rawVal, float r1Val, float r2Val);
extern UINT8  lm78PlusVoltToRaw (float Vs, float r1Val, float r2Val);
extern float  lm78RawToMinusVolt (UINT8 rawVal, float rInVal, float rFVal);
extern UINT8  lm78MinusVoltToRaw (float Vs, float rInVal, float rFVal);
extern int    lm78RawToTemperature (UINT8 rawVal);
extern UINT8  lm78TemperatureToRaw (int temperature);
extern UINT   lm78RawToRPM (UINT8 rawVal, UINT  divisor);
extern UINT8  lm78RPMToRaw (UINT  rpm, UINT  divisor);
extern STATUS lm78MonitorOn (void);
extern STATUS lm78MonitorOff (void);
extern STATUS lm78RegGet (UINT regId, UINT8 * regVal);
extern STATUS lm78RegSet (UINT regId, UINT8 regVal);
extern STATUS lm78RawStatusGet (LM78_RAW_STAT * rawStat);
extern STATUS lm78RawStatusSet (LM78_RAW_STAT * rawStat);
extern void   lm78RawStatusFormat (LM78_RAW_STAT * rawStat, 
			           LM78_FORMAT_STAT * formatStat);

#ifdef __cplusplus
}
#endif

#endif	/* INClm78h */
