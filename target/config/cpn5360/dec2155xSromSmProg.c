/* dec2155xSromProg - DEC 2155X (Drawbridge) SROM programming function */

/* Copyright 2000 Motorola, Inc. All Rights Reserved */

/*
modification history
--------------------
01b,24mar00,scb Beefed up commentary, WRS coding standards work.
01a,14dec99,scb Initial writing, derived from dec2155xCpci.c
*/

/*
DESCRIPTION

Program the Dec2155x SROM to allow configuration header initialization
to conform with #define'd values in BSP header files like "config.h".
This file depends upon the low level routines contained in "dec2155xSrom.c".

*/

/* includes */

#include "vxWorks.h"
#include "config.h"
#include "dec2155xCpci.h"

/* defines */

/* this section validates the upstream and downstream window parameters */

#if (DEC2155X_CSR_AND_DS_MEM0_SIZE & (DEC2155X_CSR_AND_DS_MEM0_SIZE - 1))
#    error DEC2155X_CSR_AND_DS_MEM0_SIZE is not a power of 2
#else
#    if (DEC2155X_CSR_AND_DS_MEM0_TRANS & (DEC2155X_CSR_AND_DS_MEM0_SIZE - 1))
#        error DEC2155X_CSR_AND_DS_MEM0_TRANS is not a multiple of window size
#    endif
#    if (DEC2155X_CSR_AND_DS_MEM0_SIZE & DEC2155X_MEM_TB_RSV_MSK)
#        error DEC2155X_CSR_AND_DS_MEM0_SIZE is less than 4KB in size
#    endif
#endif

#if (DEC2155X_DS_IO_OR_MEM1_SIZE & (DEC2155X_DS_IO_OR_MEM1_SIZE - 1))
#    error DEC2155X_DS_IO_OR_MEM1_SIZE is not a power of 2
#else
#    if (DEC2155X_DS_IO_OR_MEM1_TRANS & (DEC2155X_DS_IO_OR_MEM1_SIZE - 1))
#        error DEC2155X_DS_IO_OR_MEM1_TRANS is not a multiple of window size
#    endif
#    if (DEC2155X_DS_IO_OR_MEM1_TYPE & PCI_BAR_SPACE_IO)
#        if (DEC2155X_DS_IO_OR_MEM1_SIZE & DEC2155X_IO_OR_MEM_TB_RSV_MSK)
#            error DEC2155X_DS_IO_OR_MEM1_SIZE is less than 64 bytes
#        endif
#    else
#        if (DEC2155X_DS_IO_OR_MEM1_SIZE & DEC2155X_MEM_TB_RSV_MSK)
#            error DEC2155X_DS_IO_OR_MEM1_SIZE is less than 4KB
#        endif
#    endif        
#endif

#if (DEC2155X_DS_MEM2_SIZE & (DEC2155X_DS_MEM2_SIZE - 1))
#    error DEC2155X_DS_MEM2_SIZE is not a power of 2
#else
#    if (DEC2155X_DS_MEM2_TRANS & (DEC2155X_DS_MEM2_SIZE - 1))
#        error DEC2155X_DS_MEM2_TRANS is not a multiple of window size
#    endif
#    if (DEC2155X_DS_MEM2_SIZE & DEC2155X_MEM_TB_RSV_MSK)
#        error DEC2155X_DS_MEM2_SIZE is less than 4KB in size
#    endif
#endif

#if (DEC2155X_DS_MEM3_SIZE & (DEC2155X_DS_MEM3_SIZE - 1))
#    error DEC2155X_DS_MEM3_SIZE is not a power of 2
#else
#    if (DEC2155X_DS_MEM3_TRANS & (DEC2155X_DS_MEM3_SIZE - 1))
#        error DEC2155X_DS_MEM3_TRANS is not a multiple of window size
#    endif
#    if (DEC2155X_DS_MEM3_SIZE & DEC2155X_MEM_TB_RSV_MSK)
#        error DEC2155X_DS_MEM3_SIZE is less than 4KB in size
#    endif
#endif

#if (DEC2155X_US_IO_OR_MEM0_SIZE & (DEC2155X_US_IO_OR_MEM0_SIZE - 1))
#    error DEC2155X_US_IO_OR_MEM0_SIZE is not a power of 2
#else
#    if (DEC2155X_US_IO_OR_MEM0_TRANS & (DEC2155X_US_IO_OR_MEM0_SIZE - 1))
#        error DEC2155X_US_IO_OR_MEM0_TRANS is not a multiple of window size
#    endif
#    if (DEC2155X_US_IO_OR_MEM0_TYPE & PCI_BAR_SPACE_IO)
#        if (DEC2155X_US_IO_OR_MEM0_SIZE & DEC2155X_IO_OR_MEM_TB_RSV_MSK)
#            error DEC2155X_US_IO_OR_MEM0_SIZE is less than 64 bytes
#        endif
#    else
#        if (DEC2155X_US_IO_OR_MEM0_SIZE & DEC2155X_MEM_TB_RSV_MSK)
#            error DEC2155X_US_IO_OR_MEM0_SIZE is less than 4KB
#        endif
#    endif        
#endif

#if (DEC2155X_US_MEM1_SIZE & (DEC2155X_US_MEM1_SIZE - 1))
#    error DEC2155X_US_MEM1_SIZE is not a power of 2
#else
#    if (DEC2155X_US_MEM1_TRANS & (DEC2155X_US_MEM1_SIZE - 1))
#        error DEC2155X_US_MEM1_TRANS is not a multiple of window size
#    endif
#    if (DEC2155X_US_MEM1_SIZE & DEC2155X_MEM_TB_RSV_MSK)
#        error DEC2155X_US_MEM1_SIZE is less than 4KB in size
#    endif
#endif

/* create window setup values */

#if (DEC2155X_CSR_AND_DS_MEM0_SIZE)
#   define DEC2155X_CSR_AND_DS_MEM0_SETUP ( \
           DEC2155X_SETUP_REG_BAR_ENABLE        | \
           ~(DEC2155X_CSR_AND_DS_MEM0_SIZE - 1) | \
           DEC2155X_CSR_AND_DS_MEM0_TYPE)
#else
#   define DEC2155X_CSR_AND_DS_MEM0_SETUP 0x00000000
#endif

#if (DEC2155X_DS_IO_OR_MEM1_SIZE)
#   define DEC2155X_DS_IO_OR_MEM1_SETUP ( \
           DEC2155X_SETUP_REG_BAR_ENABLE      | \
           ~(DEC2155X_DS_IO_OR_MEM1_SIZE - 1) | \
           DEC2155X_DS_IO_OR_MEM1_TYPE)
#else
#   define DEC2155X_DS_IO_OR_MEM1_SETUP 0x00000000
#endif

#if (DEC2155X_DS_MEM2_SIZE)
#   define DEC2155X_DS_MEM2_SETUP ( \
           DEC2155X_SETUP_REG_BAR_ENABLE | \
           ~(DEC2155X_DS_MEM2_SIZE - 1)  | \
           DEC2155X_DS_MEM2_TYPE)
#else
#   define DEC2155X_DS_MEM2_SETUP 0x00000000
#endif

#if (DEC2155X_DS_MEM3_SIZE)
#   define DEC2155X_DS_MEM3_SETUP ( \
           DEC2155X_SETUP_REG_BAR_ENABLE | \
           ~(DEC2155X_DS_MEM3_SIZE - 1)  | \
           DEC2155X_DS_MEM3_TYPE)
#else
#   define DEC2155X_DS_MEM3_SETUP 0x00000000
#endif

#if (DEC2155X_US_IO_OR_MEM0_SIZE)
#   define DEC2155X_US_IO_OR_MEM0_SETUP ( \
           DEC2155X_SETUP_REG_BAR_ENABLE      | \
           ~(DEC2155X_US_IO_OR_MEM0_SIZE - 1) | \
           DEC2155X_US_IO_OR_MEM0_TYPE)
#else
#   define DEC2155X_US_IO_OR_MEM0_SETUP 0x00000000
#endif

#if (DEC2155X_US_MEM1_SIZE)
#   define DEC2155X_US_MEM1_SETUP ( \
           DEC2155X_SETUP_REG_BAR_ENABLE | \
           ~(DEC2155X_US_MEM1_SIZE - 1)  | \
           DEC2155X_US_MEM1_TYPE)
#else
#   define DEC2155X_US_MEM1_SETUP 0x00000000
#endif

#define DEC2155X_UPR32_DS_MEM3_SETUP 0x00000000

/* SROM access macros */

#define DEC2155X_SROM_BYTE_WRITE(offset, data) \
	{ \
	printf("\r %3.3x: ", offset); \
	printf("%2.2x", data); \
	if (sysDec2155xOffsetWrite (dec2155xPciDevNumber, \
	    offset, data) == ERROR) \
	    { \
	    printf("\n"); \
            return (ERROR); \
	    } \
        }

#define DEC2155X_SROM_WORD_WRITE(offset, data) \
	{ \
	if (dec2155xSromWordWrite (dec2155xPciDevNumber, \
	    offset, data) == ERROR) \
	    { \
	    printf("\n"); \
            return (ERROR); \
	    } \
        }

#define DEC2155X_SROM_LONG_WRITE(offset, data) \
	{ \
	if (dec2155xSromLongWrite (dec2155xPciDevNumber, \
	    offset, data) == ERROR) \
	    { \
	    printf("\n"); \
            return (ERROR); \
	    } \
        }

/* forward declarations */

LOCAL STATUS dec2155xSromLongWrite (UINT, UINT, UINT32);
LOCAL STATUS dec2155xSromWordWrite (UINT, UINT, UINT16);

/* external declarations */

IMPORT STATUS sysDec2155xOffsetWrite (UINT devSel, UINT offset, UINT8 value);

/* globals */

UINT dec2155xPciDevNumber;

/******************************************************************************
*
* sysDec2155xSromSmProg - Initialize the DEC2155x SROM.
*
* This routine initializes the SROM of the Dec2155x (Drawbridge) chip
* in such a manner that the values defined in "config.h" will be
* programmed into the configuration header at startup.  This function will
* typically be used to program the Dec2155x SROM to support VxWorks shared 
* memory.  The function enables the serial ROM programming bit and clears 
* the primary access lockout bit.  This function is included in the
* "dec2155xSrom" utility when compiled with INCLUDE_DEC2155X #defined in
* "config.h".
*
* RETURNS: OK on success, else ERROR (SROM may be partially reprogrammed).
*/

STATUS sysDec2155xSromSmProg (void)
    {

    dec2155xPciDevNumber = DEC2155X_PCI_DEV_NUMBER;

    /* Program serial preload enable */

    DEC2155X_SROM_BYTE_WRITE (DEC2155X_SROM_SER_PRELOAD, 
			      DEC2155X_SROM_SER_PRELOAD_ENABLE_VAL);

    /* configure the primary and secondary SERR disables */

    DEC2155X_SROM_BYTE_WRITE (DEC2155X_SROM_PRI_SERR_DISABLES,
                              DEC2155X_PRI_SERR_VAL);

    DEC2155X_SROM_BYTE_WRITE (DEC2155X_SROM_SEC_SERR_DISABLES,
                              DEC2155X_SEC_SERR_VAL);

    /* configure primary class register */

    DEC2155X_SROM_BYTE_WRITE (DEC2155X_SROM_PRI_PROGRAMMING_IF,
                              DEC2155X_PRI_PRG_IF_VAL);
    DEC2155X_SROM_BYTE_WRITE (DEC2155X_SROM_PRI_SUBCLASS,
                              DEC2155X_PRI_SUBCLASS_VAL);
    DEC2155X_SROM_BYTE_WRITE (DEC2155X_SROM_PRI_CLASS,
                              DEC2155X_PRI_CLASS_VAL);

    /* configure bist register (shared) */

    DEC2155X_SROM_BYTE_WRITE (DEC2155X_SROM_BIST, DEC2155X_BIST_VAL);
            
    /* configure the downstream windows */

    DEC2155X_SROM_LONG_WRITE (DEC2155X_SROM_DS_MEM0_SETUP,
			      DEC2155X_CSR_AND_DS_MEM0_SETUP);

    DEC2155X_SROM_LONG_WRITE (DEC2155X_SROM_DS_IO_OR_MEM1_SETUP,
                              DEC2155X_DS_IO_OR_MEM1_SETUP);

    DEC2155X_SROM_LONG_WRITE (DEC2155X_SROM_DS_MEM2_SETUP,
                              DEC2155X_DS_MEM2_SETUP);

    DEC2155X_SROM_LONG_WRITE (DEC2155X_SROM_DS_MEM3_SETUP,
                              DEC2155X_DS_MEM3_SETUP);

    DEC2155X_SROM_LONG_WRITE (DEC2155X_SROM_UPR32_BITS_DS_MEM3_SU,
	                      DEC2155X_UPR32_DS_MEM3_SETUP);

    /* configure primary expansion ROM setup */

    DEC2155X_SROM_WORD_WRITE (DEC2155X_SROM_PRI_EXP_ROM_SETUP,
			      DEC2155X_SROM_PRI_EXP_ROM_SETUP_VAL);

    /* configure subsystem and vendor id (shared) */

    DEC2155X_SROM_WORD_WRITE (DEC2155X_SROM_SUB_VENDER_ID, 
			      DEC2155X_SUB_VNDR_ID_VAL);

    DEC2155X_SROM_WORD_WRITE (DEC2155X_SROM_SUB_SYSTEM_ID, 
			      DEC2155X_SUB_SYS_ID_VAL);

    /* configure the primary max latency and minimum grant registers */

    DEC2155X_SROM_BYTE_WRITE (DEC2155X_SROM_PRI_MAX_LATENCY, 
			      DEC2155X_MAX_LAT_VAL);

    DEC2155X_SROM_BYTE_WRITE (DEC2155X_SROM_PRI_MIN_GRANT, 
			      DEC2155X_MIN_GNT_VAL);

    /* configure chip ctrl 0, clear primary lock-out */

    DEC2155X_SROM_WORD_WRITE (DEC2155X_SROM_CHP_CTRL0, 
			      DEC2155X_CHP_CTRL0_VAL);

    /* configure chip control 1 */

    DEC2155X_SROM_WORD_WRITE (DEC2155X_SROM_CHP_CTRL1,
                              DEC2155X_CHP_CTRL1_VAL);

    /* configure secondary class register */

    DEC2155X_SROM_BYTE_WRITE (DEC2155X_SROM_SEC_PROGRAMMING_IF,
                              DEC2155X_SEC_PRG_IF_VAL);

    DEC2155X_SROM_BYTE_WRITE (DEC2155X_SROM_SEC_SUBCLASS,
                              DEC2155X_SEC_SUBCLASS_VAL);

    DEC2155X_SROM_BYTE_WRITE (DEC2155X_SROM_SEC_CLASS,
                              DEC2155X_SEC_CLASS_VAL);

    /* configure upstream windows */

    DEC2155X_SROM_LONG_WRITE (DEC2155X_SROM_US_IO_OR_MEM0_SETUP,
                              DEC2155X_US_IO_OR_MEM0_SETUP);

    DEC2155X_SROM_LONG_WRITE (DEC2155X_SROM_US_MEM1_SETUP,
                              DEC2155X_US_MEM1_SETUP);

    /* configure secondary max latency and minimum grant */

    DEC2155X_SROM_BYTE_WRITE (DEC2155X_SROM_SEC_MAX_LATENCY, 
			      DEC2155X_MAX_LAT_VAL);

    DEC2155X_SROM_BYTE_WRITE (DEC2155X_SROM_SEC_MIN_GRANT, 
			      DEC2155X_MIN_GNT_VAL);

    /* configure secondary (local) bus arbiter */

    DEC2155X_SROM_WORD_WRITE (DEC2155X_SROM_ARB_CTRL,
                              DEC2155X_ARB_CTRL_VAL);

    /* configure power management registers */

    DEC2155X_SROM_BYTE_WRITE (DEC2155X_SROM_PWR_MGMT_0,
			      DEC2155X_SROM_PWR_MGMT_0_VAL);

    DEC2155X_SROM_BYTE_WRITE (DEC2155X_SROM_PWR_MGMT_1,
			      DEC2155X_SROM_PWR_MGMT_1_VAL);

    DEC2155X_SROM_BYTE_WRITE (DEC2155X_SROM_PWR_MGMT_2,
			      DEC2155X_SROM_PWR_MGMT_2_VAL);

    DEC2155X_SROM_BYTE_WRITE (DEC2155X_SROM_PWR_MGMT_3,
			      DEC2155X_SROM_PWR_MGMT_3_VAL);

    DEC2155X_SROM_BYTE_WRITE (DEC2155X_SROM_PWR_MGMT_4,
			      DEC2155X_SROM_PWR_MGMT_4_VAL);

    DEC2155X_SROM_BYTE_WRITE (DEC2155X_SROM_PWR_MGMT_5,
			      DEC2155X_SROM_PWR_MGMT_5_VAL);

    DEC2155X_SROM_BYTE_WRITE (DEC2155X_SROM_PWR_MGMT_6,
			      DEC2155X_SROM_PWR_MGMT_6_VAL);

    DEC2155X_SROM_BYTE_WRITE (DEC2155X_SROM_PWR_MGMT_7,
			      DEC2155X_SROM_PWR_MGMT_7_VAL);

    DEC2155X_SROM_BYTE_WRITE (DEC2155X_SROM_PWR_MGMT_CR,
			      DEC2155X_SROM_PWR_MGMT_CR_VAL);

    /* return status based on error flag */

    printf("\n");
    return (OK);
    }

/******************************************************************************
*
* dec2155xSromLongWrite - write long (32 bits) into the SROM
*
* This routine writes the long (32 bit) value specified into the SROM at
* the offset specified.
*
* RETURNS: OK on success, else ERROR (SROM may be partially reprogrammed).
*/

LOCAL STATUS dec2155xSromLongWrite
    (
    UINT   devSel,	/* PCI device number of	the Dec2155x */
    UINT   offset,	/* Byte	offset at which	to write the data */
    UINT32 data		/* 32-bit data item to write to	SROM */
    )
    {
    UINT8 tempData;
    int	  i;
    for	(i = 0;	i < 4; i++)
	{
	tempData = data	& 0xff;
	DEC2155X_SROM_BYTE_WRITE (offset + i, tempData)
	data = data >> 8;
	}
    return (OK);
    }

/******************************************************************************
*
* dec2155xSromWordWrite - write word (16 bits) into the SROM
*
* This routine writes the word (16 bit) value specified into the SROM at
* the offset specified.
*
* RETURNS: OK on success, else ERROR (SROM may be partially reprogrammed).
*/

LOCAL STATUS dec2155xSromWordWrite
    (
    UINT   devSel,	/* PCI device number of	the Dec2155x */
    UINT   offset,	/* Byte	offset at which	to write the data */
    UINT16 data		/* 16-bit data item to write to	SROM */
    )
    {
    UINT8 tempData;
    int	  i;
    for	(i = 0;	i < 2; i++)
	{
	tempData = data	& 0xff;
	DEC2155X_SROM_BYTE_WRITE (offset + i, tempData);
	data = data >> 8;
	}
    return (OK);
    }
