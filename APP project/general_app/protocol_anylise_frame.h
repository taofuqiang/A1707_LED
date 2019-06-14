/***************************************************************************************************
*                    (c) Copyright 1992-2015 Syncretize technologies co.,ltd.
*                                       All Rights Reserved
*
*\File          protocol_anylise_frame.h
*\Description   协议解析框架
*\Note          
*\Log           2015.07.11    Ver 1.0    张波
*               创建文件。
***************************************************************************************************/
#ifndef _PROTOCOL_ANYLISE_FRAME_H
#define _PROTOCOL_ANYLISE_FRAME_H

#include "common_lib.h"

/*解析用结构*/
typedef enum
{
    anylise_waiting,
    anylise_accept,
    anylise_reject,   
}anylise_state;

typedef struct del_info_type_st 
{
    anylise_state state;
    u32 del_len;
}del_info_type;


/*协议解析最大支持组，即同一组数据源可解析的协议类型个数*/
#define PROTOCOL_FUN_MAX    3
typedef struct comm_dev_type_st comm_dev_type;
//协议处理函数指针，协议处理直接再函数中，需要外部处理则通过邮箱通信
typedef del_info_type (*protocol_anylise_handle)(comm_dev_type* comm, u8* buf, u32 len);

/*协议解析通讯设备类型*/
typedef struct comm_dev_type_st
{
    char* name;
    u8* buf;//协议解析用缓冲区
    u32 buf_len;//缓冲区长度 
    u32 (*recv)(u8* buf, u32 len, bool state);//接收数据函数
    u32 (*del)(u32 len);//删除数据函数
    bool (*clear)();//清除数据函数
    u32 (*send)(u8* buf, u32 len);
    u32 ticks_out;//超时删除时间 每个解析函数可能会不一样 by Job 2018年1月29日
    protocol_anylise_handle  protocol_anylise_fun[PROTOCOL_FUN_MAX];
    u32 protocol_num;//实际添加的协议个数
    rt_thread_t tid;
    u32 wait_ticks;//等待超时计时
}comm_dev_type;


comm_dev_type* comm_dev_create(char* name, u32 len, \
                               u32 (*recv)(u8* buf, u32 len, bool state), \
                               u32 (*del)(u32 len),\
                               bool (*clear)(),\
                               u32 (*send)(u8* buf, u32 len),\
                               u32 out_time
                               );
bool protocol_anylise_add(comm_dev_type* comm_dev, protocol_anylise_handle protocol_anylise);
bool protocol_anylise_del(comm_dev_type* comm_dev, protocol_anylise_handle protocol_anylise);
bool protocol_anylise_startup(comm_dev_type* comm_dev, rt_uint8_t priority, rt_uint32_t stack);
bool protocol_anylise_destroy(comm_dev_type* comm_dev);
#endif /*_PROTOCOL_ANYLISE_FRAME_H*/

