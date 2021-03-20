/* configNet.h - Network configuration header file */

/* Copyright 1984-2002 Wind River Systems, Inc. */
/* Copyright 1999 Motorola, Inc., All Rights Reserved */

/*
modification history
--------------------
01d,14jun02,rhe  Add C++ Protection
01c,01feb00,djs  incorporate WRS review comments.
01b,14oct99,sjb  Added fei1 element to endDevTbl[]
01a,14oct99,sjb  written based on pcPentium version 01a,31mar98,cn.
*/

#ifndef INCconfigNeth
#define INCconfigNeth

#ifdef __cplusplus
extern "C" {
#endif

#include "vxWorks.h"
#include "end.h"

#ifdef INCLUDE_FEI_END

#define FEI82557_LOAD_FUNC   fei82557EndLoad     /* driver external interface */
#define FEI82557_BUFF_LOAN   1                   /* enable buffer loaning */

/*
 * The fei82557End initialization string format is:
 *
 * <memBase>:<memSize>:<nCFDs>:<nRFDs>:<userFlags>
 */

#define FEI82557_LOAD_STRING "-1:0x00:0x20:0x20:0x00"

IMPORT END_OBJ* FEI82557_LOAD_FUNC (char*, void*);

#endif /* INCLUDE_FEI_END */

/* max number of SENS ipAttachments we can have */

#ifndef IP_MAX_UNITS
#   define IP_MAX_UNITS (NELEMENTS (endDevTbl) - 1)
#endif
 
END_TBL_ENTRY endDevTbl [] =
    {
#ifdef INCLUDE_FEI_END
    { 0, FEI82557_LOAD_FUNC, FEI82557_LOAD_STRING, FEI82557_BUFF_LOAN,
	NULL, FALSE},
#ifdef INCLUDE_SECONDARY_ENET
    { 1, FEI82557_LOAD_FUNC, FEI82557_LOAD_STRING, FEI82557_BUFF_LOAN,
	NULL, FALSE},
#endif /* INCLUDE_SECONDARY_ENET */
#endif /* INCLUDE_FEI_END */

    { 0, END_TBL_END, NULL, 0, NULL, FALSE},
    };

#ifdef __cplusplus
}
#endif

#endif /* INCconfigNeth */

