/***************************************************************************************************
*                    (c) Copyright 2008-2018  Syncretize technologies co.,ltd.
*										All Rights Reserved
*
*\File          stm32_Inrtc.c
*\Description   
*\Note          
*\Log           2018.05.07    Ver 1.0    Job
*               创建文件。
***************************************************************************************************/
#include "stm32_Inrtc.h"

//#define RTC_CALIB_OUT_EN //校准输出使能

//第一次上电标记 记录在备份域中
#define RTC_FIRST_FLAG_DR1 0x12345678

//RTC时钟分频定义
/* subsecond number of bits */
#define N_PREDIV_S                 10
/* Synchonuous prediv  */
#define PREDIV_S                  ((1<<N_PREDIV_S)-1)
/* Asynchonuous prediv   */
#define PREDIV_A                  (1<<(15-N_PREDIV_S))-1

//自动唤醒单元 唤醒时间为1ms至32秒
#define MIN_WU_TIME    1
#define MAX_WU_TIME    32000 /* 32 = 65536 / (32768 / 16) */

/* Convert miliseconds to wake up counter. */
#define mSEC_2_WU_CNT(ms)    (32768ul / 16 * (ms) / 1000) /* the MAX=32s */


RTC_HandleTypeDef RtcHandle;
/***************************************************************************************************
*\Function      stm32_Inrtc_init
*\Description    first,如果为TRUE，则强制进行一次初始化,将时间设置为2010-1-1 0:0:0
*\Parameter     force
*\Return        void
*\Note          1、系统在第一次上电时候初始化
*\Log           2018.05.07    Ver 1.0    Job               
				创建函数。
***************************************************************************************************/
#define WAKEUP_TIMER_ENABLE 0x32F2
void stm32_Inrtc_init(bool force)
{
    TIME_SubSec time = {INIT_TIME, 0};
    RCC_OscInitTypeDef        RCC_OscInitStruct;
    RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;

    /*##-1- Enables the PWR Clock and Enables access to the backup domain ###################################*/
    /* To change the source clock of the RTC feature (LSE, LSI), You have to:
    - Enable the power clock using __HAL_RCC_PWR_CLK_ENABLE()
    - Enable write access using HAL_PWR_EnableBkUpAccess() function before to 
    configure the RTC clock source (to be done once after reset).
    - Reset the Back up Domain using __HAL_RCC_BACKUPRESET_FORCE() and 
    __HAL_RCC_BACKUPRESET_RELEASE().
    - Configure the needed RTc clock source */
    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();

    //先初始化结构体，读备份区域的时候使用
     /*##-1- Configure the RTC peripheral #######################################*/
    /* Configure RTC prescaler and RTC data registers */
    /* RTC configured as follows:
    - Hour Format    = Format 24
    - Asynch Prediv  = Value according to source clock
    - Synch Prediv   = Value according to source clock
    - OutPut         = Output Disable
    - OutPutPolarity = High Polarity
    - OutPutType     = Open Drain */ 
    RtcHandle.Instance = RTC; 
	RtcHandle.Init.AsynchPrediv = RTC_AUTO_1_SECOND;

    if (force || HAL_RTCEx_BKUPRead(&RtcHandle, RTC_BKP_DR1) != RTC_FIRST_FLAG_DR1)
    {
        HAL_RTC_DeInit(&RtcHandle);

        /*##-2- Configure LSI as RTC clock source ###################################*/
		HAL_RCC_GetOscConfig(&RCC_OscInitStruct);

		RCC_OscInitStruct.OscillatorType =  RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
		RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
//		RCC_OscInitStruct.PLL2.PLL2State = RCC_PLL2_NONE;
		RCC_OscInitStruct.LSIState = RCC_LSI_ON;
		RCC_OscInitStruct.LSEState = RCC_LSE_OFF;
		if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
		{
			printf("RCC LSI init fail!\n\r");
//			Error_Handler();
		}		
		
		PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
		PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
		if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
		{ 
			printf("RTC LSI error!\n\r");
//			Error_Handler();
		}		
		
        /*##-3- Enable RTC peripheral Clocks #######################################*/
        /* Enable RTC Clock */
        __HAL_RCC_RTC_ENABLE();

        if (HAL_RTC_Init(&RtcHandle) != HAL_OK)
        {
            /* Initialization Error */
            printf("RTC init error!\n");
        }
        stm32_Inrtc_setTime(time);

        /*Enable Direct Read of the calendar registers (not through Shadow) */
//        HAL_RTCEx_EnableBypassShadow(&RtcHandle);
	/*##-2- Check if data stored in BackUp register2: Wakeup timer enable #######*/
	/* Read the Back Up Register 2 Data */
	if (HAL_RTCEx_BKUPRead(&RtcHandle, RTC_BKP_DR2) == WAKEUP_TIMER_ENABLE)
	{
		/* if the wakeup timer is enabled then desable it to disable the wakeup timer interrupt */
		if(HAL_RTCEx_DeactivateSecond(&RtcHandle) != HAL_OK)
		{
			  /* Initialization Error */
			printf("RTC DS error!\n");
//			  Error_Handler(); 
		}
	}		
		
		HAL_RTCEx_SetSecond_IT(&RtcHandle);	
		HAL_RTCEx_BKUPWrite(&RtcHandle, RTC_BKP_DR2, WAKEUP_TIMER_ENABLE);
	
        HAL_RTCEx_BKUPWrite(&RtcHandle, RTC_BKP_DR1, RTC_FIRST_FLAG_DR1);
//        printf("RTC init ...\n\r");
    }
    else
    {
//         printf("RTC init\n\r");
    }
	/*##-3- Configure the NVIC for RTC Alarm ###################################*/
	HAL_NVIC_SetPriority(RTC_IRQn, 0x0, 0);
	/* Enable the RTC global Interrupt */
	HAL_NVIC_EnableIRQ(RTC_IRQn);
}

/***************************************************************************************************
*\Function      stm32_Inrtc_setTime
*\Description   设置DEC时间
*\Parameter     SetT
*\Return        bool
*\Note          1、因为亚秒是只读的，不能设置 
*               2、设置时间时 亚秒会自动清零 所以使用偏移设置亚秒部分
*\Log           2018.05.07    Ver 1.0    Job               
				创建函数。
***************************************************************************************************/
bool stm32_Inrtc_setTime(TIME_SubSec SetT)
{
    RTC_TimeTypeDef times;
    RTC_DateTypeDef date;
    TIME time = SetT.T;

    if (!TimeDECCheck(&time))
    {
        return FALSE;
    }
    /*时间格式*/
    times.Seconds   = time.Second;
    times.Minutes   = time.Minute;
    times.Hours     = time.Hour;
//    times.TimeFormat = RTC_HOURFORMAT_24;
//    times.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
//    times.StoreOperation = RTC_STOREOPERATION_RESET;

    /*获取星期几*/
    date.WeekDay    = TimeDECWeekDay(time) + 1;
    date.Date       = time.Day;
    date.Month      = time.Month;
    date.Year       = time.Year;

    if (HAL_RTC_SetDate(&RtcHandle, &date, RTC_FORMAT_BIN) != HAL_OK)
    {
        return FALSE;
    }

    if(HAL_RTC_SetTime(&RtcHandle, &times, RTC_FORMAT_BIN) != HAL_OK)
    {
        return FALSE;
    }

//    if (stm32_Inrtc_msShift((s16)SetT.SubSecs) == FALSE)
//    {
//        return FALSE;
//    }

    return TRUE;
}

/***************************************************************************************************
*\Function      stm32_Inrtc_getTime
*\Description   
*\Parameter     getT
*\Return        bool
*\Note          
*\Log           2018.05.07    Ver 1.0    Job               
				创建函数。
***************************************************************************************************/
bool stm32_Inrtc_getTime(TIME_SubSec* getT)
{
    RTC_TimeTypeDef times;
    RTC_DateTypeDef date;
    u32 first_read;
#if (N_PREDIV_S != 10)
    #error "you must understand your set value!"
#else
    /* Get Time and Date*/
    HAL_RTC_GetTime(&RtcHandle, &times, RTC_FORMAT_BIN );

    //读两次是为了保证时间没有发生跳变，该处的前提是1个subsecond计数内可以执行完成
    /* make sure it is correct due to asynchronus nature of RTC*/
    do
    {
        first_read =0;// times.SubSeconds;
        HAL_RTC_GetDate( &RtcHandle, &date, RTC_FORMAT_BIN );
        HAL_RTC_GetTime( &RtcHandle, &times, RTC_FORMAT_BIN );
    } while (first_read !=0 );//times.SubSeconds
#endif
    getT->T.Second    = times.Seconds;
    getT->T.Hour      = times.Hours;
    getT->T.Minute    = times.Minutes;

    getT->T.Day       = date.Date;
    getT->T.Month     = date.Month;
    getT->T.Year      = date.Year;

    //如果正在平移，此处毫秒值可能不正确
    getT->SubSecs = 0;//(u16)((times.SecondFraction - times.SubSeconds) * (uint32_t)1000 /(times.SecondFraction + 1));//毫秒部分
    /*检查合法性*/
    if (!TimeDECCheck(&getT->T))
    {
        return FALSE;
    }

    return TRUE;
}

/***************************************************************************************************
*\Function      stm32_Inrtc_EnWakeup
*\Description   使能自动唤醒
*\Parameter     ms
*\Return        bool
*\Note          1ms-32秒 但是最好不要用太小的间隔 需要中断则自行添加中断处理函数
*\Log           2018.05.08    Ver 1.0    Job               
				创建函数。
***************************************************************************************************/
bool stm32_Inrtc_EnWakeup(u16 ms)
{
    if (!(MIN_WU_TIME <= ms && ms <= MAX_WU_TIME))
    {
        return FALSE;
    }

//    __HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&RtcHandle, RTC_FLAG_WUTF);

//    HAL_RTCEx_SetWakeUpTimer_IT(&RtcHandle, mSEC_2_WU_CNT(ms), RTC_WAKEUPCLOCK_RTCCLK_DIV16);

    return TRUE;
}
/***************************************************************************************************
*\Function      stm32_Inrtc_DisWakeup
*\Description   禁能自动唤醒
*\Parameter     
*\Return        bool
*\Note          
*\Log           2018.05.08    Ver 1.0    Job               
				创建函数。
***************************************************************************************************/
bool stm32_Inrtc_DisWakeup(void)
{
//    if (HAL_RTCEx_DeactivateWakeUpTimer(&RtcHandle) != HAL_OK)
//    {
//        return FALSE;
//    }

//    __HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&RtcHandle, RTC_FLAG_WUTF);

    return TRUE;
}

void RTC_IRQHandler(void)
{
  HAL_RTCEx_RTCIRQHandler(&RtcHandle);
}




