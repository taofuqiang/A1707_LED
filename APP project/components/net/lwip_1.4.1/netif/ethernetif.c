/**
 * @file
 * Ethernet Interface Skeleton
 *
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/*
 * This file is a skeleton for developing Ethernet network interface
 * drivers for lwIP. Add code to the low_level functions and do a
 * search-and-replace for the word "ethernetif" to replace it with
 * something that better describes your network interface.
 */

#include "lwip/opt.h"
#include "rtconfig.h"
#include "rtthread.h"
#include "drv_f1_eth.h"

#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include <lwip/stats.h>
#include "netif/etharp.h"
#include "netif/ethernetif.h"

/* Define those to better describe your network interface. */
#define IFNAME0 'e'
#define IFNAME1 '0'

/**
 * Helper struct to hold private data used to operate your ethernet interface.
 * Keeping the ethernet address of the MAC in this struct is not necessary
 * as it is already kept in the struct netif.
 * But this is only an example, anyway...
 */
struct ethernetif {
  struct eth_addr *ethaddr;
  /*Add whatever per-interface state that is needed here.*/
  rt_sem_t eth_rx_sem;//网卡接收信号量
};

static struct ethernetif *stm32_eth_if;

void eth_rx_sem_release(void)
{
    rt_sem_release(stm32_eth_if->eth_rx_sem);
}
/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static void
low_level_init(struct netif *netif)
{
  //struct ethernetif *ethernetif = netif->state;
  
  /* set MAC hardware address length */
  netif->hwaddr_len = ETHARP_HWADDR_LEN;

  /* set MAC hardware address */
  rt_memcpy(netif->hwaddr, stm32_eth_mac, netif->hwaddr_len);

  /* maximum transfer unit */
  netif->mtu = ETHERNET_MTU;
  
  /* device capabilities */
  /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;

#if LWIP_DHCP
  /* DHCP support */
  netif->flags |= NETIF_FLAG_DHCP;
#endif

#if LWIP_IGMP
  /* IGMP support */
  netif->flags |= NETIF_FLAG_IGMP;
#endif
}

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become availale since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */

static err_t
low_level_output(struct netif *netif, struct pbuf *p)
{
    rt_err_t result;

    result = rt_stm32_eth_tx(p);
    if (result != RT_EOK)
    {
        rt_kprintf("eth tx is err:%d!\n", (int)result);
    }

    LINK_STATS_INC(link.xmit);

    return ERR_OK;
}

/**
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
static void
ethernetif_input(void* para)
{
  struct netif* netif = (struct netif*)para;
  struct ethernetif *eth_netif;
  struct pbuf *p;

  eth_netif = netif->state;

  while (1)
  {
     rt_sem_take(eth_netif->eth_rx_sem, RT_WAITING_FOREVER);
     /* receive all of buffer */
     while (1)
     {
         p = rt_stm32_eth_rx();
         if (p != RT_NULL)
         {
             /* notify to upper layer */
             if(netif->input(p, netif) != ERR_OK )
             {
                 LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: Input error\n"));     
                 pbuf_free(p);
                 p = RT_NULL;
             }          
         }
         else
         {
             break;
         }
     }
  }
}
/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t
ethernetif_init(struct netif *netif)
{
  rt_thread_t tid;

  LWIP_ASSERT("netif != NULL", (netif != NULL));
    
  stm32_eth_if = mem_malloc(sizeof(struct ethernetif));
  if (stm32_eth_if == NULL) {
    LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_init: out of memory\n"));
    return ERR_MEM;
  }

#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

  /*
   * Initialize the snmp variables and counters inside the struct netif.
   * The last argument should be replaced with your link speed, in units
   * of bits per second.
   */
  //NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, LINK_SPEED_OF_YOUR_NETIF_IN_BPS);

  netif->state = stm32_eth_if;
  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;
  /* We directly use etharp_output() here to save a function call.
   * You can instead declare your own function an call etharp_output()
   * from it if you have to do some checks before sending (e.g. if link
   * is available...) */
  netif->output = etharp_output;
  netif->linkoutput = low_level_output;
  
  stm32_eth_if->eth_rx_sem = rt_sem_create("erx_sem", 0, RT_IPC_FLAG_FIFO);

  stm32_eth_if->ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);
  
  tid = rt_thread_create("eth_RxTh", ethernetif_input, netif, RT_LWIP_ETHTHREAD_STACKSIZE, RT_LWIP_ETHTHREAD_PRIORITY, 10);
  if (tid != RT_NULL)
  {
      rt_thread_startup(tid);
  }
   //最后初始化网卡,MAC的生成在此函数中，所以必须在low_level_init()之前初始化
   stm32_eth_init(netif);

   /* initialize the hardware */
   low_level_init(netif);

   return ERR_OK;
}
 
#ifdef RT_USING_FINSH
#include <finsh.h>
void set_if(char* ip_addr, char* gw_addr, char* nm_addr)
{
    struct ip_addr *ip;
    struct ip_addr addr;

    ip = (struct ip_addr *)&addr;

    /* set ip address */
    if ((ip_addr != RT_NULL) && ipaddr_aton(ip_addr, &addr))
    {
        netif_set_ipaddr(netif_default, ip);
    }

    /* set gateway address */
    if ((gw_addr != RT_NULL) && ipaddr_aton(gw_addr, &addr))
    {
        netif_set_gw(netif_default, ip);
    }

    /* set netmask address */
    if ((nm_addr != RT_NULL) && ipaddr_aton(nm_addr, &addr))
    {
        netif_set_netmask(netif_default, ip);
    }
}
FINSH_FUNCTION_EXPORT(set_if, set network interface address);
#if LWIP_DNS
#include <lwip/dns.h>
void set_dns(char* dns_server)
{
    struct ip_addr addr;

    if ((dns_server != RT_NULL) && ipaddr_aton(dns_server, &addr))
    {
        dns_setserver(0, &addr);
    }
}
FINSH_FUNCTION_EXPORT(set_dns, set DNS server address);
#endif

void list_if(void)
{
    rt_ubase_t index;
    struct netif * netif;

    rt_enter_critical();

    netif = netif_list;

    while( netif != RT_NULL )
    {
        rt_kprintf("network interface: %c%c%s\n",
            netif->name[0],
            netif->name[1],
            (netif == netif_default)?" (Default)":"");
        rt_kprintf("MTU: %d\n", netif->mtu);
        rt_kprintf("MAC: ");
        for (index = 0; index < netif->hwaddr_len; index ++)
            rt_kprintf("%02X ", netif->hwaddr[index]);
        rt_kprintf("\nFLAGS:");
        if (netif->flags & NETIF_FLAG_UP) rt_kprintf(" UP");
        else rt_kprintf(" DOWN");
        if (netif->flags & NETIF_FLAG_DHCP) rt_kprintf(" | DHCP");
        if (netif->flags & NETIF_FLAG_POINTTOPOINT) rt_kprintf(" | PPP");
        if (netif->flags & NETIF_FLAG_ETHARP) rt_kprintf(" | ETHARP");
        if (netif->flags & NETIF_FLAG_IGMP) rt_kprintf(" | IGMP");
        rt_kprintf("\n");
        rt_kprintf("ip address: %s\n", ipaddr_ntoa(&(netif->ip_addr)));
        rt_kprintf("gw address: %s\n", ipaddr_ntoa(&(netif->gw)));
        rt_kprintf("net mask  : %s\n", ipaddr_ntoa(&(netif->netmask)));
        rt_kprintf("\r\n");

        netif = netif->next;
    }

#if LWIP_DNS
    {
        struct ip_addr ip_addr;

        for(index=0; index<DNS_MAX_SERVERS; index++)
        {
            ip_addr = dns_getserver(index);
            rt_kprintf("dns server #%d: %s\n", index, ipaddr_ntoa(&(ip_addr)));
        }
    }
#endif /**< #if LWIP_DNS */

    rt_exit_critical();
}
FINSH_FUNCTION_EXPORT(list_if, list network interface information);

#if LWIP_TCP
#include <lwip/tcp.h>
#include <lwip/tcp_impl.h>

void list_tcps(void)
{
    rt_uint32_t num = 0;
    struct tcp_pcb *pcb;
    char local_ip_str[16];
    char remote_ip_str[16];

    extern struct tcp_pcb *tcp_active_pcbs;
    extern union tcp_listen_pcbs_t tcp_listen_pcbs;
    extern struct tcp_pcb *tcp_tw_pcbs;
    extern const char *tcp_state_str[];

    rt_enter_critical();
    rt_kprintf("Active PCB states:\n");
    for(pcb = tcp_active_pcbs; pcb != NULL; pcb = pcb->next)
    {
        strcpy(local_ip_str, ipaddr_ntoa(&(pcb->local_ip)));
        strcpy(remote_ip_str, ipaddr_ntoa(&(pcb->remote_ip)));

        rt_kprintf("#%d %s:%d <==> %s:%d snd_nxt 0x%08X rcv_nxt 0x%08X ",
            num++,
            local_ip_str,
            pcb->local_port,
            remote_ip_str,
            pcb->remote_port,
            pcb->snd_nxt,
            pcb->rcv_nxt);
        rt_kprintf("state: %s\n", tcp_state_str[pcb->state]);
    }

    rt_kprintf("Listen PCB states:\n");
    num = 0;
    for(pcb = (struct tcp_pcb *)tcp_listen_pcbs.pcbs; pcb != NULL; pcb = pcb->next)
    {
        rt_kprintf("#%d local port %d ", num++, pcb->local_port);
        rt_kprintf("state: %s\n", tcp_state_str[pcb->state]);
    }

    rt_kprintf("TIME-WAIT PCB states:\n");
    num = 0;
    for(pcb = tcp_tw_pcbs; pcb != NULL; pcb = pcb->next)
    {
        strcpy(local_ip_str, ipaddr_ntoa(&(pcb->local_ip)));
        strcpy(remote_ip_str, ipaddr_ntoa(&(pcb->remote_ip)));

        rt_kprintf("#%d %s:%d <==> %s:%d snd_nxt 0x%08X rcv_nxt 0x%08X ",
            num++,
            local_ip_str,
            pcb->local_port,
            remote_ip_str,
            pcb->remote_port,
            pcb->snd_nxt,
            pcb->rcv_nxt);
        rt_kprintf("state: %s\n", tcp_state_str[pcb->state]);
    }
    rt_exit_critical();
}
FINSH_FUNCTION_EXPORT(list_tcps, list all of tcp connections);
#endif
#endif
