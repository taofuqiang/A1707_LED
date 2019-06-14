
/***************************************************************************************************
*                    (c) Copyright 1992-2019 Syncretize technologies co.,ltd.
*                                       All Rights Reserved
*
*\File          can_search_protocol.h
*\Description   
*\Note          
*\Log           2019.01.15    Ver 1.0    庞国瑞
*               创建文件。
***************************************************************************************************/
#ifndef _CAN_SEARCH_PROTOCOL_H
#define _CAN_SEARCH_PROTOCOL_H
#include "public_type.h"

extern u32 CAN_Paramset_Read_Construct(u8* buf , u32 len);
extern u32 CAN_Automatic_Check_Construct(u8* buf, u32 len);
extern void CAN_Paramset_Receive_Analyse(u8* buf);
extern u32 CAN_Paramset_Write_Construct(u8* buf, u32 len);
extern u32 CAN_Paramset_Save_Construct(u8* buf, u32 len);
extern u32 CAN_Device_Restart_Construct(u8* buf, u32 len);
extern u32 CAN_Paramset_Default_Construct(u8* buf, u32 len);
extern u32 CAN_Device_Search_Construct(u8* buf, u32 len);
extern u32 CAN_Device_Paramset_Search_Construct(u8* buf, u32 len);
extern void CAN_Device_Paramset_Search_SetID(u8* buf);



#endif /*_CAN_SEARCH_PROTOCOL_H*/



/***************************************************************************************************
******************************************** END OF FILE *******************************************
***************************************************************************************************/


