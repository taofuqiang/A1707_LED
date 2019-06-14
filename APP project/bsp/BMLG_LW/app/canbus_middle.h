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

/* USER CODE BEGIN Private defines */
#define UART1_RxBuf_SIZE       8
#define UART1_Frame_Head       0xFE
#define UART1_Frame_End        0xBE
	 
#define State_Frame_Head       0
#define State_Frame_Length     1
#define State_Frame_Data       2
#define State_Frame_CS         3
#define State_Frame_End        4
/*Define a Counter Struct*/

extern uint8_t UART1_RxFlag;

u32 canbus_middle_send(CANBus_type* can, u32 stdID, u8* buf, u32 len);
void canbus_middle_thread_entry(void* p);

void USART1_IRQHandler_Callback(void);
#endif /*_CANBUS_MIDDLE_H*/

