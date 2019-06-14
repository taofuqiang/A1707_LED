/***************************************************************************************************
*                    (c) Copyright 1992-2015 Syncretize technologies co.,ltd.
*                                       All Rights Reserved
*
*\File          ethernetif.h
*\Description   
*\Note          
*\Log           2015.04.10    Ver 1.0    张波
*               创建文件。
***************************************************************************************************/
#ifndef _ETHERNETIF_H
#define _ETHERNETIF_H

#include "lwipopts.h"

#define ETHERNET_MTU		1500
 
void eth_rx_sem_release(void);
err_t ethernetif_init(struct netif *netif);

#endif /*_ETHERNETIF_H*/

