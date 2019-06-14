/***************************************************************************************************
*                    (c) Copyright 2008-2017  Syncretize technologies co.,ltd.
*										All Rights Reserved
*
*\File          drv_can.h
*\Description   
*\Note          
*\Log           2017.10.17    Ver 1.0    Job
*               创建文件。
***************************************************************************************************/
#ifndef _DRV_CAN_H
#define _DRV_CAN_H
#include "common_lib.h"

//波特率
typedef enum CAN_BR_type_st
{
    CAN_BAUDRATE_100K,
    CAN_BAUDRATE_200K,
    CAN_BAUDRATE_500K,
    CAN_BAUDRATE_1M,
}CAN_BR_type;

#define CAN_RX_BUFFER_SIZE 20
#define CAN_TX_TRY_TIMES 2

typedef struct CANBus_type_st
{
    CAN_TypeDef* can;
    GpioType*   TxGpio; //can发送脚
    GpioType*   RxGpio; //can接收脚
    CAN_BR_type baudrate;
    //以上需要配置
    CanRxMsg CANRxBuf[CAN_RX_BUFFER_SIZE];
    u32 CanRx_Read;
    u32 CanRx_Write;
}CANBus_type;

void stm32_CAN_init(CANBus_type* can_dev);
bool stm32_CAN_Filter_Add(CANBus_type* can_dev, u8 filter_num, u8 mode, u8 fifo, u32 id, u32 id_mask);
bool stm32_CAN_recv(CANBus_type* can_dev, CanRxMsg* rx);
bool stm32_CAN_send(CANBus_type* can_dev, CanTxMsg* tx);
bool stm32_CAN_clear(CANBus_type* can_dev);

void stm32_CAN_recv_isr(CANBus_type* can_dev, u8 can_fifo);
#endif /*_DRV_CAN_H*/

