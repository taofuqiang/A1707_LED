/***************************************************************************************************
*                    (c) Copyright 2008-2018  Syncretize technologies co.,ltd.
*										All Rights Reserved
*
*\File          canbus_middle.h
*\Description   
*\Note          
*\Log           2018.08.29    Ver 1.0    Job
*               创建文件。
***************************************************************************************************/
#ifndef _CANBUS_MIDDLE_H
#define _CANBUS_MIDDLE_H
#include "app_board.h"
#include "can_ota_protocol.h"

u32 canbus_middle_send(CANBus_type* can, u32 stdID, u8* buf, u32 len);
void canbus_middle_thread_entry(void* p);
#endif /*_CANBUS_MIDDLE_H*/

