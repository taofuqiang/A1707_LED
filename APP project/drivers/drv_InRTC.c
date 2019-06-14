/***************************************************************************************************
*                    (c) Copyright 2008-2017  Syncretize technologies co.,ltd.
*										All Rights Reserved
*
*\File          drv_InRTC.c
*\Description   
*\Note          因为使用RTC，rtc备份域就可以保存数据，所以将rtc第一次初始化标志放在rtc备份域中
*               RTC备份域和BKPSRAM写保护去除是不一样的
*\Log           2017.06.01    Ver 1.0    Job
*               创建文件。
***************************************************************************************************/
#include "drv_InRTC.h"

/***************************************************************************************************
*\Function      stm32_rtc_init
*\Description   rtc时钟初始化
*\Parameter     first,如果为TRUE，则强制进行一次初始化,将时间设置为2000-1-1 0:0:0
*\Return        void
*\Note          1、系统在第一次上电时候初始化
*\Log           2014.11.07    Ver 1.0    张波
*               创建函数。
***************************************************************************************************/
void stm32_Inrtc_init(bool first)
{
    u32 count = 0;

    /*使能电源和后备接口时钟*/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

    /*设置寄存器PWR_CR的DBP位，使能对后备寄存器和RTC的访问*/
    PWR_BackupAccessCmd(ENABLE);
    /*复位备份域*/
    if (first || BKP_ReadBackupRegister(BKP_DR1) != 0x1234)
    {
        BKP_DeInit();//只有第一次上电复位bkp
        /*使能LSI时钟*/
		RCC_LSICmd(ENABLE);
//        RCC_LSEConfig(RCC_LSE_ON);
        /*等待LSE变的有效*/
        while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
        {
            if (count++ > 5)
            {
                rt_kprintf("InRTC init fail!\n");
                return;
            }
            rt_thread_delay(RT_TICK_PER_SECOND);
        }

        /*设置RTC的时钟源为LSI*/
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
        /*RTC时钟使能*/
        RCC_RTCCLKCmd(ENABLE);

        RTC_WaitForSynchro();
        /*等待RTC最后一次操作完成*/
        RTC_WaitForLastTask();

        /*设置预分频计数器的值*/
        RTC_SetPrescaler(0x7fff); /*RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(0x7fff+1)*/
        /*等待操作完成*/
        RTC_WaitForLastTask();
        
        //初始时间设定
        RTC_SetCounter(0);
        /*等待操作完成*/
        RTC_WaitForLastTask();

        BKP_WriteBackupRegister(BKP_DR1, 0x1234); 
        rt_kprintf("InRTC initialized Firstly...\n");
    }  
    else
    {
        rt_kprintf("InRTC initialized...\n");
    }
}

/***************************************************************************************************
*\Function      stm32_rtc_getTime
*\Description   获取dec时间
*\Parameter     time
*\Return        bool
*\Note          
*\Log           2014.11.10    Ver 1.0    张波
*               创建函数。
***************************************************************************************************/
bool stm32_Inrtc_getTime(TIME* time)
{
    u32 stamp = 0;

    stamp = RTC_GetCounter();
    *time = TimeDECStampToTime(stamp);
    return TRUE;
}
/***************************************************************************************************
*\Function      stm32_rtc_setTime
*\Description   设置DEC时间
*\Parameter     time
*\Return        bool
*\Note          
*\Log           2014.11.10    Ver 1.0    张波
*               创建函数。
***************************************************************************************************/
bool stm32_Inrtc_setTime(TIME time)
{
    u32 stamp = 0;

    stamp = TimeDECToStamp(time);
    RTC_SetCounter(stamp);
    /*等待操作完成*/
    RTC_WaitForLastTask();
    return TRUE;
}

