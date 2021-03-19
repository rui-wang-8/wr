/* cdromFsLib.h - ISO 9660 File System definitions */

/* Copyright 1984-1995 Wind River Systems, Inc. */

/*
modification history
--------------------
01b,07dec98,lrn  add C++ support (SPR#23776)
01a,10apr97,dds  SPR#7538: add CDROM file system support to vxWorks.
*/

/*
 * currently only ISO 9660 is supported. All data dependent on this 
 * standard has prefix "ISO", or "iso" or "Iso"
 */

#ifndef cdromFsLib_h
#define cdromFsLib_h

#ifdef __cplusplus
extern "C" {
#endif

#include <vxWorks.h>
#include <iosLib.h>
#include <lstLib.h>
#include <semLib.h>
#include <blkIo.h>	/* for BLK_DEV */
#include <time.h>

#define KB			*1024

#define CDROM_STD_LS_SIZE	(2 KB) /* standard cdrom logical sector size */

#if (_BYTE_ORDER == _LITTLE_ENDIAN)
#define BYTE_ORDER_LITTLE_ENDIAN
#define ENDIAN_OFF_SHORT	0
#define ENDIAN_OFF_LONG		0
#elif (_BYTE_ORDER == _BIG_ENDIAN)
#define BYTE_ORDER_BIG_ENDIAN
#define ENDIAN_OFF_SHORT	2
#define ENDIAN_OFF_LONG		4
#else
#error "_BYTE_ORDER must be defined, check compilation flags"
#endif /* (_BYTE_ORDER == _LITTLE_ENDIAN) */

#define LEN32 4 /* for fields reading in CD data descriptors */
#define LEN16 2

#define VD_SET_MAG	(0x06200556)
#define VD_LIST_MAG	(0x03110364)
#define FD_MAG		(0x08211061)
#define SEC_BUF_MAG	(0x05071163)

/* ----------- ISO Volume Descriptors ----------- */

/* constants */

#define ISO_PVD_BASE_LS	16
#define ISO_STD_ID	"CD001"	/* must be in each VD */
#define ISO_STD_ID_SIZE	5	/* must be in each VD */
#define ISO_VD_SIZE	2048

/* VD types: */
#define ISO_VD_BOOT	((u_char)0)
#define ISO_VD_PRIMARY	((u_char)1)
#define ISO_VD_SUPPLEM	((u_char)2)
#define ISO_VD_PARTN	((u_char)3)
#define ISO_VD_SETTERM	((u_char)255)

#define ISO_VD_VERSION	((u_char)1)

/* VD header, may be used as mask, since byte order is stored */
typedef struct isoVdHead
    {
    u_char	type;			/* any of VD types */
    u_char	stdID[ ISO_STD_ID_SIZE ];	/* have to be ISO_STD_ID */
    u_char	version;		/* have to be ISO_VD_VERSION */
    } T_ISO_VD_HEAD;

typedef T_ISO_VD_HEAD *	T_ISO_VD_HEAD_ID;

/*
 * VD-descriptor structure fields data types 
 * volume date-time fields sizes and offsets 
 */

#define ISO_V_DATE_TIME_YEAR	       		0
#define ISO_V_DATE_TIME_YEAR_SIZE		4
#define ISO_V_DATE_TIME_FIELD_STD_SIZE		2
#define ISO_V_DATE_TIME_MONTH			4
#define ISO_V_DATE_TIME_DAY			6
#define ISO_V_DATE_TIME_HOUR			8
#define ISO_V_DATE_TIME_MINUTE			10
#define ISO_V_DATE_TIME_SEC			12
#define ISO_V_DATE_TIME_100_OF_SEC		14
#define ISO_V_DATE_TIME_FROM_GREENW_OFF		16
#define ISO_V_DATE_TIME_FROM_GREENW_NIMUTE	15

typedef struct isoVDDateTime
    {
    u_char	year[ ISO_V_DATE_TIME_YEAR_SIZE ];		/* 1-9999 */
    u_char	month[ ISO_V_DATE_TIME_FIELD_STD_SIZE ];	/* 1-12 */
    u_char	day[ ISO_V_DATE_TIME_FIELD_STD_SIZE ];		/* 1-31 */
    u_char	hour[ ISO_V_DATE_TIME_FIELD_STD_SIZE ];		/* 0-23 */
    u_char	minute[ ISO_V_DATE_TIME_FIELD_STD_SIZE ];	/* 0-59 */
    u_char	sec[ ISO_V_DATE_TIME_FIELD_STD_SIZE ];		/* 0-59 */
    u_char	sec100[ ISO_V_DATE_TIME_FIELD_STD_SIZE ];	/* 0-99 */
    u_char	greenwOffBy15Minute;
    } T_ISO_VD_DATE_TIME;

typedef T_ISO_VD_DATE_TIME *	T_ISO_VD_DATE_TIME_ID;

/* PVD and SVD fields offsets and sizes */

#define	ISO_VD_SYSTEM_ID	8
#define	ISO_VD_SYSTEM_ID_SIZE	(40-8)
#define	ISO_VD_VOLUME_ID	40
#define	ISO_VD_VOLUME_ID_SIZE	(72-40)
#define	ISO_VD_VOL_SPACE_SIZE	(80+ENDIAN_OFF_LONG)
#define	ISO_VD_ESCAPE_SEC	88
#define	ISO_VD_ESCAPE_SEC_SIZE	(120-88)
#define	ISO_VD_VOL_SET_SIZE	(120+ENDIAN_OFF_SHORT)
#define	ISO_VD_VOL_SEQUENCE_N	(124+ENDIAN_OFF_SHORT)
#define	ISO_VD_LB_SIZE		(128+ENDIAN_OFF_SHORT)
#define	ISO_VD_PT_SIZE		(132+ENDIAN_OFF_LONG)

/* 
 * ISO 9660 structures contan some data in both Big and Little endian order
 * Hopefully all mastering software implementations fill them all in 
 */

#ifdef BYTE_ORDER_BIG_ENDIAN
#define	ISO_VD_PT_OCCUR		148	/* u_long */
#define	ISO_VD_PT_OPT_OCCUR	152	/* u_long */

#else	/* BYTE_ORDER_LITTLE_ENDIAN */

#define	ISO_VD_PT_OCCUR		140	/* u_long */
#define	ISO_VD_PT_OPT_OCCUR	144	/* u_long */
#endif	/* BYTE_ORDER_BIG_ENDIAN */

#define	ISO_VD_ROOT_DIR_REC		156
#define	ISO_VD_ROOT_DIR_REC_SIZE	(190-156)

#define	ISO_VD_ID_STD_SIZE	128
#define	ISO_VD_VOL_SET_ID	190
#define	ISO_VD_PUBLISH_ID	318
#define	ISO_VD_DATA_PREP_ID	446
#define	ISO_VD_APPLIC_ID	574

#define	ISO_VD_F_ID_STD_SIZE	37
#define	ISO_VD_COPYR_F_ID	702
#define	ISO_VD_ABSTR_F_ID	739
#define	ISO_VD_BIBLIOGR_F_ID	776

#define	ISO_VD_VOL_DATE_TIME_STD_SIZE	17
#define	ISO_VD_VOL_CR_DATE_TIME		813
#define	ISO_VD_VOL_MODIF_DATE_TIME	830
#define	ISO_VD_VOL_EXPIR_DATE_TIME	847
#define	ISO_VD_VOL_EFFECT_DATE_TIME	864

#define	ISO_VD_FILE_STRUCT_VER		881

#define	CDROM_MAX_DIR_LEV	8	/* max dir nesting level */

/* Primary / Supplementary VD, for internal use, not defined by ISO 9660 */

typedef struct isoPrimarySuplementaryVolDescriptor
    {
    u_long	volSize;	/* logical sector per volume */
    u_long	PTSize;		/* bytes per Path Table */
    u_long      PTOccur;	/* Path table occurence */
    u_long	PTOptOccur;	/* optional occurence */
    u_long	rootDirSize;	/* bytes per Root Directory */
    u_long	rootDirStartLB;	/* where root dir starts */
    u_short	volSetSize;	/* number of phis devices in set */
    u_short	volSeqNum;	/* device number in set */
    u_short	LBSize;		/* pytes per Logical Block */
		
    u_char	type;		/* volume descriptor type, have to be one */
				/* of ISO_VD_PRIMARY or ISO_VD_SUPPLEM */

    u_char	fileStructVersion;	/* currently 1 only (ISO) */
    u_char	stdID[ ISO_STD_ID_SIZE + 1 ];	/* currently have to be */
    						/* ISO_STD_ID */
    u_char	systemId[ ISO_VD_SYSTEM_ID_SIZE +1 ];	/* a-u_chars */
    u_char	volumeId[ ISO_VD_VOLUME_ID_SIZE +1 ];	/* d-u_chars */
    u_char	volSetId[ ISO_VD_ID_STD_SIZE +1 ];	/* d-u_chars */
    u_char	publisherId[ ISO_VD_ID_STD_SIZE +1 ];	/* a-u_chars */
    u_char	preparerId[ ISO_VD_ID_STD_SIZE +1 ]; 	/* a-u_chars */
    u_char	applicatId[ ISO_VD_ID_STD_SIZE +1 ];	/* a-u_chars */
    u_char	cprightFId[ ISO_VD_F_ID_STD_SIZE +1 ]; 	/* file ID */
    u_char	abstractFId[ ISO_VD_F_ID_STD_SIZE +1 ];	/* file ID */
    u_char	bibliogrFId[ ISO_VD_F_ID_STD_SIZE +1 ];	/* file ID */
    T_ISO_VD_DATE_TIME	creationDate;
    T_ISO_VD_DATE_TIME	modificationDate;
    T_ISO_VD_DATE_TIME	expirationDate;
    T_ISO_VD_DATE_TIME  effectiveDate;
    } T_ISO_PVD_SVD;

typedef T_ISO_PVD_SVD *	T_ISO_PVD_SVD_ID;


/* structure to hold data while files are opened. */

typedef struct secBuf	/* buffer for reading sectors */
    {
    u_long startSecNum;
    u_long numSects;
    int	   magic;   /* SEC_BUF_MAG after sectData allocation */
    u_char *	sectData;
    } SEC_BUF;

typedef SEC_BUF *	SEC_BUF_ID;

/* 
 * CDROM_VOL_DESCR - 
 * Since in future it will cover, we'll hope, Rock Ridge extensions, a name
 * has no preffix 'ISO'.
 * Many fields, pertaining to T_ISO_PVD_SVD, are excluded from
 * CDROMFS_VOL_DESCR for memory saving and code simplicity,
 * since they aren't realy use for device control (such as publisherId ...).
 */

typedef struct cdromVolDescr
    {
    DEV_HDR    devHdr;		/* for adding to device table */
    int	       magic;		/* VD_SET_MAG */
    BLK_DEV *  pBlkDev;	        /* Ptr to Block Device structure. */
    SEM_ID     mDevSem;	        /* device mutual-exclusion semaphore */
    SEC_BUF    sectBuf;	        /* Sector reading buffer. must be inited */
				/* over cdromFsDevCreate() */
    u_int      sectSize;	/* device sector size, copy of  */
				/* bd_bytesPerBlk in BLK_DEV */
    LIST       VDList;		/* VD list header. All device VD (at least */
				/* one primary and each supplementary, if */
				/* any exist) are connected to linked list */
    LIST       FDList;		/* FD list header. All file descriptors are */
				/* connected to linked list for to arrive */
				/* each of them in case vol unmount */
    u_char     LSToPhSSizeMult; /* LSSize/PhSSize ( may be need in case */
    				/* any blkDevDrv understand cdrom LS size */
    				/* less then 2Kb ) */
    u_char     unmounted;	/* != 0 => VDList must be bult newrly */
    } CDROM_VOL_DESC;

typedef CDROM_VOL_DESC *	CDROM_VOL_DESC_ID;

/* device VDs-list element */

typedef struct cdromVDLst
    {
    NODE	list;
    int	magic;	   	          /* VD_LIST_MAG */
    CDROM_VOL_DESC_ID	pVolDesc; /* ptr to list header's CDROM_VOL_DESC, */
				  /* may be uzed as BLK_DEV ptr */
    u_long	VDInSector;	  /* number of sector, VD is layed in */
    u_long	VDOffInSect;	  /* VD start offset in the sector */
    u_long	volSize;	  /* LS per volume */
    u_long	PTSize;		  /* bytes per Path Table */
    u_long	PTStartLB;
    u_long	rootDirSize;	  /* bytes per Root Directory */
    u_long	rootDirStartLB;
    u_short	volSetSize;	  /* number of phis devices in set */
    u_short	volSeqNum;	  /* device number in set */
    u_short	LBSize;		  /* pytes per Logical Block */
		
    /* In accordance with ISO9660 all directory records in
     * PT are sorted by hierarchy levels and are numbered from 1
     * ( only root has number 1 and is always placed on level 1 ).
     * Number of levels is restricted by 8.
     * dirLevBordersOff[ n ] contains offset of first PT record on level
     * (n+2) (root is excluded and array is encountered from 0) from PT start.
     * dirLevLastRecNum[ n ] contains number of last PT record on level
     * (n+2).
     */

    u_short	dirLevBordersOff[ CDROM_MAX_DIR_LEV ];	/* first PT-record */
							/* offset in each */
							/* level */
    u_short	dirLevLastRecNum[ CDROM_MAX_DIR_LEV ];	/* last PT-record */
							/* number in each */
							/* level */
    u_short	numDirLevs;	/* number of PT dir hierarchy levels, */
				/* exclude root */
    u_short	numPTRecs;	/* last PT record number (number of records) */
    u_char	LBToLSShift;	/* to get LS number, containes given LS */
				/* number of last may be just shifted */
    u_short	type;		/* volume descriptor type, have to be one */
				/*  of ISO_VD_PRIMARY or ISO_VD_SUPPLEM */

    /* currently 1 only (ISO) */
    u_short	fileStructVersion;
    SEC_BUF	PTBuf;		/* buffer for permanent storage PT during */
    				/* volume is mounted */
    } T_CDROMFS_VD_LST;				
typedef T_CDROMFS_VD_LST *	T_CDROMFS_VD_LST_ID;

/* ISO Path Table record fields offsets and sizes */

#define ISO_PT_REC_LEN_DI		0	
#define ISO_PT_REC_EAR_LEN		1
#define ISO_PT_REC_EXTENT_LOCATION	2
#define ISO_PT_REC_PARENT_DIR_N		6 /* short in format due to PT type */
#define ISO_PT_REC_DI			8

/* ISO Directory Record fields offsets and sizes */

#define ISO_DIR_REC_REC_LEN		0
#define ISO_DIR_REC_EAR_LEN		1
#define ISO_DIR_REC_EXTENT_LOCATION	(2+ENDIAN_OFF_LONG)
#define ISO_DIR_REC_DATA_LEN		(10+ENDIAN_OFF_LONG)
#define ISO_DIR_REC_DATA_TIME		18
#define ISO_DIR_REC_FLAGS		25
#define ISO_DIR_REC_FU_SIZE		26
#define ISO_DIR_REC_IGAP_SIZE		27
#define ISO_DIR_REC_VOL_SEQU_N		(28+ENDIAN_OFF_SHORT)
#define ISO_DIR_REC_LEN_FI		32	
#define ISO_DIR_REC_FI			33

/* flags masks */

#define DRF_DIRECTORY		0x02
#define DRF_IS_REC_DESCRIPT	0x04	/* file records are described in EAR */
#define DRF_PROTECT             0x10    /* location of user's permissions */
#define DRF_LAST_REC		0x80	/* last file-record */


/* ISO Extended Attribute Record fields offsets and sizes */

#define ISO_EAR_ONER		(0+ENDIAN_OFF_SHORT)
#define ISO_EAR_GROUPE		(4+ENDIAN_OFF_SHORT)
#define ISO_EAR_PERMIT		8

#define ISO_EAR_F_CR_DATE_TIME_SIZE	ISO_VD_VOL_DATE_TIME_STD_SIZE
#define ISO_EAR_F_CR_DATE_TIME		10
#define ISO_EAR_F_MODIF_DATE_TIME	27
#define ISO_EAR_F_EXPIR_DATE_TIME	44
#define ISO_EAR_F_EFFECT_DATE_TIME	81

#define ISO_EAR_REC_FORMAT	78
#define ISO_EAR_REC_ATTR	79
#define ISO_EAR_REC_LEN		(80+ENDIAN_OFF_SHORT)

#define ISO_EAR_SYS_ID		84
#define ISO_EAR_SYS_ID_SIZE	(116-84)
#define ISO_EAR_SYS_USE		116
#define ISO_EAR_SYS_USE_SIZE	(180-116)
#define ISO_EAR_VERSION		180	  /* currently have to be 1 (ISO) */

/* EAR-Record Format values */

#define EAR_REC_FORM_IGNORE	((u_char)0)
#define EAR_REC_FORM_FIX_LEN	CDROM_MDU_TYPE_FIX
#define EAR_REC_FORM_VAR_LEN_LE	CDROM_MDU_TYPE_VAR_LE	/* RCW in 7.2.1 */
#define EAR_REC_FORM_FIX_LEN_BE	CDROM_MDU_TYPE_VAR_BE	/* RCW in 7.2.2 */

/* EAR-Record Attribute values */

#define EAR_REC_ATTR_LF_CR	((u_char)0)	/* cr-record-lf */
#define EAR_REC_ATTR_VERT_SP	((u_char)1)
#define EAR_REC_ATTR_CONTROL	((u_char)3)

typedef struct cdromFileDateTime	/* directory record date time may be */
    {					/* copied directly to this structure */
    u_char	year;	/* from 1900 */
    u_char	month;
    u_char	day;
    u_char	hour;
    u_char	minuts;
    u_char	seconds;
    u_char	fromGreenwOf;
    } T_FILE_DATE_TIME;

typedef T_FILE_DATE_TIME *	T_FILE_DATE_TIME_ID;

#define CDROM_MAX_PATH_LEN	1024
#define CDROM_MAX_FILE_NAME_LEN	64

#define CDROM_MDU_TYPE_FIX	((u_char)1)
#define CDROM_MDU_TYPE_VAR_LE	((u_char)2)	/* RCW in 7.2.1 */
#define CDROM_MDU_TYPE_VAR_BE	((u_char)3)	/* RCW in 7.2.2 */

#define CDROM_NOT_INTERLEAVED	0
#define CDROM_INTERLEAVED	1

typedef struct cdromFile
    {
    NODE	list;
    int	magic;			  /* FD_MAG */
    u_char	inList;		  /* != 0 <=> FD added to volume FD list */
    T_CDROMFS_VD_LST_ID	pVDList;  /* not need to comment */
    u_char	volUnmount;	  /* (!= 0) => file unaccessable */

    /* File (F) static data */

    u_char	name[ CDROM_MAX_FILE_NAME_LEN + 1 ];
    u_short	parentDirNum;		/* parent dir PT number */
    u_int	parentDirPTOffset;	/* parent dir record offset in PT */
    u_long	parentDirStartLB;       /* parent DIR start LB     */
    u_long	FFirstRecLBNum;		/* LB in which file's dir records */
    					/* start */
    u_int	FFirstDirRecOffInDir;	/* offset of first FDR in dir */   
    u_int    	FFirstDirRecOffInLB;	/* offset of first FDR in  curr LB */
    u_int	FNumRecs;		/* number of File's Dir Records */
    u_char *	FRecords;		/* File's Directory Records buffer */

    u_long	FStartLB;          /* file first LB ( include EAR ) */
    u_long	FDataStartOffInLB; /* file data start LB ( exclude EAR ) */
    u_long	FSize;		   /* total file data size (netto)	*/
    u_short	FType;		   /*  Proper constants are contained in */
    				   /*  ioLib.h                           */
    u_char	FMDUType;	   /* EAR_REC_FORM_IGNORE/		*/
				   /* CDROM_MDU_TYPE_FIX/		*/
				   /* CDROM_MDU_TYPE_VAR_LE/	*/
				   /* CDROM_MDU_TYPE_VAR_BE	*/
    u_short	FMDUSize;	   /* byte per MDU (=0, in case MDU varLen) */

    /* time-date */
    T_FILE_DATE_TIME	FDateTime;

    time_t	FDateTimeLong;	/* 32-bit seconds */

    SEC_BUF	sectBuf;	/* Sector reading buffer. must be inited */
				/* over openDir() (and may be, over open */
				/* too )                                 */

    /* 
     * File Current (FC) Dir Record and File Current Section descriptions 
     * this fields are together used in case directory was opened (not file) 
     */

    u_char *	FCDirRecPtr;	/* ptr on current dir rec in dir rec buffer */
    u_int	FCDirRecNum;	/* current dir rec sequence number */
    				/* ( counts from 0 )         */
    u_int	FCDirRecAbsOff;	/* current dir rec offset in directory      */
    u_int	FCDirRecOffInLB; /* current dir rec offset within LB        */
    u_long	FCDirRecLB;	/* number of LB, containing directory record */

    u_long	FCSStartLB;	/* File Section first LB ( after EAR ) */

    u_long	FCSSize;	/* FS bytes (exclude EAR) */
    u_long	FCSSizeLB;	/* LB per FS (exclude EAR) */
    u_long	FCSAbsOff;	/* total data size in all previous sections */
    u_short	FDType;		/*  Proper constants are contained in */
    				/*  ioLib.h        (use for readdir)  */

    u_char	FCSSequenceSetNum;	/* current section Volume */
					/* Set Setsequence number */

    u_char	FCSInterleaved;	/* CDROM_INTERLEAVED/CDROM_NOT_INTERLEAVED */
				/* just may be checked as bolean */
    u_char	FCSFUSizeLB;	/* LB per FileUnit */
    u_char	FCSGapSizeLB;	/* LB per Gap */
    u_char	FCSFlags;	/* current file section flags */

    /* File Current MDU  (FCMDU)description */

    u_long	FCMDUStartLB;	/* current MDU start LB number */
    u_short	FCMDUSize;	/* byte per current MDU */

    /* File Current Data (FCD) positions description */

    u_long	FCDAbsLB;	/* current absolute LB  number */
    u_long	FCDAbsPos;	/* current absolute position in file */
    u_int	FCDOffInLB;	/* offset within current LB */
    u_short	FCDOffInMDU;	/* offset in current MDU */
    int		FCEOF;		/* !=0 <=> EOF encounted */

    /* some fields for using this structure for directories */

    u_short	DNumInPT;	/* Directory record number in PT */
    u_int	DRecOffInPT;	/* not need to comment              */
    u_long	DRecLBPT;	/* absolute LB number, that contain  */
    				/* PT directory record                 */
    u_int	DRecOffLBPT;	/* PT Directory record offset within LB */

    } T_CDROM_FILE;

typedef T_CDROM_FILE *	T_CDROM_FILE_ID;	/* not to mix with ISO */
						/* file ID, that equevalents */
						/* to file name in fact */



/* ========= errno codes =========== */

#define S_cdromFsLib_ALREADY_INIT			(M_cdromFsLib | 1)
#define S_cdromFsLib_DEVICE_REMOVED			(M_cdromFsLib | 3)
#define S_cdromFsLib_SUCH_PATH_TABLE_SIZE_NOT_SUPPORTED (M_cdromFsLib | 4)
#define S_cdromFsLib_ONE_OF_VALUES_NOT_POWER_OF_2	(M_cdromFsLib | 5)
#define S_cdromFsLib_UNNOWN_FILE_SYSTEM			(M_cdromFsLib | 6)
#define S_cdromFsLib_INVAL_VOL_DESCR			(M_cdromFsLib | 7)
#define S_cdromFsLib_INVALID_PATH_STRING		(M_cdromFsLib | 8)
#define S_cdromFsLib_MAX_DIR_HIERARCHY_LEVEL_OVERFLOW	(M_cdromFsLib | 9)
#define S_cdromFsLib_NO_SUCH_FILE_OR_DIRECTORY		(M_cdromFsLib | 10)
#define S_cdromFsLib_INVALID_DIRECTORY_STRUCTURE	(M_cdromFsLib | 11)
#define S_cdromFsLib_INVALID_FILE_DESCRIPTOR		(M_cdromFsLib | 12)
#define S_cdromFsLib_READ_ONLY_DEVICE			(M_cdromFsLib | 13)
#define S_cdromFsLib_END_OF_FILE			(M_cdromFsLib | 14)
#define S_cdromFsLib_INV_ARG_VALUE			(M_cdromFsLib | 15)
#define S_cdromFsLib_SEMTAKE_ERROR			(M_cdromFsLib | 16)
#define S_cdromFsLib_SEMGIVE_ERROR			(M_cdromFsLib | 17)
#define S_cdromFsLib_VOL_UNMOUNTED			(M_cdromFsLib | 18)
#define S_cdromFsLib_INVAL_DIR_OPER			(M_cdromFsLib | 19)
#define S_cdromFsLib_READING_FAILURE			(M_cdromFsLib | 20)
#define S_cdromFsLib_INVALID_DIR_REC_STRUCT             (M_cdromFsLib | 21)

/* user-callable functions prototypes */

#if defined(__STDC__) || defined(__cplusplus)
extern CDROM_VOL_DESC_ID cdromFsDevReset( void * pVolDesc, STATUS retStat );
extern CDROM_VOL_DESC_ID cdromFsDevCreate( char * devName, BLK_DEV * pBlkDev );
#else	/* __STDC__ */
extern CDROM_VOL_DESC_ID cdromFsDevReset ();
extern CDROM_VOL_DESC_ID cdromFsDevCreate ();
#endif	/* __STDC__ */

#ifdef __cplusplus
}
#endif

#endif /* cdromFsLib_h */
