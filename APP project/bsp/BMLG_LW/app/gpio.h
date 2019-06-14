/***************************************************************************************************
*                    (c) Copyright 2008-2018  Syncretize technologies co.,ltd.
*										All Rights Reserved
*
*\File          exti.h
*\Description   
*\Note          
*\Log           2019.04.17    Ver 1.0    baron
*               创建文件。
***************************************************************************************************/
#ifndef _GPIO_H_
#define _GPIO_H_
#include "public_type.h"

#define red 0
#define orange 1

#define red1_OFF   GPIO_ResetBits(GPIOA,GPIO_Pin_12)
#define red2_OFF   GPIO_ResetBits(GPIOC,GPIO_Pin_11)
#define red3_OFF   GPIO_ResetBits(GPIOD,GPIO_Pin_5)
#define red4_OFF   GPIO_ResetBits(GPIOG,GPIO_Pin_12)
#define red5_OFF   GPIO_ResetBits(GPIOG,GPIO_Pin_15)
#define red6_OFF   GPIO_ResetBits(GPIOE,GPIO_Pin_4)
#define red7_OFF   GPIO_ResetBits(GPIOC,GPIO_Pin_14)
#define red8_OFF   GPIO_ResetBits(GPIOE,GPIO_Pin_2)
#define red9_OFF   GPIO_ResetBits(GPIOE,GPIO_Pin_1)
#define red10_OFF   GPIO_ResetBits(GPIOE,GPIO_Pin_0)
#define red11_OFF   GPIO_ResetBits(GPIOE,GPIO_Pin_14)
#define red12_OFF   GPIO_ResetBits(GPIOE,GPIO_Pin_8)
#define red13_OFF   GPIO_ResetBits(GPIOF,GPIO_Pin_14)
#define red14_OFF   GPIO_ResetBits(GPIOF,GPIO_Pin_11)
#define red15_OFF   GPIO_ResetBits(GPIOB,GPIO_Pin_1)
#define red16_OFF   GPIO_ResetBits(GPIOB,GPIO_Pin_10)
#define red17_OFF   GPIO_ResetBits(GPIOB,GPIO_Pin_11)
#define red18_OFF   GPIO_ResetBits(GPIOB,GPIO_Pin_12)
#define red19_OFF   GPIO_ResetBits(GPIOB,GPIO_Pin_13)

#define red1_ON   GPIO_SetBits(GPIOA,GPIO_Pin_12)
#define red2_ON   GPIO_SetBits(GPIOC,GPIO_Pin_11)
#define red3_ON   GPIO_SetBits(GPIOD,GPIO_Pin_5)
#define red4_ON   GPIO_SetBits(GPIOG,GPIO_Pin_12)
#define red5_ON   GPIO_SetBits(GPIOG,GPIO_Pin_15)
#define red6_ON   GPIO_SetBits(GPIOE,GPIO_Pin_4)
#define red7_ON   GPIO_SetBits(GPIOC,GPIO_Pin_14)
#define red8_ON   GPIO_SetBits(GPIOE,GPIO_Pin_2)
#define red9_ON   GPIO_SetBits(GPIOE,GPIO_Pin_1)
#define red10_ON   GPIO_SetBits(GPIOE,GPIO_Pin_0)
#define red11_ON   GPIO_SetBits(GPIOE,GPIO_Pin_14)
#define red12_ON   GPIO_SetBits(GPIOE,GPIO_Pin_8)
#define red13_ON   GPIO_SetBits(GPIOF,GPIO_Pin_14)
#define red14_ON   GPIO_SetBits(GPIOF,GPIO_Pin_11)
#define red15_ON   GPIO_SetBits(GPIOB,GPIO_Pin_1)
#define red16_ON   GPIO_SetBits(GPIOB,GPIO_Pin_10)
#define red17_ON   GPIO_SetBits(GPIOB,GPIO_Pin_11)
#define red18_ON   GPIO_SetBits(GPIOB,GPIO_Pin_12)
#define red19_ON   GPIO_SetBits(GPIOB,GPIO_Pin_13)

#define orange1_OFF   GPIO_ResetBits(GPIOC,GPIO_Pin_10)
#define orange2_OFF   GPIO_ResetBits(GPIOA,GPIO_Pin_15)
#define orange3_OFF   GPIO_ResetBits(GPIOD,GPIO_Pin_7)
#define orange4_OFF   GPIO_ResetBits(GPIOD,GPIO_Pin_6)
#define orange5_OFF   GPIO_ResetBits(GPIOB,GPIO_Pin_4)
#define orange6_OFF   GPIO_ResetBits(GPIOC,GPIO_Pin_13)
#define orange7_OFF   GPIO_ResetBits(GPIOE,GPIO_Pin_5)
#define orange8_OFF   GPIO_ResetBits(GPIOE,GPIO_Pin_6)
#define orange9_OFF   GPIO_ResetBits(GPIOB,GPIO_Pin_9)
#define orange10_OFF   GPIO_ResetBits(GPIOB,GPIO_Pin_5)
#define orange11_OFF   GPIO_ResetBits(GPIOE,GPIO_Pin_15)
#define orange12_OFF   GPIO_ResetBits(GPIOE,GPIO_Pin_7)
#define orange13_OFF   GPIO_ResetBits(GPIOF,GPIO_Pin_15)
#define orange14_OFF   GPIO_ResetBits(GPIOF,GPIO_Pin_12)
#define orange15_OFF   GPIO_ResetBits(GPIOB,GPIO_Pin_0)
#define orange16_OFF   GPIO_ResetBits(GPIOD,GPIO_Pin_8)
#define orange17_OFF   GPIO_ResetBits(GPIOD,GPIO_Pin_12)
#define orange18_OFF   GPIO_ResetBits(GPIOB,GPIO_Pin_14)
#define orange19_OFF   GPIO_ResetBits(GPIOB,GPIO_Pin_15)

#define orange1_ON   GPIO_SetBits(GPIOC,GPIO_Pin_10)
#define orange2_ON   GPIO_SetBits(GPIOA,GPIO_Pin_15)
#define orange3_ON   GPIO_SetBits(GPIOD,GPIO_Pin_7)
#define orange4_ON   GPIO_SetBits(GPIOD,GPIO_Pin_6)
#define orange5_ON   GPIO_SetBits(GPIOB,GPIO_Pin_4)
#define orange6_ON   GPIO_SetBits(GPIOC,GPIO_Pin_13)
#define orange7_ON   GPIO_SetBits(GPIOE,GPIO_Pin_5)
#define orange8_ON   GPIO_SetBits(GPIOE,GPIO_Pin_6)
#define orange9_ON   GPIO_SetBits(GPIOB,GPIO_Pin_9)
#define orange10_ON   GPIO_SetBits(GPIOB,GPIO_Pin_5)
#define orange11_ON   GPIO_SetBits(GPIOE,GPIO_Pin_15)
#define orange12_ON   GPIO_SetBits(GPIOE,GPIO_Pin_7)
#define orange13_ON   GPIO_SetBits(GPIOF,GPIO_Pin_15)
#define orange14_ON   GPIO_SetBits(GPIOF,GPIO_Pin_12)
#define orange15_ON   GPIO_SetBits(GPIOB,GPIO_Pin_0)
#define orange16_ON   GPIO_SetBits(GPIOD,GPIO_Pin_8)
#define orange17_ON   GPIO_SetBits(GPIOD,GPIO_Pin_12)
#define orange18_ON   GPIO_SetBits(GPIOB,GPIO_Pin_14)
#define orange19_ON   GPIO_SetBits(GPIOB,GPIO_Pin_15)

extern u8 LED_Twinkle_Mode;
/* Private function prototypes -----------------------------------------------*/
void led_gpio_init(void);
void LEDcontrol_thread_entry(void* parameter);
#endif
