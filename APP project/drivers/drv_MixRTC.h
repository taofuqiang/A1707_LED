/***************************************************************************************************
*                    (c) Copyright 2008-2017  Syncretize technologies co.,ltd.
*										All Rights Reserved
*
*\File          drv_MixRTC.h
*\Description   
*\Note          
*\Log           2017.06.01    Ver 1.0    Job
*               创建文件。
***************************************************************************************************/
#ifndef _DRV_MIXRTC_H
#define _DRV_MIXRTC_H
#include "drv_InRTC.h"

void stm32_rtc_init(void);

bool stm32_rtc_getTime(TIME* time);
bool stm32_rtc_setTime(TIME time);

#endif /*_DRV_MIXRTC_H*/

