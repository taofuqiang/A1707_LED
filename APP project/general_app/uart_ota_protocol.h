/***************************************************************************************************
*                    (c) Copyright 2008-2018  Syncretize technologies co.,ltd.
*										All Rights Reserved
*
*\File          uart_ota_protocol.h
*\Description   
*\Note          
*\Log           2018.01.19    Ver 1.0    Job
*               创建文件。
***************************************************************************************************/
#ifndef _UART_OTA_PROTOCOL_H
#define _UART_OTA_PROTOCOL_H
#include "app_board.h"

#define COMM_PROTOCOL_HEAD    0xFE
#define COMM_PROTOCOL_TAIL    0xBE

#define COMM_PROTOCOL_MIN_LEN  8
#define COMM_PROTOCOL_MAX_LEN  1280

enum
{
    OTA_Start  = 0xE0,
    OTA_Update = 0xE1,
    OTA_End    = 0xE2,
};


#define FUC_PARAMSET_RESET          0xA0
#define FUC_PARAMSET_SAVE           0xA1

#define FUC_PARAM_IAP_READ      		0x0E
#define FUC_PARAM_IAP_WRITE        		0x0F



uint32_t iap_param_read(uint8_t* buf, uint32_t len);
void iap_param_analysis(uint8_t* buf);
void uart_ota_init(void);
#endif /*_UART_OTA_PROTOCOL_H*/

