/*******************************************************************************
* File Name    : ax88796End.c
*
* Description  : ax88796 END  network interface driver 
*
* Revision History :
*
* Date        Modification                                               Name  
* ----        ------------                                             --------
*                                       
*
*******************************************************************************/



#include "copyright_wrs.h"

#include "vxWorks.h"
#include "stdlib.h"
#include "cacheLib.h"
#include "intLib.h"
#include "end.h"			/* Common END structures. */
#include "endLib.h"
#include "lstLib.h"			/* Needed to maintain protocol list. */
#include "wdLib.h"
#include "iv.h"
#include "semLib.h"
#include "etherLib.h"
#include "logLib.h"
#include "netLib.h"
#include "netBufLib.h"
#include "stdio.h"
#include "sysLib.h"
#include "errno.h"
#include "errnoLib.h"
#include "memLib.h"
#include "iosLib.h"
#undef	ETHER_MAP_IP_MULTICAST
#include "etherMultiLib.h"		/* multicast stuff. */
#include "muxLib.h"
#include "net/mbuf.h"
#include "net/unixLib.h"
#include "net/protosw.h"
#include "net/systm.h"
#include "net/if_subr.h"
#include "net/route.h"
#include "netinet/in.h"
#include "sys/socket.h"
#include "sys/ioctl.h"
#include "sys/times.h"
#include "arpLib.h"
#include "hostLib.h"
#include "netShow.h"
#include "usrconfig.h"
#include "pingLib.h"
#include "bootpLib.h"

#include "TLPrivate.h"
#include "EagleCommon.h"
#include "sen_bsp.h"			// sen_bsp.h for 5.5
#include "ax88796end.h"
#include "osa.h"
#include "cm_mux.h"
#include "TLgpio.h"
#include "mainflash.h"


#define ETH_DEBUG_ENABLE
#ifdef	ETH_DEBUG_ENABLE
#define ETH_PRT(x, y)	if (x <= show_eth) printf("_ETH_\t"), printf y
#define EMPTY_PRT(x, y)	if (x <= show_eth) printf("\t"), printf y
#else
#define ETH_PRT(x, y)
#define EMPTY_PRT(x, y)
#endif

/* defines */

#define AX88796_PHY          0x40    /* PHY address 0x01 */
#define AX88796_PKT_MAX      1522    /* Received packet max size */
#define AX88796_PKT_MIN      64   	/* Received packet minsize */
#define AX88796_PKT_RDY      0x01    /* Packet ready to receive */
#define AX88796_MIN_FBUF     AX88796_PKT_MAX  /* min first buffer size */

#define AX88796_10MHD        0
#define AX88796_100MHD       1
#define AX88796_10MFD        4
#define AX88796_100MFD       5
#define AX88796_AUTO         8
#define AX88796_1M_HPNA      0x10

#define AX88796_MEDIA_MODE   AX88796_AUTO

#define AX88796_DEV_NAME     "ax"
#define AX88796_DEV_NAME_LEN 3

#define AX88796_IOADDR       LB_CS2_BASE
#define AX88796_IODATA       (LB_CS2_BASE+0x10)

/* Configuration items */

#define END_BUFSIZ      	(ETHERMTU + ENET_HDR_REAL_SIZ + 6) /* 1500 + 14 + 6 */
#define EH_SIZE         	(14)
#define END_SPEED_10M   	10000000    /* 10Mbs */
#define END_SPEED_100M 	 	100000000   /* 100Mbs */
#define END_SPEED       	END_SPEED_100M

/* Definitions for the flags field */
#define AX88796_POLLING      0x2

/* Status register bits, returned by ax88796StatusRead() */
#define AX88796_RINT         0x1     /* Rx interrupt pending */
#define AX88796_TINT         0x2     /* Tx interrupt pending */
#define AX88796_VALID_INT    0x3     /* Any valid interrupt pending */
#define AX88796_RXON         0x4     /* Rx on (enabled) */
#define AX88796_TFULL        0x8     /* tx full */
#define AX88796_RXRDY        0x10    /* data rdy */

#undef htons
#define htons(x)  ((((x)&0xff00)>>8)|((x)&0x00ff)<<8)

/* Cache macros */

#define END_CACHE_INVALIDATE(address, len) \
        CACHE_DRV_INVALIDATE (&dev->cacheFuncs, (address), (len))
#define END_CACHE_PHYS_TO_VIRT(address) \
        CACHE_DRV_PHYS_TO_VIRT (&dev->cacheFuncs, (address))
#define END_CACHE_VIRT_TO_PHYS(address) \
        CACHE_DRV_VIRT_TO_PHYS (&dev->cacheFuncs, (address))

 #ifndef SYS_INT_CONNECT
 #define SYS_INT_CONNECT(pDrvCtrl,rtn,arg,pResult) \
 { \
	 tl_IntGPIOConfig(GPIO_ETH_INT, TLGPIO_NEGATIVE_EDGE); \
	 tl_gpio_int_install((VOIDFUNCPTR)rtn, arg, GPIO_ETH_INT);\
	 *pResult = OK; \
 }
 #endif
 
 #ifndef SYS_INT_DISCONNECT
 #define SYS_INT_DISCONNECT(pDrvCtrl,rtn,arg,pResult) \
 { \
	 tl_gpio_int_uninstall(GPIO_ETH_INT); \
	 *pResult = OK;\
 }
 #endif

#ifndef SYS_INT_ENABLE
 #define SYS_INT_ENABLE(pDrvCtrl) \
{ \
	tl_gpio_int_enable();\
}
#endif

#ifndef SYS_INT_DISABLE
#define SYS_INT_DISABLE(pDrvCtrl) \
{ \
	tl_gpio_int_clear(GPIO_ETH_INT);\
}
#endif

/* Macro to get the ethernet address from the BSP */

/*
 * Macros to do a short (UINT16) access to the chip. Default
 * assumes a normal memory mapped device.
 */
#ifndef AX88796_IN_CHAR
#define AX88796_IN_CHAR(offset,data) \
 	data = sysInWord( (AX88796_IOADDR+(offset)));	
//	data = sysInByte( AX88796_IOADDR+offset*2);
#endif

#ifndef AX88796_OUT_CHAR
#define AX88796_OUT_CHAR(offset, value) \
 	sysOutWord( (AX88796_IOADDR+(offset)), (value));
//	sysOutByte( AX88796_IOADDR+offset*2, value);
 #endif

#ifndef AX88796_IN_ADDR
#define AX88796_IN_ADDR( addr ) \
    addr = sysInByte( (AX88796_IOADDR));
#endif

#ifndef AX88796_OUT_ADDR
#define AX88796_OUT_ADDR( addr ) \
    sysOutByte(AX88796_IOADDR, addr);
#endif

#ifndef AX88796_IN_WORD
#define AX88796_IN_WORD( offset, data ) \
 	data = sysInWord( (AX88796_IOADDR+(offset)));
#endif
    
#ifndef AX88796_OUT_WORD
#define AX88796_OUT_WORD( offset, data ) \
 	sysOutWord( (AX88796_IOADDR+(offset)), (data));
#endif


#ifndef AX88796_IN_LONG
#define AX88796_IN_LONG( offset, data ) \
 	data = sysInLong( (AX88796_IOADDR+(offset)));
#endif
    
#ifndef AX88796_OUT_LONG
#define AX88796_OUT_LONG( offset, data ) \
 	sysOutLong( (AX88796_IOADDR+(offset)), (data));
#endif

/* A shortcut for getting the hardware address from the MIB II stuff. */

#define END_HADDR(pEnd) \
        ((pEnd)->mib2Tbl.ifPhysAddress.phyAddress)

#define END_HADDR_LEN(pEnd) \
        ((pEnd)->mib2Tbl.ifPhysAddress.addrLength)

#define AX88796_PKT_LEN_GET(pPkt) (((PKT *)pPkt)->len)
#define AX88796_PKT_DATA_GET(pPkt) (((PKT *)pPkt)->pData)

#define INT_LOCK()
#define INT_UNLOCK(x)

#define AX_LOAD_FUNC		sysEtherEndLoad
#define AX_BUFF_LOAD		1
#define AX_LOAD_STRING		"0xBE000000:0x25:0x5"
#define INCLUDE_DHCPC

/* The definition of the driver control structure */
typedef struct end_device
{
    END_OBJ             end;            /* The class we inherit from. */
    END_ERR             err;
	unsigned int		IOBase;
    int                 unit;           /* unit number */
    int                 ivec;           /* interrupt vector */
    int                 ilevel;         /* interrupt level */
    long                flags;          /* Our local flags. */
    unsigned char       enetAddr[6];    /* ethernet address */

    CACHE_FUNCS         cacheFuncs;     /* cache function pointers */
    CL_POOL_ID          pClPoolId;      /* cluster pool */
    BOOL                rxHandling;     /* rcv task is scheduled */

    unsigned char       io_mode;        /* 1:word, 0:byte */
    char                tx_pkt_cnt;
    char                device_wait_reset;
    unsigned short int  queue_pkt_len;
    unsigned char       reg0;           /* registers saved */
    unsigned char       nic_type;       /* NIC type */
    unsigned char       op_mode;        /* PHY operation mode */

    unsigned char       mcastFilter[8]; /* multicast filter */
    unsigned char       txBuf[ETHERMTU + EH_SIZE + 6 + 64 ];
} END_DEVICE;


#define ___AX88796_VAR___

int show_eth = 0, show_eth_pkt = 0;	

M_CL_CONFIG ax88796MclBlkConfig =    /* network mbuf configuration table */
{
    /*
    no. mBlks       no. clBlks  memArea     memSize
    -----------     ----------  -------     -------
    */
    256,          128,      NULL,       0
};

CL_DESC ax88796ClDescTbl [] =    /* network cluster pool configuration table */
{
    /*
    clusterSize         num memArea     memSize
    -----------         ----    -------     -------
    */
    {END_BUFSIZ,    0,  NULL,       0}
};
 
int ax88796ClDescTblNumEnt = (NELEMENTS(ax88796ClDescTbl));
NET_POOL ax88796CmpNetPool;

static dm_page_info pageInfo;

/* LOCALS */
//static unsigned char  nfloor = 0;
unsigned long CrcTable[256] = {
   0x00000000L, 0x77073096L, 0xEE0E612CL, 0x990951BAL,
   0x076DC419L, 0x706AF48FL, 0xE963A535L, 0x9E6495A3L,
   0x0EDB8832L, 0x79DCB8A4L, 0xE0D5E91EL, 0x97D2D988L,
   0x09B64C2BL, 0x7EB17CBDL, 0xE7B82D07L, 0x90BF1D91L,
   0x1DB71064L, 0x6AB020F2L, 0xF3B97148L, 0x84BE41DEL,
   0x1ADAD47DL, 0x6DDDE4EBL, 0xF4D4B551L, 0x83D385C7L,
   0x136C9856L, 0x646BA8C0L, 0xFD62F97AL, 0x8A65C9ECL,
   0x14015C4FL, 0x63066CD9L, 0xFA0F3D63L, 0x8D080DF5L,
   0x3B6E20C8L, 0x4C69105EL, 0xD56041E4L, 0xA2677172L,
   0x3C03E4D1L, 0x4B04D447L, 0xD20D85FDL, 0xA50AB56BL,
   0x35B5A8FAL, 0x42B2986CL, 0xDBBBC9D6L, 0xACBCF940L,
   0x32D86CE3L, 0x45DF5C75L, 0xDCD60DCFL, 0xABD13D59L,
   0x26D930ACL, 0x51DE003AL, 0xC8D75180L, 0xBFD06116L,
   0x21B4F4B5L, 0x56B3C423L, 0xCFBA9599L, 0xB8BDA50FL,
   0x2802B89EL, 0x5F058808L, 0xC60CD9B2L, 0xB10BE924L,
   0x2F6F7C87L, 0x58684C11L, 0xC1611DABL, 0xB6662D3DL,
   0x76DC4190L, 0x01DB7106L, 0x98D220BCL, 0xEFD5102AL,
   0x71B18589L, 0x06B6B51FL, 0x9FBFE4A5L, 0xE8B8D433L,
   0x7807C9A2L, 0x0F00F934L, 0x9609A88EL, 0xE10E9818L,
   0x7F6A0DBBL, 0x086D3D2DL, 0x91646C97L, 0xE6635C01L,
   0x6B6B51F4L, 0x1C6C6162L, 0x856530D8L, 0xF262004EL,
   0x6C0695EDL, 0x1B01A57BL, 0x8208F4C1L, 0xF50FC457L,
   0x65B0D9C6L, 0x12B7E950L, 0x8BBEB8EAL, 0xFCB9887CL,
   0x62DD1DDFL, 0x15DA2D49L, 0x8CD37CF3L, 0xFBD44C65L,
   0x4DB26158L, 0x3AB551CEL, 0xA3BC0074L, 0xD4BB30E2L,
   0x4ADFA541L, 0x3DD895D7L, 0xA4D1C46DL, 0xD3D6F4FBL,
   0x4369E96AL, 0x346ED9FCL, 0xAD678846L, 0xDA60B8D0L,
   0x44042D73L, 0x33031DE5L, 0xAA0A4C5FL, 0xDD0D7CC9L,
   0x5005713CL, 0x270241AAL, 0xBE0B1010L, 0xC90C2086L,
   0x5768B525L, 0x206F85B3L, 0xB966D409L, 0xCE61E49FL,
   0x5EDEF90EL, 0x29D9C998L, 0xB0D09822L, 0xC7D7A8B4L,
   0x59B33D17L, 0x2EB40D81L, 0xB7BD5C3BL, 0xC0BA6CADL,
   0xEDB88320L, 0x9ABFB3B6L, 0x03B6E20CL, 0x74B1D29AL,
   0xEAD54739L, 0x9DD277AFL, 0x04DB2615L, 0x73DC1683L,
   0xE3630B12L, 0x94643B84L, 0x0D6D6A3EL, 0x7A6A5AA8L,
   0xE40ECF0BL, 0x9309FF9DL, 0x0A00AE27L, 0x7D079EB1L,
   0xF00F9344L, 0x8708A3D2L, 0x1E01F268L, 0x6906C2FEL,
   0xF762575DL, 0x806567CBL, 0x196C3671L, 0x6E6B06E7L,
   0xFED41B76L, 0x89D32BE0L, 0x10DA7A5AL, 0x67DD4ACCL,
   0xF9B9DF6FL, 0x8EBEEFF9L, 0x17B7BE43L, 0x60B08ED5L,
   0xD6D6A3E8L, 0xA1D1937EL, 0x38D8C2C4L, 0x4FDFF252L,
   0xD1BB67F1L, 0xA6BC5767L, 0x3FB506DDL, 0x48B2364BL,
   0xD80D2BDAL, 0xAF0A1B4CL, 0x36034AF6L, 0x41047A60L,
   0xDF60EFC3L, 0xA867DF55L, 0x316E8EEFL, 0x4669BE79L,
   0xCB61B38CL, 0xBC66831AL, 0x256FD2A0L, 0x5268E236L,
   0xCC0C7795L, 0xBB0B4703L, 0x220216B9L, 0x5505262FL,
   0xC5BA3BBEL, 0xB2BD0B28L, 0x2BB45A92L, 0x5CB36A04L,
   0xC2D7FFA7L, 0xB5D0CF31L, 0x2CD99E8BL, 0x5BDEAE1DL,
   0x9B64C2B0L, 0xEC63F226L, 0x756AA39CL, 0x026D930AL,
   0x9C0906A9L, 0xEB0E363FL, 0x72076785L, 0x05005713L,
   0x95BF4A82L, 0xE2B87A14L, 0x7BB12BAEL, 0x0CB61B38L,
   0x92D28E9BL, 0xE5D5BE0DL, 0x7CDCEFB7L, 0x0BDBDF21L,
   0x86D3D2D4L, 0xF1D4E242L, 0x68DDB3F8L, 0x1FDA836EL,
   0x81BE16CDL, 0xF6B9265BL, 0x6FB077E1L, 0x18B74777L,
   0x88085AE6L, 0xFF0F6A70L, 0x66063BCAL, 0x11010B5CL,
   0x8F659EFFL, 0xF862AE69L, 0x616BFFD3L, 0x166CCF45L,
   0xA00AE278L, 0xD70DD2EEL, 0x4E048354L, 0x3903B3C2L,
   0xA7672661L, 0xD06016F7L, 0x4969474DL, 0x3E6E77DBL,
   0xAED16A4AL, 0xD9D65ADCL, 0x40DF0B66L, 0x37D83BF0L,
   0xA9BCAE53L, 0xDEBB9EC5L, 0x47B2CF7FL, 0x30B5FFE9L,
   0xBDBDF21CL, 0xCABAC28AL, 0x53B39330L, 0x24B4A3A6L,
   0xBAD03605L, 0xCDD70693L, 0x54DE5729L, 0x23D967BFL,
   0xB3667A2EL, 0xC4614AB8L, 0x5D681B02L, 0x2A6F2B94L,
   0xB40BBE37L, 0xC30C8EA1L, 0x5A05DF1BL, 0x2D02EF8DL
};

END_DEVICE *ax_end;

static OS_SEMAPHORE_ID axSema4;
static int axTid;
static int axTaskInited = 0;
static int forceDelaxTask;

#define ___AX88796_STATIC_API___

static STATUS		ax88796Parse( END_DEVICE * dev, char * initString );
static STATUS		ax88796MemInit( END_DEVICE * dev );
static STATUS		ax88796Start( END_DEVICE * dev );
static void 		ax88796Int( END_DEVICE  *dev  );
static STATUS 	ax88796PacketGet( END_DEVICE  *dev  );
static STATUS 	ax88796Recv( END_DEVICE *dev, char* pData );
static void 		ax88796HandleRcvInt( END_DEVICE *dev );
static STATUS 	ax88796Send( END_DEVICE * dev,  M_BLK_ID     pMblk );
static int 			ax88796Ioctl( END_DEVICE * dev, int cmd, caddr_t data );
static void 		ax88796Config( END_DEVICE *dev );
static void 		ax88796AddrFilterSet( END_DEVICE *dev );
static STATUS 	ax88796PollRcv( END_DEVICE *dev,  M_BLK_ID   pMblk );
static STATUS 	ax88796PollSend( END_DEVICE *dev,  M_BLK_ID   pMblk );
static STATUS 	ax88796MCastAdd( END_DEVICE *dev, char* pAddress );
static STATUS 	ax88796MCastDel( END_DEVICE *dev, char *pAddress );
static STATUS 	ax88796MCastGet( END_DEVICE *dev,  MULTI_TABLE *pTable );
static STATUS 	ax88796Stop( END_DEVICE *dev );
static STATUS 	ax88796Unload( END_DEVICE *dev );
static STATUS 	ax88796PollStart( END_DEVICE *dev );
static STATUS 	ax88796PollStop( END_DEVICE *dev );
static STATUS 	ax88796PollStop( END_DEVICE *dev );
static void 		ax88796Reset( END_DEVICE *dev );
static UINT 		ax88796StatusRead( END_DEVICE *dev );
END_OBJ			*sysEtherEndLoad( char* initString, void *arg);
extern void	test_createEthernet(void);


static NET_FUNCS ax88796FuncTable =
{
    (FUNCPTR) ax88796Start,      /* Function to start the device. */
    (FUNCPTR) ax88796Stop,       /* Function to stop the device. */
    (FUNCPTR) ax88796Unload,     /* Unloading function for the driver. */
    (FUNCPTR) ax88796Ioctl,      /* Ioctl function for the driver. */
    (FUNCPTR) ax88796Send,       /* Send function for the driver. */
    (FUNCPTR) ax88796MCastAdd,   /* Multicast add function for the */
                                /* driver. */
    (FUNCPTR) ax88796MCastDel,   /* Multicast delete function for */
                                /* the driver. */
    (FUNCPTR) ax88796MCastGet,   /* Multicast retrieve function for */
                                /* the driver. */
    (FUNCPTR) ax88796PollSend,   /* Polling send function */
    (FUNCPTR) ax88796PollRcv,    /* Polling receive function */

    endEtherAddressForm,        /* put address info into a NET_BUFFER */
    endEtherPacketDataGet,      /* get pointer to data in NET_BUFFER */
    endEtherPacketAddrGet,       /* Get packet addresses. */
};


END_TBL_ENTRY axEndDevTbl [] =
{
	(END_TBL_ENTRY){ 0, AX_LOAD_FUNC, AX_LOAD_STRING, AX_BUFF_LOAD, NULL, FALSE},
	(END_TBL_ENTRY){ 0, END_TBL_END, NULL, 0, NULL, FALSE }
};


#define ___AX88796_EXTERN_API____

extern int		endMultiLstCnt( END_OBJ* pEnd );
extern STATUS	usrNetProtoInit (void); 
extern int 		ipAttach ();
extern void 	getDhcpInfo(void);
extern void 	_cmResolvLibInit(void);

#define ___AX88796_SUB_API___




void	axend_readreg8(EG_U8 offset, EG_U8 *value)
{
	EG_U16	reg=0;
	AX88796_IN_CHAR(offset, reg);
	*value = (reg & 0xFF);	
	
}

void	axend_writereg8(EG_U8 offset, EG_U8 value)
{
	EG_U16	reg=0;
	reg = (EG_U16)value;
	AX88796_OUT_CHAR(offset, reg);
}

void	axend_readreg16(EG_U8 offset, EG_U16 *value)
{
	AX88796_IN_WORD(offset, *value);
//	*value = ((*value & 0xFF) << 8) |((*value >> 8) & 0xFF);
}

void	axend_writereg16(EG_U8 offset, EG_U16 value)
{
	AX88796_OUT_WORD(offset, value);
}

void	axend_readreg32(EG_U8 offset, EG_U32 *value)
{
	AX88796_IN_LONG(offset, *value);
}

void	axend_writereg32(EG_U8 offset, EG_U32 value)
{
	AX88796_OUT_LONG(offset, value);
}

static void axend_msdelay(int msec)
{
	if( msec < (1000/OS_GetTicksPerSecond()) )
	{
		EMPTY_PRT(0, ("\n\n@@@@ ERROR!!! Task delay should be more than %d ms @@@@\n", 1000/OS_GetTicksPerSecond() ));
	}
	else
	{
		OS_Delay(msec*OS_GetTicksPerSecond()/1000);
	}

}

static void axend_mdio_sync(void)
{
    int bits;
	
    for (bits = 0; bits < 32; bits++) 
	{
		axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE1);
		axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE1 | MDIO_SHIFT_CLK);
    }
}

static void axend_mdio_clear(void)
{
    int bits;
	
    for (bits = 0; bits < 16; bits++) 
	{
		axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE0);
		axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE0 | MDIO_SHIFT_CLK);
    }
}

static int axend_mdio_read(int phy_id, int loc)
{
//    unsigned int cmd = (0xf6<<10)|(phy_id<<5)|loc;
    int i;
	unsigned char pre_data=0, value=0;
	
	axend_mdio_clear();
    axend_mdio_sync();


	// preamble
	for(i=0;i<4;i++)
	{
		axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE1);
		axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE1 | MDIO_SHIFT_CLK);
	}

	// st
	axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE0);
	axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE0 | MDIO_SHIFT_CLK); 
	axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE1);
	axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE1 | MDIO_SHIFT_CLK);

	// op
	axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE1);
	axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE1 | MDIO_SHIFT_CLK);	
	axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE0);
	axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE0 | MDIO_SHIFT_CLK); 

	// phyadd
	for(i=4;i>=0;i--)
	{
		if((phy_id >> i) & 1)
		{
			axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE1);
			axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE1 | MDIO_SHIFT_CLK);
		}
		else
		{
			axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE0);
			axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE0 | MDIO_SHIFT_CLK); 
		}
	}

	// regad
	for(i=4; i>=0; i--)
	{
		if ((loc>>i)&1)
		{
			axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE1);
			axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE1 | MDIO_SHIFT_CLK);
		}
		else
		{
			axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE0);
			axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE0 | MDIO_SHIFT_CLK); 
		}
	}

	// ta
	axend_writereg8( EN0_MII_EEPROM, MDIO_ENB_IN);
	axend_writereg8( EN0_MII_EEPROM, MDIO_ENB_IN | MDIO_SHIFT_CLK);	
	axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE0);
	axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE0 | MDIO_SHIFT_CLK); 

	// data
	for( i=0; i<16; i++)
	{
		axend_writereg8( EN0_MII_EEPROM, MDIO_ENB_IN);
		axend_readreg8(EN0_MII_EEPROM, &pre_data);
		
		axend_writereg8( EN0_MII_EEPROM, MDIO_ENB_IN | MDIO_SHIFT_CLK); 
		value = (value << 1) | ((pre_data & MDIO_DATA_READ) >> 2);
	}

	// idle
	axend_writereg8( EN0_MII_EEPROM, MDIO_ENB_IN);
	axend_writereg8( EN0_MII_EEPROM, MDIO_ENB_IN | MDIO_SHIFT_CLK);

	return value;

}

static void axend_mdio_write(int phy_id, int loc, int value)
{

	int i;
	
	axend_mdio_clear();
    	axend_mdio_sync();

	// preamble
	for(i=0;i<4;i++)
	{
		axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE1);
		axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE1 | MDIO_SHIFT_CLK);
	}

	// st
	axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE0);
	axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE0 | MDIO_SHIFT_CLK);	
	axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE1);
	axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE1 | MDIO_SHIFT_CLK);

	// op
	axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE0);
	axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE0 | MDIO_SHIFT_CLK);	
	axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE1);
	axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE1 | MDIO_SHIFT_CLK);

	// phyadd
	for(i=4;i>=0;i--)
	{
		if((phy_id >> i) & 1)
		{
			axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE1);
			axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE1 | MDIO_SHIFT_CLK);
		}
		else
		{
			axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE0);
			axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE0 | MDIO_SHIFT_CLK); 
		}
	}

	// regad
	for(i=4; i>=0; i--)
	{
		if ((loc>>i)&1)
		{
			axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE1);
			axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE1 | MDIO_SHIFT_CLK);
		}
		else
		{
			axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE0);
			axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE0 | MDIO_SHIFT_CLK); 
		}
	}

	// ta
	axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE1);
	axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE1 | MDIO_SHIFT_CLK);
	axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE0);
	axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE0 | MDIO_SHIFT_CLK);	

	// data
	for( i=15; i>=0; i--)
	{
		if ((value >> i) & 1)
		{
			axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE1);
			axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE1 | MDIO_SHIFT_CLK);
		}
		else
		{
			axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE0);
			axend_writereg8(EN0_MII_EEPROM, MDIO_DATA_WRITE0 | MDIO_SHIFT_CLK); 
		}
	}

	// idle
	axend_writereg8( EN0_MII_EEPROM, MDIO_ENB_IN);
	axend_writereg8( EN0_MII_EEPROM, MDIO_ENB_IN | MDIO_SHIFT_CLK);

}

/* Block input and output, similar to the Crynwr packet driver.  If you
   are porting to a new ethercard, look at the packet driver source for hints.
   The NEx000 doesn't share the on-board packet memory -- you have to put
   the packet out through the "remote DMA" dataport using writeb. */
static inline void axend_block_input(END_DEVICE *dev, int count, PKT *skb, int readpage)
{
	unsigned char 	*buf = skb->pData;
	unsigned int	i;
	unsigned short	temp;

	dev = dev;
	
	ETH_PRT(2, ("axend_block_input(), count = %d\n", count));

	axend_writereg8(EN0_ISR, ENISR_RX);

	// 10. 11.
	axend_writereg8(EN0_RSAR1, readpage);
	axend_writereg8(EN0_RSAR0, 0x04);

	// 12. 13.
	axend_writereg8(EN0_RBCR1, count >> 8);
	axend_writereg8(EN0_RBCR0, count & 0xff);	

	// 14.
	axend_writereg8(EN0_CMD, E8390_RREAD | E8390_START | E8390_PAGE0);

	// 15.

	for(i=0;i < (unsigned int)count;i+=2) 
	{
		axend_readreg16(EN0_DP, &temp);

		buf[i] = temp & 0xFF;
		buf[i+1] = (temp >> 8) & 0xFF;
	}


	if(show_eth_pkt == 1)
	{
		printf("\t###################################################################\n\tPacket Length=%d\n\t", skb->len );
		for(i=0;i<(unsigned int)count;i++)
		{
			if((i != 0) && ((i%16) == 0))
			{
				printf("\n\t");
			}
			
			printf("%02X  ", skb->pData[i]);
		}
		printf("\n\t###################################################################\n");
	}


	axend_writereg8(EN0_CMD, E8390_NODMA | E8390_START |E8390_PAGE0);
}

static void axend_block_output(END_DEVICE *dev, int count, unsigned char *buf, const int start_page)
{
	int	i;
	unsigned long temp;
	unsigned char rdata;
	
	ETH_PRT(1, ("axend_block_output(), count = %d, start paget = %d\n", count, start_page));

	dev = dev;


	/* Now the normal output. */
	// 7.
	axend_writereg8(EN0_RSAR0, 0x00);
	axend_writereg8(EN0_RSAR1, start_page);

	// 8. 9.
	axend_writereg8(EN0_RBCR0, count & 0xff);
	axend_writereg8(EN0_RBCR1, count >> 8);

	// 10.
	axend_writereg8(EN0_CMD, E8390_PAGE0 | E8390_RWRITE|E8390_START);

	// Burst Data Access Mode
//	addr = (void*)(AX88796_IOADDR + EN0_DATA_ADDR);
//	memcpy(addr, buf, count);//	memcpy(addr, buf, (count+count%8));
	// Single Data Access mode
	// 11.
	#if 1
		for(i=0;i<count;i+=4)
		{
			temp = (buf[i+1] << 24) | (buf[i] << 16) | (buf[i+3] << 8) | buf[i+2];
//			axend_writereg16(EN0_DP, temp);
			AX88796_OUT_LONG(EN0_DP, temp);
		}
	#endif

	// 12. write current page
	axend_writereg8(EN0_TPSR, start_page);  

	// 13. 14.
	axend_writereg8(EN0_TBCR0, count & 0xff);
	axend_writereg8(EN0_TBCR1, count >> 8);
	// 15.
	axend_writereg8(EN0_CMD, (E8390_PAGE0 | E8390_NODMA|E8390_TRANS|E8390_START));

	axend_readreg8(EN0_ISR, &rdata);
	EMPTY_PRT(1, ("## EN0_ISR = %x\n", rdata));

	axend_writereg8(EN0_ISR, ENISR_RDC);	/* Ack intr. */
 	return;
}

static void axend_reset( END_DEVICE *dev )
{
    char status = 1;
	int res;
	unsigned char bootdata[35];
	unsigned int bootdatalen;
	unsigned char data=0;
	unsigned char cnt = 0;
	int	readmii;

	ETH_PRT(2, ("axend_reset()\n"));

	/* 1. Read Reset register (offset 1Fh) to reset MAC and then wait 1.6ms for MAC reset completion */
	axend_readreg8(EN0_RESET, &data);
	while (1)
	{
		axend_msdelay(5);	/* 1.6ms */
		axend_readreg8(EN0_ISR, &data);		
		status = (data & ENISR_RESET);
		if(status != 0)
		{
			break;
		}
		if(cnt > 1)
		{
			EMPTY_PRT(0, ("axend_reset() timeout\n"));
			break;
		}
		cnt++;
	}


	/* 2. select register page 0. */
	axend_writereg8(EN0_CMD, E8390_NODMA|E8390_PAGE0|E8390_STOP);
	
	/* 3. disable AXX88796B interrupt */
	axend_writereg8(EN0_IMR, 0x00 );

	/* 4. enable word-wide DMA transfer mode */
	axend_writereg8(EN0_DCFG, 0x01);/* word-base */
	dev->io_mode = 0x01;

	/* 5. enter MAC loopback mode, then the driver can configure TX/RX relevant registers */
	axend_writereg8(EN0_TCR, ENTCR_LOOPBACK); /* 0x02 - Internal loopback mode */
	
	/* 6. Configure RX Start page by write a proper page number to PSTART register */	
	axend_writereg8(EN0_PSTART, NESM_RX_START_PG );

	/* 7. set RX stop page */
	axend_writereg8(EN0_PSTOP, NESM_STOP_PG );
	
	/* 8. set RX boundary page */
	axend_writereg8(EN0_BNRY, NESM_RX_START_PG );

	/* 9. Clear interrupt status register */	
	axend_writereg8(EN0_ISR, 0xFF );

	/* 10. Select register page 1 */
	axend_writereg8(EN0_CMD, E8390_NODMA|E8390_PAGE1|E8390_STOP );
	
	/* 11. set the MAC address */
	bootdatalen = 35;
	res = ReadMainFlash(bootdata, BRAND_DATA_OFFSET, bootdatalen);
	if (res != 0)
	{
		EMPTY_PRT(0, ("MAC address read failed.\n"));
	}

	dev->enetAddr[0] = bootdata[28];
	dev->enetAddr[1] = bootdata[29];
	dev->enetAddr[2] = bootdata[30];
	dev->enetAddr[3] = bootdata[31];
	dev->enetAddr[4] = bootdata[32];
	dev->enetAddr[5] = bootdata[33];

	axend_writereg8(EN1_PAR0, dev->enetAddr[0]);
	axend_writereg8(EN1_PAR1, dev->enetAddr[1]);
	axend_writereg8(EN1_PAR2, dev->enetAddr[2]);
	axend_writereg8(EN1_PAR3, dev->enetAddr[3]);
	axend_writereg8(EN1_PAR4, dev->enetAddr[4]);
	axend_writereg8(EN1_PAR5, dev->enetAddr[5]);

	/* 12. Set the multicast address */
	axend_writereg8(EN1_MAR0, 0xFF);
	axend_writereg8(EN1_MAR1, 0xFF);
	axend_writereg8(EN1_MAR2, 0xFF);
	axend_writereg8(EN1_MAR3, 0xFF);
	axend_writereg8(EN1_MAR4, 0xFF);
	axend_writereg8(EN1_MAR5, 0xFF);
	axend_writereg8(EN1_MAR6, 0xFF);
	axend_writereg8(EN1_MAR7, 0xFF);

	/* 13. set RX start page + 1 */
	axend_writereg8(EN1_CURPAG, NESM_RX_START_PG + 1);		

	/* 14. select register page 0 */
	axend_writereg8(EN0_CMD, E8390_NODMA|E8390_PAGE0|E8390_STOP );

	/* 15. enable flow control and back pressure bits */
	axend_writereg8(EN0_FLOW, ENFLOW_ENABLE | ENBP_ENABLE |7);  // 2007-05-08  large size ping 

	/* 16. enable the TX/RX Pause Frame capability of AX88796B internal PHY */
	readmii = axend_mdio_read(0x10, 0x04);
	axend_mdio_write(0x10, 0x04, (readmii |0x0400));

	/* 17. restart the auto-negoriation function */
	axend_mdio_write(0x10, 0x00, 0x1200);

	/* 18. enable Back-To-Back Transmission function*/
	axend_writereg8(EN0_MCR, ENMCR_BTBENABLE );

	/* 19. select register page3 */
	axend_writereg8(EN0_CMD, E8390_NODMA|E8390_PAGE3|E8390_STOP );

	/* 20. enable TX Buffer Ring function */
	axend_writereg8(EN3_MISC, ENTBR_ENABLE);

	/* 21. select register page 0 */
	axend_writereg8(EN0_CMD, E8390_NODMA|E8390_PAGE0|E8390_STOP );
	
	/* 22. set TX start page */	
	axend_writereg8(EN0_TPSR, NESM_START_PG );

	/* 23. set RX packets filter */
	axend_writereg8(EN0_RCR, E8390_RXCONFIG);
	axend_writereg8(EN0_TCR, ENTCR_LOOPBACK | E8390_FULLDUPLEX);

	/* 24. enable AX88796B interrupt */
	axend_writereg8(EN0_IMR, E8390_IMRALL);

	/* 25. start AX88796 TX/RX operation */
	axend_writereg8(EN0_CMD, E8390_NODMA|E8390_PAGE0|E8390_START);
	
//	axend_writereg8(EN0_BTCR, ENBTCR_IRQ_PUSHPULL);

	pageInfo.tx_prev_ctepr = 0;
	pageInfo.tx_curr_page = NESM_START_PG;
	pageInfo.tx_start_page = NESM_START_PG;
	pageInfo.tx_stop_page = NESM_START_PG + TX_PAGES + 1;
	pageInfo.rx_start_page = NESM_RX_START_PG;
	pageInfo.rx_curr_page = NESM_RX_START_PG + 1;	
	pageInfo.rx_stop_page = NESM_STOP_PG;	

}

void	axend_buffer_recovery(void)
{
	unsigned char cmdReg;
    int resend=0;
	
	ETH_PRT(2, ("axend_buffer_recovery()\n"));

	axend_readreg8(EN0_CMD, &cmdReg);
	axend_writereg8(EN0_CMD, E8390_NODMA |E8390_STOP);

	// clear remote byte count registers
	axend_writereg8(EN0_RBCR0, 0x00);
	axend_writereg8(EN0_RBCR1, 0x00);

	// if we were transmitting something
	if(cmdReg & E8390_TRANS)
	{		
		axend_readreg8(EN0_ISR, &cmdReg);	// check if the transmit completed
		if((cmdReg & ENISR_TX) || (cmdReg & ENISR_TX_ERR))
		{
			resend = 0;     // transmit completed
		}
		else
		{
			resend = 1;     // transmit was interrupted, must resend
		}
	}
	
	axend_writereg8(EN0_TCR, ENTCR_LOOPBACK);
	axend_writereg8(EN0_BNRY, NESM_RX_START_PG);

	axend_writereg8(EN0_CMD, E8390_PAGE1 | E8390_NODMA |E8390_STOP);	
	axend_writereg8(EN1_CURPAG, NESM_RX_START_PG);
	
	axend_writereg8(EN0_CMD, E8390_NODMA |E8390_STOP);	
	axend_readreg8(EN0_ISR, &cmdReg);
	axend_writereg8(EN0_ISR, cmdReg);	
	axend_writereg8(EN0_TCR, E8390_FULLDUPLEX);
                                                                                           
	// if previous transmit was interrupted, then resend                               
	if(resend)
	{
		axend_writereg8(EN0_CMD, E8390_NODMA |E8390_START | E8390_TRANS);
	}
}

/* Hardware start transmission.
   Send a packet to media from the upper layer.
*/
static int axend_start_transmit( PKT *skb, END_DEVICE *dev)
{
	unsigned char data=0;
	unsigned char	cur_page, free_pages=0, need_pages;
	
	ETH_PRT(2, ("axend_start_transmit()\n"));

    if( (skb->len == 0) || (skb->pData == NULL) )
    {
		EMPTY_PRT(0, ("axend_start_transmit() illegal parameter.\n"));
        return 2;
    }
	// 1.
	axend_writereg8(EN0_IMR, 0x00);

	// 2.
	need_pages = (skb->len + 255)/256;
	axend_readreg8(EN0_CTEPR, &data);

	// 3.
	if(data & ENCTEPRT_TXCQF)
	{
		EMPTY_PRT(0, ("tx queue is full. EN0_CTEPR=%02X\n", data));
		return 2;
	}

	// 4.
	cur_page = data & ENCTEPRT_CTEPR;

	EMPTY_PRT(1,("cur_page = %d\n", cur_page));

	if(cur_page == 0) 
	{
		if((pageInfo.tx_curr_page == pageInfo.tx_start_page) && (pageInfo.tx_prev_ctepr == 0))
		{
			free_pages = pageInfo.tx_stop_page - pageInfo.tx_start_page;// TX_PAGES;
		}
		else
		{
			free_pages = pageInfo.tx_stop_page - pageInfo.tx_curr_page;
		}
	}
	else if(cur_page < (pageInfo.tx_curr_page - 1))
	{
		free_pages = pageInfo.tx_stop_page - pageInfo.tx_curr_page + cur_page - pageInfo.tx_start_page + 1;
	}
	else if(cur_page > (pageInfo.tx_curr_page - 1))
	{
		free_pages = cur_page + 1 - pageInfo.tx_curr_page;
	}
	else if(cur_page == (pageInfo.tx_curr_page - 1))
	{
		if(pageInfo.tx_full)
		{
			free_pages = 0;
		}
		else
		{
			free_pages = pageInfo.tx_stop_page - pageInfo.tx_start_page;// TX_PAGES;
		}
	}  

	EMPTY_PRT(1, (" free_pages = %d, need_pages = %d\n", free_pages, need_pages));
	if(free_pages < (need_pages+1)) 
	{
		EMPTY_PRT(0, ("free_pages(%d) < need_pages(%d)\n", free_pages, need_pages));
		pageInfo.tx_full = 1;
		axend_writereg8(EN0_IMR, ENISR_ALL);
		return 1;
	}

	if(show_eth_pkt == 1)
	{
		int i;

		EMPTY_PRT(1, ("\t###################################################################\n\tPacket Length=%d\n\t", skb->len ));

		for(i=0;i<skb->len;i++)
		{
			if((i != 0) && ((i%16) == 0))
			{
				printf("\n\t");
			}
			
			printf("%02X  ", skb->pData[i]);
		}
		printf("\n\t###################################################################\n");
	}


	axend_block_output(dev, skb->len, skb->pData, pageInfo.tx_curr_page);

	if(free_pages == need_pages) 
	{
		pageInfo.tx_full = 1;
		EMPTY_PRT(1, (" free_pages == need_pages \n"));
	}
	
	pageInfo.tx_prev_ctepr = cur_page;
	pageInfo.tx_curr_page = (pageInfo.tx_curr_page + need_pages < pageInfo.tx_stop_page) ? 
							pageInfo.tx_curr_page + need_pages : 
							need_pages - (pageInfo.tx_stop_page - pageInfo.tx_curr_page) + pageInfo.tx_start_page;

	axend_writereg8(EN0_IMR, E8390_IMRALL);
							
    return 0;
}

/* Received a packet and pass to upper layer
 * return 0  good
 * return 1  bad data
 * return 2  need reset
 */
static int axend_packet_receive( PKT *skb, END_DEVICE *dev )
{
	unsigned char	next_frame;
	dm_pkt_hdr		rx_frame;
	unsigned int	pkt_len, pkt_stat;
	unsigned char	write_point, read_point;
	unsigned char	data;
	int				cnt;
	EG_U16		temp;

	ETH_PRT(2, ("axend_packet_receive()\n"));

	// 1. 2.
	axend_readreg8(EN0_CPR, &write_point);
	axend_readreg8(EN0_BNRY, &read_point);

	ETH_PRT(2, ("\naxend_packet_receive:"));
	EMPTY_PRT(2, (", write = 0x%X ", write_point));
	EMPTY_PRT(2, (", read = 0x%X \n", read_point));

	read_point ++;  
	
	if (read_point == NESM_STOP_PG) 
		read_point = NESM_RX_START_PG;

	if(read_point == write_point)
	   return 1;

	while(write_point != pageInfo.rx_curr_page)	
	{
		axend_readreg8(EN0_BNRY, &read_point);		
		read_point++;
		
		if (read_point == NESM_STOP_PG) 
			read_point = NESM_RX_START_PG;

		EMPTY_PRT(2, ("write_point = 0x%X \n", write_point));
		EMPTY_PRT(2, ("read_point = 0x%X \n", read_point));
		
		axend_writereg8(EN0_ISR, ENISR_RX);

		// 3. 4. 
		axend_writereg8(EN0_RSAR0, 0x00);
		axend_writereg8(EN0_RSAR1, read_point);		

		// 5. 6.
		axend_writereg8(EN0_RBCR0, 0x20);//sizeof(dm_pkt_hdr) );
		axend_writereg8(EN0_RBCR1, 0x00);

		// 7.
		axend_writereg8(EN0_CMD, E8390_RREAD | E8390_START | E8390_PAGE0 );

		axend_readreg16(EN0_DP, &temp);
		rx_frame.status = temp & 0xFF;
		rx_frame.next = (temp >> 8) & 0xFF;
		EMPTY_PRT(1, ("\tFirst value=0x%0X\n", temp));
		
		axend_readreg16(EN0_DP, &temp);
		rx_frame.count = temp;
		EMPTY_PRT(1, ("\tSecond value=0x%0X\n", temp));
		EMPTY_PRT(1, ("\tstatus=0x%X, next=0x%X, count=0x%X\n", rx_frame.status, rx_frame.next, rx_frame.count));
	

		if((rx_frame.status & ENRSR_CRCERR) ||(rx_frame.status & ENRSR_ALIGNERR))
		{
			// 2007-05-02 axend_buffer_recovery();
			ax88796Reset( dev );
			EMPTY_PRT(0, ("Into the recovery() \n"));
			return 1;
		}		

		pkt_len = rx_frame.count - sizeof(dm_pkt_hdr);	/* count - (header size) => packet length */
		pkt_stat = rx_frame.status;
		next_frame = rx_frame.next;		

		if ((pkt_len < 60) || (pkt_len > 1518))
		{
			EMPTY_PRT(0, ("Packet Length Error : %d -----------\n", pkt_len));
			next_frame = write_point;

			if (next_frame == NESM_RX_START_PG)	
			{
				axend_writereg8(EN0_BNRY, NESM_STOP_PG - 1);	

			}
			else
			{
				axend_writereg8(EN0_BNRY, next_frame - 1);

			}
			
			continue;
		}
		else if ((pkt_stat & 0x0F) == ENRSR_RXOK)
		{
			skb->len = pkt_len;
			axend_block_input(dev, pkt_len, skb, read_point);
		}
		
		next_frame = rx_frame.next;

		if (next_frame == NESM_RX_START_PG)	
		{
			axend_writereg8(EN0_BNRY, NESM_STOP_PG - 1);	
		
		}
		else
		{
			axend_writereg8(EN0_BNRY, next_frame - 1);
		
		}
		pageInfo.rx_curr_page = next_frame;		
		axend_readreg8(EN0_CPR, &write_point);
		
		break;
	}

	axend_readreg8(EN0_CPR, &write_point);
	axend_readreg8(EN0_BNRY, &read_point);
	ETH_PRT(2, ("\naxend_packet_receive:"));
	EMPTY_PRT(2, (", write = 0x%X ", write_point));
	EMPTY_PRT(2, (", read = 0x%X \n", read_point));

	axend_writereg8(EN0_ISR, ENISR_RDC); 	

	return 0; 
}

/* Stop the interface.
   The interface is stopped when it is brought.
*/
static int axend_stop_ax88796( END_DEVICE *dev)
{
	ETH_PRT(2, ("axend_stop_ax88796()\n"));

	dev = dev;	

    /* RESET device */
	axend_mdio_write( 0x10, 0x00, 0x8000);		/* PHY RESET */
	axend_writereg8(EN0_IMR,0x00 );				/* Disable all interrupt */
	axend_writereg8(EN0_RCR, 0x00 );				/* Disable RX */

    return 0;
}

/**
 * ei_rx_overrun - handle receiver overrun
 * @dev: network device which threw exception
 *
 * We have a receiver overrun: we have to kick the 8390 to get it started
 * again. Problem is that you have to kick it exactly as NS prescribes in
 * the updated datasheets, or "the NIC may act in an unpredictable manner."
 * This includes causing "the NIC to defer indefinitely when it is stopped
 * on a busy network."  Ugh.
 * Called with lock held. Don't call this with the interrupts off or your
 * computer will hate you - it takes 10ms or so. 
 */
static void	axend_rx_overrun(END_DEVICE *dev)
{
	unsigned char was_txing, must_resend = 0;
	unsigned char data=0;
	
	ETH_PRT(2, ("axend_rx_overrun()\n"));
    
	/*
	 * Record whether a Tx was in progress and then issue the
	 * stop command.
	 */	
	axend_readreg8(EN0_CMD, &data);
	was_txing = data & E8390_TRANS;
	axend_writereg8(EN0_CMD, E8390_NODMA+E8390_PAGE0+E8390_STOP );

	/* 
	 * Wait a full Tx time (1.2ms) + some guard time, NS says 1.6ms total.
	 * We wait at least 2ms.
	 */
	axend_msdelay(5);	/* 2ms */

	/*
	 * Reset RBCR[01] back to zero as per magic incantation.
	 */
	axend_writereg8(EN0_RBCR0,0x00 );
	axend_writereg8(EN0_RBCR1,0x00 );

	/*
	 * See if any Tx was interrupted or not. According to NS, this
	 * step is vital, and skipping it will cause no end of havoc.
	 */

	if (was_txing)
	{ 
		unsigned char tx_completed;

		axend_readreg8(EN0_ISR, &data);
		tx_completed = data & (ENISR_TX+ENISR_TX_ERR);
		if (!tx_completed)
		{
			must_resend = 1;
		}
	}

	/*
	 * Have to enter loopback mode and then restart the NIC before
	 * you are allowed to slurp packets up off the ring.
	 */
	axend_writereg8(EN0_TCR, ENTCR_LOOPBACK );
	axend_writereg8(EN0_CMD,E8390_NODMA + E8390_PAGE0 + E8390_START );

	/*
	 * Clear the Rx ring of all the debris, and ack the interrupt.
	 */
	ax88796HandleRcvInt(dev);
	
	/*
	 * Leave loopback mode, and resend any packet that got stopped.
	 */
	axend_writereg8(EN0_TCR,E8390_FULLDUPLEX ); 
	if (must_resend)
	{
		axend_writereg8(EN0_CMD,E8390_NODMA + E8390_PAGE0 + E8390_START + E8390_TRANS );
	}

	EMPTY_PRT(0, ("RX overrun\n"));
}

/**
 * ei_tx_err - handle transmitter error
 * @dev: network device which threw the exception
 *
 * A transmitter error has happened. Most likely excess collisions (which
 * is a fairly normal condition). If the error is one where the Tx will
 * have been aborted, we try and send another one right away, instead of
 * letting the failed packet sit and collect dust in the Tx buffer. This
 * is a much better solution as it avoids kernel based Tx timeouts, and
 * an unnecessary card reset.
 *
 * Called with lock held.
 */
static void axend_tx_err(END_DEVICE *dev)
{
	unsigned char txsr=0;
	
	ETH_PRT(2, ("axend_tx_err() 0x%X\n", txsr));
	
	dev = dev;
	
	axend_readreg8(EN0_TSR, &txsr);

	if (txsr & ENTSR_ABT)
		EMPTY_PRT(4, ("excess-collisions\n"));
	if (txsr & ENTSR_ND)
		EMPTY_PRT(4, ("non-deferral \n"));
	if (txsr & ENTSR_CRS)
		EMPTY_PRT(4, ("lost-carrier \n"));
	if (txsr & ENTSR_FU)
		EMPTY_PRT(4, ("FIFO-underrun \n"));
	if (txsr & ENTSR_CDH)
		EMPTY_PRT(4, ("lost-heartbeat \n"));
	pageInfo.tx_full = 0;
}

/* Calculate the CRC valude of the Rx packet
  flag = 1 : return the reverse CRC (for the received packet CRC)
         0 : return the normal CRC (for Hash Table index)
 */
static unsigned long axend_cal_CRC(unsigned char * Data, unsigned int Len, unsigned char flag)
{
    unsigned long Crc = 0xffffffff;

    while (Len--)
    {
        Crc = CrcTable[(Crc ^ *Data++) & 0xFF] ^ (Crc >> 8);
    }

    if( flag )
    {
        return ~Crc;
    }
    else
    {
        return Crc;
    }
}

static void	axend_printIsr(int status)
{
	if(status & ENISR_RX)
	{
		EMPTY_PRT(4, ("ENISR_RX\n"));
	}
	if(status & ENISR_TX)
	{
		EMPTY_PRT(4, ("ENISR_TX\n"));
	}	
	if(status & ENISR_RX_ERR)
	{
		EMPTY_PRT(4, ("ENISR_RX_ERR\n"));
	}	
	if(status & ENISR_TX_ERR)
	{
		EMPTY_PRT(4, ("ENISR_TX_ERR\n"));
	}	
	if(status & ENISR_OVER)
	{
		EMPTY_PRT(4, ("ENISR_OVER\n"));
	}	
	if(status & ENISR_COUNTERS)
	{
		EMPTY_PRT(4, ("ENISR_COUNTERS\n"));
	}	
	if(status & ENISR_RDC)
	{
		EMPTY_PRT(4, ("ENISR_RDC\n"));
	}
	if(status & ENISR_RESET)
	{
		EMPTY_PRT(4, ("ENISR_RESET\n"));
	}
}


#define ___AX88796_INT_API___

static void	axend_IsrAsync(int dev)
{
	END_DEVICE *pDev;
	pDev = (END_DEVICE *)dev;
	semGive(axSema4);
	tl_gpio_int_clear(GPIO_ETH_INT);
}

static void	axend_IsrTask(unsigned int dev)
{
	END_DEVICE *pDev;
	pDev = (END_DEVICE*)dev;

	axSema4 = semCCreate(OS_SEM_FIFO, 0);
	tl_IntGPIOConfig(GPIO_ETH_INT, TLGPIO_NEGATIVE_EDGE);

	tl_gpio_int_install(axend_IsrAsync, (int)pDev, GPIO_ETH_INT);
	axTaskInited = 1;
	forceDelaxTask = 0;
	tl_gpio_int_enable();

	while(1)
	{
		if(forceDelaxTask == 3)
		{
			axTaskInited = 0;
			if(axTid)
			{
				taskDelete(axTid);
			}
		}

		semTake(axSema4, WAIT_FOREVER);
		ax88796Int((END_DEVICE*)pDev);
	}	
}

static void	axend_InitIsr(unsigned int pDev)
{
	if(axTaskInited == 0)
	{
		axTid = taskSpawn("tAxISR", 20, 0, 512000, (FUNCPTR)axend_IsrTask, (unsigned int)pDev, 0,0,0,0,0,0,0,0,0);
		if(axTid == 0)
		{
			EMPTY_PRT(1, ("taskSpawn() tAxISR error\n"));
		}
	}
	else
	{
		EMPTY_PRT(1, ("axend_IsrTask already created.\n"));
	}
}

#define ___AX88796_MAIN_API___

/*******************************************************************************
*
* ax88796Parse - parse the init string
*
* Parse the input string.  Fill in values in the driver control structure.
*
* The muxLib.o module automatically prepends the unit number to the user's
* initialization string from the BSP (configNet.h).
*
* RETURNS: OK or ERROR for invalid arguments.
*/

static STATUS ax88796Parse( END_DEVICE * dev,  /* device pointer */
                           char * initString )     /* information string */
{
	char *	tok;
    char *	pHolder = NULL;
    long	address;
	
	ETH_PRT(2, ("ax88796Parse()\n"));
    /* Parse the initString */

    /* Unit number. */
    tok = strtok_r (initString, ":", &pHolder);
    if (tok == NULL) 
	{
		return ERROR;
    }
    dev->unit = atoi (tok);
   if ( dev->unit > 1 )
   {
      EMPTY_PRT(0, ("Invalid unit number (%d)\n", dev->unit ));
      return ERROR;
   }
   
    /* IO Base */
    tok = strtok_r (NULL, ":", &pHolder);
    if (tok == NULL) 
	{
		return ERROR;
    }
    address = strtoul (tok, NULL, 16);
    dev->IOBase = (UINT)address;

    /* Interrupt vector. */
    tok = strtok_r (NULL, ":", &pHolder);
    if (tok == NULL) return ERROR;
    dev->ivec = (UINT)strtoul(tok,NULL,16);

    /* Interrupt level. */
    tok = strtok_r (NULL, ":", &pHolder);
    if (tok == NULL)
	{
		return ERROR;
    }
    dev->ilevel = (UINT)strtoul (tok,NULL,16);

	EMPTY_PRT(4, ( "Unit %d, IOBase 0x%X, ivec %d, ilevel %d\n", 
					dev->unit, dev->IOBase, dev->ivec, dev->ilevel));
    return OK;

}

/*******************************************************************************
*
* ax88796MemInit - initialize memory for the chip
*
* This routine is highly specific to the device.
*
* RETURNS: OK or ERROR.
*/

static STATUS ax88796MemInit( END_DEVICE * dev )    /* device to be initialized */
{
     /* This is how we would set up and END netPool using netBufLib(1).
     * This code is pretty generic.
     */

	 ETH_PRT(2, ("ax88796MemInit()\n"));

    if ((dev->end.pNetPool = malloc (sizeof(NET_POOL))) == NULL)
    {
        return (ERROR);
    }

    ax88796MclBlkConfig.mBlkNum = 256;
    ax88796ClDescTbl[0].clNum = 128;
    ax88796MclBlkConfig.clBlkNum = ax88796ClDescTbl[0].clNum;

    /* Calculate the total memory for all the M-Blks and CL-Blks. */

    ax88796MclBlkConfig.memSize = (ax88796MclBlkConfig.mBlkNum *
                                 (M_BLK_SZ + sizeof (long))) +
                                 (ax88796MclBlkConfig.clBlkNum *
                                 (CL_BLK_SZ + sizeof(long)));

    if ((ax88796MclBlkConfig.memArea = (char *) memalign (sizeof(long),
                                 ax88796MclBlkConfig.memSize)) == NULL)
    {
		EMPTY_PRT(0, ("ax88796MclBlkConfig.memArea NULL.\n"));
        return (ERROR);
    }

    /* Calculate the memory size of all the clusters. */

    ax88796ClDescTbl[0].memSize = (ax88796ClDescTbl[0].clNum *
                                 (END_BUFSIZ + 8)) + sizeof(int);

    /* Allocate the memory for the clusters from cache safe memory. */

    if ((ax88796ClDescTbl[0].memArea = (char *) memalign ( sizeof(long),
                                  ax88796ClDescTbl[0].memSize)) == NULL)
    {
		EMPTY_PRT(0, ("ax88796ClDescTbl[0].memArea NULL.\n"));
        return (ERROR);
    }

    if (ax88796ClDescTbl[0].memArea == NULL)
    {
        EMPTY_PRT(0,  ("system memory unavailable\n"));
        return (ERROR);
    }

    /* Initialize the memory pool. */

    if (netPoolInit(dev->end.pNetPool, &ax88796MclBlkConfig,
                    &ax88796ClDescTbl[0], ax88796ClDescTblNumEnt,
            NULL) == ERROR)
    {
		EMPTY_PRT(0,  ("netPoolInit() error.\n"));
        return (ERROR);
    }

    /*
     * If you need clusters to store received packets into then get them
     * here ahead of time.
     */

    if ((dev->pClPoolId = netClPoolIdGet (dev->end.pNetPool,
        (END_BUFSIZ), FALSE))
        == NULL)
    {
		EMPTY_PRT(0,  ("netClPoolIdGet() error.\n"));
        return (ERROR);
    }
	
	EMPTY_PRT(0,  ("Memory setup complete\n"));

    return OK;
}

/*******************************************************************************
*
* ax88796Reset - reset device
*
* RETURNS: N/A.
*/

static void ax88796Reset( END_DEVICE *dev )
{
    /* TODO - reset the controller */
	ETH_PRT(2, ("ax88796Reset()\n"));

    axend_reset( dev );
}

/*******************************************************************************
*
* ax88796Start - start the device
*
* This function calls BSP functions to connect interrupts and start the
* device running in interrupt mode.
*
* RETURNS: OK or ERROR
*
*/

static STATUS ax88796Start( END_DEVICE * dev )  /* device ID */
{

	ETH_PRT(2, ("ax88796Start()\n"));

	if(axTaskInited == 0)
	{
		axend_InitIsr((unsigned int)dev);
	}

	END_FLAGS_SET(&dev->end, (IFF_UP | IFF_RUNNING));

	
	EMPTY_PRT(4, ("Interrupt connected.\n"));
	

	axend_writereg8(EN0_RCR, E8390_RXCONFIG );				/* RX enable */
	axend_writereg8(EN0_IMR, /*(E8390_PRXE | E8390_PTXE) */ E8390_IMRALL);	/* Enable TX/RX interrupt mask */


    return (OK);
}



/*******************************************************************************
*
* ax88796Stop - stop the device
*
* This function calls BSP functions to disconnect interrupts and stop
* the device from operating in interrupt mode.
*
* RETURNS: OK or ERROR.
*/

static STATUS ax88796Stop( END_DEVICE *dev )     /* device to be stopped */
{
    STATUS result = OK;
	
	ETH_PRT(2, ("ax88796Stop()\n"));

    /* TODO - stop/disable the device. */
    axend_stop_ax88796( dev );
	
	if(axSema4)
	{
		semDelete(axSema4);
		forceDelaxTask = 3;
		tl_GPIOConfig(GPIO_ETH_INT, TLGPIO_OUTPUT);
		tl_gpio_int_uninstall(GPIO_ETH_INT);
	}

	END_FLAGS_CLR (&dev->end, IFF_UP | IFF_RUNNING);

    return (result);
}


/*******************************************************************************
*
* ax88796Int - handle controller interrupt
*
* This routine is called at interrupt level in response to an interrupt from
* the controller.
*
* RETURNS: N/A.
*/

static void ax88796Int( END_DEVICE  *dev  )  /* interrupting device */
{
    unsigned char isr_status=0;
    unsigned char TX_comple_status=0;
	
	ETH_PRT(2, ("ax88796Int()\n"));

	if(dev == NULL)
	{
		EMPTY_PRT(0, ("ax88796Int() dev is NULL\n"));
		return;
	}


	axend_writereg8(EN0_IMR, 0x00);

	axend_readreg8(EN0_ISR, &isr_status);
	axend_writereg8(EN0_ISR, isr_status );	 /* Clear ISR status */

	axend_printIsr(isr_status);

    /* Have netTask handle any input packets */
    if(isr_status & (ENISR_RX|ENISR_RX_ERR))	/* Packet Received */
    {
        if ( dev->rxHandling != TRUE )
        {
			dev->rxHandling = TRUE;
            ax88796HandleRcvInt(dev);
//           netJobAdd ((FUNCPTR)ax88796HandleRcvInt, (int)dev,0,0,0,0);
        }
    }

    /* TODO - handle transmit interrupts */
    if( isr_status & ENISR_TX )				/* Packet Transmitted */
    {
		axend_readreg8(EN0_TSR, &TX_comple_status);
		EMPTY_PRT(1, ("TSR stauts 0x%02X\n", TX_comple_status));
		pageInfo.tx_full = 0;
    }

	if(isr_status & ENISR_OVER)
	{
		axend_rx_overrun(dev);
	}

	if(isr_status & ENISR_TX_ERR)
	{
		axend_tx_err(dev);
	}

	if(isr_status & ENISR_COUNTERS)
	{
		axend_writereg8(EN0_ISR,ENISR_COUNTERS ); /* Ack intr. */
	}

	if (isr_status & ENISR_RDC)
	{
		axend_writereg8(EN0_ISR, ENISR_RDC);
	}

//intRet:
    /* Re-enable interrupt mask */
	axend_writereg8(EN0_CMD, E8390_NODMA|E8390_PAGE0|E8390_START );
	axend_writereg8(EN0_IMR, E8390_IMRALL);

}


/*******************************************************************************
*
* ax88796PacketGet - get next received message
*
* Get next received message.  Returns NULL if none are
* ready.
*
* RETURNS: ptr to next packet, or NULL if none ready.
*/

STATUS ax88796PacketGet( END_DEVICE  *dev  )
{
	unsigned char rxing_page=0, this_frame;
	unsigned char data=0;
	
	ETH_PRT(2, ("ax88796PacketGet()\n"));

	dev = dev;

	axend_writereg8(EN0_CMD, E8390_PAGE0 |E8390_NODMA);  
	axend_readreg8(EN0_CPR, &rxing_page);
 
	axend_readreg8(EN0_BNRY, &data);
	this_frame = data + 1;	
// 2007-05-02
	if (this_frame == NESM_STOP_PG) 
		 this_frame = NESM_RX_START_PG;

	
	if (this_frame == rxing_page) /* Read all the frames? */
	{ 
		EMPTY_PRT(4, ("No packet is received.\n"));	
		return ERROR;				/* Done for now */
	}

	EMPTY_PRT(4, ("Some packets are received. CurPage=0x%02X, Boundary=0x%02X\n", rxing_page, (data+1)));
	return OK;
}


/*******************************************************************************
*
* ax88796Recv - process the next incoming packet
*
* Handle one incoming packet.  The packet is checked for errors.
*
* RETURNS: N/A.
*/

unsigned int num_pkt_bytes_to_show = 256; // set > 0 limits how many packet bytes are dumped
unsigned int show_dn_pkt = 0;
unsigned int show_up_pkt = 0;

static STATUS ax88796Recv( END_DEVICE *dev,   /* device structure */
                         char* pData )           /* packet to process */
{
	char*       pClusterData;
	M_BLK_ID    pMblk, pMBuff;
	char*       pNewCluster;
	CL_BLK_ID   pClBlk;
	PKT         skb;
	
	ETH_PRT(2, ("ax88796Recv()\n"));

	pData = pData;
	
  

	pNewCluster = netClusterGet (dev->end.pNetPool, dev->pClPoolId);
	if (pNewCluster == NULL)
	{
		EMPTY_PRT(0, ("netClusterGet() error.\n"));
	    END_ERR_ADD (&dev->end, MIB2_IN_ERRS, +1);
	    goto cleanRXD;
	}

	/* Grab a cluster block to marry to the cluster we received. */
	if ((pClBlk = netClBlkGet (dev->end.pNetPool, M_DONTWAIT)) == NULL)
	{
	    netClFree (dev->end.pNetPool, pNewCluster);
		EMPTY_PRT(0, ("netClBlkGet() error.\n"));
	    END_ERR_ADD (&dev->end, MIB2_IN_ERRS, +1);
	    goto cleanRXD;
	}

	/*
	 * OK we've got a spare, let's get an M_BLK_ID and marry it to the
	 * one in the ring.
	 */
	if ((pMblk = mBlkGet (dev->end.pNetPool, M_DONTWAIT, MT_DATA)) == NULL)
	{
	    netClBlkFree (dev->end.pNetPool, pClBlk);
	    netClFree (dev->end.pNetPool, pNewCluster);
		EMPTY_PRT(0, ("mBlkGet() error.\n"));
	    END_ERR_ADD (&dev->end, MIB2_IN_ERRS, +1);
	    goto cleanRXD;
	}

	END_ERR_ADD (&dev->end, MIB2_IN_UCAST, +1);

	/* align IP header at 32-bit boundary, this is reqired by VxWorks
	** TCP/IP stack
	*/
	pClusterData = (char*)((((unsigned long)pNewCluster + 1) & (~1)) | 2 );

	/* move packet from Ethernet on-board SRAM to the mbuf pre-allocated
	** in system memory
	*/
	skb.pData = pClusterData;
	skb.len = 0;
	if( axend_packet_receive( (PKT*)(&skb), dev ) == 1)
	{
	EMPTY_PRT(0, ("axend_packet_receive() no data.\n"));
	    mBlkFree (dev->end.pNetPool, pMblk);
	    netClBlkFree (dev->end.pNetPool, pClBlk);
	    netClFree (dev->end.pNetPool, pNewCluster);
	    END_ERR_ADD (&dev->end, MIB2_IN_ERRS, +1);
	    goto cleanRXD;
	}

	/* Join the cluster to the MBlock */
	netClBlkJoin (pClBlk, pNewCluster, skb.len, NULL, 0, 0, 0);
	netMblkClJoin (pMblk, pClBlk);


	pMblk->mBlkHdr.mData = pClusterData;


	if((skb.pData[12] == 0x08) && (skb.pData[13] == 0x06))
	{
		pMblk->mBlkHdr.mLen = 42;
		EMPTY_PRT(1, ("### ARP length 42 \n"));
	}
	else
	{
		pMblk->mBlkHdr.mLen = skb.len;
	}


	pMblk->mBlkHdr.mFlags |= M_PKTHDR;
	pMblk->mBlkPktHdr.len = skb.len;


	EMPTY_PRT(1, ("Start show packet\n"));
	
	 if( show_dn_pkt )
	  {
		 unsigned int i,j;
		 unsigned char *tmpPtr;

		 tmpPtr = (unsigned char *) pMblk->mBlkHdr.mData;

		 printf("PKT FRM Lan: len=%d\n",pMblk->mBlkHdr.mLen);
		 if( ntohs(*((unsigned short *)tmpPtr + 6)) == ETHERTYPE_IP )
		 {
			printf("IP ID = 0x%x",ntohs(*((unsigned short *)tmpPtr + 9)) ); 
			printf("%x",ntohs(*((unsigned short *)tmpPtr + 9)) ); 

			/* #define	IP_MF 0x2000	   more fragments flag */
			if ( ntohs(*((unsigned short *)tmpPtr + 10)) & IP_MF )
				printf(", FRAG(%d)\n",(ntohs(*((unsigned short *)tmpPtr + 10)) & IP_OFFMASK)<<3); 
			else
				printf("\n");
			printf( "PROTO=%d",*(tmpPtr + 23));



			switch( *(tmpPtr + 23) )
			{
				case 1: 
					printf(" (ICMP)\n");
					break;
				case 2: 
					printf(" (IGMP)\n");
					break;
				case 6:
					printf(" (TCP)\n" );
					break;
				case 17: 
					printf(" (UDP)\n" );
					printf( "src port=%d",ntohs(*((unsigned short *)tmpPtr + 17)) );
					if( ntohs(*((unsigned short *)tmpPtr + 17)) == _BOOTP_CPORT )
						printf( " (BOOTPCLIENT)\n" );
					else if( ntohs(*((unsigned short *)tmpPtr + 17)) == _BOOTP_SPORT )
						printf( " (BOOTPSERVER)\n" );
					else 
						printf( "\n" );
					printf( "dst port=%d",ntohs(*((unsigned short *)tmpPtr + 18)) );
					if( ntohs(*((unsigned short *)tmpPtr + 18)) == _BOOTP_SPORT )
						printf( " (BOOTPSERVER)\n" );
					else if( ntohs(*((unsigned short *)tmpPtr + 18)) == _BOOTP_CPORT )
						printf( " (BOOTPCLIENT)\n" );
					else 
						printf( "\n" );
					break;
				 default: printf("\n");
			   }

		 }
		 else if( ntohs(*((unsigned short *)tmpPtr + 6)) == ETHERTYPE_ARP )
		   printf("ARP %s\n",ntohs(*((unsigned short *)tmpPtr + 10)) == 1 ? "REQUEST" : "REPLY" );

		 if( num_pkt_bytes_to_show > 0 )
			 j = num_pkt_bytes_to_show;
		 else
			 j = (unsigned int)pMblk->mBlkHdr.mLen; 

		 if( num_pkt_bytes_to_show > (unsigned int)pMblk->mBlkHdr.mLen	)
			 j = (unsigned int)pMblk->mBlkHdr.mLen;

		 for(i=0; i<j; i++)
		 {
			if( (i != 0) && ((i%16) == 0) )
			{
				printf("\n");
				printf("%02X ",(unsigned char)*(tmpPtr + i) );
			}
			else						 
			{
				printf("%02X ",(unsigned char)*(tmpPtr + i) );
			}
		 }
		 printf("\n\n");

	  }
	
	

    /* make the packet data coherent */
	END_CACHE_INVALIDATE (pMblk->mBlkHdr.mData, skb.len);

	EMPTY_PRT(4, ("Calling upper layer!\n"));

	/* TODO - Done with processing, clean up and pass it up. */

	/* Call the upper layer's receive routine. */
	END_ERR_ADD (&dev->end, MIB2_IN_UCAST, +1);
	END_RCV_RTN_CALL(&dev->end, pMblk);

	return (OK);
cleanRXD:

    return (ERROR);
}

/*******************************************************************************
*
* ax88796HandleRcvInt - task level interrupt service for input packets
*
* This routine is called at task level indirectly by the interrupt
* service routine to do any message received processing.
*
* The double loop is to protect against a race condition where the interrupt
* code see rxHandling as TRUE, but it is then turned off by task code.
* This race is not fatal, but does cause occassional delays until a second
* packet is received and then triggers the netTask to call this routine again.
*
* RETURNS: N/A.
*/

static void ax88796HandleRcvInt( END_DEVICE *dev )   /* interrupting device */
{
	ETH_PRT(2, ("ax88796HandleRcvInt()\n"));
	
// 2007-05-02	axend_writereg8(EN0_IMR, 0x00);

	dev->rxHandling = TRUE;

	while (OK==ax88796PacketGet (dev))
	{
		if(ax88796Recv (dev, (char*)NULL ) == ERROR)
		{
			break;
		}
	}
 
    dev->rxHandling = FALSE;
// 2007-05-02	axend_writereg8(EN0_IMR, E8390_IMRALL);
}

/*******************************************************************************
*
* ax88796Send - the driver send routine
*
* This routine takes a M_BLK_ID sends off the data in the M_BLK_ID.
* The buffer must already have the addressing information properly installed
* in it.  This is done by a higher layer.  The last arguments are a free
* routine to be called when the device is done with the buffer and a pointer
* to the argument to pass to the free routine.
*
* RETURNS: OK, ERROR, or END_ERR_BLOCK.
*/

static STATUS ax88796Send( END_DEVICE * dev,  /* device ptr */
                         M_BLK_ID     pMblk )    /* data to send */
{
	int         oldLevel = 0;
	BOOL        freeNow = TRUE;
	PKT         skb;
	int         len;
	int  rtnval = 0;

	ETH_PRT(2, ("ax88796Send()\n"));

    /*
     * Obtain exclusive access to transmitter.  This is necessary because
     * we might have more than one stack transmitting at once.
     */
    if (!((dev->flags) & AX88796_POLLING))
    {
        END_TX_SEM_TAKE (&dev->end, WAIT_FOREVER);
    }
	
    /* This is the normal case where all the data is in one M_BLK_ID */

    /* Set pointers in local structures to point to data. */

    /*
     * If no buffers are available,
     * release the semaphore and return END_ERR_BLOCK.
     * Do not free packet
     */

    /* place a transmit request */
//    if (!((dev->flags) & AX88796_POLLING))
    {
        oldLevel = intLock ();  /* protect ax88796Int */
    }

	len = netMblkToBufCopy (pMblk, (char*)(dev->txBuf), NULL);
	len = max (len, ETHERSMALL);

	skb.len = len;
	skb.pData = (char*)dev->txBuf;


	
	  if( show_up_pkt )
	  {
		 unsigned int i,j;
		 unsigned char *tmpPtr;

		 printf("PKT TO LAN: Len = %d\n", pMblk->mBlkHdr.mLen ); 

		 // tmpPtr = (unsigned char *) ((pMblk->mBlkHdr.mData)+2);
		 tmpPtr = (unsigned char *) ((pMblk->mBlkHdr.mData));

		 if ((unsigned int)pMblk->pClBlk->clSize < (unsigned int)pMblk->mBlkHdr.mLen)
			printf("PKT TO Lan: CL LENGTH NOT BIG ENOUGH !!!\n");

		 if( ntohs(*((unsigned short *)tmpPtr + 6)) == ETHERTYPE_IP )
		 {
		   printf("IP ID = 0x%x",ntohs(*((unsigned short *)tmpPtr + 9)) ); 
		   /* #define	IP_MF 0x2000	   more fragments flag */
		   if ( ntohs(*((unsigned short *)tmpPtr + 10)) & IP_MF )
				printf(", FRAG(%d)\n",(ntohs(*((unsigned short *)tmpPtr + 10)) & IP_OFFMASK)<<3); 
		   else
				printf("\n");
		   
		   printf( "PROTO=%d",*(tmpPtr + 23));
		   
		   switch( *(tmpPtr + 23) )
		   {
			 case 1: printf(" (ICMP)\n");
					 break;
			 case 2: printf(" (IGMP)\n");
					 break;
			 case 6: printf(" (TCP)\n" );
					 break;
			case 17: printf(" (UDP)\n" );
					 printf( "src port=%d",ntohs(*((unsigned short *)tmpPtr + 17)) );
					 if( ntohs(*((unsigned short *)tmpPtr + 17)) == _BOOTP_SPORT )
					   printf( " (BOOTPSERVER)\n" );
					 else if( ntohs(*((unsigned short *)tmpPtr + 17)) == _BOOTP_CPORT )
					   printf( " (BOOTPCLIENT)\n" );
					 else 
					   printf( "\n" );
					 printf( "dst port=%d",ntohs(*((unsigned short *)tmpPtr + 18)) );
					 if( ntohs(*((unsigned short *)tmpPtr + 18)) == _BOOTP_CPORT )
					   printf( " (BOOTPCLIENT)\n" );
					 else if( ntohs(*((unsigned short *)tmpPtr + 18)) == _BOOTP_SPORT )
					   printf( " (BOOTPSERVER)\n" );
					 else 
					   printf( "\n" );
					 break;
			 default: printf("\n");
		   }
		 }
		 else if( ntohs(*((unsigned short *)tmpPtr + 6)) == ETHERTYPE_ARP )
		   printf("ARP %s\n",ntohs(*((unsigned short *)tmpPtr + 10)) == 1 ? "REQUEST" : "REPLY" );

		 if( num_pkt_bytes_to_show > 0 )
			 j = num_pkt_bytes_to_show;
		 else
			 j = (unsigned int)pMblk->mBlkHdr.mLen; 

		 if( num_pkt_bytes_to_show > (unsigned int)pMblk->mBlkHdr.mLen	)
			 j = (unsigned int)pMblk->mBlkHdr.mLen;

		 for(i=0; i<j; i++)
		 {
			if( (i != 0) && ((i%16) == 0) )
			{
				printf("\n");
				printf("%02X ",(unsigned char)*(tmpPtr + i) );
			}
			else
			{
				printf("%02X ",(unsigned char)*(tmpPtr + i) );
			}
		 }
		 printf("\n\n");
	  }


	rtnval = axend_start_transmit( &skb, dev );

    if(rtnval == 1)
    {
		EMPTY_PRT(0, ("axend_start_transmit() error.\n"));

        freeNow = FALSE;

        /* Advance our management index(es) */
//        if (!((dev->flags) & AX88796_POLLING))
        {
            intUnlock (oldLevel);
        }
		netMblkClChainFree (pMblk);
        if (!((dev->flags) & AX88796_POLLING))
        {
            END_TX_SEM_GIVE (&dev->end);
	     EMPTY_PRT(1, ("END_TX_SEM_GIVE \n"));
        }

        return(END_ERR_BLOCK);
    }
   else
   	{

		EMPTY_PRT(1, (" axend_start_transmit = %d\n", rtnval));
   	}

    /* Advance our management index(es) */
//    if (!((dev->flags) & AX88796_POLLING))
    {
        intUnlock (oldLevel);
    }

	if (!((dev->flags) & AX88796_POLLING))
   	{
        END_TX_SEM_GIVE (&dev->end);
	 EMPTY_PRT(1, ("	END_TX_SEM_GIVE \n"));
	}

    /* Bump the statistics counters. */
    END_ERR_ADD (&dev->end, MIB2_OUT_UCAST, +1);

    /*
     * Cleanup.  The driver must either free the packet now or
     * set up a structure so it can be freed later after a transmit
     * interrupt occurs.
     */
	if (freeNow)
	{
		netMblkClChainFree (pMblk);
		EMPTY_PRT(1, ("netMblkClChainFree \n"));
	}

    return (OK);
}

/*******************************************************************************
*
* ax88796Ioctl - the driver I/O control routine
*
* Process an ioctl request.
*
* RETURNS: A command specific response, usually OK or ERROR.
*/

static int ax88796Ioctl( END_DEVICE * dev,   /* device receiving command */
                       int cmd,                 /* ioctl command code */
                       caddr_t data )           /* command argument */
{
    int error = 0;
    long value;

	ETH_PRT(2, ("ax88796Ioctl()\n"));

    switch (cmd)
    {
        case EIOCSADDR:
			EMPTY_PRT(4, ("EIOICSADDR\n"));
            if (data == NULL)
            {
                return (EINVAL);
            }
            bcopy ((char *)data, (char *)END_HADDR(&dev->end), END_HADDR_LEN(&dev->end));
            bcopy ((char *)data, (char *)dev->enetAddr, 6);
			break;
			
        case EIOCGADDR:
			EMPTY_PRT(4, ("EIOCGADDR\n"));
            if (data == NULL)
            {
                return (EINVAL);
            }
            bcopy ((char *)END_HADDR(&dev->end), (char *)data, END_HADDR_LEN(&dev->end));
            break;

		case EIOCSFLAGS:
			EMPTY_PRT(4, ("EIOCSFLAGS\n"));
            value = (long)data;
            if (value < 0)
            {
                value = -(--value);
                END_FLAGS_CLR (&dev->end, value);
            }
            else
            {
                END_FLAGS_SET (&dev->end, value);
            }
//            ax88796Config (dev);
            break;

        case EIOCGFLAGS:
			EMPTY_PRT(4, ("EIOCGFLAGS\n"));
            *(int *)data = END_FLAGS_GET(&dev->end);
            break;
			
		case EIOCMULTIADD:
			EMPTY_PRT(4, ("EIOCMULTIADD\n"));
			error = ax88796MCastAdd(dev, (char *)data);
			break;
			
		case EIOCMULTIDEL:
			EMPTY_PRT(4, ("EIOCMULTIDEL\n"));
			error = ax88796MCastDel(dev, (char *)data);
			break;		  	

		case EIOCMULTIGET:
			EMPTY_PRT(4, ("EIOCMULTIGET\n"));
			error = ax88796MCastGet(dev, (MULTI_TABLE *)data);
			break;				

        case EIOCPOLLSTART: /* Begin polled operation */
			EMPTY_PRT(4, ("EIOCPOLLSTART\n"));
            ax88796PollStart (dev);
            break;

        case EIOCPOLLSTOP:  /* End polled operation */
			EMPTY_PRT(4, ("EIOCPOLLSTOP"));
            ax88796PollStop (dev);
            break;

        case EIOCGMIB2:     /* return MIB information */
			EMPTY_PRT(4, ("EIOCGMIB2\n"));
            if (data == NULL)
            {
                return (EINVAL);
            }
            bcopy((char *)&dev->end.mib2Tbl, (char *)data, sizeof(dev->end.mib2Tbl));
            break;

		case EIOCGFBUF:     /* return minimum First Buffer for chaining */
			EMPTY_PRT(4, ("EIOCGFBUF\n"));
            if (data == NULL)
            {
                return (EINVAL);
            }
            *(int *)data = AX88796_MIN_FBUF;
			return AX88796_MIN_FBUF;
            break;

		case EIOCGHDRLEN:
			EMPTY_PRT(4, ("EIOCGFBUF\n"));
            if (data == NULL)
            {
                return (EINVAL);
            }
			*(int*)data = 14;
			break;
			
        default:
			switch(cmd)
			{				
				case EIOCGNPT:
					EMPTY_PRT(4, ("EIOCGNPT\n"));
					break;
				
				case EIOCQUERY:
					EMPTY_PRT(4, ("EIOCQUERY\n"));
					break;
				
				case EIOCGMIB2233:
					EMPTY_PRT(4, ("EIOCGMIB2233\n"));
					break;
				default:
					EMPTY_PRT(0, ("Undefined command. 0x%X\n", cmd));
					break;
			}
            error = EINVAL;
			break;
    }
    return (error);
}

/******************************************************************************
*
* ax88796Config - reconfigure the interface under us.
*
* Reconfigure the interface setting promiscuous mode, and changing the
* multicast interface list.
*
* RETURNS: N/A.
*/

static void ax88796Config( END_DEVICE *dev ) /* device to be re-configured */
{
    /* Set promiscuous mode if it's asked for. */
	ETH_PRT(2, ("ax88796Config()\n"));

    if (END_FLAGS_GET(&dev->end) & IFF_PROMISC)
    {
		EMPTY_PRT(0, ("Setting promiscuous mode on!\n"));
    }
    else
    {
		EMPTY_PRT(0, ("Setting promiscuous mode off!\n"));
    }

    /* Set up address filter for multicasting. */
    if (END_MULTI_LST_CNT(&dev->end) > 0)
    {
        ax88796AddrFilterSet (dev);
//        ax88796_hash_table( dev );
    }

	axend_writereg8(EN0_ISR, 0xff);
	axend_writereg8(EN0_IMR, E8390_IMRALL);
	axend_writereg8(EN0_CMD, E8390_NODMA|E8390_PAGE0|E8390_START);
	axend_writereg8(EN0_TCR, E8390_FULLDUPLEX);
	axend_writereg8(EN0_RCR, E8390_RXCONFIG);	

	EMPTY_PRT(4, ("ax88796Config() done\n"));
    return;
}


/******************************************************************************
*
* ax88796AddrFilterSet - set the address filter for multicast addresses
*
* This routine goes through all of the multicast addresses on the list
* of addresses (added with the endAddrAdd() routine) and sets the
* device's filter correctly.
*
* RETURNS: N/A.
*/

void ax88796AddrFilterSet( END_DEVICE *dev )
{
    int i;
    unsigned long crc;
    unsigned long hash_table[4];
    ETHER_MULTI* pCurr;

	ETH_PRT(2, ("ax88796AddrFilterSet()\n"));

	axend_writereg8(EN0_CMD, E8390_NODMA | E8390_PAGE1/* | E8390_STOP*/ );
	axend_writereg8(EN1_CURPAG, NESM_START_PG + TX_PAGES + 1 );
	
	for(i=0;i<6;i++)
	{
		axend_writereg8(EN1_PHYS_SHIFT(i), dev->enetAddr[i] );
	}	

	EMPTY_PRT(4, ("Physical  %02X:%02X:%02X:%02X:%02X:%02X\n", 
			dev->enetAddr[0],
            dev->enetAddr[1],
            dev->enetAddr[2],
            dev->enetAddr[3],
            dev->enetAddr[4],
            dev->enetAddr[5] ));

	memset(dev->mcastFilter, 0, 8);

    for(i=0;i<4;i++)
	{
		hash_table[i]=0;
    }
    hash_table[3]=0x8000;

    /* broadcast address */
    /*dev->mcastFilter[7] = 0x80; */      /*mabe  [6] */

    /* the multicast address in Hash Table : 64 bits */

    pCurr = END_MULTI_LST_FIRST (&dev->end);

    while (pCurr != NULL)
    {
        /* TODO - set up the multicast list */
        crc = axend_cal_CRC(((unsigned char *)&pCurr->addr), 6, 0 )&0x3f;
        hash_table[crc/16] |= (unsigned short) 1 << (crc % 16);

    	pCurr = END_MULTI_LST_NEXT(pCurr);
    }

	dev->mcastFilter[0] |= hash_table[0]&0xff;
	dev->mcastFilter[1] |= (hash_table[0]>>8)&0xff;
	dev->mcastFilter[2] |= hash_table[1]&0xff;
	dev->mcastFilter[3] |= (hash_table[1]>>8)&0xff;
	dev->mcastFilter[4] |= hash_table[2]&0xff;
	dev->mcastFilter[5] |= (hash_table[2]>>8)&0xff;
	dev->mcastFilter[6] |= hash_table[3]&0xff;
	dev->mcastFilter[7] |= (hash_table[3]>>8)&0xff;

	axend_writereg8(EN0_CMD, E8390_NODMA + E8390_PAGE1 );
	for(i = 0; i < 8; i++) 
	{
		axend_writereg8(EN1_MULT_SHIFT(i), dev->mcastFilter[i]);
	}
	axend_writereg8(EN0_CMD, E8390_NODMA + E8390_PAGE0 );

	if(END_FLAGS_GET(&dev->end) & IFF_PROMISC)
	{
		axend_writereg8(EN0_RCR, E8390_RXCONFIG | 0x18 );
	}
	else
	{
		axend_writereg8(EN0_RCR, E8390_RXCONFIG );
	}

	EMPTY_PRT(4, ("Multicast %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\n", 
			dev->mcastFilter[0],
            dev->mcastFilter[1],
            dev->mcastFilter[2],
            dev->mcastFilter[3],
            dev->mcastFilter[4],
            dev->mcastFilter[5],
            dev->mcastFilter[6],
            dev->mcastFilter[7]));	
}

/*******************************************************************************
*
* ax88796PollRcv - routine to receive a packet in polled mode.
*
* This routine is called by a user to try and get a packet from the
* device.
*
* RETURNS: OK upon success.  EAGAIN is returned when no packet is available.
*/

static STATUS ax88796PollRcv( END_DEVICE *dev,  /* device to be polled */
                            M_BLK_ID   pMblk )     /* ptr to buffer */
{
    u_short stat;
    char* pPacket;
    int len;
    PKT         skb;

	ETH_PRT(2, ("ax88796PollRcv()\n"));

    stat = ax88796StatusRead (dev);
    /* TODO - If no packet is available return immediately */

    if( !(stat&AX88796_RXRDY) )
    {
		EMPTY_PRT(0, ("ax88796PollRcv() no data\n"));
        return (EAGAIN);
    }


    pPacket = NULL; /* DUMMY CODE */
    len = 64;       /* DUMMY CODE */

    /* Upper layer must provide a valid buffer. */

    if ((pMblk->mBlkHdr.mLen < len) || (!(pMblk->mBlkHdr.mFlags & M_EXT)))
    {
		EMPTY_PRT(0, ("ax88796PollRcv() PRX bad mblk\n"));
        return (EAGAIN);
    }

    /* TODO - clear any status bits that may be set. */

    /* TODO - Check packet and device for errors */

    END_ERR_ADD (&dev->end, MIB2_IN_UCAST, +1);

    /* TODO - Process device packet into net buffer */
    skb.len = 0;
    skb.pData = pMblk->m_data;
    if( axend_packet_receive( &skb, dev ) )
    {
		EMPTY_PRT(0, ("axend_packet_receive() Error.\n"));
        return (EAGAIN);
    }

/*    bcopy (pPacket, pMblk->m_data, len);   */
    pMblk->mBlkHdr.mFlags |= M_PKTHDR;  /* set the packet header */
    pMblk->mBlkHdr.mLen = len;          /* set the data len */
    pMblk->mBlkPktHdr.len = len;        /* set the total len */

    /* TODO - Done with packet, clean up and give it to the device. */
	EMPTY_PRT(4, ("ax88796PollRcv() OK.\n"));

    return (OK);
}

/*******************************************************************************
*
* ax88796PollSend - routine to send a packet in polled mode.
*
* This routine is called by a user to try and send a packet on the
* device.
*
* RETURNS: OK upon success.  EAGAIN if device is busy.
*/

static STATUS ax88796PollSend( END_DEVICE *dev,   /* device to be polled */
                             M_BLK_ID   pMblk )      /* packet to send */
{
    int         len;
    u_short     stat;
    PKT         skb;
    int         oldv;
	
	ETH_PRT(2, ("ax88796PollSend()\n"));

    /* TODO - test to see if tx is busy */
    stat = ax88796StatusRead (dev);             /* dummy code */
    if ((stat & (AX88796_TINT|AX88796_TFULL)) == 0)
    {
        return ((STATUS) EAGAIN);
    }

    /* TODO - Process the net buffer into a device transmit packet */

    /* TODO - transmit packet */
    len = netMblkToBufCopy (pMblk, (char*)(dev->txBuf), NULL);
    len = max (len, ETHERSMALL);
    skb.len = len;
    skb.pData = (char*)dev->txBuf;

    oldv = intLock();
    axend_start_transmit( &skb, dev );
    intUnlock( oldv );

    /* Bump the statistic counter. */

    END_ERR_ADD (&dev->end, MIB2_OUT_UCAST, +1);

    /* Free the data if it was accepted by device */

    netMblkClFree (pMblk);

	EMPTY_PRT(4, ("ax88796PollSend() OK.\n"));

    return (OK);
}

/*****************************************************************************
*
* ax88796MCastAdd - add a multicast address for the device
*
* This routine adds a multicast address to whatever the driver
* is already listening for.  It then resets the address filter.
*
* RETURNS: OK or ERROR.
*/

static STATUS ax88796MCastAdd( END_DEVICE *dev,      /* device pointer */
                             char* pAddress )           /* new address to add */
{
 //   int error;
    int oldv=intLock();

	ETH_PRT(2, ("ax88796MCastAdd()\n"));

    if (etherMultiAdd (&dev->end.multiList, pAddress) == ENETRESET)
    {
        ax88796AddrFilterSet (dev);
//        ax88796_hash_table( dev );
    }
	
    intUnlock(oldv);
    return (OK);
}

/*****************************************************************************
*
* ax88796MCastDel - delete a multicast address for the device
*
* This routine removes a multicast address from whatever the driver
* is listening for.  It then resets the address filter.
*
* RETURNS: OK or ERROR.
*/

static STATUS ax88796MCastDel( END_DEVICE *dev,  /* device pointer */
                             char *pAddress )       /* address to be deleted */
{
//    int error;
    int oldv=intLock();
	
	ETH_PRT(2, ("ax88796MCastDel()\n"));
	
    if (etherMultiDel (&dev->end.multiList, (char *)pAddress) == ENETRESET)
    {
        ax88796AddrFilterSet (dev);
//        ax88796_hash_table( dev );
    }

	intUnlock(oldv);
    return (OK);
}

/*****************************************************************************
*
* ax88796MCastGet - get the multicast address list for the device
*
* This routine gets the multicast list of whatever the driver
* is already listening for.
*
* RETURNS: OK or ERROR.
*/

static STATUS ax88796MCastGet( END_DEVICE *dev,  /* device pointer */
                             MULTI_TABLE *pTable )  /* address table to be filled in */
{
	ETH_PRT(2, ("ax88796MCastGet()\n"));

    return (etherMultiGet (&dev->end.multiList, pTable));
}


/******************************************************************************
*
* ax88796Unload - unload a driver from the system
*
* This function first brings down the device, and then frees any
* stuff that was allocated by the driver in the load function.
*
* RETURNS: OK or ERROR.
*/

static STATUS ax88796Unload( END_DEVICE *dev )   /* device to be unloaded */
{
	ETH_PRT(2, ("ax88796Unload()\n"));

    END_OBJECT_UNLOAD (&dev->end);

    /* TODO - Free any shared DMA memory */

    return (OK);
}

/*******************************************************************************
*
* ax88796PollStart - start polled mode operations
*
* RETURNS: OK or ERROR.
*/

static STATUS ax88796PollStart( END_DEVICE *dev )    /* device to be polled */
{
    int         oldLevel;
	
	ETH_PRT(2, ("ax88796PollStart()\n"));
	
    oldLevel = intLock ();          /* disable ints during update */

    /* TODO - turn off interrupts */

    (dev->flags) |= AX88796_POLLING;

    ax88796Reset( dev );
    ax88796Config( dev );       /* reconfigure device */

    intUnlock (oldLevel);   /* now ax88796Int won't get confused */

	ETH_PRT(4, ("Started\n"));

    return (OK);
}

/*******************************************************************************
*
* ax88796PollStop - stop polled mode operations
*
* This function terminates polled mode operation.  The device returns to
* interrupt mode.
*
* The device interrupts are enabled, the current mode flag is switched
* to indicate interrupt mode and the device is then reconfigured for
* interrupt operation.
*
* RETURNS: OK or ERROR.
*/

static STATUS ax88796PollStop( END_DEVICE *dev ) /* device to be polled */
{
    int         oldLevel;

	ETH_PRT(2, ("ax88796PollStop()\n"));

    oldLevel = intLock ();  /* disable ints during register updates */

    /* TODO - re-enable interrupts */

    (dev->flags) &= ~AX88796_POLLING;

    ax88796Reset( dev );
    ax88796Config( dev );       /* reconfigure device */

    intUnlock (oldLevel);

	ETH_PRT(4, ("Stopped\n"));

    return (OK);
}

/*******************************************************************************
*
* ax88796StatusRead - get current device state/status
*
* RETURNS: status bits.
*/

static UINT ax88796StatusRead( END_DEVICE *dev )
{
    /* TODO - read and return status bits/register */
    UINT retv = 0;
    unsigned char status = 0;
    unsigned char rxbyte = 0;
	unsigned char data=0;
	
	ETH_PRT(2, ("ax88796StatusRead()\n"));

	axend_readreg8(EN0_ISR, &status);
	axend_readreg8(EN0_RSR, &data);
	rxbyte = (data & ENRSR_RXOK);	// Check Packet Received Intact

    if( !((dev->flags) & AX88796_POLLING) )
    {
        retv |= AX88796_RXON;
    }

    if( status & 0x01 )
    {
        retv |= AX88796_RINT;
    }

    if( status & 0x02 )
    {
        retv |= AX88796_TINT;
    }

    if( dev->tx_pkt_cnt > 2 )
    {
        retv |= AX88796_TFULL;
    }

    if( rxbyte == AX88796_PKT_RDY )
    {
        retv |= AX88796_RXRDY;
    }
    return (retv);
}

#define ___AX88796_SVC_API___

void	SVC_ETHER_Init(void)
{
 	tl_GPIO_Configure (GPIO_ETH_RESET, TLGPIO_GPIO, TLGPIO_OUTPUT, 0, 1);
	tl_SetGPIO(GPIO_ETH_RESET);
	tl_SetGPIO(GPIO_ETH_INT);
 }

/*******************************************************************************
*
* sysEtherEndLoad - initialize the driver and device
*
* This routine initializes the driver and the device to the operational state.
* All of the device specific parameters are passed in the initString.
*
* The string contains the target specific parameters like this:
*
* "register addr:int vector:int level:shmem addr:shmem size:shmem width"
*
* RETURNS: An END object pointer or NULL on error.
*/

END_OBJ* sysEtherEndLoad( char* initString, void *arg)    /* String to be parsed by the driver. */
{
    END_DEVICE  *dev;
//	char tmp;

	ETH_PRT(2, ("sysEtherEndLoad()\n"));

    if (initString == NULL)
    {
		EMPTY_PRT(0, ("initString == NULL\n"));
        return (NULL);
    }

    if (initString[0] == '\0')
    {
		EMPTY_PRT(0, ("initString[0] == 0\n"));
        bcopy((char *)AX88796_DEV_NAME, initString, AX88796_DEV_NAME_LEN);
        return ((END_OBJ *)OK);
    }

    /* allocate the device structure */

    dev = (END_DEVICE *)calloc(sizeof (END_DEVICE), 1);
    if (dev == NULL)
    {
		EMPTY_PRT(0, ("calloc() Error.\n"));
        goto errorExit;
    }	
    ax_end = dev;
	
    memset( (void*)dev, 0, sizeof( END_DEVICE ) );
    (dev->flags) = 0;
    (dev->op_mode) = AX88796_MEDIA_MODE;

    /* parse the init string, filling in the device structure */
    if (ax88796Parse(dev, initString) == ERROR)
    {
		EMPTY_PRT(0, ("ax88796Parse() Error.\n"));
        goto errorExit;
    }

    /* Ask the BSP to provide the ethernet address. */
    ax88796Reset (dev);
    ax88796Config (dev);

    /* initialize the END and MIB2 parts of the structure
     * The M2 element must come from m2Lib.h
     * This ax88796 is set up for a DIX type ethernet device.
     */
    if ((END_OBJ_INIT (&dev->end, 
					(DEV_OBJ *)dev, 
					AX88796_DEV_NAME,
					dev->unit, 
					&ax88796FuncTable,
					"END Template Driver.") == ERROR) || 
		(END_MIB_INIT (&dev->end, 
					M2_ifType_ethernet_csmacd,
					&dev->enetAddr[0], 
					6, 
					ETHERMTU,
					END_SPEED) == ERROR))
    {
		EMPTY_PRT(0, ("END_OBJ_INIT()/END_MIB_INIT() Error.\n"));
        goto errorExit;
    }

    /* Perform memory allocation/distribution */
    if (ax88796MemInit (dev) == ERROR)
    {
		EMPTY_PRT(0, ("ax88796MemInit() Error\n"));
        goto errorExit;
    }

    /* reset and reconfigure the device */
//    ax88796Reset (dev);
//    ax88796Config (dev);

    /* set the flags to indicate readiness */
    END_OBJ_READY (&dev->end,
                 IFF_UP | IFF_RUNNING | IFF_NOTRAILERS | IFF_BROADCAST | IFF_MULTICAST | IFF_SIMPLEX);
	
	EMPTY_PRT(0, ("Done loading Template...\n"));

    return (&dev->end);

errorExit:
    if (dev != NULL)
    free ((char *)dev);

    return NULL;
}

static void	ethernet_stack(void)
{
//	BOOT_PARAMS		params;
#ifdef INCLUDE_END
	int 			count;
	END_TBL_ENTRY	*pDevTbl;
	END_OBJ			*pCookie = NULL;
	END_OBJ			*pEnd;
	EG_S32 			iRetcode = OK;
#endif /* INCLUDE_END */

	ETH_PRT(2, ("ethernet_stack()\n"));

 
	/* Add in mux ENDs. */
	for (count = 0, pDevTbl = axEndDevTbl; pDevTbl->endLoadFunc != END_TBL_END; pDevTbl++, count++)
	{
		EMPTY_PRT(4, ("AX88796B  : Endload String %s...\n", pDevTbl->endLoadString));
	
		/* Make sure that WDB has not already installed the device. */
		if (!pDevTbl->processed)
		{
			EMPTY_PRT(4, ("AX88796B  : Endload unit %d...\n", pDevTbl->unit));
			pCookie = muxDevLoad(pDevTbl->unit,
						 pDevTbl->endLoadFunc,
						 pDevTbl->endLoadString,
						 pDevTbl->endLoan, pDevTbl->pBSP);
			if (pCookie == NULL)
			{
				EMPTY_PRT(0, ("muxDevLoad failed for device entry %d!\n", count));
			}
			else
			{
				pDevTbl->processed = TRUE;
				if (muxDevStart(pCookie) == ERROR)
				{
					EMPTY_PRT(0, ("muxDevStart failed for device entry %d!\n", count));
				}
			}
		}
	}
  
	pEnd = endFindByName(AX88796_DEV_NAME, 0);
	
	if (ipAttach(0, AX88796_DEV_NAME) != OK)
	{
		EMPTY_PRT(0, ("muxDevStart failed for device entry %d!\n", count));
		iRetcode = ERROR;
    }
	else
	{
		EMPTY_PRT(0, ("IP Attach success...(ETHERNET)\n"));
	}


	EMPTY_PRT(4, ("dhcp hook\n"));
//	dhcpcHook();

	ifRouteDelete(AX88796_DEV_NAME,0);
	mRouteDelete("0.0.0.0",0,0,0);
	ifFlagChange( "ax0", IFF_UP, TRUE ); /* Bring UP the END Network Interface */
#if 0	
	iRetcode = getDhcpLease("ax0"); 
	if (iRetcode == OK)
	{
		getDhcpInfo();	
		_cmResolvLibInit();
	}
#endif
}

OSA_TaskFunction_t	task_ethernet_stack(void *arg)
{
	ETH_PRT(2, ("task_ethernet_stack()\n"));

	arg = arg;
	
	(void) pingLibInit ();              /* initialize the ping utility */ 
	ethernet_stack();
	OSA_DeleteTask(OSA_GetTaskID());
	return	OK;
}

#define ___AX88796_TEST_API___

void set_mediamode(int mode)
{
	EG_U8 dsr=0;

	axend_writereg8(EN0_CMD, (E8390_NODMA+E8390_PAGE0+E8390_STOP));
	switch (mode)
	{
		case 0:
			EMPTY_PRT(2, ("MEDIA_AUTO\n"));
			axend_mdio_write(0x10,0x00,0x1200);
			axend_mdio_write(0x10,0x04,0x01E1);
			break;
			
		case 1:
			EMPTY_PRT(2, ("MEDIA_100FULL\n"));
			axend_mdio_write(0x10,0x00,0x1200);
			axend_mdio_write(0x10,0x04,0x0501);
			break;
		
		case 2:
			EMPTY_PRT(2, ("MEDIA_100HALF\n"));
			axend_mdio_write(0x10,0x00,0x1200);
			axend_mdio_write(0x10,0x04,0x0081);
			break;
		
		case 3:
			EMPTY_PRT(2, ("MEDIA_10FULL\n"));
			axend_mdio_write(0x10,0x00,0x1200);
			axend_mdio_write(0x10,0x04,0x0441);
			break;
		
		case 4:
			EMPTY_PRT(2, ("MEDIA_10HALF\n"));
			axend_mdio_write(0x10,0x00,0x1200);
			axend_mdio_write(0x10,0x04,0x0021);
			break;
	}


	axend_readreg8(EN0_SR, &dsr);
	EMPTY_PRT(1, ("DSR 0x%X\n", dsr));
}

void get_mediamode(void)
{
	int value_mr0, value_mr1, value_mr2, value_mr3, value_mr4;
	EG_U8 dsr=0;
	
	value_mr0 = axend_mdio_read(0x10, 0x00);
	value_mr1 = axend_mdio_read(0x10, 0x01);
	value_mr2 = axend_mdio_read(0x10, 0x02);
	value_mr3 = axend_mdio_read(0x10, 0x03);
	value_mr4 = axend_mdio_read(0x10, 0x04);

	axend_readreg8(EN0_SR, &dsr);
	
	EMPTY_PRT(1, ("MR0 0x%x, MR1 0x%X, MR2 0x%X, MR2 0x%X, MR4 0x%X, DSR 0x%X\n", value_mr0, value_mr1, value_mr2, value_mr3, value_mr4, dsr));
}

void	test_createEthernet(void)
{
	OSA_TaskID_t taskID;	
	taskID = OSA_CreateTask((OSA_TaskFunction_t)task_ethernet_stack, "EthernetTask", SVC_GWM_TASK_PRIORITY, 5120, 0);	
}

void print_deviceinfo(void)
{
	printf("IOBase\t0x%X\n", ax_end->IOBase);	
	printf("unit\t0x%X\n", ax_end->unit);
	printf("ivec\t0x%X\n", ax_end->ivec);
	printf("flags\t0x%lX\n", ax_end->flags);
	printf("rxHandling\t0x%X\n", ax_end->rxHandling);
	printf("io_mode\t0x%X\n", ax_end->io_mode);
	printf("tx_pkt_cnt\t0x%X\n", ax_end->tx_pkt_cnt);
	printf("device_wait_reset\t0x%X\n", ax_end->device_wait_reset);
	printf("queue_pkt_len\t0x%X\n", ax_end->queue_pkt_len);
	printf("nic_type\t0x%X\n", ax_end->nic_type);
	printf("reg0\t0x%X\n", ax_end->reg0);
	printf("op_mode\t0x%X\n", ax_end->op_mode);
}

void	print_axreg(void)
{
	EG_U8 data;
	EG_U8 temp;

	axend_readreg8(EN0_CMD, &temp);
	printf("** EN0_CMD\t%02X\n", temp);

	axend_readreg8(EN0_CLDALO, &data);
	printf("** EN0_CLDALO\t%02X\n", data);
	
	axend_readreg8(EN0_CLDAHI, &data);
	printf("** EN0_CLDAHI\t%02X\n", data);

	axend_readreg8(EN0_PSTART, &data);
	printf("** EN0_PSTART\t%02X\n", data);

	axend_readreg8(EN0_PSTOP, &data);
	printf("** EN0_PSTOP\t%02X\n", data);

	axend_readreg8(EN0_BNRY, &data);
	printf("** EN0_BNRY\t%02X\n", data);

	axend_readreg8(EN0_TSR, &data);
	printf("** EN0_TSR\t%02X\n", data);

	axend_readreg8(EN0_NCR, &data);
	printf("** EN0_NCR\t%02X\n", data);
	
	axend_readreg8(EN0_CPR, &data);
	printf("** EN0_CPR\t%02X\n", data);

	axend_readreg8(EN0_ISR, &data);
	printf("** EN0_ISR\t%02X\n", data);

	axend_readreg8(EN0_CRDALO, &data);
	printf("** EN0_CRDALO\t%02X\n", data);

	axend_readreg8(EN0_CRDAHI, &data);
	printf("** EN0_CRDAHI\t%02X\n", data);

	axend_readreg8(EN0_RSR, &data);
	printf("** EN0_RSR\t%02X\n", data);
	
	axend_readreg8(EN0_COUNTER0, &data);
	printf("** EN0_COUNTER0\t%02X\n", data);
	
	axend_readreg8(EN0_COUNTER1, &data);
	printf("** EN0_COUNTER1\t%02X\n", data);

	axend_readreg8(EN0_COUNTER2, &data);
	printf("** EN0_COUNTER2\t%02X\n", data);

	axend_readreg8(EN0_MII_EEPROM, &data);
	printf("** MII_EEPROM\t%02X\n", data);

	axend_readreg8(EN0_BTCR, &data);
	printf("** EN0_BTCR\t%02X\n", data);

	axend_readreg8(EN0_SR, &data);
	printf("** EN0_SR\t%02X\n", data);

	axend_readreg8(EN0_FLOW, &data);
	printf("** EN0_FLOW\t%02X\n", data);

	axend_readreg8(EN0_MCR, &data);
	printf("** EN0_MCR\t%02X\n", data);

	axend_readreg8(EN0_CTEPR, &data);
	printf("** EN0_CTEPR\t%02X\n", data);

	axend_writereg8(EN0_CMD, E8390_NODMA | E8390_PAGE3);
	axend_readreg8(EN3_MISC, &data);
	printf("** EN3_MISC\t%02X\n", data);	
	
	axend_writereg8(EN0_CMD, temp);
}

