/***************************************************************************************************
*                    (c) Copyright 2008-2017  Syncretize technologies co.,ltd.
*										All Rights Reserved
*
*\File          app_board.c
*\Description   
*\Note          
*\Log           2017.06.02    Ver 1.0    Job
*               创建文件。
***************************************************************************************************/
#include "app_board.h"
#include "param.h"
#include "uart_ota_protocol.h"
//指示灯
static switch_type MsgRed_Switch = {GPIOB, GPIO_Pin_5, FALSE};//红色
static switch_type MsgBlue_Switch = {GPIOB, GPIO_Pin_1, FALSE};//蓝色
static switch_type MsgGreen_Switch = {GPIOB, GPIO_Pin_13, FALSE};//绿色  pxu

BreatheMsgLedType MsgRed  = {&MsgRed_Switch};
BreatheMsgLedType MsgBlue = {&MsgBlue_Switch};
BreatheMsgLedType MsgGreen = {&MsgGreen_Switch};

//串口
static GpioType UartTx = {GPIOA, GPIO_Pin_9};
static GpioType UartRx = {GPIOA, GPIO_Pin_10};
UartDevType UartTTL = {USART1, &UartTx, &UartRx, RT_NULL, 115200};

//CAN  RX PA11 TX PA12
static GpioType CAN_Tx = {GPIOA, GPIO_Pin_12};
static GpioType CAN_Rx = {GPIOA, GPIO_Pin_11};
CANBus_type CAN = {CAN1, &CAN_Tx, &CAN_Rx, CAN_BAUDRATE_500K};

//MCU内部温度传感器*
//static u16 ADC_InterT_Buf[32];
//ADC_DMA_Type ADC_InterT = {DMA1_Channel1, ADC1, 1, (u16*)&ADC_InterT_Buf, countof(ADC_InterT_Buf)};

/***************************************************************************************************
*\Function      app_board_init
*\Description   初始化所有变量
*\Parameter     
*\Return        void
*\Note          1、注意必须在相关应用之前初始化
*\Log           2017.06.02    Ver 1.0    Job               
				创建函数。
***************************************************************************************************/
void app_board_init(void)
{
    //RTC时钟
    stm32_rtc_init();

    //指示灯
	breathe_msg_led_init(&MsgRed, "MsgRed",   100, 100);
	breathe_msg_led_init(&MsgBlue, "MsgBlue", 100, 100);
	breathe_msg_led_init(&MsgGreen, "MsgGreen", 100, 100);
    //启动
//    breathe_msg_led_on(&MsgRed, 0, FALSE);
//    breathe_msg_led_on(&MsgBlue, 0, FALSE);
	breathe_msg_led_on(&MsgGreen, 0, FALSE);
	stm32_CAN_init(&CAN);
//	uart_ota_init();//
    //MCU温度
//    stm32_adc_with_dma_init(&ADC_InterT);
//    stm32_adc_channel_add(&ADC_InterT, 0, ADC_Channel_16, ADC_SampleTime_239Cycles5, RT_NULL);
//    stm32_adc_with_dma_start(&ADC_InterT);
	
}

