/***************************************************************************************************
*                    (c) Copyright 2008-2017  Syncretize technologies co.,ltd.
*										All Rights Reserved
*
*\File          drv_can.c
*\Description   
*\Note          
*\Log           2017.10.17    Ver 1.0    Job
*               创建文件。
***************************************************************************************************/
#include "drv_can.h"
#include "app_board.h"
//#include "rthw.h"
CAN_HandleTypeDef     CanHandle;



void HAL_CAN_MspInit(CAN_HandleTypeDef *hcan)
{
  GPIO_InitTypeDef   GPIO_InitStruct;

  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* CAN1 Periph clock enable */
  __HAL_RCC_CAN1_CLK_ENABLE();
  /* Enable GPIO clock ****************************************/
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*##-2- Configure peripheral GPIO ##########################################*/
  /* CAN1 TX GPIO pin configuration */
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Pull = GPIO_PULLUP;

  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* CAN1 RX GPIO pin configuration */
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Pull = GPIO_PULLUP;

  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*##-3- Configure the NVIC #################################################*/
  /* NVIC configuration for CAN1 Reception complete interrupt */
  HAL_NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
}

/**
  * @brief CAN MSP De-Initialization
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO to their default state
  * @param hcan: CAN handle pointer
  * @retval None
  */
void HAL_CAN_MspDeInit(CAN_HandleTypeDef *hcan)
{
	 if(hcan->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspDeInit 0 */

  /* USER CODE END CAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN1_CLK_DISABLE();
  
    /**CAN GPIO Configuration    
    PA11     ------> CAN_RX
    PA12     ------> CAN_TX 
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);

    /* CAN1 interrupt DeInit */
    HAL_NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);
  /* USER CODE BEGIN CAN1_MspDeInit 1 */

  /* USER CODE END CAN1_MspDeInit 1 */
  }

	
}

/***************************************************************************************************
*\Function      stm32_CAN_init
*\Description   can初始化
*\Parameter     can_dev
*\Return        void
*\Note          
*\Log           2017.10.30    Ver 1.0    Job               
				创建函数。
***************************************************************************************************/
void stm32_CAN_init(CANBus_type* can_dev)
{
	CAN_FilterTypeDef  sFilterConfig;
	
    if (can_dev == NULL || can_dev->RxGpio == NULL || can_dev->TxGpio == NULL)
    {
        return;
    }

    can_dev->CanRx_Read = 0;
    can_dev->CanRx_Write = 0;


	
	CanHandle.Instance = CAN1;
	CanHandle.Init.TimeTriggeredMode = DISABLE;
	CanHandle.Init.AutoBusOff = DISABLE;
	CanHandle.Init.AutoWakeUp = DISABLE;
	CanHandle.Init.AutoRetransmission = ENABLE;
	CanHandle.Init.ReceiveFifoLocked = DISABLE;
	CanHandle.Init.TransmitFifoPriority = DISABLE;
	CanHandle.Init.Mode = CAN_MODE_NORMAL;
	CanHandle.Init.SyncJumpWidth = CAN_SJW_1TQ;
//	CanHandle.Init.TimeSeg1 = CAN_BS1_6TQ;
//	CanHandle.Init.TimeSeg2 = CAN_BS2_2TQ;
//	CanHandle.Init.Prescaler = 4;

    /* CAN Baudrate = 1 MBps (CAN clocked at 36 MHz) 36Mhz / ((CAN_BS1 + CAN_BS2 + 1) * CAN_Prescaler)*/
    if (can_dev->baudrate == CAN_BAUDRATE_100K)
    {
		CanHandle.Init.TimeSeg1 = CAN_BS1_6TQ;
		CanHandle.Init.TimeSeg2 = CAN_BS2_2TQ;
		CanHandle.Init.Prescaler = 40;
    }
    else if (can_dev->baudrate == CAN_BAUDRATE_200K)
    {
		CanHandle.Init.TimeSeg1 = CAN_BS1_6TQ;
		CanHandle.Init.TimeSeg2 = CAN_BS2_2TQ;
		CanHandle.Init.Prescaler = 20;
    }
    else if (can_dev->baudrate == CAN_BAUDRATE_500K)
    {
		CanHandle.Init.TimeSeg1 = CAN_BS1_6TQ;
		CanHandle.Init.TimeSeg2 = CAN_BS2_2TQ;
		CanHandle.Init.Prescaler = 8;
    }
    else if (can_dev->baudrate == CAN_BAUDRATE_1M)
    {
		CanHandle.Init.TimeSeg1 = CAN_BS1_6TQ;
		CanHandle.Init.TimeSeg2 = CAN_BS2_2TQ;
		CanHandle.Init.Prescaler = 4;
    }
    else
    {
        //默认500K
		CanHandle.Init.TimeSeg1 = CAN_BS1_6TQ;
		CanHandle.Init.TimeSeg2 = CAN_BS2_2TQ;
		CanHandle.Init.Prescaler = 8;
    }
	
	if (HAL_CAN_Init(&CanHandle) != HAL_OK)
	{
		/* Initialization Error */
		printf("can init fail!\n\r");
//		Error_Handler();
	}
	
	  /* Configure the CAN Filter */
	sFilterConfig.FilterBank = 0;
	sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	sFilterConfig.FilterIdHigh = 0x0000;
	sFilterConfig.FilterIdLow = 0x0000;
	sFilterConfig.FilterMaskIdHigh = 0x0000;
	sFilterConfig.FilterMaskIdLow = 0x0000;
	sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
	sFilterConfig.FilterActivation = ENABLE;
	sFilterConfig.SlaveStartFilterBank = 14;
	
	if (HAL_CAN_ConfigFilter(&CanHandle, &sFilterConfig) != HAL_OK)
	{
		/* Filter configuration Error */
		printf("can configfilter fail!\n\r");
	}
	
	  /* Start the CAN peripheral */
	if (HAL_CAN_Start(&CanHandle) != HAL_OK)
	{
	/* Start Error */
		printf("can start fail!\n\r");
	//    Error_Handler();
	}
  
    /* Activate CAN RX notification */
  if (HAL_CAN_ActivateNotification(&CanHandle, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
  {
    /* Notification Error */
	  printf("can notification fail!\n\r");
//    Error_Handler();
  }
    

}

/***************************************************************************************************
*\Function      stm32_CAN_recv
*\Description   接收一帧报文
*\Parameter     can_dev
*\Parameter     rx
*\Return        bool
*\Note          
*\Log           2017.10.30    Ver 1.0    Job               
				创建函数。
***************************************************************************************************/
bool stm32_CAN_recv(CANBus_type* can_dev, CANBusRx_type* rx)
{
    if (can_dev == NULL || rx == NULL)
    {
        return FALSE;
    }
    
    if (can_dev->CanRx_Read == can_dev->CanRx_Write)
    {
        return FALSE;
    }

    memcpy(rx, &can_dev->CANRxBuf[can_dev->CanRx_Read++], sizeof(CANBusRx_type));
    can_dev->CanRx_Read %= CAN_RX_BUFFER_SIZE;

    return TRUE;
}


/***************************************************************************************************
*\Function      stm32_CAN_clear
*\Description   
*\Parameter     can_dev
*\Return        bool
*\Note          
*\Log           2017.11.18    Ver 1.0    Job               
				创建函数。
***************************************************************************************************/
bool stm32_CAN_clear(CANBus_type* can_dev)
{
    if (can_dev == NULL)
    {
        return FALSE;
    }

    can_dev->CanRx_Read = can_dev->CanRx_Write;

    return TRUE;
}
/***************************************************************************************************
*\Function      stm32_CAN_send
*\Description   发送一帧报文
*\Parameter     can_dev
*\Parameter     tx
*\Return        bool
*\Note          
*\Log           2017.10.30    Ver 1.0    Job               
				创建函数。
***************************************************************************************************/
bool stm32_CAN_send(CANBus_type* can_dev, CANBusTx_type* tx)
{
	u32 transmit_mailbox;
	u32 fail_times = 0;
	u32 uwCounter = 0;
	
	if (can_dev == NULL || tx == NULL)
	{
		return FALSE;
	}

	/* polling mode */
	while (1)
	{
		
		uwCounter = 0;
		while (uwCounter++ <= 0xFFFF)
		{
			if (HAL_CAN_AddTxMessage(&CanHandle,&(tx->CANTxHead), tx->Data, &transmit_mailbox) == HAL_OK)
			{
				return TRUE;
			}
		}
		fail_times++;
		if (fail_times >= CAN_TX_TRY_TIMES)
		{
			return FALSE;
		}
	}
}

/***************************************************************************************************
*\Function      stm32_CAN_recv_isr
*\Description   中断处理
*\Parameter     can_dev
*\Parameter     can_fifo
*\Return        void
*\Note          
*\Log           2017.10.30    Ver 1.0    Job               
				创建函数。
***************************************************************************************************/
void stm32_CAN_recv_isr(CANBus_type* can_dev, u8 can_fifo)
{
//    u32 it_status;

//    if(can_fifo == CAN_FIFO0)
//    {
//        it_status = CAN_IT_FMP0;
//    }
//    else if(can_fifo == CAN_FIFO1)
//    {
//        it_status = CAN_IT_FMP1;
//    }

//    if(CAN_GetITStatus(can_dev->can, it_status) != RESET)
//    {
//        /* save on rx buffer */
//        while (CAN_MessagePending(can_dev->can, can_fifo) > 0)
//        {
//            rt_base_t level;

//            /* disable interrupt */
//            level = rt_hw_interrupt_disable();

//            /* save character */
//            CAN_Receive(can_dev->can, can_fifo, &(can_dev->CANRxBuf[can_dev->CanRx_Write++]));
//            can_dev->CanRx_Write %= CAN_RX_BUFFER_SIZE;

//            if (can_dev->CanRx_Write == can_dev->CanRx_Read)
//            {
//                can_dev->CanRx_Read++;
//                can_dev->CanRx_Read %= CAN_RX_BUFFER_SIZE;
//            }
//            /* enable interrupt */
//            rt_hw_interrupt_enable(level);
//        }
//    }
}
/**
  * @brief  Rx Fifo 0 message pending callback in non blocking mode
  * @param  CanHandle: pointer to a CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @retval None
  */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *CanHandle)
{
	CAN_RxHeaderTypeDef   RxHeader;
	uint8_t               RxData[8];
  /* Get RX message */
  if (HAL_CAN_GetRxMessage(CanHandle, CAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK)
  {
    /* Reception Error */
	   printf("can reception fail!\n\r");
//    Error_Handler();
  }
//printf("can recv\n\r");
  
	memcpy(&CAN.CANRxBuf[CAN.CanRx_Write].CANRxHead, &RxHeader, sizeof(CAN_RxHeaderTypeDef));
	memcpy(&CAN.CANRxBuf[CAN.CanRx_Write].Data, RxData, sizeof(RxData));
    CAN.CanRx_Write++;
	CAN.CanRx_Write %= CAN_RX_BUFFER_SIZE;
	if (CAN.CanRx_Write == CAN.CanRx_Read)
	{
		CAN.CanRx_Read++;
		CAN.CanRx_Read %= CAN_RX_BUFFER_SIZE;
	} 

}


void USB_LP_CAN1_RX0_IRQHandler(void)
{
  /* USER CODE BEGIN USB_LP_CAN1_RX0_IRQn 0 */

  /* USER CODE END USB_LP_CAN1_RX0_IRQn 0 */
  HAL_CAN_IRQHandler(&CanHandle);
  /* USER CODE BEGIN USB_LP_CAN1_RX0_IRQn 1 */

  /* USER CODE END USB_LP_CAN1_RX0_IRQn 1 */
}


