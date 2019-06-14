/***************************************************************************************************
*                    (c) Copyright 1992-2015 Syncretize technologies co.,ltd.
*                                       All Rights Reserved
*
*\File          iap_if.c
*\Description   iap相关接口
*\Note          
*\Log           2015.05.27    Ver 1.0    张波
*               创建文件。
***************************************************************************************************/
#include "iap_if.h"
#include "stdio.h"
#include "string.h"
#include "flash_if.h"

char* IAPoE_Name = "stm32_ota";//IAP固件名称
char* IAPoE_Ver = "V1.3";//IAP固件版本
char* IAPoE_Date = __DATE__;//IAP固件发布日期

uint64_t IAP_FLAG;//iap模式标志
iap_param_type iap_param;
/***************************************************************************************************
*\Function      Get_FlashSize
*\Description   获取flash容量
*\Parameter     
*\Return        uint16_t
*\Note          单位KB
*\Log           2015.05.29    Ver 1.0    张波
*               创建函数。
***************************************************************************************************/
uint16_t Get_FlashSize(void)
{
    return *(uint16_t*)(FLASH_SIZE_ADDR);//读取flash大小   
}
/***************************************************************************************************
*\Function      RunAppMode
*\Description   进入正常模式
*\Parameter     
*\Return        void
*\Note          0xFFFFFFFF FFFFFFFF
*\Log           2018.01.18    Ver 1.0    Job               
				创建函数。
***************************************************************************************************/
void RunAppMode(void)
{	
    uint32_t addr_iapflag = FLASH_START_ADDR + Get_FlashSize() * 1024 - FLASH_IAP_FLAG_ADDR*FLASH_PAGE_SIZE;//FLASH_IAP_FLAG_ADDR;
    IAP_FLAG = OTA_RUN_APP;
    /* FLASH 解锁 */
	Flash_Unlock();
    /* 擦除这个扇区 */
	if (FLASH_ErasePage(addr_iapflag) != FLASH_COMPLETE)
	{
		rt_kprintf("\r page 0x%08X erase fail!\n\r", addr_iapflag);
	}	
    /* FLASH 上锁 */
    FLASH_Lock();			
}
/***************************************************************************************************
*\Function      OutIAPMode
*\Description   进入更新App模式
*\Parameter     
*\Return        void
*\Note          
*\Log           2015.05.27    Ver 1.0    张波
*               创建函数。
***************************************************************************************************/
void UpdateAppMode(void)
{
    uint32_t addr_iapflag = FLASH_START_ADDR + Get_FlashSize() * 1024 - FLASH_IAP_FLAG_ADDR*FLASH_PAGE_SIZE;//FLASH_IAP_FLAG_ADDR;

    //不是0xFFFFFFFFFFFF..则擦除
    if (ReadIAPMode() != OTA_RUN_APP)
    {
        RunAppMode();
    }
    /* FLASH 解锁 */
	Flash_Unlock();
    IAP_FLAG = OTA_Update_APP;
	FLASH_WaitForLastOperation((uint32_t)0x00002000);
	FLASH_ProgramWord(addr_iapflag, IAP_FLAG);
	FLASH_ProgramWord(addr_iapflag+2, IAP_FLAG>>16);
//	FLASH_ProgramHalfWord(addr_iapflag, IAP_FLAG);

    /* FLASH 上锁 */
    FLASH_Lock();
	
}

/***************************************************************************************************
*\Function      ForceUpdateAppMode
*\Description   强制更新模式
*\Parameter     
*\Return        void
*\Note          
*\Log           2018.01.18    Ver 1.0    Job               
				创建函数。
***************************************************************************************************/
void ForceUpdateAppMode(void)
{
    uint32_t addr_iapflag = FLASH_START_ADDR + Get_FlashSize() * 1024 - FLASH_IAP_FLAG_ADDR*FLASH_PAGE_SIZE;//FLASH_IAP_FLAG_ADDR;

    //不是0xFFFFFFFFFFFF..则擦除
    if (ReadIAPMode() != OTA_RUN_APP || ReadIAPMode() != OTA_Update_APP)
    {
        RunAppMode();
    }
    /* FLASH 解锁 */
	Flash_Unlock();
    IAP_FLAG = OTA_Force_Update_APP;
	FLASH_ProgramWord(addr_iapflag, IAP_FLAG);
	FLASH_ProgramWord(addr_iapflag+2, IAP_FLAG>>16);
    /* FLASH 上锁 */
    FLASH_Lock();
}
/***************************************************************************************************
*\Function      ReadIAPMode
*\Description   读取IAP_FLAG标志
*\Parameter     
*\Return        uint16_t
*\Note          
*\Log           2015.05.29    Ver 1.0    张波
*               创建函数。
***************************************************************************************************/
uint64_t ReadIAPMode(void)
{
    return *(uint64_t*)(FLASH_START_ADDR + Get_FlashSize() * 1024 - FLASH_IAP_FLAG_ADDR*FLASH_PAGE_SIZE);
}

/***************************************************************************************************
*\Function      Get_ChipID
*\Description   获取uuid
*\Parameter     chipID
*\Return        void
*\Note          
*\Log           2018.07.17    Ver 1.0    庞国瑞
*               创建函数。
***************************************************************************************************/
void Get_ChipID(uint16_t* chipID)
{
    chipID[0] = *(__IO uint16_t *)(0X1FFFF7E8);  // 低字节
    chipID[1] = *(__IO uint16_t *)(0X1FFFF7EA); //  
    chipID[2] = *(__IO uint16_t *)(0X1FFFF7EC); // 高字节
	chipID[3] = *(__IO uint16_t *)(0X1FFFF7EE);  // 低字节
    chipID[4] = *(__IO uint16_t *)(0X1FFFF7F0); //  
    chipID[5] = *(__IO uint16_t *)(0X1FFFF7F2); // 高字节
}


/***************************************************************************************************
*\Function      iap_param_save
*\Description   iap参数存储
*\Parameter     
*\Return        void
*\Note          
*\Log           2018.07.17    Ver 1.0    庞国瑞
*               创建函数。
***************************************************************************************************/
void iap_param_save(void)
{

    uint32_t addr_iapparam = FLASH_START_ADDR + Get_FlashSize() * 1024 - FLASH_IAP_PARAM_ADDR*FLASH_PAGE_SIZE;//FLASH_IAP_PARAM_ADDR;

    /* FLASH 解锁 */
	Flash_Unlock();
    /* 擦除这个扇区 */
	if (FLASH_ErasePage(addr_iapparam) != FLASH_COMPLETE)
	{
		rt_kprintf("\r page 0x%08X erase fail!\n\r", addr_iapparam);
	}		
	
    Flash_Write(addr_iapparam, (uint8_t*)&iap_param,sizeof(iap_param));
    addr_iapparam += sizeof(iap_param);

    /* FLASH 上锁 */
    FLASH_Lock();

}

/***************************************************************************************************
*\Function      iap_param_default
*\Description   iap默认参数
*\Parameter     
*\Return        void
*\Note          
*\Log           2018.07.17    Ver 1.0    庞国瑞
*               创建函数。
***************************************************************************************************/
void iap_param_default(void)
{
    memset(&iap_param, 0, sizeof(iap_param));
	Get_ChipID(iap_param.ChipUniqueID);	
	iap_param.Power_First_Flag = DEV_FIRST_POWER_FLAG;   
	iap_param.swversion = 0x01;
	iap_param.hwversion = 0x01;
	iap_param.IAP_flag = 0x01;
	iap_param.remoteID =0x00;
	iap_param.ID=0xFFFF;
  iap_param_save();
}

/***************************************************************************************************
*\Function      iap_param_load
*\Description   iap参数加载
*\Parameter     
*\Return        void
*\Note          
*\Log           2018.07.17    Ver 1.0    庞国瑞
*               创建函数。
***************************************************************************************************/
void iap_param_load(void)
{
    uint32_t addr_iapparam =  FLASH_START_ADDR + Get_FlashSize() * 1024 - FLASH_IAP_PARAM_ADDR*FLASH_PAGE_SIZE;//FLASH_IAP_PARAM_ADDR;

    iap_param.Power_First_Flag = *(__IO uint16_t*)(addr_iapparam);
    if (iap_param.Power_First_Flag != DEV_FIRST_POWER_FLAG)
    {     
		rt_kprintf("IAP Load Default Param...\r\n");
        //如果第一次上电，则默认值
        iap_param_default(); //调用默认参数,擦除FLASH之后调用
    }
    else
    {
        rt_kprintf("IAP Load Param... \r\n");
    }

    addr_iapparam =   FLASH_START_ADDR + Get_FlashSize() * 1024 - FLASH_IAP_PARAM_ADDR*FLASH_PAGE_SIZE;//FLASH_IAP_PARAM_ADDR;
    Flash_Read(addr_iapparam, (uint8_t*)&iap_param, sizeof(iap_param));
    addr_iapparam += sizeof(iap_param);
	
	iap_param.IAP_flag = 0x02;
}





