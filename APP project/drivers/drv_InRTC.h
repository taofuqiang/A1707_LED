/***************************************************************************************************
*                    (c) Copyright 2008-2017  Syncretize technologies co.,ltd.
*										All Rights Reserved
*
*\File          drv_InRTC.h
*\Description   
*\Note          
*\Log           2017.06.01    Ver 1.0    Job
*               创建文件。
***************************************************************************************************/
#ifndef _DRV_INRTC_H
#define _DRV_INRTC_H
#include "common_lib.h"

#define HOUR_FORMAT     0   /*为0:24小时格式 大于0:为12小时格式*/


void stm32_Inrtc_init(bool force);
bool stm32_Inrtc_getTime(TIME* time);
bool stm32_Inrtc_setTime(TIME time);
#endif /*_DRV_INRTC_H*/

