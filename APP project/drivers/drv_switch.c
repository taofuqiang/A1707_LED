/***************************************************************************************************
*                    (c) Copyright 2006-2016  Syncretize technologies co.,ltd.
*										All Rights Reserved
*	
*\File 			drv_switch.c
*\Description   输出开关量控制结构
*\Note          
*\Log           2016.07.08    Ver 1.0   Job
*               创建文件。
***************************************************************************************************/
#include "drv_switch.h"

/***************************************************************************************************
*\Function      rt_switch_on
*\Description   打开switch
*\Parameter     sw
*\Return        void
*\Note          
*\Log           2016.07.08    Ver 1.0    Job
*               创建函数。
***************************************************************************************************/
void stm32_switch_on(switch_type* sw)
{
    if (sw == RT_NULL)
    {
        return;
    }
    if (sw->valid)
    {
        GPIO_SetBits(sw->gpio, sw->pin);
    }
    else
    {
        GPIO_ResetBits(sw->gpio, sw->pin);
    }
}
/***************************************************************************************************
*\Function      rt_switch_off
*\Description   关闭输出
*\Parameter     sw
*\Return        void
*\Note          
*\Log           2016.07.08    Ver 1.0    Job
*               创建函数。
***************************************************************************************************/
void stm32_switch_off(switch_type* sw)
{
    if (sw == RT_NULL)
    {
        return;
    }
    if (sw->valid)
    {
        GPIO_ResetBits(sw->gpio, sw->pin);
    }
    else
    {
        GPIO_SetBits(sw->gpio, sw->pin);
    }
}
/***************************************************************************************************
*\Function      rt_switch_evert
*\Description   反转输出
*\Parameter     sw
*\Return        void
*\Note          
*\Log           2016.07.08    Ver 1.0    Job
*               创建函数。
***************************************************************************************************/
void stm32_switch_evert(switch_type* sw)
{
    if (sw == RT_NULL)
    {
        return;
    }
    GPIO_ToggleBits(sw->gpio, sw->pin);
}
/***************************************************************************************************
*\Function      rt_switch_init
*\Description   
*\Parameter     gpio,端口
*\Parameter     pin，引脚
*\Parameter     valid,on时的有效电平
*\Return        void
*\Note          
*\Log           2016.07.08    Ver 1.0    Job
*               创建函数。
***************************************************************************************************/
void stm32_switch_init(switch_type* sw)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_TypeDef* gpio = sw->gpio;
    u16 pin = sw->pin;

    if (sw == RT_NULL)
    {
        return;
    }

    //使能端口时钟
    switch((u32)gpio)
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
        rt_kprintf("gpio clk error!\n");
        break;
    }

    //默认关闭输出
    stm32_switch_off(sw);

    /*引脚初始化,pp模式*/
    GPIO_InitStructure.GPIO_Pin = pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(gpio, &GPIO_InitStructure);

    //默认关闭输出
    stm32_switch_off(sw);
}
