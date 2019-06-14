
/***************************************************************************************************
*                    (c) Copyright 1992-2019 Syncretize technologies co.,ltd.
*                                       All Rights Reserved
*
*\File          manage_id.c
*\Description   
*\Note          
*\Log           2019.03.14    Ver 1.0    庞国瑞
*               创建文件。
***************************************************************************************************/
#include "manage_id.h"
#include "param.h"
#include "iap_if.h"

//根据PCB在param.h 使用宏定义
//#define VARI3101_SCU_ID 	//*1 平台控制板
//#define VARI3101_DCG_ID 	//*1 驱动控制网关A*1
//#define VARI3101_RCG_ID 	//*1 遥控网关*1
//#define VARI3101_PMU_ID 	//*1 电池管理网关*1
//#define VARI3101_PXU_ID 	//*2 电气连接板*4
//#define VARI3101_GNSS_ID 	//*1 GNSS网关*1
//#define VARI3101_ACCG_ID 	//*2 姿态控制网关*4
//#define VARI3101_BMLG_ID 	//*2 边舱灯组网关*4 左右舱体照明灯组*4
//#define VARI3101_MLDG_ID 	//*2 左右舱体装饰灯组*4
//#define VARI3101_WSG_ID 	//*1 环境检测网关
//#define VARI3101_OAG_ID 	//*2 避障网关*4
//#define VARI3101_DWSM_ID 	//*2 从动轮检测*2
//#define VARI3101_OWSM_ID 	//*2 OES速度传感器*2



/***************************************************************************************************
*\Function      device_id_set
*\Description   //设备ID设置
*\Parameter     
*\Return        void
*\Note          
*\Log           2019.03.14    Ver 1.0    庞国瑞
*               创建函数。
***************************************************************************************************/
void device_id_set(void)
{
	u16 dial_switch_flag = 0;//拨码开关标志位 靠近主动轮为0，远离主动轮为1，现在默认为最大两个同样的板子
	u16 id_increase = 0;
	//预留拨码开关配置
	
	//在pcb未使用拨码开关前使用模拟拨码开关
	dial_switch_flag=dev_param.analog_dial_switch;
	
	//根据不同的工作模式设置不同的id主从模式 主+0；从单个设备+1,多个设备+2
	if (dev_param.mswork_mode == 0)
	{
		id_increase = 0;
	}
	else
	{
		id_increase = 1;
	}

#ifdef VARI3101_SCU_ID
	iap_param.ID = 0x0101 + dial_switch_flag + id_increase;
#endif	

#ifdef VARI3101_DCG_ID
	iap_param.ID = 0x0201 + dial_switch_flag + id_increase;
#endif	

#ifdef VARI3101_RCG_ID
	iap_param.ID = 0x0301 + dial_switch_flag + id_increase;
#endif	

#ifdef VARI3101_PMU_ID
	iap_param.ID = 0x0401 + dial_switch_flag + id_increase;
#endif	

#ifdef VARI3101_PXU_ID
	if (dev_param.mswork_mode == 0)
	{
		id_increase = 0;
	}
	else
	{
		id_increase = 2;
	}
	iap_param.ID = 0x0501 + dial_switch_flag + id_increase;
#endif	

#ifdef VARI3101_GNSS_ID
	iap_param.ID = 0x0601 + dial_switch_flag + id_increase;
#endif	

#ifdef VARI3101_ACCG_ID
	if (dev_param.mswork_mode == 0)
	{
		id_increase = 0;
	}
	else
	{
		id_increase = 2;
	}
	iap_param.ID = 0x0701 + dial_switch_flag + id_increase;
#endif	

#ifdef VARI3101_BMLG_ID
	if (dev_param.mswork_mode == 0)
	{
		id_increase = 0;
	}
	else
	{
		id_increase = 2;
	}
	iap_param.ID = 0x0801 + dial_switch_flag + id_increase;
#endif	

#ifdef VARI3101_MLDG_ID
	if (dev_param.mswork_mode == 0)
	{
		id_increase = 0;
	}
	else
	{
		id_increase = 2;
	}
	iap_param.ID = 0x0901 + dial_switch_flag + id_increase;
#endif	

#ifdef VARI3101_WSG_ID
	iap_param.ID = 0x0A01 + dial_switch_flag + id_increase;
#endif	

#ifdef VARI3101_OAG_ID
	if (dev_param.mswork_mode == 0)
	{
		id_increase = 0;
	}
	else
	{
		id_increase = 2;
	}
	iap_param.ID = 0x0B01 + dial_switch_flag + id_increase;
#endif	

#ifdef VARI3101_DWSM_ID
	if (dev_param.mswork_mode == 0)
	{
		id_increase = 0;
	}
	else
	{
		id_increase = 2;
	}
	iap_param.ID = 0x0C01 + dial_switch_flag + id_increase;
#endif	

#ifdef VARI3101_OWSM_ID
	if (dev_param.mswork_mode == 0)
	{
		id_increase = 0;
	}
	else
	{
		id_increase = 2;
	}
	iap_param.ID = 0x0D01 + dial_switch_flag + id_increase;
#endif	

}

/***************************************************************************************************
******************************************** END OF FILE *******************************************
***************************************************************************************************/


