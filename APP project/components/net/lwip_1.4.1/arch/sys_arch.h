/***************************************************************************************************
*                    (c) Copyright 1992-2015 Syncretize technologies co.,ltd.
*                                       All Rights Reserved
*
*\File          sys_arch.h
*\Description   
*\Note          
*\Log           2015.04.03    Ver 1.0    张波
*               创建文件。
***************************************************************************************************/
#ifndef _SYS_ARCH_H
#define _SYS_ARCH_H

#include "cc.h"
#include "rtthread.h"

#ifndef BYTE_ORDER
#define BYTE_ORDER  LITTLE_ENDIAN
#endif

#define SYS_LWIP_SEM_NAME   "lw_sem"
#define SYS_LWIP_MBOX_NAME  "lw_mb"
#define SYS_LWIP_MUTEX_NAME "lw_mut"

typedef u32_t sys_prot_t;

typedef rt_sem_t        sys_sem_t;
typedef rt_mailbox_t    sys_mbox_t;
typedef rt_thread_t     sys_thread_t;
typedef rt_mutex_t      sys_mutex_t;

void lwip_sys_init(void);
u8_t netif_up_state(void);
#endif /*_SYS_ARCH_H*/

