
#ifndef __AX88796END_H__
#define __AX88796END_H__

#define TX_PAGES      		0x1f   // 0x11  // 0x0B
#define NESM_START_PG		0x40	/* First page of TX buffer */
#define NESM_RX_START_PG	(NESM_START_PG + TX_PAGES + 1)	/* First page of RX buffer */
#define NESM_STOP_PG		0x80	/* Last page +1 of RX ring */

/* Some generic ethernet register configurations. */
 
#define E8390_RXCONFIG		 0x4	/* EN0_RCR: broadcasts, no multicast,errors */
#define E8390_RXOFF			0x20	/* EN0_RCR: Accept no packets */
#define E8390_FULLDUPLEX	0x80	/* EN0_TCR: Normal transmit mode */
#define ENTCR_LOOPBACK		0x02	/* EN0_TCR: Internal AC88796B loop-back */

/*  Register accessed at EN_CMD, the 8390 base addr.  */
#define E8390_STOP			0x01   /* Stop and reset the chip */
#define E8390_START			0x02   /* Start the chip, clear reset */
#define E8390_TRANS			0x04   /* Transmit a frame */
#define E8390_RREAD			0x08   /* Remote read */
#define E8390_RWRITE        0x10   /* Remote write  */
#define E8390_NODMA			0x20   /* Remote DMA */
#define E8390_PAGE0			0x00   /* Select page chip registers */
#define E8390_PAGE1			0x40   /* using the two high-order bits */
#define E8390_PAGE2			0x80   /* Page 2 is invalid. */
#define E8390_PAGE3			0xc0   /* Page 3 for AX88796B */

/* Interrupt mask Register */
#define E8390_PRXE			0x01 	/* Racket Received Interrupt Enable/ */
#define E8390_PTXE			0x02 	/* Packet Transmitted Interrupt Enable */
#define E8390_RXEE			0x04 	/* Receive Error Interrupt Enable */
#define E8390_TXEE			0x08 	/* Transmit Error Interrupt Enable */
#define E8390_OVWE			0x10 	/* Overwrite Interrupt Enable */
#define E8390_CNTE			0x20 	/* Counter Overflow Interrupt Enable */
#define E8390_RDCE			0x40 	/* DMA Complete Interrupt Enable */
#define E8390_IMRALL		0x7F

#define EI_SHIFT(x)			((x) << 1)

#define EN0_CMD				EI_SHIFT(0x00)  /* The command register (for all pages) */
/* Page 0 register offsets. */
#define EN0_CLDALO			EI_SHIFT(0x01)	/* Low byte of current local dma addr  RD */
#define EN0_PSTART			EI_SHIFT(0x01)	/* Starting page of ring bfr WR */
#define EN0_CLDAHI			EI_SHIFT(0x02)	/* High byte of current local dma addr  RD */
#define EN0_PSTOP			EI_SHIFT(0x02)	/* Ending page +1 of ring bfr WR */
#define EN0_BNRY       		EI_SHIFT(0x03)	/* Boundary page of ring bfr RD WR */
#define EN0_TSR             EI_SHIFT(0x04)	/* Transmit status reg RD */
#define EN0_TPSR			EI_SHIFT(0x04)	/* Transmit starting page WR */
#define EN0_NCR             EI_SHIFT(0x05)	/* Number of collision reg RD */
#define EN0_TBCR0			EI_SHIFT(0x05)	/* Low  byte of tx byte count WR */
#define EN0_CPR				EI_SHIFT(0x06)	/* CurrentPage Register RD */
#define EN0_TBCR1			EI_SHIFT(0x06)	/* High byte of tx byte count WR */
#define EN0_ISR             EI_SHIFT(0x07)	/* Interrupt status reg RD WR */
#define EN0_CRDALO			EI_SHIFT(0x08)	/* low byte of current remote dma address RD */
#define EN0_RSAR0			EI_SHIFT(0x08)	/* Remote start address reg 0 */
#define EN0_CRDAHI			EI_SHIFT(0x09)	/* high byte, current remote dma address RD */
#define EN0_RSAR1			EI_SHIFT(0x09)	/* Remote start address reg 1 */
#define EN0_RBCR0			EI_SHIFT(0x0a)	/* Remote byte count reg WR */
#define EN0_RBCR1			EI_SHIFT(0x0b)	/* Remote byte count reg WR */
#define EN0_RSR             EI_SHIFT(0x0c)	/* rx status reg RD */
#define EN0_RCR				EI_SHIFT(0x0c)	/* RX configuration reg WR */
#define EN0_TCR				EI_SHIFT(0x0d)	/* TX configuration reg WR */
#define EN0_COUNTER0        EI_SHIFT(0x0d)	/* Rcv alignment error counter RD */
#define EN0_DCFG			EI_SHIFT(0x0e)	/* Data configuration reg WR */
#define EN0_COUNTER1        EI_SHIFT(0x0e)	/* Rcv CRC error counter RD */
#define EN0_IMR             EI_SHIFT(0x0f)	/* Interrupt mask reg WR */
#define EN0_COUNTER2        EI_SHIFT(0x0f)	/* Rcv missed frame error counter RD */
#define EN0_DP        		EI_SHIFT(0x10)
#define EN0_PHYID			EI_SHIFT(0x10)
#define EN0_MII_EEPROM  	EI_SHIFT(0x14)
#define EN0_BTCR			EI_SHIFT(0x15)
#define EN0_SR              EI_SHIFT(0X17)	/* Louis add for AX88796B Status Register */
#define EN0_FLOW			EI_SHIFT(0x1a)	/* AX88796B Flow control register */
#define EN0_MCR             EI_SHIFT(0X1b)  /* Mac configure register */
#define EN0_CTEPR			EI_SHIFT(0x1c)	/* Current TX End Page */
#define EN0_VID0			EI_SHIFT(0x1c)	/* VLAN ID 0 */
#define EN0_VID1			EI_SHIFT(0x1d)	/* VLAN ID 1 */
#define EN0_BER				EI_SHIFT(0x1e)	/* Big-Endian Register */
#define EN0_RESET			EI_SHIFT(0X1f)		/* Issue a read to reset, a write to clear. */

#define EN0_DATA_ADDR		0x0800

#define ENVLAN_ENABLE		0x08

/* Bits in EN0_ISR - Interrupt status register */
#define ENISR_RX			0x01	/* Receiver, no error */
#define ENISR_TX			0x02	/* Transmitter, no error */
#define ENISR_RX_ERR		0x04	/* Receiver, with error */
#define ENISR_TX_ERR  		0x08	/* Transmitter, with error */
#define ENISR_OVER			0x10	/* Receiver overwrote the ring */
#define ENISR_COUNTERS		0x20	/* Counters need emptying */
#define ENISR_RDC			0x40	/* remote dma complete */
#define ENISR_RESET			0x80	/* Reset completed */
#define ENISR_ALL			(ENISR_RX | ENISR_TX | ENISR_RX_ERR | ENISR_TX_ERR | ENISR_OVER | ENISR_COUNTERS)/* Interrupts we will enable */

	
/* Bits in EN0_DCFG - Data config register */
#define ENDCFG_WTS			0x01	/* word transfer mode selection */
#define ENDCFG_BOS			0x02	/* byte order selection */

#define ENFLOW_ENABLE		0x80		/* Flow Control Enable */
#define ENBP_ENABLE			0x40		/* Back Pressure Enable */
#define ENMCR_BTBENABLE    	0x20		/* Enable Back-To-Back Transmission Control */

#define EN3_TBR         	EI_SHIFT(0x0d)	/* Transmit Buffer Ring Control Register */
#define ENTBR_ENABLE    	0x01			/* Enable Transmit Buffer Ring */

/* Page 1 register offsets. */
#define EN1_PHYS            EI_SHIFT(0x01)  /* This board's physical enet addr RD WR */
#define EN1_PHYS_SHIFT(i)   EI_SHIFT(i+1)   /* Get and set mac address */
#define EN1_CURPAG          EI_SHIFT(0x07)  /* Current memory page RD WR */
#define EN1_MULT            EI_SHIFT(0x08)  /* Multicast filter mask array (8 bytes) RD WR */
#define EN1_MULT_SHIFT(i)   EI_SHIFT(8+i)   /* Get and set multicast filter */
#define EN1_PAR0        	EI_SHIFT(0x01)    // Physical Address Register 0
#define EN1_PAR1        	EI_SHIFT(0x01)    // Physical Address Register 1
#define EN1_PAR2       	 	EI_SHIFT(0x01)    // Physical Address Register 2
#define EN1_PAR3     	   	EI_SHIFT(0x01)    // Physical Address Register 3
#define EN1_PAR4        	EI_SHIFT(0x01)    // Physical Address Register 4
#define EN1_PAR5        	EI_SHIFT(0x01)    // Physical Address Register 5
#define EN1_MAR0        	EI_SHIFT(0x01)    // Multicast Address Register 0
#define EN1_MAR1        	EI_SHIFT(0x01)    // Multicast Address Register 1
#define EN1_MAR2        	EI_SHIFT(0x01)    // Multicast Address Register 2
#define EN1_MAR3        	EI_SHIFT(0x01)    // Multicast Address Register 3
#define EN1_MAR4        	EI_SHIFT(0x01)    // Multicast Address Register 4
#define EN1_MAR5        	EI_SHIFT(0x01)    // Multicast Address Register 5
#define EN1_MAR6        	EI_SHIFT(0x01)    // Multicast Address Register 6
#define EN1_MAR7        	EI_SHIFT(0x01)    // Multicast Address Register 7

/* Bits in received packet status byte and EN0_RSR*/
#define ENRSR_RXOK			0x01	/* Received a good packet */
#define ENRSR_CRCERR		0x02	/* CRC error */
#define ENRSR_ALIGNERR		0x04	/* frame alignment error */
#define ENRSR_FO			0x08	/* FIFO overrun */
#define ENRSR_MISSEDPKT		0x10	/* missed pkt */
#define ENRSR_PHY			0x20	/* physical/multicast address */
#define ENRSR_DIS			0x40	/* receiver disable. set in monitor mode */
#define ENRSR_DEF			0x80	/* deferring */

/* Transmitted packet status, EN0_TSR. */
#define ENTSR_PTX    		0x01   /* Packet transmitted without error */
#define ENTSR_ND     		0x02   /* The transmit wasn't deferred. */
#define ENTSR_COL   		0x04   /* The transmit collided at least once. */
#define ENTSR_ABT    		0x08   /* The transmit collided 16 times, and was deferred. */
#define ENTSR_CRS    		0x10   /* The carrier sense was lost. */
#define ENTSR_FU     		0x20   /* A "FIFO underrun" occurred during transmit. */
#define ENTSR_CDH    		0x40   /* The collision detect "heartbeat" signal was lost. */
#define ENTSR_OWC   		0x80   /* There was an out-of-window collision. */

/* Current TX End Page Register */
#define ENCTEPRT_TXCQF		0x80
#define ENCTEPRT_CTEPR		0x7F

/* Power Management register offsets. */
#define EN3_BM0				EI_SHIFT(0x01)
#define EN3_BM1				EI_SHIFT(0x02)
#define EN3_BM2				EI_SHIFT(0x03)
#define EN3_BM3				EI_SHIFT(0x04)
#define EN3_BM10CRC			EI_SHIFT(0x05)
#define EN3_BM32CRC			EI_SHIFT(0x06)
#define EN3_BMOFST			EI_SHIFT(0x07)
#define EN3_LSTBYT			EI_SHIFT(0x08)
#define EN3_BMCD			EI_SHIFT(0x09)
#define EN3_WUCS			EI_SHIFT(0x0a)
#define EN3_PMR				EI_SHIFT(0x0b)
#define EN3_MISC			EI_SHIFT(0x0d)

/* Bits in Wake up Control */
#define ENWUCS_MPEN			0x01
#define ENWUCS_WUEN			0x02
#define ENWUCS_LINK			0x04

/* Bits in PM Control */
#define ENPMR_D1			0x01
#define ENPMR_D2			0x02

#define ENBTCR_IRQ_PUSHPULL 0x20

#define MEDIA_AUTO			0
#define MEDIA_100FULL		1
#define MEDIA_100HALF		2
#define MEDIA_10FULL		3
#define MEDIA_10HALF		4

/*======================================================================
    MII interface support
======================================================================*/
#define MDIO_SHIFT_CLK		0x01
#define MDIO_DATA_WRITE0	0x00
#define MDIO_DATA_WRITE1	0x08
#define MDIO_DATA_READ		0x04
#define MDIO_MASK			0x0f
#define MDIO_ENB_IN			0x02

typedef struct
{
    int 			len;
    unsigned char	*pData;
} PKT;  /* A dummy DMA data packet */

typedef struct _dm_pkt_hdr {
  unsigned char		status;		/* status */
  unsigned char		next;   	/* pointer to next packet. */
  unsigned short	count; 		/* header + packet length in bytes */
}dm_pkt_hdr;

typedef struct _dm_page_info
{
	unsigned char	tx_curr_page;
	unsigned char	tx_start_page;
	unsigned char	tx_stop_page;
	unsigned char	rx_start_page;
	unsigned char	rx_stop_page;
	unsigned char	rx_curr_page;	/* Read pointer in buffer  */
	unsigned char 	tx_prev_ctepr;
	unsigned char	tx_full;
}dm_page_info;

typedef struct rfd
{
    PKT *  pPkt;
    struct rfd * next;
} RFD;  /* dummy rx frame descriptor */

extern void	SVC_ETHER_Init(void);

#endif

