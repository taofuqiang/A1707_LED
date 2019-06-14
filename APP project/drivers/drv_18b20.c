/***************************************************************************************************
*                    (c) Copyright 1992-2014 Syncretize technologies co.,ltd.
*                                       All Rights Reserved
*
*\File          stm32_18b20.c
*\Description   
*\Note          
*\Log           2014.12.04    Ver 1.0    张波
*               创建文件。
***************************************************************************************************/
#include "drv_18b20.h"

Ds18B20TempType Env_TMP;
Ds18B20TempType DC_TMP;

TempDs18B20Type Tempreture1,Tempreture2;

//精确一位小数表
const unsigned char TempX_TAB[16]={0x00,0x01,0x01,0x02,0x03,0x03,0x04,0x04,0x05,0x06,0x06,0x07,0x08,0x08,0x09,0x09};

/***************************************************************************************************
*\Function      stm32_18b20_delay
*\Description   此函数延时必须是us级别的，否则，影响系统调度
*\Parameter     temp
*\Parameter     us
*\Return        void
*\Note          
*\Log           2014.12.04    Ver 1.0    张波
*               创建函数。
***************************************************************************************************/
void stm32_ds18b20_delay(TempDs18B20Type* temp, u32 us)
{
    //u32 i = 0xFFFFFF;//最大计时，防止超时无法返回

    (temp->Tim)->CNT = 0;
    (temp->Tim)->ARR = (us);
    TIM_Cmd(temp->Tim, ENABLE);
    while (((temp->Tim)->CR1) & TIM_CR1_CEN)//只要使能，说明计时未结束
    {
        /*i--;
        if (!i)
        {
            break;
        }*/
    }
}

/***************************************************************************************************
*\Function      ds18B20_reset
*\Description   18b20复位接口
*\Parameter     temp
*\Return        bool
*\Note          
*\Log           2014.12.04    Ver 1.0    张波
*               创建函数。
***************************************************************************************************/
bool ds18B20_reset(TempDs18B20Type* temp)
{
    bool result = FALSE;

    GPIO_SetBits(temp->Gpio, temp->Pin);
    stm32_ds18b20_delay(temp, 50);
    GPIO_ResetBits(temp->Gpio, temp->Pin);
    stm32_ds18b20_delay(temp, 500);
    GPIO_SetBits(temp->Gpio, temp->Pin);
    stm32_ds18b20_delay(temp, 40);
    if (GPIO_ReadInputDataBit(temp->Gpio, temp->Pin) == 0)//ds18b20返回0，表示复位成功
    {
        result = TRUE;
    }
    stm32_ds18b20_delay(temp, 500);
    return result;
}

/***************************************************************************************************
*\Function      ds18b20_write_byte
*\Description   写一个字节
*\Parameter     temp
*\Parameter     data
*\Return        void
*\Note          
*\Log           2014.12.04    Ver 1.0    张波
*               创建函数。
***************************************************************************************************/
void ds18b20_write_byte(TempDs18B20Type* temp, u8 data)
{
    u8 i = 0;

    for (i = 0; i < 8; i++)
    {
        GPIO_ResetBits(temp->Gpio, temp->Pin);//在15u内送数到数据线上，DS18B20在15-60u读数
        stm32_ds18b20_delay(temp, 5);
        if (data & 0x01)
        {
            GPIO_SetBits(temp->Gpio, temp->Pin);
        }
        else
        {
            GPIO_ResetBits(temp->Gpio, temp->Pin);
        }
        stm32_ds18b20_delay(temp, 80);
        GPIO_SetBits(temp->Gpio, temp->Pin);
        stm32_ds18b20_delay(temp, 2);
        data >>= 1;
    }
}

/***************************************************************************************************
*\Function      ds18b20_read_byte
*\Description   读一个字节
*\Parameter     temp
*\Return        u8
*\Note          
*\Log           2014.12.04    Ver 1.0    张波
*               创建函数。
***************************************************************************************************/
u8 ds18b20_read_byte(TempDs18B20Type* temp)
{
    u8 i =0;
    u8 data = 0;

    GPIO_SetBits(temp->Gpio, temp->Pin);
    stm32_ds18b20_delay(temp, 5);
    for(i = 8; i > 0; i--)
    {
        data >>= 1;
        GPIO_ResetBits(temp->Gpio, temp->Pin);     //从读时序开始到采样信号线必须在15u内，且采样尽量安排在15u的最后
        stm32_ds18b20_delay(temp, 5);   //5us
        GPIO_SetBits(temp->Gpio, temp->Pin);
        stm32_ds18b20_delay(temp, 5);   //5us
        if(GPIO_ReadInputDataBit(temp->Gpio, temp->Pin))
        {
            data |= 0x80;
        }
        else
        {
            data &=0x7f;  
        }
        stm32_ds18b20_delay(temp, 65);   //65us
        GPIO_SetBits(temp->Gpio, temp->Pin);
    }
    return data;
}

/***************************************************************************************************
*\Function      stm32_ds18b20_init
*\Description   初始化ds18b20,
*\Parameter     temp
*\Parameter     gpio
*\Parameter     pin，io
*\Parameter     tim,精确延时TIM
*\Return        void
*\Note          1、默认18b20的分辨率为12位，即0.0625°
*\Log           2014.12.05    Ver 1.0    张波
*               创建函数。
***************************************************************************************************/
void stm32_ds18b20_init(TempDs18B20Type* temp, GPIO_TypeDef* gpio, u16 pin, TIM_TypeDef* tim)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    temp->Gpio = gpio;
    temp->Pin = pin;
    temp->Tim = tim;

    //端口时钟使能
    switch((u32)(temp->Gpio))
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
    default:
        rt_kprintf("18b20 gpio is undefine!\n");
        break;
    }
    /*引脚初始化,开漏模式*/
    GPIO_InitStructure.GPIO_Pin = temp->Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(temp->Gpio, &GPIO_InitStructure);
    GPIO_SetBits(temp->Gpio, temp->Pin);//输出高电平

    //tim初始化
    /*开启定时器对应的时钟*/
    switch ((u32)(temp->Tim))
    {
    case TIM1_BASE:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
        break;
    case TIM2_BASE:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
        break;
    case TIM3_BASE:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
        break;
    case TIM4_BASE:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
        break;
    case TIM5_BASE:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
        break;
    case TIM6_BASE:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
        break;
    case TIM7_BASE:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
        break;
        //     case TIM8_BASE:
        //         RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);
        //         break;
    default:
        rt_kprintf("temp tim is undefine!\n");
        break;
    }
    TIM_DeInit(temp->Tim);
    TIM_TimeBaseStructure.TIM_Period = 0xFFFF;	
    TIM_TimeBaseStructure.TIM_Prescaler = 72-1;    	//预分频,此值+1为分频的除数,每个计数1us
    TIM_TimeBaseStructure.TIM_ClockDivision = 0x0; 
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; 	//向上计数	
    TIM_TimeBaseInit(temp->Tim, &TIM_TimeBaseStructure); 
    TIM_SelectOnePulseMode(temp->Tim, TIM_OPMode_Single);
}

/***************************************************************************************************
*\Function      stm32_ds18b20_conv_start
*\Description   开启温度转换，转换后有一个时间的延时，不能立即去读取
*\Parameter     temp
*\Return        void
*\Note          
*\Log           2014.12.05    Ver 1.0    张波
*               创建函数。
***************************************************************************************************/
void stm32_ds18b20_conv_start(TempDs18B20Type* temp)
{
    ds18B20_reset(temp);
    ds18b20_write_byte(temp, DS18B20_ROM_CMD_SKIP);
    ds18b20_write_byte(temp, DS18B20_REG_CMD_CONV);
}

/***************************************************************************************************
*\Function      stm32_ds18b20_read_rawdata
*\Description   读取18b20寄存器中的原始数据
*\Parameter     temp
*\Return        u16，默认12位分辨率下，高5位为符号位
*\Note          1、当为负数时为补码形式
*\Log           2014.12.05    Ver 1.0    张波
*               创建函数。
***************************************************************************************************/
u16 stm32_ds18b20_read_rawdata(TempDs18B20Type* temp)
{
    u16 data1 = 0;
    u16 data2 = 0;

    ds18B20_reset(temp);
    ds18b20_write_byte(temp, DS18B20_ROM_CMD_SKIP);
    ds18b20_write_byte(temp, DS18B20_REG_CMD_READ_RAM);
    data1 = ds18b20_read_byte(temp);
    data2 = ds18b20_read_byte(temp);
    data2 = (data2 << 8) | data1;

    return data2;
}

/***************************************************************************************************
*\Function      stm32_ds18b20_raw2reality
*\Description   转换为float形式，精度不丢失
*\Parameter     data
*\Return        float
*\Note          
*\Log           2014.12.05    Ver 1.0    张波
*               创建函数。
***************************************************************************************************/
float stm32_ds18b20_raw2reality(u16 data)
{
    float t = 0.0;

    if (data & 0x8000)//补码形式，负数
    {
        data = ~data;
        data += 1;
        t = -(data * 0.0625);
    }
    else
    {
        t = data * 0.0625;
    }

    return t;
}

/***************************************************************************************************
*\Function      stm32_ds18b20_raw2Temp
*\Description   原始数据转化为整数、小数（1位）、符号
*\Parameter     data
*\Return        Ds18B20TempType
*\Note          
*\Log           2014.12.05    Ver 1.0    张波
*               创建函数。
***************************************************************************************************/
Ds18B20TempType stm32_ds18b20_raw2Temp(u16 data)
{
    Ds18B20TempType aTmp;
    u8 tmp_h = (u8)(data >> 8);
    u8 tmp_l = (u8)(data);
    u8 sign = 0;

    if(tmp_h & 0x80)
    {
        tmp_h = (~tmp_h) | 0x08;
        tmp_l = ~tmp_l + 1;
        if(tmp_l == 0)
        {
            tmp_h += 1;
        }
    }

    tmp_h = (tmp_h << 4) + ((tmp_l & 0xf0) >> 4);
    tmp_l = TempX_TAB[tmp_l & 0x0f];

    sign = tmp_h & 0x80;
    if(sign == 0x00)
    {
        aTmp.Signal = 0;//正数
    }
    else
    {
        aTmp.Signal = 1;
    }

    tmp_h &= 0x7f;
    aTmp.TmpH = tmp_h;
    aTmp.TmpL = tmp_l;
    
    return aTmp;
}

/***************************************************************************************************
*\Function      stm32_ds18b20_read_reality
*\Description   读取精确数值
*\Parameter     temp
*\Return        float
*\Note          
*\Log           2014.12.05    Ver 1.0    张波
*               创建函数。
***************************************************************************************************/
float stm32_ds18b20_read_reality(TempDs18B20Type* temp)
{
    float t = 0.0;
    u16 data = 0;

    stm32_ds18b20_conv_start(temp);
    rt_thread_delay(80);//延时800ms
    data = stm32_ds18b20_read_rawdata(temp);
    t = stm32_ds18b20_raw2reality(data);

    return t;
}

/***************************************************************************************************
*\Function      stm32_ds18b20_read_Temp
*\Description   读取整数 小数 符号类型温度
*\Parameter     temp
*\Return        Ds18B20TempType
*\Note          
*\Log           2014.12.05    Ver 1.0    张波
*               创建函数。
***************************************************************************************************/
Ds18B20TempType stm32_ds18b20_read_Temp(TempDs18B20Type* temp)
{
    Ds18B20TempType aTmp;
    u16 data = 0;

    stm32_ds18b20_conv_start(temp);
    rt_thread_delay(80);//延时800ms
    data = stm32_ds18b20_read_rawdata(temp);
    aTmp = stm32_ds18b20_raw2Temp(data);

    return aTmp;
}
void stm32_ds18b20_thread_entry(void* para)
{
//    stm32_ds18b20_init(&Tempreture1, GPIOC, GPIO_Pin_11, TIM5);
    //stm32_ds18b20_init(&Tempreture2, GPIOB, GPIO_Pin_6, TIM6);
    while(1)
    {  
        //DC_TMP =  stm32_ds18b20_read_Temp(&Tempreture1);
        Env_TMP = stm32_ds18b20_read_Temp(&Tempreture1);
        
        rt_thread_delay(100);
    }

}
