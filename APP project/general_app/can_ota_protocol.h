
/***************************************************************************************************
*                    (c) Copyright 1992-2019 Syncretize technologies co.,ltd.
*                                       All Rights Reserved
*
*\File          can_ota_protocol.h
*\Description   
*\Note          
*\Log           2019.01.15    Ver 1.0    庞国瑞
*               创建文件。
***************************************************************************************************/
#ifndef _CAN_OTA_PROTOCOL_H
#define _CAN_OTA_PROTOCOL_H
#include "public_type.h"
#include "string.h"
#include "stdio.h"
#include "usdl_type.h"
#include "canbus_middle.h"

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

extern RingBufType CAN_BUF500;
void can_ota_init(void);
#endif /*_CAN_OTA_PROTOCOL_H*/



/***************************************************************************************************
******************************************** END OF FILE *******************************************
***************************************************************************************************/


