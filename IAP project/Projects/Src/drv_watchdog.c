/***************************************************************************************************
*                    (c) Copyright 1992-2015 Syncretize technologies co.,ltd.
*                                       All Rights Reserved
*
*\File          watchdog.c
*\Description   
*\Note          
*\Log           2015.05.30    Ver 1.0    张波
*               创建文件。
***************************************************************************************************/
#include "drv_watchdog.h"
#include "stm32f1xx.h"
#include "stdio.h"
#include "stm32f1xx_hal.h"
#include "stm32_Inrtc.h"

#define WATCHDOG_TIMER_INTERVAL     (1000)//喂狗间隔1000ms
#define AUTORESET_TIMER_INTERVAL    (1000 * 60 * 10)//30分钟

uint32_t WatchdogTimer = 0;
static IWDG_HandleTypeDef IwdgHandle;
int64_t Sys_Run_Time=0;//系统运行时间 s 计算到s
/***************************************************************************************************
*\Function      watchdog_init
*\Description   看门狗初始化
*\Parameter     8s复位
*\Return        void
*\Note          
*\Log           2015.05.30    Ver 1.0    张波
*               创建函数。
***************************************************************************************************/
void  watchdog_init(void)
{
    RCC_OscInitTypeDef oscinit = {0};

    /* Enable LSI Oscillator */
    oscinit.OscillatorType = RCC_OSCILLATORTYPE_LSI;
    oscinit.LSIState = RCC_LSI_ON;
    oscinit.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&oscinit)!= HAL_OK)
    {
        printf("LSI init fail!\n");
    }

    /*##-3- Configure the IWDG peripheral ######################################*/	
    IwdgHandle.Instance = IWDG;
    IwdgHandle.Init.Prescaler = IWDG_PRESCALER_128;
    IwdgHandle.Init.Reload    = 4000;//0-0xFFF

    if (HAL_IWDG_Init(&IwdgHandle) != HAL_OK)
    {
        /* Initialization Error */
        printf("watchdog init fail!\n");
    }
}

/***************************************************************************************************
*\Function      Watchdog_Refresh
*\Description   喂狗动作
*\Parameter     
*\Return        void
*\Note          
*\Log           2018.05.23    Ver 1.0    Job               
				创建函数。
***************************************************************************************************/
void Watchdog_Refresh(void)
{
    HAL_IWDG_Refresh(&IwdgHandle);
}

u8 SysRstFlag = 0;//复位标志记录
//得到系统复位的方式,看门狗&软件&掉电
void GetRestFlag(void)
{
	
    //看门狗
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST) != RESET)
    {
        SysRstFlag = 1;
        //看门狗
        printf("WatchDog RST!\n\r");
    }
    //软件复位标志
    else if(__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST) != RESET)
    {
        SysRstFlag = 2;
        //software复位
        printf("SoftWare RST!\n\r");
    }	  	
    //NRST
    else if(__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST) != RESET)
    {
        SysRstFlag = 3;
        //NRst复位
        printf("PINRST RST!\n\r");
    }
    //WWDGRST
    else if(__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST) != RESET)
    {
        SysRstFlag = 4;
        //WWDGRST复位
        printf("WWDGRST RST!\n\r");
    }
    //LPWRRST
    else if(__HAL_RCC_GET_FLAG(RCC_FLAG_LPWRRST) != RESET)
    {
        SysRstFlag = 5;
        //LPWRRST复位
        printf("Low Power RST!\n\r");
    }
    else if(__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST) != RESET)
    {
        SysRstFlag = 6;
        //PORRST复位
        printf("POR RST!\n\r");
    }
    else
    {
        SysRstFlag = 7;
        printf("Other RST!\n\r");
    }
    __HAL_RCC_CLEAR_RESET_FLAGS();//清楚复位标志
}

extern void system_reset(void);
//看门狗喂狗 时间数据累加
void watchdog_refresh_and_count(void)
{
	TIME_SubSec run_time_rtc;//
	stm32_Inrtc_getTime(&run_time_rtc);
	Watchdog_Refresh();//看门狗喂狗
	Sys_Run_Time +=1;	 //单位s
//	printf("run time %d s\n\r",(int)Sys_Run_Time);
	if( Sys_Run_Time>(10*60))//10min 超过10min 系统自动重启
	{
		Sys_Run_Time=0;
		system_reset();
	}	
	
//	printf("%d-%02d-%02d--%02d:%02d:%02d\n\r",(run_time_rtc.T.Year+2000),run_time_rtc.T.Month,run_time_rtc.T.Day,
//	run_time_rtc.T.Hour,run_time_rtc.T.Minute,run_time_rtc.T.Second);					 
}

//上电打印版本信息
void show_version(void)
{							
	Get_ChipID(iap_param.ChipUniqueID);
    printf("\n\n\r STM32_OTA is Entry! \n\r ");
    printf("\r DeviceName      : OTAoE.%d.%d \n\r",(char)(0x0103 >> 8), (char)0x0103);
    printf("\r Release Date    : %s \n\r",__DATE__ );
    printf("\r Firmware version: %d \n\r", iap_param.swversion );
    printf("\r Flash Size      : %d KB\n\r", Get_FlashSize());
    printf("\r IAP_FLAG        : %d \n\r", iap_param.IAP_flag);
    printf("\r UUID            : %02X-%02X-%02X-%02X-%02X-%02X \n\r", iap_param.ChipUniqueID[0], iap_param.ChipUniqueID[1], iap_param.ChipUniqueID[2],iap_param.ChipUniqueID[3], iap_param.ChipUniqueID[4], iap_param.ChipUniqueID[5]); 
	printf("\r ID              : 0x%02X \n\r", iap_param.ID);
}


