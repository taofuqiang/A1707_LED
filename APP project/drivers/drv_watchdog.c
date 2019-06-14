/***************************************************************************************************
*                    (c) Copyright 2008-2017  Syncretize technologies co.,ltd.
*										All Rights Reserved
*
*\File          drv_watchdog.c
*\Description   
*\Note          
*\Log           2017.06.01    Ver 1.0    Job
*               创建文件。
***************************************************************************************************/
#include "drv_watchdog.h"

rt_uint32_t Sys_Run_Time;//系统运行时间

/***************************************************************************************************
*\Function      rt_hw_watchdog_init
*\Description   看门狗使能
*\Parameter     
*\Return        void
*\Note          
*\Log           2014.12.22    Ver 1.0    张波
*               创建函数。
***************************************************************************************************/
void rt_hw_watchdog_init(void)
{
	//启动内部LSI
	RCC_LSICmd(ENABLE);
	
	while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY)==RESET);//等待直到LSI稳定

	/* 写入0x5555,用于允许看门狗寄存器写入功能 */
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	
	/* 狗狗时钟分频,40K/256=156HZ(6.4ms)*/
	IWDG_SetPrescaler(IWDG_Prescaler_256);

	/*喂狗时间 （单位ms）= （预分频系数/4）*0.1*RLR（重装载值）
	000 = （256/4）*0.1*RLR
	RLR = 1000/(256/4*0.1) = 156 = 0x9C */	

	/* 喂狗时间 5s/6.4MS=781 .注意不能大于0xfff*/ 

	IWDG_SetReload(0x186);	  //2.5s
	
	/* 喂狗*/
	IWDG_ReloadCounter();
	
	/* 使能狗狗*/
	IWDG_Enable(); 

}

//看门狗任务
void rt_watchdog_thread_entry(void* parameter)
{	
	Sys_Run_Time = 0;
   
	while (1)
	{	
		//定时喂狗   1S一次 
		IWDG_ReloadCounter();
		Sys_Run_Time++;
		//delay
		rt_thread_delay(100);
	}
}

u8 SysRstFlag = 0;//复位标志记录
//得到系统复位的方式,看门狗&软件&掉电
void GetRestFlag(void)
{
    //上电复位标志
    if(RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)
    {
        SysRstFlag = 1;
        //掉电复位
        rt_kprintf("PowerDown RST!\n");
    }
    //看门狗
    else if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET)
    {
        SysRstFlag = 2;
        //看门狗
        rt_kprintf("WatchDog RST!\n");
    }
    //软件复位标志
    else if(RCC_GetFlagStatus(RCC_FLAG_SFTRST) != RESET)
    {
        SysRstFlag = 3;
        //software复位
        rt_kprintf("SoftWare RST!\n");
    }	  	
    //NRST
    else if(RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)
    {
        SysRstFlag = 4;
        //NRst复位
        rt_kprintf("PINRST RST!\n");
    }
    //WWDGRST
    else if(RCC_GetFlagStatus(RCC_FLAG_WWDGRST) != RESET)
    {
        SysRstFlag = 5;
        //WWDGRST复位
        rt_kprintf("WWDGRST RST!\n");
    }
    //LPWRRST
    else if(RCC_GetFlagStatus(RCC_FLAG_LPWRRST) != RESET)
    {
        SysRstFlag = 6;
        //LPWRRST复位
        rt_kprintf("Low Power RST!\n");
    }
    else
    {
        SysRstFlag = 7;
        rt_kprintf("Other RST!\n");
    }
    RCC_ClearFlag();//清楚复位标志

}
void RstFlag(void)
{
    switch(SysRstFlag)
    {
    case 1:
        //掉电复位
        rt_kprintf("PowerDown RST!\n");
        break;
    case 2:
        //看门狗
        rt_kprintf("WatchDog RST!\n");
        break;
    case 3:
        //software复位
        rt_kprintf("SoftWare RST!\n");
        break;
    case 4:
        //NRst复位
        rt_kprintf("PINRST RST!\n");
        break;
    case 5:
        //WWDGRST复位
        rt_kprintf("WWDGRST RST!\n");
        break;
    case 6:
        //Low Power复位
        rt_kprintf("Low Power RST!\n");
        break;
    case 7:
        rt_kprintf("Other RST!\n");
        break;
    default:
        rt_kprintf("RstFlag is Err!\n");
        break;
    }
}
void system_reset(void)
{   
    rt_kprintf("System restarting\n"); 

    rt_thread_delay(1);

    __set_PRIMASK(1);//关闭中断

    NVIC_SystemReset();
}

void runtime(void)
{
    rt_kprintf("SysRunTime = %d Sec", Sys_Run_Time);
}

#ifdef RT_USING_FINSH
#include <finsh.h>

FINSH_FUNCTION_EXPORT(RstFlag, system Reset Flag.)
FINSH_FUNCTION_EXPORT(system_reset, Reset system.)
FINSH_FUNCTION_EXPORT(runtime, sys run time.)

#endif



