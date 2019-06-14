/***************************************************************************************************
*                    (c) Copyright 2008-2017  Syncretize technologies co.,ltd.
*										All Rights Reserved
*
*\File          app_board.c
*\Description   
*\Note          
*\Log           2017.06.02    Ver 1.0    Job
*               �����ļ���
***************************************************************************************************/
#include "app_board.h"
#include "param.h"
#include "uart_ota_protocol.h"
//ָʾ��

static switch_type MsgBlue_Switch = {GPIOA, GPIO_Pin_1, FALSE};//��ɫ

BreatheMsgLedType MsgBlue = {&MsgBlue_Switch};

//����
static GpioType UartTx = {GPIOB, GPIO_Pin_6};
static GpioType UartRx = {GPIOB, GPIO_Pin_7};
UartDevType UartTTL = {USART1, &UartTx, &UartRx, RT_NULL, 115200};

//CAN  RX PA11 TX PA12
static GpioType CAN_Tx = {GPIOA, GPIO_Pin_12};
static GpioType CAN_Rx = {GPIOA, GPIO_Pin_11};
CANBus_type CAN = {CAN1, &CAN_Tx, &CAN_Rx, CAN_BAUDRATE_500K};

//MCU�ڲ��¶ȴ�����*
//static u16 ADC_InterT_Buf[32];
//ADC_DMA_Type ADC_InterT = {DMA1_Channel1, ADC1, 1, (u16*)&ADC_InterT_Buf, countof(ADC_InterT_Buf)};

/***************************************************************************************************
*\Function      app_board_init
*\Description   ��ʼ�����б���
*\Parameter     
*\Return        void
*\Note          1��ע����������Ӧ��֮ǰ��ʼ��
*\Log           2017.06.02    Ver 1.0    Job               
				����������
***************************************************************************************************/
void app_board_init(void)
{
	//RTCʱ��
	stm32_rtc_init();

    //ָʾ��
	breathe_msg_led_init(&MsgBlue, "MsgBlue", 100, 100);
    //����
  breathe_msg_led_on(&MsgBlue, 0, FALSE);
 
	stm32_uart_init(&UartTTL);         //����ͨ��  ���Ƶư����˸ģʽ
	/* Enable the USART2 Pins Software Remapping */
    GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);
	led_gpio_init();                   //led�˿ڳ�ʼ��
	//stm32_CAN_init(&CAN);
//	uart_ota_init();//
    //MCU�¶�
//    stm32_adc_with_dma_init(&ADC_InterT);
//    stm32_adc_channel_add(&ADC_InterT, 0, ADC_Channel_16, ADC_SampleTime_239Cycles5, RT_NULL);
//    stm32_adc_with_dma_start(&ADC_InterT);
}
