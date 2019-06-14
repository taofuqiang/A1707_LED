/***************************************************************************************************
*                    (c) Copyright 2008-2017  Syncretize technologies co.,ltd.
*										All Rights Reserved
*
*\File          drv_adc.c
*\Description   
*\Note          
*\Log           2017.05.11    Ver 1.0    Job
*               创建文件。
***************************************************************************************************/
#include "drv_adc.h"
#include "common_lib.h"

/***************************************************************************************************
*\Function      stm32_adc_with_dma_init
*\Description   初始化adc dma配置，请没有启动数据传输
*\Parameter     adc_dev
*\Return        bool
*\Note          
*\Log           2016.11.16    Ver 1.0    Job               
创建函数。
***************************************************************************************************/
bool stm32_adc_with_dma_init(ADC_DMA_Type* adc_dev)
{
    DMA_InitTypeDef DMA_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;

    if (adc_dev == RT_NULL || adc_dev->dma == RT_NULL || adc_dev->adc == RT_NULL
        || adc_dev->adc_data == RT_NULL)
    {
        return FALSE;
    }

    if (adc_dev->dma == DMA1_Channel1 || adc_dev->dma == DMA1_Channel2|| adc_dev->dma == DMA1_Channel3||
        adc_dev->dma == DMA1_Channel4 || adc_dev->dma == DMA1_Channel5 || adc_dev->dma == DMA1_Channel6|| 
        adc_dev->dma == DMA1_Channel7)
    {
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    }
    else if(adc_dev->dma == DMA2_Channel1 || adc_dev->dma == DMA2_Channel2|| adc_dev->dma == DMA2_Channel3||
        adc_dev->dma == DMA2_Channel4 || adc_dev->dma == DMA2_Channel5)
    {
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
    }
    else
    {
        return FALSE;
    }

    // 允许ADC
    RCC_ADCCLKConfig(RCC_PCLK2_Div4);   
    //adc时钟使能
    if (adc_dev->adc == ADC1)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);
    }
    else if (adc_dev->adc == ADC2)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2,ENABLE);
    }
    else if (adc_dev->adc == ADC3)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3,ENABLE);
    }
    else
    {
        return FALSE;
    }
    // DMA1 CHANNEL1
    DMA_DeInit(adc_dev->dma);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&adc_dev->adc->DR;            //ADC1外设地址
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)adc_dev->adc_data;                     //内存地址
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;                                 //dma传输方向
    DMA_InitStructure.DMA_BufferSize = adc_dev->data_len;                              //设置DMA在传输时缓冲区的长度 word
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;                   //设置DMA的外设递增模式，一个外设
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                            //设置DMA的内存递增模式，
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;        //外设数据字长
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;                //内存数据字长
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                                    //设置DMA的传输模式：连续不断的循环模式
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;                                 //设置DMA的优先级别
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                              
    DMA_Init(adc_dev->dma, &DMA_InitStructure);
    // Enable DMA1 channel1
    DMA_ITConfig(adc_dev->dma, DMA_IT_TC, DISABLE);//暂时禁能dma传输完成中断
    DMA_Cmd(adc_dev->dma, ENABLE);

    // ADC1 configuration ------------------------------------------------------
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;                   //独立工作模式
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;                         //扫描方式
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;                   //连续转换
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;  //外部触发禁止
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;               //数据右对齐
    ADC_InitStructure.ADC_NbrOfChannel = adc_dev->max_num;                //用于转换的通道数
    ADC_Init(adc_dev->adc, &ADC_InitStructure);

    //使能内部温度传感器的参考电压 
    ADC_TempSensorVrefintCmd(ENABLE); 

    //允许ADC1的DMA模式
    ADC_DMACmd(adc_dev->adc, ENABLE);

    //允许ADC1
    ADC_Cmd(adc_dev->adc, ENABLE);

    //重置校准寄存器   
    ADC_ResetCalibration(adc_dev->adc);
    while(ADC_GetResetCalibrationStatus(adc_dev->adc));

    //开始校准状态
    ADC_StartCalibration(adc_dev->adc);
    while(ADC_GetCalibrationStatus(adc_dev->adc));

    return TRUE;
}
/***************************************************************************************************
*\Function      stm32_adc_channel_add
*\Description   添加adc通道
*\Parameter     adc_dev
*\Parameter     seq_num ，序号 从0 开始内部+1
*\Parameter     channel，通道
*\Parameter     sample，采样时间
*\Return        bool
*\Note          1、注意rank是从1开始的
*\Log           2016.11.16    Ver 1.0    Job               
创建函数。
***************************************************************************************************/
bool stm32_adc_channel_add(ADC_DMA_Type* adc_dev,u32 seq_num, u32 channel, u32 sample, GpioType* port)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    if (adc_dev == RT_NULL || adc_dev->adc_count >= 16)
    {
        return FALSE;
    }

    if (port != RT_NULL)
    {
        //使能端口时钟
        switch((u32)port->gpio)
        {
        case GPIOA_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
            break;
        case GPIOB_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
            break;
        case GPIOC_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
            break;
        case GPIOD_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
            break;
        case GPIOE_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
            break;
        case GPIOF_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);
            break;
        case GPIOG_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, ENABLE);
            break;
        default:
            break;
        }

        //初始化gpio引脚
        GPIO_InitStruct.GPIO_Pin   = port->gpio_pin;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

        GPIO_Init(port->gpio, &GPIO_InitStruct); 
    }
    /*##-2- Configure ADC regular channel ######################################*/
    ADC_RegularChannelConfig(adc_dev->adc, channel, seq_num + 1, sample);
    adc_dev->adc_count++;
    return TRUE;
}
/***************************************************************************************************
*\Function      stm32_adc_with_dma_start
*\Description   启动dma数据传输
*\Parameter     adc_dev
*\Return        bool
*\Note          
*\Log           2016.11.16    Ver 1.0    Job               
				创建函数。
***************************************************************************************************/
bool stm32_adc_with_dma_start(ADC_DMA_Type* adc_dev)
{
    if (adc_dev == RT_NULL)
    {
        return FALSE;
    }

    //人工打开ADC转换
    ADC_SoftwareStartConvCmd(adc_dev->adc, ENABLE);
    return TRUE;
}

/***************************************************************************************************
*\Function      stm32_adc_get_value
*\Description   获取adc数据，采取滤波算法之后的数据
*\Parameter     adc_dev
*\Parameter     seq_num 从0开始
*\Return        u16
*\Note          1、暂时不添加过滤算法，看下效果
*\Log           2016.11.16    Ver 1.0    Job               
				创建函数。
***************************************************************************************************/
u16 stm32_adc_get_value(ADC_DMA_Type* adc_dev, u32 seq_num)
{
    u32 i = 0;
    u32 adc_sum = 0;
    u16 adc_avg = 0;

    if (adc_dev == RT_NULL)
    {
        return FALSE;
    }
    
    for (i = 0; i < adc_dev->data_len / adc_dev->max_num; i++)
    {
        adc_sum += adc_dev->adc_data[i * adc_dev->max_num + seq_num];
    }

    adc_avg = adc_sum / (adc_dev->data_len / adc_dev->max_num);

    return adc_avg;
}

/***************************************************************************************************
*\Function      get_chip_temp
*\Description   转换芯片温度
*\Parameter     
*\Return        double
*\Note          计算公式（3.3*Vadc/4096 - 0.76)*1000/ 2.5 + 25
*\Log           2016.07.01    Ver 1.0    job
*               创建函数。
***************************************************************************************************/
u32 stm32_get_chip_temp(u16 value)
{
    double temp = 0.0;

    temp= (1.42 - value * 3.3 / 4096) * 1000 / 4.3 + 25;

    return (u32)temp;
}
/***************************************************************************************************
不使用DMA的接口
***************************************************************************************************/
#if 0
bool stm32_adc_init(ADC_DMA_Type* adc_dev)
{
    ADC_InitTypeDef ADC_InitStructure;

    if (adc_dev == RT_NULL || adc_dev->dma == RT_NULL || adc_dev->adc == RT_NULL
        || adc_dev->adc_data == RT_NULL)
    {
        return FALSE;
    }

    if (adc_dev->dma == DMA1_Channel1 || adc_dev->dma == DMA1_Channel2|| adc_dev->dma == DMA1_Channel3||
        adc_dev->dma == DMA1_Channel4 || adc_dev->dma == DMA1_Channel5 || adc_dev->dma == DMA1_Channel6|| 
        adc_dev->dma == DMA1_Channel7)
    {
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    }
    else if(adc_dev->dma == DMA2_Channel1 || adc_dev->dma == DMA2_Channel2|| adc_dev->dma == DMA2_Channel3||
        adc_dev->dma == DMA2_Channel4 || adc_dev->dma == DMA2_Channel5)
    {
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
    }
    else
    {
        return FALSE;
    }

    // 允许ADC
    RCC_ADCCLKConfig(RCC_PCLK2_Div4);   
    //adc时钟使能
    if (adc_dev->adc == ADC1)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);
    }
    else if (adc_dev->adc == ADC2)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2,ENABLE);
    }
    else if (adc_dev->adc == ADC3)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3,ENABLE);
    }
    else
    {
        return FALSE;
    }
    // ADC1 configuration ------------------------------------------------------
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;                   //独立工作模式
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;                         //扫描方式
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;                   //连续转换
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;  //外部触发禁止
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;               //数据右对齐
    ADC_InitStructure.ADC_NbrOfChannel = 1;                //用于转换的通道数
    ADC_Init(adc_dev->adc, &ADC_InitStructure);

    //使能内部温度传感器的参考电压 
    ADC_TempSensorVrefintCmd(ENABLE); 

    //允许ADC1
    ADC_Cmd(adc_dev->adc, ENABLE);

    //重置校准寄存器   
    ADC_ResetCalibration(adc_dev->adc);
    while(ADC_GetResetCalibrationStatus(adc_dev->adc));

    //开始校准状态
    ADC_StartCalibration(adc_dev->adc);
    while(ADC_GetCalibrationStatus(adc_dev->adc));

    return TRUE;
}
/***************************************************************************************************
*\Function      stm32_adc_start_convert
*\Description   
*\Parameter     adc_dev
*\Parameter     channel
*\Parameter     sample
*\Parameter     port
*\Return        u16
*\Note          
*\Log           2017.05.12    Ver 1.0    Job               
				创建函数。
***************************************************************************************************/
u16 stm32_adc_start_convert(ADC_DMA_Type* adc_dev, u32 channel, u32 sample, GpioType* port)
{
    u32 time_out = 0x00;
    GPIO_InitTypeDef GPIO_InitStruct;

    if (adc_dev == RT_NULL || adc_dev->adc_count >= 16)
    {
        return FALSE;
    }

    if (port != RT_NULL)
    {
        //使能端口时钟
        switch((u32)port->gpio)
        {
        case GPIOA_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
            break;
        case GPIOB_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
            break;
        case GPIOC_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
            break;
        case GPIOD_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
            break;
        case GPIOE_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
            break;
        case GPIOF_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);
            break;
        case GPIOG_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, ENABLE);
            break;
        default:
            break;
        }

        //初始化gpio引脚
        GPIO_InitStruct.GPIO_Pin   = port->gpio_pin;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

        GPIO_Init(port->gpio, &GPIO_InitStruct); 
    }
    /*##-2- Configure ADC regular channel ######################################*/
    ADC_RegularChannelConfig(adc_dev->adc, channel, 1, sample);   //芯片温度

    ADC_SoftwareStartConvCmd(ADC1, ENABLE);     //使能指定的ADC1的软件转换启动功能
    while (time_out++ < RT_TICK_PER_SECOND)
    {
        rt_thread_delay(1);
        if (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ))//等待转换结束
        {
            break;
        }
    }

    return ADC_GetConversionValue(ADC1);    //返回最近一次ADC1规则组的转换结果
}

///*MCU内部温度传感器*/
static u16 ADC_InterT_Buf[16];
ADC_DMA_Type ADC_InterT = {DMA1_Channel1, ADC1, 1, (u16*)&ADC_InterT_Buf, countof(ADC_InterT_Buf)};

u32 Temp_Chip;
void chip_temp_thread_entry(void* p)
{
    //adc内部温度
    stm32_adc_init(&ADC_InterT);

    while (1)
    {
//        Temp_Chip = stm32_get_chip_temp(stm32_adc_start_convert(&ADC_InterT, ADC_Channel_16, ADC_SampleTime_239Cycles5, RT_NULL));
        rt_thread_delay(100);
    }
}
#endif
