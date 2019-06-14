
/***************************************************************************************************
*                    (c) Copyright 1992-2019 Syncretize technologies co.,ltd.
*                                       All Rights Reserved
*
*\File          can_heartbeat_protocol.c
*\Description   
*\Note          
*\Log           2019.03.12    Ver 1.0    庞国瑞
*               创建文件。
***************************************************************************************************/
#include "can_heartbeat_protocol.h"
#include "can_ota_protocol.h"
#include "iap_if.h"
#include "param.h"


/***************************************************************************************************
*\Function      CAN_Heartbeat_Package_Send
*\Description   //默认向0x0101平台控制板发送心跳包
*\Parameter     
*\Return        void
*\Note          
*\Log           2019.03.14    Ver 1.0    庞国瑞
*               创建函数。
***************************************************************************************************/
__weak void CAN_Heartbeat_Package_Send(void)
{
	u32 data_len = 8;
	u8 buf[8];
	u32 i = 0;
	
	buf[i++] = (u8)iap_param.ID;//ID
	buf[i++] = (u8)(iap_param.ID >> 8);//ID
	buf[i++] = realtime_param.heartbeat_count;//序号
	buf[i++] = dev_param.version& 0xFF;//app版本低
	buf[i++] = (u8)realtime_param.voltage;//电压低
	buf[i++] = (u8)(realtime_param.voltage >> 8);//电压高
	buf[i++] = (u8)realtime_param.current;//电流低
	buf[i++] = (u8)(realtime_param.current >> 8);//电流高
	canbus_middle_send(&CAN, 0x600, buf, data_len);
}

//can 心跳包 每10s一次
void rt_thread_heartbeat_entry(void* parameter)
{	
	rt_thread_delay(1000);//设备启动10s后开启心跳包

	while (1)
	{
		realtime_param.heartbeat_count+=1;
		if(realtime_param.heartbeat_count>=255)
		{
			realtime_param.heartbeat_count=0;
		}		
		CAN_Heartbeat_Package_Send();
		//心跳包   10S一次 
		rt_thread_delay(1000);
	}
}

/***************************************************************************************************
*\Function      rt_thread_heartbeat_init
*\Description   
*\Parameter     
*\Return        void
*\Note          
*\Log           2019.03.14    Ver 1.0    庞国瑞
*               创建函数。
***************************************************************************************************/
void rt_thread_heartbeat_init(void)
{
	rt_thread_t heartbeat_init_thread;                    
	heartbeat_init_thread = rt_thread_create("heart",
		rt_thread_heartbeat_entry,RT_NULL,512,12,5);
	if (heartbeat_init_thread != RT_NULL)
		rt_thread_startup(heartbeat_init_thread);
}
/***************************************************************************************************
******************************************** END OF FILE *******************************************
***************************************************************************************************/


