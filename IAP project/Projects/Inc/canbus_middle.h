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
#include "public_type.h"
#include "string.h"
#include "stdio.h"
#include "drv_can.h"


#define CAN_COMM_PROTOCOL_HEAD    0xFE
#define CAN_COMM_PROTOCOL_TAIL    0xBE

#define CAN_COMM_PROTOCOL_MIN_LEN  8
#define CAN_COMM_PROTOCOL_MAX_LEN  1460

enum
{
	CAN_OTA_Recv_Start = 0x01,
	CAN_OTA_Recv_Update = 0x02,
	CAN_OTA_Recv_End = 0x03,
	CAN_OTA_Send_Start = 0x81,
	CAN_OTA_Send_Update = 0x82,
	CAN_OTA_Send_End = 0x83,
};

/***************START***********设备管理/搜索/自检baron****************************************/
#define FUC_PARAMSET_READ           0xA1
#define FUC_PARAMSET_WRITE          0xA2
#define FUC_PARAMSET_SAVE           0xA3
#define FUC_DEVICE_RESTART          0xA4
#define FUC_PARAMSET_DEFAULT        0xA5

#define FUC_DEVICE_SEARCH           0xB1
#define FUC_DEVICE_PARAMSET_SEARCH  0xB2

#define FUC_AUTOMATIC_CHECK         0xC1
/***************END****************************************************************************/
/*解析用结构*/
typedef enum
{
    can_anylise_waiting,
    can_anylise_accept,
    can_anylise_reject,   
}can_anylise_state;

typedef struct can_del_info_type_st 
{
    can_anylise_state state;
    u32 del_len;
}can_del_info_type;

u32 canbus_middle_send(CANBus_type* can, u32 stdID, u8* buf, u32 len);
void CAN_OTA_Periodic_Handle(void);
void CAN_middle_init(void);
#endif /*_CANBUS_MIDDLE_H*/

