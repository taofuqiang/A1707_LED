
/***************************************************************************************************
*                    (c) Copyright 1992-2019 Syncretize technologies co.,ltd.
*                                       All Rights Reserved
*
*\File          can_ota_protocol.c
*\Description   
*\Note          
*\Log           2019.01.15    Ver 1.0    庞国瑞
*               创建文件。
***************************************************************************************************/
#include "can_ota_protocol.h"
#include "protocol_anylise_frame.h"
#include "can_search_protocol.h"
#include "param.h"
#include "iap_if.h"
#include "flash_if.h"
#include "can_heartbeat_protocol.h"

RingBufType CAN_BUF500;
static comm_dev_type* CAN500_Anylise;

static u16 OTA_Version = 0;//版本
static u32 OTA_TotalSize = 0;//bin长度
static u32 OTA_SubSize = 0;//分包固定长
static u32 OTA_CRC32 = 0;//校验
static u16 OTA_Frame_Num = 0;//升级包序号
//OTA协议标志
static u8 OTA_UpdateFlag = 0;// 0 未升级，1 开始，2 传输中，3 传输完成
u32 can500_buf_recv(u8* buf, u32 len, bool state)
{
    return ReadDataFromBuf(&CAN_BUF500, buf, len, state);
}
u32 can500_buf_del(u32 len)
{
    return DelDataFromBuf(&CAN_BUF500, len);  
}
bool can500_buf_clear(void)
{
    return ClearDateFromBuf(&CAN_BUF500);
}

extern u8 Get_Sum8(u8* buf, u32 len);


/***************************************************************************************************
*\Function      OTA_Start_ACK_Construct
*\Description   OTA开始ACK构建
*\Parameter     buf
*\Parameter     len
*\Return        u32
*\Note
*\Log           2018.01.19    Ver 1.0    Job
*               创建函数。
***************************************************************************************************/
u32 CAN_OTA_Start_ACK_Construct(u8* buf, u32 len)
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
	buf[i++] = CAN_OTA_Send_Start;
	buf[i++] = (u8)iap_param.remoteID;//remoteID
	buf[i++] = (u8)(iap_param.remoteID >> 8);//remoteID
	buf[i++] = (u8)iap_param.ID;//ID
	buf[i++] = (u8)(iap_param.ID >> 8);//ID
	buf[i++] = Get_Sum8(buf + 1, data_len - 3);
	buf[i++] = CAN_COMM_PROTOCOL_TAIL;

	return i;
}

/***************************************************************************************************
*\Function      OTA_Update_ACK_Construct
*\Description   传输过程中ACK
*\Parameter     buf
*\Parameter     len
*\Return        u32
*\Note
*\Log           2018.01.19    Ver 1.0    Job
*               创建函数。
***************************************************************************************************/
u32 CAN_OTA_Update_ACK_Construct(u8* buf, u32 len, u8 state_code)
{
	u32 data_len = 13;
	u32 i = 0;

	if (len < data_len)
	{
		return 0;
	}

	buf[i++] = CAN_COMM_PROTOCOL_HEAD;
	buf[i++] = (u8)(data_len);
	buf[i++] = (u8)((data_len) >> 8);
	buf[i++] = CAN_OTA_Send_Update;
	buf[i++] = (u8)iap_param.remoteID;//remoteID
	buf[i++] = (u8)(iap_param.remoteID >> 8);//remoteID
	buf[i++] = (u8)iap_param.ID;//ID
	buf[i++] = (u8)(iap_param.ID >> 8);//ID

	buf[i++] = (u8)OTA_Frame_Num;
	buf[i++] = (u8)(OTA_Frame_Num >> 8);
	buf[i++] = state_code;

	buf[i++] = Get_Sum8(buf + 1, data_len - 3);
	buf[i++] = CAN_COMM_PROTOCOL_TAIL;

	return i;
}
/***************************************************************************************************
*\Function      OTA_End_ACK_Construct
*\Description
*\Parameter     buf
*\Parameter     len
*\Return        u32
*\Note
*\Log           2018.01.19    Ver 1.0    Job
*               创建函数。
***************************************************************************************************/
u32 CAN_OTA_End_ACK_Construct(u8* buf, u32 len)
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
	buf[i++] = CAN_OTA_Send_End;
	buf[i++] = (u8)iap_param.remoteID;//remoteID
	buf[i++] = (u8)(iap_param.remoteID >> 8);//remoteID
	buf[i++] = (u8)iap_param.ID;//ID
	buf[i++] = (u8)(iap_param.ID >> 8);//ID
	buf[i++] = Get_Sum8(buf + 1, data_len - 3);
	buf[i++] = CAN_COMM_PROTOCOL_TAIL;

	return i;
}

/***************************************************************************************************
*\Function      can500_protocol_anylise
*\Description   
*\Parameter     comm
*\Parameter     buf
*\Parameter     len
*\Return        del_info_type
*\Note          
*\Log           2018.08.30    Ver 1.0    Job               
				创建函数。
***************************************************************************************************/
static del_info_type can500_protocol_anylise(comm_dev_type* comm, u8* buf, u32 len)
{
    del_info_type info = {anylise_waiting, 0};
    u32 i = 0;
    u32 data_len = 0;
    u16 dev_ID = 0;

    u32 updata_len = 0;//升级包小包长度
    u32 send_len = 0;
    u8 send_buf[100];
//    CRC32_CTX CRC32;
	iap_updata_param_type iap_updata_param_data;
	uint32_t addr_appcopy = FLASH_START_ADDR + Get_FlashSize() * 1024 - FLASH_APP_COPY_ADDR*FLASH_PAGE_SIZE;//FLASH_APP_COPY_ADDR;
    if (len >= CAN_COMM_PROTOCOL_MIN_LEN)
    {
        /*开始解析*/
        i = 0;
        while (1)
        {
            /*同步头、尾不正确或者校验和不正确*/
            if ((buf[i] !=  CAN_COMM_PROTOCOL_HEAD))
            {
                for (i += 1; i < len; i++)
                {
                    if (buf[i] == CAN_COMM_PROTOCOL_HEAD)
                    {
                        break;
                    }
                }
                if (len - i >=  CAN_COMM_PROTOCOL_MIN_LEN)
                {
                    continue;
                }
                else
                {
                    info.state = anylise_reject;
                    info.del_len = i;
                    return info;
                }      
            }
            else
            {
                /*查找数据包长度*/
                data_len = buf[i + 2];
                data_len = (data_len << 8) + buf[i + 1];
                //长度合法判断 目前长度允许8-1200
                if (data_len > CAN_COMM_PROTOCOL_MAX_LEN || data_len < CAN_COMM_PROTOCOL_MIN_LEN)
                {
                    i++;
                    continue;
                }
                /*如果接收数据长度小于帧构成长度 继续等待*/
                if (len - i < data_len)
                {
                    /*i == 0表示正确的帧头前面没有错误数据 返回等待,否则删除错误的数据*/
                    if (i == 0)
                    {
                        info.state = anylise_waiting;
                        info.del_len = 0;
                    }
                    else
                    {
                        info.state = anylise_reject;
                        info.del_len = i;
                    }
                    return info;
                }
                /*检测帧尾,帧尾符合规则 就认为这是一个完整的数据帧*/
                if (buf[i + data_len - 1] == CAN_COMM_PROTOCOL_TAIL && 
                    buf[i + data_len - 2] == Get_Sum8(buf + i + 1, data_len - 3))
                {
                    break;
                }
                else//帧尾校验不符合 删除一个字节 继续查找
                {
                    i += 1;
                    continue;
                }
            }
        }
        //验证通过，进行协议处理
        info.state = anylise_accept;//先预设为接收
        //先筛选出ID 备用
        dev_ID = buf[i + 5];
        dev_ID = (dev_ID << 8) | buf[i + 4];
		
        iap_param.remoteID = buf[i + 7];
        iap_param.remoteID = (iap_param.remoteID << 8) | buf[i + 6];

		if ((iap_param.ID == dev_ID) || (iap_param.ID == 0xFFFF))
		{
			switch (buf[i + 3])
			{
			case CAN_OTA_Recv_Start:
				//解析版本 包长 分包长 总校验 此处开始张波调整协议增加一位6 信道
				OTA_UpdateFlag = 1;
				OTA_Version = buf[i + 9];
				OTA_Version = (OTA_Version << 8) | buf[i + 8];

				OTA_TotalSize = ArrayU8ToU32(buf + i + 12);

				OTA_SubSize = ArrayU8ToU16(buf + i + 16);

				OTA_CRC32 = ArrayU8ToU32(buf + i + 18);

				rt_kprintf("\r\nCAN_OTA_Start...\r\n");

				memcpy(&iap_updata_param_data, buf + i + 8, sizeof(iap_updata_param_data));
				iap_updata_param_data.Version = OTA_Version;

				rt_kprintf("Bin[%d], Sub[%d], CRC32[%08X]\r\n", OTA_TotalSize, OTA_SubSize, OTA_CRC32);
				//存储bin长度+crc32
				Flash_Storage_Header((uint8_t *)&iap_updata_param_data);
				//擦除待写区域
				Flash_Erase_Page(addr_appcopy, OTA_TotalSize);
				
				//回复ACK
				send_len = CAN_OTA_Start_ACK_Construct(send_buf, sizeof(send_buf));
				if (send_len)
				{
					canbus_middle_send(&CAN, 0x500, send_buf, send_len);
				}
				rt_kprintf("Update:");
				break;
			case CAN_OTA_Recv_Update:
				OTA_UpdateFlag = 2;
				//升级包 更新
				OTA_Frame_Num = ArrayU8ToU16(buf + i + 8);
				rt_kprintf("%d,", OTA_Frame_Num + 1);
				if (data_len > 10 && OTA_Frame_Num * OTA_SubSize < OTA_BIN_MAX_SIZE)
				{
					//updata_len应该是4的倍数
					updata_len = data_len - 12;
					Flash_Write(addr_appcopy + (OTA_Frame_Num)* OTA_SubSize, buf + i + 10, updata_len);
					//ack
					send_len = CAN_OTA_Update_ACK_Construct(send_buf, sizeof(send_buf), 0);
					if (send_len)
					{
						canbus_middle_send(&CAN, 0x500, send_buf, send_len);
					}
				}
				else
				{
					send_len = CAN_OTA_Update_ACK_Construct(send_buf, sizeof(send_buf), 1);
					if (send_len)
					{
						canbus_middle_send(&CAN, 0x500, send_buf, send_len);
					}
				}

				break;
			case CAN_OTA_Recv_End:
				rt_thread_delay(1);
				//升级完成 重启更新
				OTA_UpdateFlag = 3;
				//回复ACK
				rt_kprintf("\r\nCAN_OTA_End\r\n");
				send_len = CAN_OTA_End_ACK_Construct(send_buf, sizeof(send_buf));
				if (send_len)
				{
					canbus_middle_send(&CAN, 0x500, send_buf, send_len);
				}
//#ifdef USE_IAP
				UpdateAppMode();
//#endif
				rt_kprintf("IAP_FLAG: 0x%X \n\r", (uint16_t)ReadIAPMode());
				rt_thread_delay(20);
				NVIC_SystemReset();
				break;
				/*********************START**********设备管理/搜索/自检baron***************************************/
			case FUC_PARAMSET_READ:            //设备参数读取			
				send_len = CAN_Paramset_Read_Construct(send_buf, sizeof(send_buf));
				if (send_len)
				{
					canbus_middle_send(&CAN, 0x500, send_buf, send_len);
				}			
				break;
			case FUC_PARAMSET_WRITE:           //设备参数写入
				//具体参数的写入单独设置
				CAN_Paramset_Receive_Analyse(&buf[i]); //将数据存入receive_buf
				send_len = CAN_Paramset_Write_Construct(send_buf, sizeof(send_buf));
				if (send_len)
				{
					canbus_middle_send(&CAN, 0x500, send_buf, send_len);
				}		
				break;
			case FUC_PARAMSET_SAVE:            //参数写入flash			
				//具体参数写入flash单独设置
				stm32_param_save();
				iap_param_save();
				send_len = CAN_Paramset_Save_Construct(send_buf, sizeof(send_buf));
				if (send_len)
				{
					canbus_middle_send(&CAN, 0x500, send_buf, send_len);
				}			
				break;
			case FUC_DEVICE_RESTART:           //重启设备 			
				send_len = CAN_Device_Restart_Construct(send_buf, sizeof(send_buf));
				if (send_len)
				{
					canbus_middle_send(&CAN, 0x500, send_buf, send_len);
				}
				rt_thread_delay(10);
				NVIC_SystemReset();					
				break;
			case FUC_PARAMSET_DEFAULT:         //恢复默认参数		仅恢复app默认参数					
				send_len = CAN_Paramset_Default_Construct(send_buf, sizeof(send_buf));
				if (send_len)
				{
					canbus_middle_send(&CAN, 0x500, send_buf, send_len);
				}
				stm32_param_default();
			break;
			case FUC_DEVICE_SEARCH:            //设备搜索		
				send_len = CAN_Device_Search_Construct(send_buf, sizeof(send_buf));
				if (send_len)
				{		
					if(iap_param.ID == 0xFFFF)
					{
						//无等待
					}
					else
					{
						rt_thread_delay(((iap_param.ID >> 8) & 0xFF) * 10 + (iap_param.ID & 0xFF) * 2);
					}
					canbus_middle_send(&CAN, 0x500, send_buf, send_len);
				}	
				break;
			case FUC_DEVICE_PARAMSET_SEARCH:   //设备搜索参数设置			
				//设备搜索过程仅可以修改id
				CAN_Device_Paramset_Search_SetID(&buf[i]); //将数据存入receive_buf
				send_len = CAN_Device_Paramset_Search_Construct(send_buf, sizeof(send_buf));
				if (send_len)
				{
					canbus_middle_send(&CAN, 0x500, send_buf, send_len);
				}		
			break;
			case FUC_AUTOMATIC_CHECK:          //自动检测		
				send_len = CAN_Automatic_Check_Construct(send_buf, sizeof(send_buf));
				if (send_len)
				{
					canbus_middle_send(&CAN, 0x500, send_buf, send_len);
				}			
				break;
			/*********************END*******************************************************************/				

			default:
				printf("the cmd[%d] is no support!\n", buf[i + 3]);
				break;
			}
		}
		
        

        info.del_len = data_len + i; 
        return info;
    }
    else
    {
        return info;
    } 
}



/***************************************************************************************************
*\Function      can_ota_init
*\Description   
*\Parameter     
*\Return        void
*\Note          
*\Log           2019.03.14    Ver 1.0    庞国瑞
*               创建函数。
***************************************************************************************************/
void can_ota_init(void)
{
	
    RxBufInit(&CAN_BUF500, 1024);

    CAN500_Anylise = comm_dev_create("CAN500T", 1024, can500_buf_recv, can500_buf_del, can500_buf_clear, RT_NULL, 1*RT_TICK_PER_SECOND);
    if (CAN500_Anylise == RT_NULL)
    {
        rt_kprintf("CAN500_Anylise creat fail\n!");
        return;
    }
    //添加具体协议
    protocol_anylise_add(CAN500_Anylise, can500_protocol_anylise);
    protocol_anylise_startup(CAN500_Anylise, rt_thread_self()->current_priority, 1024);	
	
	//can心跳包
	rt_thread_heartbeat_init();
}




















/***************************************************************************************************
******************************************** END OF FILE *******************************************
***************************************************************************************************/


