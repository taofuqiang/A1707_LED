
/***************************************************************************************************
*                    (c) Copyright 1992-2019 Syncretize technologies co.,ltd.
*                                       All Rights Reserved
*
*\File          can_search_protocol.c
*\Description   
*\Note          
*\Log           2019.01.15    Ver 1.0    庞国瑞
*               创建文件。
***************************************************************************************************/
#include "can_search_protocol.h"
#include "can_ota_protocol.h"
#include "param.h"
#include "iap_if.h"
#include "flash_if.h"

/***************START***********设备管理/搜索/自检baron****************************************/
extern rt_uint32_t Sys_Run_Time;//系统运行时间
/***************************************************************************************************
*\Function      CAN_Paramset_Read_Construct
*\Description   设备参数的读取
*\Parameter     buf
*\Parameter     id_buf  目标id
*\Parameter     len
*\Return        u32
*\Note
*\Log           2019.03.07    Ver 1.0    baron
*               创建函数。
***************************************************************************************************/
__weak u32 CAN_Paramset_Read_Construct(u8* buf , u32 len)//需要每人根据自己的项目组包
{
	u32 data_len = 32;
	u32 i = 0;

	if (len < data_len)
	{
		return 0;
	}

	buf[i++] = CAN_COMM_PROTOCOL_HEAD;
	buf[i++] = (u8)(data_len);
	buf[i++] = (u8)((data_len) >> 8);
	buf[i++] = FUC_PARAMSET_READ;
	buf[i++] = (u8)iap_param.remoteID;//remoteID
	buf[i++] = (u8)(iap_param.remoteID >> 8);//remoteID
	buf[i++] = (u8)iap_param.ID;//ID
	buf[i++] = (u8)(iap_param.ID >> 8);//ID

	buf[i++] = (u8)iap_param.ID;//ID
	buf[i++] = (u8)(iap_param.ID >> 8);//ID
	buf[i++] = dev_param.mswork_mode;// 主从模式
	buf[i++] = realtime_param.can_debug_flag;// 调试信息
	buf[i++] = dev_param.analog_dial_switch;// 模拟拨码开关
	buf[i++] = (u8)realtime_param.voltage;// 电压低
	buf[i++] = (u8)(realtime_param.voltage >> 8);// 电压高
	buf[i++] = (u8)realtime_param.current;// 电流低
	buf[i++] = (u8)(realtime_param.current >> 8);// 电流高
	buf[i++] = (u8)Sys_Run_Time;// 运行时间0
	buf[i++] = (u8)(Sys_Run_Time >> 8);// 运行时间8
	buf[i++] = (u8)(Sys_Run_Time >> 16);// 运行时间16
	buf[i++] = (u8)(Sys_Run_Time >> 24);// 运行时间24
	buf[i++] = (u8)dev_param.version;//app版本低;
	buf[i++] = (u8)(dev_param.version >> 8);// app版本高
	buf[i++] = (u8)iap_param.swversion;// iap版本低
	buf[i++] = (u8)(iap_param.swversion >> 8);// iap版本高
	buf[i++] = iap_param.hwversion;// 硬件版本
	buf[i++] = 0x00;
	buf[i++] = 0x00;
	buf[i++] = 0x00;
	buf[i++] = 0x00;	
	buf[i++] = Get_Sum8(buf + 1, data_len - 3);
	buf[i++] = CAN_COMM_PROTOCOL_TAIL;

	return i;
}

/***************************************************************************************************
*\Function      CAN_Paramset_Receive_Analyse
*\Description   数据解析
*\Parameter     buf
*\Parameter     len
*\Return        u32
*\Note
*\Log           2019.03.08    Ver 1.0    baron
*               创建函数。
***************************************************************************************************/
__weak void CAN_Paramset_Receive_Analyse(u8* buf)//需要根据自己的协议解析函数
{
	iap_param.ID=((buf[9] << 8) | buf[8]);//ID
	dev_param.mswork_mode=buf[10];// 主从模式
	realtime_param.can_debug_flag=buf[11];// 调试信息
	dev_param.analog_dial_switch=buf[12];// 模拟拨码开关
	
}

/***************************************************************************************************
*\Function      CAN_AUTOMATIC_CHECK_ACK_Construct
*\Description   自动检测发送回复
*\Parameter     buf
*\Parameter     id_buf  目标id
*\Parameter     len
*\Return        u32
*\Note
*\Log           2019.03.08    Ver 1.0    baron
*               创建函数。
***************************************************************************************************/
__weak u32 CAN_Automatic_Check_Construct(u8* buf, u32 len)//根据不同的设备进行组包
{
	u32 data_len = 24;
	u32 i = 0;

	if (len < data_len)
	{
		return 0;
	}

	buf[i++] = CAN_COMM_PROTOCOL_HEAD;
	buf[i++] = (u8)(data_len);
	buf[i++] = (u8)((data_len) >> 8);
	buf[i++] = FUC_DEVICE_SEARCH;
	buf[i++] = (u8)iap_param.remoteID;//remoteID
	buf[i++] = (u8)(iap_param.remoteID >> 8);//remoteID
	buf[i++] = (u8)iap_param.ID;//ID
	buf[i++] = (u8)(iap_param.ID >> 8);//ID

	buf[i++] = 0x00;     //具体需检测参数单独添加
	buf[i++] = 0x00;
	buf[i++] = 0x00;
	buf[i++] = 0x00;
	buf[i++] = 0x00;
	buf[i++] = 0x00;
	buf[i++] = 0x00;
	buf[i++] = 0x00;
	buf[i++] = 0x00;
	buf[i++] = 0x00;
	buf[i++] = 0x00;
	buf[i++] = 0x00;
	buf[i++] = 0x00;
	buf[i++] = 0x00;
	buf[i++] = Get_Sum8(buf + 1, data_len - 3);
	buf[i++] = CAN_COMM_PROTOCOL_TAIL;

	return i;
}


/***************************************************************************************************
*\Function      CAN_Paramset_write_Construct
*\Description   设备参数写入回复
*\Parameter     buf
*\Parameter     id_buf  目标id
*\Parameter     len
*\Return        u32
*\Note
*\Log           2019.03.07    Ver 1.0    baron
*               创建函数。
***************************************************************************************************/
u32 CAN_Paramset_Write_Construct(u8* buf, u32 len)
{
	u32 data_len = 10;
	u32 i = 0;

	if (len < data_len)
	{
		return 0;
	}

	buf[i++] = CAN_COMM_PROTOCOL_HEAD;
	buf[i++] = (u8)(data_len);
	buf[i++] = (u8)((data_len) >> 8);
	buf[i++] = FUC_PARAMSET_WRITE;
	buf[i++] = (u8)iap_param.remoteID;//remoteID
	buf[i++] = (u8)(iap_param.remoteID >> 8);//remoteID
	buf[i++] = (u8)iap_param.ID;//ID
	buf[i++] = (u8)(iap_param.ID >> 8);//ID

	buf[i++] = Get_Sum8(buf + 1, data_len - 3);
	buf[i++] = CAN_COMM_PROTOCOL_TAIL;

	return i;
}
/***************************************************************************************************
*\Function      CAN_PARAMSET_SAVE_ACK_Construct
*\Description   参数写入flash回复
*\Parameter     buf
*\Parameter     id_buf  目标id
*\Parameter     len
*\Return        u32
*\Note
*\Log           2019.03.07    Ver 1.0    baron
*               创建函数。
***************************************************************************************************/
u32 CAN_Paramset_Save_Construct(u8* buf, u32 len)
{
	u32 data_len = 10;
	u32 i = 0;

	if (len < data_len)
	{
		return 0;
	}

	buf[i++] = CAN_COMM_PROTOCOL_HEAD;
	buf[i++] = (u8)(data_len);
	buf[i++] = (u8)((data_len) >> 8);
	buf[i++] = FUC_PARAMSET_SAVE;
	buf[i++] = (u8)iap_param.remoteID;//remoteID
	buf[i++] = (u8)(iap_param.remoteID >> 8);//remoteID
	buf[i++] = (u8)iap_param.ID;//ID
	buf[i++] = (u8)(iap_param.ID >> 8);//ID

	buf[i++] = Get_Sum8(buf + 1, data_len - 3);
	buf[i++] = CAN_COMM_PROTOCOL_TAIL;

	return i;
}
/***************************************************************************************************
*\Function      CAN_DEVICE_RESTART_ACK_Construct
*\Description   重启设备回复
*\Parameter     buf
*\Parameter     id_buf  目标id
*\Parameter     len
*\Return        u32
*\Note
*\Log           2019.03.07    Ver 1.0    baron
*               创建函数。
***************************************************************************************************/
u32 CAN_Device_Restart_Construct(u8* buf, u32 len)
{
	u32 data_len = 10;
	u32 i = 0;

	if (len < data_len)
	{
		return 0;
	}

	buf[i++] = CAN_COMM_PROTOCOL_HEAD;
	buf[i++] = (u8)(data_len);
	buf[i++] = (u8)((data_len) >> 8);
	buf[i++] = FUC_DEVICE_RESTART;
	buf[i++] = (u8)iap_param.remoteID;//remoteID
	buf[i++] = (u8)(iap_param.remoteID >> 8);//remoteID
	buf[i++] = (u8)iap_param.ID;//ID
	buf[i++] = (u8)(iap_param.ID >> 8);//ID

	buf[i++] = Get_Sum8(buf + 1, data_len - 3);
	buf[i++] = CAN_COMM_PROTOCOL_TAIL;

	return i;
}
/***************************************************************************************************
*\Function      CAN_PARAMSET_DEFAULT_ACK_Construct
*\Description   恢复默认回复
*\Parameter     buf
*\Parameter     id_buf  目标id
*\Parameter     len
*\Return        u32
*\Note
*\Log           2019.03.07    Ver 1.0    baron
*               创建函数。
***************************************************************************************************/
u32 CAN_Paramset_Default_Construct(u8* buf, u32 len)
{
	u32 data_len = 10;
	u32 i = 0;

	if (len < data_len)
	{
		return 0;
	}

	buf[i++] = CAN_COMM_PROTOCOL_HEAD;
	buf[i++] = (u8)(data_len);
	buf[i++] = (u8)((data_len) >> 8);
	buf[i++] = FUC_PARAMSET_DEFAULT;
	buf[i++] = (u8)iap_param.remoteID;//remoteID
	buf[i++] = (u8)(iap_param.remoteID >> 8);//remoteID
	buf[i++] = (u8)iap_param.ID;//ID
	buf[i++] = (u8)(iap_param.ID >> 8);//ID

	buf[i++] = Get_Sum8(buf + 1, data_len - 3);
	buf[i++] = CAN_COMM_PROTOCOL_TAIL;

	return i;
}
/***************************************************************************************************
*\Function      CAN_DEVICE_SEARCH_ACK_Construct
*\Description   设备搜索回复
*\Parameter     buf
*\Parameter     id_buf  目标id
*\Parameter     len
*\Return        u32
*\Note
*\Log           2019.03.08    Ver 1.0    baron
*               创建函数。
***************************************************************************************************/
u32 CAN_Device_Search_Construct(u8* buf, u32 len)
{
	u32 data_len = 32;
	u32 i = 0;

	if (len < data_len)
	{
		return 0;
	}

	buf[i++] = CAN_COMM_PROTOCOL_HEAD;
	buf[i++] = (u8)(data_len);
	buf[i++] = (u8)((data_len) >> 8);
	buf[i++] = FUC_DEVICE_SEARCH;
	buf[i++] = (u8)iap_param.remoteID;//remoteID
	buf[i++] = (u8)(iap_param.remoteID >> 8);//remoteID
	buf[i++] = (u8)iap_param.ID;//ID
	buf[i++] = (u8)(iap_param.ID >> 8);//ID

	buf[i++] = (u8)iap_param.ChipUniqueID[0];     //UUID0
	buf[i++] = (u8)(iap_param.ChipUniqueID[0] >> 8);
	buf[i++] = (u8)iap_param.ChipUniqueID[1];
	buf[i++] = (u8)(iap_param.ChipUniqueID[1] >> 8);
	buf[i++] = (u8)iap_param.ChipUniqueID[2];
	buf[i++] = (u8)(iap_param.ChipUniqueID[2] >> 8);
	buf[i++] = (u8)iap_param.ChipUniqueID[3];
	buf[i++] = (u8)(iap_param.ChipUniqueID[3] >> 8);
	buf[i++] = (u8)iap_param.ChipUniqueID[4];
	buf[i++] = (u8)(iap_param.ChipUniqueID[4] >> 8);
	buf[i++] = (u8)iap_param.ChipUniqueID[5];
	buf[i++] = (u8)(iap_param.ChipUniqueID[5] >> 8);//UUID11
	buf[i++] = (u8)iap_param.swversion;             //iap版本低
	buf[i++] = (u8)(iap_param.swversion >> 8);        //iap版本高
	buf[i++] = iap_param.hwversion;                  //硬件版本
	buf[i++] = iap_param.IAP_flag;                 //iap标志位
	buf[i++] = (u8)dev_param.version;//app版本低
	buf[i++] = (u8)(dev_param.version >> 8);//app版本高
	buf[i++] = (u8)iap_param.ID;;//网关ID低
	buf[i++] = (u8)(iap_param.ID >> 8);//网关ID高
	buf[i++] = 0x00;
	buf[i++] = 0x00;
	buf[i++] = Get_Sum8(buf + 1, data_len - 3);
	buf[i++] = CAN_COMM_PROTOCOL_TAIL;

	return i;
}
/***************************************************************************************************
*\Function      CAN_DEVICE_PARAMSET_SEARCH_ACK_Construct
*\Description   设备搜索参数设置回复
*\Parameter     buf
*\Parameter     id_buf  目标id
*\Parameter     len
*\Return        u32
*\Note
*\Log           2019.03.08    Ver 1.0    baron
*               创建函数。
***************************************************************************************************/
u32 CAN_Device_Paramset_Search_Construct(u8* buf, u32 len)
{
	u32 data_len = 10;
	u32 i = 0;

	if (len < data_len)
	{
		return 0;
	}

	buf[i++] = CAN_COMM_PROTOCOL_HEAD;
	buf[i++] = (u8)(data_len);
	buf[i++] = (u8)((data_len) >> 8);
	buf[i++] = FUC_DEVICE_PARAMSET_SEARCH;
	buf[i++] = (u8)iap_param.remoteID;//remoteID
	buf[i++] = (u8)(iap_param.remoteID >> 8);//remoteID
	buf[i++] = (u8)iap_param.ID;//ID
	buf[i++] = (u8)(iap_param.ID >> 8);//ID

	buf[i++] = Get_Sum8(buf + 1, data_len - 3);
	buf[i++] = CAN_COMM_PROTOCOL_TAIL;

	return i;
}


//设备搜索 设置设备ID
/***************************************************************************************************
*\Function      CAN_Device_Paramset_Search_SetID
*\Description   
*\Parameter     buf
*\Return        void
*\Note          
*\Log           2019.03.14    Ver 1.0    庞国瑞
*               创建函数。
***************************************************************************************************/
void CAN_Device_Paramset_Search_SetID(u8* buf)
{
	u32 i=0;
	int check_sum=0;
	uint16_t RecvUUID[6];//uuid
	uint16_t RecvID;//设备需要调整的id
	RecvID=((buf[27] << 8) | buf[26]);
	RecvUUID[0]=((buf[9] << 8) | buf[8]);
	RecvUUID[1]=((buf[11] << 8) | buf[10]);
	RecvUUID[2]=((buf[13] << 8) | buf[12]);
	RecvUUID[3]=((buf[15] << 8) | buf[14]);
	RecvUUID[4]=((buf[17] << 8) | buf[16]);
	RecvUUID[5]=((buf[19] << 8) | buf[18]);
	for(i=0;i<6;i++)
	{
		if(RecvUUID[i]==iap_param.ChipUniqueID[i])
		{
			check_sum+=1;
		}
		else
		{
			check_sum+=0;
		}
	}
	
	if(check_sum==6)
	{//uuid校验通过 写入id
		iap_param.ID=RecvID;
	}
	else
	{//uuid校验不通过
		
	}
	
}


/***************END****************************************************************************/
/***************************************************************************************************
******************************************** END OF FILE *******************************************
***************************************************************************************************/


