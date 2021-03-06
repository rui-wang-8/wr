/* if.c - network interface utility routines */

/* Copyright 1984-1996 Wind River Systems, Inc. */
#include "copyright_wrs.h"

/*	$NetBSD: if.c,v 1.18 1994/10/30 21:48:46 cgd Exp $	*/

/*
 * Copyright (c) 1980, 1986, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)if.c	8.3 (Berkeley) 1/4/94
 */

/*
modification history
--------------------
01j,10mar00,ann  Added the callback to the m2Lib routine
01i,24mar99,ead  Added a call to m2SetIfLastChange() in ifioctl() when the
                 interfaces operational status changes. (SPR #23290)
01h,16jul98,n_s  fixed ifioctl to pass correct flags to driver Ioctl. SPR 21124 
01g,16dec97,vin  fixed if_dettach SPR 9970.
01f,10dec97,vin  added spl protection to ifreset, if_dettach, if_attach,
		 ifIndexToIfp SPR 9987.
01e,05oct97,vin  fixed if_dettach, for changes in multicast code.
01d,12aug97,rjc  made if_index global to support non ip interfaces (SPR 9060).
01c,02jul97,vin  fixed warnings. added _rtIfaceMsgHook for scaling out routing
		 sockets.
01b,05dec96,vin  moved ifafree() from route.c(), added ifIndexToIfp(),
		 deleted ifnet_addrs in ifattach(), made if_index static.
		 made ifqmaxlen static, replaced calloc with MALLOC, replaced
		 free(..) with FREE(..)
01a,03mar96,vin  created from BSD4.4 stuff,integrated with 02t of if.c.
		 rewrote if_dettach(), added in_ifaddr_remove in if_dettach
*/

/*
DESCRIPTION
*/

#include "vxWorks.h"
#include "sys/socket.h"
#include "net/socketvar.h"
#include "net/protosw.h"
#include "net/route.h"
#include "net/if_dl.h"
#include "net/if_subr.h"
#include "sys/ioctl.h"
#include "errno.h"

#include "net/if.h"

#include "wdLib.h"

#include "net/systm.h"
#include "net/mbuf.h"
#ifdef INET
#include "netinet/in_var.h"
#endif /* INET */
#include "m2Lib.h"

IMPORT void netJobAdd ();
extern pfctlinput(int cmd, struct sockaddr * sa);
extern sysClkRateGet();
extern void in_ifaddr_remove ();
extern arpioctl (int cmd, caddr_t data); 

/* XXX temporary fix should be removed after updating all drivers */

extern void ether_ifattach(struct ifnet * ifp);
extern int ether_output (); 


int ifAttachChange;	/* Set when a change in the Net I/F list occurs */
struct ifnet *ifnet;	/* head of linked list of interfaces */

/* locals */
LOCAL WDOG_ID ifslowtimoWd;	/* watchdog timer for slowtimo routine */
static int ifqmaxlen = IFQ_MAXLEN;
static char * sprint_d(unsigned int, char *, int);

/* globals */
int if_index = 0;

/*
 * Network interface utility routines.
 *
 * Routines with ifa_ifwith* names take sockaddr *'s as
 * parameters.
 */

void ifinit()
    {
    register struct ifnet *ifp;

    for (ifp = ifnet; ifp; ifp = ifp->if_next)
	if (ifp->if_snd.ifq_maxlen == 0)
	    ifp->if_snd.ifq_maxlen = ifqmaxlen;

    /* XXX using watchdogs good idea? -gae */
    ifslowtimoWd = wdCreate ();
    if_slowtimo ();
    }


/*
 * Call each interface on a reset.
 */
void ifreset ()

    {
    register struct ifnet *ifp;
    int 	s;

    s = splnet ();
    for (ifp = ifnet; ifp; ifp = ifp->if_next)
	if (ifp->if_reset)
	    (*ifp->if_reset)(ifp->if_unit);
    splx (s);
    }

/*
 * Attach an interface to the list of "active" interfaces.
 */

void if_attach(ifp)
    struct ifnet *ifp;
    {
    unsigned socksize, ifasize;
    int namelen, unitlen, masklen;
    char workbuf[12], *unitname;
    register struct ifnet **p = &ifnet;
    register struct sockaddr_dl *sdl;
    register struct ifaddr *ifa;
    extern void link_rtrequest();
    int 	s;

    /* make interface not use trailer protocol by default */
	
    ifp->if_flags |= IFF_NOTRAILERS;

    /* make send queue maxlen be default if none specified;
     * this relaxes the requirement that all interfaces be attached
     * before calling ifinit */
	
    if (ifp->if_snd.ifq_maxlen == 0)
	ifp->if_snd.ifq_maxlen = ifqmaxlen;
	
    s = splnet ();
    while (*p)
	p = &((*p)->if_next);
    splx (s);

    *p = ifp;

    /* Notify MIB-II of change */
	
    ifAttachChange++;

    ifp->if_index = ++if_index;

    /*
     * create a Link Level name for this device
     */
    unitname = sprint_d((u_int)ifp->if_unit, workbuf, sizeof(workbuf));
    namelen = strlen(ifp->if_name);
    unitlen = strlen(unitname);
#define _offsetof(t, m) ((int)((caddr_t)&((t *)0)->m))
    masklen = _offsetof(struct sockaddr_dl, sdl_data[0]) +
    unitlen + namelen;
    socksize = masklen + ifp->if_addrlen;
#define ROUNDUP(a) (1 + (((a) - 1) | (sizeof(long) - 1)))
    socksize = ROUNDUP(socksize);
    if (socksize < sizeof(*sdl))
	socksize = sizeof(*sdl);
    ifasize = sizeof(*ifa) + 2 * socksize;
    MALLOC (ifa, struct ifaddr *, ifasize, MT_IFADDR, M_WAIT); 
    if (ifa == 0)
	return;
    bzero((caddr_t)ifa, ifasize);
    sdl = (struct sockaddr_dl *)(ifa + 1);
    sdl->sdl_len = socksize;
    sdl->sdl_family = AF_LINK;
    bcopy(ifp->if_name, sdl->sdl_data, namelen);
    bcopy(unitname, namelen + (caddr_t)sdl->sdl_data, unitlen);
    sdl->sdl_nlen = (namelen += unitlen);
    sdl->sdl_index = ifp->if_index;
    sdl->sdl_type = ifp->if_type;
    ifa->ifa_ifp = ifp;
    ifa->ifa_next = ifp->if_addrlist;
    ifa->ifa_rtrequest = link_rtrequest;
    ifp->if_addrlist = ifa;
    ifa->ifa_addr = (struct sockaddr *)sdl;
    sdl = (struct sockaddr_dl *)(socksize + (caddr_t)sdl);
    ifa->ifa_netmask = (struct sockaddr *)sdl;
    sdl->sdl_len = masklen;
    while (namelen != 0)
	sdl->sdl_data[--namelen] = 0xff;

    m2IfTableUpdate(ifp, M2_IF_TABLE_INSERT);

    /* XXX Temporary fix before changing ethernet drivers */

    s = splnet ();
    if (ifp->if_output == ether_output)
	ether_ifattach (ifp);
    splx (s);
    }

/***************************************************************************
*
* if_dettach - dettaches an interface from the linked list of interfaces
*
* if_dettach is used to dettach an interface from the linked list of
* all available interfaces.  This function needs to be used with
* if_down () when shutting down a network interface.
*/

void if_dettach (ifp)
    struct ifnet *ifp;			/* inteface pointer */

    {
    FAST struct ifnet **	pPtrIfp;
    FAST struct ifaddr * 	pIfAddr;
    FAST struct ifaddr **	pPtrIfAddr;
    int 	s;

    pPtrIfp = &ifnet; 		
    s = splnet ();
    while (*pPtrIfp != NULL)
	{
	if (*pPtrIfp == ifp)
	    {
	    pPtrIfAddr = &(*pPtrIfp)->if_addrlist; 
	    while (*pPtrIfAddr != NULL)
		{
		pIfAddr = *pPtrIfAddr; 
		
		*pPtrIfAddr = (*pPtrIfAddr)->ifa_next; 
#ifdef INET 
		if (pIfAddr->ifa_addr->sa_family == AF_INET)
		    in_ifaddr_remove (pIfAddr); 
		else 
#endif /* INET */
		    {
		    IFAFREE (pIfAddr);
		    }
		}
#ifdef INET
            if ((*pPtrIfp)->pInmMblk != NULL)
                in_delmulti ((*pPtrIfp)->pInmMblk, NULL);
#endif /* INET */
	    *pPtrIfp = (*pPtrIfp)->if_next; 
	    break; 
	    }
	pPtrIfp = &(*pPtrIfp)->if_next; 
	}

    /* Notify MIB-II of change */

    ifAttachChange++;
    m2IfTableUpdate(ifp, M2_IF_TABLE_REMOVE);

    splx (s);
    }

/*******************************************************************************
*
* ifIndexToIfp - obtain the ifnet pointer from the index
*
* This function obtains the ifnet pointer from the if_index.
* The interface index is unique for every new interface added.
* 
* NOMANUAL
*
* RETURNS ifnet/NULL
*/

struct ifnet * ifIndexToIfp
    (
    int		ifIndex		/* index of the interface to find */
    )
    {
    register struct ifnet * 	pIfp = NULL;
    int 	s;

    /* traverse the list of ifnet structures in the system */

    s = splnet ();
    for (pIfp = ifnet; pIfp != NULL; pIfp = pIfp->if_next)
	{
	if (pIfp->if_index == ifIndex)
	    break;
	}
    splx (s);

    return (pIfp); 
    }

/*
 * Locate an interface based on a complete address.
 */
/*ARGSUSED*/
struct ifaddr *
ifa_ifwithaddr(addr)
	register struct sockaddr *addr;
{
	register struct ifnet *ifp;
	register struct ifaddr *ifa;

#define	equal(a1, a2) \
  (bcmp((caddr_t)(a1), (caddr_t)(a2), ((struct sockaddr *)(a1))->sa_len) == 0)
	for (ifp = ifnet; ifp; ifp = ifp->if_next)
	    for (ifa = ifp->if_addrlist; ifa; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr->sa_family != addr->sa_family)
			continue;
		if (equal(addr, ifa->ifa_addr))
			return (ifa);
		if ((ifp->if_flags & IFF_BROADCAST) && ifa->ifa_broadaddr &&
		    equal(ifa->ifa_broadaddr, addr))
			return (ifa);
	}
	return ((struct ifaddr *)0);
}
/*
 * Locate the point to point interface with a given destination address.
 */
/*ARGSUSED*/
struct ifaddr *
ifa_ifwithdstaddr(addr)
	register struct sockaddr *addr;
{
	register struct ifnet *ifp;
	register struct ifaddr *ifa;

	for (ifp = ifnet; ifp; ifp = ifp->if_next) 
	    if (ifp->if_flags & IFF_POINTOPOINT)
		for (ifa = ifp->if_addrlist; ifa; ifa = ifa->ifa_next) {
			if (ifa->ifa_addr->sa_family != addr->sa_family)
				continue;
			if (equal(addr, ifa->ifa_dstaddr))
				return (ifa);
	}
	return ((struct ifaddr *)0);
}

/*
 * Find an interface on a specific network.  If many, choice
 * is most specific found.
 */
struct ifaddr *
ifa_ifwithnet(addr)
	struct sockaddr *addr;
{
	register struct ifnet *ifp;
	register struct ifaddr *ifa;
	struct ifaddr *ifa_maybe = (struct ifaddr *) 0;
	u_int af = addr->sa_family;
	char *addr_data = addr->sa_data, *cplim;

	if (af == AF_LINK) {
	    register struct sockaddr_dl *sdl = (struct sockaddr_dl *)addr;
	    /* eliminated need for ifnet_addrs array*/	    
	    if (sdl->sdl_index && sdl->sdl_index <= if_index)
		{
		ifp = ifIndexToIfp (sdl->sdl_index); 
		for (ifa = ifp->if_addrlist; ifa != NULL; ifa = ifa->ifa_next)
		    if (ifa->ifa_addr->sa_family == af)
			break; 
		return (ifa); 
		}
	}
	for (ifp = ifnet; ifp; ifp = ifp->if_next)
	    for (ifa = ifp->if_addrlist; ifa; ifa = ifa->ifa_next) {
		register char *cp, *cp2, *cp3;

		if (ifa->ifa_addr->sa_family != af || ifa->ifa_netmask == 0)
			next: continue;
		cp = addr_data;
		cp2 = ifa->ifa_addr->sa_data;
		cp3 = ifa->ifa_netmask->sa_data;
		cplim = ifa->ifa_netmask->sa_len + (char *)ifa->ifa_netmask;
		while (cp3 < cplim)
			if ((*cp++ ^ *cp2++) & *cp3++)
				goto next;
		if (ifa_maybe == 0 ||
		    rn_refines((caddr_t)ifa->ifa_netmask,
		    (caddr_t)ifa_maybe->ifa_netmask))
			ifa_maybe = ifa;
	    }
	return (ifa_maybe);
}

/*
 * Find an interface using a specific address family
 */
struct ifaddr *
ifa_ifwithaf(af)
	register int af;
{
	register struct ifnet *ifp;
	register struct ifaddr *ifa;

	for (ifp = ifnet; ifp; ifp = ifp->if_next)
	    for (ifa = ifp->if_addrlist; ifa; ifa = ifa->ifa_next)
		if (ifa->ifa_addr->sa_family == af)
			return (ifa);
	return ((struct ifaddr *)0);
}

/*
 * Find an interface address specific to an interface best matching
 * a given address.
 */
struct ifaddr *
ifaof_ifpforaddr(addr, ifp)
	struct sockaddr *addr;
	register struct ifnet *ifp;
{
	register struct ifaddr *ifa;
	register char *cp, *cp2, *cp3;
	register char *cplim;
	struct ifaddr *ifa_maybe = 0;
	u_int af = addr->sa_family;

	if (af >= AF_MAX)
		return (0);
	for (ifa = ifp->if_addrlist; ifa; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr->sa_family != af)
			continue;
		ifa_maybe = ifa;
		if (ifa->ifa_netmask == 0) {
			if (equal(addr, ifa->ifa_addr) ||
			    (ifa->ifa_dstaddr && equal(addr, ifa->ifa_dstaddr)))
				return (ifa);
			continue;
		}
		cp = addr->sa_data;
		cp2 = ifa->ifa_addr->sa_data;
		cp3 = ifa->ifa_netmask->sa_data;
		cplim = ifa->ifa_netmask->sa_len + (char *)ifa->ifa_netmask;
		for (; cp3 < cplim; cp3++)
			if ((*cp++ ^ *cp2++) & *cp3)
				break;
		if (cp3 == cplim)
			return (ifa);
	}
	return (ifa_maybe);
}

/* 
 * Find an interface address for a specified destination or gateway
 */
struct ifaddr *
ifa_ifwithroute(flags, dst, gateway)
	int flags;
	struct sockaddr	*dst, *gateway;
{
	register struct ifaddr *ifa;
	if ((flags & RTF_GATEWAY) == 0) {
		/*
		 * If we are adding a route to an interface,
		 * and the interface is a pt to pt link
		 * we should search for the destination
		 * as our clue to the interface.  Otherwise
		 * we can use the local address.
		 */
		ifa = 0;
		if (flags & RTF_HOST) 
			ifa = ifa_ifwithdstaddr(dst);
		if (ifa == 0)
			ifa = ifa_ifwithaddr(gateway);
	} else {
		/*
		 * If we are adding a route to a remote net
		 * or host, the gateway may still be on the
		 * other end of a pt to pt link.
		 */
		ifa = ifa_ifwithdstaddr(gateway);
	}
	if (ifa == 0)
		ifa = ifa_ifwithnet(gateway);
	if (ifa == 0) {
		struct rtentry *rt = rtalloc1(dst, 0);
		if (rt == 0)
			return (0);
		rt->rt_refcnt--;
		if ((ifa = rt->rt_ifa) == 0)
			return (0);
	}
	if (ifa->ifa_addr->sa_family != dst->sa_family) {
		struct ifaddr *oifa = ifa;
		ifa = ifaof_ifpforaddr(dst, ifa->ifa_ifp);
		if (ifa == 0)
			ifa = oifa;
	}
	return (ifa);
}

/* free the interface address */

void
ifafree(ifa)
	register struct ifaddr *ifa;
{
	if (ifa == NULL)
		panic("ifafree");
	if (ifa->ifa_refcnt == 0)
		{
		FREE(ifa, MT_IFADDR);  
		}
	else
		ifa->ifa_refcnt--;
}

/*
 * Default action when installing a route with a Link Level gateway.
 * Lookup an appropriate real ifa to point to.
 * This should be moved to /sys/net/link.c eventually.
 */
void
link_rtrequest(cmd, rt, sa)
	int cmd;
	register struct rtentry *rt;
	struct sockaddr *sa;
{
	register struct ifaddr *ifa;
	struct sockaddr *dst;
	struct ifnet *ifp;

	if (cmd != RTM_ADD || ((ifa = rt->rt_ifa) == 0) ||
	    ((ifp = ifa->ifa_ifp) == 0) || ((dst = rt_key(rt)) == 0))
		return;
	if ((ifa = ifaof_ifpforaddr(dst, ifp))) {
		IFAFREE(rt->rt_ifa);
		rt->rt_ifa = ifa;
		ifa->ifa_refcnt++;
		if (ifa->ifa_rtrequest && ifa->ifa_rtrequest != link_rtrequest)
			ifa->ifa_rtrequest(cmd, rt, sa);
	}
}

/*
 * Mark an interface down and notify protocols of
 * the transition.
 * NOTE: must be called at splnet or eqivalent.
 */
void
if_down(ifp)
	register struct ifnet *ifp;
{
	register struct ifaddr *ifa;

	ifp->if_flags &= ~IFF_UP;
	for (ifa = ifp->if_addrlist; ifa; ifa = ifa->ifa_next)
		pfctlinput(PRC_IFDOWN, ifa->ifa_addr);
	if_qflush(&ifp->if_snd);
        if (_rtIfaceMsgHook) 
            (*_rtIfaceMsgHook) (ifp);
}

/*
 * Mark an interface up and notify protocols of
 * the transition.
 * NOTE: must be called at splnet or equivalent.
 */
void
if_up(ifp)
	register struct ifnet *ifp;
{

	ifp->if_flags |= IFF_UP;
#ifdef notyet
        {
	register struct ifaddr *ifa;
	/* this has no effect on IP, and will kill all ISO connections XXX */
	for (ifa = ifp->if_addrlist; ifa; ifa = ifa->ifa_next)
		pfctlinput(PRC_IFUP, ifa->ifa_addr);
        }
#endif
        if (_rtIfaceMsgHook) 
            (*_rtIfaceMsgHook) (ifp);
}

/*
 * Flush an interface queue.
 */
void
if_qflush(ifq)
	register struct ifqueue *ifq;
{
	register struct mbuf *m, *n;

	n = ifq->ifq_head;
	while ((m = n)) {
		n = m->m_act;
		m_freem(m);
	}
	ifq->ifq_head = 0;
	ifq->ifq_tail = 0;
	ifq->ifq_len = 0;
}

/*
 * Handle interface watchdog timer routines.  Called
 * from softclock, we decrement timers (if set) and
 * call the appropriate interface routine on expiration.
 */
void if_slowtimo (void)
    {
    register struct ifnet *ifp;

    for (ifp = ifnet; ifp; ifp = ifp->if_next)
	{
	if (ifp->if_timer == 0 || --ifp->if_timer)
	    continue;
	if (ifp->if_watchdog)
	    (*ifp->if_watchdog)(ifp->if_unit);
	}

    /* XXX using watchdogs good idea? calculate clock rate once only -gae */
    wdStart (ifslowtimoWd, sysClkRateGet()/IFNET_SLOWHZ,
	    (FUNCPTR) netJobAdd, (int) if_slowtimo);
    }

/* ifunit () has been moved to ifLib.c */

/*
 * Interface ioctls.
 */
int
ifioctl(so, cmd, data)
	struct socket *so;
	u_long cmd;
	caddr_t data;
{
	register struct ifnet *ifp;
	register struct ifreq *ifr;

	switch (cmd) {

	case SIOCGIFCONF:
	case OSIOCGIFCONF:
		return (ifconf(cmd, data));
#ifdef	INET
	case SIOCSARP:
	case SIOCDARP:
		/* FALL THROUGH */
	case SIOCGARP:
		return (arpioctl(cmd, data));
#endif	/* INET */
	}
	ifr = (struct ifreq *)data;
	ifp = ifunit(ifr->ifr_name);
	if (ifp == 0)
		return (ENXIO);
	switch (cmd) {

	case SIOCGIFFLAGS:
		ifr->ifr_flags = ifp->if_flags;
		break;

	case SIOCGIFMETRIC:
		ifr->ifr_metric = ifp->if_metric;
		break;

	case SIOCSIFFLAGS:
		if (ifp->if_flags & IFF_UP && (ifr->ifr_flags & IFF_UP) == 0) {
			int s = splimp();
			if_down(ifp);
			splx(s);
                        m2SetIfLastChange(ifp->if_name, ifp->if_unit,
			                  ifp->if_flags);
		}
		if (ifr->ifr_flags & IFF_UP && (ifp->if_flags & IFF_UP) == 0) {
			int s = splimp();
			if_up(ifp);
			splx(s);
                        m2SetIfLastChange(ifp->if_name, ifp->if_unit,
			                  ifp->if_flags);
		}
		ifp->if_flags = (ifp->if_flags & IFF_CANTCHANGE) |
			(ifr->ifr_flags &~ IFF_CANTCHANGE);
		ifr->ifr_flags = ifp->if_flags;
		if (ifp->if_ioctl)
			(void) (*ifp->if_ioctl)(ifp, cmd, data);
		break;

	case SIOCSIFMETRIC:
		ifp->if_metric = ifr->ifr_metric;
		break;

	case SIOCADDMULTI:
	case SIOCDELMULTI:
		if (ifp->if_ioctl == 0)
			return (EOPNOTSUPP);
		return ((*ifp->if_ioctl)(ifp, cmd, data));

	default:
		if (so->so_proto == 0)
			return (EOPNOTSUPP);
#ifndef COMPAT_43
		return ((*so->so_proto->pr_usrreq)(so, PRU_CONTROL,
			cmd, data, ifp));
#else
	    {
                int error = 0;
		int ocmd = cmd;

		switch (cmd) {

		case SIOCSIFDSTADDR:
		case SIOCSIFADDR:
		case SIOCSIFBRDADDR:
		case SIOCSIFNETMASK:
#if _BYTE_ORDER != _BIG_ENDIAN
			if (ifr->ifr_addr.sa_family == 0 &&
			    ifr->ifr_addr.sa_len < 16) {
				ifr->ifr_addr.sa_family = ifr->ifr_addr.sa_len;
				ifr->ifr_addr.sa_len = 16;
			}
#else
			if (ifr->ifr_addr.sa_len == 0)
				ifr->ifr_addr.sa_len = 16;
#endif
			break;

		case OSIOCGIFADDR:
			cmd = SIOCGIFADDR;
			break;

		case OSIOCGIFDSTADDR:
			cmd = SIOCGIFDSTADDR;
			break;

		case OSIOCGIFBRDADDR:
			cmd = SIOCGIFBRDADDR;
			break;

		case OSIOCGIFNETMASK:
			cmd = SIOCGIFNETMASK;
		}
		error =  ((*so->so_proto->pr_usrreq)(so, PRU_CONTROL,
							    cmd, data, ifp));
		switch (ocmd) {

		case OSIOCGIFADDR:
		case OSIOCGIFDSTADDR:
		case OSIOCGIFBRDADDR:
		case OSIOCGIFNETMASK:
			*(u_short *)&ifr->ifr_addr = ifr->ifr_addr.sa_family;
		}
		return (error);

	    }
#endif
	}
	return (0);
}

/*
 * Return interface configuration
 * of system.  List may be used
 * in later ioctl's (above) to get
 * other information.
 */
/*ARGSUSED*/
int
ifconf(cmd, data)
	int cmd;
	caddr_t data;
{
	register struct ifconf *ifc = (struct ifconf *)data;
	register struct ifnet *ifp = ifnet;
	register struct ifaddr *ifa;
	register char *cp, *ep;
	struct ifreq ifr, *ifrp;
	int space = ifc->ifc_len, error = 0;

	ifrp = ifc->ifc_req;
	ep = ifr.ifr_name + sizeof (ifr.ifr_name) - 2;
	for (; space > sizeof (ifr) && ifp; ifp = ifp->if_next) {
		strncpy(ifr.ifr_name, ifp->if_name, sizeof(ifr.ifr_name) - 2);
		for (cp = ifr.ifr_name; cp < ep && *cp; cp++)
			continue;
		*cp++ = '0' + ifp->if_unit; *cp = '\0';
		if ((ifa = ifp->if_addrlist) == 0) {
			bzero((caddr_t)&ifr.ifr_addr, sizeof(ifr.ifr_addr));
			error = copyout((caddr_t)&ifr, (caddr_t)ifrp,
			    sizeof(ifr));
			if (error)
				break;
			space -= sizeof (ifr), ifrp++;
		} else 
		    for ( ; space > sizeof (ifr) && ifa; ifa = ifa->ifa_next) {
			register struct sockaddr *sa = ifa->ifa_addr;
#ifdef COMPAT_43
			if (cmd == OSIOCGIFCONF) {
				struct osockaddr *osa =
					 (struct osockaddr *)&ifr.ifr_addr;
				ifr.ifr_addr = *sa;
				osa->sa_family = sa->sa_family;
				error = copyout((caddr_t)&ifr, (caddr_t)ifrp,
						sizeof (ifr));
				ifrp++;
			} else
#endif
			if (sa->sa_len <= sizeof(*sa)) {
				ifr.ifr_addr = *sa;
				error = copyout((caddr_t)&ifr, (caddr_t)ifrp,
						sizeof (ifr));
				ifrp++;
			} else {
				space -= sa->sa_len - sizeof(*sa);
				if (space < sizeof (ifr))
					break;
				error = copyout((caddr_t)&ifr, (caddr_t)ifrp,
						sizeof (ifr.ifr_name));
				if (error == 0)
				    error = copyout((caddr_t)sa,
				      (caddr_t)&ifrp->ifr_addr, sa->sa_len);
				ifrp = (struct ifreq *)
					(sa->sa_len + (caddr_t)&ifrp->ifr_addr);
			}
			if (error)
				break;
			space -= sizeof (ifr);
		}
	}
	ifc->ifc_len -= space;
	return (error);
}

/*
 * Set/clear promiscuous mode on interface ifp based on the truth value
 * of pswitch.  The calls are reference counted so that only the first
 * "on" request actually has an effect, as does the final "off" request.
 * Results are undefined if the "off" and "on" requests are not matched.
 */
int
ifpromisc(ifp, pswitch)
	struct ifnet *ifp;
	int pswitch;
{
	struct ifreq ifr;
	/*
	 * If the device is not configured up, we cannot put it in
	 * promiscuous mode.
	 */
	if ((ifp->if_flags & IFF_UP) == 0)
		return (ENETDOWN);

	if (pswitch) {
		if (ifp->if_pcount++ != 0)
			return (0);
		ifp->if_flags |= IFF_PROMISC;
	} else {
		if (--ifp->if_pcount > 0)
			return (0);
		ifp->if_flags &= ~IFF_PROMISC;
	}
	ifr.ifr_flags = ifp->if_flags;
	return ((*ifp->if_ioctl)(ifp, SIOCSIFFLAGS, (caddr_t)&ifr));
}

static char *
sprint_d(n, buf, buflen)
	u_int n;
	char *buf;
	int buflen;
{
	register char *cp = buf + buflen - 1;

	*cp = 0;
	do {
		cp--;
		*cp = "0123456789"[n % 10];
		n /= 10;
	} while (n != 0);
	return (cp);
}
