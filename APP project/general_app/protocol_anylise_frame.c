/***************************************************************************************************
*                    (c) Copyright 1992-2015 Syncretize technologies co.,ltd.
*                                       All Rights Reserved
*
*\File          protocol_anylise_frame.c
*\Description   协议解析框架
*\Note          1、当解析成功时 应该重置wait_tick,否则当有多条数据帧到达时，由于解析成功，最后一帧
*               数据可能还未接收完而导致超时
*\Log           2015.07.11    Ver 1.0    张波
                2017年11月9日 Ver 1.1    Job
*               创建文件。
***************************************************************************************************/
#include "protocol_anylise_frame.h"

//定义协议输出信息函数
#define protocol_printf   rt_kprintf

/***************************************************************************************************
*\Function      comm_dev_create
*\Description   通讯设备创建函数接口
*\Parameter     name
*\Parameter     len 通讯设备需要的缓冲区长度
*\Parameter     recv   通讯设备接收数据函数
*\Parameter     del    通讯设备删除数据函数
*\Parameter     clear  通讯设备清除数据函数
*\Parameter     send   通讯设备发送数据函数
*\Return        comm_dev_type*
*\Note          
*\Log           2015.07.11    Ver 1.0    张波
*               创建函数。
***************************************************************************************************/
comm_dev_type* comm_dev_create(char* name, u32 len, \
                     u32 (*recv)(u8* buf, u32 len, bool state), \
                     u32 (*del)(u32 len),\
                     bool (*clear)(),\
                     u32 (*send)(u8* buf, u32 len),\
                     u32 out_time
                     )
{
    comm_dev_type* comm_dev = RT_NULL;

    comm_dev = (comm_dev_type*)rt_malloc(sizeof(comm_dev_type));
    if (comm_dev == RT_NULL)
    {
        protocol_printf("comm_dev creat fail!\n");
        return comm_dev;
    }
    comm_dev->name = name;
    comm_dev->buf_len = len;
    comm_dev->buf = rt_malloc(comm_dev->buf_len);
    if (comm_dev->buf == RT_NULL)
    {
        rt_kprintf("comm_dev buf create fail!\n");
        rt_free(comm_dev);
        comm_dev = RT_NULL;
        return comm_dev;
    }
    comm_dev->recv = recv;
    comm_dev->send = send;
    comm_dev->del = del;
    comm_dev->clear = clear;
    comm_dev->wait_ticks = 0;
    comm_dev->ticks_out = out_time;//2018年3月5日 By Job

    comm_dev->protocol_num = 0;
    rt_memset(comm_dev->protocol_anylise_fun, 0, sizeof(comm_dev->protocol_anylise_fun));

    comm_dev->tid = RT_NULL;

    return comm_dev;
}

/***************************************************************************************************
*\Function      protocol_anylise_add
*\Description   向协议解析器中添加实际解析函数
*\Parameter     protocol_anylise
*\Return        bool， TRUE代表添加成功
*\Note          
*\Log           2015.07.13    Ver 1.0    张波
*               创建函数。
***************************************************************************************************/
bool protocol_anylise_add(comm_dev_type* comm_dev, protocol_anylise_handle protocol_anylise)
{
    u32 i = 0;

    if (comm_dev == RT_NULL)
    {
        protocol_printf("comm_dev is NULL\n");
        return FALSE;
    }
    /*协议组已满，无法再添加*/
    if (comm_dev->protocol_num >= PROTOCOL_FUN_MAX)
    {
        protocol_printf("protocol fun is max!\n");
        return FALSE;
    }
    //先挂起线程
    if (comm_dev->tid != RT_NULL)
    {
        rt_thread_suspend(comm_dev->tid);
    }
    for (i = 0; i < PROTOCOL_FUN_MAX; i++)
    {
        //查找到空，添加
        if (comm_dev->protocol_anylise_fun[i] == RT_NULL)
        {
            comm_dev->protocol_anylise_fun[i] = protocol_anylise;
            comm_dev->protocol_num++;
            //先挂起线程
            if (comm_dev->tid != RT_NULL)
            {
                rt_thread_resume(comm_dev->tid);
            }
            return TRUE;
        }
    }

    if (comm_dev->tid != RT_NULL)
    {
        rt_thread_resume(comm_dev->tid);
    }
    return FALSE;
}

/***************************************************************************************************
*\Function      protocol_anylise_del
*\Description   从解析器容器中删除要解析的协议
*\Parameter     comm_dev
*\Parameter     protocol_anylise
*\Return        bool
*\Note          
*\Log           2015.07.13    Ver 1.0    张波
*               创建函数。
***************************************************************************************************/
bool protocol_anylise_del(comm_dev_type* comm_dev, protocol_anylise_handle protocol_anylise)
{
    u32 i = 0;

    if (comm_dev == RT_NULL)
    {
        protocol_printf("comm_dev is NULL\n");
        return FALSE;
    }
    //协议个数为0,代表不存在
    if (comm_dev->protocol_num == 0)
    {
        return TRUE;
    }
    //挂起线程
    if (comm_dev->tid != RT_NULL)
    {
        rt_thread_suspend(comm_dev->tid);
    }
    for (i = 0; i < PROTOCOL_FUN_MAX; i++)
    {
        //查找到，删除
        if (comm_dev->protocol_anylise_fun[i] == protocol_anylise)
        {
            comm_dev->protocol_anylise_fun[i] = RT_NULL;
            comm_dev->protocol_num--;
            if (comm_dev->tid != RT_NULL)
            {
                rt_thread_resume(comm_dev->tid);
            }
            return TRUE;
        }
    }

    if (comm_dev->tid != RT_NULL)
    {
        rt_thread_resume(comm_dev->tid);
    }

    return FALSE;
}

/***************************************************************************************************
*\Function      protocol_anylise_process
*\Description   协议解析框架函数
*\Parameter     comm
*\Return        void
*\Note          
*\Log           2015.7.13    Ver 1.0    张波
*               创建函数。
***************************************************************************************************/
static void protocol_anylise_process(comm_dev_type* comm)
{
    del_info_type info[PROTOCOL_FUN_MAX];
    u32 i = 0;
    u32 len = 0;
    u32 del_min = 0;
    int time_diff = 0;

    /*获取通讯设备数据*/
    len = comm->recv(comm->buf, comm->buf_len, FALSE);
    if (len == 0)
    {
        comm->wait_ticks = rt_tick_get();
        return;
    }
    for (i = 0; i < PROTOCOL_FUN_MAX; i++)
    {
        /*如果解析函数为空 则认为解析结果为拒绝，删除长度为最大*/
        if (comm->protocol_anylise_fun[i] == RT_NULL)
        {
            info[i].state = anylise_reject;
            info[i].del_len = len;
            continue;
        }
        info[i] = comm->protocol_anylise_fun[i](comm, comm->buf, len);
        if (info[i].state == anylise_accept)
        {
            /*删除成功解析的命令数据*/
            comm->del(info[i].del_len);
            //接收成功了 应该也重置一下时间 否则当有好几帧数据，可能会导致wait_tick超时删除 
            comm->wait_ticks = rt_tick_get();
            return;//解析成功，这次解析结束
        }
        else
        {
            continue;
        }
    }
    //运行到此处时，不会有accept，因为accept直接返回了
    /*处理解析结果，如果都是拒绝 则删除最小拒绝长度*/
    for (i = 0; i < PROTOCOL_FUN_MAX; i++)
    {
        if (info[i].state == anylise_reject)
        {
            if (del_min == 0)
            {
                del_min = info[i].del_len;
            }
            else
            {
                del_min = Min(del_min, info[i].del_len);
            }         
        }
        else
        {
            break;
        }
    }
    if (i >= PROTOCOL_FUN_MAX)//此条件成立，说明都是reject
    {
        comm->del(del_min);   
        comm->wait_ticks = rt_tick_get();
        return;
    }
    else
    {  
        time_diff = rt_tick_get()  - comm->wait_ticks;//直接放到Abs中会出现问题 跨界的时候
        if (Abs(time_diff) > comm->ticks_out)//超过200ms解析不成功，则清空缓冲区数据（或者保守一点，删除一个字节，但这样会解析比较慢）
        {
            comm->clear();
            comm->wait_ticks = rt_tick_get();
        }    
    }
}

/***************************************************************************************************
*\Function      protocol_anylise_thread_entry
*\Description   协议解析线程
*\Parameter     para
*\Return        void
*\Note          
*\Log           2015.07.13    Ver 1.0    张波
*               创建函数。
***************************************************************************************************/
static void protocol_anylise_thread_entry(void* para)
{
    comm_dev_type* comm_dev = (comm_dev_type*)para;

    if (comm_dev == RT_NULL)
    {
        protocol_printf("the para of procotol anylise thread is NULL\n");
        return;
    }
    
    while (1)
    {
        protocol_anylise_process(comm_dev);
        rt_thread_delay(RT_TICK_PER_SECOND / 50);
    }
}
/***************************************************************************************************
*\Function      protocol_anylise_startup
*\Description   开启协议解析
*\Parameter     comm_dev
*\Parameter     priority，解析线程优先级
*\Parameter     stack,线程堆栈
*\Return        bool
*\Note          
*\Log           2015.07.13    Ver 1.0    张波
*               创建函数。
***************************************************************************************************/
bool protocol_anylise_startup(comm_dev_type* comm_dev, rt_uint8_t priority, rt_uint32_t stack)
{
    if (comm_dev == RT_NULL)
    {
        protocol_printf("comm_dev is NULL\n");
        return FALSE;
    }
    //只有添加协议组 才开启解析线程
    if (comm_dev->protocol_num == 0)
    {
        protocol_printf("protocol fun is Null\n");
        return FALSE;
    }
    //解析线程已经启动
    if (comm_dev->tid != RT_NULL)
    {
        protocol_printf("protocol thread has startup!\n");
        return TRUE;
    }
    comm_dev->tid = rt_thread_create(comm_dev->name, protocol_anylise_thread_entry, comm_dev, stack, priority, 5);
    if (comm_dev->tid != RT_NULL)
    {
        rt_thread_startup(comm_dev->tid);
        return TRUE;
    }
    return FALSE;
}

/***************************************************************************************************
*\Function      protocol_anylise_destroy
*\Description   消灭comm_dev
*\Parameter     comm_dev
*\Return        bool
*\Note          
*\Log           2015.07.13    Ver 1.0    张波
*               创建函数。
***************************************************************************************************/
bool protocol_anylise_destroy(comm_dev_type* comm_dev)
{
    if (comm_dev == RT_NULL)
    {
        protocol_printf("comm_dev is NULL\n");
        return FALSE;
    }

    if (comm_dev->tid != RT_NULL)
    {
        rt_thread_delete(comm_dev->tid);
    }
    
    if (comm_dev->buf != RT_NULL)
    {
        rt_free(comm_dev->buf);
    }

    rt_free(comm_dev);

    return TRUE;
}
