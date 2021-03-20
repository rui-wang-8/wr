/* cpuid.h - I80x86 registers */

/* Copyright 1999-2002 Wind River Systems, Inc. */
/* Copyright 1999-2000 Motorola, Inc. All Rights Reserved */

/*
modification history
--------------------
01b,20mar00,djs  Initial code review changes.
01a,19aug99,scb  initial writing
*/

#ifndef	__INCcpuidh
#define	__INCcpuidh

#ifdef __cplusplus
extern "C" {
#endif

/*
 * "cpuid" is an Intel assembly language instruction used to 
 * identify the type of processor that we're running on.  The 
 * identification includes the vendor e.g. Intel or AMD, the 
 * family e.g. "P5" or "P6" type processor, the model e.g. 
 * "Pentium PRO" or "Pentium II", and the stepping identifier.
 *
 * The information that follows aids in decoding the data returned
 * by the "cpuid" instruction.
 */

/* CPUID Vendor ID - "GenuineIntel" */

#define CPUID_VENDOR_INTEL_ECX 0x6c65746e      /* letn */
#define CPUID_VENDOR_INTEL_EDX 0x49656e69      /* Ieni */
#define CPUID_VENDOR_INTEL_EBX 0x756e6547      /* uneG */

/* CPUID Vendir ID - "AuthenticAMD" */

#define CPUID_VENDOR_AMD_ECX 0x444d4163        /* DMAc */
#define CPUID_VENDOR_AMD_EDX 0x69746e65        /* itne */
#define CPUID_VENDOR_AMD_EBX 0x68747541        /* htuA */

/* Model identifiers associated with "P6" processors */

#define CPUID_INTEL_MODEL_P6M1  0x00000010  /* Pentium PRO */
#define CPUID_INTEL_MODEL_P6M3  0x00000030  /* Pentium II, model 3 */
#define CPUID_INTEL_MODEL_P6M5  0x00000050  /* Pentium II, model 5 & Celeron */
#define CPUID_INTEL_MODEL_P6M6  0x00000060  /* Celeron, model 6 */
#define CPUID_INTEL_MODEL_P6M7  0x00000070  /* Pentium III */
#define CPUID_INTEL_MODEL_P6M8  0x00000080

/* Descriptor decode values (from CPUID(2) ) */

#define CPUID_DESCRIPTOR_NOCACHE 0x40 /* Discriminator for Celeron, model 5 */

#ifdef __cplusplus
}
#endif

#endif	/* __INCcpuidh */
