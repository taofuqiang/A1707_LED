/*-
 * Copyright (c) 2011-2012 George V. Neville-Neil,
 *                         Steven Kreuzer, 
 *                         Martin Burnicki, 
 *                         Jan Breuer,
 *                         Gael Mace, 
 *                         Alexandre Van Kempen,
 *                         Inaqui Delgado,
 *                         Rick Ratzel,
 *                         National Instruments.
 * Copyright (c) 2009-2010 George V. Neville-Neil, 
 *                         Steven Kreuzer, 
 *                         Martin Burnicki, 
 *                         Jan Breuer,
 *                         Gael Mace, 
 *                         Alexandre Van Kempen
 *
 * Copyright (c) 2005-2008 Kendall Correll, Aidan Williams
 *
 * All Rights Reserved
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file   net.c
 * @date   Tue Jul 20 16:17:49 2010
 *
 * @brief  Functions to interact with the network sockets and NIC driver.
 *
 *
 */

#include "../ptpd.h"

/* choose kernel-level nanoseconds or microseconds resolution on the client-side */
//#if !defined(SO_TIMESTAMPNS) && !defined(SO_TIMESTAMP) && !defined(SO_BINTIME)
//#error kernel-level timestamps not detected
//#endif

/**
 * shutdown the IPv4 multicast for specific address
 *
 * @param netPath
 * @param multicastAddr
 * 
 * @return TRUE if successful
 */

static Boolean
netShutdownMulticastIPv4(NetPath * netPath, Integer32 multicastAddr) {
    /* Close General Multicast */
    ip_addr_t if_addr;
    ip_addr_t multi_addr;

    ip4_addr_set_u32(&if_addr, netPath->interfaceAddr.s_addr);
    ip4_addr_set_u32(&multi_addr, multicastAddr);


    if (netPath->eventConn != NULL && netPath->generalConn != NULL) {
        if ((ERR_OK != netconn_join_leave_group(netPath->eventConn, &multi_addr, &if_addr, NETCONN_LEAVE))
                || (ERR_OK != netconn_join_leave_group(netPath->generalConn, &multi_addr, &if_addr, NETCONN_LEAVE))) {
            ERROR("netShutdownMulticastIPv4: failed");
            //return FALSE;
        }
    }

    return TRUE;
}

/**
 * shutdown the multicast (both General and Peer)
 *
 * @param netPath 
 * 
 * @return TRUE if successful
 */
static Boolean
netShutdownMulticast(NetPath * netPath, RunTimeOpts * rtOpts, PtpClock * ptpClock) {
    (void) ptpClock;

        /* Close General Multicast */
    netShutdownMulticastIPv4(netPath, netPath->multicastAddr);
    netPath->multicastAddr = 0;

    if (rtOpts->delayMechanism == P2P) {
        /* Close Peer Multicast */
        netShutdownMulticastIPv4(netPath, netPath->peerMulticastAddr);
        netPath->peerMulticastAddr = 0;
    }

    return TRUE;
}

/* shut down the UDP stuff */
Boolean
netShutdown(NetPath * netPath, RunTimeOpts * rtOpts, PtpClock * ptpClock) {
    netShutdownMulticast(netPath, rtOpts, ptpClock);

#ifdef PTPD_UNICAST_MODE
    netPath->unicastAddr = 0;
#endif
    /* Close sockets */
    if (netPath->eventConn != NULL) {
        netconn_delete(netPath->eventConn);
    }
    netPath->eventConn = NULL;

    if (netPath->generalConn != NULL) {
        netconn_delete(netPath->generalConn);
    }
    netPath->generalConn = NULL;

    return TRUE;
}

Boolean
chooseMcastGroup(RunTimeOpts * rtOpts, ip_addr_t *netAddr) {

    char *addrStr;

#ifdef PTPD_EXPERIMENTAL
    switch (rtOpts->mcast_group_Number) {
        case 0:
            addrStr = DEFAULT_PTP_DOMAIN_ADDRESS;
            break;

        case 1:
            addrStr = ALTERNATE_PTP_DOMAIN1_ADDRESS;
            break;
        case 2:
            addrStr = ALTERNATE_PTP_DOMAIN2_ADDRESS;
            break;
        case 3:
            addrStr = ALTERNATE_PTP_DOMAIN3_ADDRESS;
            break;

        default:
            ERROR("Unk group %d\n", rtOpts->mcast_group_Number);
            exit(3);
            break;
    }
#else
    (void) rtOpts;
    addrStr = DEFAULT_PTP_DOMAIN_ADDRESS;
#endif

    if (!ipaddr_aton(addrStr, netAddr)) {
        ERROR("failed to encode multicast address: %s\n", addrStr);
        return FALSE;
    }
    return TRUE;
}

/*Test if network layer is OK for PTP*/
UInteger8
lookupCommunicationTechnology(UInteger8 communicationTechnology) {
#if defined(linux)
    switch (communicationTechnology) {
        case ARPHRD_ETHER:
        case ARPHRD_EETHER:
        case ARPHRD_IEEE802:
            return PTP_ETHER;

        default:
            break;
    }
#else  /* defined(linux) */
    (void) communicationTechnology;
#endif

    return PTP_DEFAULT;
}

/* Find the local network interface */
UInteger32
findIface(Octet * ifaceName, UInteger8 * communicationTechnology,
        Octet * uuid, NetPath * netPath) {
    struct netif * iface;
    (void) communicationTechnology;
    (void) netPath;
    iface = netif_default;
    memcpy(uuid, iface->hwaddr, iface->hwaddr_len);
    memcpy(ifaceName, iface->name, sizeof (iface->name));
    return iface->ip_addr.addr;
}

/**
 * Init the multcast for specific IPv4 address
 * 
 * @param netPath 
 * @param multicastAddr 
 * 
 * @return TRUE if successful
 */
static Boolean
netInitMulticastIPv4(NetPath * netPath, Integer32 multicastAddr) {
    ip_addr_t if_addr;
    ip_addr_t multi_addr;

    /* multicast send only on specified interface */
    ip4_addr_set_u32(&if_addr, netPath->interfaceAddr.s_addr);
    ip4_addr_set_u32(&multi_addr, multicastAddr);

    /* IP_MULTICAST_IF */
    ip_addr_copy(netPath->eventConn->pcb.udp->multicast_ip, if_addr);
    ip_addr_copy(netPath->generalConn->pcb.udp->multicast_ip, if_addr);

    /* IP_ADD_MEMBERSHIP */
    /* join multicast group (for receiving) on specified interface */
    if ((ERR_OK != netconn_join_leave_group(netPath->eventConn, &multi_addr, &if_addr, NETCONN_JOIN))
            || (ERR_OK != netconn_join_leave_group(netPath->generalConn, &multi_addr, &if_addr, NETCONN_JOIN))) {
        PERROR("failed to join the multi-cast group");
        return FALSE;
    }
    return TRUE;
}

/**
 * Init the multcast (both General and Peer)
 * 
 * @param netPath 
 * @param rtOpts 
 * 
 * @return TRUE if successful
 */
Boolean
netInitMulticast(NetPath * netPath, RunTimeOpts * rtOpts, PtpClock * ptpClock) {
    ip_addr_t netAddr;
    char addrStr[NET_ADDRESS_LENGTH];

    (void) ptpClock;

    /* Init General multicast IP address */
    if (!chooseMcastGroup(rtOpts, &netAddr)) {
        return FALSE;
    }
    netPath->multicastAddr = netAddr.addr;
    if (!netInitMulticastIPv4(netPath, netPath->multicastAddr)) {
        return FALSE;
    }
    /* End of General multicast Ip address init */


    if (rtOpts->delayMechanism == P2P) {
        /* Init Peer multicast IP address */
        memcpy(addrStr, PEER_PTP_DOMAIN_ADDRESS, NET_ADDRESS_LENGTH);

        if (!ipaddr_aton(addrStr, &netAddr)) {
            ERROR("failed to encode multi-cast address: %s\n", addrStr);
            return FALSE;
        }
        netPath->peerMulticastAddr = netAddr.addr;
        if (!netInitMulticastIPv4(netPath, netPath->peerMulticastAddr)) {
            return FALSE;
        }
        /* End of Peer multicast Ip address init */
    }

    return TRUE;
}

/**
 * Initialize timestamping of packets
 *
 * @param netPath 
 * 
 * @return TRUE if successful
 */
Boolean
netInitTimestamping(NetPath * netPath) {
    Boolean result = TRUE;
    (void) netPath;

    // Already initialised
    return result;
}

static void
netCallbackE(struct netconn *conn, enum netconn_evt evt, u16_t len) {
    if (evt == NETCONN_EVT_RCVPLUS) {
        DBGV("==>netCallbackE: (%d.%09d) %d\r\n", conn->timestamp.tv_sec, conn->timestamp.tv_nsec, len);
        (void) conn;
        (void) len;
        signalAction(SELECT_EVENT);
    }
}

static void
netCallbackG(struct netconn *conn, enum netconn_evt evt, u16_t len) {
    if (evt == NETCONN_EVT_RCVPLUS) {
        DBGV("==>netCallbackG: %d\r\n", len);
        (void) conn;
        (void) len;
        signalAction(SELECT_GENEAL);
    }
}

/**
 * start all of the UDP stuff 
 * must specify 'subdomainName', and optionally 'ifaceName', 
 * if not then pass ifaceName == "" 
 * on socket options, see the 'socket(7)' and 'ip' man pages 
 *
 * @param netPath 
 * @param rtOpts 
 * @param ptpClock 
 * 
 * @return TRUE if successful
 */
Boolean
netInit(NetPath * netPath, RunTimeOpts * rtOpts, PtpClock * ptpClock) {
    ip_addr_t interfaceAddr;

    DBG("netInit\n");
    netPath->eventConn = netconn_new_with_callback(NETCONN_UDP, netCallbackE);
    netPath->generalConn = netconn_new_with_callback(NETCONN_UDP, netCallbackG);
    /* open sockets */
    if (!(netPath->eventConn) || !(netPath->generalConn)) 
    {
        PERROR("failed to initalize sockets");
        return FALSE;
    }

    netconn_set_recvtimeout(netPath->eventConn, 1);
    netconn_set_recvtimeout(netPath->generalConn, 1);

    if (netPath->sendBuf == NULL) {
        netPath->sendBuf = netbuf_new();
    }

    /* find a network interface */
    interfaceAddr.addr =
            findIface(rtOpts->ifaceName,
            &ptpClock->port_communication_technology,
            ptpClock->port_uuid_field, netPath);
    if (!(interfaceAddr.addr))
        return FALSE;

    /* save interface address for IGMP refresh */
    netPath->interfaceAddr.s_addr = ip4_addr_get_u32(&interfaceAddr);

    DBG("Local IP address used: %s \n", ipaddr_ntoa(&interfaceAddr));

    /* bind sockets */
    /*
     * need INADDR_ANY to allow receipt of multi-cast and uni-cast
     * messages
     */
    if (ERR_OK != netconn_bind(netPath->eventConn, IP_ADDR_ANY, PTP_EVENT_PORT)) {
        PERROR("failed to bind event socket");
        return FALSE;
    }

    if (ERR_OK != netconn_bind(netPath->generalConn, IP_ADDR_ANY, PTP_GENERAL_PORT)) {
        PERROR("failed to bind general socket");
        return FALSE;
    }

#ifdef PTPD_UNICAST_MODE
    /* send a uni-cast address if specified (useful for testing) */
    if (rtOpts->unicastAddress[0]) {
        ip_addr_t netAddr;
#if LWIP_DNS
        //netconn_gethostbyname
        if (ERR_OK == netconn_gethostbyname(rtOpts->unicastAddress, &netAddr)) {
            netPath->unicastAddr = netAddr.addr;
        } else
#endif                    
        {
            /* Maybe it's a dotted quad. */
            if (!ipaddr_aton(rtOpts->unicastAddress, &netAddr)) {
                ERROR("failed to encode uni-cast address: %s\n",
                        rtOpts->unicastAddress);
                return FALSE;
                netPath->unicastAddr = netAddr.addr;
            }
        }
    } else {
        netPath->unicastAddr = 0;
    }
#else       
#endif

    /* init UDP Multicast on both Default and Pear addresses */
    if (!netInitMulticast(netPath, rtOpts, ptpClock)) {
        return FALSE;
    }

    /* set socket time-to-live to 1 */
    netPath->eventConn->pcb.udp->ttl = rtOpts->ttl;
    netPath->generalConn->pcb.udp->ttl = rtOpts->ttl;

    /* make timestamps available through recvmsg() */
    if (!netInitTimestamping(netPath)) {
        ERROR("failed to enable receive time stamps\n");
        return FALSE;
    }

    return TRUE;
}

/*Check if data have been received*/
int
netSelect(TimeInternal * timeout, NetPath * netPath) {
    int ret;
    (void) timeout;
    (void) netPath;
    ret = waitSignal();

    return ret;
}

/** 
 * store received data from network to "buf" , get and store the
 * SO_TIMESTAMP value in "time" for an event message
 *
 * @note Should this function be merged with netRecvGeneral(), below?
 * Jan Breuer: I think that netRecvGeneral should be simplified. Timestamp returned by this
 * function is never used. According to this, netInitTimestamping can be also simplified
 * to initialize timestamping only on eventSock.
 *
 * @param buf 
 * @param time 
 * @param netPath 
 *
 * @return
 */

rt_int32_t
netRecvEvent(Octet * buf, TimeInternal * time, NetPath * netPath) {
    err_t ret;
    struct netbuf * nb;
    rt_int32_t data_len;
    uint16_t expected_len;

    ret = netconn_recv(netPath->eventConn, &nb);

    if (ret != ERR_OK) {
        ERROR("netRecvEvent: recv failed %d\n", ret);
        return 0;
    }

    expected_len = min(netbuf_len(nb), PACKET_SIZE);
    data_len = netbuf_copy(nb, buf, expected_len);
    LWIP_ASSERT("expected_len == data_len", expected_len == data_len);

#ifdef PTPD_EXPERIMENTAL
    netPath->lastRecvAddr = netbuf_fromaddr(nb)->addr;
#endif

    netbuf_delete(nb);

#ifdef LWIP_PTP
    time->seconds = netPath->eventConn->timestamp.tv_sec;
    time->nanoseconds = netPath->eventConn->timestamp.tv_nsec;

    DBG("netRecvEvent (%d.%09d),len:%d\r\n", time->seconds, time->nanoseconds, data_len);
#else
    LWIP_UNUSED_ARG(time);
#endif
    return data_len;
}

/** 
 * 
 * store received data from network to "buf" get and store the
 * SO_TIMESTAMP value in "time" for a general message
 * 
 * @param buf 
 * @param time 
 * @param netPath 
 * 
 * @return 
 */

rt_int32_t
netRecvGeneral(Octet * buf, NetPath * netPath) {
    err_t ret;
    struct netbuf * nb;
    rt_int32_t data_len;
    uint16_t expected_len;

    ret = netconn_recv(netPath->generalConn, &nb);

    if (ret != ERR_OK) {
        ERROR("netRecvGeneral: recv failed %d\n", ret);
        return 0;
    }

    expected_len = min(netbuf_len(nb), PACKET_SIZE);
    data_len = netbuf_copy(nb, buf, expected_len);
    LWIP_ASSERT("expected_len == data_len", expected_len == data_len);

#ifdef PTPD_EXPERIMENTAL
    netPath->lastRecvAddr = netbuf_fromaddr(nb)->addr;
#endif

    netbuf_delete(nb);

    DBG("netRecvGeneral,len:%d\r\n", data_len);

    return data_len;
}





//
// alt_dst: alternative destination.
//   if filled, send to this unicast dest;
//   if zero, do the normal operation (send to unicast with -u, or send to the multcast group)
//
///
/// TODO: merge these 2 functions into one
///

rt_int32_t
netSendEvent(Octet * buf, UInteger16 length, TimeInternal * tint, NetPath * netPath, Integer32 alt_dst) {
    err_t ret;
    ip_addr_t addr;

#ifdef PTPD_UNICAST_MODE
    if (netPath->unicastAddr) {
        addr.addr = netPath->unicastAddr;
    } else
#endif
        if (alt_dst) {
        addr.addr = alt_dst;
    } else {
        addr.addr = netPath->multicastAddr;
    }
    addr.addr = netPath->multicastAddr;


    netbuf_ref(netPath->sendBuf, buf, length);   
    ret = netconn_sendto(netPath->eventConn, netPath->sendBuf, &addr, PTP_EVENT_PORT);
    netbuf_free(netPath->sendBuf);

    if (ret != ERR_OK) {
        ERROR("netSendEvent: error %d\n", ret);
        return 0;
    }

#ifdef LWIP_PTP
    tint->seconds = netPath->eventConn->timestamp.tv_sec;
    tint->nanoseconds = netPath->eventConn->timestamp.tv_nsec;
#else
    LWIP_UNUSED_ARG(tint);
#endif

    return length;
}

rt_int32_t
netSendGeneral(Octet * buf, UInteger16 length, NetPath * netPath, Integer32 alt_dst) {
    rt_int32_t ret;
    ip_addr_t addr;

#ifdef PTPD_UNICAST_MODE
    if (netPath->unicastAddr) {
        addr.addr = netPath->unicastAddr;
    } else
#endif
        if (alt_dst) {
        addr.addr = alt_dst;
    } else {
        addr.addr = netPath->multicastAddr;
    }

    ret = netbuf_ref(netPath->sendBuf, buf, length);
    if (ret == ERR_OK) {
        ret = netconn_sendto(netPath->generalConn, netPath->sendBuf, &addr, PTP_GENERAL_PORT);
        netbuf_free(netPath->sendBuf);
    }
    if (ret != ERR_OK) {
        ERROR("netSendGeneral: error %d\n", ret);
    }
    return length;
}

rt_int32_t
netSendPeerGeneral(Octet * buf, UInteger16 length, NetPath * netPath) {

    rt_int32_t ret;
    ip_addr_t addr;

#ifdef PTPD_UNICAST_MODE
    if (netPath->unicastAddr) {
        addr.addr = netPath->unicastAddr;
    } else
#endif
    {
        addr.addr = netPath->peerMulticastAddr;
    }

    ret = netbuf_ref(netPath->sendBuf, buf, length);
    if (ret == ERR_OK) {
        ret = netconn_sendto(netPath->generalConn, netPath->sendBuf, &addr, PTP_GENERAL_PORT);
        netbuf_free(netPath->sendBuf);
    }
    if (ret != ERR_OK) {
        ERROR("netSendPeerGeneral: error %d\n", ret);
    }
    return length;
}

rt_int32_t
netSendPeerEvent(Octet * buf, UInteger16 length, TimeInternal * tint, NetPath * netPath) {
    rt_int32_t ret;
    ip_addr_t addr;

#ifdef PTPD_UNICAST_MODE
    if (netPath->unicastAddr) {
        addr.addr = netPath->unicastAddr;
    } else
#endif
    {
        addr.addr = netPath->peerMulticastAddr;
    }

    netbuf_ref(netPath->sendBuf, buf, length);
    ret = netconn_sendto(netPath->eventConn, netPath->sendBuf, &addr, PTP_EVENT_PORT);
    netbuf_free(netPath->sendBuf);

    if (ret != ERR_OK) {
        ERROR("netSendEvent: error %d\n", ret);
    }

#ifdef LWIP_PTP
    tint->seconds = netPath->eventConn->timestamp.tv_sec;
    tint->nanoseconds = netPath->eventConn->timestamp.tv_nsec;
#else
    LWIP_UNUSED_ARG(tint);
#endif

    return length;
}



/*
 * refresh IGMP on a timeout
 */

/*
 * @return TRUE if successful
 */
Boolean
netRefreshIGMP(NetPath * netPath, RunTimeOpts * rtOpts, PtpClock * ptpClock) {
     

    DBG("netRefreshIGMP\n");

    netShutdownMulticast(netPath, rtOpts, ptpClock);

    /* suspend process 200 milliseconds, to make sure the kernel sends the IGMP_leave properly */
    rt_thread_delay(20);

    if (!netInitMulticast(netPath, rtOpts, ptpClock)) {
        return FALSE;
    }

    INFO("Refreshed IGMP multicast memberships\n");
    return TRUE;
}
