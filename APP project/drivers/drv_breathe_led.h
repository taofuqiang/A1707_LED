/***************************************************************************************************
*                    (c) Copyright 2006-2016  Syncretize technologies co.,ltd.
*										All Rights Reserved
*	
*\File 			drv_breathe_led.h
*\Description   
*\Note          
*\Log           2016.08.23    Ver 1.0   Job
*               创建文件。
***************************************************************************************************/
#ifndef _DRV_BREATHE_LED_H
#define _DRV_BREATHE_LED_H
#include "common_lib.h"
#include "drv_switch.h"

typedef struct BreatheMsgLedType_st
{
    switch_type*   sw_led;
    //上面需初始化
    rt_timer_t    timer_msg;
    u32           count;//点亮次数
    u32           count_light;//已经点亮次数
    u32           on_cycle;//点亮时间
    u32           off_cycle;//关闭时间
}BreatheMsgLedType;

bool breathe_msg_led_on(BreatheMsgLedType* msg, u32 count, bool steady);
bool breathe_msg_led_off(BreatheMsgLedType* msg);
bool breathe_msg_led_attr_set(BreatheMsgLedType* msg, u32 on_cycle, u32 off_cycle);
bool breathe_msg_led_init(BreatheMsgLedType* msg_auto, char* name, u32 on_cycle, u32 off_cycle);


#endif /*_DRV_BREATHE_LED_H*/
