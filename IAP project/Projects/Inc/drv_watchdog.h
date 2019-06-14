/***************************************************************************************************
*                    (c) Copyright 1992-2015 Syncretize technologies co.,ltd.
*                                       All Rights Reserved
*
*\File          watchdog.h
*\Description   
*\Note          
*\Log           2015.05.30    Ver 1.0    张波
*               创建文件。
***************************************************************************************************/
#ifndef _DRV_WATCHDOG_H
#define _DRV_WATCHDOG_H
#include "public_type.h"
#include "iap_if.h"

void watchdog_init(void);
void Watchdog_Refresh(void);

void Watchdog_Periodic_Handle(uint32_t localtime);
void AutoReset_Periodic_Handle(void);

extern u8 SysRstFlag;//复位标志记录
extern int64_t Sys_Run_Time;//系统运行时间
//得到系统复位的方式,看门狗&软件&掉电
extern void GetRestFlag(void);
extern void watchdog_refresh_and_count(void);
void show_version(void);
#endif /*_WATCHDOG_H*/

