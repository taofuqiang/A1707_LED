/***************************************************************************************************
*                    (c) Copyright 2008-2018  Syncretize technologies co.,ltd.
*										All Rights Reserved
*
*\File          gpio.c
*\Description   通过中断有无判定通信的有无
*\Note          
*\Log           2019.05.06    Ver 1.0    baron
*               创建文件。
***************************************************************************************************/
#include "gpio.h"
#include "app_board.h"


/** @addtogroup STM32F10x_StdPeriph_Examples
  * @{
  */

/** @addtogroup EXTI_Config
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
u8 LED_Twinkle_Mode = 0xff;
GPIO_InitTypeDef   GPIO_InitStructure;

void Led_ModeSelect(u8 num, u8 sel, bool control );
/**
  * @brief  Configure led control mode
  * @param  None
  * @retval None
  */
void led_gpio_init(void)
{
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);//禁止JTACK  for pa15/pb4
	
  /* Enable GPIOA clock */ 
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	/* control pin (PA.12/15/-B19/B13) configuration */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_12 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
  /* Enable GPIOB clock */ 
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	/* control pin (C20/C18/B1/B20/B16/C19/C15/C11/C7/C9/C5) configuration */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  
  GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	/* Enable GPIOC clock */ 
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	/* control pin (B17/B15/B4/B6) configuration */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_13 | GPIO_Pin_14;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  
  GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	/* Enable GPIOD clock */ 
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
	/* control pin (B11/B5/B9/C17/C13/C3/C1) configuration */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_12 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  
  GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	/* Enable GPIOE clock */ 
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
	/* control pin (B18/B14/B10/B2/B8/B12/C8/C6/C2/C4) configuration */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  
  GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	/* Enable GPIOF clock */ 
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);
	/* control pin (C14/C16/C10/C12) configuration */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  
  GPIO_Init(GPIOF, &GPIO_InitStructure);
	
	/* Enable GPIOG clock */ 
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, ENABLE);
	/* control pin (C14/C16/C10/C12) configuration */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_12 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  
  GPIO_Init(GPIOG, &GPIO_InitStructure);
	//以下两个引脚需单独关断
	orange2_OFF;
	orange5_OFF;
}

/**
  * @brief  Configure led twinkle mode on
  * @param  None 全开
  * @retval None
  */
void twinkle_modeon(void)
{
  u8 num = 0;
	
	for( num=0; num<19; num++ )
	{
	  Led_ModeSelect(num, red, TRUE );//open red led
		Led_ModeSelect(num, orange, TRUE );//open orange led
	}
}
/**
  * @brief  Configure led twinkle mode off
  * @param  None 全关
  * @retval None
  */
void twinkle_modeoff(void)
{
  u8 num = 0;
	
	for( num=0; num<19; num++ )
	{
	  Led_ModeSelect(num, red, FALSE );//close red led
		Led_ModeSelect(num, orange, FALSE );//close orange led
	}
}
/**
  * @brief  Configure led twinkle mode1
  * @param  None 红灯闪烁
  * @retval None
  */
void twinkle_mode1(void)
{
  u8 num = 0;
	
	for( num=0; num<19; num++ )
	{
	  Led_ModeSelect(num, red, TRUE );//open red led
	}
	rt_thread_delay(50);
	for( num=0; num<19; num++ )
	{
	  Led_ModeSelect(num, red, FALSE );//close red led
	}
	rt_thread_delay(80);
}
/**
  * @brief  Configure led twinkle mode2
  * @param  None 橙灯闪烁
  * @retval None
  */
void twinkle_mode2(void)
{
  u8 num = 0;
	
	for( num=0; num<19; num++ )
	{
	  Led_ModeSelect(num, orange, TRUE );//open orange led
	}
	rt_thread_delay(50);
	for( num=0; num<19; num++ )
	{
	  Led_ModeSelect(num, orange, FALSE );//close orange led
	}
	rt_thread_delay(80);
}
/**
  * @brief  Configure led twinkle mode3
  * @param  None 红灯橙灯闪烁
  * @retval None
  */
void twinkle_mode3(void)
{
  u8 num = 0;
	
	for( num=0; num<19; num++ )
	{
		Led_ModeSelect(num, red, TRUE );//open red led
	  Led_ModeSelect(num, orange, TRUE );//open orange led
	}
	rt_thread_delay(50);
	for( num=0; num<19; num++ )
	{
		Led_ModeSelect(num, red, FALSE );//close red led
	  Led_ModeSelect(num, orange, FALSE );//close orange led
	}
	rt_thread_delay(80);
}
/**
  * @brief  Configure led twinkle mode4
  * @param  None 红灯依次点亮
  * @retval None
  */
void twinkle_mode4(void)
{
  u8 num = 0;
	
	for( num=0; num<19; num++ )//依次点亮
	{
		Led_ModeSelect(num, red, TRUE );//open red led
	  rt_thread_delay(5);
	}
	rt_thread_delay(20);
	for( num=0; num<19; num++ )//全关
	{
		Led_ModeSelect(num, red, FALSE );//close red led
	}
}
/**
  * @brief  Configure led twinkle mode5
  * @param  None 橙灯依次点亮
  * @retval None
  */
void twinkle_mode5(void)
{
  u8 num = 0;
	
	for( num=0; num<19; num++ )//依次点亮
	{
		Led_ModeSelect(num, orange, TRUE );//open orange led
	  rt_thread_delay(5);
	}
	rt_thread_delay(40);
	for( num=0; num<19; num++ )//全关
	{
		Led_ModeSelect(num, orange, FALSE );//close orange led
	}
}
/**
  * @brief  Configure led twinkle mode6
  * @param  None 红橙两灯依次点亮
  * @retval None
  */
void twinkle_mode6(void)
{
  u8 num = 0;
	
	for( num=0; num<19; num++ )//依次点亮
	{
		Led_ModeSelect(num, red, TRUE );//open red led
		Led_ModeSelect(num, orange, TRUE );//open orange led
	  rt_thread_delay(5);
	}
	rt_thread_delay(40);
	for( num=0; num<19; num++ )//全关
	{
		Led_ModeSelect(num, red, FALSE );//close red led
		Led_ModeSelect(num, orange, FALSE );//close orange led
	}
}
/**
  * @brief  Configure led twinkle mode7
  * @param  None 红灯线条扫描
  * @retval None
  */
void twinkle_mode7(void)
{
  u8 num = 0;
	
	for( num=0; num<20; num++ )//依次点亮
	{
		Led_ModeSelect(num, red, TRUE );//open red led
		Led_ModeSelect(num-1, red, FALSE );//close red led
	  rt_thread_delay(30);
	}
}
/**
  * @brief  Configure led twinkle mode8
  * @param  None 橙灯线条扫描
  * @retval None
  */
void twinkle_mode8(void)
{
  u8 num = 0;
	
	for( num=0; num<20; num++ )//依次点亮
	{
		Led_ModeSelect(num, orange, TRUE );//open orange led
		Led_ModeSelect(num-1, orange, FALSE );//open orange led
	  rt_thread_delay(30);
	}
}
/**
  * @brief  Configure led twinkle mode8
  * @param  None 红橙两灯线条扫描
  * @retval None
  */
void twinkle_mode9(void)
{
  u8 num = 0;
	
	for( num=0; num<20; num++ )//依次点亮
	{
		Led_ModeSelect(num, red, TRUE );//open red led
		Led_ModeSelect(num, orange, TRUE );//open orange led
		Led_ModeSelect(num-1, red, FALSE );//close red led
		Led_ModeSelect(num-1, orange, FALSE );//close orange led
	  rt_thread_delay(30);
	}
}
/**
  * @brief  Configure led twinkle mode7
  * @param  None 红灯依次从中间向两边点亮
  * @retval None
  */
void twinkle_mode10(void)
{
  u8 num = 0;
	
	for( num=0; num<10; num++ )//依次点亮
	{
		Led_ModeSelect(9+num, red, TRUE );//open red led
		Led_ModeSelect(9-num, red, TRUE );//open red led
	  rt_thread_delay(15);
	}
}
/**
  * @brief  Configure led twinkle mode7
  * @param  None 橙灯依次从中间向两边点亮
  * @retval None
  */
void twinkle_mode11(void)
{
  u8 num = 0;
	
	for( num=0; num<10; num++ )//依次点亮
	{
		Led_ModeSelect(9+num, orange, TRUE );//open red led
		Led_ModeSelect(9-num, orange, TRUE );//open red led
	  rt_thread_delay(15);
	}
}
/**
  * @brief  Configure led twinkle mode7
  * @param  None 红橙两灯依次从中间向两边点亮
  * @retval None
  */
void twinkle_mode12(void)
{
  u8 num = 0;
	
	for( num=0; num<10; num++ )//依次点亮
	{
		Led_ModeSelect(9+num, red, TRUE );//open red led
		Led_ModeSelect(9-num, red, TRUE );//open red led
		Led_ModeSelect(9+num, orange, TRUE );//open red led
		Led_ModeSelect(9-num, orange, TRUE );//open red led
	  rt_thread_delay(15);
	}
}
/**
  * @brief  Configure led twinkle mode7
  * @param  None 红灯线条从中间向两边扫描
  * @retval None
  */
void twinkle_mode13(void)
{
  u8 num = 0;
	
	for( num=0; num<10; num++ )//依次点亮
	{
		Led_ModeSelect(9+num, red, TRUE );//open red led
		Led_ModeSelect(9-num, red, TRUE );//close red led
		Led_ModeSelect(9+num-1, red, FALSE );//close red led
		Led_ModeSelect(9-num+1, red, FALSE );//close red led
	  rt_thread_delay(15);
	}
}
/**
  * @brief  Configure led twinkle mode7
  * @param  None 橙灯线条从中间向两边扫描
  * @retval None
  */
void twinkle_mode14(void)
{
  u8 num = 0;
	
	for( num=0; num<10; num++ )//依次点亮
	{
		Led_ModeSelect(9+num, orange, TRUE );//open red led
		Led_ModeSelect(9-num, orange, TRUE );//close red led
		Led_ModeSelect(9+num-1, orange, FALSE );//close red led
		Led_ModeSelect(9-num+1, orange, FALSE );//close red led
	  rt_thread_delay(15);
	}
}
/**
  * @brief  Configure led twinkle mode7
  * @param  None 红橙两灯线条从中间向两边扫描
  * @retval None
  */
void twinkle_mode15(void)
{
  u8 num = 0;
	
	for( num=0; num<10; num++ )//依次点亮
	{
		Led_ModeSelect(9+num, red, TRUE );//open red led
		Led_ModeSelect(9-num, red, TRUE );//close red led
		Led_ModeSelect(9+num, orange, TRUE );//open red led
		Led_ModeSelect(9-num, orange, TRUE );//close red led
		Led_ModeSelect(9+num-1, red, FALSE );//close red led
		Led_ModeSelect(9-num+1, red, FALSE );//close red led
		Led_ModeSelect(9+num-1, orange, FALSE );//close red led
		Led_ModeSelect(9-num+1, orange, FALSE );//close red led
	  rt_thread_delay(15);
	}
}
/**
  * @brief  Configure led twinkle mode on
  * @param  None 红灯全亮
  * @retval None
  */
void twinkle_mode16(void)
{
  u8 num = 0;
	
	for( num=0; num<19; num++ )
	{
	  Led_ModeSelect(num, red, TRUE );//open red led
	}
}
/**
  * @brief  Configure led twinkle mode on
  * @param  None 橙灯全亮
  * @retval None
  */
void twinkle_mode17(void)
{
  u8 num = 0;
	
	for( num=0; num<19; num++ )
	{
		Led_ModeSelect(num, orange, TRUE );//open orange led
	}
}
/**
  * @brief  Configure led twinkle mode4
  * @param  None  num:0-18号灯  sel: 0：红灯 1:橙灯   control：0：灭  1：亮
  * @retval None
  */
void Led_ModeSelect(u8 num, u8 sel, bool control )
{
  switch(num)
	{
		case 0x00:
		{
		  if(sel == 0)
			{
			  if(control == 0)
				{
				  red1_OFF;
				}
				else
				{
				  red1_ON;
				}
			}
			else
			{
			  if(control == 0)
				{
				  orange1_OFF;
				}
				else
				{
				  orange1_ON;
				}
			}
		}
		break;
		case 0x01:
		{
		  if(sel == 0)
			{
			  if(control == 0)
				{
				  red2_OFF;
				}
				else
				{
				  red2_ON;
				}
			}
			else
			{
			  if(control == 0)
				{
				  orange2_OFF;
				}
				else
				{
				  orange2_ON;
				}
			}
		}
		break;
		case 0x02:
		{
		  if(sel == 0)
			{
			  if(control == 0)
				{
				  red3_OFF;
				}
				else
				{
				  red3_ON;
				}
			}
			else
			{
			  if(control == 0)
				{
				  orange3_OFF;
				}
				else
				{
				  orange3_ON;
				}
			}
		}
		break;
		case 0x03:
		{
		  if(sel == 0)
			{
			  if(control == 0)
				{
				  red4_OFF;
				}
				else
				{
				  red4_ON;
				}
			}
			else
			{
			  if(control == 0)
				{
				  orange4_OFF;
				}
				else
				{
				  orange4_ON;
				}
			}
		}
		break;
		case 0x04:
		{
		  if(sel == 0)
			{
			  if(control == 0)
				{
				  red5_OFF;
				}
				else
				{
				  red5_ON;
				}
			}
			else
			{
			  if(control == 0)
				{
				  orange5_OFF;
				}
				else
				{
				  orange5_ON;
				}
			}
		}
		break;
		case 0x05:
		{
		  if(sel == 0)
			{
			  if(control == 0)
				{
				  red6_OFF;
				}
				else
				{
				  red6_ON;
				}
			}
			else
			{
			  if(control == 0)
				{
				  orange6_OFF;
				}
				else
				{
				  orange6_ON;
				}
			}
		}
		break;
		case 0x06:
		{
		  if(sel == 0)
			{
			  if(control == 0)
				{
				  red7_OFF;
				}
				else
				{
				  red7_ON;
				}
			}
			else
			{
			  if(control == 0)
				{
				  orange7_OFF;
				}
				else
				{
				  orange7_ON;
				}
			}
		}
		break;
		case 0x07:
		{
		  if(sel == 0)
			{
			  if(control == 0)
				{
				  red8_OFF;
				}
				else
				{
				  red8_ON;
				}
			}
			else
			{
			  if(control == 0)
				{
				  orange8_OFF;
				}
				else
				{
				  orange8_ON;
				}
			}
		}
		break;
		case 0x08:
		{
		  if(sel == 0)
			{
			  if(control == 0)
				{
				  red9_OFF;
				}
				else
				{
				  red9_ON;
				}
			}
			else
			{
			  if(control == 0)
				{
				  orange9_OFF;
				}
				else
				{
				  orange9_ON;
				}
			}
		}
		break;
		case 0x09:
		{
		  if(sel == 0)
			{
			  if(control == 0)
				{
				  red10_OFF;
				}
				else
				{
				  red10_ON;
				}
			}
			else
			{
			  if(control == 0)
				{
				  orange10_OFF;
				}
				else
				{
				  orange10_ON;
				}
			}
		}
		break;
		case 0x0a:
		{
		  if(sel == 0)
			{
			  if(control == 0)
				{
				  red11_OFF;
				}
				else
				{
				  red11_ON;
				}
			}
			else
			{
			  if(control == 0)
				{
				  orange11_OFF;
				}
				else
				{
				  orange11_ON;
				}
			}
		}
		break;
		case 0x0b:
		{
		  if(sel == 0)
			{
			  if(control == 0)
				{
				  red12_OFF;
				}
				else
				{
				  red12_ON;
				}
			}
			else
			{
			  if(control == 0)
				{
				  orange12_OFF;
				}
				else
				{
				  orange12_ON;
				}
			}
		}
		break;
		case 0x0c:
		{
		  if(sel == 0)
			{
			  if(control == 0)
				{
				  red13_OFF;
				}
				else
				{
				  red13_ON;
				}
			}
			else
			{
			  if(control == 0)
				{
				  orange13_OFF;
				}
				else
				{
				  orange13_ON;
				}
			}
		}
		break;
		case 0x0d:
		{
		  if(sel == 0)
			{
			  if(control == 0)
				{
				  red14_OFF;
				}
				else
				{
				  red14_ON;
				}
			}
			else
			{
			  if(control == 0)
				{
				  orange14_OFF;
				}
				else
				{
				  orange14_ON;
				}
			}
		}
		break;
		case 0x0e:
		{
		  if(sel == 0)
			{
			  if(control == 0)
				{
				  red15_OFF;
				}
				else
				{
				  red15_ON;
				}
			}
			else
			{
			  if(control == 0)
				{
				  orange15_OFF;
				}
				else
				{
				  orange15_ON;
				}
			}
		}
		break;
		case 0x0f:
		{
		  if(sel == 0)
			{
			  if(control == 0)
				{
				  red16_OFF;
				}
				else
				{
				  red16_ON;
				}
			}
			else
			{
			  if(control == 0)
				{
				  orange16_OFF;
				}
				else
				{
				  orange16_ON;
				}
			}
		}
		break;
		case 0x10:
		{
		  if(sel == 0)
			{
			  if(control == 0)
				{
				  red17_OFF;
				}
				else
				{
				  red17_ON;
				}
			}
			else
			{
			  if(control == 0)
				{
				  orange17_OFF;
				}
				else
				{
				  orange17_ON;
				}
			}
		}
		break;
		case 0x11:
		{
		  if(sel == 0)
			{
			  if(control == 0)
				{
				  red18_OFF;
				}
				else
				{
				  red18_ON;
				}
			}
			else
			{
			  if(control == 0)
				{
				  orange18_OFF;
				}
				else
				{
				  orange18_ON;
				}
			}
		}
		break;
		case 0x12:
		{
		  if(sel == 0)
			{
			  if(control == 0)
				{
				  red19_OFF;
				}
				else
				{
				  red19_ON;
				}
			}
			else
			{
			  if(control == 0)
				{
				  orange19_OFF;
				}
				else
				{
				  orange19_ON;
				}
			}
		}
		break;
		default:rt_thread_delay(2);break;
	}
}
/***************************************************************************************************
*\Function      LEDcontrol_thread_entry
*\Description   
*\Parameter     parameter
*\Return        void
*\Note          
*\Log           2019.05.06    Ver 1.0    baron
*               创建函数。
***************************************************************************************************/
void LEDcontrol_thread_entry(void* parameter)
{

	while(1)
	{
		rt_thread_delay(5);	
		if(UART1_RxFlag == 1)
		{
		  LED_Twinkle_Mode = USART1_RxBuffer[0];
			UART1_RxFlag = 0;
		}
    switch(LED_Twinkle_Mode)
		//switch(0x0b)
		{
		  case 0x00:
			{
			  twinkle_modeoff();//全关红橙
			}
			break;
			case 0x01:
			{
			  twinkle_modeon();//全开红橙
			}
			break;
			case 0x02:
			{
				twinkle_modeoff();//全关
			  twinkle_mode1();//红灯闪烁
			}
			break;
			case 0x03:
			{
				twinkle_modeoff();//全关
			  twinkle_mode2();//橙灯闪烁
			}
			break;
			case 0x04:
			{
				twinkle_modeoff();//全关
			  twinkle_mode3();//红橙灯闪烁
			}
			break;
			case 0x05:
			{
				twinkle_modeoff();//全关
			  twinkle_mode4();//红灯依次点亮
			}
			break;
			case 0x06:
			{
				twinkle_modeoff();//全关
			  twinkle_mode5();//橙灯依次点亮
			}
			break;
			case 0x07:
			{
				twinkle_modeoff();//全关
			  twinkle_mode6();//红橙两灯依次点亮
			}
			break;
			case 0x08:
			{
				twinkle_modeoff();//全关
			  twinkle_mode7();//红灯线条扫描
			}
			break;
			case 0x09:
			{
				twinkle_modeoff();//全关
			  twinkle_mode8();//橙灯线条扫描
			}
			break;
			case 0x0a:
			{
				twinkle_modeoff();//全关
			  twinkle_mode9();//红橙两灯线条扫描
			}
			break;
			case 0x0b:
			{
				twinkle_modeoff();//全关
			  twinkle_mode10();//红灯依次从中间向两边点亮
			}
			break;
			case 0x0c:
			{
				twinkle_modeoff();//全关
			  twinkle_mode11();//橙灯依次从中间向两边点亮
			}
			break;
			case 0x0d:
			{
				twinkle_modeoff();//全关
			  twinkle_mode12();//红橙两灯依次从中间向两边点亮
			}
			break;
			case 0x0e:
			{
				twinkle_modeoff();//全关
			  twinkle_mode13();//红灯线条从中间向两边扫描
			}
			break;
			case 0x0f:
			{
				twinkle_modeoff();//全关
			  twinkle_mode14();//橙灯线条从中间向两边扫描
			}
			break;
			case 0x10:
			{
				twinkle_modeoff();//全关
			  twinkle_mode15();//红橙两红线条从中间向两边扫描
			}
			break;
			case 0x1a:
			{
			  twinkle_mode16();//红灯全亮
			}
			break;
			case 0x1b:
			{
			  twinkle_mode17();//橙灯全亮
			}
			break;
			default: rt_thread_delay(10);break;
		}
	}
}
/**
  * @}
  */ 
