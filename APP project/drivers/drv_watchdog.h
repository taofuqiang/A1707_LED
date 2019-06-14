/***************************************************************************************************
*                    (c) Copyright 2008-2017  Syncretize technologies co.,ltd.
*										All Rights Reserved
*
*\File          drv_watchdog.h
*\Description   
*\Note          
*\Log           2017.06.01    Ver 1.0    Job
*               创建文件。
***************************************************************************************************/
#ifndef _DRV_WATCHDOG_H
#define _DRV_WATCHDOG_H
#include "public_type.h"

void GetRestFlag(void);
void system_reset(void);
void rt_hw_watchdog_init(void);
void rt_watchdog_thread_entry(void* p);

extern rt_uint32_t Sys_Run_Time;//系统运行时间

#endif /*_DRV_WATCHDOG_H*/

