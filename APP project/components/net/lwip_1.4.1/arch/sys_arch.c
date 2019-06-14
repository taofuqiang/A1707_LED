/***************************************************************************************************
*                    (c) Copyright 1992-2015 Syncretize technologies co.,ltd.
*                                       All Rights Reserved
*
*\File          sys_arch.c
*\Description   
*\Note          
*\Log           2015.04.03    Ver 1.0    张波
*               创建文件。
***************************************************************************************************/
#include "sys_arch.h"
#include "lwip/sys.h"
#include "lwip/opt.h"
#include "lwip/stats.h"
#include "lwip/err.h"
#include "arch/sys_arch.h"
#include "lwip/debug.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "lwip/sio.h"
#include <lwip/init.h>
#include "netif/ethernetif.h"
#include "param.h"

struct netif* stm32_netif;

/***************************************************************************************************
*\Function      sys_sem_new
*\Description   创建一个信号量
*\Parameter     sem
*\Parameter     count
*\Return        err_t
*\Note          
*\Log           2015.04.03    Ver 1.0    张波
*               创建函数。
***************************************************************************************************/
err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{
    static u32_t sem_lwip_count = 0;
    char sem_name[RT_NAME_MAX];
    sys_sem_t sem_tmp = RT_NULL;

    RT_DEBUG_NOT_IN_INTERRUPT;

    rt_snprintf(sem_name, RT_NAME_MAX, "%s%d", SYS_LWIP_SEM_NAME, sem_lwip_count++);

    sem_tmp = rt_sem_create(sem_name, count, RT_IPC_FLAG_FIFO);
    if (sem_tmp != RT_NULL)
    {
        *sem = sem_tmp;
        return ERR_OK;
    }
    else
    {
        rt_kprintf("%s creat fail!\n", sem_name);
        return ERR_MEM;
    }
}
//删除信号量
void sys_sem_free(sys_sem_t* sem)
{
    RT_DEBUG_NOT_IN_INTERRUPT;
    rt_sem_delete(*sem);
}
//释放信号量
void sys_sem_signal(sys_sem_t* sem)
{
    rt_sem_release(*sem);
}
//等待信号量
u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
    rt_err_t ret;
    s32_t t;
    u32_t tick1, tick2, tick;
    
    RT_DEBUG_NOT_IN_INTERRUPT;

    tick1 = rt_tick_get();
    if (timeout == 0)
    {
        t = RT_WAITING_FOREVER;
    }
    else
    {
        t = timeout / (1000 / RT_TICK_PER_SECOND);
        t++;
    }
    ret = rt_sem_take(*sem, t);

    if (ret == -RT_ETIMEOUT)
    {
        return SYS_ARCH_TIMEOUT;
    }
    else if (ret == RT_EOK)
    {
        //正确返回，计算等待的时间
    }
    tick2 = rt_tick_get();

    tick = (tick2 + RT_TICK_MAX - tick1) % RT_TICK_MAX;
    tick = tick * (1000 / RT_TICK_PER_SECOND);//tick 转化为ms
 
    return tick;
}
#ifndef sys_sem_valid
/** Check if a semaphore is valid/allocated:
 *  return 1 for valid, 0 for invalid
 */
int sys_sem_valid(sys_sem_t *sem)
{
    if (*sem == RT_NULL)
    {
        return 0;
    }
    else
    {
        return 1;
    }
    //return (int)(*sem);
}
#endif

#ifndef sys_sem_set_invalid
/** Set a semaphore invalid so that sys_sem_valid returns 0
 */
void sys_sem_set_invalid(sys_sem_t *sem)
{
    *sem = RT_NULL;
}
#endif

//创建互斥锁
err_t sys_mutex_new(sys_mutex_t * mutex)
{
    static u32_t mutex_lwip_count = 0;
    char mutex_name[RT_NAME_MAX];
    sys_mutex_t mutex_tmp = RT_NULL;

    RT_DEBUG_NOT_IN_INTERRUPT;

    rt_snprintf(mutex_name, RT_NAME_MAX, "%s%d", SYS_LWIP_MUTEX_NAME, mutex_lwip_count++);

    mutex_tmp = rt_mutex_create(mutex_name, RT_IPC_FLAG_FIFO);
    if (mutex_tmp != RT_NULL)
    {
        *mutex = mutex_tmp;
        return ERR_OK;
    }
    else
    {
        rt_kprintf("%s creat fail!\n", mutex_name);
        return ERR_MEM;
    }
}
//锁定
void sys_mutex_lock(sys_mutex_t *mutex)
{
    RT_DEBUG_NOT_IN_INTERRUPT;

    rt_mutex_take(*mutex, RT_WAITING_FOREVER);
}
//解锁
void sys_mutex_unlock(sys_mutex_t *mutex)
{
    rt_mutex_release(*mutex);
}
//消灭
void sys_mutex_free(sys_mutex_t *mutex)
{
    RT_DEBUG_NOT_IN_INTERRUPT;
    rt_mutex_delete(*mutex);
}
#ifndef sys_mutex_valid
/** Check if a mutex is valid/allocated:
 *  return 1 for valid, 0 for invalid
 */
int sys_mutex_valid(sys_mutex_t *mutex)
{
    if (*mutex == RT_NULL)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}
#endif

#ifndef sys_mutex_set_invalid
/** Set a mutex invalid so that sys_mutex_valid returns 0
 */
void sys_mutex_set_invalid(sys_mutex_t *mutex)
{
    *mutex = RT_NULL;
}
#endif


/***************************************************************************************************
*\Function      sys_mbox_new
*\Description   消息邮箱创建
*\Parameter     mbox
*\Parameter     size
*\Return        err_t
*\Note          
*\Log           2015.04.03    Ver 1.0    张波
*               创建函数。
***************************************************************************************************/
err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
    static u32_t lwip_mbox_count = 0;
    char mbox_name[RT_NAME_MAX];
    sys_mbox_t mbox_tmp = RT_NULL;

    RT_DEBUG_NOT_IN_INTERRUPT;

    rt_snprintf(mbox_name, RT_NAME_MAX, "%s%d", SYS_LWIP_MBOX_NAME, lwip_mbox_count++);
    
    mbox_tmp = rt_mb_create(mbox_name, size, RT_IPC_FLAG_FIFO);
    if (mbox_tmp != RT_NULL)
    {
        *mbox = mbox_tmp;
        return ERR_OK;
    }
    else
    {
        rt_kprintf("%s creat fail!\n", mbox_name);
        return ERR_MEM;
    }
}
//消灭
void sys_mbox_free(sys_mbox_t *mbox)
{
    RT_DEBUG_NOT_IN_INTERRUPT;

    rt_mb_delete(*mbox);

}
/** Post a message to an mbox - may not fail
 * -> blocks if full, only used from tasks not from ISR
 * @param mbox mbox to posts the message
 * @param msg message to post (ATTENTION: can be NULL)
 */
void sys_mbox_post(sys_mbox_t *mbox, void *msg)
{
    RT_DEBUG_NOT_IN_INTERRUPT;

    rt_mb_send_wait(*mbox, (rt_uint32_t)msg, RT_WAITING_FOREVER);
}
err_t sys_mbox_trypost(sys_mbox_t *mbox, void* msg)
{
    if (rt_mb_send(*mbox, (rt_uint32_t)msg) == RT_EOK)
    {
        return ERR_OK;
    }
    else
    {
        return ERR_MEM;
    }
}
/** Wait for a new message to arrive in the mbox
 * @param mbox mbox to get a message from
 * @param msg pointer where the message is stored
 * @param timeout maximum time (in milliseconds) to wait for a message
 * @return time (in milliseconds) waited for a message, may be 0 if not waited
           or SYS_ARCH_TIMEOUT on timeout
 *         The returned time has to be accurate to prevent timer jitter!
 */
u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout)
{
    rt_err_t ret;
    s32_t t;
    u32_t  tick1,tick2,tick;

    RT_DEBUG_NOT_IN_INTERRUPT;

    tick1 = rt_tick_get();
    if (timeout == 0)
    {
        t = RT_WAITING_FOREVER;
    }
    else
    {
        t = timeout / (1000 / RT_TICK_PER_SECOND);
        t++;
    }
    ret = rt_mb_recv(*mbox, (rt_uint32_t*)msg, t);

    if (ret == -RT_ETIMEOUT)
    {
        return SYS_ARCH_TIMEOUT;
    }
    else
    {
        LWIP_ASSERT("rt_mb_recv returned with error!", ret == RT_EOK);
    }
    tick2 = rt_tick_get();
    tick = (tick2 + RT_TICK_MAX - tick1) % RT_TICK_MAX;
    tick = tick * (1000 / RT_TICK_PER_SECOND);//tick 转化为ms
 
    return tick;
}
/** Wait for a new message to arrive in the mbox
 * @param mbox mbox to get a message from
 * @param msg pointer where the message is stored
 * @param timeout maximum time (in milliseconds) to wait for a message
 * @return 0 (milliseconds) if a message has been received
 *         or SYS_MBOX_EMPTY if the mailbox is empty
 */
u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
    int ret;

    ret = rt_mb_recv(*mbox, (rt_uint32_t *)msg, 0);

    if(ret == -RT_ETIMEOUT)
    {
        return SYS_ARCH_TIMEOUT;
    }
    else if (ret == RT_EOK) 
    {
        ret = 1;
    }

    return ret;
}

#ifndef sys_mbox_valid
/** Check if an mbox is valid/allocated:
 *  return 1 for valid, 0 for invalid
 */
int sys_mbox_valid(sys_mbox_t *mbox)
{
   if (*mbox == RT_NULL)
   {
       return 0;
   }
   else
   {
       return 1;
   }
}
#endif

#ifndef sys_mbox_set_invalid
/** Set an mbox invalid so that sys_mbox_valid returns 0
 */
void sys_mbox_set_invalid(sys_mbox_t *mbox)
{
    *mbox = RT_NULL;
}
#endif
//创建线程函数
sys_thread_t sys_thread_new(const char *name, lwip_thread_fn thread, void *arg, int stacksize, int prio)
{
    rt_thread_t t;

    RT_DEBUG_NOT_IN_INTERRUPT;

    t = rt_thread_create(name, thread, arg, stacksize, prio, 20);
    RT_ASSERT(t != RT_NULL);

    rt_thread_startup(t);

    return t;
}

sys_prot_t sys_arch_protect(void)
{
    rt_base_t level;

    /* disable interrupt */
    level = rt_hw_interrupt_disable();

    return level;
}

void sys_arch_unprotect(sys_prot_t pval)
{
    /* enable interrupt */
    rt_hw_interrupt_enable(pval);

    return;
}

void sys_arch_assert(const char *file, int line)
{
    rt_kprintf("\nAssertion: %d in %s, thread %s\n",
        line, file, rt_thread_self()->name);
    RT_ASSERT(0);
}

#ifndef sys_jiffies
/** Ticks/jiffies since power up. */
u32_t sys_jiffies(void)
{
    return rt_tick_get();
}
#endif

u32_t sys_now(void)
{
    return rt_tick_get() * (1000 / RT_TICK_PER_SECOND);
}
void sys_init(void)
{
    //nothing to do
}


/***************************************************************************************************
*\Function      lwip_sys_init
*\Description   lwip协议栈初始化函数接口
*\Parameter     
*\Return        void
*\Note          
*\Log           2015.04.10    Ver 1.0    张波
*               创建函数。
***************************************************************************************************/
void lwip_sys_init(void)
{
    struct ip_addr ip_addr, netmask, gw;

    //设置默认netif
    netif_default = RT_NULL;
    stm32_netif = (struct netif*)rt_malloc(sizeof(struct netif));

    //初始化lwip及其协议栈，在函数中创建了一个tcpip线程
    tcpip_init(RT_NULL, RT_NULL);

#if LWIP_DHCP
    IP4_ADDR(&gw, 0,0,0,0);
    IP4_ADDR(&ip_addr, 0,0,0,0);
    IP4_ADDR(&netmask, 0,0,0,0);

    netif_add(stm32_netif, &ip_addr, &netmask, &gw, RT_NULL, ethernetif_init, tcpip_input);

    if (stm32_netif->flags & NETIF_FLAG_DHCP)//检查是否启用了dhcp标志
    {
        dhcp_start(stm32_netif);
    }
    else
    {
        rt_kprintf("DHCP Flag is Disable!\n");
    }
#else
    IP4_ADDR(&ip_addr, RT_LWIP_IPADDR0, RT_LWIP_IPADDR1, RT_LWIP_IPADDR2, RT_LWIP_IPADDR3);
    IP4_ADDR(&gw, RT_LWIP_GWADDR0, RT_LWIP_GWADDR1, RT_LWIP_GWADDR2, RT_LWIP_GWADDR3);
    IP4_ADDR(&netmask, RT_LWIP_MSKADDR0, RT_LWIP_MSKADDR1, RT_LWIP_MSKADDR2, RT_LWIP_MSKADDR3);
    netif_add(stm32_netif, &ip_addr, &netmask, &gw, RT_NULL, ethernetif_init, tcpip_input);
#endif

    netif_set_default(stm32_netif);
    netif_set_up(stm32_netif);

    rt_kprintf("lwip-%d.%d.%d initialized!\n", LWIP_VERSION_MAJOR, LWIP_VERSION_MINOR, LWIP_VERSION_REVISION);
}

u8_t netif_up_state(void)
{
    return netif_is_up(stm32_netif);
}

