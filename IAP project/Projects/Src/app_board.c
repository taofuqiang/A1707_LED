/***************************************************************************************************
*                    (c) Copyright 2008-2018  Syncretize technologies co.,ltd.
*										All Rights Reserved
*
*\File          app_board.c
*\Description   
*\Note          
*\Log           2018.01.19    Ver 1.0    Job
*               创建文件。
***************************************************************************************************/
#include "app_board.h"

//LED
switch_type MSGRED_LED = {GPIOB, GPIO_PIN_5, FALSE};
switch_type MSGBLUE_LED = {GPIOB, GPIO_PIN_1, FALSE};
switch_type MSGGREEN_LED = {GPIOB, GPIO_PIN_13, FALSE};

//串口
static GpioType UartTx = {GPIOA, GPIO_PIN_9};
static GpioType UartRx = {GPIOA, GPIO_PIN_10};
static u8 UartRxBuf[1024];

UartDevType UartTTL = {USART1, &UartTx, &UartRx, NULL, 115200, UartRxBuf, sizeof(UartRxBuf)};

//CAN  TX PA12 RX PA11
static GpioType CAN_Tx = {GPIOA, GPIO_PIN_12};
static GpioType CAN_Rx = {GPIOA, GPIO_PIN_11};
CANBus_type CAN = {CAN1, &CAN_Tx, &CAN_Rx, CAN_BAUDRATE_500K};

/***************************************************************************************************
*\Function      stm32_board_lowpower_cfg
*\Description   低功耗配置
*\Parameter     
*\Return        void
*\Note          
*\Log           2018.05.25    Ver 1.0    Job               
				创建函数。
***************************************************************************************************/
void stm32_board_lowpower_cfg(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable GPIOs clock */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();

  /* Configure all GPIO port pins in Analog Input mode (floating input trigger OFF) */
  /* Note: Debug using ST-Link is not possible during the execution of this   */
  /*       example because communication between ST-link and the device       */
  /*       under test is done through UART. All GPIO pins are disabled (set   */
  /*       to analog input mode) including  UART I/O pins.           */
  GPIO_InitStructure.Pin = GPIO_PIN_All;
  GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStructure.Pull = GPIO_NOPULL;

  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure); 
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
  HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);
  HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
  HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);

  /* Disable GPIOs clock */
  __HAL_RCC_GPIOA_CLK_DISABLE();
  __HAL_RCC_GPIOB_CLK_DISABLE();
  __HAL_RCC_GPIOC_CLK_DISABLE();
  __HAL_RCC_GPIOD_CLK_DISABLE();
  __HAL_RCC_GPIOE_CLK_DISABLE();

}
/***************************************************************************************************
*\Function      acs_app_board_init
*\Description   
*\Parameter     
*\Return        void
*\Note          
*\Log           2018.01.19    Ver 1.0    Job               
				创建函数。
***************************************************************************************************/
extern  void sensor_gpio_Init(void);
void app_board_init(void)
{
    //全部引脚配置一次
    stm32_board_lowpower_cfg();
    //看门狗先初始化
    watchdog_init();
    //串口 如果使用该串口调试 则优先初始化
    stm32_uart_init(&UartTTL);
    stm32_switch_init(&MSGRED_LED);
	stm32_switch_init(&MSGBLUE_LED);
	stm32_switch_init(&MSGGREEN_LED);
	show_version();
    GetRestFlag();
	//RTC
    stm32_Inrtc_init(FALSE);   
	stm32_CAN_init(&CAN);
}


/* Private function prototypes -----------------------------------------------*/
#ifdef USING_USART_PRINTF
int fputc(int ch, FILE *f)
{
    stm32_uart_send(&UartTTL, (u8*)&ch, 1);
    return ch;
}
#endif /* __GNUC__ */
