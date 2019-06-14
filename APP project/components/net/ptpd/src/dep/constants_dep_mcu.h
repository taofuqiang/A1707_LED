
/* constants_dep_mcu.h */

#ifndef CONSTANTS_DEP_MCU_H
#define CONSTANTS_DEP_MCU_H

/**
 *\file
 * \brief Plateform-dependent constants definition for bare metal
 *
 */

/* platform dependent */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lwip/sockets.h"

#include "lwip/mem.h"
#include "lwip/udp.h"
#include "lwip/igmp.h"
#include "lwip/arch.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/api.h"
#include "netif/ethernetif.h"
#include "rtthread.h"
#include "stm32f10x.h"

#define	LOG_EMERG	0	/* system is unusable */
#define	LOG_ALERT	1	/* action must be taken immediately */
#define	LOG_CRIT	2	/* critical conditions */
#define	LOG_ERR		3	/* error conditions */
#define	LOG_WARNING	4	/* warning conditions */
#define	LOG_NOTICE	5	/* normal but significant condition */
#define	LOG_INFO	6	/* informational */


#define IF_NAMESIZE             2
#define INET_ADDRSTRLEN         16
#define MAXHOSTNAMELEN          256


#define IFACE_NAME_LENGTH         IF_NAMESIZE
#define NET_ADDRESS_LENGTH        INET_ADDRSTRLEN

#define IFCONF_LENGTH 10

#if BYTE_ORDER == LITTLE_ENDIAN
#define PTPD_LSBF
#elif BYTE_ORDER == BIG_ENDIAN
#define PTPD_MSBF
#endif

/*
 * The number of bytes in an ethernet (MAC) address.
 */
#define	ETHER_ADDR_LEN		6

/*
 * Structure of a 48-bit Ethernet address.
 */
struct	ether_addr 
{
	u8 octet[ETHER_ADDR_LEN];
};

//#define PTPD_UNICAST_MODE        1


#define SELECT_TIMER    1
#define SELECT_EVENT    2
#define SELECT_GENEAL   3


#define STA_INS         0x01
#define STA_DEL         0x02

#define SERIAL_PORT_PC  1


#endif /*CONSTANTS_DEP_H_*/

